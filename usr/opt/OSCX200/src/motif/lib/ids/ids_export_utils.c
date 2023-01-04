
/****************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
***************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      Image Display Services (IDS)
**
**  ABSTRACT:
**
**      This module contains routines for preparing a pre-rendered image
**	for exporting to its presentation level protocol.  Both widget and 
**	hardcopy rendition modules may make use of these routines.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	DECwindows V1.0
**
**  AUTHOR(S):
**
**	Robert NC Shelley
**      Subu Garikapati
**      Michael O'Connor (6-Oct-1991)
**
**  CREATION DATE:
**
**      February 1, 1989
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
    /*
    **  Standard C include files
    */

    /*
    **  ISL and IDS include files
    */
#include <img/ImgDef.h>		/* ISL public symbols			    */
#include <img/ImgEntry.h>	/* ISL public entry points	    */
#include <img/IdsStatusCodes.h>     /* IDS status codes                     */

#ifndef IDS_NOX
#include <ids__widget.h>	/* IDS public/private, Dwtoolkit, Xlib defs */
#else
#include <ids__widget_nox.h>	/* IDS public/private only                  */
#endif
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
IdsPipeDesc	    IdsCompileExport();		/* compile export pipe	    */

#ifndef IDS_NOX
static RemapLut		PixelAllocation();	/* Color allocation driver  */
static RemapLut         MatchStaticColors();    /* Static colors map driver */
static void		    AllocExact();	/* allocate exact colors    */
static void		    AllocClosest();	/* allocate best color match*/
static XColor		   *SetXColor();	/* load XColor structure    */
static void		    FreePixelList();	/* dealloc pixel index list */
static void		    CreateColorList();	/* create list of colors req*/

static unsigned long	RemapPixels();		/* remap to alloc'd colors  */
static unsigned long	RemapTruePixels();      /* remap to alloc'd colors  */
#endif
static unsigned long	PadScanlines();		/* add scanline padding	    */
static unsigned long	ExportImage();		/* export fid to XImage	    */
unsigned long           IdsPlaneSwapByPtr();   /* Swap red & blue w/UDP ptrs */
static void             PlaneSwapByData();     /* Swap red & blue data values*/
#else
#ifndef IDS_NOX
PROTO(static RemapLut PixelAllocation, (unsigned long /*fid*/, RenderContext /*ctx*/, IdsHistogramData /*histogram*/));
PROTO(static RemapLut MatchStaticColors, (unsigned long /*fid*/, RenderContext /*ctx*/, IdsHistogramData /*histogram*/));
PROTO(static void AllocExact, (RenderContext /*ctx*/));
PROTO(static void AllocClosest, (RenderContext /*ctx*/));
PROTO(static XColor *SetXColor, (RenderContext /*ctx*/, XColor */*color*/, unsigned long /*pixel*/));
PROTO(static void FreePixelList, (RenderContext /*ctx*/, Boolean /*clear*/));
PROTO(static void CreateColorList, (RenderContext /*ctx*/, IdsHistogramData /*histogram*/));
PROTO(static unsigned long RemapPixels, (unsigned long /*fid*/, RenderContext /*ctx*/, RemapLut /*lut*/, unsigned long /*roi*/));
PROTO(static unsigned long RemapTruePixels, (unsigned long /*fid*/, RenderContext /*ctx*/, unsigned long /*roi*/));
#endif
PROTO(static unsigned long PadScanlines, (unsigned long /*fid*/, RenderContext /*ctx*/, unsigned long /*roi*/));
PROTO(static unsigned long ExportImage, (unsigned long /*fid*/, RenderContext /*ctx*/, Boolean /*DoPlaneSwap*/));
PROTO(static void PlaneSwapByData, (unsigned long /*fid*/, RenderContext /*ctx*/));
#endif


/*
**  MACRO definitions
*/
    /* none */
/*
**  Equated Symbols
*/
    /* none */
/*
**  External References
*/
#ifdef NODAS_PROTO
extern unsigned long	*IdsInsertPipe();   /* Insert function into pipe    */
extern IdsHistogramData	 IdsHistogram();    /* histogram pixel value usage  */
extern unsigned long int IdsPixelRemap();   /* remap pixel values	    */
extern void		 IdsPutItem();      /* append item to ISL itmlst    */
extern void		 IdsFreePutList();  /* deallocate ISL itmlst	    */
IdsMatchData	         IdsMatchColors();  /* IDS appl req colors matching */
#endif

extern unsigned long     ImgCvtCompSpaceOrg();
/* mask array for the GetField_() and PutField_() MACROs */
#if ( ( defined(__VAXC) || defined(VAXC) ) && (!defined(__alpha) && !defined(ALPHA) ) ) 
globalref unsigned int IMG_AL_ONES_ASCENDING[33];
#else
extern    unsigned int IMG_AL_ONES_ASCENDING[33];
#endif

/* lookup tables */
#if ( ( defined(__VAXC) || defined(VAXC) ) && (!defined(__alpha) && !defined(ALPHA) ) ) 
globalref unsigned int IMG_AB_BIN_RVSD[256];
globalref unsigned int IMG_AB_NIB_RVSD[256];
#else
extern	  unsigned int IMG_AB_BIN_RVSD[256];
extern	  unsigned int IMG_AB_NIB_RVSD[256];
#endif
/*
**	Local Storage
*/
    /* none */

/*******************************************************************************
**  IdsCompileExport
**
**  FUNCTIONAL DESCRIPTION:
**
**	Compile an IDS rendering Pipe with export preparation routines.  The 
**	FIFO pipe will contain the IDS and ISL functions (and arguments) needed
**	to export the rendered image.  These functions may be appended to an
**	existing pipe (init flag FALSE).
**
**  FORMAL PARAMETERS:
**
**	ctx	     - rendering context.
**	fid_pointer  - pointer to fid containing image to be rendered.
**	init	     - flag: if TRUE, pipe should be init'd from descriptor.
**
**  FUNCTION VALUE:
**
**	pd	- pipe descriptor (from ctx) containing rendering sequence.
**
*******************************************************************************/
IdsPipeDesc  IdsCompileExport( 
     RenderContext  ctx
    ,unsigned long  *fid_pointer
    ,Boolean	    init
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);
    Boolean copied;
    unsigned long  new_cs;
