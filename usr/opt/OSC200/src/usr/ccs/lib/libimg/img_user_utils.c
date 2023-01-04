
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

/*******************************************************************************
**  IMG_USER_UTILS.C
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	The utility functions in this module are for use in testing image 
**	frames.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	John M. Poltrack
**
**  CREATION DATE:     
**
**	1-OCT-1990
**
*******************************************************************************/

/*
**  Include files
*/
#include <math.h>
#include <string.h>

#include <img/ChfDef.h>
#include <img/ImgDef.h>
#include <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif


/*
**  Table of contents
**
**	Global routines
**
*/
#ifdef NODAS_PROTO
struct	    FCT	    *ImgMakeFrame();
struct	    FCT	    *ImgMakeCompressedFrame();
void		    ImgMakeFile();
void		    ImgMakeCompressedFile();
#endif

/*
**	Local routines
*/
#ifdef NODAS_PROTO
static unsigned long _FindMax();
#else
PROTO(static unsigned long _FindMax, (unsigned long /*data_type*/, unsigned char */*data_ptr*/, unsigned long /*pix_cnt*/));
#endif


/*
**  Equated Symbols
*/
#define FLOATSIZE   (sizeof(float))

/*
**  External References
*/
#ifdef NODAS_PROTO
struct FCT	*ImgAllocateFrame();
char		*ImgAllocDataPlane();
long		 ImgAttachDataPlane();
long		 ImgCloseFile();
long		 ImgExportFrame();
long		 ImgOpenFile();
long		 ImgStandardizeFrame();
void		 ImgVerifyFrame();

struct ITMLST	*_ImgCreateItmlst();
long		 _ImgGetVerifyStatus();

void		ChfSignal();
void		ChfStop();
#endif

/*
** External symbol definitions (status codes)
*/
#if defined(__VAXC) || defined(VAXC)
globalvalue
     ImgX_INVARGCON
    ,ImgX_UNSOPTION
    ;
#else
#include <img/ImgStatusCodes.h>
#endif

