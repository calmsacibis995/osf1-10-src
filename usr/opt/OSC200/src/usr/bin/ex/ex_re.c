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
static char rcsid[] = "@(#)$RCSfile: ex_re.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 20:12:28 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_re.c
 *
 * FUNCTION: advance, cclass, cerror, compile, comprhs, compsub, confirmed,
 * dosub, dosubcon, execute, fixcase, getsub, global, inrange, place,
 * re_classcode, same, snote, substitute, ugo
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.12  com/cmd/edit/vi/ex_re.c, , bos320, 9134320 8/11/91 12:29:29
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */

#include "ex.h"
#include "ex_re.h"

static void cerror(char *);
static wctype_t chandle;

#ifndef _NO_PROTO
wchar_t colval(wchar_t c);
#endif
/*
 * Global, substitute and regular expressions.
 * Very similar to ed, with some re extensions and
 * confirmed substitute.
 */
void global(short k)
{
	register wchar_t *gp;
	register int c;
	register line *a1;
	wchar_t globuf[GBSIZE];
	char *Cwas;
	int nlines = lineDOL();
	int oinglobal = inglobal;
	wchar_t *oglobp = globp;
	Cwas = Command;
	/*
	 * States of inglobal:
	 *  0: ordinary - not in a global command.
	 *  1: text coming from some buffer, not tty.
	 *  2: like 1, but the source of the buffer is a global command.
	 * Hence you're only in a global command if inglobal==2. This
	 * strange sounding convention is historically derived from
	 * everybody simulating a global command.
	 */
	if (inglobal==2)
		error(MSGSTR(M_111, "Global within global@not allowed"), DUMMY_INT);
	markDOT();
	setall();
	nonzero();
	if (skipend())
		error(MSGSTR(M_112, "Global needs re|Missing regular expression for global"), DUMMY_INT);
	c = ex_getchar();
	ignore(compile(c, 1));
	savere(scanre);
	gp = globuf;
	while ((c = ex_getchar()) != '\n') {
		switch (c) {

		case EOF:
			c = '\n';
			goto brkwh;

		case '\\':
			c = ex_getchar();
			switch (c) {

			case '\\':
				ungetchar(c);
				break;

			case '\n':
				break;

			default:
				*gp++ = '\\';
				break;
			}
			break;
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE])
			error(MSGSTR(M_113, "Global command too long"), DUMMY_INT);
	}
brkwh:
	ungetchar(c);
	donewline();
	*gp++ = c;
	*gp++ = 0;
	saveall();
	inglobal = 2;
	for (a1 = one; a1 <= dol; a1++) {
		*a1 &= ~01;
		if (a1 >= addr1 && a1 <= addr2 && execute(0, a1) == k)
			*a1 |= 01;
	}
	if (inopen)
		inopen = -1;
	/*
	 * Now for each marked line, set dot there and do the commands.
	 * Note the n^2 behavior here for lots of lines matching.
	 * This is really needed: in some cases you could delete lines,
	 * causing a marked line to be moved before a1 and missed if
	 * we didn't restart at zero each time.
	 */
	for (a1 = one; a1 <= dol; a1++) {
		if (*a1 & 01) {
			*a1 &= ~01;
			dot = a1;
			globp = globuf;
			commands((short)1, (short)1);
			a1 = zero;
		}
	}
	globp = oglobp;
	inglobal = oinglobal;
	endline = 1;
	Command = Cwas;
	netchHAD(nlines);
	setlastchar(EOF);
	if (inopen) {
		ungetchar(EOF);
		inopen = 1;
	}
}

short	cflag;
int	scount, slines, stotal;

