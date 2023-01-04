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
static char rcsid[] = "@(#)$RCSfile: cmds.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/30 18:39:34 $";
#endif

/* cmds.c	1.5  com/cmd/tip,3.1,9013 12/21/89 16:48:40"; */
/* 
 * COMPONENT_NAME: UUCP cmds.c
 * 
 * FUNCTIONS: MSGSTR, abort, anyof, args, chdirectory, consh, cu_put, 
 *            cu_take, execute, expand, finish, genbrk, getfl, 
 *            intcopy, pipefile, pipeout, prtime, send, sendfile, 
 *            setscript, shell, stopsnd, suspend, tandem, timeout, 
 *            transfer, transmit, variable 
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

/* static char sccsid[] = "cmds.c	5.4 (Berkeley) 5/5/86"; */

#define BUFSIZE 1024
#include "tip.h"
#include "pathnames.h"

/*
 * tip
 *
 * miscellaneous commands
 */

int	quant[] = { 60, 60, 24 };

char	null = '\0';
char	*sep[] = { "second", "minute", "hour" };
static char *argv[10];		/* argument vector for take and put */

void	timeout();		/* timeout function called on alarm */
void	stopsnd();		/* SIGINT handler during file transfers */
void	intprompt();		/* used in handling SIG_INT during prompt */
void	intcopy();		/* interrupt routine for file transfers */

extern int bauds[];             /* array of legal baud rates */

/*
 * FTP - remote ==> local
 *  get a file from the remote host
 */
getfl(c)
	char c;
{
	char buf[256], *cp, *expand();
	
	putchar(c);
	/*
	 * get the UNIX receiving file's name
	 */
	if (prompt(MSGSTR(LOCALFILE, "Local file name? "), copyname)) /*MSG*/
		return;
	cp = expand(copyname);
	if ((sfd = creat(cp, 0666)) < 0) {
		printf(MSGSTR(CANTCREAT, "\r\n%s: cannot creat\r\n"), copyname); /*MSG*/
		return;
	}
	
	/*
	 * collect parameters
	 */
	if (prompt(MSGSTR(LISTCOMM, "List command for remote system? "), buf)) { /*MSG*/
		unlink(copyname);
		return;
	}
	transfer(buf, sfd, value(EOFREAD));
}

/*
 * Cu-like take command
 */
cu_take(cc)
	char cc;
{
	int fd, argc;
	char line[BUFSIZE], *expand(), *cp;

	if (prompt(MSGSTR(TAKE, "[take] "), copyname)) /*MSG*/
		return;
	if ((argc = args(copyname, argv)) < 1 || argc > 2) {
		printf(MSGSTR(USAGE, "usage: <take> from [to]\r\n")); /*MSG*/
		return;
	}
	if (argc == 1)
		argv[1] = argv[0];
	cp = expand(argv[1]);
	if ((fd = creat(cp, 0666)) < 0) {
		printf(MSGSTR(CANTCREAT2, "\r\n%s: cannot create\r\n"), argv[1]); /*MSG*/
		return;
	}
#ifdef _DEBUG
	fprintf(stderr, "cu_take: fd = %d\n", fd);
#endif
	sprintf(line, "/usr/bin/sh -c \"cat %s; echo '\\01'\"", argv[0]);
	transfer(line, fd, "\01");
}

static	jmp_buf intbuf;
/*
 * Bulk transfer routine --
 *  used by getfl(), cu_take(), and pipefile()
 */
