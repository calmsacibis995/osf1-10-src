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
Copyright 1989-1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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
**	This module contains definitions required by the XIE wire protocol
**	-- XIE library and XIE server DDX layers.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Richard J. Piccolo
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      March 1, 1989
**
************************************************************************/

    /*
    **	Symbol XIEPROTO allows XieProto.h to be included multiple times.
    */
#ifndef XIEPROTO
#define XIEPROTO	/* the "endif" MUST be the last line of this file   */

/*
**  Include files
*/
#ifdef VMS
#include <XieAppl.h>			/* XIE inter-layer definitions	    */
#include <XieFloat.h>    		/* XIE device dependant definitions */
#else
#include <X11/extensions/XieAppl.h>	/* XIE inter-layer definitions	    */
#include <X11/extensions/XieFloat.h>    /* XIE device dependant definitions */
#endif

/*
**  Wire protocol constant definitions.
*/
    /*
    **	Version number
    */
#define XieK_MajorVersion	     2
#define XieK_MinorVersion	     3

/*******************************************************************************
** Protocol request packet minor op-codes
*******************************************************************************/
#define X_ieInitSession              1
#define X_ieTermSession              2

#define X_ieQueryEvents		     3
#define X_ieSelectEvents	     4

#define X_ieSetOpDefaults            5
#define X_ieQueryOpDefaults          6

#define X_ieCreateByReference        7
#define X_ieCreateByValue	     8
#define X_ieDeleteResource	     9
#define X_ieQueryResource           10

#define X_ieBindPhotomap	    11
#define X_ieAbortPhotoflo	    12
#define X_ieExecutePhotoflo	    13
#define X_ieTapPhotoflo		    14

#define X_ieAbortTransport          15
#define X_ieGetStream               16
#define X_ieGetTile                 17
#define X_iePutStream               18
#define X_iePutTile                 19
#define X_ieSetTransport            20

#define X_ieExport		    21
#define X_ieFreeExport		    22
#define X_ieImport		    23
#define X_ieQueryExport		    24

#define X_ieArea		    25
#define X_ieAreaStats               26
#define X_ieArith                   27
#define X_ieCalcHist                28
#define X_ieChromeCom               29
#define X_ieChromeSep               30
#define X_ieCompare                 31
#define X_ieConstrain               32
#define X_ieCrop		    33
#define X_ieDither		    34
#define X_ieFill		    35
#define X_ieLogical                 36
#define X_ieLuminance		    37
#define X_ieMatchHistogram          38
#define X_ieMath                    39
#define X_ieMirror		    40
#define X_iePoint		    41
#define X_iePointStats   	    42
#define X_ieRotate		    43
#define X_ieScale		    44
#define X_ieTranslate		    45

#define X_ieLastRequest		    46

/*******************************************************************************
** Protocol packet size definitions required by Xlibint.h
*******************************************************************************/
#define sz_xieReq		     4
#define sz_xieInitSessionReq	     8
#define sz_xieTermSessionReq	     4

#define sz_xieQueryEventsReq	     8
#define sz_xieSelectEventsReq	     8

#define sz_xieSetOpDefaultsReq      20
#define sz_xieQueryOpDefaultsReq     4

#define sz_xieCreateByReferenceReq  16
#define sz_xieCreateByValueReq	    12
#define sz_xieCreateCppReq	    24
#define sz_xieCreatePhotoReq	    48
#define sz_xieCreateRoiReq	    28
#define sz_xieCreateTmpReq	    24
#define sz_xieDeleteResourceReq	    12
#define sz_xieQueryResourceReq	    12

#define sz_xieBindPhotomapReq	    12
#define sz_xieAbortPhotofloReq	     8
#define sz_xieExecutePhotofloReq     8
#define sz_xieTapPhotofloReq	    16

#define sz_xieAbortTransportReq	    12
#define sz_xieGetStreamReq	    16
#define sz_xieGetTileReq	    32
#define sz_xiePutStreamReq	    16
#define sz_xiePutTileReq	    32
#define sz_xieSetTransportReq	   140

#define sz_xieExportReq		    64
#define sz_xieFreeExportReq	     8
#define sz_xieImportReq		    20
#define sz_xieQueryExportReq	    16

