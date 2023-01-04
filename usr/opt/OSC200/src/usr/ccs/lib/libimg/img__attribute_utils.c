
/***************************************************************************** 
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation. Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the 
**  U.S. Government is subject to restrictions as set forth in 
**  Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*****************************************************************************/

/************************************************************************
**  IMG__ATTRIBUTE_UTILS
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This module contains utility functions that create and manage
**	ISL internal data structures which describe attribute information
**	that is associated with image frames and image frame definitions.
**
**  ENVIRONMENT:
**
**	VAX/VMS. VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark Sornson
**
**  CREATION DATE:
**
**	2-MAY-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
**	Global routines
*/
#ifdef NODAS_PROTO
struct FAT	    *_ImgAdjustFat();
long		     _ImgConvertLevelsToBits();
struct FAT	    *_ImgCreateFat();
void		     _ImgDeleteCsa();
void		     _ImgDeleteFat();
struct CSA	    *_ImgExtractCsa();
struct FAT	    *_ImgExtractFat();
struct ITMLST	    *_ImgGetStandardizedAttrlst();
void		     _ImgVerifyFat();
#endif

/*
**  Include files:
*/
#include    <img/ChfDef.h>
#include    <img/ImgDef.h>
#include    <ImgDefP.h>
#include    <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif
#include    <math.h>
#include    <string.h>

#if defined(__VMS) || defined(VMS)
#include    <ddif$def.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include    <ddifdef.h>
#else
#include    <ddif_def.h>
#endif
#endif

/*
**  MACRO definitions:
**
**	NORMALIZE_BPC_VALUE ... Normalize a bits per component value.
**
**	    Transform any arbitrary bpc value into one the size
**	    of the nearest (i.e., minimum) addressable memory unit.
**	    For values <= 32, pick the closest of 8, 16, or 32.
**	    For values > 32, assume (for now) that the value is
**	    aligned, and return it as is.
*/
#define STANDARDIZE_BPC_VALUE_( bpc )	(( bpc <= 8 )?	    \
					    8:		    \
					(( bpc <= 16 )?	    \
					    16:		    \
					(( bpc <= 32 )?	    \
					    32: bpc )))

/*
**  Equated Symbols:
**
**	none
*/

/*
**  External References:
**
**	External Routines		    ........   from module   ........
*/
#ifdef NODAS_PROTO
void	 ChfSignal();			    /*				    */
void	 ChfStop();			    /*				    */

char	*_ImgCalloc();			    /* IMG__MEMORY_MGT		    */
void	 _ImgCfree();			    /* IMG__MEMORY_MGT		    */
long	 _ImgExtractItmlstItem();	    /* IMG__ITEMLIST_UTILS	    */
long	 _ImgGet();			    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
long	 _ImgItmlstItemCount();		    /* IMG__ITEMLIST_UTILS	    */
char	*_ImgMalloc();			    /* IMG__MEMORY_MGT		    */
char	*_ImgRealloc();			    /* IMG__MEMORY_MGT		    */
#endif


/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static struct FAT   *Allocate_fat();
static struct FAT   *Apply_defaults();
static void	     Apply_itmlst_to_fat();
static void	     Copy_user_label_strs();
static void	     Deallocate_fat();
#else
PROTO(static struct FAT *Allocate_fat, (long /*component_cnt*/, long /*ice_cnt*/, long /*plane_cnt*/));
PROTO(static struct FAT *Apply_defaults, (struct FAT */*fat*/, long /*image_data_class*/));
PROTO(static void Apply_itmlst_to_fat, (struct FAT */*fat*/, struct ITMLST */*itmlst*/));
PROTO(static void Copy_user_label_strs, (struct FAT */*fat_in*/, struct FAT */*fat_out*/));
PROTO(static void Deallocate_fat, (struct FAT */*fat*/));
#endif


/*
**	Symbol Definitions For Message Codes
*/
#include    <img/ImgStatusCodes.h>

/*
**  Local Storage:
**
**	Default item values
*/
static long Bit_order				= ImgK_LsbitFirst;
static long Bits_per_comp_bitonal		= 1;
static long Bits_per_comp_greyscale		= 8;
static long Bits_per_comp_multispect[3]		= { 8,8,8 };
static long Bits_per_comp_private		= 8;
static long Brt_polarity_bitonal		= ImgK_ZeroMaxIntensity;
static long Brt_polarity_greyscale		= ImgK_ZeroMinIntensity;
static long Brt_polarity_multispect		= ImgK_ZeroMinIntensity;
static long Brt_polarity_private		= ImgK_ZeroMinIntensity;
static long Byte_order				= ImgK_LsbyteFirst;
static long Byte_unit				= 1;
static long Comp_space_org_bitonal		= ImgK_BandIntrlvdByPixel;
static long Comp_space_org_greyscale		= ImgK_BandIntrlvdByPixel;
static long Comp_space_org_multispect		= ImgK_BandIntrlvdByPlane;
static long Compression_type			= ImgK_PcmCompression;
static long Data_offset				= 0;
static long Data_type_bitonal			= ImgK_DataTypeInteger;
static long Data_type_greyscale			= ImgK_DataTypeInteger;
static long Data_type_multispect		= ImgK_DataTypeInteger;
static long Data_type_private			= ImgK_DataTypeInteger;
static long Grid_type				= ImgK_RectangularGrid;
#if defined(NEW_CDA_SYMBOLS)
static long Frm_box_ll_x_c			= DDIF_K_VALUE_CONSTANT;
static long Frm_box_ll_y_c			= DDIF_K_VALUE_CONSTANT;
static long Frm_box_ur_x_c			= DDIF_K_VALUE_CONSTANT;
static long Frm_box_ur_y_c			= DDIF_K_VALUE_CONSTANT;
static long Frm_position_c			= DDIF_K_FRAME_FIXED;
static long Frmfxd_position_x_c			= DDIF_K_VALUE_CONSTANT;
static long Frmfxd_position_y_c			= DDIF_K_VALUE_CONSTANT;
#else
static long Frm_box_ll_x_c			= DDIF$K_VALUE_CONSTANT;
static long Frm_box_ll_y_c			= DDIF$K_VALUE_CONSTANT;
static long Frm_box_ur_x_c			= DDIF$K_VALUE_CONSTANT;
static long Frm_box_ur_y_c			= DDIF$K_VALUE_CONSTANT;
static long Frm_position_c			= DDIF$K_FRAME_FIXED;
static long Frmfxd_position_x_c			= DDIF$K_VALUE_CONSTANT;
static long Frmfxd_position_y_c			= DDIF$K_VALUE_CONSTANT;
#endif
static long Frm_box_ll_x			= 0;
static long Frm_box_ll_y			= 0;
static long Frm_box_ur_x			= 0;
static long Frm_box_ur_y			= 0;
static long Frmfxd_position_x			= 0;
static long Frmfxd_position_y			= 0;
static long Line_progression			= ImgK_TopToBottom;
static long Lp_pixel_dist			= 1;
static long Number_of_comp_bitonal		= 1;
static long Number_of_comp_greyscale		= 1;
static long Number_of_comp_multispect		= 3;
static long Number_of_comp_private		= 1;
static long Pixel_alignment_bitonal		= ImgK_AlignBit;
static long Pixel_alignment_greyscale		= ImgK_AlignByte;
static long Pixel_alignment_multispect		= ImgK_AlignByte;
static long Pixel_alignment_private		= ImgK_AlignByte;
static long Pixel_group_order			= ImgK_StandardPixelOrder;
static long Pixel_group_size			= 1;
static long Pixel_path				= ImgK_LeftToRight;
static long Pixel_stride_bitonal		= 1;
static long Pixel_stride_greyscale		= 8;
static long Pixel_stride_multispect		= 8;
static long Pixel_stride_private		= 8;
static long Plane_bits_per_pixel_bitonal	= 1;
static long Plane_bits_per_pixel_greyscale	= 8;
static long Plane_bits_per_pixel_multispect	= 8;
static long Plane_bits_per_pixel_private	= 8;
static long Plane_signif			= ImgK_LsbitFirst;
static long Planes_per_pixel_bitonal		= 1;
static long Planes_per_pixel_greyscale		= 1;
static long Planes_per_pixel_multispect		= 3;
static long Planes_per_pixel_private		= 1;
static long Pp_pixel_dist			= 1;
static long Quant_levels_per_comp_bitonal	= 2;
static long Quant_levels_per_comp_greyscale	= 256;
static long Quant_levels_per_comp_multispect[3]	= { 256, 256, 256 };
static long Quant_levels_per_comp_private	= 256;
static long Scanline_alignment_bitonal		= ImgK_AlignByte;
static long Scanline_alignment_greyscale	= ImgK_AlignByte;
static long Scanline_alignment_multispect	= ImgK_AlignByte;
static long Scanline_alignment_private		= ImgK_AlignByte;
static long Spectral_mapping_bitonal		= ImgK_MonochromeMap;
static long Spectral_mapping_greyscale		= ImgK_MonochromeMap;
static long Spectral_mapping_multispect		= ImgK_RGBMap;
static long Spectral_mapping_private		= ImgK_PrivateMap;

static struct ITMLST General_defaults[] = {
     { Img_GridType,		LONGSIZE, (char*)&Grid_type,		0, 0 }
    ,{ Img_FrmBoxLLX,		LONGSIZE, (char*)&Frm_box_ll_x,		0, 0 }
    ,{ Img_FrmBoxLLXC,		LONGSIZE, (char*)&Frm_box_ll_x_c,	0, 0 } 
    ,{ Img_FrmBoxLLY,		LONGSIZE, (char*)&Frm_box_ll_y,		0, 0 }
    ,{ Img_FrmBoxLLYC,		LONGSIZE, (char*)&Frm_box_ll_y_c,	0, 0 }
    ,{ Img_FrmBoxURX,		LONGSIZE, (char*)&Frm_box_ur_x,		0, 0 }
    ,{ Img_FrmBoxURXC,		LONGSIZE, (char*)&Frm_box_ur_x_c,	0, 0 }
    ,{ Img_FrmBoxURY,		LONGSIZE, (char*)&Frm_box_ur_y,		0, 0 }
    ,{ Img_FrmBoxURYC,		LONGSIZE, (char*)&Frm_box_ur_y_c,	0, 0 }
    ,{ Img_FrmPositionC,	LONGSIZE, (char*)&Frm_position_c,	0, 0 }
    ,{ Img_FrmFxdPositionX,	LONGSIZE, (char*)&Frmfxd_position_x,	0, 0 }
    ,{ Img_FrmFxdPositionXC,	LONGSIZE, (char*)&Frmfxd_position_x_c,	0, 0 }
    ,{ Img_FrmFxdPositionY,	LONGSIZE, (char*)&Frmfxd_position_y,	0, 0 }
    ,{ Img_FrmFxdPositionYC,	LONGSIZE, (char*)&Frmfxd_position_y_c,	0, 0 }
    ,{ Img_LineProgression,	LONGSIZE, (char*)&Line_progression,	0, 0 }
    ,{ Img_LPPixelDist,		LONGSIZE, (char*)&Lp_pixel_dist,	0, 0 }
    ,{ Img_PixelGroupOrder,	LONGSIZE, (char*)&Pixel_group_order,	0, 0 }
    ,{ Img_PixelGroupSize,	LONGSIZE, (char*)&Pixel_group_size,	0, 0 }
    ,{ Img_PixelPath,		LONGSIZE, (char*)&Pixel_path,		0, 0 }
    ,{ Img_PlaneSignif,		LONGSIZE, (char*)&Plane_signif,		0, 0 }
    ,{ Img_PPPixelDist,		LONGSIZE, (char*)&Pp_pixel_dist,	0, 0 }
    ,{ 0,			0,	  0,				0, 0 }
    };

static struct ITMLST Bitonal_defaults[] = {
     { Img_BitOrder,		LONGSIZE, (char*)&Bit_order,		    0,0}
    ,{ Img_BitsPerComp,		LONGSIZE, (char*)&Bits_per_comp_bitonal,    0,0}
    ,{ Img_BrtPolarity,		LONGSIZE, (char*)&Brt_polarity_bitonal,	    0,0}
    ,{ Img_ByteOrder,		LONGSIZE, (char*)&Byte_order,		    0,0}
    ,{ Img_ByteUnit,		LONGSIZE, (char*)&Byte_unit,		    0,0}
    ,{ Img_CompressionType,	LONGSIZE, (char*)&Compression_type,	    0,0}
    ,{ Img_CompSpaceOrg,	LONGSIZE, (char*)&Comp_space_org_bitonal,   0,0}
    ,{ Img_DataOffset,		LONGSIZE, (char*)&Data_offset,		    0,0}
    ,{ Img_DataType,		LONGSIZE, (char*)&Data_type_bitonal,	    0,0}
    ,{ Img_NumberOfComp,	LONGSIZE, (char*)&Number_of_comp_bitonal,   0,0}
    ,{ Img_PixelStride,		LONGSIZE, (char*)&Pixel_stride_bitonal,	    0,0}
    ,{ Img_PlaneBitsPerPixel,	LONGSIZE, (char*)&Plane_bits_per_pixel_bitonal,
									    0,0}
/*    ,{ Img_PlanesPerPixel,	LONGSIZE, (char*)&Planes_per_pixel_bitonal, 0,0}
*/
    ,{ Img_QuantLevelsPerComp,	LONGSIZE, (char*)&Quant_levels_per_comp_bitonal,
									    0,0}
    ,{ Img_SpectralMapping,	LONGSIZE, (char*)&Spectral_mapping_bitonal, 0,0}
    ,{ 0,			0,	  0,				    0,0}
    };

static struct ITMLST Greyscale_defaults[] = {
     { Img_BitOrder,		LONGSIZE, (char*)&Bit_order,		    0,0}
    ,{ Img_BitsPerComp,		LONGSIZE, (char*)&Bits_per_comp_greyscale,  0,0}
    ,{ Img_BrtPolarity,		LONGSIZE, (char*)&Brt_polarity_greyscale,   0,0}
    ,{ Img_ByteOrder,		LONGSIZE, (char*)&Byte_order,		    0,0}
    ,{ Img_ByteUnit,		LONGSIZE, (char*)&Byte_unit,		    0,0}
    ,{ Img_CompressionType,	LONGSIZE, (char*)&Compression_type,	    0,0}
    ,{ Img_CompSpaceOrg,	LONGSIZE, (char*)&Comp_space_org_greyscale, 0,0}
    ,{ Img_DataOffset,		LONGSIZE, (char*)&Data_offset,		    0,0}
    ,{ Img_DataType,		LONGSIZE, (char*)&Data_type_greyscale,	    0,0}
    ,{ Img_NumberOfComp,	LONGSIZE, (char*)&Number_of_comp_greyscale, 0,0}
    ,{ Img_PixelStride,		LONGSIZE, (char*)&Pixel_stride_greyscale,   0,0}
    ,{ Img_PlaneBitsPerPixel,	LONGSIZE, 
				    (char*)&Plane_bits_per_pixel_greyscale, 0,0}
/*    ,{ Img_PlanesPerPixel,	LONGSIZE, 
**				    (char*)&Planes_per_pixel_greyscale,	    0,0}
*/
    ,{ Img_QuantLevelsPerComp,	LONGSIZE, 
				    (char*)&Quant_levels_per_comp_greyscale,0,0}
    ,{ Img_SpectralMapping,	LONGSIZE, 
				    (char*)&Spectral_mapping_greyscale,	    0,0}
    ,{ 0,			0,	      0,			    0,0}
    };

