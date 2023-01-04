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
static char	*sccsid = "@(#)$RCSfile: timedc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:33:31 $";
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
 * COMPONENT_NAME: TCPIP timedc.c
 * 
 * FUNCTIONS: MSGSTR, Mtimedc, cmdscanner, getcmd, help, intr, 
 *            makeargv 
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* timedc.c	1.3  com/sockcmd/timed,3.1,9021 4/4/90 12:05:43 */
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "timedc.c	2.7 (Berkeley) 11/20/88";
#endif  not lint */

#include "timedc.h"
#include <signal.h>
#include <ctype.h>
#include <setjmp.h>
#include <sys/syslog.h>

#include "timed_msg.h"
#define MSGSTR(n,s)   NLgetamsg(MF_TIMED, MS_TIMED, n, s)

int	top;
int	margc;
int	fromatty;
char	*margv[20];
char	cmdline[200];
jmp_buf	toplevel;
int	intr();
int priv_resources();
struct	cmd *getcmd();

#include <locale.h>

main(argc, argv)
	char *argv[];
{
	register struct cmd *c;

	openlog("timedc", LOG_ODELAY, LOG_AUTH);
	setlocale(LC_ALL,"");
	tab_load();

	/*
	 * security dictates!
	 */
	if (priv_resources() < 0) {
		fprintf(stderr, MSGSTR(PRIVILEDGE, "Could not get priviledged resources\n"));
		exit(1);
	}
	(void) setuid(getuid());

	if (--argc > 0) {
		c = getcmd(*++argv);
		if (c == (struct cmd *)-1) {
			printf(MSGSTR(AMBIGUOUS, "?Ambiguous command\n"));
			exit(1);
		}
		if (c == 0) {
			printf(MSGSTR(UNKCMD, "?Invalid command\n"));
			exit(1);
		}
		if (c->c_priv && getuid()) {
			printf(MSGSTR(ERRCMD, "?Privileged command\n"));
			exit(1);
		}
		(*c->c_handler)(argc, argv);
		exit(0);
	}
	fromatty = isatty(fileno(stdin));
	top = setjmp(toplevel) == 0;
	if (top)
		(void) signal(SIGINT, (void (*)(int))intr);
	for (;;) {
		cmdscanner(top);
		top = 1;
	}
}

intr()
{
	if (!fromatty)
		exit(0);
	longjmp(toplevel, 1);
}

/*
 * Command parser.
 */
cmdscanner(top)
	int top;
{
	register struct cmd *c;
	extern int help();

	if (!top)
		putchar('\n');
	for (;;) {
		if (fromatty) {
			printf(MSGSTR(TIMEDC, "timedc> "));
			(void) fflush(stdout);
		}
		if (fgets(cmdline, sizeof(cmdline), stdin) == 0)
			quit();
		if (cmdline[0] == 0)
			break;
		makeargv();
		if (margv[0] == NULL)
			continue;
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf(MSGSTR(AMBIGUOUS, "?Ambiguous command\n"));
			continue;
		}
		if (c == 0) {
			printf(MSGSTR(UNKCMD, "?Invalid command\n"));
			continue;
		}
		if (c->c_priv && getuid()) {
			printf(MSGSTR(ERRCMD, "?Privileged command\n"));
			continue;
		}
		(*c->c_handler)(margc, margv);
	}
	longjmp(toplevel, 0);
}

struct cmd *
getcmd(name)
	register char *name;
{
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;
	extern struct cmd cmdtab[];
	extern int NCMDS;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; c < &cmdtab[NCMDS]; c++) {
		p = c->c_name;
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return(c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return((struct cmd *)-1);
	return(found);
}

/*
 * Slice a string up into argc/argv.
 */
makeargv()
{
	register char *cp;
	register char **argp = margv;

	margc = 0;
	for (cp = cmdline; *cp;) {
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
	}
	*argp++ = 0;
}

#define HELPINDENT (sizeof ("directory"))

/*
 * Help command.
 */
help(argc, argv)
	int argc;
	char *argv[];
{
	register struct cmd *c;
	extern struct cmd cmdtab[];

	if (argc == 1) {
		register int i, j, w;
		int columns, width = 0, lines;
		extern int NCMDS;

		printf(MSGSTR(CMDS, "Commands may be abbreviated.  Commands are:\n\n"));
		for (c = cmdtab; c < &cmdtab[NCMDS]; c++) {
			int len = strlen(c->c_name);

			if (len > width)
				width = len;
		}
		width = (width + 8) &~ 7;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				printf("%s", c->c_name);
				if (c + lines >= &cmdtab[NCMDS]) {
					printf("\n");
					break;
				}
				w = strlen(c->c_name);
				while (w < width) {
					w = (w + 8) &~ 7;
					putchar('\t');
				}
			}
		}
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf(MSGSTR(ERRHELP, "?Ambiguous help command %s\n"), arg);
		else if (c == (struct cmd *)0)
			printf(MSGSTR(ERRHELP2, "?Invalid help command %s\n"), arg);
		else
			printf("%-*s\t%s\n", HELPINDENT,
				c->c_name, c->c_help);
	}
}
