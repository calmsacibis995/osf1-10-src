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
/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL MIT BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: multibuf.c,v 1.8 90/07/31 09:48:31 rws Exp $ */
#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#define _MULTIBUF_SERVER_	/* don't want Xlib structures */
#include "multibuf.h"
#include "multibufst.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "inputstr.h"
#include <sys/time.h>
#include "tm.h"

#include "tmdblbuf.h"

/*
 * XXX: per-screen data.  The original MIT multi-buffering extension used
 * pixmaps to implement the image buffers.  A wrap-routine around the
 * positionwindow function of the screen structure to resize the pixmaps
 * associated with the multi-buffered window.  They way double-buffering
 * is implemented in this version of the server means that the back-buffer
 * is resized (and moved) whenever the window does by tmPositionWindow, which 
 * means that we don't need this extra wrapper routine.
typedef struct _MultibufferScreen {
    Bool	(*PositionWindow)();
} MultibufferScreenRec, *MultibufferScreenPtr;
 */


/*
 * per display-image-buffers request data.
 */
typedef struct _DisplayRequest {
    struct _DisplayRequest	*next;
    TimeStamp			activateTime;
    ClientPtr			pClient;
    XID				id;
} DisplayRequestRec, *DisplayRequestPtr;


static unsigned char	BufferReqCode;
static int		BufferEventBase;
static int		BufferErrorBase;

static int		BlockHandlerRegistered;
static void		BufferBlockHandler(), BufferWakeupHandler();

static void		PerformDisplayRequest ();
static void		DisposeDisplayRequest ();
static Bool		QueueDisplayRequest ();

static void		BumpTimeStamp ();

void			BufferExpose ();
static void		BufferUpdate ();
static void		AliasBuffer ();
static void		RecalculateBufferOtherEvents ();
void 			SetupBackgroundPainter ();
static void		DisposeBuffersAndIds();
static int		EventSelectForBuffer();

/*
 * The Pixmap associated with a buffer can be found as a resource
 * with this type
 */
static RESTYPE		BufferDrawableResType;
static void		BufferDrawableDelete ();
/*
 * The per-buffer data can be found as a resource with this type.
 * the resource id of the per-buffer data is the same as the resource
 * id of the pixmap
 */
static RESTYPE		BufferResType;
static void		BufferDelete ();
/*
 * The per-window data can be found as a resource with this type,
 * using the window resource id
 */
static RESTYPE		BuffersResType;
static void		BuffersDelete ();
/*
 * Per display-buffers request is attached to a resource so that
 * it will disappear if the client dies before the request should
 * be processed
 */
static RESTYPE		DisplayRequestResType;
static void		DisplayRequestDelete ();
/*
 * Clients other than the buffer creator attach event masks in
 * OtherClient structures; each has a resource of this type.
 */
static RESTYPE		OtherClientResType;
static void		OtherClientDelete ();

/****************
 * BufferExtensionInit
 *
 * Called from InitExtensions in main()
 *
 ****************/

static int		ProcBufferDispatch(), SProcBufferDispatch();
static void		BufferResetProc();
static void		SClobberNotifyEvent(), SUpdateNotifyEvent();
static Bool		BufferPositionWindow();

void
MultibufferExtensionInit()
{
    ExtensionEntry	    *extEntry;
    int			    i, j;
    ScreenPtr		    pScreen;

    /*
     * create the resource types
     */
    BufferDrawableResType =
	CreateNewResourceType(BufferDrawableDelete)|RC_CACHED|RC_DRAWABLE;
    BufferResType = CreateNewResourceType(BufferDelete);
    BuffersResType = CreateNewResourceType(BuffersDelete);
    DisplayRequestResType = CreateNewResourceType(DisplayRequestDelete);
    OtherClientResType = CreateNewResourceType(OtherClientDelete);
    if (BufferDrawableResType && BufferResType &&
	BuffersResType && DisplayRequestResType &&
	OtherClientResType &&
	(extEntry = AddExtension(MULTIBUFFER_PROTOCOL_NAME,
				 MultibufferNumberEvents, 
				 MultibufferNumberErrors,
				 ProcBufferDispatch, SProcBufferDispatch,
				 BufferResetProc, StandardMinorOpcode)))
    {
	BufferReqCode = (unsigned char)extEntry->base;
	BufferEventBase = extEntry->eventBase;
	BufferErrorBase = extEntry->errorBase;
	EventSwapVector[BufferEventBase + MultibufferClobberNotify] = SClobberNotifyEvent;
	EventSwapVector[BufferEventBase + MultibufferUpdateNotify] = SUpdateNotifyEvent;
    }
}

/*ARGSUSED*/
static void
BufferResetProc (extEntry)
ExtensionEntry	*extEntry;
{
    int			    i;
    ScreenPtr		    pScreen;
}

