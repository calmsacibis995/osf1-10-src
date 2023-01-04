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
static char	*sccsid = "@(#)$RCSfile: tload2.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:29 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* tload2.c
 * Test loader calls
 *
 * Conditionals:
 *   KTLOAD2				build kernel loader client (using IPC)
 *   KTLOAD2+CALL_SERVER_DIRECTLY	build kernel loader server
 *   LDR_STATIC_LINK			statically link in loader
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

#include <loader.h>
#include "ldr_types.h"
#include "ldr_errno.h"
#include "ldr_lock.h"

#ifdef	KTLOAD2
#include <sys/kloadcall.h>
#include <sys/param.h>
#include "ldr_errno.h"

char kmodname[MAXPATHLEN];

int ldr_absolute_pathname(const char *relname, char *buf, size_t buflen);
#endif

#define	MAXARGS		6			/* must also change sscanf in getcmd() */


extern	int	cmd_load(), cmd_getentry(), cmd_unload(), cmd_callentry();
extern 	int	cmd_info(), cmd_next_module(), cmd_inq_module(), cmd_exec();
extern	int	cmd_context(), cmd_global(), cmd_atexit();
extern	int	cmd_inq_region(), cmd_install(), cmd_remove(), cmd_lookup_pkg();
extern	int	cmd_dump(), cmd_quit(), cmd_help();
extern	void	onbus();
#ifdef	LOOKUP
extern	int	cmd_lookup();
#endif

struct cmd {
	char *cmd_name;
	int	cmd_args;
	int	(*cmd_func)();
	char *cmd_help;
} cmds[] = {
	{ "load", 2, cmd_load, "load <file> <flags>" },
	{ "getentry", 1, cmd_getentry, "getentry <module>" },
	{ "callentry", 1, cmd_callentry, "callentry <module>" },
	{ "info", 0, cmd_info, "info" },
	{ "next_module", 1, cmd_next_module, "next_module <module>" },
	{ "inq_module", 1, cmd_inq_module, "inq_module <module>" },
	{ "inq_region", 2, cmd_inq_region, "inq_region <module> <region>" },
#ifdef	LOOKUP
	{ "lookup", 2, cmd_lookup, "lookup <module> <sym>" },
#endif
	{ "lookup_package", 2, cmd_lookup_pkg, "lookup_package <pkg> <sym>" },
	{ "unload", 1, cmd_unload, "unload <module>" },
#ifndef	KTLOAD2
	{ "install", 1, cmd_install, "install <file>" },
	{ "remove", 1, cmd_remove, "remove <file>" },
	{ "exec", 0, cmd_exec, "exec" },
	{ "context", 0, cmd_context, "context" },
	{ "global", 1, cmd_global, "global <file>" },
	{ "atexit", 0, cmd_atexit, "atexit" },
#endif
	{ "dump", 2, cmd_dump, "dump <addr> <len>" },
	{ "quit", 0, cmd_quit, "quit" },
	{ "help", 0, cmd_help, "help" },
	{ "?", 0, cmd_help, "?" },
	{ NULL, 0, NULL, NULL }
};
extern	struct	cmd	*getcmd();

ldr_process_t process;
void *ldr_process_context;

#ifdef LDR_STATIC_LINK

/* Definition of standard loader global data file */

/* structure to save locking functions used by loader */

lib_lock_functions_t	ldr_lock_funcs;

/* This is the only loader lock for this process */

ldr_lock_t ldr_global_lock;

const char *ldr_global_data_file = "/tmp/ldr_global.dat";
const char *ldr_dyn_database = "/tmp/ldr_dyn_mgr.conf";

#endif /* LDR_STATIC_LINK */

#ifdef	KTLOAD2
extern int errno;
#ifdef LDR_STATIC_LINK
extern int ldr_kernel_bootstrap(char *);
#endif /* LDR_STATIC_LINK */
#else
#ifdef LDR_STATIC_LINK
extern int ldr_bootstrap(char *, void **);
#endif /* LDR_STATIC_LINK */
#endif

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

#ifdef	KTLOAD2
#ifdef	CALL_KERNEL_DIRECTLY
	if ((rc = ldr_kernel_bootstrap("/vmunix")) != LDR_SUCCESS) {
		fprintf(stderr, "ldr_kernel_bootstrap failed %d\n", rc);
		exit(1);
	}
