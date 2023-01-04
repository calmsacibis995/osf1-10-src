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
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of MIT not be used in
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

/* $XConsortium: multibuf.c,v 1.16 92/11/14 16:40:25 rws Exp $ */
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
#include "multibufst.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "inputstr.h"
#include <sys/time.h>

#ifdef VMS
#include <socket.h>
#endif

/*
 * NOTES:
 * 
 * This multibuffer implementation is di/dd split of the original
 * multibuffering implmentation.  (This does not have the full spec
 * implemented as the sample implementation doesn't have it implemented.)
 *
 * The di portion is fully in this file and the dd portion is in
 * $(EXTENSIONSRC)/server/draw/generic.  This is a generic implementation
 * of drawlib (using pixmaps - as the original MIT multibuf implementation
 * used).  The interface was defined by the 3D folks at the MILL.
 * 
 * There is a drawlib for pxg and pvg in the directory
 * $(EXTENSIONSRC)/server/draw.  This implementation expects that drawlib
 * is already loaded as part of the server ddx.  To initialize drawlib for
 * each of these devices. they should be placed in the corresponding
 * InitProc.  Note, in the future, it might be wise to always initialize dd
 * in the generic server ddx so that there is always some sort of drawlib
 * around.
 * 
 * NOTE:  drawlib HAS to be there for this implemenation of multibuf to
 * work. 
 */

/*
 * per-Multibuffer data
 */
 
typedef struct _Multibuffers	*MultibuffersPtr;

#define SameClient(obj,client) \
	(CLIENT_BITS((obj)->resource) == (client)->clientAsMask)
#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])
#define bClient(b)   (clients[CLIENT_ID(b->id)])

#define ValidEventMasks \
         (ExposureMask|MultibufferClobberNotifyMask|MultibufferUpdateNotifyMask)

typedef struct _Multibuffer {
    MultibuffersPtr pMultibuffers;  /* associated window data */
    Mask	    eventMask;	    /* MultibufferClobberNotifyMask|
				      ExposureMask|MultibufferUpdateNotifyMask*/
    Mask	    otherEventMask; /* mask of all other clients event masks */
    OtherClients    *otherClients;
    int		    number;	    /* index into array */
    int		    side;	    /* alwys Mono */
    int		    clobber;	    /* Unclobbered, PartiallyClobbered,
				       FullClobbered */
    XID		    id;		    /* ID for aliasing */
    int             backBufIndex;     /* which back buffer, -1 means window */
} MultibufferRec, *MultibufferPtr;

/*
 * per-window data
 */

typedef struct _Multibuffers {
    WindowPtr	pWindow;		/* associated window */
    int		numMultibuffer;		/* count of buffers */
    int		refcnt;			/* ref count for delete */
    int		displayedMultibuffer;	/* currently active buffer */
    int		updateAction;		/* Undefined, Background, Untouched,
					   Copied */
    /*
      NOTE: we store the updateHint, but we don't really need to.
      We should always be going to drawlib to get the updateHint back.
     */ 
    int		updateHint;		/* Frequent, Intermittent, Static */
    int		windowMode;		/* always Mono */

    TimeStamp	lastUpdate;		/* time of last update */

    unsigned short	width, height;	/* last known window size */
    short		x, y;		/* for static gravity */

    MultibufferPtr	buffers;
} MultibuffersRec;

/*
 * per-screen data
 */
typedef struct _MultibufferScreen {
    Bool	(*PositionWindow)();
} MultibufferScreenRec, *MultibufferScreenPtr;

/*
 * per display-image-buffers request data.
 */
typedef struct _DisplayRequest {
    struct _DisplayRequest	*next;
    TimeStamp			activateTime;
    ClientPtr			pClient;
    XID				id;
} DisplayRequestRec, *DisplayRequestPtr;

static unsigned char	MultibufferReqCode;
static int		MultibufferEventBase;
static int		MultibufferErrorBase;
int			MultibufferScreenIndex;
int			MultibufferWindowIndex;

static int		BlockHandlerRegistered;
static void		MultibufferBlockHandler(), MultibufferWakeupHandler();

static void		PerformDisplayRequest ();
static void		DisposeDisplayRequest ();
static Bool		QueueDisplayRequest ();

static void		BumpTimeStamp ();

void			MultibufferExpose ();
void			MultibufferUpdate ();
static void		AliasMultibuffer ();
int			CreateImageBuffers ();
void			DestroyImageBuffers ();
static void		RecalculateMultibufferOtherEvents ();
static int		EventSelectForMultibuffer();

/*
 * The Pixmap associated with a buffer can be found as a resource
 * with this type
 */
RESTYPE			MultibufferDrawableResType;
static void		MultibufferDrawableDelete ();
/*
 * The per-buffer data can be found as a resource with this type.
 * the resource id of the per-buffer data is the same as the resource
 * id of the pixmap
 */
RESTYPE			MultibufferResType;
static void		MultibufferDelete ();

/*
 * The per-window data can be found as a resource with this type,
 * using the window resource id
 */
RESTYPE			MultibuffersResType;
static void		MultibuffersDelete ();
/*
 * Per display-buffers request is attached to a resource so that
 * it will disappear if the client dies before the request should
 * be processed
 */
