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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: screen.c,v $ $Revision: 4.2.7.7 $ (DEC) $Date: 1993/12/21 21:08:48 $";
#endif
/*
 * HISTORY
 */
/*
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)screen.c	5.8 (Berkeley) 6/28/92";
#endif /* not lint */

/*
 * Routines which deal with the characteristics of the terminal.
 * Uses termcap to be as terminal-independent as possible.
 *
 * {{ Someday this should be rewritten to use curses. }}
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "less.h"

#if TERMIO
#include <termio.h>
#else
#include <sgtty.h>
#endif

#include <sys/ioctl.h>

extern int tty;

/*
  Possible codes needed:
  column_address	   hpa	   ch	  Sets cursor column. (PG)
  cursor_to_ll		   ll	   ll	  Moves	cursor to first	column of last
					  line (if no cup).
  cursor_down		   cud1	   do	  Moves	cursor down one	line.
*/

/*
 * Strings passed to tputs() to do various terminal functions.
 */
static  char
	*sc_pad,		/* Pad string */
	*sc_home,		/* Cursor home */
	*sc_addline,		/* Add line, scroll down following lines */
	*sc_lower_left,		/* Cursor to last line, first column */
	*sc_move,		/* General cursor positioning */
	*sc_clear,		/* Clear screen */
	*sc_eol_clear,		/* Clear to end of line */
	*sc_s_in,		/* Enter standout (highlighted) mode */
	*sc_s_out,		/* Exit standout mode */
	*sc_u_in,		/* Enter underline mode */
	*sc_u_out,		/* Exit underline mode */
	*sc_b_in,		/* Enter bold mode */
	*sc_b_out,		/* Exit bold mode */
	*sc_backspace,		/* Backspace cursor */
	*sc_init,		/* Startup terminal initialization */
	*sc_deinit,		/* Exit terminal de-intialization */
        *sc_last_line,		/* Go to first col of previous line */
        *sc_cur_down,		/* Move directly down one row */
        *sc_cur_up,		/* Move cursor directly up one row. */
        *sc_carriage_ret,	/* Carriage return w/o linefeed. */
        *sc_parm_delete_line,	/* Delete line */
        *sc_parm_insert_line,	/* Insert line */
        *sc_col_adr;		/* Direct column positioning directive */

static char *bold_on="\033[1m";
static char *under_on="\033[4m";
static char *highl_on="\033[7m";
static char *mode_off="\033[0m";
static char *mode_off_1="\033[m";
struct edata			/* Escape sequence table */
{
	char line[12];
	int  num;
} e_table[20];
int e_table_cnt;
int auto_wrap;			/* Terminal does \r\n when write past margin */
int ignaw;			/* Terminal ignores \n immediately after wrap */
				/* The user's erase and line-kill chars */
int erase_char, kill_char, werase_char;
int sc_width, sc_height = -1;	/* Height & width of screen */
int sc_window = -1;		/* window size for forward and backward */
int sc_window_set = 0;		/* Set '1' when sc_window is set */
int bo_width, be_width;		/* Printing width of boldface sequences */
int ul_width, ue_width;		/* Printing width of underline sequences */
int so_width, se_width;		/* Printing width of standout sequences */
int use_tite = 1;		/* use sc_init and sc_deinit */

/*
 * These two variables are sometimes defined in,
 * and needed by, the termcap library.
 * It may be necessary on some systems to declare them extern here.
 */
/*extern*/ short ospeed;	/* Terminal output baud rate */
/*extern*/ char PC;		/* Pad character */

extern int back_scroll;
extern void init_table(); /* GZ001 */
char *tgetstr();
char *tgoto();

/*
 * Change terminal to "raw mode", or restore to "normal" mode.
 * "Raw mode" means 
 *	1. An outstanding read will complete on receipt of a single keystroke.
 *	2. Input is not echoed.  
 *	3. On output, \n is mapped to \r\n.
 *	4. \t is NOT expanded into spaces.
 *	5. Signal-causing characters such as ctrl-C (interrupt),
 *	   etc. are NOT disabled.
 * It doesn't matter whether an input \n is mapped to \r, or vice versa.
 */
