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
static char *rcsid = "@(#)$RCSfile: udp.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/17 20:06:11 $";
#endif
#ifndef lint
static char sccsid[] = "@(#)udp.c	1.3 90/11/09 NFSSRC4.0 1.14 88/06/11 Copyr 1984 Sun Micro";
#endif

	/*
	 * Copyright (c) 1988 by Sun Microsystems, Inc.
	 */

/*
 * this file consists of routines to support call_udp();
 * client handles are cached in a hash table;
 * clntudp_create is only called if (site, prog#, vers#) cannot
 * be found in the hash table;
 * a cached entry is destroyed, when remote site crashes
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netdb.h>
#define MAX_HASHSIZE 100


extern char *xmalloc();
static int mysock = RPC_ANYSOCK;
extern int debug;
extern int HASH_SIZE;

struct cache {
	char *host;
	int prognum;
	int versnum;
	int sock;
	CLIENT *client;
	struct cache *nxt;
};

struct cache *ctable[MAX_HASHSIZE];
int cache_len = sizeof (struct cache);

hash(name)
	unsigned char *name;
{
	int len;
	int i, c;

	c = 0;
	len = strlen(name);
	for (i = 0; i< len; i++) {
		c = c +(int) name[i];
	}
	c = c %HASH_SIZE;
	return (c);
}

/*
 * find_hash returns the cached entry;
 * it returns NULL if not found;
 */
struct cache *
find_hash(host, prognum, versnum)
	char *host;
	int prognum, versnum;
{
	struct cache *cp;

	cp = ctable[hash(host)];
	while ( cp != NULL) {
		if (strcmp(cp->host, host) == 0 &&
		 cp->prognum == prognum && cp->versnum == versnum) {
			/* found */
			return (cp);
		}
		cp = cp->nxt;
	}
	return (NULL);
}

struct cache *
add_hash(host, prognum, versnum)
	char *host;
	int prognum, versnum;
{
	struct cache *cp;
	int h;

	if ((cp = (struct cache *) xmalloc(cache_len)) == NULL ) {
		return (NULL);	/* malloc error */
	}
	if ((cp->host = xmalloc(strlen(host)+1)) == NULL ) {
		if (cp)
			free(cp);
		return (NULL);	/* malloc error */
	}
	(void) strcpy(cp->host, host);
	cp->prognum = prognum;
	cp->versnum = versnum;
	h = hash(host);
	cp->nxt = ctable[h];
	ctable[h] = cp;
	return (cp);
}

void
delete_hash(host)
	char *host;
{
	struct cache *cp;
	struct cache *cp_prev = NULL;
	struct cache *next;
	int h;

	/*
	 * if there is more than one entry with same host name;
	 * delete has to be recurrsively called
	 */

	h = hash(host);
	next = ctable[h];
	while ((cp = next) != NULL) {
		next = cp->nxt;
		if (strcmp(cp->host, host) == 0) {
			if (cp_prev == NULL) {
				ctable[h] = cp->nxt;
			}
			else {
				cp_prev->nxt = cp->nxt;
			}
			if (debug)
				printf("delete hash entry (%x), %s \n", cp, host);
			if (cp->client)
				clnt_destroy(cp->client);
			if (cp->host)
				free(cp->host);
			if (cp)
				free(cp);
		}
		else {
			cp_prev = cp;
		}
	}
}

call_udp(host, prognum, versnum, procnum, inproc, in, outproc, out, valid_in, t)
	char *host;
	xdrproc_t inproc, outproc;
	char *in, *out;
	int valid_in;
	int t;
{
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct hostent *hp;
	struct timeval timeout, tottimeout;
	struct cache *cp;

	/* Get a long lived socket to talk to the status monitor with */
	if (mysock == RPC_ANYSOCK) {
		int	dontblock = 1;
		mysock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (mysock < 0) {
			fprintf(stderr,
				"rpc.lockd : cannot send because socket could not be created.\n");
			return (-1);
		}
		/* attempt to bind to prov port */
		(void)bindresvport(mysock, (struct sockaddr_in *)0);
		/* the sockets rpc controls are non-blocking */
		(void)ioctl(mysock, FIONBIO, (char *) &dontblock);
	}
	if ((cp = find_hash(host, prognum, versnum)) == NULL) {
		if ((cp = add_hash(host, prognum, versnum)) == NULL) {
			fprintf(stderr, "udp cannot send due to out of cache\n");
			return (-1);
		}
		if (debug)
			printf("(%x):[%s, %d, %d] is a new connection\n", cp, host, prognum, versnum);

		if ((hp = gethostbyname(host)) == NULL) {
			delete_hash(host);
			return ((int) RPC_UNKNOWNHOST);
		}
		timeout.tv_usec = 0;
		timeout.tv_sec = 15;
		bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
		server_addr.sin_family = AF_INET;
		server_addr.sin_port =  0;
		if ((cp->client = clntudp_create(&server_addr, prognum,
			versnum, timeout, &mysock)) == NULL) {
			delete_hash(host);
			/*
			 * this is a quick/safe (?) fix as the above layer
			 * doesn't have recovery code
			 * should return ((int) rpc_createerr.cf_stat);
			 * Ejw: Now return portmap problems, those are handled.
			 */
			if (rpc_createerr.cf_stat == RPC_PMAPFAILURE ||
			    rpc_createerr.cf_stat == RPC_PROGNOTREGISTERED)
				return ((int) rpc_createerr.cf_stat);
			return ((int) RPC_TIMEDOUT);
		}

	}
	else {
		if (valid_in == 0) { /* cannot use cache */
			if (debug)
				printf("(%x):[%s, %d, %d] is a new connection\n",
					cp, host, prognum, versnum);
			if ((hp = gethostbyname(host)) == NULL) {
				delete_hash(host);
				return ((int) RPC_UNKNOWNHOST);
			}
			/* get rid of previous client struct */
			if (cp->client != NULL) clnt_destroy(cp->client);
			timeout.tv_usec = 0;
			timeout.tv_sec = 15;
			bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
			server_addr.sin_family = AF_INET;
			server_addr.sin_port =  0;
			if ((cp->client = clntudp_create(&server_addr, prognum,
				versnum, timeout, &mysock)) == NULL) {

				delete_hash(host);
				/*
			 	 * this is a quick/safe(?) fix as the above
			 	 * layer doesn't have recovery code
			 	 * should return ((int) rpc_createerr.cf_stat);
				 * Ejw: Now return portmap problems.
			 	 */
				if (rpc_createerr.cf_stat == RPC_PMAPFAILURE ||
				    rpc_createerr.cf_stat == RPC_PROGNOTREGISTERED)
					return ((int) rpc_createerr.cf_stat);
				return ((int) RPC_TIMEDOUT);
			}
		}
	}

	cp->sock = mysock;
	tottimeout.tv_sec = t;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(cp->client, procnum, inproc, in,
	    outproc, out, tottimeout);
	if (debug) printf("clnt_stat=%d\n", (int) clnt_stat);
	return ((int) clnt_stat);
}
