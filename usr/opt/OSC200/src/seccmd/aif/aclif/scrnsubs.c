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
static char	*sccsid = "@(#)$RCSfile: scrnsubs.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:33 $";
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
 * Copyright (c) 1989-1990 SecureWare, Inc.  All rights reserved.
 */



/* This file contains all screen support routines used by the administrative
 * programs.  All routines which call into here use the header file "userif.h".
 */

#include	<sys/secdefines.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/signal.h>
#include 	<ctype.h>
#include	<stdio.h>
#include	<setjmp.h>
#include	<string.h>
#ifndef STANDALONE
#include	<sys/security.h>
#include	<sys/audit.h>
#endif
#include	"userif.h"

#define	ENTSIZ	14

#ifdef DEBUG
extern	FILE	*logfp;
#endif

/* constants used by utility routines */

#ifndef ON
#define ON 	1
#define OFF	0
#endif

/* Where messages appear in a window */

#define WMSGROW(screen)	(screen->nbrrows - 2)	/* On bottom line */
#define WMSGCOL		5			/* 5th column */

/* Where insert indicator appears in a window */

#define WINSROW(screen) (screen->nbrrows - 2)  /* On bottom line */
#define WINSCOL(screen) (screen->nbrcols - 10) /* 10th character from left */
#define INSERTIND	"INS"		       /* insert indicator */

#define ABOVEIND	"^"			/* more above */
#define BELOWIND	"v"			/* more below */

/* field boundaries for screens without underline capability */

#define LEFT_BOUNDARY   '<'
#define RIGHT_BOUNDARY  '>'
#define SINGLE_BOUNDARY '|' /* for scrolling regions with s_spaces == 1 */

/* cursor states */

#define BLOCKCURSOR	1
#define INVISCURSOR	2
#define UNDERCURSOR	3

/* key definitions - depends on curses */

#define CTRL(c)	(c & 037)

#define ENTER		KEY_ENTER
#define DOWN		KEY_DOWN
#define UP		KEY_UP
#define LEFT		KEY_LEFT
#define RIGHT		KEY_RIGHT
#define DELWORD		CTRL('W')
#define BACKSPACE	KEY_BACKSPACE
#define TAB		'\t'
#define BACKTAB		CTRL('T')
#define DELFIELD	CTRL('F')
#define	INSFIELD	KEY_IL
#define INSTOGGLE	KEY_IC
#define EXECUTE		CTRL('X')
#define HELP		CTRL('Y')
#define REDRAW		CTRL('L')
#define SPACE		' '
#define SCROLLDOWN	CTRL('D')
#define SCROLLUP	CTRL('U')

/* Screen state maintained by putscreen routines */

struct	state {
	uchar	curfield;	/* which scrn_desc is the current input */
	uchar	firstfield;	/* which scrn_desc is first input */
	uchar	lastfield;	/* which scrn_desc is last input */
	uchar	itemchanged;	/* whether item currently editing changed */
	uchar	columninfield;	/* current column in field */
	uchar	insert;		/* whether in insert mode */
	uchar	message;	/* whether message field on screen */
	struct	scrn_struct	*structp;	/* current struct table */
	struct	scrn_parms	*screenp;	/* current screen */
	WINDOW	*window;			/* current window */
	struct	scrn_ret	ret;		/* for return value */
	ushort	scrollitem;	/* in scrolling region, field currently in */
	ushort	topscroll;	/* upper left field is this offset in table */
	char	scrnrep[80];	/* screen representation of current field */
};

/*  error messages */

#define	REQUIREDERROR	"Field must be filled"
#define NOTNUMBERERROR	"Digits only in a number field"
#define YESNOERROR	"Answer (y)es or (n)o in this field"
#define NOPROMPT	"Tried to move to prompt field!!"

#define PRESSRETURN	"Press <RETURN> to continue"
#define PRESSSHORT	"<RETURN>"
#define NOHELP		"No help available for this field"
#define BLANKSCROLL	"No blank fields in scrolling regions"
#define	FULLSCROLL	"The scrolling region is full"

#define LONGESTMSG	strlen(BLANKSCROLL)

static char *screen_copyr = "Copyright (c) 1988 SecureWare, Inc.";

/* routine headers */

WINDOW	*new_window();
static void
   initstates(), downfill(), upfill(),
   leftfill(), rightfill(), delword(), backspace(), tabfill(),
   backtabfill(), delfield(), instoggle(), help(), redraw(),
   fillinkey(), downmenu(), rightmenu(), upmenu(), leftmenu(),
   downmprompt(), upmprompt(), backtabmprompt(), tabmprompt(),
   leftmprompt(), rightmprompt(), wpadstring(), padnumber(),
   putspaces(), cursor(), movetofield(), clearfield(),
   highlight(), unhighlight(), delchars(), moveright(), findchoice(),
   message(), rm_message(), correctcursor(),
   movetoscrollreg(), tabscrollreg(), backtabscrollreg(),
   scrolldownkey(), scrollupkey(), downscrollreg(), upscrollreg(),
   scroll_down(), scroll_up(), drawscrollreg(),
   scr_above_ind(), scr_below_ind(), scr_rowcol(), scr_moveto(),
   insfield(), scroll_rshift(), scroll_lshift(), helpscrn();
static int
   clearwindow(), drawbox(), wmoveto(), addstring(), underline(),
   waddchar(), ringbell(), leavefield(), findupitem(),
   finddownitem(), find_terminal(), eighthbit(), min(),
   reqfillin(), reqmprompt(), scr_offscreen();
static char *
   fixalpha();

/*
	Routine to display a screen, given a screen description and
	the data in a screen structure.
 */

extern char	*getenv(), *strrchr(), *strchr();

#define	TERMINFODIR	"/usr/lib/terminfo"
#define TTYTYPE_FILE	"/etc/ttytype"

/* convert the current field in the state structure to a scrn_desc */
#define		sttosd(stp)	&((stp)->screenp->sd[(stp)->curfield])

/* Help directory which stores the root of the help tree */
extern char *HelpDir;

extern	void	(*signal())();

/* routine that prints a screen on stdout */
#ifdef DEBUG
printscreen (screenp)
struct	scrn_parms	*screenp;
{
	struct	scrn_desc	*sdp;
	int	i, j;
	int	currow, curcol;

	putchar ('\n');
	currow = 1;
	curcol = 0;
	for (sdp = screenp->sd, i = 0; i < screenp->ndescs; i++, sdp++)  {
		print_moveto (currow, curcol,
			      sdp->row, sdp->col, screenp->nbrcols);
		currow = sdp->row;
		curcol = sdp->col;
		switch (sdp->type)  {
		case	FLD_PROMPT:
		case	FLD_CHOICE:
			printf (sdp->prompt);
			curcol += strlen (sdp->prompt);
			break;
		case	FLD_ALPHA:
		case	FLD_NUMBER:
			for (j = 0; j < sdp->len; j++) {
				putchar ('_');
				curcol++;
			}
			break;
		case	FLD_YN:
		case	FLD_CONFIRM:
		case	FLD_POPUP:
			putchar ('_');
			curcol++;
			break;
		case	FLD_SCROLL:
		    {
			int	temprow, tempcol;
			int	i, j, k;

			temprow = sdp->row;
			for (i = 0; i < sdp->s_lines; i++)  {
				tempcol = sdp->col;
				for (j = 0; j < sdp->s_itemsperline; j++) {
					print_moveto (currow, curcol,
						      temprow, tempcol,
						      screenp->nbrcols);
					curcol = tempcol;
					currow = temprow;
					for (k = 0; k < sdp->len; k++) {
						putchar ('_');
						curcol++;
					}
					tempcol += sdp->len + sdp->s_spaces;
				}
				temprow++;
			}
		    }
		}
	}
	/* fill out to 11 or 23 rows to support SS and ST doc macros.  */
	if (currow < 11)
		for (; currow < 11; currow++)
			putchar ('\n');
	else
		for (; currow < 23; currow++)
			putchar ('\n');
}

print_moveto (currow, curcol, torow, tocol, width)
{
	/* draw down to the current line */
	for (; currow <= torow; currow++) {
		for ( ;
		     curcol < ((currow == torow) ? tocol : width);
		     curcol++)
			putchar (' ');
		if (curcol == width) {
			putchar ('\n');
			curcol = 0;
		} else
			break;
	}
}
#endif

/* for a screen, list FLD_BOTH or FLD_INPUT fields that don't
 * have help files defined.  Also list fields for which files do not exist.
 * The prefix argument is the pathname of the release directory.
 */

printhelp (prefix, scrn)
char *prefix;
struct scrn_parms *scrn;
{
	int i;
	struct scrn_desc *sd = scrn->sd;
	char filename[100];
	int len;
	struct stat sb;

	if (scrn->scrntype == SCR_NOCHANGE)
		return;
	/* build the file name prefix for all help screens */
	(void) sprintf (filename, "%s%s", prefix, HelpDir);
	len = strlen (filename);

	for (i = 0; i < scrn->ndescs; i++, sd++) {
		if (sd->inout == FLD_OUTPUT)
			continue;
		/* on menu prompt screens, choice fields with
		 * blanks don't have help, choice fields without
		 * blanks do.
		 */
		if (scrn->scrntype == SCR_MENUPROMPT &&
		    sd->type == FLD_CHOICE &&
		    i < scrn->ndescs - 1 &&
		    (sd+1)->type != FLD_CHOICE &&
		    (sd+1)->inout == FLD_INPUT)
			continue;
		else if (sd->help == (char *) 0)
			(void) printf ("Field %d, no help defined\n", i);
		else {
			(void) strcpy (&filename[len], sd->help);
			if (prefix && stat (filename, &sb) < 0)
				(void) printf ("%s not found\n", filename);
			else
				(void) puts (filename);
		}
	}
}

/* signal catching logic depends on catching hangups until we can deal
 * with them and longjumping out of SIGINT and SIGQUIT when they are enabled.
 */

jmp_buf	env;
int	LowerLevel = 0;		/* set when a lower level screen must return to
				 * a higher level screen without popping through
				 */

int_catch (sig)
{
	(void) signal (SIGINT, SIG_IGN);
#ifdef DEBUG
	fprintf (logfp, "int_catch\n");
#endif
	if (LowerLevel) {
		LowerLevel = 0;
		longjmp (env, CONTINUE);
	} else
		longjmp (env, INTERRUPT);
	return;
}

/* quit always causes the whole program to exit */

quit_catch (sig)
{
	(void) signal (SIGQUIT, SIG_IGN);
#ifdef DEBUG
	fprintf (logfp, "quit_catch\n");
#endif
	longjmp (env, QUIT);
	return;
}

int	hup_caught = 0;

void
hup_catch (sig)
{
	(void) signal (SIGHUP, SIG_IGN);
#ifdef DEBUG
	fprintf (logfp, "hup_catch\n");
#endif
	hup_caught = 1;
	longjmp (env, HANGUP);
	return;
}

/* Window pointer that we save in case user interrupts screen drawing and
 * we need to free it from a higher level.
 */

WINDOW	*Global_window;

