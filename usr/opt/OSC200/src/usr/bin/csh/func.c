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
static char rcsid[] = "@(#)$RCSfile: func.c,v $ $Revision: 4.2.10.6 $ (DEC) $Date: 1993/11/02 16:56:17 $";
#endif
/*
 * OSF/1 1.1.1
 */
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: isbfunc func dolabel doonintr donohup dozip prvars doalias 
 *            unalias dologout dologin donewgrp islogin doif reexecute doelse
 *            dogoto doswitch dobreak doexit doforeach dowhile preread 
 *            doend docontin doagain dorepeat doswbrk srchx search getword 
 *            toend wfree doecho doglob echo dosetenv dounsetenv setcenv 
 *            unsetcenv doumask findlim dolimit getval limtail plim dounlimit
 *            setlim dosuspend doeval
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.25  com/cmd/csh/func.c, cmdcsh, bos320, 9145320h 11/7/91 08:59:57
 *
 */

/* 001 RNF  Integrate Silver bug fix for QARs 7010 and 7211 */
#include <sys/ioctl.h>
#include <locale.h>
#include <unistd.h>
#include <ctype.h>
#ifndef _SBCS
#include <wchar.h>
#endif
#include "sh.h"
#include "pathnames.h"

#ifndef _SBCS
/* Testing c for less than zero does not work when c's type is */
/* wchar_t and wchar_t is typedef'ed to an unsigned type. */
#define IsEOF(wc) (wc == (wchar_t)WEOF)
#else
#ifndef EOF
#define EOF (-1)
#endif
#define IsEOF(c) (c == EOF)
#endif

extern char	**environ;


struct biltins *
isbfunc(register struct command *t)
{
	register uchar_t *cp = t->t_dcom[0];
	register uchar_t *dp;
	register struct biltins *bp;
	void dolabel(), dofg1(), dobg1();
	static struct biltins label = { (uchar_t *)"", dolabel, 0, 0 };
	static struct biltins foregnd = { (uchar_t *)"%job", dofg1, 0, 0 };
	static struct biltins backgnd = { (uchar_t *)"%job &", dobg1, 0, 0 };

	if (lastchr(cp) == ':') {
		label.bname = cp;
		return (&label);
	}
	if (*cp == '%') {
		if (t->t_dflg & FAND) {
			t->t_dflg &= ~FAND;
			backgnd.bname = cp;
			return (&backgnd);
		} 
		foregnd.bname = cp;
		return (&foregnd);
	}
	for (bp = bfunc; dp = bp->bname; bp++) {
		if (dp[0] == cp[0] && EQ(dp, cp))
			return (bp);
		if (dp[0] > cp[0])
			break;
	}
	return (0);
}

void
func(register struct command *t, register struct biltins *bp)
{
	int i;

	xechoit(t->t_dcom);
	setname(bp->bname);
	i = blklen(t->t_dcom) - 1;
	if (i < bp->minargs)
		bferr(MSGSTR(M_TOOFEW, "Too few arguments"));
	if (i > bp->maxargs)
		bferr(MSGSTR(M_TOOMANY, "Too many arguments"));
	(*bp->bfunct)(t->t_dcom, t);
	return;
}

void
dolabel(void)
{
return;
}

void
doonintr( uchar_t **v)
{
	register uchar_t *cp;
	register uchar_t *vv = v[1];
	struct sigvec nsv;

	if (parintr == SIG_IGN)
		return;
	if (setintr && intty)
		bferr(MSGSTR(M_CANT, "Can't from terminal"));
	cp = gointr;
	gointr = 0;
	xfree(cp);
	if (vv == 0) {
		if (setintr) {
			(void)sigblock(sigmask(SIGINT));
		}
		else  {
			nsv.sv_handler = SIG_DFL;
			nsv.sv_mask = 0;
			nsv.sv_onstack = 0;
			(void)sigvec(SIGINT, &nsv, (struct sigvec *)NULL);
		}
		gointr = 0;
	} else if (EQ((vv = strip(vv)), "-")) {
		nsv.sv_handler = SIG_IGN;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGINT, &nsv, (struct sigvec *)NULL);
		gointr = (uchar_t *)"-";
	} else {
		gointr = savestr(vv);
		nsv.sv_handler = (void (*)(int))pintr;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGINT, &nsv, (struct sigvec *)NULL);
	}
}

void
donohup(void)
{
	struct sigvec nsv;	

	if (intty) {
		bferr(MSGSTR(M_CANT, "Can't from terminal"));
		return;
	}
	if (!loginsh && !intact) {
		nsv.sv_handler = SIG_IGN;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGHUP, &nsv, (struct sigvec *)NULL);
	}
}

void
dozip(void)
{
return;
}

void
prvars(void)
{
plist(&shvhed);
}

void
doalias(register uchar_t **v)
{
	register struct varent *vp;
	register uchar_t *p;

	v++;
	p = *v++;
	if (p == 0)
		plist(&aliases);
	else if (*v == 0) {
		vp = adrof1((uchar_t *)strip(p), &aliases);
		if (vp)
			blkpr(vp->vec), csh_printf("\n");
	} else {
		if (EQ(p, "alias") || EQ(p, "unalias")) {
			setname(p);
			bferr(MSGSTR(M_ALIAS, "Too dangerous to alias that"));
		}
		set1(strip(p), saveblk(v), &aliases);
	}
}

