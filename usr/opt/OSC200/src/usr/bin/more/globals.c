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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: globals.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/12/01 15:34:41 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 *	Copyright 1990, Eric Shienbrood
 *
 * This software may be freely copied, distributed, or modified, as long
 * this copyright notice is preserved.
 *
 *
 * globals.c : global variables definitions
 */

#include <stdio.h>
#include "globals.h"

int	verbose;
int	fold_opt = 1;	/* Fold long lines */
int	stop_opt = 1;	/* Stop after form feeds */
int	ul_opt = 1;	/* Underline as best we can */
int	show_opt = 1;	/* Show control characters as ^c */
int	show_all_opt;	/* Show all ctrl chars, even CR, BS, and TAB */
int	ssp_opt;	/* Suppress white space */
int	clreol;		/* Don't scroll, use clear-eol mode */

char	**fnames;	/* The list of file names */
int	fnum;		/* Index of current file in fnames */
int	nfiles;		/* Number of files left to process */
int	noscroll;
int     freeze_lines = 0;/* user has specified window lines */ /* 001 */

struct screeninfo *Screen;	/* the screeninfo array */
struct screeninfo hidden_line;	/* represents imaginary line immediately
				above the top actual screen line */
int	screen_end;	/* index in Screen of screen bottom */

int	cursor_line;	/* index in Screen of current display line */
int	display_line;	/* current display line */
int	cursor_column;	/* current screen column */
int	pstate;		/* current underline state */
int     bstate;		/* current bold state */ /* paw: QAR 2411 */
int	lines_to_display;/* # of lines to display in one screenful */
int	lines_to_scroll;/* # of lines scrolled by 'd' */

/* BEGIN TEMPORARY: */
int	promptlen;
/* END TEMPORARY */

int	docrterase;
int	docrtkill;
struct sgttyb otty, savetty;
int	no_intty;	/* standard input is not a tty */
int	no_tty;		/* standard output is not a tty */
int	slow_tty;	/* tty output speed is slow (<1200 baud) */
int	ttyfd;		/* file descriptor open on /dev/tty */
int	dumb;		/* terminal is dumb */
int	hard;		/* terminal is hardcopy, not crt */
int	hardtabs;	/* terminal can handle tab characters */
int	Wrap = 1;	/* set if automargins */
int	colflag;	/* true if last line ended without newline */

/*
 * Numerical capabilities from termcap.  The first two
 * are obtained dynamically using ioctl(TIOCGWINSZ) if possible.
 */
int	ScreenLength;	/* Height of display in lines */
int	ScreenWidth;	/* Width of display in columns */
int	soglitch;	/* terminal has standout mode glitch */
int	ulglitch;	/* terminal has underline mode glitch */

/* Boolean terminal capabilities from termcap.  */

int	bad_so;	/* True if overwriting does not turn off standout */
int	eatnl;  /* Eats newline after 80th col (vt100 and c100) */

/* Strings from termcap for cursor motion, etc: */

char	*HomeStr;		/* home the cursor */
char	*CursorMotionStr;	/* cursor motion string */
char	*ClearScreenStr;	/* clear screen */
char	*EodClrStr;		/* clear to end of display */
char	*ScrollUpStr;		/* scroll backwards */
char	*EraseLineStr;		/* erase line */
char	*Senter, *Sexit;	/* enter and exit standout mode */
char	*ULenter, *ULexit;	/* enter and exit underline mode */
char    *Benter = "[1m";	/* enter bold mode */ /* paw: QAR 2411 */
char    *Mexit = "[0m";	/* exit attribute mode */ /* paw: QAR 1882 */
char	*chUL;			/* underline character */
char	*chBS;			/* backspace character */
char	*visible_bell;		/* sequence to use instead of bell */

line_rec	*lineindex;	/* Seek address of each line in current file */
char		*curfilename;	/* Current input file name */
FILE		*curfile;	/* Current input file */
int		Currline;	/* Line in input we are currently at */
int		Currfrag;	/* Next line fragment */
int		Curresid;	/* Residual character count for partially */
				/*     displayed control character */
int		Prevline;	/* Line in input where last search started */
off_t		file_pos;	/* Current seek position in file */
off_t		file_size;	/* Size in byte of current file */
int		ispipe;		/* Current file does not support seeking */

int		inwait;
int		Pause;
