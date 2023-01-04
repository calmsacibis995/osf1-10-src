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
 * @(#)$RCSfile: glob.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 01:01:39 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#ifndef _H_GLOB
#define _H_GLOB

#include <standards.h>
/*
 * POSIX implies that you don't have to include <sys/types.h> with glob.h.
 * (compare with regex.h, where explicit decl is required....)
 */
#ifndef _SIZE_T
#define	_SIZE_T
typedef unsigned long size_t;
#endif

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
 * com/inc/glob.h, bos320 2/26/91 17:33:08 
 */

/* glob() flags */

#define GLOB_APPEND	0x01	/* append to end of gl_pathv		*/
#define GLOB_DOOFFS	0x02	/* first allocate gl_offs NULL ptrs	*/
#define GLOB_ERR	0x04	/* return on error			*/
#define GLOB_MARK	0x08	/* add / to end of directory name	*/
#define GLOB_NOCHECK	0x10	/* return pattern if no matches		*/
#define GLOB_NOSORT	0x20	/* do not sort matched filenames	*/
#ifdef _OSF_SOURCE		/* Deprecate - GLOB_QUOTE is now default*/
#define GLOB_QUOTE	0x40	/* <backslash> protects next character	*/
#endif
#define GLOB_NOESCAPE	0x80	/* disable backlash escaping		*/

/* Pathname Matching error codes - large so not confused with errno.h	*/

#define GLOB_ABORTED	0x1000	/* error detected			*/
#define GLOB_NOSPACE	0x2000	/* memory allocation failure		*/
#define GLOB_NOMATCH	0x4000	/* do not match pathname		*/

#ifdef _XOPEN_SOURCE
#define GLOB_NOSYS	-1	/* function not implemented		*/
#endif /* _XOPEN_SOURCE */

typedef struct {
	size_t	gl_pathc;	/* matched pathname count (not gl_offs)	*/
	char	**gl_pathv;	/* ptr to list of matched pathnames	*/
	size_t	gl_offs;	/* # of gl_pathv reserved slots		*/
	void	*gl_padr;	/* ptr to pathname address structure	*/
	void	*gl_ptx;	/* ptr to first pathname text buffer	*/
} glob_t;

/* Pathname Matching function prototypes */

#if defined(__cplusplus)
extern "C" {
#endif
extern	int	glob __((const char *, int, int(*)(const char *,int), glob_t *));
extern	void	globfree __((glob_t *));
#if defined(__cplusplus)
}
#endif

#endif /* _H_GLOB */
