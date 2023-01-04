/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*	"@(#)inst.h	1.2	(ULTRIX/OSF)	1/8/92"	*/
/* --------------------------------------------------- */
/* | Copyright (c) 1986 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                            | */
/* --------------------------------------------------- */

#ifndef	_ALPHA_INST_H_
#define	_ALPHA_INST_H_	

/*
 * inst.h -- instruction format defines
 */

#ifdef __LANGUAGE_C__
union alpha_instruction {
	unsigned word;
	unsigned char byte[4];

	/* common fields to all instructions */
	struct {
		unsigned : 26;
		unsigned opcode : 6;
	} common;

	/* memory format */
	struct {
		signed memory_displacement : 16;
		unsigned rb : 5;
		unsigned ra : 5;
		unsigned opcode : 6;
	} m_format;

	/* branch format */
	struct {
		signed branch_displacement : 21;
		unsigned ra : 5;
		unsigned opcode : 6;
	} b_format;

	/* operate format */
	struct {
		unsigned rc : 5;
		unsigned function : 7;
		unsigned form : 1;
		unsigned : 3;
		unsigned rb : 5;
		unsigned ra : 5;
		unsigned opcode : 6;
	} o_format;

	/* literal (operate) format */
	struct {
		unsigned rc : 5;
		unsigned function : 7;
		unsigned form : 1;
		unsigned literal : 8;
		unsigned ra : 5;
		unsigned opcode : 6;
	} l_format;

	/* floating point format */
	struct {
		unsigned fc: 5;
		unsigned function : 11;
		unsigned fb : 5;
		unsigned fa : 5;
		unsigned opcode : 6;
	} f_format;

	/* pal format */
	struct {
		unsigned function : 26;
		unsigned opcode : 6;
	} pal_format;

	/* jsr (memory) format */
	struct {
		signed hint : 14;
		unsigned function : 2;
		unsigned rb : 5;
		unsigned ra : 5;
		unsigned opcode : 6;
	} j_format;
};
#endif /* __LANGUAGE_C__ */

/*
 * The following are the major instruction opcodes which are contained
 * in the 6-bit 'opcode' field above. The opcodes which are commented as
 * being a "group" have sub-opcodes which are defined later.
 */
#define op_call_pal	0x00		/* pal group; see pal.h functions */
#define op_opc01	0x01
#define op_opc02	0x02
#define op_opc03	0x03
#define op_opc04	0x04
#define op_opc05	0x05
#define op_opc06	0x06
#define op_opc07	0x07
#define op_lda		0x08
#define op_ldah		0x09
#define op_opc0a	0x0a
#define op_ldq_u	0x0b
#define op_opc0c	0x0c
#define op_opc0d	0x0d
#define op_opc0e	0x0e
#define op_stq_u	0x0f
#define op_inta		0x10		/* integer arithmetic group */
#define op_intl		0x11		/* integer logical group */
#define op_ints		0x12		/* integer shift group */
#define op_intm		0x13		/* integer multiply group */
#define op_opc14	0x14
#define op_fltv		0x15		/* vax floating point group */
#define op_flti		0x16		/* ieee floating point group */
#define op_fltl		0x17		/* datatype independent FP group */
#define op_misc		0x18		/* miscellenous group */
#define op_pal19	0x19
#define op_jsr		0x1a		/* jsr group */
#define op_pal1b	0x1b
#define op_opc1c	0x1c
#define op_pal1d	0x1d
#define op_pal1e	0x1e
#define op_pal1f	0x1f
#define op_ldf		0x20
#define op_ldg		0x21
#define op_lds		0x22
#define op_ldt		0x23
#define op_stf		0x24
#define op_stg		0x25
#define op_sts		0x26
#define op_stt		0x27
#define op_ldl		0x28
#define op_ldq		0x29
#define op_ldl_l	0x2a
#define op_ldq_l	0x2b
#define op_stl		0x2c
#define op_stq		0x2d
#define op_stl_c	0x2e
#define op_stq_c	0x2f
#define op_br		0x30
#define op_fbeq		0x31
#define op_fblt		0x32
#define op_fble		0x33
#define op_bsr		0x34
#define op_fbne		0x35
#define op_fbge		0x36
#define op_fbgt		0x37
#define op_blbc		0x38
#define op_beq		0x39
#define op_blt		0x3a
#define op_ble		0x3b
#define op_blbs		0x3c
#define op_bne		0x3d
#define op_bge		0x3e
#define op_bgt		0x3f