#ifndef IDS_NOX
    Visual *visual = Vis_(ctx);
#endif

#ifdef TRACE
printf( "Entering Routine IdsCompileExport in module IDS_EXPORT_UTILS \n");
#endif


    iScheme_(rcb) &=  0x0000ffff;

    copied = PDcopiedROI_(pd);
    PDfid_(pd)  = fid_pointer;
    if( init )
	{
	PDpipe_(pd) = PDbase_(pd);
	PDpipe_(pd)->flink = NULL;
	}

    /*
    **  No export rendition for protocol Sixel or Postscript. This line of
    **  code should not be here eventually
    if(Proto_(ctx) == Ids_PostScript || Proto_(ctx) == Ids_Sixel )
        return( pd );
    */

    /*
    ** This is a place holder so that PipeNotify can tell the save image
    ** callback to get the fid that came out of the render pipe and copy it.
    ** 
    **  Pipe <== IDS function ExportPipeTap: returns nothing
    PDrfid_(pd)  = fid_pointer; 
    InsertPipe_( pd, Ids_FunctionExportPipeTap,
		 IdsVoid, ExportPipeTap,( EndArgs_ ));
    */

#ifndef IDS_NOX
    /*
    **	Load pipe with IDS functions to prepare for image export, depending on
    **	whether the final rendered image will be continuous tone or bitonal,
    **	and depending on our final protocol.
    */
    if( iGRA_(rcb) != 2  && (iProto_(rcb) == Ids_XImage
			 ||  iProto_(rcb) == Ids_Pixmap))
      {
      switch( visual->class )
           {
        case PseudoColor:
        case GrayScale:    

         /*
   	 **  Pipe <== IDS function AnalyzeColorUsage: returns IdsHistogramData.
	 */
	 InsertPipe_( pd, Ids_FunctionHistogram,
		     IdsTemp, IdsHistogram,
		   ( ArgByPtr_( PDfid_(pd)),      /* comp space convert fid */ 
		     ArgByVal_( copied ? 0 : iROI_(rcb) ),
		     EndArgs_ ));
	 /*
	 **  Pipe <== IDS function PixelAllocation: returns RemapLut.
	 */
	 InsertPipe_( pd, Ids_FunctionAllocColor,
		     IdsTemp, PixelAllocation,
		   ( ArgByPtr_( PDfid_(pd) ), ArgByVal_( ctx ),
		     ArgByPtr_( PDtmp_(pd) ),		/* IdsHistogramData */
		     EndArgs_ ));
	 /*
	 **  Pipe <== IDS function RemapPixels: returns final rendered fid.
	 */
	 InsertPipe_( pd, Ids_FunctionRemapColor|(copied ? 0 : Ids_FunctionROI),
		     IdsTemp, RemapPixels,
		   ( ArgByPtr_( PDfid_(pd) ), ArgByVal_( ctx ),
		     ArgByPtr_( PDtmp_(pd) ),		/* RemapLut	    */
		     ArgByVal_( copied ? 0 : iROI_(rcb) ),
		     EndArgs_ ));
         break;
        case StaticColor:
         /*
         **  Pipe <== IDS function AnalyzeColorUsage: returns IdsHistogramData.
         */
         InsertPipe_( pd, Ids_FunctionHistogram,
                     IdsTemp, IdsHistogram,
                   ( ArgByPtr_( PDfid_(pd) ),
                     ArgByVal_( copied ? 0 : iROI_(rcb) ),
                     EndArgs_ ));
         /*
         **  Pipe <== IDS function MatchingColors: returns RemapLut.
         */
         InsertPipe_( pd, Ids_FunctionMatchColor,
                     IdsTemp, MatchStaticColors,
                   ( ArgByPtr_( PDfid_(pd) ), ArgByVal_( ctx ),
                     ArgByPtr_( PDtmp_(pd) ),           /* IdsHistogramData */
                     EndArgs_ ));
         /*
         **  Pipe <== IDS function RemapPixels: returns final rendered fid.
         */
         InsertPipe_( pd, Ids_FunctionRemapColor|(copied ? 0 : Ids_FunctionROI),
                     IdsTemp, RemapPixels,
                   ( ArgByPtr_( PDfid_(pd) ), ArgByVal_( ctx ),
                     ArgByPtr_( PDtmp_(pd) ),           /* RemapLut         */
                     ArgByVal_( copied ? 0 : iROI_(rcb) ),
                     EndArgs_ ));
         break;

        case DirectColor:
        case StaticGray:
             break;
        case TrueColor:
 	 /*
	 **  Pipe <== IDS function RemapTruePixels: returns final rendered fid.
	 */
         if( iAlgor_(rcb) != Ids_Requantize )
	 InsertPipe_( pd, Ids_FunctionRemapColor|(copied ? 0 : Ids_FunctionROI),
		     IdsTemp, RemapTruePixels,
		   ( ArgByPtr_( PDfid_(pd) ), ArgByVal_( ctx ),
		     ArgByVal_( copied ? 0 : iROI_(rcb) ),
		     EndArgs_ ));
            break;
        default:
            break;
           }/* end of switch */
      }/* end of if */
    else
