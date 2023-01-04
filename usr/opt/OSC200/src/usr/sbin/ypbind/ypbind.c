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
static char     *sccsid = "@(#)$RCSfile: ypbind.c,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1992/12/02 13:01:40 $";
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

/*
 * This constructs a list of servers by domains, and keeps more-or-less up to
 * date track of those server's reachability.
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <rpc/rpc.h>
#include <rpc/svc.h>
#include <dirent.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypv1_prot.h>
#include <rpcsvc/ypclnt.h>
#include <netinet/in.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#define CACHE_DIR	"/var/yp/binding"

/*
 * The domain struct is the data structure used by the NIS binder to remember
 * mappings of domain to a server.  The list of domains is pointed to by
 * known_domains.  Domains are added when the NIS binder gets binding requests
 * for domains which are not currently on the list.  Once on the list, an
 * entry stays on the list forever.  Bindings are initially made by means of
 * a broadcast method, using functions ypbind_broadcast_bind and
 * ypbind_broadcast_ack.  This means of binding is re-done any time the domain
 * becomes unbound, which happens when a server doesn't respond to a ping.
 * current_domain is used to communicate among the various functions in this
 * module; it is set by ypbind_get_binding.
 *  
 */
struct domain {
	struct domain *dom_pnext;
	char dom_name[MAXNAMLEN + 1];
	unsigned short dom_vers;	/* YPVERS or YPOLDVERS */
	bool dom_boundp;
	CLIENT *ping_clnt;
	struct in_addr dom_serv_addr;
	unsigned short int dom_serv_port;
	int dom_report_success;		/* Controls msg to /dev/console*/
	pid_t dom_broadcaster_pid;
	int bindfile;			/* File with binding info in it */

	int broadcaster_fd;
	FILE *broadcaster_pipe;	        /* to get answer from broadcaster*/
	XDR broadcaster_xdr;		/**/
	struct timeval  lastping;
};
static int ping_sock = RPC_ANYSOCK;
struct domain *known_domains = (struct domain *) NULL;
struct domain *current_domain;		/* Used by ypbind_broadcast_ack, set
					 *   by all callers of clnt_broadcast */
struct domain *broadcast_domain;	/* Set by ypbind_get_binding, used
					 *   by the mainline. */
SVCXPRT *tcphandle;
SVCXPRT *udphandle;

#define BINDING_TRIES 1			/* Number of times we'll broadcast to
					 *   try to bind default domain.  */
#define PINGTOTTIM 20			/* Total seconds for ping timeout */
#define PINGINTERTRY 10
#define SETDOMINTERTRY 20
#define SETDOMTOTTIM 60

/* Timeout structures for Digital's added -S security option */

#define UDPINTER_TRY 5                  /* Secs between tries for udp*/
#define UDPTIMEOUT UDPINTER_TRY*3       /* Total timeout for udp */
#define CALLINTER_TRY 10                /* Secs between callback tries*/
#define CALLTIMEOUT CALLINTER_TRY*2     /* Total timeout for callback */
struct timeval udp_intertry = {
        UDPINTER_TRY,
        0
};
struct timeval udp_timeout = {
        UDPTIMEOUT,
        0
};
struct timeval tcp_timeout = {
        180,    /* timeout for map enumeration (could be long) */
        0
};
 
/* End Digital -S option code */

#ifdef VERBOSE
int silent = FALSE;
#else
int silent = TRUE;
#endif
#define	YPSETLOCAL	3
static int secure = FALSE;  /* running more securely; associated with the -s flag */
static int securebind = FALSE;  /* running insecurely*/
static int setok = FALSE;   /* accept ypset*/
static int decosf_secure = FALSE;  /* running with Digital -S option */

extern int errno;

void dispatch();
void ypbind_dispatch();
void ypbind_olddispatch();
void ypbind_get_binding();
void ypbind_set_binding();
void ypbind_pipe_setdom();
struct domain *ypbind_point_to_domain();
bool ypbind_broadcast_ack();
bool ypbind_ping();
void ypbind_init_default();
void broadcast_proc_exit();

extern bool xdr_ypdomain_wrap_string();
extern bool xdr_ypbind_resp();
static int interlock_fd = -1;
int unregister();

/******************************************************************************/
/*      Digital's added -S security option                                    */
/*                                                                            */
/*      usage - ypbind -S domainname,server1[,server2,server3,server4]        */
/*      The -S option allows the system administrator to startup ypbind       */
/*      in a secure mode.  The -S option locks this ypbind process into the   */
/*      specified domainname and server list.                                 */
/*      It will not accept any other system as its server.                    */
/*      Consequently the secure option should only be used where either the   */
/*      system itself is a server and you are setting it to itself or when a  */
/*      very reliable server is available in your environment.                */
/******************************************************************************/

/* Global variables required by Digital's -S security option */
char srvname[4][64];
char srvaddr[4][64];
char setdomain[64];
struct hostent *hp;
struct sockaddr_in sock[4];
struct sockaddr_in *sin;
struct sockaddr_in myaddr;
int locked=0;
int preflock= -1;

