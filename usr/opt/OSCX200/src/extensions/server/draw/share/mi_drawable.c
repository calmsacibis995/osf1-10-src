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
/****************************************************************************/
/** COPYRIGHT (c) 1988                                                      */
/** by DIGITAL Equipment Corporation, Maynard, Massachusetts.               */
/** ALL RIGHTS RESERVED.                                                    */
/**                                                                         */
/** This software is furnished under a license and may be used and copied   */
/** only  in  accordance with the  terms  of  such  license  and with the   */
/** inclusion of the above copyright notice. This software or  any  other   */
/** copies thereof may not be provided or otherwise made available to any   */
/** other person.  No title to and  ownership of the  software is  hereby   */
/** transferred.                                                            */
/**                                                                         */
/** The information in this software is  subject to change without notice   */
/** and  should  not  be  construed  as a commitment by DIGITAL Equipment   */
/** Corporation.                                                            */
/**                                                                         */
/** DIGITAL assumes no responsibility for the use  or  reliability of its   */
/** software on equipment which is not supplied by DIGITAL.                 */
/****************************************************************************/

/*
 * FILE:
 *     mi_drawable.c
 *
 *  Author:
 *     MPW 1/7/93
 *
 *  Description:
 *
 *  Routines which are not device specific--device specific drawlib may use these
 *  routines or define their own versions.
 *
 *  Revisions: 
 */

#include "X.h"
#include "Xmd.h"
#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "servermd.h"
#include "scrnintstr.h"

#include "draw.h"
#include "mi_drawable.h"

int   _DScreenPrivateIndex;
int   _DWindowPrivateIndex;
#define D_GenerationName "DRAWLIB_GENERATION"

#define MIN_CLIPS_IN_LIST       (10)

int _DDrawInit()
{
    D_ScreenInfo *screenInfo;
    long         drawlibGeneration;

    /*
     * Call the reset proc if the server has reset. 
     */
    if ( !GetGlobalNamedResource( D_GenerationName, &drawlibGeneration) ) 
    {
        /* First time through we must store D_Generation */
        PutGlobalNamedResource( D_GenerationName, serverGeneration);
    } 
    else if (drawlibGeneration != serverGeneration) 
    {
        /* The server reset so reset drawlib and update D_Generation */
	_DDrawReset();
        PutGlobalNamedResource( D_GenerationName, serverGeneration);
    }
 
    if ( !GetGlobalNamedResource( D_GlobalResName, (long *)&screenInfo) ) 
    {
        int i;

        _DScreenPrivateIndex = AllocateScreenPrivateIndex();
        _DWindowPrivateIndex = AllocateWindowPrivateIndex();
        if (_DScreenPrivateIndex < 0 || _DWindowPrivateIndex < 0)
            return FALSE;

        screenInfo = (D_ScreenInfo *) Xalloc( sizeof(D_ScreenInfo));
        screenInfo->screenPrivateIndex = _DScreenPrivateIndex;
        screenInfo->windowPrivateIndex = _DWindowPrivateIndex;
        screenInfo->numScreens = 0;
        for (i=0; i < MAXSCREENS; i++)
            screenInfo->screens[i] = NULL;

        PutGlobalNamedResource( D_GlobalResName, (long)screenInfo);
    }
    else
    {
	_DScreenPrivateIndex = screenInfo->screenPrivateIndex;
        _DWindowPrivateIndex = screenInfo->windowPrivateIndex;
    }

    if (drawlibGeneration != serverGeneration) 
    {
	_DInitPublicDevPrivates();
    }
    

    drawlibGeneration = serverGeneration;

    return TRUE;
}

_DInitPublicDevPrivates()
{

	int		i;
	ScreenPtr	pScreen;

	for (i=0;i<screenInfo.numScreens;i++)
	{
	    pScreen = (ScreenPtr)screenInfo.screens[i];
	    pScreen->devPrivates[_DScreenPrivateIndex].ptr = NULL;
	}

}

