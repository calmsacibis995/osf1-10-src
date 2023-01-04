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
 * $XConsortium: omron.h,v 1.1 91/06/29 13:48:51 xguest Exp $
 *
 * Copyright 1991 by OMRON Corporation
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OMRON not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  OMRON makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * OMRON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL OMRON
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>

#include <fcntl.h>
#ifdef	uniosu
# include <termio.h>
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/signal.h>

#ifdef	uniosu
# include <sys/sys9100.h>
# include <sys/kbms.h>
#else /* not uniosu */
# if defined(luna88k) || defined(luna2)
#  include <machine/sysomron.h>
#  include <dev/kbms.h>
# else /* uniosb */
#  include <om68k/sysomron.h>
#  include <om68kdev/kbms.h>
# endif
#endif

#include <errno.h>

#include "X.h"
#include "Xmd.h"

#define	NEED_EVENTS
#include "Xproto.h"

#include "osdep.h"
#include "misc.h"

#include "scrnintstr.h"
#include "screenint.h"

#include "servermd.h"

#include "input.h"
#include "inputstr.h"

#include "mipointer.h"

#include "mfb.h"

#define SET_KBTIME 1
#define SET_MSTIME 0

/* libc */
extern int	open();
extern int  read();
extern int  write();
extern int	ioctl();
extern int	close();
extern int	getpagesize();
extern int	getpid();
extern char *valloc();
extern int	free();
extern int	mmap();
extern int	munmap();
extern int	exit();
extern int  sysomron();
extern int  fcntl();
extern int  ffs();
extern int  sigblock();
extern int  sigsetmask();

/* dix */
extern void NoopDDA();
extern int	AddScreen();
extern int	AllocateScreenPrivateIndex();
#ifdef	UNUSE_SIGIO_SIGNAL
extern Bool RegisterBlockAndWakeupHandlers();
#endif

/* os */
extern int  AddEnabledDevice();
extern int  RemoveEnabledDevice();

/* ddx/mi */
extern Bool mieqInit();
extern int  mieqProcessInputEvents();
extern int  mieqEnqueue();
extern void miRegisterPointerDevice();
extern Bool miDCInitialize();

/* ddx/omron */
extern void omronSetLastEventTime();
extern Bool omronParseFbArg();
extern void omronSetIoHandler();
extern void omronInitEventPrvRec();
extern void omronSetDriverTimeMode();

#ifndef	UNUSE_SIGIO_SIGNAL
extern void omronSigIOHandler();
#else
extern void omronWakeupProc();
#endif

extern void omronEnqueueEvents();
#ifndef UNUSE_DRV_TIME
extern void omronEnqueueTEvents();
#endif

/* libc */
extern int errno;

/* os */
extern long EnabledDevices[];
extern long LastSelectMask[];

/* ddx/omron */
extern int omronScreenIndex;	
extern int monitorResolution;

extern int  scrw;
extern int  scrh;
extern int  QueryFb;
extern char *fb_type;

extern int lastEventTime;

extern CARD8 *omronKeyModMap[];
extern KeySymsRec omronKeySyms[];

