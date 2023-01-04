
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
**  Img_Rotate
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	This module contains the routine that dispatches to image-type
**	specific functions for rotating images.
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
**	October 2, 1989
**
*****************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$ROTATE();       /* Rotate VMS bindings			    */
struct FCT *IMG$ROTATE_FRAME(); /* V3 Rotate VMS bindings		    */
#endif
struct FCT *ImgRotate();	/* Rotate portable bindings	 	    */
struct FCT *ImgRotateFrame();   /* V3 portable bindings  	            */
#endif

/*
**  Include files 
*/
#include <img/ChfDef.h>                            /* Condition handler          */
#include <img/ImgDef.h>                            /* Image definitions          */
#include <ImgDefP.h>			       /* Private definitions	     */
#include <ImgMacros.h>                         /* Common macro definitions   */
#include <ImgVectorTable.h>                    /* Dispatch definitions       */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  External References from ISL                      <- from module ->
*/
#ifdef NODAS_PROTO
unsigned long	ImgCvtCompSpaceOrg();
void		ImgDeallocateFrame();
unsigned long	ImgGetFrameSize();
unsigned long	ImgSetFrameSize();
unsigned long	ImgStandardizeFrame();
long		ImgSetRectRoi();		    /* IMG_ROI_UTILS		    */
long		ImgUnsetRectRoi();		    /* IMG_ROI_UTILS		    */
void		ImgVerifyFrame();

struct FCT  *_ImgCheckNormal();		    /* img__verify_utils	    */
struct FCT  *_ImgCloneFrame();		    /* img__frame_utils		    */
void	     _ImgErrorHandler();	    /* img_error_handler	    */
struct FCT  *_ImgGet();			    /* img__attribute_access_utils  */
long	     _ImgGetVerifyStatus();
struct FCT  *_ImgPut();			    /* img__attribute_access_utils  */
struct UDP  *_ImgSetRoi();		    /* img__roi			    */
long	     _ImgValidateRoi();		    /* img_roi_utils		    */
long	     _ImgRotateBitonal();	    /* ImgRotateBitonal		    */
long	     _ImgRotateOrthogonal();	    /* ImgRotateCont		    */
long	     _ImgRotateNearestNeighbor();   /* ImgRotateCont		    */
long	     _ImgRotateInterpolation();	    /* ImgRotateCont		    */
long	     _ImgVerifyStandardFormat();

/*
**  External References from the Condition Handling Facility
*/
void                    ChfStop();              /* Exception stop */
void                    ChfSignal();            /* Exception signal */
#endif
/*
** Status codes
*/
#include <img/ImgStatusCodes.h>
 

/****************************************************************************
**              BEGINNING OF V2.0 ENTRY POINTS                             **
****************************************************************************/
/*
**  NOTE:
**
**      The "alignment" parameter of V2.0 has been decommitted
**      and is accepted but ignored in both of these calls.
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$ROTATE( src_fid, angle, roi_id, flags, alignment )
struct FCT *src_fid;		/* REQUIRED source frame id	    */
float	   *angle;		/* REQUIRED angle of rotation	    */
struct ROI *roi_id;		/* OPTIONAL ROI block pointer	    */
long	   flags;		/* OPTIONAL FLAGS		    */
long	   alignment;		/* OPTIONAL ALIGNMENT indicator	    */
{
return (ImgRotate( src_fid, angle, roi_id, flags, alignment));
}
#endif

struct FCT *ImgRotate( src_fid, angle, roi_id, flags, alignment)
struct FCT *src_fid;		/* REQUIRED source frame id	    */
float	   *angle;		/* REQUIRED angle of rotation	    */
struct ROI *roi_id;		/* OPTIONAL ROI block pointer	    */
long	   flags;		/* OPTIONAL FLAGS		    */
long	   alignment;		/* OPTIONAL ALIGNMENT indicator	    */
{
/*
** Variables used in conversion
*/
struct   FCT	*ret_fid;
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
    tmp_fid_1 = src_fid;

/*
** Call V3.0 entry point ...
*/
ImgSetRectRoi( tmp_fid_1, roi_id, 0 );
tmp_fid_2 = ImgRotateFrame( tmp_fid_1, angle, flags);
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
} /* end of ImgRotate */

