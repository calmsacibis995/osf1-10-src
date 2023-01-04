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
static char	*sccsid = "@(#)$RCSfile: tc2.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:19:56 $";
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint

#endif not lint

/*
 * tc2 [term]
 * Dummy program to test out termlib.
 * Commands are "tcc\n" where t is type (s for string, f for flag,
 * or n for number) and cc is the name of the capability.
 */
#include <stdio.h>
char buf[1024];
char *getenv(), *tgetstr();

main(argc, argv) char **argv; {
	char *p, *q;
	int rc;
	char b[3], c;
	char area[200];

	if (argc < 2)
		p = getenv("TERM");
	else
		p = argv[1];
	rc = tgetent(buf,p);
	for (;;) {
		c = getchar();
		if (c < 0)
			exit(0);
		b[0] = getchar();
		if (b[0] < ' ')
			exit(0);
		b[1] = getchar();
		b[2] = 0;
		getchar();
		switch(c) {
			case 'f':
				printf("%s: %d\n",b,tgetflag(b));
				break;
			case 'n':
				printf("%s: %d\n",b,tgetnum(b));
				break;
			case 's':
				q = area;
				printf("%s: %s\n",b,tgetstr(b,&q));
				break;
			default:
				exit(0);
		}
	}
}
