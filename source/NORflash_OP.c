#include <gba_interrupt.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <gba_base.h>
#include <gba_dma.h>
#include <string.h>
#include <gba_systemcalls.h>

#include "ez_define.h"
#include "NORflash_OP.h"
#include "ezkernel.h"
#include "draw.h"
#include "lang.h"
#include "Ezcard_OP.h"
#include "GBApatch.h"
//#define DEBUG

extern FM_NOR_FS pNorFS[MAX_NOR]EWRAM_BSS;
extern u8 pReadCache [MAX_pReadCache_size]EWRAM_BSS;
extern u32 gl_currentpage;
extern u32 gl_norOffset;
extern FIL gfile;
extern u32 game_total_NOR;
extern u32 iTrimSize;

extern u8 NOR_FAT_buffer[NOR_FAT_SIZE]EWRAM_BSS;
//---------------------------------------------------------------
void IWRAM_CODE Chip_Reset()
{
	SetRompage_MODE(0x8000,SYSTEM_MODE_OS);//select flash1
	*((vu16 *)(FlashBase_S98)) = 0xF0 ;
	SetRompage_MODE(0x0000,SYSTEM_MODE_OS);//select flash0
	*((vu16 *)(FlashBase_S98)) = 0xF0 ;
	
}
//---------------------------------------------------------------
void IWRAM_CODE Block_Erase(u32 blockAdd) //0x20000 BYTE pre block
{
	vu16 page,v1,v2;
	u32 Address;
	u32 loop;

	page=gl_currentpage;
	Address=blockAdd;
	while(Address>=0x800000)
	{
		Address-=0x800000;
		page+=0x1000;
	}
	//u8 ramdata = *((vu8 *)(0xE000000+1)) ;
	
	SetRompage_MODE(page,SYSTEM_MODE_OS);
	//u16 norid = Read_S98NOR_ID();

	v1=0;v2=1;
	if((blockAdd==0) || (blockAdd==0x3FE0000)   || (blockAdd==0x4000000) || (blockAdd==0x7FE0000) )
	{
		//Address=blockAdd;
		for(loop=0;loop<0x20000;loop+=0x8000)
		{
			*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
			*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
			*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
			*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase_S98+Address+loop)) = 0x30 ;
			do
			{
				v1 = *((vu16 *)(FlashBase_S98+Address+loop)) ;
				v2 = *((vu16 *)(FlashBase_S98+Address+loop)) ;
			}while(v1!=v2);
		}
	}
	else
	{
		loop=Address;

		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+loop)) = 0x30 ;
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+loop)) ;
			v2 = *((vu16 *)(FlashBase_S98+loop)) ;
		}while(v1!=v2);
	}
	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);
}
//-----------------------------------------------------------
/*void Chip_Erase()
{
	char msg[128];
	u32 count=0;
	vu16 v1,v2=0 ;
	REG_IME = 0 ;
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
	*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
	*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0x10 ;
	do
	{
		VBlankIntrWait();		
		VBlankIntrWait();	
		ShowTime(NOR_list,0);
		Clear(56, 103, 128, 13, gl_color_cheat_black, 1);//show menu pic
		
		itoa(count,msg,2);
		DrawHZText12(msg,0,60,90+13,gl_color_text,1);
		count++;

		VBlankIntrWait();
		v1 = *((vu16 *)(FlashBase_S98)) ;
		v2 = *((vu16 *)(FlashBase_S98)) ;		
	}while(v1!=v2);
	REG_IME = 1 ;
}
*/
//-----------------------------------------------------------
void IWRAM_CODE FormatNor()
{
	char msg[128];
	sprintf(msg,"%s",gl_formatnor_info1);
	DrawHZText12(msg,0,60,90+13,gl_color_text,1);
	
	u32 blocknum;
	
	while(1)
	{
		delay(500);
		scanKeys();
		u16 keys = keysDown();
		if (keys & KEY_A) {
			//Chip_Erase();
			
			for(blocknum=First_game_address;blocknum<NOR_end_address;blocknum+=0x20000)
			{		
				sprintf(msg,"%luMb/%luMb",(blocknum)/0x20000,NOR_end_address/0x20000);
				//Clear(70+54,160-15,100,15,gl_color_cheat_black,1);
				//DrawHZText12(msg,0,70+54,160-15,gl_color_text,1);
				Clear(56, 103, 128, 13, gl_color_cheat_black, 1);//show menu pic
				DrawHZText12(msg,0,60,90+13,gl_color_text,1);
				Block_Erase(blocknum);
			}		
			memset(pNorFS,0,sizeof(pNorFS));
            memset(NOR_FAT_buffer,0,NOR_FAT_SIZE);
            Save_NORgame_info((u16*)pNorFS,sizeof(pNorFS));
            Save_NOR_FAT_info();
            return;
		}
		else if(keys & KEY_B) {
			return;
		}
	}
}
// --------------------------------------------------------------------
void IWRAM_CODE WriteFlash_with64word(u32 address,u8 *buffer,u32 size)
{
	vu16 page,v1,v2;
	register u32 loopwrite ;
	vu16* buf = (vu16*)buffer ;
	u32 i;

	Chip_Reset();
	page=gl_currentpage;
	while(address>=0x800000)
	{
		address-=0x800000;
		page+=0x1000;
	}
	SetRompage_MODE(page,SYSTEM_MODE_OS);

	//
	v1=0;v2=1;
	for(loopwrite=0;loopwrite<(size/64);loopwrite++)
	{
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+address+loopwrite*64)) = 0x25;
		*((vu16 *)(FlashBase_S98+address+loopwrite*64)) = 31;
		for(i=0;i<=31;i++)
		{
			*((vu16 *)(FlashBase_S98+address+loopwrite*64 +2*i )) = buf[loopwrite*32+i];
		}	
		*((vu16 *)(FlashBase_S98+address+loopwrite*64)) = 0x29;
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+address+loopwrite*64)) ;
			v2 = *((vu16 *)(FlashBase_S98+address+loopwrite*64)) ;
		}while(v1!=v2);
		
		//DelayMicroseconds(50);
		
	}

	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);
}
//---------------------------------------------------------------
void IWRAM_CODE WriteFlash_with32word(u32 address,u8 *buffer,u32 size)
{
	vu16 page,v1,v2;
	register u32 loopwrite ;
	vu16* buf = (vu16*)buffer ;
	u32 i;

	page=gl_currentpage;
	while(address>=0x800000)
	{
		address-=0x800000;
		page+=0x1000;
	}
	SetRompage_MODE(page,SYSTEM_MODE_OS);

	//Chip_Reset();
	v1=0;v2=1;
	for(loopwrite=0;loopwrite<(size/32);loopwrite++)
	{
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 0x25;
		*((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 15;
		for(i=0;i<=15;i++)
		{
			*((vu16 *)(FlashBase_S98+address+loopwrite*32 +2*i )) = buf[loopwrite*16+i];
		}	
		*((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 0x29;
		
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+address+loopwrite*32+0xF*2)) ;
			v2 = *((vu16 *)(FlashBase_S98+address+loopwrite*32+0xF*2)) ;
		}while(v1!=v2);
	}
	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);
}
//---------------------------------------------------------------
void IWRAM_CODE WriteFlash(u32 address,u8 *buffer,u32 size)
{
	vu16 page,v1,v2;
	register u32 loopwrite ;
	vu16* buf = (vu16*)buffer ;

	page=gl_currentpage;
	while(address>=0x800000)
	{
		address-=0x800000;
		page+=0x1000;
	}
	SetRompage_MODE(page,SYSTEM_MODE_OS);

	//Chip_Reset();
	v1=0;v2=1;
	for(loopwrite=0;loopwrite<(size/2);loopwrite++)
	{
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xA0 ;
		*((vu16 *)(FlashBase_S98+address+loopwrite*2)) = buf[loopwrite];
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+address+loopwrite*2)) ;
			v2 = *((vu16 *)(FlashBase_S98+address+loopwrite*2)) ;
		}while(v1!=v2);
	}
	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);
}
//-----------------------------------------------------------
u32 Find_NOR_space(u32 needsize)
{
	u32 i,j;
	u32 page=0;
	u32 needpage;
	u32 start_addr=0;
	
	needpage = needsize >> 17;
	
	for(i=0;i<NOR_FAT_SIZE;i++){			
		if(NOR_FAT_buffer[i] == 0x00){
			page++;	
			if(page == needpage){
				start_addr = First_game_address + ((i+1-needpage)<<17);
				break;
			}		
	
		}
		else{
			page = 0;
		}
	}
	
	if(start_addr){
		for(j=i+1-needpage;j<i+1;j++ ){
			NOR_FAT_buffer[j] = 0xff;
		}	
		/* Commit only after the ROM has been written. */				
	}
		
	return start_addr;
}
//-----------------------------------------------------------
void Check_NOR_flash(void)
{
	Read_NORgame_info();	    	
	gl_norOffset = First_game_address;
	game_total_NOR = GetFileListFromNor();//initialize to prevent direct writes to NOR without page turning
	if(game_total_NOR==0)
	{
		memset(pNorFS,00,sizeof(FM_NOR_FS)*MAX_NOR);
		Save_NORgame_info((u16*)pNorFS,sizeof(FM_NOR_FS)*MAX_NOR);
		
		memset(NOR_FAT_buffer,0x0,NOR_FAT_SIZE);////init
		Save_NOR_FAT_info();
	}
	else{
		Read_NOR_FAT_info();
	}	
}

