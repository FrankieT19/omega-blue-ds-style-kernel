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
u32 IWRAM_CODE Loadfile2NOR(TCHAR *filename, u32 NORaddress,u16 have_patch,u8 SAVEMODE,u16 flag_64MBrom)
{
	u32 res;
	u32 ret;
	u32 filesize;
	u32 fileneedsize;
	
	u32 blocknum;
	char msg[128];
	FM_NOR_FS tmpNorFS = {0};
	char temp[50];
	char flag[4];
	//u16 flag_64MBrom=0;
	
	//u16 readdata;
	u32 add_patch = 0;
			
	u32 Real_block_addr;
	
	u16 norid = Read_S98NOR_ID();
	
	if(norid == NOR_ID)//S98
	{	
		res = f_open(&gfile, filename, FA_READ);
		if(res != FR_OK) return 1;
		
		filesize = f_size(&gfile);
        if(filesize < 0xc0 || filesize > 0x4000000 ||
           (filesize > 0x2000000 && (!flag_64MBrom || have_patch)))
        {
            f_close(&gfile);
            return 1;
        }		
		f_lseek(&gfile, 0xa0);		
		if(f_read(&gfile,temp,0x10,(UINT*)&ret) != FR_OK || ret != 0x10)
        {
            f_close(&gfile);
            return 1;
        }
		f_lseek(&gfile, 0xB0);		
		f_read(&gfile, flag, 0x4, (UINT *)&ret);//read flag
		


		memcpy(tmpNorFS.gamename,temp,0x10);
		//tmpNorFS.rompage = NORaddress >> 17;
		
		fileneedsize = ((((filesize+0x3FFFF)/0x40000)*0x40000));
		if(have_patch)
		{			
			if(iTrimSize>=fileneedsize)
			{
				fileneedsize = fileneedsize+0x40000;
				add_patch = 1;
			}
		}		

		
		if(!Blue_PrepareNORWrite())
        {
            f_close(&gfile);
            return 2;
        }
		u32 Write2Addr = Find_NOR_space(fileneedsize);
		if(Write2Addr==0){f_close(&gfile);return 2;}

		gl_norOffset = Write2Addr;	
			
		tmpNorFS.rompage = Write2Addr >> 17;
				
		tmpNorFS.filesize = fileneedsize;
		tmpNorFS.have_patch = have_patch ? 2 : 0;
		tmpNorFS.have_RTS = gl_rts_on;
		tmpNorFS.is_64MBrom = flag_64MBrom;
		tmpNorFS.savemode = SAVEMODE;
		
		snprintf(tmpNorFS.filename,sizeof(tmpNorFS.filename),"%s",filename);
		dmaCopy(&tmpNorFS,&pNorFS[game_total_NOR], sizeof(FM_NOR_FS));
 
		Clear(60,160-15,120,15,gl_color_cheat_black,1);	
		DrawHZText12(gl_writing,0,70,160-15,gl_color_text,1);	
		
		Chip_Reset();	

		f_lseek(&gfile, 00);
		for(blocknum=0;blocknum<filesize;blocknum+=0x20000)
		{		
			//Real_block_addr = Get_map_addr(blocknum);
			
			sprintf(msg,"%luMb/%luMb",(blocknum)/0x20000,filesize/0x20000);
			Clear(70+54,160-15,100,15,gl_color_cheat_black,1);
			DrawHZText12(msg,0,70+54,160-15,gl_color_text,1);
			
			//Block_Erase(blocknum+NORaddress);

			//f_lseek(&gfile, blocknum);
			UINT wanted = filesize - blocknum < 0x20000 ? filesize - blocknum : 0x20000;
            memset(pReadCache,0xff,0x20000);
            if(f_read(&gfile,pReadCache,wanted,(UINT*)&ret) != FR_OK || ret != wanted)
            {
                f_close(&gfile);
                return 1;
            }
			if(have_patch){
				if((gl_reset_on==1) || (gl_rts_on==1) || (gl_sleep_on==1) || (gl_cheat_on==1))		    
				{
					PatchInternal((u32*)pReadCache,0x20000,blocknum);	
					GBApatch_NOR((u32*)pReadCache,0x20000,blocknum,tmpNorFS.rompage);//some nes need check				
				}
			}
			else{
				GBApatch_Cleanrom_NOR((u32*)pReadCache,blocknum);
			}

			Block_Erase(blocknum + Write2Addr);		
			WriteFlash_with64word(blocknum + Write2Addr,pReadCache,0x20000);

		}
		f_close(&gfile);
		
		if(have_patch)
		{
			if(add_patch)
			{
				Block_Erase(blocknum+Write2Addr);
				GBApatch_NOR((u32*)pReadCache,0x20000,blocknum,tmpNorFS.rompage);
				WriteFlash_with64word(blocknum+Write2Addr,pReadCache,0x20000);
			}
		}
		
		Save_NORgame_info((u16*)pNorFS,sizeof(pNorFS));
        Save_NOR_FAT_info();
		//wait_btn();	
		return 0;
	}		
	else
	{
		#ifdef DEBUG
			//DEBUG_printf("Bad NOR ID");
		#endif
		return 1;
	}
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