main(argc, argv)
	int argc;
	char **argv;
{
	pid_t pid;
	int t;
	int i;
	int j,k;   /* for Digital's -S security option */
	int nd;
	int readfds;
	char *pname;
	bool true;
	char *securarg;
        extern char *optarg;

	if (geteuid()!= 0) {

	fprintf(stderr, "ypbind: can only be run by root\n");
	(void) fflush(stderr);
	exit(1);
	}
	setreuid(0,0);
#ifdef	LOCK_EX
	interlock();
#endif
	/* Initialize my address reference for Digital's -S security option */
        get_myaddress(&myaddr);

	for (i=1;i<=SIGTERM;i++){
		if (i != SIGHUP) signal(i,unregister);
		else signal(i, SIG_IGN);
	}
	(void) pmap_unset(YPBINDPROG, YPBINDVERS);
	(void) pmap_unset(YPBINDPROG, YPBINDOLDVERS);
	/*
	 * Scrap the initial binding processing for now, and see if we like
	 * fast startup better than initial bindings.
	 */
#ifdef INIT_DEFAULT
	 ypbind_init_default();
#endif
#ifdef SUN_S
	/*
	 * Check to see if we are running "secure" which means that we should
	 * require that the server be using a reserved port.  We are running
	 * secure if the -s option is specified
	 */
#endif SUN_S
	argc--;
	argv++;
	while (argc > 0) {
#ifdef SUN_S
		if (!strcmp(*argv,"-s")) {
			secure = TRUE;
			securebind = TRUE;
		}
		else if (!strcmp(*argv,"-secure")) {
			secure = TRUE;
			securebind = TRUE;
		}
		else if (!strcmp(*argv,"-insecure")) {
			secure = FALSE;
			securebind = FALSE;
		} else
#endif SUN_S
		if (strcmp(*argv,"-S") == 0) {    /* DEC's -S security option */
			decosf_secure = TRUE;
 			argc--;
                        argv++;
			strcpy(&securarg,(char *) argv);
		}
		else if (!strcmp(*argv,"-v")) {
			silent = FALSE;
			fprintf(stderr, "running verbose (for debugging)\n");
		}
		else if (!strcmp(*argv,"-ypset")) {
			setok = TRUE;
		}
		else if (!strcmp(*argv,"-ypsetme")) {
			setok = YPSETLOCAL;
		}
		else {
			fprintf(stderr, "usage: ypbind [-S domainname,server1[,server2[,server3[,server4]]]] [-v] [-ypset] [-ypsetme]\n");
			exit(1);
		}
		argc--,
		argv++;
	}

	if (decosf_secure == TRUE)    /*** Digital's -S security option ***/
	   	{
                for ( t=0, j=0, i=0, k=0;
		      securarg[i] != '\0'; 
		      i++ )
                	{ 
                     	if(!locked)
                             if(securarg[i] == ',')
                                        {
                                        setdomain[t]='\0';
                                        setdomainname(setdomain,strlen(setdomain));
                                        locked=1;
                                        }
                                else setdomain[t++]=securarg[i];
                        else    {
                                if(securarg[i] == ',')
                                        {
                                        srvname[k++][j]='\0';
                                        if(k > 3)
                                                {
                                                fprintf(stderr,"Maximum of fourservers selectable\n");
                                                exit(1);
                                                }

                                        j=0;
                                        }
                                else    srvname[k][j++]=securarg[i];
                        }
           	 }
     
		 /* save number of servers in locked flag */
                 if(!locked)
                 	{
                        fprintf(stderr,
	      "usage: ypbind [-S domainname,server1[,server2[,server3[,server4]]]] [-v] [-ypset] [-ypsetme]\n");
                        exit(1);
			}
                  locked=k+1;
	 	  srvname[k][j]='\0';
                  for (i=0;i<locked;i++)
                          {
                          bzero((caddr_t)&sock[i], sizeof (struct sockaddr_in));
                          sin = &sock[i];
                          sin->sin_family = AF_INET;
                          sin->sin_port = 0;
                          sin->sin_addr.s_addr = inet_addr(srvname[i]);
                          if (sin->sin_addr.s_addr == -1) {
                                hp=gethostbyname(srvname[i]);
                                if (hp == NULL) {
                                        fprintf(stderr, 
				        "Unknown host %s.  Please check that host %s is in the /etc/hosts file on this machine.  Then rerun ypbind.\n", srvname[i], srvname[i]);
                                        exit(1);
                                        }
                                bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,sizeof sin->sin_addr);
                           } 
                  /* show preference to the local server */
                  if(sin->sin_addr.s_addr == myaddr.sin_addr.s_addr)
                            preflock=i;
                  }
                  /******* End DEC OSF/1 -S security option *******/
                
	}

	signal(SIGHUP,SIG_IGN); /*try this*/
	if (silent) {
		pid = fork();
		
		if (pid == -1) {
			(void) fprintf(stderr, "ypbind:  fork failure.\n");
			(void) fflush(stderr);
			abort();
		}
	
		if (pid != 0) {
			exit(0);
		}
	
		nd = getdtablesize();
		for (t = 0; t < nd ; t++) {
			if (t!=interlock_fd)
			(void) close(t);
		}
	

 		(void) open("/dev/console", O_WRONLY+O_NOCTTY); /*posix*/
 		(void) dup2(0, 1);
 		(void) dup2(0, 2);

 		t = open("/dev/tty", 2);
	
 		if (t >= 0) {
 			(void) ioctl(t, TIOCNOTTY, (char *)0);
 			(void) close(t);
 		}
		(void) setpgrp(getpid(),0);  /*posix should be setsid XXX*/
	}
	else (void) setpgrp(getpid(),0);  /*posix should be setsid XXX */

#ifdef SUN_S
	if (secure==TRUE) {
		fprintf(stderr, "ypbind: Secure mode sunos 3.x servers rejected.\n");
	}
