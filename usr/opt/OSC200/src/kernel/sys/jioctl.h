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
 *	@(#)$RCSfile: jioctl.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:58:13 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	Unix to Blit I/O control codes
 */

#ifndef	_SYS_JIOCTL_H_
#define _SYS_JIOCTL_H_

#include <sys/ioctl.h>

#define JSMPX		TIOCUCNTL
#define JMPX		_IO(u,0)
#define JBOOT		_IO(u, 1)
#define JTERM		_IO(u, 2)
#define JTIMO		_IO(u, 4)	/* Timeouts in seconds */
#define JTIMOM		_IO(u, 6)	/* Timeouts in millisecs */
#define JZOMBOOT	_IO(u, 7)
#define JWINSIZE	TIOCGWINSZ
#define JSWINSIZE	TIOCSWINSZ

/**	Channel 0 control message format **/

struct jerqmesg
{
	char	cmd;		/* A control code above */
	char	chan;		/* Channel it refers to */
};

/*
**	Character-driven state machine information for Blit to Unix communication.
*/

#define C_SENDCHAR	1	/* Send character to layer process */
#define C_NEW		2	/* Create new layer process group */
#define C_UNBLK		3	/* Unblock layer process */
#define C_DELETE	4	/* Delete layer process group */
#define C_EXIT		5	/* Exit */
#define C_BRAINDEATH	6	/* Send terminate signal to proc. group */
#define C_SENDNCHARS	7	/* Send several characters to layer proc. */
#define C_RESHAPE	8	/* Layer has been reshaped */
#define C_JAGENT	9	/* Jagent return (What do they mean? */

/*
 * Map to new window structure
 */
#define bitsx	ws_xpixel
#define bitsy	ws_ypixel
#define bytesx	ws_col
#define bytesy	ws_row
#define jwinsize winsize

/*
**	Usual format is: [command][data]
*/

#endif	/* _SYS_JIOCTL_H_ */
