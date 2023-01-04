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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: yacc.c,v $ $Revision: 4.2.2.2 $ (OSF) $Date: 1993/08/25 22:27:07 $";
#endif
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *       yacc.c	 5.5 (Berkeley) 6/1/90
 */


#include "ctags.h"
#include <strings.h>

/*
 * y_entries:
 *	find the yacc tags and put them in.
 */
void
y_entries(void)
{
	register int	c;
	register char	*sp;
	register bool	in_rule;
	char	tok[MAXTOKEN];

	while (GETC(!=,EOF))
		switch ((char)c) {
		case '\n':
			SETLINE;
			/* FALLTHROUGH */
		case ' ':
		case '\f':
		case '\r':
		case '\t':
			break;
		case '{':
			if (skip_key((int)'}'))
				in_rule = NO;
			break;
		case '\'':
		case '"':
			if (skip_key(c))
				in_rule = NO;
			break;
		case '%':
			if (GETC(==,'%'))
				return;
			(void)ungetc(c,inf);
			break;
		case '/':
			if (GETC(==,'*'))
				skip_comment();
			else
				(void)ungetc(c,inf);
			break;
		case '|':
		case ';':
			in_rule = NO;
			break;
		default:
			if (in_rule || !isalpha(c) && c != (int)'.'
			    && c != (int)'_')
				break;
			sp = tok;
			*sp++ = c;
			while (GETC(!=,EOF) && (intoken(c) || c == (int)'.'))
				*sp++ = c;
			*sp = EOS;
			getline();		/* may change before ':' */
			while (iswhite(c)) {
				if (c == (int)'\n')
					SETLINE;
				if (GETC(==,EOF))
					return;
			}
			if (c == (int)':') {
				pfnote(tok,lineno);
				in_rule = YES;
			}
			else
				(void)ungetc(c,inf);
		}
}

/*
 * toss_yysec --
 *	throw away lines up to the next "\n%%\n"
 */
void
toss_yysec(void)
{
	register int	c,			/* read character */
			state;

	/*
	 * state == 0 : waiting
	 * state == 1 : received a newline
	 * state == 2 : received first %
	 * state == 3 : recieved second %
	 */
	lineftell = ftell(inf);
	for (state = 0;GETC(!=,EOF);)
		switch ((char)c) {
			case '\n':
				++lineno;
				lineftell = ftell(inf);
				if (state == 3)		/* done! */
					return;
				state = 1;		/* start over */
				break;
			case '%':
				if (state)		/* if 1 or 2 */
					++state;	/* goto 3 */
				break;
			default:
				state = 0;		/* reset */
		}
}