#endif
	/*
	**  Pipe <== IDS function PadScanlines: returns final rendered fid.
	*/
	InsertPipe_( pd, Ids_FunctionPad | (copied ? 0 : Ids_FunctionROI),
		     IdsTemp, PadScanlines,
		   ( ArgByPtr_( PDfid_(pd) ), ArgByVal_( ctx ),
		     ArgByVal_( copied ? 0 : iROI_(rcb) ),
		     EndArgs_ ));
    /*
    **  Pipe <== IDS function ExportImage: returns fid passed in.
    */
    InsertPipe_( pd, Ids_FunctionExport,
		 &PDfid_(pd), ExportImage,
	       ( ArgByPtr_( PDtmp_(pd) ),	    /* final rendered fid   */
		 ArgByVal_( ctx ),
		 ArgByVal_( (iScheme_(rcb) & Ids_UsePlaneSwapByData) ? 1 : 0),
		 EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine IdsCompileExport  in module IDS_EXPORT_UTILS \n");
#endif
    return( pd );
}

/*******************************************************************************
**  PixelAllocation
**
**  FUNCTIONAL DESCRIPTION:
**
**	IDS Pipe utility function: driver for X11 color allocation.  Returns
**	a re-map table for mapping the image data to the pixels allocated.
**
**  FORMAL PARAMETERS:
**
**	fid
**	ctx
**	histogram
**
**  FUNCTION VALUE:
**
**	address of LUT for use by RemapPixels
**
*******************************************************************************/
#ifndef IDS_NOX
static RemapLut	PixelAllocation( fid, ctx, histogram )
 unsigned long	    fid;
 RenderContext	    ctx;
 IdsHistogramData   histogram;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    PixelAlloc   pa;
    RemapLut    lut;
    unsigned long i;

#ifdef TRACE
printf( "Entering Routine PixelAllocation in module IDS_EXPORT_UTILS \n");
#endif
    /*
    **  Allocate a new PixelAllocStruct.
    */
    PxlAlloc_(ctx) = (PixelAlloc) _ImgCalloc( 1, sizeof(PixelAllocStruct));
    pa = PxlAlloc_(ctx);
    /*
    **  Initialize constants and variables required for color allocations.
    */
    GetIsl_( fid, Img_SpectType,     pa->spect_type, 0 );
    GetIsl_( fid, Img_BitsPerPixel,  pa->pixel_bits, 0 );
    GetIsl_( fid, Img_BrtPolarity,   pa->polarity,   0 );
    pa->polarity = pa->polarity == ImgK_ZeroMinIntensity ? 0 : 0xFFFF;
    GetBitsPerComponent_( fid, pa->spect_count, pa->cmp_bpc );
    for( i = 0; i < Ids_MaxComponents; i++ )
	{
	pa->cmp_off[i] = i == 0 ? 0 : pa->cmp_off[i-1] + pa->cmp_bpc[i-1];
	pa->cmp_msk[i] = (1 << pa->cmp_bpc[i]) - 1;
	pa->cmp_lvl[i] = ( pa->spect_type == ImgK_StypeGrayscale
				    ? iGRA_(rcb) : iRGB_(rcb)[i] ) - 1;
	}

    /*
    **  Create a list of colors needed (ordered by frequency of usage).
    */
    CreateColorList( ctx, histogram );
    MemFree_(  &histogram->pointer );
    _ImgCfree( histogram );

    if( CmpMode_(ctx) == Ids_PrivateColors )
       {
       /*
       ** Match appl supplied color list and indecies for color allocation
       */
       PxlDat_(ctx) = IdsMatchColors( Scr_(ctx), Cmap_(ctx), pa->colors,
                                       pa->count, ClrMap_(ctx), PltCnt_(ctx),
                                       CSpace_(ctx));
       }
    else
       {
       /*
       ** IdsNmatchDistance determines the choice of color alloc algorithms.
       */
       if( MchLim_(ctx) == 0.0 )
     	   /*
	   **  Attempt to allocate all the exact colors we need.
	   */
	   AllocExact( ctx );

       else
	   /*
	   **  Share colors that are perceptually "close" to those we need.
	   */
	   AllocClosest( ctx );
       }

    /*
    **  Release all the colors allocated during a previous rendering.
    */
    if( CmpMode_(ctx) != Ids_PrivateColors )
        FreePixelList( ctx, FALSE );

    /*
    **  Initialize a RemapLutStruct.
    */
    lut = (RemapLut) _ImgCalloc( 1, sizeof(RemapLutStruct) );
    lut->stride = PxlStr_(ctx);			     /* match dst stride      */
    lut->mask   = IMG_AL_ONES_ASCENDING[lut->stride];/* match dst stride      */
    i           = MAX(PxlStr_(ctx), pa->pixel_bits); /* use larger: PxlStr_() */
    lut->bytes  = (i * (1 << i) + 7) / 8;	     /* ...or bits-per-pixel  */
    lut->base   = (unsigned char *) _ImgCalloc( lut->bytes, sizeof(char) );

    /*
    **  Stash the pixel values in the LUT and create a list of pixels in use.
    */
    for( i = 0; i < pa->count; i++ )
	{
	PutField_( lut->base, lut->stride * pa->pixels[i],
			      lut->mask,    pa->colors[i].pixel );
	pa->pixels[i] = pa->colors[i].pixel;
	}
    PxlLst_(ctx) = pa->pixels;
    PxlCnt_(ctx) = pa->count;
    MemFree_( &pa->colors );
    MemFree_( &PxlAlloc_(ctx) );

#ifdef TRACE
printf( "Leaving Routine PixelAllocation in module IDS_EXPORT_UTILS \n");
#endif
    return( lut );
}

/*******************************************************************************
**  MatchStaticColors
**
**  FUNCTIONAL DESCRIPTION:
**
**	IDS Pipe utility function: driver for X11 color Matching.  Returns
**	a re-map table for mapping the image data to the pixels allocated.
**
**  FORMAL PARAMETERS:
**
**	fid
**	ctx
**	histogram
**
**  FUNCTION VALUE:
**
**	address of LUT for use by RemapPixels
**
*******************************************************************************/
static RemapLut	MatchStaticColors( fid, ctx, histogram )
 unsigned long	    fid;
 RenderContext	    ctx;
 IdsHistogramData   histogram;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    PixelAlloc   pa;
    RemapLut    lut;
    unsigned long i;

