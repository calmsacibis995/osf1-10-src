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
static char	*sccsid = "@(#)$RCSfile: route.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/12/17 00:00:19 $";
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
 * Copyright (c) 1983, 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <sys/param.h>
#define	_SOCKADDR_LEN
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/mbuf.h>

#include <net/route.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netns/ns.h>

#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <paths.h>

struct keytab {
	char	*kt_cp;
	int	kt_i;
} keywords[] = {
#if	!OSF
/* In BSD Reno, this file is generated */
#include "keywords.h"
#else
#define	K_ADD	2
	{"add", K_ADD},
#define	K_CHANGE	3
	{"change", K_CHANGE},
#define	K_CLONING	4
	{"cloning", K_CLONING},
#define	K_DELETE	5
	{"delete", K_DELETE},
#define	K_DESTINATION	6
	{"destination", K_DESTINATION},
#define	K_EXPIRE	7
	{"expire", K_EXPIRE},
#define	K_FLUSH	8
	{"flush", K_FLUSH},
#define	K_GATEWAY	9
	{"gateway", K_GATEWAY},
#define	K_GENMASK	10
	{"genmask", K_GENMASK},
#define	K_GET	11
	{"get", K_GET},
#define	K_HOST	12
	{"host", K_HOST},
#define	K_HOPCOUNT	13
	{"hopcount", K_HOPCOUNT},
#define	K_IFACE	14
	{"iface", K_IFACE},
#define	K_INTERFACE	15
	{"interface", K_INTERFACE},
#define	K_IFADDR	16
	{"ifaddr", K_IFADDR},
#define	K_INET	17
	{"inet", K_INET},
#define	K_ISO	18
	{"iso", K_ISO},
#define	K_LINK	19
	{"link", K_LINK},
#define	K_LOCK	20
	{"lock", K_LOCK},
#define	K_LOCKREST	21
	{"lockrest", K_LOCKREST},
#define	K_MASK	22
	{"mask", K_MASK},
#define	K_MONITOR	23
	{"monitor", K_MONITOR},
#define	K_MTU	24
	{"mtu", K_MTU},
#define	K_NET	25
	{"net", K_NET},
#define	K_NETMASK	26
	{"netmask", K_NETMASK},
#define	K_OSI	27
	{"osi", K_OSI},
#define	K_RECVPIPE	28
	{"recvpipe", K_RECVPIPE},
#define	K_RTT	29
	{"rtt", K_RTT},
#define	K_RTTVAR	30
	{"rttvar", K_RTTVAR},
#define	K_SENDPIPE	31
	{"sendpipe", K_SENDPIPE},
#define	K_SSTHRESH	32
	{"ssthresh", K_SSTHRESH},
#define	K_XNS	33
	{"xns", K_XNS},
#define	K_XRESOLVE	34
	{"xresolve", K_XRESOLVE},
#endif
{0, 0}
};

struct	ortentry route;
union	sockunion {
	struct	sockaddr sa;
	struct	sockaddr_in sin;
	struct	sockaddr_ns sns;
	struct	sockaddr_dl sdl;
} so_dst, so_gate, so_mask, so_genmask, so_ifa, so_ifp, *so_addrs[] =
{ &so_dst, &so_gate, &so_mask, &so_genmask, &so_ifa, &so_ifp, 0}; 
typedef union sockunion *sup;
int	rtm_addrs = 0;
pid_t	pid;
int	s;
int	forcehost, forcenet, doflush, nflag, af = 0, qflag, Cflag, keyword();
int	iflag, verbose, aflen = sizeof (struct sockaddr_in);
int	locking, lockrest, debugonly;
struct	sockaddr_in sin = { sizeof(sin), AF_INET };
struct	rt_metrics rt_metrics;
u_int  rtm_inits;
struct	in_addr inet_makeaddr();
char	*malloc(), *routename(), *netname();
extern	char *link_ntoa();
#define kget(p, d) \
	(lseek(kmem, (off_t)(p), 0), read(kmem, (char *)&(d), sizeof (d)))

usage(cp)
char *cp;
{
	fprintf(stderr,
	    "usage: route [ -nqv ]  cmd [[ -<modifiers> ] args ]\n");
	if (cp) fprintf(stderr, "(botched keyword: %s)\n", cp);

	exit(1);
}

main(argc, argv)
	int argc;
	char *argv[];
{

	char *argvp;
	if (argc < 2)
		usage((char *)0);
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, "route: need sysadmin authorization\n");
		exit(1);
	}
	if (!forcepriv(SEC_REMOTE)) {
		fprintf(stderr, "route: insufficient privileges\n");
		exit(1);
	}
#endif
	argc--, argv++;
	for (; argc >  0 && argv[0][0] == '-'; argc--, argv++) {
		for (argvp = argv[0]++; *argvp; argvp++)
			switch (*argv[0]) {
			case 'n':
				nflag++;
				break;
			case 'q':
				qflag++;
				break;
			case 'C':
				Cflag++; /* Use old ioctls */
				break;
			case 'v':
				verbose++;
				break;
			default:
				usage((char *)0);
			}
	}
	pid = getpid();
	if (Cflag)
		s = socket(AF_INET, SOCK_RAW, 0);
	else
		s = socket(PF_ROUTE, SOCK_RAW, 0);
	if (s < 0) {
		perror("route: socket");
		exit(1);
	}
	if (argc > 0) switch (keyword(*argv)) {
		case K_ADD:
		case K_CHANGE:
		case K_DELETE:
			/* we must have >= 2 args for these commands */
			if (argc-1 < 2)
			   break;
			else
			   newroute(argc, argv);
		case K_MONITOR:
			monitor();
		case K_FLUSH:
			flushroutes(argc, argv);
	}
	usage(*argv);
}

/*
 * Purge all entries in the routing tables not
 * associated with network interfaces.
 */