static int
ProcGetBufferVersion (client)
    register ClientPtr	client;
{
    REQUEST(xMbufGetBufferVersionReq);
    xMbufGetBufferVersionReply	rep;
    register int		n;

    REQUEST_SIZE_MATCH (xMbufGetBufferVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = MULTIBUFFER_MAJOR_VERSION;
    rep.minorVersion = MULTIBUFFER_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof (xMbufGetBufferVersionReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcCreateImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST(xMbufCreateImageBuffersReq);
    xMbufCreateImageBuffersReply	rep;
    register int		n;
    WindowPtr			pWin;
    XID				*ids;
    int				len, nbuf;
    int				i;
    BuffersPtr			pBuffers;
    BufferPtr			pBuffer;
    ScreenPtr			pScreen;
    int				width, height, depth;
    PixmapPtr			pPixmap;
    vmdPtr			pvmd;
    GCPtr			pGC;
    xRectangle			rect;
    BoxRec			box;

    REQUEST_AT_LEAST_SIZE (xMbufCreateImageBuffersReq);
    len = stuff->length - (sizeof(xMbufCreateImageBuffersReq) >> 2);
    if (len == 0)
	return BadLength;
    if (!(pWin = LookupWindow (stuff->window, client)))
	return BadWindow;
    if (pWin->drawable.class == InputOnly)
	return BadMatch;
    switch (stuff->updateAction)
    {
    case MultibufferUpdateActionUndefined:
    case MultibufferUpdateActionBackground:
    case MultibufferUpdateActionUntouched:
    case MultibufferUpdateActionCopied:
	break;
    default:
	client->errorValue = stuff->updateAction;
	return BadValue;
    }
    switch (stuff->updateHint)
    {
    case MultibufferUpdateHintFrequent:
    case MultibufferUpdateHintIntermittent:
    case MultibufferUpdateHintStatic:
	break;
    default:
	client->errorValue = stuff->updateHint;
	return BadValue;
    }

    /* we only support single and double-buffering, not multi-buffering */
    nbuf = (len >= 2) ? 2 : 1;

    if ( sizeof (long) != sizeof(CARD32) )
    {
        int i;
        CARD32 *value;
        ids = (XID *)ALLOCATE_LOCAL(sizeof(XID)*nbuf);
        if (!ids)
            return BadAlloc;
        value = (CARD32 *)&stuff[1];
        for ( i = 0; i < nbuf; i++ )
            ids[i] = (XID)value[i];
    }
    else
        ids = (XID *) &stuff[1];
    for (i = 0; i < nbuf; i++)
    {
	LEGAL_NEW_RESOURCE(ids[i], client);
    }
    pBuffers = (BuffersPtr) xalloc (sizeof (BuffersRec));
    if (!pBuffers) {
        if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
	return BadAlloc;
    }
    pBuffers->pWindow = pWin;
    pBuffers->buffers = (BufferPtr) xalloc (nbuf * sizeof (BufferRec));
    if (!pBuffers->buffers)
    {
        if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
	xfree (pBuffers);
	return BadAlloc;
    }
    if (!AddResource (pWin->drawable.id, BuffersResType, (pointer) pBuffers))
    {
        if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
	xfree (pBuffers->buffers);
	xfree (pBuffers);
	return BadAlloc;
    }

    width = WINPRIV(pWin)->w;
    height = WINPRIV(pWin)->h;
    depth = pWin->drawable.depth;
    pScreen = pWin->drawable.pScreen;

    /*
     * get a pixmap
     */
    if (!(pPixmap = (*pScreen->CreatePixmap)(pScreen, width, height, depth))) {
	Xfree(pBuffers->buffers);
	Xfree(pBuffers);
	return(BadAlloc);
    }
    
    /* change pixmap's vmd to make it look like a back buffer */
    pvmd = PIXPRIV(pPixmap);

    (*pScreen->RegionDestroy)(pvmd->prgnClip);
    pvmd->prgnClip = &(pWin->clipList);
    
    pvmd->type = osmBack;
    pvmd->x = pWin->drawable.x;
    pvmd->y = pWin->drawable.y;
    pvmd->pBuffers = pBuffers;
    
    pBuffers->pPixmap = pPixmap;
    
    for (i = 0; i < nbuf; i++)
    {
	pBuffer = &pBuffers->buffers[i];
	pBuffer->eventMask = 0L;
	pBuffer->otherEventMask = 0L;
	pBuffer->otherClients = (OtherClients *) NULL;
	pBuffer->number = i;
	pBuffer->side = MultibufferSideMono;
	pBuffer->clobber = MultibufferUnclobbered;
	pBuffer->pBuffers = pBuffers;
	/* This mapping never changes.  It's how we find the per buffer data */
	if (!AddResource(ids[i], BufferResType, (pointer) pBuffer))
	    break;
	/* This mapping changes when we update the display list.
	 * It's how we figure out which drawable to display */
	if (!AddResource(ids[i], BufferDrawableResType, (pointer) pPixmap)) {
	    FreeResource(ids[i], BufferResType);
	    break;
	}
	
	pBuffer->id = ids[i];
    }

    pBuffers->numBuffer = i;
    pBuffers->displayedBuffer = -1;
    if (i > 0)
	AliasBuffer (pBuffers, 0);
    pBuffers->updateAction = stuff->updateAction;
    pBuffers->updateHint = stuff->updateHint;
    pBuffers->windowMode = MultibufferModeMono;
    pBuffers->lastUpdate.months = 0;
    pBuffers->lastUpdate.milliseconds = 0;
    pBuffers->width = width;
    pBuffers->height = height;
    pBuffers->valid_update_region = (*pScreen->RegionCreate)((BoxPtr)NULL, 0);
    WINPRIV(pWin)->pBuffers = pBuffers;

    /* Tile the back buffer's background.
     * XXX: should have a tmback.c function that does exposures
     * and clears.  Should check window realized. */
    pGC = GetScratchGC(pWin->drawable.depth, pWin->drawable.pScreen);
    SetupBackgroundPainter(pWin, pGC);
    ValidateGC(pPixmap, pGC);
    rect.x = 0;
    rect.y = 0;
    rect.width = pPixmap->drawable.width;
    rect.height = pPixmap->drawable.height;
    (*pGC->ops->PolyFillRect) (pPixmap, pGC, 1, &rect);
    FreeScratchGC(pGC);

    /* send reply */
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.numberBuffer = pBuffers->numBuffer;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.numberBuffer, n);
    }
    WriteToClient(client, sizeof (xMbufCreateImageBuffersReply), (char *)&rep);
    if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
    return (client->noClientException);
}