transfer(buf, fd, eofchars)
	char *buf, *eofchars;
	int fd;
{
	register int ct;
	char c, buffer[BUFSIZE];
	register char *p = buffer;
	register int cnt, eof;
	time_t start;
	sig_t f;
	char creturn[2] = { '\r', 0};
        char *parity;
        parity = value(PARITY);


	pwrite(FD, buf, size(buf));
	quit = 0;
	kill(pid, SIGIOT);
	read(repdes[0], (char *)&ccc, 1);  /* Wait until read process stops */
	
	/*
	 * finish command
	 */
	pwrite(FD, creturn, 1);
	if(strncmp(parity,"none",4) != 0)
        {
	do
		read(FD, &c, 1); 
		while ((c&0177) != '\n');
        }
        else
        {
        do
                read(FD, &c, 1);
                while ((c) != '\n');
        }
                                     
	ioctl(0, TIOCSETC, &defchars);
	
	(void) setjmp(intbuf);
	f = signal(SIGINT, intcopy);
	start = time(0);
	for (ct = 0; !quit;) {
		eof = (read(FD, &c, 1) <= 0);
		if(strncmp(parity,"none",4) != 0)
		   c &= 0177;
#ifdef _DEBUG
	fprintf(stderr,"read(%c) = %d\n",c,eof);
#endif
		if (quit)
			continue;
		if (eof || any(c, eofchars))
			break;
		if (c == 0)
			continue;	/* ignore nulls */
		if (c == '\r')
			continue;
		*p++ = c;

		if (c == '\n' && boolean(value(VERBOSE)))
			printf("\r%d", ++ct);
		if ((cnt = (p-buffer)) == number(value(FRAMESIZE))) {
#ifdef _DEBUG
		fprintf(stderr,"\n\nwrite(%d,buffer,%d)\n\n\n",fd,cnt);
#endif
			if (write(fd, buffer, cnt) != cnt) {
				printf(MSGSTR(WRITERR, "\r\nwrite error\r\n")); /*MSG*/
				quit = 1;
			}
			p = buffer;
		}
	}
	if (cnt = (p-buffer)) {
#ifdef _DEBUG
		fprintf(stderr,"\n\nwrite(%d,buffer,%d)\n\n\n",fd,cnt);
#endif
		if (write(fd, buffer, cnt) != cnt)
			printf(MSGSTR(WRITERR, "\r\nwrite error\r\n")); /*MSG*/
	}

	if (boolean(value(VERBOSE)))
		prtime(MSGSTR(LINESXFER, " lines transferred in "), time(0)-start); /*MSG*/
	ioctl(0, TIOCSETC, &tchars);
	write(fildes[1], (char *)&ccc, 1);
	signal(SIGINT, f);
	close(fd);
}

/*
 * FTP - remote ==> local process
 *   send remote input to local process via pipe
 */
pipefile()
{
	int cpid, pdes[2];
	char buf[256];
	int status, p;
	extern int errno;

	if (prompt(MSGSTR(LOCALCOMM, "Local command? "), buf)) /*MSG*/
		return;

	if (pipe(pdes)) {
		printf(MSGSTR(CANTPIPE, "can't establish pipe\r\n")); /*MSG*/
		return;
	}

	if ((cpid = fork()) < 0) {
		printf(MSGSTR(CANTFORK, "can't fork!\r\n")); /*MSG*/
		return;
	} else if (cpid) {
		if (prompt(MSGSTR(LISTCOMM, "List command for remote system? "), buf)) { /*MSG*/
			close(pdes[0]), close(pdes[1]);
			kill (cpid, SIGKILL);
		} else {
			close(pdes[0]);
			signal(SIGPIPE, (void(*)(int)) intcopy);
			transfer(buf, pdes[1], value(EOFREAD));
			signal(SIGPIPE, SIG_DFL);
			while ((p = wait(&status)) > 0 && p != cpid)
				;
		}
	} else {
		register int f;

		dup2(pdes[0], 0);
		close(pdes[0]);
		for (f = 3; f < 20; f++)
			close(f);
		execute(buf);
		printf(MSGSTR(CANTEXECL, "can't execl!\r\n")); /*MSG*/
		exit(0);
	}
}

/*
 * Interrupt service routine for FTP
 */
void
stopsnd()
{

	stop = 1;
	signal(SIGINT, SIG_IGN);
}

