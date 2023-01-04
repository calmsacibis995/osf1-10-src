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
**	This Xie module consists of DIX procedures for Display
**	service requests.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      November 1, 1989
**
************************************************************************/

/*
**  Definition to let include files know who's calling.
*/
#define _XieDisplay

/*
**  Include files
*/
#include <stdio.h>
    /*
    **  Core X Includes
    */
#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <miscstruct.h>
#include <extnsionst.h>
#include <dixstruct.h>
#include <resource.h>
#include <gc.h>
#include <gcstruct.h>

    /*
    **  XIE Includes
    */
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieProto.h>
#include <XieServer.h>
#include <XieDisplay.h>
#include <XieMacros.h>
#include <XieDdx.h>

/*
**  Table of contents
*/
    /*
    **  Xie protocol proceedures called from XieMain
    */
int  ProcDisplaySequence();
int  ProcExport();
int  ProcFreeExport();
int  ProcImport();
    /*
    **  routines referenced by other modules
    */
void FreeExport();
    /*
    **  routines used internally by XieDisplay.
    */
static int	 CreateExportContext();
static int	 CvtRGBBitZPixmap();
static int	 CvtRGBBytZPixmap();
#if defined(DWV3)
static void	 ExportSpans();
#endif
static int	 CvtZPixmap();
static int	 Export();
static void	 ExportDone();
static int	 Format();
static void	 FormatDirectColor();
static void	 FormatPseudoColor();
static void	 Histogram();
static CARD32	 HistogramAnalysis();
static int	 Import();
static int	 ImportSingleMapData();
static int	 ImportTripleMapData();
static int	 InitForSpans();
static CARD32	 MatchDirectAllocColor();
static CARD32	 MatchPseudoAllocColor();
static void	 MatchPseudoClosest();
static void	 MatchPseudoDistant();
static CARD32	 MatchPseudoInit();
static void	 ReadSingleMap();
static void	 ReadTripleMap();
static void	 SetLutEntry();
static void	 SetupUdp();
    /*
    **  routines used to support pipelines.
    */
static int 	_ExportCreate();
static int 	_ExportInitialize();
static int 	_ExportActivate();
static int 	_ExportFlush();
static int 	_ExportDestroy();
static int 	_ExportAbort();
static int 	_ExportFreeData();

/*
**  Equated Symbols
*/
/*
**  MACRO definitions
*/
/*
**  External References
*/
#if   defined(X11R3) && !defined(DWV3)
extern int	      UtilFreeResource();   /* ref'd  by AddResource_ macro */
#endif
extern int	      AllocColor();	    /* X11 colormap.c		    */
extern int	      FreeColors();	    /* X11 colormap.c		    */

extern int	      xieDoSendEvent();	    /* XieEvents.c		    */
extern UdpPtr	      UtilAllocUdp();	    /* XieUtils.c		    */
extern int	      UtilBePCM();	    /* XieUtils.c		    */
extern PhotomapPtr    UtilCreatePhotomap(); /* XieUtils.c		    */
extern UdpPtr	      UtilFreeUdp();	    /* XieUtils.c		    */

extern CARD32	      xieClients;	    /* num of clients served	    */
extern CARD32	      xieEventBase;	    /* Base for Xie events          */
extern CARD32	      xieReqCode;	    /* XIE main opcode		    */
#if   defined(X11R3) && !defined(DWV3)
extern CARD16	      RC_xie;		    /* XIE Class		    */
extern CARD16	      RT_photo;		    /* Photo{flo|map|tap} resource  */
extern CARD16	      RT_idc;		    /* Cpp, Roi & Tmp resource types*/
#else /* X11R4 || DWV3 */
extern RESTYPE	      RC_xie;		    /* XIE Class		    */
extern RESTYPE	      RT_photo;		    /* Photo{flo|map|tap} resource  */
extern RESTYPE	      RT_idc;		    /* Cpp, Roi & Tmp resource types*/
#endif

externalref int (*xieDixSwap[X_ieLastRequest])(); /* Reply swap dispatch
						   * table */
externalref int (*xieDixSwapReply[])();
externalref void (*xieDixSwapData[])();

extern void	Swap32Write();
/*
**	Local Storage
*/

    /*
    **  Export Element Vector
    */
static PipeElementVector ExportPipeElement = {
    NULL,                               /* FLINK                            */
    NULL,                               /* BLINK                            */
    sizeof(PipeElementVector),          /* Size                             */
    TypePipeElementVector,              /* Structure type                   */
    StypeExportElement,                 /* Structure subtype                */
    sizeof(ExportPipeCtx),              /* Context block size               */
    0,                                  /* No input flag                    */
    0,                                  /* reserved flags                   */
    _ExportInitialize,                  /* Initialize entry                 */
    _ExportActivate,                    /* Activate entry                   */
    _ExportFlush,                       /* Flush entry                      */
    _ExportDestroy,                     /* Destroy entry                    */
    _ExportAbort			/* Abort entry point                */
    };

#ifdef BIGENDIAN  /* BIGENDIANEVILHACK */
static unsigned char _reverse_byte[0x100] = {
 	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
 	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
 	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
 	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
 	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
 	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
 	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
 	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
 	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
 	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
 	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
 	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
 	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
 	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
 	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
 	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
 	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
 	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
 	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
 	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
 	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
 	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
 	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
 	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
 	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
 	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
 	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
 	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
 	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
 	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
 	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
 	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};
#endif

/*-----------------------------------------------------------------------
----------------------  Display Sequence Procedure ----------------------
------------------------------------------------------------------------*/
int ProcDisplaySequence( client )
 ClientPtr client;
{
    REQUEST( xieReq );

    BadRequest_(0);
}				    /* end ProcDisplaySequence */

/*-----------------------------------------------------------------------
---------------------- Export to drawable Procedure ---------------------
------------------------------------------------------------------------*/
int ProcExport( client )
 ClientPtr client;
{
    PhotofloPtr  flo;
    PhotomapPtr  img;
    ExpCtxPtr	 exp;
    INT32	 x, y, w, h, dx, dy, status;
    REQUEST( xieExportReq );
    REQUEST_SIZE_MATCH( xieExportReq );
    /* 
    **	Lookup Photo{flo|map|tap} -- check export region.
    */
    LookupPhotos_(&flo, &img, stuff->photo_id, -1);
    MatchOkIf_(Constrained_(img), stuff->photo_id);
    x  = max(stuff->src_x, 0);
    y  = max(stuff->src_y, 0);
    w  = min(stuff->src_x, 0) + stuff->width;
    h  = min(stuff->src_y, 0) + stuff->height;
    w  = Width_(img)  < x + w ? Width_(img)  - x : w;
    h  = Height_(img) < y + h ? Height_(img) - y : h;
    dx = stuff->dst_x + x - stuff->src_x;
    dy = stuff->dst_y + y - stuff->src_y;
    ValueOkIf_( w > 0 && h > 0, stuff->photo_id );

    status = CreateExportContext( client, img, x, y, w, h );
    if( status != Success )
	FreeExport( img );

    else if( flo != NULL )
	{   /*
	    **  Export later in a Photoflo.
	    */
	status = _ExportCreate( Pipe_(flo), img, dx, dy );
	OkIf_(status == Success, (FreeExport(img), stuff->photo_id), status);
	}
    else
	{   /*
	    **	Export now.
	    */
	BePCM_( img, (FreeExport(img), stuff->photo_id) );
	exp    = Eport_(img);
	status = Format( exp, Sink_(img) );
	OkIf_(status == Success, (FreeExport(img), stuff->photo_id), status);
	status = Export( exp, Sink_(img), ePixUdp_(exp), dx, dy );
	OkIf_(status == Success, (FreeExport(img), stuff->photo_id), status);
	ExportDone( exp );
	status = xieDoSendEvent( client, img, XieK_DisplayEvent );
#ifdef PEZD
	LIB$SHOW_TIMER();
	printf("PEZD : End Export\n");
#endif
	}
    return( status );
}				/* end ProcExport */

/*-----------------------------------------------------------------------
------------------------ Free Export context Procedure ------------------
------------------------------------------------------------------------*/
int ProcFreeExport( client )
 ClientPtr client;
{
    PhotofloPtr flo;
    PhotomapPtr img;
    REQUEST( xieFreeExportReq );
    REQUEST_SIZE_MATCH( xieFreeExportReq );
    /* 
    **	Check out the Photo{flo|map|tap}.
    */
    LookupPhotos_(&flo, &img, stuff->photo_id, 0);
    AccessOkIf_( !flo || !Make_(flo) || !Run_(flo), ResId_(flo));

    FreeExport( img );

    return( Success );
}				/* end ProcFreeExport */

/*-----------------------------------------------------------------------
---------------------- Import from drawable Procedure -------------------
------------------------------------------------------------------------*/
int ProcImport( client )
 ClientPtr client;
{
    PhotomapPtr img = NULL;
    REQUEST( xieImportReq );
    REQUEST_SIZE_MATCH( xieImportReq );

    CheckNewResource_( stuff->photomap_id, RT_photo );
    /* 
    **	Import the Drawable's image data.
    */
    if( Import( client, &img ) == Success )
	{   /* 
	    **  Add new Photomap.
	    */
	AddResource_( stuff->photomap_id, RT_photo, img );
	xieDoSendEvent( client, img, XieK_ComputationEvent );
	}
    if( img )
	FreeExport( img );

    return( Success );
}				/* end ProcImport */

/*-----------------------------------------------------------------------
----------------------- Query Export context Procedure ------------------
------------------------------------------------------------------------*/
int ProcQueryExport( client )
 ClientPtr client;
{
    PhotofloPtr flo;
    PhotomapPtr img;
    ExpCtxPtr	exp;
    xieQueryExportReply rep;
    REQUEST( xieQueryExportReq );
    REQUEST_SIZE_MATCH( xieQueryExportReq );
    /* 
    **	Check out the Photo{flo|map|tap}.
    */
    LookupPhotos_(&flo, &img, stuff->photo_id, 0);
    AccessOkIf_( !flo || Done_(flo), ResId_(flo));
    exp = Eport_(img);

    /*
    **	Create reply for client.
    */
    memset( &rep, 0, sizeof(xieQueryExportReply));
    rep.type		= X_Reply;
    rep.sequenceNumber  = client->sequence;
    if( exp != NULL )
	{
	if( stuff->get_pixels )
	    rep.length	= ePxlCnt_(exp);
	rep.pixel_count = ePxlCnt_(exp);

	if( stuff->lut_id != 0  &&  eLutMap_(exp) != NULL )
	    {
	    if( ResId_(eLutMap_(exp)) == 0 )
		{
		RefCnt_(eLutMap_(exp))++;
		AddResource_( stuff->lut_id, RT_photo, eLutMap_(exp) );
		}
	    rep.lut_id	    =  ResId_(eLutMap_(exp));
	    rep.lut_mapping = CmpMap_(eLutMap_(exp));
	    }
	}
    WriteXieReplyToClient(client, sizeof(xieQueryExportReply), &rep);

    if( rep.length > 0 )
	{
	SetSwappedDataRtn(client, CARD32);
	WriteSwappedXieDataToClient(client,
			       ePxlCnt_(exp) * sizeof(CARD32), ePxlLst_(exp));
	ePxlCnt_(exp) = 0;
	ePxlLst_(exp) = (CARD32 *) DdxFree_( ePxlLst_(exp) );
	}
    return( Success );
}				/* end ProcQueryExport */

/*-----------------------------------------------------------------------
------------------------  routine: Free Export context ------------------
------------------------------------------------------------------------*/
void FreeExport( img )
 PhotomapPtr img;
{
    ExpCtxPtr exp = Eport_(img);
    Pixel  pixel;
    CARD32 i;

    if( IsPointer_(exp) )
	{
	if( IsPointer_(eLutMap_(exp)) )
	    /*
	    **	Free the LUT.
	    */
	    UtilFreeResource( eLutMap_(exp) );

	if( IsPointer_(ePxlLst_(exp)) )
	    {	/*
		**  Free the pixels -- if the Colormap still exists.
		*/
	    LookupColormap_( eCmap_(exp), eCmapId_(exp) );

	    if( eCmap_(exp) && ePxlCnt_(exp) )
		if( sizeof(Pixel) == sizeof(CARD32) )
		    FreeColors(eCmap_(exp), eClientIdx_(exp),
			       ePxlCnt_(exp), (Pixel*)ePxlLst_(exp), (Pixel)0);
		else
		    for( i = 0; i < ePxlCnt_(exp); i++ )
			{
			pixel = ePxlLst_(exp)[i];
			FreeColors( eCmap_(exp), eClientIdx_(exp),
					      1, &pixel, (Pixel)0 );
			}
	    DdxFree_( ePxlLst_(exp) );
	    }

	/*
	**  Free everything else.
	*/
	ExportDone( exp );

	Eport_(img) = (ExpCtxPtr) DdxFree_( exp );
	}
}				/* end FreeExport */

