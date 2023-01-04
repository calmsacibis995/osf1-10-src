
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
**      Subu Garikapati
**
**  CREATION DATE:
**
**      May 31, 1990
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
#include    <ids__widget.h> /* IDS public/private, Dwtoolkit, Xlib defs.    */
#include    <img/ImgDef.h>  /* ISL public defs.				    */
#include    <img/ChfDef.h>     /* Condition handling functions              */
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
void		    IdsApplyModelXie();		/* apply IDS rendering model*/
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
static void		SetDither();		/* dither driver	    */
static void 		SetAreaConstrain();	/* set area & constrain     */

    /*
    **	Functions which compile the IDS rendering model.
    */
XiePhoto		IdsCompileServerPipe();	/* compile routines & args   */
static void		CompileRenditionPipe(); /* builds the rendition part */
static void		CompileExportPipe();    /* builds the export part    */
static void		CompileExecutePipe();   /* builds the execute part   */
static void		ArithmeticPhoto();	/* XIE Pipe <== XieArithmetic*/
static void		LogicalPhoto(); 	/* XIE Pipe <== XieLogical   */
XiePhoto		InternalRoiPhoto();     /* XIE Pipe <== XieCrop-intrnl*/
XiePhoto		CropPhoto();		/* XIE Pipe <== XieCrop      */
XiePhoto		LuminancePhoto();	/* XIE Pipe <== XieLuminance */
XiePhoto 	        ScalePhoto();           /* XIE Pipe <== XieScale     */
XiePhoto		RotatePhoto();		/* XIE Pipe <== XieRotate    */
XiePhoto		MirrorPhoto();   	/* XIE Pipe <== XieMirror    */
XiePhoto		DitherPhoto();   	/* XIE Pipe <== XieDither    */
XiePhoto		ToneScalePhoto();   	/* XIE Pipe <== XiePoint     */
XiePhoto 	    	AreaPhoto();		/* XIE Pipe <== XieArea      */
XiePhoto 	    	ConstrainPhoto();	/* XIE Pipe <== XieConstrain */
static void		TranslatePhoto();   	/* XIE Pipe <== XieTranslate */
static void		TapPhoto();		/* XIE Pipe <== XieTap       */
static void		ExportPhoto();   	/* Xie Pipe <== XieExport    */
static void	        FreeServerResources();
static void		NotifyEvent();   	/* XIE pipe complete notify  */
static void		SavePhoto();   	        /* XIE save ehen appl asks for*/
static XieImage         IdsCreateLut();         /* create tonescale lut      */
static XiePhoto 	IdsCreateLutForExport();/* Builds the lut for export */
Boolean          	VxtImagingOption();     /* Check for the vxt atom   */
#else
PROTO(static void InitModel, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/, ModelData /*model*/));
PROTO(static void SetComponentLevels, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetClassCvt, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void PreSetScale, (IdsRenderCallback /*rcb*/, DataForXie /*xiedat*/, ModelData /*model*/));
PROTO(static void SetAngle, (IdsRenderCallback /*rcb*/, ModelData /*model*/));
PROTO(static void SetFlip, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/));
PROTO(static void PostSetScale, (IdsRenderCallback /*rcb*/, DataForXie /*xiedat*/, ModelData /*model*/));
PROTO(static void SetProtocol, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetToneScale, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetDither, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SetAreaConstrain, (RenderContext /*ctx*/, ModelData /*model*/));
PROTO(static void SavePhoto, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/));
PROTO(static void CompileRenditionPipe, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/, PhotoContext /*phoctx*/));
PROTO(static void CompileExportPipe, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/, PhotoContext /*phoctx*/));
PROTO(static void CompileExecutePipe, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/, PhotoContext /*phoctx*/));
PROTO(static void ArithmeticPhoto, (RenderContext /*ctx*/, XieRoi /*roi*/));
PROTO(static void LogicalPhoto, (RenderContext /*ctx*/, XieRoi /*roi*/));
PROTO(static void TranslatePhoto, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/));
PROTO(static void TapPhoto, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/));
PROTO(static void ExportPhoto, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/, PhotoContext /*phoctx*/));
PROTO(static void FreeServerResources, (RenderContext /*ctx*/, RenderContextXie /*xiectx*/));
PROTO(static XieImage IdsCreateLut, (PhotoContext /*phoctx*/, unsigned long /*dlevs*/[], float */*punch_1*/, float */*punch_2*/));
PROTO(static XiePhoto IdsCreateLutForExport, (RenderContext, DataForXie, XiePhoto, PhotoContext /*phoctx*/, Display *));
PROTO(static void NotifyEvent, (unsigned long /*scheme*/, RenderContextXie /*xiectx*/));
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
/*
**	Local Storage
*/
    /* none */

/*******************************************************************************
**  IdsApplyModelXie
**
**  FUNCTIONAL DESCRIPTION:
**
**      Apply IDS rendering model to: determine which ISL and IDS functions to
**	call, calculate their parameters, and determine their order of call.
**
**  FORMAL PARAMETERS:
*
**	ctx	- rendering context.
**	xiectx	- xie rendering context.
**
*******************************************************************************/
void IdsApplyModelXie( ctx, xiectx )
RenderContext	    ctx;
RenderContextXie   xiectx;
    {
    IdsRenderCallback rcb = PropRnd_(ctx);
    ModelDataStruct   model;
    DataForXie       xiedat = PriXie_(xiectx);
 
#ifdef TRACE
printf( "Entering Routine IdsApplyModelXie in module IDS_BUILD_XIE_PIPE \n");
#endif

    InitModel( ctx, xiectx, &model );	/* initialize dynamic model info    */
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
        if ((iScheme_(rcb) & Ids_UseROI ) && 
  	   ((Croiw_(xiedat) != 0) || (Croih_(xiedat) != 0)))
	    {
            IrW_(xiedat)  = (long int )(Croiw_(xiedat));
            IrH_(xiedat) = (long int )(Croih_(xiedat));
  	    model.width = IrW_(xiedat) ;
 	    model.height = IrH_(xiedat) ;
	    }

        if ((iScheme_(rcb) & Ids_UseIROI ) && 
	    ((Iroiw_(xiedat) != 0) || (Iroih_(xiedat) != 0)))
	    {
            IrW_(xiedat)  = (long int )(Iroiw_(xiedat));
            IrH_(xiedat) = (long int )(Iroih_(xiedat));
  	    model.width = IrW_(xiedat) ;
 	    model.height = IrH_(xiedat) ;
	    }


	PreSetScale( rcb, xiedat, &model );/* set scale factors		    */
	if (iScheme_(rcb) & Ids_UseScale_1)
	    {
            IrW_(xiedat) = model.width / iXSc_(rcb);
            IrH_(xiedat) = model.height / iYSc_(rcb);
	    }
	SetAngle( rcb, &model );	   /* set rotation angle	    */
	if (iScheme_(rcb) & Ids_UseScale_2)
	    {
            IrW_(xiedat) = model.width / iXSc_(rcb);
            IrH_(xiedat) = model.height / iYSc_(rcb);
	    }
	SetFlip( ctx, xiectx );		   /* set flip			    */
	break;
    case Ids_FitWithin:
    case Ids_FitWidth:
    case Ids_FitHeight:
    case Ids_Flood:
	/*
	**  Sequence for scale modes which have lower priority than rotation.
	*/
	SetAngle( rcb, &model );	    /* set rotation angle	    */
	SetFlip( ctx, xiectx );		    /* set flip			    */
        /* 
        ** Save width height of model so that the call to XIE gets the correct
        ** values.
        */
        IrW_(xiedat) = model.width;
        IrH_(xiedat) = model.height;

        if ((iScheme_(rcb) & Ids_UseROI ) && 
  	   ((Croiw_(xiedat) != 0) || (Croih_(xiedat) != 0)))
	    {
            IrW_(xiedat)  = (long int )(Croiw_(xiedat));
            IrH_(xiedat) = (long int )(Croih_(xiedat));
  	    model.width = IrW_(xiedat) ;
 	    model.height = IrH_(xiedat) ;
	    }

        if ((iScheme_(rcb) & Ids_UseIROI ) && 
	    ((Iroiw_(xiedat) != 0) || (Iroih_(xiedat) != 0)))
	    {
            IrW_(xiedat)  = (long int )(Iroiw_(xiedat));
            IrH_(xiedat) = (long int )(Iroih_(xiedat));
  	    model.width = IrW_(xiedat) ;
 	    model.height = IrH_(xiedat) ;
	    }

	PostSetScale( rcb, xiedat, &model );/* set scale factors	    */
	break;
    default:
	XtWarningMsg("InvSclMod","PreSetScale","IdsImageError",
		     "invalid Scale Mode", NULL,NULL);
	}
    SetProtocol( ctx, &model );		/* set presentation protocol	    */
    SetToneScale( ctx, &model );        /* set tone-scale parameters        */
    SetDither( ctx, &model );		/* set dither parameters	    */
    SetAreaConstrain( ctx, &model );	/* set area and constrain parameters */

#ifdef TRACE
printf( "Leaving Routine IdsApplyModelXie in module IDS_BUILD_XIE_PIPE \n");
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
**	xiectx	- XIE RenderContext data.
**	model	- address of ModelDataStruct to be initialized.
**
*******************************************************************************/
static void InitModel( ctx, xiectx, model )
 RenderContext	    ctx;
 RenderContextXie   xiectx;
 ModelData	    model;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    XieImage          img = iXieimage_(rcb);
    DataForXie        pridat = PriXie_(xiectx);
    int i;		  
    long int w, h;
    unsigned long x, y;


