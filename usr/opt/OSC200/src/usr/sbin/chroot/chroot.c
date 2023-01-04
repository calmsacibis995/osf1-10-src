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
static char	*sccsid = "@(#)$RCSfile: chroot.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/07 21:54:29 $";
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
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */

/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/* chroot */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <stdio.h>

#if defined(NLS) || defined(KJI)
#define	NLSKJI 1
#include <NLctype.h>
#include <NLchar.h>
#define printf NLprintf
#define fprintf NLfprintf
#endif
#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "chroot_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) NLcatgets(catd,MS_CHROOT,Num,Str)
#else
#define MSGSTR(Num,Str) Str
#endif

main(argc, argv)
char **argv;
{
#ifdef NLS
	(void) setlocale (LC_ALL,"");
#endif
#ifdef MSG
	catd = NLcatopen(MF_CHROOT,NL_CAT_LOCALE);
#endif
	if(argc < 3) {
		printf(MSGSTR(USAGE,"usage: chroot rootdir command arg ...\n"));
		exit(1);
	}
	argv[argc] = 0;
	if(argv[argc-1] == (char *) -1) /* don't ask why */
		argv[argc-1] = (char *) -2;

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("chroot")) {
		printf(MSGSTR(AUTH, "%s: need chroot authorization\n"),
			"chroot");
		exit(1);
	}

	if (!forcepriv(SEC_CHROOT)) {
		printf(MSGSTR(PRIV, "%s: insufficient privileges\n"), "chroot");
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);
#endif

	if (chroot(argv[1]) < 0) {
		perror(argv[1]);
		exit(1);
	}
#if SEC_BASE
	forcepriv(SEC_SUSPEND_AUDIT);
#endif
	if (chdir("/") < 0) {
		printf(MSGSTR(CANTCHROOT,"Can't chdir to new root\n"));
		exit(1);
	}
	execv(argv[2], &argv[2]);
	close(2);
	open("/dev/tty", 1);
	printf(MSGSTR(NOTFOUND,"%s: not found\n"),argv[2]);
	exit(1);
}