#ifdef TRACE
printf( "Entering Routine MatchStaticColors in module IDS_EXPORT_UTILS \n");
#endif
    /*
    **  Allocate a new PixelAllocStruct.
    */
    PxlAlloc_(ctx) = (PixelAlloc) _ImgCalloc( 1, sizeof(PixelAllocStruct));
    pa = PxlAlloc_(ctx);
    /*
    **  Initialize constants and variables required for color allocations.
    */
    GetIsl_( fid, Img_SpectType,     pa->spect_type, 0 );
    GetIsl_( fid, Img_BitsPerPixel,  pa->pixel_bits, 0 );
    GetIsl_( fid, Img_BrtPolarity,   pa->polarity,   0 );
    pa->polarity = pa->polarity == ImgK_ZeroMinIntensity ? 0 : 0xFFFF;
    GetBitsPerComponent_( fid, pa->spect_count, pa->cmp_bpc );
    for( i = 0; i < Ids_MaxComponents; i++ )
	{
	pa->cmp_off[i] = i == 0 ? 0 : pa->cmp_off[i-1] + pa->cmp_bpc[i-1];
	pa->cmp_msk[i] = (1 << pa->cmp_bpc[i]) - 1;
	pa->cmp_lvl[i] = ( pa->spect_type == ImgK_StypeGrayscale
				    ? GRA_(rcb) : RGB_(rcb)[i] ) - 1;
	}

    /*
    **  Create a list of colors needed (ordered by frequency of usage).
    */
    CreateColorList( ctx, histogram );
    MemFree_(  &histogram->pointer );
    _ImgCfree( histogram );

    /*
    **  Share the static colors that are perceptually "close" to those we need.
    */
    AllocClosest( ctx );

    /*
    **  Initialize a RemapLutStruct.
    */
    lut = (RemapLut) _ImgCalloc( 1, sizeof(RemapLutStruct) );
    lut->stride = PxlStr_(ctx);			     /* match dst stride      */
    lut->mask   = IMG_AL_ONES_ASCENDING[lut->stride];/* match dst stride      */
    i           = MAX(PxlStr_(ctx), pa->pixel_bits); /* use larger: PxlStr_() */
    lut->bytes  = (i * (1 << i) + 7) / 8;	     /* ...or bits-per-pixel  */
    lut->base   = (unsigned char *) _ImgCalloc( lut->bytes, sizeof(char) );

    /*
    **  Stash the pixel values in the LUT and create a list of pixels in use.
    */
    for( i = 0; i < pa->count; i++ )
	{
	PutField_( lut->base, lut->stride * pa->pixels[i],
			      lut->mask,    pa->colors[i].pixel );
	pa->pixels[i] = pa->colors[i].pixel;
	}
    PxlLst_(ctx) = pa->pixels;
    PxlCnt_(ctx) = pa->count;
    MemFree_( &PxlAlloc_(ctx) );

#ifdef TRACE
printf( "Leaving Routine MatchStaticColors in module IDS_EXPORT_UTILS \n");
#endif
    return( lut );
}

/*******************************************************************************
**  AllocExact
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate all the exact colors required.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context
**
*******************************************************************************/
static void AllocExact( ctx )
 RenderContext ctx;
{
    PixelAlloc  pa = PxlAlloc_(ctx);
    unsigned long index;

#ifdef TRACE
printf( "Entering Routine AllocExact in module IDS_EXPORT_UTILS \n");
#endif

    for( index = 0; index < pa->count; ++index )
	{
	if( XAllocColor( Dpy_(ctx), Cmap_(ctx), pa->colors+index ))
	    continue;

	/*
	**  Make space by freeing colors allocated during a previous rendering,
	**  then try to allocate this color again (the window is also cleared).
	*/
	FreePixelList( ctx, TRUE );

	if( !XAllocColor( Dpy_(ctx), Cmap_(ctx), pa->colors+index ))
	    {
	    /*
	    **	Insufficient colors available in the colormap -- free all the 
	    **	colors we've allocated so far and use the color match routines.
	    */
	    IdsFreeXColors( Scr_(ctx), Cmap_(ctx), pa->colors, index );
	    AllocClosest( ctx );	    
	    break;
	    }
	}
#ifdef TRACE
printf( "Leaving Routine AllocExact in module IDS_EXPORT_UTILS \n");
#endif
}

/*******************************************************************************
**  AllocClosest
**
**  FUNCTIONAL DESCRIPTION:
**
**	When colormap space is limited: allocates best colors available.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context
**
*******************************************************************************/
static void AllocClosest( ctx )
 RenderContext ctx;
{
    PixelAlloc pa = PxlAlloc_(ctx);
    IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine AllocClosest in module IDS_EXPORT_UTILS \n");
#endif
    /*
    **  Release the colors allocated during a previous rendering (the window is
    **  cleared to prevent our user from seeing the old image go technicolor.
    **  If the visual type is StaticColor colors are not freed.
    */
    if( Vis_(ctx)->class != StaticColor && Vis_(ctx)->class != StaticGray )
       FreePixelList( ctx, TRUE );

    /*
    **  Share colors that are perceptually "close" to what we want.
    **	    "Close" depends on the distance resources specified by 
    **	    the application (IdsNmatchDistance and IdsNgrayDistance).
    */
    PxlDat_(ctx) = IdsAllocColors( Scr_(ctx), Cmap_(ctx), pa->colors, pa->count,
				   CSpace_(ctx), MchLim_(ctx),
					pa->spect_type == ImgK_StypeGrayscale
					       ? GraLim_(ctx) : 1.0 );

    if( PxlDat_(ctx) == NULL )
	{
	/*
	**  Apparently the colormap must be totally allocated private!
	*/
	XtWarningMsg("ClrMapPvt","AllocateColors","IdsImageError",
		     "No color cells free to share in colormap.",0,0);

	/*
	**  Free our list of pixel values so we won't attempt to remap to
	**  non-existant colors.  If this is a widget, we can abort the pipe
	**  (see IDS_RENDER_IMAGE\PipeNotify), otherwise the image will be
	**  remapped to whatever the color for index 0 happens to be -- UGH!
	*/
	MemFree_( &pa->pixels );
	pa->count = 0;
	iReason_(rcb) = IdsCRPurge;		    /* abort widget pipe    */
	iRender_(rcb) = Ids_Purge;		    /* don't re-start pipe  */
	}
#ifdef TRACE
printf( "Leaving Routine AllocClosest in module IDS_EXPORT_UTILS \n");
#endif
}

