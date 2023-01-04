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
static char rcsid[] = "@(#)$RCSfile$ $Revision$ (DEC) $Date$";
#endif
/*
 * HISTORY
 */
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


# include       "cursesext.h"
# include	<signal.h>
# include	<errno.h>
# include	<stdlib.h>

# include	<sys/time.h>
# include	<sys/types.h>

/* Include file for wchar_t */
#include	<locale.h>
#include	<wchar.h>

static int	_fpk(FILE *);
static void	_catch_alarm(int);

static int sig_caught;

/*
 *	This routine reads in a wchar_t character from the window.
 *
 *	wgetwch MUST return an int, not a char, because it can return
 *	things like ERR, meta characters, and function keys > 256.
 */

extern	int	wgetwch( register WINDOW *win )
{

	register int inp;
	register int i, j;
	char c;
	int arg;
	bool	weset = FALSE;
	FILE *inf;
	int		mb_len = 0;
	wchar_t		wc;
	unsigned char	mbuf[10];
	int 	dont_echo = 0;

	if (SP->fl_echoit && !win->_scroll && (win->_flags&_FULLWIN)
	    && win->_curx == win->_maxx && win->_cury == win->_maxy)
		return ERR;
# ifdef DEBUG
	if(outf) fprintf(outf,
			"WGEWTCH: SP->fl_echoit = %c, SP->fl_rawmode = %c\n",
			SP->fl_echoit ? 'T' : 'F', SP->fl_rawmode ? 'T' : 'F');
	if (outf) fprintf(outf,
			"_use_keypad %d, kp_state %d\n",
			win->_use_keypad, SP->kp_state);
# endif
	if (SP->fl_echoit && !SP->fl_rawmode) {
		cbreak();
		weset++;
	}

#ifdef KEYPAD
	/* Make sure keypad on is in proper state */
	if (win->_use_keypad != SP->kp_state) {
		_kpmode(win->_use_keypad);
		fflush(stdout);
	}
#endif

	/* Make sure we are in proper nodelay state */
        /* removed '!= SP->fl_nodelay' from the test, as it should never */
	/* be set except here. */
        /* paw: QAR 5935 */

	if (win->_nodelay)
		_fixdelay(SP->fl_nodelay, win->_nodelay);

	/* Check for pushed typeahead.  We make the assumption that
	 * if a function key is being typed, there is no pushed
	 * typeahead from the previous key waiting.
	 */
retry:
	if (SP->input_queue[0] >= 0) {
		inp = SP->input_queue[0];
		for (i=0; i<16; i++) {
			SP->input_queue[i] = SP->input_queue[i+1];
			if (SP->input_queue[i] < 0)
				break;
		}
		dont_echo = 1;
		goto gotit;
	}

	inf = SP->input_file;
	if (inf == stdout)	/* so output can be teed somewhere */
		inf = stdin;
#ifdef FIONREAD
	if (win->_nodelay) {
		ioctl(fileno(inf), FIONREAD, &arg);
#ifdef DEBUG
		if (outf) fprintf(outf, "FIONREAD returns %d\n", arg);
#endif
                if (arg < 1) {
                	/* turn off non-blocking i/o as is causes write */
			/* problems. paw: QAR 5935 */
                        if (win->_nodelay)
                                _fixdelay(SP->fl_nodelay, FALSE);
                        return -1;
                }
	}
#endif
	for (i = -1; i<0; ) {
		extern int errno;
		sig_caught = 0;
		i = read(fileno(inf), &c, 1);
                /* paw: QAR 5544, removed the test case 'errno != EINTR' */
		/* from followin if statement */

		/*
		 * I hope the system won't retern infinite EINTRS - maybe
		 * there should be a hop count here.
		 */
		if (i < 0 && !sig_caught) {
			inp = ERR;
			goto done;
		}
	}
	if (i > 0) {
		inp = c;
#ifdef ULTRIX
		if (!win->_use_meta)
			inp &= A_CHARTEXT;
		else
			inp &= 0377;
#else
		inp &= 0377;
#endif
	} else {
		inp = ERR;
		goto done;
	}
# ifdef  DEBUG
/*	if(outf) fprintf(outf,"WGETCH got '%s'\n",unctrl(inp)); */
	if(outf) fprintf(outf,"WGETCH got '%04x'\n",inp );

# endif

#ifdef KEYPAD
	/* Check for arrow and function keys */
	if (win->_use_keypad) {
		SP->input_queue[0] = inp;
		SP->input_queue[1] = -1;
		for (i=0; SP->kp[i].keynum > 0; i++) {
			if (SP->kp[i].sends[0] == inp) {
				for (j=0; ; j++) {
					if (SP->kp[i].sends[j] < 0)
						break;	/* found */
					if (SP->input_queue[j] == -1) {
						SP->input_queue[j] = _fpk(inf);
						SP->input_queue[j+1] = -1;
					}
					if (SP->kp[i].sends[j] != SP->input_queue[j])
						goto contouter; /* not this one */
				}
				/* It matched the function key. */
				inp = SP->kp[i].keynum;
				SP->input_queue[0] = -1;
				goto done;
			}
		contouter:;
		}
		/* Didn't match any function keys. */
		inp = SP->input_queue[0];
		for (i=0; i<16; i++) {
			SP->input_queue[i] = SP->input_queue[i+1];
			if (SP->input_queue[i] < 0)
				break;
		}
		goto gotit;
	}
#endif

gotit:
	mbuf[mb_len++] = (unsigned char)inp;
	mbuf[mb_len] = '\0';
	if( mbtowc( &wc, (char *)mbuf, MB_CUR_MAX ) < 0 )
	{
		if( mb_len >= MB_CUR_MAX )
			inp = ERR;
		else
			goto retry;
	}
	else
	{
		inp = wc;
		if (SP->fl_echoit && !win->_use_keypad) {
			if (dont_echo)
				dont_echo = 0;
			else {
				waddwch(win, (chtype) wc);
				wrefresh(win);
			}
		}
	}
done:
	if (weset)
		nocbreak();
#ifdef DEBUG
	if(outf) fprintf(outf, "getch returns %o, pushed %o %o %o\n",
		inp, SP->input_queue[0], SP->input_queue[1], SP->input_queue[2]);
#endif
        /* turn off non-blocking i/o as it causes write problems. paw: QAR 5935 */
        if (win->_nodelay)
                _fixdelay(SP->fl_nodelay, FALSE);
	return inp;
}

