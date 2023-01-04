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
**  Copyright 1989-1991 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**
**                        All Rights Reserved
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

/*******************************************************************************
**
**  FACILITY:
**
**      XIE - X11 Image Extension
**  
**
**  ABSTRACT:
**
**      This module contains definitions required by all XIE client side 
**      layers -- from the application level through the XieLib level.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.3
**      ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      April 13, 1989
**
*******************************************************************************/

    /*
    **  Symbol XIELIB allows XieLib.h to be included multiple times.
    */
#ifndef _XIELIB
#define _XIELIB         /* the "endif" MUST be the last line of this file   */

/*
**  Include files
*/
#ifdef VMS
#include <decw$include/Xlib.h>          /* X11 Library Definitions          */
#else
#include <X11/Xlib.h>                   /* X11 Library Definitions          */
#endif
#ifdef VMS
#include <XieUdpDef.h>   		/* universal data plane definitions */
#include <XieAppl.h>     		/* XIE inter-layer definitions      */
#else
#include <X11/extensions/XieUdpDef.h>   /* universal data plane definitions */
#include <X11/extensions/XieAppl.h>     /* XIE inter-layer definitions      */
#endif

/*
**  Equated symbols
*/
    /*
    **  XieCopyImage, XieGetImage, and XiePutImage image data mode constants.
    */
#define XieK_DataNone   1   /* no buffers, no transport setup, no data copy */
#define XieK_DataFlo    2   /* prepare a Photoflo for image data transport  */
#define XieK_DataMap    3   /* prepare a Photomap for image data transport  */
#define XieK_DataAlloc  4   /* allocate image buffers containing no data    */
#define XieK_DataCopy   5   /* allocate image buffers and copy data         */
#define XieK_DataShare  6   /* copy pointers to existing image data         */
    /*
    **  Opaque Xie resource definitions.
    */
typedef unsigned long *XieStream;   /* Stream transport context pointer     */
typedef unsigned char
           *XiePhotoflo,    /* Photoflo: server image pipeline object       */
           *XiePhotomap,    /* Photomap: permanent server image object      */
           *XiePhototap,    /* Phototap: ephemeral server image object      */
           *XieCpp,         /* Cpp: control processing plane                */
           *XieRoi,         /* Roi: region of interest                      */
           *XieTmp,         /* Tmp: template (eg. convolution kernel)       */
           *XiePhoto,       /* Any of: XiePhotoflo, XiePhotomap, XiePhototap*/
           *XieIdc,         /* Any of: XieCpp, XieRoi, XieTmp,              */
           *XieResource;    /* Any of: XiePhoto, XieIdc                     */

#ifdef __alpha
#define RES_XID     16      /* offset to XieResource X11 resource-id        */
#define RES_TYPE    24      /* offset to XieResource resource type          */
#define RES_MAPPING 25      /* offset to XiePhoto component mapping         */
#else
#define RES_XID      8      /* offset to XieResource X11 resource-id        */
#define RES_TYPE    12      /* offset to XieResource resource type          */
#define RES_MAPPING 13      /* offset to XiePhoto component mapping         */
#endif
/*
**  Macros
*/
    /*
    **  Macro for returning a resource's X11-id.
    */
#define XId_(res)         (*((XID *)(&((XieResource)(res))[RES_XID])))
    /*
    **  Boolean macros for determining a resource's type.
    */
#define IsPhotoflo_(res)  (((XieResource)(res))[RES_TYPE]==XieK_Photoflo)
#define IsPhotomap_(res)  (((XieResource)(res))[RES_TYPE]==XieK_Photomap)
#define IsPhototap_(res)  (((XieResource)(res))[RES_TYPE]==XieK_Phototap)
#define IsCpp_(res)       (((XieResource)(res))[RES_TYPE]==XieK_IdcCpp)
#define IsRoi_(res)       (((XieResource)(res))[RES_TYPE]==XieK_IdcRoi)
#define IsTmp_(res)       (((XieResource)(res))[RES_TYPE]==XieK_IdcTmp)
#define IsPhoto_(res)     (IsPhotoflo_(res)||IsPhotomap_(res)||IsPhototap_(res))
#define IsIdc_(res)       (IsCpp_(res)||IsRoi_(res)||IsTmp_(res))
    /*
    **  Boolean macros for determining a XiePhotomap's component mapping.
    */
#define IsBitonal_(pho)   (((XiePhoto)(pho))[RES_MAPPING]==XieK_Bitonal)
#define IsGrayScale_(pho) (((XiePhoto)(pho))[RES_MAPPING]==XieK_GrayScale)
#define IsRGB_(pho)       (((XiePhoto)(pho))[RES_MAPPING]==XieK_RGB)

    /*
    **  Return the component count of an XiePhotomap.
    */
