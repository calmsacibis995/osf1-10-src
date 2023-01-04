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
static char *rcsid = "@(#)$RCSfile: rpc.rusersd.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/26 17:48:27 $";
#endif


/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <rpcsvc/rusers.h>


#define	DIV60(t)	((t+30)/60)    /* x/60 rounded */
#define MAXINT 0x7fffffff
#define min(a,b) ((a) < (b) ? (a) : (b))

struct utmparr utmparr;
struct utmpidlearr utmpidlearr;
int cnt;
int rusers_service();

#define NLNTH 8			/* sizeof ut_name */

main()
{

	register SVCXPRT * transp;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);


#ifdef DEBUG
	{
		int s;
		struct sockaddr_in addr;
		int len = sizeof(struct sockaddr_in);

		if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			perror("inet: socket");
			return - 1;
		}

		bzero((char *)&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = 0;

		if (bind(s, &addr, sizeof(addr)) < 0) {
			perror("bind");
			return - 1;
		}
		if (getsockname(s, &addr, &len) != 0) {
			perror("inet: getsockname");
			(void)close(s);
			return - 1;
		}
		pmap_unset(RUSERSPROG, RUSERSVERS_ORIG);
		pmap_set(RUSERSPROG, RUSERSVERS_ORIG, IPPROTO_UDP,
		    ntohs(addr.sin_port));
		pmap_unset(RUSERSPROG, RUSERSVERS_IDLE);
		pmap_set(RUSERSPROG, RUSERSVERS_IDLE, IPPROTO_UDP,
		    ntohs(addr.sin_port));
		if (dup2(s, 0) < 0) {
			perror("dup2");
			exit(1);
		}
	}
#endif	

#define USE_INETD

#ifdef USE_INETD


	if (getsockname(0, &addr, &len) != 0) {
		perror("rstat: getsockname");
		exit(1);
	}
	if ((transp = svcudp_create(0)) == NULL) {
		fprintf(stderr, "svc_rpc_udp_create: error\n");
		exit(1);
	}
	if (!svc_register(transp, RUSERSPROG, RUSERSVERS_ORIG,
	    rusers_service,0)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}
	if (!svc_register(transp, RUSERSPROG, RUSERSVERS_IDLE,
	    rusers_service,0)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}


#else /* USE_INETD */

	if ((transp = svcudp_create(RPC_ANYSOCK)) == NULL) {
		fprintf(stderr, "svc_rpc_udp_create: error\n");
		exit(1);
	}
	if (!svc_register(transp, RUSERSPROG, RUSERSVERS_ORIG,
	    rusers_service, IPPROTO_UDP)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}
	if (!svc_register(transp, RUSERSPROG, RUSERSVERS_IDLE,
	    rusers_service,IPPROTO_UDP)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}

#endif /* USE_INETD */

	svc_run();		/* never returns */
	fprintf(stderr, "run_svc_rpc should never return\n");
	exit(1);
	/* NOTREACHED */
}

