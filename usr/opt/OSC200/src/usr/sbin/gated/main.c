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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1993/10/07 23:08:52 $";
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
/*

*/
/*
 * COMPONENT_NAME: TCPIP main.c
 *
 * FUNCTIONS: procname1
 *
 * ORIGINS: 10 26 27 39 36
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   CENTER FOR THEORY AND SIMULATION IN SCIENCE AND ENGINEERING
 *			CORNELL UNIVERSITY
 *
 *      Portions of this software may fall under the following
 *      copyrights: 
 *
 *	Copyright (c) 1983 Regents of the University of California.
 *	All rights reserved.  The Berkeley software License Agreement
 *	specifies the terms and conditions for redistribution.
 *
 *  GATED - based on Kirton's EGP, UC Berkeley's routing daemon (routed),
 *	    and DCN's HELLO routing Protocol.
 */
/*
#ifndef	lint
#endif	not lint
*/

#ifdef MSG
#include "gated_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_MAIN,n,s) 
#else
#define MSGSTR(n,s) s
#endif

/* main.c
 *
 * Main function of the EGP user process. Includes packet reception,
 * ICMP handler and timer interrupt handler.
 *
 * Functions: main, recvpkt, icmpin, timeout, getod, quit, p_error
 */
/*
 * Software Overview:
 *
 * At start up, the controlling function, main(), first sets tracing options
 * based on starting command arguments. It then calls the initialization
 * functions described in the next section, sets up signal handlers for
 * termination (SIGTERM) and timer interrupt (SIGALRM) signals, calls
 * timeout() to start periodic interrupt processing, and finally waits in a 
 * loop to receive incoming EGP or ICMP redirect packets
 *
 * When an EGP packet is received, egpin() is called to handle it. It in turn
 * calls a separate function for each EGP command type, which sends the
 * appropriate response. When an ICMP packet is received icmpin() is called.
 *
 * The timer interrupt routine, timeout(), calls egpjob() to perform periodic
 * EGP processing such as reachability determination, command sending and
 * retransmission. It in turn calls separate functions to format each command
 * type. Timeout() also periodically calls rt_time() to increment route ages 
 * and delete routes when too old.
 */

#include "include.h"

#ifndef NSS
struct rip *ripmsg = (struct rip *)rip_packet;
SIGTYPE timeout();
SIGTYPE set_reinit_flag();
#endif
FILE *conf_open();
int no_config_file;

SIGTYPE setdumpflg();
SIGTYPE chgtrace();
extern SIGTYPE egpallcease();

#if     defined(AGENT_SNMP) || defined(AGENT_SGMP)
extern int snmp_init();
#endif  defined(AGENT_SNMP) || defined(AGENT_SGMP)

#ifdef  AGENT_SNMP
extern int snmpin();
extern int register_snmp_var();
#endif  AGENT_SNMP

#ifdef  AGENT_SGMP
extern int sgmpin();
extern int register_sgmp_var();
#endif  AGENT_SGMP