WINDOW *
putscreen (screenp, structp, clearpop)
struct	scrn_parms	*screenp;	/* screen description */
struct	scrn_struct	*structp;	/* data values for screen */
uchar	clearpop;			/* clear screen or pop window */
{
	register WINDOW	*window;
	register struct	scrn_desc	*sdp;
	register struct	scrn_struct	*sp;
	register int	i;

	static	int	first_time = 1;	/* for copyright message */

	/*  initialize and draw box around window */
	screenp->nbrrows = (LINES <= screenp->nbrrows) ?
				LINES - 1 : screenp->nbrrows;
	screenp->nbrcols = (COLS < screenp->nbrcols) ?
				COLS : screenp->nbrcols;
	window = new_window (screenp->toprow,
			     screenp->leftcol,
			     screenp->nbrrows,
			     screenp->nbrcols);

	/* save the window created so that it can be removed if the
	 * user interrupts the program while we're drawing it.
	 * Now's the time to break out in case there was a hangup (no
	 * harm done).  At this point, interrupts can be turned back on.
	 */
	Global_window = window;
	if (hup_caught)
		exit (0);
	(void) signal (SIGINT, int_catch);
	(void) signal (SIGQUIT, quit_catch);
	(void) signal (SIGHUP, SIG_DFL);

	clearwindow (window);
	drawbox (window);

	sp = structp;
	for (sdp = screenp->sd, i = 0; i < screenp->ndescs; i++, sdp++)  {
		/* find structure description for this field, if not
		   a CHOICE or PROMPT field */
		if (sdp->type != FLD_CHOICE && sdp->type != FLD_PROMPT)
			while (sp->desc != i)
				sp++;
		switch (sdp->type)  {
		case	FLD_PROMPT:
		case	FLD_CHOICE:
			wmoveto (window, sdp->row, sdp->col);
			addstring (window, sdp->prompt);
			break;
		case	FLD_ALPHA:
			if (!has_underline) {
				wmoveto (window, sdp->row, sdp->col - 1);
				waddchar (window, LEFT_BOUNDARY);
			} else {
				wmoveto (window, sdp->row, sdp->col);
				underline (window, ON);
			}
			if (sdp->inout == FLD_OUTPUT ||
			   (sdp->inout == FLD_BOTH && sp->filled))
				wpadstring (window, s_alpha(sp), sdp->len);
			else
				putspaces (window, sdp->len);
			if (!has_underline)
				waddchar (window, RIGHT_BOUNDARY);
			else
				underline (window, OFF);
			break;
		case	FLD_NUMBER:
			if (!has_underline) {
				wmoveto (window, sdp->row, sdp->col - 1);
				waddchar (window, LEFT_BOUNDARY);
			} else {
				wmoveto (window, sdp->row, sdp->col);
				underline (window, ON);
			}
			if (sdp->inout == FLD_OUTPUT ||
			   (sdp->inout == FLD_BOTH && sp->filled))
				padnumber (window, *s_number(sp), sdp->len);
			else	putspaces (window, sdp->len);
			if (!has_underline)
				waddchar (window, RIGHT_BOUNDARY);
			else
				underline (window, OFF);
			break;
		case	FLD_YN:
		case	FLD_CONFIRM:
		case	FLD_POPUP:
			if (!has_underline) {
				wmoveto (window, sdp->row, sdp->col - 1);
				waddchar (window, LEFT_BOUNDARY);
			}else {
				wmoveto (window, sdp->row, sdp->col);
				underline (window, ON);
			}
			if (sdp->inout == FLD_OUTPUT ||
			   (sdp->inout == FLD_BOTH && sp->filled))
				if (*s_yesno(sp))
					waddchar (window, YESCHAR);
				else	waddchar (window, NOCHAR);
			else	putspaces (window, 1);
			if (!has_underline)
				waddchar (window, RIGHT_BOUNDARY);
			else
				underline (window, OFF);
			break;
		case	FLD_SCROLL:
		    {
			int	temprow, tempcol;
			int	i, j;
			char	**strings;

			temprow = sdp->row;
			strings = s_scrollreg(sp);
			for (i = 0; i < sdp->s_lines; i++)  {
				tempcol = sdp->col;
				for (j = 0; j < sdp->s_itemsperline; j++) {
					if (!has_underline) {
					  wmoveto (window, temprow, tempcol-1);
					  if (sdp->s_spaces == 1)
					    waddchar (window, SINGLE_BOUNDARY);
					  else
					    waddchar (window, LEFT_BOUNDARY);
					} else {
					  wmoveto (window, temprow, tempcol);
					  underline (window, ON);
					}
					if ((i * sdp->s_itemsperline) + j
					< sp->filled && *strings)  {
						wpadstring (window,
							   *strings,
							   sdp->len);
						strings++;
					}
					else	putspaces (window, sdp->len);
					if (!has_underline)
					  if (sdp->s_spaces == 1)
					    waddchar (window, SINGLE_BOUNDARY);
					  else
					    waddchar (window, RIGHT_BOUNDARY);
					else
					  underline (window, OFF);
					tempcol += sdp->len + sdp->s_spaces;
				}
				temprow++;
			}
			/* check if there is more below the window */
			if (sdp->s_lines * sdp->s_itemsperline < sp->filled &&
			     strings[0][0] != '\0')  {
				wmoveto (
				 window,
				 sdp->row + sdp->s_lines - 1,
				 sdp->col +
				  (sdp->s_itemsperline * sdp->len) +
				  (sdp->s_itemsperline - 1) * sdp->s_spaces + 1
				);
				addstring (window, BELOWIND);
			}
		    }
		}
	}

	/* if a message only screen, do the press return to continue message */

	if (screenp->scrntype == SCR_MSGONLY) {
		char	*msg;

		/* decide between short form and long form of
		 * "Press return to continue"
		 */
		msg = (strlen (PRESSRETURN) > screenp->nbrcols - 2) ?
			PRESSSHORT : PRESSRETURN;
		message (NULL, window, screenp, msg, YES);
		cursor (UNDERCURSOR);
		touchwin (stdscr, TRUE);
		wnoutrefresh (stdscr);
		doupdate();
		/* Turn off interrupts now to go back into processing mode */
		(void) signal (SIGINT, SIG_IGN);
		(void) signal (SIGQUIT, SIG_IGN);
		rm_window (window);
		Global_window = (WINDOW *) 0;
		window = (WINDOW *) 0;
	}
	else  {
		touchwin (stdscr, TRUE);
		/* set and clear copyright string */
		switch (first_time) {
		default:
			break;
		case 1:
			mvaddstr (LINES-1, COLS-strlen(screen_copyr) - 3,
			 screen_copyr);
			first_time = 2;
			break;
		case 2:
			move (LINES - 1, COLS - strlen(screen_copyr) - 3);
			for (i = 0; i < strlen(screen_copyr); i++)
				addch (' ');
			first_time = 0;
			break;
		}
		wnoutrefresh (stdscr);
		wnoutrefresh (window);
		doupdate();
	}

	return (window);
}

/*  Prompt a user to fill in a screen.
 *  Returns the state of the screen, including whether a
 *  confirm entry was changed, whether the user just wants to
 *  execute the screen, and whether the screen itself was changed,
 *  as well as telling which item was current when the user finished.
 */

struct	scrn_ret
getscreen (window, screenp, structp, first_desc)
register WINDOW	*window;			/* which window screen is in */
struct	scrn_parms	*screenp;		/* screen description */
struct	scrn_struct	*structp;	/* screen data returned to process */
int	first_desc;			/* which scrn_struct to start */
{
	struct	state	states;		/* state variables */
	register struct	state	*stp;
	int	thischar;		/* current character */
	int	userbschar;		/* user backspace character */
	int	userkillchar;		/* user kill character */

	stp = &states;

	/* set up state structure, including curfield */

	initstates (window, screenp, structp, first_desc, stp);
	userbschar = erasechar();
	userkillchar = killchar();

	movetofield (stp);
	wrefresh (stp->window);


	/* flush readahead to avoid re-execute screen problem */

	flushinp();

	do {
		thischar = getch();
#ifdef DEBUG
fprintf (logfp, "wgetch: character 0%o isprint %d\n", thischar, isprint (thischar));
#endif
		if (thischar == userbschar)
			thischar = BACKSPACE;
		else if (thischar == '\r')
			thischar = ENTER;
		else if (thischar == '\n')
			thischar = DOWN;
		else if (thischar == CTRL('V'))
			thischar = INSTOGGLE;
		else if (thischar == CTRL('R'))
			thischar = INSFIELD;
		else if (thischar == CTRL('O')) {
			helpscrn ("help.keys");
			touchwin (stp->window, TRUE);
			touchwin (stdscr, TRUE);
			wnoutrefresh (stdscr);
			wnoutrefresh (stp->window);
			correctcursor (stp);
			doupdate();
			continue;
		}
		/* Bug in some versions of curses returns a '\0'
		 * on interrupt, causing the screen to beep.
		 * This code just throws it away.
		 */
		if (thischar == 0) {
			;
		}
		else if (screenp->scrntype == SCR_FILLIN)  {
			switch (thischar)  {
			case	SCROLLDOWN:
				scrolldownkey (stp);
				break;
			case	SCROLLUP:
				scrollupkey (stp);
				break;
			case	DOWN:
				downfill (stp);
				break;
			case	UP:
				upfill (stp);
				break;
			case	LEFT:
				leftfill (stp);
				break;
			case	RIGHT:
				rightfill (stp);
				break;
			case	DELWORD:
				delword (stp);
				break;
			case	BACKSPACE:
				backspace (stp);
				break;
			case	TAB:
			case	ENTER:
				tabfill (stp);
				break;
			case	BACKTAB:
				backtabfill (stp);
				break;
			case	DELFIELD:
				delfield (stp);
				break;
			case	INSFIELD:
				insfield (stp);
				break;
			case	INSTOGGLE:
				instoggle (stp);
				break;
			case	EXECUTE:
				/* have to successfully leave a field */
				if (!leavefield (stp))
					if (reqfillin (stp) == 0)
						stp->ret.flags |= R_EXECUTE;
				break;
			case	HELP:
				help (stp);
				break;
			case	REDRAW:
				redraw (stp);
				break;
			default:
				if (isprint (thischar))
					fillinkey (stp, thischar);
				else	ringbell();
			}
		} else if (screenp->scrntype == SCR_MENU)  {
			switch (thischar)  {
			case	DOWN:
				downmenu (stp);
				break;
			case	UP:
				upmenu (stp);
				break;
			case	EXECUTE:
				stp->ret.item = stp->curfield;
				stp->ret.flags |= R_EXECUTE;
				break;
			case	HELP:
				help (stp);
				break;
			case	REDRAW:
				redraw (stp);
				break;
			case	LEFT:
			case	BACKTAB:
				leftmenu (stp);
				break;
			case	RIGHT:
			case	ENTER:
			case	TAB:
				rightmenu (stp);
				break;
			default:
				ringbell();
			}
		} else if (screenp->scrntype == SCR_MENUPROMPT) {
			uchar	handled = 1;
			struct	scrn_desc	*sdp;

			switch (thischar)  {
			case	DOWN:
				downmprompt (stp);
				break;
			case	UP:
				upmprompt (stp);
				break;
			case	LEFT:
				leftmprompt (stp);
				break;
			case	RIGHT:
				rightmprompt (stp);
				break;
			case	EXECUTE:
				/* if no error, prepare to exit */
				if (!leavefield (stp))  {
					findchoice (stp);
					if (reqmprompt (stp) == 0) {
						stp->ret.flags |= R_EXECUTE;
						stp->ret.item = stp->curfield;
					}
				}
				break;
			case	HELP:
				help (stp);
				break;
			case	REDRAW:
				redraw (stp);
				break;
			default:
				handled = 0;
				break;
			}
			/*  all other cases other than enter require
			 *  user to be in non-choice
			 */
			if (!handled)  {
				sdp = sttosd (stp);
				if (sdp->type == FLD_CHOICE)
					if (thischar == ENTER)
						downmprompt (stp);
					else
						ringbell();
				else switch (thischar) {
				case	DELWORD:
					delword (stp);
					break;
				case	BACKSPACE:
					backspace (stp);
					break;
				case	TAB:
					tabmprompt (stp);
					break;
				case	BACKTAB:
					backtabmprompt (stp);
					break;
				case	DELFIELD:
					delfield (stp);
					break;
				case	INSTOGGLE:
					instoggle (stp);
					break;
				default:
					if (thischar == ENTER)
						downmprompt(stp) ;
					else
						if (isprint(thischar))
							fillinkey(stp, thischar);
						else
							ringbell() ;
				}
			}
		} else { /* SCR_NOCHANGE */
			switch (thischar)  {
			case	SCROLLDOWN:
				scrolldownkey (stp);
				break;
			case	SCROLLUP:
				scrollupkey (stp);
				break;
			case	DOWN:
				downfill (stp);
				break;
			case	UP:
				upfill (stp);
				break;
			case	TAB:
			case	ENTER:
				tabfill (stp);
				break;
			case	BACKTAB:
				backtabfill (stp);
				break;
			case	REDRAW:
				redraw (stp);
				break;
			default:
				ringbell();
				break;
			}
		}
		correctcursor (stp);
		wrefresh (stp->window);
	} while ((stp->ret.flags &
			(R_CONFIRM | R_EXECUTE | R_POPUP | R_ABORTED)) == 0);

	rm_window (stp->window);
	Global_window = (WINDOW *) 0;
	return (stp->ret);
}

/* routine to initialize the state structure used by the routine */