/*-----------------------------------------------------------------------
----------- routine:  Create Context for Export to drawable -------------
------------------------------------------------------------------------*/
static int CreateExportContext( client, img, X, Y, W, H )
 ClientPtr client;
 PhotomapPtr	img;
 INT32		X,Y;
 CARD32		W,H;
{
    PhotofloPtr  flo;
    PhotomapPtr	 lut = NULL;
    ExpCtxPtr	 exp = Eport_(img);
    double src_lvls, dst_lvls;
    CARD32 c, scnstr, padmsk, pxlstr, max_lvls, lvl[XieK_MaxComponents];
    CARD32 w, h, format, mapping, lut_width, level, val, off, mask;
    CARD8 *base;

    REQUEST( xieExportReq );

    LookupDrawableAndGC_(client, stuff->drawable_id, stuff->gc_id);
    if( exp != NULL )
	{   /*
	    **	See if we can re-use the existing ExportContext.
	    */
	AccessOkIf_( !eBusy_(exp), ResId_(img) );
	if( eCmapId_(exp) )
	    LookupColormap_( eCmap_(exp), eCmapId_(exp) );
	if( !IsBitonal_(img) &&		/* continuous-tone AND different... */
	  ( client->lastDrawable->depth != eDepth_(exp)	    /*     depth    */
	 || stuff->photo_lut_id != 0			    /* OR: LUT	    */
	 && stuff->photo_lut_id != (eLutMap_(exp) ? ResId_(eLutMap_(exp)) : 0)
	 || stuff->colormap_id  != 0			    /* OR: Colormap */
	 && stuff->colormap_id  != (eCmap_(exp)   ? eCmap_(exp)->mid      : 0)))
	    FreeExport( img );
	}
    if( Eport_(img) == NULL )
	{
        Eport_(img) = (ExpCtxPtr) DdxCalloc_(1, sizeof(ExpCtxRec));
	AllocOkIf_(Eport_(img), ResId_(img));
        exp = Eport_(img);
	eClient_(exp) = client;
	eDepth_(exp)  = IsBitonal_(img) ? 1 : eClientDrw_(exp)->depth;
	eRGBPol_(exp) = PxlPol_(img) == XieK_ZeroDark ? 0 : ~0;
	for( max_lvls = 0, src_lvls = 1.0, c = 0; c < CmpCnt_(img); c++ )
	    {	/*
		**  Initialize format conversion parameters.
		*/
	    eMul_(exp,c) = c == 0 ? 1 : eMul_(exp,c-1) * eLvl_(exp,c-1);
	    eLen_(exp,c) = BitsFromLevels_(uLvl_(img,c));
	    eLvl_(exp,c) = uLvl_(img,c);
	    src_lvls	*= eLvl_(exp,c);
	    max_lvls	 = max(max_lvls, eLvl_(exp,c));
	    }
	if( stuff->photo_lut_id == 0 )
	    MatchOkIf_( src_lvls <= 1<<eDepth_(exp), ResId_(img));

	if( !IsBitonal_(img) && stuff->photo_lut_id )
	    {
	    LookupPhotos_( &flo, &lut, stuff->photo_lut_id, -1 );
	    BePCM_( lut, stuff->photo_lut_id );
	    eLutMap_(exp) = lut;
	    RefCnt_(lut)++;
	    AccessOkIf_( FloLnk_(img) == flo, ResId_(lut) );
	    if( IsRGB_(lut) )
		{   /*
		    **  Ok if client LUT is compatible with image and drawable.
		    */
		for( dst_lvls = 1.0, c = 0; c < XieK_MaxComponents; c++ )
		    {
		    MatchOkIf_( uLvl_(img,c) <= uWidth_(lut,c), ResId_(lut) );
		    dst_lvls *= uLvl_(lut,c);
		    eLen_(exp,c) = BitsFromLevels_(uLvl_(lut,c));
		    }
		MatchOkIf_( dst_lvls <= 1<<eDepth_(exp), ResId_(lut) );
		}
	    else
		MatchOkIf_( uWidth_(lut,0) >= src_lvls
			   && uLvl_(lut,0) <= 1<<eDepth_(exp), ResId_(lut) );
	    if( stuff->colormap_id )
		{   /*
		    **	Ok if Colormap and LUT are compatible.
		    */
		LookupColormap_( eCmap_(exp), stuff->colormap_id );
		ColorOkIf_(eCmap_(exp), stuff->colormap_id);
		eCmapId_(exp) = stuff->colormap_id;
		switch( eClass_(exp) )
		    {
		case StaticColor :
		case DirectColor :
		case TrueColor   :
		    lvl[0] = eRedMsk_(exp) >> eRedOff_(exp);
		    lvl[1] = eGrnMsk_(exp) >> eGrnOff_(exp);
		    lvl[2] = eBluMsk_(exp) >> eBluOff_(exp);
		    if( lvl[0]++ & lvl[1]++ & lvl[2]++ )
			{
			MatchOkIf_(IsRGB_(lut) || IsGrayScale_(img)
					       && eStaticCmap_(exp)
					       && lvl[0]==lvl[1]
					       && lvl[1]==lvl[2], ResId_(lut));
			eMasks_(exp) = TRUE;
			break;
			}    /* else StaticColor with no masks -- fall thru */
		default :
		    MatchOkIf_( !IsRGB_(lut), ResId_(lut) );
		    lvl[0] = 1<<eDepth_(exp);
		    }
		for( c = 0; c < CmpCnt_(lut); c++ )
		    MatchOkIf_( lvl[c] == uLvl_(lut,c), ResId_(lut) );
		}
	    ePreFmt_(exp) = IsRGB_(img) && !IsRGB_(lut);
	    }

	else if( !IsBitonal_(img) && stuff->colormap_id )
	    {
	    LookupColormap_( eCmap_(exp), stuff->colormap_id );
	    ColorOkIf_(eCmap_(exp), stuff->colormap_id);
	    MatchOkIf_(eClass_(exp) > GrayScale || !IsRGB_(img), ResId_(img));
	    eCmapId_(exp) = stuff->colormap_id;
	    eMchLmt_(exp) = MiDecodeDouble( &stuff->match_limit );
	    eGryLmt_(exp) = MiDecodeDouble( &stuff->gray_limit  );
	    switch( eClass_(exp) )
		{
	    case DirectColor :
		eRGBCnt_(exp) = max_lvls;
	    case TrueColor   :
	    case StaticColor :
		lvl[0] = eRedMsk_(exp) >> eRedOff_(exp);
		lvl[1] = eGrnMsk_(exp) >> eGrnOff_(exp);
		lvl[2] = eBluMsk_(exp) >> eBluOff_(exp);
		if( lvl[0]++ & lvl[1]++ & lvl[2]++ )
		    {
		    for( dst_lvls = 1.0, c = 0; c < XieK_MaxComponents; c++ )
			{   /*
			    **  Ok if Photomap levels are compatible with masks.
			    */
			eLen_(exp,c) = BitsFromLevels_(lvl[c]);
			eMul_(exp,c) = 1 << ( c == 0 ? eRedOff_(exp)
					    : c == 1 ? eGrnOff_(exp)
						     : eBluOff_(exp) );
			if( eLvl_(exp,c) )
			    {
			    MatchOkIf_( eLvl_(exp,c) <= lvl[c], ResId_(img) );
			    dst_lvls *= lvl[c];
			    }
			else
			    MatchOkIf_( eDynamicCmap_(exp)
				     && lvl[c] >= eLvl_(exp,0)
				     || lvl[c] == lvl[0], ResId_(img) );
			}
		    eMasks_(exp) = TRUE;
		    lut_width	 = max_lvls;
		    mapping	 = eDynamicCmap_(exp) ? XieK_RGB : CmpMap_(img);
		    break;
		    }	    /* else StaticColor with no masks -- fall thru */
	    case PseudoColor :
	    case GrayScale   :
		eRGBCnt_(exp) = src_lvls;
	    case StaticGray  :
		dst_lvls      = 1<<eDepth_(exp);
		lvl[0]	      = dst_lvls;
		lut_width     = src_lvls;
		mapping	      = XieK_GrayScale;
		ePreFmt_(exp) = IsRGB_(img);
		    /*
		    **  Ok if total levels fit within visual's depth.
		    */
		MatchOkIf_( src_lvls <= dst_lvls, ResId_(img));
		}

	    /*
	    **  How wide a LUT might we need to create (for Step E) ?
	    */
	    lut_width = eRGBCnt_(exp) > 0 ? eRGBCnt_(exp)
		      : eRGBPol_(exp) || src_lvls < dst_lvls ? lut_width : 0;
	    if( lut_width > 0 )
		{   /*
		    **	Create LUT to re-map image to Colormap pixel indices.
		    */
		lut = UtilCreatePhotomap( mapping, lvl, 1.0, 0, 0, 0,
					  lut_width, 1, TRUE, TRUE, NULL );
		AllocOkIf_( lut, ResId_(img) );
		eLutMap_(exp) = lut;
		if( lut_width > eRGBCnt_(exp) )
		    /*
		    **  Build a LUT to match pixels to static Colormap.
		    */
		    for( c = 0; c < CmpCnt_(lut); c++ )
			{
			mask =  uLvl_(lut,c) - 1;
			base = uBase_(lut,c);
			memset( base, 0, uArSize_(lut,c)+7>>3 );
			for( level = 0; level < eLvl_(exp,c); level++ )
			    {
			    off = level*uPxlStr_(lut,c);
			    val = level*mask/(eLvl_(exp,c)-1)^eRGBPol_(exp);
			    PUT_VALUE_(base, off, val, mask);
			    }
			}
		}
	    if( eRGBCnt_(exp) > 0 )
		{   /*
		    **  Allocate pixel allocation/match structures (for Step D).
		    */
		ePxlLst_(exp) = (CARD32*) DdxCalloc_( eRGBCnt_(exp),
						      sizeof(CARD32) );
		AllocOkIf_(ePxlLst_(exp),ResId_(img));
		eRGBLst_(exp) = (RGBPtr)  DdxCalloc_( eRGBCnt_(exp),
						      sizeof(RGBRec) );
		AllocOkIf_(eRGBLst_(exp),ResId_(img));
		eMapLst_(exp) = (RGBPtr)  DdxCalloc_( eMapLen_(exp),
						      sizeof(RGBRec) );
		AllocOkIf_(eMapLst_(exp),ResId_(img));

		for( c = 0; c < CmpCnt_(lut); c++ )
		    {	/*
			**  Allocate histogram space (for Step C).
			*/
		    eHst_(exp,c) = (CARD32 *) DdxCalloc_( eRGBCnt_(exp),
							 sizeof(CARD32));
		    AllocOkIf_( eHst_(exp,c), ResId_(img) );
		    }
		eHstAll_(exp) = IsPhotomap_(img) && (W <  Width_(img)
						 ||  H < Height_(img));
		}
	    }
	}

    /*
    **	The Drawable and GC are allowed to change between Export calls.
    */
    eDrwId_(exp) = stuff->drawable_id;
    eGCId_(exp)	 = stuff->gc_id;
    /*
    **	Find drawable's pixel and scanline requirements for exporting our image.
    */
    for( format = 0; format < screenInfo.numPixmapFormats
	   && eDepth_(exp) != screenInfo.formats[format].depth; format++ );

    pxlstr = IsBitonal_(img) ? 1 : screenInfo.formats[format].bitsPerPixel;
    padmsk = screenInfo.formats[format].scanlinePad - 1;

    if( ePreFmt_(exp) )
	{   /*
	    **  Allocate a Udp for ZPixmap conversion (for Step B).
	    */
	scnstr = Width_(img) * pxlstr + padmsk & ~padmsk;
	ePreUdp_(exp) = UtilAllocUdp(
			   UtilDType(eDepth_(exp), XieK_PCM, pxlstr, scnstr),
				     XieK_PCM, Width_(img), Height_(img),
				     pxlstr, scnstr,
				     1<<eDepth_(exp), 0, FALSE, 0, NULL, NULL );
	AllocOkIf_( ePreUdp_(exp), ResId_(img) );
	}
    /*
    **  Allocate a Udp in which to create the finished ZPixmap masterpiece.
    */
    w = eClientDrw_(exp)->type == DRAWABLE_WINDOW
				? min(eClientDrw_(exp)->pScreen->width,  W) : W;
    h = eClientDrw_(exp)->type == DRAWABLE_WINDOW
				? min(eClientDrw_(exp)->pScreen->height, H) : H;
    scnstr = w * pxlstr + padmsk & ~padmsk;
    ePixUdp_(exp) = UtilAllocUdp(UtilDType(eDepth_(exp),XieK_PCM,pxlstr,scnstr),
				 XieK_PCM, w, h, pxlstr, scnstr,
				 1<<eDepth_(exp), 0, FALSE, 0, NULL, NULL );
    AllocOkIf_( ePixUdp_(exp), ResId_(img) );
    upX1_(ePixUdp_(exp))  = X;
    upX2_(ePixUdp_(exp)) += X;
    upY1_(ePixUdp_(exp))  = Y;
    upY2_(ePixUdp_(exp)) += Y;
    eBusy_(exp)		  = TRUE;

    return( Success );
}				/* end CreateExportContext */