//-----------------------------------------------------------
u32 blue_nor_write_state_key;
#include "blue_nor_write_plan.h"

u32 IWRAM_CODE Loadfile2NOR(TCHAR *filename, u32 NORaddress, u16 have_patch, u8 SAVEMODE, u16 flag_64MBrom)
{
    (void)NORaddress;
    if(Read_S98NOR_ID() != NOR_ID || f_open(&gfile, filename, FA_READ) != FR_OK) return 1;

    u32 filesize = f_size(&gfile) <= 0x4000000 ? (u32)f_size(&gfile) : 0;
    u32 program_size, allocation_size;
    FM_NOR_FS entry = {0};
    UINT got;
    if((filesize > 0x2000000 && !flag_64MBrom) ||
       !Blue_NORWritePlan(filesize, have_patch, iTrimSize, &program_size, &allocation_size) ||
       f_lseek(&gfile, 0xa0) != FR_OK ||
       f_read(&gfile, entry.gamename, sizeof(entry.gamename), &got) != FR_OK ||
       got != sizeof(entry.gamename) || f_lseek(&gfile, 0) != FR_OK)
    {
        f_close(&gfile);
        return 1;
    }

    u32 address = Blue_PrepareNORWrite() ? Find_NOR_space(allocation_size) : 0;
    if(!address)
    {
        f_close(&gfile);
        return 2;
    }

    gl_norOffset = address;
    entry.rompage = address >> 17;
    entry.filesize = allocation_size;
    entry.have_patch = have_patch ? 2 : 0;
    entry.have_RTS = have_patch && gl_rts_on == 1 ? 2 : 0;
    entry.reserved2 = entry.have_RTS ? 0x4252 : 0;
    entry.is_64MBrom = flag_64MBrom;
    entry.savemode = SAVEMODE;
    snprintf(entry.filename, sizeof(entry.filename), "%s", filename);
    blue_nor_write_state_key = entry.have_RTS ? Blue_NORStateKey(&entry) : 0;

    Clear(60, 145, 120, 15, gl_color_cheat_black, 1);
    DrawHZText12(gl_writing, 0, 70, 145, gl_color_text, 1);
    Chip_Reset();
    for(u32 offset = 0; offset < program_size; offset += BLUE_NOR_SECTOR_SIZE)
    {
        char msg[32];
        snprintf(msg, sizeof(msg), "%lu/%lu", offset / BLUE_NOR_SECTOR_SIZE + 1,
                 program_size / BLUE_NOR_SECTOR_SIZE);
        Clear(124, 145, 100, 15, gl_color_cheat_black, 1);
        DrawHZText12(msg, 0, 124, 145, gl_color_text, 1);

        UINT want = offset < filesize ? filesize - offset : 0;
        if(want > BLUE_NOR_SECTOR_SIZE) want = BLUE_NOR_SECTOR_SIZE;
        memset(pReadCache, 0xff, BLUE_NOR_SECTOR_SIZE);
        if(want && (f_read(&gfile, pReadCache, want, &got) != FR_OK || got != want))
        {
            f_close(&gfile);
            return 1;
        }
        if(have_patch)
        {
            if(want) PatchInternal((u32*)pReadCache, BLUE_NOR_SECTOR_SIZE, offset);
            GBApatch_NOR((u32*)pReadCache, BLUE_NOR_SECTOR_SIZE, offset, entry.rompage);
        }
        else GBApatch_Cleanrom_NOR((u32*)pReadCache, offset);

        Block_Erase(address + offset);
        WriteFlash_with64word(address + offset, pReadCache, BLUE_NOR_SECTOR_SIZE);
    }
    if(f_close(&gfile) != FR_OK) return 1;

    pNorFS[game_total_NOR] = entry;
    Save_NORgame_info((u16*)pNorFS, sizeof(pNorFS));
    Save_NOR_FAT_info();
    return 0;
}
//-----------------------------------------------------------
void* IWRAM_CODE my_memcpy(void *destination, const void *source, u32 num)
{
    u8 *dest = destination;
    u8 *src  = (u8*)source;
    while (num) {
        *dest++ = *src++;
        num--;
    }
    return destination;
}