static int
ProcDisplayImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST(xMbufDisplayImageBuffersReq);
    BufferPtr	    *pBuffer;
    BuffersPtr	    *ppBuffers;
    int		    nbuf;
    XID		    *ids;
    int		    i, j;
    CARD32	    minDelay, maxDelay;
    TimeStamp	    activateTime, bufferTime;
    
    REQUEST_AT_LEAST_SIZE (xMbufDisplayImageBuffersReq);
    nbuf = stuff->length - (sizeof (xMbufDisplayImageBuffersReq) >> 2);
    if (!nbuf)
	return Success;
    minDelay = stuff->minDelay;
    maxDelay = stuff->maxDelay;
    if ( sizeof (long) != sizeof(CARD32) )
    {
        int i;
        CARD32 *value;
        ids = (XID *)ALLOCATE_LOCAL(sizeof(XID)*nbuf);
        if (!ids)
            return BadAlloc;
        value = (CARD32 *)&stuff[1];
        for ( i = 0; i < nbuf; i++ )
            ids[i] = (XID)value[i];
    }
    else
        ids = (XID *) &stuff[1];
    ppBuffers = (BuffersPtr *) xalloc (nbuf * sizeof (BuffersPtr));
    pBuffer = (BufferPtr *) xalloc (nbuf * sizeof (BufferPtr));
    if (!ppBuffers || !pBuffer)
    {
        if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
	xfree (ppBuffers);
	xfree (pBuffer);
	client->errorValue = 0;
	return BadAlloc;
    }
    activateTime.months = 0;
    activateTime.milliseconds = 0;
    for (i = 0; i < nbuf; i++)
    {
	pBuffer[i] = (BufferPtr) LookupIDByType (ids[i], BufferResType);
	if (!pBuffer[i])
	{
            if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
	    xfree (ppBuffers);
	    xfree (pBuffer);
	    client->errorValue = ids[i];
	    return BufferErrorBase + MultibufferBadBuffer;
	}
	ppBuffers[i] = pBuffer[i]->pBuffers;
	for (j = 0; j < i; j++)
	{
	    if (ppBuffers[i] == ppBuffers[j])
	    {
                if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
 	    	xfree (ppBuffers);
	    	xfree (pBuffer);
		client->errorValue = ids[i];
	    	return BadMatch;
	    }
	}
	bufferTime = ppBuffers[i]->lastUpdate;
	BumpTimeStamp (&bufferTime, minDelay);
	if (CompareTimeStamps (bufferTime, activateTime) == LATER)
	    activateTime = bufferTime;
    }
    UpdateCurrentTime ();
    if (CompareTimeStamps (activateTime, currentTime) == LATER &&
	QueueDisplayRequest (client, activateTime))
    {
	;
    }
    else
	PerformDisplayRequest (ppBuffers, pBuffer, nbuf);
    if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
    xfree (ppBuffers);
    xfree (pBuffer);
    return Success;
}

static int
ProcDestroyImageBuffers (client)
    register ClientPtr	client;
{
    REQUEST (xMbufDestroyImageBuffersReq);
    WindowPtr	pWin;

    REQUEST_SIZE_MATCH (xMbufDestroyImageBuffersReq);
    if (!(pWin = LookupWindow (stuff->window, client)))
	return BadWindow;

    DisposeBuffersAndIds(pWin, TRUE);

    return Success;
}

static int
ProcSetMultiBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST (xMbufSetMBufferAttributesReq);
    WindowPtr	pWin;
    BuffersPtr	pBuffers;
    int		len;
    Mask	vmask;
    Mask	index;
    CARD32	updateHint;
    XID		*vlist;

    REQUEST_AT_LEAST_SIZE (xMbufSetMBufferAttributesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;

    pBuffers = WINPRIV(pWin)->pBuffers;
    if (!pBuffers)
	return BadMatch;

    len = stuff->length - (sizeof (xMbufSetMBufferAttributesReq) >> 2);
    vmask = stuff->valueMask;
    if (len != Ones (vmask))
	return BadLength;
    if ( sizeof (long) != sizeof(CARD32) )
    {
        int i;
        CARD32 *value;
        vlist = (XID *)ALLOCATE_LOCAL(sizeof(XID)*len);
        if (!vlist)
            return BadAlloc;
        value = (CARD32 *)&stuff[1];
        for ( i = 0; i < len; i++ )
            vlist[i] = (XID)value[i];
    }
    else
        vlist = (XID *) &stuff[1];

    while (vmask)
    {
	index = (Mask) lowbit (vmask);
	vmask &= ~index;
	switch (index)
	{
	case MultibufferWindowUpdateHint:
	    updateHint = (CARD32) *vlist;
	    switch (updateHint)
	    {
	    case MultibufferUpdateHintFrequent:
	    case MultibufferUpdateHintIntermittent:
	    case MultibufferUpdateHintStatic:
		pBuffers->updateHint = updateHint;
		break;
	    default:
		client->errorValue = updateHint;
                if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(vlist);
		return BadValue;
	    }
	    vlist++;
	    break;
	default:
	    client->errorValue = stuff->valueMask;
            if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(vlist);
	    return BadValue;
	}
    }
    if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(vlist);
    return Success;
}

static int
ProcGetMultiBufferAttributes (client)
    ClientPtr	client;
{
    REQUEST (xMbufGetMBufferAttributesReq);
    WindowPtr	pWin;
    BuffersPtr	pBuffers;
    XID		*ids;
    xMbufGetMBufferAttributesReply  rep;
    int		i, n;

    REQUEST_SIZE_MATCH (xMbufGetMBufferAttributesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;

    pBuffers = WINPRIV(pWin)->pBuffers;
    if (!pBuffers)
	return BadAccess;

    ids = (XID *) ALLOCATE_LOCAL (pBuffers->numBuffer * sizeof (XID));
    if (!ids)
	return BadAlloc;
    for (i = 0; i < pBuffers->numBuffer; i++)
	ids[i] = pBuffers->buffers[i].id;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = pBuffers->numBuffer;
    rep.displayedBuffer = pBuffers->displayedBuffer;
    rep.updateAction = pBuffers->updateAction;
    rep.updateHint = pBuffers->updateHint;
    rep.windowMode = pBuffers->windowMode;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.displayedBuffer, n);
	SwapLongs (ids, pBuffers->numBuffer);
    }
    WriteToClient (client, sizeof (xMbufGetMBufferAttributesReply), &rep);
    WriteToClient (client, (int) (pBuffers->numBuffer * sizeof (XID)), ids);
    DEALLOCATE_LOCAL((pointer) ids);
    return client->noClientException;
}

