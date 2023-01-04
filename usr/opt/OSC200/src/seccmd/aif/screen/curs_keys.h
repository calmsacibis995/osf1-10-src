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
 *	@(#)$RCSfile: curs_keys.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:28 $
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   curses.h
 *
 * ORIGINS: 3, 10, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef KEY_BREAK

#ifdef NLS
/* Funny "characters" enabled for various special function keys for input */
/*
 * Under NLS, we start their collation around 3500 octal (1856 decimal)
 * to permit room for NLS characters with large values
*/
#define KEY_BREAK	03501		/* break key (unreliable) */
#define KEY_DOWN	03502		/* The four arrow keys ... */
#define KEY_UP		03503
#define KEY_LEFT	03504
#define KEY_RIGHT	03505		/* ... */
#define KEY_HOME	03506		/* Home key (upward+left arrow) */
#define KEY_BACKSPACE	03507		/* backspace (unreliable) */
#define KEY_F0		03510		/* Function keys.  Space for 64 */
#define KEY_F(n)	(KEY_F0+(n))	/* keys is reserved. */
#define KEY_DL		03610		/* Delete line */
#define KEY_IL		03611		/* Insert line */
#define KEY_DC		03612		/* Delete character */
#define KEY_IC		03613		/* Insert char or enter insert mode */
#define KEY_EIC		03614		/* Exit insert char mode */
#define KEY_CLEAR	03615		/* Clear screen */
#define KEY_EOS		03616		/* Clear to end of screen */
#define KEY_EOL		03617		/* Clear to end of line */
#define KEY_SF		03620		/* Scroll 1 line forward */
#define KEY_SR          03621           /* Scroll 1 line backwards (reverse)*/
#define KEY_NPAGE	03622		/* Next page */
#define KEY_PPAGE	03623		/* Previous page */
#define KEY_STAB	03624		/* Set tab */
#define KEY_CTAB	03625		/* Clear tab */
#define KEY_CATAB	03626		/* Clear all tabs */
#define KEY_ENTER	03627		/* Enter or send (unreliable) */
#define KEY_SRESET      03630           /* soft (partial) reset (unreliable)*/
#define KEY_RESET	03631		/* reset or hard reset (unreliable) */
#define KEY_PRINT	03632		/* print or copy */
#define KEY_LL		03633		/* home down or bottom (lower left) */
					/* The keypad is arranged like this:*/
					/*    a1    up    a3   */
					/*   left   b2  right  */
					/*    c1   down   c3   */
#define KEY_A1		03634		/* upper left of keypad */
#define KEY_A3		03635		/* upper right of keypad */
#define KEY_B2		03636		/* center of keypad */
#define KEY_C1		03637		/* lower left of keypad */
#define KEY_C3		03640		/* lower right of keypad */
#define KEY_ACTION      03641           /* Action key            */
#else
/* Funny "characters" enabled for various special function keys for input */
#define KEY_BREAK	0401		/* break key (unreliable) */
#define KEY_DOWN	0402		/* The four arrow keys ... */
#define KEY_UP		0403
#define KEY_LEFT	0404
#define KEY_RIGHT	0405		/* ... */
#define KEY_HOME	0406		/* Home key (upward+left arrow) */
#define KEY_BACKSPACE	0407		/* backspace (unreliable) */
#define KEY_F0		0410		/* Function keys.  Space for 64 */
#define KEY_F(n)	(KEY_F0+(n))	/* keys is reserved. */
#define KEY_DL		0510		/* Delete line */
#define KEY_IL		0511		/* Insert line */
#define KEY_DC		0512		/* Delete character */
#define KEY_IC		0513		/* Insert char or enter insert mode */
#define KEY_EIC		0514		/* Exit insert char mode */
#define KEY_CLEAR	0515		/* Clear screen */
#define KEY_EOS		0516		/* Clear to end of screen */
#define KEY_EOL		0517		/* Clear to end of line */
#define KEY_SF		0520		/* Scroll 1 line forward */
#define KEY_SR          0521            /* Scroll 1 line backwards (reverse)*/
#define KEY_NPAGE	0522		/* Next page */
#define KEY_PPAGE	0523		/* Previous page */
#define KEY_STAB	0524		/* Set tab */
#define KEY_CTAB	0525		/* Clear tab */
#define KEY_CATAB	0526		/* Clear all tabs */
#define KEY_ENTER	0527		/* Enter or send (unreliable) */
#define KEY_SRESET      0530            /* soft (partial) reset (unreliable)*/
#define KEY_RESET	0531		/* reset or hard reset (unreliable) */
#define KEY_PRINT	0532		/* print or copy */
#define KEY_LL		0533		/* home down or bottom (lower left) */
					/* The keypad is arranged like this:*/
					/*    a1    up    a3   */
					/*   left   b2  right  */
					/*    c1   down   c3   */
#define KEY_A1		0534		/* upper left of keypad */
#define KEY_A3		0535		/* upper right of keypad */
#define KEY_B2		0536		/* center of keypad */
#define KEY_C1		0537		/* lower left of keypad */
#define KEY_C3		0540		/* lower right of keypad */
#define KEY_ACTION      0541            /* Action key            */
#endif /* NLS */


#endif /* KEY_BREAK */

