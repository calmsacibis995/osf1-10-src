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
static char rcsid[] = "@(#)$RCSfile: ex_tty.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1994/01/21 22:52:47 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_tty.c
 *
 * FUNCTION: WCkpadd, WCkpboth, cost, countnum, fkey, gettmode, kpadd, kpboth,
 * setterm
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.22  com/cmd/edit/vi/ex_tty.c, , bos320, 9134320 8/11/91 12:30:24
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */

#include "ex.h"
#include "ex_tty.h"
#include <values.h>

void    addmac(register wchar_t *, register wchar_t *, register wchar_t *, register struct maps *);
int setupterm(char *, int, int *);
int tgetnum(char *);

#define CNOSTR ((char *) 0)

static void kpboth(struct maps *, struct maps *, char *, char *, char *);
static void WCkpboth(struct maps *, struct maps *, wchar_t *, wchar_t *, wchar_t *);
static void WCkpadd(struct maps *, wchar_t *, wchar_t *, wchar_t *);
static void kpadd(struct maps *, char *, char *, char *);
static wchar_t allocspace[1024];
static wchar_t *freespace;

/*
 * Terminal type initialization routines,
 * and calculation of flags at entry or after
 * a shell escape which may change them.
 */
static short GT;

void gettmode(void)
{
#ifdef _POSIX_SOURCE
	speed_t  output_speed;
#endif

	GT = 1;
#ifdef _POSIX_SOURCE
        if (tcgetattr(2, &tty) < 0)
                return;
	output_speed = cfgetospeed(&tty);
	if (ospeed != output_speed)	/* mjm */
		value(SLOWOPEN) = output_speed < B1200;
	ospeed = output_speed;
#else
        if (ioctl(2, TCGETA, &tty) < 0)
                return;
	if (ospeed != (tty.c_cflag & CBAUD))	/* mjm */
		value(SLOWOPEN) = (tty.c_cflag & CBAUD) < B1200;
	ospeed = tty.c_cflag & CBAUD;
#endif
	normf = tty;
	UPPERCASE = (tty.c_iflag & IUCLC) != 0;
	if ((tty.c_oflag & TABDLY) == TAB3 || teleray_glitch)
		GT = 0;
	NONL = (tty.c_oflag & ONLCR) == 0;
}