void
unalias(uchar_t **v)
{
unset1(v, &aliases);
}

void
dologout(void)
{

	islogin();
	goodbye();
}

void
dologin(uchar_t **v)
{
	struct sigvec nsv;	

	islogin();
	rechist();
	nsv.sv_handler = parterm;
	nsv.sv_mask = 0;
	nsv.sv_onstack = 0;
	(void)sigvec(SIGTERM, &nsv, (struct sigvec *)NULL);
	execv(_PATH_LOGIN, (char**)v);
	untty();
	exitcsh(1);
}

void
donewgrp(uchar_t **v)
{
	struct sigvec nsv;	

	if (chkstop == 0 && setintr)
		panystop(0);
	nsv.sv_handler = parterm;
	nsv.sv_mask = 0;
	nsv.sv_onstack = 0;
	(void)sigvec(SIGTERM, &nsv, (struct sigvec *)NULL);
	execl(_PATH_NEWGRP, "newgrp", v[1], 0);
	untty();
	exitcsh(1);
}
/* 001 RNF  Merge in DEC OSF/1 Bug fix by Dave Gray.
* void
* doinlib(uchar_t **v)
* {
*	if((v = glob(v)) == 0 || v[1] == NULL)
*		bferr(MSGSTR(M_NOMATCH, "No match"));
*	else if (v[2] != NULL)
*		bferr(MSGSTR(M_TOOMANY, "Too many arguments"));
*	else {
*		if(!ldr_install(v[1]))
*			return;
*		error(MSGSTR(M_BADINLIB, "Inlib install failed"));
*	}
*	return;
* }
*
* void
* dormlib(uchar_t **v)
* {
*	if((v = glob(v)) == 0 || v[1] == NULL)
*		bferr(MSGSTR(M_NOMATCH, "No match"));
*	else if (v[2] != NULL)
*		bferr(MSGSTR(M_TOOMANY, "Too many arguments"));
*	else {
*		if(!ldr_remove(v[1]))
*			return;
*		error(MSGSTR(M_BADRMLIB, "rmlib removal failed"));
*	}
*	return;
* }
*********************************************************************/
void
islogin(void)
{

	if (chkstop == 0 && setintr)
		panystop(0);
	if (loginsh)
		return;
	error(MSGSTR(M_NOTLOGIN, "Not login shell"));
}

void
doif(uchar_t **v, struct command *kp)
{
	register long i;                                  /* 002 */
	register uchar_t **vv;

	v++;
	i = exp(&v);
	vv = v;
	if (*vv == NOSTR)
		bferr(MSGSTR(M_EMPTYIF, "Empty if"));
	if (EQ(*vv, "then")) {
		if (*++vv)
			bferr(MSGSTR(M_THEN, "Improper then"));
		setname((uchar_t *)"then");
		/*
		 * If expression was zero, then scan to else,
		 * otherwise just fall into following code.
		 */
		if (!i)
			search(ZIF, 0, (uchar_t)NULL);
		return;
	}
	/*
	 * Simple command attached to this if.
	 * Left shift the node in this tree, munging it
	 * so we can reexecute it.
	 */
	if (i) {
		lshift(kp->t_dcom, vv - kp->t_dcom);
		reexecute(kp);
		if (didfds)
			donefds();
	}
}

/*
 * Reexecute a command, being careful not
 * to redo i/o redirection, which is already set up.
 */
void
reexecute(register struct command *kp)
{

	kp->t_dflg &= FSAVE;
	kp->t_dflg |= FREDO;
	/*
	 * If tty is still ours to arbitrate, arbitrate it;
	 * otherwise dont even set pgrp's as the jobs would
	 * then have no way to get the tty (we can't give it
	 * to them, and our parent wouldn't know their pgrp, etc.
	 */
	execute(kp, tpgrp > 0 ? tpgrp : -1, (int *)0, (int *)0);
}

void
doelse(void)
{

	search(ZELSE, 0, (uchar_t)NULL);
}

void
dogoto(uchar_t **v)
{
	register struct whyle *wp;
	uchar_t *lp;

	/*
	 * While we still can, locate any unknown ends of existing loops.
	 * This obscure code is the WORST result of the fact that we
	 * don't really parse.
	 */
	for (wp = whyles; wp; wp = wp->w_next)
		if (wp->w_end == 0) {
			search(ZBREAK, 0, (uchar_t)NULL);
			wp->w_end = btell();
		} else
			bseek(wp->w_end);
	search(ZGOTO, 0, lp = globone(v[1]));
	xfree(lp);
	/*
	 * Eliminate loops which were exited.
	 */
	wfree();
}

void
doswitch(register uchar_t **v)
{
	register uchar_t *cp, *lp;

	v++;
	if (!*v || *(*v++) != '(')
		goto syntax;
	cp = (**v == ')') ? (uchar_t *)"" : *v++;
	if (*(*v++) != ')')
		v--;
	if (*v)
syntax:
		error(MSGSTR(M_SYNERR, "Syntax error"));
	search(ZSWITCH, 0, lp = globone(cp));
	xfree(lp);
}