static void
initstates (window, screenp, structp, firstdesc, stp)
WINDOW	*window;
struct	scrn_parms	*screenp;
struct	scrn_struct	*structp;
register struct	state	*stp;
int	firstdesc;
{
	register struct	scrn_desc	*sdp;	/* for walking sd for screen */
	register int	i;		/* for counting structp */
	int	nstructp;

	stp->itemchanged = 0;
	stp->columninfield = 0;
	stp->insert = 0;
	stp->structp = structp;	/* point this at the right one?  */
	stp->screenp = screenp;
	stp->window = window;
	stp->ret.flags = 0;
	stp->ret.item = 0;

	/*  look for first and last scrn_desc's that are input fields */
	/* just to flag that we haven't changed */
	stp->firstfield = (uchar) 0xff;
	nstructp = 0;
	for (sdp = screenp->sd, i = 0; i < screenp->ndescs; i++, sdp++) {
		if (sdp->type == FLD_PROMPT)  { /* non-input field */
			sdp->inout = FLD_OUTPUT; /* correct user error */
			continue;
		}
		if (sdp->prompt &&
		     sdp->type != FLD_PROMPT &&
		     sdp->type != FLD_CHOICE) {
			restorescreen();
			printf (
			"Field %d had prompt \'%s\' but was not FLD_PROMPT.\n",
			  i, sdp->prompt);
			exit (1);
		}
		if (sdp->type == FLD_CHOICE)
			sdp->inout = FLD_INPUT;  /* correct user error */
		else if (sdp->inout == FLD_OUTPUT)  { /* output only */
			nstructp++;	/* still have structure entry */
			continue;
		}
		if (stp->firstfield == (uchar) 0xff) /* set first desc */
			stp->firstfield = i;
		/* in a menuprompt screen, keep track of last choice field */
		if (screenp->scrntype == SCR_MENUPROMPT) {
			if (sdp->type == FLD_CHOICE)
				stp->lastfield = i;
			else	nstructp++;
		} else {
			stp->lastfield = i;
			nstructp++;
		}
		if (sdp->type == FLD_SCROLL)
			sdp->s_topleft = 0;
	}
	/*  on menu screens, have to start at the top menu
	 *  item, but in fillin screens, can start somewhere
	 *  in the middle */
	if (screenp->scrntype == SCR_MENUPROMPT ||
	     screenp->scrntype == SCR_MENU)
		stp->curfield = stp->firstfield;
	else
		stp->curfield = structp[firstdesc].desc;
	/* initialize pointers from scrn_desc's to structp's  */
	if (structp)
		for (i = 0; i < nstructp; i++)
			screenp->sd[structp[i].desc].scrnstruct = &structp[i];
	stp->message = stp->curfield;
#ifdef DEBUG
fprintf (logfp, "curfield %d lastfield %d firstfield %d.\n",
  stp->curfield, stp->lastfield, stp->firstfield);
#endif
	return;
}

/* key response routines */

/* "down" key in a fillin screen 
 * move to the first field on the next row that has fillin prompts
 */
static void
downfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	int	i, row;

	/* if error return, stay in same field */
	if (leavefield (stp))
		return;
	sdp = sttosd (stp);
	/* if scrolling region, handle separately */
	if (sdp->type == FLD_SCROLL) {
		downscrollreg (stp, sdp);
		return;
	}
	/* move to the first item on the next line */
	if ((i = finddownitem(stp, sdp)) != -1) {
		stp->curfield = i;
		movetofield (stp);
	}
	else	
	/*  no later items -- another choice would be to wrap around to top */
		ringbell();
	return;
}

/* "up" key in a fillin screen */
static void
upfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	int	i, row;

	/* stay in the same field if there was an error */
	if (leavefield (stp))
		return;

	sdp = sttosd (stp);
	if (sdp->type == FLD_SCROLL)  {
		upscrollreg (stp, sdp);
		return;
	}

	if ((i = findupitem (stp, sdp)) != -1)  {
		stp->curfield = i;
		movetofield (stp);
	}  else			/* the other choice would be to wrap */
		ringbell();
	return;
}

/* "left" key in a fillin screen */
static void
leftfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;

	if (stp->columninfield == 0)  {
		ringbell();
		return;
	}
	stp->columninfield--;
	sdp = sttosd (stp);
	/* if there is a move left one character call, use it here */
	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmoveto (stp->window, row, col + stp->columninfield);
	return;
}

/* "right" key in a fillin screen */
static void
rightfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;

	sdp = sttosd (stp);
	if (stp->columninfield == sdp->len - 1)  {  /* right edge */
		ringbell();
		return;
	}
	stp->columninfield++;
	/* if there is a move right one character call, use it here */
	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmoveto (stp->window, row, col + stp->columninfield);
	return;
}

/* "delete word" key */
static void
delword (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	char	*field;		/* first character in word to delete */
	register char
		*lastfield,	/* last position in the field */
		*lastword;	/* first position past what is to delete */
	uchar	breakout;	/* flag for inner loop */

	sdp = sttosd (stp);
	field = stp->scrnrep;
	lastfield = &field[sdp->len];
	field = &field[stp->columninfield];
	/* find last character past this position that isn't SPACE */
	breakout = 0;
	for (lastword = field; lastword < lastfield; lastword++) {
		while ((*lastword == SPACE || *lastword == '\0') &&
		   lastword < lastfield) {
			lastword++;
			breakout = 1;
		}
		if (breakout)
			break;
	}
	delchars (stp, sdp, lastword - field);
	/* reflect that item was changed */
	stp->itemchanged = 1;
	return;
}

/* "backspace" key
 * Delete character to the left of the cursor
 */

static void
backspace (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;

	if (stp->columninfield == 0)  {
		ringbell();
		return;
	}
	sdp = sttosd (stp);
	stp->columninfield--;
	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmoveto (stp->window, row, col + stp->columninfield);
	delchars (stp, sdp, 1);
	/* reflect that item was changed */
	stp->itemchanged = 1;
	return;
}

/* "tab" key in a fillin screen
 * move to next field that is not OUTPUT only
 * wraps around to first field if on the last field
 */
static void
tabfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register int	i;

	/* update the item as changed, if necessary */
	if (leavefield (stp))
		return;
	sdp = sttosd (stp);
	if (sdp->type == FLD_SCROLL) {
		tabscrollreg (stp, sdp);
		return;
	}
	/* move to next field that is input or both field
	 * wrap around to top of screen if necessary
	 */
	if (stp->curfield == stp->lastfield)
		stp->curfield = stp->firstfield;
	else {
		stp->curfield++;
		sdp++;
		for (i = stp->curfield; i <= stp->lastfield; i++, sdp++) {
			if (sdp->type == FLD_PROMPT)
				continue;
			if (sdp->inout == FLD_OUTPUT)
				continue;
			break;
		}
		stp->curfield = i;
	}
	movetofield (stp);
	return;
}

/* "back tab" key in a fillin screen */
static void
backtabfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register int	i;

	/* update the item as changed, if necessary */
	if (leavefield (stp))
		return;
	sdp = sttosd (stp);
	if (sdp->type == FLD_SCROLL) {
		backtabscrollreg (stp, sdp);
		return;
	}
	/* move to previous field that is input or both (wrap) */
	if (stp->curfield == stp->firstfield)
		stp->curfield = stp->lastfield;
	else {
		sdp--;
		stp->curfield--;
		for (i = stp->curfield; i >= stp->firstfield; i--, sdp--) {
			if (sdp->type == FLD_PROMPT)
				continue;
			if (sdp->inout == FLD_OUTPUT)
				continue;
			break;
		}
		stp->curfield = i;
	}
	movetofield (stp);
	return;
}

/* "insert field" key
 * only works in SCROLL fields
 */

static void
insfield(stp)
struct state	*stp;
{
	struct scrn_desc	*sdp;

	sdp = sttosd (stp);
	if (sdp->type != FLD_SCROLL)
		ringbell ();
	else {
		scroll_rshift (stp, sdp);
		stp->itemchanged = 1;
	}
	return;
}

/* "delete field" key
 * only works in ALPHA or NUMBER fields
 */

static void
delfield (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;
	
	sdp = sttosd (stp);
	switch (sdp->type) {
	case FLD_ALPHA:
	case FLD_NUMBER:
	case FLD_SCROLL:
		stp->columninfield = 0;
		if (sdp->type == FLD_SCROLL)
			scr_rowcol (stp, sdp, &row, &col);
		else  {
			row = sdp->row;
			col = sdp->col;
		}
		wmoveto (stp->window, row, col);
		delchars (stp, sdp, sdp->len);
		stp->itemchanged = 1;
		break;
	default:
		ringbell();
		break;
	}
	return;
}

/* "Insert toggle" key */
static void
instoggle (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;
	int	i;

	sdp = sttosd (stp);
	if (sdp->type == FLD_CHOICE)  {
		ringbell();
		return;
	}
	/* move to the place the insert indicator goes */
	wmoveto (stp->window,
		 WINSROW (stp->screenp),
		 WINSCOL (stp->screenp));
	if (stp->insert)  {
		/* remove insert indicator */
		for (i = 0; i < strlen(INSERTIND); i++)
			waddch (stp->window, ' ');
		stp->insert = 0;
	} else {
		/* put insert indicator in */
		addstring (stp->window, INSERTIND);
		stp->insert = 1;
	}
	/* move back to where the cursor was */
	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmoveto (stp->window, row, col + stp->columninfield);
	return;
}

/*  "help" screen display */
static void
help (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;

	sdp = sttosd (stp);
	if (sdp->help == NULL)
		message (stp, stp->window, stp->screenp, NOHELP, NO);
	else
		helpscrn (sdp->help);
	/* necessary becauses curses handles overlapped windows buggily */
	touchwin (stp->window);
	/* move back to original place on the screen */
	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmoveto (stp->window, row, col + stp->columninfield);
	return;
}

/* "redraw screen" key */
static void
redraw (stp)
struct	state	*stp;
{
	clearok (stp->window, TRUE);
	return;
}

/* Entered a regular key in a fillin window */
static void
fillinkey (stp, input)
register struct	state	*stp;
int	input;
{
	register struct	scrn_desc	*sdp;
	uchar	key = input & 0177;
	uchar	row, col;

	sdp = sttosd (stp);
	if (stp->insert)  { /* insert mode */
		/* if at rightmost column, don't allow going off end.
		 * Otherwise, move rest of field to right, truncating
		 * rightmost characters.
		 */
		if (stp->columninfield == sdp->len - 1)  {
			ringbell();
			return;
		}
		moveright (stp, sdp, 1);
	}
	stp->scrnrep[stp->columninfield] = key;
	stp->itemchanged = 1;
	/* assumes adding a character moves the cursor over one position */
	underline (stp->window, ON);
	waddchar (stp->window, key);
	underline (stp->window, OFF);
	if (stp->columninfield == sdp->len - 1) {  /* end of line */
		if (sdp->type == FLD_SCROLL)
			scr_rowcol (stp, sdp, &row, &col);
		else  {
			row = sdp->row;
			col = sdp->col;
		}
		wmoveto (stp->window, row, col + stp->columninfield);
	}
	else	stp->columninfield++;
	return;
}

/* "down" key in a menu screen */
static void
downmenu (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	stp->curfield = finddownitem (stp, sdp);
	sdp = sttosd (stp);
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	return;
}

/* right key in a menu screen */

static void
rightmenu (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	if (stp->curfield == stp->lastfield)  {
		stp->curfield = stp->firstfield;
		sdp = sttosd (stp);
	} else
		for (stp->curfield++, sdp++;
		     stp->curfield <= stp->lastfield;
		     sdp++, stp->curfield++)
			if (sdp->type == FLD_CHOICE)
				break;
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	return;
}

/* "up" key in a menu screen */
static void
upmenu (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	stp->curfield = findupitem (stp, sdp);
	sdp = sttosd (stp);
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	return;
}

/* left key in a menu screen */
static void
leftmenu (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	sdp = sttosd (stp);
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	if (stp->curfield == stp->firstfield)  {
		stp->curfield = stp->lastfield;
		sdp = sttosd (stp);
	} else
		for (stp->curfield--, sdp--;
		     stp->curfield >= stp->firstfield;
		     sdp--, stp->curfield--)
			if (sdp->type == FLD_CHOICE)
				break;
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	return;
}

/* "down" key in a menu prompt screen */

static void
downmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp, *tsdp;
	uchar	maxfield, tfield;

	sdp = sttosd (stp);
	maxfield = stp->screenp->ndescs - 1;
	/*  move back to choice field on this line */
	while (sdp->type != FLD_CHOICE)  {
		sdp--;
		stp->curfield--;
	}
	/* in all cases, we're on the choice prompt */
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	/* clear out non-choice input fields on the current line */
	for (tsdp = sdp + 1, tfield = stp->curfield + 1;
	     tfield <= maxfield &&
	       tsdp->type != FLD_CHOICE && tsdp->row == sdp->row;
	     tsdp++, tfield++)  {
		wmoveto (stp->window, tsdp->row, tsdp->col);
		underline (stp->window, ON);
		putspaces (stp->window, tsdp->len);
		underline (stp->window, OFF);
		clearfield (tsdp);
		tsdp->scrnstruct->changed = 0;
	}
	/* until select a menu item, screen hasn't changed */
	stp->ret.flags |= R_CHANGED;
	/* Move to the choice prompt on the next line, or wrap */
	if (stp->curfield == stp->lastfield)  {
		stp->curfield = stp->firstfield;
		sdp = sttosd (stp);
	} else
		for (sdp++, stp->curfield++; sdp->type != FLD_CHOICE; sdp++)
			stp->curfield++;
	/* we're on proper menu choice field, highlight and see if it has
	 * input fields
	 */
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	if (stp->curfield < maxfield && (++sdp)->type != FLD_CHOICE)  {
		stp->curfield++;
		movetofield (stp);
	} else
		cursor (INVISCURSOR);
	return;
}

