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
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header */
#if __mips /* This is all architecture specific */
#define fli_op 0x26

#ifndef l_fmt
#define l_fmt 5
#endif

#define S_FMT (cop_op+s_fmt)
#define D_FMT (cop_op+d_fmt)
#define W_FMT (cop_op+w_fmt)
#define E_FMT (cop_op+e_fmt)
#define L_FMT (cop_op+l_fmt)
#ifdef _MAD_MSB
#define SMA_FMT (0x18+s_fmt)
#define DMA_FMT (0x18+d_fmt)
#endif

			/* ssc1, swc1 has the same slot, hence included once */
#define CASE_STORES \
	case sb_op: \
	case sh_op: \
	case sw_op: \
	case sc_op: \
	case swl_op: \
	case swr_op: \
	case sd_op: \
	case swc1_op: \
	case sdc1_op: \
	case swc2_op: \
	case sdc2_op: \
	case sdl_op: \
	case sdr_op: \
	case scd_op

#define CASE_LOADS \
	case lb_op: \
	case lbu_op: \
	case lh_op: \
	case lhu_op: \
	case lw_op: \
	case lwl_op: \
	case lwr_op: \
	case ll_op: \
	case ld_op: \
	case ldl_op: \
	case ldr_op: \
	case lld_op: \
	case lwc1_op: \
	case ldc1_op: \
	case lwc2_op: \
	case ldc2_op


#define CASE_FCMP \
	case fcmp_op+0: \
	case fcmp_op+1: \
	case fcmp_op+2: \
	case fcmp_op+3: \
	case fcmp_op+4: \
	case fcmp_op+5: \
	case fcmp_op+6: \
	case fcmp_op+7: \
	case fcmp_op+8: \
	case fcmp_op+9: \
	case fcmp_op+10: \
	case fcmp_op+11: \
	case fcmp_op+12: \
	case fcmp_op+13: \
	case fcmp_op+14: \
	case fcmp_op+15:

#undef lcache_op	0x30
#undef flush_op		0x34
#undef scache_op	0x38
#undef inval_op		0x3C

#undef ll_op
#undef sc_op

#define ll_op		0x30
#define sc_op		0x38

#ifdef ld_op
#undef ld_op
#undef sd_op
#define ld_op	0x37
#define sd_op	0x3f
#endif

#undef ldl_op
#undef ldr_op
#undef lld_op
#undef lwu_op
#undef sdl_op
#undef sdr_op
#undef daddi_op
#undef daddiu_op
#undef dadd_op
#undef daddu_op
#undef scd_op
#undef dsub_op
#undef dsubu_op
#undef dsll_op
#undef dsrl_op
#undef dsra_op
#undef dsll32_op
#undef dsrl32_op
#undef dsra32_op
#undef dsllv_op
#undef dsrlv_op
#undef dsrav_op
#undef dmult_op
#undef dmultu_op
#undef ddiv_op
#undef ddivu_op
#undef lsc1_op
#undef ssc1_op
#undef dmtc1_op
#undef dmfc1_op

#define ldl_op		0x1A
#define ldr_op		0x1B
#define lld_op		0x34
#define lwu_op		0x27
#define sdl_op		0x2C
#define sdr_op		0x2D
#define scd_op		0x3C
#define daddi_op	0x18
#define daddiu_op	0x19
#define dadd_op		0x2C
#define daddu_op	0x2D
#define dsub_op		0x2E
#define dsubu_op	0x2F
#define dsll_op		0x38
#define dsrl_op		0x3A
#define dsra_op		0x3B
#define dsll32_op	0x3C
#define dsrl32_op	0x3E
#define dsra32_op	0x3F
#define dsllv_op	0x14
#define dsrlv_op	0x16
#define dsrav_op	0x17
#define dmult_op	0x1C
#define dmultu_op	0x1D
#define ddiv_op		0x1E
#define ddivu_op	0x1F
#define lsc1_op		0x33
#define ssc1_op		0x3B
#define dmtc1_op	0x05
#define dmfc1_op	0x01
#define dmtc_op		0x05		/* generic */
#define dmfc_op		0x01
#define fcvtl_op        0x25
#define froundl_op      0x08
#define ftruncl_op      0x09
#define fceill_op       0x0A
#define ffloorl_op      0x0B
#define eret_op         0x18            /* cop0 opcode */
#define cache_op        0x2F            /* cop0 opcode */
#endif
