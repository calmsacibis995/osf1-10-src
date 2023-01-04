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
static char	*sccsid = "@(#)$RCSfile: curscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:41 $";
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
/*
 * Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
 */



/* This file contains basic curses support routines specific to
 * this sort of package.
 */

#include	<sys/stat.h>
#include	<sgtty.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<termios.h>

#ifndef _POSIX_VDISABLE
# define _POSIX_VDISABLE 0
#endif /* _POSIX_VDISABLE */

#include	"userif.h"
#include	"curs_supp.h"
#include	"key_map.h"
#include	"kitch_sink.h"
#include	"logging.h"
#ifdef OLD_XENIX
#include	<sys/console.h>	/* for CONS_CURRENT ioctl */
#include	<sys/comcrt.h>	/* for EGA #define, etc. */
#endif OLD_XENIX

#ifdef DEBUG
extern	FILE	*logfp;
#endif DEBUG

extern char *Malloc();

#define ENTSIZ	14

/*
 * initialize the screen & keyboard handlers
 * this routine calls the underlying screen support library.
 * It is extremely #defined, as this is one place unix variants
 * vary a great deal
 */


int	has_underline;

static	struct termios	save_tstate;


initcurses()
{
	unsigned char	save_intr, save_quit, save_bs;
	static	int	first = 1;
	static	struct termios	cur_tstate;

	if (find_terminal ()) {
		printf ("Exiting due to find_terminal failure.\n");
		printf ("terminal type \'%s\'\n", getenv ("TERM"));
		exit (-1);
	}
	DUMPVARS ("Terminal type %s\n", getenv ("TERM"), NULL, NULL);

	if (tcgetattr (0, &save_tstate)) {
		printf ("Exiting due to inability to get terminal state\n");
		exit (-1);
	}
	init_key_conv (save_tstate.c_cc[VERASE]);
	bcopy (&save_tstate, &cur_tstate, sizeof (struct termios));
	cur_tstate.c_cc[VINTR]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VQUIT]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VERASE]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VKILL]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VEOF]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VEOL]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VEOL2]		= _POSIX_VDISABLE;
#ifdef DONTWANT
	cur_tstate.c_cc[VMIN]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VTIME]		= _POSIX_VDISABLE;
#endif /* DONTWANT */
	cur_tstate.c_cc[VSTART]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VSTOP]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VSUSP]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VDSUSP]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VFLUSH]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VWERASE]	= _POSIX_VDISABLE;
	cur_tstate.c_cc[VLNEXT]		= _POSIX_VDISABLE;
#ifdef ULTRIX
	cur_tstate.c_cc[VSWTCH]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VRPRNT]		= _POSIX_VDISABLE;
	cur_tstate.c_cc[VQUOTE]		= _POSIX_VDISABLE;
#endif /* ULTRIX */
#ifdef _POSIX_SOURCE
	cur_tstate.c_cc[VREPRINT]	= _POSIX_VDISABLE;
	cur_tstate.c_cc[VDISCARD]	= _POSIX_VDISABLE;
	cur_tstate.c_cc[VSTATUS]	= _POSIX_VDISABLE;
#endif /* _POSIX_SOURCE */
	if (tcsetattr (0, TCSANOW, &cur_tstate)) {
		printf ("Exiting due to inability to set terminal state\n");
		exit (-1);
	}

	initscr();
	if (!stdscr) {
		enditall ("Could not initialize window on screen.\n");
	}
	cbreak();
#ifndef NO_SIGS
	raw();
#endif /* NO_SIGS */
	noecho();
	nonl();
	idlok (stdscr, TRUE);
	fixwin (stdscr);

	/* save state for shell escapes and normal programs */
	def_prog_mode();

	overlay_win = newwin (1, 0, LINES - 2, COLS - 1);
	fixwin (overlay_win);

	/* figure out whether the terminal can UNDERLINE */

#ifdef OLD_XENIX
	/* for the Xenix graphics device, only the monochrome has UNDERLINE */
	switch (ioctl (fileno (stdin), CONS_CURRENT, 0)) {
	case	MONO:
		has_underline = 1;
		break;
	case	CGA:
	case	EGA:
	case	VGA:
	case	PGA:
		has_underline = 0;
		break;
	case	-1: /* non-graphics device, therefore has UNDERLINE */
		has_underline = 1;
		break;
	}
