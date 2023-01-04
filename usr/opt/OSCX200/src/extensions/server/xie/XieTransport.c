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

*******************************************************************************/

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	This Xie module consists of DIX procedures for Transport 
**	service requests.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      September 27, 1989
**
*******************************************************************************/

/*
**  Definition to let include files know who's calling.
*/
#define _XieTransport

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include <stdio.h>
    /*
    **  Core X Includes
    */
#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <dixstruct.h>
#include <os.h>
    /*
    **  XIE Includes
    */
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieProto.h>
#include <XieServer.h>
#include <XieTransport.h>
#include <XieMacros.h>
#include <XiePipeInterface.h>


/*
**  Table of contents
*/
    /*
    **  Xie protocol proceedures called from XieMain
    */
int		ProcAbortTransport();
int		ProcGetStream();
int		ProcGetTile();
int		ProcPutStream();
int		ProcPutTile();
int		ProcSetTransport();
    /*
    **	routines referenced by other modules.
    */
void		FreeTransport();
int		StreamCreate();
int		xieStreamInit();
int		StreamPending();
    /*
    **  routines used internally by XieTransport
    */
static void	    AbortXport();
static void	    FreeData();
static int	    LookupTransport();
static int	    PlaneFromSink();
static int	    PlaneToSink();
static int	    PutScanlines();
static int	    PutStreamCmp();
static int	    PutStreamPCM();
static void	    SetupXportUdp();
    /*
    **  routines used to support pipelines.
    */
static PipeSinkPtr _CreateDecode();
static int 	   _GetStreamCreate();
static int 	   _GetStreamInitialize();
static int 	   _GetStreamActivate();
static int 	   _GetStreamDestroy();
static int 	   _GetStreamConvert();
static int 	   _GetStreamFreeData();
static int	   _PutCmp();
static int	   _PutDataPCM();
static int	   _PutNextPCM();

/*
**  MACRO definitions
*/
#define LookupTransport_(pflo,pmap,xport,src_id,mode,set)\
        {int status = LookupTransport(client,pflo,pmap,xport,src_id,mode,set);\
         if( status ) return status;}

/*
**  External References
*/
extern int		 FloDone();	    /* XieResource.c		    */
extern UdpPtr		 UtilAllocUdp();    /* XieUtils.c		    */
extern int		 UtilBePCM();	    /* XieUtils.c		    */
extern PipeSinkPtr	 UtilCreateSink();  /* XieUtils.c		    */
extern CARD8		 UtilClass();	    /* XieUtils.c		    */
extern CARD8		 UtilDType();	    /* XieUtils.c		    */
extern void		 UtilSetPending();  /* XieUtils.c		    */

extern CARD32		 xieClients;	    /* num of clients served	    */
extern CARD32		 xieEventBase;	    /* Base for Xie events          */
extern CARD32		 xieReqCode;	    /* XIE main opcode		    */
#if   defined(X11R3) && !defined(DWV3)
extern CARD16		 RC_xie;	    /* XIE Class		    */
extern CARD16		 RT_photo;	    /* Photo{flo|map|tap} resource  */
extern CARD16		 RT_idc;	    /* Cpp, Roi & Tmp resource types*/
#else /* X11R4 || DWV3 */
extern RESTYPE		 RC_xie;	    /* XIE Class		    */
extern RESTYPE		 RT_photo;	    /* Photo{flo|map|tap} resource  */
extern RESTYPE		 RT_idc;	    /* Cpp, Roi & Tmp resource types*/
#endif

externalref int (*xieDixSwapReply[])();	    /* Swapped client reply table   */

/*
**  Local Storage
*/
    /*
    **  GetStream Element Vector
    */
static PipeElementVector GetStreamPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeGetStreamElement,		/* Structure subtype		    */
    sizeof(GetStreamPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* reserved flags		    */
    _GetStreamInitialize,		/* Initialize entry		    */
    _GetStreamActivate,			/* Activate entry		    */
    _GetStreamFreeData,			/* Flush entry			    */
    _GetStreamDestroy,			/* Destroy entry		    */
    _GetStreamFreeData			/* Abort entry point		    */
    };

/*-----------------------------------------------------------------------
-------------------------- Abort Transport Procedure --------------------
------------------------------------------------------------------------*/
int ProcAbortTransport( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr img;
    XportPtr	 xp;
    INT32    status = Success;
    REQUEST( xieAbortTransportReq );
    REQUEST_SIZE_MATCH( xieAbortTransportReq );
    /* 
    **	Lookup Photomap id.
    */
    LookupTransport_(&flo, &img, &xp, stuff->photo_id, XieK_PutStream, -1);

    if( flo && ResId_(flo) == stuff->photo_id )
	{   /*
	    **	Abort transport for ephemeral Phototaps (ie. SrcMap & DstMap).
	    */
	if( ResId_(SrcMap_(flo)) == 0 )
	    AbortXport( SrcMap_(flo), XieK_AllPlanes );

	if( ResId_(DstMap_(flo)) == 0 )
	    AbortXport( DstMap_(flo), XieK_AllPlanes );
	}
    else
	/*
	**  Release transport resources for all specified planes.
	*/
	AbortXport( img, stuff->plane_mask );

    if( RunningFlo_(img) )
	{
	status = StreamPending( flo );
	OkIf_( status <= Success, ResId_(flo), status );
	if( status == Success && !Yielded_(flo) )
	    status  = FloDone( client, flo );
	}
    return( status <= Success ? Success : status );
}				    /* end ProcAbortTransport */

/*-----------------------------------------------------------------------
---------------------------  Get Stream Procedure -----------------------
------------------------------------------------------------------------*/
int ProcGetStream( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo;
    PhotomapPtr img;
    XportPtr	xp;
    INT32 p, bits, bytes, s = Success;
    xieGetStreamReply rep;
    REQUEST( xieGetStreamReq );
    REQUEST_SIZE_MATCH( xieGetStreamReq );

    p = stuff->plane_num;
    LookupTransport_( &flo, &img, &xp, stuff->photo_id, XieK_GetStream, p );
    /*
    **	Move available Photo{flo|map|tap} data to the Xport Udp for this plane.
    */
    if( SnkFull_(xpSnk_(xp))  &&  !IsPointer_(xpBase_(xp,p)) )
	{
	switch( xpCmpres_(xp) )
	    {
	case XieK_DCT  :
	    /* 
	    **	Always re-encode because we don't know original xpCmpPrm_.
	    */
	    BePCM_(img, ResId_(img));
	    xpSnk_(xp) = Sink_(img);
	    s = DdxEncodeDct_( &Udp_(img,p),
				xpIsBndPxl_(xp) ? CmpCnt_(img) : 1,
				xpUdp_(xp,p), xpCmpPrm_(xp), TRUE, &bits );
	    break;

	case XieK_G42D :
	    if( IsG42D_(img) )
		{	/* we can send G42D data already in the Photomap    */
		bits	      = uArSize_(img,p);
		xpBase_(xp,p) = uBase_(img,p);
		xpPos_(xp,p)  = uPos_(img,p);
		}
	    else
		{	/* compress Photomap data into the transport Udp    */
		BePCM_(img, ResId_(img));	/* must start with PCM data */
		xpSnk_(xp) = Sink_(img);
		s = DdxEncodeG4_( Udp_(img,p), xpUdp_(xp,p), &bits );
		}
	    break;

	case XieK_PCM  :
	    BePCM_(img, ResId_(img));		/* must start with PCM data */
	    xpSnk_(xp) = Sink_(img);
	    if( xpCmpIdx_(xp,p) == p
	     && xpLvl_(xp,p)    == uLvl_(img,p)
	     && xpPxlStr_(xp,p) == uPxlStr_(img,p)
	     && xpScnStr_(xp,p) == uScnStr_(img,p) )
		*xpUdp_(xp,p) = *Udp_(img,p);   /* Xport from the Photomap  */
	    else				/* Convert from Photomap    */
		s = PlaneFromSink( xp, xpSnk_(xp), p );
	    bits = xpArSize_(xp,p);
	    break;

	case XieK_G31D :			/* not yet supported	    */
	case XieK_G32D :			/* not yet supported	    */
	default        : s = BadValue;
	    }
	OkIf_(s==Success, (AbortXport(img,XieK_AllPlanes), stuff->photo_id), s);
	xpArSize_(xp,p) = bits;
	xpFinMsk_(xp)  |= 1<<p;
	}
    s = xpFinMsk_(xp) & 1<<p ? (xpArSize_(xp,p) == 0 ? XieK_StreamError
						     : XieK_StreamFinal)
						     : XieK_StreamEmpty;
    /* 
    **	Amount to send is the lesser of how much we have or how much is wanted.
    */
    bits  = xpBase_(xp,p) ? xpArSize_(xp,p) - xpPos_(xp,p) : 0;
    bytes = min(stuff->max_bytes<<3, bits) + (s==XieK_StreamFinal ? 7 : 0 )>>3;
    rep.type		= X_Reply;
    rep.sequenceNumber	= client->sequence;
    rep.length		= bytes + 3 >> 2;
    rep.byte_count	= bytes;
    rep.status		= bytes < bits>>3 ? XieK_StreamMore : s;
    WriteXieReplyToClient( client, sizeof(xieGetStreamReply), &rep );
    if( bytes != 0 )
	WriteToClient( client, bytes, xpBase_(xp,p) + (xpPos_(xp,p)>>3) );

    if( bytes << 3 < bits )
	xpPos_(xp,p) += bytes<<3;	/* offset to next segment to send   */
    else if( IsPointer_(xpBase_(xp,p)) )
	{
	if( xpBase_(xp,p) != uBase_(img,xpCmpIdx_(xp,p)) )
	    DdxFreeBits_( xpBase_(xp,p) );
	xpBase_(xp,p)   = NULL;
	xpArSize_(xp,p) = 0;
	xpPos_(xp,p)    = 0;
	}
    if( rep.status == XieK_StreamFinal || rep.status == XieK_StreamError )
	FreeTransport(img, rep.status == XieK_StreamFinal ? p : XieK_AllPlanes);

    return( Success );
}				    /* end ProcGetStream */
 
