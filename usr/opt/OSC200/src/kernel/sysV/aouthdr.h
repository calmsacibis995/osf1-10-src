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
 *	@(#)aouthdr.h	9.2	(ULTRIX/OSF)	10/18/91
 */ 
/*
 */
/* --------------------------------------------------------- */
/* | Copyright (c) 1986, 1989 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                                  | */
/* --------------------------------------------------------- */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./kernel/sysV/aouthdr.h,v 4.3.2.4 1992/12/09 15:01:30 Jay_Estabrook Exp $ */
#ifndef __AOUTHDR_H
#define __AOUTHDR_H

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#if mips || __alpha
/*
 * Values for the magic field in aouthdr
 */
#ifdef __LANGUAGE_C__
#define OMAGIC	0407		/* old impure format */
#define NMAGIC	0410		/* read-only text */
#define ZMAGIC	0413		/* demand load format */
#define	LIBMAGIC	0443
#define	N_BADMAG(x) \
    (((x).magic)!=OMAGIC && ((x).magic)!=NMAGIC && ((x).magic)!=ZMAGIC && \
     ((x).magic)!=LIBMAGIC)

#endif /* __LANGUAGE_C__ */
#ifdef LANGUAGE_PASCAL
#define	OMAGIC	8#407
#define	NMAGIC	8#410
#define	ZMAGIC	8#413
#define	LIBMAGIC	8#443
#endif /* LANGUAGE_PASCAL */

#ifdef __LANGUAGE_C__
typedef	struct aouthdr {
	short	magic;		/* see above				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to DW bdry*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
#if u3b
	long	dum1;
	long	dum2;		/* pad to entry point	*/
#endif
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
#if mips || __alpha
	long	bss_start;	/* base of bss used for this file	*/
/* START ALPHA */
	int	gprmask;	/* general purpose register mask	*/
	int	fprmask; 	/* floating point register mask		*/
/* END ALPHA */
	long	gp_value;	/* the gp value used for this object    */
#endif /* mips */
} AOUTHDR;
#define AOUTHSZ sizeof(AOUTHDR)
#endif /* __LANGUAGE_C__ */

#ifdef LANGUAGE_PASCAL
type
  aouthdr = packed record
      magic : short;			/* see magic.h			     */
      vstamp : short;			/* version stamp		     */
      tsize : quad;			/* text size in bytes, padded to FW  */
					/* bdry 			     */
      dsize : quad;			/* initialized data " " 	     */
      bsize : quad;			/* uninitialized data " "	     */
      entry : quad;			/* entry pt.			     */
      text_start : quad;		/* base of text used for this file   */
      data_start : quad;		/* base of data used for this file   */
      bss_start : quad;			/* base of bss used for this file    */
      gprmask : long;			/* general purpose register mask     */
      fprmask : long;			/* floating point register mask	     */
      gp_value : quad;			/* the gp value used for this object */
    end {record};
#endif /* LANGUAGE_PASCAL */
#endif /* mips */
#endif
