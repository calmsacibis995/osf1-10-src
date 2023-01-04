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
static char *sccsid = "@(#)$RCSfile: fregex.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 16:34:40 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1988 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/************************************************************************
 *			Modification History				*
 ************************************************************************/

/*
 * routines to do regular expression matching
 *
 * Entry points:
 *
 *	re_comp(s)
 *		char *s;
 *	... return a compiled regular expression, or proints an error message
 *		and return -1.
 *
 *	re_exec(s)
 *		char *s;
 *	 ... returns 1 if the string s matches the last compiled regular
 *		       expression, 
 *		     0 if the string s failed to match the last compiled
 *		       regular expression, and
 *		    -1 if the compiled regular expression was invalid 
 *		       (indicating an internal error).
 *
 * The strings passed to both re_comp and re_exec may have trailing or
 * embedded newline characters; they are terminated by nulls.
 *
 * The identity of the author of these routines is lost in antiquity;
 * this is essentially the same as the re code in the original V6 ed.
 *
 * The regular expressions recognized are described below. This description
 * is essentially the same as that for ed.
 *
 *	A regular expression specifies a set of strings of characters.
 *	A member of this set of strings is said to be matched by
 *	the regular expression.  In the following specification for
 *	regular expressions the word `character' means any character but NUL.
 *
 *	1.  Any character except a special character matches itself.
 *	    Special characters are the regular expression delimiter plus
 *	    \ [ . and sometimes ^ * $.
 *	2.  A . matches any character.
 *	3.  A \ followed by any character except a digit or ( )
 *	    matches that character.
 *	4.  A nonempty string s bracketed [s] (or [^s]) matches any
 *	    character in (or not in) s. In s, \ has no special meaning,
 *	    and ] may only appear as the first letter. A substring 
 *	    a-b, with a and b in ascending ASCII order, stands for
 *	    the inclusive range of ASCII characters.
 *	5.  A regular expression of form 1-4 followed by * matches a
 *	    sequence of 0 or more matches of the regular expression.
 *	6.  A regular expression, x, of form 1-8, bracketed \(x\)
 *	    matches what x matches.
 *	7.  A \ followed by a digit n matches a copy of the string that the
 *	    bracketed regular expression beginning with the nth \( matched.
 *	8.  A regular expression of form 1-8, x, followed by a regular
 *	    expression of form 1-7, y matches a match for x followed by
 *	    a match for y, with the x match being as long as possible
 *	    while still permitting a y match.
 *	9.  A regular expression of form 1-8 preceded by ^ (or followed
 *	    by $), is constrained to matches that begin at the left
 *	    (or end at the right) end of a line.
 *	10. A regular expression of form 1-9 picks out the longest among
 *	    the leftmost matches in a line.
 *	11. An empty regular expression stands for a copy of the last
 *	    regular expression encountered.
 */


#include	<stdio.h>
#include	<stdlib.h>
#include	"magic_data.h"

/*
 * Forward References:
 */
static struct matcher *advance(char *lp, char *ep);

/*
 * constants for re's
 */
#define	CBRA	1
#define	CCHR	2
#define CIRCF	13
#define	CDOT	4
#define NOCIRCF	15
#define	CCL	6
#define CRETTXT	17
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define	CBACK	18

#define	CSTAR	01

#define	ESIZE	512
#define	NBRA	9

static	char	expbuf[ESIZE], *braslist[NBRA], *braelist[NBRA];

/*
 * compile the regular expression argument into a dfa
 */
char *
fre_comp(sp)
	register char	*sp;
{
	register int	c;
	register char	*ep = expbuf;
	int	cclcnt, numbra = 0;
	char	*lastep = 0;
	char	bracket[NBRA];
	char	*bracketp = &bracket[0];
	static	char	*retoolong = "Regular expression too long";
	char *buf;
	int inret = 0;
	int size;

#define	comerr(msg) {expbuf[0] = 0; numbra = 0; fprintf(stderr, "bad string in magic file: %s\n", msg); return(0); }

	if (*sp == '^') {
		*ep++ = CIRCF;
		sp++;
	}
	else
		*ep++ = NOCIRCF;
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			comerr(retoolong);
		if ((c = *sp++) == '\0') {
			if (bracketp != bracket)
				comerr("unmatched \\(");
			if (inret)
				comerr("unmatched \\%");
			*ep++ = CEOF;
			*ep++ = 0;
			size = ep - &expbuf[0] + 1;
			buf = (char *)malloc(size);
			strncpy(buf, expbuf, size);
			return(buf);
		}
		if (c != '*')
			lastep = ep;
		switch (c) {

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep == 0 || *lastep == CBRA || *lastep == CKET)
				goto defchar;
			*lastep |= CSTAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c = *sp++) == '^') {
				c = *sp++;
				ep[-2] = NCCL;
			}
			do {
				if (c == '\0')
					comerr("missing ]");
				if (c == '-' && ep [-1] != 0) {
					if ((c = *sp++) == ']') {
						*ep++ = '-';
						cclcnt++;
						break;
					}
					while (ep[-1] < c) {
						*ep = ep[-1] + 1;
						ep++;
						cclcnt++;
						if (ep >= &expbuf[ESIZE])
							comerr(retoolong);
					}
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					comerr(retoolong);
			} while ((c = *sp++) != ']');
			lastep[1] = cclcnt;
			continue;

		case '\\':
			if ((c = *sp++) == '(') {
				if (numbra >= NBRA)
					comerr("too many \\(\\) pairs");
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket)
					comerr("unmatched \\)");
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			if (c >= '1' && c < ('1' + NBRA)) {
				*ep++ = CBACK;
				*ep++ = c - '1';
				continue;
			}
			if (c == '%') {
				if (inret) inret = 0;
				else inret = 1;
				*ep++ = CRETTXT;
				continue;
			}
			*ep++ = CCHR;
			*ep++ = c;
			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}

