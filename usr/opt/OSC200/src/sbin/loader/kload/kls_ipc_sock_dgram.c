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
static char	*sccsid = "@(#)$RCSfile: kls_ipc_sock_dgram.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:49 $";
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
 * kls_ipc_sock_dgram.c
 *
 * This file implements the Kernel Load Server (KLS) IPC primitives
 * using UNIX domain datagram sockets.  The primities themselves are
 * described in kls_ipc.h.  This file implements only the server
 * The code in this file is to be linked into the server.  Although
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
 * system calls fail.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdio.h>
#include <loader.h>
#include <errno.h>
#include <string.h>

#include <loader/kloadsrv.h>

#include "kls_ipc.h"
#include "ldr_macro_help.h"

#define	SERVER_SOCKET_NAME	"/dev/kloadsrv"

#define	TRUE	1

#define	dprintf(x) \
	MACRO_BEGIN \
		if (kls_debug_level > 1) \
			(void) printf x ; \
	MACRO_END

int kls_debug_level;

		    
/******************* Server Static Data *******************/

static struct sockaddr_un server_address;
static int                server_socket;
static char               server_reply_address[8192];
static int                server_reply_address_length;
static char               server_receive_buffer[KLS_MAX_MESSAGE_SIZE];


/************************* Server Functions *************************/

int
kls_server_ipc_initialize()
{
	int rc;

	/* get a socket */
	dprintf(("%s: kls_server_ipc_init: calling socket()\n",
		KLS_SERVER_NAME));
	if ((server_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		(void)fprintf(stderr, "%s: kls_server_ipc_init: socket() failed: %s\n",
			KLS_SERVER_NAME, strerror(errno));
		return(-errno);
	}

	/* bind name to socket */
	server_address.sun_family = AF_UNIX;
	(void)strcpy(server_address.sun_path, SERVER_SOCKET_NAME);
	(void)unlink(server_address.sun_path);
	dprintf(("%s: kls_server_ipc_init: calling bind(\"%s\")\n",
		KLS_SERVER_NAME, server_address.sun_path));
	if ((rc = bind(server_socket, &server_address,
	    sizeof(server_address))) == -1) {
		(void)fprintf(stderr, "%s: kls_server_ipc_init: bind(\"%s\") failed: %s\n",
			KLS_SERVER_NAME, server_address.sun_path,
			strerror(errno));
		(void)close(server_socket);
		return(-errno);
	}

	/* change access mode on name associated with socket */
	dprintf(("%s: kls_server_ipc_init: calling chmod(\"%s\", 0600)\n",
		KLS_SERVER_NAME, server_address.sun_path));
	if (chmod(server_address.sun_path, 0600) == -1) {
		fprintf(stderr, "%s: kls_server_ipc_init: chmod(\"%s\", 0600) failed: %s\n",
			KLS_SERVER_NAME, server_address.sun_path,
			strerror(errno));
		(void)close(server_socket);
		(void)unlink(server_address.sun_path);
		return(-errno);
	}

	return(0);
}

void
kls_server_ipc_terminate()
{
	(void)close(server_socket);
	(void)unlink(server_address.sun_path);
}

int
kls_server_ipc_receive_request(requestp)
	kls_request_header_t **requestp;
{
	dprintf(("%s: kls_server_ipc_receive_request: calling recvfrom()\n",
		KLS_SERVER_NAME));
	server_reply_address_length = sizeof(server_reply_address);
	if (recvfrom(server_socket, server_receive_buffer,
	    sizeof(server_receive_buffer), 0,
	    (struct sockaddr *)server_reply_address,
	    &server_reply_address_length) == -1) {
		fprintf(stderr, "%s: kls_server_ipc_receive_request: recvfrom() failed: %s\n",
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
	dprintf(("%s: kls_server_ipc_send_reply: calling sendto()\n",
		KLS_SERVER_NAME));
	if (sendto(server_socket, (char *)reply, reply->klsi_msg_size, 0,
	    (struct sockaddr *)server_reply_address, server_reply_address_length) == -1) {
		(void)fprintf(stderr, "%s: sendto() failed: %s\n",
			KLS_SERVER_NAME, strerror(errno));
		return;
	}
}
