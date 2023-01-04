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
 *	@(#)$RCSfile: pathnames.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:15:05 $
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
 * Copyright (c) 1989 The Regents of the University of California.
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


#define	_PATH_BOOT	"/etc/named.boot"

#if defined(BSD) && BSD >= 198810
#include <paths.h>
#define	_PATH_XFER	"/usr/sbin/named-xfer"
#define	_PATH_DEBUG	"/var/tmp/named.run"
#define	_PATH_DUMPFILE	"/var/tmp/named_dump.db"
#define	_PATH_PIDFILE	"/var/run/named.pid"
#define	_PATH_STATS	"/var/tmp/named.stats"
#define	_PATH_TMPXFER	"/var/tmp/xfer.ddt.XXXXXX"
#define	_PATH_TMPDIR	"/var/tmp"

#else /* BSD */
#define	_PATH_DEVNULL	"/dev/null"
#define	_PATH_TTY	"/dev/tty"
#define	_PATH_XFER	"/etc/named-xfer"
#define	_PATH_DEBUG	"/usr/tmp/named.run"
#define	_PATH_DUMPFILE	"/usr/tmp/named_dump.db"
#define	_PATH_PIDFILE	"/etc/named.pid"
#define	_PATH_STATS	"/usr/tmp/named.stats"
#define	_PATH_TMPXFER	"/usr/tmp/xfer.ddt.XXXXXX"
#define	_PATH_TMPDIR	"/usr/tmp"
#endif /* BSD */
