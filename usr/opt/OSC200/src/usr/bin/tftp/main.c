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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/11 19:20:30 $";
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
 * main.c
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/* static char sccsid[] = "main.c	1.6  com/sockcmd/tftp,3.1,8943 10/8/89 17:37:44"; */
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "main.c	5.9 (Berkeley) 6/1/90";
#endif
*/
/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

/*
 * TFTP User Program -- Command Interface.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <setjmp.h>
#include <ctype.h>
#include <netdb.h>

#include <nl_types.h>
#include <locale.h>
#include "tftp_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TFTP,n,s) 

#define	TIMEOUT		5		/* secs between rexmt's */

#define MAX_LINE_LENGTH	256
#define MAX_MARGV_SIZE	64

struct	sockaddr_in sin;
int	f;
short   port;
int	trace;
int	verbose;
int     hash;
int     rate;
int	connected;
char	mode[32];
char	line[MAX_LINE_LENGTH];
int	margc;
char	*margv[MAX_MARGV_SIZE];
char	*prompt = "tftp";
jmp_buf	toplevel;
void	intr();
struct	servent *sp;
int     UseCmdArgs = 0;
int     gargc = 0;
char    **gargv;

int	quit(), help(), setverbose(), settrace(), status();
int     get(), put(), setpeer(), modecmd(), setrexmt(), settimeout();
int     setbinary(), setascii();
int     setrate(), sethash();

#define HELPINDENT (sizeof("connect"))

struct cmd {
	char	*name;
	int	help_id;
	char	*help;
	int	(*handler)();
};

char	vhelp[] = "toggle verbose mode";
char	thelp[] = "toggle packet tracing";
char    hashhelp[] = "toggle hask marks";
char	chelp[] = "connect to remote tftp";
char	qhelp[] = "exit tftp";
char	hhelp[] = "print help information";
char	shelp[] = "send file";
char    writehelp[] = "write a file from a local file system to a remote file system";
char	rhelp[] = "receive file";
char    readhelp[] = "read a file from a remote file system to a local file system";
char	mhelp[] = "set file transfer mode";
char	sthelp[] = "show current status";
char	xhelp[] = "set per-packet retransmission timeout";
char	ihelp[] = "set total retransmission timeout";
char    ashelp[] = "set mode to netascii";
char    bnhelp[] = "set mode to octet";
char    ratehelp[] = "display transfer rate information";

struct cmd cmdtab[] = {
	{ "connect",
	HELP_CONN,
	chelp,		setpeer },
	{ "mode",
	HELP_FILE_TRANS,
	mhelp,          modecmd },
	{ "put",	
	HELP_SEND_FILE,
	shelp,		put },
	{ "get",	
	HELP_REC_FILE,
	rhelp,		get },
	{ "quit",	
	HELP_EXIT,
	qhelp,		quit },
	{ "verbose",	
	HELP_T_VERBOSE,
	vhelp,		setverbose },
	{ "hash",
	HELP_HASH,
	 hashhelp,      sethash },
	{ "trace",	
	HELP_TRACE,
	thelp,		settrace },
	{ "status",	
	HELP_SHOW_STAT,
	sthelp,		status },
	{ "binary", 
	HELP_SET_OCTECT,
	bnhelp,         setbinary },
	{ "octet",
	HELP_OCTECT,
	 bnhelp,      setbinary },
	{ "ascii", 
	HELP_SET_ASCII,
	ashelp,         setascii },
	{ "netascii",
	HELP_NETASCII,
	 ashelp,      setascii },
	{ "rexmt",	
	HELP_SET_TRANS,
	xhelp,		setrexmt },
	{ "timeout",	
	HELP_SET_TIME,
	ihelp,		settimeout, },
	{ "help",
	HELP_HELP,
	 hhelp,      help },
	{ "rate",
	HELP_RATE,
	 ratehelp,      setrate },
	{ "?",	
	HELP_PRINT,
	hhelp,		help },
	0
};

struct	cmd *getcmd();
char	*tail();
char	*index();
char	*rindex();