#define sz_xieAreaReq               24
#define sz_xieAreaStatsReq          20
#define sz_xieArithReq              36
#define sz_xieCalcHistReq           16
#define sz_xieChromeComReq          20    
#define sz_xieChromeSepReq          16    
#define sz_xieCompareReq            36
#define sz_xieConstrainReq          28
#define sz_xieCropReq		    16
#define sz_xieDitherReq		    24
#define sz_xieFillReq		    28
#define sz_xieLogicalReq            36
#define sz_xieLuminanceReq	    12
#define sz_xieMatchHistogramReq     36
#define sz_xieMathReq               20
#define sz_xieMirrorReq		    16
#define sz_xiePointReq		    20
#define sz_xiePointStatsReq         20
#define sz_xieRotateReq		    40
#define sz_xieScaleReq		    20
#define sz_xieTranslateReq	    24
  
/*******************************************************************************
** Sub-structures
*******************************************************************************/

    /*
    **	Structure defining the Xie function vector.
    */
typedef struct {
    CARD8   function[32];
} XieFunctionsRec, *XieFunctionVector;

    /*
    **	Structure defining IdcTmp template data crossing wire.
    */
typedef struct {
    INT32	    x;
    INT32	    y;
    XieFloatRec	    value;
} XieTmpEntryRec, *XieTmpEntry;
  
/*******************************************************************************
** Protocol request packets
*******************************************************************************/

    /*
    **	Generic part of all XIE requests
    */
typedef struct {
    CARD8	    reqType;	/* major op-code assigned to extension	    */
    CARD8	    opcode;	/* minor op-code == XIE function	    */
    CARD16	    length;	/* length in longwords of entire request    */
} xieReq;

    /*
    **	Init Session Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieInitSession	    */
    CARD16	    length;
    CARD16	    major_version;
    CARD16	    minor_version;
} xieInitSessionReq;

    /*
    **	Term Session Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieTermSession	    */
    CARD16	    length;
} xieTermSessionReq;

    /* 
    ** Query Events Requests
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;
    CARD16	     length;
    CARD32	     event_param;
} xieQueryEventsReq;

    /* 
    ** Select Events Requests
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;
    CARD16	     length;
    CARD32	     event_mask;
} xieSelectEventsReq;

    /*
    **	Set Operational Defaults
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieSetOpDefaults   */
    CARD16	    length;
    CARD8           model;
    CARD8           _pad[3];
    CARD32          levels[XieK_MaxComponents];
} xieSetOpDefaultsReq;

    /*
    **	Query Operational Defaults
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieQueryOpDefaults   */
    CARD16	    length;
} xieQueryOpDefaultsReq;

    /*
    **	Create (ie. clone Photo{flo|map}) resource By Reference Request.
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieCreateByReference*/
    CARD16	    length;
    CARD32	    resource_type;
    CARD32	    resource_id;	/* new Photo{flo|map}-id	    */
    CARD32	    photomap_id;	/* existing  Photomap-id	    */
} xieCreateByReferenceReq;

    /*
    **	Generic Create resource By Value Request.
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieCreateByValue    */
    CARD16	    length;
    CARD32	    resource_type;	/* new resource type		    */
    CARD32	    resource_id;	/* new resource identifier	    */
} xieCreateByValueReq;

    /*
    ** Create Cpp Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieCreateByValue    */
    CARD16	    length;
    CARD32	    resource_type;	/* resource type == XieK_IdcCpp	    */
    CARD32	    resource_id;
    CARD32	    photomap_id;
    INT32	    x;
    INT32	    y;
} xieCreateCppReq;

    /*
    **	Create Photo{flo|map} from client specified attributes.
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieCreateByValue    */
    CARD16	    length;
    CARD32	    resource_type;	/* XieK_Photomap or XieK_Photoflo   */
    CARD32	    resource_id;
    CARD32	    width;
    CARD32	    height;	    
    XieFloatRec	    aspect_ratio;
    CARD32	    levels[XieK_MaxComponents];
    CARD8	    number_of_components;
    CARD8	    component_mapping;
    CARD8	    pixel_progression;
    CARD8	    line_progression;
    CARD8	    polarity;
    CARD8	    _pad[3];
} xieCreatePhotoReq;

    /* 
    ** Create Roi Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieCreateByValue    */
    CARD16	    length;
    CARD32	    resource_type;	/* resource type == XieK_IdcRoi	    */
    CARD32	    resource_id;
    INT32	    x;
    INT32	    y;
    CARD32	    width;
    CARD32	    height;
} xieCreateRoiReq;

    /*
    ** Create Tmp Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieCreateByValue    */
    CARD16	    length;
    CARD32	    resource_type;	/* resource type == XieK_IdcTmp	    */
    CARD32	    resource_id;
    INT32	    center_x;
    INT32	    center_y;
    CARD32	    data_count;		/* number of XieTmpEntryRec's	    */
