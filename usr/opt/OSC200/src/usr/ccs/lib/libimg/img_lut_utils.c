
/************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
************************************************************************/

/************************************************************************
**  IMG_LUT_UTILS
**
**  FACILITY:
**
**	DECimage Application Services, Image Services Library
**
**  ABSTRACT:
**
**	Utility functions for dynamicly allocated lookup table 
**	management.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark Sornson 
**
**  CREATION DATE:
**
**	18-JUN-1990
**
**  MODIFICATION HISTORY:
**
**	V1-001	MWS001	Mark W. Sornson	    18-JUN-1990
**		Initial module creation.
**
************************************************************************/

/*
**  Include files:
*/
#include    <string.h>

#include    <img/ChfDef.h>
#include    <img/ImgDef.h>
#include    <ImgDefP.h>
#include    <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  Table of contents:
**
**	VMS-bindings for global entry points
*/
#if defined(__VMS) || defined(VMS)
unsigned char	*IMG$ALLOCATE_LUT();
struct LTD	*IMG$ATTACH_LUT();
struct LTD	*IMG$CREATE_LUT_DEF();
void		 IMG$DEALLOCATE_LUT();
unsigned char	*IMG$DETACH_LUT();
void		 IMG$DELETE_LUT_DEF();
/* struct LTD	*IMG$GET_LUT_ATTRIBUTES();  */
void		 IMG$VERIFY_LUT_DEF();
#endif

/*
**	Portable bindings for global entry points
*/
#ifdef NODAS_PROTO
unsigned char	*ImgAllocateLut();
struct LTD	*ImgAttachLut();
struct LTD	*ImgCreateLutDef();
void		 ImgDeallocateLut();
unsigned char	*ImgDetachLut();
void		 ImgDeleteLutDef();
/* struct LTD	*ImgGetLutAttributes();	*/
void		 ImgVerifyLutDef();
#endif

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static struct LTD   *Alloc_and_init_ltd();
static struct LTD   *Allocate_ltd();
static struct LTD   *Apply_ltd_defaults();
static void	     Attach_user_lut();
static struct LTD   *Clone_ltd();
static long	     Get_dtype_size();
static long	     Get_min_lut_size();
static struct LTD   *Initialize_8bit_ltd();
static struct LTD   *Initialize_predefined_ltd();
static void	     Verify_ltd_attrs();
static void	     Verify_table();
#else
PROTO(static struct LTD *Alloc_and_init_ltd, (unsigned long /*dfn_type*/, struct ITMLST */*table_attrs*/));
PROTO(static struct LTD *Allocate_ltd, (unsigned long /*entry_data_type*/, unsigned long /*entry_tupple_size*/, unsigned long /*number_of_comp*/));
PROTO(static struct LTD *Apply_ltd_defaults, (struct LTD */*ltd*/));
PROTO(static void Attach_user_lut, (struct LTD */*ltd*/, struct ITMLST */*table_attrs*/));
PROTO(static struct LTD *Clone_ltd, (struct LTD */*srcltd*/, unsigned long /*flags*/));
PROTO(static long Get_dtype_size, (unsigned long /*data_type*/));
PROTO(static long Get_min_lut_size, (struct LTD */*ltd*/));
PROTO(static struct LTD *Initialize_8bit_ltd, (struct LTD */*ltd*/));
PROTO(static struct LTD *Initialize_predefined_ltd, (struct LTD */*ltd*/, unsigned long /*dfn_type*/));
PROTO(static void Verify_ltd_attrs, (struct LTD */*ltd*/));
PROTO(static void Verify_table, (struct LTD */*ltd*/));
#endif

/*
**  MACRO definitions:
**
**	none
*/

/*
**  Equated Symbols:
**
**	none
*/

/*
**  External References:
**
**	Symbol Definitions For Message Codes
*/
#include <img/ImgStatusCodes.h>

/*
**	External Entry Points
*/
#ifdef NODAS_PROTO
struct	BHD *_ImgBlkAlloc();
void	     _ImgBlkDealloc();
char	    *_ImgCalloc();
long	     _ImgConvertLevelsToBits();
long	     _ImgExtractItmlstItem();
void	     _ImgFree();
char	    *_ImgMalloc();
#endif

/*
**  External Lookup Table Referencs
*/
#if defined(__VAXC) || defined(VAXC)
globalref unsigned char
#else
extern unsigned char
#endif
     IMG_AB_BIN_RVSD[]
    ,IMG_AB_LW_BIN[]
    ,IMG_AB_LW_BIN_RVSD[]
    ,IMG_AB_NIB_RVSD[]
    ,IMG_AB_NOCHANGE[]
    ,IMG_AB_RGB332_Y1_LUT[]
    ,IMG_AB_RGB332_Y8_LUT[]
    ,IMG_AB_TRASH_BIN[]
    ,IMG_AW_ASCII_HEX[]
    ,IMG_AW_ASCII_HEX_RVSD[]
    ;

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  IMG$ALLOCATE_LUT
**  ImgAllocateLut
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned char *IMG$ALLOCATE_LUT( lut_size, lut_def, flags )
unsigned long	 lut_size;
struct LTD	*lut_def;
unsigned long	 flags;
{

return (ImgAllocateLut( lut_size, lut_def, flags ));
} /* end of IMG$ALLOCATE_LUT */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
unsigned char *ImgAllocateLut( lut_size, lut_def, flags )
unsigned long	 lut_size;
struct LTD	*lut_def;
unsigned long	 flags;
{
unsigned char	*lut;
unsigned long	 local_lut_size	= lut_size;

if ( lut_size != 0 && lut_def != 0 )
    /*
    ** Optional arguments conflict
    */
    ChfStop( 1, ImgX_OPARGCONF );

if ( lut_def != 0 )
    {
    ImgVerifyLutDef( lut_def, ImgM_NoTableVerify, 0, 0 );
    local_lut_size = lut_def->LutL_TableSize;
    }

if ( local_lut_size == 0 )
    ChfStop( 1, ImgX_ZERLUTSIZ );

lut = (unsigned char *)_ImgCalloc( 1, local_lut_size );

if ( lut_def != 0 && !(flags & ImgM_NoLutAttach) )
    ImgAttachLut( lut, local_lut_size, lut_def, 0 );

return lut;
} /* end of ImgAllocateLut */


