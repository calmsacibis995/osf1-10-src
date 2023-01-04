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
static char	*sccsid = "@(#)$RCSfile: users.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/11 19:31:03 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * users.c	1.3  com/cmd/stat,3.1,9021 12/4/89 16:41:48
 *
 * users - List the login names of the users currently on
 *         the system in a compact, one-line format.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <utmp.h>
#include <stdlib.h>
#include <locale.h>
#include <nl_types.h>
#include "users_msg.h"

#define NMAX 		sizeof(utmp.ut_name)
/* Safety factor used in allocating user table.  DAL001 */
#define MINUSERS	100


nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_USERS,num,str)  

#ifndef __STDC__
#define const
#endif /* __STDC__ */


static void	summary();
static void	putline();
int scmp(char **p, char **q);
extern struct utmp *getutent();

struct utmp utmp;
struct utmp *utmp_ptr;		/* read pointer */
char	**names;		/* names table.  DAL001 */
int	ncnt;			/* count of names */
int	maxusers;		/* Maximum number of names.  DAL001 */
char	**namp;			/* pointer to names table.  DAL001 */


main(argc, argv)
char **argv;
int argc;
{
	struct stat statbuf;	/* DAL001 */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_USERS, NL_CAT_LOCALE);

/*
 * Determine size of utmp file.  DAL001
 */
	if(stat(UTMP_FILE, &statbuf) < 0) {	/* DAL001 */
		fputs(MSGSTR(NO_FILE, "users: unable to open utmp file.\n"),	/* DAL001 */
			stderr);		/* DAL001 */
		exit(1);			/* DAL001 */
	}					/* DAL001 */
/*
 * Determine number of utmp entries in utmp file.  DAL001
 */
	maxusers = MINUSERS + (statbuf.st_size / (sizeof (struct utmp)));	/* DAL001 */
/*
 * Allocate a pointer table somewhat large than the number of entries
 * in the umtp file.  DAL001
 */
	if(!(names=calloc(maxusers, sizeof (char *)))) {	/* DAL001 */
		fputs(MSGSTR(CALLOC, "users: unable to calloc memory.\n"),	/* DAL001 */
			stderr);		/* DAL001 */
		exit(1);	/* DAL001 */
	}			/* DAL001 */
	namp = names;		/* DAL001 */
	setutent();
	while ((utmp_ptr = getutent()) != NULL) {
		if (utmp_ptr->ut_name[0] == '\0' || 
		    utmp_ptr->ut_type != USER_PROCESS)
			continue;
		if (++ncnt > maxusers) {	/* DAL001 */
			ncnt = maxusers;	/* DAL001 */
			fputs(MSGSTR(TOO_MANY, "users: too many users.\n"),
			      stderr);
			break;
		}
		putline();
	}
	endutent();
	summary();
	exit(0);
}

/*
 *  NAME:  putline
 *
 *  FUNCTION:  A valid user name was identified from the utmp file.
 *		Store the users name in memory to be sorted later.
 *	      
 *  RETURN VALUE:  	 none
 */

static void
putline()
{
	char temp[NMAX+1];

	strncpy(temp, utmp_ptr->ut_name, NMAX);
	temp[NMAX] = 0;
	*namp = malloc((size_t)(strlen(temp) + 1));
	if (namp == NULL) {
		perror("malloc");
		exit(1);
	}
	strcpy(*namp++, temp);
}

/*
 *  NAME:  scmp
 *
 *  FUNCTION:  Compare the first elements in two arrays of strings.
 *	      
 *  RETURN VALUE:  	 0   - equal
 *			 1   - not
 */

scmp(char **p, char **q)
{
	return(strcmp(*p, *q));
}

/*
 *  NAME:  summary
 *
 *  FUNCTION:  Sort all the users alphabetacally and print them out.
 *	      
 *  RETURN VALUE:  	 none
 */

static void
summary()
{
	register char **p;

	qsort((void *)names, 
		(size_t)ncnt, 
		(size_t)sizeof(names[0]), 
		(int(*)(const void *, const void *))scmp);
	for (p=names; p < namp; p++) {
		if (p != names)
			putchar(' ');
		fputs(*p, stdout);
	}
	if (namp != names)		/* at least one user */
		putchar('\n');
}
