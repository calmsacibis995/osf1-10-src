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
 * @(#)$RCSfile: tty_pty_data.c,v $ $Revision: 1.2.10.2 $ (DEC) $Date: 1993/03/26 21:58:02 $
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *
 * tty_pty_data.c
 *
 * Modification History:
 *
 * 31-Aug-91	Fred Canter
 *	Created file to make NPTY configurable.
 */

#include "pty.h"
#include <rt_preempt.h>

#include <sys/secdefines.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/limits.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/user.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/vnode.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/lock_types.h>
#include <kern/assert.h>
#include <kern/parallel.h>
#if	SEC_BASE
#include <sys/security.h>
#endif
#if	SEC_ARCH
#include <sys/secpolicy.h>
#endif
#include <kji.h>

/*
 * pts == /dev/tty[pqrs]?
 * ptc == /dev/pty[pqrs]?
 */
struct	pt_ioctl {
	int	pt_flags;
	int	pt_count;
	queue_head_t	pt_selq;
	u_char	pt_send;
	u_char	pt_ucntl;
};
struct pt_data {
	struct tty pt_tty;
	struct pt_ioctl pt_ioctl;
};

#ifdef BINARY

extern struct	pt_data *pt_datap[];
extern u_long 	pt_mask[];
extern int	npty;
extern int 	nptymask;

#if	SEC_ARCH
/*
 * Allocate space for pty tag pools.  On systems that allocate the pty
 * structures dynamically, the tag pools should also be dynamically
 * allocated at the same time as the ptys.
 */
extern tag_t	ptctag[];
extern tag_t	ptstag[];
#endif

#else	/* BINARY */

#if	NPTY == 1
#undef	NPTY
#define NPTY	(64+16)		/* crude XXX */
#endif

#if	NPTY > 3162
#undef	NPTY
#define	NPTY 3162
#endif

struct	pt_data *pt_datap[NPTY];
u_long	pt_mask[(NPTY/LONG_BIT)+1];
int	npty = NPTY;
int	nptymask = (NPTY/LONG_BIT)+1;

#if	SEC_ARCH
/*
 * Allocate space for pty tag pools.  On systems that allocate the pty
 * structures dynamically, the tag pools should also be dynamically
 * allocated at the same time as the ptys.
 */
tag_t	ptctag[NPTY * SEC_TAG_COUNT];
tag_t	ptstag[NPTY * SEC_TAG_COUNT];
#endif

#endif	/* BINARY */
