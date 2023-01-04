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
static char *rcsid = "@(#)$RCSfile: sm_svc.c,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/12/21 20:49:31 $";
#endif
#ifndef lint
static char sccsid[] = 	"@(#)sm_svc.c	1.2 90/07/23 4.1NFSSRC Copyr 1988 Sun Micro";
#endif

	/*
	 * Copyright (c) 1989,1990 by Sun Microsystems, Inc.
	 */

#include <stdio.h>
#include <signal.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <net/if.h>
#include <rpcsvc/sm_inter.h>
#include "sm_statd.h"

#define current	        "sm"
#define backup		"sm.bak"
#define state		"state"

extern crash_notice(), recovery_notice(), sm_try();
extern char *strcpy();



struct bindhostname_entry {
	struct bindhostname_entry *next;
	struct sockaddr_in addr;
};

struct bindhostname_entry *bindhostname_q = NULL;
char STATE[MAXHOSTNAMELEN], CURRENT[MAXHOSTNAMELEN], BACKUP[MAXHOSTNAMELEN];
int debug;
char *bindhostname = NULL;

static void
get_bindhostname(name)
	char *name;
{
	int i,sd;
	struct bindhostname_entry *entry;
	struct bindhostname_entry *entry_tmp;
	struct hostent *hp;
	struct ifconf ifc;
	struct ifreq ifreq,*ifr;
	int duplicate_flag;

	while (bindhostname_q != NULL) {
		entry = bindhostname_q;
		bindhostname_q = bindhostname_q->next;
		free(entry);
	}
		
	if (name != NULL) {
		if ((hp=gethostbyname(name)) == NULL) {
			fprintf(stderr,"rpc.statd: %s is an unknown hostname\n",name);
			exit(1);
		}
		bindhostname_q = (struct bindhostname_entry *)
			malloc(sizeof(struct bindhostname_entry));
		if (bindhostname_q == NULL) {
			fprintf(stderr,"rpc.statd: get_bindhostname() malloc error\n");
			exit(1);
		}
		bzero((char *)&bindhostname_q->addr, sizeof(bindhostname_q->addr));
		bcopy(hp->h_addr, (char *)&bindhostname_q->addr.sin_addr.s_addr, hp->h_length);
		bindhostname_q->addr.sin_family = hp->h_addrtype;
		bindhostname_q->next = NULL;
		return;
	}


	ifc.ifc_len = UDPMSGSIZE;
	ifc.ifc_buf = (char *) malloc(ifc.ifc_len);
	
	if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("rpc.statd: get_bindhostname() socket error\n");
		exit(1);
	}

	if (ioctl(sd, SIOCGIFCONF, (char *) &ifc) < 0) {
		perror("rpc.statd: getbindhost() ioctl SIOCGIFCONF");
		exit(1);
	}

	ifr = ifc.ifc_req;
	for (i=0; i<(ifc.ifc_len / sizeof (struct ifreq)); i++,ifr++) {
		ifreq = *ifr;
		if (ioctl(sd, SIOCGIFFLAGS, (char *) &ifreq) < 0) {
			perror("rpc.statd: getbindhost() ioctl SIOCGIFFLAGS");
			exit(1);
		}
		if ((ifreq.ifr_flags & IFF_UP) && ifr->ifr_addr.sa_family == AF_INET) {
			if (ioctl(sd, SIOCGIFADDR, (char *) &ifreq) < 0) {
				perror("rpc.statd: getbindhost() ioctl SIOCGIFADDR");
				exit(1);
			} else {
				if (bindhostname_q != NULL) {
					duplicate_flag = 0;
					for (entry_tmp = bindhostname_q; entry_tmp != NULL; entry_tmp=entry_tmp->next) {
						if (bcmp(&ifreq.ifr_addr,&entry_tmp->addr,sizeof(struct sockaddr_in)) == 0) {
							duplicate_flag = 1;
							break;
						}
					}
					if (duplicate_flag) {
						continue;
					}
					entry->next = (struct bindhostname_entry *)
						malloc(sizeof(struct bindhostname_entry));
					if (entry->next == NULL) {
						fprintf(stderr,"rpc.statd: get_bindhostname() malloc error\n");
						exit(1);
					}
					entry = entry->next;
				} else {
					bindhostname_q = (struct bindhostname_entry *)
						malloc(sizeof(struct bindhostname_entry));
					if (bindhostname_q == NULL) {
						fprintf(stderr,"rpc.statd: get_bindhostname() malloc error\n");
						exit(1);
					}
					entry = bindhostname_q;
				}
				bcopy(&ifreq.ifr_addr,&entry->addr,sizeof(struct sockaddr_in));
				entry->next = NULL;
			}
		}
	}
	(void) close(sd);
	free(ifc.ifc_buf);
}