/*-----------------------------------------------------------------------
------ routine:  Convert bit stream image data to ZPixmap Format --------
------------------------------------------------------------------------*/
static int CvtRGBBitZPixmap( snk, dst_udp, mul )
 PipeSinkPtr snk;
 UdpPtr	     dst_udp;
 CARD32	     mul[];
{
    UdpPtr  red_udp  = SnkUdpPtr_(snk,0),
	    grn_udp  = SnkUdpPtr_(snk,1),
	    blu_udp  = SnkUdpPtr_(snk,2);
    CARD8  *red_base = upBase_(red_udp),
	   *grn_base = upBase_(grn_udp),
	   *blu_base = upBase_(blu_udp),
	   *dst_base;
    CARD32  red_msk  = ( 1 << upPxlLen_(red_udp)) - 1,
	    grn_msk  = ( 1 << upPxlLen_(grn_udp)) - 1,
	    blu_msk  = ( 1 << upPxlLen_(blu_udp)) - 1,
	    dst_msk  = ( 1 << upPxlStr_(dst_udp)) - 1;
    CARD32  red_val, grn_val, blu_val, dst_val;
    INT32   red_off, grn_off, blu_off, dst_off;
    INT32   red_pxl, grn_pxl, blu_pxl, dst_pxl;
    INT32   pix, scn, x1, y1, x2, y2,  lg2 = upPxlStr_(dst_udp) ==  8 ? 3
					   : upPxlStr_(dst_udp) == 16 ? 4
					   : upPxlStr_(dst_udp) == 32 ? 5 : 0;
    if( upBase_(dst_udp) == NULL )
	upBase_(dst_udp)  = (CARD8 *)DdxMallocBits_(upArSize_(dst_udp));
    if( upBase_(dst_udp) == NULL ) return( BadAlloc );
    dst_base = upBase_(dst_udp);
    x1	     = max(upX1_(red_udp), upX1_(dst_udp));
    x2	     = min(upX2_(red_udp), upX2_(dst_udp));
    y1	     = max(upY1_(red_udp), upY1_(dst_udp));
    y2	     = min(upY2_(red_udp), upY2_(dst_udp));
    red_off  = upPos_(red_udp)+upScnStr_(red_udp)*(y1-upY1_(red_udp))
			      +upPxlStr_(red_udp)*(x1-upX1_(red_udp));
    grn_off  = upPos_(grn_udp)+upScnStr_(grn_udp)*(y1-upY1_(grn_udp))
			      +upPxlStr_(grn_udp)*(x1-upX1_(grn_udp));
    blu_off  = upPos_(blu_udp)+upScnStr_(blu_udp)*(y1-upY1_(blu_udp))
			      +upPxlStr_(blu_udp)*(x1-upX1_(blu_udp));
    dst_off  = upPos_(dst_udp)+upScnStr_(dst_udp)*(y1-upY1_(dst_udp))
			      +upPxlStr_(dst_udp)*(x1-upX1_(dst_udp))>>lg2;
    for( scn = y1; scn <= y2; scn++ )
	{
	red_pxl = red_off;
	grn_pxl = grn_off;
	blu_pxl = blu_off;
	dst_pxl = dst_off;
	switch( lg2 )
	    {
	case 3 :
	    for( pix = x1; pix <= x2; pix++ )
		{
		red_val = GET_VALUE_(red_base,red_pxl,red_msk);
		grn_val = GET_VALUE_(grn_base,grn_pxl,grn_msk);
		blu_val = GET_VALUE_(blu_base,blu_pxl,blu_msk);
		dst_val = mul[0]*red_val + mul[1]*grn_val + mul[2]*blu_val;
		dst_base[dst_pxl++] = dst_val;
		red_pxl += upPxlStr_(red_udp);
		grn_pxl += upPxlStr_(grn_udp);
		blu_pxl += upPxlStr_(blu_udp);
		}
	    break;
	 case 4 :
	    for( pix = x1; pix <= x2; pix++ )
	       {
		red_val = GET_VALUE_(red_base,red_pxl,red_msk);
		grn_val = GET_VALUE_(grn_base,grn_pxl,grn_msk);
		blu_val = GET_VALUE_(blu_base,blu_pxl,blu_msk);
		dst_val = mul[0]*red_val + mul[1]*grn_val + mul[2]*blu_val;
	       ((CARD16 *)dst_base)[dst_pxl++] = dst_val;
		red_pxl += upPxlStr_(red_udp);
		grn_pxl += upPxlStr_(grn_udp);
		blu_pxl += upPxlStr_(blu_udp);
	       }
	    break;	    
	 case 5 :
	    for( pix = x1; pix <= x2; pix++ )
	       {
		red_val = GET_VALUE_(red_base,red_pxl,red_msk);
		grn_val = GET_VALUE_(grn_base,grn_pxl,grn_msk);
		blu_val = GET_VALUE_(blu_base,blu_pxl,blu_msk);
		dst_val = mul[0]*red_val + mul[1]*grn_val + mul[2]*blu_val;
	       ((CARD32 *)dst_base)[dst_pxl++] = dst_val;
		red_pxl += upPxlStr_(red_udp);
		grn_pxl += upPxlStr_(grn_udp);
		blu_pxl += upPxlStr_(blu_udp);
	       }
	    break;	    
	case 0 :
	    for( pix = x1; pix <= x2; pix++ )
		{
		red_val = GET_VALUE_(red_base,red_pxl,red_msk);
		grn_val = GET_VALUE_(grn_base,grn_pxl,grn_msk);
		blu_val = GET_VALUE_(blu_base,blu_pxl,blu_msk);
		dst_val = mul[0]*red_val + mul[1]*grn_val + mul[2]*blu_val;
		PUT_VALUE_( dst_base, dst_pxl, dst_val, dst_msk );
		red_pxl += upPxlStr_(red_udp);
		grn_pxl += upPxlStr_(grn_udp);
		blu_pxl += upPxlStr_(blu_udp);
		dst_pxl += upPxlStr_(dst_udp);
		}
	    break;
	    }
	red_off += upScnStr_(red_udp);
	grn_off += upScnStr_(grn_udp);
	blu_off += upScnStr_(blu_udp);
	dst_off += upScnStr_(dst_udp)>>lg2;
	}
    return( Success );
}				    /* end CvtRGBBitZPixmap */

/*-----------------------------------------------------------------------
---------- routine:  Convert byte image data to ZPixmap Format ----------
------------------------------------------------------------------------*/
static int CvtRGBBytZPixmap( snk, dst_udp, mul )
 PipeSinkPtr snk;
 UdpPtr	     dst_udp;
 CARD32	     mul[];
{
    UdpPtr  red_udp  = SnkUdpPtr_(snk,0),
	    grn_udp  = SnkUdpPtr_(snk,1),
	    blu_udp  = SnkUdpPtr_(snk,2);
    CARD8  *red_base = upBase_(red_udp),
	   *grn_base = upBase_(grn_udp),
	   *blu_base = upBase_(blu_udp),
	   *dst_base;
    CARD32  red_msk  = (1<<BitsFromLevels_(upLvl_(red_udp)))-1,
	    grn_msk  = (1<<BitsFromLevels_(upLvl_(grn_udp)))-1,
	    blu_msk  = (1<<BitsFromLevels_(upLvl_(blu_udp)))-1,
	    dst_msk  = (1<<upPxlStr_(dst_udp))-1;
    INT32   red_off, grn_off, blu_off, dst_off, dst_val;
    INT32   red_pxl, grn_pxl, blu_pxl, dst_pxl;
    INT32   pix, scn, x1, y1, x2, y2,  lg2 = upPxlStr_(dst_udp) ==  8 ? 3
					   : upPxlStr_(dst_udp) == 16 ? 4
					   : upPxlStr_(dst_udp) == 32 ? 5 : 0;
    if( upBase_(dst_udp) == NULL )
	upBase_(dst_udp)  = (CARD8 *)DdxMallocBits_(upArSize_(dst_udp));
    if( upBase_(dst_udp) == NULL ) return( BadAlloc );
    dst_base = upBase_(dst_udp);

    x1 = max(upX1_(red_udp), upX1_(dst_udp));
    x2 = min(upX2_(red_udp), upX2_(dst_udp));
    y1 = max(upY1_(red_udp), upY1_(dst_udp));
    y2 = min(upY2_(red_udp), upY2_(dst_udp));

    red_off = upPos_(red_udp)+upScnStr_(red_udp)*(y1-upY1_(red_udp))
			     +upPxlStr_(red_udp)*(x1-upX1_(red_udp))>>3;
    grn_off = upPos_(grn_udp)+upScnStr_(grn_udp)*(y1-upY1_(grn_udp))
			     +upPxlStr_(grn_udp)*(x1-upX1_(grn_udp))>>3;
    blu_off = upPos_(blu_udp)+upScnStr_(blu_udp)*(y1-upY1_(blu_udp))
			     +upPxlStr_(blu_udp)*(x1-upX1_(blu_udp))>>3;
    dst_off = upPos_(dst_udp)+upScnStr_(dst_udp)*(y1-upY1_(dst_udp))
			     +upPxlStr_(dst_udp)*(x1-upX1_(dst_udp))>>lg2;

    for( scn = y1; scn <= y2; scn++ )
	{
	red_pxl = red_off;
	grn_pxl = grn_off;
	blu_pxl = blu_off;
	dst_pxl = dst_off;

	switch( lg2 )
	    {
	case 3 :
	    for( pix = x1; pix <= x2; pix++ )
		dst_base[dst_pxl++] = mul[0] * (red_base[red_pxl++] & red_msk)
				    + mul[1] * (grn_base[grn_pxl++] & grn_msk)
				    + mul[2] * (blu_base[blu_pxl++] & blu_msk);
	    break;
	case 4 :
	    for( pix = x1; pix <= x2; pix++ )
	       ((CARD16 *)dst_base)[dst_pxl++] =
				      mul[0] * (red_base[red_pxl++] & red_msk)
				    + mul[1] * (grn_base[grn_pxl++] & grn_msk)
				    + mul[2] * (blu_base[blu_pxl++] & blu_msk);
	    break;
	case 5 :
	    for( pix = x1; pix <= x2; pix++ )
	       ((CARD32 *)dst_base)[dst_pxl++] =
				      mul[0] * (red_base[red_pxl++] & red_msk)
				    + mul[1] * (grn_base[grn_pxl++] & grn_msk)
				    + mul[2] * (blu_base[blu_pxl++] & blu_msk);
	    break;
	case 0 :
	    for( pix = x1; pix <= x2; pix++ )
		{
		dst_val = mul[0] * (red_base[red_pxl++] & red_msk)
			+ mul[1] * (grn_base[grn_pxl++] & grn_msk)
			+ mul[2] * (blu_base[blu_pxl++] & blu_msk);
		PUT_VALUE_( dst_base, dst_pxl, dst_val, dst_msk );
		dst_pxl += upPxlStr_(dst_udp);
		}
	    break;
	    }
	red_off += upScnStr_(red_udp)>>3;
	grn_off += upScnStr_(grn_udp)>>3;
	blu_off += upScnStr_(blu_udp)>>3;
	dst_off += upScnStr_(dst_udp)>>lg2;
	}
    return( Success );
}				    /* end CvtRGBBytZPixmap */