static struct ITMLST Multispect_defaults[] = {
     { Img_BitOrder,		LONGSIZE, (char*)&Bit_order,		    0,0}
    ,{ Img_BitOrder,		LONGSIZE, (char*)&Bit_order,		    0,1}
    ,{ Img_BitOrder,		LONGSIZE, (char*)&Bit_order,		    0,2}
    ,{ Img_BitsPerComp,		LONGSIZE, 
				    (char*)&Bits_per_comp_multispect[0],    0,0}
    ,{ Img_BitsPerComp,		LONGSIZE, 
				    (char*)&Bits_per_comp_multispect[1],    0,1}
    ,{ Img_BitsPerComp,		LONGSIZE, 
				    (char*)&Bits_per_comp_multispect[2],    0,2}
    ,{ Img_BrtPolarity,		LONGSIZE, (char*)&Brt_polarity_multispect,  0,0}
    ,{ Img_ByteOrder,		LONGSIZE, (char*)&Byte_order,		    0,0}
    ,{ Img_ByteOrder,		LONGSIZE, (char*)&Byte_order,		    0,1}
    ,{ Img_ByteOrder,		LONGSIZE, (char*)&Byte_order,		    0,2}
    ,{ Img_ByteUnit,		LONGSIZE, (char*)&Byte_unit,		    0,0}
    ,{ Img_ByteUnit,		LONGSIZE, (char*)&Byte_unit,		    0,1}
    ,{ Img_ByteUnit,		LONGSIZE, (char*)&Byte_unit,		    0,2}
    ,{ Img_CompressionType,	LONGSIZE, (char*)&Compression_type,	    0,0}
    ,{ Img_CompressionType,	LONGSIZE, (char*)&Compression_type,	    0,1}
    ,{ Img_CompressionType,	LONGSIZE, (char*)&Compression_type,	    0,2}
    ,{ Img_CompSpaceOrg,	LONGSIZE, (char*)&Comp_space_org_multispect,0,0}
    ,{ Img_DataOffset,		LONGSIZE, (char*)&Data_offset,		    0,0}
    ,{ Img_DataOffset,		LONGSIZE, (char*)&Data_offset,		    0,1}
    ,{ Img_DataOffset,		LONGSIZE, (char*)&Data_offset,		    0,2}
    ,{ Img_DataType,		LONGSIZE, (char*)&Data_type_multispect,	    0,0}
    ,{ Img_DataType,		LONGSIZE, (char*)&Data_type_multispect,	    0,1}
    ,{ Img_DataType,		LONGSIZE, (char*)&Data_type_multispect,	    0,2}
    ,{ Img_NumberOfComp,	LONGSIZE, (char*)&Number_of_comp_multispect,0,0}
    ,{ Img_PixelStride,		LONGSIZE, (char*)&Pixel_stride_multispect,  0,0}
    ,{ Img_PixelStride,		LONGSIZE, (char*)&Pixel_stride_multispect,  0,1}
    ,{ Img_PixelStride,		LONGSIZE, (char*)&Pixel_stride_multispect,  0,2}
    ,{ Img_PlaneBitsPerPixel,	LONGSIZE, 
				    (char*)&Plane_bits_per_pixel_multispect,0,0}
/*    ,{ Img_PlanesPerPixel,	LONGSIZE, 
**				    (char*)&Planes_per_pixel_multispect,    0,0}
*/
    ,{ Img_PlaneBitsPerPixel,	LONGSIZE, 
				    (char*)&Plane_bits_per_pixel_multispect,0,1}
    ,{ Img_PlaneBitsPerPixel,	LONGSIZE, 
				    (char*)&Plane_bits_per_pixel_multispect,0,2}
    ,{ Img_QuantLevelsPerComp,	LONGSIZE, 
				    (char*)&Quant_levels_per_comp_multispect[0],
									    0,0}
    ,{ Img_QuantLevelsPerComp,	LONGSIZE, 
				    (char*)&Quant_levels_per_comp_multispect[1],
									    0,1}
    ,{ Img_QuantLevelsPerComp,	LONGSIZE, 
				    (char*)&Quant_levels_per_comp_multispect[2],
									    0,2}
    ,{ Img_SpectralMapping,	LONGSIZE, (char*)&Spectral_mapping_multispect, 
									    0,0}
    ,{ 0,			0,	  0,				    0,0}
    };

static struct ITMLST Private_defaults[] = {
     { Img_BrtPolarity,		LONGSIZE, (char*)&Brt_polarity_private,	    0,0}
    ,{ Img_NumberOfComp,	LONGSIZE, (char*)&Number_of_comp_private,   0,0}
    ,{ Img_SpectralMapping,	LONGSIZE, (char*)&Spectral_mapping_private, 0,0}
    ,{ 0,			0,	      0,			    0,0}
    };

static struct ITMLST Std_bitonal_attrs[] = {
     { Img_ByteOrder,		LONGSIZE, (char*)&Byte_order,		    0,0}
    ,{ Img_BitOrder,		LONGSIZE, (char*)&Bit_order,		    0,0}
    ,{ Img_CompSpaceOrg,	LONGSIZE, (char*)&Comp_space_org_bitonal,   0,0}
    ,{ Img_PixelAlignment,	LONGSIZE, (char*)&Pixel_alignment_bitonal,  0,0}
    ,{ Img_ScanlineAlignment,	LONGSIZE, (char*)&Scanline_alignment_bitonal,
									    0,0}
    ,{ Img_DataType,		LONGSIZE, (char*)&Data_type_bitonal,	    0,0}
    ,{ Img_GridType,		LONGSIZE, (char*)&Grid_type,		    0,0}
    ,{ Img_LineProgression,	LONGSIZE, (char*)&Line_progression,	    0,0}
    ,{ Img_PixelPath,		LONGSIZE, (char*)&Pixel_path,		    0,0}
    ,{ Img_PixelGroupOrder,	LONGSIZE, (char*)&Pixel_group_order,	    0,0}
    ,{ 0,			0,	  0,				    0,0}
    };

static struct ITMLST Std_greyscale_attrs[] = {
     { Img_ByteOrder,		LONGSIZE, (char*)&Byte_order,		    0,0}
    ,{ Img_BitOrder,		LONGSIZE, (char*)&Bit_order,		    0,0}
    ,{ Img_CompSpaceOrg,	LONGSIZE, (char*)&Comp_space_org_greyscale, 0,0}
    ,{ Img_PlaneSignif,		LONGSIZE, (char*)&Plane_signif,		    0,0}
    ,{ Img_PixelAlignment,	LONGSIZE, (char*)&Pixel_alignment_greyscale,0,0}
    ,{ Img_ScanlineAlignment,	LONGSIZE, (char*)&Scanline_alignment_greyscale,
									    0,0}
    ,{ Img_DataType,		LONGSIZE, (char*)&Data_type_greyscale,	    0,0}
    ,{ Img_GridType,		LONGSIZE, (char*)&Grid_type,		    0,0}
    ,{ Img_LineProgression,	LONGSIZE, (char*)&Line_progression,	    0,0}
    ,{ Img_PixelPath,		LONGSIZE, (char*)&Pixel_path,		    0,0}
    ,{ Img_PixelGroupOrder,	LONGSIZE, (char*)&Pixel_group_order,	    0,0}
    ,{ 0,			0,	  0,				    0,0}
    };

static struct ITMLST Std_multispectral_attrs[] = {
     { Img_ByteOrder,		LONGSIZE, (char*)&Byte_order,		    0,0}
    ,{ Img_BitOrder,		LONGSIZE, (char*)&Bit_order,		    0,0}
    ,{ Img_CompSpaceOrg,	LONGSIZE, (char*)&Comp_space_org_multispect,0,0}
    ,{ Img_PlaneSignif,		LONGSIZE, (char*)&Plane_signif,		    0,0}
    ,{ Img_PixelAlignment,	LONGSIZE, (char*)&Pixel_alignment_multispect,
									    0,0}
    ,{ Img_PixelAlignment,	LONGSIZE, (char*)&Pixel_alignment_multispect,
									    0,1}
    ,{ Img_PixelAlignment,	LONGSIZE, (char*)&Pixel_alignment_multispect,
									    0,2}
    ,{ Img_ScanlineAlignment,	LONGSIZE, (char*)&Scanline_alignment_multispect,
									    0,0}
    ,{ Img_ScanlineAlignment,	LONGSIZE, (char*)&Scanline_alignment_multispect,
									    0,1}
    ,{ Img_ScanlineAlignment,	LONGSIZE, (char*)&Scanline_alignment_multispect,
									    0,2}
    ,{ Img_DataType,		LONGSIZE, (char*)&Data_type_multispect,	    0,0}
    ,{ Img_GridType,		LONGSIZE, (char*)&Grid_type,		    0,0}
    ,{ Img_LineProgression,	LONGSIZE, (char*)&Line_progression,	    0,0}
    ,{ Img_PixelPath,		LONGSIZE, (char*)&Pixel_path,		    0,0}
    ,{ Img_PixelGroupOrder,	LONGSIZE, (char*)&Pixel_group_order,	    0,0}
    ,{ 0,			0,	  0,				    0,0}
    };

/******************************************************************************
**  _ImgAdjustFat
**
**  FUNCTIONAL DESCRIPTION:
**
**	Adjust frame attributes according to new attribute values specified
**	in the input item list.
**
**	This function not only established new values for attributes that
**	are specified, but also automatically adjusts the values of other
**	attributes that may be affected by the setting of a single attribute.
**
**	The auto-adjust feature helps eliminate the need to respecify
**	many attributes when just a few will do, and help eliminate
**	inconsistencies that arise when one attribute is changed but not
**	another (related one).
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
struct FAT *_ImgAdjustFat( fat_in, itmlst, flags )
struct FAT	    *fat_in;
struct ITMLST	    *itmlst;
unsigned long	     flags;
{
long	    bits_per_comp;
long	    bpc_item_occurances;
long	    comp_space_org;
long	    component_cnt;
long	    found_status;
long	    ice_cnt			= 1;
long	    ice_idx;
long	    ici_idx;
long	    idu_cnt;
long	    idu_idx;
long	    index			= 0;
long	    item_index;
long	    item_value;
long	    length			= 0;
long	    number_of_comp		= 0;
long	    number_of_lines;
long	    occurance			= 1;
long	    old_bits_per_comp;
long	    pixels_per_line;
long	    plane_cnt;
long	    quant_levels_per_comp;
long	    spectral_mapping		= -1;
long	    total_plane_bits_per_pixel;

struct FAT	*fat_out;
struct ICE	*ice_in;
struct ICE	*ice_out;
struct ICI	*ici_in;
struct ICI	*ici_out;
struct IDU	*idu_in;
struct IDU	*idu_out;
struct ICE	*tmp_ice_in;
struct ICE	*tmp_ice_out;
struct IDU	*tmp_idu_in;
struct IDU	*tmp_idu_out;
struct ITMLST	*list_item	= itmlst;


/*
** Allocate the output fat, and initialize it to the same values as the
** input fat.  Also allocate ICIs. (Wait until later to attach ICEs and IDUs.)
*/
fat_out = (struct FAT *) _ImgCalloc( 1, FatK_Size );
*fat_out = *fat_in;
Copy_user_label_strs( fat_in, fat_out );

/*
** Look for and process other items in the list that may have an effect
** on the number of components and number of planes that will be required
** by the output fat.
*/
if ( _ImgItmlstItemCount( Img_NumberOfComp, itmlst ) != 0 )


    _ImgExtractItmlstItem( itmlst, Img_NumberOfComp, &number_of_comp,
			&length, 0, &index, occurance );
else
    number_of_comp = fat_in->FatL_NumberOfComp;

/*
** Get spectral mapping and modify number of comps if it changes
*/
if ( _ImgItmlstItemCount( Img_SpectralMapping, itmlst ) != 0 )
    {
    _ImgExtractItmlstItem( itmlst, Img_SpectralMapping, &spectral_mapping,
			    &length, 0, &index, occurance );

    switch ( spectral_mapping )
	{
	case ImgK_MonochromeMap:
	    number_of_comp = 1;
	    break;
	case ImgK_RGBMap:
	case ImgK_CMYMap:
	case ImgK_YUVMap:
	case ImgK_HSVMap:
	case ImgK_HLSMap:
	case ImgK_YIQMap:
	    number_of_comp = 3;
	    break;
	default:
	    break;
	} /* end switch */
    } /* end if then */
else
    spectral_mapping = fat_in->FatL_SpectralMapping;    

/*
** Peek ahead to see what the component space org is going to be.
** If it's not being changed, find out what it is on input.
*/
found_status = _ImgExtractItmlstItem( itmlst, Img_CompSpaceOrg, &comp_space_org,
					&length, 0, &index, occurance );
if ( !found_status )
    comp_space_org = fat_in->FatL_CompSpaceOrg;

/*
** Allocate Image Component Info data areas.  If the number of components
** on output is the same as the number on input, copy the input to the
** output.  (Otherwise, assume that the application will fill them in.)
*/
fat_out->FatR_Ici = (struct ICI *) _ImgCalloc( number_of_comp, IciK_Size );
fat_out->FatL_NumberOfComp = number_of_comp;

