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
static char rcsid[] = "@(#)$RCSfile: exp.c,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1994/01/18 15:43:56 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: exp exp0 exp1 exp2 exp2a exp2b exp2c exp3 exp3a
 *            exp4 exp5 exp6 evalav isa egetn
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 */ 

#include <sys/access.h>
#include <sys/mode.h>
#include "sh.h"

#define IGNORE	0x1	/* in ignore, it means to ignore value, just parse */
#define NOGLOB	0x2	/* in ignore, it means not to globone */

#define	ADDOP	0x1
#define	MULOP	0x2
#define	EQOP	0x4
#define	RELOP	0x8
#define	RESTOP	0x10
#define	ANYOP	(ADDOP + MULOP + EQOP + RELOP + RESTOP)

#define	EQEQ		0x1
#define	GTR		0x2	
#define	LSS		0x4
#define	NOTEQ		0x6
#define EQMATCH		0x7
#define NOTEQMATCH	0x8

#define GTREQ		(GTR | EQEQ)
#define LSSEQ		(LSS | EQEQ) 

long                                                      /* 002 */
exp(uchar_t ***vp)
{
	return (exp0(vp, 0));
}

long                                                      /* 002 */
exp0(uchar_t ***vp, bool ignore)
{
	register long p1 = exp1(vp, ignore);              /* 002 */
	
#ifdef DEBUG
etraci("exp0 p1", p1, vp);
#endif
	if (**vp && EQ(**vp, "||")) {
		register int p2;

		(*vp)++;
		p2 = exp0(vp, (ignore&IGNORE) || p1);
#ifdef DEBUG
etraci("exp0 p2", p2, vp);
#endif
		return (p1 || p2);
	}
	return (p1);
}

long                                                     /* 002 */
exp1(uchar_t ***vp, bool ignore)
{
	register long  p1 = exp2(vp, ignore);

#ifdef DEBUG
etraci("exp1 p1", p1, vp);
#endif
	if (**vp && EQ(**vp, "&&")) {
		register int p2;

		(*vp)++;
		p2 = exp1(vp, (ignore&IGNORE) || !p1);
#ifdef DEBUG
etraci("exp1 p2", p2, vp);
#endif
		return (p1 && p2);
	}
	return (p1);
}

long                                                        /* 002 */
exp2(uchar_t ***vp, bool ignore)
{
	register long p1 = exp2a(vp, ignore);               /* 002 */

#ifdef DEBUG
etraci("exp3 p1", p1, vp);
#endif
	if (**vp && EQ(**vp, "|")) {
		register int p2;

		(*vp)++;
		p2 = exp2(vp, ignore);
#ifdef DEBUG
etraci("exp3 p2", p2, vp);
#endif
		return (p1 | p2);
	}
	return (p1);
}

long                                                       /* 002 */
exp2a(uchar_t ***vp, bool ignore)
{
	register long p1 = exp2b(vp, ignore);             /* 002 */

#ifdef DEBUG
etraci("exp2a p1", p1, vp);
#endif
	if (**vp && EQ(**vp, "^")) {
		register int p2;

		(*vp)++;
		p2 = exp2a(vp, ignore);
#ifdef DEBUG
etraci("exp2a p2", p2, vp);
#endif
		return (p1 ^ p2);
	}
	return (p1);
}

long                                                          /* 002 */
exp2b(uchar_t ***vp, bool ignore)
{
	register long p1 = exp2c(vp, ignore);                /* 002 */

#ifdef DEBUG
etraci("exp2b p1", p1, vp);
#endif
	if (**vp && EQ(**vp, "&")) {
		register int p2;

		(*vp)++;
		p2 = exp2b(vp, ignore);
#ifdef DEBUG
etraci("exp2b p2", p2, vp);
#endif
		return (p1 & p2);
	}
	return (p1);
}

