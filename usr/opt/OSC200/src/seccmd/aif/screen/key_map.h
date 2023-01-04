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
 *	@(#)$RCSfile: key_map.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:09 $
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
#ifdef SEC_BASE
#ifndef _KEY_MAP_H
#define _KEY_MAP_H

/*
 * Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
 */



/* This file contains key mapping data structures & routines.
 */


/* key definitions - depends on curses */

#ifndef CTRL
#define CTRL(c)	(c & 037)
#endif /* CTRL */

#define BACKSPACE	KEY_BACKSPACE
#define BACKTAB		CTRL('T')
#define DELFIELD	CTRL('F')
#define DELWORD		CTRL('W')
#define DOWN		KEY_DOWN
#define ENTER		KEY_ENTER
#define ESCAPE		27
#define EXECUTE		CTRL('X')
#define HELP		CTRL('Y')
#define KEYS_HELP	CTRL('O')
#define INSFIELD	KEY_IL
#define INSTOGGLE	KEY_IC
#define LEFT		KEY_LEFT
#define NEXTPAGE	KEY_NPAGE
#define PREVPAGE	KEY_PPAGE
#define QUITMENU	CTRL('C')
#define QUITPROG	CTRL('B')
#define REDRAW		CTRL('L')
#define RIGHT		KEY_RIGHT
#define SCROLLDOWN	CTRL('D')
#define SCROLLUP	CTRL('U')
#define SPACE		' '
#define TAB		'\t'
#define UP		KEY_UP

#ifndef DELETE
#define DELETE		0x7f
#endif /* DELETE */

#endif /* _KEY_MAP_H */
#endif /* SEC_BASE */
