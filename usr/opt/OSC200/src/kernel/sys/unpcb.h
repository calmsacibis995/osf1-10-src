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
 *	@(#)$RCSfile: unpcb.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:02:40 $
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base:	unpcb.h	7.4 (Berkeley) 5/9/89
 *	Merged: unpcb.h	7.6 (Berkeley) 6/28/90
 */

#ifndef	_SYS_UNPCB_H_
#define _SYS_UNPCB_H_

/*
 * Protocol control block for an active
 * instance of a UNIX internal protocol.
 *
 * A socket may be associated with a vnode in the
 * file system.  If so, the unp_vnode pointer holds
 * a reference count to this vnode, which should be vrele'd
 * when the socket goes away.
 *
 * A socket may be connected to another socket, in which
 * case the control block of the socket to which it is connected
 * is given by unp_conn.
 *
 * A socket may be referenced by a number of sockets (e.g. several
 * sockets may be connected to a datagram socket.)  These sockets
 * are in a linked list starting with unp_refs, linked through
 * unp_nextref and null-terminated.  Note that a socket may be referenced
 * by a number of other sockets and may also reference a socket (not
 * necessarily one which is referencing it).  This generates
 * the need for unp_refs and unp_nextref to be separate fields.
 *
 * Stream sockets keep copies of receive sockbuf sb_cc and sb_mbcnt
 * so that changes in the sockbuf may be computed to modify
 * back pressure on the sender accordingly.
 */
struct	unpcb {
	struct	socket *unp_socket;	/* pointer back to socket */
	struct	vnode *unp_vnode;	/* if associated with file */
	ino_t	unp_vno;		/* fake vnode number */
	struct	unpcb *unp_conn;	/* control block of connected socket */
	struct	unpcb *unp_refs;	/* referencing socket linked list */
	struct 	unpcb *unp_nextref;	/* link in unp_refs list */
	struct	mbuf *unp_addr;		/* bound address of socket */
	int	unp_cc;			/* copy of rcv.sb_cc */
	int	unp_mbcnt;		/* copy of rcv.sb_mbcnt */
	time_t	unp_atime;		/* pipe access time for stat */
	time_t	unp_mtime;		/* pipe modify time for stat */
	time_t	unp_ctime;		/* pipe change time for stat */
};

#define	sotounpcb(so)	((struct unpcb *)((so)->so_pcb))

#ifdef	_KERNEL
#if	NETSYNC_LOCK
extern simple_lock_data_t	global_unpconn_lock, unp_misc_lock;
#define UNPCONN_LOCKINIT()	simple_lock_init(&global_unpconn_lock)
#define UNPCONN_LOCK()		simple_lock(&global_unpconn_lock)
#define UNPCONN_UNLOCK()	simple_unlock(&global_unpconn_lock)
#define UNPMISC_LOCKINIT()	simple_lock_init(&unp_misc_lock)
#define UNPMISC_LOCK()		simple_lock(&unp_misc_lock)
#define UNPMISC_UNLOCK()	simple_unlock(&unp_misc_lock)
#else
#define UNPCONN_LOCKINIT()
#define UNPCONN_LOCK()
#define UNPCONN_UNLOCK()
#define UNPMISC_LOCKINIT()
#define UNPMISC_LOCK()
#define UNPMISC_UNLOCK()
#endif
#endif
#endif
