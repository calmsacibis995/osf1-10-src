
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
**  Img_Filter
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains routines for filter support
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
struct FCT *IMG$FILTER();
struct FCT *IMG$FILTER_FRAME();
#endif
#ifdef NODAS_PROTO
struct FCT *ImgFilter();
struct FCT *ImgFilterFrame();
#endif

#include <img/ChfDef.h>                         /* Condition handler	    */
#include <img/ImgDef.h>                         /* Image definitions	    */
#include <ImgDefP.h>			    /* ISL private definitions	    */
#include <ImgMacros.h>                      /* Common macro definitions	    */
#include <ImgVectorTable.h>                 /* Dispatch definitions	    */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif
#if defined(__VMS) || defined(VMS)
#include <descrip.h>			    /* descriptor codes             */
#endif
/*
**  External References from ISL               <----   from module   ---->
*/
#ifdef NODAS_PROTO
unsigned long	ImgAllocateFrame();
unsigned long	ImgCvtCompSpaceOrg();	    /* IMG_CONVERT_UTILS	    */
void		ImgDeallocateFrame();	    /* IMG_FRAME_UTILS		    */
unsigned long	ImgSetFrameSize();
void		ImgSetRectRoi();
unsigned long	ImgStandardizeFrame();	    /* IMG_FRAME_UTILS		    */
void		ImgUnsetRectRoi();
void		ImgVerifyFrame();	    /* IMG_FRAME_UTILS		    */

struct  FCT *_ImgCloneFrame();		    /* img__frame_utils		    */
void         _ImgErrorHandler();	    /* img_error_handler	    */
struct  FCT *_ImgGet();			    /* img__attribute_access_utils  */
long	     _ImgGetVerifyStatus();
struct  FCT *_ImgPut();			    /* img__attribute_access_utils  */
struct  UDP *_ImgSetRoi();		    /* img__roi			    */
long         _ImgValidateRoi();		    /* img_roi_utils		    */
long	     _ImgVerifyStandardFormat();    /* IMG__VERIFY_UTILS	    */

/*
**  External References from the Condition Handling Facility
**					
*/
void		 ChfSignal();			/* Signal Exception	*/
void		 ChfStop();			/* Exception Stop	*/
#endif

/*
** Status codes
*/
#include <img/ImgStatusCodes.h>                    /* Status Codes               */


/*
**  Equated Symbols (from descrip.h)
**  On Ultrix, the DSC codes are in ImgDefP.h because descrip.h does not
**  exist.  
*/

/*
**  Filter kernels
*/
#if defined(__VAXC) || defined(VAXC)
 readonly static float FilterClear[9] =          { 0, 0, 0, 0, 1, 0, 0, 0, 0};
 readonly static float FilterLightSharpen[9] =   { 0,-1, 0,-1, 5,-1, 0,-1, 0};
 readonly static float FilterHeavySharpen[9] =   {-1,-1,-1,-1, 9,-1,-1,-1,-1};
 readonly static float FilterEdge[9] =           {-1, 0, 1,-1, 0, 1,-1, 0, 1};
 readonly static float FilterLightSmooth[9] =
                             {0,1.0/5.0,0,1.0/5.0,1.0/5.0,1.0/5.0,0,1.0/5.0,0};
 readonly static float FilterHeavySmooth[9] =
     {1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0};
 readonly static float FilterLaplacian[9] =
   {-1.0/8.0,-1.0/8.0,-1.0/8.0,-1.0/8.0,1,-1.0/8.0,-1.0/8.0,-1.0/8.0,-1.0/8.0};
#else
 static float FilterClear[9]=                    { 0, 0, 0, 0, 1, 0, 0, 0, 0};
 static float FilterLightSharpen[9]=             { 0,-1, 0,-1, 5,-1, 0,-1, 0};
 static float FilterHeavySharpen[9]=             {-1,-1,-1,-1, 9,-1,-1,-1,-1};
 static float FilterEdge[9]=                     {-1, 0, 1,-1, 0, 1,-1, 0, 1};
 static float FilterLightSmooth[9]=             
                            {0,1.0/5.0,0,1.0/5.0,1.0/5.0,1.0/5.0,0,1.0/5.0,0};
 static float FilterHeavySmooth[9]=
          {1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,0};
 static float FilterLaplacian[9]= 
  {-1.0/8.0,-1.0/8.0,-1.0/8.0,-1.0/8.0,1,-1.0/8.0,-1.0/8.0,-1.0/8.0,-1.0/8.0};
#endif

/****************************************************************************
**              BEGINNING OF V2.0 ENTRY POINTS                             **
****************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$FILTER( src_fid, filter_type, roi_id, flags )
struct FCT  *src_fid;           /* REQUIRED source frame id            */
unsigned long filter_type;	/* Flag indicating which filter to use */
struct ROI  *roi_id;            /* OPTIONAL ROI block pointer          */
long	     flags;		/* flags			       */
{

return (ImgFilter( src_fid, filter_type, roi_id, flags ));
} /* end of IMG$FILTER */
#endif


