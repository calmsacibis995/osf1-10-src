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
static char	*sccsid = "@(#)$RCSfile: telnetd.c,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/10/08 16:12:59 $";
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
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#define TTYDEFCHARS
#include <unistd.h>
#include "telnetd.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>


/*
 * I/O data buffers,
 * pointers, and counters.
 */
char	ptyibuf[BUFSIZ], *ptyip = ptyibuf;
char	ptyibuf2[BUFSIZ];

int debug = 0;
char *progname;

#ifdef IP_TOS
struct tosent {
	char	*t_name;	/* name */
	char	**t_aliases;	/* alias list */
	char	*t_proto;	/* protocol */
	int	t_tos;		/* Type Of Service bits */
};

struct tosent *
gettosbyname_d(name, proto)
char *name, *proto;
{
	static struct tosent te;
	static char *aliasp = 0;

	te.t_name = name;
	te.t_aliases = &aliasp;
	te.t_proto = proto;
	te.t_tos = IPTOS_LOWDELAY;	/* Low Delay bit */
	return(&te);
}

#define   MAXALIASES    35
#define   BSIZ          512
#define   TABBUFSIZ	79
static char TOSDB[] = "/etc/iptos" ;
static FILE *tosf = NULL ;
static int stayopen ;
static char *getcommon_any() ;
static struct tosent *gettosbyname() ;
static struct tosent *gettosent_local() ;
static struct tosent *interpret_local() ;

struct tosent *tp;
#endif  /* IP_TOS */

main(argc, argv)
	char *argv[];
{
	struct sockaddr_in from;
	int on = 1, fromlen;

	setlocale(LC_ALL, "");
        catd = NLcatopen(MF_TELNETD,NL_CAT_LOCALE);

	pfrontp = pbackp = ptyobuf;
	netip = netibuf;
	nfrontp = nbackp = netobuf;

	progname = *argv;


top:
	argc--, argv++;

	if (argc > 0 && strcmp(*argv, "-debug") == 0) {
		debug++;
		goto top;
	}

#ifdef	LINEMODE
	if (argc > 0 && !strcmp(*argv, "-l")) {
		alwayslinemode = 1;
		goto top;
	}
#endif	/* LINEMODE */

#ifdef DIAGNOSTICS
	/*
	 * Check for desired diagnostics capabilities.
	 */
	if (argc > 0 && !strncmp(*argv, "-D", 2)) {
		*argv += 2;
		if (**argv == '\0') {
			if (argc < 2) {
				usage();
				/* NOT REACHED */
			}
			argv++, argc--;
			if (**argv == '\0') {
				usage();
				/* NOT REACHED */
			}
		}
		if (!strcmp(*argv, "report")) {
			diagnostic |= TD_REPORT|TD_OPTIONS;
		} else if (!strcmp(*argv, "exercise")) {
			diagnostic |= TD_EXERCISE;
		} else if (!strcmp(*argv, "netdata")) {
			diagnostic |= TD_NETDATA;
		} else if (!strcmp(*argv, "ptydata")) {
			diagnostic |= TD_PTYDATA;
		} else if (!strcmp(*argv, "options")) {
			diagnostic |= TD_OPTIONS;
		} else {
			usage();
			/* NOT REACHED */
		}
		goto top;
	}
#endif /* DIAGNOSTICS */

	if (argc > 0 && **argv == '-') {
		fprintf(stderr, MSGSTR( TELNETD_UNKN_OPT, 
			"telnetd: %s: unknown option\n"), *argv+1);
		usage();
		/* NOT REACHED */
	}

	if (debug) {
	    int s, ns, foo;
	    struct servent *sp;
	    static struct sockaddr_in sin = { AF_INET };

	    if (argc > 1) {
		usage();
		/* NOT REACHED */
	    } else if (argc == 1) {
		    if (sp = getservbyname(*argv, "tcp")) {
			sin.sin_port = sp->s_port;
		    } else {
			sin.sin_port = atoi(*argv);
			if ((int)sin.sin_port <= 0) {
			    fprintf(stderr, MSGSTR( TELNETD_BAD_PT, 
				"telnetd: %s: bad port #\n"), *argv);
			    usage();
			    /* NOT REACHED */
			}
			sin.sin_port = htons((u_short)sin.sin_port);
		   }
	    } else {
		sp = getservbyname("telnet", "tcp");
		if (sp == 0) {
		    fprintf(stderr, MSGSTR( TELNETD_UNKN_SERV, 
			 "telnetd: tcp/telnet: unknown service\n"));
		    exit(1);
		}
		sin.sin_port = sp->s_port;
	    }


	    s = socket(AF_INET, SOCK_STREAM, 0);
	    if (s < 0) {
		    perror("telnetd: socket");;
		    exit(1);
	    }
	    (void) setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	    if (bind(s, (struct sockaddr *)&sin, sizeof sin) < 0) {
		perror("bind");
		exit(1);
	    }
	    if (listen(s, 1) < 0) {
		perror("listen");
		exit(1);
	    }
	    foo = sizeof sin;
	    ns = accept(s, (struct sockaddr *)&sin, &foo);
	    if (ns < 0) {
		perror("accept");
		exit(1);
	    }
	    (void) dup2(ns, 0);
	    (void) close(ns);
	    (void) close(s);
	} else if (argc > 0) {
		usage();
		/* NOT REACHED */
	}

	openlog("telnetd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
	fromlen = sizeof (from);
	if (getpeername(0, (struct sockaddr *)&from, &fromlen) < 0) {
		fprintf(stderr, MSGSTR( TELNETD_PROGNAME, "%s: "), progname);
		perror("getpeername");
		_exit(1);
	}
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0) {
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	}

#ifdef  IP_TOS
        tp = gettosbyname("telnet","tcp");
	if (tp == NULL)
	  tp = gettosbyname_d("telnet", "tcp");
	  if(setsockopt(0,IPPROTO_IP,IP_TOS,(char *)&tp->t_tos, sizeof(int)) < 0)
		syslog(LOG_WARNING, "setsockopt (IP_TOS): %m");
#endif	/* IP_TOS */
	net = 0;
	doit(&from);
	/* NOTREACHED */
}  /* end of main */

