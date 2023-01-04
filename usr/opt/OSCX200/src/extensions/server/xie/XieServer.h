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
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	The Xie module contains the definitions and structures used by the
**	XIE server.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Richard J. Piccolo
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      March 1, 1989
**
************************************************************************/

    /*
    **  Smi includes
    */
#include <SmiPipe.h>	/* sample machine independent pipeline definitions  */
#include <XieDdx.h>	/* DDX interface definitions			    */
#include <XieUdpDef.h>	/* universal data plane descriptor definitions	    */

#if defined(VMS) && !defined(ALPHA)
#define X11R3
#endif

#ifndef externalref
#ifdef VMS
#define externalref globalref
#else
#define externalref extern
#endif
#endif

#ifndef externaldef
#ifdef VMS
#define externaldef(psect) globaldef {"psect"} noshare
#else
#define externaldef(psect)
#endif
#endif

    /*
    **	Client connection context table.
    */
typedef struct _ClientTable {
    CARD32	event_mask;		    /* selected events 		    */
    CARD32	model;			    /* default constrain model	    */
    CARD32      levels[XieK_MaxComponents]; /* default constrain levels	    */
    } ClientTableRec, *ClientTablePtr;

    /*
    **	A byte of flags.
    */
typedef struct _Flags {
    unsigned	flag0 : 1;
    unsigned	flag1 : 1;
    unsigned	flag2 : 1;
    unsigned	flag3 : 1;
    unsigned	flag4 : 1;
    unsigned	flag5 : 1;
    unsigned	flag6 : 1;
    unsigned	flag7 : 1;
} FlagsRec, *FlagsPtr;

    /*
    **	Common part of all resource structures.
    */
typedef struct _CommonPart {
    CARD32	    Id;			/* X11 resource-id		    */
    CARD8	    Type;		/* Xie resource type		    */
    FlagsRec	    flags;		/* resource flags		    */
    INT16	    RefCnt;		/* resource reference count	    */
} CommonPartRec, *CommonPartPtr;
    /*
    **  Convenience macros for accessing CommonPartPtr fields.
    */
#define ResId_(res)	    (((CommonPartPtr)(res))->Id)
#define ResType_(res)	    (((CommonPartPtr)(res))->Type)
#define RefCnt_(res)	    (((CommonPartPtr)(res))->RefCnt)
    /*
    **  Convenience macros for determining resource types.
    */
#define IsPhotoflo_(res)    (ResType_(res)==XieK_Photoflo)
#define IsPhotomap_(res)    (ResType_(res)==XieK_Photomap)
#define IsPhototap_(res)    (ResType_(res)==XieK_Phototap)
#define IsPhoto_(res)	    (IsPhotomap_(res)||IsPhototap_(res))
#define IsCpp_(res)	    (ResType_(res)==XieK_IdcCpp)
#define IsRoi_(res)	    (ResType_(res)==XieK_IdcRoi)
#define IsTmp_(res)	    (ResType_(res)==XieK_IdcTmp)

    /*
    **	Photoflo resource queue structure.
    */
typedef struct _FloQue {
    struct _FloQue	*flink;		/* flink to next FloQue element	    */
    struct _FloQue	*blink;		/* blink to prev FloQue element	    */
    struct _CommonPart	*resource;	/* link to queued resource	    */
} FloQueRec, *FloQuePtr;
    /*
    **  Convenience macros for accessing FloQuePtr fields.
    */
#define FloNxt(que) 	((que)->flink)
#define FloPrv_(que)	((que)->blink)
#define FloRes_(que)	((que)->resource)
#define FloMap_(que)	((PhotomapPtr)FloRes_(que))

typedef struct _ThreadCtx *ThreadCtxPtr; /* Used with VMS DECwindows only */

    /*
    **	Photoflo structure.
    */
typedef struct _Photoflo {
    CommonPartRec   Res;		/* type: XieK_Photoflo		    */
    /*	  Res.flags.flag0;		/* Form: Photoflo under construction*/
    /*	  Res.flags.flag1;		/* Run:  Photoflo executing	    */
    /*	  Res.flags.flag2;		/* Done: Photoflo execution complete*/
    /*    Res.flags.flag3;              /* Yielded: Photoflo has yielded    */
    FloQuePtr	    QueSrc;		/* Src Photomap in resource queue   */
    FloQuePtr	    QueDst;		/* Dst Photomap in resource queue   */
    Pipe	    Pipe;		/* root of executable Pipeline	    */
    ThreadCtxPtr    ThreadCtx;          /* Thread context                   */
} PhotofloRec, *PhotofloPtr;
    /*
    **  Convenience macros for accessing PhotofloPtr fields.
    */