RESTYPE			DisplayRequestResType;
static void		DisplayRequestDelete ();

/*
 * Clients other than the buffer creator attach event masks in
 * OtherClient structures; each has a resource of this type.
 */
RESTYPE			OtherClientResType;
static void		OtherClientDelete ();

/*
 * For drawlib interaction.
 */
int 		DrawlibScreenIndex; 
int 		DrawlibWindowIndex;

/*
 * Macros.
 */
/* multibuff */
#define M_WIN_MBUFFS(_pwin_) \
  ( (MultibuffersPtr)((_pwin_)->devPrivates[MultibufferWindowIndex].ptr) )

/* drawlib */
#define M_D_SCREENPRIV(_pscreen_) \
  ( (D_ScreenPrivPtr) ((_pscreen_)->devPrivates[DrawlibScreenIndex].ptr) )

#define M_D_DRAWOPS(_pscreen_) \
  ( (D_Func *) &(M_D_SCREENPRIV(_pscreen_)->drawOps) )

#define M_D_BUFFERS(_pwin_) \
  ( (D_BufferPtr) (_pwin_)->devPrivates[DrawlibWindowIndex].ptr )

/*
 * include files for drawlib.
 */
#include "draw.h"

/****************
 * MultibufferExtensionInit
 *
 * Called from InitExtensions in main()
 *
 ****************/

static int		ProcMultibufferDispatch(), SProcMultibufferDispatch();
static void		MultibufferResetProc();
static void		SClobberNotifyEvent(), SUpdateNotifyEvent();
static Bool		MultibufferPositionWindow();
static void		AliasMultibuffer();

/* 
 * initialize a few variables.
 */
MultibufferScreenIndex = -1;
MultibufferWindowIndex = -1;
DrawlibScreenIndex = -1;
DrawlibWindowIndex = -1;

void
MultibufferExtensionInit()
{
    ExtensionEntry	    *extEntry;
    int			    i, j;
    ScreenPtr		    pScreen;
    MultibufferScreenPtr    pMultibufferScreen;
    /* drawlib screen data structure. */
    D_ScreenInfo 	    *DscreenInfo;
    
    /*
     * allocate private pointers in windows and screens.  Allocating
     * window privates may seem like an unnecessary expense, but every
     * PositionWindow call must check to see if the window is
     * multi-buffered; a resource lookup is too expensive.
     */
    MultibufferScreenIndex = AllocateScreenPrivateIndex ();
    if (MultibufferScreenIndex < 0)
    {
	ErrorF("MultibufferExtensionInit: Unable to allocate screen private index\n");
	return;
    }
    MultibufferWindowIndex = AllocateWindowPrivateIndex ();

    /*
     * Initialize the dev independent code.  All dev specific code
     * is in drawlib.
     */
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (!AllocateWindowPrivate (pScreen, MultibufferWindowIndex, 0) ||
	    !(pMultibufferScreen = 
	      (MultibufferScreenPtr) xalloc (sizeof (MultibufferScreenRec))))
	{
	    for (j = 0; j < i; j++)
	      xfree (screenInfo.screens[j]->
		     devPrivates[MultibufferScreenIndex].ptr);
	    return;
	}
	pScreen->devPrivates[MultibufferScreenIndex].ptr = 
	  (pointer) pMultibufferScreen;
	/*
	 * wrap PositionWindow to resize the pixmap when the window
	 * changes size
	 */
	pMultibufferScreen->PositionWindow = pScreen->PositionWindow;
	pScreen->PositionWindow = MultibufferPositionWindow;
    }

    /*
     * create the resource types
     */
    if (!(MultibufferDrawableResType =
	  CreateNewResourceType(MultibufferDrawableDelete)|RC_CACHED|RC_DRAWABLE))
    {
	ErrorF("MultibufferExtensionInit: Unable to allocate resource types\n");
	return;
    }
    if (!(MultibufferResType = CreateNewResourceType(MultibufferDelete)))
    {
	ErrorF("MultibufferExtensionInit: Unable to allocate resource types\n");
	return;
    }
    if (!(MultibuffersResType = CreateNewResourceType(MultibuffersDelete)))
    {
	ErrorF("MultibufferExtensionInit: Unable to allocate resource types\n");
	return;
    }
    if (!(DisplayRequestResType = CreateNewResourceType(DisplayRequestDelete)))
    {
	ErrorF("MultibufferExtensionInit: Unable to allocate resource types\n");
	return;
    }
    if (!(OtherClientResType = CreateNewResourceType(OtherClientDelete)))
    {
	ErrorF("MultibufferExtensionInit: Unable to allocate resource types\n");
	return;
    }
    
    /* 
     * Get the Drawlib Screen Index
     */
    if ( !GetGlobalNamedResource( D_GlobalResName, (long *)&DscreenInfo) ) 
    {
	ErrorF("MultibufferExtensionInit: Drawlib not loaded.\n");
	return;
    }
    else
    {
	DrawlibScreenIndex = DscreenInfo->screenPrivateIndex;
	DrawlibWindowIndex = DscreenInfo->windowPrivateIndex;
    }
      
    /*
     * Add extension if Resources were allocated.
     */
    extEntry = AddExtension(MULTIBUFFER_PROTOCOL_NAME,
			    MultibufferNumberEvents, 
			    MultibufferNumberErrors,
			    ProcMultibufferDispatch, 
			    SProcMultibufferDispatch,
			    MultibufferResetProc, StandardMinorOpcode);
    if (!extEntry)
    {
	ErrorF("MultibufferExtensionInit: AddExtensions failed\n");
	return;
    }
    
    /*
     * Initialize variables
     */
    MultibufferReqCode = (unsigned char)extEntry->base;
    MultibufferEventBase = extEntry->eventBase;
    MultibufferErrorBase = extEntry->errorBase;
    EventSwapVector[MultibufferEventBase + MultibufferClobberNotify] = 
      SClobberNotifyEvent;
    EventSwapVector[MultibufferEventBase + MultibufferUpdateNotify] = 
      SUpdateNotifyEvent;
}

