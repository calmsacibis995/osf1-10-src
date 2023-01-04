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
static char	*sccsid = "@(#)$RCSfile: lpc.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1993/10/08 15:14:19 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
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
 *
 * lpc.c	5.6 (Berkeley) 6/30/88
 * lpc.c	4.1 15:58:43 7/19/90 SecureWare 
 */


/*
 * lpc -- line printer control program
 */
#include <sys/secdefines.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <setjmp.h>
#include <syslog.h>
#include <locale.h>
#include "lpc.h"
#include "printer_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)
#if SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_PRINTER_SEC,n,s)
#endif SEC_BASE

int	fromatty;

#define MAXLINE 256
char	cmdline[MAXLINE];

int	margc;
char	*margv[20];
int	top;
void	intr();
struct	cmd *getcmd();

jmp_buf	toplevel;

#if SEC_BASE
uid_t	lp_uid;
gid_t	lp_gid;
#endif

int	abort(), clean(), enable(), disable(), down(), help();
int	quit(), restart(), start(), status(), stop(), topq(), up();

static char *msglookup (int);

main(argc, argv)
	char *argv[];
{
	register struct cmd *c;
	extern char *name;
	
        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_PRINTER,NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	/*
	 * In case we end up creating any status or lock files,
	 * set our umask to 0.
	 */
	umask(0);

	lp_uid = pw_nametoid("lp");
	if (lp_uid == (uid_t) -1) {
		fprintf(stderr, MSGSTR_SEC(USERUNDEF, "%s: user \"lp\" not defined\n"), "lpc");
		exit(1);
	}
	lp_gid = gr_nametoid("lp");
	if (lp_gid == (gid_t) -1) {
		fprintf(stderr, MSGSTR_SEC(GROUPUNDEF, "%s: group \"lp\" not defined\n"), "lpc");
		exit(1);
	}
#if SEC_MAC
	if (mand_init()) {
		fprintf(stderr, MSGSTR_SEC(INITLBL,
			"%s: cannot initialize for sensitivity labels\n"),
			"lpc");
		exit(1);
	}
#endif
#endif

	name = argv[0];
	openlog("lpd", 0, LOG_LPR);

	if (--argc > 0) {
		c = getcmd(*++argv);
		if (c == (struct cmd *)-1) {
			fprintf(stderr, MSGSTR(LPC_1, "?Ambiguous command\n"));
			exit(1);
		}
		if (c == 0) {
			fprintf(stderr, MSGSTR(LPC_2, "?Invalid command\n"));
			exit(1);
		}
#if SEC_BASE
		if (!checkauth(c))
#else
		if (c->c_priv && getuid())
#endif
		{
			fprintf(stderr, MSGSTR(LPC_3, "?Privileged command\n"));
			exit(1);
		}
		exit((*c->c_handler)(argc, argv));
	}
	fromatty = isatty(fileno(stdin));
	top = setjmp(toplevel) == 0;
	if (top)
		signal(SIGINT, intr);
	for (;;) {
		cmdscanner(top);
		top = 1;
	}
	exit(0);
}

void intr()
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
	register int i, ch;

	if (!top)
		putchar('\n');
	for (;;) {
		if (fromatty) {
			printf(MSGSTR(LPC_4, "lpc> "));
			fflush(stdout);
		}

		for (i = 0; i < MAXLINE - 1 && (ch = getc(stdin)) != EOF && ch != '\n'; i++)
			cmdline[i] = ch;
		cmdline[i] = '\0';

		if (i == 0)
			quit();
		if (cmdline[0] == 0)
			break;

		makeargv();
		if (margc == 0)
			continue;

		c = getcmd(margv[0]);

		if (c == (struct cmd *)-1) {
			fprintf(stderr, MSGSTR(LPC_1, "?Ambiguous command\n"));
			continue;
		}
		if (c == 0) {
			fprintf(stderr, MSGSTR(LPC_2, "?Invalid command\n"));
			continue;
		}
#if SEC_BASE
		if (!checkauth(c))
#else
		if (c->c_priv && getuid())
#endif
		{
			fprintf(stderr,
				MSGSTR(LPC_3, "?Privileged command\n"));
			continue;
		}
		(*c->c_handler)(margc, margv);
	}

	longjmp(toplevel, 0);
}