if ( fat_out->FatL_NumberOfComp == fat_in->FatL_NumberOfComp )
    {
    ici_in = fat_in->FatR_Ici;
    ici_out = fat_out->FatR_Ici;
    fat_out->FatL_TotalBitsPerPixel = 0;
    fat_out->FatL_TotalQuantBitsPerPixel = 0;
    for ( ici_idx = 0; ici_idx < fat_out->FatL_NumberOfComp; ++ici_idx )
	{
	switch ( comp_space_org )
	    {
	    /*
	    ** Promote the bits per comp value to the nearest
	    ** addressable memory size if the output component
	    ** space org is Band Interleaved by Plane (when image 
	    ** is continous tone).
	    */
	    case ImgK_BandIntrlvdByPlane:
		if ( fat_out->FatL_NumberOfComp == 1 && 
		     ici_in->IciL_BitsPerComp == 1 )
		    ici_out->IciL_BitsPerComp = 1;
		else
		    ici_out->IciL_BitsPerComp = STANDARDIZE_BPC_VALUE_(
						ici_in->IciL_BitsPerComp );

		ici_out->IciL_QuantLevelsPerComp =
						ici_in->IciL_QuantLevelsPerComp;
		ici_out->IciL_QuantBitsPerComp = ici_in->IciL_QuantBitsPerComp;
		break;
	    /*
	    ** Minimize the bits per component for all other organizations.
	    */
	    case ImgK_BandIntrlvdByPixel:
	    case ImgK_BitIntrlvdByPlane:
	    case ImgK_BandIntrlvdByLine:
	    default:
		occurance = ici_idx + 1;
		/*
		** Are quant bits per comp supplied in the itmlst? ...
		*/
		if ( _ImgExtractItmlstItem( itmlst, Img_QuantLevelsPerComp,
			    &quant_levels_per_comp, &length, 0, &index, 
			    occurance ) )
		    {
		    ici_out->IciL_QuantLevelsPerComp = quant_levels_per_comp;
		    ici_out->IciL_BitsPerComp = 
			    _ImgConvertLevelsToBits( quant_levels_per_comp );
		    ici_out->IciL_QuantBitsPerComp = ici_out->IciL_BitsPerComp;
		    }
		/*
		** No?  How about bits per comp? ... (infer levels from bits)
		*/
		else if ( _ImgExtractItmlstItem( itmlst, Img_BitsPerComp,
			    &bits_per_comp, &length, 0, &index, 
			    occurance ) )
		    {
		    ici_out->IciL_BitsPerComp = bits_per_comp;
		    ici_out->IciL_QuantLevelsPerComp = 1 << bits_per_comp;
		    ici_out->IciL_QuantBitsPerComp = ici_out->IciL_BitsPerComp;
		    }
		/*
		** Still no?  Can source quant levels per comp be used? ....
		*/
		else if ( ici_in->IciL_QuantLevelsPerComp != 0 )
		    {
		    ici_out->IciL_QuantLevelsPerComp = 
						ici_in->IciL_QuantLevelsPerComp;
		    ici_out->IciL_QuantBitsPerComp = _ImgConvertLevelsToBits(
					    ici_out->IciL_QuantLevelsPerComp );

		    /*
		    ** Set bits per comp.  Note, special rules apply.
		    **
		    ** If both input and output images are band-by-pixel
		    ** and the input bits per comp are known, use the
		    ** input bits per comp as is.
		    **
		    ** If we are adjusting to band-by-pixel from some other
		    ** org, then set bits per comp to be the minimum
		    ** (which implies that their is no padding).
		    */
		    if ( ici_in->IciL_BitsPerComp != 0 && 
			 comp_space_org == ImgK_BandIntrlvdByPixel &&
			 comp_space_org == fat_in->FatL_CompSpaceOrg )
			    ici_out->IciL_BitsPerComp = 
						    ici_in->IciL_BitsPerComp;
		    else
			ici_out->IciL_BitsPerComp = 
						ici_out->IciL_QuantBitsPerComp;
		    }
		/*
		** If all else fails, use the bits per comp from the
		** source Ici (as is).
		*/
		else
		    {
		    ici_out->IciL_BitsPerComp = ici_in->IciL_BitsPerComp;
		    ici_out->IciL_QuantLevelsPerComp = 
						ici_in->IciL_QuantLevelsPerComp;
		    }
		break;
	    } /* end switch */

	fat_out->FatL_TotalBitsPerPixel += ici_out->IciL_BitsPerComp;
	fat_out->FatL_TotalQuantBitsPerPixel += ici_out->IciL_QuantBitsPerComp;
	++ici_in;
	++ici_out;
	}
    }


/*
** Search the item list for other items that may have an effect on
** the number of planes to attach to the output fat.
*/
for ( ; list_item->ItmL_Code != 0; ++list_item )
    {
    if ( list_item->ItmA_Buffer != 0 )
	item_value = *((long *)(list_item->ItmA_Buffer));
    else
	continue;   /* jump back to top of loop	*/

    switch ( list_item->ItmL_Code )
	{
	case Img_CompSpaceOrg:
	    switch ( item_value )
		{
		case ImgK_BandIntrlvdByPixel:
		    fat_out->FatL_CompSpaceOrg = item_value;
		    fat_out->FatL_PlanesPerPixel = 1;
		    break;
		case ImgK_BandIntrlvdByPlane:
		    fat_out->FatL_CompSpaceOrg = item_value;
		    fat_out->FatL_PlanesPerPixel = 
					    fat_out->FatL_NumberOfComp;
		    fat_out->FatL_PlaneSignif = ImgK_LsbitFirst;
		    break;
		case ImgK_BitIntrlvdByPlane:
		    fat_out->FatL_CompSpaceOrg = item_value;
		    fat_out->FatL_PlanesPerPixel = 
					    fat_out->FatL_TotalBitsPerPixel;
		    fat_out->FatL_PlaneSignif = ImgK_LsbitFirst;
		    break;
		case ImgK_BandIntrlvdByLine:
		    fat_out->FatL_CompSpaceOrg = item_value;
		    fat_out->FatL_PlanesPerPixel = 1;
		default:
		    break;
		} /* end switch */
	    break;
	case Img_SpectralMapping:
	    switch ( *((long *)(list_item->ItmA_Buffer)) )
		{
		case ImgK_MonochromeMap:
		    fat_out->FatL_SpectralMapping = item_value;
		    fat_out->FatL_NumberOfComp = 1;
		    switch( fat_out->FatL_CompSpaceOrg )
			{
			case ImgK_BandIntrlvdByPixel:
			case ImgK_BandIntrlvdByPlane:
			case ImgK_BandIntrlvdByLine:
			    fat_out->FatL_PlanesPerPixel = 1;
			    break;
			case ImgK_BitIntrlvdByPlane:
			    fat_out->FatL_PlanesPerPixel = 
				fat_out->FatL_TotalBitsPerPixel;
			default:
			    break;
			} /* end switch */
		    break;
		case ImgK_RGBMap:
		case ImgK_CMYMap:
		case ImgK_YUVMap:
		case ImgK_HSVMap:
		case ImgK_HLSMap:
		case ImgK_YIQMap:
		    fat_out->FatL_SpectralMapping = item_value;
		    fat_out->FatL_NumberOfComp = 3;
		    switch( fat_out->FatL_CompSpaceOrg )
			{
			case ImgK_BandIntrlvdByPixel:
			case ImgK_BandIntrlvdByLine:
			    fat_out->FatL_PlanesPerPixel = 1;
			    break;
			case ImgK_BandIntrlvdByPlane:
			    fat_out->FatL_PlanesPerPixel = 3;
			    break;
			case ImgK_BitIntrlvdByPlane:
			    fat_out->FatL_PlanesPerPixel = 
				fat_out->FatL_TotalBitsPerPixel;
			default:
			    break;
			} /* end switch */
		default:
		    break;
		} /* end switch */
	    break;

	case Img_PlanesPerPixel:
	    fat_out->FatL_PlanesPerPixel = item_value;
	    break;
	case Img_TotalBitsPerPixel:
	    fat_out->FatL_TotalBitsPerPixel = item_value;
	    break;

	case Img_QuantLevelsPerComp:
	    {
	    long    quant_levels    = item_value;

	    /*
	    ** Verify the item index.
	    */
	    if ( list_item->ItmL_Index >= fat_out->FatL_NumberOfComp )
		ChfSignal( 4, ImgX_INVATRIDX, 2, list_item->ItmL_Code,
				list_item->ItmL_Index );

	    ici_out = &(fat_out->FatR_Ici[list_item->ItmL_Index]);
	    ici_out->IciL_QuantLevelsPerComp = quant_levels;

	    /*
	    ** If the quant levels for this component increase, the number
	    ** of bits per comp may also increase.  Therefore, figure out
	    ** the increase and automatically adjust the number of bits
	    ** per comp.  Do nothing if the number of levels decreases.
	    */
	    if ( quant_levels > (1 << ici_out->IciL_BitsPerComp) )
		{
		bits_per_comp = _ImgConvertLevelsToBits( quant_levels );
		old_bits_per_comp = ici_out->IciL_BitsPerComp;
		ici_out->IciL_QuantBitsPerComp = bits_per_comp;

		/*
		** Make sure the actual number of bits per component is
		** a multiple of 8 if quant_bits_per_comp is greater than
		** 1 (for bitonal images).
		*/
		if ( bits_per_comp > 1 )
		    bits_per_comp = ((bits_per_comp + 7)/8) * 8;

		ici_out->IciL_BitsPerComp = bits_per_comp;

		/*
		** Now adjust those attributes that depend on the number
		** of bits for this particular number of components
		*/
		fat_out->FatL_TotalBitsPerPixel += (bits_per_comp -
							old_bits_per_comp);
		} /* end if */

	    break;
	    } /* end case */

	case Img_BitsPerComp:
	    /*
	    ** Verify the item index.
	    */
	    if ( list_item->ItmL_Index >= fat_out->FatL_NumberOfComp )
		ChfSignal( 4, ImgX_INVATRIDX, 2, list_item->ItmL_Code,
				list_item->ItmL_Index );

	    ici_out = &(fat_out->FatR_Ici[list_item->ItmL_Index]);

	    old_bits_per_comp = ici_out->IciL_BitsPerComp;
	    bits_per_comp = item_value;
	    ici_out->IciL_BitsPerComp = bits_per_comp;

	    /*
	    ** Now adjust those attributes that depend on the number
	    ** of bits for this particular number of components
	    */
	    fat_out->FatL_TotalBitsPerPixel += (bits_per_comp -
							old_bits_per_comp);
	    break;

	default:
	    break;
	} /* end switch */
    } /* end for */


/*
** Allocate and attach ICEs to the fat and IDUs to the ICEs.
*/
switch( fat_out->FatL_CompSpaceOrg )
    {
    case ImgK_BandIntrlvdByPixel:
	plane_cnt = 1;
        break;
    case ImgK_BandIntrlvdByPlane:
        plane_cnt = fat_out->FatL_NumberOfComp;
        break;
    case ImgK_BitIntrlvdByPlane:
        plane_cnt = fat_out->FatL_TotalBitsPerPixel;
        break;
    case ImgK_BandIntrlvdByLine:
        plane_cnt = 1;
    default:
        break;
    }

fat_out->FatR_Ice = (struct ICE *) _ImgCalloc( fat_out->FatL_IceCnt, 
						IceK_Size );
ice_out = fat_out->FatR_Ice;

for ( ice_idx = 0; ice_idx < fat_out->FatL_IceCnt; ++ice_idx )
    {
    ice_out->IceL_IduCnt = plane_cnt;
    ice_out->IceR_Idu = (struct IDU *) _ImgCalloc( plane_cnt, IduK_Size );
    }


