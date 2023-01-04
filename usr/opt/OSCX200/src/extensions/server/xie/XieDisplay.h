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
**      February 19, 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

    /*
    **  Core X Includes (to access Colormap, Drawable, GC, Screen, Visual...).
    */
#if defined(VAX) && !defined(MITR5)
#include <dmemorystr.h>
#endif
#include <colormapst.h>
#include <gcstruct.h>
#include <scrnintstr.h>
#include <windowstr.h>

    /*
    **  RGB record and state constants.
    */
typedef struct _RGB {
    CARD16	red;
    CARD16	grn;
    CARD16	blu;
    BOOL	status;
    unsigned	Pstate : 2;
    unsigned	Rstate : 2;
    unsigned	Gstate : 2;
    unsigned	Bstate : 2;
    Pixel	pixel;
    CARD32	distance;
} RGBRec, *RGBPtr;
#define PRIVATE	    0		/* private color cell or pending allocation */
#define FREE	    1		/* available color cell			    */
#define SHARED	    2		/* sharable color cell			    */
#define OWNED	    3		/* static color cell or allocated color	    */

    /*
    **  ExportContext.
    */
typedef struct _ExpCtxRec {
    ClientPtr	 Client;		    /* X11 client pointer	    */
    CARD32	 DrwId;			    /* X11 drawable id		    */
    CARD32	 GCId;			    /* X11 GC id		    */
    CARD32	 CmapId;		    /* X11 Colormap id		    */
    ColormapPtr	 Cmap;			    /* X11 Colormap		    */
    PhotomapPtr	 LutMap;		    /* LUT for pixel mapping	    */
    UdpPtr	 PixUdp;		    /* final export ZPixmap Udp	    */
    UdpPtr	 PreUdp;		    /* pre-format   ZPixmap Udp	    */
    double	 MchLmt;		    /* match_limit		    */
    double	 GryLmt;		    /* gray_limit		    */
    CARD32	*PxlLst;		    /* allocated pixel list	    */
    CARD32	 PxlCnt;		    /* allocated pixel count	    */
    RGBPtr	 MapLst;		    /* colors available list 	    */
    RGBPtr	 RGBLst;		    /* colors required  list 	    */
    CARD32	 RGBCnt;		    /* colors required count	    */
    CARD32	*Hst[XieK_MaxComponents];   /* histogram arrays		    */
    CARD32	 Len[XieK_MaxComponents];   /* RGB lengths		    */
    CARD32	 Lvl[XieK_MaxComponents];   /* RGB levels		    */
    CARD32	 Mul[XieK_MaxComponents];   /* RGB component multipliers    */
#if !defined(X11R3) || defined(DWV3)
    DDXPointPtr  SpanPoints;		    /* Span points array            */
    CARD32      *SpanWidths;                /* Span widths array            */
    GCPtr	 SpanGC;		    /* GC to use when drawing spans */
    CARD32	 SpanCount;		    /* Span entries allocated	    */
    CARD32	 SpanIndex;		    /* Curr position in span list   */
#endif
    CARD16	 RGBPol;		    /* RGB polarity		    */
    CARD8	 Depth;			    /* depth			    */
    unsigned	 Busy	   : 1;		    /* True == export in progress   */
    unsigned	 Masks     : 1;		    /* True == use RGB masks	    */
    unsigned	 PreFmt    : 1;		    /* True == pre-format to ZPixmap*/
    unsigned	 HstAll	   : 1;		    /* True == histogram entire src */
    unsigned	 _reserved : 4;
} ExpCtxRec;
    /*
    **  ExportContext access macros.
    */