/*
 * FTP - local ==> remote
 *  send local file to remote host
 *  terminate transmission with pseudo EOF sequence
 */
sendfile(cc)
	char cc;
{
	FILE *fd;
	char *fnamex;
	char *expand();

	putchar(cc);
	/*
	 * get file name
	 */
	if (prompt(MSGSTR(LOCALFILE, "Local file name? "), fname)) /*MSG*/
		return;

	/*
	 * look up file
	 */
	fnamex = expand(fname);
	if ((fd = fopen(fnamex, "r")) == NULL) {
		printf(MSGSTR(CANTOPEN, "%s: cannot open\r\n"), fname); /*MSG*/
		return;
	}
	transmit(fd, value(EOFWRITE), NULL);
	if (!boolean(value(ECHOCHECK))) {
		 struct sgttyb buf;

		ioctl(FD, TIOCGETP, &buf);	/* this does a */
		ioctl(FD, TIOCSETP, &buf);	/*   wflushtty */
	}
}

/*
 * Bulk transfer routine to remote host --
 *   used by sendfile() and cu_put()
 */
transmit(fd, eofchars, command)
	FILE *fd;
	char *eofchars, *command;
{
	char *pc, lastc;
	int c, ccount, lcount;
	time_t start_t, stop_t;
	sig_t f;
        char *parity;
        parity = value(PARITY);


	kill(pid, SIGIOT);	/* put TIPOUT into a wait state */
	stop = 0;
	f = signal(SIGINT, stopsnd);
	ioctl(0, TIOCSETC, &defchars);
	read(repdes[0], (char *)&ccc, 1);
	if (command != NULL) {
		for (pc = command; *pc; pc++)
			send(*pc);
		if (boolean(value(ECHOCHECK)))
			read(FD, (char *)&c, 1);	/* trailing \n */
		else {
			struct sgttyb buf;

			ioctl(FD, TIOCGETP, &buf);	/* this does a */
			ioctl(FD, TIOCSETP, &buf);	/*   wflushtty */
			sleep(5); /* wait for remote stty to take effect */
		}
	}
	lcount = 0;
	lastc = '\0';
	start_t = time(0);
	while (1) {
		ccount = 0;
		do {
			c = getc(fd);
			if (stop)
				goto out;
			if (c == EOF)
				goto out;
			if (c == 0177 && !boolean(value(RAWFTP)))
				continue;
			lastc = c;
			if (c < 040) {
				if (c == '\n') {
					if (!boolean(value(RAWFTP)))
						c = '\r';
				}
				else if (c == '\t') {
					if (!boolean(value(RAWFTP))) {
						if (boolean(value(TABEXPAND))) {
							send(' ');
							while ((++ccount % 8) != 0)
								send(' ');
							continue;
						}
					}
				} else
					if (!boolean(value(RAWFTP)))
						continue;
			}
			send(c);
		} while (c != '\r' && !boolean(value(RAWFTP)));
		if (boolean(value(VERBOSE)))
			printf("\r%d", ++lcount);
		if (boolean(value(ECHOCHECK))) {
			timedout = 0;
			alarm((int) value(ETIMEOUT));
                       if(strncmp(parity,"none",4) != 0)
                        {
                                do {    /* wait for prompt */
                                        read(FD, (char *)&c, 1);
                                        if (timedout || stop) {
                                                if (timedout)
                                                        printf(MSGSTR(TIMEDOUT, "\r\ntimed out at eol\r\n")); /*MSG*/
                                                alarm(0);
                                                goto out;
                                        }
                                } while ((c&0177) != character(value(PROMPT)));
                        }
                        else
                        {
                                do {    /* wait for prompt */
                                        read(FD, (char *)&c, 1);
                                        if (timedout || stop) {
                                                if (timedout)
                                                        printf(MSGSTR(TIMEDOUT, "\r\ntimed out at eol\r\n")); /*MSG*/
                                                alarm(0);
                                                goto out;
                                        }
                                } while ((c) != character(value(PROMPT)));
                        }

			alarm(0);
		}
	}
out:
	if (lastc != '\n' && !boolean(value(RAWFTP)))
		send('\r');
	if (eofchars != (char *) 0)
		for (pc = eofchars; *pc; pc++)
			send(*pc);
	stop_t = time(0);
	fclose(fd);
	signal(SIGINT, f);
	if (boolean(value(VERBOSE)))
		if (boolean(value(RAWFTP)))
			prtime(MSGSTR(CHARSXFER, " chars transferred in "), stop_t-start_t); /*MSG*/
		else
			prtime(MSGSTR(LINESXFER, " lines transferred in "), stop_t-start_t); /*MSG*/
	write(fildes[1], (char *)&ccc, 1);
	ioctl(0, TIOCSETC, &tchars);
}