void
dobreak(void)
{

	if (whyles)
		toend();
	else
		bferr(MSGSTR(M_WHILE, "Not in while/foreach"));
}

void
doexit(uchar_t **v)
{

	if (chkstop == 0)
		panystop(0);
	/*
	 * Don't DEMAND parentheses here either.
	 */
	v++;
	if (*v) {
		set((uchar_t *)"status", putn(exp(&v)));
		if (*v)
			bferr(MSGSTR(M_EXPR, "Expression syntax"));
	}

	btoeof();
	if (intty)
		close(SHIN);
}

void
doforeach(register uchar_t **v)
{
	register uchar_t *cp;
	register struct whyle *nwp;
#ifndef _SBCS
	int n;
	wchar_t nlc;
#endif

	v++;
	cp = strip(*v);
#ifndef _SBCS
        if (*cp) {
            n = mbtowc(&nlc, (char *)cp, mb_cur_max);
	    if (n < 1) {
		n = 1;
		nlc = *cp & 0xff;
	    }
            if (letter(nlc))
                for (cp += n; *cp; cp += n) {
                    n = mbtowc(&nlc, (char *)cp, mb_cur_max);
		    if (n < 1) {
			n = 1;
			nlc = *cp & 0xff;
		    }
                    if (!alnum(nlc))
                        break;
                }
        }
        if (*cp || strlen((char *)*v) >= 20*MB_LEN_MAX)
#else
        if (letter(*cp))
		while (alnum(*++cp));
        if (*cp || strlen(*v) >= 20)
#endif
		bferr(MSGSTR(M_INVVAR, "Invalid variable"));
	cp = *v++;
	if (v[0][0] != '(' || v[blklen(v) - 1][0] != ')')
		bferr(MSGSTR(M_PAREN, "Words not ()'ed"));
	v++;
	gflag = 0;
	rscan(v, tglob);
        v = glob(v);
        if (v == 0)
                bferr(MSGSTR(M_NOMATCH, "No match"));
	nwp = (struct whyle *)calloc(1, sizeof *nwp);
	nwp->w_fe = nwp->w_fe0 = v; gargv = 0;
	nwp->w_start = btell();
	nwp->w_fename = savestr(cp);
	nwp->w_next = whyles;
	whyles = nwp;
	/*
	 * Pre-read the loop so as to be more
	 * comprehensible to a terminal user.
	 */
	if (intty)
		preread();
	doagain();
}

void
dowhile(uchar_t **v)
{
	register long status;                              /* 002 */
	register bool again = whyles != 0 && whyles->w_start == lineloc &&
	    whyles->w_fename == 0;

	v++;
	/*
	 * Implement prereading here also, taking care not to
	 * evaluate the expression before the loop has been read up
	 * from a terminal.
	 */
	if (intty && !again)
		status = !exp0(&v, 1);
	else
		status = !exp(&v);
	if (*v)
		bferr(MSGSTR(M_EXPR, "Expression syntax"));
	if (!again) {
		register struct whyle *nwp;

		nwp = (struct whyle *)calloc(1, sizeof (*nwp));
		nwp->w_start = lineloc;
		nwp->w_end = 0;
		nwp->w_next = whyles;
		whyles = nwp;
		if (intty) {
			/*
			 * The tty preread
			 */
			preread();
			doagain();
			return;
		}
	}
	if (status)
		/* We ain't gonna loop no more, no more! */
		toend();
}

void
preread(void)
{

	whyles->w_end = -1;
	if (setintr)
		sigrelse(SIGINT);
	search(ZBREAK, 0, (uchar_t)NULL);
	if (setintr)
		sighold(SIGINT);
	whyles->w_end = btell();
}

void
doend(void)
{

	if (!whyles)
		bferr(MSGSTR(M_WHILE, "Not in while/foreach"));
	whyles->w_end = btell();
	doagain();
}

void
docontin(void)
{

	if (!whyles)
		bferr(MSGSTR(M_WHILE, "Not in while/foreach"));
	doagain();
}

void
doagain(void)
{

	/* Repeating a while is simple */
	if (whyles->w_fename == 0) {
		bseek(whyles->w_start);
		return;
	}
	/*
	 * The foreach variable list actually has a spurious word
	 * ")" at the end of the w_fe list.  Thus we are at the
	 * of the list if one word beyond this is 0.
	 */
	if (!whyles->w_fe[1]) {
		dobreak();
		return;
	}
	set(whyles->w_fename, savestr(*whyles->w_fe++));
	bseek(whyles->w_start);
}

void
dorepeat(uchar_t **v, struct command *kp)
{
	register int i;

	i = getn(v[1]);
	if (setintr)
		sighold(SIGINT);
	lshift(v, 2);
	while (i > 0) {
		if (setintr)
			sigrelse(SIGINT);
		reexecute(kp);
		--i;
	}
	if (didfds)
		donefds();
	if (setintr)
		sigrelse(SIGINT);
}

void
doswbrk(void)
{

	search(ZBRKSW, 0, (uchar_t)NULL);
}