/*-----------------------------------------------------------------------
-----------------------------  Get Tile Procedure -----------------------
------------------------------------------------------------------------*/
int ProcGetTile( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotomapPtr img;
    XportPtr	xp;
    CARD32 msk, total;
    INT32  x, y, w, h, plane, s;
    xieGetTileReply rep;
    REQUEST( xieGetTileReq );
    REQUEST_SIZE_MATCH( xieGetTileReq );
    /* 
    **	Lookup Photomap id and verify the transport context and plane mask.
    */
    LookupTransport_( NULL, &img, &xp, stuff->photomap_id,
			    XieK_Tile, stuff->plane_mask );
    /*
    **  Make sure the coordinates and dimensions are within reasonable limits.
    */
    x = max(stuff->x, 0);
    y = max(stuff->y, 0);
    w = stuff->width + stuff->x >  Width_(img) ? Width_(img)   - x
					       : stuff->width  + stuff->x - x;
    h = stuff->height+ stuff->y > Height_(img) ? Height_(img)  - y
					       : stuff->height + stuff->y - y;
    ValueOkIf_(w > 0  &&  h > 0, stuff->photomap_id);

    for( total = 0, plane = 0, msk = stuff->plane_mask; msk  != 0;
					       plane++, msk >>= 1 )
	if( msk & 1 )
	    {	/*
		**  Update the transport Udp to describe the tile of image data.
		*/
	    xpWidth_(xp,plane)	= w;
	    xpHeight_(xp,plane) = h;
	    xpX1_(xp,plane)	= x;
	    xpY1_(xp,plane)	= y;
	    xpX2_(xp,plane)	= x + w - 1;
	    xpY2_(xp,plane)	= y + h - 1;
	    xpScnStr_(xp,plane) = w * xpPxlStr_(xp,plane);
	    xpArSize_(xp,plane) = h * xpScnStr_(xp,plane);
	    /*
	    **	Get the image data from the Photomap and update the total
	    **	bytes to be sent -- each plane rounded up to a full longword.
	    */
	    s = PlaneFromSink( xp, xpSnk_(xp), plane );
	    OkIf_(s == Success, stuff->photomap_id,
		 (AbortXport(img,XieK_AllPlanes), s));
	    total += xpArSize_(xp,plane) + 31 >> 3 & ~3;
	    }
    /* 
    **	Send the reply header.
    */
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = total + 3 >> 2;
    WriteXieReplyToClient( client, sizeof(xieGetTileReply), &rep );
    /* 
    **	Send all the requested data.
    */
    for( plane = 0, msk = stuff->plane_mask; msk != 0; plane++, msk >>= 1 )
	if( msk & 1 )
	    {
	    WriteToClient(client, xpArSize_(xp,plane)- xpPos_(xp,plane)+7>>3,
				    xpBase_(xp,plane)+(xpPos_(xp,plane)>>3));
	    if( xpBase_(xp,plane) != uBase_(img,xpCmpIdx_(xp,plane)) )
		DdxFreeBits_(xpBase_(xp,plane));
	    xpBase_(xp,plane) = NULL;
	    }
    if( stuff->final )
	/* 
	**  Deallocate the transport context from this Photomap.
	*/
	FreeTransport( img, XieK_AllPlanes );

    return( Success );
}				    /* end ProcGetTile */
 
/*-----------------------------------------------------------------------
---------------------------  Put Stream Procedure -----------------------
------------------------------------------------------------------------*/
int ProcPutStream( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo;
    PhotomapPtr img;
    XportPtr	xp;
    INT32 plane, s;
    REQUEST( xiePutStreamReq );
    REQUEST_AT_LEAST_SIZE( xiePutStreamReq );

    plane = stuff->plane_num;
    LookupTransport_( &flo, &img, &xp, stuff->photo_id, XieK_PutStream, plane );

    if( stuff->final )
	/*
	**  Mark this plane done -- all its data is here now.
	*/
	UtilSetPending( img, plane, xpCmpOrg_(xp), FALSE );
    /*
    **	Move the stream of client data into the Photo{flo|map|tap}.
    */
    s = xpIsPCM_(xp) ? PutStreamPCM( client, img, xp, plane )
		     : PutStreamCmp( client, img, xp, plane );
    if( s != Success )
	AbortXport( img, XieK_AllPlanes );
    else if( stuff->final )
	{
	FreeTransport( img, plane );
	if( !AnyPending_(img)  &&  RunningFlo_(img) )
	    {	/*
		**  We're done -- see if there are any other streams pending.
		*/
	    s = StreamPending( flo );
	    OkIf_( s <= Success, ResId_(flo), s );
	    if( s == Success && !Yielded_(flo) )
		s  = FloDone( client, flo );	    /* everybody's finished */
	    }
	else
	    {
	    SnkFull_(xpSnk_(xp)) = !AnyPending_(img);
#ifdef PEZD
	    LIB$SHOW_TIMER();
	    printf("PEZD : Put Stream Final, plane = %d\n", plane);
#endif
	    }
	}
    return( s < Success ? Success : s );
}				    /* end ProcPutStream */

/*-----------------------------------------------------------------------
----------------------------  Put Tile Procedure ------------------------
------------------------------------------------------------------------*/
int ProcPutTile( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotomapPtr img;
    XportPtr	xp;
    CARD32 msk, plane;
    INT32  s, x, y, w, h;
    unsigned char *data;
    REQUEST( xiePutTileReq );
    REQUEST_AT_LEAST_SIZE( xiePutTileReq );
    /* 
    **	Lookup Photomap id and verify the transport context and plane mask.
    */
    LookupTransport_( NULL, &img, &xp, stuff->photomap_id,
			    XieK_Tile, stuff->plane_mask );
    /*
    **  Make sure the coordinates and dimensions are within reasonable limits.
    */
    x = max(stuff->x, 0);
    y = max(stuff->y, 0);
    w = stuff->width + stuff->x >  Width_(img) ? Width_(img)   - x
					       : stuff->width  + stuff->x - x;
    h = stuff->height+ stuff->y > Height_(img) ? Height_(img)  - y
					       : stuff->height + stuff->y - y;
    ValueOkIf_(w > 0  &&  h > 0, stuff->photomap_id);

    for( data = (unsigned char*) &stuff[1], plane = 0, msk = stuff->plane_mask;
				  msk != 0; plane++,   msk >>= 1 )
	if( msk & 1 )
	    {	/*
		**  Update the transport Udp to describe the tile of image data.
		*/
	    xpX1_(xp,plane)	= x;
	    xpY1_(xp,plane)	= y;
	    xpX2_(xp,plane)	= x + w - 1;
	    xpY2_(xp,plane)	= y + h - 1;
	    xpWidth_(xp,plane)	= w;
	    xpHeight_(xp,plane) = h;
	    xpScnStr_(xp,plane) = w * xpPxlStr_(xp,plane);
	    xpArSize_(xp,plane) = h * xpScnStr_(xp,plane);
	    xpBase_(xp,plane)	= data;
	    /*
	    **	Move the image data into the Photomap -- don't free "stuff".
	    */
	    s = PlaneToSink( xp, xpSnk_(xp), plane, stuff->final );
	    xpBase_(xp,plane) = NULL;
	    OkIf_( s == Success, stuff->photomap_id, s );
	    /*
	    **	Update the data pointer -- if there are more planes of
	    **	data the next plane's data starts at the next longword.
	    */
	    data += xpArSize_(xp,plane) + 31 >> 3 & ~3;
	    }
    if( stuff->final )
	/* 
	**  Deallocate transport context from this Photomap.
	*/
	FreeTransport( img, XieK_AllPlanes );

    return( Success );
}				    /* end ProcPutTile */

