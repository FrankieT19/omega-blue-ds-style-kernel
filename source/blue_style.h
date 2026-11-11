#ifndef DS_STYLE_BLUE_STYLE_H
#define DS_STYLE_BLUE_STYLE_H

#define BLUE_STYLE_SEAL_OFFSET 0x194ff0u
#define BLUE_STYLE_FW_OFFSET 0x195000u
#define BLUE_STYLE_IMAGE_SIZE 0x2069dcu

/* The builder seals everything before the vendor firmware. */
static int Blue_ValidateStyle(const TCHAR *path)
{
    FIL file;
    UINT got;
    u8 header[0xc0];
    struct { char tag[8]; u32 crc, inverse; } seal;
    u32 crc = 0xffffffff;
    u8 check = 0x19;
    int valid = 0;
    if(f_open(&file, path, FA_READ) != FR_OK) return 0;
    if(f_size(&file) != BLUE_STYLE_IMAGE_SIZE ||
       f_read(&file, header, sizeof(header), &got) != FR_OK || got != sizeof(header) ||
       memcmp(header + 0xa0, "DSBLUE", 6) || header[0xb2] != 0x96 ||
       (header[3] != 0xea)) goto done;
    for(unsigned i = 0xa0; i < 0xbe; i++) check += header[i];
    if(check) goto done;
    crc = Blue_CRC32Update(crc, header, sizeof(header));
    for(u32 left = BLUE_STYLE_SEAL_OFFSET - sizeof(header); left;)
    {
        UINT count = left < 0x20000 ? left : 0x20000;
        if(f_read(&file, pReadCache, count, &got) != FR_OK || got != count) goto done;
        crc = Blue_CRC32Update(crc, pReadCache, count);
        left -= count;
    }
    if(f_read(&file, &seal, sizeof(seal), &got) != FR_OK || got != sizeof(seal) ||
       memcmp(seal.tag, "DSBLUE3", sizeof(seal.tag)) ||
       seal.crc != (crc ^ 0xffffffff) || seal.inverse != ~seal.crc) goto done;
    crc = 0xffffffff;
    for(u32 left = BLUE_STYLE_IMAGE_SIZE - BLUE_STYLE_FW_OFFSET; left;)
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
