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
static char	*sccsid = "@(#)$RCSfile: kls_ipc.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/19 19:17:05 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak kls_client_ipc_connect_to_server = __kls_client_ipc_connect_to_server
#pragma weak kls_client_ipc_disconnect_from_server = __kls_client_ipc_disconnect_from_server
#pragma weak kls_client_ipc_receive_reply = __kls_client_ipc_receive_reply
#pragma weak kls_client_ipc_send_request = __kls_client_ipc_send_request
#endif
#endif
#ifdef	_KLS_IPC_SOCK_DGRAM

/*
 * kls_ipc_sock_dgram.c
 *
 * This file implements the Kernel Load Server (KLS) IPC primitives
 * using UNIX domain datagram sockets.  The primities themselves are
 * described in kls_ipc.h.  This file implements both the client and
 * the server primitives.  The code in this file is to be linked into
 * each client and the server.  Although each does not need the other's
 * code, it helps with development and maintenance to keep all the KLS
 * IPC code for UNIX domain datagram sockets in one file.
 * The implementation of these primitives is fairly straightforward
 * and the code below should provide an adequate description.
 *
 *
 * Here are a few points about this implementation.  All sockets are
 * bound to a name.  We use chmod(2) to give read/write access only to
 * the owner, which is expected to be ROOT.
 *
 * Each client and the server are expected to send and/or receive only
 * one message at a time.  Single static buffers are used to hold the
 * messages.  The server code will always prints error messages when
 * system calls fail.  The client code never prints error messages
 * when system calls fail, it simply  returns the appropriate error
 * status.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdio.h>
#include <loader.h>
#include <errno.h>
#include <string.h>

#include <loader/kloadsrv.h>
#include <loader/kls_ipc.h>

#include <malloc.h>
#include <assert.h>

#define		MACRO_BEGIN	do {
#define		MACRO_END	} while (0)

#define	SERVER_SOCKET_NAME	"/dev/kloadsrv"

#define	TRUE	1

#define	dprintf(x) \
	MACRO_BEGIN \
		if (_kls_debug_level > 1) \
			(void) printf x ; \
	MACRO_END

int _kls_debug_level;

		    
/************************ Client Static Data ************************/

static struct sockaddr_un server_address;
static int                client_socket;
static struct sockaddr_un client_address;
static int                client_connected_to_server;
static char               *client_receive_buffer = NULL;


/******************** Client Exported Functions ********************/


int
kls_client_ipc_connect_to_server()
{
	/* get a socket */
	dprintf(("kls_client_ipc_connect_to_server: calling socket()\n"));
	if ((client_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		dprintf(("kls_client_ipc_connect_to_server: socket() failed: %s\n",
			strerror(errno)));
		return(-errno);
	}

	/* next bind name to socket */
	client_address.sun_family = AF_UNIX;
	(void)tmpnam(client_address.sun_path);
	(void)unlink(client_address.sun_path);
	dprintf(("kls_client_ipc_connect_to_server: calling bind(\"%s\")\n",
		client_address.sun_path));
	if (bind(client_socket, &client_address, sizeof(client_address)) == -1) {
		dprintf(("kls_client_ipc_connect_to_server: bind(\"%s\") failed: %s\n",
			client_address.sun_path, strerror(errno)));
		(void)close(client_socket);
		return(-errno);
	}

	/* next change access mode on name associated with socket */
	dprintf(("kls_client_ipc_connect_to_server: calling chmod(\"%s\", 0600)\n",
		client_address.sun_path));
	if (chmod(client_address.sun_path, 0600) == -1) {
		dprintf(("kls_client_ipc_connect_to_server: chmod(\"%s\", 0600) failed: %s\n",
			client_address.sun_path, strerror(errno)));
		(void)close(client_socket);
		(void)unlink(client_address.sun_path);
		return(-errno);
	}
	/* set-up server address */
	server_address.sun_family = AF_UNIX;
	(void)strcpy(server_address.sun_path, SERVER_SOCKET_NAME);

	/* mark in KLS IPC connected state */
	client_connected_to_server = 1;
	return(0);
}

int
kls_client_ipc_disconnect_from_server()
{
	int rc;

	if (!client_connected_to_server)
		return(0);

	(void)close(client_socket);
	(void)unlink(client_address.sun_path);
	return(0);
}

int
kls_client_ipc_send_request(p)
	kls_request_header_t *p;
{
	if (!client_connected_to_server) {
		errno = ENOTCONN;
		return(-errno);
	}

	dprintf(("kls_client_ipc_send_request: calling sendto()\n"));
	if (sendto(client_socket, (char *)p, p->klsi_msg_size, 0,
		&server_address, sizeof(server_address)) == -1) {
		dprintf(("kls_client_ipc_send_request: sendto() failed: %s\n",
			strerror(errno)));
		return(-errno);
	}
	return(0);
}

int
kls_client_ipc_receive_reply(pp)
	kls_reply_header_t **pp;
{
	if (!client_connected_to_server) {
		errno = ENOTCONN;
		return(-errno);
	}

	dprintf(("kls_client_ipc_receive_reply: calling recvfrom()\n"));
	if(client_receive_buffer == NULL) {
                client_receive_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_receive_buffer);
        }
	if (recvfrom(client_socket, client_receive_buffer,
	    KLS_MAX_MESSAGE_SIZE, 0, (struct sockaddr_un *)0,
	    0) == -1) {
		dprintf(("kls_client_ipc_receive_reply: recvfrom() failed: %s\n",
			strerror(errno)));
		return(-errno);
	}
	*pp = (kls_reply_header_t *)client_receive_buffer;
	return(0);
}