/*-----------------------------------------------------------------------
--------------------------- Set Transport Procedure ---------------------
------------------------------------------------------------------------*/
int ProcSetTransport( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo = NULL;
    PhotomapPtr img;
    PipeSinkPtr snk;
    XportPtr	xp;
    CARD32   c, plane, org, msk, max_planes, DType, PxlBits;
    CARD32   bpc[XieK_MaxComponents], lvl[XieK_MaxComponents];
    REQUEST( xieSetTransportReq );
    REQUEST_SIZE_MATCH( xieSetTransportReq );
    /* 
    **	Lookup Photo{flo|map} and verify the transport mode requested.
    */
    LookupTransport_( stuff->mode == XieK_Tile ? NULL : &flo,
		     &img, &xp, stuff->photo_id, stuff->mode, -1 );
    AccessOkIf_(!flo || Make_(flo)
		     || Done_(flo) && IsPhotomap_(img), stuff->photo_id);
    /*
    **	Verify the size of each component and the total bits per pixel.
    */
    for( PxlBits = 0, c = 0; c < XieK_MaxComponents; PxlBits += bpc[c++] )
	{
	bpc[c] = IsPointer_(Udp_(img,c)) ? BitsFromLevels_(uLvl_(img,c)) : 0;
	ValueOkIf_( bpc[c] <= XieK_MaxComponentDepth, stuff->photo_id );
	}
    ValueOkIf_( stuff->plane_mask > 0
	     && stuff->plane_mask <= XieK_AllPlanes, stuff->photo_id );
    /*
    **	Degenerate the specified component_organization to its simplest form.
    */
    org = IsBitonal_(img)  ? XieK_BandByPlane
	    : IsRGB_(img) || stuff->component_organization == XieK_BitByPlane
			   ? stuff->component_organization  : XieK_BandByPlane;
    if( IsPointer_(xp) )
	{   /*
	    **  Just adding more planes, make sure everything else is the same.
	    */
	MatchOkIf_((xpMode_(xp) == stuff->mode
	     &&   xpCmpOrg_(xp) == org
	     &&   xpCmpres_(xp) == stuff->compression_scheme
	     &&   xpCmpPrm_(xp) == stuff->compression_parameter
	     && !(xpPlnMsk_(xp)  & stuff->plane_mask)
	     && !(xpFinMsk_(xp)  & stuff->plane_mask)
	     &&   xpMaxMsk_(xp) >= stuff->plane_mask), stuff->photo_id);
	xpPlnMsk_(xp) |= stuff->plane_mask; /* just update the plane_mask   */
	}
    else
	{
	AccessOkIf_(stuff->mode != XieK_PutStream   /* Ok if client created */
		 || WriteMap_(img),stuff->photo_id);/* this Photo{map|tap}  */

	switch( org )
	    { /* test component_organization and number of planes requested */
	case XieK_BandByPixel : max_planes = 1;		    break;
	case XieK_BandByPlane : max_planes = CmpCnt_(img);  break;
	case XieK_BitByPlane  : max_planes = PxlBits;	    break;
	    }
	ValueOkIf_( stuff->plane_mask <= (1<<max_planes)-1, stuff->photo_id );

	switch( stuff->compression_scheme )
	    {				    /* verify compression_scheme    */
	case XieK_PCM  : break;
	case XieK_G31D :
	case XieK_G32D :
	case XieK_G42D : 
	    MatchOkIf_( IsBitonal_(img) && stuff->mode != XieK_Tile,
						  stuff->photo_id );  break;
	case XieK_DCT  :
	    MatchOkIf_(!IsBitonal_(img) && stuff->mode != XieK_Tile
		       && org != XieK_BitByPlane, stuff->photo_id );  break;
	default: BadValue_(stuff->photo_id);
	    }
	MatchOkIf_(stuff->mode != XieK_PutStream || !SnkUdpFinalMsk_(Sink_(img))
	       || Cmpres_(img) == stuff->compression_scheme, stuff->photo_id);

	xp = (XportPtr) DdxCalloc_(1,sizeof(XportRec));
	AllocOkIf_( xp != NULL, stuff->photo_id );
        Tport_(img)   = xp;
	xpMode_(xp)   = stuff->mode;
	xpCmpOrg_(xp) = org;
	xpCmpres_(xp) = stuff->compression_scheme;
	xpCmpPrm_(xp) = stuff->compression_parameter;
	xpPlnMsk_(xp) = stuff->plane_mask;
	xpMaxMsk_(xp) = (1<<max_planes)-1;
	xpFinMsk_(xp) = 0;

	if( !xpIsPutStream_(xp) )
	    UtilSetPending(img,0,XieK_BandByPixel,FALSE);   /* none pending */
	else if( Cmpres_(img) != xpCmpres_(xp) )
	    {	/* Re-init Photomap & Sink to match new compression_scheme  */
	    Cmpres_(img) = xpCmpres_(xp);
	    CmpOrg_(img) = xpIsPCM_(xp) ? XieK_BandByPlane : org;
	    for(c = 0; c < CmpCnt_(img); lvl[c] = SnkLvl_(Sink_(img),c), c++);
	    snk = UtilCreateSink( img, Cmpres_(img), lvl, FALSE, FALSE,
				  IsPCM_(img) ? DdxAlignDefault_ : NULL );
	    AllocOkIf_(snk, (AbortXport(img,XieK_AllPlanes), stuff->photo_id));
	    DdxRmDestroySink_( Sink_(img) );
	    SnkPrm_(snk) = IsPhotomap_(img);
	    Sink_(img)	 = snk;
	    }
	xpSnk_(xp) = Sink_(img);    /* remember Sink involved in transport  */
	}

    /*
    **	Set parameters for specified planes.
    */
    for( plane = 0, msk = 1;  msk & xpMaxMsk_(xp);  plane++, msk <<= 1 )
	{
	if( !(msk & stuff->plane_mask) )
	    continue;

	switch( org )
	    {
	case XieK_BandByPlane : PxlBits = bpc[plane];	break;
	case XieK_BitByPlane  : PxlBits = 1;		break;
	    }
	ValueOkIf_( !xpIsPCM_(xp)
		  || stuff->pixel_stride[plane] > 0
		  && stuff->scanline_stride[plane] > 0,
		  (AbortXport(img,XieK_AllPlanes), stuff->photo_id));

	DType = UtilDType( PxlBits, xpCmpres_(xp), stuff->pixel_stride[plane],
					        stuff->scanline_stride[plane]);
	/*
	**  Allocate a Udp for this plane (height == 0 if PutStream).
	*/
	xpUdp_(xp,plane) = UtilAllocUdp(DType, xpCmpres_(xp),	  Width_(img),
					xpIsPutStream_(xp) ? 0 : Height_(img),
					stuff->pixel_stride[plane],
					stuff->scanline_stride[plane],
					1<<PxlBits, plane, FALSE, 0, NULL,NULL);
	AllocOkIf_(xpUdp_(xp,plane),
		  (AbortXport(img,XieK_AllPlanes), stuff->photo_id));
	switch( org )
	    {	/*
		**  Determine the component index for this plane.
		*/
	case XieK_BandByPixel : xpCmpIdx_(xp,plane) = 0;     break;
	case XieK_BandByPlane : xpCmpIdx_(xp,plane) = plane; break;
	case XieK_BitByPlane  : xpCmpIdx_(xp,plane) =
					     plane  < bpc[0] ? 0
					   : plane  < bpc[0] + bpc[1] ? 1 : 2;
	    }
	if( xpIsPutStream_(xp) )
	    FreeData( img, plane, TRUE );
	}
#ifdef	PEZD
    printf("PEZD : End of Set Transport Context\n");
    LIB$INIT_TIMER();
#endif
    return( Success );
}				    /* end ProcSetTransport */