#ifdef  NSS
gated_init(argc, argv)
#else   NSS
main(argc, argv)
#endif  NSS
        int   argc;
        char *argv[];
{
#ifndef vax11c
  struct sigvec vec, ovec;
  int   selectbits,
        forever = TRUE,
        error = FALSE,
        arg_n = 0;
#else   vax11c
  int   i;
#endif  vax11c
  char	*cp;
  FILE  *fp;


  /* turn off audit for this process (must be privileged) */
  (void) audcntl ( SET_PROC_ACNTL, (char *)0, 0, AUDIT_OFF, 0, 0 );

#ifdef MSG
	catd = NLcatopen(MF_GATED,NL_CAT_LOCALE);
#endif

  getod();		/* start time */

  if ( !(my_hostname = calloc(MAXHOSTNAMELENGTH+1, sizeof(char))) ) {
    p_error("main: calloc");
    quit();
  }
  if ( gethostname(my_hostname, MAXHOSTNAMELENGTH+1) ) {
    p_error("main: gethostname");
    quit();
  }
  	
  /* check arguments for turning on tracing and a trace file */

  my_name = argv[0];
  if (cp = rindex(my_name, '/')) {
    my_name = cp + 1;
  }
  tracing = savetrace = are_tracing = 0;
  logfile = NULL;
#ifndef vax11c
  while (--argc > 0 && !error) {
    argv++;
    arg_n++;
    switch (arg_n) {
      case 1:
        cp = *argv;
        if (*cp++ != '-') {
          if (argc > 1) {
            error = TRUE;
          } else {
            logfile = *argv;
          }
        } else if (*cp++ != 't') {
          error = TRUE;
        } else if (*cp == '\0') {
          savetrace = TR_INT | TR_EXT | TR_RT | TR_EGP;
        } else {
          while (*cp != '\0') {
            switch (*cp++) {
              case 'i':
                savetrace |= TR_INT;
                break;
              case 'e':
                savetrace |= TR_EXT;
                break;
              case 'r':
                savetrace |= TR_RT;
                break;
              case 'p':
                savetrace |= TR_EGP;
                break;
              case 'u':
                savetrace |= TR_UPDATE;
                break;
#ifdef  NSS
              case 'I':
                savetrace |= TR_ISIS;
                break;
              case 'E':
                savetrace |= TR_ESIS;
                break;
#else   NSS
              case 'R':
                savetrace |= TR_RIP;
                break;
              case 'H':
                savetrace |= TR_HELLO;
                break;
#endif  NSS
#if     defined(AGENT_SNMP) || defined(AGENT_SGMP)
              case 'M':
	        savetrace |= TR_SNMP;
	        break;
#endif  defined(AGENT_SNMP) || defined(AGENT_SGMP)
              default:
                error = TRUE;
            }
          }
        }
        break;
      case 2:
        logfile = *argv;
        break;
      default:
        error = TRUE;
    }
  }
  if (error) {
    fprintf(stderr, MSGSTR(MAIN_4,"Usage: %s [-t [i][e][r][p][u][R][H]] [logfile]\n"), my_name);
    exit(1);
  }
  if ((savetrace == 0) || (logfile != NULL)) {
    int t;

  if (fork()) {
    exit(0); 
  }
#if BSD42 || UTX32_1_X
    t = 20;
#else BSD42 || UTX32_1_X
    t = getdtablesize();
#endif BSD42 || UTX32_1_X
    for (t--; t >= 0; t--) {
	(void) close(t);
    }
    (void) open("/dev/null", O_RDONLY);
    (void) dup2(0, 1);
    (void) dup2(0, 2);
    t = open("/dev/tty", O_RDWR);
    if (t >= 0) {
      (void) ioctl(t, TIOCNOTTY, (char *)NULL);
      (void) close(t);
    }
  }
  my_pid = getpid();

  openlog(my_name, LOG_PID|LOG_CONS|LOG_NDELAY, LOG_FACILITY);
#ifndef NSS
  (void) setlogmask(LOG_UPTO(LOG_NOTICE));
#endif

  if ( savetrace ) {
    (void) traceon(logfile, GEN_TRACE);
  } else if (savetrace = traceflags()) {
    (void) traceon(logfile, GEN_TRACE);
  }

#else   vax11c
  for(i = 1; i < argc; i++) {
        if (strcasecmp(argv[i],"bootfile") == 0) {
                if (i >= argc) {
                        printf("ERROR: No GATED boot file specified!\n");
                        return;
                }
         	 i++;
                EGPINITFILE = argv[i];
                continue;
        }
        if (strcasecmp(argv[i],"trace") == 0) {
                tracing = TR_INT | TR_EXT | TR_RT | TR_EGP;
                continue;
	if (strcasecmp(argv[i],"trace-all") == 0) {
                tracing = TR_INT | TR_EXT | TR_RT | TR_EGP |
                          TR_UPDATE | TR_RIP | TR_HELLO;
                continue;
        }
        if (strcasecmp(argv[i],"trace-internal-errors") == 0) {
       		tracing |= TR_INT;
                continue;
        }
        if (strcasecmp(argv[i],"trace-external-changes") == 0) {
                tracing |= TR_EXT;
                continue;
        }
	if (strcasecmp(argv[i],"trace-routing-changes") == 0) {
                tracing |= TR_RT;
                continue;
        }
        if (strcasecmp(argv[i],"trace-packets") == 0) {
                tracing |= TR_EGP;
                continue;
        }
	if (strcasecmp(argv[i],"trace-egp-updates") == 0) {
                tracing |= TR_UPDATE;
                continue;
        }
        if (strcasecmp(argv[i],"trace-rip-updates") == 0) {
                tracing |= TR_RIP;
                continue;
        }
	if (strcasecmp(argv[i],"trace-hello-updates") == 0) {
                tracing |= TR_HELLO;
                continue;
        }
#if     defined(AGENT_SNMP) || defined(AGENT_SGMP)
        if (strcasecmp(argv[i],"trace-snmp") == 0) {
                tracing |= TR_SNMP;
                continue;
        }
#endif  defined(AGENT_SNMP) || defined(AGENT_SGMP)
        printf("GATED bad arg \"%s\"\n",argv[i]);
        return;
  }
#endif  /* vax11c */
  if (tracing) {
    are_tracing++;
  }

  syslog(LOG_NOTICE, MSGSTR(MAIN_10,"Start %s version %s"), my_name, Version);
  TRACE_TRC(MSGSTR(MAIN_11,"Start %s[%d] version %s at %s\n"), my_name, my_pid, Version, strtime);

/* open initialization file */
  no_config_file = FALSE;
  if ( (fp = conf_open()) == NULL) {
    quit();
  }
#ifndef	vax11c
  setnetent(1);
  sethostent(1);
#endif
  martian_init();	/* initialize martian net table */
  rt_init();		/* initialize route hash tables */
  init_options(fp);	/* initialize protocol options */
#ifdef	NSS
  init_if(fp);  		/* initialize interface tables */
#else 	NSS
  init_if();  		/* initialize interface tables */
#endif	/* NSS	*/
  rt_resinit(fp);	/* initialize routing restriction tables. */
  rt_ASinit(fp, TRUE);	/* initialize AS routing restrictions. */
  rt_NRadvise_init(fp);	/* initialize interior routes to be EGP advised */

#ifdef  NSS
  s = getsocket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) {
    quit();
  }
#else   NSS
  /*
   * only need one socket per protocol type in 4.3bsd
   */
  icmp_socket = getsocket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (icmp_socket < 0) {
    quit();
  }
  if (doing_egp) {
    egp_socket = get_egp_socket(AF_INET, SOCK_RAW, IPPROTO_EGP);
    if (egp_socket < 0) {
      quit();
    }
  }
  if (doing_hello) {
#ifdef  SO_RCVBUF
    int on = 48*1024;
#endif  SO_RCVBUF
    hello_socket = getsocket(AF_INET, SOCK_RAW, IPPROTO_HELLO);
    if (hello_socket < 0) {
      quit();
    }
#ifdef  SO_RCVBUF
    if (setsockopt(hello_socket, SOL_SOCKET, SO_RCVBUF, (char *)&on, sizeof(on))
 < 0) {
      p_error("init_hello: setsockopt SO_RCVBUF");
    } else {
      TRACE_TRC("main: HELLO receive buffer size set to %dK\n", on/1024);
    }
#endif  SO_RCVBUF
  }
  if (doing_rip)
    rip_socket = rip_init();

  s = icmp_socket;   	/* socket desc. for ioctl calls to install routes */
#endif	/* NSS	*/

#ifdef AGENT_SNMP
  if ((snmp_socket = snmp_init()) < 0) {
    syslog(LOG_ERR, MSGSTR(MAIN_12,"main: can't open snmp socket"));
    quit();
  }
#endif /* AGENT_SNMP */
#ifdef AGENT_SGMP
  if ((sgmp_socket = snmp_init()) < 0) {
    syslog(LOG_ERR, MSGSTR(MAIN_12,"main: can't open sgmp socket"));
    quit();
  }
#endif /* AGENT_SGMP */

  if (doing_egp) {
    init_egpngh(fp);	/* read egp neighbor list */
    init_egp();		/* initialize EGP neighbor tables */
  }
#ifndef NSS
  rt_readkernel();	/* Initailize routing tables with kernel's */
  rt_ifoptinit(fp);	/* Initialize options for interfaces */
  rt_ifinit();		/* initialize interior routes for direct nets */

  if (doing_rip && rip_supplier < 0) {
    rip_supplier = FALSE;
  }
  if (doing_hello && hello_supplier < 0) {
    hello_supplier = FALSE;
  }

  if (no_config_file && !rip_supplier && rt_locate( (int)EXTERIOR, &hello_dfltnet, RTPROTO_KERNEL) ) {
    char *s = MSGSTR(MAIN_13,"No config file, one interface and a default route, gated exiting\n");
    TRACE_TRC(s);
    syslog(LOG_NOTICE,s);
    quit();
  }

  if (doing_rip) {
    ripmsg->rip_cmd = RIPCMD_REQUEST;
    ripmsg->rip_vers = RIPVERSION;
    ripmsg->rip_nets[0].rip_dst.sa_family = AF_UNSPEC;
    ripmsg->rip_nets[0].rip_metric = RIPHOPCNT_INFINITY;
    ripmsg->rip_nets[0].rip_dst.sa_family = htons(AF_UNSPEC);
    ripmsg->rip_nets[0].rip_metric = htonl((u_int)RIPHOPCNT_INFINITY);
    toall(sendripmsg);
  }
#else   NSS
  addrouteforbackbone();
#endif  NSS

  install = TRUE;	/* install routes from now on */

  if (install) {
    TRACE_TRC(MSGSTR(MAIN_14,"\n***Routes are being installed in kernel\n\n"));
  } else {
    TRACE_TRC(MSGSTR(MAIN_15,"\n***Routes are not being installed in kernel\n\n"));
  }

#ifndef NSS
  rt_dumbinit(fp);	/* initialize static routes gateways */
#endif

  (void) fclose(fp);

#ifndef vax11c
  fp = fopen(PIDFILE, "w");
  if (fp != NULL) {
    fprintf(fp, "%d\n", my_pid);
    (void) fclose(fp);
  }

  fp = fopen(VERSIONFILE, "w");
  if (fp != NULL) {
    fprintf(fp, MSGSTR(MAIN_19,"%s version %s \n\tpid %d, started %s"),
      my_name, Version, my_pid, strtime);
    (void) fclose(fp);
  }

  /* Setup signal processing */
  bzero((char *)&vec, sizeof(struct sigvec));
  vec.sv_mask = sigmask(SIGIO) | sigmask(SIGALRM) | sigmask(SIGTERM) | sigmask(SIGUSR1) | sigmask(SIGINT) | sigmask(SIGHUP);

  /* SIGTERM to terminate */
  vec.sv_handler = egpallcease;
  if (error = sigvec(SIGTERM, &vec, &ovec)) {
    p_error("main: sigvec SIGTERM");
    quit();
  }
#ifndef NSS
  /* SIGALRM for route delete processing */
  vec.sv_handler = timeout;
  if (error = sigvec(SIGALRM, &vec, &ovec)) {
    p_error("main: sigvec SIGALRM");
    quit();
  }
  /* SIGUSR1 for reconfiguration */
  vec.sv_handler = set_reinit_flag;
  if (error = sigvec(SIGUSR1, &vec, &ovec)) {
    p_error("main: sigvec SIGUSR1");
    quit();
  }
#endif /* NSS */
  /* SIGINT for dumps */
  vec.sv_handler = setdumpflg;
  if (error = sigvec(SIGINT, &vec, &ovec)) {
    p_error("main: sigvec SIGINT");
    quit();
  }
  /* SIGHUP for tracing */
  vec.sv_handler = chgtrace;
  if (error = sigvec(SIGHUP, &vec, &ovec)) {
    p_error("main: sigvec SIGHUP");
    quit();
  }
#endif /* vax11c */

  TRACE_INT("\n");
  init_display_config("main: RIP", doing_rip, rip_supplier, rip_gateway, rip_pointopoint, rip_default);
  init_display_config("main: HELLO", doing_hello, hello_supplier, hello_gateway, hello_pointopoint, hello_default);
  init_display_config("main: EGP", doing_egp, -1, 0, 0, 0);
  TRACE_INT(MSGSTR(MAIN_29,"main: commence routing updates:\n\n"));

  timeout();

#ifndef vax11c
#ifndef NSS
  /* wait to receive HELLO, RIP, EGP, ICMP or IMP messages */

  selectbits = 0;			/* select descriptor mask */
  if (doing_egp)
    selectbits |= 1 << egp_socket;	/* EGP socket */
  selectbits |= 1 << icmp_socket;	/* ICMP socket */
  if (doing_hello)
    selectbits |= 1 << hello_socket;	/* HELLO socket */
  if (doing_rip)
    selectbits |= 1 << rip_socket; 	/* RIP socket */
#ifdef AGENT_SNMP
  selectbits |= 1 << snmp_socket;       /* SNMP socket */
#endif AGENT_SNMP
#ifdef AGENT_SGMP
  selectbits |= 1 << sgmp_socket;       /* SGMP socket */
#endif AGENT_SGMP
#endif  NSS
	
  endnetent();
  endhostent();

#ifndef NSS
  while (forever) {
    int ibits;
    register int n;

    ibits = selectbits;
    n = select(20, (struct fd_set *)&ibits, (struct fd_set *)0,
                          (struct fd_set *)0, (struct timeval *)0);
    if (n < 0) {
      if (errno != EINTR) {
        p_error("main: select");
        quit();
      }
      continue;
    }
    if (doing_egp && (ibits & (1 << egp_socket)))
      recvpkt(egp_socket, IPPROTO_EGP);
    if (ibits & (1 << icmp_socket))
      recvpkt(icmp_socket, IPPROTO_ICMP);
    if (doing_hello && (ibits & (1 << hello_socket)))
      recvpkt(hello_socket, IPPROTO_HELLO);
    if (doing_rip && (ibits & (1 << rip_socket)))
      recvpkt(rip_socket, IPPROTO_RIP);
#ifdef AGENT_SNMP
    if (ibits & (1 << snmp_socket))
      recvpkt(snmp_socket, IPPROTO_SNMP);
#endif AGENT_SNMP
#ifdef AGENT_SGMP
    if (ibits & (1 << sgmp_socket))
      recvpkt(sgmp_socket, IPPROTO_SGMP);
#endif AGENT_SGMP
  }
#endif  NSS
#else   vax11c
  /*
   *    Wait for data arrival on appropriate sockets
   */
  if (doing_egp) Setup_VMS_Receive(IPPROTO_EGP, egp_socket);
  Setup_VMS_Receive(IPPROTO_ICMP, icmp_socket);
  if (doing_hello) Setup_VMS_Receive(IPPROTO_HELLO, hello_socket);
  if (doing_rip) Setup_VMS_Receive(IPPROTO_RIP, rip_socket);
#ifdef AGENT_SNMP
  Setup_VMS_Receive(IPPROTO_SNMP, snmp_socket);
#endif AGENT_SNMP
#ifdef AGENT_SGMP
  Setup_VMS_Receive(IPPROTO_SGMP, sgmp_socket);
#endif AGENT_SGMP
#endif  vax11c
}