#else	/* _KLS_IPC_SOCK_DGRAM */

#ifdef	_KLS_IPC_SYSV_MSG

/*
 * kls_ipc_sysV_msg.c
 *
 * This file implements the Kernel Load Server (KLS) IPC primitives
 * using System V messages.  The primities themselves are described in
 * kls_ipc.h.  This file implements both the client and  the server
 * primitives.  The code in this file is to be linked into each client
 * and the server.  Although each does not need the other's code, it
 * helps with development and maintenance to keep all the KLS IPC code
 * for System V messages in one file.  The implementation of these
 * primitives is fairly straightforward and the code below should
 * provide an adequate description.
 *
 * Here are a few points about this implementation.  Each client and
 * the server must be run with the same UID and be owners of all
 * message queues.  That UID is expected to be ROOT.  The only
 * permission granted on message queues is to the owners.  In order
 * for the server to reply to the client, the client must send the
 * server the key to a message queue, upon which the client will wait
 * for a reply.  The client, during the initial connect, may loop to
 * find a free key.  The key is typically the PID of the client.  The
 * server is expected to do a msgget(2) on that reply key and then
 * msgsnd(2) the reply.
 *
 * Each client and the server are expected to send and/or receive only
 * one message at a time.  Single static buffers are used to hold the
 * messages.  The server code will always prints error messages when
 * system calls fail.  The client code never prints error messages
 * when system calls fail, it simply  returns the appropriate error
 * status.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdio.h>
#include <loader.h>
#include <errno.h>

#include <loader/kloadsrv.h>
#include <loader/kls_ipc.h>

#include <malloc.h>
#include <assert.h>

#define		MACRO_BEGIN	do {
#define		MACRO_END	} while (0)

#define	TRUE	1

#define	dprintf(x) \
	MACRO_BEGIN \
		if (_kls_debug_level > 1) \
			(void) printf x ; \
	MACRO_END

int _kls_debug_level;

static int   client_msgget_server(void);
static int   client_rmid_reply(void);

static int   client_get_server_key(key_t *);
static int   get_server_key(key_t *);

extern key_t ftok(char *, char);
			  

/************************ Client Static Data ************************/

static int   client_server_msqid;
static int   client_reply_msqid;
static key_t client_reply_key;
static int   client_connected_to_server;
static char  *client_receive_buffer = NULL;


/******************** Client Exported Functions ********************/

int
kls_client_ipc_connect_to_server()
{
	int rc;

	if ((rc = client_msgget_server()) < 0)
		return(rc);
	if ((rc = client_msgget_reply()) < 0)
		return(rc);
	client_connected_to_server = 1;
	return(0);
}