#include <nlist.h>

struct nlist nl[] = {
#define	N_RTHOST	0
	{ "_rthost" },
#define	N_RTNET		1
	{ "_rtnet" },
#define N_RTHASHSIZE	2
	{ "_rthashsize" },
#define N_RTREE		3
	{ "_radix_node_head" },
	"",
};

int kmem;
char *vmunix = _PATH_UNIX;

flushroutes(argc, argv)
int argc;
char *argv[];
{
	extern int errno;
	register struct ortentry *rt;
	register struct mbuf *m;
	struct mbuf mb, **routehash;
	int rthashsize, i, doinghost = 1;

	shutdown(s, 0); /* Don't want to read back our messages */
	if (argc > 1) {
		argv++;
		if (argc == 2 && **argv == '-') switch (keyword(1 + *argv)) {
			case K_INET:	af = AF_INET;	break;
			case K_XNS:	af = AF_NS;	break;
			case K_LINK:	af = AF_LINK;	break;
			case K_ISO: case K_OSI:	af = AF_ISO; break;
			default: goto bad;
		} else
			bad: usage(*argv);
	}
	nlist(vmunix, nl);
	kmem = open(_PATH_KMEM, O_RDONLY, 0);
	if (kmem < 0) {
		(void)fprintf(stderr,
		    "route: %s: %s\n", _PATH_KMEM, strerror(errno));
		exit(1);
	}
	if (nl[N_RTREE].n_value) {
		treestuff(nl[N_RTREE].n_value);
		exit(0);
	}
	/* Else ... old kernel !?! */
	if (nl[N_RTHOST].n_value == 0) {
		fprintf(stderr,
		    "route: \"rthost\", symbol not in namelist\n");
		exit(1);
	}
	if (nl[N_RTNET].n_value == 0) {
		fprintf(stderr,
		    "route: \"rtnet\", symbol not in namelist\n");
		exit(1);
	}
	if (nl[N_RTHASHSIZE].n_value == 0) {
		fprintf(stderr,
		    "route: \"rthashsize\", symbol not in namelist\n");
		exit(1);
	}
	kget(nl[N_RTHASHSIZE].n_value, rthashsize);
	routehash = (struct mbuf **)malloc(rthashsize*sizeof (struct mbuf *));

	lseek(kmem, (off_t)nl[N_RTHOST].n_value, 0);
	read(kmem, routehash, rthashsize*sizeof (struct mbuf *));
	printf("Flushing routing tables:\n");
again:
	for (i = 0; i < rthashsize; i++) {
		if (routehash[i] == 0)
			continue;
		m = routehash[i];
		while (m) {
			kget(m, mb);
			d_ortentry((struct ortentry *)(mb.m_dat), doinghost);
			m = mb.m_next;
		}
	}
	if (doinghost) {
		lseek(kmem, (off_t)nl[N_RTNET].n_value, 0);
		read(kmem, routehash, rthashsize*sizeof (struct mbuf *));
		doinghost = 0;
		goto again;
	}
	close(kmem);
	free(routehash);
	exit(0);
}
typedef u_char	blob[128];

struct rtbatch {
	struct	rtbatch *nb;
	int	ifree;
	struct	x {
		struct	rtentry rt;
		union {
			struct sockaddr sa;
			blob data;
		} dst, gate, mask;
	} x[100];
} firstbatch, *curbatch = &firstbatch;

w_tree(rn)
struct radix_node *rn;
{

	struct radix_node rnode;
	register struct rtentry *rt;
	struct sockaddr *dst;
	register struct x *x;

	kget(rn, rnode);
	if (rnode.rn_b < 0) {
		if ((rnode.rn_flags & RNF_ROOT) == 0) {
			register struct rtbatch *b = curbatch;
			if ((rnode.rn_flags & RNF_ACTIVE) == 0) {
				printf("Dead entry in tree: %x\n", rn);
				exit(1);
			}
			if (b->ifree >= 100) {
				R_Malloc(b->nb, struct rtbatch *,
						sizeof (*b));
				if (b->nb) {
					b = b->nb;
					Bzero(b, sizeof(*b));
					curbatch = b;
				} else {
					printf("out of space\n");
					exit(1);
				}
			}
			x = b->x + b->ifree;
			rt = &x->rt;
			kget(rn, *rt);
			dst = &x->dst.sa;
			kget(rt_key(rt), *dst);
			if (dst->sa_len > sizeof (*dst))
				kget(rt_key(rt), x->dst);
			rt->rt_nodes->rn_key = (char *)dst;
			kget(rt->rt_gateway, x->gate.sa);
			if (x->gate.sa.sa_len > sizeof (*dst))
				kget(rt->rt_gateway, x->gate);
			rt->rt_gateway = &x->gate.sa;
			if (Cflag == 0) {
			    kget(rt_mask(rt), x->mask.sa);
			    if (x->mask.sa.sa_len > sizeof(x->mask.sa))
				kget(rt_mask(rt), x->mask);
			    rt->rt_nodes->rn_mask = (char *)&x->mask.sa;
			}
			b->ifree++;
		}
		if (rnode.rn_dupedkey)
			w_tree(rnode.rn_dupedkey);
	} else {
		rn = rnode.rn_r;
		w_tree(rnode.rn_l);
		w_tree(rn);
	}
}

treestuff(rtree)
off_t rtree;
{
	struct radix_node_head *rnh, head;
	register struct rtbatch *b;
	register int i;
	    
	for (kget(rtree, rnh); rnh; rnh = head.rnh_next) {
		kget(rnh, head);
		if (head.rnh_af && (af == 0 || af == head.rnh_af)) {
			w_tree(head.rnh_treetop);
		}
	}
	for (b = &firstbatch; b; b = b->nb)
		for (i = 0; i < b->ifree; i++)
			d_rtentry(&(b->x[i].rt));
}

