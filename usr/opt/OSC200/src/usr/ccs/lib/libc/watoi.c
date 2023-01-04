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
static char	*sccsid = "@(#)$RCSfile: watoi.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:58:28 $";
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
 * LIBCGEN: watoi.c
 *
 * FUNCTIONS: watoi
 *
 * ORIGINS: 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* watoi.c	1.2  com/lib/c/cnv/KJI,3.1,9021 12/14/89 13:08:46 */

/*
 * NAME: watoi
 *                                                                    
 * FUNCTION: 	SHIFT-JIS TO INTEGER
 *                                                                    
 * RETURNS:
 *	Converts the string 'str' to integer.
 */ 
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak watoi = __watoi
#endif
#endif
#include <NLctype.h>

#ifdef KJI
int
watoi(NLchar *p)
{
	register int n, f;

	int ascp;
	n = 0;
	f = 0;
	for(;;p++) {
		switch(ascp = (isascii(*p)? *p: _jistoa(*p))) {
		case ' ':
		case '\t':
			continue;
		case '-':
			f++;
		case '+':
			p++;
			ascp = isascii(*p)? *p: _jistoa(*p);
		}
		break;
	}
	while(ascp >= '0' && ascp <= '9') {
		n = n*10 + ascp - '0';
		ascp = isascii(*(++p))? *p: _jistoa(*p);
	}
	return(f? -n: n);
}
#else /* non KJI stub */
int
watoi(register NLchar *p)
{
}
#endif /* KJI */
