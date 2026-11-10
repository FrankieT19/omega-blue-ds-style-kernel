#ifndef DS_STYLE_BLUE_ARTWORK_BRIDGE_H
#define DS_STYLE_BLUE_ARTWORK_BRIDGE_H

#include <gba_dma.h>
#include "blue_artwork.h"

const BlueArtwork *BlueArtwork_Find(const void *base);

static inline void BlueArtwork_Restore(const uint16_t *base, unsigned stride,
                                       volatile uint16_t *dst, unsigned x,
                                       unsigned y, unsigned count)
{
    const BlueArtwork *art = BlueArtwork_Find(base);
    if(art) BlueArtwork_CopySpan(art, dst, x, y, count);
    else if(count) dmaCopy(base + y * stride + x, (void*)dst, count * 2);
}

static inline void BlueArtwork_DrawPopupClip(const uint16_t *base, unsigned sx,
                                            unsigned sy, unsigned x, unsigned y,
                                            unsigned w, unsigned h)
{
    for(unsigned row = 0; row < h; row++)
        BlueArtwork_Restore(base, 168, (volatile uint16_t*)0x06000000 +
                            (y + row) * 240 + x, sx, sy + row, w);
}

#endif