#if defined(DWV3)
/*-----------------------------------------------------------------------
------- routine:  Draw changelist image data using core X spans  --------
------------------------------------------------------------------------*/
static void ExportSpans( exp, datudp, pix, win_x, win_y)
    ExpCtxPtr	exp;			/* Pointer to export context */
    UdpPtr	datudp;			/* Pointer to Udp for changelist  */
    UdpPtr	pix;			/* Export region UDP */
    CARD32      win_x;			/* Window X coord for first pixel */
    CARD32      win_y;			/* Window Y coord for first pixel */
{
    CARD32	     index;		/* index into changelist */
    CARD32	     x1, x2;		/* Adjusted limits of export region */
    CARD32	     X1, X2;		/* Limits of current span */
    CARD32	     scanline;		/* Scanline within the data segment */
    unsigned int    *cl;		/* Current changelist entry */
    int		     clpos;		/* Pointer to current changelist */
    int		     clcnt;		/* Count of change list entries in  */
					/* this scanline		    */
    int		     spncnt;		/* Number of spans remaining in	    */
					/* list				    */

    DDXPointPtr	     pt;		/* Pointer to span points buffer  */
    CARD32	    *ptw;		/* Pointer to span width buffer   */

    GCPtr	     spanGC = (eSpnGC_(exp) == NULL) ? eClientGC_(exp) :
						       eSpnGC_(exp);
    /*
    **	Get pointer into change list.
    */
    clpos = DdxPosCl_(upBase_(datudp),upPos_(datudp),0);
    /*
    **	Get pointers into span list and count of remaining entries.
    */
    pt     = eSpnPt_(exp) + eSpnIdx_(exp);
    ptw    = eSpnW_(exp)  + eSpnIdx_(exp);
    spncnt = eSpnCnt_(exp) - eSpnIdx_(exp);
    /*
    **	Compute horizontal bounds
    */
    x1 = upX1_(pix) - upX1_(datudp);
    x2 = upX2_(pix) - upX1_(datudp);
    /*
    **	Process each scanline of the input changelist.
    */
    for (scanline = 0; scanline < upHeight_(pix); scanline++) 
    {
	cl = upBase_(datudp) + (clpos >> 3);
					/* Start at top of current list */
	clcnt = *cl++;			/* Count of entries in change list */

	for (index = 0; index < clcnt; index += 2 ) 
	{
	    if (*cl > x2) break;	/* Span begins beyond right edge */

	    X1 = *cl++;			/* X1 = max(*cl++, x1) */
	    if (x1 > X1) X1 = x1;
	    X2 = *cl++ - 1;		/* X2 = min(*cl++, x2) */
	    if (x2 < X2) X2 = x2;

	    if( X2 >= X1 )
	    {
		pt->x = win_x + (X1 - x1);
		(pt++)->y = win_y + scanline;
		*ptw++ = X2 - X1 + 1;
		/*
		**  If span buffer full, draw them...
		*/
                if (--spncnt <= 0)
                {
                    FillSpans_(eClientDrw_(exp),
			       spanGC,
			       eSpnCnt_(exp),
			       eSpnPt_(exp),
			       eSpnW_(exp));

		    pt = eSpnPt_(exp);
		    ptw = eSpnW_(exp);
		    spncnt = eSpnCnt_(exp);
                }
	    }
	}
	clpos = DdxPosCl_(upBase_(datudp),clpos,1);
    }
    eSpnIdx_(exp) = eSpnCnt_(exp) - spncnt;
}				    /* ExportSpans */
#endif

/*-----------------------------------------------------------------------
--------------- routine:  Convert image data to ZPixmap Format ----------
------------------------------------------------------------------------*/
static int CvtZPixmap( exp, snk, pix )
 ExpCtxPtr	exp;
 PipeSinkPtr	snk;
 UdpPtr		pix;
{
    UdpRec dst;
    CARD32 c, status = Success;

    if( eLvl_(exp,1) > 0 )
	/*
	**  Convert region of RGB image to ZPixmap format.
	*/
	if( SnkDatTyp_(snk,0) == UdpK_DTypeBU )
	    status = CvtRGBBytZPixmap( snk, pix, &eMul_(exp,0) );
	else
	    status = CvtRGBBitZPixmap( snk, pix, &eMul_(exp,0) );

    else
    {
#ifndef BIGENDIAN   /* BIGENDIANEVILHACK */
/* Can't apply this optimization on big-endian machines since we must bit
 * swap the copy of the data we give to the core server.
 */
	if( upWidth_(pix)  == SnkWidth_(snk,0)
	  && upHeight_(pix) <= SnkHeight_(snk,0)
	  && upPxlStr_(pix) == SnkPxlStr_(snk,0)
	  && upScnStr_(pix) == SnkScnStr_(snk,0)
	  && upLvl_(pix)    == SnkLvl_(snk,0)
	  && upX1_(pix)     == SnkX1_(snk,0) )
	{   /*
	    **	Use monochrome image directly from the Sink Udp buffer.
	    */
	upBase_(pix)   = SnkBase_(snk,0);
	upArSize_(pix) = SnkArSize_(snk,0);
	upPos_(pix)    = SnkScnStr_(snk,0) * (upY1_(pix) - SnkY1_(snk,0))
							 + SnkPos_(snk,0);
        } else
#endif /* BIGENDIAN */
	{
	    /*
	     **  Convert region of monochrome image.
	     */
	    status = DdxConvert_( SnkUdpPtr_(snk,0), pix, XieK_MoveMode );
	    if( eMasks_(exp) )
		/*
		 **  Copy GrayScale data to green and blue components.
		 */
		for(dst= *pix, c=1 ; c < XieK_MaxComponents &&
		    status==Success; c++)
		{
		    SetupUdp(&dst, SnkLvl_(snk,0), eLen_(exp,c), c);
		    urPos_(dst) += eLen_(exp,c-1);
		    status = DdxConvert_(SnkUdpPtr_(snk,0), &dst,
					 XieK_MoveMode);
		}
	}
#ifdef BIGENDIAN /* BIGENDIANEVILHACK */
	if( status == Success )
	{
	    unsigned char *p;
	    unsigned char *endp = upBase_(pix)+(7+upArSize_(pix))/8+1;
	    for(p = upBase_(pix); p < endp; ++p) 
		*p = _reverse_byte[*p];
	}
#endif
    }
    return( status );
}				    /* end CvtZPixmap */

/*-----------------------------------------------------------------------
-------- routine: Export formatted Photo{map|tap} data to drawable ------
------------------------------------------------------------------------*/
static int Export( exp, snk, pix, x, y )
 ExpCtxPtr	exp;
 PipeSinkPtr	snk;
 UdpPtr		pix;
 INT32		x,y;
{
    PhotomapPtr	lut = eLutMap_(exp);
    UdpRec src, dst;
    CARD32 sc,  dc, status = Success;

#if defined(DWV3)
    if (SnkDatTyp_(snk, 0) != UdpK_DTypeCL) {
#endif
    if( lut != NULL )
	{
	if( upBase_(pix) == NULL &&
	  ( upBase_(pix)  = (CARD8 *)DdxMallocBits_(upArSize_(pix)) ) == NULL )
		return( BadAlloc );
	/*
	**  Step E: re-map image pixels to LUT (Colormap) pixel indices.
	*/
	if( IsRGB_(lut) )
	    for( src = *pix, dst = *pix, dc = 0; dc < CmpCnt_(lut); dc++ )
		{   /*
		    **	Set up source Udp.
		    */
		sc = eLvl_(exp,dc) ? dc : 0;
		if( ePreFmt_(exp) )
		    SetupUdp( &src, eLvl_(exp,sc), eLen_(exp,sc), sc );
		else
		    src = SnkUdp_(snk,sc);
		    /*
		    **	Set up destination Udp.
		    */
		SetupUdp( &dst, uLvl_(lut,dc), eLen_(exp,dc), dc );
		if( eMasks_(exp) )
		    urPos_(dst) = dc == 0 ? eRedOff_(exp)
				: dc == 1 ? eGrnOff_(exp) : eBluOff_(exp);

		status = DdxPoint_( &src, &dst, NULL, Udp_(lut,dc) );
		if( status != Success ) return( status );
		if( !eMasks_(exp) )
		    urPos_(dst) += urPxlLen_(dst);
		if( ePreFmt_(exp) && eLvl_(exp,dc) )
		    urPos_(src) += urPxlLen_(src);
		}

	else
	    {	/*
		**  Choose the source Udp -- then do it.
		*/
	    src = ePreFmt_(exp) ?
		  eHstAll_(exp) ? *ePreUdp_(exp) : *pix : SnkUdp_(snk,0);

	    status = DdxPoint_( &src, pix, NULL, Udp_(lut,0) );
	    if( status != Success ) return( status );

	    if( eMasks_(exp) )
		{   /*
		    **  Copy GrayScale data to green and blue components.
		    */
		src = *pix;
		dst = *pix;
		SetupUdp(&src, uLvl_(lut,0), eLen_(exp,0), 0);
		for( dc = 1; dc < XieK_MaxComponents; dc++ )
		    {
		    SetupUdp(&dst, uLvl_(lut,0), eLen_(exp,dc), dc);
		    urPos_(dst) += eLen_(exp,dc-1);
		    status = DdxConvert_( &src, &dst, XieK_MoveMode );
		    if( status != Success ) return( status );
 		    }
		}
	    }
	}
    else
	{   /*
	    **  Step F: post-format as ZPixmap image data.
	    */
	status = CvtZPixmap( exp, snk, pix );
	if( status != Success ) return( status );
	}
    /*
    **	Step G: export our finished masterpiece to the drawable.
    */
    PutImage_( eClientDrw_(exp), eClientGC_(exp),	/* Drawable & GC    */
	       eDepth_(exp),				/* Drawable depth   */
	       x, y, upWidth_(pix), upHeight_(pix),	/* Drawable region  */
	       upPos_(pix) & 31,			/* data left pad    */
	       eDepth_(exp) > 1 ? ZPixmap :		/* data format	    */
	       eClientDrw_(exp)->type == DRAWABLE_PIXMAP ? XYPixmap : XYBitmap,
	       upBase_(pix) + (upPos_(pix) >> 3 & ~3));	/* data address	    */
    if( upBase_(pix) == SnkBase_(snk,0) )
	upBase_(pix)  = NULL;

#if defined(DWV3)
    } else {
	/*
	 * Export changelist data using core X FillSpan operation.
	 */
	ExportSpans(exp, SnkUdpPtr_(snk, 0), pix, x, y);
    }		        
#endif

    return( Success );
}				/* end Export */

/*-----------------------------------------------------------------------
----------------  routine: free temporary Export resources --------------
------------------------------------------------------------------------*/
static void ExportDone( exp )
 ExpCtxPtr exp;
{
    CARD32   c;

    for( c = 0; c < XieK_MaxComponents && eHst_(exp,c); c++ )
	eHst_(exp,c) = (CARD32 *) DdxFree_( eHst_(exp,c) );

    eMapLst_(exp) = (RGBPtr) DdxFree_( eMapLst_(exp) );
    eRGBLst_(exp) = (RGBPtr) DdxFree_( eRGBLst_(exp) );
    ePreUdp_(exp) = UtilFreeUdp( ePreUdp_(exp) );
    ePixUdp_(exp) = UtilFreeUdp( ePixUdp_(exp) );

    eHstAll_(exp) = FALSE;
    eBusy_(exp)   = FALSE;

#if defined(DWV3)
    /* Free the span buffers and span GC, if used */
    if( eSpnW_(exp) != NULL )
	eSpnW_(exp) = (CARD32 *) DdxFree_(eSpnW_(exp));

    if( eSpnPt_(exp) != NULL )
	eSpnPt_(exp) = (DDXPointPtr)DdxFree_(eSpnPt_(exp));

    if( eSpnGC_(exp) != NULL  )
	{
	FreeScratchGC(eSpnGC_(exp));
	eSpnGC_(exp) = NULL;
	}
#endif
}				/* end ExportDone */

/*-----------------------------------------------------------------------
---------- routine: Format Photo{map|tap} for export to drawable --------
------------------------------------------------------------------------*/
static int Format( exp, snk )
 ExpCtxPtr	exp;
 PipeSinkPtr	snk;
{
    INT32   status = Success;

    if( ePreFmt_(exp) )
	/*
	**  Step B: pre-format as ZPixmap image data.
	*/
	status  = CvtZPixmap( exp, snk, eHstAll_(exp) ? ePreUdp_(exp)
						      : ePixUdp_(exp) );
    if( status == Success && eHst_(exp,0) != NULL )
	/*
	**  Step C: create and analyze histogram.
	**  Step D: allocate and/or match colors, then fill in the LUT.
	*/
	switch( eClass_(exp) )
	    {
	case GrayScale   :
	case StaticColor :
	case PseudoColor :
	    FormatPseudoColor( exp, snk );
	    break;

	case DirectColor :
	    FormatDirectColor( exp, snk );
	    break;
	    }
    return( status );
}				/* end Format */

