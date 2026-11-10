	.section .iwram, "ax", %progbits
	.align 2
	.thumb
	.thumb_func
	.global return_to_ds_style
	.type return_to_ds_style, %function

return_to_ds_style:
	@ Map Blue flash 0 in OS mode before returning to the kernel.
	ldr  r1, =0x04000208
	movs r0, #0
	strh r0, [r1]
	ldr  r1, =0x04000000
	strh r0, [r1]

	ldr  r0, =0x0000D200
	ldr  r1, =0x09FE0000
	strh r0, [r1]

	ldr  r0, =0x00001500
	ldr  r1, =0x08000000
	strh r0, [r1]

	ldr  r0, =0x0000D200
	ldr  r1, =0x08020000
	strh r0, [r1]

	ldr  r0, =0x00001500
	ldr  r1, =0x08040000
	strh r0, [r1]

	movs r0, #1
	ldr  r1, =0x09880000
	strh r0, [r1]

	ldr  r0, =0x00001500
	ldr  r1, =0x09FC0000
	strh r0, [r1]

	ldr  r1, =0x03007FFA
	movs r0, #0
	strb r0, [r1]
	movs r0, #8
	swi  0x01
	@ Restart the mapped kernel directly. A hard reset would replay the GBA BIOS.
	swi  0x00

1:	b 1b

	.ltorg
	.size return_to_ds_style, .-return_to_ds_style