/*
 * Cu-like put command
 */
cu_put(cc)
	char cc;
{
	FILE *fd;
	char line[BUFSIZE];
	int argc;
	char *expand();
	char *copynamex;

	if (prompt(MSGSTR(PUT, "[put] "), copyname)) /*MSG*/
		return;
	if ((argc = args(copyname, argv)) < 1 || argc > 2) {
		printf(MSGSTR(USAGE2, "usage: <put> from [to]\r\n")); /*MSG*/
		return;
	}
	if (argc == 1)
		argv[1] = argv[0];
	copynamex = expand(argv[0]);
	if ((fd = fopen(copynamex, "r")) == NULL) {
		printf(MSGSTR(CANTOPEN, "%s: cannot open\r\n"), copynamex); /*MSG*/
		return;
	}
	if (boolean(value(ECHOCHECK)))
		sprintf(line, "cat>%s\r", argv[1]);
	else
		sprintf(line, "stty -echo;cat>%s;stty echo\r", argv[1]);
	transmit(fd, "\04", line);
}

/*
 * FTP - send single character
 *  wait for echo & handle timeout
 */
send(c)
	char c;
{
	char cc;
	int retry = 0;

	cc = c;
	pwrite(FD, &cc, 1);
#ifdef notdef
	if (number(value(CDELAY)) > 0 && c != '\r')
		nap(number(value(CDELAY)));
#endif
	if (!boolean(value(ECHOCHECK))) {
#ifdef notdef
		if (number(value(LDELAY)) > 0 && c == '\r')
			nap(number(value(LDELAY)));
#endif
		return;
	}
tryagain:
	timedout = 0;
	alarm((int) value(ETIMEOUT));
	read(FD, &cc, 1);
	alarm(0);
	if (timedout) {
		printf(MSGSTR(TIMEOUTERR, "\r\ntimeout error (%s)\r\n"), ctrl(c)); /*MSG*/
		if (retry++ > 3)
			return;
		pwrite(FD, &null, 1); /* poke it */
		goto tryagain;
	}
}

void
timeout()
{
	signal(SIGALRM, timeout);
	timedout = 1;
}

/*
 * Stolen from consh() -- puts a remote file on the output of a local command.
 *	Identical to consh() except for where stdout goes.
 */
pipeout(c)
{
	char buf[256];
	int cpid, status, p;
	time_t start;

	putchar(c);
	if (prompt(MSGSTR(LOCALCOMM, "Local command? "), buf)) /*MSG*/
		return;
	kill(pid, SIGIOT);	/* put TIPOUT into a wait state */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	ioctl(0, TIOCSETC, &defchars);
	read(repdes[0], (char *)&ccc, 1);
	/*
	 * Set up file descriptors in the child and
	 *  let it go...
	 */
	if ((cpid = fork()) < 0)
		printf(MSGSTR(CANTFORK, "can't fork!\r\n")); /*MSG*/
	else if (cpid) {
		start = time(0);
		while ((p = wait(&status)) > 0 && p != cpid)
			;
	} else {
		register int i;

		dup2(FD, 1);
		for (i = 3; i < 20; i++)
			close(i);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execute(buf);
		printf(MSGSTR(CANTFIND, "can't find `%s'\r\n"), buf); /*MSG*/
		exit(0);
	}
	if (boolean(value(VERBOSE)))
		prtime(MSGSTR(AWAYFOR, "away for "), time(0)-start); /*MSG*/
	write(fildes[1], (char *)&ccc, 1);
	ioctl(0, TIOCSETC, &tchars);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}