/*-----------------------------------------------------------------------
------------------------- routine:  Free Xport context ------------------
------------------------------------------------------------------------*/
void FreeTransport( img, plane )
 PhotomapPtr img;   /* Photo{map|tap} from which to free transport resources  */
 CARD32	     plane; /* Xport plane number or XieK_AllPlanes		      */
{
    XportPtr xp = Tport_(img);
    CARD32  msk, c, p;

    if( !IsPointer_(xp) )
	return;

    msk = plane == XieK_AllPlanes ? xpPlnMsk_(xp) : 1<<plane;

    for( p = 0;  msk;  p++, msk >>= 1 )
	{
	if( !(msk & 1)  ||  !IsPointer_(xpUdp_(xp,p)) )
	    continue;

	if( IsPointer_(xpBase_(xp,p))
		    && xpBase_(xp,p) != uBase_(img,xpCmpIdx_(xp,p)) )
	    DdxFreeBits_(xpBase_(xp,p));

	xpUdp_(xp,p)   =  (UdpPtr) DdxFree_(xpUdp_(xp,p));
	xpPlnMsk_(xp) &= ~(1<<p);		    /* zap plane requested  */
	xpFinMsk_(xp) &= ~(1<<p);		    /* zap plane finished   */
	}
    if( xpPlnMsk_(xp) == 0 )
	{
	for( c = 0; c < CmpCnt_(img); c++ )
	    if( IsPointer_(xpDat_(xp,c)) )	    /* free Photoflo data   */
		DdxRmDeallocData_(xpDat_(xp,c));

	Tport_(img) = (XportPtr) DdxFree_( xp );    /* free Xport context   */
	}
}				    /* end FreeTransport */

/*-----------------------------------------------------------------------
--------------- routine:  Create Decode and Encode elements -------------
------------------------------------------------------------------------*/
int StreamCreate( flo )
 PhotofloPtr	flo;		/* Photoflo pointer			    */
{
    PipeSinkPtr snk;
    PhotomapPtr	img;
    XportPtr	 xp;
    FloQuePtr   que = QueSrc_(flo);
    INT32      c, s = Success;

    do  {
	if( IsPhoto_(FloRes_(que)) )
	    {
	    img = (PhotomapPtr) FloRes_(que);
	    xp  = Tport_(img);

	    if( !IsPCM_(img) )
		if( !QueueEmpty_(SnkDrnLst_(Sink_(img))) || IsPointer_(xp)
			  && xpIsGetStream_(xp) && !SnkFull_(xpSnk_(xp)) )
		    {	/*
			**  Prepend a Decode element.
			*/
		    snk = _CreateDecode( Pipe_(flo), img );
		    if( snk == NULL ) return( BadAlloc );
		    if( IsPointer_(xp) )
			xpSnk_(xp) = snk;
		    }
		else	    /* PCM not needed, remember Sink is compressed  */
		    DdxRmSetDType_( Sink_(img), UdpK_DTypeV, DtM_V );
	    else if( IsPointer_(xp) && xpIsPutStream_(xp) )
		{   /*
		    **  Set DType choices and Quantum for PCM PutStream Sink.
		    */
		DdxRmSetDType_( xpSnk_(xp), UdpK_DTypeUndefined,
					    IsBitonal_(img) ? DtM_VU
							    : DtM_Constrained );
		DdxRmSetQuantum_( xpSnk_(xp),
				  xpIsBitPln_(xp) ? Height_(img) : 0 );
		}
	    if( IsPointer_(xp) && xpIsGetStream_(xp) && !SnkFull_(xpSnk_(xp)) )
		/*
		**  Append an Encode element.
		*/
		s = _GetStreamCreate( Pipe_(flo), img );
	    }
	que = FloNxt(que);
        }
    while( s == Success  &&  que != QueSrc_(flo) );

    return( s );
}				/* End of StreamCreate */

/*-----------------------------------------------------------------------
------------------- routine:  Initialize PutStream Sinks ----------------
------------------------------------------------------------------------*/
int xieStreamInit( flo )
 PhotofloPtr	flo;		/* Photoflo pointer			    */
{
    PhotomapPtr	img;
    XportPtr	 xp;
    FloQuePtr   que = QueSrc_(flo);
    INT32  c, p, h, y, s = Success;
    
    do  {
	if( IsPhoto_(FloRes_(que)) )
	    {
	    img = (PhotomapPtr) FloRes_(que);
	    xp  = Tport_(img);

	    if( IsPointer_(xp) && xpIsPutStream_(xp) )
		{
		s = DdxRmInitializePort_( Pipe_(flo), xpSnk_(xp) );

		if( s == Success && xpIsPCM_(xp) )
		    for( c = 0; c < CmpCnt_(img) && s == Success; c++ )
			{   /*
			    **  Look for any scanlines we may have accumulated.
			    */
			if( xpIsBitPln_(xp) && CmpPending_(img,c) )
			    continue; /* can only handle completed components */
			/*
			**  If not all here, look at xp Y1 & Y2 to see what we
			**  have, otherwise the entire component is in the Sink.
			*/
			p = xpIsBndPln_(xp) ? c : 0;
			h = CmpPending_(img,c) ? xpY1_(xp,p) : Height_(img);
			y = CmpPending_(img,c) ? xpY2_(xp,p) : uY2_(img,c);

			if( h > 0 )
			    {	    /* push "h" scanlines into the Photoflo */
			    s = _PutNextPCM( xp, xpSnk_(xp), c, 0, -h );
			    if( s == Success )
				s = _PutDataPCM( xp, xpSnk_(xp), c, y );
			    }
			}
		}
	    }
	que = FloNxt(que);
        }
    while( s == Success  &&  que != QueSrc_(flo) );

    return( s );
}				/* End of xieStreamInit */

/*-----------------------------------------------------------------------
-- routine: push pending input into Photoflo, tell if Xport is pending --
------------------------------------------------------------------------*/
int StreamPending( flo )
 PhotofloPtr	flo;	/* Photoflo pointer				    */
{
/*
**  return value:
**
**	-1 == input pending for some sink
**	 0 == Success -- no transport pending
**	>0 == error code
*/
    FloQuePtr   que = QueSrc_(flo);
    PhotomapPtr	img;
    XportPtr	 xp;
    INT32	  s = Success;
    
    do  {
	if( IsPhoto_(FloRes_(que)) )
	    {
	    img = FloMap_(que);
	    xp  = Tport_(img);
	    if( xp && xpIsPutStream_(xp) )
		s = -1;
	    else if( AnyPending_(img) )
		return( BadAccess );
	    }
	que = FloNxt(que);
        }
    while( que != QueSrc_(flo) );

    return( s );
}				/* end StreamPending */

/*-----------------------------------------------------------------------
------------------------- routine:  Abort transport ---------------------
------------------------------------------------------------------------*/
static void AbortXport( img, mask )
 PhotomapPtr img;	/* Photomap pointer				    */
 CARD32	     mask;	/* Xport plane mask				    */
{
    XportPtr xp = Tport_(img);
    CARD32 p, wait, msk;

    if( !IsPointer_(xp) )  return;
    
    for( p = 0, msk = xpPlnMsk_(xp) & mask; msk && Tport_(img); p++, msk >>= 1 )
	if( msk & 1 )
	    {
	    if( xpIsPutStream_(xp) )
		/*
		**  Free the image data if the component is totally aborted.
		*/
		FreeData( img, p, FALSE );

	    /*
	    **  Release the transport resources of this client plane.
	    */
	    FreeTransport( img, p );
	    }
}				    /* end AbortXport */

/*-----------------------------------------------------------------------
-------------- routine:  Free image Data for empty components -----------
------------------------------------------------------------------------*/
static void FreeData( img, plane, set )
 PhotomapPtr img;	/* Photomap pointer				    */
 CARD32	   plane;	/* client plane to be freed			    */
 BOOL	   set;		/* TRUE if SetTransport, FALSE if AbortTransport    */
{
    XportPtr    xp = Tport_(img);
    CARD32    c = xpCmpIdx_(xp,plane);
    CARD16 pending = CmpPending_(img,c);

    /*
    **	Remember this plane is pending.
    */
    UtilSetPending( img, plane, xpCmpOrg_(xp), TRUE );

    do  {   /*
	    **	Test for: no planes previously pending (for SetTransport),
	    **	      or  all planes pending (after AbortTransport).
	    */
	if( set ? !pending
		: CmpPending_(img,c) == (1<<BitsFromLevels_(uLvl_(img,c)))-1 )
	    {	/*
		**  Free plane's component image array(s).
		*/
	    if( IsPointer_(uBase_(img,c)) )
		uBase_(img,c) = DdxFreeBits_( uBase_(img,c) );
	    if( !IsPCM_(img) )
	       uArSize_(img,c) = 0;
	    }
	/*
	**  Remember this component is incomplete.
	*/
	SnkUdpFinalMsk_(Sink_(img)) &= ~(1<<c);

	} while( xpIsBndPxl_(xp) && ++c < CmpCnt_(img) );

    /*
    **	Reset SnkFull_ if anything is pending.
    */
    SnkFull_(Sink_(img)) = !AnyPending_(img);

}				    /* end FreeData */

