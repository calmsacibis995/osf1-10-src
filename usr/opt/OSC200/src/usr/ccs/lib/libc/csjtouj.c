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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: csjtouj.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:43:35 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: csjtouj
 *
 * ORIGINS: 10
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/* csjtouj.c	1.1  com/lib/c/gen/KJI,3.1,9021 10/19/89 17:24:09 */
/************************************************************************/
/*									*/
/*  SYNOPSIS								*/
/*	unsigned char *							*/
/*	csjtouj(s1, s2)							*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	Shift-jis to UNIX-jis conversion routine. Converts the double-	*/
/*	byte shift-jis character pointed to by s2 to a double-byte 	*/
/*	UNIX-jis character pointed to by s1. Returns s1.		*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef KJI
unsigned char *
csjtouj(s1, s2)
unsigned char *s1, *s2;
{
	int c1, c2, p;

	c1 = s2[0];
	c2 = s2[1];
	p = 0;
	if (c2 < 0x7f)
		c2 -= 0x1f;
	else if (c2 < 0x9f)
		c2 -= 0x20;
	else {
		c2 -= 0x7e;
		p++;
	}
	if (c1 < 0xa0)
		s1[0] = (((c1 - 0x81) << 1) + 0x21 + p) | 0x80;
	else
		s1[0] = (((c1 - 0xe0) << 1) + 0x5f + p) | 0x80;
	s1[1] = c2 | 0x80;
	return s1;
}
#endif  /* KJI */
