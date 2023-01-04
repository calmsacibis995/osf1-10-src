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
static char	*sccsid = "@(#)$RCSfile: tojkata.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:32:37 $";
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
 * FUNCTIONS: to_jkata, _tojkata
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
/* tojkata.c	1.1  com/lib/c/gen/KJI,3.1,9021 10/19/89 17:27:12 */
/************************************************************************/
/*									*/
/*  SYNOPSIS								*/
/*	int								*/
/*	tojkata(c)							*/
/*	register c;							*/
/*									*/
/*	int								*/
/*	_tojkata(c)							*/
/*	int c;								*/
/*									*/
/*  DESCRIPTION								*/
/*	Returns the katakana equivalent of a hiragana character. The	*/
/*	unchecked form, _tojkata, does not check for a valid hiragana	*/
/*	character. 							*/
/*									*/
/*  DIAGNOSTICS								*/
/*	For the checked form tojkata, the input value will be returned	*/
/*	unchanged if the conversion cannot be made.			*/
/*									*/
/************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef KJI
#include <NLctype.h>

int
tojkata(c)	/* checked form */
register c;
{
	int _tojkata();

	if (isjhira(c))
		return _tojkata(c);
	else
		return c;
}

int
_tojkata(c)	/* unchecked form */
int c;
{
	register unsigned char c0, c1;

	/* get lsb */
	c1 = _NCbot(c);

	/* converted msb is always 0x83 */
	c0 = 0x83;
	
	/* convert lsb */
	c1 = c1 - 0x5f;

	/* correct for 0x7f gap */
	if (c1 >= 0x7f)
		c1++;

	return _NCd2(c0, c1);
}
#endif  /* KJI */