#ifndef NSS
recvpkt(sock, protocol)
        int sock, protocol;
{
  char packet[SLOP(MAXPACKETSIZE)];  	/* packet buffer */
  struct sockaddr_in from;
  int fromlen = sizeof(from), count, omask;

  bzero((char *)&from, sizeof(from));

  getod();			/* current time */
  count = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&from, &fromlen);
  if (count <= 0) {
    if (count < 0 && errno != EINTR)
      p_error(MSGSTR(MAIN_91,"recvpkt: recvfrom"));
    return;
  }
  if (fromlen != sizeof (struct sockaddr_in)) {
    syslog(LOG_ERR, MSGSTR(MAIN_92,"recvpkt: fromlen %d invalid\n\n"), fromlen);
    return;
  }
  bzero((char *)from.sin_zero, sizeof(from.sin_zero));

  if (count > sizeof(packet)) {
    syslog(LOG_ERR, MSGSTR(MAIN_93,"recvfrom: packet discarded, length %d > %d"),
                       count, sizeof(packet));
    syslog(LOG_ERR, MSGSTR(MAIN_94,", from addr %s\n\n"), inet_ntoa(from.sin_addr));
    return;
  }

#ifndef vax11c
#define	mask(s)	(1<<((s)-1))

  omask = sigblock(mask(SIGALRM | SIGTERM));
