#ifndef DS_STYLE_BLUE_PLUGIN_H
#define DS_STYLE_BLUE_PLUGIN_H

#define BLUE_PLUGIN_LIMIT 0x7f8000u
#define BLUE_MULTIBOOT_LIMIT 0x3fbf8u

static int Blue_IsDocument(const TCHAR *name)
{
    const TCHAR *ext = strrchr(name, '.');
    return ext && (!strcasecmp(ext, ".txt") || !strcasecmp(ext, ".md") ||
                   !strcasecmp(ext, ".markdown"));
}

/* Validate the stream before handing it to the BIOS decompressor. */
static int Blue_ValidMultibootLZ(const u8 *data, u32 size)
{
    if(size < 4 || data[0] != 0x10) return 0;
    u32 expected = data[1] | ((u32)data[2] << 8) | ((u32)data[3] << 16);
    if(expected < 4 || expected > BLUE_MULTIBOOT_LIMIT) return 0;
    u32 input = 4, output = 0;
    while(output < expected)
    {
        if(input == size) return 0;
        unsigned flags = data[input++];
        for(unsigned bit = 0x80; bit && output < expected; bit >>= 1)
        {
            if(input == size) return 0;
            u8 first = data[input++];
            if(!(flags & bit)) { output++; continue; }
            if(input == size) return 0;
            u32 back = ((first & 0xf) << 8) + data[input++] + 1;
            u32 count = (first >> 4) + 3;
            if(back > output || count > expected - output) return 0;
            output += count;
        }
    }
    return 1;
}

static int Blue_CopyPluginFile(FIL *file, u32 offset, u32 size)
{
    for(u32 pos = 0; pos < size;)
    {
        UINT got;
        UINT chunk = size - pos > 0x20000 ? 0x20000 : size - pos;
        if(f_read(file, pReadCache, chunk, &got) != FR_OK || got != chunk) return 0;
        if(chunk & 3) memset(pReadCache + chunk, 0, -chunk & 3u);
        dmaCopy(pReadCache, PSRAMBase_S98 + offset + pos, (chunk + 3) & ~3u);
        pos += chunk;
    }
    return 1;
}

static u32 Blue_LoadPlugin(TCHAR *filename, u32 type)
{
    FIL app, document;
    if(f_open(&app, plugin, FA_READ) != FR_OK) return 1;
    if(f_open(&document, filename, FA_READ) != FR_OK)
    {
        f_close(&app);
        return 1;
    }
    const FSIZE_t app_size = f_size(&app), file_size = f_size(&document);
    int ok = 0;
    SetPSRampage(0);
    if(app_size >= 4 && app_size <= BLUE_PLUGIN_LIMIT && file_size <= BLUE_PLUGIN_LIMIT)
    {
        u32 start = ((u32)app_size + 3) & ~3u;
        u32 directory = (start + (u32)file_size + 0x7fff) & ~0x7fffu;
        if(directory <= BLUE_PLUGIN_LIMIT &&
           (type != 6 || start <= BLUE_MULTIBOOT_LIMIT) &&
           Blue_CopyPluginFile(&app, 0, app_size) &&
           (type != 7 || Blue_ValidMultibootLZ(PSRAMBase_S98, app_size)) &&
           Blue_CopyPluginFile(&document, start, file_size))
        {
            make_pogoshell_arguments(plugin + sizeof("/SYSTEM/PLUG/BLUE/") - 1,
                                     filename, start, file_size, directory, directory);
            ok = 1;
        }
    }
    if(f_close(&app) != FR_OK) ok = 0;
    if(f_close(&document) != FR_OK) ok = 0;
    return !ok;
}

#endif
