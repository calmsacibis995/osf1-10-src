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
static char rcsid[] = "@(#)$RCSfile: main.c,v $ $Revision: 4.3.8.7 $ (DEC) $Date: 1993/11/22 19:02:53 $";
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *	1.34  com/cmd/sh/sh/main.c, cmdsh, bos320, 9132320b 8/1/91 08:52:18
 */

#include        <sys/types.h>
#include	<unistd.h>
#include        "defs.h"
#include        "sym.h"
#include        "timeout.h"
#include        <sys/stat.h>
#include	<fcntl.h>
#include	<sys/ioctl.h>	
#include	<sys/termios.h>	

#include	<locale.h>

#define               SVR4_SH         "/usr/opt/svr4/usr/bin/sh"
#define               XPG4_KSH        "/usr/bin/ksh"
#define               __SH_SVR4 "svr4"
#define               __SH_XPG4 "xpg4"

static BOOL     beenhere = FALSE;
#define _TMP_SH "/tmp/sh"
uchar_t         tmpout[26] = _TMP_SH;
struct fileblk  stdfile;
struct fileblk *standin = &stdfile;
int mailchk = 0;
int timeout = TIMEOUT;
int timecroak;

static uchar_t     *mailp;
static time_t     *mod_time = 0;
static int      exfile();
extern uchar_t     *simple();
extern void	settmp();
extern int	_open_max;
extern struct fdsave *fdmap;

nl_catd catd;           /* message catalog descriptor */

