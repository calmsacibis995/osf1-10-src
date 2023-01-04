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
**      This module contains XIE library routines for manipulating resources.
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
**      March 18, 1989
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
#define _XieLibResource

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
void		XieAbortFlo();		    /* Abort Photoflo execution	    */
void	        XieBindMapToFlo();	    /* bind Photomap to Photoflo    */
XiePhoto	XieClonePhoto();	    /* Photo{flo|map} from Photomap */
XieCpp	        XieCreateCpp();		    /* Idc type: Cpp  from args	    */
XiePhoto	XieCreatePhoto();	    /* Photo{flo|map} from args	    */
XieRoi	        XieCreateRoi();		    /* Idc type: Roi  from args	    */
XieTmp		XieCreateTmp();		    /* Idc type: Tmp  from args	    */
void	        XieExecuteFlo();	    /* Execute Photoflo pipeline    */
void		XieExport();		    /* export Photomap to drawable  */
XieResource	XieFindResource();	    /* find XieResource using XID   */
void		XieFreeExport();	    /* free export context	    */
XieResource	XieFreeResource();	    /* free a server resource	    */
XiePhotomap	XieImport();		    /* Photomap from X11 drawable   */
void	        XieQueryCpp();		    /* query Idc type: Cpp	    */
void		XieQueryExport();	    /* query ExportContext	    */
unsigned long	XieQueryFlo();		    /* query Photoflo status	    */
XiePhoto	XieQueryMap();		    /* query Photo{flo|map|tap}	attr*/
void	        XieQueryRoi();		    /* query Idc type: Roi	    */
XieTemplate	XieQueryTmp();		    /* query Idc type: Tmp	    */
XiePhoto	XieTapFlo();		    /* Photo{map|tap} from Photoflo */
    /*
    **  private routines called from other Xie modules.
    */
void	       _XieAddResource();	    /* allocate XID/add resource    */
Status	       _XieQueryResource();	    /* query a server resource	    */
XieResPtr      _XieGetDst();		    /* return destination resource  */
    /*
    **  internal routines.
    */