#endif	/* vax11c */

  switch (protocol) {
    case IPPROTO_EGP:
      if (tracing & TR_EGP) 
        tracercv(sock, protocol, packet, count, &from);
      egpin(packet);
      break;
    case IPPROTO_ICMP:
      icmpin(packet, &from);
      break;
    case IPPROTO_HELLO:
      helloin(packet);
      break;
    case IPPROTO_RIP:
      ripin((struct sockaddr *)&from, count, packet);
      break;
#ifdef AGENT_SNMP
    case IPPROTO_SNMP:
      snmpin(&from, count, packet);
      break;
#endif AGENT_SNMP
#ifdef AGENT_SGMP
    case IPPROTO_SGMP:
      sgmpin(&from, count, packet);
      break;
#endif AGENT_SGMP
#ifdef IMPLINK_IP
    case IMPLINK_IP:
      break;  		/* not implemented - need care with different
                         * address family for imp?
                         */
#endif
  }
#ifndef vax11c
  (void) sigsetmask(omask);
#endif /* vax11c */
}
#endif /* NSS */


/*
 * timer control for periodic route-age and interface processing.
 * timeout() is called when the periodic interrupt timer expires.
 */

SIGTYPE timeout()
{
  static unsigned int
      		next_egpjob = 0,
		next_rttime = 0,
#ifndef NSS
		next_ripjob = 0,
		next_hellojob = 0,
#endif /* NSS */
		next_timestamp = 0,
		next_iftime = 0;
  register unsigned int interval;
#ifndef NSS
  struct itimerval  value, ovalue;
#endif

  getod();
#ifndef NSS
  TRACE_JOB(MSGSTR(MAIN_95,"JOB BEGIN:\ttime: %u egp: %u rip: %u hello: %u\n\t\trt: %u if: %u stamp: %u\n"),
            gatedtime, next_egpjob, next_ripjob, next_hellojob,
            next_rttime, next_iftime, next_timestamp);
#else   NSS
  TRACE_JOB("JOB BEGIN:\ttime: %u egp: %u rt: %u if: %u\n",
            gatedtime, next_egpjob, next_rttime, next_iftime);
#endif  NSS

  if (next_rttime == 0)	{		/* initialization */
    next_rttime = gatedtime + RT_TIMERRATE;
  }

  interval = gatedtime+1000;	/* some large value */

  if (gatedtime >= next_timestamp) {
    TRACE_STAMP(MSGSTR(MAIN_96,"TIMESTAMP: %s"), strtime);
    next_timestamp = gatedtime+TIME_STAMP;
  }

  if (doing_egp && gatedtime >= next_egpjob) {
    TRACE_JOB(MSGSTR(MAIN_97,"JOB CALL:\tegpjob()\n"));
    egpjob();
    TRACE_JOB(MSGSTR(MAIN_98,"JOB RETURN:\tegpjob()\n"));
    next_egpjob = gatedtime + egpsleep;
  }
  if (doing_egp && interval > next_egpjob)
      interval = next_egpjob;
#ifndef NSS
  if (doing_rip && gatedtime >= next_ripjob) {
    if (rip_supplier) {
      TRACE_JOB(MSGSTR(MAIN_99,"JOB CALL:\ttoall()\n"));
      toall(supply);
      TRACE_JOB(MSGSTR(MAIN_100,"JOB RETURN:\ttoall()\n"));
    }
    next_ripjob = gatedtime + RIP_INTERVAL;
  }
  if ( doing_rip && interval > next_ripjob) {
      interval = next_ripjob;
  }
  if (doing_hello && gatedtime >= next_hellojob) {
    if (hello_supplier) {
      TRACE_JOB(MSGSTR(MAIN_101,"JOB CALL:\thellojob()\n"));
      hellojob();
      TRACE_JOB(MSGSTR(MAIN_102,"JOB RETURN:\thellojob()\n"));
    }
    next_hellojob = gatedtime + HELLO_TIMERRATE;
  }
  if (doing_hello && interval > next_hellojob)
      interval = next_hellojob;
#endif /* NSS */
  if (gatedtime >= next_rttime) {
    TRACE_JOB(MSGSTR(MAIN_103,"JOB CALL:\trt_time()\n"));
    rt_time();
    TRACE_JOB(MSGSTR(MAIN_104,"JOB RETURN:\trt_time()\n"));
    next_rttime = gatedtime + RT_TIMERRATE;
    if (sched_a_dump == 1) {
      TRACE_JOB(MSGSTR(MAIN_105,"JOB CALL:\tdumpinfo()\n"));
      dumpinfo();
#ifdef  NSS
      TRACE_JOB("JOB CALL:\tl2lsdb_dump()\n");
      l2lsdb_dump();
      TRACE_JOB("JOB RETURN:\tl2lsdb_dump()\n");
#endif  /* NSS */
      TRACE_JOB(MSGSTR(MAIN_106,"JOB RETURN:\tdumpinfo()\n"));
      sched_a_dump = 0;
    }
  }
  if (interval > next_rttime)
      interval = next_rttime;
#ifndef NSS
  if (gatedtime >= next_iftime) {
    TRACE_JOB(MSGSTR(MAIN_107,"JOB CALL:\tif_check()\n"));
    if_check();
    TRACE_JOB(MSGSTR(MAIN_108,"JOB RETURN:\tif_check()\n"));
#ifdef AGENT_SNMP
    register_snmp_vars();
#endif AGENT_SNMP
#ifdef  AGENT_SGMP
    register_sgmp_vars();
#endif  AGENT_SGMP
    next_iftime = gatedtime + CHECK_INTERVAL;
  }
  if (interval > next_iftime) {
      interval = next_iftime;
  }

  if (do_reinit) {
    do_reinit = 0;
    reinit();
  }
#endif /* NSS */

#ifndef NSS
  TRACE_JOB(MSGSTR(MAIN_109,"JOB END:\ttime: %u egp: %u rip: %u hello: %u\n\t\trt: %u if: %u stamp: %u\n"),
            gatedtime, next_egpjob, next_ripjob, next_hellojob,
            next_rttime, next_iftime);
#else   NSS
  TRACE_JOB("JOB END:\ttime: %u egp: %u rt: %u if: %u\n",
            gatedtime, next_egpjob, next_rttime, next_iftime);
#endif  /* NSS */
  TRACE_JOB(MSGSTR(MAIN_110,"JOB TIME:\tinterval: %u delta: %u\n"), interval, interval-gatedtime);

  interval -= gatedtime;
#ifndef NSS
  value.it_interval.tv_sec = 0;		/* no auto timer reload */
  value.it_interval.tv_usec = 0;
  value.it_value.tv_sec = interval;
  value.it_value.tv_usec = 0;
#ifndef vax11c
  (void) setitimer(ITIMER_REAL, &value, &ovalue);
#else   vax11c
  Set_VMS_Timeout(&value, timeout, 0, "GATED_timeout()");
#endif  vax11c
#endif  NSS
  return;
}

