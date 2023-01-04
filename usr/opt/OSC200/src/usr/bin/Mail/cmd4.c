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
static char rcsid[] = "@(#)$RCSfile: cmd4.c,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/09/07 18:16:09 $";
#endif
/*
 * HISTORY
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:cmd4.c	1.4"
#

#include "rcv.h"
#include <errno.h>

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

char *c4snarf();

#ifdef ASIAN_I18N
extern long cvtfile_size();
#endif /* ASIAN_I18N */

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * More commands..
 */


/*
 * pipe messages to cmd.
 */

dopipe(str)
	char str[];
{
	register int *ip, mesg;
	register struct message *mp;
	char *cp, *cmd;
	int f, *msgvec, lc, t, nowait=0;
	long cc;
	register int pid;
	int page, s, pivec[2], (*sigint)();
	char *Shell;
	FILE *pio;


	msgvec = (int *) salloc((msgCount + 2) * sizeof *msgvec);
	if ((cmd = c4snarf(str, &f)) == NULLSTR) {
		if (f == -1) {
			printf(MSGSTR(CMDERR,"pipe command error\n"));
			return(1);
			}
		if ( (cmd = value("cmd")) == NULLSTR) {
			printf(MSGSTR(CMDNS,"\"cmd\" not set, ignored.\n"));
			return(1);
			}
	}
	if (!f) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == NULL) {
			printf(MSGSTR(NOMSG,"No messages to pipe.\n"));
			return(1);
		}
		msgvec[1] = NULL;
	}
	if (f && getmsglist(str, msgvec, 0) < 0)
		return(1);
	if (*(cp=cmd+strlen(cmd)-1)=='&'){
		*cp=0;
		nowait++;
		}
	if ((cmd = expand(cmd)) == NULLSTR)
		return(1);
	printf(MSGSTR(PIPETO, "Pipe to: \"%s\"\n"), cmd);
	flush();

					/*  setup pipe */
	if (pipe(pivec) < 0) {
		perror("pipe");
		/* signal(SIGINT, sigint) */
		return(0);
	}

	if ((pid = vfork()) == 0) {
		close(pivec[1]);	/* child */
		fclose(stdin);
		dup(pivec[0]);
		close(pivec[0]);
		setuid(getuid());
		setgid(getgid());
		if ((Shell = value("SHELL")) == NULLSTR || *Shell=='\0')
			Shell = SHELL;
		execlp(Shell, Shell, "-c", cmd, 0);
		perror(Shell);
		_exit(1);
	}
	if (pid == -1) {		/* error */
		perror("fork");
		close(pivec[0]);
		close(pivec[1]);
		return(0);
	}

	close(pivec[0]);		/* parent */
	pio=fdopen(pivec[1],"w");

					/* send all messages to cmd */
	page = (value("page")!=NULLSTR);
	cc = 0L;
	lc = 0;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
#ifdef ASIAN_I18N
		if ((t = send(mp, pio, 0, 1, NOCHANGE)) < 0) {
#else
		if ((t = send(mp, pio)) < 0) {
#endif
			perror(cmd);
			return(1);
		}
		lc += t;
#ifdef ASIAN_I18N
		cc += cvtfile_size(mp);
#else
		cc += mp->m_size;
#endif
		if (page) putc('\f', pio);
	}

	fflush(pio);
	if (ferror(pio))
	      perror(cmd);
	fclose(pio);

					/* wait */
	if (!nowait){
		while (wait(&s) != pid);
		s &= 0377;
		if (s != 0) {
			printf(MSGSTR(PIPETOF,"Pipe to \"%s\" failed\n"), cmd);
			goto err;
		}
	}

	printf("\"%s\" %d/%ld\n", cmd, lc, cc);
	return(0);

err:
	/* signal(SIGINT, sigint); */
	return(0);
}


/* This was added for SVID-2
 * the SVID-2 definition "pipe [msgstr] [command]" is somewhat ambiguous;
 * SVRV 3.2 does not accept "pipe msgstr".
 *
 *
 * inputs:	char* string	- input string to parse
 *       	int*  flag	- status flag to return
 *
 * return values::
 * function ret: pointer to a valid command, or NULLSTR if no command found
 * flag value:	 1 if a valid msgstr was found, 0 if not
 * 
 *
 * command - input string		return_value_pointer	flag
 * 
 *    pipe  msgstr cmd			cmd			 1
 *    pipe  cmd (env var cmd not set)	cmd			 0
 *    pipe  msgstr (env var cmd set)	NULLSTR			 1
 *    pipe  msgsrtr cmd			cmd			 1
 *    pipe				NULLSTR			 0
 *
 *    syntax error			NULLSTR			-1
 *
 *    pipe  cmd msgstr			msgstr			 1 ***
 *
 * *** the above line is a syntax error which will cause the pipe command
 * to either fail or return unwanted results unless cmd just happens to be
 * a valid msgstr and msgstr just happens to be a valid system command
 */