/******************************************************************************
**  IMG$ATTACH_LUT
**  ImgAttachLut
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct LTD *IMG$ATTACH_LUT( lut, lut_size, lut_def, flags )
unsigned char	*lut;
unsigned long	 lut_size;
struct LTD	*lut_def;
unsigned long	 flags;
{

return (ImgAttachLut( lut, lut_size, lut_def, flags ));
} /* end of IMG$ATTACH_LUT */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct LTD *ImgAttachLut( lut, lut_size, lut_def, flags )
unsigned char	*lut;
unsigned long	 lut_size;
struct LTD	*lut_def;
unsigned long	 flags;
{
unsigned char	*local_lut;
long		 min_lut_size;

if ( lut == 0 )
    ChfStop( 1, ImgX_ZERLUTADR );

if ( lut_size == 0 )
    ChfStop( 1, ImgX_ZERLUTSIZ );

ImgVerifyLutDef( lut_def, ImgM_NoTableVerify, 0, 0 );
if ( lut_def->LutL_DfnType != ImgK_UserLut )
    ChfStop( 1, ImgX_NOTUSRLTD );

/*
** Attach the supplied lut if it is not already attached
*/
if ( lut_def->LutA_Table != lut )
    {
    /*
    ** Erase existing lookup table (if there is one)
    */
    if ( lut_def->LutA_Table != 0 )
	{
	local_lut = ImgDetachLut( lut_def, 0, 0 );
	ImgDeallocateLut( local_lut );
	}

    min_lut_size = Get_min_lut_size( lut_def );
    if ( lut_size < min_lut_size )
	ChfStop( 3, ImgX_LUTTOOSMA, 2, lut_size, min_lut_size );

    /*
    ** Store the Lut in the lut definition object
    */
    lut_def->LutL_TableSize = lut_size;
    lut_def->LutA_Table = lut;

    /*
    ** Mark attached lut as user owned if requested
    */
    if ( (flags & ImgM_UserOwnedLut) )
	lut_def->LutL_Flags.LutV_UserOwnedLut = 1;
    }

return lut_def;
} /* end of ImgAttachLut */


/******************************************************************************
**  IMG$CREATE_LUT_DEF
**  ImgCreateLutDef
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct LTD *IMG$CREATE_LUT_DEF( dfn_type, table_attrs, lut_def, flags )
unsigned long	 dfn_type;
struct ITMLST	*table_attrs;
struct LTD	*lut_def;
unsigned long	 flags;
{

return (ImgCreateLutDef( dfn_type, table_attrs, lut_def, flags ));
} /* end of IMG$CREATE_LUT_DEF */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct LTD *ImgCreateLutDef( dfn_type, table_attrs, srcltd, flags )
unsigned long	 dfn_type;
struct ITMLST	*table_attrs;
struct LTD	*srcltd;
unsigned long	 flags;
{
unsigned long		 local_flags;
struct ITMLST		*local_table_attrs  = table_attrs;
struct LTD		*retltd;
static struct ITMLST	 empty_itmlst	    = { 0, 0, 0, 0, 0 };

/*
** Verify validity of input parameters
*/
if ( dfn_type == 0 && srcltd == 0 )
    ChfStop( 1, ImgX_ZERDFNTYP );

if ( dfn_type != 0 && srcltd != 0)
    ChfStop( 1, ImgX_LUTDFNCNF );

if ( dfn_type != 0 && dfn_type != ImgK_UserLut && table_attrs != 0 )
    ChfStop( 1, ImgX_TABATRCNF );

if ( local_table_attrs == 0 )
    local_table_attrs = &empty_itmlst;

/*
** Allocate and initialize lookup table definition block
*/
if ( srcltd != 0 )
    {
    /*
    ** Copy the src lut def ...
    **
    **	NOTE: clear the flags and the table pointer field
    */
    retltd = Clone_ltd( srcltd, local_flags );
    }
else
    {
    /*
    ** Allocate the dst ltd from scratch, and adjust its attributes
    ** to reflect specified user supplied values
    */
    retltd = Alloc_and_init_ltd( dfn_type, table_attrs );
    }

/*
** Now that the ltd has been created, attach a lookup table
** unless directed not to (assuming that this is a user lut) ...
**
**  NOTE: predefined tables already have the table attached
*/
local_flags = ImgM_NoTableVerify;
if ( !(flags & ImgM_NoLutAttach) && dfn_type == ImgK_UserLut )
    {
    Attach_user_lut( retltd, table_attrs );
    local_flags = 0;
    }

ImgVerifyLutDef( retltd, local_flags, 0, 0 );

return retltd;
} /* end ImgCreateLutDef */


/******************************************************************************
**  IMG$DEALLOCATE_LUT
**  ImgDeallocateLut
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
void IMG$DEALLOCATE_LUT( lut )
unsigned char	*lut;
{

ImgDeallocateLut( lut );
return;
} /* end of IMG$DEALLOCATE_LUT */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
void ImgDeallocateLut( lut )
unsigned char	*lut;
{

if ( lut == 0 )
    ChfStop( 1, ImgX_ZERLUTADR );

_ImgFree( lut );

return;
} /* end of ImgDeallocateLut */


/******************************************************************************
**  IMG$DETACH_LUT
**  ImgDetachLut
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned char *IMG$DETACH_LUT( ltd, flags, ret_lut_size )
struct LTD	*ltd;
unsigned long	 flags;
unsigned long	*ret_lut_size;
{

return ImgDetachLut( ltd, flags, ret_lut_size );
} /* end of IMG$DETACH_LUT */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
unsigned char *ImgDetachLut( ltd, flags, ret_lut_size )
struct LTD	*ltd;
unsigned long	 flags;
unsigned long	*ret_lut_size;	    /* optional	*/
{
unsigned char	*lut	= 0;

ImgVerifyLutDef( ltd, ImgM_NoTableVerify, 0, 0 );

/*
** Only detach user luts (i.e., not predefined luts)
**
**  However, we will allow any lut address to be returned.
*/
if ( !(ltd->LutL_Flags.LutV_PredefinedLut) )
    {
    if ( !(flags & ImgM_GetLutAdrOnly) )
	{
        ltd->LutA_Table = 0;
	ltd->LutL_TableSize = 0;
	}
    }

lut = ltd->LutA_Table;
if ( ret_lut_size != 0 )
    *ret_lut_size = ltd->LutL_TableSize;

return lut;
} /* end of ImgDetachLut */