/*******************************************************************************
**  SetXColor
**
**  FUNCTIONAL DESCRIPTION:
**
**	Load an XColor structure according to the pixel value passed in.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context
**	color	- XColor structure to set
**	pixel	- pixel value
**
**  FUNCTION VALUE:
**
**	XColor	- color structure passed in.
**
*******************************************************************************/
static XColor *SetXColor( ctx, color, pixel )
 RenderContext ctx;
 XColor	      *color;
 unsigned long pixel;
{
/*
**  MACRO to compute an XColor RGB component value as follows:
**	component = the component_level (extracted from pel)
**		  * the maximum value that can be passed to any server
**		  / the maximum level in the rendered image for this component.
**		  ^ polarity (0x0000 == normal, 0xffff == invert)
*/
#define ComputeColor_(comp,pel,vars)    vars->cmp_lvl[comp] == 0 ? 0 : \
	((pel >> vars->cmp_off[comp]) & vars->cmp_msk[comp]) \
			      * 65535 / vars->cmp_lvl[comp] ^ vars->polarity

    PixelAlloc  pa = PxlAlloc_(ctx);

#ifdef TRACE
printf( "Entering Routine SetXColor in module IDS_EXPORT_UTILS \n");
#endif

    switch( pa->spect_type )
	{
    case ImgK_StypeGrayscale:
	color->red   = ComputeColor_( GRA, pixel, pa );
	color->green = color->red;
	color->blue  = color->red;
	break;

    case ImgK_StypeMultispect:
	color->red   = ComputeColor_( RED, pixel, pa );
	color->green = ComputeColor_( GRN, pixel, pa );
	color->blue  = ComputeColor_( BLU, pixel, pa );
	break;
	}
    color->pixel = 0;
    color->flags = DoRGB;
    color->pad   = 0;
    return( color );
  
#ifdef TRACE
printf( "Leaving Routine SetXColor in module IDS_EXPORT_UTILS \n");
#endif
}

/*******************************************************************************
**  FreePixelList
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate list of colors.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context
**	clear	- flag: True = clear window
**
*******************************************************************************/
static void FreePixelList( 
     RenderContext  ctx
    ,Boolean	    clear
    )
{
#ifdef TRACE
printf( "Entering Routine FreePixelList in module IDS_EXPORT_UTILS \n");
#endif
    if( PxlLst_(ctx) != 0 && PxlCnt_(ctx) != 0 )
	{
	/*
	**  Free colors allocated during a previous rendering.
	*/
	XFreeColors( Dpy_(ctx), Cmap_(ctx), PxlLst_(ctx), PxlCnt_(ctx), 0 );
	MemFree_( &PxlLst_(ctx) );
	PxlCnt_(ctx) = 0;

	if( clear )
	    /*
	    **  The server needs to be encouraged to get on with the job.
	    **	Otherwise Xlib seems to remember if the colormap is full
	    **	and denies color allocation requests until hearing from
	    **	the server about the new condition of the colormap.
	    */
	    XSync( Dpy_(ctx), FALSE );

	if( clear && Win_(ctx) != 0 )
	    /*
	    **  Clear the window so the old image doesn't go technicolor.
	    */
	    XClearArea( Dpy_(ctx), Win_(ctx), 0, 0, WinW_(ctx), WinH_(ctx), 0 );
	}

#ifdef TRACE
printf( "Leaving Routine FreePixelList in module IDS_EXPORT_UTILS \n");
#endif
}

/*******************************************************************************
**  CreateColorList
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create lists of pixel values and XColors from the histogram data.
**	The lists are sorted by frequency of usage: from most to least used.
**
**  FORMAL PARAMETERS:
**
**	ctx	  - rendering context
**	histogram - array of pixel usage values.
**
*******************************************************************************/
static void CreateColorList( ctx, histogram )
 RenderContext ctx;
 IdsHistogramData histogram;
{
    PixelAlloc  pa = PxlAlloc_(ctx);
    unsigned long i, j, swap, *list, *data = histogram->pointer;

#ifdef TRACE
printf( "Entering Routine CreateColorList in module IDS_EXPORT_UTILS \n");
#endif
    /*
    **  Find how many distinct pixel values are in use.
    */
    for( pa->count = 0, i = 0;  i < histogram->count;  i++ )
	if( data[i] != 0 )
	    ++pa->count;
    /*
    **  Allocate the 'pixels' list and fill in the pixel values required.
    */
    list = (unsigned long *) _ImgMalloc( pa->count * sizeof(unsigned long));
    pa->pixels = list;
    for( i = 0, j = 0;  j < pa->count;  i++ )
	if( data[i]  != 0 )
	    list[j++] = i;
    /*
    **  Sort the list, ordered by descending frequency of usage.
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
    **	Load all the XColors we need.
    */
    pa->colors = (XColor *) _ImgMalloc( pa->count * sizeof(XColor));
    for( i = 0; i < pa->count; ++i )
	SetXColor( ctx, pa->colors+i, pa->pixels[i] );

#ifdef TRACE
printf( "Leaving Routine CreateColorList in module IDS_EXPORT_UTILS \n");
#endif
}

