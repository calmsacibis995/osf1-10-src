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
static char rcsid[] = "@(#)$RCSfile: lisp.c,v $ $Revision: 4.2.2.2 $ (OSF) $Date: 1993/08/25 22:26:42 $";
#endif
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * lisp.c	5.4 (Berkeley) 6/1/90
 */


#include "ctags.h"
#include <strings.h>

extern char	*lbp;			/* pointer shared with fortran */

/*
 * lisp tag functions
 * just look for (def or (DEF
 */
void
l_entries(void)
{
	register int	special;
	register char	*cp,
			savedc;
	char	tok[MAXTOKEN];

	for (;;) {
		lineftell = ftell(inf);
		if (!fgets(lbuf,sizeof(lbuf),inf))
			return;
		++lineno;
		lbp = lbuf;
		if (!cicmp("(def"))
			continue;
		special = NO;
		switch(*lbp | ' ') {
		case 'm':
			if (cicmp("method"))
				special = YES;
			break;
		case 'w':
			if (cicmp("wrapper") || cicmp("whopper"))
				special = YES;
		}
		for (;!isspace(*lbp);++lbp);
		for (;isspace(*lbp);++lbp);
		for (cp = lbp;*cp && *cp != '\n';++cp);
		*cp = EOS;
		if (special) {
			if (!(cp = index(lbp,')')))
				continue;
			for (;cp >= lbp && *cp != ':';--cp);
			if (cp < lbp)
				continue;
			lbp = cp;
			for (;*cp && *cp != ')' && *cp != ' ';++cp);
		}
		else
			for (cp = lbp + 1;
			    *cp && *cp != '(' && *cp != ' ';++cp);
		savedc = *cp;
		*cp = EOS;
		(void)strcpy(tok,lbp);
		*cp = savedc;
		getline();
		pfnote(tok,lineno);
	}
	/*NOTREACHED*/
}
