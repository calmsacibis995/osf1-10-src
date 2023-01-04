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
static char rcsid[] = "@(#)$RCSfile: misc.c,v $ $Revision: 4.2.11.3 $ (DEC) $Date: 1993/06/10 16:57:13 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: alnum blkend blkpr blklen 
 *            blkcpy blkcat blkfree saveblk strspl blkspl lastchr closem 
 *            closech donefds dmove dcopy renum copy lshift number copyblk 
 *            strend strip udvar prefix
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.14  com/cmd/csh/misc.c, 9123320, bos320 5/20/91 12:48:49	
 */ 

#include <ctype.h>
#include "sh.h"
#include "local.h"


/* int
 * #ifndef _SBCS
 * alnum(register wint_t c)
 * #else
 * alnum(register uchar_t c)
 * #endif
 * {
 * 	return (letter(c) || digit(c));
 * }
 */

int
any(register wint_t c, register uchar_t *s)
{
	while (*s)
		if (*s++ == c)
			return(1);
	return(0);
}

#ifndef _SBCS
/*
 *	Determine if there is an occurrence of the character "c" in the
 *	string "s".  If an NLQUOTE is encountered in "s", no comparison
 *	is made with the character following it. 
 */
int
any_noquote(register wint_t c, register uchar_t *s)
{
	register int n;		/* # bytes for character */
	wchar_t nlc;		/* character process code */

	while(*s) {
		n = mbtowc(&nlc, (char *)s, mb_cur_max);
		if (n < 0) {
			n = 1;
			nlc = *s & 0xff;
		}
		if (nlc == c)
			return (1);
		s += n;
		if (nlc == NLQUOTE && *s) {
			n = mbtowc(&nlc, (char *)s, mb_cur_max);
			if (n < 0) {
				n = 1;
				nlc = *s & 0xff;
			}
			s += n;
		}
	}
	return (0);
}
#endif

uchar_t **
blkend(register uchar_t **up)
{

	while (*up)
		up++;
	return (up);
}
 
void
blkpr(register uchar_t **av)
{

	for (; *av; av++) {
		csh_printf("%s", *av);
		if (av[1])
			display_char(' ');
	}
}

int
blklen(register uchar_t **av)
{
	register int i = 0;

	while (*av++)
		i++;
	return (i);
}

uchar_t **
blkcpy(uchar_t **oav, register uchar_t **bv)
{
	register uchar_t **av = oav;

	while (*av++ = *bv++)
		continue;
	return (oav);
}

uchar_t **
blkcat(uchar_t **up, uchar_t **vp)
{

	blkcpy(blkend(up), vp);
	return (up);
}

int
blkfree(uchar_t **av0)
{
	register uchar_t **av = av0;

	while (*av)
		xfree(*av++);
	xfree((uchar_t *)av0);
}

uchar_t **
saveblk(register uchar_t **v)
{
	register int len = blklen(v) + 1;
	register uchar_t **newv = (uchar_t **)calloc(len, sizeof (char **));
	uchar_t **onewv = newv;

	while (*v)
		*newv++ = savestr(*v++);
	return (onewv);
}

uchar_t *
strspl(register uchar_t *cp, register uchar_t *dp)
{
	register uchar_t *ep;

	ep = calloc(1, strlen((char *)cp) + strlen((char *)dp) + 1);
	strcpy( (char *)ep, (char *) cp);
	strcat(ep, dp);
	return (ep);
}

uchar_t **
blkspl(register uchar_t **up, register uchar_t **vp)
{
	register uchar_t **wp;

	wp = (uchar_t **)calloc(blklen(up) + blklen(vp) + 1, sizeof(char **));
	blkcpy(wp, up);
	return (blkcat(wp, vp));
}

int
lastchr(register uchar_t *cp)
{
	if (!*cp)
		return (0);
	while (cp[1])
		cp++;
	return (*cp);
}

/*
 * This routine is called after an error to close up
 * any units which may have been left open accidentally.
 */