/*
 * Resource delete func for MultibufferDrawableResType
 */
/*ARGSUSED*/
static void
MultibufferDrawableDelete (pDrawable, id)
    DrawablePtr	pDrawable;
    XID		id;
{
    /*
     * don't do anything all the drawlib buffers will be deleted at once.
     */
}

/*
 * Resource delete func for MultibufferResType
 */
/*ARGSUSED*/
static void
MultibufferDelete (pMultibuffer, id)
    MultibufferPtr	pMultibuffer;
    XID		id;
{
    MultibuffersPtr	pMultibuffers;

    pMultibuffers = pMultibuffer->pMultibuffers;
    if (--pMultibuffers->refcnt == 0)
    {
	FreeResourceByType (pMultibuffers->pWindow->drawable.id,
			    MultibuffersResType, TRUE);
	xfree (pMultibuffers);
    }
}

/*
 * Resource delete func for MultibuffersResType
 */
/*ARGSUSED*/
static void
MultibuffersDelete (pMultibuffers, id)
    MultibuffersPtr	pMultibuffers;
    XID		id;
{
    int	i;

    if (pMultibuffers->refcnt == pMultibuffers->numMultibuffer)
    {
	for (i = pMultibuffers->numMultibuffer; --i >= 0; )
	    FreeResource (pMultibuffers->buffers[i].id,FALSE);
    }
}

/*
 * Resource delete func for DisplayRequestResType
 */
/*ARGSUSED*/
static void
DisplayRequestDelete (pRequest, id)
    DisplayRequestPtr	pRequest;
    XID			id;
{
    DisposeDisplayRequest (pRequest);
}

/*
 * Resource delete func for OtherClientResType
 */
