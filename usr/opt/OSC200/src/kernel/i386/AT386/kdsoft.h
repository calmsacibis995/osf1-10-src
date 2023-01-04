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
 *	@(#)$RCSfile: kdsoft.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:34 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
 
/* **********************************************************************
 File:         kdsoft.h
 Description:  Software structures for keyboard/display driver, shared with
	drivers for specific graphics cards.


 Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989.
 All rights reserved.
********************************************************************** */

/*
 * This driver handles two types of graphics cards.  The first type
 * (e.g., EGA, CGA), treats the screen as a page of characters and
 * has a hardware cursor.  The second type (e.g., the Blit) treats the
 * screen as a bitmap.  A hardware cursor may be present, but it is
 * ignored in favor of a software cursor.
 *
 *
 * Most of the driver uses the following abstraction for the display:
 *
 * The cursor position is simply an index into a (logical) linear char
 * array that wraps around at the end of each line.  Each character
 * takes up ONE_SPACE bytes.  Values in [0..ONE_PAGE) are positions in
 * the displayed page.  Values < 0 and >= ONE_PAGE are off the page
 * and require some scrolling to put the cursor back on the page.
 *
 * The kd_dxxx routines handle the conversion from this abstraction to
 * what the hardware requires.
 *
 * (*kd_dput)(pos, ch, chattr)
 *	csrpos_t pos;
 *	char ch, chattr;
 *  Displays a character at "pos", where "ch" = the character to
 *  be displayed and "chattr" is its attribute byte.
 *
 * (*kd_dmvup)(from, to, count)
 *	csrpos_t from, to;
 *	int count;
 *  Does a (relatively) fast block transfer of characters upward.
 *  "count" is the number of character positions (not bytes) to move.
 *  "from" is the character position to start moving from (at the start
 *  of the block to be moved).  "to" is the character position to start
 *  moving to.
 *
 * (*kd_dmvdown)(from, to, count)
 *	csrpos_t from, to;
 *	int count;
 *  "count" is the number of character positions (not bytes) to move.
 *  "from" is the character position to start moving from (at the end
 *  of the block to be moved).  "to" is the character position to
 *  start moving to.
 *
 * (*kd_dclear)(to, count, chattr)
 *	csrpos_t, to;
 *	int count;
 *	char chattr;
 *  Erases "count" character positions, starting with "to".
 *
 * (*kd_dsetcursor)(pos)
 *  Sets kd_curpos and moves the displayed cursor to track it.  "pos"
 *  should be in the range [0..ONE_PAGE).
 *  
 * (*kd_dreset)()
 *  In some cases, the boot program expects the display to be in a
 *  particular state, and doing a soft reset (i.e.,
 *  software-controlled reboot) doesn't put it into that state.  For
 *  these cases, the machine-specific driver should provide a "reset"
 *  procedure, which will be called just before the kd code causes the
 *  system to reboot.
 */

extern void bmpput(), bmpmvup(), bmpmvdown(), bmpclear(), bmpsetcursor();

extern void	(*kd_dput)();		/* put attributed char */
extern void	(*kd_dmvup)();		/* block move up */
extern void	(*kd_dmvdown)();	/* block move down */
extern void	(*kd_dclear)();		/* block clear */
extern void	(*kd_dsetcursor)();
extern void	(*kd_dreset)();		/* prepare for reboot */
				/* set cursor position on displayed page */


/*
 * Globals used for both character-based controllers and bitmap-based
 * controllers.
 */
typedef	short	csrpos_t;	/* cursor position, ONE_SPACE bytes per char */

extern u_char 	*vid_start;	/* VM start of video RAM or frame buffer */
extern csrpos_t kd_curpos;		/* should be set only by kd_setpos */
extern short	kd_lines;		/* num lines in tty display */
extern short	kd_cols;
extern char	kd_attr;		/* current character attribute */


/*
 * Globals used only for bitmap-based controllers.
 * XXX - probably needs reworking for color.
 */

/*
 * The following font layout is assumed:
 *
 *  The top scan line of all the characters comes first.  Then the
 *  second scan line, then the third, etc.
 *
 *     ------ ... ---------|-----N--------|-------------- ... -----------
 *     ------ ... ---------|-----N--------|-------------- ... -----------
 *		.
 *		.
 *		.
 *     ------ ... ---------|-----N--------|-------------- ... -----------
 *
 * In the picture, each line is a scan line from the font.  Each scan
 * line is stored in memory immediately after the previous one.  The
 * bits between the vertical lines are the bits for a single character
 * (e.g., the letter "N").
 * There are "char_height" scan lines.  Each character is "char_width"
 * bits wide.  We make the simplifying assumption that characters are
 * on byte boundaries.  (We also assume that a byte is 8 bits.)
 */

extern u_char	*font_start;		/* starting addr of font */

extern short	fb_width;		/* bits in frame buffer scan line */
extern short	fb_height;		/* scan lines in frame buffer*/
extern short	char_width;		/* bit width of 1 char */
extern short	char_height;		/* bit height of 1 char */
extern short	chars_in_font;
extern short	cursor_height;		/* bit height of cursor */
			/* char_height + cursor_height = line_height */

extern u_char	char_black;		/* 8 black (off) bits */
extern u_char	char_white;		/* 8 white (on) bits */


/*
 * The tty emulation does not usually require the entire frame buffer.
 * (xstart, ystart) is the bit address for the upper left corner of the 
 * tty "screen".
 */

extern short	xstart, ystart;


/*
 * Accelerators for bitmap displays.
 */

extern short	char_byte_width;	/* char_width/8 */
extern short	fb_byte_width;		/* fb_width/8 */
extern short	font_byte_width;	/* num bytes in 1 scan line of font */
