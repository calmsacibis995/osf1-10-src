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
static char	*sccsid = "@(#)$RCSfile: jistoa.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:20:19 $";
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
 * FUNCTIONS: jistoa
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
/* jistoa.c	1.1  com/lib/c/gen/KJI,3.1,9021 10/19/89 17:25:27 */
/**********************************************************************/
/*								      */
/* SYNOPSIS							      */
/*	int							      */
/*	jistoa (c)						      */
/*	register c;						      */
/*								      */
/* DESCRIPTION							      */
/*	jistoa returns the ASCII equivalent of a Shift-JIS character. */
/*	The function uses the _jistoa macro, which in turn does a     */
/*	lookup into the _jistoatab table.			      */
/*								      */
/* DIAGNOSTICS							      */
/*	If the input character does not have an ASCII equivalent,     */
/*	the function returns the input value.			      */
/*								      */
/**********************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef KJI
#include <NLctype.h>

int
jistoa (c)
register c;
{
	int b3, b4;		/* 3rd and 4th bytes of c */

	if (c > 0x829a)
		return (c);
	if ((b3 = c >> 8) != 0x81 && b3 != 0x82)
		/* c has no ASCII equivalent--first byte is not 0x81 or 0x82 */
		return (c);
	if ((b4 = c & 0xff) < 0x40 || b4 > 0x9a)
		/* c has no ASCII equivalent--second byte is not valid */
		return (c);
	return (_jistoa (c));
}
#endif  /* KJI */