int substitute(int c)
{
	register line *addr;
	register int n;
	int gsubf, hopcount;
	int olistf = listf;

	gsubf = compsub(c);

	/* 
	 * This is a hack to handle the case where a substitute
	 * command, on the echo line (i.e, in visual mode, with a ":"
	 * command), ends with a list-mode command, a la
	 * "s/pat1/pat2/l".  We identify this condition by noting that
	 * we are in open mode (inopen == 1) and when compsub() causes
	 * listf to be incremented by 1 (via a call to donewline()).
	 * Normally, the list mode would have been caught in setflav()
	 * and setlist() would have been called.  But this won't
	 * happen in visual mode, so we need to force setlist() here.
	 */
	if ((inopen) && (olistf == listf-1)) {
		setlist((int)1);
	}

	if(FIXUNDO)
		save12(), undkind = UNDCHANGE;
	stotal = 0;
	slines = 0;
	for (addr = addr1; addr <= addr2; addr++) {
		scount = hopcount = 0;
		if (dosubcon((short)0, addr) == 0)
			continue;
		if (gsubf) {
			/*
			 * The loop can happen from s/\</&/g
			 * but we don't want to break other, reasonable cases.
			 */
			hopcount = 0;
			while (*loc2) {
				if (++hopcount > sizeof linebuf)
					error(MSGSTR(M_114, "substitution loop"), DUMMY_INT);
				if (dosubcon((short)1, addr) == 0)
					break;
			}
		}
		if (scount) {
			stotal += scount;
			slines++;
			putmark(addr);
			n = append(getsub, addr);
			addr += n;
			addr2 += n;
		}
	}
	if (stotal == 0 && !inglobal && !cflag)
		error(MSGSTR(M_115, "Fail|Substitute pattern match failed"), DUMMY_INT);
	snote(stotal, slines);
	return (stotal);
}

int compsub(int ch)
{
	register int seof, c, uselastre;
	static int gsubf;

	if (!value(EDCOMPATIBLE))
		gsubf = cflag = 0;
	uselastre = 0;
	switch (ch) {

	case 's':
		ignore(skipwh());
		seof = ex_getchar();
		if (endcmd(seof) || any(seof, "gcr")) {
			ungetchar(seof);
			goto redo;
		}
		if (iswalnum(seof))
			error(MSGSTR(M_116, "Substitute needs re|Missing regular expression for substitute"), DUMMY_INT);
		seof = compile(seof, 1);
		uselastre = 1;
		comprhs(seof);
		if (!value(EDCOMPATIBLE))
			gsubf = cflag = 0;
		break;

	case '~':
		uselastre = 1;
		/* fall into ... */
	case '&':
	redo:
		if (re.Expbuf[0] == 0)
			error(MSGSTR(M_117, "No previous re|No previous regular expression"), DUMMY_INT);
		if (subre.Expbuf[0] == 0)
			error(MSGSTR(M_118, "No previous substitute re|No previous substitute to repeat"), DUMMY_INT);
		break;
	}
	for (;;) {
		c = ex_getchar();
		switch (c) {

		case 'g':
			gsubf = !gsubf;
			continue;

		case 'c':
			cflag = !cflag;
			continue;

		case 'r':
			uselastre = 1;
			continue;

		default:
			ungetchar(c);
			setcount();
			donewline();
			if (uselastre)
				savere(subre);
			else
				resre(subre);
			return (gsubf);
		}
	}
}

void comprhs(int seof)
{
	register wchar_t *rp, *orp;
	register int c;
	wchar_t orhsbuf[RHSSIZE + 1];

	rp = rhsbuf;
	CP(orhsbuf, rp);
	for (;;) {
		c = ex_getchar();
		if (c == seof)
			break;
		switch (c) {

		case '\\':
			c = ex_getchar();
			if (c == EOF) {
				ungetchar(c);
				break;
			}
			if (value(MAGIC)) {
				/*
				 * When "magic", \& turns into a plain &,
				 * and all other chars work fine quoted.
				 */
				if (c != '&')
					*rp++ = '\\';
				break;
			}
magic:
                        if ((c == '%') && (value(EDCOMPATIBLE))) {
                                for (orp = orhsbuf; *orp; *rp++ = *orp++)
                                        if (rp > &rhsbuf[RHSSIZE + 1])
                                                goto toobig;
                                continue;
                        }
			if (c == '~') {
				for (orp = orhsbuf; *orp; *rp++ = *orp++)
					if (rp > &rhsbuf[RHSSIZE + 1])
						goto toobig;
				continue;
			}
			*rp++ = '\\';
			break;

		case '\n':
		case EOF:
			if (!(globp && globp[0])) {
				ungetchar(c);
				goto endrhs;
			}

		case '~':
		case '%':
		case '&':
			if (value(MAGIC))
				goto magic;
			break;
		}
		if (rp > &rhsbuf[RHSSIZE + 1]) {
toobig:
			*rp = 0;
			error(MSGSTR(M_119, "Replacement pattern too long@- limit 256 characters"), DUMMY_INT);
		}
		*rp++ = c;
	}
endrhs:
	*rp++ = 0;
}