int _DDrawReset()
{
    D_ScreenInfo *screenInfo;
 
    if ( GetGlobalNamedResource( D_GlobalResName, (long *)&screenInfo) ) 
    {
	DestroyGlobalNamedResource( D_GlobalResName);
        Xfree( screenInfo);
    }
}


D_ScreenPrivPtr _DInitScreenPriv(pScreen, screenIndex)
ScreenPtr pScreen;
int screenIndex;
{
    D_ScreenPrivPtr     pScreenPriv;
    D_ScreenInfo *screenInfo;

    if (pScreen == NULL) return NULL;

    GetGlobalNamedResource( D_GlobalResName, (long *)&screenInfo);
    if (screenInfo == NULL) return NULL;
    screenInfo->screens[screenInfo->numScreens] = pScreen;
    screenInfo->numScreens++;

    /* Allocate memory for screen private */
    pScreenPriv = (D_ScreenPrivPtr) Xalloc( sizeof(D_ScreenPriv));
    if (pScreenPriv == NULL) return NULL;
    pScreen->devPrivates[_DScreenPrivateIndex].ptr = (pointer) pScreenPriv;

    /* Allocate memory for window privates. */
    AllocateWindowPrivate(pScreen, _DWindowPrivateIndex, 0);

    /* Wrap create and destroy window */
    pScreenPriv->WrappedCreateWindow = pScreen->CreateWindow;
    pScreen->CreateWindow = _DCreateWindow;

    pScreenPriv->WrappedDestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = _DDestroyWindow;

    return pScreenPriv;
}


/*****************************************************************************/
/*
 * This is wrapped around the server's existing CreateWindow routine.
 * The window buffer data is created on an as needed basis.
 */
Bool _DCreateWindow (pWindow)
WindowPtr pWindow;
{
    ScreenPtr       pScreen = pWindow->drawable.pScreen;
    D_ScreenPrivPtr scrPriv = D_SCREENPRIV(pScreen);
    Bool            status;

    /*
     * Call the wrapped create window first.
     */
    pScreen->CreateWindow = scrPriv->WrappedCreateWindow;
    status = (*pScreen->CreateWindow)(pWindow);
    pScreen->CreateWindow = _DCreateWindow;

    /*
     * Don't allocate window private data until it is needed .. it is too
     * much stuff to carry around for all windows
     */
    D_SET_WINDOWPRIV(pWindow, NULL);

    return status;
}


/*
 * This is wrapped around the server's existing DestroyWindow routine.
 */
Bool _DDestroyWindow (pWindow)
WindowPtr pWindow;
{
    ScreenPtr       pScreen = pWindow->drawable.pScreen;
    D_ScreenPrivPtr scrPriv = D_SCREENPRIV(pScreen);
    D_BufferPtr     winPriv = D_WINDOWPRIV(pWindow);
    Bool            status;

    /* Let us follow the instructions in the X11 Server book on how to	     */
    /* write wrapped routines.						     */

    /* 1. Unwrap the destroy routine */

    pScreen->DestroyWindow = scrPriv->WrappedDestroyWindow;

    /* 2. Do what we need to do */

    if ( winPriv != NULL )
    {
        D_FreeDrawBuffers( winPriv);
        Xfree( winPriv);
        D_SET_WINDOWPRIV(pWindow, NULL);
    }
    
    /*
     * 3. Call the wrapped destroy window routine.
     */

    status = (*pScreen->DestroyWindow)(pWindow);

    /* 4. We have nothing more to do for this window */

    /* 5. Re-wrap the destroy window routine */

    pScreen->DestroyWindow = _DDestroyWindow;

    return status;
}