/*
 * get time of day in seconds and as an ASCII string.
 * Called at each interrupt and saved in external variables.
 */

getod()
{
  struct timeval tp;
  struct timezone tzp;

  if (gettimeofday (&tp, &tzp))
    p_error("getod: gettimeofday");
  gatedtime = tp.tv_sec;				/* time in seconds */
  strtime = (char *) ctime(&gatedtime);		 /* time as an ASCII string */
  return;
}


/* exit gated */

quit()
{
  if (rt_default_active == TRUE) {
    (void) rt_default(MSGSTR(MAIN_112,"DELETE"));
  }
  syslog(LOG_NOTICE, MSGSTR(MAIN_113,"Exit %s version %s"), my_name, Version);
  getod();
  TRACE_TRC(MSGSTR(MAIN_114,"\nExit %s[%d] version %s at %s\n"), my_name, my_pid, Version, strtime);
  exit(1);
}

/*
 * Print error message to system logger.
 *
 * First flush stdout stream to preserve log order as perror writes directly
 * to file
 */

p_error(str)
	char *str;
{
  int error_number = errno;

  TRACE_TRC("%s: %s\n", str, gd_error(error_number));
#ifdef	notdef
  (void) fflush(stdout);
#endif	notdef
  syslog(LOG_ERR, "%s: %s", str, gd_error(error_number));
}

