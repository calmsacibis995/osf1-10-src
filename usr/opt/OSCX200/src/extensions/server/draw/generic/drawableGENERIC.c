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
/** COPYRIGHT (c) 1993                                                      */
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
 *     drawableGENERIC.c
 *
 *  Author:
 *     LVO 2/2/93
 *
 *  NOTE: This only supports backBuffers. 
 *
 */

#include "X.h"
#include "Xmd.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "servermd.h"
#include "gcstruct.h"

#define _MULTIBUF_SERVER_       /* don't want Xlib structures */
#include "multibuf.h"
#include "draw.h"
#include "drawint.h"

#define MIN_CLIPS_IN_LIST       (10)


D_BufferAttr *D_GENERICCreateBuffer (pDraw, bufType, bufferDraw)
DrawablePtr pDraw; 
int bufType;
DrawablePtr *bufferDraw;
{
    D_DrawState  *drawState;
    D_BufferAttr *bufAttr;

    /*
     * Allocate and Initialize the data for each buffer.
     */
    drawState = (D_DrawState *) Xalloc ( sizeof(D_DrawState) + 
					sizeof(D_BufferAttr));
    if (drawState == NULL)
      return (D_BufferAttr *)NULL;

    drawState->pScreen = pDraw->pScreen;
    drawState->pDrawable = (DrawablePtr)NULL;

    bufAttr = (D_BufferAttr *) ( ((char *)drawState) + sizeof(D_DrawState) );
    bufAttr->addr = (HostVirtAddrB)NULL;
    bufAttr->devPriv = NULL;
    bufAttr->mode = NULL;
    bufAttr->type = bufType;
    bufAttr->depth = pDraw->depth;
    bufAttr->x = pDraw->x;
    bufAttr->y = pDraw->y;
    bufAttr->width = pDraw->width;
    bufAttr->height = pDraw->height;

    /* 
     * Create pixmap buffer 
     */
    if ((bufType & D_BACK_LEFT_BUFFER_MASK) ||
	(bufType & D_FRONT_LEFT_BUFFER_MASK) )
    {
	drawState->pDrawable = (void *)(*pDraw->pScreen->CreatePixmap)
	                                                   (pDraw->pScreen,
							    pDraw->width,
							    pDraw->height,
							    pDraw->depth) ;
	*bufferDraw = (DrawablePtr) drawState->pDrawable ;
	return (bufAttr) ;
    }
    else
    {
	Xfree(drawState);
	return ((D_BufferAttr *)NULL) ;
    }
}

void D_GENERICFreeBuffer (bufDesc)
D_BufferAttr *bufDesc;
{
    D_DrawState  *drawState;

    /*
     * Get the buffer data.
     */
    drawState = (D_DrawState *) ( ((char *)bufDesc) - sizeof(D_DrawState));

    if (bufDesc->type == DRAWABLE_PIXMAP)
      (*drawState->pScreen->DestroyPixmap) ((PixmapPtr)drawState->pDrawable);
}

static void
_DSetupBackgroundPainter (pWin, pGC)
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

