
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
**      This module implements the IDS resource management type converters.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**
**  AUTHOR(S):
**
**      Robert NC Shelley
**      Subu Garikapati
**
**  CREATION DATE:  February 29, 1988
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

    /*
    **  IDS and ISL include files
    */
#include    <img/ImgDef.h>		    /* ISL public definitions		    */
#include    <img/IdsImage.h>	    /* IDS public definitions		    */
#include    <img/ids_alloc_colors.h>    /* IDS color allocation definitions	    */
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/
    /*
    **  Public IDS  -- entry points
    */
#ifdef NODAS_PROTO
void		IdsStringToFloat();		/* string to float	    */
void		IdsStringToRenderMode();	/* string to render mode    */
void		IdsStringToComputeMode();	/* string to computemode    */
void		IdsStringToRenderClass();	/* string to rendering class*/
void		IdsStringToProtocol();		/* string to protocol type  */
void		IdsStringToRotateMode();	/* string to rotation mode  */
void		IdsStringToRotateOpts();	/* string to rotate options */
void		IdsStringToFlipOpts();		/* string to flip options   */
void		IdsStringToScaleMode();		/* string to scale mode	    */
void		IdsStringToScaleOpts();		/* string to scale options  */
void		IdsStringToDitherMode();	/* string to dither mode    */
void		IdsStringToColormapMode();	/* string to color space    */
void		IdsStringToColorSpace();	/* string to color space    */
void		IdsStringToSaveRendMode();	/* string to save image     */
void		IdsStringToCompressMode();	/* string to compress mode  */
void		IdsStringToComporgMode();	/* string to color org mode */
void		IdsStringToGravity();		/* string to gravity	    */
#ifdef VMS
void		IDS$STRING_TO_FLOAT();		/* string to float	    */
void		IDS$STRING_TO_RENDER_MODE();	/* string to render mode    */
void		IDS$STRING_TO_COMPUTE_MODE();	/* string to compute mode   */
void		IDS$STRING_TO_RENDER_CLASS();	/* string to rendering class*/
void		IDS$STRING_TO_PROTOCOL();	/* string to protocol type  */
void		IDS$STRING_TO_ROTATE_MODE();	/* string to rotation mode  */
void		IDS$STRING_TO_ROTATE_OPTS();	/* string to rotate options */
void		IDS$STRING_TO_FLIP_OPTS();	/* string to flip options   */
void		IDS$STRING_TO_SCALE_MODE();	/* string to scale mode	    */
void		IDS$STRING_TO_SCALE_OPTS();	/* string to scale options  */
void		IDS$STRING_TO_DITHER_MODE();	/* string to dither mode    */
void		IDS$STRING_TO_COLOR_SPACE();	/* string to color space    */
void		IDS$STRING_TO_SAVEREND_MODE();	/* string to save image     */
void		IDS$STRING_TO_COMPRESS_MODE();	/* string to compress mode  */
void		IDS$STRING_TO_COMPORG_MODE();	/* string to color org mode */
void		IDS$STRING_TO_GRAVITY();	/* string to gravity	    */
#endif
#endif
    /*
    **  supporting routines
    */
#ifndef IDS_NOX
#ifndef NODAS_PROTO
static void	LowerFrom();			/* string to lower case	    */
static void	ExtraArgsWarning();		/* "...needs no extra args" */
static void	StringCvtWarning();		/* "can't convert string..."*/
#else
PROTO(static void LowerFrom, (XrmValuePtr /*from*/, String /*dest*/));
PROTO(static void ExtraArgsWarning, (String /*name*/, String /*type*/));
PROTO(static void StringCvtWarning, (String /*name*/, XrmValuePtr /*from*/, XrmValuePtr /*to*/, String /*type*/));
#endif
#endif

/*
**  MACRO definitions
*/
#define Converted_(to, ptr, value) \
	( (*to).size=sizeof((*ptr)), (*to).addr=(caddr_t)ptr, (*ptr)=(value) )

/*
**  Equated Symbols
*/
#define ALPHA_NUM "0123456789abcdefghijklmnopqrstuvwxyz_"

/*
**  External References
*/

