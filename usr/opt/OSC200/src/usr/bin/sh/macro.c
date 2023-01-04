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
static char rcsid[] = "@(#)$RCSfile: macro.c,v $ $Revision: 4.2.9.4 $ (DEC) $Date: 1994/01/24 20:39:57 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *	1.17  com/cmd/sh/sh/macro.c, cmdsh, bos320, 9141320 10/3/91 10:13:11
 */

#include	"defs.h"
#include	"sym.h"
#include        <termios.h>

static uchar_t	quote;	/* used locally */
static uchar_t	quoted;	/* used locally */
static uchar_t	dqmac;	/* used locally */

static	int	comsubst ();
static	uchar_t	*copyto ();
static	void	flush ();
static	uchar_t	getch ();

static uchar_t *
copyto(endch)
register uchar_t   endch;
{
	register uchar_t	c;

	while ((c = getch(endch)) != endch && c)
		pushstak(c | quote);
	zerostak();
	if (c != endch)
		error(MSGSTR(M_BADSUB,(char *)badsub));
}

static
skipto(endch)
register uchar_t	endch;
{
	/*
	 * skip uchar_ts up to }
	 */
	register uchar_t	c;

	while ((c = readc()) && c != endch)
	{
		switch (c)
		{
		case SQUOTE:
			skipto(SQUOTE);
			break;

		case DQUOTE:
			skipto(DQUOTE);
			break;

		case DOLLAR:
			if (readc() == BRACE)
				skipto('}');
		}
	}
	if (c != endch)
		error(MSGSTR(M_BADSUB,(char *)badsub));
}

