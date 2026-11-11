#ifndef DS_STYLE_BLUE_LARGE_ROM_H
#define DS_STYLE_BLUE_LARGE_ROM_H

#define BLUE_ADDON_WINDOW 0x2000000u
#define BLUE_ADDON_SCRATCH 0x3000000u

static int Blue_IsPadding(const u8 *data, u32 length)
{
    if(!length) return 1;
    u8 fill = *data;
    if(fill && fill != 0xff) return 0;
    while(length--) if(*data++ != fill) return 0;
    return 1;
}

static u32 Blue_LargePatchOffset(const u8 *last_sector)
{
    const u32 start = BLUE_ADDON_WINDOW - BLUE_NOR_SECTOR_SIZE;
    u32 run = 0;
    u8 fill = 0;
    for(u32 i = BLUE_NOR_SECTOR_SIZE; i;)
    {
        u8 value = last_sector[--i];
        if(value && value != 0xff) { run = 0; continue; }
        run = run && value == fill ? run + 1 : 1;
        fill = value;
        if(run >= BLUE_NOR_PATCH_RESERVE && !(i & 15)) return start + i;
    }
    return 0;
}

static u32 Blue_PrepareLargeAddon(const TCHAR *name, u32 size, int sd, int rts)
{
    FIL file;
    UINT got;
    u32 offset = 0;
    if(f_open(&file, name, FA_READ) != FR_OK) return 0;
    if(f_size(&file) == size && f_lseek(&file, BLUE_ADDON_WINDOW - BLUE_NOR_SECTOR_SIZE) == FR_OK &&
       f_read(&file, pReadCache, BLUE_NOR_SECTOR_SIZE, &got) == FR_OK && got == BLUE_NOR_SECTOR_SIZE)
        offset = Blue_LargePatchOffset(pReadCache);

    /* Stock's game-mode DDR switch uses the latter 32 MiB for scratch. */
    if(offset && sd && size > BLUE_ADDON_SCRATCH)
    {
        u32 left = size - BLUE_ADDON_SCRATCH;
        u32 span = rts ? 0x90000u : 0x12c00u;
        if(left > span) left = span;
        if(f_lseek(&file, BLUE_ADDON_SCRATCH) != FR_OK) offset = 0;
        while(offset && left)
        {
            UINT count = left > BLUE_NOR_SECTOR_SIZE ? BLUE_NOR_SECTOR_SIZE : left;
            if(f_read(&file, pReadCache, count, &got) != FR_OK || got != count ||
               !Blue_IsPadding(pReadCache, count)) offset = 0;
            left -= count;
        }
    }
    return f_close(&file) == FR_OK ? offset : 0;
}

#endif