void setterm(char *type)
{
	register int unknown, i;
	int errret;
	extern char termtype[];

	unknown = 0;
	if (cur_term && exit_ca_mode)
		putpad(exit_ca_mode);
	cur_term = NULL;
	strcpy(termtype, type);
#ifdef TRACE
	if (trace) fprintf(trace, "before setupterm & ioctl, termtype %s ,lines %d, columns %d, clear_screen '%s', cursor_address '%s'\n", termtype, lines, columns, clear_screen, cursor_address);
#endif
	setupterm(type, 2, &errret);
	switch(errret) {
	    case -1:			/* Terminfo database not found */
		merror(MSGSTR(M_513, "Terminfo database missing, unable to continue.\n"), DUMMY_INT);
		unknown++;
		/* FALLTHROUGH */
	    case 0:			/* Terminal description not found */
		if (strcmp(type, "dumb") != 0) unknown++;
		cur_term = NULL;
		setupterm("unknown", 1, &errret);
		break;
	    case 1:			/* terminal description found */
		break;
	}
	resetterm();
#ifdef TRACE
	if (trace) fprintf(trace, "after setupterm, termtype %s, lines %d, columns %d, clear_screen '%s', cursor_address '%s'\n", termtype, lines, columns, clear_screen, cursor_address);
#endif
	setsize();
	/*
	 * Initialize keypad arrow keys.
	 */
	freespace = allocspace;

	kpadd(arrows, key_ic, "i", "inschar");
	kpadd(immacs, key_ic, "\033", "inschar");
	kpadd(arrows, key_eic, "i", "inschar");
	kpadd(immacs, key_eic, "\033", "inschar");

	kpboth(arrows, immacs, key_up, "k", "up  ");
	kpboth(arrows, immacs, key_down, "j", "down");
	kpboth(arrows, immacs, key_left, "h", "left");
	kpboth(arrows, immacs, key_right, "l", "right");
	kpboth(arrows, immacs, key_home, "H", "home");
	kpboth(arrows, immacs, key_il, "o\033", "insline");
	kpboth(arrows, immacs, key_dl, "dd", "delline");
	kpboth(arrows, immacs, key_clear, "\014", "clear");
	kpboth(arrows, immacs, key_eol, "d$", "clreol");
	kpboth(arrows, immacs, key_sf, "\005", "scrollf");
        kpadd(arrows, key_dc, "x", "delchar");
        kpadd(immacs, key_dc, "\010", "delchar");
	kpboth(arrows, immacs, key_npage, "\006", "npage");
	kpboth(arrows, immacs, key_ppage, "\002", "ppage");
        kpboth(arrows, immacs, key_sr, "\031", "sr  ");
        /*
         * key_eos is mapped to the clear to end of file command "dG".
         * key_eos for the IBM 3100 series of terminals is identified
         * by the key sequence ESC J. Because this sequence may be
         * entered for returning to command mode and the joining of lines,
         * problems may occur. i.e. If the sequence is entered fast by the
         * user, it is interpreted as the key_eos sequence.
         *
         * Therefore, for the IBM 3100 series key_eos is not mapped. If
         * the escape sequence is received for this series of terminals
         * it is treated as two separate key strokes.
         */
        if (strncmp(termtype, "ibm31", 5) != 0)
                kpboth(arrows, immacs, key_eos, "dG", "clreos");

	/*
	 * Handle funny termcap capabilities
	 */
	if (change_scroll_region && save_cursor && restore_cursor) insert_line=delete_line="";
	if (parm_insert_line && insert_line==NULL) insert_line="";
	if (parm_delete_line && delete_line==NULL) delete_line="";
	if (insert_character && enter_insert_mode==NULL) enter_insert_mode="";
	if (insert_character && exit_insert_mode==NULL) exit_insert_mode="";
	if (GT == 0)
		tab = back_tab = CNOSTR;

#ifdef SIGTSTP
	/*
	 * Now map users susp char to ^Z, being careful that the susp
	 * overrides any arrow key, but only for new tty driver.
	 */
	{
		static wchar_t sc[2];
		int ii;

#ifdef NTTYDISC
		ioctl(2, TIOCGETD, (char *)&ldisc);
#endif
		if (!value(NOVICE)) {
#if    defined(_POSIX_JOB_CONTROL)
                        if(mbtowc(sc, (char*)&tty.c_cc[VSUSP], 1) <= 0){
#else  /*TIOCLGET Berkeley 4BSD */
                        if(mbtowc(sc, &olttyc.t_suspc, 1) <= 0){
#endif
                                sc[0] = 0;   /* the conversion has failed so set it to null */
                        }
                        sc[1] = 0;
#if    defined(_POSIX_JOB_CONTROL)
                        if (tty.c_cc[VSUSP] == Ctrl('Z')) {
#else  /*TIOCLGET Berkeley 4BSD */
			if (olttyc.t_suspc == Ctrl('Z')) {
#endif
				for (ii=0; ii<=4; ii++)
					if (arrows[ii].cap != NULL &&
					    arrows[ii].cap[0] == Ctrl('Z'))
						addmac(sc, (wchar_t *)NULL, (wchar_t *)NULL, arrows);
			}
                        else if (sc[0] != 0) {
                                wchar_t addm_tmp[8], addm_tmp2[8];
                                char *addm_c;
                                addm_c = "\32";
                                (void) mbstowcs(addm_tmp, addm_c, strlen(addm_c) + 1);
                                addm_c = "susp";
                                (void) mbstowcs(addm_tmp2, addm_c, strlen(addm_c) + 1);
                                addmac(sc, addm_tmp, addm_tmp2, arrows);
                        }
		}
	}
#endif

#ifdef TRACE
		if (trace) fprintf(trace, "before OOPS , termtype %s ,lines %d, columns %d, clear_screen '%s', cursor_address '%s'\n", termtype, lines, columns, clear_screen, cursor_address);
#endif
	costCM = cost(tparm(cursor_address, 10, 8));
	if (costCM >= 10000)
		cursor_address = NULL;
	costSR = cost(scroll_reverse);
	costAL = cost(insert_line);
	costDP = cost(tparm(parm_down_cursor, 10));
	costLP = cost(tparm(parm_left_cursor, 10));
	costRP = cost(tparm(parm_right_cursor, 10));
	costCE = cost(clr_eol);
	costCD = cost(clr_eos);
	/* proper strings to change tty type */
	termreset();
	gettmode();
	value(REDRAW) = insert_line && delete_line;
	value(OPTIMIZE) = !cursor_address && !tab;
	if (ospeed == B1200 && !value(REDRAW))
		value(SLOWOPEN) = 1;	/* see also gettmode above */
	if (unknown)
		serror(MSGSTR(M_202, "%s: Unknown terminal type"), type);
#ifdef TRACE
		if (trace) fprintf(trace, "exit setterm , termtype %s ,lines %d, columns %d, clear_screen '%s', cursor_address '%s'\n", termtype, lines, columns, clear_screen, cursor_address);
#endif
}

void setsize(void)
{
	register int l, i;
	int olines = lines, ocols = columns; /* NO clobbering by bad ioctl */
	char *cp;
	long  temp;

#ifdef TIOCGWINSZ
	struct winsize win;

	if (ioctl(0, TIOCGWINSZ, (char *)&win) < 0) {
#endif
		i = lines = tgetnum("li");
		columns = tgetnum("co");
#ifdef TIOCGWINSZ
	} else {
		if ((lines = winsz.ws_row = win.ws_row) == 0)
			lines = tgetnum("li");
		i = lines;
		if ((columns = winsz.ws_col = win.ws_col) == 0)
			columns = tgetnum("co");
	}
#endif
        TUBELINES=DEF_TUBELINES;        /* WW-01 */
        TUBECOLS=DEF_TUBECOLS;          /* WW-01 */
        TUBESIZE=DEF_TUBESIZE;          /* WW-01 */

	if ((cp = getenv("COLUMNS")) != NULL) {
	  if ((temp = strtol(cp, NULL, 0)) > 0) {
	    columns = temp;
	  }
	}

	if ((cp = getenv("LINES")) != NULL) {
	  if ((temp = strtol(cp, NULL, 0)) > 0) {
	    lines = temp;
	  }
	}

	if (!lines && olines)	/* Prevent clobbering by bad ioctl */
		i = lines = olines;
	if (!columns && ocols)
		columns = ocols;
	if (lines <= 5)
		lines = 24;
	l = lines;
	if (ospeed < B1200)
		l = 9;	/* including the message line at the bottom */
	else if (ospeed < B2400)
		l = 17;
	if (l > lines)
		l = lines;
	if (columns <= 4)
		columns = 1000;
	value(WINDOW) = options[WINDOW].odefault = l - 1;
	if (defwind) value(WINDOW) = defwind;
	value(SCROLL) = (options[SCROLL].odefault =
		hard_copy ? 11 : (value(WINDOW) / 2));
	if (i <= 0)
		lines = 2;
        if (lines > DEF_TUBELINES)              /* WW-01 */
                TUBELINES=lines;                /* WW-01 */
        if (columns > DEF_TUBECOLS)             /* WW-01 */
                TUBECOLS=columns;               /* WW-01 */
        TUBESIZE=TUBELINES * TUBECOLS;          /* WW-01 */
}

static void kpboth(struct maps *m1, struct maps *m2, char *k,char *m,char *d)
{
	if (k) {
		wchar_t *key, *mapto, *descr;
		int str_len;
                if ((str_len = mbstowcs(freespace, k, strlen(k)+1)) > 0){
                        key = freespace;
                        freespace += str_len + 1;
                }
                else
                        error(MSGSTR(M_650, "Invalid multibyte character string, conversion failed."), DUMMY_INT);
                if ((str_len = mbstowcs(freespace, m, strlen(m)+1)) > 0){
                        mapto = freespace;
                        freespace += str_len + 1;
                }
                else
                        error(MSGSTR(M_650, "ex: Invalid multibyte character string, conversion failed."), DUMMY_INT);

                if ((str_len = mbstowcs(freespace, d, strlen(d)+1)) > 0){
                        descr = freespace;
                        freespace += str_len + 1;
                }
                else
                        error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);

                WCkpboth(m1, m2, key, mapto, descr);
	}
}

static void kpadd(struct maps *map, char *k, char *m, char *d)
{
	if (k) {
                wchar_t *key, *mapto, *descr;
                int str_len;

                if ((str_len = mbstowcs(freespace, k, strlen(k)+1)) > 0){
                        key = freespace;
                        freespace += str_len + 1;
                }
                else
                        error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);

                if ((str_len = mbstowcs(freespace, m, strlen(m)+1)) > 0){
                        mapto = freespace;
                        freespace += str_len + 1;
                }
                else
                        error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);

                if ((str_len = mbstowcs(freespace, d, strlen(d)+1)) > 0){
                        descr = freespace;
                        freespace += str_len + 1;
                }
                else
                        error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);

                WCkpadd(map, key, mapto, descr);

	}
}

