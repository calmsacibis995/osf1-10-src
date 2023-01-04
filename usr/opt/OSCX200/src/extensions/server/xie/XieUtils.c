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
**	The Xie Utilities module 
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
**      February 28, 1989
**
*******************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <extnsionst.h>
#include <dixstruct.h>
#include <os.h>			/* for (DE)ALLOCATE_LOCAL Macros */

#include <XieAppl.h>
#include <XieDdx.h>
#include <XieProto.h>
#include <XieServer.h>
#include <XieMacros.h>
#include <XiePipeInterface.h>

/*
**  Table of contents
*/
UdpPtr		UtilAllocUdp();
int		UtilBePCM();
PhotomapPtr	UtilCreatePhotomap();
PipeSinkPtr	UtilCreateSink();
CARD8		UtilClass();
CARD8		UtilDType();
int		UtilFreeResource();
UdpPtr		UtilFreeUdp();
void		UtilSetPending();
Bool            UtilValidRegion();
/*
**  Equated Symbols
*/

/*
**  MACRO definitions
*/

/*
**  External References
*/
extern void	 FreeResource();	    /* ref'd  by FreeResource_ macro*/

extern void	 FreeExport();		    /* XieDisplay.c		    */
extern void	 FreeTransport();	    /* XieTransport.c		    */

/*
**	Local Storage
*/

/*-----------------------------------------------------------------------
--------------------------- routine:  Allocate Udp ----------------------
------------------------------------------------------------------------*/
UdpPtr UtilAllocUdp( type, cmpres, w, h, pxlstr, scnstr,
		     lvl, idx, base, pos, align, algn_dat )
 unsigned char	type;	    /* data type of Udp				    */
 unsigned char	cmpres;	    /* data compression scheme			    */
 CARD32		w;	    /* width of image in pixels			    */
 CARD32		h;	    /* height of image in pixels		    */
 CARD32		pxlstr;	    /* pixel stride in bits			    */
 CARD32		scnstr;	    /* scanline stride in bits			    */
 CARD32		lvl;	    /* levels					    */
 CARD32		idx;	    /* component index				    */
 Bool		base;	    /* True == allocate buffer			    */
 CARD32		pos;	    /* offset to beginning of data in bits	    */
 Fcard32	align;	    /* ptr to image array alignment function	    */
 unsigned long *algn_dat;   /* ptr to prvt data for align func		    */
{
    UdpPtr udp = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( udp == NULL ) return(NULL);

    if( cmpres == XieK_PCM )
	switch( type )
	    {
	case UdpK_DTypeBU : upPxlLen_(udp) =  8; break;
	case UdpK_DTypeWU : upPxlLen_(udp) = 16; break;
	case UdpK_DTypeLU :
	case UdpK_DTypeF  : upPxlLen_(udp) = 32; break;
	case UdpK_DTypeV  :
	case UdpK_DTypeVU : upPxlLen_(udp) = BitsFromLevels_(lvl); break;
	    }
    else		    upPxlLen_(udp) = 0;

    upDType_(udp)  = type;
    upClass_(udp)  = UtilClass( type, cmpres );
    upPxlStr_(udp) = cmpres == XieK_PCM ? pxlstr : 0;
    upScnStr_(udp) = cmpres == XieK_PCM ? scnstr : 0;
    upX1_(udp)	   = 0;
    upX2_(udp)	   = w - 1;
    upY1_(udp)	   = 0;
    upY2_(udp)	   = (signed long) h - 1;
    upWidth_(udp)  = w;
    upHeight_(udp) = h;
    upPos_(udp)    = pos;
    upCmpIdx_(udp) = idx;
    upLvl_(udp)    = lvl;
    upArSize_(udp) = pos + h * upScnStr_(udp);
    if( IsPointer_(align) )
	(*align)(udp, algn_dat);

    if( base && upArSize_(udp) > 0 )
	{
	upBase_(udp) = DdxMallocBits_( upArSize_(udp) );
	if( upBase_(udp) == NULL )
	    udp = UtilFreeUdp(udp);
	}
    else
	upBase_(udp) = NULL;

    return( udp );
}				/* end UtilAllocUdp */

