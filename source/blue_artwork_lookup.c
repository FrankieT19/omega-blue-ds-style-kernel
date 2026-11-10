#include <stddef.h>
#include "blue_artwork_bridge.h"
extern const BlueArtwork blue_HELP_BLANK;
const unsigned char gImage_HELP_BLANK[4] __attribute__((aligned(4))) = {1};
extern const BlueArtwork blue_HELP_DARK;
const unsigned char gImage_HELP_DARK[4] __attribute__((aligned(4))) = {2};
extern const BlueArtwork blue_MENU_BLANK;
const unsigned char gImage_MENU_BLANK[4] __attribute__((aligned(4))) = {3};
extern const BlueArtwork blue_MENU_DARK;
const unsigned char gImage_MENU_DARK[4] __attribute__((aligned(4))) = {4};
extern const BlueArtwork blue_SD_HORIZONTAL_BLANK;
const unsigned char gImage_SD_HORIZONTAL_BLANK[4] __attribute__((aligned(4))) = {5};
extern const BlueArtwork blue_SD_HORIZONTAL_DARK;
const unsigned char gImage_SD_HORIZONTAL_DARK[4] __attribute__((aligned(4))) = {6};
extern const BlueArtwork blue_SD_LIST_BLANK;
const unsigned char gImage_SD_LIST_BLANK[4] __attribute__((aligned(4))) = {7};
extern const BlueArtwork blue_SD_LIST_DARK;
const unsigned char gImage_SD_LIST_DARK[4] __attribute__((aligned(4))) = {8};
extern const BlueArtwork blue_SD_VERTICAL_BLANK;
const unsigned char gImage_SD_VERTICAL_BLANK[4] __attribute__((aligned(4))) = {9};
extern const BlueArtwork blue_SD_VERTICAL_DARK;
const unsigned char gImage_SD_VERTICAL_DARK[4] __attribute__((aligned(4))) = {10};
extern const BlueArtwork blue_SET_BLANK;
const unsigned char gImage_SET_BLANK[4] __attribute__((aligned(4))) = {11};
extern const BlueArtwork blue_SET_DARK;
const unsigned char gImage_SET_DARK[4] __attribute__((aligned(4))) = {12};
extern const BlueArtwork blue_START_BLANK;
const unsigned char gImage_START_BLANK[4] __attribute__((aligned(4))) = {13};
extern const BlueArtwork blue_START_DARK;
const unsigned char gImage_START_DARK[4] __attribute__((aligned(4))) = {14};
const BlueArtwork *BlueArtwork_Find(const void *base)
{
    if(base == gImage_HELP_BLANK) return &blue_HELP_BLANK;
    if(base == gImage_HELP_DARK) return &blue_HELP_DARK;
    if(base == gImage_MENU_BLANK) return &blue_MENU_BLANK;
    if(base == gImage_MENU_DARK) return &blue_MENU_DARK;
    if(base == gImage_SD_HORIZONTAL_BLANK) return &blue_SD_HORIZONTAL_BLANK;
    if(base == gImage_SD_HORIZONTAL_DARK) return &blue_SD_HORIZONTAL_DARK;
    if(base == gImage_SD_LIST_BLANK) return &blue_SD_LIST_BLANK;
    if(base == gImage_SD_LIST_DARK) return &blue_SD_LIST_DARK;
    if(base == gImage_SD_VERTICAL_BLANK) return &blue_SD_VERTICAL_BLANK;
    if(base == gImage_SD_VERTICAL_DARK) return &blue_SD_VERTICAL_DARK;
    if(base == gImage_SET_BLANK) return &blue_SET_BLANK;
    if(base == gImage_SET_DARK) return &blue_SET_DARK;
    if(base == gImage_START_BLANK) return &blue_START_BLANK;
    if(base == gImage_START_DARK) return &blue_START_DARK;
    return NULL;
}
