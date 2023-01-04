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
static char rcsid[] = "@(#)$RCSfile: t3.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 17:55:50 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)t3.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)t3.c	4.2 8/11/83";
*/

 /* t3.c: interpret commands affecting whole table */
# include "t..c"
struct optstr {
	char *optnam; 
	int *optadd;
} 
options [] = {
	"expand", &expflg,
	"EXPAND", &expflg,
	"center", &ctrflg,
	"CENTER", &ctrflg,
	"box", &boxflg,
	"BOX", &boxflg,
	"allbox", &allflg,
	"ALLBOX", &allflg,
	"doublebox", &dboxflg,
	"DOUBLEBOX", &dboxflg,
	"frame", &boxflg,
	"FRAME", &boxflg,
	"doubleframe", &dboxflg,
	"DOUBLEFRAME", &dboxflg,
	"tab", &tab,
	"TAB", &tab,
	"linesize", &linsize,
	"LINESIZE", &linsize,
	"delim", &delim1,
	"DELIM", &delim1,
	0,0};
getcomm()
{
	char line[200], *cp, nb[25], *t;
	struct optstr *lp;
	int c, ci, found;
	for(lp= options; lp->optnam; lp++)
		*(lp->optadd) = 0;
	texname = texstr[texct=0];
	tab = '\t';
	printf(".nr %d \\n(.s\n", LSIZE);
	gets1(line);
	/* see if this is a command line */
	if (!index(line,';'))
	{
		backrest(line);
		return;
	}
	for(cp=line; (c = *cp) != ';'; cp++)
	{
		if (!letter(c)) continue;
		found=0;
		for(lp= options; lp->optadd; lp++)
		{
			if (prefix(lp->optnam, cp))
			{
				*(lp->optadd) = 1;
				cp += strlen(lp->optnam);
				if (letter(*cp))
					error(catgets(catd, 1, 4, "Misspelled global option"));
				while (*cp==' ')cp++;
				t=nb;
				if ( *cp == '(')
					while ((ci= *++cp) != ')')
						*t++ = ci;
				else cp--;
				*t++ = 0; 
				*t=0;
				if (lp->optadd == &tab)
				{
					if (nb[0])
						*(lp->optadd) = nb[0];
				}
				if (lp->optadd == &linsize)
					printf(".nr %d %s\n", LSIZE, nb);
				if (lp->optadd == &delim1)
				{
					delim1 = nb[0];
					delim2 = nb[1];
				}
				found=1;
				break;
			}
		}
		if (!found)
			error(catgets(catd, 1, 5, "Illegal option"));
	}
	cp++;
	backrest(cp);
	return;
}
backrest(cp)
char *cp;
{
	char *s;
	for(s=cp; *s; s++);
	un1getc('\n');
	while (s>cp)
		un1getc(*--s);
	return;
}
