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
static char	*sccsid = "@(#)$RCSfile: mbspbrk.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:28:12 $";
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
 * FUNCTIONS: mbspbrk
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
 * 1.6  com/lib/c/nls/mbspbrk.c, libcnls, bos320, 9132320m 8/2/91 14:05:20
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak mbspbrk = __mbspbrk
#endif
#endif
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
/*
 * NAME: mbspbrk
 *
 * FUNCTION: Locate the first occurrence of multibyte characters (code 
 *           points) in a string
 *
 * PARAMETERS: 
 *	     char *string - compared string
 *	     char *brkset - comparing multibyte character set
 *
 * RETURN VALUE DESCRIPTION: 
 *  Return ptr to first occurrence of any character from `brkset'
 *  in the character string `string'; NULL if none exists.
 */

char *
mbspbrk(char *string, char *brkset)
{
	register wchar_t *p;
	wchar_t sc;
	wchar_t *nlbrkset;
	register int i;

	/**********
	  if in a single byte codeset, mbspbrk == strpbrk
	*********/
	if (MB_CUR_MAX == 1)
	    return (strpbrk(string, brkset));

	/**********
	  get the space for the process code version of the brkset
	**********/
	if ((nlbrkset=(wchar_t *)malloc(sizeof(wchar_t)*(strlen(brkset)+1)))
	    == (wchar_t *)NULL)
	    return((char *)NULL);

	/*********
	  convert the brkset to process code
	**********/
	if((mbstowcs(nlbrkset, brkset, strlen(brkset)+1)) == -1) {
	    free (nlbrkset);
	    return((char *)NULL);
	}
		    
	for (; ; ) {
		if (!*string)
			break;
		/**********
		  if at any time an invalid character is found,
		  stop processing
		**********/
		if ((i=mbtowc(&sc, string, MB_CUR_MAX)) == -1) {
		    free(nlbrkset);
		    return((char *)NULL);
		}
		for(p = nlbrkset; *p != 0 && *p != sc; ++p)
			;
		if(*p != 0) {
			free(nlbrkset);
			return(string);
		}
		string += i;
	}
	free(nlbrkset);
	return(NULL);
}