/*
** Figure out what the coding attributes in the IDU should be, depending
** on the component space organization.  Use the first ICE and the first
** IDU from the input fat as a basic reference.
*/
switch ( fat_out->FatL_CompSpaceOrg )
    {
    case ImgK_BandIntrlvdByPixel:
	switch ( fat_in->FatL_CompSpaceOrg )
	    {
	    /*
	    ** Since the organization has changed, use the first input
	    ** IDU as a partial template for the output IDU.
	    */
	    case ImgK_BandIntrlvdByPlane:
	    case ImgK_BitIntrlvdByPlane:
	    case ImgK_BandIntrlvdByLine:
		ice_in = fat_in->FatR_Ice;
		ice_out = fat_out->FatR_Ice;
		for ( ice_idx = 0; ice_idx < fat_out->FatL_IceCnt; ++ice_idx )
		    {
		    idu_out = ice_out->IceR_Idu;
		    idu_in = ice_in->IceR_Idu;

		    idu_out->IduL_BitOrder	    = idu_in->IduL_BitOrder;
		    idu_out->IduL_ByteOrder	    = idu_in->IduL_ByteOrder;
		    idu_out->IduL_ByteUnit	    = idu_in->IduL_ByteUnit;
		    idu_out->IduL_CompressionType   = ImgK_PcmCompression;
		    idu_out->IduL_DataOffset	    = 0;
		    idu_out->IduL_DataType	    = idu_in->IduL_DataType;
		    idu_out->IduL_Dtype		    = idu_in->IduL_Dtype;
		    idu_out->IduL_NumberOfLines	    = 
						    idu_in->IduL_NumberOfLines;

		    idu_out->IduL_PixelStride	    = 
						fat_out->FatL_TotalBitsPerPixel;

		    idu_out->IduL_PixelsPerLine	    = 
						    idu_in->IduL_PixelsPerLine;

		    idu_out->IduL_PlaneBitsPerPixel =
						fat_out->FatL_TotalBitsPerPixel;
		    idu_out->IduL_ScanlineStride    = 
						    idu_out->IduL_PixelStride * 
						    idu_in->IduL_PixelsPerLine;
		    ++ice_out;
		    ++ice_in;
		    } /* end for */
		break;
	    /*
	    ** Since the organization hasn't changed, copy the input IDUs
	    ** into the output IDUs.  (NOTE that there will always only
	    ** be one IDU per ICE for this organization.)
	    */
	    case ImgK_BandIntrlvdByPixel:
		ice_in = fat_in->FatR_Ice;
		ice_out = fat_out->FatR_Ice;
		for ( ice_idx = 0; ice_idx < fat_out->FatL_IceCnt; ++ice_idx )
		    {
		    idu_in = ice_in->IceR_Idu;
		    idu_out = ice_out->IceR_Idu;
		    *idu_out = *idu_in;

		    ++ice_in;
		    ++ice_out;
		    } /* end for */
		break;
	    } /* end switch */
	break;
    case ImgK_BandIntrlvdByPlane:
	{
	switch ( fat_in->FatL_CompSpaceOrg )
	    {
	    /*
	    ** Loop through the ICEs and generate the new UDPs for
	    ** each ICE.  Since the organization hasn't changed, just
	    ** copy the input IDUs into the output IDUs.  
	    */
	    case ImgK_BandIntrlvdByPlane:
		ice_in  = fat_in->FatR_Ice;
		ice_out = fat_out->FatR_Ice;
		ice_cnt = fat_out->FatL_IceCnt;
		for ( ice_idx = 0; ice_idx < ice_cnt; ++ice_idx )
		    {
		    idu_idx = 0;
		    idu_cnt = ice_out->IceL_IduCnt;
		    idu_in  = ice_in->IceR_Idu;
		    idu_out = ice_out->IceR_Idu;
		    for ( ; idu_idx < idu_cnt; ++idu_idx )
			{
			*idu_out = *idu_in;
			++idu_in;
			++idu_out;
			} /* end for */

		    ++ice_in;
		    ++ice_out;
		    } /* end for */

		break;
	    /*
	    ** Loop through the ICEs and generate the new UDPs for
	    ** each ICE.  Since this is a multiplane organization,
	    ** use the src IDU of the first component as a partial
	    ** template for each output IDU.
	    */
	    case ImgK_BandIntrlvdByPixel:
	    case ImgK_BitIntrlvdByPlane:
	    case ImgK_BandIntrlvdByLine:
	    default:
		ice_in = fat_in->FatR_Ice;
		ice_out = fat_out->FatR_Ice;
		ice_cnt = fat_out->FatL_IceCnt;
		ici_out = fat_out->FatR_Ici;
		ice_idx = 0;
		for ( ; ice_idx < ice_cnt; ++ice_idx, ++ice_in, ++ice_out )
		    {
		    idu_idx = 0;
		    idu_in = ice_in->IceR_Idu;
		    idu_out = ice_out->IceR_Idu;
		    idu_cnt = ice_out->IceL_IduCnt;
		    for ( ; idu_idx < idu_cnt; ++idu_idx, ++idu_out, ++ici_out )
			{
			idu_out->IduL_BitOrder		= idu_in->IduL_BitOrder;
			idu_out->IduL_ByteOrder		= 
							idu_in->IduL_ByteOrder;
			idu_out->IduL_ByteUnit		= idu_in->IduL_ByteUnit;
			idu_out->IduL_CompressionType   = ImgK_PcmCompression;
			idu_out->IduL_DataOffset	= 0;
			idu_out->IduL_DataType		= idu_in->IduL_DataType;
			idu_out->IduL_Dtype		= idu_in->IduL_Dtype;
			idu_out->IduL_NumberOfLines	= 
						    idu_in->IduL_NumberOfLines;
			idu_out->IduL_PixelStride	= 
						    ici_out->IciL_BitsPerComp;
			idu_out->IduL_PixelsPerLine	= 
						    idu_in->IduL_PixelsPerLine;
			idu_out->IduL_PlaneBitsPerPixel =
						    idu_out->IduL_PixelStride;
			idu_out->IduL_ScanlineStride    = 
						    idu_out->IduL_PixelStride * 
						    idu_in->IduL_PixelsPerLine;

			} /* end for */
		    } /* end for */
		break;
	    } /* end switch */
	break;
	} /* end case of output ImgK_BandIntrlvdByPlane */

    case ImgK_BitIntrlvdByPlane:
	if ( fat_out->FatL_CompSpaceOrg != fat_in->FatL_CompSpaceOrg )
	    /*
	    ** Since the organization has changed, use the first input
	    ** IDU as a partial template for the output IDU.
	    */
	    {
	    ice_in = fat_in->FatR_Ice;
	    ice_out = fat_out->FatR_Ice;
	    for ( ice_idx = 0; ice_idx < fat_out->FatL_IceCnt; ++ice_idx )
		{
		idu_out = ice_out->IceR_Idu;
		idu_in = ice_in->IceR_Idu;
		for ( idu_idx = 0; idu_idx < ice_out->IceL_IduCnt; ++idu_idx )
		    {

		    idu_out->IduL_BitOrder	    = idu_in->IduL_BitOrder;
		    idu_out->IduL_ByteOrder	    = idu_in->IduL_ByteOrder;
		    idu_out->IduL_ByteUnit	    = idu_in->IduL_ByteUnit;
		    idu_out->IduL_CompressionType   = ImgK_PcmCompression;
		    idu_out->IduL_DataOffset	    = 0;
		    idu_out->IduL_DataType	    = idu_in->IduL_DataType;
		    idu_out->IduL_Dtype		    = idu_in->IduL_Dtype;
		    idu_out->IduL_NumberOfLines	    = 
						    idu_in->IduL_NumberOfLines;
		    idu_out->IduL_PixelStride	    = 1;
		    idu_out->IduL_PixelsPerLine	    = 
						    idu_in->IduL_PixelsPerLine;
		    idu_out->IduL_PlaneBitsPerPixel = 1;
		    idu_out->IduL_ScanlineStride    = 
						    idu_out->IduL_PixelStride * 
						    idu_in->IduL_PixelsPerLine;
		    ++idu_out;
		    } /* end for */
		++ice_in;
		++ice_out;
		} /* end for */
	    } /* end if then */
	else
	    {
	    /*
	    ** Since the organization hasn't changed, copy the input IDUs
	    ** into the output IDUs.  (NOTE that there will always only
	    ** be one IDU per ICE for this organization.)
	    */
	    ice_in = fat_in->FatR_Ice;
	    ice_out = fat_out->FatR_Ice;
	    for ( ice_idx = 0; ice_idx < fat_out->FatL_IceCnt; ++ice_idx )
		{
		idu_out = ice_out->IceR_Idu;
		idu_in = ice_in->IceR_Idu;
		for ( idu_idx = 0; idu_idx < ice_out->IceL_IduCnt; ++idu_idx )
		    {
		    *idu_out = *idu_in;
		    ++idu_in;
		    ++idu_out;
		    } /* end for */

		++ice_in;
		++ice_out;
		} /* end for */
	    } /* end if else */
	    break;

    case ImgK_BandIntrlvdByLine:
	if ( fat_out->FatL_CompSpaceOrg != fat_in->FatL_CompSpaceOrg )
	    /*
	    ** Since the organization has changed, use the first input
	    ** IDU as a partial template for the output IDU.
	    */
	    {
	    ice_in = fat_in->FatR_Ice;
	    ice_out = fat_out->FatR_Ice;
	    for ( ice_idx = 0; ice_idx < fat_out->FatL_IceCnt; ++ice_idx )
		{
		idu_out = ice_out->IceR_Idu;
		idu_in = ice_in->IceR_Idu;

		idu_out->IduL_BitOrder		= idu_in->IduL_BitOrder;
		idu_out->IduL_ByteOrder		= idu_in->IduL_ByteOrder;
		idu_out->IduL_ByteUnit		= idu_in->IduL_ByteUnit;
		idu_out->IduL_CompressionType	= ImgK_PcmCompression;
	    	idu_out->IduL_DataOffset	= 0;
		idu_out->IduL_DataType	    	= idu_in->IduL_DataType;
		idu_out->IduL_Dtype		= idu_in->IduL_Dtype;
		idu_out->IduL_NumberOfLines	= idu_in->IduL_NumberOfLines;
	    /*
	    ** NOTE that for this organization, pixel stride doesn't really
	    ** have an unambiguous meaning, since the component data for
	    ** each pixel, although in the same data plane, is not 
	    ** contiguous on a pixel-by-pixel basis.  Therefore, for the
	    ** sake of argument (and verification), it's set somewhat
	    ** arbitrarily to be equal to the number of total bits per 
	    ** pixel.  Doing so has the side-benefit of making the stride
	    ** initialization work out correctly.
	    */
		idu_out->IduL_PixelStride	= 
						fat_out->FatL_TotalBitsPerPixel;
		idu_out->IduL_PixelsPerLine	= idu_in->IduL_PixelsPerLine;
		idu_out->IduL_PlaneBitsPerPixel	= idu_out->IduL_PixelStride;
		idu_out->IduL_ScanlineStride	= idu_out->IduL_PixelStride * 
						  idu_in->IduL_PixelsPerLine;
		++ice_out;
		++ice_in;
		} /* end for */
	    } /* end if then */
	else
	    {
	    /*
	    ** Since the organization hasn't changed, copy the input IDUs
	    ** into the output IDUs.  (NOTE that there will always only
	    ** be one IDU per ICE for this organization.)
	    */
	    ice_in = fat_in->FatR_Ice;
	    ice_out = fat_out->FatR_Ice;
	    for ( ice_idx = 0; ice_idx < fat_out->FatL_IceCnt; ++ice_idx )
		{
		idu_in = ice_in->IceR_Idu;
		idu_out = ice_out->IceR_Idu;
		*idu_out = *idu_in;

		++ice_in;
		++ice_out;
		} /* end for */
	    } /* end if else */

    default:
	    break;
    } /* end switch */


/*
** Adjust the output fat according to the default rules of adjustment.
** Note that these adjustments may be over-ridden when the entire item
** list is applied to the output fat.
*/
ice_out = fat_out->FatR_Ice;
for ( list_item = itmlst; list_item->ItmL_Code != 0; ++list_item )
    {
    if ( list_item->ItmA_Buffer != 0 )
	{
	item_value = *((long *)(list_item->ItmA_Buffer));
	item_index = list_item->ItmL_Index;
	}
    else
	continue;   /* jump back to top of loop */

    switch ( list_item->ItmL_Code )
	{
	case Img_BitsPerComp:
	    if ( item_index >= fat_out->FatL_NumberOfComp )
		ChfSignal( 4, ImgX_INVATRIDX, 2, list_item->ItmL_Code, 
			    item_index );


	    ici_out = fat_out->FatR_Ici;
	    if ( ici_out[item_index].IciL_QuantLevelsPerComp == 0 )
		{
		ici_out[item_index].IciL_QuantLevelsPerComp = 1 << item_value;
		ici_out[item_index].IciL_QuantBitsPerComp = item_value;
		}
	    break; /* case Img_BitsPerComp */

	case Img_PlaneBitsPerPixel:
	    if ( item_index >= fat_out->FatL_PlanesPerPixel )
		ChfSignal( 4, ImgX_INVATRIDX, 2, list_item->ItmL_Code, 
			    item_index );

	    idu_out = &(ice_out->IceR_Idu[list_item->ItmL_Index]);
	    idu_out->IduL_PlaneBitsPerPixel = item_value;

	    /*
	    ** Adjust pixel stride?
	    */
	    if ( idu_out->IduL_PixelStride < 
		    idu_out->IduL_PlaneBitsPerPixel )
		{
		idu_out->IduL_PixelStride = 
		    idu_out->IduL_PlaneBitsPerPixel;

		/*
		** Adjust scanline stride?
		*/
		if ( idu_out->IduL_PixelStride * 
			idu_out->IduL_PixelsPerLine >
			    idu_out->IduL_ScanlineStride )
		    idu_out->IduL_ScanlineStride =
						idu_out->IduL_PixelStride * 
						idu_out->IduL_PixelsPerLine;
		}
	    
	    break; /* case Img_PlaneBitsPerPixel */

	case Img_PixelStride:
	    if ( item_index >= fat_out->FatL_PlanesPerPixel )
		ChfSignal( 4, ImgX_INVATRIDX, 2, list_item->ItmL_Code, 
			    item_index );

	    idu_out = &(ice_out->IceR_Idu[list_item->ItmL_Index]);
	    idu_out->IduL_PixelStride = item_value;

	    /*
	    ** Adjust scanline stride?
	    */
	    if ( idu_out->IduL_PixelStride * idu_out->IduL_PixelsPerLine >
		 idu_out->IduL_ScanlineStride )
		idu_out->IduL_ScanlineStride = idu_out->IduL_PixelStride * 
						idu_out->IduL_PixelsPerLine;

	    break; /* case Img_PixelStride */

	case Img_PixelsPerLine:
	    if ( item_index >= fat_out->FatL_PlanesPerPixel )
		ChfSignal( 4, ImgX_INVATRIDX, 2, list_item->ItmL_Code, 
			    item_index );

	    idu_out = &(ice_out->IceR_Idu[list_item->ItmL_Index]);
	    idu_out->IduL_PixelsPerLine = item_value;

	    /*
	    ** Adjust scanline stride?
	    */
	    if ( idu_out->IduL_PixelStride * idu_out->IduL_PixelsPerLine >
		 idu_out->IduL_ScanlineStride )
		idu_out->IduL_ScanlineStride = idu_out->IduL_PixelStride * 
						idu_out->IduL_PixelsPerLine;

	    break; /* case Img_PixelsPerLine */

	case Img_QuantLevelsPerComp:
	    if ( item_index >= fat_out->FatL_NumberOfComp )
		ChfSignal( 4, ImgX_INVATRIDX, 2, list_item->ItmL_Code, 
			    item_index );
	    switch ( fat_out->FatL_CompSpaceOrg )
		{
		case ImgK_BandIntrlvdByPixel:
		case ImgK_BandIntrlvdByLine:
		    tmp_ice_in = fat_in->FatR_Ice;
		    tmp_ice_out = fat_out->FatR_Ice;
		    ice_cnt = fat_out->FatL_IceCnt;
		    ice_idx = 0;
		    for ( ; ice_idx < ice_cnt; 
				++ice_idx, ++tmp_ice_in, ++tmp_ice_out )
			{
			idu_out = tmp_ice_out->IceR_Idu;
			idu_in = tmp_ice_in->IceR_Idu;

			/*
			** Adjust Pixel Stride(s)
			*/
			idu_out->IduL_PixelStride = 
					    fat_out->FatL_TotalBitsPerPixel;

			/*
			** Is the output pixels per line count still 
			** initialized to zero?
			*/
			if ( idu_out->IduL_PixelsPerLine != 0 )
			    /*
			    ** No, so use it ...
			    */
			    pixels_per_line = idu_out->IduL_PixelsPerLine;
			/*
			** Yes, so see if pixels per line is given in the
			** itemlist ...
			*/
			else if ( !(_ImgExtractItmlstItem( itmlst, 
				    Img_PixelsPerLine, &pixels_per_line,
				    &length, 0, &index, 1 )))
			    /*
			    ** No it's not there either, so use whatever 
			    ** value is in the input idu.
			    */
			    pixels_per_line = idu_in->IduL_PixelsPerLine;

			/*
			** Adjust Scanline Stride(s)
			*/
			idu_out->IduL_ScanlineStride = pixels_per_line *
						    idu_out->IduL_PixelStride;
			} /* end ice for loop */
		    break;
		case ImgK_BandIntrlvdByPlane:
		    ici_in = fat_in->FatR_Ici;
		    tmp_ice_in = fat_in->FatR_Ice;
		    tmp_ice_out = fat_out->FatR_Ice;
		    ice_cnt = fat_out->FatL_IceCnt;
		    ice_idx = 0;
		    for ( ; ice_idx < ice_cnt; 
				++ice_idx, ++tmp_ice_in, ++tmp_ice_out )
			{
			idu_out = tmp_ice_out->IceR_Idu;
			idu_in = tmp_ice_in->IceR_Idu;
			idu_idx = 0;
			idu_cnt = tmp_ice_out->IceL_IduCnt;
			for ( ; idu_idx < idu_cnt; ++idu_idx, ++idu_out,
							++ici_in )
			    {
			    /*
			    ** Adjust Pixel Stride(s)
			    */
			    idu_out->IduL_PixelStride = 
						    ici_in->IciL_BitsPerComp;

			    /*
			    ** Is the output pixels per line count still 
			    ** initialized to zero?
			    */
			    if ( idu_out->IduL_PixelsPerLine != 0 )
				/*
				** No, so use it ...
				*/
				pixels_per_line = idu_out->IduL_PixelsPerLine;
			    /*
			    ** Yes, so see if pixels per line is given in the
			    ** itemlist ...
			    */
			    else if ( !(_ImgExtractItmlstItem( itmlst, 
					Img_PixelsPerLine, &pixels_per_line,
					&length, 0, &index, 1 )))
				/*
				** No it's not there either, so use whatever 
				** value is in the input idu.
				*/
				pixels_per_line = idu_in->IduL_PixelsPerLine;

			    /*
			    ** Adjust Scanline Stride(s)
			    */
			    idu_out->IduL_ScanlineStride = pixels_per_line *
						    idu_out->IduL_PixelStride;
			    } /* end idu loop */
			} /* end ice for loop */
		    break;
		case ImgK_BitIntrlvdByPlane:
		    tmp_ice_in = fat_in->FatR_Ice;
		    tmp_ice_out = fat_out->FatR_Ice;
		    ice_cnt = fat_out->FatL_IceCnt;
		    ice_idx = 0;
		    for ( ; ice_idx < ice_cnt; 
				++ice_idx, ++tmp_ice_in, ++tmp_ice_out )
			{
			idu_out = tmp_ice_out->IceR_Idu;
			idu_in = tmp_ice_in->IceR_Idu;
			idu_idx = 0;
			idu_cnt = tmp_ice_out->IceL_IduCnt;
			for ( ; idu_idx < idu_cnt; ++idu_idx, ++idu_out )
			    {
			    /*
			    ** Adjust Pixel Stride(s)
			    */
			    idu_out->IduL_PixelStride = 1;

			    /*
			    ** Is the output pixels per line count still 
			    ** initialized to zero?
			    */
			    if ( idu_out->IduL_PixelsPerLine != 0 )
				/*
				** No, so use it ...
				*/
				pixels_per_line = idu_out->IduL_PixelsPerLine;
			    /*
			    ** Yes, so see if pixels per line is given in the
			    ** itemlist ...
			    */
			    else if ( !(_ImgExtractItmlstItem( itmlst, 
					Img_PixelsPerLine, &pixels_per_line,
					&length, 0, &index, 1 )))
				/*
				** No it's not there either, so use whatever 
				** value is in the input idu.
				*/
				pixels_per_line = idu_in->IduL_PixelsPerLine;

			    /*
			    ** Adjust Scanline Stride(s)
			    */
			    idu_out->IduL_ScanlineStride = pixels_per_line *
						    idu_out->IduL_PixelStride;
			    } /* end idu loop */
			} /* end ice for loop */
		    break;
		}
	    break; /* case Img_QuantLevelsPerComp */

	case Img_ScanlineStride:
	    if ( item_index >= fat_out->FatL_PlanesPerPixel )
		ChfSignal( 4, ImgX_INVATRIDX, 2, list_item->ItmL_Code, 
			    item_index );

	    idu_out = &(ice_out->IceR_Idu[list_item->ItmL_Index]);
	    idu_out->IduL_ScanlineStride = item_value;

	    break; /* case Img_ScanlineStride */
	default:
	    break;
	} /* end switch */
    } /* end for */


