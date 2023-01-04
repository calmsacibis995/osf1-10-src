
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
**
**  img_scale
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains the ISL layer one image scaling routines.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Joe Mauro   - Digital Equipment Corporation
**
**  CREATION DATE:
**
**	August 10, 1989
**
*****************************************************************************/

/*
**  Table of contents
*/
#if defined(__VMS) || defined(VMS)
struct FCT		*IMG$SCALE();
struct FCT		*IMG$SCALE_FRAME();
#endif
#ifdef NODAS_PROTO
struct FCT              *ImgScale();
struct FCT              *ImgScaleFrame();
#endif


/*
**  Include Files 
*/

#include <img/ChfDef.h>		    /* Condition Handling Facility  */
#include <img/ImgDef.h>		    /* ISL Image Definitions	    */
#include <ImgDefP.h>		    /* ISL Private Definitions	    */
#include <ImgMacros.h>		    /* ISL Macro Definitions	    */
#include <ImgVectorTable.h>	    /* ISL Layer II Functions       */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  Macro Definitions
*/

/*
**  Equated Symbols
*/


/*
**  External References from ISL                 <- from module ->
*/
#ifdef NODAS_PROTO
unsigned long	ImgAllocateFrame();
unsigned long	ImgCopyFrame();
unsigned long	ImgCvtCompSpaceOrg();
void		ImgDeallocateFrame();		/* IMG_FRAME_UTILS	*/
void		ImgVerifyFrame();		/* IMG_FRAME_UTILS	*/
unsigned long	ImgSetRectRoi();		/* IMG_ROI_UTILS	*/
unsigned long	ImgStandardizeFrame();		/* IMG_FRAME_UTILS	*/
unsigned long	ImgUnsetRectRoi();		/* IMG_ROI_UTILS	*/

struct FCT	*_ImgCheckNormal();		/* img__verify		*/
struct FCT	*_ImgCloneFrame();		/* img__frame_utils	*/
void		 _ImgErrorHandler();		/* img_error_handler	*/
struct FCT	*_ImgGet();			/* img__access_utils    */
long		 _ImgGetVerifyStatus();
struct FCT	*_ImgPut();			/* img__access_utils    */
struct UDP	*_ImgSetRoi();			/* img__roi		*/
long		 _ImgValidateRoi();		/* img__roi		*/
long		 _ImgVerifyStandardFormat();	/* IMG__VERIFY_UTILS	*/

/*
**  External References from Conditional Handling Facility
*/

void		 ChfStop();			/* exception stop	*/
void		 ChfSignal();			/* exception signal	*/
#endif

/*
** Status codes
*/
#include <img/ImgStatusCodes.h>
 

/*******************************************************************************
**		    BEGINNING OF V2.0 ENTRY POINTS			      **
*******************************************************************************/

/*
**  NOTE:
**
**	The "alignment" parameter of V2.0 has been decommitted 
**	and is accepted but ignored in both of these calls.
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$SCALE(src_fid,scale_1,scale_2,roi_id,flags,alignment)
struct FCT	    *src_fid;		/* source frame id		    */
float		    *scale_1,		/* scale factor (X or XY)	    */
		    *scale_2;		/* scale factor (Y)		    */
struct ROI	    *roi_id;		/* ROI block pointer		    */

int		     flags;		/* scaling modifier flags	    */
int                  alignment;         /* alignment modulus                */
{

return (ImgScale(src_fid,scale_1,scale_2,roi_id,flags,alignment));
} /* end of IMG$SCALE */
#endif


struct FCT *ImgScale(src_fid,scale_1,scale_2,roi_id,flags,alignment)
struct FCT	    *src_fid;		/* source frame id		    */
float		    *scale_1,		/* scale factor (X or XY)	    */
		    *scale_2;		/* scale factor (Y)		    */
