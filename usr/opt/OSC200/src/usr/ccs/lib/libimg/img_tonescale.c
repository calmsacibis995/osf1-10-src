
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
**  IMG_TONESCALE
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**	Performs tone scale adjustment on an image.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Karen Rodwell
**
**  CREATION DATE:
**
**	August 24, 1989
**
*****************************************************************************/

/*
**  Table of contents
*/
#if defined(__VMS) || defined(VMS)
unsigned long	IMG$TONESCALE_ADJUST();       /* Tonescale VMS bindings     */
#endif
#ifdef NODAS_PROTO
struct FCT *ImgTonescaleAdjust();         /* Tonescale portable bindings*/
#endif


/*
** Include files:
*/
#include <img/ChfDef.h>			       /* Condition handler	     */
#include <img/ImgDef.h>			       /* Image definitions	     */
#include <ImgDefP.h>			       /* Image definitions	     */
#include <ImgMacros.h>			       /* Common macro definitions   */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif


/*
**  External References from ISL	           <- from module ->
*/				
#ifdef NODAS_PROTO
struct FCT  *_ImgGet();			    /* img__attribute_access_utils  */
long	     _ImgGetVerifyStatus();
struct FCT  *_ImgPut();			    /* img__attribute_access_utils  */
long	     _ImgVerifyStandardFormat();    /* IMG__VERIFY_UTILS	    */

unsigned long	ImgAdjustCompTonescale();
unsigned long	ImgCvtCompSpaceOrg();
void		ImgDeallocateFrame();
unsigned long	ImgCopyFrame();
unsigned long	ImgSetRectRoi();
unsigned long	ImgStandardizeFrame();
unsigned long	ImgUnsetRectRoi();
void		ImgVerifyFrame();

/*
**  External References from the Condition Handling Facility
*/				
void             	ChfStop();     		/* Exception stop */
void             	ChfSignal();     	/* Exception signal */
#endif

/*
** Status codes
*/
#include <img/ImgStatusCodes.h>
 