#define Make_(flo)	    ((flo)->Res.flags.flag0)
#define Run_(flo)	    ((flo)->Res.flags.flag1)
#define Done_(flo)	    ((flo)->Res.flags.flag2)
#define Yielded_(flo)       ((flo)->Res.flags.flag3)
#define Aborted_(flo)	    (!(Make_(flo)||Run_(flo)||Done_(flo)))
#define QueSrc_(flo)	    ((flo)->QueSrc)
#define    SrcMap_(flo)	     ((PhotomapPtr)(QueSrc_(flo)->resource))
#define    QueEnd_(flo)	     (QueSrc_(flo)->blink)
#define QueDst_(flo)	    ((flo)->QueDst)
#define    DstMap_(flo)	     ((PhotomapPtr)(QueDst_(flo)->resource))
#define Pipe_(flo)	    ((flo)->Pipe)
#define ThreadCtx_(flo)     ((flo)->ThreadCtx)

typedef struct _ExpCtxRec *ExpCtxPtr;	/* Import/Export context pointer    */
typedef struct _XportRec  *XportPtr;	/* Transport context pointer	    */
    /*
    **	Photomap (and Phototap) structure.
    */
typedef struct _Photomap {
    CommonPartRec   Res;			/* type: XieK_Photo{map|tap}*/
    /* Res.flags.flag0;				/* Constrained (NOT float)  */
    /* Res.flags.flag1;				/* AllPending (all  pending)*/
    /* Res.flags.flag2;				/* AnyPending (some pending)*/
    /* Res.flags.flag3;				/* WriteMap   (PutStream OK)*/
    CARD8	Cmpres;				/* compression scheme	    */
    CARD8	CmpOrg;				/* component organization   */
    CARD8	CmpMap;				/* component mapping	    */
    CARD8	CmpCnt;				/* component count	    */
    CARD8	PxlPol;				/* pixel polarity	    */
    CARD8	PxlProg;			/* pixel progression	    */
    CARD8	ScnProg;			/* scanline progression	    */
    CARD8      _pad;
    CARD16	Pending[XieK_MaxComponents];	/* comp's pending PutStream */
    CARD32	Width;				/* Photomap width  in pixels*/
    CARD32	Height;				/* Photomap height in pixels*/
    double	PxlRatio;			/* pixel aspect ratio	    */
    ExpCtxPtr	Eport;				/* import/export context    */
    XportPtr	Tport;				/* transport context	    */
    PhotofloPtr	FloLnk;				/* binding to Photoflo	    */
    PipeSinkPtr	Sink;				/* image data sink	    */
} PhotomapRec, *PhotomapPtr;
    /*
    **  Convenience macros for accessing PhotomapPtr fields.
    */