long                                                        /* 002 */
exp2c(uchar_t ***vp, bool ignore)
{
	register uchar_t *p1 = exp3(vp, ignore);
	register uchar_t *p2;
	register long i;                                   /* 002 */

#ifdef DEBUG
etracc("exp2c p1", p1, vp);
#endif
	if (i = isa(**vp, EQOP)) {
		(*vp)++;
		if (i == EQMATCH || i == NOTEQMATCH)
			ignore |= NOGLOB;
		p2 = exp3(vp, ignore);
#ifdef DEBUG
etracc("exp2c p2", p2, vp);
#endif
		if (!(ignore&IGNORE)) switch (i) {

		case EQEQ:
			i = EQ(p1, p2);
			break;

		case NOTEQ:
			i = !EQ(p1, p2);
			break;

		case EQMATCH:
			i = Gmatch(p1, p2);
			break;

		case NOTEQMATCH:
			i = !Gmatch(p1, p2);
			break;
		}
		xfree(p1);
		xfree(p2);
		return (i);
	}
	i = egetn(p1);
	xfree(p1);
	return (i);
}

uchar_t *
exp3(uchar_t ***vp, bool ignore)
{
	register uchar_t *p1, *p2;
	register long i;                                     /* 002 */

	p1 = exp3a(vp, ignore);
#ifdef DEBUG
etracc("exp3 p1", p1, vp);
#endif
	if (i = isa(**vp, RELOP)) {
		(*vp)++;
		if (**vp && EQ(**vp, "="))
			i |= EQEQ, (*vp)++;
		p2 = exp3(vp, ignore);
#ifdef DEBUG
etracc("exp3 p2", p2, vp);
#endif
		if (!(ignore&IGNORE)) switch (i) {

		case GTR:
			i = egetn(p1) > egetn(p2);
			break;

		case GTREQ:
			i = egetn(p1) >= egetn(p2);
			break;

		case LSS:
			i = egetn(p1) < egetn(p2);
			break;

		case LSSEQ:
			i = egetn(p1) <= egetn(p2);
			break;
		}
		xfree(p1);
		xfree(p2);
		return (putn(i));
	}
	return (p1);
}

uchar_t *
exp3a(uchar_t ***vp, bool ignore)
{
	register uchar_t *p1, *p2, *op;
	register long i;                                    /* 002 */

	p1 = exp4(vp, ignore);
#ifdef DEBUG
etracc("exp3a p1", p1, vp);
#endif
	op = **vp;
	if (op && any(op[0],(uchar_t *)"<>") && op[0] == op[1]) {
		(*vp)++;
		p2 = exp3a(vp, ignore);
#ifdef DEBUG
etracc("exp3a p2", p2, vp);
#endif
		if (op[0] == '<')
			i = egetn(p1) << egetn(p2);
		else
			i = egetn(p1) >> egetn(p2);
		xfree(p1);
		xfree(p2);
		return (putn(i));
	}
	return (p1);
}

uchar_t *
exp4(uchar_t ***vp, bool ignore)
{
	register uchar_t *p1, *p2;
	register long i = 0L;                              /* 002 */

	p1 = exp5(vp, ignore);
#ifdef DEBUG
etracc("exp4 p1", p1, vp);
#endif
	if (isa(**vp, ADDOP)) {
		register uchar_t *op = *(*vp)++;

		p2 = exp4(vp, ignore);
#ifdef DEBUG
etracc("exp4 p2", p2, vp);
#endif
		if (!(ignore&IGNORE)) switch (op[0]) {

		case '+':
			i = egetn(p1) + egetn(p2);
			break;

		case '-':
			i = egetn(p1) - egetn(p2);
			break;
		}
		xfree(p1);
		xfree(p2);
		return (putn(i));
	}
	return (p1);
}

uchar_t *
exp5(uchar_t ***vp, bool ignore)
{
	register uchar_t *p1, *p2;
	register long i = 0L;                               /* 002 */

	p1 = exp6(vp, ignore);
#ifdef DEBUG
etracc("exp5 p1", p1, vp);
#endif
	if (isa(**vp, MULOP)) {
		register uchar_t *op = *(*vp)++;

		p2 = exp5(vp, ignore);
#ifdef DEBUG
etracc("exp5 p2", p2, vp);
#endif
		if (!(ignore&IGNORE)) switch (op[0]) {

		case '*':
			i = egetn(p1) * egetn(p2);
			break;

		case '/':
			i = egetn(p2);
			if (i == 0)
				error(MSGSTR(M_DIV, "Divide by 0"));
			i = egetn(p1) / i;
			break;

		case '%':
			i = egetn(p2);
			if (i == 0)
				error(MSGSTR(M_MOD, "Mod by 0"));
			i = egetn(p1) % i;
			break;
		}
		xfree(p1);
		xfree(p2);
		return (putn(i));
	}
	return (p1);
}