/*  XieTmpEntryRec  data[data_count]	/* list of XieTmpEntryRec	    */
} xieCreateTmpReq;

    /*
    **	Delete resource request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieDeleteResource   */
    CARD16	    length;
    CARD32	    resource_type;
    CARD32	    resource_id;
} xieDeleteResourceReq;

    /*
    **	Query resource request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieQueryResource    */
    CARD16	    length;
    CARD32	    resource_type;
    CARD32	    resource_id;
} xieQueryResourceReq;

    /* 
    **	Bind an existing Photomap to an existing Photoflo.
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieBindPhotomap	    */
    CARD16	    length;
    CARD32	    photoflo_id;
    CARD32	    photomap_id;
} xieBindPhotomapReq;

    /* 
    **	Abort Photoflo request.
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieAbortPhotoflo    */
    CARD16	    length;
    CARD32	    photoflo_id;
} xieAbortPhotofloReq;

    /* 
    ** Execute Photoflo Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieExecutePhotoflo  */
    CARD16	    length;
    CARD32	    photoflo_id;
} xieExecutePhotofloReq;

    /*
    **	Tap Photoflo Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieTapPhotoflo	    */
    CARD16	    length;
    CARD32	    photoflo_id;
    CARD32	    photo_id;
    BOOL	    permanent;
    CARD8	    _pad[3];
} xieTapPhotofloReq;

    /*
    **	Abort transport Request (release transport resources)
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieAbortTransport    */
    CARD16	    length;
    CARD32	    photo_id;
    CARD32	    plane_mask;
} xieAbortTransportReq;

    /*
    **	Get Stream Request (transport image data: server --> client)
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieGetStream	    */
    CARD16	    length;
    CARD32	    photo_id;
    CARD32	    max_bytes;		/* maximum return image data length */
    CARD8	    plane_num;
    CARD8	    _pad[3];
} xieGetStreamReq;

    /* 
    ** Get Tile Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieGetTile	    */
    CARD16	    length;
    CARD32	    photomap_id;
    INT32	    x;
    INT32	    y;
    CARD32	    width;
    CARD32	    height;
    CARD32	    plane_mask;
    BOOL	    final;
    CARD8	    _pad[3];
} xieGetTileReq;

    /*
    **	Put Stream Request (transport image data: client --> server)
    */
typedef struct _xiePutStreamReq {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_iePutStream	    */
    CARD16	    length;
    CARD32	    photo_id;
    CARD32	    byte_count;		/* image plane data length in bytes */
    BOOL	    final;
    CARD8	    plane_num;
    CARD8	    _pad[2];
} xiePutStreamReq;

    /*
    **	Put Tile Request (transport image data: client --> server)
    */
