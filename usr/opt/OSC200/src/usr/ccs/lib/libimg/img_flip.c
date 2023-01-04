
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
**  IMG$FLIP
**
**  FACILITY:
**
**      Image Services Library
**
**
**  ABSTRACT:
**
**      This module contains routines which flip an image frame vertically
**	or horizontally all or a region of interest from a 
**	user supplied source image frame.
**
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX RISC/ULTRIX
**
**
**  AUTHOR(S):
**
**	Richard Piccolo
**	Revised for V3.0 by John Poltrack
**
**  CREATION DATE:
**
**	October 15, 1986
**
*****************************************************************************/

/*
**  Table of contents
*/
#if defined(__VMS) || defined(VMS)
struct FCT	*IMG$FLIP();	         /* Flip routine VMS bindings       */
struct FCT	*IMG$FLIP_FRAME();       /* V3 Flip routine VMS bindings    */
#endif
#ifdef NODAS_PROTO
struct FCT      *ImgFlip();	         /* Flip routine portable binding   */
struct FCT      *ImgFlipFrame();         /* V3 Flip routine portable binding*/
#endif


#include <img/ChfDef.h>			    /* Condition handler	    */
#include <img/ImgDef.h>			    /* Image definitions	    */
#include <ImgDefP.h>			    /* Private image definitions    */
#include <ImgMacros.h>			    /* Common macro definitions	    */
#include <ImgVectorTable.h>		    /* Dispatch definitions	    */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif
/*
**  MACRO definitions -- none
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
void		ImgDeallocateFrame();
void		ImgSetRectRoi();
unsigned long	ImgStandardizeFrame();
void		ImgUnsetRectRoi();
void		ImgVerifyFrame();

struct FCT	*_ImgCloneFrame();		/* img__frame_utils	*/
struct ITMLST	*_ImgCreateItmlst();
void		 _ImgErrorHandler();		/* img_error_handler	*/
struct FCT	*_ImgGet();			/* img__access_utils    */
long		 _ImgGetVerifyStatus();
struct FCT	*_ImgPut();			/* img__access_utils    */
struct UDP	*_ImgSetRoi();			/* img__roi		*/
long		 _ImgValidateRoi();		/* img_roi_utils	*/
long		 _ImgVerifyStandardFormat();

/*
** External references form Condition Handling Facility
*/

void	ChfStop();				/* Exception stop	*/
void	ChfSignal();				/* Exception signal	*/
#endif

/*
** Status codes
*/
#include <img/ImgStatusCodes.h>


/*****************************************************************************
**	    VMS  and Ultrix version 2 entry points                          **
*****************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$FLIP( src_fid, roi_id, flags)
struct FCT	    *src_fid;		/* source frame id		    */
struct ROI	    *roi_id;		/* ROI block pointer		    */
long		     flags;		/* flip modifier flags	    	    */
{
return ( ImgFlip( src_fid, roi_id, flags));
} /* end of IMG$FLIP */
#endif


struct FCT *ImgFlip( src_fid, roi_id, flags)
struct FCT	    *src_fid;		/* source frame id		    */
struct ROI	    *roi_id;		/* ROI block pointer		    */
long		     flags;		/* flip modifier flags	    	    */
{
struct FCT *ret_fid;
unsigned long	std_cs_org	    = ImgK_BandIntrlvdByPlane;
unsigned long	saved_cs_org;
unsigned long	saved_data_class;
unsigned long	saved_dp_signif;
unsigned long	status;
struct   FCT	*tmp_fid_1;
struct   FCT	*tmp_fid_2;

/*
** Verify the source frame ...
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( src_fid, ImgM_NonstandardVerify );

/*
** Convert the source frame to standard format if necessary
*/
_ImgGet( src_fid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0, 0 );
_ImgGet( src_fid, Img_ImageDataClass, &saved_data_class, LONGSIZE, 0, 0 );
_ImgGet( src_fid, Img_PlaneSignif, &saved_dp_signif, LONGSIZE, 0, 0 );
if ( (saved_data_class == ImgK_ClassBitonal ||
      saved_data_class == ImgK_ClassGreyscale ) &&
      saved_cs_org == ImgK_BandIntrlvdByPixel )
    {
    _ImgPut( src_fid, Img_CompSpaceOrg, &std_cs_org, LONGSIZE, 0 );
    }

status = _ImgVerifyStandardFormat( src_fid, ImgM_NoChf );
if ( !(status & 1) )
    tmp_fid_1 = (struct FCT *) ImgStandardizeFrame( src_fid, 0 );
else
    tmp_fid_1 = (struct FCT *) src_fid;

/*
** Call V3.0 entry point ... and unset roi if specified
*/
ImgSetRectRoi( tmp_fid_1, roi_id, 0);
tmp_fid_2 = ImgFlipFrame ( tmp_fid_1, 0, flags);
ImgUnsetRectRoi(tmp_fid_1);

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

return (struct FCT *) ret_fid;
} /* end of ImgFlip */