struct FCT *ImgFilter( src_fid, filter_type, roi_id, flags )
struct FCT  *src_fid;           /* REQUIRED source frame id            */
unsigned long filter_type;	/* Flag indicating which filter to use */
struct ROI  *roi_id;            /* OPTIONAL ROI block pointer          */
long	     flags;		/* flags			       */
{
struct FCT *ret_fid;
long	 std_cs_org	    = ImgK_BandIntrlvdByPlane;
long	 saved_cs_org;
long	 saved_data_class;
long	 saved_dp_signif;
long	 status;
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
** Call V3.0 entry point ... and process roi_id if specified
*/
ImgSetRectRoi( tmp_fid_1, roi_id, 0);
tmp_fid_2 = ImgFilterFrame( tmp_fid_1, filter_type, flags );
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
} /* end of ImgFilter */

/****************************************************************************
**                       END OF V2.0 ENTRY POINTS                          **
****************************************************************************/

/****************************************************************************
**               BEGINNING OF CURRENT ENTRY POINTS                         **
****************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$FILTER_FRAME( src_fid, filter_type, flags )
struct FCT	*src_fid;       /* REQUIRED source frame id           */
unsigned long	 filter_type;	/* Flag indicates which filter to use */
long		 flags;		/* flags			      */
{

return (ImgFilterFrame( src_fid, filter_type, flags ));
} /* end of IMG$FILTER_FRAME */
#endif


/*********************************************************************
**  ImgFilterFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	Filters an image
**
**      1. Parse input parameters
**      2. Validate/Conformance checking of the fid
**      3. parameter assembly for layer 2 filter routines
**      4. Dispatch to layer 2 filter routines
**      5. Handling of layer 2 filter routine returns
**
**  FORMAL PARAMETERS:
**	srcfid	    - source frame id
**	filter_type - type of filter to be applied to image
**	flags	    - user supplied parameter list (OPTIONAL)
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
*****************************************************************************/

struct FCT *ImgFilterFrame( src_fid, filter_type, flags )
struct FCT  *src_fid;           /* REQUIRED source frame id           */
long	     filter_type;	/* Flag indicates which filter to use */
long	     flags;		/* Flags			      */
{
    struct UDP src_udp[MAX_NUMBER_OF_COMPONENTS];/* source UDP descriptor */
    struct UDP dst_udp[MAX_NUMBER_OF_COMPONENTS];/* destination UDP descr */
    struct FCT *dst_fid;                         /* destination FID       */
    long number_of_comp=0;	/*  Number of Components of image         */
    long index=0;		/*  Index for getting bits per comp.      */
    long data_class = 0;        /* data class                             */
    long status = 0;            /* status from layer II call              */
    float x_size, y_size;
/*
** Kernel variables for layer 2
*/
float		*kernel;
unsigned long	k_width;
unsigned long	k_height;
unsigned long	k_type;


    /*****************************************************************
    **      start of validation and conformance checking            **
    *****************************************************************/
    /*
    **  Verify Integrity of frame and image data
    */
    if ( VERIFY_ON_ )
	ImgVerifyFrame(src_fid, 0);

    /*************************************************************
    **            End of validation and conformance checking    **
    *************************************************************/

    /*************************************************************
    **    Start dispatch to layer 2 convolve routines           **
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

    /*
    ** Set up kernel variables before dispatch
    */
    k_width = 3;
    k_height = 3;
    k_type = IpsK_DTypeF;
    switch(filter_type)
	{
	case ImgK_FilterClear:
	    kernel = FilterClear; 
	    break;
	case ImgK_FilterLightSharpen:
	    kernel = FilterLightSharpen;
	    break;
	case ImgK_FilterHeavySharpen:
	    kernel = FilterHeavySharpen;
	    break;
	case ImgK_FilterEdge:
	    kernel = FilterEdge;
	    break;
	case ImgK_FilterLightSmooth:
	    kernel = FilterLightSmooth;
	    break;
	case ImgK_FilterHeavySmooth:
	    kernel = FilterHeavySmooth;
	    break;
	case ImgK_FilterLaplacian:
	    kernel = FilterLaplacian;
	    break;
	default:
	    ChfStop( 1, ImgX_PARAMCONF);
	}/* end filter type dispatch */    

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
    dst_fid = (struct FCT *)ImgAllocateFrame( 0, 0, src_fid, 
						ImgM_NoDataPlaneAlloc );
    for (index = 0; index < number_of_comp; index++)
        _ImgPut(dst_fid,Img_Udp,&dst_udp[index], sizeof(struct UDP),index);
    x_size = dst_udp[0].UdpL_PxlPerScn;
    y_size = dst_udp[0].UdpL_ScnCnt;
    dst_fid = ImgSetFrameSize (dst_fid,&x_size,&y_size,ImgK_Bmus);

    if ( VERIFY_ON_ )
	ImgVerifyFrame( dst_fid, 0 );

    return(dst_fid);
} /* end ImgFilterFrame */
