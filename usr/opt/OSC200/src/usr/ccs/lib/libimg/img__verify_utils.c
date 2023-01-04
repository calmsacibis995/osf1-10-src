
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
**  IMG__VERIFY_UTILS
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains routines for verifing the "correctness" of an
**	image frame (or definition) structure and its component blocks. 
**
**	Nonaggregate fields within the component block which have predictable 
**	contents are verified. Any directly linked auxillary structures 
**	(including lists) are themselves verified. The philosophy is that 
**	correctness of structures directly linked to a component block 
**	determine the correctness of the block. Therefore, verifing the top 
**	level block implies verifing the entire structure.
**
**  ENVIRONMENT:
**
**      VAX/VMS, Ultrix/VAX, Ultrix/RISC, OSF1/RISC
**
**  AUTHOR(S):
**
**	Mark W. Sornson 
**
**  CREATION DATE:     
**
**	5-OCT-1989
**
************************************************************************/

/*
**  Include files
*/
#include    <ctype.h>
#include    <stdlib.h>
#include    <string.h>

#include    <img/ChfDef.h>		/* Condition handler definitions */
#include    <img/ImgDef.h>
#include    <ImgDefP.h>
#include    <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include <ddif$def.h>
#include <cda$msg.h>
#include <cda$ptp.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include <ddifdef.h>
#include <cdamsg.h>
#else
#include <ddif_def.h>
#include <cda_msg.h>
#endif
#if defined(NEW_CDA_CALLS)
#include <cdaptp.h>
#else
#include <cda_ptp.h>
#endif
#endif

/*
**  Table of Contents
**
**	Global Entry Points
*/
#ifdef NODAS_PROTO
long	_ImgCheckNormal();
long	_ImgGetVerifyStatus();
long	_ImgVerifyAttributes();
long	_ImgVerifyDataPlanes();
long	_ImgVerifyNativeFormat();
long	_ImgVerifyStandardFormat(); /* replaces _ImgVerifyNativeFormat	*/
long	_ImgVerifyStructure();
#endif

/*
**  MACRO definitions
**
**	none
*/

/*
**  Equated Symbols
*/


    /*
    **	Structure for storing item code along with the expected value in the
    **	"normal form" case.
    */
struct NATIVE_FORMAT_ITMLST
    {
    long    item_code;
    long    item_value;
    };


/*
**  External routine references
**
**  Chf references done in <img/ChfDef.h>
*/
#ifdef NODAS_PROTO
void	    ImgResetCtx();
void	    ImgRestoreCtx();
void	    ImgSaveCtx();

char	    *_ImgCalloc();
void	     _ImgCfree();
void	     _ImgDeleteFat();
struct FAT  *_ImgExtractFat();
struct FCT  *_ImgGet();	
long	     _ImgNextContentElement();
long	     _ImgVerifyDataPlane();
long	     _ImgVerifyFat();
#endif
/*
**	Module local entry points
*/
#ifdef NODAS_PROTO
static long Verify_special_conditions();
static long Verify_standard_attrs();
#else
PROTO(static long Verify_special_conditions, (struct FCT */*fid*/, long /*flags*/));
PROTO(static long Verify_standard_attrs, (struct FCT */*fid*/, struct NATIVE_FORMAT_ITMLST */*itmlst*/, long /*flags*/));
#endif

/*
**  External symbol definitions (for status codes)
*/
#include    <img/ImgStatusCodes.h>

/*
**  Local Storage
**
**  Item lists for Native Format attribute values..
**
*/
#if defined(VAXC) || defined(__VAXC)
static readonly struct NATIVE_FORMAT_ITMLST common_attrs_itmlst[] = 
#else
struct NATIVE_FORMAT_ITMLST common_attrs_itmlst[] = 
#endif
    {
/*     {Img_CompSpaceOrg,	    ImgK_BandIntrlvdByPlane }	*/
     {Img_GridType,	    ImgK_RectangularGrid }
    ,{Img_LineProgression,  ImgK_TopToBottom }
/*    ,{Img_LPPixelDist,	    1 }			*/
    ,{Img_PixelPath,	    ImgK_LeftToRight }
    ,{Img_PixelGroupOrder,  ImgK_StandardPixelOrder }
    ,{Img_PixelGroupSize,   1 }
    ,{Img_PlaneSignif,	    ImgK_LsbitFirst }
/*    ,{Img_PPPixelDist,	    1 }			*/
    ,{0,0}
    };

