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
#ifndef XVLIB_H
#define XVLIB_H
/*
** File: 
**
**   Xvlib.h --- Xv library public header file
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   26.06.91 Carver
**     - changed XvFreeAdaptors to XvFreeAdaptorInfo
**     - changed XvFreeEncodings to XvFreeEncodingInfo
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   05.15.91 Carver
**     - version 2.0 upgrade
**
**   01.24.91 Carver
**     - version 1.4 upgrade
**
*/

#include <X11/extensions/Xv.h>

typedef struct {
  int numerator;
  int denominator;
} XvRational;

typedef struct {
  XvEncodingID encoding_id;
  char *name;
  unsigned long width;
  unsigned long height;
  XvRational rate;
  unsigned long num_encodings;
} XvEncodingInfo;

typedef struct {
  char depth;
  unsigned long visual_id;
} XvFormat;

typedef struct {
  XvPortID base_id;
  unsigned long num_ports;
  char type;
  char *name;
  unsigned long num_formats;
  XvFormat *formats;
  unsigned long num_adaptors;
} XvAdaptorInfo;

typedef struct {
  int type;
  unsigned long serial;	   /* # of last request processed by server */
  Bool send_event;	   /* true if this came from a SendEvent request */
  Display *display;	   /* Display the event was read from */
  Drawable drawable;       /* drawable */
  unsigned long reason;    /* what generated this event */
  XvPortID port_id;        /* what port */
  Time time;		   /* milliseconds */
} XvVideoNotifyEvent;

typedef struct {
  int type;
  unsigned long serial;	   /* # of last request processed by server */
  Bool send_event;	   /* true if this came from a SendEvent request */
  Display *display;	   /* Display the event was read from */
  XvPortID port_id;        /* what port */
  Time time;		   /* milliseconds */
  Atom attribute;           /* atom that identifies attribute */
  long value;              /* value of attribute */
} XvPortNotifyEvent;

typedef union {
  int type;
  XvVideoNotifyEvent xvvideo;
  XvPortNotifyEvent xvport;
  long pad[24];
} XvEvent;

extern int XvQueryExtension(
#if NeedFunctionPrototypes
  Display*                 /* display */,
  unsigned int*            /* p_version */,
  unsigned int*            /* p_revision */,
  unsigned int*            /* p_requestBase */,
  unsigned int*            /* p_eventBase */, 
  unsigned int*            /* p_errorBase */
#endif
);

extern int XvQueryAdaptors(
#if NeedFunctionPrototypes
  Display*                 /* display */,
  Window                   /* window */,
  unsigned int*            /* p_nAdaptors */,
  XvAdaptorInfo**          /* p_pAdaptors */
#endif
);

extern int XvQueryEncodings(
#if NeedFunctionPrototypes
  Display*                 /* display */,
  XvPortID                 /* port */,
  unsigned int*            /* p_nEncoding */,
  XvEncodingInfo**         /* p_pEncoding */
#endif
);

extern int XvPutVideo(
#if NeedFunctionPrototypes
  Display*                 /* display */,
  XvPortID                 /* port */,
  Drawable                 /* d */,
  GC                       /* gc */,
  int                      /* vx */, 
  int                      /* vy */,
  unsigned int             /* vw */, 
  unsigned int             /* vh */,
  int                      /* dx */, 
  int                      /* dy */,
  unsigned int             /* dw */,
  unsigned int             /* dh */
#endif
);

extern int XvPutStill(
#if NeedFunctionPrototypes
  Display*                 /* display */,
  XvPortID                 /* port */,
  Drawable                 /* d */,
  GC                       /* gc */,
  int                      /* vx */, 
  int                      /* vy */,
  unsigned int             /* vw */, 
  unsigned int             /* vh */,
  int                      /* dx */, 
  int                      /* dy */,
  unsigned int             /* dw */,
  unsigned int             /* dh */
#endif
);

extern int XvGetVideo(
#if NeedFunctionPrototypes
  Display*                 /* display */,
  XvPortID                 /* port */,
  Drawable                 /* d */,
  GC                       /* gc */,
  int                      /* vx */, 
  int                      /* vy */,
  unsigned int             /* vw */, 
  unsigned int             /* vh */,
  int                      /* dx */, 
  int                      /* dy */,
  unsigned int             /* dw */,
  unsigned int             /* dh */
#endif
);

extern int XvGetStill(
#if NeedFunctionPrototypes
  Display*                 /* display */,
  XvPortID                 /* port */,
  Drawable                 /* d */,
  GC                       /* gc */,
  int                      /* vx */, 
  int                      /* vy */,
  unsigned int             /* vw */, 
  unsigned int             /* vh */,
  int                      /* dx */, 
  int                      /* dy */,
  unsigned int             /* dw */,
  unsigned int             /* dh */
#endif
);

extern int XvStopVideo(
#if NeedFunctionPrototypes
  Display*                /* display */,
  XvPortID                /* port */,
  Drawable                /* drawable */
#endif
);

extern int XvGrabPort(
#if NeedFunctionPrototypes
  Display*                /* display */,
  XvPortID                /* port */,
  Time                    /* time */
#endif
);

extern int XvUngrabPort(
#if NeedFunctionPrototypes
  Display*                /* display */,
  XvPortID                /* port */,
  Time                    /* time */
#endif
);

extern int XvSelectVideoNotify(
#if NeedFunctionPrototypes
  Display*                /* display */,
  Drawable                /* drawable */,
  Bool                    /* onoff */
#endif
);

extern int XvSelectPortNotify(
#if NeedFunctionPrototypes
  Display*                /* display */,
  XvPortID                /* port */,
  Bool                    /* onoff */
#endif
);

extern int XvSetPortAttribute(
#if NeedFunctionPrototypes
  Display*                /* display */,
  XvPortID                /* port */,
  Atom                    /* attribute */,
  int                     /* value */
#endif
);

extern int XvGetPortAttribute(
#if NeedFunctionPrototypes
  Display*                /* display */,
  XvPortID                /* port */,
  Atom                    /* attribute */,
  int*                    /* p_value */
#endif
);

extern int XvQueryBestSize(
#if NeedFunctionPrototypes
  Display*                /* display */,
  XvPortID                /* port */,
  Bool                    /* motion */,
  unsigned int            /* vid_w */, 
  unsigned int            /* vid_h */,
  unsigned int            /* drw_w */, 
  unsigned int            /* drw_h */,
  unsigned int*           /* p_actual_width */, 
  unsigned int*           /* p_actual_width */
#endif
);

extern void XvFreeAdaptorInfo(
#if NeedFunctionPrototypes
  XvAdaptorInfo*          /* adaptors */
#endif
);

extern void XvFreeEncodingInfo(
#if NeedFunctionPrototypes
  XvEncodingInfo*         /* encodings */
#endif
);

#endif XVLIB_H