int D_GENERICReadyBuffers (buffers, bufferMask, nBackBufs, backBufList,clipOut)
D_BufferPtr     buffers;
int	        bufferMask;
int	        nBackBufs;
int	        *backBufList;
D_ClipRects     *clipOut;
{
    WindowPtr	    pWin;
    ScreenPtr	    pScreen;
    int		    width, height;
    int		    i;
    int		    dx, dy, dw, dh;
    int		    sourcex, sourcey;
    int		    destx, desty;
    PixmapPtr	    pPixmap;
    GCPtr	    pGC;
    int		    savewidth, saveheight;
    xRectangle	    clearRect;
    Bool	    clear;
    D_BufferAttr    *buffer;
    D_DrawState	    *drawState;

    /*
     * Initialize variables.
     */
    pWin = (WindowPtr) buffers->pDraw;
    pScreen = buffers->pScreen;

    width = pWin->drawable.width;
    height = pWin->drawable.height;

    /*
     * if the with and height has changed, but not all the
     * windows have been requested to be updated, return an error -
     * if the size of the window changes, the client HAS to update
     * all the buffers to match it.
     *
     * note: this should never happen when the generic drawlib is called
     * from multibuf, but I put in this check for future. 
     */
    if ((width != buffers->backLeftDraw[backBufList[0]]->width ||
	height != buffers->backLeftDraw[backBufList[0]]->height) &&
	nBackBufs != buffers->numBackBufs)
	return FALSE;
    
    /*
     * check to see if the first back buffer (which should be the
     * same as all the rest) size is the same as the drawable.
     */
    if (buffers->backLeftDraw[backBufList[0]]->width == width &&
        buffers->backLeftDraw[backBufList[0]]->height == height)
	return TRUE;
    
    /*
     * take data from the first back buffer in the list since
     * all of them should be the same.
     */
    dx = pWin->drawable.x - buffers->backLeftDraw[backBufList[0]]->x;
    dy = pWin->drawable.y - buffers->backLeftDraw[backBufList[0]]->y;
    dw = width - buffers->backLeftDraw[backBufList[0]]->width;
    dh = height - buffers->backLeftDraw[backBufList[0]]->height;
    
    GravityTranslate (0, 0, -dx, -dy, dw, dh,
		      pWin->bitGravity, &destx, &desty);
    clear = buffers->backLeftDraw[backBufList[0]]->width < width ||
	buffers->backLeftDraw[backBufList[0]]->height < height ||
	    pWin->bitGravity == ForgetGravity;

    sourcex = 0;
    sourcey = 0;
    savewidth = buffers->backLeftDraw[backBufList[0]]->width ;
    saveheight = buffers->backLeftDraw[backBufList[0]]->height ;

    /*
     * clip rectangle to source and destination
     */
    if (destx < 0)
    {
	savewidth += destx;
	sourcex -= destx;
	destx = 0;
    }
    if (destx + savewidth > width)
	savewidth = width - destx;
    if (desty < 0)
    {
	saveheight += desty;
	sourcey -= desty;
	desty = 0;
    }
    if (desty + saveheight > height)
	saveheight = height - desty;

    pGC = GetScratchGC (pWin->drawable.depth, pScreen);
    if (clear)
    {
	_DSetupBackgroundPainter (pWin, pGC);
	clearRect.x = 0;
	clearRect.y = 0;
	clearRect.width = width;
	clearRect.height = height;
    }
    for (i = 0; i < nBackBufs; i++)
    {
	buffer = buffers->backLeftBuf[backBufList[i]];
	/*
	 * save the new size and location.
	 */
	buffer->width = width;
	buffer->height = height;
	buffer->x = pWin->drawable.x;
	buffer->y = pWin->drawable.y;

	/*
	 * create new pixmap.
	 */
	pPixmap = (*pScreen->CreatePixmap) (pScreen, width,
					    height, pWin->drawable.depth);
	if (!pPixmap)
	{
	    /*
	     * destroy all the buffers and return.
	     */
	    D_FreeDrawBuffers(buffers);
	    FreeScratchGC(pGC);
	    return FALSE;
	}
	
	ValidateGC (pPixmap, pGC);
	/*
	 * I suppose this could avoid quite a bit of work if
	 * it computed the minimal area required.
	 */
	if (clear)
	    (*pGC->ops->PolyFillRect) (pPixmap, pGC, 1, &clearRect);
	if (pWin->bitGravity != ForgetGravity)
	{
	    /*
	     * copy the contents of old buffer to the new buffer.
	     */
	    (*pGC->ops->CopyArea) ((PixmapPtr)
				       buffers->backLeftDraw[backBufList[i]],
				   pPixmap, pGC,
				   sourcex, sourcey, savewidth, saveheight,
				   destx, desty);
	}
	/*
	 * set new id; destroy old pixmap; set new pixmap.
	 */
	pPixmap->drawable.id = buffers->backLeftDraw[backBufList[i]]->id;
	(*pScreen->DestroyPixmap) (buffers->backLeftDraw[backBufList[i]]);
	buffers->backLeftDraw[backBufList[i]] = (DrawablePtr) pPixmap;
	/*
	 * store off the pixmap in the drawState so that the freeBuffer will
	 * work. 
	 */
	drawState = (D_DrawState *) ( ((char *)buffer) - sizeof(D_DrawState));
	drawState->pDrawable =  (DrawablePtr) pPixmap;
    }
    FreeScratchGC (pGC);
    return TRUE;
}

int D_GENERICDisplayBuffer ( buffers, updateAction, backBufNum, eventMask,
			    ppExpose )
