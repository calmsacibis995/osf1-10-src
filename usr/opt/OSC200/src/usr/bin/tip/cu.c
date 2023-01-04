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
static char rcsid[] = "@(#)$RCSfile: cu.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/09/07 15:36:42 $";
#endif
/*
cu.c	1.5  com/cmd/tip,3.1,9013 12/21/89 16:44:44";
 */
/* 
 * COMPONENT_NAME: UUCP cu.c
 * 
 * FUNCTIONS: MSGSTR, cumain 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "cu.c	5.2 (Berkeley) 1/13/86"; */

#include "tip.h"

void	cleanup();
void	timeout();

/*
 * Botch the interface to look like cu's
 */
cumain(argc, argv)
	char *argv[];
{
	register int i;
	static char sbuf[12];

	if (argc < 2) {
		printf(MSGSTR(USAGE3, "usage: cu telno [-t] [-s speed] [-a acu] [-l line] [-#]\n")); /*MSG*/
		exit(8);
	}
	CU = DV = NOSTR;
	BR = DEFBR;
	for (; argc > 1; argv++, argc--) {
		if (argv[1][0] != '-')
			PN = argv[1];
		else switch (argv[1][1]) {

		case 't':
			HW = 1, DU = -1;
			--argc;
			continue;

		case 'a':
			CU = argv[2]; ++argv; --argc;
			break;

		case 's':
			if (argc < 3 || speed(atoi(argv[2])) == 0) {
				fprintf(stderr, MSGSTR(NOSUPPORT, "cu: unsupported speed %s\n"), /*MSG*/
					argv[2]);
				exit(3);
			}
			BR = atoi(argv[2]); ++argv; --argc;
			break;

		case 'l':
			DV = argv[2]; ++argv; --argc;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (CU)
				CU[strlen(CU)-1] = argv[1][1];
			if (DV)
				DV[strlen(DV)-1] = argv[1][1];
			break;

		default:
			printf(MSGSTR(BADFLAG, "Bad flag %s"), argv[1]); /*MSG*/
			break;
		}
	}
	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGHUP, cleanup);
	signal(SIGTERM, cleanup);

	/*
	 * The "cu" host name is used to define the
	 * attributes of the generic dialer.
	 */
	(void)sprintf(sbuf, "cu%d", BR);
	if ((i = hunt(sbuf)) == 0) {
		printf(MSGSTR(ALLBUSY, "all ports busy\n")); /*MSG*/
		exit(3);
	}
	if (i == -1) {
		printf(MSGSTR(LINKDOWN, "link down\n")); /*MSG*/
		uu_unlock(uucplock);
		exit(3);
	}
	setbuf(stdout, NULL);
	loginit();
	user_uid();
	vinit();
	setparity("none");
	boolean(value(VERBOSE)) = 0;
	if (HW)
		ttysetup(speed(BR));
	if (connect()) {
		printf(MSGSTR(CONNECTFAIL, "Connect failed\n")); /*MSG*/
		daemon_uid();
		uu_unlock(uucplock);
		exit(1);
	}
	if (!HW)
		ttysetup(speed(BR));
}
