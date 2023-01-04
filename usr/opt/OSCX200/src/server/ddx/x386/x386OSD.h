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
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/x386/x386OSD.h,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */

#ifndef _X386OSD_H
#define _X386OSD_H

#include <X11/Xos.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#undef NULL
#include <sys/param.h>
#include <signal.h>
#include <termio.h>
#include <errno.h>
extern int errno;

#ifdef _NEED_SYSI86
#include <sys/immu.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/tss.h>
#include <sys/sysi86.h>
#include <sys/v86.h>
#endif

#if defined(ATT) && !defined(i386)
#define i386 /* not defined in ANSI C mode */
#endif
#include <sys/emap.h>

#ifndef SCO
# include    <sys/at_ansi.h>
# include    <sys/kd.h>
# include    <sys/vt.h>
#else /* SCO */
# include    <sys/vtkd.h>
# include    <sys/console.h>
# include    <sys/keyboard.h>
# define LED_CAP  0x01
# define LED_NUM  0x02
# define LED_SCR  0x04
#endif /* SCO */

/* 
 * Special hack for isc 2.2 posix compatible include files
 */
#if !defined(O_NDELAY) && defined(O_NONBLOCK)
# define	O_NDELAY	O_NONBLOCK
#endif

#ifndef VT_ACKACQ
# define VT_ACKACQ 2  /* bed-time for bonzo ... */
#endif

#if defined(ATT) || defined(DELL)
# define XQUEUE
# include <sys/xque.h>
#endif

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 32
#endif

#if !defined(SVR4) || defined(WGA_HACK)
#define usleep(usec) syscall(3112, usec / 1000)
#endif

#endif /* _X386OSD_H */
