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
 *       @(#)$RCSfile: varargs.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/01/28 20:02:31 $
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * varargs.h
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+. Replace OSF varargs.h with MIPS version.
 *
 */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
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
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/PMAX/varargs.h,v 4.2.4.2 1992/01/28 20:02:31 Al_Delorey Exp $ */

#ifdef __vax
/*    varargs.h       4.1     83/05/03        */

#ifndef       _VA_LIST
#define       _VA_LIST
typedef char *va_list;
#endif        /* _VA_LIST */
# define va_dcl int va_alist;
# define va_start(list) list = (char *) &va_alist
# define va_end(list)
# define va_arg(list,mode) ((mode *)(list += sizeof(mode)))[-1]
#endif /* __vax */

#ifdef __mips__
#if	!(defined(_VARARGS_) || defined(_STDDEF_H))
#define	_VARARGS_	1


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _VA_LIST
#define _VA_LIST
typedef char *va_list;
#endif  /* _VA_LIST */
#define va_dcl int va_alist;
#define va_start(list) list = (char *) &va_alist
#define va_end(list)
#ifdef __u370
#define va_arg(list, mode) ((mode *)(list = \
	(char *) ((int)list + 2*sizeof(mode) - 1 & -sizeof(mode))))[-1]
#else
#ifdef __host_mips__
#ifdef lint /* complains about constant in conditional context */
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]
#else /* !lint */
#ifndef MS_STAMP
#define ___INCLUDING___
#include <stamp.h>      /* get the version numbers */
#endif

#if MS_STAMP<2 || (MS_STAMP==2 && LS_STAMP<10)
#define va_arg(list, mode) ((mode *)(list = \
	(char *) (sizeof(mode) > 4 ? ((int)list + 2*8 - 1) & -8 \
				   : ((int)list + 2*4 - 1) & -4)))[-1]
#else
/* this works for structures also */
#define va_arg(list, mode) ((mode *)(list =\
 (char *) ((((int)list + (__builtin_alignof(mode)<=4?3:7)) &\
 (__builtin_alignof(mode)<=4?-4:-8))+sizeof(mode))))[-1]
/*  +++++++++++++++++++++++++++++++++++++++++++
    Because of parameter passing conventions in C:
    use mode=int for char, and short types
    use mode=double for float types
    use a pointer for array types
    +++++++++++++++++++++++++++++++++++++++++++ */

#endif

#ifdef ___INCLUDING___  /* did we include stamp.h? */
#undef _MS_STAMP         /* undo the definitions to clean up cpp */
#undef _LS_STAMP
#undef ___INCLUDING___
#endif

#endif /* lint */
#else
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]
#endif
#endif


#endif	/* _VARARGS_ */
#endif  /* __mips__ */