#endif SUN_S
	if (setok==TRUE) {
		fprintf(stderr, "ypbind -ypset: allowing ypset! (this is insecure)\n");
	}
	if (setok==YPSETLOCAL) {
		fprintf(stderr, "ypbind -ypsetme: allowing local ypset! (this is insecure)\n");
	}

	if ((int) signal(SIGCHLD, broadcast_proc_exit) == -1) {
		(void) fprintf(stderr,
		    "ypbind:  Can't catch broadcast process exit signal.\n");
		(void) fflush(stderr);
		abort();
	}
	if ((int) signal(SIGPIPE, SIG_IGN) == -1) {
		(void) fprintf(stderr,
			"ypbind: Can't ignore pipe exit signal.\n");
		(void) fflush(stderr);
		abort();
	}
        /* Open a socket for pinging everyone can use */
        ping_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (ping_sock < 0) {
                (void) fprintf(stderr,
                  "ypbind: Cannot create socket for pinging.\n");
                (void) fflush(stderr);
                abort();
        }

	/* 
	 * if not running c2 secure, don't use privileged ports.
	 * Accomplished by side effect of not being root when creating
	 * rpc based sockets.
	 */
	if (! securebind) {
		(void) setreuid(-1, 3);
	}

	if ((tcphandle = svctcp_create(RPC_ANYSOCK,
	    RPCSMALLMSGSIZE, RPCSMALLMSGSIZE)) == NULL) {
		(void) fprintf(stderr, "ypbind:  can't create tcp service.\n");
		(void) fflush(stderr);
		abort();
	}

	if (!svc_register(tcphandle, YPBINDPROG, YPBINDVERS,
	    ypbind_dispatch, IPPROTO_TCP) ) {
		(void) fprintf(stderr,
		    "ypbind:  can't register tcp service.\n");
		(void) fflush(stderr);
		abort();
	}

	if ((udphandle = svcudp_bufcreate(RPC_ANYSOCK,
	    RPCSMALLMSGSIZE, RPCSMALLMSGSIZE)) == (SVCXPRT *) NULL) {
		(void) fprintf(stderr, "ypbind:  can't create udp service.\n");
		(void) fflush(stderr);
		abort();
	}

	if (!svc_register(udphandle, YPBINDPROG, YPBINDVERS,
	    ypbind_dispatch, IPPROTO_UDP) ) {
		(void) fprintf(stderr,
		    "ypbind:  can't register udp service.\n");
		(void) fflush(stderr);
		abort();
	}

	if (!svc_register(tcphandle, YPBINDPROG, YPBINDOLDVERS,
	    ypbind_olddispatch, IPPROTO_TCP) ) {
		(void) fprintf(stderr,
		    "ypbind:  can't register tcp service.\n");
		(void) fflush(stderr);
		abort();
	}

	if (!svc_register(udphandle, YPBINDPROG, YPBINDOLDVERS,
	    ypbind_olddispatch, IPPROTO_UDP) ) {
		(void) fprintf(stderr,
		    "ypbind:  can't register udp service.\n");
		(void) fflush(stderr);
		abort();
	}
	/* undo the gross hack associated with c2 security */
	(void) setreuid(-1, 0);

	/* Digital's -S security option */
	/* If we are running in locked mode lock up the domain and server */
        if (locked)
                {
                ypbind_locked(preflock,YPVERS);
                }

	for (;;) {
		readfds = svc_fds;
		errno = 0;

		switch ( (int) select(32, &readfds, NULL, NULL, NULL) ) {

		case -1:  {
		
			if (errno != EINTR) {
			    (void) fprintf (stderr,
			    "ypbind: bad fds bits in main loop select mask.\n");
			}

			break;
		}

		case 0:  {
			(void) fprintf (stderr,
			    "ypbind:  invalid timeout in main loop select.\n");
			break;
		}

		default:  {
			svc_getreq (readfds);
			break;
		}
		
		}
	}
		/* NOTREACHED */
}

/*
 * ypbind_dispatch and ypbind_olddispatch are wrappers for dispatch which
 * remember which protocol the requestor is looking for.  The theory is,
 * that since YPVERS and YPBINDVERS are defined in the same header file, if
 * a request comes in on the old binder protocol, the requestor is looking
 * for the old NIS server.
 */
void
ypbind_dispatch(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	dispatch(rqstp, transp, (unsigned short) YPVERS);
}

void
ypbind_olddispatch(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	dispatch(rqstp, transp, (unsigned short) YPOLDVERS);
}

/*
 * This dispatches to binder action routines.
 */
void
dispatch(rqstp, transp, vers)
	struct svc_req *rqstp;
	SVCXPRT *transp;
	unsigned short vers;
{

	switch (rqstp->rq_proc) {

	case YPBINDPROC_NULL:

		if (!svc_sendreply(transp, xdr_void, 0) ) {
			(void) fprintf(stderr,
			    "ypbind:  Can't reply to rpc call.\n");
		}

		break;

	case YPBINDPROC_DOMAIN:
		ypbind_get_binding(rqstp, transp, vers, FALSE, NULL );
		break;

	case YPBINDPROC_SETDOM:
		ypbind_set_binding(rqstp, transp, vers);
		break;

	default:
		svcerr_noproc(transp);
		break;

	}
}

/*
 * This is a Unix SIGCHILD handler which notices when a broadcaster child
 * process has exited, and retrieves the exit status.  The broadcaster pid
 * is set to 0.  If the broadcaster succeeded, dom_report_success will be
 * be set to -1.
 */

