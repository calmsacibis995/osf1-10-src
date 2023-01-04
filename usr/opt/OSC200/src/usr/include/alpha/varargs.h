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
/* $Header: /usr/sde/osf1/rcs/os/src/usr/include/alpha/varargs.h,v 1.2.7.2 1993/06/08 00:53:46 Thomas_Woodburn Exp $ */

#ifndef _VARARGS_H_
#define _VARARGS_H_

#include <va_list.h>		/* defines va_list */

#define va_dcl long va_alist;
#define va_start(list) __builtin_va_start(list, va_alist, 0)
#define va_end(list)
#define va_arg(list, mode) \
	(*(((list)._offset += ((int)sizeof(mode) + 7) & -8), \
	    (mode *)((list)._a0 + (list)._offset - \
		((__builtin_isfloat(mode) && (list)._offset <= (6 * 8)) ? \
		(6 * 8) + 8 : ((int)sizeof(mode) + 7) & -8))))

#endif /* _VARARGS_H_ */
