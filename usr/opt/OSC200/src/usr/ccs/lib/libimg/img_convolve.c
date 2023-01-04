
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
**  Img_Convolve
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains routines for convolution support
**
**	
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Michael D. O'Connor 
**      Revised for V3.0 by Karen Rodwell
**
**  CREATION DATE:
**
**	November 20, 1989
**
*****************************************************************************/

/*
**  Table of contents
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$CONVOLVE();
struct FCT *IMG$CONVOLVE_FRAME();
#endif
#ifdef NODAS_PROTO
struct FCT *ImgConvolve();
struct FCT *ImgConvolveFrame();
#endif

#include <img/ChfDef.h>                            /* Condition handler         */
#include <img/ImgDef.h>                            /* Image definitions         */
#include <ImgDefP.h>			       /* Private image definitions */	
#include <ImgMacros.h>                         /* Common macro definitions  */
#include <ImgVectorTable.h>                    /* Dispatch definitions      */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  External References from ISL	    <- from module ->
*/
#ifdef NODAS_PROTO
long	    ImgCvtCompSpaceOrg();
void	    ImgDeallocateFrame();
long	    ImgStandardizeFrame();
long	    ImgSetFrameSize();
void	    ImgSetRectRoi();
void	    ImgUnsetRectRoi();
void	    ImgVerifyFrame();		    /* IMG_FRAME_UTILS		    */

void	     _ImgErrorHandler();	    /* IMG_ERROR_HANDLER	    */
struct FCT  *_ImgCloneFrame();
struct FCT  *_ImgGet();			    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
long	     _ImgGetVerifyStatus();
long	     _ImgPut();
long	     _ImgVerifyStandardFormat();    /* IMG__VERIFY_UTILS	    */

/*
**  External References from the Condition Handling Facility
**					
*/
void		 ChfSignal();
void		 ChfStop();
#endif

/*
**	Status codes
*/
#include    <img/ImgStatusCodes.h>


/****************************************************************************
**              BEGINNING OF V2.0 ENTRY POINTS                             **
****************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$CONVOLVE(src_fid,desc,roi_id,flags)
struct FCT *src_fid;            /* REQUIRED source frame id         */
struct A2D_DESC *desc;		/* descriptor structure in ImgDefP.h*/
struct ROI *roi_id;             /* OPTIONAL ROI block pointer       */
long flags;			/* User flags			    */
{

return (ImgConvolve( src_fid, desc, roi_id, flags ));
} /* end of IMG$CONVOLVE */
#endif


struct FCT *ImgConvolve( src_fid, desc, roi_id, flags )
struct FCT *src_fid;            /* REQUIRED source frame id         */
struct A2D_DESC *desc;		/* descriptor structure in ImgDefP.h*/
struct ROI *roi_id;             /* OPTIONAL ROI block pointer       */
long flags;
{
/*
** Variables used in conversion
*/
struct FCT *ret_fid;
unsigned long	std_cs_org	    = ImgK_BandIntrlvdByPlane;
unsigned long	saved_cs_org;
unsigned long	saved_data_class;
unsigned long	saved_dp_signif;
unsigned long	status;
struct   FCT	*tmp_fid_1;
struct   FCT	*tmp_fid_2;

unsigned char	*kernel;
unsigned long	k_width;
unsigned long	k_height;
unsigned long	k_type;

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
    tmp_fid_1 = (struct FCT *) ImgStandardizeFrame( src_fid, 0 );
else
    tmp_fid_1 = (struct FCT *) src_fid;

/*
** Break down descriptor into its components
*/
switch (desc->A2D_B_DTYPE)
    {
    case DSC_K_DTYPE_BU:
	k_type = ImgK_DTypeBU;
	kernel = (unsigned char *)desc->A2D_A_POINTER;
	break;
    case DSC_K_DTYPE_WU:
	k_type = ImgK_DTypeWU;
	kernel = (unsigned char *)desc->A2D_A_POINTER;
	break;
    case DSC_K_DTYPE_LU:
	k_type = ImgK_DTypeLU;
	kernel = (unsigned char *)desc->A2D_A_POINTER;
	break;
    case DSC_K_DTYPE_F:
	k_type = ImgK_DTypeF;
	kernel = (unsigned char *)desc->A2D_A_POINTER;
	break;
    default:
	ChfStop( 1, ImgX_UNSOPTION);
    }/* end switch on dtype */

k_width  = desc->A2D_L_M1;
k_height = desc->A2D_L_M2;


/*
** Call V3.0 entry point ... and process roi_id if specified
*/
ImgSetRectRoi( tmp_fid_1, roi_id, 0);
tmp_fid_2 = ImgConvolveFrame(tmp_fid_1,kernel,k_width,k_height,k_type,flags);
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

return (struct FCT *) ret_fid;
} /* end of ImgConvolve */