/*
 * set dump flag so the database may be dumped.
 */
SIGTYPE setdumpflg()
{
	sched_a_dump = 1;
	getod();
	syslog(LOG_NOTICE, MSGSTR(MAIN_117,"setdumpflg: dump request received"));
	TRACE_TRC(MSGSTR(MAIN_118,"setdumpflg: dump request received at %s"), strtime);
}

/*
 * toggle the trace on/off on a HUP.
 */
SIGTYPE chgtrace()
{
  int new_flags;

  if (logfile == NULL) {
    syslog(LOG_ERR, MSGSTR(MAIN_119,"chgtrace: can not toggle tracing to console"));
    return;
  }
  if (are_tracing) {
    traceoff(GEN_TRACE);
    are_tracing = 0;
  } else {
    if ( (new_flags = traceflags()) ) {
      savetrace = new_flags;
    }
    traceon(logfile, GEN_TRACE);
    are_tracing = 1;
  }
}

#ifndef NSS
/*
 *	set_reinit_flag() - Set re-init flag to re-read config file
 */

SIGTYPE set_reinit_flag()
{
	do_reinit = 1;
	getod();
	syslog(LOG_NOTICE, MSGSTR(MAIN_120,"set_reinit_flag: re-init request received"));
	TRACE_TRC(MSGSTR(MAIN_121,"set_reinit_flag: re-init request received at %s"), strtime);
}
#endif /* NSS */