typedef struct _xiePutTileReq {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_iePutTile	    */
    CARD16	    length;
    CARD32	    photomap_id;
    INT32	    x;
    INT32	    y;
    CARD32	    width;
    CARD32	    height;
    CARD32	    plane_mask;
    BOOL	    final;
    CARD8	    _pad[3];
} xiePutTileReq;

    /*
    **	Set transport Request (set transport parameters)
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieSetTransport	    */
    CARD16	    length;
    CARD32	    photo_id;
    CARD32	    compression_parameter;
    CARD8	    compression_scheme;
    CARD8	    mode;
    CARD8	    component_organization;
    CARD8	    _pad;
    CARD32	    plane_mask;
    CARD8	    pixel_stride[XieK_MaxPlanes];
    CARD32	    scanline_stride[XieK_MaxPlanes];
} xieSetTransportReq;

    /*
    **	Export image data to drawable Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieExport	    */
    CARD16	    length;
    CARD32	    photo_id;
    CARD32	    drawable_id;
    CARD32	    gc_id;
    INT32	    src_x;
    INT32	    src_y;
    CARD32	    width;
    CARD32	    height;
    INT32	    dst_x;
    INT32	    dst_y;
    CARD32	    photo_lut_id;
    CARD32	    colormap_id;
    XieFloatRec	    match_limit;
    XieFloatRec	    gray_limit;
} xieExportReq;

    /*
    **	Free ExportContext from Photomap Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieFreeExport	    */
    CARD16	    length;
    CARD32	    photo_id;
} xieFreeExportReq;

    /* 
    ** Import image data from drawable Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieImport	    */
    CARD16	    length;
    CARD32	    photomap_id;
    CARD32	    drawable_id;
    CARD32	    colormap_id;
    CARD8	    polarity;
    CARD8	    _pad[3];
} xieImportReq;

    /*
    **	Query ExportContext Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieQueryExport	    */
    CARD16	    length;
    CARD32	    photo_id;
    CARD32	    lut_id;
    BOOL	    get_pixels;
    CARD8	    _pad[3];
} xieQueryExportReq;

    /* 
    ** Area Request
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;
    CARD16	     length;
    CARD32	     src_photo_id;
    CARD32	     dst_photo_id;
    CARD32	     ipcc_id;
    CARD32	     ipct_id;
    CARD8	     op1;
    CARD8	     op2;
    CARD8	     constrain;
    CARD8	     _pad;
} xieAreaReq;

    /* 
    ** Area Statistics Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
    CARD32	    idc_id;
    CARD8	    edge_size;
    CARD8	    stat_type;
    CARD8	    _pad[2];
} xieAreaStatsReq;

    /* 
    ** Arith Request 
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;		/* function == X_ieArithmetic */
    CARD16	     length;
    CARD32	     src1_photo_id;
    CARD32	     src2_photo_id;
    CARD32	     constants[XieK_MaxComponents];
    CARD32	     dst_photo_id;
    CARD32	     idc_id;
    CARD8	     op;
    CARD8	     constrain;
    CARD16	     _pad;
} xieArithReq;

    /* 
    ** Calculate Histogram Request
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;
    CARD16	     length;
    CARD32	     photo_id;
    CARD32	     idc_id;
    CARD8	     by_component;
    CARD8	     _pad[3];
} xieCalcHistReq;

    /* 
    ** Chrominance Combine Request 
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;
    CARD16	     length;
    CARD32	     photo_id1;
    CARD32	     photo_id2;
    CARD32	     photo_id3;
    CARD32	     dst_photo_id;
} xieChromeComReq;

    /* 
    ** Chrominance Separate Request 
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;
    CARD16	     length;
    CARD32	     src_photo;
    CARD32	     dst_photo;
    CARD32           component;
} xieChromeSepReq;

    /* 
    ** Compare Request 
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;
    CARD16	     length;
    CARD32	     src1_photo_id;
    CARD32	     src2_photo_id;
    CARD32	     constants[XieK_MaxComponents];
    CARD32	     dst_photo_id;
    CARD32	     idc_id;
    CARD8	     op;
    CARD8	     combine;
    CARD16	     _pad;
} xieCompareReq;

    /* 
    ** Constrain Request 
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;		/* function == X_ieConstrain       */
    CARD16	     length;
    CARD32	     src_photo_id;
    CARD32	     dst_photo_id;
    CARD8            model;
    CARD8            _pad[3];
    CARD32           levels[XieK_MaxComponents];

} xieConstrainReq;

    /* 
    ** Crop Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieCrop		    */
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
    CARD32	    idc_id;
} xieCropReq;

    /* 
    ** Dither Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieDither	    */
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
    CARD32	    levels[XieK_MaxComponents];
} xieDitherReq;

    /* 
    ** Fill Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieFill		    */
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
    CARD32	    dst_idc_id;
    CARD32	    constant[XieK_MaxComponents];
} xieFillReq;

    /* 
    ** Logical Request 
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;		/* function == X_ieLogical */
    CARD16	     length;
    CARD32	     src1_photo_id;
    CARD32	     src2_photo_id;
    CARD32	     constants[XieK_MaxComponents];
    CARD32	     dst_photo_id;
    CARD32	     idc_id;
    CARD8	     op;
    CARD8	     _pad[3];
    } xieLogicalReq;

    /* 
    ** Luminance Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieLuminance	    */
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
} xieLuminanceReq;

    /* 
    ** Math Function Request 
    */