#if defined(__VMS) || defined(VMS)
struct FCT *IMG$FLIP_FRAME( src_fid, roi_id, flags)
struct FCT	    *src_fid;		/* source frame id		    */
struct ROI	    *roi_id;		/* ROI block pointer		    */
long		     flags;		/* flip modifier flags	    	    */
{
return ( ImgFlipFrame( src_fid, roi_id, flags));
} /* end of IMG$FLIP_FRAME */
#endif


/****************************************************************************
**  ImgFlipFrame - FLIP image
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return an image frame to the user which has been FLIP'd from
**	a supplied image frame. This module (level 1) performs the following:
**
**		1. parse input parameters
**		2. validate / conformance checking of the src_fid
**		3. udp and parameter assembly for layer 2 scale routines
**		4. dispatch to layer 2 flip routines
**		5. handling of layer 2 flip routine returns
**
**  FORMAL PARAMETERS:
**
**	src_fid	- frame id of source image frame
**	roi     - optional pointer to ROI structure
**	flags	- optional flip modifier flags
**		    ImgM_FlipHorizontal		 about y axis
**		    ImgM_FlipVertical  		 about x axis	      
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
**      retfid - frame id of created image frame
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	- invalid argument count
**	ImgX_UNSOPTION  - unsupported spect map or component organization     
**	ImgX_PARAMCONF  - inconsistent frame data    
**
**  SIDE EFFECTS:
**
**	none
**
*****************************************************************************/
struct FCT *ImgFlipFrame( src_fid, roi_id, flags)
struct FCT	    *src_fid;		/* Source frame id		    */
struct ROI	    *roi_id;		/* ROI block pointer		    */
long		     flags;		/* Flip modifier flags	    	    */
{
struct	FCT *dst_fid;				  /* Destination fid	    */
struct	UDP src_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Source  UDP descriptor */
struct	UDP dst_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Dest    UDP descriptor */
struct	UDP tmp_udp[MAX_NUMBER_OF_COMPONENTS];	  /* scratch UDP descriptor */

long	i;				/* scratch index variable	    */
long	local_flags;
long	num_of_comp;			/* number of spectral components    */
long	rect_roi_info	= 0;
long	spectral_type;			/* spectral type		    */
long	status;				/* status of layer 2 routine	    */

unsigned long	idx;			/* index variable		    */
unsigned long	planes_per_pixel;	/* planes in frame		    */

unsigned long	new_data_offset[MAX_NUMBER_OF_COMPONENTS];
unsigned long	new_scanline_stride[MAX_NUMBER_OF_COMPONENTS];
unsigned long	pixel_stride[MAX_NUMBER_OF_COMPONENTS];
unsigned long	pixels_per_line[MAX_NUMBER_OF_COMPONENTS];

struct	 ITMLST	*itmlst, *itmlst_ptr;	/* item list for allocate frame	    */

/*****************************************************************************
**	    start of validation & conformance checking                      **
*****************************************************************************/
if ( VERIFY_ON_ )
    ImgVerifyFrame(src_fid,0);

if (roi_id == 0)
    {
    /*
    ** Obtain frame attributes for itemlist
    */
    _ImgGet(src_fid,Img_PlanesPerPixel,&planes_per_pixel,LONGSIZE,0,0);

    itmlst = (struct ITMLST *) _ImgCreateItmlst((2 * planes_per_pixel) + 1);
    itmlst_ptr = itmlst;

    for (idx = 0; idx < planes_per_pixel; idx++)
        {
        _ImgGet(src_fid,Img_PixelStride,&pixel_stride[idx],LONGSIZE,0,idx);
        _ImgGet(src_fid,Img_PixelsPerLine,&pixels_per_line[idx],LONGSIZE,0,idx);
	new_data_offset[idx] = 0;
	new_scanline_stride[idx] = 
	    (((pixels_per_line[idx] * pixel_stride[idx]) + 7)/8) * 8;

	itmlst_ptr->ItmL_Code = Img_DataOffset;
	itmlst_ptr->ItmL_Length = sizeof(long);
	itmlst_ptr->ItmA_Buffer =  (char *) &new_data_offset[idx];
	itmlst_ptr->ItmA_Retlen = 0;
	itmlst_ptr->ItmL_Index = idx;
	itmlst_ptr++;

	itmlst_ptr->ItmL_Code = Img_ScanlineStride;
	itmlst_ptr->ItmL_Length = sizeof(long);
	itmlst_ptr->ItmA_Buffer =  (char *) &new_scanline_stride[idx];
	itmlst_ptr->ItmA_Retlen = 0;
	itmlst_ptr->ItmL_Index = idx;
	itmlst_ptr++;

        }
    dst_fid = (struct FCT *) ImgAllocateFrame(0,itmlst,src_fid,0);
    }/* end no roi_id */
else
    dst_fid = ImgCopyFrame(src_fid, 0);
		
_ImgGet(src_fid,Img_ImageDataClass, &spectral_type,sizeof(spectral_type),0,0);
_ImgGet(src_fid,Img_NumberOfComp,   &num_of_comp,sizeof(num_of_comp),0,0);

/*
** Initialize temporary data plane addresses for double flips
*/
for (i=0;i<num_of_comp;i++) 
    {
    tmp_udp[i].UdpA_Base = 0;
    }

/*
** Validate ROI and process source udp
*/
for (i=0;i<num_of_comp;i++)
    {
    /*
    ** Validate and set ROI for each component
    */
    _ImgGet(src_fid,Img_Udp,&src_udp[i],sizeof(struct UDP),0,i);
    _ImgGet(dst_fid,Img_Udp,&dst_udp[i],sizeof(struct UDP),0,i);
    if (roi_id != 0)
	{ 
	if (! _ImgValidateRoi(roi_id,&src_udp[i]))
	    ChfStop(1,ImgX_INVROI);
	else
	    {
	    _ImgSetRoi(&src_udp[i],roi_id);
	    _ImgSetRoi(&dst_udp[i],roi_id);
	    }
	};
    };/* end for components */
/*****************************************************************************
**	    End ROI validation						    **
*****************************************************************************/


/*****************************************************************************
**	    Start Dispatch to layer 2 flip routines			    **
*****************************************************************************/
_ImgGet( src_fid, Img_RectRoiInfo, &rect_roi_info, LONGSIZE, 0, 0 );
if ( roi_id != 0 )
    local_flags = IpsM_RetainSrcDim;
else
    local_flags = 0;

for (i=0;i<num_of_comp;i++)
    {
    switch (flags & (ImgM_FlipHorizontal|ImgM_FlipVertical))
        {/* switch on flip flags */
        case 0:
            /*
            ** No flags set copy udp's
            */
	    if (spectral_type == ImgK_ClassBitonal)
	        status =
		    (*ImgA_VectorTable[ImgK_CopyBitonal])
			(&src_udp[i],&dst_udp[i],local_flags);
	    else
	        status =
		    (*ImgA_VectorTable[ImgK_Copy])
			(&src_udp[i],&dst_udp[i],local_flags);

	    if ((status & 1) != 1)
		_ImgErrorHandler(status);
	    break; 

        case (ImgM_FlipHorizontal|ImgM_FlipVertical):

	    /*
	    ** Both flags set is equivalent to 180 degree rotate
	    */

	    if (spectral_type == ImgK_ClassBitonal)
		{
	        status = 
		    (*ImgA_VectorTable[ImgK_FlipHorizontalBitonal])
			(&src_udp[i],&tmp_udp[i],local_flags);
		if ((status & 1) != 1)
		    _ImgErrorHandler(status);
		 /*
		 ** flip again about the x axis
		 */
		 status = 
		     (*ImgA_VectorTable[ImgK_FlipVerticalBitonal])
			 (&tmp_udp[i],&dst_udp[i], local_flags );
		 if ((status & 1) != 1)
		    _ImgErrorHandler(status);
		 }
            else
		{
		/*
		** Non-bitonal case
		*/
	        status = 
		    (*ImgA_VectorTable[ImgK_FlipHorizontal])
			(&src_udp[i],&tmp_udp[i],local_flags);
		if ((status & 1) != 1)
		    _ImgErrorHandler(status);
		 /*
		 ** flip again about the x axis
		 */
		 status = 
		     (*ImgA_VectorTable[ImgK_FlipVertical])
			 (&tmp_udp[i],&dst_udp[i],local_flags);
		 if ((status & 1) != 1)
		    _ImgErrorHandler(status);
		 }
	    /*
	    ** deallocate temporary scratch udp
	    */
	    (*ImgA_VectorTable[ImgK_FreeDataPlane])(tmp_udp[i].UdpA_Base);
	    break;

	case (ImgM_FlipHorizontal):
	    if (spectral_type == ImgK_ClassBitonal)
	       status = (*ImgA_VectorTable[ImgK_FlipHorizontalBitonal])
			    (&src_udp[i],&dst_udp[i],local_flags);
	    else
	       status =	(*ImgA_VectorTable[ImgK_FlipHorizontal])
			    (&src_udp[i],&dst_udp[i],local_flags);
	    if ((status & 1) != 1)
		_ImgErrorHandler(status);
	    break; 

        case (ImgM_FlipVertical):
	    if (spectral_type == ImgK_ClassBitonal)
	        status = (*ImgA_VectorTable[ImgK_FlipVerticalBitonal])
				(&src_udp[i],&dst_udp[i],local_flags);
	    else
	        status = (*ImgA_VectorTable[ImgK_FlipVertical])
				(&src_udp[i],&dst_udp[i],local_flags);

	    if ((status & 1) != 1)
		_ImgErrorHandler(status);
	    break; 

        default:
	    ChfStop(1,ImgX_INVFLGARG);	
	    break;
        }/* end switch on flip flags */
    }/* end component loop */

/*****************************************************************************
**	    End dispatch to layer 2					    **
*****************************************************************************/
if ( VERIFY_ON_ )
    ImgVerifyFrame(dst_fid,0);

return (dst_fid);
} /* end of ImgFlipFrame */
