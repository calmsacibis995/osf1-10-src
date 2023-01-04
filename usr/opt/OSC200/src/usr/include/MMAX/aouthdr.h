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
 *	@(#)$RCSfile: aouthdr.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:05:19 $
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
/*	@(#)aouthdr.h	3.1	*/
/*		*/
/*	2/26/91	*/
/*	5.2 SID aouthdr.h	2.4 12/15/82	*/


#ifndef _AOUTHDR_H_
#define _AOUTHDR_H_

typedef	struct aouthdr {
	short	magic;		/* see magic.h				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to FW
				   bdry					*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
#if u3b
	long	dum1;
	long	dum2;		/* pad to entry point	*/
#endif
#ifdef ns32000
	long	msize;		/* size of module table, "modsize"	*/
	long	mod_start;	/* start addr of mod table, "modstart"	*/
#endif
	long	entry;		/* entry point, value of "start"	*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
#ifdef ns32000
	short	entry_mod;	/* module number of entry point, unused	*/
	unsigned short	flags;	/* section alignment and protection	*/
#endif
} AOUTHDR;

#ifdef ns32000
	/* flag word definitions for file positioning of raw data	*/
	/* also for segment protections and Unix system type		*/
	/* only the 1024, 4096 and sys flavors are supported		*/

#define U_AL		0x07	/* section alignment mask		*/
/*#define U_AL_NONE	0x00	/* fullword alignment			*/
/*#define U_AL_512	0x01	/* 512 byte alignment			*/
#define	U_AL_1024	0x02	/* 1K alignment				*/
/*#define U_AL_2048	0x03	/* 2K alignment				*/
#define	U_AL_4096	0x04	/* 4K alignment				*/
/*#define U_AL_8192	0x05	/* 8K alignment				*/
/*#define U_PR		0x38	/* section proctections			*/
/*#define U_PR_DATA	0x08	/* data section				*/
/*#define U_PR_TEXT	0x10	/* text section				*/
/*#define U_PR_MOD	0x20	/* module section			*/
#define	U_SYS_5		0x100	/* UMAX System V rel 2			*/
#define	U_SYS_42	0x200	/* UMAX System 4.2 BSD			*/
#endif

#endif /* _AOUTHDR_H_ */
