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
/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
** File: 
**
**   xvrop.h --- Xv RasterOps device dependent header file
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   26.11.91 Carver
**     - optimized PutStill to not redraw enable plane between like stills.
**       added fields to XvropPortRec to cache info from last request.
**
**   29.08.91 Carver
**     - added support for video in StaticGray windows
**     - added field to Port priv to store handle to rop and libpip: prop
**
**   26.06.91 Carver
**     - fixed GC wrappers
**
**   04.06.91 Carver
**     - changed interface to libpip.c, use new libpip.h header file
**     - changed assignment of encoding id's
**
**   31.05.91 Carver
**     - made big fixes to occlusion stuff
**
**   29.05.91 Carver
**     - added serialNumber to port private structure.
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   19.03.91 Carver
**     - original port for v1r4
**
**
*/
#ifndef XVROP_H
#define XVROP_H

#include "Xvproto.h"
#include "region.h"
#include "libpip.h"

#ifdef GLOBAL
#define EXTERNAL
#define INIT(i) = {i}
#else /* GLOBAL */
#define EXTERNAL extern
#define INIT(i)
#endif

#define XVROP_NUM_ADAPTORS 1

typedef struct _XvropPortRec {
  GCPtr pGC;
  int vx,vy,dx,dy;
  unsigned int vw,vh,dw,dh;
  BoxRec enabled_box;
  XvEncodingPtr pEncoding;
  int hue,saturation,brightness,contrast;
  unsigned long serialNumber;
  GCFuncs *wrapFuncs;
  pointer prop;
  Bool gray;
  unsigned int vis;
  unsigned int Dx,Dy;
  DrawablePtr pDrawStill;
  int cvx,cvy,cdx,cdy;
  unsigned int cvw,cvh,cdw,cdh;
} XvropPortRec, *XvropPortPtr;

typedef struct _XvropScreenRec {
  void (* ClipNotify)();
  void (* CopyWindow)();
  void (* WindowExposures)();
  Bool (* UnrealizeWindow)();
} XvropScreenRec, *XvropScreenPtr;

EXTERNAL int XvropNumEncodings[1] INIT(9);
EXTERNAL int XvropNumPorts[1] INIT(1);
EXTERNAL int XvropNumFormats[1] INIT(1);

/* PIP_xxx SYMBOLS TAKEN FROM libpip.h */

#ifdef GLOBAL
XvEncodingRec XvropEncodings[1][9] = 
{
  {
    {PIP_COMPOSITE | PIP_NTSC, (ScreenPtr)NULL, 
       "ntsc-composite", 640, 480, 5994, 100},
    {PIP_COMPOSITE | PIP_PAL, (ScreenPtr)NULL, 
       "pal-composite", 768, 572, 50, 1}, 
    {PIP_COMPOSITE | PIP_SECAM, (ScreenPtr)NULL, 
       "secam-composite", 768, 572, 50, 1},
    {PIP_SVIDEO | PIP_NTSC, (ScreenPtr)NULL, 
       "ntsc-svideo", 640, 480, 5994, 100},
    {PIP_SVIDEO | PIP_PAL, (ScreenPtr)NULL, 
       "pal-svideo", 768, 572, 50, 1}, 
    {PIP_SVIDEO | PIP_SECAM, (ScreenPtr)NULL, 
       "secam-svideo", 768, 572, 50, 1},
    {PIP_RGB | PIP_NTSC, (ScreenPtr)NULL, 
       "ntsc-rgb", 640, 480, 5994, 100},
    {PIP_RGB | PIP_PAL, (ScreenPtr)NULL, 
       "pal-rgb", 768, 572, 50, 1}, 
    {PIP_RGB | PIP_SECAM, (ScreenPtr)NULL, 
       "secam-rgb", 768, 572, 50, 1}
  }
};
#else
XvEncodingRec XvropEncodings[1][9];
#endif /* GLOBAL */

#define ROP_SCREEN_PROLOGUE(pScreen, props, field)\
  ((pScreen)->field = props->field)

#define ROP_SCREEN_EPILOGUE(pScreen, field, wrapper)\
  ((pScreen)->field = wrapper)

/* LIST OF RASTEROPS PORT CONTROLS XXX --- MORE LATER */

EXTERNAL Atom XvropEncoding;
EXTERNAL Atom XvropHue;
EXTERNAL Atom XvropSaturation;
EXTERNAL Atom XvropBrightness;
EXTERNAL Atom XvropContrast;

#undef EXTERNAL
#undef INIT

#endif /* XVROP_H */


