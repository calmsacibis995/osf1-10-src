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
/* strings.h - define standard string functions */
/* @(#)$RCSfile: strings.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/12/21 21:38:19 $ devrcs Exp Locker: devbld $ */

#ifndef	_STRINGS		/* once-only... */
#define	_STRINGS

#ifdef	SYS5
#define	index	strchr
#define	rindex	strrchr
#endif	/* SYS5 */

char   *index ();
char   *mktemp ();
char   *rindex ();

/* #ifndef _BSD   tk0001 6/6/91 */
/* #ifndef	SPRINTFTYPE  */
/* char   *sprintf ();	/*	 I guess this is the new standard */
/* #else  */
/* SPRINTFTYPE sprintf ();  */
/* #endif  */
/* #endif _BSD  */

char   *strcat ();
int     strcmp ();
char   *strcpy ();
int	strlen ();
char   *strncat ();
int     strncmp ();
char   *strncpy ();
char   *getenv ();
#ifndef OSF   /* use declarations in stdlib.h */
char   *calloc (), *malloc (), *realloc ();
#endif

#ifdef	SYS5
#include <memory.h>
#define bcmp(b1,b2,length)	memcmp(b1, b2, length)
#define	bcopy(b1,b2,length)	(void) memcpy (b2, b1, length)
#define	bcpy(b1,b2,length)	memcmp (b1, b2, length)
#define	bzero(b,length)		(void) memset (b, 0, length)
#endif	/* SYS5 */

#endif	/* not _STRINGS */
