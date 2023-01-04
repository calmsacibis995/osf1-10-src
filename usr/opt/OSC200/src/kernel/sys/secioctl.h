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
 *	@(#)$RCSfile: secioctl.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:59:48 $
 */ 
/*
 */
#ifndef __SECIOCTL__
#define __SECIOCTL__

#if SEC_BASE

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Copyright (c) 1990 SecureWare, Inc.  All Rights Reserved.
 *

 *
 * Definitions of ioctl command codes for the audit and
 * security policy devices.
 */
/*
 * OSF/1 Release 1.0
 */

#include <sys/secdefines.h>
#include <sys/ioctl.h>

#ifndef _IO
#define _IO(c,n)	n
#endif

/*
 * Audit subsystem ioctl command codes.
 */

#ifdef _OSF_SOURCE
/*
 * For systems that require a character literal as first argument to _IO()
 */

#define	AUDIOC_ENABLE	_IO('A',1)	/* initialize and enable auditing */
#define	AUDIOC_SHUTDOWN	_IO('A',2)	/* system shutdown */
#define	AUDIOC_DISABLE	_IO('A',3)	/* disable audit subsystem */
#define	AUDIOC_SYSMASK	_IO('A',4)	/* system audit mask */
#define	AUDIOC_USERMASK	_IO('A',5)	/* user masks */
#define	AUDIOC_DAEMON	_IO('A',6)	/* daemon read control */
#define	AUDIOC_ACK	_IO('A',7)	/* daemon acknowledge */
#define	AUDIOC_STATS	_IO('A',8)	/* get subsystem statistics */
#define	AUDIOC_IDS	_IO('A',9)	/* uid/gid selective collection */
#if SEC_MAC
#define	AUDIOC_SLEVEL	_IO('A',10)	/* MAC sensitivity level collection */
#endif
#define	AUDIOC_INHERIT	_IO('A',11)	/* Inherit daemon process identity */
#define AUDIOC_CSWITCH	_IO('A',12)	/* Tell the daemon to switch files */

#else /* _OSF_SOURCE */

#define	AUDIOC_ENABLE	_IO(A,1)	/* initialize and enable auditing */
#define	AUDIOC_SHUTDOWN	_IO(A,2)	/* system shutdown */
#define	AUDIOC_DISABLE	_IO(A,3)	/* disable audit subsystem */
#define	AUDIOC_SYSMASK	_IO(A,4)	/* system audit mask */
#define	AUDIOC_USERMASK	_IO(A,5)	/* user masks */
#define	AUDIOC_DAEMON	_IO(A,6)	/* daemon read control */
#define	AUDIOC_ACK	_IO(A,7)	/* daemon acknowledge */
#define	AUDIOC_STATS	_IO(A,8)	/* get subsystem statistics */
#define	AUDIOC_IDS	_IO(A,9)	/* uid/gid selective collection */
#if SEC_MAC
#define	AUDIOC_SLEVEL	_IO(A,10)	/* MAC sensitivity level collection */
#endif
#define	AUDIOC_INHERIT	_IO(A,11)	/* Inherit daemon process identity */
#define AUDIOC_CSWITCH	_IO(A,12)	/* Tell the daemon to switch files */

#endif /* _OSF_SOURCE */

#if SEC_ARCH

/*
 * Security policy message device ioctl command codes.
 */

#ifdef _OSF_SOURCE
/*
 * for systems that require a character literal as the first argument to _IO()
 */

#define SPIOC_INIT	_IO('A',1)	/* initialize the policy cache */
#define SPIOC_INV_CACHE	_IO('A',2)	/* invalidate the decision cache */
#define SPIOC_INV_TAG	_IO('A',3)	/* invalidate cache entry */
#define SPIOC_IMMUNE	_IO('A',4)	/* prepare for shutdown */
#define SPIOC_SHUTDOWN	_IO('A',5)	/* terminate */
#define SPIOC_SET_CACHE_SIZE	_IO('A',6)	/* alter the cache size */
#define SPIOC_GET_STATS	_IO('A',7)	/* retrieve policy statistics */
#define SPIOC_GETCONF	_IO('A',8)	/* retrieve policy configuration */
#define SPIOC_EXCL	_IO('A',9)	/* make daemon device exclusive */
#define SPIOC_REINIT	_IO('A',20)	/* reinitialize spd structures */

#else /* _OSF_SOURCE */

#define SPIOC_INIT	_IO(A,1)	/* initialize the policy cache */
#define SPIOC_INV_CACHE	_IO(A,2)	/* invalidate the decision cache */
#define SPIOC_INV_TAG	_IO(A,3)	/* invalidate cache entry */
#define SPIOC_IMMUNE	_IO(A,4)	/* prepare for shutdown */
#define SPIOC_SHUTDOWN	_IO(A,5)	/* terminate */
#define SPIOC_SET_CACHE_SIZE	_IO(A,6)	/* alter the cache size */
#define SPIOC_GET_STATS	_IO(A,7)	/* retrieve policy statistics */
#define SPIOC_GETCONF	_IO(A,8)	/* retrieve policy configuration */
#define SPIOC_EXCL	_IO(A,9)	/* make daemon device exclusive */
#define SPIOC_REINIT	_IO(A,20)	/* reinitialize spd structures */

#endif /* _OSF_SOURCE */

#endif /* SEC_ARCH */
#endif /* SEC_BASE */
#endif /* __SECIOCTL__ */