D_BufferPtr _DInitWinPriv( pDraw)
DrawablePtr     pDraw;
{
    D_BufferPtr     buffers;

    buffers =  (D_BufferPtr) Xalloc( sizeof(D_Buffer));
    if (buffers == NULL) return NULL;

    D_SET_WINDOWPRIV((WindowPtr)pDraw, buffers);

    buffers->numBackBufs = 0;

    buffers->frontLeftBuf = NULL;
    buffers->frontRightBuf = NULL;
    buffers->alphaFrontLeftBuf = NULL;
    buffers->alphaBackLeftBuf = NULL;
    buffers->alphaFrontRightBuf = NULL;
    buffers->alphaBackRightBuf = NULL;
    buffers->depthBuf = NULL;
    buffers->stencilBuf = NULL;
    buffers->accumBuf = NULL;

    return buffers;
}


int D_InitDrawBuffers( pDraw, config, buffers)
DrawablePtr     pDraw;
D_ConfigPtr     config;
D_BufferPtr     buffers;
{
    D_Func              *drawOps = D_DRAWOP(pDraw->pScreen);
    int			i, status;
    DrawablePtr         backDraw;

    /*
     * Allocate window priv if necessary--this is too much stuff to carry around
     * for all windows
     */
    if ( buffers == NULL ) {
        if (pDraw->type != DRAWABLE_WINDOW) return -1;
        buffers = _DInitWinPriv( pDraw);
        if (buffers == NULL) return -1;
    }

    /*
     * If this if the first time the drawable has been used by PEX/GL/MBUF then
     * we need to set up the front buffer and initialize the data structure 
     */
    if ( buffers->frontLeftBuf == NULL )
    {
	buffers->pDraw = pDraw;
	buffers->pScreen = pDraw->pScreen;
	buffers->drawSerial = 0;
	buffers->savedBufferMask = 0;
	buffers->devClips.num = buffers->devClips.maxClips = 0;
	buffers->extClips = NULL;
	buffers->devClips.clips = NULL;
        buffers->updateHint = D_UpdateHintUndefined; 

        bcopy(config, &buffers->config, sizeof(D_Config));

	buffers->frontRightBuf = NULL;
	buffers->currentBackBuf = 0;
        buffers->numBackBufs = 0;

        if ( (buffers->frontLeftBuf = (*drawOps->createBuffer)
		( buffers->pDraw, D_FRONT_LEFT_BUFFER_MASK, &backDraw)) == NULL)
        {
            return -1; /* no bufs created */
	}
    }
    /*
     * If not first time through then check to see if more buffers are 
     * requested. 
     */
    else
    {
	if (config->depth) 
	    buffers->config.depth = TRUE;
	if (config->stencil) 
	    buffers->config.stencil = TRUE;
	if (config->alpha) 
	    buffers->config.alpha = TRUE;
	if (config->accum) 
	    buffers->config.accum = TRUE;

    }

    /* 
     * Check if stereo was enabled 
     */
    status = Success;
    if ( buffers->config.stereo && buffers->frontRightBuf == NULL)
    {
	if ( (buffers->frontRightBuf = (*drawOps->createBuffer) 
		(buffers->pDraw, D_FRONT_RIGHT_BUFFER_MASK, &backDraw)) == NULL)
        {
 	    return -1; /* no bufs created */
	}

        for (i = 0; i < buffers->numBackBufs; i++)
        {
	    if ( (buffers->backRightBuf[i] = (*drawOps->createBuffer)
		 ( buffers->pDraw, D_BACK_RIGHT_BUFFER_MASK, &backDraw) ) == NULL)
	    {
	        (*drawOps->freeBuffer) (buffers->frontRightBuf);
 	        status = !Success;
                break;
	    }
	    buffers->backRightDraw[i] = backDraw;
	}
	if ( status != Success)
	{
	    for ( ; i > 0; i--)
		if ( buffers->backRightBuf[i] != NULL)
		    (*drawOps->freeBuffer) (buffers->backRightBuf[i]);

	    return -1; /* no bufs created */
	}
    }

    /*
     * Bump refcnt of existing back buffers
     */
    i = min( buffers->numBackBufs, buffers->config.numBackBufs);
    while (--i >= 0)
	buffers->refcnts[i]++;

    /*
     * Allocate additional back buffers now. Ancillary buffers aren't 
     * allocated until used
     */
    for (i = buffers->numBackBufs; i < buffers->config.numBackBufs; i++)
    {
	if ( (buffers->backLeftBuf[i] = (*drawOps->createBuffer)
		( buffers->pDraw, D_BACK_LEFT_BUFFER_MASK, &backDraw) ) == NULL)
        {
 	    status = !Success;
            break;
        }
	buffers->backLeftDraw[i] = backDraw;

        if ( buffers->config.stereo)
        {
	    if ( (buffers->backRightBuf[i] = (*drawOps->createBuffer)
		 ( buffers->pDraw, D_BACK_RIGHT_BUFFER_MASK, &backDraw) ) == NULL)
	    {
	       (*drawOps->freeBuffer)(buffers->backLeftBuf[i]);
 	        status = !Success;
                break;
	    }
	    buffers->backRightDraw[i] = backDraw;
	}
	buffers->refcnts[i] = 1;
    }

    buffers->numBackBufs = i;
    return (buffers->numBackBufs);
}


