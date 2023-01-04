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
**	This module contains XieImage routines.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Robert NC Shelley
**
**  CREATION DATE:
**
**	April 17, 1989
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
#define _XieLibImage

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include <math.h>
#include <stdio.h>
    /*
    **  X11 and XIE include files
    */
#ifdef VMS
#include <Xlibint.h>			/* X11 internal lib/transport defs  */
#else
#include <X11/Xlibint.h>		/* X11 internal lib/transport defs  */
#endif
#include <XieDdx.h>			/* XIE client/server common defs    */
#include <XieProto.h>			/* XIE Req/Reply protocol defs	    */
#include <XieLib.h>			/* XIE public  definitions	    */
#include <XieLibint.h>			/* XIE private definitions	    */

/*
**  Table of contents
*/
    /*
    **  PUBLIC entry points
    */
XieImage      XieCopyImage();	/* Image from Image (optional data)	    */
XieImage      XieCreateImage();	/* Image from argumemts (no data)	    */
XieImage      XieFreeImage();	/* free Image resources			    */
XieImage      XieGetImage();	/* Image from Photo{any} (optional data)    */
unsigned char XieGetImageData();/* transport image data from Photo{any}	    */
XiePhoto      XiePutImage();	/* Photo{any} from Image (optional data)    */
unsigned char XiePutImageData();/* transport image data to Photo{flo|map}   */
    /*
    **  internal routines
    */
static void   CreateXport();	/* create XieImage stream transport context */

/*
**  Macro definitions
*/

/*
**  Equated symbols
*/

/*
**  Externals
*/

/*
**  Local Storage
*/

/*******************************************************************************
**  XieCopyImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create an XieImage by copying an existing XieImage.
**
**  FORMAL PARAMETERS:
**
**	img	- pointer to XieImageRec
**	mode	- what to do about image data:
**		    XieK_DataNone   - no image buffers, no data copy
**		    XieK_DataAlloc  - allocate new image buffers only, no data
**		    XieK_DataCopy   - allocate new image buffers and copy data
**		    XieK_DataShare  - copy pointers to image data (share)
**
**  FUNCTION VALUE:
**
**	XieImage - pointer to new XieImageRec
**
*******************************************************************************/
XieImage XieCopyImage( img, mode )
 XieImage	img;
 unsigned char	mode;
{
    CARD32  i;

    /*
    **	Make a copy of the XieImageRec.
    */
    XieImage copy  = (XieImage) XieMalloc(sizeof(XieImageRec));
    *copy = *img;
    OwnData_(copy) = False;

    /*
    **	Make a copy of each existing UdpRec.
    */
    for( i = 0; i < XieK_MaxPlanes; i++ )
	if( Plane_(img,i) != NULL )
	    {
	     Plane_(copy,i)  = (UdpPtr) XieMalloc(sizeof(UdpRec));
	    *Plane_(copy,i)  = *Plane_(img,i);

	    switch( mode )
		{
	    case XieK_DataNone :
		uBase_(copy,i)   = NULL;
		break;

	    case XieK_DataAlloc :
		OwnData_(copy)   = True;
		uBase_(copy,i)   = XieCallocBits( uArSize_(copy,i) );
		break;

	    case XieK_DataCopy :
		if( uBase_(img,i) != NULL )
		    {
		    OwnData_(copy) = True;
		    uBase_(copy,i) = XieMallocBits( uArSize_(img,i) );
		    memcpy(uBase_(copy,i),uBase_(img,i),uArSize_(img,i)+7>>3);
		    }
		break;

	    case XieK_DataShare :
		break;

	    default : _XieLibError( EINVAL, "XieCopyImage (mode)" );
		}
	    }
    return( copy );
}				    /* end XieCopyImage */

