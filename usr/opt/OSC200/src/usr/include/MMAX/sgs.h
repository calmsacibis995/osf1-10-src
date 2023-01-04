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
 *	@(#)$RCSfile: sgs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:06:00 $
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


#if vax
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)VAXROMAGIC) || \
			  (((unsigned short)x)==(unsigned short)VAXWRMAGIC))
#endif
#if m68
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)MC68MAGIC) || \
			  (((unsigned short)x)==(unsigned short)MC68TVMAGIC))
#endif
#if n16
#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)N16ROMAGIC) || \
			 (((unsigned short)x)==(unsigned short)N16WRMAGIC) || \
			 (((unsigned short)x)==(unsigned short)NS32SMAGIC) || \
			 (((unsigned short)x)==(unsigned short)NS32GMAGIC))
#define ISN16MAGIC(x)	((((unsigned short)x)==(unsigned short)N16ROMAGIC) || \
			 (((unsigned short)x)==(unsigned short)N16WRMAGIC))
#endif
#ifdef ARTYPE	
#define ISARCHIVE(x)	((((unsigned short)x)==(unsigned short)ARTYPE))
#define BADMAGIC(x)	((((x)>>8) < 7) && !ISMAGIC(x) && !ISARCHIVE(x))
#else
#define BADMAGIC(x)	((((x)>>8) < 7) && !ISMAGIC(x))
#endif


/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC :	     : readonly sharable text segment
 *		AOUT2MAGIC:	     : writable text segment
 *		PAGEMAGIC  : default : configured for paging
 */

#define AOUT1MAGIC 0410
#define AOUT2MAGIC 0407
#define PAGEMAGIC  0413

#define	SGSNAME	""
#define SGS ""
#define RELEASE "System V Release 2.0 9/1/83"
