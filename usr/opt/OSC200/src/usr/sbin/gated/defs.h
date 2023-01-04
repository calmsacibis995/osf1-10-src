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
/*	
 *	@(#)$RCSfile: defs.h,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1992/09/30 14:12:32 $
 */ 
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
 * defs.h
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */

/*
 * COMPONENT_NAME: TCPIP defs.h
 *
 * FUNCTIONS: 
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
 *
 *
 */

/* defs.h
 *
 * Compiler switches and miscellaneous definitions.
 */

/* compiler switches */

#define INSTALL 1		/* 0 => debugging - dont install routes in 
					kernel. Reference in main() */

#define SAMENET 0		/* 1 => all egp neighbors must have the same
					shared net in common. Referenced in
					init_egpngh() */

#ifndef	LOG_FACILITY
#define	LOG_FACILITY LOG_DAEMON	/* Insure that syslog facility is defined. */
#endif	/* LOG_FACILITY */

/* Systems that use the BSD 4.3 syslog facility */
#if     defined(BSD43) || defined(BETA_BSD43) || defined(UTX32_2_X) || defined(RT43) || defined(SUNOS)
#define	SYSLOG_43
#endif

/* Systems that include the IP header with ICMP packets */
#define	ICMP_IP_HEADER

/* Systems with compiler bugs that put char arrays on odd byte boundaries if
 * the array size is odd */
#define SLOP(x) (x+1)

/* Systems that have problems with sscanf */
#define	SSCANF_BROKEN

#if	!defined(SIGTYPE)
#define	SIGTYPE	void
#endif	/* !defined(SIGTYPE) */

/* Some systems do not define sigmask (Ultrix) */
#if	!defined(sigmask)
#define sigmask(m)      (1 << ((m)-1))
#endif	/* !defined(sigmask) */

/* initialization file */
#ifndef vax11c
#ifndef NSS
#define INITFILE	"/etc/gated.conf"
#define PIDFILE		"/var/run/gated.pid"
#define	VERSIONFILE	"/etc/gated.version"
#define DUMPFILE	"/usr/tmp/gated_dump"
#else   /* NSS */
#define INITFILE        "/etc/rcp_routed.conf"
#define PIDFILE         "/var/run/rcp_routed.pid"
#define VERSIONFILE     "/etc/rcp_routed.version"
#define DUMPFILE        "/usr/tmp/rcp_routed.dump"
#endif  /* NSS */
#else   /* vax11c */
#define DUMPFILE        "MULTINET:GATEWAY-DAEMON.DUMP"
#endif  /* vax11c */
extern	char *Gated_Configuration_File;
#define EGPINITFILE	Gated_Configuration_File

extern char Version[];
extern char *build_date;
extern char *version_kernel;
extern char *my_hostname;

/* general definitions for GATED user process */

#define TRUE	 1
#define FALSE	 0
#define ERROR	-1			/* used in rt_mknr() and rt_NRnets()*/
#define NOERROR -2			/* used in egppoll() */

#ifndef NULL
#define NULL	 0L
#endif

#define LOOPBACKNET	(127 << 24)  /* loopback network */

#define MAXHOSTNAMELENGTH 64		/*used in init_egpngh & rt_dumb_init*/

#undef  MAXPACKETSIZE
#define EGPMAXPACKETSIZE 8192
#define HELLOMAXPACKETSIZE 1440
#define RIPPACKETSIZE 512

#if EGPMAXPACKETSIZE > RIPPACKETSIZE
#  if HELLOMAXPACKETSIZE > EGPMAXPACKETSIZE
#    define MAXPACKETSIZE HELLOMAXPACKETSIZE
#  else
#    define MAXPACKETSIZE EGPMAXPACKETSIZE
#  endif
#else
#  if HELLOMAXPACKETSIZE > RIPPACKETSIZE
#    define MAXPACKETSIZE HELLOMAXPACKETSIZE
#  else
#    define MAXPACKETSIZE RIPPACKETSIZE
#  endif
#endif

/* macros to select internet address given pointer to a struct sockaddr */

/* result is u_int */
#define sock_inaddr(x) (((struct sockaddr_in *)(x))->sin_addr)

/* result is struct in_addr */
#define in_addr_ofs(x) (((struct sockaddr_in *)(x))->sin_addr)


/* additional definitions to netinet/in.h */

#ifndef IPPROTO_EGP
#define IPPROTO_EGP 8
#endif

/* definitions from C-gateway */

#define reg register
#define ext extern

#define	AMSK	0200		/* Mask values used to decide on which */
#define	AVAL	0000		/* class of address we have */
#define	BMSK	0300
#define	BVAL	0200
#define	CMSK	0340		/* The associated macros take an arg */
#define	CVAL	0300		/* of the form in_addr.i_aaddr.i_anet */

