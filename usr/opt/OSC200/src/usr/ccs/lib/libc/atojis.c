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
static char	*sccsid = "@(#)$RCSfile: atojis.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:26:00 $";
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
 *  COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: atojis
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
/* atojis.c	1.2  com/lib/c/gen/KJI,3.1,9021 1/30/90 14:38:33 */

/**********************************************************************/
/*								      */
/* SYNOPSIS							      */
/*	int							      */
/*	atojis (c)						      */
/*	register c;						      */
/*								      */
/* DESCRIPTION							      */
/*	atojis returns the Shift-JIS equivalent of an ASCII character.*/
/*	The function uses the _atojis macro, which in turn does a     */
/*	lookup into the _atojistab table.			      */
/*								      */
/* DIAGNOSTICS							      */
/*	If the input character does not have a Shift-JIS equivalent,  */
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
atojis (c)
register c;
{
	if (c < 0x20 || c > 0x7e)
		/* c has no Shift-JIS equivalent */
		return (c);
	return (_atojis (c));
}
#endif  /* KJI */
