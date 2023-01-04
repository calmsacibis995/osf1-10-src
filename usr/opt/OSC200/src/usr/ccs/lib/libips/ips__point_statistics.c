/************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**/

/************************************************************************
**
**  FACILITY:
**
**      Image Core Services
**
**  ABSTRACT:
**
**      This module contains the user level service and support routines
**	for determination of local area statistical information from image
**      data at and surrounding a specified pixel.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Karen Rodwell
**
**  CREATION DATE:
**
**      14-June-1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long _IpsPointStatistics();         /* main entry routine, tests dtype       */
long _IpsStatisticsByt();	    /* Statistics on entire byte plane       */
long _IpsStatisticsCppByt();	    /* Statistics on bytes in src set in cpp */
long _IpsStatisticsW();		    /* Statistics on entire word plane       */
long _IpsStatisticsCppW();	    /* Statistics on words in src set in cpp */
long _IpsStatisticsLong();	    /* Statistics on entire long type plane  */
long _IpsStatisticsCppLong();	    /* Statistics on lwords in src set in cpp*/
long _IpsStatisticsFlt();	    /* Stats on entire float type plane	     */
long _IpsStatisticsCppFlt();	    /* Stats on float data where set in cpp  */
#endif


/*
**  Include files
*/
#include <IpsDef.h>			    /* IPS Image Definitions	     */
#include <IpsMacros.h>			    /* IPS Macro Definitions	     */
#include <IpsMemoryTable.h>                 /* IPS Memory Mgt Functions      */
#include <IpsStatusCodes.h>		    /* IPS Status Codes		     */
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif

/*****************************************************************************
**  _IpsPointStatistics - 
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      src_udp -- Pointer to first source udp
**	cpp  -- Control Processing Plane Descriptor.
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsPointStatistics(src_udp, cpp, number_of_pixels, number_of_zeroes, minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
struct UDP *cpp;           /* Control Processing Plane */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
if  (src_udp->UdpB_Class != UdpK_ClassA)
    return (IpsX_UNSOPTION);

if ((src_udp->UdpB_DType == UdpK_DTypeVU) ||
    (src_udp->UdpB_DType == UdpK_DTypeV))
    return (IpsX_INVDTYPE);

/*
** Validate cpp if present
*/
if (cpp != 0)
    VALIDATE_CPP_(cpp, src_udp);

*sum = 0.0;
*sum_of_squares = 0.0;

switch (src_udp->UdpB_DType)
    {
    case UdpK_DTypeBU:
	if (cpp != 0)
	    return (_IpsStatisticsCppByt(src_udp, cpp, number_of_pixels, 
		number_of_zeroes, minimum, maximum, sum, sum_of_squares));
	else
	    return (_IpsStatisticsByt(src_udp, number_of_pixels, 
		number_of_zeroes, minimum, maximum, sum, sum_of_squares));
    case UdpK_DTypeWU:
	if (cpp != 0)
	    return (_IpsStatisticsCppW(src_udp, cpp, number_of_pixels, 
		number_of_zeroes, minimum, maximum, sum, sum_of_squares));
	else
	    return (_IpsStatisticsW(src_udp, number_of_pixels, 
		number_of_zeroes, minimum, maximum, sum, sum_of_squares));

    case UdpK_DTypeLU: 
        if (cpp != 0)
	    return (_IpsStatisticsCppLong(src_udp, cpp, number_of_pixels, 
		number_of_zeroes, minimum, maximum, sum, sum_of_squares));
	else
	    return (_IpsStatisticsLong(src_udp, number_of_pixels, 
		number_of_zeroes, minimum, maximum, sum, sum_of_squares));

    case UdpK_DTypeF:
	if (cpp != 0)
	    return (_IpsStatisticsCppFlt(src_udp, cpp, number_of_pixels, 
		number_of_zeroes, minimum, maximum, sum, sum_of_squares));
	else
	    return (_IpsStatisticsFlt(src_udp, number_of_pixels, 
		number_of_zeroes, minimum, maximum, sum, sum_of_squares));

    default:
        return (IpsX_UNSOPTION);
        break;
    };  /* end switch on DType */
return (IpsX_SUCCESS);
}