/*
**	Local Storage
*/
struct CONVERT {
    char *string;
    int	  mask;
    };
static struct CONVERT Render[] = {
    { IdsSPassive,	      Ids_Passive       },
    { IdsSNormal,	      Ids_Normal        },
    { IdsSOverride,	      Ids_Override      },
    { IdsSPurge,	      Ids_Purge         },
    { IdsSAbort,	      Ids_Abort         },
    { 0, 0 }};
static struct CONVERT Compute[] = {
    { IdsSIslClient,	      Ids_IslClient     },
    { IdsSXieServer,	      Ids_XieServer     },
    { 0, 0 }};
static struct CONVERT RenderClass[] = {
    { IdsSBitonal,	      Ids_Bitonal       },
    { IdsSGrayScale,	      Ids_GrayScale	},
    { IdsSColor,	      Ids_Color		},
    { 0, 0 }};
static struct CONVERT Protocol[] = {
    { IdsSXImage,	      Ids_XImage     },
    { IdsSPixmap,	      Ids_Pixmap     },
    { IdsSSixel,	      Ids_Sixel      },
    { IdsSPostScript,	      Ids_PostScript },
    { IdsSFid,		      Ids_Fid},
    { IdsSPhotomap,	      Ids_Photomap},
    { 0, 0 }};
static struct CONVERT Rotate[] = {
    { IdsSNoRotate,	      Ids_NoRotate },
    { IdsSRotate,	      Ids_Rotate   },
    { IdsSBestFit,	      Ids_BestFit  },
    { 0, 0 }};
static struct CONVERT RotateOpts[] = {
    { IdsSBilinear,	    ~(ImgM_NearestNeighbor) },
    { IdsSNearestNeighbor,    ImgM_NearestNeighbor  },
    { IdsSReverseEdgeFill,    ImgM_ReverseEdgeFill },
    { 0, 0 }};
static struct CONVERT FlipOpts[] = {
    { IdsSFlipVertical,	      Ids_FlipVertical   },
    { IdsSFlipHorizontal,     Ids_FlipHorizontal },
    { 0, 0 }};
static struct CONVERT Scale[] = {
    { IdsSNoScale,	      Ids_NoScale   },
    { IdsSScale,	      Ids_Scale     },
    { IdsSPhysical,	      Ids_Physical  },
    { IdsSFitWithin,	      Ids_FitWithin },
    { IdsSFitWidth,	      Ids_FitWidth  },
    { IdsSFitHeight,	      Ids_FitHeight },
    { IdsSFlood,	      Ids_Flood     },
    { 0, 0 }};
static struct CONVERT ScaleOpts[] = {
    { IdsSSubsampleH,	    ~(ImgM_SaveHorizontal)    },
    { IdsSSubsampleV,	    ~(ImgM_SaveVertical
			    | ImgM_ReversePreference
			    | ImgM_DisablePreference) },
    { IdsSSaveH,	      ImgM_SaveHorizontal     },
    { IdsSSaveV,	      ImgM_SaveVertical       },
    { IdsSReversePreference,  ImgM_SaveVertical
			    | ImgM_ReversePreference  },
    { IdsSDisablePreference,  ImgM_SaveVertical
			    | ImgM_DisablePreference  },
    { IdsSBilinear,	    ~(ImgM_NearestNeighbor)   },
    { IdsSNearestNeighbor,    ImgM_NearestNeighbor    },
    { 0, 0 }};
static struct CONVERT Dither[] = {
    { IdsSRequantize,	      Ids_Requantize },
    { IdsSClustered,	      Ids_Clustered  },
    { IdsSBlueNoise,	      Ids_BlueNoise  },
    { IdsSDispersed,	      Ids_Dispersed  },
    { 0, 0 }};
static struct CONVERT Colormode[] = {
    { IdsSShareColors,	      Ids_ShareColors   },
    { IdsSPrivateColors,      Ids_PrivateColors },
    { 0, 0 }};
static struct CONVERT Savemode[] = {
    { IdsSSaveNone,	 Ids_SaveNone },
    { IdsSSaveFid,       Ids_SaveFid },
    { IdsSSaveXieimg,    Ids_SaveXieimg },
    { IdsSSavePhoto,     Ids_SavePhoto },
    { IdsSSaveDec,       Ids_SaveDec },
    { IdsSSaveXimage,    Ids_SaveXimage },
    { 0, 0 }};
