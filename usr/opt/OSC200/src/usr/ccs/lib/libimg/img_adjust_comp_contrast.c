
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

/*****************************************************************************
**  IMG_ADJUST_COMP_TONESCALE
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	Performs tonescale adjustment on an specified component
**	plane of an image.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark Sornson 
**	Karen Rodwell
**
**  CREATION DATE:
**
**	17-JUL-1990
**
**  MODIFICATION HISTORY:
**
**	V1-001  MWS001	Mark W. Sornson	    17-JUL-1990
**		Initial module creation: rewrite of IMG_TONESCALE.
**
*****************************************************************************/

/*
** Include files:
*/
#include <img/ChfDef.h>			       /* Condition handler	    */
#include <img/ImgDef.h>			       /* Image definitions	    */
#include <ImgDefP.h>			       /* Image private definitions */
#include <ImgMacros.h>			       /* Common macro definitions  */
#include <ImgVectorTable.h>		       /* Dispatch definitions	    */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif
#include <math.h>			       /* Math library		    */


/*
**  Table of contents
**
**	VMS Bindings for global entry points
*/
#if defined(__VMS) || defined(VMS)
unsigned long	IMG$ADJUST_COMP_TONESCALE();
#endif

/*
**	Portable bindings for global entry points
*/
#ifdef NODAS_PROTO
unsigned long	ImgAdjustCompTonescale();
#endif

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static struct LTD   *Create_lut();
#else
PROTO(static struct LTD *Create_lut, (long /*quant_levels*/, float */*punch1*/, float */*punch2*/));
#endif


/*
**  External References from ISL	           <- from module ->
*/				
#ifdef NODAS_PROTO
unsigned long	 _ImgCloneFrame();	/* img__frame_utils		    */
char		*_ImgCalloc();		/* img__memory_mgt		    */
void		 _ImgCfree();		/* img__memory_mgt		    */
void		 _ImgErrorHandler();	/* img_error_handler		    */
unsigned long	 _ImgGet();		/* img__attribute_access_utils	    */
long		 _ImgGetVerifyStatus(); /* img__verify_utils		    */
unsigned long	 _ImgPut();		/* img__attribute_access_utils	    */
struct UDP	*_ImgSetRoi();		/* img__roi			    */
long		 _ImgValidateRoi();	/* img_roi_utils		    */

unsigned long	  ImgCopyFrame();
struct LTD	 *ImgCreateLutDef();
void		  ImgDeleteLutDef();
unsigned long	  ImgRemapDataPlane();
void		  ImgVerifyFrame();
void		  ImgVerifyRoi();
#endif

/*
** Status codes
*/
#include <img/ImgStatusCodes.h>
 

/****************************************************************************
**  IMG$ADJUST_COMP_TONESCALE
**  ImgAdjustCompTonescale
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remap image component values as a function of the punch factors.
**
**	1. Parse input parameters
**	2. Validate/Conformance checking of the fid
**	3. parameter assembly for layer 2 tonescale routines
**	4. Dispatch to layer 2 tonescale routines
**	5. Handling of layer 2 tonescale routine returns
**
**  FORMAL PARAMETERS:
**
**      fid        - Image frame ID.
**	comp_index - Component index:
**      punch_1	   - The lower bound of the tonescale enhancement function.
**      punch_2	   - The upper bound of the tonescale enhancement function.
**      roi        - Region of interest identifier.
**	flags      - Retained for future use.
**
**  IMPLICIT INPUTS:
**	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**
**	returns new fid	- remapped according to new "punch factors".
**
**  SIGNAL CODES:
**
**      ImgX_INVARGCNT - Invalid Argument Count
**      ImgX_INVCMPTYP - Invalid Compression Type
**      ImgX_INVROI    - Invalid Region of interest definition
**      ImgX_PARAMCONF - Parameter conflict between context-id and other params
**      ImgX_PXLTOOLRG - Pixels greater tan 25 bits not supported
**      ImgX_UNSOPTION - Option not yet supported
**
** RESTRICTIONS:
**
**      1. The input FID must be uncompressed
**      2. The input FID must be organized as band_interleaved_by_plane
**
*******************************************************************************
** VMS-specific Entry Point
*******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$ADJUST_COMP_TONESCALE( srcfid, comp_index, punch_1, punch_2, 
					    roi, flags )
unsigned long	 srcfid;
unsigned long	 comp_index;
float		*punch_1;
float		*punch_2;
unsigned long	 roi;
unsigned long	 flags;
{
unsigned long	 retfid;

retfid = ImgAdjustCompTonescale( srcfid, comp_index, punch_1, punch_2, roi, 
				    flags );

return retfid;
} /* end of IMG$ADJUST_COMP_TONESCALE */
#endif