static void
OtherClientDelete (pMultibuffer, id)
    MultibufferPtr	pMultibuffer;
    XID		id;
{
    register OtherClientsPtr	other, prev;

    prev = 0;
    for (other = pMultibuffer->otherClients; other; other = other->next)
    {
	if (other->resource == id)
	{
	    if (prev)
		prev->next = other->next;
	    else
		pMultibuffer->otherClients = other->next;
	    xfree (other);
	    RecalculateMultibufferOtherEvents (pMultibuffer);
	    break;
	}
	prev = other;
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

static void MultibufferResetProc (extEntry)
ExtensionEntry	*extEntry;
{
    int			    i;
    ScreenPtr		    pScreen;
    MultibufferScreenPtr    pMultibufferScreen;
    
    if (MultibufferScreenIndex < 0)
	return;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (pScreen->devPrivates[MultibufferScreenIndex].ptr)
	{
	    pMultibufferScreen = (MultibufferScreenPtr)
	      pScreen->devPrivates[MultibufferScreenIndex].ptr;
	    pScreen->PositionWindow = pMultibufferScreen->PositionWindow;
	    xfree (pMultibufferScreen);
	}
    }
}

/*
 * resize the buffers when the window is resized
 */ 
static Bool
MultibufferPositionWindow (pWin, x, y)
    WindowPtr	pWin;
    int		x, y;
{
    MultibufferPtr	    pMultibuffer;
    MultibuffersPtr	    pMultibuffers;
    MultibufferScreenPtr    pMultibufferScreen;
    int 		    *backBufList;
    int 		    nbufs, i;
    D_BufferPtr		    drawBuffer;
    /* drawlib funcs */
    D_Func 		    *drawOps;
    

    /*
     * First call the original positionWindow routine.
     */
    pMultibufferScreen = (MultibufferScreenPtr)
	(pWin->drawable.pScreen)->devPrivates[MultibufferScreenIndex].ptr;
    (*pMultibufferScreen->PositionWindow) (pWin, x, y);

    
    if ((!pWin->parent) || !(pMultibuffers = M_WIN_MBUFFS(pWin)))
	return TRUE;

    if (pMultibuffers->width == pWin->drawable.width &&
        pMultibuffers->height == pWin->drawable.height)
	return TRUE;

    /*
     * setup the list of buffers to be readied.
     */
    nbufs = pMultibuffers->numMultibuffer ;
    backBufList = (int *)xalloc(sizeof(int) * (nbufs-1)) ;
    for (i=0 ; i<(nbufs-1) ; i++)
	backBufList[i] = i;
	    
    /*
     * call the drawlib function to position the buffers.
     */
    drawOps = M_D_DRAWOPS(pWin->drawable.pScreen);
    
    if (!(drawOps->readyBuffers(M_D_BUFFERS(pWin), D_BACK_LEFT_BUFFER_MASK,
			  nbufs-1, backBufList, NULL)))
	return FALSE;
    
    xfree(backBufList);
    
    /*
     * setup the multibuffers width, height and x and y after readying them.
     */
    pMultibuffers->width = pWin->drawable.width;
    pMultibuffers->height = pWin->drawable.height;
    pMultibuffers->x = pWin->drawable.x;
    pMultibuffers->y = pWin->drawable.y;

    /*
     * change the pixmap pointer to the new pixmap.
     * 
     * change the resource id to point to the correct pixmap for each
     * new pixmap.
     *
     * note: we should go through the backBufList to be precise, but
     * since we know that in this case we go through all the buffers, we're
     * ok.
     */
    drawBuffer = M_D_BUFFERS(pWin);


    for (i=0 ; i<nbufs ; i++)
    {
	/*
	 * setup the new multibuffer pixmap pointer.
	 */
	pMultibuffer = &pMultibuffers->buffers[i];
	/*
	 * change the resource value.
	 */
	if (i != pMultibuffers->displayedMultibuffer)
	{
	    ChangeResourceValue (pMultibuffer->id,
				 MultibufferDrawableResType,
				 (pointer) drawBuffer->backLeftDraw[pMultibuffer->backBufIndex]);

	    drawBuffer->backLeftDraw[pMultibuffer->backBufIndex]->id = pMultibuffer->id;
	    

	}
    }
    
    return TRUE;
}

static int
ProcMultibufferDispatch (client)
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
	return ProcSetMBufferAttributes (client);
    case X_MbufGetMBufferAttributes:
	return ProcGetMBufferAttributes (client);
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
    int				err;

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
    nbuf = len;
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
    err = CreateImageBuffers (pWin, nbuf, ids,
                            stuff->updateAction, stuff->updateHint);
    if (err != Success) {
      if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
      return err;
    }
    
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.numberBuffer = M_WIN_MBUFFS(pWin)->numMultibuffer;
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
    MultibufferPtr	    *pMultibuffer;
    MultibuffersPtr	    *ppMultibuffers;
    int		    nbuf;
    XID		    *ids;
    int		    i, j;
    CARD32	    minDelay;
    TimeStamp	    activateTime, bufferTime;
    
    REQUEST_AT_LEAST_SIZE (xMbufDisplayImageBuffersReq);
    nbuf = stuff->length - (sizeof (xMbufDisplayImageBuffersReq) >> 2);
    if (!nbuf)
	return Success;
    minDelay = stuff->minDelay;
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
    ppMultibuffers = (MultibuffersPtr *) xalloc(nbuf*sizeof (MultibuffersPtr));
    pMultibuffer = (MultibufferPtr *) xalloc (nbuf * sizeof (MultibufferPtr));
    if (!ppMultibuffers || !pMultibuffer)
    {
    	if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
	xfree (ppMultibuffers);
	xfree (pMultibuffer);
	client->errorValue = 0;
	return BadAlloc;
    }
    activateTime.months = 0;
    activateTime.milliseconds = 0;
    for (i = 0; i < nbuf; i++)
    {
	pMultibuffer[i] = (MultibufferPtr)
	  LookupIDByType (ids[i], MultibufferResType);
	if (!pMultibuffer[i])
	{
    	    if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
	    xfree (ppMultibuffers);
	    xfree (pMultibuffer);
	    client->errorValue = ids[i];
	    return MultibufferErrorBase + MultibufferBadBuffer;
	}
	ppMultibuffers[i] = pMultibuffer[i]->pMultibuffers;
	for (j = 0; j < i; j++)
	{
	    if (ppMultibuffers[i] == ppMultibuffers[j])
	    {
    	    	if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
	    	xfree (ppMultibuffers);
	    	xfree (pMultibuffer);
		client->errorValue = ids[i];
	    	return BadMatch;
	    }
	}
	bufferTime = ppMultibuffers[i]->lastUpdate;
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
	PerformDisplayRequest (ppMultibuffers, pMultibuffer, nbuf);
    if ( sizeof (long) != sizeof(CARD32) ) DEALLOCATE_LOCAL(ids);
    xfree (ppMultibuffers);
    xfree (pMultibuffer);
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
    DestroyImageBuffers (pWin);
    return Success;
}

static int
ProcSetMBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST (xMbufSetMBufferAttributesReq);
    WindowPtr	pWin;
    MultibuffersPtr	pMultibuffers;
    int		len;
    Mask	vmask;
    Mask	index;
    CARD32	updateHint;
    XID		*vlist;
    /* drawlib funcs */
    D_Func *drawOps;

    REQUEST_AT_LEAST_SIZE (xMbufSetMBufferAttributesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    pMultibuffers = (MultibuffersPtr)LookupIDByType
      (pWin->drawable.id, MultibuffersResType);
    if (!pMultibuffers)
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
		/* locally store the update hint*/
		pMultibuffers->updateHint = updateHint;
		/*
		 * store the updateHint in drawlib.
		 */
		drawOps = M_D_DRAWOPS(pWin->drawable.pScreen);
		drawOps->setUpdateHint( M_D_BUFFERS(pWin), updateHint);
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
ProcGetMBufferAttributes (client)
    ClientPtr	client;
{
    REQUEST (xMbufGetMBufferAttributesReq);
    WindowPtr	pWin;
    MultibuffersPtr	pMultibuffers;
    CARD32		*ids;
    xMbufGetMBufferAttributesReply  rep;
    int		i, n;
    /* drawlib funcs */
    D_Func *drawOps;

    REQUEST_SIZE_MATCH (xMbufGetMBufferAttributesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin) {
	return BadWindow;
    }
    pMultibuffers =
      (MultibuffersPtr)LookupIDByType (pWin->drawable.id, MultibuffersResType);
    if (!pMultibuffers) {
	return BadAccess;
    }
    ids = (CARD32 *) ALLOCATE_LOCAL (pMultibuffers->numMultibuffer * 
	sizeof (CARD32));
    if (!ids)
	return BadAlloc;
    for (i = 0; i < pMultibuffers->numMultibuffer; i++) 
	ids[i] = pMultibuffers->buffers[i].id;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = pMultibuffers->numMultibuffer;
    /*
     * get currently displayed buffer.
     */
    drawOps = M_D_DRAWOPS(pWin->drawable.pScreen) ;
    rep.displayedBuffer = pMultibuffers->displayedMultibuffer;
    rep.updateAction = pMultibuffers->updateAction;
    /*
     * Get updateHint from drawlib.
     */
    rep.updateHint = drawOps->getUpdateHint( M_D_BUFFERS(pWin)) ;
    rep.windowMode = pMultibuffers->windowMode;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.displayedBuffer, n);
	SwapLongs (ids, pMultibuffers->numMultibuffer);
    }
    WriteToClient (client, sizeof (xMbufGetMBufferAttributesReply), &rep);
    WriteToClient (client, (size_t) (pMultibuffers->numMultibuffer * 
	sizeof (CARD32)), ids);
    DEALLOCATE_LOCAL((pointer) ids);
    return client->noClientException;
}

