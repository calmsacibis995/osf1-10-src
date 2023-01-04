
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
**
**  ImgDecompress
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains the interface from the user level to the 
**	routines that decompresses images.
**
**  ENVIRONMENT:
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHORS:
**
**      Michael D. O'Connor
**      Revised for V3.0 by Karen Rodwell
**
**  CREATION DATE:     20-OCT-1989
**
*************************************************************************/

/*
**
**  INCLUDE FILES
**
**/
#include <img/ChfDef.h>
#include <img/ImgDef.h>
#include <ImgMacros.h>              /* ISL Macro Definitions        */
#include <ImgVectorTable.h>         /* ISL Layer II Functions       */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  Table of Contents
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$DECOMPRESS();
struct FCT *IMG$DECOMPRESS_FRAME();
#endif
#ifdef NODAS_PROTO
struct FCT *ImgDecompress();
struct FCT *ImgDecompressFrame();
#endif

/*
**	Module local entry points
*/
#ifdef NODAS_PROTO
static long No_compressed_data();
#else
PROTO(static long No_compressed_data, (struct FCT * /*fid*/));
#endif


/*
**  Equated symbols.
*/

/*
**  Temporary flag symbol for use between V2 and V3 entry point
*/
#define	ImgM_NoBitonalAlign	(1 << 30)


/*
**	Status codes
*/
#include <img/ImgStatusCodes.h>         /* ISL Status Codes             */

/*
**  External References from ISL                 <- from module ->
*/
#ifdef NODAS_PROTO
long	     ImgAllocateFrame();
void	     ImgDeallocateFrame();
char	    *ImgDetachDataPlane();
void	     ImgFreeDataPlane();
void	     ImgVerifyFrame();

void	    _ImgGet();
long	    _ImgGetVerifyStatus();
void	    _ImgErrorHandler();		/* img_error_handler    */
void	    _ImgPut();
#endif


/*****************************************************************************
**                  BEGINNING OF V2.0 ENTRY POINTS                          **
*****************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$DECOMPRESS( fid )
struct FCT	*fid;
{
unsigned long	flags;

flags = ImgM_InPlace | ImgM_NoBitonalAlign;
return (ImgDecompressFrame( fid, flags, 0 ));
} /* end of IMG$DECOMPRESS */
#endif

struct FCT *ImgDecompress( fid )
struct FCT	*fid;
{
unsigned long	flags;
flags = ImgM_InPlace | ImgM_NoBitonalAlign;
return (ImgDecompressFrame( fid, flags, 0 ));
} /* end of ImgDecompress */


/****************************************************************************
**                       BEGINNING OF CURRENT ENTRY POINTS                 **
****************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$DECOMPRESS_FRAME( fid, flags, comp_params )
struct FCT	*fid;
unsigned long    flags;
struct ITMLST   *comp_params;
{

return (ImgDecompressFrame( fid, flags, comp_params));
} /* end of IMG$DECOMPRESS_FRAME */
#endif


