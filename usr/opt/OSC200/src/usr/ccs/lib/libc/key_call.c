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
static char     *sccsid = "@(#)$RCSfile: key_call.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:21:23 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.11 88/02/08 
 */

/*
 * key_call.c, Interface to keyserver
 *
 * setsecretkey(key) - set your secret key
 * encryptsessionkey(agent, deskey) - encrypt a session key to talk to agent
 * decryptsessionkey(agent, deskey) - decrypt ditto
 * gendeskey(deskey) - generate a secure des key
 * netname2user(...) - get unix credential for given name (kernel only)
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak key_decryptsession = __key_decryptsession
#pragma weak key_encryptsession = __key_encryptsession
#pragma weak key_gendes = __key_gendes
#pragma weak key_setsecret = __key_setsecret
#endif
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef socket
#undef socket
#endif
#ifdef stat
#undef stat
#endif
#endif

#ifdef KERNEL
#include "param.h"
#include "user.h"
#include "socket.h"
#include "../rpc/rpc.h"
#include "../rpc/key_prot.h"
#else
#include <sys/param.h>
#include <sys/socket.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#endif

#define KEY_TIMEOUT	5	/* per-try timeout in seconds */
#define KEY_NRETRY	12	/* number of retries */

#define debug(msg)		/* turn off debugging */

static struct timeval trytimeout = { KEY_TIMEOUT, 0 };
#ifndef KERNEL
static struct timeval tottimeout = { KEY_TIMEOUT * KEY_NRETRY, 0 };
#endif

#ifndef KERNEL
key_setsecret(secretkey)
	char *secretkey;
{
	keystatus status;

	if (!key_call((u_long)KEY_SET, xdr_keybuf, secretkey, xdr_keystatus, 
		(char*)&status)) 
	{
		return (-1);
	}
	if (status != KEY_SUCCESS) {
		debug("set status is nonzero");
		return (-1);
	}
	return (0);
}
#endif


key_encryptsession(remotename, deskey)
	char *remotename;
	des_block *deskey;
{
	cryptkeyarg arg;
	cryptkeyres res;

	arg.remotename = remotename;
	arg.deskey = *deskey;
	if (!key_call((u_long)KEY_ENCRYPT, 
		xdr_cryptkeyarg, (char *)&arg, xdr_cryptkeyres, (char *)&res)) 
	{
		return (-1);
	}
	if (res.status != KEY_SUCCESS) {
		debug("encrypt status is nonzero");
		return (-1);
	}
	*deskey = res.cryptkeyres_u.deskey;
	return (0);
}


key_decryptsession(remotename, deskey)
	char *remotename;
	des_block *deskey;
{
	cryptkeyarg arg;
	cryptkeyres res;

	arg.remotename = remotename;
	arg.deskey = *deskey;
	if (!key_call((u_long)KEY_DECRYPT, 
		xdr_cryptkeyarg, (char *)&arg, xdr_cryptkeyres, (char *)&res))  
	{
		return (-1);
	}
	if (res.status != KEY_SUCCESS) {
		debug("decrypt status is nonzero");
		return (-1);
	}
	*deskey = res.cryptkeyres_u.deskey;
	return (0);
}

key_gendes(key)
	des_block *key;
{
#ifdef KERNEL
	if (!key_call((u_long)KEY_GEN, xdr_void, (char *)NULL, xdr_des_block, 
		(char *)key)) 
	{
		return (-1);
	}
#else
	struct sockaddr_in sin;
	CLIENT *client;
	int socket;
	enum clnt_stat stat;

 
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	bzero(sin.sin_zero, sizeof(sin.sin_zero));
	socket = RPC_ANYSOCK;
	client = clntudp_bufcreate(&sin, (u_long)KEY_PROG, (u_long)KEY_VERS,
		trytimeout, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client == NULL) {
		return (-1);
	}
	stat = clnt_call(client, KEY_GEN, xdr_void, NULL,
		xdr_des_block, key, tottimeout);
	clnt_destroy(client);
	(void) close(socket);
	if (stat != RPC_SUCCESS) {
		return (-1);
	}
#endif
	return (0);
}
 