/******************************************************************************
**  IMG$DELETE_LUT_DEF
**  ImgDeleteLutDef
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
void IMG$DELETE_LUT_DEF( ltd )
struct LTD  *ltd;
{

ImgDeleteLutDef( ltd );

return;
} /* end of IMG$DELETE_LUT_DEF */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
void ImgDeleteLutDef( ltd )
struct LTD  *ltd;
{

/*
** Make sure that this is a ltd object
*/
ImgVerifyLutDef( ltd, (ImgM_NoAttrVerify | ImgM_NoTableVerify), 0, 0 );

/*
** Delete the table if it isn't predefined and it doesn't belong
** to the user ...
*/
if ( !(ltd->LutL_Flags.LutV_PredefinedLut) && 
     !(ltd->LutL_Flags.LutV_UserOwnedLut) )
    if ( ltd->LutA_Table != 0 )
    _ImgFree( ltd->LutA_Table );

/*
** Delete all the attached sub-structures, lists, and buffers  ...
*/
if ( ltd->LutA_BitsPerComp != 0 )
    _ImgFree( ltd->LutA_BitsPerComp );
if ( ltd->LutA_MaxFltValue != 0 )
    _ImgFree( ltd->LutA_MaxFltValue );
if ( ltd->LutA_MaxIntValue != 0 )
    _ImgFree( ltd->LutA_MaxIntValue );
if ( ltd->LutA_QuantLevels != 0 )
    _ImgFree( ltd->LutA_QuantLevels );

if ( ltd->LutA_TableLabel != 0 )
    _ImgFree( ltd->LutA_TableLabel );
if ( ltd->LutA_UserLabel != 0 )
    _ImgFree( ltd->LutA_UserLabel );

/*
** Now delete the ltd object block
*/
_ImgBlkDealloc( (struct BHD *)ltd );

return;
} /* end of ImgDeleteLutDef */


/******************************************************************************
**  IMG$VERIFY_LUT_DEF
**  ImgVerifyLutDef
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
void IMG$VERIFY_LUT_DEF( ltd, flags, fid, dp_index )
struct LTD	*ltd;
unsigned long	 flags;
unsigned long	 fid;
unsigned long	 dp_index;
{

ImgVerifyLutDef( ltd, flags, fid, dp_index );
return;
} /* end of IMG$VERIFY_LUT_DEF */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
void ImgVerifyLutDef( ltd, flags, fid, dp_index )
struct LTD	*ltd;
unsigned long	 flags;
unsigned long	 fid;
unsigned long	 dp_index;
{
float		*max_flt_value;
unsigned long	*bits_per_comp;
unsigned long	*max_int_value;
unsigned long	 total_bits_per_comp;
unsigned long	*quant_levels;
struct UDP	 plane_udp;

if ( ltd == 0 )
    ChfStop( 1, ImgX_ZERDFNTYP );

if ( ltd->LutR_Blkhd.BhdB_Type != ImgK_BlktypLut )
    ChfStop( 1, ImgX_INVLUTDEF );

/*
** Verify the consistency of the contents
*/
if ( !(flags & ImgM_NoAttrVerify) )
    {
    Verify_ltd_attrs( ltd );
    }

/*
** Verify the validity of the table ...
*/
if ( !(flags & ImgM_NoTableVerify) )
    {
    Verify_table( ltd );
    }

/*
** Verify that the lut can be used to remap the image data plane
**
**  Comment this out for now to make code freeze ... make testing
**  more robust later ...
**
**if ( fid != 0 )
**    {
**    _ImgGet( fid, Img_Udp, &plane_udp, sizeof( plane_udp ), 0, dp_index );
**    if ( plane_udp.UdpB_DType != ltd->LutL_EntryDataType )
**	ChfSignal( 4, ImgX_DTYPMISMA, 
**		    2, ltd->LutL_EntryDataType, plane_udp.UdpB_DType );
**    }
*/

} /* end of ImgVerifyLutDef */