void
raw_mode(int on)
{
#if TERMIO
	struct termio s;
	static struct termio save_term;

	if (on)
	{
		/*
		 * Get terminal modes.
		 */
		(void)ioctl(tty, TCGETA, &s);

		/*
		 * Save modes and set certain variables dependent on modes.
		 */
		save_term = s;
		ospeed = s.c_cflag & CBAUD;
		erase_char = s.c_cc[VERASE];
		kill_char = s.c_cc[VKILL];
		werase_char = s.c_cc[VWERASE];

		/*
		 * Set the modes to the way we want them.
		 */
		s.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
		s.c_oflag |=  (OPOST|ONLCR|TAB3);
		s.c_oflag &= ~(OCRNL|ONOCR|ONLRET);
		s.c_cc[VMIN] = 1;
		s.c_cc[VTIME] = 0;
	} else
	{
		/*
		 * Restore saved modes.
		 */
		s = save_term;
	}
	(void)ioctl(tty, TCSETAW, &s);
#else
	struct sgttyb s;
	struct ltchars l;
	static struct sgttyb save_term;

	if (on)
	{
		/*
		 * Get terminal modes.
		 */
		(void)ioctl(tty, TIOCGETP, &s);
		(void)ioctl(tty, TIOCGLTC, &l);

		/*
		 * Save modes and set certain variables dependent on modes.
		 */
		save_term = s;
		ospeed = s.sg_ospeed;
		erase_char = s.sg_erase;
		kill_char = s.sg_kill;
		werase_char = l.t_werasc;

		/*
		 * Set the modes to the way we want them.
		 */
		s.sg_flags |= CBREAK;
		s.sg_flags &= ~(ECHO|XTABS);
	} else
	{
		/*
		 * Restore saved modes.
		 */
		s = save_term;
	}
	(void)ioctl(tty, TIOCSETN, &s);
#endif
}

/*
 * Get terminal capabilities via termcap.
 */