#ifdef TRACE
printf( "Entering Routine InitModel in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Decompression is done during transport at the server automatically
    **  and there is no need to init render scheme for decompresiion.
    */
    if(  Cmpres_(img) !=  XieK_PCM )
        iScheme_(rcb) |= Ids_Decompress;

    /*
    **  Fill in w, h, spect info: type, comp cnt, and bits/comp.
    */
    model->width        = Width_(img);
    model->height       = Height_(img);
    model->spect_type   = (int)CmpMap_(img);
    model->spect_cnt    = (int)CmpCnt_(img);
    
    for(i = 0, model->pixel_bits = 0; i < model->spect_cnt; ++i) 
     {
      model->levels[i] = CmpLvl_(img,i);
      model->bpc[i] = ceil(log10((double)CmpLvl_(img,i))/log10(2.0));
      model->pixel_bits += model->bpc[i];
     }

    /*
    ** Check the validity of the geometry of roi and set scheme to for roi's
    */
    if( (iROI_(rcb) != 0) && (Croiw_(pridat) != 0 && Croih_(pridat) != 0))
        iScheme_(rcb) |= Ids_UseROI;

    if( Iroiw_(pridat) != 0 && Iroih_(pridat) != 0)
        iScheme_(rcb) |= Ids_UseIROI;

    SetComponentLevels( ctx, model );         /* init level per component    */

#ifdef TRACE
printf( "Leaving Routine InitModel in module IDS_BUILD_XIE_PIPE \n");
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

#ifdef TRACE
printf( "Entering Routine SetComponentLevels in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Set maximum levels and grayscale levels allowed.
    */
    iLevs_(rcb) = RClass_(ctx) == Ids_Bitonal ? 2
                : FitL_(ctx) != 0 ? FitL_(ctx)
                : Vis_(ctx) != 0 && ( Vis_(ctx)->class == TrueColor || Vis_(ctx)->class == DirectColor )
                ? Cells_(ctx) * Cells_(ctx) * Cells_(ctx) : Cells_(ctx);
    iGRA_(rcb)  =  GRA_(ctx) == 0 ? MIN(iLevs_(rcb), 1<<model->pixel_bits )
                :  MIN( GRA_(ctx),  MIN(iLevs_(rcb), 1<<model->pixel_bits ));
    /*
    **  If the rendering class isn't Ids_Color, OR our image isn't color,
    **  THEN no levels will be alotted for the color RGB components.
    */
    if( RClass_(ctx) != Ids_Color || model->spect_cnt < Ids_MaxComponents )
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
            iRGB_(rcb)[i] = RGB_(ctx)[i] == 0 ? model->levels[i]
                          :  MIN( RGB_(ctx)[i], model->levels[i] );
            /*
            **  Accumulate the number of bits and levels desired.
            */
            SetBitsPerComponent_( bpc[i], iRGB_(rcb)[i] );
            max_bpc = MAX( max_bpc, bpc[i] );
            bits   += bpc[i];
            levels *= iRGB_(rcb)[i];
            }

        if( max_bpc > 25 || levels > iLevs_(rcb) )
            {
            /*
            **  The bits_per_component or number of levels exceeds our limits.
            **  IDS will choose the "default_bits_per_component" by dividing
            **  the hardware limit, WinD_(), between the components as follows:
            **  BLU_bits = floor( WinD_() / 3 ),           (least bits)
            **  GRN_bits =  ceil( WinD_() / 3 ),            (most bits)
            **  RED_bits = WinD_() - GRN_bits - BLU_bits. (remainder)
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
            **  BLU_levels = (int)   cube_root( iLevs_() )             (least)
            **  RED_levels = (int) square_root( iLevs_() / BLU_levels )
            **  GRN_levels = (int) iLevs_() / RED_levels / BLU_levels   (most)
            */
            iRGB_(rcb)[BLU] = MIN( iRGB_(rcb)[BLU], MIN( 1<<bpc[BLU],
                           (int)pow( (double) iLevs_(rcb), 1.0/3.0 )));
            iRGB_(rcb)[RED] = MIN( iRGB_(rcb)[RED], MIN( 1<<bpc[RED],
                           (int)sqrt((double) iLevs_(rcb) / iRGB_(rcb)[BLU])));
            iRGB_(rcb)[GRN] = MIN( iRGB_(rcb)[GRN], MIN( 1<<bpc[GRN],
                            iLevs_(rcb) / iRGB_(rcb)[RED] / iRGB_(rcb)[BLU]));
            }

        }
#ifdef TRACE
printf( "Leaving Routine SetComponentLevels in module IDS_BUILD_XIE_PIPE \n");
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
printf( "Entering Routine SetClassCvt in module IDS_BUILD_XIE_PIPE \n");
#endif

    if( model->spect_type == XieK_RGB &&
        ( iRGB_(rcb)[RED] | iRGB_(rcb)[GRN] | iRGB_(rcb)[BLU] ) < 2 )
      {
        /*
        **  Tell compiler to include spectral class conversion.
        */
	model->spect_cnt    = 1;
	model->spect_type   = XieK_GrayScale;
	model->pixel_bits   = model->bpc[GRA];
        iScheme_(rcb) |= Ids_UseClassCvt;
      }
#ifdef TRACE
printf( "Leaving Routine SetClassCvt in module IDS_BUILD_XIE_PIPE \n");
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
** 	xiedat  - Data for xie
**	model	- dynamic model data.
**
*******************************************************************************/
static void PreSetScale( rcb, xiedat, model )
 IdsRenderCallback rcb;
 DataForXie        xiedat;
 ModelData	   model;
{
    XieImage  img = (XieImage)iXieimage_(rcb);
    int wide, high, ll_x, ll_y, ur_x, ur_y;
    int pp_dist, lp_dist;
    double device_par, image_par;               /* Pixel aspect ratios      */

#ifdef TRACE
printf( "Entering Routine PreSetScale in module IDS_BUILD_XIE_PIPE \n");
#endif

    /* 
    ** Save width height of model so that the call to XIE gets the correct
    ** values.
    */
    IrW_(xiedat) = model->width;
    IrH_(xiedat) = model->height;
    if( iRender_(rcb) != Ids_Override )
	switch( iSMode_(rcb) )	
	    {
	case Ids_Physical:
            ll_x = Cllx_(xiedat);
            ll_y = Clly_(xiedat);
            ur_x = Curx_(xiedat);
            ur_y = Cury_(xiedat);
            wide = uWidth_(img,0);
            high = uHeight_(img,0);

	    iXSc_(rcb) = iXres_(rcb) * (ur_x - ll_x) / wide;
	    iYSc_(rcb) = iYres_(rcb) * (ur_y - ll_y) / high;

	    if( iXSc_(rcb) > 0.0  &&  iYSc_(rcb) > 0.0 )
		break;				    /* dimensions are legal */

	case Ids_AspectOnly:
	    /*
	    **	AspectOnly: Adjust X,Y scale factors to match image pixel
            **  aspect ratio to presentation surface aspect ratio.
	    */
            pp_dist = Cppd_(xiedat);
            lp_dist = Clpd_(xiedat);

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
printf( "Leaving Routine PreSetScale in module IDS_BUILD_XIE_PIPE \n");
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
printf( "Entering Routine SetAngle in module IDS_BUILD_XIE_PIPE \n");
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
		iScheme_(rcb) |= Ids_UseAngle;
		}
	    break;
	case Ids_NoRotate :
            if ( iRAng_(rcb) != 0.0 )
                {
                iScheme_(rcb) |= Ids_UseAngle;
                iRAng_(rcb) = 0.0;
                }
   	    break;
	case Ids_Rotate :
	    {
	    float   local_angle = iRAng_(rcb) * -1;              
	    long    int_angle = iRAng_(rcb) * -1;
	    float   fraction = 0.0;

	    fraction = iRAng_(rcb) + int_angle;
	    int_angle %= 360;
 
            if ((int_angle != 0) || (fraction != 0))
                iScheme_(rcb) |= Ids_UseAngle;
	    break;
	    }
	default :
	    XtWarningMsg("InvRotMod","SetAngle","IdsImageError",
			 "invalid Rotate Mode",0,0);
	    }
    ca    = fabs( cos( DEG_RAD * iRAng_(rcb) ));
    sa    = fabs( sin( DEG_RAD * iRAng_(rcb) ));
    rw		  = ca * model->width + sa * model->height;
    model->height = sa * model->width + ca * model->height;
    model->width  = rw;