/*  "up" key in a menu prompt screen */
static void
upmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp, *tsdp;
	uchar	maxfield, tfield;

	sdp = sttosd (stp);
	maxfield = stp->screenp->ndescs - 1;
	/*  move back to choice field on this line */
	while (sdp->type != FLD_CHOICE)  {
		sdp--;
		stp->curfield--;
	}
	/* in all cases, we're on the choice prompt */
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	/* clear out non-choice input fields on the current line */
	for (tsdp = sdp + 1, tfield = stp->curfield + 1;
	     tfield <= maxfield &&
	       tsdp->type != FLD_CHOICE && tsdp->row == sdp->row;
	     tsdp++, tfield++)  {
		wmoveto (stp->window, tsdp->row, tsdp->col);
		underline (stp->window, ON);
		putspaces (stp->window, tsdp->len);
		underline (stp->window, OFF);
		clearfield (tsdp);
		tsdp->scrnstruct->changed = 0;
	}
	/* until select a menu item, screen hasn't changed */
	stp->ret.flags &= ~R_CHANGED;
	/* Move to the choice prompt on the previous line, or wrap */
	if (stp->curfield == stp->firstfield) {
		stp->curfield = stp->lastfield;
		sdp = sttosd (stp);
	} else
		for (sdp--, stp->curfield--; sdp->type != FLD_CHOICE; sdp--)
			stp->curfield--;
	/* we're on proper menu choice field, highlight and see if it has
	 * input fields
	 */
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	if (stp->curfield < maxfield && (++sdp)->type != FLD_CHOICE)  {
		stp->curfield++;
		movetofield (stp);
	} else
		cursor (INVISCURSOR);
	return;
}

/*  "backtab" key in a menu prompt screen */

static void
backtabmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	if ((sdp-1)->type == FLD_CHOICE)  { /* leftmost fillin field */
		ringbell();
		return;
	}
	if (leavefield (stp))
		return;
	stp->curfield--;
	movetofield (stp);
	return;
}

/*  "tab" key in a menu prompt screen */

static void
tabmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	/* check if rightmost fillin field */
	if ((sdp+1)->type == FLD_CHOICE || (sdp+1)->row != sdp->row)  {
		ringbell();
		return;
	}
	if (leavefield (stp))
		return;
	stp->curfield++;
	movetofield (stp);
	return;
}

/* "left" key in a menu prompt screen */

static void
leftmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	if (sdp->type != FLD_CHOICE)
		leftfill (stp);
	else	ringbell();
	return;
}

/* "right" key in a menu prompt screen
 * works the same way as a left key
 */

static void
rightmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	if (sdp->type != FLD_CHOICE)
		rightfill (stp);
	else	ringbell();
	return;
}
	

/* utility routines used by the main routines: */

/* puts the string in the current screen position.
 * pad out to "length" with spaces.
 */

static void
wpadstring (window, string, length)
WINDOW	*window;
char	*string;
int	length;
{
	register int	i;

	if (strlen (string) > length)
		for (i = 0; i < length; i++, string++)
			waddchar (window, string);
	else {
		addstring (window, string);
		putspaces (window, length - strlen (string));
	}
	return;
}

/* puts the number in the current screen position.
 * pad out to "length" with spaces.
 */

static void
padnumber (window, number, length)
WINDOW	*window;
long	number;
uchar	length;
{
	register int	i;
	char	numbuf[11];  /* largest 32 bit num is 4294967296
						      0123456789 */

	sprintf (numbuf, "%ld", number);
	if (strlen (numbuf) > length)
		for (i = 0; i < length; i++)
			waddchar (window, '#');
	else
		wpadstring (window, numbuf, length);
	return;
}

/* create a new window with the specified attributes.
 * return a WINDOW pointer.
 */

static
WINDOW *
new_window (toprow, leftcol, nbrrows, nbrcols)
uchar	toprow, leftcol, nbrrows, nbrcols;
{
	return (newwin (nbrrows, nbrcols, toprow, leftcol));
}

rm_window (window)
WINDOW	*window;
{
	return (delwin (window));
}

/* clear the specified window (blank it). */

static
clearwindow (window)
WINDOW	*window;
{
	return (wclear (window));
}

/* draw a box around the window, takes up the window's top and bottom rows,
 * and the leftmost and rightmost columns.
 */

static
drawbox (window)
WINDOW	*window;
{
	return (box (window, '|', '-'));
}

/* move to a specific row and column position in a window */
static
wmoveto (window, row, col)
WINDOW	*window;
uchar	row, col;
{
	return (wmove (window, row, col));
}

/* put a string on the window, no padding */
static
addstring (window, string)
WINDOW	*window;
char	*string;
{
	return (waddstr (window, string));
}

/* flag set if hardware supports underline */

/* turn underline mode on or off, if terminal supports it.  Otherwise,
 * use standout mode */
static
underline (window, onoff)
WINDOW	*window;
uchar	onoff;
{
	if (has_underline)
		if (onoff == ON)
			wattron  (window, A_UNDERLINE);
		else	wattroff (window, A_UNDERLINE);
}

/* put the specified number of spaces at the current position in the window */

static void
putspaces (window, number)
WINDOW	*window;
uchar	number;
{
	register int	i;

	for (i = 0; i < number; i++)
		waddchar (window, SPACE);
	return;
}

/* put the specified character in the window at the current position */

static
waddchar (window, character)
WINDOW	*window;
uchar	character;
{
	return (waddch (window, (int) character));
}

/* change the appearance of the cursor */

static void
cursor (cstate)
uchar	cstate;
{
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
}

/* toot, toot */

static
ringbell()
{
	return (beep());
}

/* get a character from the keyboard, and return the char as int */

static
inputch ()
{
	return (getch());
}

/* move to a particular field, given an index into scrn_desc table
 * Assume that any cleanup necessary for old field was done.
 */

static void
movetofield (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register int	i;

	if (stp->message != stp->curfield)  {
		rm_message (stp);
		stp->message = stp->curfield;
	}
	sdp = sttosd(stp);
	switch (sdp->type) {
	case FLD_SCROLL:
		movetoscrollreg (stp, sdp);
		break;
	case FLD_PROMPT:
		message (stp, stp->window, stp->screenp, NOPROMPT, NO);
		wrefresh (stp->window);
		restorescreen();
		exit (1);
	case FLD_CHOICE:
		/* highlight choice field */
		highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
		if (stp->screenp->scrntype == SCR_MENU || 
		    stp->curfield == stp->screenp->ndescs - 1 ||
		    (sdp+1)->type == FLD_CHOICE ||
		    (sdp+1)->type == FLD_PROMPT) {
			cursor (INVISCURSOR);
			break;
		}
		/* otherwise, fall into input field for next item */
		stp->curfield++;
		sdp++;
	default:  /* input field */
		wmoveto (stp->window, sdp->row, sdp->col);
		cursor (BLOCKCURSOR);
		stp->columninfield = 0;
		stp->itemchanged = 0;
		/* prepare screen representation of field */
		switch (sdp->type)  {
		case	FLD_ALPHA:
			strcpy (stp->scrnrep, s_alpha(sdp->scrnstruct));
			break;
		case	FLD_NUMBER:
			sprintf (stp->scrnrep,
				 "%ld",
				 *(s_number(sdp->scrnstruct)));
			break;
		case	FLD_YN:
			stp->scrnrep[0] = *(s_yesno(sdp->scrnstruct)) ?
						YESCHAR : NOCHAR;
			stp->scrnrep[1] = '\0';
			break;
		case	FLD_POPUP:
			stp->scrnrep[0] = *(s_popup(sdp->scrnstruct)) ?
						YESCHAR : NOCHAR;
			stp->scrnrep[1] = '\0';
			break;
		case	FLD_CONFIRM:
			stp->scrnrep[0] = *(s_confirm(sdp->scrnstruct)) ?
						YESCHAR : NOCHAR;
			stp->scrnrep[1] = '\0';
			break;
		}
		/* pad out scrnrep to nulls */
		for (i = strlen (stp->scrnrep); i < sizeof (stp->scrnrep); i++)
			stp->scrnrep[i] = '\0';
		break;
	}
}

/* leave an input field, and store any changed information from
 * the screen representation of the field into the internal representation.
 *
 * returns	0 on success,
 *		1 if error in the field itself
 *		2 if an action in that field should cause a return
 */

static int
leavefield (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	char	*endcp, *begincp, *cp;
	char	thischar;
	char	**stringtab;
	long	atol();
	char	negative;

	sdp = sttosd (stp);
	if (sdp->type == FLD_CHOICE) {
		unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
		return (0);
	}
	if (stp->itemchanged == 0)
		return (0);
	switch (sdp->type)  {
	case	FLD_SCROLL:
		stringtab = s_scrollreg (sdp->scrnstruct);
		cp = fixalpha (stp->scrnrep, sdp->len);
		strcpy (stringtab[stp->scrollitem], cp);
		/* check for a newly-created null slot */
		if (stp->scrollitem != sdp->scrnstruct->filled - 1 &&
		    stringtab[stp->scrollitem][0] == '\0' &&
		    stringtab[stp->scrollitem+1][0] != '\0') {
			scroll_lshift (stp, sdp);
			return (2);
		}
		break;
	case	FLD_YN:
		thischar = stp->scrnrep[0];
		if (thischar == 'y' || thischar == 'Y')
			*(s_yesno(sdp->scrnstruct)) = 1;
		else if (thischar == 'n' || thischar == 'N')
			*(s_yesno(sdp->scrnstruct)) = 0;
		else  {
			message (stp, stp->window, stp->screenp,
			  YESNOERROR, NO);
			wmoveto (stp->window, sdp->row, sdp->col);
			stp->columninfield = 0;
			return (1);
		}
		break;
	case	FLD_CONFIRM:
		thischar = stp->scrnrep[0];
		if (thischar == 'y' || thischar == 'Y')
			*(s_confirm(sdp->scrnstruct)) = 1;
		else if (thischar == 'n' || thischar == 'N')
			*(s_confirm(sdp->scrnstruct)) = 0;
		else  {
			message (stp, stp->window, stp->screenp,
			  YESNOERROR, NO);
			wmoveto (stp->window, sdp->row, sdp->col);
			stp->columninfield = 0;
			return (1);
		}
		stp->ret.flags |= (R_CONFIRM | R_CHANGED);
		stp->ret.item = stp->curfield;
		sdp->scrnstruct->changed = 1;
		return (2);
		break;
	case	FLD_POPUP:
		thischar = stp->scrnrep[0];
		if (thischar == 'y' || thischar == 'Y')
			*(s_popup(sdp->scrnstruct)) = 1;
		else if (thischar == 'n' || thischar == 'N')
			*(s_popup(sdp->scrnstruct)) = 0;
		else  {
			message (stp, stp->window, stp->screenp,
			  YESNOERROR, NO);
			wmoveto (stp->window, sdp->row, sdp->col);
			stp->columninfield = 0;
			return (1);
		}
		stp->ret.flags |= (R_POPUP | R_CHANGED);
		stp->ret.item = stp->curfield;
		sdp->scrnstruct->changed = 1;
		return (2);
		break;
	case	FLD_ALPHA:
		/* fix field to remove NULLs, trailing spaces */
		cp = fixalpha (stp->scrnrep, sdp->len);
		if (cp == NULL)
		{
			s_alpha(sdp->scrnstruct)[0] = '\0';
			return (0);
		}
		strcpy (s_alpha(sdp->scrnstruct), cp);
		break;
	case	FLD_NUMBER:
		begincp = stp->scrnrep;
		/* find last non-null and non-space in field */
		for (endcp = &stp->scrnrep[sdp->len];
		     endcp >= begincp;
		     endcp--)
			if (*endcp && *endcp != SPACE)
				break;
		/* check for nothing in field */
		if (begincp == endcp && *endcp == '\0') {
			*s_number(sdp->scrnstruct) = 0L;
			return (0);
		}
		/* skip leading nulls and spaces */
		for ( ; begincp < endcp; begincp++)
			if (*begincp && *begincp != SPACE)
				break;
		/* check for negative number */
		if (*begincp == '-') {
			negative = 1;
			begincp++;
		} else	negative = 0;
		/* look for non-digits */
		for (cp = begincp; cp <= endcp; cp++)
			if (!isdigit (*cp)) {
				message (stp, stp->window, stp->screenp,
					 NOTNUMBERERROR, NO);
				wmoveto (stp->window, sdp->row, sdp->col);
				stp->columninfield = 0;
				return (1);
			}
		*s_number(sdp->scrnstruct) =
		  negative ? -atol (begincp) : atol (begincp);
		break;
	}  /* end switch */
	stp->ret.flags |= R_CHANGED;
	stp->ret.item = stp->curfield;
	sdp->scrnstruct->changed = 1;
	stp->itemchanged = 0;
	return (0);
}

/* modify an FLD_ALPHA's screen representation such that all internal
 * NULLs are turned to spaces.
 * All trailing spaces are ignored.
 * Returns a pointer to the beginning of the fixed string, or NULL
 * if there were nothing but spaces and NULLs in the string
 */