/*
** Apply the item list to the output fat in order to set any other
** attributes that weren't set by the adjustment process.  NOTE that
** the whole list gets processed (which shouldn't matter).
*/
Apply_itmlst_to_fat( fat_out, itmlst );

/*
** Reset totals so that they are consistent with attribute values
** which have been adjusted.
*/
ici_out = fat_out->FatR_Ici;
fat_out->FatL_TotalBitsPerPixel = 0;
fat_out->FatL_TotalQuantBitsPerPixel = 0;

for ( ici_idx = 0; ici_idx < fat_out->FatL_NumberOfComp; ++ici_idx )
    {
    fat_out->FatL_TotalBitsPerPixel += ici_out->IciL_BitsPerComp;
    fat_out->FatL_TotalQuantBitsPerPixel += ici_out->IciL_QuantBitsPerComp;
    ++ici_out;
    }

/*
** Last chance consistency check ...
**
**	- Make sure that the total plane bits per pixel equals the
**	  total bits per component.  If they aren't, adjust the
**	  bits per component to be right (on the high side).
**
**	  NOTE: this logic only corrects the attributes which are
**		writeable to the frame; all read-only attributes are
**		not updated (for now ...)
*/
total_plane_bits_per_pixel = 0;
ice_out = fat_out->FatR_Ice;
idu_out = ice_out->IceR_Idu;
for ( index = 0; index < fat_out->FatL_PlanesPerPixel; ++index )
    {
    total_plane_bits_per_pixel += idu_out->IduL_PlaneBitsPerPixel;
    ++idu_out;
    }

if ( total_plane_bits_per_pixel != fat_out->FatL_TotalBitsPerPixel )
    {
    switch (fat_out->FatL_CompSpaceOrg )
	{
	case ImgK_BandIntrlvdByPixel:
	/*
	** NOTE: for ImgK_BandIntrlvdByLine, the pixel stride adjustment
	**	 that's done doesn't really mean anything.
	*/
	case ImgK_BandIntrlvdByLine:
	    /*
	    ** If there are fewer component bits per pixel than plane
	    ** bits per pixel ...
	    */
	    ici_out = fat_out->FatR_Ici;
	    idu_out = fat_out->FatR_Ice->IceR_Idu;
	    if ( fat_out->FatL_TotalBitsPerPixel < total_plane_bits_per_pixel )
		{
		/*
		** We can fix it if there's only one component, otherwise
		** there's going to be a problem (later).
		*/
		if ( fat_out->FatL_NumberOfComp == 1 )
		    {
		    ici_out->IciL_BitsPerComp = idu_out->IduL_PlaneBitsPerPixel;
		    }
		/* NOTE: no else here, meaning the problem wasn't fixed	*/
		}
	    /*
	    ** There are more component bits per pixel than plane bits ...
	    */
	    else
		/*
		** Make the plane-bits equal to the component bits and
		** adjust the strides if necessary.
		*/
		{
		idu_out->IduL_PlaneBitsPerPixel = ici_out->IciL_BitsPerComp;
		if ( idu_out->IduL_PlaneBitsPerPixel < 
			idu_out->IduL_PixelStride )
		    {
		    idu_out->IduL_PixelStride = idu_out->IduL_PlaneBitsPerPixel;
		    idu_out->IduL_ScanlineStride =
			idu_out->IduL_PixelStride * idu_out->IduL_NumberOfLines;
		    }
		}
	    break;
	case ImgK_BandIntrlvdByPlane:
	    /*
	    ** If there are fewer component bits per pixel than plane
	    ** bits per pixel ...
	    */
	    ici_out = fat_out->FatR_Ici;
	    idu_out = fat_out->FatR_Ice->IceR_Idu;
	    if ( fat_out->FatL_TotalBitsPerPixel < total_plane_bits_per_pixel )
		{
		/*
		** Make the bits per component equal to the number of
		** plane bits per pixel (since each component is in a 
		** separate plane).
		*/
		for ( index = 0; index < fat_out->FatL_NumberOfComp; ++index )
		    {
		    ici_out->IciL_BitsPerComp = idu_out->IduL_PlaneBitsPerPixel;
		    ++ici_out;
		    ++idu_out;
		    }
		}
	    /*
	    ** There are more component bits per pixel than plane bits ...
	    */
	    else
		/*
		** Make the plane-bits equal to the component bits and
		** adjust the strides if necessary.
		*/
		{
		for ( index = 0; index < fat_out->FatL_NumberOfComp; ++index )
		    {
		    idu_out->IduL_PlaneBitsPerPixel = ici_out->IciL_BitsPerComp;
		    if ( idu_out->IduL_PlaneBitsPerPixel < 
			    idu_out->IduL_PixelStride )
			{
			idu_out->IduL_PixelStride = 
						idu_out->IduL_PlaneBitsPerPixel;
			idu_out->IduL_ScanlineStride = 
						idu_out->IduL_PixelStride * 
						    idu_out->IduL_NumberOfLines;
			}
		    ++ici_out;
		    ++idu_out;
		    }
		}
	    break;
	case ImgK_BitIntrlvdByPlane:
	    /*
	    ** This is a major problem ... no attempt to fix it
	    ** will be made by this code (for the moment).
	    */
	    break;
	}
    } 

/*
** AutoDeallocateate input fat?
*/
if ( (flags & ImgM_AutoDeallocate) != 0 )
    Deallocate_fat( fat_in );

return fat_out;
} /* end of _ImgAdjustFat */


/******************************************************************************
**  _ImgConvertLevelsToBits
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgConvertLevelsToBits( levels )
long	levels;
{
long	bits;

/*
** Calculate the new number of bits per component by finding the 
** base 2 log of the number of levels, and then taking the integer
** ceiling of that.  
**
** Since log2 functions aren't available, we have to make due with 
** functions log10 and a little math.  The identity being used is:
**
**	log X  =  log X   /  log b  [this is a variation on the chain rule]
**	   b         a          a
**
** which translates in this instance to:
**
**	log levels = log  levels  /  log  2
**         2            10              10
**
** Note that when levels is 1, this doesn't work, because log10(1) is zero,
** so we have to have a special case for levels == 1.
*/

if ( levels != 1 )
    bits = ceil( (log10((double)levels)/log10(2.0))  );
else
    bits = 1;

return bits;
} /* end of _ImgConvertLevelsToBits */


/******************************************************************************
**  _ImgCreateFat()
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
struct FAT *_ImgCreateFat( image_data_class, itmlst )
long		image_data_class;
struct ITMLST   *itmlst;
{
long	     bits_per_comp;
long	     component_count;
long	     comp_space_org;
long	     flags		= ImgM_AutoDeallocate; 
long	     found_status;
long	     ice_count		= 1;
long	     index;
long	     length;
long	     plane_count;
long	     quant_levels_per_comp;

struct FAT  *fat;
struct FAT  *ret_fat;

found_status = _ImgExtractItmlstItem( itmlst, Img_CompSpaceOrg,
		    &comp_space_org, &length, 0, &index, 1 );
if (!found_status )
    comp_space_org = ImgK_BandIntrlvdByPlane;

/*
** Figure out the plane count and the component count.  Note that
** the component space org is significant.
*/
switch ( image_data_class )
    {
    case ImgK_ClassBitonal:
    case ImgK_ClassGreyscale:
    default:
	component_count = 1;
	plane_count = 1;
	break;

    case ImgK_ClassMultispect:
	component_count = 3;
	plane_count = 3;
	break;
    }

fat = Allocate_fat( component_count, ice_count, plane_count );
Apply_defaults( fat, image_data_class );

ret_fat = _ImgAdjustFat( fat, itmlst, flags );

return ret_fat;
} /* end of _ImgCreateFat */


/******************************************************************************
**  _ImgDeleteCsa
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgDeleteCsa( csa )
struct CSA  *csa;
{

_ImgCfree( csa->CsaL_BitsPerComp );
_ImgCfree( csa->CsaL_QuantBitsPerComp );
_ImgCfree( csa->CsaR_PlaneUdpList );
if ( csa->CsaR_CompUdpList != 0 )
    _ImgCfree( csa->CsaR_CompUdpList );
_ImgCfree( csa );

return;
} /* end of _ImgDeleteCsa */


/******************************************************************************
**  _ImgDeleteFat
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgDeleteFat( fat )
struct FAT  *fat;
{

/*
** Deallocate the block and all memory associated with it.
*/
Deallocate_fat( fat );

return;
} /* end of _ImgDeleteFat */


/******************************************************************************
**  _ImgExtractCsa
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
struct CSA *_ImgExtractCsa( fid )
struct FCT *fid;
{
long	     comp_idx;
long	     comp_space_org;
long	     image_data_class;
long	     number_of_comp;
long	     plane_idx;
long	     plane_signif;
long	     planes_per_pixel;
long	     pos;
long	     quant_levels;
long	     spectral_mapping;
long	     total_bits_per_pixel;
long	     total_quant_bits_per_pixel;

struct UDP  *comp_udp;
struct CSA  *csa;

_ImgGet( fid, Img_CompSpaceOrg,		&comp_space_org,	LONGSIZE, 0,0 );
_ImgGet( fid, Img_ImageDataClass,	&image_data_class,	LONGSIZE, 0,0 );
_ImgGet( fid, Img_NumberOfComp,		&number_of_comp,	LONGSIZE, 0,0 );
_ImgGet( fid, Img_PlaneSignif,		&plane_signif,		LONGSIZE, 0,0 );
_ImgGet( fid, Img_PlanesPerPixel,	&planes_per_pixel,	LONGSIZE, 0,0 );
_ImgGet( fid, Img_SpectralMapping,	&spectral_mapping,	LONGSIZE, 0,0 );
_ImgGet( fid, Img_TotalBitsPerPixel,	&total_bits_per_pixel,	LONGSIZE, 0,0 );
_ImgGet( fid, Img_TotalQuantBitsPerPixel, &total_quant_bits_per_pixel,
								LONGSIZE, 0,0 );

csa = (struct CSA *) _ImgCalloc( 1, CsaK_Size );

csa->CsaL_CompSpaceOrg		    = comp_space_org;
csa->CsaL_ImageDataClass	    = image_data_class;
csa->CsaL_NumberOfComp		    = number_of_comp;
csa->CsaL_PlaneSignif		    = plane_signif;
csa->CsaL_PlanesPerPixel	    = planes_per_pixel;
csa->CsaL_SpectralMapping	    = spectral_mapping;
csa->CsaL_TotalBitsPerPixel	    = total_bits_per_pixel;
csa->CsaL_TotalQuantBitsPerPixel    = total_quant_bits_per_pixel;

csa->CsaL_BitsPerComp = (long*) _ImgCalloc( number_of_comp, LONGSIZE );
csa->CsaL_QuantBitsPerComp = (long*) _ImgCalloc( number_of_comp, LONGSIZE );
for ( comp_idx = 0; comp_idx < number_of_comp; ++comp_idx )
    {
    _ImgGet( fid, Img_BitsPerComp, &((csa->CsaL_BitsPerComp)[comp_idx]),
		LONGSIZE, 0, comp_idx );
    _ImgGet( fid, Img_QuantBitsPerComp, 
		&((csa->CsaL_QuantBitsPerComp)[comp_idx]), 
		LONGSIZE, 0, comp_idx );
    }

/*
** Allocate and set up UDPs for each data plane.
*/
csa->CsaR_PlaneUdpList = (struct UDP *) _ImgCalloc( planes_per_pixel, 
						sizeof( struct UDP ) );