/*-----------------------------------------------------------------------
------------ routine:  ensure that Photomap contains PCM data -----------
------------------------------------------------------------------------*/
int UtilBePCM( img )
 PhotomapPtr img;	/* Photomap pointer				    */
{
    PipeSinkPtr new, old = Sink_(img);
    INT32      c, status = Success, lvl[XieK_MaxComponents];

    if( Tport_(img) && AnyPending_(img) ) return( BadAccess );
    if(!IsPCM_(img) )
	{				    /* create a PCM Sink for decode */
	for( c = 0; c < CmpCnt_(img); lvl[c] = SnkLvl_(old,c), c++ );
	new = UtilCreateSink(img, XieK_PCM, lvl, TRUE, FALSE, DdxAlignDefault_);
	if( new == NULL ) return( BadAlloc );
	if( !AnyPending_(img) )
	    switch( Cmpres_(img) )	    /* since all the data's here    */
		{			    /* decode it into the new Sink  */
	    case XieK_DCT  :
		if( IsBndPxl_(img) )
		    status = DdxDecodeDct_( SnkUdpPtr_(old,0),
					   &SnkUdpPtr_(new,0), CmpCnt_(img) );
		else
		    for( c = 0; c < CmpCnt_(img) && status == Success; c++ )
			status = DdxDecodeDct_( SnkUdpPtr_(old,c),
					       &SnkUdpPtr_(new,c), 1 );
		break;
	    case XieK_G42D :
		status = DdxDecodeG4_( SnkUdpPtr_(old,0), SnkUdpPtr_(new,0) );
		break;
	    case XieK_G31D :
	    case XieK_G32D :
	    default	   : status = BadValue;
		}
	Sink_(img)   = new;			 /* replace the compressed  */
	Cmpres_(img) = XieK_PCM;		 /* Sink with the PCM Sink  */
	CmpOrg_(img) = XieK_BandByPlane;
	DdxRmDestroySink_( old );
	if( AnyPending_(img) )			 /* if some data is missing */
	    return( UtilBePCM(img) );		 /* make a zeroed PCM image */
	}
    else
	for( c = 0; c < CmpCnt_(img); c++ )	 /* create a zeroed array   */
	    if( !IsPointer_(uBase_(img,c)) )	 /* for absent components   */
		{
		uBase_(img,c) = DdxCallocBits_( uArSize_(img,c) );
		if( uBase_(img,c) == NULL ) return( BadAlloc );
		}
    SnkFull_(Sink_(img))	= TRUE;
    SnkUdpFinalMsk_(Sink_(img)) = (1<<CmpCnt_(img))-1;
    UtilSetPending( img, 0, XieK_BandByPixel, FALSE );

    return( status );
}				    /* end UtilBePCM */

