/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */

/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      X Image Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains code for image data histograms.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      John Weber
**
**  CREATION DATE:
**
**      December 28, 1989
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>

#include "SmiHistogram.h"
#include "XieDdx.h"

/*
**  Table of contents
*/
int SmiHistogram();

/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**	Local Storage
*/

/*****************************************************************************
**  SmiHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function ...
**
**  FORMAL PARAMETERS:
**
**      src	    - src udp
**	idc	    - cpp or roi (optional)
**	histogram   - histogram buffer
**
**  FUNCTION VALUE:
**
**      Success
**
*****************************************************************************/
int SmiHistogram( src, idc, histogram )
UdpPtr	 src;
UdpPtr   idc;
unsigned long histogram[];
{
    long int scanline;
    long int pixel;

    int src_mask = (1 << src->UdpW_PixelLength) - 1;
    long int src_scan_offset;
    long int src_offset;

    long int cpp_scan_offset;
    long int cpp_offset;

    unsigned long value;
    long int x1, y1, x2, y2;

    memset( histogram, NULL, src->UdpL_Levels * sizeof(long) );

    /* Determine the boundaries of the intersection of the image and the
     * ROI or CPP (if present).
     */
    if( idc == NULL )
	{
	x1 = src->UdpL_X1;
	x2 = src->UdpL_X2;
	y1 = src->UdpL_Y1;
	y2 = src->UdpL_Y2;
	}
    else
	{
	x1 = src->UdpL_X1 > idc->UdpL_X1 ? src->UdpL_X1 : idc->UdpL_X1;
	x2 = src->UdpL_X2 < idc->UdpL_X2 ? src->UdpL_X2 : idc->UdpL_X2;
	y1 = src->UdpL_Y1 > idc->UdpL_Y1 ? src->UdpL_Y1 : idc->UdpL_Y1;
	y2 = src->UdpL_Y2 < idc->UdpL_Y2 ? src->UdpL_Y2 : idc->UdpL_Y2;
	}
    src_scan_offset = src->UdpL_Pos + (y1 - src->UdpL_Y1) * src->UdpL_ScnStride
				    + (x1 - src->UdpL_X1) * src->UdpL_PxlStride;

    if( idc == NULL || idc->UdpA_Base == NULL )
	{
	/* Not using a CPP */
	for( scanline = y1;  scanline <= y2 ;  scanline++ )
	    {
	    src_offset = src_scan_offset;

	    for( pixel = x1; pixel <= x2; pixel++ )
		{
		value = GET_VALUE_(src->UdpA_Base, src_offset, src_mask);
		histogram[value]++;
		src_offset += src->UdpL_PxlStride;
		}
	    src_scan_offset += src->UdpL_ScnStride;
	    }
	}
    else
	{
	/* Using a CPP */
	cpp_scan_offset = idc->UdpL_Pos;

	for( scanline = y1;  scanline <= y2 ;  scanline++ )
	    {
	    src_offset = src_scan_offset;
	    cpp_offset = cpp_scan_offset;

	    for( pixel = x1; pixel <= x2; pixel++ )
		{
		if( GET_VALUE_(idc->UdpA_Base, cpp_offset, 1) )
		    {
		    value = GET_VALUE_(src->UdpA_Base, src_offset, src_mask);
		    histogram[value]++;
		    }
		src_offset += src->UdpL_PxlStride;
		cpp_offset += idc->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    cpp_scan_offset += idc->UdpL_ScnStride;
	    }
	}	
    return( Success );
}
