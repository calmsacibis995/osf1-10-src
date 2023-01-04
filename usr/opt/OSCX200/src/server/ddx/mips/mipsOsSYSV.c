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
 * $XConsortium: mipsOsSYSV.c,v 1.2 91/07/18 22:58:55 keith Exp $
 *
 * Copyright 1991 MIPS Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of MIPS not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  MIPS makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * MIPS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL MIPS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsOsSYSV.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

/*
 * OS support code which requires svr3 include files
 */

#include <sysv/sys/types.h>
#include <sysv/sys.s>
#include <sysv/sys/ipc.h>
#include <sysv/sys/shm.h>

/* XXX 5.0 brain damage */
#define	_BSD43_SYS_TYPES_	/* don't include bsd43/sys/types.h */
/*#include <sysv/sys/conf.h>*/	/* bsd43/sys/conf.h has no guards! */
#define FMNAMESZ	8
#include <sysv/sys/stropts.h>

#include "mips.h"

#ifndef SYSV
plock(op)
	int op;
{
	return syscall(SYS_plock, op);
}
#endif /* SYSV */

#ifndef SYSV

#define	SHMAT	0
#define	SHMCTL	1
#define	SHMDT	2
#define	SHMGET	3

char *
shmat(shmid, shmaddr, shmflg)
	int shmid;
	char *shmaddr;
	int shmflg;
{
	return (char *) syscall(SYS_shmsys, SHMAT, shmid, shmaddr, shmflg);
}

shmget(key, size, shmflg)
	key_t key;
	int size, shmflg;
{
	return (syscall(SYS_shmsys, SHMGET, key, size, shmflg));
}

shmctl(shmid, cmd, buf)
	int shmid, cmd;
	struct shmid_ds *buf;
{
	return (syscall(SYS_shmsys, SHMCTL, shmid, cmd, buf));
}

shmdt(shmaddr)
	char *shmaddr;
{
	return (syscall(SYS_shmsys, SHMDT, shmaddr));
}
#endif /* SYSV */

mipsStreamAsync(fd, on)
	int fd;
	int on;
{
	return ioctl(fd, I_SETSIG, on ? S_INPUT | S_HIPRI : 0);
}

/*
 * An SVR3 ioctl call has to be used for KTCWRTCOLOR because (as of 4.x)
 * struct colorm is too large for the BSD ioctl code to handle.
 *
 * It is also used for KTMBLANK but I don't know if that's necessary.
 */
sysvIoctl(fd, req, data)
	int fd;
	int req;
	char *data;
{
	return syscall(SYS_ioctl, fd, req, data);
}