usage()
{
	fprintf(stderr, MSGSTR( TELNETD_USAGE1, 
		"Usage: telnetd [-debug] [-h]"));
#ifdef DIAGNOSTICS
	fprintf(stderr, MSGSTR( TELNETD_USAGE3, 
		" [-D (options|report|exercise|netdata|ptydata)]"));
#endif /* DIAGNOSTICS */
#ifdef LINEMODE
	fprintf(stderr, MSGSTR( TELNETD_USAGE4, " [-l]"));
#endif
	fprintf(stderr, MSGSTR( TELNETD_USAGE7, " [port]\n"));
	exit(1);
}

void	cleanup();

/*
 * getterminaltype
 *
 *	Ask the other end to send along its terminal type and speed.
 * Output is the variable terminaltype filled in.
 */
static char ttytype_sbbuf[] = { IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE };
void
getterminaltype()
{
    void ttloop();

    settimer(baseline);

    send_do(TELOPT_TTYPE, 1);
    send_do(TELOPT_TSPEED, 1);
    send_do(TELOPT_XDISPLOC, 1);
    send_do(TELOPT_ENVIRON, 1);
    while (his_will_wont_is_changing(TELOPT_TTYPE) ||
	   his_will_wont_is_changing(TELOPT_TSPEED) ||
	   his_will_wont_is_changing(TELOPT_XDISPLOC) ||
	   his_will_wont_is_changing(TELOPT_ENVIRON)) {
	ttloop();
    }
    if (his_state_is_will(TELOPT_TSPEED)) {
	static char sbbuf[] = { IAC, SB, TELOPT_TSPEED, TELQUAL_SEND, IAC, SE };

	bcopy(sbbuf, nfrontp, sizeof sbbuf);
	nfrontp += sizeof sbbuf;
    }
    if (his_state_is_will(TELOPT_XDISPLOC)) {
	static char sbbuf[] = { IAC, SB, TELOPT_XDISPLOC, TELQUAL_SEND, IAC, SE };

	bcopy(sbbuf, nfrontp, sizeof sbbuf);
	nfrontp += sizeof sbbuf;
    }
    if (his_state_is_will(TELOPT_ENVIRON)) {
	static char sbbuf[] = { IAC, SB, TELOPT_ENVIRON, TELQUAL_SEND, IAC, SE };

	bcopy(sbbuf, nfrontp, sizeof sbbuf);
	nfrontp += sizeof sbbuf;
    }
    if (his_state_is_will(TELOPT_TTYPE)) {

	bcopy(ttytype_sbbuf, nfrontp, sizeof ttytype_sbbuf);
	nfrontp += sizeof ttytype_sbbuf;
    }
    if (his_state_is_will(TELOPT_TSPEED)) {
	while (sequenceIs(tspeedsubopt, baseline))
	    ttloop();
    }
    if (his_state_is_will(TELOPT_XDISPLOC)) {
	while (sequenceIs(xdisplocsubopt, baseline))
	    ttloop();
    }
    if (his_state_is_will(TELOPT_ENVIRON)) {
	while (sequenceIs(environsubopt, baseline))
	    ttloop();
    }
    if (his_state_is_will(TELOPT_TTYPE)) {
	char first[256], last[256];

	while (sequenceIs(ttypesubopt, baseline))
	    ttloop();

	/*
	 * If the other side has already disabled the option, then
	 * we have to just go with what we (might) have already gotten.
	 */
	if (his_state_is_will(TELOPT_TTYPE) && !terminaltypeok(terminaltype)) {
	    (void) strncpy(first, terminaltype, sizeof(first));
	    for(;;) {
		/*
		 * Save the unknown name, and request the next name.
		 */
		(void) strncpy(last, terminaltype, sizeof(last));
		_gettermname();
		if (terminaltypeok(terminaltype))
		    break;
		if ((strncmp(last, terminaltype, sizeof(last)) == 0) ||
		    his_state_is_wont(TELOPT_TTYPE)) {
		    /*
		     * We've hit the end.  If this is the same as
		     * the first name, just go with it.
		     */
		    if (strncmp(first, terminaltype, sizeof(first) == 0))
			break;
		    /*
		     * Get the terminal name one more time, so that
		     * RFC1091 compliant telnets will cycle back to
		     * the start of the list.
		     */
		     _gettermname();
		    if (strncmp(first, terminaltype, sizeof(first) != 0))
			(void) strncpy(terminaltype, first, sizeof(first));
		    break;
		}
	    }
	}
    }
}  /* end of getterminaltype */

