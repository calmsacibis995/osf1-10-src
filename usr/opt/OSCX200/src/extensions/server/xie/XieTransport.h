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

/*******************************************************************************
**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**  
**                          All Rights Reserved
**  
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**  
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**  
*******************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**	X Imaging Extension DIX
**
**  ABSTRACT:
**
**      This is the header file for the DIX DISPLAY module.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      August 11, 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

    /*
    **	Transport Context structure.
    */
typedef struct  _XportRec {
    CARD8	 Mode;			 /* GetStream, PutStream, or Tile   */
    CARD8	_pad;
    CARD8	 CmpOrg;		 /* component organization	    */
    CARD8	 Cmpres;		 /* compression scheme		    */
    CARD32	 CmpPrm;		 /* compression parameter	    */
    CARD32	 PlnMsk;		 /* plane mask: initialized planes  */
    CARD32	 FinMsk;		 /* plane mask: completed planes    */
    CARD32	 MaxMsk;		 /* plane mask: all possible planes */
    PipeSinkPtr  Snk;			 /* Sink to transport to or from    */
    PipeDataPtr  Dat[XieK_MaxComponents];/* Photoflo data descriptors	    */
    UdpPtr	 Udp[XieK_MaxPlanes];	 /* Client plane descriptors	    */
} XportRec;
    /*
    **  Convenience macros for accessing XportPtr fields.
    */
#define xpMode_(xp)	    ((xp)->Mode)
#define   xpIsGetStream_(xp)  (xpMode_(xp)==XieK_GetStream)
#define   xpIsPutStream_(xp)  (xpMode_(xp)==XieK_PutStream)
#define   xpIsTile_(xp)       (xpMode_(xp)==XieK_Tile)
#define xpCmpOrg_(xp)       ((xp)->CmpOrg)
#define   xpIsBndPxl_(xp)     (xpCmpOrg_(xp)==XieK_BandByPixel)
#define   xpIsBndPln_(xp)     (xpCmpOrg_(xp)==XieK_BandByPlane)
#define   xpIsBitPln_(xp)     (xpCmpOrg_(xp)==XieK_BitByPlane)
#define xpCmpres_(xp)       ((xp)->Cmpres)
#define   xpIsDCT_(xp)	      (xpCmpres_(xp)==XieK_DCT)
#define   xpIsG31D_(xp)	      (xpCmpres_(xp)==XieK_G31D)
#define   xpIsG32D_(xp)	      (xpCmpres_(xp)==XieK_G32D)
#define   xpIsG42D_(xp)	      (xpCmpres_(xp)==XieK_G42D)
#define   xpIsPCM_(xp)	      (xpCmpres_(xp)==XieK_PCM)
#define xpCmpPrm_(xp)       ((xp)->CmpPrm)
#define xpPlnMsk_(xp)	    ((xp)->PlnMsk)
#define xpFinMsk_(xp)       ((xp)->FinMsk)
#define xpMaxMsk_(xp)       ((xp)->MaxMsk)
#define xpSnk_(xp)	    ((xp)->Snk)
#define xpDat_(xp,comp)	    ((xp)->Dat[(comp)])
#define xpUdp_(xp,plane)    ((xp)->Udp[(plane)])
#define   xpPxlLen_(xp,plane) (xpUdp_(xp,plane)->UdpW_PixelLength)
#define   xpDType_(xp,plane)  (xpUdp_(xp,plane)->UdpB_DType)
#define   xpClass_(xp,plane)  (xpUdp_(xp,plane)->UdpB_Class)
#define   xpBase_(xp,plane)   (xpUdp_(xp,plane)->UdpA_Base)
#define   xpArSize_(xp,plane) (xpUdp_(xp,plane)->UdpL_ArSize)
#define   xpPxlStr_(xp,plane) (xpUdp_(xp,plane)->UdpL_PxlStride)
#define   xpScnStr_(xp,plane) (xpUdp_(xp,plane)->UdpL_ScnStride)
#define   xpX1_(xp,plane)     (xpUdp_(xp,plane)->UdpL_X1)
#define   xpX2_(xp,plane)     (xpUdp_(xp,plane)->UdpL_X2)
#define   xpY1_(xp,plane)     (xpUdp_(xp,plane)->UdpL_Y1)
#define   xpY2_(xp,plane)     (xpUdp_(xp,plane)->UdpL_Y2)
#define   xpWidth_(xp,plane)  (xpUdp_(xp,plane)->UdpL_PxlPerScn)
#define   xpHeight_(xp,plane) (xpUdp_(xp,plane)->UdpL_ScnCnt)
#define   xpPos_(xp,plane)    (xpUdp_(xp,plane)->UdpL_Pos)
#define   xpCmpIdx_(xp,plane) (xpUdp_(xp,plane)->UdpL_CompIdx)
#define   xpLvl_(xp,plane)    (xpUdp_(xp,plane)->UdpL_Levels)

/*
**  Structure definitions and Typedefs
*/
    /*
    **  GetStream Transport Pipe Context.
    */
typedef struct _GetStreamPipeCtx {
    PipeElementCommonPart    common;
    struct _GetStreamPart {
	PhotomapPtr	Map;			    /* source Photo{map|tap}*/
	PipeSinkRec	Cvt;			    /* data conversion sink */
	UdpRec		Udp[XieK_MaxComponents];    /* copy of drains' Udp  */
	PipeDrainPtr	Drn[XieK_MaxComponents];    /* drains		    */
	PipeDataPtr	Dat[XieK_MaxComponents];    /* drains' data	    */
	CARD8	        DrnCnt;			    /* number of drains	    */
	CARD8	       _pad[3];
    } GetStreamPart;
} GetStreamPipeCtx, *GetStreamPipeCtxPtr;
    /*
    **  Pipeline context, GetStream part, access macros
    */
#define GsMap_(ctx)	    ((ctx)->GetStreamPart.Map)
#define GsCvt_(ctx)	    ((ctx)->GetStreamPart.Cvt)
#define GsUdp_(ctx,comp)    ((ctx)->GetStreamPart.Udp[(comp)])
#define GsDrn_(ctx,comp)    ((ctx)->GetStreamPart.Drn[(comp)])
#define GsDat_(ctx,comp)    ((ctx)->GetStreamPart.Dat[(comp)])
#define GsDrnCnt_(ctx)	    ((ctx)->GetStreamPart.DrnCnt)