static uchar_t
getch(endch)
uchar_t	endch;
{
	register uchar_t	d;

retry:
	d = readc();
	if (!subchar(d))
		return(d);
	if (d == DOLLAR)
	{
		register unsigned int	c;
#ifndef _SBCS
		register unsigned int	nlc;
#endif

		if (dqmac)
			return(d);
#ifndef _SBCS
		c = readc();
		nlc = readwc(c);
		if (dolchar(c)|| (NLSencchar(c) && NLSletter(nlc)))
#else
		if ((c = readc(), dolchar(c)||NLSalphanum(c)))
#endif
		/* if this works, new dolchar() should be derived */
		{
			struct namnod *n = (struct namnod *)NIL;
			int		dolg = 0;
			BOOL		bra;
			BOOL		nulflg;
			BOOL		qflag = 0;
			register uchar_t	*argp, *v;
			uchar_t		idb[2];
			uchar_t		*id = idb;

			if (bra = (c == BRACE))
#ifndef _SBCS
			{
				c = readc();
				nlc = readwc(c);
			}
#else
				c = readc();
#endif
#ifndef _SBCS
			if (NLSencchar(c) && NLSletter(nlc))
#else
			if (NLSletter(c))
#endif
			{
				register uchar_t *s;
				argp = (uchar_t *)relstak();
				pushstak(FNLS);
#ifndef _SBCS
				while (NLSencchar(c) && NLSalphanum (nlc)) {
					pushstak (c);
					if (NLSfontshift(c))
					    NLpushstak (nlc);
					c = readc ();
					nlc = readwc(c);
				}	
				if (NLSfontshift (c))
					unreadwc (nlc);
#else
				while (1)
				{
					if (NLSfontshift(c)) {
						pushstak(c);
						c = readc();
					}
					else if (!NLSalphanum(c)) break;
					pushstak(c);
					c = readc();
				}
# endif
				zerostak();
				n =  lookup(NLSndecode(absstak(argp)));
				setstak(argp);
				if (n->namflg & N_FUNCTN)
					error(MSGSTR(M_BADSUB,(char *)badsub));
				v = n->namval;
				id = n->namid;
				peekc = c | MARK;
			}
			else if (digchar(c))
			{
				*id = c;
				idb[1] = 0;
				if (astchar(c))
				{
					dolg = 1;
					c = '1';
				}
				c -= '0';
				v = ((c == 0) ? cmdadr : (c <= dolc) ? dolv[c] : (uchar_t *)(dolg = 0));
			}
			else if (c == '$')
				v = pidadr;
			else if (c == '!')
				v = pcsadr;
			else if (c == '#')
			{
				itos(dolc);
				v = numbuf;
			}
			else if (c == '?')
			{
				itos(retval);
				v = numbuf;
			}
			else if (c == '-')
				v = flagadr;
			else if (bra)
				error(MSGSTR(M_BADSUB,(char *)badsub));
			else
				goto retry;
			c = readc();
			if (c == ':' && bra)	/* null and unset fix */
			{
				nulflg = 1;
				c = readc();
			}
			else 
			{
				nulflg = 0;
			}
			if ( (c == '?') && bra && (!quote) )
				qflag = 1;
			if (!defchar(c) && bra)
				error(MSGSTR(M_BADSUB,(char *)badsub));
			argp = 0;
			if (bra)
			{
				if (c != '}')
				{
					argp = (uchar_t *)relstak();
					/* Added check for NLS encoded character.  In
					   instance of string substitution from NULL
					   NLS encoded character, the substition would
					   incorrectly fail.
					   */
					if ((v == 0 || (nulflg && (*v == 0 ||
								   (NLSisencoded(v) &&
								    *++v == 0))))
					    ^ (setchar(c)))
					{
					/* if possibility of conditional   */
					/* substitution, add magic header  */
					/* to mark string encoded         */
					    if (qflag == 1)
						pushstak(FNLS);
					    copyto('}');
					}
					else
					    skipto('}');
					argp = absstak(argp);
				}
			}
			else
			{
				peekc = c | MARK;
				c = 0;
			}
			if (v && (!nulflg || *v))
			{
				uchar_t tmp = (*id == '*' ? SP | quote : SP);

				if (c != '+')
				{
					for (;;)
					{
						if (*v == 0 && quote) {
							pushstak(QUOTE);
						} else
						{

	/* a bizarre inversion is happening here.
	Ordinarily decoded characters are put into the
	input stream and encoded by readc().  macro() sets
	a flag to allow it to put encoded characters into
	the stream.  But within $ substitutions it gets
	decoded strings from lookup.  So here it encodes
	them & pushes them into the stream.  The quote
	flag is merged into characters as required. No
	FNLS character is pushed on the stack.
	*/

#ifndef _SBCS
							if (NLSisencoded(v))
							    while (c = *(++v))
									pushstak(c | quote);
							else
# endif
			    				NLSencode_stk(v,quote);
						}

						if (dolg == 0 || (++dolg > dolc))
							break;
						else
						{
							v = dolv[dolg];
							pushstak(tmp);
						}
					}
				}
			}
			else if (argp)
			{
				if (c == '?') {
					/*
					 * if there is an error message behind
					 * the '?', print it.  else print the
					 * standard "parameter null or not
					 * set" error
					 */

					/*
					 * have to do something special here
					 * for this to work with the FNLS
					 * code...
					 */
					uchar_t *arg = argp;

					/*
					 * if the arg is encoded, but that's
					 * the only thing here, point at the
					 * NULL so we get the proper mesg
					 */
					if (NLSisencoded(arg)) {
						(void) NLSskiphdr(arg);
						if (*arg != '\0')
							arg = argp;
						}

					failed(id, *arg ? arg : 
					       (uchar_t *)(MSGSTR(M_BADPARAM,(char *)badparam)));
					}
				else if (c == '=')
				{
					if (n)
					{
						trim(argp);
						assign(n, NLSndecode (argp));
					}
					else
						error(MSGSTR(M_BADSUB,(char *)badsub));
				}
			}
			else if (flags & setflg)
				failed(id, MSGSTR(M_UNSET,(char *)unset));
			goto retry;
		}
		else
#ifndef _SBCS
		{
			peekc = c | MARK;
			if (NLSfontshift(c))
				unreadwc (nlc);
		}
#else
			peekc = c | MARK;
#endif
	}
	else if (d == endch)
		return(d);
	else if (d == SQUOTE)
	{
		if (dqmac)
			return(d);
		comsubst();
		goto retry;
	}
	else if (d == DQUOTE)
	{
		quoted++;
		quote ^= QUOTE;
		goto retry;
	}
	return(d);
}

uchar_t *
macro(as)
uchar_t	*as;
{
	/*
	 * Strip "" and do $ substitution
	 * Leaves result on top of stack
	 */
	register BOOL	savqu = quoted;
	register uchar_t	savq = quote;
	struct fileheader fb;

	push(&fb);
	estabf(as);
# ifdef NLSDEBUG
	debug("macro in",as);
# endif
	standin->fraw = TRUE;

	/* estabf puts the given string into a file descriptor, then
	   processes it (using copyto()) as if it were input.  In general,
	   such processing encodes NLS strings to the internal format.
	   Since in this case the string 'as' is already encoded,
	   the flag fraw tells readc() to disable NLS encoding.
	   Some characters may be quoted by the shell at this point;
	   all extended NLS characters are preceded by some Font Shift.
	*/
	usestak();
	quote = 0;
	quoted = 0;
	copyto(0);
	pop();
	if (quoted && (stakbot == staktop-1))
		/* quoted null string still has FNLS header */
		pushstak(QUOTE);
/*
 * above is the fix for *'.c' bug
 */
	quote = savq;
	quoted = savqu;
#if defined (NLSDEBUG)
	{ register uchar_t *s = fixstak();
	  fprintf ( stderr , "macro out = <%s>\n",s);
	  return(s);
	}
#else
	return(fixstak());
#endif
}

uchar_t *
dqmacro(as)
uchar_t	*as;
{
	dqmac = 1;
	as = macro(as);
	dqmac = 0;
	return(as);
}

