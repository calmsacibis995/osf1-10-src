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
#ifndef XV_H
#define XV_H
/*
** File: 
**
**   Xv.h --- Xv shared library and server header file
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   05.15.91 Carver
**     - version 2.0 upgrade
**
**   01.24.91 Carver
**     - version 1.4 upgrade
**
*/

#include <X11/X.h>

#define XvName "XVideo"
#define XvVersion 2
#define XvRevision 2

/* Symbols */

typedef XID XvPortID;
typedef XID XvEncodingID;

#define XvNone 0

#define XvInput          0
#define XvOutput         1

#define XvInputMask      (1L<<XvInput)
#define XvOutputMask     (1L<<XvOutput)

/* Events */

#define XvVideoNotify 0
#define XvPortNotify 1
#define XvNumEvents 2

/* Video Notify Reasons */

#define XvStarted 0
#define XvStopped 1
#define XvBusy 2
#define XvPreempted 3
#define XvHardError 4
#define XvLastReason 4

#define XvNumReasons (XvLastReason + 1)

#define XvStartedMask     (1L<<XvStarted)
#define XvStoppedMask     (1L<<XvStopped)
#define XvBusyMask        (1L<<XvBusy)
#define XvPreemptedMask   (1L<<XvPreempted)
#define XvHardErrorMask   (1L<<XvHardError)

#define XvAnyReasonMask   ((1L<<XvNumReasons) - 1)
#define XvNoReasonMask    0

/* Errors */

#define XvBadPort 0
#define XvBadEncoding 1
#define XvBadControl 2
#define XvNumErrors 3

/* Status */

#define XvBadExtension 1
#define XvAlreadyGrabbed 2
#define XvInvalidTime 3
#define XvBadReply 4
#define XvBadAlloc 5

#endif XV_H