/*
 * Fork a program with:
 *  0 <-> local tty in
 *  1 <-> local tty out
 *  2 <-> local tty out
 *  3 <-> remote tty in
 *  4 <-> remote tty out
 */
consh(c)
{
	char buf[256];
	int cpid, status, p;
	time_t start;

	putchar(c);
	if (prompt(MSGSTR(LOCALCOMM, "Local command? "), buf)) /*MSG*/
		return;
	kill(pid, SIGIOT);	/* put TIPOUT into a wait state */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	ioctl(0, TIOCSETC, &defchars);
	read(repdes[0], (char *)&ccc, 1);
	/*
	 * Set up file descriptors in the child and
	 *  let it go...
	 */
	if ((cpid = fork()) < 0)
		printf(MSGSTR(CANTFORK, "can't fork!\r\n")); /*MSG*/
	else if (cpid) {
		start = time(0);
		while ((p = wait(&status)) > 0 && p != cpid)
			;
	} else {
		register int i;

		dup2(FD, 3);
		dup2(3, 4);
		for (i = 5; i < 20; i++)
			close(i);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execute(buf);
		printf(MSGSTR(CANTFIND, "can't find `%s'\r\n"), buf); /*MSG*/
		exit(0);
	}
	if (boolean(value(VERBOSE)))
		prtime(MSGSTR(AWAYFOR, "away for "), time(0)-start); /*MSG*/
	write(fildes[1], (char *)&ccc, 1);
	ioctl(0, TIOCSETC, &tchars);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}

/*
 * Escape to local shell
 */
shell()
{
	int shpid, status;
	extern char **environ;
	char *cp;

	printf(MSGSTR(SH, "[sh]\r\n")); /*MSG*/
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	unraw();
	if (shpid = fork()) {
		while (shpid != wait(&status));
		raw();
		printf("\r\n!\r\n");
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		return;
	} else {
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		if ((cp = (char *) rindex(value(SHELL), '/')) == NULL)
			cp = value(SHELL);
		else
			cp++;
		shell_uid();
		execl(value(SHELL), cp, 0);
		printf(MSGSTR(CANTEXECL2, "\r\ncan't execl!\r\n")); /*MSG*/
		exit(1);
	}
}

/*
 * TIPIN portion of scripting
 *   initiate the conversation with TIPOUT
 */
setscript()
{
	char c;
	/*
	 * enable TIPOUT side for dialogue
	 */
	kill(pid, SIGEMT);
	if (boolean(value(SCRIPT)))
		write(fildes[1], value(RECORD), size(value(RECORD)));
	write(fildes[1], "\n", 1);
	/*
	 * wait for TIPOUT to finish
	 */
	read(repdes[0], &c, 1);
	if (c == 'n')
		printf(MSGSTR(CANTCREAT3, "can't create %s\r\n"), value(RECORD)); /*MSG*/
}

/*
 * Change current working directory of
 *   local portion of tip
 */
chdirectory()
{
	char dirname[80];
	register char *cp = dirname;

	if (prompt(MSGSTR(CD, "[cd] "), dirname)) { /*MSG*/
		if (stoprompt)
			return;
		cp = value(HOME);
	}
	if (chdir(cp) < 0)
		printf(MSGSTR(BADDIR, "%s: bad directory\r\n"), cp); /*MSG*/
	printf("!\r\n");
}