/*-----------------------------------------------------------------------
---------------------- routine: Format for DirectColor ------------------
------------------------------------------------------------------------*/
static void FormatDirectColor( exp, snk )
 ExpCtxPtr	exp;
 PipeSinkPtr	snk;
{
    UdpPtr src, roi;
    RGBRec state, *rgb;
    CARD32 status, c, i, maxCnt, cnt[XieK_MaxComponents];

    for( maxCnt = 0, c = 0; c < XieK_MaxComponents && eLvl_(exp,c); c++ )
	{   /*
	    **  Step C: create histogram of individual component usage.
	    */
	src = SnkUdpPtr_(snk,c);
	roi = eHstAll_(exp) ? src : ePixUdp_(exp);
	Histogram( src, roi, eHst_(exp,c) );
	cnt[c] = HistogramAnalysis( exp, c );
	maxCnt = max(maxCnt, cnt[c]);
	memset( uBase_(eLutMap_(exp),c), 0, uArSize_(eLutMap_(exp),c)+7>>3 );
	}
    for( eRGBCnt_(exp) = maxCnt; c < XieK_MaxComponents; c++ )
	{
	cnt[c] = cnt[0];
	memset( uBase_(eLutMap_(exp),c), 0, uArSize_(eLutMap_(exp),c)+7>>3 );
	}
    ReadTripleMap( exp, &state, cnt );

    for( rgb = eRGBLst_(exp), i = 0; i < maxCnt; rgb++, i++ )
	{
	rgb->red = i < cnt[0] ? SetDirectColor_(exp,eHst_(exp,0)[i],0,rgb)
			      : state.red;
	rgb->grn = i < cnt[1] ? SetDirectColor_(exp,eHst_(exp,1)[i],1,rgb)
			      : state.grn;
	rgb->blu = i < cnt[2] ? SetDirectColor_(exp,eHst_(exp,2)[i],2,rgb)
			      : state.blu;
	}
    for( rgb = eRGBLst_(exp), i = 0; i < maxCnt; rgb++, i++ )
	{   /*
	    **  Step D: allocate pixels.
	    */
	rgb->pixel = ~0;
	if( MatchDirectAllocColor( exp, &state, rgb ) != Success )
	    continue;

	ePxlLst_(exp)[ePxlCnt_(exp)++] = rgb->pixel;
	if( i < cnt[0] )
	    SetLutEntry(exp, (CARD32)(rgb->pixel >> eRedOff_(exp)), 0, i);
	if( i < cnt[1] )
	    SetLutEntry(exp, (CARD32)(rgb->pixel >> eGrnOff_(exp)), 1, i);
	if( i < cnt[2] )
	    SetLutEntry(exp, (CARD32)(rgb->pixel >> eBluOff_(exp)), 2, i);
	}
}				/* end FormatDirectColor */

/*-----------------------------------------------------------------------
---------------------- routine: Format for PseudoColor ------------------
------------------------------------------------------------------------*/
static void FormatPseudoColor( exp, snk )
 ExpCtxPtr	exp;
 PipeSinkPtr	snk;
{
    UdpPtr src, roi;
    CARD32 i, state;
    double range = 65535.0 * 65535.0 * 3.0;
    double limit = eMchLmt_(exp) * range;

    /*
    **  Step C: create histogram of combined component usage.
    */
    src = ePreFmt_(exp) ? eHstAll_(exp) ? ePreUdp_(exp) : ePixUdp_(exp)
							: SnkUdpPtr_(snk,0);
    roi = eHstAll_(exp) ? SnkUdpPtr_(snk,0) : ePixUdp_(exp);
    Histogram( src, roi, eHst_(exp,0) );
    eRGBCnt_(exp) = HistogramAnalysis( exp, 0 );
    memset( uBase_(eLutMap_(exp),0), 0, uArSize_(eLutMap_(exp),0)+7>>3 );

    /*
    **  Step D: allocate pixels.
    */
    state = MatchPseudoInit( exp );
    switch( state )
	{
    case FREE :
	/*
	**  Allocate new pixels: there are enough free so allocation can't fail.
	*/
	for( i = 0; i < eRGBCnt_(exp); MatchPseudoAllocColor(exp, i++, FALSE) );
	break;

    case SHARED :
	if( limit < range )
	    /*
	    **	Allocate new pixels where no reasonable match can be found.
	    */
	    MatchPseudoDistant( exp, limit );
	    /*	fall thru to allocate any remaining */
    case OWNED :
	if( ePxlCnt_(exp) < eRGBCnt_(exp) )
	    /*
	    **	Use the closest match for each required pixel.
	    */
	    MatchPseudoClosest( exp );
	break;
	}
}				/* end FormatPseudoColor */

/*-----------------------------------------------------------------------
------------------ routine: create a Histogram of pixel useage ----------
------------------------------------------------------------------------*/
static void Histogram( udp, roi, hst )
 UdpPtr	    udp, roi;
 CARD32	    hst[];
{
    INT32  x, y, X1, Y1, X2, Y2, pxl, pxl_nxt, scn_nxt;
    CARD8 *base = upBase_(udp);
    CARD32 msk  = (1<<BitsFromLevels_(upLvl_(udp)))-1;

    X1 = max(upX1_(udp), upX1_(roi));
    Y1 = max(upY1_(udp), upY1_(roi));
    X2 = min(upX2_(udp), upX2_(roi));
    Y2 = min(upY2_(udp), upY2_(roi));
    if( X2 < X1 || Y2 < Y1 )
	return;

    switch( upDType_(udp) )
	{
    case UdpK_DTypeBU :
	pxl_nxt = upPxlStr_(udp) >> 3;
	scn_nxt =(upScnStr_(udp) >> 3) - (X2 - X1 + 1) * pxl_nxt;
	pxl     = upPos_(udp) + upScnStr_(udp) * (upY1_(udp) - Y1)
			      + upPxlStr_(udp) * (upX1_(udp) - X1) >> 3;

	for( y = Y1; y++ <= Y2; pxl += scn_nxt )
	    for( x = X1; x++ <= X2; pxl += pxl_nxt )
		hst[ base[pxl] & msk ]++;
	break;

    default :
	pxl_nxt = upPxlStr_(udp);
	scn_nxt = upScnStr_(udp) - (X2 - X1 + 1) * pxl_nxt;
	pxl     = upPos_(udp) + upScnStr_(udp) * (upY1_(udp) - Y1)
			      + upPxlStr_(udp) * (upX1_(udp) - X1);

	for( y = Y1; y++ <= Y2; pxl += scn_nxt )
	    for( x = X1; x++ <= X2; pxl += pxl_nxt )
		hst[GET_VALUE_(base,pxl,msk)]++;
	break;
	}
}				/* end Histogram */

/*-----------------------------------------------------------------------
---------------------- routine: Analyze histogram data ------------------
------------------------------------------------------------------------*/
static CARD32 HistogramAnalysis( exp, comp )
 ExpCtxPtr	exp;
 CARD32		comp;
{
    CARD32 i, j, count, swap, *list = ePxlLst_(exp), *data = eHst_(exp,comp);
    /*
    **  Fill PxlLst with values required.
    */
    for( j = 0, i = 0;  i < eRGBCnt_(exp); i++ )
	if( data[i]  != 0 )
	    list[j++] = i;

    count = j;
    /*
    **  Sort PxlLst -- ordered by descending frequency of usage.
    */
    while( j-- > 1 )
        for( i = 0;  i < j;  i++ )
            if( data[ list[i] ] < data[ list[i+1] ] )
                {
                swap      = list[i];
                list[i]   = list[i+1];
                list[i+1] = swap;
                }
    /*
    **	Replace the Histogram with the sorted list and leave PxlLst zeroed.
    */
    if( count < eRGBCnt_(exp) )
	memset( &list[count], 0, (eRGBCnt_(exp) - count) * sizeof(CARD32) );
    eHst_(exp,comp) = list;
    memset( data, 0, eRGBCnt_(exp) * sizeof(CARD32) );
    ePxlLst_(exp)   = data;

    return( count );
}				/* end HistogramAnalysis */

/*-----------------------------------------------------------------------
--------------------- routine:  Import from Drawable --------------------
------------------------------------------------------------------------*/
static int Import( client, img )
 ClientPtr    client;
 PhotomapPtr *img;
{
    ExpCtxPtr  exp;
    CARD32   c, format, mapping, scnstr, padmsk, pxlstr, width, height;
    REQUEST( xieImportReq );

    exp = (ExpCtxPtr) DdxCalloc_(1, sizeof(ExpCtxRec));
    AllocOkIf_(exp, stuff->photomap_id);
    eClient_(exp) = client;

    LookupDrawable_( client, stuff->drawable_id );
    DrawableOkIf_( eClientDrw_(exp), (DdxFree_(exp), stuff->drawable_id) );
    MatchOkIf_( eClientDrw_(exp)->type == DRAWABLE_PIXMAP
	   ||   eClientDrw_(exp)->type == DRAWABLE_WINDOW
	   && ((WindowPtr)eClientDrw_(exp))->realized
	   &&  DrawableX_(eClientDrw_(exp)) >= 0
	   &&  DrawableX_(eClientDrw_(exp)) + DrawableWidth_(eClientDrw_(exp))
		       <= eClientDrw_(exp)->pScreen->width
	   &&  DrawableY_(eClientDrw_(exp)) >= 0
	   &&  DrawableY_(eClientDrw_(exp)) + DrawableHeight_(eClientDrw_(exp))
		       <= eClientDrw_(exp)->pScreen->height,
	      (DdxFree_(exp), stuff->drawable_id));
    eDepth_(exp) = eClientDrw_(exp)->depth;
    if( eDepth_(exp) > 1 )
	{
	LookupColormap_( eCmap_(exp), stuff->colormap_id );
	ColorOkIf_(eCmap_(exp), (DdxFree_(exp), stuff->colormap_id));
	}
    /*
    **  Allocate the destination Photomap.
    */
    for( c = 0; c < XieK_MaxComponents; c++ )
	eLvl_(exp, c) = !eCmap_(exp) ? 2 : 1<<eVisual_(exp)->bitsPerRGBValue;
    width   = DrawableWidth_( eClientDrw_(exp));
    height  = DrawableHeight_(eClientDrw_(exp));
    mapping = !eCmap_(exp) ? XieK_Bitonal : eClass_(exp) <= GrayScale
					  ? XieK_GrayScale : XieK_RGB;
    *img = UtilCreatePhotomap( mapping, &eLvl_(exp,0),
			       1.0, XieK_PP0, XieK_LP270,
			       XieK_ZeroDark, width, height, 
			       TRUE, TRUE, NULL );
    AllocOkIf_( *img, (DdxFree_(exp), stuff->photomap_id) );
    PxlPol_(*img) = stuff->polarity;
    Eport_(*img)  = exp;
    if( IsBitonal_(*img) )
	/*
	**  Discard the Photomap Udp -- probably doesn't match the Drawable.
	*/
	Udp_(*img,0) = UtilFreeUdp( Udp_(*img,0) );

    /*
    **  Allocate a Udp for importing ZPixmap formatted Drawable contents.
    */
    for( format = 0; format < screenInfo.numPixmapFormats
	   && eDepth_(exp) != screenInfo.formats[format].depth; format++ );
    pxlstr  = screenInfo.formats[format].bitsPerPixel;
    padmsk  = screenInfo.formats[format].scanlinePad - 1;
    scnstr  = pxlstr * width + padmsk & ~padmsk;
    ePixUdp_(exp) = UtilAllocUdp(UtilDType(eDepth_(exp),XieK_PCM,pxlstr,scnstr),
				 XieK_PCM, width, IsBitonal_(*img) ? height : 1,
				 pxlstr,  scnstr,
				 1<<eDepth_(exp), 0, TRUE, 0, NULL, NULL );
    AllocOkIf_( ePixUdp_(exp), stuff->photomap_id );
    if( IsBitonal_(*img) )
	{   /*
	    **	No analysis necessary -- GetImage directly into Photomap.
	    */
	(*eClientDrw_(exp)->pScreen->GetImage)
	 (eClientDrw_(exp),0,0,width,height,ZPixmap,1,upBase_(ePixUdp_(exp)));
	Udp_(*img,0)  = ePixUdp_(exp);
	ePixUdp_(exp) = NULL;
	}
    else
	{   /*
	    **  Allocate Colormap space and interpret the imported data.
	    */
	eMapLst_(exp) = (RGBPtr) DdxCalloc_(eMapLen_(exp), sizeof(RGBRec));
	AllocOkIf_(eMapLst_(exp),stuff->photomap_id);

	CmpMap_(*img) = eClass_(exp) <= PseudoColor ? ImportSingleMapData(*img)
						    : ImportTripleMapData(*img);
	if( CmpMap_(*img) < mapping )
	    {	/*
		**  Demote the Photomap to match the image data analysis.
		*/
	    for( c = CmpCnt_(*img), CmpCnt_(*img) = 1;
	       --c > 0; Udp_(*img,c) = UtilFreeUdp( Udp_(*img,c) ));

	    if( IsBitonal_(*img) )
		uLvl_(*img,0) = 2;
	    }
	ePixUdp_(exp) = UtilFreeUdp( ePixUdp_(exp) );
	}

    return( Success );
}				/* end Import */

