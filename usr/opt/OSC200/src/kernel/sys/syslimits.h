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
 *	@(#)$RCSfile: syslimits.h,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/11/02 16:19:45 $
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
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

#ifndef	_SYS_SYSLIMITS_H_
#define _SYS_SYSLIMITS_H_

#define	ARG_MAX		38912	/* max bytes for an exec function */
#if	defined(multimax) || defined(balance)
#define CHILD_MAX	100     /* max processes per user */
#else	/* defined(multimax) || defined(balance) */
#define CHILD_MAX	64	/* max processes per user */
#endif	/* defined(multimax) || defined(balance) */
#define	LINK_MAX	32767	/* max file link count */
#define	MAX_CANON	255	/* max bytes in terminal canonical input line */
#define	MAX_INPUT	255	/* max bytes in terminal input */
#define	NAME_MAX	255	/* max number of bytes in a file name */
#define	NGROUPS_MAX	32	/* max number of supplemental group id's */
#define SSIZE_MAX       LONG_MAX /* Max value fitting ssize_t (a long) */
#define TZNAME_MAX	255

#if !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
#define	OPEN_MAX	64	/* max open files per process - OBSOLETE, sysconf() interface should be used */
#endif

#define	PATH_MAX	1023	/* max number of bytes in pathname */
#define	PIPE_BUF	4096	/* max number of bytes for atomic pipe writes */

#endif	/* _SYS_SYSLIMITS_H_ */
