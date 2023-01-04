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
#if !defined(lint) && !defined(_NOIDENT)
static char	*sccsid = "@(#)$RCSfile: hostid.c,v $ $Revision: 4.3.2.3 $ (DEC) $Date: 1993/10/08 16:21:18 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* 3.3 18:25:41 6/19/90 SecureWare */

/* Copyright (c) 1983, 1988 Regents of the University of California.\n\*/

/*  hostid.c	5.6 (Berkeley) 6/18/88 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>

#include "hostid_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_HOSTID,n,s)
#include <locale.h>


main(argc, argv)
	int argc;
	char **argv;
{
	register char *id;
	struct hostent *hp;
	u_int addr, inet_addr();
	int hostid, gethostid();
	char *index();
#if SEC_BASE
	privvec_t saveprivs;
	int status;
#endif


        setlocale(LC_ALL,"");
        catd = catopen(MF_HOSTID,NL_CAT_LOCALE);


	if (argc < 2) {
		printf("%#x\n", gethostid());
		exit(0);
	}
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
	if (!authorized_user("sysadmin")) {

		fprintf(stderr, MSGSTR(SYS_ADM, "%s: need sysadmin authorization\n"),
			command_name);
		exit(1);
	}
#endif

	id = argv[1];
	if ((hostid = inet_addr(id)) != -1)
		hostid = ntohl(hostid);
	else if (hp = gethostbyname(id)) {
		bcopy(hp->h_addr, (char *)&addr, sizeof(addr));
		hostid = addr;
		hostid = ntohl(hostid);
	} else if (index(id, '.'))
		goto usage;
	else {
		if (id[0] == '0' && (id[1] == 'x' || id[1] == 'X'))
			id += 2;
		if (sscanf(id, "%lx", &hostid) != 1) {
usage:                  fprintf(stderr, MSGSTR(USAGE, "usage: %s [hexnum or internet address]\n"), /*MSG*/ argv[0]);
			exit(1);
		}
	}

#if SEC_BASE
	if (forceprivs(privvec(SEC_SYSATTR, -1), saveprivs)) {
		fprintf(stderr,  MSGSTR(SYS_ATTR, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);
	status = sethostid(hostid);
	seteffprivs(saveprivs, (priv_t *) 0);
	if (status < 0) {
		perror( MSGSTR(SETHOST,"sethostid"));
		exit(1);
	}
#else /* !SEC_BASE */
	if (sethostid(hostid) < 0) {
		perror( MSGSTR(SETHOST,"sethostid"));
		exit(1);
	}
#endif /* !SEC_BASE */

	exit(0);
}
