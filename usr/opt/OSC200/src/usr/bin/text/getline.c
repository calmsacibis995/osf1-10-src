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
static	char	*sccsid = "@(#)$RCSfile: getline.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/09/21 17:11:45 $";
#endif lint
/*
 */
/*
 * Modification history
 * ~~~~~~~~~~~~~~~~~~~~
 * 001  Tom Woodburn, 21 Apr 1991
 *	- Added modification history.
 *	- Included curses.h if OSF, cursesX.h if not.
 *	- Replaced strings with calls to catgets(3).
 *	- Changed copyright notice to DIGITAL_COPYRIGHT macro.
 *
 */

#ifdef OSF		/* TJW 001 */
#include <curses.h>
#else
#include <cursesX.h>
#endif
#include <ctype.h>
#include <signal.h>
#include "trans_msg.h"

extern nl_catd catd;

/* 
 * definitions for the key editing when in translate mode
 */

#define BSPACE	  010		/* ^h - backspace	*/
#define FSPACE	  014		/* ^l - forward space	*/

#define DELCHAR	 0177		/* DEL- delete previous */
#define DELINPUT  025		/* ^u - input kill*/

#define BWORD	  027		/* ^w - back word	*/
#define FWORD	  006		/* ^f - forward word	*/

#define INSERT    011		/* ^i - insert mode     */
#define REPLACE	  022		/* ^r - replace mode	*/

#define ENDINPUT   005		/* ^e - end line	*/
#define STARTINPUT 002		/* ^b - start of line	*/

#define NEXTLINE  016		/* ^n - next line	*/
#define PREVLINE  020		/* ^p - previous line	*/

#define KEYHELP	  013		/* ^k - show keys (help)*/

#define EXIT	  012		/* ^j - end of input	*/

#define EOT	  004		/* ^d - end of file     */
#define INTR	  003		/* ^c - interrupt       */

/*
 * getline()
 * 	Read a line from a window doing editing. The key definitions 
 *	above control the editing available. If quote is set non-zero
 *	then this represents a floating end character that CANNOT be
 *	deleted or altered.
 *
 *	NOTE: To make this function clearer macros are heavily used in
 *	      a way similar to PASCALs subfunctions. This is to save us
 *	      messing with lots of global variables!!
 */

/* 
 * following are several macros which are used in the getline function
 */


#define fchar(ptr) { 	/* skip forward one character */  \
		ptr++; \
		if (*ptr == '\n') { \
			cury++; \
			curx = 0; \
			ptr++; \
		} else \
			curx++; \
}

#define bchar(ptr) {	/* step backward one character */ \
		if (*--ptr == '\n')  { \
			ptr--; \
			cury--; \
			setcol(ptr); \
		} \
		curx--; \
}

			/* find start column of line */
#define startcol(line) (line == starty ? startx : 0)


#define setcol(ptr) { 	/* set curx to offset from last newline */ \
		curx = startcol(cury); \
		for (tp = ptr; tp >= line && *tp != '\n'; tp--) \
			curx++; \
}