int getsub(void)
{
	register wchar_t *p;

	if ((p = linebp) == 0)
		return (EOF);
	strcLIN(p);
	linebp = 0;
	return (0);
}

int dosubcon(short f, line *a)
{

	if (execute(f, a) == 0)
		return (0);
	if (confirmed(a)) {  
		dosub();
		scount++;
	}
	return (1);
}

#define YNSIZ	32
int confirmed(line *a)
{
	register int c;
	char ynbuf[YNSIZ];
	char *ynptr;

	if (cflag == 0)
		return (1);
	pofix();
	pline(lineno(a));
	if (inopen) {
		ex_putchar(QUOTE_NL);
	}
	c = column(loc1 - 1);
	ugo(c - 1 + (inopen ? 1 : 0), ' ');
	ugo(column(loc2 - 1) - c, '^');
	flush();
	ynptr = ynbuf;
	while ((c = getkey()) != EOF && c != '\n' && c != '\r') {
		if (ynptr < &ynbuf[YNSIZ-2]) {
			ynptr += wctomb(ynptr, (wchar_t)c);
			if (inopen) {
				ex_putchar(c);
				flush();
			}
		}
	}
	if (c == '\r')
		c = '\n';
	if (inopen) {
		ex_putchar(c);
		flush();
	}
	*ynptr = '\0';
	noteinp();
	return (rpmatch(ynbuf) == 1);
}
void ugo(int cnt, int with)
{

	if (cnt > 0)
		do
			ex_putchar(with);
		while (--cnt > 0);
}

int	casecnt;
short	destuc;

void dosub(void)
{
	register wchar_t *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	casecnt = 0;
	while (c = *rp++) {
		/* ^V <return> from vi to split lines */
		if (c == '\r')
			c = '\n';

		if (c == '\\') {
			c = *rp++;
			switch (c) {

			case '&':
				sp = place(sp, loc1, loc2);
				if (sp == 0)
					goto ovflo;
				continue;

			case 'l':
				casecnt = 1;
				destuc = 0;
				continue;

			case 'L':
				casecnt = LBSIZE;
				destuc = 0;
				continue;

			case 'u':
				casecnt = 1;
				destuc = 1;
				continue;

			case 'U':
				casecnt = LBSIZE;
				destuc = 1;
				continue;

			case 'E':
			case 'e':
				casecnt = 0;
				continue;
			}
			if (c >= '1' && c < nbra + '1') {
				sp = place(sp, braslist[c - '1'], braelist[c - '1']);
				if (sp == 0)
					goto ovflo;
				continue;
			}
		}
		if (casecnt)
			c = fixcase(c);
		*sp++ = c;

		if (sp > &genbuf[LBSIZE])
ovflo:
			error(MSGSTR(M_120, "The substitution creates a line that is too long.@ The limit is %d characters."), LBSIZE);
	}
	lp = loc2;
	loc2 = sp + (linebuf - genbuf);
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE])
			goto ovflo;
	strcLIN(genbuf);
}

int fixcase(register int c)
{

	if (casecnt == 0)
		return (c);
	casecnt--;
	if (destuc) {
		if (iswlower(c))
			c = towupper(c);
	} else
		if (iswupper(c))
			c = towlower(c);
	return (c);
}

wchar_t *place(register wchar_t *sp, register wchar_t *l1, register wchar_t *l2)
{

	while (l1 < l2) {
		*sp++ = fixcase(*l1++);
		if (sp > &genbuf[LBSIZE])
			return (0);
	}
	return (sp);
}

