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
static char *rcsid = "@(#)$RCSfile: regerror.c,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/09/23 18:30:00 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions
 *
 * FUNCTIONS: regerror
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
 * 1.3  com/lib/c/pat/regerror.c, libcpat, bos320, 9130320 7/17/91 15:26:45
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak regerror = __regerror
#endif
#include <sys/localedef.h>
#include <regex.h>
#include <sys/types.h>
#include <stdlib.h>
#include "libc_msg.h"

/************************************************************************/
/* __regerror_std() - Get Text for RE Error Message			*/
/************************************************************************/

size_t
__regerror_std( int errcode, const regex_t *preg,
	char *errbuf, size_t errbuf_size, _LC_collate_t *phdl)
{
	int	erroff;		/* index into local error text table	*/
	nl_catd	catd;		/* MRI catalog descriptor		*/
	int	msglen;		/* error message length			*/
	char	*pmsg;		/* ptr to MRI message			*/
	char	*etable[] =
		{
		"pattern not found",			/* REG_NOMATCH	*/
		"invalid pattern",			/* REG_BADPAT	*/
		"invalid collating element",		/* REG_ECOLLATE	*/
		"invalid character class",		/* REG_ECTYPE	*/
		"last character is \\",			/* REG_EESCAPE	*/
		"invalid backreference number",		/* REG_ESUBREG	*/
		"[] imbalance",				/* REG_EBRACK	*/
		"\\( \\) or ( ) imbalance",		/* REG_EPAREN	*/
		"\\{ \\} imbalence",			/* REG_EBRACE	*/
		"invalid \\{ \\} repetition",		/* REG_BADBR	*/
		"invalid range expression",		/* REG_ERANGE	*/
		"out of memory",			/* REG_ESPACE	*/
		"*?+ not preceded by valid expression",	/* REG_BADRPT	*/
		"invalid multibyte character",		/* REG_ECHAR	*/
		};
#define MAXERROR (sizeof etable / sizeof (char *)) /* max error index	*/

/*
 * verify error code is a valid RE error
 * return error status if invalid error code
 */
	erroff = errcode - REG_NOMATCH;
	if (erroff < 0 || erroff >= MAXERROR)
		return (0);
/*
 * open message catalog
 * get error message text pointer
 * move up to 'errbuf_size' bytes of error message text into errbuf
 * close catalog
 * return true length of error message text
 */
	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
	pmsg = catgets(catd, MS_REG, errcode, etable[erroff]);
	msglen = strlen(pmsg) + 1;
	if (errbuf_size > 0)
		if (msglen <= errbuf_size)
			strcpy(errbuf, pmsg);
		else
			{
			strncpy(errbuf, pmsg, errbuf_size-1);
			errbuf[errbuf_size-1] = '\0';
			}
	catclose(catd);
	return (msglen);
}


/*
 * FUNCTION: regerror()
 *
 * DESCRIPTION: fetch message text of regcomp() or regexec() error
 *	        invoke appropriate method for this locale.
 */

size_t 
regerror(int errcode, const regex_t *preg, 
		char *errbuf, size_t errbuf_size)
{
	if (METHOD(__lc_collate,regerror) == NULL)
		return __regerror_std(errcode, preg, 
					errbuf,errbuf_size, __lc_collate);
	else
		return METHOD(__lc_collate,regerror)( errcode, preg, 
					errbuf, errbuf_size, __lc_collate);
}

