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
 *	@(#)$RCSfile: search.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/08 01:08:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _SEARCH_H_
#define _SEARCH_H_

#include <standards.h>
#include <sys/types.h>

#ifdef _XOPEN_SOURCE

/* HSEARCH(3C) */
typedef struct entry { char *key, *data; } ENTRY;
typedef enum { FIND, ENTER } ACTION;

/* TSEARCH(3C) */
typedef enum { preorder, postorder, endorder, leaf } VISIT;

#ifdef	_NONSTD_TYPES
extern char *lsearch();
extern char *lfind();
extern char *tsearch();
extern char *tfind();
extern char *tdelete();

#elif	defined	_NO_PROTO

extern void *lsearch();
extern void *lfind();
extern void *tsearch();
extern void *tfind();
extern void *tdelete();

#else	/* _NONSTD_TYPES, _NO_PROTO */
#if defined(__cplusplus)
extern "C" {
#endif
extern void *lsearch(const void *, void *, size_t *, size_t, int (*) (const void *, const void *));
extern void *lfind(const void *, const void *, size_t *, size_t, int (*)(const void *, const void *));
extern void *tsearch(const void *, void **, int (*)(const void *,const void *));
extern void *tfind(const void *, void *const *, int (*) (const void *,const void *));
extern void *tdelete(const void *, void **, int (*)(const void *,const void *));

#if defined(__cplusplus)
}
#endif
#endif	/* _NONSTD_TYPES, _NO_PROTO */
 
#ifdef _NO_PROTO
extern int hcreate();
extern void hdestroy();
extern ENTRY *hsearch();
extern void twalk();

#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif

extern int hcreate(size_t);
extern void hdestroy(void);
extern ENTRY *hsearch(ENTRY, ACTION);
extern void twalk(const void *, void (*)(const void *,VISIT,int));

#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
struct hsearch_data {
	void	*table;
	uint_t	length;
	uint_t	dim;
	uint_t	prcnt;
	uint_t	count;
};

extern int hcreate_r (size_t, struct hsearch_data *);
extern void hdestroy_r (struct hsearch_data *);
extern int hsearch_r (ENTRY, ACTION,ENTRY **, struct hsearch_data *);

#endif  /* _REENTRANT || _THREAD_SAFE */

#endif /* _XOPEN_SOURCE */

#endif /* _SEARCH_H_ */