static
char *
fixalpha (string, len)
char	*string;
int	len;
{
	char	*begincp, *endcp, *cp;

	begincp = string;
	for (endcp = &string[len]; endcp >= string; endcp--)
		if (*endcp && *endcp != SPACE)
			break;
	/* check if nothing in field */
	if (begincp == endcp && *endcp == '\0')
		return (NULL);
	/* place '\0' at end of string (change all spaces to '\0') */
	for (cp = endcp + 1; cp < &string[len]; cp++)
		*cp++ = '\0';
	/* replace '\0' with spaces  - retain leading spaces */
	for (begincp = string; begincp < endcp; begincp++)
		if (*begincp == '\0')
			*begincp = SPACE;
	return (string);
}

/* zeros an input field in its internal representation */
static void
clearfield (sdp)
struct	scrn_desc	*sdp;
{
	char	*cp;
	int	i;

	switch (sdp->type)  {
	case	FLD_ALPHA:
		for (cp = s_alpha(sdp->scrnstruct), i = 0; i < sdp->len; i++)
			*cp++ = '\0';
		break;
	case	FLD_NUMBER:
		*(s_number(sdp->scrnstruct)) = 0L;
		break;
	case	FLD_YN:
		*(s_yesno(sdp->scrnstruct)) = '\0';
		break;
	case	FLD_POPUP:
		*(s_popup(sdp->scrnstruct)) = '\0';
		break;
	case	FLD_CONFIRM:
		*(s_confirm(sdp->scrnstruct)) = '\0';
		break;
	}
	return;
}

/* highlight a prompt on the screen */

static void
highlight (window, row, col, string)
WINDOW	*window;
uchar	row, col;
char	*string;
{
	static	int	first = 1;
	static	int	reverse;

	/* remember if cursor handles reverse video, o.w. just use standout */
	if (first) {
		if (enter_reverse_mode == (char *) 0 ||
		    enter_reverse_mode[0] == '\0')
			reverse = 0;
		else	reverse = 1;
		first = 0;
	}

	if (reverse)
		wattron  (window, A_REVERSE);
	else
		wattron (window, A_STANDOUT);
	wmoveto (window, row, col);
	addstring (window, string);
	if (reverse)
		wattroff (window, A_REVERSE);
	else
		wattroff (window, A_STANDOUT);
	return;
}

/* unhighlight a prompt on the screen (maybe have hardware support?) */

static void
unhighlight (window, row, col, string)
uchar	row, col;
char	*string;
WINDOW	*window;
{
	wmoveto (window, row, col);
	addstring (window, string);
	return;
}
	

/* delete n characters from a field on the screen
 * Leaves the cursor in the same screen position
 */

static void
delchars (stp, sdp, n)
register struct	state		*stp;	/* get columninfield and window */
register struct	scrn_desc	*sdp;	/* which screen description */
int	n;			/* how many columns */
{
	register int	i;
	char	*string, *start;
	char	scrnbuf[80];
	uchar	row, col;

	start = &stp->scrnrep[stp->columninfield];
	string = start;
	for (i = stp->columninfield; i < sdp->len - n; i++)  {
		*string = *(string + n);
		string++;
	}
	/* pad out rest of string with '\0' */
	for (i = 0; i < n; i++)
		*string++ = '\0';
	underline (stp->window, ON);
	wpadstring (stp->window,
		    start,
		    sdp->len - stp->columninfield);
	underline (stp->window, OFF);
	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmoveto (stp->window, row, col + stp->columninfield);
	return;
}

/* move a partial field n characters to the right, starting at
 * the current columninfield
 */
static void
moveright (stp, sdp, n)
register struct	scrn_desc	*sdp;	/* which screen description */
register struct	state		*stp;	/* get columninfield and window */
int	n;			/* how many columns */
{
	register char	*start;
	register int	i;
	uchar	row, col;

	start = &stp->scrnrep[sdp->len - 1];
	for (i = sdp->len - 1; i >= stp->columninfield + n; i--) {
		*start = *(start - n);
		start--;
	}
	start = &stp->scrnrep[stp->columninfield];
	for (i = 0; i < n; i++)
		*start++ = SPACE;
	underline (stp->window, ON);
	wpadstring (stp->window, &stp->scrnrep[stp->columninfield],
		sdp->len - stp->columninfield);
	underline (stp->window, OFF);
	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmoveto (stp->window, row, col + stp->columninfield);
	return;
}

/* find the choice field corresponding to the current input field */

static void
findchoice (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	while (sdp->type != FLD_CHOICE)  {
		sdp--;
		stp->curfield--;
	}
	return;
}

/* find the item on the previous line that is closest to the current column
 * returns the descriptor offset, or -1 if there wasn't one
 */

static int
findupitem (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{
	struct	scrn_desc	*foundsdp;
	int	founddesc;
	register int	i;
	uchar	row, col;

	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else {
		row = sdp->row;
		col = sdp->col;
	}
	/* find first non-prompt on a previous line that is not output only */
	sdp--;			/* start on previous desc */
	foundsdp = NULL;	/* set when found one on previous line */
	founddesc = -1;		/* ditto */
loop:	for (i = stp->curfield - 1; i >= stp->firstfield; i--, sdp--) {
		if (sdp->type == FLD_PROMPT)
			continue;
		/* find CLOSEST input or both field on a previous line */
		if (sdp->row < row && sdp->inout != FLD_OUTPUT) {
			if (foundsdp) { /* already have one on previous line */
				/* Was it 1st on that line? */
				if (sdp->row < foundsdp->row)  /* yes */
					break;
				/* it wasn't 1st on that line */
				/* check if closest to column */
				else if (abs (sdp->col - col) <
					    abs (foundsdp->col - col)) {
					foundsdp = sdp;
					founddesc = i;
				}
			}
			else  {  /* found a candidate for previous line */
				foundsdp = sdp;
				founddesc = i;
			}
		}
	}
	if (founddesc == -1)  {  /* wrap to bottom line */
		stp->curfield = stp->lastfield;
		sdp = sttosd (stp);
		stp->curfield++;	/* to trick for loop at loop: */
		row = sdp->row + 1;
		goto loop;
	}
	return (founddesc);
}

/* find the item on the next row that is closest to the column of the
 * current item.
 * return a curfield value if found, o.w. -1.
 */

static int
finddownitem (stp, sdp)
struct	state	*stp;
struct	scrn_desc	*sdp;
{
	uchar	row, col;
	struct	scrn_desc	*foundsdp;
	int	founddesc;
	int	i;

	if (sdp->type == FLD_SCROLL)
		scr_rowcol (stp, sdp, &row, &col);
	else {
		row = sdp->row;
		col = sdp->col;
	}
	/* find first non-prompt on a later line that is not output only */
	foundsdp = NULL;
	founddesc = -1;
	sdp++;		/* start on next desc */
loop:	for (i = stp->curfield + 1; i <= stp->lastfield; i++, sdp++) {
		if (sdp->type == FLD_PROMPT)
			continue;
		if (sdp->row > row && sdp->inout != FLD_OUTPUT) {
			if (foundsdp) {
				if (sdp->row > foundsdp->row)
					break;
				if (abs (sdp->col - col) <
				    abs (foundsdp->col - col)) {
					foundsdp = sdp;
					founddesc = i;
				}
			} else {
				foundsdp = sdp;
				founddesc = i;
			}
		}
	}
	if (founddesc == -1)  {  /* wrap to top line */
		stp->curfield = stp->firstfield;
		sdp = sttosd (stp);
		stp->curfield--;	/* to trick for loop at loop: */
		row = sdp->row - 1;
		goto loop;
	}
	return (founddesc);
}

/* initialize the screen
 * this routine calls the underlying screen support library.
 */

static
struct	termio	init_termio;
int	has_underline;

initscreen()
{
	unsigned char	save_intr, save_quit, save_bs;
	static	int	first = 1;

	if (find_terminal ()) {
		printf ("Exiting due to find_terminal failure.\n");
		printf ("terminal type \'%s\'\n", getenv ("TERM"));
		exit (1);
	}
#ifdef DEBUG
fprintf (logfp, "Terminal type %s\n", getenv ("TERM"));
#endif
	/* set up terminal interrupt, quit, and erase chars to avoid conflict */
	ioctl (fileno(stdin), TCGETA, &init_termio);
	save_intr = init_termio.c_cc[VINTR];
	save_quit = init_termio.c_cc[VQUIT];
	save_bs =   init_termio.c_cc[VERASE];
	init_termio.c_cc[VINTR] = CTRL('C');
	init_termio.c_cc[VQUIT] = CTRL('B');
	init_termio.c_cc[VERASE] = CTRL('H');
	ioctl (fileno(stdin), TCSETA, &init_termio);
	init_termio.c_cc[VINTR] = save_intr;
	init_termio.c_cc[VQUIT] = save_quit;
	init_termio.c_cc[VERASE] = save_bs;
	
	/* Turn off signal catching now because need a setjmp environment
	 * to recover from a signal.  Signals will be allowed again once
	 * we get into the first putscreen.
	 */
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
	/* just note that hangup happened until we can get around to it */
	signal (SIGHUP, hup_catch);

	initscr();
	cbreak();
	keypad (stdscr, TRUE);
	if ((eighthbit (key_down) || eighthbit (key_up) ||
	    eighthbit (key_left) || eighthbit (key_right)) && has_meta_key) {
#ifdef DEBUG
fprintf (logfp, "eighth bit is set and has meta key\n");
#endif
		if (meta (stdscr, TRUE) == ERR) {
			restorescreen();
			printf (
"Terminal reports eight bits but doesn't have km terminfo capability\n");
			exit (1);
		}
	}
	noecho();
	nonl();
	idlok (stdscr, TRUE);
	/* save state for shell escapes and normal programs */
	def_prog_mode();

	/* figure out whether the terminal can underline */
	/* on non-Xenix, assume has underline unless terminfo says otherwise */
	has_underline = 1;
	/* for those that have underline, make sure that terminfo supports it */
	if (has_underline)
		if (enter_underline_mode == (char *) 0 ||
		    enter_underline_mode[0] == '\0')
			has_underline = 0;
#ifdef DEBUG
fprintf (logfp, "Has_underline: %d\n", has_underline);
#endif
	return;
}

/* look up the terminal type in the TTYTYPE database and ask user
 * the terminal type if not found in TERMINFO database.
 */

static	char	termbuf[5 + ENTSIZ+1];	/* "TERM=" + ENTSIZ name */

static int
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
		tty = malloc (strlen (terminfodir) + 1);
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
				free (terminfodir);
			return (0);
		}
		if (terminfoset) {
			sprintf (buf, "%s/%c/%s", TERMINFODIR, tty[0], tty);
			if (eaccess (buf, 4) == 0) {
				free (terminfodir);
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
					free (terminfodir);
				return (0);
			}
			if (terminfoset) {
				sprintf (buf, "%s/%c/%s",
				  TERMINFODIR, ttytype[0], ttytype);
				if (eaccess (buf, 4) == 0) {
					sprintf (termbuf, "TERM=%s", ttytype);
					putenv (termbuf);
					free (terminfodir);
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
				free (terminfodir);
			return (1);
		}
		buf[ENTSIZ] = '\0';
		strcpy (ttytype, buf);
		found = 1;
	}
}

/*  check if eighth bit is set in any screen description */

static int
eighthbit (string)
char	*string;
{
	unsigned char	c;
	if (string)
		while (c = *string) {
#ifdef DEBUG
fprintf (logfp, "Eighth bit called for string 0x%lx\n", c);
#endif
			if (c & 0x80)
				return (1);
			else	string++;
		}
	return (0);
}

/* undoes the initscreen, restoring the screen to normal input mode */

restorescreen()
{
	clear();
	move (LINES-1, 0);
	/* V.3 doesn't reset cursor properly */
	if (cursor_normal)
		putp (cursor_normal);
	refresh();
	endwin();
	ioctl (fileno(stdin), TCSETA, &init_termio);
	return;
}

/*  put a message in the window at the normal message row and column */

static void
message (stp, window, screenp, string, confirm)
struct	state	*stp;
WINDOW	*window;		/* window to put it in */
struct	scrn_parms *screenp;	/* characteristics of window (row and col) */
char	*string;		/* message itself */
uchar	confirm;		/* whether to ask user to press return first */
{
	int	thischar;
	int	i, count;

	wmoveto (window, WMSGROW (screenp), WMSGCOL);
	/* pad string so it doesn't expand into or past window border */
	waddstr (window, string);
	count = min (LONGESTMSG, screenp->nbrcols - WMSGCOL - 1) -
	  strlen(string);
	if (count > 0)
		if (has_underline)
			putspaces (window, count);
		else
			for (i = 0; i < count; i++)
				waddch (window, ' ');
		
	if (confirm == YES) {
		wmoveto (window, WMSGROW(screenp),
				 WMSGCOL + strlen (string) + 2);
		cursor (BLOCKCURSOR);
		wrefresh (window);
		while (thischar = wgetch(window))  {
			switch (thischar) {
			case ENTER:
			case '\r':
			case '\n':
				goto out;
			case REDRAW:
				clearok (window, TRUE);
				wrefresh (window);
				wmoveto (window, WMSGROW(screenp),
						 WMSGCOL + strlen (string) + 2);
				break;
			default:
				ringbell();
			}
		}
	}
out:
	if (stp)
		stp->message = stp->curfield;
	return;
}