getutmp(all, idle)
	int all;		/* give all listings? */
	int idle;		/* get idle time? */
{

/*
 * to maintain compatibility with Sun, we couldn't use
 * the utmp structure from our local utmp.h files.
 * Instead, we used the utmp structure from SunOS.
 * But now, we will have to read the local utmp file
 * and the buffer into which the data is captured has 
 * to be of the local Alpha DEC OSF/1 type. So, utmp_alpha
 * is actually taken from the pool's utmp.h file.
 * Stuff gathered in Alpha style utmp will be converted to
 * Sun style utmp structure before passing the data to xdr
 * routines for transport. 
 */
  struct utmp_alpha  /* this is cut and pasted here from /usr/include/utmp.h */
  {
        char ut_user[32] ;              /* User login name */
        char ut_id[14] ;               /* /etc/inittab id- IDENT_LEN in init */
        char ut_line[32] ;              /* device name (console, lnxx) */
        short ut_type ;                 /* type of entry */
        pid_t ut_pid ;                  /* process id */
        struct exit_status
          {
            short e_termination ;       /* Process termination status */
            short e_exit ;              /* Process exit status */
          }
        ut_exit ;                       /* The exit status of a process
                                         * marked as DEAD_PROCESS.
                                         */
        time_t ut_time ;                /* time entry was made */
        char ut_host[64] ;              /* host name same as MAXHOSTNAMELEN */
  } ;


	struct utmp_alpha buf; /* buffer to read local(Alpha) utmp info */
	struct utmp buf2, **p; /* buf2 to transform Alpha utmp to Sun's */
	struct utmpidle **q, *console = NULL;
	int minidle;
	FILE *fp;
	char *file;
	char name[NLNTH];
	
	cnt = 0;
	file = "/var/adm/utmp";
	if ((fp = fopen(file, "r")) == NULL) {
		fprintf(stderr, "can't open %s\n", file);
		exit(1);
	};
	p = utmparr.uta_arr;
	q = utmpidlearr.uia_arr;
	while (fread(&buf, sizeof(buf), 1, fp) == 1) {
		if (buf.ut_line[0] == 0 || buf.ut_user[0] == 0)
			continue;
		/* 
		 * if tty[pqr]? and not remote, then skip it
		 */

		if (!all && nonuser(buf))
			continue;


		/* 
		 * need to free this
		 */
		if (idle) {
			*q = (struct utmpidle *)
			    malloc(sizeof(struct utmpidle));
			if (strncmp(buf.ut_line, "console",
			    strlen("console")) == 0) {
				console = *q;
				strncpy(name, buf.ut_user, NLNTH);
			    }
			strncpy(buf2.ut_line,buf.ut_line,SUN_UT_LINE);
			strncpy(buf2.ut_name,buf.ut_user,SUN_UT_NAME);
			strncpy(buf2.ut_host,buf.ut_host,SUN_UT_HOST);
			buf2.ut_time = (int) buf.ut_time;
			bcopy(&buf2, &((*q)->ui_utmp), sizeof(buf2));
			(*q)->ui_idle = (int) findidle(buf.ut_line,
			    sizeof(buf.ut_line));
#ifdef DEBUG
			printf("%-10s %-10s %-18s %s; idle %d",
			    buf.ut_line, buf.ut_name,
			    buf.ut_host, ctime(&buf.ut_time),
			    (*q)->ui_idle);
#endif
			q++;
		}
		else {
			*p = (struct utmp *)malloc(sizeof(struct utmp));
			strncpy(buf2.ut_line,buf.ut_line,SUN_UT_LINE);
			strncpy(buf2.ut_name,buf.ut_user,SUN_UT_NAME);
			strncpy(buf2.ut_host,buf.ut_host,SUN_UT_HOST);
			buf2.ut_time = (int) buf.ut_time;
			bcopy(&buf2, *p, sizeof(buf2));
#ifdef DEBUG
			printf("%-10s %-10s %-18s %s",
			    buf.ut_line, buf.ut_name,
			    buf.ut_host, ctime(&buf.ut_time));
#endif
			p++;
		}
		cnt++;
	      }
	/* 
	 * On the console, the user may be running a window system; if so,
	 * their activity will show up in the last-access times of
	 * "/dev/kbd" and "/dev/mouse", so take the minimum of the idle
	 * times on those two devices and "/dev/console" and treat that as
	 * the idle time.
	 */
	if (idle && console)
		console->ui_idle = min(console->ui_idle,
		    min(findidle("kbd", strlen("kbd")),
		    findidle("mouse", strlen("mouse"))));
}

rusers_service(rqstp, transp)
	register struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	switch (rqstp->rq_proc) {
	case 0:
		if (svc_sendreply(transp, xdr_void, 0)  == FALSE) {
			fprintf(stderr, "err: rusersd");
			exit(1);
		    }
		exit(0);
	case RUSERSPROC_NUM:
		utmparr.uta_arr = (struct utmp **)
		    malloc(MAXUSERS*sizeof(struct utmp *));
		getutmp(0, 0);
		if (!svc_sendreply(transp, xdr_u_int, &cnt))
			perror("svc_rpc_send_results");
		free(utmparr.uta_arr);
		exit(0);
	case RUSERSPROC_NAMES:
	case RUSERSPROC_ALLNAMES:
		if (rqstp->rq_vers == RUSERSVERS_ORIG) { 
			utmparr.uta_arr = (struct utmp **)
			    malloc(MAXUSERS*sizeof(struct utmp *));
			getutmp(rqstp->rq_proc == RUSERSPROC_ALLNAMES, 0);
		        utmparr.uta_cnt = cnt;
			if (!svc_sendreply(transp, xdr_utmparr, &utmparr))
				perror("svc_rpc_send_results");
			svc_freeargs(transp, xdr_utmparr, &utmparr); 
			free(utmparr.uta_arr); 
			exit(0);
		}
		else { 
			utmpidlearr.uia_arr = (struct utmpidle **)
			    malloc(MAXUSERS*sizeof(struct utmpidle *));
			getutmp(rqstp->rq_proc == RUSERSPROC_ALLNAMES, 1);
			utmpidlearr.uia_cnt = cnt;
			if (!svc_sendreply(transp, xdr_utmpidlearr,
			    &utmpidlearr))
				perror("svc_rpc_send_results");
			svc_freeargs(transp, xdr_utmpidlearr, &utmpidlearr); 
			free(utmpidlearr.uia_arr);
			exit(0);
		} 
	default: 
		svcerr_noproc(transp);
		exit(0);
	}
}

/* find & return number of minutes current tty has been idle */
findidle(name, ln)
	char *name;
{
	time_t	now;
	struct stat stbuf;
	long lastaction, diff;
	char ttyname[20];

	strcpy(ttyname, "/dev/");
	strncat(ttyname, name, ln);
	if (stat(ttyname, &stbuf) < 0)
		return(MAXINT);
	time(&now);
	lastaction = stbuf.st_atime;
	diff = now - lastaction;
	diff = DIV60(diff);
	if (diff < 0) diff = 0;
	return(diff);
}


nonuser( buffer )
struct utmp buffer;
{

if ( strcmp (buffer.ut_name, "LOGIN"))
  return 0;
else
  return 1;
}