/*-----------------------------------------------------------------------
-------------- Lookup/verify transport Photo{flo|map|tap}s --------------
------------------------------------------------------------------------*/
static int LookupTransport( client, flo, img, xp, src_id, mode, plane )
 ClientPtr     client;	/* client pointer, needed by LookupTransport_() macro */
 PhotofloPtr  *flo;	/* where to put PhotofloPtr: NULL if not allowed      */
 PhotomapPtr  *img;	/* where to put PhotomapPtr: NOT optional	      */
 XportPtr     *xp;	/* where to put XportPtr:    NOT optional	      */
 CARD32	       src_id;	/* X11 src resource-id:      NOT optional	      */
 CARD8	       mode;	/* Transport mode, Tile, GetStream, or PutStream      */
 INT32	       plane;	/* Set/Abort=-1, Stream=plane_number, Tile=plane_mask */
{
    REQUEST( xieReq );
    *img = (PhotomapPtr) LookupId_(src_id, RT_photo);
    IDChoiceOkIf_(*img, src_id);
    switch( ResType_(*img) )
	{
    case XieK_Photoflo :
	IDChoiceOkIf_(flo, src_id);		/* Ok if Photoflo is allowed*/
	*flo = (PhotofloPtr) *img;		/* return Photoflo pointer  */
	*img = mode == XieK_PutStream		/* return Photomap pointer  */
		     ? SrcMap_(*flo)		/* Put at start of Photoflo */
		     : DstMap_(*flo);		/* Get from end of Photoflo */
	break;
    case XieK_Photomap :
    case XieK_Phototap :
	if( flo )				/* if Photoflo allowed...   */
	   *flo = FloLnk_(*img);		/* return Photoflo pointer  */
	else					/* Ok if no Photoflo link   */
	    IDChoiceOkIf_(!FloLnk_(*img),src_id);
	break;
    default : BadIDChoice_(src_id);
	}
    AccessOkIf_(!flo || !*flo || !Aborted_(*flo), src_id);
    *xp = Tport_(*img);
    if( plane < 0  ||  !IsPointer_(*xp) )
	AccessOkIf_(plane < 0, src_id);		/* Ok if Set/Abort Transport*/
    else
	MatchOkIf_(mode==xpMode_(*xp),src_id);	/* Ok if Xport mode matches */
    switch( mode )
	{
    case XieK_GetStream :
    case XieK_PutStream :			/* check stream plane number*/
	MatchOkIf_(plane < 0 || (xpPlnMsk_(*xp) & 1<<plane), src_id);
	break;
    case XieK_Tile :
	if( IsPointer_(*xp) )			/* check tile plane mask    */
	    MatchOkIf_((xpPlnMsk_(*xp) & plane) == plane, src_id);
	else
	    BePCM_( *img, src_id );		/* must start with PCM data */
	break;
    default : BadValue_(src_id);
	}
    return( Success );
}				/* end LookupTransport */

/*-----------------------------------------------------------------------
--------------- routine:  Xport image data from Sink Udp(s) -------------
------------------------------------------------------------------------*/
static int PlaneFromSink( xp, snk, plane )
 XportPtr    xp;	/* Xport context pointer			    */
 PipeSinkPtr snk;	/* Sink pointer					    */
 CARD32	     plane;	/* Xport plane number				    */
{
    UdpRec  udp;
    CARD32 comp, s;

    switch( xpCmpOrg_(xp) )
	{
    case XieK_BandByPixel :
    case XieK_BandByPlane :
	s = DdxConvert_(SnkUdpPtr_(snk,plane), xpUdp_(xp,plane), XieK_MoveMode);

	if( xpIsBndPxl_(xp) && IsPointer_(SnkUdpPtr_(snk,1)) )
	    for( comp = 1, udp = *xpUdp_(xp,plane);
		 comp < XieK_MaxComponents && s == Success; comp++ )
		{
		SetupXportUdp( &udp, snk, xp, plane, comp );
		urPos_(udp) += BitsFromLevels_(SnkLvl_(snk,comp-1));
		s = DdxConvert_( SnkUdpPtr_(snk,comp), &udp, XieK_MoveMode );
		}
	break;

    case XieK_BitByPlane :
	SetupXportUdp( &udp, snk, xp, plane, xpCmpIdx_(xp,plane) );
	s = DdxConvert_( &udp, xpUdp_(xp,plane), XieK_MoveMode );
	break;
	}	    
    return( s );
}				    /* end PlaneFromSink */

/*-----------------------------------------------------------------------
----------------- routine:  Xport image data to Sink Udp(s) -------------
------------------------------------------------------------------------*/
static int PlaneToSink( xp, snk, p, final )
 XportPtr	xp;	    /* Xport context pointer			    */
 PipeSinkPtr	snk;	    /* Sink pointer				    */
 CARD32		p;	    /* Xport plane number			    */
 BOOL		final;	    /* TRUE if xpUdp contains final scanline	    */
{
    UdpRec  udp, *dst;
    CARD32  s, c;

    switch( xpCmpOrg_(xp) )
	{
    case XieK_BandByPixel :
	for( c = 0, udp = *xpUdp_(xp,p);
	     c < XieK_MaxComponents && SnkUdpPtr_(snk,c); c++ )
	    {
	    SnkUdpFinalMsk_(snk) |= final ? 1<<c : 0;
	    SetupXportUdp( &udp, snk, xp, p, c );
	    dst = IsPointer_(xpDat_(xp,c)) ? DatUdpPtr_(xpDat_(xp,c))
					   : SnkUdpPtr_(snk,c);
	    s = DdxConvert_( &udp, dst, XieK_MoveMode );
	    urPos_(udp) += urPxlLen_(udp);
	    if( s == Success && IsPointer_(xpDat_(xp,c)) )
		s = _PutDataPCM(xp, snk, c, xpY2_(xp,p));
	    if( s != Success ) break;
	    }
	break;
    case XieK_BandByPlane :
	SnkUdpFinalMsk_(snk) |= final ? 1<<p : 0;
	dst = IsPointer_(xpDat_(xp,p)) ? DatUdpPtr_(xpDat_(xp,p))
				       : SnkUdpPtr_(snk,p);
	s = DdxConvert_( xpUdp_(xp,p), dst, XieK_MoveMode );
	if( s == Success && IsPointer_(xpDat_(xp,p)) )
	    s = _PutDataPCM(xp, snk, p, xpY2_(xp,p));
	break;
    case XieK_BitByPlane :
	c = xpCmpIdx_(xp,p);
	SnkUdpFinalMsk_(snk) |= final ? 1<<c : 0;
	SetupXportUdp( &udp, snk, xp, p, c );
	urBase_(udp) = IsPointer_(xpDat_(xp,c)) ? DatBase_(xpDat_(xp,c))
						: SnkBase_(snk,c);
	if( !IsPointer_(urBase_(udp)) )
	    urBase_(udp) = DdxCallocBits_(urArSize_(udp));
	s = DdxConvert_( xpUdp_(xp,p), &udp, XieK_MoveMode );
	if( IsPointer_(xpDat_(xp,c)) )
	    DatBase_(xpDat_(xp,c)) = urBase_(udp);
	else
	    SnkBase_(snk,c)	   = urBase_(udp);
	if( s == Success && IsPointer_(xpDat_(xp,c)) )
	    s = _PutDataPCM(xp, snk, c, xpY2_(xp,p));
	}	    
    return( s );
}				    /* end PlaneToSink */

/*-----------------------------------------------------------------------
---------- routine:  Put Scanlines of image into Photo{flo|map} ---------
------------------------------------------------------------------------*/
static int PutScanlines( img, xp, p, final )
 PhotomapPtr	img;	/* Photomap pointer				    */
 XportPtr	xp;	/* Transport context pointer			    */
 CARD32		p;	/* Xport plane number				    */
 BOOL		final;	/* TRUE if xpUdp contains final scanline	    */
{
    PipeSinkPtr snk = xpSnk_(xp);
    INT32  c, s;

    if( RunningFlo_(img) )
	{
	switch( xpCmpOrg_(xp) )
	    {   /*
		**  Initialize the Photoflo data descriptor(s).
		*/
	case XieK_BandByPixel :
	    for( c = 0; c < CmpCnt_(img); c++ )
		{
		s = _PutNextPCM( xp, snk, c, xpY1_(xp,p), xpHeight_(xp,p) );
		if( s != Success ) break;
		}
	    break;
	case XieK_BandByPlane :
	    s = _PutNextPCM( xp, snk, p, xpY1_(xp,p), xpHeight_(xp,p) );
	    break;
	case XieK_BitByPlane  :
	    c = xpCmpIdx_(xp,p);
	    if( !final || CmpPending_(img,c) )
		/*
		**  Some scanlines or planes for this component still pending.
		*/
		s =  PlaneToSink( xp, snk, p, FALSE );
	    else
		s = _PutNextPCM( xp, snk, c, 0, -Height_(img) );
	    }	    
	while( s == Success && IsPointer_(xpDat_(xp,xpCmpIdx_(xp,p))) )
	    {	/*
		**  Push data from X11 wire buffer into the running Photoflo.
		*/
	    s = PlaneToSink( xp, snk, p, final );
	    if( s == Success )
		s  = ResumePipeline_( FloLnk_(img) );
	    }
	}
    else
	/*
	**  Copy data from X11 wire buffer into the Sink Udp(s).
	*/
	s = PlaneToSink( xp, snk, p, final );
    return( s );
}				    /* end PutScanlines */
 