d_ortentry(rt)
register struct ortentry *rt;
{
	int doinghost = rt->rt_flags & RTF_HOST;

	if (rt->rt_flags & RTF_GATEWAY) {
	   if (qflag == 0) {
		printf("%-20.20s ", doinghost ?
		    routename(&rt->rt_dst) :
		    netname(&rt->rt_dst));
		printf("%-20.20s ", routename(&rt->rt_gateway));
		if (ioctl(s, SIOCDELRT, (caddr_t)rt) < 0) {
			fflush(stdout);
			error("delete");
		} else
			printf("done\n");
	    } else
		(void) ioctl(s, SIOCDELRT, (caddr_t)rt);
	}
}

struct ortentry ortentry;
d_rtentry(rt)
register struct rtentry *rt;
{
	int doinghost = rt->rt_flags & RTF_HOST;
	int result;

	ortentry.rt_flags = rt->rt_flags;
	ortentry.rt_dst = so_dst.sa = *(rt_key(rt));
	ortentry.rt_gateway = so_gate.sa = *(rt->rt_gateway);
	so_mask.sa = *(rt_mask(rt));
	rtm_addrs = RTA_GATEWAY|RTA_NETMASK|RTA_DST;	/* all of 'em? */
	if (rt->rt_flags & RTF_GATEWAY) {
	       if (Cflag == 0)
			result = rtmsg('d', rt->rt_flags);
	       else
			result = ioctl(s, SIOCDELRT, (caddr_t)&ortentry);
	       if (qflag == 0) {
			printf("%-20.20s ", doinghost ?
				routename(rt_key(rt)) : netname(rt_key(rt)));
			printf("%-20.20s ", routename(rt->rt_gateway));
			if (result < 0) {
				fflush(stdout);
				error("delete");
			} else
				printf("done\n");
		}
	}
}

char *
routename(sa)
	struct sockaddr *sa;
{
	register char *cp;
	static char line[50];
	struct hostent *hp;
	static char domain[MAXHOSTNAMELEN + 1];
	static int first = 1;
	char *ns_print();

	if (first) {
		first = 0;
		if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
		    (cp = index(domain, '.')))
			(void) strcpy(domain, cp + 1);
		else
			domain[0] = 0;
	}
	switch (sa->sa_family) {

	case AF_INET:
	    {	struct in_addr in;
		in = ((struct sockaddr_in *)sa)->sin_addr;

		cp = 0;
		if (in.s_addr == INADDR_ANY)
			cp = "default";
		if (cp == 0 && !nflag) {
			hp = gethostbyaddr((char *)&in, sizeof (struct in_addr),
				AF_INET);
			if (hp) {
				if ((cp = index(hp->h_name, '.')) &&
				    !strcmp(cp + 1, domain))
					*cp = 0;
				cp = hp->h_name;
			}
		}
		if (cp)
			strcpy(line, cp);
		else {
#define C(x)	((x) & 0xff)
			in.s_addr = ntohl(in.s_addr);
			(void)sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			   C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
		}
		break;
	    }

	case AF_NS:
		return (ns_print((struct sockaddr_ns *)sa));

	case AF_LINK:
		return (link_ntoa((struct sockaddr_dl *)sa));

	default:
	    {	u_short *s = (u_short *)sa->sa_data;
		u_short *slim = s + ((sa->sa_len + 1)>>1);
		char *cp = line + sprintf(line, "(%d)", sa->sa_family);
		int n;

		while (s < slim) {
			n = sprintf(cp, " %x", *s);
			s++; cp += n;
		}
		break;
	    }
	}
	return (line);
}

/*
 * Return the name of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */
char *
netname(sa)
	struct sockaddr *sa;
{
	char *cp = 0;
	static char line[50];
	struct netent *np = 0;
	u_int net, mask;
	register u_int i;
	int subnetshift;
	char *ns_print();

	switch (sa->sa_family) {

	case AF_INET:
	    {	struct in_addr in;
		in = ((struct sockaddr_in *)sa)->sin_addr;

		i = in.s_addr = ntohl(in.s_addr);
		if (in.s_addr == 0)
			cp = "default";
		else if (!nflag) {
			if (IN_CLASSA(i)) {
				mask = IN_CLASSA_NET;
				subnetshift = 8;
			} else if (IN_CLASSB(i)) {
				mask = IN_CLASSB_NET;
				subnetshift = 8;
			} else {
				mask = IN_CLASSC_NET;
				subnetshift = 4;
			}
			/*
			 * If there are more bits than the standard mask
			 * would suggest, subnets must be in use.
			 * Guess at the subnet mask, assuming reasonable
			 * width subnet fields.
			 */
			while (in.s_addr &~ mask)
				mask = (int)mask >> subnetshift;
			net = in.s_addr & mask;
			while ((mask & 1) == 0)
				mask >>= 1, net >>= 1;
			np = getnetbyaddr(net, AF_INET);
			if (np)
				cp = np->n_name;
		}
		if (cp)
			strcpy(line, cp);
		else if ((in.s_addr & 0xffffff) == 0)
			(void)sprintf(line, "%u", C(in.s_addr >> 24));
		else if ((in.s_addr & 0xffff) == 0)
			(void)sprintf(line, "%u.%u", C(in.s_addr >> 24),
			    C(in.s_addr >> 16));
		else if ((in.s_addr & 0xff) == 0)
			(void)sprintf(line, "%u.%u.%u", C(in.s_addr >> 24),
			    C(in.s_addr >> 16), C(in.s_addr >> 8));
		else
			(void)sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			    C(in.s_addr >> 16), C(in.s_addr >> 8),
			    C(in.s_addr));
		break;
	    }

	case AF_NS:
		return (ns_print((struct sockaddr_ns *)sa));
		break;