static struct LTD *Alloc_and_init_ltd( dfn_type, table_attrs )
unsigned long	 dfn_type;
struct ITMLST	*table_attrs;
{
char		*strbuf;
float		*lut_max_flt_value;
long		 entry_data_type    = ImgK_DTypeBU;
long		 entry_tupple_size  = 1;
long		 item_found;
long		*lut_bits_per_comp;
long		*lut_max_int_value;
long		*lut_quant_levels;
long		 number_of_comp	    = 1;
long		 occurance	    = 1;
long		 strlen;
unsigned long	 code;
unsigned long	 index;
unsigned long	 value;
struct ITMLST	*local_table_attrs  = table_attrs;
struct LTD	*ltd;

/*
** Allocate and initialize ltd structure
*/
if ( dfn_type != ImgK_UserLut )
    /*
    ** Initialize ltd with a predefined type ...
    */
    {
    ltd = Allocate_ltd( entry_data_type, entry_tupple_size, number_of_comp );
    Initialize_predefined_ltd( ltd, dfn_type );
    }
else if ( table_attrs == 0)
    /*
    ** Initialize as a default, implicitly indexed, 8-bit
    ** monochrome lut ...
    */
    {
    ltd = Allocate_ltd( entry_data_type, entry_tupple_size, number_of_comp );
    Initialize_8bit_ltd( ltd );
    ltd->LutL_DfnType = dfn_type;
    }
else
    /*
    ** Initialize ltd with user values supplied in the attr table.
    */
    {
    /*
    ** Explicitly extract entry tupple size, entry data type, and 
    ** number of components (if present in the table attr list),
    ** and allocate the ltd structure.
    */
    _ImgExtractItmlstItem( table_attrs, Img_LutEntryTuppleSize,
			    &entry_tupple_size, 0, 0, 0,
			    occurance );

    _ImgExtractItmlstItem( table_attrs, Img_LutEntryDataType,
			    &entry_data_type, 0, 0, 0,
			    occurance );

    item_found = _ImgExtractItmlstItem( 
			    table_attrs, Img_LutNumberOfComp,
			    &number_of_comp, 0, 0, 0,
			    occurance );

    if ( !item_found && entry_tupple_size > 1 )
	number_of_comp = entry_tupple_size;

    if ( number_of_comp > Get_dtype_size( entry_data_type ) * 8 )
	ChfStop( 1, ImgX_INVCMPCNT );

    ltd = Allocate_ltd( entry_data_type, entry_tupple_size, number_of_comp );
    ltd->LutL_DfnType = dfn_type;

    lut_bits_per_comp = (long *) ltd->LutA_BitsPerComp;
    lut_max_flt_value =          ltd->LutA_MaxFltValue;
    lut_max_int_value = (long *) ltd->LutA_MaxIntValue;
    lut_quant_levels  = (long *) ltd->LutA_QuantLevels;

    /*
    ** Initialize ltd block with values from the rest of
    ** the table attrs list
    */
    for ( ; local_table_attrs->ItmL_Code != 0; ++local_table_attrs )
	{
	code	= local_table_attrs->ItmL_Code;
	value   = *((unsigned long*)local_table_attrs->ItmA_Buffer);
	index   = local_table_attrs->ItmL_Index;
	if ( index >= number_of_comp )
	    ChfSignal( 4, ImgX_INVATRIDX, 2, code, index );
	else
	    switch ( local_table_attrs->ItmL_Code )
		{
		case Img_LutBitsPerComp:
		    lut_bits_per_comp[index] = value;
		    break;
		case Img_LutEntryCount:
		    ltd->LutL_EntryCount = value;
		    break;
		case Img_LutMaxFltValue:
		    lut_max_flt_value[index] = (float)value;
		    break;
		case Img_LutMaxIntValue:
		    lut_max_int_value[index] = value;
		    break;
		case Img_LutQuantLevels:
		    lut_quant_levels[index] = value;
		    break;
		case Img_LutSpectralMapping:
		    ltd->LutL_SpectralMapping = value;
		    break;
		case Img_LutTableType:
		    ltd->LutL_TableType = value;
		    break;
		default:
		    break;
		} /* end switch */
	} /* end for */

    /*
    ** Now apply defaults to fill in any values not yet specified ...
    */
    Apply_ltd_defaults( ltd );
    } /* end else */

/*
** Set any specified label or user field values ...
*/
local_table_attrs = table_attrs;
for ( ; table_attrs != 0 && local_table_attrs->ItmL_Code != 0; 
	++local_table_attrs )
    {
    switch ( local_table_attrs->ItmL_Code )
	{
	case Img_LutTableLabel:
	    strlen = local_table_attrs->ItmL_Length;
	    if ( local_table_attrs->ItmA_Buffer != 0 && strlen != 0 )
		{
		strbuf = (char *)_ImgMalloc( strlen );
		ltd->LutA_TableLabel = strbuf;
		ltd->LutL_TableLabelSize = strlen;
		memcpy( strbuf, local_table_attrs->ItmA_Buffer, strlen ); 
		}
	    break;
	case Img_LutUserField:
	    ltd->LutL_UserField = *((long *)local_table_attrs->ItmA_Buffer);
	    break;
	case Img_LutUserLabel:
	    strlen = local_table_attrs->ItmL_Length;
	    if ( local_table_attrs->ItmA_Buffer != 0 && strlen != 0 )
		{
		strbuf = (char *) _ImgMalloc( strlen );
		ltd->LutA_UserLabel = strbuf;
		ltd->LutL_UserLabelSize = strlen;
		memcpy( strbuf, local_table_attrs->ItmA_Buffer, strlen ); 
		}
		break;
	default:
	    break;
	} /* end switch */
    } /* end for */

return ltd;
} /* end of Alloc_and_init_ltd */


static struct LTD *Allocate_ltd( entry_data_type, entry_tupple_size, 
				    number_of_comp )
unsigned long	entry_data_type;
unsigned long	entry_tupple_size;
unsigned long	number_of_comp;
{
struct LTD  *ltd;

/*
** Allocate the LTD structure and give it initial values
*/
ltd = (struct LTD *)_ImgBlkAlloc( sizeof(struct LTD), ImgK_BlktypLut );

ltd->LutL_EntryDataType = entry_data_type;
switch ( entry_data_type )
    {
    case ImgK_DTypeF:
	ltd->LutA_MaxFltValue	= (float *)
	  _ImgCalloc( entry_tupple_size, LONGSIZE );
	break;
    case ImgK_DTypeB:
    case ImgK_DTypeBU:
    case ImgK_DTypeW:
    case ImgK_DTypeWU:
    case ImgK_DTypeL:
    case ImgK_DTypeLU:
	ltd->LutA_MaxIntValue	= (unsigned long *)
	  _ImgCalloc( entry_tupple_size, LONGSIZE );
	ltd->LutA_QuantLevels	= (unsigned long *) 
	  _ImgCalloc( number_of_comp, LONGSIZE );
	break;
    default:
	_ImgBlkDealloc( (struct BHD *)ltd );
	ChfStop( 3, ImgX_UNSLTDTYP, 1, entry_data_type );
    }

ltd->LutL_EntryTuppleSize   = entry_tupple_size;
ltd->LutA_BitsPerComp	    = (unsigned long *) 
  _ImgCalloc( number_of_comp, LONGSIZE );
ltd->LutL_NumberOfComp	    = number_of_comp;

return ltd;
} /* end of Allocate_ltd */