/******************************************************************************
** Portable Entry Point
*******************************************************************************/
struct FCT *ImgAdjustCompTonescale( srcfid, comp_index, punch_1, punch_2, 
					    roi, flags )
struct FCT *srcfid;
unsigned long	 comp_index;
float		*punch_1;
float		*punch_2;
struct ROI *roi;
unsigned long	 flags;
{

long	bits_per_comp;
long	comp_org	    = 0;
long	comp_type	    = 0;
long	image_data_class;
long	index		    = 0;
long	local_flags	    = 0;
long	number_of_comp	    = 0;
long	quant_levels;
long	spectral_mapping    = 0;
long	status;

struct FCT *retfid;

struct UDP   src_udp;
struct UDP   ret_udp;
struct LTD  *lut;

/*****************************************************************
** 	start of validation and conformance checking		**
*****************************************************************/

/*
** Verify the src frame and the roi ...
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, 0 );

if ( roi != 0 )
    ImgVerifyRoi( roi, srcfid, local_flags );

/* 
** Validate image data class ...
*/
_ImgGet( srcfid, Img_ImageDataClass, &image_data_class, sizeof(long), 0, 0);
switch ( image_data_class )
    {
    case ImgK_ClassGreyscale:
    case ImgK_ClassMultispect:
  	break;

    case ImgK_ClassBitonal:
    default:
	ChfStop( 1, ImgX_UNSDATACL );
	break;
    };

/*
** Disallow any non-8-bit-per-comp data planes
*/
_ImgGet( srcfid, Img_BitsPerComp, &bits_per_comp, LONGSIZE, 0, comp_index );
if ( bits_per_comp != 8 )
    ChfStop( 1, ImgX_UNSBITSPC );


/*************************************************************
**    Start dispatch to layer 2 remap routines		    **
*************************************************************/    

/*
** If input fid is the same tonescale as the original image,
** just return a copy of the original ...
*/
if (*punch_1 == 0.0 && *punch_2 == 1.0) 
    {
    retfid = ImgCopyFrame( srcfid, 0 );
    }
/*
** ... Else adjust the component tonescale by building the 
** tonescale adjustment lookup table and remaping the data plane ...
*/
else
    {
    _ImgGet( srcfid, Img_QuantLevelsPerComp, &quant_levels, LONGSIZE, 0, 
		comp_index );
    lut = Create_lut( quant_levels, punch_1, punch_2 );
    local_flags = flags;
    retfid = ImgRemapDataPlane( srcfid, comp_index, lut, roi, local_flags );
    ImgDeleteLutDef( lut );
    }

/*************************************************************
**    End dispatch to layer 2 tonescale remap routines      **
*************************************************************/
if ( VERIFY_ON_ )
    ImgVerifyFrame( retfid, 0 );

return retfid;
} /* end of ImgAdjustCompTonescale*/


