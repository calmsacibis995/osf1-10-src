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
static char rcsid[] = "@(#)$RCSfile: ex_cmdsub.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/08/03 11:20:26 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_cmdsub.c
 *
 * FUNCTION: addmac, append, appendnone, cmdmac, delete, deletenone, getcopy,
 * getput, jnoop, join, mapcmd, move, move1, pargs, plines, pofix, pragged,
 * put, shift, somechange, splitit, squish, tagfind, undo, yank, zop, zop2
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.13  com/cmd/edit/vi/ex_cmdsub.c, , bos320, 9134320 8/11/91 12:28:44
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */

#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"
#include "ex_vis.h"

/*
 * True if c is a legal "word" character  (001 gray)
 */
#define	WORDCHAR(c)	(iswalnum(c) || c == L'_')

/*
 * Command mode subroutines implementing
 *	append, args, copy, delete, join, move, put,
 *	shift, tag, yank, z and undo
 */

short	endline = 1;
line	*tad1;
static	int jnoop(void);
static  void splitit(void);

void    addmac(register wchar_t *, register wchar_t *, register wchar_t *, register struct maps *);

/*
 * Append after line a lines returned by function f.
 * Be careful about intermediate states to avoid scramble
 * if an interrupt comes in.
 */
int
append(int (*f)(void), line *a)
{
	register line *a1, *a2, *rdot;
	int nline;

	nline = 0;
	dot = a;
	if(FIXUNDO && !inopen && f!=getsub) {
		undap1 = undap2 = dot + 1;
		undkind = UNDCHANGE;
	}
	while ((*f)() == 0) {
		if (truedol + 1 >= endcore) {
			if (morelines() < 0) {
				if (FIXUNDO && f == getsub) {
					undap1 = addr1;
					undap2 = addr2 + 1;
				}
				error(MSGSTR(M_036, "Out of memory@- too many lines in file"), DUMMY_INT);
			}
		}
		nline++;
		a1 = truedol + 1;
		a2 = a1 + 1;
		dot++;
		undap2++;
		dol++;
		unddol++;
		truedol++;
		for (rdot = dot; a1 > rdot;)
			*--a2 = *--a1;
		*rdot = 0;
		putmark(rdot);
		if (f == gettty) {
			dirtcnt++;
			TSYNC();
		}
	}
	return (nline);
}

void
appendnone(void)
{

	if(FIXUNDO) {
		undkind = UNDCHANGE;
		undap1 = undap2 = addr1;
	}
}

/*
 * Print out the argument list, with []'s around the current name.
 */
void
pargs(void)
{
	register char **av = argv0, *as = args0;
	register int ac;

	for (ac = 0; ac < argc0; ac++) {
		if (ac != 0)
			ex_putchar(' ');
		if (ac + argc == argc0 - 1)
			ex_printf("[");
		lprintf("%s", as);
		if (ac + argc == argc0 - 1)
			ex_printf("]");
		as = av ? *++av : strend(as) + 1;
	}
	noonl();
}

/*
 * Delete lines; two cases are if we are really deleting,
 * more commonly we are just moving lines to the undo save area.
 */
void
delete(short hush)
{
	register line *a1, *a2;

	nonzero();
	if(FIXUNDO) {
		void (*dsavint)(int);

#ifdef UNDOTRACE
		if (trace)
			vudump("before delete");
#endif
		change();
		dsavint = signal(SIGINT, SIG_IGN);
		undkind = UNDCHANGE;
		a1 = addr1;
		squish();
		a2 = addr2;
		if (a2++ != dol) {
			reverse(a1, a2);
			reverse(a2, dol + 1);
			reverse(a1, dol + 1);
		}
		dol -= a2 - a1;
		unddel = a1 - 1;
		if (a1 > dol)
			a1 = dol;
		dot = a1;
		pkill[0] = pkill[1] = 0;
		signal(SIGINT, dsavint);
#ifdef UNDOTRACE
		if (trace)
			vudump("after delete");
#endif
	} else {
		register line *a3;
		register int i;

		change();
		a1 = addr1;
		a2 = addr2 + 1;
		a3 = truedol;
		i = a2 - a1;
		unddol -= i;
		undap2 -= i;
		dol -= i;
		truedol -= i;
		do
			*a1++ = *a2++;
		while (a2 <= a3);
		a1 = addr1;
		if (a1 > dol)
			a1 = dol;
		dot = a1;
	}
	if (!hush)
		killed();
}

void
deletenone(void)
{

	if(FIXUNDO) {
		undkind = UNDCHANGE;
		squish();
		unddel = addr1;
	}
}

/*
 * Crush out the undo save area, moving the open/visual
 * save area down in its place.
 */
