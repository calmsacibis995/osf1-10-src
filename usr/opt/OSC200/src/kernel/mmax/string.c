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
static char	*sccsid = "@(#)$RCSfile: string.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:07 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
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
/*
 *	String manipulation support.  Grabbed these files from
 *	libc.
 *
 *	For now, use C routines, although assembly language should be
 *	considerably faster (fewer instructions, and much tigher inner
 *	loops).   Assembler must be rechecked before trying to use it.
 */

/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

strcmp(s1, s2)
register char *s1, *s2;
{

	while (*s1 == *s2++)
		if (*s1++=='\0')
			return(0);
	return(*s1 - *--s2);
}

/* Attempt at assembly language

	movd	r4,tos		# save temporary.
	movd	r0,r2		# r1 = s2, r2 = s1
	movd	$0x7fffffff,r0	# Ridiculously large number
	movqd	$0,r4		# Look for null terminator
	cmpsb	[u]		# compare the strings
	movxbd	0(r2),r0	# last elt of s1
	movxbd	0(r1),r1	# last elt of s2
	subd	r1,r0		# return (*s1 - *s2)
	movd	tos,r4		# restore temporary
	ret	$0
*/

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

/* Attempt at assembly language

	enter	[r0,r4], $0	# save temporary, original s1
	movd	r0,r2		# r1 = s2 [from], r2 = s1 [to]
	movd	0x7fffffff,r0	# Ridiculously large number
	movqd	$0,r4		# Look for null terminator
	movsb	[u]		# move the strings
	exit	[r0,r4]		# restore temporary, original s1 (returned)
	ret	$0
*/

/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */

char *
strncpy(s1, s2, n)
register char *s1, *s2;
{
	register i;
	register char *os1;

	os1 = s1;
	for (i = 0; i < n; i++)
		if ((*s1++ = *s2++) == '\0') {
			while (++i < n)
				*s1++ = '\0';
			return(os1);
		}
	return(os1);
}

/* Attempt at assembly language (messy)

	enter	[r0,r4], $0	# save temporary, original s1
	movd	r0,r2		# r1 = s2 [from], r2 = s1 [to]
	movd	8(fp),r0	# number of elements
	movqd	$0,r4		# Look for null terminator
	movsb	[u]		# move strings
	bfc	cpydone		# If no terminator, done.
	movd	r0,r1		# remaining bytes in s1
	movd	r2,r0		# address of what's left
	jsr	_bzero		# Clear it.
cpydone:
	exit	[r0,r4], $0	# restore temporary, original s1
	ret	$0
*/
