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
static char *rcsid = "@(#)$RCSfile: ialloc.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/11/19 16:21:24 $";
#endif

/*LINTLIBRARY*/

#include <stdio.h>

#ifndef alloc_t
#define alloc_t	unsigned
#endif /* !alloc_t */

#ifdef MAL
#define NULLMAL(x)	((x) == NULL || (x) == MAL)
#else /* !MAL */
#define NULLMAL(x)	((x) == NULL)
#endif /* !MAL */

extern char *	calloc();
extern char *	malloc();
extern char *	realloc();
extern char *	strcpy();

char *
imalloc(n)
{
#ifdef MAL
	register char *	result;

	if (n == 0)
		n = 1;
	result = malloc((alloc_t) n);
	return (result == MAL) ? NULL : result;
#else /* !MAL */
	if (n == 0)
		n = 1;
	return malloc((alloc_t) n);
#endif /* !MAL */
}

char *
icalloc(nelem, elsize)
{
	if (nelem == 0 || elsize == 0)
		nelem = elsize = 1;
	return calloc((alloc_t) nelem, (alloc_t) elsize);
}

char *
irealloc(pointer, size)
char *	pointer;
{
	if (NULLMAL(pointer))
		return imalloc(size);
	if (size == 0)
		size = 1;
	return realloc(pointer, (alloc_t) size);
}

char *
icatalloc(old, new)
char *	old;
char *	new;
{
	register char *	result;
	register	oldsize, newsize;

	oldsize = NULLMAL(old) ? 0 : strlen(old);
	newsize = NULLMAL(new) ? 0 : strlen(new);
	if ((result = irealloc(old, oldsize + newsize + 1)) != NULL)
		if (!NULLMAL(new))
			(void) strcpy(result + oldsize, new);
	return result;
}

char *
icpyalloc(string)
char *	string;
{
	return icatalloc((char *) NULL, string);
}

ifree(p)
char *	p;
{
	if (!NULLMAL(p))
		free(p);
}
