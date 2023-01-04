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
 *     drawinit.c
 *
 *  Author:
 *     LVO 2/2/93
 *
 *  Revisions: 
 */

#include <string.h>
#include <sys/types.h>


#include "X.h"
#include "Xmd.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "misc.h"
#include "scrnintstr.h"

#include "draw.h"
#include "drawint.h"
#include "drawableGENERIC.h"
#include "mi_drawable.h"

#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <X11/Xserver/ws.h>

extern int wsScreenPrivateIndex;

/* 
 * Called by drawlib initialization to initialize the screen priv data. This
 * contains pointers to drawlib fn and screen config info
 */
static int _DGENERICInitScreenPriv(pScreen, screenIndex, dfuncs, dBufParam)
ScreenPtr pScreen; 
int screenIndex; 
D_Func *dfuncs;
D_BufferParam *dBufParam;
{
    wsScreenPrivate * wsp = (wsScreenPrivate *)
	pScreen->devPrivates[wsScreenPrivateIndex].ptr;
    ws_screen_descriptor * screenDesc = wsp->screenDesc;

    dfuncs->createBuffer = D_GENERICCreateBuffer;
    dfuncs->freeBuffer = D_GENERICFreeBuffer;
    dfuncs->initDrawBuffers = D_InitDrawBuffers;
    dfuncs->freeDrawBuffers = D_FreeDrawBuffers;
    dfuncs->freeBackBuffers = D_FreeBackBuffers;
    dfuncs->setDrawableClip = D_SetDrawableClip;
    dfuncs->setUpdateHint = D_SetUpdateHint;
    dfuncs->getUpdateHint = D_GetUpdateHint;
    dfuncs->setCurrentBackBuf = D_SetCurrentBackBuf;
    dfuncs->getCurrentBackBuf = D_GetCurrentBackBuf;
    dfuncs->getConfig = D_GetConfig;
    dfuncs->validateBuffers = D_ValidateBuffers;
    dfuncs->readyBuffers = D_GENERICReadyBuffers;
    dfuncs->saveRenderState = D_SaveRenderState;
    dfuncs->restoreRenderState = D_RestoreRenderState;
    dfuncs->displayBuffer = D_GENERICDisplayBuffer;

    if (dBufParam != NULL) 
    {
/*
        dBufParam->LGIDevType = N_DevHX;  

        we're using a hard-coded constant (2) instead of N_DevHX, so that
        this file will NOT need to include (or depend on) and LGI files.

*/
        dBufParam->LGIDevType = 2;


	dBufParam->maxBackBuffers = MAX_BACK_BUFS;
	dBufParam->fullDevCoordAddr = FALSE;

	dBufParam->stereo = FALSE;
	dBufParam->stencilBufferSize = 0;
	dBufParam->alphaBufferSize = 0;
	dBufParam->accumRedSize = 0;
	dBufParam->accumGreenSize = 0;
	dBufParam->accumBlueSize = 0;
	dBufParam->accumAlphaSize = 0;

	/*
	 * must be a TX
	 */
	if (strncmp("PMAG-RO", screenDesc->moduleID, strlen("PMAG-RO")) == 0 ||
	    strncmp("PMAG-JA", screenDesc->moduleID, strlen("PMAG-JA")) == 0 )
	{
	    dBufParam->colorBufferSize = 24;
	    strcpy( dBufParam->osmbufferDesc, "I24 I24");
	}
	/*
	 * must be a 8 plane machine
	 */
	else
	{
	    dBufParam->colorBufferSize = 8;
	    strcpy( dBufParam->osmbufferDesc, "I8 I8");
	}
	dBufParam->depthBufferSize = 0;
	strcat( dBufParam->osmbufferDesc, " Z24");
	strcpy( dBufParam->rendererName, "GENERIC");
    }
    return TRUE;
}

/* 
 * drawlib initialization is called from ddx.  must do:
 *	allocate window and screen privates 
 *      initialize screen private to contain ptrs to draw fn and config info 
 */
int D_GENERICDrawInit(pScreen, screenIndex)
ScreenPtr pScreen; 
int screenIndex; 
{
    D_ScreenPrivPtr 	pScreenPriv;
   
    if (pScreen == NULL) return;

    /* Allocate window and screen privates */
    _DDrawInit();

    /* Initialize screen privates */
    pScreenPriv = _DInitScreenPriv(pScreen, screenIndex);
    if (pScreenPriv == NULL) return FALSE;

    if ( !_DGENERICInitScreenPriv(pScreen, screenIndex, &pScreenPriv->drawOps, 
				  &pScreenPriv->bufParam) ) 
    {
	ErrorF("Error initializing screen %d\n",screenIndex);
	return FALSE;
    }

    return TRUE;
}

/* 
 * Called by ddx when screen is closed
 */
void D_GENERICDrawClose(pScreen, screenIndex)
ScreenPtr pScreen; 
int screenIndex; 
{
    /* Un-allocate window and screen privates */
    _DDrawReset();
}