/*-----------------------------------------------------------------------
------------------ routine:  Create Photomap (or Phototap) --------------
------------------------------------------------------------------------*/
PhotomapPtr UtilCreatePhotomap(	cmpmap, cmplvl, pxlratio, pxlprog, scnprog,
				pxlpol, width, height, permanent, base, align )
 int		cmpmap;		/* component mapping			    */
 CARD32	       *cmplvl;		/* levels per component			    */
 float		pxlratio;	/* pixel aspect ratio			    */
 int		pxlprog;	/* pixel progression			    */
 int		scnprog;	/* scanline progression			    */
 int		pxlpol;		/* pixel brightness polarity		    */
 int		width, height;	/* dimensions of image in pixels	    */
 Bool		permanent;	/* True == Photomap, else Phototap	    */
 Bool		base;		/* True == alloc image data arrays	    */
 Fcard32	align;		/* ptr to image array alignment function    */
{
    PhotomapPtr img = (PhotomapPtr) DdxCalloc_(1,sizeof(PhotomapRec));
    if( img == NULL ) return( NULL );
    /*
    **	Fill in the Photo{map|tap}'s attributes.
    */
    ResType_(img)     =  permanent ? XieK_Photomap : XieK_Phototap;
    RefCnt_(img)      =  1;
    Constrained_(img) = *cmplvl > 0;
    CmpOrg_(img)      =  XieK_BandByPlane;
    Cmpres_(img)      =  XieK_PCM;
    CmpMap_(img)      =  cmpmap;
    CmpCnt_(img)      =  IsRGB_(img) ? 3 : 1;
    PxlRatio_(img)    =  pxlratio;
    PxlProg_(img)     =  pxlprog;
    ScnProg_(img)     =  scnprog;
    PxlPol_(img)      =  pxlpol;
    Width_(img)	      =  width;
    Height_(img)      =  height;
    /*
    **	Create a Sink to hold the Photo{map|tap}'s data (assume PCM for now).
    */
    Sink_(img) = UtilCreateSink(img, XieK_PCM, cmplvl, permanent, base, align);
    if( Sink_(img) == NULL )
	img = (PhotomapPtr) DdxFree_( img );
    
    return( img );
}				/* end UtilCreatePhotomap */

/*-----------------------------------------------------------------------
------------ routine:  Create a Sink for a Photo{map|tap} ---------------
------------------------------------------------------------------------*/
PipeSinkPtr UtilCreateSink( img, cmpres, cmplvl, permanent, base, align )
 PhotomapPtr	img;		/* Photo{map|tap} pointer		    */
 unsigned long  cmpres;		/* compression scheme			    */
 CARD32	       *cmplvl;		/* levels per component			    */
 Bool		permanent;	/* True == sink for Photomap, else Phototap */
 Bool		base;		/* True == alloc image data arrays	    */
 Fcard32	align;		/* ptr to image array alignment function    */
{
    CARD32    pxlstr,  scnstr, dtype,  c = CmpCnt_(img);
    PipeSinkPtr sink = DdxRmCreateSink_( NULL );
    if( !IsPointer_(sink) ) return( NULL );
    /* 
    **	Allocate Udp's describing image data.
    */
    while( c-- > 0 )
	{
	pxlstr = cmplvl[c] == 0       ? 32	    /* unconstrained  float */
	       : cmplvl[c] <= 1 <<  1 ?  1	    /* bitonal	       bits */
	       : cmplvl[c] <= 1 <<  8 ?  8	    /* continuous tone byte */
	       : cmplvl[c] <= 1 << 16 ? 16	    /* continuous tone word */
				      : 32;	    /* continuous tone long */
	scnstr = pxlstr * Width_(img);
	dtype  = cmplvl[c] > 0 ? UtilDType( pxlstr, cmpres, pxlstr, scnstr )
			       : UdpK_DTypeF;

	SnkUdpPtr_(sink,c) = UtilAllocUdp( dtype, cmpres,
					   Width_(img), Height_(img),
					   pxlstr, scnstr, cmplvl[c],
					   c, base, 0, align, NULL );
	if( !IsPointer_(SnkUdpPtr_(sink,c)) )
	    return( DdxRmDestroySink_( sink ) );
	}
    /*
    **  Only be specific about DType for PCM data.
    */
    if( cmpres == XieK_PCM )
	DdxRmSetDType_( sink, dtype, 1<<dtype );
    else
	DdxRmSetDType_( sink, UdpK_DTypeUndefined, DtM_Any );

    DdxRmSetAlignment_( sink, align, NULL );
    SnkPrm_(sink)   = permanent;
    SnkWrite_(sink) = permanent;
    
    return( sink );
}				/* end UtilCreateSink */