static int
EventSelectForMultibuffer (pMultibuffer, client, mask)
    MultibufferPtr	pMultibuffer;
    ClientPtr	client;
    Mask	mask;
{
    OtherClientsPtr	other;

    if (mask & ~ValidEventMasks)
    {
	client->errorValue = mask;
	return BadValue;
    }
    if (bClient (pMultibuffer) == client)
    {
	pMultibuffer->eventMask = mask;
    }
    else
    {
	for (other = pMultibuffer->otherClients; other; other = other->next)
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
	    if (!AddResource (other->resource, OtherClientResType,
			      (pointer) pMultibuffer))
	    {
		xfree (other);
		return BadAlloc;
	    }
	    other->next = pMultibuffer->otherClients;
	    pMultibuffer->otherClients = other;
	}
	RecalculateMultibufferOtherEvents (pMultibuffer);
    }
    return (client->noClientException);
}

static int
ProcSetBufferAttributes (client)
    register ClientPtr	client;
{
    REQUEST(xMbufSetBufferAttributesReq);
    MultibufferPtr	pMultibuffer;
    int		len;
    Mask	vmask, index;
    XID		*vlist;
    Mask	eventMask;
    int		result;

    REQUEST_AT_LEAST_SIZE (xMbufSetBufferAttributesReq);
    pMultibuffer =
      (MultibufferPtr) LookupIDByType (stuff->buffer, MultibufferResType);
    if (!pMultibuffer)
	return MultibufferErrorBase + MultibufferBadBuffer;
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
	    result =
	      EventSelectForMultibuffer (pMultibuffer, client, eventMask);
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
    MultibufferPtr	pMultibuffer;
    xMbufGetBufferAttributesReply	rep;
    OtherClientsPtr		other;
    int				n;

    REQUEST_SIZE_MATCH (xMbufGetBufferAttributesReq);
    pMultibuffer =
      (MultibufferPtr) LookupIDByType (stuff->buffer, MultibufferResType);
    if (!pMultibuffer)
	return MultibufferErrorBase + MultibufferBadBuffer;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.window = pMultibuffer->pMultibuffers->pWindow->drawable.id;
    if (bClient (pMultibuffer) == client)
	rep.eventMask = pMultibuffer->eventMask;
    else
    {
	rep.eventMask = (Mask) 0L;
	for (other = pMultibuffer->otherClients; other; other = other->next)
	    if (SameClient (other, client))
	    {
		rep.eventMask = other->mask;
		break;
	    }
    }
    rep.bufferIndex = pMultibuffer->number;
    rep.side = pMultibuffer->side;
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
    if (!pDrawable) {
	return BadDrawable;
    }
    pScreen = pDrawable->pScreen;
    nInfo = 0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	pDepth = &pScreen->allowedDepths[i];
	nInfo += pDepth->numVids;
    }
    pInfo = (xMbufBufferInfo *)
	    ALLOCATE_LOCAL (pScreen->numVisuals * sizeof (xMbufBufferInfo));
    if (!pInfo) {
	return BadAlloc;
    }

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
SProcMultibufferDispatch (client)
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
	return SProcSetMBufferAttributes (client);
    case X_MbufGetMBufferAttributes:
	return SProcGetMBufferAttributes (client);
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
SProcSetMBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufSetMBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufSetMBufferAttributesReq);
    swapl (&stuff->window, n);
    swapl (&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSetMBufferAttributes (client);
}

