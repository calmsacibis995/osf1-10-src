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
 *static char *rcsid = "@(#)$RCSfile: pty_data.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/05/26 17:47:44 $";
 */
#include <rpty.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/poll.h>
#include <sys/strlog.h>
#include <sys/lock_types.h>
#include <kern/sched_prim.h>
#if     SEC_BASE
#include <sys/security.h>
#endif
#if     SEC_ARCH
#include <sys/secpolicy.h>
#endif
#include <kern/assert.h> 
#include <sys/ioctl.h>

#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/eucioctl.h>
#include <tty/stream_tty.h>
#include <sys/sysconfig.h>
#include <kern/lock.h>
#include <tty/pty.h>

#ifdef BINARY
extern uint_t	nptys;
#else	/* BINARY */

#if	NRPTY == 1
#undef	NRPTY
#define NRPTY	(64+16)		/* crude XXX */
#endif

#if	NRPTY > 3162
#undef	NRPTY
#define	NRPTY 3162
#endif
uint_t	nptys = NRPTY;
#endif /* BINARY */