/*
 * NAME:        _catch_alarm
 */

static void
_catch_alarm(int c)
{
	sig_caught = 1;
}


/*
 * NAME:        _fpk
 *
 * FUNCTION:
 *
 *      Fast peek key.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Like getchar but if the right flags are set, times out
 *      quickly if there is nothing waiting, returning -1.
 *      f is an output stdio descriptor, we read from the fileno.  win is
 *      the window this is supposed to have something to do with.
 *
 *      Traditional implementation.  The best resolution we have is 1
 *      second, so we set a 1 second alarm and try to read.  If we fail for
 *      1 second, we assume there is no key waiting.  Problem here is that 1
 *      second is too long, people can type faster than this.
 *
 *      If we have the select system call (FIONREAD), we can do much better.
 *      We wait for long enough for a terminal to send another character
 *      (At 15cps repeat rate, this is 67 ms, I'm using 100ms to allow
 *      a bit of a fudge factor) and time out more quickly.  Even if we
 *      don't have the real 4.2BSD select, we can emulate it with napms
 *      and FIONREAD.  napms might be done with only 1 second resolution,
 *      but this is no worse than what we have above.
 *
 **003* If we have the select system call (FIONREAD), the select() call
 **003* calls the function _curses_dtablesize() to get the number of
 **003* file descriptors currently active, instead of the traditional limit of
 **003* "20".
 *
 **001*   * The R1.0.1 code waited indefinitely for another character if a
 **001*     single ESC (keycode sequence prefix character) was entered.
 **001*     select() would timeout, and the timeout code was ignored.
 **001*     _fpk() now returns -2 if select() returns 0 (timeout).
 **002*
 **002*     The 100ms timeout of traditional System V implementations,
 **002*     however, is too short for remotely logged (rlogin) terminals, or
 **002*     if the system is overloaded, or if the application is generating
 **002*     lots of output per input character.  The timeout period is
 **002*     increased to 500ms.
 */

#ifndef FIONREAD
static
_fpk(f)
FILE *f;
{
	char c;
	int rc;
	void  (*oldsig)();
	int oldalarm;

	oldsig = signal(SIGALRM, (void (*)(int))_catch_alarm);
	oldalarm = alarm(1);
	sig_caught = 0;
	rc = read(fileno(f), &c, 1);
	if (sig_caught) {
		sig_caught = 0;
		alarm(oldalarm);
		return -2;
	}
	alarm(oldalarm);
	signal(SIGALRM, (void (*)(int))oldsig);
	return rc == 1 ? c : -1;
}
#else FIONREAD

#include	<sys/time.h>

static
_fpk(f)
FILE *f;
{
	int rc;
	fd_set infd;
	struct timeval timeout;
	char c;

	FD_ZERO(&infd);
	FD_SET(fileno(f), &infd);
/*002*/ timeout.tv_sec = 0L;
/*002*/ timeout.tv_usec = 500*1000L;
/*003*/ rc = select(_curses_dtablesize(), &infd, NULL, NULL, &timeout);

/*001*/ /* Return -2 if select() failed (<0) or if select() timed out (rc = 0).
         * No attempt is made to determine a specific cause for the failure.
         */

/*001*/ if (rc <= 0)
		return -2;
	rc = read(fileno(f), &c, 1);
	return rc == 1 ? c : -1;
}

static                                                  /* 004 */
_curses_dtablesize()
{
        static int dt_size;

        if (dt_size == 0) {
                dt_size = getdtablesize();
        }
        return (dt_size);
}

#endif /* FIONREAD */