/*-----------------------------------------------------------------------
------------------------- routine:  compute Udp Class -------------------
------------------------------------------------------------------------*/
CARD8 UtilClass( dtype, cmpres )
 CARD8	dtype;			/* Udp data type			    */
 CARD8	cmpres;			/* data compression scheme		    */
{
    CARD8 class;

    if( cmpres == XieK_PCM )
	switch( dtype )
	    {
	case UdpK_DTypeBU :
	case UdpK_DTypeWU :
	case UdpK_DTypeLU :
	case UdpK_DTypeF  : class = UdpK_ClassA;   break;
	case UdpK_DTypeV  :
	case UdpK_DTypeVU : class = UdpK_ClassUBA; break;
	    }
    else
	class = UdpK_ClassUBS;

    return( class );
}				    /* end UtilClass */

/*-----------------------------------------------------------------------
---------------------- routine:  compute Udp Data Type ------------------
------------------------------------------------------------------------*/
CARD8 UtilDType( bpp, cmpres, pxlstr, scnstr )
 CARD8	bpp, cmpres, pxlstr;	/* bits per pxl, compression & pixel stride */
 CARD32	scnstr;			/* scanline stride			    */
{
    CARD8 dtype;

    if( cmpres == XieK_PCM  &&  scnstr % pxlstr == 0  &&  pxlstr % bpp == 0 )
	switch( bpp )
	    {
	case  8 : dtype = UdpK_DTypeBU; break;
	case 16 : dtype = UdpK_DTypeWU; break;
	case 32 : dtype = UdpK_DTypeLU; break;
	default : dtype = UdpK_DTypeVU; break;
	    }
    else if( cmpres == XieK_PCM && pxlstr & 7 )
	    dtype = UdpK_DTypeVU;
    else
	    dtype = UdpK_DTypeV;

    return( dtype );
}				    /* end UtilDType */

/*-----------------------------------------------------------------------
---------------------------- routine:  Free Resource --------------------
------------------------------------------------------------------------*/
int UtilFreeResource( res )
 CommonPartPtr res;	/* resource pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr	img;

    if( IsPointer_(res)  &&  --RefCnt_(res) == 0 )
	{
	switch( ResType_(res) )
	    {
	case XieK_Photoflo :
	    flo = (PhotofloPtr) res;
	    if( IsPointer_(Pipe_(flo)) )
		DestroyPipeline_(  flo );
	    if( IsPointer_(QueSrc_(flo)) )
		do  {	/*
			**  Free all the resources in our resource queue.
			*/
		    QueDst_(flo) = QueEnd_(flo);
		    if( IsPhotomap_(FloRes_(QueDst_(flo))) )
			FloLnk_(DstMap_(flo)) = NULL;
		    if( IsPhototap_(FloRes_(QueDst_(flo)))
			  && ResId_(FloRes_(QueDst_(flo))) != 0 )
			FreeResource_(ResId_(FloRes_(QueDst_(flo))));
		    else
			UtilFreeResource(FloRes_(QueDst_(flo)));
		    DdxFree_( RemQue_(QueDst_(flo)));
		    }
		while( QueDst_(flo) != QueSrc_(flo) );
	    break;

	case XieK_Photomap :
	case XieK_Phototap :
	    img = (PhotomapPtr) res;
	    /* 
	    **  Release contexts before releasing the sink.
	    */
	    FreeExport( img );
	    FreeTransport( img, XieK_AllPlanes );
	    /* 
	    **  Free image data sink (unless the Pipeline is suppose to).
	    */
	    if( IsPointer_(Sink_(img)) && !SnkDelete_(Sink_(img)) )
		DdxRmDestroySink_( Sink_(img) );
	    break;

	case XieK_IdcCpp :
	    UtilFreeResource( CppMap_((CppPtr)res) );
	    /* Don't free the Udp bits -- they belonged to the Photomap */
	    DdxFree_( CppUdp_((CppPtr)res) );
	    break;

	case XieK_IdcRoi :
	    UtilFreeUdp( RoiUdp_((RoiPtr)res) );
	    break;

	case XieK_IdcTmp :
	    if( IsPointer_(TmpDat_((TmpPtr)res)) )
		DdxFree_( TmpDat_((TmpPtr)res)->array );
	    DdxFree_(TmpDat_((TmpPtr)res));
	    break;

	default : return(BadRequest);
	    }
	DdxFree_( res );
	}
    return( Success );
}				/* end UtilFreeResource */