main(int c, uchar_t *v[], uchar_t *e[])
{
	register uchar_t	*shname;
	register int    rflag = ttyflg;
	int             rsflag = 1;     /* local restricted flag */
	struct namnod   *n;
	int	fd_tty;			/* fd of /dev/tty	*/
	extern void stdsigs();
	extern int setsig();
	uchar_t         vec0[4];
	uchar_t         vec1[3];
	uchar_t		**vector;
	char *getptr = NULL;
	int i;

	if(**v == '-')
		stdsigs();
	else
		if((c == 1) || (strcmp((char *)v[1], "-c") != 0))
			specsigs();

	setlocale (LC_ALL, "");
	catd = catopen(MF_SH, NL_CAT_LOCALE);

	/*
	 * setup for max number of file pointers
	 */
	if( (_open_max = sysconf(_SC_OPEN_MAX)) == -1) {
		perror(MSGSTR(M_SYSCONF,"sh: sysconf error"));
		exit(ERROR);
	}
	if( (fdmap = (struct fdsave *)malloc( 
		_open_max * (sizeof(struct fdsave)))) == NULL) {
		perror(MSGSTR(M_NOMEM,"sh: no memory"));
		exit(ERROR);
	}

	/*
	 * set names from userenv
	 */
	setup_env();

	/*
	 * 'rsflag' is non-zero if SHELL variable is
	 *  set in environment and has a simple name of
	 *  'Rsh', '-Rsh', 'rsh' or '-rsh'.
	 */
	if ((n = findnam("SHELL")) && n->namval != NULL)
	{
		if (*(shname = simple(n->namval)) == '-')
			++shname;
		if (   (strcmp((char *)shname,"rsh") == 0)
		    || (strcmp((char *)shname,"Rsh") == 0))
			rsflag = 0;
	}

	/*
	 * a shell is also restricted if argv(0) has
	 * a simple name of 'Rsh', '-Rsh', 'rsh' or '-rsh'.
	 */

	if (c > 0) {
		if (*(shname = simple(*v)) == '-')
			++shname;
		if (   (strcmp((char *)shname,"rsh") == 0)
		    || (strcmp((char *)shname,"Rsh") == 0))
			rflag = 0;
	}

	/* DS001 Changed the code to switch on the value of the
	 * environment variable BIN_SH. If the value is __SH_XPG4,
	 * exec the defined program XPG4_KSH. If it is __SH_SVR4
	 * exec the defined program SVR4_SH. If BIN_SH is not
	 * defined or defined to anything but these two, give
	 * an error and exit.
	 */

	if ((getptr = getenv( "BIN_SH" )) != (char *) NULL)
	{
		if (!strcmp(getptr, __SH_XPG4))
		{

			sprintf(vec0, "ksh\0");
			v[0] = vec0; /* Change program name vector */

			if (rsflag == 0 || rflag == 0) {
				/* restricted shell. Invoke ksh with "-r". */
				vector = (uchar **) 
					malloc(sizeof(uchar *) * (c + 1));
				vector[0] = v[0];
				sprintf(vec1, "-r\0");
				vector[1] = vec1;

				for (i = 1; i < c; i++) {
					vector[i+1] = v[i];
				}

			}
			else {
				vector = v;
			}
			execve(XPG4_KSH, vector, e);
			fprintf(stderr, MSGSTR(M_SHMISS,
				"%s specified but %s is missing.\n"),
				getptr, XPG4_KSH);
			exit(ERROR);
		}
		else if (!strcmp(getptr, __SH_SVR4))
		{
			execve(SVR4_SH, v, e);
			fprintf(stderr, MSGSTR(M_SHMISS,
				"%s specified but %s is missing.\n"),
				getptr, SVR4_SH);
			exit(ERROR);
		}
		fprintf(stderr, MSGSTR(M_BADSH,
			"Bad BIN_SH value %s.\n"), getptr);
		exit(ERROR);
	}

	hcreate();
	set_dotpath();

	/*
	 * look for options
	 * dolc is $#
	 */
	eflag = 0;	/* fail on error flag */
	dolc = options(c, v);

	if (dolc < 2)
	{
		flags |= stdflg;
		{
			register uchar_t *flagc = flagadr;

			while (*flagc)
				flagc++;
			*flagc++ = STDFLG;
			*flagc = 0;
		}
	}
	if ((flags & stdflg) == 0)
		dolc--;
	dolv = v + c - dolc;
	dolc--;

	/*
	 * return here for shell file execution
	 * but not for parenthesis subshells
	 */
	setjmp(subshell);

	/*
	 * number of positional parameters
	 */
	replace(&cmdadr, dolv[0]);      /* cmdadr is $0 */

	/*
	 * set pidname '$$'
	 */
	assnum(&pidadr, getpid());

	/*
	 * set up temp file names
	 */
	settmp();

	/*
	 * default internal field separators - $IFS
	 */
	dfault(&ifsnod, sptbnl);

	dfault(&mchknod, MAILCHECK);
	mailchk = stoi(mchknod.namval);
	if(timenod.namval)
		timeout = stoi(timenod.namval);

	/*
	 * catch longjmps that are not caught elsewhere
	 */
	if (setjmp(errshell))
	{
		exit(ERROR);
	}

	if ((beenhere++) == FALSE)      /* ? profile */
	{
		if (*(simple(cmdadr)) == '-')
		{                       /* system profile */

			if ((input = pathopen(nullstr, sysprofile)) >= 0)
				exfile(rflag);          /* file exists */

			if ((input = pathopen(nullstr, profile)) >= 0)
			{
				exfile(rflag);
				flags &= ~ttyflg;
			}
		}
		if (rsflag == 0 || rflag == 0)
			flags |= rshflg;
		/*
		 * open input file if specified
		 */
		if (comdiv)
		{
			estabf(comdiv);
			input = -1;
		}
		else
		{
				/*	Set cmdadr to argv[0] so if
				*	chkopen fails it will print
				*	"sh: file: not found"
				*/
			uchar_t	*oldcmd = cmdadr;
			cmdadr = v[0];
			input = ((flags & stdflg) ? 0 : chkopen(oldcmd));

#ifdef ACCT
			if (input != 0)
				preacct(oldcmd);
#endif
			cmdadr = oldcmd;
			comdiv--;
		}
	}


	exfile(0);
	done();
}

long    mailtime;       /* last time mail was checked */

