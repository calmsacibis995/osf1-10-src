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
 * $XConsortium: omronMouse.h,v 1.1 91/06/29 13:49:05 xguest Exp $
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

#define MSB_LEFT	4
#define MSB_MIDDLE	2
#define MSB_RIGHT	1

#define	LEFT_PRESSED(b)		((b) & (MSB_LEFT << 6))
#define MIDDLE_PRESSED(b)	((b) & (MSB_MIDDLE << 6))
#define RIGHT_PRESSED(b)	((b) & (MSB_RIGHT << 6))

#define LEFT_RELEASED(b)	((b) & (MSB_LEFT << 3))
#define MIDDLE_RELEASED(b)	((b) & (MSB_MIDDLE << 3))
#define RIGHT_RELEASED(b)	((b) & (MSB_RIGHT << 3))

#define MOUSE_EVENT(b)		((b) & 0x01f8)
#define BUTTON_PRESSED(b)	((b) & 0x01c0)
#define BUTTON_RELEASED(b)	((b) & 0x0038)

#define	LEFT_UP(b)		((b) & MSB_LEFT)
#define MIDDLE_UP(b)		((b) & MSB_MIDDLE)
#define RIGHT_UP(b)		((b) & MSB_RIGHT)

#define LEFT_DOWN(b)		(!LEFT_UP(b))
#define MIDDLE_DOWN(b)		(!MIDDLE_UP(b))
#define RIGHT_DOWN(b)		(!RIGHT_UP(b))


typedef	struct	_omronMousePrvRec {
	int fd;
	int ctl_flags;
	int button_state;
#ifdef uniosu
	int ttyfd;
#endif
}	omronMousePrv,	*omronMousePrvPtr;

extern int  omronMouseProc();
extern void omronMouseGiveUp();

extern void omronMouseEnqueueEvent();
extern struct msdata  *omronMouseGetEvents();

#ifndef UNUSE_DRV_TIME
extern void	omronMouseEnqueueTEvent();
extern struct msdatat *omronMouseGetTEvents();
#endif
