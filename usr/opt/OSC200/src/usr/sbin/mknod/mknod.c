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
static char	*sccsid = "@(#)$RCSfile: mknod.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/08 15:46:28 $";
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
#ifndef lint

#endif not lint

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <sys/stat.h>
#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "mknod_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MKNOD,n,s)
#ifdef SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_MKNOD_SEC,n,s)
#endif
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

main(argc, argv)
	int argc;
	char **argv;
{
	int m, a, b;

	setlocale( LC_ALL, "" );
	catd = catopen(MF_MKNOD,NL_CAT_LOCALE);

	if(argc == 3) {
		if (strcmp(argv[2],"p") != 0)
			goto usage;
		if(mkfifo(argv[1], 0666) < 0) {
			fprintf(stderr, "mkfifo: ");
			perror(argv[1]);
			exit(1);
		}
		exit(0);

	} else if(argc != 5) {
		fprintf(stderr, MSGSTR(ARGCOUNT, "arg count\n"));
		goto usage;
	}
	if(*argv[2] == 'b')
		m = 060666; else
	if(*argv[2] == 'c')
		m = 020666; else
		goto usage;
	a = number(argv[3]);
	if(a < 0)
		goto usage;
	b = number(argv[4]);
	if(b < 0)
		goto usage;

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
	if (!authorized_user("mknod")) {
		fprintf(stderr, MSGSTR_SEC(AUTH,
			"%s: need mknod authorization\n"), command_name);
		exit(1);
	}
	if (forceprivs(privvec(SEC_MKNOD
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
				-1), (priv_t *) 0) ||
            enableprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr,
			MSGSTR_SEC(PRIV, "%s: insufficient privileges\n"),
			command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);
#endif /* SEC_BASE */
	if(mknod(argv[1], m, makedev(a,b)) < 0) {
		fprintf(stderr, "mknod: ");
		perror(argv[1]);
		exit(1);
	}
	exit(0);

usage:
	fprintf(stderr, MSGSTR(USAGE,
		"Usage: mknod specialfile b/c major minor\nor mknod fifo p\n"));
	exit(1);
}

number(s)
char *s;
{
	int n, c;

	n = 0;
	while(c = *s++) {
		if(c<'0' || c>'9')
			return(-1);
		n = n*10 + c-'0';
	}
	return(n);
}
