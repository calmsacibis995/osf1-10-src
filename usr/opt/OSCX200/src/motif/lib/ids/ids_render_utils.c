
/***********************************************************************
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
**      This module contains rendition logic common to both widgets and 
**	hardcopy rendition modules.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	DECwindows V1.0
**
**  AUTHOR(S):
**
**	Robert NC Shelley
**      John Weber
**      Subu Garikapati
**
**  CREATION DATE:
**
**      June 2, 1988
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
#include <math.h>			    /* math routines		    */
#include <errno.h>

    /*
    **  ISL and IDS include files
    */
#include    <img/ImgDef.h>     /* ISL public symbols			    */
#include    <img/ImgEntry.h>   /* ISL public entry points		    */
#include    <img/ChfDef.h>     /* Condition handling functions               */
#include    <img/IdsStatusCodes.h>   /* IDS status codes                     */

/* Remember, this is going to break when rich p. removes the udp out */

#ifndef IDS_NOX
#include    <ids__widget.h> /* IDS public/private, Dwtoolkit, Xlib defs.    */
#else
#include    <ids__widget_nox.h> /* IDS public/private, Dwtoolkit, Xlib defs.    */
#endif
#ifndef NODAS_PROTO
#include <idsprot.h>            /* IDS prototypes */
#endif


/*
**  Table of contents
*/
    /*
    **	Functions which generate the IDS rendering model.
    */
#ifdef NODAS_PROTO
void		    IdsApplyModel();		/* apply IDS rendering model*/
static void		InitModel();		/* init ApplyModel data	    */
static void		SetComponentLevels();	/* set levels/component	    */
static void		SetClassCvt();		/* set spectral conversion  */
static void		SetRGBtoMonochrome();	/* set RGB conversion	    */
static void		PreSetScale();		/* calc scale before rotate */
static void		SetAngle();		/* set rotation angle	    */
static void		SetFlip();		/* set flip		    */
static void		PostSetScale();		/* calc scale after rotate  */
static void		SetProtocol();		/* HACK for GPX pixmaps	    */
static void             SetSharpen();           /* set sharpen adjustment   */
static void		SetToneScale();		/* set tone-scale adjustment*/
static void             SetMonoToneScale();     /* set gray tone scale adj  */
static void             SetMultiToneScale();    /* set color tone scale adj */
static void		SetDither();		/* dither driver	    */
static void		SetMonoDither();	/* set grayscale dither	    */
static void		SetMultiSpectDither();	/* set multispectral dither */
static void		SetSpaceConvert();	/* component space convert  */
static void		SetPlaneSwap();	        /* swap red and blue planes */

    /*
    **	Functions which compile the IDS rendering model.
    */
IdsPipeDesc	    IdsCompileRendering();	/* compile routines & args  */
static void             LoadExportPipeTap();    /* Pipe <== ExportPipeTap   */
static unsigned long   *LoadPlaneSwap();	/* Pipe <== ImgDetachDataPlane */
static unsigned long   *LoadClassCvt();		/* Pipe <== IdsRequantize   */
static unsigned long   *LoadRotate();		/* Pipe <== ImgRotate	    */
static unsigned long   *LoadFlip();		/* Pipe <== ImgFlip	    */
static unsigned long   *LoadScale();		/* Pipe <== ImgScale	    */
static unsigned long   *LoadToneScale();	/* Pipe <== IdsToneScale    */
static unsigned long   *LoadSharpen();          /* Pipe <== IdsSharpen      */
static unsigned long   *LoadDither();		/* Pipe <== IdsDither	    */
static unsigned long   *LoadCombineFrame();	/* Pipe <== IdsCombineFrame */
static unsigned long   *LoadSpaceCvt();         /* Pipe <== ImgCvtCompSpaceOrg*/

static unsigned long ExportPipeTap();	/* export fid from export pipe*/

int Error_condition_handler();
#else
PROTO(static void InitModel, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetComponentLevels, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetClassCvt, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetRGBtoMonochrome, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void PreSetScale, (IdsRenderCallback /*rcb*/, ModelData /*model*/));
PROTO(static void SetAngle, (IdsRenderCallback /*rcb*/, ModelData /*model*/));
PROTO(static void SetFlip, (IdsRenderCallback /*rcb*/, ModelData /*model*/));
PROTO(static void PostSetScale, (IdsRenderCallback /*rcb*/, ModelData /*model*/));
PROTO(static void SetProtocol, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetToneScale, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetMonoToneScale, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetMultiToneScale, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetSharpen, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetDither, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetMonoDither, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetMultiSpectDither, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetSpaceConvert, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetPlaneSwap, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void LoadExportPipeTap, (RenderContext /*ctx*/));
PROTO(static unsigned long *LoadPlaneSwap, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long *LoadClassCvt, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long *LoadSpaceCvt, (RenderContext /*ctx*/, unsigned long */*fid*/, unsigned long /*cs_org*/));
PROTO(static unsigned long *LoadRotate, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long *LoadFlip, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long *LoadScale, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long *LoadToneScale, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long *LoadSharpen, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long *LoadDither, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long *LoadCombineFrame, (RenderContext /*ctx*/, unsigned long */*fid*/, Boolean /*do_roi*/));
PROTO(static unsigned long ExportPipeTap, (void));
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
extern unsigned long int ImgCombineFrame(); /* used to reverse polarity     */
extern unsigned long int IdsScale();	    /* scales any pixel aspect ratio*/
extern unsigned long int IdsPixelRemap();   /* remap pixel values	    */
extern unsigned long int IdsToneScale();    /* tone-scale adjustment	    */
extern unsigned long int IdsSharpen();      /* sharpening adjustment        */
extern unsigned long int IdsDither();	    /* call dither or requantize    */
extern unsigned long int IdsRequantize();   /* requantize pixel values	    */
extern void		 IdsPutItem();      /* append item to ISL itmlst    */
extern void		 IdsFreePutList();  /* deallocate ISL itmlst	    */
extern unsigned long     IdsPlaneSwapByPtr();/* swap color planes by ptrs   */
#endif

extern unsigned long int ImgCvtCompSpaceOrg();
 
/* mask array for the GetField_() and PutField_() MACROs */

/* global variables */
                                                                              
/*
**	Local Storage
*/
static IdsErrorFunc IdsErrorCb;            /* addr ptr to jump to widget world    */

/*******************************************************************************
**  IdsApplyModel
**
**  FUNCTIONAL DESCRIPTION:
**
**      Apply IDS rendering model to: determine which ISL and IDS functions to
**	call, calculate their parameters, and determine their order of call.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**
*******************************************************************************/
void IdsApplyModel( ctx )
 RenderContext ctx;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    ModelDataStruct   model;

#ifdef TRACE
printf( "Entering Routine IdsApplyModel in module IDS_RENDER_UTILS \n");
#endif


#if !(defined sparc) && !(defined __osf__) 
    ChfEstablish( Error_condition_handler );
#endif

    /*
    ** copy the func addr of error struct in a global value
    */
    if( Error_(ctx) != 0 )
        IdsErrorCb = Error_(ctx);

    InitModel( ctx, &model );		/* initialize dynamic model info    */
    SetClassCvt( ctx, &model );		/* set spectral class conversion    */

    switch( iRender_(rcb) == Ids_Override ? Ids_Scale : iSMode_(rcb) )
	{
    case Ids_NoScale:
    case Ids_Scale:
    case Ids_Physical:
    case Ids_AspectOnly:
	/*
	**  Sequence for scale modes which have priority over rotation.
	*/
	PreSetScale( rcb, &model );	/* set scale factors		    */
	SetAngle( rcb, &model );	/* set rotation angle		    */
	SetFlip( rcb, &model );		/* set flip			    */
	break;
    case Ids_FitWithin:
    case Ids_FitWidth:
    case Ids_FitHeight:
    case Ids_Flood:
	/*
	**  Sequence for scale modes which have lower priority than rotation.
	*/
	SetAngle( rcb, &model );	/* set rotation angle		    */
	SetFlip( rcb, &model );		/* set flip			    */
	PostSetScale( rcb, &model );	/* set scale factors		    */
	break;
    default:
#ifndef IDS_NOX
	XtWarningMsg("InvSclMod","PreSetScale","IdsImageError",
		     "invalid Scale Mode", NULL,NULL);
#endif
	break;
	}
    SetProtocol( ctx, &model );		/* set presentation protocol	    */
    SetToneScale( ctx, &model );	/* set tone-scale parameters	    */
    SetSharpen( ctx, &model );          /* set sharpen parameters           */
    SetDither( ctx, &model );		/* set dither parameters	    */

    /*
     ** SetSpaceConvert() checks to see if anything above will require a change
     ** to Band Interleaved by Plane.  Otherwise, don't do it.
     */
    SetSpaceConvert( ctx, &model );	/* set class convert parameters	    */

    SetPlaneSwap( ctx, &model );       /* set plane swap parameters        */

