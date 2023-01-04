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
 *	@(#)$RCSfile: kls_ipc.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:34 $
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


/********************** Server IPC Interfaces **********************/

extern int
kls_server_ipc_initialize(void);

extern void
kls_server_ipc_terminate(void);

extern int
kls_server_ipc_receive_request(kls_request_header_t **);

extern void
kls_server_ipc_send_reply(kls_request_header_t *, kls_reply_header_t *);
