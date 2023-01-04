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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: env.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/11 16:46:32 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
  *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * env.c	1.12  com/cmd/sh/env.c, cmdsh, bos320, 9125320 6/6/91 17:40:25	
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "env_msg.h"
#include <errno.h>
#include <unistd.h>

extern	char **environ;

char	*nvmatch(register char *, register char *); 
int	printvar(register char *);
void	addname(register char *), print(void), usage(int);

#define NENV	250		/* max # of env vars we handle	*/
char	*newenv[NENV+1];	/* our new environment		*/
char	*nullp = NULL;		/* a null environment		*/

/*
 * NAME:	env, printenv
 *
 * SYNTAX:	env [-] [name] [name=value] ... [command arg...]
 *		printenv [name]
 *
 * FUNCTION:	Both env and printenv can be used to print out the
 *		current environment or a particular environment
 *		variable.  If called without any arguments, both
 *		simply print out all environment variables.  If
 *		called with 1 argument, and that argument matches
 *		an environment variable name, the environment
 *		variable name and its value is printed.
 *
 *		Env can be used to set environment variables, then
 *		execute a command with the new environment.  Arguments
 *		of the form "name=value" are added to the current
 *		environment before "command" is executed.  If "-" is
 *		specified, env ignores the current environment and
 *		builds a new environment from scratch using the
 *		"name=value" argument.
 *
 * RETURN VALUE DESCRIPTION:	this program returns 0 if no errors
 *		occurred, else it returns 129 
 */

int
main(int argc, char *argv[], char *envp[])
{
	register char *progname;
	register int   printenv;
	register int   i, c;

	(void) setlocale (LC_ALL, "");

	/*
	 * get the program name minus the path
	 */
	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	printenv = (strcmp(progname, "printenv") == 0);

	/*
	 * handle special meaning of "-"
	 */
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-") == 0) {
			argv[i] = "-i";
		}
	}

	/*
	 * check for Posix "-i" argument
	 */
	opterr = 0;
	while ((c = getopt(argc, argv, "i")) != -1) {
		switch (c) {
		    case 'i':
			if (printenv) {
				usage(PUSAGE);	
				exit(1);
			}
			/* start with a null environment */
			envp = &nullp;
			break;
		default:
			usage(printenv ? PUSAGE : EUSAGE);
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	if (printenv)
	{
		/*
		 * printenv specific ...
		 */
		if (argc == 0)
			/*
			 * just print the environment if no arguments given
			 */
			print();

		else if (argc == 1)                         
		{
			/*
			 * print out a particular value
			 */
			if (!printvar(argv[0]))
				exit(1);
		}

		else
		{
			/*
			 * can only run with 1 or no arguments
			 */
			usage(PUSAGE);
			exit(1);
		}
	}

	else
	{		
		/*
		 * env specific ...
		 */

		/*
		 * put current environment into new environment
		 */
		for (; *envp != NULL; envp++)
			if (strchr(*envp, '=') != NULL)
				addname(*envp);

		/*
		 * add "name=value" arguments
		 */
		while (*argv != NULL && strchr(*argv, '=') != NULL)
			addname(*argv++);

		environ = newenv;	/* reset environment */

		if (*argv == NULL)
			/*
			 * just print out environment if no arguments given
			 */
			print();

		/*
		 * if there's only one argument left, try to print it.
		 * if it's not in our environment, execute it
		 */
		else if (*(argv + 1) != NULL || !printvar(*argv))
		{
			/*
			 * execute a command
			 */
			(void) execvp(*argv, argv);

			perror(*argv);
			if ( errno == ENOENT )
				exit(127);
			else
				exit(126);
		}
	}

	exit(0);
}

/*
 * NAME:  addname
 *
 * FUNCTION:  adds a variable and its value to the environment. 
 */

void
addname(register char *arg)
{
	register char **p;

	for (p = newenv; *p != NULL; p++)	/* search for variable */
	{
		if (p >= &newenv[NENV-1])	/* environment full... */
		{
			usage(OVERFLOW);
			print();
			exit(1);
		}

		if (nvmatch(arg, *p) != NULL)	/* match? */
			break;
	}

	*p = arg;		/* add it */
}

/*
 * NAME:  print
 *
 * FUNCTION: prints out the entire environment
 *	     (variable names and their value).
 *	     
 */

void
print(void)
{
	register char **e;

	for (e = environ; *e != NULL; e++)
		(void) puts(*e);
}

/*
 *	s1 is either name, or name=value
 *	s2 is name=value
 *	if names match, return value of s2, else NULL
 */

char *
nvmatch(register char *s1, register char *s2)
{

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return(s2);

	if (*s1 == '\0' && *(s2-1) == '=')
		return(s2);

	return(NULL);
}

/*
 * NAME: printvar
 * 
 * FUNCTION:  searches for the value of name and then it prints out the value.
 *
 * RETURN: returns 1 if we found and printed "name=value", else 0
 */

int
printvar(register char *name)
{
	register char **e, *value;

	for (e = environ; *e != NULL; e++)	/* search enviroment */
		if ((value = nvmatch(name, *e)) != NULL)
		{
			printf ("%s\n", value);
			return(1);
		}

	return(0);
}

/*
 * NAME:  usage
 *
 * FUNCTION: display the correct usage of this command.
 *
 */

void
usage(int error)
{
	char msgstr[40];
	nl_catd cat;

	cat = catopen(MF_ENV,NL_CAT_LOCALE);

	switch(error)
	{
		case OVERFLOW:
			strcpy(msgstr,"too many values in environment\n");
			break;
		case PUSAGE:
			strcpy(msgstr,"Usage: printenv [name]\n");
			break;
		case EUSAGE:
			strcpy(msgstr,"Usage: env [-] [-i] [name=value] [command [args..]]\n");
			break;
	}

	(void) fputs(catgets(cat, MS_ENV, error, msgstr), stderr);
	catclose(cat);
}