#if defined(__VAXC) || defined(VAXC)
static readonly struct NATIVE_FORMAT_ITMLST bitonal_general_itmlst[] = 
#else
struct NATIVE_FORMAT_ITMLST bitonal_general_itmlst[] = 
#endif
    {
     {Img_ImageDataClass,	ImgK_ClassBitonal }
    ,{Img_NumberOfComp,		1 }
    ,{Img_PlanesPerPixel,	1 }
    ,{Img_SpectralMapping,	ImgK_MonochromeMap }
    ,{Img_TotalBitsPerPixel,	1 }
    ,{0,0}
    };

#if defined(__VAXC) || defined(VAXC)
static readonly struct NATIVE_FORMAT_ITMLST bitonal_idu_itmlst[] = 
#else
struct NATIVE_FORMAT_ITMLST bitonal_idu_itmlst[] = 
#endif
    {
     {Img_BitOrder,		ImgK_LsbitFirst }
    ,{Img_ByteOrder,		ImgK_LsbyteFirst }
    ,{Img_ByteUnit,		1 }
    ,{Img_CompressionType,	ImgK_PcmCompression }
/*    ,{Img_DataOffset,		0 }		    */
    ,{Img_DataType,		ImgK_DataTypeInteger }
    ,{Img_PixelAlignment,	ImgK_AlignBit }
    ,{Img_PixelStride,		1 }
    ,{Img_PlaneBitsPerPixel,	1 }
/*    ,{Img_ScanlineAlignment,	ImgK_AlignLongword }    */
    ,{0,0}
    };

#if defined(__VAXC) || defined(VAXC)
static readonly struct NATIVE_FORMAT_ITMLST greyscale_general_itmlst[] = 
#else
struct NATIVE_FORMAT_ITMLST greyscale_general_itmlst[] = 
#endif
    {
     {Img_ImageDataClass,	ImgK_ClassGreyscale }
    ,{Img_NumberOfComp,		1 }
    ,{Img_PlanesPerPixel,	1 }
    ,{Img_SpectralMapping,	ImgK_MonochromeMap }
/*    ,{Img_TotalBitsPerPixel,	8 }			*/
    ,{0,0}
    };

#if defined(__VAXC) || defined(VAXC)
static readonly struct NATIVE_FORMAT_ITMLST greyscale_idu_itmlst[] = 
#else
struct NATIVE_FORMAT_ITMLST greyscale_idu_itmlst[] = 
#endif
    {
     {Img_BitOrder,		ImgK_LsbitFirst }
    ,{Img_ByteOrder,		ImgK_LsbyteFirst }
    ,{Img_ByteUnit,		1 }
    ,{Img_CompressionType,	ImgK_PcmCompression }
/*    ,{Img_DataOffset,		0 }		    */
    ,{Img_DataType,		ImgK_DataTypeInteger }
/*    ,{Img_PixelAlignment,	ImgK_AlignByte }    */
/*    ,{Img_PixelStride,	8 } */
/*    ,{Img_PlaneBitsPerPixel,	8 } */
/*    ,{Img_ScanlineAlignment,	ImgK_AlignLongword }    */
    ,{0,0}
    };

#if defined(__VAXC) || defined(VAXC)
static readonly struct NATIVE_FORMAT_ITMLST multispect_general_itmlst[] = 
#else
struct NATIVE_FORMAT_ITMLST multispect_general_itmlst[] = 
#endif
    {
     {Img_CompSpaceOrg,		ImgK_BandIntrlvdByPlane }
    ,{Img_ImageDataClass,	ImgK_ClassMultispect }
    ,{Img_NumberOfComp,		3 }
    ,{Img_PlanesPerPixel,	3 }
    ,{Img_SpectralMapping,	ImgK_RGBMap }
/*    ,{Img_TotalBitsPerPixel,	24 }			*/
    ,{0,0}
    };