#define eClient_(exp)	    ((exp)->Client)
#define   eClientDrw_(exp)    (eClient_(exp)->lastDrawable)
#define   eClientGC_(exp)     (eClient_(exp)->lastGC)
#define   eClientIdx_(exp)    (eClient_(exp)->index)
#define eDrwId_(exp)	    ((exp)->DrwId)
#define eGCId_(exp)	    ((exp)->GCId)
#define eCmapId_(exp)	    ((exp)->CmapId)
#define eCmap_(exp)	    ((exp)->Cmap)
#define   eClass_(exp)	      (eCmap_(exp)->class)
#define     eDynamicCmap_(exp)	(eCmap_(exp)->class&DynamicClass)
#define     eStaticCmap_(exp)	(!eDynamicCmap_(exp))
#define   eVisual_(exp)	      (eCmap_(exp)->pVisual)
#define     eRedMsk_(exp)	(eCmap_(exp)->pVisual->redMask)
#define     eGrnMsk_(exp)	(eCmap_(exp)->pVisual->greenMask)
#define     eBluMsk_(exp)	(eCmap_(exp)->pVisual->blueMask)
#define     eRedOff_(exp)	(eCmap_(exp)->pVisual->offsetRed)
#define     eGrnOff_(exp)	(eCmap_(exp)->pVisual->offsetGreen)
#define     eBluOff_(exp)	(eCmap_(exp)->pVisual->offsetBlue)
#define     eMapLen_(exp)	(eCmap_(exp)->pVisual->ColormapEntries)
#define eLutMap_(exp)	    ((exp)->LutMap)
#define ePixUdp_(exp)	    ((exp)->PixUdp)
#define ePreUdp_(exp)	    ((exp)->PreUdp)
#define eMchLmt_(exp)	    ((exp)->MchLmt)
#define eGryLmt_(exp)	    ((exp)->GryLmt)
#define ePxlLst_(exp)	    ((exp)->PxlLst)
#define ePxlCnt_(exp)	    ((exp)->PxlCnt)
#define eMapLst_(exp)	    ((exp)->MapLst)
#define eRGBLst_(exp)	    ((exp)->RGBLst)
#define eRGBCnt_(exp)	    ((exp)->RGBCnt)
#define eHst_(exp,comp)	    ((exp)->Hst[(comp)])
#define eLen_(exp,comp)	    ((exp)->Len[(comp)])
#define eLvl_(exp,comp)	    ((exp)->Lvl[(comp)])
#define eMul_(exp,comp)	    ((exp)->Mul[(comp)])
#if !defined(X11R3) || defined(DWV3)
#define eSpnPt_(exp)	    ((exp)->SpanPoints)
#define eSpnW_(exp)         ((exp)->SpanWidths)
#define eSpnGC_(exp)	    ((exp)->SpanGC)
#define eSpnCnt_(exp)	    ((exp)->SpanCount)
#define eSpnIdx_(exp)	    ((exp)->SpanIndex)
#endif
#define eRGBPol_(exp)	    ((exp)->RGBPol)
#define eDepth_(exp)	    ((exp)->Depth)
#define eBusy_(exp)	    ((exp)->Busy)
#define eMasks_(exp)	    ((exp)->Masks)
#define ePreFmt_(exp)	    ((exp)->PreFmt)
#define eHstAll_(exp)	    ((exp)->HstAll)

/*
**  Structure definitions and Typedefs
*/
    /*
    **  Export Pipe Context.
    */
typedef struct _ExportPipeCtx {
    PipeElementCommonPart    common;
    struct _ExportPart {
	PhotomapPtr	Src;			    /* source Photo{map|tap}*/
	PipeSinkRec	Cvt;			    /* data conversion sink */
	PipeDrainPtr	Drn[XieK_MaxComponents];    /* drains		    */
	PipeDataPtr	Dat[XieK_MaxComponents];    /* drains' data	    */
	INT32		DstX, DstY;		    /* drawable X,Y	    */
    } ExportPart;
} ExportPipeCtx, *ExportPipeCtxPtr;
    /*
    **  Pipeline context, Export part, access macros
    */
#define ExpSrc_(ctx)	    ((ctx)->ExportPart.Src)
#define ExpCvt_(ctx)	   (&(ctx)->ExportPart.Cvt)
#define ExpDrn_(ctx,comp)   ((ctx)->ExportPart.Drn[(comp)])
#define ExpDat_(ctx,comp)   ((ctx)->ExportPart.Dat[(comp)])
#define ExpDstX_(ctx)	    ((ctx)->ExportPart.DstX)
#define ExpDstY_(ctx)	    ((ctx)->ExportPart.DstY)

/*
**  MACRO definitions
*/
    /*
    **  MACRO to compute an RGB component value:
    **      Component value 
    **          * the maximum dst value that can be expressed (65535)
    **          / the maximum src value for component
    **          ^ polarity (0x0000 == normal, 0xffff == invert)
    */
#define ScaleColor_(exp,value,comp)  (eLvl_(exp,comp) < 2 ? 0 : \
	           (value) * 65535 / (eLvl_(exp,comp) - 1) ^ eRGBPol_(exp))

    /*
    **  MACRO to extract an RGB component value from a ZPixmap pixel:
    **      pixel / "component multiplier" % "number of levels for component"
    */
#define ExtractColor_(exp,value,comp) ((value)/eMul_(exp,comp)%eLvl_(exp,comp))

    /*
    **  MACRO to set DirectColor RGB component values
    */
#define SetDirectColor_(exp,pxl,comp,rgb) \
		 (eLvl_(exp,comp) == 0 ? rgb->red : ScaleColor_(exp,pxl,comp))

    /*
    **  MACRO to extract PseudoColor RGB component values from a ZPixmap pixel
    */
#define SetPseudoColor_(exp,pxl,comp,rgb) \
	        ( eLvl_(exp,comp) == 0 ? rgb->red \
	        : ScaleColor_(exp,((pxl)/eMul_(exp,comp)%eLvl_(exp,comp)),comp))

