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
static char	*sccsid = "@(#)$RCSfile: kls_ipc_sysV_msg.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:53 $";
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
 * kls_ipc_sysV_msg.c
 *
 * This file implements the Kernel Load Server (KLS) IPC primitives
 * using System V messages.  The primities themselves are described in
 * kls_ipc.h.  This file implements only the server primitives.
 * The code in this file is to be linked into the server.  The implementation
 * of these primitives is fairly straightforward and the code below should
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
 * system calls fail.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdio.h>
#include <loader.h>
#include <errno.h>

#include <loader/kloadsrv.h>

#include "kls_ipc.h"
#include "ldr_macro_help.h"


#define	SERVER_FTOK_PATH	KLS_SERVER_PATHNAME
#define	SERVER_FTOK_ID	'A'

#define	TRUE	1

#define	dprintf(x) \
	MACRO_BEGIN \
		if (kls_debug_level > 1) \
			(void) printf x ; \
	MACRO_END

int kls_debug_level;


static int   server_get_server_key(key_t *);
static int   get_server_key(key_t *);

extern key_t ftok(char *, char);
			  

/************************ Server Static Data ************************/

static int   server_msqid;
static char  server_receive_buffer[KLS_MAX_MESSAGE_SIZE];

		     
/******************** Server Exported Functions ********************/

int
kls_server_ipc_initialize()
{
	int msqid, rc;
	key_t key;

	/* get server key */
	if ((rc = server_get_server_key(&key)) < 0)
		return(rc);

	/* msgget() the queue associated with server key */
	if ((msqid = msgget(key, (MSG_R|MSG_W))) != -1)
		(void)msgctl(msqid, IPC_RMID, 0);
	dprintf(("%s: kls_server_ipc_init: calling msgget()\n",
		KLS_SERVER_NAME));
	if ((msqid = msgget(key, (IPC_CREAT|IPC_EXCL|MSG_R|MSG_W))) == -1) {
		(void)fprintf(stderr, "%s: cannot create server message queue: %s\n",
			KLS_SERVER_NAME, strerror(errno));
		return(-errno);
	}

	/* success, so save queue id */
	server_msqid = msqid;
	return(0);
}

void
kls_server_ipc_terminate()
{
	(void)msgctl(server_msqid, IPC_RMID, 0);
}

int
kls_server_ipc_receive_request(requestp)
	kls_request_header_t **requestp;
{
	dprintf(("%s: kls_server_ipc_receive_request: calling msgrcv()\n",
		KLS_SERVER_NAME));
	if (msgrcv(server_msqid, server_receive_buffer,
	    sizeof(server_receive_buffer), 0, 0) == -1) {
		fprintf(stderr, "%s: kls_server_ipc_receive_request: msgrcv() failed: %s\n",
			KLS_SERVER_NAME, strerror(errno));
		return(-errno);
	}
	*requestp = (kls_request_header_t *)server_receive_buffer;
	return(0);
}

void
kls_server_ipc_send_reply(request, reply)
	kls_request_header_t *request;
	kls_reply_header_t *reply;
{
	int msqid;

	dprintf(("%s: kls_server_ipc_send_reply: calling msgget()\n",
		KLS_SERVER_NAME));
	if ((msqid = msgget(request->klsi_reply_key, MSG_W)) == -1) {
		(void)fprintf(stderr, "%s: msgget() on reply key failed: %s\n",
			KLS_SERVER_NAME, strerror(errno));
		return;
	}

	dprintf(("%s: kls_server_ipc_send_reply: calling msgsnd()\n",
		KLS_SERVER_NAME));
	if (msgsnd(msqid, reply, reply->klsi_msg_size, IPC_NOWAIT) == -1) {
		(void)fprintf(stderr, "%s: msgsnd() of reply failed: %s\n",
			KLS_SERVER_NAME, strerror(errno));
		return;
	}
}

		   
/****************** Internal Server IPC Functions ******************/

static int
server_get_server_key(kp)
	key_t *kp;
{
	int rc;

	if ((rc = get_server_key(kp)) < 0) {
		if (errno) {
			(void)fprintf(stderr, "%s: get_server_key: ftok() failed: %s\n",
				KLS_SERVER_NAME, strerror(errno));
		} else
			(void)fprintf(stderr, "%s: get_server_key: ftok() failed\n",
				KLS_SERVER_NAME);
		return(rc);
	}
	return(0);
}


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