/*****************************************************************************
**  _IpsStatisticsByt - 
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      src_udp -- Pointer to first source udp
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsStatisticsByt(src_udp, number_of_pixels, number_of_zeroes, minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
unsigned char   *src_ptr;   /* pointer to source data base */
unsigned long   ix,iy;      /* loop counters               */
unsigned long   src_stride; /* src stride in bytes	   */
unsigned long   src_pad;    /* src pad                     */

src_ptr= (unsigned char *)src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>3);

src_stride = (src_udp->UdpL_ScnStride >> 3);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
*minimum = *src_ptr;
*maximum = *src_ptr;

for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
        {
	*number_of_pixels += 1.0;
	if (*src_ptr == 0)
	    *number_of_zeroes += 1.0;
	if (*src_ptr > *maximum)
	    *maximum = *src_ptr;	
	if (*src_ptr < *minimum)
	    *minimum = *src_ptr;	
	*sum += *src_ptr;
	*sum_of_squares += (*src_ptr * *src_ptr);
	src_ptr++;
        }/* end ix loop */
    src_ptr += src_pad;
    } /*end iy loop */
return (IpsX_SUCCESS);
}/* end of _IpsStatisticsByt */

/*****************************************************************************
**  _IpsStatisticsCppByt - 
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      src_udp -- Pointer to first source udp
**	cpp  -- Control Processing Plane Descriptor.
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
**
**
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsStatisticsCppByt(src_udp, cpp, number_of_pixels, number_of_zeroes, minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
struct UDP *cpp;           /* Control Processing Plane */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
unsigned char   *src_ptr;   /* pointer to source data base */
unsigned long   ix,iy;      /* loop counters               */
unsigned long   src_stride; /* src stride in bytes	   */
unsigned long   src_pad;    /* src pad                     */
unsigned char   *cpp_ptr;   /* control proc plane base     */
unsigned long   cpp_stride; /* control proc plane stride   */

cpp_ptr = (unsigned char *) cpp->UdpA_Base
                + (((cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1)+7)/8);

cpp_stride = (cpp->UdpL_ScnStride + 7)/8;
src_ptr= (unsigned char *)src_udp->UdpA_Base + 
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>3);
src_stride = (src_udp->UdpL_ScnStride >> 3);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
*minimum = 255.0;
*maximum = 0.0;

for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
        {
        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
	    {
	    *number_of_pixels += 1.0;
	    if (*src_ptr == FALSE)
	        *number_of_zeroes += 1.0;
	    if (*src_ptr > *maximum)
		*maximum = *src_ptr;	
	    if (*src_ptr < *minimum)
		*minimum = *src_ptr;	
	    *sum += *src_ptr;
	    *sum_of_squares += (*src_ptr * *src_ptr);
	    }
	src_ptr++;
        }/* end ix loop */
    cpp_ptr += cpp_stride;
    src_ptr += src_pad;
    } /*end iy loop */
return (IpsX_SUCCESS);
}/* end of _IpsStatisticsCppByt */

/*****************************************************************************
**  _IpsStatisticsW -
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      src_udp -- Pointer to first source udp
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
**
**
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
**********************************************************************/
long _IpsStatisticsW(src_udp, number_of_pixels, number_of_zeroes, 
minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
unsigned short int *src_ptr;  /* pointer to source data base */
unsigned long      size;                   /* buffer size                 */
unsigned long      ix,iy;                  /* loop counters               */
unsigned long      src_stride;            /* src stride in bytes        */
unsigned long      src_pad;               /* src pad                    */

src_ptr = (unsigned short int *)src_udp->UdpA_Base + 
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>4);
src_stride = (src_udp->UdpL_ScnStride >> 4);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
*minimum = *src_ptr;
*maximum = *src_ptr;        
for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
        {
	*number_of_pixels += 1.0;
	if (*src_ptr == FALSE)
	    *number_of_zeroes += 1.0;
	if (*src_ptr > *maximum)
	    *maximum = *src_ptr;	
	if (*src_ptr < *minimum)
	    *minimum = *src_ptr;	
	*sum += *src_ptr;
	*sum_of_squares += (*src_ptr * *src_ptr);
	src_ptr++;
	}/* end ix loop */
    src_ptr  += src_pad;
    } /*end iy loop */
return (IpsX_SUCCESS);
}/* end of _IpsStatisticsW */