void
squish(void)
{
	register line *a1 = dol + 1, *a2 = unddol + 1, *a3 = truedol + 1;

	if(FIXUNDO) {
		if (inopen == -1)
			return;
		if (a1 < a2 && a2 < a3)
			do
				*a1++ = *a2++;
			while (a2 < a3);
		truedol -= unddol - dol;
		unddol = dol;
	}
}

/*
 * Join lines.	Special mods put in spaces, two spaces if
 * preceding line ends with '.', or no spaces if next line starts with ).
 */
static	int jcount;

void
join(int c)
{
	register line *a1;
	register wchar_t *cp, *cp1;

	cp = genbuf;
	*cp = 0;
	for (a1 = addr1; a1 <= addr2; a1++) {
		getline(*a1);
		cp1 = linebuf;
		if (a1 != addr1 && c == 0) {
			while (*cp1 == ' ' || *cp1 == '\t')
				cp1++;
			if (*cp1 && cp > genbuf && cp[-1] != ' ' && cp[-1] != '\t') {
				if (*cp1 != ')') {
					*cp++ = ' ';
					if (cp[-2] == '.')
						*cp++ = ' ';
				}
			}
		}
		while (*cp++ = *cp1++)
			if (cp > &genbuf[LBSIZE])
				error(MSGSTR(M_037, "Line too long.| The result of a line join would be greater than %d characters."), LBSIZE);
		cp--;
	}
	strcLIN(genbuf);
	delete((short)0);
	jcount = 1;
	if (FIXUNDO)
		undap1 = undap2 = addr1;
	ignore(append(jnoop, --addr1));
	if (FIXUNDO)
		vundkind = VMANY;
}

static int
jnoop(void)
{

	return(--jcount);
}

/*
 * Move and copy lines.  Hard work is done by move1 which
 * is also called by undo.
 */

void
move(void)
{
	register line *adt;
	short iscopy = 0;

	if (Command[0] == 'm') {
		setdot1();
		markpr(addr2 == dol ? addr1 - 1 : addr2 + 1);
	} else {
		iscopy++;
		setdot();
	}
	nonzero();
	adt = address((wchar_t *)0);
	if (adt == 0)
		serror(MSGSTR(M_038, "%s where?|%s requires a trailing address"), Command);
	donewline();
	move1(iscopy, adt);
	killed();
}