#ifdef TRACE
printf( "Leaving Routine SetAngle in module IDS_BUILD_XIE_PIPE \n");
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
**	ctx	- RenderContext data.
**	xiectx	- XIE RenderContext data.
**
*******************************************************************************/
static void SetFlip( ctx, xiectx )
 RenderContext      ctx;
 RenderContextXie   xiectx;
{
    IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine SetFlip in module IDS_BUILD_XIE_PIPE \n");
#endif

    switch( iFlip_(rcb) )
	{
    case 0 :
	break;

    case Ids_FlipVertical  :
 	Rxmir_(xiectx) = True;  
	iScheme_(rcb) |= Ids_UseFlip;		/* request flip to happen   */
	break;

    case Ids_FlipHorizontal :
	Rymir_(xiectx) = True; 
	iScheme_(rcb) |= Ids_UseFlip;		/* request flip to happen   */
        break; 

    case Ids_FlipVertical | Ids_FlipHorizontal :
	Rxmir_(xiectx) = True;
	Rymir_(xiectx) = True;
	iScheme_(rcb) |= Ids_UseFlip;		/* request flip to happen   */
	break;

    default :
	XtWarningMsg("InvFlpMod","SetFlip","IdsImageError",
		     "invalid Flip Options",0,0);
	}
#ifdef TRACE
printf( "Leaving Routine SetFlip in module IDS_BUILD_XIE_PIPE \n");
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
static void PostSetScale( rcb, xiedat, model )
 IdsRenderCallback  rcb;
 DataForXie        xiedat;
 ModelData	    model;
{
    double rw, rh;
    double device_par, image_par;               /* Pixel aspect ratios      */
    XieImage  img = (XieImage)iXieimage_(rcb);
    int pixel_path_distance, line_path_distance;


#ifdef TRACE
printf( "Entering Routine PostSetScale in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Get image attributes we need.
    */
    /*
    **  Initialize parameters
    */
    rw = model->width;
    rh = model->height;

    device_par = iXres_(rcb) / iYres_(rcb);
    image_par  = PxlRatio_(img);

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
            **  FitWithin : choose smaller of two scale factors. Adjust X scale
            **              factor for presentation surface aspect ratio.
            */
            iXSc_(rcb)  = MIN(iXSc_(rcb), iYSc_(rcb));
            iYSc_(rcb)  = MIN(iXSc_(rcb), iYSc_(rcb));
            iXSc_(rcb) /= image_par;
            iXSc_(rcb) *= device_par;
            break;

       case Ids_FitWidth:
            /*
            **  FitWidth : Unconditionally choose X scale factor. Adjust X scale
            **             factor for presentation surface aspect ratio.
            */

            iYSc_(rcb)  = iXSc_(rcb);
            iXSc_(rcb) /= image_par;
            iXSc_(rcb) *= device_par;
            break;

        case Ids_FitHeight:
            /*
            **  FitHeight : Unconditionally choose Y scale factor. Adjust X
            **              scale factor for presentation surface aspect ratio.
            */
            iXSc_(rcb)  = iYSc_(rcb);
            iXSc_(rcb) /= image_par;
            iXSc_(rcb) *= device_par;
            break;

       case Ids_Flood:
            /*
            **  Flood : Use computed scale factors as is.
            */
            break;
            }
        }


    model->width  *= iXSc_(rcb);            /* update width  with X scale   */
    model->height *= iYSc_(rcb);            /* update height with Y scale   */

    if( iXSc_(rcb) != 1.0  ||  iYSc_(rcb) != 1.0 )
        if( iXSc_(rcb) == iYSc_(rcb) && iXSc_(rcb) * iYSc_(rcb) <= 1.0 )
            /*
            **  Scaling symetrically to a smaller area.
            */
            iScheme_(rcb) |= Ids_UseScale_1;
                    
        else
            /*
            **  Scaling asymetrically, or to a larger area.
            */
            iScheme_(rcb) |= Ids_UseScale_2;
#ifdef TRACE
printf( "Leaving Routine PostSetScale in module IDS_BUILD_XIE_PIPE \n");
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
    IdsRenderCallback rcb = PropRnd_(ctx);
    long int_angle = 0;
    float fraction = 0.0;

#ifdef TRACE
printf( "Entering Routine SetProtocol in module IDS_BUILD_XIE_PIPE \n");
#endif

    int_angle = iRAng_(rcb);
    fraction = iRAng_(rcb) - int_angle;

    /* Determine if the VXT imaging atom is present and if the operation
    ** is suitable for hardware. If the atom is not present, determine if
    ** the operation can be performed on the server in XIE software.
    */

    if (( model->spect_type == XieK_Bitonal ) /* image is bitonal */
	     && (!(getenv( "IDS_COMPUTE_MODE_ISL" )))
	     && (!(getenv( "IDS_NO_VXT_RERENDITION" )))
	     && (VxtImagingOption(Dpy_(ctx), 0))  /* vxt img atom present */
	     &&
	 ( 
	   (!(iScheme_(rcb) & Ids_UseAngle)) ||		/* no rotate */
	   (
	    (iScheme_(rcb) & Ids_UseAngle) && 
		( ((int_angle % 90) == 0) && (fraction == 0.0))
	    )
	   ) 						/* rotate and orthog*/
	     &&
	     (   ( 
		   (!(iScheme_(rcb) & Ids_UseScale_1)) &&    /* no scale */
	           (!(iScheme_(rcb) & Ids_UseScale_2))
		 ) 
		 || /* or scale and scale factors not greater than 64 */
	         ( 
		   (
		    (iScheme_(rcb) & Ids_UseScale_1) || 
		    (iScheme_(rcb) & Ids_UseScale_2)
		   )
		   && 
		   (
		    (iXSc_(rcb) <= 64.0) &&
		    (iYSc_(rcb) <= 64.0) 
		   ) 
	         ) 
  	       )
	     )
	{ /* operation is suitable for hardware */
        iProto_(rcb) = Ids_Window; 
	intCompute_(rcb) = Ids_XieServer;
	}
    else
        if (!(getenv( "IDS_COMPUTE_MODE_ISL" )))
	    { /* operation is not suitable for hardware, compute in XIE sw */
            iProto_(rcb) = Ids_Pixmap; 
	    intCompute_(rcb) = Ids_XieServer;
	    }
	else
	    {
	    iProto_(rcb) = Ids_XImage; 	
	    intCompute_(rcb) = Ids_IslClient;		
            }

    /*
    **  For Pixmap protocol we have to see if the hardware supports it.
    */
    if (  !(iProto_(rcb) != Ids_Photomap
        && ( model->spect_type == XieK_Bitonal ) /* image is bitonal */
	&& (!(getenv( "IDS_COMPUTE_MODE_ISL" )))
	&& (getenv( "IDS_NO_VXT_RERENDITION" ))
        && (VxtImagingOption(Dpy_(ctx), 0))  /* vxt img atom present */
        && ( model->width  >  WidthOfScreen(Scr_(ctx))
             ||   model->height > HeightOfScreen(Scr_(ctx)))) )
            {
            /*
            **  Comparing the width and height of the image and to the screen
            **  size is a hack to get around a limitation of the GPX server.
            */
            iProto_(rcb) = Ids_Photomap;
	    }

/*
    printf ("Proto %d \n", iProto_(rcb));
    printf ("Compute mode %d \n", intCompute_(rcb));
*/

#ifdef TRACE
printf( "Leaving Routine SetProtocol in module IDS_BUILD_XIE_PIPE \n");
#endif
    }

/*******************************************************************************
**  SetToneScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      Determine the need for tone-scale adjustment of grayscale, color images.
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
    int i;

#ifdef TRACE
printf( "Entering Routine SetToneScale in module IDS_BUILD_XIE_PIPE \n");
#endif

    switch( model->spect_type )
	{
    case XieK_GrayScale :
	/*
	**  See if we need grayscale tonescale.
	*/
	if ( iPun1_(rcb) != 0.0 || iPun2_(rcb) != 1.0 )
	  {
	    /*
	    **  Set number of grayscale levels for tone-scale adjustment.
	    */
	    TsLev_(ctx)[GRA] = MAX(ClassCvtLevels, iLevs_(rcb));
	    TsLev_(ctx)[GRN] = TsLev_(ctx)[BLU] = 0;
	    iScheme_(rcb) |= Ids_UseToneScale;
          }
	break;
    case XieK_RGB :
	/*
	**  See if we need color tonescale.
	*/
	if ( iPun1_(rcb) != 0.0 || iPun2_(rcb) != 1.0 )
          {
	    for( i = 0; i < Ids_MaxComponents; i++ )
/*		TsLev_(ctx)[i] = MAX(ClassCvtLevels, iLevs_(rcb));           */
/*		TsLev_(ctx)[i] = iLevs_(rcb);                                */
		TsLev_(ctx)[i] = iRGB_(rcb)[i];           
            iScheme_(rcb) |= Ids_UseToneScale;
          }
	break;
	}
#ifdef TRACE
printf( "Leaving Routine SetToneScale in module IDS_BUILD_XIE_PIPE \n");
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
    unsigned long i;

#ifdef TRACE
printf( "Entering Routine SetDither in module IDS_BUILD_XIE_PIPE \n");
#endif

    switch( model->spect_type )
        {
    case XieK_GrayScale :
        /*
        **  See if we need grayscale dither.
        */
	
     	if (model->bpc[GRA] > 25 || 1<<model->bpc[0] > iGRA_(rcb))
	    iScheme_(rcb) |= Ids_UseDither;

/*
        if( model->pixel_bits > 25 || 1<<model->pixel_bits > iGRA_(rcb) )
	    {

            iScheme_(rcb) |= Ids_UseDither;
	    }
*/
        break;
    case XieK_RGB :
        /*
        **  See if we need color dither.
        */
        if( model->levels[RED] > iRGB_(rcb)[RED]
         || model->levels[GRN] > iRGB_(rcb)[GRN]
         || model->levels[BLU] > iRGB_(rcb)[BLU] )
	    {
            iScheme_(rcb) |= Ids_UseDither;
	    iGRA_(rcb) = 0;                          /* no levels of gray    */
	    }
        break;
        }
