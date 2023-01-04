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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: macro.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/20 10:06:55 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint

#endif 
*/
# include "sendmail.h"

/*
**  EXPAND -- macro expand a string using $x escapes.
**
**	Parameters:
**		s -- the string to expand.
**		buf -- the place to put the expansion.
**		buflim -- the buffer limit, i.e., the address
**			of the last usable position in buf.
**		e -- envelope in which to work.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

expand(s, buf, buflim, e)
	register char *s;
	register char *buf;
	char *buflim;
	register ENVELOPE *e;
{
	register char *xp;
	register char *q;
	bool skipping;		/* set if conditionally skipping output */
	bool recurse = FALSE;	/* set if recursion required */
	bool quote, inquote, inescape;
	int i;
	char xbuf[BUFSIZ];
	extern char *macvalue();

	if (tTd(35, 24))
	{
		printf("expand(");
		xputs(s);
		printf(")\n");
	}

	skipping = FALSE;
	if (s == NULL)
		s = "";
	for (xp = xbuf; *s != '\0'; s++)
	{
		char c;

		/*
		**  Check for non-ordinary (special?) character.
		**	'q' will be the interpolated quantity.
		*/

		q = NULL;
		quote = FALSE;
		c = *s;
		switch (c)
		{
		  case CONDIF:		/* see if var set */
			c = *++s;
			skipping = macvalue(c, e) == NULL;
			continue;

		  case CONDELSE:	/* change state of skipping */
			skipping = !skipping;
			continue;

		  case CONDFI:		/* stop skipping */
			skipping = FALSE;
			continue;

		  case QUOTE822:
			quote = TRUE;
			/* FALLTHROUGH */
		  case '\001':		/* macro interpolation */
			c = *++s;
			q = macvalue(c & 0177, e);
			if (q == NULL)
				continue;
			if (quote && !mustquote(q))
				quote = FALSE;
			break;
		}

		/*
		**  Interpolate q or output one character
		*/

		if (skipping || xp >= &xbuf[(sizeof(xbuf)-1)])
			continue;
		inquote = FALSE;
		inescape = FALSE;
		if (q == NULL)
			*xp++ = c;
		else
		{
			/* copy to end of q or max space remaining in buf */
			while ((c = *q++) != '\0' && xp < &xbuf[sizeof xbuf - 1])
			{
				if (iscntrl(c) && !isspace(c))
					recurse = TRUE;
				if (quote) {
					if (!inquote) {
						*xp++ = '"';
						inquote = TRUE;
					}
					if (c == '"' && !inescape)
						*xp++ = '\\';
					if (c == '\\')
						inescape = !inescape;
					else
						inescape = FALSE;
				}
				*xp++ = c;
			}
			if (inescape && xp < &xbuf[sizeof buf - 1])
				*xp++ = '\\';
			if (inquote && xp < &xbuf[sizeof xbuf - 1])
				*xp++ = '"';
		}
	}
	*xp = '\0';

	if (tTd(35, 24))
	{
		printf("expand ==> ");
		xputs(xbuf);
		printf("\n");
	}

	/* recurse as appropriate */
	if (recurse)
	{
		expand(xbuf, buf, buflim, e);
		return;
	}

	/* copy results out */
	i = buflim - buf - 1;
	if (i > xp - xbuf)
		i = xp - xbuf;
	bcopy(xbuf, buf, i);
	buf[i] = '\0';
}
/*
**  DEFINE -- define a macro.
**
**	this would be better done using a #define macro.
**
**	Parameters:
**		n -- the macro name.
**		v -- the macro value.
**		e -- the envelope to store the definition in.
**
**	Returns:
**		none.
**
**	Side Effects:
**		e->e_macro[n] is defined.
**
**	Notes:
**		There is one macro for each ASCII character,
**		although they are not all used.  The currently
**		defined macros are:
**
**		$a   date in ARPANET format (preferring the Date: line
**		     of the message)
**		$b   the current date (as opposed to the date as found
**		     the message) in ARPANET format
**		$c   hop count
**		$d   (current) date in UNIX (ctime) format
**		$e   the SMTP entry message+
**		$f   raw from address
**		$g   translated from address
**		$h   to host
**		$i   queue id
**		$j   official SMTP hostname, used in messages+
**		$k   our UUCP hostname, if different from $w
**		$l   UNIX-style from line+
**		$n   name of sendmail ("MAILER-DAEMON" on local
**		     net typically)+
**		$o   delimiters ("operators") for address tokens+
**		$p   my process id in decimal
**		$q   the string that becomes an address -- this is
**		     normally used to combine $g & $x.
**		$r   protocol used to talk to sender
**		$s   sender's host name
**		$t   the current time in seconds since 1/1/1970
**		$u   to user
**		$v   version number of sendmail
**		$w   our host name (if it can be determined)
**		$x   signature (full name) of from person
**		$y   the tty id of our terminal
**		$z   home directory of to person
**
**		Macros marked with + must be defined in the
**		configuration file and are used internally, but
**		are not set.
**
**		There are also some macros that can be used
**		arbitrarily to make the configuration file
**		cleaner.  In general all upper-case letters
**		are available.
*/

define(n, v, e)
	char n;
	char *v;
	register ENVELOPE *e;
{
	if (tTd(35, 9))
	{
		printf("define(%c as ", n);
		xputs(v);
		printf(")\n");
	}
	e->e_macro[n & 0177] = v;
}
/*
**  MACVALUE -- return uninterpreted value of a macro.
**
**	Parameters:
**		n -- the name of the macro.
**
**	Returns:
**		The value of n.
**
**	Side Effects:
**		none.
*/

char *
macvalue(n, e)
	char n;
	register ENVELOPE *e;
{
	n &= 0177;
	while (e != NULL)
	{
		register char *p = e->e_macro[n];

		if (p == MACNULL)
			/* shadowing null */
			return (NULL);
		if (p != NULL)
			return (p);
		e = e->e_parent;
	}
	return (NULL);
}
/*
**  MUSTQUOTE -- Check if string contains special RFC-822 chars.
**
**	Parameters:
**		s -- the string to be checked.
**
**	Returns:
**		TRUE if string is in need to be quoted, FALSE otherwise.
**
**	Side Effects:
**		none.
**
**	Does this string contain any characters that RFC 822 says
**	must be quoted?
**	This is not strictly correct, since we consider ' ' non-special.
**	Otherwise we'd quote "My Name", which is just too ugly.
*/
mustquote(s)
	register char *s;
{
	register int c;

	while (c = *s++) {
		c &= 0177;
		if (c <= 037 || c == 0177 ||		/* CTLs */
		    index(".()<>@,;:\\\"[]", c) != NULL)/* 822 specials */
			return TRUE;
	}
	return FALSE;
}