/*******************************************************************************
**  RemapPixels
**
**  FUNCTIONAL DESCRIPTION:
**
**	IDS Pipe utility function: re-maps image pixels to allocated colors.
**
**  FORMAL PARAMETERS:
**
**	fid
**	ctx
**	lut
**	roi
**
*******************************************************************************/
static unsigned long RemapPixels( fid, ctx, lut, roi )
 unsigned long fid;
 RenderContext ctx;
 RemapLut      lut;
 unsigned long roi;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int  stride;
    struct PUT_ITMLST  *attr = 0;
    struct UDP  udp;
    unsigned long  new_fid;

#ifdef TRACE
printf( "Entering Routine RemapPixels in module IDS_EXPORT_UTILS \n");
#endif
    /*
    **  Build remap item list.
    */
    GetIsl_( fid, Img_Udp, udp, 0 );	/* scanline stride depends on ROI   */
    if( roi != 0 )
	_ImgSetRoi( &udp, roi );
    stride  = PxlStr_(ctx) * (udp.UdpL_X2 - udp.UdpL_X1 + 1);
    stride += AlignBits_(stride, Pad_(ctx));
    IdsPutItem(&attr, Img_ScanlineStride,   stride,		0);
    IdsPutItem(&attr, Img_SpectralMapping,  ImgK_PrivateMap,	0);
    IdsPutItem(&attr, Img_NumberOfComp,     1,			0);
    IdsPutItem(&attr, Img_PixelStride,      PxlStr_(ctx),	0);
    IdsPutItem(&attr, Img_BitsPerPixel,     WinD_(ctx),		0);
    IdsPutItem(&attr, Img_ImgBitsPerComp,   WinD_(ctx),		0); 
    /*
    **  Remap pixel indices (and add padding).
    */
    new_fid = IdsPixelRemap( fid, lut, attr, roi );

    if( fid != iFid_(rcb) )
	ImgDeleteFrame( fid );
    _ImgCfree( lut->base );
    _ImgCfree( lut );
    IdsFreePutList( &attr );

#ifdef TRACE
printf( "Leaving Routine RemapPixels in module IDS_EXPORT_UTILS \n");
#endif
    return( new_fid );
}

/*******************************************************************************
**  RemapTruePixels
**
**  FUNCTIONAL DESCRIPTION:
**
**	IDS Pipe utility function: re-maps image pixels to allocated colors.
**
**  FORMAL PARAMETERS:
**
**	fid
**	ctx
**	roi
**
*******************************************************************************/
static unsigned long RemapTruePixels( fid, ctx, roi )
 unsigned long fid;
 RenderContext ctx;
 unsigned long roi;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int  stride, s_type, levels[3];
    struct PUT_ITMLST  *attr = 0;
    struct UDP udp;
    unsigned long  new_fid;

#ifdef TRACE
printf( "Entering Routine RemapTruePixels in module IDS_EXPORT_UTILS \n");
#endif

    /*
    **  Build remap item list.
    */
    GetIsl_( fid, Img_Udp, udp, 0 );	/* scanline stride depends on ROI   */
    if( roi != 0 )
	_ImgSetRoi( &udp, roi );
    GetIsl_( fid, Img_SpectType, s_type, 0 );
    stride  = PxlStr_(ctx) * (udp.UdpL_X2 - udp.UdpL_X1 + 1);
    stride += AlignBits_(stride, Pad_(ctx));
    switch( s_type )
        {
    case ImgK_StypeGrayscale :
        /*
        ** prepare itmlst for gray and monochrome images to display on Firefox
        */
    IdsPutItem(&attr, Img_ScanlineStride,   stride,		0);
    IdsPutItem(&attr, Img_SpectralMapping,  ImgK_PrivateMap,	0);
    IdsPutItem(&attr, Img_NumberOfComp,	    1,			0);
    IdsPutItem(&attr, Img_PixelStride,      PxlStr_(ctx),	0);
    IdsPutItem(&attr, Img_BitsPerPixel,	    WinD_(ctx),		0);
    IdsPutItem(&attr, Img_ImgBitsPerComp,   WinD_(ctx),		0); 
    new_fid = IdsPixelTrueRemap( fid, attr, roi );
        break;
    case ImgK_StypeMultispect :
        /*
        ** prepare itmlst for color images to display on Firefox 24 plane
        ** workstation so that it uses Bob's IdsRequantize. May be replaced soon
        */
    IdsPutItem(&attr, Img_ScanlineStride,   stride,	    0);
    IdsPutItem(&attr, Img_NumberOfComp,	    3,		    0);
    IdsPutItem(&attr, Img_PixelStride,      PxlStr_(ctx),   0);
    IdsPutItem(&attr, Img_BitsPerPixel,	    WinD_(ctx),	    0);
    IdsPutItem(&attr, Img_ImgBitsPerComp,   8,		    0);
    IdsPutItem(&attr, Img_ImgBitsPerComp,   8,		    1);
    IdsPutItem(&attr, Img_ImgBitsPerComp,   8,		    2);
    iRGB_(rcb)[0] = iRGB_(rcb)[1] = iRGB_(rcb)[2] = 256;          
    new_fid = IdsRequantize( fid, attr, roi, iRGB_(rcb) );
        break;
        }/* end of case */
 
    if( fid != iFid_(rcb) )
	ImgDeleteFrame( fid );
    IdsFreePutList( &attr );
    return( new_fid );

#ifdef TRACE
printf( "Leaving Routine RemapTruePixels in module IDS_EXPORT_UTILS \n");
#endif
}
#endif

/*******************************************************************************
**  PadScanlines
**
**  FUNCTIONAL DESCRIPTION:
**
**	IDS Pipe utility function: adds padding for X11, and/or handles ROI.
**
**  FORMAL PARAMETERS:
**
**	fid
**	ctx
**	roi
**
**  FUNCTION VALUE:
**
**	returns fid passed in.
**
*******************************************************************************/
static unsigned long PadScanlines( fid, ctx, roi )
 unsigned long fid;
 RenderContext ctx;
 unsigned long roi;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    unsigned long  pad, stride, tmp_fid, wrk_fid = fid;
    struct PUT_ITMLST  *attr = 0;

