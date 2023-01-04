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
static char	*sccsid = "@(#)$RCSfile: tnmmap.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:33 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 *	tests.c
 * 	command line tests interface
 *
 * OSF/1 Release 1.0
 */

/*
 *	NOTE: the procedures cmd_ldr_stat and cmd_ldr_fstat donot
 *	free the memory associated with the stat structs they allocate.
 *	Similarly cmd_ldr_read does not free memory allocated for the
 *	buffer into which the data is read.
 */

#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <loader.h>

#include "ldr_types.h"
#include "ldr_malloc.h"
#include "ldr_windows.h"
#include "ldr_sys_int.h"

#define	MAXARGS		6			/* must also change sscanf in getcmd() */


extern	int	cmd_open(), cmd_close(), cmd_fill(), cmd_dump();
extern	int	cmd_mzero(), cmd_mset(), cmd_mclear();
extern	int	cmd_mmap(), cmd_munmap(), cmd_mremap(), cmd_msync(), cmd_mprotect();
extern	int	cmd_madvise(), cmd_mincore(), cmd_mvalid();
extern	int	cmd_fork(), cmd_exec();
extern	int	cmd_quit(), cmd_help();
extern	int	cmd_ldr_mmap(), cmd_ldr_munmap(), cmd_ldr_msync();
extern	int	cmd_ldr_open(), cmd_ldr_close(), cmd_ldr_read();
extern	int	cmd_ldr_write(), cmd_ldr_stat(), cmd_ldr_fstat();
extern	int	cmd_ldr_file_window(), cmd_ldr_init_window(), cmd_ldr_unwindow();
extern	int	cmd_ldr_walk_map_q(), cmd_ldr_walk_lru_q(), cmd_ldr_malloc(), cmd_ldr_free();
extern	int	cmd_heap_create(), cmd_heap_malloc(), cmd_heap_free();
extern	int	cmd_malloc(), cmd_free();
extern	int	cmd_flush_lru_q();
extern	void	onbus();


struct ldr_mapping_t	{
	struct ldr_mapping_t	*LRU;	/* linked list of mappings arranged in
					   least recently used order */
	ldr_file_t	fd;		/* file handle for file mapped */
	int		refcount;	/* no of windows associated with map */
	off_t		start;		/* starting pt. of mapping in file */
	size_t		length;		/* no of bytes of file mapped in */
	mode_t		mode;		/* ? ? */
	char		*data;		/* mapped file bytes */
};

typedef struct ldr_mapping_t ldr_mapping_t;


struct	ldr_hd_tail_queue_t {
	ldr_mapping_t	*sq_head;
	ldr_mapping_t	*sq_tail;
};

extern  struct ldr_hd_tail_queue_t *ldr_map_q;
extern	struct ldr_hd_tail_queue_t *ldr_lru_q;



