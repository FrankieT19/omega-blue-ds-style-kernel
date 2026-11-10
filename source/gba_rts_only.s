@;--------------------------------------------------------------------

    .section .text.blue_game_patch,"ax",%progbits
		
	.global  RTS_only_ReplaceIRQ_start
	.global  RTS_only_ReplaceIRQ_end
	.global  RTS_only_Return_address_L
	.global  RTS_only_SAVE_key
	.global  RTS_only_LOAD_key

	.global  RTS_only_file_FAT
	.global  RTS_onlySecortPcluster
	.global	 RTS_only_Rompage_ingame
	
REG_BASE		= 0x4000000
REG_DISPCNT		= 0x00
REG_DISPSTAT	= 0x04
REG_VCOUNT		= 0x06
REG_BG0CNT		= 0x08
REG_BG1CNT		= 0x0A
REG_BG2CNT		= 0x0C
REG_BG3CNT		= 0x0E
REG_BG0HOFS		= 0x10
REG_BG0VOFS		= 0x12
REG_BG1HOFS		= 0x14
REG_BG1VOFS		= 0x16
REG_BG2HOFS		= 0x18
REG_BG2VOFS		= 0x1A
REG_BG3HOFS		= 0x1C
REG_BG3VOFS		= 0x1E
REG_WIN0H		= 0x40
REG_WIN1H		= 0x42
REG_WIN0V		= 0x44
REG_WIN1V		= 0x46
REG_WININ		= 0x48
REG_WINOUT		= 0x4A
REG_BLDCNT		= 0x50
REG_BLDALPHA	= 0x52
REG_BLDY		= 0x54
REG_SOUND1CNT_L	= 0x60
REG_SOUND1CNT_H	= 0x62
REG_SOUND1CNT_X	= 0x64
REG_SOUND2CNT_L	= 0x68
REG_SOUND2CNT_H	= 0x6C
REG_SOUND3CNT_L	= 0x70
REG_SOUND3CNT_H	= 0x72
REG_SOUND3CNT_X	= 0x74
REG_SOUND4CNT_L	= 0x78
REG_SOUND4CNT_H	= 0x7c
REG_SOUNDCNT_L		= 0x80
REG_SOUND2CNT_H		= 0x82
REG_SOUNDCNT_X		= 0x84
REG_SOUNDBIAS		= 0x88
REG_WAVE_RAM0_L		= 0x90
REG_FIFO_A_L	= 0xA0
REG_FIFO_A_H	= 0xA2
REG_FIFO_B_L	= 0xA4
REG_FIFO_B_H	= 0xA6
REG_DM0SAD		= 0xB0
REG_DM0DAD		= 0xB4
REG_DM0CNT_L	= 0xB8
REG_DM0CNT_H	= 0xBA
REG_DM1SAD		= 0xBC
REG_DM1DAD		= 0xC0
REG_DM1CNT_L	= 0xC4
REG_DM1CNT_H	= 0xC6
REG_DM2SAD		= 0xC8
REG_DM2DAD		= 0xCC
REG_DM2CNT_L	= 0xD0
REG_DM2CNT_H	= 0xD2
REG_DM3SAD		= 0xD4
REG_DM3DAD		= 0xD8
REG_DM3CNT_L	= 0xDC
REG_DM3CNT_H	= 0xDE
REG_TM0D		= 0x100
REG_TM0CNT		= 0x102
REG_IE			= 0x200
REG_IF			= 0x202
REG_P1			= 0x130
REG_P1CNT		= 0x132
REG_WAITCNT		= 0x204

ADDR_SetRampage = 0x9c00000
ADDR_DDR_flag  = 0x9720000
ADDR_SetPSRampage = 0x9860000
ADDR_SetRompage = 0x9880000
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
	.arm
