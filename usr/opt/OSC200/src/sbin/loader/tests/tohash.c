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
static char	*sccsid = "@(#)$RCSfile: tohash.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:36 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* tohash.c
 * Test open hash functions
 *
 * OSF/1 Release 1.0
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <loader.h>

#include "ldr_types.h"
#include "ldr_hash.h"
#include "open_hash.h"

#define	MAXARGS		3		/* must also change sscanf in getcmd() */


extern	int	cmd_create(), cmd_destroy(), cmd_resize();
extern	int	cmd_search(), cmd_insert(), cmd_lookup(), cmd_elements();
extern	int	cmd_quit(), cmd_help();

extern	void	print_node();

extern	char	*strdup();

struct cmd {
	char *cmd_name;
	int	cmd_args;
	int	(*cmd_func)();
	char *cmd_help;
} cmds[] = {
	{ "create", 2, cmd_create, "create <size> <flags>" },
	{ "destroy", 0, cmd_destroy, "destroy" },
	{ "resize", 1, cmd_resize, "resize <size>" },
	{ "search", 3, cmd_search, "search <key> <value> <action>" },
	{ "insert", 2, cmd_insert, "insert <key> <value>" },
	{ "lookup", 1, cmd_lookup, "lookup <key>" },
	{ "elements", 0, cmd_elements, "elements" },
	{ "quit", 0, cmd_quit, "quit" },
	{ "help", 0, cmd_help, "help" },
	{ "?", 0, cmd_help, "?" },
	{ NULL, 0, NULL, NULL }
};
extern	struct	cmd	*getcmd();
extern	int	errno;

open_hashtab_t	table = NULL;		/* the table */


main()
{
	struct	cmd	*cmdp;
	char	*args[MAXARGS];
	int	nargs;

	for (;;) {
		if ((cmdp = getcmd(args, &nargs)) == NULL)
			break;
		(*cmdp->cmd_func)(args, nargs);
	}
	printf("Exiting...\n");
	exit(0);
}


struct cmd *
getcmd(args, pnargs)
char	*args[];
int	*pnargs;
{
	struct cmd *cmdp, *cand;
	int	pref;
	static	char	line[128];
	int	i;
	char	*cmdname;
	char	*nextarg;

	for (;;) {
		printf("cmd> ");
		if (gets(line) == NULL)
			break;

		cmdname = line;
		for (i = 0, nextarg = line; i < MAXARGS && nextarg != 0; i++) {
			if ((nextarg = index(nextarg, ' ')) != 0) {
				*nextarg++ = '\0';
				args[i] = nextarg;
				*pnargs = i + 1;
			}
		}

		cand = NULL;
		for (cmdp = cmds; cmdp->cmd_name != NULL; cmdp++) {
			pref = prefix(cmdp->cmd_name, cmdname);
			if (pref == 0)
				goto found;
			if (pref < 0)
				if (cand != NULL) {
					fprintf(stderr, "Ambiguous command name\n");
					goto usage;
				} else
					cand = cmdp;
		}

		if (cand == NULL)
			goto usage;
		cmdp = cand;

found:
		if (*pnargs < cmdp->cmd_args) {
			fprintf(stderr, "usage: %s\n", cmdp->cmd_help);
			continue;
			}
		return(cmdp);

usage:
		fprintf(stderr, "Type '?' for help\n");
	}
	return(NULL);
}


int
prefix(st1, st2)
char *st1;
char *st2;
{
	while (*st1 != '\0' && *st2 != '\0') {
		if (*st1 != *st2)
			return(1);
		st1++;
		st2++;
	}
	if (*st2 != 0)
		return(1);
	return((*st1 == 0) ? 0 : -1);
}


cmd_quit(args, narg)
char *args[];
int narg;
{
	printf("Exiting...\n");
	exit(0);
}


cmd_help(args, narg)
char *args[];
int narg;
{
	struct cmd *cmdp;

	printf("Commands are:\n");
	for (cmdp = cmds; cmdp->cmd_name != NULL; cmdp++)
		printf("%s\n", cmdp->cmd_help);
}


cmd_create(args, narg)
char *args[];
int narg;
{
	int	size;
	open_hash_flags_t	flags;
	int	rc;

	if (sscanf(args[0], "%d", &size) != 1) {
		fprintf(stderr, "Bad size %s\n", args[0]);
		return;
	}
	if ((flags = getflags(args[1])) < 0) {
		fprintf(stderr, "Bad flags %s\n", args[1]);
		return;
	}

	if ((rc = open_hash_create(size, (ldr_hash_p)hash_string, 
				   (ldr_hash_compare_p)strcmp, flags,
				   &table)) < 0) {
		fprintf(stderr, "create error %d\n", rc);
		return;
	}

	printf("table at 0x%x\n", table);
}

cmd_destroy(args, narg)
char *args[];
int narg;
{
	int	rc;

	if ((rc = open_hash_destroy(table)) < 0) {
		fprintf(stderr, "destroy error %d\n", rc);
		return;
	}

	table = NULL;
}