int D_FreeBackBuffers( buffers, nBuf)
D_BufferPtr     buffers;
int             nBuf;
{
    int i, buf;
    D_Func              *drawOps = D_DRAWOP(buffers->pScreen);

    if (nBuf > buffers->numBackBufs) return FALSE;

    buf = buffers->numBackBufs - 1;
    for (i = 0; i < nBuf; i++, buf--)
    {
        if (--buffers->refcnts[buf] <= 0)
	{
	    (*drawOps->freeBuffer) ( buffers->backLeftBuf[buf]);
	    buffers->backLeftBuf[buf] = NULL;

            if ( buffers->config.stereo )
	    {
	        (*drawOps->freeBuffer) ( buffers->backRightBuf[buf]);
		buffers->backRightBuf[buf] = NULL;
	    }
	}
    }
    buffers->numBackBufs -= nBuf;
}


int D_FreeDrawBuffers( buffers)
D_BufferPtr     buffers;
{
    D_Func              *drawOps = D_DRAWOP(buffers->pScreen);
    int			i;

    if (buffers->extClips != NULL )
    {
        (*buffers->pScreen->RegionDestroy)(buffers->extClips);
	buffers->extClips = NULL;
    }

    if (buffers->devClips.maxClips > 0)
    {
        Xfree(buffers->devClips.clips);
	buffers->devClips.clips = NULL;
    }

    for (i=0; i < buffers->numBackBufs; i++) {
        (*drawOps->freeBuffer)( buffers->backLeftBuf[i]);
	buffers->backLeftBuf[i] = NULL;
        if ( buffers->config.stereo )
	{
            (*drawOps->freeBuffer)( buffers->backRightBuf[i]);
	    buffers->backRightBuf[i] = NULL;
	}
    }
    buffers->numBackBufs = 0;

    if ( buffers->frontRightBuf )
    {
        (*drawOps->freeBuffer)( buffers->frontRightBuf);
	buffers->frontRightBuf = NULL;
    }
    if ( buffers->depthBuf )
    {
        (*drawOps->freeBuffer)( buffers->depthBuf);
	buffers->depthBuf = NULL;
    }
    if ( buffers->alphaFrontLeftBuf )
    {
        (*drawOps->freeBuffer)( buffers->alphaFrontLeftBuf);
	buffers->alphaFrontLeftBuf = NULL;
    }
    if ( buffers->alphaBackLeftBuf )
    {
        (*drawOps->freeBuffer)( buffers->alphaBackLeftBuf);
	buffers->alphaBackLeftBuf = NULL;
    }
    if ( buffers->alphaFrontRightBuf )
    {
        (*drawOps->freeBuffer)( buffers->alphaFrontRightBuf);
	buffers->alphaFrontRightBuf = NULL;
    }
    if ( buffers->alphaBackRightBuf )
    {
        (*drawOps->freeBuffer)( buffers->alphaBackRightBuf);
	buffers->alphaBackRightBuf = NULL;
    }
    if ( buffers->stencilBuf )
    {
        (*drawOps->freeBuffer)( buffers->stencilBuf);
	buffers->stencilBuf = NULL;
    }
    if ( buffers->accumBuf )
    {
        (*drawOps->freeBuffer)( buffers->accumBuf);
	buffers->accumBuf = NULL;
    }
    if ( buffers->frontLeftBuf )
    {
        (*drawOps->freeBuffer)( buffers->frontLeftBuf);
	buffers->frontLeftBuf = NULL;
    }
}


