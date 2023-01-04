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
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getttyent.c,v $ $Revision: 4.2.5.2 $ (OSF) $Date: 1993/06/07 23:04:03 $";
#endif
/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * getttyent.c	5.4 (Berkeley) 5/19/86
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endttyent_r = __endttyent_r
#pragma weak getttyent_r = __getttyent_r
#pragma weak setttyent_r = __setttyent_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak endttyent = __endttyent
#pragma weak getttyent = __getttyent
#pragma weak setttyent = __setttyent
#endif
#endif
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <ttyent.h>
#include <fcntl.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE

#define	TTY_FP		(*tty_fp)

#define	SETTTYENT(f)	setttyent_r(f)
#define	ENDTTYENT()	endttyent_r(&tty_fp)
#define	TTYSCAN(t, l)	ttyscan(t, l, len, TTY_FP)

#else

#define MAXLINELENGTH	1024

#define	TTY_FP		tty_fp

#define	SETTTYENT(f)	setttyent()
#define	ENDTTYENT()	endttyent()
#define	TTYSCAN(t, l)	ttyscan(t, l, MAXLINELENGTH, TTY_FP)

static struct ttyent	tty_ttys;
static FILE		*tty_fp = NULL;
static char		*line;		/* [MAXLINELENGTH]; */

#endif	/* _THREAD_SAFE */

#define	TTY_FILE	"/etc/securettys"
#define	QUOTED		1


/*
 * Skip over the current field, removing quotes,
 * and return a pointer to the next field.
 */
static char *
skip(register char *p, char *zap)
{
	register char *t = p;
	register int c;
	register int q = 0;

	for (; (c = *p) != '\0'; p++) {
		if (c == '"') {
			q ^= QUOTED;	/* obscure, but nice */
			continue;
		}
		if (q == QUOTED && *p == '\\' && *(p+1) == '"')
			p++;
		*t++ = *p;
		if (q == QUOTED)
			continue;
		if (c == '#') {
			*zap = c;
			*p = 0;
			break;
		}
		if (c == '\t' || c == ' ' || c == '\n') {
			*zap = c;
			*p++ = 0;
			while ((c = *p) == '\t' || c == ' ' || c == '\n')
				p++;
			break;
		}
	}
	*--t = '\0';
	return (p);
}


static char *
value(register char *p)
{
	if ((p = index(p,'=')) == 0)
		return(NULL);
	p++;			/* get past the = sign */
	return(p);
}

#define	GETTY_NONE	"none"
#define TYPE_NONE	"none"

static int
ttyscan(struct ttyent *tte, char *line, int len, FILE *fp)
{
	register char	*p;
	register int	c;
	char		zapchar;

	for (;;) {
		if (!(p = fgets(line, len, fp)))
			return (0);

		/* skip lines that are too big */
		if (!index(line, '\n')) {
			while ((c = getc(fp)) != '\n' && c != EOF)
				;
			continue;
		}
		while (isspace(*p))
			++p;
		if (*p && *p != '#')
			break;
	}

	/* fill in the dummy (i.e. unused fields) */
	tte->ty_getty = GETTY_NONE;
	tte->ty_type = TYPE_NONE;
	tte->ty_window = NULL;
	tte->ty_comment = NULL;

	/* a device name in the /etc/securettys file implies it is secure */
	zapchar = 0;
	tte->ty_name = p;
	p = skip(p, &zapchar);
	tte->ty_status = 0; 
	tte->ty_status &= ~TTY_ON;
	tte->ty_status |= TTY_SECURE;

	return (1);
}


#ifdef _THREAD_SAFE
int
getttyent_r(struct ttyent *ttyent, char *line, int len, FILE **tty_fp)
{
#else
struct ttyent *
getttyent(void)
{
	register struct ttyent	*ttyent = &tty_ttys;

	if (!line && !(line = malloc(MAXLINELENGTH)))
		return (0);
#endif	/* _THREAD_SAFE */

	TS_EINVAL((tty_fp == 0 || ttyent == 0 || line == 0 || len <= 0));

	if (!TTY_FP && SETTTYENT(tty_fp) != TS_SUCCESS
	    || !TTYSCAN(ttyent, line))
		return (TS_NOTFOUND);
	return (TS_FOUND(ttyent));
}


#ifdef _THREAD_SAFE
int
setttyent_r(FILE **tty_fp)
#else
int
setttyent(void)
#endif	/* _THREAD_SAFE */
{
	int	flags;

	TS_EINVAL((tty_fp == 0));

	if (TTY_FP) {
		rewind(TTY_FP);
		return (TS_SUCCESS);
	}

	if ((TTY_FP = fopen(TTY_FILE, "r")) != NULL) {
		flags = fcntl(fileno(TTY_FP), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(TTY_FP), F_SETFD, flags) == 0)
			return (TS_SUCCESS);
		(void)fclose(TTY_FP);
	}
	return (TS_FAILURE);
}


#ifdef _THREAD_SAFE
void
endttyent_r(FILE **tty_fp)
#else
void
endttyent(void)
#endif	/* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (tty_fp == 0) return;
#endif	/* _THREAD_SAFE */

	if (TTY_FP) {
		(void)fclose(TTY_FP);
		TTY_FP = NULL;
	}
}
