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
**      This module contains XIE library image transport routines.
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
#define _XieLibTransport

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include    <stdio.h>
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
void		XieAbortStream();   /* abort outstanding transport requests */
unsigned char	XieGetStream();	    /* image bit stream: server to client   */
void		XiePutStream();	    /* image bit stream: client to server   */
void		XieSetStream();	    /* send stream mode transport parameters*/
void		XieGetTile();	    /* image tile: server to client 	    */
void		XiePutTile();	    /* image tile: client to server	    */
    /*
    **  PRIVATE entry points
    */
void	       _XieSetTransport();  /* send SetTransport request	    */
    /*
    **  internal routines
    */
static void	PutPlane();	    /* segment an image plane tile	    */
static void	PutTile();	    /* send an image tile segment to server */

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
**  XieAbortStream
**
**  FUNCTIONAL DESCRIPTION:
**
**	Release server transport resources.
**
**  FORMAL PARAMETERS:
**
**	pho_id	- Photo{flo|map} id
**	p_mask	- plane mask of planes to release transport resources from
**
*******************************************************************************/
void XieAbortStream( pho_id, p_mask )
 XiePhoto   	pho_id;
 unsigned long	p_mask;
{
    PhotoPtr       pho = (PhotoPtr) pho_id;
    Display       *dpy =  ResDpy_(pho);
    XieSessionPtr  ses = _XieGetSession(dpy);
    xieAbortTransportReq *req;

    /*
    **	Create request to abort transport to/from the Photomap.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieAbortTransport,req);
    req->photo_id   = ResId_(pho);
    req->plane_mask = min(p_mask, 1<<XieK_MaxPlanes);
    XieReqDone_(dpy);
}				    /* end XieAbortStream */

/*******************************************************************************
**  XieGetStream
**
**  FUNCTIONAL DESCRIPTION:
**
**	Transport a stream of image data from Photomap to client.  XieGetStream
**	calls itself recursively until 'getbytes' or 'status' is satisfied.
**
**  FORMAL PARAMETERS:
**
**	pho_id	    - Photo{flo|map|tap} id
**	plane	    - plane number that data is to be retrieved for
**	ptr_ptr	    - pointer to address of buffer to receive image data,
**		      if address of buffer is NULL a buffer is allocated.
**	getbytes    - maximum number of bytes of data to be received
**	retbytes    - pointer: where to store actual number of bytes received
**
**  FUNCTION VALUE:
**
**	server status: empty, more, final, or error
**
*******************************************************************************/
unsigned char XieGetStream( pho_id, plane, ptr_ptr, getbytes, retbytes )
 XiePhoto   	 pho_id;
 unsigned char **ptr_ptr;
 CARD32		 plane, getbytes, *retbytes;
{
    PhotoPtr      pho = (PhotoPtr) pho_id;
    Display      *dpy =  ResDpy_(pho);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieGetStreamReply    rep;
    xieGetStreamReq     *req;
    CARD8        *ptr = *ptr_ptr;
    CARD32	 more = 0;
    /*
    **	Request the image data.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieGetStream,req);
    req->photo_id   = ResId_(pho);
    req->max_bytes  = getbytes;
    req->plane_num  = plane;

    if( !_XReply(dpy, (xReply *) &rep, 0, False) || rep.byte_count > getbytes )
	{
	if( retbytes != NULL )
	   *retbytes  = 0;
	_XieSrvError( dpy, req->opcode, ResId_(pho), rep.sequenceNumber,
		      BadLength, "GetStream: reply error" );
	return( XieK_StreamError );
	}

    if( rep.byte_count > 0 )
	{   /*
	    **  Allocate a buffer (if none supplied), and read the image data.
	    */
	if( *ptr_ptr == NULL )
	    *ptr_ptr  = XieMallocBits( rep.byte_count << 3 );

	_XReadPad( dpy, *ptr_ptr, rep.byte_count );
	}
    XieReqDone_(dpy);

    if( rep.status == XieK_StreamMore  &&  rep.byte_count < getbytes )
	{
	if( ptr != NULL )
	    /*
	    **  The application supplied a buffer -- offset to next segment.
	    */
	    ptr += rep.byte_count;

	/*
	**  Re-call XieGetStream to get the remainder of what was requested.
	*/
	rep.status = XieGetStream(pho,plane,&ptr,getbytes-rep.byte_count,&more);

	if( ptr != *ptr_ptr + rep.byte_count )
	    {
	    /*
	    **	Since we allocated the buffer we must now reallocate it and
	    **  append the remainder of the data to what we already have.
	    */
	   *ptr_ptr = XieReallocBits( *ptr_ptr, rep.byte_count + more << 3 );
	    memcpy( *ptr_ptr + rep.byte_count, ptr, more );
	    XieFreeBits( ptr );
	    }
	}

    if( retbytes != NULL )
	/*
	**  Report back the total number of bytes retrieved.
	*/
       *retbytes  = rep.byte_count + more;

    return( rep.status );
}				    /* end XieGetStream */

