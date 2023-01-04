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
static char     *sccsid = "@(#)$RCSfile: yp_get_master.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/05/08 13:59:37 $";
#endif
/*
 */


/*this code is stolen from ypxfr.c */
/*this allows us to bind to the master server for a map
with _yp_dobind_master(domain, map, pdomb) and returns us
a pdomb. yppush needs to use the master server -bug 1015489 and 1016486*/ 

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/time.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypv1_prot.h>


static bool     ping_server();
extern bool	debug;


#define UDPINTER_TRY 10		/* Seconds between tries for udp */
#define UDPTIMEOUT UDPINTER_TRY*4	/* Total timeout for udp */
#define CLRTIMEOUT UDPINTER_TRY*2	/* Total timeout for clr */
#define CALLINTER_TRY 10	/* Seconds between callback tries */
#define CALLTIMEOUT CALLINTER_TRY*6	/* Total timeout for callback */


static struct timeval udp_intertry = {
	UDPINTER_TRY,
	0
};
static struct timeval udp_timeout = {
	UDPTIMEOUT,
	0
};
static struct timeval clr_timeout = {
	CLRTIMEOUT,
	0
};

static          bool
get_master_addr(master, master_addr)
	char           *master;
	struct in_addr *master_addr;
{
	struct in_addr  tempaddr;
	struct hostent *h;
	bool            error = FALSE;



	if (isdigit(*master)) {
		tempaddr.s_addr = inet_addr(master);

		if ((int) tempaddr.s_addr != -1) {
			*master_addr = tempaddr;
		} else {
			error = TRUE;
		}

	} else {

		if (h = gethostbyname(master)) {
			(void) bcopy(h->h_addr, (char *) master_addr,
				     h->h_length);
		} else {
			error = TRUE;
		}
	}

	if (error) {
		fprintf(stderr,
		 "Can't translate master name %s to an address.\n", master);
	}
	return (!error);
}

/*
 * This sets up a udp connection to speak the correct program and version to
 * a NIS server.  vers is set to one of YPVERS or YPOLDVERS to reflect which
 * language the server speaks.
 */
static          bool
bind_to_server(host, host_addr, pdomb, vers, status)
	char           *host;
	struct in_addr  host_addr;
	struct dom_binding *pdomb;
	unsigned int   *vers;
	int            *status;
{
	if (ping_server(host, host_addr, pdomb, YPVERS, status)) {
		*vers = YPVERS;
		return (TRUE);
	} else if (*status == YPPUSH_YPERR) {
		return (FALSE);
	} else {
		if (ping_server(host, host_addr, pdomb, YPOLDVERS, status)) {
			*vers = YPOLDVERS;
			return (TRUE);
		} else {
			return (FALSE);
		}
	}
}

/*
 * This sets up a UDP channel to a server which is assumed to speak an input
 * version of YPPROG.  The channel is tested by pinging the server.  In all
 * error cases except "Program Version Number Mismatch", the error is
 * reported, and in all error cases, the client handle is destroyed and the
 * socket associated with the channel is closed.
 */
static          bool
ping_server(host, host_addr, pdomb, vers, status)
	char           *host;
	struct in_addr  host_addr;
	struct dom_binding *pdomb;
	unsigned int    vers;
	int            *status;
{
	enum clnt_stat  rpc_stat;
#ifdef MONTE
	extern bool            run_secure;
#else /*MONTE*/
	bool	run_secure = FALSE;
#endif /*MONTE*/

	pdomb->dom_server_addr.sin_addr = host_addr;
	pdomb->dom_server_addr.sin_family = AF_INET;
	pdomb->dom_server_addr.sin_port = htons((u_short) 0);
	pdomb->dom_server_port = htons((u_short) 0);
	pdomb->dom_socket = RPC_ANYSOCK;

	if (pdomb->dom_client = clntudp_create(&(pdomb->dom_server_addr),
			YPPROG, vers, udp_intertry, &(pdomb->dom_socket))) {

		/*
		 * if we are on a c2 system, we should only accept data from
		 * a server which is on a reserved port.
		 */
		if (run_secure &&
		    (pdomb->dom_server_addr.sin_family != AF_INET ||
		     pdomb->dom_server_addr.sin_port >= IPPORT_RESERVED)) {
			clnt_destroy(pdomb->dom_client);
			close(pdomb->dom_socket);
			(void) fprintf(stderr, "bind_to_server: server is not using a privileged port\n");
			*status = YPPUSH_YPERR;
			return (FALSE);
		}
		rpc_stat = clnt_call(pdomb->dom_client, YPBINDPROC_NULL,
				     xdr_void, 0, xdr_void, 0, udp_timeout);

		if (rpc_stat == RPC_SUCCESS) {
			return (TRUE);
		} else {
			clnt_destroy(pdomb->dom_client);
			close(pdomb->dom_socket);

			if (rpc_stat != RPC_PROGVERSMISMATCH) {
				(void) clnt_perror(pdomb->dom_client,
				   "ypxfr: bind_to_server clnt_call error");
			}
			*status = YPPUSH_RPC;
			return (FALSE);
		}
	} else {
		fprintf(stderr, "bind_to_server clntudp_create error");
		(void) clnt_pcreateerror("");
		fflush(stderr);
		*status = YPPUSH_RPC;
		return (FALSE);
	}
}