void
broadcast_proc_exit()
{
	pid_t pid;
	union wait wait_status;
	register struct domain *pdom;
	struct ypbind_setdom req;

	pid = 0;

	for (;;) {
		pid = wait3(&wait_status, WNOHANG, NULL);

		if (pid == 0) {
			return;
		} else if (pid == -1) {
			return;
		}
		
		for (pdom = known_domains; pdom != (struct domain *)NULL;
		    pdom = pdom->dom_pnext) {
			    
			if (pdom->dom_broadcaster_pid == pid) {
				pdom->dom_broadcaster_pid = 0;
				if ((wait_status.w_termsig == 0) &&
				    (wait_status.w_retcode == 0))
				{
					if (pdom->broadcaster_pipe) {
					pdom->dom_report_success = -1;
					/*do the xdr*/
					if (xdr_ypbind_setdom(
					&(pdom->broadcaster_xdr),&req)){

					/*might check domain and version here*/

						pdom->dom_serv_addr = req.ypsetdom_addr;
						pdom->dom_serv_port = req.ypsetdom_port;
						pdom->dom_boundp = TRUE;
						/* get rid of old pinging client if one exists */
						if (pdom->ping_clnt != (CLIENT *)NULL) {
							clnt_destroy(pdom->ping_clnt);
							pdom->ping_clnt = (CLIENT *)NULL;
						}
					}
					else {
					fprintf(stderr,"xdr_ypbind_setdom fails\n");
					}
					}
					else {
					    fprintf(stderr,"ypbind:internal error -- no broadcaster pipe.\n");
					}
				}

				/*Success or failure free the pipe*/
				if (pdom->broadcaster_pipe) {
					xdr_destroy( &(pdom->broadcaster_xdr));
					fclose(pdom->broadcaster_pipe);
				}
				close(pdom->broadcaster_fd);
				pdom->broadcaster_pipe=0;
				pdom->broadcaster_fd= -1;
			}
		}
	}

}

/*
 * This returns the current binding for a passed domain.
 */
