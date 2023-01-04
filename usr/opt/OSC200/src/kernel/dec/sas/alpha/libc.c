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
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

/*
 * libc.c -- routines extracted from c library
 */

#include <sys/types.h>
#include <machine/rpb.h>

#ifdef SECONDARY

/*
 * Concatenate s2 on the end of s1.  S1's space must be large enough.
 * Return s1.
 */

char *
strcat(s1, s2)
register char *s1, *s2;
{
	register char *os1;

	os1 = s1;
	while (*s1++)
		;
	--s1;
	while (*s1++ = *s2++)
		;
	return(os1);
}

/*
 * Copy string s2 to s1.  s1 must be large enough.
 * return s1
 */

char *
strcpy(s1, s2)
register char *s1, *s2;
{
	register char *os1;

	os1 = s1;
	while (*s1++ = *s2++)
		;
	return(os1);
}

/*
 * Check for existence of character c in string s.
 */

char *
strchr(s, c)
char *s, c;
{
	for (; *s; s++)
		if (*s == c)
			return(s);
	return((char *)0);
}

unsigned
getcputype()
{
	struct rpb *rpb = (struct rpb *)HWRPB_ADDR;
	return(rpb->rpb_systype);
}

/*
 * Routine:  exit
 *
 * An error has been encountered that prevents further progress.  Return
 * control back over to the console subsystem.
 */

extern int prom_io;
exit()
{
	prom_close(prom_io);
	stop();
}

#endif SECONDARY

bzero(dst, bcount)
char *dst;
int bcount;
{
    while (bcount--) {
	*dst++ = 0;
    }
}

/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

strcmp(s1, s2)
register char *s1;
register char *s2;
{

	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return(0);
	return(*s1 - *--s2);
}

/*
 * Returns the number of
 * non-NULL bytes in string argument.
 */

strlen(s)
register char *s;
{
	register n;

	n = 0;
	while (*s++)
		n++;
	return(n);
}