/*******************************************************************************
**  XieCreateImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create an XieImage as specified by formal parameters -- no image data.
**
**  FORMAL PARAMETERS:
**
**	width	- width  of proposed image data
**	height	- height of proposed image data
**	cmpres	- compression scheme
**	cmp_org	- component organization
**	cmp_map	- component mapping
**	cmp_cnt - number of components
**	cmp_lvl	- array of component levels
**	pxl_mod	- pixel stride modulo
**	scn_mod	- scanline stride modulo
**
**  FUNCTION VALUE:
**
**	XieImage - pointer to new XieImageRec
**
*******************************************************************************/
XieImage XieCreateImage( width, height, cmpres,
			 cmp_org, cmp_map, cmp_cnt, cmp_lvl, pxl_mod, scn_mod )
 unsigned long width,  height, *cmp_lvl;
 unsigned char cmpres, cmp_org, cmp_map, cmp_cnt, pxl_mod, scn_mod;
{
    CARD32  i, levels, pxl_bits, planes;
    /*
    **	Allocate the XieImageRec and fill in the non-Udp fields.
    */
    XieImage img = (XieImage) XieCalloc(1,sizeof(XieImageRec));

    CmpOrg_(img)    = cmp_org;
    CmpMap_(img)    = cmp_map;
    CmpCnt_(img)    = cmp_cnt;
    for( levels = 1, pxl_bits = 0, i = 0; i < cmp_cnt; i++ )
	{
	CmpLvl_(img,i) = cmp_lvl[i];
	CmpLen_(img,i) = BitsFromLevels_(cmp_lvl[i]);
	levels	      *= CmpLvl_(img,i);
	pxl_bits      += CmpLen_(img,i);
	}
    Cmpres_(img)    = cmpres;
    PxlPol_(img)    = cmp_map == XieK_Bitonal ? XieK_ZeroBright : XieK_ZeroDark;
    PxlRatio_(img)  = 1.0;
    PxlProg_(img)   = XieK_PP0;
    ScnProg_(img)   = XieK_LP270;
    OwnData_(img)   = False;

    /*
    **	Allocate UdpRec's and fill in component organization based fields.
    */
    switch( cmp_org )
	{
    case XieK_BandByPixel :
	planes = 1;
	Plane_(img,0)   = (UdpPtr) XieMalloc(sizeof(UdpRec));
	uCmpIdx_(img,0) = 0;
	uPxlLen_(img,0) = pxl_bits;
	uLvl_(img,0)	= levels;
	break;
    case XieK_BandByPlane :
	planes = CmpCnt_(img);
	for( i = 0; i < planes; i++ )
	    {
	    Plane_(img,i)   = (UdpPtr) XieMalloc(sizeof(UdpRec));
	    uCmpIdx_(img,i) = i;
	    uPxlLen_(img,i) = CmpLen_(img,i);
	    uLvl_(img,i)    = CmpLvl_(img,i);
	    }
	break;
    case XieK_BitByPlane :
	planes = pxl_bits;
	for( i = 0; i < planes; i++ )
	    {
	    Plane_(img,i)   = (UdpPtr) XieMalloc(sizeof(UdpRec));
	    uCmpIdx_(img,i) = i < CmpLen_(img,0) ? 0
			    : i < CmpLen_(img,0) + CmpLen_(img,1) ? 1 : 2;
	    uPxlLen_(img,i) = 1;
	    uLvl_(img,i)    = CmpLvl_(img,uCmpIdx_(img,i)) < 2
			    ? CmpLvl_(img,uCmpIdx_(img,i)) : 2;
	    }
	break;
    default : _XieLibError(EINVAL, "XieCreateImage (component organization)");
	}

    /*
    **	Complete filling in UdpRec's based on per-plane information.
    */
    for( i = 0; i < planes; i++ )
	{
	if( cmpres == XieK_PCM )
	    switch( uPxlLen_(img,i) )
		{
	    case 8 :
		uDType_(img,i) = UdpK_DTypeBU;
		uClass_(img,i) = UdpK_ClassA;
		break;
	    case 16:
		uDType_(img,i) = UdpK_DTypeWU;
		uClass_(img,i) = UdpK_ClassA;
		break;
	    case 32 :
		uDType_(img,i) = UdpK_DTypeLU;
		uClass_(img,i) = UdpK_ClassA;
		break;
	    default :
		uDType_(img,i) = UdpK_DTypeVU;
		uClass_(img,i) = UdpK_ClassUBA;
		break;
		}
	else
		{
		uDType_(img,i) = UdpK_DTypeV;
		uClass_(img,i) = UdpK_ClassUBS;
		}
	uPxlStr_(img,i)	= Modulo_( uPxlLen_(img,i), pxl_mod );
	uScnStr_(img,i)	= Modulo_( uPxlStr_(img,i) * width, scn_mod );
	uX1_(img,i)	= 0;
	uX2_(img,i)	= width  - 1;
	uY1_(img,i)	= 0;
	uY2_(img,i)	= height - 1;
	uWidth_(img,i)  = width;
	uHeight_(img,i) = height;
	uArSize_(img,i) = uScnStr_(img,i) * height;
	uBase_(img,i)	= NULL;
	uPos_(img,i)	= 0;
	}

    return( img );
}				    /* end XieCreateImage */