cmd_resize(args, narg)
char *args[];
int narg;
{
	int	size;
	int	rc;

	if (sscanf(args[0], "%d", &size) != 1) {
		fprintf(stderr, "Bad size %s\n", args[0]);
		return;
	}

	if ((rc = open_hash_resize(&table, size)) < 0) {
		fprintf(stderr, "resize error %d\n", rc);
		return;
	}

	printf("resize successful, new table = 0x%x\n", table);
}

cmd_search(args, narg)
char *args[];
int narg;
{
	char *key, *val;
	ldr_hash_action	action;
	int	rc;

	if ((action = getaction(args[2])) < 0) {
		fprintf(stderr, "bad action %s\n", args[2]);
		return;
	}
	key = strdup(args[0]);
	val = strdup(args[1]);

	if ((rc = open_hash_search(table, (univ_t)key, (univ_t *)&val, action)) < 0) {
		fprintf(stderr, "search error %d\n", rc);
		return;
	}

	printf("search succeeded, value = %s\n", val);
}


cmd_insert(args, narg)
char *args[];
int narg;
{
	char *key, *val;
	int	rc;

	key = strdup(args[0]);
	val = strdup(args[1]);

	if ((rc = open_hash_insert(table, (univ_t)key, (univ_t)val)) < 0) {
		fprintf(stderr, "insert error %d\n", rc);
		return;
	}

	printf("insert succeeded\n");
}


cmd_lookup(args, narg)
char *args[];
int narg;
{
	char *key, *val;
	int	rc;

	if ((rc = open_hash_lookup(table, (univ_t)args[0], (univ_t *)&val)) < 0) {
		fprintf(stderr, "lookup error %d\n", rc);
		return;
	}

	printf("lookup succeeded, value = %s\n", val);
}


cmd_elements(args, narg)
char *args[];
int narg;
{
	char	*key, *val;
	open_hash_element_index	ix;
	int	rc;

	for (ix = 0; (rc = open_hash_elements(table, &ix, (univ_t *)&key, (univ_t *)&val)) >= 0; )
		printf("key = %s, val = %s\n", key, val);
}


open_hash_flags_t
getflags(arg)
char *arg;
{
	int	flags;
	char	*p, *fstr;

	/* Flags format is: flag,flag,flag,...
	 * where flags supported are as listed below.
	 */
	static struct flag {
		char *flag_name;
		int	flag_value;
	} all_flags[] = {
		{ "none", 0 },
		{ "rebalance", OPEN_HASH_REBALANCE },
		{ NULL, 0 },
	};
	struct flag *flagp, *cand;
	int pref, done;

	flags = 0;
	for (p = arg, done = 0; !done;) {

		while (*p == ',')
			p++;
		fstr = p;
		while (*p != '\0' && *p != ',')
			p++;
		if (*p == '\0')
			done = 1;
		*p++ = '\0';

		cand = NULL;
		for (flagp = all_flags; flagp->flag_name != NULL; flagp++) {
			pref = prefix(flagp->flag_name, fstr);
			if (pref == 0)
				goto found;
			if (pref < 0)
				if (cand != NULL) {
					fprintf(stderr, "Ambiguous flag name %s\n", fstr);
					return(-1);
				} else
					cand = flagp;
		}

		if (cand == NULL) {
			fprintf(stderr, "unknown flag %s\n", fstr);
			return(-1);
		}
		flagp = cand;
found:
		flags |= flagp->flag_value;
	}

		
	return(flags);
}

ldr_hash_action
getaction(arg)
char *arg;
{
	int	flags;
	char	*p, *fstr;

	/* Flags format is: flag,flag,flag,...
	 * where flags supported are as listed below.
	 */
	static struct flag {
		char *flag_name;
		ldr_hash_action	flag_value;
	} all_flags[] = {
		{ "none", 0 },
		{ "lookup", LDR_HASH_LOOKUP },
		{ "insert", LDR_HASH_INSERT },
		{ NULL, 0 },
	};
	struct flag *flagp, *cand;
	int pref, done;

	flags = 0;
	for (p = arg, done = 0; !done;) {

		while (*p == ',')
			p++;
		fstr = p;
		while (*p != '\0' && *p != ',')
			p++;
		if (*p == '\0')
			done = 1;
		*p++ = '\0';

		cand = NULL;
		for (flagp = all_flags; flagp->flag_name != NULL; flagp++) {
			pref = prefix(flagp->flag_name, fstr);
			if (pref == 0)
				goto found;
			if (pref < 0)
				if (cand != NULL) {
					fprintf(stderr, "Ambiguous flag name %s\n", fstr);
					return(-1);
				} else
					cand = flagp;
		}

		if (cand == NULL) {
			fprintf(stderr, "unknown flag %s\n", fstr);
			return(-1);
		}
		flagp = cand;
found:
		flags |= flagp->flag_value;
	}

		
	return(flags);
}

char *
strdup(str)
char *str;
{
	char	*res;
	extern	void	*malloc(int size);

	if ((res = malloc(strlen(str)+1)) == NULL)
		return(NULL);
	strcpy(res, str);
	return(res);
}