/******************************************************************************
**  ImgMakeFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine creates a frame structure from data and attributes 
**	specified by user for test purposes.
**
**  FORMAL PARAMETERS:
**
**	data_type	-   longword (unsigned), read only, by value
**			    determines data type of image frame to create:
**
**			    ImgK_ClassBitonal	    (bitsteam)
**			    ImgK_ClassGreyscale	    (byte,word,longword,float)
**			    ImgK_ClassMultispect    (byte,word,longword,float)
**
**	x,y		-   pixels_per_line and scanline count
**
**	num_of_planes	-   number of planes in frame
**
**	data_ptr_list	-   array of pointers to data arrays
**
**	flags		-   processing flag ImgM_VerifyOn to verify frame
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
**      retfid - id value of the created frame
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
******************************************************************************/
struct FCT *ImgMakeFrame(data_type,x,y,num_of_planes,data_ptr_list,flags)
unsigned long data_type;
unsigned long x,y;
unsigned long num_of_planes;
unsigned char *data_ptr_list[];
unsigned long flags;
{
unsigned char *data_plane[MAX_NUMBER_OF_COMPONENTS];

unsigned long byte_count;
unsigned long data_plane_size;
unsigned long idx;
/*
** frame attributes
*/
unsigned long bits_per_comp;
unsigned long data_class;
unsigned long plane_bits_per_pixel; 
unsigned long quant_levels;
unsigned long quant_levels_per_comp[MAX_NUMBER_OF_COMPONENTS]; 

struct FCT	*fid;
struct ITMLST	*itmlst,*itmlst_ptr;


/*
** Allocate an item list based upon data_type, x,y and num_of_planes
*/
switch (data_type)
    {
    case ImgK_DTypeBU:
	data_plane_size = CHARSIZE * x * y;

	if (num_of_planes > 1)
	    data_class = ImgK_ClassMultispect;
	else
	    data_class = ImgK_ClassGreyscale;
	bits_per_comp = CHARSIZE * 8;
	plane_bits_per_pixel = CHARSIZE * 8;
	quant_levels = 1 << 8;
	break;
    case ImgK_DTypeWU:
	data_plane_size = SHORTSIZE * x * y;

	if (num_of_planes > 1)
	    data_class = ImgK_ClassMultispect;
	else
	    data_class = ImgK_ClassGreyscale;
	bits_per_comp = SHORTSIZE * 8;
	plane_bits_per_pixel = SHORTSIZE * 8;
	quant_levels = 1 << 16;
	break;
    case ImgK_DTypeLU:
	data_plane_size = LONGSIZE * x * y;

	if (num_of_planes > 1)
	    data_class = ImgK_ClassMultispect;
	else
	    data_class = ImgK_ClassGreyscale;
	bits_per_comp = LONGSIZE * 8;
	quant_levels = 0x3fffffff;
	plane_bits_per_pixel = LONGSIZE * 8;
	break;
    case ImgK_DTypeV:
    case ImgK_DTypeVU:
	data_plane_size = ((x * y) + 7)/8;

	if (num_of_planes == 1)
	    data_class = ImgK_ClassBitonal;
	else
	    ChfStop(1, ImgX_INVARGCON);
	bits_per_comp = 1;
	plane_bits_per_pixel = 1;
	quant_levels = 2;
	break;
    case ImgK_DTypeF:
	ChfStop(1,ImgX_UNSOPTION);
	break;
    default:	
	ChfStop(1, ImgX_INVARGCON);
    }/* end data_type switch */

/*
** Set the quant levels based upon bits per comp;
*/
for( idx = 0; idx < num_of_planes; idx++)
    quant_levels_per_comp[idx] = quant_levels;

itmlst = (struct ITMLST *)_ImgCreateItmlst((5 * num_of_planes) + 1);
itmlst_ptr = itmlst;

for (idx = 0; idx < num_of_planes; idx++)
    {
    itmlst_ptr->ItmL_Code = Img_BitsPerComp;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&bits_per_comp;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_NumberOfLines;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&y;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_PixelsPerLine;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&x;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_PlaneBitsPerPixel;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&plane_bits_per_pixel;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_QuantLevelsPerComp;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&quant_levels_per_comp[idx];
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    }/* end plane loop */

/*
**  Create an empty frame with the proper attributes
*/
fid = (struct FCT *)
    ImgAllocateFrame(data_class, itmlst, 0, ImgM_NoDataPlaneAlloc);

/*
** Allocate and attach the data planes
*/
for (idx = 0; idx < num_of_planes; idx++)
    {
    unsigned char fill_pattern = 0;

    data_plane[idx] = (unsigned char *)
	ImgAllocDataPlane(data_plane_size,ImgM_InitMem,fill_pattern);

    /*
    **	Copy data into data_plane
    */
    memcpy(data_plane[idx],data_ptr_list[idx],data_plane_size);

    fid = (struct FCT *)ImgAttachDataPlane(fid,(char *)data_plane[idx],idx);

    }/* end plane loop */

if ((data_type == ImgK_DTypeVU) && (x % 8 != 0))
    fid = (struct FCT *)ImgStandardizeFrame(fid,ImgM_AutoDeallocate);

if ( VERIFY_ON_ )
    ImgVerifyFrame(fid, 0 );

return(fid);
}/* end of ImgMakeFrame */

/******************************************************************************
**  ImgMakeCompressedFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine creates a compressed frame structure from data and 
**	attributes specified by user for test purposes.
**
**  FORMAL PARAMETERS:
**
**	compression_type -  longword (unsigned), read only, by value
**			    determines compression type of image frame to create:
**
**			    ImgK_G31dCompression
**			    ImgK_G32dCompression
**			    ImgK_G42dCompression
**			    ImgK_DctCompression
**
**	x,y		-   pixels_per_line and scanline count
**
**	num_of_planes	-   number of planes in frame
**
**	data_ptr_list	-   array of pointers to data arrays
**
**	flags		-   processing flag ImgM_VerifyOn to verify frame
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
**      retfid - id value of the created frame
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
******************************************************************************/
struct FCT *ImgMakeCompressedFrame
		(compression_type,x,y,num_of_planes,data_ptr_list,flags)