void snote(register int total, register int nlines)
{

	if (!notable(total))
		return;
	ex_printf(mesg(MSGSTR(M_121, "%d subs|%d substitutions")), total);
	if (nlines != 1 && nlines != total)
		ex_printf(MSGSTR(M_122, " on %d lines"), nlines);
	noonl();
	flush();
}

static wchar_t re_classcode(wchar_t *id)
{

        char mbbuf[64];

        if (wcstombs(mbbuf, id, 64) == -1)
                error(MSGSTR(M_651, "Invalid wide character string, conversion failed."), DUMMY_INT);
        chandle = wctype(mbbuf);
        if (chandle == (wctype_t) -1)
        {
                error(MSGSTR(M_670, "Property invalid for locale."), DUMMY_INT);
                return (CL_BADCLASS);
        }
        return (CL_GOODCLASS);

}

int compile(int eof, int oknl)
{
	register int c;
	register wchar_t *ep;
	wchar_t *lastep;
	wchar_t bracket[NBRA], *bracketp, *rhsp;
	wchar_t *lastcp, *lastkp;

	if (iswalnum(eof))
		error(MSGSTR(M_123, "Regular expressions cannot be delimited by letters or digits"), DUMMY_INT);
	ep = expbuf;
	c = ex_getchar();
	if (eof == '\\')
		switch (c) {

		case '/':
		case '?':
			if (scanre.Expbuf[0] == 0)
error(MSGSTR(M_124, "No previous scan re|No previous scanning regular expression"), DUMMY_INT);
			resre(scanre);
			return (c);

		case '&':
			if (subre.Expbuf[0] == 0)
error(MSGSTR(M_118, "No previous substitute re|No previous substitute regular expression"), DUMMY_INT);
			resre(subre);
			return (c);

		default:
			error(MSGSTR(M_126, "Badly formed re|Regular expression \\ must be followed by / or ?"), DUMMY_INT);
		}
	if (c == eof || c == '\n' || c == EOF) {
		if (*ep == 0)
			error(MSGSTR(M_117, "No previous re|No previous regular expression"), DUMMY_INT);
		if (c == '\n' && oknl == 0)
			error(MSGSTR(M_128, "Missing closing delimiter@for regular expression"), DUMMY_INT);
		if (c != eof)
			ungetchar(c);
		return (eof);
	}
	bracketp = bracket;
	nbra = 0;
	circfl = 0;
	if (c == '^') {
		c = ex_getchar();
		circfl++;
	}
	ungetchar(c);
	for (;;) {
		if (ep >= &expbuf[ESIZE - 2])
complex:
			cerror(MSGSTR(M_129, "Re too complex|Regular expression too complicated"));
		c = ex_getchar();
		if (c == eof || c == EOF) {
			if (bracketp != bracket)
cerror(MSGSTR(M_130, "Unmatched \\(|More \\('s than \\)'s in regular expression"));
			*ep++ = CEOFC;
			if (c == EOF)
				ungetchar(c);
			return (eof);
		}
		if (value(MAGIC)) {
			if (c != '*' || ep == expbuf)
				lastep = ep;
		} else
			if (c != '\\' || peekchar() != '*' || ep == (wchar_t *)expbuf)
				lastep = ep;
		switch (c) {

		case '\\':
			c = ex_getchar();
			switch (c) {

			case '(':
				if (nbra >= NBRA) {
cerror(MSGSTR(M_131, "Awash in \\('s!|Too many \\('d subexressions in a regular expression"));
				}
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;

			case ')':
				if (bracketp <= bracket)
cerror(MSGSTR(M_132, "Extra \\)|More \\)'s than \\('s in regular expression"));
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;

			case '<':
				*ep++ = CBRC;
				continue;

			case '>':
				*ep++ = CLET;
				continue;
			}
			if (value(MAGIC) == 0)
magic:
			switch (c) {

			case '.':
				*ep++ = CDOT;
				continue;

			case '~':
				rhsp = rhsbuf;
				while (*rhsp) {
					c = *rhsp++;
					if (c == '\\') {
						c = *rhsp++;
						if (c == '&')
error(MSGSTR(M_133, "Replacement pattern contains &@- cannot use in re"), DUMMY_INT);
						if (c >= '1' && c <= '9')
error(MSGSTR(M_134, "Replacement pattern contains \\d@- cannot use in re"), DUMMY_INT);
					}
					if (ep >= &expbuf[ESIZE-2])
						goto complex;
					*ep++ = CCHR;
					*ep++ = c;
				}
				continue;

			case '*':
				if (ep == expbuf)
					break;
				if (*lastep == CBRA || *lastep == CKET)
cerror(MSGSTR(M_135, "Illegal *|Can't * a \\( ... \\) in regular expression"));
				*lastep |= STAR;
				continue;

			case '[':
				if (ep >= &expbuf[ESIZE-2]) goto complex;
				*ep++ = CCL;
				*ep++ = 0;
				lastcp = lastkp = NULL;
				c = ex_getchar();
				if (c == '^') {
					c = ex_getchar();
					ep[-2] = NCCL;
				}
				if (c == ']')
cerror(MSGSTR(M_137, "Bad character class|Empty character class '[]' or '[^]' cannot match"));
				while (c != ']') {
					if (c == '-' && lastcp != NULL && peekchar() != ']') {
						c = *lastcp;
						ep = lastcp;
						if (ep >= &expbuf[ESIZE-4]) goto complex;
						*ep++ = '\\';
						*ep++ = CL_RANGE;
						*ep++ = c;
						if ((c = ex_getchar()) == '\\' && any(peekchar(), "]-^\\")) c = ex_getchar();
						*ep++ = c;
						lastcp = NULL;
					} else {
						if (c == '\\' && any(peekchar(), "]-^\\")) c = ex_getchar();
						if (c == '\n' || c == EOF)
							cerror(MSGSTR(M_138, "Missing ]"));
						lastcp = ep;
						if (c == ':' && lastkp != NULL && peekchar() == ']') {
							*ep = '\0';
							if ((c = re_classcode(lastkp)) == CL_BADCLASS)
cerror(MSGSTR(M_252, "Unrecognized ctype@class in [: ... :] construct"));
							(void)ex_getchar();
							ep = lastkp - 2;
							*ep++ = '\\';
							lastcp = lastkp = NULL;
						}
						if (ep >= &expbuf[ESIZE-2]) goto complex;
						*ep++ = c;
						if (c == '\\') *ep++ = CL_BACKSLASH;
						if (c == '[' && peekchar() == ':') {
							lastcp = ep;
							*ep++ = ex_getchar();
							lastkp = ep;
						}
					}
					c = ex_getchar();
				}
				lastep[1] = ep - &lastep[1];
				continue;
			}
			if (c == EOF) {
				ungetchar(EOF);
				c = '\\';
				goto defchar;
			}
			*ep++ = CCHR;
			if (c == '\n')
cerror(MSGSTR(M_139, "No newlines in re's|Can't escape newlines into regular expressions"));
			*ep++ = c;
			continue;

		case '\n':
			if (oknl) {
				ungetchar(c);
				*ep++ = CEOFC;
				return (eof);
			}
cerror(MSGSTR(M_140, "Badly formed re|Missing closing delimiter for regular expression"));

		case '$':
			if (peekchar() == eof || peekchar() == EOF || oknl && peekchar() == '\n') {
				*ep++ = CDOL;
				continue;
			}
			goto defchar;

		case '.':
		case '~':
		case '*':
		case '[':
			if (value(MAGIC))
				goto magic;
defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
			continue;
		}
	}
}