int
static 
find_map_master(source, map, master)
	char           *source, *map, **master;
{
	int             err;

	if (err = yp_master(source, map, master)) {
		if (debug) fprintf(stderr, "Can't get master of %s. Reason: %s.\n", map,
			yperr_string(err));
	}
	return (err);

}

/* source,and map is inbound */
static          bool
bind_to_master(source, map, master, master_addr, pdomb, vers, status)
	char           *source, *map, **master;
	struct in_addr *master_addr;
	struct dom_binding *pdomb;
	unsigned int   *vers;
	int            *status;

{
	*status = YPPUSH_YPERR;
	if (find_map_master(source, map, master) != 0)
		return (FALSE);
	if (get_master_addr(*master, master_addr) == FALSE)
		return (FALSE);
	return (bind_to_server(*master, *master_addr, pdomb, vers, status));

}

/* This looks like _yp_dobind and is called by the 
routines yp_first_master and yp_next_master which yppush uses
to enumerate master copies of maps */

static struct dom_binding *last_domain;
static char    *last_map;
static char    *last_dname;
_yp_dobind_master(domain, map, pdomb)
	char           *domain, *map;
	struct dom_binding **pdomb;

{
	char           *strdup();
	struct in_addr  master_addr;
	char            master[256];
	int             status;
	int             vers;
	if (last_map && last_domain && last_dname) {
		if ((strcmp(domain, last_dname) == 0) && (strcmp(map, last_map) == 0)) {
			*pdomb = last_domain;
			return (0);
		}
	}
	*pdomb = (struct dom_binding *) calloc(1, sizeof(struct dom_binding));
	if ((*pdomb) == 0) {
		return (YPERR_RESRC);
	}
	if (bind_to_master(domain, map, master, &master_addr, *pdomb, &vers, &status) == FALSE) {
		return (status);
	}
	(*pdomb)->dom_vers = vers;
	last_domain = *pdomb;
	if (last_dname)
		free(last_dname);
	last_dname = strdup(domain);
	if (last_map)
		free(last_map);
	last_map = strdup(map);
	return (0);
}

_yp_unbind_master(pdomb)
	struct dom_binding *pdomb;
{
	if (pdomb == last_domain) {
		last_domain = 0;
		free(last_dname);
		last_dname = 0;
		free(last_map);
		last_map = 0;
	}
	clnt_destroy(pdomb->dom_client);
	close(pdomb->dom_socket);
	free(pdomb);
	return (0);

}

bool
yp_clear_master(domain,map)
	char *domain, *map;
{
	struct sockaddr_in myaddr;
	struct dom_binding *domb;
	char local_host_name[256];
	unsigned int progvers;
	int status;
	if ( _yp_dobind_master(domain, map, &domb)!= 0) {
		fprintf(stderr, "Can't bind master to send ypclear message to ypserv for map %s.\n",map);
		return (FALSE);
		}


	if((enum clnt_stat) clnt_call(domb->dom_client,
	    YPPROC_CLEAR, xdr_void, 0, xdr_void, 0,
	    clr_timeout) != RPC_SUCCESS) {
		fprintf(stderr, "Can't send ypclear message to ypserv on the master machine for map %s.\n",map);
		return (FALSE);
	}

	return (TRUE);
}

char *strdup(s1)
char *s1;
{
    char *s2;

    s2 = malloc(strlen(s1)+1);
    if (s2 != NULL)
        s2 = strcpy(s2, s1);
    return(s2);
}