unsigned long compression_type;
unsigned long x,y;
unsigned long num_of_planes;
unsigned char *data_ptr_list[];
unsigned long flags;
{
unsigned char *data_plane[MAX_NUMBER_OF_COMPONENTS];

unsigned long byte_count;
unsigned long data_plane_size;
unsigned long idx;
/*
** frame attributes
*/
unsigned long bits_per_comp;
unsigned long data_class;
unsigned long plane_bits_per_pixel; 
unsigned long quant_levels;
unsigned long quant_levels_per_comp[MAX_NUMBER_OF_COMPONENTS]; 

struct FCT	*fid;
struct ITMLST	*itmlst,*itmlst_ptr;


/*
** Allocate an item list based upon compression_type, x,y and num_of_planes
*/
switch (compression_type)
    {
    case ImgK_DctCompression:
	data_plane_size = CHARSIZE * x * y;

	if (num_of_planes > 1)
	    data_class = ImgK_ClassMultispect;
	else
	    data_class = ImgK_ClassGreyscale;
	bits_per_comp = CHARSIZE * 8;
	plane_bits_per_pixel = CHARSIZE * 8;
	quant_levels = 1 << 8;
	break;
    case ImgK_G31dCompression:
    case ImgK_G32dCompression:
    case ImgK_G42dCompression:
	data_plane_size = ((x * y) + 7)/8;

	if (num_of_planes == 1)
	    data_class = ImgK_ClassBitonal;
	else
	    ChfStop(1, ImgX_INVARGCON);
	bits_per_comp = 1;
	plane_bits_per_pixel = 1;
	quant_levels = 2;
	break;
    default:	
	ChfStop(1, ImgX_INVARGCON);
    }/* end data_type switch */

/*
** Set the quant levels based upon bits per comp;
*/
for( idx = 0; idx < num_of_planes; idx++)
    quant_levels_per_comp[idx] = quant_levels;

itmlst = (struct ITMLST *)_ImgCreateItmlst((5 * num_of_planes) + 1);
itmlst_ptr = itmlst;

for (idx = 0; idx < num_of_planes; idx++)
    {
    itmlst_ptr->ItmL_Code = Img_BitsPerComp;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&bits_per_comp;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_CompressionType;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&compression_type;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_NumberOfLines;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&y;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_PixelsPerLine;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&x;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_PlaneBitsPerPixel;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&plane_bits_per_pixel;
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    itmlst_ptr->ItmL_Code = Img_QuantLevelsPerComp;
    itmlst_ptr->ItmL_Length = sizeof(unsigned long);
    itmlst_ptr->ItmA_Buffer =  (char *)&quant_levels_per_comp[idx];
    itmlst_ptr->ItmA_Retlen = 0;
    itmlst_ptr->ItmL_Index = idx;
    itmlst_ptr++;

    }/* end plane loop */

/*
**  Create an empty frame with the proper attributes
*/
fid = (struct FCT *)
    ImgAllocateFrame(data_class, itmlst, 0, ImgM_NoDataPlaneAlloc);

/*
** Allocate and attach the data planes
*/
for (idx = 0; idx < num_of_planes; idx++)
    {
    unsigned char fill_pattern = 0;

    data_plane[idx] = (unsigned char *)
	ImgAllocDataPlane(data_plane_size,ImgM_InitMem,fill_pattern);

    /*
    **	Copy data into data_plane
    */
    memcpy(data_plane[idx],data_ptr_list[idx],data_plane_size);

    fid = (struct FCT *)ImgAttachDataPlane(fid,(char *)data_plane[idx],idx);

    }/* end plane loop */

return(fid);
}/* end of ImgMakeCompressedFrame */

/******************************************************************************
**  ImgMakeFile
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine creates a DDIF file from data and attributes 
**	specified by user for test purposes.
**
**  FORMAL PARAMETERS:
**
**	filename_str	-   char pointer to filename string
**
**	data_type	-   longword (unsigned), read only, by value
**			    determines data type of image frame to create:
**
**			    ImgK_ClassBitonal	    (bitsteam)
**			    ImgK_ClassGreyscale	    (byte,word,longword,float)
**			    ImgK_ClassMultispect    (byte,word,longword,float)
**
**	x,y		-   pixels_per_line and scanline count
**
**	num_of_planes	-   number of planes in frame
**
**	data_ptr_list	-   array of pointers to data arrays
**
**	flags		-   processing flag ImgM_VerifyOn to verify frame
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
**      retfid - id value of the created frame
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
******************************************************************************/
void ImgMakeFile(filename_str,data_type,x,y,num_of_planes,data_ptr_list,flags)