for ( plane_idx = 0; plane_idx < planes_per_pixel; ++plane_idx )
    {
    _ImgGet( fid, Img_Udp, &((csa->CsaR_PlaneUdpList)[plane_idx]), 
		    sizeof(struct UDP), 0, plane_idx );
    }

/*
** Allocate and set up UDPs for each component (unless the data is
** bit interleaved by plane).
*/
if ( comp_space_org != ImgK_BitIntrlvdByPlane )
    {
    comp_udp = (struct UDP *) _ImgCalloc( number_of_comp, sizeof( struct UDP ));
    csa->CsaR_CompUdpList = comp_udp; 
    switch ( comp_space_org )
	{
	case ImgK_BandIntrlvdByPixel:
	    pos = 0;
	    for ( comp_idx = 0; comp_idx < number_of_comp; ++comp_idx )
		{
		/*
		** Get the same UDP every time (since there is only one
		** data plane), and set up the pixel length, pos, comp idx,
		** and quant levels fields to correspond exactly to the
		** component data.  NOTE, for instance, that the pixel length
		** is equal to the quant bits per comp.
		*/
		_ImgGet( fid, Img_Udp, comp_udp, sizeof(struct UDP), 0, 0 );
		comp_udp->UdpW_PixelLength = 
				    (csa->CsaL_QuantBitsPerComp)[comp_idx];
		comp_udp->UdpL_Pos = pos;
		comp_udp->UdpL_CompIdx = comp_idx;
		_ImgGet( fid, Img_QuantLevelsPerComp, &quant_levels, LONGSIZE,
			    0, comp_idx );
		comp_udp->UdpL_Levels = quant_levels;

		pos += comp_udp->UdpW_PixelLength;
		++comp_udp;
		}
	    break;

	case ImgK_BandIntrlvdByPlane:
	    for ( comp_idx = 0; comp_idx < number_of_comp; ++comp_idx )
		{
		/*
		** Get each componant UDP.  Set the pixel length to be
		** equal to the quant bits per comp.
		*/
		_ImgGet( fid, Img_Udp, comp_udp, sizeof(struct UDP), 0, 
			    comp_idx );
		comp_udp->UdpW_PixelLength = 
				    (csa->CsaL_QuantBitsPerComp)[comp_idx];
		++comp_udp;
		}
	    break;

	case ImgK_BandIntrlvdByLine:
	    pos = 0;
	    for ( comp_idx = 0; comp_idx < number_of_comp; ++comp_idx )
		{
		/*
		** Get the same UDP every time (since there is only one
		** data plane), and set up the pixel length, pos, comp idx,
		** and quant levels fields to correspond exactly to the
		** component data.  
		**
		** NOTE that the pixel length AND the pixel strides
		** are equal to the quant bits per comp.
		*/
		_ImgGet( fid, Img_Udp, comp_udp, sizeof(struct UDP), 0, 0 );
		comp_udp->UdpW_PixelLength = 
				    (csa->CsaL_QuantBitsPerComp)[comp_idx];
		comp_udp->UdpL_PxlStride = comp_udp->UdpW_PixelLength;
		comp_udp->UdpL_Pos = pos;
		comp_udp->UdpL_CompIdx = comp_idx;
		_ImgGet( fid, Img_QuantLevelsPerComp, &quant_levels, LONGSIZE,
			    0, comp_idx );
		comp_udp->UdpL_Levels = quant_levels;

		pos += comp_udp->UdpL_PxlStride * comp_udp->UdpL_PxlPerScn;
		++comp_udp;
		}

	default:
	    break;
	} /* end switch */
    }

return csa;
} /* end of _ImgExtractCsa */


/******************************************************************************
**  _ImgExtractFat()
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
struct FAT *_ImgExtractFat( fid )
struct FCT	    *fid;
{
long	     component_count;
long	     flags		= 0;
long	     ice_cnt;
long	     ice_idx;
long	     idu_cnt;
long	     idu_idx;
long	     index;
long	     plane_count;

struct FAT  *fat;
struct ICE  *ice;
struct ICI  *ici;
struct IDU  *idu;


_ImgGet( fid, Img_NumberOfComp, &component_count, LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_Icecnt, &ice_cnt, LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_PlanesPerPixel, &plane_count, LONGSIZE, 0, NOINDEX );

fat = Allocate_fat( component_count, ice_cnt, plane_count );

/*
** Get all attributes that are common to frames and to frame definitions.
*/

    /*
    ** Array valued attributes based on the number of components
    */
ici = fat->FatR_Ici;
for ( index = 0; index < component_count; ++index )
    {
    _ImgGet( fid, Img_BitsPerComp, &(ici->IciL_BitsPerComp),
		LONGSIZE, 0, index );
    _ImgGet( fid, Img_QuantBitsPerComp, &(ici->IciL_QuantBitsPerComp),
		LONGSIZE, 0, index );
    _ImgGet( fid, Img_QuantLevelsPerComp, 
		&(ici->IciL_QuantLevelsPerComp), LONGSIZE, 0, index );
    ++ici;
    }

    /*
    ** Array valued attributes based on the number of image
    ** component elements and image data units (data planes).
    */
ice = fat->FatR_Ice;
for ( ice_idx = 0; ice_idx < ice_cnt; ++ice_idx )
    {
    idu = ice->IceR_Idu;
    idu_cnt = ice->IceL_IduCnt;
    for ( idu_idx = 0; idu_idx < idu_cnt; ++idu_idx )
	{
	_ImgGet( fid, Img_BitOrder, &(idu->IduL_BitOrder),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_ByteOrder, &(idu->IduL_ByteOrder),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_ByteUnit, &(idu->IduL_ByteUnit),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_CdpPrsnt, &(idu->IduL_CdpPrsnt),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_CompressionType, &(idu->IduL_CompressionType),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_CompressionParams, 
		&(idu->IduR_CompressionParams), LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_DataOffset, &(idu->IduL_DataOffset),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_DataPlaneBase, &(idu->IduA_DataPlaneBase), 
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_DataPlaneBitSize, &(idu->IduL_DataPlaneBitSize), 
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_DataPlaneSize, &(idu->IduL_DataPlaneSize), 
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_DataType, &(idu->IduL_DataType),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_Dtype, &(idu->IduL_Dtype),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_NumberOfLines, &(idu->IduL_NumberOfLines),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_PixelAlignment, &(idu->IduL_PixelAlignment),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_PixelStride, &(idu->IduL_PixelStride),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_PixelsPerLine, &(idu->IduL_PixelsPerLine),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_PlaneBitsPerPixel,
		&(idu->IduL_PlaneBitsPerPixel), LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_ScanlineAlignment, &(idu->IduL_ScanlineAlignment),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_ScanlineStride, &(idu->IduL_ScanlineStride),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_UdpPrsnt, &(idu->IduL_UdpPrsnt),
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_VirtualArsize, &(idu->IduL_VirtualArsize),
		LONGSIZE, 0, idu_idx );
	++idu;
	} /* end of idu for */

    ++ice;
    } /* end ice for */

    /*
    ** Miscellaneous attributes.
    */
_ImgGet( fid, Img_PxlAspectRatio, &(fat->FatR_PxlAspectRatio), 
	    QUADSIZE, 0, NOINDEX );
_ImgGet( fid, Img_BrtPolarity, &(fat->FatL_BrtPolarity), 
	    LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_CompSpaceOrg, &(fat->FatL_CompSpaceOrg),
		LONGSIZE, 0, NOINDEX );
/*
** omit comp wavelenth info for now
*/

_ImgGet( fid, Img_GridType, &(fat->FatL_GridType),
		LONGSIZE, 0, NOINDEX );

_ImgGet( fid, Img_FrmBoxLLX, &(fat->FatL_FrmBoxLLX),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmBoxLLXC, &(fat->FatL_FrmBoxLLXC),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmBoxLLY, &(fat->FatL_FrmBoxLLY),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmBoxLLYC, &(fat->FatL_FrmBoxLLYC),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmBoxURX, &(fat->FatL_FrmBoxURX),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmBoxURXC, &(fat->FatL_FrmBoxURXC),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmBoxURY, &(fat->FatL_FrmBoxURY),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmBoxURYC, &(fat->FatL_FrmBoxURYC),
		LONGSIZE, 0, NOINDEX );

_ImgGet( fid, Img_FrmPositionC, &(fat->FatL_FrmPositionC),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmFxdPositionX, &(fat->FatL_FrmfxdPositionX),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmFxdPositionXC, &(fat->FatL_FrmfxdPositionXC),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmFxdPositionY, &(fat->FatL_FrmfxdPositionY),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_FrmFxdPositionYC, &(fat->FatL_FrmfxdPositionYC),
		LONGSIZE, 0, NOINDEX );

_ImgGet( fid, Img_ImageDataClass, &(fat->FatL_ImageDataClass),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_LineProgression, &(fat->FatL_LineProgression),
		LONGSIZE, 0, NOINDEX );

/* skip lookup tables for now */

_ImgGet( fid, Img_LPPixelDist, &(fat->FatL_LpPixelDist),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_NumberOfComp, &(fat->FatL_NumberOfComp),
		LONGSIZE, 0, NOINDEX );

_ImgGet( fid, Img_PixelGroupOrder , &(fat->FatL_PixelGroupOrder),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_PixelGroupSize, &(fat->FatL_PixelGroupSize),
		LONGSIZE, 0, NOINDEX );

_ImgGet( fid, Img_PixelPath, &(fat->FatL_PixelPath),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_PlaneSignif, &(fat->FatL_PlaneSignif ),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_PlanesPerPixel, &(fat->FatL_PlanesPerPixel),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_PPPixelDist, &(fat->FatL_PpPixelDist),
		LONGSIZE, 0, NOINDEX );

/* skip private data for now */

_ImgGet( fid, Img_SpectralMapping, &(fat->FatL_SpectralMapping),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_StandardFormat, &(fat->FatL_StandardFormat),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_TotalBitsPerPixel, &(fat->FatL_TotalBitsPerPixel),
		LONGSIZE, 0, NOINDEX );
_ImgGet( fid, Img_TotalQuantBitsPerPixel, &(fat->FatL_TotalQuantBitsPerPixel),
		LONGSIZE, 0, NOINDEX );

_ImgGet( fid, Img_UserField, &(fat->FatL_UserField),
		LONGSIZE, 0, NOINDEX );

_ImgGet( fid, Img_UserLabelCnt, &(fat->FatL_UserLabelCnt),
		LONGSIZE, 0, NOINDEX );

if ( fat->FatL_UserLabelCnt != 0 )
    {
    char    **user_label_strptrs;
    char     *user_label_str;
    long      label_idx		    = 0;
    long      label_len;

    /*
    ** Create an array of user label pointers
    */
    fat->FatA_UserLabelStrptrs = (char **)_ImgCalloc( 
				    fat->FatL_UserLabelCnt, LONGSIZE );
    user_label_strptrs = fat->FatA_UserLabelStrptrs;

    /*
    ** Create buffers and copy user label strings as ASCIZ strings 
    ** from the frame or definition structure into the fat.
    */
    for ( ; label_idx < fat->FatL_UserLabelCnt; ++label_idx )
	{
	_ImgGet( fid, Img_UserLabelLen , &label_len, LONGSIZE, 0, label_idx );

	user_label_str = _ImgCalloc( label_len + 1, CHARSIZE );
	*user_label_strptrs = user_label_str;
	++user_label_strptrs;

	_ImgGet( fid, Img_UserLabel, user_label_str, label_len, 0, label_idx );

	} /* end for */

    } /* end if */

return fat;
} /* end of _ImgExtractFat */


/******************************************************************************
**  _ImgGetStandardizedAttrlst
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
struct ITMLST *_ImgGetStandardizedAttrlst( fid )
struct FCT *fid;
{
long		 data_class;
struct ITMLST	*standardized_attrlst   = 0;

_ImgGet( fid, Img_ImageDataClass, &data_class, sizeof(long), 0, 0 );

switch ( data_class )
    {
    case ImgK_ClassBitonal:
	standardized_attrlst = Std_bitonal_attrs;
	break;
    case ImgK_ClassGreyscale:
	standardized_attrlst = Std_greyscale_attrs;
	break;
    case ImgK_ClassMultispect:
	standardized_attrlst = Std_multispectral_attrs;
	break;
    default:
	break;
    }

return standardized_attrlst;
} /* end of _ImgGetStandardizedAttrlst */


/******************************************************************************
**  _ImgVerifyFat
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgVerifyFat( fat, flags )
struct FAT	*fat;
unsigned long	 flags;
{
long	ice_cnt;
long	ice_idx;
long	idu_cnt;
long	idu_idx;
long	index;
long	total_bits_per_comp	    = 0;
long	total_quant_bits_per_comp   = 0;
long	total_plane_bits_per_pixel  = 0;

struct ICE  *ice;
struct ICI  *ici;
struct IDU  *idu;

/*
** Verify items that must be non-zero
*/
if ( fat->FatL_NumberOfComp == 0 )	/* REQUIRED BY DDIF */
    ChfSignal( 4, ImgX_INVZERVAL, 2, Img_NumberOfComp, NOINDEX );

if ( fat->FatL_LpPixelDist == 0 )
    ChfSignal( 4, ImgX_INVZERVAL, 2, Img_LPPixelDist, NOINDEX );

if ( fat->FatL_PlanesPerPixel == 0 )
    ChfSignal( 4, ImgX_INVZERVAL, 2, Img_PlanesPerPixel, NOINDEX );

if ( fat->FatL_PpPixelDist == 0 )
    ChfSignal( 4, ImgX_INVZERVAL, 2, Img_PPPixelDist, NOINDEX );

if ( !(flags & ImgM_NoDataPlaneVerify) )
    {
    ici = fat->FatR_Ici;
    for ( index = 0; index < fat->FatL_NumberOfComp; ++index )
	{
	if ( ici->IciL_BitsPerComp == 0 )	/* REQUIRED BY DDIF */
	    ChfSignal( 4, ImgX_INVZERVAL, 2, Img_BitsPerComp, index );
	total_bits_per_comp += ici->IciL_BitsPerComp;

	if ( ici->IciL_QuantBitsPerComp == 0 )
	    ChfSignal( 4, ImgX_INVZERVAL, 2, Img_QuantBitsPerComp, index );
	total_quant_bits_per_comp += ici->IciL_QuantBitsPerComp;

	if ( ici->IciL_QuantLevelsPerComp == 0 )
	    ChfSignal( 4, ImgX_INVZERVAL, 2, Img_QuantLevelsPerComp, index );

	/*
	** While we're in the for-loop, compare the number of levels with
	** the number of bits per comp.
        */
	if ( ici->IciL_BitsPerComp < 32 )
	    {
	    if ( ici->IciL_QuantLevelsPerComp > (1 << ici->IciL_BitsPerComp) )
		ChfSignal( 5, ImgX_INVATRVAL, 3, Img_QuantLevelsPerComp,
			    ici->IciL_QuantLevelsPerComp, index );
	    }

	++ici;
	} /* end for */
    } /* end if */