uchar_t *
exp6(uchar_t ***vp, bool ignore)
{
	long ccode, i;                                      /* 002 */
	register uchar_t *cp, *dp, *ep;

	if (**vp == 0)              /* Bug fix from DEC OSF/1.2  */
	  bferr(MSGSTR(M_EXPR, "Expression syntax"));
	if (EQ(**vp, "!")) {
		(*vp)++;
		cp = exp6(vp, ignore);
#ifdef DEBUG
etracc("exp6 ! cp", cp, vp);
#endif
		i = egetn(cp);
		xfree(cp);
		return (putn(!i));
	}
	if (EQ(**vp, "~")) {
		(*vp)++;
		cp = exp6(vp, ignore);
#ifdef DEBUG
etracc("exp6 ~ cp", cp, vp);
#endif
		i = egetn(cp);
		xfree(cp);
		return (putn(~i));
	}
	if (EQ(**vp, "(")) {
		(*vp)++;
		ccode = exp0(vp, ignore);
#ifdef DEBUG
etraci("exp6 () ccode", ccode, vp);
#endif
		if (*vp == 0 || **vp == 0 || ***vp != ')')
			bferr(MSGSTR(M_EXPR, "Expression syntax"));
		(*vp)++;
		return (putn(ccode));
	}
	if (EQ(**vp, "{")) {
		register uchar_t **v;
		struct command faket;
		uchar_t *fakecom[2];

		faket.t_dtyp = TCOM;
		faket.t_dflg = 0;
		faket.t_dcar = faket.t_dcdr = faket.t_dspr = (struct command *)0;
		faket.t_dcom = fakecom;
		fakecom[0] = (uchar_t *)"{ ... }";
		fakecom[1] = NOSTR;
		(*vp)++;
		v = *vp;
		for (;;) {
			if (!**vp)
				bferr(MSGSTR(M_MISSBRC, "Missing }"));
			if (EQ(*(*vp)++, "}"))
				break;
		}
		if (ignore&IGNORE)
			return ((uchar_t *)"");
		psavejob();
		if (pfork(&faket, -1) == 0) {
			*--(*vp) = 0;
			evalav(v);
			exitstat();
		}
		pwait();
		prestjob();
#ifdef DEBUG
etraci("exp6 {} status", 
	egetn((uchar_t *)value((uchar_t *)"status")),
	vp);
#endif
		return (putn(egetn(value((uchar_t *)"status")) == 0));
	}
	if (isa(**vp, ANYOP))
		return ((uchar_t *)"");
	cp = *(*vp)++;
	if (*cp == '-' && any(cp[1], (uchar_t *)"erwxfdzol")) {	/* FPM001*/
		struct stat stb;

/*  DEC OSF/1.2 bug fix 
 * The check used to be for ANYOP, but that isn't correct since it caused
 * the expression ( -d / ) to say 'Missing file name'. RESTOP is somewhat
 * better, but not perfect. This -[rwxd...] stuff is for the birds anyway.
 * We should be using Lisp syntax, functions like isdir(name), or something
* equally clear instead of all of this ambiuous junk.
*/
		if (isa(**vp, RESTOP))
			bferr(MSGSTR(M_NOFNAME, "Missing file name"));
		dp = *(*vp)++;
		if (ignore&IGNORE)
			return ((uchar_t *)"");
		ep = globone(dp);
		switch (cp[1]) {

		case 'r':
			i = !access(ep, R_OK);
			break;

		case 'w':
			i = !access(ep, W_OK);
			break;

		case 'x':
			i = !access(ep, X_OK);
			break;

		default:
                        if (cp[1] == 'l') {	/*BJB001*/
                                i = lstat((char *)ep, &stb);
                        } else {
                                i = stat((char *)ep, &stb);
                        }
                        if (i) {
                                xfree(ep);
                                return ((uchar_t *)"0");
			}
			switch (cp[1]) {

			case 'f':
                                i = (stb.st_mode & S_IFMT) == S_IFREG;/*BJB001*/
				break;

			case 'd':
				i = S_ISDIR(stb.st_mode);
				break;

			case 'z':
				i = stb.st_size == 0;
				break;

			case 'e':
				i = 1;
				break;

			case 'o':
				i = stb.st_uid == uid;
				break;

			case 'l':				/* FPM001 */
				i = S_ISLNK(stb.st_mode);	/* FPM001 */
				break;				/* FPM001 */

			}
		}
#ifdef DEBUG
etraci("exp6 -? i", i, vp);
#endif
		xfree(ep);
		return (putn(i));
	}
#ifdef DEBUG
etracc("exp6 default", cp, vp);
#endif
	return (ignore&NOGLOB ? savestr(cp) : globone(cp));
}