/*****************************************************************************
**  _IpsStatisticsCppW -
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      src_udp -- Pointer to first source udp
**	cpp  -- Control Processing Plane Descriptor.
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
**********************************************************************/
long _IpsStatisticsCppW(src_udp, cpp, number_of_pixels, number_of_zeroes, minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
struct UDP *cpp;           /* Control Processing Plane */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
unsigned short int *src_ptr;   /* pointer to source data base */
unsigned long      size;       /* buffer size                 */
unsigned long      ix,iy;      /* loop counters               */
unsigned long      src_stride; /* src stride in bytes         */
unsigned long      src_pad;    /* src pad                     */
unsigned char   *cpp_ptr;      /* control proc plane base     */
unsigned long   cpp_stride;    /* control proc plane stride   */

src_ptr = (unsigned short int *)src_udp->UdpA_Base + 
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>4);
src_stride = (src_udp->UdpL_ScnStride >> 4);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
cpp_ptr = (unsigned char *) cpp->UdpA_Base 
                + (((cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1)+7)/8);
cpp_stride = (cpp->UdpL_ScnStride + 7)/8;
*minimum = 65535.0;
*maximum = 0.0;

for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
        {
        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
	    {
	    *number_of_pixels += 1.0;
	    if (*src_ptr == FALSE)
	        *number_of_zeroes += 1.0;
	    if (*src_ptr > *maximum)
		*maximum = *src_ptr;	
	    if (*src_ptr < *minimum)
		*minimum = *src_ptr;	
	    *sum += *src_ptr;
	    *sum_of_squares += (*src_ptr * *src_ptr);
	    }
	src_ptr++;
	}/* end ix loop */
    cpp_ptr += cpp_stride;
    src_ptr  += src_pad;
    } /*end iy loop */
return (IpsX_SUCCESS);
}/* end of _IpsStatisticsCppW */


/*****************************************************************************
**  _IpsStatisticsLong -
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      src_udp -- Pointer to first source udp
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
**********************************************************************/
long _IpsStatisticsLong(src_udp, number_of_pixels, number_of_zeroes, minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
unsigned long      *src_ptr;      /* pointer to source data base */
unsigned long      size;          /* buffer size                 */
unsigned long      ix,iy;         /* loop counters               */
unsigned long      src_stride;    /* src stride in bytes         */
unsigned long      src_pad;       /* src pad                     */

src_ptr = (unsigned long *)src_udp->UdpA_Base + 
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>5);

src_stride = (src_udp->UdpL_ScnStride >> 5);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
*minimum = *src_ptr;
*maximum = *src_ptr;        
for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
        {
	*number_of_pixels += 1.0;
	if (*src_ptr == FALSE)
	    *number_of_zeroes += 1.0;
	if (*src_ptr > *maximum)
	    *maximum = *src_ptr;	
	if (*src_ptr < *minimum)
	    *minimum = *src_ptr;	
	*sum += *src_ptr;
	*sum_of_squares += (*src_ptr * *src_ptr);
	src_ptr++;
	}/* end ix loop */
    src_ptr  += src_pad;
    } /*end iy loop */
return (IpsX_SUCCESS);
}/* end of _IpsStatisticsLong */

/*****************************************************************************
**  _IpsStatisticsCppLong -
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      src_udp -- Pointer to first source udp
**	cpp  -- Control Processing Plane Descriptor.
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
**********************************************************************/
long _IpsStatisticsCppLong(src_udp, cpp, number_of_pixels, number_of_zeroes, minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
struct UDP *cpp;           /* Control Processing Plane */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
unsigned long      *src_ptr;   /* pointer to source data base */
unsigned long      size;       /* buffer size                 */
unsigned long      ix,iy;      /* loop counters               */
unsigned long      src_stride; /* src stride in bytes         */
unsigned long      src_pad;    /* src pad                     */
unsigned char   *cpp_ptr;      /* control proc plane base     */
unsigned long   cpp_stride;    /* control proc plane stride   */

src_ptr = (unsigned long *)src_udp->UdpA_Base + 
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>5);
src_stride = (src_udp->UdpL_ScnStride >> 5);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
cpp_ptr = (unsigned char *) cpp->UdpA_Base 
                + (((cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1)+7)/8);