D_BufferPtr     buffers;
int	        updateAction;
int             backBufNum;
Mask		eventMask;
RegionPtr	*ppExpose;
{

    DrawablePtr  pFront, pBack;
    
    WindowPtr	 pWin; 
    GC           *pGC;
    xRectangle	 clearRect;
    RegionPtr	 pExposed;
    XID		 bool;
    XID          gcfunc;

    if ( backBufNum > buffers->numBackBufs )
      return FALSE;

    pFront = buffers->pDraw;
    
    if (backBufNum == -1)
    {
	pBack = pFront;
    }
    else
    {
	pBack = buffers->backLeftDraw[backBufNum];
    }
    

    if ((!pFront) || (!pBack))
    {
	return FALSE;
    }

    /*
     * get scratch gc.
     */
    if ((pGC = GetScratchGC (buffers->pDraw->depth, buffers->pScreen)) == NULL)
    {
	return FALSE;
    }
    

    if ((updateAction == MultibufferUpdateActionUntouched) &&
	(backBufNum != -1))
    {
	/*
        ** swap the contents of the window with those of the back buffer
        **
	** Swap buffer is done as 3 copy areas with Xor as the function:
	**
	** Stage 1:
	**  W contains      F
	**  P contains      B
	** Step 1: Xor W into P
	**  W contains      F
	**  P contains      B^F
	** Step 2: Xor P into W
	**  W contains      F^(B^F) == B  (back image displayed in window)
	**  P contains      B^F
	** Step 3: Xor W into P
	**  W contains      B
	**  P contains      (B^F)^B == F  (front image contained in pixmap)
        **
	*/

	if (eventMask & ExposureMask)
	{
	    bool = TRUE;
	    DoChangeGC (pGC, GCGraphicsExposures, &bool, FALSE);
	}
	
        gcfunc = GXxor;
        DoChangeGC(pGC, GCFunction, &gcfunc, FALSE);
        ValidateGC(pBack, pGC);
	
        pExposed = (*pGC->ops->CopyArea)(pFront, 
					 pBack, 
					 pGC, 
					 0, 
					 0,
					 buffers->pDraw->width, 
					 buffers->pDraw->height, 
					 0, 
					 0);
	
	if (eventMask & ExposureMask)
	{
	    if (pExposed && (pFront->type == DRAWABLE_WINDOW))
	    {
		RegionPtr          pWinSize;
		extern RegionPtr   CreateUnclippedWinSize();
		
		
		pWinSize = CreateUnclippedWinSize(pFront);

		(*pFront->pScreen->Intersect)(pExposed,
					      pExposed,
					      pWinSize);
		(*pFront->pScreen->RegionDestroy) (pWinSize);
	    }

	    bool = FALSE;
	    DoChangeGC(pGC, GCGraphicsExposures, &bool, FALSE);
	}
	*ppExpose = pExposed;


        ValidateGC(pFront, pGC);
        
	(*pGC->ops->CopyArea)(pBack, 
			      pFront, 
			      pGC, 
			      0, 
			      0,
			      buffers->pDraw->width, 
			      buffers->pDraw->height, 
			      0, 
			      0);
	
        ValidateGC(pBack, pGC);

        (*pGC->ops->CopyArea)(pFront, 
			      pBack, 
			      pGC,
			      0, 
			      0,
			      buffers->pDraw->width, 
			      buffers->pDraw->height, 
			      0, 
			      0);
    }
    else if (backBufNum != -1)
    {
	/*
	** copy the back buffer to the front.
	*/
	
	ValidateGC(pFront, pGC);
	
	(*pGC->ops->CopyArea)(pBack,
			      pFront, 
			      pGC, 
			      0, 
			      0,
			      pFront->width,
			      pFront->height,
			      0,
			      0);
    }
    

    if (updateAction ==  MultibufferUpdateActionBackground)
    {
	/* 
        ** paint the back buffer with the window's background color.
        */
	_DSetupBackgroundPainter (pFront, pGC);
	ValidateGC(pBack, pGC);
	clearRect.x = 0;
	clearRect.y = 0;
	clearRect.width = pBack->width;
	clearRect.height = pBack->height;
	(*pGC->ops->PolyFillRect)(pBack, pGC, 1, &clearRect);
    }




    if (backBufNum != -1)
    {
	buffers->currentBackBuf = backBufNum;
    }
    
    

    FreeScratchGC(pGC);


    return TRUE;
}