/*******************************************************************************
**  XieFreeImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free XieImageRec, including plane descriptors and image data (if owned).
**
**  FORMAL PARAMETERS:
**
**	img	- pointer to XieImageRec
**
**  FUNCTION VALUE:
**
**	NULL
**
*******************************************************************************/
XieImage  XieFreeImage( img )
 XieImage img;
{
    CARD32 i, planes;

    if( img != NULL )
	{
	/*
	**  Free all the image data planes.
	*/
	for( planes = PlaneCount_(img), i = 0; i < planes; i++ )
	    if( Plane_(img,i) != NULL )
		{
		if( OwnData_(img) )
		    XieFreeBits( uBase_(img,i) );
		XieFree( Plane_(img,i) );
		}

	if( Xport_(img) != NULL )
	    XieFree( Xport_(img) );

	/*
	**  Free the XieImageRec.
	*/
	XieFree( img );
	}
    return( NULL );
}				    /* end XieFreeImage */

/*******************************************************************************
**  XieGetImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create an XieImage from a server Photomap: with optional image data.
**
**  FORMAL PARAMETERS:
**
**	pho_id	- Photo{flo|map|tap} id
**	cmpres	- compression scheme
**	cmpprm	- compression parameter
**	cmp_org	- component organization
**	pxl_mod	- pixel stride modulo
**	scn_mod	- scanline stride modulo
**	mode	- what to do about image data:
**		    XieK_DataNone  - no image buffers, no data copy
**		    XieK_DataAlloc - allocate new image buffers only, no data
**		    XieK_DataCopy  - setup GetStream transport and attempt to
**				     retrieve data into new image buffers
**
**  FUNCTION VALUE:
**
**	XieImage - pointer to XieImageRec
**
*******************************************************************************/
XieImage XieGetImage( pho_id, cmpres, cmpprm, cmp_org, pxl_mod, scn_mod, mode )
 XiePhoto      pho_id;
 unsigned char cmpres, cmp_org, pxl_mod, scn_mod, mode;
 unsigned long cmpprm;
{
    PhotoPtr pho = (PhotoPtr) pho_id;
    Display *dpy =  ResDpy_(pho);
    xieQueryPhotomapReply reply;
    XieImage img;
    CARD32   plane, planes;
    unsigned long *levels, long_levels[XieK_MaxComponents];
    /*
    **	Get the attributes from of the server's Photomap.
    */
    if( !_XieQueryResource( pho, &reply ) )
	return( NULL );
    UnlockDisplay(dpy);
    SyncHandle();
    /*
    **	Copy the levels data into an unsigned long array, if necessary,
    **  because XieCreateImage() requires one.
    */
    if (sizeof (CARD32) != sizeof (unsigned long))
	{
	int i;
	CARD32 *card32_levels;

	for (i = 0, levels = long_levels, card32_levels = reply.levels;
	     i < XieK_MaxComponents;
	     i++)
	    {
	    *levels++ = (unsigned long) *card32_levels++;
	    }
	levels = long_levels;
	}
    else
	levels = (unsigned long *)reply.levels;
    /*
    **	Create an XieImage from the Photomap attributes and arguments passed in.
    */
    img = XieCreateImage(
			(unsigned long)reply.width, 
			(unsigned long)reply.height, 
			cmpres,  cmp_org,
			reply.component_mapping,   reply.component_count,
			levels,
			pxl_mod, scn_mod );

    PxlPol_(img)    = reply.polarity;
    PxlRatio_(img)  = MiDecodeDouble( &reply.aspect_ratio );
    PxlProg_(img)   = reply.pixel_progression;
    ScnProg_(img)   = reply.line_progression;

    planes = PlaneCount_(img);
    /*
    **	Handle the image data according to the mode passed in.
    */
    switch( mode )
	{
    case XieK_DataAlloc :
	/*
	**  Allocate zeroed image data planes.
	*/
	for( plane = 0; plane < planes; plane++ )
	    uBase_(img,plane) = XieCallocBits( uArSize_(img,plane) );
	OwnData_(img) = True;
	break;

    case XieK_DataNone :
	break;

    case XieK_DataCopy :
	/*
	**  Setup GetStream image transport and attempt to retrieve the data.
	*/
	CreateXport(pho, img, XieK_GetStream, cmpprm);
	XieGetImageData( img );
	break;

    default : _XieLibError(EINVAL, "XieGetImage (mode)");
	}
    return( img );
}				    /* end XieGetImage */

