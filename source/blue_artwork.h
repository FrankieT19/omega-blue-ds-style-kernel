#ifndef DS_STYLE_BLUE_ARTWORK_H
#define DS_STYLE_BLUE_ARTWORK_H

#include <stdint.h>

typedef struct {
    uint16_t width, height;
    const uint8_t *rows;
    const uint16_t *offsets;
    const uint16_t *pixels;
} BlueArtwork;

/* Restore a clipped span without a full-screen RAM buffer. Halfword
   stores work for both a RAM composition target and mode 3 VRAM. */
static inline void BlueArtwork_CopySpan(const BlueArtwork *art,
                                        volatile uint16_t *dst,
                                        unsigned x, unsigned y, unsigned count)
{
    if(x >= art->width || y >= art->height) return;
    if(count > art->width - x) count = art->width - x;
    uint16_t entry = art->offsets[art->rows[y]];
    const uint16_t *src = art->pixels + (entry & 0x7fff);
    if(entry & 0x8000)
    {
        uint16_t colour = *src;
        while(count--) *dst++ = colour;
    }
    else
    {
        src += x;
        while(count--) *dst++ = *src++;
    }
}

#endif
