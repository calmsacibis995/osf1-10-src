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
 *       @(#)$RCSfile: stdarg.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/04/15 08:10:59 $
 */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/PMAX/stdarg.h,v 4.2.4.3 1992/04/15 08:10:59 Al_Delorey Exp $ */

/* 4.8 Variable arguments */

#ifndef _STDARG_H
#define _STDARG_H

#ifndef _SYSTYPE_SVR4

#ifndef _VA_LIST
#define _VA_LIST
typedef char *va_list;
#endif  /* _VA_LIST */

#define va_end(list)

#ifdef __host_mips__

#if defined(_CFE)
#if defined(__STDC__) && (__STDC__ != 0 )
	/* va_start makes list point past the parmN */
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))
#else
#define va_start(list, name) (void) (list = (void *)((char *)&...))
#endif
#else
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))
#endif
        /* va_arg aligns list and points past data */
#define va_arg(list, mode) ((mode *)(list =\
 (char *) ((((int)list + (__builtin_alignof(mode)<=4?3:7)) &\
 (__builtin_alignof(mode)<=4?-4:-8))+sizeof(mode))))[-1]

/*  +++++++++++++++++++++++++++++++++++++++++++
    Because of parameter passing conventions in C:
    use mode=int for char, and short types
    use mode=double for float types
    use a pointer for array types
    +++++++++++++++++++++++++++++++++++++++++++ */


#endif  /* __host_mips__ */

#else   /* _SYSTYPE_SVR4 */
#if (defined(__STDC__) || defined (__SVR4__STDC))

#ifndef _VA_LIST
#define _VA_LIST
typedef void *va_list;
#endif  /* _VA_LIST */

#define va_end(list)

#ifdef __host_mips__

#if defined(_CFE)
#if defined(__STDC__) && (__STDC__ != 0 )
	/* va_start makes list point past the parmN */
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))
#else
#define va_start(list, name) (void) (list = (void *)((char *)&...))
#endif
#else
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))
#endif
        /* va_arg aligns list and points past data */
#define va_arg(list, mode) ((mode *)(list =\
 (char *) ((((int)list + (__builtin_alignof(mode)<=4?3:7)) &\
 (__builtin_alignof(mode)<=4?-4:-8))+sizeof(mode))))[-1]

/*  +++++++++++++++++++++++++++++++++++++++++++
    Because of parameter passing conventions in C:
    use mode=int for char, and short types
    use mode=double for float types
    use a pointer for array types
    +++++++++++++++++++++++++++++++++++++++++++ */

#endif  /* __host_mips__ */

#else	/* not __STDC__ */
#include <varargs.h>
#endif	/* __STDC__ */

#endif  /* _SYSTYPE_SVR4 */
#endif  /* _STDARG_H */