main(argc, argv)
	int  argc;
	char *argv[];
{
	struct sockaddr_in sin;
	int top;

	setlocale(LC_ALL, "");
	catd = NLcatopen(MF_TFTP,NL_CAT_LOCALE);

	err_load();

	sp = getservbyname("tftp", "udp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(UNKNOWN_SERVICE, "tftp: udp/tftp: unknown service\n")); /*MSG*/
		exit(1);
	}
	f = socket(AF_INET, SOCK_DGRAM, 0);
	if (f < 0) {
		perror("tftp: socket");
		exit(3);
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	if (bind(f, &sin, sizeof (sin)) < 0) {
		perror("tftp: bind");
		exit(1);
	}
	strcpy(mode, "netascii");
	signal(SIGINT, intr);
	argc--, argv++;

	if (argc > 0) {
                int cnt;
                if (setjmp(toplevel) != 0) {
                        exit(0);
                }
                 cnt = 1;
                if (argc > 1 && isdigit(argv[1][0]))
                        cnt++;
                setpeer(cnt + 1, argv - 1);
                argc -= cnt;
                argv += cnt;
        }
        if (argc > 0 && argv[0][0] != '-') {
            fprintf(stderr,"usage: %s host-name [port] [[-command [args] ...\
]\n",argv[0]);
                exit(1);
        }
	UseCmdArgs = argc > 0;
        gargc = argc;
        gargv = argv;
        top = setjmp(toplevel) == 0;
        for (;;)
                command(top);
}

char    hostname[MAXHOSTNAMELEN];

setpeer(argc, argv)
	int argc;
	char *argv[];
{
	struct hostent *host;

	if (argc < 2) {
		strcpy(line, "Connect ");
		printf(MSGSTR(CONN_TO, "(to) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc > 3) {
		printf(MSGSTR(USAGE, "usage: %s host-name [port]\n"), argv[0]); /*MSG*/
		return;
	}
	host = gethostbyname(argv[1]);
	if (host) {
		sin.sin_family = host->h_addrtype;
		bcopy(host->h_addr, &sin.sin_addr, host->h_length);
		strcpy(hostname, host->h_name);
	} else {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(argv[1]);
		if (sin.sin_addr.s_addr == -1) {
			connected = 0;
			printf(MSGSTR(UNKNOWN_HOST, "%s: unknown host\n"), argv[1]); /*MSG*/
			return;
		}
		strcpy(hostname, argv[1]);
	}
	port = sp->s_port;
	if (argc == 3) {
		port = atoi(argv[2]);
		if (port < 0) {
			printf(MSGSTR(BAD_PORT_NO, "%s: bad port number\n"), argv[2]); /*MSG*/
			connected = 0;
			return;
		}
		port = htons(port);
	}
	connected = 1;
}

struct	modes {
	char *m_name;
	char *m_mode;
} modes[] = {
	{ "ascii",	"netascii" },
	{ "netascii",   "netascii" },
	{ "binary",     "octet" },
	{ "image",      "octet" },
	{ "octet",     "octet" },
/*      { "mail",       "mail" },       */
	{ 0,		0 }
};

modecmd(argc, argv)
	int argc;
	char *argv[];
{
	register struct modes *p;
	char *sep;

	if (argc < 2) {
		printf(MSGSTR(USING_MODE, "Using %s mode to transfer files.\n"), mode); /*MSG*/
		return;
	}
	if (argc == 2) {
		for (p = modes; p->m_name; p++)
			if (strcmp(argv[1], p->m_name) == 0)
				break;
		if (p->m_name) {
			setmode(p->m_mode);
			return;
		}
		printf(MSGSTR(UNKNOWN_MODE, "%s: unknown mode\n"), argv[1]); /*MSG*/
		/* drop through and print usage message */
	}

	printf(MSGSTR(USAGE_2, "usage: %s ["), argv[0]); /*MSG*/
	sep = " ";
	for (p = modes; p->m_name; p++) {
		printf("%s%s", sep, p->m_name);
		if (*sep == ' ')
			sep = " | ";
	}
	printf(" ]\n");
	return;
}

setbinary(argc, argv)
int argc;
char *argv[];
{       setmode("octet");
}

setascii(argc, argv)
int argc;
char *argv[];
{       setmode("netascii");
}

setmode(newmode)
char *newmode;
{
	strcpy(mode, newmode);
	if (verbose)
		printf(MSGSTR(MODE_SET, "mode set to %s\n"), mode); /*MSG*/
}


/*
 * Send file(s).
 */
put(argc, argv)
	int argc;
	char *argv[];
{
	int fd;
	register int n;
	register char *cp, *targ;


	if (argc < 2) {
		strcpy(line, "send ");
		printf(MSGSTR(PUT_FILE, "(file) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		putusage(argv[0]);
		return;
	}
	targ = argv[argc - 1];
	if (index(argv[argc - 1], ':')) {
		char *cp;
		struct hostent *hp;

		for (n = 1; n < argc - 1; n++)
			if (index(argv[n], ':')) {
				putusage(argv[0]);
				return;
			}
		cp = argv[argc - 1];
		targ = index(cp, ':');
		*targ++ = 0;
		hp = gethostbyname(cp);
		if (hp == NULL) {
			fprintf(stderr, MSGSTR(UNKHOST, "tftp: %s "), cp);
			herror((char *)NULL);
			return;
		}
		bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
		sin.sin_family = hp->h_addrtype;
		connected = 1;
		strcpy(hostname, hp->h_name);
	}
	if (!connected) {
		printf(MSGSTR(NO_TARGET, "No target machine specified.\n")); /*MSG*/
		return;
	}
	if (argc < 4) {
		cp = argc == 2 ? tail(targ) : argv[1];
		fd = open(cp, O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, MSGSTR(TFTP, "tftp: ")); perror(cp); /*MSG*/
			return;
		}
		if (verbose)
			NLprintf(MSGSTR(PUTTING, "putting %s to %s:%s [%s]\n"), /*MSG*/
				cp, hostname, targ, mode);
		sin.sin_port = port;
		sendfile(fd, targ, mode);
		return;
	}
				/* this assumes the target is a directory */
				/* on a remote unix system.  hmmmm.  */
	cp = index(targ, '\0'); 
	*cp++ = '/';
	for (n = 1; n < argc - 1; n++) {
		strcpy(cp, tail(argv[n]));
		fd = open(argv[n], O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, MSGSTR(TFTP, "tftp: ")); perror(argv[n]); /*MSG*/
			continue;
		}
		if (verbose)
			NLprintf(MSGSTR(PUTTING, "putting %s to %s:%s [%s]\n"), /*MSG*/
				argv[n], hostname, targ, mode);
		sin.sin_port = port;
		sendfile(fd, targ, mode);
	}
}

putusage(s)
	char *s;
{
	printf(MSGSTR(USAGE3, "usage: %s file ... host:target, or\n"), s); /*MSG*/
	printf(MSGSTR(USAGE_4, "       %s file ... target (when already connected)\n"), s); /*MSG*/
}

/*
 * Receive file(s).
 */
get(argc, argv)
	int argc;
	char *argv[];
{
	int fd;
	register int n;
	register char *cp;
	char *src;


	if (argc < 2) {
		strcpy(line, "get ");
		printf(MSGSTR(GET_FILES, "(files) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		getusage(argv[0]);
		return;
	}
	if (!connected) {
		for (n = 1; n < argc ; n++)
			if (index(argv[n], ':') == 0) {
				getusage(argv[0]);
				return;
			}
	}
	for (n = 1; n < argc ; n++) {
		src = index(argv[n], ':');
		if (src == NULL)
			src = argv[n];
		else if (*(src-1) == '\\') {
                        /* clobber the back slash */
                        strcpy(src-1,src);
                }
		else {
			struct hostent *hp;

			*src++ = 0;
			hp = gethostbyname(argv[n]);
			if (hp == NULL) {
				fprintf(stderr, MSGSTR(UNKHOST, "tftp: %s "),argv[n]); /*MSG*/
				herror((char *)NULL);
				continue;
			}
			bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
			sin.sin_family = hp->h_addrtype;
			connected = 1;
			strcpy(hostname, hp->h_name);
		}
		if (argc < 4) {
			cp = argc == 3 ? argv[2] : tail(src);
			fd = creat(cp, 0644);
			if (fd < 0) {
				fprintf(stderr, MSGSTR(TFTP, "tftp: ")); perror(cp); /*MSG*/
				return;
			}
			if (verbose)
				NLprintf(MSGSTR(GET_GETTING, "getting from %s:%s to %s [%s]\n"), /*MSG*/
					hostname, src, cp, mode);
			sin.sin_port = port;
			recvfile(fd, src, mode);
			break;
		}
		cp = tail(src);
		fd = creat(cp, 0644);
		if (fd < 0) {
			fprintf(stderr, MSGSTR(TFTP, "tftp: ")); perror(cp); /*MSG*/
			continue;
		}
		if (verbose)
			NLprintf(MSGSTR(GET_GETTING, "getting from %s:%s to %s [%s]\n"), /*MSG*/
				hostname, src, cp, mode);
		sin.sin_port = port;
		recvfile(fd, src, mode);
	}
}

getusage(s)
char * s;
{
	printf(MSGSTR(USAGE_5, "usage: %s host:file host:file ... file, or\n"), s); /*MSG*/
	NLprintf(MSGSTR(GET_USAGE, "       %s file file ... file if connected\n"), s); /*MSG*/
}

int	rexmtval = TIMEOUT;

setrexmt(argc, argv)
	int argc;
	char *argv[];
{
	int t;

	if (argc < 2) {
		strcpy(line, "Rexmt-timeout ");
		printf(MSGSTR(SETREXMT_VALUE, "(value) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc != 2) {
		printf(MSGSTR(USAGE_6, "usage: %s value\n"), argv[0]); /*MSG*/
		return;
	}
	t = atoi(argv[1]);
	if (t < 0)
		printf(MSGSTR(BAD_VAL, "%s: bad value\n"), argv[1]); /*MSG*/
	else
		rexmtval = t;
}

int	maxtimeout = 5 * TIMEOUT;

settimeout(argc, argv)
	int argc;
	char *argv[];
{
	int t;

	if (argc < 2) {
		strcpy(line, "Maximum-timeout ");
		printf(MSGSTR(SETREXMT_VALUE, "(value) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc != 2) {
		printf(MSGSTR(USAGE_6, "usage: %s value\n"), argv[0]); /*MSG*/
		return;
	}
	t = atoi(argv[1]);
	if (t < 0)
		printf(MSGSTR(BAD_VAL, "%s: bad value\n"), argv[1]); /*MSG*/
	else
		maxtimeout = t;
}

status(argc, argv)
	int argc;
	char *argv[];
{
	if (connected)
		printf(MSGSTR(CONNECTED_TO, "Connected to %s.\n"), hostname); /*MSG*/
	else
		printf(MSGSTR(NOT_CONNECTED, "Not connected.\n")); /*MSG*/
	printf(MSGSTR(MODE_V_T, "Mode: %s Verbose: %s Tracing: %s\n"), mode, /*MSG*/
		verbose ? "on" : "off", trace ? "on" : "off");
	printf(MSGSTR(REXMT_INT, "Rexmt-interval: %d seconds, Max-timeout: %d seconds\n"), /*MSG*/
		rexmtval, maxtimeout);
}

void
intr()
{
	signal(SIGALRM, SIG_IGN);
	alarm(0);
	longjmp(toplevel, -1);
}

char *
tail(filename)
	char *filename;
{
	register char *s;
	
	while (*filename) {
		s = rindex(filename, '/');
		if (s == NULL)
			break;
		if (s[1])
			return (s + 1);
		*s = '\0';
	}
	return (filename);
}

/*
 * Command parser.
 */
command(top)
	int top;
{
	register struct cmd *c;

	if (!top)
		putchar('\n');
	for (;;) {
		if (!UseCmdArgs)
			printf("%s> ", prompt);

		if (UseCmdArgs) {
                    /* no more args? then quit */
                    if (gargc <= 0) {
                        quit();
                        break;
                    }
                    /* copy switch in as command */

                    strcpy(line,&gargv[0][1]);
                    gargv++; gargc--;
                    while (gargc > 0 && (gargv[0][0] != '-' || gargv[0][1] == '-')) {
                        strcat(line," ");
                        if (gargv[0][0] == '-')
                            strcat(line,&gargv[0][1]);
                        else
                            strcat(line,gargv[0]);
                        gargv++; gargc--;
                    }
                }
		else {
                	if (gets(line) == 0) {
                        	if (feof(stdin)) {
                                	quit();
                        	} else {
                                	continue;
                        	}
			}
                }
		if (line[0] == 0)
			continue;
		makeargv();
		if (margc == 0)
			continue;
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf(MSGSTR(AMBIGUOUS, "?Ambiguous command\n")); /*MSG*/
			continue;
		}
		if (c == 0) {
			printf(MSGSTR(INVALID_CMD, "?Invalid command\n")); /*MSG*/
			continue;
		}
		(*c->handler)(margc, margv);
	}
}

struct cmd *
getcmd(name)
	register char *name;
{
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; p = c->name; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

/*
 * Slice a string up into argc/argv.
 */
makeargv()
{
	register char *cp;
	register char **argp = margv;


	margc = 0;
	for (cp = line; *cp;) {
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
		if (margc >= (MAX_MARGV_SIZE - 1))
			break;
	}
	*argp++ = 0;
}

/*VARARGS*/
quit()
{
	exit(0);
}

/*
 * Help command.
 */
help(argc, argv)
	int argc;
	char *argv[];
{
	register struct cmd *c;

	if (argc == 1) {
		printf(MSGSTR(HELP_TABLE, "Commands may be abbreviated.  Commands are:\n\n")); /*MSG*/
		for (c = cmdtab; c->name; c++)
			printf(MSGSTR(c->help_id, "%-*s\t%s\n"), HELPINDENT,
				c->name, c->help); /*MSG*/
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf(MSGSTR(AMB_HELP, "?Ambiguous help command %s\n"), arg); /*MSG*/
		else if (c == (struct cmd *)0)
			printf(MSGSTR(INVALID_HELP, "?Invalid help command %s\n"), arg); /*MSG*/
		else
			printf("%s\n", c->help);
	}
}

/*VARARGS*/
settrace()
{
	trace = !trace;
	printf(MSGSTR(PACKET_TRACE, "Packet tracing %s.\n"), trace ? "on" : "off"); /*MSG*/
}

/*VARARGS*/
setverbose()
{
	verbose = !verbose;
	printf(MSGSTR(VERBOSE_MODE, "Verbose mode %s.\n"), verbose ? "on" : "off"); /*MSG*/
}
/*VARARGS*/
sethash()
{
        hash = !hash;
        printf("Hash mode %s.\n", hash ? "on" : "off");
}

/*VARARGS*/
setrate()
{
        rate = !rate;
        printf("Rate display %s.\n", rate ? "on" : "off");
}