static void
rm_message (stp)
struct	state	*stp;
{
	int	i;

	wmoveto (stp->window, WMSGROW (stp->screenp), WMSGCOL);
	for (i = 0; i < LONGESTMSG; i++)
		waddch (stp->window, ' ');
	return;
}

/*  unsigned char maximum */

static int
min (a, b)
int a, b;
{
	return (a < b ? a : b);
}

/*  correct the way the cursor looks */

static void
correctcursor (stp)
struct	state	*stp;
{
	struct	scrn_desc	*sdp;
	int	cursor_state;

	sdp = sttosd (stp);
	switch (stp->screenp->scrntype) {
	case SCR_MENU:
		cursor_state = INVISCURSOR;
		wmoveto (stp->window, sdp->row, sdp->col + strlen(sdp->prompt));
		break;
	case SCR_MENUPROMPT:
		if (sdp->type != FLD_CHOICE)
			cursor_state = BLOCKCURSOR;
		else if (stp->curfield < stp->screenp->ndescs - 1 &&
		    (sdp+1)->type != FLD_CHOICE)
			cursor_state = BLOCKCURSOR;
		else {
			cursor_state = INVISCURSOR;
			wmoveto (stp->window, sdp->row, sdp->col +
			  strlen(sdp->prompt));
		}
		break;
	default:
		cursor_state = BLOCKCURSOR;
		break;
	}
	cursor (cursor_state);
	return;
}

/* check for required items in a fillin screen */

static int
reqfillin (stp)
struct	state	*stp;
{
	struct	scrn_desc	*sdp;
	int	i;

	sdp = stp->screenp->sd;

	for (i = stp->firstfield; i <= stp->lastfield; i++) {
		if (sdp[i].inout == FLD_OUTPUT ||
		    sdp[i].type == FLD_CHOICE)
			continue;
		if (sdp[i].required)
			if ((sdp[i].scrnstruct->filled == 0 &&
			    sdp[i].scrnstruct->changed == 0)  ||
			   (sdp[i].type == FLD_ALPHA &&
			    sdp[i].scrnstruct->pointer[0] == '\0'))
			{
				stp->curfield = i;
				message (stp, stp->window, stp->screenp,
					 REQUIREDERROR, NO);
				movetofield (stp);
				return (1);
			}
	}
	return (0);
}

/* in a menu prompt field, need to assure that all required fields
 * filled in on that prompt line.
 */

static int
reqmprompt (stp)
struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register int	i;

	sdp = sttosd (stp);

	for (sdp++, i = stp->curfield + 1;
	     i < stp->screenp->ndescs && sdp->type != FLD_CHOICE;
	     i++, sdp++)
		if (sdp->required)
		    	if ((sdp->scrnstruct->filled == 0 &&
			     sdp->scrnstruct->changed == 0) ||
		    	    (sdp->type == FLD_ALPHA &&
		             sdp->scrnstruct->pointer[0] == '\0'))
			{
				stp->curfield = i;
				message (stp, stp->window, stp->screenp,
				 REQUIREDERROR, NO);
				movetofield (stp);
				return (1);
			}
	return (0);
}

/* scrolling region routines */

/* move into a scrolling region -- to top left blank on screen */

static void
movetoscrollreg (stp, sdp)
struct	state		*stp;
struct	scrn_desc	*sdp;
{
	/* saved copy of topleft blank is in sdp */
	stp->topscroll = sdp->s_topleft;
	stp->scrollitem = stp->topscroll;

	/* move to this field in the scrolling region */

	scr_moveto (stp, sdp);

	return;
}

/* tab key in a scrolling region
 * If the current field doesn't have anything in it but the next
 * one does, move other fields over to fill this field and stay
 * in the same field.
 * If the current field and next field are empty, don't allow.
 * Otherwise, move to next sequential field.
 * If on last item of last line, scroll the screen up.
 */

