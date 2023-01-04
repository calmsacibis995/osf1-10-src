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
/* trmsbr.c - minor termcap support (load with -ltermlib) */
#ifndef	lint
static char ident[] = "@(#)$RCSfile: trmsbr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:39:23 $ devrcs Exp Locker: devbld $";
#endif	lint

#include "../h/mh.h"
#include <stdio.h>
#ifndef	SYS5
#include <sgtty.h>
#if	defined(ULTRIX) && !defined(BSD43)
#undef	TIOCGWINSZ
#endif
#else	SYS5
#include <sys/types.h>
#include <termio.h>
#ifndef	NOIOCTLH
#include <sys/ioctl.h>
#endif	NOIOCTLH
#undef	TIOCGWINSZ
#endif	SYS5


#if	BUFSIZ<2048
#define	TXTSIZ	2048
#else
#define	TXTSIZ	BUFSIZ
#endif

#ifndef	SYS5
extern char PC;
extern short    ospeed;
#else   SYS5
char	PC;
short	ospeed;
#endif	SYS5

int     tgetent (), tgetnum ();
char   *tgetstr ();

/*  */

static int  HC = 0;

static int  initLI = 0;
static int  LI = 40;
static int  initCO = 0;
static int  CO = 80;
static char *CL = NULL;
static char *SE = NULL;
static char *SO = NULL;

static char termcap[TXTSIZ];

/*  */

static  read_termcap () {
    register char  *bp,
                   *term;
    char   *cp,
	    myterm[TXTSIZ];
#ifndef	SYS5
    struct sgttyb   sg;
#else	SYS5
    struct termio   sg;
#endif	SYS5
    static int  inited = 0;

    if (inited++)
	return;

    if ((term = getenv ("TERM")) == NULL || tgetent (myterm, term) <= OK)
	return;

#ifndef	SYS5
    ospeed = ioctl (fileno (stdout), TIOCGETP, (char *) &sg) != NOTOK
		? sg.sg_ospeed : 0;
#else	SYS5
    ospeed = ioctl (fileno (stdout), TCGETA, &sg) != NOTOK
		? sg.c_cflag & CBAUD : 0;
#endif	SYS5

    HC = tgetflag ("hc");

    if (!initCO && (CO = tgetnum ("co")) <= 0)
	CO = 80;
    if (!initLI && (LI = tgetnum ("li")) <= 0)
	LI = 24;

    cp = termcap;
    CL = tgetstr ("cl", &cp);
    if (bp = tgetstr ("pc", &cp))
	PC = *bp;
    if (tgetnum ("sg") <= 0) {
	SE = tgetstr ("se", &cp);
	SO = tgetstr ("so", &cp);
    }
}

/*  */

int     sc_width () {
#ifdef	TIOCGWINSZ
    struct winsize win;
    int width;

    if (ioctl (fileno (stderr), TIOCGWINSZ, &win) != NOTOK
	    && (width = win.ws_col) > 0) {
	CO = width;
	initCO++;
    } else
#endif	TIOCGWINSZ
	read_termcap ();

    return CO;
}


int     sc_length () {
#ifdef	TIOCGWINSZ
    struct winsize win;

    if (ioctl (fileno (stderr), TIOCGWINSZ, &win) != NOTOK
	    && (LI = win.ws_row) > 0)
	initLI++;
    else
#endif	TIOCGWINSZ
	read_termcap ();

    return LI;
}

/*  */

static int  outc (c)
register char    c;
{
    (void) putchar (c);
}


void clear_screen () {
    read_termcap ();

    if (CL && ospeed)
	tputs (CL, LI, outc);
    else {
	printf ("\f");
	if (ospeed)
	    printf ("\200");
    }

    (void) fflush (stdout);
}

/*  */

/* VARARGS1 */

int     SOprintf (fmt, a, b, c, d, e, f)
char   *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    read_termcap ();
    if (SO == NULL || SE == NULL)
	return NOTOK;

    tputs (SO, 1, outc);
    printf (fmt, a, b, c, d, e, f);
    tputs (SE, 1, outc);

    return OK;
}

/*  */

int	sc_hardcopy () {
    read_termcap ();

    return HC;
}