#if defined(__VAXC) || defined(VAXC)
static readonly struct NATIVE_FORMAT_ITMLST multispect_idu_itmlst[] = 
#else
struct NATIVE_FORMAT_ITMLST multispect_idu_itmlst[] = 
#endif
    {
     {Img_BitOrder,		ImgK_LsbitFirst }
    ,{Img_ByteOrder,		ImgK_LsbyteFirst }
    ,{Img_ByteUnit,		1 }
    ,{Img_CompressionType,	ImgK_PcmCompression }
/*    ,{Img_DataOffset,		0 }		    */
    ,{Img_DataType,		ImgK_DataTypeInteger }
/*    ,{Img_PixelAlignment,	ImgK_AlignByte }    */
/*    ,{Img_PixelStride,	8 } */
/*    ,{Img_PlaneBitsPerPixel,	8 } */
/*    ,{Img_ScanlineAlignment,	ImgK_AlignLongword }    */
    ,{0,0}
    };


/************************************************************************
**  _ImgCheckNormal - Check for normal frame format
** 
**  FUNCTIONAL DESCRIPTION:
**
**	This function verifies that the attributes in a given frame
**	are "normal", i.e., they meet a certain criteria for standardization
**	within each data class.
**
**	NOTE that this routine is obsolete, and has been replaced by
**	_ImgVerifyNativeFormat().
**
**  FORMAL PARAMETERS:
**
**      fid - Address of an FCT block
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      status	    longword, by value.
**		    Low bit set indicates verification completed successfully.
**
**  SIGNAL CODES:
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
long _ImgCheckNormal( fid )
struct FCT *fid;
{
long	status;
status = _ImgVerifyNativeFormat( fid, 0 );

return status;
} /* end of _ImgCheckNormal */


/******************************************************************************
**  _ImgGetVerifyStatus
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine checks the user's process environment (for a logical
**	under VMS and an environment variable under ULTRIX) to determine
**	whether frame verification should be globally turned on or off.
**
**	The logical or environment variable that's checked is:
**
**		IMG_VERIFY_STATUS
**
**	It may be left undefined, or given string values of:
**
**		"ON"	(Turned on everywhere, even when not user requiested)
**		"OFF"	(Turned off everywhere, including places which are
**			 normally turned on for internal reasons.)
**
**  FORMAL PARAMETERS:
**
**	none
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
**	verify_status	VERIFY_UNDEFINED
**			VERIFY_ON
**			VERIFY_OFF
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgGetVerifyStatus()
{
char	*environment_var		= "IMG_VERIFY_STATUS";
char	*env_symbol_trans;
char	*off				= "OFF";
char	*on				= "ON";
long	 env_symbol_len;
long	 index;
long	 match_status;

#if defined(__VAXC) || defined(VAXC)
noshare static long verify_initialized	= FALSE;
noshare static long verify_status	= VERIFY_UNDEFINED;
#else
static long verify_initialized		= FALSE;
static long verify_status		= VERIFY_UNDEFINED;
#endif

if ( !verify_initialized )
    {
    verify_initialized = TRUE;
    env_symbol_trans = (char *) getenv( environment_var );
    if ( env_symbol_trans != NULL )
	{
	env_symbol_len = strlen( env_symbol_trans );
	for ( index = 0; index < env_symbol_len; ++index )
	    {
	    env_symbol_trans[index] = toupper( (int)env_symbol_trans[index] );
	    }

	match_status = memcmp( env_symbol_trans, on, strlen( on ) );
	if ( match_status == 0 )
	    verify_status = VERIFY_ON;
	else
	    {
	    match_status = memcmp( env_symbol_trans, off, strlen( off ) );
	    if ( match_status == 0 )
		verify_status = VERIFY_OFF;
	    }
	}
    }

return verify_status;
} /* end of _ImgGetVerifyStatus */


/******************************************************************************
**  _ImgVerifyAttributes
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
long _ImgVerifyAttributes( fct, flags )
struct FCT	*fct;
unsigned long	 flags;
{
long	     status	= ImgX_SUCCESS;
struct FAT  *fat;

/*
** Do this check at all times!!!
*/
if ( fct == 0 )
    ChfStop( 1, ImgX_INVZERFID );

/*
** Allow further verification to be turned off ...
*/
if ( VERIFY_OFF_ )
    return status;

fat = _ImgExtractFat( fct );
_ImgVerifyFat( fat, flags );
_ImgDeleteFat( fat );

return status;
} /* end of _ImgVerifyAttributes */


/******************************************************************************
**  _ImgVerifyDataPlanes
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
long _ImgVerifyDataPlanes( fct )
struct FCT  *fct;
{
char	*dp_base;
long	 ice_cnt;
long	 ice_idx    = 0;
long	 idu_cnt;
long	 idu_idx;
long	 status	    = ImgX_SUCCESS;

/*
** Do this check at all times!!!
*/
if ( fct == 0 )
    ChfStop( 1, ImgX_INVZERFID );