/*******************************************************************************
**  XiePutStream
**
**  FUNCTIONAL DESCRIPTION:
**
**	Transport a stream of image data from client to Photomap.
**
**  FORMAL PARAMETERS:
**
**	pho_id	- Photo{flo|map} id
**	plane	- plane number that data is being sent from
**	buffer	- address of buffer to send image data from
**	bytes	- number of bytes of data to send
**	final	- Boolean - true when final segment of image is to be sent
**
*******************************************************************************/
void XiePutStream( pho_id, plane, buffer, bytes, final )
 XiePhoto   	 pho_id;
 unsigned char	*buffer, final;
 unsigned long	 plane, bytes;
{
    PhotoPtr          pho = (PhotoPtr) pho_id;
    Display          *dpy =  ResDpy_(pho);
    XieSessionPtr     ses = _XieGetSession(dpy);
    xiePutStreamReq  *req;
    CARD32 remaining, size, left = ((dpy->max_request_size < 65536 ?
				     dpy->max_request_size : 65536) << 2)
				   - sz_xiePutStreamReq;
    /*
    **	Send the image data -- segmented as necessary.
    */
    for( remaining = bytes; remaining > 0; remaining -= size )
	{
	size = remaining <= left ? remaining : left;
	/*
	**  Create request to send image data in byte stream format.
	*/
	XieReq_(dpy,0,SesOpCode_(ses),iePutStream,req);
	req->length	+= size + 3 >> 2;
	req->photo_id	 = ResId_(pho);
	req->byte_count	 = size;
	req->final	 = size == remaining ? final : False;
	req->plane_num   = plane;
	_XSend( dpy, buffer + bytes - remaining, size );
	XieReqDone_(dpy);
	}
}				    /* end XiePutStream */