static int
ProcSetBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST(xMbufSetBufferAttributesReq);
    BufferPtr	pBuffer;
    int		len;
    Mask	vmask, index;
    XID		*vlist;
    Mask	eventMask;
    int		result;

    REQUEST_AT_LEAST_SIZE (xMbufSetBufferAttributesReq);
    pBuffer = (BufferPtr) LookupIDByType (stuff->buffer, BufferResType);
    if (!pBuffer)
	return BufferErrorBase + MultibufferBadBuffer;
    len = stuff->length - (sizeof (xMbufSetBufferAttributesReq) >> 2);
    vmask = stuff->valueMask;
    if (len != Ones (vmask))
	return BadLength;
    if ( sizeof (long) != sizeof(CARD32) )
    {
        int i;
        CARD32 *value;
        vlist = (XID *)ALLOCATE_LOCAL(sizeof(XID)*len);
        if (!vlist)
            return BadAlloc;
        value = (CARD32 *)&stuff[1];
        for ( i = 0; i < len; i++ )
            vlist[i] = (XID)value[i];
    }
    else
        vlist = (XID *) &stuff[1];
    while (vmask)
    {
	index = (Mask) lowbit (vmask);
	vmask &= ~index;
	switch (index)
	{
	case MultibufferBufferEventMask:
	    eventMask = (Mask) *vlist;
	    vlist++;
	    result = EventSelectForBuffer (pBuffer, client, eventMask);
	    if (result != Success) {
                if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(vlist);
		return result;
	    }
	    break;
	default:
	    client->errorValue = stuff->valueMask;
            if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(vlist);
	    return BadValue;
	}
    }
    if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(vlist);
    return Success;
}

ProcGetBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST(xMbufGetBufferAttributesReq);
    BufferPtr	pBuffer;
    xMbufGetBufferAttributesReply	rep;
    OtherClientsPtr		other;
    int				n;

    REQUEST_SIZE_MATCH (xMbufGetBufferAttributesReq);
    pBuffer = (BufferPtr) LookupIDByType (stuff->buffer, BufferResType);
    if (!pBuffer)
	return BufferErrorBase + MultibufferBadBuffer;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.window = pBuffer->pBuffers->pWindow->drawable.id;
    if (bClient (pBuffer) == client)
	rep.eventMask = pBuffer->eventMask;
    else
    {
	rep.eventMask = (Mask) 0L;
	for (other = pBuffer->otherClients; other; other = other->next)
	    if (SameClient (other, client))
	    {
		rep.eventMask = other->mask;
		break;
	    }
    }
    rep.bufferIndex = pBuffer->number;
    rep.side = pBuffer->side;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.window, n);
	swapl(&rep.eventMask, n);
	swaps(&rep.bufferIndex, n);
    }
    WriteToClient(client, sizeof (xMbufGetBufferAttributesReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcGetBufferInfo (client)
    register ClientPtr	client;
{
    REQUEST (xMbufGetBufferInfoReq);
    DrawablePtr		    pDrawable;
    xMbufGetBufferInfoReply rep;
    ScreenPtr		    pScreen;
    int			    i, j, k;
    int			    n;
    xMbufBufferInfo	    *pInfo;
    int			    nInfo;
    DepthPtr		    pDepth;

    pDrawable = (DrawablePtr) LookupDrawable (stuff->drawable, client);
    if (!pDrawable)
	return BadDrawable;
    pScreen = pDrawable->pScreen;
    nInfo = 0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	pDepth = &pScreen->allowedDepths[i];
	nInfo += pDepth->numVids;
    }
    pInfo = (xMbufBufferInfo *)
	    ALLOCATE_LOCAL (pScreen->numVisuals * sizeof (xMbufBufferInfo));
    if (!pInfo)
	return BadAlloc;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = nInfo * (sizeof (xMbufBufferInfo) >> 2);
    rep.normalInfo = nInfo;
    rep.stereoInfo = 0;
    if (client->swapped)
    {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swaps(&rep.normalInfo, n);
	swaps(&rep.stereoInfo, n);
    }

    k = 0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	pDepth = &pScreen->allowedDepths[i];
	for (j = 0; j < pDepth->numVids; j++)
	{
	    pInfo[k].visualID = pDepth->vids[j];
	    pInfo[k].maxBuffers = 0;
	    pInfo[k].depth = pDepth->depth;
	    if (client->swapped)
	    {
		swapl (&pInfo[k].visualID, n);
		swaps (&pInfo[k].maxBuffers, n);
	    }
	    k++;
	}
    }
    WriteToClient (client, sizeof (xMbufGetBufferInfoReply), (pointer) &rep);
    WriteToClient (client, (int) nInfo * sizeof (xMbufBufferInfo), (pointer) pInfo);
    DEALLOCATE_LOCAL ((pointer) pInfo);
    return client->noClientException;
}

static int
ProcBufferDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_MbufGetBufferVersion:
	return ProcGetBufferVersion (client);
    case X_MbufCreateImageBuffers:
	return ProcCreateImageBuffers (client);
    case X_MbufDisplayImageBuffers:
	return ProcDisplayImageBuffers (client);
    case X_MbufDestroyImageBuffers:
	return ProcDestroyImageBuffers (client);
    case X_MbufSetMBufferAttributes:
	return ProcSetMultiBufferAttributes (client);
    case X_MbufGetMBufferAttributes:
	return ProcGetMultiBufferAttributes (client);
    case X_MbufSetBufferAttributes:
	return ProcSetBufferAttributes (client);
    case X_MbufGetBufferAttributes:
	return ProcGetBufferAttributes (client);
    case X_MbufGetBufferInfo:
	return ProcGetBufferInfo (client);
    default:
	return BadRequest;
    }
}

static int
SProcGetBufferVersion (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetBufferVersionReq);

    swaps (&stuff->length, n);
    return ProcGetBufferVersion (client);
}

static int
SProcCreateImageBuffers (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufCreateImageBuffersReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xMbufCreateImageBuffersReq);
    swapl (&stuff->window, n);
    SwapRestL(stuff);
    return ProcCreateImageBuffers (client);
}

