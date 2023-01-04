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
 *	@(#)$RCSfile: char.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:10:20 $
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
 * Copyright (c) 1983 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	char.h	3.6 (Berkeley) 6/29/88
 */

/*
 * Macros and things to deal with control characters.
 *
 * Unctrl() is just like the standard function, except we don't want
 * to include curses.
 * Isctrl() returns true for all characters less than space and
 * greater than or equal to delete.
 * Isprt() is tab and all characters not isctrl().  It's used
 * by wwwrite().
 * Isunctrl() includes all characters that should be expanded
 * using unctrl() by wwwrite() if ww_unctrl is set.
 */

extern char *_unctrl[];
extern char _cmap[];
#define ctrl(c)		(c & 0x1f)
#define unctrl(c)	(_unctrl[(unsigned char) (c)])
#define _C		0x01
#define _P		0x02
#define _U		0x04
#define isctrl(c)	(_cmap[(unsigned char) (c)] & _C)
#define isprt(c)	(_cmap[(unsigned char) (c)] & _P)
#define isunctrl(c)	(_cmap[(unsigned char) (c)] & _U)