abort(msg)
	char *msg;
{

	kill(pid, SIGTERM);
	disconnect(msg);
	if (msg != NOSTR)
		printf("\r\n%s", msg);
	printf(MSGSTR(EOT, "\r\n[EOT]\r\n")); /*MSG*/
	daemon_uid();
	uu_unlock(uucplock);
	unraw();
	exit(0);
}

finish()
{
	char *dismsg;

	if ((dismsg = value(DISCONNECT)) != NOSTR) {
		write(FD, dismsg, strlen(dismsg));
		sleep(5);
	}
	abort(NOSTR);
}

void
intcopy()
{

	raw();
	quit = 1;
	longjmp(intbuf, 1);
}

execute(s)
	char *s;
{
	register char *cp;

	if ((cp = (char *) rindex(value(SHELL), '/')) == NULL)
		cp = value(SHELL);
	else
		cp++;
	shell_uid();
	execl(value(SHELL), cp, "-c", s, 0);
}

args(buf, a)
	char *buf, *a[];
{
	register char *p = buf, *start;
	register char **parg = a;
	register int n = 0;

	do {
		while (*p && (*p == ' ' || *p == '\t'))
			p++;
		start = p;
		if (*p)
			*parg = p;
		while (*p && (*p != ' ' && *p != '\t'))
			p++;
		if (p != start)
			parg++, n++;
		if (*p)
			*p++ = '\0';
	} while (*p);

	return(n);
}

prtime(s, a)
	char *s;
	time_t a;
{
	register i;
	int nums[3];

	for (i = 0; i < 3; i++) {
		nums[i] = (int)(a % quant[i]);
		a /= quant[i];
	}
	printf("%s", s);
	while (--i >= 0)
		if (nums[i] || i == 0 && nums[1] == 0 && nums[2] == 0)
			if (nums[i] == 1)
				printf(MSGSTR(NUMSING, "%d %s "), nums[i], sep[i]); /*MSG*/
			else
				printf(MSGSTR(NUMPLURL, "%d %ss "), nums[i], sep[i]); /*MSG*/
	printf("\r\n!\r\n");
}

variable()
{
	char	buf[256];

	if (prompt(MSGSTR(SET, "[set] "), buf)) /*MSG*/
		return;
	vlex(buf);
	if (vtable[BEAUTIFY].v_access&CHANGED) {
		vtable[BEAUTIFY].v_access &= ~CHANGED;
		kill(pid, SIGSYS);
	}
        if (vtable[BAUDRATE].v_access&CHANGED) {
                vtable[BAUDRATE].v_access &= ~CHANGED;
                setbaudrate();
        }
	if (vtable[SCRIPT].v_access&CHANGED) {
		vtable[SCRIPT].v_access &= ~CHANGED;
		setscript();
		/*
		 * So that "set record=blah script" doesn't
		 *  cause two transactions to occur.
		 */
		if (vtable[RECORD].v_access&CHANGED)
			vtable[RECORD].v_access &= ~CHANGED;
	}
	if (vtable[RECORD].v_access&CHANGED) {
		vtable[RECORD].v_access &= ~CHANGED;
		if (boolean(value(SCRIPT)))
			setscript();
	}
	if (vtable[TAND].v_access&CHANGED) {
		vtable[TAND].v_access &= ~CHANGED;
		if (boolean(value(TAND)))
			tandem("on");
		else
			tandem("off");
	}
 	if (vtable[LECHO].v_access&CHANGED) {
 		vtable[LECHO].v_access &= ~CHANGED;
 		HD = boolean(value(LECHO));
 	}
	if (vtable[PARITY].v_access&CHANGED) {
		vtable[PARITY].v_access &= ~CHANGED;
		setparity();
	}
}

/*
 * Checks the new baudrate to see if valid. If it is, then the line
 * is reset to that baudrate, else it is left to the existing baudrate
 * and the vtable[BAUDRATE] entry is set to the old baudrate.
 */
setbaudrate()
{
    int i;

    if ((i = speed(number(value(BAUDRATE)))) == NULL) {
        printf("bad baud rate, resetting to old value\r\n");
        resetbdrate();
    } else {
        ttysetup(i);
    }
}

