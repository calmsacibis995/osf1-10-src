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
static char	*sccsid = "@(#)$RCSfile: ujtosj.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:42:14 $";
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
 * FUNCTIONS: ujtosj
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
/* ujtosj.c	1.1  com/lib/c/gen/KJI,3.1,9021 10/19/89 17:28:11 */
/************************************************************************/
/*									*/
/*  SYNOPSIS								*/
/*	unsigned char *							*/
/*	ujtosj(s1, s2)							*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	UNIX-jis to shift-jis string conversion routine. The input 	*/
/*	string s2, containing double-byte UNIX-jis characters, is 	*/
/*	converted to a string of double-byte shift-jis characters, s1. 	*/
/*	Returns s1.							*/
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
ujtosj(s1, s2)
unsigned char *s1, *s2;
{
	int c1, c2;
	unsigned char *s0 = s1;

	while (c1 = *s2++) {
		c1--;
		c1 &= 0x7f;			/* clear MSB */
		c2 = *s2++ & 0x7f;		/* clear MSB */
		if (c1 < 0x5e)
			*s1++ = (c1 >> 1) + 0x71;
		else
			*s1++ = (c1 >> 1) + 0xb1;
		if (c1 & 1)
			*s1++ = c2 + 0x7e;
		else if (c2 <= 0x5f)
			*s1++ = c2 + 0x1f;
		else
			*s1++ = c2 + 0x20;
	}
        *s1 = '\0';
	return s0;
}
#endif  /* KJI */
