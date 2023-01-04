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
static char *rcsid = "@(#)$RCSfile: str.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:05:51 $";
#endif

/*-
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include "make.h"

/*-
 * Str_Concat --
 *	concatenate the two strings, inserting a space or slash between them,
 *	freeing them if requested.
 *
 * returns --
 *	the resulting string in allocated space.
 */
char *
Str_Concat(char *s1, char *s2, int flags)
{
	/* char *s1, *s2; */
	/* int flags; */

	register int len1, len2;
	char *result;

	/* get the length of both strings */
	len1 = strlen(s1);
	len2 = strlen(s2);

	/* allocate length plus separator plus EOS */
	result = (char *)emalloc((u_int)(len1 + len2 + 2));

	/* copy first string into place */
	bcopy(s1, result, len1);

	/* add separator character */
	if (flags & STR_ADDSPACE) {
		result[len1] = ' ';
		++len1;
	} else if (flags & STR_ADDSLASH) {
		result[len1] = '/';
		++len1;
	}

	/* copy second string plus EOS into place */
	bcopy(s2, result + len1, len2 + 1);

	/* free original strings */
	if (flags & STR_DOFREE) {
		(void)free(s1);
		(void)free(s2);
	}
	return(result);
}

/*-
 * Str_Break --
 *	Fracture a string into an array of words (as delineated by tabs or
 *	spaces) taking quotation marks into account.  Leading tabs/spaces
 *	are ignored.
 *
 * returns --
 *	Pointer to the array of pointers to the words.  To make life easier,
 *	the first word is always the value of the .MAKE variable.
 */
char **
Str_Break(char *prog, char *str, int *store_argc)
{
	/* char *prog; */
	/* char *str; */
	/* int *store_argc; */

	static int argmax, curlen;
	static char **argv, *buf;
	register int argc, ch;
	register char inquote, *p, *start, *t;
	int len;

	/* save off pmake variable */
	if (!argv) {
		argv = (char **)emalloc((argmax = 50) * sizeof(char *));
	}
	argv[0] = prog;

	/* skip leading space chars */
	for (; *str == ' ' || *str == '\t'; ++str);

	/* allocate room for a copy of the string */
	if ((len = strlen(str) + 1) > curlen)
		buf = (char *)emalloc(curlen = len);

	/*
	 * copy the string; at the same time, parse backslashes,
	 * quotes and build the argument list.
	 */
	argc = 1;
	inquote = '\0';
	for (p = str, start = t = buf;; ++p) {
		switch(ch = *p) {
		case '"':
		case '\'':
			if (inquote)
				if (inquote == ch)
					inquote = '\0';
				else
					break;
			else
				inquote = ch;
			continue;
		case ' ':
		case '\t':
			if (inquote)
				break;
			if (!start)
				continue;
			/* FALLTHROUGH */
		case '\n':
		case '\0':
			/*
			 * end of a token -- make sure there's enough argv
			 * space and save off a pointer.
			 */
			*t++ = '\0';
			if (argc == argmax) {
				argmax *= 2;		/* ramp up fast */
				if (!(argv = (char **)realloc(argv,
				    argmax * sizeof(char *))))
				enomem();
			}
			argv[argc++] = start;
			start = (char *)NULL;
			if (ch == '\n' || ch == '\0')
				goto done;
			continue;
		case '\\':
			switch (ch = *++p) {
			case '\0':
			case '\n':
				/* hmmm; fix it up as best we can */
				ch = '\\';
				--p;
				break;
			case 'b':
				ch = '\b';
				break;
			case 'f':
				ch = '\f';
				break;
			case 'n':
				ch = '\n';
				break;
			case 'r':
				ch = '\r';
				break;
			case 't':
				ch = '\t';
				break;
			}
			break;
		}
		if (!start)
			start = t;
		*t++ = ch;
	}
done:	argv[argc] = (char *)NULL;
	if (prog == 0)
		argc--;
	if (store_argc)
		*store_argc = argc;
	if (prog == 0)
		return(&argv[1]);
	return(argv);
}

/*
 * Str_FindSubstring -- See if a string contains a particular substring.
 * 
 * Results: If string contains substring, the return value is the location of
 * the first matching instance of substring in string.  If string doesn't
 * contain substring, the return value is NULL.  Matching is done on an exact
 * character-for-character basis with no wildcards or special characters.
 * 
 * Side effects: None.
 */
char *
Str_FindSubstring(char *string, char *substring)
{
	/* char *string;		String to search. */
	/* char *substring;		Substring to find in string */

	register char *a, *b;

	/*
	 * First scan quickly through the two strings looking for a single-
	 * character match.  When it's found, then compare the rest of the
	 * substring.
	 */

	for (b = substring; *string != 0; string += 1) {
		if (*string != *b)
			continue;
		a = string;
		for (;;) {
			if (*b == 0)
				return(string);
			if (*a++ != *b++)
				break;
		}
		b = substring;
	}
	return((char *) NULL);
}