/*******************************************************************************
**  XieGetImageData
**
**  FUNCTIONAL DESCRIPTION:
**
**	Transport all planes of image data from server Photo{flo|map|tap}.
**			      {Xport context is created by XieGetImage()}
**  FORMAL PARAMETERS:
**
**	img	- pointer to XieImageRec
**
**  FUNCTION VALUE:
**
**	GetStream status: Final, Empty, or Error
**
*******************************************************************************/
unsigned char XieGetImageData( img )
 XieImage   img;
{
    XieStreamPtr xp = (XieStreamPtr) Xport_(img);
    XiePhoto pho_id = xp ? XieFindResource(xpDpy_(xp), xpId_(xp)) : NULL;
    CARD32   plane, getb, retb, planes = PlaneCount_(img);
    CARD8    *base, state;

    if( xp == NULL || pho_id == NULL )
	return( XieK_StreamError );
    for( Stream_(img) = XieK_StreamFinal, plane = 0;
		    !StreamError_(img) && plane < planes; plane++ )
	{
	if( xpBits_(xp,plane) >= uArSize_(img,plane) )
	    continue;
	if( uBase_(img,plane) == NULL )
	    {
	    uBase_(img,plane)   = (CARD8 *) XieMallocBits(uArSize_(img,plane));
	    OwnData_(img)       = True;
	    }
	base  =   uBase_(img,plane) + xpBytes_(xp,plane);
	getb  = uArSize_(img,plane) -  xpBits_(xp,plane) + 7 >> 3;
	state = XieGetStream( pho_id, plane, &base, getb, &retb );

	xpBytes_(xp,plane) += retb;
	if( state == XieK_StreamFinal && retb < getb )
	    {
	    uArSize_(img,plane) = xpBits_(xp,plane);
	    uBase_(img,plane)   = XieReallocBits( uBase_(img,plane),
						uArSize_(img,plane));
	    }
	Stream_(img) = max(Stream_(img), state);
	}
    if( !StreamPending_(img) )
	Xport_(img) = (XieStream) XieFree( Xport_(img) );

    return( Stream_(img) );
}				    /* end XieGetImageData */

