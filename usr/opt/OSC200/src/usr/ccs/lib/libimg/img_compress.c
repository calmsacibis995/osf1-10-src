
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

/******************************************************************************
**
**  ImgCompress
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains the interface from the user level to the 
**	routines that compresses images.
**
**  ENVIRONMENT:
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHORS:
**      Michael D. O'Connor
**	Revised for V3.0 by Karen Rodwell
**
**  CREATION DATE:     October 16, 1989
**
*****************************************************************************/

/*
**  INCLUDE FILES
**/
#include <img/ChfDef.h>                 /* Condition Handling Facility  */
#include <img/ImgDef.h>                 /* ISL Image Definitions        */
#include <ImgDefP.h>		    /* ISL Private Definitions	    */
#include <ImgMacros.h>              /* ISL Macro Definitions        */
#include <ImgVectorTable.h>         /* IPS Layer II Functions       */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  Table of Contents
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$COMPRESS();
struct FCT *IMG$COMPRESS_FRAME();
#endif
#ifdef NODAS_PROTO
struct FCT *ImgCompress();
struct FCT *ImgCompressFrame();
#endif



/*
** External status code symbols
*/
#include <img/ImgStatusCodes.h>

/*
**  External References from ISL            <- from module ->
*/
#ifdef NODAS_PROTO
long	  ImgAllocateFrame();
long	  ImgCvtCompSpaceOrg();		    /* IMG_CONVERT_UTILS	    */
void	  ImgDeallocateFrame();		    /* IMG_FRAME_UTILS		    */
long	  ImgStandardizeFrame();	    /* IMG_FRAME_UTILS		    */
void	  ImgVerifyFrame();		    /* IMG_FRAME_UTILS		    */

char	*_ImgCalloc();
void	 _ImgCfree();
void	 _ImgErrorHandler();		    /* IMG_ERROR_HANDLER	    */
void	 _ImgGet();			    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
long	 _ImgGetVerifyStatus();		    /* IMG__VERIFY_UTILS	    */
void	 _ImgPut();			    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
long	 _ImgVerifyNativeFormat();	    /* IMG__VERIFY_UTILS	    */
#endif

/* 
** Param block descriptor declared here to avoid inclusion of vms 
** style descriptors and support V2 interface
*/
struct  param_descriptor
{
        unsigned short  ParamW_Length;   /* specific to descriptor class;  
        unsigned char   ParamB_DType;    /* data type code */
        unsigned char   ParamB_Class;    /* descriptor class code */
        char            *ParamA_Pointer; /* address of first byte of data element */
};

/*
**  Equated symbols
*/
#define DEFAULT_K_FACTOR         4	/* used by bitonal    G32D scheme   */
#define DEFAULT_DCT_COMP_FACTOR 50	/* used by continuous DCT  scheme   */


/*****************************************************************************
**                  BEGINNING OF V2.0 ENTRY POINTS                          **
*****************************************************************************/
#if defined(__VMS) || defined(VMS)                                                  
struct FCT *IMG$COMPRESS( fid, scheme, param_blk )
struct  FCT             *fid;           /* source frame id 	*/
int			 scheme;        /* compression scheme   */
struct param_descriptor *param_blk;     /* parameter block      */
{                                                 

return (ImgCompress( fid, scheme, param_blk ));
} /* end of IMG$COMPRESS */
#endif                                                      

struct FCT *ImgCompress( src_fid, scheme, param_blk_ptr )
struct FCT              *src_fid;       /* source frame id 	*/
int                      scheme;        /* compression scheme   */
char			*param_blk_ptr; /* parameter block      */
{                                                 
struct param_descriptor *param_blk;     /* parameter block      */
long		    flags = ImgM_InPlace;
long		    ret_fid;
long		    std_cs_org = ImgK_BandIntrlvdByPlane;
long		    status;
struct	 ITMLST	    *itmlst;

/*
** Cast into a struct param_descriptor *
*/
param_blk = (struct param_descriptor *)param_blk_ptr;

/*
** Verify the source frame ...
*/
if ( !VERIFY_OFF_ )
    ImgVerifyFrame( src_fid, ImgM_NonstandardVerify );

/*
** Call V3.0 entry point after setting up the itemlist ...
*/
if (param_blk != 0)
    {
    itmlst = (struct ITMLST *)_ImgCalloc(2, sizeof(struct ITMLST));
    itmlst[0].ItmL_Code   = Img_G32dKfactor;
    itmlst[0].ItmL_Length = param_blk->ParamW_Length;
    itmlst[0].ItmA_Buffer = param_blk->ParamA_Pointer;
    itmlst[0].ItmA_Retlen = 0;
    itmlst[0].ItmL_Index  = 0;
    }/* end conversion from descriptor to item list */
else
    itmlst = 0;

ImgCompressFrame( src_fid, scheme, flags, itmlst);

if (param_blk != 0)
    _ImgCfree(itmlst);

/*
** Always return src fid no matter what ... it doesn't matter
** what was passed back from ImgCompressFrame.
*/
return (struct FCT *) src_fid;
} /* end of ImgCompress */