/* 
 * match the argument string against the compiled re
 */

struct matcher ret;

struct matcher *
fre_exec(p1, p2)
	register char	*p1, *p2;
{
	register int	c;
	struct matcher *rv;

	ret.start = ret.finish = (char *) 0;

	for (c = 0; c < NBRA; c++) {
		braslist[c] = (char *) 0;
		braelist[c] = (char *) 0;
	}
	if (*p2++ == CIRCF)
		return((advance(p1, p2)));
	/*
	 * fast check for first character
	 */
	if (*p2 == CCHR) {
		c = p2[1];
		do {
			if (*p1 != c)
				continue;
			if (rv = advance(p1, p2))
				return(rv);
		} while (*p1++);
		return((struct matcher *) 0);
	}
	/*
	 * regular algorithm
	 */
	do
		if (rv = advance(p1, p2))
			return(rv);
	while (*p1++);
	return((struct matcher *) 0);
}

/* 
 * try to match the next thing in the dfa
 */
static struct matcher *
advance(lp, ep)
	register char	*lp, *ep;
{
	register char	*curlp;
	int	ct, i;
	struct matcher *rv;

	for (;;) {
		switch (*ep++) {

		case CCHR:
			if (*ep++ == *lp++)
				continue;
			return((struct matcher *) 0);

		case CDOT:
			if (*lp++)
				continue;
			return((struct matcher *) 0);

		case CDOL:
			if (*lp == '\0')
				continue;
			return((struct matcher *) 0);

		case CEOF:
			return(&ret);

		case CRETTXT:
			if (ret.start == (char *) 0)
				ret.start = lp;
			else
				ret.finish = lp;
			continue;

		case CCL:
			if (cclass(ep, *lp++, 1)) {
				ep += *ep;
				continue;
			}
			return((struct matcher *) 0);

		case NCCL:
			if (cclass(ep, *lp++, 0)) {
				ep += *ep;
				continue;
			}
			return((struct matcher *) 0);

		case CBRA:
			braslist[*ep++] = lp;
			continue;

		case CKET:
			braelist[*ep++] = lp;
			continue;

		case CBACK:
			if (braelist[i = *ep++] == (char *) 0)
				return((struct matcher *) -1);
			if (backref(i, lp)) {
				lp += braelist[i] - braslist[i];
				if (lp < ret.start)
					ret.start = ret.finish = (char *) 0;
				continue;
			}
			return((struct matcher *) 0);

		case CBACK|CSTAR:
			if (braelist[i = *ep++] == (char *) 0)
				return((struct matcher *) -1);
			curlp = lp;
			ct = braelist[i] - braslist[i];
			while (backref(i, lp))
				lp += ct;
			while (lp >= curlp) {
				if (rv = advance(lp, ep))
					return(rv);
				lp -= ct;
			}
			if (lp < ret.start)
				ret.start = ret.finish = (char *) 0;
			continue;

		case CDOT|CSTAR:
			curlp = lp;
			while (*lp++)
				;
			goto star;

		case CCHR|CSTAR:
			curlp = lp;
			while (*lp++ == *ep)
				;
			ep++;
			goto star;

		case CCL|CSTAR:
		case NCCL|CSTAR:
			curlp = lp;
			while (cclass(ep, *lp++, ep[-1] == (CCL|CSTAR)))
				;
			ep += *ep;
			goto star;

		star:
			do {
				lp--;
				if (lp < ret.start)
					ret.start = ret.finish = (char *) 0;
				if (rv = advance(lp, ep))
					return(rv);
			} while (lp > curlp);
			return((struct matcher *) 0);

		default:
			return((struct matcher *) -1);
		}
	}
}

int
backref(i, lp)
	register int	i;
	register char	*lp;
{
	register char	*bp;

	bp = braslist[i];
	while (*bp++ == *lp++)
		if (bp >= braelist[i])
			return(1);
	return(0);
}

int
cclass(set, c, af)
	register char	*set, c;
	int	af;
{
	register int	n;

	if (c == 0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(! af);
}