/****************************************************************************
**               BEGINNING OF CURRENT ENTRY POINTS                         **
****************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$ROTATE_FRAME( src_fid, angle, flags)
struct FCT *src_fid;		/* REQUIRED source frame id	    */
float	   *angle;		/* REQUIRED angle of rotation	    */
long	   flags;		/* OPTIONAL FLAGS		    */
{
return (ImgRotateFrame( src_fid, angle, flags));
} /* end of IMG$ROTATE_FRAME */
#endif

/***********************************************************************
**  ImgRotateFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return an image frame to the user which has been rotated from
**	a supplied image frame.
**
**      1. Parse input parameters
**      2. Validate/Conformance checking of the fid
**      3. parameter assembly for layer 2 rotate routines
**      4. Dispatch to layer 2 rotate routines
**      5. Handling of layer 2 rotate routine returns
**
**  FORMAL PARAMETERS:
**	src_fid	    - frame id of source image frame	    by value
**	angle	    - angle of rotation (degrees)	    by reference
**	FLAGS	    - optional processing flags		    by value
**
**  IMPLICIT INPUTS:
**      none
**
**  IMPLICIT OUTPUTS:
**      none
**
**  FUNCTION VALUE:
**      retfid - frame id of created image frame
**
**  SIGNAL CODES:
**
**      ImgX_INVARGCNT - Invalid Argument Count
**      ImgX_INVCMPTYP - Invalid Compression Type
**      ImgX_PXLTOOLRG - Pixels greater tan 25 bits not supported
**
**  SIDE EFFECTS:
**	none
**
** RESTRICTIONS:
**      1. The input FID must be uncompressed
**      2. The input FID must be organized as band_interleaved_by_plane
**
*****************************************************************************/
struct FCT *ImgRotateFrame( src_fid, angle, flags)
struct FCT *src_fid;		/* REQUIRED source frame id	    */
float	   *angle;		/* REQUIRED angle of rotation	    */
long	   flags;		/* OPTIONAL FLAGS		    */
{
float	fraction	= 0.0;
float	local_angle	= *angle;
float	scale_x_factor;
float	scale_y_factor;
long	data_class;				/* data class		    */
long	dst_line_cnt;
long	dst_pxl_cnt;
int	dst_x_size;
int	dst_y_size;
long	int_angle	= 0;
long	number_of_comp	= 0;			/* no. of spectral component*/
long	src_line_cnt;
long	src_pxl_cnt;
int	src_x_size;
int	src_y_size;
long	status,index	= 0;			/* scratch index counter    */
long	norm_angle	= 0;
struct FCT  *dst_fid;				/* destination FID	    */
struct UDP   dst_udp[MAX_NUMBER_OF_COMPONENTS];	/* destination UDP descr    */
struct UDP   src_udp[MAX_NUMBER_OF_COMPONENTS];	/* source UDP descriptor    */


/*****************************************************************
**      start of validation and conformance checking            **
*****************************************************************/
if ( VERIFY_ON_ )
    ImgVerifyFrame( src_fid, 0 );

/*************************************************************
**            End of validation and conformance checking    **
*************************************************************/

/*************************************************************
**    Start dispatch to layer 2 rotate routines             **
*************************************************************/

    _ImgGet (src_fid, Img_NumberOfComp,   &number_of_comp,sizeof(long),0,0);
    _ImgGet (src_fid, Img_ImageDataClass, &data_class,    sizeof(long),0,0);

    /*
    ** Get source udp and call lower level routines with an empty dst_udp
    */
    for (index = 0; index < number_of_comp; index++)
	{
        _ImgGet(src_fid,Img_Udp,&src_udp[index],sizeof(struct UDP),0,index);
	dst_udp[index].UdpA_Base = 0;    
	}

if (local_angle == 0.0)
    {
    for (index = 0; index < number_of_comp; index++)
	{
	if (data_class == ImgK_ClassBitonal)
	    status = (*ImgA_VectorTable[ImgK_CopyBitonal])
			(&src_udp[index],&dst_udp[index], 0); 
	else
	    status = (*ImgA_VectorTable[ImgK_Copy])
			(&src_udp[index],&dst_udp[index], 0); 
        if ((status & 1) != 1)
            _ImgErrorHandler(status);
	}
    }  /* end if angle zero */
else
    {
    switch( data_class )
        {
        case ImgK_ClassBitonal:
	    {
            status = (*ImgA_VectorTable[ImgK_RotateBitonal])
                    (&src_udp[0],&dst_udp[0],&local_angle,flags);
            if ((status & 1) != 1)
                _ImgErrorHandler(status);
   	    break;
	    }
        case ImgK_ClassGreyscale:
        case ImgK_ClassMultispect:
	    {
	    /* Normalize the angle */
	    int_angle = local_angle;
	    fraction = local_angle - int_angle;
	    int_angle %= 360;
	    local_angle = int_angle + fraction; 
	    norm_angle = int_angle % 90;
	    for (index = 0; index < number_of_comp; index++)
	        {
	        if ((norm_angle == 0) && (fraction == 0.0))
    		    {
		    status = (*ImgA_VectorTable[ImgK_RotateOrthogonal])
                           (&src_udp[index],&dst_udp[index],&int_angle,flags);
                    if ((status & 1) != 1)
       	                _ImgErrorHandler(status);
                    }
                else
                    {
    		    if ( flags & ImgM_NearestNeighbor )
        	        {
	                status = (*ImgA_VectorTable[ImgK_RotateNearestNeighbor])
                            (&src_udp[index],&dst_udp[index],&local_angle,flags);
                        if ((status & 1) != 1)
       	                    _ImgErrorHandler(status);
		        }
     		    else
		        {
       		        status = (*ImgA_VectorTable[ImgK_RotateInterpolation])
                            (&src_udp[index],&dst_udp[index],&local_angle,flags);
        	        if ((status & 1) != 1)
       	                    _ImgErrorHandler(status);
		        } /* end else of interpolation or nearest neibor check*/
    		    } /* end else of Orthogonal angle check*/
		} /* end loop */
	    break;
	    }/* end case greyscale or multispectral */
        default:
    	    ChfStop( 1,  ImgX_NOTYPESUP );
        } /* end switch on data class */
    }/* end else angle not zero */
/*************************************************************
**      End dispatch to layer 2 Rotate routines             **
*************************************************************/
/*************************************************************
**              begin post processing                       **
*************************************************************/
dst_fid = (struct FCT *) _ImgCloneFrame ( src_fid );
for (index = 0; index < number_of_comp; index++)
    _ImgPut(dst_fid,Img_Udp,&dst_udp[index], sizeof(struct UDP),index);

/*
** Calculate and set frame size of rotated frame; (Bmus are int's, but cast as
** floats to satisfy ImgGetFrameSize()).
*/
ImgGetFrameSize( src_fid, (float *)&src_x_size, (float *)&src_y_size, ImgK_Bmus );
_ImgGet( src_fid, Img_PixelsPerLine, &src_pxl_cnt, sizeof(src_pxl_cnt), 0, 0 );
_ImgGet( src_fid, Img_NumberOfLines, &src_line_cnt, sizeof(src_line_cnt), 0, 0);
_ImgGet( dst_fid, Img_PixelsPerLine, &dst_pxl_cnt, sizeof(dst_pxl_cnt), 0, 0 );
_ImgGet( dst_fid, Img_NumberOfLines, &dst_line_cnt, sizeof(dst_line_cnt), 0, 0);

scale_x_factor = (float)dst_pxl_cnt / (float)src_pxl_cnt;
scale_y_factor = (float)dst_line_cnt / (float)src_line_cnt;

dst_x_size = (float)src_x_size * scale_x_factor;
dst_y_size = (float)src_y_size * scale_y_factor;

ImgSetFrameSize( dst_fid, (float *)&dst_x_size, (float *)&dst_y_size, ImgK_Bmus );

if ( VERIFY_ON_ )
    ImgVerifyFrame( dst_fid, 0 );

return( dst_fid );
} /* end of ImgRotateFrame */