	case AF_LINK:
		return (link_ntoa((struct sockaddr_dl *)sa));

	default:
	    {	u_short *s = (u_short *)sa->sa_data;
		u_short *slim = s + ((sa->sa_len + 1)>>1);
		char *cp = line + sprintf(line, "af %d:", sa->sa_family);
		int n;

		while (s < slim) {
			n = sprintf(cp, " %x", *s);
			s++; cp += n;
		}
		break;
	    }
	}
	return (line);
}

set_metric(value, key)
char *value;
int key;
{
	int flag = 0; 
	u_int noval, *valp = &noval;

	switch (key) {
#define caseof(x, y, z)	case x: valp = &rt_metrics.z; flag = y; break
	caseof(K_MTU, RTV_MTU, rmx_mtu);
	caseof(K_HOPCOUNT, RTV_HOPCOUNT, rmx_hopcount);
	caseof(K_EXPIRE, RTV_EXPIRE, rmx_expire);
	caseof(K_RECVPIPE, RTV_RPIPE, rmx_recvpipe);
	caseof(K_SENDPIPE, RTV_SPIPE, rmx_sendpipe);
	caseof(K_SSTHRESH, RTV_SSTHRESH, rmx_ssthresh);
	caseof(K_RTT, RTV_RTT, rmx_rtt);
	caseof(K_RTTVAR, RTV_RTTVAR, rmx_rttvar);
	}
	rtm_inits |= flag;
	if (lockrest || locking)
		rt_metrics.rmx_locks |= flag;
	if (locking)
		locking = 0;
	*valp = atoi(value);
}

newroute(argc, argv)
	int argc;
	register char **argv;
{
	struct sockaddr_in *sin;
	char *cmd, *dest, *gateway, *mask;
	int ishost, metric = 0, ret, attempts, oerrno, flags = 0, next;
	int key;
	struct hostent *hp = 0;
	extern int errno;

	shutdown(s, 0); /* Don't want to read back our messages */
	cmd = argv[0];
	while (--argc > 0) {
		if (**(++argv)== '-') {
			switch(key = keyword(1 + *argv)) {
			case K_LINK:
				af = AF_LINK;
				aflen = sizeof(struct sockaddr_dl);
				break;
			case K_OSI:
			case K_ISO:
				af = AF_ISO;
				aflen = 0/*sizeof(struct sockaddr_iso)*/;
				break;
			case K_INET:
				af = AF_INET;
				aflen = sizeof(struct sockaddr_in);
				break;
			case K_XNS:
				af = AF_NS;
				aflen = sizeof(struct sockaddr_ns);
				break;
			case K_IFACE:
			case K_INTERFACE:
				iflag++;
				break;
			case K_LOCK:
				locking = 1;
				break;
			case K_LOCKREST:
				lockrest = 1;
				break;
			case K_HOST:
				forcehost++;
				break;
			case K_NETMASK:
				argc--;
				(void) getaddr(RTA_NETMASK, *++argv, 0);
				/* FALLTHROUGH */
			case K_NET:
				forcenet++;
				break;
			case K_CLONING:
				flags |= RTF_CLONING;
				break;
			case K_XRESOLVE:
				flags |= RTF_XRESOLVE;
				break;
			case K_GENMASK:
				argc--;
				(void) getaddr(RTA_GENMASK, *++argv, 0);
				break;
			case K_MTU:
			case K_HOPCOUNT:
			case K_EXPIRE:
			case K_RECVPIPE:
			case K_SENDPIPE:
			case K_SSTHRESH:
			case K_RTT:
			case K_RTTVAR:
				argc--;
				set_metric(*++argv, key);
				break;
			default:
				usage(1+*argv);
			}
		} else {
			if ((rtm_addrs & RTA_DST) == 0) {
				dest = *argv;
				ishost = getaddr(RTA_DST, *argv, &hp);
			} else if ((rtm_addrs & RTA_GATEWAY) == 0) {
				gateway = *argv;
				(void) getaddr(RTA_GATEWAY, *argv, &hp);
			} else {
				int ret = atoi(*argv);
				if (ret == 0) {
				    printf("%s,%s", "old usage of trailing 0",
					   "assuming route to if\n");
				    iflag = 1;
				    continue;
				} else if (ret > 0 ) {
				    printf("old usage of trailing digit, ");
				    printf("assuming route via gateway\n");
				    iflag = 0;
				    continue;
				}
				/* use -netmask option to specify netmask */
				/*(void) getaddr(RTA_NETMASK, *argv, 0);*/
			}
		}
	}
	if (forcehost)
		ishost = 1;
	if (forcenet)
		ishost = 0;
	flags |= RTF_UP;
	if (ishost)
		flags |= RTF_HOST;
	if (iflag == 0)
		flags |= RTF_GATEWAY;
	for (attempts = 1; ; attempts++) {
		errno = 0;
		if (Cflag && (af == AF_INET || af == AF_NS)) {
			route.rt_flags = flags;
			route.rt_dst = so_dst.sa;
			route.rt_gateway = so_gate.sa;
			if ((ret = ioctl(s, *cmd == 'a' ? SIOCADDRT : SIOCDELRT,
			     (caddr_t)&route)) == 0)
				break;
		} else {
		    if ((ret = rtmsg(*cmd, flags)) == 0) /* BSD had ";" ??? */
				break;
		}
		if (errno != ENETUNREACH && errno != ESRCH)
			break;
		if (hp && hp->h_addr_list[1]) {
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (caddr_t)&so_dst.sin.sin_addr,
			    hp->h_length);
		} else
			break;
	}
	oerrno = errno;
	printf("%s %s %s: gateway %s", cmd, ishost? "host" : "net",
		dest, gateway);
	if (attempts > 1 && ret == 0)
	    printf(" (%s)",
		inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
	if (ret == 0)
		printf("\n");
	else {
		printf(": ");
		fflush(stdout);
		errno = oerrno;
		error("");
	}
	exit(0);
}

