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
static char	*sccsid = "@(#)$RCSfile: mda.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:24 $";
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
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include <stdio.h>
#include <ci.h>
#include <strings.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <setjmp.h>

int	cpu_cmd(), display_cmd(), find_cmd(), lookup_cmd();
int	quit_cmd(), read_cmd(), show_cmd(), times_cmd();
int	trace_cmd(), translate_cmd();
int	history_cmd(), bang_cmd(), bangbang_cmd();

extern char *malloc();

int	vm_enabled = 1;		/* true if translation is on */
unsigned int mask_val = 0xffffffff;

CSTRING(kernel_description, kernel_file, MAXPATHLEN, "kernel file: ");
CSTRING(dump_description, dump_file, MAXPATHLEN, "dump file: ");

CIENTRY commands[] =
{
	CICMD("cpu", cpu_cmd),  
	CICMD("display", display_cmd), 
	CICMD("find", find_cmd), 
	CICMD("history",  history_cmd),
	CICMD("lookup", lookup_cmd), 
	CICMD("quit", quit_cmd), 
	CICMD("read-dump-file", read_cmd), 
	CICMD("show", show_cmd), 
	CICMD("times", times_cmd), 
	CICMD("trace", trace_cmd), 
	CICMD("translate", translate_cmd), 
	CICMD("#", bang_cmd),
	CICMD("##", bangbang_cmd),
	CICSTRING("dump-file", dump_description), 
	CICSTRING("kernel-file", kernel_description), 
	CIBOOL("virtual_memory", vm_enabled), 
	CIHEX("and_mask", mask_val),
	CIEND
};

#define	KERNELFILE	"/../../mach"
#define	DUMPFILE	"/dumps/machdump"
#define	HELPDIR		"/usr/local/adm/mda/helpdir"
#define	MEG		(1024 * 1024)

jmp_buf	cmdjbuf;
extern	int	cidepth;
int	onsegv();
int	onintr();

main(argc, argv)
int	argc;
char	*argv[];
{
	char	*prompt, *helppath, *cmdfpath;
	FILE	*file;
	int	depth;
	CIENTRY	*list;
	char 	prog[MAXPATHLEN];
	int	dump = 0;
	int	kernel = 0;

	strcpy(prog,argv[0]);
	argv++;
	if(argc >= 3) {
		if(fncipher(argc, argv)) {
			fprintf(stderr, "%s: using as a dump file\n", 
				dump_file);

			fprintf(stderr, "%s: using as a kernel file\n", 
				kernel_file);

			goto doit;
		} else {
			fprintf(stderr,"Usage: %s [ dump-file ] [ kernel ]\n", 
				prog);

			exit(1);
		}
	}

	strcpy(dump_file, DUMPFILE);
	strcpy(kernel_file, KERNELFILE);
	while (--argc) {
		struct stat buf;

		if(stat(argv[0], &buf) != 0) {
			fprintf(stderr, "%s: Can't stat %s\n", prog, argv[0]);
			exit(1);
		}

		if(buf.st_size > 8 * MEG) {	/* Assume dump file */
			strcpy(dump_file, argv[0]);
			dump = 1;
		} else {			/* Assume kernel file */
			strcpy(kernel_file, argv[0]);
			dump = 0;
		}
		fprintf(stderr, "%s: using as a %s file\n", argv[0],
			(dump ? "dump" : "kernel"));
		argv++;
	}
doit:
	{
		char	*c;

		if ((c = malloc(1 * 1024 * 1024)) != (char *) 0)
			free(c);
	}
	if (strcmp(dump_file,DUMPFILE) != 0)
		read_cmd(dump_file);

	init_symbol_cache();
	list = &commands[0];
	prompt = "mda>";
	signal(SIGINT, onintr);
	signal(SIGSEGV, onsegv);
	setjmp(cmdjbuf);
	ci(prompt, 0, 0, list, HELPDIR, 0);
}

onintr()
{
	printf("\nInterrupt!\n\n");
	cidepth = 0;
	longjmp(cmdjbuf, 1);
}

onsegv()
{
	printf("\nSegmentation Violation!\n\n");
	cidepth = 0;
	longjmp(cmdjbuf, 1);
}

char *rindex();

/*
 * "Cipher" out which of the file names on the command line may
 *  be our dump and kernel image files.
 */
fncipher(argc, argv)
int	argc;
char	*argv[];
{
	int	i;
	int	gotd, gotm;
	char	*p, *q;

	gotd = 0;
	gotm = 0;
	i = 0;
	while(--argc && (!gotd || !gotm)) {
		p = argv[i];
		if(q = rindex(p, '/'))
			p = q + 1;
		while(*p) {
			if(!gotd && *p == 'd') {
				if(strncmp(p, "dump", 4) == 0) {
					gotd++;
					strcpy(dump_file, argv[i]);
					break;
				}
			}
			if(!gotm && *p == 'm') {
				if(strncmp(p, "mach", 4) == 0) {
					q = argv[i] - strlen(argv[i]) - 4;
					if(strcmp(q, ".cdb")) {
						gotm++;
						strcpy(kernel_file, argv[i]);
						break;
					}
				}
			}
			p++;
		}
		i++;
	}

	return(gotd && gotm);
}
