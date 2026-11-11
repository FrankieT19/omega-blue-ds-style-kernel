#ifndef DS_STYLE_BLUE_LEGACY_NOR_H
#define DS_STYLE_BLUE_LEGACY_NOR_H

/* Stock Blue K1.02's combined RTS patch, measured from its ARM source. */
#define BLUE_STOCK_RTS_PAGE 0x7fcu
#define BLUE_STOCK_RTS_MAP 0x800u
#define BLUE_STOCK_RTS_SIGNATURE 0x8a8u
#define BLUE_STOCK_RTS_SWITCH 0xf18u

static void IWRAM_CODE Blue_ReadNORBytes(u32 address, void *dest, u32 length)
{
    u8 *out = dest;
    while(length)
    {
        u32 offset = address & 0x7fffff;
        u32 count = 0x800000 - offset;
        if(count > length) count = length;
        SetRompage_MODE((address >> 23) << 12, SYSTEM_MODE_OS);
        const volatile u8 *src = (const volatile u8 *)FlashBase_S98 + offset;
        length -= count;
        address += count;
        while(count--) *out++ = *src++;
    }
    SetRompage_MODE(0, SYSTEM_MODE_OS);
}

static int Blue_MatchesLegacyMap(const u32 stored[16], const u32 live[16])
{
    u32 previous = 0;
    for(unsigned i = 0; i < 16; i += 2)
    {
        u32 offset = stored[i];
        if(offset != live[i] || stored[i + 1] != live[i + 1]) return 0;
        if(offset == 0xffffffff) return i && stored[i + 1] == 0;
        if((!i && offset) || (i && offset <= previous) || (offset & 3) ||
           offset >= 0x380 || !stored[i + 1]) return 0;
        previous = offset;
    }
    return 0;
}

static int Blue_PrepareLegacyNORState(const FM_NOR_FS *entry)
{
    static const u32 stock_prefix[] = {0xe3a00301, 0xe28f100c, 0xe5001004, 0xe51f0000, 0xe12fff10};
    static const char *const folders[] = {"/RTS", "/SYSTEM/RTS/BLUE"};
    u32 prefix[5], branch, page, enabled, map[16];
    char signature[16] __attribute__((aligned(4)));
    u32 address = (u32)entry->rompage << 17;
    Blue_ReadNORBytes(address, &branch, sizeof(branch));
    if((branch & 0xff800000) != 0xea000000) return 0;
    u32 trim = ((branch & 0x7fffff) << 2) + 8;
    if(trim < 0xc0 || trim > entry->filesize || entry->filesize - trim < BLUE_STOCK_RTS_SWITCH + 4)
        return 0;
    address += trim;
    Blue_ReadNORBytes(address, prefix, sizeof(prefix));
    if(memcmp(prefix, stock_prefix, sizeof(prefix))) return 0;
    Blue_ReadNORBytes(address + BLUE_STOCK_RTS_PAGE, &page, sizeof(page));
    Blue_ReadNORBytes(address + BLUE_STOCK_RTS_MAP, map, sizeof(map));
    Blue_ReadNORBytes(address + BLUE_STOCK_RTS_SIGNATURE, signature, sizeof(signature));
    Blue_ReadNORBytes(address + BLUE_STOCK_RTS_SWITCH, &enabled, sizeof(enabled));
    if(page != entry->rompage || enabled != 1 || memcmp(signature, "EZ-OmegaRTCFILE.", sizeof(signature)))
        return 0;

    size_t len = strlen(entry->filename);
    if(len < 4 || len >= sizeof(entry->filename)) return 0;
    for(unsigned i = 0; i < sizeof(folders) / sizeof(folders[0]); ++i)
    {
        char path[132];
        snprintf(path, sizeof(path), "%s/%.*s.rts", folders[i], (int)len - 4, entry->filename);
        if(Check_game_RTS_FAT(path, 3) == 0xffffffff ||
           !Blue_MatchesLegacyMap(map, FAT_table_buffer + FAT_table_RTS_offset / 4) || LoadRTSfile(path)) continue;
        Blue_StateTransfer(BLUE_STATE_SIGNATURE, signature, sizeof(signature), 0);
        if(!memcmp(signature, "EZ-OmegaRTCFILE.", sizeof(signature))) return 1;
    }
    return 0;
}

#endif
