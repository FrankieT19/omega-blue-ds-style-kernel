#ifndef DS_STYLE_BLUE_NOR_STATE_H
#define DS_STYLE_BLUE_NOR_STATE_H

#define BLUE_STATE_SIZE 0x70000u
#define BLUE_STATE_SIGNATURE 0x62ff0u
#define BLUE_STATE_CONTROL 0x63000u
#define BLUE_STATE_READY 0x4e525453u
#define BLUE_STATE_DIRTY 0x4e525444u

typedef struct { u32 key, inverse, status, status_inverse; } BlueStateControl;

u32 Blue_NORStateKey(const FM_NOR_FS *entry)
{
    u32 key = (2166136261u ^ entry->rompage) * 16777619u;
    key = (key ^ entry->filesize) * 16777619u;
    for(unsigned i = 0; i < sizeof(entry->gamename); i++)
        key = (key ^ (u8)entry->gamename[i]) * 16777619u;
    for(unsigned i = 0; i < sizeof(entry->filename) && entry->filename[i]; i++)
        key = (key ^ (u8)entry->filename[i]) * 16777619u;
    return key ? key : 1;
}

static void Blue_StatePaths(u32 key, char *file, char *temp, char *backup)
{
    snprintf(file, 48, "/SYSTEM/RTS/BLUE/NOR/%08lx.rts", key);
    snprintf(temp, 48, "/SYSTEM/RTS/BLUE/NOR/%08lx.tmp", key);
    snprintf(backup, 48, "/SYSTEM/RTS/BLUE/NOR/%08lx.bak", key);
}

static void Blue_StateTransfer(u32 offset, void *buffer, u32 size, int writing)
{
    void *address = PSRAMBase_S98 + 0x20000 + offset;
    SetPSRampage(0x6000);
    if(writing) dmaCopy(buffer, address, size);
    else dmaCopy(address, buffer, size);
    SetPSRampage(0);
}

static int Blue_PrepareNORState(u32 key)
{
    char file[48], temp[48], backup[48];
    BlueStateControl control = {0, 0, 0, 0};
    static const char *const parents[] = {"/SYSTEM/RTS", "/SYSTEM/RTS/BLUE", "/SYSTEM/RTS/BLUE/NOR"};
    Blue_StateTransfer(BLUE_STATE_CONTROL, &control, sizeof(control), 1);
    for(unsigned i = 0; i < sizeof(parents) / sizeof(parents[0]); i++)
    {
        FRESULT res = f_mkdir(parents[i]);
        if(res != FR_OK && res != FR_EXIST) return 0;
    }
    Blue_StatePaths(key, file, temp, backup);
    FRESULT res = f_stat(file, NULL);
    if(res == FR_NO_FILE)
    {
        res = f_stat(backup, NULL);
        if(res == FR_OK && f_rename(backup, file) != FR_OK) return 0;
    }
    if(res == FR_OK)
    {
        if(LoadRTSfile(file))
        {
            Blue_StateTransfer(BLUE_STATE_CONTROL, &control, sizeof(control), 1);
            return 0;
        }
        Blue_StateTransfer(BLUE_STATE_CONTROL, &control, sizeof(control), 0);
        u32 crc = 0xffffffff;
        for(u32 offset = 0; offset < BLUE_STATE_SIZE; offset += 0x10000)
        {
            Blue_StateTransfer(offset, pReadCache, 0x10000, 0);
            if(offset == (BLUE_STATE_CONTROL & ~0xffffu))
                memset(pReadCache + (BLUE_STATE_CONTROL & 0xffffu), 0, sizeof(control));
            crc = Blue_CRC32Update(crc, pReadCache, 0x10000);
        }
        crc ^= 0xffffffff;
        if(control.key != key || control.inverse != ~key ||
           control.status != crc || control.status_inverse != ~crc)
        {
            memset(&control, 0, sizeof(control));
            Blue_StateTransfer(BLUE_STATE_CONTROL, &control, sizeof(control), 1);
            return 0;
        }
    }
    else if(res == FR_NO_FILE)
        Blue_StateTransfer(BLUE_STATE_SIGNATURE, &control, sizeof(control), 1);
    else return 0;
    control = (BlueStateControl){key, ~key, BLUE_STATE_READY, ~BLUE_STATE_READY};
    Blue_StateTransfer(BLUE_STATE_CONTROL, &control, sizeof(control), 1);
    return 1;
}

/* Only a verified warm return may commit DDR data through FatFs. */
static int Blue_FlushNORState(u32 key)
{
    BlueStateControl control;
    Blue_StateTransfer(BLUE_STATE_CONTROL, &control, sizeof(control), 0);
    if(control.key != key || control.inverse != ~key || control.status != BLUE_STATE_DIRTY ||
       control.status_inverse != ~BLUE_STATE_DIRTY) return 1;

    char file[48], temp[48], backup[48], signature[16] __attribute__((aligned(4)));
    Blue_StateTransfer(BLUE_STATE_SIGNATURE, signature, sizeof(signature), 0);
    if(memcmp(signature, "EZ-OmegaRTCFILE.", sizeof(signature))) return 0;
    Blue_StatePaths(key, file, temp, backup);
    FIL out;
    if(f_open(&out, temp, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) return 0;
    int ok = 1;
    u32 crc = 0xffffffff;
    for(u32 offset = 0; offset < BLUE_STATE_SIZE; offset += 0x10000)
    {
        UINT written;
        Blue_StateTransfer(offset, pReadCache, 0x10000, 0);
        if(offset == (BLUE_STATE_CONTROL & ~0xffffu))
            memset(pReadCache + (BLUE_STATE_CONTROL & 0xffffu), 0, sizeof(control));
        crc = Blue_CRC32Update(crc, pReadCache, 0x10000);
        if(f_write(&out, pReadCache, 0x10000, &written) != FR_OK || written != 0x10000)
        {
            ok = 0;
            break;
        }
    }
    crc ^= 0xffffffff;
    control = (BlueStateControl){key, ~key, crc, ~crc};
    if(ok)
    {
        UINT written;
        ok = f_lseek(&out, BLUE_STATE_CONTROL) == FR_OK &&
             f_write(&out, &control, sizeof(control), &written) == FR_OK && written == sizeof(control);
    }
    if(ok && f_sync(&out) != FR_OK) ok = 0;
    if(f_close(&out) != FR_OK) ok = 0;
    if(!ok) { f_unlink(temp); return 0; }

    FRESULT exists = f_stat(file, NULL);
    if(exists == FR_OK)
    {
        FRESULT res = f_unlink(backup);
        if((res != FR_OK && res != FR_NO_FILE) || f_rename(file, backup) != FR_OK) return 0;
    }
    else if(exists != FR_NO_FILE) return 0;
    if(f_rename(temp, file) != FR_OK)
    {
        if(exists == FR_OK) f_rename(backup, file);
        return 0;
    }
    memset(&control, 0, sizeof(control));
    Blue_StateTransfer(BLUE_STATE_CONTROL, &control, sizeof(control), 1);
    return 1;
}

#endif