/*
 * op_inta: integer arithmetic group. These instructions are operate format
 * (o_format or l_format) instructions with the function code encoded in
 * the 7-bit 'function' field.
 */
#define inta_addl	0x00
#define inta_s4addl	0x02
#define inta_subl	0x09
#define inta_s4subl	0x0b
#define inta_cmpbge	0x0f
#define inta_s8addl	0x12
#define inta_s8subl	0x1b
#define inta_cmpult	0x1d
#define inta_addq	0x20
#define inta_s4addq	0x22
#define inta_subq	0x29
#define inta_s4subq	0x2b
#define inta_cmpeq	0x2d
#define inta_s8addq	0x32
#define inta_s8subq	0x3b
#define inta_cmpule	0x3d
#define inta_addlv	0x40
#define inta_sublv	0x49
#define inta_cmplt	0x4d
#define inta_addqv	0x60
#define inta_subqv	0x69
#define inta_cmple	0x6d

/*
 * op_intl: integer logical group. These instructions are operate format
 * (o_format or l_format) instructions with the function code encoded in
 * the 7-bit 'function' field.
 */
#define intl_and	0x00
#define intl_bic	0x08
#define intl_cmovlbs	0x14
#define intl_cmovlbc	0x16
#define intl_bis	0x20
#define intl_cmoveq	0x24
#define intl_cmovne	0x26
#define intl_ornot	0x28
#define intl_xor	0x40
#define intl_cmovlt	0x44
#define intl_cmovge	0x46
#define intl_eqv	0x48
#define intl_cmovle	0x64
#define intl_cmovgt	0x66

/*
 * op_ints: integer shift group. These instructions are operate format
 * (o_format or l_format) instructions with the function code encoded in
 * the 7-bit 'function' field.
 */
#define ints_mskbl	0x02
#define ints_extbl	0x06
#define ints_insbl	0x0b
#define ints_mskwl	0x12
#define ints_extwl	0x16
#define ints_inswl	0x1b
#define ints_mskll	0x22
#define ints_extll	0x26
#define ints_insll	0x2b
#define ints_zap	0x30
#define ints_zapnot	0x31
#define ints_mskql	0x32
#define ints_srl	0x34
#define ints_extql	0x36
#define ints_sll	0x39
#define ints_insql	0x3b
#define ints_sra	0x3c
#define ints_mskwh	0x52
#define ints_inswh	0x57
#define ints_extwh	0x5a
#define ints_msklh	0x62
#define ints_inslh	0x67
#define ints_extlh	0x6a
#define ints_mskqh	0x72
#define ints_insqh	0x77
#define ints_extqh	0x7a

/*
 * op_intm: integer multiply group. These instructions are operate format
 * (o_format or l_format) instructions with the function code encoded in
 * the 7-bit 'function' field.
 */
#define intm_mull	0x00
#define intm_mulq	0x20
#define intm_umulh	0x30
#define intm_mullv	0x40
#define intm_mulqv	0x60

/*
 * op_jsr: jsr group. These instructions are memory format (j_format)
 * instructions with the function code encoded in the 2-bit 'function'
 * field.
 */
#define jsr_jmp		0x0
#define jsr_jsr		0x1
#define jsr_ret		0x2
#define jsr_jsr_coroutine 0x3

/*
 * op_misc: miscellenous group. These instructions are memory format
 * (m_format) instructions with the function code encoded in the 16-bit
 * displacement field
 */
#define misc_trapb	0x0000
#define misc_excb	0x0400
#define misc_mb		0x4000
#define misc_wmb	0x4400
#define misc_fetch	0x8000
#define misc_fetch_m	0xa000
#define misc_rpcc	0xc000
#define misc_rc		0xe000
#define misc_rs		0xf000

