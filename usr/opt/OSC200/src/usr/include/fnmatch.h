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
/*
 * @(#)$RCSfile: fnmatch.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:55:08 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#ifndef _H_FNMATCH
#define _H_FNMATCH

#include <standards.h>

/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Matching
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * com/inc/fnmatch.h, bos320 2/26/91 17:32:59 
 */

/*
 * values for the fnmatch() flags variable
 */

#define FNM_PATHNAME	1	/* Slash in string only matches slash in pattern */
#define FNM_PERIOD	2	/* Leading period must be exactly matched */
#ifdef _OSF_SOURCE
#define FNM_QUOTE	4	/* Deprecate - removed after POSIX D11 */
#endif				/* FNM_QUOTE is 'opposite' of FNM_NOESCAPE */
#define FNM_NOESCAPE	8	/* Don't treat backslash as an escape */


/* Pattern Matching error codes */

#define	FNM_NOMATCH	1	/* pattern does not match string	*/
#define FNM_ESLASH	2	/* * ? [] can't match /			*/
#define FNM_EPERIOD	3	/* * ? [] can't match .			*/

#ifdef _XOPEN_SOURCE
#define FNM_NOSYS	4	/* no support for fnmatch() */		*/
#endif /* _XOPEN_SOURCE */

/* Pattern Matching function prototypes */

#if defined(__cplusplus)
extern "C" {
#endif
extern	int	fnmatch __((const char *, const char *, int));
#if defined(__cplusplus)
}
#endif

#endif /* _H_FNMATCH */
