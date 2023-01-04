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
static char	*sccsid = "@(#)$RCSfile: mbsncmp.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:28:01 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: mbsncmp
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * 1.8  com/lib/c/nls/mbsncmp.c, libcnls, bos320, 9132320b 7/23/91 09:30:19
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak mbsncmp = __mbsncmp
#endif
#endif
#include <sys/types.h>
#include <stdlib.h>

/*
 * NAME: mbsncmp
 *
 * FUNCTION: Compare a specific number of multibyte characters (code points) 
 *  in one multibyte character string to another multibyte character string.
 *
 * PARAMETERS:
 *	char *s1	-	the multibyte character string
 *	char *s2	-	the multibyte character string
 *	size_t    n	-	the number of multibyte characters
 *
 * RETURN VALUE DESCRIPTION: 
 *   s1>s2; >0  s1==s2; 0  s1<s2; <0.
 *
 *
 */

int
mbsncmp(const char *s1, const char *s2, size_t n) 
{
	size_t i;
	wchar_t pc1, pc2;


	if (s1 == s2)
		return(0);
	mbtowc(&pc1,s1,MB_CUR_MAX);
	mbtowc(&pc2,s2,MB_CUR_MAX);
	for (i = 0; i < n && pc1 == pc2; i++) {
		if(*s1 == '\0')
			return(0);
		s1 += mblen(s1,MB_CUR_MAX);
		s2 += mblen(s2,MB_CUR_MAX);
		mbtowc(&pc1,s1,MB_CUR_MAX);
		mbtowc(&pc2,s2,MB_CUR_MAX);
	}
	return( (i == n) ? 0: (pc1 > pc2) ? 1 : -1);
}
