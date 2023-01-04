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
static char *rcsid = "@(#)$RCSfile: regfree.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 21:39:44 $";
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
 * FUNCTIONS: regfree
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
 * 1.3  com/lib/c/pat/regfree.c, libcpat, bos320, 9130320 7/17/91 15:28:01
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak regfree = __regfree
#endif
#endif
#include <sys/localedef.h>
#include <regex.h>
#include <unistd.h>

/************************************************************************/
/* __regfree_std() - Free Compiled RE Pattern Dynamic Memory		*/
/************************************************************************/

void
__regfree_std( regex_t *preg, _LC_collate_t *phdl)
{
	if (preg->re_comp != NULL)
		{
		free(preg->re_comp);
		preg->re_comp = NULL;
		}
	return;
}


/*
 * FUNCTION:  regfree()
 *
 * DESCRIPTION: release preg memory malloc'd by regcomp()
 *              invoke appropriate method for this locale
*/

void 
regfree(regex_t *preg)
{
	if (METHOD(__lc_collate, regfree) == NULL)
		__regfree_std( preg, __lc_collate);
	else
		METHOD(__lc_collate, regfree)( preg, __lc_collate);
}
