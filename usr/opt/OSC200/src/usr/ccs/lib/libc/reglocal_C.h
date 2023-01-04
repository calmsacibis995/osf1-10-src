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
 * @(#)$RCSfile: reglocal_C.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 22:18:41 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#ifndef _H_REGLOCAL_C
#define _H_REGLOCAL_C
/*
 * COMPONENT_NAME: (LIBCPAT) Internal Regular Expression
 *
 * FUNCTIONS: 
 *
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.1  com/lib/c/pat/reglocal_C.h, 9115320a, bos320 3/28/91 14:52:13 
 */

#include <sys/types.h>

/************************************************************************/
/* Refer to reglocal.h for a description of Regular Expression syntax	*/
/*									*/
/* This header file defines symbols for C locale RE processing		*/
/************************************************************************/
/*									*/
/* Global symbols for regcomp_C() and regexec_C()			*/
/*									*/
/************************************************************************/

#define	BITMAP_LEN	32	/* # bytes for a bitmap array		*/
#define CLASS_SIZE	32	/* max length of character class name	*/

/************************************************************************/
/*									*/
/* Expression repetition interval codes					*/
/*									*/
/************************************************************************/

#define CR_MASK		0xe0	/* repetition type mask			*/
#define CR_STAR		0xe0	/* * == zero or more matches		*/
#define CR_QUESTION	0xc0	/* ? == zero or one match (ERE only)	*/
#define CR_PLUS		0xa0	/* + == one or more mathces (ERE only)	*/
#define CR_INTERVAL	0x80	/* \{m,n\} (BRE) or {m,n} (ERE)		*/
#define CR_INTERVAL_ALL 0x60    /* \{m,\} (BRE) or {m,} (ERE)           */
/************************************************************************/
/*									*/
/* Expression compilation codes						*/
/*									*/
/************************************************************************/

				/* repetition codes			*/
#define CC_CHAR		0x01	/* a single character			*/
#define CC_DOT		0x02	/* . any single character		*/
#define CC_BITMAP	0x03	/* bracket expression bitmap		*/
#define CC_BACKREF	0x04	/* \n subexpression backreference (BRE)	*/
#define CC_SUBEXP_E	0x05	/* \) [BRE] ) [ERE] end subexpression	*/
#define CC_I_CHAR	0x06	/* ignore case CC_CHAR			*/
#define CC_I_BACKREF	0x07	/* ignore case CC_BACKREF		*/

#define CC_NOREP	0x08	/* non-repetition codes			*/
#define CC_STRING	0x08	/* character string			*/
#define CC_I_STRING	0x09	/* ignore case CC_STRING		*/
#define CC_BOL		0x0a	/* ^ beginning-of-line anchor		*/
#define CC_EOL		0x0b	/* $ end-of-line anchor			*/
#define CC_SUBEXP	0x0c	/* \( [BRE] ( [ERE] start subexpression	*/
#define CC_ALTERNATE	0x0d	/* | start alternate expression		*/
#define CC_ALTERNATE_E	0x0e	/* | end alternate expression		*/
#define CC_EOP		0x1f	/* end of compiled RE pattern		*/


#endif /* _H_REGLOCAL_C */