#define	in_isa(x)	(((x) & AMSK) == AVAL)
#define	in_isb(x)	(((x) & BMSK) == BVAL)
#define	in_isc(x)	(((x) & CMSK) == CVAL)

#define CLAA 1
#define CLAB 2
#define CLAC 3

/* definitions from routed/defs.h */

#define equal(a1, a2) \
	(bcmp((caddr_t)(a1), (caddr_t)(a2), sizeof (struct sockaddr)) == 0)


/* system definitions */

extern	char *err_message;
extern	int errno;


/* external definitions */

extern char *my_name;                    /* name we were invoked as */
extern int my_pid;                       /* my process ID */
extern FILE *ftrace;
extern int tracing;			 /* trace packets and route changes */
extern int savetrace;			 /* save tracing flags */
extern int are_tracing;			 /* are we tracing? */
extern char *logfile;			 /* the log file */
extern int n_interfaces;		 /* # internet interfaces */
extern int n_remote_nets;		 /* # remote nets via internal 
						non-routing gateways */
extern int n_routes;			/* # networks in routing tables */
extern struct rthash nethash[];  	/* net routes */
extern struct rthash hosthash[]; 	/* host routes */
extern struct restricthash rt_restrict[]; /*restrict control routes*/
extern struct interface *ifnet;	   /* direct internet interface list */
extern int terminate;		   /* terminate EGP process - set by
					egpallcease(); tested by 
					egpstunacq() and egpacq() */
extern int	s;		/* socket for ioctl calls installing routes,
				   set in main() */
extern int	rip_socket;	/* rip socket for sending rip packets */
extern int	hello_socket;	/* hello socket for sending hello packets */
extern int	egp_socket;	/* egp socket for sending egp packets */
extern int	icmp_socket;	/* icmp socket for listening icmp packets */
extern int	install;	/* if TRUE install route in kernelcall kernel,
				 * it is set by main() after kernel routes
				 * initially read and tested in table2.c */
extern int  rt_default_active;	/* TRUE if gateway default is active */
extern u_short	mysystem;		/* autonomous system number */
extern struct as_list *my_aslist;	/* list of autonomous systems to announce to my AS */
extern	int	nneigh;			/* number of trusted neighbors in 
						egpnn[] */
extern	int	maxacq;		/* maximum number neighbors to be acquired */
extern  int	n_acquired;	/* number neighbors acquired */
extern	int	egpsleep;	/* No. seconds between egpjob wakeups.
				   Time computed when neigh. (re)acquired 
				   or dropped */
extern	struct egpngh *egpngh;	/* start of linked list of egp neighbor state
				   tables */

extern	u_short	egprid_h;	/* sequence number of received egp packet
				   in host byte order - all ids in internal
				   tables are in host byte order */
extern  int	rt_maxage;	/* maximum allowed age of any route since last
				   updated by an NR message */
extern	int	maxpollint;	/* maximum poll interval of acquired neighbors
				   set in egpstime(), used in rt_NRupdate() */
extern	time_t	gatedtime;	/* time of day in seconds of current interrupt */
extern	char	*strtime;	/* time of day as an ASCII string */
extern	time_t	last_time;	/* Last time rt_time was run */
extern  int     sched_a_dump;   /* flag to schedule a dump */
extern	int	do_reinit;	/* flag to indicate a reinit should be done */

extern	int	conv_factor;		/* conversion factor for RIP, HELLO to
					EGP metrics */
extern	int	doing_rip;		/* Are we running RIP protocols? */
extern	int	doing_hello;		/* Are we running HELLO protocols? */
extern	int	doing_egp;		/* Are we running EGP protocols? */
extern  int	doing_snmp;		/* Are we talking with a SNMP agent? */
extern	int	rip_pointopoint;	/* Are we ONLY doing pointopoint RIP? */
extern	int	hello_pointopoint;	/* ONLY doing pointopoint HELLO? */
extern	int	rip_supplier;		/* Are we broadcasting RIP protocols? */
extern	int	hello_supplier;		/* Are we broadcasting HELLO info? */
extern	int	rip_gateway;		/* Are we sending a RIP default? */
extern	int	hello_gateway;		/* Are we sending a HELLO default? */
extern	int	rip_default;		/* Default metric for RIP default */
extern	int	hello_default;		/* Default metric for HELLO default */
extern	int	announcethesenets;	/* Announce restriction list? */
extern	int	donotannounce;		/* Forbid restriction list? */
extern	int	glob_announcethesenets;	/* Global announce restriction list? */
extern	int	glob_donotannounce;	/* Global forbid restriction list? */
extern  int	donotlisten;		/* is there a listen Black list? */
extern  int	islisten;		/* a source listen Black list? */
extern	int	rttable_changed;	/* Routing table has been changed */

