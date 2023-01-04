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
 *	@(#)$RCSfile: stdlib.h,v $ $Revision: 4.3.10.4 $ (DEC) $Date: 1993/10/07 21:18:54 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: stdlib.h
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <standards.h>

/*
 *
 *      The ANSI standard requires that certain values be in stdlib.h.
 *      It also requires if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present. This header includes all the ANSI required entries.
 *
 */
/* FIXME - uncomment all "const" directive when we get the ANSI compiler */
#ifdef _ANSI_C_SOURCE

/*
 *      The following 3 definitions are included in <sys/types.h>.  They are
 *      also included here to comply with ANSI standards.
 */

#ifndef NULL
#define NULL    0L
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long  size_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T

#ifndef __WCHAR_T_LEN
#    define __WCHAR_T_LEN 4
#endif

#if __WCHAR_T_LEN == 1
    typedef unsigned char wchar_t;
#elif __WCHAR_T_LEN == 2
    typedef unsigned short wchar_t;
#else
    typedef unsigned int  wchar_t;
#endif /* __WCHAR_T_LEN == ?? */

#endif /* _WCHAR_T */

typedef struct div_t  {			/* struct returned from div	*/
	int quot;			/* quotient			*/
	int rem; } div_t;			/* remainder			*/

typedef struct ldiv_t  {		/* struct returned from ldiv	*/
	long int quot;			/* quotient			*/
	long int rem; } ldiv_t;		/* remainder			*/

#define EXIT_FAILURE   (1)		/* exit function failure	*/
#define EXIT_SUCCESS	0		/* exit function success	*/

#ifdef _BSD
#define RAND_MAX	2147483647	/* max value returned by rand	*/
#else
#define RAND_MAX	32767		/* max value returned by rand	*/
#endif

/* Some header files define abs. If defined, undef to prevent syntax error */
#ifdef abs				
#undef abs
#endif

#define MB_CUR_MAX	__getmbcurmax()	/* max bytes in multibyte char	*/
#if defined(__cplusplus)
extern "C"
{
#endif
extern int __getmbcurmax __((void));

/**********
** Functions that are methods
**********/
extern int	mblen __((const char *, size_t));
extern size_t	mbstowcs __((wchar_t *, const char *, size_t));
extern int	mbtowc __((wchar_t *, const char *, size_t));
extern size_t	wcstombs __((char *, const wchar_t *, size_t));
extern int	wctomb __((char *, wchar_t));
#ifdef _OSF_SOURCE
extern int	rpmatch __((const char *));
extern void 	*valloc __((register size_t ));
#endif /*_OSF_SOURCE */

extern double 	atof __((const char *));
extern int 	atoi __((const char *));
extern long int atol __((const char *));
extern double 	strtod __((const char *, char **));
extern long int strtol __((const char *, char **, int ));
extern unsigned long int strtoul __((const char *, char **, int ));
extern int 	rand __((void));
extern void	srand __((unsigned int ));
extern void 	*calloc __((size_t , size_t ));
extern void	free __((void *));
extern void	*malloc __((size_t ));
extern void 	*realloc __((void *, size_t ));
extern void	abort __((void));
extern int	atexit __((void (*)(void)));
extern void	exit __((int ));
extern char	*getenv __((const char *));
extern int 	system __((const char *));
extern void 	*bsearch __((const void *, const void *, size_t , size_t , int(*)(const void *, const void *)));
extern void 	qsort __((void *, size_t , size_t ,int(*)(const void *, const void *)));
extern int 	abs __((int ));
extern struct div_t	div __((int , int ));
extern long int	labs __((long int ));
extern struct ldiv_t 	ldiv __((long int , long int ));

#ifdef AES_SOURCE

extern int	setenv __((const char *, const char *, int));
extern void	unsetenv __((const char *));

#endif /* AES_SOURCE */

#if defined(_REENTRANT) || defined(_THREAD_SAFE)

extern int	rand_r __((unsigned int *, int *));

#endif	/* _REENTRANT || _THREAD_SAFE */
#if defined(__cplusplus)
}
#endif

#ifdef _INTRINSICS
#pragma intrinsic(abs, labs)
#endif
#endif /*_ANSI_C_SOURCE */

#ifdef _XOPEN_SOURCE

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
struct drand48_data {
	unsigned	x[3];		/* 48 bit integer value */
	unsigned	a[3];		/* mutiplier value */
	unsigned	c;		/* addend value */
	unsigned short	lastx[3];	/* previous value of Xi */
	int		init;		/* initialize ? */
};
#endif	/* _REENTRANT || _THREAD_SAFE */

#if defined(__cplusplus)
extern "C"
{
#endif
extern double	drand48 __((void));
extern double	erand48 __((unsigned short []));
extern long	jrand48 __((unsigned short []));
extern void	lcong48 __((unsigned short []));
extern long	lrand48 __((void));
extern long	mrand48 __((void));
extern long	nrand48 __((unsigned short []));
extern unsigned short *seed48 __((unsigned short []));
extern void	srand48 __((long));
extern int 	putenv __((const char *));
extern void	setkey __((const char *));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int	drand48_r __((struct drand48_data *, double *));
extern int	erand48_r __((unsigned short [],struct drand48_data *,double*));
extern int	lrand48_r __((struct drand48_data *, long *));
extern int	mrand48_r __((struct drand48_data *, long *));
extern int	srand48_r __((long, struct drand48_data *));
extern int	seed48_r __((unsigned short [], struct drand48_data *));
extern int	lcong48_r __((unsigned short [], struct drand48_data *));
extern int	nrand48_r __((unsigned short [], struct drand48_data *, long*));
extern int	jrand48_r __((unsigned short [], struct drand48_data *, long*));
#endif	/* _REENTRANT || _THREAD_SAFE */
#if defined(__cplusplus)
}
#endif

#endif  /* _XOPEN_SOURCE */

#ifdef __MATH__
#    define abs(j)          __abs(j)
#    define labs(j)         __labs(j)
#endif /* ifdef __MATH__ */

/*
 * Definition of functions and structures used by the thread-safe
 * random_r() functions.
 */
#ifdef _OSF_SOURCE

#if defined(__cplusplus)
extern "C"
{
#endif
extern int	srandom __((unsigned int));
extern char	*initstate __((unsigned int, char *, int));
extern char	*setstate __((char *));
extern int	random __((void));
#if defined(__cplusplus)
}
#endif

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
struct random_data {
	long	*fptr;
	long	*rptr;
	long	*state;
	int	rand_type;
	int	rand_deg;
	int	rand_sep;
	long	*end_ptr;
};

/* functions */

#if defined(__cplusplus)
extern "C"
{
#endif
extern int	srandom_r __((unsigned, struct random_data *));
extern int	initstate_r __((unsigned, char *, int, char **, \
				struct random_data *));
extern int	setstate_r __((char *, char **, struct random_data *));
extern int	random_r __((long *, struct random_data *));
#if defined(__cplusplus)
}
#endif

#endif	/* _REENTRANT || _THREAD_SAFE */

#endif /* _OSF_SOURCE */

#ifdef _AES_SOURCE

/* optarg, optind, and opterr declared in stdlib (AES) and stdio (XPG3) */
#include <getopt.h>

#if defined(__cplusplus)
extern "C"
{
#endif
extern int 	clearenv __(());
extern char 	*getpass __((const char *));
#if defined(__cplusplus)
}
#endif

#endif  /* _AES_SOURCE */

/*
 *
 *   The following function prototypes are not defined for programs
 *   that are adhering to strict ANSI conformance, but are included
 *   for floating point support.
 *
 */

#ifdef _OSF_SOURCE
#include <sys/types.h>
     float    atoff();
char	*ecvt __((double, int, int *, int *));
char	*fcvt __((double, int, int *, int *));
char	*gcvt __((double, int, char *));
#if defined(_REENTRANT) || defined(_THREAD_SAFE)
int	ecvt_r __((double, int, int *, int *, char *, int));
int	fcvt_r __((double, int, int *, int *, char *, int));
#endif	/* _REENTRANT || _THREAD_SAFE */
     float    strtof();
     void     imul_dbl();
     void     umul_dbl();

#endif /* _OSF_SOURCE */
#endif /* _STDLIB_H_ */