/*-----------------------------------------------------------------------
-------------- routine:  Import Single ColorMap image Data --------------
------------------------------------------------------------------------*/
static int ImportSingleMapData( img )
 PhotomapPtr img;
{
    ExpCtxPtr	exp = Eport_(img);
    RGBPtr	map;
    UdpPtr  src_udp = ePixUdp_(exp),
	    dst_udp = Udp_(img,0);
    BOOL       gray = FALSE,
		rgb = IsRGB_(img);
    CARD8  *src_buf = upBase_(src_udp),
	   *red_buf = upBase_(dst_udp),
	   *grn_buf = (rgb ? uBase_(img,1) : NULL),
	   *blu_buf = (rgb ? uBase_(img,2) : NULL);
    CARD32  src_lg2 = upPxlStr_(src_udp) ==  8 ? 3
		    : upPxlStr_(src_udp) == 16 ? 4
		    : upPxlStr_(src_udp) == 32 ? 5 : 0;
    CARD32  dst_lg2 = upPxlStr_(dst_udp) ==  8 ? 3
		    : upPxlStr_(dst_udp) == 16 ? 4
		    : upPxlStr_(dst_udp) == 32 ? 5 : 0;
    CARD32  src_msk = (1 << eVisual_(exp)->nplanes)-1,
	    dst_msk = (1 << dst_lg2)-1;
    CARD32  dst_off = 0,
	    discard = 16 - BitsFromLevels_(upLvl_(dst_udp));
    CARD32  src_val, dst_pxl, i, scn, pix;

    /*
    **  Grab a snapshot of the Colormap.
    */
    ReadSingleMap( exp, NULL );

    /*
    **  Fill in Photomap data based on Colormap RGB values.
    */
    for( scn = 0; scn < upHeight_(dst_udp); scn++ )
	{
	(*eClientDrw_(exp)->pScreen->GetImage)
	 (eClientDrw_(exp),0,scn,upWidth_(src_udp),1,ZPixmap,src_msk,src_buf);

	for( dst_pxl = dst_off, pix = 0; pix < upWidth_(dst_udp); pix++ )
	    {
	    src_val = src_lg2 == 3 ? ((CARD8  *)src_buf)[pix] & src_msk
		    : src_lg2 == 4 ? ((CARD16 *)src_buf)[pix] & src_msk
		    : src_lg2 == 5 ? ((CARD32 *)src_buf)[pix] & src_msk
		    : GET_VALUE_(src_buf,(pix*upPxlStr_(src_udp)),src_msk);

	    map = &eMapLst_(exp)[src_val];
	    map->status = TRUE;

	    if( dst_lg2 == 3 )
		{
		if( rgb )
		    {
		    blu_buf[dst_pxl] = map->blu >> discard;
		    grn_buf[dst_pxl] = map->grn >> discard;
		    }
		red_buf[dst_pxl++]   = map->red >> discard;
		}
	    else
		{
		dst_pxl = dst_off + pix * upPxlStr_(dst_udp);
		if( rgb )
		    {
		    PUT_VALUE_(blu_buf, dst_pxl, map->blu >> discard, dst_msk);
		    PUT_VALUE_(grn_buf, dst_pxl, map->grn >> discard, dst_msk);
		    }
		PUT_VALUE_( red_buf, dst_pxl, map->red >> discard, dst_msk );
		}
	    }
	dst_off += upScnStr_(dst_udp) >> dst_lg2;
	}
    /*
    **  Scan thru the colors used to determine the actual mapping.
    */
    for(map = eMapLst_(exp), gray = FALSE, i = 0; i < eMapLen_(exp); map++, i++)
	if( map->status )
	    {
	    if( map->red ^ map->grn | map->red ^ map->blu )
		break;				/* color: red<>green<>blue  */
	    pix  = (INT16)(map->red)>>discard;
	    gray = gray || pix && ~pix;		/* gray: not black or white */
	    }
    return(i < eMapLen_(exp) ? XieK_RGB : gray ? XieK_GrayScale : XieK_Bitonal);
}				/* end ImportSingleMapData */

/*-----------------------------------------------------------------------
-------------- routine:  Import Triple ColorMap image Data --------------
------------------------------------------------------------------------*/
static int ImportTripleMapData( img )
 PhotomapPtr img;
{
    ExpCtxPtr	exp = Eport_(img);
    RGBPtr      map = eMapLst_(exp);
    UdpPtr  src_udp = ePixUdp_(exp),
	    dst_udp = Udp_(img,0);
    BOOL       gray = FALSE, rgb = FALSE;
    CARD8  *src_buf = upBase_(src_udp),
	   *red_buf =  uBase_(img,0),
	   *grn_buf =  uBase_(img,1),
	   *blu_buf =  uBase_(img,2);
    CARD32  src_lg2 = upPxlStr_(src_udp) ==  8 ? 3
		    : upPxlStr_(src_udp) == 16 ? 4
		    : upPxlStr_(src_udp) == 32 ? 5 : 0;
    CARD32  dst_lg2 = upPxlStr_(dst_udp) ==  8 ? 3
		    : upPxlStr_(dst_udp) == 16 ? 4
		    : upPxlStr_(dst_udp) == 32 ? 5 : 0;
    CARD32  red_shf = eRedOff_(exp),
	    grn_shf = eGrnOff_(exp),
	    blu_shf = eBluOff_(exp);
    CARD32  src_msk = (1 << eVisual_(exp)->nplanes)-1,
	    dst_msk = (1 << dst_lg2)-1,
	    red_msk = eRedMsk_(exp),
	    grn_msk = eGrnMsk_(exp),
	    blu_msk = eBluMsk_(exp);
    CARD32  dst_off = 0,
	    discard = 16 - BitsFromLevels_(upLvl_(dst_udp));
    CARD32  src_val, dst_pxl, red_val, grn_val, blu_val, scn, pix;

    /*
    **  Grab a snapshot of the Colormap.
    */
    ReadTripleMap( exp, NULL, NULL );

    /*
    **  Fill in Photomap data based on Colormap RGB values.
    */
    for( scn = 0; scn < upHeight_(dst_udp); scn++ )
	{
	(*eClientDrw_(exp)->pScreen->GetImage)
	 (eClientDrw_(exp),0,scn,upWidth_(src_udp),1,ZPixmap,src_msk,src_buf);

	for( dst_pxl = dst_off, pix = 0; pix < upWidth_(dst_udp); pix++ )
	    {
	    src_val = src_lg2 == 5 ? ((CARD32 *)src_buf)[pix]
		    : src_lg2 == 3 ? ((CARD8  *)src_buf)[pix]
		    : src_lg2 == 4 ? ((CARD16 *)src_buf)[pix]
		    : GET_VALUE_(src_buf,(pix*upPxlStr_(src_udp)),src_msk);
	    red_val = (INT16)(map[(src_val&red_msk)>>red_shf].red)>>discard;
	    grn_val = (INT16)(map[(src_val&grn_msk)>>grn_shf].grn)>>discard;
	    blu_val = (INT16)(map[(src_val&blu_msk)>>blu_shf].blu)>>discard;
	    rgb	    = rgb  || red_val ^ grn_val | red_val ^ blu_val;
	    gray    = gray || red_val && ~red_val;

	    if( dst_lg2 == 3 )
		{
		red_buf[dst_pxl  ] = red_val;
		grn_buf[dst_pxl  ] = grn_val;
		blu_buf[dst_pxl++] = blu_val;
		}
	    else
		{
		dst_pxl = dst_off + pix * upPxlStr_(dst_udp);
		PUT_VALUE_( red_buf, dst_pxl, red_val, dst_msk );
		PUT_VALUE_( grn_buf, dst_pxl, grn_val, dst_msk );
		PUT_VALUE_( blu_buf, dst_pxl, blu_val, dst_msk );
		}
	    }
	dst_off += upScnStr_(dst_udp) >> dst_lg2;
	}
    return( rgb ? XieK_RGB : gray ? XieK_GrayScale : XieK_Bitonal );
}				/* end ImportTripleMapData */

/*-----------------------------------------------------------------------
-------------------- routine: initialize for export using spans ---------
------------------------------------------------------------------------*/
static int InitForSpans( ctx, exp )
 ExportPipeCtxPtr   ctx;
 ExpCtxPtr  exp;    /* export context					    */
{
#if defined(DWV3)

    int size, i;
    DDXPointPtr pt, nxtpt;
    CARD32 *w, *nxtw;
    GCPtr spanGC;
    GCPtr revGC = NULL;
    CARD32 GCval;

    eSpnGC_(exp) = NULL;
    eSpnW_(exp) = NULL;
    eSpnPt_(exp) = NULL;

    LookupDrawable_( eClient_(exp), eDrwId_(exp) );

    /*
     * Select the GC needed to draw the spans.
     */
    if (eClientDrw_(exp)->type == DRAWABLE_PIXMAP &&
	eClientDrw_(exp)->depth == 1) {
	/* For depth 1 pixmap, make a GC that will just copy the bits */
	eSpnGC_(exp) = (GCPtr)CreateScratchGC(eClientDrw_(exp)->pScreen,
					      1);
	if (eSpnGC_(exp) == NULL)
	    goto alloc_fail;
	CopyGC(eClientGC_(exp), eSpnGC_(exp), ~0L);
        GCval = ~0L;
	ChangeGC(eSpnGC_(exp), GCForeground, &GCval);
	GCval = 0L;
	ChangeGC(eSpnGC_(exp), GCBackground, &GCval);

	spanGC = eSpnGC_(exp);

    } else {
	LookupGC_( eClient_(exp), eGCId_(exp) );
	spanGC = eClientGC_(exp);
    }
    ValidateGC(eClientDrw_(exp), spanGC);

    /* Allocate buffers for describing the largest possible number of
     * spans we will draw:
     *     one for each scanline in the window (drawing background)
     *     one for every other pixel in a scanline (drawing foreground)
     */
    size = max(upWidth_(ePixUdp_(exp))/2+1, upHeight_(ePixUdp_(exp)));
    eSpnW_(exp) = (CARD32 *)DdxMalloc_(size * sizeof(int));
    if (!IsPointer_(eSpnW_(exp)))
	goto alloc_fail;
    eSpnPt_(exp) = (DDXPointPtr)DdxMalloc_(size *
					   sizeof(DDXPointRec));
    if (!IsPointer_(eSpnPt_(exp)))
	goto alloc_fail;

    eSpnCnt_(exp) = size - 1;
    eSpnIdx_(exp) = 0;

    /* Create a GC with reversed foreground/background for drawing
     * the spans which must be in the background color.
     */
    revGC = (GCPtr)CreateScratchGC(eClientDrw_(exp)->pScreen,
					      eClientDrw_(exp)->depth);
    if (revGC == NULL)
	goto alloc_fail;
    CopyGC(spanGC, revGC, ~0L);
    GCval = spanGC->fgPixel;
    ChangeGC(revGC, GCBackground, &GCval);
	
    GCval = spanGC->bgPixel;
    ChangeGC(revGC, GCForeground, &GCval);
	
    ValidateGC(eClientDrw_(exp), revGC);
	
    /* Build the changelists needed to draw the entire window background */
    nxtpt = eSpnPt_(exp);
    nxtw = eSpnW_(exp);
    for (i = 0; i < upHeight_(ePixUdp_(exp)); i++) {
	nxtpt->x = ExpDstX_(ctx);
	(nxtpt++)->y = ExpDstY_(ctx) + i;
	*nxtw++ =  upWidth_(ePixUdp_(exp));
    }

    /* Draw the background */
    FillSpans_( eClientDrw_(exp), revGC,
	       upHeight_(ePixUdp_(exp)), eSpnPt_(exp), eSpnW_(exp));
    (void)FreeScratchGC(revGC);
    UsingScratchGC_(eClient_(exp), NULL);

    return( Success );

alloc_fail:

    if (eSpnGC_(exp) != NULL) {
	FreeScratchGC(eSpnGC_(exp));
	eSpnGC_(exp) = 0;
    }
    if (eSpnW_(exp) != NULL)
	eSpnW_(exp) = (CARD32 *)DdxFree_(eSpnW_(exp));
    if (eSpnPt_(exp) != NULL)
	eSpnPt_(exp) = (DDXPointPtr)DdxFree_(eSpnPt_(exp));
    if (revGC != NULL)
	FreeScratchGC(revGC);
    
    return( BadAlloc );
#else
   return( Success );
#endif
}