static struct LTD *Apply_ltd_defaults( ltd )
struct LTD	*ltd;
{
float		*max_flt_value		    = ltd->LutA_MaxFltValue;
unsigned long	*bits_per_comp		    = ltd->LutA_BitsPerComp;
unsigned long	 calculated_max_int_value;
unsigned long	 def_bits_per_comp;
unsigned long	 dtype_size;
unsigned long	 entry_data_type	    = ltd->LutL_EntryDataType;
unsigned long	 entry_tupple_size	    = ltd->LutL_EntryTuppleSize;
unsigned long	 index;
unsigned long	*max_int_value		    = ltd->LutA_MaxIntValue;
unsigned long	 max_levels_per_comp;
unsigned long	 number_of_comp		    = ltd->LutL_NumberOfComp;
unsigned long	*quant_levels		    = ltd->LutA_QuantLevels;
unsigned long	 total_bits_per_comp	    = 0;

static long	 def_332_bits_per_comp[3]   = { 3, 3, 2 };
static long	 def_332_quant_levels[3]    = { 8, 8, 4 };

/*
** Attributes that are already defined for sure:
**
**	dfn_type
**	entry_data_type
**	entry_tupple_size
**	number_of_comp
**
** Attributes that may have to be defined
**
**	bits_per_comp	    depends on data type and quant levels
**	entry_count	    depends on data type and max int value
**	entry_size	    depends on data type, tupple size, and table type
**	max_flt_value	    depends on data type
**	max_int_value	    depends on data type, tupple size, and quant levels
**	quant_levels	    depends on data type, number of comp, and tupple 
**				size
**	spectral_mapping    depends on tupple size
**	table_type	    no dependency
**
*/

if ( ltd->LutL_TableType == 0 )
    ltd->LutL_TableType = ImgK_LtypeImplicitIndex;

if ( entry_data_type == ImgK_DTypeF )
    /*
    ** Attr set up for floating point values.
    **
    ** Relevant attrs:
    **
    **	    bits_per_comp
    **	    entry count
    **	    entry_size
    **	    max_flt_value
    **	    spectral_mapping
    */
    {
    for ( index = 0; index < number_of_comp; ++index )
	{
	bits_per_comp[index] = sizeof( float );
	if ( max_flt_value[index] == 0.0 )
	    max_flt_value[index] = 1.0;
	} /* end for */

    if ( ltd->LutL_EntryCount == 0 )
	ltd->LutL_EntryCount = 256;

    if ( ltd->LutL_TableType == ImgK_LtypeImplicitIndex )
	ltd->LutL_EntrySize = sizeof(float) * entry_tupple_size;
    else
	ltd->LutL_EntrySize = LONGSIZE + ( sizeof(float) * entry_tupple_size );


    if ( ltd->LutL_SpectralMapping == 0)
	switch ( ltd->LutL_EntryTuppleSize )
	    {
	    case 1:
		ltd->LutL_SpectralMapping = ImgK_MonochromeMap;
		break;
	    case 3:
		ltd->LutL_SpectralMapping = ImgK_RGBMap;
		break;
	    default:
		ltd->LutL_SpectralMapping = ImgK_GeneralMap;
		break;
	    } /* end switch */

    } /* end  */
else
    /*
    ** Attr set up for integer point values.
    **
    ** Relevant attrs:
    **
    **	    bits_per_comp
    **	    entry count
    **	    entry_size
    **	    max_int_value
    **	    quant_levels
    **	    spectral_mapping
    */
    {
    if ( ltd->LutL_SpectralMapping == 0 )
	switch ( ltd->LutL_EntryTuppleSize )
	    {
	    case 1:
		ltd->LutL_SpectralMapping = ImgK_MonochromeMap;
		break;
	    case 3:
		ltd->LutL_SpectralMapping = ImgK_RGBMap;
		break;
	    default:
		ltd->LutL_SpectralMapping = ImgK_GeneralMap;
		break;
	    } /* end switch */

    if ( entry_tupple_size > 1 )
	{
	def_bits_per_comp = Get_dtype_size( entry_data_type ) * 8;
	max_levels_per_comp = (1 << def_bits_per_comp);

	for ( index = 0; index < entry_tupple_size; ++index )
	    {
	    /*
	    ** If the max int value is already set, make sure
	    ** it's not larger than it's supposed to be.  If it
	    ** is too big, clip it.
	    */
	    if ( max_int_value[index] > max_levels_per_comp )
		max_int_value[index] = max_levels_per_comp;

	    /*
	    ** If bits per component are already set, make sure
	    ** they are 8 bit multiples
	    */
	    if ( bits_per_comp[index] != 0 )
		(bits_per_comp[index] + 7)/8;

	    /*
	    ** Set quant levels ...
	    */
	    if ( quant_levels[index] == 0 )
		{
		if ( max_int_value[index] != 0 )
		    quant_levels[index] = (1 << max_int_value[index]);
		else if ( bits_per_comp[index] != 0 )
		    quant_levels[index] = (1 << bits_per_comp[index]);
		else
		    quant_levels[index] = max_levels_per_comp;
		}
	    else
		/*
		** Make sure quant levels is only as large as the
		** data type allows
		*/
		if ( quant_levels[index] > max_levels_per_comp )
		    quant_levels[index] = max_levels_per_comp;

	    /*
	    ** Init the max int value per tupple component if
	    ** it's not already set
	    */
	    if ( max_int_value[index] == 0 )
		max_int_value[index] = quant_levels[index];

	    /*
	    ** Init the bits per component if they're not already
	    ** set up, and round to the nearest byte
	    */
	    if ( bits_per_comp[index] == 0 )
		bits_per_comp[index] = (_ImgConvertLevelsToBits(
					    quant_levels[index] ) + 7)/8;

	    } /* end for */
		
	if ( ltd->LutL_TableType == ImgK_LtypeImplicitIndex )
	    ltd->LutL_EntrySize = (def_bits_per_comp / 8) * entry_tupple_size;
	else
	    ltd->LutL_EntrySize = LONGSIZE + 
			(((def_bits_per_comp * entry_tupple_size) + 31)/32)*4;

	} /* end if entry_tupple_size > 1 */
    else
	/*
	** Set up attributes for element tupples equal to 1 ...
	*/
	{
	dtype_size = Get_dtype_size( entry_data_type );
	calculated_max_int_value = ( dtype_size >= 4 ? 0xFFFFFFFF :
					1 << (dtype_size * 8) );
	switch ( number_of_comp )
	    {
	    case 1:
		max_levels_per_comp = calculated_max_int_value;
		if ( quant_levels[0] == 0 )
		    {
		    if ( max_int_value[0] != 0 )
			quant_levels[0] = (1 << max_int_value[0]);
		    else if ( bits_per_comp[0] != 0 )
			quant_levels[0] = (1 << bits_per_comp[0]);
		    else
			quant_levels[0] = max_levels_per_comp;
		    }
		else
		    {
		    /*
		    ** Make sure quant levels are only as large as the
		    ** data type allows
		    */
		    if ( quant_levels[0] > max_levels_per_comp )
			quant_levels[0] = max_levels_per_comp;
		    } /* end else */

		/*
		** Set up the max int value per tupple component
		*/
		if ( max_int_value[0] == 0 )
		    max_int_value[0] = quant_levels[0];
		else
		    {
		    if ( max_int_value[0] > max_levels_per_comp )
			max_int_value[0] = max_levels_per_comp;
		    }

		/*
		** Set up bits per component, and make sure each value
		** is rounded to the nearest byte.
		*/
		if ( bits_per_comp[0] == 0 )
		    bits_per_comp[0] = 
				    _ImgConvertLevelsToBits( quant_levels[0] );

		bits_per_comp[0] = ((bits_per_comp[0] + 7)/8)*8;
		break;
	    case 3:
		if ( bits_per_comp[0] != 0 )
		    /*
		    ** Assume that the bits per comp are already set,
		    ** therefore check to make sure the total doesn't
		    ** exceed what the data type allows.
		    */
		    {
		    total_bits_per_comp = 0;
		    for ( index = 0; index < number_of_comp; ++index )
			{
			if ( bits_per_comp[index] == 0 )
			    ChfStop( 3, ImgX_INVBITSPC, 1, index );

			total_bits_per_comp += bits_per_comp[index];
			if ( total_bits_per_comp > dtype_size * 8 )
			    ChfStop( 3, ImgX_INVBITSPC, 1, index );
			} /* end for */
		    }

		if ( quant_levels[0] == 0 )
		    /*
		    ** Assume all quant levels need to be initialized ...
		    **
		    **	For 8 bit entries, if no bits per comp are supplied,
		    **	use default quant levels  of 8, 8, 4.
		    */
		    {
		    if ( dtype_size == 1 )
			{
			if ( bits_per_comp[0] != 0 )
			    quant_levels[0] = 1 << bits_per_comp[0];
			else
			    quant_levels[0] = 8;

			if ( bits_per_comp[1] != 0 )
			    quant_levels[1] = 1 << bits_per_comp[1];
			else
			    quant_levels[1] = 8;

			if ( bits_per_comp[2] != 0 )
			    quant_levels[2] = 1 << bits_per_comp[2];
			else
			    quant_levels[2] = 4;
			}
		    else
			/*
			** Data type is 2 or more bytes long ...
			**
			**  If bits per comp are not supplied, do nothing,
			**  which means the user required to set up the
			**  quant levels.
			*/
			{
			if ( bits_per_comp[0] != 0 )
			    quant_levels[0] = 1 << bits_per_comp[0];

			if ( bits_per_comp[1] != 0 )
			    quant_levels[1] = 1 << bits_per_comp[1];

			if ( bits_per_comp[2] != 0 )
			    quant_levels[2] = 1 << bits_per_comp[2];
			} /* end else */
		    } /* end else */

		if ( bits_per_comp[0] == 0 )
		    /*
		    ** Assume all bits per comp need to be initialized ...
		    */
		    if ( dtype_size == 1 )
			/*
			** Initialize bits_per_comp as 3,3,2 if quant
			** levels aren't given; else do nothing, and
			** require the user to set up bits per pixel.
			*/
			{
			if ( quant_levels[0] != 0 )
			    bits_per_comp[0] == _ImgConvertLevelsToBits(
						    quant_levels[0] );

			if ( quant_levels[1] != 0 )
			    bits_per_comp[1] == _ImgConvertLevelsToBits(
						    quant_levels[1] );

			if ( quant_levels[2] != 0 )
			    bits_per_comp[2] == _ImgConvertLevelsToBits(
						    quant_levels[2] );
			}

		/*
		** Initialize or clip (if set) the max int value
		** to be the max that the data type can represent...
		*/
		if ( max_int_value[0] == 0 )
		    max_int_value[0] = calculated_max_int_value;
		else
/* NOTE:  THIS DOES NOT LOOK RIGHT
** max_int_value should probably max_int_value[0]
*/
		    if ( max_int_value > (unsigned long *) calculated_max_int_value )
			max_int_value[0] = calculated_max_int_value;
		break;

	    default:
		/*
		** Set up max int value based data type:
		**
		**  initialize it or clip it to the max possible
		**  for the entry data type size.
		*/
		if ( max_int_value[0] == 0 )
		    max_int_value[0] = calculated_max_int_value;
		else if ( max_int_value[0] > calculated_max_int_value )
		    max_int_value[0] = calculated_max_int_value;
		break;

		/*
		** NOTE: quant levels and bits per comp MUST be set up
		** by the user in this case.
		*/
	    } /* end switch */

	/*
	** Set up entry size for 1-tupple entries
	*/
	if ( ltd->LutL_TableType == ImgK_LtypeImplicitIndex )
	    ltd->LutL_EntrySize = dtype_size * entry_tupple_size;
	else
	    /*
	    ** Make sure all entries are longword aligned.
	    */
	    ltd->LutL_EntrySize = LONGSIZE + 
			((dtype_size * entry_tupple_size) + 31)/32;
	} /* end else entry_tupple_size == 1 */

    /*
    ** Set up entry count for integer lut ...
    */
    if ( ltd->LutL_EntryCount == 0 )
	ltd->LutL_EntryCount = 256;

    } /* end else entry_data_type not equal ImgK_DTypeF */

return ltd;
} /* end of Apply_ltd_defaults */


