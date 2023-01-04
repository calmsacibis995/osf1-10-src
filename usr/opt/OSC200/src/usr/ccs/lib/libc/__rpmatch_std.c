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
static char *rcsid = "@(#)$RCSfile: __rpmatch_std.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:22:05 $";
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
 * FUNCTIONS: __rpmatch_std
 *
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/lib/c/pat/__rpmatch_std.c, libcpat, bos320, 9132320m 8/11/91
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <regex.h>


/************************************************************************/
/* __rpmatch_std - determine if response matches yes or no pattern	*/
/*	       - works for all locales and all code sets		*/
/*	       - enforces requirement for pattern to be at beginning	*/
/*	       -   of response						*/
/************************************************************************/
int
__rpmatch_std(const char *response, _LC_resp_t *phdl)
{
	regex_t	reg;		/* regular expression status buffer	*/
	regmatch_t pmatch;	/* start/stop match offsets		*/

/*
 * check for positive response
 */
	if (phdl->yesexpr != (char *)0)
		if (regcomp(&reg, phdl->yesexpr, REG_EXTENDED) == 0)
			if ((regexec(&reg, response, (size_t)1, &pmatch, 0) == 0) &&
				(pmatch.rm_so == 0))
				{
				regfree(&reg);
				return (1);
				}
			else
				regfree(&reg);
/*
 * check for negative response
 */
	if (phdl->noexpr != (char *)0)
		if (regcomp(&reg, phdl->noexpr, REG_EXTENDED) == 0)
			if ((regexec(&reg, response, (size_t)1, &pmatch, 0) == 0) &&
				(pmatch.rm_so == 0))
				{
				regfree(&reg);
				return (0);
				}
			else
				regfree(&reg);
/*
 * response does not match either yes or no expression
 */
	return (-1);
}