/*-----------------------------------------------------------------------
  -------------------- routine: allocate a new DirectColor ----------------
  ------------------------------------------------------------------------*/
static CARD32 MatchDirectAllocColor( exp, state, rgb )
    ExpCtxPtr  exp;			/* export context		    */
    RGBPtr	    state;		/* Colormap state info		    */
    RGBPtr	    rgb;		/* RGB triplet to be allocated	    */
{
    RGBPtr  r,  g,  b, map = eMapLst_(exp);
    CARD32 dr, dg, db, i, status;

    status = AllocColor( eCmap_(exp), &rgb->red, &rgb->grn, &rgb->blu,
			&rgb->pixel, eClientIdx_(exp) );
    if( status != Success )
    {					/*
	    **	Find the closest match for each component.
	    */
	for( r = g = b = map, dr = dg = db = -1, i = 0; i < eMapLen_(exp); i++ )
	    {
	    if(map[i].Rstate >= SHARED && dr > abs((int)(map[i].red-rgb->red)))
		{
		dr = abs((int)(map[i].red-rgb->red));
		r  = &map[i];
		}
	    if(map[i].Gstate >= SHARED && dg > abs((int)(map[i].grn-rgb->grn)))
		{
		dg = abs((int)(map[i].grn-rgb->grn));
		g  = &map[i];
		}
	    if(map[i].Bstate >= SHARED && db > abs((int)(map[i].blu-rgb->blu)))
		{
		db = abs((int)(map[i].blu-rgb->blu));
		b  = &map[i];
		}
	    }
	rgb->red   = r->red;
	rgb->grn   = g->grn;
	rgb->blu   = b->blu;
	rgb->pixel = r->pixel & eRedMsk_(exp)
		   | g->pixel & eGrnMsk_(exp)
		   | b->pixel & eBluMsk_(exp);
	status = AllocColor( eCmap_(exp), &rgb->red, &rgb->grn, &rgb->blu,
					  &rgb->pixel, eClientIdx_(exp) );
	}
    if( status == Success )
	{
	r = &map[(rgb->pixel & eRedMsk_(exp)) >> eRedOff_(exp)];
	g = &map[(rgb->pixel & eGrnMsk_(exp)) >> eGrnOff_(exp)];
	b = &map[(rgb->pixel & eBluMsk_(exp)) >> eBluOff_(exp)];
	r->red = rgb->red;
	g->grn = rgb->grn;
	b->blu = rgb->blu;
	r->Rstate = SHARED;
	g->Gstate = SHARED;
	b->Bstate = SHARED;
	}
    return( status );
}				/* end MatchDirectAllocColor */

/*-----------------------------------------------------------------------
-------------------- routine: allocate a new PseudoColor ----------------
------------------------------------------------------------------------*/
static CARD32 MatchPseudoAllocColor( exp, index, update )
 ExpCtxPtr	exp;
 CARD32		index;
 BOOL		update;
{
    RGBPtr lst, rgb = &eRGBLst_(exp)[index];
    CARD32 i, status, distance, sum, sqr;
    Pixel  pixel;
    INT32  dif;

    ePxlLst_(exp)[ePxlCnt_(exp)] = -1;

    status = AllocColor( eCmap_(exp), &rgb->red, &rgb->grn, &rgb->blu,
					 &pixel, eClientIdx_(exp));
    if( status == Success )
	{
	ePxlLst_(exp)[ePxlCnt_(exp)++] = pixel;
	rgb->Pstate   = OWNED;
	rgb->pixel    = pixel;
	rgb->distance = 0.0;
	eRGBLst_(exp)[index] = *rgb;
	eMapLst_(exp)[pixel] = *rgb;
	SetLutEntry( exp, (CARD32)pixel, 0, index );

	if( update )
	    /*
	    **  See if our new color is a better match for any pending colors.
	    */
	    for( lst = eRGBLst_(exp), i = 0; i < eRGBCnt_(exp); lst++, i++ )
		if( lst->Pstate == PRIVATE )
		    {
		    dif = rgb->red - lst->red;
		    sqr = dif * dif;
		    dif = rgb->grn - lst->grn;
		    sum = dif * dif + sqr;
		    if( sum < sqr )
			continue;   /* overflow */

		    dif = rgb->blu - lst->blu;
		    distance = dif * dif + sum;

		    if( lst->distance > distance  &&  distance >= sum )
			{
			lst->distance = distance;
			lst->pixel    = pixel;
			}
		    }
	}
    return( status );
}				/* end MatchPseudoAllocColor */

/*-----------------------------------------------------------------------
-------------------- routine: share existing PseudoColors ---------------
------------------------------------------------------------------------*/
static void MatchPseudoClosest( exp )
 ExpCtxPtr	exp;
{
    RGBPtr rgb, map;
    CARD32 index;

    for( rgb = eRGBLst_(exp), index = 0; index < eRGBCnt_(exp); rgb++, index++ )
	if( rgb->Pstate == PRIVATE )
	    {
	    map = &eMapLst_(exp)[rgb->pixel];

	    if( map->Pstate == SHARED )
		{
		AllocColor(eCmap_(exp), &map->red, &map->grn, &map->blu,
					&map->pixel, eClientIdx_(exp));
		ePxlLst_(exp)[ePxlCnt_(exp)++] = map->pixel;
		}
	    map->Pstate = OWNED;
	    *rgb = *map;
	    SetLutEntry( exp, (CARD32)(map->pixel), 0, index );
	    }
}				/* end MatchPseudoClosest */

/*-----------------------------------------------------------------------
-------- routine: allocate PseudoColors which have no close match -------
------------------------------------------------------------------------*/
static void MatchPseudoDistant( exp, limit )
 ExpCtxPtr	exp;
 double		limit;
{
    CARD32 i, count;
    double mean;

    MatchPseudoAllocColor( exp, 0, TRUE );  /* in case Colormap was empty   */

    while( ePxlCnt_(exp) < eRGBCnt_(exp) )
	{   /*
	    **  Calculate the mean match distance of all the pending colors.
	    */
	for( count = 0, mean = 0.0, i = 0; i < eRGBCnt_(exp); i++ )
	    if( eRGBLst_(exp)[i].distance > 0 )
		{
		count++;
		mean += eRGBLst_(exp)[i].distance;
		}
	mean = mean < limit * count ? limit : mean / (count + 1e-6);

	for( i = 0; i < eRGBCnt_(exp); i++ )
	    if( eRGBLst_(exp)[i].Pstate   == PRIVATE
	     && eRGBLst_(exp)[i].distance >= mean
	     && MatchPseudoAllocColor( exp, i, TRUE) != Success )
		break;				   /* the colormap is full  */

	if( mean == limit  ||  i < eRGBCnt_(exp) )
		break;
	}
}				/* end MatchPseudoDistant */

/*-----------------------------------------------------------------------
----------- routine: Init Colormap list and required colors list --------
------------------------------------------------------------------------*/
static CARD32 MatchPseudoInit( exp )
 ExpCtxPtr  exp;
{
    RGBRec state, *map, *rgb;
    CARD32 i,m, distance, sum, sqr;
    INT32  dif, entries = eMapLen_(exp);
    
    ReadSingleMap( exp, &state );

    for( rgb = eRGBLst_(exp), i = 0; i < eRGBCnt_(exp); rgb++, i++ )
	{   /*
	    **	Init RGB components.
	    */
	rgb->red = SetPseudoColor_(exp, eHst_(exp,0)[i], 0, rgb);
	rgb->grn = SetPseudoColor_(exp, eHst_(exp,0)[i], 1, rgb);
	rgb->blu = SetPseudoColor_(exp, eHst_(exp,0)[i], 2, rgb);

	if( state.Pstate != FREE )
	    {	/*
		**  Find closest existing match that the hardware can provide.
		*/
	    (*eCmap_(exp)->pScreen->ResolveColor)
		    (&rgb->red, &rgb->grn, &rgb->blu, eVisual_(exp));
	    for(rgb->distance=~0, map=eMapLst_(exp), m=0; m<entries; map++, m++)
		if( map->Pstate >= SHARED )
		    {
		    dif = rgb->red - map->red;
		    sqr = dif * dif;
		    dif = rgb->grn - map->grn;
		    sum = dif * dif + sqr;
		    if( sum < sqr )
			continue;   /* overflow */

		    dif = rgb->blu - map->blu;
		    distance = dif * dif + sum;

		    if( rgb->distance > distance  &&  distance >= sum )
			{
			rgb->distance = distance;
			rgb->pixel    = m;
			if( distance == 0 )
			    break;
			}
		    }
	    }
	}
    return( state.Pstate );
}				/* end MatchPseudoInit */

/*-----------------------------------------------------------------------
------------------ routine: Get Snapshot of Colormap State --------------
------------------------------------------------------------------------*/
static void ReadSingleMap( exp, state )
 ExpCtxPtr  exp;    /* export context					    */
 RGBPtr	    state;  /* OPTIONAL: pointer to RGBRec in which to return state */
{
    RGBPtr map;
    CARD32 i, available = 0, gray = eGryLmt_(exp) * 65535.0 + 0.5;

    /*
    **  Take a snapshot of the current Colormap state.
    */
    for( map = eMapLst_(exp), i = 0; i < eMapLen_(exp); map++, i++ )
	{
	if( eCmap_(exp)->red[i].fShared )
	    {
	    map->red = eCmap_(exp)->red[i].co.shco.red->color;
	    map->grn = eCmap_(exp)->red[i].co.shco.green->color;
	    map->blu = eCmap_(exp)->red[i].co.shco.blue->color;
	    }
	else
    	    {
	    map->red = eCmap_(exp)->red[i].co.local.red;
	    map->grn = eCmap_(exp)->red[i].co.local.green;
	    map->blu = eCmap_(exp)->red[i].co.local.blue;
	    }
	map->pixel  = i;
	map->Pstate = eStaticCmap_(exp) ? OWNED
		    : eCmap_(exp)->red[i].refcnt ==  0 ? FREE
		    : eCmap_(exp)->red[i].refcnt == -1 ? PRIVATE : SHARED;

	if( map->Pstate == FREE )
	    available++;

	else if( eLvl_(exp,1) == 0  &&  map->Pstate >= SHARED
				    && (abs((int)(map->red - map->grn)) > gray
				    ||  abs((int)(map->grn - map->blu)) > gray
				    ||  abs((int)(map->blu - map->red)) > gray))
	    map->Pstate  = PRIVATE;
	}
    if( state )
        state->Pstate = eMchLmt_(exp) == 0.0
		     && eRGBCnt_(exp) <= available ? FREE
				       : available ? SHARED : OWNED;

}				/* end ReadSingleMap */

/*-----------------------------------------------------------------------
------------------ routine: Get Snapshot of Colormap State --------------
------------------------------------------------------------------------*/
static void ReadTripleMap( exp, state, cnt )
 ExpCtxPtr  exp;    /* export context					    */
 RGBPtr	    state;  /* OPTIONAL: pointer to RGBRec in which to return state */
 CARD32	    cnt[];  /* OPTIONAL: array of number of pixels required	    */
{
    RGBPtr map;
    CARD32 i, Rfree = 0, Gfree = 0, Bfree = 0;

    for( map = eMapLst_(exp), i = 0; i < eMapLen_(exp); map++, i++ )
	{
	map->pixel = i << eRedOff_(exp) & eRedMsk_(exp)
		   | i << eGrnOff_(exp) & eGrnMsk_(exp)
		   | i << eBluOff_(exp) & eBluMsk_(exp);
	if( eClass_(exp) == TrueColor )
	    {
	    map->red = map->grn = map->blu = eCmap_(exp)->red[i].co.local.red;
	    map->Rstate = map->Gstate = map->Bstate = OWNED;
	    }
	else
	    {
	    map->red	= eCmap_(exp)->red  [i].co.local.red;
	    map->grn	= eCmap_(exp)->green[i].co.local.green;
	    map->blu	= eCmap_(exp)->blue [i].co.local.blue;
	    map->Rstate = eCmap_(exp)->red  [i].refcnt ==  0 ? FREE
			: eCmap_(exp)->red  [i].refcnt == -1 ? PRIVATE : SHARED;
	    map->Gstate = eCmap_(exp)->green[i].refcnt ==  0 ? FREE
			: eCmap_(exp)->green[i].refcnt == -1 ? PRIVATE : SHARED;
	    map->Bstate = eCmap_(exp)->blue [i].refcnt ==  0 ? FREE
			: eCmap_(exp)->blue [i].refcnt == -1 ? PRIVATE : SHARED;
	    if( map->Rstate == FREE )
		Rfree++;
	    else if( state  &&  map->Rstate == SHARED )
		state->red = map->red;
	    if( map->Gstate == FREE )
		Gfree++;
	    else if( state  &&  map->Gstate == SHARED )
		state->grn = map->grn;
	    if( map->Bstate == FREE )
		Bfree++;
	    else if( state  &&  map->Bstate == SHARED )
		state->blu = map->blu;
	    }
	}
    if( state )
	{
	state->Rstate = cnt[0] <= Rfree ? FREE : Rfree ? SHARED : OWNED;
	state->Gstate = cnt[1] <= Gfree ? FREE : Gfree ? SHARED : OWNED;
	state->Bstate = cnt[2] <= Bfree ? FREE : Bfree ? SHARED : OWNED;
	}
}				/* end ReadTripleMap */

