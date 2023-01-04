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
 * @(#)$RCSfile: va_list.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/08 00:53:39 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/*
 * HISTORY
 * $OSF_Log:	va_list.h,v $
 * Revision 1.1.1.1  92/03/07  00:45:00  devrcs
 * *** OSF1_1B23 version ***
 * 
 * Revision 1.1.2.2  1991/11/14  13:10:39  tom
 * 	Initial version.
 * 	[91/11/14  13:02:50  tom]
 *
 * $OSF_EndLog$
 */
/* @(#)$RCSfile: va_list.h,v $ $Revision: 1.1.5.2 $ (OSF) $Date: 1993/06/08 00:53:39 $ */

/*
 * Four possible situations:
 * 	- We are being included by {var,std}args.h (or anyone) before stdio.h.
 * 	  define real type.
 *
 * 	- We are being included by stdio.h before {var,std}args.h.
 * 	  define hidden type for prototypes in stdio, don't pollute namespace.
 * 
 * 	- We are being included by {var,std}args.h after stdio.h.
 * 	  define real type to match hidden type.  no longer use hidden type.
 * 
 * 	- We are being included again after defining the real va_list.
 * 	  do nothing.
 * 
 */

#if	!defined(_HIDDEN_VA_LIST) && !defined(_VA_LIST)
#define _VA_LIST
typedef struct {
	char	*_a0;		/* pointer to first homed integer arg */
	int	_offset;		/* byte offset of next param */
} va_list;

#elif	defined(_HIDDEN_VA_LIST) && !defined(_VA_LIST)
#define _VA_LIST
typedef struct {
	char	*_a0;		/* pointer to first homed integer arg */
	int	_offset;		/* byte offset of next param */
} __va_list;

#elif	defined(_HIDDEN_VA_LIST) && defined(_VA_LIST)
#undef _HIDDEN_VA_LIST
typedef __va_list va_list;

#endif