static struct CONVERT Cmpresmode[] = {
    { IdsSUnCompress,	 Ids_UnCompress },
    { IdsSBitonalG42d,   Ids_CompressG42D },
    { IdsSColorDCT,      Ids_CompressDCT },
    { 0, 0 }};
static struct CONVERT Cmporgmode[] = {
    { IdsSBandByPixel,	 Ids_BandByPixel },
    { IdsSBandByPlane,   Ids_BandByPlane },
    { IdsSBitByPlane,    Ids_BitByPlane },
    { 0, 0 }};
static struct CONVERT ColorSpace[] = {
    { IdsSHLSSpace,	      Ids_HLSSpace },
    { IdsSLabSpace,	      Ids_LabSpace },
    { IdsSLUVSpace,	      Ids_LUVSpace },
    { IdsSRGBSpace,	      Ids_RGBSpace },
    { IdsSUVWSpace,	      Ids_UVWSpace },
    { IdsSYIQSpace,	      Ids_YIQSpace },
    { 0, 0 }};
static struct CONVERT Gravity[] = {
    { IdsSNoGravity,	      Ids_NoGravity  },
    { IdsSTop,		      Ids_Top        },
    { IdsSBottom,	      Ids_Bottom     },
    { IdsSRight,	      Ids_Right      },
    { IdsSLeft,		      Ids_Left       },
    { IdsSCenterHorz,	      Ids_CenterHorz },
    { IdsSCenterVert,	      Ids_CenterVert },
    { IdsSCenter,	      Ids_Center     },
    { IdsSNorth,	      Ids_North      },
    { IdsSSouth,	      Ids_South      },
    { IdsSEast,		      Ids_East       },
    { IdsSWest,		      Ids_West       },
    { IdsSNorthEast,	      Ids_NorthEast  },
    { IdsSNorthWest,	      Ids_NorthWest  },
    { IdsSSouthEast,	      Ids_SouthEast  },
    { IdsSSouthWest,	      Ids_SouthWest  },
    { 0, 0 }};

