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
static char	*sccsid = "@(#)$RCSfile: link.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/08 16:21:26 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: link, unlink
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/* link.c	1.5  com/cmd/fs/progs,3.1,9021 11/8/89 16:56:49 */

/*
** single source link/unlink
** link and unlink are, ahem, links to each other ;-)
*/

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <NLctype.h>
#include <NLchar.h>
#include <locale.h>
#include <nl_types.h>
#include "link_msg.h"

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_LINK,Num,Str)

extern int link(), unlink();
extern char *rindex();

/*
 * un-needed complexity, isn't this great!
 */
struct progs {
    char *progname;		/* program name */
    int (*func)();		/* function to call */
    int args;			/* 1 or 2 args */
    int msgnum;			/* usage message number */
    char *usage;		/* text of usage message */
    int errexit;		/* what code to exit on bad return from func */
} prog[] = { 
    { "link", link, 2, LINK_USAGE, "Usage: link from to\n", 2 }, 
    { "unlink", unlink, 1, UNLINK_USAGE, "Usage: unlink name\n", 2 } 
};

#define EQ(a,b) (!strcmp(a,b))
#define NELEM(a) (sizeof(a)/sizeof(a[0]))

main(argc, argv) 
char *argv[]; 
{
    int i;
    int rv;
    char *progname;

    (void) setlocale (LC_ALL,"");
    catd = catopen((char *)MF_LINK,NL_CAT_LOCALE);
#if SEC_BASE
    set_auth_parameters(argc, argv);
    initprivs();
#endif
    progname = rindex(argv[0],'/'); 
    if (progname) progname++; else progname = argv[0];

#if SEC_BASE
    if (authorized_user("linkdir") && !forcepriv(SEC_LINKDIR)) {
        fprintf(stderr, "%s: insufficient privileges\n", progname);
        exit(1);
    }
#endif

    /* identify our name */
    for ( i = 0 ; i < NELEM(prog); i++)
	if (EQ(progname,prog[i].progname)) break;

    /* did we find our name? */
    if (i == NELEM(prog)) {
	fprintf(stderr,
	    MSGSTR(OOPS,"link/unlink: I should be called link or unlink. defaulting to %s\n"),prog[0].progname);
	i = 0;
    }

    /* check args before calling function */
    if (argc != prog[i].args+1) {
	fprintf(stderr,MSGSTR(prog[i].msgnum,prog[i].usage));
	exit(1);
    }

#if SEC_BASE
    disablepriv(SEC_SUSPEND_AUDIT);
#endif

    /* call the desired function */
    rv = (prog[i].func)(argv[1],argv[2]);

    exit(rv==0 ? 0 : prog[i].errexit);
    /*NOTREACHED*/
}
