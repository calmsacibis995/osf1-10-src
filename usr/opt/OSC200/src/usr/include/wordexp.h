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
 * @(#)$RCSfile: wordexp.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 01:15:30 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#ifndef _H_WORDEXP
#define _H_WORDEXP
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Matching
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.1  com/inc/wordexp.h, bos, bos320 2/26/91 17:33:33 
 */

#include <standards.h>

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long size_t;
#endif

/* wordexp() flags */

#define WRDE_APPEND	0x01	/* append to end of we_wordv		*/
#define WRDE_DOOFFS	0x02	/* first allocate we_offs NULL ptrs	*/
#define WRDE_NOCMD	0x04	/* error on command substitution	*/
#define WRDE_REUSE	0x08	/* first call wordfree()		*/
#define WRDE_SHOWERR	0x10	/* do not redirect stderr to /dev/null	*/
#define WRDE_UNDEF	0x20	/* undefined shell variable is error	*/

/* Word Expansion error codes */

#define WRDE_BADCHAR	1	/* unquoted special character		*/
#define WRDE_BADVAL	2	/* undefined shell variable		*/
#define WRDE_CMDSUB	3	/* command substitution not permitted	*/
#define WRDE_NOSPACE	4	/* memory allocation failure		*/
#define WRDE_SYNTAX	5	/* shell syntax error			*/
#define WRDE_EPOPEN	6	/* popen() failure			*/
#define WRDE_ESHELL	7	/* error reading psh data		*/

#ifdef _XOPEN_SOURCE
#define WRDE_NOSYS	8	/* function not supported		*/
#endif

typedef struct {
	size_t	we_wordc;	/* expanded word count (not we_offs)	*/
	char	**we_wordv;	/* ptr to list of expanded words	*/
	size_t	we_offs;	/* # of we_wordv reserved slots		*/
	int	we_sflags;	/* saved flags for wordfree()		*/
	size_t	we_soffs;	/* saved we_offs for wordfree()		*/
} wordexp_t;

/* Word Expansion function prototypes */

#if defined(__cplusplus)
extern "C" {
#endif
extern	int	wordexp __((const char *, wordexp_t *, int));
extern	void	wordfree __((wordexp_t *));
#if defined(__cplusplus)
}
#endif


#endif /* _H_WORDEXP */