void
get_term(void)
{
	char termbuf[2048];
	char *sp;
	char *term;
	int hard;
	struct winsize w;
	static char sbuf[1024];

	/*
	 * Find out what kind of terminal this is.
	 */
 	if ((term = getenv("TERM")) == NULL)
 		term = "unknown";
 	if (tgetent(termbuf, term) <= 0)
 		(void)strcpy(termbuf, "dumb:co#80:hc:");

	/*
	 * Get size of the screen.
	 */
	if (ioctl(tty, TIOCGWINSZ, &w) == 0 && w.ws_row > 0)
		sc_height = w.ws_row;
	else
		sc_height = tgetnum("li");
	hard = (sc_height < 0 || tgetflag("hc"));
	if (hard) {
		/* Oh no, this is a hardcopy terminal. */
		sc_height = 24;
	}
	/* GZ001 Update table with new sc_height been set */
	init_table();
	

	if (!sc_window_set && (sp = getenv("LINES")) != NULL) 
		sc_window = atoi(sp);

	if ((sp = getenv("COLUMNS")) != NULL)
		sc_width = atoi(sp);
 	else if (ioctl(tty, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
		sc_width = w.ws_col;
	else
 		sc_width = tgetnum("co");
 	if (sc_width < 0)
  		sc_width = 80;

	auto_wrap = tgetflag("am");
	ignaw = tgetflag("xn");

	/*
	 * Assumes termcap variable "sg" is the printing width of
	 * the standout sequence, the end standout sequence,
	 * the underline sequence, the end underline sequence,
	 * the boldface sequence, and the end boldface sequence.
	 */
	if ((so_width = tgetnum("sg")) < 0)
		so_width = 0;
	be_width = bo_width = ue_width = ul_width = se_width = so_width;

	/*
	 * Get various string-valued capabilities.
	 */
	sp = sbuf;

	sc_pad = tgetstr("pc", &sp);
	if (sc_pad != NULL)
		PC = *sc_pad;

	sc_init = tgetstr("ti", &sp);
	if (sc_init == NULL)
		sc_init = "";

	sc_deinit= tgetstr("te", &sp);
	if (sc_deinit == NULL)
		sc_deinit = "";

	sc_eol_clear = tgetstr("ce", &sp);
	if (hard || sc_eol_clear == NULL || *sc_eol_clear == '\0')
	{
		sc_eol_clear = "";
	}

	/* Add the following string definitions to support the */
	/* move_bol() function. */
	if ((sc_parm_delete_line = tgetstr("dl", &sp)) == NULL) {
	   sc_parm_delete_line = "";
	}

        if ((sc_parm_insert_line = tgetstr("il", &sp)) == NULL) {
	   sc_parm_insert_line = "";
	}

	if ((sc_last_line = tgetstr("ll", &sp)) == NULL) {
	   sc_last_line = "";
	}

	if ((sc_col_adr = tgetstr("ch", &sp)) == NULL) {
	   sc_col_adr = "";
	}

	if ((sc_cur_down = tgetstr("do", &sp)) == NULL) {
	   sc_cur_down = "";
	}

	if ((sc_carriage_ret = tgetstr("cr", &sp)) == NULL) {
	   sc_carriage_ret = "";
	}

	if ((sc_cur_up = tgetstr("cuu1", &sp)) == NULL) {
	   sc_cur_up = "";
	}

	sc_clear = tgetstr("cl", &sp);
	if (hard || sc_clear == NULL || *sc_clear == '\0')
	{
		sc_clear = "\n\n";
	}

	sc_move = tgetstr("cm", &sp);
	if (hard || sc_move == NULL || *sc_move == '\0')
	{
		/*
		 * This is not an error here, because we don't 
		 * always need sc_move.
		 * We need it only if we don't have home or lower-left.
		 */
		sc_move = "";
	}

	sc_s_in = tgetstr("so", &sp);
	if (hard || sc_s_in == NULL)
		sc_s_in = "";

	sc_s_out = tgetstr("se", &sp);
	if (hard || sc_s_out == NULL)
		sc_s_out = "";

	sc_u_in = tgetstr("us", &sp);
	if (hard || sc_u_in == NULL)
		sc_u_in = sc_s_in;

	sc_u_out = tgetstr("ue", &sp);
	if (hard || sc_u_out == NULL)
		sc_u_out = sc_s_out;

	sc_b_in = tgetstr("md", &sp);
	if (hard || sc_b_in == NULL)
	{
		sc_b_in = sc_s_in;
		sc_b_out = sc_s_out;
	} else
	{
		sc_b_out = tgetstr("me", &sp);
		if (hard || sc_b_out == NULL)
			sc_b_out = "";
	}

	sc_home = tgetstr("ho", &sp);
	if (hard || sc_home == NULL || *sc_home == '\0')
	{
		if (*sc_move == '\0')
		{
			/*
			 * This last resort for sc_home is supposed to
			 * be an up-arrow suggesting moving to the 
			 * top of the "virtual screen". (The one in
			 * your imagination as you try to use this on
			 * a hard copy terminal.)
			 */
			sc_home = "|\b^";
		} else
		{
			/* 
			 * No "home" string,
			 * but we can use "move(0,0)".
			 */
			(void)strcpy(sp, tgoto(sc_move, 0, 0));
			sc_home = sp;
			sp += strlen(sp) + 1;
		}
	}

	sc_lower_left = tgetstr("ll", &sp);
	if (hard || sc_lower_left == NULL || *sc_lower_left == '\0')
	{
		if (*sc_move == '\0')
		{
			sc_lower_left = "\r";
		} else
		{
			/*
			 * No "lower-left" string, 
			 * but we can use "move(0,last-line)".
			 */
			(void)strcpy(sp, tgoto(sc_move, 0, sc_height-1));
			sc_lower_left = sp;
			sp += strlen(sp) + 1;
		}
	}

	/*
	 * To add a line at top of screen and scroll the display down,
	 * we use "al" (add line) or "sr" (scroll reverse).
	 */
	if ((sc_addline = tgetstr("al", &sp)) == NULL || 
		 *sc_addline == '\0')
		sc_addline = tgetstr("sr", &sp);

	if (hard || sc_addline == NULL || *sc_addline == '\0')
	{
		sc_addline = "";
		/* Force repaint on any backward movement */
		back_scroll = 0;
	}
	if (tgetflag("bs"))
		sc_backspace = "\b";
	else
	{
		sc_backspace = tgetstr("bc", &sp);
		if (sc_backspace == NULL || *sc_backspace == '\0')
			sc_backspace = "\b";
	}
	create_esc_table();
}

create_esc_table()
{
	add_esc_table(bold_on);
	add_esc_table(under_on);
	add_esc_table(highl_on);
	add_esc_table(mode_off);
	add_esc_table(mode_off_1);
	add_esc_table(sc_b_in);
	add_esc_table(sc_b_out);
	add_esc_table(sc_s_in);
	add_esc_table(sc_s_out);
	add_esc_table(sc_u_in);
	add_esc_table(sc_u_out);
}
add_esc_table(aline)
char *aline;
{
	strcpy(e_table[e_table_cnt].line,aline);
	e_table[e_table_cnt++].num = strlen(aline);
}
check_esc(cline)
char *cline;
{
	int cnt;
	int n;
	int len=0;

	for(cnt=0;e_table[cnt].num !=0;cnt++){
		if((n=wstrstr(cline,e_table[cnt].line,e_table[cnt].num))){
			len += n * e_table[cnt].num;
		}
	}
	return(len);
}
wstrstr(cline,cstr,num)
wchar_t *cline;
char *cstr;
int num;
{
	int k,n,m;

	for(n=0;cline[n] != '\0';n++){
		if(cline[n] == cstr[0]){
			for(m=1;m < num;m++){
				if(cline[n + m] != cstr[m]) goto cont;
			}
			k++;
		}
cont:		continue;
	}
	return(k);
}

/*
 * Below are the functions which perform all the 
 * terminal-specific screen manipulation.
 */

/*
 * Initialize terminal
 */
void
init(void)
{
	if (use_tite)
		tputs(sc_init, sc_height, putchr);
}

/*
 * Deinitialize terminal
 */
void
deinit(void)
{
	if (use_tite)
		tputs(sc_deinit, sc_height, putchr);
}

/*
 * Home cursor (move to upper left corner of screen).
 */
void
home(void)
{
	tputs(sc_home, 1, putchr);
}

/*
 * Add a blank line (called with cursor at home).
 * Should scroll the display down.
 */
void
add_line(void)
{
	tputs(sc_addline, sc_height, putchr);
}

int short_file;				/* if file less than a screen */
void
lower_left(void)
{
	if (short_file) {
		putchr('\r');
		flush();
	}
	else
		tputs(sc_lower_left, 1, putchr);
}

/*
 * Ring the terminal bell.
 */
void
bell(void)
{
	putchr('\7');
}

/*
 * Clear the screen.
 */
void
clear(void)
{
	tputs(sc_clear, sc_height, putchr);
}

/*
 * Clear from the cursor to the end of the cursor's line.
 * {{ This must not move the cursor. }}
 */
void
clear_eol(void)
{
	tputs(sc_eol_clear, 1, putchr);
}

/*
 * Move to the beginning of the line.
 */
int
move_bol(void)
{
   /* First try deleting this line and inserting a new empty one. */
   if (*sc_parm_delete_line && *sc_parm_insert_line) {
      tputs(tparm(sc_parm_delete_line, 1), 1, putchr);
      tputs(tparm(sc_parm_insert_line, 1), 1, putchr);
      return(1);
   }
   if ((*sc_last_line) && (*sc_cur_down)) {
      /* Try just dropping back to column one of last line and going */
      /* straight down one row. */
      tputs(sc_last_line, 1, putchr);
      tputs(sc_cur_down, 1, putchr);
      return(1);
   }
   if (*sc_col_adr) {
      /* If that doesn't work, try direct column addressing. */
      tputs(tparm(sc_col_adr, 0), 1, putchr);
      return(1);
   }
   if (*sc_carriage_ret) {
      tputs(sc_carriage_ret, 1, putchr);
      return(1);
   }
   return(0);
}

/*
 * Begin "standout" (bold, underline, or whatever).
 */
void
so_enter(void)
{
	tputs(sc_s_in, 1, putchr);
}

/*
 * End "standout".
 */
void
so_exit(void)
{
	tputs(sc_s_out, 1, putchr);
}

/*
 * Begin "underline" (hopefully real underlining, 
 * otherwise whatever the terminal provides).
 */
void
ul_enter(void)
{
	tputs(sc_u_in, 1, putchr);
}

/*
 * End "underline".
 */
void
ul_exit(void)
{
	tputs(sc_u_out, 1, putchr);
}

/*
 * Begin "bold"
 */
void
bo_enter(void)
{
	tputs(sc_b_in, 1, putchr);
}

/*
 * End "bold".
 */
void
bo_exit(void)
{
	tputs(sc_b_out, 1, putchr);
}

/*
 * Erase the character to the left of the cursor 
 * and move the cursor left.
 */
void
backspace(void)
{
	/* 
	 * Try to erase the previous character by overstriking with a space.
	 */
	tputs(sc_backspace, 1, putchr);
	putchr(' ');
	tputs(sc_backspace, 1, putchr);
}

/*
 * Output a plain backspace, without erasing the previous char.
 */
void
putbs(void)
{

	tputs(sc_backspace, 1, putchr);
}