int
kls_client_ipc_disconnect_from_server()
{
	int rc;

	if (!client_connected_to_server)
		return(0);

	/*
	 * Sigh, doesn't seem to be anyway to undo the
	 * msgget() to the server, so there isn't a
	 * client_rmid_server() function.
	 */

	rc = client_rmid_reply();
	client_connected_to_server = 0;
	return(rc);
}

int
kls_client_ipc_send_request(p)
	kls_request_header_t *p;
{
	if (!client_connected_to_server) {
		errno = ENOTCONN;
		return(-errno);
	}

	p->klsi_reply_key = client_reply_key;

	dprintf(("kls_client_ipc_send_request: calling msgsnd()\n"));
	if (msgsnd(client_server_msqid, p, p->klsi_msg_size, 0) == -1) {
		dprintf(("kls_client_ipc_send_request: msgsnd() failed: %s\n",
			strerror(errno)));
		return(-errno);
	}
	return(0);
}

int
kls_client_ipc_receive_reply(pp)
	kls_reply_header_t **pp;
{
	if (!client_connected_to_server) {
		errno = ENOTCONN;
		return(-errno);
	}

	dprintf(("kls_client_ipc_receive_reply: calling msgrcv()\n"));
	if(client_receive_buffer == NULL) {
                client_receive_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_receive_buffer);
        }
	if (msgrcv(client_reply_msqid, client_receive_buffer,
	    KLS_MAX_MESSAGE_SIZE, 0, 0) == -1) {
		dprintf(("kls_client_ipc_receive_reply: msgrcv() failed: %s\n",
			strerror(errno)));
		return(-errno);
	}
	*pp = (kls_reply_header_t *)client_receive_buffer;
	return(0);
}
/****************** Internal Client IPC Functions ******************/

static int
client_get_server_key(kp)
	key_t *kp;
{
	return(get_server_key(kp));
}

static int
client_msgget_server()
{
	int msqid, rc;
	key_t key;

	if ((rc = client_get_server_key(&key)) < 0)
		return(rc);
	dprintf(("client_msgget_server: calling msgget()\n"));
	if ((msqid = msgget(key, MSG_W)) == -1) {
		dprintf(("client_msgget_server: msgget() failed: %s\n",
			strerror(errno)));
		return(-errno);
	}
	client_server_msqid = msqid;
	return(0);
}

int
client_msgget_reply()
{
	int msqid;
	key_t key;

	key = (key_t)getpid();
	while (TRUE) {
		dprintf(("client_msgget_reply: calling msgget(%d)\n",
			key));
		if ((msqid = msgget(key, (IPC_CREAT|IPC_EXCL|MSG_R|MSG_W))) != -1)
			break;
		dprintf(("client_msgget_reply: msgget(%d) failed: %s\n",
			key, strerror(errno)));
		if ((msqid == -1) && (errno == ENOSPC))
			return(-errno);
		key = (key_t)(((int)key)+1);
	}
	client_reply_key = key;
	client_reply_msqid = msqid;
	return(0);
}

static int
client_rmid_reply()
{
	int rc;

	dprintf(("client_rmid_reply: calling msgctl(IPC_RMID)\n"));
	if (msgctl(client_reply_msqid, IPC_RMID, (struct msqid_ds *)0) == -1) {
		dprintf(("client_rmid_reply: msgctl(IPC_RMID) failed: %s\n",
			strerror(errno)));
		rc = -errno;
	} else
		rc = 0;
	client_reply_key = 0;
	client_reply_msqid = 0;
	return(rc);
}

		     
/******************** Common Internal Functions ********************/

static int
get_server_key(kp)
	key_t *kp;
{
	key_t key;

	dprintf(("%s: calling ftok(\"%s\", \'%c\')\n",
		KLS_SERVER_NAME, SERVER_FTOK_PATH, SERVER_FTOK_ID));
	errno = 0;
	if ((key = ftok(SERVER_FTOK_PATH, SERVER_FTOK_ID)) == ((key_t)-1)) {
		if (errno)
			return(-errno);
		else
			return(-1);
	}
	*kp = key;
	return(0);
}

#endif	/* _KLS_IPC_SYSV_MSG */
#endif	/* _KLS_IPC_SOCK_DGRAM */