static void	FreeResource();		    /* free an Xie resource	    */

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
**  XieAbortFlo
**
**  FUNCTIONAL DESCRIPTION:
**
**	Abort Photoflo execution prematurely.
**
**  FORMAL PARAMETERS:
**
**	flo_id	- id of Photoflo to be aborted (or any bound Photo{map|tap}).
**
*******************************************************************************/
void XieAbortFlo( flo_id )
 XiePhoto   flo_id;
{
    PhotoPtr      flo =  FloLnk_((PhotoPtr)flo_id);
    Display      *dpy =  ResDpy_(flo);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieAbortPhotofloReq *req;

    /*
    **	Create a request to abort the photoflo.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieAbortPhotoflo,req);
    req->photoflo_id = ResId_(flo);
    XieReqDone_(dpy);
}				    /* end XieAbortFlo */

/*******************************************************************************
**  XieBindMapToFlo
**
**  FUNCTIONAL DESCRIPTION:
**
**	Bind a server Photomap resource to a server Photoflo resource.
**
**  FORMAL PARAMETERS:
**
**	phf_id	- id of Photoflo to bind (or any bound Photo{map|tap}).
**	phm_id	- id of Photomap to bind to to Photoflo
**
*******************************************************************************/
void XieBindMapToFlo( phf_id, phm_id )
 XiePhoto	phf_id;
 XiePhotomap	phm_id;
{
    PhotoPtr      phf =  FloLnk_((PhotoPtr)phf_id);
    PhotoPtr      phm = (PhotoPtr) phm_id;
    Display      *dpy =  ResDpy_(phf);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieBindPhotomapReq *req;

    if( !IsPhotomap_(phm)  ||  FloLnk_(phm) != NULL )
	return;		/* consider an error message for this */

    FloLnk_(phm)   = phf;		/* bind Photomap to Photoflo	    */
    PhotoLnk_(phm) = PhotoLnk_(phf);	/* insert Photomap at the head of   */
    PhotoLnk_(phf) = phm;		/* ... the Photoflo's Photomap list */
    /*
    **	Create request to bind the Photomap to the Photoflo.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieBindPhotomap,req);
    req->photoflo_id    = ResId_(phf);
    req->photomap_id    = ResId_(phm);
    XieReqDone_(dpy);
}				    /* end XieBindMapToFlo */

/*******************************************************************************
**  XieClonePhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**	Clone a server Photo{flo|map} resource from an existing Photomap.
**
**  FORMAL PARAMETERS:
**
**	phm_id	- Photomap id of input to Photoflo
**	type	- Photo type: XieK_Photoflo or XieK_Photomap
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of created Photo{flo|map}
**
*******************************************************************************/
XiePhoto XieClonePhoto( phm_id, type )
 XiePhotomap	phm_id;
 unsigned long	type;
{
    PhotoPtr      phm = (PhotoPtr) phm_id;
    PhotoPtr      pho =  IsPhotomap_(phm) ? _XieGetDst(phm) : NULL;
    Display      *dpy =  ResDpy_(phm);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieCreateByReferenceReq *req;

    if( pho == NULL  ||  IsPhotoflo_(pho) )
	return( NULL );	/* consider an error message for this */

    if( type == XieK_Photoflo )
	{
	FloLnk_(pho)   = pho;		    /* link Photoflo to itself	    */
	FloLnk_(phm)   = pho;		    /* bind Photomap to Photoflo    */
	PhotoLnk_(pho) = phm;		    /* link Photomap to Photoflo    */
	}
    ResType_(pho) = type;
    /*
    **	Request a Photo{flo|map} be created by reference, using "phm_id".
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieCreateByReference,req);
    req->resource_type	= type;
    req->resource_id	= ResId_(pho);
    req->photomap_id	= ResId_(phm);
    XieReqDone_(dpy);

    return( (XiePhoto) pho );
}				    /* end XieClonePhoto */

/*******************************************************************************
**  XieCreateCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a server control processing plane image domain context resource.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**	x	- X Photomap registration coordinate
**	y	- Y Photomap registration coordinate
**	phm_id	- Photomap id of control processing plane
**
**  FUNCTION VALUE:
**
**	XieCpp - id of created image domain context, of type Cpp
**
*******************************************************************************/
XieCpp XieCreateCpp( dpy, x, y, phm_id )
 Display    *dpy;
 long	     x, y;
 XiePhotomap phm_id;
{
    PhotoPtr      phm   = (PhotoPtr) phm_id;
    IdcPtr	  cpp;
    XieSessionPtr ses = _XieGetSession(dpy);
    xieCreateByValueReq *create;
    xieCreateCppReq	*req;

    if( !IsPhotomap_(phm) )
	return( NULL );	/* consider an error message for this */

    cpp = (IdcPtr) XieCalloc(1,sizeof(XieResRec));

    ResType_(cpp)  = XieK_IdcCpp;
    ResDpy_(cpp)   = dpy;			    /* save display pointer */
    PhotoLnk_(cpp) = phm;			    /* linkage to Photomap  */
   _XieAddResource(cpp);			    /* add to resource list */

    XieReqExtra_(dpy,0,SesOpCode_(ses),ieCreateByValue,create,
		sz_xieCreateCppReq-sz_xieCreateByValueReq);
    req = (xieCreateCppReq *) create;
    req->resource_type	= ResType_(cpp);
    req->resource_id	= ResId_(cpp);
    req->x		= x;
    req->y		= y;
    req->photomap_id	= ResId_(phm);
    XieReqDone_(dpy);

    return( (XieCpp) cpp );
}				    /* end XieCreateCpp */

/*******************************************************************************
**  XieCreatePhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a server Photoflo or Photomap resource.
**
**  FORMAL PARAMETERS:
**
**	dpy	    - pointer to X11 display structure
**	type	    - XieK_Photoflo or XieK_Photomap
**	width	    - width 
**	height	    - height
**	cmp_map	    - component mapping 
**	cmp_cnt	    - component count
**	cmp_lvl	    - levels-per-component list
**	pxl_pol	    - pixel brightness polarity 
**	pxl_ratio   - pixel aspect ratio 
**	pxl_prog    - pixel progression 
**	scn_prog    - line progression
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of created Photo{flo|map}
**
*******************************************************************************/
XiePhoto XieCreatePhoto( dpy, type, width, height, cmp_map, cmp_cnt, cmp_lvl,
				    pxl_pol, pxl_ratio, pxl_prog, scn_prog )
 Display      *dpy;
 unsigned long width, height, *cmp_lvl;
 unsigned char type,  cmp_map, cmp_cnt, pxl_pol, pxl_prog, scn_prog;
 double	       pxl_ratio;

{
    XieSessionPtr ses = _XieGetSession(dpy);
    xieCreateByValueReq *create;
    xieCreatePhotoReq   *req;
    CARD32 i;

    PhotoPtr  pho = (PhotoPtr) XieCalloc(1,sizeof(XieResRec));

    ResType_(pho) = type;
    ResMap_(pho)  = cmp_map;			    /* component mapping    */
    ResDpy_(pho)  = dpy;			    /* display pointer	    */
    FloLnk_(pho)  = IsPhotoflo_(pho) ? pho : NULL;  /* link self if Photoflo*/
   _XieAddResource(pho);			    /* add to resource list */
    /*
    **	Create request to establish a Photo{flo|map}.
    */
    XieReqExtra_(dpy,0,SesOpCode_(ses),ieCreateByValue,create,
	      sz_xieCreatePhotoReq-sz_xieCreateByValueReq);
    req = (xieCreatePhotoReq *) create;
    req->resource_type		= type;
    req->resource_id		= ResId_(pho);
    req->width			= width;
    req->height			= height;
    req->number_of_components	= cmp_cnt;
    req->component_mapping	= cmp_map;
    req->pixel_progression	= pxl_prog;
    req->line_progression	= scn_prog;
    req->polarity		= pxl_pol;
    MiEncodeDouble( &req->aspect_ratio, pxl_ratio );
    for( i = 0; i < XieK_MaxComponents; i++ )
        req->levels[i] = i < cmp_cnt ? cmp_lvl[i] : 0;

    XieReqDone_(dpy);

    return( (XiePhoto) pho );
}				    /* end XieCreatePhoto */

/*******************************************************************************
**  XieCreateRoi
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a server region of interest image domain context resource.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**	x	- X Photomap registration coordinate
**	y	- Y Photomap registration coordinate
**	width	- width  of region
**	height	- height of region
**
**  FUNCTION VALUE:
**
**	XieRoi - id of created image domain context, of type Roi
**
*******************************************************************************/
XieRoi XieCreateRoi( dpy, x, y, width, height )
 Display	*dpy;
 long		 x, y;
 unsigned long	 width, height;
{
    XieSessionPtr ses = _XieGetSession(dpy);
    xieCreateByValueReq *create;
    xieCreateRoiReq	*req;

    IdcPtr roi = (IdcPtr) XieCalloc(1,sizeof(XieResRec));

    ResType_(roi) = XieK_IdcRoi;
    ResDpy_(roi)  = dpy;			    /* save display pointer */
   _XieAddResource(roi);			    /* add to resource list */
    /*
    **	Create request to establish an Roi.
    */
    XieReqExtra_(dpy,0,SesOpCode_(ses),ieCreateByValue,create,
		sz_xieCreateRoiReq-sz_xieCreateByValueReq);
    req = (xieCreateRoiReq *) create;
    req->resource_type	= ResType_(roi);
    req->resource_id	= ResId_(roi);
    req->x		= x;
    req->y		= y;
    req->width		= width;
    req->height		= height;
    XieReqDone_(dpy);

    return( (XieRoi) roi );
}				    /* end XieCreateRoi */

/*******************************************************************************
**  XieCreateTmp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a server template image domain context resource.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**	x	- X coordinate of center of template
**	y	- Y coordinate of center of template
**	count	- number of XieTemplateRec's in list
**	list	- array of XieTemplateRec
**
**  FUNCTION VALUE:
**
**	XieTmp - id of created image domain context, of type Tmp
**
*******************************************************************************/
XieTmp XieCreateTmp( dpy, x, y, count, list )
 Display	*dpy;
 long		 x, y;
 unsigned long	 count;
 XieTemplate	 list;
{
    XieSessionPtr ses = _XieGetSession(dpy);
    xieCreateByValueReq *create;
    xieCreateTmpReq	*req;
    XieTmpEntryRec	*data;
    CARD32 i;
    IdcPtr tmp = (IdcPtr) XieCalloc(1,sizeof(XieResRec));

    ResType_(tmp) = XieK_IdcTmp;
    ResDpy_(tmp)  = dpy;			    /* save display pointer */
   _XieAddResource(tmp);			    /* add to resource list */

    XieReqExtra_(dpy,0,SesOpCode_(ses),ieCreateByValue,create,
		sz_xieCreateTmpReq-sz_xieCreateByValueReq
				  +sizeof(XieTmpEntryRec)*count);
    req = (xieCreateTmpReq *) create;
    req->resource_type = ResType_(tmp);
    req->resource_id   = ResId_(tmp);
    req->center_x      = x;
    req->center_y      = y;
    req->data_count    = count;
    for( data = (XieTmpEntryRec *) &req[1], i = 0; i < count; i++ )
	{
	data[i].x      = list[i].x;
	data[i].y      = list[i].y;
	MiEncodeDouble( &data[i].value, list[i].value );
	}
    XieReqDone_(dpy);
    return( (XieTmp) tmp );
}				    /* end XieCreateTmp */

/*******************************************************************************
**  XieExecuteFlo
**
**  FUNCTIONAL DESCRIPTION:
**
**	Begin Photoflo execution.
**
**  FORMAL PARAMETERS:
**
**	flo_id	- id of Photoflo to be executed (or any bound Photo{map|tap}).
**
*******************************************************************************/
void XieExecuteFlo( flo_id )
 XiePhoto   flo_id;
{
    PhotoPtr      flo =  FloLnk_((PhotoPtr)flo_id);
    Display      *dpy =  ResDpy_(flo);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieExecutePhotofloReq *req;

    /*
    **	Create a request to execute the photoflo.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieExecutePhotoflo,req);
    req->photoflo_id	= ResId_(flo);
    XieReqDone_(dpy);
}				    /* end XieExecuteFlo */

/*******************************************************************************
**  XieExport
**
**  FUNCTIONAL DESCRIPTION:
**
**	Export image data from a Photomap to an X11 drawable.
**
**  FORMAL PARAMETERS:
**
**	src_id	- id of source Photo{flo|map|tap} to export from
**	d	- drawable id
**	gc	- graphics context id
**	sx, sy	- from Photomap X,Y coordinate
**	dx, dy	- to drawable X,Y coordinate
**	w, h	- dimensions of region to be exported
**	lut_id	- optional: LUT to map pixels to pre-allocated palette
**	cmap_id - optional: X11 colormap assigned to target drawable
**	match	- optional: granularity for pixel matching
**	gray	- optional: purity of gray shades
**
*******************************************************************************/
void XieExport( src_id, d, gc, sx,sy, dx,dy, w,h, lut_id, cmap_id, match, gray )
 XiePhoto   	src_id, lut_id;
 Drawable	d;
 GC		gc;
 long		sx, sy, dx, dy;
 unsigned long	w,  h;
 Colormap	cmap_id;
 double		match, gray;
{
    PhotoPtr      src = (PhotoPtr) src_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieExportReq *req;
    /*
    **	Create request to export image data from Photomap to drawable.
    */
    XieReq_(dpy,gc,SesOpCode_(ses),ieExport,req);
    req->photo_id	= ResId_(src);
    req->drawable_id	= d;
    req->gc_id		= gc->gid;
    req->src_x		= sx;
    req->src_y		= sy;
    req->width		= w;
    req->height		= h;
    req->dst_x		= dx;
    req->dst_y		= dy;
    req->photo_lut_id	= lut_id == 0 ? 0 : XId_(lut_id);
    req->colormap_id	= cmap_id;
    MiEncodeDouble( &req->match_limit, match );
    MiEncodeDouble( &req->gray_limit,  gray  );
    XieReqDone_(dpy);
}				    /* end XieExport */

/*******************************************************************************
**  XieFindResource
**
**  FUNCTIONAL DESCRIPTION:
**
**	Find an XieResource using the resource's X11 XID.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**	id	- resource's X11 XID
**
**  FUNCTION VALUE:
**
**	XieResource - XieResource id
**
*******************************************************************************/
XieResource XieFindResource( dpy, id )
 Display *dpy;
 XID	  id;
{
    XieResPtr     res;
    XieSessionPtr ses = _XieGetSession(dpy);

    for( res = SesResLst_(ses); res != NULL; res = ResLnk_(res) )
	if( ResId_(res) == id )
	    break;

    return( (XieResource) res );
}				    /* end XieFindResource */

/*******************************************************************************
**  XieFreeExport
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free ExportContext.
**
**  FORMAL PARAMETERS:
**
**	src_id	    - id of source Photo{flo|map|tap}
**
*******************************************************************************/
void XieFreeExport( src_id )
 XiePhoto	src_id;
{
    PhotoPtr	  src = (PhotoPtr) src_id;
    Display      *dpy =  ResDpy_(src);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieFreeExportReq  *req;
    /*
    **	Create a request to free ExportContext.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieFreeExport,req);
    req->photo_id   = ResId_(src);
    XieReqDone_(dpy);
}				    /* end XieFreeExport */

/*******************************************************************************
**  XieFreeResource
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free a server image extension resource
**
**  FORMAL PARAMETERS:
**
**	res_id	- id of resource to be freed
**
**  FUNCTION VALUE:
**
**	NULL
**
*******************************************************************************/
XieResource  XieFreeResource( res_id )
 XieResource	res_id;
{
    XieResPtr res = (XieResPtr) res_id;
    PhotoPtr  phm, nxt;

    if( res != NULL )
	switch( ResType_(res) )
	    {
	case XieK_Photoflo :
	    for( phm = PhotoLnk_(res); phm != NULL; phm = nxt )
		{
		nxt = PhotoLnk_(phm);
		PhotoLnk_(phm) = NULL;	    /* zap Photo{map|tap} list	    */
		FloLnk_(phm)   = NULL;	    /* zap Photoflo binding	    */
		if( IsPhototap_(phm) )
		    FreeResource(phm);	    /* free the Phototap 	    */
		}	
	    FreeResource( res );	    /* free the Photoflo 	    */
	    break;

	case XieK_Photomap :
	case XieK_Phototap :
	    if( FloLnk_(res) != NULL )
		/*
		**  Unbind this Photo{map|tap} from the Photoflo.
		*/
		for( phm = FloLnk_(res); phm != NULL; phm = PhotoLnk_(phm) )
		    if( PhotoLnk_(phm) == res )
			PhotoLnk_(phm)  = PhotoLnk_(PhotoLnk_(phm));

	    FreeResource( res );	    /* free Photo{map|tap} resource */
	    break;

	case XieK_IdcCpp :
	case XieK_IdcRoi :
	case XieK_IdcTmp :
	    FreeResource( res );	    /* free Idc type resource	    */
	    break;
	    }
    return( NULL );
}				    /* end XieFreeResource */

/*******************************************************************************
**  XieImport
**
**  FUNCTIONAL DESCRIPTION:
**
**	Import image data from an X11 drawable into a new Photomap.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**	drawable- drawable id
**	cmap_id - X11 colormap id
**	pxl_pol	- pixel brightness polarity
**
**  FUNCTION VALUE:
**
**	XiePhotomap - id of created Photomap
**
*******************************************************************************/
XiePhotomap XieImport( dpy, drawable, cmap_id, pxl_pol )
 Display	*dpy;
 Drawable	 drawable;
 Colormap	 cmap_id;
 unsigned char	 pxl_pol;
{
    XieSessionPtr ses = _XieGetSession(dpy);
    xieImportReq *req;
    xieQueryPhotomapReply rep;
    PhotoPtr  phm = (PhotoPtr) XieCalloc(1,sizeof(XieResRec));

    ResType_(phm) = XieK_Photomap;		/* resource type	    */
    ResDpy_(phm)  = dpy;			/* copy display pointer	    */
   _XieAddResource(phm);			/* add to our resource list */
    /*
    **	Create request to import image data from drawable to Photomap.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieImport,req);
    req->photomap_id	= ResId_(phm);
    req->drawable_id	= drawable;
    req->colormap_id	= cmap_id;
    req->polarity	= pxl_pol;
    XieReqDone_(dpy);

    if( _XieQueryResource( phm, &rep ) )
	{
	ResMap_(phm) = rep.component_mapping;
	XieReqDone_(dpy);
	}

    return( (XiePhotomap) phm );
}				    /* end XieImport */

/*******************************************************************************
**  XieQueryCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Obtain control processing plane context from server.
**
**  FORMAL PARAMETERS:
**
**	cpp_id	- Cpp id
**	x_ret	- X return pointer
**	y_ret	- Y return pointer
**	phm_ret	- Photomap return pointer
**
*******************************************************************************/
void XieQueryCpp( cpp_id, x_ret, y_ret, phm_ret )
 XieCpp	      cpp_id;
 long	     *x_ret, *y_ret;
 XiePhotomap *phm_ret;
{
    IdcPtr    cpp = (IdcPtr) cpp_id;
    Display  *dpy =  ResDpy_(cpp);
    xieQueryCppReply rep;

    /*
    **	Query the Cpp.
    */
    if( _XieQueryResource( cpp, &rep ) )
	{
	XieReqDone_(dpy);
	/*
	**  Unload the reply packet.
	*/
	if(  x_ret   != NULL )
	    *x_ret    = rep.x;
	if(  y_ret   != NULL )
	    *y_ret    = rep.y;
	if(  phm_ret != NULL )
	    *phm_ret  = (XiePhotomap) PhotoLnk_(cpp);
	}
}				    /* end XieQueryCpp */

/*******************************************************************************
**  XieQueryExport
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return ExportContext LUT and allocated pixel list.
**
**  FORMAL PARAMETERS:
**
**	pho_id	    - Photoflo id (or any bound Photo{map|tap}).
**	lut_id_ret  - where to stash returned LUT_id (or NULL).
**	pxl_lst_ret - where to stash returned pixel list pointer
**			(none are returned if pxl_lst or pxl_cnt are NULL).
**	pxl_cnt_ret - where to stash count of allocated pixels.
**
*******************************************************************************/
void XieQueryExport( pho_id, lut_id_ret, pxl_lst_ret, pxl_cnt_ret )
 XiePhoto	  pho_id;
 XiePhotomap	 *lut_id_ret;
 unsigned long	**pxl_lst_ret;
 unsigned long   *pxl_cnt_ret;
{
    PhotoPtr       pho = (PhotoPtr) pho_id;
    PhotoPtr      *lut = (PhotoPtr *) lut_id_ret;
    Display       *dpy =  ResDpy_(pho);
    unsigned long *pxl =  NULL;
    XieSessionPtr  ses = _XieGetSession(dpy);
    xieQueryExportReq  *req;
    xieQueryExportReply rep;

    if( lut != NULL )
	{   /*
	    **	Begin to create a Photomap record for the LUT.
	    */
	*lut = (PhotoPtr) XieCalloc(1,sizeof(XieResRec));
	ResType_(*lut) = XieK_Photomap;		/* resource type for lut    */
	ResDpy_(*lut)  = dpy;			/* copy display pointer	    */
	ResId_(*lut)   = XAllocID(dpy);		/* resource id for lut	    */
	}
    /*
    **	Query the ExportContext.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieQueryExport,req);
    req->photo_id    = ResId_(pho);
    req->lut_id      = lut != NULL ? ResId_(*lut) : 0;
    req->get_pixels  = pxl_lst_ret != NULL && pxl_cnt_ret != NULL;

    if( !_XReply( dpy, &rep, 0, !req->get_pixels ) )
	{   /*
	    **	Reply error.
	    */
	_XieSrvError( dpy, req->opcode, req->photo_id, rep.sequenceNumber,
		      BadLength, "QueryExport: reply error" );

	if( lut != NULL )
	   *lut  = (PhotoPtr) XieFree( *lut );	/* no lut on error	    */
	rep.pixel_count = 0;			/* no pixel count on error  */
	}
    else
	{
	if( rep.length > 0 )
	    {	/*
		**  Get the allocated pixel list.
		*/
	    pxl = (unsigned long *) XieMalloc(rep.pixel_count * 
					sizeof(unsigned long));
		/*
		**  Pixels from X are ALWAYS CARD32's.
		*/
	   _XRead( dpy, pxl, rep.length * sizeof(CARD32));
		/*
		**  If longs are greater than CARD32's, space the data out from
		**  the CARD32 array to fill out the long array.
		*/
	    if (sizeof (CARD32) != sizeof (unsigned long))
		{
		unsigned long *pxl_long = (pxl + rep.length - 1);
		CARD32 *pxl_card32 = (((CARD32 *)pxl) + rep.length - 1);

		while (pxl_long >= pxl)
		    *pxl_long-- = (unsigned long)*pxl_card32--;
		}
	    }
	XieReqDone_(dpy);			/* unlock the display	    */

	if( lut != NULL )
	    if( rep.lut_id == ResId_(*lut) )
		{   /*
		    **  Finish creating the LUT's Photomap.
		    */
		ResMap_(*lut)   =  rep.lut_mapping;
		ResLnk_(*lut)   =  SesResLst_(ses);
		SesResLst_(ses) = *lut;
		}
	    else
		{   /*
		    **  Trash new LUT -- instead find LUT returned in reply.
		    */
		*lut = (PhotoPtr) XieFree( *lut );
		if( rep.lut_id != 0 )
		    *lut_id_ret = XieFindResource( dpy, rep.lut_id );
		}
	}
    if(  pxl_lst_ret != NULL )
	*pxl_lst_ret  = pxl;			/* return pixel list, if any*/

    if(  pxl_cnt_ret != NULL )
 	*pxl_cnt_ret = rep.pixel_count;		/* return pixel count	    */

}				    /* end XieQueryExport */

/*******************************************************************************
**  XieQueryFlo
**
**  FUNCTIONAL DESCRIPTION:
**
**	Obtain Photoflo status from server.
**
**  FORMAL PARAMETERS:
**
**	phf_id	    - Photoflo id (or any bound Photo{map|tap}).
**
**  FUNCTION VALUE:
**
**	status
**
*******************************************************************************/
unsigned long XieQueryFlo( phf_id )
 XiePhoto     phf_id;
{
    PhotoPtr       phf =  FloLnk_((PhotoPtr)phf_id);
    Display       *dpy =  ResDpy_(phf);
    XieSessionPtr  ses = _XieGetSession(dpy);
    xieQueryResourceReq  *req;
    xieQueryPhotofloReply rep;

    /*
    **	Query the Photoflo.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieQueryResource,req);
    req->resource_type	= ResType_(phf);
    req->resource_id	= ResId_(phf);

    if( !_XReply( dpy, &rep, 0, True ) )
	_XieSrvError( dpy, req->opcode, req->resource_id, rep.sequenceNumber,
		      BadLength, "QueryPhotoflo: reply error" );
    else
	XieReqDone_(dpy);

    return( rep.status );
}				    /* end XieQueryFlo */

/*******************************************************************************
**  XieQueryMap
**
**  FUNCTIONAL DESCRIPTION:
**
**	Obtain Photomap attributes from Photo{flo|map|tap}.
**
**  FORMAL PARAMETERS:
**
**	pho_id		- Photo{flo|map|tap} id
**	width_ret	- width return pointer
**	height_ret	- height return pointer
**	cmp_map_ret	- component mapping return pointer
**	cmp_cnt_ret	- component count return pointer
**	cmp_lvl_ret	- levels-per-component return pointer
**	pxl_pol_ret	- polarity return pointer
**	pxl_ratio_ret	- aspect ratio return pointer
**	pxl_prog_ret	- pixel progression return pointer
**	scn_prog_ret	- line progression return pointer
**	constrained_ret	- constrained flag return pointer
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of queried Photo
**
*******************************************************************************/
XiePhoto XieQueryMap( pho_id,       width_ret,     height_ret,
		      cmp_map_ret,  cmp_cnt_ret,   cmp_lvl_ret,
		      pxl_pol_ret,  pxl_ratio_ret, pxl_prog_ret,
		      scn_prog_ret, constrained_ret )
 XiePhoto        pho_id;
 unsigned long  *width_ret;
 unsigned long	*height_ret;
 unsigned char	*cmp_map_ret;
 unsigned char	*cmp_cnt_ret;
 unsigned long	*cmp_lvl_ret;
 unsigned char	*pxl_pol_ret;
 double		*pxl_ratio_ret;
 unsigned char	*pxl_prog_ret;
 unsigned char	*scn_prog_ret;
 unsigned char	*constrained_ret;

{
    PhotoPtr  pho = (PhotoPtr) pho_id;
    Display  *dpy =  ResDpy_(pho);
    xieQueryPhotomapReply rep;
    CARD32  i;

    /*
    **	Query the Photo{flo|map|tap} for Photomap attributes.
    */
    if( _XieQueryResource( pho, &rep ) )
	{
	XieReqDone_(dpy);
	/*
	**  Unload the reply packet.
	*/
	if(  width_ret       != NULL )
	    *width_ret        = rep.width;
	if(  height_ret      != NULL )
	    *height_ret       = rep.height;
	if(  cmp_map_ret     != NULL )
	    *cmp_map_ret      = rep.component_mapping;
	if(  cmp_cnt_ret     != NULL )
	    *cmp_cnt_ret      = rep.component_count;
	if(  pxl_pol_ret     != NULL )
	    *pxl_pol_ret      = rep.polarity;
	if(  pxl_ratio_ret   != NULL )
	    *pxl_ratio_ret    = MiDecodeDouble( &rep.aspect_ratio );
	if(  pxl_prog_ret    != NULL )
	    *pxl_prog_ret     = rep.pixel_progression;
	if(  scn_prog_ret    != NULL )
	    *scn_prog_ret     = rep.line_progression;
	if(  constrained_ret != NULL )
	    *constrained_ret  = rep.constrained;
	if(  cmp_lvl_ret     != NULL )
	    for( i = 0; i < XieK_MaxComponents; i++ )
		cmp_lvl_ret[i] = i < rep.component_count ? rep.levels[i] : 0;
	}
    return( rep.photo_id == 0 ? NULL : rep.photo_id == ResId_(pho) ? pho_id
				     : XieFindResource( dpy, rep.photo_id ));
}				    /* end XieQueryMap */

/*******************************************************************************
**  XieQueryRoi
**
**  FUNCTIONAL DESCRIPTION:
**
**	Obtain region of interest context from server.
**
**  FORMAL PARAMETERS:
**
**	roi_id	    - Roi id
**	x_ret	    - X return pointer
**	y_ret	    - Y return pointer
**	width_ret   - width  return pointer
**	height_ret  - height return pointer
**
*******************************************************************************/
void XieQueryRoi( roi_id, x_ret, y_ret, width_ret, height_ret )
 XieRoi           roi_id;
 long		 *x_ret,     *y_ret;
 unsigned long   *width_ret, *height_ret;
{
    IdcPtr    roi = (IdcPtr) roi_id;
    Display  *dpy =  ResDpy_(roi);
    xieQueryRoiReply rep;

    /*
    **	Query the Roi.
    */
    if( _XieQueryResource( roi, &rep ) )
	{
	XieReqDone_(dpy);
	/*
	**  Unload the reply packet.
	*/
	if(  x_ret      != NULL )
	    *x_ret       = rep.x;
	if(  y_ret      != NULL )
	    *y_ret       = rep.y;
	if(  width_ret  != NULL )
	    *width_ret   = rep.width;
	if(  height_ret != NULL )
	    *height_ret  = rep.height;
	}
}				    /* end XieQueryRoi */

/*******************************************************************************
**  XieQueryTmp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Obtain template context from server.
**
**  FORMAL PARAMETERS:
**
**	tmp_id	    - Tmp id
**	x_ret	    - center X return pointer
**	y_ret	    - center Y return pointer
**	count_ret   - XieTemplateRec count return pointer
**
**  FUNCTION VALUE:
**
**	XieTemplate - pointer to list of XieTemplateRec (dealloc with XieFree)
**
*******************************************************************************/
XieTemplate XieQueryTmp( tmp_id, x_ret, y_ret, count_ret )
 XieTmp          tmp_id;
 long		*x_ret, *y_ret;
 unsigned long  *count_ret;
{
    IdcPtr    tmp = (IdcPtr) tmp_id;
    Display  *dpy =  ResDpy_(tmp);
    xieQueryTmpReply rep;
    XieTemplate	     data = NULL;
    XieTmpEntryRec   entry;
    CARD32  i;

    if( _XieQueryResource( tmp, &rep ) )
	{
	if(  x_ret      != NULL )
	    *x_ret       = rep.center_x;
	if(  y_ret      != NULL )
	    *y_ret       = rep.center_y;
	if(  count_ret  != NULL )
	    *count_ret   = rep.data_count;

	data = (XieTemplate) XieMalloc(rep.data_count*sizeof(XieTemplateRec));

	for( i = 0; i < rep.data_count; i++ )
	    {
	    _XRead( dpy, &entry, sizeof(XieTemplateRec) );
	    data[i].x     = entry.x;
	    data[i].y     = entry.y;
	    data[i].value = MiDecodeDouble( &entry.value );
	    }
	XieReqDone_(dpy);
	}
    return( data );
}				    /* end XieQueryTmp */

/*******************************************************************************
**  XieTapFlo
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get a Photo{map|tap}-id for the Photomap at the of a Photoflo.
**
**  FORMAL PARAMETERS:
**
**	flo_id	  - Photoflo id (or any bound Photo{map|tap}).
**	permanent - Boolean: true=permanent Photomap, false=ephemeral Phototap
**
**  FUNCTION VALUE:
**
**	XiePhoto - id of permanent Photomap or ephemeral Phototap
**
*******************************************************************************/
XiePhoto XieTapFlo( flo_id, permanent )
 XiePhoto	flo_id;
 unsigned char	permanent;
{
    PhotoPtr      flo =  FloLnk_((PhotoPtr)flo_id);
    Display      *dpy =  ResDpy_(flo);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieTapPhotofloReq	*req;

    PhotoPtr   tap = (PhotoPtr) XieMalloc(sizeof(XieResRec));
   *tap = *flo;
    ResType_(tap)  = permanent ? XieK_Photomap : XieK_Phototap;
    PhotoLnk_(flo) = tap;			/* bind to Photoflo	    */
   _XieAddResource(tap);			/* add to the resource list */
    /*
    **  Create request to tap the end of the Photoflo.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieTapPhotoflo,req);
    req->photoflo_id    = ResId_(flo);
    req->photo_id	= ResId_(tap);
    req->permanent	= permanent;
    XieReqDone_(dpy);

    return( (XiePhoto) tap );
}				    /* end XieTapFlo */

/*******************************************************************************
**  _XieAddResource
**
**  FUNCTIONAL DESCRIPTION:
**
**	Add an XieResource to our resource list.
**
**  FORMAL PARAMETERS:
**
**	res	- resource pointer
**
*******************************************************************************/
void _XieAddResource( res )
 XieResPtr res;
{
    Display      *dpy =  ResDpy_(res);
    XieSessionPtr ses = _XieGetSession(dpy);

    /*
    **	Allocate an X11 resource-id.
    */
    ResId_(res) = XAllocID(dpy);

    /*
    **	Stash the new resource at the head of the list.
    */
    ResLnk_(res)    = SesResLst_(ses);
    SesResLst_(ses) = res;
}				    /* end _XieAddResource */

/*******************************************************************************
**  _XieGetDst
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create or point to an Xie destination resource.
**
**  FORMAL PARAMETERS:
**
**	src	- pointer to source resource.
**
**  FUNCTION VALUE:
**
**	XieResPtr - pointer to destination
**
*******************************************************************************/
XieResPtr _XieGetDst( src )
 XieResPtr  src;
{
    XieResPtr dst;

    if( FloLnk_(src) == NULL )
	{
	/*
	**  Create a new destination resource of the same type.
	*/
	dst = (XieResPtr) XieMalloc(sizeof(XieResRec));
       *dst = *src;
       _XieAddResource(dst);
	}
    else
	/*
	**  Destination is the binding Photoflo.
	*/
	dst = FloLnk_(src);

    return( dst );
}				    /* end _XieGetDst */

/*******************************************************************************
**  _XieQueryResource
**
**  FUNCTIONAL DESCRIPTION:
**
**	Obtain resource information from server.
**
**  FORMAL PARAMETERS:
**
**	res	- pointer to resource to be queried
**	reply	- pointer to reply structure
**
**  FUNCTION VALUE:
**
**	Boolean - True if reply is successful
**
*******************************************************************************/
Status _XieQueryResource( res, reply )
 XieResPtr      res;
 xGenericReply *reply;
{
    Display      *dpy =  ResDpy_(res);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieQueryResourceReq *req;
    Status status;

    /*
    **	Create request to query the specified resource.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieQueryResource,req);
    req->resource_type = IsPhoto_(res) ? XieK_Photomap : ResType_(res);
    req->resource_id   = ResId_(res);

    status = _XReply( dpy, reply, 0, !(IsPhoto_(res) || IsTmp_(res)) );

    if( !status )
	_XieSrvError(dpy, req->opcode, req->resource_id, reply->sequenceNumber,
		     BadLength, "QueryResource: reply error");

    else if( IsPhoto_(res) )
	_XRead( dpy, &reply[1], reply->length<<2 );

    return( status );
}				    /* end _XieQueryResource */

/*******************************************************************************
**  FreeResource
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free an Xie resource
**
**  FORMAL PARAMETERS:
**
**	res	- pointer to resource to be freed
**
*******************************************************************************/
static void FreeResource( res )
 XieResPtr  res;
{
    Display      *dpy =  ResDpy_(res);
    XieSessionPtr ses = _XieGetSession(dpy);
    XieResPtr	  lst;
    xieDeleteResourceReq *req;

    /*
    **	Remove this resource from our resource list.
    */
    if( SesResLst_(ses) == res )
	SesResLst_(ses)  = ResLnk_(res);
    else
	for( lst = SesResLst_(ses); lst != NULL; lst = ResLnk_(lst) )
	    if( ResLnk_(lst) == res )
		{
		ResLnk_(lst)  = ResLnk_(res);
		break;
		}

    if( !IsPhototap_(res) )
	{   /*
	    **  Create a request to delete the server resource.
	    */
	XieReq_(dpy,0,SesOpCode_(ses),ieDeleteResource,req);
	req->resource_type  = ResType_(res);
	req->resource_id    = ResId_(res);
	XieReqDone_(dpy);
	}
    XieFree( res );
}				    /* end FreeResource */
/* end module XieLibResource.c */