static void Attach_user_lut( ltd, table_attrs )
struct LTD	*ltd;
struct ITMLST	*table_attrs;
{
long		 occurance	    = 1;
long		 table_size_found;
long		 user_table_found;
unsigned char	*user_table_adr	    = 0;
unsigned long	 user_table_size    = 0;

/*
** See if the user supplied a table to attach ...
*/
user_table_found = _ImgExtractItmlstItem( 
			    table_attrs, Img_LutTableAdr,
			    (long *)&user_table_adr, 0, 0, 0,
			    occurance );

table_size_found = _ImgExtractItmlstItem(
			    table_attrs, Img_LutTableSize,
			    (long *)&user_table_size, 0, 0, 0,
			    occurance );

if ( user_table_found && !table_size_found )
    {
    ImgDeleteLutDef( ltd );
    ChfStop( 1, ImgX_TABSZNTFO );
    }

if ( user_table_size == 0 )
    user_table_size = ltd->LutL_EntryCount * ltd->LutL_EntrySize;

if ( user_table_adr == 0 )
    user_table_adr = (unsigned char *) _ImgCalloc( 1, user_table_size );

/*
** Attach the table to the definition block.
*/
ltd->LutA_Table = user_table_adr;
ltd->LutL_TableSize = user_table_size;

return;
} /* end of Attach_user_lut */


