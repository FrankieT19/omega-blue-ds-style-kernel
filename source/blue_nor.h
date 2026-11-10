#ifndef DS_STYLE_BLUE_NOR_H
#define DS_STYLE_BLUE_NOR_H

/* Blue uses 128 KiB allocation units and an independent on-flash map. */
_Static_assert(sizeof(FM_NOR_FS) == 132, "Blue NOR record size");
_Static_assert((NOR_end_address - First_game_address) / 0x20000 == NOR_FAT_SIZE,
               "Blue NOR geometry");

static unsigned blue_nor_can_write;

static int Blue_NORExtent(const FM_NOR_FS *entry, u32 *start, u32 *length)
{
    const u32 first = First_game_address >> 17;
    if(entry->rompage < first || (entry->filesize & 0x1ffff)) return 0;
    *start = entry->rompage - first;
    *length = entry->filesize >> 17;
    return *start < NOR_FAT_SIZE && *length && *length <= NOR_FAT_SIZE - *start;
}

/* Refreshing the browser must never erase or program flash. */
static void Blue_ReadNOR(void)
{
    u8 occupied[NOR_FAT_SIZE / 8] = {0};
    unsigned count = 0;
    blue_nor_can_write = 1;
    Read_NORgame_info();
    Read_NOR_FAT_info();
    for(unsigned i = 0; i < MAX_NOR; i++)
    {
        FM_NOR_FS *entry = &pNorFS[i];
        u32 first, length;
        if(!entry->rompage || entry->rompage == 0xffff) continue;
        if(!Blue_NORExtent(entry, &first, &length) || !entry->filename[0] ||
           !memchr(entry->filename, 0, sizeof(entry->filename)))
            goto invalid;
        for(unsigned j = first; j < first + length; j++)
        {
            if(occupied[j >> 3] & (1u << (j & 7))) goto invalid;
            occupied[j >> 3] |= 1u << (j & 7);
        }
        if(count != i) pNorFS[count] = *entry;
        count++;
    }
    for(unsigned i = 0; i < NOR_FAT_SIZE; i++)
        if(!!NOR_FAT_buffer[i] != !!(occupied[i >> 3] & (1u << (i & 7))))
            blue_nor_can_write = 0;
    /* Mode B and manual FRAM transfer address the first physical ROM. */
    for(unsigned i = 1; i < count; i++)
    {
        FM_NOR_FS entry = pNorFS[i];
        unsigned j = i;
        while(j && pNorFS[j - 1].rompage > entry.rompage)
        {
            pNorFS[j] = pNorFS[j - 1];
            j--;
        }
        pNorFS[j] = entry;
    }
    game_total_NOR = count;
    memset(pNorFS + count, 0, sizeof(pNorFS[0]) * (MAX_NOR - count));
    return;

invalid:
    blue_nor_can_write = 0;
    game_total_NOR = 0;
    memset(pNorFS, 0, sizeof(pNorFS));
}

int Blue_PrepareNORWrite(void)
{
    Blue_ReadNOR();
    return blue_nor_can_write && game_total_NOR < MAX_NOR;
}

static int Blue_DeleteNOR(u32 item)
{
    u32 first, length;
    if(!blue_nor_can_write || item >= game_total_NOR ||
       !Blue_NORExtent(&pNorFS[item], &first, &length)) return 0;
    Block_Erase((u32)pNorFS[item].rompage << 17);
    memset(NOR_FAT_buffer + first, 0, length);
    memmove(pNorFS + item, pNorFS + item + 1,
            (game_total_NOR - item - 1) * sizeof(pNorFS[0]));
    memset(&pNorFS[--game_total_NOR], 0, sizeof(pNorFS[0]));
    Save_NORgame_info((u16*)pNorFS, sizeof(pNorFS));
    Save_NOR_FAT_info();
    return 1;
}

#endif
