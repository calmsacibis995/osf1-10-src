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
static char rcsid[] = "@(#)$RCSfile: uuname.c,v $ $Revision: 4.3.6.3 $ (DEC) $Date: 1993/10/11 19:35:51 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uuname.c
 * 
 * FUNCTIONS: Muuname 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/*
1.7  uuname.c, bos320 3/27/91 10:59:22";
*/
#include "uucp.h"
/* VERSION( uuname.c	5.2 -  -  ); */
 
nl_catd catd;
/*
 * returns a list of all remote systems.
 * option:
 *	-l	-> returns only the local system name.
 */
main(argc,argv, envp)
int argc;
char **argv, **envp;
{
	FILE *np;
	register short lflg = 0;
	char s[BUFSIZ], prev[BUFSIZ], name[BUFSIZ];
	int ch;

	setlocale(LC_ALL,"");

	while ((ch = getopt(argc, argv, "l")) != -1)
		switch(ch) {
		case 'l':
			lflg++;
			break;
		default:
			usage();
		}
	argc -= optind;
	if (argc > 0)
		usage();

	if (lflg) {
		uucpname(name);

		/* initialize to null string */
		(void) printf("%s",name);
		(void) printf("\n");
		exit(0);
	}
	if ((np=fopen(SYSFILE, "r")) == NULL) {
		catd = catopen(MF_UUCP, NL_CAT_LOCALE);
		(void) fprintf(stderr, MSGSTR(MSG_UUNAME2, 
		       "File \" %s \" is protected\n"), SYSFILE);
		exit(1);
	}
 
	while (fgets(s, BUFSIZ, np) != NULL) {
		if((s[0] == '#') || (s[0] == ' ') || (s[0] == '\t') || 
		    (s[0] == '\n'))
			continue;
		(void) sscanf(s, "%s", name);
		if (EQUALS(name, prev))
		    continue;
		(void) printf("%s", name);
		(void) printf("\n");
		(void) strcpy(prev, name);
	}
	catclose(catd);

	exit(0);
}

usage()
{
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);
	(void) fprintf(stderr, MSGSTR(MSG_UUNAME1, "usage: uuname [-l]\n"));
	exit(1);
}