/*
** Allow further verification to be turned off ...
*/
if ( VERIFY_OFF_ )
    return status;

_ImgGet( fct, Img_Icecnt, &ice_cnt, sizeof(long), 0, 0 );

/*ImgSaveCtx( fct );
*/
ImgResetCtx( fct );

for ( ; ice_idx < ice_cnt; ++ice_idx )
    {
    _ImgGet( fct, Img_IduCnt, &idu_cnt, sizeof(long), 0, ice_idx );
    idu_idx = 0;
    for ( ; idu_idx < idu_cnt; ++idu_idx )
	{
	_ImgGet( fct, Img_DataPlaneBase, &dp_base, sizeof(long), 0, idu_idx );
	if ( dp_base != 0 )
	    _ImgVerifyDataPlane( fct, dp_base, idu_idx );
	else
	    ChfSignal( 4, ImgX_NODATAPLA, 2, idu_idx, ice_idx );
	}
    _ImgNextContentElement( fct );
    }

/*ImgRestoreCtx( fct );
*/
return status;
} /* end of _ImgVerifyDataPlanes */


/******************************************************************************
**  _ImgVerifyNativeFormat
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
long _ImgVerifyNativeFormat( fid, flags )
struct FCT  *fid;
long	     flags;
{
long	ret_status  = ImgX_SUCCESS;

ret_status = _ImgVerifyStandardFormat( fid, flags );

return ret_status;
} /* end of_ImgVerifyNativeFormat */


/******************************************************************************
**  _ImgVerifyStandardFormat
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
long _ImgVerifyStandardFormat( fid, flags )
struct FCT  *fid;
long	     flags;
{
long	data_class;
long	status;
long	ret_status  = ImgX_SUCCESS;

/*
** Do this check at all times!!!
*/
if ( fid == 0 )
    ChfStop( 1, ImgX_INVZERFID );

/*
** Allow further verification to be turned off ...
*/
if ( VERIFY_OFF_ )
    return ret_status;

/*
** Verify that the attribute values that define standard format
** equal the attribute values of the frame (according to data class).
*/
status = Verify_standard_attrs( fid, common_attrs_itmlst, flags );
if ( (status&1) != 1 )
    ret_status = status;

_ImgGet( fid, Img_ImageDataClass, &data_class, sizeof(long), 0, 0 );
switch ( data_class )
    {
    case ImgK_ClassBitonal:
	status = Verify_standard_attrs( fid, bitonal_general_itmlst, flags );
	if ( (status&1) != 1 )
	    ret_status = status;
	status = Verify_standard_attrs( fid, bitonal_idu_itmlst, flags );
	if ( (status&1) != 1 )
	    ret_status = status;
	break;
    case ImgK_ClassGreyscale:
	status = Verify_standard_attrs( fid, greyscale_general_itmlst, flags );
	if ( (status&1) != 1 )
	    ret_status = status;
	status = Verify_standard_attrs( fid, greyscale_idu_itmlst, flags );
	if ( (status&1) != 1 )
	    ret_status = status;
	break;
    case ImgK_ClassMultispect:
	status = Verify_standard_attrs( fid, multispect_general_itmlst, flags );
	if ( (status&1) != 1 )
	    ret_status = status;
	status = Verify_standard_attrs( fid, multispect_idu_itmlst, flags );
	if ( (status&1) != 1 )
	    ret_status = status;

	if( (status&1) != 1 )
	    ret_status = status;
	break;
    case ImgK_ClassPrivate:
    default:
	ChfSignal( 1, ImgX_UNSSPCTYP);
	break;
    }

/*
** Verify special conditions that can't be checked by itemlist/itemcode
** comparisions alone.
*/
status = Verify_special_conditions( fid, flags );
if ( (status&1) != 1 )
    ret_status = status;

/*
** Set the Native Format flag in the frame context block.
*/
if ( (ret_status&1) != 1 )
    fid->FctL_Flags.FctV_NativeFormat = 0;
else
    fid->FctL_Flags.FctV_NativeFormat = 1;

return ret_status;
} /* end of_ImgVerifyStandardFormat */


