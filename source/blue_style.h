#ifndef DS_STYLE_BLUE_STYLE_H
#define DS_STYLE_BLUE_STYLE_H

#define BLUE_STYLE_IMAGE_SIZE 0x2069dcu
#define BLUE_STYLE_SEAL_OFFSET (BLUE_STYLE_IMAGE_SIZE - 16u)

/* The builder seals the entire firmware-free kernel image. */
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
       memcmp(seal.tag, "DSBLUE5", sizeof(seal.tag)) ||
       seal.crc != (crc ^ 0xffffffff) || seal.inverse != ~seal.crc) goto done;
    valid = 1;
done:
    return f_close(&file) == FR_OK && valid;
}

#endif