struct ROI	    *roi_id;		/* ROI block pointer		    */
int		     flags;		/* scaling modifier flags	    */
int                  alignment;         /* alignment modulus                */
{
struct FCT *ret_fid;
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
    ImgVerifyFrame( src_fid, ImgM_NonstandardVerify );

/*
** Convert the source frame to standard format if necessary
*/
_ImgGet( src_fid, Img_ImageDataClass, &saved_data_class, LONGSIZE, 0, 0 );
_ImgGet( src_fid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0, 0 );
_ImgGet( src_fid, Img_PlaneSignif, &saved_dp_signif, LONGSIZE, 0, 0 );
if ( (saved_data_class == ImgK_ClassBitonal ||
      saved_data_class == ImgK_ClassGreyscale ) &&
      saved_cs_org == ImgK_BandIntrlvdByPixel )
    {
    _ImgPut( src_fid, Img_CompSpaceOrg, &std_cs_org, LONGSIZE, 0 );
    }

status = _ImgVerifyStandardFormat( src_fid, ImgM_NoChf );
if ( !(status & 1) )
    tmp_fid_1 = ImgStandardizeFrame( src_fid, 0 );
else
    tmp_fid_1 = src_fid;

/*
** Call V3.0 entry point ...
*/
ImgSetRectRoi( tmp_fid_1, roi_id, 0 );
tmp_fid_2 = ImgScaleFrame( tmp_fid_1, scale_1, scale_2, flags);
ImgUnsetRectRoi( tmp_fid_1 );

/*
** Convert Comp. Space Org of the resultant frame back to the original
** saved value if the original value was not band interleaved by plane.
*/
if ( saved_cs_org != ImgK_BandIntrlvdByPlane )
    {
    ret_fid = ImgCvtCompSpaceOrg( tmp_fid_2, saved_cs_org, saved_dp_signif, 0 );
    ImgDeallocateFrame( tmp_fid_2 );
    _ImgPut( src_fid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0 );
    }
else
    ret_fid = tmp_fid_2;

if ( tmp_fid_1 != src_fid )
    ImgDeallocateFrame( tmp_fid_1 );

if ( VERIFY_ON_ )
    ImgVerifyFrame( ret_fid, ImgM_NonstandardVerify );

return ret_fid;
} /* end of ImgScale */


/*******************************************************************************
**		BEGINNING OF CURRENT ENTRY POINTS                            **
*******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$SCALE_FRAME( src_fid, scale_1, scale_2, flags )
struct FCT	    *src_fid;		/* source frame id		    */
float		    *scale_1,		/* scale factor (X or XY)	    */
		    *scale_2;		/* scale factor (Y)		    */
int		     flags;		/* scaling modifier flags	    */
{

return (ImgScaleFrame( src_fid, scale_1, scale_2, flags ));
} /* end of IMG$SCALE_FRAME */
#endif


/*****************************************************************************
**  ImgScaleFrame 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return an image frame to the user which has been scaled from a
**	supplied image frame. Specificly, this routine performs the following:
**
**		1. parse input parameters
**		2. validate / conformance checking of the fid
**		3. udp and parameter assembly for layer 2 scale routines
**		4. dispatch to layer 2 scale routines
**		5. handling of layer 2 scale routine returns
**
**  FORMAL PARAMETERS:
**
**	src_fid - frame id of source image frame
**	scale_1	- scale factor (X only, or X and Y if scale_2 not present or 0)
**	scale_2	- optional Y scale factor
**	flags	- optional scaling modifier flags
**		    ImgM_SaveVertical        	(BITONAL ONLY)
**		    ImgM_SaveHorizontal		(BITONAL ONLY)
**		    ImgM_ReversePreference	(BITONAL ONLY)
**		    ImgM_DisablePreference	(BITONAL ONLY)
**		    ImgM_NearestNeighbor	
**
**  IMPLICIT INPUTS:	none
**
**  IMPLICIT OUTPUTS:	none
**
**  FUNCTION VALUE:	frame id of created image frame
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	- invalid argument count
**	ImgX_INVCMPTYP  - invalid compression type (compressed data)
**	ImgX_INVROI	- invalid roi
**	ImgX_INVSCLFTR	- invalid scale factor (shrink to zero size)
**
**  SIDE EFFECTS:	none
**
**  RESTRICTIONS:
**
**  1. The input fid must be uncompressed
**  2. The input fid must be organized as band_interleaved_by_plane
**
*****************************************************************************/
struct FCT *ImgScaleFrame( src_fid, scale_1, scale_2, flags)
struct FCT	    *src_fid;		/* source frame id		    */
float		    *scale_1,		/* scale factor (X or XY)	    */
		    *scale_2;		/* scale factor (Y)		    */
