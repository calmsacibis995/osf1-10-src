/************************************************************************
**
**   COPYRIGHT (c) 1989 BY
**   DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**   ALL RIGHTS RESERVED.
**   
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
** ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
** COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
************************************************************************/

/************************************************************************
**  ImgVectorTable.H
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This include file contains an external reference to the ISL dispatch
**	table and defines symbols for each supported Layer II routine.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Richard Piccolo
**
**  CREATION DATE:
**
**	8-AUG-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/* define symbols for each supported Layer II function */

# define ImgK_ScaleInterpolation		0
# define ImgK_ScaleNearestNeighbor		1
# define ImgK_FlipHorizontal			2
# define ImgK_FlipVertical			3
# define ImgK_PixelRemapLUT			4
# define ImgK_Alloc				5
# define ImgK_Dealloc				6
# define ImgK_Realloc				7
# define ImgK_AllocateDataPlane			8 
# define ImgK_FreeDataPlane			9 
# define ImgK_ReallocateDataPlane		10 
# define ImgK_RESERVED_1			11 
# define ImgK_Combine				12
# define ImgK_RotateBitonal			13
# define ImgK_RotateOrthogonal			14
# define ImgK_RotateNearestNeighbor		15 
# define ImgK_RotateInterpolation		16
# define ImgK_BluenoiseDither			17
# define ImgK_OrderedDither			18
# define ImgK_EncodeG31d                        19
# define ImgK_EncodeG32d                        20
# define ImgK_EncodeG42d                        21
# define ImgK_DecodeG31d                        22
# define ImgK_DecodeG32d                        23
# define ImgK_DecodeG42d                        24
# define ImgK_ScaleBitonal			25
# define ImgK_Histogram				26
# define ImgK_HistogramSorted			27
# define ImgK_Copy				28
# define ImgK_Convolve				29
/*
** These are the new guys for Ips, someday we'll put them in order
*/
# define ImgK_CopyBitonal			30
# define ImgK_FlipHorizontalBitonal		31
# define ImgK_FlipVerticalBitonal		32
# define ImgK_MergePlanes			33
# define ImgK_LogicalBitonal			34
# define ImgK_Logical				35
# define ImgK_EncodeDct				36
# define ImgK_DecodeDct				37

/* 
** External reference to the ImgA_VectorTable which is initialized in 
** img_dispatch.c
*/

#if defined(__VAXC) || defined(VAXC)
globalref int (*ImgA_VectorTable[])();
#else
extern int (*ImgA_VectorTable[])();
#endif
                      
/* end of ImgVectorTable.h */