#ifdef TRACE
printf( "Entering Routine PadScanlines in module IDS_EXPORT_UTILS \n");
#endif

    /*
    **  Release the colors allocated during a previous rendering and clear the
    **  window to the background color.  The latter prevents a Bitonal image 
    **	from being displayed in technicolor.
    **  If the visual type is StaticColor colors are not freed.
    */
#ifndef IDS_NOX
    if( Vis_(ctx) != 0  && Vis_(ctx)->class != StaticColor && 
						Vis_(ctx)->class != StaticGray )
        FreePixelList( ctx, TRUE );
#endif

    GetIsl_( fid, Img_ScanlineStride, stride, 0 );

    pad	    = AlignBits_(stride, Pad_(ctx));
    stride += pad;

	/*
	**  Copy the image to: add padding, or extract ROI, or both.
	*/
    if( pad != 0 || roi != 0)
	{
	IdsPutItem( &attr, Img_ScanlineStride, stride, 0 );
	if ( roi != 0 )
	    {
            tmp_fid = ImgCopy( fid, (struct ROI *)roi, attr );
	    wrk_fid = ImgCvtAlignment( tmp_fid, attr, 0 );
	    ImgDeleteFrame( tmp_fid );
	    }
	else
	    wrk_fid = ImgCvtAlignment( fid, attr, 0 );
	if( fid != iFid_(rcb) )
	    ImgDeleteFrame( fid );
	IdsFreePutList( &attr );
	}		

#ifdef TRACE
printf( "Leaving Routine PadScanlines in module IDS_EXPORT_UTILS \n");
#endif

    return( wrk_fid );

/*    return( fid ); */
}

/*******************************************************************************
**  ExportImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	IDS Pipe utility function: exports the image into an XImage structure.
**
**  FORMAL PARAMETERS:
**
**	fid
**	ctx
**
**  FUNCTION VALUE:
**
**	returns fid passed in
**
*******************************************************************************/
static unsigned long ExportImage( 
     unsigned long  fid
    ,RenderContext  ctx
    ,Boolean	    DoPlaneSwap
    )
{
#ifndef IDS_NOX
    IdsRenderCallback rcb = PropRnd_(ctx);
    unsigned long  length, wrk_fid = fid;

#ifdef TRACE
printf( "Entering Routine ExportImage in module IDS_EXPORT_UTILS \n");
#endif

    /*
    ** To support PC's with Ximage fields bitmap_bit_order being ||
    ** byte_order  being MSB
    */
    if( fid == iFid_(rcb) && ( Image_(ctx)->byte_order       == MSBFirst
                          ||   Image_(ctx)->bitmap_bit_order == MSBFirst ))
        /*
        ** Copy image so we can swap bits and/or bytes and keep org fid in tact
        */
        wrk_fid = ImgCopy( fid, 0, 0 );
 
    /*
    **  Load ISL image frame attributes into the X11 XImage structure.
    */
    GetIsl_( wrk_fid, Img_PlaneDataBase,  Image_(ctx)->data,           0 );
    GetIsl_( wrk_fid, Img_DataOffset,     Image_(ctx)->xoffset,        0 );
    GetIsl_( wrk_fid, Img_BitsPerPixel,   Image_(ctx)->depth,          0 );
    GetIsl_( wrk_fid, Img_PixelStride,    Image_(ctx)->bits_per_pixel, 0 );
    GetIsl_( wrk_fid, Img_PixelsPerLine,  Image_(ctx)->width,          0 );
    GetIsl_( wrk_fid, Img_NumberOfLines,  Image_(ctx)->height,         0 );
    GetIsl_( wrk_fid, Img_ScanlineStride, Image_(ctx)->bytes_per_line, 0 );
     
    /*
    **  Convert ISL scanline stride to X11 bytes per line.
    */
    Image_(ctx)->bytes_per_line >>= 3;
    length = Image_(ctx)->bytes_per_line * Image_(ctx)->height;

    /*
    **  Set the XImage format according to its depth.
    */
    Image_(ctx)->format = Image_(ctx)->depth > 1 ? ZPixmap : XYBitmap;

    if( Image_(ctx)->bitmap_bit_order == MSBFirst
	       && Image_(ctx)->format != ZPixmap )
	/*
	**  Pre-reverse bits if server expects image data to be MSBitFirst.
	**  (X11 only specifies bit reversal for XYBitmap and XYPixmap formats).
	*/
	_IpsMovtcLong( length, Image_(ctx)->data, 0, IMG_AB_BIN_RVSD,
			 length, Image_(ctx)->data );

    if( Image_(ctx)->byte_order != Image_(ctx)->bitmap_bit_order
	 && Image_(ctx)->format != ZPixmap )
	/*
	**  Pre-reverse bytes if server expects image data to be MSByteFirst.
	**  For XYBitmap and XYPixmap formats this is done per 'bitmap unit'.
	*/
	switch( Image_(ctx)->bitmap_unit )
	    {
	case 32 :
	    SwapFourBytes_(Image_(ctx)->data, length);
	    break;
	case 16 :
	    SwapTwoBytes_(Image_(ctx)->data, length);
	    break;
	    }
    if( Image_(ctx)->byte_order == MSBFirst && Image_(ctx)->format == ZPixmap )
	/*
	**  Pre-reverse bytes if server expects image data to be MSByteFirst.
	**  For ZPixmap format this is done per pixel.  X11 also allows the
	**  swapping of nibbles within bytes for 4 bit pixels.
	*/
	switch( Image_(ctx)->bits_per_pixel )
	    {
	case 32 :
	    SwapFourBytes_(Image_(ctx)->data, length);
	    break;
	case 24 :
	    SwapThreeBytes_(Image_(ctx)->data, length);
	    break;
	case 16 :
	    SwapTwoBytes_(Image_(ctx)->data, length);
	    break;
	case 4 :
	    _IpsMovtcLong( length, Image_(ctx)->data, 0, IMG_AB_NIB_RVSD,
			     length, Image_(ctx)->data );
	    break;
	    }

    if (DoPlaneSwap)
      PlaneSwapByData(fid,ctx);
#endif

#ifdef TRACE
printf( "Leaving Routine ExportImage in module IDS_EXPORT_UTILS \n");
#endif
    return( fid );
  }