/*******************************************************************************
**  XieSetStream
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set server transport parameters for stream mode Get or Put.
**
**  FORMAL PARAMETERS:
**
**	pho_id	- Photo{flo|map|tap} id to set parameters for
**	plane	- plane number to set up transport parameters for
**	mode	- transport mode, XieK_GetStream or XieK_PutStream
**	cmpres	- compression scheme of image data
**	cmpprm	- compression parameter for image data
**	cmporg	- component organization of image data
**	pxlstr	- pixel stride of image data
**	scnstr	- scanline stride of image data
**
*******************************************************************************/
void XieSetStream( pho_id, plane, mode, cmpres, cmpprm, cmporg, pxlstr, scnstr )
 XiePhoto   	pho_id;
 unsigned char	plane,  mode,   cmpres, cmporg;
 unsigned long	cmpprm, pxlstr, scnstr;
{
    PhotoPtr      pho = (PhotoPtr) pho_id;
    Display      *dpy =  ResDpy_(pho);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieSetTransportReq  *req;
    CARD32 p;

    /*
    **	Create request to set server transport parameters to/from Photomap.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieSetTransport,req);
    req->photo_id		= ResId_(pho);
    req->plane_mask		= 1 << min(plane, XieK_MaxPlanes);
    req->compression_scheme	= cmpres;
    req->compression_parameter	= cmpprm;
    req->mode			= mode;
    req->component_organization	= cmporg;
    for( p = 0; p < XieK_MaxPlanes; p++ )
	{
	req->scanline_stride[p] = p == plane ? scnstr : 0;
	req->pixel_stride[p]    = p == plane ? pxlstr : 0;
	}
    XieReqDone_(dpy);
}				    /* end XieSetStream */

/*******************************************************************************
**  XieGetTile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Transport a tile of uncompressed image data from Photomap to client.
**
**  FORMAL PARAMETERS:
**
**	phm_id	- Photomap id
**	img	- pointer to XieImage structure
**	p_mask	- mask specifying planes to retrieve tiles from
**	src_x	- src X coordinate of tile (within Photomap)
**	src_y	- src Y coordinate of tile (within Photomap)
**	dst_x	- dst X coordinate of tile (within XieImage)
**	dst_y	- dst Y coordinate of tile (within XieImage)
**	width	- width  of tile
**	height	- height of tile
**
*******************************************************************************/
void XieGetTile( phm_id, img, p_mask, src_x, src_y, dst_x, dst_y, width, height)
 XiePhotomap	 phm_id;
 XieImage	 img;
 unsigned long	 p_mask, width, height;
 long	    	 src_x, src_y, dst_x, dst_y;
{
    PhotoPtr      phm = (PhotoPtr) phm_id;
    Display      *dpy =  ResDpy_(phm);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieGetTileReply      rep;
    xieGetTileReq       *req;
    unsigned char       *data;
    CARD32 msk, plane_mask = 0, planes = PlaneCount_(img);
    CARD32 i,   longs, bytes[XieK_MaxPlanes];
    INT32  sx, sy, dx, dy, sw, dw, sh, dh, w, h;
    /*
    **	Make sure the coordinates and dimensions are within reasonable limits.
    */
    sx = src_x < 0 ? 0 : src_x;
    sy = src_y < 0 ? 0 : src_y;
    sw = width + src_x > Width_(img) ?  Width_(img) - sx : width + src_x - sx;
    sh = height+ src_y > Height_(img)? Height_(img) - sy : height+ src_y - sy;
    dx = dst_x < 0 ? 0 : dst_x;
    dy = dst_y < 0 ? 0 : dst_y;
    dw = width + dst_x > Width_(img) ?  Width_(img) - dx : width + dst_x - dx;
    dh = height+ dst_y > Height_(img)? Height_(img) - dy : height+ dst_y - dy;
    w  = sw < dw ? sw : dw;
    h  = sh < dh ? sh : dh;

    if( w > 0  &&  h > 0  &&  planes <= XieK_MaxPlanes )
	/*
	**  Calculate the number of full bytes to be retrieved for each plane.
	*/
	for( longs = 0, msk = 1, i = 0; i < planes; msk <<= 1, i++ )
	    if( p_mask & msk  &&  Plane_(img,i) != NULL )
		{
		bytes[i]    = uPxlStr_(img,i) * w * h + 7 >> 3;
		longs      += bytes[i] + 3 >> 2;
		plane_mask |= msk;
		}
    if( !plane_mask )
	return;
    /*
    **	Send the transport parameters.
    */
    _XieSetTransport( phm, img, XieK_Tile, plane_mask, 0 );
    /*
    **	Request the tile of image data for all the specified planes.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieGetTile,req);
    req->photomap_id = ResId_(phm);
    req->x	     = sx;
    req->y	     = sy;
    req->width	     = w;
    req->height	     = h;
    req->plane_mask  = plane_mask;
    req->final	     = True;

    if( !_XReply(dpy, (xReply *) &rep, 0, False)  ||  rep.length != longs )
	{
	_XieSrvError( dpy, req->opcode, ResId_(phm), rep.sequenceNumber,
		      BadLength, "GetTile: reply error");
	return;
	}
    for( i = 0; plane_mask; i++, plane_mask >>= 1 )
	if( plane_mask & 1 )
	    if( dx != 0  ||  uPxlStr_(img,i) * w != uScnStr_(img,i)
			 ||  bytes[i] < uScnStr_(img,i) * Height_(img) + 7 >> 3
			 && (bytes[i] & 7  ||  uPos_(img,i) & 7) )
		{   /*
		    **  Transfer data from wire to scratch buffer to Udp.
		    */
		data = (unsigned char *) _XAllocScratch( dpy, bytes[i]+4 );
		_XReadPad( dpy, data, bytes[i] );
		_XieBufToUdp( data, Plane_(img,i), dx, dy, w, h );
		}
	    else    /*
		    **  Read the data directly into the Udp buffer.
		    */
		_XReadPad(dpy, uBase_(img,i)
			      +(uPos_(img,i)+uScnStr_(img,i)*dy>>3), bytes[i]);
    XieReqDone_(dpy);
}				    /* end XieGetTile */