/*
 * Resets the vtable value of the baudrate to that of the line
 */
resetbdrate()
{
    struct sgttyb tty;

    ioctl(FD, TIOCGETP, &tty);
    number(value(BAUDRATE)) = bauds[tty.sg_ispeed];
}                                                     

/*
 * Turn tandem mode on or off for remote tty.
 */
tandem(option)
	char *option;
{
	struct sgttyb rmtty;

	ioctl(FD, TIOCGETP, &rmtty);
	if (strcmp(option,"on") == 0) {
		rmtty.sg_flags |= TANDEM;
		arg.sg_flags |= TANDEM;
	} else {
		rmtty.sg_flags &= ~TANDEM;
		arg.sg_flags &= ~TANDEM;
	}
	ioctl(FD, TIOCSETP, &rmtty);
	ioctl(0,  TIOCSETP, &arg);
}

/*
 * Send a break.
 */
genbrk()
{

	ioctl(FD, TIOCSBRK, NULL);
	sleep(1);
	ioctl(FD, TIOCCBRK, NULL);
}

/*
 * Suspend tip
 */
suspend(c)
	char c;
{

	unraw();
	kill(c == CTRL('y') ? getpid() : 0, SIGTSTP);
	raw();
}

/*
 *	expand a file name if it includes shell meta characters
 */

char *
expand(name)
	char name[];
{
	static char xname[BUFSIZE];
	char cmdbuf[BUFSIZE];
	register int pid, l, rc;
	register char *cp, *Shell;
	int s, pivec[2], (*sigint)();

	if (!anyof(name, "~{[*?$`'\"\\"))
		return(name);
	/* sigint = signal(SIGINT, SIG_IGN); */
	if (pipe(pivec) < 0) {
		perror(MSGSTR(PIPE, "pipe")); /*MSG*/
		/* signal(SIGINT, sigint) */
		return(name);
	}
	sprintf(cmdbuf, "echo %s", name);
	if ((pid = vfork()) == 0) {
		Shell = value(SHELL);
		if (Shell == NOSTR)
			Shell = _PATH_BSHELL;
		close(pivec[0]);
		close(1);
		dup(pivec[1]);
		close(pivec[1]);
		close(2);
		shell_uid();
		execl(Shell, Shell, "-c", cmdbuf, 0);
		_exit(1);
	}
	if (pid == -1) {
		perror(MSGSTR(FORK, "fork")); /*MSG*/
		close(pivec[0]);
		close(pivec[1]);
		return(NOSTR);
	}
	close(pivec[1]);
	l = read(pivec[0], xname, BUFSIZE);
	close(pivec[0]);
	while (wait(&s) != pid)
		;
	s &= 0377;
	if (s != 0 && s != SIGPIPE) {
		fprintf(stderr, MSGSTR(ECHOFAIL, "\"Echo\" failed\n")); /*MSG*/
		return(NOSTR);
	}
	if (l < 0) {
		perror(MSGSTR(READIT, "read")); /*MSG*/
		return(NOSTR);
	}
	if (l == 0) {
		fprintf(stderr, MSGSTR(NOMATCH, "\"%s\": No match\n"), name); /*MSG*/
		return(NOSTR);
	}
	if (l == BUFSIZE) {
		fprintf(stderr, MSGSTR(BUFFOFLOW, "Buffer overflow expanding \"%s\"\n"), name); /*MSG*/
		return(NOSTR);
	}
	xname[l] = 0;
	for (cp = &xname[l-1]; *cp == '\n' && cp > xname; cp--)
		;
	*++cp = '\0';
	return(xname);
}

/*
 * Are any of the characters in the two strings the same?
 */

anyof(s1, s2)
	register char *s1, *s2;
{
	register int c;

	while (c = *s1++)
		if (any(c, s2))
			return(1);
	return(0);
}