/******************************************************************************
**  _ImgVerifyStructure
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
long _ImgVerifyStructure( fct )
struct FCT  *fct;
{
CDAconstant aggregate_item;
long	 img_aggr_valid	= TRUE;
CDAaddress item;
CDAagghandle ret_idu;
CDAagghandle ret_img;
long	 ret_len;
CDAagghandle ret_sga;
int	 status		= ImgX_SUCCESS;

/*
** Do this check at all times!!!
*/
if ( fct == 0 )
    ChfStop( 1, ImgX_INVZERFID );

if ( (long)fct == 0 )
    ChfStop( 1, ImgX_FRAMEDEAL );

/*
** Allow further verification to be turned off ...
*/
if ( VERIFY_OFF_ )
    return status;

/*
** Verify that required pointers in FCT are not zero.
*/
if (fct->FctL_RootAggr == 0)
    ChfSignal( 1, ImgX_NOROOTFOU);
if (fct->FctL_SegAggr == 0)
    ChfSignal( 1, ImgX_NOSEGFOU);
if (fct->FctL_SgaAggr == 0)
    ChfSignal( 1, ImgX_NOSGAFOU);

/*
** Verify that SEGment aggregate really points to the SGA 
** (segment attribute) aggregate.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_SEG_SPECIFIC_ATTRIBUTES;
#else
aggregate_item = DDIF$_SEG_SPECIFIC_ATTRIBUTES;
#endif
status = CDA_LOCATE_ITEM_(   &(fct->FctL_RootAggr),
			    &(fct->FctL_SegAggr),
			    &aggregate_item,
			    &item,
			    &ret_len,
			    0,0 );
LOWBIT_TEST_( status );

ret_sga = *(CDAagghandle *)item;
if (ret_sga != fct->FctL_SgaAggr)
    ChfSignal( 1, ImgX_NOSGAFOU);


/*
** Verify that the aggregates that actually point to data are
** valid if they appear to be present.
*/
if ( fct->FctL_IduAggr != 0 && fct->FctL_ImgAggr == 0 )
    ChfSignal( 1, ImgX_NOIMGFOU );

if ( fct->FctL_IduAggr != 0 && fct->FctL_ImgAggr != 0 )
    {
    /*
    ** Verify that the IMG aggr in the FCT is actually linked to
    ** the SEG aggr in the FCT.
    */
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_SEG_CONTENT;
#else
    aggregate_item = DDIF$_SEG_CONTENT;
#endif
    status = CDA_LOCATE_ITEM_(	&(fct->FctL_RootAggr),
				&(fct->FctL_SegAggr),
				&aggregate_item,
				&item,
				&ret_len,
				0,0 );
    LOWBIT_TEST_( status );
    ret_img = *(CDAagghandle *)item;
    while (TRUE)
	{
	if (ret_img == fct->FctL_ImgAggr)
	    {
	    img_aggr_valid = TRUE;
	    break;
	    }
	status = CDA_NEXT_AGGREGATE_( &ret_img, &ret_img );
#if defined(NEW_CDA_SYMBOLS)
	if (status == CDA_ENDOFSEQ)
#else
	if (status == CDA$_ENDOFSEQ)
#endif
	    {
	    ChfSignal( 1, ImgX_NOIMGFOU);
	    break;
	    }
	} /* end while */

    /*
    ** Verify that IDU aggr in the FCT is actually linked to the
    ** IMG aggr specified in the FCT.
    */
    if ( img_aggr_valid )
	{
#if defined(NEW_CDA_SYMBOLS)
	aggregate_item = DDIF_IMG_CONTENT;
#else
	aggregate_item = DDIF$_IMG_CONTENT;
#endif
	status = CDA_LOCATE_ITEM_(   &(fct->FctL_RootAggr),
				    &(fct->FctL_ImgAggr),
				    &aggregate_item,
				    &item,
				    &ret_len,
				    0,0 );
	ret_idu = *(CDAagghandle *)item;
	while (TRUE)
	    {
	    if (ret_idu == fct->FctL_IduAggr)
		break;
	    status = CDA_NEXT_AGGREGATE_(&ret_idu,&ret_idu);
#if defined(NEW_CDA_SYMBOLS)
	    if (status == CDA_ENDOFSEQ)
#else
	    if (status == CDA$_ENDOFSEQ)
#endif
		{
		ChfSignal( 1, ImgX_NOIDUFOU);
		break;
		}
	    } /* end while */
	} /* end if */
    } /* end if */

