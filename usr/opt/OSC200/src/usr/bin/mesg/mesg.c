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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: mesg.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/10/11 17:26:04 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: mesg
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 * 
 * mesg.c    1.10  com/cmd/comm,3.1,9021 2/7/90 14:17:03
 * mesg.c	4.1 22:03:52 7/15/90 SecureWare 
 */
/*
 *                                                                    
 * mesg -- set current tty to accept or
 *	forbid write permission.
 *
 *	mesg [-y] [-n]
 *		y allow messages
 *		n forbid messages
 *	return codes
 *		0 if messages are ON or turned ON
 *		1 if messages are OFF or turned OFF
 *		2 if usage error
 */

#include <sys/secdefines.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/access.h>

#include <nl_types.h>
#include <langinfo.h>
#include "mesg_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_MESG,Num,Str)

static void newmode(mode_t);
struct stat sbuf;

char *tty;

char *progname;

int
main(int argc, char *argv[])
{
	int i;
	char *ystr = "y";
	char *nstr = "n";

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_MESG,NL_CAT_LOCALE);

	if ((ystr = nl_langinfo(YESSTR)) == "") 
		ystr = "y";

	if ((nstr = nl_langinfo(NOSTR)) == "")
		nstr = "n";

	progname = argv[0];

	for(i = 0; i <= 2; i++) {
		if ((tty = ttyname(i)) != NULL)
			break;
	}
	if (tty == NULL) {
		fprintf(stderr, MSGSTR(NOTERM,"Cannot find terminal.\n"));
		exit(2);
	}
	if (stat(tty, &sbuf) < 0) {
		fprintf(stderr, 
		         MSGSTR(NOSTATUS,"Cannot get status of \"%s\".\n"), tty);
		exit(2);
	}
	if (argc < 2) {
		if (sbuf.st_mode & (S_IWGRP | S_IWOTH)) {
			printf(MSGSTR(IS,"is %s\n"), ystr); /*MSG*/
			exit(0);
		}
		else  {
			printf(MSGSTR(IS,"is %s\n"), nstr); /*MSG*/
			exit(1);
		}
	}


#define YES 1
#define NO  0

	if (argv[1][0] == '-')
		argv[1]++;

	switch(rpmatch(argv[1])) {
		case YES:
			newmode(sbuf.st_mode | (S_IWGRP | S_IWOTH));
			exit(0);
		case NO:
			newmode(sbuf.st_mode &~ (S_IWGRP | S_IWOTH));
			exit(1);
		default: 
			fprintf(stderr, 
                                MSGSTR(USAGE,"usage: %s [-][%s|%s]\n"),
                                progname, ystr, nstr);
			exit(2);
	}
/*NOTREACHED*/
}


/* 
 * NAME: newmode
 * FUNCTION: change permission mode on tty
 */
static void
newmode(mode_t mode)
{
#if SEC_BASE
    	mode &= ~ (S_IROTH | S_IWOTH | S_IXOTH); /* Deny any WORLD access modes */
#endif
	if ( chmod(tty, mode) < 0 )
	{
		fprintf(stderr, 
		  MSGSTR(CANTCHG,"Cannot change mode of \"%s\".\n"), tty); /*MSG*/
		exit(2);
	}
}