/****************************************************************************
**  ImgDecompressFrame - Decompress an ISL image
**
**  FUNCTIONAL DESCRIPTION:
**
**      Image data attached to the input frame is decoded using the data
**	compression scheme specified in the frame.  The original compressed 
**	image data is discarded.
**
**      1. parse input parameters
**      2. validate / conformance checking of the fid
**      3. parameter assembly for layer 2 decompress routines
**      4. dispatch to layer 2 decompress routines
**      5. handling of layer 2 decompress routine returns
**
**
**  FORMAL PARAMETERS:
**
**      fid	    - Frame ID (pointer to a FCT)
**	flags	    - Processing flags
**	comp_params - None presently
**
**  IMPLICIT INPUTS: None.
**  IMPLICIT OUTPUTS: None.
**  SIDE EFFECTS: None.
**  ERROR CODES SIGNALED:
**	ImgX_UNSCMPTYP	- unsupported compression type
**
**
*******************************************************************************/
struct FCT *ImgDecompressFrame( src_fid, flags, comp_params )
    struct FCT	    *src_fid;
    unsigned long   flags;
    struct ITMLST   *comp_params;
    {
    char    *data_plane[MAX_NUMBER_OF_COMPONENTS];
    char    *bufptr;
    long     buflen;
    long     compression_type;
    long     data_offset;
    long    *dataptr;
    long     number_of_lines;
    long     pixels_per_line;
    long     ret_buflen;
    long     scanline_stride;
    long     bits_per_pixel;
    long     spectral_mapping = 0;
    long     number_of_comp = 0;
    long     number_of_planes;
    long     comp_org = 0;
    long     spec_type = 0;
    long     status,index = 0;                   /* scratch index counter     */
    long     bits_perc[MAX_NUMBER_OF_COMPONENTS];/* bits per component        */
    struct   FCT *dst_fid;			   /* destination fid ptr     */
    struct   UDP src_udp[MAX_NUMBER_OF_COMPONENTS];/* src UDP descriptors     */
    struct   UDP dst_udp[MAX_NUMBER_OF_COMPONENTS];/* dst UDP descriptors     */
    struct   UDP *udp_ptr_list[MAX_NUMBER_OF_COMPONENTS];/* dst UDP pointer(s)*/

    unsigned long   bitonal_flags   = 0;

/*****************************************************************
**      start of validation and conformance checking            **
*****************************************************************/
if ( VERIFY_ON_ )
    ImgVerifyFrame(src_fid,ImgM_NonstandardVerify);

/*
** If the source frame contains no uncompressed data, go no further
*/
if ( No_compressed_data( src_fid ) )
    {
    if ( (flags & ImgM_InPlace) )
	dst_fid = src_fid;
    else
	{
	dst_fid = (struct FCT *) ImgCopyFrame( src_fid, 0 );
	if ( (flags & ImgM_AutoDeallocate) )
	    ImgDeallocateFrame( src_fid );
	}
    return dst_fid;
    }

/*
** The source frame contains at least one plane of compressed data ...
*/
_ImgGet (src_fid, Img_CompSpaceOrg, &comp_org, sizeof(long), 0, 0);
_ImgGet (src_fid, Img_DataOffset,&data_offset,sizeof(long),0,0);
_ImgGet (src_fid, Img_ImageDataClass, &spec_type, sizeof(long), 0, 0);
_ImgGet (src_fid, Img_NumberOfComp,  &number_of_comp,sizeof(long),0,0);
_ImgGet (src_fid, Img_NumberOfLines,&number_of_lines,sizeof(long),0,0);
_ImgGet (src_fid, Img_PixelsPerLine,&pixels_per_line,sizeof(long),0,0);
_ImgGet (src_fid, Img_PlanesPerPixel,&number_of_planes,sizeof(long),0,0);
_ImgGet (src_fid, Img_ScanlineStride,&scanline_stride,sizeof(long),0,0);
_ImgGet (src_fid, Img_SpectralMapping,&spectral_mapping, sizeof(long),0,0);

for (index=0;index<number_of_planes;index++)
    {
    _ImgGet(src_fid, Img_BitsPerComp,&bits_perc[index],sizeof(long),0,index);
    _ImgGet(src_fid,Img_Udp,&src_udp[index],sizeof(struct UDP), 0, index);
    if (bits_perc[index] > 25)
        ChfStop( 1, ImgX_PXLTOOLRG);
    }
/*
** Validate Component Organization and Number of Components
*/
switch (spec_type)
    {
    case ImgK_ClassBitonal:
        break;
     case ImgK_ClassGreyscale:
        {
        if  ((comp_org != ImgK_BandIntrlvdByPixel) &&
             (comp_org != ImgK_BandIntrlvdByPlane))
            ChfStop( 1, ImgX_UNSOPTION);
        if (bits_perc[0] != 8)
            ChfStop(1,ImgX_UNSOPTION);
        }; break;
     case ImgK_ClassMultispect:
        {
        if  (comp_org != ImgK_BandIntrlvdByPlane)
            ChfStop( 1, ImgX_UNSOPTION);
        if (number_of_comp > MAX_NUMBER_OF_COMPONENTS)
            ChfStop( 1, ImgX_PARAMCONF);
        if (bits_perc[0] != 8)
            ChfStop(1,ImgX_UNSOPTION);
        }; break;
    default: ChfStop (1, ImgX_UNSOPTION); break;
    };
  
/*************************************************************
**            End of validation and conformance checking    **
*************************************************************/
/*************************************************************
**    Start dispatch to layer 2  decompression routines     **
*************************************************************/

/*
** Initialize dst_udp and load src udp list
*/
for (index = 0; index < number_of_planes; index++)
    {
    dst_udp[index].UdpA_Base = 0;
    udp_ptr_list[index] = &dst_udp[index];

    /*
    **	Process depending on data encoding scheme specified in the frame.
    **   If already decompresses, skip the following and return.....
    */
    _ImgGet (src_fid, Img_CompressionType,&compression_type,sizeof(long),0,index);

    if ( (flags & ImgM_NoBitonalAlign) )
	bitonal_flags = IpsK_DTypeIntBits;
    if (compression_type != ImgK_PcmCompression)
        {
        switch(compression_type)
            {
            case ImgK_G31dCompression:
                status = (*ImgA_VectorTable[ImgK_DecodeG31d])
                    (&src_udp[index],&dst_udp[index], bitonal_flags);
                if ((status & 1) != 1)
                    _ImgErrorHandler(status);
                break;
            case ImgK_G32dCompression:
                status = (*ImgA_VectorTable[ImgK_DecodeG32d])
                    (&src_udp[index],&dst_udp[index], bitonal_flags);
                if ((status & 1) != 1)
                    _ImgErrorHandler(status);
                break;
            case ImgK_G42dCompression:
                status = (*ImgA_VectorTable[ImgK_DecodeG42d])
                    (&src_udp[index],&dst_udp[index], bitonal_flags);
                if ((status & 1) != 1)
                _ImgErrorHandler(status);
                break;
            case ImgK_DctCompression:
                status = (*ImgA_VectorTable[ImgK_DecodeDct])
                    (&src_udp[index],&udp_ptr_list[index],1);
                if ((status & 1) != 1)
                _ImgErrorHandler(status);
                break;
            default: /* unknown or unsupported type requested     */
                ChfStop( 1,  ImgX_UNSCMPTYP );
                break;
            }/* end switch */
	}/* end if not decompressed data */
    }/* end plane count loop */
/*************************************************************
**    End dispatch to layer 2 compression routines          **
*************************************************************/
/*************************************************************
**              begin post processing                       **
*************************************************************/
compression_type = ImgK_PcmCompression;
data_offset = 0;

/*
** Allocate destination frame if new frame is requested else use the source
** frame after stripping off the data planes
*/
if ((flags & ImgM_InPlace) != 0)
    {
    for (index = 0; index < number_of_planes; index++)
	{
	data_plane[index] = (char *) ImgDetachDataPlane(src_fid,index);
	ImgFreeDataPlane(data_plane[index]);
	}/* end component loop */
    dst_fid = src_fid;
    }/* end in place operation */
else
    {
    dst_fid = (struct FCT *) ImgAllocateFrame(0, 0, src_fid, ImgM_NoDataPlaneAlloc );
    /*
    ** Check for auto deallocation of source frame
    */
    if ((flags & ImgM_AutoDeallocate) != 0)
	ImgDeallocateFrame( src_fid);
    }/* end no-inplace operation */

for (index = 0; index < number_of_planes; index++)
    {
    /*
    ** Store the uncompressed data in the frame.  Note that this operation
    ** is being performed on a frame devoid of data
    */
    _ImgPut(dst_fid,Img_DataOffset,&data_offset,sizeof(int),index);
    _ImgPut(dst_fid,Img_CompressionType,&compression_type,sizeof(int),index);
    _ImgPut(dst_fid,Img_Udp,&dst_udp[index],sizeof(struct UDP),index);
    }/* end loop to form uncompressed frame */

if ( VERIFY_ON_ )
    ImgVerifyFrame( dst_fid, ImgM_NonstandardVerify );

return(dst_fid);
}/* end of ImgDecompressFrame */


/******************************************************************************
** No_compressed_data()
**
**  This function determines whether any of a frame's data planes
**  are compressed.
**
**  It returns:
**
**	1/TRUE	    if no compressed data is present, and
**	0/FALSE	    if compressed data is present.
******************************************************************************/
static long No_compressed_data( fid )
struct FCT *fid;
{
long	comp_type;
long	index;
long	plane_cnt;
long	status	    = TRUE;

_ImgGet( fid, Img_PlanesPerPixel, &plane_cnt, sizeof(long), 0, 0 );

index = 0;
if ( plane_cnt >= 1 )
    do 
	{
	_ImgGet( fid, Img_CompressionType, &comp_type, sizeof(long), 0, index );
	if ( comp_type != ImgK_PcmCompression )
	    status = FALSE;
	++index;
	}
    while ( index < plane_cnt && status == TRUE );

return status;
} /* end of No_compressed_data */