#ifdef TRACE
printf( "Leaving Routine IdsApplyModel in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  InitModel
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill in the ModelDataStruct with model-dynamic image parameters.
**
**  FORMAL PARAMETERS:
**
**	ctx	- RenderContext data.
**	model	- address of ModelDataStruct to be initialized.
**
*******************************************************************************/
static void InitModel( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int	width, height, isl_parm, i,j, cnt, levels, polarity;
    struct UDP udp[Ids_MaxComponents];

#ifdef TRACE
printf( "Entering Routine InitModel in module IDS_RENDER_UTILS \n");
#endif


    /*
    **  Determine if image needs to be decompressed and init render scheme.
    */

    GetIsl_( iFid_(rcb), Img_CompressionType, isl_parm, 0 );
    if( isl_parm != ImgK_PcmCompression )
	iScheme_(rcb) = Ids_Decompress;
    else
	iScheme_(rcb) = 0;

    /*
    **  Fill in the image dimensions: bits per pixel, width, and height.
    **  spectral info: type, component count,  and bits per component
    */

    GetIsl_(iFid_(rcb), Img_SpectType, model->spect_type, 0);
    GetIsl_( iFid_(rcb), Img_NumberOfComp, model->spect_cnt, 0);
    for( i = 0, j = 0, model->pixel_bits = 0; i < model->spect_cnt; ++i,++j )
     {
       GetIsl_( iFid_(rcb), Img_Udp, udp[i], i );   /* use udps below */
       /* 
       ** Get levels using Img_QuantLevelsPerComp, this
       ** will work for V2 and V3 images
       */
       GetIsl_( iFid_(rcb), Img_QuantLevelsPerComp, levels,i);
       model->levels[j] = levels;
       SetBitsPerComponent_(model->bpc[i], levels);
       model->bpc[i] = ceil( (log10((double)levels)/log10(2.0)));
       model->pixel_bits += model->bpc[i];
     }
    if( iROI_(rcb) != 0 )
	{
        for( i = 0; i < model->spect_cnt; i++ )
  	     _ImgSetRoi( &udp[i], iROI_(rcb) );
	iScheme_(rcb) |= Ids_UseROI;
	}
    model->width      = udp[0].UdpL_X2 - udp[0].UdpL_X1 + 1;
    model->height     = udp[0].UdpL_Y2 - udp[0].UdpL_Y1 + 1;

    /*
     ** Check the brightness bit.  If zero is max intensity (white)
     ** and this is a TrueColor or DirectColor visual and the image
     ** is multispectral, then reverse the polarity.
     **
     ** If the visual is not True or Direct Color the polarity issue
     ** is handled when remapping the pixels into the color map.
     **
     ** The image is band by plane now, so even 8 bit TrueColor should work.
     */
#ifndef IDS_NOX
    GetIsl_(iFid_(rcb),Img_BrtPolarity,polarity,0);
    if (Vis_(ctx) != NULL)
        {
	if ((polarity == ImgK_ZeroMaxIntensity) &&
	   (Vis_(ctx)->class == TrueColor || Vis_(ctx)->class == DirectColor) &&
	   ((model->spect_type == ImgK_ClassMultispect) ||
	   (model->spect_type == ImgK_ClassGreyscale)))
	  iScheme_(rcb) |= Ids_UseReversePolarity;
	}
#endif

    GetIsl_(iFid_(rcb),Img_CompSpaceOrg,model->comp_space_org,0);

    SetComponentLevels( ctx, model );	    /* init level per component    */

#ifdef TRACE
printf( "Leaving Routine InitModel in module IDS_RENDER_UTILS \n");
#endif
}

/*****************************************************************************
**  SetComponentLevels
**
**  FUNCTIONAL DESCRIPTION:
**
**      Determine spectral type and levels per component of rendering.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*****************************************************************************/
static void SetComponentLevels( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    double levels;
    int i, bits, max_bpc, bpc[Ids_MaxComponents];
    int levs = 0;

#ifdef TRACE
printf( "Entering Routine SetComponentLevels in module IDS_RENDER_UTILS \n");
#endif

    /*
    **	Set maximum levels and grayscale levels allowed.
    */
    iLevs_(rcb) = (RClass_(ctx) == Ids_Bitonal) ? 2
      : (FitL_(ctx) != 0) ? FitL_(ctx)
#ifndef IDS_NOX
	: Vis_(ctx) != 0 && 
		  ( Vis_(ctx)->class == TrueColor || Vis_(ctx)->class == DirectColor )
		    ? Cells_(ctx) * Cells_(ctx) * Cells_(ctx) 
#endif
		      : Cells_(ctx);

    iGRA_(rcb)  =  GRA_(ctx) == 0 ? MIN(iLevs_(rcb), 1<<model->pixel_bits )
		:  MIN( GRA_(ctx),  MIN(iLevs_(rcb), 1<<model->pixel_bits ));
    /*
    **  If the rendering class isn't Ids_Color, OR our image isn't color,
    **  THEN no levels will be alotted for the color RGB components.
    */
    if( RClass_(ctx) != Ids_Color || model->spect_cnt < Ids_MaxComponents )
        for( i = 0; i < Ids_MaxComponents; i++ )
            iRGB_(rcb)[i] = 0;
    /*
    **	Set levels per pixel for PostScript
    */
    else if ( Proto_(ctx)  == Ids_PostScript )
    {
        for(bits=0, max_bpc=0, levels = 1.0, i=0; i < Ids_MaxComponents; i++)
            {
            /*
            **
            ** Note: ctx at this point already has the minimum of the
            ** three RGB values that are in the image.  This was already
            ** set in IdsAllocateRenderContext.
	    ** If the user specified RGB values in an item list then 
	    ** use the smallest of the RGB specified levels.
	    ** If this selected level happens to be 8 then set levels to 4,
	    ** else if it selected level is 32 or 64 then set it to 16.  
	    ** This is because Postscript only supports 2, 4, 16 and 256 levels 
	    ** of RGB.
	    ** NOTE:
	    ** When we get a chance to implement Requantize, we should look 
	    ** into setting the next higher supported level instead of
	    ** dithering down.
            **/
            if ( i == 0)
                levs = iRGB_(rcb)[i];
            else if( iRGB_(rcb)[i] < iRGB_(rcb)[i-1] )
                levs = iRGB_(rcb)[i];
            }
        SetBitsPerComponent_( bpc[0], levs );
        switch (bpc[0])
        {
            case 3:  levs = 4; /* if levels is 8 then set to 4 */
                 break;
            case 5:
            case 6: levs = 16; /* if levels is 32 or 64 then set to 16 */
                 break;
        }
        iRGB_(rcb)[0] = iRGB_(rcb)[1] = iRGB_(rcb)[2] = levs;
	SetBitsPerComponent_( bpc[0], levs );
	bpc[1] = bpc[2] = bpc[0];
        /*
        **  Accumulate the number of bits and levels desired.
        */
        max_bpc = bpc[0];
        bits  = bpc[0] * 3;
        levels = iRGB_(rcb)[0] * iRGB_(rcb)[1] * iRGB_(rcb)[2];
	if ( RGB_(ctx)[0] < iRGB_(rcb)[0] )
	    iRGB_(rcb)[0] = iRGB_(rcb)[1] = iRGB_(rcb)[2] = RGB_(ctx)[0];
	iGRA_(rcb)  =  iRGB_(rcb)[0];
    }
    /*
    **  If the rendering class isn't Ids_Color, OR our image isn't color, 
    **	THEN no levels will be alotted for the color RGB components.
    */
    else if( RClass_(ctx) != Ids_Color || model->spect_cnt < Ids_MaxComponents )
	for( i = 0; i < Ids_MaxComponents; i++ )
	    iRGB_(rcb)[i] = 0;
    else
	{
	for(bits=0, max_bpc=0, levels = 1.0, i=0; i < Ids_MaxComponents; i++)
	    {
	    /*
	    **  "RGB_levels_desired" will be least of: the number RGB levels
	    **  in the image or the number of RGB levels requested.
	    */
	    iRGB_(rcb)[i] = iRGB_(ctx)[i] == 0 ? model->levels[i]
			  :  MIN( RGB_(rcb)[i], model->levels[i] );
	    /*
	    **  Accumulate the number of bits and levels desired.
	    */
	    SetBitsPerComponent_( bpc[i], iRGB_(rcb)[i] );
	    max_bpc = MAX( max_bpc, bpc[i] );
	    bits   += bpc[i];
	    levels *= iRGB_(rcb)[i];
	    }

        /*
        ** if  RGB levels requested are 1 or less RGB bits/comp is 0 which ISL
        ** does not support. So display the original image
        */
	if( iRGB_(rcb)[BLU] <= 1 || iRGB_(rcb)[RED] <= 1 || 
                                                          iRGB_(rcb)[GRN] <= 1)	
	    for( i=0; i < Ids_MaxComponents; i++)
		{
		/*
                ** Since ISL at this time does not support 1 level of any
                ** rgb component this case has to be filtered out. So in 
                ** this case the original image is displayed with a warning
		**  "RGB_levels_desired" will be the levels in the image 
		*/
		iRGB_(rcb)[i] = model->levels[i];
#ifndef IDS_NOX
		XtWarningMsg("InvRGBLevs","SetComponentLevels",
			    "IdsImageError","invalid ISL RGB Levels",0,0);
#endif
		}
	if( max_bpc > 25 || levels > iLevs_(rcb) )
	    {
	    /*
	    **  The bits_per_component or number of levels exceeds our limits.
	    **  IDS will choose the "default_bits_per_component" by dividing
	    **  the hardware limit, WinD_(), between the components as follows: 
	    **	BLU_bits = floor( WinD_() / 3 ),	   (least bits)
	    **	GRN_bits =  ceil( WinD_() / 3 ),	    (most bits)
	    **	RED_bits = WinD_() - GRN_bits - BLU_bits. (remainder)
	    */
	    SetBitsPerComponent_( bpc[RED], 1<<WinD_(ctx) );
	    bpc[BLU]  = bpc[RED] / model->spect_cnt;
	    bpc[GRN]  = ceil( (double) bpc[RED] / model->spect_cnt );
	    bpc[RED] -= bpc[GRN] + bpc[BLU];
	    /*
	    **  The "default_RGB_levels" will be the least of:
	    **  - "RGB_levels_desired",
	    **  -  RGB_levels supported by "default_bits_per_component",
	    **  -  a portion of iLevs_() computed as follows:
	    **	BLU_levels = (int)   cube_root( iLevs_() )	       (least)
	    **	RED_levels = (int) square_root( iLevs_() / BLU_levels )
	    **	GRN_levels = (int) iLevs_() / RED_levels / BLU_levels	(most)
	    */
            /*
            ** Although errno is set here specifically to solve a problem
            ** with shared libs on OSF Silver (see qar 3265 in OSF_QAR), it
            ** should be used to check the results of the pow() call.
            */
            errno = 0;

	    iRGB_(rcb)[BLU] = MIN( iRGB_(rcb)[BLU], MIN( 1<<bpc[BLU],
			   (int)pow( (double) iLevs_(rcb), 1.0/3.0 )));
	    iRGB_(rcb)[RED] = MIN( iRGB_(rcb)[RED], MIN( 1<<bpc[RED],
			   (int)sqrt((double) iLevs_(rcb) / iRGB_(rcb)[BLU])));
	    iRGB_(rcb)[GRN] = MIN( iRGB_(rcb)[GRN], MIN( 1<<bpc[GRN],
			    iLevs_(rcb) / iRGB_(rcb)[RED] / iRGB_(rcb)[BLU]));

            /*
            ** If the RGB_levels_desired is less than 8 than one of the comp
            ** would have 1 level which ISL does not support.
            */
            if( iRGB_(rcb)[BLU] == 1 || iRGB_(rcb)[RED] == 1 ||
					    iRGB_(rcb)[GRN] == 1)	
	    	for( i=0; i < Ids_MaxComponents; i++)
		    {
		    /*
                    ** Since ISL at this time does not support 1 level of any
                    ** rgb component this case has to be filtered out. So in 
                    ** this case the original image is displayed with a warning
		    **  "RGB_levels_desired" will be the levels in the image 
		    */
		    iRGB_(rcb)[i] = model->levels[i];
#ifndef IDS_NOX
		    XtWarningMsg("InvRGBLevs","SetComponentLevels",
				"IdsImageError","invalid ISL RGB Levels",0,0);
#endif
		    }

	    }
	}
#ifdef TRACE
printf( "Leaving Routine SetComponentLevels in module IDS_RENDER_UTILS \n");
#endif
}

/*****************************************************************************
**  SetClassCvt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Driver for spectral class conversion -- e.g. changing an RGB image
**	to grayscale.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*****************************************************************************/
static void SetClassCvt( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine SetClassCvt in module IDS_RENDER_UTILS \n");
#endif

    IdsFreePutList( &RqLst_(ctx) );

    if( model->spect_type == ImgK_ClassMultispect  &&
      ( iRGB_(rcb)[RED] | iRGB_(rcb)[GRN] | iRGB_(rcb)[BLU] ) < 2 )
	/*
	**  Setup conversion from RGB to monochrome.
	*/
	SetRGBtoMonochrome( ctx, model );

    if( RqLst_(ctx) != NULL )
	/*
	**  Tell compiler to include spectral class conversion.
	*/
	iScheme_(rcb) |= Ids_UseClassCvt;

#ifdef TRACE
printf( "Leaving Routine SetClassCvt in module IDS_RENDER_UTILS \n");
#endif
}

/*****************************************************************************
**  SetRGBtoMonochrome
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set pixel requantization to convert RGB to monochrome.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*****************************************************************************/
static void SetRGBtoMonochrome( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int i, stride;

#ifdef TRACE
printf( "Entering Routine SetRGBtoMonochrome in module IDS_RENDER_UTILS \n");
#endif


    /*
    **	Determine number of output levels and bits per component/pixel.
    **  Since gray levels limit is 256 for Direct and TrueColor MIN is taken
       RqLev_(ctx) = MIN(ClassCvtLevels, iLevs_(rcb));
    */
#ifndef IDS_NOX
    if( Vis_(ctx) != 0 &&  ( Vis_(ctx)->class == DirectColor || Vis_(ctx)->class == TrueColor )  )    
       RqLev_(ctx) = ClassCvtLevels;
    else
       RqLev_(ctx) = MAX(ClassCvtLevels, iLevs_(rcb));
#else
       RqLev_(ctx) = MAX(ClassCvtLevels, iLevs_(rcb));
#endif
    SetBitsPerComponent_( model->bpc[GRA], RqLev_(ctx) );
    model->pixel_bits   = model->bpc[GRA];
    for( i = 1; i < Ids_MaxComponents; model->bpc[i++] = 0 );

    /*
    **	Load up the conversion item list.
    */
    model->spect_type = ImgK_ClassGrayscale;
    model->spect_cnt  = 1;
    stride = MAX(PxlStr_(ctx), model->bpc[GRA]);

    IdsPutItem(&RqLst_(ctx), Img_SpectralMapping,  ImgK_MonochromeMap,   0);
    IdsPutItem(&RqLst_(ctx), Img_NumberOfComp,     model->spect_cnt,     0);
    IdsPutItem(&RqLst_(ctx), Img_ImgBitsPerComp,   model->bpc[GRA],    GRA);
    IdsPutItem(&RqLst_(ctx), Img_BitsPerPixel,     model->pixel_bits,    0);
    IdsPutItem(&RqLst_(ctx), Img_QuantLevelsPerComp,RqLev_(ctx),	 0);
    IdsPutItem(&RqLst_(ctx), Img_PixelStride,      stride,		 0);

    if( RClass_(ctx) == Ids_Bitonal)
    IdsPutItem( &RqLst_(ctx), Img_CompSpaceOrg,  ImgK_BandIntrlvdByPlane, 0);
    /*
    **	Might just as well add scanline padding.
    */
    stride *= model->width;
    stride += AlignBits_(stride, Pad_(ctx));
    IdsPutItem(&RqLst_(ctx), Img_ScanlineStride, stride, 0);

#ifdef TRACE
printf( "Leaving Routine SetRGBtoMonochrome in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  PreSetScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set scale factors for scale modes which have priority over rotation.
**
**  FORMAL PARAMETERS:
**
**	rcb	- rendering parameters contained in a IdsRenderCallback struct.
**	model	- dynamic model data.
**
*******************************************************************************/
static void PreSetScale( rcb, model )
 IdsRenderCallback rcb;
 ModelData	   model;
{
    int wide, high, ll_x, ll_y, ur_x, ur_y;
    int pp_dist, lp_dist;
    double device_par, image_par;               /* Pixel aspect ratios      */

#ifdef TRACE
printf( "Entering Routine PreSetScale in module IDS_RENDER_UTILS \n");
#endif

    if( iRender_(rcb) != Ids_Override )
	switch( iSMode_(rcb) )	
	    {
	case Ids_Physical:
	    GetIsl_( iFid_(rcb), Img_FrmBoxLLX,    ll_x, 0 );
	    GetIsl_( iFid_(rcb), Img_FrmBoxLLY,    ll_y, 0 );
	    GetIsl_( iFid_(rcb), Img_FrmBoxURX,    ur_x, 0 );
	    GetIsl_( iFid_(rcb), Img_FrmBoxURY,    ur_y, 0 );
	    GetIsl_( iFid_(rcb), Img_PixelsPerLine, wide, 0 );
	    GetIsl_( iFid_(rcb), Img_NumberOfLines, high, 0 );
	    iXSc_(rcb) = iXres_(rcb) * (ur_x - ll_x) / wide;
	    iYSc_(rcb) = iYres_(rcb) * (ur_y - ll_y) / high;

	    if( iXSc_(rcb) > 0.0  &&  iYSc_(rcb) > 0.0 )
		break;				    /* dimensions are legal */

	case Ids_AspectOnly:
	    /*
	    **	AspectOnly: Adjust X,Y scale factors to match image pixel
            **  aspect ratio to presentation surface aspect ratio.
	    */
            GetIsl_(iFid_(rcb), Img_PPPixelDist, pp_dist, 0 );
            GetIsl_(iFid_(rcb), Img_LPPixelDist, lp_dist,  0 );
            device_par = iXres_(rcb) / iYres_(rcb);
            image_par  = (double)pp_dist/(double)lp_dist;
            iXSc_(rcb) = iYSc_(rcb) = 1;
            if( pp_dist < lp_dist )   
              {      
		iXSc_(rcb) /= device_par;
		iXSc_(rcb) *= image_par;
              }
            else
              {      
		iXSc_(rcb) /= image_par;
		iXSc_(rcb) *= device_par;
              }
  	    break;

	case Ids_NoScale:
	    iXSc_(rcb) = 1.0;			    /* reset X scale factor */
	    iYSc_(rcb) = 1.0;			    /* reset Y scale factor */
	    }
    model->width  *= iXSc_(rcb);	    /* update width  with X scale   */
    model->height *= iYSc_(rcb);	    /* update height with Y scale   */

    if( iXSc_(rcb) != 1.0  ||  iYSc_(rcb) != 1.0 )
	if( iXSc_(rcb) != iYSc_(rcb) || iXSc_(rcb) * iYSc_(rcb) <= 1.0 )
	    /*
	    **	Scaling asymetrically, or to a smaller area.
	    */
	    iScheme_(rcb) |= Ids_UseScale_1;
	else
	    /*
	    **	Scaling symetrically to a larger area.
	    */
	    iScheme_(rcb) |= Ids_UseScale_2;

#ifdef TRACE
printf( "Leaving Routine PreSetScale in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  SetAngle
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set the rotate angle depending on mode and image dimensions.
**
**  FORMAL PARAMETERS:
**
**	rcb	- rendering parameters contained in a IdsRenderCallback struct.
**	model	- dynamic model data.
**
*******************************************************************************/
static void SetAngle( rcb, model )
 IdsRenderCallback rcb;
 ModelData	   model;
{
    double  ca, sa;
    int	    rw, dst_wide_bmu = iWide_(rcb) / iXres_(rcb);
    int	        dst_high_bmu = iHigh_(rcb) / iYres_(rcb);

#ifdef TRACE
printf( "Entering Routine SetAngle in module IDS_RENDER_UTILS \n");
#endif

    if( iRender_(rcb) != Ids_Override )
	switch( iRMode_(rcb) )
	    {
	case Ids_BestFit :
	    /*
	    **  Test for source and destination having mismatched aspect ratios.
	    */
	    if( dst_wide_bmu != dst_high_bmu  && model->width != model->height
	   && ( dst_wide_bmu  / dst_high_bmu  == 0
	      ^ model->width  / model->height == 0 ))
		{
		iRAng_(rcb) = 90.0;	    /* rotate 90 degrees clockwise  */
		break;
		}
	case Ids_NoRotate :
	    iRAng_(rcb) = 0.0;		    /* no rotation required	    */
	case Ids_Rotate :
	    break;
	default :
#ifndef IDS_NOX
	    XtWarningMsg("InvRotMod","SetAngle","IdsImageError",
			 "invalid Rotate Mode",0,0);
#endif
	    break;
	    }
    ca    = fabs( cos( DEG_RAD * iRAng_(rcb) ));
    sa    = fabs( sin( DEG_RAD * iRAng_(rcb) ));
    rw		  = ca * model->width + sa * model->height;
    model->height = sa * model->width + ca * model->height;
    model->width  = rw;
    if( iRAng_(rcb) != 0.0 )
	iScheme_(rcb) |= Ids_UseAngle;		/* request Angle to be used */

#ifdef TRACE
printf( "Leaving Routine SetAngle in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  SetFlip
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set flip.
**
**  FORMAL PARAMETERS:
**
**	rcb	- rendering parameters contained in a IdsRenderCallback struct.
**	model	- dynamic model data.
**
*******************************************************************************/
static void SetFlip( rcb, model )
 IdsRenderCallback rcb;
 ModelData	   model;
{
#ifdef TRACE
printf( "Entering Routine SetFlip in module IDS_RENDER_UTILS \n");
#endif

    switch( iFlip_(rcb) )
	{
    case 0 :
	break;

    case Ids_FlipVertical   :
    case Ids_FlipHorizontal :
    case Ids_FlipVertical | Ids_FlipHorizontal :
	iScheme_(rcb) |= Ids_UseFlip;		/* request flip to happen   */
	break;

    default :
#ifndef IDS_NOX
	XtWarningMsg("InvFlpMod","SetFlip","IdsImageError",
		     "invalid Flip Options",0,0);
#endif
	break;
	}
#ifdef TRACE
printf( "Leaving Routine SetFlip in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  PostSetScale
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale the image to "fit" or "flood" the specified area.  Fit modes 
**	preserve the aspect ratio, scaling the image to fit completely "within"
**	the area, or make only the "width", or "height" fit the area.  Flood 
**	mode distorts the image to fill the width and height of the area.
**
**  FORMAL PARAMETERS:
**
**	rcb	- rendering parameters contained in a IdsRenderCallback struct.
**	model	- dynamic model data.
**
*******************************************************************************/
static void PostSetScale( rcb, model )
 IdsRenderCallback rcb;
 ModelData	   model;
{
    double rw, rh;
    double device_par, image_par;		/* Pixel aspect ratios	    */
    int pixel_path_distance, line_path_distance;

#ifdef TRACE
printf( "Entering Routine PostSetScale in module IDS_RENDER_UTILS \n");
#endif

    /*
    **	Get image attributes we need.
    */
    GetIsl_(iFid_(rcb), Img_PPPixelDist, pixel_path_distance, 0 );
    GetIsl_(iFid_(rcb), Img_LPPixelDist, line_path_distance,  0 );
    /*
    **	Initialize parameters
    */
    rw = model->width;
    rh = model->height,

    device_par = iXres_(rcb) / iYres_(rcb);
    image_par  = (double)pixel_path_distance / (double)line_path_distance;

    if( iRender_(rcb) != Ids_Override )
	{
	/*
	**  If scale mode other than flood, adjust the image dimensions to
	**  account for presentation surface and image pixel aspect ratio. It
	**  should be sufficient to adjust only one dimension.  The choice is
	**  arbitrary.
	*/
	if (iSMode_(rcb) != Ids_Flood)
	    {
		rw /= image_par;
		rw *= device_par;
	    }
	/*
	**  Now, determine scale factors required to completely fill width and
	**  height of presentation surface.
	*/
	iXSc_(rcb) = iWide_(rcb) / rw + 1.0 / (rw * 2.0);
	iYSc_(rcb) = iHigh_(rcb) / rh + 1.0 / (rh * 2.0);

	/*
	**  Set scale factors based on scale mode
	*/
	switch ( iSMode_(rcb) )
	    {
	case Ids_FitWithin:
	    /*
	    **	FitWithin : choose smaller of two scale factors. Adjust X scale
	    **		    factor for presentation surface aspect ratio.
	    */
	    iXSc_(rcb)  = MIN(iXSc_(rcb), iYSc_(rcb));
	    iYSc_(rcb)  = MIN(iXSc_(rcb), iYSc_(rcb));
	    iXSc_(rcb) /= image_par;
	    iXSc_(rcb) *= device_par;
	    break;

	case Ids_FitWidth:
	    /*
	    **	FitWidth : Unconditionally choose X scale factor. Adjust X scale
	    **		   factor for presentation surface aspect ratio.
	    */
	    iYSc_(rcb)  = iXSc_(rcb);
	    iXSc_(rcb) /= image_par;
	    iXSc_(rcb) *= device_par; 
	    break;

	case Ids_FitHeight:
	    /*
	    **	FitHeight : Unconditionally choose Y scale factor. Adjust X
	    **		    scale factor for presentation surface aspect ratio.
	    */
	    iXSc_(rcb)  = iYSc_(rcb);
	    iXSc_(rcb) /= image_par;
	    iXSc_(rcb) *= device_par;
	    break;

	case Ids_Flood:
	    /*
	    **	Flood : Use computed scale factors as is.
	    */
	    break;
	    }
	}
    model->width  *= iXSc_(rcb);	    /* update width  with X scale   */
    model->height *= iYSc_(rcb);	    /* update height with Y scale   */

    if( iXSc_(rcb) != 1.0  ||  iYSc_(rcb) != 1.0 )
	if( iXSc_(rcb) == iYSc_(rcb) && iXSc_(rcb) * iYSc_(rcb) <= 1.0 )
	    /*
	    **	Scaling symetrically to a smaller area.
	    */
	    iScheme_(rcb) |= Ids_UseScale_1;
	else
	    /*
	    **	Scaling asymetrically, or to a larger area.
	    */
	    iScheme_(rcb) |= Ids_UseScale_2;

#ifdef TRACE
printf( "Leaving Routine PostSetScale in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  SetProtocol
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set presentation protocol.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*******************************************************************************/
static void SetProtocol( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
#ifndef IDS_NOX
#ifdef GPX_HACKS
    IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine SetProtocol in module IDS_RENDER_UTILS \n");
#endif

    /*
    **  For Pixmap protocol we have to see if the hardware supports it.
    */
    if( iProto_(rcb) == Ids_Pixmap && PxlStr_(ctx) > 1
	&& ( model->width  >  WidthOfScreen(Scr_(ctx))
	||   model->height > HeightOfScreen(Scr_(ctx)) ))
	    /*
	    **	Comparing the width and height of the image and to the screen 
	    **	size is a hack to get around a limitation of the GPX server.
	    */
	    iProto_(rcb) = Ids_XImage;
#endif
#endif
#ifdef TRACE
printf( "Leaving Routine SetProtocol in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  SetToneScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      Determine the need for tone-scale adjustment of grayscale images.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*******************************************************************************/
static void SetToneScale( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int stride;

#ifdef TRACE
printf( "Entering Routine SetToneScale in module IDS_RENDER_UTILS \n");
#endif

    IdsFreePutList( &TsLst_(ctx) );		/* free unused item list    */

    switch( model->spect_type )
	{
    case ImgK_ClassGrayscale :
	/*
	**  See if we need grayscale tonescale.
	*/
	if ( iPun1_(rcb) != 0.0 || iPun2_(rcb) != 1.0 )
          {
	    SetMonoToneScale( ctx, model );
            iScheme_(rcb) |= Ids_UseToneScale;
          }
	break;
    case ImgK_ClassMultispect:
	/*
	**  See if we need color tonescale.
	*/
	if ( iPun1_(rcb) != 0.0 || iPun2_(rcb) != 1.0 )
          {
	    SetMultiToneScale( ctx, model );
            iScheme_(rcb) |= Ids_UseToneScale;
          }
	break;
	}
#ifdef TRACE
printf( "Leaving Routine SetToneScale in module IDS_RENDER_UTILS \n");
#endif
}

/*****************************************************************************
**  SetMonoToneScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      Do the needed gray tonescale adjustment.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*****************************************************************************/
static void SetMonoToneScale( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int  stride;

#ifdef TRACE
printf( "Entering Routine SetMonoToneScale in module IDS_RENDER_UTILS \n");
#endif


    /*
    **  Set number of grayscale levels for tone-scale adjustment.
    */
    TsLev_(ctx)[GRA] = MAX(ClassCvtLevels, iLevs_(rcb));
    TsLev_(ctx)[GRN] = TsLev_(ctx)[BLU] = 0;
    /*
    **  Set pixel stride stride.
    */
    stride = MAX(PxlStr_(ctx), model->bpc[GRA]);
    IdsPutItem(&TsLst_(ctx), Img_PixelStride,    stride, 0);

    /*
    **  Reserve room for scanline stride by stashing the scanline pad 
    **  modulo.  The actual scanline stride will be computed during 
    **  pipe execution.
    */
    IdsPutItem( &TsLst_(ctx), Img_ScanlineStride, Pad_(ctx), 0);

#ifdef TRACE
printf( "Leaving Routine SetMonoToneScale in module IDS_RENDER_UTILS \n");
#endif
}

/*****************************************************************************
**  SetMultiToneScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      Do the needed color tonescale adjustment.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*****************************************************************************/
static void SetMultiToneScale( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int  stride,i;

#ifdef TRACE
printf( "Entering Routine SetMultiToneScale in module IDS_RENDER_UTILS \n");
#endif


    /*
    **  Set number of color levels for tone-scale adjustment and load the 
    **  list with bits per component/pixel. 8 bits per component/pixel
    */
    for( model->pixel_bits = 0, i = 0; i < Ids_MaxComponents; i++ )
        {
        TsLev_(ctx)[i] = MAX(ClassCvtLevels, iLevs_(rcb));
/*        iRGB_(rcb)[i] = MIN(1<<model->bpc[i], iRGB_(rcb)[i]);     */
        SetBitsPerComponent_(model->bpc[i], TsLev_(ctx)[i]); 
        IdsPutItem( &TsLst_(ctx), Img_ImgBitsPerComp, model->bpc[i], i);
        model->pixel_bits += model->bpc[i];
        }
  
    /*
    **  Set pixel stride stride.
    */
    stride  = MAX(PxlStr_(ctx), model->pixel_bits);
    IdsPutItem( &TsLst_(ctx), Img_PixelStride,    stride, 0);

    /*
    **  Reserve room for scanline stride by stashing the scanline pad 
    **  modulo.  The actual scanline stride will be computed during 
    **  pipe execution.
    */
    IdsPutItem( &TsLst_(ctx), Img_ScanlineStride, Pad_(ctx), 0);

#ifdef TRACE
printf( "Leaving Routine SetMultiToneScale in module IDS_RENDER_UTILS \n");
#endif
}


/*******************************************************************************
**  SetSharpen
**
**  FUNCTIONAL DESCRIPTION:
**
**      Determine the need for sharpening adjustment of grayscale and color 
**      images.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*******************************************************************************/
static void SetSharpen( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int stride;

#ifdef TRACE
printf( "Entering Routine SetSharpen in module IDS_RENDER_UTILS \n");
#endif

    switch( model->spect_type )
	{
    case ImgK_ClassGrayscale :
	/*
	**  See if we need grayscale sharpening.
	*/
	if ( iSharp_(rcb) != 0.0 )
            iScheme_(rcb) |= Ids_UseSharpen;
	break;
    case ImgK_ClassMultispect:
	/*
	**  See if we need color sharpening.
	*/
	if ( iSharp_(rcb) != 0.0 )
            iScheme_(rcb) |= Ids_UseSharpen;
	break;
	}
#ifdef TRACE
printf( "Leaving Routine SetSharpen in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  SetDither
**
**  FUNCTIONAL DESCRIPTION:
**
**      Driver for setting dither parameters.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*******************************************************************************/
static void SetDither( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine SetDither in module IDS_RENDER_UTILS \n");
#endif

    IdsFreePutList( &DiLst_(ctx) );		/* free unused item list    */

    switch( model->spect_type )
	{
    case ImgK_ClassGrayscale :
	/*
	**  See if we need grayscale dither.
	*/
	if( model->pixel_bits > 25 || 1<<model->pixel_bits > iGRA_(rcb) )
	    SetMonoDither( ctx, model );
	break;
    case ImgK_ClassMultispect:
	/*
	**  See if we need color dither.
	*/
	if( model->levels[RED] > iRGB_(rcb)[RED]
	 || model->levels[GRN] > iRGB_(rcb)[GRN]
	 || model->levels[BLU] > iRGB_(rcb)[BLU] )
	    SetMultiSpectDither( ctx, model );
	break;
	}

    if( DiLst_(ctx) != NULL )
	iScheme_(rcb) |= Ids_UseDither;
#ifdef TRACE
printf( "Leaving Routine SetDither in module IDS_RENDER_UTILS \n");
#endif
}

/*****************************************************************************
**  SetMonoDither
**
**  FUNCTIONAL DESCRIPTION:
**
**      Determine the need for grayscale dithering.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*****************************************************************************/
static void SetMonoDither( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int stride;

#ifdef TRACE
printf( "Entering Routine SetMonoDither in module IDS_RENDER_UTILS \n");
#endif

    /*
    **  Load the dither item list...
    */
    SetBitsPerComponent_( model->bpc[GRA], iGRA_(rcb));
    model->pixel_bits   = model->bpc[GRA];
    IdsPutItem( &DiLst_(ctx), Img_ImgBitsPerComp, model->bpc[GRA], GRA);
    IdsPutItem( &DiLst_(ctx), Img_BitsPerPixel,    model->pixel_bits, 0);

    /*
    **  Load item list with pixel stride.
    */
    stride = model->bpc[GRA] == 1 ? 1 : MAX(PxlStr_(ctx), model->bpc[GRA]);
    IdsPutItem( &DiLst_(ctx), Img_PixelStride,    stride, 0);

/*    IdsPutItem( &DiLst_(ctx), Img_CompSpaceOrg,  ImgK_BandIntrlvdByPlane, 0);*/
    /*
    **  Reserve room for scanline stride by stashing the scanline pad modulo.
    **	The actual scanline stride will be computed during pipe execution.
    */
    IdsPutItem( &DiLst_(ctx), Img_ScanlineStride, Pad_(ctx), 0);

#ifdef TRACE
printf( "Leaving Routine SetMonoDither in module IDS_RENDER_UTILS \n");
#endif
}

/*****************************************************************************
**  SetMultiSpectDither
**
**  FUNCTIONAL DESCRIPTION:
**
**      Determine the need for color dithering.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*****************************************************************************/
static void SetMultiSpectDither( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    int i, stride;

#ifdef TRACE
printf( "Entering Routine SetMultiSpectDither in module IDS_RENDER_UTILS \n");
#endif

    /*
    **  Load item list with bits per component/pixel.
    */
    for( model->pixel_bits = 0, i = 0; i < model->spect_cnt; i++ )
	{
	model->levels[i] = iRGB_(rcb)[i] = MIN(1<<model->bpc[i], iRGB_(rcb)[i]);
	SetBitsPerComponent_(model->bpc[i], iRGB_(rcb)[i]);
	IdsPutItem( &DiLst_(ctx), Img_ImgBitsPerComp, model->bpc[i], i);
        IdsPutItem( &DiLst_(ctx), Img_QuantLevelsPerComp, model->levels[i], i);
        /*
        **  Load item list with pixel stride.
        */
        stride  = MAX(PxlStr_(ctx), model->bpc[i]);
        IdsPutItem( &DiLst_(ctx), Img_PixelStride,    stride, 0);
        /*
        **  Reserve room for scanline stride by stashing the scanlin pad modulo.
        **  The actual scanline stride will be computed during pipe execution.
        */
        IdsPutItem( &DiLst_(ctx), Img_ScanlineStride, Pad_(ctx), i);
	}

    iGRA_(rcb) = 0;				    /* no levels of gray    */
#ifdef TRACE
printf( "Leaving Routine SetMultiSpectDither in module IDS_RENDER_UTILS \n");
#endif
}

/*******************************************************************************
**  SetSpaceConvert
**
**  FUNCTIONAL DESCRIPTION:
**
**      Driver for setting component space conversion parameters.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*******************************************************************************/
static void SetSpaceConvert( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
  IdsRenderCallback rcb = PropRnd_(ctx);
  unsigned long allISLroutines = Ids_UseClassCvt | Ids_UseScale_1 | 
    Ids_UseFlip | Ids_UseAngle    | Ids_UseScale_2 | Ids_UseToneScale | 
      Ids_UseDither | Ids_UseReversePolarity;

#ifdef TRACE
printf( "Entering Routine SetSpaceConvert in module IDS_RENDER_UTILS \n");
#endif

  if ((allISLroutines & iScheme_(rcb)) &&
      (model->comp_space_org != ImgK_BandIntrlvdByPlane))
    iScheme_(rcb) |= Ids_UseNativeFormatCvt;


  if (((model->comp_space_org != ImgK_BandIntrlvdByPixel) ||
      (iScheme_(rcb) & Ids_UseNativeFormatCvt)) &&
      (model->spect_type != ImgK_ClassBitonal) )
    iScheme_(rcb) |= Ids_UseOldFormatCvt;
#ifdef TRACE
printf( "Leaving Routine SetSpaceConvert in module IDS_RENDER_UTILS \n");
#endif
}

static void SetPlaneSwap( ctx, model )
 RenderContext ctx;
 ModelData     model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
#if !defined(IDS_NOX)
    Visual *visual = Vis_(ctx);

#ifdef TRACE
printf( "Entering Routine SetPlaneSwap in module IDS_RENDER_UTILS \n");
#endif
    /*
     **  Pipe <== IDS function PlaneSwapByPtr: returns fid.
     */
   if (visual != NULL)
   {
    if (((visual->class == TrueColor) || (visual->class == DirectColor)) &&
	(RClass_(ctx) == Ids_Color) &&
	!((Image_(ctx)->red_mask < Image_(ctx)->green_mask) &&
	  (Image_(ctx)->green_mask < Image_(ctx)->blue_mask)))
	{
	if ((iRGB_(rcb)[RED] == 256) && 
	    (iRGB_(rcb)[GRN] == 256) && 
	    (iRGB_(rcb)[BLU] == 256))
	    iScheme_(rcb) |= Ids_UsePlaneSwapByPtr;

	if ((iRGB_(rcb)[RED] == 8) && 
	    (iRGB_(rcb)[GRN] == 8) && 
	    (iRGB_(rcb)[BLU] == 4))
	    iScheme_(rcb) |= Ids_UsePlaneSwapByData;
	}
   }
#endif
#ifdef TRACE
printf( "Leaving Routine SetPlaneSwap in module IDS_RENDER_UTILS \n");
#endif
  }

/*******************************************************************************
**  IdsCompileRendering
**
**  FUNCTIONAL DESCRIPTION:
**
**	Compile an IDS rendering Pipe according to the Rendering Scheme.
**	The FIFO rendering pipe will contain the IDS and ISL functions 
**	(and arguments) needed to render the image as currently specified.
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
IdsPipeDesc  IdsCompileRendering( 
     RenderContext  ctx
    ,unsigned long  *fid_pointer
    ,Boolean	    init
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine IdsCompileRendering in module IDS_RENDER_UTILS \n");
#endif

#if !(defined sparc) && !(defined __osf__) 
    ChfEstablish( Error_condition_handler );
#endif

    PDcopiedROI_(pd) = iScheme_(rcb) & Ids_UseROI ? FALSE : TRUE;
    PDfid_(pd) = fid_pointer; 
    if( init )
	{
	PDpipe_(pd) = PDbase_(pd);
	PDpipe_(pd)->flink = NULL;
	}

    /*
     **  Pipe <== ISL function ImgDecompress: ignore returned fid.
     */
    if (iScheme_(rcb) & Ids_Decompress)
      {
	InsertPipe_( pd, Ids_FunctionDecompress, IdsVoid, ImgDecompressFrame,
		    (ArgByPtr_( PDfid_(pd) ), 
		     ArgByVal_(ImgM_InPlace), 
		     ArgByVal_(0), 
		     EndArgs_ ));
      }

    /*
     **  Pipe <== IDS function ImgCvtCompSpaceOrg: returns tmp fid.
     */
    IfReplace_ (iScheme_(rcb) & Ids_UseNativeFormatCvt, PDfid_(pd),
		LoadSpaceCvt(ctx, PDfid_(pd),ImgK_BandIntrlvdByPlane));

    /*
     **  Pipe <== IDS function IdsRequantize: returns tmp fid.
     */
    if (iScheme_(rcb) & Ids_UseClassCvt)
      {
	/*
	 **  Pipe <== ISL function ImgCvtCompSpaceOrg : returns tmp fid.
	 */
	IfReplace_ (1, PDfid_(pd),
		    LoadSpaceCvt(ctx, PDfid_(pd),ImgK_BandIntrlvdByPixel));

	PDcopiedROI_(pd) |= IfReplace_( iScheme_(rcb) & Ids_UseClassCvt,  PDfid_(pd),
			LoadClassCvt( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));
      }

    /*
    **  Pipe <== ISL function ImgScale: returns tmp fid.
    */
    PDcopiedROI_(pd) |= IfReplace_( iScheme_(rcb) & Ids_UseScale_1,  PDfid_(pd),
			  LoadScale( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));

    /*
    **  Pipe <== ISL function ImgRotate: returns tmp fid.
    */
    PDcopiedROI_(pd) |= IfReplace_( iScheme_(rcb) & Ids_UseAngle,  PDfid_(pd),
			  LoadRotate( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));

    /*
    **  Pipe <== ISL function ImgFlip: returns tmp fid.
    */
    PDcopiedROI_(pd) |= IfReplace_( iScheme_(rcb) & Ids_UseFlip,  PDfid_(pd),
			  LoadFlip( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));

    /*
    **  Pipe <== ISL function ImgScale: returns tmp fid.
    */
    PDcopiedROI_(pd) |= IfReplace_( iScheme_(rcb) & Ids_UseScale_2,  PDfid_(pd),
			  LoadScale( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));

    /*
    **  Pipe <== ISL function IdsToneScale: returns tmp fid.
    */
    PDcopiedROI_(pd) |= IfReplace_( iScheme_(rcb) & Ids_UseToneScale,  PDfid_(pd),
			  LoadToneScale( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));

    /*
    **  Function IdsSharpen is written in BandIntrlvdByPixel. This is a round
    **  about way to do it and in ans is replaced when available. 
    */
    if( iScheme_(rcb) & Ids_UseSharpen )
      {
	/*
	 **  Pipe <== ISL function ImgCvtCompSpaceOrg : returns tmp fid.
	 */
	IfReplace_ (1, PDfid_(pd),
		    LoadSpaceCvt(ctx, PDfid_(pd),ImgK_BandIntrlvdByPixel));

	/*
	 **  Pipe <== IDS function IdsSharpen: returns tmp fid.
	 */
	PDcopiedROI_(pd) |= IfReplace_( 1,  PDfid_(pd),
			     LoadSharpen( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));
	
	/*
	 **  Pipe <== ISL function ImgCvtCompSpaceOrg : returns tmp fid.
	 */
	IfReplace_ (1, PDfid_(pd),
		    LoadSpaceCvt(ctx, PDfid_(pd),ImgK_BandIntrlvdByPlane));
      }

    /*
    **  Pipe <== ISL function IdsDither: returns tmp fid.
    */
    PDcopiedROI_(pd) |= IfReplace_( iScheme_(rcb) & Ids_UseDither,  PDfid_(pd),
			  LoadDither( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));

    /*
    **  Pipe <== ISL function IdsCombineFrame: returns tmp fid.
    */
    PDcopiedROI_(pd) |= IfReplace_( iScheme_(rcb) & Ids_UseReversePolarity,  
				   PDfid_(pd),
			LoadCombineFrame( ctx, PDfid_(pd), !PDcopiedROI_(pd) ));

    /*
     **  Pipe <== IDS function ExportPipeTap: returns nothing
     */
    PDrfid_(pd)  = PDfid_(pd); 
    LoadExportPipeTap(ctx);

    /*
     **  Pipe <== IDS function PlaneSwapByPtr: returns tmp fid (maybe).
     */
    PDcopiedROI_(pd) != IfReplace_(iScheme_(rcb) & Ids_UsePlaneSwapByPtr,
				   PDfid_(pd),
			     LoadPlaneSwap( ctx, PDfid_(pd), !PDcopiedROI_(pd)));
    
    /*
    **  Pipe <== ISL function ImgCvtCompSpaceOrg : returns tmp fid.
    */
    IfReplace_ (iScheme_(rcb) & Ids_UseOldFormatCvt, PDfid_(pd),
		LoadSpaceCvt(ctx, PDfid_(pd),ImgK_BandIntrlvdByPixel));

    return(pd);
#ifdef TRACE
printf( "Leaving Routine IdsCompileRendering in module IDS_RENDER_UTILS \n");
#endif
  }

/*******************************************************************************
**  LoadExportPipeTap
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function ExportPipeTap; returns nothing
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**
**  FUNCTION VALUE:
**
**	none
**
*******************************************************************************/
static void LoadExportPipeTap(ctx)
     RenderContext  ctx;
{
  IdsRenderCallback rcb = PropRnd_(ctx);
  IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadExportPipeTap in module IDS_RENDER_UTILS \n");
#endif
  /*
   ** Function ExportPipeTap; returns nothing.
   */
  InsertPipe_( pd, Ids_FunctionExportPipeTap,
	      IdsVoid, (IdsPipeFunction)ExportPipeTap,( EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadExportPipeTap in module IDS_RENDER_UTILS \n");
#endif
  return;
}

/*******************************************************************************
**  LoadPlaneSwap
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function ImgDetachPlane: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadPlaneSwap( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadPlaneSwap in module IDS_RENDER_UTILS \n");
#endif
    /*
    ** Function IdsDataPlaneSwapByPtr: returns a new fid if do_roi is TRUE
    */
    InsertPipe_( pd, 0,
		 IdsTemp, IdsPlaneSwapByPtr,
	       ( ArgByPtr_( fid ),
	         ArgByVal_( ctx ),
		 ArgByVal_( ((fid == &iFid_(rcb)) | do_roi) ? TRUE : FALSE),
		 ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		 EndArgs_ ));

    if ((fid != &iFid_(rcb)) &&	(do_roi))
      {
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));
      }

#ifdef TRACE
printf( "Leaving Routine LoadPlaneSwap in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
  }

/*******************************************************************************
**  LoadClassCvt
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function IdsRequantize: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadClassCvt( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadClassCvt in module IDS_RENDER_UTILS \n");
#endif
    /*
    **  Pipe <== IDS function IdsRequantize: returns a new (temporary) fid.
    */
    InsertPipe_( pd, Ids_FunctionClassCvt | ( do_roi ? Ids_FunctionROI : 0 ),
		 IdsTemp, IdsRequantize,
	       ( ArgByPtr_( fid ),
	         ArgByVal_( RqLst_(ctx) ),
		 ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		 ArgByRef_( RqLev_(ctx) ),
		 EndArgs_ ));

    if( fid != &iFid_(rcb) )
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadClassCvt in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}

/*******************************************************************************
**  LoadSpaceCvt
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function ImgCvtCompSpaceOrg: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**      cs_org  - component space organization to change to
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadSpaceCvt( ctx, fid, cs_org )
 RenderContext  ctx;
 unsigned long *fid;
 unsigned long cs_org;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadClassCvt in module IDS_RENDER_UTILS \n");
#endif
    /*
     **  Pipe <== ISL function ImgCvtCompSpaceOrg: returns tmp fid.
     */
    InsertPipe_(pd, Ids_FunctionCspConvert, IdsTemp, ImgCvtCompSpaceOrg,
		( ArgByPtr_(fid),
		 ArgByVal_(cs_org),
		 ArgByVal_(ImgK_LsbitFirst),
		 ArgByVal_(0),
		 EndArgs_ ));

    /*
     **  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
     */
    if( fid != &iFid_(rcb) )
      {
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		    ( ArgByPtr_( fid ), EndArgs_ ));
      }

#ifdef TRACE
printf( "Leaving Routine LoadClassCvt in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}

/*******************************************************************************
**  LoadRotate
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== ISL function ImgRotate: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadRotate( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadRotate in module IDS_RENDER_UTILS \n");
#endif
    /*
    **  Pipe <== ISL function ImgRotate: returns a new (temporary) fid.
    */
    InsertPipe_( pd, Ids_FunctionRotate | ( do_roi ? Ids_FunctionROI : 0 ),
		 IdsTemp, ImgRotate,
	       ( ArgByPtr_( fid ),
	         ArgByRef_( iRAng_(rcb) ),
		 ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		 ArgByVal_( iROpts_(rcb) ),
		 ArgByVal_( Pad_(ctx) ),
		 EndArgs_ ));

    if( fid != &iFid_(rcb) )
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadRotate in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}

/*******************************************************************************
**  LoadFlip
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== ISL function ImgFlip: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadFlip( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadFlip in module IDS_RENDER_UTILS \n");
#endif
    /*
    **  Pipe <== ISL function ImgFlip: returns a new (temporary) fid.
    */
    InsertPipe_( pd, Ids_FunctionFlip | ( do_roi ? Ids_FunctionROI : 0 ),
		 IdsTemp, ImgFlip,
	       ( ArgByPtr_( fid ),
		 ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		 ArgByVal_( iFlip_(rcb) ),
		 EndArgs_ ));

    if( fid != &iFid_(rcb) )
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadFlip in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}

/*******************************************************************************
**  LoadScale
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== ISL function ImgScale: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadScale( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadScale in module IDS_RENDER_UTILS \n");
#endif
    /*
    **  Pipe <== ISL function IdsScale: returns a new (temporary) fid.
    */
    InsertPipe_( pd, Ids_FunctionScale | ( do_roi ? Ids_FunctionROI : 0 ),
		 IdsTemp, IdsScale,
	       ( ArgByPtr_( fid ),
	         ArgByRef_( iXSc_(rcb) ),
	         ArgByRef_( iYSc_(rcb) ),
		 ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		 ArgByVal_( iSOpts_(rcb) ),
		 ArgByVal_( Pad_(ctx) ),
		 EndArgs_ ));

    if( fid != &iFid_(rcb) )
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadScale in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}

/*******************************************************************************
**  LoadToneScale
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function IdsToneScale: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadToneScale( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);
    static int        flags, comp_index = 0;

#ifdef TRACE
printf( "Entering Routine LoadToneScale in module IDS_RENDER_UTILS \n");
#endif

    /*
    **  Pipe <== IDS function IdsToneScale: returns a new (temporary) fid.
    */
    InsertPipe_( pd, Ids_FunctionToneScale | ( do_roi ? Ids_FunctionROI : 0 ),
		 IdsTemp, ImgTonescaleAdjust,
	       ( ArgByPtr_( fid ),
	         ArgByRef_( iPun1_(rcb) ),
	         ArgByRef_( iPun2_(rcb) ),
		 ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		 ArgByVal_( flags ),
		 ArgByVal_( comp_index ),
		 EndArgs_ ));

    if( fid != &iFid_(rcb) )
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadToneScale in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}

/*******************************************************************************
**  LoadSharpen
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function IdsSharpen: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadSharpen( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadSharpen in module IDS_RENDER_UTILS \n");
#endif

    /*
    **  Pipe <== IDS function IdsSharpen: returns a new (temporary) fid.
    */
    InsertPipe_( pd, Ids_FunctionSharpen | ( do_roi ? Ids_FunctionROI : 0 ),
		 IdsTemp, IdsSharpen,
	       ( ArgByPtr_( fid ),
	         ArgByRef_( iSharp_(rcb) ),
		 ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		 EndArgs_ ));

    if( fid != &iFid_(rcb) )
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadSharpen in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}

/*******************************************************************************
**  LoadDither
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== ISL function ImgDither: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadDither( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadDither in module IDS_RENDER_UTILS \n");
#endif
    /*
    **  Pipe <== ISL function IdsDither: returns a new (temporary) fid.
    */
    InsertPipe_( pd, Ids_FunctionDither | ( do_roi ? Ids_FunctionROI : 0 ),
		 IdsTemp, IdsDither,
	       ( ArgByPtr_( fid ),
	         ArgByVal_( DiLst_(ctx) ),
	         ArgByVal_( iAlgor_(rcb) ),
	         ArgByVal_( iThrsh_(rcb) ),
		 ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		 ArgByVal_( iGRA_(rcb) != 0 ? &iGRA_(rcb) : &iRGB_(rcb)[RED] ),
		 EndArgs_ ));

    if( fid != &iFid_(rcb) )
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadDither in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}
 
/*******************************************************************************
**  LoadCombineFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== ISL function ImgCombineFrame: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	fid	- pointer to src ISL Frame-id
**	do_roi	- TRUE if ROI should be used
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
static unsigned long *LoadCombineFrame( 
     RenderContext  ctx
    ,unsigned long  *fid
    ,Boolean	    do_roi
    )
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    IdsPipeDesc	      pd  = Pipe_(ctx);

#ifdef TRACE
printf( "Entering Routine LoadCombineFrame in module IDS_RENDER_UTILS \n");
#endif
    /*
    **  Pipe <== ISL function IdsCombine: returns a new (temporary) fid.
    */
     InsertPipe_( pd, Ids_FunctionCombine | ( do_roi ? Ids_FunctionROI : 0 ),
		  IdsTemp, ImgCombineFrame,
		  (ArgByPtr_( fid ),
		   ArgByVal_( do_roi ? iROI_(rcb) : 0 ),
		   ArgByVal_(0),
		   ArgByVal_(0),
		   ArgByVal_(ImgK_NotSrc1),
		   ArgByVal_(0),
		   EndArgs_ ));

    if( fid != &iFid_(rcb) )
	/*
	**  Delete temporary "fid":  Pipe <== ISL function ImgDeleteFrame.
	*/
	InsertPipe_( pd, 0, IdsVoid, (IdsPipeFunction)ImgDeleteFrame,
		   ( ArgByPtr_( fid ), EndArgs_ ));

#ifdef TRACE
printf( "Leaving Routine LoadCombineFrame in module IDS_RENDER_UTILS \n");
#endif
    return( PDtmp_(pd) );
}
/*****************************************************************************
**  Error_contition_handler
**
**  FUNCTIONAL DESCRIPTION:
**
**  Condition handler to trap and funnel ISL errors through IDS and back to
**  the application via IDS widget callback.
**
**  FORMAL PARAMETERS:
**
**      signal          - VMS signal array containing signal names
**      mechanism       - VMS mechanism vector containing depth on the stack
**
**  FUNCTION VALUE:
**
**      Chf_Resignal (always)
**
******************************************************************************/
int Error_condition_handler( 
     int	    *signal
#ifndef VMS
    ,int	    *mechanism
#else
    ,ChfMchArgsPtr  mechanism
#endif
    )
{
   int    *error_code = (int *) signal;

#ifdef TRACE
printf( "Entering Routine Error_contition_handler in module IDS_RENDER_UTILS \n");
#endif
    /*
    ** IdsErrorCb is global to this module and is defined as ErrorCallbackFunc
    ** It holds a pointer to the render context widget.
    */

    *error_code++;
   if (IdsErrorCb != 0)
    if (ECcall_(IdsErrorCb) != 0)
	 (ECcall_(IdsErrorCb))(*error_code);

#ifdef TRACE
printf( "Leaving Routine Error_contition_handler in module IDS_RENDER_UTILS \n");
#endif
#ifndef VMS
return 2328;
#else
return (Chf_Resignal);
#endif
}

/*******************************************************************************
**  ExportPipeTap
**
**  FUNCTIONAL DESCRIPTION:
**
**	IDS Pipe utility function: Tap the export pipe to get the fid.
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
static unsigned long ExportPipeTap()
{
#ifdef TRACE
printf( "Entering Routine ExportPipeTap in module IDS_RENDER_UTILS \n");
#endif

#ifdef TRACE
printf( "Leaving Routine ExportPipeTap in module IDS_RENDER_UTILS \n");
#endif
    return;
}