return status;
} /* _ImgVerifyStructure */


/******************************************************************************
**  Verify_special_conditions()
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
static long Verify_special_conditions( fid, flags )
struct FCT *fid;
long	flags;
{
long	*bits_per_comp;
long	 comp_idx;
long	 comp_space_org;
long	 data_class;
long	 dtype;
long	 ice_cnt;
long	 ice_idx;
long	 idu_cnt;
long	 idu_idx;
long	 lp_pixel_dist;
long	 number_of_comp;
long	 pixel_alignment;
long	 pixel_alignment_ok	= TRUE;
long	 plane_bits_per_pixel;
long	 pp_pixel_dist;
long	 rect_roi_info		= 0;
long	 ret_status		= ImgX_SUCCESS;
long	 scanline_alignment;
long	 scanline_alignment_ok	= TRUE;
long	 total_bits_per_pixel;

if ( VERIFY_OFF_ )
    return ret_status;

_ImgGet( fid, Img_ImageDataClass, &data_class, LONGSIZE, 0, 0 );
_ImgGet( fid, Img_CompSpaceOrg, &comp_space_org, LONGSIZE, 0, 0 );
_ImgGet( fid, Img_TotalBitsPerPixel, &total_bits_per_pixel, LONGSIZE, 0, 0 );

switch ( data_class )
    {
    case ImgK_ClassBitonal:
	break;
    case ImgK_ClassGreyscale:
	/*
	** Verify that the component space organization is *NOT* 
	** bit-interleaved-by-plane.
	*/
	if ( comp_space_org == ImgK_BitIntrlvdByPlane )
	    {
	    ret_status = ImgX_ATRNOTSTD;
	    if ( !(flags&ImgM_NoChf) )
		ChfSignal( 5, ImgX_ATRNOTSTD, 3, comp_space_org,
			    Img_CompSpaceOrg, 0 );

	    }
	/*
	** Verify that the total number of bits per pixel is an
	** 8 bit multiple.
	*/
	if ( (total_bits_per_pixel % 8 != 0) )
	    {
	    ret_status = ImgX_NONSTDBPP;
	    if ( !(flags&ImgM_NoChf) )
		ChfSignal( 1, ImgX_NONSTDBPP );
	    }
	break;
    case ImgK_ClassMultispect:
	/*
	** Verify that the total number of bits per pixel is an
	** 8 bit multiple.
	*/
	if ( (total_bits_per_pixel % 8 != 0) )
	    {
	    ret_status = ImgX_NONSTDBPP;
	    if ( !(flags&ImgM_NoChf) )
		ChfSignal( 1, ImgX_NONSTDBPP );
	    }
	/*
	** Verify that the number of bits per comp are equal
	*/
	_ImgGet( fid, Img_NumberOfComp, &number_of_comp, LONGSIZE, 0, 0 );
	bits_per_comp = (long *)_ImgCalloc( number_of_comp, LONGSIZE );
	for ( comp_idx = 0; comp_idx < number_of_comp; ++comp_idx )
	    {
	    _ImgGet( fid, Img_BitsPerComp, &bits_per_comp[comp_idx], LONGSIZE,
			0, comp_idx );
	    }

	for ( comp_idx = 1; comp_idx < number_of_comp; ++comp_idx )
	    {
	    if ( bits_per_comp[comp_idx] != bits_per_comp[comp_idx-1] )
		{
		ret_status = ImgX_BPCNOTEQL;
		if ( !(flags&ImgM_NoChf) )
		    ChfSignal( 6, ImgX_BPCNOTEQL, 4, 
				comp_idx-1, bits_per_comp[comp_idx-1],
				comp_idx, bits_per_comp[comp_idx] );
		}
	    }

	_ImgCfree( bits_per_comp );
	break;
    default:
	break;
    } /* end switch */