#if defined(__VMS) || defined(VMS)
struct FCT *IMG$CONVOLVE_FRAME(src_fid,kernel,k_width,k_height,k_type,flags)
struct FCT *src_fid;            /* REQUIRED source frame id		    */
unsigned char *kernel;		/* address of 2d kernel array		    */
unsigned long k_width;		/* width of kernel array in elements	    */
unsigned long k_height;		/* height of kernel array in elements	    */
unsigned long k_type;		/* data type of kernel elements		    */
unsigned long flags;		/* processing flags			    */
{

return (ImgConvolveFrame(src_fid,kernel,k_width,k_height,k_type,flags));
} /* IMG$CONVOLVE_FRAME */
#endif

/*********************************************************************
**  Img_Convolve
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convolves a kernel with an image
**
**      1. Parse input parameters
**      2. Validate/Conformance checking of the fid
**      3. parameter assembly for layer 2 convolution routines
**      4. Dispatch to layer 2 convolution routines
**      5. Handling of layer 2 convolution routine returns
**
**  FORMAL PARAMETERS:
**
**  src_fid	- REQUIRED source frame id
**  kernel	- address of 2d kernel array
**  k_width	- width of kernel array in elements
**  k_height	- height of kernel array in elements
**  k_type	- data type of kernel elements
**  flags	- processing flags
**
**  IMPLICIT INPUTS:
**      none
**
**  IMPLICIT OUTPUTS:
**      none
**
**  FUNCTION VALUE:
**      retfid - return frame id
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	- invalid argument count
**	ImgX_UNSOPTION	- unsupported option
**	ImgX_PARAMCONF  - parameter conflict
**	ImgX_PXLTOOLRG  - pixel too large
**
**  SIDE EFFECTS:
**	none
**
**  RESTRICTIONS:
**
**  1. The input fid must be uncompressed
**  2. The input fid must be organized as band_interleaved_by_plane
*****************************************************************************/

struct FCT *ImgConvolveFrame(src_fid,kernel,k_width,k_height,k_type,flags)
struct FCT *src_fid;            /* REQUIRED source frame id		    */
unsigned char *kernel;		/* address of 2d kernel array		    */
unsigned long k_width;		/* width of kernel array in elements	    */
unsigned long k_height;		/* height of kernel array in elements	    */
unsigned long k_type;		/* data type of kernel elements		    */
unsigned long flags;		/* processing flags			    */
{
    struct UDP src_udp[MAX_NUMBER_OF_COMPONENTS];/* source UDP descriptor */
    struct UDP dst_udp[MAX_NUMBER_OF_COMPONENTS];/* destination UDP descr */
    struct FCT *dst_fid;                         /* destination FID       */
    long number_of_comp=0;	/*  Number of Components of image         */
    long index=0;		/*  Index for getting bits per comp.      */
    long data_class = 0;        /* data class                             */
    long status = 0;            /* status from layer II call              */

/*****************************************************************
**      start of validation and conformance checking            **
*****************************************************************/
    /*
    **  Verify Integrity of frame and image data
    */
    if ( VERIFY_ON_ )
	ImgVerifyFrame( src_fid, 0 );


    /*************************************************************
    **            End of validation and conformance checking    **
    *************************************************************/

    /*************************************************************
    **    Start dispatch to layer 2 filter routines             **
    *************************************************************/
    
    _ImgGet (src_fid, Img_NumberOfComp,  &number_of_comp,sizeof(long),0,0);
    _ImgGet (src_fid, Img_ImageDataClass, &data_class, sizeof(long), 0, 0);

    /*
    ** Get source udp and call lower level routines with an empty dst_udp
    */
    for (index = 0; index < number_of_comp; index++)
	{
        _ImgGet(src_fid,Img_Udp,&src_udp[index],sizeof(struct UDP),0,index);
	dst_udp[index].UdpA_Base = 0;    
	}

    switch(data_class)
	{
        case ImgK_ClassGreyscale :
        case ImgK_ClassMultispect :
            for (index = 0; index < number_of_comp; index++)
                {
                status = (*ImgA_VectorTable[ImgK_Convolve])
		    (&src_udp[index],&dst_udp[index],
			kernel,k_width,k_height,k_type,flags);
                if ((status & 1) != 1)
                    _ImgErrorHandler(status);
                }
  
          break;
        case ImgK_ClassBitonal:
        default:
            status = (*ImgA_VectorTable[ImgK_CopyBitonal])
                 (&src_udp[0],&dst_udp[0],0);
            if ((status & 1) != 1)
                    _ImgErrorHandler(status);
            break;
        } /* end switch on data class */

    /*************************************************************
    **      End dispatch to layer 2 Filter routines             **
    *************************************************************/
    /*************************************************************
    **              begin post processing                       **
    *************************************************************/

    dst_fid = _ImgCloneFrame ( src_fid );
    for (index = 0; index < number_of_comp; index++)
        _ImgPut(dst_fid,Img_Udp,&dst_udp[index], sizeof(struct UDP),index);
    dst_fid = ImgSetFrameSize
        (dst_fid,(float *)&dst_udp[0].UdpL_PxlPerScn,(float *)&dst_udp[0].UdpL_ScnCnt,ImgK_Bmus);

    if ( VERIFY_ON_ )
	ImgVerifyFrame( dst_fid, 0 );

    return(dst_fid);
} /* end ImgConvolveFrame */