extern struct cmd cmdtab[];

struct cmd *
getcmd(name)
	register char *name;
{
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; p = c->c_name; c++) {
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

#define HELPINDENT (sizeof (MSGSTR(LPC_5, "directory")))

/*
 * Help command.
 */
help(argc, argv)
	int argc;
	char *argv[];
{
	register struct cmd *c;

	if (argc == 1) {
		register int i, j, w;
		int columns, width = 0, lines;
		extern int NCMDS;

		printf(MSGSTR(LPC_6,
			"Commands may be abbreviated.  Commands are:\n\n"));
		for (c = cmdtab; c < &cmdtab[NCMDS - 1]; c++) {
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
				if (c + lines >= &cmdtab[NCMDS-1]) {
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
			fprintf(stderr, MSGSTR(LPC_7,
				"?Ambiguous help command %s\n"), arg);
		else if (c == (struct cmd *)0)
			fprintf(stderr, MSGSTR(LPC_8,
				"?Invalid help command %s\n"), arg);
		else
			printf("%-*s\t%s\n", HELPINDENT, c->c_name,
				msglookup(c->c_help));
	    }
}

static char *msglookup (index)
    int index;
{
    switch (index)
    {
    case CMDTAB_ABORTHELP:
	return(MSGSTR(CMDTAB_ABORTHELP,
            "terminate a spooling daemon immediately and disable printing"));
    case CMDTAB_CLEANHELP:
	return(MSGSTR(CMDTAB_CLEANHELP, "remove cruft files from a queue"));
    case CMDTAB_ENABLEHELP:
	return (MSGSTR(CMDTAB_ENABLEHELP, "turn a spooling queue on"));
    case CMDTAB_DISABLEHELP:
	return(MSGSTR(CMDTAB_DISABLEHELP, "turn a spooling queue off"));
    case CMDTAB_DOWNHELP:
	return(MSGSTR(CMDTAB_DOWNHELP,
            "do a 'stop' followed by 'disable' and put a message in status"));
    case CMDTAB_HELPHELP:
	return(MSGSTR(CMDTAB_HELPHELP, "get help on commands"));
    case CMDTAB_QUITHELP:
	return(MSGSTR(CMDTAB_QUITHELP, "exit lpc"));
    case CMDTAB_RESTARTHELP:
	return(MSGSTR(CMDTAB_RESTARTHELP,
            "kill (if possible) and restart a spooling daemon"));
    case CMDTAB_STARTHELP:
	return(MSGSTR(CMDTAB_STARTHELP,
            "enable printing and start a spooling daemon"));
    case CMDTAB_STATUSHELP:
	return(MSGSTR(CMDTAB_STATUSHELP, "show status of daemon and queue"));
    case CMDTAB_STOPHELP:
	return(MSGSTR(CMDTAB_STOPHELP,
"stop a spooling daemon after current job completes and disable printing"));
    case CMDTAB_TOPQHELP:
	return(MSGSTR(CMDTAB_TOPQHELP, "put job at top of printer queue"));
    case CMDTAB_UPHELP:
	return(MSGSTR(CMDTAB_UPHELP,
            "enable everything and restart spooling daemon"));
    default:
	return(MSGSTR(LPC_9,"Unknown help Message"));
    }
}

#if SEC_BASE
checkauth(c)
	register struct cmd	*c;
{
	if (c->c_priv == (char *) 0)	/* no authorization needed */
		return 1;
	
	if (c->c_hasauth == -1)		/* authorization not yet checked */
		c->c_hasauth = authorized_user(c->c_priv);
	return c->c_hasauth;
}
#endif