void
ypbind_get_binding(rqstp, transp, vers, vrfy, vfname)
	struct svc_req *rqstp;
	register SVCXPRT *transp;
	unsigned short vers;
	bool_t vrfy;
	char *vfname;
{
	char domain_name[YPMAXDOMAIN + 1];
	char *pdomain_name = domain_name;
	char *pname;
	struct stat	buf;
	struct ypbind_resp response;
	bool newbinding=FALSE; /*better set  variables before using them*/
	char outstring[YPMAXDOMAIN + 256];
	pid_t broadcaster_pid;
	struct domain *v1binding;
	int fildes[2];	/*pipe*/
	int oldmask;
	struct timeval tp; /*for holddown*/
	int i;
#define YPBIND_PINGHOLD_DOWN 5   /*seconds*/
	if (vrfy)  strcpy(pdomain_name,vfname);
	else if (!svc_getargs(transp, xdr_ypdomain_wrap_string, &pdomain_name) ) {
		svcerr_decode(transp);
		return;
	}

	if ( (current_domain = ypbind_point_to_domain(pdomain_name, vers) ) !=
	    (struct domain *) NULL) {

		/*
		 * Ping the server to make sure it is up.
		 */
		 
		if (current_domain->dom_boundp) {
			(void) gettimeofday(&tp,0);
			if ((tp.tv_sec  - current_domain->lastping.tv_sec)>
				YPBIND_PINGHOLD_DOWN){

			newbinding = ypbind_ping(current_domain);
			}
		}

		/*
		 * Bound or not, return the current state of the binding.
		 */

		if (current_domain->dom_boundp) {
			response.ypbind_status = YPBIND_SUCC_VAL;
			response.ypbind_respbody.ypbind_bindinfo.ypbind_binding_addr =
			    current_domain->dom_serv_addr;
			response.ypbind_respbody.ypbind_bindinfo.ypbind_binding_port = 
			    current_domain->dom_serv_port;
		} else {
			response.ypbind_status = YPBIND_FAIL_VAL;
			response.ypbind_respbody.ypbind_error =
			    YPBIND_ERR_NOSERV;
		}
		    
	} else {
		response.ypbind_status = YPBIND_FAIL_VAL;
		response.ypbind_respbody.ypbind_error = YPBIND_ERR_RESC;
	}

#ifndef NO_BINDING_FILE
	/* If this is a new binding, update the binding file */
	if (newbinding) {
		/* 
		 * This is pretty much a gross hack to speed up binding 
		 * operations using the yp_bind library call. By saving 
		 * this info in a file we save an rpc call and a server 
	  	 * 'ping'
		 */
tryagain:
		/* If no file is open then try to open it. */
		if (current_domain->bindfile == -1) {
			/* Generate the filename required ... */
			sprintf(outstring, "%s/%s.%d", CACHE_DIR,
				current_domain->dom_name, 
				current_domain->dom_vers);
			current_domain->bindfile = 
				open(outstring, O_RDWR+O_CREAT, 0644);
			/* XXX remove when the new libc routines are ready */
			if (current_domain->bindfile != -1)
				flock(current_domain->bindfile, LOCK_EX);
		}
		/* Write the binding information to it ... */
		if (current_domain->bindfile != -1) {
			lseek(current_domain->bindfile, 0L, L_SET);
			write(current_domain->bindfile, &(udphandle->xp_port),
				sizeof(u_short));
			write(current_domain->bindfile, &response,
				sizeof(response));
		} else {
			/* 
			 * If it failed to open, check to see that the 
			 * directory exists, if it does not exist, create 
			 * it and try again. If it does exist then abort
		 	 * and don't bother with the cache file. 
			 */
			if (stat(CACHE_DIR,&buf) < 0) {
				if (errno == ENOENT) mkdir("/var/yp", 0655);
				mkdir(CACHE_DIR, 0655);
				goto tryagain;
			}
		}
	}
#endif
        
	if (!vrfy ){
	if (!svc_sendreply(transp, xdr_ypbind_resp, &response) ) {
		(void) fprintf(stderr,
		    "ypbind:  Can't respond to rpc request.\n");
	}

	if (!svc_freeargs(transp, xdr_ypdomain_wrap_string, &pdomain_name) ) {
		(void) fprintf(stderr,
		    "ypbind:  ypbind_get_binding can't free args.\n");
	}
	}
	if (locked && (!current_domain->dom_boundp))  /* DEC's -S option */
                ypbind_locked (preflock,vers);

	else if ((current_domain) && (!current_domain->dom_boundp) &&
	    (!current_domain->dom_broadcaster_pid)) {
		/*
		 * The current domain is unbound, and there is no broadcaster 
		 * process active now.  Fork off a child who will yell out on 
		 * the net.  Because of the flavor of request we're making of 
		 * the server, we only expect positive ("I do serve this
		 * domain") responses.
		 */
		broadcast_domain = current_domain;
		broadcast_domain->dom_report_success++;
		pname = current_domain->dom_name;


		if ( pipe(fildes) >=0){
		oldmask=sigblock(sigmask(SIGCHLD));
		if ( (broadcaster_pid = fork() ) == 0) {
			int	true = 1;
			for (i=1;i<SIGPIPE;i++) signal(i,SIG_DFL);
			for (i=SIGPIPE+1;i<=SIGTERM;i++) signal(i,SIG_DFL);
			if (interlock_fd>=0) close(interlock_fd);
			/*child shouldn't hold lock*/
			sigsetmask(oldmask);
			current_domain->broadcaster_fd=fildes[1];
			current_domain->broadcaster_pipe=fdopen(fildes[1],"w");
			if (current_domain->broadcaster_pipe)
				xdrstdio_create(&(current_domain->broadcaster_xdr), (current_domain->broadcaster_pipe),XDR_ENCODE);
			else{
				perror("fdopen-pipe");
				exit(-1);
			}

			(void) clnt_broadcast(YPPROG, vers,
			    YPPROC_DOMAIN_NONACK, xdr_ypdomain_wrap_string,
			    &pname, xdr_int, &true, ypbind_broadcast_ack);
			    
			if (current_domain->dom_boundp) {
				
				/*
				 * Send out a set domain request to our parent
				 */
				ypbind_pipe_setdom(current_domain,pname,
				    current_domain->dom_serv_addr,
				    current_domain->dom_serv_port, vers);
				    
				if (current_domain->dom_report_success > 0) {
					(void) sprintf(outstring,
					 "NIS: server for domain \"%s\" OK",
					    pname);
					writeit(outstring);
				}
				exit(0);
			} else {
				/*
				 * Hack time.  If we're looking for a current-
				 * version server and can't find one, but we
				 * do have a previous-version server bound, then
				 * suppress the console message.
				 */
				if (vers == YPVERS && ((v1binding =
				   ypbind_point_to_domain(pname, YPOLDVERS) ) !=
				    (struct domain *) NULL) &&
				    v1binding->dom_boundp) {
					    exit(1);
				}
				
				(void) sprintf(outstring,
	      "NIS: server not responding for domain \"%s\"; still trying",
				    pname);
				writeit(outstring);
				exit(1);
			}

		} else if (broadcaster_pid == -1) {
			sigsetmask(oldmask);
			close(fildes[0]);
			close(fildes[1]);
			(void) fprintf(stderr,
			    "ypbind:  broadcaster fork failure.\n");
		} else {
		/*parent*/
			close (fildes[1]);
			current_domain->broadcaster_fd=fildes[0];
			current_domain->broadcaster_pipe=fdopen(fildes[0],"r");
			if (current_domain->broadcaster_pipe)
				xdrstdio_create(&(current_domain->broadcaster_xdr), (current_domain->broadcaster_pipe),XDR_DECODE);
			current_domain->dom_broadcaster_pid = broadcaster_pid;
			sigsetmask(oldmask);
		}
	} else  {
		(void) fprintf(stderr,
		    "ypbind: broadcaster pipe failure.\n");
	}
}
}

static int
writeit(s)
	char *s;
{
	FILE *f;

	if ((f = fopen("/dev/console", "w")) != NULL) {
		(void) fprintf(f, "%s.\n", s);
		(void) fclose(f);
	}
	
}

/*
 * This sends a (current version) ypbind "Set domain" message back to our
 * parent.  The version embedded in the protocol message is that which is passed
 * to us as a parameter.
 */
void
ypbind_pipe_setdom(dp, dom, addr, port, vers)
	struct domain *dp;
	char *dom;
	struct in_addr addr;
	unsigned short int port;
	unsigned short int vers;
{
	struct ypbind_setdom req;

	strcpy(req.ypsetdom_domain, dom);
	req.ypsetdom_addr = addr;
	req.ypsetdom_port = port;
	req.ypsetdom_vers = vers;
	xdr_ypbind_setdom(&(dp->broadcaster_xdr),&req);
	xdr_destroy(&(dp->broadcaster_xdr));
	fclose(dp->broadcaster_pipe);
	close(dp->broadcaster_fd);
	dp->broadcaster_fd = -1;
	dp->broadcaster_pipe=(FILE *) NULL;
}