#else /* OLD_XENIX */
	/* on non-Xenix, assume has UNDERLINE unless terminfo says otherwise */
	has_underline = 1;
#endif /* OLD_XENIX */

#ifdef OSF
	/* for those that have UNDERLINE, make sure that terminfo supports it */
	if (has_underline)
		if (enter_underline_mode == (char *) 0 ||
		    enter_underline_mode[0] == '\0')
			has_underline = 0;
#endif /* OSF */
	DUMPVARS ("Has_underline: %d\n", has_underline, NULL, NULL);

   /* set up screen loc & size variables based on curses screen size */

	FTR_LINE = LINES - 1;
	CMDS_LINE1 = FTR_LINE - 2;
	CMDS_LINE2 = CMDS_LINE1 - 1;
	CMDS_LINE3 = CMDS_LINE2 - 1;
	MENU_ROWS = LINES - 8;
	MENU_COLS = COLS;
	WMSGROW = CMDS_LINE3 - 2;
	WMSGCOL = 5;
	WMSGLEN = COLS - 10;

	return;
}



fixwin (w)
WINDOW *w;
{
#ifdef OSF
# ifndef NOKEYPAD
	keypad (w, TRUE);
# endif /* NOKEYPAD */
	if ((eighthbit (key_down) || eighthbit (key_up) ||
	    eighthbit (key_left) || eighthbit (key_right)) && has_meta_key) {
		DUMPLVARS ("eighth bit is set and has meta key\n", 0, 0, 0);
		if (meta (w, TRUE) == ERR) {
			enditall (
"Terminal reports eight bits but doesn't have km terminfo capability\n");
		}
	}
#endif /* OSF */
}



/*
 * undoes the initcurses, restoring the screen to normal input mode
 */

restorescreen()
{
	if (stdscr) {
		clear();
		move (LINES-1, 0);
		/* V.3 doesn't reset cursor properly */
#ifdef OSF
		if (cursor_normal)
			putp (cursor_normal);
#endif /* OSF */
		refresh();
		nl();
		echo();
#ifndef NO_SIGS
		noraw();
#endif /* NO_SIGS */
		nocbreak();
		endwin();
	}
	tcsetattr (0, TCSANOW, &save_tstate);
	return;
}



/* look up the terminal type in the TTYTYPE database and ask user
 * the terminal type if not found in TERMINFO database.
 */

static	char	termbuf[5 + ENTSIZ+1];	/* "TERM=" + ENTSIZ name */

int
find_terminal ()
{
	char	*tty;
	char	buf[80];
	char	ttybuf[2 * (ENTSIZ + 1)];
	FILE	*fp;
	char	terminal[ENTSIZ + 1], ttytype[ENTSIZ + 1];
	int	found;
	char	*terminfodir;
	int	terminfoset = 0;

	terminfodir = getenv ("TERMINFO");
	if (terminfodir) {
		tty = Malloc (strlen (terminfodir) + 1);
		strcpy (tty, terminfodir);
		terminfodir = tty;
		terminfoset = 1;
		tty = NULL;
	} else
		terminfodir = TERMINFODIR;
	if ((tty = getenv ("TERM")) != (char *) 0 && tty[0] != '\0') {
		sprintf (buf, "%s/%c/%s", terminfodir, tty[0], tty);
		if (eaccess (buf, 4) == 0) {
			if (terminfoset)
				Free (terminfodir);
			return (0);
		}
		if (terminfoset) {
			sprintf (buf, "%s/%c/%s", TERMINFODIR, tty[0], tty);
			if (eaccess (buf, 4) == 0) {
				Free (terminfodir);
				return (0);
			}
		}
	}
	strcpy (ttybuf, ttyname(0));
	tty = strrchr (ttybuf, '/') + 1;
	/* check ttytype file for this terminal */
	fp = fopen (TTYTYPE_FILE, "r");
	found = 0;
	if (fp != (FILE *) 0)
		while (fgets (buf, sizeof (buf), fp) != (char *) 0) {
			if (buf[0] == '#')
				continue;
			sscanf (buf, "%s %s", ttytype, terminal);
			if (strcmp (terminal, tty) == 0) {
				found = 1;
				break;
			}
		}
	fclose (fp);
	if (!found)
		ttytype[0] = '\0';
	for (;;) {
		if (found)  {
			sprintf (buf, "%s/%c/%s",
			  terminfodir, ttytype[0], ttytype);
			if (eaccess (buf, 4) == 0) {
				sprintf (termbuf, "TERM=%s", ttytype);
				putenv (termbuf);
				if (terminfoset)
					Free (terminfodir);
				return (0);
			}
			if (terminfoset) {
				sprintf (buf, "%s/%c/%s",
				  TERMINFODIR, ttytype[0], ttytype);
				if (eaccess (buf, 4) == 0) {
					sprintf (termbuf, "TERM=%s", ttytype);
					putenv (termbuf);
					Free (terminfodir);
					return (0);
				}
			}
		}
		if (!found)
			printf ("Your terminal type is not specified.\n");
		else	printf (
	  "Cannot find terminal type \'%s\' in the terminal database \'%s\'.\n",
	   	  ttytype, TERMINFODIR);
		printf ("Please enter your terminal type now (q to exit): ");
		fflush (stdout);
		gets (buf);
		if ((buf[0] == 'q' || buf[0] == 'Q') && buf[1] == '\0') {
			if (terminfoset)
				Free (terminfodir);
			return (1);
		}
		buf[ENTSIZ] = '\0';
		strcpy (ttytype, buf);
		found = 1;
	}
}

