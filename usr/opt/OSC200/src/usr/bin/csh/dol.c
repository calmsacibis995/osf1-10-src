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
static char rcsid[] = "@(#)$RCSfile: dol.c,v $ $Revision: 4.2.9.5 $ (DEC) $Date: 1994/01/23 22:57:10 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: Dfix Dfix1 Dfix2 Dword DgetC Dgetdol setDolp unDredc Dredc 
 *            Dtest Dtestq heredoc
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.14  com/cmd/csh/dol.c, cmdcsh, bos320, 9139320 9/18/91 14:13:34	
 */ 

#include  <fcntl.h>
#include "sh.h"

/*
 * These routines perform variable substitution and quoting via ' and ".
 * To this point these constructs have been preserved in the divided
 * input words.  Here we expand variables and turn quoting via ' and " into
 * QUOTE bits on character (which prevent further interpretation).
 * If the `:q' modifier was applied during history expansion, then
 * some QUOTEing may have occurred already, so we dont "scan(,&trim)" here.
 */
/* Under _SBCS, quoted character not only have their QUOTE bit on but are
 * preceded by the character NLQUOTE.  The QUOTE protects further
 * interpretation, but the NLQUOTE identifies the character as an
 * originally ASCII character rather than an extended character.
 * Under not _SBCS, quoted characters are merely preceeded by an NLQUOTE.
 */

#define	unDgetC(c)	Dpeekc = c

int	Dpeekc, Dpeekrd;		/* Peeks for DgetC and Dredc */
uchar_t	*Dcp, **Dvp;			/* Input vector for Dredc */
char	*QUOTES = "\\'`\"";
char    *HERECHARS = "$\\`\"\'";                                 /*FPM001*/

/*
 * The following variables give the information about the current
 * $ expansion, recording the current word position, the remaining
 * words within this expansion, the count of remaining words, and the
 * information about any : modifier which is being applied.
 */
uchar_t	*dolp;			/* Remaining uchar_ts from this word */
uchar_t	**dolnxt;		/* Further words */
int	dolcnt;			/* Count of further words */
uchar_t	dolmod;			/* : modifier character */
int	dolmcnt;		/* :gx -> 10000, else 1 */

/*
 * Fix up the $ expansions and quotations in the
 * argument list to command t.
 */
void
Dfix(struct command *t)
{
	if (noexec)
		return;
	gflag = 0, rscan(t->t_dcom, Dtest);
	if (gflag == 0)
		return;
	Dfix2(t->t_dcom);
	blkfree(t->t_dcom);
	t->t_dcom = gargv;
	gargv = 0;
}

/*
 * $ substitute one word, for i/o redirection
 */
uchar_t *
Dfix1(uchar_t *cp)
{
	uchar_t *Dv[2];

	if (noexec)
		return (0);
	Dv[0] = cp; Dv[1] = NOSTR;
	Dfix2(Dv);
	if (gargc != 1) {
		setname(cp);
		bferr(MSGSTR(M_AMBIG, "Ambiguous"));
	}
	cp = savestr(gargv[0]);
	blkfree(gargv), gargv = 0;
	return (cp);
}

/*
 * Subroutine to do actual fixing after state initialization.
 */
void
Dfix2(uchar_t **v)
{
	uchar_t *agargv[GAVSIZ];

	ginit(agargv);			/* Initialize glob's area pointers */
	Dvp = v; Dcp = (uchar_t *)"";	/* Setup input vector for Dreadc */
	unDgetC(0); unDredc(0);		/* Clear out any old peeks (at error) */
	dolp = 0; dolcnt = 0;		/* Clear out residual $ expands (...) */
	while (Dword())
		continue;
	gargv = copyblk(gargv);
}

/*
 * Get a word.  This routine is analogous to the routine
 * word() in sh.lex.c for the main lexical input.  One difference
 * here is that we don't get a newline to terminate our expansion.
 * Rather, DgetC will return a EOF when we hit the end-of-input.
 */