static int
exfile(prof)
BOOL prof;
{
	jmp_buf savebuf;
	time_t    curtime = 0;
	register int    userid;

	/*
	 * preserve the previous longjmp context
	 */
	memcpy(savebuf,errshell,sizeof(savebuf));
	/*
	 * move input
	 */
	if (input > 0)
	{
		Ldup(input, INIO);
		input = INIO;
	}

	userid = geteuid();

	/*
	 * decide whether interactive
	 */
	if ((flags & intflg) ||
	    ((flags&oneflg) == 0 &&
	    isatty(output) &&
	    isatty(input)) )

	{
		uchar_t    *stdprompt, *supprompt;
		
		stdprompt = shstdprompt;
		supprompt = shsupprompt;
		
		dfault(&ps1nod, (userid ? stdprompt : supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg | prompt;
		ignsig(SIGTERM);
		if (mailpnod.namflg != N_DEFAULT)
			setmail(mailpnod.namval);
		else
			setmail(mailnod.namval);
	}
	else
	{
		flags |= prof;
		flags &= ~prompt;
	}

	if (setjmp(errshell) && prof)
	{
		memcpy(errshell,savebuf,sizeof(errshell));
		close(input);
		return;
	}
	/*
	 * error return here
	 */

	loopcnt = peekc = peekn = 0;
	fshift = 0;
	fndef = 0;
	nohash = 0;
	iopend = 0;

	if (input >= 0)
		initf(input);
	/*
	 * command loop
	 */
	for (;;)
	{
		tdystak(0);
		stakchk();      /* may reduce sbrk */
		exitset();

		if ((flags & prompt) && standin->fstak == 0 && !eof)
		{

			timecroak = 0;
			if (mailp || timeout)
			{
				time(&curtime);
				if(timeout)
					timecroak = curtime + 60*timeout;
				if (mailp && (curtime - mailtime) >= mailchk)
				{
					(void)chkmail();
					mailtime = curtime;
				}
			}

			prs(ps1nod.namval);
			if(timeout || mailp && mailchk)
				alarm(mailchk < 60 * timeout ? mailchk
							     : 60 * timeout);
			flags |= waiting;
		}

		trapnote = 0;
		peekc = readc();
		if (eof)
			break;

		alarm(0);

		flags &= ~waiting;

		execute(cmd(NL, MTFLG), 0, eflag);
		eof |= (flags & oneflg);
	}
	/*
	 * restore the previous longjmp context
	 */
	memcpy(errshell,savebuf,sizeof(errshell));
}

void
chkpr()
{
	if ((flags & prompt) && standin->fstak == 0)
		prs(ps2nod.namval);
}

void
settmp()
{
	itos(getpid()); serial = 0;
	tempname = movstr(numbuf, &tmpout[strlen(_TMP_SH)]);
}

int
Ldup(int fa, int fb)
{
	dup2(fa, fb);
	close(fa);
	ioctl(fb, FIOCLEX, 0);
}


int
chkmail()
{
	register uchar_t   *s = mailp;
	register uchar_t   *save;

	time_t    *ptr = mod_time;
	uchar_t    *start;
	BOOL    flg;
	struct stat     statb;

	while (s && *s)
	{
		start = s;
		save = 0;
		flg = 0;

		while (*s)
		{
			if (*s != COLON)
			{
				if (*s == '%' && save == 0)
					save = s;

				s++;
			}
			else
			{
				flg = 1;
				*s = 0;
			}
		}

		if (save)
			*save = 0;

		if (*start && stat((char *)start, &statb) >= 0)
		{
			if(statb.st_size && *ptr
				&& statb.st_mtime != *ptr)
			{
				if (save)
				{
					prs(save+1);
					newline();
				}
				else
				{
					if (mailmnod.namflg != N_DEFAULT)
					{
						prs(mailmnod.namval);
						newline();
					} else
						prs(MSGSTR(M_MAILMSG,(char *)mailmsg));
				}
			}
			*ptr = statb.st_mtime;
		}
		else if (*ptr == 0)
			*ptr = 1;

		if (save)
			*save = '%';

		if (flg)
			*s++ = COLON;

		ptr++;
	}
}

int
setmail(uchar_t *mailpath)
{
	register uchar_t   *s = mailpath;
	register int    cnt = 1;

	time_t    *ptr;

	alloc_free((void *)mod_time);
	if (mailp = mailpath)
	{
		while (*s)
		{
			if (*s == COLON)
				cnt += 1;

			s++;
		}

		ptr = mod_time = (time_t *)malloc(sizeof(long) * cnt);

		while (cnt)
		{
			*ptr = 0;
			ptr++;
			cnt--;
		}
	}
}
