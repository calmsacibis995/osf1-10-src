
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
**  IMG_COMBINE
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains the user level service and support routines
**	for IMG_COMBINE. This service provides logical combination of bitmap
**	image data.
**
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX RISC/ULTRIX
**
**
**  AUTHOR(S):
**
**      John Weber
**	Revised for V3.0 by John Poltrack
**
**  CREATION DATE:
**
**      11-JUN-1986
**
*****************************************************************************/

#include <img/ChfDef.h>			    /* Condition handler	    */
#include <img/ImgDef.h>			    /* Image definitions	    */
#include <ImgDefP.h>			    /* Image Private definitions    */
#include <ImgMacros.h>			    /* Common macro definitions	    */
#include <ImgVectorTable.h>		    /* Dispatch definitions	    */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  Table of contents
*/
#ifdef VMS
struct FCT	*IMG$COMBINE();	      /* Combine routine VMS bindings       */
struct FCT	*IMG$COMBINE_FRAME(); /* V3 routine VMS bindings            */
#endif
#ifdef NODAS_PROTO
struct FCT      *ImgCombine();	      /* Combine routine portable binding   */
struct FCT      *ImgCombineFrame();   /* V3 routine portable binding        */
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
void		ImgVerifyFrame();

struct FCT	*_ImgCloneFrame();		/* img__frame_utils	*/
void		 _ImgErrorHandler();		/* img_error_handler	*/
struct FCT	*_ImgGet();			/* img__access_utils    */
long		 _ImgGetVerifyStatus();
struct FCT	*_ImgPut();			/* img__access_utils    */
struct UDP	*_ImgSetRoi();			/* img__roi		*/
long		 _ImgValidateRoi();		/* img_roi_utils	*/

/*
** External references form Condition Handling Facility
*/
void	ChfStop();				/* Exception stop	*/
void	ChfSignal();				/* Exception signal	*/
#endif

#include <img/ImgStatusCodes.h>


/*****************************************************************************
**	    VMS  and Ultrix version 2 entry points                          **
*****************************************************************************/

/*
** Version 2.0 VMS
*/
#ifdef VMS
struct FCT *IMG$COMBINE(dstfid, dstroi, srcfid, srcroi, mask, rule)
    struct FCT *dstfid;                     /* Destination frame ID         */
    struct ROI *dstroi;                     /* Destination frame region     */
    struct FCT *srcfid;                     /* Source frame ID		    */
    struct ROI *srcroi;                     /* Source frame region	    */
    char *mask;                             /* region source mask pattern   */
    long rule;				    /* Combination rule (logical)   */ 
{

return (ImgCombine(dstfid, dstroi, srcfid, srcroi, mask, rule));
} /* end of IMG$COMBINE */
#endif


/****************************************************************************
**  ImgCombine - Combine image data using logical rules
**
**  FUNCTIONAL DESCRIPTION:
**
**      This service combines image data from a source frame with data
**	in the destination frame and a specified mask pattern into the
**	destination frame. 
**
**  FORMAL PARAMETERS:
**
**      Return an image frame to the user which has been COMBINED from
**	a supplied image frame. This module (level 1) performs the following:
**
**		1. parse input parameters
**		2. validate / conformance checking of the srcfid, dstfid
**		3. validate / conformance checking of the srcroi, dstroi
**		4. udp and parameter assembly for layer 2 scale routines
**		5. dispatch to layer 2 combine routines
**		6. handling of layer 2 combine routine returns
**
**  FORMAL PARAMETERS:
**
**	dstfid	- frame which is primary source/target of combination rule
**	dstroi	- region of interest identifier in dstfid
**	srcfid	- frame which is secondary source of combination (unchanged)
**	srcroi	- region of interest in srcfid
**	mask	- byte square pattern that is combined with dstfid
**	rule	- logical combination rule (clear, set, and, or, xor etc.)
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
struct FCT *ImgCombine(dstfid, dstroi, srcfid, srcroi, mask, rule)
    struct FCT *dstfid;                     /* Destination frame ID         */
    struct ROI *dstroi;                     /* Destination frame region     */
    struct FCT *srcfid;                     /* Source frame ID		    */
    struct ROI *srcroi;                     /* Source frame region	    */
    char *mask;                             /* region source mask pattern   */
    long rule;				    /* Combination rule (logical)   */ 