/*-----------------------------------------------------------------------
---------------------------- routine:  Free Udp -------------------------
------------------------------------------------------------------------*/
UdpPtr UtilFreeUdp(udp)
 UdpPtr udp;
{
    if( IsPointer_(udp) )
	{   /*
	    **  If you don't want your data freed, you must NULL the pointer.
	    */
	if( IsPointer_(upBase_(udp)) )
	    DdxFreeBits_( upBase_(udp) );
	DdxFree_( udp );
	}
    return( NULL );
}				/* end UtilFreeUdp */

/*-----------------------------------------------------------------------
------------------ routine:  Set/reset plane Pending mask ---------------
------------------------------------------------------------------------*/
void UtilSetPending( map, plane, org, state )
 PhotomapPtr map;	/* Photo{map|tap} pointer			    */
 CARD32	     plane;	/* client plane index				    */
 CARD32	     org;	/* client component organization		    */
 CARD8	     state;	/* Boolean - True to set, False to reset	    */
{
    CARD32  c, p, bits[XieK_MaxComponents];
    /*
    **	Get bit width required for each component.
    */
    for( c = 0; c < CmpCnt_(map); c++ )
	bits[c] = BitsFromLevels_(uLvl_(map,c));

    switch( org )
        {
    case XieK_BandByPixel :
        /*
        **  Set or clear all bits of all components.
        */
        for( c = 0; c < CmpCnt_(map); c++ )
	    CmpPending_(map,c) = state ? (1 << bits[c]) - 1 : 0;
        break;

    case XieK_BandByPlane :
        /*
        **  Set or clear all bits of the specified component.
        */
	CmpPending_(map,plane) = state ? (1 << bits[plane]) - 1 : 0;
        break;

    case XieK_BitByPlane  :
        /*
        **  Set or clear a single bit.
        */
        for( p = 0, c = 0; c < CmpCnt_(map) && p <= plane; p += bits[c++] )
	    if( plane < bits[c] + p )
		if( state )
		    CmpPending_(map,c) |=   1 << plane - p;
		else
		    CmpPending_(map,c) &= ~(1 << plane - p);
        break;
        }
    /*
    **	Now see if any and all planes are still pending.
    */
    AnyPending_(map) = FALSE;
    AllPending_(map) = TRUE;
    for( c = 0; c < CmpCnt_(map); c++ )
	{
	AnyPending_(map) |= CmpPending_(map,c) != 0;
	AllPending_(map) &= CmpPending_(map,c) == (1 << bits[c]) - 1;
	}
}				/* end UtilSetPending */

/*-----------------------------------------------------------------------
---------------- routine:  Validate region against Photomap -------------
------------------------------------------------------------------------*/
Bool UtilValidRegion( map, idcudp )
 PhotomapPtr map;	/* Photo{map|tap} pointer			    */
 UdpPtr	     idcudp;	/* Udp corresponding to ROI/CPP	                    */
{
    /* To be valid, the IDC region must intersect the Photo at some point.  */

    if( upX1_(idcudp) > uX2_(map,0) || upY1_(idcudp) > uY2_(map, 0) ||
	upX2_(idcudp) < uX1_(map,0) || upY2_(idcudp) < uY1_(map, 0) )
	return( FALSE );
    else
	return( TRUE );
}					/* end UtilValidRegion */
/* End of Module XieUtils.c */