int
srchx(register uchar_t *cp)
{
	register struct srch *sp;

	for (sp = srchn; sp->s_name; sp++)
		if (EQ(cp, sp->s_name))
			return (sp->s_value);
	return (-1);
}

uchar_t	Stype;
uchar_t	*Sgoal;

void
search(int type, register int level, uchar_t *goal)
{
	uchar_t wordbuf[BUFR_SIZ];
	register uchar_t *aword = wordbuf;
	register uchar_t *cp;
	extern uchar_t *linp, linbuf[];

	Stype = type; Sgoal = goal;
	if (type == ZGOTO)
		bseek(0L);
	do {
		if (intty && fseekp == feobp) {
			csh_printf("? ");
			flush();
		}
		aword[0] = 0, getword(aword);
		switch (srchx(aword)) {

		case ZELSE:
			if (level == 0 && type == ZIF)
				return;
			break;

		case ZIF:
			while (getword(aword))
				continue;
			if ((type == ZIF || type == ZELSE) && EQ(aword, "then"))
				level++;
			break;

		case ZENDIF:
			if (type == ZIF || type == ZELSE)
				level--;
			break;

		case ZFOREACH:
		case ZWHILE:
			if (type == ZBREAK)
				level++;
			break;

		case ZEND:
			if (type == ZBREAK)
				level--;
			break;

		case ZSWITCH:
			if (type == ZSWITCH || type == ZBRKSW)
				level++;
			break;

		case ZENDSW:
			if (type == ZSWITCH || type == ZBRKSW)
				level--;
			break;

		case ZLABEL:
			if (type == ZGOTO && getword(aword) && EQ(aword, goal))
				level = -1;
			break;

		default:
			if (type != ZGOTO && (type != ZSWITCH || level != 0))
				break;
			if (lastchr(aword) != ':')
				break;
			aword[strlen((char *)aword) - 1] = 0;
			if (type == ZGOTO && EQ(aword, goal) || type == ZSWITCH && EQ(aword, "default"))
				level = -1;
			break;

		case ZCASE:
			if (type != ZSWITCH || level != 0)
				break;
			getword(aword);
			if (lastchr(aword) == ':')
				aword[strlen((char *)aword) - 1] = 0;
			cp = strip(Dfix1(aword));
			if (Gmatch(goal, cp))
				level = -1;
			xfree(cp);
			break;

		case ZDEFAULT:
			if (type == ZSWITCH && level == 0)
				level = -1;
			break;
		}
		getword(NOSTR);
	} while (level >= 0);
}

int
getword(register uchar_t *wp)
{
	register int found = 0;
	register int d;
#ifndef _SBCS
	wchar_t c;
#else
	register int c;
#endif

	c = readc(1);
	d = 0;
	do {
#ifndef _SBCS
	   while (c == ' ' || c == '\t' ||  c == '(' || iswblank(c))
#else
	   while (c == ' ' || c == '\t' || c == '(')
#endif
	      c = readc(1);
	   if (c == '#') {
	      do {
		 c = readc(1);
	      } while ((!IsEOF(c)) && c != '\n');
	   }
	   if (IsEOF(c)) goto past;
	   if (c == '\n') {
	      if (wp) break;
	      return (0);
	   }
	   unreadc(c);
	   found = 1;
	   do {
	      c = readc(1);
	      if (c == '\\' && (c = readc(1)) == '\n')
		  c = ' ';
	      if (strchr("'\"",c)) {
		 if (d == 0)
		     d = c;
		 else if (d == c)
		     d = 0;
	      }
	      if (IsEOF(c)) goto past;
	      if (wp)
#ifndef _SBCS
		 PUTCH (wp,c);
	   } while ((d || c != ' ' && c != '\t' && c != '('  &&
		   !iswblank(c)) && c != '\n');
#else
	         *wp++ = c;
	   } while ((d || c != ' ' && c != '\t' && c != '(') &&
		   c != '\n');
#endif
	} while (wp == 0);
        unreadc(c);
        if (found) *--wp = 0;
	return (found);
past:
	switch (Stype) {

	case ZIF:
		bferr(MSGSTR(M_ZIF, "then/endif not found"));

	case ZELSE:
		bferr(MSGSTR(M_ZELSE, "endif not found"));

	case ZBRKSW:
	case ZSWITCH:
		bferr(MSGSTR(M_ZSWITCH, "endsw not found"));

	case ZBREAK:
		bferr(MSGSTR(M_ZBREAK, "end not found"));

	case ZGOTO:
		setname(Sgoal);
		bferr(MSGSTR(M_ZGOTO, "label not found"));
	}
	/*NOTREACHED*/
}

void
toend(void)
{

	if (whyles->w_end == 0) {
		search(ZBREAK, 0, (uchar_t)NULL);
		whyles->w_end = btell() - 1;
	} else
		bseek(whyles->w_end);
	wfree();
}

