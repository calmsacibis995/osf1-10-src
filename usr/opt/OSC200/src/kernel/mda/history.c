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
static char	*sccsid = "@(#)$RCSfile: history.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:17 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include "mda.h"
#include <ctype.h>

static	int	hptr;
static	int	nxt;
static	char	*history[MAXHISTORY][MAXHISTLEN];

extern	char	*nxtarg();
extern	char	cinext[];
static	dobang();

dohist(cmd, arglist)
char	*cmd, *arglist;
{
	int	len;

	len = strlen(cmd);
	strcpy(history[hptr], cmd);
	strcat(history[hptr], " ");
	strncat(history[hptr], arglist, MAXHISTLEN - len - 2);

	hptr = ++hptr % MAXHISTORY;
	nxt++;
}

history_cmd(arglist)
char	*arglist;
{
	char	*ptr, *p;
	int	i, j, len;

	p = arglist;
	dohist("history", arglist);
	if((ptr = nxtarg(&p, 0))) {
		len = atoi(ptr);
		if(len < 1 || len > MAXHISTORY)
			len = MAXHISTORY;
	} else
		len = MAXHISTORY;

	for(i = hptr - 1, j = nxt; i != hptr && len > 0; i--, j--, len--) {
		if(i < 0)
			i = MAXHISTORY - 1;
		if(history[i][0] == '\0')
			break;
		printf("%3d\t%s\n", j, history[i]);
	}
}

bang_cmd(arglist)
char	*arglist;
{
	dobang(0, arglist);
}

bangbang_cmd(arglist)
{
	dobang(1, arglist);
}

static
dobang(last, arglist)
int	last;
char	*arglist;
{
	char	*ptr, *p, *q;
	int	i, j, len;

	p = arglist;
	ptr = nxtarg(&p, 0);

	if(last || ptr == 0) {
		i = hptr - 1;
		if(i < 0)
			i = MAXHISTORY - 1;
		p = arglist;
	} else {
		if(isdigit(*ptr)) {
			i = atoi(ptr);
			if(i >= nxt || i < 1 || nxt - MAXHISTORY > i)
				goto notfound;
			i = hptr - (nxt - i) - 1;
			if(i < 0)
				i = MAXHISTORY + i;
		} else  {
			len = strlen(ptr);
			/*
			 * Zap trailing space
			 */
			if(*(ptr+len-1) == ' ') {
				len--;
				*(ptr+len) = '\0';
			}
			for(i = hptr - 1; i != hptr; i--) {
				if(i < 0)
					i = MAXHISTORY - 1;
				if(history[i][0] == '\0')
					goto notfound;
				if(*ptr == '\0')
					goto notfound;
				q = history[i];
				while(*q) {
					if(*ptr == *q) {
						if(strncmp(ptr, q, len) == 0)
							goto found;
					}
					q++;
				}
			}
			goto notfound;
		}
	}
found:
	ptr = nxtarg(&ptr, 0);
	if(*ptr == '^') {
		dosub(history[i], ptr);
		return;
	}
		
	/*
	 * history[i] contains the command we want to execute.
	 * p has a pointer to the remainder of the argument list.
	 */
	printf("%s %s\n", history[i], p);
	strcpy(cinext, history[i]);
	strcat(cinext, " ");
	strcat(cinext, p);
	return;

notfound:
	printf("%s: Event not found\n", arglist);
	return;
}

dosub(orig, new)
char	*orig, *new;
{
	int	len, didsub;
	char	*p;
	char	*old, *sub;

	p = new + 1;
	old = p;
	while(*p && *p != '^')
		p++;
	if(*p) {
		*p++ = '\0';
		sub = p;
		while(*p && *p != '^')
			p++;
		if(*p)
			*p = '\0';
	} else
		sub = "";

	p = cinext;
	len = strlen(old);
	didsub = 0;
	while(*orig) {
		if(*orig == *old && didsub == 0) {
			if(strncmp(orig, old, len) == 0) {
				while(*sub)
					*p++ = *sub++;
				orig += len;
				didsub++;
			} else
				*p++ = *orig++;
		} else
			*p++ = *orig++;
	}
	*p = '\0';
	printf("%s\n", cinext);
}