static void cerror(char *s)
{

	expbuf[0] = 0;
	error(s, DUMMY_INT);
}

int same(register int a, register int b)
{

	return (a == b || value(IGNORECASE) &&
	   ((iswlower(a) && towupper(a) == b) || (iswlower(b) && towupper(b) == a)));
}

wchar_t	*locs;

int execute(int gf, line *addr)
{
	register wchar_t *p1, *p2;
	register int c;

	if (gf) {
		if (circfl)
			return (0);
		locs = p1 = loc2;
	} else {
		if (addr == zero)
			return (0);
		p1 = linebuf;
		getline(*addr);
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		return (advance(p1, p2));
	}
	/* fast check for first character */
	if (*p2 == CCHR) {
		c = p2[1];
		do {
			if (c != *p1 && (!value(IGNORECASE) ||
			   !((iswlower(c) && towupper(c) == *p1) ||
			   (iswlower(*p1) && towupper(*p1) == c))))
				continue;
			if (advance(p1, p2)) {
				loc1 = p1;
				return (1);
			}
		} while (*p1++);
		return (0);
	}
	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return (1);
		}
	} while (*p1++);
	return (0);
}


int advance(wchar_t *lp, wchar_t *ep)
{
	register wchar_t *curlp;

	for (;;) switch (*ep++) {

	case CCHR:
		if (!same(*ep, *lp))
			return (0);
		ep++, lp++;
		continue;

	case CDOT:
		if (*lp++)
			continue;
		return (0);

	case CDOL:
		if (*lp == 0)
			continue;
		return (0);

	case CEOFC:
		loc2 = lp;
		return (1);

	case CCL:
		if (cclass(ep, &lp, 1)) {
			ep += *ep;
			continue;
		}
		return (0);

	case NCCL:
		if (cclass(ep, &lp, 0)) {
			ep += *ep;
			continue;
		}
		return (0);

	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CDOT|STAR:
		curlp = lp;
		while (*lp++)
			continue;
		goto star;

	case CCHR|STAR:
		curlp = lp;
		while (same(*lp, *ep))
			lp++;
		lp++;
		ep++;
		goto star;

	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (cclass(ep, &lp, ep[-1] == (CCL|STAR)))
			continue;
		ep += *ep;
		goto star;
star:
		do {
			lp--;
			if (lp == locs)
				break;
			if (advance(lp, ep))
				return (1);
		} while (lp > curlp);
		return (0);

	case CBRC:
		if (lp == linebuf)
			continue;
		if (!wordch(&lp[-1]) && wordch(lp))
			continue;
		return (0);

	case CLET:
		if (!wordch(lp))
			continue;
		return (0);

	default:
		error(MSGSTR(M_141, "Re internal error"), DUMMY_INT);
	}
}

