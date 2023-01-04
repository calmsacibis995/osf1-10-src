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
static char     *sccsid = "@(#)$RCSfile: ypserv.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/03/27 12:32:46 $";
#endif
/*
 */

/*
 * This contains the mainline code for the NIS server.  Data
 * structures which are process-global are also in this module.  
 */

#include <rpcsvc/ypsym.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <rpcsvc/ypdefs.h>

USE_YPDBPATH
USE_YP_LAST_MODIFIED
USE_YP_MASTER_NAME
#define YPBIGSENDSZ 24*1024   /*raise sendsz*/
#include <signal.h>
int debuginterdomain=0;		/* if  non-zero do interdomain*/
int dbinterdomain=0;		/* if  non-zero trace interdomain*/

static char create_failed[] = "ypserv:  Unable to create server for ";
static char register_failed[] = "ypserv:  Unable to register service for ";
struct timeval ypintertry = {		/* udp secs betw tries in peer comm */
	YPINTERTRY_TIME,		/* Seconds */
	0				/* uSecs */
};
struct timeval yptimeout = {		/* udp total timeout for peer comm */
	YPTOTAL_TIME,			/* Seconds */
	0				/* uSecs */
};
char myhostname[MAX_MASTER_NAME + 1];
SVCXPRT *udphandle;
SVCXPRT *tcphandle;
bool silent = TRUE;
char logfile[] = "/var/yp/ypserv.log";

void ypexit();
void ypinit();
void ypdispatch();
void ypolddispatch();
void ypget_command_line_args();
void dezombie();
void logprintf();

/*
 * External refs to functions named only by the dispatchers.
 */
extern void ypdomain();
extern void ypmatch();
extern void ypfirst();
extern void ypnext();
extern void ypxfr();
extern void ypall();
extern void ypmaster();
extern void yporder();
extern void ypoldmatch();
extern void ypoldfirst();
extern void ypoldnext();
extern void ypoldpoll();
extern void yppush();
extern void yppull();
extern void ypget();
extern void ypmaplist();

/*
 * This is the main line code for the NIS server.
 */
#ifdef SVC_RUN_AS
main(argc, argv)
	int argc;
	char **argv;
{
	int readfds;

	if (geteuid() != 0) {
		(void)fprintf(stderr, "must be root to run %s\n", argv[0]);
		exit(1);
	}
 	ypinit(argc, argv); 			/* Set up shop */
	svc_run_as(); /*run asynchronous services*/
	fprintf(stderr,"svc_run_as returned\n");
	pmap_unset(YPPROG, YPVERS);
	pmap_unset(YPPROG, YPOLDVERS);
	abort();
}
#else
main(argc, argv)
        int argc;
        char **argv;
{
        int readfds;
        struct timer_action *palarm_action;

        /*
         * must be superuser to run
         */
        if (geteuid() != 0){
                (void) fprintf(stderr, "ypserv:  must be super user\n");
                (void) fflush(stderr);
                exit(1);
        }

        ypinit(argc, argv);                     /* Set up shop */
	

        for (;;) {

                readfds = svc_fds;
                errno = 0;
                switch ( (int) select(32, &readfds, (int *) NULL, (int *) NULL, (struct  timeval *) NULL) ) {

                case -1:  {

                        if (errno != EINTR) {
                            logprintf(
                           "ypserv:  bad fds bits in main loop select mask.\n");
                        }

                        break;
                }

                case 0:  {
                        logprintf(
                            "ypserv:  invalid timeout in main loop select.\n");
                        break;
                }

                default:  {
                        svc_getreq(readfds);
                        break;
                }
                }
        }
}
#endif

/*
 * Does startup processing for the NIS server.
 */
void
ypinit(argc, argv)
	int argc;
	char **argv;
{
	pid_t pid;
	int t;


	pmap_unset(YPPROG, YPVERS);
	pmap_unset(YPPROG, YPOLDVERS);
	ypget_command_line_args(argc, argv);

	if (silent) {
		
		pid = fork();
		
		if (pid == -1) {
			logprintf(
			     "ypserv:  ypinit fork failure.\n");
			ypexit();
		}
	
		if (pid != 0) {
			exit(0);
		}
	
		if (access(logfile, W_OK)) {
			(void) freopen("/dev/null", "w", stderr);
		} else {
			(void) freopen(logfile, "a", stderr);
			(void) freopen(logfile, "a", stdout);
		}

		for (t = 3; t < 20; t++) {
			(void) close(t);
		}
	

 		t = open("/dev/tty", 2);
	
 		if (t >= 0) {
 			(void) ioctl(t, (int) TIOCNOTTY, (char *) 0);
 			(void) close(t);
 		}
	}

	(void) gethostname(myhostname, 256);

	if ((int) signal(SIGCHLD, dezombie) == -1) {
		logprintf( "Can't catch process exit signal.\n");
		ypexit();
	}

	if ((udphandle = svcudp_bufcreate(RPC_ANYSOCK, YPMSGSZ, YPMSGSZ))
	    == (SVCXPRT *) NULL) {
		logprintf( "%s%s.\n", create_failed, "udp");
		ypexit();
	}

	if ((tcphandle = svctcp_create(RPC_ANYSOCK, YPMSGSZ, YPBIGSENDSZ))
	    == (SVCXPRT *) NULL) {
		logprintf( "%s%s.\n", create_failed, "tcp");
		ypexit();
	}

	if (!svc_register(udphandle, YPPROG, YPVERS, ypdispatch,
	    IPPROTO_UDP) ) {
		logprintf( "%s%s.\n", register_failed, "udp");
		ypexit();
	}

	if (!svc_register(tcphandle, YPPROG, YPVERS, ypdispatch,
	    IPPROTO_TCP) ) {
		logprintf( "%s%s.\n", register_failed, "tcp");
		ypexit();
	}
	
	if (!svc_register(udphandle, YPPROG, YPOLDVERS, ypolddispatch,
	    IPPROTO_UDP) ) {
		logprintf( "%s%s.\n", register_failed, "udp");
		ypexit();
	}

	if (!svc_register(tcphandle, YPPROG, YPOLDVERS, ypolddispatch,
	    IPPROTO_TCP) ) {
		logprintf( "%s%s.\n", register_failed, "tcp");
		ypexit();
	}
}