void
evalav(uchar_t **v)
{
	struct wordent paraml;
	register struct wordent *hp = &paraml;
	struct command *t;
	register struct wordent *wdp = hp;
	
	set((uchar_t *)"status",(uchar_t *)"0");
	hp->prev = hp->next = hp;
	hp->word = (uchar_t *)"";
	while (*v) {
		register struct wordent *new;

		new  = (struct wordent *)calloc(1, sizeof *wdp);
		new->prev = wdp;
		new->next = hp;
		wdp->next = new;
		wdp = new;
		wdp->word = savestr(*v++);
	}
	hp->prev = wdp;
	alias(&paraml);
	t = syntax(paraml.next, &paraml, 0);
	if (err)
		error((char *)err);
	execute(t, -1, (int *)NULL, (int *)NULL);
	freelex(&paraml), freesyn(t);
}

long                                                          /* 002 */
isa(register uchar_t *cp, register int what)
{

	if (cp == 0)
		return ((what & RESTOP) != 0);
	if (cp[1] == 0) {
		if (what & ADDOP && (*cp == '+' || *cp == '-'))
			return (1);
		if (what & MULOP && (*cp == '*' || *cp == '/' || *cp == '%'))
			return (1);
		if (what & RESTOP && (*cp == '(' || *cp == ')' || *cp == '!' ||
				      *cp == '~' || *cp == '^' || *cp == '"'))
			return (1);
	} else if (cp[2] == 0) {
		if (what & RESTOP) {
			if (cp[0] == '|' && cp[1] == '&')
				return (1);
			if (cp[0] == '<' && cp[1] == '<')
				return (1);
			if (cp[0] == '>' && cp[1] == '>')
				return (1);
		}
		if (what & EQOP) {
			if (cp[0] == '=') {
				if (cp[1] == '=')
					return (EQEQ);
				if (cp[1] == '~')
					return (EQMATCH);
			} else if (cp[0] == '!') {
				if (cp[1] == '=')
					return (NOTEQ);
				if (cp[1] == '~')
					return (NOTEQMATCH);
			}
		}
	}
	if (what & RELOP) {
		if (*cp == '<')
			return (LSS);
		if (*cp == '>')
			return (GTR);
	}
	return (0);
}

long                                                            /* 002 */
egetn(uchar_t *cp)
{
	if (*cp && *cp != '-' && !digit((wint_t)*cp))
		bferr(MSGSTR(M_EXPR, "Expression syntax"));
	return (getn(cp));
}

#ifdef DEBUG
etraci(uchar_t *str, int i, uchar_t ***vp)
{
	csh_printf("%s=%d\t", str, i);
	blkpr(*vp);
	csh_printf("\n");
}

etracc(uchar_t *str, uchar_t *cp, uchar_t ***vp)
{
	csh_printf("%s=%s\t", str, cp);
	blkpr(*vp);
	csh_printf("\n");
}
#endif