void
move1(int cflag, line *addrt)
{
	register line *adt, *ad1, *ad2;
	int nlines;

	adt = addrt;
	nlines = (addr2 - addr1) + 1;
	if (cflag) {
		tad1 = addr1;
		ad1 = dol;
		ignore(append(getcopy, ad1++));
		ad2 = dol;
	} else {
		ad2 = addr2;
		for (ad1 = addr1; ad1 <= ad2;)
			*ad1++ &= ~01;
		ad1 = addr1;
	}
	ad2++;
	if (adt < ad1) {
		if (adt + 1 == ad1 && !cflag && !inglobal)
			error(MSGSTR(M_039, "That move would do nothing!"), DUMMY_INT);
		dot = adt + (ad2 - ad1);
		if (++adt != ad1) {
			reverse(adt, ad1);
			reverse(ad1, ad2);
			reverse(adt, ad2);
		}
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		error(MSGSTR(M_040, "Move to a moved line"), DUMMY_INT);
	change();
	if (!inglobal)
		if(FIXUNDO) {
			if (cflag) {
				undap1 = addrt + 1;
				undap2 = undap1 + nlines;
				deletenone();
			} else {
				undkind = UNDMOVE;
				undap1 = addr1;
				undap2 = addr2;
				unddel = addrt;
				squish();
			}
		}
}

int
getcopy(void)
{

	if (tad1 > addr2)
		return (EOF);
	getline(*tad1++);
	return (0);
}

/*
 * Put lines in the buffer from the undo save area.
 */
int
getput(void)
{

	if (tad1 > unddol)
		return (EOF);
	getline(*tad1++);
	tad1++;
	return (0);
}

void
put(void)
{
	register int cnt;

	if (!FIXUNDO)
		error(MSGSTR(M_041, "Cannot put inside global/macro"), DUMMY_INT);
	cnt = unddol - dol;
	if (cnt && inopen && pkill[0] && pkill[1]) {
		pragged((short)1);
		return;
	}
	tad1 = dol + 1;
	ignore(append(getput, addr2));
	undkind = UNDPUT;
	notecnt = cnt;
	netchange(cnt);
}

/*
 * A tricky put, of a group of lines in the middle
 * of an existing line.  Only from open/visual.
 * Argument says pkills have meaning, e.g. called from
 * put; it is 0 on calls from putreg.
 */
void
pragged(short kill)
{
	extern wchar_t *cursor;
	register wchar_t *gp = &genbuf[cursor - linebuf];

	/*
	 * This kind of stuff is TECO's forte.
	 * We just grunge along, since it cuts
	 * across our line-oriented model of the world
	 * almost scrambling our addled brain.
	 */
	if (!kill)
		getDOT();
	wcscpy(genbuf, linebuf);
	getline(*unddol);
	if (kill)
		*pkill[1] = 0;
	wcscat(linebuf, gp);
	putmark(unddol);
	getline(dol[1]);
	if (kill)
		strcLIN(pkill[0]);
	wcscpy(gp, linebuf);
	strcLIN(genbuf);
	putmark(dol+1);
	undkind = UNDCHANGE;
	undap1 = dot;
	undap2 = dot + 1;
	unddel = dot - 1;
	undo((short)1);
}

/*
 * Shift lines, based on c.
 * If c is neither < nor >, then this is a lisp aligning =.
 */
void
shift(int c, int cnt)
{
	register line *addr;
	register wchar_t *cp;
	wchar_t *dp;
	register int i;

	if(FIXUNDO)
		save12(), undkind = UNDCHANGE;
	cnt *= value(SHIFTWIDTH);
	for (addr = addr1; addr <= addr2; addr++) {
		dot = addr;
		if (c == '=' && addr == addr1 && addr != addr2)
			continue;
		getDOT();
		i = whitecnt(linebuf);
		switch (c) {

		case '>':
			if (linebuf[0] == 0)
				continue;
			cp = genindent(i + cnt);
			break;

		case '<':
			if (i == 0)
				continue;
			i -= cnt;
			cp = i > 0 ? genindent(i) : genbuf;
			break;

		default:
			i = lindent(addr);
			getDOT();
			cp = genindent(i);
			break;
		}
		if (cp + wcslen(dp = vpastwh(linebuf)) >= &genbuf[LBSIZE - 2])
			error(MSGSTR(M_042, "Line too long.| A shift cannot create a line longer than %d characters."), LBSIZE);
		CP(cp, dp);
		strcLIN(genbuf);
		putmark(addr);
	}
	killed();
}

/*
 * Find a tag in the tags file.
 * Most work here is in parsing the tags file itself.
 */
void
tagfind(short quick)
{
	wchar_t cmdbuf[LBSIZE];
	char filebuf[FNSIZE];
	char tagfbuf[128];
	register int c, d;
	short samef = 1;
	int tfcount = 0;
	int omagic, tl;
	char *tempchar;
	char *fn, *fne;
#ifdef STDIO		/* mjm: was VMUNIX */
	/*
	 * We have lots of room so we bring in stdio and do
	 * a binary search on the tags file.
	 */
	FILE *iof;
	wchar_t iofbuf[LBSIZE];
	long mid;	/* assumed byte offset */
	long top, bot;	/* length of tag file */
	struct stat sbuf;
#endif

	omagic = value(MAGIC);
	tl = value(TAGLENGTH);
	if (!skipend()) {
		register wchar_t *lp = lasttag;
		while (!iswhite(peekchar()) && !endcmd(peekchar()))
			if (lp < &lasttag[WCSIZE (lasttag) - 2])
				*lp++ = ex_getchar();
			else
				ignchar();
		*lp++ = 0;
		if (!endcmd(peekchar()))
badtag:
			error(MSGSTR(M_043, "Bad tag|Give one tag per line"), DUMMY_INT);
	} else if (lasttag[0] == 0)
		error(MSGSTR(M_044, "No previous tag"), DUMMY_INT);
	c = ex_getchar();
	if (!endcmd(c))
		goto badtag;
	if (c == EOF)
		ungetchar(c);
	clrstats();

	/*
	 * Loop once for each file in tags "path".
	 */
	strcpy(tagfbuf, svalue(TAGS));
	fne = tagfbuf - 1;
	while (fne) {
		fn = ++fne;
		while (*fne && *fne != ' ')
			fne++;
		if (*fne == 0)
			fne = 0;	/* done, quit after this time */
		else
			*fne = 0;	/* null terminate filename */
#ifdef STDIO		/* mjm: was VMUNIX */
		iof = fopen(fn, "r");
		if (iof == NULL)
			continue;
		tfcount++;
		setbuffer(iof, (char *)iofbuf, sizeof(iofbuf));
		fstat(fileno(iof), &sbuf);
		top = sbuf.st_size;
		if (top == 0L || iof == NULL)
			top = -1L;
		bot = 0L;
		while (top >= bot) {
#else
		/*
		 * Avoid stdio and scan tag file linearly.
		 */
		io = open(fn, 0);
		if (io<0)
			continue;
		tfcount++;
		while (getfile() == 0) {
#endif
			/* loop for each tags file entry */
			register wchar_t *cp = linebuf;
			register wchar_t *lp = lasttag;
			char   *fnp;	     /* ptr to filename */
			wchar_t *oglobp;

#ifdef STDIO		/* mjm: was VMUNIX */
			mid = (top + bot) / 2;
			fseek(iof, mid, 0);
			{
				char tmpbuf[sizeof(linebuf)];

				if (mid > 0)	/* to get first tag in file to work */
					/* scan to next \n */
					if(fgets(tmpbuf, sizeof tmpbuf, iof)==NULL)
						goto goleft;
				/* get the line itself */
				if(fgets(tmpbuf, sizeof tmpbuf, iof)==NULL)
					goto goleft;
				tmpbuf[strlen(tmpbuf)-1] = 0;	/* was '\n' */
				if (mbstowcs(linebuf, tmpbuf, LBSIZE) == -1)
					error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
			}
#endif
			while (*cp && *lp == *cp)
				cp++, lp++;
			/*
			 * This if decides whether there is a tag match.
			 *  A positive taglength means that a
			 *  match is found if the tag given matches at least
			 *  taglength chars of the tag found.
			 *  A taglength of greater than 511 means that a
			 *  match is found even if the tag given is a proper
			 *  prefix of the tag found.  i.e. "ab" matches "abcd"
			 */
#ifdef STDIO		/* mjm: was VMUNIX */
			if ( *lp == 0 && (iswhite(*cp) || tl > 511 || tl > 0 && lp-lasttag >= tl) ) {
				/*
				 * Found a match.  Force selection to be
				 *  the first possible.
				 */
				if ( mid == bot  &&  mid == top ) {
					; /* found first possible match */
				}
				else {
					/* postpone final decision. */
					top = mid;
					continue;
				}
			}
			else {
				if (*lp > *cp)
					bot = mid + 1;
				else
goleft:
					top = mid - 1;
				continue;
			}
#else
			if ( *lp == 0 && (iswhite(*cp) || tl > 511 || tl > 0 && lp-lasttag >= tl) ) {
				; /* Found it. */
			}
			else {
				/* Not this tag.  Try the next */
				continue;
			}
#endif
			/*
			 * We found the tag.  Decode the line in the file.
			 */
#ifdef STDIO		/* mjm: was VMUNIX */
			fclose(iof);
#else
			close(io);
#endif
			/* Rest of tag if abbreviated */
			while (!iswhite(*cp))
				cp++;

			/* name of file */
			while (*cp && iswhite(*cp))
				cp++;
			if (!*cp)
badtags:
				serror(MSGSTR(M_045, "%S: Bad tags file entry"), (char *)lasttag);
			fnp = filebuf;
			while (*cp && *cp != ' ' && *cp != '\t') {
				if (fnp < &filebuf[sizeof filebuf - 2])
				    /* put a wide char into 1 or 2 bytes and */
				    /* increments fnp once or twice */
				    fnp += wctomb(fnp,*cp);
				cp++;
			}
			*fnp++ = 0;
			if (*cp == 0)
				goto badtags;
			if (dol != zero) {
				/*
				 * Save current position in 't for ^^ in visual.
				 */
				names['t'-'a'] = *dot &~ 01;
				if (inopen) {
					extern wchar_t *ncols['z'-'a'+2];
					extern wchar_t *cursor;
					ncols['t'-'a'] = cursor;
				}
			}
			wcscpy(cmdbuf, cp);
			if (strcmp(filebuf, savedfile) || !edited) {
				wchar_t cmdbuf2[sizeof filebuf + 10];
				/* Different file.  Do autowrite & get it. */
				if (!quick) {
					ckaw();
					if (chng && dol > zero)
						error(MSGSTR(M_046, "No write@since last change (:tag! overrides)"), DUMMY_INT);
				}
				oglobp = globp;
				/* strcpy */
				tempchar = "e! ";
				if (mbstowcs(cmdbuf2, tempchar, WCSIZE(cmdbuf2)) == -1)
					error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
				/* strcat */
				if (mbstowcs(cmdbuf2 + wcslen(cmdbuf2), filebuf,
					 WCSIZE(cmdbuf2) - wcslen(cmdbuf2)) == -1)
					error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);

				globp = cmdbuf2;
				d = peekc; ungetchar(0);
				commands((short)1, (short)1);
				peekc = d;
				globp = oglobp;
				value(MAGIC) = omagic;
				samef = 0;
			}

			/*
			 * Look for pattern in the current file.
			 */
			oglobp = globp;
			globp = cmdbuf;
			d = peekc; ungetchar(0);
			if (samef)
				markpr(dot);
			/*
			 * BUG: if it isn't found (user edited header
			 * line) we get left in nomagic mode.
			 */
			value(MAGIC) = 0;
			commands((short)1, (short)1);
			peekc = d;
			globp = oglobp;
			value(MAGIC) = omagic;
			return;
		}	/* end of "for each tag in file" */

		/*
		 * No such tag in this file.  Close it and try the next.
		 */
#ifdef STDIO		/* mjm: was VMUNIX */
		fclose(iof);
#else
		close(io);
#endif
	}	/* end of "for each file in path" */
	if (tfcount <= 0)
		error(MSGSTR(M_047, "No tags file"), DUMMY_INT);
	else
		serror(MSGSTR(M_048, "%S: No such tag@in tags file"), (char *)lasttag);

}

/*
 * Save lines from addr1 thru addr2 as though
 * they had been deleted.
 */
void
yank(void)
{

	if (!FIXUNDO)
		error(MSGSTR(M_049, "Can't yank inside global/macro"), DUMMY_INT);
	save12();
	undkind = UNDNONE;
	killcnt(addr2 - addr1 + 1);
}

/*
 * z command; print windows of text in the file.
 *
 * If this seems unreasonably arcane, the reasons
 * are historical.  This is one of the first commands
 * added to the first ex (then called en) and the
 * number of facilities here were the major advantage
 * of en over ed since they allowed more use to be
 * made of fast terminals w/o typing .,.22p all the time.
 */
short	zhadpr;
short	znoclear;
short	zweight;

void
zop(int hadpr)
{
	register int c, nlines, op;
	short excl;

	zhadpr = hadpr;
	notempty();
	znoclear = 0;
	zweight = 0;
	excl = exclam();
	switch (c = op = ex_getchar()) {

	case '^':
		zweight = 1;
	case '-':
	case '+':
		while (peekchar() == op) {
			ignchar();
			zweight++;
		}
	case '=':
	case '.':
		c = ex_getchar();
		break;

	case EOF:
		znoclear++;
		break;

	default:
		op = 0;
		break;
	}
	if (iswdigit(c)) {
		nlines = c - '0';
		for(;;) {
			c = ex_getchar();
			if (!iswdigit(c))
				break;
			nlines *= 10;
			nlines += c - '0';
		}
		if (nlines < lines)
			znoclear++;
		value(WINDOW) = nlines;
		if (op == '=')
			nlines += 2;
	} else if (op == EOF && !inopen && !splitw) {
		nlines = value(SCROLL);
	} else if (!excl) {
		nlines = value(WINDOW);
	} else {
		nlines = lines - 1;
	}

	if (inopen || c != EOF) {
		ungetchar(c);
		donewline();
	}
	addr1 = addr2;
	if (addr2 == 0 && dot < dol && op == 0)
		addr1 = addr2 = dot+1;
	setdot();
	zop2(nlines, op);
}

void
zop2(register int nlines, int op)
{
	register line *split;

	split = NULL;
	switch (op) {

	case EOF:
		if (addr2 == dol)
			error(MSGSTR(M_050, "\nAt EOF"), DUMMY_INT);
	case '+':
		if (addr2 == dol)
			error(MSGSTR(M_051, "At EOF"), DUMMY_INT);
		addr2 += nlines * zweight;
		if (addr2 > dol)
			error(MSGSTR(M_052, "Hit BOTTOM"), DUMMY_INT);
		addr2++;
	default:
		addr1 = addr2;
		addr2 += nlines-1;
		dot = addr2;
		break;

	case '=':
	case '.':
		znoclear = 0;
		nlines--;
		nlines >>= 1;
		if (op == '=')
			nlines--;
		addr1 = addr2 - nlines;
		if (op == '=')
			dot = split = addr2;
		addr2 += nlines;
		if (op == '.') {
			markDOT();
			dot = addr2;
		}
		break;

	case '^':
	case '-':
		addr2 -= nlines * zweight;
		if (addr2 < one)
			error(MSGSTR(M_053, "Hit TOP"), DUMMY_INT);
		nlines--;
		addr1 = addr2 - nlines;
		dot = addr2;
		break;
	}
	if (addr1 <= zero)
		addr1 = one;
	if (addr2 > dol)
		addr2 = dol;
	if (dot > dol)
		dot = dol;
	if (addr1 > addr2)
		return;
	if (op == EOF && zhadpr) {
		getline(*addr1);
		ex_putchar(QUOTE_CR);
		shudclob = 1;
	} else if (znoclear == 0 && clear_screen != (char *) 0 && !inopen) {
		flush1();
		vclear();
	}
	if (addr2 - addr1 > 1)
		pstart();
	if (split) {
		plines(addr1, split - 1, (short)0);
		splitit();
		plines(split, split, (short)0);
		splitit();
		addr1 = split + 1;
	}
	plines(addr1, addr2, (short)0);
}

static void
splitit(void)
{
	register int l;

	for (l = columns > 80 ? 40 : columns / 2; l > 0; l--)
		ex_putchar('-');
	putnl();
}

void
plines(line *adr1, register line *adr2, short movedot)
{
	register line *addr;

	pofix();
	for (addr = adr1; addr <= adr2; addr++) {
		getline(*addr);
		pline(lineno(addr));
		if (inopen) {
			ex_putchar(QUOTE_NL);
		}
		if (movedot)
			dot = addr;
	}
}

void
pofix(void)
{

	if (inopen && Outchar != termchar) {
		vnfl();
		setoutt();
	}
}

/*
 * Dudley doright to the rescue.
 * Undo saves the day again.
 * A tip of the hatlo hat to Warren Teitleman
 * who made undo as useful as do.
 *
 * Command level undo works easily because
 * the editor has a unique temporary file
 * index for every line which ever existed.
 * We don't have to save large blocks of text,
 * only the indices which are small.  We do this
 * by moving them to after the last line in the
 * line buffer array, and marking down info
 * about whence they came.
 *
 * Undo is its own inverse.
 */
void
undo(short c)
{
	register int i;
	register line *jp, *kp;
	line *dolp1, *newdol, *newadot;

#ifdef UNDOTRACE
	if (trace)
		vudump("before undo");
#endif
	if (inglobal && inopen <= 0)
		error(MSGSTR(M_054, "Can't undo in global@commands"), DUMMY_INT);
	if (!c)
		somechange();
	pkill[0] = pkill[1] = 0;
	change();
	if (undkind == UNDMOVE) {
		/*
		 * Command to be undone is a move command.
		 * This is handled as a special case by noting that
		 * a move "a,b m c" can be inverted by another move.
		 */
		if ((i = (jp = unddel) - undap2) > 0) {
			/*
			 * when c > b inverse is a+(c-b),c m a-1
			 */
			addr2 = jp;
			addr1 = (jp = undap1) + i;
			unddel = jp-1;
		} else {
			/*
			 * when b > c inverse is  c+1,c+1+(b-a) m b
			 */
			addr1 = ++jp;
			addr2 = jp + ((unddel = undap2) - undap1);
		}
		kp = undap1;
		move1(0, unddel);
		dot = kp;
		Command = "move";
		killed();
	} else {
		int cnt;

		newadot = dot;
		cnt = lineDOL();
		newdol = dol;
		dolp1 = dol + 1;
		/*
		 * Command to be undone is a non-move.
		 * All such commands are treated as a combination of
		 * a delete command and a append command.
		 * We first move the lines appended by the last command
		 * from undap1 to undap2-1 so that they are just before the
		 * saved deleted lines.
		 */
		if ((i = (kp = undap2) - (jp = undap1)) > 0) {
			if (kp != dolp1) {
				reverse(jp, kp);
				reverse(kp, dolp1);
				reverse(jp, dolp1);
			}
			/*
			 * Account for possible backward motion of target
			 * for restoration of saved deleted lines.
			 */
			if (unddel >= jp)
				unddel -= i;
			newdol -= i;
			/*
			 * For the case where no lines are restored, dot
			 * is the line before the first line deleted.
			 */
			dot = jp-1;
		}
		/*
		 * Now put the deleted lines, if any, back where they were.
		 * Basic operation is: dol+1,unddol m unddel
		 */
		if (undkind == UNDPUT) {
			unddel = undap1 - 1;
			squish();
		}
		jp = unddel + 1;
		if ((i = (kp = unddol) - dol) > 0) {
			if (jp != dolp1) {
				reverse(jp, dolp1);
				reverse(dolp1, ++kp);
				reverse(jp, kp);
			}
			/*
			 * Account for possible forward motion of the target
			 * for restoration of the deleted lines.
			 */
			if (undap1 >= jp)
				undap1 += i;
			/*
			 * Dot is the first resurrected line.
			 */
			dot = jp;
			newdol += i;
		}
		/*
		 * Clean up so we are invertible
		 */
		unddel = undap1 - 1;
		undap1 = jp;
		undap2 = jp + i;
		dol = newdol;
		netchHAD(cnt);
		if (undkind == UNDALL) {
			dot = undadot;
			undadot = newadot;
		} else
			undkind = UNDCHANGE;
	}
	/*
	 * Defensive programming - after a munged undadot.
	 * Also handle empty buffer case.
	 */
	if ((dot <= zero || dot > dol) && dot != dol)
		dot = one;
#ifdef UNDOTRACE
	if (trace)
		vudump("after undo");
#endif
}

/*
 * Be (almost completely) sure there really
 * was a change, before claiming to undo.
 */
void
somechange(void)
{
	register line *ip, *jp;

	switch (undkind) {

	case UNDMOVE:
		return;

	case UNDCHANGE:
		if (undap1 == undap2 && dol == unddol)
			break;
		return;

	case UNDPUT:
		if (undap1 != undap2)
			return;
		break;

	case UNDALL:
		if (unddol - dol != lineDOL())
			return;
		for (ip = one, jp = dol + 1; ip <= dol; ip++, jp++)
			if ((*ip &~ 01) != (*jp &~ 01))
				return;
		break;

	case UNDNONE:
		error(MSGSTR(M_055, "Nothing to undo"), DUMMY_INT);
	}
	error(MSGSTR(M_056, "Nothing changed|Last undoable command didn't change anything"), DUMMY_INT);
}

/*
 * Map command:
 * map src dest
 */
void
mapcmd(int un, int ab)
/* un true if this is unmap or unabbreviate command */
/* ab true if this is abbr command or false if it is the map command*/
/* So the four calls relate to the following commands
*  0 0 map
*  0 1 abbreviate
*  1 0 unmap
*  1 1 unabbreviate */
{
	wchar_t lhs[100], rhs[100];	/* max sizes resp. */
	register wchar_t *p;
	register int c;
	struct maps *mp;	/* the map structure we are working on */
	char mapbuf[100];	/* buffer for conversion of wchar to char */

	mp = ab ? abbrevs : exclam() ? immacs : arrows;
	if (skipend()) {
		int i;

		/* print current mapping values */
		if (peekchar() != EOF)
			ignchar();
		if (un)
			error(MSGSTR(M_057, "Missing lhs"), DUMMY_INT);
		if (inopen)
			pofix();
		for (i=0; mp[i].mapto; i++)
			if (mp[i].cap) {
				(void)wcstombs(mapbuf, mp[i].descr, 100);
				lprintf("%s", mapbuf);
				ex_putchar('\t');
				(void)wcstombs(mapbuf, mp[i].cap, 100);
				lprintf("%s", mapbuf);
				ex_putchar('\t');
				(void)wcstombs(mapbuf, mp[i].mapto, 100);
				lprintf("%s", mapbuf);
				putNFL();
			}
		return;
	}
		ignore(skipwh());
		for (p=lhs; ; ) {
			c = ex_getchar();
			if (c == Ctrl('V')) {
				c = ex_getchar();
			}
			else if (!un && any(c, " \t")) {
				/* End of lhs */
				break;
			} else if (endcmd(c) && c!='"') {
				ungetchar(c);
				if (un) {
					donewline();
					*p = 0;
					addmac(lhs, NOWCSTR, NOWCSTR, mp);
					return;
				} else
					error(MSGSTR(M_058, "Missing rhs"), DUMMY_INT);
			}
			if (!un && ab && !WORDCHAR(c)) {		/* 001 gray */
				error(MSGSTR(M_059,
				"The lhs can only contain letters, digits, and _'s"), DUMMY_INT);
			}
			*p++ = c;
		}
		*p = 0;

		if (skipend())
			error(MSGSTR(M_058, "Missing rhs"), DUMMY_INT);
		for (p=rhs; ; ) {
			c = ex_getchar();
			if (c == Ctrl('V')) {
				c = ex_getchar();
			} else if (endcmd(c) && c!='"') {
				ungetchar(c);
				break;
			}
			*p++ = c;
		}
		*p = 0;
		donewline();
		/*
		 * Special modification for function keys: #1 means key f1, etc.
		 * If the terminal doesn't have function keys, we just use #1.
		 */
		if (lhs[0] == '#') {
			char *fnkey;
			int lenlhs;	/* size of left hand size */
			int lhsindex;	/* for loop index to strip with */
			wchar_t funkey[3];

			fnkey = fkey(lhs[1] - '0');
			funkey[0] = 'f'; funkey[1] = lhs[1]; funkey[2] = 0;
			if (fnkey) {
				if ((lenlhs = mbstowcs(lhs, fnkey, 100)) <= 0) {
					error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
					exit(-1);
				}
			} else {
				/* there is a space after the # so map the # not a f key */
				funkey[0] = lhs[0]; funkey[1] = 0;
				lenlhs = wcslen(lhs);
			}
			for (lhsindex = 0; lhsindex <= lenlhs; lhsindex++){
		 		if (lhs[lhsindex] == '\n')   /* pull out the '\n' */
			    		 wcscpy(&lhs[lhsindex],&lhs[lhsindex+1]);
			  } /* endfor */
			addmac(lhs,rhs,funkey,mp);
		} else {
			addmac(lhs,rhs,lhs,mp);
		}
}

/*
 * Add a macro definition to those that already exist. The sequence of
 * chars "src" is mapped into "dest". If src is already mapped into something
 * this overrides the mapping. There is no recursion. Unmap is done by
 * using NOWCSTR for dest.  Dname is what to show in listings.  mp is
 * the structure to affect (arrows, etc).
 */
void
addmac(register wchar_t *src,register wchar_t *dest,register wchar_t *dname, register struct maps *mp)
{
	register int slot, zer;

#ifdef UNDOTRACE
	if (trace)
		fprintf(trace, "addmac(src='%S', dest='%S', dname='%S', mp=%x\n", src, dest, dname, mp);
#endif
	if (dest && mp==arrows) {
		/* Make sure user doesn't screw himself */
		/*
		 * Prevent tail recursion. We really should be
		 * checking to see if src is a suffix of dest
		 * but this makes mapping involving escapes that
		 * is reasonable mess up.
		 */
		if (src[1] == 0 && src[0] == dest[wcslen(dest)-1])
			error(MSGSTR(M_060, "No tail recursion"), DUMMY_INT);
		/*
		 * We don't let the user rob himself of ":", and making
		 * multi wide char words is a bad idea so we don't allow it.
		 * Note that if user sets mapinput and maps all of return,
		 * linefeed, and escape, he can make life unlivable for
		 * himself. This is so weird that it is not checked .
		 */
		if (iswalpha(src[0]) && src[1] || any(src[0],":"))
			error(MSGSTR(M_061, "Too dangerous to map that"), DUMMY_INT);
	}
	else if (dest) {
		/* Check for tail recursion in input mode: fussier    */
		int lentail = wcslen(dest) - wcslen(src);
		if (lentail >= 0 && WCeq(src, dest+lentail))
			error(MSGSTR(M_062, "Definitions do not allow recursion.\n"), DUMMY_INT);
	}
	/*
	 * If the src were null it would cause the dest to
	 * be mapped always forever. This is not good.
	 */
	if (src == NOWCSTR || src[0] == 0)
		error(MSGSTR(M_057, "Missing lhs"), DUMMY_INT);

	/* see if we already have a def for src */
	zer = -1;
	for (slot=0; mp[slot].mapto; slot++) {
		if (mp[slot].cap) {
		    if (WCeq(src, mp[slot].cap) || WCeq(src, mp[slot].mapto))
			break;	/* if so, reuse slot */
		} else {
		    zer = slot; /* remember an empty slot */
		}
	}

	if (dest == NOWCSTR) {
		/* unmap */
		if (mp[slot].cap) {
			mp[slot].cap = NOWCSTR;
			mp[slot].descr = NOWCSTR;
		} else {
			error(MSGSTR(M_064, "Not mapped|That macro wasn't mapped"), DUMMY_INT);
		}
		return;
	}

	/* reuse empty slot, if we found one and src isn't already defined */
	if (zer >= 0 && mp[slot].mapto == 0)
		slot = zer;

	/* if not, append to end */
	if (slot >= MAXNOMACS)
		error(MSGSTR(M_065, "Too many macros"), DUMMY_INT);
	if (msnext == 0)	/* first time */
		msnext = mapspace;
	/* Check is a bit conservative, we charge for dname even if reusing src */
	if (msnext - mapspace + wcslen(dest) + wcslen(src) + wcslen(dname) + 3 > MAXCHARMACS)
		error(MSGSTR(M_066, "Too much macro text"), DUMMY_INT);
	CP(msnext, src);
	mp[slot].cap = msnext;
	msnext += wcslen(src) + 1;	/* plus 1 for null on the end */
	CP(msnext, dest);
	mp[slot].mapto = msnext;
	msnext += wcslen(dest) + 1;
	if (dname) {
		CP(msnext, dname);
		mp[slot].descr = msnext;
		msnext += wcslen(dname) + 1;
	} else {
		/* default descr to string user enters */
		mp[slot].descr = src;
	}
}

/*
 * Implements macros from command mode. c is the buffer to
 * get the macro from.
 */
void
cmdmac(char c)
{
	wchar_t macbuf[LBSIZE];
	line *ad, *a1, *a2;
	wchar_t *oglobp;
	short pk;
	short oinglobal;

	lastmac = c;
	oglobp = globp;
	oinglobal = inglobal;
	pk = peekc; peekc = 0;
	if (inglobal < 2)
		inglobal = 1;
	regbuf((wchar_t)c, macbuf, WCSIZE(macbuf));
	a1 = addr1; a2 = addr2;
	for (ad=a1; ad<=a2; ad++) {
		globp = macbuf;
		dot = ad;
		commands((short)1,(short)1);
	}
	globp = oglobp;
	inglobal = oinglobal;
	peekc = pk;
}