int _DResizeVMBuffer( bufDesc, w, h)
D_BufferAttr *bufDesc;
unsigned short w;
unsigned short h;
{
    if ( bufDesc->width != w || bufDesc->height != h )
    {
        bufDesc->addr = (HostVirtAddrB) 
	    Xrealloc( bufDesc->addr, w * h * (bufDesc->depth>>3));
        if (bufDesc->addr == NULL) return FALSE;

        bzero( bufDesc->addr, w * h * (bufDesc->depth>>3));
        bufDesc->width = w;
        bufDesc->height = h;
    }
}


int D_SetDrawableClip (buffers, myClips)
D_BufferPtr buffers;
RegionPtr myClips;
{
    if (buffers->extClips != NULL)
        (*buffers->pScreen->RegionDestroy)( buffers->extClips);

    buffers->extClips = myClips;
    buffers->drawSerial = 0;
    buffers->savedBufferMask = 0;

    return TRUE;
}


int D_SetUpdateHint (buffers, updateHint)
D_BufferPtr buffers;
int updateHint;
{
    buffers->updateHint = updateHint;

    return TRUE;
}


int D_GetUpdateHint (buffers)
D_BufferPtr buffers;
{
    return (buffers->updateHint);
}


int D_SetCurrentBackBuf (buffers, currentBackBuf)
D_BufferPtr buffers;
int currentBackBuf;
{
    buffers->currentBackBuf = currentBackBuf;

    return TRUE;
}


int D_GetCurrentBackBuf (buffers)
D_BufferPtr buffers;
{
    return (buffers->currentBackBuf);
}


int D_GetConfig( visual, config)
VisualID        visual;
D_ConfigPtr     config;
{
    /*
     * XXX: Base this on the visual
     */
    config->numBackBufs = 1;
    config->stereo = FALSE;
    config->depth = TRUE;
    config->stencil = TRUE;
    config->alpha = TRUE;
    config->accum = TRUE;
}

