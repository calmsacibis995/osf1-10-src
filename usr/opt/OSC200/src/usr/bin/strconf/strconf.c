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
static char *rcsid = "@(#)$RCSfile: strconf.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/10/11 20:00:34 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1991  Mentat Inc.
 ** strconf.c 1.1, last change 4/8/91
 **/

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <stropts.h>
#include <stdio.h>
#include <malloc.h>

#include <locale.h>
#include "strconf_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_STRCONF,n,s)

#include <sys/stream.h>
#define staticf static

staticf int	get_module_names(   int fd, struct str_list * list   );

extern	char	* optarg;
extern	int	optind;

staticf char *
errmsg (err)
	int	err;
{
	static	char	buf[40];

	if (err  ||  (err = errno)) {
		if (err > 0  &&  err < sys_nerr  &&  sys_errlist[err])
			return sys_errlist[err];
	}
	if (err)
		(void)sprintf(buf, MSGSTR(STRCNF_ERRNUM, "error number %d"),
		    err);
	else
		(void)sprintf(buf, MSGSTR(STRCNF_UNERR, "unspecified error"));
	return buf;
}

main (argc, argv)
	int	argc;
	char	** argv;
{
	int	c;
	int	i1;
	struct str_list	list;
	char	* module_name;
	int	print_topmost_only;

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_STRCONF,NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, MSGSTR(STRCNF_AUTH,
		    "strconf: need sysadmin authorization\n"));
		exit(1);
	}
	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_OWNER,
#if SEC_MAC
		SEC_ALLOWMACACCESS,
#endif
		-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(STRCNF_PRIV,
		    "strconf: insufficient privileges\n"));
		exit(1);
	}
#endif

	module_name = 0;
	print_topmost_only = 0;
	while ((c = getopt(argc, argv, "m:M:tT")) != -1) {
		switch (c) {
		case 'm':
		case 'M':
			if (print_topmost_only)
				goto usage;
			module_name = optarg;
			break;
		case 't':
		case 'T':
			if (module_name)
				goto usage;
			print_topmost_only = 1;
			break;
		default:
usage:
			printf(MSGSTR(STRCNF_USAGE,
			    "usage: strconf [-t | -m module]\n"));
			exit(1);
		}
	}
	/* If there's extra stuff on the command line, print usage. */
	if (optind != argc)
		goto usage;

	/* Get the list of modules on the stream */
	get_module_names(0, &list);

	/* Handle -m option */
	if (module_name) {
		/* Check all module names on the stream, but not the device */
		for (i1 = 0; i1 < list.sl_nmods-1; i1++) {
			if (strcmp(module_name, list.sl_modlist[i1].l_name) == 0) {
				printf(MSGSTR(STRCNF_YES, "yes"));
				exit(0);
			}
		}
		printf(MSGSTR(STRCNF_NO, "no"));
		exit(1);
	}

	/* Handle -t option */
	if (print_topmost_only) {
		if (list.sl_nmods > 1)
			printf("%s\n", list.sl_modlist[0].l_name);
		exit(0);
	}

	/* Default is to print the names of all modules on the stream */
	for (i1 = 0; i1 < list.sl_nmods; i1++)
		printf("%s\n", list.sl_modlist[i1].l_name);
	exit(0);
}

/*
 * This routine returns the number of modules on a stream (including
 * the device) and places the module names in "list" (if non-nil).
 */
staticf int
get_module_names (fd, list)
	int	fd;
	struct str_list	* list;
{
	int	i1;

	/* First get the number of modules on the stream */
	i1 = ioctl(fd, I_LIST, (struct str_list *)0);
	if (i1 == -1) {
		printf("I_LIST ioctl failed, %s\n", errmsg(0));
		exit(1);
	}

	/* If the caller wants the module names as well, place them in list. */
	if (list) {
		/* Allocate space for the module names */
		list->sl_nmods = i1;
		if (!(list->sl_modlist = (struct str_mlist *)malloc(i1 * sizeof(struct str_mlist)))) {
			printf(MSGSTR(STRCNF_NMALL,
			    "Couldn't malloc space for module list"));
			exit(1);
		}

		/* Now get the actual module names */
		i1 = ioctl(fd, I_LIST, list);
		if (i1 == -1) {
			printf("I_LIST ioctl failed, %s\n", errmsg(0));
			exit(1);
		}
	}
	return i1;
}