/*
 * check if eighth bit is set in any screen description
 */

int
eighthbit (string)
char	*string;
{
	unsigned char	c;
	if (string)
		while (c = *string) {
		DUMPVARS ("Eighth bit called for string 0x%lx\n", c, 0, 0);
			if (c & 0x80)
				return (1);
			else	string++;
		}
	return (0);
}



/*
 * UTILITY ROUTINES USED BY THE APPLICATION ROUTINES
 */

/* puts the string in the current screen position.
 * pad out to "length" with spaces.
 */

void
wpadstring (window, string, length)
WINDOW	*window;
char	*string;
int	length;
{
	register int	i;

	if (strlen (string) > length)
		for (i = 0; i < length; i++, string++)
			WADDCHAR (window, *string);
	else {
		waddstr (window, string);
		putspaces (window, length - strlen (string));
	}
	return;
}


/* puts the number in the current screen position.
 * pad out to "length" with spaces.
 */

void
padnumber (window, number, length)
WINDOW	*window;
long	number;
uchar	length;
{
	register int	i;
	char	numbuf[11];  /* largest 32 bit num is 4294967296
						      0123456789 */

	sprintf (numbuf, "%ld", number);
	if (number == BIGNEGNUM)
		for (i = 0; i < length; i++)
			 WADDCHAR (window, ' ');
	else if (strlen (numbuf) > length)
		for (i = 0; i < length; i++)
			WADDCHAR (window, '#');
	else
		wpadstring (window, numbuf, length);
	return;
}


/*
 * put the specified number of spaces at the current position in the window
 */

void
putspaces (window, number)
WINDOW	*window;
uchar	number;
{
	register int	i;

	for (i = 0; i < number; i++)
		WADDCHAR (window, SPACE);
	return;
}

/*
 * change the appearance of the cursor
 */

void
cursor (cstate)
uchar	cstate;
{
	CURSOR_STATE = cstate;
#ifdef OSF
	switch (cstate) {
	case	BLOCKCURSOR:
		if (cursor_visible)
			putp (cursor_visible);
		else if (cursor_normal)
			putp (cursor_normal);
		break;
	case	INVISCURSOR:
		if (cursor_invisible)
			putp (cursor_invisible);
		break;
	case	UNDERCURSOR:
		if (cursor_normal)
			putp (cursor_normal);
		break;
	}
	return;
#endif /* OSF */
}


/*
 * correct the way the cursor looks
 */

void
correctcursor (stp)
struct	state	*stp;
{
	struct	scrn_desc	*sdp;
	int	cursor_state;

	sdp = sttosd (stp);
	switch (stp->screenp->scrntype) {
	case SCR_MENU:
		cursor_state = INVISCURSOR;
		parkcursor ();
		break;
	case SCR_MENUPROMPT:
		if (sdp->type != FLD_CHOICE)
			cursor_state = BLOCKCURSOR;
		else if (stp->curfield < stp->screenp->ndescs - 1 &&
		    (sdp+1)->type != FLD_CHOICE)
			cursor_state = BLOCKCURSOR;
		else {
			cursor_state = INVISCURSOR;
			wmove (stp->window, sdp->row, sdp->col +
			  strlen(sdp->prompt));
		}
		break;
	case SCR_TEXT:
		cursor_state = INVISCURSOR;
		parkcursor ();
		break;
	default:
		cursor_state = BLOCKCURSOR;
		break;
	}
	cursor (cursor_state);
	return;
}


