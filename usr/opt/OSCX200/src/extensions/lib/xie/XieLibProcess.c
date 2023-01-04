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

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension (XIE)
**
**  ABSTRACT:
**
**      This module contains XIE library routines for processing image objects.
**	
**	
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      August 7, 1989
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Definitions required by X11 include files
*/
#define NEED_EVENTS
#define NEED_REPLIES

/*
**  Definition to let include files know who's calling.
*/
#define _XieLibProcess

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include <stdio.h>
    /*
    **  X11 and XIE include files
    */
#ifdef VMS
#include <Xlibint.h>			/* X11 internal lib/transport defs  */
#else
#include <X11/Xlibint.h>		/* X11 internal lib/transport defs  */
#endif
#include <XieProto.h>			/* XIE Req/Reply protocol defs	    */
#include <XieLib.h>			/* XIE public  definitions	    */
#include <XieLibint.h>			/* XIE private definitions	    */

/*
**  Table of contents
*/
    /*
    **  PUBLIC entry points
    */
XiePhoto        XieArea();	/* create Photomap by applying area op to    */
				/* src. Optional IDC.                        */
XiePhoto        XieAreaStats();	/* create Photomap by applying area stats   */
				/* op to src. Optional IDC.		    */
XiePhoto        XieArithmetic();/* create Photomap from arithmetic combination
				 * of Photomap, and Photomap or constant.
				 * Optional IDC.                             */
XiePhoto        XieChromeCom();  /* create an RGB Photomap by combining three
				  * monochrome Photomaps.                    */
XiePhoto        XieChromeSep();  /* create an monochrome Photomap by selecting
				  * one component of an RGB Photomap         */
XiePhoto        XieCompare();    /* create a photomap which contains the
				  * boolean results of per-pixel compares    */
XiePhoto        XieConstrain(); /* constrain an existing photomap            */
XiePhoto	XieCrop();	/* create Photomap from Photomap.
				 * Optional ROI                              */
XiePhoto	XieDither();	/* create Photomap from dithered src Photomap*/
XiePhoto	XieFill();	/* fill ROI of Photomap with a constant      */
XiePhoto        XieLogical();   /* create Photomap from bitwise-logical
				 * combination of Photomap, and Photomap or 
				 * constant. Optional IDC.                   */
XiePhoto	XieLuminance();	/* create Photomap from luminance of src     */
XiePhoto	XieMatchHistogram();/* create Photomap from src Photomap
				 * remapped to new distribution.
				 * Optional IDC.                             */
XiePhoto	XieMath();	/* create Photomap from math function of
				 * src Photomap.  Optional IDC. */
XiePhoto	XieMirror();	/* create Photomap from mirrored src Photomap*/
XiePhoto	XiePoint();	/* create Photomap from point remapped src   */
void		XiePointStats();/* Return statistics around a point	     */
XiePhoto	XieRotate();	/* create Photomap from rotated src Photomap */
XiePhoto	XieScale();	/* create Photomap from scaled src Photomap  */
XiePhoto	XieTranslate();	/* translate ROI between Photomap's          */
    /*
    **  internal routines
    */

/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**	Local Storage
*/

