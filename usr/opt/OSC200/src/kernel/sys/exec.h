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
 *	@(#)$RCSfile: exec.h,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/12/08 18:13:25 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#ifndef	_SYS_EXEC_H_
#define _SYS_EXEC_H_

/*
 * Header prepended to each a.out file.
 */

#if	!defined(mips) && !defined(__alpha)

struct exec {

#if	defined(sun3) || defined(sun4) || defined(__hp_osf)
	unsigned short  a_machtype;     /* machine type */
	unsigned short  a_magic;        /* magic number */
#else	/* defined(sun3) || defined(sun4) || defined(__hp_osf) */
	long	a_magic;	/* magic number */
#endif	/* defined(sun3) || defined(sun4) || defined(__hp_osf) */
	unsigned long	a_text;		/* size of text segment */
	unsigned long	a_data;		/* size of initialized data */
	unsigned long	a_bss;		/* size of uninitialized data */
	unsigned long	a_syms;		/* size of symbol table */
	unsigned long	a_entry;	/* entry point */
	unsigned long	a_trsize;	/* size of text relocation */
	unsigned long	a_drsize;	/* size of data relocation */
};
#endif	/* !defined(mips) && !defined(__alpha) */


#define OMAGIC	0407		/* old impure format */
#define NMAGIC	0410		/* read-only text */
#define ZMAGIC	0413		/* demand load format */


#ifdef	mips

#if	BYTE_MSF
#define MIPSMAGIC	MIPSEBMAGIC
#else
#define MIPSMAGIC	MIPSELMAGIC
#endif
#endif	/* mips */

#if defined (mips) || defined (__alpha)
/*
 * The macro N_TXTOFF() takes pointers to file header
 * [struct filehdr*] and optional header [struct aouthdr *] and returns
 * the file offset to the start of the raw data for the .text section.
 * The raw data for the three data sections follows the start of the .text
 * section by the value of tsize in the optional header.
 */
/* SCNROUND is the size that section headers are rounded off to */
#define SCNROUND 16
#define OSCNRND  8

#define N_TXTOFF(f, a) \
 ((long)((a).magic == ZMAGIC ? 0 : \
  ((a).vstamp < 23 ? \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + OSCNRND-1) & ~(OSCNRND-1)) : \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + SCNROUND-1) & ~(SCNROUND-1)))))

#ifndef _KERNEL
#include <aouthdr.h>
#include <filehdr.h>

struct exec {
	struct filehdr	ex_f;
	struct aouthdr	ex_o;
};

#endif
/*
 * for compatibility
 */
#define a_data	ex_o.dsize
#define a_text	ex_o.tsize
#define a_bss	ex_o.bsize
#define a_entry	ex_o.entry
#define a_magic	ex_o.magic	/* Will be [ONZ]MAGIC */

#endif	/* mips or __alpha */

#if	defined(sun3) || defined(sun4) || defined(__hp_osf)
/* Sun machine types */

#define M_OLDSUN2	0	/* old sun-2 executable files */
#define M_68010		1	/* runs on either 68010 or 68020 */
#define M_68020		2	/* runs only on 68020 */
#endif	/* defined(sun3) || defined(sun4) || defined(__hp_osf) */

#endif	/* _SYS_EXEC_H_ */