static int
SProcDisplayImageBuffers (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufDisplayImageBuffersReq);
    
    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xMbufDisplayImageBuffersReq);
    swaps (&stuff->minDelay, n);
    swaps (&stuff->maxDelay, n);
    SwapRestL(stuff);
    return ProcDisplayImageBuffers (client);
}

static int
SProcDestroyImageBuffers (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufDestroyImageBuffersReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xMbufDestroyImageBuffersReq);
    swapl (&stuff->window, n);
    return ProcDestroyImageBuffers (client);
}

static int
SProcSetMultiBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufSetMBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufSetMBufferAttributesReq);
    swapl (&stuff->window, n);
    swapl (&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSetMultiBufferAttributes (client);
}

static int
SProcGetMultiBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetMBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufGetMBufferAttributesReq);
    swapl (&stuff->window, n);
    return ProcGetMultiBufferAttributes (client);
}

static int
SProcSetBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufSetBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufSetBufferAttributesReq);
    swapl (&stuff->buffer, n);
    swapl (&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSetBufferAttributes (client);
}

static int
SProcGetBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufGetBufferAttributesReq);
    swapl (&stuff->buffer, n);
    return ProcGetBufferAttributes (client);
}

static int
SProcGetBufferInfo (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetBufferInfoReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xMbufGetBufferInfoReq);
    swapl (&stuff->drawable, n);
    return ProcGetBufferInfo (client);
}

static int
SProcBufferDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_MbufGetBufferVersion:
	return SProcGetBufferVersion (client);
    case X_MbufCreateImageBuffers:
	return SProcCreateImageBuffers (client);
    case X_MbufDisplayImageBuffers:
	return SProcDisplayImageBuffers (client);
    case X_MbufDestroyImageBuffers:
	return SProcDestroyImageBuffers (client);
    case X_MbufSetMBufferAttributes:
	return SProcSetMultiBufferAttributes (client);
    case X_MbufGetMBufferAttributes:
	return SProcGetMultiBufferAttributes (client);
    case X_MbufSetBufferAttributes:
	return SProcSetBufferAttributes (client);
    case X_MbufGetBufferAttributes:
	return SProcGetBufferAttributes (client);
    case X_MbufGetBufferInfo:
	return SProcGetBufferInfo (client);
    default:
	return BadRequest;
    }
}

static void
SUpdateNotifyEvent (from, to)
    xMbufUpdateNotifyEvent	*from, *to;
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->buffer, to->buffer);
    cpswapl (from->timeStamp, to->timeStamp);
}

static void
SClobberNotifyEvent (from, to)
    xMbufClobberNotifyEvent	*from, *to;
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->buffer, to->buffer);
    to->state = from->state;
}

void
SetupBackgroundPainter (pWin, pGC)
    WindowPtr	pWin;
    GCPtr	pGC;
{
    XID		    gcvalues[4];
    int		    ts_x_origin, ts_y_origin;
    PixUnion	    background;
    int		    backgroundState;
    Mask	    gcmask;

    /*
     * set up the gc to clear the pixmaps;
     */
    ts_x_origin = ts_y_origin = 0;

    backgroundState = pWin->backgroundState;
    background = pWin->background;
    if (backgroundState == ParentRelative) {
	WindowPtr	pParent;

	pParent = pWin;
	while (pParent->backgroundState == ParentRelative) {
	    ts_x_origin -= pParent->origin.x;
	    ts_y_origin -= pParent->origin.y;
	    pParent = pParent->parent;
	}
	backgroundState = pParent->backgroundState;
	background = pParent->background;
    }

    /*
     * First take care of any ParentRelative stuff by altering the
     * tile/stipple origin to match the coordinates of the upper-left
     * corner of the first ancestor without a ParentRelative background.
     * This coordinate is, of course, negative.
     */

    if (backgroundState == BackgroundPixel)
    {
	gcvalues[0] = (XID) background.pixel;
	gcvalues[1] = FillSolid;
	gcmask = GCForeground|GCFillStyle;
    }
    else
    {
	gcvalues[0] = FillTiled;
	gcvalues[1] = (XID) background.pixmap;
	gcvalues[2] = ts_x_origin;
	gcvalues[3] = ts_y_origin;
	gcmask = GCFillStyle|GCTile|GCTileStipXOrigin|GCTileStipYOrigin;
    }
    DoChangeGC(pGC, gcmask, gcvalues, TRUE);
}

