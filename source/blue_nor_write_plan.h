#ifndef DS_STYLE_BLUE_NOR_WRITE_PLAN_H
#define DS_STYLE_BLUE_NOR_WRITE_PLAN_H

#define BLUE_NOR_SECTOR_SIZE 0x20000u
#define BLUE_NOR_PATCH_RESERVE 0x2000u

static int Blue_NORWritePlan(u32 size, int patched, u32 trim, u32 *program, u32 *allocate)
{
    if(size < 0xc0 || size > 0x4000000) return 0;
    u32 end = size;
    if(patched)
    {
        if(size > 0x2000000 || trim < 0xc0 || (trim & 3) ||
           trim > 0x2000000 - BLUE_NOR_PATCH_RESERVE ||
           (trim & (BLUE_NOR_SECTOR_SIZE - 1)) > BLUE_NOR_SECTOR_SIZE - BLUE_NOR_PATCH_RESERVE)
            return 0;
        if(trim + BLUE_NOR_PATCH_RESERVE > end) end = trim + BLUE_NOR_PATCH_RESERVE;
    }
    *program = (end + BLUE_NOR_SECTOR_SIZE - 1) & ~(BLUE_NOR_SECTOR_SIZE - 1);
    *allocate = (*program + BLUE_NOR_SECTOR_SIZE) & ~(2 * BLUE_NOR_SECTOR_SIZE - 1);
    return 1;
}

#endif
