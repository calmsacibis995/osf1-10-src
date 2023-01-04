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
 *	@(#)$RCSfile: string.h,v $ $Revision: 4.3.7.7 $ (DEC) $Date: 1993/12/15 22:14:24 $
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <standards.h>
#include <sys/types.h>

/*
 *
 *      The ANSI standard requires that certain values be in string.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present. This header includes all the ANSI required entries.
 *
 */

/* FIXME - uncomment all "const" directive when we get the ANSI compiler */

#ifdef   _ANSI_C_SOURCE

/*
 *      The following definitions (NULL, size_t) are included in <sys/types.h>.
 *      They are also included here to comply with ANSI standards.
 */

#ifndef NULL
#define NULL    0L
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long   size_t;
#endif

#ifdef	_NONSTD_TYPES
extern char	*memchr();
extern char	*memcpy();
extern char	*memset();
extern int	strcspn();
extern int	strlen();
extern int	strspn();
#elif	defined	_NO_PROTO
extern void	*memchr();
extern void	*memcpy();
extern void	*memset();
extern size_t 	strcspn();
extern size_t	strlen();
extern size_t	strspn();
#else	/* _NONSTD_TYPES, _NO_PROTO */
_BEGIN_CPLUSPLUS
extern void	*memchr(const void *, int , size_t );
extern void	*memcpy(void *, const void *, size_t );
extern void	*memset(void *, int , size_t );
extern size_t 	strcspn(const char *, const char *);
extern size_t	strlen(const char *);
extern size_t	strspn(const char *, const char *);
_END_CPLUSPLUS
#endif	/* _NONSTD_TYPES, _NO_PROTO */


#ifdef   _NO_PROTO
extern void 	*memmove();
extern char 	*strcpy();
extern char 	*strncpy();
extern char 	*strcat();
extern char	*strncat();
extern int 	memcmp();
extern int	strcmp();
extern int	strcoll();
extern int	strncmp();
extern size_t 	strxfrm();
extern char	*strchr();
extern char	*strpbrk();
extern char	*strrchr();
extern char	*strstr();
extern char 	*strtok();
extern char 	*strerror();

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern char 	*strtok_r();	/* _POSIX_REENTRANT_FUNCTIONS */
extern int 	strerror_r();
#endif

#else  /*_NO_PROTO */
#if defined(__STDC__) || defined(__cplusplus)
_BEGIN_CPLUSPLUS
extern void 	*memmove(void *, const void *, size_t );
extern char 	*strcpy(char *, const char *);
extern char 	*strncpy(char *, const char *, size_t );
extern char 	*strcat(char *, const char *);
extern char	*strncat(char *, const char *, size_t );
extern int 	memcmp(const void *, const void *,size_t );
extern int	strcmp(const char *, const char *);
extern int	strcoll(const char *, const char *);
extern int	strncmp(const char *,const char *,size_t );
extern size_t 	strxfrm(char *, const char *, size_t );
extern char	*strchr(const char *, int );
extern char	*strpbrk(const char *, const char *);
extern char	*strrchr(const char *, int );
extern char	*strstr(const char *, const char *);
extern char 	*strtok(char *, const char *);
extern char 	*strerror(int);
#if defined(_REENTRANT) || defined(_THREAD_SAFE)
		/* _POSIX_REENTRANT_FUNCTIONS */
extern char 	*strtok_r(char *, char *, char **); 
extern int	strerror_r(int, char *, int);
#endif
_END_CPLUSPLUS
#endif
#endif /*_NO_PROTO */

#ifdef _INTRINSICS
#pragma intrinsic(strcpy)
#endif
#endif /*_ANSI_C_SOURCE */

#ifdef	_XOPEN_SOURCE

#ifdef	_NONSTD_TYPES
extern char	*memccpy();
#elif	defined	_NO_PROTO
extern void	*memccpy();
#else	/* _NONSTD_TYPES, _NO_PROTO */
_BEGIN_CPLUSPLUS
extern void	*memccpy(void *, const void *, int , size_t );
_END_CPLUSPLUS
#endif	/* _NONSTD_TYPES, _NO_PROTO */

#endif	/* _XOPEN_SOURCE */


#ifdef _AES_SOURCE

#ifdef _NO_PROTO
extern void	swab();
#else
#if defined(__STDC__) || defined(__cplusplus)
#if !defined(__cplusplus) || !defined(_UNISTD_H_)
_BEGIN_CPLUSPLUS
extern void	swab(const void *, void *, ssize_t);
_END_CPLUSPLUS
#endif /* !__cplusplus || (__cplusplus && !_UNISTD_H_) */
#endif /* __STDC__ || __cplusplus */
#endif /* _NO_PROTO */

#endif  /* _AES_SOURCE */

#ifdef _OSF_SOURCE

#ifdef _NO_PROTO
extern int  ffs();
extern void  bzero();
extern int  bcmp();
extern void bcopy();
extern char *basename();
extern char *dirname();

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int  dirname_r ();
#endif  /* _REENTRANT || _THREAD_SAFE */

extern int  strcasecmp();
extern int  strncasecmp();
extern char *strdup();
extern char *index();
extern char *rindex();
#else
#if defined(__STDC__) || defined(__cplusplus)
_BEGIN_CPLUSPLUS
extern int  ffs(long);
extern void  bzero(char *, int);
extern int  bcmp(const char *, const char *, int);
extern void bcopy(const char *, char *, int);
extern char *strdup(const char *);
extern int  strcasecmp(const char *, const char *);
extern int  strncasecmp(const char *, const char *, size_t );
extern char *index(const char*, char);
extern char *rindex(const char*, char);
extern char *basename(char *);
extern char *dirname(char *);

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int  dirname_r (char *, char *, int);
#endif  /* _REENTRANT || _THREAD_SAFE */

_END_CPLUSPLUS
#endif
#endif  /* _NO_PROTO */

#endif /* _OSF_SOURCE */

#endif /* _STRING_H_ */