#define PhotoCount_(pho) (IsRGB_(pho) ? 3 : 1)
    /*
    **  Return the number of planes (UdpPtr's) an XieImage "should" contain.
    */
#define PlaneCount_(img) \
            ( CmpOrg_(img) == XieK_BandByPixel ? 1 \
            : CmpOrg_(img) == XieK_BandByPlane ? CmpCnt_(img) \
            : CmpLen_(img,0)+CmpLen_(img,1)+CmpLen_(img,2) )
    /*
    **  Return "val" rounded up (if necessary) to a multiple of "mod".
    */
#define Modulo_(val,mod) \
              ((mod) < 2 ? (val) : (((mod) - (val) % (mod)) % (mod) + (val)))

/*
**  Structures
*/
    /*
    **  XIE Client Event Structure
    */
typedef struct {
    int             type;               /* see XQueryExtention "first_event"*/
    unsigned long   serial;             /* X11 serial number                */
    Bool            send_event;         /* Boolean: true if client generated*/
    Display        *display;            /* X11 display pointer              */
    XieResource     resource;           /* Xie resource-id (source of event)*/
} XieEventRec, *XieEvent;
    /*
    **  Image domain context template data for XieCreateTmp / XieQueryTmp.
    */
typedef struct {
    long    x;                          /* X coordinate of template entry   */
    long    y;                          /* Y coordinate of template entry   */
    double  value;                      /* value of template entry          */
} XieTemplateRec, *XieTemplate;
    /*
    **  Statistics data
    */
typedef struct {
    double  value;
    double  minimum;
    double  maximum;
    double  mean;
    double  stddev;
    double  variance;
} XieStatsRec, *XieStats;

    /*
    **  XIE Image structure (client side image descriptor).
    */
typedef struct {
    unsigned char   Compression;                /* compression scheme       */
    unsigned char   CmpOrg;                     /* component organization   */
    unsigned char   CmpMap;                     /* component mapping        */
    unsigned char   CmpCnt;                     /* component count          */
    unsigned long   CmpLvl[XieK_MaxComponents]; /* levels per component     */
    unsigned char   CmpLen[XieK_MaxComponents]; /* bits   per component     */
    unsigned char   PxlPol;                     /* pixel brightness polarity*/
    unsigned char   PxlProg;                    /* pixel progression        */
    unsigned char   ScnProg;                    /* scanline progression     */
    unsigned char   OwnData;                    /* image array ownership    */
    unsigned char   Stream;                     /* Stream transport status  */
    XieStream       Xport;                      /* Stream transport context */
    double          PxlRatio;                   /* pixel aspect ratio       */
    UdpPtr          Plane[XieK_MaxPlanes];      /* data plane pointers      */
} XieImageRec, *XieImage;
    /*
    **  Access macros for XIE Image structure.
    */
#define Cmpres_(img)        ((img)->Compression)
#define CmpOrg_(img)        ((img)->CmpOrg)
#define CmpMap_(img)        ((img)->CmpMap)
#define CmpCnt_(img)        ((img)->CmpCnt)
#define CmpLvl_(img,ind)    ((img)->CmpLvl[(ind)])
#define CmpLen_(img,ind)    ((img)->CmpLen[(ind)])
#define PxlPol_(img)        ((img)->PxlPol)
#define PxlProg_(img)       ((img)->PxlProg)
#define ScnProg_(img)       ((img)->ScnProg)
#define OwnData_(img)       ((img)->OwnData)
#define Stream_(img)        ((img)->Stream)
#define   StreamFinal_(img)   (Stream_(img) == XieK_StreamFinal)
#define   StreamError_(img)   (Stream_(img) == XieK_StreamError)
#define   StreamPending_(img) (!(StreamFinal_(img) || StreamError_(img)))
#define PxlRatio_(img)      ((img)->PxlRatio)
#define Width_(img)         ((img)->Plane[0]->UdpL_PxlPerScn)
#define Height_(img)        ((img)->Plane[0]->UdpL_ScnCnt)
#define Plane_(img,ind)     ((img)->Plane[(ind)])
#define   uPxlLen_(img,ind)   ((img)->Plane[(ind)]->UdpW_PixelLength)
#define   uDType_(img,ind)    ((img)->Plane[(ind)]->UdpB_DType)
#define   uClass_(img,ind)    ((img)->Plane[(ind)]->UdpB_Class)
#define   uBase_(img,ind)     ((img)->Plane[(ind)]->UdpA_Base)
#define   uArSize_(img,ind)   ((img)->Plane[(ind)]->UdpL_ArSize)
#define   uPxlStr_(img,ind)   ((img)->Plane[(ind)]->UdpL_PxlStride)
#define   uScnStr_(img,ind)   ((img)->Plane[(ind)]->UdpL_ScnStride)
#define   uX1_(img,ind)       ((img)->Plane[(ind)]->UdpL_X1)
#define   uX2_(img,ind)       ((img)->Plane[(ind)]->UdpL_X2)
#define   uY1_(img,ind)       ((img)->Plane[(ind)]->UdpL_Y1)
#define   uY2_(img,ind)       ((img)->Plane[(ind)]->UdpL_Y2)
#define   uWidth_(img,ind)    ((img)->Plane[(ind)]->UdpL_PxlPerScn)
#define   uHeight_(img,ind)   ((img)->Plane[(ind)]->UdpL_ScnCnt)
#define   uPos_(img,ind)      ((img)->Plane[(ind)]->UdpL_Pos)
#define   uCmpIdx_(img,ind)   ((img)->Plane[(ind)]->UdpL_CompIdx)
#define   uLvl_(img,ind)      ((img)->Plane[(ind)]->UdpL_Levels)