error(cmd)
	char *cmd;
{
	extern int errno;

	switch(errno) {
	case ESRCH:
		printf("not in table\n");
		break;
	case EBUSY:
		printf("entry in use\n");
		break;
	case ENOBUFS:
		printf("routing table overflow\n");
		break;
	default:
		printf("ioctl returns %d\n", errno);
		perror(cmd);
	}
	fflush(stdout);
	errno = 0;
}

char *
savestr(s)
	char *s;
{
	char *sav;

	sav = malloc(strlen(s) + 1);
	if (sav == NULL) {
		fprintf(stderr, "route: out of memory\n");
		exit(1);
	}
	strcpy(sav, s);
	return (sav);
}

inet_makenetandmask(net, sin)
u_int net;
register struct sockaddr_in *sin;
{
	u_int addr;
	u_int mask = 0;
	register char *cp;

	rtm_addrs |= RTA_NETMASK;
	if (net == 0)
		mask = addr = 0;
	else if (net < 128) {
		addr = net << IN_CLASSA_NSHIFT;
		mask = IN_CLASSA_NET;
	} else if (net < 65536) {
		addr = net << IN_CLASSB_NSHIFT;
		mask = IN_CLASSB_NET;
	} else if (net < 16777216L) {
		addr = net << IN_CLASSC_NSHIFT;
		mask = IN_CLASSC_NET;
	} else {
		addr = net;
		if ((addr & IN_CLASSA_HOST) == 0)
			mask =  IN_CLASSA_NET;
		else if ((addr & IN_CLASSB_HOST) == 0)
			mask =  IN_CLASSB_NET;
		else if ((addr & IN_CLASSC_HOST) == 0)
			mask =  IN_CLASSC_NET;
		else
			mask = -1;
	}
	sin->sin_addr.s_addr = htonl(addr);
	sin = &so_mask.sin;
	sin->sin_addr.s_addr = htonl(mask);
	sin->sin_len = 0;
	sin->sin_family = af;
	cp = (char *)(1 + &(sin->sin_addr));
	while (*--cp == 0 && cp > (char *)sin)
		;
	sin->sin_len = 1 + cp - (char *)sin;
}

/*
 * Interpret an argument as a network address of some kind,
 * returning 1 if a host address, 0 if a network address.
 */
getaddr(which, s, hpp)
	int which;
	char *s;
	struct hostent **hpp;
{
	register union sockunion *su;
	struct	ns_addr ns_addr();
	struct hostent *hp;
	struct netent *np;
	u_int val;

	if (af == 0) {
		af = AF_INET;
		aflen = sizeof(struct sockaddr_in);
	}
	rtm_addrs |= which;
	switch (which) {
	case RTA_DST:		su = so_addrs[0]; su->sa.sa_family = af; break;
	case RTA_GATEWAY:	su = so_addrs[1]; su->sa.sa_family = af; break;
	case RTA_NETMASK:	su = so_addrs[2]; su->sa.sa_family = af; break;
	case RTA_GENMASK:	su = so_addrs[3]; break;
	default:		usage("Internal Error"); /*NOTREACHED*/
	}
	su->sa.sa_len = aflen;
	if (strcmp(s, "default") == 0) {
		switch (which) {
		case RTA_DST:
			forcenet++;
			getaddr(RTA_NETMASK, s, 0);
			break;
		case RTA_NETMASK:
		case RTA_GENMASK:
			su->sa.sa_len = 0;
		}
		return 0;
	}
	if (af == AF_NS)
		goto do_xns;
	if (af == AF_OSI)
		goto do_osi;
	if (af == AF_LINK)
		goto do_link;
	if (hpp == 0) hpp = &hp;
	*hpp = 0;
	if (((val = inet_addr(s)) != -1) &&
	    (which != RTA_DST || forcenet == 0)) {
		su->sin.sin_addr.s_addr = val;
		if (inet_lnaof(su->sin.sin_addr) != INADDR_ANY)
			return (1);
		else {
			val = ntohl(val);
		out:	if (which == RTA_DST)
				inet_makenetandmask(val, &su->sin);
			return (0);
		}
	}
	val = inet_network(s);
	if (val != -1) {
		goto out;
	}
	np = getnetbyname(s);
	if (np) {
		val = np->n_net;
		goto out;
	}
	hp = gethostbyname(s);
	if (hp) {
		*hpp = hp;
		su->sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr, &su->sin.sin_addr, hp->h_length);
		return (1);
	}
	fprintf(stderr, "%s: bad value\n", s);
	exit(1);
do_xns:
	if (val == 0)
		return(0);
	if (which == RTA_DST) {
		extern short ns_bh[3];
		struct sockaddr_ns *sms = &(so_mask.sns);
		bzero((char *)sms, sizeof(*sms));
		sms->sns_family = 0;
		sms->sns_len = 6;
		sms->sns_addr.x_net = *(union ns_net *)ns_bh;
		rtm_addrs |= RTA_NETMASK;
	}
	su->sns.sns_addr = ns_addr(s);
	return (!ns_nullhost(su->sns.sns_addr));
do_osi:
	return (0);
do_link:
	link_addr(s, &su->sdl);
	return (1);
}

short ns_nullh[] = {0,0,0};
short ns_bh[] = {-1,-1,-1};

char *
ns_print(sns)
struct sockaddr_ns *sns;
{
	struct ns_addr work;
	union { union ns_net net_e; u_int long_e; } net;
	u_short port;
	static char mybuf[50], cport[10], chost[25];
	char *host = "";
	register char *p; register u_char *q; u_char *q_lim;