/*
 * This sets the internet address and port for the passed domain to the
 * passed values, and marks the domain as supported.  This accepts both the
 * old style message (in which case the version is assumed to be that of the
 * requestor) or the new one, in which case the protocol version is included
 * in the protocol message.  This allows our child process (which speaks the
 * current protocol) to look for NIS servers on behalf old-style clients.
 */
void
ypbind_set_binding(rqstp, transp, vers)
	struct svc_req *rqstp;
	register SVCXPRT *transp;
	unsigned short vers;
{
	struct ypbind_setdom req;
	struct ypbind_oldsetdom oldreq;
	unsigned short version;
	struct in_addr addr;
	struct sockaddr_in *who; 
	unsigned short int port;
	char *domain;
	int i;          /* DEC -S security option */

	if (vers == YPVERS) {

		if (!svc_getargs(transp, xdr_ypbind_setdom, &req) ) {
			svcerr_decode(transp);
			return;
		}

		version = req.ypsetdom_vers;
		addr = req.ypsetdom_addr;
		port = req.ypsetdom_port;
		domain = req.ypsetdom_domain;
	} else {

		if (!svc_getargs(transp, _xdr_ypbind_oldsetdom, &oldreq) ) {
			svcerr_decode(transp);
			return;
		}

		version = vers;
		addr = oldreq.ypoldsetdom_addr;
		port = oldreq.ypoldsetdom_port;
		domain = oldreq.ypoldsetdom_domain;
	}

	/* find out who originated the request */
	who = svc_getcaller(transp);

	if (setok == FALSE) {
	fprintf(stderr, "ypbind: Set domain request to host %s, ",
		inet_ntoa(addr));
	fprintf(stderr, "from host %s, failed (ypset not allowed)!\n",
		inet_ntoa(who->sin_addr));
	svcerr_systemerr(transp);
	return;
	}
		
	/* This code implements some restrictions on who can set the	*
 	 * NIS server for this host 					*/

	/* This policy is that root can set the NIS server to anything, *
     	 * everyone else can't. This should also check for a valid NIS 	*
	 * server but time is short, 4.1 for sure			*/


   	if (ntohs(who->sin_port) > IPPORT_RESERVED) {
		fprintf(stderr,"ypbind: Set domain request to host %s, ",
			inet_ntoa(addr));
		fprintf(stderr,"from host %s, failed (bad port).\n",
			inet_ntoa(who->sin_addr));
		svcerr_systemerr(transp);
		return;
	}

	if (setok == YPSETLOCAL) {
		if (!chklocal(who->sin_addr)) {

		fprintf(stderr,"ypbind: Set domain request to host %s, ",
			inet_ntoa(addr));
		fprintf(stderr,"from host %s, failed (not local).\n",
			inet_ntoa(who->sin_addr));
		svcerr_systemerr(transp);
		return;
	}
	}
	/* Now check the credentials */
	if (rqstp->rq_cred.oa_flavor == AUTH_UNIX) {
		if (((struct authunix_parms *)rqstp->rq_clntcred)->aup_uid != 0) {
			fprintf(stderr,"ypbind: Set domain request to host %s,",
					inet_ntoa(addr));
			fprintf(stderr," from host %s, failed (not root).\n", 
					inet_ntoa(who->sin_addr));
			svcerr_systemerr(transp);
			return;
		}
	} else {
		fprintf(stderr, "ypbind: Set domain request to host %s,",
				inet_ntoa(addr));
		fprintf(stderr," from host %s, failed (credentials).\n", 
				inet_ntoa(who->sin_addr));
		svcerr_weakauth(transp);
		return;
	}

	if (!svc_sendreply(transp, xdr_void, 0) ) {
		fprintf(stderr, "ypbind:  Can't reply to rpc call.\n");
	}

	if(locked)    /* DEC's -S security option */
                {
                if(strcmp(setdomain,domain) == 0)
                        {
                        if((current_domain = ypbind_point_to_domain(domain,version)) != (struct domain *) NULL)
                        for (i=0;i<locked;i++)
                                {
                                sin = &sock[i];
                                if(sin->sin_addr.s_addr == addr.s_addr)
                                        {
                                        ypbind_locked(i,version);
                                        }
                                }
                        }
                }  /* End DEC's -S security option */
        else if ( (current_domain = ypbind_point_to_domain(domain,
	    version) ) != (struct domain *) NULL) {
		current_domain->dom_serv_addr = addr;
		current_domain->dom_serv_port = port;
		current_domain->dom_boundp = TRUE;
		current_domain->lastping.tv_sec = 0;
		current_domain->lastping.tv_usec = 0; /*require ping*/
		/* get rid of old pinging client if one exists */
		if (current_domain->ping_clnt != (CLIENT *)NULL) {
			clnt_destroy(current_domain->ping_clnt);
			current_domain->ping_clnt = (CLIENT *)NULL;
		}
		ypbind_get_binding(rqstp, transp, vers, TRUE, domain);
		/* make sure this binding is really set in binding file */
	}
}
/*
 * This returns a pointer to a domain entry.  If no such domain existed on
 * the list previously, an entry will be allocated, initialized, and linked
 * to the list.  Note:  If no memory can be malloc-ed for the domain structure,
 * the functional value will be (struct domain *) NULL.
 */