/*
 * Map both map1 and map2 as below.  map2 surrounded by esc and a.
 */
static void WCkpboth(struct maps *map1, struct maps *map2, wchar_t *key, wchar_t *mapto, wchar_t *desc)
{
	wchar_t *p;

	WCkpadd(map1, key, mapto, desc);
	if (any(*key, "\b\n "))
		return;
	p = freespace;
	*freespace++ = '\033';
	wcscpy(freespace, mapto);
	freespace = WCstrend(freespace);
	*freespace++ = 'a';
	*freespace++ = '\0';
	WCkpadd(map2, key, p, desc);
}

/*
 * Define a macro.  mapstr is the structure (mode) in which it applies.
 * key is the input sequence, mapto what it turns into, and desc is a
 * human-readable description of what's going on.
 */
static void WCkpadd(struct maps *mapstr, wchar_t *key, wchar_t *mapto, wchar_t *desc)
{
	int i;

	for (i=0; i<MAXNOMACS; i++)
		if (mapstr[i].cap == 0)
			break;
	if (i >= MAXNOMACS)
		return;
	mapstr[i].cap = key;
	mapstr[i].mapto = mapto;
	mapstr[i].descr = desc;
}

char *
fkey(int i)
{
	switch (i) {
	case 0: return key_f0;
	case 1: return key_f1;
	case 2: return key_f2;
	case 3: return key_f3;
	case 4: return key_f4;
	case 5: return key_f5;
	case 6: return key_f6;
	case 7: return key_f7;
	case 8: return key_f8;
	case 9: return key_f9;
	default: return CNOSTR;
	}
}

/*
 * cost figures out how much (in characters) it costs to send the string
 * str to the terminal.  It takes into account padding information, as
 * much as it can, for a typical case.	(Right now the typical case assumes
 * the number of lines affected is the size of the screen, since this is
 * mainly used to decide if insert_line or scroll_reverse is better, and
 * this always happens at the top of the screen.  We assume cursor motion
 * (cursor_address) has little * padding, if any, required, so that case,
 * which is really more important than insert_line vs scroll_reverse,
 * won't be really affected.)
 */
static int costnum;
int cost(char *str)
{
	/* int countnum(void); */

	if (str == NULL || *str=='O')	/* OOPS */
		return 10000;	/* infinity */
	costnum = 0;
	tputs(str, lines, (int(*)(int))countnum);
	return costnum;
}

/* ARGSUSED */
void countnum(char ch)
{
	costnum++;
}
