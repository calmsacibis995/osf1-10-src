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
 * $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/x386/x386.h,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */

#ifndef _X386_H
#define _X386_H

/*
 * structure common for all modes
 */
typedef struct _DispM {
  struct _DispM	*prev,*next;
  char		*name;              /* identifier of this mode */
  int		Clock;              /* doclock */
  int           HDisplay;           /* horizontal timing */
  int           HSyncStart;
  int           HSyncEnd;
  int           HTotal;
  int           VDisplay;           /* vertical timing */
  int           VSyncStart;
  int           VSyncEnd;
  int           VTotal;
  int           Flags;
} DisplayModeRec, *DisplayModePtr;

#define V_PHSYNC    0x0001
#define V_NHSYNC    0x0002
#define V_PVSYNC    0x0004
#define V_NVSYNC    0x0008
#define V_INTERLACE 0x0010
#define V_DBLSCAN   0x0020

#define MAXCLOCKS   32

/*
 * the graphic device
 */
typedef struct {
  Bool           configured;
  int            index;
  Bool           (* Probe)();
  Bool           (* Init)();
  void           (* EnterLeaveVT)();
  void           (* EnterLeaveMonitor)();
  void           (* EnterLeaveCursor)();
  void           (* AdjustFrame)();
  void           (* SwitchMode)();
  int            depth;
  int            bitsPerPixel;
  int            defaultVisual;
  int            virtualX,virtualY; 
  int            frameX0, frameY0, frameX1, frameY1;
  char           *vendor;
  char           *chipset;
  int            clocks;
  int            clock[MAXCLOCKS];
  int            videoRam;
  int            width, height;            /* real display dimensions */
  DisplayModePtr modes;
} ScrnInfoRec, *ScrnInfoPtr;

#define VGA_DRIVER  1
#define V256_DRIVER 2
#define WGA_DRIVER  3
#define XGA_DRIVER  4

#define ENTER       1
#define LEAVE       0

extern Bool        x386VTSema;

#endif /* _X386_H */