/*
** Verify that the pixel and scanline alignment are aligned
** by data type.
*/
/*ImgSaveCtx( fid );
*/
_ImgGet( fid, Img_Icecnt, &ice_cnt, LONGSIZE, 0, 0 );
_ImgGet( fid, Img_RectRoiInfo, &rect_roi_info, LONGSIZE, 0, 0 );
for ( ice_idx = 0; ice_idx < ice_cnt; ++ice_idx )
    {
    _ImgGet( fid, Img_IduCnt, &idu_cnt, LONGSIZE, 0, ice_idx );
    for( idu_idx = 0; idu_idx < idu_cnt; ++idu_idx )
	{
	_ImgGet( fid, Img_Dtype, &dtype, LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_PixelAlignment, &pixel_alignment,
		LONGSIZE, 0, idu_idx );
	_ImgGet( fid, Img_ScanlineAlignment, &scanline_alignment,
		LONGSIZE, 0, idu_idx );

	switch ( data_class )
	    {
	    case ImgK_ClassBitonal:
		/*
		** Pixels MUST be bit aligned for bitonal images
		*/
		if ( pixel_alignment != ImgK_AlignBit )
		    {
		    ret_status = ImgX_ATRNOTSTD;
		    if ( !(flags&ImgM_NoChf) )
			ChfSignal( 5, ImgX_ATRNOTSTD, 3, pixel_alignment,
				    Img_PixelAlignment, idu_idx );
		    }
		break;
	    case ImgK_ClassGreyscale:
	    case ImgK_ClassMultispect:
	    default:
		/*
		** Pixels must be aligned according to data type ...
		*/
		switch ( dtype )
		    {
		    case ImgK_DTypeIntByte:
		    case ImgK_DTypeIntByteU:
/*
**  Limit this check to byte aligned only for now, until
**  IPS supports pixel padding.
**			if ( pixel_alignment != ImgK_AlignByte &&
**			     pixel_alignment != ImgK_AlignWord &&
**			     pixel_alignment != ImgK_AlignLongword )
*/
			if ( pixel_alignment != ImgK_AlignByte )
			    pixel_alignment_ok = FALSE;
			break;
		    case ImgK_DTypeIntWord:
		    case ImgK_DTypeIntWordU:
/*
**  Limit this check to word aligned only for now, until
**  IPS supports pixel padding.
**			if ( pixel_alignment != ImgK_AlignWord &&
**			     pixel_alignment != ImgK_AlignLongword )
*/
			if ( pixel_alignment != ImgK_AlignWord )
			    pixel_alignment_ok = FALSE;
			break;
		    case ImgK_DTypeIntLongword:
		    case ImgK_DTypeIntLongwordU:
		    case ImgK_DTypeFloat:
			if ( pixel_alignment != ImgK_AlignLongword )
			    pixel_alignment_ok = FALSE;
			break;
		    default:
			break;
		    } /* end switch */

		if ( !pixel_alignment_ok )
		    {
		    ret_status = ImgX_ATRNOTSTD;
		    if ( !(flags&ImgM_NoChf) )
			ChfSignal( 5, ImgX_ATRNOTSTD, 3, pixel_alignment,
				    Img_PixelAlignment, idu_idx );
		    }
		break;
	    } /* end switch */

	/*
	** All scanlines must be aligned on the data type boundary,
	** with the exception of bitonal, which must be at least
	** byte aligned if no ROI has been set.
	*/
	switch ( data_class )
	    {
	    case ImgK_ClassBitonal:
		if ( rect_roi_info == 0 )
		    if ( scanline_alignment != ImgK_AlignByte &&
			 scanline_alignment != ImgK_AlignWord &&
			 scanline_alignment != ImgK_AlignLongword )
			scanline_alignment_ok = FALSE;
		break;
	    case ImgK_ClassGreyscale:
	    case ImgK_ClassMultispect:
	    default:
		switch ( dtype )
		    {
		    case ImgK_DTypeIntByte:
		    case ImgK_DTypeIntByteU:
			if ( scanline_alignment != ImgK_AlignByte && 
			     scanline_alignment != ImgK_AlignWord &&
			     scanline_alignment != ImgK_AlignLongword )
			    scanline_alignment_ok = FALSE;
			break;
		    case ImgK_DTypeIntWord:
		    case ImgK_DTypeIntWordU:
			if ( scanline_alignment != ImgK_AlignWord &&
			     scanline_alignment != ImgK_AlignLongword )
			    scanline_alignment_ok = FALSE;
			break;
		    case ImgK_DTypeIntLongword:
		    case ImgK_DTypeIntLongwordU:
		    case ImgK_DTypeFloat:
			if ( scanline_alignment != ImgK_AlignLongword )
			    scanline_alignment_ok = FALSE;
			break;
		    default:
			break;
		    } /* end switch */
		break;
	    }

	if ( !scanline_alignment_ok )
	    {
	    ret_status = ImgX_ATRNOTSTD;
	    if ( !(flags&ImgM_NoChf) )
		ChfSignal( 5, ImgX_ATRNOTSTD, 3, scanline_alignment,
			    Img_ScanlineAlignment, idu_idx );
	    }

	} /* end for */
    _ImgNextContentElement( fid );
    } /* end for */

