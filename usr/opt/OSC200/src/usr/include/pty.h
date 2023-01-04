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
 * @(#)$RCSfile: pty.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 16:21:23 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
#ifndef _PTY_H_
#define _PTY_H_

#include <termios.h>
#include <sys/ioctl.h>
#include <paths.h>	/* for _PATH_PTY */

#define MAX_PTYNAME_LEN		sizeof("/dev/pts/XXXXXXXXXX")

#define PTY_MASTER_DEV		"/dev/ptmx_bsd" /* BSD pty master device */
#define PTY_MASTER_DEV_SYSV	"/dev/ptmx"	/* System V pty master device*/

extern int openpty __((int *, int *, char *, struct termios *, struct winsize *));
extern int forkpty __((int *, char *, struct termios *, struct winsize *));
extern int pty_master __(());
extern int pty_slave __((int));

#endif /* _PTY_H_ */