#ifdef KERNEL
netname2user(name, uid, gid, len, groups)
	char *name;
	uid_t *uid;
	gid_t *gid;
	int *len;
	int *groups;
{
	struct getcredres res;

	res.getcredres_u.cred.gids.gids_val = groups;
	if (!key_call((u_long)KEY_GETCRED, xdr_netnamestr, (char *)&name, 
		xdr_getcredres, (char *)&res)) 
	{
		debug("netname2user: timed out?");
		return (0);
	}
	if (res.status != KEY_SUCCESS) {
		return (0);
	}
	*uid = res.getcredres_u.cred.uid;
	*gid = res.getcredres_u.cred.gid;
	*len = res.getcredres_u.cred.gids.gids_len;
	return (1);
}
#endif

#ifdef KERNEL
static
key_call(procn, xdr_args, args, xdr_rslt, rslt)
	u_long procn;
	bool_t (*xdr_args)();	
	char *args;
	bool_t (*xdr_rslt)();
	char *rslt;
{
	struct sockaddr_in sin;
	CLIENT *client;
	enum clnt_stat stat;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	bzero(sin.sin_zero, sizeof(sin.sin_zero));
	if (getport_loop(&sin, (u_long)KEY_PROG, (u_long)KEY_VERS, 
		(u_long)IPPROTO_UDP) != 0) 
	{
		debug("unable to get port number for keyserver");
		return (0);
	}
	client = clntkudp_create(&sin, (u_long)KEY_PROG, (u_long)KEY_VERS, 
		KEY_NRETRY, u.u_cred);
	if (client == NULL) {
		debug("could not create keyserver client");
		return (0);
	}
	stat = clnt_call(client, procn, xdr_args, args, xdr_rslt, rslt, 
			 trytimeout);
	auth_destroy(client->cl_auth);
	clnt_destroy(client);
	if (stat != RPC_SUCCESS) {
		debug("keyserver clnt_call failed: ");
		debug(clnt_sperrno(stat));
		return (0);
	}
	return (1);
}

#else

#include <stdio.h>
#include <sys/wait.h>

 
static
key_call(proc, xdr_arg, arg, xdr_rslt, rslt)
	u_long proc;
	bool_t (*xdr_arg)();
	char *arg;
	bool_t (*xdr_rslt)();
	char *rslt;
{
	XDR xdrargs;
	XDR xdrrslt;
	FILE *fargs;
	FILE *frslt;
	void (*osigchild)();
	union wait status;
	int pid;
	int success;
	int ruid;
	int euid;
	static char *MESSENGER = "/usr/etc/keyenvoy";

	success = 1;
	osigchild = signal(SIGCHLD, SIG_IGN);

	/*
	 * We are going to exec a set-uid program which makes our effective uid
	 * zero, and authenticates us with our real uid. We need to make the 
	 * effective uid be the real uid for the setuid program, and 
	 * the real uid be the effective uid so that we can change things back.
	 */
	euid = geteuid();
	ruid = getuid();
	(void) setreuid(euid, ruid);
	pid = _openchild(MESSENGER, &fargs, &frslt);
	(void) setreuid(ruid, euid);
	if (pid < 0) {
		debug("open_streams");
		return (0);
	}
	xdrstdio_create(&xdrargs, fargs, XDR_ENCODE);
	xdrstdio_create(&xdrrslt, frslt, XDR_DECODE);

	if (!xdr_u_long(&xdrargs, &proc) || !(*xdr_arg)(&xdrargs, arg)) {
		debug("xdr args");
		success = 0; 
	}
	(void) fclose(fargs);

	if (success && !(*xdr_rslt)(&xdrrslt, rslt)) {
		debug("xdr rslt");
		success = 0;
	}

	(void) fclose(frslt); 
	if (wait4(pid, &status, 0, NULL) < 0 || status.w_retcode != 0) {
		debug("wait4");
		success = 0; 
	}
	(void)signal(SIGCHLD, osigchild);

	return (success);
}
#endif