static void
PerformDisplayRequest (ppBuffers, pBuffer, nbuf)
    BufferPtr	    *pBuffer;
    BuffersPtr	    *ppBuffers;
    int		    nbuf;
{
    GCPtr	    pGC;
    PixmapPtr	    pPixmap;
    xRectangle	    clearRect;
    WindowPtr	    pWin;
    RegionPtr	    pExposed;
    int		    i;
    BufferPtr  	    pPrevBuffer;
    XID		    gcfunc;

    /*
     * The following is quite different from the MIT Multi-buffering extension
     */
    UpdateCurrentTime();
    for (i = 0; i < nbuf; i++) {
	pWin = ppBuffers[i]->pWindow;
	pPixmap = ppBuffers[i]->pPixmap;
	pGC = GetScratchGC (pWin->drawable.depth, pWin->drawable.pScreen);
	pPrevBuffer = &ppBuffers[i]->buffers[ppBuffers[i]->displayedBuffer];
	osmReadyVmd(PIXPRIV(pPixmap));

	switch(ppBuffers[i]->updateAction) {
	  case MultibufferUpdateActionBackground:
/*	    tmCopyMBufs(pWin, pPixmap); */
	    ValidateGC(pWin, pGC);
	    (*pGC->ops->CopyArea) (pPixmap, pWin, pGC,
				   0, 0, pWin->drawable.width, pWin->drawable.height,
				   0, 0);
	    SetupBackgroundPainter(pWin, pGC);
	    ValidateGC(pPixmap, pGC);
	    clearRect.x = 0;
	    clearRect.y = 0;
	    clearRect.width = pPixmap->drawable.width;
	    clearRect.height = pPixmap->drawable.height;
	    (*pGC->ops->PolyFillRect)(pPixmap, pGC, 1, &clearRect);
	    break;

	  case MultibufferUpdateActionUntouched:
/*	    tmSwapMBufs(pWin, pPixmap); */
	    /*
	     * This a swap buffer operation. We do this with 3 copy areas with
	     * Xor as the function:
	     * Stage 1:
	     * 	W contains	F
	     * 	P contains	B
	     * Step 1: Xor W into P
	     *	W contains	F
	     *	P contains	B^F
	     * Step 2: Xor P into W
	     *	W contains 	F^(B^F) == B  (back image displayed in window)
	     *	P contains	B^F
	     * Step 3: Xor W into P
	     * 	W contains	B
	     *	P contains	(B^F)^B == F  (front image contained in pixmap)
	     */
	    gcfunc = GXxor;
	    DoChangeGC(pGC, GCFunction, &gcfunc, FALSE);
	   
	    /* Xor W into P */
	    ValidateGC(pPixmap, pGC);
	    (*pGC->ops->CopyArea) (pWin, pPixmap, pGC, 
				   0, 0, pWin->drawable.width, pWin->drawable.height,
				   0, 0);

	    /* Xor P into W, do we really have to validate the gc everytime */
	    /* ValidateGC(pWin, pGC); XXX don't think this is necessary */
	    (*pGC->ops->CopyArea) (pPixmap, pWin, pGC, 
				   0, 0, pWin->drawable.width, pWin->drawable.height,
				   0, 0);

	    /* Xor W into P */
	    /* ValidateGC(pPixmap, pGC); XXX don't think this is necessary */
	    (*pGC->ops->CopyArea) (pWin, pPixmap, pGC, 
				   0, 0, pWin->drawable.width, pWin->drawable.height,
				   0, 0);
	    break;

	  case MultibufferUpdateActionUndefined:
	  case MultibufferUpdateActionCopied:
/*	    tmCopyMBufs(pWin, pPixmap); */
	    ValidateGC(pWin, pGC);
	    /* use gc copy area so we are sure to update backing store */
	    (*pGC->ops->CopyArea) (pPixmap, pWin, pGC,
				   0, 0, pWin->drawable.width, pWin->drawable.height,
				   0, 0);
	    break;
	}

	(*pWin->drawable.pScreen->RegionCopy)
	    (ppBuffers[i]->valid_update_region, GCPRIV(pGC)->pCompositeClip);

	ppBuffers[i]->lastUpdate = currentTime;
	BufferUpdate (pBuffer[i], ppBuffers[i]->lastUpdate.milliseconds);
	AliasBuffer (ppBuffers[i], pBuffer[i] - ppBuffers[i]->buffers);
	FreeScratchGC (pGC);
    }
    return;
}



static DisplayRequestPtr    pPendingRequests;

static void
DisposeDisplayRequest (pRequest)
    DisplayRequestPtr	pRequest;
{
    DisplayRequestPtr	pReq, pPrev;

    pPrev = 0;
    for (pReq = pPendingRequests; pReq; pReq = pReq->next)
	if (pReq == pRequest)
	{
	    if (pPrev)
		pPrev->next = pReq->next;
	    else
		pPendingRequests = pReq->next;
	    xfree (pReq);
	    break;
	}
}

static Bool
QueueDisplayRequest (client, activateTime)
    ClientPtr	    client;
    TimeStamp	    activateTime;
{
    DisplayRequestPtr	pRequest, pReq, pPrev;

    if (!BlockHandlerRegistered)
    {
	if (!RegisterBlockAndWakeupHandlers (BufferBlockHandler,
					     BufferWakeupHandler,
					     (pointer) 0))
	{
	    return FALSE;
	}
	BlockHandlerRegistered = TRUE;
    }
    pRequest = (DisplayRequestPtr) xalloc (sizeof (DisplayRequestRec));
    if (!pRequest)
	return FALSE;
    pRequest->pClient = client;
    pRequest->activateTime = activateTime;
    pRequest->id = FakeClientID (client->index);
    if (!AddResource (pRequest->id, DisplayRequestResType, (pointer) pRequest))
    {
	xfree (pRequest);
	return FALSE;
    }
    pPrev = 0;
    for (pReq = pPendingRequests; pReq; pReq = pReq->next)
    {
	if (CompareTimeStamps (pReq->activateTime, activateTime) == LATER)
	    break;
	pPrev = pReq;
    }
    if (pPrev)
	pPrev->next = pRequest;
    else
	pPendingRequests = pRequest;
    pRequest->next = pReq;
    if (client->swapped)
    {
    	register int    n;
    	REQUEST (xMbufDisplayImageBuffersReq);
    	
    	SwapRestL(stuff);
    	swaps (&stuff->length, n);
    	swaps (&stuff->minDelay, n);
    	swaps (&stuff->maxDelay, n);
    }
    client->sequence--;
    ResetCurrentRequest (client);
    IgnoreClient (client);
    return TRUE;
}

static void
BufferBlockHandler (data, wt, LastSelectMask)
    pointer	    data;		/* unused */
    struct timeval  **wt;		/* wait time */
    long	    *LastSelectMask;
{
    DisplayRequestPtr	    pReq, pNext;
    unsigned long	    newdelay, olddelay;
    static struct timeval   delay_val;

    if (!pPendingRequests)
	return;
    UpdateCurrentTimeIf ();
    for (pReq = pPendingRequests; pReq; pReq = pNext)
    {
	pNext = pReq->next;
	if (CompareTimeStamps (pReq->activateTime, currentTime) == LATER)
	    break;
	AttendClient (pReq->pClient);
	FreeResource (pReq->id, 0);
    }
    pReq = pPendingRequests;
    if (!pReq)
	return;
    newdelay = pReq->activateTime.milliseconds - currentTime.milliseconds;
    if (*wt == NULL)
    {
	delay_val.tv_sec = newdelay / 1000;
	delay_val.tv_usec = 1000 * (newdelay % 1000);
	*wt = &delay_val;
    }
    else
    {
	olddelay = (*wt)->tv_sec * 1000 + (*wt)->tv_usec / 1000;
	if (newdelay < olddelay)
	{
	    (*wt)->tv_sec = newdelay / 1000;
	    (*wt)->tv_usec = 1000 * (newdelay % 1000);
	}
    }
}