void
wfree(void)
{
	off_t seek_ptr = btell();

	while (whyles) {
		register struct whyle *wp = whyles;
		register struct whyle *nwp = wp->w_next;

		if (seek_ptr >= wp->w_start && 
		(wp->w_end == 0 || seek_ptr < wp->w_end))
			break;
		if (wp->w_fe0)
			blkfree(wp->w_fe0);
		if (wp->w_fename)
			xfree(wp->w_fename);
		xfree((uchar_t *)wp);
		whyles = nwp;
	}
}

void
doecho(uchar_t **v)
{
	echo(' ', v);
}

void
doglob(uchar_t **v)
{

	echo(0, v);
	flush();
}

void
echo(uchar_t sep, register uchar_t **v)
{
	register uchar_t *cp;
	int nonl = 0;

	if (setintr)
		sigrelse(SIGINT);
	v++;
	if (*v == 0)
		return;
	gflag = 0;
	rscan(v, tglob);
	if (gflag) {
		v = glob(v);
		if (v == 0)
			bferr(MSGSTR(M_NOMATCH, "No match"));
		if (*v == 0)
			return;
	} else
		scan(v, trim);
	if (sep && !strcmp((char *)*v, "-n")) {
		nonl++;
		v++;
	}
	while (cp = *v++) {
		register int c;
		/* quoting here is to print control uchar_ts as is */
		while (c = *cp++) {
#ifndef _SBCS
			if (c < ' ' || c == 0177)
#else
			if (c < 034 || c == 0177)
#endif
				display_char(NLQUOTE);
			display_char(c);
		}
		if (*v) {
			if (!sep)
				display_char(NLQUOTE);
			display_char(sep);
		}
	}
	if (sep && !nonl)
		display_char('\n');
	else
		flush();
	if (setintr)
		sighold(SIGINT);
	if (gargv)
		blkfree(gargv), gargv = 0;
}


void
dosetenv(register uchar_t **v)
{
	uchar_t *vp, *lp;

	v++;
	if ((vp = *v++) == (uchar_t *)0) {
		register uchar_t **ep;

		if (setintr)
			(void)sigsetmask(sigblock(0) & ~ sigmask(SIGINT));
		for (ep = (uchar_t **)environ; *ep; ep++)
			csh_printf("%s\n", *ep);
		return;
	}
	if ((lp = *v++) == (uchar_t *)0)
            setcenv((char *)vp, (uchar_t *)"");
	setcenv((char *)vp, lp = globone(lp));
	if (EQ(vp, "PATH")) {
		importpath(lp);
		dohash();
	}
        else if (EQ(vp, "LANG") || EQ(vp, "LC_ALL") || EQ(vp, "NLSPATH"))  {
		catclose(catd);
		setlocale(LC_ALL, "");
		catd = catopen(MF_CSH, NL_CAT_LOCALE);
		(void)MSGSTR(M_BYEBYE,"");
        }
        else if (EQ(vp, "LC_CTYPE")) setlocale(LC_CTYPE, "");
        else if (EQ(vp, "LC_COLLATE")) setlocale(LC_COLLATE, "");
        else if (EQ(vp, "LC_MONETARY")) setlocale(LC_MONETARY, "");
        else if (EQ(vp, "LC_NUMERIC")) setlocale(LC_NUMERIC, "");
        else if (EQ(vp, "LC_TIME")) setlocale(LC_TIME, "");
        else if (EQ(vp, "LC_MESSAGES"))  {
		catclose(catd);
		setlocale(LC_MESSAGES, "");
		catd = catopen(MF_CSH, NL_CAT_LOCALE);
		(void)MSGSTR(M_BYEBYE,"");
        }
#ifndef _SBCS
	mb_cur_max = MB_CUR_MAX;
	iswblank_handle = wctype("blank");
#endif
	xfree(lp);
}

void
dounsetenv(register uchar_t **v)
{
	v++;
	do {
		unsetcenv(*v++);
	}while (*v);
}

void
setcenv(char *name, uchar_t *value)
{
	register uchar_t **ep = (uchar_t **)environ;
	register uchar_t *cp, *dp;
	uchar_t *blk[2], **oep = ep;

#ifdef DEBUG
csh_printf("debug: environ=0x%x\n", ep);
csh_printf("debug: setcenv(%s, %s)\n", name, value);
#endif
	for (; *ep; ep++) {
#ifdef DEBUG
csh_printf("debug:ep=0x%x, *ep=0x%x\n", ep, *ep);
csh_printf("debug: *ep=%s\n", *ep);
csh_printf("debug:**ep=%c\n", **ep);
#endif
		for (cp=(uchar_t *)name, dp =*ep; *cp && *cp == *dp; cp++, dp++)
{
#ifdef DEBUG
csh_printf("debug: *dp=%c\n", *dp);
#endif
			continue;
}
#ifdef DEBUG
csh_printf("debug name=%s\n",name);
#endif
		if (*cp != 0 || *dp != '=')
			continue;
		cp = (uchar_t *)strspl((uchar_t *)"=", value);
#ifdef DEBUG
csh_printf("debug: before first free, *ep=%s\n", *ep);
#endif
		xfree(*ep);
		*ep = strspl((uchar_t *)name, cp);
#ifdef DEBUG
csh_printf("debug: before second free, cp=%s\n", cp);
#endif
		xfree(cp);
		scan(ep, trim);
		return;
	}
#ifdef DEBUG
csh_printf("here\n");
#endif
	blk[0] = strspl((uchar_t *)name, (uchar_t *)"=");
	blk[1] = 0;
#ifdef DEBUG
csh_printf("here\n");
#endif
	environ = (char **)blkspl((uchar_t **)environ, (uchar_t **)blk);
#ifdef DEBUG
csh_printf("here\n");
#endif
	xfree((uchar_t *)oep);
	setcenv(name, value);
}