_gettermname()
{
    /*
     * If the client turned off the option,
     * we can't send another request, so we
     * just return.
     */
    if (his_state_is_wont(TELOPT_TTYPE))
	return;
    settimer(baseline);
    bcopy(ttytype_sbbuf, nfrontp, sizeof ttytype_sbbuf);
    nfrontp += sizeof ttytype_sbbuf;
    while (sequenceIs(ttypesubopt, baseline))
	ttloop();
}

terminaltypeok(s)
char *s;
{
    char buf[1024];

    if (terminaltype == NULL)
	return(1);

    /*
     * tgetent() will return 1 if the type is known, and
     * 0 if it is not known.  If it returns -1, it couldn't
     * open the database.  But if we can't open the database,
     * it won't help to say we failed, because we won't be
     * able to verify anything else.  So, we treat -1 like 1.
     */
    if (tgetent(buf, s) == 0)
	return(0);
    return(1);
}

/*
 * Get a pty, scan input lines.
 */
doit(who)
	struct sockaddr_in *who;
{
	char *host, *inet_ntoa();
	int t;
	struct hostent *hp;
	struct termios termios;
	extern char *line;

	termios.c_iflag = TTYDEF_IFLAG;
	termios.c_oflag = TTYDEF_OFLAG;
	termios.c_lflag = TTYDEF_LFLAG;
        termios.c_cflag = TTYDEF_CFLAG;
	termios.c_ispeed = TTYDEF_SPEED ;	/* Default speed is 9600 */
	termios.c_ospeed = TTYDEF_SPEED ;       /* Default speed is 9600 */

