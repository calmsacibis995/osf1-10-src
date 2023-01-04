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
 *	@(#)$RCSfile: tss386.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:19 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#ifdef MACH

struct tss386 { 	/* not really a TSS but this will do for now */
	int t_gs;
	int t_fs;
	int t_es;
	int t_ds;
	int t_edi;
	int t_esi;
	int t_ebp;
	int t_esp;
	int t_ebx;
	int t_edx;
	int t_ecx;
	int t_eax;
	int t_trap;
	int t_error;
	int t_eip;
	int t_cs;
	int t_eflags;
	int t_uesp;
	int t_uss;
};
#else

/* 
 * (C) Copyright IBM Corp. 1989 
 * All Rights Reserved 
 * Licensed Materials - Property of IBM 
 */

#ifndef _h_TSS386
#define	_h_TSS386

#define 	TSSNOBITMAP	0xDFFF		/* GH Errata 6 0ffff to 0dfff */
#define 	MAXIOPORT	0x10000		/* Maximum mappable I/O port */

	/* NEEDSWORK: this really should not be needed as it is now dynamic */
#define 	IOMAPSIZE	(MAXIOPORT >> 9) /* Size of I/O bit map */

struct tss386 {
	ushort t_back;		/* Back link to previous tss */
	ushort t_rsvdx;
	ulong t_esp0;		/* SP to load on change to priv level 0 */
	ushort t_ss0;		/* SS to load on change to priv level 0 */
	ushort t_rsvd0;
	ulong t_esp1;		/* SP to load on change to priv level 1 */
	ushort t_ss1;		/* SS to load on change to priv level 1 */
	ushort t_rsvd1;
	ulong t_esp2;		/* SP to load on change to priv level 2 */
	ushort t_ss2;		/* SS to load on change to priv level 2 */
	ushort t_rsvd2;
	ulong t_pdbr;		/* Page directory base register (cr3) */
	ulong t_eip;		/* Instruction pointer */
	ulong t_eflags;		/* Machine status flags */
	ulong t_eax;		/* EAX (accumulator) register */
	ulong t_ecx;		/* ECX (count) register */
	ulong t_edx;		/* EDX (data) register */
	ulong t_ebx;		/* EBX (base) register */
	ulong t_esp;		/* Stack pointer */
	ulong t_ebp;		/* Base Pointer */
	ulong t_esi;		/* ESI (Source/Index) register */
	ulong t_edi;		/* EDI (Destination/Index) register */
	ushort t_es;		/* ES (Extra segment) register */
	ushort t_rsvd3;
	ushort t_cs;		/* CS (Code segment) register */
	ushort t_rsvd4;
	ushort t_ss;		/* SS (Stack segment) register */
	ushort t_rsvd5;
	ushort t_ds;		/* DS (Data segment) register */
	ushort t_rsvd6;
	ushort t_fs;		/* FS (extra segment) register */
	ushort t_rsvd7;
	ushort t_gs;		/* GS (extra segment) register */
	ushort t_rsvd8;
	ushort t_ldt;		/* LDT for this task */
	ushort t_rsvd9;
	ushort t_bits;		/* Special bits (e.g. the T-bit) */
	ushort t_bitmapbase;	/* Base of the i/o permission bit map */
#ifdef MERGE386
	char t_iobitmap[IOMAPSIZE];	/* I/O bit map */
#endif
};

#ifdef KERNEL
extern struct tss386 tss;
#endif

#endif /* _h_TSS386 */
#endif /* MACH */
