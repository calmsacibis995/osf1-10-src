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
static char	*sccsid = "@(#)$RCSfile: atox.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:14 $";
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
/*
 * 5799-CGZ (C) COPYRIGHT IBM CORPORATION 1986,1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef lint
#endif

#ifdef STANDALONE
#define isdigit(x) ('0' <= (x) && (x) <= '9')
#else
#include "i386/rdb/ctype.h"
#endif


/*
 * convert string in hex to binary
 */
int atox(ptr)
	register char *ptr;
{
	register int n = 0;
	register int c;

	for (; c = *ptr++;) {
		if (isdigit(c))
			c -= '0';
		else if ('a' <= c && c <= 'f')
			c -= 'a' - 10;
		else if ('A' <= c && c <= 'F')
			c -= 'A' - 10;
		else
			break;
		n = (n << 4) + c;
	}
	return (n);
}