/*******************************************************************************
**  XiePutTile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Transport a tile of uncompressed image data from client to Photomap.
**
**  FORMAL PARAMETERS:
**
**	phm_id	- Photomap id
**	img	- pointer to XieImage structure
**	p_mask	- mask specifying planes to retrieve tiles from
**	src_x	- src X coordinate of tile (within XieImage)
**	src_y	- src Y coordinate of tile (within XieImage)
**	dst_x	- dst X coordinate of tile (within Photomap)
**	dst_y	- dst Y coordinate of tile (within Photomap)
**	width	- width  of tile
**	height	- height of tile
**
*******************************************************************************/
void XiePutTile( phm_id, img, p_mask, src_x, src_y, dst_x, dst_y, width, height)
 XiePhotomap	 phm_id;
 XieImage	 img;
 unsigned long	 p_mask, width, height;
 long		 src_x, src_y, dst_x, dst_y;

{
    PhotoPtr       phm = (PhotoPtr) phm_id;
    Display       *dpy =  ResDpy_(phm);
    XieSessionPtr  ses = _XieGetSession(dpy);
    xiePutTileReq *req;
    unsigned char *data;
    INT32   sx, sy, dx, dy, sw, dw, sh, dh, w, h;
    CARD32  i, pels, lines, bytes[XieK_MaxPlanes], line[XieK_MaxPlanes];
    CARD32  msk, longs = 0, plane_mask = 0, planes = PlaneCount_(img);
    CARD32  max_longs = (dpy->max_request_size < 65536 ?
			 dpy->max_request_size : 65536)-(sz_xiePutTileReq>>2);
    /*
    **	Make sure the coordinates and dimensions are within reasonable limits.
    */
    sx = src_x < 0 ? 0 : src_x;
    sy = src_y < 0 ? 0 : src_y;
    sw = width + src_x > Width_(img) ?  Width_(img) - sx : width + src_x - sx;
    sh = height+ src_y > Height_(img)? Height_(img) - sy : height+ src_y - sy;
    dx = dst_x < 0 ? 0 : dst_x;
    dy = dst_y < 0 ? 0 : dst_y;
    dw = width + dst_x > Width_(img) ?  Width_(img) - dx : width + dst_x - dx;
    dh = height+ dst_y > Height_(img)? Height_(img) - dy : height+ dst_y - dy;
    w  = sw < dw ? sw : dw;
    h  = sh < dh ? sh : dh;

    if( w > 0  &&  h > 0  &&  planes <= XieK_MaxPlanes )
	/*
	**  Calculate the number of: bits per line, bytes per plane (rounded
	**  up to a full lonword), and the total number of longwords.
	*/
	for( msk = 1, i = 0; i < planes; msk <<= 1, i++ )
	    if( p_mask & msk  &&  Plane_(img,i) != NULL )
		{
		line[i]     = w * uPxlStr_(img,i);
		bytes[i]    = h * line[i] + 7 >> 3;
		longs      += bytes[i] + 3 >> 2;
		plane_mask |= msk;
		}
    if( !plane_mask )
	return;

    /*
    **	Send the transport parameters.
    */
    _XieSetTransport( phm, img, XieK_Tile, plane_mask, 0 );

    if( longs > max_longs )
	{   /*
	    **  Send each image plane separately, segmented as necessary.
	    */
	for( i = 0; plane_mask; i++, plane_mask >>= 1 )
	    if( plane_mask & 1 )
		PutPlane( phm, img, i, max_longs<<5, line[i],
			  sx, sy, dx, dy, w, h, plane_mask == 1 );
	}
    else
	{   /*
	    **  Send all requested planes in a single protocol message.
	    */
	XieReq_(dpy,0,SesOpCode_(ses),iePutTile,req);
	req->length	+= longs;
	req->photomap_id = ResId_(phm);
	req->x		 = dx;
	req->y		 = dy;
	req->width	 = w;
	req->height	 = h;
	req->plane_mask	 = plane_mask;
	req->final	 = True;

	for( i = 0; plane_mask != 0; i++, plane_mask >>= 1 )
	    if( plane_mask & 1 )
		if( dx != 0  ||  uPxlStr_(img,i) * w != uScnStr_(img,i)
			     ||  bytes[i] < uScnStr_(img,i) * Height_(img)+7>>3
			     && (bytes[i] & 7  ||  uPos_(img,i) & 7) )
		    {	/*
			**  Transfer the data to a scratch buffer -- then send.
			*/
		    data = (unsigned char *) _XAllocScratch( dpy, bytes[i]+4 );
		    _XieUdpToBuf( Plane_(img,i), data, sx, sy, w, h );
		    _XSend( dpy, data, bytes[i] );
		    }
		else	/*
			**  Send the full width plane directly from the Udp.
			*/
		    _XSend(dpy, uBase_(img,i)
			      +(uPos_(img,i)+uScnStr_(img,i)*dy>>3), bytes[i]);
	XieReqDone_(dpy);
	}
}				    /* end XiePutTile */