#endif
	process = ldr_kernel_process();
	(void)ldr_xattach(process);
#else
#ifdef LDR_STATIC_LINK
	if ((rc = ldr_bootstrap(argv[0], &ldr_process_context)) != LDR_SUCCESS) {
		fprintf(stderr, "ldr_bootstrap failed %d\n", rc);
		exit(1);
	}
#endif /* LDR_STATIC_LINK */
	process = ldr_my_process();
	(void)ldr_xattach(process);
#endif

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
	(void)ldr_xdetach(process);
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
		{ "main", LDR_MAIN },
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

#ifdef	KTLOAD2
	if ((rc = ldr_absolute_pathname(file, kmodname,
	    sizeof(kmodname))) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("cannot canonicallize filename");
		return;
	}
	file = kmodname;
#endif

	if ((rc = ldr_xload(process, file, flags, &module)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("load error");
	} else
		printf("load of %s successful, module = %d\n", file, module);
}

cmd_getentry(args, narg)
char *args[];
int narg;
{
	ldr_module_t	module;
	ldr_entry_pt_t	entry;
	int		rc;

	if (sscanf(args[0], "%d", &module) != 1) {
		fprintf(stderr, "bad module %s\n", args[0]);
		return;
	}

	if ((rc = ldr_xentry(process, module, &entry)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_entry error");
	} else
		printf("entry point for module %d at 0x%x\n", module, entry);
	return;
}

cmd_callentry(args, narg)
char *args[];
int narg;
{
	ldr_module_t	module;
	ldr_entry_pt_t	entry;
	int		val;
	int		rc;

	if (sscanf(args[0], "%d", &module) != 1) {
		fprintf(stderr, "bad module %s\n", args[0]);
		return;
	}

	if ((rc = ldr_xentry(process, module, &entry)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_entry error");
		return;
	} else
		printf("entry point for module %d at 0x%x\n", module, entry);

#ifdef	KTLOAD2
	val = kloadcall(KLC_CALL_FUNCTION, entry, 0, 0, 0);
#else
	val = (*entry)();
#endif
	printf("entry returned value %d\n", val);
}

cmd_unload(args, narg)
char *args[];
int narg;
{
	ldr_module_t	module;
	int		rc;

	if (sscanf(args[0], "%d", &module) != 1) {
		fprintf(stderr, "bad module %s\n", args[0]);
		return;
	}

	if ((rc = ldr_xunload(process, module)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_unload error");
	} else
		printf("module %d unloaded\n", module);
	return;
}

#ifdef	LOOKUP
cmd_lookup(args, narg)
char *args[];
int narg;
{
	ldr_module_t	module;
	void		*val;
	int		rc;

	if (sscanf(args[0], "%d", &module) != 1) {
		fprintf(stderr, "bad module %s\n", args[0]);
		return;
	}

	if ((rc = ldr_xlookup(process, module, args[1], &val)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_lookup error");
	} else
		printf("%s: 0x%x\n", args[1], val);
	return;
}
#endif

cmd_lookup_pkg(args, narg)
char *args[];
int narg;
{
	void	*val;
	int	rc;

	if ((rc = ldr_xlookup_package(process, args[0], args[1], &val)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_lookup_package error");
	} else
		printf("%s$%s: 0x%x\n", args[0], args[1], val);
	return;
}

cmd_info(args, narg)
char *args[];
int narg;
{
	ldr_module_t	module;
	ldr_module_info_t minfo;
	ldr_region_t	region;
	ldr_region_info_t rinfo;
	int		rc;
	size_t		size;

	module = LDR_NULL_MODULE;

	for (;;) {

		ldr_next_module(process, &module);
		if (module == LDR_NULL_MODULE)
			break;

		if ((rc = ldr_inq_module(process, module, &minfo, sizeof(minfo),
					 &size)) < 0) {
			errno = ldr_status_to_errno(rc);
			perror("ldr_inq_module error");
			return;
		}

		if (size < sizeof(minfo))
			printf("warning: short module info record %d\n", size);

		dump_minfo(&minfo);

		for (region = 0; region < minfo.lmi_nregion; region++) {

			if ((rc = ldr_inq_region(process, module, region, &rinfo, sizeof(rinfo),
						 &size)) < 0) {
				errno = ldr_status_to_errno(rc);
				perror("ldr_inq_region error");
				return;
			}

			if (size < sizeof(rinfo))
				printf("warning: short info record %d\n", size);

			dump_rinfo(module, &rinfo);
		}
	}
}

cmd_next_module(args, narg)
char *args[];
int narg;
{
	ldr_module_t	module;
	int		rc;

	if (sscanf(args[0], "%d", &module) != 1) {
		fprintf(stderr, "bad module %s\n", args[0]);
		return;
	}

	if ((rc = ldr_next_module(process, &module)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_next_module error");
	} else
		printf("next module id is %d\n", module);
	return;
}

cmd_inq_module(args, narg)
char *args[];
int narg;
{
	ldr_module_t	module;
	ldr_module_info_t info;
	int		rc;
	size_t		size;

	if (sscanf(args[0], "%d", &module) != 1) {
		fprintf(stderr, "bad module %s\n", args[0]);
		return;
	}

	if ((rc = ldr_inq_module(process, module, &info, sizeof(info),
				 &size)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_inq_module error");
		return;
	}

	if (size < sizeof(info))
		printf("warning: short info record %d\n", size);

	dump_minfo(&info);
	return;
}

cmd_inq_region(args, narg)
char *args[];
int narg;
{
	ldr_module_t	module;
	ldr_region_t	region;
	ldr_region_info_t info;
	int		rc;
	size_t		size;

	if (sscanf(args[0], "%d", &module) != 1) {
		fprintf(stderr, "bad module %s\n", args[0]);
		return;
	}

	if (sscanf(args[1], "%d", &region) != 1) {
		fprintf(stderr, "bad region %s\n", args[1]);
		return;
	}

	if ((rc = ldr_inq_region(process, module, region, &info, sizeof(info),
				 &size)) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_inq_region error");
		return;
	}

	if (size < sizeof(info))
		printf("warning: short info record %d\n", size);

	dump_rinfo(module, &info);
	return;
}


dump_minfo(minfo)
ldr_module_info_t *minfo;
{
	printf("%d\t\"%s\"\t(Nregion %d Flags 0x%08x):\n", minfo->lmi_modid,
	       minfo->lmi_name, minfo->lmi_nregion, minfo->lmi_flags);
}

dump_rinfo(mod, rinfo)
ldr_module_t mod;
ldr_region_info_t *rinfo;
{
	printf("\t%d\t%s\n", rinfo->lri_region_no, rinfo->lri_name);
	printf("\t\tVA 0x%08x\tMA 0x%08x\tSZ %8d\tProt 0x%03x\n",
	       rinfo->lri_vaddr, rinfo->lri_mapaddr,
	       rinfo->lri_size, rinfo->lri_prot);
}

cmd_dump(args, narg)
char *args[];
int narg;
{
	unsigned char	*addr;
	int	len;
	int	bad;
	int	i;

	if (sscanf(args[0], "%x", &addr) != 1) {
		bad = 0;
		goto err;
	}
	if (sscanf(args[1], "%d", &len) != 1) {
		bad = 1;
		goto err;
	}

	printf("Dump of %d bytes at 0x%x:", len, addr);
	for (i = 0; i < len; i++, addr++) {
		if ((i % 16) == 0)
			printf("\n%08x    ", addr);
		printf("%02x  ", *addr);
	}
	printf("\n");
	return;

err:
	fprintf(stderr, "bad argument %s\n", args[bad]);
	return;
}


#ifdef	KTLOAD2
/* ldr_absolute_pathname(), from old versions of ldr_sys_int.[ch] */

/* LDR_RELATIVE_PATHNAME returns nonzero if the specified pathname is a
 * relative pathname, zero if it is absolute.
 */

#define LDR_RELATIVE_PATHNAME(name)	(*(name) != '/')

int
ldr_getcwd(char *buf, size_t buflen)

/* Return the current working directory of the loader in buf.  Buflen
 * is the size of the buffer.  Return LDR_SUCCESS on success.
 * If the buffer size is too small, return LDR_ENOSPC; return other
 * negative error status on other errors.
 * NOTE: for the standalone loader this implementation will have to
 * change; getcwd() depends on malloc().  But that's OK, because the
 * standard implementation is pretty damaged.
 */
{
	int	rc;

	if (getwd(buf) != NULL)
		return(LDR_SUCCESS);
	return(-errno);
}


int
ldr_absolute_pathname(const char *relname, char *buf, size_t buflen)

/* Convert a relative pathname into an absolute pathname, in the specified
 * buffer.  Buflen is the size of the buffer.  Returns LDR_SUCCESS on
 * success, negative error status on error.
 */
{
	int	cwdlen;
	int	rellen;
	int	rc;

	rellen = strlen(relname);
	if (!LDR_RELATIVE_PATHNAME(relname)) {

		if (rellen >= buflen)
			return(LDR_ENOSPC);
		(void)strcpy(buf, relname);
		return(LDR_SUCCESS);}
		

	if ((rc = ldr_getcwd(buf, buflen)) != LDR_SUCCESS)
		return(rc);
	if ((rellen + strlen(buf) + 1) >= buflen)
		return(LDR_ENOSPC);
	(void)strcat(buf, "/");
	(void)strcat(buf, relname);
	return(LDR_SUCCESS);
}


#ifdef	CALL_KERNEL_DIRECTLY
int kls_client_ipc_connect_to_server()
	{ return(0;) }
int kls_client_ipc_disconnect_from_server()
	{ return(0}; }
int kls_client_load(a, b, c)
	{ return(kernel_load(a, b, c)); }
int kls_client_unload(a)
	{ return(kernel_unload(a)); }
int kls_client_entry(a, b)
	{ return(kernel_entry(a, b)); }
int kls_client_lookup(a, b, c)
	{ return(kernel_lookup(a, b, c)); }
int kls_client_lookup_package(a, b, c, d)
	{ return(kernel_lookup_package(a, b, c, d)); }
int kls_client_next_module(a)
	{ return(kernel_next_module(a)); }
int kls_client_inq_module(a, b, c, d)
	{ return(kernel_inq_module(a, b, c, d)); }
int kls_client_inq_region(a, b, c, d, e)
	{ return(kernel_inq_region(a, b, c, d, e)); }
#endif	/* CALL_KERNEL_DIRECTLY */

#else	/* KTLOAD2 */

cmd_install(args, narg)
char *args[];
int narg;
{
	int	rc;

	if ((rc = ldr_install(args[0])) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_install failed");
	}
}

cmd_remove(args, narg)
char *args[];
int narg;
{
	int	rc;

	if ((rc = ldr_remove(args[0])) < 0) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_install failed");
	}
}

cmd_context(args, narg)
char *args[];
int narg;
{
	void		*ctxt;
	extern int	preload_alloc_abs(), preload_alloc_rel(), preload_dealloc();
	extern int	ldr_context_create(), ldr_context_boostrap();
	int		rc;

	if ((rc = ldr_context_global_file_remove(ldr_process_context)) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_context_global_file_remove failed");
		return;
	}

	if ((rc = ldr_context_create(10, preload_alloc_abs, preload_alloc_rel,
				     preload_dealloc, &ctxt)) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror("context create failed");
		return;
	}

	if ((rc = ldr_context_bootstrap(ctxt, "")) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror("context bootstrap failed");
		return;
	}

	ldr_process_context = ctxt;
}

cmd_global(args, narg)
char *args[];
int narg;
{
	int	fd;
	int	rc;

	if ((fd = open(args[0], O_CREAT|O_TRUNC|O_RDWR, 0666)) < 0) {
		perror("open failed");
		return;
	}

	if ((rc = ldr_context_global_file_init(ldr_process_context, fd)) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_context_global_file_init failed");
	}
		
	close(fd);
}

cmd_exec(args, narg)
char *args[];
int narg;
{
	execl(prog, prog, 0);
	perror("exec failed");
}

cmd_atexit(args, narg)
char *args[];
int narg;
{
	int	rc;

	if ((rc = ldr_atexit()) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror("ldr_atexit failed");
	}
}

#endif	/* KTLOAD2 */