static void
BufferWakeupHandler (data, i, LastSelectMask)
    pointer	    data;
    int		    i;
    long	    *LastSelectMask;
{
    DisplayRequestPtr	pReq, pNext;

    if (!pPendingRequests)
    {
	RemoveBlockAndWakeupHandlers (BufferBlockHandler,
				      BufferWakeupHandler,
				      (pointer) 0);
	BlockHandlerRegistered = 0;
	return;
    }
    UpdateCurrentTimeIf ();
    for (pReq = pPendingRequests; pReq; pReq = pNext)
    {
	pNext = pReq->next;
	if (CompareTimeStamps (pReq->activateTime, currentTime) == LATER)
	    break;
	AttendClient (pReq->pClient);
	FreeResource (pReq->id, 0);
    }
}

/*
 * Deliver events to a buffer
 */

static int
DeliverEventsToBuffer (pBuffer, pEvents, count, filter)
    BufferPtr	pBuffer;
    xEvent	*pEvents;
    int		count;
{
    int deliveries = 0, nondeliveries = 0;
    int attempt;
    OtherClients *other;

    if (!((pBuffer->otherEventMask|pBuffer->eventMask) & filter))
	return 0;
    if (attempt = TryClientEvents(
	bClient(pBuffer), pEvents, count, pBuffer->eventMask, filter, (GrabPtr) 0))
    {
	if (attempt > 0)
	    deliveries++;
	else
	    nondeliveries--;
    }
    for (other = pBuffer->otherClients; other; other=other->next)
    {
	if (attempt = TryClientEvents(
	      rClient(other), pEvents, count, other->mask, filter, (GrabPtr) 0))
	{
	    if (attempt > 0)
		deliveries++;
	    else
		nondeliveries--;
	}
    }
    if (deliveries)
	return deliveries;
    return nondeliveries;
}

/*
 * Send Expose events to interested clients. Expose events will be sent with 
 * buffer relative positions.
 */
void
BufferExpose (pBuffer, pRegion, x, y)
BufferPtr	pBuffer;
RegionPtr	pRegion;	/* Absolute exposed region */
int		x, y;		/* Absolute buffer origin */
{
    if (pRegion && !REGION_NIL(pRegion))
    {
	xEvent *pEvent;
	register xEvent *pe;
	register BoxPtr pBox;
	register int i;
	int numRects;

	numRects = REGION_NUM_RECTS(pRegion);
	pEvent = (xEvent *) ALLOCATE_LOCAL(numRects * sizeof(xEvent));

	if (pEvent) {
	    pBox = REGION_RECTS(pRegion);
	    pe = pEvent;

	    for (i=1; i<=numRects; i++, pe++, pBox++)
	    {
		pe->u.u.type = Expose;
		pe->u.expose.window = pBuffer->id;
		pe->u.expose.x = pBox->x1 - x;
		pe->u.expose.y = pBox->y1 - y;
		pe->u.expose.width = pBox->x2 - pBox->x1;
		pe->u.expose.height = pBox->y2 - pBox->y1;
		pe->u.expose.count = (numRects - i);
	    }
	    (void) DeliverEventsToBuffer (pBuffer, pEvent, numRects, ExposureMask);
	    DEALLOCATE_LOCAL(pEvent);
	}
    }
}

static void
BufferUpdate (pBuffer, time)
    BufferPtr	pBuffer;
    CARD32	time;
{
    xMbufUpdateNotifyEvent	event;

    event.type = BufferEventBase + MultibufferUpdateNotify;
    event.buffer = pBuffer->id;
    event.timeStamp = time;
    (void) DeliverEventsToBuffer (pBuffer, (xEvent *)&event,
				       1, MultibufferUpdateNotifyMask);
}


#define MapVisibilityToClobber(visibility) (visibility)

BufferClobber(pWindow)
WindowPtr pWindow;
{
    xMbufClobberNotifyEvent	event;
    BuffersPtr		pBuffers;
    BufferPtr		pBuffer;
    int				i;
    
    /* Set up clobber event */
    event.type = BufferEventBase + MultibufferClobberNotify;
    event.state = MapVisibilityToClobber(pWindow->visibility);

    /* send events to all buffers, since in our implementation they all use
       the same clip list and have gotten clobbered */
    pBuffers = WINPRIV(pWindow)->pBuffers;

    for (i = 0; i < pBuffers->numBuffer; i++) {
	pBuffer = &(pBuffers->buffers[i]);
	event.buffer = pBuffer->id;
	(void) DeliverEventsToBuffer (pBuffer, (xEvent *)&event,
					   1, MultibufferClobberNotifyMask);
    }
}

/*
 * make the resource id for buffer i refer to the window
 * drawable instead of the pixmap;
 */

static void
AliasBuffer (pBuffers, i)
    BuffersPtr	pBuffers;
    int		i;
{
    BufferPtr	pBuffer;

    if (i == pBuffers->displayedBuffer)
	return;
    /*
     * remove the old association
     */
    if (pBuffers->displayedBuffer >= 0)
    {
	pBuffer = &pBuffers->buffers[pBuffers->displayedBuffer];
	ChangeResourceValue (pBuffer->id,
			     BufferDrawableResType,
 			     (pointer) pBuffers->pPixmap);
    }
    /*
     * make the new association
     */
    pBuffer = &pBuffers->buffers[i];
    ChangeResourceValue (pBuffer->id,
			 BufferDrawableResType,
			 (pointer) pBuffers->pWindow);
    pBuffers->displayedBuffer = i;
}

/*
 * free everything associated with Buffering for this
 * window
 */