typedef struct {
    CARD8	     reqType;
    CARD8	     opcode;
    CARD16	     length;
    CARD32	     src_photo_id;
    CARD32	     dst_photo_id;
    CARD32	     idc_id;
    CARD8	     op;
    CARD8	     constrain;
    CARD16	     _pad;
} xieMathReq;

    /* 
    ** Match Histogram Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;
    CARD16	    length;
    CARD32	    photo_id1;
    CARD32	    photo_id2;
    CARD32	    idc_id;
    XieFloatRec	    param1;
    XieFloatRec	    param2;
    CARD8	    h_shape;
    CARD8	    _pad[3];
} xieMatchHistogramReq;

    /* 
    ** Mirror Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieMirror	    */
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
    BOOL	    x_mirror;
    BOOL	    y_mirror;
    CARD8	    _pad[2];
} xieMirrorReq;

    /* 
    ** Point Request
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
    CARD32	    idc_id;
    CARD32          trans_photo_id;
} xiePointReq;

    /* 
    ** Point Statistics Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;
    CARD16	    length;
    CARD32	    photo_id;
    INT32	    x;
    INT32	    y;
    CARD8	    height;
    CARD8	    width;
    CARD8   	    _pad[2];
} xiePointStatsReq;

    /* 
    ** Rotate Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieRotate	    */
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
    XieFloatRec	    angle;
    CARD32	    width;
    CARD32	    height;
    CARD32	    fill[XieK_MaxComponents];
} xieRotateReq;

    /* 
    ** Scale Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieScale	    */
    CARD16	    length;
    CARD32	    src_photo_id;
    CARD32	    dst_photo_id;
    CARD32	    width;
    CARD32	    height;
} xieScaleReq;

    /* 
    ** Translate Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;		/* function == X_ieTranslate	    */
    CARD16	    length;
    CARD32	    src1_photo_id;
    CARD32          src2_photo_id;
    CARD32	    src_idc_id;
    CARD32	    dst_photo_id;
    CARD32	    dst_idc_id;
} xieTranslateReq;

#ifdef	NOT_YET_IMPLEMENTED
/************ NONE OF THE FOLLOWING REQUESTS HAVE BEEN IMPLEMENTED ************/
    /* 
    ** Display Sequence Request 
    */
typedef struct {
    CARD8	    reqType;
    CARD8	    opcode;
    CARD16	    length;
    CARD32	    window_id;
    CARD16	    frame_count;
    CARD16	    frame_timing;
    CARD8	    forward;
    CARD8	    number_of_pixmaps;
} xieDisplaySeqReq;

/**************************** END OF UNIMPLEMENTED REQUESTS *******************/
#endif

/*******************************************************************************
** Events
*******************************************************************************/
    /*
    **	Send Xie Event
    */
typedef struct {
    CARD8	    type;
    CARD8	    detail;
    CARD16	    sequenceNumber;
    CARD32	    resource_id;
    CARD8	    _pad[24];
} xieSendEvent;

/*******************************************************************************
** Replies 
*******************************************************************************/
    /*
    **	Reply for Init Session
    */
typedef struct {
    INT8	    type;
    CARD8	    success;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD16	    major_version;
    CARD16	    minor_version;
    CARD32	    service_class;			/* not implemented  */
    CARD8	    _pad[16];
} xieInitSessionReply;

    /* 
    ** Reply for Query Events
    */
typedef struct {
    INT8	    type;
    INT8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD32	    event_mask;
    CARD8	    _pad[20];		
} xieQueryEventsReply;

    /* 
    ** Reply for Query Operational Defaults
    */
typedef struct {
    INT8	    type;
    INT8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD8           model;
    CARD8           _pad[3];
    CARD32          levels[XieK_MaxComponents];
    CARD8	    _pad1[8];		
} xieQueryOpDefaultsReply;

    /*
    ** Reply for Query Resource: resource type == Photoflo
    */
typedef struct {
    INT8	    type;
    INT8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD32	    status;
    CARD8	    _pad[20];
} xieQueryPhotofloReply;

    /*
    ** Reply for Query Resource: resource type == Photo{map|tap}
    */
typedef struct {
    INT8	    type;
    INT8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD32	    width;
    CARD32	    height;
    XieFloatRec	    aspect_ratio;
    CARD8	    polarity;
    CARD8	    pixel_progression;
    CARD8	    line_progression;
    BOOL	    constrained;
    CARD8	    component_mapping;
    CARD8	    component_count;
    CARD8	    _pad[2];
    CARD32	    levels[XieK_MaxComponents];
    CARD32	    photo_id;
} xieQueryPhotomapReply;

    /*
    ** Reply for Query Resource: resource type == Idc of type XieK_Cpp
    */
