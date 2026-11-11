#ifndef EZ_DEFINE_HEADER
#define EZ_DEFINE_HEADER

#define MAX_pReadCache_size 0x20000
#define MAX_files     0x200
#define MAX_folder    0x60
#define MAX_NOR				0x40

#define MAX_path_len 0x100

#define FAT_table_size 0x400
//#define FAT_table_SAV_offset 0x200
#define FAT_table_RTS_offset 0x300

#define DEBUG

#define VideoBuffer    (u16*)0x6000000
#define Vcache         (u16*)pReadCache
#define RGB(r,g,b) ((r)+(g<<5)+(b<<10))

#define PSRAMBase_S98			(void*)0x08800000
#define FlashBase_S98 		(u32)0x09000000
#define FlashBase_S98_end (u32)0x09800000

#define SAVE_sram_base (u32)0x0E000000
#define SRAMSaver		   (u32)0x0E000000

#define UNBCD(x) (((x) & 0xF) + (((x) >> 4) * 10))
#define _BCD(x) ((((x) / 10)<<4) + ((x) % 10))
#define _YEAR	0
#define _MONTH	1
#define _DAY	2
#define _WKD	3
#define _HOUR	4
#define _MIN	5
#define _SEC	6

#define	_UnusedVram 		(void*)0x06012c00




#define NOR_ID         0x223D
#define First_game_address (u32)0x00800000
#define NOR_end_address (u32)0x08000000
#define SYSTEM_MODE_OS  1
#define SYSTEM_MODE_GAME 0
#define OP_LOAD  1
#define OP_SAVE  2
#define SAVE_info_offset 0x780000 //new
#define NOR_info_offset 0x7A0000
#define SET_info_offset 0x7C0000
#define NOR_FAT_offset 0x7E0000
#define  NOR_FAT_SIZE 960
#define LX16_FW_built_in_ver 3
#define LX16_newomega_top_bin_address (void*)0x08195000
#define LX16_newomega_top_bin_size  0x716a0
#define LX16_wirte_address 0x100000
#define LX16_FW_crc32 0x22DD74FA

#define SAVER_FOLDER "/SYSTEM/SAVER"

#define  DMA_COPY_MODE 0X1
#define  SET_PARAMETER_MODE  0x2







#define assress_language 0
#define assress_v_reset 1
#define assress_v_rts	2
#define assress_v_sleep 3
#define assress_v_cheat 4
#define assress_edit_sleephotkey_0 5
#define assress_edit_sleephotkey_1 6
#define assress_edit_sleephotkey_2 7
#define assress_edit_rtshotkey_0 8
#define assress_edit_rtshotkey_1 9
#define assress_edit_rtshotkey_2 10
#define assress_engine_sel  11
#define assress_show_Thumbnail 12
#define assress_ingame_RTC_open_status 13
#define assress_auto_save_sel 14
#define assress_ModeB_INIT 15
#define assress_led_open_sel 16
#define assress_Breathing_R 17
#define assress_Breathing_G 18
#define assress_Breathing_B 19
#define assress_SD_R 20
#define assress_SD_G 21
#define assress_SD_B 22
#define assress_toggle_reset 23
#define assress_toggle_backup 24

#define assress_saveMODE 64
#define assress_blue_boot_pref 65
#define BLUE_BOOT_PREF_TAG 0x4450u
#define assress_max 66






extern const unsigned char gImage_English_manual[9800]__attribute__((aligned(4)));
extern const unsigned char gImage_Chinese_manual[9800]__attribute__((aligned(4)));
extern const unsigned char gImage_HELP[76800]__attribute__((aligned(4)));
extern const unsigned char gImage_icons[1344]__attribute__((aligned(4)));
extern const unsigned char gImage_MENU[36960]__attribute__((aligned(4)));
extern const unsigned char gImage_SET[76800]__attribute__((aligned(4)));
extern const unsigned char gImage_START[76800]__attribute__((aligned(4)));

const unsigned char *Launcher_ImageHELP(void);
const unsigned char *Launcher_ImageMENU(void);
const unsigned char *Launcher_ImageSDList(void);
const unsigned char *Launcher_ImageSET(void);
const unsigned char *Launcher_ImageSTART(void);
const unsigned short *Launcher_ImageIconGBA(void);
const unsigned short *Launcher_ImageIconFolder(void);
const unsigned short *Launcher_ImageIconChip(void);

#ifndef LAUNCHER_THEME_ASSET_DEFINITIONS
#define gImage_HELP (Launcher_ImageHELP())
#define gImage_MENU (Launcher_ImageMENU())
#define gImage_NOR (Launcher_ImageSDList())
#define gImage_SD (Launcher_ImageSDList())
#define gImage_SET (Launcher_ImageSET())
#define gImage_START (Launcher_ImageSTART())
#define gImage_icon_gba (Launcher_ImageIconGBA())
#define gImage_icon_folder (Launcher_ImageIconFolder())
#define gImage_icon_chip (Launcher_ImageIconChip())
#endif

#endif