static int
inrange(wchar_t c, wchar_t lo, wchar_t hi) 
{
        int cc;
        wchar_t tc;
        wchar_t ulo = colval(lo);   /* get colval for boundary */
        wchar_t uhi = colval(hi);
        wchar_t uc = colval(c);      /* get unique for subject character */
 
	/* check if outside range and ignorecase is set */
	if (value(IGNORECASE) && (uc < ulo || uc > uhi)) {
            if ((tc = towupper(c)) == c)
                tc = towlower(c);
                    uc = colval(tc);
        }
        cc = (uc >= ulo && uc <= uhi);
        return (cc);
}

wchar_t colval(wchar_t c)
{
	wchar_t cvalue[3], colvalue[3];

        cvalue[0] = c;
        cvalue[1] = 0;
        if(wcsxfrm(colvalue, cvalue, 3) >= 3){
                error(MSGSTR(M_671, "Error in character collation value"), DUMMY_INT);
        }
    return (colvalue[0]);
}

int cclass(register wchar_t *set, wchar_t **s, int af)
{
        register wchar_t c;
        register wchar_t *endset;
        wchar_t *t = *s;

        c = *t++;
        *s = t;
	if (c == 0)
		return (0);
	endset = set;
	endset += *set++;
	while (set < endset) {
		if (*set != '\\') {
			if (c == *set++) return (af);
		} else {
			++set;
			switch (*set++) {
			case CL_BACKSLASH:
				if (c == '\\') return (af);
				break;
			case CL_RANGE:
                                if (inrange(c, set[0], set[1])) {
                                        *s = t;
                                        return (af);
                                }
				set += 2;
				break;
			case CL_GOODCLASS:
				if (iswctype(c, chandle))
					return (af);
				break;
			default:
				error(MSGSTR(M_253, "Internal regexp error in cclass"), DUMMY_INT);
			}
		}
	}
	return (!af);
}