struct cmd {
	char *cmd_name;
	int	cmd_args;
	int	(*cmd_func)();
	char *cmd_help;
} cmds[] = {
	{ "mmap", 6, cmd_mmap, "mmap <addr> <len> <prot> <flags> <fd> <pos>" },
	{ "munmap", 2, cmd_munmap, "munmap <addr> <len>" },
	{ "mremap", 3, cmd_mremap, "mremap <addr> <len> <pos>" },
	{ "msync", 2, cmd_msync, "msync <addr> <len>" },
	{ "mprotect", 3, cmd_mprotect, "mprotect <addr> <len> <prot>" },
	{ "madvise", 3, cmd_madvise, "madvise <addr> <len> <behav>" },
	{ "mincore", 2, cmd_mincore, "mincore <addr> <len>" },
	{ "mvalid", 3, cmd_mvalid, "mvalid <addr> <len> <prot>" },
	{ "mzero", 1, cmd_mzero, "mzero <semaphore>" },
	{ "mset", 2, cmd_mset, "mset <semaphore> <wait>", },
	{ "mclear", 1, cmd_mclear, "mclear <semaphore>" },
	{ "fill", 3, cmd_fill, "fill <addr> <len> <val>" },
	{ "dump", 2, cmd_dump, "dump <addr> <len>" },
	{ "open", 2, cmd_open, "open <file> <opts>" },
	{ "close", 1, cmd_close, "close <fd>" },
	{ "fork", 0, cmd_fork, "fork" },
	{ "exec", 0, cmd_exec, "exec" },
	{ "quit", 0, cmd_quit, "quit" },
	{ "help", 0, cmd_help, "help" },
	{ "ldr_open", 2, cmd_ldr_open, "ldr_open <file> <opts>" },
	{ "ldr_close", 1, cmd_ldr_close, "ldr_close <fd>" },
	{ "ldr_read", 2, cmd_ldr_read, "ldr_read <fd> <nbytes>" },
	{ "ldr_stat", 1, cmd_ldr_stat, "ldr_stat <file>" },
	{ "ldr_fstat", 1, cmd_ldr_fstat, "ldr_fstat <fd>" },
	{ "ldr_mmap", 6, cmd_ldr_mmap, 
		  "ldr_mmap <addr> <len> <prot> <flags> <fd> <pos>" },
	{ "ldr_munmap", 2, cmd_ldr_munmap, "ldr_munmap <addr> <len>" },
	{ "ldr_msysnc", 3, cmd_ldr_msync, "ldr_msync <addr> <len> <flags>" },
	{ "ldr_window", 3, cmd_ldr_file_window, "ldr_window <start> <len> <wp>" },
	{ "ldr_unwindow", 1, cmd_ldr_unwindow, "ldr_unwindow <wp>" },
	{ "ldr_init_window", 1, cmd_ldr_init_window, "ldr_init_window <wp>" },
	{ "ldr_walk_map_q", 0, cmd_ldr_walk_map_q, "ldr_walk_map_q" },
	{ "ldr_walk_lru_q", 0, cmd_ldr_walk_lru_q, "ldr_walk_lru_q" },
	{ "ldr_flush_lru_q", 0, cmd_flush_lru_q, "ldr_flush_lru_q" },
	{ "ldr_malloc", 1, cmd_ldr_malloc, "ldr_malloc <size>" },
	{ "ldr_free", 1, cmd_ldr_free, "ldr_free <addr>" },
	{ "malloc", 1, cmd_malloc, "malloc <size>" },
	{ "free", 1, cmd_free, "free <addr>" },
	{ "heap_create", 4, cmd_heap_create, "heap_create <addr> <fd> <flags> <offset>" },
	{ "heap_malloc", 2, cmd_heap_malloc, "heap_malloc <heap> <size>" },
	{ "heap_free", 2, cmd_heap_free, "heap_free <heap> <addr>" },
	{ "?", 0, cmd_help, "?" },
	{ NULL, 0, NULL, NULL }
};
extern	struct	cmd	*getcmd();

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

	if ((rc = ldr_heap_init()) < 0) {
		printf("failed to init ldr heap, %d\n", rc);
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


int
getprot(arg)
char *arg;
{
	int	prot;
	int	i;

	if (sscanf(arg, "%x", &prot) == 1)
		return(prot);
	for (i = 0, prot = 0; arg[i] != '\0'; i++)
		switch (arg[i]) {
case 'r':
			prot |= PROT_READ;
			break;
case 'w':
			prot |= PROT_WRITE;
			break;
case 'x':
			prot |= PROT_EXEC;
			break;
default:
			return(-1);
		}
	return(prot);
}

int
getbool(arg)
char *arg;
{
	int	bool;
	int	i;

	if (sscanf(arg, "%d", &bool) == 1)
		return(bool);
	bool = 0;
	if (arg[0] == 't' || arg[0] == 'y')
		bool = 1;
	return(bool);
}

int
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
		{ "shared", MAP_SHARED },
		{ "private", MAP_PRIVATE },
		{ "file", MAP_FILE },
		{ "anon", MAP_ANON },
		{ "fixed", MAP_FIXED },
		{ "hassemaphore", MAP_HASSEMAPHORE },
		{ "inherit", MAP_INHERIT },
		{ "unaligned", MAP_UNALIGNED },
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

cmd_open(args, narg)
char *args[];
int narg;
{
	int	fd;
	int	prot;

	if (strcmp(args[1], "r") == 0)
		prot = O_RDONLY;
	else if (strcmp(args[1], "w") == 0)
		prot = O_WRONLY;
	else if (strcmp(args[1], "rw") == 0)
		prot = O_RDWR;
	else {
		fprintf(stderr, "bad open mode %s\n", args[1]);
		return;
	}
	if ((fd = open(args[0], prot)) < 0) {
		perror("open failure");
		return;
	}

	printf("%s opened on fd %d\n", args[0], fd);
}

cmd_close(args, narg)
char *args[];
int narg;
{
	int	fd;

	if (sscanf(args[0], "%d", &fd) != 1) {
		fprintf(stderr, "bad fd %s!\n", args[0]);
		return;
	}

	close(fd);
}

cmd_fill(args, narg)
char *args[];
int narg;
{
	char	*addr;
	int	len;
	int	val;
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
	if (sscanf(args[2], "%x", &val) != 1) {
		bad = 2;
		goto err;
	}

	for (i = 0; i < len; i++)
		*addr++ = val;
	return;

err:
	fprintf(stderr, "bad argument %s\n", args[bad]);
	return;
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

cmd_mzero(args, narg)
char *args[];
int narg;
{
#ifdef	NOTYET
	semaphore *sem;

	if (sscanf(args[0], "%x", &sem) != 1) {
		fprintf(stderr, "bad addr %s!\n", args[0]);
		return;
	}

	bzero(sem, sizeof(semaphore));
#else
	fprintf(stderr, "not yet implemented\n");
#endif
}

cmd_mset(args, narg)
char *args[];
int narg;
{
#ifdef	NOTYET
	semaphore *sem;
	int	wait;
	int	val;

	if (sscanf(args[0], "%x", &sem) != 1) {
		fprintf(stderr, "bad addr %s!\n", args[0]);
		return;
	}
	wait = getbool(args[1]);

	val = mset(sem, wait);
	printf("Returned val %d\n", val);
#else
	fprintf(stderr, "not yet implemented\n");
#endif
}

cmd_mclear(args, narg)
char *args[];
int narg;
{
#ifdef	NOTYET
	semaphore *sem;

	if (sscanf(args[0], "%x", &sem) != 1) {
		fprintf(stderr, "bad addr %s!\n", args[0]);
		return;
	}

	mclear(sem);
#else
	fprintf(stderr, "not yet implemented\n");
#endif
}

cmd_mmap(args, narg)
char *args[];
int narg;
{
	caddr_t		addr;
	int		len;
	int		prot;
	int		flags;
	int		fd;
	int		pos;
	int		bad;
	caddr_t		paddr;

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }
	if ((prot = getprot(args[2])) < 0) { bad = 2; goto err; }
	if ((flags = getflags(args[3])) < 0) { bad = 3; goto err; }
	if (sscanf(args[4], "%d", &fd) != 1) { bad = 4; goto err; }
	if (sscanf(args[5], "%x", &pos) != 1) { bad = 5; goto err; }

	if ((paddr = mmap(addr, len, prot, flags, fd, pos)) == (caddr_t)(-1)) 
		perror("mmap error");
	else
		printf("mmap successful at 0x%08x, len %d\n", paddr, len);
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
}

cmd_munmap(args, narg)
char *args[];
int narg;
{
	caddr_t		addr;
	int		len;
	int		bad;

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }

	if (munmap(addr, len) != 0) 
		perror("munmap error");
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
}