typedef struct {
    INT8	    type;
    INT8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    INT32	    x;
    INT32	    y;
    CARD32	    photomap_id;
    CARD8	    _pad[12];
} xieQueryCppReply;

    /*
    ** Reply for Query Resource: resource type == Idc of type XieK_Roi
    */
typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    INT32	    x;
    INT32	    y;
    CARD32	    width;
    CARD32	    height;
    CARD8	    _pad[8];
} xieQueryRoiReply;

    /*
    ** Reply for Query Resource: resource type == Idc of type XieK_Tmp
    */
typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;		/* length of entries: long words    */
    INT32	    center_x;
    INT32	    center_y;
    CARD32	    data_count;		/* number of XieTmpEntryRec's to    */
    CARD8	    _pad[12];		/* follow xieQueryTmpReply as data  */
} xieQueryTmpReply;

    /*
    ** Reply for Get Stream
    */
typedef struct {
    INT8	    type;
    CARD8	    status;
    CARD16	    sequenceNumber;
    CARD32	    length;		/* length of image data: longwords  */
    CARD32	    byte_count;		/* image plane data length in bytes */
    CARD8	    _pad[20];
} xieGetStreamReply;

    /*
    ** Reply for Get Tile 
    */
typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;		/* length of image data: longwords  */
    CARD8	    _pad[24];
} xieGetTileReply;

    /*
    ** Reply for QueryExport -- returns LUT and list of allocated pixels
    */
typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;		/* number of pixels being returned  */
    CARD32	    pixel_count;
    CARD32	    lut_id;
    CARD8	    lut_mapping;
    CARD8	    _pad[15];
} xieQueryExportReply;

    /*
    **	Reply for CalcHistogram -- returns frequency distribution data
    */
typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;		/* size of histogram list in longs */
    CARD8	    _pad[24];
} xieCalcHistReply;

    /*
    **	Reply for Point Statistics
    */
typedef struct {
    INT8	    type;
    INT8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD8	    _pad[24];
} xiePointStatsReply;

/* the following structure is used for Point Statistics */

typedef struct {
    XieFloatRec	    pixel;
    XieFloatRec	    minimum;
    XieFloatRec	    maximum;
    XieFloatRec	    mean;
    XieFloatRec	    std_dev;
    XieFloatRec	    variance;
} xiePointStatsData, *xiePointStatsDataPtr;


#ifdef	NOT_YET_IMPLEMENTED
/************ NONE OF THE FOLLOWING REPLIES HAVE BEEN IMPLEMENTED *************/

typedef struct {
    CARD16	    screen_width_millimeters;
    CARD16	    screen_height_millimeters;
    CARD16	    screen_width_pixels;
    CARD16	    screen_height_pixels;
    CARD16	    screen_depth;
    CARD16	    screen_luts_size;
    CARD8	    screen_number;
    CARD8	    screen_luts;
    CARD8	    screen_brightness_control;
    CARD8	    screen_contrast_control;
    CARD8	    screen_hue_control;
    CARD8	    _pad[5];
} screen_data;

typedef struct {
    CARD32	     minimum;
    CARD32	     maximum;
    CARD32	     mean;
    CARD32	     std_dev;
    CARD32	     variance;
} statistics_data;

/* Reply for Dot Product */

typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD32	    _pad[6];
} xieDotProductReply;

/*** following display stuff may be deleted or expanded */

typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD8	    idc_type;
    CARD8	    skip_or_show;
    CARD32	    pixmap;
    CARD16	    _pad;
} xieSingleStaticReply;

typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD8	    idc_type;
    CARD8	    map_x_tiles;
    CARD8	    map_y_tiles;
    CARD32	    current_frame;
    CARD32	    number_of_display_frames;
} xieSpatialMapReply;

typedef struct {
    CARD32	    pixmap;
    CARD8	    skip_or_show;
    CARD8	    _pad[3];
} display_frame;

typedef struct {
    INT8	    type;
    CARD8	    _reserved;
    CARD16	    sequenceNumber;
    CARD32	    length;
    CARD32	    current_frame;
    CARD32	    number_of_display_frames;
    CARD8	    idc_type;
    CARD8	    map_x_tiles;
    CARD8	    map_y_tiles;
    CARD8	    _pad[14];
} xieTimeSequenceReply;

/**************************** END OF UNIMPLEMENTED REPLIES ********************/
#endif
/*
**  This "endif" MUST be the last line of this file.
*/
#endif	/* end of XIEPROTO -- NO DEFINITIONS ALLOWED BEYOND THIS POINT */