/*******************************************************************************
**  _XieSetTransport
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set server transport parameters for tile mode Get or Put.
**
**  FORMAL PARAMETERS:
**
**	pho	- Photo{flo|map|tap} to set parameters for
**	img	- pointer to XieImage structure
**	mode	- transport mode: XieK_GetStream, XieK_PutStream, or XieK_Tile
**	p_mask	- mask specifying planes to be transported
**	cmpprm	- compression parameter (for DCT)
**
*******************************************************************************/
void _XieSetTransport( pho, img, mode, p_mask, cmpprm )
 PhotoPtr   pho;
 XieImage   img;
 CARD8	    mode;
 CARD32	    p_mask;
 CARD32	    cmpprm;
{
    Display      *dpy =  ResDpy_(pho);
    XieSessionPtr ses = _XieGetSession(dpy);
    xieSetTransportReq  *req;
    CARD32     i, msk;

    /*
    **	Create request to set server transport parameters for new Photomap.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieSetTransport,req);
    req->photo_id		= ResId_(pho);
    req->plane_mask		= min(p_mask, 1<<XieK_MaxPlanes);
    req->compression_scheme	= Cmpres_(img);
    req->compression_parameter	= cmpprm;
    req->mode			= mode;
    req->component_organization	= CmpOrg_(img);
    for( msk = 1, i = 0; i < XieK_MaxPlanes; msk <<= 1, i++ )
	{
	req->scanline_stride[i] = p_mask & msk ? uScnStr_(img,i) : 0;
	req->pixel_stride[i]    = p_mask & msk ? uPxlStr_(img,i) : 0;
	}
    XieReqDone_(dpy);
}				    /* end _XieSetTransport */

/*******************************************************************************
**  PutPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Send a tile from a plane of image data to the Photomap.  PutPlane
**	calls itself recursively until all the data has been sent.
**
**  FORMAL PARAMETERS:
**
**	phm	- Photomap
**	img	- pointer to XieImage structure
**	plane	- plane number
**	maxlen	- maximum nuymber of bits we can send
**	scnlen	- length of a scanline in bits
**	sx	- src X coordinate
**	sy	- src Y coordinate
**	dx	- dst X coordinate
**	dy	- dst Y coordinate
**	w	- width
**	h	- height
**	final	- Boolean - true when last tile is to be sent
**
*******************************************************************************/
static void PutPlane(phm, img, plane, maxlen, scnlen, sx,sy, dx,dy, w,h, final)
 PhotoPtr   phm;
 XieImage   img;
 CARD32	    plane, maxlen, scnlen, sx,sy, dx,dy, w,h;
 BOOL	    final;
{
    CARD32   pels, lines, total = scnlen * h + 7 & ~7;

    if( total <= maxlen )
	PutTile( phm, img, plane, sx, sy, dx, dy, w, h, final );

    else if( scnlen <= maxlen )
	{
	lines = maxlen / scnlen;
	PutTile( phm, img, plane, sx, sy, dx, dy, w, lines, False );
	PutPlane(phm, img, plane, maxlen, scnlen, sx, sy + lines,
						  dx, dy + lines,
						  w,  h  - lines, final);
	}
    else
	{
	pels = (maxlen << 5) / uPxlStr_(img,plane);
	PutTile( phm, img, plane, sx, sy, dx, dy, pels, 1, False );
	PutPlane(phm, img, plane, maxlen, scnlen, sx + pels, sy,
						  dx + pels, dy,
						  w  - pels, 1, h == 1 ? final
								       : False);
	}
}				    /* end PutPlane */

/*******************************************************************************
**  PutTile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Transport a tile of image data from client image to Photomap.
**
**  FORMAL PARAMETERS:
**
**	phm		- Photomap
**	img		- pointer to XieImage structure
**	plane		- number of image plane being sent
**	sx, sy, dx, dy	- src and dst coordinates
**	w, h		- width and height
**	final		- Boolean - true when last tile is to be sent
**
*******************************************************************************/
static void PutTile( phm, img, plane, sx, sy, dx, dy, w, h, final )
 PhotoPtr   phm;
 XieImage   img;
 CARD32	    plane, sx, sy, dx, dy, w, h;
 BOOL	    final;
{
    Display      *dpy =  ResDpy_(phm);
    XieSessionPtr ses = _XieGetSession(dpy);
    CARD32      bytes = uPxlStr_(img,plane) * w * h + 7 >> 3;
    unsigned char *data;
    xiePutTileReq *req;

    XieReq_(dpy,0,SesOpCode_(ses),iePutTile,req);
    req->length	    += bytes + 3 >> 2;
    req->photomap_id = ResId_(phm);
    req->x	     = dx;
    req->y	     = dy;
    req->width	     = w;
    req->height	     = h;
    req->plane_mask  = 1 << plane;
    req->final	     = final;

    if( uPos_(img,plane) & 7 || bytes != uScnStr_(img,plane)*Height_(img)+7>>3 )
	{   /*
	    **  Extract and send the tile using a scratch buffer.
	    */
	data = (unsigned char *) _XAllocScratch( dpy, bytes+4 );
	_XieUdpToBuf( Plane_(img,plane), data, sx, sy, w, h );
	_XSend( dpy, data, bytes );
	}
    else    /*
	    **  Send the full image plane directly from the Udp's buffer.
	    */
	_XSend( dpy, uBase_(img,plane)+(uPos_(img,plane)>>3), bytes );
    XieReqDone_(dpy);
}				    /* end PutTile */
/* end module XieLibTransport.c */