/*
 * op_fltl: datatype independent floating point group. These instructions
 * are floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
#define fltl_cvtlq	0x010
#define fltl_cpys	0x020
#define fltl_cpysn	0x021
#define fltl_cpyse	0x022
#define fltl_mt_fpcr	0x024
#define fltl_mf_fpcr	0x025
#define fltl_fcmoveq	0x02a
#define fltl_fcmovne	0x02b
#define fltl_fcmovlt	0x02c
#define fltl_fcmovge	0x02d
#define fltl_fcmovle	0x02e
#define fltl_fcmovgt	0x02f
#define fltl_cvtql	0x030
#define fltl_cvtqlv	0x130
#define fltl_cvtqlsv	0x530

/*
 * op_flti: ieee floating point group. These instructions
 * are floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
#define flti_addsc	0x000
#define flti_subsc	0x001
#define flti_mulsc	0x002
#define flti_divsc	0x003
#define flti_addtc	0x020
#define flti_subtc	0x021
#define flti_multc	0x022
#define flti_divtc	0x023
#define flti_cvttsc	0x02c
#define flti_cvttqc	0x02f
#define flti_cvtqsc	0x03c
#define flti_cvtqtc	0x03e
#define flti_addsm	0x040
#define flti_subsm	0x041
#define flti_mulsm	0x042
#define flti_divsm	0x043
#define flti_addtm	0x060
#define flti_subtm	0x061
#define flti_multm	0x062
#define flti_divtm	0x063
#define flti_cvttsm	0x06c
#define flti_cvttqm	0x06f
#define flti_cvtqsm	0x07c
#define flti_cvtqtm	0x07e
#define flti_adds	0x080
#define flti_subs	0x081
#define flti_muls	0x082
#define flti_divs	0x083
#define flti_addt	0x0a0
#define flti_subt	0x0a1
#define flti_mult	0x0a2
#define flti_divt	0x0a3
#define flti_cmptun	0x0a4
#define flti_cmpteq	0x0a5
#define flti_cmptlt	0x0a6
#define flti_cmptle	0x0a7
#define flti_cvtts	0x0ac
#define flti_cvttq	0x0af
#define flti_cvtqs	0x0bc
#define flti_cvtqt	0x0be
#define flti_addsd	0x0c0
#define flti_subsd	0x0c1
#define flti_mulsd	0x0c2
#define flti_divsd	0x0c3
#define flti_addtd	0x0e0
#define flti_subtd	0x0e1
#define flti_multd	0x0e2
#define flti_divtd	0x0e3
#define flti_cvttsd	0x0ec
#define flti_cvttqd	0x0ef
#define flti_cvtqsd	0x0fc
#define flti_cvtqtd	0x0fe
#define flti_addsuc	0x100
#define flti_subsuc	0x101
#define flti_mulsuc	0x102
#define flti_divsuc	0x103
#define flti_addtuc	0x120
#define flti_subtuc	0x121
#define flti_multuc	0x122
#define flti_divtuc	0x123
#define flti_cvttsuc	0x12c
#define flti_cvttqvc	0x12f
#define flti_addsum	0x140
#define flti_subsum	0x141
#define flti_mulsum	0x142
#define flti_divsum	0x143
#define flti_addtum	0x160
#define flti_subtum	0x161
#define flti_multum	0x162
#define flti_divtum	0x163
#define flti_cvttsum	0x16c
#define flti_cvttqvm	0x16f
#define flti_addsu	0x180
#define flti_subsu	0x181
#define flti_mulsu	0x182
#define flti_divsu	0x183
#define flti_addtu	0x1a0
#define flti_subtu	0x1a1
#define flti_multu	0x1a2
#define flti_divtu	0x1a3
#define flti_cvttsu	0x1ac
#define flti_cvttqv	0x1af
#define flti_addsud	0x1c0
#define flti_subsud	0x1c1
#define flti_mulsud	0x1c2
#define flti_divsud	0x1c3
#define flti_addtud	0x1e0
#define flti_subtud	0x1e1
#define flti_multud	0x1e2
#define flti_divtud	0x1e3
#define flti_cvttsud	0x1ec
#define flti_cvttqvd	0x1ef
#define flti_cvtst	0x2ac
#define flti_cvtsts	0x6ac
#define flti_addssuc	0x500
#define flti_subssuc	0x501
#define flti_mulssuc	0x502
#define flti_divssuc	0x503
#define flti_addtsuc	0x520
#define flti_subtsuc	0x521
#define flti_multsuc	0x522
#define flti_divtsuc	0x523
#define flti_cvttssuc	0x52c
#define flti_cvttqsvc	0x52f
#define flti_addssum	0x540
#define flti_subssum	0x541
#define flti_mulssum	0x542
#define flti_divssum	0x543
#define flti_addtsum	0x560
#define flti_subtsum	0x561
#define flti_multsum	0x562
#define flti_divtsum	0x563
#define flti_cvttssum	0x56c
#define flti_cvttqsvm	0x56f
#define flti_addssu	0x580
#define flti_subssu	0x581
#define flti_mulssu	0x582
#define flti_divssu	0x583
#define flti_addtsu	0x5a0
#define flti_subtsu	0x5a1
#define flti_multsu	0x5a2
#define flti_divtsu	0x5a3
#define flti_cmptunsu	0x5a4
#define flti_cmpteqsu	0x5a5
#define flti_cmptltsu	0x5a6
#define flti_cmptlesu	0x5a7
#define flti_cvttssu	0x5ac
#define flti_cvttqsv	0x5af
#define flti_addssud	0x5c0
#define flti_subssud	0x5c1
#define flti_mulssud	0x5c2
#define flti_divssud	0x5c3
#define flti_addtsud	0x5e0
#define flti_subtsud	0x5e1
#define flti_multsud	0x5e2
#define flti_divtsud	0x5e3
#define flti_cvttssud	0x5ec
#define flti_cvttqsvd	0x5ef
#define flti_cvttss	0x6ac
#define flti_addssuic	0x700
#define flti_subssuic	0x701
#define flti_mulssuic	0x702
#define flti_divssuic	0x703
#define flti_addtsuic	0x720
#define flti_subtsuic	0x721
#define flti_multsuic	0x722
#define flti_divtsuic	0x723
#define flti_cvttssuic	0x72c
#define flti_cvttqsvic	0x72f
#define flti_cvtqssuic	0x73c
#define flti_cvtqtsuic	0x73e
#define flti_addssuim	0x740
#define flti_subssuim	0x741
#define flti_mulssuim	0x742
#define flti_divssuim	0x743
#define flti_addtsuim	0x760
#define flti_subtsuim	0x761
#define flti_multsuim	0x762
#define flti_divtsuim	0x763
#define flti_cvttssuim	0x76c
#define flti_cvttqsvim	0x76f
#define flti_cvtqssuim	0x77c
#define flti_cvtqtsuim	0x77e
#define flti_addssui	0x780
#define flti_subssui	0x781
#define flti_mulssui	0x782
#define flti_divssui	0x783
#define flti_addtsui	0x7a0
#define flti_subtsui	0x7a1
#define flti_multsui	0x7a2
#define flti_divtsui	0x7a3
#define flti_cvttssui	0x7ac
#define flti_cvttqsvi	0x7af
#define flti_cvtqssui	0x7bc
#define flti_cvtqtsui	0x7be
#define flti_addssuid	0x7c0
#define flti_subssuid	0x7c1
#define flti_mulssuid	0x7c2
#define flti_divssuid	0x7c3
#define flti_addtsuid	0x7e0
#define flti_subtsuid	0x7e1
#define flti_multsuid	0x7e2
#define flti_divtsuid	0x7e3
#define flti_cvttssuid	0x7ec
#define flti_cvttqsvid	0x7ef
#define flti_cvtqssuid	0x7fc
#define flti_cvtqtsuid	0x7fe

/*
 * op_fltv: vax floating point group. These instructions are
 * floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
#define fltv_addfc	0x000
#define fltv_subfc	0x001
#define fltv_mulfc	0x002
#define fltv_divfc	0x003
#define fltv_cvtdgc	0x01e
#define fltv_addgc	0x020
#define fltv_subgc	0x021
#define fltv_mulgc	0x022
#define fltv_divgc	0x023
#define fltv_cvtgfc	0x02c
#define fltv_cvtgdc	0x02d
#define fltv_cvtgqc	0x02f
#define fltv_cvtqfc	0x03c
#define fltv_cvtqgc	0x03e
#define fltv_addf	0x080
#define fltv_subf	0x081
#define fltv_mulf	0x082
#define fltv_divf	0x083
#define fltv_cvtdg	0x09e
#define fltv_addg	0x0a0
#define fltv_subg	0x0a1
#define fltv_mulg	0x0a2
#define fltv_divg	0x0a3
#define fltv_cmpgeq	0x0a5
#define fltv_cmpglt	0x0a6
#define fltv_cmpgle	0x0a7
#define fltv_cvtgf	0x0ac
#define fltv_cvtgd	0x0ad
#define fltv_cvtgq	0x0af
#define fltv_cvtqf	0x0bc
#define fltv_cvtqg	0x0be
#define fltv_addfuc	0x100
#define fltv_subfuc	0x101
#define fltv_mulfuc	0x102
#define fltv_divfuc	0x103
#define fltv_cvtdguc	0x11e
#define fltv_addguc	0x120
#define fltv_subguc	0x121
#define fltv_mulguc	0x122
#define fltv_divguc	0x123
#define fltv_cvtgfuc	0x12c
#define fltv_cvtgduc	0x12d
#define fltv_cvtgqvc	0x12f
#define fltv_addfu	0x180
#define fltv_subfu	0x181
#define fltv_mulfu	0x182
#define fltv_divfu	0x183
#define fltv_cvtdgu	0x19e
#define fltv_addgu	0x1a0
#define fltv_subgu	0x1a1
#define fltv_mulgu	0x1a2
#define fltv_divgu	0x1a3
#define fltv_cvtgfu	0x1ac
#define fltv_cvtgdu	0x1ad
#define fltv_cvtgqv	0x1af
#define fltv_addfsc	0x400
#define fltv_subfsc	0x401
#define fltv_mulfsc	0x402
#define fltv_divfsc	0x403
#define fltv_cvtdgsc	0x41e
#define fltv_addgsc	0x420
#define fltv_subgsc	0x421
#define fltv_mulgsc	0x422
#define fltv_divgsc	0x423
#define fltv_cvtgfsc	0x42c
#define fltv_cvtgdsc	0x42d
#define fltv_cvtgqsc	0x42f
#define fltv_addfs	0x480
#define fltv_subfs	0x481
#define fltv_mulfs	0x482
#define fltv_divfs	0x483
#define fltv_cvtdgs	0x49e
#define fltv_addgs	0x4a0
#define fltv_subgs	0x4a1
#define fltv_mulgs	0x4a2
#define fltv_divgs	0x4a3
#define fltv_cmpgeqs	0x4a5
#define fltv_cmpglts	0x4a6
#define fltv_cmpgles	0x4a7
#define fltv_cvtgfs	0x4ac
#define fltv_cvtgds	0x4ad
#define fltv_cvtgqs	0x4af
#define fltv_addfsuc	0x500
#define fltv_subfsuc	0x501
#define fltv_mulfsuc	0x502
#define fltv_divfsuc	0x503
#define fltv_cvtdgsuc	0x51e
#define fltv_addgsuc	0x520
#define fltv_subgsuc	0x521
#define fltv_mulgsuc	0x522
#define fltv_divgsuc	0x523
#define fltv_cvtgfsuc	0x52c
#define fltv_cvtgdsuc	0x52d
#define fltv_cvtgqsvc	0x52f
#define fltv_addfsu	0x580
#define fltv_subfsu	0x581
#define fltv_mulfsu	0x582
#define fltv_divfsu	0x583
#define fltv_cvtdgsu	0x59e
#define fltv_addgsu	0x5a0
#define fltv_subgsu	0x5a1
#define fltv_mulgsu	0x5a2
#define fltv_divgsu	0x5a3
#define fltv_cvtgfsu	0x5ac
#define fltv_cvtgdsu	0x5ad
#define fltv_cvtgqsv	0x5af

#endif /* _ALPHA_INST_H_ */