/*****************************************************************************
**  IdsStringToFloat
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "0.95e-1") to float.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToFloat( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static float f;

#ifdef TRACE
printf( "Entering Routine IdsStringToFloat in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToFloat", IdsRFloat );

    if( sscanf( (char *)from->addr, "%f", &f ) == 1 )
        Converted_( to, &f, f );
    else
	StringCvtWarning( "IdsStringToFloat", from, to, IdsRFloat );

#ifdef TRACE
printf( "Leaving Routine IdsStringToFloat in module IDS_CONVERTERS \n");
#endif

}
#endif

/*****************************************************************************
**  IdsStringToRenderMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "Normal") to a rendering mode.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToRenderMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToRenderMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToRenderMode", IdsRRenderMode );
    LowerFrom( from, lower );

    for( i = 0; Render[i].string != NULL; i++ )
	if( strcmp( lower, Render[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Render[i].mask );
	    break;
	    }

    if( Render[i].string == NULL )
	StringCvtWarning( "IdsStringToRenderMode", from, to, IdsRRenderMode );

#ifdef TRACE
printf( "Leaving Routine IdsStringToRenderMode in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToConvertMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "XieServer") to a rendering mode.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToComputeMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToComputeMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToComputeMode", IdsRComputeMode );
    LowerFrom( from, lower );

    for( i = 0; Compute[i].string != NULL; i++ )
	if( strcmp( lower, Compute[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Compute[i].mask );
	    break;
	    }

    if( Compute[i].string == NULL )
	StringCvtWarning( "IdsStringToComputeMode", from, to, IdsRComputeMode );

#ifdef TRACE
printf( "Leaving Routine IdsStringToComputeMode in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToRenderClass
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "GrayScale") to a rendering class.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToRenderClass( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToRenderClass in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToRenderClass", IdsRRenderingClass );
    LowerFrom( from, lower );

    for( i = 0; RenderClass[i].string != NULL; i++ )
	if( strcmp( lower, RenderClass[i].string ) == 0 )
	    {
	    Converted_( to, &mode, RenderClass[i].mask );
	    break;
	    }

    if( RenderClass[i].string == NULL )
	StringCvtWarning("IdsStringToRenderClass", from,to, IdsRRenderingClass);

#ifdef TRACE
printf( "Leaving Routine IdsStringToRenderClass in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToProtocol
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "XImage") to a renderering protocol type.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToProtocol( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  protocol;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToProtocol in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToProtocol", IdsRProtocol );
    LowerFrom( from, lower );

    for( i = 0; Protocol[i].string != NULL; i++ )
	if( strcmp( lower, Protocol[i].string ) == 0 )
	    {
	    Converted_( to, &protocol, Protocol[i].mask );
	    break;
	    }

    if( Protocol[i].string == NULL )
	StringCvtWarning( "IdsStringToProtocol", from, to, IdsRProtocol );

#ifdef TRACE
printf( "Leaving Routine IdsStringToProtocol in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToColormapMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "Sharecolors") to colormapmode.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToColormapMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToColormapMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToColormapMode", IdsRColormapMode );
    LowerFrom( from, lower );

    for( i = 0; Colormode[i].string != NULL; i++ )
	if( strcmp( lower, Colormode[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Colormode[i].mask );
	    break;
	    }

    if( Colormode[i].string == NULL )
	StringCvtWarning("IdsStringToColormapMode",from, to, IdsRColormapMode );

#ifdef TRACE
printf( "Leaving Routine IdsStringToColormapMode in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToSaveRendMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "SaveNone") to savenone.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToSaveRendMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToSaveRendMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToSaveRendMode", IdsRSaveRendition);
    LowerFrom( from, lower );

    for( i = 0; Savemode[i].string != NULL; i++ )
	if( strcmp( lower, Savemode[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Savemode[i].mask );
	    break;
	    }

    if( Savemode[i].string == NULL )
	StringCvtWarning("IdsStringToSaveRendMode",from, to, IdsRSaveRendition);

#ifdef TRACE
printf( "Leaving Routine IdsStringToSaveRendMode in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToCompressMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "UnCompress") to uncompress.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToCompressMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToCompressMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToCompressMode", IdsRCompressMode);
    LowerFrom( from, lower );

    for( i = 0; Cmpresmode[i].string != NULL; i++ )
	if( strcmp( lower, Cmpresmode[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Cmpresmode[i].mask );
	    break;
	    }

    if( Cmpresmode[i].string == NULL )
	StringCvtWarning("IdsStringToCompressMode",from, to, IdsRCompressMode);

#ifdef TRACE
printf( "Leaving Routine IdsStringToCompressMode in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**   IdsStringToComporgMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "BandByPixel") to bandbypixel.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void  IdsStringToComporgMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToComporgMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( " IdsStringToComporgMode", IdsRComporgMode);
    LowerFrom( from, lower );

    for( i = 0; Cmporgmode[i].string != NULL; i++ )
	if( strcmp( lower, Cmporgmode[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Cmporgmode[i].mask );
	    break;
	    }

    if( Cmporgmode[i].string == NULL )
	StringCvtWarning(" IdsStringToComporgMode",from, to, IdsRComporgMode);

#ifdef TRACE
printf( "Leaving Routine IdsStringToComporgMode in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToRotateMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "BestFit") to rotation mode.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToRotateMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToRotateMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToRotateMode", IdsRRotateMode );
    LowerFrom( from, lower );

    for( i = 0; Rotate[i].string != NULL; i++ )
	if( strcmp( lower, Rotate[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Rotate[i].mask );
	    break;
	    }

    if( Rotate[i].string == NULL )
	StringCvtWarning( "IdsStringToRotateMode", from, to, IdsRRotateMode );

#ifdef TRACE
printf( "Leaving Routine IdsStringToRotateMode in module IDS_CONVERTERS \n");
#endif
}
#endif


/*****************************************************************************
**  IdsStringToRotateOpts
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert string (eg."NearestNeighbor") to rotate options.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToRotateOpts( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int    options;
       char   lower[CVT_STRING_SIZE];
       int    n, i;
       String s;

#ifdef TRACE
printf( "Entering Routine IdsStringToRotateOpts in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToRotateOpts", IdsRRotateOptions );
    LowerFrom( from, lower );
    Converted_( to, &options, 0 );
    for( s = lower; *s != 0; n += strcspn(s + n, ALPHA_NUM), s += n )
	{
	n = strspn( s, ALPHA_NUM );
	if( n != 0 )
	    {
	    for( i = 0; RotateOpts[i].string != NULL; i++ )
		if( strncmp( s, RotateOpts[i].string, n ) == 0 )
		    {
		    if( RotateOpts[i].mask < 0 )
			options &= RotateOpts[i].mask;
		    else
			options |= RotateOpts[i].mask;
		    break;
		    }
	    if( RotateOpts[i].string == NULL )
		{
		StringCvtWarning( "IdsStringToRotateOpts", from, to,
				   IdsRRotateOptions );
		break;
		}
	    }
	}
#ifdef TRACE
printf( "Leaving Routine IdsStringToRotateOpts in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToFlipOpts
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert string (eg."FlipVertical+FlipHorizontal") to flip options.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToFlipOpts( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int    options;
       char   lower[CVT_STRING_SIZE];
       int    n, i;
       String s;

#ifdef TRACE
printf( "Entering Routine IdsStringToFlipOpts in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToFlipOpts", IdsRFlipOptions );
    LowerFrom( from, lower );
    Converted_( to, &options, 0 );
    for( s = lower; *s != 0; n += strcspn(s + n, ALPHA_NUM), s += n )
	{
	n = strspn( s, ALPHA_NUM );
	if( n != 0 )
	    {
	    for( i = 0; FlipOpts[i].string != NULL; i++ )
		if( strncmp( s, FlipOpts[i].string, n ) == 0 )
		    {
		    if( FlipOpts[i].mask < 0 )
			options &= FlipOpts[i].mask;
		    else
			options |= FlipOpts[i].mask;
		    break;
		    }
	    if( FlipOpts[i].string == NULL )
		{
		StringCvtWarning( "IdsStringToFlipOpts", from, to,
				   IdsRFlipOptions );
		break;
		}
	    }
	}
#ifdef TRACE
printf( "Leaving Routine IdsStringToFlipOpts in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToScaleMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "FitWithin") to scale mode.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToScaleMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToScaleMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToScaleMode", IdsRScaleMode );
    LowerFrom( from, lower );

    for( i = 0; Scale[i].string != NULL; i++ )
	if( strcmp( lower, Scale[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Scale[i].mask );
	    break;
	    }

    if( Scale[i].string == NULL )
	StringCvtWarning( "IdsStringToScaleMode", from, to, IdsRScaleMode );

#ifdef TRACE
printf( "Leaving Routine IdsStringToScaleMode in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToScaleOpts
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert string (eg."SaveHorizontal|SubSampleVertical") to scale options.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToScaleOpts( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int    options;
       char   lower[CVT_STRING_SIZE];
       int    n, i;
       String s;

#ifdef TRACE
printf( "Entering Routine IdsStringToScaleOpts in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToScaleOpts", IdsRScaleOptions );
    LowerFrom( from, lower );
    Converted_( to, &options, 0 );
    for( s = lower; *s != 0; n += strcspn(s + n, ALPHA_NUM), s += n )
	{
	n = strspn( s, ALPHA_NUM );
	if( n != 0 )
	    {
	    for( i = 0; ScaleOpts[i].string != NULL; i++ )
		if( strncmp( s, ScaleOpts[i].string, n ) == 0 )
		    {
		    if( ScaleOpts[i].mask < 0 )
			options &= ScaleOpts[i].mask;
		    else
			options |= ScaleOpts[i].mask;
		    break;
		    }
	    if( ScaleOpts[i].string == NULL )
		{
		StringCvtWarning( "IdsStringToScaleOpts", from, to,
				   IdsRScaleOptions );
		break;
		}
	    }
	}

#ifdef TRACE
printf( "Leaving Routine IdsStringToScaleOpts in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToDitherMode
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "BlueNoise") to dither mode.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToDitherMode( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToDitherMode in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToDitherMode", IdsRDitherMode );
    LowerFrom( from, lower );

    for( i = 0; Dither[i].string != NULL; i++ )
	if( strcmp( lower, Dither[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Dither[i].mask );
	    break;
	    }

    if( Dither[i].string == NULL )
	StringCvtWarning( "IdsStringToDitherMode", from, to, IdsRDitherMode );

#ifdef TRACE
printf( "Leaving Routine IdsStringToDitherMode in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToColorSpace
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "LabSpace") to a color space.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToColorSpace( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  space;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToColorSpace in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToColorSpace", IdsRColorSpace );
    LowerFrom( from, lower );

    for( i = 0; ColorSpace[i].string != NULL; i++ )
	if( strcmp( lower, ColorSpace[i].string ) == 0 )
	    {
	    Converted_( to, &space, ColorSpace[i].mask );
	    break;
	    }

    if( ColorSpace[i].string == NULL )
	StringCvtWarning( "IdsStringToColorSpace", from, to, IdsRColorSpace );

#ifdef TRACE
printf( "Leaving Routine IdsStringToColorSpace in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsStringToGravity
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a string (eg. "NorthEast") to a gravity mask.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
void IdsStringToGravity( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from;
    XrmValuePtr to;
{
static int  mode;
       char lower[CVT_STRING_SIZE];
       int  i;

#ifdef TRACE
printf( "Entering Routine IdsStringToGravity in module IDS_CONVERTERS \n");
#endif

    if( *num_args != 0 )
	ExtraArgsWarning( "IdsStringToGravity", IdsRGravity );
    LowerFrom( from, lower );

    for( i = 0; Gravity[i].string != NULL; i++ )
	if( strcmp( lower, Gravity[i].string ) == 0 )
	    {
	    Converted_( to, &mode, Gravity[i].mask );
	    break;
	    }

    if( Gravity[i].string == NULL )
	StringCvtWarning( "IdsStringToGravity", from, to, IdsRGravity );

#ifdef TRACE
printf( "Leaving Routine IdsStringToGravity in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IdsNameToIndex
**
**  FUNCTIONAL DESCRIPTION:
**
**      Search a table of string pointers for an entry which matches the
**	specified string.
**
**  FORMAL PARAMETERS:
**
**	string
**	strtab
**	tabsiz
**
**  FUNCTION VALUE:
**
**	Returns the table entry index where matching string was found or
**	-1 if string was not found.
**
*****************************************************************************/
int IdsNameToIndex(string,strtab,tabsiz)
char *string;
char *strtab[];
int   tabsiz;
{
    int i, entry;

#ifdef TRACE
printf( "Entering Routine IdsNameToIndex in module IDS_CONVERTERS \n");
#endif

    entry = -1;
    for (i = 0; i < tabsiz; i++)
	if (strcmp(string,strtab[i]) == 0)
	    {
		entry = i;
		break;
	    }

#ifdef TRACE
printf( "Leaving Routine IdsNameToIndex in module IDS_CONVERTERS \n");
#endif
    return(entry);
}

