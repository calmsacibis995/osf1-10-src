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
static char	*sccsid = "@(#)$RCSfile: echo_loader.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:42:23 $";
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

#define	DEFAULT_LOADER	"/sbin/loader"

extern char **environ;

extern int _ldr_crt0_request;
static int *_lcrp = &_ldr_crt0_request;
int _ldr_present = 1;

void print_header(), print_argv(), print_env(), print_auxv();
void auxv_init(), auxv_print();

int
main(argc, argv, envp, auxv)
	char *argv[];
	void *envp, *auxv;
{
	auxv_init();

	print_header();

	(void)printf("\n");
	(void)printf("  argc = %d\n", argc);
	(void)printf("  argv = 0x%x\n", argv);
	(void)printf("  envp = 0x%x\n", envp);
	(void)printf("  auxv = 0x%x\n", auxv);

	print_argv(argv);
	print_env();
	print_auxv();

	exit(0);
	return(0);
}

void
print_header()
{
	char *exec_loader_filename, *auxv_get_exec_loader_filename();
	char *exec_filename, *auxv_get_exec_filename();
	int exec_loader_flags;

	(void)printf("\n\nEcho Loader");
	exec_filename = auxv_get_exec_filename();
	if (auxv_get_exec_loader_flags(&exec_loader_flags)) {
		(void)printf(": called via execve()\n");
		(void)printf("  file = \"%s\"\n", exec_filename);
	} else {
		(void)printf(": called via exec_with_loader()\n");
		exec_loader_filename = auxv_get_exec_loader_filename();
		if (strcmp(exec_loader_filename, DEFAULT_LOADER))
			(void)printf("  loader = \"%s\"\n",
				exec_loader_filename);
		else
			(void)printf("  loader = \"%s\" (DEFAULT_LOADER)\n",
				exec_loader_filename);
		(void)printf("  file = \"%s\"\n", exec_filename);
	}
}

void
print_argv(argv)
	char **argv;
{
	int i;

	if (!(*argv))
		return;
	(void)printf("\n");
	for (i = 0; *argv; i++) {
		(void)printf("  argv[%d] = 0x%x \"%s\"\n", i, argv, *argv);
		argv++;
	}
}

void
print_env()
{
	char **envp;
	int i;

	if (!(envp = environ))
		return;
	(void)printf("\n");
	for (i = 0; *envp; i++) {
		(void)printf("  envp[%d] = 0x%x \"%s\"\n", i, envp, *envp);
		envp++;
	}
}

void
print_auxv()
{
	(void)printf("\n");
	auxv_print("  ");
}