/*******************************************************************************
**  IdsPlaneSwapByPtr
**
**  FUNCTIONAL DESCRIPTION:
**
**	IDS Pipe utility function: Tap the export pipe to get 
**
**
**  FORMAL PARAMETERS:
**
**	fid     - frame id of the source image
**	ctx	- rendering context
**
**  FUNCTION VALUE:
**
**	None
**
**  SIDE EFFECTS:
**
**	Swaps UDP pointers for Red and Blue Planes
**
*******************************************************************************/
unsigned long IdsPlaneSwapByPtr( 
     unsigned long  fid
    ,RenderContext  ctx
    ,Boolean	    copy_fid
    ,unsigned long  roi
    )
{
  int num_comp, red_levels, blue_levels;
  char *red_plane, *blue_plane;
  unsigned long wrkfid = fid;

#ifdef TRACE
printf( "Entering Routine IdsPlaneSwapByPtr in module IDS_EXPORT_UTILS \n");
#endif

  GetIsl_(fid, Img_NumberOfComp, num_comp, 0 );

  if (num_comp == 3) 
    {
     _ImgGet(wrkfid,Img_QuantLevelsPerComp,&red_levels,sizeof(red_levels),0,0);
     _ImgGet(wrkfid,Img_QuantLevelsPerComp,&blue_levels,sizeof(blue_levels),0,2);

      if (red_levels == 256 & blue_levels == 256)
	{
	  if (copy_fid)
	    wrkfid = ImgCopyFrame(fid,0);
	  if (roi != 0)
	    ImgSetRectRoi(wrkfid, roi, 0);
    
	  red_plane = ImgDetachDataPlane(wrkfid,0);
	  blue_plane = ImgDetachDataPlane(wrkfid,2);
	  ImgAttachDataPlane(wrkfid,red_plane,2);
	  ImgAttachDataPlane(wrkfid,blue_plane,0);
	}
    }

#ifdef TRACE
printf( "Leaving Routine IdsPlaneSwapByPtr in module IDS_EXPORT_UTILS \n");
#endif
  return wrkfid;
}

/*******************************************************************************
**  PlaneSwapByData
**
**  FUNCTIONAL DESCRIPTION:
**
**      Swap the red and blue pixels in a band interleaved by pixel image.
**      This is done only when displaying on DECstation 5000 Model 200 (PXG)
**      because images are BGR, not RGB as is the normal case.
**
**      This routine is only used in the RGB (8/8/4 levels) case.  In the
**      256/256/256 levels case, IDS swaps the planes at the end of the
**      render pipe.
**
**  FORMAL PARAMETERS:
**
**	fid     - frame id of the source image
**	ctx	- rendering context
**
**  FUNCTION VALUE:
**
**	fid     - returns fid passed in
**
**  SIDE EFFECTS:
**
**	Modifies the XImage data buffer that will be sent to the X server.
**
*******************************************************************************/
#ifndef IDS_NOX
static void PlaneSwapByData( fid, ctx )
 unsigned long fid;
 RenderContext ctx;
{
  int            i;
#ifdef sparc
  unsigned long  red_mask    = 0xFF00;
  unsigned long  green_mask  = 0xFF0000;
  unsigned long  blue_mask   = 0xFF000000;
  int            red_shift   = 8;
  int            green_shift = 16;
  int            blue_shift  = 24;
#else
  unsigned long  red_mask    = 0xFF;
  unsigned long  green_mask  = 0xFF00;
  unsigned long  blue_mask   = 0xFF0000;
  int            red_shift   = 0;
  int            green_shift = 8;
  int            blue_shift  = 16;
#endif
  int            blue2red_shift = blue_shift - red_shift;
  unsigned long *srcptr      = (unsigned long *) Image_(ctx)->data;
  unsigned char *bufptr      = NULL;
  unsigned char *savptr      = NULL;
  int            bufsiz      = 0;

#ifdef TRACE
printf( "Entering Routine PlaneSwapByData in module IDS_EXPORT_UTILS \n");
#endif

  /*
  **  First thing is to save the red plane in a spare buffer
  */
  bufsiz = Image_(ctx)->width * Image_(ctx)->height;
  bufptr = (unsigned char *) _ImgCalloc(sizeof(char),bufsiz);
  savptr = bufptr;
  
  if (red_shift != 0)
    for (i = 0; i < bufsiz; i++)
      *bufptr++ = (*srcptr++ & red_mask) >> red_shift;
  else
    for (i = 0; i < bufsiz; i++)
      *bufptr++ = *srcptr++ & red_mask;

  bufptr = savptr;
  srcptr = (unsigned long *) Image_(ctx)->data;

  /*
  ** Writing the data back out swapping blue and red data plane.
  ** Green plane doesn't change.
  **
  ** NOTE:  I'm using the fact Blue is already more significant 
  ** (bit and byte wise) in the pixel than Red.  To be RGB order independent,
  ** 'blue2red_shift' below should be replaced by ABS(blue_shift - red_shift).
  */
  for (i = 0; i < bufsiz; i++)
    {
      *srcptr = (*srcptr & green_mask) | 
	((*srcptr & blue_mask) >> blue2red_shift) | 
	  (*bufptr++ << blue_shift);
      srcptr++;
    }

  bufptr = savptr;
  _ImgCfree(bufptr);

#ifdef TRACE
printf( "Leaving Routine PlaneSwapByData in module IDS_EXPORT_UTILS \n");
#endif

  return;
}
#endif