{
struct	UDP dst_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Src/Dst UDP descriptor */
struct	UDP src_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Src     UDP descriptor */

long	dst_bpc[MAX_NUMBER_OF_COMPONENTS]; /* bits per component	    */
long	dst_cmprss_type;		    /* compression type		    */
long	dst_cs_org;			    /* comp space organization	    */
long	dst_num_of_comp;		    /* number of components	    */
long	dst_plane_bits_per_pixel[MAX_NUMBER_OF_COMPONENTS];
long	dst_planes_per_pixel;		    /* number of planes		    */
long	dst_spect_map;			    /* spectral mapping		    */
long	dst_spectral_type;		    /* spectral type		    */
long	src_bpc[MAX_NUMBER_OF_COMPONENTS];  /* bits per component	    */
long	src_cmprss_type;		    /* compression type		    */
long	src_cs_org;			    /* comp space organization	    */
long	src_num_of_comp;		    /* number of components	    */
long	src_plane_bits_per_pixel[MAX_NUMBER_OF_COMPONENTS];
long	src_planes_per_pixel;		    /* number of planes		    */
long	src_spect_map;			    /* spectral mapping		    */
long	src_spectral_type;		    /* spectral type		    */

long	idx;				    /* scratch index variable	    */
long	status;				    /* status of layer 2 routine    */


/*****************************************************************************
**	    start of validation & conformance checking                      **
*****************************************************************************/

/*
** first verify dstfid(required)
*/
if ( !VERIFY_OFF_ )
    ImgVerifyFrame( dstfid, ImgM_NonstandardVerify );

_ImgGet(dstfid,Img_ImageDataClass, &dst_spectral_type,
	sizeof(dst_spectral_type),0,0);

_ImgGet(dstfid,Img_CompSpaceOrg,   &dst_cs_org,
	sizeof(dst_cs_org),0,0);

_ImgGet(dstfid,Img_CompressionType,&dst_cmprss_type,
	sizeof(dst_cmprss_type),0,0);

_ImgGet(dstfid,Img_NumberOfComp,   &dst_num_of_comp,
	sizeof(dst_num_of_comp),0,0);

_ImgGet(dstfid,Img_PlanesPerPixel,   &dst_planes_per_pixel,
	sizeof(dst_planes_per_pixel),0,0);

for ( idx = 0; idx < dst_planes_per_pixel; ++idx )
    _ImgGet( dstfid, Img_PlaneBitsPerPixel, dst_plane_bits_per_pixel,
		LONGSIZE, 0, idx );

_ImgGet(dstfid,Img_SpectralMapping,&dst_spect_map,
	sizeof(dst_spect_map),0,0); 

for (idx=0;idx<dst_num_of_comp;idx++) 
    _ImgGet(dstfid,Img_BitsPerComp,&dst_bpc[idx],sizeof(dst_bpc[idx]),0,idx); 

/*
** Validate that dstfid is Uncompressed
*/
if (dst_cmprss_type != ImgK_PcmCompression)
    ChfStop(1,ImgX_INVCMPTYP);	

/*
** Validate Component Organization, Number of Components, Bits per Component
*/
switch (dst_spectral_type)
    {
    case ImgK_ClassBitonal:
       {
       if (dst_num_of_comp != 1)
          ChfStop(1,ImgX_PARAMCONF);
       if (dst_bpc[0] != 1)
          ChfStop(1,ImgX_PARAMCONF);
       }; 
       break;
    case ImgK_ClassGreyscale:
       {
       if ((dst_cs_org != ImgK_BandIntrlvdByPixel) &&
           (dst_cs_org != ImgK_BandIntrlvdByPlane))
            ChfStop(1,ImgX_UNSOPTION);
       if (dst_num_of_comp != 1)
           ChfStop(1,ImgX_PARAMCONF);
       /*
       ** Allow only 8 bit multiples
       */
       if (dst_bpc[0]%8 != 0)
           ChfStop(1,ImgX_UNSOPTION);
       };
       break;
    case ImgK_ClassMultispect:
       {
       if ((dst_cs_org != ImgK_BandIntrlvdByPixel) &&
           (dst_cs_org != ImgK_BandIntrlvdByPlane))
            ChfStop(1,ImgX_UNSOPTION);
       if (dst_num_of_comp != 3)
           ChfStop(1,ImgX_PARAMCONF);
       if (dst_spect_map != ImgK_RGBMap)
           ChfStop(1,ImgX_UNSOPTION);

	/*
	** Disallow any non-eight-bit per plane data
	*/
	for ( idx=0; idx < dst_planes_per_pixel ; idx++ ) 
            if ( dst_plane_bits_per_pixel[idx]%8 != 0)
                ChfStop(1,ImgX_UNSOPTION);
       }; 
       break;
    default:
      ChfStop(1,ImgX_UNSOPTION);
      break;
    };/* end switch */

/*
** If srcfid is present?
*/
if (srcfid != 0)
    {
    if ( !VERIFY_OFF_ )
	ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

    _ImgGet(srcfid,Img_ImageDataClass, &src_spectral_type,
	    sizeof(src_spectral_type),0,0);

    _ImgGet(srcfid,Img_CompSpaceOrg,   &src_cs_org,
	    sizeof(src_cs_org),0,0);

    _ImgGet(srcfid,Img_CompressionType,&src_cmprss_type,
	    sizeof(src_cmprss_type),0,0);

    _ImgGet(srcfid,Img_NumberOfComp,   &src_num_of_comp,
	    sizeof(src_num_of_comp),0,0);

    _ImgGet(dstfid,Img_PlanesPerPixel,   &src_planes_per_pixel,
	    sizeof(src_planes_per_pixel),0,0);

    for ( idx = 0; idx < src_planes_per_pixel; ++idx )
	_ImgGet( srcfid, Img_PlaneBitsPerPixel, src_plane_bits_per_pixel,
		    LONGSIZE, 0, idx );

    _ImgGet(srcfid,Img_SpectralMapping,&src_spect_map,
	    sizeof(src_spect_map),0,0); 

    for (idx=0;idx<src_num_of_comp;idx++) 
        _ImgGet(srcfid,Img_BitsPerComp,&src_bpc[idx],
		sizeof(src_bpc[idx]),0,idx); 

    /*
    ** Validate that Fid is Uncompressed
    */
    if (src_cmprss_type != ImgK_PcmCompression)
        ChfStop(1,ImgX_INVCMPTYP);	

    /*
    ** Validate Component Organization, Number of Components, Bits per Component
    */
    switch (src_spectral_type)
        {
        case ImgK_ClassBitonal:
           {
           if (src_num_of_comp != 1 || dst_num_of_comp != 1)
              ChfStop(1,ImgX_PARAMCONF);
           if (src_bpc[0] != 1 || dst_bpc[0] != 1)
              ChfStop(1,ImgX_PARAMCONF);
           }; 
           break;
        case ImgK_ClassGreyscale:
           {
           if (((src_cs_org != ImgK_BandIntrlvdByPixel) &&
               (src_cs_org != ImgK_BandIntrlvdByPlane)) ||
	       ((dst_cs_org != ImgK_BandIntrlvdByPixel) &&
	       (dst_cs_org != ImgK_BandIntrlvdByPlane)))
                ChfStop(1,ImgX_UNSOPTION);
           if (src_num_of_comp != 1 || dst_num_of_comp != 1)
               ChfStop(1,ImgX_PARAMCONF);
           if (src_bpc[0]%8 != 0 || dst_bpc[0]%8 != 0)
               ChfStop(1,ImgX_UNSOPTION);
           };
           break;
        case ImgK_ClassMultispect:
           {
	   if ( src_cs_org == dst_cs_org &&
                src_cs_org != ImgK_BandIntrlvdByPlane &&
                src_cs_org != ImgK_BandIntrlvdByPixel )
               ChfStop(1,ImgX_UNSOPTION);
           if (src_num_of_comp != 3 || dst_num_of_comp != 3)
               ChfStop(1,ImgX_PARAMCONF);
           if (src_spect_map != ImgK_RGBMap || dst_spect_map != ImgK_RGBMap)
               ChfStop(1,ImgX_UNSOPTION);
	    /*
	    ** Disallow any non-eight-bit per plane data
	    */
	    for ( idx=0; idx < src_planes_per_pixel ; idx++ ) 
		if ( src_plane_bits_per_pixel[idx]%8 != 0)
		    ChfStop(1,ImgX_UNSOPTION);

           }; 
           break;
        default:
          ChfStop(1,ImgX_UNSOPTION);
          break;
        };/* end switch */
    }/* end if (srcfid not null) */

for ( idx=0; idx < dst_planes_per_pixel; idx++)
    {
    /*
    ** Validate and set destination udp and roi for each component
    */
    _ImgGet(dstfid,Img_Udp,&dst_udp[idx],sizeof(struct UDP),0,idx);
    if (dstroi != 0)
	{ 
	if (! _ImgValidateRoi(dstroi,&dst_udp[idx]))
	    ChfStop(1,ImgX_INVROI);
	else
	    _ImgSetRoi(&dst_udp[idx],dstroi);
	};

    /*
    ** If srcfid is NULL set up a NULL addressed UDP for layer 2 routines
    */
    if (srcfid == 0)
	{
	src_udp[idx] = dst_udp[idx];
	src_udp[idx].UdpA_Base = 0;	
	}
    else
	{
	/*
        ** Validate and set source udp and roi for each component
        */
        _ImgGet(srcfid,Img_Udp,&src_udp[idx],sizeof(struct UDP),0,idx);
        if (srcroi != 0)
	    if (! _ImgValidateRoi(srcroi,&src_udp[idx]))
                ChfStop(1,ImgX_INVROI);
            else
                _ImgSetRoi(&src_udp[idx],srcroi);
	}/* end srcfid specified */
    }/* end component loop */
/*****************************************************************************
**	    End ROI validation						    **
*****************************************************************************/


/*****************************************************************************
**	    Start Dispatch to layer 2 combine routines			    **
*****************************************************************************/
for ( idx=0; idx < dst_planes_per_pixel; idx++)
    {
    status = (*ImgA_VectorTable[ImgK_Combine])
                  (&src_udp[idx],&dst_udp[idx],mask,rule); 
    if ((status & 1) != 1)
	_ImgErrorHandler(status);
    }/* end component loop */

if ( !VERIFY_OFF_ )
    ImgVerifyFrame( dstfid, ImgM_NonstandardVerify );
return (dstfid);
} /* end of ImgCombine */


