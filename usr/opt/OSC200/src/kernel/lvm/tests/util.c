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
static char	*sccsid = "@(#)$RCSfile: util.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:31 $";
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
#include <ctype.h>
#include <stdio.h>

#include "utilproto.h"

/* Character string manipulation/tokenizing routines */
char *
skipwhite(char *s)
{
	while (s && isspace(*s)) s++;

	return(s);
}

char *
splitword(char *s)
{
	/* skip to first whitespace */
	if (s) {
		while (*s && !isspace(*s)) s++;
		/* s points to first whitespace character, or NULL  */
		if (*s) {
			*s++ = '\0';
			/* s points to first char beyond word*/
			return(s);
		}
		/* s is pointing to the NULL at the EOS */
	}
	return(NULL);
}

char *
nextword(char *s)
{
	if ((s=splitword(s)) && (s=skipwhite(s)) && (*s))
		return(s);
	else
		return(NULL);
}
