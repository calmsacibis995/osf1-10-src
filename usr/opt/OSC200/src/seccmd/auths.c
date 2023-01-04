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
static char *rcsid = "@(#)$RCSfile: auths.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/07 14:27:48 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1990 SecureWare, Inc.  All Rights Reserved.
 *
 * auths [-x]
 *
 * Display a user's command authorizations.  With no arguments,
 * displays all command authorizations available either explicitly
 * or implicitly.  With the -x option, displays only those
 * authorizations that are explicitly granted.
 */



#include <sys/types.h>
#include <sys/security.h>
#include <prot.h>
#include <values.h>
#include <stdio.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "auths_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_AUTHS,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

extern struct cmdauth {
	char	*auth_name;
	priv_t	*auth_impl;
} *cmdauths;

extern priv_t	*userauths;
extern int	ST_MAX_CPRIV;
extern char	*malloc();

main(argc, argv)
	int	argc;
	char	*argv[];
{
	register int	i, j, authwords, tot, xflag = 0;
	register char	*buf, *bp;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_AUTHS,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);

	if (argc == 2 && strcmp(argv[1], "-x") == 0)
		xflag = 1;
	else if (argc != 1) {
		fprintf(stderr, MSGSTR(USAGE,
			"usage: %s [-x]\n"), command_name);
		exit(1);
	}

	tot = total_auths();
	if (tot == -1)
	  exit(1);
	authwords = WORD_OF_BIT(ST_MAX_CPRIV - 1) + 1;
	bp = buf = malloc(tot * (widest_auth() + 1));
	if (bp == (char *) 0) {
		fprintf(stderr, MSGSTR(MEMFAIL,
			"%s: memory allocation failure\n"),
			command_name);
		exit(1);
	}

	for (i = 0; i < tot; ++i) {
		if (!ISBITSET(userauths, i)) {
			if (xflag)
				continue;
			for (j = 0; j < authwords; ++j)
				if (userauths[j] & cmdauths[i].auth_impl[j])
					break;
			if (j >= authwords)
				continue;
		}
		if (bp != buf)
			*bp++ = ' ';
		strcpy(bp, cmdauths[i].auth_name);
		bp += strlen(cmdauths[i].auth_name);
	}
	*bp = '\0';
	printbuf(buf, 0, " ");
	exit(0);
}
