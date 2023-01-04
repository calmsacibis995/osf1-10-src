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
 *	@(#)$RCSfile: paths.h,v $ $Revision: 4.3.8.3 $ (DEC) $Date: 1993/06/07 19:42:13 $
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
 * Modification History:
 *
 * 01 - gaudet Thu Oct 10 09:57:21 EDT 1991
 *	changed _PATH_MAILDIR from "/var/mail" to "/var/spool/mail"
 */

#define	_PATH_BSHELL	"/bin/sh"
#define	_PATH_CONSOLE	"/dev/console"
#define	_PATH_CSHELL	"/bin/csh"
#define	_PATH_DEV	"/dev/"
#define	_PATH_DEVNULL	"/dev/null"
#define	_PATH_DRUM	"/dev/drum"
#define	_PATH_KMEM	"/dev/kmem"
#define	_PATH_KSH	"/bin/ksh"
#define _PATH_MAILDIR	"/var/spool/mail"
#define _PATH_MAN       "/usr/share/man"
#define	_PATH_MEM	"/dev/mem"
#define _PATH_NOLOGIN   "/etc/nologin"
#define	_PATH_SENDMAIL	"/usr/sbin/sendmail"
#define	_PATH_TMP	"/tmp/"
#define	_PATH_TTY	"/dev/tty"
#define	_PATH_UNIX	"/vmunix"
#define _PATH_VARTMP    "/var/tmp"
#define	_PATH_VI	"/usr/bin/vi"
#define _PATH_PTY	"/dev/pts/"
#define _PATH_PTYP	"/dev/ptyp"
#define _PATH_LOGIN	"/usr/bin/login"