/*
**  XieLib entry points.
*/
#ifndef _XieLibEvents
extern unsigned long    XieQueryEvents();
extern void             XieSelectEvents();
#endif
#ifndef _XieLibImage
extern XieImage         XieCopyImage();
extern XieImage         XieCreateImage();
extern XieImage         XieFreeImage();
extern XieImage         XieGetImage();
extern unsigned char    XieGetImageData();
extern XiePhoto         XiePutImage();
extern unsigned char    XiePutImageData();
#endif
#ifndef _XieLibProcess
extern XiePhoto         XieArea();
extern XiePhoto         XieAreaStats();
extern XiePhoto         XieArithmetic();
extern XiePhoto         XieChromeCom();
extern XiePhoto         XieChromeSep();
extern XiePhoto         XieCompare();
extern XiePhoto         XieConstrain();
extern XiePhoto         XieCrop();
extern XiePhoto         XieDither();
extern XiePhoto         XieFill();
extern unsigned long   *XieHistogram();
extern XiePhoto         XieLogical();
extern XiePhoto         XieLuminance();
extern XiePhoto         XieMatchHistogram();
extern XiePhoto         XieMath();
extern XiePhoto         XieMirror();
extern XiePhoto         XiePoint();
extern void             XiePointStats();
extern XiePhoto         XieRotate();
extern XiePhoto         XieScale();
extern XiePhoto         XieTranslate();
#endif
#ifndef _XieLibResource
extern void             XieAbortFlo();
extern void             XieBindMapToFlo();
extern XiePhoto         XieClonePhoto();
extern XieCpp           XieCreateCpp();
extern XiePhoto         XieCreatePhoto();
extern XieRoi           XieCreateRoi();
extern XieTmp           XieCreateTmp();
extern void             XieExecuteFlo();
extern void             XieExport();
extern XieResource      XieFindResource();
extern void             XieFreeExport();
extern XieResource      XieFreeResource();
extern XiePhotomap      XieImport();
extern void             XieQueryCpp();
extern void             XieQueryExport();
extern unsigned long    XieQueryFlo();
extern XiePhoto         XieQueryMap();
extern void             XieQueryRoi();
extern XieTemplate      XieQueryTmp();
extern XiePhoto         XieTapFlo();
#endif
#ifndef _XieLibSession
extern char            *XieCheckFunction();
extern char           **XieListFunctions();
extern void             XieQueryOpDefaults();
extern void             XieSetOpDefaults();
#endif
#ifndef _XieLibTransport
extern void             XieAbortStream();
extern unsigned char    XieGetStream();
extern void             XieGetTile();
extern void             XiePutStream();
extern void             XiePutTile();
extern void             XieSetStream();
#endif
#ifndef _XieLibUtils
extern void            *XieCalloc();
extern unsigned char   *XieCallocBits();
extern void            *XieCfree();
extern void            *XieFree();
extern unsigned char   *XieFreeBits();
extern unsigned char    XieIsBitonal();
extern unsigned char    XieIsCpp();
extern unsigned char    XieIsGrayScale();
extern unsigned char    XieIsIdc();
extern unsigned char    XieIsPhoto();
extern unsigned char    XieIsPhotoflo();
extern unsigned char    XieIsPhotomap();
extern unsigned char    XieIsPhototap();
extern unsigned char    XieIsRGB();
extern unsigned char    XieIsRoi();
extern unsigned char    XieIsTmp();
extern void            *XieMalloc();
extern unsigned char   *XieMallocBits();
extern unsigned long    XiePhotoCount();
extern unsigned long    XiePlaneCount();
extern void            *XieRealloc();
extern unsigned char   *XieReallocBits();
extern unsigned long    XieXId();
#endif

/*
**  This "endif" MUST be the last line of this file.
*/
#endif  /* end of _XIELIB -- NO DEFINITIONS ALLOWED BEYOND THIS POINT */