char *
c4snarf(str, flag)
	char *str;		/* input line to parse */
	int  *flag;		/* extra return value */
{
	int tokens;		/* # of tokens found */
	int cmd_def;		/* 0 if environment variable cmd not set */
	char *smsg, *scmd;	/* pointers to msglist and cmd */
	char cmd_begin;		/* used if cmd is quoted */
#ifdef ASIAN_I18N
	int mb;
#endif

	tokens = cmd_def = 0;
	smsg = scmd = (char*)0;


	if (value("cmd") != NULLSTR)
		cmd_def = 1;		/* cmd is defined */

	/* skip over all leading white space before msglist, set smsg to first
	 * non-blank char
	 */
	smsg = str;
#ifdef ASIAN_I18N
	while(mb_any(smsg, "\t ", &mb))
		smsg+=mb;
#else
	while(any(*smsg, "\t "))
		smsg++;
#endif

	/* if any tokens in input string parse it, otherwise exit
	 * tokens is already 0
	 */
	if( strlen(smsg) > 0)
	{
		/* start at the end of the input string, & try to find a cmd */
		/* look for command starting from end of string, because
		 * msgstr can be more than 1 white space separated token -
		 * "pipe 1 2 3 pg"
		 */

		/* strip off trailing white space, starting from end */
		scmd = (str + strlen(str) -1);
#ifdef ASIAN_I18N
		while( mb_any(scmd, "\t ", &mb)) {
			*scmd = '\0';
			scmd-=mb;
		}
#else
		while( any(*scmd, "\t "))
			*scmd-- = '\0';
#endif

		/* see if string is quoted */
		if( any(*scmd, "'\""))
		{
			cmd_begin = *scmd;	/* save quote char */
			*scmd = '\0';
			while(scmd > smsg && *scmd != cmd_begin)
				scmd--;
			/* if beginning of string, and no quote, ERROR */
			if(*scmd != cmd_begin)
			{
				printf(MSGSTR(NOQUOTE,
				"Syntax error: missing %c.\n"), cmd_begin);
				*flag = -1;
				return(NULLSTR);
			}
			/* if there is only 1 token and cmd environ var is
			 * set, then the msgstr is quoted, so replace ending
			 * quote; otherwise, it is a cmd - zero beginning quote
			 * whether there are 1 or 2 tokens in input string
			 */
			if(scmd == smsg && cmd_def)
				*(scmd +(strlen(scmd))) = cmd_begin;
			else
			{
				*scmd++ = '\0';
				smsg++;	 /* adv past quote, for # tokens */
			}
		}
		else	/* no quote, look for white space or start of line */
		{
#ifdef ASIAN_I18N
			while(scmd > smsg && !mb_any(scmd, "\t ", &mb))
#else
			while(scmd > smsg && !any(*scmd, "\t "))
#endif
				scmd--;
			/* if not beginning of string, zero white space char
			 * to separate cmd from msgstr
			 */
			if(scmd > smsg)
				*scmd++ = '\0';
		}

		if( smsg == scmd)	/* if last token is first token,,, */
			tokens = 1;
		else
			tokens = 2;

	} /* end parsing tokens in str */


	/* return flag and value here */
	switch(tokens)
	{
		case 0:
			*flag = 0;		/* no msgstr found */
			return(NULLSTR);		/* no cmd found */
			break;
		case 1:
			if(!cmd_def)
			{
				*flag = 0;	/* no msgstr found */
				return(scmd);	/* pointer to cmd */
			}
			else
			{
				*flag = 1;	/* msgstr found */
				return(NULLSTR);	/* no cmd found */
			}
			break;
		case 2:
			*flag = 1;		/* msgstr found */
			return(scmd);		/* pointer to cmd */
			break;
		default:
			*flag = -1;		/* error */
			return(NULLSTR);		/* error */
			break;
	}

	/*NOTREACHED*/
}