/*******************************************************************************
**  XieArea
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a Photomap by applying an area operator to a server
**      Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id	  - id of source Photo{flo|map|tap} to be processed
**      tmp_id    - id of template Idc containing the area kernel.
**	idc_id	  - image domain context, XieCpp or XieRoi {optional}
**      op1       - request code for first operation to be performed
**      op2       - request code for second operation to be performed
**      constrain - output should be constrained (Boolean)
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
******************************************************************************/
XiePhoto XieArea( src_id, tmp_id, idc_id, op1, op2, constrain)
 XiePhoto   	src_id;
 XieTmp   	tmp_id;
 XieIdc	        idc_id;
 unsigned long  op1;
 unsigned long  op2;
 unsigned long  constrain;
{
    PhotoPtr dst;
    PhotoPtr src = (PhotoPtr) src_id;
    IdcPtr   tmp = (IdcPtr) tmp_id;
    IdcPtr   idc = (IdcPtr) idc_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i, cmp_cnt =  PhotoCount_(src);
    xieAreaReq	 *req;

    dst = _XieGetDst(src);

    /*
    **	Create a request to do area between the operands.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieArea,req);
    req->src_photo_id	= ResId_(src);
    req->dst_photo_id	= ResId_(dst);
    req->ipct_id	= ResId_(tmp);
    req->ipcc_id	= idc == NULL ? 0 : ResId_(idc);
    req->op1		= op1;
    req->op2		= op2;
    req->constrain	= (constrain != 0);
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieArea */

/*******************************************************************************
**  XieAreaStats
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a Photomap by applying an area statistics operator to a server
**      Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id	  - id of source Photo{flo|map|tap} to be processed
**	idc_id	  - image domain context, XieCpp or XieRoi {optional}
**	op	  - statistics operation to perform, one of:
**
**		    XieK_Minimum
**		    XieK_Maximum
**		    XieK_Mean
**		    XieK_StdDev
**		    XieK_Variance
**
**	edge	  - size of the local area square region surrounding each
**		    pixel over which the statistics operation is performed.
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
******************************************************************************/
XiePhoto XieAreaStats( src_id, idc_id, op, edge )
 XiePhoto   	src_id;
 XieIdc	        idc_id;
 unsigned long	op;
 unsigned long	edge;
{
    xieAreaStatsReq	    *req;

    PhotoPtr		     src = (PhotoPtr) src_id;
    PhotoPtr		     dst;
    IdcPtr		     idc = (IdcPtr) idc_id;

    Display		    *dpy =  ResDpy_(src);
    XieSessionPtr	     ses = _XieGetSession(dpy);

    dst = _XieGetDst(src);
    /*
    **	Create a request to do area stats.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieAreaStats,req);
    req->src_photo_id	= ResId_(src);
    req->dst_photo_id	= ResId_(dst);
    req->idc_id		= idc == NULL ? 0 : ResId_(idc);
    req->edge_size	= edge;
    req->stat_type	= op;
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}

/*******************************************************************************
**  XieArithmetic
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create the arithmetic combination of two server Photomap resources
**      or one server Photomap resource and a "constant" image.
**
**  FORMAL PARAMETERS:
**
**	src_id1	  - id of first  source Photo{flo|map|tap} to be processed
**	src_id2	  - id of second source Photo{flo|map|tap} to be processed
**      constant  - "constant" image values if src_id2 == 0.
**	idc_id	  - image domain context, XieCpp or XieRoi {optional}
**      operator  - request code for the operation to be performed
**      constrain - output should be constrained (Boolean)
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieArithmetic( src_id1, src_id2, constant, idc_id, operator,
		       constrain)
 XiePhoto   	src_id1;
 XiePhoto   	src_id2;
 unsigned long  *constant;
 XieIdc	        idc_id;
 unsigned long  operator;
 unsigned long  constrain;
{
    PhotoPtr     dst;
    PhotoPtr     src1 = (PhotoPtr) src_id1;
    PhotoPtr     src2 = (PhotoPtr) src_id2;
    IdcPtr        idc = (IdcPtr) idc_id;
    Display      *dpy =  ResDpy_(src1);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i, cmp_cnt =  PhotoCount_(src1);
    xieArithReq	 *req;

    if( constant == NULL  &&  !XieIsPhoto( src2 ) )
	return( NULL );	    /* One or the other required */

    dst = src2 == NULL || FloLnk_(src2) == NULL ? _XieGetDst(src1)
						:  FloLnk_(src2);
    /*
    **	Create a request to do arithmetic between the operands.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieArith,req);
    req->src1_photo_id	= ResId_(src1);
    req->src2_photo_id	= src2 == NULL ? 0 : ResId_(src2);
    for( i = 0; i < XieK_MaxComponents; i++ )
	req->constants[i] = i < cmp_cnt  &&  constant != NULL ? constant[i] : 0;
    req->dst_photo_id	= ResId_(dst);
    req->idc_id		= idc == NULL ? 0 : ResId_(idc);
    req->op		= operator;
    req->constrain	= (constrain != 0);
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieArithmetic */

/*******************************************************************************
**  XieChromeCom
**
**  FUNCTIONAL DESCRIPTION:
**
**	Combine three monochrome server Photomaps into one
**      RGB server Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id1	  - id of first  source Photo{flo|map|tap} to be processed
**	src_id2	  - id of second source Photo{flo|map|tap} to be processed
**	src_id3	  - id of third  source Photo{flo|map|tap} to be processed
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo  passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieChromeCom( src_id1, src_id2, src_id3)

 XiePhoto   	src_id1;
 XiePhoto   	src_id2;
 XiePhoto   	src_id3;
{
    PhotoPtr dst;
    PhotoPtr src1 = (PhotoPtr) src_id1;
    PhotoPtr src2 = (PhotoPtr) src_id2;
    PhotoPtr src3 = (PhotoPtr) src_id3;
    Display      *dpy =  ResDpy_(src1);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieChromeComReq	 *req;

    dst = FloLnk_(src1) != NULL ? FloLnk_(src1)
	: FloLnk_(src2) != NULL ? FloLnk_(src2)
	: FloLnk_(src3) != NULL ? FloLnk_(src3) : _XieGetDst(src1);
    ResMap_(dst) = XieK_RGB;
    /*
    **	Create a request to do the combine.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieChromeCom,req);
    req->photo_id1	= ResId_(src1);
    req->photo_id2	= ResId_(src2);
    req->photo_id3	= ResId_(src3);
    req->dst_photo_id	= ResId_(dst);

    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieChromeCom */

/*******************************************************************************
**  XieChromeSep
**
**  FUNCTIONAL DESCRIPTION:
**
**	Separate one component of an RGB Photomap into an monochrome
**      server Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id	  - id of polychromatic Photo{flo|map|tap} to be processed
**      component - index of component to separate
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo  passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieChromeSep( src_id, component )

 XiePhoto   	src_id;
 CARD32         component;
{
    PhotoPtr dst;
    PhotoPtr src = (PhotoPtr) src_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i;
    xieChromeSepReq	 *req;
    xieQueryPhotomapReply rep;

    dst = _XieGetDst(src);

    /*
    **	Create a request to do the separate.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieChromeSep,req);
    req->src_photo	= ResId_(src);
    req->dst_photo	= ResId_(dst);
    req->component      = component;
    XieReqDone_(dpy);

    if( _XieQueryResource( dst, &rep ) )
	{
	ResMap_(dst) = rep.component_mapping;
	XieReqDone_(dpy);
	}
    return( (XiePhoto) dst );
}				    /* end XieChromeSep */

/*******************************************************************************
**  XieCompare
**
**  FUNCTIONAL DESCRIPTION:
**
**	Perform a pixel-by-pixel comparison between two server Photomaps
**      or one server Photomap resource and a "constant" image.
**      The results are returned as a Photomap where the pixel values
**      are one where the comparisons is true and zero elsewhere.
**
**  FORMAL PARAMETERS:
**
**	src_id1	  - id of first source Photo{flo|map|tap} to be processed
**	src_id2	  - id of second source Photo{flo|map|tap} to be processed
**      constant  - "constant" image values if src_id2 == 0.
**	idc_id	  - image domain context, XieCpp or XieRoi {optional}
**      operator  - request code for the operation to be performed
**      combine   - Combine the per-component results for multi-component
**                  input.
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieCompare( src_id1, src_id2, constant, idc_id, operator, combine )

 XiePhoto   	src_id1;
 XiePhoto   	src_id2;
 unsigned long  *constant;
 XieIdc	        idc_id;
 unsigned long  operator;
 unsigned long  combine;
{
    PhotoPtr dst;
    PhotoPtr src1 = (PhotoPtr) src_id1;
    PhotoPtr src2 = (PhotoPtr) src_id2;
    IdcPtr      idc   = (IdcPtr) idc_id;
    Display      *dpy =  ResDpy_(src1);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i, cmp_cnt =  PhotoCount_(src1);
    xieCompareReq	 *req;

    if( constant == NULL  &&  !XieIsPhoto( src2 ) )
	return( NULL );	    /* One or the other required */

    dst = src2 == NULL || FloLnk_(src2) == NULL ? _XieGetDst(src1)
						:  FloLnk_(src2);
    /*
    **	Create a request to do compare between the operands.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieCompare,req);
    req->src1_photo_id	= ResId_(src1);
    req->src2_photo_id	= src2 == NULL ? 0 : ResId_(src2);
    for( i = 0; i < XieK_MaxComponents; i++ )
	req->constants[i] = i < cmp_cnt  &&  constant != NULL   ? constant[i]
		     : 0;
    req->dst_photo_id	= ResId_(dst);
    req->idc_id		= idc == NULL ? 0 : ResId_(idc);
    req->op		= operator;
    req->combine        = combine;

    if (!IsRGB_(dst) || combine)
	ResMap_(dst) = XieK_Bitonal;	/* Compnent mapping changes except
					 * for RGB/no combine */

    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieCompare */

/*******************************************************************************
**  XieConstrain
**
**  FUNCTIONAL DESCRIPTION:
**
**	Constrain a photomap using the current default constraint model.
**
**  FORMAL PARAMETERS:
**
**	src_id	- id of source Photo{flo|map|tap}
**      model   - constraint model to use, or zero for the default
**                  {XieK_HardClip, XieK_ClipScale, XieK_HardScale}
**      levels  - if model != 0, array of levels to constrain each component.
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieConstrain( src_id, model, levels)
 XiePhoto	         src_id;
 int                     model;
 unsigned long int       levels[XieK_MaxComponents];
{
    PhotoPtr      src = (PhotoPtr) src_id;
    PhotoPtr	  dst = _XieGetDst(src);
    Display      *dpy =  ResDpy_(dst);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i, cmp_cnt =  PhotoCount_(dst);
    xieQueryPhotomapReply rep;
    xieConstrainReq *req;

    /*
    **	Create a request to constrain a photomap.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieConstrain,req);
    req->src_photo_id  = ResId_(src);
    req->dst_photo_id  = ResId_(dst);
    req->model         = model;
    for (i = 0; i < XieK_MaxComponents; i++)
	req->levels[i] = levels[i];
    XieReqDone_(dpy);

    /* Change component mapping */
    if( model > 0 && cmp_cnt == 1 )
	ResMap_(dst) = (levels[0] > 2) ? XieK_GrayScale : XieK_Bitonal;
    else if( _XieQueryResource( dst, &rep ) )
	{
	ResMap_(dst) = rep.component_mapping;
	XieReqDone_(dpy);
	}

    return( (XiePhoto) dst );
}				    /* end XieConstrain */

/*******************************************************************************
**  XieCrop
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a Photomap from an existing Photomap (ROI optional).
**
**  FORMAL PARAMETERS:
**
**	src_id	- id of source Photo{flo|map|tap}
**	idc_id	- image domain context, XieCpp or XieRoi {optional}
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto    XieCrop( src_id, idc_id )
 XiePhoto   src_id;
 XieIdc	    idc_id;
{
    PhotoPtr    src   = (PhotoPtr) src_id;
    PhotoPtr	dst   = _XieGetDst(src);
    IdcPtr      idc   = (IdcPtr) idc_id;
    Display    *dpy   =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieCropReq *req;

    /*
    **	Create a request to copy the area from the src into the dst.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieCrop,req);
    req->src_photo_id	= ResId_(src);
    req->dst_photo_id   = ResId_(dst);
    req->idc_id		= idc == NULL ? 0 : ResId_(idc);
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieCrop */

/*******************************************************************************
**  XieDither
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a dithered copy of a Photomap.
**
**  FORMAL PARAMETERS:
**
**	src_id	- id of source Photo{flo|map|tap} to be dithered
**	levels	- list of number of levels to dither each component down to
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieDither( src_id, levels )
 XiePhoto	 src_id;
 unsigned long	*levels;
{
    PhotoPtr	src   = (PhotoPtr) src_id;
    PhotoPtr	dst   = _XieGetDst(src);
    Display    *dpy   =  ResDpy_(src);
    CARD32 i, cmp_cnt =  PhotoCount_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieDitherReq *req;

    /*
    **	Create a request to dither the src to the dst.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieDither,req);
    req->src_photo_id    = ResId_(src);
    req->dst_photo_id    = ResId_(dst);
    for( i = 0; i < XieK_MaxComponents; i++ )
	req->levels[i] = i >= cmp_cnt ? 0 : levels == NULL || levels[i] == 0
				      ? 2 : levels[i];

    if( cmp_cnt == 1  &&  req->levels[0] <= 2 )
    	ResMap_(dst) = XieK_Bitonal;		/* change component mapping */

    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieDither */

/*******************************************************************************
**  XieFill
**
**  FUNCTIONAL DESCRIPTION:
**
**	Fill a region of an existing Photomap with a constant, creating
**      a new photomap.
**
**  FORMAL PARAMETERS:
**
**	src_id	    - id of Photo{flo|map|tap} of image to be filled.
**	idc_id	    - id of image domain context, XieCpp or XieRoi
**	fill	    - array of constants
**
*******************************************************************************/
XiePhoto XieFill( src_id, idc_id, fill )
 XiePhoto	src_id;
 XieIdc		idc_id;
 unsigned long *fill;
{
    PhotoPtr      src = (PhotoPtr) src_id;
    PhotoPtr      dst = _XieGetDst(src);
    IdcPtr        idc = (IdcPtr) idc_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i, cmp_cnt =  PhotoCount_(src);
    xieQueryPhotomapReply rep;
    xieFillReq *req;

    if( fill == NULL  &&  !_XieQueryResource( src, &rep ) )
	return ( (XiePhoto)NULL );/* if you can't query it, you can't fill it*/
    /*
    **	Create a request to fill an area of the dst with a constant value.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieFill,req);
    req->src_photo_id    = ResId_(src);
    req->dst_photo_id    = ResId_(dst);
    req->dst_idc_id	 = idc == NULL ? 0 : ResId_(idc);
    for( i = 0; i < XieK_MaxComponents; i++ )
	req->constant[i] = i < cmp_cnt  &&  fill != NULL   ? fill[i]
			 : rep.polarity == XieK_ZeroBright ? 0 : ~0;
    XieReqDone_(dpy);
    return( (XiePhoto) dst);
}				    /* end XieFill */

/*******************************************************************************
**  XieHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**	Calculate the frequency histogram for the given image.
**
**  FORMAL PARAMETERS:
**
**	photo		- photomap id
**	idc		- region to calculate over
**	by_component	- calculate histogram by component
**
**  FUNCTION VALUE:
**
**	Address of histogram data.
**
*******************************************************************************/
CARD32	    *XieHistogram(photo, idc_id, by_component)
XiePhoto     photo;
XieIdc	     idc_id;
BOOL	     by_component;
{
    PhotoPtr		     src = (PhotoPtr)photo;
    IdcPtr		     idc = (IdcPtr)idc_id;
    Display		    *dpy =  ResDpy_(src);
    XieSessionPtr	     ses = _XieGetSession(dpy);
    xieCalcHistReq	    *req;

    xieCalcHistReply	     reply;
    CARD32		    *repdat = NULL;
    int			     repsiz;

    int			     status;
    /*
    **	Create request to point remap the image.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieCalcHist,req);

    req->photo_id	= ResId_(src);
    req->idc_id		= idc == NULL ? 0 : ResId_(idc);
    req->by_component	= by_component;
    /*
    **	Wait for reply...
    */
    status = _XReply(dpy, &reply, 0, False);
    if (!status)
    	_XieSrvError(dpy, req->opcode, req->photo_id, reply.sequenceNumber,
		     BadLength, "CalcHist : reply error");
    else
    {
	repsiz = reply.length << 2;
	repdat = (CARD32 *)XieMalloc(repsiz);
    	_XRead(dpy, repdat, repsiz);
	XieReqDone_(dpy);
    }

    return (repdat);
}

/*******************************************************************************
**  XieLogical
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create the bitwise logical combination of two server Photomap resources
**      or one server Photomap resource and a "constant" image.
**
**  FORMAL PARAMETERS:
**
**	src_id1	  - id of first  source Photo{flo|map|tap} to be processed
**	src_id2	  - id of second source Photo{flo|map|tap} to be processed
**      constant  - "constant" image values if src_id2 == 0.
**	idc_id	  - image domain context, XieCpp or XieRoi {optional}
**      operator  - request code for the operation to be performed
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieLogical( src_id1, src_id2, constant, idc_id, operator)

 XiePhoto   	src_id1;
 XiePhoto   	src_id2;
 unsigned long  *constant;
 XieIdc	        idc_id;
 unsigned long  operator;
{
    PhotoPtr dst;
    PhotoPtr src1 = (PhotoPtr) src_id1;
    PhotoPtr src2 = (PhotoPtr) src_id2;
    IdcPtr      idc   = (IdcPtr) idc_id;
    Display      *dpy =  ResDpy_(src1);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i, cmp_cnt =  PhotoCount_(src1);
    xieLogicalReq	 *req;

    if( constant == NULL  &&  !XieIsPhoto( src2 ) )
	return( NULL );	    /* One or the other required */

    dst = src2 == NULL || FloLnk_(src2) == NULL ? _XieGetDst(src1)
						:  FloLnk_(src2);
    /*
    **	Create a request to do logical between the operands.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieLogical,req);
    req->src1_photo_id	= ResId_(src1);
    req->src2_photo_id	= src2 == NULL ? 0 : ResId_(src2);
    for( i = 0; i < XieK_MaxComponents; i++ )
	req->constants[i] = i < cmp_cnt  &&  constant != NULL   ? constant[i]
		     : 0;
    req->dst_photo_id	= ResId_(dst);
    req->idc_id		= idc == NULL ? 0 : ResId_(idc);
    req->op		= operator;

    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieLogical */

/*******************************************************************************
**  XieLuminance
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a luminance only copy of a server Photomap resource
**
**  FORMAL PARAMETERS:
**
**	src_id	- id of Photo{flo|map|tap} from which to extract luminance info
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieLuminance( src_id )
 XiePhoto  src_id;
{
    PhotoPtr      src = (PhotoPtr) src_id;
    PhotoPtr      dst = _XieGetDst(src);
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieLuminanceReq *req;

    ResMap_(dst) = XieK_GrayScale;
    /*
    **	Create a request to extract luminance from the src into the dst.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieLuminance,req);
    req->src_photo_id   = ResId_(src);
    req->dst_photo_id	= ResId_(dst);
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieLuminance */

/*******************************************************************************
**  XieMatchHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a remapped copy of a server Photomap resource with the
**      requested distribution.
**
**  FORMAL PARAMETERS:
**
**	src_id	 - id of source Photo{flo|map|tap} to be remapped.
**      idc_id   - id of domain context, XieCpp or XieRoi {optional}
**      h_shape  - desired distribution
**                   {XieK_Flat, XieK_Gaussian, XieK_Hyperbolic}
**      param1   - First distribution parameter
**      param2   - Second distribution parameter
**      
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieMatchHistogram( src_id, idc_id, h_shape, param1, param2 )
 XiePhoto   	src_id;
 XieIdc		idc_id;
 unsigned long  h_shape;
 double         param1, param2;
{
    PhotoPtr	  src = (PhotoPtr) src_id;
    PhotoPtr	  dst = _XieGetDst(src);
    IdcPtr        idc = (IdcPtr)   idc_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieMatchHistogramReq  *req;

    /*
    **	Create request to remap the image with a new histogram.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieMatchHistogram,req);
    req->photo_id1	= ResId_(src);
    req->photo_id2	= ResId_(dst);
    req->idc_id         = idc == NULL ? 0 : ResId_(idc);
    req->h_shape        = h_shape;
    MiEncodeDouble( &req->param1, param1);
    MiEncodeDouble( &req->param2, param2);
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieMatchHistogram */

/*******************************************************************************
**  XieMath
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create the mathematical function of a server Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id	  - id of source Photo{flo|map|tap} to be processed
**	idc_id	  - image domain context, XieCpp or XieRoi {optional}
**      operator  - request code for the operation to be performed
**      constrain - output should be constrained (Boolean)
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieMath( src_id, idc_id, operator, constrain)
 XiePhoto   	src_id;
 XieIdc	        idc_id;
 unsigned long  operator;
 unsigned long  constrain;
{
    PhotoPtr dst;
    PhotoPtr src = (PhotoPtr) src_id;
    IdcPtr      idc   = (IdcPtr) idc_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i, cmp_cnt =  PhotoCount_(src);
    xieMathReq	 *req;

    dst = _XieGetDst(src);

    /*
    **	Create a request to do the math function
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieMath,req);
    req->src_photo_id	= ResId_(src);
    req->dst_photo_id	= ResId_(dst);
    req->idc_id		= idc == NULL ? 0 : ResId_(idc);
    req->op		= operator;
    req->constrain	= (constrain != 0);
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieMath */

/*******************************************************************************
**  XieMirror
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a mirrored copy of a server Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id	- id of source Photo{flo|map|tap} to be mirrored
**	x_mirror- mirror about the X axis (Boolean)
**	y_mirror- mirror about the Y axis (Boolean)
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieMirror( src_id, x_mirror, y_mirror )
 XiePhoto   	src_id;
 unsigned long  x_mirror, y_mirror;
{
    PhotoPtr	  src = (PhotoPtr) src_id;
    PhotoPtr	  dst = _XieGetDst(src);
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieMirrorReq	 *req;

    /*
    **	Create request to flip (mirror) the image.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieMirror,req);
    req->src_photo_id	= ResId_(src);
    req->dst_photo_id	= ResId_(dst);
    req->x_mirror	= x_mirror;
    req->y_mirror	= y_mirror;
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieMirror */

/*******************************************************************************
**  XiePoint
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a point remapped copy of a server Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id	 - id of source Photo{flo|map|tap} to be remapped.
**      idc_id   - id of domain context, XieCpp or XieRoi {optional}
**      tbl_id   - id of Photomap containing transfer function lookup table(s)
**                 (lookup tables).
**      
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XiePoint( src_id, idc_id, tbl_id )
 XiePhoto   	src_id;
 XieIdc		idc_id;
 XiePhoto       tbl_id;
 
{
    PhotoPtr	  src = (PhotoPtr) src_id;
    PhotoPtr	  dst = _XieGetDst(src);
    IdcPtr        idc = (IdcPtr)   idc_id;
    PhotoPtr      tbl = (PhotoPtr) tbl_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xiePointReq	 *req;

    /*
    **	Create request to point remap the image.
    */
    XieReq_(dpy,0,SesOpCode_(ses),iePoint,req);
    req->src_photo_id	= ResId_(src);
    req->dst_photo_id	= ResId_(dst);
    req->idc_id         = idc == NULL ? 0 : ResId_(idc);
    req->trans_photo_id = ResId_(tbl);
    XieReqDone_(dpy);

    /*
    **	Dst component mapping will be the same as the lookup tables.
    */
    ResMap_(dst) = ResMap_(tbl);

    return( (XiePhoto) dst );
}				    /* end XiePoint */

/*******************************************************************************
**  XiePointStats
**
**  FUNCTIONAL DESCRIPTION:
**
**	Returns statistic information about pixel values around a point in
**	the image.
**
**  FORMAL PARAMETERS:
**
**	src_id	 - id of source Photo{flo|map|tap} to be remapped.
**	     x   - x coordinate of point
**	     y	 - y coordinate of point
**	height	 - height of region
**	 width   - width of region
**	 value   - value of pixel at (x,y)
**	   min   - minimum value of region
**	   max	 - maximum value of region
**	  mean   - mean value of region
**	stddev	 - standard deviation of region
**         var   - variance of region
**      
*******************************************************************************/
void	XiePointStats(src_id, x, y, width, height, cmpmsk, stats)
 XiePhoto	src_id;
 unsigned long	x;
 unsigned long	y;
 unsigned long 	height;
 unsigned long 	width;
 unsigned long	cmpmsk;
 XieStatsRec	stats[];
{
    PhotoPtr		     src = (PhotoPtr) src_id;
    Display		    *dpy =  ResDpy_(src);
    XieSessionPtr	     ses = _XieGetSession(dpy);
    xiePointStatsReply	     reply;
    xiePointStatsReq	    *req;
    xiePointStatsDataPtr     data;
    int			     status;
    int			     data_bytes;
    int			     cmpcnt;
    int			     repidx;
    int			     retidx;
    /*
    **	Create request to point remap the image.
    */
    XieReq_(dpy,0,SesOpCode_(ses),iePointStats,req);
    req->photo_id	= ResId_(src);
    req->x		= x;
    req->y		= y;
    req->height		= height;
    req->width		= width;
    /*
    **	Wait for reply...
    */
    status = _XReply(dpy, &reply, 0, False);
    if (!status)
    	_XieSrvError(dpy, req->opcode, req->photo_id, reply.sequenceNumber,
		     BadLength, "PointStats : reply error");
    else
        {
	data_bytes = reply.length << 2;
	cmpcnt     = data_bytes / sizeof(xiePointStatsData);
	data       = (xiePointStatsDataPtr) XieMalloc(data_bytes);
    	_XRead(dpy, data, data_bytes);

	for( retidx = 0, repidx = 0;  repidx < cmpcnt;  repidx++ )
	    if( 1<<repidx & cmpmsk )
		{
		stats[retidx].value	= MiDecodeDouble(&data->pixel);
		stats[retidx].minimum	= MiDecodeDouble(&data->minimum);
		stats[retidx].maximum	= MiDecodeDouble(&data->maximum);
		stats[retidx].mean	= MiDecodeDouble(&data->mean);
		stats[retidx].stddev	= MiDecodeDouble(&data->std_dev);
		stats[retidx].variance	= MiDecodeDouble(&data->variance);
		retidx++; data++;
		}
	XieReqDone_(dpy);

	}
    XieFree(data);
}				    /* end XiePointStats */

/*******************************************************************************
**  XieRotate
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a rotated copy of a server Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id	- id of source Photo{flo|map|tap} to be rotated
**	angle	- angle of rotation
**	width	- width  of result Photomap (0 to allow rotate to calculate it)
**	height	- height of result Photomap (0 to allow rotate to calculate it)
**	fill	- fill pattern
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieRotate( src_id, angle, width, height, fill )
 XiePhoto   	src_id;
 double         angle;
 unsigned long	width, height, *fill;
{
    PhotoPtr dst, src = (PhotoPtr) src_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32 i, cmp_cnt =  PhotoCount_(src);
    xieQueryPhotomapReply rep;
    xieRotateReq	 *req;

    if( fill == NULL  &&  !_XieQueryResource( src, &rep ) )
	return( NULL );	    /* if you can't query it, you can't rotate it!  */

    dst = _XieGetDst(src);
    /*
    **	Create a request to rotate from the src into the dst.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieRotate,req);
    req->src_photo_id	= ResId_(src);
    req->dst_photo_id	= ResId_(dst);
    req->width		= width;
    req->height		= height;
    MiEncodeDouble( &req->angle, angle );
    for( i = 0; i < XieK_MaxComponents; i++ )
	req->fill[i] = i < cmp_cnt  &&  fill != NULL   ? fill[i]
		     : rep.polarity == XieK_ZeroBright ? 0 : ~0;
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieRotate */

/*******************************************************************************
**  XieScale
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a scaled copy of a server Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	src_id	- id of source Photo{flo|map|tap} to be scaled
**	width	- width  of scaled Photomap
**	height	- height of scaled Photomap
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of Photoflo passed in, or created Photomap
**
*******************************************************************************/
XiePhoto XieScale( src_id, width, height )
 XiePhoto	 src_id;
 unsigned long	 width;
 unsigned long	 height;
{
    PhotoPtr	  src = (PhotoPtr) src_id;
    PhotoPtr	  dst = _XieGetDst(src);
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieScaleReq  *req;

    /*
    **	Create a request to scale from the src into the dst.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieScale,req);
    req->src_photo_id	= ResId_(src);
    req->dst_photo_id	= ResId_(dst);
    req->width		= width;
    req->height		= height;
    XieReqDone_(dpy);

    return( (XiePhoto) dst );
}				    /* end XieScale */

/*******************************************************************************
**  XieTranslate
**
**  FUNCTIONAL DESCRIPTION:
**
**	Copy a region of interest from an existing Photomap resource 
**	into a region of interest of an existing Photomap, producing
**      a new photomap.
**
**  FORMAL PARAMETERS:
**
**	src1_id	    - id of first  Photo{flo|map|tap} to translate
**	src1_idc_id - id of first  image domain context, XieCpp or XieRoi
**	src2_id	    - id of second Photo{map|tap} for translate
**	src2_idc_id - id of second image domain context, XieCpp or XieRoi
**
*******************************************************************************/
XiePhoto XieTranslate( src1_id,  src1_idc_id, src2_id, src2_idc_id )
 XiePhoto   src1_id,	 src2_id;
 XieIdc	    src1_idc_id, src2_idc_id;
{
    PhotoPtr        src1 = (PhotoPtr)  src1_id;
    PhotoPtr        src2 = (PhotoPtr)  src2_id;
    PhotoPtr        dst  =  FloLnk_(src1) ? FloLnk_(src1) : _XieGetDst(src2);
    IdcPtr      src1_idc = (IdcPtr) src1_idc_id,
	        src2_idc = (IdcPtr) src2_idc_id;
    Display         *dpy =  ResDpy_(src2);
    XieSessionPtr    ses = _XieGetSession(dpy);
    xieTranslateReq *req;

    /*
    **	Create a request to translate ROI from the src into the dst.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieTranslate,req);
    req->src1_photo_id	= ResId_(src1);
    req->src2_photo_id	= ResId_(src2);
    req->dst_photo_id	= ResId_(dst);
    req->src_idc_id	= src1_idc == NULL ? 0 : ResId_(src1_idc);
    req->dst_idc_id	= src2_idc == NULL ? 0 : ResId_(src2_idc);

    XieReqDone_(dpy);
    return( (XiePhoto) dst );
}				    /* end XieTranslate */
/* end module XieLibProcess.c */