cmd_mremap(args, narg)
char *args[];
int narg;
{
#ifdef	NOTYET
	caddr_t		addr;
	int		len;
	int		pos;
	int		bad;
	caddr_t		paddr;

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }
	if (sscanf(args[2], "%x", &pos) != 1) { bad = 2; goto err; }

	if ((paddr = mremap(addr, &len, pos)) == NULL)
		perror("mremap error");
	else
		printf("mremap successful at 0x%08x, len %d\n", paddr, len);
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
#else
	fprintf(stderr, "not yet implemented\n");
#endif
}

cmd_msync(args, narg)
char *args[];
int narg;
{
#ifdef	NOTYET
	caddr_t		addr;
	int		len;
	int		bad;

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }

	if (msync(addr, len) != 0)
		perror("msync error");
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
#else
	fprintf(stderr, "not yet implemented\n");
#endif
}

cmd_madvise(args, narg)
char *args[];
int narg;
{
#ifdef	NOTYET
	caddr_t		addr;
	int		len;
	int		bad;
	int		behav;

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }
	if (sscanf(args[2], "%d", &behav) != 1) { bad = 2; goto err; }

	if (madvise(addr, len, behav) != 0)
		perror("madvise error");
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
#else
	fprintf(stderr, "not yet implemented\n");
#endif
}