/*******************************************************************************
**  XiePutImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a server Photomap, and optionally transport data to it.
**
**  FORMAL PARAMETERS:
**
**	display	- X11 display pointer
**	img	- pointer to XieImageRec
**	mode	- what to do about image data:
**		    XieK_DataFlo    - create Photoflo + SetStream for put data
**		    XieK_DataMap    - create Photomap + SetStream for put data
**		    XieK_DataNone   - create Photomap, don't send image data
**		    XieK_DataCopy   - create Photomap, then  send image data
**
**  FUNCTION VALUE:
**
**	XiePhoto - newly created Photo{flo|map} id
**
*******************************************************************************/
XiePhoto XiePutImage( display, img, mode )
 Display       *display;
 XieImage	img;
 unsigned char	mode;
{
    /*
    **	Create a server XiePhoto{flo|map}.
    */
    XiePhoto pho = XieCreatePhoto(display, mode == XieK_DataFlo ? XieK_Photoflo
								: XieK_Photomap,
				  Width_(img),   Height_(img),
				  CmpMap_(img),  CmpCnt_(img), &CmpLvl_(img,0),
				  PxlPol_(img),  PxlRatio_(img),
				  PxlProg_(img), ScnProg_(img) );
    switch( mode )
	{
    case XieK_DataFlo :
    case XieK_DataMap :
	CreateXport(pho, img, XieK_PutStream, 0);
    case XieK_DataNone :
	break;

    case XieK_DataCopy :
	CreateXport(pho, img, XieK_PutStream, 0);
	XiePutImageData( img );
	break;

    default : _XieLibError(EINVAL, "XiePutImage (mode)");
	}
    return( pho );
}				    /* end XiePutImage */

/*******************************************************************************
**  XiePutImageData
**
**  FUNCTIONAL DESCRIPTION:
**
**	Transport all planes of image data to server Photo{flo|map}.
**			{Xport context is created by XiePutImage()}
**
**  FORMAL PARAMETERS:
**
**	img	- pointer to XieImageRec
**
**  FUNCTION VALUE:
**
**	GetStream status: Final or More
**
*******************************************************************************/
unsigned char XiePutImageData( img )
 XieImage   img;
{
    XieStreamPtr   xp = (XieStreamPtr) Xport_(img);
    PhotoPtr pho = xp ? (PhotoPtr) XieFindResource(xpDpy_(xp),xpId_(xp)) : NULL;
    CARD32   done, size, bytes, plane, planes = PlaneCount_(img);

    Stream_(img) = XieK_StreamFinal;
    if( xp == NULL || pho == NULL )
	return( XieK_StreamError );
    /*
    **	If we have a Photoflo we only send one "max_request_size" chunck of
    **	planes, otherwise we send the entire image a plane at a time.
    */
    size = FloLnk_(pho) == NULL ? ~0
				: (((xpDpy_(xp)->max_request_size < 65536 ?
				     xpDpy_(xp)->max_request_size : 65536)<<2)
						/ planes - sz_xiePutStreamReq);
    for( plane = 0; plane < planes; plane++ )
	{
	if( xpBits_(xp,plane) >= uArSize_(img,plane) )
	    continue;
	bytes = uArSize_(img,plane) - xpBits_(xp,plane) + 7 >> 3;
	done  = bytes <= size;	    
	if( !done )
	    {
	    Stream_(img) = XieK_StreamMore;
	    bytes = size;
	    }
	XiePutStream(pho,plane,uBase_(img,plane)+xpBytes_(xp,plane),bytes,done);
	xpBytes_(xp,plane) += bytes;
	}
    if( !StreamPending_(img) )
	Xport_(img) = (XieStream) XieFree( Xport_(img) );

    return( Stream_(img) );
}				    /* end XiePutImageData */

/*******************************************************************************
**  CreateXport
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a stream transport context for XieImage and server.
**
**  FORMAL PARAMETERS:
**
**	pho	- Photo{flo|map|tap} pointer
**	img	- XieImageRec pointer
**	mode	- XieK_GetStream or XieK_PutStream
**	cmpprm	- compression parameter
**
*******************************************************************************/
static void CreateXport( pho, img, mode, cmpprm )
 PhotoPtr	pho;
 XieImage	img;
 unsigned char	mode;
 unsigned long	cmpprm;
{
    XieStreamPtr xp;

    /*
    **	Create a stream transport context for the XieImage.
    */
    xp		 = (XieStreamPtr) XieCalloc(1, sizeof(XieStreamRec));
    xpId_(xp)	 =  ResId_(pho);
    xpDpy_(xp)	 =  ResDpy_(pho);
    Stream_(img) =  XieK_StreamMore;
    Xport_(img)  = (XieStream) xp;

    /*
    **	Create a stream transport context for the server.
    */
    _XieSetTransport( pho, img, mode, (1<<PlaneCount_(img))-1, cmpprm );

}				    /* end CreateXport */
/* end module XieLibImage.c */