void
unsetcenv(uchar_t *name)
{
	register uchar_t **ep = (uchar_t **)environ;
	register uchar_t *cp, *dp;
	uchar_t **oep = ep;

	for (; *ep; ep++) {
		for (cp = name, dp = *ep; *cp && *cp == *dp; cp++, dp++)
			continue;
		if (*cp != 0 || *dp != '=')
			continue;
		cp = *ep;
		*ep = (uchar_t *)NULL;
		environ = (char **)blkspl((uchar_t **)environ, ep+1);
		*ep = cp;
		xfree(cp);
		xfree((uchar_t *)oep);
		return;
	}
}

void
doumask(register uchar_t **v)
{
	register uchar_t *cp = v[1];
	register int i;

	if (cp == 0) {
		i = umask(0);
		umask(i);
		csh_printf("%o\n", i);
		return;
	}
	i = 0;
	while (*cp >= '0' && *cp <= '7')
		i = i * 8 + *cp++ - '0';
	if (*cp || i > 0777 || i < 0)
		bferr(MSGSTR(M_MASK,"Improper mask"));
	umask(i);
}

struct limits {
        int     limconst;
        char    *limname;
        long     limdiv;    /* 001 RNF */
        char    *limscale;
} limits[] = {
        RLIMIT_CPU,     "cputime",      1,      "seconds",
        RLIMIT_FSIZE,   "filesize",     1024,   "kbytes",
        RLIMIT_DATA,    "datasize",     1024,   "kbytes",
        RLIMIT_STACK,   "stacksize",    1024,   "kbytes",
        RLIMIT_CORE,    "coredumpsize", 1024,   "kbytes",
        RLIMIT_RSS,     "memoryuse",    1024,   "kbytes",
	RLIMIT_NOFILE,  "descriptors",  1,      "files",
#ifdef	RLIMIT_AS
	RLIMIT_AS,      "addressspace", 1024,   "kbytes",
#endif
        -1,             0,
};


struct limits *
findlim(uchar_t *cp)
{
	register struct limits *lp, *res;

	res = 0;
	for (lp = limits; lp->limconst >= 0; lp++)
		if (prefix(cp, (uchar_t *)lp->limname)) {
			if (res)
				bferr(MSGSTR(M_AMBIG, "Ambiguous"));
			res = lp;
		}
	if (res)
		return (res);
	bferr(MSGSTR(M_NOLIMIT, "No such limit"));
}

void
dolimit(register uchar_t **v)
{
	register struct limits *lp;
	register long limit;
	uchar_t hard = 0;
	long getval();

	v++;
        if (*v && EQ(*v, "-h")) {
                hard = 1;
                v++;
        }
	if (*v == 0) {
		for (lp = limits; lp->limconst >= 0; lp++)
			plim(lp, hard);
		return;
	}
	lp = findlim(v[0]);
	if (v[1] == 0) {
		plim(lp, hard);
		return;
	}
	limit = getval(lp, v+1);
        if (setlim(lp, hard, limit) < 0)
                error((char *)NOSTR);
}

long
getval(register struct limits *lp, uchar_t **v)
{
	register double f;
	double atof();
	char **ptr;
	uchar_t *cp = *v++;

	f = atof(cp);
	while (digit(*cp) || *cp == '.' || *cp == 'e' || *cp == 'E')
		cp++;
	if (*cp == 0) {
	        if (*v == 0)
		    return ((long)(f+0.5) * lp->limdiv);
		cp = *v;
	}
	switch (*cp) {

	case ':':
		if (lp->limconst != RLIMIT_CPU)
			goto badscal;
		return ((long)(f * 60.0 + atof(cp+1)));

	case 'h':
		if (lp->limconst != RLIMIT_CPU)
			goto badscal;
		limtail(cp, (uchar_t *)"hours");
		f *= 3600.;
		break;

	case 'b':
		if (lp->limconst == RLIMIT_CPU)
			goto badscal;
		limtail(cp, (uchar_t *)"blocks");
		f *= 512.;
		break;
	case 'm':
		if (lp->limconst == RLIMIT_CPU) {
			limtail(cp, (uchar_t *)"minutes");
			f *= 60.;
			break;
		}
	case 'M':
		if (lp->limconst == RLIMIT_CPU)
			goto badscal;
		*cp = 'm';
		limtail(cp, (uchar_t *)"megabytes");
		f *= 1024.*1024.;
		break;

	case 's':
		if (lp->limconst != RLIMIT_CPU)
			goto badscal;
		limtail(cp, (uchar_t *)"seconds");
		break;

	case 'k':
		if (lp->limconst == RLIMIT_CPU)
			goto badscal;
		limtail(cp, (uchar_t *)"kbytes");
		f *= 1024;
		break;

	case 'u':
		limtail(cp, (uchar_t *)"unlimited");
		return (RLIM_INFINITY);

	default:
badscal:
		bferr(MSGSTR(M_SCALE, "Improper or unknown scale factor"));
	}
	return ((long)(f+0.5));
}