RTS_only_ReplaceIRQ_start:
	MOV             R0, #0x4000000
	ADR             R1, RTS_irq
	STR             R1, [R0,#-4] @; 0x3FFFFFC = RTS_irq;
	LDR             R0, =0x12345678
	BX              R0
	.align
RTS_only_Return_address_L:
	.ltorg 			@;return address need modify
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RTS_irq:
	LDR			R1, [R0,#0x200]
	TST			R1, #0x10000
	TSTEQ		R1, #0x10000000
	LDREQ		PC, [R0,#-0xC]			@;old_interrupt_handler

	ldr 		r2,[r0,#REG_P1]
	bic 		r2,r2,#0xFF000000
	bic 		r2,r2,#0x00FF0000
	
check_save:
	adr 		r3,RTS_only_SAVE_key 
	ldr 		r3,[r3]
	cmp 		r2,r3		
	beq			call_Save	
check_load:
	adr 		r3,RTS_only_LOAD_key 
	ldr 		r3,[r3]
	cmp 		r2,r3 	
	beq 		call_Load	
	ldr 		pc,[r0,#-(0x04000000-0x03FFFFF4)] @;to normal IRQ routine								
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RTS_only_SAVE_key:
	.word 0xFB @;L+R+select
RTS_only_LOAD_key:
	.word 0xF7 @;L+R+Start
	
	
@;######################################################	
@;######################################################
.align
DDR_OP_code:
	stmfd	sp!,{r0-r7,lr}
	mov 	r0,#0x1  @; 打开DDR GAME MODE写 并切换到 后32MB的空间
	ldr   r1,=ADDR_DDR_flag
	bl		Set_ADDR_val

	mov 	r0,#0x400  @;open DDR mapper
	ldr   r1,=ADDR_SetRompage
	bl		Set_ADDR_val	
	
	cmp   r2,#0x2
	beq   is_saveRTS
	cmp		r2,#0x3
	beq		is_loadRTS	
@;##########################################	
is_saveRTS:  @; R2=2
	@;02000000-0203FFFF   WRAM - On-board Work RAM  (256 KBytes)
	ldr 	r0,=0x09020000  @;
	mov 	r1,#0x02000000
	@;add r1,#0x800
	ldr 	r2,=0x3f800    @;0x40000 	
	bl 		WriteSDDR_ingame
	
	@;03000000-03007FFF   WRAM - On-chip Work RAM   (32 KBytes)
	ldr r0,=0x09060000
	mov r1,#0x3000000
	mov r2,#0x8000
	bl 	WriteSDDR_ingame

	@;05000000-050003FF   BG/OBJ Palette RAM        (1 Kbyte)
	ldr r0,=0x09068000
	mov r1,#0x5000000
	mov r2,#0x400
	bl 	WriteSDDR_ingame	
	
	@;06000000-06017FFF   VRAM - Video RAM          (96 KBytes)
	ldr 	r0,=0x09069000
	mov 	r1,#0x6000000
	mov 	r2,#0x18000
	bl 		WriteSDDR_ingame
	
	@;07000000-070003FF   OAM - OBJ Attributes      (1 Kbyte)
	ldr r0,=0x09081000
	mov r1,#0x7000000
	mov r2,#0x400
	bl 	WriteSDDR_ingame
	
	@;R4-R11
	mrs	  r0,CPSR    @;Back up
	adrl	r7, spend_0x80
	ldr		r7,[r7]
	add		r7,#0x50  @;{r4-r11,sp,lr} SPSR  0x28+4
		
	mov		r1, #0xDF		@; Switch to systme Mode
	msr		cpsr_cf, r1
	NOP
	mov		r6,sp
	stmia r7!,{r6,lr}	
	
	msr 	cpsr_cf,r0	;@return IRQ mode	
	NOP
	
	ldr 	r0,=0x09081400
	adrl 	r1, spend_0x80
	ldr		r1,[r1]
	mov 	r2,#0x80
	bl 		WriteSDDR_ingame		
			
	@;04000000-040003FE   I/O Registers
	ldr r0,=0x09082000
	mov r1,#0x4000000
	mov r2,#0x210					@;0x0-0x60
	bl 	WriteSDDR_ingame
	
	
	@;FLAG	
	ldr r0,=0x09082FF0
	adrl r1,S_RTS_FLAG
	mov r2,#0x10
	bl 	WriteSDDR_ingame		
	
	@;ldmfd	sp!,{r6}
	@;mov		r7,#0x4000000
	@;strh 	r6,[r7] @;re Displaly Control
	@;b	is_restore_LCD	
	
	mov 	r0,#0x0001  @system mode 
	ldr   r1,=ADDR_SetRompage
	bl		Set_ADDR_val	
	
	mov 	r0,#0x6000  @; page =0x6000
	ldr   r1,=ADDR_SetPSRampage
	bl		Set_ADDR_val
	
	mov 	r0,#0x1 @;SD_Enable
	ldr   r1,=0x9400000
	bl		Set_ADDR_val	
	mov 	r0,#0x3 @;SD_Read_state
	ldr   r1,=0x9400000
	bl		Set_ADDR_val		

	mov		r5,#0
con_W_TF:
	ldr 	r0,=0x08820000  @;
	add		r0,r5
	mov 	r1,#0x09E00000 @;TF buff
	mov 	r2,#0x800 	
	bl 		ReadDDR_ingame
	
	@;lsr	 r0,r5,#9
	@;add  r0,#2 @start sec addr
	lsr	 r3,r5,#9
	
	adrl  r4,RTS_only_file_FAT
con_find:
	ldr		r6,[r4,#4] @;0   314c0
	ldr		r7,[r4,#8] @;40, 31540
	add  r4,#8
	cmp  r3,r7
	blo  get_sec 
	b 	con_find
get_sec:
	@;ldr r6,=0x314c0

	add r0,r6,r3
	
	mov  r1,#4 @blacks
	bl Write_SD_sectors
	bl Wait_SD_Response
	
	mov r7,#0x500
delay_loop:
	cmp			r7,#0
	beq			delay_end	
	nop
	sub 		r7,#0x01
	b				delay_loop
delay_end:	

	add r5,#0x800
	cmp r5,#0x70000
	bge END_W_TF
	b	con_W_TF
END_W_TF:
	
	mov 	r0,#0x0 @;SD_Disable
	ldr   r1,=0x9400000
	bl		Set_ADDR_val	
	
	
	b		DDR_OP_code_exit
	.ltorg
@;##########################################	
is_loadRTS:	

	@;02000000-0203FFFF   WRAM - On-board Work RAM  (256 KBytes)
	ldr 	r0,=0x09020000  @;
	mov 	r1,#0x02000000
	@;add 	r1,#0x800
	mov 	r2,#0x3f800 	
	bl 		ReadDDR_ingame
	
	@;03000000-03007FFF   WRAM - On-chip Work RAM   (32 KBytes)
	ldr r0,=0x09060000
	mov r1,#0x3000000
	mov r2,#0x8000
	bl 	ReadDDR_ingame

	@;05000000-050003FF   BG/OBJ Palette RAM        (1 Kbyte)
	ldr r0,=0x09068000
	mov r1,#0x5000000
	mov r2,#0x400
	bl 	ReadDDR_ingame	
	
	@;06000000-06017FFF   VRAM - Video RAM          (96 KBytes)
	ldr 	r0,=0x09069000
	mov 	r1,#0x6000000
	mov 	r2,#0x18000
	bl 		ReadDDR_ingame		
	

	@;07000000-070003FF   OAM - OBJ Attributes      (1 Kbyte)
	ldr r0,=0x09081000
	mov r1,#0x7000000
	mov r2,#0x400
	bl 	ReadDDR_ingame
	
	@;-------------------------------------
	mov r10,#0x4000000
	LDR r11,=0x09082000
	
	adr r9,register_list
register_list_loop:
	ldrh r2,[r9],#2
	cmp r2 ,#0xFF00
	beq register_list_end

	add r0,r10,r2   @;0x4000000  0x4000002 0x4000004
	add r1,r11,r2
	bl  restore2_IO
	b 	register_list_loop
register_list_end:


	@;ldr r0,=0x4000202   @;0x4000202
	@;mov r1,#0
	@;strh r1,[r0]


	ldr 	r0,=0x09081400
	adrl 	r1, spend_0x80 @;temp buff
	ldr		r1,[r1] 
	mov 	r2,#0x80
	bl 		ReadDDR_ingame	
	
	mrs	  r0,CPSR
	adrl	r7, spend_0x80
	ldr		r7,[r7] 
	add		r7,#0x28      @;SPSR offset
	
	ldmia	r7!,{r2}			@;r7=0x2C
	msr		SPSR_cxsf,r2	@;restore SPSR_irq  

	mov		r1, #0xDF		  @;Switch to systme Mode
	msr		cpsr_cf, r1
	NOP
	add		r7,#0x24					@;offset 0x50
	ldmia r7!,{r13-r14}

	msr 	cpsr_cf,r0	  @;restore IRQ	
	NOP

	bl 		Return_gamemode 		
	
	@;return 	
	mov 	r0,#0x00
	mov   r1,#ADDR_SetRampage
	bl		Set_ADDR_val
		
	BL 		Restore_
	
	adrl	r12, spend_0x80
	ldr		r12,[r12] 
	ldmia r12!,{r4-r11,sp,lr}
	mov 	r0,#0x04000000
	ldr 	pc,[r0,#-(0x04000000-0x03FFFFF4)] @;to normal IRQ routine		

	@;----------------------------------------------------------------------
	NOP	
	
DDR_OP_code_exit:	
	
	bl 		Return_gamemode 	
	ldmfd	sp!,{r0-r7,PC}
	.ltorg
@;------------------------------------------------------
Return_gamemode:
	stmfd	sp!,{r0-r1,lr}
	adr		r0,RTS_only_Rompage_ingame
	ldr 	r0,[r0]      @;Rompage  return to game mode 
	ldr   r1,=ADDR_SetRompage
	bl		Set_ADDR_val	

	mov 	r0,#0x0  @; 关闭DDR GAME MODE写
	ldr   r1,=ADDR_DDR_flag
	bl		Set_ADDR_val
	ldmfd	sp!,{r0-r1,PC}
@;------------------------------------------------------
Set_ADDR_val:
	stmfd	sp!,{r0-r7}
	ldr 	r2,=0xD200
	ldr 	r3,=0x1500
	ldr 	r4,=0x9fe0000
	strh 	r2,[r4]
	mov 	r4,#0x8000000
	strh  r3,[r4]
	ldr 	r4,=0x8020000
	strh 	r2,[r4]
	ldr 	r4,=0x8040000
	strh  r3,[r4]
	@;ldr 	r4,=0x9720000
	strh  r0,[r1]
	ldr 	r4,=0x9FC0000
	strh  r3,[r4]
	ldmfd	sp!,{r0-r7}
	bx		lr	
@;------------------------------------------------------	
Write_SD_sectors:
	stmfd	sp!,{r0-r7}
	ldr 	r2,=0xD200
	ldr 	r3,=0x1500
	ldr 	r4,=0x9fe0000  @;0xD200
	strh 	r2,[r4]
	mov 	r4,#0x8000000  @;0x1500
	strh  r3,[r4]
	ldr 	r4,=0x8020000  @;0xD200
	strh 	r2,[r4]
	ldr 	r4,=0x8040000  @;0x1500
	strh  r3,[r4]
	
	ldr 	r4,=0x9600000  @;l
	strh  r0,[r4]
	lsr   r0,#16
	ldr 	r4,=0x9620000  @;h
	strh  r0,[r4]
	
	add r1,#0x8000
	ldr 	r4,=0x9640000  @;blacks
	strh  r1,[r4]
	
	ldr 	r4,=0x9FC0000  @0x1500
	strh  r3,[r4]
	ldmfd	sp!,{r0-r7}
	bx		lr	
@;------------------------------------------------------	
Wait_SD_Response:
    LDR     R1, =0x9E00000
    ldr 		r2,=0xEEE1
LOOP_WAIT:
    LDRH    R0, [R1]
    CMP     R0, r2
    BNE     EXIT_RET
    B       LOOP_WAIT
EXIT_RET:
    MOV     R0, #0
    BX      LR

		.ltorg
@;------------------------------------------------------
@;void WriteSDDR(u32 address, u16* data, u32 size)
    .arm
WriteSDDR_ingame:
		stmfd	 sp!,{r0-r5}
    lsr 	r2, r2, #1       
    MOV   r3, #0           
loop_start:
    cmp   r3, r2           
    bge   loop_end         

    lsl 	r5,r3,#1
    ldrh  r4, [r1, r5]  
    strh    r4, [r0, r5]
    add     r3, r3, #1       
    b       loop_start
loop_end:
		ldmfd	sp!,{r0-r5}
    bx      lr               
@;------------------------------------------------------
ReadDDR_ingame:
		stmfd	 sp!,{r0-r5}
    lsr	r2, r2, #2       
    mov	r3, #0           

loop_rd:
    cmp	r3, r2
    bge	rd_end

    lsl	r5,r3,#2
    ldr	r4, [r0, r5]
    str	r4, [r1, r5]

    add	r3, r3, #1
    b		loop_rd
rd_end:
		ldmfd	sp!,{r0-r5}
		bx	lr
@;------------------------------------------------------
ReadSram: @;(u32 address, u8 *data, u32 size)
	stmfd	 sp!,{r0-r5}
	ADDS    R2, R2, R0
	SUBS    R1, R1, R0
rSram_loop1:
	CMP     R0, R2
	BNE     rSram_loop
	ldmfd	sp!,{r0-r5}
	BX      LR
rSram_loop:
	LDRB    R3, [R0]
	LSL 		R4,R3,#0
	LDRB    R3, [R0,#1]
	LSL 		R5,R3,#8
	ORR			R4,R5
	LDRB    R3, [R0,#2]
	LSL 		R5,R3,#16
	ORR			R4,R5
	LDRB    R3, [R0,#3]
	LSL 		R5,R3,#24
	ORR			R4,R5
	STR     R4, [R1,R0]
	ADDS    R0, #4
	B       rSram_loop1
@;--------------------
restore2_IO:        @; IOaddress, offset
	LDRB    R3, [R1]
	LSL 		R4,R3,#0
	LDRB    R3, [R1,#1]
	LSL 		R5,R3,#8
	ORR			R4,R5
	STRH    R4, [R0]
	bx lr
@;===================================================			
Restore_:
	adrl		R3, spend_0x80
	ldr			R3,[R3]
	add 		R3,#0x30
	
	LDR			R0, =0x4000100
	
	LDRH		R1, [R3,#2]
	STRH		R1, [R0,#2]@;v4000102
	
	LDRH		R1, [R3,#4]
	STRH		R1, [R0,#6]@;v4000106;
	
	LDRH		R1, [R3,#6]
	STRH		R1, [R0,#0xA]@;v400010A;
	
	LDRH		R1, [R3,#8]
	STRH		R1, [R0,#0xE]@;v400010E;
	
	LDR			R0, =0x4000200
	LDR			R2, =0x0
	STRH		R2, [R0,#0x2]@;v4000202;
	LDRH		R1, [R3,#0]
	STRH		R1, [R0,#0x8]@;v4000208;	 IME
	MOV			PC, LR
@;--------------------	
	.ltorg
	.align
spend_0x80:
	.word 0x0203FE00   @; default
RTS_only_Rompage_ingame:
	.word 0x400   @; default
RTS_only_file_FAT:
	.space 0x40
RTS_onlySecortPcluster:
	.word 0x40   @; default
	.align
register_list:
	.hword 0x0000,0x0002,0x0004,0x0008,0x000A,0x000C,0x000E,0x0048
	.hword 0x004A,0x0050,0x0052
	.hword 0x0084
	.hword 0x0060,0x0062,0x0068,0x0070,0x0072,0x0078
	.hword 0x0080,0x0082,0x0088                 @;0x008C,0x008E,
	.hword 0x0090,0x0092,0x0094,0x0096,0x0098,0x009A,0x009C,0x009E
	
	.hword 0x00B8,0x00C4,0x00D0,0x00DC  @;DMA
	.hword 0x0120,0x0122,0x0124,0x0126,0x0128,0x012A,0x012C,0x0132,0x0134
	.hword 0x0140,0x0150,0x0154,0x0200,0x0204,0x0208,0xFF00
	.align	
S_RTS_FLAG:
 	.byte  'E','Z','-','O','m','e','g','a','R','T','C','F','I','L','E','.'

DDR_OP_code_end:
@;######################################################	
@;######################################################	
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.arm
@;------------------------------------------------------
@;SetRampage:
@;	ldr 	r1,=0xD200
@;	ldr 	r2,=0x1500
@;	ldr 	r3,=0x9fe0000
@;	strh 	r1,[r3]
@;	mov 	r3,#0x8000000
@;	strh  r2,[r3]
@;	ldr 	r3,=0x8020000
@;	strh 	r1,[r3]
@;	ldr 	r3,=0x8040000
@;	strh  r2,[r3]
@;	ldr 	r3,=0x9C00000
@;	strh  r0,[r3]
@;	ldr 	r3,=0x9FC0000
@;	strh  r2,[r3]
@;	bx		lr
@;------------------------------------------------------
WriteSram: @;(u32 address, u8 *data, u32 size)
	ADD 		R2,R2,R0
	SUB 		R1,R1,R0
wSram_loop1:
	CMP     R0, R2
	BNE     wSram_loop
	BX      LR
wSram_loop:
	LDR     R3, [R1,R0]
	STRB    R3, [R0],#1
	LSR     R3, R3, #0x8
	STRB    R3, [R0],#1
	LSR     R3, R3, #0x8
	STRB    R3, [R0],#1
	LSR     R3, R3, #0x8
	STRB    R3, [R0],#1	
	B       wSram_loop1
@;------------------------------------------------------
@;------------------------------------------------------
@;------------------------------------------------------
	.ltorg	
@;------------------------------------------------------
copy_code:
	stmfd	 sp!,{r0-r5,lr}
	mov 	r0,#0x20
	mov   r1,#ADDR_SetRampage
	bl		Set_ADDR_val
	
	mov r0,#0x0E000000
	ldr r1,=0x0203f800 @;0x3f800
	mov r2,#0x800
	bl 	WriteSram
	
@;----copy DDR OP code  to 0x02000000
	adrl r1,DDR_OP_code
	adrl r3,DDR_OP_code_end
	ldr r2,=0x0203f800 @;0x3f800

copyddr_loop:
	ldr r0,[r1],#4
	str r0,[r2],#4
	cmp r1,r3
	blt copyddr_loop
	
	ldmfd	sp!,{r0-r5,pc}
@;------------------------------------------------------
call_Save:	
	@;adrl	r7,RTS_switch
	@;ldr		r7,[r7]
	@;cmp 	r7,#1
	@;bne 	errorRTS
	adrl		r2, spend_0x80
	ldr			r2,[r2]
	stmia		r2!,{r4-r11,sp,lr} @;0x0
	mrs	 		r3,SPSR
	stmia		r2!,{r3}      			@;0x28
	
	stmfd		sp!,{r0-r10,lr}
	bl			BAK_	

	bl 		copy_code
	ldr r0,=0x0203f800 @;0x3f800
	mov r2,#0x2  @;is saveRTS
	MOV LR ,PC
	bx r0	
	
save_exit:
	mov r0,#0x0E000000
	ldr r1,=0x0203f800
	mov r2,#0x800
	bl 	ReadSram

	mov 	r0,#0x00
	mov   r1,#ADDR_SetRampage
	bl		Set_ADDR_val

	BL 		Restore_	
	ldmfd	sp!,{r0-r10,lr}
	@;mov 	r0,#0x04000000
	ldr 	pc,[r0,#-(0x04000000-0x03FFFFF4)] @;to normal IRQ routine	
@;===================================================	
BAK_:	
	adrl		R2, spend_0x80
	ldr			R2,[R2]
	add 		R2,#0x30
	LDR			R0, =0x4000200
	MOV			R1, #0
	
	LDRH		R3, [R0,#8]
	STRH		R3, [R2,#0]
	STRH		R1, [R0,#8]@;v4000208 = 0;
		
	LDR			R0, =0x4000100
	LDRH		R3, [R0,#2]
	STRH		R3, [R2,#2]
	STRH		R1, [R0,#2]@;v4000102 = 0;
	
	LDRH		R3, [R0,#6]
	STRH		R3, [R2,#4]
	STRH    R1, [R0,#6]@;v4000106 = 0;
	
	LDRH    R3, [R0,#0xA]
	STRH		R3, [R2,#6]
	STRH    R1, [R0,#0xA]@;v400010A = 0;
	
	LDRH    R3, [R0,#0xE]
	STRH		R3, [R2,#8]
	STRH    R1, [R0,#0xE]@;v400010E = 0;
	MOV			PC, LR
@;===================================================	
call_Load:	
	@;check 	;FLAG	
	stmfd		sp!,{r0-r3,lr}	
	
@;	mov 	r0,#0xA0
@;	bl 		SetRampage
@;	ldr 	r0,=0x0E00FFF0
@;	adrl 	r1, spend_0x80  @;temp buff
@;	ldr 	r1,[r1]
@;	mov 	r2,#0x10
@;	bl 		ReadSram	
	
@;	adrl 	r1,S_RTS_FLAG	
@;	adrl 	r2, spend_0x80 @;temp buff
@;	ldr 	r2,[r2]
@;	mov 	r3,#0
@;loop_check:
@;	ldr 	r4,[r1],#4
@;	ldr 	r5,[r2],#4
@;	cmp 	r4,r5
@;	bne 	errorRTS
@;	add 	r3,#1
@;	cmp 	r3,#4
@;	bne 	loop_check
	b 		checkOK
@;errorRTS:
@;	mov 	r0,#0x0
@;	bl 		SetRampage		
@;	ldmfd		sp!,{r0-r3,lr}
@;	ldr 		pc,[r0,#-(0x04000000-0x03FFFFF4)] @;to normal IRQ routine		
@;===================================================	
@;===================================================	
checkOK:
	ldr r0,=0x4000208   @;0x4000208
	mov r1,#0
	strh r1,[r0]

	bl 		copy_code
	ldr r0,=0x0203f800
	mov r2,#0x3  @;is call_Load RTS
	MOV LR ,PC
	bx r0
	NOP 
	@;mov 	r0,#0x0
	@;bl 		SetRampage
		
	@;BL 		Restore_	
	
	@;adrl	r12, spend_0x80
	@;ldr		r12,[r12] 
	@;ldmia r12!,{r4-r11,sp,lr}
	@;mov 	r0,#0x04000000
	@;ldr 	pc,[r0,#-(0x04000000-0x03FFFFF4)] @;to normal IRQ routine		

	@;===================================================		
	.ltorg	
	@;===================================================		
	.align	
RTS_only_ReplaceIRQ_end:
   .end