extern struct rt_entry *default_gateway; /* Default gateway for dynamic restore */
extern struct sockaddr_in hello_dfltnet;

struct advlist {
	struct advlist *next;
	struct in_addr destnet;
	};
extern struct advlist *adlist;
extern struct advlist *srcriplist;
extern struct advlist *srchellolist;
extern struct advlist *trustedripperlist;
extern struct advlist *trustedhelloerlist;
extern struct advlist *martians;

extern struct as_list *sendas;
extern struct as_valid *validas;

#ifndef RIPCMDS           		/* only external when necessary */
extern char *ripcmds[];
#endif /* RIPCMDS   */

/* function type declarations */

extern void *calloc();
extern void *malloc();
extern char *inet_ntoa();

extern struct rt_entry *rt_add();
extern struct rt_entry *rt_lookup();
extern struct rt_entry *rt_find();
extern struct rt_entry *rt_locate();

extern struct interface *if_withdst();
extern struct interface *if_ifwithaddr();

extern struct restrictlist *control_lookup();

extern u_int gd_inet_wholenetof();
extern u_int gd_inet_netof();
extern u_int gd_inet_lnaof();
extern u_int gd_inet_isnetwork();
extern char *gd_inet_ntoa();
extern struct in_addr gd_inet_makeaddr();

/* Error message defines (provided by system on entry to main()) */

extern int errno;		/* last error number returned by system call */
extern int sys_nerr;		/* number of system-defined error messages   */
extern char *sys_errlist[];	/* array of system message pointers          */

#ifdef MSG
#define	MF_LIBC	"libc.cat"
#define	MS_LIBC	1

#define	gd_error(x)	x < sys_nerr ? \
			NLgetamsg(MF_LIBC, MS_LIBC, x, sys_errlist[x]) : \
			NLcatgets(catd, MS_DEFS, DEFS_1, \
				"Unknown error number") /*MSG*/

#else

#define	gd_error(x)	x < sys_nerr ? sys_errlist[x] : "Unknown error number"

#endif

/*
 *	EGP stats definition
 */
struct egpstats_t {
	u_int inmsgs;
	u_int inerrors;
	u_int outmsgs;
	u_int outerrors;
};

struct egpstats_t egp_stats;

/*
 *	Definitions of descriptions of bits
 */

struct bits {
	u_int	t_bits;
	char	*t_name;
};

#ifndef XDECLARE	           	/* external only where needed */
extern struct bits flagbits[];		/* Route flag bits */
extern struct bits statebits[];		/* Route state bits */
extern struct bits intfbits[];		/* Interface flag bits */
extern struct bits protobits[];		/* Protocol types */
extern struct bits egp_states[];	/* EGP states */
extern struct bits egp_flags[];		/* EGP flag bits */
extern struct bits trace_types[];	/* Tracing types */
#endif  /* XDECLARE	 */
#ifndef XDECLARE	          	/* external only where needed */
extern struct bits snmp_types[];	/* SNMP packet types */
#endif  /* XDECLARE	 */

#define AGENT_REG	0x01
#define AGENT_REQ	0x02
#define AGENT_ERR	0x03
#define AGENT_RSP	0x04
#define	AGENT_REQN	0x05


/*
 *	Definitions of EGP bits
 */

#ifndef XDECLARE	                /* external only where needed */
extern char *egp_acq_codes[];		/* Acquisition packet types */
extern char *egp_reach_codes[];		/* Reachability codes */
extern char *egp_nr_status[];		/* Network reachability states */
extern char *egp_acq_status[];		/* Acquisition packet codes */
extern char *egp_reasons[];		/* Error code reasons */
#endif  /* XDECLARE	 */

/*
 *	SGMP and SNMP definitions
 */

extern int snmp_socket;

#define	IPPROTO_SNMP	167
#define SNMP_TRAP_PORT  162

#define RT_VAR_SIZE 4
#define RT_SIZE  5
#define EGP_VAR_SIZE 4
#define EGP_SIZE 5

#define	MIB_EGP_NEIGHBORS	0x01
#define	MIB_INTERFACES		0x02
#define	MIB_AD_ROUTE		0x03
#define	MIB_NET_ROUTE		0x04
#define	MIB_EGP_STATS		0x05
#define	MIB_ISIS_NEIGH		0x06
#define	MIB_ISIS_STATS		0x07
#define	MIB_AD_TABLE		0x08
#define	MIB_TRAP_IF_DOWN	0x09
#define	MIB_TRAP_IF_UP		0x0a
#define	MIB_TRAP_EGP_LOSS	0x0b
#define	MIB_TRAP_ISIS_LOSS	0x0c