/*ImgRestoreCtx( fid );
*/

/*
** Verify that the aspect ratio is square.
*/
_ImgGet( fid, Img_LPPixelDist, &lp_pixel_dist, LONGSIZE, 0, 0 );
_ImgGet( fid, Img_PPPixelDist, &pp_pixel_dist, LONGSIZE, 0, 0 );
if ( lp_pixel_dist != pp_pixel_dist )
    ret_status = ImgX_FAILURE;

/*
** Verify that the plane-bits-per-pixel attribute is byte aligned for
** continuous tone data
*/
switch ( data_class )
    {
    case ImgK_ClassMultispect:
    case ImgK_ClassGreyscale:
	for ( idu_idx = 0; idu_idx < idu_cnt; ++idu_idx )
	    {
	    _ImgGet( fid, Img_PlaneBitsPerPixel, &plane_bits_per_pixel,
			sizeof(plane_bits_per_pixel), 0, idu_idx );

	    if ( (plane_bits_per_pixel % 8) != 0 )
		{
		ret_status = ImgX_ATRNOTSTD;
		if ( !(flags&ImgM_NoChf) )
		    ChfSignal( 5, ImgX_ATRNOTSTD, 3, plane_bits_per_pixel,
				Img_PlaneBitsPerPixel, idu_idx );
		}
	    }
	break;
    case ImgK_ClassBitonal:
    default:
	break;
    }

return ret_status;
} /* end of Verify_special_conditions */


/******************************************************************************
**  Verify_standard_attrs()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Match the attributes in a frame with standardized (or expected)
**	values for each attribute.  This function verifies that image
**	attributes for a given image data class are standardized and
**	meet the requirements of the ISL Native Format (ISL-NF) specification.
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
static long Verify_standard_attrs( fid, itmlst, flags )
struct FCT		    *fid;
struct NATIVE_FORMAT_ITMLST *itmlst;
long			     flags;
{
long		idu_cnt;
long		idu_idx;
long		item_value;
struct ITEMCODE	item_code;
long		status;
long		ret_status	= ImgX_SUCCESS;

if ( VERIFY_OFF_ )
    return ret_status;

/*
** Go through each itmlst entry and get the attribute that is to be
** checked from the frame, and compare it's value to the standardized
** (or expected) value in the itmlst.
*/
_ImgGet( fid, Img_IduCnt, &idu_cnt, sizeof(long), 0, 0 );   /* 1st ICE only */
while ( itmlst->item_code != 0 )
    {
    item_value = 0;
    item_code = *((struct ITEMCODE*)&(itmlst->item_code));

    switch ( item_code.ItmV_StructType )
	{
	/*
	** Idu aggr item ... check the attribute for each idu
	*/
	case ImgK_IduAggrItem:
	    for ( idu_idx = 0; idu_idx < idu_cnt; ++idu_idx )
		{
		_ImgGet( fid, itmlst->item_code, &item_value, sizeof(long), 0, 
			    idu_idx );
		if ( item_value != itmlst->item_value)
		    {
		    ret_status = ImgX_FAILURE;
		    if ( !(flags&ImgM_NoChf) )
			ChfSignal( 5, ImgX_ATRNOTSTD, 3, 
				item_value, itmlst->item_code, idu_idx );
		    }
		}
	    break;
	/*
	** Non-idu aggr items.
	*/
	default:
	    _ImgGet( fid, itmlst->item_code, &item_value, sizeof(long), 0, 0 );
	    if ( item_value != itmlst->item_value)
		{
		ret_status = ImgX_FAILURE;
		if ( !(flags&ImgM_NoChf) )
		    ChfSignal( 5, ImgX_ATRNOTSTD, 3, 
			    item_value, itmlst->item_code, idu_idx );
		}
	    break;
	} /* end switch */

    ++itmlst;
    } /* end while */

return ret_status;
} /* end of Verify_standard_attrs */
