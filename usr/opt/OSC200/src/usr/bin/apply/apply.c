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
static char rcsid[] = "@(#)$RCSfile: apply.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 15:35:49 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * 1.9  com/cmd/sh/apply.c, cmdsh, bos320, 9126320 6/7/91 19:14:48
 */
/*
 * NAME:  apply - apply a command to a set of arguments
 *
 * FUNCTION: 
 *      apply [-aCharacter] [-n] command args ...
 *      -a    character of your choice c.
 *      -n    number of arguments to be passed to the commmand.
 * NOTE:
 *	apply echo * == ls
 *	apply -2 cmp A1 B1 A2 B2   compares A's with B's
 *	apply "ln %1 /usr/fred/dir" *  duplicates a directory
 *        If your are having problems getting apply to work quote the command.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <sys/errno.h>

#include <ctype.h>
#include "apply_msg.h" 
#include "pathnames.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_APPLY,n,s) 

void usage();
char	*cmdp;
#define	NCHARS 512

wchar_t	cmd[NCHARS];

char	defargs=1;
#define	DEFARGCHAR	'%'

wchar_t	argchar=DEFARGCHAR;

int	nchars;

main(argc, argv)
int argc;
char *argv[];
{
	register int n;

	(void) setlocale (LC_ALL, "");

	catd = catopen(MF_APPLY,NL_CAT_LOCALE);

	while(argc>2 && argv[1][0]=='-'){
		if(argv[1][1]=='a'){

			mbtowc(&argchar,&argv[1][2],MB_CUR_MAX);

			if(argchar=='\0')
				argchar=DEFARGCHAR;
		} else {
			char *end;

			errno = 0;
			defargs = strtol(&argv[1][1], &end, 10);
			if (errno != 0 || end == &argv[1][1]) {
				usage();
				exit(1);
			}
			if((int)defargs < 0)
				defargs = 1;
		}
		--argc; ++argv;
	}

	if(argc<3){
	  	usage();
		exit(1);
	}

	argc -= 2;
	cmdp = argv[1];
	argv += 2;
	while(n=docmd(argc, argv)){
		argc -= n;
		argv += n;
	}
	exit(0);
}

/*
 * NAME: addc
 *
 * FUNCTION: add character to command
 */
wchar_t addc(c)
char **c;
{
	wchar_t tmp;
	int x;
	if(nchars++>=NCHARS){

		fprintf(stderr, "apply: command too long\n");
		exit(1);
	}

	x=mbtowc(&tmp,*c,MB_CUR_MAX);
	*c+=x;
	return (tmp);
}

/*
 * NAME: addarg
 *
 * FUNCTION: check length of s and copy to t.
 */
wchar_t *addarg(s, t)
	char *s;
	register wchar_t *t;
{
	while(*t = addc(&s))
		*t++;
	return(t);
}

/*
 * NAME: docmd
 *
 * FUNCTION:  get command from argv
 */
docmd(argc, argv)
	char *argv[];
{
	char *p;
	wchar_t *q;
	register max, i;
	char gotit;
	if(argc<=0)
		return(0);
	nchars = 0;
	max = 0;
	gotit = 0;
	p = cmdp;
	q = cmd;
	while(*q = addc(&p)){
		if(*q++!=argchar || *p<'1' || '9'<*p)
			continue;
		if((i= *p++-'1') > max)
			max = i;
		if(i>=argc){
	Toofew:
			fprintf(stderr, MSGSTR(TOOFEWARGS, "apply: expecting argument(s) after `%s'\n"), argv[argc-1]); /*MSG*/
			exit(1);
		}
		q = addarg(argv[i], q-1);
		gotit++;
	}
	if(defargs!=0 && gotit==0){
		if(defargs>argc)
			goto Toofew;
		for(i=0; i<defargs; i++){
		    char *dummy = " ";
			*q++ = addc(&dummy);
			q = addarg(argv[i], q);
		}
	}
	i = eXec(cmd);
	if(i == 127){
		fprintf(stderr, MSGSTR(NOSHELL, "apply: no shell!\n")); /*MSG*/
		exit(1);
	}
	return(max==0? (defargs==0? 1 : defargs) : max+1);
}

/*
 * NAME: eXec
 * FUNCTION: execute command s
 */
eXec(s)
wchar_t *s;
{
char tmpbuf[NCHARS];
	int status, pid, w;
	char *shell = getenv("SHELL");

	wcstombs(tmpbuf,s,sizeof(tmpbuf));
	if ((pid = fork()) == 0) {

		execl(shell ? shell : _PATH_SH, "sh", "-c", tmpbuf, 0);
		exit(1);
	}
	if(pid == -1){
		fprintf(stderr, MSGSTR(NOFORK, "apply: can't fork\n")); /*MSG*/
		exit(1);
	}
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	return(status);
}

void
usage()
{
	fprintf(stderr, MSGSTR(USAGE, 
		"usage: apply [-aCharacter] [-Number] Command Argument...\n"));
}