reinit()
{
  FILE *fp;

  if ( (fp = conf_open()) == NULL) {
    syslog(LOG_WARNING, MSGSTR(MAIN_122,"main: initialization file %s missing"), EGPINITFILE);
    TRACE_TRC(MSGSTR(MAIN_123,"main: initialization file %s missing at %s"), EGPINITFILE, strtime);
    return;
  }

  syslog(LOG_INFO, MSGSTR(MAIN_124,"reinit: reinitializing from %s started"), EGPINITFILE);
  TRACE_TRC(MSGSTR(MAIN_125,"reinit: reinitializing from %s started at %s"), EGPINITFILE, strtime);

  /* Need to add code to set the tracing depending on traceflags */

  rt_ASinit(fp, FALSE);
#ifndef NSS
  if_check();
#endif /* NSS */
#ifdef AGENT_SNMP
  register_snmp_vars();
#endif /* AGENT_SNMP */
#ifdef  AGENT_SGMP
  register_sgmp_vars();
#endif  AGENT_SGMP

  (void) fclose(fp);

  syslog(LOG_INFO, MSGSTR(MAIN_126,"reinit: reinitializing from %s done"), EGPINITFILE);
  TRACE_TRC(MSGSTR(MAIN_127,"reinit: reinitializing from %s done at %s"), EGPINITFILE, strtime);

  return;
}

FILE *conf_open()
{
  FILE *fp;

  if ( (fp = fopen(EGPINITFILE, "r")) == NULL) {
    syslog(LOG_WARNING, MSGSTR(MAIN_129,"conf_open: initialization file %s missing - using defaults\n"), EGPINITFILE);
    TRACE_TRC(MSGSTR(MAIN_129,"conf_open: initialization file %s missing - using defaults\n"), EGPINITFILE);
    no_config_file = TRUE;
    if ( (fp = fopen(EGPINITFILE = "/dev/null", "r")) == NULL) {
      syslog(LOG_WARNING, MSGSTR(MAIN_133,"conf_open: error opening %s\n"), EGPINITFILE);
      TRACE_TRC(MSGSTR(MAIN_133,"conf_open: error opening %s\n"), EGPINITFILE);
    }
  }
  return(fp);
}