static int
SProcGetMBufferAttributes (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMbufGetMBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufGetMBufferAttributesReq);
    swapl (&stuff->window, n);
    return ProcGetMBufferAttributes (client);
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

/*
 * Create the Image buffers and set up the devprivate multibuffer
 * data structures for the window.
 */
int
CreateImageBuffers (pWin, nbuf, ids, action, hint)
    WindowPtr	pWin;
    int		nbuf;
    XID		*ids;
    int		action;
    int		hint;
{
    MultibuffersPtr	pMultibuffers;
    MultibufferPtr	pMultibuffer;
    ScreenPtr		pScreen;
    int			width, height, depth;
    int			i;
    D_Config		drawConfig;
    int                 numBackBufs = nbuf - 1;
    DrawablePtr         *ppBackBufs;
    
    
    /* drawlib funcs */
    D_Func *drawOps;

    /*
     * first destroy all existing buffers.
     */
    DestroyImageBuffers(pWin);
    pMultibuffers = (MultibuffersPtr) xalloc (sizeof (MultibuffersRec) +
					      nbuf * sizeof (MultibufferRec));


    if (!pMultibuffers)
      return BadAlloc;

    
    pMultibuffers->pWindow = pWin;
    pMultibuffers->buffers = (MultibufferPtr) (pMultibuffers + 1);
    pMultibuffers->refcnt = pMultibuffers->numMultibuffer = 0;
    if (!AddResource (pWin->drawable.id, MultibuffersResType,
		      (pointer) pMultibuffers))
	return BadAlloc;

    width = pWin->drawable.width;
    height = pWin->drawable.height;
    depth = pWin->drawable.depth;
    pScreen = pWin->drawable.pScreen;

    /*
     * create all the drawlib buffers at once.  Setups the current buffer
     * to be the 0th buffer from within drawlib. 
     *
     * return BadAlloc if the buffers do not get created. 
     */
    drawConfig.numBackBufs = numBackBufs;
    drawConfig.stereo = 0 ;
    drawConfig.depth = 0 ;
    drawConfig.stencil = 0;
    drawConfig.alpha = 0;
    drawConfig.accum = 0; 
    
    drawOps = M_D_DRAWOPS(pScreen);
    numBackBufs = drawOps->initDrawBuffers((DrawablePtr)pWin,
				    &drawConfig,
				    NULL);
    if (numBackBufs < 0)
      return BadAlloc ;
	
     ppBackBufs = M_D_BUFFERS(pWin)->backLeftDraw;

    /*
     * foreach buffer, create and initialize the pMultibuffer
     * data structure; get the drawable id from the drawlib
     * data structure and set up the resource.
     */


    pMultibuffer = &pMultibuffers->buffers[0];
    pMultibuffer->eventMask = 0L;
    pMultibuffer->otherEventMask = 0L;
    pMultibuffer->otherClients = (OtherClientsPtr) NULL;
    pMultibuffer->number = 0;
    pMultibuffer->side = MultibufferSideMono;
    pMultibuffer->clobber = MultibufferUnclobbered;
    pMultibuffer->pMultibuffers = pMultibuffers;
    pMultibuffer->backBufIndex = -1;
    pMultibuffer->id = ids[0];

    AddResource (ids[0], MultibufferResType, (pointer) pMultibuffer);
    AddResource (ids[0], MultibufferDrawableResType, pWin);

    
    for (i = 1; i <= numBackBufs; i++)
    {
	pMultibuffer = &pMultibuffers->buffers[i];
	pMultibuffer->eventMask = 0L;
	pMultibuffer->otherEventMask = 0L;
	pMultibuffer->otherClients = (OtherClientsPtr) NULL;
	/*
	 * note: we are assuming that the index into the drawlib array of
	 *       back buffers is the same as this number.  Thus, we
	 *       use it to  index into the backbuffer list and store
	 *       it here.
	 */
	pMultibuffer->number = i;
	pMultibuffer->side = MultibufferSideMono;
	pMultibuffer->clobber = MultibufferUnclobbered;
	pMultibuffer->pMultibuffers = pMultibuffers;
	if (!AddResource (ids[i], MultibufferResType, (pointer) pMultibuffer))
	    break;

	pMultibuffer->backBufIndex = i - 1;

	if (!ppBackBufs[pMultibuffer->backBufIndex])
	{
	    break;
	}
	
	if (!AddResource (ids[i], MultibufferDrawableResType,
			  (pointer) ppBackBufs[pMultibuffer->backBufIndex]))
	{
	    FreeResource (ids[i], MultibufferResType);
	    /*
	     * free all the n-i drawlib buffers which will not get used.
	     */
	    drawOps->freeBackBuffers(M_D_BUFFERS(pWin), (nbuf-i-1)) ;
	    break;
	}
	
	pMultibuffer->id = ids[i];
	ppBackBufs[pMultibuffer->backBufIndex]->id = ids[i];
    }

    /*
     * set up the generic data for all the multiBuffers.
     */
    pMultibuffers->numMultibuffer = i;
    pMultibuffers->refcnt = i;
    pMultibuffers->displayedMultibuffer = -1;
    if (i > 0)
	AliasMultibuffer (pMultibuffers, -1, 0, drawOps);
    pMultibuffers->updateAction = action;
    pMultibuffers->updateHint = hint;
    /*
     * set the update hint in drawlib.
     */
    drawOps->setUpdateHint(M_D_BUFFERS(pWin),hint);
    pMultibuffers->windowMode = MultibufferModeMono;
    pMultibuffers->lastUpdate.months = 0;
    pMultibuffers->lastUpdate.milliseconds = 0;
    pMultibuffers->width = width;
    pMultibuffers->height = height;
    /*
     * store the multibuffers in the devPrivates data structure. 
     */
    M_WIN_MBUFFS(pWin) = (MultibuffersPtr) pMultibuffers;
    return Success;
}

