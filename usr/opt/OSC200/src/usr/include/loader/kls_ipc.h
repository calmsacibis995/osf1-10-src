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
/*	
 *	@(#)$RCSfile: kls_ipc.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:08:59 $
 */ 
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
 * Four IPC Facilities Available			
 *
 *	Stream Sockets		(kls_ipc_socket_stream.c)
 *	Datagram Sockets	(kls_ipc_socket_dgram.c)
 *	System V Messages	(kls_ipc_sysV_msg.c)
 *	Mach IPC		(kls_ipc_mach.c)
 */


/********************** Client IPC Interfaces **********************/

extern int
kls_client_ipc_connect_to_server(void);

extern int
kls_client_ipc_disconnect_from_server(void);

extern int
kls_client_ipc_send_request(kls_request_header_t *);

extern int
kls_client_ipc_receive_reply(kls_reply_header_t **);


#ifdef	_KLS_IPC_SOCK_DGRAM

#define	SERVER_SOCKET_NAME	"/dev/kloadsrv"

#else	/* _KLS_IPC_SOCK_DGRAM */

#ifdef	_KLS_IPC_SYSV_MSG

#define	SERVER_FTOK_PATH	KLS_SERVER_PATHNAME
#define	SERVER_FTOK_ID	'A'

#endif	/* _KLS_IPC_SYSV_MSG */
#endif	/* _KLS_IPC_SOCK_DGRAM */
