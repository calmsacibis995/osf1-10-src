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
static char rcsid[] = "@(#)$RCSfile: tee.c,v $ $Revision: 4.2.9.4 $ (DEC) $Date: 1993/11/09 20:32:15 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * 1.6  com/cmd/scan/tee.c, 9107320a, bos320 1/11/91 15:36:28
 */

/*
 *
 * tee-- pipe fitting.	Displays the  output of  a
 *	program and  copies it into a file.
 */                                                                   

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include "tee_msg.h"

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_TEE,Num,Str) 

extern char *optarg;
extern int optind, optopt;


#define MAXF 20			/* # output files simultaneously open */
				/* POSIX requires at least 13 */

static int openf[MAXF] = { 1 };	/* Always write to stdout */
static int n = 1;		/* Initialized to first free output slot */

static char in[BUFSIZ];


main(int argc,char **argv)
{
	int exstat = 0;
	int aflag = 0;
	int i,c,w;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_TEE, NL_CAT_LOCALE);

	while ((c = getopt(argc,argv, "ai")) != -1) {
		switch(c) {
		case 'a':			/* append mode            */
			aflag++;
			break;
		case 'i':			/* ignore the kill signal */
			signal(SIGINT, SIG_IGN);
			break;
		default:
			fprintf(stderr,
			  MSGSTR(USAGE,"Usage: tee [-ai] [file ...]\n"));
			exstat++;
			exit(exstat);
		}
	}

	(void) lseek(1,0L,SEEK_CUR);		/* Fail on PIPE, but who cares? */

	for ( ; optind < argc; optind++ ) {
	  if(n>MAXF) {
	    fprintf(stderr, MSGSTR(TOOMANY,"tee: too many files\n"));
	    exit(++exstat);
	  }
	  openf[n] = open(argv[optind],O_WRONLY|O_CREAT|(aflag?O_APPEND:O_TRUNC), 0666);
	  if(openf[n++] < 0) {
	    fprintf(stderr,MSGSTR(NO_FILE,"tee: cannot open %s\n"), argv[optind]);
	    n--;
	    exstat++;
	  }
	}

	for(;;) {
	  w = read(0, in, BUFSIZ);
	  if (w <= 0)			/* Input error or EOF */
	    exit(exstat);

	  for(i=0; i<n; i++)
	    if (write(openf[i], in, w) == -1)
	      exstat++;			/* Remember output errors */
	}
}