static void
sm_prog_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		struct sm_name sm_stat_1_arg;
		struct mon sm_mon_1_arg;
		struct mon_id sm_unmon_1_arg;
		struct my_id sm_unmon_all_1_arg;
		struct stat_chge ntf_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();
	extern struct sm_stat_res *sm_stat_1();
	extern struct sm_stat_res *sm_mon_1();
	extern struct sm_stat *sm_unmon_1();
	extern struct sm_stat *sm_unmon_all_1();
	extern void *sm_simu_crash_1();
	extern void *sm_notify();
	extern bool_t xdr_notify();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(transp, xdr_void, NULL);
		return;

	case SM_STAT:
		xdr_argument = xdr_sm_name;
		xdr_result = xdr_sm_stat_res;
		local = (char *(*)()) sm_stat_1;
		break;

	case SM_MON:
		xdr_argument = xdr_mon;
		xdr_result = xdr_sm_stat_res;
		local = (char *(*)()) sm_mon_1;
		break;

	case SM_UNMON:
		xdr_argument = xdr_mon_id;
		xdr_result = xdr_sm_stat;
		local = (char *(*)()) sm_unmon_1;
		break;

	case SM_UNMON_ALL:
		xdr_argument = xdr_my_id;
		xdr_result = xdr_sm_stat;
		local = (char *(*)()) sm_unmon_all_1;
		break;

	case SM_SIMU_CRASH:
		xdr_argument = xdr_void;
		xdr_result = xdr_void;
		local = (char *(*)()) sm_simu_crash_1;
		break;

	case SM_NOTIFY:
		xdr_argument = xdr_notify;
		xdr_result = xdr_void;
		local = (char *(*)()) sm_notify;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	bzero(&argument, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument);
	if (!svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (rqstp->rq_proc != SM_MON)
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		fprintf(stderr,"rpc.statd: unable to free arguments\n");
		exit(1);
	}
}