cmd_mprotect(args, narg)
char *args[];
int narg;
{
	caddr_t		addr;
	int		len;
	int		bad;
	int		prot;

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }
	if ((prot = getprot(args[2])) < 0) { bad = 2; goto err; }

	if (mprotect(addr, len, prot) != 0)
		perror("mprotect error");
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
}

cmd_mincore(args, narg)
char *args[];
int narg;
{
#ifdef	NOTYET
	caddr_t		addr;
	int		len;
	int		bad;
	int		pagesize;
	int		npages;
	int		i;
	char		*vec;
	extern	char	*malloc();

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }

	pagesize = getpagesize();
	npages = (len + pagesize - 1) / pagesize;
	if ((vec = malloc(npages)) == NULL) {
		perror("malloc of vec failed");
		return;
	}

	if (mincore(addr, len, vec) != 0)
		perror("mincore error");

	printf("In-core pages:");
	for (i = 0; i < npages; i++) {
		if ((i % 16) == 0)
			printf("\n%08x    ", addr + (pagesize * i));
		printf("%02d  ", vec[i]);
	}
	printf("\n");
	free(vec);
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
#else
	fprintf(stderr, "not yet implemented\n");
#endif
}


cmd_mvalid(args, narg)
char *args[];
int narg;
{
	caddr_t		addr;
	int		len;
	int		bad;
	int		prot;

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }
	if ((prot = getprot(args[2])) < 0) { bad = 2; goto err; }

	if (mvalid(addr, len, prot) != 0)
		perror("mvalid error");
	else
		printf("Range is valid\n");
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
}


cmd_fork(args, narg)
char *args[];
int narg;
{
	int	pid;
	int	status;

	if ((pid = fork()) < 0) {
		perror("fork error");
	} else if (pid != 0) {
		wait(&status);
		printf("child returned status %d\n", status);
	}
}


cmd_exec(args, narg)
char *args[];
int narg;
{
	execl(prog, prog, 0);
	perror("exec failed");
}


int 
cmd_ldr_mmap(char *args[], int narg)
{
	univ_t		addr;
	int		len;
	int		prot;
	int		flags;
	int		fd;
	int		pos;
	int		bad;
	univ_t		paddr;
	int		rc;		/* return code */

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }
	if ((prot = getprot(args[2])) < 0) { bad = 2; goto err; }
	if ((flags = getflags(args[3])) < 0) { bad = 3; goto err; }
	if (sscanf(args[4], "%d", &fd) != 1) { bad = 4; goto err; }
	if (sscanf(args[5], "%x", &pos) != 1) { bad = 5; goto err; }

	if ((rc = ldr_mmap(addr, len, prot, flags, fd, pos, &paddr)) < 0)
		perror("ldr_mmap error");
	else
		printf("ldr_mmap successful at 0x%08x, len %d rc %d\n", 
		       paddr, len, rc);
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
}
	

int
cmd_ldr_munmap(args, narg)
char *args[];
int narg;
{
	univ_t		addr;
	int		len;
	int		bad;
	int		rc;		/* return code */

	if (sscanf(args[0], "%x", &addr) != 1) { bad = 0; goto err; }
	if (sscanf(args[1], "%d", &len) != 1) { bad = 1; goto err; }

	if ((rc = ldr_munmap(addr, len)) != 0) 
		printf("ldr_munmap error rc = %d", rc);
	else
		printf("ldr_munmap succeeded\n");
	return;
err:
	fprintf(stderr, "argument error on %s\n", args[bad]);
}

cmd_ldr_msync(args, narg)
char *args[];
int narg;
{
#ifdef	NOTYET
#else
	fprintf(stderr, "not yet implemented\n");
#endif
}

cmd_ldr_open(args, narg)
char *args[];
int narg;
{
	ldr_file_t	fd;
	int		prot;

	if (strcmp(args[1], "r") == 0)
		prot = O_RDONLY;
	else if (strcmp(args[1], "w") == 0)
		prot = O_WRONLY;
	else if (strcmp(args[1], "rw") == 0)
		prot = O_RDWR;
	else {
		fprintf(stderr, "bad open mode %s\n", args[1]);
		return;
	}
	if ((fd = ldr_open(args[0], prot)) < 0) {
		printf("ldr_open failure fd = %d",fd);
		return;
	}

	printf("ldr_open: %s opened on fd %d\n", args[0], fd);
}