	bcopy(ttydefchars, termios.c_cc, sizeof(termios.c_cc));
	if (openpty(&pty, &t, line, &termios, (long)NULL) == -1)
		fatal(net, "All network ports in use");
	init_termbuf();
	
	/* get name of connected client */
	hp = gethostbyaddr((char *)&who->sin_addr, sizeof (struct in_addr),
		who->sin_family);
	if (hp)
		host = hp->h_name;
	else
		host = inet_ntoa(who->sin_addr);

	init_env();
	/*
	 * get terminal type.
	 */
	getterminaltype();
	setenv("TERM", terminaltype ? terminaltype : "network", 1);

	/*
	 * Start up the login process on the slave side of the terminal
	 */
	startslave(t, host);

	telnet(net, pty);  /* begin server processing */
	/*NOTREACHED*/
}  /* end of doit */

#ifndef	MAXHOSTNAMELEN
#define	MAXHOSTNAMELEN 64
#endif	MAXHOSTNAMELEN
/*
 * Main loop.  Select from pty and network, and
 * hand data to telnet receiver finite state machine.
 */
telnet(f, p)
int f, p;
{
	int on = 1;
	char hostname[MAXHOSTNAMELEN];
	char lmbuf[TABBUFSIZ+1];
	char *HE;
	char *HN;
	char *IM;
	void netflush();
	
	/*
	 * Initialize the slc mapping table.
	 */
	get_slc_defaults();

	/*
	 * Do some tests where it is desireable to wait for a response.
	 * Rather than doing them slowly, one at a time, do them all
	 * at once.
	 */
	if (my_state_is_wont(TELOPT_SGA))
		send_will(TELOPT_SGA, 1);
	/*
	 * Is the client side a 4.2 (NOT 4.3) system?  We need to know this
	 * because 4.2 clients are unable to deal with TCP urgent data.
	 *
	 * To find out, we send out a "DO ECHO".  If the remote system
	 * answers "WILL ECHO" it is probably a 4.2 client, and we note
	 * that fact ("WILL ECHO" ==> that the client will echo what
	 * WE, the server, sends it; it does NOT mean that the client will
	 * echo the terminal input).
	 */
	send_do(TELOPT_ECHO, 1);

#ifdef	LINEMODE
	if (his_state_is_wont(TELOPT_LINEMODE)) {
		/* Query the peer for linemode support by trying to negotiate
		 * the linemode option.
		 */
		linemode = 0;
		editmode = 0;
		send_do(TELOPT_LINEMODE, 1);  /* send do linemode */
	}
#endif	/* LINEMODE */

	/*
	 * Send along a couple of other options that we wish to negotiate.
	 */
	send_do(TELOPT_NAWS, 1);
	send_will(TELOPT_STATUS, 1);
	flowmode = 1;  /* default flow control state */
	send_do(TELOPT_LFLOW, 1);

	/*
	 * Spin, waiting for a response from the DO ECHO.  However,
	 * some REALLY DUMB telnets out there might not respond
	 * to the DO ECHO.  So, we spin looking for NAWS, (most dumb
	 * telnets so far seem to respond with WONT for a DO that
	 * they don't understand...) because by the time we get the
	 * response, it will already have processed the DO ECHO.
	 * Kludge upon kludge.
	 */
	while (his_will_wont_is_changing(TELOPT_NAWS))
		ttloop();

	/*
	 * But...
	 * The client might have sent a WILL NAWS as part of its
	 * startup code; if so, we'll be here before we get the
	 * response to the DO ECHO.  We'll make the assumption
	 * that any implementation that understands about NAWS
	 * is a modern enough implementation that it will respond
	 * to our DO ECHO request; hence we'll do another spin
	 * waiting for the ECHO option to settle down, which is
	 * what we wanted to do in the first place...
	 */
	if (his_want_state_is_will(TELOPT_ECHO) &&
	    his_state_is_will(TELOPT_NAWS)) {
		while (his_will_wont_is_changing(TELOPT_ECHO))
			ttloop();
	}
	/*
	 * On the off chance that the telnet client is broken and does not
	 * respond to the DO ECHO we sent, (after all, we did send the
	 * DO NAWS negotiation after the DO ECHO, and we won't get here
	 * until a response to the DO NAWS comes back) simulate the
	 * receipt of a will echo.  This will also send a WONT ECHO
	 * to the client, since we assume that the client failed to
	 * respond because it believes that it is already in DO ECHO
	 * mode, which we do not want.
	 */
	if (his_want_state_is_will(TELOPT_ECHO)) {
#ifdef DIAGNOSTICS
		if (diagnostic & TD_OPTIONS) {
			sprintf(nfrontp, "td: simulating recv\r\n");
			nfrontp += strlen(nfrontp);
		}
#endif /* DIAGNOSTICS */
		willoption(TELOPT_ECHO);
	}

	/*
	 * Finally, to clean things up, we turn on our echo.  This
	 * will break stupid 4.2 telnets out of local terminal echo.
	 */

	if (my_state_is_wont(TELOPT_ECHO))
		send_will(TELOPT_ECHO, 1);

	/*
	 * Turn on packet mode, and default to line at at time mode.
	 */
	(void) ioctl(p, TIOCPKT, (char *)&on);
#ifdef	LINEMODE
	tty_setlinemode(1);

# ifdef	KLUDGELINEMODE
	/*
	 * Continuing line mode support.  If client does not support
	 * real linemode, attempt to negotiate kludge linemode by sending
	 * the do timing mark sequence.
	 */
	if (lmodetype < REAL_LINEMODE)
		send_do(TELOPT_TM, 1);
# endif	/* KLUDGELINEMODE */
#endif	/* LINEMODE */

	/*
	 * Call telrcv() once to pick up anything received during
	 * terminal type negotiation, 4.2/4.3 determination, and
	 * linemode negotiation.
	 */
	telrcv();

	(void) ioctl(f, FIONBIO, (char *)&on);
	(void) ioctl(p, FIONBIO, (char *)&on);

#if	defined(SO_OOBINLINE)
	(void) setsockopt(net, SOL_SOCKET, SO_OOBINLINE, &on, sizeof on);
#endif	/* defined(SO_OOBINLINE) */

#ifdef	SIGTSTP
	(void) signal(SIGTSTP, SIG_IGN);
#endif
#ifdef	SIGTTOU
	/*
	 * Ignoring SIGTTOU keeps the kernel from blocking us
	 * in ttioct() in /sys/tty.c.
	 */
	(void) signal(SIGTTOU, SIG_IGN);
#endif

	(void) signal(SIGCHLD, cleanup);

	(void) setsid();

	/*
	 * Show banner that getty never gave.
	 *
	 * We put the banner in the pty input buffer.  This way, it
	 * gets carriage return null processing, etc., just like all
	 * other pty --> client data.
	 */

	(void) gethostname(hostname, sizeof (hostname));
	IM = lmbuf;

	if (getent(IM, "default") == 1) {
		
		if (IM == 0)
			IM = "";
		HE = 0;
	} else {
		IM = DEFAULT_IM;
		HE = 0;
	}
	edithost(HE,hostname);
	if (IM && *IM)
		putf(IM, ptyibuf2);

	if (pcc)
		(void) strncat(ptyibuf2, ptyip, pcc+1);
	ptyip = ptyibuf2;
	pcc = strlen(ptyip);
#ifdef	LINEMODE
	/*
	 * Last check to make sure all our states are correct.
	 */
	init_termbuf();
	localstat();
#endif	/* LINEMODE */

#ifdef DIAGNOSTICS
	if (diagnostic & TD_REPORT) {
		sprintf(nfrontp, "td: Entering processing loop\r\n");
		nfrontp += strlen(nfrontp);
	}
#endif /* DIAGNOSTICS */

	for (;;) {
		fd_set ibits, obits, xbits;
		register int c;

		if (ncc < 0 && pcc < 0)
			break;

		FD_ZERO(&ibits);
		FD_ZERO(&obits);
		FD_ZERO(&xbits);
		/*
		 * Never look for input if there's still
		 * stuff in the corresponding output buffer
		 */
		if (nfrontp - nbackp || pcc > 0) {
			FD_SET(f, &obits);
		} else {
			FD_SET(p, &ibits);
		}
		if (pfrontp - pbackp || ncc > 0) {
			FD_SET(p, &obits);
		} else {
			FD_SET(f, &ibits);
		}
		if (!SYNCHing) {
			FD_SET(f, &xbits);
		}
		if ((c = select(16, &ibits, &obits, &xbits,
						(struct timeval *)0)) < 1) {
			if (c == -1) {
				if (errno == EINTR) {
					continue;
				}
			}
			sleep(5);
			continue;
		}

		/*
		 * Any urgent data?
		 */
		if (FD_ISSET(net, &xbits)) {
		    SYNCHing = 1;
		}

		/*
		 * Something to read from the network...
		 */
		if (FD_ISSET(net, &ibits)) {
#if	!defined(SO_OOBINLINE)
			/*
			 * In 4.2 (and 4.3 beta) systems, the
			 * OOB indication and data handling in the kernel
			 * is such that if two separate TCP Urgent requests
			 * come in, one byte of TCP data will be overlaid.
			 * This is fatal for Telnet, but we try to live
			 * with it.
			 *
			 * In addition, in 4.2 (and...), a special protocol
			 * is needed to pick up the TCP Urgent data in
			 * the correct sequence.
			 *
			 * What we do is:  if we think we are in urgent
			 * mode, we look to see if we are "at the mark".
			 * If we are, we do an OOB receive.  If we run
			 * this twice, we will do the OOB receive twice,
			 * but the second will fail, since the second
			 * time we were "at the mark", but there wasn't
			 * any data there (the kernel doesn't reset
			 * "at the mark" until we do a normal read).
			 * Once we've read the OOB data, we go ahead
			 * and do normal reads.
			 *
			 * There is also another problem, which is that
			 * since the OOB byte we read doesn't put us
			 * out of OOB state, and since that byte is most
			 * likely the TELNET DM (data mark), we would
			 * stay in the TELNET SYNCH (SYNCHing) state.
			 * So, clocks to the rescue.  If we've "just"
			 * received a DM, then we test for the
			 * presence of OOB data when the receive OOB
			 * fails (and AFTER we did the normal mode read
			 * to clear "at the mark").
			 */
		    if (SYNCHing) {
			int atmark;

			(void) ioctl(net, SIOCATMARK, (char *)&atmark);
			if (atmark) {
			    ncc = recv(net, netibuf, sizeof (netibuf), MSG_OOB);
			    if ((ncc == -1) && (errno == EINVAL)) {
				ncc = read(net, netibuf, sizeof (netibuf));
				if (sequenceIs(didnetreceive, gotDM)) {
				    SYNCHing = stilloob(net);
				}
			    }
			} else {
			    ncc = read(net, netibuf, sizeof (netibuf));
			}
		    } else {
			ncc = read(net, netibuf, sizeof (netibuf));
		    }
		    settimer(didnetreceive);
#else	/* !defined(SO_OOBINLINE)) */
		    ncc = read(net, netibuf, sizeof (netibuf));
#endif	/* !defined(SO_OOBINLINE)) */
		    if (ncc < 0 && errno == EWOULDBLOCK)
			ncc = 0;
		    else {
			if (ncc <= 0) {
			    break;
			}
			netip = netibuf;
		    }
#ifdef DIAGNOSTICS
		    if (diagnostic & (TD_REPORT | TD_NETDATA)) {
			    sprintf(nfrontp, "td: netread %d chars\r\n", ncc);
			    nfrontp += strlen(nfrontp);
		    }
		    if (diagnostic & TD_NETDATA) {
			    printdata("nd", netip, ncc);
		    }
#endif /* DIAGNOSTICS */
		}

		/*
		 * Something to read from the pty...
		 */
		if (FD_ISSET(p, &ibits)) {
			pcc = read(p, ptyibuf, BUFSIZ);
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else {
				if (pcc <= 0)
					break;
#ifdef	LINEMODE
				/*
				 * If ioctl from pty, pass it through net
				 */
				if (ptyibuf[0] & TIOCPKT_IOCTL) {
					copy_termbuf(ptyibuf+1, pcc-1);
					localstat();
					pcc = 1;
				}
#endif	LINEMODE
				if (ptyibuf[0] & TIOCPKT_FLUSHWRITE) {
					netclear();	/* clear buffer back */
#ifdef	notdef
					/*
					 * We really should have this in, but
					 * there are client telnets on some
					 * operating systems get screwed up
					 * royally if we send them urgent
					 * mode data.  So, for now, we'll not
					 * do this...
					 */
					*nfrontp++ = IAC;
					*nfrontp++ = DM;
					neturg = nfrontp-1; /* off by one XXX */
#endif
				}
				if (his_state_is_will(TELOPT_LFLOW) &&
				    (ptyibuf[0] &
				     (TIOCPKT_NOSTOP|TIOCPKT_DOSTOP))) {
					(void) sprintf(nfrontp, "%c%c%c%c%c%c",
					    IAC, SB, TELOPT_LFLOW,
					    ptyibuf[0] & TIOCPKT_DOSTOP ? 1 : 0,
					    IAC, SE);
					nfrontp += 6;
				}
				pcc--;
				ptyip = ptyibuf+1;
			}
		}

		while (pcc > 0) {
			if ((&netobuf[BUFSIZ] - nfrontp) < 2)
				break;
			c = *ptyip++ & 0377, pcc--;
			if (c == IAC)
				*nfrontp++ = c;
			*nfrontp++ = c;
			if ((c == '\r') && (my_state_is_wont(TELOPT_BINARY))) {
				if (pcc > 0 && ((*ptyip & 0377) == '\n')) {
					*nfrontp++ = *ptyip++ & 0377;
					pcc--;
				} else
					*nfrontp++ = '\0';
			}
		}

		if (FD_ISSET(f, &obits) && (nfrontp - nbackp) > 0)
			netflush();
		if (ncc > 0)
			telrcv();
		if (FD_ISSET(p, &obits) && (pfrontp - pbackp) > 0)
			ptyflush();
	}
	cleanup();
}  /* end of telnet */
	
