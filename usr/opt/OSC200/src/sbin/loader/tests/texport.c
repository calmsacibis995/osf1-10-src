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
static char	*sccsid = "@(#)$RCSfile: texport.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:23 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* texport.c
 * Test get_exports switch routine
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <loader.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>

#include "ldr_types.h"
#include "ldr_lock.h"
#include "ldr_hash.h"
#include "chain_hash.h"
#include "dqueue.h"
#include "ldr_errno.h"
#include "ldr_malloc.h"
#include "ldr_sys_int.h"

#include "ldr_region.h"
#include "ldr_package.h"
#include "ldr_symbol.h"
#include "ldr_known_pkg.h"
#include "ldr_module.h"
#include "ldr_switch.h"
#include "ldr_global_file.h"

#define	MAXARGS		6			/* must also change sscanf in getcmd() */


extern	int	cmd_load(), cmd_install(), cmd_lookup_pkg(), cmd_get_exports();
extern	int	cmd_quit(), cmd_help();
extern	void	onbus();

struct cmd {
	char *cmd_name;
	int	cmd_args;
	int	(*cmd_func)();
	char *cmd_help;
} cmds[] = {
	{ "load", 2, cmd_load, "load <file> <flags>" },
	{ "lookup_package", 2, cmd_lookup_pkg, "lookup_package <pkg> <sym>" },
	{ "install", 1, cmd_install, "install <file>" },
	{ "get_exports", 1, cmd_get_exports, "get_exports <mod>" },
	{ "quit", 0, cmd_quit, "quit" },
	{ "help", 0, cmd_help, "help" },
	{ "?", 0, cmd_help, "?" },
	{ NULL, 0, NULL, NULL }
};
extern	struct	cmd	*getcmd();

void *ldr_process_context;
/* structure to save locking functions used by loader */

lib_lock_functions_t	ldr_lock_funcs;

/* This is the only loader lock for this process */

ldr_lock_t ldr_global_lock;


/* Definition of standard loader global data file */

const char *ldr_global_data_file = "/tmp/ldr_global.dat";
const char *ldr_dyn_database = "/tmp/ldr_dyn_mgr.conf";


jmp_buf		env;
char		*prog;


main(argc, argv)
int argc;
char **argv;
{
	struct	cmd	*cmdp;
	char	*args[MAXARGS];
	int	nargs;
	int	rc;

	prog = argv[0];

	if ((rc = ldr_bootstrap(argv[0], &ldr_process_context)) < 0) {
		fprintf(stderr, "ldr_bootstrap failed %d\n", rc);
		exit(1);
	}

	if (setjmp(env) != 0)
		printf("Reentering command loop...\n");

	signal(SIGBUS, onbus);
	signal(SIGSEGV, onbus);

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
	char	*index();

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


void
onbus(sig, code, scp)

int	sig;
int	code;
struct	sigcontext	*scp;
{
	psignal(sig, "Faulted; in onbus");
	longjmp(env, 1);
}


cmd_quit(args, narg)
char *args[];
int narg;
{
	printf("Exiting... ");
	putchar('\n');
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


ldr_load_flags_t
getflags(arg)
char *arg;
{
	ldr_load_flags_t flags;
	char	*p, *fstr;

	/* Flags format is: flag,flag,flag,...
	 * where flags supported are as listed below.
	 */
	static struct flag {
		char *flag_name;
		ldr_load_flags_t flag_value;
	} all_flags[] = {
		{ "none", LDR_NOFLAGS },
		{ "noinit", LDR_NOINIT },
		{ "wire", LDR_WIRE },
		{ "nounrefs", LDR_NOUNREFS },
		{ "noprexist", LDR_NOPREXIST },
		{ "exportonly", LDR_EXPORTONLY },
		{ "nounload", LDR_NOUNLOAD },
		{ NULL, 0 },
	};
	struct flag *flagp, *cand;
	int pref, done;

	flags = LDR_NOFLAGS;
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

cmd_load(args, narg)
char *args[];
int narg;
{
	char		*file;
	ldr_load_flags_t flags;
	ldr_module_t	module;
	int		rc;

	file = args[0];
	if ((flags = getflags(args[1])) == (ldr_load_flags_t)(-1)) {
		fprintf(stderr, "bad flags %s\n", args[1]);
		return;
	}

	if ((module = load(file, flags)) < 0)
		perror("load error");
	else
		printf("load of %s successful, module = %d\n", file, module);
}

cmd_lookup_pkg(args, narg)
char *args[];
int narg;
{
	void	*val;
	int	rc;

	if ((val = ldr_lookup_package(args[0], args[1])) == NULL)
		perror("ldr_lookup_package error");
	else
		printf("%s$%s: 0x%x\n", args[0], args[1], val);
	return;
}

cmd_install(args, narg)
char *args[];
int narg;
{
	int	rc;

	if ((rc = ldr_install(args[0])) < 0)
		perror("ldr_install failed");
}

cmd_get_exports(args, narg)
char *args[];
int narg;
{
	int	mod_id;
	ldr_module_rec	*module;
	int	pkg_count;
	ldr_package_rec *pkgs;
	int	sym_count;
	ldr_symbol_rec *syms;
	int	i;
	int	rc;

	if (sscanf(args[0], "%d", &mod_id) != 1) {
		fprintf(stderr, "bad mod id %s\n", args[0]);
		return;
	}

	if ((rc = translate_module_id(ldr_process_context, mod_id,
				      &module)) < 0) {
		fprintf(stderr, "unknown mod id, error %d\n", rc);
		return;
	}

	if ((rc = LSW_GET_EXPORT_PKGS(module, &pkg_count, &pkgs)) < 0) {
		fprintf(stderr, "get_export_pkgs error %d\n", rc);
		return;
	}

	if ((rc = LSW_GET_EXPORTS(module, &sym_count, &syms)) < 0) {
		fprintf(stderr, "get_exports error %d\n", rc);
		return;
	}

	printf("%d symbols returned:\n", sym_count);
	for (i = 0; i < sym_count; i++) {

		
		printf("%-16s%-32s:  ", (syms[i].ls_packageno <= pkg_count) ?
		       pkgs[syms[i].ls_packageno].lp_name : "BAD PKG!",
		       syms[i].ls_name);
		print_value(&syms[i].ls_value);
	}
}


print_value(val)
ldr_symval *val;
{
	if (ldr_symval_is_abs(val)) {
		printf("            0x%08x\n", ldr_symval_abs(val));
	} else if (ldr_symval_is_regrel(val)) {
		printf("Reg %-3d Off 0x%08x\n", val->ls_regno, val->ls_offset);
	} else {
		printf("unrecognized %x %x\n", val->ls_kind, val->ls_abs);
	}
}

int kls_client_ipc_connect_to_server() { return(-1); }
int kls_client_ipc_disconnect_from_server() { return(-1); }
int kls_client_load() { return(-1); }
int kls_client_unload() { return(-1); }
int kls_client_entry() { return(-1); }
int kls_client_lookup() { return(-1); }
int kls_client_lookup_package() { return(-1); }
int kls_client_next_module() { return(-1); }
int kls_client_inq_module() { return(-1); }
int kls_client_inq_region() { return(-1); }