/*
 * free everything associated with multibuffering for this
 * window
 */
void
DestroyImageBuffers (pWin)
    WindowPtr	pWin;
{
    /* drawlib funcs */
    D_Func *drawOps;
    
    FreeResourceByType (pWin->drawable.id, MultibuffersResType, FALSE);

    /*
     * free all the drawlib buffers at once.
     * (instead of individually during the resource delete procedure.)
     */
    drawOps = M_D_DRAWOPS(pWin->drawable.pScreen);
    if (M_D_BUFFERS(pWin))
	drawOps->freeDrawBuffers( M_D_BUFFERS(pWin)) ;
    /*
     * Zero out the window's pointer to the buffers so they won't be reused
     */
    M_WIN_MBUFFS(pWin) = NULL;
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
	if (!RegisterBlockAndWakeupHandlers (MultibufferBlockHandler,
					     MultibufferWakeupHandler,
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

/*ARGSUSED*/
static void
MultibufferBlockHandler (data, wt, LastSelectMask)
    pointer	    data;		/* unused */
    struct timeval  **wt;		/* wait time */
    long	    *LastSelectMask;	/* unused */
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
	FreeResource (pReq->id, FALSE);
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

/*ARGSUSED*/
static void
MultibufferWakeupHandler (data, i, LastSelectMask)
    pointer	    data;
    int		    i;
    long	    *LastSelectMask;
{
    DisplayRequestPtr	pReq, pNext;

    if (!pPendingRequests)
    {
	RemoveBlockAndWakeupHandlers (MultibufferBlockHandler,
				      MultibufferWakeupHandler,
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
	FreeResource (pReq->id, FALSE);
    }
}

/*
 * Deliver events to a buffer
 */
static int
DeliverEventsToMultibuffer (pMultibuffer, pEvents, count, filter)
    MultibufferPtr	pMultibuffer;
    xEvent	*pEvents;
    int		count;
{
    int deliveries = 0, nondeliveries = 0;
    int attempt;
    OtherClients *other;

    if (!((pMultibuffer->otherEventMask|pMultibuffer->eventMask) & filter))
	return 0;
    if (attempt = TryClientEvents(
	bClient(pMultibuffer), pEvents, count, pMultibuffer->eventMask,
				  filter, (GrabPtr) 0))
    {
	if (attempt > 0)
	    deliveries++;
	else
	    nondeliveries--;
    }
    for (other = pMultibuffer->otherClients; other; other=other->next)
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
 * make the resource id for buffer 'new' refer to the window
 * drawable instead of the pixmap;
 *
 * if oldBuf is <= 0 then it means that we don't have an old buffer to set. 
 */
static void
AliasMultibuffer (pMultibuffers, oldBuf, newBuf, drawOps)
    MultibuffersPtr	pMultibuffers;
    int			oldBuf;
    int			newBuf;
    D_Func 		*drawOps;
{
    MultibufferPtr	pOldMultibuffer, pNewMultibuffer;
    DrawablePtr         *ppBackBufs;
    


    pNewMultibuffer = &pMultibuffers->buffers[newBuf];
    ppBackBufs = M_D_BUFFERS(pMultibuffers->pWindow)->backLeftDraw;
    
    
    /*
     * remove the old association
     */
    if (oldBuf >= 0)
    {
	pOldMultibuffer = &pMultibuffers->buffers[oldBuf];
	pOldMultibuffer->backBufIndex = pNewMultibuffer->backBufIndex;
	ChangeResourceValue (pOldMultibuffer->id,
			     MultibufferDrawableResType,
			     ppBackBufs[pNewMultibuffer->backBufIndex]);
    }
    /*
     * make the new association
     */
    ChangeResourceValue (pNewMultibuffer->id,
			 MultibufferDrawableResType,
			 (pointer) pMultibuffers->pWindow);


    pMultibuffers->displayedMultibuffer = newBuf;


    pNewMultibuffer->backBufIndex = -1;
    
}

void
MultibufferUpdate (pMultibuffer, time)
    MultibufferPtr	pMultibuffer;
    CARD32	time;
{
    xMbufUpdateNotifyEvent	event;

    event.type = MultibufferEventBase + MultibufferUpdateNotify;
    event.buffer = pMultibuffer->id;
    event.timeStamp = time;
    (void) DeliverEventsToMultibuffer (pMultibuffer, (xEvent *)&event,
				       1, MultibufferUpdateNotifyMask);
}

/*
 * Send Expose events to interested clients
 */
void
MultibufferExpose (pMultibuffer, pRegion)
    MultibufferPtr	pMultibuffer;
    RegionPtr	pRegion;
{
    if (pRegion && !REGION_NIL(pRegion))
    {
	xEvent *pEvent;
	DrawablePtr   pDrawable;
	register xEvent *pe;
	register BoxPtr pBox;
	register int i;
	int numRects;
	DrawablePtr *ppBackBufs;
	
	ppBackBufs = M_D_BUFFERS(pMultibuffer->pMultibuffers->pWindow)->backLeftDraw;
	
	
	
	pDrawable = ppBackBufs[pMultibuffer->backBufIndex];

	(* pDrawable->pScreen->TranslateRegion)(pRegion,
		    -pDrawable->x, -pDrawable->y);
	/* XXX MultibufferExpose "knows" the region representation */
	numRects = REGION_NUM_RECTS(pRegion);
	pBox = REGION_RECTS(pRegion);

	pEvent = (xEvent *) ALLOCATE_LOCAL(numRects * sizeof(xEvent));
	if (pEvent) {
	    pe = pEvent;

	    for (i=1; i<=numRects; i++, pe++, pBox++)
	    {
		pe->u.u.type = Expose;
		pe->u.expose.window = pMultibuffer->id;
		pe->u.expose.x = pBox->x1;
		pe->u.expose.y = pBox->y1;
		pe->u.expose.width = pBox->x2 - pBox->x1;
		pe->u.expose.height = pBox->y2 - pBox->y1;
		pe->u.expose.count = (numRects - i);
	    }
	    (void) DeliverEventsToMultibuffer (pMultibuffer, pEvent,
					       numRects, ExposureMask);
	    DEALLOCATE_LOCAL(pEvent);
	}
    }
}

static void
PerformDisplayRequest (ppMultibuffers, pMultibuffer, nbuf)
    MultibufferPtr	    *pMultibuffer;
    MultibuffersPtr	    *ppMultibuffers;
    int		    	    nbuf;
{
    int		    i, j;
    WindowPtr	    pWin;
    RegionPtr	    pExposed;
    MultibufferPtr  pPrevMultibuffer;
    int 	    curBuf; 
    /* drawlib funcs */
    D_Func *drawOps;
    

    UpdateCurrentTime ();
    for (i = 0; i < nbuf; i++)
    {
	pWin = ppMultibuffers[i]->pWindow;
	drawOps = M_D_DRAWOPS(pWin->drawable.pScreen);

	pPrevMultibuffer = &ppMultibuffers[i]->buffers[ppMultibuffers[i]->displayedMultibuffer];

	/*
	 * call drawlib to display buffer.  It also updates
	 * the current buffer. 
	 */
	drawOps->displayBuffer( M_D_BUFFERS(pWin),
			        ppMultibuffers[i]->updateAction,
			        pMultibuffer[i]->backBufIndex,
			        pPrevMultibuffer->eventMask,
			        &pExposed);
	/*
	 * deliver Exposure event if needed.
	 */
	if ((ppMultibuffers[i]->updateAction ==
	     MultibufferUpdateActionUntouched) &&
	    (pPrevMultibuffer->eventMask & ExposureMask) &&
	    (pExposed))
	{
	    MultibufferExpose (pPrevMultibuffer, pExposed);
	    (*pWin->drawable.pScreen->RegionDestroy) (pExposed);
	}
	
	/*
	 * update the last update time. 
	 */ 
	ppMultibuffers[i]->lastUpdate = currentTime;
	
	/*
	 * deliver Update Notify event.
	 */
	MultibufferUpdate (pMultibuffer[i],
			   ppMultibuffers[i]->lastUpdate.milliseconds);
	/*
	 * alias the buffer so that the current displayed buffer id
	 * is associated with the window.
	 * curBuf was the buffer before displayBuffer changed it. 
	 */
	AliasMultibuffer (ppMultibuffers[i],
			  pPrevMultibuffer->number,
			  pMultibuffer[i]->number,
			  drawOps);
    }
}

static void
RecalculateMultibufferOtherEvents (pMultibuffer)
    MultibufferPtr	pMultibuffer;
{
    Mask	    otherEventMask;
    OtherClients    *other;

    otherEventMask = 0L;
    for (other = pMultibuffer->otherClients; other; other = other->next)
	otherEventMask |= other->mask;
    pMultibuffer->otherEventMask = otherEventMask;
}

/* add milliseconds to a timestamp */
static void
BumpTimeStamp (ts, inc)
TimeStamp   *ts;
CARD32	    inc;
{
    CARD32  newms;

    newms = ts->milliseconds + inc;
    if (newms < ts->milliseconds)
	ts->months++;
    ts->milliseconds = newms;
}

/*
 * The sample implementation will never generate MultibufferClobberNotify
 * events
 */
static void
MultibufferClobber (pMultibuffer)
    MultibufferPtr	pMultibuffer;
{
    xMbufClobberNotifyEvent	event;

    event.type = MultibufferEventBase + MultibufferClobberNotify;
    event.buffer = pMultibuffer->id;
    event.state = pMultibuffer->clobber;
    (void) DeliverEventsToMultibuffer (pMultibuffer, (xEvent *)&event,
				       1, MultibufferClobberNotifyMask);
}