static void
tabscrollreg (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{
	uchar	row, col;
	char	**stringtab;

	/* save off screen representation into string table */
	if (leavefield (stp))
		return;
	stringtab = s_scrollreg (sdp->scrnstruct);
	/* check for last item in table, or for blank item, and move to
	 * next field on screen.
	 */
	if (stp->scrollitem == sdp->scrnstruct->filled - 1 ||
	    stringtab[stp->scrollitem][0] == '\0')  {
		sdp->s_topleft = stp->topscroll;
		if (stp->curfield == stp->lastfield)
			stp->curfield = stp->firstfield;
		else for (++stp->curfield, ++sdp; ; ++stp->curfield, ++sdp)
			if (sdp->inout != FLD_OUTPUT)
				break;
		movetofield (stp);
		return;
	}
	/* ok to move, check if next one is off screen */
	stp->scrollitem++;
	if (scr_offscreen (stp, sdp))  /* below screen */
		scroll_up (stp, sdp);
	scr_moveto (stp, sdp);
	return;
}

/* backtab key in a scrolling region.
 * if on first item, move out of scrolling region.
 * if this is the first item on the screen, scroll the screen down.
 */

static void
backtabscrollreg (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{

	/* save off screen representation into string table */
	if (leavefield (stp))
		return;

	/* if on first item, move to previous field */
	if (stp->scrollitem == 0)  {
		/* save top item on screen */
		sdp->s_topleft = 0;
		if (stp->curfield == stp->firstfield)
			stp->curfield = stp->lastfield;
		else
			for (sdp--, stp->curfield--;
			     sdp->inout == FLD_OUTPUT;
			     sdp--)
				stp->curfield--;
#ifdef DEBUG
fprintf (logfp, "backtabscrollreg: new item %d firstfield %d lastfield %d.\n",
 stp->curfield, stp->firstfield, stp->lastfield);
#endif
		movetofield (stp);
		return;
	}
	/* Check for top of scrolling region */
	if (stp->scrollitem == stp->topscroll)
		scroll_down (stp, sdp);
	stp->scrollitem--;
	scr_moveto (stp, sdp);
	return;
}

/* scroll forward in a scrolling region */
static void
scrolldownkey (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register char			**stringtab;
	register int			new_top;

	sdp = sttosd(stp);
	if (sdp->type != FLD_SCROLL) {
		ringbell();
		return;
	}
	/* check for moving outside range of string table */
	stringtab = s_scrollreg (sdp->scrnstruct);
	new_top = stp->topscroll + sdp->s_itemsperline * sdp->s_lines;
	if (new_top > sdp->scrnstruct->filled - 1
	    || stringtab[new_top][0] == '\0') {
		ringbell();
		return;
	}
	if (leavefield (stp))
		return;
	stp->topscroll = stp->scrollitem = new_top;
	drawscrollreg (stp, sdp);
	scr_moveto (stp, sdp);
	return;
}

/* scroll backward in a scrolling region */
static void
scrollupkey (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd(stp);
	if (sdp->type != FLD_SCROLL) {
		ringbell();
		return;
	}
	if (stp->topscroll == 0) {
		ringbell();
		return;
	}
	if (leavefield (stp))
		return;
	/* check for moving outside range of string table */
	if (stp->topscroll < sdp->s_itemsperline * sdp->s_lines)
		stp->topscroll = 0;
	else	stp->topscroll -= sdp->s_itemsperline * sdp->s_lines;
	stp->scrollitem = stp->topscroll;
	drawscrollreg (stp, sdp);
	scr_moveto (stp, sdp);
	return;
}

/* down key in a scrolling region
 * check that the current field and all intervening fields aren't blank.
 * if so, move out of the scrolling region to the same column on the next line.
 * Otherwise, move down sdp->s_itemsperline fields
 */

static void
downscrollreg (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{
	char	**stringtab;
	register int	i;

	if (leavefield (stp))
		return;

	/* check for moving outside range of string table */
	if (stp->scrollitem + sdp->s_itemsperline >
	     sdp->scrnstruct->filled - 1) {
		sdp->s_topleft = stp->topscroll;
		if (stp->curfield == stp->lastfield) {
			/* other choice would be to wrap */
			ringbell();
			stp->columninfield = 0;
			scr_moveto (stp, sdp);
			return;
		}
		/* find the next non-output field */
		if ((i = finddownitem (stp, sdp)) != -1) {
			stp->curfield = i;
			movetofield (stp);
		} else	ringbell();
		return;
	}
	/* check for intervening blank fields between this one and the end */
	stringtab = s_scrollreg (sdp->scrnstruct);
	/* check for intervening blank fields */
	for (i = 0; i < sdp->s_itemsperline; i++)
		if (stringtab[stp->scrollitem + i][0] == '\0') {
			message (stp, stp->window, stp->screenp,
			  BLANKSCROLL, NO);
			stp->columninfield = 0;
			scr_moveto (stp, sdp);
			return;
		}
	/* move to the field */
	stp->scrollitem += sdp->s_itemsperline;
	if (scr_offscreen (stp, sdp) == 1)
		scroll_up (stp, sdp);
	scr_moveto (stp, sdp);
	return;
}

/*  up key in a scrolling region */

static void
upscrollreg (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{
	int	i;

	if (leavefield (stp))
		return;

	/* check if on top line of scrolling region and on top row */
	if (stp->scrollitem < sdp->s_itemsperline) {
		sdp->s_topleft = stp->topscroll;
		if ((i = findupitem (stp, sdp)) != -1) {
			stp->curfield = i;
			movetofield (stp);
		}
		else	ringbell();
		return;
	}
	/* Check if on top row */
	if (stp->scrollitem - stp->topscroll < sdp->s_itemsperline) {
		scroll_down (stp, sdp);
	}
	stp->scrollitem -= sdp->s_itemsperline;
	scr_moveto (stp, sdp);
	return;
}

/* checks if a particular scroll item is off screen
 * returns:	-1 if off screen above
 *		 0 if on screen
 *		 1 if off screen below
 */

static int
scr_offscreen (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	if (stp->scrollitem < stp->topscroll)
		return (-1);
	if (stp->scrollitem >=
	      stp->topscroll + (sdp->s_itemsperline * sdp->s_lines))
		return (1);
	return (0);
}

/* scroll the scrolling region down one line
 * adjusts stp->topscroll
 */

static void
scroll_down (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	stp->topscroll -= sdp->s_itemsperline;
	drawscrollreg (stp, sdp);
	return;
}

/* scroll the region up one line
 * adjusts stp->topscroll
 */

static void
scroll_up (stp, sdp)
struct	state		*stp;
struct	scrn_desc	*sdp;
{
	stp->topscroll += sdp->s_itemsperline;
	drawscrollreg (stp, sdp);
	return;
}

/* redraws the scrolling region, given stp->topscroll
 * deals with above and below arrow
 */

static void
drawscrollreg (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	char	**stringtab;
	int	temprow, tempcol;
	register int	i, j;
	uchar	item;

	stringtab = s_scrollreg (sdp->scrnstruct);
	stringtab += stp->topscroll;
	item = stp->topscroll;
	for (i = 0, temprow = sdp->row; i < sdp->s_lines; i++, temprow++)  {
		tempcol = sdp->col;
		for (j = 0; j < sdp->s_itemsperline; j++, item++) {
			wmoveto (stp->window, temprow, tempcol);
			underline (stp->window, ON);
			if (item > sdp->scrnstruct->filled - 1)
				putspaces (stp->window, sdp->len);
			else {
				wpadstring (stp->window, *stringtab, sdp->len);
				stringtab++;
			}
			underline (stp->window, OFF);
			tempcol += sdp->len + sdp->s_spaces;
		}
	}
	/*  adjust above and below indicators */
	if (stp->topscroll > 0)
		scr_above_ind (stp, ON);
	else	scr_above_ind (stp, OFF);
	item = stp->topscroll + sdp->s_lines * sdp->s_itemsperline;
	stringtab = s_scrollreg (sdp->scrnstruct);
	if (item < sdp->scrnstruct->filled && stringtab[item][0] != '\0')
		scr_below_ind (stp, ON);
	else	scr_below_ind (stp, OFF);
/*
	wrefresh (stp->window);
*/
	return;
}

/* turn on or off the scroll above indicator
 */

static void
scr_above_ind (stp, onoff)
register struct	state	*stp;
uchar	onoff;
{
	register struct	scrn_desc	*sdp;
	uchar	col;
	int	i;

	sdp = sttosd (stp);
	col = sdp->col + (sdp->s_itemsperline * sdp->len) +
		((sdp->s_itemsperline - 1) * sdp->s_spaces) + 1;
	wmoveto (stp->window, sdp->row, col);
	if (onoff == ON)
		addstring (stp->window, ABOVEIND);
	else {
		for (i = 0; i < strlen (ABOVEIND); i++)
			waddch (stp->window, ' ');
	}
	return;
}

/* turn on or off the scroll below indicator
 */

static void
scr_below_ind (stp, onoff)
register struct	state	*stp;
uchar	onoff;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;
	int	i;

	sdp = sttosd (stp);
	col = sdp->col + (sdp->s_itemsperline * sdp->len) +
		((sdp->s_itemsperline - 1) * sdp->s_spaces) + 1;
	row = sdp->row + sdp->s_lines - 1;
	wmoveto (stp->window, row, col);
	if (onoff == ON)
		addstring (stp->window, BELOWIND);
	else
		for (i = 0; i < strlen (BELOWIND); i++)
			waddch (stp->window, ' ');
	return;
}

/* scr_rowcol reports the row and column associated with the
 * item found in stp->scrollitem
 */

static void
scr_rowcol (stp, sdp, row, col)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
uchar	*row, *col;
{
	uchar	offset;

	offset = stp->scrollitem - stp->topscroll;
	*row = sdp->row + (offset / sdp->s_itemsperline);
	*col = sdp->col + (offset % sdp->s_itemsperline) *
				(sdp->len + sdp->s_spaces);
	return;
}

/* scr_moveto moves to a particular scrolling region item
 */

static void
scr_moveto (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	char	**stringtab;
	uchar	row, col;
	register int	i;

	scr_rowcol (stp, sdp, &row, &col);
	wmoveto (stp->window, row, col);

	/* set up screen representation of item */
	stringtab = s_scrollreg (sdp->scrnstruct);
	strcpy (stp->scrnrep, stringtab[stp->scrollitem]);
	for (i = strlen (stp->scrnrep); i < sizeof (stp->scrnrep); i++)
		stp->scrnrep[i] = '\0';
	stp->columninfield = 0;
	return;
}

/* scrolling region right shift.
 * shift all items right one slot starting at the current item
 * in response to the INSFIELD character
 */

static void
scroll_rshift (stp, sdp)
register struct state		*stp;
register struct scrn_desc	*sdp;
{
	register int	i, item;
	char	**stringtab;

	stringtab = s_scrollreg (sdp->scrnstruct);
	/* make sure there is room */
	if (stringtab[sdp->scrnstruct->filled - 1][0]) {
		message (stp, stp->window, stp->screenp,
			FULLSCROLL, NO);
		stp->columninfield = 0;
		scr_moveto (stp, sdp);
		return;
	}
	item = stp->scrollitem;
	/* prevent insertion if field is empty */
	if (stringtab[item][0] == '\0') {
		message (stp, stp->window, stp->screenp,
			BLANKSCROLL, NO);
		stp->columninfield = 0;
		scr_moveto (stp, sdp);
		return;
	}
	for (i = sdp->scrnstruct->filled - 1; i >= item; --i) {
		if (stringtab[i][0] == '\0')
			continue;
		memcpy(stringtab[i+1], stringtab[i], sdp->len);
		stp->scrollitem = i + 1;
		if (!scr_offscreen (stp, sdp)) {
			scr_moveto (stp, sdp);
			underline (stp->window, ON);
			wpadstring (stp->window, stringtab[i], sdp->len);
			underline (stp->window, OFF);
		}
	}
	stp->scrollitem = item;
	memset (stringtab[item], 0, sdp->len);
	scr_moveto (stp, sdp);
	underline (stp->window, ON);
	putspaces (stp->window, sdp->len);
	underline (stp->window, OFF);
	/* adjust below indicator if necessary */
	item = stp->topscroll + sdp->s_lines * sdp->s_itemsperline;
	if (item < sdp->scrnstruct->filled && stringtab[item][0])
		scr_below_ind (stp, ON);
	else	scr_below_ind (stp, OFF);
	scr_moveto (stp, sdp);
	return;
}

/* scrolling region left shift.
 * typically done when the user clears a blank, causing the next
 * items in the region to be moved back
 */

static void
scroll_lshift (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	ushort	item;
	register int	i, j;
	char	**stringtab;

	stringtab = s_scrollreg (sdp->scrnstruct);
	/* save off item while working on screen */
	item = stp->scrollitem;
	for (i = stp->scrollitem; ; i++, stp->scrollitem++) {
		/* last item is always null field */
		if (i == sdp->scrnstruct->filled - 1)
			stringtab[i][0] = '\0';
		else	strcpy (stringtab[i], stringtab[i+1]);
		/* if on screen, update that blank */
		if (!scr_offscreen (stp, sdp)) {
			scr_moveto (stp, sdp);
			underline (stp->window, ON);
			wpadstring (stp->window, stringtab[i], sdp->len);
			underline (stp->window, OFF);
		}
		/* pad rest of string to spaces */
		for (j = strlen(stringtab[i]); j < sdp->len; j++)
			stringtab[i][j] = '\0';
		/* if just created a blank space, that's the last one */
		if (stringtab[i][0] == '\0')
			break;
	}
	/* restore the current item of scrolling region and move to it */
	stp->scrollitem = item;
	/* adjust below indicator if necessary */
	item = stp->topscroll + sdp->s_lines * sdp->s_itemsperline;
	if (item < sdp->scrnstruct->filled && stringtab[item][0])
		scr_below_ind (stp, ON);
	else	scr_below_ind (stp, OFF);
	scr_moveto (stp, sdp);
	return;
}

/* message screen */

static
struct	scrn_desc	msgdesc[] = {
/* row, col, type, len, inout, required, prompt, help */
{  2, 5, FLD_PROMPT, 0, FLD_OUTPUT, NO, NULL },
{  3, 5, FLD_PROMPT, 0, FLD_OUTPUT, NO, NULL }
};

static
struct	scrn_parms	msgscrn = {
	SCR_MSGONLY,
	10, 5, 7, 70,
	NUMDESCS (msgdesc),
	msgdesc
};

pop_msg (string1, string2)
char	*string1, *string2;
{
	int	len1, len2;
	int	maxlen, lenreturn;

	len1 = strlen (string1);
	if (len1 > COLS - 4) {
		string1[COLS - 4] = '\0';
		len1 = COLS - 4;
	}
	len2 = strlen (string2);
	if (len2 > COLS - 4) {
		string2[COLS - 4] = '\0';
		len2 = COLS - 4;
	}
	maxlen = (len2 > len1) ? len2 : len1;
	/* need 4 characters less than the press return line for |b and b| */
	lenreturn = strlen (PRESSRETURN) + WMSGCOL;
	maxlen = (lenreturn > maxlen) ? lenreturn : maxlen;
	msgscrn.nbrcols = maxlen + 4;
	msgscrn.leftcol = (COLS - msgscrn.nbrcols + 1) / 2;
	msgdesc[0].prompt = string1;
	msgdesc[0].col = (msgscrn.nbrcols - len1 + 1) / 2;
	msgdesc[1].prompt = string2;
	msgdesc[1].col = (msgscrn.nbrcols - len2 + 1) / 2;
	/* putscreen will re-enable signals, which will allow us to break
	 * out if the user interrupts.  Once back here, should ignore signals
	 * again. This is a lower level screen, so we note as such.
	 */
	LowerLevel = 1;
	(void) putscreen (&msgscrn, NULL, POP);
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);
	LowerLevel = 0;
	return;
}

/* routine to look up a help file and construct a help screen.
 * The name of the help screen is used to look up a file containing
 * the help text from a directory of help files, expressed in the
 * global variable HelpDir.
 */

#define HELPFILELEN	256

static void
helpscrn (name)
char	*name;
{
	FILE	*fp;
	char	helpfile[HELPFILELEN];
	struct	stat	sb;
	char	buf[80];
	struct	scrn_parms	scrn;
	char	*cp;
	int	i;
	int	nlines;
	int	widest, thiswide;
	/* these next two are static in case we get interrupted
	 * and need to free memory next time in.
	 */
	static	struct	scrn_desc	*sd = (struct scrn_desc *) 0;
	static	char	*strings = (char *) 0;

	/* clean up from interrupted help screen */
	if (sd != (struct scrn_desc *) 0)
		free ((char *) sd);
	if (strings != (char *) 0)
		free (strings);

	/* This is a lower level screen, so pop up just ONE level */
	LowerLevel = 1;
	sprintf (helpfile, "%s%s", HelpDir, name);
	if (name == NULL || name[0] == '\0' ||
	    stat (helpfile, &sb) < 0 || sb.st_size == 0) {
		sprintf (buf, "Help file %s does not exist or is empty.", name);
		pop_msg (buf, "There is no help available for this item.");
		goto sigs;
	}
	fp = fopen (helpfile, "r");
	if (fp == (FILE *) 0) {
		sprintf (buf,
		 "Help file %s is not readable.",
		 name);
		pop_msg (buf, "There is no help available for this item.");
		goto sigs;
	}
	strings = malloc (sb.st_size);
	if (strings == (char *) 0) {
		pop_msg ("Cannot allocate memory for help screen.",
		"Please report problem and re-run program.");
		fclose (fp);
		goto sigs;
	}
	if (fread (strings, sb.st_size, 1, fp) != 1) {
		pop_msg ("Error reading help file.",
		"Please report problem and re-run program.");
		free (strings);
		strings = (char *) 0;
		fclose (fp);
		goto sigs;
	}
	fclose (fp);
	nlines = 0;
	for (cp = strings, i = 0; i < sb.st_size; i++, cp++)
		if (*cp == '\n') {
			nlines++;
		}
	sd = (struct scrn_desc *) calloc (nlines, sizeof (*sd));
	if (sd == (struct scrn_desc *) 0) {
		pop_msg ("Cannot allocate memory for help screen.",
		"Please report problem and re-run program.");
		free (strings);
		strings = (char *) 0;
		goto sigs;
	}
	cp = strings;
	for (i = 0; i < nlines; i++) {
		sd[i].row = i + 2;
		sd[i].col = 2;
		sd[i].type = FLD_PROMPT;
		sd[i].inout = FLD_OUTPUT;
		sd[i].prompt = cp;
		cp = strchr (cp, '\n');
		*cp++ = '\0';
	}
	widest = 0;
	for (i = 0; i < nlines; i++)
		if ((thiswide = strlen (sd[i].prompt)) > widest)
			widest = thiswide;
	scrn.toprow = (LINES - nlines - 5) / 2;
	scrn.leftcol = (COLS - widest - 4) / 2;
	scrn.nbrrows = nlines + 5;
	scrn.nbrcols = widest + 4;
	scrn.sd = sd;
	scrn.ndescs = nlines;
	scrn.scrntype = SCR_MSGONLY;

	putscreen (&scrn, sd, NULL, POP);
	free (sd);
	sd = (struct scrn_desc *) 0;
	free (strings);
	strings = (char *) 0;
	LowerLevel = 0;
sigs:
	/* signal processing logic - MSGONLY screens
	 * turn off signals, here we need to turn them
	 * back on because we're just accepting input
	 */
	if (hup_caught)
		exit (0);
	(void) signal (SIGINT, int_catch);
	(void) signal (SIGQUIT, quit_catch);
	(void) signal (SIGHUP, SIG_DFL);
	return;
}

#ifdef DEBUG

/* instead of executing a program, just pop up a window that says you
 * would run it "in a real game".
 */

void
popitup (program, argv)
char	*program;
char	*argv[];
{
	char	buf[80];
	char	buf1[80];
	int	i;

	sprintf (buf, "Program run: \'%s\', args:", program);
	fputs (buf, logfp);
	putc ('\n', logfp);
	buf1[0] = '\0';
	while (*argv) {
		strcat (buf1, " ");
		putc (' ', logfp);
		fputs (*argv, logfp);
		if (strlen (buf1) + strlen (*argv) > sizeof (buf1) - 1)
			continue;
		strcat (buf1, *argv);
		argv++;
	}
	pop_msg (buf, buf1);
	putc ('\n', logfp);
	return;
}
#endif

/* copy a screen descriptor table */

struct	scrn_desc *
copy_desc (ndesc, desc_template)
int	ndesc;
struct	scrn_desc	*desc_template;
{
	int	i;
	struct	scrn_desc	*sd;

	if (sd = (struct scrn_desc *) calloc (ndesc, sizeof (*sd)))
		for (i = 0; i < ndesc; i++)
			sd[i] = desc_template[i];
	else {
		pop_msg ("Not enough memory to allocate screen descriptors.",
		"Please report problem and re-run program.");
	}
	return (sd);
}

/* copy a table of scrn_structs */

struct	scrn_struct *
copy_struct (nstruct, struct_template)
int	nstruct;
struct	scrn_struct	*struct_template;
{
	struct	scrn_struct	*sp;
	int	i;

	if (sp = (struct scrn_struct *) calloc (nstruct, sizeof (*sp))) { 
		for (i = 0; i < nstruct; i++) {
			sp[i] = struct_template[i];
			sp[i].filled = 1;
		}
	} else {
		pop_msg ("Not enough memory to allocate screen structures.", 
		"Please report problem and re-run program.");
	}
	return (sp);
}

char **
alloc_cw_table (items, peritem)
int	items;
int	peritem;
{
	char	**table;
	char	*cp;
	int	i;

	/* in the case of nothing allocated, make sure at least one is */
	if (items == 0)
		items = 1;
	table = (char **) calloc (items, sizeof (char *));
	if (table == (char **) 0) {
		pop_msg ("Unable to allocate memory for table.", 
		"Please report problem and re-run program.");
		return ((char **) 0);
	}
	cp = malloc (items * peritem);
	if (cp == (char *) 0) {
		pop_msg ("Unable to allocate memory for table.", 
		"Please report problem and re-run program.");
		free ((char *) table);
		return ((char **) 0);
	}
	for (i = 0; i < items * peritem; i++)
		cp[i] = '\0';
	for (i = 0; i < items; i++, cp += peritem)
		table[i] = cp;
	return (table);
}

void
free_cw_table (table, items)
char	**table;
{
	if (table[0] != (char *) 0)
		free (table[0]);
	free ((char *) table);
	return;
}

char **
expand_cw_table (table, oldsize, newsize, peritem)
char	**table;
int	oldsize;
int	newsize;
int	peritem;
{
	char	*mem, *cp;
	int	i;

	table = (char **) realloc (table, newsize * sizeof (char *));
	if (table == (char **) 0)
		return ((char **) 0);
	mem = realloc (table[0], newsize * peritem);
	if (mem == (char *) 0) {
		free ((char *) table);
		return ((char **) 0);
	}
	cp = mem + (oldsize * peritem);
	for (i = 0; i < (newsize - oldsize) * peritem; i++)
		*cp++ = '\0';
	for (i = 0; i < newsize; i++, mem += peritem)
		table[i] = mem;
	return (table);
}

/* sort the non-null entries of a constant width table */

void
sort_cw_table (table, width, nentries)
char	**table;
int	width;
int	nentries;
{
	int	i;

	for (i = 0; i < nentries; i++)
		if (table[i][0] == '\0')
			break;
	if (i > 0)
		qsort (table[0], i, width, strcmp);
	return;
}

userif_zero()
{
	return (0);
}

userif_one()
{
	return (1);
}

void
userif_void() {}

execute_program_output (program, argv, output_file)
char	*program;
char	**argv;
char	*output_file;
{
	return (execute_program_full (program, argv, (char **) 0,
	  (char *) 0, output_file));
}

execute_program_input (program, argv, input_file)
char	*program;
char	**argv;
char	*input_file;
{
	return (execute_program_full (program, argv, (char **) 0,
	  input_file, (char *) 0));
}

execute_program (program, argv)
char	*program;
char	**argv;
{
	return (execute_program_full (program, argv,
	  (char **) 0, (char *) 0, (char *) 0));
}

/* execute a program, setting the environment variable if envp is non-NULL */

execute_program_env (program, argv, envp)
char	*program;
char	**argv;
char	**envp;
{
	return (execute_program_full (program, argv, envp,
	  (char *) 0, (char *) 0));
}

/* the execute program logic is in this routine.
 * On input to this routine, interrupts are OFF.
 * Turn on interrupts for the child so that it can be subject to the
 * normal action from user signals.
 */

execute_program_full (program, argv, envp, infile, outfile)
char	*program;
char	**argv;
char	**envp;
char	*infile;
char	*outfile;
{
	int	pid;
	int	wait_stat;
	int	ret;
	char	buf[80];

	(void) signal (SIGHUP, SIG_DFL);
	clear();
	move (0,0);
	refresh();
	reset_shell_mode();
	(void) signal (SIGHUP, hup_catch);
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);

	switch (pid = fork())  {
	case	-1:	/* error - can't fork sub-process */
		ret = INTERRUPT;
#ifdef DEBUG
		fprintf (logfp, "Fork returned -1\n");
#endif
		break;
	case	0:	/* child */
		(void) signal (SIGINT, SIG_DFL);
		(void) signal (SIGQUIT, SIG_DFL);
		(void) signal (SIGHUP, SIG_DFL);
		if (outfile != (char *) 0)
			freopen (outfile, "w", stdout);
		if (infile != (char *) 0)
			freopen (infile, "r", stdin);
		if (envp == (char **) 0) {
			if (execv (program, argv) < 0)  {
				fprintf (stderr, "execv of \'%s\' failed\n",
				  program);
				exit (INTERRUPT);
			}
		}
		else  {
			/* when sanitizing environment, reset to real owner */
			setgid(getgid());
			if (execve (program, argv, envp) < 0) {
				fprintf (stderr, "execve of \'%s\' failed\n",
				  program);
				exit (INTERRUPT);
			}
		}
	default:
		wait (&wait_stat);
		/* if hangup while program was running, no need to do output
		 * to the screen
		 */
		(void) signal (SIGHUP, SIG_DFL);
		if (hup_caught == 0) {
			printf ("\nPress <RETURN> to continue: ");
			fflush (stdout);
			gets (buf);
			reset_prog_mode();
		}
		(void) signal (SIGHUP, hup_catch);
#ifdef DEBUG
		fprintf (logfp, "wait returned %d for process %d\n",
			wait_stat,pid);
#endif
		if (wait_stat & 0xFF == 0)  /* terminated due to exit */
			ret =  (wait_stat >> 8) & 0xFF; /* exit status */
		else if ((wait_stat & 0xFF00) == 0) /* terminated by signal */
			ret =  INTERRUPT;
		/* for now, program logic always assumes that the program
		 * terminated.  Not much higher level routines can do anyway.
		 */
		ret = INTERRUPT;
		break;
	}
	return (ret);
}

FILE *
popen_all_output (program, argv)
char	*program;
char	*argv[];
{
	int	pipefd[2];
	int	pid;

	/* interrupts are always off here */
	if (pipe (pipefd) < 0)
		return ((FILE *) 0);
	switch (pid = fork())  {
		case -1:
			return ((FILE *) 0);
		case 0:
			/* close the reader end */
			close (pipefd[0]);
			/* make the stdout and stderr go up the pipe */
			close (fileno (stdout));
			dup (pipefd[1]);
			close (fileno (stderr));
			dup (pipefd[1]);
			close (pipefd[1]);
			if (execv (program, argv) < 0)
				exit (0);
		default:
			/* close the writer end */
			close (pipefd[1]);
			return (fdopen (pipefd[0], "r"));
	}
}

void
pclose_all_output (fp)
FILE	*fp;
{
	int	wait_stat;

	while (wait (&wait_stat) == -1)
		;
	fclose (fp);
}

/* 
 * This function is called from all screen routines to do the
 * logic of driving putscreen and getscreen.  It also does
 * necessary signal handling.
 */

scrnfunc (argv, fill, validatefunc, bstructfunc, bdescfunc, bfillfunc,
	  copyscreenfunc, freetabsfunc, actionfunc, firstdesc, nscrnstruct,
	  parmtemplate, desctemplate, structtemplate)
char	**argv;
char *fill;
int                  (*validatefunc)();
int                  (*bfillfunc)();
struct scrn_struct * (*bstructfunc)(), *structtemplate;
struct scrn_desc *   (*bdescfunc)(), *desctemplate;
struct scrn_parms *  (*copyscreenfunc)(), *parmtemplate;
void                 (*freetabsfunc)();
int                  (*actionfunc)();
int                  firstdesc;
int                  nscrnstruct;
{
	struct	scrn_struct	*sp, *tsp;
	struct	scrn_desc	*sd, *tsd;
	struct	scrn_parms	*screenp;
	WINDOW	*window;
	int	first_desc;
	int	i;
	int	ret;
	struct	scrn_ret	scrn_ret;
	short	changed;	/* couldn't resist */
	int	scrn_changed;
	extern	jmp_buf	env;

	/* turn off interrupts while data structures being built.
	 * putscreen will turn them back on.
	 */
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);

	if ((*validatefunc) (argv, fill) || (*bfillfunc) (fill))
		return (1);
	
	sp = NULL; screenp = NULL; sd = NULL;

	if ((screenp = (*copyscreenfunc) (fill, parmtemplate)) == NULL ||
	    (sd = (*bdescfunc) (fill, desctemplate, structtemplate)) == NULL ||
	    (sp = (*bstructfunc) (fill, structtemplate)) == NULL) {
		(*freetabsfunc) (fill, screenp, sd, sp);
		return (1);
	}

	screenp->sd = sd;
	first_desc = firstdesc;

	if (screenp->scrntype == SCR_MSGONLY)  {
		/* trap signals */
		if (ret = setjmp (env)) {
			(*freetabsfunc) (fill, screenp, sd, sp);
			return (ret);
		}

	/* for message only screens, don't worry about entry and validating */
		window = putscreen (screenp, sp, POP);
		(*freetabsfunc) (fill, screenp, sd, sp);
		return (1);
	}

	scrn_changed = 0;
	while (first_desc != -1) {
		/* trap signals */
		if (ret = setjmp (env)) {
			if (ret == CONTINUE)
				continue;
			else {
				(*freetabsfunc) (fill, screenp, sd, sp);
				return (ret);
			}
		}

		window = putscreen (screenp, sp, POP);
		scrn_ret = getscreen (window, screenp, sp, first_desc);
		if ((scrn_ret.flags & R_ABORTED) ||
		    (scrn_ret.flags & R_QUIT)) {
			(*freetabsfunc) (fill, screenp, sd, sp);
			return (1);
		}
		/*
		 * for each changed field, change desc so it will
		 * display next time.  Also, need to propogate any
		 * "fixed" changes to action will continue.
		 */
		for (i = 0; i < nscrnstruct; i++)
			if (sp[i].changed) {
				if (!sp[i].filled)
					sp[i].filled = 1;
				sd[sp[i].desc].inout = FLD_BOTH;
				scrn_changed = 1;
			}
		/*
		 * now deal with changes to the screen, if any were reported
		 */
		if (!(scrn_ret.flags & R_CHANGED)) {
			if (!scrn_changed) { /* nothing previously changed */
				(*freetabsfunc) (fill, screenp, sd, sp);
				return (1);
			}
		} else	scrn_changed = 1;

		/* walk through and run validation routines */

		changed = 0;
		for (i = 0, tsp = sp; i < nscrnstruct; i++, tsp++)  {
			tsd = &sd[tsp->desc];
			if (tsp->changed && tsd->inout != FLD_OUTPUT)
				if ((*tsp->validate) (fill)) {
					first_desc = tsp - sp;
					changed = 1;
					break;
				}
		}
		if (!changed)	/* everything validated */
			first_desc = -1;  /* break out of while loop */
	}  /* end while */

	/* turn off interrupts: screen action must proceed once user commits.
	 */

	(void) signal (SIGHUP, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);
	(void) signal (SIGINT, SIG_IGN);

	ret = (*actionfunc) (fill);
	(*freetabsfunc) (fill, screenp, sd, sp);

	return (ret);
}