int
Dword()
{
	register int c, c1;
	uchar_t wbuf[BUFSIZ];
	register uchar_t *wp = wbuf;
	register bool dolflg;
	bool sofar = 0;
#ifndef _SBCS
	register int n;
	wchar_t nlc;
#endif

loop:
	c = DgetC(DODOL);
#ifndef _SBCS
	if (iswblank(c))
		goto loop;
#endif
	switch (c) {

	case EOF:
deof:
		if (sofar == 0)
			return (0);
		/* finish this word and catch the code above the next time */
		unDredc(c);
		/* fall into ... */

	case '\n':
		*wp = 0;
		goto ret;

#ifdef _SBCS
	case ' ':
	case '\t':
		goto loop;
#endif

	case '`':
		/* We preserve ` quotations which are done yet later */
		*wp++ = c;
	case '\'':
	case '"':
		/*
		 * Note that DgetC never returns a QUOTES character
		 * from an expansion, so only true input quotes will
		 * get us here or out.
		 */
		c1 = c;
		dolflg = c1 == '"' ? DODOL : 0;
		for (;;) {
			if (wp > wbuf + sizeof(wbuf) - MB_LEN_MAX*2)
				goto toochars;
			c = DgetC(dolflg);
			if (c == c1)
				break;
			if (c == '\n' || c == EOF)  {
				char e[NL_TEXTMAX];

				sprintf(e,MSGSTR(M_NOQUOTE,"Unmatched %c"),c1);
				error(e);
			}
#ifndef _SBCS
			if (c == NLQUOTE) {
				c = DgetC(0);
				if (c != '\n')
				    *wp++ = NLQUOTE;
				if ((c == HIST) && (c1 == '`'))
				    *wp++ = '\\';
				PUTCH (wp,c);
				continue;
			}
#else
			if ((c & (QUOTE|TRIM)) == ('\n' | QUOTE) &&
				wp[-1] == NLQUOTE )
				/*
				 *  Let Q be NLQUOTE.
				 *  Quoted \n was preceded by \ Q in input
				 *  Since a Q was appended to \ in processing,
				 *  wbuf is now ... Q \ Q
				 *  This statement strips Q \ off end of wbuf:
				 */
				wp-=2;
#endif
			switch (c1) {

			case '"':
				/*
				 * Leave any `s alone for later.
				 * Other uchar_ts are all quoted, thus `...`
				 * can tell it was within "...".
				 */
#ifndef _SBCS	
				if ((c != '`') && (c != HIST))
					*wp++ = NLQUOTE;
				PUTCH (wp,c);
#else
				if (c != '`' && (c&QUOTE)==0 && c!=NLQUOTE ) {
					*wp++ = NLQUOTE;
					*wp++ = c | QUOTE;
				}
				else
					*wp++ = c;
#endif
				break;

			case '\'':
				/* Prevent all further interpretation */
#ifndef _SBCS
				*wp++ = NLQUOTE;
				PUTCH (wp,c);
#else
				if ((c&QUOTE) == 0 && c!=NLQUOTE) {
					*wp++ = NLQUOTE;
					*wp++ = c | QUOTE;
				}
				else
					*wp++ = c;
#endif
				break;

			case '`':
				/* Leave all text alone for later */
#ifndef _SBCS
				PUTCH (wp,c);
#else
				*wp++ = c;
#endif
				break;
			}
		}
		if (c1 == '`')
			*wp++ = '`';
		goto pack;		/* continue the word */

	case '\\':
		c = DgetC(0);		/* No $ subst! */
		if (c == '\n' || c == EOF)
			goto loop;
#ifndef _SBCS
		*wp++ = NLQUOTE;
		PUTCH (wp,c);
		goto pack;

	case NLQUOTE:
		c = DgetC(0);		/* No $ subst! */
		if (c == '\n' || c == EOF)
			goto loop;
                if (dolmod == 'q')				/* FPM003 */
                	PUTCH(wp,NLQUOTE);
		PUTCH (wp,c);
		goto pack;
#else
		if ((c&QUOTE)==0)
			c |= (NLQUOTE<<8) | QUOTE;
		break;
#endif
	}
		/* following three lines were added/changed for APAR 2683 */
	if (c !=  NLQUOTE)  
		unDgetC(c);
	else *wp++ = c;
pack:
	sofar = 1;
	/* pack up more character in this word */
	for (;;) {
		if (wp > wbuf + sizeof(wbuf) - MB_LEN_MAX*2)
			goto toochars;
		c = DgetC(DODOL);
#ifndef _SBCS
		if ((c == NLQUOTE) || (c == '\\')) {
#else
		if (c == '\\') {
#endif
			c = DgetC(0);
			if (c == EOF)
				goto deof;
			if (c == '\n')
				c = ' ';
			else
#ifndef _SBCS
				{
				*wp++ = NLQUOTE;
				PUTCH(wp,c);
				continue;
				}
#else
				if ((c&QUOTE) == 0)
					c |= (NLQUOTE<<8) | QUOTE;
#endif
		}
		if (c == EOF)
			goto deof;
#ifndef _SBCS
		if (any(c, (uchar_t *)" '`\"\t\n") || iswblank(c)) {
#else
		if (any(c, (uchar_t *)" '`\"\t\n")) {
#endif
			unDgetC(c);
			if (any(c,(uchar_t *)QUOTES))
				goto loop;
			*wp++ = 0;
			goto ret;
		}
#ifndef _SBCS
		PUTCH (wp,c);
#else
		if ( c>>8 )
			*wp++ = c>>8;
		*wp++ = c;
#endif
	}
ret:
	Gcat((uchar_t *)"", wbuf);
	return (1);
toochars:
	error(MSGSTR(M_WORD, "Word too long"));
	/* NOTREACHED */
}

/*
 * Get a character, performing $ substitution unless flag is 0.
 * Any QUOTES character which is returned from a $ expansion is
 * QUOTEd so that it will not be recognized above.
 */
int
DgetC(int flag)
{
	register int c;
#ifndef _SBCS
	register int n;
	wchar_t nlc;
#endif

top:
	if (c = Dpeekc) {
#ifdef _SBCS
		if ((c >> 8) == NLQUOTE) {
			c = NLQUOTE;
			Dpeekc &= 0377;
		}
		else
#endif
		Dpeekc = 0;
		return (c);
	}
	if (lap) {
#ifndef _SBCS
		n = mbtowc(&nlc, (char *)lap, mb_cur_max);
		if (n > 0){
			lap += n;
			c = nlc;
		}
		else
			c = *lap++ & 0xff;
#else
		c = *lap++ & (QUOTE|TRIM);
#endif
		if (c != 0) {
quotspec:
#ifndef _SBCS
			if (dolmod != 'q') {			/* FPM003 */ 
				if (any(c,(uchar_t *)QUOTES)) {
					Dpeekc = c;
					c = NLQUOTE;
				}
		   	}
#else
			if (any(c,(uchar_t *)QUOTES,))
			{
				Dpeekc = c | QUOTE;
				c = NLQUOTE;
			}
#endif
			return (c);
		} else
			lap = NULL;
	}
	if (dolp) {
		if (c = *dolp++ & (QUOTE|TRIM))
			goto quotspec;
		if (dolcnt > 0) {
			setDolp(*dolnxt++);
			--dolcnt;
			return (' ');
		}
		dolp = 0;
	}
	if (dolcnt > 0) {
		setDolp(*dolnxt++);
		--dolcnt;
		goto top;
	}
	c = Dredc();
	if (c == '$' && flag) {
		Dgetdol();
		goto top;
	}
	return (c);
}

static uchar_t	*nulvec[] = { 0 };
static struct varent nulargv = { nulvec, (uchar_t *)"argv", 0 };

/*
 * Handle the multitudinous $ expansion forms.
 * Ugh.
 */
void
Dgetdol(void)
{

#define	EXPAND	128
#define	COUNT	10000 

	register uchar_t *np;
	register struct varent *vp;
#ifndef _SBCS
	uchar_t name[EXPAND*MB_LEN_MAX];
#else
	uchar_t name[EXPAND];
#endif
	int c, sc;
	int subscr = 0, lwb = 1, upb = 0;
	bool dimen = 0, issset = 0;
	uchar_t wbuf[BUFSIZ];

       /*  uchar_t **vec_tmp;   001 (shajenko) */

		/*  variable to avoid requote quoted argv. */
		/*  Clumsy but work! */
	/* static  int	flag ;  001 (shajenko) */ 
	/* static  uchar_t	*ex_vec_tmp = 0;  001 (shajenko) */

	dolmod = dolmcnt = 0;
	c = sc = DgetC(0);
	if (c == '{')
		c = DgetC(0);		/* sc is { to take } later */
#ifndef _SBCS
	if (c == NLQUOTE) {
		c = DgetC(0);
		if (c != '#') { 
			unDgetC(c);
			c = NLQUOTE;
		}
	}
	if (c == '#')
#else
	if (c==NLQUOTE)
		/* if !isatty, # gets quoted by getC() in lex.c */
		/* this statement undoes the quote 'just enough' */
		c = DgetC(0);
	if ((c & TRIM) == '#')
#endif
		dimen++, c = DgetC(0);		/* $# takes dimension */
	else if (c == '?')
		issset++, c = DgetC(0);		/* $? tests existence */
	switch (c) {
	
	case '$':
		if (dimen || issset)
			goto syntax;		/* No $?$, $#$ */
		setDolp(doldol);
		goto eatbrac;

      /* To know this is quoted need to be able to check previous uchar_t */
#ifndef _SBCS
	case NLQUOTE:
		c = DgetC(0);
		if (c != '<')
			goto syntax;
#else
	case '<'|QUOTE:
#endif
		if (dimen || issset)
			goto syntax;		/* No $?<, $#< */

		for (np = wbuf; read(OLDSTD, np, 1) == 1; np++) {
			if (*np == '\n')
				break;
			if (np >= &wbuf[sizeof(wbuf)-1])
				error(MSGSTR(M_TOOLONG,"$< line too long"));
		}
		*np = 0;

                /* For input via $< the entire typed-in line is assigned as a
                   single "word", with no further interpretation at all.  Thus
                   we can have:
                      % set x = $<
                      a b $c " \t q     <---- typed in
                      % echo $x
                      a b $c " \t q
                      % echo $x[1]
                      a b $c " \t q
                      % set y = ($x)
                      % echo $y
                      a b $c " \t q
                      % echo $y[1]
                      a
                      % echo $y[3]
                      $c
                      % echo $y[4]
                      "
                */
		/*
		 * KLUDGE: dolmod is set here because it will
		 * cause setDolp to call domod and thus to copy wbuf.
		 * Otherwise setDolp would use it directly. If we saved
		 * it ourselves, no one would know when to free it.
                 * The actual function of the 'q' causes ALL uchar_ts to
                 * be quoted, no expansion to be done, and the entire
                 * input line to be treated as one word.
		 */
		dolmod = 'q';
		dolmcnt = COUNT; 
		setDolp(wbuf);
		goto eatbrac;

	case EOF:
	case '\n':
		goto syntax;

	case '*':
		strcpy((char *)name, (char *)"argv");
		vp = adrof((uchar_t *)"argv");

                  /* note that argv has not been quoted at 1st time */
                /* vec_tmp = vp->vec;  001 shajenko */

		  /* check if argv has been quoted */

		/* 001 shajenko   BEGIN COMMENT OUT
		if (ex_vec_tmp != *vec_tmp)
		{
		  flag = 0;
		  while (*vec_tmp)
                    {
			*vec_tmp = domod (*vec_tmp, 'q');
			if (flag == 0) {
			   ex_vec_tmp = *vec_tmp;
			   flag = 1;
		        }
                        vec_tmp++;
		    }
                }
                ******* END COMMENT OUT 001 shajenko */

		subscr = -1;			/* Prevent eating [...] */
		break;

	default:
		np = name;
		if (digit(c)) {
			if (dimen)
				goto syntax;	/* No $#1, e.g. */
			subscr = 0;
			do {
				subscr = subscr * 10 + c - '0';
				c = DgetC(0);
			} while (digit(c));
			unDredc(c);
			if (subscr == 0) {
				if (issset) {
					dolp = file ? (uchar_t *)"1": 
						      (uchar_t *)"0";
					goto eatbrac;
				}
				if (file == 0)
					error(MSGSTR(M_NOFILE,
						 "No file for $0"));
				setDolp(file);
				goto eatbrac;
			}
			if (issset)
				goto syntax;
			vp = adrof((uchar_t *)"argv");
			if (vp == 0) {
				vp = &nulargv;
				goto eatmod;
			}
			break;
		}
		if (!alnum(c))
			goto syntax;
		for (;;) {
#ifndef _SBCS
			PUTCH (np,c);
#else
			*np++ = c;
#endif
			c = DgetC(0);
			if (!alnum(c))
				break;
#ifndef _SBCS
			if (np >= &name[sizeof name - MB_LEN_MAX - 1])
#else
			if (np >= &name[sizeof name - 2])
#endif
				goto syntax;
		}
		*np++ = 0;
		unDredc(c);
		vp = adrof((uchar_t *)name);
	}
	if (issset) {
		dolp = (vp || getenv((char *)name)) ? (uchar_t *)"1" : (uchar_t *)"0";
		goto eatbrac;
	}
	if (vp == 0) {
		np = (uchar_t *) getenv((char *)name);
		if (np) {
			addla(np);
			goto eatbrac;
		}
		udvar(name);
		/*NOTREACHED*/
	}
	c = DgetC(0);
	upb = blklen(vp->vec);
	if (dimen == 0 && subscr == 0 && c == '[') {
		np = name;
		for (;;) {
			c = DgetC(DODOL);	/* Allow $ expand within [ ] */
			if (c == ']')
				break;
			if (c == '\n' || c == EOF)
				goto syntax;
#ifndef _SBCS
			if (np >= &name[sizeof name - MB_LEN_MAX - 1])
				goto syntax;
			PUTCH (np, c);
#else
			if (np >= &name[sizeof name - 2])
				goto syntax;
			*np++ = c;
#endif
		}
		*np = 0, np = name;
		if (dolp || dolcnt)		/* $ exp must end before ] */
			goto syntax;
		if (!*np)
			goto syntax;
		if (digit(*np)) {
			register int i = 0;

			while (digit(*np))
				i = i * 10 + *np++ - '0';
			if (i > upb && !any(*np,(uchar_t *)"-*")) {
				goto oob;
			}
			lwb = i;
			if (!*np)
				upb = lwb, np = (uchar_t *)"*";
		}
		if (*np == '*')
			np++;
		else if (*np != '-')
			goto syntax;
		else {
			register int i = upb;

			np++;
			if (digit(*np)) {
				i = 0;
				while (digit(*np))
					i = i * 10 + *np++ - '0';
				if (i > upb)
					goto oob;
			}
			if (i < lwb)
				upb = lwb - 1;
			else
				upb = i;
		}
		if (lwb == 0) {
			if (upb != 0)
				goto oob;
			upb = -1;
		}
		if (*np)
			goto syntax;
	} else {
		if (subscr > 0)
			if (subscr > upb)
				lwb = 1, upb = 0;
			else
				lwb = upb = subscr;
		unDredc(c);
	}
	if (dimen) {
		uchar_t *cp = putn(upb - lwb + 1);

		addla(cp);
		xfree(cp);
	} else {
eatmod:
		c = DgetC(0);
		if (c == ':') {
			c = DgetC(0), dolmcnt = 1;
			if (c == 'g')
				c = DgetC(0), dolmcnt = COUNT;
			if (!any(c,(uchar_t *)"htrqxe"))
				error(MSGSTR(M_DOLMOD, "Bad : mod in $"));
			dolmod = c;
			if (c == 'q')
				dolmcnt = COUNT;
		} else
			unDredc(c);
		dolnxt = &vp->vec[lwb - 1];
		dolcnt = upb - lwb + 1;
	}
eatbrac:
	if (sc == '{') {
		c = Dredc();
		if (c != '}')
			goto syntax;
	}
	return;
oob:
	setname(vp->name);
	error(MSGSTR(M_SUBOUT,"Subscript out of range"));
syntax:
	error(MSGSTR(M_VARSYN, "Variable syntax"));
	/* NOTREACHED */
}

void
setDolp(uchar_t *cp)
{
	register uchar_t *dp;

	if (dolmod == 0 || dolmcnt == 0) {
		dolp = cp;
		return;
	}
	dp = domod(cp, dolmod);
	if (dp) {
		dolmcnt--;
		addla(dp);
		xfree(dp);
	} else
		addla(cp);
	dolp = (uchar_t *)"";
}

void
unDredc(int c)
{

	Dpeekrd = c;
}

int
Dredc(void)
{
	register int c;
#ifndef _SBCS
	register int n;
	wchar_t nlc;
#endif

	if (c = Dpeekrd) {
		Dpeekrd = 0;
		return (c);
	}
#ifndef _SBCS
	if (Dcp) {
		n = mbtowc(&nlc, (char *)Dcp, mb_cur_max);
		if (n > 0) {
			Dcp += n;
			return (nlc);
		} else if (n < 0)
			return (*Dcp++);
	}
#else
	if (Dcp && (c = *Dcp++))
		return (c&(QUOTE|TRIM));
#endif
	if (*Dvp == 0) {
		Dcp = 0;
		return (EOF);
	}
	Dcp = *Dvp++;
	return (' ');
}

int
Dtest(uchar_t c)
{

	/* Note that c isn't trimmed thus !...:q's aren't lost */
	if (strchr("$\\'`\"",(int)c))
		gflag = 1;
}

int
Dtestq(uchar_t c)
{

#ifndef _SBCS
	if ( strchr("\\'`\"",(int)c)  || (c == NLQUOTE))
#else
	if ( (c & QUOTE) || strchr("\\'`\"",(int)c) )
#endif
		gflag = 1;
}

/*
 * Form a shell temporary file (in unit 0) from the words
 * of the shell input up to a line the same as "term".
 * Unit 0 should have been closed before this call.
 */
void
heredoc(uchar_t *term)
{
#ifndef _SBCS
	register wchar_t c;
#else
	register int c;
#endif
	uchar_t *Dv[2];
	uchar_t obuf[BUFR_SIZ], lbuf[BUFR_SIZ], mbuf[BUFR_SIZ];
	int ocnt;
	int anygrave;
	register uchar_t *lbp, *obp, *mbp;
	uchar_t **vp;
	bool quoted;
#ifdef _SBCS
	bool NLquoted;
#endif

	close(0);
	if (open((char *)shtemp, (O_CREAT | O_RDWR), (S_IRUSR | S_IWUSR)) < 0){
		int oerrno=errno;
		unlink(shtemp);
		errno = oerrno;
		Perror((char *)shtemp);
	}
	unlink(shtemp);

	Dv[0] = term;
        Dv[1] = NOSTR;
        gflag = 0;

#ifndef _SBCS
	/*
	 * Should not "scan" here since will need to match "term" exactly 
	 * with input line.
	 */
#else
	scan(Dv, trim);
#endif
        rscan((uchar_t **)Dv, Dtestq);
        quoted = gflag;

	ocnt = sizeof(obuf);
	obp = obuf;
	for (;;) {
		/*
		 * Read up a line
		 */
		lbp = lbuf;
		for (;;) {
#ifndef _SBCS
			if (lbp >= lbuf + sizeof(lbuf) - MB_LEN_MAX - 3)
#else
			if (lbp >= lbuf + sizeof(lbuf) - 4)
#endif
				{
				setname((uchar_t *)"<<");
				error(MSGSTR(M_OVER, "Line overflow"));
			}
			c = readc(1);		/* 1 -> Want EOF returns */
#ifndef _SBCS
			if (c == '\\' && (c = readc(1)) >= 0)
				*lbp++ = (c == '\\') ? NLQUOTE : '\\';
#else
			if (c == NLQUOTE && (c = readc(1)) >= 0)
				c &= TRIM;
#endif
			if (c == '\n')
				break;
			if (c > 0)
#ifndef _SBCS
				{PUTCH(lbp,c);}
#else
				*lbp++ = c;
#endif
			else if (c < 0) {
				setname(term);
				bferr(MSGSTR(M_TERM,"<< terminator not found"));
			}
		}
		*lbp = 0;

		/*
		 * Compare to terminator -- before expansion
		 */
		if (EQ(lbuf, term)) {
			write(0, obuf, sizeof(obuf) - ocnt);
			lseek(0, 0L, SEEK_SET);
			return;
		}

		/*
		 * If term was quoted or -n just pass it on
		 */
		if (quoted || noexec) {
			*lbp++ = '\n'; *lbp = 0;
			for (lbp = lbuf; c = *lbp++;) {
				*obp++ = c;
				if (--ocnt == 0) {
					write(0, obuf, sizeof(obuf));
					obp = obuf;
					ocnt = sizeof(obuf);
				}
			}
			continue;
		}

		/*
		 * Term wasn't quoted so variable and then command
		 * expand the input line
		 */
		anygrave = 0;
		Dcp = lbuf; Dvp = Dv + 1; mbp = mbuf;
		for (;;) {
#ifndef _SBCS
			if (mbp >= mbuf + sizeof(mbuf) - MB_LEN_MAX - 3)
#else
			if (mbp >= mbuf + sizeof(mbuf) - 4)
#endif
			{
				setname((uchar_t *)"<<");
				bferr(MSGSTR(M_OVER, "Line overflow"));
			}
			c = DgetC(DODOL);
#ifdef _SBCS
			if (c == NLQUOTE && (c = DgetC(DODOL)) != EOF)
				c &= TRIM;
#endif
#ifndef _SBCS
			if (c == (wchar_t)EOF)
				break;
#else
			if (c == EOF)
				break;
#endif
			if (c == 0)
				continue;
			if (c == '`')
				anygrave = 1;
			/* \ quotes \ $ ` here */
#ifndef _SBCS
			if ((c == NLQUOTE) || (c == '\\')) {
				*mbp++ = NLQUOTE;
				c = DgetC(0);
                                if (!any(c,(uchar_t *)HERECHARS))  { /*FPM001*/
					*mbp++ = '\\';
				}
			}
			if(c != EOF)
				PUTCH (mbp, c);
#else
			if (c =='\\') {
				c = DgetC(0);
                                if (!any(c,(uchar_t *)HERECHARS)) {  /*FPM001*/
					unDgetC(c | QUOTE | (NLQUOTE<<8) );
					c = '\\';
				}
				else {
					*mbp++ = NLQUOTE;
					c |= QUOTE;
				}
			}
			*mbp++ = c;
#endif
		}
		*mbp = 0;
		mbp = mbuf;

		/*
		 * If any ` in line do command substitution
		 */
		if (anygrave) {
			/*
			 * 1 arg to dobackp causes substitution to be literal.
			 * Words are broken only at newlines so that all blanks
			 * and tabs are preserved.  Blank lines (null words)
			 * are not discarded.
			 */
			vp = dobackp(mbuf, 1);
		} else
			/* Setup trivial vector similar to return of dobackp */
			Dv[0] = mbp, Dv[1] = NOSTR, vp = Dv;

		/*
		 * Resurrect the words from the command substitution
		 * each separated by a newline.  Note that the last
		 * newline of a command substitution will have been
		 * discarded, but we put a newline after the last word
		 * because this represents the newline after the last
		 * input line!
		 */
		for (; *vp; vp++) {
#ifndef _SBCS
			register int n;
			for (mbp = *vp; *mbp;) {
				if (*mbp == NLQUOTE) 
					mbp++;
				n = mblen((char *)mbp, mb_cur_max);
				do {
					*obp++ = *mbp++;
					if (--ocnt == 0) {
						write(0,obuf,sizeof(obuf));
						obp = obuf;
						ocnt = sizeof(obuf);
					}
				} while (--n > 0);
			}
#else
			for (mbp = *vp; *mbp; mbp++) {
				if (*mbp == NLQUOTE)
					*obp++ = *++mbp & TRIM;
				else
					*obp++ = *mbp;
				if (--ocnt == 0) {
					write(0, obuf, sizeof(obuf));
					obp = obuf;
					ocnt = sizeof(obuf);
				}
			}
#endif
			*obp++ = '\n';
			if (--ocnt == 0) {
				write(0, obuf, sizeof(obuf));
				obp = obuf;
				ocnt = sizeof(obuf);
			}
		}
		if (pargv)
			blkfree(pargv), pargv = 0;
	}
}