/*
 * park the cursor in the upper left right corner of the screen
 * This really should be lower right, but to preempt scrolling
 * curses doesn't usually let you do this. The ones that do usually
 * have problems (such as scrolling).
 */

parkcursor ()
{
	move (0, 0);
}


/*
 * highlight a prompt on the screen
 */

void
highlight (window, row, col, string)
WINDOW	*window;
uchar	row, col;
char	*string;
{
	static	int	first = 1;
	static	int	reverse;

	/* remember if cursor handles reverse video, o.w. just use standout */
#ifdef OSF
	if (first) {
		if (enter_reverse_mode == (char *) 0 ||
		    enter_reverse_mode[0] == '\0')
			reverse = 0;
		else
			reverse = 1;
		first = 0;
	}
#else /* OSF */
	reverse = 0;
#endif /* OSF */

	wmove (window, row, col);
	if (reverse)
		wattron  (window, A_REVERSE);
	else
		wattron (window, A_STANDOUT);

	waddstr (window, string);

	if (reverse)
		wattroff (window, A_REVERSE);
	else
		wattroff (window, A_STANDOUT);
	return;
}


/*
 * unhighlight a prompt on the screen (maybe have hardware support?)
 */

void
unhighlight (window, row, col, string)
uchar	row, col;
char	*string;
WINDOW	*window;
{
	mvwaddstr (window, row, col, string);
	return;
}

/*
 * APPLICATION-SPECIFIC ROUTINES
 */

/*
 * highlight a toggle field
 */

highlighttoggle (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	uchar row, col;

	if (sdp->type == FLD_SCRTOG)
		scr_rowcol(stp, sdp, &row, &col);
	else {
		row = sdp->row;
		col = sdp->col;
	}
	if (sdp->len != 1) {
		mvwaddch(stp->window, row, col - 1, TOGGLECL);
		mvwaddch(stp->window, row, col + sdp->len, TOGGLECR);
	} else {
		wmove(stp->window, row, col);
	}
}


/*
 * unhighlight a toggle field
 */

unhighlighttoggle (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	uchar row, col;

	if (sdp->type == FLD_SCRTOG)
		scr_rowcol(stp, sdp, &row, &col);
	else {
		row = sdp->row;
		col = sdp->col;
	}
	if (sdp->len != 1) {
		mvwaddch(stp->window, row, col - 1, ' ');
		mvwaddch(stp->window, row, col + sdp->len, ' ');
	} else {
		wmove(stp->window, sdp->row, sdp->col);
	}
}



/*
 * set (turn on attr) TOGGLE field
 */

void
settoggle (window, row, col, string, len)
WINDOW	*window;
uchar	row, col, len;
char	*string;
{
	static	int	first = 1;
	static	int	reverse;
	register int i;

	/* remember if cursor handles reverse video, o.w. just use standout */
#ifdef OSF
	if (first) {
		if (enter_reverse_mode == (char *) 0 ||
		    enter_reverse_mode[0] == '\0')
			reverse = 0;
		else
			reverse = 1;
		first = 0;
	}
#else /* OSF */
	reverse = 0;
#endif /* OSF */

	wmove(window, row, col);
	if (reverse)
		wattron(window, A_REVERSE);
	else
		wattron(window, A_STANDOUT);

	for (i = 0; i < strlen(string); i++)
		waddch(window, string[i]);
	for ( ; i < len; i++)
		waddch(window, ' ');

	if (reverse)
		wattroff (window, A_REVERSE);
	else
		wattroff (window, A_STANDOUT);
	return;
}

/*
 * unset (turn off attr) TOGGLE field
 */

void
unsettoggle (window, row, col, string, len)
WINDOW	*window;
uchar	row, col, len;
char	*string;
{
	register int i;

	wmove(window, row, col);
	for (i = 0; i < strlen(string); i++)
		waddch(window, string[i]);
	for ( ; i < len; i++)
		waddch(window, ' ');
	return;
}



/*
 * clean up screen, note memory error, & split
 */
void
ExitMemoryError ()
{
	MemoryError ();		/* dies */
}



/*
 * application-specific cleanup reoutine called by routines in Utils.c
 */

int
EOP_cleanup ()
{
	restorescreen();
}
