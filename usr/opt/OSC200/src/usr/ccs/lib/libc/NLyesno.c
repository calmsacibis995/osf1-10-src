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
static char	*sccsid = "@(#)$RCSfile: NLyesno.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:19:28 $";
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
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLyesno, scan
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * NLyesno.c	1.7  com/lib/c/nls,3.1,9013 2/13/90 16:00:25
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NLyesno = __NLyesno
#endif
#endif
#include <stdlib.h> /* rpmatch() */

/*
 * NAME: NLyesno
 *
 * FUNCTION: Determine "yes" or "no" response to prompts.
 *
 * (EXECUTION ENVIRONMENT:)
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : Yes
 *      VMM Critical Region: No
 *      Runs on Fixed Stack: No
 *      May Page Fault     : Yes
 *      May Backtrack      : Yes
 *
 * (NOTES:)
 *
 *      Compare a string s against the substrings given in values
 *      of YESSTR and NOSTR.  If s matches an element of YESSTR,
 *      return 1 (true).  Otherwise, if s matches an element of
 *      NOSTR, return 0 (false).  Otherwise, return -1 as an
 *      error indication (which normally indicates that the
 *      calling code should prompt again for a proper response).
 *      Note that a null response is indicated in YESSTR or NOSTR
 *      by a leading or trailing colon or by two adjacent colons.
 *      The string s is considered a null response if it's first
 *      element is either '\0' or '\n'.
 *
 * (DATA STRUCTURES:) Effects on global data structures -- none.
 *
 * RETURN VALUE DESCRIPTION: 1 for YES response
 *                           0 for NO response
 *                          -1 for error condition
 */



int
NLyesno (s)
				 /*
				    s is the pointer to the
				    response given by the user.
				 */
register char *s;
{
    return rpmatch(s);
}