void
limtail(uchar_t *cp, uchar_t *str0)
{
	register uchar_t *str = (uchar_t *)str0;

	while (*cp && *cp == *str)
		cp++, str++;
	if (*cp)
		error(MSGSTR(M_BAD, "Bad scaling"));
}

void
plim(register struct limits *lp, uchar_t hard)
{
	struct rlimit rlim;
	long lim; /* WW-01 */

	csh_printf("%s \t", lp->limname);
        (void) getrlimit(lp->limconst, &rlim);
        lim = hard ? rlim.rlim_max : rlim.rlim_cur;
	if (lim == RLIM_INFINITY)
		csh_printf(MSGSTR(M_UNLIMITED, "unlimited"));
	else if (lp->limconst == RLIMIT_CPU)
		psecs((long)lim);
	else
		csh_printf("%d %s", lim / lp->limdiv, lp->limscale);
	csh_printf("\n");
}

void
dounlimit(register uchar_t **v)
{
        register struct limits *lp;
	struct rlimit rlim; 	/* 001 RNF */
        int err = 0;
        uchar_t hard = 0;

        v++;
        if (*v && EQ(*v, "-h")) {
                hard = 1;
                v++;
        }
        if (*v == 0) {
                for (lp = limits; lp->limconst >= 0; lp++) {
			/************************************************
			* 001 RNF - If current limit is RLIM_INFINITY *
			*            then its already unlimited, so    *
			*            don't bother setting it           *
			************************************************/
			(void) getrlimit(lp->limconst, &rlim);
			if (rlim.rlim_cur != (long)RLIM_INFINITY) {
			     if (setlim(lp, hard, RLIM_INFINITY) < 0)/* WW-01*/
					err++;
			}
		}
                if (err)
                        error((char *)NOSTR);
                return;
        }
        while (*v) {
                lp = findlim(*v++);
		/************************************************
		* 001 RNF - If current limit is RLIM_INFINITY *
		*            then its already unlimited, so    *
		*            don't bother setting it           *
		************************************************/
		(void) getrlimit(lp->limconst, &rlim);
		if (rlim.rlim_cur != (long)RLIM_INFINITY) {
			if (setlim(lp, hard, RLIM_INFINITY) < 0) /* WW-01 */
				error((char *)NOSTR);
		}
        }
}

int
setlim(register struct limits *lp, uchar_t hard, long limit) /* WW-01 */
{
        struct rlimit rlim;

        (void) getrlimit(lp->limconst, &rlim);
        if (hard)
                rlim.rlim_max = limit;
        else if (limit == RLIM_INFINITY && geteuid() != 0)
                rlim.rlim_cur = rlim.rlim_max;
        else
                rlim.rlim_cur = limit;

       	if (setrlimit(lp->limconst, &rlim) < 0) {
		perror("");
               	csh_printf("%s: %s: Can't %s%s limit\n", bname, lp->limname,
               		(limit == RLIM_INFINITY) ? "remove" : "set", 
			hard ? " hard" : "");
               	return (-1);
       	}
        return (0);
}

void
dosuspend(void)
{
	int old, ldisc;
	pid_t ctpgrp;
	struct sigvec nsv, osv;	

	if (loginsh)
		error(MSGSTR(M_SUSPEND, "Can't suspend a login shell (yet)"));
	untty();
	nsv.sv_handler = SIG_DFL;
	nsv.sv_mask = SA_RESTART;
	nsv.sv_onstack = 0;
	(void)sigvec(SIGTSTP, &nsv, &osv);
	kill(0, SIGTSTP);
	/* the shell stops here */
	(void)sigvec(SIGTSTP, &osv, (struct sigvec *)NULL);
	if (tpgrp != -1) {
retry:
		IOCTL(FSHTTY, TIOCGPGRP, &ctpgrp, "15");
		if (ctpgrp != opgrp) {
			(void)sigvec(SIGTTIN, &nsv, &osv);
			kill(0, SIGTTIN);
			(void)sigvec(SIGTTIN, &osv, (struct sigvec *)NULL);
			goto retry;
		}
		nsv.sv_handler = SIG_IGN;
		(void)sigvec(SIGTTOU, &nsv, &osv);
		IOCTL(FSHTTY, TIOCSPGRP, &shpgrp, "16");
		(void)sigvec(SIGTTOU, &osv, (struct sigvec *)NULL);
		setpgrp(0, shpgrp);
	}
}