/******************************************************************************
**  IMG$TONESCALE_ADJUST
**  ImgTonescaleAdjust
**
**  FUNCTIONAL DESCRIPTION:
**
**	Adjust the tonescale of one or more plane of an image based
**	on the parameters which define the adjustment function.
**
**	The tonescale adjust function is a straight line which remaps
**	input intensities to output intensities.  The function specification
**	is based on component values being normalized to a range between
**	0.0 and 1.0.
**
**	Two "punch factors" (so-called since this function is sometimes
**	said to be a way to "punch up" the tonescale, of an image) 
**	specify points in the source range which anchor the remap function 
**	output points to 0.0 and 1.0, respectively.
**
**	For example:
**
**	1.0 +............*...
**	    |           /.  .
**	    |          / .  .
**	    |         /  .  .
**	    |        /   .  .
**	    |       /    .  .
**	    |      /     .  .
**	    |     /      .  .
**	0.0 +----*-------*--+
**         0.0   |       | 1.0
**               |       |
**               |     punch 2
**            punch 1
**
**  FORMAL PARAMETERS:
**
**	srcfid	    Source frame of image to adjust.
**	punch1	    Lower "punch factor" of the adjustment function.
**	punch2	    Upper "punch factor" of the adjustment function.
**	roi	    Region of interest identifer.  Passed by value.
**
**	flags	    No flags are defined at present.
**
**	comp_index  Index of component to adjust.  Values are:
**
**			0   adjust all components
**			1   adjust 1st component only
**			2   adjust 2nd component only
**			3   adjust 3rd component only
**			...
**			n   adjust nth component only
**
**		    This argument is passed by value.
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
**	retfid	    Frame identifier of tonescale adjusted image.
**		    Passed by value.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$TONESCALE_ADJUST( srcfid,punch1,punch2,roi,flags,comp_index )
struct FCT  *srcfid;		/* input frame			    */
float 	    *punch1;		/* lower bound of tonescale enhance  */
float	    *punch2;		/* upper bound of tonescale enhance */
struct ROI  *roi;		/* region of interest		    */
long   	     flags;		/* retained for future use	    */
long         comp_index;	/* component index 0,1,2,3...       */
{

return (ImgTonescaleAdjust( srcfid,punch1,punch2,roi,flags,comp_index ));
} /* end of IMG$TONESCALE_ADJUST */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct FCT *ImgTonescaleAdjust( srcfid,punch1,punch2,roi,flags,comp_index )
struct FCT  *srcfid;		/* input frame			    */
float 	    *punch1;		/* lower bound of tonescale enhance  */
float	    *punch2;		/* upper bound of tonescale enhance */
struct ROI  *roi;		/* region of interest		    */
long   	     flags;		/* retained for future use	    */
long         comp_index;	/* component index 0,1,2,3...       */
{
long	comp_cnt;
long	local_comp_idx;
long	local_flags;
struct FCT *retfid;
long	std_cs_org	    = ImgK_BandIntrlvdByPlane;
long	saved_cs_org;
long	saved_data_class;
long	saved_dp_signif;
long	status;
struct FCT *tmp_fid_1;
struct FCT *tmp_fid_2;

/*
** Verify the source frame ...
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

/*
** If the source frame is bitonal, copy the frame and return.
*/
_ImgGet( srcfid, Img_ImageDataClass, &saved_data_class, LONGSIZE, 0, 0 );
if ( saved_data_class == ImgK_ClassBitonal )
    {
    retfid = ImgCopyFrame( srcfid, 0 );
    return retfid;
    }

/*
** Convert the source frame to standard format if necessary
*/
_ImgGet( srcfid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0, 0 );
_ImgGet( srcfid, Img_PlaneSignif, &saved_dp_signif, LONGSIZE, 0, 0 );
if ( (saved_data_class == ImgK_ClassBitonal ||
      saved_data_class == ImgK_ClassGreyscale ) &&
      saved_cs_org == ImgK_BandIntrlvdByPixel )
    {
    _ImgPut( srcfid, Img_CompSpaceOrg, &std_cs_org, LONGSIZE, 0 );
    }

status = _ImgVerifyStandardFormat( srcfid, ImgM_NoChf );
if ( !(status & 1) )
    tmp_fid_1 = ImgStandardizeFrame( srcfid, 0 );
else
    tmp_fid_1 = srcfid;

/*
** Call V3.0 entry point ...
*/

/*
** Version 1 documentation suggests that ROI operations are to be done
** "in place" therefore the following code is commented out:
**
** ImgSetRectRoi( tmp_fid_1, roi, 0 );
**
*/

if ( comp_index != 0 )
    /*
    ** Adjust a single component ...
    */
    {
    local_flags = 0;
    local_comp_idx = comp_index - 1;
    tmp_fid_2 = ImgAdjustCompTonescale( tmp_fid_1, local_comp_idx, 
		    punch1, punch2, roi, local_flags );
    }
else
    /*
    ** Adjust all the components using the same punch factors ...
    */
    {
    _ImgGet( tmp_fid_1, Img_NumberOfComp, &comp_cnt, LONGSIZE, 0, 0 );
    tmp_fid_2 = ImgCopyFrame( tmp_fid_1, 0 );

    local_flags = ImgM_InPlace;
    for ( local_comp_idx = 0; local_comp_idx < comp_cnt; ++local_comp_idx )
	{
	tmp_fid_2 = ImgAdjustCompTonescale( tmp_fid_2, local_comp_idx, 
			punch1, punch2, 
			roi, local_flags );
	}
    }

/*
** Version 1 documentation suggests that ROI operations are to be done
** "in place" therefore the following code is commented out:
**
** ImgUnsetRectRoi( tmp_fid_1 );
**
*/

/*
** Convert Comp. Space Org of the resultant frame back to the original
** saved value if the original value was not band interleaved by plane.
*/
if ( saved_cs_org != ImgK_BandIntrlvdByPlane )
    {
    retfid = ImgCvtCompSpaceOrg( tmp_fid_2, saved_cs_org, saved_dp_signif, 0 );
    ImgDeallocateFrame( tmp_fid_2 );
    _ImgPut( srcfid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0 );
    }
else
    retfid = tmp_fid_2;

if ( tmp_fid_1 != srcfid )
    ImgDeallocateFrame( tmp_fid_1 );

if ( VERIFY_ON_ )
    ImgVerifyFrame( retfid, ImgM_NonstandardVerify );

return retfid;
} /* end of ImgTonescaleAdjust */
