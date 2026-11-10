#ifndef DS_STYLE_BLUE_RTS_LOAD_H
#define DS_STYLE_BLUE_RTS_LOAD_H

/* The Blue engine expects its 448 KiB image in DDR page 0x6000. */
u32 IWRAM_CODE LoadRTSfile(TCHAR *filename)
{
    FIL file;
    if(f_open(&file, filename, FA_READ) != FR_OK) return 1;
    if(f_size(&file) != 0x70000)
    {
        f_close(&file);
        return 1;
    }
    for(u32 offset = 0; offset < 0x70000; offset += 0x20000)
    {
        UINT count;
        UINT want = offset == 0x60000 ? 0x10000 : 0x20000;
        if(f_read(&file, pReadCache, want, &count) != FR_OK || count != want)
        {
            f_close(&file);
            return 1;
        }
        SetPSRampage(0x6000);
        dmaCopy(pReadCache, PSRAMBase_S98 + 0x20000 + offset, want);
        SetPSRampage(0);
    }
    return f_close(&file) != FR_OK;
}

#endif
