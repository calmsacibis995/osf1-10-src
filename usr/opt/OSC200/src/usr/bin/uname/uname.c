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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: uname.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/10/11 19:26:22 $";
#endif 
/*
 * OSF/1 Release 1.0
 */
/*	uname.c	1.2	*/

#include	<stdio.h>
#include	<sys/utsname.h>
#include	<locale.h>
#include	"uname_msg.h"

#define MSGSTR(n,s) catgets(catd,MS_UNAME,n,s)

struct utsname	unstr, *un;

main(argc, argv)
char **argv;
int argc;
{
	register i;
	int	sflg=1, nflg=0, rflg=0, vflg=0, mflg=0, errflg=0;
	int	optlet;

	un = &unstr;
	uname(un);

	while((optlet=getopt(argc, argv, "asnrvm")) != EOF) switch(optlet) {
	case 'a':
		sflg++; nflg++; rflg++; vflg++; mflg++;
		break;
	case 's':
		sflg++;
		break;
	case 'n':
		nflg++;
		break;
	case 'r':
		rflg++;
		break;
	case 'v':
		vflg++;
		break;
	case 'm':
		mflg++;
		break;
	case '?':
		errflg++;
	}
	if(errflg) {
		nl_catd catd;
		/*
		 * Only need message catalog and locale
		 * for usage message
		 */
		(void) setlocale( LC_ALL, "" );
		catd = catopen(MF_UNAME, NL_CAT_LOCALE);
		fprintf(stderr, MSGSTR(USAGE, "usage: uname [-snrvma]\n"));
		exit(1);
	}
	if(nflg | rflg | vflg | mflg) sflg--;
	if(sflg)
		fprintf(stdout, "%.*s", SYS_NMLN, un->sysname);
	if(nflg) {
		if(sflg) putchar(' ');
		fprintf(stdout, "%.*s", SYS_NMLN, un->nodename);
	}
	if(rflg) {
		if(sflg | nflg) putchar(' ');
		fprintf(stdout, "%.*s", SYS_NMLN, un->release);
	}
	if(vflg) {
		if(sflg | nflg | rflg) putchar(' ');
		fprintf(stdout, "%.*s", SYS_NMLN, un->version);
	}
	if(mflg) {
		if(sflg | nflg | rflg | vflg) putchar(' ');
		fprintf(stdout, "%.*s", SYS_NMLN, un->machine);
	}
	putchar('\n');
	exit(0);
}