/****************************************************************************
**                       END OF V2.0 ENTRY POINTS                          **
****************************************************************************/

/****************************************************************************
**                       BEGINNING OF CURRENT ENTRY POINTS                 **
****************************************************************************/
#if defined(__VMS) || defined(VMS)                                                  
struct FCT *IMG$COMPRESS_FRAME( src_fid, scheme, flags, comp_params )
struct FCT		*src_fid;		    /* source frame id      */
int			scheme;			    /* compression scheme   */
unsigned long		flags;			    /* process flags	    */
struct ITMLST		*comp_params;		    /* parameter block      */
{                                                 

return (ImgCompressFrame( src_fid, scheme, flags, comp_params ));
} /* end of IMG$COMPRESS_FRAME */
#endif                                                      


/*****************************************************************************
**  ImgCompressFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**      Image data attached to the input frame is encoded using the data
**	compression scheme passed in. The original uncompressed image data 
**	is discarded.
**
**	Specifically, this routine performs the following:
**
**      1. parse input parameters
**      2. validate / conformance checking of the src_fid
**      3. parameter assembly for layer 2 compress routines
**      4. dispatch to layer 2 compress routines
**      5. handling of layer 2 compress routine returns
**
**  FORMAL PARAMETERS:
**
**      SRC_FID		    Frame ID (pointer to a FCT)
**
**	SCHEME		    Scheme to use to encode image.  
**			    Default is group 4-2D (for bitonal images).
**			    This parameter is optional.
**
**	COMP_PARAMS	    Parameter block passed into lower-level
**			    compression routines.  This parameter is
**			    optional.
**
**			    For bitonal images, this argument is valid
**			    only for the G32D scheme.  It will be ignored
**			    by all other bitonal schemes.
**
**  IMPLICIT INPUTS: None.
**  IMPLICIT OUTPUTS: None.
**  SIDE EFFECTS: None.
**
**	Error codes signaled:
**	    ImgX_INVARGCNT	- Invalid argument count (VMS only)
**	    ImgX_UNSCMPTYP	- unsupported compression type
**
**
*****************************************************************************/
struct FCT *ImgCompressFrame( src_fid, scheme, flags, comp_params )
struct FCT		*src_fid;		    /* source frame id      */
int     		scheme;			    /* compression scheme   */
unsigned long		flags;			    /* process flags	    */
struct ITMLST		*comp_params;		    /* parameter block      */
{
unsigned long compressed_data_size;    /* returned compression size in bits */
/*
** ??NOTE: under the current model of the frame, DCT compression is implemented
**         as if multispectral frames were multiple greyscale frames.
*/
unsigned long plane_count = 1;

long	 compression_factor = 0;	     /* level of DCT compression     */
long	 compression_type;		     /* type of compression scheme   */
long     data_class = 0;                     /* data class                   */
long	 dct_comp_factor = 0;		     /* compression parameter dct    */
long	 do_not_continue = 0;
long     kfactor = 0;			     /* compression parameter g32d   */
long     number_of_comp = 0;                 /* number of spectral component */
char	*param_blk_adr;
long     param_blk_len;
struct FCT *ret_fid;			     /* returned frame id	     */
long     status,index = 0;                   /* scratch index counter        */
long	 udp_present;			     
struct   UDP src_udp[MAX_NUMBER_OF_COMPONENTS];/* src UDP descriptors        */
struct   UDP dst_udp[MAX_NUMBER_OF_COMPONENTS];/* dst UDP descriptors        */
struct	 UDP *udp_ptr_list[MAX_NUMBER_OF_COMPONENTS];/* source UDP pointer(s)*/
struct	 UDP *udp_ptr;				     /* layer 2 UDP pointer  */

/*
** Extract the compression parameter from the comp_params itemlist
*/
if ( comp_params != 0 && comp_params[0].ItmL_Code != 0 )
    {
    switch( comp_params[0].ItmL_Code )
	{
	case Img_G32dKfactor:
	    kfactor = *((long*)comp_params[0].ItmA_Buffer);
	    break;
	case Img_DctCompFactor:	/* IMG$_DCT_COMP_FACTOR in IMG$DEF.SDL */
            dct_comp_factor = *((long*)comp_params[0].ItmA_Buffer);
	    break;
	default:
	    break;
	} /* end switch */
    }/* end comp_params present */
else
    {
    /*
    ** set up default compression constants
    */
    kfactor = DEFAULT_K_FACTOR;
    dct_comp_factor = DEFAULT_DCT_COMP_FACTOR;
    }/* end default values for g32d and dct compression */

/*****************************************************************
**      start of validation and conformance checking            **
*****************************************************************/
_ImgGet (src_fid, Img_NumberOfComp,  &number_of_comp,sizeof(long),0,0);
_ImgGet (src_fid, Img_ImageDataClass, &data_class, sizeof(long), 0, 0);
_ImgGet (src_fid, Img_CompressionType,&compression_type,sizeof(long),0,0);
_ImgGet (src_fid, Img_UdpPrsnt,&udp_present,sizeof(long),0,0);
for (index=0;index<number_of_comp;index++)
    {
    _ImgGet(src_fid,Img_Udp,&src_udp[index],sizeof(struct UDP), 0, index);
    }
/*
** Validate Component Organization and Number of Components
*/
if (data_class == ImgK_ClassBitonal)
    {
    if ( VERIFY_ON_ )
	ImgVerifyFrame(src_fid, ImgM_NonstandardVerify);
    if (scheme == 0)
        scheme = ImgK_G42dCompression;
    }
else
    {
    if ( VERIFY_ON_ )
	ImgVerifyFrame(src_fid, 0);
    if (scheme == 0)
        scheme = ImgK_DctCompression;
    }
 
/*************************************************************
**            End of validation and conformance checking    **
*************************************************************/
/*************************************************************
**    Start dispatch to layer 2  compression routines       **
*************************************************************/

/*
** Initialize dst_udp and load src udp list
*/
for (index = 0; index < number_of_comp; index++)
    {
    dst_udp[index].UdpA_Base = 0;
    udp_ptr_list[index] = &src_udp[index];
    }

/*
**  Compression type.  NOTE that nothing will happen if the frame is 
**  already compressed.
*/
if ( udp_present == FALSE )
    ChfStop( 1,  ImgX_NOUNCDATA );	    /* no uncompressed data	*/

/*
** Compress the data.
*/
for (index = 0; index < number_of_comp; index++)
    switch(scheme)
	{
	/*
	** Bitonal compression schemes.
	*/
	case ImgK_G31dCompression:
            status = (*ImgA_VectorTable[ImgK_EncodeG31d])
                    (&src_udp[index],&dst_udp[index], flags, 
			&compressed_data_size);
            if ((status & 1) != 1)
		{
		do_not_continue = 1;
                _ImgErrorHandler(status);
		}
	    break;
	case ImgK_G32dCompression:
            status = (*ImgA_VectorTable[ImgK_EncodeG32d])
                    (&src_udp[index],&dst_udp[index],kfactor,
			flags, &compressed_data_size);
            if ((status & 1) != 1)
		{
		do_not_continue = 1;
                _ImgErrorHandler(status);
		}
	    break;
	case ImgK_G42dCompression:
            status = (*ImgA_VectorTable[ImgK_EncodeG42d])
                    (&src_udp[index],&dst_udp[index], &compressed_data_size);
            if ((status & 1) != 1)
		{
		do_not_continue = 1;
                _ImgErrorHandler(status);
		}
	    break;
	case ImgK_DctCompression:
            status = (*ImgA_VectorTable[ImgK_EncodeDct])
                    (&udp_ptr_list[index],plane_count,&dst_udp[index],
		     dct_comp_factor,1,&compressed_data_size);
            if ((status & 1) != 1)
		{
		do_not_continue = 1;
                _ImgErrorHandler(status);
		}
	    break;
	/*
	** Unrecognized compression scheme.
	*/
	default:
 	    /*
	    **	Signal that requested compression type is unsupported
	    */
	    ChfStop( 1,  ImgX_UNSCMPTYP );
	    break;
	} /* end switch */
/*************************************************************
**    End dispatch to layer 2 compression routines          **
*************************************************************/
/*************************************************************
**              begin post processing                       **
*************************************************************/

/*
** If there was some problem that was not trapped properly by
** a condition handler, do not continue beyond this point.
*/
if ( do_not_continue )
    {
    ret_fid = 0;
    return (struct FCT *) ret_fid;
    }

if ( (flags & ImgM_InPlace) )
    ret_fid = src_fid;
else
    {
    ret_fid = ImgAllocateFrame( 0, 0, src_fid, ImgM_NoDataPlaneAlloc );
    }

for (index = 0; index < number_of_comp; index++)
    {
    _ImgPut(ret_fid,Img_CompressionType,&scheme,sizeof(long),index );
    _ImgPut(ret_fid,Img_Udp,&dst_udp[index],sizeof(struct UDP),index);
    }   

if ( (flags & ImgM_AutoDeallocate) && ret_fid != src_fid )
    ImgDeallocateFrame( src_fid );

if ( VERIFY_ON_ )
    ImgVerifyFrame( ret_fid, ImgM_NonstandardVerify );

return (struct FCT *) ret_fid;
/*************************************************************
**              end of post processing                      **
*************************************************************/
/**************************************************************
**              end of current entry points                  **
**************************************************************/
} /* end of ImgCompressFrame*/
