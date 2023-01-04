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
 *	@(#)$RCSfile: globals.h,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/12/01 15:36:23 $
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
 *	Copyright 1990, Eric Shienbrood
 *
 * This software may be freely copied, distributed, or modified, as long
 * this copyright notice is preserved.
 *
 *
 * globals.h : declarations for global variables and macros
 *
 */

#include <sys/types.h>
#include <sgtty.h>

extern int	verbose;
extern int	fold_opt;	/* Fold long lines */
extern int	stop_opt;	/* Stop after form feeds */
extern int	ssp_opt;	/* Suppress white space */
extern int	show_opt;	/* Show control characters as ^c */
extern int	show_all_opt;	/* Show all ctrl chars, even CR, BS, and TAB */
extern int	ul_opt;		/* Underline as best we can */
extern int	clreol;		/* Don't scroll, use clear-eol mode */

extern char	**fnames;	/* The list of file names */
extern int	fnum;		/* Index of current file in fnames */
extern int	nfiles;		/* Number of files left to process */
extern int	noscroll;
extern int      freeze_lines;	/* user has specified window lines */ /* 001 */

/*
 * Structure describing the state of a line on the screen.
 * The variable "Screen" points to an array of screeninfo
 * structures, one for each line in the display.  To make
 * scrolling more efficient, this array is treated as a
 * circular array, whose last element is pointed to by
 * the variable "screen_end".  Thus the first element is
 * at position screen_end+1 if screen_end is not pointing
 * at the end of the array, or at position 0 if it is. 
 * When a scroll takes place, we only have to increment
 * screen_end, rather than copying all the elements of
 * the array up or down.
 *
 * The fields of a screeninfo structure are as follows:
 *
 *	fline		The line number that this screen line displays
 *			within the file.  Note that in the case of long
 *			lines (those that occupy more than one screen line),
 *			this field will have the same value for several
 *			consecutive elements in the array.
 *
 *	seek_key	The character position within the file of the first
 *			character displayed on this screen line.
 *
 *	fragnum		For a long line, the sequence number of this screen
 *			line in the sequence of screen lines that display this
 *			file line.  This value is always 0 for a line that
 *			fits on one screen line.
 *
 *	width		The number of columns that this screen line takes up.
 *			For long lines, all but the last fragment will have
 *			the value ScreenWidth in this field.
 *
 *	reschar		If a line occupies exactly ScreenWidth columns, and
 *			the last character on the line is a control character
 *			(and therefore its rendering occupies more than one
 *			screen column), then this is the number output
 *			that could not be displayed on this screen line
 *			because they would have caused it to wrap.
 */

struct screeninfo {
	off_t fline;		/* line # in file */
	off_t seek_key;		/* seek index within file */
	short fragnum;		/* fragment # of this file line on screen */
	short width;		/* width of line as displayed */
	short reschar;		/* amount of last character not displayed */
};

extern struct screeninfo *Screen;	/* the screeninfo array */
extern struct screeninfo hidden_line;	/* represents imaginary line immediately
					above the top actual screen line */

#define line_width(lineno)		Screen[(lineno)].width

extern int	screen_end;	/* index in Screen of screen bottom */

extern int	cursor_line;	/* index in Screen of current display line */
extern int	display_line;	/* current display line */
extern int	cursor_column;	/* current screen column */
extern int	pstate;		/* current underline state */
extern int      bstate;         /* current bold state */ /* paw: QAR 2411 */
extern int	lines_to_display;/* # of lines to display in one screenful */
extern int	lines_to_scroll;/* # of lines scrolled by 'd' */

/* BEGIN TEMPORARY: */

extern int	promptlen;

/* END TEMPORARY */

#define echo_off(argp)  (argp).sg_flags &= ~ECHO
#define brks_on(argp)   (argp).sg_flags |= CBREAK
#define brks_off(argp)  (argp).sg_flags &= ~CBREAK
#define obaud(argp)	((argp).sg_ispeed)
#define kill_ch(argp)   ((argp).sg_kill)
#define erase_ch(argp)  ((argp).sg_erase)
#define xtabs(argp)	((argp).sg_flags & XTABS)

#define ctrl(letter)	(letter & 077)
#define	RUBOUT		'\177'
#define ESC		'\033'
#define QUIT		'\034'

extern int	docrterase;
extern int	docrtkill;
extern struct sgttyb otty, savetty;
extern int	no_intty;	/* standard input is not a tty */
extern int	no_tty;		/* standard output is not a tty */
extern int	slow_tty;	/* tty output speed is slow (<1200 baud) */
extern int	ttyfd;		/* file descriptor open on /dev/tty */
extern int	dumb;		/* terminal is dumb */
extern int	hard;		/* terminal is hardcopy, not crt */
extern int	hardtabs;	/* terminal can handle tab characters */
extern int	Wrap;		/* set if automargins */
extern int	colflag;	/* true if last line ended without newline */

/*
 * Numerical capabilities from termcap.  The first two
 * are obtained dynamically using ioctl(TIOCGWINSZ) if possible.
 */
extern int	ScreenLength;	/* Height of display in lines */
extern int	ScreenWidth;	/* Width of display in columns */
extern int	soglitch;	/* terminal has standout mode glitch */
extern int	ulglitch;	/* terminal has underline mode glitch */

/* Boolean terminal capabilities from termcap:  */

extern int	bad_so;	/* True if overwriting does not turn off standout */
extern int	eatnl;  /* Eats newline after 80th col (vt100 and c100) */

/* Strings from termcap for cursor motion, etc: */

extern char	*HomeStr;	/* home the cursor */
extern char	*CursorMotionStr;/* cursor motion string */
extern char	*ClearScreenStr;/* clear screen */
extern char	*EodClrStr;	/* clear to end of display */
extern char	*ScrollUpStr;	/* scroll backwards */
extern char	*EraseLineStr;	/* erase line */
extern char	*Senter, *Sexit;/* enter and exit standout mode */
extern char	*ULenter, *ULexit;	/* enter and exit underline mode */
extern char     *Mexit;         /* exit attribute mode */ /* paw: QAR 1882 */
extern char     *Benter;        /* enter bold mode */ /* paw: QAR 2411 */
extern char	*chUL;		/* underline character */
extern char	*chBS;		/* backspace character */
extern char	*visible_bell;	/* sequence to use instead of bell */

/* File positioning and I/O: */

typedef struct {
	off_t	seek_key;	/* Seek offset from start of file */
	int	width;		/* Width of line in screen columns */
				/*  -1 => line is "invisible", e.g. ssp_opt */
} line_rec;

extern line_rec	*lineindex;	/* Seek address of each line in current file */
extern char	*curfilename;	/* Current input file name */
extern FILE	*curfile;	/* Current input file */
extern int	Currline;	/* Line in input we are currently at */
extern int	Currfrag;	/* Next line fragment */
extern int	Curresid;	/* Residual character count for partially */
				/*     displayed control character */
extern int	Prevline;	/* Line in input where last search started */
extern off_t	file_pos;	/* Current seek position in file */
extern off_t	file_size;	/* Size in byte of current file */
extern int	ispipe;		/* Current file does not support seeking */

#define Fopen(s,m)	(Currline = 0,file_pos=0,fopen(s,m))
#define Ftell()		file_pos
#define Fseek(off)	(file_pos=off,colflag=0,fseek(curfile,off,0))
#undef getc
#undef ungetc
#define Getc()		(++file_pos, getc(curfile))
#define Ungetc(c)	(--file_pos, ungetc(c,curfile))

/* Miscellaneous variables: */

extern int		inwait;
extern int		Pause;

extern int putch();
extern char *malloc();
extern char *realloc();
extern char *getenv();
extern struct screeninfo *get_top_screen_line();

#define printf mprintf

/* Message stuff */

#include "more_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_MORE, Num, Str)