/*
 * This picks up any command line args passed from the process invocation.
 */
void
ypget_command_line_args(argc, argv)
	int argc;
	char **argv;
{
	argv++;


	while (--argc) {
		
		if ((*argv)[0] == '-') {

			switch ((*argv)[1]) {
				case 'v':
					silent = FALSE;
					break;
				case 'i':
					debuginterdomain = TRUE;
					break;
				case 'd':
					dbinterdomain = TRUE;
					break;
				default:
					fprintf(stderr, 
					    "ypserv: illegal option %s\n",
					*argv);
					fprintf(stderr, 
					    "ypserv: usage: ypserv [-d]\n");
			}
				
		}
		argv++;
	}
}

/*
 * This dispatches to server action routines based on the input procedure
 * number.  ypdispatch is called from the RPC function svc_getreq.
 */
void
ypdispatch(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
int mask;


	mask=sigblock(sigmask(SIGCHLD)); /*prevent child calls during run*/
	switch (rqstp->rq_proc) {

	case YPPROC_NULL:

		if (!svc_sendreply(transp, xdr_void, 0) ) {
			logprintf(
			    "ypserv:  Can't reply to rpc call.\n");
		}

		break;

	case YPPROC_DOMAIN:

		ypdomain(rqstp, transp, TRUE);
		break;

	case YPPROC_DOMAIN_NONACK:

		ypdomain(rqstp, transp, FALSE);
		break;

	case YPPROC_MATCH:

		ypmatch(rqstp, transp);
		break;

	case YPPROC_FIRST:

		ypfirst(rqstp, transp);
		break;

	case YPPROC_NEXT:

		ypnext(rqstp, transp);
		break;

	case YPPROC_XFR:

		ypxfr(rqstp, transp);
		break;

	case YPPROC_CLEAR:

		ypclr_current_map();
		closealldbm();
		
		if (!svc_sendreply(transp, xdr_void, 0) ) {
			logprintf(
			    "ypserv:  Can't reply to rpc call.\n");
		}

		break;

	case YPPROC_ALL:

		ypall(rqstp, transp);
		break;

	case YPPROC_MASTER:

		ypmaster(rqstp, transp);
		break;

	case YPPROC_ORDER:

		yporder(rqstp, transp);
		break;

	case YPPROC_MAPLIST:

		ypmaplist(rqstp, transp);
		break;

	default:
		svcerr_noproc(transp);
		break;

	}
	sigsetmask(mask); /*allow child call during idle loop*/
	return;
}

/*
 * This is the dispatcher for the old NIS protocol.  The case symbols are
 * defined in ypv1_prot.h, and are copied (with an added "OLD") from version
 * 1 of yp_prot.h.
 */
void
ypolddispatch(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{


	switch (rqstp->rq_proc) {

	case YPOLDPROC_NULL:

		if (!svc_sendreply(transp, xdr_void, 0) ) {
			logprintf(
			    "ypserv:  Can't reply to rpc call.\n");
		}

		break;

	case YPOLDPROC_DOMAIN:
		ypdomain(rqstp, transp, TRUE);
		break;

	case YPOLDPROC_DOMAIN_NONACK:
		ypdomain(rqstp, transp, FALSE);
		break;

	case YPOLDPROC_MATCH:
		ypoldmatch(rqstp, transp);
		break;

	case YPOLDPROC_FIRST:
		ypoldfirst(rqstp, transp);
		break;

	case YPOLDPROC_NEXT:
		ypoldnext(rqstp, transp);
		break;

	case YPOLDPROC_POLL:
		ypoldpoll(rqstp, transp);
		break;

	case YPOLDPROC_PUSH:
		yppush(rqstp, transp);
		break;

	case YPOLDPROC_PULL:
		yppull(rqstp, transp);
		break;

	case YPOLDPROC_GET:
		ypget(rqstp, transp);
		break;

	default:
		svcerr_noproc(transp);
		break;

	}

	return;
}

/*
 * This flushes output to stderr, then aborts the server process to leave a
 * core dump.
 */
static void
ypexit()
{

	(void) fflush(stderr);
	(void) abort();
}


/*
 * This constructs a logging record.
 */
void
logprintf(arg1,arg2,arg3,arg4,arg5,arg6,arg7)
long arg1,arg2,arg3,arg4,arg5,arg6,arg7;
{
	struct timeval t;


	if (silent) {
		(void) gettimeofday(&t, NULL);
		fseek(stderr,(off_t)0,2);
		(void) fprintf(stderr, "%19.19s: ", ctime(&t.tv_sec));
	}
	(void) fprintf(stderr,arg1,arg2,arg3,arg4,arg5,arg6,arg7);
	fflush(stderr);
}
