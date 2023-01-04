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
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _AOUTHDR_H
#define _AOUTHDR_H

#if __mips__
/*
 * Values for the magic field in aouthdr
 */
#ifdef __LANGUAGE_C__
#define	OMAGIC	0407
#define	NMAGIC	0410
#define	ZMAGIC	0413
#define SMAGIC  0411
#define	LIBMAGIC	0443
#define	N_BADMAG(x) \
    (((x).magic)!=OMAGIC && ((x).magic)!=NMAGIC && ((x).magic)!=ZMAGIC && \
     ((x).magic)!=LIBMAGIC && ((x).magic)!=SMAGIC)

#endif /* __LANGUAGE_C__ */
#ifdef __LANGUAGE_PASCAL__
#define	OMAGIC	8#407
#define	NMAGIC	8#410
#define	ZMAGIC	8#413
#define SMAGIC  8#411
#define	LIBMAGIC	8#443
#endif /* __LANGUAGE_PASCAL__ */
#endif /* __mips__ */

#ifdef __LANGUAGE_C__
typedef	struct aouthdr {
	short	magic;		/* see above				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to DW bdry*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
#if __u3b
	long	dum1;
	long	dum2;		/* pad to entry point	*/
#endif
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
#if __mips__
	long	bss_start;	/* base of bss used for this file	*/
	long	gprmask;	/* general purpose register mask	*/
	long	cprmask[4];	/* co-processor register masks		*/
	long	gp_value;	/* the gp value used for this object    */
#endif /* __mips__ */
} AOUTHDR;
#define AOUTHSZ sizeof(AOUTHDR)
#endif /* __LANGUAGE_C__ */

#ifdef __LANGUAGE_PASCAL__
type
  aouthdr = packed record
      magic : short;			/* see magic.h			     */
      vstamp : short;			/* version stamp		     */
      tsize : long;			/* text size in bytes, padded to FW  */
					/* bdry 			     */
      dsize : long;			/* initialized data " " 	     */
      bsize : long;			/* uninitialized data " "	     */
#if __u3b
      dum1 : long;
      dum2 : long;			/* pad to entry point		     */
#endif
      entry : long;			/* entry pt.			     */
      text_start : long;		/* base of text used for this file   */
      data_start : long;		/* base of data used for this file   */
      bss_start : long;			/* base of bss used for this file    */
      gprmask : long;			/* general purpose register mask     */
      cprmask : array[0..3] of long;	/* co-processor register masks	     */
      gp_value : long;			/* the gp value used for this object */
    end {record};
#endif /* __LANGUAGE_PASCAL__ */

/* 
*/
#endif