static int
comsubst()
{
	/*
	 * command substn
	 */
	struct fileblk	cb;
	register uchar_t	d;
	register uchar_t *savptr = fixstak();
        register uchar_t *savptr2;
        register long savflags = flags;

	usestak();
	while ((d = readc()) != SQUOTE && d)
		pushstak(d);
	{
		register uchar_t	*argc;

		trim(argc = fixstak());
		push(&cb);
		estabf(argc);
		standin->fraw = TRUE;
	}
	{
		register struct trenod *t = makefork(FPOUT, cmd(EOFSYM, MTFLG | NLFLG));
		int		pv[2];

		/*
		 * this is done like this so that the pipe
		 * is open only when needed
		 */
		chkpipe(pv);
		initf(pv[INPIPE]);
		standin->fraw = TRUE;
		execute(t, 0, (int)(flags & errflg), 0, pv);
		close(pv[OUTPIPE]);
	}
	tdystak(savptr);
	staktop = movstr(savptr, stakbot);
	standin->fraw = FALSE;
	while (d = readc())
		pushstak(d | quote);
	await(0, 0);
	flags = savflags;
#ifndef _SBCS

	/*  For SJIS, cannot strip trailing newlines from top of
	 *  stack down by examining top byte only.  Need to find
	 *  start of character at top of stack. ((0x0a | 0x80) is 
	 *  not a legal 1-byte character but is a legal second byte
	 *  of a 2-byte character -- if so, previous byte could be 
	 *  a SJIS font shift or the second byte of a 2-byte character.)
	 */
	{
	do {
		savptr = stakbot;
		while (savptr2 = savptr, d = *savptr & STRIP){
		    if ((savptr += NLSenclen (savptr)) >= staktop)
			break;
		}
		/* d will be ASCII character or magic font shift */
/*CMR001*/
/* In the presence of a null string the above construct fails to strip
   the newline, possibly because the macro wants to check multiple bytes?? 
   So I have used the prior code if 0 is detected as a hack to correct
   this oversight. The code should all be converted to wchar and then this 
   hack, the above hack and all of the NLS hacks can be done away with. */

		if (d == 0){
       while (stakbot != staktop)
        {
                if ((*--staktop & STRIP) != NL || ((stakbot != staktop) &&
                        (NLSfontshift (staktop [-1]))))
                {
                        ++staktop;
                        break;
                }
        }
		}/*CMR*/

		if (d != NL)
			break;
#ifdef NLSDEBUG
		debug ("comsubst - NL stripped", d);
#endif
	} while ((staktop -= NLSenclen (savptr2)) > stakbot);
	}
#else
	while (stakbot != staktop)
	{
		if ((*--staktop & STRIP) != NL || ((stakbot != staktop) &&
			(NLSfontshift (staktop [-1]))))
		{
			++staktop;
			break;
		}
	}
#endif
	pop();
}

#define CPYSIZ	512

subst(in, ot)
int	in, ot;
{
	register uchar_t c;
	struct fileblk	 fb;
	register int	 count = CPYSIZ;
	register int	 mblength;

	push(&fb);
	initf(in);
	standin->fraw = TRUE;   /* avoid re-NLS-encoding input */
	/*
	 * DQUOTE used to stop it from quoting
	 */
	while (c = (getch(DQUOTE) & STRIP))
	{
		pushstak(c);
#ifdef _SBCS
		if (NLSfontshift(c)) {
			pushstak(getch(DQUOTE));
			--count;
		}
#else
		if (c == FSH0) {
			pushstak(getch(DQUOTE));
			--count;
		}
		else if ((c == FSH20) || (c == FSH21)) {
			/*
			 * Set c to the char that holds the size of the
			 * multibyte character.
			 */
			pushstak(c = getch(DQUOTE)); 
			--count;
			mblength = c & STRIP;
			while (mblength--) {
				pushstak(getch(DQUOTE));
				pushstak(getch(DQUOTE));
				count -= 2;
			}
		}
# endif
		if (--count <= 0)  /* using stack, CPYSIZ can be off 1 */
		{
			flush(ot);
			count = CPYSIZ;
		}
	}
	flush(ot);
	pop();
}

static void
flush(ot)
{
	*staktop = 0;
# if NLSDEBUG
	debug("flush in",stakbot);
# endif
	NLSdecode(stakbot);
	staktop = stakbot + strlen((char *)stakbot);
# if NLSDEBUG
	debug("flush out",stakbot);
# endif
	write(ot, stakbot, staktop - stakbot);

		/*	The execpr is a define in
		*	defs.h that will mask for the "-x" flag. Why it
		*	does this is a mystery, because it will place a
		*	"\n" in the stderr file.
		*	I have left the code here because I do not know
		*	if anything else is affected. I ran some tests
		*	and they appeared to run fine.
	if (flags & execpr)
		write(output, stakbot, staktop - stakbot);
		*/

	staktop = stakbot;
}