cpp_stride = (cpp->UdpL_ScnStride + 7)/8;
*minimum = 4294967295.0;
*maximum = 0.0;        
for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
        {
        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
	    {
	    *number_of_pixels += 1.0;
	    if (*src_ptr == FALSE)
	        *number_of_zeroes += 1.0;
	    if (*src_ptr > *maximum)
		*maximum = *src_ptr;	
	    if (*src_ptr < *minimum)
		*minimum = *src_ptr;	
	    *sum += *src_ptr;
	    *sum_of_squares += (*src_ptr * *src_ptr);
	    }
	src_ptr++;
	}/* end ix loop */
    cpp_ptr += cpp_stride;
    src_ptr  += src_pad;
    } /*end iy loop */
return (IpsX_SUCCESS);
}/* end of _IpsStatisticsCppLong */

/*****************************************************************************
**  _IpsStatisticsFlt - 
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      src_udp -- Pointer to first source udp
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
**
**
** 
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsStatisticsFlt(src_udp, number_of_pixels, number_of_zeroes, minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
float	        *src_ptr;       /* pointer to source data base */
unsigned long   size;           /* buffer size                 */
unsigned long   ix,iy;          /* loop counters               */
unsigned long   src_stride;     /* src stride in bytes	       */
unsigned long   src_pad;        /* src pad                     */

src_ptr= (float *)src_udp->UdpA_Base + 
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>5);

src_stride = (src_udp->UdpL_ScnStride >> 5);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
*minimum = *src_ptr;
*maximum = *src_ptr;
        
for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
        {
	*number_of_pixels += 1.0;
	if (*src_ptr == FALSE)
	    *number_of_zeroes += 1.0;
	if (*src_ptr > *maximum)
	    *maximum = *src_ptr;	
	if (*src_ptr < *minimum)
	    *minimum = *src_ptr;	
	*sum += *src_ptr;
	*sum_of_squares += (*src_ptr * *src_ptr);
	src_ptr++;
	}/* end ix loop */
    src_ptr += src_pad;
    } /*end iy loop */
return (IpsX_SUCCESS);
}/* end of _IpsStatisticsFlt */

/*****************************************************************************
**  _IpsStatisticsCppFlt - 
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      src_udp -- Pointer to first source udp
**	cpp  -- Control Processing Plane Descriptor.
**      number_of_pixels -- Number of Pixels in floating point in region.
**      number_of_zeroes -- Number of zero pixels in floating point in region.
**      minimum -- Floating point minimum value of pixels in the region.
**      maximum -- Floating point maximum value of pixels in the region.
**      sum -- Floating point sum of pixel values in the region.
**      sum_of_squares -- Floating point sum of squares of pixel values 
**			  in the region.
** 
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsStatisticsCppFlt(src_udp, cpp, number_of_pixels, number_of_zeroes, minimum, maximum, sum, sum_of_squares)
struct UDP *src_udp;       /* Source UDP */
struct UDP *cpp;           /* Control Processing Plane */
double *number_of_pixels;
double *number_of_zeroes;
double *minimum;
double *maximum;
double *sum;
double *sum_of_squares;
{
float	        *src_ptr;   /* pointer to source data base */
unsigned long   size;       /* buffer size                 */
unsigned long   ix,iy;      /* loop counters               */
unsigned long   src_stride; /* src stride in bytes	   */
unsigned long   src_pad;    /* src pad                     */
unsigned char   *cpp_ptr;   /* control proc plane base     */
unsigned long   cpp_stride; /* control proc plane stride   */

src_ptr= (float *)src_udp->UdpA_Base + 
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>5);

src_stride = (src_udp->UdpL_ScnStride >> 5);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
cpp_ptr = (unsigned char *) cpp->UdpA_Base 
                + (((cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1)+7)/8);
cpp_stride = (cpp->UdpL_ScnStride + 7)/8;
*minimum = 4294967295.0;
*maximum = 0.0;
        
for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
        {
        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
	    {
	    *number_of_pixels += 1.0;
	    if (*src_ptr == FALSE)
	        *number_of_zeroes += 1.0;
	    if (*src_ptr > *maximum)
		*maximum = *src_ptr;	
	    if (*src_ptr < *minimum)
		*minimum = *src_ptr;	
	    *sum += *src_ptr;
	    *sum_of_squares += (*src_ptr * *src_ptr);
	    }
	src_ptr++;
	}/* end ix loop */
    cpp_ptr += cpp_stride;
    src_ptr += src_pad;
    } /*end iy loop */
return (IpsX_SUCCESS);
}/* end of _IpsStatisticsCppFlt */
