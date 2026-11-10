#ifndef DS_STYLE_BLUE_STYLE_H
#define DS_STYLE_BLUE_STYLE_H

/* Only images built for the Blue layout, not arbitrary files named .bin. */
static int Blue_ValidateStyle(const TCHAR *path)
{
    FIL file;
    UINT got;
    u8 header[0xc0];
    u32 crc = 0xffffffff;
    u8 check = 0x19;
    int valid = 0;
    if(f_open(&file, path, FA_READ) != FR_OK) return 0;
    if(f_size(&file) != 0x2069dc ||
       f_read(&file, header, sizeof(header), &got) != FR_OK || got != sizeof(header) ||
       memcmp(header + 0xa0, "DSBLUE", 6) || header[0xb2] != 0x96 ||
       (header[3] != 0xea)) goto done;
    for(unsigned i = 0xa0; i < 0xbe; i++) check += header[i];
    if(check || f_lseek(&file, 0x195000) != FR_OK) goto done;
    for(u32 left = 0x2069dc - 0x195000; left;)
    {
        UINT count = left < 0x20000 ? left : 0x20000;
        if(f_read(&file, pReadCache, count, &got) != FR_OK || got != count) goto done;
        crc = Blue_CRC32Update(crc, pReadCache, count);
        left -= count;
    }
    valid = (crc ^ 0xffffffff) == 0x06b54368;
done:
    return f_close(&file) == FR_OK && valid;
}

#endif