static struct domain *
ypbind_point_to_domain(pname, vers)
	register char *pname;
	unsigned short vers;
{
	register struct domain *pdom;
	
	for (pdom = known_domains; pdom != (struct domain *)NULL;
	    pdom = pdom->dom_pnext) {
		if (!strcmp(pname, pdom->dom_name) && vers == pdom->dom_vers)
			return (pdom);
	}
	
	/* Not found.  Add it to the list */
	
	if (pdom = (struct domain *)malloc(sizeof (struct domain))) {
		pdom->dom_pnext = known_domains;
		known_domains = pdom;
		strcpy(pdom->dom_name, pname);
		pdom->dom_vers = vers;
		pdom->dom_boundp = FALSE;
		pdom->ping_clnt = (CLIENT *)NULL;
		pdom->dom_report_success = -1;
		pdom->dom_broadcaster_pid = 0;
		pdom->bindfile = -1;
		pdom->broadcaster_fd = -1;
		pdom->broadcaster_pipe = (FILE *)NULL;
	}
	
	return (pdom);
}

/*
 * This is called by the broadcast rpc routines to process the responses 
 * coming back from the broadcast request. Since the form of the request 
 * which is used in ypbind_broadcast_bind is "respond only in the positive  
 * case", we know that we have a server.  If we should be running secure,
 * return FALSE if this server is not using a reserved port.  Otherwise,
 * the internet address of the responding server will be picked up from
 * the saddr parameter, and stuffed into the domain.  The domain's boundp
 * field will be set TRUE.  The first responding server (or the first one
 * which is on a reserved port) will be the bound server for the domain.
 */
bool
ypbind_broadcast_ack(ptrue, saddr)
	bool *ptrue;
	struct sockaddr_in *saddr;
{
  int i;     /* DEC's -S security option */

#ifdef SUN_S
	/* if we should be running secure and the server is not using
	 * a reserved port, return FALSE
	 */
	if (secure && (saddr->sin_family != AF_INET ||
	    saddr->sin_port >= IPPORT_RESERVED) ) {
		return (FALSE);
	}
#endif SUN_S
	
	/* if ypbind is in locked mode only bind to the selected servers */
        /* This code should never be hit but just in case its here  */
        if(locked)
                {
                if ( (current_domain = 
			ypbind_point_to_domain(setdomain,YPVERS) ) !=
            		(struct domain *) NULL)
                        exit(1);

                for (i=0;i<locked;i++)
                        {
                        sin = &sock[i];
                        if(sin->sin_addr.s_addr == saddr->sin_addr.s_addr)
                                {
                                ypbind_locked(i,YPVERS);
                                }
                        }
	      }
        else    {
	current_domain->dom_boundp = TRUE;
	current_domain->dom_serv_addr = saddr->sin_addr;
	current_domain->dom_serv_port = saddr->sin_port;
      }
	gettimeofday(&(current_domain->lastping),0);
	return(TRUE);
}

/*
 * This checks to see if a server bound to a named domain is still alive and
 * well.  If he's not, boundp in the domain structure is set to FALSE.
 */
bool
ypbind_ping(pdom)
	struct domain *pdom;

{
	struct sockaddr_in addr;
	enum clnt_stat clnt_stat;
	struct timeval timeout;
	struct timeval intertry;
	bool	new_binding = FALSE;
	char *pname;
	int true = FALSE;
	

	timeout.tv_sec = PINGTOTTIM;
	timeout.tv_usec = intertry.tv_usec = 0;
	if (pdom->ping_clnt == (CLIENT *)NULL) {
		new_binding = TRUE;
		intertry.tv_sec = PINGINTERTRY;
		addr.sin_addr = pdom->dom_serv_addr;
		addr.sin_family = AF_INET;
		addr.sin_port = pdom->dom_serv_port;
		bzero(addr.sin_zero, 8);
		if ((pdom->ping_clnt = clntudp_bufcreate(&addr, YPPROG,
		    pdom->dom_vers, intertry, &ping_sock,
		    RPCSMALLMSGSIZE, RPCSMALLMSGSIZE)) == (CLIENT *)NULL) {
			clnt_pcreateerror("ypbind_ping -- clntudp_create error");
			pdom->dom_boundp = FALSE;
			return (new_binding);
		} else {
			pdom->dom_serv_port = addr.sin_port;
		}
	}
	pname = pdom->dom_name;
	if ((clnt_stat = (enum clnt_stat) clnt_call(pdom->ping_clnt,
	    YPPROC_DOMAIN, xdr_ypdomain_wrap_string, &pname, xdr_int, &true,
	    timeout))
	    != RPC_SUCCESS || (true != TRUE)) {
		new_binding = TRUE;
		pdom->dom_boundp = FALSE;
		clnt_destroy(pdom->ping_clnt);	
		pdom->ping_clnt = (CLIENT *)NULL;
	}
	gettimeofday(&(pdom->lastping),0);
	return (new_binding);
}

#ifdef INIT_DEFAULT
/*
 * Preloads the default domain's domain binding. Domain binding for the
 * local node's default domain for both the current version, and the
 * previous version will be set up.  Bindings to servers which serve the
 * domain for both versions may additionally be made.  
 */
static void
ypbind_init_default()
{
	char domain[256];
	char *pname = domain;
	int true;
	int binding_tries;

	if (getdomainname(domain, 256) == 0) {
		current_domain = ypbind_point_to_domain(domain, YPVERS);

		if (current_domain == (struct domain *) NULL) {
			abort();
		}
		
		for (binding_tries = 0;
		    ((!current_domain->dom_boundp) &&
		    (binding_tries < BINDING_TRIES) ); binding_tries++) {
			(void) clnt_broadcast(YPPROG, current_domain->dom_vers,
			    YPPROC_DOMAIN_NONACK, xdr_ypdomain_wrap_string,
			    &pname, xdr_int, &true, ypbind_broadcast_ack);
			
		}
		
		current_domain = ypbind_point_to_domain(domain, YPOLDVERS);

		if (current_domain == (struct domain *) NULL) {
			abort();
		}
		
		for (binding_tries = 0;
		    ((!current_domain->dom_boundp) &&
		    (binding_tries < BINDING_TRIES) ); binding_tries++) {
			(void) clnt_broadcast(YPPROG, current_domain->dom_vers,
			    YPPROC_DOMAIN_NONACK, xdr_ypdomain_wrap_string,
			    &pname, xdr_int, &true, ypbind_broadcast_ack);
			
		}
	}
}
#endif

