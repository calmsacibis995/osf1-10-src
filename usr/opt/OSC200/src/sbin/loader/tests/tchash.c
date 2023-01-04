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
static char	*sccsid = "@(#)$RCSfile: tchash.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:07 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* tchash.c
 * Test chained hash functions
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
#include "chain_hash.h"

#define	MAXARGS		3		/* must also change sscanf in getcmd() */


extern	int	cmd_create(), cmd_destroy();
extern	int	cmd_search(), cmd_insert(), cmd_lookup(), cmd_delete();
extern	int	cmd_elements();
extern	int	cmd_quit(), cmd_help();

extern	void	print_node();

extern	char	*strdup();

struct cmd {
	char *cmd_name;
	int	cmd_args;
	int	(*cmd_func)();
	char *cmd_help;
} cmds[] = {
	{ "create", 1, cmd_create, "create <size>" },
	{ "destroy", 0, cmd_destroy, "destroy" },
	{ "search", 3, cmd_search, "search <key> <value> <action>" },
	{ "insert", 2, cmd_insert, "insert <key> <value>" },
	{ "lookup", 1, cmd_lookup, "lookup <key>" },
	{ "delete", 1, cmd_delete, "delete <key>" },
	{ "elements", 0, cmd_elements, "elements" },
	{ "quit", 0, cmd_quit, "quit" },
	{ "help", 0, cmd_help, "help" },
	{ "?", 0, cmd_help, "?" },
	{ NULL, 0, NULL, NULL }
};
extern	struct	cmd	*getcmd();
extern	int	errno;

typedef struct	anelem {
	struct	anelem	*next;
	char	*key;
	char	*val;
} anelem;

chain_hashtab_t	table = NULL;		/* the table */


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
	int	rc;

	if (sscanf(args[0], "%d", &size) != 1) {
		fprintf(stderr, "Bad size %s\n", args[0]);
		return;
	}
	if ((rc = chain_hash_create(size, (ldr_hash_p)hash_string, 
				   (ldr_hash_compare_p)strcmp, &table)) < 0) {
		fprintf(stderr, "create error %d\n", rc);
		return;
	}

	printf("table at 0x%x\n", table);
}

cmd_destroy(args, narg)
char *args[];
int narg;
{
	anelem	*prev, *elem;
	int	rc;

	while (chain_hash_elements(table, NULL, (chain_hash_elem **)&elem) >= 0) {
		printf("deleting key %s\n", elem->key);
		chain_hash_delete(table, elem->key);
	}

	if ((rc = chain_hash_destroy(table)) < 0) {
		fprintf(stderr, "destroy error %d\n", rc);
		return;
	}

	table = NULL;
}

cmd_search(args, narg)
char *args[];
int narg;
{
	anelem *elem;
	ldr_hash_action	action;
	int	rc;

	if ((action = getaction(args[2])) < 0) {
		fprintf(stderr, "bad action %s\n", args[2]);
		return;
	}
	elem = (anelem *)malloc(sizeof (anelem));
	elem->key = strdup(args[0]);
	elem->val = strdup(args[1]);

	if ((rc = chain_hash_search(table, (univ_t)elem->key,
				    (chain_hash_elem **)&elem, action)) < 0) {
		fprintf(stderr, "search error %d\n", rc);
		return;
	}

	printf("search succeeded, value = %s\n", elem->val);
}


cmd_insert(args, narg)
char *args[];
int narg;
{
	anelem *elem;
	int	rc;

	elem = (anelem *)malloc(sizeof (anelem));
	elem->key = strdup(args[0]);
	elem->val = strdup(args[1]);

	if ((rc = chain_hash_insert(table, elem->key,
				   (chain_hash_elem **)&elem)) < 0) {
		fprintf(stderr, "insert error %d\n", rc);
		return;
	}

	printf("insert succeeded\n");
}


cmd_lookup(args, narg)
char *args[];
int narg;
{
	anelem *elem;
	int	rc;

	if ((rc = chain_hash_lookup(table, (univ_t)args[0],
				    (chain_hash_elem **)&elem)) < 0) {
		fprintf(stderr, "lookup error %d\n", rc);
		return;
	}

	printf("lookup succeeded, value = %s\n", elem->val);
}


cmd_delete(args, narg)
char *args[];
int narg;
{
	int	rc;

	if ((rc = chain_hash_delete(table, (univ_t)args[0])) < 0) {
		fprintf(stderr, "lookup error %d\n", rc);
		return;
	}

	printf("delete succeeded\n");
}


cmd_elements(args, narg)
char *args[];
int narg;
{
	anelem	*prev, *elem;
	int	rc;

 	for (prev = NULL; ; prev = elem) {
		if ((rc = chain_hash_elements(table, (chain_hash_elem *)prev,
					      (chain_hash_elem **)&elem)) < 0)
			return;
		printf("key = %s, val = %s\n", elem->key, elem->val);
	}
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
		{ "delete", LDR_HASH_DELETE },
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