int IWRAM_CODE my_memcmp(void *ptr, const void *ptr2, u32 size)
{
  u8 *p1 = (u8 *)ptr;
  u8 *p2 = (u8 *)ptr2;
  u8 *p3 = (u8 *)ptr + size;

  while(p1 != p3) {
    if (*p1 != *p2) return 1;
    p1++;
    p2++;
  }
    return 0;
}

//-----------------------------------------------------------
u32 IWRAM_CODE GetFileListFromNor(void)
{
	//REG_IME = 0 ;
	u32 page=0x000 ,gamecount=0;
	u32 StartAddress = FlashBase_S98;
	//FM_NOR_FS tmpNorFS ;
	char temp[50];
	vu16  Value;
	u16 x24;
	
	FM_NOR_FS tmpNorFS ;
	
	char tempgame[50];
	
	u32 checkAddress;

	
	for(StartAddress = First_game_address; StartAddress< NOR_end_address;StartAddress+=0x20000){
		checkAddress = 	StartAddress;
		while(checkAddress>=0x800000)
		{
			checkAddress-=0x800000;
			page+=0x1000;
		}
		SetRompage_MODE(page,SYSTEM_MODE_OS);
		
		
		Value = *(vu16 *)(FlashBase_S98+checkAddress + 0xbe);//5F
		x24 = *(vu16 *)(FlashBase_S98+checkAddress + 0x6);//3	

		page=0x000;					
							
			if(	 ((Value&0xFF)==0xCE) || ((Value&0xFF)==0xCF)|| ((Value&0xFF)==0x00)|| (x24==0x51ae) ){
				if(*(vu8 *)(FlashBase_S98+checkAddress+0xb2) == 0x96)
				{
					for(int i=0;i<50;i++) temp[i]=0;
					my_memcpy(temp,(char*)(FlashBase_S98+checkAddress+0xa0),0x10);
																
					for(u32 index_i=0;index_i<MAX_NOR;index_i++){		
												
						if((my_memcmp(temp,pNorFS[index_i].gamename,0x10) ==0)  && (pNorFS[index_i].rompage == ((StartAddress)>>17)) )
						{
													
							gl_norOffset += pNorFS[index_i].filesize;							
														
							StartAddress -= 0x20000;		
							StartAddress += pNorFS[index_i].filesize;												
						
							
							if(index_i != gamecount){
								dmaCopy(&pNorFS[gamecount],&tmpNorFS, sizeof(FM_NOR_FS));
								dmaCopy(&pNorFS[index_i],&pNorFS[gamecount], sizeof(FM_NOR_FS));
								dmaCopy(&tmpNorFS,&pNorFS[index_i], sizeof(FM_NOR_FS));																	
							}							
							gamecount ++ ;
							break;
						}
					}
				}
			}					
		
	}
	
	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);
	Save_NORgame_info((u16*)pNorFS,sizeof(FM_NOR_FS)*MAX_NOR);	//here work

	//REG_IME   = 1 ;
	return gamecount ;
}
//-----------------------------------------------------------