void
doeval(uchar_t **v)
{
	uchar_t **oevalvec = evalvec;
	uchar_t *oevalp = evalp;
	jmp_buf osetexit;
	static int reenter = 0; /* CMR001 allow reenter to accumulate */
	uchar_t **gv = 0;

	v++;
	if (*v == 0)
		return;
	gflag = 0; 
        rscan(v, tglob);
	if (gflag) 
	{
		gv = glob(v);
		v = gv;
		gargv = 0;
		if (v == 0)
			error(MSGSTR(M_NOMATCH, "No match"));
		v = copyblk(v);
	} else
		scan(v, trim);
	getexit(osetexit);
        if(reenter >= MAX_RECURSION)
           reenter = 0;         /*CMR001 allow reenter to accumulate    */
	setexit();
	reenter++;
        if (reenter >= 1 && reenter < MAX_RECURSION) {
                                                /* CMR001 limit recursion*/
		evalvec = v;
		evalp = 0;
		process(0);
                reenter--; /*CMR001 Decrement recursion tracking after  */
                           /*CMR001 process is finished.                */
	}
	evalvec = oevalvec;
	evalp = oevalp;
	doneinp = 0;
	if (gv)
		blkfree(gv);
	resexit(osetexit);
        if (reenter == MAX_RECURSION){  /* CMR001 limit recursion*/
                error(MSGSTR(M_RECURS,"Too many Levels of recursion"));
                reenter =0;
                }
}
/*   The dowhich and executable functions are from  DEC OSF1.2
 * After much disucussion, it was decided to have this fast clean internal
 * version of 'which' produce the same ugly output as it slower counterpart
 * /usr/ucb/which.  A -u (useful) flag was added to allow a user to alias
 * which to which -u for a cleaner more useful form of the command.
 * tonyb@tek
 */
void
dowhich(uchar_t **vec)
{
	register struct biltins *bp;
	register uchar_t *word, **pp;
	struct varent *vp, *pth;
	uchar_t dir[PATH_MAX+1];
	int cmp, hit, outty, uflag;


	/*
	 * Glob it up
	 */
	gflag = 0;
	uflag = 1;
        rscan(++vec, tglob);
	if(gflag) { 			/* has globbing chars */
		vec = glob(vec);	/* glob it */
		if(vec == 0)
			bferr(MSGSTR(M_NOMATCH,"No match"));
	} else
		scan(vec, trim);

	pth = adrof((uchar_t *)"path");

	/*
	 * Find out if we're writting to a tty.  If not, use "command type"
	 * output, else use "user" type output.
	 * command type output is used to do things like 'echo `which foobar`'
	 */
	outty = isatty(1);

	/*
	 * If no -u, go into useless mode, else provide useable,
	 * coherent output.
	 */
	if(*vec && EQ(*vec,"-u")) {
		uflag++;
		vec++;
	}
	/*
	 * Shouldn't be needed, but allow '-U' to go into useless mode.
	 */
	if(*vec && EQ(*vec,"-U")) {
		uflag = 0;
		vec++;
	}

	while(word = *vec++) {

		/*
		 * check aliases first.  
		 */
		vp = adrof1(word, &aliases);
		if(vp) {
			if(!uflag) {
				csh_printf(MSGSTR(M_W_ALIASED_TO,"%s: \t aliased to '"), word);
				blkpr(vp->vec); csh_printf("'\n");
			} else if(outty) {
				csh_printf(MSGSTR(M_W_ALIAS,"alias/%s '"), word);
				blkpr(vp->vec); csh_printf("'\n");
			} else
				csh_printf("%s\n",word);
			continue;
		}

		/*
		 * If a hard path, just return it if it is executable, else
		 * return nothing.
		 */
		if(index(word, '/') ) {
			if(executable((char *)word)) {
				csh_printf("%s\n",word);
				continue;
			}
			else if( !uflag ) {
				csh_printf(MSGSTR(M_W_NOT_FOUND,"%s not found\n"), word);
				continue;
			}
		}

		/*
		 * Check builtins
		 * "standard" /usr/ucb/which does not support the concept
		 * of builtin commands. YUK!!!
		 */
		if(uflag) {
			hit = 0;
			for (bp = bfunc; bp->bname; bp++) {
				if ((cmp = strcmp (bp->bname, word)) == 0) {
					if (outty)
						csh_printf(MSGSTR(M_W_BUILTIN,"builtin/%s\n"),word);
					else
						csh_printf("%s\n",word);
					hit++;
				} else if (cmp > 0)
					break;
			}
			if(hit) 
				continue;
		}


		/*
		 * Check the paths
		 * No need to read 'em, just check if it is executable.
		 */
		if(pth) {
			register uchar_t *ep;

			hit = 0;
			for(pp = pth->vec; pp && *pp && **pp; pp++) {
				ep = (uchar_t *)strcpy(dir,*pp);
				while(*ep)
					ep++;
				*ep++ = '/';
				(void)strcpy(ep,word);
				if(executable((char *)dir)) {
					csh_printf("%s\n",dir);
					hit++;
					break;
				}
			}
		}

		/*
		 * If not found and in useless mode
		 */
		if(!hit && !uflag) {
			csh_printf(MSGSTR(M_W_NO_IN,"no %s in "), word);
			blkpr(pth->vec);
			csh_printf("\n");
		}
	} /* end while */
}

int
executable(register char *path)
{
	struct stat st;

	if(access(path,1) || stat(path, &st) )
		st.st_mode = 0;
	return( (st.st_mode & S_IFMT) == S_IFREG);
}
