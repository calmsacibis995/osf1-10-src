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
 *	@(#)$RCSfile: wchar.h,v $ $Revision: 4.2.5.7 $ (DEC) $Date: 1993/09/30 20:42:22 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* 1.5  com/inc/wchar.h, 9123320, bos320 5/16/91 09:44:57 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _WCHAR_H_
#define _WCHAR_H_

#include <standards.h>
#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#ifndef WEOF
#define WEOF	(wint_t)(-1)
#endif /* WEOF */

#if defined(__cplusplus)
extern "C" {
#endif

extern int	iswalpha __((wint_t));
extern int	iswalnum __((wint_t));
extern int	iswcntrl __((wint_t));
extern int	iswdigit __((wint_t));
extern int	iswgraph __((wint_t));
extern int	iswlower __((wint_t));
extern int	iswprint __((wint_t));
extern int	iswpunct __((wint_t));
extern int	iswspace __((wint_t));
extern int	iswupper __((wint_t));
extern int	iswxdigit __((wint_t));
extern wint_t	towupper __((wint_t));
extern wint_t	towlower __((wint_t));
extern int	iswctype __((wint_t, wctype_t));
extern wctype_t	wctype __((char *));

#ifdef _OSF_SOURCE

#include <ctype.h>

#define iswalpha(c)	_ISWMACRO(c,_ISALPHA)
#define iswupper(c)	_ISWMACRO(c,_ISUPPER)
#define iswlower(c)	_ISWMACRO(c,_ISLOWER)
#define iswdigit(c)	_ISWMACRO(c,_ISDIGIT)
#define iswxdigit(c)	_ISWMACRO(c,_ISXDIGIT)
#define iswspace(c)	_ISWMACRO(c,_ISSPACE)
#define iswpunct(c)	_ISWMACRO(c,_ISPUNCT)
#define iswalnum(c)	_ISWMACRO(c,_ISALNUM)
#define iswprint(c)	_ISWMACRO(c,_ISPRINT)
#define iswgraph(c)	_ISWMACRO(c,_ISGRAPH)
#define iswcntrl(c)	_ISWMACRO(c,_ISCNTRL)

#define towupper(c)	_TOWUPPER(c)
#define towlower(c)	_TOWLOWER(c)

#endif

extern wint_t	fgetwc __((FILE *));
extern wchar_t  *fgetws __((wchar_t *, int, FILE *));
extern wint_t	fputwc __((wint_t,FILE *));
extern int	fputws __((const wchar_t *, FILE *));
extern wint_t	getwc __((FILE *));
extern wint_t	getwchar __((void));

#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef getwchar
#undef getwchar
#endif
#endif
#define getwchar()	getwc(stdin)

extern wint_t	putwc __((wint_t, FILE *));
extern wint_t	putwchar __((wint_t));
extern wint_t	ungetwc __((wint_t, FILE *));
extern double	wcstod __((const wchar_t *, wchar_t **));
extern long int	wcstol __((const wchar_t *, wchar_t **, int));
extern unsigned long int wcstoul __((const wchar_t *, wchar_t **, int));
extern wchar_t	*wcscat __((wchar_t *, const wchar_t *));
extern wchar_t	*wcschr __((const wchar_t *, wchar_t));
extern int	wcscmp __((const wchar_t *, const wchar_t *));
extern int	wcscoll __((const wchar_t *, const wchar_t *));
extern wchar_t	*wcscpy __((wchar_t *, const wchar_t *));
extern size_t	wcscspn __((const wchar_t *, const wchar_t *));
extern size_t	wcslen __((const wchar_t *));
extern wchar_t	*wcsncat __((wchar_t *, const wchar_t *, size_t));
extern int	wcsncmp __((const wchar_t *, const wchar_t *, size_t));
extern wchar_t	*wcsncpy __((wchar_t *, const wchar_t *, size_t));
extern wchar_t	*wcspbrk __((const wchar_t *, const wchar_t *));
extern wchar_t	*wcsrchr __((const wchar_t *, wchar_t));
extern size_t	wcsspn __((const wchar_t *, const wchar_t *));
extern wchar_t	*wcstok __((wchar_t *, const wchar_t *));
extern wchar_t	*wcswcs __((const wchar_t *, const wchar_t *));
extern int	wcswidth __((const wchar_t *, size_t));
extern size_t	wcsxfrm __((wchar_t *, const wchar_t *, size_t));
extern int	wcwidth __((const wchar_t));
extern size_t	wcsftime __((wchar_t *, size_t, const char *, const struct tm *));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern wchar_t	*wcstok_r __((wchar_t *, const wchar_t *, wchar_t **));
#endif	/* _REENTRANT || _THREAD_SAFE */

#if defined(__cplusplus)
}
#endif

#endif /* _WCHAR_H_ */




