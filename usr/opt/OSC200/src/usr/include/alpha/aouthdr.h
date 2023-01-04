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
/* $Header: /usr/sde/osf1/rcs/os/src/usr/include/alpha/aouthdr.h,v 1.2.6.2 1993/12/15 22:12:36 Thomas_Peterson Exp $ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _AOUTHDR_H
#define _AOUTHDR_H

#if __mips || __alpha
/*
 * Values for the magic field in aouthdr
 */
#if defined(__LANGUAGE_C__) || defined(__cplusplus)
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

#if defined(__LANGUAGE_C__) || defined(__cplusplus)
typedef	struct aouthdr {
	short	magic;		/* see above				*/
	short	vstamp;		/* version stamp			*/
#if defined(__mips64) || defined(__alpha)
        short   bldrev;
        short   padcell;
#endif
	long 	tsize;		/* text size in bytes, padded to DW bdry*/
	long 	dsize;		/* initialized data "  "		*/
	long 	bsize;		/* uninitialized data "   "		*/
#if __u3b
	long	dum1;
	long	dum2;		/* pad to entry point	*/
#endif
	long 	entry;	/* entry pt.				*/
	long 	text_start;	/* base of text used for this file	*/
	long 	data_start;	/* base of data used for this file	*/
#if defined(__mips) || defined(__mips64) || defined(__alpha)
	long	bss_start;	/* base of bss used for this file	*/
#if defined(__mips64) || defined(__alpha)
	int	gprmask;	/* general purpose register mask	*/
	int	fprmask; 	/* floating point register mask		*/
#else
	long	gprmask;	/* general purpose register mask	*/
	long	cprmask[4];	/* co-processor register masks		*/
#endif
	long	gp_value;	/* the gp value used for this object    */
#endif /* __mips */
} AOUTHDR;
#define AOUTHSZ sizeof(AOUTHDR)
#endif /* __LANGUAGE_C__ */

#ifdef __LANGUAGE_PASCAL__
type
  aouthdr = packed record
      magic : short;			/* see magic.h			     */
      vstamp : short;			/* version stamp		     */
#if defined(__mips64) || defined(__alpha)
      bldrev : short;
      padcell : short;
      tsize : integer64;		/* text size in bytes, padded to FW  */
					/* bdry 			     */
      dsize : integer64;		/* initialized data " " 	     */
      bsize : integer64;		/* uninitialized data " "	     */
      entry : integer64;		/* entry pt.			     */
      text_start : integer64;		/* base of text used for this file   */
      data_start : integer64;		/* base of data used for this file   */
      bss_start : integer64;		/* base of bss used for this file    */
#else
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
#endif
      gprmask : long;			/* general purpose register mask     */
#if defined(__mips64) || defined(__alpha)
      fprmask : long;			/* floating point register mask	     */
      gp_value : integer64;		/* the gp value used for this object */
#else
      cprmask : array[0..3] of long;	/* co-processor register masks	     */
      gp_value : long;			/* the gp value used for this object */
#endif
    end {record};
#endif /* __LANGUAGE_PASCAL__ */
#endif /* __mips */
#endif