unsigned char *filename_str;
unsigned long data_type;
unsigned long x,y;
unsigned long num_of_planes;
unsigned char *data_ptr_list[];
unsigned long flags;
{
struct	    FCT	    *fid;
unsigned    long    byte_count;
struct DCB *output_ctx;

fid = ImgMakeFrame(data_type,x,y,num_of_planes,data_ptr_list,flags);

if ((data_type == ImgK_DTypeVU) && (x % 8 != 0))
    fid = (struct FCT *)ImgStandardizeFrame(fid,ImgM_AutoDeallocate);

/*
** Open file for export
*/
output_ctx = ImgOpenFile(
		ImgK_ModeExport,
		ImgK_FtypeDDIF,
		strlen( (char *)filename_str),
		(char *)filename_str,
		0,
		0);

byte_count = ImgExportFrame(
		fid,
		output_ctx,
		0);

ImgCloseFile(
   output_ctx,
   0);

}/* end of ImgMakeFile */

/******************************************************************************
**  ImgMakeCompressedFile
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine creates a DDIF file from compressed data and attributes 
**	specified by user for test purposes.
**
**  FORMAL PARAMETERS:
**
**	filename_str	-   char pointer to filename string
**
**	compressed_type	-   longword (unsigned), read only, by value
**			    determines type of image frame to create:
**
**
**			    ImgK_G31dCompression
**			    ImgK_G32dCompression
**			    ImgK_G42dCompression
**			    ImgK_DctCompression
**
**	x,y		-   pixels_per_line and scanline count
**
**	num_of_planes	-   number of planes in frame
**
**	data_ptr_list	-   array of pointers to data arrays
**
**	flags		-   processing flag ImgM_VerifyOn to verify frame
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
**      retfid - id value of the created frame
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
******************************************************************************/
void ImgMakeCompressedFile
	(filename_str,compression_type,x,y,num_of_planes,data_ptr_list,flags)

unsigned char *filename_str;
unsigned long compression_type;
unsigned long x,y;
unsigned long num_of_planes;
unsigned char *data_ptr_list[];
unsigned long flags;
{
struct	    FCT	    *fid;
unsigned    long    byte_count;
struct DCB *output_ctx;

fid = ImgMakeCompressedFrame
	(compression_type,x,y,num_of_planes,data_ptr_list,flags);


/*
** Open file for export
*/
output_ctx = ImgOpenFile(
		ImgK_ModeExport,
		ImgK_FtypeDDIF,
		strlen( (char *)filename_str),
		(char *)filename_str,
		0,
		0);

byte_count = ImgExportFrame(
		fid,
		output_ctx,
		0);

ImgCloseFile(
   output_ctx,
   0);

}/* end of ImgMakeCompressedFile */

/******************************************************************************
**  _FindMax
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine returns the highest pixel value in an array.
**
**  FORMAL PARAMETERS:
**
**	data_type	-   longword (unsigned), read only, by value
**			    determines data type of array
**
**	data_ptr	-   data_type pointer to the array
**
**	pix_cnt		-   number of pixels to check
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
**      maximum pixel value
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
******************************************************************************/
static unsigned long _FindMax(data_type,data_ptr,pix_cnt)
unsigned long data_type;
unsigned char *data_ptr;
unsigned long pix_cnt;
{
unsigned long	idx;
unsigned long	max_value;
unsigned char	*byte_ptr;
unsigned short	*word_ptr;
unsigned long	*long_ptr;

switch (data_type)
    {
    case ImgK_DTypeBU:
	byte_ptr = (unsigned char *)data_ptr;
	for( idx = 0, max_value = 0; idx < pix_cnt; idx++)
	    {
	    if (*byte_ptr > max_value) 
		max_value = *byte_ptr;
	    byte_ptr++;
	    }	    	    	
	break;
    case ImgK_DTypeWU:
	word_ptr = (unsigned short *)data_ptr;
	for( idx = 0, max_value = 0; idx < pix_cnt; idx++)
	    {
	    if (*word_ptr > max_value) 
		max_value = *word_ptr;
	    word_ptr++;
	    }	    	    	
	break;
    case ImgK_DTypeLU:
	long_ptr = (unsigned long *)data_ptr;
	for( idx = 0, max_value = 0; idx < pix_cnt; idx++)
	    {
	    if (*long_ptr > max_value) 
		max_value = *long_ptr;
	    long_ptr++;
	    }	    	    	
	break;
    case ImgK_DTypeV:
    case ImgK_DTypeVU:
	max_value = 1;
	break;
    case ImgK_DTypeF:
	ChfStop(1,ImgX_UNSOPTION);
	break;
    default:	
	ChfStop(1, ImgX_INVARGCON);
    }/* end data_type switch */

/*
** Check for machine limit (for some reason doesn't work under DECC)
*/
if (max_value == (1 >> 31))
    max_value -= 1;

return (max_value);
}/* end _FindMax */