void
closem(void)
{
	register int f;

#ifdef DEBUG
printf("SHIN=%d, SHOUT=%d, SHDIAG=%d, OLDSTD=%d\n",
		SHIN, SHOUT, SHDIAG, OLDSTD);
#endif
	for (f = 0; f <= open_max; f++)
		if (f != SHIN && f != SHOUT && f != SHDIAG && f != OLDSTD &&
		    f != FSHTTY)
			close(f);
}

/*
 * Close files before executing a file.
 * We could be MUCH more intelligent, since (on a version 7 system)
 * we need only close files here during a source, the other
 * shell fd's being in units 16-19 which are closed automatically!
 */
void
closech(void)
{
	register int f;

	if (didcch)
		return;
	didcch = 1;
	SHIN = 0; SHOUT = 1; SHDIAG = 2; OLDSTD = 0;
	for (f = 3; f <= open_max; f++)
			close(f);
}

void
donefds(void)
{
	int err=0;

	/*
	 * Check for any errors on these file descriptors
	 */
	err += close(0);
	err += close(1);
	err += close(2);
	didfds = 0;
	if (err)
		Perror("close");

}

/*
 * Move descriptor i to j.
 * If j is -1 then we just want to get i to a safe place,
 * i.e. to a unit > 2.  This also happens in dcopy.
 */
int
dmove(register int i, register int j)
{

	if (i == j || i < 0)
		return (i);
	if (j >= 0) {
		dup2(i, j);
                track_open(j);
		return (j);
	} else
		j = dcopy(i, j);
	if (j != i)
		close(i);
	return (j);
}

int
dcopy(register int i, register int j)
{

	if (i == j || i < 0 || j < 0 && i > 2)
		return (i);
	if (j >= 0) {
		j = dup2(i, j);
                track_open(j);
		return (j);
	}
	close(j);
	return (renum(i, j));
}

int
renum(register int i, register int j)
{
	register int k = dup(i);

	if (k < 0)
		return (-1);
	if (j == -1 && k > 2) {
                track_open(k);
		return (k);
	}
	if (k != j) {
		j = renum(k, j);
		close(k);
                track_open(j);
		return (j);
	}
        track_open(k);
	return (k);
}

void
copy(register uchar_t *to, register uchar_t *from, register int size)
{
	if (size)
		do
			*to++ = *from++;
		while (--size != 0);
}

/*
 * Left shift a command argument list, discarding
 * the first c arguments.  Used in "shift" commands
 * as well as by commands like "repeat".
 */
void
lshift(register uchar_t **v, register int c)
{
	register uchar_t **u = v;

	while (*u && --c >= 0)
		xfree(*u++);
	blkcpy(v, u);
}

int
number(uchar_t *cp)
{

	if (*cp == '-') {
		cp++;
		if (!digit(*cp++))
			return (0);
	}
	while (*cp && digit(*cp))
		cp++;
	return (*cp == 0);
}

uchar_t **
copyblk(register uchar_t **v)
{
	register uchar_t **nv ;

	nv = (uchar_t **)calloc((unsigned)(blklen(v) + 1), sizeof(uchar_t **));
	return (blkcpy(nv, v));
}

uchar_t *
strend(register uchar_t *cp)
{

	while (*cp)
		cp++;
	return (cp);
}

uchar_t *
strip(uchar_t *cp)
{
	register uchar_t *dp = cp;
	register uchar_t *xp = cp;
	while (*dp) {
#ifndef _SBCS
		if (*dp == NLQUOTE) 
			if (*++dp==0) break;
		PUTSTR(xp,dp);
#else
		if (*dp == NLQUOTE) {
			if (*++dp==0) break;
			*xp++ = *dp++ & TRIM;
		}
		else
			*xp++ = *dp++;
#endif
	}
	*xp = 0;
	return (cp);
}

void
udvar(uchar_t *name)
{

	setname(name);
	bferr(MSGSTR(M_UNDEF, "Undefined variable"));
}

int
prefix(register uchar_t *sub, register uchar_t *str)
{

	for (;;) {
		if (*sub == 0)
			return (1);
		if (*str == 0)
			return (0);
		if (*sub++ != *str++)
			return (0);
	}
}
