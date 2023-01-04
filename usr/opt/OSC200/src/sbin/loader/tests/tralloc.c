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
static char	*sccsid = "@(#)$RCSfile: tralloc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:39 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* tralloc.c
 * Test process region allocation routines.
 *
 * OSF/1 Release 1.0
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/addrconf.h>
#include <loader.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>

#define	MAXARGS		6		/* must also change sscanf in getcmd() */


extern	int	cmd_getaddr(), cmd_alloc_region(), cmd_alloc_picregion();
extern	int	cmd_dealloc_region();
extern	int	cmd_quit(), cmd_help();

struct cmd {
	char *cmd_name;
	int	cmd_args;
	int	(*cmd_func)();
	char *cmd_help;
} cmds[] = {
	{ "getaddr", 0, cmd_getaddr, "getaddr" },
	{ "alloc_region", 3, cmd_alloc_region, "alloc_region <vaddr> <size> <prot>" },
	{ "alloc_picregion", 2, cmd_alloc_picregion, "alloc_picregion <size> <prot>" },
	{ "dealloc_region", 3, cmd_dealloc_region, "dealloc_region <vaddr> <mapaddr> <size>" },
	{ "quit", 0, cmd_quit, "quit" },
	{ "help", 0, cmd_help, "help" },
	{ "?", 0, cmd_help, "?" },
	{ NULL, 0, NULL, NULL }
};
extern	struct	cmd	*getcmd();
extern	int	errno;

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


cmd_getaddr(args, narg)
char *args[];
int narg;
{
	struct addressconf addr_conf[AC_N_AREAS]; 
	int	size;
	int	count;

	if ((size = getaddressconf(addr_conf, sizeof(addr_conf))) < 0) {
		perror("getaddressconf error");
		return;
	}

	count = size / sizeof(struct addressconf);

	dump_ac(addr_conf, count);

}

cmd_alloc_region(args, narg)
char *args[];
int narg;
{
	univ_t			vaddr;
	univ_t			baseaddr;
	size_t			size;
	ldr_prot_t		prot;
	int	rc;

	if (sscanf(args[0], "%x", &vaddr) != 1) {
		fprintf(stderr, "bad vaddr %s\n", args[0]);
		return;
	}
	if (sscanf(args[1], "%d", &size) != 1) {
		fprintf(stderr, "bad size %s\n", args[1]);
		return;
	}
	if ((prot = get_prot(args[2])) == (ldr_prot_t)(-1))
		return;

	rc = alloc_abs_process_region(vaddr, size, prot, &baseaddr);

	if (rc < 0) {
		fprintf(stderr, "alloc_process_region error %d\n", rc);
		return;
	}

	printf("vaddr 0x%x, baseaddr 0x%x\n", vaddr, baseaddr);
}

cmd_alloc_picregion(args, narg)
char *args[];
int narg;
{
	univ_t			vaddr;
	univ_t			baseaddr;
	size_t			size;
	ldr_prot_t		prot;
	int	rc;

	if (sscanf(args[0], "%d", &size) != 1) {
		fprintf(stderr, "bad size %s\n", args[0]);
		return;
	}
	if ((prot = get_prot(args[1])) == (ldr_prot_t)(-1))
		return;

	rc = alloc_rel_process_region(size, prot, &vaddr, &baseaddr);

	if (rc < 0) {
		fprintf(stderr, "alloc_process_region error %d\n", rc);
		return;
	}

	printf("vaddr 0x%x, baseaddr 0x%x\n", vaddr, baseaddr);
}

cmd_dealloc_region(args, narg)
char *args[];
int narg;
{
	univ_t			vaddr;
	univ_t			baseaddr;
	size_t			size;
	int	rc;

	if (sscanf(args[0], "%x", &vaddr) != 1) {
		fprintf(stderr, "bad vaddr %s\n", args[0]);
		return;
	}
	if (sscanf(args[1], "%x", &baseaddr) != 1) {
		fprintf(stderr, "bad baseaddr %s\n", args[1]);
		return;
	}
	if (sscanf(args[2], "%d", &size) != 1) {
		fprintf(stderr, "bad size %s\n", args[2]);
		return;
	}

	rc = dealloc_process_region(vaddr, baseaddr, size);

	if (rc < 0) {
		fprintf(stderr, "alloc_process_region error %d\n", rc);
		return;
	}

	printf("region deallocated\n");
}

int
get_prot(arg)
char *arg;
{
	int	prot;
	int	i;

	if (sscanf(arg, "%x", &prot) == 1)
		return(prot);
	for (i = 0, prot = 0; arg[i] != '\0'; i++)
		switch (arg[i]) {
case 'r':
			prot |= LDR_R;
			break;
case 'w':
			prot |= LDR_W;
			break;
case 'x':
			prot |= LDR_X;
			break;
default:
			return(-1);
		}
	return(prot);
}

dump_ac(ac, count)
struct addressconf *ac;
int count;
{
	int		i;

	for (i = 0; i < count; i++) {

		prname(i);
		printf("base 0x%08x", ac[i].ac_base);
		if ((ac[i].ac_flags & (AC_FIXED|AC_FLOAT)) == AC_FLOAT) {
			printf(" floating\n");
		} else {
			printf(" fixed\n");
		}
	}
}

prname(i)
int i;
{
	static char *regions[] = {
		"AC_TEXT ",	
		"AC_DATA ",	
		"AC_BSS ",	
		"AC_STACK ",
		"AC_LDR_TEXT ",
		"AC_LDR_DATA ",
		"AC_LDR_BSS ",
		"AC_LDR_PRIV ",
		"AC_LDR_GLB ",
		"unused ",
		"AC_MMAP_TEXT ",
		"AC_MMAP_DATA ",
		"AC_MMAP_BSS ",
	};

	if (i >= AC_N_AREAS) {
		printf("region %i ");
	} else {
		printf("%s", regions[i]);
	}
}
