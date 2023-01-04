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
static char rcsid[] = "@(#)$RCSfile: te.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 18:26:36 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)te.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)te.c	4.2 8/11/83";
*/

 /* te.c: error message control, input line count */
# include "t..c"
error(s)
char *s;
{
	fprintf(stderr,catgets(catd, 1, 2, "\n%s: line %d: %s\n"), ifile, iline, s);
# ifdef unix
	fprintf(stderr,catgets(catd, 1, 30, "tbl quits\n"));
	exit(1);
# endif
# ifdef gcos
	fprintf(stderr, "run terminated due to error condition detected by tbl preprocessor\n");
	exit(0);
# endif
}
gets1(s)
char *s;
{
	char *p;
	int nbl = 0;
	iline++;
	p=fgets(s,BUFSIZ,tabin);
	while (p==0)
	{
		if (swapin()==0)
			return(0);
		p = fgets(s,BUFSIZ,tabin);
	}

DX(fprintf(stderr," *p = %s in gets1()\n",p));
	while (*s) s++;
	s--;
	if (*s == '\n') *s-- =0;
	for(nbl=0; *s == '\\' && s>p; s--)
		nbl++;
	if (linstart && nbl % 2) /* fold escaped nl if in table */
		gets1(s+1);

	return((int)p);
}
wgets1(s)
wchar_t *s;
{
	wchar_t *wcsp;
	int nbl = 0;
	char *p;
	char ctmp[BUFSIZ];

	iline++;
	p=fgets(ctmp,BUFSIZ,tabin);
	while (p==0)
	{
		if (swapin()==0)
			return(0);
		p = fgets(ctmp,BUFSIZ,tabin);
	}
DX(fprintf(stderr," *p = %s in wgets1()\n",p));
	while (*p) p++;
	p--;
	if (*p == '\n') *p-- =0;

	if(mbstowcs(s,ctmp,BUFSIZ)<0){
		fprintf(stderr,catgets(catd, 1, 31, "Illegal code\n"));
		return(0);
	}

DX(fprintf(stderr,"wgets1(): *s = %x %x %x %x %x %x %x %x %x %x %x \n",*s,*(s+1),*(s+2),*(s+3),*(s+4),*(s+5),*(s+6),*(s+7),*(s+8),*(s+9),*(s+10)));
	wcsp = s;
	while (*s) s++;
	s--;
	for(nbl=0; *s == '\\' && s>wcsp; s--)
		nbl++;
	if (linstart && nbl % 2) /* fold escaped nl if in table */
		wgets1(s+1);

	return((int)wcsp);
}
# define BACKMAX 500
char backup[BACKMAX];
char *backp = backup;
un1getc(c)
{
	if (c=='\n')
		iline--;
	*backp++ = c;
	if (backp >= backup+BACKMAX)
		error(catgets(catd, 1, 27, "too much backup"));
}
get1char()
{
	int c;
	if (backp>backup)
		c = *--backp;
	else
		c=getc(tabin);
DX(fprintf(stderr," c = %x in get1char()\n",c));
	if (c== EOF) /* EOF */
	{
		if (swapin() ==0)
			error(catgets(catd, 1, 28, "unexpected EOF"));
		c = getc(tabin);
	}
	if (c== '\n')
		iline++;
	return(c);
}