main(argc, argv)
	int argc;
	char **argv;
{
	SVCXPRT *transp;
	int t;
	int c;
	int ppid;
	extern int optind;
	extern char *optarg;
	char pathname[NAME_MAX]="/etc/";
	struct bindhostname_entry *bhn_entry;
	struct hostent *hp;
	int sd;
	struct sockaddr_in addr;
	u_short pmap_rv;
	char hostname[MAXHOSTNAMELEN];
	struct servent *servbuf;
	char filePath[MAXPATHLEN];
	FILE *runFile;
	pid_t pid;

	(void) signal(SIGALRM, sm_try);
	while ((c = getopt(argc, argv, "b:p:Dd:")) != EOF)
		switch(c) {
		case 'b':
			bindhostname = optarg;
			break;
		case 'p':
			(void) sscanf(optarg, "%s", pathname);
			(void) strcat(pathname,"/");
			break;
		case 'D':
			(void) strcpy(pathname,"");
			break;
		case 'd':
			(void) sscanf(optarg, "%d", &debug);
			break;
		default:
			fprintf(stderr, "rpc.statd -b[hostname] -p[pathname] -D -d[debug]\n");
			return (1);
		}

	(void) get_bindhostname(bindhostname);

	(void) sprintf(CURRENT,"%s%s",pathname,current);
	(void) sprintf(BACKUP,"%s%s",pathname,backup);
	(void) sprintf(STATE,"%s%s",pathname,state);
	
	if (debug) {
		printf("debug on\n");
		printf("\n");
		printf("CURRENT=%s\n",CURRENT);
		printf("BACKUP =%s\n",BACKUP);
		printf("STATE  =%s\n",STATE);
	}

	if (!debug) {
		ppid = fork();
		if (ppid == -1) {
			(void) fprintf(stderr, "rpc.statd: fork failure\n");
			(void) fflush(stderr);
			abort();
		}
		if (ppid != 0) {
			exit(0);
		}
		for (t = 0; t< 20; t++) {
			(void) close(t);
		}

		(void) open("/dev/console", 2);
		(void) open("/dev/console", 2);
		(void) open("/dev/console", 2);
		(void) setpgrp(0, 0);
	}
	else {
		setlinebuf(stderr);
		setlinebuf(stdout);
	}

	gethostname(hostname, MAXHOSTNAMELEN);
	hp = gethostbyname(hostname);
	bzero((char *)&addr, sizeof(addr));
	bcopy(hp->h_addr, (char *)&addr.sin_addr.s_addr,hp->h_length);
	addr.sin_family = hp->h_addrtype;

	for (bhn_entry = bindhostname_q; bhn_entry != NULL; bhn_entry=bhn_entry->next) {

		if (debug) {
			hp = gethostbyaddr((char *)&bhn_entry->addr.sin_addr,sizeof(struct in_addr),AF_INET);
			printf("\n");
			printf("binding %s (%s)\n",hp->h_name,inet_ntoa(bhn_entry->addr.sin_addr));
		}


		/*
		 * bind and register UDP
		 */

		if ((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror("rpc.statd socket (udp)");
			exit(1);
		}
		bhn_entry->addr.sin_port = 
			htons(pmap_getport(&addr,
					   SM_PROG,
					   SM_VERS,
					   IPPROTO_UDP));
		if (bind(sd, &bhn_entry->addr, sizeof(bhn_entry->addr))) {
			perror("rpc.statd bind (udp)");
			exit(1);
		}
		transp = svcudp_create(sd);
		if (transp == NULL) {
			fprintf(stderr,"rpc.statd: cannot create udp service\n");
			exit(1);
		}
		if (!svc_register(transp, SM_PROG, SM_VERS, sm_prog_1, IPPROTO_UDP)) {
			fprintf(stderr,"rpc.statd: svc_register error for %d, %d (udp)\n",
				SM_PROG,
				SM_VERS);
			exit(1);
		}


		/*
		 * bind and register TCP
		 */

		if ((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			perror("rpc.statd socket (tcp)");
			exit(1);
		}
		bhn_entry->addr.sin_port = 
			htons(pmap_getport(&addr,
					   SM_PROG,
					   SM_VERS, 
					   IPPROTO_TCP));
		if (bind(sd, &bhn_entry->addr, sizeof(bhn_entry->addr))) {
			perror("rpc.statd bind (tcp)");
			exit(1);
		}
		transp = svctcp_create(sd, 0, 0);
		if (transp == NULL) {
			fprintf(stderr,"rpc.statd: cannot create tcp service\n");
			exit(1);
		}
		if (!svc_register(transp, SM_PROG, SM_VERS, sm_prog_1, IPPROTO_TCP)) {
			fprintf(stderr,"rpc.statd: svc_register error for %d, %d (tcp)\n",
				SM_PROG,
				SM_VERS);
			exit(1);
		}
	}

	if (debug) {
		printf("\n");
	}
	statd_init();

	/*
	 * Place entry in /var/run
	 */
	pid = getpid();
	if (bindhostname) {
		sprintf(filePath, "/var/run/rpc.statd.%s.pid", bindhostname);
	} else {
		sprintf(filePath, "/var/run/rpc.statd.pid");
	}
	if ((runFile = fopen(filePath, "w")) == 0) {
		fprintf(stderr, "Can't open PID file: %s: ", filePath);
	} else {
		fprintf(runFile, "%d", pid);
		fclose(runFile);
	}

	svc_run();
	fprintf(stderr,"rpc.statd: svc_run returned\n");
	exit(1);
	/* NOTREACHED */
}