cmd_ldr_close(args, narg)
char *args[];
int narg;
{
	ldr_file_t	fd;
	int		rc;		/* return code */

	if (sscanf(args[0], "%d", &fd) != 1) {
		fprintf(stderr, "bad fd %s!\n", args[0]);
		return;
	}

	rc = ldr_close(fd);
	if (rc != 0) {
		fprintf(stderr,"ldr_close error rc = %d\n",rc);
	}
}

int
cmd_ldr_read(char *args[], int nargs)
{
	ldr_file_t	fd;
	int		nbytes;
	int		rc;		/* return code */
	char		*buf;

	if (sscanf(args[0], "%d", &fd) != 1) {
		fprintf(stderr, "bad fd %s!\n", args[0]);
		return;
	}

	if (sscanf(args[1], "%d", &nbytes) != 1) {
		fprintf(stderr, "bad no of bytes %s!\n", args[1]);
		return;
	}

	buf = (char*) malloc(nbytes);
	if (buf == NULL) {
		fprintf(stderr, "error allocating %d bytes!\n",nbytes);
		return;
	}

	rc = ldr_read(fd, buf, nbytes);
	if (rc < 0) {
		printf("ldr_read: error rc = %d\n",rc);
		return;
	}
	else {
		printf("ldr_read: %d bytes read into buffer at 0x%x\n",
		       rc, buf);
	}
}


int
cmd_ldr_stat(char *args[], int nargs)
{
	int		rc;		/* return code */
	struct	stat	*buf;

	buf = (struct stat*) malloc(sizeof(struct stat));
	if (buf == NULL) {
		fprintf(stderr, "error allocating struct stat!\n");
		return;
	}

	rc = ldr_stat(args[0], buf);
	if (rc != 0) {
		printf("ldr_stat: error rc = %d\n",rc);
		return;
	}
	else {
		printf("ldr_stat: stat buffer at 0x%x size %d\n", 
		       buf, sizeof(struct stat));
	}
}

int
cmd_ldr_fstat(char *args[], int nargs)
{
	int		rc;		/* return code */
	ldr_file_t	fd;
	struct	stat	*buf;

	if (sscanf(args[0], "%d", &fd) != 1) {
		fprintf(stderr, "bad fd %s!\n", args[0]);
		return;
	}

	buf = (struct stat*) malloc(sizeof(struct stat));
	if (buf == NULL) {
		fprintf(stderr, "error allocating struct stat!\n");
		return;
	}

	rc = ldr_fstat(fd, buf);
	if (rc != 0) {
		printf("ldr_fstat: error rc = %d\n",rc);
		return;
	}
	else {
		printf("ldr_fstat: stat buffer at 0x%x size %d\n", 
		       buf, sizeof(struct stat));
	}
}


cmd_ldr_file_window(char *args[], int narg)
{
	int start, len;
	ldr_window_t *wp;
	ldr_mapping_t *mp;

	if (sscanf(args[0], "%x", &start) != 1) {
		fprintf(stderr, "bad start %s!\n", args[0]);
		return;
	}

	if (sscanf(args[1], "%x", &len) != 1) {
		fprintf(stderr, "bad len %s!\n", args[1]);
		return;
	}

	if (sscanf(args[2], "%x", &wp) != 1) {
		fprintf(stderr, "bad window addr %s!\n", args[2]);
		return;
	}

	if (ldr_file_window(start, len, wp) == 0) {
		printf("ldr_file_window: returned FAILURE\n");
	}
	else {
		mp = (ldr_mapping_t *)wp->map;
		printf("ldr_file_window: file windowed start 0x%x len 0x%x data at 0x%x\n",
		       start, len, mp->data);
	}
}

cmd_ldr_unwindow(char *args[], int narg)
{
	ldr_window_t *wp;

	if (sscanf(args[0], "%x", &wp) != 1) {
		fprintf(stderr, "bad wp %s!\n", args[0]);
		return;
	}
	ldr_unwindow(wp);
	printf("ldr_unwindow: performed\n");
}

cmd_ldr_init_window(char *args[], int narg)
{
	ldr_file_t fd;
	ldr_window_t *wp;

	if (sscanf(args[0], "%d", &fd) != 1) {
		fprintf(stderr, "bad fd %s!\n", args[0]);
		return;
	}
	wp = ldr_init_window(fd);
	printf("ldr_init_window: window initialized wp = 0x%x\n",wp);
}