static struct LTD *Clone_ltd( srcltd, flags )
struct LTD	*srcltd;
unsigned long	 flags;
{
long	     size;
struct LTD  *dstltd;

/*
** Verify the source lut; 
*/
ImgVerifyLutDef( srcltd, ImgM_NoTableVerify, 0, 0 );

/*
** Clone the source ...
*/
dstltd = Allocate_ltd(	srcltd->LutL_EntryDataType,
			srcltd->LutL_EntryTuppleSize,
			srcltd->LutL_NumberOfComp );
/*
** Copy the src ltd attrs into the dst ltd
*/
size = LONGSIZE * srcltd->LutL_NumberOfComp;
memcpy( dstltd->LutA_BitsPerComp, srcltd->LutA_BitsPerComp, size );
dstltd->LutL_DfnType		= ImgK_UserLut;
dstltd->LutL_EntryCount		= srcltd->LutL_EntryCount;
dstltd->LutL_EntryDataType	= srcltd->LutL_EntryDataType;
dstltd->LutL_EntrySize		= srcltd->LutL_EntrySize;
dstltd->LutL_EntryTuppleSize	= srcltd->LutL_EntryTuppleSize;
if ( srcltd->LutA_MaxFltValue != 0 )
    {
    size = dstltd->LutL_EntryTuppleSize * sizeof(float);
    memcpy( &(dstltd->LutA_MaxFltValue), &(srcltd->LutA_MaxFltValue), size );
    }
if ( srcltd->LutA_MaxIntValue != 0 )
    {
    size = dstltd->LutL_EntryTuppleSize * sizeof(long);
    memcpy( &(dstltd->LutA_MaxIntValue), &(srcltd->LutA_MaxIntValue), size );
    }
dstltd->LutL_NumberOfComp	= srcltd->LutL_NumberOfComp;
if ( srcltd->LutA_MaxIntValue != 0 )
    {
    size = LONGSIZE * srcltd->LutL_NumberOfComp;
    memcpy( dstltd->LutA_QuantLevels, srcltd->LutA_QuantLevels, size );
    }
dstltd->LutL_SpectralMapping	= srcltd->LutL_SpectralMapping;
dstltd->LutL_TableType		= srcltd->LutL_TableType;

return dstltd;
} /* end of Clone_ltd */


static long Get_dtype_size( data_type )
unsigned long	data_type;
{
unsigned long	size;

switch ( data_type )
    {
    case ImgK_DTypeComplexD:
	size = sizeof( double ) * 2;
	break;
    case ImgK_DTypeComplex:
	size = sizeof( float) * 2;
	break;
    case ImgK_DTypeFloatD:
	size = sizeof( double );
	break;
    case ImgK_DTypeFloat:
    case ImgK_DTypeL:
    case ImgK_DTypeLU:
	size = LONGSIZE;
	break;
    case ImgK_DTypeW:
    case ImgK_DTypeWU:
	size = SHORTSIZE;
	break;
    case ImgK_DTypeB:
    case ImgK_DTypeBU:
    default:
	size = CHARSIZE;
	break;
    } /* end switch */

return size;
} /* end of Get_dtype_size */


static long Get_min_lut_size( ltd )
struct LTD  *ltd;
{
long	min_lut_size;

min_lut_size = ltd->LutL_EntrySize * ltd->LutL_EntryCount;

return min_lut_size;
} /* end of Get_min_lut_size */


static struct LTD *Initialize_8bit_ltd( ltd )
struct LTD  *ltd;
{
long	entry_data_type		= ImgK_DTypeBU;
long	entry_tupple_size	= 1;
long	entry_number_of_comp	= 1;

(ltd->LutA_BitsPerComp)[0]  = 8;
ltd->LutL_EntryCount	    = 256;
ltd->LutL_EntryDataType	    = ImgK_DTypeBU;
ltd->LutL_EntrySize	    = 1;
ltd->LutL_EntryTuppleSize   = 1;
(ltd->LutA_MaxIntValue)[0]  = 255;
ltd->LutL_NumberOfComp	    = 1;
(ltd->LutA_QuantLevels)[0]  = 256;
ltd->LutL_SpectralMapping   = ImgK_MonochromeMap;
ltd->LutL_TableType	    = ImgK_LtypeImplicitIndex;

return ltd;
} /* end of Initialize_8bit_ltd */


static struct LTD *Initialize_predefined_ltd( ltd, dfn_type )
struct LTD	*ltd;
unsigned long	 dfn_type;
{
unsigned char	*lut;

Initialize_8bit_ltd( ltd );
ltd->LutL_Flags.LutV_PredefinedLut = 1;
ltd->LutL_DfnType = dfn_type;
ltd->LutL_TableSize = 256;

switch ( dfn_type )
    {
    case ImgK_PredefLutNoChange:
	lut = IMG_AB_NOCHANGE;
	break;
    case ImgK_PredefLutBitRvsd:
	lut = IMG_AB_BIN_RVSD;
	break;
    case ImgK_PredefLutNibRvsd:
	lut = IMG_AB_NIB_RVSD;
	break;
    case ImgK_PredefLutLWBin:
	lut = IMG_AB_LW_BIN;
	break;
    case ImgK_PredefLutLWBinRvsd:
	lut = IMG_AB_LW_BIN_RVSD;
	break;
    case ImgK_PredefLutASCIIHex:
	lut = IMG_AW_ASCII_HEX;
	break;
    case ImgK_PredefLutASCIIHexRvsd:
	lut = IMG_AW_ASCII_HEX_RVSD;
	break;
    case ImgK_PredefLutTrashToZeros:
	lut = IMG_AB_TRASH_BIN;
	break;
    case ImgK_PredefLutRGB332ToL8:
	lut = IMG_AB_RGB332_Y8_LUT;
	break;
    case ImgK_PredefLutRGB332ToL1:
	lut = IMG_AB_RGB332_Y1_LUT;
	break;
    default:
	break;
    } /* end switch */

ltd->LutA_Table = lut;

return ltd;
} /* end of Initialize_predefined_lut */