#ifdef TRACE
printf( "Leaving Routine SetDither in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/*******************************************************************************
**  SetAreaConstrain
**
**  FUNCTIONAL DESCRIPTION:
**
**      Driver for setting area and constrain parameters.
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	model	- dynamic model data.
**
*******************************************************************************/
static void SetAreaConstrain( ctx, model )
RenderContext ctx;
ModelData     model;
    {
    IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine SetAreaConstrain in module IDS_BUILD_XIE_PIPE \n");
#endif

    /* If the mode is Ids_Window, this indicates that the VXT accelerator
    ** board is present. If the screen depth is 8, we will add an Area and
    ** Constrain operation to the pipe if there is a Scale operation 
    ** currently in the pipe. 
    */
    if (( ( iProto_(rcb) == Ids_Window ) 
   	|| (( model->spect_type == XieK_Bitonal ) /* image is bitonal */
	     && (VxtImagingOption(Dpy_(ctx), 0)))  /* vxt img atom present */
	&& 
	 (DefaultDepth(Dpy_(ctx),DefaultScreen(Dpy_(ctx)) ) > 1) &&
		((iScheme_(rcb) & Ids_UseScale_1) || 
		    (iScheme_(rcb) & Ids_UseScale_2))
		) 
	&& (!getenv( "IDS_NO_VXT_CONVOLUTION" )))
	{
        iScheme_(rcb) |= Ids_UseArea;
        iScheme_(rcb) |= Ids_UseConstrain;
	}
  

#ifdef TRACE
printf( "Leaving Routine SetAreaConstrain in module IDS_BUILD_XIE_PIPE \n");
#endif
    }

/*******************************************************************************
**  SavePhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**      Crop the final rendered photo to hand over to the application. The
**      application is responsible to free this photo.
**
**  FORMAL PARAMETERS:
**
**      ctx     - rendering context.
** 	xiectx  - XIE rendering context.
**
*******************************************************************************/
static void SavePhoto( ctx, xiectx )
 RenderContext    ctx;
 RenderContextXie xiectx;
{
#ifdef TRACE
printf( "Entering Routine SavePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

   if ( Save_(ctx) == Ids_SaveFid || Save_(ctx) == Ids_SaveXieimg || 
			             Save_(ctx) == Ids_SavePhoto )
      /*
      ** Crop just does what a copy photo should do to get the final rend Photo 
      */
      SPhoto_(xiectx) = XieTapFlo( XieCrop( Photo_(xiectx), 0 ), True );
#ifdef TRACE
printf( "Leaving Routine SavePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/*******************************************************************************
**  IdsCompileServerPipe
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
** 	xiectx  - XIE rendering context.
**
**  FUNCTION VALUE:
**
**	pd	- pipe descriptor (from ctx) containing rendering sequence.
**
*******************************************************************************/
XiePhoto	IdsCompileServerPipe( ctx, xiectx )
 RenderContext	    ctx;
 RenderContextXie   xiectx;
{
    PhotoContextStruct  phoctx;

#ifdef TRACE
printf( "Entering Routine IdsCompileServerPipe in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    ** If the photomap has to rerender create a new Photoflo
    */
    if( RRend_(xiectx) )
        /*
        **  Clone a Photoflo pipeline using the perma Photomap as its source.
        */
        Photo_(xiectx) = XieClonePhoto( TPhoto_(xiectx),  /* Photomap id      */
                                        XieK_Photoflo );

    else 
	/*
        ** Tap a permanent photomap for rerendition but with no image data
        ** for the first time of rendition. Since this is the first pipe
        ** element before rendition the photomap is filled with data it has
        ** recieved across the wire only after the server pipe build 
        ** is completed.
        */
        TPhoto_(xiectx) = XieTapFlo( Photo_(xiectx), True );
      
    /*
    **  Enable completion events.
    */
    XieSelectEvents( Dpy_(ctx), XieM_ComputationEvent |
                                     XieM_DisplayEvent  | XieM_PhotofloEvent );
    /*
    ** Build rendition part of the pipe
    */
    CompileRenditionPipe( ctx, xiectx, &phoctx ) ;

    /*
    ** When application asks for a rendered photo or xieimage.. give them       
    */
    SavePhoto( ctx, xiectx );

    /*
    ** Build Export part of the pipe
    */
    CompileExportPipe( ctx, xiectx, &phoctx ) ;

    /*
    ** Build Execute part of the pipe
    */
    CompileExecutePipe( ctx, xiectx, &phoctx ) ;

    /*
    ** Free all the Server side Roi's and the photoflo
    */
    if (XieQueryFlo( Photo_(xiectx) ) != XieK_PhotofloAborted)
        FreeServerResources( ctx, xiectx );
    else
	{
#ifdef TRACE
printf( "Leaving Routine IdsCompileServerPipe in module IDS_BUILD_XIE_PIPE \n");
#endif
	return 0;
	}

#ifdef TRACE
printf( "Leaving Routine IdsCompileServerPipe in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( TPhoto_(xiectx) );
}

/*******************************************************************************
**  CompileRenditionPipe()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**	ctx	     - rendering context.
** 	xiectx       - XIE rendering context.
** 	phoctx	     - photo context structure.
**
**  FUNCTION VALUE:
**
**
*******************************************************************************/
static void	CompileRenditionPipe( ctx, xiectx, phoctx )
 RenderContext	    ctx;
 RenderContextXie   xiectx;
 PhotoContext       phoctx;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    Boolean	      added;
    int               i;

    DataForXie         xiedat;

#ifdef TRACE
printf( "Entering Routine CompileRenditionPipe in module IDS_BUILD_XIE_PIPE \n");
#endif

    xiedat = PriXie_(xiectx);

    /*
    **  Pipe <== XIE function Crop: add ROI into the XIE pipe.
    */
    added = AddPipeElement_( iScheme_(rcb) & Ids_UseROI, Photo_(xiectx),
  	          CropPhoto( Photo_(xiectx), PriXie_(xiectx), Dpy_(ctx)));

    added = AddPipeElement_( ((Iroiw_(xiedat) != 0) && (Iroih_(xiedat) != 0)), 
	      Photo_(xiectx),
  	      InternalRoiPhoto( Photo_(xiectx), PriXie_(xiectx), Dpy_(ctx)));

    /*
    **  Pipe <== XIE function Luminance: add Luminance into the XIE pipe.
    */
    added = AddPipeElement_( iScheme_(rcb) & Ids_UseClassCvt, Photo_(xiectx),
                             LuminancePhoto( Photo_(xiectx), Dpy_(ctx) ));

    /* If there is a scale operation and the algorithm selected was
    ** linear interpolation and Ids_UseArea is set, then we are rendering 
    ** a bitonal image on an 8 plane display and will perform
    ** an Area and Constrain operation in the pipe before the scale.
    ** The Export operation will contain a LUT for remapping as well.
    */
    if (iScheme_(rcb) & Ids_UseScale_1) 
        {
	/* If Ids_UseArea bit was set, add Area operation to the pipe. */
        added = AddPipeElement_( iScheme_(rcb) & Ids_UseArea, Photo_(xiectx),
		   AreaPhoto( rcb, ctx, Photo_(xiectx), Dpy_(ctx), xiedat, phoctx ));

	/* 
	** If Ids_UseConstrain bit was set, add constrain operation to 
	** the pipe.
	*/
        added = AddPipeElement_( iScheme_(rcb) & Ids_UseConstrain, 
			Photo_(xiectx), ConstrainPhoto( Photo_(xiectx) ));
	}

    /*
    **  Pipe <== XIE function Scale: add scale before rotate into the XIE pipe.
    */
    added = AddPipeElement_( iScheme_(rcb) & Ids_UseScale_1, Photo_(xiectx),
	    ScalePhoto( rcb, Photo_(xiectx), PriXie_(xiectx), Dpy_(ctx) ));

    /*
    **  Pipe <== XIE function Rotate: add rotate into the XIE pipe.
    */
    added = AddPipeElement_( iScheme_(rcb) & Ids_UseAngle, Photo_(xiectx),
   	           RotatePhoto( rcb, Photo_(xiectx), Dpy_(ctx) ));

    /*
    **  Pipe <== XIE function Mirror: add flip into the XIE pipe.
    */
    added = AddPipeElement_( iScheme_(rcb) & Ids_UseFlip, Photo_(xiectx),
			     MirrorPhoto( xiectx, Photo_(xiectx), Dpy_(ctx) ));

    /* If there is a scale operation and the algorithm selected was
    ** linear interpolation and Ids_UseArea is set, then we are rendering 
    ** a bitonal image on an 8 plane display and will perform
    ** an Area and Constrain operation in the pipe before the scale.
    ** The Export operation will contain a LUT for remapping as well.
    */
    if (iScheme_(rcb) & Ids_UseScale_2) 
        {
	/* If Ids_UseArea bit was set, add Area operation to the pipe */
        added = AddPipeElement_( iScheme_(rcb) & Ids_UseArea, Photo_(xiectx),
		   AreaPhoto( rcb, ctx, Photo_(xiectx), Dpy_(ctx), xiedat, phoctx ));

	/* 
	** If Ids_UseConstrain bit was set, add constrain operation to 
	** the pipe.
	*/
        added = AddPipeElement_( iScheme_(rcb) & Ids_UseConstrain, 
			Photo_(xiectx), ConstrainPhoto( Photo_(xiectx) ));
	}

    /*
    **  Pipe <== XIE function Scale: add rotate before scale into the XIE pipe.
    */
    added = AddPipeElement_( iScheme_(rcb) & Ids_UseScale_2, Photo_(xiectx), 
		ScalePhoto( rcb, Photo_(xiectx), PriXie_(xiectx), Dpy_(ctx) ));

    /*
    **  Pipe <== XIE function ToneScale: add point operatopn into  XIE pipe.
    */
    added = AddPipeElement_( iScheme_(rcb) & Ids_UseToneScale, Photo_(xiectx), 
              ToneScalePhoto( rcb, ctx, Photo_(xiectx), phoctx, Dpy_(ctx) ));
   
    /*
    **  Pipe <== XIE function Dither: add dither into the XIE pipe.
    */
    added = AddPipeElement_( iScheme_(rcb) & Ids_UseDither, Photo_(xiectx),
			     DitherPhoto( rcb, Photo_(xiectx), Dpy_(ctx) ));


    ExportPhoto( ctx, xiectx, phoctx );

    /* If an Area and Constrain operation was added to the pipe and
    ** a Scale operation was present, create a LUT to be passed into
    ** XieExport to remap the values to new colors. The LUT will contain
    ** the interpolated values between foreground and background colors.
    */
    if  (((iScheme_(rcb) & Ids_UseScale_1) || (iScheme_(rcb) & Ids_UseScale_2))
	 && (iScheme_(rcb) & Ids_UseArea))
        LPhoto_(xiectx) = (XiePhoto) IdsCreateLutForExport( ctx, 
						            xiedat, 
							    Photo_(xiectx), 
						            phoctx, 
						            Dpy_(ctx) );
    

#ifdef TRACE
printf( "Leaving Routine CompileRenditionPipe in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/*******************************************************************************
**  CompileExportPipe()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**	ctx	     - rendering context.
** 	xiectx       - XIE rendering context.
** 	phoctx	     - photo context structure.
**
**  FUNCTION VALUE: void
**
**
*******************************************************************************/
static void 	CompileExportPipe( ctx, xiectx, phoctx )
 RenderContext	    ctx;
 RenderContextXie   xiectx;
 PhotoContext       phoctx;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    DataForXie        xiedat = PriXie_(xiectx);   
    XGCValues         gcv;
    unsigned long     number_of_planes;  

#ifdef TRACE
printf( "Entering Routine CompileExportPipe in module IDS_BUILD_XIE_PIPE \n");
#endif

    RPixGC_(phoctx)  = 0;
    /*
    **  Choose GC to use with bitonal image if the image is bitonal.
    */
    if( RenD_(phoctx) == 1 )    
	CGC_(xiedat) = RPol_(phoctx) == XieK_ZeroBright ? CStdGC_(xiedat)
                                                        : CRevGC_(xiedat);
    if( iProto_(rcb) == Ids_Pixmap )
        {
        /*
	** If protocol is Pixmap export photoflo to pixmap. Use Pixmap later
	*/
	CPix_(xiedat) = XCreatePixmap( Dpy_(ctx), Win_(ctx),
				       RenW_(phoctx), RenH_(phoctx), 
				       RenD_(phoctx) == 1
				       ? 1 : DefaultDepthOfScreen(Scr_(ctx)) );
        if( CPix_(xiedat) != 0 )
            {
            /*
            **  Load pixmap from the photomap
            */
            if( Rcmp_(phoctx) != XieK_Bitonal )
                XieExport( Photo_(xiectx),         /* server Photo{flo|map}*/
                           CPix_(xiedat),           /* pixmap id            */
                           CGC_(xiedat),            /* continuous tone  GC  */
                           0, 0,                    /* from:  img X,Y       */
                           0, 0,                    /* to: drawable X,Y     */
                           RenW_(phoctx),           /* width                */
                           RenH_(phoctx),           /* height               */
                           LPhoto_(xiectx),         /* no client LUT        */
                           Cmap_(ctx),              /* colormap             */
                           MchLim_(ctx),            /* match_limit          */
                           GraLim_(ctx));           /* gray_limit           */

            else
                {
                /*
                **  We need a temporary GC for loading the pixmap; can't
                **  use one of our standard GCs because the drawable,
                **  Pix_(xiectx), may not be of the same depth as XtWindow(xiectx).
                gcv.foreground  = RPol_(phoctx) == XieK_ZeroBright ? 1 : 0;
                gcv.background  = RPol_(phoctx) == XieK_ZeroBright ? 0 : 1;
		RPixGC_(phoctx) = XCreateGC( Dpy_(ctx), CPix_(xiedat),
					     GCForeground | GCBackground, &gcv);
                */
		RPixGC_(phoctx) = XCreateGC( Dpy_(ctx), CPix_(xiedat),0, &gcv);

                XieExport( Photo_(xiectx),          /* server Photo{flo|map}*/
                           CPix_(xiedat),           /* Pixmap id            */
                           RPixGC_(phoctx),         /* bitonal GC           */
                           0, 0,                    /* from:  img X,Y       */
                           0, 0,                    /* to: drawable X,Y     */
                           RenW_(phoctx),           /* width                */
                           RenH_(phoctx),           /* height               */
                           LPhoto_(xiectx),         /* no client LUT        */
                           Cmap_(ctx),              /* colormap             */
                           MchLim_(ctx),            /* match_limit          */
                           GraLim_(ctx) );          /* gray_limit           */
                }
            }
        else
            XtWarningMsg("PixAllErr","PostRendering","IdsImageError",
                         "can't allocate Pixmap, using XImage instead",0,0);
        }

    else if( iProto_(rcb) == Ids_Photomap )
        /*
	** If protocol is Photomap tap before exec of photoflo.Use Photmap later
        ** for panning. 
	*/
	TapPhoto( ctx, xiectx );

    if ( iProto_(rcb) == Ids_Window )
        {
	/* if we're in window mode. Keep a copy of what is currently in the
	window so that on expose events which don't require re-rendition,
	we can refresh from the pixmap 
	*/

        CPix_(xiedat) = XCreatePixmap( Dpy_(ctx), Win_(ctx),
                    ((RenW_(phoctx) < WinW_(ctx)) ? RenW_(phoctx) : WinW_(ctx)),
                    ((RenH_(phoctx) < WinH_(ctx)) ? RenH_(phoctx) : WinH_(ctx)),
				       RenD_(phoctx) == 1
				       ? 1 : DefaultDepthOfScreen(Scr_(ctx)) );

        if (RenW_(phoctx) <= WinW_(ctx) )
            IULx_(xiedat) = 0;

        if (RenH_(phoctx) <= WinH_(ctx) )
            IULy_(xiedat) = 0;
 
        if( CPix_(xiedat) != 0 )
            {
            if( Rcmp_(phoctx) != XieK_Bitonal )
                XieExport( Photo_(xiectx),         /* server Photo{flo|map}*/
                           CPix_(xiedat),           /* pixmap id            */
                           CGC_(xiedat),            /* continuous tone  GC  */
                           IULx_(xiedat),IULy_(xiedat),/* from:  img X,Y   */
                           0,0,			    /* to: drawable X,Y     */
                    ((RenW_(phoctx) < WinW_(ctx)) ? RenW_(phoctx) : WinW_(ctx)),
                    ((RenH_(phoctx) < WinH_(ctx)) ? RenH_(phoctx) : WinH_(ctx)),
                           LPhoto_(xiectx),         /* client LUT           */
                           Cmap_(ctx),              /* colormap             */
                           MchLim_(ctx),            /* match_limit          */
                           GraLim_(ctx));           /* gray_limit           */
            else
                {
                /*
                **  We need a temporary GC for loading the pixmap; can't
                **  use one of our standard GCs because the drawable,
                **  Pix_(xiectx), may not be of the same depth as 
		**  XtWindow(xiectx).
		*/
		RPixGC_(phoctx) = XCreateGC( Dpy_(ctx), CPix_(xiedat),0, &gcv);

                XieExport( Photo_(xiectx),          /* server Photo{flo|map}*/
                           CPix_(xiedat),           /* Pixmap id            */
                           RPixGC_(phoctx),         /* bitonal GC           */
                           IULx_(xiedat),IULy_(xiedat),/* from:  img X,Y   */
			   0,0,			    /* to: drawable X,Y     */
                    ((RenW_(phoctx) < WinW_(ctx)) ? RenW_(phoctx) : WinW_(ctx)),
                    ((RenH_(phoctx) < WinH_(ctx)) ? RenH_(phoctx) : WinH_(ctx)),
                           LPhoto_(xiectx),         /* client LUT           */
                           Cmap_(ctx),              /* colormap             */
                           MchLim_(ctx),            /* match_limit          */
                           GraLim_(ctx) );          /* gray_limit           */
                }
            }

        }

#ifdef TRACE
printf( "Leaving Routine CompileExportPipe in module IDS_BUILD_XIE_PIPE \n");
#endif
    }

/*******************************************************************************
**  CompileExecutePipe()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**	ctx	     - rendering context.
** 	xiectx       - XIE rendering context.
** 	phoctx	     - photo context structure.
**
**  FUNCTION VALUE:	void
**
**
*******************************************************************************/
static void 	CompileExecutePipe( ctx, xiectx, phoctx )
 RenderContext	    ctx;
 RenderContextXie   xiectx;
 PhotoContext       phoctx;

{
    IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine CompileExecutePipe in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    ** This macro puts the worknotify callback box up thus generating an extra
    ** expose event. When the pipe is built and ready to execute and the 
    ** events are processed at the server along with the xie events, this 
    ** expose event triggers the RenderImage function and again
    ** a new xie pipe is built. This causes two photoflows existing back to 
    ** back with the first photoflo not being freed and hence the noticed 
    ** problem. For now, this is commented out so that no new expose 
    ** events are generated and thus works fine. In this case the single 
    ** photoflo never becomes zero.
    */
   
    /*
    ** Let the application know that XIE pipe execution has started.
    */
    PipeDone_(xiectx)(0, iScheme_(rcb), XieWidget_(xiectx) );

    /*
    **  Flow the source image data through the Photoflo pipeline.
    */
    if (Photo_(xiectx))
	XieExecuteFlo( Photo_(xiectx) );

    if( !RRend_(xiectx) )
	{
	/*
	**  Transport the image data into the running Photoflo.
	*/
	while( StreamPending_(iXieimage_(rcb)) )
	      XiePutImageData(iXieimage_(rcb));	    /* original image data  */

        }

    /*
    ** Set the rendering flag to True so that pipe building can take a 
    ** different path to rerender with the permanent original photomap 
    */
    if (Photo_(xiectx))
    {
        RRend_(xiectx) = True;
        NotifyEvent( iScheme_(rcb), xiectx ); 
	if( RPixGC_(phoctx) != 0 )
	{
	    XFreeGC( Dpy_(ctx), RPixGC_(phoctx) ); 
	    RPixGC_(phoctx) = 0;
	}
    }
#ifdef TRACE
printf( "Leaving Routine CompileExecutePipe in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/*******************************************************************************
**  ArithmeticPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== ISL function ImgScale: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
** 	roi	- XIE ROI structure.
**
**  FUNCTION VALUE:
**
**	void
**
*******************************************************************************/
static void	ArithmeticPhoto( ctx, roi )
 RenderContext	ctx;
 XieRoi		roi;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
#ifdef TRACE
printf( "Entering Routine ArithmeticPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

#ifdef TRACE
printf( "Leaving Routine ArithmeticPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

}

/*******************************************************************************
**  LogicalPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function IdsToneScale: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
** 	roi	- XIE ROI structure.
**
**  FUNCTION VALUE: void
**
**
*******************************************************************************/
static void	LogicalPhoto( ctx, roi )
 RenderContext  ctx;
 XieRoi         roi;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
#ifdef TRACE
printf( "Entering Routine LogicalPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

#ifdef TRACE
printf( "Leaving Routine LogicalPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/*******************************************************************************
**  CropPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function IdsSharpen: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	photo 	- source XIE photo
** 	xiedat  - XIE private data
** 	dpy	- display 
**
**  FUNCTION VALUE:
**
**	XIE photo
**
*******************************************************************************/
XiePhoto CropPhoto( photo, xiedat, dpy )
 XiePhoto           photo;
 DataForXie         xiedat;
 Display           *dpy;
{
    XieRoi   roi;
    XiePhoto crop_photomap;

#ifdef TRACE
printf( "Entering Routine CropPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    ** Create the roi with the dimensions 
    */
    roi  = XieCreateRoi( dpy, Croix_(xiedat), Croiy_(xiedat), 
				Croiw_(xiedat), Croih_(xiedat) );
    /*
    **  Create a grayscale photomap from an RGB photomap.
    */
    crop_photomap = XieCrop( photo, roi );    
       
    /*
    **  Free <== XIE function XieFreeResource: free server side roi resources.
    */
    XieFreeResource( roi );
            
#ifdef TRACE
printf( "Leaving Routine CropPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( crop_photomap );
}

/*******************************************************************************
**  InternalRoiPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function IdsSharpen: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	photo 	- source XIE photo
** 	xiedat  - XIE private data
** 	dpy	- display 
**
**  FUNCTION VALUE:
**
**	XIE photo
**
*******************************************************************************/
XiePhoto InternalRoiPhoto( photo, xiedat, dpy )
 XiePhoto           photo;
 DataForXie         xiedat;
 Display           *dpy;
{
    XieRoi   roi;
    XiePhoto internal_roi_photomap;


#ifdef TRACE
printf( "Entering Routine InternalRoiPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    ** Create the roi with the dimensions 
    */

    roi  = XieCreateRoi( dpy, Iroix_(xiedat), Iroiy_(xiedat), 
				Iroiw_(xiedat), Iroih_(xiedat) );
    /*
    **  Create a grayscale photomap from an RGB photomap.
    */
    internal_roi_photomap = XieCrop( photo, roi );    
       
    /*
    **  Free <== XIE function XieFreeResource: free server side roi resources.
    */
    XieFreeResource( roi );
            
#ifdef TRACE
printf( "Leaving Routine InternalRoiPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( internal_roi_photomap );
}

/*******************************************************************************
**  LuminancePhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**	Pipe <== IDS function IdsSharpen: returns a temporary fid.
**	Pipe <== ISL function ImgDeleteFrame (conditional).
**      
**  FORMAL PARAMETERS:
**
**	photo 	- source XIE photo
** 	dpy	- display 
**
**  FUNCTION VALUE:
**
**	XIE photo
**
*******************************************************************************/
XiePhoto LuminancePhoto( photo, dpy )
 XiePhoto           photo;
 Display           *dpy;
{
    XiePhoto luminance_photomap;

#ifdef TRACE
printf( "Entering Routine LuminancePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Create a grayscale photomap from an RGB photomap.
    */
    luminance_photomap = XieLuminance( photo );    /* server Photomap      */
       
#ifdef TRACE
printf( "Leaving Routine LuminancePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( luminance_photomap );
}

/******************************************************************************
**  AreaPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**  Perform the Area operation on the XiePhoto.
**      
**  FORMAL PARAMETERS:
**
**	rcb	- rendering context block.
**	photo 	- source XIE photo
** 	dpy	- display 
** 	xiedat  - XIE private data
**	phoctx 	- source XIE photo context
**
**  FUNCTION VALUE:
**
**	XIE photo
**
*******************************************************************************/
XiePhoto 	    AreaPhoto( rcb, ctx, photo, dpy, xiedat, phoctx )
IdsRenderCallback  rcb;
RenderContext	   ctx;
XiePhoto           photo;
Display           *dpy;
DataForXie         xiedat;
PhotoContext       phoctx;
    {
    XiePhoto      area_photomap;
    XieIdc        tmp_idc;
    XieIdc        idc;
    long          op1, op2;
    int		  sx, sy;
    long          choose_scale, kernel_width;
    float         sum, b, main_1d[5];
    long          j, k;    
    XieImage      img = iXieimage_(rcb);

    /* initialize the kernel */

    static XieTemplateRec area_kernel_points[25] = {
        {0, 4, 0.0}, {1, 4, 0.0}, {2, 4, 0.0}, {3, 4, 0.0}, {4, 4, 0.0},
        {0, 3, 0.0}, {1, 3, 0.0}, {2, 3, 0.0}, {3, 3, 0.0}, {4, 3, 0.0},
        {0, 2, 0.0}, {1, 2, 0.0}, {2, 2, 255.0}, {3, 2, 0.0}, {4, 2, 0.0},
        {0, 1, 0.0}, {1, 1, 0.0}, {2, 1, 0.0}, {3, 1, 0.0}, {4, 1, 0.0},
        {0, 0, 0.0}, {1, 0, 0.0}, {2, 0, 0.0}, {3, 0, 0.0}, {4, 0, 0.0}
    };

#ifdef TRACE
printf( "Entering Routine AreaPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    XieQueryMap( photo, &RenW_(phoctx), &RenH_(phoctx), &Rcmp_(phoctx),
                &Rcnt_(phoctx), Rlevs_(phoctx), &RPol_(phoctx),
                &RAsp_(phoctx), NULL, NULL, NULL );

    if ((Iroiw_(xiedat) != 0) || (Iroih_(xiedat) != 0))
        {
        sx = Iroiw_(xiedat) * 1000 / RenW_(phoctx);
        sy = Iroih_(xiedat) * 1000 / RenH_(phoctx);
        }
    else
        {
        sx = Width_(img) * 1000 / iWide_(rcb);
        sy = Height_(img) * 1000 / iHigh_(rcb);
        }

    choose_scale  = (int)(sx > sy ? sx : sy);

    /* Determine the kernel width */
    if (choose_scale <= 1000)
        kernel_width = 0;
    else if (choose_scale >= 4000)
        kernel_width = 99;
    else kernel_width = ((choose_scale - 1000) * 99) / 4000;

    if (kernel_width == 0) 
        /*  special case for 0 width kernels */
	{
        for (j = 0; j < 5; j++)
            for (k = 0; k < 5; k++)
                area_kernel_points[j*5+k].value = (double) 0.0;
        area_kernel_points[12].value = (double) 254.0;
        }
    else 
       /* compute the 1d separable filters */
	{
        b = ( (float)(kernel_width)/100.0 );
        sum = 0.0;
        main_1d[0] = 2.0 * b - 1.0;
        if (main_1d[0] < 0.0)
            main_1d[0] = 0.0;
        main_1d[4] = main_1d[0];
        main_1d[3] = main_1d[1] = b;
        main_1d[2] = 1.0;

        for (j = 0; j < 5; j++)
            for (k = 0; k < 5; k++)
                sum += (main_1d[j]*main_1d[k]);

        sum = 254.0/sum;
        /*  generate the 2-d array for the kernel */
        for (j = 0; j < 5; j++)
            for (k = 0; k < 5; k++)
		 /*  main_1d X main_1d */
  	      {
              area_kernel_points[j*5+k].value =
                    (double)( sum * (main_1d[j]*main_1d[k]) );
	      }
        }

/*
    for (j = 0; j < 5; j++)
	{
        for (k = 0; k < 5; k++)
            printf ("kernel : %f", area_kernel_points[j*5+k].value);
	printf ("\n");
	}
*/
    tmp_idc = XieCreateTmp(
                   dpy,                			/* X11 display   */
                   2,                  			/* center x      */
                   2,                  			/* center y      */
                   25,                 			/* element count */
                   (XieTemplate) &area_kernel_points[0] /* template data */
                   );

    idc = 0;

    /*
    **  Add the AREA operation to the pipe. 
    */
    area_photomap = XieArea( photo,            /* server Photomap       */
                             tmp_idc,          /* template idc (kernel) */
			     idc,	       /* idc id                */
			     XieK_AreaOp1Mult, /* operation code        */
			     XieK_AreaOp2Sum,  /* operation code        */
			     0                 /* No constrain          */
			   );

    XieFreeResource( tmp_idc );

#ifdef TRACE
printf( "Leaving Routine AreaPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( area_photomap );
    }

/******************************************************************************
**  ConstrainPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**  Perform the Constrain operation on the XiePhoto.
**      
**  FORMAL PARAMETERS:
**
**	photo 	- source XIE photo
** 
**  FUNCTION VALUE:
**
**	XIE photo
**
*******************************************************************************/
XiePhoto ConstrainPhoto( photo )
XiePhoto photo;
    {
    unsigned long levels[XieK_MaxComponents];
    XiePhoto constrained_photomap;
    long model = XieK_HardClip;

#ifdef TRACE
printf( "Entering Routine ConstrainPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    levels[0] = levels[1] = levels[2] = 256;
    constrained_photomap = XieConstrain(photo,       /* server Photomap   */
					model,	     /* Constrain model   */
					levels);     /* grey levels       */

#ifdef TRACE
printf( "Leaving Routine ConstrainPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( constrained_photomap );
    }

/******************************************************************************
**  ScalePhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**  Scale the XiePhoto to the requested width and height.
**      
**  FORMAL PARAMETERS:
**
**	rcb	- rendering context block.
**	photo 	- source XIE photo
** 	xiedat  - XIE private data
** 	dpy	- display 
**
**  FUNCTION VALUE:
**
**	xie photo 
**
*******************************************************************************/
XiePhoto 	    ScalePhoto( rcb, photo, xiedat, dpy )
 IdsRenderCallback  rcb;
 XiePhoto           photo;
 DataForXie         xiedat;
 Display           *dpy;
{
    XieImage  img = iXieimage_(rcb);
    XiePhoto scaled_photomap;
    long int width,height;

#ifdef TRACE
printf( "Entering Routine ScalePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Do the required scaling
    */

    /* 
    ** If we have done a rotate in the pipe, give scale the new
    ** dimensions. 
    */
    if (((IrW_(xiedat) != 0) && (IrH_(xiedat) != 0))
	&& (iScheme_(rcb) & Ids_UseScale_2))
	{
	width  = (long int )(IrW_(xiedat) *  iXSc_(rcb));
        height = (long int )(IrH_(xiedat) * iYSc_(rcb));
	}
    else
	{
        width  = (long int )(Width_(img) *  iXSc_(rcb));
        height = (long int )(Height_(img) * iYSc_(rcb));
	}

    /*
    **  Scale the XiePhoto to the requested width and height. 
    */
    scaled_photomap = XieScale( photo,                   /* server Photomap   */
			        width,                   /* Width             */
		                height                   /* Height            */
                              );

#ifdef TRACE
printf( "Leaving Routine ScalePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( scaled_photomap );
}

/*******************************************************************************
**  RotatePhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**  Rotate the XiePhoto by "angle" degrees.
**
**      
**  FORMAL PARAMETERS:
**
**	rcb	- rendering context block.
**	photo 	- source XIE photo
** 	dpy	- X11 display 
**
**  FUNCTION VALUE:
**
**	new_fid	- pointer to where new Frame-id will be saved.
**
*******************************************************************************/
XiePhoto	    RotatePhoto( rcb,  photo, dpy )
 IdsRenderCallback  rcb;
 XiePhoto           photo;
 Display           *dpy;
{
    unsigned char pxl_pol, cmp_cnt, i;
    unsigned long fill[XieK_MaxComponents];
    XiePhoto rotated_photomap;

#ifdef TRACE
printf( "Entering Routine RotatePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    ** Rotate width,  height values do not take values < 0
    */

    if( iRWid_(rcb) <= 0 || iRHei_(rcb) <= 0 )
        iRWid_(rcb) = iRHei_(rcb) = 0;

    /*
    ** Set the fill values according to rotate options.
    */
    XieQueryMap(photo, NULL, NULL, NULL, &cmp_cnt, fill, &pxl_pol, NULL, 
		NULL, NULL, NULL );
    for( i = 0; i < cmp_cnt; i++ )
	if( iROpts_(rcb) & ImgM_ReverseEdgeFill )
	    fill[i] = pxl_pol != XieK_ZeroBright ? 0 : fill[i]-1;   /* black*/
	else
	    fill[i] = pxl_pol == XieK_ZeroBright ? 0 : fill[i]-1;   /* white*/

    /*
    **  Rotate the XiePhoto by "angle" degrees.
    */

    /* VXT does not handle negative 90 rotation, always set this to 270 */

    if (iRAng_(rcb) == -90.0)
        iRAng_(rcb) = 270.0;
 
    rotated_photomap = XieRotate( photo,            /* server Photomap      */
			          iRAng_(rcb),      /* Angle                */
                                  iRWid_(rcb),      /* width                */
				  iRHei_(rcb),      /* height               */
				  fill );	    /* fill                 */
       
#ifdef TRACE
printf( "Leaving Routine RotatePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( rotated_photomap );
}

/*******************************************************************************
**  MirrorPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**  mirror a photomap producing a new photomap.
**      
**  FORMAL PARAMETERS:
**
**	xiectx	- XIE rendering context.
**	photo 	- source XIE photo
** 	dpy	- X11 display 
**
**  FUNCTION VALUE:
**
**
*******************************************************************************/
XiePhoto	    MirrorPhoto( xiectx,  photo, dpy )
 RenderContextXie   xiectx;
 XiePhoto           photo;
 Display           *dpy;
{
    XiePhoto mirrored_photomap;

#ifdef TRACE
printf( "Entering Routine MirrorPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Mirror the photomap as requested
    */
    mirrored_photomap = XieMirror(photo,            /* server Photomap      */
                            Rxmir_(xiectx),	    /* Flip about x flag    */
                            Rymir_(xiectx));        /* Flip about y flag    */

    Rxmir_(xiectx) = Rymir_(xiectx) = False;

#ifdef TRACE
printf( "Leaving Routine MirrorPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( mirrored_photomap );
}

/*******************************************************************************
**  TranslatePhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**  translate a photomap.
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	xiectx	- XIE rendering context.
**
**  NOTE: translate can operate in place (ie. src_pmp maybe = dst_pmp)
**
**  FUNCTION VALUE:
**
**	nil (void)
**
*******************************************************************************/
static void	    TranslatePhoto( ctx, xiectx )
 RenderContext    ctx;
 RenderContextXie xiectx;
{

#ifdef TRACE
printf( "Entering Routine MirrorPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Translate an area of the XiePhoto.
    */
    XieTranslate(Photo_(xiectx),                  /* server Photomap      */
	         Tsrc_(xiectx),	                  /* server src ROI       */
	         Photo_(xiectx),                  /* server Photomap      */
		 Tdst_(xiectx)		          /* server dst ROI       */
		);

#ifdef TRACE
printf( "Leaving Routine MirrorPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/*******************************************************************************
**  ToneScalePhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**   Do the needed color or gary tonescale adjustment
**      
**  FORMAL PARAMETERS:
**
** 	rcb 	- rendering context block.
**	ctx	- rendering context.
**	photo 	- source XIE photo
**	phoctx	- XIE photo context.
** 	dpy	- X11 display 
**
**
**  FUNCTION VALUE:
**
**	Photo
** 
*******************************************************************************/
XiePhoto	    ToneScalePhoto( rcb, ctx, photo, phoctx, dpy )
 IdsRenderCallback  rcb;
 RenderContext	    ctx;
 XiePhoto	    photo;
 PhotoContext       phoctx;
 Display           *dpy;
{
    XiePhoto    point_photomap;
    XiePhoto    tone_photo;
    XieImage    tone_xieimg = 0; 
    XieIdc      roi = (XieIdc) iROI_(rcb);
    int         i;

#ifdef TRACE
printf( "Entering Routine ToneScalePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Return dimensions, mapping, and pixel polarity of the rendered photomap
    **  Photo{map|tap}. Fill up the phoctx 
    */
    XieQueryMap( photo, &RenW_(phoctx), &RenH_(phoctx), &Rcmp_(phoctx),
                &Rcnt_(phoctx), Rlevs_(phoctx), &RPol_(phoctx),
                &RAsp_(phoctx), NULL, NULL, NULL );
    for( i = 0, RenD_(phoctx) = 0; i < Rcnt_(phoctx); i++ )
        RenD_(phoctx) += BitsFromLevels_(Rlevs_(phoctx)[i]);

    /*
    **  Create a lut taking punch_1 and punch_2 into consideration
    */
    tone_xieimg = IdsCreateLut( phoctx, TsLev_(ctx),&iPun1_(rcb),&iPun2_(rcb) );
    
    /*
    **  Send the lookup table into a photo map
    */
    tone_photo = XiePutImage( dpy, tone_xieimg, XieK_DataCopy );
  
    /* 
    **	Since we are pipelined, bind the tone scale photomap to photoflo
    */
    XieBindMapToFlo( photo, tone_photo );

    /*
    **  Point Remap for tone scale adjustment of the XiePhoto. 
    */
    point_photomap  =  XiePoint( photo,            /* server Photomap      */
				 roi,              /* CPP or ROI           */
                                 tone_photo );     /* LUT photomap         */
    /*
    ** Free the lut xieimage, returns null ptr
    */
    tone_xieimg = XieFreeImage( tone_xieimg );
/* printf("\n Freed tone scale xieimage  ToneScalePhoto" ); */

#ifdef TRACE
printf( "Leaving Routine ToneScalePhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( point_photomap );
}

/*******************************************************************************
**  DitherPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**  Dither the XiePhoto to the number of levels specified.
**      
**  FORMAL PARAMETERS:
**
** 	rcb 	- rendering context block.
**	photo 	- source XIE photo
** 	dpy	- X11 display 
**
**  FUNCTION VALUE:
**
**	XIE photo
** 
*******************************************************************************/
XiePhoto	    DitherPhoto( rcb, photo, dpy )
 IdsRenderCallback  rcb;
 XiePhoto	    photo;
 Display           *dpy;
{

    XiePhoto    dithered_photomap;

#ifdef TRACE
printf( "Entering Routine DitherPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Dither the XiePhoto to the number of levels specified.
    */
    dithered_photomap = XieDither( photo,            /* server Photomap      */
			    iGRA_(rcb) != 0 ? &iGRA_(rcb) : &iRGB_(rcb)[RED] );
			                             /* levels per component */
#ifdef TRACE
printf( "Leaving Routine DitherPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
    return( dithered_photomap );
}

/*******************************************************************************
**  TapPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
** 
**  Tap the Photoflo so we can save the result.
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	xiectx	- xie rendering context.
**
**  FUNCTION VALUE:
**
**	nil (void)
**
*******************************************************************************/
static void	    TapPhoto( ctx, xiectx )
 RenderContext    ctx;
 RenderContextXie xiectx;
{
    IdsRenderCallback rcb = PropRnd_(ctx);
    XiePhoto        photo;

#ifdef TRACE
printf( "Entering Routine TapPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    if( iProto_(rcb) == Ids_Photomap )
        {
	/*
	** If the original photomap has some rendition in the pipe then  
        ** XieQueryMap returns null else returns raw photo
        */
        photo = XieQueryMap( Photo_(xiectx), NULL, NULL, NULL, NULL, NULL, 
			NULL, NULL, NULL, NULL, NULL);
        if( photo == NULL )
            DPhoto_(xiectx) = XieTapFlo( Photo_(xiectx), True );
	else 
	    DPhoto_(xiectx) = photo;
        }
#ifdef TRACE
printf( "Leaving Routine TapPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/*******************************************************************************
**  ExportPhoto
**
**  FUNCTIONAL DESCRIPTION:
**
**   Return dimensions, mapping, and pixel polarity of the final Photo{map|tap}.
**   for Postrendering and display
**   
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	xiectx	- xie rendering context.
** 	phoctx  - photo context.
**
**  FUNCTION VALUE:
**
**	ctx	- filled with photomap image attributes.
**
*******************************************************************************/
static void	    ExportPhoto( ctx, xiectx, phoctx )
 RenderContext    ctx;
 RenderContextXie xiectx;
 PhotoContext     phoctx;
{
    int i;
  
#ifdef TRACE
printf( "Entering Routine ExportPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    **  Return dimensions, mapping, and pixel polarity of the rendered photomap
    **  Photo{map|tap}. Fill up the phoctx 
    */
    XieQueryMap( Photo_(xiectx), &RenW_(phoctx), &RenH_(phoctx),&Rcmp_(phoctx),
                &Rcnt_(phoctx), Rlevs_(phoctx), &RPol_(phoctx),
                &RAsp_(phoctx), NULL, NULL, NULL );

    for( i = 0, RenD_(phoctx) = 0; i < Rcnt_(phoctx); i++ )
        RenD_(phoctx) += BitsFromLevels_(Rlevs_(phoctx)[i]);

	

/* printf("\n photoflo  %d   raw_photo   %d \n", Photo_(xiectx),
							    TPhoto_(xiectx));*/
    Image_(ctx)->width  =  RenW_(phoctx);
    Image_(ctx)->height =  RenH_(phoctx);
    Image_(ctx)->depth  =  RenD_(phoctx);

#ifdef TRACE
printf( "Leaving Routine ExportPhoto in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/*******************************************************************************
**  FreeServerResources
**
**  FUNCTIONAL DESCRIPTION:
**
**  Generic free for XIE server resource
**      
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
**	xiectx	- xie rendering context.
**
**  FUNCTION VALUE:
**
**	nil (void)
**
*******************************************************************************/
static void	    FreeServerResources( ctx, xiectx )
 RenderContext	    ctx;
 RenderContextXie   xiectx;
{
   unsigned long      count;
   char               cmp_cnt;
 
   IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine FreeServerResources in module IDS_BUILD_XIE_PIPE \n");
#endif

   if( RRend_(xiectx) && Photo_(xiectx))
	{
    	if( iProto_(rcb) == Ids_Photomap )
	    XieQueryExport(Photo_(xiectx), &LPhoto_(xiectx), &PxlLst_(ctx), 
								&PxlCnt_(ctx)); 
	else
	    XieQueryExport( Photo_(xiectx), NULL, &PxlLst_(ctx), 
								&PxlCnt_(ctx)); 

        /* 
        ** If pixmap mode and the context hasn't been freed, free it now.
        ** It is no longer needed.
        */
        if ( iProto_(rcb) == Ids_Pixmap ) 
            XieFreeExport( Photo_(xiectx) );

        Photo_(xiectx) = XieFreeResource( Photo_(xiectx) );
/* printf("\n%d     pixel%s allocated.\n", count, count == 1 ? "" : "s"); */
/*        printf("\n Freed Photoflo FreeServerResources\n" ); */

        }

    if  (LPhoto_(xiectx) != 0)
	{
        XieFreeResource((XiePhotomap)LPhoto_(xiectx));
	LPhoto_(xiectx) = 0;
	}

#ifdef TRACE
printf( "Leaving Routine FreeServerResources in module IDS_BUILD_XIE_PIPE \n");
#endif
}

/****************************************************************************
**  IdsCreateLut()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a LUT.
**
**
**  FORMAL PARAMETERS:
**
** 	phoctx        - Photo context.
**      dlevs[];      - number of dest output levels/comp
**      punch_1	      - The lower bound of the contrast enhancement function.
**      punch_2	      - The upper bound of the contrast enhancement function.
**
**  IMPLICIT INPUTS:
**	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**
**	returns new lut	- values calucalted according to new "punch factors".
**
**  SIGNAL CODES:
**
*******************************************************************************/
static XieImage IdsCreateLut( phoctx, dlevs, punch_1, punch_2 )
PhotoContext       phoctx;
unsigned long      dlevs[];            /* number of dest output levels/comp*/
float             *punch_1;
float             *punch_2;
{ 
    XieImage      tone_lut;
    float  punch1, punch2, val;
    long   range,slev,i,imax,imin,half,hi,lo,neg=0;
    int    src_maxval, dst_maxval, comp;
    long   temp;                             /* temp buffer for lut reverse  */
    long   j;
    unsigned char  *temp_ptr1, *temp_ptr2;
	   
#ifdef TRACE
printf( "Entering Routine IdsCreateLut in module IDS_BUILD_XIE_PIPE \n");
#endif

    /*
    ** Obtain the highest # of levels of src image for lut size
    */
    for( slev = 0, i = 0; i < Rcnt_(phoctx); i++ )
	{
        if( slev < Rlevs_(phoctx)[i] )
            slev = Rlevs_(phoctx)[i] ;
	}
    /*
    ** Create a xieimage  which forms the lookup table for tonescale 
    ** remap operation. height = 1, width = highest # of levels of any compon
    ** PCM, XieK_BandByPlane, comp_map - same as of photo at server, comp levels
    ** -- array of dst levels,  pixel stride -- 8, scanline stride -- 0
    */
    tone_lut   =  XieCreateImage(slev,                  /* wid # of src lvls */
                                 1,                     /* hgt One scanline  */
                                 XieK_PCM,              /* no compression    */
                                 XieK_BandByPlane,      /* comp org          */
                                 Rcmp_(phoctx),         /* cmp map of src pho*/
                                 Rcnt_(phoctx),         /* cmp cnt of src pho*/
                                 dlevs,                 /* dst levels        */
                                 8,                     /* pixel stride mod  */
				 8 );                   /* scan stride mod   */
   
  


    /*
    **  Fill up the lookup table data which will map each pixel to its inverse
    */
    for( comp = 0; comp < Rcnt_(phoctx); comp++ )
        {
        punch1 = *punch_1;
        punch2 = *punch_2;
        if( punch1 > punch2 )
	    neg = 1, val = punch1, punch1 = punch2, punch2 = val;


        uBase_(tone_lut,comp) = XieCallocBits( uArSize_(tone_lut,comp) );

	src_maxval = Rlevs_(phoctx)[comp] - 1;   
	punch2 *= src_maxval, punch1 *= src_maxval;
	lo = punch1 + (punch1 > 0.0 ? 0.5 : -0.5 ); /* round */
	hi = punch2 + (punch2 > 0.0 ? 0.5 : -0.5 );

	/*
	** Compute area of table between punch1 and punch2.
	*/
	imin  = MAX( lo, 0 );
	imax  = MIN( hi, src_maxval );
	range = imax - imin;
      
	dst_maxval =  dlevs[comp] - 1;

        /*
        ** Prepare the LUT for the tonescale func, x-axis input y-axis ouput 
        ** levels value.
        */
        if( range != 0 )
         for( i = imin; i < imax; i++ )
          *(uBase_(tone_lut,comp) + i) = 
			           (int)(dst_maxval * (i - imin) / range + 0.5);
	/*
	** Fill punch2 clip area
	*/
	for( i = imax; i <  slev; i++ )
          *( uBase_(tone_lut,comp) + i ) = dst_maxval;

	
	 /* for( i = 0; i < slev; i++ )
	 ** printf( "     %d\n",     *( uBase_(tone_lut,comp) + i ));       
	 */
	
        /*
	** If necessary negate the input by reversing the order of the table
        ** some more work over here
	*/
	if( neg ) 
	   {
           temp_ptr1 = XieCallocBits( uArSize_(tone_lut,comp) );
	   temp_ptr2 = uBase_(tone_lut,comp);
	   for( i = 0; i < slev; i++ )
		  temp_ptr1[i-1] = temp_ptr2[slev - i];
	   XieFree(temp_ptr2);
	   uBase_(tone_lut,comp) = temp_ptr1;
	/*
	** printf("\n After inversion\n");
	** for( i = 0; i < slev; i++ )
	** printf( "     %d\n",     *( uBase_(tone_lut,comp) + i ));       
	*/
	   }
        }

    OwnData_(tone_lut) = TRUE;           /* Let library free the buffer  */   

#ifdef TRACE
printf( "Leaving Routine IdsCreateLut in module IDS_BUILD_XIE_PIPE \n");
#endif
    return (tone_lut);
} 

/*******************************************************************************
**  NotifyEvent
**
**  FUNCTIONAL DESCRIPTION:
**
**  Look at all the events in X11's event queue
**      
**  FORMAL PARAMETERS:
**
**	scheme  
** 	xiectx  - XIE rendering context.
**
**  FUNCTION VALUE:
**
**	nil (void)
**
*******************************************************************************/
static void	    NotifyEvent( scheme, xiectx )
 unsigned long      scheme;
 RenderContextXie   xiectx;
{
#ifdef TRACE
printf( "Entering Routine NotifyEvent in module IDS_BUILD_XIE_PIPE \n");
#endif

    if (Photo_(xiectx))
    {
	while( (XieQueryFlo( Photo_(xiectx) ) != XieK_PhotofloComplete ) &&
		(XieQueryFlo( Photo_(xiectx) ) != XieK_PhotofloAborted)) ;
    }
    
#ifdef TRACE
printf( "Leaving Routine NotifyEvent in module IDS_BUILD_XIE_PIPE \n");
#endif
}           

/****************************************************************************
**  IdsCreateLutForExport()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a LUT to be passed to export when the VXT 2000 atom
** 	is present.
**
**
**  FORMAL PARAMETERS:
**
**	ctx	- rendering context.
** 	xiedat  - XIE private data.
**	photo 	- source XIE photo
**	phoctx	- XIE photo context.
** 	dpy	- X11 display 
**
**  IMPLICIT INPUTS:
**	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**
**	returns a lut - containing values interpolated between 
** 			foreground and background colors.
**
**  SIGNAL CODES:
**
*******************************************************************************/
static XiePhoto IdsCreateLutForExport( ctx, xiedat, photo, phoctx, dpy )
RenderContext	   ctx;
DataForXie         xiedat;
XiePhoto	   photo;
PhotoContext       phoctx;
Display           *dpy;
    { 
    XieImage      lut;
    XiePhoto      lut_photo;
    long          i,c = 0;
    unsigned long key_value;
    XColor	  foreground;
    XColor	  background;
    XColor        *f_color;
    XColor        *b_color;
    XColor        *color;
    XColor        colors[14];
    long	  temp_pixel;
    static        dst_levels[] = {256, 0, 0};
    unsigned long midpoint;

#ifdef TRACE
printf( "Entering Routine IdsCreateLutForExport in module IDS_BUILD_XIE_PIPE \n");
#endif

    lut =  XieCreateImage(256,                  /* wid # of src lvls */
                            1,                  /* hgt One scanline  */
                          XieK_PCM,             /* no compression    */
                          XieK_BandByPlane,     /* comp org          */
                          Rcmp_(phoctx),        /* cmp map of src pho*/
                          Rcnt_(phoctx),        /* cmp cnt of src pho*/
                          dst_levels,           /* dst levels        */
                          8,                    /* pixel stride mod  */
			  8 );                  /* scan stride mod   */
   
    uBase_(lut,0) = XieCallocBits( uArSize_(lut,0) );
  
    /* get the two pixels to interpolate between */

    foreground.pixel  = CForePix_(xiedat);
    XQueryColor( dpy, Cmap_(ctx), &foreground);

    background.pixel = CBackPix_(xiedat);
    XQueryColor( dpy, Cmap_(ctx), &background);
    if (RPol_(phoctx) != XieK_ZeroBright)
	{
	temp_pixel =  foreground.pixel;
	foreground.pixel =  background.pixel;
     	background.pixel = temp_pixel;	
	}
                
    /* Load foreground and background values in front and back of the LUT. */

    for (i = 0; i < 16; i++)
       *( uBase_(lut,0) + i ) = background.pixel;
	
    for (i = 0; i < 16; i++)
	*( uBase_(lut,0) + (255 - i) ) = foreground.pixel;

    /*    
    ** Determine the 14 intermediate colors by interpolating between
    ** foreground and background.
    */

    key_value = background.red;
    for (i = 0; i < 15; i++)
	{
        key_value -= (65536 / 15);
	color = &colors[i];

    	color->red   = key_value;
        color->green = key_value; 
        color->blue  = key_value;
        color->pixel = 0;
        color->flags = DoRGB;
        color->pad   = 0;
	}

    /*
    **  Share colors that are perceptually "close" to what we want.
    **      "Close" depends on the distance resources specified by
    **      the application (IdsNmatchDistance and IdsNgrayDistance).
    */

    PxlDat_(ctx) = IdsAllocColors( Scr_(ctx), Cmap_(ctx), colors, 14,
                                   CSpace_(ctx), MchLim_(ctx),
                                         1.0 );

    if( PxlDat_(ctx) == NULL )
        {
        /*
        **  Apparently the colormap must be totally allocated private!
        */
        XtWarningMsg("ClrMapPvt","AllocateColors","IdsImageError",
                     "No color cells free to share in colormap.",0,0);

	}

    /*
    **  Fill up the rest of the lookup table data which will map each 
    **  pixel to its color.
    */

    for (i = 16; i < 240; i++)
	{
	*( uBase_(lut,0) + i) =  (unsigned long ) colors[c].pixel;
	
	/* Have we reached the point where the color changes
	** to the next value?
	*/
	if (i == (16 * (c + 2)))
	    c++;
	}

    OwnData_(lut) = TRUE;           

    /* Let library free the buffer  */   

    lut_photo = XiePutImage( dpy, lut, XieK_DataCopy );

    XieBindMapToFlo( photo, lut_photo );
    XieFreeImage((XieImage) lut);

#ifdef TRACE
printf( "Leaving Routine IdsCreateLutForExport in module IDS_BUILD_XIE_PIPE \n");
#endif
    return (lut_photo);
    } 

/*****************************************************************************
**  VxtImagingOption
**
**  FUNCTIONAL DESCRIPTION:
**
**      Performs a check to see if the vxt2000 bitonal image accelerator
**      is present.
**
**  FORMAL PARAMETERS:
**
**	dpy - display
** 	ctx - rendering context
** 
**  FUNCTION VALUE:
**
**
*****************************************************************************/
Boolean VxtImagingOption(dpy, ctx)
Display *dpy;
RenderContext ctx;
    {

Boolean retStatus   = FALSE;

#ifdef TRACE
printf( "Entering Routine VxtImagingOption in module IDS_BUILD_XIE_PIPE \n");
#endif

    if 
      ((XInternAtom (dpy, "_DEC_XIE_ACCEL_TYPE_A", True )) != 0) 
	{
        if (ctx != 0)
            {
            IdsRenderCallback rcb = PropRnd_(ctx);
            XieImage   img = iXieimage_(rcb);
	    /* 
	    ** Verify that this is a bitonal image 
	    ** and we're computing on the server.
	    */
            if (( intCompute_(rcb) == Ids_XieServer ) &&
                ((img == 0) || (CmpMap_(img) == XieK_Bitonal)))
		{
                retStatus = TRUE;
		}
            else
		{
                retStatus = FALSE;
		}
            }
        else
	    {
            retStatus = TRUE;
	    }
	}
    else
	{
        retStatus = FALSE;
	}

#ifdef TRACE
printf( "Leaving Routine VxtImagingOption in module IDS_BUILD_XIE_PIPE \n");
#endif
    return retStatus;
    }
