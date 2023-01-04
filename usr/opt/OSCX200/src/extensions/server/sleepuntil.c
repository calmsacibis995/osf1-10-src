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
 * $XConsortium: sleepuntil.c,v 1.1 92/02/24 19:02:27 keith Exp $
 *
 * Copyright 1992 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/* dixsleep.c - implement millisecond timeouts for X clients */

#include "X.h"
#include "Xmd.h"
#include "misc.h"
#include "windowstr.h"
#include "dixstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

typedef struct _Sertafied {
    struct _Sertafied	*next;
    TimeStamp		revive;
    ClientPtr		pClient;
    XID			id;
    void		(*notifyFunc)();
    pointer		closure;
} SertafiedRec, *SertafiedPtr;

static SertafiedPtr pPending;
static RESTYPE	    SertafiedResType;
static Bool	    BlockHandlerRegistered;
static int	    SertafiedGeneration;
static void	    WachetAuf();
static void	    SertafiedDelete();
static void	    SertafiedBlockHandler();
static void	    SertafiedWakeupHandler();

ClientSleepUntil (client, revive, notifyFunc, closure)
    ClientPtr	client;
    TimeStamp	*revive;
    void	(*notifyFunc)();
    pointer	closure;
{
    SertafiedPtr	pRequest, pReq, pPrev;

    if (SertafiedGeneration != serverGeneration)
    {
	SertafiedResType = CreateNewResourceType (SertafiedDelete);
	if (!SertafiedResType)
	    return FALSE;
	SertafiedGeneration = serverGeneration;
	BlockHandlerRegistered = FALSE;
    }
    pRequest = (SertafiedPtr) xalloc (sizeof (SertafiedRec));
    if (!pRequest)
	return FALSE;
    pRequest->pClient = client;
    pRequest->revive = *revive;
    pRequest->id = FakeClientID (client->index);
    pRequest->closure = closure;
    if (!BlockHandlerRegistered)
    {
	if (!RegisterBlockAndWakeupHandlers (SertafiedBlockHandler,
					     SertafiedWakeupHandler,
					     (pointer) 0))
	{
	    xfree (pRequest);
	    return FALSE;
	}
	BlockHandlerRegistered = TRUE;
    }
    pRequest->notifyFunc = 0;
    if (!AddResource (pRequest->id, SertafiedResType, (pointer) pRequest))
	return FALSE;
    if (!notifyFunc)
	notifyFunc = WachetAuf;
    pRequest->notifyFunc = notifyFunc;
    /* Insert into time-ordered queue, with earliest activation time coming first. */
    pPrev = 0;
    for (pReq = pPending; pReq; pReq = pReq->next)
    {
	if (CompareTimeStamps (pReq->revive, *revive) == LATER)
	    break;
	pPrev = pReq;
    }
    if (pPrev)
	pPrev->next = pRequest;
    else
	pPending = pRequest;
    pRequest->next = pReq;
    IgnoreClient (client);
    return TRUE;
}

static void
WachetAuf (client, closure)
    ClientPtr	client;
    pointer	closure;
{
    if (!client->clientGone)
	AttendClient (client);
}


static void
SertafiedDelete (pRequest)
    SertafiedPtr	pRequest;
{
    SertafiedPtr	pReq, pPrev;

    pPrev = 0;
    for (pReq = pPending; pReq; pReq = pReq->next)
	if (pReq == pRequest)
	{
	    if (pPrev)
		pPrev->next = pReq->next;
	    else
		pPending = pReq->next;
	    break;
	}
    if (pRequest->notifyFunc)
	(*pRequest->notifyFunc) (pRequest->pClient, pRequest->closure);
    xfree (pRequest);
}

static void
SertafiedBlockHandler (data, wt, LastSelectMask)
    pointer	    data;		/* unused */
    pointer	    wt;			/* wait time */
    long	    *LastSelectMask;
{
    SertafiedPtr	    pReq, pNext;
    unsigned long	    newdelay, olddelay;
    TimeStamp		    now;

    if (!pPending)
	return;
    now.milliseconds = GetTimeInMillis ();
    now.months = currentTime.months;
    if ((int) (now.milliseconds - currentTime.milliseconds) < 0)
	now.months++;
    for (pReq = pPending; pReq; pReq = pNext)
    {
	pNext = pReq->next;
	if (CompareTimeStamps (pReq->revive, now) == LATER)
	    break;
	FreeResource (pReq->id, RT_NONE);
    }
    pReq = pPending;
    if (!pReq)
	return;
    newdelay = pReq->revive.milliseconds - now.milliseconds;
    AdjustWaitForDelay (wt, newdelay);
}

static void
SertafiedWakeupHandler (data, i, LastSelectMask)
    pointer	    data;
    int		    i;
    long	    *LastSelectMask;
{
    SertafiedPtr	pReq, pNext;
    TimeStamp		now;

    now.milliseconds = GetTimeInMillis ();
    now.months = currentTime.months;
    if ((int) (now.milliseconds - currentTime.milliseconds) < 0)
	now.months++;
    for (pReq = pPending; pReq; pReq = pNext)
    {
	pNext = pReq->next;
	if (CompareTimeStamps (pReq->revive, now) == LATER)
	    break;
	FreeResource (pReq->id, RT_NONE);
    }
    if (!pPending)
    {
	RemoveBlockAndWakeupHandlers (SertafiedBlockHandler,
				      SertafiedWakeupHandler,
				      (pointer) 0);
	BlockHandlerRegistered = FALSE;
    }
}