/*****************************************************************************
**  LowerFrom
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert "convert-from" string to lower case.
**
**  FORMAL PARAMETERS:
**
**	from	- source string.
**	dest	- destination string.
**
*****************************************************************************/
#ifndef IDS_NOX
static void LowerFrom( from, dest )
    XrmValuePtr from;
    String	dest;
{
    String s, d;

#ifdef TRACE
printf( "Entering Routine LowerFrom in module IDS_CONVERTERS \n");
#endif

    for( s = (char *)from->addr, d = dest; *s != 0; s++, d++ )
	*d = ( *s < 'A' || *s > 'Z' ) ? *s : *s - 'A' + 'a';

    *d = '\0';

#ifdef TRACE
printf( "Leaving Routine LowerFrom in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  ExtraArgsWarning
**
**  FUNCTIONAL DESCRIPTION:
**
**      Warn that extra arguments are not needed.
**
**  FORMAL PARAMETERS:
**
**	name	- name of converter where error was detected.
**	type	- type to convert to.
**
*****************************************************************************/
#ifndef IDS_NOX
static void ExtraArgsWarning( name, type )
    String name, type;
{
    String params[1];
    Cardinal num_params = 1;

#ifdef TRACE
printf( "Entering Routine ExtraArgsWarning in module IDS_CONVERTERS \n");
#endif

    params[0] = type;

    XtWarningMsg("InvArgCnt", name, "IdsImageError",
		 "String to %s conversion needs no extra arguments",
		  params, &num_params );

#ifdef TRACE
printf( "Leaving Routine ExtraArgsWarning in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  StringCvtWarning
**
**  FUNCTIONAL DESCRIPTION:
**
**      Warn that extra arguments are not needed.
**
**  FORMAL PARAMETERS:
**
**	name	- name of converter where error was detected.
**	from	- string to be converted.
**	to	- destination of conversion.
**	type	- type to convert to.
**
*****************************************************************************/
#ifndef IDS_NOX
static void StringCvtWarning( name, from, to, type )
    String	name;
    XrmValuePtr from;
    XrmValuePtr to;
    String	type;
{
    String params[2];
    Cardinal num_params = 2;

#ifdef TRACE
printf( "Entering Routine StringCvtWarning in module IDS_CONVERTERS \n");
#endif

    params[0] = (char *)from->addr;
    params[1] = type;
    to->size  = 0;
    to->addr  = 0;

    XtWarningMsg("InvCvtStr", name, "IdsImageError",
		 "Cannot convert string \"%s\" to type %s",
		  params, &num_params );

#ifdef TRACE
printf( "Leaving Routine StringCvtWarning in module IDS_CONVERTERS \n");
#endif
}
#endif

/*****************************************************************************
**  IDS$STRING_TO_...
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS -- convert a string to an IDS resource type.
**
**  FORMAL PARAMETERS:
**
**	args	    - ??? not documented in intrinsic manual.
**	num_args    - ??? not documented in intrinsic manual.
**	from	    - pointer to structure containing pointer to string.
**	to	    - pointer to structure which will point to result.
**
*****************************************************************************/
#ifndef IDS_NOX
#ifdef VMS
void IDS$STRING_TO_FLOAT( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToFloat( args, num_args, from, to );
}
void IDS$STRING_TO_RENDER_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToRenderMode( args, num_args, from, to );
}
void IDS$STRING_TO_COMPUTE_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToComputeMode( args, num_args, from, to );
}
void IDS$STRING_TO_RENDER_CLASS( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToRenderClass( args, num_args, from, to );
}
void IDS$STRING_TO_PROTOCOL( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToProtocol( args, num_args, from, to );
}
void IDS$STRING_TO_ROTATE_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToRotateMode( args, num_args, from, to );
}

void IDS$STRING_TO_ROTATE_OPTS( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToRotateOpts( args, num_args, from, to );
}
void IDS$STRING_TO_FLIP_OPTS( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToFlipOpts( args, num_args, from, to );
}
void IDS$STRING_TO_SCALE_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToScaleMode( args, num_args, from, to );
}
void IDS$STRING_TO_SCALE_OPTS( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToScaleOpts( args, num_args, from, to );
}
void IDS$STRING_TO_DITHER_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToDitherMode( args, num_args, from, to );
}
void IDS$STRING_TO_COLORMAP_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToColormapMode( args, num_args, from, to );
}
void IDS$STRING_TO_SAVEREND_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToSaveRendMode( args, num_args, from, to );
}
void IDS$STRING_TO_COMPRESS_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToCompressMode( args, num_args, from, to );
}
void IDS$STRING_TO_COMPORG_MODE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToComporgMode( args, num_args, from, to );
}
void IDS$STRING_TO_COLOR_SPACE( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToColorSpace( args, num_args, from, to );
}
void IDS$STRING_TO_GRAVITY( args, num_args, from, to )
    XrmValuePtr args;
    Cardinal	*num_args;
    XrmValuePtr from,  to;
{
    IdsStringToGravity( args, num_args, from, to );
}
#endif
#endif