	work = sns->sns_addr;
	port = ntohs(work.x_port);
	work.x_port = 0;
	net.net_e  = work.x_net;
	if (ns_nullhost(work) && net.long_e == 0) {
		if (port ) {
			(void)sprintf(mybuf, "*.%xH", port);
			upHex(mybuf);
		} else
			(void)sprintf(mybuf, "*.*");
		return (mybuf);
	}

	if (bcmp(ns_bh, work.x_host.c_host, 6) == 0) { 
		host = "any";
	} else if (bcmp(ns_nullh, work.x_host.c_host, 6) == 0) {
		host = "*";
	} else {
		q = work.x_host.c_host;
		(void)sprintf(chost, "%02x%02x%02x%02x%02x%02xH",
			q[0], q[1], q[2], q[3], q[4], q[5]);
		for (p = chost; *p == '0' && p < chost + 12; p++);
		host = p;
	}
	if (port)
		(void)sprintf(cport, ".%xH", htons(port));
	else
		*cport = 0;

	(void)sprintf(mybuf,"%xH.%s%s", ntohl(net.long_e), host, cport);
	upHex(mybuf);
	return(mybuf);
}

upHex(p0)
char *p0;
{
	register char *p = p0;
	for (; *p; p++) switch (*p) {

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		*p += ('A' - 'a');
	}
}

monitor()
{
	int n;
	char msg[2048];
	verbose = 1;
	for(;;) {
		n = read(s, msg, 2048);
		printf("got message of size %d\n", n);
		print_rtmsg((struct rt_msghdr *)msg);
	}
}

struct {
	struct	rt_msghdr m_rtm;
	char m_space[512];
} m_rtmsg;

rtmsg(cmd, flags)
{
	static int seq;
	int rlen;
	extern int errno;
	register char *cp = m_rtmsg.m_space;
	register int l;

	errno = 0;
	bzero((char *)&m_rtmsg, sizeof(m_rtmsg));
	if (cmd == 'a')
		cmd = RTM_ADD;
	else if (cmd == 'c')
		cmd = RTM_CHANGE;
	else
		cmd = RTM_DELETE;
	m_rtmsg.m_rtm.rtm_flags = flags;
	m_rtmsg.m_rtm.rtm_version = RTM_VERSION;
	m_rtmsg.m_rtm.rtm_seq = ++seq;
	m_rtmsg.m_rtm.rtm_addrs = rtm_addrs;
	m_rtmsg.m_rtm.rtm_rmx = rt_metrics;
	m_rtmsg.m_rtm.rtm_inits = rtm_inits;

#define ROUND(a) (1 + (((a) - 1) | (sizeof(long) - 1)))
#define NEXTADDR(w, u) { if (rtm_addrs & (w)) {l = (u).sa.sa_len;\
	if(verbose)sodump(&(u),"u");if(l == 0) l = sizeof(long); l = ROUND(l);\
		bcopy((char *)&(u), cp, l); cp += l;}}

	NEXTADDR(RTA_DST, so_dst);
	NEXTADDR(RTA_GATEWAY, so_gate);
	NEXTADDR(RTA_NETMASK, so_mask);
	NEXTADDR(RTA_GENMASK, so_genmask);
	m_rtmsg.m_rtm.rtm_msglen = l = cp - (char *)&m_rtmsg;
	m_rtmsg.m_rtm.rtm_type = cmd;
	if (verbose)
		print_rtmsg(&m_rtmsg.m_rtm, l);
	if (debugonly)
		return 0;
	if ((rlen = write(s, (char *)&m_rtmsg, l)) != l) {
		perror("writing to routing socket");
		if (rlen >= 0) printf("got only %d for rlen\n", rlen);
		return (-1);
	}
	return (0);
}

char *msgtypes[] = {
"",
"RTM_ADD: Add Route",
"RTM_DELETE: Delete Route",
"RTM_CHANGE: Change Metrics or flags",
"RTM_GET: Report Metrics",
"RTM_LOSING: Kernel Suspects Partitioning",
"RTM_REDIRECT: Told to use different route",
"RTM_MISS: Lookup failed on this address",
"RTM_LOCK: fix specified metrics",
"RTM_OLDADD: caused by SIOCADDRT",
"RTM_OLDDEL: caused by SIOCDELRT",
0, };

char metricnames[] =
"\010rttvar\7rtt\6ssthresh\5sendpipe\4recvpipe\3expire\2hopcount\1mtu";
char routeflags[] = 
"\1UP\2GATEWAY\3HOST\5DYNAMIC\6MODIFIED\7DONE\010MASK_PRESENT\011CLONING\012XRESOLVE";

#define ROUNDUP(a) ((char *)(1 + (((((long)a)) - 1) | (sizeof(long) - 1))))

print_rtmsg(rtm, n)
register struct rt_msghdr *rtm;
int n;
{
	char *cp;
	register struct sockaddr *sa;
	int i;

	if (verbose == 0)
		return;
	if (rtm->rtm_version != RTM_VERSION) {
	    printf("routing message version %d not understood\n",
							rtm->rtm_version);
	} else {
	    printf("%s\npid: %d, len %d, seq %d, errno %d, flags:",
		    msgtypes[rtm->rtm_type], rtm->rtm_pid, rtm->rtm_msglen,
		    rtm->rtm_seq, rtm->rtm_errno); 
	    bprintf(stdout, rtm->rtm_flags, routeflags);
	    printf("\nlocks: "); bprintf(stdout, rtm->rtm_rmx.rmx_locks, metricnames);
	    printf(" inits: "); bprintf(stdout, rtm->rtm_inits, metricnames);
	    printf("\nsockaddrs: ");
	    bprintf(stdout, rtm->rtm_addrs,
		"\1DST\2GATEWAY\3NETMASK\4GENMASK\5IFP\6IFA\7AUTHOR");
	    putchar('\n');
	    cp = ((char *)(rtm + 1));
	    if (rtm->rtm_addrs)
		for (i = 1; i; i <<= 1)
		    if (i & rtm->rtm_addrs) {
			    sa = (struct sockaddr *)cp;
			    printf(" %s", routename(sa));
			    cp = ROUNDUP(cp + sa->sa_len);
		    }
	    putchar('\n');
	}
	fflush(stdout);
}