int
chklocal(taddr)
	struct in_addr taddr;
{
	struct in_addr addrs;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	struct sockaddr_in *sin;
	int n, i,j, sock;
	char buf[UDPMSGSIZE];

	ifc.ifc_len = UDPMSGSIZE;
	ifc.ifc_buf = buf;
	sock=socket(PF_INET,SOCK_DGRAM,0);
	if (sock<0) return(FALSE);
	if (ioctl(sock, SIOCGIFCONF, (char *)&ifc) < 0) {
		perror("SIOCGIFCONF");
		close(sock);
		return (FALSE);
	}
	ifr = ifc.ifc_req;
	j=0;
	for (i = 0, n = ifc.ifc_len/sizeof (struct ifreq); n > 0; n--, ifr++) {
		ifreq = *ifr;
		if (ioctl(sock, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
			perror("SIOCGIFFLAGS");
			continue;
		}
		if ((ifreq.ifr_flags & IFF_UP) && ifr->ifr_addr.sa_family == AF_INET) {
			sin = (struct sockaddr_in *)&ifr->ifr_addr;
			if (ioctl(sock, SIOCGIFADDR, (char *)&ifreq) < 0) {
			perror("SIOCGIFADDR");
			} else {

				addrs = ((struct sockaddr_in*)
				&ifreq.ifr_addr)->sin_addr;
				if (bcmp((char *)&taddr,(char *)&addrs,sizeof(addrs))==0)
				{
				close(sock);
				return(TRUE);
				}

			}
		}
	}
	close(sock);
	return (FALSE);
}
#ifdef LOCK_EX
#define LOCKFILE "/etc/ypbind.lock"
interlock()
{
pid_t pid;
int len;
int status;
int fd;
fd=open(LOCKFILE,O_RDWR+O_CREAT,0600);
if (fd<0) {
	(void) fprintf(stderr,"ypbind can't create/open lock file -- ");
	perror(LOCKFILE);
	abort();
	}

if (fd<3) {
	if (dup2(fd,8)>0) fd=8;
	}

if (flock(fd,LOCK_EX|LOCK_NB)<0) {
	fprintf(stderr,"ypbind is already running -- ");
	perror(LOCKFILE);
	exit (-1);
	}


interlock_fd = fd;
}
#endif
unregister(x)
int x;
{
int mask;
	(void) pmap_unset(YPBINDPROG, YPBINDVERS);
	(void) pmap_unset(YPBINDPROG, YPBINDOLDVERS);
	signal(x,SIG_DFL);
	fprintf(stderr,"ypbind: going down on signal %d\n",x);
	mask=sigblock(sigmask(SIGCHLD)|sigmask(SIGTERM));
	killpg(getpid(),SIGTERM); /*kill children*/
	sigsetmask(mask & ~(sigmask (x))); /*allow the exception*/
	kill(getpid(),x);
	sleep(2);
	exit(-1);
}

/* Digital's -S security option */

ypbind_locked(srvpref,version)
     int srvpref;
     unsigned short version;
{
  struct dom_binding domb;
  enum clnt_stat clnt_stat;
  int i,j,done,true;
  char domain[256];
  char *pname = setdomain;

  if (getdomainname(domain, 256) == 0) {

    /* Exit if the default domainname is unspecified */
    if (strcmp(domain,"") == 0) {
      (void) fprintf(stderr,
		     "ypbind exiting: default domainname not specified\n");
      (void) fflush(stderr);
      exit(1);
    }

    current_domain = ypbind_point_to_domain(domain, version);
    current_domain->dom_boundp = FALSE;
  }

  /* check if there is a preferred local server */
  if(srvpref != -1)
    i=srvpref;
  else i=0;

  /* Loop until we find a server to bind to.
   */

  done=0;
  true=0;

  while(!done)
    {
      sin = &sock[i];
      i = (i+1) % locked;
      domb.dom_server_addr.sin_addr = sin->sin_addr;
      domb.dom_server_addr.sin_family = AF_INET;
      domb.dom_server_addr.sin_port = htons((u_short) 0);
      domb.dom_server_port = htons((u_short) 0);
      domb.dom_socket = RPC_ANYSOCK;
      if (domb.dom_client = 
	  clntudp_create(&(domb.dom_server_addr),
			 YPPROG, version, udp_intertry, &(domb.dom_socket)))
	{
	  if((clnt_stat = (enum clnt_stat) 
	      clnt_call(domb.dom_client, 
			YPPROC_DOMAIN,xdr_ypdomain_wrap_string,&pname,xdr_int,
			&true,udp_timeout)) == RPC_SUCCESS)
	    if(true)
	      {
		done=1;
		current_domain->dom_boundp = TRUE;
		current_domain->dom_serv_addr = sin->sin_addr;
		current_domain->dom_serv_port = sin->sin_port;
		current_domain->ping_clnt = (CLIENT *)NULL;
	      }
	  clnt_destroy(domb.dom_client);
	  close(domb.dom_socket);
	}
    }
}

