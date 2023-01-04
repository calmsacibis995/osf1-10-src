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
#ifndef XVDIX_H
#define XVDIX_H
/*
** File: 
**
**   xvdix.h --- Xv device independent header file
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   29.08.91 Carver
**     - removed UnrealizeWindow wrapper unrealizing windows no longer 
**       preempts video
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   24.01.91 Carver
**     - version 1.4 upgrade
**
*/

#include "pixmap.h"
#include "Xvproto.h"

#ifdef GLOBAL
#define EXTERNAL
#define INIT(i) = i
#else GLOBAL
#define EXTERNAL extern
#define INIT(i)
#endif

EXTERNAL int  XvScreenIndex;
EXTERNAL unsigned long XvExtensionGeneration INIT(0);
EXTERNAL unsigned long XvScreenGeneration INIT(0);
EXTERNAL unsigned long XvResourceGeneration INIT(0);

EXTERNAL int XvReqCode;
EXTERNAL int XvEventBase;
EXTERNAL int XvErrorBase;

EXTERNAL unsigned long XvRTPort;
EXTERNAL unsigned long XvRTEncoding;
EXTERNAL unsigned long XvRTGrab;
EXTERNAL unsigned long XvRTVideoNotify;
EXTERNAL unsigned long XvRTVideoNotifyList;
EXTERNAL unsigned long XvRTPortNotify;

typedef struct {
  int numerator;
  int denominator;
} XvRationalRec, *XvRationalPtr;

typedef struct {
  char depth;
  unsigned long visual;
} XvFormatRec, *XvFormatPtr;

typedef struct {
  XID id;
  ClientPtr client;
} XvGrabRec, *XvGrabPtr;

typedef struct _XvVideoNotifyRec {
  struct _XvVideoNotifyRec *next;
  ClientPtr client;
  XID id;
  unsigned long mask;
} XvVideoNotifyRec, *XvVideoNotifyPtr;

typedef struct _XvPortNotifyRec {
  struct _XvPortNotifyRec *next;
  ClientPtr client;
  XID id;
} XvPortNotifyRec, *XvPortNotifyPtr;

typedef struct {
  XID id;
  ScreenPtr pScreen;
  char *name;
  unsigned short width, height;
  XvRationalRec rate;
} XvEncodingRec, *XvEncodingPtr;

typedef struct {
  unsigned long base_id;
  unsigned char type; 
  char *name;
  int nEncodings;
  XvEncodingPtr pEncodings;  
  int nFormats;
  XvFormatPtr pFormats;  
  int nPorts;
  struct _XvPortRec *pPorts;
  ScreenPtr pScreen; 
  int (* ddAllocatePort)();
  int (* ddFreePort)();
  int (* ddPutVideo)();
  int (* ddPutStill)();
  int (* ddGetVideo)();
  int (* ddGetStill)();
  int (* ddStopVideo)();
  int (* ddSetPortAttribute)();
  int (* ddGetPortAttribute)();
  int (* ddQueryBestSize)();
  DevUnion devPriv;
} XvAdaptorRec, *XvAdaptorPtr;

typedef struct _XvPortRec {
  XID id;
  XvAdaptorPtr pAdaptor;
  XvPortNotifyPtr pNotify;
  DrawablePtr pDraw;
  ClientPtr client;
  XvGrabRec grab;
  TimeStamp time;
  DevUnion devPriv;
} XvPortRec, *XvPortPtr;

#define LOOKUP_PORT(_id, client)\
     ((XvPortPtr)LookupIDByType(_id, XvRTPort))

#define LOOKUP_ENCODING(_id, client)\
     ((XvEncodingPtr)LookupIDByType(_id, XvRTEncoding))

#define LOOKUP_VIDEONOTIFY_LIST(_id, client)\
     ((XvVideoNotifyPtr)LookupIDByType(_id, XvRTVideoNotifyList))

#define LOOKUP_PORTNOTIFY_LIST(_id, client)\
     ((XvPortNotifyPtr)LookupIDByType(_id, XvRTPortNotifyList))

typedef struct {
  int version, revision;
  int nAdaptors;
  XvAdaptorPtr pAdaptors;
  Bool (* DestroyWindow)();
  Bool (* DestroyPixmap)();
  Bool (* CloseScreen)();
  Bool (* ddCloseScreen)();
  int (* ddQueryAdaptors)();
  DevUnion devPriv;
} XvScreenRec, *XvScreenPtr;

#define SCREEN_PROLOGUE(pScreen, field)\
  ((pScreen)->field = \
   ((XvScreenPtr) \
    (pScreen)->devPrivates[XvScreenIndex].ptr)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)

/* Errors */

#define _XvBadPort (XvBadPort+XvErrorBase)
#define _XvBadEncoding (XvBadEncoding+XvErrorBase)

extern int ProcXvDispatch();
extern int SProcXvDispatch();

extern void XvExtensionInit();
extern int XvScreenInit();
extern Bool XvCloseScreen();
extern Bool XvDestroyPixmap();
extern Bool XvDestroyWindow();
extern void XvResetProc();

extern int XvdiDestroyGrab();
extern int XvdiDestroyEncoding();
extern int XvdiDestroyVideoNotify();
extern int XvdiDestroyPortNotify();
extern int XvdiDestroyVideoNotifyList();
extern int XvdiDestroyPort();
extern int XvdiValidatePort();
extern int XvdiSendVideoNotify();
extern int XvdiSendPortNotify();
extern int XvdiVideoStopped();

extern int XvdiPutVideo();
extern int XvdiPutStill();
extern int XvdiGetVideo();
extern int XvdiGetStill();
extern int XvdiSelectVideoNotify();
extern int XvdiSelectPortNotify();
extern int XvdiSetPortAttribute();
extern int XvdiGetPortAttribute();
extern int XvdiAbortVideo();
extern int XvdiStopVideo();
extern int XvdiPreemptVideo();
extern int XvdiMatchPort();

#if defined(__STDC__) && !defined(UNIXCPP)

#define XVCALL(name) Xv##name

#else

#define XVCALL(name) Xv/**/name

#endif

#undef EXTERNAL
#undef INIT

#endif XVDIX_H

