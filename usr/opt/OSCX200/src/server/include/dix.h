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
/* $XConsortium: dix.h,v 1.62 92/08/21 19:24:23 rws Exp $ */

#ifndef DIX_H
#define DIX_H

#include "gc.h"
#include "window.h"

#define EARLIER -1
#define SAMETIME 0
#define LATER 1

#define NullClient ((ClientPtr) 0L)
#define REQUEST(type) \
	register type *stuff = (type *)client->requestBuffer


#define REQUEST_SIZE_MATCH(req)\
    if ((sizeof(req) >> 2) != client->req_len)\
         return(BadLength)

#define REQUEST_AT_LEAST_SIZE(req) \
    if ((sizeof(req) >> 2) > client->req_len )\
         return(BadLength)

#define REQUEST_FIXED_SIZE(req, n)\
    if (((sizeof(req) >> 2) > client->req_len) || \
        (((sizeof(req) + (n) + 3) >> 2) != client->req_len)) \
         return(BadLength)

#define LEGAL_NEW_RESOURCE(id,client)\
    if (!LegalNewID(id,client)) \
    {\
	client->errorValue = id;\
        return(BadIDChoice);\
    }

/* XXX if you are using this macro, you are probably not generating Match
 * errors where appropriate */
#define LOOKUP_DRAWABLE(did, client)\
    ((client->lastDrawableID == did) ? \
     client->lastDrawable : (DrawablePtr)LookupDrawable(did, client))

#define VERIFY_DRAWABLE(pDraw, did, client)\
    if (client->lastDrawableID == did)\
	pDraw = client->lastDrawable;\
    else \
    {\
	pDraw = (DrawablePtr) LookupIDByClass(did, RC_DRAWABLE);\
	if (!pDraw) \
	{\
	    client->errorValue = did; \
	    return BadDrawable;\
	}\
	if (pDraw->type == UNDRAWABLE_WINDOW)\
	    return BadMatch;\
    }

#define VERIFY_GEOMETRABLE(pDraw, did, client)\
    if (client->lastDrawableID == did)\
	pDraw = client->lastDrawable;\
    else \
    {\
	pDraw = (DrawablePtr) LookupIDByClass(did, RC_DRAWABLE);\
	if (!pDraw) \
	{\
	    client->errorValue = did; \
	    return BadDrawable;\
	}\
    }

#define VERIFY_GC(pGC, rid, client)\
    if (client->lastGCID == rid)\
        pGC = client->lastGC;\
    else\
	pGC = (GC *)LookupIDByType(rid, RT_GC);\
    if (!pGC)\
    {\
	client->errorValue = rid;\
	return (BadGC);\
    }

#define VALIDATE_DRAWABLE_AND_GC(drawID, pDraw, pGC, client)\
    if ((stuff->gc == INVALID) || (client->lastGCID != stuff->gc) ||\
	(client->lastDrawableID != drawID))\
    {\
	VERIFY_GEOMETRABLE(pDraw, drawID, client);\
	VERIFY_GC(pGC, stuff->gc, client);\
	if ((pGC->depth != pDraw->depth) ||\
	    (pGC->pScreen != pDraw->pScreen))\
	    return (BadMatch);\
	client->lastDrawable = pDraw;\
	client->lastDrawableID = drawID;\
	client->lastGC = pGC;\
	client->lastGCID = stuff->gc;\
    }\
    else\
    {\
        pGC = client->lastGC;\
        pDraw = client->lastDrawable;\
    }\
    if (pGC->serialNumber != pDraw->serialNumber)\
	ValidateGC(pDraw, pGC);

#define WriteReplyToClient(pClient, size, pReply) \
   if ((pClient)->swapped) \
      (*ReplySwapVector[((xReq *)(pClient)->requestBuffer)->reqType]) \
           (pClient, (int)(size), pReply); \
      else (void) WriteToClient(pClient, (int)(size), (char *)(pReply));

#define WriteSwappedDataToClient(pClient, size, pbuf) \
   if ((pClient)->swapped) \
      (*(pClient)->pSwapReplyFunc)(pClient, (int)(size), pbuf); \
   else (void) WriteToClient (pClient, (int)(size), (char *)(pbuf));

typedef struct _TimeStamp *TimeStampPtr;
typedef struct _Client *ClientPtr;

typedef struct _WorkQueue	*WorkQueuePtr;


extern ClientPtr requestingClient;
extern ClientPtr *clients;
extern ClientPtr serverClient;
extern int currentMaxClients;
#ifdef __alpha /* THIS NEEDS TO BE LONG!!! */
extern int *checkForInput[2];
#else
extern long *checkForInput[2];
#endif

extern int ProcAllowEvents();
extern int ProcBell();
extern int ProcChangeActivePointerGrab();
extern int ProcChangeKeyboardControl();
extern int ProcChangePointerControl();
extern int ProcGetKeyboardMapping();
extern int ProcGetPointerMapping();
extern int ProcGetInputFocus();
extern int ProcGetKeyboardControl();
extern int ProcGetMotionEvents();
extern int ProcGetPointerControl();
extern int ProcGrabButton();
extern int ProcGrabKey();
extern int ProcGrabKeyboard();
extern int ProcGrabPointer();
extern int ProcQueryKeymap();
extern int ProcQueryPointer();
extern int ProcSetInputFocus();
extern int ProcSetKeyboardMapping();
extern int ProcSetPointerMapping();
extern int ProcSendEvent();
extern int ProcUngrabButton();
extern int ProcUngrabKey();
extern int ProcUngrabKeyboard();
extern int ProcUngrabPointer();
extern int ProcWarpPointer();
extern int ProcRecolorCursor();

extern WindowPtr LookupWindow();
extern pointer LookupDrawable();

extern void NoopDDA();

#endif /* DIX_H */