/*-----------------------------------------------------------------------
--------------------- routine:  Put Stream Compressed -------------------
------------------------------------------------------------------------*/
static int PutStreamCmp( client, img, xp, p )
 ClientPtr	client;	    /* client pointer				    */
 PhotomapPtr	img;	    /* Photo{map|tap} pointer			    */
 XportPtr	xp;	    /* transport context pointer		    */
 CARD32		p;	    /* client plane number			    */
{
    INT32  c, own_data, s;
    REQUEST( xiePutStreamReq );
    /*
    **	Let the Photoflo decompress directly from the protocol wire buffer.
    **	Otherwise accumulate data until it's all here or the Photoflo starts.
    */
    xpArSize_(xp,p) += stuff->byte_count << 3;
    xpBase_(xp,p) = IsPointer_(xpBase_(xp,p))		/* get larger buffer*/
		  ? DdxReallocBits_(xpBase_(xp,p), xpArSize_(xp,p))
		  : RunningFlo_(img) && !QueueEmpty_(SnkDrnLst_(xpSnk_(xp)))
		  ? (CARD8 *)&stuff[1]			/* use wire  buffer */
		  : DdxMallocBits_(xpArSize_(xp,p));    /* get first buffer */
    AllocOkIf_( xpBase_(xp,p), stuff->photo_id );
    /*
    **	See who owns the data buffer, X11 or Xie.
    */
    own_data = xpBase_(xp,p) != (CARD8 *) &stuff[1];

    if( own_data )
	{   /*
	    **  Append the wire buffer data to our temporary buffer.
	    */
	memcpy( xpBase_(xp,p)+(xpPos_(xp,p)>>3), (CARD8 *) &stuff[1], 
						  stuff->byte_count);
	xpPos_(xp,p) += stuff->byte_count<<3;
	}
    if( stuff->final )
	SnkUdpFinalMsk_(xpSnk_(xp)) |= 1<<p;

    if( RunningFlo_(img) && !QueueEmpty_(SnkDrnLst_(xpSnk_(xp))) )
	{   /*
	    **  Push compressed data into the running Photoflo.
	    */
	s = _PutCmp( img, xp, p, own_data );
	OkIf_( s == Success, stuff->photo_id, s );
	}
    else if( stuff->final )
	{   /*
	    **  Transfer compressed data ownership to the Photomap.
	    */
	c		= xpCmpIdx_(xp,p);
	uArSize_(img,c) = xpArSize_(xp,p);
	uBase_(img,c)	= xpBase_(xp,p);
	xpBase_(xp,p)	= NULL;
	}
    return( Success );
}				    /* end PutStreamCmp */
 
/*-----------------------------------------------------------------------
------- routine:  Put Stream PCM: convert stream to scanlines -----------
------------------------------------------------------------------------*/
static int PutStreamPCM( client, img, xp, plane )
 ClientPtr	client;	    /* client pointer				    */
 PhotomapPtr	img;	    /* Photo{map|tap} pointer			    */
 XportPtr	xp;	    /* transport context pointer		    */
 CARD32		plane;	    /* client plane number			    */
{
    REQUEST( xiePutStreamReq );
    INT32  s, total = stuff->byte_count << 3;
    INT32     lines = (xpPos_(xp,plane) + total) / xpScnStr_(xp,plane);
    INT32     used  = 0;
    /*
    **	PutStreamPCM converts stream image data into scanline(s) that are
    **	copied into the waiting Photo{map|tap} or if a Photoflo is running the
    **	data is pushed directly into the running Photoflo.
    **
    **	Any bits left over (a partial scanline) are saved in the transport
    **	context Udp.  Y1 indicates which scanline the data belongs to, and Pos
    **	is used to keep the bit offset and byte count.  Since the client is
    **	required to send full bytes of data (except final) the lower 3 bits of
    **	Pos is the actual Pos (bit offset) and the upper bits are the byte 
    **	count.
    */
    lines = min(lines, uY2_(img,xpCmpIdx_(xp,plane)) - xpY2_(xp,plane));
    if( IsPointer_(xpBase_(xp,plane)) )
	{	/*
		**  Append new data to previous partial scanline.
		*/
	used = min( total, xpScnStr_(xp,plane) - xpPos_(xp,plane) );
	memcpy( xpBase_(xp,plane) + (xpPos_(xp,plane) + 7 >> 3),
			    (CARD8 *) &stuff[1], used + 7 >> 3);
	if( lines-- == 0 )
		/*
		**  Still don't have a full scanline.
		*/
	    xpPos_(xp,plane) += used;
	else
	    {	/*
		**  Copy (and convert) the single completed scanline.
		*/
	    xpHeight_(xp,plane) =  1;
	    xpY2_(xp,plane)	=  xpY1_(xp,plane);
	    xpPos_(xp,plane)	= -xpPos_(xp,plane) & 7;
	    s = PutScanlines( img, xp, plane, (stuff->final && lines == 0) );
		/*
		**  Reset our context to accept all the full scanlines.
		*/
	    xpBase_(xp,plane)   = DdxFreeBits_( xpBase_(xp,plane) );
	    xpPos_(xp,plane)    = used & 7;
	    xpY1_(xp,plane)++;
	    OkIf_( s == Success, stuff->photo_id, s );
	    }
	}

    if( lines > 0 )
	{	/*
		**  Copy (and convert) all the full scanlines.
		*/
	xpY2_(xp,plane)     = lines + xpY1_(xp,plane) - 1;
	xpHeight_(xp,plane) = lines;
	xpBase_(xp,plane)   = (CARD8 *) &stuff[1] + (used >> 3);
	s = PutScanlines( img, xp, plane, stuff->final );
		/*
		**  Reset our context to save any remaining bits.
		*/
	xpBase_(xp,plane)   = NULL;
	used		   += lines * xpScnStr_(xp,plane);
	xpY1_(xp,plane)    += lines;
	OkIf_( s == Success, stuff->photo_id, s );
	}
    if( !stuff->final  &&  used < total )
	{	/*
		**  Stash the remaining bits away until next time.
		*/
	xpPos_(xp,plane)    = total - used;
	xpBase_(xp,plane)   = DdxMallocBits_(xpScnStr_(xp,plane) + 8);
	AllocOkIf_( xpBase_(xp,plane), stuff->photo_id );
	memcpy(xpBase_(xp,plane),
	      (CARD8 *) &stuff[1] + (used >> 3), total - used + 7 >> 3);
	}
    else if( lines >= 0 )
	xpPos_(xp,plane) = 0;		/* there are no bits remainding	    */

    xpHeight_(xp,plane)  = 0;		/* there are no complete scanlines  */

    return( Success );
}				    /* end PutStreamPCM */

/*-----------------------------------------------------------------------
------------------------ routine:  Setup Xport Udp ----------------------
------------------------------------------------------------------------*/
static void SetupXportUdp( udp, snk, xp, plane, comp )
 UdpPtr		udp;	/* Udp pointer					    */
 PipeSinkPtr	snk;	/* Sink pointer					    */
 XportPtr	xp;	/* Xport context pointer			    */
 CARD32	plane, comp;	/* client plane number and sink component index	    */
{
    CARD32  i;

    switch( xpCmpOrg_(xp) )
	{
    case XieK_BandByPixel :
	upPxlLen_(udp) = BitsFromLevels_(SnkLvl_(snk,comp));
	upLvl_(udp)    = SnkLvl_(snk,comp);
	upCmpIdx_(udp) = SnkCmpIdx_(snk,comp);
	upDType_(udp)  = UtilDType(upPxlLen_(udp),XieK_PCM,xpPxlStr_(xp,plane),
							   xpScnStr_(xp,plane));
	upClass_(udp)  = UtilClass( upDType_(udp), XieK_PCM );
	break;

    case XieK_BandByPlane :
	break;

    case XieK_BitByPlane :
       *udp		 = SnkUdp_(snk,comp);
	upDType_(udp)    = UdpK_DTypeVU;
	upClass_(udp)    = UdpK_ClassUBA;
	upPxlLen_(udp)   = 1;
	upLvl_(udp)      = 2;
	for(upPos_(udp) += plane, i = 0; i < comp; i++ )
	    upPos_(udp) -= BitsFromLevels_(SnkLvl_(snk,i));
	break;
	}
}				    /* end SetupXportUdp */