#ifndef	TCSIG
# ifdef	TIOCSIG
#  define TCSIG TIOCSIG
# endif
#endif

/*
 * Send interrupt to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write intr char.
 */
interrupt()
{
	ptyflush();	/* half-hearted */

#ifdef	TCSIG
	(void) ioctl(pty, TCSIG, (char *)SIGINT);
#else	/* TCSIG */
	init_termbuf();
	*pfrontp++ = slctab[SLC_IP].sptr ?
			(unsigned char)*slctab[SLC_IP].sptr : '\177';
#endif	/* TCSIG */
}

/*
 * Send quit to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write quit char.
 */
sendbrk()
{
	ptyflush();	/* half-hearted */
#ifdef	TCSIG
	(void) ioctl(pty, TCSIG, (char *)SIGQUIT);
#else	/* TCSIG */
	init_termbuf();
	*pfrontp++ = slctab[SLC_ABORT].sptr ?
			(unsigned char)*slctab[SLC_ABORT].sptr : '\034';
#endif	/* TCSIG */
}

sendsusp()
{
#ifdef	SIGTSTP
	ptyflush();	/* half-hearted */
# ifdef	TCSIG
	(void) ioctl(pty, TCSIG, (char *)SIGTSTP);
# else	/* TCSIG */
	*pfrontp++ = slctab[SLC_SUSP].sptr ?
			(unsigned char)*slctab[SLC_SUSP].sptr : '\032';
# endif	/* TCSIG */
#endif	/* SIGTSTP */
}