if ( fat->FatL_PixelGroupSize == 0 )
    ChfSignal( 4, ImgX_INVZERVAL, 2, Img_PixelGroupSize, NOINDEX );


if ( !(flags & ImgM_NoDataPlaneVerify) )
    {
    if ( fat->FatL_TotalBitsPerPixel == 0 )
	ChfSignal( 4, ImgX_INVZERVAL, 2, Img_TotalBitsPerPixel, NOINDEX );
    }

if ( !(flags & ImgM_NoDataPlaneVerify) )
    {
    ice = fat->FatR_Ice;
    ice_cnt = fat->FatL_IceCnt;
    for ( ice_idx = 0; ice_idx < ice_cnt; ++ice_idx, ++ice )
	{
	idu = ice->IceR_Idu;
	idu_cnt = ice->IceL_IduCnt;
	if ( idu_cnt != fat->FatL_PlanesPerPixel )
	    ChfSignal( 4, ImgX_INVIDUCNT, 2, idu_cnt, ice_idx );
	total_plane_bits_per_pixel = 0;
	for ( idu_idx = 0; idu_idx < idu_cnt; ++idu_idx, ++idu )
	    {
	    if ( idu->IduL_NumberOfLines == 0 ) /* REQUIRED BY DDIF	*/
		ChfSignal( 4, ImgX_INVZERVAL, 2, Img_NumberOfLines, idu_idx );

	    if ( idu->IduL_PixelStride == 0 )   
		ChfSignal( 4, ImgX_INVZERVAL, 2, Img_PixelStride, idu_idx );

	    if ( idu->IduL_PixelsPerLine == 0 ) /* REQUIRED BY DDIF	*/
		ChfSignal( 4, ImgX_INVZERVAL, 2, Img_PixelsPerLine, idu_idx );

	    if ( idu->IduL_PlaneBitsPerPixel == 0 )
		ChfSignal( 4, ImgX_INVZERVAL, 2, Img_PlaneBitsPerPixel, idu_idx );

	    if ( fat->FatL_CompSpaceOrg == ImgK_BitIntrlvdByPlane &&
		 idu->IduL_PlaneBitsPerPixel != 1 )
		ChfSignal( 5, ImgX_INVATRVAL, 3, Img_PlaneBitsPerPixel,
		    idu->IduL_PlaneBitsPerPixel, idu_idx );

	    total_plane_bits_per_pixel += idu->IduL_PlaneBitsPerPixel;

	    if ( idu->IduL_ScanlineStride == 0 )
		ChfSignal( 4, ImgX_INVZERVAL, 2, Img_ScanlineStride, idu_idx );
	    } /* end for */

	if ( total_bits_per_comp != total_plane_bits_per_pixel )
	    ChfSignal( 4, ImgX_INCBPPTOT, 2, total_bits_per_comp, 
				    total_plane_bits_per_pixel );
	} /* end for */
    } /* end if */

/*
** Verify enumerated items
*/
if ( !(flags & ImgM_NoDataPlaneVerify) )
    {
    ice = fat->FatR_Ice;
    for ( ice_idx = 0; ice_idx < fat->FatL_IceCnt; ++ice_idx )
	{
	idu = ice->IceR_Idu;
	for ( idu_idx = 0; idu_idx < ice->IceL_IduCnt; ++idu_idx )
	    {
	    switch ( idu->IduL_BitOrder )
		{
		case ImgK_LsbitFirst:
		case ImgK_MsbitFirst:
		    break;
		default:
		    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_BitOrder, 
				idu->IduL_BitOrder, idu_idx );
		} /* end switch */

	    switch ( idu->IduL_ByteOrder )
		{
		case ImgK_LsbyteFirst:
		case ImgK_MsbyteFirst:
		    break;
		default:
		    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_ByteOrder, 
				idu->IduL_ByteOrder, idu_idx );

		    break;
		} /* end switch */

	    switch ( idu->IduL_DataType )
		{
		case ImgK_DataTypePrivate:
		case ImgK_DataTypeBitstream:
		case ImgK_DataTypeInteger:
		    break;
		default:
		    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_DataType,
				idu->IduL_DataType, idu_idx );
		    break;
		} /* end switch */

	    switch ( idu->IduL_Dtype )
		{
		case ImgK_DTypeVU:
		case ImgK_DTypeV:
		case ImgK_DTypeBitstream:
		case ImgK_DTypeIntBit:
		case ImgK_DTypeIntBits:
		case ImgK_DTypeIntBitsU:
		case ImgK_DTypeIntByte:
		case ImgK_DTypeIntByteU:
		case ImgK_DTypeIntWord:
		case ImgK_DTypeIntWordU:
		case ImgK_DTypeIntLongword:
		case ImgK_DTypeIntLongwordU:
		case ImgK_DTypeFloat:
		case ImgK_DTypeFloatD:
		case ImgK_DTypeComplex:
		case ImgK_DTypeComplexD:
		    break;
		default:
		    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_Dtype,
				idu->IduL_Dtype, idu_idx );
		    break;
		} /* end switch */
	    ++idu;
	    } /* end for */
	++ice;
	} /* end for */
    } /* end if */

switch ( fat->FatL_BrtPolarity )
    {
    case ImgK_ZeroMaxIntensity:
    case ImgK_ZeroMinIntensity:
	break;
    default:
	ChfSignal( 5, ImgX_INVATRVAL, 3, Img_BrtPolarity, 
		    fat->FatL_BrtPolarity, NOINDEX );
    }

switch ( fat->FatL_CompSpaceOrg )
    {
    case ImgK_BandIntrlvdByPixel:
    case ImgK_BandIntrlvdByPlane:
    case ImgK_BitIntrlvdByPlane:
    case ImgK_BandIntrlvdByLine:
	break;
    default:
	ChfSignal( 5, ImgX_INVATRVAL, 3, Img_CompSpaceOrg, 
		    fat->FatL_CompSpaceOrg, NOINDEX );
    }

switch ( fat->FatL_GridType )
    {
    case ImgK_RectangularGrid:
    case ImgK_HexEvenIndent:
    case ImgK_HexOddIndent:
	break;
    default:
	ChfSignal( 5, ImgX_INVATRVAL, 3, Img_GridType, 
		    fat->FatL_GridType, NOINDEX );
    }

switch ( fat->FatL_FrmBoxLLXC )
    {
#if defined(NEW_CDA_SYMBOLS)
    case DDIF_K_VALUE_CONSTANT:
#else
    case DDIF$K_VALUE_CONSTANT:
#endif
	break;
    default:
	ChfSignal( 3, ImgX_VALNOTCON, 1, Img_FrmBoxLLXC );
    }

switch ( fat->FatL_FrmBoxLLYC )
    {
#if defined(NEW_CDA_SYMBOLS)
    case DDIF_K_VALUE_CONSTANT:
#else
    case DDIF$K_VALUE_CONSTANT:
#endif
	break;
    default:
	ChfSignal( 3, ImgX_VALNOTCON, 1, Img_FrmBoxLLYC );
    }

switch ( fat->FatL_FrmBoxURXC )
    {
#if defined(NEW_CDA_SYMBOLS)
    case DDIF_K_VALUE_CONSTANT:
#else
    case DDIF$K_VALUE_CONSTANT:
#endif
	break;
    default:
	ChfSignal( 3, ImgX_VALNOTCON, 1, Img_FrmBoxURXC );
    }

switch ( fat->FatL_FrmBoxURYC )
    {
    case ImgK_ValueConstant:
	break;
    default:
	ChfSignal( 3, ImgX_VALNOTCON, 1, Img_FrmBoxURYC );
    }

switch ( fat->FatL_FrmPositionC )
    {
#if defined(NEW_CDA_SYMBOLS)
    case DDIF_K_FRAME_FIXED:
#else
    case DDIF$K_FRAME_FIXED:
#endif
	break;
    default:
	ChfSignal( 1, ImgX_FRMNOTFXD );
    }

switch ( fat->FatL_FrmfxdPositionXC )
    {
#if defined(NEW_CDA_SYMBOLS)
    case DDIF_K_VALUE_CONSTANT:
#else
    case DDIF$K_VALUE_CONSTANT:
#endif
	break;
    default:
	ChfSignal( 3, ImgX_VALNOTCON, 1, Img_FrmFxdPositionXC );
    }

switch ( fat->FatL_FrmfxdPositionYC )
    {
#if defined(NEW_CDA_SYMBOLS)
    case DDIF_K_VALUE_CONSTANT:
#else
    case DDIF$K_VALUE_CONSTANT:
#endif
	break;
    default:
	ChfSignal( 3, ImgX_VALNOTCON, 1, Img_FrmFxdPositionYC );
    }

switch ( fat->FatL_ImageDataClass )
    {
    case ImgK_ClassPrivate:
    case ImgK_ClassBitonal:
    case ImgK_ClassGreyscale:
    case ImgK_ClassMultispect:
	break;
    default:
	ChfSignal( 5, ImgX_INVATRVAL, 3, Img_ImageDataClass,
		    fat->FatL_ImageDataClass, NOINDEX );
    }


switch ( fat->FatL_PixelGroupOrder )
    {
    case ImgK_StandardPixelOrder:
    case ImgK_ReversePixelOrder:
	break;
    default:
	ChfSignal( 5, ImgX_INVATRVAL, 3, Img_PixelGroupOrder,
		    fat->FatL_PixelGroupOrder, NOINDEX );
    }


switch ( fat->FatL_PlaneSignif )
    {
    case ImgK_LsbitFirst:
    case ImgK_MsbitFirst:
	break;
    default:
	ChfSignal( 5, ImgX_INVATRVAL, 3, Img_PlaneSignif, 
		    fat->FatL_PlaneSignif, NOINDEX );
    }

switch ( fat->FatL_SpectralMapping )
    {
    case ImgK_PrivateMap:
    case ImgK_MonochromeMap:
    case ImgK_GeneralMap:
    case ImgK_ColorMap:
    case ImgK_RGBMap:
    case ImgK_CMYMap:
    case ImgK_YUVMap:
    case ImgK_HSVMap:
    case ImgK_HLSMap:
    case ImgK_YIQMap:
	break;
    default:
	ChfSignal( 5, ImgX_INVATRVAL, 3, Img_SpectralMapping,
		    fat->FatL_SpectralMapping, NOINDEX );
    }

/*
** Verify that all the right quantities match up.
*/
switch ( fat->FatL_SpectralMapping )
    {
    case ImgK_PrivateMap:
    case ImgK_GeneralMap:
	break;
    case ImgK_MonochromeMap:
    case ImgK_ColorMap:
	if ( fat->FatL_NumberOfComp != 1 )
	    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_NumberOfComp, 
			fat->FatL_NumberOfComp, NOINDEX );
	break;
    case ImgK_RGBMap:
    case ImgK_CMYMap:
    case ImgK_YUVMap:
    case ImgK_HSVMap:
    case ImgK_HLSMap:
    case ImgK_YIQMap:
	if ( fat->FatL_NumberOfComp != 3 )
	    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_NumberOfComp, 
			fat->FatL_NumberOfComp, NOINDEX );
	break;
    default:
	break;
    }

switch ( fat->FatL_CompSpaceOrg )
    {
    case ImgK_BandIntrlvdByPixel:
    case ImgK_BandIntrlvdByLine:
	if (fat->FatL_PlanesPerPixel != 1 )
	    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_PlanesPerPixel,
			fat->FatL_PlanesPerPixel, NOINDEX );
	break;
    case ImgK_BitIntrlvdByPlane:
	if (fat->FatL_PlanesPerPixel != fat->FatL_TotalBitsPerPixel )
	    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_PlanesPerPixel,
			fat->FatL_PlanesPerPixel, NOINDEX );
	break;
    case ImgK_BandIntrlvdByPlane:
	if (fat->FatL_PlanesPerPixel != fat->FatL_NumberOfComp )
	    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_PlanesPerPixel,
			fat->FatL_PlanesPerPixel, NOINDEX );
	break;
    }

if ( !(flags & ImgM_NoDataPlaneVerify) )
	{
	if ( fat->FatL_PlanesPerPixel > fat->FatL_TotalBitsPerPixel )
	ChfSignal( 4, ImgX_INVPLNCNT, 2, fat->FatL_PlanesPerPixel,
		    fat->FatL_TotalBitsPerPixel );
	}

/*
** Verify all the strides and compression type values.
*/
if ( !(flags & ImgM_NoDataPlaneVerify) )
    {
    ice = fat->FatR_Ice;
    ice_cnt = fat->FatL_IceCnt;
    for ( ice_idx = 0; ice_idx < ice_cnt; ++ice_idx, ++ice )
	{
	idu = ice->IceR_Idu;
	idu_cnt = ice->IceL_IduCnt;
	for ( idu_idx = 0; idu_idx < idu_cnt; ++idu_idx, ++idu )
	    {

	    switch ( idu->IduL_CompressionType )
		{
		case ImgK_PrivateCompression:
		case ImgK_PcmCompression:
		case ImgK_G31dCompression:
		case ImgK_G32dCompression:
		case ImgK_G42dCompression:
		case ImgK_DctCompression:
		    break;
		default:
		    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_CompressionType, 
				idu->IduL_CompressionType, idu_idx );
		} /* end switch */

	    if ( idu->IduL_PixelStride < 
		 idu->IduL_PlaneBitsPerPixel )
		ChfSignal( 4, ImgX_INVPXLSTR, 2, idu->IduL_PixelStride, 
			    idu_idx );

	    if ( idu->IduL_ScanlineStride < 
		 (idu->IduL_PixelsPerLine * idu->IduL_PixelStride) )
		ChfSignal( 4, ImgX_INVSCNSTR, 2, idu->IduL_ScanlineStride,
			    idu_idx);

	    if ( idu->IduL_CompressionType == ImgK_PcmCompression &&
		 idu->IduL_DataPlaneSize != 0 )
		{
		long	all_but_last_line_span;
		long	last_line_span;
		long	total_bit_span;
		long	total_byte_span;

		all_but_last_line_span = idu->IduL_DataOffset +
					    (idu->IduL_ScanlineStride *
						(idu->IduL_NumberOfLines-1));
		last_line_span = idu->IduL_PixelsPerLine * 
				    idu->IduL_PixelStride;

		total_bit_span = all_but_last_line_span + last_line_span;
		total_byte_span = (total_bit_span + 7)/8;

		if ( idu->IduL_DataPlaneSize < total_byte_span  )
		    ChfSignal( 4, ImgX_INSDPSIZE, 2, idu->IduL_DataPlaneSize,
				idu_idx );

		} /* end if */
	    } /* end for */
	} /* end for */
    } /* end if */