int		     flags;		/* scaling modifier flags	    */
{
struct FCT *dst_fid;				/* destination fid ptr	*/
struct UDP dst_udp[MAX_NUMBER_OF_COMPONENTS];   /* dst UDP descriptors  */
struct UDP src_udp[MAX_NUMBER_OF_COMPONENTS];   /* src UDP descriptors  */

int    comp_numb;	/* number of spectral components 	    */
int    dst_npx;		/* destination number of pixels per line    */
int    dst_nsl;		/* destination number of scan lines	    */
float  fsx,fsy;		/* local x & y scale factors		    */
int    i;		/* scratch index variable		    */
int    spec_type;	/* spectral type			    */
int    status;		/* layer 2 status code (returned)	    */
int    vector;		/* dispatch vector table index variable	    */

/*
** Parameter Parsing & Initialization
*/
fsx = fsy = *scale_1;
if (scale_2 !=0) fsy = *scale_2;

    /***********************************************************
    **	    start of validation & conformance checking        **
    ************************************************************/

if ( VERIFY_ON_ )
    ImgVerifyFrame( src_fid, 0 );

_ImgGet(src_fid,Img_ImageDataClass, &spec_type,sizeof(int),0,0);
_ImgGet(src_fid,Img_NumberOfComp,   &comp_numb,sizeof(int),0,0);


/*
** Validate Scale Factors 
*/
for ( i=0; i < comp_numb; ++i )
    {
    _ImgGet( src_fid, Img_Udp, &src_udp[i], sizeof(struct UDP), 0, i );
    dst_npx = ((src_udp[i].UdpL_X2 - src_udp[i].UdpL_X1 + 1) * fsx);
    dst_nsl = ((src_udp[i].UdpL_Y2 - src_udp[i].UdpL_Y1 + 1) * fsy);
    if (dst_npx < 1 ||  dst_nsl < 1) 
	ChfStop(1,ImgX_INVSCLFTR);
    };

    /***********************************************************
    **	    end of validation & conformance checking          **
    ************************************************************/

    /***********************************************************
    **       start dispatch to layer 2 scale routines         **
    ***********************************************************/
/*
** Copy the source frame if the scale factors are 1 to 1
*/
if ( fsx == 1.0 && fsx == fsy )
    dst_fid = (struct FCT *) ImgCopyFrame( src_fid, 0 );
/*
** Else, scale the image ...
*/
else
    {
    dst_fid = (struct FCT *) ImgAllocateFrame( 0, 0, src_fid, ImgM_NoDataPlaneAlloc );
    switch (spec_type)
	{
	case ImgK_ClassBitonal:
	    vector = ImgK_ScaleBitonal; 
	    dst_udp[0].UdpA_Base = 0;
	    status = (*ImgA_VectorTable[vector])(
				     &src_udp[0],
				     &dst_udp[0],
				     fsx,fsy,flags);
	    if ((status & 1) != 1)
		 _ImgErrorHandler(status);
	    break;

	case ImgK_ClassGreyscale:
	case ImgK_ClassMultispect:
	    /*
	    ** Select output pixel re-estimation method:
	    **
	    **	NOTE that images that are one pixel wide or
	    **	one scanline deep use a NEAREST NEIGHBOR algorithm
	    **	regardless of the method specified by the caller.
	    */
	    if ((flags & ImgM_NearestNeighbor) != 0 ||
		(src_udp[0].UdpL_PxlPerScn == 1) ||    
		(src_udp[0].UdpL_ScnCnt == 1))	
		    vector = ImgK_ScaleNearestNeighbor;
		else 
		    vector = ImgK_ScaleInterpolation;

	    for (i=0;i<comp_numb;i++)
		{
		dst_udp[i].UdpA_Base = 0;
		status = (*ImgA_VectorTable[vector])(
					     &src_udp[i],
					     &dst_udp[i],
					     fsx,fsy
					    );
		if ((status & 1) != 1)
		    _ImgErrorHandler(status);
		} /* end for */
	    break;
	} /* end switch */

    /*
    ** Load destination udp into destination fid ...
    **
    **	NOTE that dimensions of dst_udp(s) have been 
    **	set by the scale function, and override those
    **	setup by the allocate frame function.
    */
    for (i=0;i<comp_numb;i++)
	_ImgPut( dst_fid, Img_Udp, &dst_udp[i], sizeof(struct UDP), i );

    } /* end else */

if ( VERIFY_ON_ )
    ImgVerifyFrame( dst_fid, 0 );

return (dst_fid); 
} /* end ImgScaleFrame */