/*****************************************************************************
**	    VMS  and Ultrix version 3 entry points                          **
*****************************************************************************/

/*
** Version 3.0 VMS
*/
#ifdef VMS
struct FCT *IMG$COMBINE_FRAME(srcfid1, roi1, srcfid2, roi2, rule, flags)
    struct FCT *srcfid1;                    /* primary source frame ID      */
    struct ROI *roi1;                       /* primary source frame region  */
    struct FCT *srcfid2;                    /* source frame ID		    */
    struct ROI *roi2;                       /* source frame region	    */
    unsigned long rule;			    /* combination rule (logical)   */ 
    unsigned long flags;		    /* processing flags		    */
{
return (ImgCombineFrame(srcfid1, roi1, srcfid2, roi2, rule, flags));
} /* end of IMG$COMBINE_FRAME */
#endif


/****************************************************************************
**  ImgCombineFrame - Combine image data using logical rules
**
**  FUNCTIONAL DESCRIPTION:
**
**      This service combines image data from a source frame with data
**	in another source frame into the destination frame.
**
**  FORMAL PARAMETERS:
**
**      Return an image frame to the user which has been COMBINED from
**	a supplied image frame. This module (level 1) performs the following:
**
**		1. parse input parameters
**		2. validate / conformance checking of the srcfids
**		3. validate / conformance checking of the srcrois
**		4. udp and parameter assembly for layer 2 combine routines
**		5. dispatch to layer 2 combine routines
**		6. handling of layer 2 combine routine returns
**
**  FORMAL PARAMETERS:
**
**	srcfid1	- frame which is primary source/target of combination rule
**	roi1	- region of interest identifier in srcfid1
**	srcfid2	- frame which is secondary source of combination
**	roi2	- region of interest in srcfid
**	rule	- logical combination rule (clear, set, and, or, xor etc.)
**	flags	- processing flags
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
**	ImgX_INVDARG	- invalid flags argument passed
**	ImgX_UNSOPTION  - unsupported spect map or component organization     
**	ImgX_PARAMCONF  - inconsistent frame data    
**
**  SIDE EFFECTS:
**
**	none
**
*****************************************************************************/
struct FCT *ImgCombineFrame(srcfid1, roi1, srcfid2, roi2, rule, flags)
    struct FCT *srcfid1;                    /* primary source frame ID      */
    struct ROI *roi1;                       /* primary source frame region  */
    struct FCT *srcfid2;                    /* source frame ID		    */
    struct ROI *roi2;                       /* source frame region	    */
    unsigned long rule;			    /* combination rule (logical)   */ 
    unsigned long flags;		    /* processing flags		    */
{
struct	FCT *dstfid;			    /* combined destination frame id*/
struct	UDP src1_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Src1    UDP descriptor */
struct	UDP src2_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Src2    UDP descriptor */
struct	UDP  dst_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Destination UDP	    */

long	src1_bpc[MAX_NUMBER_OF_COMPONENTS]; /* bits per component	    */
long	src2_bpc[MAX_NUMBER_OF_COMPONENTS]; /* bits per component	    */
long	src1_cmprss_type;		    /* compression type		    */
long	src2_cmprss_type;		    /* compression type		    */
long	src1_cs_org;			    /* comp space organization	    */
long	src2_cs_org;			    /* comp space organization	    */
long	src1_num_of_comp;		    /* number of components	    */
long	src2_num_of_comp;		    /* number of components	    */
long	src1_num_of_lines;		    /* number of scanlines	    */
long	src2_num_of_lines;		    /* number of scanlines	    */
long	src1_pixels_per_line;		    /* pixels per line		    */
long	src2_pixels_per_line;		    /* pixels per line		    */
long	src1_quant_levels[MAX_NUMBER_OF_COMPONENTS]; /* levels per comp	    */
long	src2_quant_levels[MAX_NUMBER_OF_COMPONENTS]; /* levels per comp	    */
long	src1_spect_map;			    /* spectral mapping		    */
long	src2_spect_map;			    /* spectral mapping		    */
long	src1_spectral_type;		    /* spectral type		    */
long	src2_spectral_type;		    /* spectral type		    */

long	idx;				    /* scratch index variable	    */
long	status;				    /* status of layer 2 routine    */


/*****************************************************************************
**	    start of validation & conformance checking                      **
*****************************************************************************/

/*
** first verify srcfid1(required)
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame(srcfid1,0);

/*
** Make copy of src1 to support in place operations
*/
if (roi1 == 0)
    dstfid = (struct FCT *) ImgAllocateFrame(0,0,srcfid1,0);
else
    dstfid = (struct FCT *) ImgCopyFrame(srcfid1, 0);

_ImgGet(srcfid1,Img_CompressionType,&src1_cmprss_type,
	sizeof(src1_cmprss_type),0,0);

_ImgGet(srcfid1,Img_ImageDataClass, &src1_spectral_type,
	sizeof(src1_spectral_type),0,0);

_ImgGet(srcfid1,Img_NumberOfComp,&src1_num_of_comp,
	sizeof(src1_num_of_comp),0,0);

/*
** Validate that srcfid1 is Uncompressed
*/
if (src1_cmprss_type != ImgK_PcmCompression)
    ChfStop(1,ImgX_INVCMPTYP);	

/*
** Determine quant levels for each component, validate and set udp's and roi's 
** for each component of srcfid1
*/
for (idx=0;idx<src1_num_of_comp;idx++)
    {
    _ImgGet(srcfid1,Img_QuantLevelsPerComp,&src1_quant_levels[idx],
	    sizeof(src1_quant_levels[idx]),0,idx);

    _ImgGet(srcfid1,Img_Udp,&src1_udp[idx],sizeof(struct UDP),0,idx);
    _ImgGet(dstfid,Img_Udp,&dst_udp[idx],sizeof(struct UDP),0,idx);

    if (roi1 != 0)
	{ 
	if (! _ImgValidateRoi(roi1,&src1_udp[idx]))
	    ChfStop(1,ImgX_INVROI);
	else
	    {
	    _ImgSetRoi(&src1_udp[idx],roi1);
	    _ImgSetRoi(&dst_udp[idx],roi1);
	    }
	}
    }/* end component loop */

/*
** Get size of srcfid1 after roi1 has been applied
*/
_ImgGet(srcfid1,Img_NumberOfLines,&src1_num_of_lines,
	sizeof(src1_num_of_lines),0,0);

_ImgGet(srcfid1,Img_PixelsPerLine,&src1_pixels_per_line,
	sizeof(src1_pixels_per_line),0,0);

/*
** Verify srcfid2 if present
*/
if (srcfid2 != 0)
    {
    if ( VERIFY_ON_ )
	ImgVerifyFrame(srcfid2,0);

    _ImgGet(srcfid2,Img_ImageDataClass, &src2_spectral_type,
	    sizeof(src2_spectral_type),0,0);

    _ImgGet(srcfid2,Img_CompressionType,&src2_cmprss_type,
	    sizeof(src2_cmprss_type),0,0);

    _ImgGet(srcfid2,Img_NumberOfComp,&src2_num_of_comp,
	    sizeof(src2_num_of_comp),0,0);

    /*
    ** srcfid2 must match srcfid1 in attributes
    */
    if (src2_spectral_type != src1_spectral_type ||
	src2_cmprss_type != src1_cmprss_type ||
	src2_num_of_comp != src1_num_of_comp)
	    ChfStop(1,ImgX_FRMATTMIS);	    	

    /*
    ** Validate and set udp's and roi's for each component of srcfid2
    */
    for (idx=0;idx<src1_num_of_comp;idx++)
        {
        _ImgGet(srcfid2,Img_QuantLevelsPerComp,&src2_quant_levels[idx],
	    sizeof(src1_quant_levels[idx]),0,idx);

        _ImgGet(srcfid2,Img_Udp,&src2_udp[idx],sizeof(struct UDP),0,idx);

        if (roi2 != 0)
	    { 
	    if (! _ImgValidateRoi(roi2,&src2_udp[idx]))
	        ChfStop(1,ImgX_INVROI);
	    else
	        _ImgSetRoi(&src2_udp[idx],roi2);
	    }

	    /*
	    ** src2_udp must match src1_udp in size after roi(s) 
	    ** have been applied
	    */
	    if (src2_udp[idx].UdpL_ScnCnt != src1_udp[idx].UdpL_ScnCnt ||
	    	src2_udp[idx].UdpL_PxlPerScn != src1_udp[idx].UdpL_PxlPerScn)
	    	    ChfStop(1,ImgX_FRMSIZMIS);

	    /*
	    ** src2 quantization levels must match src1 quantiztion levels
	    */
	    if (src2_quant_levels[idx] != src1_quant_levels[idx])
		ChfStop(3,ImgX_FRMLEVMIS,1,idx);

	}/* end component loop */
    }/* end srcfid2 processing */
else
    {
    /*
    ** set up NULL addressed UDP for layer 2 as a courtesy
    */
    for	(idx =0;idx<src1_num_of_comp;idx++)
	 {
	 src2_udp[idx] = src1_udp[idx];
	 src2_udp[idx].UdpA_Base = 0;
	 }	
    }/* end no srcfid2 specified */
/*****************************************************************************
**	    End ROI validation						    **
*****************************************************************************/

/*****************************************************************************
**	    Start Dispatch to layer 2 combine routines			    **
*****************************************************************************/
for (idx=0;idx<src1_num_of_comp;idx++)
    {
    unsigned long operator;

    switch (src1_spectral_type)
	{
	case ImgK_ClassBitonal:
	    /*
	    ** Fix for QAR 92:  pass in an IPS rule and TWO valid images.
	    */
	    switch (rule)
		{
		case ImgK_NotSrc1:
		    status = (*ImgA_VectorTable[ImgK_LogicalBitonal])
	        		(&src1_udp[idx],
				&src1_udp[idx],	/* IPS needs TWO valid images */
				&dst_udp[idx],
				0,
				IpsK_SetToNotSrc2); 
		    break;

		default:
		    status = (*ImgA_VectorTable[ImgK_LogicalBitonal])
	        		(&src1_udp[idx],
				&src2_udp[idx],
				&dst_udp[idx],
				0,
				rule); 
		break;
		}/* end switch */
	    if ((status & 1) != 1)
		_ImgErrorHandler(status);
	    break;
	case ImgK_ClassGreyscale:
	case ImgK_ClassMultispect:
	    switch (rule)
		{
		case ImgK_NotSrc1:
		    status = (*ImgA_VectorTable[ImgK_Logical])
		        	(&src1_udp[idx],
			    	&src1_udp[idx],	/* IPS needs TWO valid images */
			    	&dst_udp[idx],
		    		0,
				IpsK_SetToNotSrc2); 
		    break;

		default:
		    status = (*ImgA_VectorTable[ImgK_Logical])
		                 (&src1_udp[idx],
				 &src2_udp[idx],
				 &dst_udp[idx],
				 0,
				 rule); 
		break;
		}/* end switch */
	    if ((status & 1) != 1)
		_ImgErrorHandler(status);
	    break;
	default:
	    ChfStop(1,ImgX_UNSOPTION);
	}/* end switch spectral type */
    }/* end component loop */

/*****************************************************************************
**	    End dispatch to layer 2					    **
*****************************************************************************/
if ( VERIFY_ON_ )
    ImgVerifyFrame(dstfid,0);

return (dstfid);
} /* end of ImgCombineFrame */