/*
 * The following code is for the debug of allocated memory.
 * It maintains a circular buffer of malloc records that allow you
 * to walk back through malloc history and match allocations with release.
 */

#ifdef DEBUG_MALLOC

#undef malloc
#undef realloc
#undef calloc
#undef free
#undef strdup

extern char *malloc(), *realloc(), *calloc(), *strdup();
extern void free();

#define MALLOC_CIRC_SIZE 100

struct malloc_circ {
	int size;
	char *ptr1;
	char *ptr2;
	char func[4];
} malloc_circ[MALLOC_CIRC_SIZE];
int nmalloc_circ;

char *
dbmalloc(bytes)
int bytes;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];
	char *cp;

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	cp = malloc (bytes);
	mp->size = bytes;
	mp->ptr1 = cp;
	mp->ptr2 = (char *) 0;
	strcpy (mp->func, "mal");
	return(cp);
}

char *
dbrealloc(pointer, newsize)
char *pointer;
int newsize;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];
	char *cp;

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	cp = realloc (pointer, newsize);
	mp->size = newsize;
	mp->ptr1 = pointer;
	mp->ptr2 = cp;
	strcpy (mp->func, "rea");
	return(cp);
}

char *
dbcalloc(nelem, elsize)
int nelem;
int elsize;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];
	char *cp;

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	cp = calloc (nelem, elsize);
	mp->size = nelem*elsize;
	mp->ptr1 = cp;
	mp->ptr2 = (char *) 0;
	strcpy (mp->func, "cal");
	return(cp);
}

void
dbfree(pointer)
char *pointer;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	free(pointer);
	mp->size = 0;
	mp->ptr1 = pointer;
	mp->ptr2 = (char *) 0;
	strcpy(mp->func, "fre");
}

char *
dbstrdup(pointer)
char *pointer;
{
	struct malloc_circ *mp = &malloc_circ[nmalloc_circ];
	char *cp;

	if (++nmalloc_circ == MALLOC_CIRC_SIZE)
		nmalloc_circ = 0;
	/* note: some systems do not have strdup */
	cp = malloc(strlen(pointer) + 1);
	if (cp) {
		strcpy(cp, pointer);
		mp->size = strlen(pointer);
		mp->ptr1 = cp;
		mp->ptr2 = (char *) 0;
		strcpy(mp->func, "dup");
	}
	return(cp);
}
#endif
