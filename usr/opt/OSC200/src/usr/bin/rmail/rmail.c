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
static char	*sccsid = "@(#)$RCSfile: rmail.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/21 16:20:47 $";
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
 * COMPONENT_NAME: CMDMAILX rmail.c
 * 
 * FUNCTIONS: MSGSTR, Mrmail 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *  Sendmail
 *  Copyright (c) 1983  Eric P. Allman
 *  Berkeley, California
 *
 *  Copyright (c) 1983 Regents of the University of California.
 *  All rights reserved.  The Berkeley software License Agreement
 *  specifies the terms and conditions for redistribution.
 *  Copyright (c) 1980 Regents of the University of California.
 *  All rights reserved.
 *	rmail.c	5.1 (Berkeley) 6/7/85
 */

/*
**  RMAIL -- UUCP mail server.
**
**	This program reads the >From ... remote from ... lines that
**	UUCP is so fond of and turns them into something reasonable.
**	It calls sendmail giving it a -f option built from these
**	lines.
*/

# include <stdio.h>
# include "sysexits.h"

#include "rmail_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_RMAIL,n,s) 

typedef char	bool;
#define TRUE	1
#define FALSE	0

extern char	*strchr();
extern char	*strrchr();

bool	Debug;

# define MAILER	"/usr/sbin/sendmail"

main(argc, argv)
	char **argv;
{
	FILE *out;	/* output to sendmail */
	char lbuf[512];	/* one line of the message */
	char from[512];	/* accumulated path of sender */
	char ufrom[64];	/* user on remote system */
	char sys[64];	/* a system in path */
	char junk[512];	/* scratchpad */
	char cmd[2000];
	register char *cp;
	register char *uf;	/* ptr into ufrom */
	int i, ifrom, nouucp;
        int donotdo=1; /* Do not do if loop when "From" is not specified(mik)*/

	catd = catopen(MF_RMAIL,NL_CAT_LOCALE);

# ifdef DEBUG
	if (argc > 1 && strcmp(argv[1], "-T") == 0)
	{
		Debug = TRUE;
		argc--;
		argv++;
	}
# endif DEBUG

	if (argc < 2)
	{
		fprintf(stderr, MSGSTR(USAGE, "Usage: rmail user ...\n")); /*MSG*/
		exit(EX_USAGE);
	}

	(void) strcpy(from, "");
	(void) strcpy(ufrom, "/dev/null");

	for (;;)
	{
		(void) fgets(lbuf, sizeof lbuf, stdin);
		if (strncmp(lbuf, "From ", 5) != 0 && strncmp(lbuf, ">From ", 6) != 0)
		{
		        donotdo=0;		/*mik*/
                        break;
                }
/* We have a UNIX from line. Is it uucp??????      */
		(void) sscanf(lbuf, "%s %s", junk, ufrom);
		cp = lbuf;
		uf = ufrom;
		for (;;)
		{
			cp = strchr(cp+1, 'r');
			if (cp == NULL)
			{
				register char *p = strrchr(uf, '!');

				if (p != NULL)
				{
					*p = '\0';
					(void) strcpy(sys, uf);
					uf = p + 1;
					break;
				}
/* Have a from line but no uucp indicators. 		*/
				nouucp++;
				break;
			}
#ifdef DEBUG
			if (Debug)
				printf("cp='%s'\n", cp);
#endif
			if (strncmp(cp, "remote from ", 12)==0)
				break;
		}
		if (cp != NULL)
			{
			(void) sscanf(cp, "remote from %s", sys);
			(void) strcat(from, sys);
			(void) strcat(from, "!");
			}
		if (nouucp) break;
#ifdef DEBUG
		if (Debug)
			printf("ufrom='%s', sys='%s', from now '%s'\n", uf, sys, from);
#endif
	}
        if(uf && donotdo)			/*mik*/
		(void) strcat(from, uf);  /* append last "From" user to return path */

/* Check to see if any from was encountered. This was necessary to
   allow rmail to be invoked interactively. Why anyone would do this
   is still a ??????			*/

	ifrom=strlen(from);
/* Normal uucp call.					*/
	if (ifrom)
	(void) sprintf(cmd, "%s -ee -f%s", MAILER, from);
/* Weird from encountered -of preserves it in body of letter.	*/
/* Or no from (unix) style encountered				*/
	else
	(void) sprintf(cmd, "%s -ee -of", MAILER);
	while (*++argv != NULL)
	{
		(void) strcat(cmd, " '");
		if (**argv == '(')
			(void) strncat(cmd, *argv + 1, strlen(*argv) - 2);
		else
			(void) strcat(cmd, *argv);
		(void) strcat(cmd, "'");
	}
#ifdef DEBUG
	if (Debug)
		printf("cmd='%s'\n", cmd);
#endif
	out = popen(cmd, "w");
	fputs(lbuf, out);
	while (fgets(lbuf, sizeof lbuf, stdin))
		fputs(lbuf, out);
	i = pclose(out);
	if ((i & 0377) != 0)
	{
		fprintf(stderr, MSGSTR(PCLOSE, "pclose: status 0%o\n"), i); /*MSG*/
		exit(EX_OSERR);
	}

	exit((i >> 8) & 0377);
}
