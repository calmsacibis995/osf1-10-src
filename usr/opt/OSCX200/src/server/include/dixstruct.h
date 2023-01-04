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
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/* $XConsortium: dixstruct.h,v 1.17 92/08/21 19:26:00 rws Exp $ */

#ifndef DIXSTRUCT_H
#define DIXSTRUCT_H

#include "dix.h"
#include "resource.h"
#include "cursor.h"
#include "gc.h"
#include "pixmap.h"

/*
 * 	direct-mapped hash table, used by resource manager to store
 *      translation from client ids to server addresses.
 */

typedef struct _TimeStamp {
#if LONG_BIT == 64
    unsigned int months;	/* really ~49.7 days */
    unsigned int milliseconds;
#else /* LONG_BIT == 32 */
    unsigned long months;	/* really ~49.7 days */
    unsigned long milliseconds;
#endif /* LONG_BIT */
}           TimeStamp;

#ifdef DEBUG
#define MAX_REQUEST_LOG 100
#endif

typedef struct _Client {
    int         index;
    Mask        clientAsMask;
    pointer     requestBuffer;
    pointer     osPrivate;	/* for OS layer, including scheduler */
    Bool        swapped;
    void        (*pSwapReplyFunc) ();
    XID         errorValue;
    int         sequence;
    int         closeDownMode;
    int         clientGone;
    int         noClientException;	/* this client died or needs to be
					 * killed */
    DrawablePtr lastDrawable;
    Drawable    lastDrawableID;
    GCPtr       lastGC;
    GContext    lastGCID;
    pointer    *saveSet;
    int         numSaved;
    pointer     screenPrivate[MAXSCREENS];
    int         (**requestVector) ();
    unsigned long req_len;		/* length of current request */
    Bool	big_requests;		/* supports large requests */

#ifdef DEBUG
    unsigned char requestLog[MAX_REQUEST_LOG];
    int         requestLogIndex;
#endif
}           ClientRec;

typedef struct _WorkQueue {
    struct _WorkQueue *next;
    Bool        (*function) ();
    ClientPtr   client;
    pointer     closure;
}           WorkQueueRec;

extern TimeStamp currentTime;
extern TimeStamp lastDeviceEventTime;
extern void CloseDownClient();

extern TimeStamp ClientTimeToServerTime();
extern void UpdateCurrentTime();
extern void UpdateCurrentTimeIf();

#endif				/* DIXSTRUCT_H */