getline(win, helpwin, line, len, quote)
WINDOW *win, *helpwin;
char *line;
int len;
int quote;
{	char *cp;		/* our current position			*/
	char *eol;		/* the end of the line			*/
	int   quotebias = 0;	/* used to stop overwriting quote	*/
	char *tp;
	int   mode = INSERT;
	int   redraw = FALSE;	/* true if we must redraw the screen	*/
	int   c;
	int   starty, startx, cury, curx, maxy;

	inputmode(catgets(catd, MS_GETLINE, M_GETLINE_1, "INSERT"));

	keypad(win, TRUE);
	getyx(win, starty, startx);	
	getyx(win, cury, curx);
	maxy = cury;

	cp = eol = line;
	if (quote) {
		quotebias = 1;
		*eol++ = quote;
		redraw = TRUE;
	}
	
	for(;;) {
		if (redraw)   {	/* redraw the screen */
			wmove(win , starty, startx);
			wclrtobot(win); 
			*eol = '\0';
			waddstr(win, line);
			wmove(win, cury, curx);
			redraw = FALSE;
		}
		wrefresh(win);
		c = wgetch(win);
		switch(c) {

		case EXIT:
			/*
			 * if previous char was back slash then this is an
			 * escaped newline so drop through to add it to the
			 * string. Otherwise return.
			 */
			if (cp == line || *(cp - 1) != '\\') {
				*eol = '\0';
				return;
			}
			/* drop through */

		default:
			/* 
			 * take care at end of string not to overwrite quote
			 */
			if (mode == INSERT || cp >= eol - quotebias) {
				/*
				 * check for buffer overflow
				 */
				if (eol - quotebias == line + len) {
					beep();
					break;
				}
				/* 
				 * move rest of line up to make space 
			  	 */
				for (tp = eol++; tp >= cp; tp--)
					tp[1] = tp[0];

				*cp++ = c;
				curx++;
				winsch(win, c);
				wmove(win, cury, curx);
			} else  {	/* replace mode */
				*cp++ = c;
				curx++;
				waddch(win, c);
			}
			/*
			 * if character was a newline redo the display since
			 * addch and insch functions hack the screen
			 */
			if (c == '\n') {
				maxy = ++cury;
				curx = 0;
				redraw = TRUE;
			}
			break;

		case REPLACE:
		case INSERT:
			mode = c;
			inputmode(mode == REPLACE ?
			    catgets(catd, MS_GETLINE, M_GETLINE_2, "REPLACE") :
			    catgets(catd, MS_GETLINE, M_GETLINE_1, "INSERT"));
			break;

		case FSPACE:
		case KEY_RIGHT:
			if (cp == eol - quotebias)
				beep();
			else {
				fchar(cp);
				wmove(win, cury, curx);
			}
			break;

		case BSPACE:
		case KEY_LEFT:
			if (cp > line) {
				bchar(cp);
				wmove(win, cury, curx);
			} else
				beep();
			break;

		case DELCHAR:
			if (cp == line)
				beep();
			else {
				/* 
				 * save character in case a newline
				 */
				c = *(cp - 1);
				/* 
				 * move line down to remove character 
				 */
				cp--;
				eol--;
				for (tp = cp; tp < eol; tp++)
					tp[0] = tp[1];
				if (c == '\n') {
					cury--;
					maxy--;
					setcol(cp - 1);
					redraw = TRUE;
				} else {
					wmove(win, cury, --curx);
					wdelch(win);
				}
			}
			break;

		case DELINPUT:
			maxy = cury = starty;
			curx = startx;
			cp = eol = line;
			if (quote)
				*eol++ = quote;
			redraw = TRUE;
			break;

		case STARTINPUT:
			wmove(win, starty, startx);
			cury = starty;
			curx = startx;
			cp = line;
			break;

		case ENDINPUT:
			cury = maxy;
			cp = eol - quotebias;
			setcol(cp);
			wmove(win, maxy, --curx);
			break;

		case BWORD:
			/*
			 * skip to the start of the previous word
			 */
			if (cp > line)
				bchar(cp);

			while (cp > line && isspace(*cp))
				bchar(cp);

			while (cp > line && ! isspace(*cp))
				bchar(cp);
			
			if (cp > line && cp < eol)
				fchar(cp);

			wmove(win, cury, curx);
			break;

		case FWORD:
			/*
			 * skip to start of next word 
			 */
			while (cp < eol - quotebias && ! isspace(*cp)) 
				fchar(cp);

			while (cp < eol - quotebias && isspace(*cp))
				fchar(cp);

			wmove(win, cury, curx);
			break;

		case NEXTLINE:
		case KEY_DOWN:
			if (cury == maxy)
				beep();
			else { 
				curx = 0;
				wmove(win, ++cury, curx);
				while (*cp++ != '\n')
					;
			}
			break;

		case PREVLINE:
		case KEY_UP:
			if (cury == starty)
				beep();
			else {
				cury--;
				curx = startcol(cury);
				wmove(win, cury, curx);
				while (*--cp != '\n')
					;
			}
			break;

		case EOT:
			windup(0);
			break;

		case INTR:		/* simulate interrupt		*/
			windup(SIGINT);
			break;

		case KEYHELP:
		case KEY_F(15):		/* DEC standard mapping		*/
			werase(helpwin);
			wprintw(helpwin, catgets(catd, MS_GETLINE, M_GETLINE_3,
"^h  back space    ^l  forward space   ^w  back word    ^f forward word\n"));
			wprintw(helpwin, catgets(catd, MS_GETLINE, M_GETLINE_4,
"^e  end of input  ^b  start of input  ^n  next line    ^p previous line\n"));
			wprintw(helpwin, catgets(catd, MS_GETLINE, M_GETLINE_5,
"DEL delete char   ^u  delete input    ^i  insert mode  ^r replace mode\n"));
			wprintw(helpwin, catgets(catd, MS_GETLINE, M_GETLINE_6,
"^d  exit          ^c  quit"));
			wrefresh(helpwin);
			break;
		}

	}
}