doeof()
{
	init_termbuf();

	*pfrontp++ = slctab[SLC_EOF].sptr ?
			(unsigned char)*slctab[SLC_EOF].sptr : '\004';
}

#ifdef  IP_TOS  

struct tosent *	
gettosbyname(name,proto)

   char *name, *proto ;

{
	register struct tosent *p;
	register char **cp;
	
	settosent_local(0);
	while(p = gettosent_local()) {
        if (strcmp(name,p->t_name) == 0)
		goto gotname ;
	for (cp = p->t_aliases; *cp; cp++)
		if (strcmp(name, *cp) == 0)
			goto gotname ;
	continue ;
gotname:
		if ((proto == 0) || (*p->t_proto == '*') ||
		     (strcmp(p->t_proto, proto) == 0))
		         break ;
       }
       endtosent_local();
       return(p) ;
}
struct tosent *
gettosent_local()

{

	static char line1[BSIZ+1];
 
        if (tosf == NULL && (tosf = fopen(TOSDB,"r")) == NULL)
		return(NULL);
	if (fgets(line1, BSIZ, tosf) == NULL)
		return (NULL) ;
	return interpret_local(line1, strlen(line1));
}

static struct tosent *
interpret_local(val,len)
	char *val;
	int len;
{
	static char *tos_aliases[MAXALIASES];
	static struct tosent tos ;
	static char line[BSIZ+1];
	char *p = NULL;
	register char *cp, **q;

	strncpy(line, val, len) ;
	p = line;
	line[len] = '\n';
	if (*p == '#')
		return(gettosent_local());
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		return(gettosent_local());
	*cp = '\0' ;
	tos.t_name = p;			/* get application name */
	p = getcommon_any(p, " \t") ;
	if (p == NULL)
		return(gettosent_local());
	*p++ = '\0' ;
	while (*p == ' ' || *p == '\t')
		p++ ;
        tos.t_proto = p;		/* get protocol */
        p = getcommon_any(p, " \t") ;
	if (p == NULL)
		return (gettosent_local());
        *p++ = '\0' ;
	while (*p == ' ' || *p == '\t')
		p++ ;
	tos.t_tos = strtol(p,NULL,0);		/* get TOS bits */
	q = tos.t_aliases = tos_aliases ;
	cp = getcommon_any(p, " \t") ;
	if (cp != NULL)
		*cp++ = '\0' ;
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++ ;
			continue;
		}
		if (q < &tos_aliases[MAXALIASES - 1])
			*q++ = cp ;		/* get aliases */
		cp = getcommon_any(cp, " \t") ;
		if (cp != NULL) 
			*cp++ = '\0';
	}
	*q = NULL;
	return(&tos) ;
}

static char *
getcommon_any(cp,match) 
	register char *cp;
	char *match;
{
        register char *mp, c;

        while (c = *cp) {
                for (mp = match; *mp; mp++)
                        if (*mp == c)
                                return (cp);
                cp++;
        }
        return ((char *)0);
}

settosent_local(f)
        int f;
{
        if (tosf == NULL)
                tosf = fopen(TOSDB, "r");
        else
                rewind(tosf);
        stayopen |= f;
}
endtosent_local()
{
        if (tosf && !stayopen) {
                fclose(tosf);
                tosf = NULL;
        }
}
#endif /* IP_TOS */