/****************************************************************************
**  Create_lut()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a LUT.
**
**
**  FORMAL PARAMETERS:
**
**      lut           - Lookup table pointer.
**      bits_per_comp - Bits per component.
**      punch_1	      - The lower bound of the tonescale enhancement function.
**      punch_2	      - The upper bound of the tonescale enhancement function.
**	comp_index    - Component index:
**
**  IMPLICIT INPUTS:
**	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**
**	returns new fid	- remapped according to new "punch factors".
**
**  SIGNAL CODES:
**
*******************************************************************************/
static struct LTD *Create_lut( quant_levels, punch1, punch2 )
long	     quant_levels;
float	    *punch1;
float	    *punch2;
{
float	    local_punch1    = *punch1;
float	    local_punch2    = *punch2;
float	    tmp_punch;

long		 entry_data_type    = ImgK_DTypeLU;
long		 half;
long		 hi;
long		 i;
long		 imax;
long		 imin;
long		 j;
long		 lo;
long		 maxval;
long		 range;
long		 neg	=0;
long		*temp; /* temp buffer for lut reverse  */

struct LTD	*lut;
struct ITMLST	*lut_attrs;

lut_attrs = (struct ITMLST *)_ImgCalloc( 4, sizeof( struct ITMLST ) );

lut_attrs[0].ItmL_Code	    = Img_LutEntryDataType;
lut_attrs[0].ItmL_Length    = LONGSIZE;
lut_attrs[0].ItmA_Buffer    = (char *) &entry_data_type;
lut_attrs[0].ItmA_Retlen    = 0;
lut_attrs[0].ItmL_Index	    = 0;

lut_attrs[1].ItmL_Code	    = Img_LutQuantLevels;
lut_attrs[1].ItmL_Length    = LONGSIZE;
lut_attrs[1].ItmA_Buffer    = (char *) &quant_levels;
lut_attrs[1].ItmA_Retlen    = 0;
lut_attrs[1].ItmL_Index	    = 0;

lut_attrs[2].ItmL_Code	    = Img_LutEntryCount;
lut_attrs[2].ItmL_Length    = LONGSIZE;
lut_attrs[2].ItmA_Buffer    = (char *) &quant_levels;
lut_attrs[2].ItmA_Retlen    = 0;
lut_attrs[2].ItmL_Index	    = 0;

lut = ImgCreateLutDef( ImgK_UserLut, lut_attrs, 0, 0 );
maxval = quant_levels - 1;
temp = (long *) _ImgCalloc( lut->LutL_TableSize, LONGSIZE );

/* 
** The following algorithm courtesy of Bob Ulichney. ...
** Generate LUT tonescale adjust values for component and remap 
*/
if ( local_punch1 > local_punch2 )
    {
    neg = 1;
    tmp_punch = local_punch1;
    local_punch1 = local_punch2;
    local_punch2 = tmp_punch;
    }

local_punch1 *= maxval;
local_punch2 *= maxval;
lo = local_punch1 + (local_punch1 > 0.0 ? 0.5 : -0.5 );/* round */
hi = local_punch2 + (local_punch2 > 0.0 ? 0.5 : -0.5 );

/* Compute area of table between punch1 and punch2. */
range = hi - lo;

/*   set imin to the MAX of lo or 0 */
imin = (lo > 0.0 ? lo : 0.0);

/*  Set imax to the MIN of hi or maxval*/
imax = (hi < maxval ? hi : maxval);

/* 
** Loop through the range values and insert 
** the values into the LUT for remap to use 
*/
i = 0;
for( i = imin; i < imax; i++ )
    *(lut->LutA_Table + (i*(sizeof (long)))) = 
	(long)(maxval * (i - lo) / range + 0.5);

/* Fill punch2 clip area */
i = 0;
for( i = imax; i < quant_levels; i++ )
    *(lut->LutA_Table + (i*(sizeof (long)))) = (long)maxval;
			
/* Negate the input if necessary */
if ( neg ) 
    {
    for( i = imin; i <= imax; i++ )
	{
        j = *(lut->LutA_Table + (i*(sizeof (long))));
        *(temp+i) = j;
        *(temp+i) = *(lut->LutA_Table + (i*(sizeof (long))));
        *(temp+i) = *(lut->LutA_Table + (i*(sizeof (long))));
        *(temp+i) = j;
        }
         
    /* 
    ** Replace the reversed values in the range
    ** from temp into the lut 
    */
    for( i = 0; i < maxval; i++ )
	*(lut->LutA_Table + (i*(sizeof (long)))) = *(temp+i);

    /* fill > P1 with 0, fill < P2 with max */

    for( i = 0; i < imin; i++ )
	*(lut->LutA_Table + (i*(sizeof (long)))) = (long)maxval;

    for( i = imax; i <= maxval; i++ )
	*(lut->LutA_Table + (i*(sizeof (long)))) = (long)0.0;
    } /* end if neg */

    _ImgCfree( temp );

return lut;
} /* end Create_lut */
