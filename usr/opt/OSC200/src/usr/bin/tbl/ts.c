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
static char rcsid[] = "@(#)$RCSfile: ts.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 18:41:17 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)ts.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)ts.c	4.2 8/11/83";
*/

 /* ts.c: minor string processing subroutines */
#include "t..c"

match (s1, s2)
char *s1, *s2;
{
	while (*s1 == *s2)
		if (*s1++ == '\0')
			return(1);
		else
			s2++;
	return(0);
}
wmatch (s1, s2)
wchar_t *s1, *s2;
{
	while (*s1 == *s2)
		if (*s1++ == 0)
			return(1);
		else
			s2++;
	return(0);
}
prefix(small, big)
char *small, *big;
{
	int c;
	while ((c= *small++) == *big++)
		if (c==0) return(1);
	return(c==0);
}
wprefix(small, big)
wchar_t *small, *big;
{
	int c;
	while ((c= *small++) == *big++)
		if (c==0) return(1);
	return(c==0);
}
letter (ch)
{
	if (ch >= 'a' && ch <= 'z')
		return(1);
	if (ch >= 'A' && ch <= 'Z')
		return(1);
	return(0);
}
numb(str)
char *str;
{
	/* convert to integer */
	int k;
	for (k=0; *str >= '0' && *str <= '9'; str++)
		k = k*10 + *str - '0';
	return(k);
}
digit(x)
{
	return(x>= '0' && x<= '9');
}
max(a,b)
{
	return( a>b ? a : b);
}
tcopy (s,t)
char *s, *t;
{
	while (*s++ = *t++);
}
wtcopy (s,t)
wchar_t *s, *t;
{
	while (*s++ = *t++);
}

/* The following codes were extraced from sh.misc.c
*/

blklen(av)
        register char **av;
{
        register int i = 0;

        while (*av++)
                i++;
        return (i);
}

wblklen(av)
        register wchar_t **av;
{
        register int i = 0;

        while (*av++)
                i++;
        return (i);
}

nomem(i)
	unsigned i;
{
	error(catgets(catd, 1, 24, "no space for characters"));
	return (0);
}

xfree(cp)
        char *cp;
{
        extern char end[];

        if (cp >= end && cp < (char *) &cp)
                free(cp);
}

char *savestr(s)
        register char *s;
{
        char *n;
        register char *p;
        if (s == 0)
                s = "";
        for (p = s; *p++;)
                ;
        n = p = xalloc((unsigned) (p - s));
        while (*p++ = *s++)
                ;
        return (n);
}

wchar_t *savewcs(s)
        register wchar_t *s;
{
        wchar_t *n;
        register wchar_t *p;
        if (s == 0)
                s = WNULLp;
        for (p = s; *p++;)
                ;
        n = p = (wchar_t *)xalloc((unsigned)(p - s)*sizeof (wchar_t));
        while (*p++ = *s++)
                ;
        return (n);
}

blkfree(av0)
        char **av0;
{
        register char **av = av0;

        for (; *av; av++)
                XFREE((char *)*av)
        XFREE((char *)av0)
}

wblkfree(av0)
        wchar_t **av0;
{
        register wchar_t **av = av0;

        for (; *av; av++)
                XFREE((char *)*av)
        XFREE((char *)av0)
}

char **saveblk(v)
        register char **v;
{
        register char **newv =
		(char **) calloc((unsigned) (blklen(v) + 1),
				 (unsigned)sizeof (char **));
        char **onewv = newv;

        while (*v)
                *newv++ = savestr(*v++);
        return (onewv);
}

wchar_t **wsaveblk(v)
        register wchar_t **v;
{
        register wchar_t **newv =
		(wchar_t **) calloc((unsigned) (wblklen(v) + 1),
				    (unsigned)sizeof (wchar_t **));
        wchar_t **onewv = newv;

        while (*v)
                *newv++ = savewcs(*v++);
        return (onewv);
}

wchar_t wcsbuf[BUFSIZ];
char    mbsbuf[BUFSIZ];

wchar_t *Towcs(cp)
char *cp;
{
	mbstowcs(wcsbuf, cp, BUFSIZ);
	return(savewcs(wcsbuf));
}

wchar_t **Towcsv(cp)
char **cp;
{
        register wchar_t **wcp =
		(wchar_t **)calloc((unsigned)(wblklen(cp) + 1),
				   (unsigned)sizeof (wchar_t *));
	register wchar_t **backwcp = wcp;
	register char **tmpcp = cp;

	while(*tmpcp){
		mbstowcs(wcsbuf, *tmpcp, BUFSIZ);
		*wcp = savewcs(wcsbuf);
		tmpcp++;
		wcp++;
	}
        *wcp = 0;
	return(backwcp);
}

char *Tombs(wcp)
wchar_t *wcp;
{
	wcstombs(mbsbuf, wcp, sizeof (mbsbuf));
	return(savestr(mbsbuf));
}

char **Tombsv(wcp)
wchar_t **wcp;
{
        register char **cp =
		(char **)calloc((unsigned)(wblklen(wcp) + 1),
				(unsigned)sizeof (char *));
	register char **backcp = cp;
	register wchar_t **tmpwcp = wcp;

	while(*tmpwcp){
		wcstombs(mbsbuf, *tmpwcp, sizeof (mbsbuf));
		*cp = savestr(mbsbuf);
		tmpwcp++;
		cp++;
	}
	*cp = 0;
	return(backcp);
}