int D_ValidateBuffers( buffers, enableMask)
D_BufferPtr     buffers;
int             enableMask;
{
    D_Func              *drawOps = D_DRAWOP(buffers->pScreen);
    DrawablePtr         tempDraw;

    /*
     * Mask indicates which other buffers are about to be used
     */
    if ( enableMask & D_DEPTH_BUFFER_MASK && buffers->depthBuf == NULL )
    {
        buffers->depthBuf = (*drawOps->createBuffer)
            ( buffers->pDraw, D_DEPTH_0_BUFFER_MASK, &tempDraw);
    }

    if ( enableMask & D_FRONT_LEFT_ALPHA_BUFFER_MASK && 
         buffers->alphaFrontLeftBuf == NULL )
    {
        buffers->alphaFrontLeftBuf = (*drawOps->createBuffer)
	    ( buffers->pDraw, D_FRONT_LEFT_ALPHA_BUFFER_MASK, &tempDraw);
    }

    if ( enableMask & D_BACK_LEFT_ALPHA_BUFFER_MASK && 
         buffers->alphaBackLeftBuf == NULL )
    {
        buffers->alphaBackLeftBuf = (*drawOps->createBuffer)
            ( buffers->pDraw, D_BACK_LEFT_ALPHA_BUFFER_MASK, &tempDraw);
    }

    if ( enableMask & D_FRONT_RIGHT_ALPHA_BUFFER_MASK && 
         buffers->alphaFrontRightBuf == NULL)
    {
        buffers->alphaFrontRightBuf = (*drawOps->createBuffer)
            ( buffers->pDraw, D_FRONT_RIGHT_ALPHA_BUFFER_MASK, &tempDraw);
    }

    if ( enableMask & D_BACK_RIGHT_ALPHA_BUFFER_MASK && 
         buffers->alphaBackRightBuf == NULL )
    {
        buffers->alphaBackRightBuf = (*drawOps->createBuffer)
	    ( buffers->pDraw, D_BACK_RIGHT_ALPHA_BUFFER_MASK, &tempDraw);
    }

    if ( enableMask & D_STENCIL_BUFFER_MASK && buffers->stencilBuf == NULL)
    {
        buffers->stencilBuf = (*drawOps->createBuffer)
            ( buffers->pDraw, D_STENCIL_BUFFER_MASK, &tempDraw);
    }

    if ( enableMask & D_ACCUM_BUFFER_MASK && buffers->accumBuf == NULL )
    {
        buffers->accumBuf = (*drawOps->createBuffer)
            ( buffers->pDraw, D_ACCUM_BUFFER_MASK, &tempDraw);
    }
}


int _DValidateClip( buffers)
D_BufferPtr     buffers;
{
    ScreenPtr     pScreen = buffers->pScreen;
    RegionPtr     drawClip;
    RegionPtr     tempRegion = NULL;

    if (buffers->pDraw->type ==  DRAWABLE_WINDOW)
    {
        drawClip = &(((WindowRec *)buffers->pDraw)->clipList);
    }
    else /* for pixmaps and back buffers there is no clip list*/
    {
        drawClip = NULL;
    }

    if ( drawClip == NULL && buffers->extClips == NULL )
    {
        buffers->devClips.num = 0;
        if (buffers->devClips.maxClips > 0)
              Xfree(buffers->devClips.clips);
        buffers->devClips.clips = NULL;
        buffers->devClips.maxClips = 0;
    }
    /* Intersect clip lists and make clips window relative */
    else /* non-null composite */
    {
        if (drawClip != NULL)
        {
            tempRegion = (*pScreen->RegionCreate)( (BoxPtr)NULL, 0);
            (*pScreen->RegionCopy)( tempRegion, drawClip);

            (*pScreen->TranslateRegion)(tempRegion, -buffers->pDraw->x, 
		-buffers->pDraw->y);

            if (buffers->extClips != NULL)
      		(*pScreen->Intersect)( tempRegion, tempRegion, 
		     buffers->extClips);

            drawClip = tempRegion;

	}
        else /* only extension clips */
	    drawClip = buffers->extClips;

	buffers->devClips.num = REGION_NUM_RECTS( drawClip);
        if (buffers->devClips.num > buffers->devClips.maxClips)
        {
            if (buffers->devClips.maxClips > 0)
                Xfree(buffers->devClips.clips);

            buffers->devClips.maxClips = 
			(buffers->devClips.num > MIN_CLIPS_IN_LIST) ?
                        (buffers->devClips.num) : MIN_CLIPS_IN_LIST;

            buffers->devClips.clips = (void *) 
			Xalloc( buffers->devClips.maxClips * sizeof(BoxRec));
        }
        
        if (buffers->devClips.num > 0)
        {
            bcopy(REGION_RECTS(drawClip), buffers->devClips.clips, 
		  buffers->devClips.num * sizeof(BoxRec));
        }

        if (tempRegion != NULL)
            (*pScreen->RegionDestroy)( tempRegion);
    }
}


int D_SaveRenderState ( buffers)
D_BufferPtr     buffers;
{
    return TRUE;
}


int D_RestoreRenderState ( buffers)
D_BufferPtr     buffers;
{
    return TRUE;
}