/*-----------------------------------------------------------------------
-------------------- routine:  Create a Decode element ------------------
------------------------------------------------------------------------*/
static PipeSinkPtr _CreateDecode( pipe, img )
 Pipe		pipe;	/* DDX pipeline pointer				    */
 PhotomapPtr	img;	/* Photo{map|tap} pointer			    */
{
    PipeSinkPtr src, dst = Sink_(img);
    UdpPtr	udp;
    INT32 c, s, lvl[XieK_MaxComponents];
    /*
    **	Create a new Sink for the encoded data.  Request PCM Udps
    **	so we can swap them for the encoded Udps in the dst Sink.
    */
    for( c = 0; c < CmpCnt_(img); lvl[c] = SnkLvl_(dst,c), c++ );
    src = UtilCreateSink( img, XieK_PCM, lvl,
			  IsPhotomap_(img) && DdxCmpPreferred_(Cmpres_(img)),
			  FALSE, NULL );	
    if( src == NULL ) return( NULL );
    SnkFull_(src)        = SnkFull_(dst);		/* swap SinkFull    */
    SnkFull_(dst)        = FALSE;
    SnkUdpFinalMsk_(src) = SnkUdpFinalMsk_(dst);	/* swap UdpFinal    */
    SnkUdpFinalMsk_(dst) = 0;
    DdxRmSetDType_( src, UdpK_DTypeV, DtM_V );		/* src encoded	    */
    DdxRmSetAlignment_( dst, DdxAlignDefault_, NULL );	/* dst alignment    */
    for( c = 0; c < CmpCnt_(img); c++ ) {
	udp               = SnkUdpPtr_(src,c);		/* swap Udps	    */
	SnkUdpPtr_(src,c) = SnkUdpPtr_(dst,c);
	SnkUdpPtr_(dst,c) = udp;
	if( IsPointer_(DdxAlignDefault_) )
	    (*DdxAlignDefault_)(udp, NULL);		/* align PCM Udps   */
	}
    switch( Cmpres_(img) )	/* prepend the appropriate decoder element  */
	{
    case XieK_DCT  : s = DdxCreateDecodeDct_(pipe,src,dst,CmpOrg_(img)); break;
    case XieK_G42D : s = DdxCreateDecodeG4_( pipe,src,dst);		 break;
    case XieK_G31D :
    case XieK_G32D :
    default	   : s = BadValue;
	}
    if( s != Success )	return( DdxRmDestroySink_(src) );

    if( IsPhotomap_(img) && DdxCmpPreferred_(Cmpres_(img)) ) {
	Sink_(img)	= src;			/* keep the encoded data    */
	SnkDelete_(dst) = TRUE;			/* delete the PCM data	    */
	SnkWrite_(dst)  = FALSE;
	SnkPrm_(dst)    = FALSE;
    } else {
	SnkDelete_(src) = TRUE;			/* delete the encoded data  */
	Cmpres_(img)	= XieK_PCM;		/* keep the PCM data	    */
	CmpOrg_(img)	= XieK_BandByPlane;
    }
    return( src );
}				/* End of _CreateDecode */

/*-----------------------------------------------------------------------
-------------- routine: Create Get_Stream Photoflo element --------------
------------------------------------------------------------------------*/
static int  _GetStreamCreate( pipe, img )
 Pipe		pipe;
 PhotomapPtr	img;
{
    XportPtr xp = Tport_(img);
    GetStreamPipeCtxPtr  ctx;
    CARD32  drn, mask, plane;
    INT32   i, s = Success;

    switch( xpCmpres_(xp) )
	{
    case XieK_DCT  :
	if( xpIsBndPxl_(xp) )
	    s = DdxCreateEncodeDct_( pipe, xpSnk_(xp), (1<<CmpCnt_(img))-1, 
				     xpUdp_(xp,0), xpCmpPrm_(xp),
				    &xpFinMsk_(xp) );
	else
	    for( i = 0;  i < CmpCnt_(img) && s == Success;  i++ )
		 if( xpUdp_(xp,i) )
		    s = DdxCreateEncodeDct_( pipe, xpSnk_(xp), 1<<i,
					     xpUdp_(xp,i), xpCmpPrm_(xp),
					    &xpFinMsk_(xp) );
	break;
    case XieK_G42D :
	s = DdxCreateEncodeG4_(pipe, xpSnk_(xp), &xpUdp_(xp,0), &xpFinMsk_(xp));
	break;
    case XieK_PCM :
	ctx = (GetStreamPipeCtxPtr)
	       DdxCreatePipeCtx_( pipe, &GetStreamPipeElement, FALSE );
	if( !IsPointer_(ctx) ) return( (int) ctx );
	GsMap_(ctx)    = img;
	GsDrnCnt_(ctx) = xpIsBndPxl_(xp) ? CmpCnt_(img) : 1;

	if( !xpIsBndPxl_(xp) )
	    {   /*
		**  Create a mask for the components the client has requested.
		*/
	    mask  = 0;
	    plane = xpIsBndPln_(xp) ? XieK_MaxComponents : XieK_MaxPlanes;
	    while( plane-- > 0 )
		if( xpUdp_(xp,plane) )
		    mask |= 1<<xpCmpIdx_(xp,plane);
	    }
	for( drn = 0; drn < GsDrnCnt_(ctx); drn++ )
	    {	/*
		**  Create drains as required (either 1 or 3).
		*/
	    if( xpIsBndPxl_(xp) )
		mask = (1<<drn);
	    GsDrn_(ctx,drn) = DdxRmCreateDrain_( xpSnk_(xp), mask );
	    if( !IsPointer_(GsDrn_(ctx,drn)) ) return( (int) GsDrn_(ctx,drn) );
	    DdxRmSetDType_(GsDrn_(ctx,drn), uDType_(img,0), 1<<uDType_(img,0));
	    DdxRmSetQuantum_( GsDrn_(ctx,drn), 0 );
	    }
	break;
    case XieK_G31D :				       /* not yet supported */
    case XieK_G32D :				       /* not yet supported */
    default        : s = BadValue;
	}
    return( s );
}				    /* end _GetStreamCreate */

/*-----------------------------------------------------------------------
----------- routine: Initialize Get_Stream Photoflo element -------------
------------------------------------------------------------------------*/
static int  _GetStreamInitialize(ctx)
 GetStreamPipeCtxPtr  ctx;	/* GetStream pipe context pointer	    */
{
    CARD32  comp, drn, s = Success;

    /*
    **	Initialize our format conversion sink.
    */
    DdxRmCreateSink_( &GsCvt_(ctx) );

    for( comp = 0; comp < CmpCnt_(GsMap_(ctx)); comp++ )
	{   /*
	    **	Initially copy the source sink's Udps so BitByPlane can figure
	    **	out the correct "Pos" offset for the plane being requested.
	    */
	GsUdp_(ctx,comp) = *Udp_(GsMap_(ctx),comp);
	urBase_(GsUdp_(ctx,comp)) = NULL;

	SnkUdpPtr_(&GsCvt_(ctx),comp) = &GsUdp_(ctx,comp);
	}

    /*
    **	Initialize input drain(s).
    */
    for( drn = 0; drn < GsDrnCnt_(ctx) && s == Success; drn++ )
	s = DdxRmInitializePort_( CtxHead_(ctx), GsDrn_(ctx,drn) );

    /*
    **	Request input from drain 0 first.
    */
    CtxInp_(ctx) = GsDrn_(ctx,0);

    return( s );
}				    /* end _GetStreamInitialize */