void
DisposeBuffers(pWin)
WindowPtr	pWin;
{
    
/*    FreeResourceByType (pWin->drawable.id, BuffersResType, FALSE);    */
      DisposeBuffersAndIds(pWin, FALSE);

}


/* Called by DestroyImageBuffers and by tmDestroyWindow.
 * Either free the ids from the resource table or just mark the
 * the ids as invalid.  Regardless, any future references to these
 * ids should result in a BadResource error.  It would have been
 * nice to just always free the ids, but calling FreeResource
 * during a FreeClientResources is death due to the way FreeClientResources
 * is implemented.  So, we only call this function with freeBufIds
 * TRUE during a DestroyImageBuffers and FALSE during a DestroyWindow.
 */
static void
DisposeBuffersAndIds(pWin, freeBufIds)
WindowPtr	pWin;
int		freeBufIds;	/* Whether to free the buffer ids
				 * or just mark them invalid */
{
    BuffersPtr	pBuffers = WINPRIV(pWin)->pBuffers;
    int		i;
    
    /* We don't want to die if we're called with a non-double buffered window,
     * or if someone else has already freed this stuff. */
    if(!pBuffers)
	return;
    
    /* Either free the ids from the resource table or just mark the
     * the ids as invalid.  Regardless, any future references to these
     * ids should result in a BadResource error.  It would have been
     * nice to just always free the ids, but calling FreeResource
     * during a FreeClientResources is death due to the way FreeClientResources
     * is implemented.  So, we only call this function with freeBufIds
     * TRUE during a DestroyImageBuffers and FALSE during a Client shutdown.
     */
    for (i = 0; i < pBuffers->numBuffer; i++)
    {
	BufferPtr pBuffer = &pBuffers->buffers[i];

	if (freeBufIds)
	{
	    /* Free buffer id from resource table */
	    FreeResource(pBuffer->id, 0);
	}
	else
	{
	    /* Mark id invalid.  Making value NULL will result
	     * in a bad lookup.  Slimmy, but expedient.  If 
	     * LookupID interface changes, so should this.
	     */
	    ChangeResourceValue(pBuffer->id, BufferResType, (pointer) NULL);
	    ChangeResourceValue(pBuffer->id, BufferDrawableResType, 
				(pointer) NULL);
	}

	/* Free other stuff hanging off each buffer */
	BufferDelete(pBuffer, pBuffer->id);
    }

    (*pWin->drawable.pScreen->DestroyPixmap) (pBuffers->pPixmap);
    (*pWin->drawable.pScreen->RegionDestroy) (pBuffers->valid_update_region);
    Xfree (pBuffers->buffers);
    Xfree (pBuffers);
    WINPRIV(pWin)->pBuffers = NULL;
}




/* Resource delete func for BufferDrawableResType */
static void
BufferDrawableDelete (pDrawable, id)
DrawablePtr	pDrawable;
XID		id;
{
    return;
}

/* Resource delete func for BufferResType */
static void
BufferDelete (pBuffer, id)
BufferPtr	pBuffer;
XID		id;
{
    return;
}

/* Resource delete func for BuffersResType */
static void
BuffersDelete (pBuffers, id)
BuffersPtr	pBuffers;
XID		id;
{
/*    DisposeBuffersAndIds(pBuffers->pWindow, FALSE); */
    return;
}

/* Resource delete func for DisplayRequestResType */
static void
DisplayRequestDelete (pRequest, id)
    DisplayRequestPtr	pRequest;
    XID			id;
{
    DisposeDisplayRequest (pRequest);
}

/* Resource delete func for OtherClientResType */
static void
OtherClientDelete (pBuffer, id)
    BufferPtr	pBuffer;
    XID		id;
{
    register OtherClientsPtr	other, prev;

    prev = 0;
    for (other = pBuffer->otherClients; other; other = other->next)
    {
	if (other->resource == id)
	{
	    if (prev)
		prev->next = other->next;
	    else
		pBuffer->otherClients = other->next;
	    xfree (other);
	    RecalculateBufferOtherEvents (pBuffer);
	    break;
	}
	prev = other;
    }
}

static int
EventSelectForBuffer (pBuffer, client, mask)
    BufferPtr	pBuffer;
    ClientPtr	client;
    Mask	mask;
{
    OtherClientsPtr	other;

    if (mask & ~ValidEventMasks)
    {
	client->errorValue = mask;
	return BadValue;
    }
    if (bClient (pBuffer) == client)
    {
	pBuffer->eventMask = mask;
    }
    else
    {
	for (other = pBuffer->otherClients; other; other = other->next)
	{
	    if (SameClient (other, client))
	    {
		if (mask == 0)
		{
		    FreeResource (other->resource, RT_NONE);
		    break;
		}
		other->mask = mask;
		break;
	    }
	}
	if (!other)
	{
	    other = (OtherClients *) xalloc (sizeof (OtherClients));
	    if (!other)
		return BadAlloc;
	    other->mask = mask;
	    other->resource = FakeClientID (client->index);
	    if (!AddResource (other->resource, OtherClientResType, (pointer) pBuffer))
	    {
		xfree (other);
		return BadAlloc;
	    }
	    other->next = pBuffer->otherClients;
	    pBuffer->otherClients = other;
	}
	RecalculateBufferOtherEvents (pBuffer);
    }
}

static void
RecalculateBufferOtherEvents (pBuffer)
    BufferPtr	pBuffer;
{
    Mask	    otherEventMask;
    OtherClients    *other;

    otherEventMask = 0L;
    for (other = pBuffer->otherClients; other; other = other->next)
	otherEventMask |= other->mask;
    pBuffer->otherEventMask = otherEventMask;
}

/* add milliseconds to a timestamp */
static void
BumpTimeStamp (ts, inc)
TimeStamp   *ts;
CARD32	    inc;
{
    CARD32  newms;

    newms = ts->milliseconds + inc;
    if (newms < (CARD32) ts->milliseconds)
	ts->months++;
    ts->milliseconds = newms;
}