static void Verify_ltd_attrs( ltd )
struct LTD  *ltd;
{
long	bad_ltd		    = 0;
long	dtype_size;
long	expected_dtype_size;
long	tupple_size;

/*
** Verify the validity of the definition type
*/
switch ( ltd->LutL_DfnType )
    {
    case ImgK_UserLut:
    case ImgK_PredefLutNoChange:
    case ImgK_PredefLutBitRvsd:
    case ImgK_PredefLutNibRvsd:
    case ImgK_PredefLutLWBin:
    case ImgK_PredefLutLWBinRvsd:
    case ImgK_PredefLutASCIIHex:
    case ImgK_PredefLutASCIIHexRvsd:
    case ImgK_PredefLutTrashToZeros:
    case ImgK_PredefLutRGB332ToL1:
    case ImgK_PredefLutRGB332ToL8:
	break;
    default:
	bad_ltd = 1;
	ChfSignal( 5, ImgX_INVLUTATR, 3, Img_LutDfnType, ltd->LutL_DfnType, 0 );
    }
/*
** Verify the validity of the entry data type
*/
switch ( ltd->LutL_EntryDataType )
    {
    case ImgK_DTypeB:
    case ImgK_DTypeBU:
    case ImgK_DTypeW:
    case ImgK_DTypeWU:
    case ImgK_DTypeL:
    case ImgK_DTypeLU:
    case ImgK_DTypeFloat:
	break;
    default:
	bad_ltd = 1;
	ChfSignal( 5, ImgX_INVLUTATR, 3, Img_LutEntryDataType, 
		    ltd->LutL_EntryDataType, 0 );
    }
/*
** Verify the validity of the table type
*/
switch( ltd->LutL_TableType )
    {
    case ImgK_LtypeImplicitIndex:
    case ImgK_LtypeExplicitIndex:
	break;
    default:
	ChfSignal( 5, ImgX_INVLUTATR, 3, Img_LutTableType, 
		    ltd->LutL_TableType, 0 );
	bad_ltd = 1;
    }

/*
** Verify that the spectral mapping scheme is valid and that the 
** number of components is valid  ...
*/
if ( ltd->LutL_NumberOfComp == 0 )
    {
    bad_ltd = 1;
    ChfSignal( 5, ImgX_INVLUTATR, Img_LutNumberOfComp, ltd->LutL_NumberOfComp,
		0 );
    }

switch ( ltd->LutL_SpectralMapping )
    {
    case ImgK_PrivateMap:
    case ImgK_GeneralMap:
	break;
    case ImgK_MonochromeMap:
	if ( ltd->LutL_NumberOfComp != 1 )
	    {
	    bad_ltd = 1;
	    ChfSignal( 5, ImgX_INVLUTATR, Img_LutNumberOfComp, 
			ltd->LutL_NumberOfComp, 0 );
	    }
	break;
    case ImgK_RGBMap:
    case ImgK_CMYMap:
    case ImgK_YUVMap:
    case ImgK_HSVMap:
    case ImgK_HLSMap:
    case ImgK_YIQMap:
	if ( ltd->LutL_NumberOfComp != 3 )
	    {
	    bad_ltd = 1;
	    ChfSignal( 5, ImgX_INVLUTATR, Img_LutNumberOfComp, 
			ltd->LutL_NumberOfComp, 0 );
	    }
	break;
    default:
	bad_ltd = 1;
	ChfSignal( 5, ImgX_INVLUTATR, 3, Img_LutSpectralMapping, 
		    ltd->LutL_SpectralMapping, 0 );
    }

/*
** Verify the tupple size
**
**	NOTE: the check of this attr should be more robust ...
*/
if ( ltd->LutL_EntryTuppleSize == 0 )
    {
    bad_ltd = 1;
    ChfSignal( 5, ImgX_INVLUTATR, 3, Img_LutEntryTuppleSize,
		ltd->LutL_EntryTuppleSize, 0 );
    }

/*
** Verify that the entry size is valid
*/
dtype_size = Get_dtype_size( ltd->LutL_EntryDataType );

switch ( ltd->LutL_TableType )
    {
    case ImgK_LtypeImplicitIndex:
	if ( ltd->LutL_EntrySize != dtype_size )
	    {
	    bad_ltd = 1;
	    ChfSignal( 5, ImgX_INVLUTATR, Img_LutEntrySize, ltd->LutL_EntrySize,
			0 );
	    }
	break;
    case ImgK_LtypeExplicitIndex:
	/*
	** Expected size is the index (one longword) plus the tupple size
	** times the data type size rounded up to the nearest longword.
	*/
	tupple_size = ltd->LutL_EntryTuppleSize;
	expected_dtype_size = LONGSIZE + 
			((((dtype_size * 8 * tupple_size) + 31)/32)*4);
	if ( ltd->LutL_EntrySize != expected_dtype_size )
	    {
	    bad_ltd = 1;
	    ChfSignal( 5, ImgX_INVLUTATR, Img_LutEntrySize, ltd->LutL_EntrySize,
			0 );
	    }
	break;
    }

/*
** Verify:
**
**	* bits per comp
**	* max flt value
**	* max int value
**	* quant levels (per comp)
*/

/*
** Stop if the attributes are bad.
*/
if ( bad_ltd )
    ChfStop( 1, ImgX_INVLUTDEF );

return;
} /* end of Verify_ltd_attrs */


static void Verify_table( ltd )
struct LTD  *ltd;
{
long	min_size;

/*
** Verify table size ...
*/
min_size = ltd->LutL_EntrySize * ltd->LutL_EntryCount;
if ( ltd->LutL_TableSize < min_size )
    ChfSignal( 4, ImgX_INVLUTSIZ, 2, ltd->LutL_TableSize, min_size );

/*
** Verify that table is present ...
*/
if ( ltd->LutA_Table == 0 )
    ChfSignal( 1, ImgX_ZERLUTTAB );

/*
** If the table is a predefined type, verify that the contents
** matches the definition type ...
*/
if ( ltd->LutL_Flags.LutV_PredefinedLut )
    {
    switch ( ltd->LutL_DfnType )
	{
	case ImgK_PredefLutNoChange:
	    if ( ltd->LutA_Table != IMG_AB_NOCHANGE )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutBitRvsd:
	    if ( ltd->LutA_Table != IMG_AB_BIN_RVSD )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutNibRvsd:
	    if ( ltd->LutA_Table != IMG_AB_NIB_RVSD )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutLWBin:
	    if ( ltd->LutA_Table != IMG_AB_LW_BIN )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutLWBinRvsd:
	    if ( ltd->LutA_Table != IMG_AB_LW_BIN_RVSD )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutASCIIHex:
	    if ( ltd->LutA_Table != IMG_AW_ASCII_HEX )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutASCIIHexRvsd:
	    if ( ltd->LutA_Table != IMG_AW_ASCII_HEX_RVSD )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutTrashToZeros:
	    if ( ltd->LutA_Table != IMG_AB_TRASH_BIN )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutRGB332ToL8:
	    if ( ltd->LutA_Table != IMG_AB_RGB332_Y8_LUT )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	case ImgK_PredefLutRGB332ToL1:
	    if ( ltd->LutA_Table != IMG_AB_RGB332_Y1_LUT )
		ChfSignal( 1, ImgX_INVPREDFL );
	    break;
	default:
	    ChfSignal( 1, ImgX_INVPREDFL );
	} /* end switch */
    }

return;
} /* end of Verify_table */