cmd_ldr_walk_map_q(char *args[], int narg)
{
	ldr_mapping_t *p = ldr_map_q->sq_head;

	while (p != NULL) {
		printf("fd = %d, refcount = %d, start = %d, len = %d, data = 0x%x\n",
		       p->fd, p->refcount, p->start, p->length, p->data);
		p = p->LRU;
	}
}

cmd_ldr_walk_lru_q(char *args[], int narg)
{
	ldr_mapping_t *p = ldr_lru_q->sq_head;

	while (p != NULL) {
		printf("fd = %d, refcount = %d, start = %d, len = %d, data = 0x%x\n",
		       p->fd, p->refcount, p->start, p->length, p->data);
		p = p->LRU;
	}
}

cmd_ldr_malloc(char *args[], int narg)
{
	int size, rc;
	univ_t ptr;

	if (sscanf(args[0], "%d", &size) != 1) {
		fprintf(stderr, "bad size %s!\n", args[0]);
		return;
	}
        if ((rc = ldr_malloc(size, 0, &ptr)) < 0)
		printf("returned error %d\n", rc);
	else
		printf("memory allocated of size %d at address 0x%x\n", size, ptr);
}

cmd_ldr_free(char *args[], int narg)
{
	int *p;

	if (sscanf(args[0], "%x", &p) != 1) {
		fprintf(stderr, "bad addr %s!\n", args[0]);
		return;
	}

	ldr_free((univ_t) p);
	printf("memory released at 0x%x\n",p);
}

cmd_malloc(char *args[], int narg)
{
	int size;

	if (sscanf(args[0], "%d", &size) != 1) {
		fprintf(stderr, "bad size %s!\n", args[0]);
		return;
	}
	printf("memory allocd of size %d at address 0x%x\n", size,
	       malloc(size));
}

cmd_free(char *args[], int narg)
{
	int *p;

	if (sscanf(args[0], "%x", &p) != 1) {
		fprintf(stderr, "bad addr %s!\n", args[0]);
		return;
	}

	free((univ_t) p);
	printf("memory freed at 0x%x\n",p);
}

cmd_flush_lru_q()
{
	int rc = ldr_flush_lru_maps();
}

cmd_heap_create(char *args[], int narg)
{
	univ_t addr;
	ldr_heap_t heap;
	ldr_file_t fd;
	int flags;
	int offset;
	int rc;

	if (sscanf(args[0], "%x", &addr) != 1) {
		fprintf(stderr, "bad heap %s!\n", args[0]);
		return;
	}
	if (sscanf(args[1], "%d", &fd) != 1) {
		fprintf(stderr, "bad fd %s!\n", args[1]);
		return;
	}
	if ((flags = getflags(args[2])) < 0) { 
		fprintf(stderr, "bad flags %s!\n", args[2]);
		return;
	}
	if (sscanf(args[3], "%d", &offset) != 1) {
		fprintf(stderr, "bad offset %s!\n", args[3]);
		return;
	}

	rc = ldr_heap_create(addr, fd, flags, offset, &heap);
	if (rc < 0)
		printf("returned error %d\n", rc);
	else
		printf("heap created at 0x%x\n", heap);
}
	

cmd_heap_malloc(char *args[], int narg)
{
	ldr_heap_t heap;
	int size, rc;
	univ_t ptr;

	if (sscanf(args[0], "%x", &heap) != 1) {
		fprintf(stderr, "bad heap %s!\n", args[0]);
		return;
	}
	if (sscanf(args[1], "%d", &size) != 1) {
		fprintf(stderr, "bad size %s!\n", args[1]);
		return;
	}
        if ((rc = ldr_heap_malloc(heap, size, 0, &ptr)) < 0)
		printf("returned error %d\n", rc);
	else
		printf("memory allocated of size %d at address 0x%x\n", size, ptr);
}

cmd_heap_free(char *args[], int narg)
{
	ldr_heap_t heap;
	int *p;
	int rc;

	if (sscanf(args[0], "%x", &heap) != 1) {
		fprintf(stderr, "bad heap %s!\n", args[0]);
		return;
	}
	if (sscanf(args[1], "%x", &p) != 1) {
		fprintf(stderr, "bad addr %s!\n", args[1]);
		return;
	}

	rc = ldr_heap_free(heap, (univ_t) p);
	if (rc < 0)
		printf("returned error %d\n", rc);
	else
		printf("memory released at 0x%x\n",p);
}