bprintf(fp, b, s)
register FILE *fp;
register int b;
register u_char *s;
{
	register int i;
	int gotsome = 0;

	if (b == 0)
		return;
	while (i = *s++) {
		if (b & (1 << (i-1))) {
			if (gotsome == 0) i = '<'; else i = ',';
			putc(i, fp);
			gotsome = 1;
			for (; (i = *s) > 32; s++)
				putc(i, fp);
		} else
			while (*s > 32)
				s++;
	}
	if (gotsome)
		putc('>', fp);
}
int
keyword(cp)
char *cp;
{
	register struct keytab *kt = keywords;
	while (kt->kt_cp && strcmp(kt->kt_cp, cp))
		kt++;
	return kt->kt_i;
}

sodump(su, which)
register union sockunion *su;
char *which;
{
	switch (su->sa.sa_family) {
	case AF_LINK:
		printf("%s: link %s; ", which, link_ntoa(&su->sdl));
		break;
	case AF_INET:
		printf("%s: inet %s; ", which, inet_ntoa(su->sin.sin_addr));
		break;
	case AF_NS:
		printf("%s: xns %s; ", which, ns_ntoa(su->sns.sns_addr));
		break;
	}
	fflush(stdout);
}

/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if	0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ns_ntoa.c	6.5 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <netns/ns.h>
#endif

static char *spectHex();

char *
ns_ntoa(addr)
struct ns_addr addr;
{
	static char obuf[40];
	char *spectHex();
	union { union ns_net net_e; u_int long_e; } net;
	u_short port = htons(addr.x_port);
	register char *cp;
	char *cp2;
	register u_char *up = addr.x_host.c_host;
	u_char *uplim = up + 6;

	net.net_e = addr.x_net;
	sprintf(obuf, "%lx", ntohl(net.long_e));
	cp = spectHex(obuf);
	cp2 = cp + 1;
	while (*up==0 && up < uplim) up++;
	if (up == uplim) {
		if (port) {
			sprintf(cp, ".0");
			cp += 2;
		}
	} else {
		sprintf(cp, ".%x", *up++);
		while (up < uplim) {
			while (*cp) cp++;
			sprintf(cp, "%02x", *up++);
		}
		cp = spectHex(cp2);
	}
	if (port) {
		sprintf(cp, ".%x", port);
		spectHex(cp + 1);
	}
	return (obuf);
}

static char *
spectHex(p0)
char *p0;
{
	int ok = 0;
	int nonzero = 0;
	register char *p = p0;
	for (; *p; p++) switch (*p) {

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		*p += ('A' - 'a');
		/* fall into . . . */
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		ok = 1;
	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
		nonzero = 1;
	}
	if (nonzero && !ok) { *p++ = 'H'; *p = 0; }
	return (p);
}

/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * J.Q. Johnson.
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

#if	0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ns_addr.c	6.6 (Berkeley) 6/6/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <netns/ns.h>
#endif

static struct ns_addr addr, zero_addr;
static int Field(), cvtbase();

struct ns_addr 
ns_addr(name)
#ifdef	__STDC__
	const char *name;
#else
	char *name;
#endif
{
	char separator = '.';
	char *hostname, *socketname, *cp;
	char buf[50];

	addr = zero_addr;
	(void)strncpy(buf, name, 49);

	/*
	 * First, figure out what he intends as a field separtor.
	 * Despite the way this routine is written, the prefered
	 * form  2-272.AA001234H.01777, i.e. XDE standard.
	 * Great efforts are made to insure backward compatability.
	 */
	if (hostname = index(buf, '#'))
		separator = '#';
	else {
		hostname = index(buf, '.');
		if ((cp = index(buf, ':')) &&
		    ( (hostname && cp < hostname) || (hostname == 0))) {
			hostname = cp;
			separator = ':';
		}
	}
	if (hostname)
		*hostname++ = 0;
	Field(buf, addr.x_net.c_net, 4);
	if (hostname == 0)
		return (addr);  /* No separator means net only */

	socketname = index(hostname, separator);
	if (socketname) {
		*socketname++ = 0;
		Field(socketname, (u_char *)&addr.x_port, 2);
	}

	Field(hostname, addr.x_host.c_host, 6);

	return (addr);
}

