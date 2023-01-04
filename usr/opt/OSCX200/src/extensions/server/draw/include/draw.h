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
 *     draw.h
 *
 *  Author:
 *     MPW 6/9/92 
 *
 */

#ifndef _DRAW_H_
#define _DRAW_H_ 

#include "regionstr.h"
#include "X.h"
#include "pixmap.h"
#include "drawbuf.h"
extern int D_InitDraw();

#define D_UpdateHintUndefined  -1

/* 
 * global data -- shared by all incarnations of drawlib
 */
#define D_GlobalResName "DRAWLIB_SCREENINFO"
typedef struct {
     int screenPrivateIndex;
     int windowPrivateIndex;
     int numScreens;
     ScreenPtr screens[MAXSCREENS];
} D_ScreenInfo;

/* 
 * screen private data 
 */
extern int _DScreenPrivateIndex;

typedef struct 
{
    D_BufferAttr    *(*createBuffer)();
    void	    (*freeBuffer)();
    int		    (*initDrawBuffers)();
    int             (*freeDrawBuffers)();
    int             (*freeBackBuffers)();
    int             (*setDrawableClip)();
    int             (*setUpdateHint)();
    int             (*getUpdateHint)();
    int             (*setCurrentBackBuf)();
    int             (*getCurrentBackBuf)();
    int             (*getConfig)();
    int             (*validateBuffers)();
    int             (*readyBuffers)();
    int             (*saveRenderState)();
    int             (*restoreRenderState)();
    int             (*displayBuffer)();
} D_Func, *D_FuncPtr;

#define MAX_OPTION_NAME (10)
#define MAX_BUFFER_NAME (20)

typedef struct
{
    int		    stereo;
    int 	    colorBufferSize;
    int 	    depthBufferSize;
    int 	    stencilBufferSize;
    int 	    alphaBufferSize;
    int 	    accumRedSize;
    int 	    accumGreenSize;
    int 	    accumBlueSize;
    int 	    accumAlphaSize;
    int 	    maxBackBuffers;
    int		    fullDevCoordAddr;
    char            rendererName[MAX_OPTION_NAME];
    char            osmbufferDesc[MAX_BUFFER_NAME];
    int	            LGIDevType;
    char	    *LGIDevData;
    char	    *devData;
} D_BufferParam, *D_BufferParamPtr;

typedef struct 
{
    D_Func              drawOps;
    D_BufferParam       bufParam;
    Bool                (*WrappedCreateWindow)();
    Bool                (*WrappedDestroyWindow)();
} D_ScreenPriv, *D_ScreenPrivPtr;

#define D_SCREENPRIV(_pscreen_) \
  ( (D_ScreenPrivPtr) ((_pscreen_)->devPrivates[_DScreenPrivateIndex].ptr) )

#define D_DRAWOP(_pscreen_) \
  ( (D_Func *) &(D_SCREENPRIV(_pscreen_)->drawOps) )

#define D_BUFPARAM(_pscreen_) \
  ( (D_BufferParam *) &(D_SCREENPRIV(_pscreen_)->bufParam) )

/* 
 * window private data 
 */
extern int _DWindowPrivateIndex;

typedef struct
{
    int		    numBackBufs;
    int             stereo;
    int             depth;
    int             stencil;
    int             alpha;
    int             accum;
} D_Config, *D_ConfigPtr;

typedef struct
{
    int		    num;
    int		    maxClips;
    void	    *clips;
} D_ClipRects, *D_ClipRectsPtr;

#define D_MAXBUFFERS 	11
#define D_MAXBACKBUFS   20 /* XXX: reasonable? */

typedef struct 
{
    DrawablePtr           pDraw;
    long		  drawSerial;
    ScreenPtr		  pScreen;
    D_Config              config;
    RegionPtr		  extClips;
    D_ClipRects           devClips; 
    int			  savedBufferMask;
    int			  updateHint;
    int                   currentBackBuf;
    int                   numBackBufs;
    int			  refcnts[D_MAXBACKBUFS];
    DrawablePtr		  backLeftDraw[D_MAXBACKBUFS];
    DrawablePtr		  backRightDraw[D_MAXBACKBUFS];
    D_BufferAttr          *frontLeftBuf;
    D_BufferAttr          *backLeftBuf[D_MAXBACKBUFS];
    D_BufferAttr          *frontRightBuf;
    D_BufferAttr          *backRightBuf[D_MAXBACKBUFS];
    D_BufferAttr          *alphaFrontLeftBuf;
    D_BufferAttr          *alphaBackLeftBuf;
    D_BufferAttr          *alphaFrontRightBuf;
    D_BufferAttr          *alphaBackRightBuf;
    D_BufferAttr          *depthBuf;
    D_BufferAttr          *stencilBuf;
    D_BufferAttr          *accumBuf;
} D_Buffer, *D_BufferPtr;

#define D_WINDOWPRIV(_pwin_) \
  ( (D_BufferPtr) ((_pwin_)->devPrivates[_DWindowPrivateIndex].ptr) )

#define D_SET_WINDOWPRIV(_pwin_,_ptr_) \
  ( ((_pwin_)->devPrivates[_DWindowPrivateIndex].ptr) = (unsigned char *) (_ptr_))

#endif /* DRAW_H */
