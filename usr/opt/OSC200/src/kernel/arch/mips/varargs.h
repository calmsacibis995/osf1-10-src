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
 *	@(#)$RCSfile: varargs.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:19:43 $
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

/*	varargs.h	4.1	83/05/03	*/

#include <sys/types.h>		/* for va_list */

#if	ibmrt
# define va_alist va_alist1, va_alist2, va_alist3, va_alist4
# define va_dcl int va_alist1, va_alist2, va_alist3, va_alist4;
# define va_start(list) list = (char *) &va_alist1
#else	/* ibmrt */
# define va_dcl int va_alist;
# define va_start(list) list = (char *) &va_alist
#endif	/* ibmrt */

# define va_end(list)

#ifdef	mips
# define va_arg(list, mode) ((mode *)(list = \
	(char *) (sizeof(mode) > 4 ? ((int)list + 2*8 - 1) & -8 \
				   : ((int)list + 2*4 - 1) & -4)))[-1]
#else	/* mips */
#if	BYTE_MSF
# define va_arg(list,mode) ((mode *)((list += (sizeof(mode)+3)&(-4))\
		-((sizeof(mode)<4)?sizeof(mode):(sizeof(mode)+3)&(-4))))[0]
#else	/* BYTE_MSF */
# define va_arg(list,mode) ((mode *)((list += (sizeof(mode)+3)&(-4))\
		-((sizeof(mode)+3)&(-4))))[0]
#endif	/* BYTE_MSF */
#endif	/* mips */