static
Field(buf, out, len)
char *buf;
u_char *out;
int len;
{
	register char *bp = buf;
	int i, ibase, base16 = 0, base10 = 0, clen = 0;
	int hb[6], *hp;
	char *fmt;

	/*
	 * first try 2-273#2-852-151-014#socket
	 */
	if ((*buf != '-') &&
	    (1 < (i = sscanf(buf, "%d-%d-%d-%d-%d",
			&hb[0], &hb[1], &hb[2], &hb[3], &hb[4])))) {
		cvtbase(1000L, 256, hb, i, out, len);
		return;
	}
	/*
	 * try form 8E1#0.0.AA.0.5E.E6#socket
	 */
	if (1 < (i = sscanf(buf,"%x.%x.%x.%x.%x.%x",
			&hb[0], &hb[1], &hb[2], &hb[3], &hb[4], &hb[5]))) {
		cvtbase(256L, 256, hb, i, out, len);
		return;
	}
	/*
	 * try form 8E1#0:0:AA:0:5E:E6#socket
	 */
	if (1 < (i = sscanf(buf,"%x:%x:%x:%x:%x:%x",
			&hb[0], &hb[1], &hb[2], &hb[3], &hb[4], &hb[5]))) {
		cvtbase(256L, 256, hb, i, out, len);
		return;
	}
	/*
	 * This is REALLY stretching it but there was a
	 * comma notation separting shorts -- definitely non standard
	 */
	if (1 < (i = sscanf(buf,"%x,%x,%x",
			&hb[0], &hb[1], &hb[2]))) {
		hb[0] = htons(hb[0]); hb[1] = htons(hb[1]);
		hb[2] = htons(hb[2]);
		cvtbase(65536L, 256, hb, i, out, len);
		return;
	}

	/* Need to decide if base 10, 16 or 8 */
	while (*bp) switch (*bp++) {

	case '0': case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '-':
		break;

	case '8': case '9':
		base10 = 1;
		break;

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		base16 = 1;
		break;
	
	case 'x': case 'X':
		*--bp = '0';
		base16 = 1;
		break;

	case 'h': case 'H':
		base16 = 1;
		/* fall into */

	default:
		*--bp = 0; /* Ends Loop */
	}
	if (base16) {
		fmt = "%3x";
		ibase = 4096;
	} else if (base10 == 0 && *buf == '0') {
		fmt = "%3o";
		ibase = 512;
	} else {
		fmt = "%3d";
		ibase = 1000;
	}

	for (bp = buf; *bp++; ) clen++;
	if (clen == 0) clen++;
	if (clen > 18) clen = 18;
	i = ((clen - 1) / 3) + 1;
	bp = clen + buf - 3;
	hp = hb + i - 1;

	while (hp > hb) {
		(void)sscanf(bp, fmt, hp);
		bp[0] = 0;
		hp--;
		bp -= 3;
	}
	(void)sscanf(buf, fmt, hp);
	cvtbase((int)ibase, 256, hb, i, out, len);
}

static
cvtbase(oldbase,newbase,input,inlen,result,reslen)
	int oldbase;
	int newbase;
	int input[];
	int inlen;
	unsigned char result[];
	int reslen;
{
	int d, e;
	long sum;

	e = 1;
	while (e > 0 && reslen > 0) {
		d = 0; e = 0; sum = 0;
		/* long division: input=input/newbase */
		while (d < inlen) {
			sum = sum*oldbase + (int) input[d];
			e += (sum > 0);
			input[d++] = sum / newbase;
			sum %= newbase;
		}
		result[--reslen] = sum;	/* accumulate remainder */
	}
	for (d=0; d < reslen; d++)
		result[d] = 0;
}

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if	0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)linkaddr.c	5.1 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#endif

/* States*/
#define NAMING	0
#define GOTONE	1
#define GOTTWO	2
#define RESET	3
/* Inputs */
#define	DIGIT	(4*0)
#define	END	(4*1)
#define DELIM	(4*2)
#define LETTER	(4*3)

link_addr(addr, sdl)
register char *addr;
register struct sockaddr_dl *sdl;
{
	register char *cp = sdl->sdl_data;
	char *cplim = sdl->sdl_len + (char *)sdl;
	register int byte = 0, state = NAMING, new;

	bzero((char *)&sdl->sdl_family, sdl->sdl_len - 1);
	sdl->sdl_family = AF_LINK;
	do {
		state &= ~LETTER;
		if ((*addr >= '0') && (*addr <= '9')) {
			new = *addr - '0';
		} else if ((*addr >= 'a') && (*addr <= 'f')) {
			new = *addr - 'a' + 10;
		} else if ((*addr >= 'A') && (*addr <= 'F')) {
			new = *addr - 'A' + 10;
		} else if (*addr == 0) {
			state |= END;
		} else if (state == NAMING &&
			   (((*addr >= 'A') && (*addr <= 'Z')) ||
			   ((*addr >= 'a') && (*addr <= 'z'))))
			state |= LETTER;
		else
			state |= DELIM;
		addr++;
		switch (state /* | INPUT */) {
		case NAMING | DIGIT:
		case NAMING | LETTER:
			*cp++ = addr[-1]; continue;
		case NAMING | DELIM:
			state = RESET; sdl->sdl_nlen = cp - sdl->sdl_data; continue;
		case GOTTWO | DIGIT:
			*cp++ = byte; /*FALLTHROUGH*/
		case RESET | DIGIT:
			state = GOTONE; byte = new; continue;
		case GOTONE | DIGIT:
			state = GOTTWO; byte = new + (byte << 4); continue;
		default: /* | DELIM */
			state = RESET; *cp++ = byte; byte = 0; continue;
		case GOTONE | END:
		case GOTTWO | END:
			*cp++ = byte; /* FALLTHROUGH */
		case RESET | END:
			break;
		}
		break;
	} while (cp < cplim); 
	sdl->sdl_alen = cp - LLADDR(sdl);
	new = cp - (char *)sdl;
	if (new > sizeof(*sdl))
		sdl->sdl_len = new;
	return;
}

static char hexlist[] = "0123456789abcdef";

char *
link_ntoa(sdl)
register struct sockaddr_dl *sdl;
{
	static char obuf[64];
	register char *out = obuf; 
	register int i;
	register u_char *in = (u_char *)LLADDR(sdl);
	u_char *inlim = in + sdl->sdl_nlen;
	int firsttime = 1;

	if (sdl->sdl_nlen) {
		bcopy(sdl->sdl_data, obuf, sdl->sdl_nlen);
		out += sdl->sdl_nlen;
		*out++ = ':';
	}
	while (in < inlim) {
		if (firsttime) firsttime = 0; else *out++ = '.';
		i = *in++;
		if (i > 0xf) {
			out[1] = hexlist[i & 0xf];
			i >>= 4;
			out[0] = hexlist[i];
			out += 2;
		} else
			*out++ = hexlist[i];
	}
	*out = 0;
	return(obuf);
}