#define Constrained_(img)   ((img)->Res.flags.flag0)
#define AllPending_(img)    ((img)->Res.flags.flag1)
#define AnyPending_(img)    ((img)->Res.flags.flag2)
#define WriteMap_(img)	    ((img)->Res.flags.flag3)
#define Cmpres_(img)        ((img)->Cmpres)
#define   IsDCT_(img)	     (Cmpres_(img)==XieK_DCT)
#define   IsG31D_(img)	     (Cmpres_(img)==XieK_G31D)
#define   IsG32D_(img)	     (Cmpres_(img)==XieK_G32D)
#define   IsG42D_(img)	     (Cmpres_(img)==XieK_G42D)
#define   IsPCM_(img)	     (Cmpres_(img)==XieK_PCM)
#define CmpOrg_(img)        ((img)->CmpOrg)
#define   IsBndPxl_(img)     (CmpOrg_(img)==XieK_BandByPixel)
#define   IsBndPln_(img)     (CmpOrg_(img)==XieK_BandByPlane)
#define   IsBitPln_(img)     (CmpOrg_(img)==XieK_BitByPlane)
#define CmpMap_(img)	    ((img)->CmpMap)
#define   IsBitonal_(img)    (CmpMap_(img)==XieK_Bitonal)
#define   IsGrayScale_(img)  (CmpMap_(img)==XieK_GrayScale)
#define   IsRGB_(img)	     (CmpMap_(img)==XieK_RGB)
#define CmpCnt_(img)	    ((img)->CmpCnt)
#define PxlPol_(img)	    ((img)->PxlPol)
#define PxlProg_(img)	    ((img)->PxlProg)
#define ScnProg_(img)	    ((img)->ScnProg)
#define CmpPending_(img,c)  ((img)->Pending[(c)])
#define Width_(img)	    ((img)->Width)
#define Height_(img)	    ((img)->Height)
#define PxlRatio_(img)	    ((img)->PxlRatio)
#define Eport_(img)	    ((img)->Eport)
#define Tport_(img)	    ((img)->Tport)
#define FloLnk_(img)	    ((img)->FloLnk)
#define Sink_(img)	    ((img)->Sink)
#define  Udp_(img,comp)	     (SnkUdpPtr_(Sink_(img),comp))
#define   uPxlLen_(img,comp)  (SnkPxlLen_(Sink_(img),comp))
#define   uDType_(img,comp)   (SnkDatTyp_(Sink_(img),comp))
#define   uClass_(img,comp)   (SnkDatCls_(Sink_(img),comp))
#define   uBase_(img,comp)    (SnkBase_(Sink_(img),comp))
#define   uArSize_(img,comp)  (SnkArSize_(Sink_(img),comp))
#define   uPxlStr_(img,comp)  (SnkPxlStr_(Sink_(img),comp))
#define   uScnStr_(img,comp)  (SnkScnStr_(Sink_(img),comp))
#define   uX1_(img,comp)      (SnkX1_(Sink_(img),comp))
#define   uX2_(img,comp)      (SnkX2_(Sink_(img),comp))
#define   uY1_(img,comp)      (SnkY1_(Sink_(img),comp))
#define   uY2_(img,comp)      (SnkY2_(Sink_(img),comp))
#define   uWidth_(img,comp)   (SnkWidth_(Sink_(img),comp))
#define   uHeight_(img,comp)  (SnkHeight_(Sink_(img),comp))
#define   uPos_(img,comp)     (SnkPos_(Sink_(img),comp))
#define   uCmpIdx_(img,comp)  (SnkCmpIdx_(Sink_(img),comp))
#define   uLvl_(img,comp)     (SnkLvl_(Sink_(img),comp))
#define RunningFlo_(img)  (FloLnk_(img)&&Run_(FloLnk_(img)))

    /*
    **	Cpp structure.
    */
typedef struct {
    CommonPartRec   Res;		/* type: XieK_IdcCpp		    */
    PhotomapPtr	    photomap;		/* Cpp Photomap pointer		    */
    UdpPtr          udp;                /* Cpp coordinates and data plane   */
} CppRec, *CppPtr;
    /*
    **  Convenience macros for accessing CppPtr fields.
    */
#define CppX_(cpp)	    ((cpp)->udp->UdpL_X1)
#define CppY_(cpp)	    ((cpp)->udp->UdpL_Y1)
#define CppMap_(cpp)	    ((cpp)->photomap)
#define CppUdp_(cpp)        ((cpp)->udp)

    /*
    **	Roi structure.
    */
typedef struct {
    CommonPartRec   Res;		/* type: XieK_IdcRoi		    */
    UdpPtr          udp;                /* Roi coordinates in UDP format    */
} RoiRec, *RoiPtr;
    /*
    **  Convenience macros for accessing RoiPtr fields.
    */
#define RoiX_(roi)	    ((roi)->udp->UdpL_X1)
#define RoiY_(roi)	    ((roi)->udp->UdpL_Y1)
#define RoiW_(roi)	    ((roi)->udp->UdpL_PxlPerScn)
#define RoiH_(roi)	    ((roi)->udp->UdpL_ScnCnt)
#define RoiUdp_(roi)        ((roi)->udp)

    /*
    **	Tmp structure.
    */
typedef struct {
    CommonPartRec   Res;		/* type: XieK_IdcTmp		    */
    INT32	    center_x;		/* Template center X coordinate     */
    INT32	    center_y;		/* Template center Y coordinate     */
    TmpDatPtr       dat;                /* Template data in DDX format      */
    CARD32	    data_count;		/* number of TmpEntryRec's in data  */
/*  TmpEntryRec	    data		   list of TmpEntryRec's	    */
} TmpRec, *TmpPtr;
    /*
    **	Tmp resource data entry structure.
    */
typedef struct {
    INT32   x;				/* template entry X coordinate	    */
    INT32   y;				/* template entry Y coordinate	    */
    double  value;			/* template entry value		    */
} TmpEntryRec, *TmpEntryPtr;
    /*
    **  Convenience macros for accessing TmpPtr fields.
    */
#define TmpX_(tmp)	    ((tmp)->center_x)
#define TmpY_(tmp)	    ((tmp)->center_y)
#define TmpCnt_(tmp)	    ((tmp)->data_count)
#define TmpEnt_(tmp)	    ((TmpEntryPtr)&((tmp)[1]))
#define TmpDat_(tmp)        ((tmp)->dat)