/*
** Miscellaneous comparisions
*/
if ( fat->FatL_FrmBoxURX < fat->FatL_FrmBoxLLX )
    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_FrmBoxURX, 
		fat->FatL_FrmBoxURX, NOINDEX );

if ( fat->FatL_FrmBoxURY < fat->FatL_FrmBoxLLY )
    ChfSignal( 5, ImgX_INVATRVAL, 3, Img_FrmBoxURX, 
		fat->FatL_FrmBoxURY, NOINDEX );

return;
} /* end of _ImgVerifyFat */


/******************************************************************************
**  Allocate_fat()
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static struct FAT *Allocate_fat( component_cnt, ice_cnt, plane_cnt )
long	component_cnt;
long	ice_cnt;
long	plane_cnt;
{
long	     compcnt;
long	     icecnt;
long	     index;
long	     planecnt;

struct FAT  *fat;
struct ICE  *ice;

/*
** Allocate the block.
*/
fat = (struct FAT *) _ImgCalloc( 1, FatK_Size );

/*
** Allocate image component info descriptors 
*/
if ( component_cnt > 0 )
    compcnt = component_cnt;
else
    compcnt = 1;
fat->FatR_Ici = (struct ICI *) _ImgCalloc( compcnt, IciK_Size );

/*
** Allocate ICE descriptors
*/
if ( ice_cnt == 0 )
    icecnt = 1;
else
    icecnt = ice_cnt;
ice = (struct ICE *) _ImgCalloc( icecnt, IceK_Size );
fat->FatR_Ice = ice;
fat->FatL_IceCnt = icecnt;

/*
** Allocate image data unit descriptors for each ICE
*/
if ( plane_cnt > 0 )
    planecnt = plane_cnt;
else
    planecnt = 1;

fat->FatL_PlanesPerPixel = plane_cnt;

for ( index = 0; index < icecnt; ++index )
    {
    ice->IceR_Idu = (struct IDU *) _ImgCalloc( planecnt, IduK_Size );
    ice->IceL_IduCnt = planecnt;
    ++ice;
    }

return fat;
} /* end of Allocate_fat */


/******************************************************************************
**  Apply_defaults()
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static struct FAT *Apply_defaults( fat, image_data_class )
struct FAT  *fat;
long	     image_data_class;
{

Apply_itmlst_to_fat( fat, General_defaults );
switch ( image_data_class )
    {
    case ImgK_ClassBitonal:
	Apply_itmlst_to_fat( fat, Bitonal_defaults );
	break;
    case ImgK_ClassGreyscale:
	Apply_itmlst_to_fat( fat, Greyscale_defaults );
	break;
    case ImgK_ClassMultispect:
	Apply_itmlst_to_fat( fat, Multispect_defaults );
	break;
    case ImgK_ClassPrivate:
	Apply_itmlst_to_fat( fat, Private_defaults );
	break;
    default:
	break;
    }

return fat;
} /* end of Apply_defaults */


/******************************************************************************
**  Apply_itmlst_to_fat()
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static void Apply_itmlst_to_fat( fat, itmlst )
struct FAT	*fat;
struct ITMLST	*itmlst;
{
long		     ice_cnt;
long		     ice_idx;
long		     idu_cnt;
long		     idu_idx;
long		     item_code;
long		     item_index;
long		     item_value;

struct ICE	    *ice    = fat->FatR_Ice;
struct ICI	    *ici    = fat->FatR_Ici;
struct IDU	    *idu;
struct ITMLST	    *item   = itmlst;

for ( ; item->ItmL_Code != 0; ++item )
    {
    /*
    ** Index range check.
    */
    item_code = item->ItmL_Code;
    item_index = item->ItmL_Index;
    if ( item->ItmA_Buffer != 0 )
	item_value = *((long *)item->ItmA_Buffer);
    else
	continue;   /* jump back to top of loop	*/
    switch ( item_code )
	{
	case Img_BitsPerComp:
	case Img_QuantLevelsPerComp:
	    if ( fat->FatL_NumberOfComp > 0 &&
		 item_index > fat->FatL_NumberOfComp - 1 )
		ChfStop( 4, ImgX_IDXRNGEXC, 2, item_code, item_index );
	    break;
	case Img_BitOrder:
	case Img_ByteOrder:
	case Img_ByteUnit:
	case Img_Cdp:
	case Img_CdpPrsnt:
	case Img_CompressionParams:
	case Img_CompressionType:
	case Img_DataOffset:
	case Img_DataPlaneBase:
	case Img_DataPlaneBitSize:
	case Img_DataPlaneSize:
	case Img_DataType:
	case Img_NumberOfLines:
	case Img_PixelStride:
	case Img_PixelsPerLine:
	case Img_PlaneBitsPerPixel:
	case Img_ScanlineStride:
	case Img_Udp:
	    if ( fat->FatL_PlanesPerPixel > 0 &&
		 item_index > fat->FatL_PlanesPerPixel - 1 )
		ChfStop( 4, ImgX_IDXRNGEXC, 2, item_code, item_index );
	    break;
	case Img_UserLabel:
	    break;
	default:
	    break;
	}

    /*
    ** put the item list value into the master item value list
    */
    switch ( item_code )
	{
	case Img_BitsPerComp:
	    fat->FatR_Ici[item_index].IciL_BitsPerComp = item_value;
	    break;
	case Img_BitOrder:
	    ice->IceR_Idu[item_index].IduL_BitOrder = item_value;
	    break;
	case Img_BrtPolarity:
	    fat->FatL_BrtPolarity = item_value;
	    break;
	case Img_ByteOrder:
	    ice->IceR_Idu[item_index].IduL_ByteOrder = item_value;
	    break;
	case Img_ByteUnit:
	    ice->IceR_Idu[item_index].IduL_ByteUnit = item_value;
	    break;
	case Img_CompSpaceOrg:
	    fat->FatL_CompSpaceOrg = item_value;
	    break;
	case Img_CompressionType:
	    ice->IceR_Idu[item_index].IduL_CompressionType = item_value;
	    break;
	case Img_DataOffset:
	    ice->IceR_Idu[item_index].IduL_DataOffset = item_value;
	    break;
	case Img_DataType:
	    ice->IceR_Idu[item_index].IduL_DataType = item_value;
	    break;
	case Img_GridType:
	    fat->FatL_GridType = item_value;
	    break;
	case Img_FrmBoxLLX:
	    fat->FatL_FrmBoxLLX = item_value;
	    break;
	case Img_FrmBoxLLXC:
	    fat->FatL_FrmBoxLLXC = item_value;
	    break;
	case Img_FrmBoxLLY:
	    fat->FatL_FrmBoxLLY = item_value;
	    break;
	case Img_FrmBoxLLYC:
	    fat->FatL_FrmBoxLLYC = item_value;
	    break;
	case Img_FrmBoxURX:
	    fat->FatL_FrmBoxURX = item_value;
	    break;
	case Img_FrmBoxURXC:
	    fat->FatL_FrmBoxURXC = item_value;
	    break;
	case Img_FrmBoxURY:
	    fat->FatL_FrmBoxURY = item_value;
	    break;
	case Img_FrmBoxURYC:
	    fat->FatL_FrmBoxURYC = item_value;
	    break;
	case Img_FrmPositionC:
	    fat->FatL_FrmPositionC = item_value;
	    break;
	case Img_FrmFxdPositionX:
	    fat->FatL_FrmfxdPositionX = item_value;
	    break;
	case Img_FrmFxdPositionXC:
	    fat->FatL_FrmfxdPositionXC = item_value;
	    break;
	case Img_FrmFxdPositionY:
	    fat->FatL_FrmfxdPositionY = item_value;
	    break;
	case Img_FrmFxdPositionYC:
	    fat->FatL_FrmfxdPositionYC = item_value;
	    break;
	case Img_LineProgression:
	    fat->FatL_LineProgression = item_value;
	    break;
	case Img_LPPixelDist:
	    fat->FatL_LpPixelDist = item_value;
	    break;
	case Img_NumberOfComp:
	    fat->FatL_NumberOfComp = item_value;
	    break;
	case Img_NumberOfLines:
	    ice->IceR_Idu[item_index].IduL_NumberOfLines = item_value;
	    break;
	case Img_PixelGroupOrder:
	    fat->FatL_PixelGroupOrder = item_value;
	    break;
	case Img_PixelGroupSize:
	    fat->FatL_PixelGroupSize = item_value;
	    break;
	case Img_PixelPath:
	    fat->FatL_PixelPath = item_value;
	    break;
	case Img_PixelStride:
	    ice->IceR_Idu[item_index].IduL_PixelStride = item_value;
	    break;
	case Img_PixelsPerLine:
	    ice->IceR_Idu[item_index].IduL_PixelsPerLine = item_value;
	    break;
	case Img_PlaneBitsPerPixel:
	    ice->IceR_Idu[item_index].IduL_PlaneBitsPerPixel = item_value;
	    break;
	case Img_PlaneSignif:
	    fat->FatL_PlaneSignif = item_value;
	    break;
	case Img_PlanesPerPixel:
	    fat->FatL_PlanesPerPixel = item_value;
	    break;
	case Img_PPPixelDist:
	    fat->FatL_PpPixelDist = item_value;
	    break;
	case Img_QuantLevelsPerComp:
	    fat->FatR_Ici[item_index].IciL_QuantLevelsPerComp = item_value;
	    break;
	case Img_ScanlineStride:
	    ice->IceR_Idu[item_index].IduL_ScanlineStride = item_value;
	    break;
	case Img_SpectralMapping:
	    fat->FatL_SpectralMapping = item_value;
	    break;
	case Img_UserField:
	    fat->FatL_UserField = item_value;
	    break;
	case Img_UserLabel:
	    if ( item_index >= fat->FatL_UserLabelCnt )
		/*
		** Extend the label list by one and add it as the last
		** item.
		*/
		{
		char	**user_label_strptrs;
		long	  index = fat->FatL_UserLabelCnt;

		++fat->FatL_UserLabelCnt;
		user_label_strptrs = fat->FatA_UserLabelStrptrs;
		user_label_strptrs = (char **)_ImgRealloc( 
					    (char *)user_label_strptrs, 
					    (LONGSIZE*fat->FatL_UserLabelCnt) );
		fat->FatA_UserLabelStrptrs = user_label_strptrs;

		user_label_strptrs[ index ] = 
				    _ImgCalloc( 1, item->ItmL_Length + 1 );

		memcpy( user_label_strptrs[index], item->ItmA_Buffer,
			    item->ItmL_Length );
		}
	    else
		{
		char	 *user_label_str;
		char	**user_label_strptrs;
		long	  index = item->ItmL_Index;
		/*
		** Replace the entry that exists.
		*/
		user_label_strptrs = fat->FatA_UserLabelStrptrs;
		user_label_str = user_label_strptrs[ index ];
		user_label_str = _ImgRealloc( user_label_str, 
						item->ItmL_Length + 1 );
		user_label_strptrs[ index ] = user_label_str;
		memcpy( user_label_str, item->ItmA_Buffer, item->ItmL_Length );
		}
	default:
	    break;
	} /* end switch */
    } /* end for */
return;
} /* end of Apply_itmlst_to_fat */


/******************************************************************************
**  Copy_user_label_strs()
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static void Copy_user_label_strs( fat_in, fat_out )
struct FAT  *fat_in;
struct FAT  *fat_out;
{
char	 *in_user_label_str;
char	**in_user_label_strptrs;
char	 *out_user_label_str;
char	**out_user_label_strptrs;
long	  str_cnt;
long	  str_idx;
long	  str_len;

str_cnt = fat_in->FatL_UserLabelCnt;

if ( str_cnt != 0 )
    {
    /*
    ** Set up input and output string pointer arrays
    */
    in_user_label_strptrs = fat_in->FatA_UserLabelStrptrs;
    out_user_label_strptrs = (char **) _ImgMalloc( str_cnt * LONGSIZE );
    fat_out->FatA_UserLabelStrptrs = out_user_label_strptrs;

    /*
    ** Copy user label string(s)
    */
    for ( str_idx = 0; str_idx < str_cnt; ++str_idx )
	{
	/*
	** Get address and length of user label string
	*/
	in_user_label_str = *in_user_label_strptrs;
	str_len = strlen( in_user_label_str );

	/*
	** Allocate memory for output string, and copy input string.
	*/
	out_user_label_str = (char *) _ImgCalloc( str_len + 1, 1 );
	*out_user_label_strptrs = out_user_label_str;
	memcpy( out_user_label_str, in_user_label_str, str_len );

	++in_user_label_strptrs;
	++out_user_label_strptrs;
	}
    }

return;
} /* end of Copy_user_label_strs */


/******************************************************************************
**  Deallocate_fat 
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static void Deallocate_fat( fat )
struct FAT  *fat;
{
long	ice_cnt	= fat->FatL_IceCnt;
long	ice_idx;
long	idu_cnt;
long	idu_idx;

long	index;

struct ICE  *ice    = fat->FatR_Ice;
struct IDU  *idu;

/*
** Free the image component info memory
*/
_ImgCfree( fat->FatR_Ici );

/*
** Free all the dynamic memory associated with and used by
** all the IDUs and all the ICEs.
*/
for ( ice_idx = 0; ice_idx < ice_cnt; ++ice_idx )
    {
    idu = ice->IceR_Idu;
    idu_cnt = ice->IceL_IduCnt;
    for ( idu_idx = 0; idu_idx < idu_cnt; ++idu_idx )
	{
	/*
	** Free dynamic memory associated with the IDU
	*/
	if ( idu->IduR_CompressionParams != 0 )
	    _ImgCfree( idu->IduR_CompressionParams );
	}

    /*
    ** Free all IDU's for the current ICE.
    */
    _ImgCfree( ice->IceR_Idu );
    ++ice;
    }
_ImgCfree( fat->FatR_Ice );

/*
** Deallocate all optional dynamic item value memory.
*/

/* free component wavelength info */

/* free lookup tables */

/*
** Deallocate user label storage
*/
if ( fat->FatL_UserLabelCnt != 0 )
    {
    char     *user_label_str;
    char    **user_label_strptrs;

    user_label_strptrs = fat->FatA_UserLabelStrptrs;
    
    for ( index = 0; index < fat->FatL_UserLabelCnt; ++index )
	{
	user_label_str = user_label_strptrs[ index ];
	_ImgCfree( user_label_str );
	}
    _ImgCfree( user_label_strptrs );
    }

/*
** Deallocate the master item-value list block.
*/
_ImgCfree( fat );

return;
} /* end of Deallocate_fat */