/*-----------------------------------------------------------------------
------------- routine: Activate Get_Stream Photoflo element -------------
------------------------------------------------------------------------*/
static int  _GetStreamActivate(ctx)
 GetStreamPipeCtxPtr  ctx;	/* GetStream pipe context pointer	    */
{
    PipeDataPtr  dat;
    XportPtr	 xp = Tport_(GsMap_(ctx));
    CARD32 comp, drn, plane, s;

    for( s = Success; s == Success; _GetStreamFreeData(ctx) )
	{
	for( drn = 0; drn < GsDrnCnt_(ctx); drn++ )
	    if( !IsPointer_(GsDat_(ctx,drn)) )
		{
		if( drn > 0 )
		    /*
		    **  Set quantum to match what we got from the red drain.
		    */
		    DdxRmSetQuantum_(GsDrn_(ctx,drn),DatHeight_(GsDat_(ctx,0)));

		dat = DdxRmGetData_( ctx, GsDrn_(ctx,drn) );
		if( dat == NULL )      break;		    /* no more data */
		if( !IsPointer_(dat) ) return( (int) dat ); /* error status */
		comp = DatCmpIdx_(dat);
		GsDat_(ctx,comp) = dat;
		GsUdp_(ctx,comp) = DatUdp_(dat);
		}
	if( drn < GsDrnCnt_(ctx) ) break;   /* insufficient data available  */
	if( !IsPointer_(xp) )   continue;   /* client no longer interested  */

	switch( xpCmpOrg_(xp) )
	    {	/*
		**  Move available data to the Xport Udp(s).
		*/
	case XieK_BandByPixel :
	    s = _GetStreamConvert( ctx, xp, 0 );
	    break;

	case XieK_BandByPlane :
	    s = _GetStreamConvert( ctx, xp, comp );
	    break;

	case XieK_BitByPlane  :
	    for( plane = 0; plane < XieK_MaxPlanes && s == Success; plane++ )
		if( xpUdp_(xp,plane) && xpCmpIdx_(xp,plane) == comp )
		    s = _GetStreamConvert( ctx, xp, plane );
	    }	    
	}
    return( s );
}				    /* end _GetStreamActivate */

/*-----------------------------------------------------------------------
------------- routine: Destroy Get_Stream Photoflo element --------------
------------------------------------------------------------------------*/
static int  _GetStreamDestroy(ctx)
 GetStreamPipeCtxPtr  ctx;	/* GetStream pipe context pointer	    */
{
    CARD32  drn;

    for( drn = 0; drn < GsDrnCnt_(ctx); drn++ )
	GsDrn_(ctx,drn) = DdxRmDestroyDrain_( GsDrn_(ctx,drn) );

    return( Success );
}				    /* end _GetStreamDestroy */

/*-----------------------------------------------------------------------
----------------- routine: Convert Get_Stream Data segment --------------
------------------------------------------------------------------------*/
static int  _GetStreamConvert( ctx, xp, plane )
 GetStreamPipeCtxPtr  ctx;	/* GetStream pipe context pointer	    */
 XportPtr	      xp;	/* Xport context pointer		    */
 CARD32		      plane;	/* Xport plane number	    		    */
{
    PipeSinkPtr	     snk = &GsCvt_(ctx);
    CARD32  s, pos, comp = xpCmpIdx_(xp,plane);

    if( IsPointer_(xpBase_(xp,plane)) )
	{
	pos		     = xpPos_(xp,plane);
	xpPos_(xp,plane)     = xpArSize_(xp,plane)  -  ( SnkY1_(snk,comp)
						    * xpScnStr_(xp,plane));
	xpArSize_(xp,plane) += SnkHeight_(snk,comp) * xpScnStr_(xp,plane);
	xpBase_(xp,plane)    = DdxReallocBits_(xpBase_(xp,plane),
					     xpArSize_(xp,plane));
	}
    else
	{
	pos		     = 0;
	xpPos_(xp,plane)     =  -( SnkY1_(snk,comp) * xpScnStr_(xp,plane));
	xpArSize_(xp,plane)  = SnkHeight_(snk,comp) * xpScnStr_(xp,plane);
	xpBase_(xp,plane)    = DdxMallocBits_(xpArSize_(xp,plane));
	}
    if( xpBase_(xp,plane) == NULL ) return( BadAlloc );

    s = PlaneFromSink( xp, snk, plane );
    if( s == Success )
	{
	xpPos_(xp,plane) = pos;

	if( SnkY2_(snk,comp) >= xpY2_(xp,plane) )
	    xpFinMsk_(xp) |= 1<<plane;
	}
    return( s );
}				    /* end _GetStreamConvert */

/*-----------------------------------------------------------------------
------------------ routine: Free Get_Stream Data segment ----------------
------------------------------------------------------------------------*/
static int  _GetStreamFreeData(ctx)
 GetStreamPipeCtxPtr  ctx;	/* GetStream pipe context pointer	    */
{
    CARD32 comp;

    /*
    **	Flush all data collected from our drain(s).
    */
    for( comp = 0; comp < XieK_MaxComponents; comp++ )
	if( IsPointer_(GsDat_(ctx,comp)) )
	    {
	    GsDat_(ctx,comp) = DdxRmDeallocData_( GsDat_(ctx,comp) );
	    urBase_(GsUdp_(ctx,comp)) = NULL;
	    }
    return( Success );
}				    /* end _GetStreamFreeData */
 
/*-----------------------------------------------------------------------
--------- routine:  push Compressed data into a running Photoflo --------
------------------------------------------------------------------------*/
static int _PutCmp( img, xp, p, own )
 PhotomapPtr	img;	/* Photo{map|tap} pointer			    */
 XportPtr	xp;	/* transport context pointer			    */
 CARD32		p;	/* client plane number				    */
 BOOL		own;	/* TRUE: XIE owns data, FALSE: X11 owns data	    */
{
    PipeSinkPtr	snk = xpSnk_(xp);
    PipeDataPtr data;
    INT32 s;
    /*
    **  Allocate a pipeline descriptor for the compressed data.
    */
    data = DdxRmAllocCmpDataDesc_( snk, p, xpArSize_(xp,p) );
    if( IsPointer_(data) )
	{   /*
	    **	Install the data pointer and remember who owns it.
	    */
	DatBase_(data)  = xpBase_(xp,p);
	DatOwned_(data) = own;
	    /*
	    **  Push compressed data into the running Photoflo.
	    */
	s = DdxRmPutData_( snk, data );
	if( s == Success )
	    s  = ResumePipeline_( FloLnk_(img) );
	if( s == Success && !own )
	    /*
	    **  Make sure the pipe isn't hanging on to X11's wire buffer.
	    */
	    s  = DdxRmOwnData_( snk, xpBase_(xp,p), p );
	}
    else
	{
	s = (int) data;
	if( own )
	    DdxFreeBits_( xpBase_(xp,p) );
	}
    /*
    **  All this data is now safely out of our hands -- so forget about it.
    */
    xpArSize_(xp,p) = 0;
    xpBase_(xp,p)   = NULL;

    return( s );
}				/* end _PutCmp */
 
/*-----------------------------------------------------------------------
---------- routine:  push PCM scanlines into running Photoflo -----------
------------------------------------------------------------------------*/
static int _PutDataPCM( xp, snk, comp, y2 )
 XportPtr     xp;	/* Xport context pointer			    */
 PipeSinkPtr  snk;	/* Destination sink pointer			    */
 CARD32	      comp;	/* component index				    */
 INT32		y2;	/* y2 -- ending scanline of available data	    */
{
    INT32 s, Y2 = DatY2_(xpDat_(xp,comp));

    /*
    **	Push the image data into the Photoflo.
    */
    s = DdxRmPutData_( snk, xpDat_(xp,comp) );

    if( s == Success )
	if( Y2 < y2 )
	    /*
	    **  Initialize another destination data segment.
	    */
	    s = _PutNextPCM( xp, snk, comp, Y2+1, y2-Y2 );
	else
	    /*
	    **	No more data availbable.
	    */
	    xpDat_(xp,comp) = NULL;

    return( s );
}				/* end _PutDataPCM */

/*-----------------------------------------------------------------------
------------ routine:  allocate Next PCM Photoflo Data segment ----------
------------------------------------------------------------------------*/
static int _PutNextPCM( xp, snk, comp, start, height )
 XportPtr    xp;	/* transport context pointer			    */
 PipeSinkPtr snk;	/* Destination sink pointer			    */
 CARD32	     comp;	/* component index				    */
 INT32	     start;	/* beginning scanline for data segment		    */
 INT32	     height;	/* scanlines available (negative = override quantum)*/
{
    INT32 length, s = Success;

    if( height < 0 )
	length = -height;	    /* must accept all available scanlines  */
    else
	/*
	**  Use the lesser: the maximum quantum, or the amount available.
	*/
	length = SnkMaxQnt_(snk) > 0 ? min(SnkMaxQnt_(snk), height) : height;

    /*
    **  Don't go beyond the end of the image.
    */
    length = min(length, SnkHeight_(snk,comp)-start);
    /*
    **  Allocate a data segment descriptor.
    */
    xpDat_(xp,comp) = DdxRmAllocData_( snk, comp, start, length );
    if( !IsPointer_(xpDat_(xp,comp)) )
	{
	s = (int) xpDat_(xp,comp);
	xpDat_(xp,comp) = NULL;
	}

    return( s );
}				/* end _PutNextPCM */

/* end module XieTransport.c */