/*-----------------------------------------------------------------------
------------------- routine:  Set an Entry in the LUT -------------------
------------------------------------------------------------------------*/
static void SetLutEntry( exp, value, comp, entry )
 ExpCtxPtr  exp;
 CARD32	    value;
 CARD32	    comp;
 CARD32	    entry;
{
    UdpPtr udp    = Udp_(eLutMap_(exp),comp);
    CARD8 *base   = upBase_(udp);
    CARD32 offset = upPxlStr_(udp)*(eHst_(exp,eLvl_(exp,comp)?comp:0)[entry]);
    CARD32 mask   = (1 << upPxlStr_(udp)) - 1;

    PUT_VALUE_(base, offset, value, mask);
}				    /* end SetLutEntry */

/*-----------------------------------------------------------------------
-------------- routine:  Setup Udp for various export operations --------
------------------------------------------------------------------------*/
static void SetupUdp( udp, levels, bits, comp )
 UdpPtr	    udp;
 CARD32	    levels;
 CARD32	    bits;
 CARD32	    comp;
{
    upCmpIdx_(udp) = comp;
    upLvl_(udp)    = levels;
    upPxlLen_(udp) = bits;
    upDType_(udp)  = UtilDType(bits, XieK_PCM, upPxlStr_(udp), upScnStr_(udp));
    upClass_(udp)  = UtilClass(upDType_(udp),XieK_PCM);
}				    /* end SetupUdp */

/*-----------------------------------------------------------------------
----------------- routine: Create Export pipeline element ---------------
------------------------------------------------------------------------*/
static int _ExportCreate(pipe, img, dstx, dsty)
 Pipe		pipe;
 PhotomapPtr	img;
 INT32		dstx, dsty; 
{
    CARD32  comp, dtype, dtype_mask;
    ExportPipeCtxPtr ctx = (ExportPipeCtxPtr)
			    DdxCreatePipeCtx_( pipe, &ExportPipeElement,FALSE);
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Build our ExportPipeCtx from the arguments passed in.
    */
    ExpSrc_(ctx)    = img;
    ExpDstX_(ctx)   = dstx;
    ExpDstY_(ctx)   = dsty;

#if defined(DWV3)
    if( IsBitonal_(img) )
	{
	dtype = UdpK_DTypeUndefined;
	dtype_mask = DtM_VU | DtM_CL;
	}
    else
	{
	dtype = IsGrayScale_(img) ? UdpK_DTypeBU : UdpK_DTypeUndefined;
	dtype_mask = DtM_Constrained;
	}
#else
    dtype = IsBitonal_(img)   ? UdpK_DTypeVU
	  : IsGrayScale_(img) ? UdpK_DTypeBU : UdpK_DTypeUndefined;
    dtype_mask = DtM_Constrained;
#endif
    /*
    **	Create a drain for each available source component.
    */
    for( comp = 0; comp < CmpCnt_(img); comp++ )
	{
	ExpDrn_(ctx,comp) = DdxRmCreateDrain_( Sink_(img), (1<<comp) );
	if( !IsPointer_(ExpDrn_(ctx,comp)) ) return( (int) ExpDrn_(ctx,comp) );
	DdxRmSetDType_( ExpDrn_(ctx,comp), dtype, dtype_mask );
	if( eHst_(Eport_(img),0) )
	    DdxRmSetQuantum_( ExpDrn_(ctx,comp), Height_(img) );
	}

    return( Success );
}				/* end _ExportCreate */

/*-----------------------------------------------------------------------
----------------------- routine: _ExportInitialize ----------------------
------------------------------------------------------------------------*/
static int _ExportInitialize(ctx)
 ExportPipeCtxPtr   ctx;
{
    PipeSinkPtr	src =  Sink_(ExpSrc_(ctx));
    ExpCtxPtr	exp = Eport_(ExpSrc_(ctx));
    CARD32  comp, GCval, status = Success;

    if( !IsPointer_(exp) ) return( BadAccess );
    /*
    **	See if we need to create a histogram of the "entire" image.
    */
    eHstAll_(exp) = SnkPrm_(src) && eHst_(exp,0)
		 && (upWidth_(ePixUdp_(exp)) <  SnkWidth_(src,0)
		 || upHeight_(ePixUdp_(exp)) < SnkHeight_(src,0));

    for( comp = 0; comp < CmpCnt_(ExpSrc_(ctx)); comp++ )
	{
	status = DdxRmInitializePort_( CtxHead_(ctx), ExpDrn_(ctx,comp) );
	if( status != Success ) return( status );
	}
    /*
    **  Assume input will be available from drain 0 first.
    */
    CtxInp_(ctx) = ExpDrn_(ctx,0);
    /*
    **  If input data is changelists, allocate buffers for span output
    **  and deal with the background pixels.
    */
    if (DrnDtyp_(ExpDrn_(ctx,0)) == UdpK_DTypeCL) 
	status = InitForSpans(ctx, exp);
    else
	{
	eSpnW_(exp)  = NULL;
	eSpnPt_(exp) = NULL;
	eSpnGC_(exp) = NULL;
	}
    /*
    **  Initialize our X11 image format conversion sink.
    */
    DdxRmCreateSink_( ExpCvt_(ctx) );

    return( status );
}				/* end _ExportInitialize */

/*-----------------------------------------------------------------------
------------------------ routine: _ExportActivate -----------------------
------------------------------------------------------------------------*/
static int _ExportActivate(ctx)
 ExportPipeCtxPtr   ctx;
{
    ExpCtxPtr	exp = Eport_(ExpSrc_(ctx));
    PipeSinkPtr cvt = ExpCvt_(ctx);
    UdpRec	pix;
    CARD32     comp, s;

    if( !IsPointer_(exp) ) return( BadAccess );

    LookupDrawable_( eClient_(exp), eDrwId_(exp) );
    if (eSpnGC_(exp) == NULL) {
	LookupGC_( eClient_(exp), eGCId_(exp) );
    } else { 
	UsingScratchGC_(eClient_(exp), eSpnGC_(exp));
    }
    if( eCmapId_(exp) )
	LookupColormap_( eCmap_(exp), eCmapId_(exp) );

    for( s = Success; s == Success; _ExportFreeData(ctx) )
	{
	for( comp = 0; comp < CmpCnt_(ExpSrc_(ctx)); comp++ )
	    if( ExpDat_(ctx,comp) == NULL )
		{
		if( comp > 0 )
		    /*
		    **	Set the quantum for the green and blue drains to
		    **	match what we obtained from the red drain.
		    */
		    DdxRmSetQuantum_( ExpDrn_(ctx,comp),
				      DatHeight_(ExpDat_(ctx,0)) );

		ExpDat_(ctx,comp) = DdxRmGetData_(ctx,ExpDrn_(ctx,comp));

		if( IsPointer_(ExpDat_(ctx,comp)) )
		    SnkUdpPtr_(cvt,comp) = DatUdpPtr_(ExpDat_(ctx,comp));
		else if( ExpDat_(ctx,comp) == NULL )
		    break;
		else
		    return( (int) ExpDat_(ctx,comp) );
		}
	if( comp < CmpCnt_(ExpSrc_(ctx)) )
	    break;

	/*
	** If the client trashed one of his resources, return the
	** appropriate error.
	*/
	if( !eClientDrw_(exp) )
	    s = BadDrawable;
	else if ( !eClientGC_(exp) )
	    s = BadGC;
	else if ( eCmapId_(exp) && !eCmap_(exp) )
	    s = BadColor;
	if ( s != Success )
	    continue;

	/*
	**  Do pre-export stuff.
	*/
	ValidateDrawableAndGC_( eClient_(exp) );
	s = Format( exp, cvt );
	if( s != Success ) break;
	/*
	**  Determine the coincidence of drain data and export region.
	*/
	pix = *ePixUdp_(exp);
	urX1_(pix)	= max(SnkX1_(cvt,0), urX1_(pix));
	urX2_(pix)	= min(SnkX2_(cvt,0), urX2_(pix));
	urY1_(pix)	= max(SnkY1_(cvt,0), urY1_(pix));
	urY2_(pix)	= min(SnkY2_(cvt,0), urY2_(pix));
	if( urX2_(pix) < urX1_(pix) || urY2_(pix) < urY1_(pix) )
	    continue;

	urWidth_(pix)   = urX2_(pix) - urX1_(pix) + 1;
	urHeight_(pix)  = urY2_(pix) - urY1_(pix) + 1;
	urArSize_(pix)  = urScnStr_(pix) * urHeight_(pix);

	s = Export( exp, cvt, &pix,
		    ExpDstX_(ctx) + urX1_(pix) - upX1_(ePixUdp_(exp)),
		    ExpDstY_(ctx) + urY1_(pix) - upY1_(ePixUdp_(exp)) );

	if( IsPointer_(urBase_(pix))
		    && urBase_(pix) != upBase_(ePixUdp_(exp)) )
	    DdxFreeBits_( urBase_(pix) );	    /* free converted data  */
	}
    return( s );
}				/* end _ExportActivate */

/*-----------------------------------------------------------------------
------------------------- routine: _ExportFlush -------------------------
------------------------------------------------------------------------*/
static int 	   _ExportFlush(ctx)
 ExportPipeCtxPtr  ctx;
{
    ExpCtxPtr	exp  = Eport_(ExpSrc_(ctx));
    PipeSinkPtr	sink = Sink_(ExpSrc_(ctx));

    if( !IsPointer_(exp) )
	return( Success );		/* Something isn't right. Punt. */

    if (DrnDtyp_(ExpDrn_(ctx,0)) == UdpK_DTypeCL)
    {
	/*
	**  If change lists remain to be drawn, do it now...
	*/
        if (eSpnIdx_(exp) != 0)
        {
	    GCPtr spanGC = (eSpnGC_(exp) == NULL) ? eClientGC_(exp) : 
						    eSpnGC_(exp);
	    FillSpans_(eClientDrw_(exp),
		       spanGC,
		       eSpnIdx_(exp),
		       eSpnPt_(exp),
		       eSpnW_(exp));

	    eSpnIdx_(exp) = 0;
        }
    }

    _ExportFreeData(ctx);
     ExportDone( Eport_(ExpSrc_(ctx)) );

    return( Success );
}				/* end _ExportFlush */

/*-----------------------------------------------------------------------
------------------------- routine: _ExportDestroy -----------------------
------------------------------------------------------------------------*/
static int _ExportDestroy(ctx)
 ExportPipeCtxPtr   ctx;
{
    CARD32  comp;

    for( comp = 0; comp < CmpCnt_(ExpSrc_(ctx)); comp++ )
	if( IsPointer_(ExpDrn_(ctx,comp)) )
	    DdxRmDestroyDrain_( ExpDrn_(ctx,comp) );

    return( Success );
}				/* end _ExportDestroy */

/*-----------------------------------------------------------------------
------------------------- routine: _ExportAbort -------------------------
------------------------------------------------------------------------*/
static int _ExportAbort(ctx)
 ExportPipeCtxPtr  ctx;
{
    _ExportFlush(ctx);
    eCmap_(Eport_(ExpSrc_(ctx))) = NULL;	    /* forget the Colormap  */

    return( Success );
}				/* end _ExportAbort */

/*-----------------------------------------------------------------------
-------------------- routine: _Export Free Data segment -----------------
------------------------------------------------------------------------*/
static int _ExportFreeData(ctx)
 ExportPipeCtxPtr  ctx;
{
    PipeSinkPtr cvt = ExpCvt_(ctx);
    CARD32     comp;

    for( comp = 0; comp < CmpCnt_(ExpSrc_(ctx)); comp++ )
	{
	if( IsPointer_(ExpDat_(ctx,comp)) )
	    DdxRmDeallocData_( ExpDat_(ctx,comp) );

	if( IsPointer_(cvt) )
	   SnkUdpPtr_(cvt,comp) = NULL;
	ExpDat_(ctx,comp)    = NULL;
	}
    return( Success );
}				/* end _ExportFreeData */
/* end module XieDisplay.c */
