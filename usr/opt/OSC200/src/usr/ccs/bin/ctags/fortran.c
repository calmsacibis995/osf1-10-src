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
static char rcsid[] = "@(#)$RCSfile: fortran.c,v $ $Revision: 4.2.2.2 $ (OSF) $Date: 1993/08/25 22:26:36 $";
#endif
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * fortran.c	 5.4 (Berkeley) 6/1/90
 */


#include "ctags.h"
#include <strings.h>

char	*lbp;				/* line buffer pointer */
static	void takeprec(void);

int
PF_funcs(void)
{
	register bool	pfcnt;		/* pascal/fortran functions found */
	register char	*cp;
	char	tok[MAXTOKEN],
		*gettoken();

	for (pfcnt = NO;;) {
		lineftell = ftell(inf);
		if (!fgets(lbuf,sizeof(lbuf),inf))
			return(pfcnt);
		++lineno;
		lbp = lbuf;
		if (*lbp == '%')	/* Ratfor escape to fortran */
			++lbp;
		for (;isspace(*lbp);++lbp);
		if (!*lbp)
			continue;
		switch (*lbp | ' ') {	/* convert to lower-case */
		case 'c':
			if (cicmp("complex") || cicmp("character"))
				takeprec();
			break;
		case 'd':
			if (cicmp("double")) {
				for (;isspace(*lbp);++lbp);
				if (!*lbp)
					continue;
				if (cicmp("precision"))
					break;
				continue;
			}
			break;
		case 'i':
			if (cicmp("integer"))
				takeprec();
			break;
		case 'l':
			if (cicmp("logical"))
				takeprec();
			break;
		case 'r':
			if (cicmp("real"))
				takeprec();
			break;
		}
		for (;isspace(*lbp);++lbp);
		if (!*lbp)
			continue;
		switch (*lbp | ' ') {
		case 'f':
			if (cicmp("function"))
				break;
			continue;
		case 'p':
			if (cicmp("program") || cicmp("procedure"))
				break;
			continue;
		case 's':
			if (cicmp("subroutine"))
				break;
		default:
			continue;
		}
		for (;isspace(*lbp);++lbp);
		if (!*lbp)
			continue;
		for (cp = lbp + 1;*cp && intoken(*cp);++cp);
		if (cp = lbp + 1)
			continue;
		*cp = EOS;
		(void)strcpy(tok,lbp);
		getline();			/* process line for ex(1) */
		pfnote(tok,lineno);
		pfcnt = YES;
	}
	/*NOTREACHED*/
}

/*
 * cicmp --
 *	do case-independent strcmp
 */
int
cicmp(char *cp)
{
	register int	len;
	register char	*bp;

	for (len = 0,bp = lbp;*cp && (*cp &~ ' ') == (*bp++ &~ ' ');
	    ++cp,++len);
	if (!*cp) {
		lbp += len;
		return(YES);
	}
	return(NO);
}

static void
takeprec(void)
{
	for (;isspace(*lbp);++lbp);
	if (*lbp == '*') {
		for (++lbp;isspace(*lbp);++lbp);
		if (!isdigit(*lbp))
			--lbp;			/* force failure */
		else
			while (isdigit(*++lbp));
	}
}
