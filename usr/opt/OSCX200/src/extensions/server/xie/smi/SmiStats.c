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
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
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
**      X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains routines for computing image data statistics.
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
**      May 15, 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
#include <X.h>				/* X definitions		    */
#include <XieDdx.h>                     /* XIE device dependant definitions */
#include <XieUdpDef.h>			/* UDP structure definitions	    */
#ifndef VXT
#include <math.h>
#else
#include <vxtmath.h>
#endif

/*
**  Table of contents
*/

/*
**  Point statistics routines.
*/

int		    SmiStatsPoint();

static int	    PointStatsByte();
static int	    PointStatsWord();
static int	    PointStatsLong();
static int	    PointStatsFloat();
static int	    PointStatsBits();
/*
**  Area statistics routines.
*/
int		    SmiStatsArea();

static int	    _AreaErr();

static int	    _AreaMinB();
static int	    _AreaMinW();
static int	    _AreaMinL();
static int	    _AreaMinF();
static int	    _AreaMinV();

static int	    _AreaMaxB();
static int	    _AreaMaxW();
static int	    _AreaMaxL();
static int	    _AreaMaxF();
static int	    _AreaMaxV();

static int	    _AreaMeanB();
static int	    _AreaMeanW();
static int	    _AreaMeanL();
static int	    _AreaMeanF();
static int	    _AreaMeanV();

static int	    _AreaStdDevB();
static int	    _AreaStdDevW();
static int	    _AreaStdDevL();
static int	    _AreaStdDevF();
static int	    _AreaStdDevV();

static int	    _AreaVarB();
static int	    _AreaVarW();
static int	    _AreaVarL();
static int	    _AreaVarF();
static int	    _AreaVarV();

static int	    _AreaCppMinB();
static int	    _AreaCppMinW();
static int	    _AreaCppMinL();
static int	    _AreaCppMinF();
static int	    _AreaCppMinV();

static int	    _AreaCppMaxB();
static int	    _AreaCppMaxW();
static int	    _AreaCppMaxL();
static int	    _AreaCppMaxF();
static int	    _AreaCppMaxV();

static int	    _AreaCppMeanB();
static int	    _AreaCppMeanW();
static int	    _AreaCppMeanL();
static int	    _AreaCppMeanF();
static int	    _AreaCppMeanV();

static int	    _AreaCppStdDevB();
static int	    _AreaCppStdDevW();
static int	    _AreaCppStdDevL();
static int	    _AreaCppStdDevF();
static int	    _AreaCppStdDevV();

static int	    _AreaCppVarB();
static int	    _AreaCppVarW();
static int	    _AreaCppVarL();
static int	    _AreaCppVarF();
static int	    _AreaCppVarV();

/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/
#define XieK_StypeMin	    0
#define XieK_StypeMinimum   1
#define XieK_StypeMaximum   2
#define XieK_StypeMean	    3
#define XieK_StypeStdDev    4
#define XieK_StypeVariance  5
#define XieK_StypeMax	    6
/*
**  External References
*/

/*
**	Local Storage
*/
    /*	 
    **  These tables describe routines to call for statistics based on source
    **	data type, selector, and if a CPP is required.
    */
static int (*area_stats_table[UdpK_DTypeMax+1][XieK_StypeMax])() = {
    _AreaErr, _AreaErr,  _AreaErr,  _AreaErr,   _AreaErr,     _AreaErr,
    _AreaErr, _AreaMinB, _AreaMaxB, _AreaMeanB, _AreaStdDevB, _AreaVarB,
    _AreaErr, _AreaMinW, _AreaMaxW, _AreaMeanW, _AreaStdDevW, _AreaVarW,
    _AreaErr, _AreaMinL, _AreaMaxL, _AreaMeanL, _AreaStdDevL, _AreaVarL,
    _AreaErr, _AreaMinF, _AreaMaxF, _AreaMeanF, _AreaStdDevF, _AreaVarF,
    _AreaErr, _AreaMinV, _AreaMaxV, _AreaMeanV, _AreaStdDevV, _AreaVarV,
    _AreaErr, _AreaMinV, _AreaMaxV, _AreaMeanV, _AreaStdDevV, _AreaVarV
};

static int (*area_stats_cpp_table[UdpK_DTypeMax+1][XieK_StypeMax])() = {
    _AreaErr, _AreaErr,     _AreaErr,     _AreaErr,      _AreaErr,        _AreaErr,
    _AreaErr, _AreaCppMinB, _AreaCppMaxB, _AreaCppMeanB, _AreaCppStdDevB, _AreaCppVarB,
    _AreaErr, _AreaCppMinW, _AreaCppMaxW, _AreaCppMeanW, _AreaCppStdDevW, _AreaCppVarW,
    _AreaErr, _AreaCppMinL, _AreaCppMaxL, _AreaCppMeanL, _AreaCppStdDevL, _AreaCppVarL,
    _AreaErr, _AreaCppMinF, _AreaCppMaxF, _AreaCppMeanF, _AreaCppStdDevF, _AreaCppVarF,
    _AreaErr, _AreaCppMinV, _AreaCppMaxV, _AreaCppMeanV, _AreaCppStdDevV, _AreaCppVarV,
    _AreaErr, _AreaCppMinV, _AreaCppMaxV, _AreaCppMeanV, _AreaCppStdDevV, _AreaCppVarV
};

/*****************************************************************************
**  SmiStatsPoint
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine returns statistics about a rectangular region surrounding 
**	a pixel location.
**
**  FORMAL PARAMETERS:
**
**      src	- source data descriptor.
**	x	- X location of pixel.
**	y	- Y location of pixel.
**	width	- width of region.
**	height	- height of region.
**	value	- returned value of pixel at (x,y).
**	min	- returned minimum pixel value in region.
**	max	- returned maxumum pixel value in region.
**	mean	- returned mean pixel value in region.
**	stddev	- returned standard deviation of pixel values in region.
**	var	- returned variance of pixel values in region.
**
**  FUNCTION VALUE:
**
**      status, returns Success, or error.
**
*****************************************************************************/
int	     SmiStatsPoint(src,x,y,width,height,value,min,max,mean,stddev,var)
UdpPtr	     src;	/* source data descriptor			    */
int	     x;		/* X location of pixel				    */
int	     y;		/* Y location of pixel				    */
int          width;	/* width of region				    */
int	     height;	/* height of region				    */
double	    *value;	/* returned value of pixel at (x,y)		    */
double	    *min;	/* returned minimum pixel value in region	    */
double	    *max;	/* returned maxumum pixel value in region	    */
double	    *mean;	/* returned mean pixel value in region		    */
double	    *stddev;	/* returned standard deviation of pixel values in   */
			/*     region					    */
double	    *var;	/* returned variance of pixel values in region	    */
{
    UdpRec  srcudp;
    int	    valoff;
    int	    status;
    /*
    **	Initial X,Y must be inside the source image.
    */
    if (x < src->UdpL_X1 || x > src->UdpL_X2 ||
	y < src->UdpL_Y1 || y > src->UdpL_Y2)
        return( BadValue );
    /*
    **	Determine the spatial extent over which the image statistics
    **	will be calculated. If the corners of the region go out of range of
    **	the source, they are reset to the nearest edge.
    */
    srcudp = *src;

    srcudp.UdpL_X1 = x - width / 2;
    srcudp.UdpL_Y1 = y - height / 2;
    srcudp.UdpL_X2 = srcudp.UdpL_X1 + width;
    srcudp.UdpL_Y2 = srcudp.UdpL_Y1 + height;

    if (srcudp.UdpL_X1 < src->UdpL_X1)	srcudp.UdpL_X1 = src->UdpL_X1;
    if (srcudp.UdpL_Y1 < src->UdpL_Y1)	srcudp.UdpL_Y1 = src->UdpL_Y1;
    if (srcudp.UdpL_X2 > src->UdpL_X2)	srcudp.UdpL_X2 = src->UdpL_X2;
    if (srcudp.UdpL_Y2 > src->UdpL_Y2)	srcudp.UdpL_Y2 = src->UdpL_Y2;

    srcudp.UdpL_PxlPerScn = srcudp.UdpL_X2 - srcudp.UdpL_X1 + 1;
    srcudp.UdpL_ScnCnt    = srcudp.UdpL_Y2 - srcudp.UdpL_Y1 + 1;

    srcudp.UdpL_Pos += (srcudp.UdpL_X1 - src->UdpL_X1) * src->UdpL_PxlStride +
		       (srcudp.UdpL_Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    /*
    **	Now, call calculation routine based on data type.
    */
    valoff = srcudp.UdpL_Pos + (y - srcudp.UdpL_Y1) * srcudp.UdpL_ScnStride
			     + (x - srcudp.UdpL_X1) * srcudp.UdpL_PxlStride;
	     
    switch (srcudp.UdpB_DType)
	{
    case UdpK_DTypeBU:
	*value = *(unsigned char *)(srcudp.UdpA_Base + valoff / 8);
	status = PointStatsByte(&srcudp,min,max,mean,var);
	break;	    
    case UdpK_DTypeWU:
	*value = *(unsigned short *)(srcudp.UdpA_Base + valoff / 8);
	status = PointStatsWord(&srcudp,min,max,mean,var);
	break;	    
    case UdpK_DTypeLU:
	*value = *(unsigned char *)(srcudp.UdpA_Base + valoff / 8);
	status = PointStatsLong(&srcudp,min,max,mean,var);
	break;	    
    case UdpK_DTypeF:
	*value = *(unsigned char *)(srcudp.UdpA_Base + valoff / 8);
	status = PointStatsFloat(&srcudp,min,max,mean,var);
	break;	    
    case UdpK_DTypeVU:
    case UdpK_DTypeV:
	*value = GET_VALUE_(srcudp.UdpA_Base,
			    valoff,
			    (1<<srcudp.UdpW_PixelLength)-1);
	status = PointStatsBits(&srcudp,min,max,mean,var);
	break;	    
    default:
	return( BadValue );
	}
    
    *stddev = sqrt(*var);

    return( status );
}

/*****************************************************************************
**  PointStatsByte
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine calculates point stats for a byte UDP.
**
**  FORMAL PARAMETERS:
**
**      src	- source data descriptor.
**	value	- returned value of pixel at (x,y).
**	min	- returned minimum pixel value in region.
**	max	- returned maxumum pixel value in region.
**	mean	- returned mean pixel value in region.
**	stddev	- returned standard deviation of pixel values in region.
**	var	- returned variance of pixel values in region.
**
**  FUNCTION VALUE:
**
**      status, returns Success, or error.
**
*****************************************************************************/
static int   PointStatsByte(src,min,max,mean,var)
UdpPtr	     src;	/* source data descriptor			    */
double	    *min;	/* returned minimum pixel value in region	    */
double	    *max;	/* returned maxumum pixel value in region	    */
double	    *mean;	/* returned mean pixel value in region		    */
double	    *var;	/* returned variance of pixel values in region	    */
{
    int		     scan;
    int		     pel;

    unsigned char   *scan_ptr	= src->UdpA_Base + src->UdpL_Pos / 8;
    unsigned char   *pel_ptr	= scan_ptr;
    int		     scnstride	= src->UdpL_ScnStride / 8;
    int		     pxlstride	= src->UdpL_PxlStride / 8;

    unsigned char    value;

    int		     count	= src->UdpL_PxlPerScn * src->UdpL_ScnCnt;
    unsigned char    lclmin	= *pel_ptr;
    unsigned char    lclmax	= *pel_ptr;

    double	     sum	= 0.0;
    double	     sumsqr	= 0.0;
    /*
    **	Do first pass to get min, max, and mean
    */
    for (scan = src->UdpL_Y1;  scan <= src->UdpL_Y2;  scan++)
	{
	pel_ptr = scan_ptr;
        for (pel = src->UdpL_X1;  pel <= src->UdpL_X2;  pel++)
	    {
	    value = *pel_ptr;
	    /*
	    **	Keep track of minimum and maximum values.
	    */
            if (value > lclmax)	lclmax = value;
	    if (value < lclmin)	lclmin = value;
	    /*
	    **	Keep runnning sum of samples and square of samples.
	    */
	    sum += value;
	    sumsqr += value * value;
	    /*
	    **	Update source data pointer
	    */
	    pel_ptr += pxlstride;
	    }
	/*
	**  Update source data pointer
	*/
	scan_ptr += scnstride;
	}
    /*
    **	Return values.
    */
    *min = lclmin;
    *max = lclmax;
    *mean = sum / (double)count;
    *var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);

    return( Success );
}

/*****************************************************************************
**  PointStatsWord
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine calculates point stats for a word UDP.
**
**  FORMAL PARAMETERS:
**
**      src	- source data descriptor.
**	value	- returned value of pixel at (x,y).
**	min	- returned minimum pixel value in region.
**	max	- returned maxumum pixel value in region.
**	mean	- returned mean pixel value in region.
**	stddev	- returned standard deviation of pixel values in region.
**	var	- returned variance of pixel values in region.
**
**  FUNCTION VALUE:
**
**      status, returns Success, or error.
**
*****************************************************************************/
static int   PointStatsWord(src,min,max,mean,var)
UdpPtr	     src;	/* source data descriptor			    */
double	    *min;	/* returned minimum pixel value in region	    */
double	    *max;	/* returned maxumum pixel value in region	    */
double	    *mean;	/* returned mean pixel value in region		    */
double	    *var;	/* returned variance of pixel values in region	    */
{
    int		     scan;
    int		     pel;

    unsigned short  *scan_ptr	= 
	(unsigned short *)(src->UdpA_Base + src->UdpL_Pos / 8);
    unsigned short  *pel_ptr	= scan_ptr;
    int		     scnstride	= src->UdpL_ScnStride / 16;
    int		     pxlstride	= src->UdpL_PxlStride / 16;

    unsigned short   value;
    unsigned short   lclmin	= *pel_ptr;
    unsigned short   lclmax	= *pel_ptr;
    int		     count	= src->UdpL_PxlPerScn * src->UdpL_ScnCnt;

    double	     sum	= 0.0;
    double	     sumsqr	= 0.0;
    /*
    **	Do first pass to get min, max, and mean
    */
    for (scan = src->UdpL_Y1;  scan <= src->UdpL_Y2;  scan++)
        {
	pel_ptr = scan_ptr;
        for (pel = src->UdpL_X1;  pel <= src->UdpL_X2;  pel++)
            {
	    value = *pel_ptr;
	    /*
	    **	Keep track of minimum and maximum values.
	    */
            if (value > lclmax)	lclmax = value;
	    if (value < lclmin)	lclmin = value;
	    /*
	    **	Keep runnning sum.
	    */
	    sum += value;
	    sumsqr += value * value;
	    /*
	    **	Update source data pointer
	    */
	    pel_ptr += pxlstride;
            }
	/*
	**  Update source data pointer
	*/
	scan_ptr += scnstride;
        }
    /*
    **	Return values.
    */
    *min = lclmin;
    *max = lclmax;
    *mean = sum / (double)count;
    *var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);

    return( Success );
}

/*****************************************************************************
**  PointStatsLong
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine calculates point stats for a long UDP.
**
**  FORMAL PARAMETERS:
**
**      src	- source data descriptor.
**	value	- returned value of pixel at (x,y).
**	min	- returned minimum pixel value in region.
**	max	- returned maxumum pixel value in region.
**	mean	- returned mean pixel value in region.
**	stddev	- returned standard deviation of pixel values in region.
**	var	- returned variance of pixel values in region.
**
**  FUNCTION VALUE:
**
**      status, returns Success, or error.
**
*****************************************************************************/
static int   PointStatsLong(src,min,max,mean,var)
UdpPtr	     src;	/* source data descriptor			    */
double	    *min;	/* returned minimum pixel value in region	    */
double	    *max;	/* returned maxumum pixel value in region	    */
double	    *mean;	/* returned mean pixel value in region		    */
double	    *var;	/* returned variance of pixel values in region	    */
{
    int		     scan;
    int		     pel;

    unsigned long   *scan_ptr	= 
	(unsigned long *)(src->UdpA_Base + src->UdpL_Pos / 8);
    unsigned long   *pel_ptr	= scan_ptr;
    int		     scnstride	= src->UdpL_ScnStride / 32;
    int		     pxlstride	= src->UdpL_PxlStride / 32;

    unsigned long    value;
    unsigned long    lclmin	= *pel_ptr;
    unsigned long    lclmax	= *pel_ptr;
    int		     count	= src->UdpL_PxlPerScn * src->UdpL_ScnCnt;

    double	     sum	= 0.0;
    double	     sumsqr	= 0.0;
    /*
    **	Do first pass to get min, max, and mean
    */
    for (scan = src->UdpL_Y1;  scan <= src->UdpL_Y2;  scan++)
        {
	pel_ptr = scan_ptr;
        for (pel = src->UdpL_X1;  pel <= src->UdpL_X2;  pel++)
            {
	    value = *pel_ptr;
	    /*
	    **	Keep track of minimum and maximum values.
	    */
            if (value > lclmax)	lclmax = value;
	    if (value < lclmin)	lclmin = value;
	    /*
	    **	Keep runnning sum.
	    */
	    sum += value;
	    sumsqr += value * value;
	    /*
	    **	Update source data pointer
	    */
	    pel_ptr += pxlstride;
            }
	/*
	**  Update source data pointer
	*/
	scan_ptr += scnstride;
        }
    /*
    **	Return values.
    */
    *min = lclmin;
    *max = lclmax;
    /*
    **	MEAN :
    **	_          N
    **	x = 1/N * sum  x
    **	          i=1   i
    **	    
    */
    *mean = (double)sum / (double)count;
    /*
    **	VARIANCE :
    **   2                  N               N
    **	s  = 1 / (N-1) * ( sum x² - 1/N * (sum x )²)
    **                     i=1             i=1  i
    */
    *var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);

    return( Success );
}

/*****************************************************************************
**  PointStatsFloat
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine calculates point stats for a float UDP.
**
**  FORMAL PARAMETERS:
**
**      src	- source data descriptor.
**	value	- returned value of pixel at (x,y).
**	min	- returned minimum pixel value in region.
**	max	- returned maxumum pixel value in region.
**	mean	- returned mean pixel value in region.
**	stddev	- returned standard deviation of pixel values in region.
**	var	- returned variance of pixel values in region.
**
**  FUNCTION VALUE:
**
**      status, returns Success, or error.
**
*****************************************************************************/
static int   PointStatsFloat(src,min,max,mean,var)
UdpPtr	     src;	/* source data descriptor			    */
double	    *min;	/* returned minimum pixel value in region	    */
double	    *max;	/* returned maxumum pixel value in region	    */
double	    *mean;	/* returned mean pixel value in region		    */
double	    *var;	/* returned variance of pixel values in region	    */
{
    int		     scan;
    int		     pel;

    float	    *scan_ptr	= (float *)(src->UdpA_Base + src->UdpL_Pos / 8);
    float	    *pel_ptr	= scan_ptr;
    int		     scnstride	= src->UdpL_ScnStride / 32;
    int		     pxlstride	= src->UdpL_PxlStride / 32;

    float	     value;
    float	     lclmin	= *pel_ptr;
    float	     lclmax	= *pel_ptr;
    int		     count	= src->UdpL_PxlPerScn * src->UdpL_ScnCnt;

    double	     sum	= 0.0;
    double	     sumsqr	= 0.0;
    /*
    **	Do first pass to get min, max, and mean
    */
    for (scan = src->UdpL_Y1;  scan <= src->UdpL_Y2;  scan++)
        {
	pel_ptr = scan_ptr;
        for (pel = src->UdpL_X1;  pel <= src->UdpL_X2;  pel++)
            {
	    value = *pel_ptr;
	    /*
	    **	Keep track of minimum and maximum values.
	    */
            if (value > lclmax)	lclmax = value;
	    if (value < lclmin)	lclmin = value;
	    /*
	    **	Keep runnning sum.
	    */
	    sum += value;
	    sumsqr += value * value;
	    /*
	    **	Update source data pointer
	    */
	    pel_ptr += pxlstride;
            }
	/*
	**  Update source data pointer
	*/
	scan_ptr += scnstride;
        }
    /*
    **	Return values.
    */
    *min = lclmin;
    *max = lclmax;
    *mean = sum / (double)count;
    *var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);

    return( Success );
}

/*****************************************************************************
**  PointStatsBits
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine calculates point stats for a bits UDP.
**
**  FORMAL PARAMETERS:
**
**      src	- source data descriptor.
**	value	- returned value of pixel at (x,y).
**	min	- returned minimum pixel value in region.
**	max	- returned maxumum pixel value in region.
**	mean	- returned mean pixel value in region.
**	stddev	- returned standard deviation of pixel values in region.
**	var	- returned variance of pixel values in region.
**
**  FUNCTION VALUE:
**
**      status, returns success, or error.
**
*****************************************************************************/
static int   PointStatsBits(src,min,max,mean,var)
UdpPtr	     src;	/* source data descriptor			    */
double	    *min;	/* returned minimum pixel value in region	    */
double	    *max;	/* returned maxumum pixel value in region	    */
double	    *mean;	/* returned mean pixel value in region		    */
double	    *var;	/* returned variance of pixel values in region	    */
{
    int		     scan;
    int		     pel;

    int		     scan_ptr	= src->UdpL_Pos;
    int		     pel_ptr	= scan_ptr;

    unsigned long    value;
    unsigned long    pel_mask	= (1 << src->UdpW_PixelLength) - 1;
    unsigned long    lclmin	= GET_VALUE_(src->UdpA_Base,pel_ptr,pel_mask);
    unsigned long    lclmax	= GET_VALUE_(src->UdpA_Base,pel_ptr,pel_mask);
    int		     count	= src->UdpL_PxlPerScn * src->UdpL_ScnCnt;

    double	     sum	= 0.0;
    double	     sumsqr	= 0.0;
    /*
    **	Do first pass to get min, max, and mean
    */
    for (scan = src->UdpL_Y1;  scan <= src->UdpL_Y2;  scan++)
        {
	pel_ptr = scan_ptr;
        for (pel = src->UdpL_X1;  pel <= src->UdpL_X2;  pel++)
            {
	    value = GET_VALUE_(src->UdpA_Base,pel_ptr,pel_mask);
	    /*
	    **	Keep track of minimum and maximum values.
	    */
            if (value > lclmax)	lclmax = value;
	    if (value < lclmin)	lclmin = value;
	    /*
	    **	Keep runnning sum.
	    */
	    sum += value;
	    sumsqr += value * value;
	    /*
	    **	Update source data pointer
	    */
	    pel_ptr += src->UdpL_PxlStride;
            }
	/*
	**  Update source data pointer
	*/
	scan_ptr += src->UdpL_ScnStride;
        }
    /*
    **	Return values.
    */
    *min = lclmin;
    *max = lclmax;
    *mean = sum / (double)count;
    *var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);

    return( Success );
}

/*****************************************************************************
**  SmiAreaStats
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine fills the destination UDP with the results of the
**	statistics operation specified by the selector calculated over values
**	surrounding each point in the source. The size of the area calculated
**	over is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	roi	    - region of interest or CPP, may be NULL.
**	selector    - operation to perform.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      status, returns Success, or error.
**
*****************************************************************************/
int	SmiStatsArea(src,dst,roi,selector,edge,constrain)
UdpPtr	src;
UdpPtr	dst;
UdpPtr	roi;
int	selector;
int	edge;
int	constrain;
{
    int	status;
    /*	 
    **  Local copies of the UDPs which will describe the intersection of the
    **	ROI, source UDP, and destination UDP.
    */	 
    UdpRec  srcudp;
    UdpRec  dstudp;
    UdpRec  roiudp;

    /*
    **  Allocate the destination image array
    */
    if( dst->UdpA_Base == NULL )
    {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
    }

    /*	 
    **  Set up destination UDP to always be float
    */
    if (dst->UdpB_DType != UdpK_DTypeF)
        {
	dstudp = *dst;

	dstudp.UdpW_PixelLength = sizeof(float) * 8;
	dstudp.UdpB_DType = UdpK_DTypeF;
	dstudp.UdpB_Class = UdpK_ClassA;
	dstudp.UdpL_PxlStride = dstudp.UdpW_PixelLength;
	dstudp.UdpL_ScnStride = dstudp.UdpL_PxlPerScn * dstudp.UdpL_PxlStride;
	dstudp.UdpL_Pos = 0;

	dstudp.UdpL_ArSize = dstudp.UdpL_ScnCnt * dstudp.UdpL_ScnStride;
	dstudp.UdpA_Base = (unsigned char *) DdxMalloc_(dstudp.UdpL_ArSize / 8);
        }
    /*	 
    **  Determine intersection of source, destination, and ROI.
    */	 
    roiudp.UdpA_Base = NULL;
    status = DdxResolveUdp_( src,
			     dst->UdpB_DType == UdpK_DTypeF ? dst : &dstudp,
			     roi,
			    &srcudp,
			    &dstudp,
			    &roiudp );
    if( status != Success ) return( status );
    /*	 
    **  Call routine based on selector and source data type.
    */	 
        /*	 
        **  Perform bounds checking on dtype and selector before entering??
        */	 
    if (roiudp.UdpA_Base == NULL)
	status = (*area_stats_table[srcudp.UdpB_DType][selector])
		    (&srcudp,&dstudp,edge);
    else
	status = (*area_stats_cpp_table[selector][srcudp.UdpB_DType])
		  (&srcudp,&dstudp,&roiudp,edge);
    /*	 
    **  If output UDP not float, do a convert into the required data type.
    */	 
    if (dst->UdpB_DType != UdpK_DTypeF)
        {
	DdxConvert_( &dstudp, dst, constrain );
	DdxFree_( dstudp.UdpA_Base );
        }

    return( status );
}

/*****************************************************************************
**  _AreaErr
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine returns BadValue error for selectors and/or data type out
**	of range errors.
**
**  FORMAL PARAMETERS:
**
**	none
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
**      status, returns BadValue always.
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
static	int _AreaErr()
{
    return( BadValue );
}

/*****************************************************************************
**  _AreaMin*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the minimum of pixel
**	values surrounding each point in the source. The size of the area
**	calculated over is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int	    _AreaMinB(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned char *spelptr;		/* Source pixel pointer		    */
    unsigned char *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned char min;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(char) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(char) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial minimum.
            */	 
	    sscnptr = (unsigned char *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(char) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    min = *(sscnptr);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
                    if (min > *spelptr)
                        min = *spelptr;
		    spelptr += spelstr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local minimum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = min;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int	    _AreaMinW(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned short   *spelptr;		/* Source pixel pointer		    */
    unsigned short   *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned short    min;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(short) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(short) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial minimum.
            */	 
	    sscnptr = (unsigned short *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(short) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    min = *(sscnptr);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
                    if (min > *spelptr)
                        min = *spelptr;
		    spelptr += spelstr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local minimum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = min;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMinL(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned long    *spelptr;		/* Source pixel pointer		    */
    unsigned long    *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long     min;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(long) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(long) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial minimum.
            */	 
	    sscnptr = (unsigned long *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(long) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    min = *(sscnptr);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
                    if (min > *spelptr)
                        min = *spelptr;
		    spelptr += spelstr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local minimum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = min;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMinF(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    float   *spelptr;			/* Source pixel pointer		    */
    float   *sscnptr;			/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    float    min;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(float) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(float) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial minimum.
            */	 
	    sscnptr = (float *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(float) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    min = *(sscnptr);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
                    if (min > *spelptr)
                        min = *spelptr;
		    spelptr += spelstr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local minimum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = min;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMinV(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    int	     speloff;			/* Source pixel bit offset	    */
    int	     sscnoff;			/* Source scanline bit offset	    */
    int	     srcmsk;			/* Source pixel mask		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    float    min;
    int	     value;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride;
    sscnstr = src->UdpL_ScnStride;
    srcmsk  = (1 << src->UdpL_Levels) - 1;
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial minimum.
            */	 
	    sscnoff = src->UdpL_Pos + (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    min = GET_VALUE_(src->UdpA_Base,sscnoff,srcmsk);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		speloff = sscnoff;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    value = GET_VALUE_(src->UdpA_Base,speloff,srcmsk);
                    if (min > value)
                        min = value;
		    speloff += spelstr;
                    }
		sscnoff += sscnstr;
                }
            /*	 
            **  Store the local minimum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = min;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

/*****************************************************************************
**  _AreaMax*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the maximum of pixel
**	values surrounding each point in the source. The size of the area
**	calculated over is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int   _AreaMaxB(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned char    *spelptr;		/* Source pixel pointer		    */
    unsigned char    *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned char   max;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(char) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(char) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial maximum.
            */	 
	    sscnptr = (unsigned char *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(char) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    max = *(sscnptr);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
                    if (max < *spelptr)
                        max = *spelptr;
		    spelptr += spelstr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local maximum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = max;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMaxW(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned short *spelptr;		/* Source pixel pointer		    */
    unsigned short *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned short max;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(short) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(short) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial maximum.
            */	 
	    sscnptr = (unsigned short *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(short) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    max = *(sscnptr);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
                    if (max < *spelptr)
                        max = *spelptr;
		    spelptr += spelstr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local maximum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = max;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMaxL(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned long *spelptr;		/* Source pixel pointer		    */
    unsigned long *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long max;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(long) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(long) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial maximum.
            */	 
	    sscnptr = (unsigned long *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(long) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    max = *(sscnptr);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
                    if (max < *spelptr)
                        max = *spelptr;
		    spelptr += spelstr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local maximum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = max;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMaxF(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    float   *spelptr;			/* Source pixel pointer		    */
    float   *sscnptr;			/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    float    max;

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(float) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(float) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Get the first pixel value in this block, and save that as the
	    **	initial maximum.
            */	 
	    sscnptr = (float *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(float) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    max = *(sscnptr);

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
                    if (max < *spelptr)
                        max = *spelptr;
		    spelptr += spelstr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local maximum in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = max;
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMaxV(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _AreaMean*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the mean of pixel
**	values surrounding each point in the source. The size of the area
**	calculated over is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/

static	int   _AreaMeanB(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned char *spelptr;		/* Source pixel pointer		    */
    unsigned char *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    double   sum;		/* Sum of pixel values in area	    */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(char) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(char) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned char *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(char) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    sum = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
		    sum += *spelptr;
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local mean in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = (double)sum / (double)((escn-bscn+1) * (epxl-bpxl+1));
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMeanW(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned short *spelptr;		/* Source pixel pointer		    */
    unsigned short *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    double   sum;			/* Sum of pixel values in area	    */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(short) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(short) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned short *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(short) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    sum = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
		    sum += *spelptr;
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local mean in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = sum / (double)((escn-bscn+1) * (epxl-bpxl+1));
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMeanL(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned long *spelptr;		/* Source pixel pointer		    */
    unsigned long *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    double   sum;			/* Sum of pixel values in area	    */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(long) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(long) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned long *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(long) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    sum = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
		    sum += *spelptr;
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local mean in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = sum / (double)((escn-bscn+1) * (epxl-bpxl+1));
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMeanF(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    float   *spelptr;			/* Source pixel pointer		    */
    float   *sscnptr;			/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    double   sum;			/* Sum of pixel values in area	    */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(float) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(float) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (float *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(float) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    sum = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
		    sum += *spelptr;
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local mean in the destination UDP buffer, and
	    **	update destination pointer.
            */
	    *dpelptr = sum / (double)((escn-bscn+1) * (epxl-bpxl+1));
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaMeanV(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _AreaStdDev*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the standard deviation of
**	pixel values surrounding each point in the source. The size of the area
**	calculated over is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int   _AreaStdDevB(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned char *spelptr;		/* Source pixel pointer		    */
    unsigned char *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long   count;		/* Count of pixels in local area    */
    double   sum;			/* Sum of pixel values in area	    */
    double   sumsqr;			/* Sum of pixel values in area	    */

    double  var;			/* Variance */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(char) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(char) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned char *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(char) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    count = (escn - bscn + 1) * (epxl - bpxl + 1);
	    sum = 0.0;
	    sumsqr = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    sum += *spelptr;
		    sumsqr += *spelptr * *spelptr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local standard deviation in the destination UDP 
	    **	buffer, and update destination pointer.
            */
	    var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);
	    *dpelptr = sqrt(var);
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaStdDevW(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned short *spelptr;		/* Source pixel pointer		    */
    unsigned short *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long   count;		/* Count of pixels in local area    */
    double   sum;			/* Sum of pixel values in area	    */
    unsigned long   sumsqr;		/* Sum of pixel values in area	    */

    double  var;			/* Variance */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(short) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(short) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned short *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(short) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    count = (escn - bscn + 1) * (epxl - bpxl + 1);
	    sum = 0.0;
	    sumsqr = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    sum += *spelptr;
		    sumsqr += *spelptr * *spelptr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local standard deviation in the destination UDP 
	    **	buffer, and update destination pointer.
            */
	    var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);
	    *dpelptr = sqrt(var);
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaStdDevL(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned long *spelptr;		/* Source pixel pointer		    */
    unsigned long *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long   count;		/* Count of pixels in local area    */
    double   sum;			/* Sum of pixel values in area	    */
    double   sumsqr;			/* Sum of pixel values in area	    */

    double  var;			/* Variance */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(long) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(long) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned long *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(long) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    count = (escn - bscn + 1) * (epxl - bpxl + 1);
	    sum = 0.0;
	    sumsqr = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    sum += *spelptr;
		    sumsqr += *spelptr * *spelptr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local standard deviation in the destination UDP 
	    **	buffer, and update destination pointer.
            */
	    var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);
	    *dpelptr = sqrt(var);
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaStdDevF(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    float   *spelptr;			/* Source pixel pointer		    */
    float   *sscnptr;			/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long   count;		/* Count of pixels in local area    */
    double   sum;			/* Sum of pixel values in area	    */
    double   sumsqr;			/* Sum of pixel values in area	    */

    double  var;			/* Variance */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(float) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(float) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (float *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(float) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    count = (escn - bscn + 1) * (epxl - bpxl + 1);
	    sum = 0.0;
	    sumsqr = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    sum += *spelptr;
		    sumsqr += *spelptr * *spelptr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local standard deviation in the destination UDP 
	    **	buffer, and update destination pointer.
            */
	    var = (sumsqr-((sum*sum)/(double)count))/(double)(count-1);
	    *dpelptr = sqrt(var);
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaStdDevV(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _AreaVar*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the variance of pixel
**	values surrounding each point in the source. The size of the area
**	calculated over is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int   _AreaVarB(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned char *spelptr;		/* Source pixel pointer		    */
    unsigned char *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long   count;		/* Count of pixels in local area    */
    double   sum;			/* Sum of pixel values in area	    */
    double  sumsqr;			/* Sum of pixel values in area	    */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(char) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(char) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned char *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(char) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    count = (escn - bscn + 1) * (epxl - bpxl + 1);
	    sum = 0.0;
	    sumsqr = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    sum += *spelptr;
		    sumsqr += *spelptr * *spelptr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local standard deviation in the destination UDP 
	    **	buffer, and update destination pointer.
            */
	    *dpelptr = 
		(sumsqr-((sum*sum)/(float)count))/(float)(count-1);
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaVarW(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned short *spelptr;		/* Source pixel pointer		    */
    unsigned short *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long   count;		/* Count of pixels in local area    */
    double   sum;			/* Sum of pixel values in area	    */
    double   sumsqr;			/* Sum of pixel values in area	    */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(short) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(short) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned short *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(short) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    count = (escn - bscn + 1) * (epxl - bpxl + 1);
	    sum = 0.0;
	    sumsqr = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    sum += *spelptr;
		    sumsqr += *spelptr * *spelptr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local standard deviation in the destination UDP 
	    **	buffer, and update destination pointer.
            */
	    *dpelptr = 
		(sumsqr-((sum*sum)/(float)count))/(float)(count-1);
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaVarL(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    unsigned long *spelptr;		/* Source pixel pointer		    */
    unsigned long *sscnptr;		/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long   count;		/* Count of pixels in local area    */
    double   sum;			/* Sum of pixel values in area	    */
    double   sumsqr;			/* Sum of pixel values in area	    */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(long) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(long) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (unsigned long *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(long) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    count = (escn - bscn + 1) * (epxl - bpxl + 1);
	    sum = 0.0;
	    sumsqr = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    sum += *spelptr;
		    sumsqr += *spelptr * *spelptr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local standard deviation in the destination UDP 
	    **	buffer, and update destination pointer.
            */
	    *dpelptr = 
		(sumsqr-((sum*sum)/(float)count))/(float)(count-1);
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaVarF(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    int	     dpel;			/* Destination pixel index	    */
    int	     dscn;			/* Destination scanline index	    */
    int	     dpelstr;			/* Destination pixel stride	    */
    int	     dscnstr;			/* Destination scanline stride	    */
    float   *dpelptr;			/* Destination pixel pointer	    */
    float   *dscnptr;			/* Destination scanline pointer	    */

    int	     spel;			/* Source pixel index		    */
    int	     sscn;			/* Source scanline index	    */
    int	     spelstr;			/* Source pixel stride		    */
    int	     sscnstr;			/* Source scanline stride	    */
    float   *spelptr;			/* Source pixel pointer		    */
    float   *sscnptr;			/* Source pixel pointer		    */

    int	     bscn;			/* The beginning and ending pixel   */
    int	     escn;			/* index for each local area	    */
    int	     bpxl;			/* operation.			    */
    int	     epxl;

    unsigned long   count;		/* Count of pixels in local area    */
    double   sum;			/* Sum of pixel values in area	    */
    double   sumsqr;			/* Sum of pixel values in area	    */

    dpelstr = dst->UdpL_PxlStride / (sizeof(float) * 8);
    dscnstr = dst->UdpL_ScnStride / (sizeof(float) * 8);
    dscnptr = (float *)(dst->UdpA_Base + dst->UdpL_Pos / 8);
    
    spelstr = src->UdpL_PxlStride / (sizeof(float) * 8);
    sscnstr = src->UdpL_ScnStride / (sizeof(float) * 8);
    
    for (dscn = dst->UdpL_Y1;  dscn <= dst->UdpL_Y2;  dscn++)
        {
	dpelptr = dscnptr;
        for (dpel = dst->UdpL_X1;  dpel <= dst->UdpL_X2;  dpel++)
            {
            /*	 
            **  Calculate source scanlines required to do this computation such
	    **	that we don't extend beyond the boundary of the source.
            */	 
	    bscn = dscn - edge / 2 > src->UdpL_Y1 ?
			dscn - edge / 2 : dst->UdpL_Y1;

	    escn = dscn - edge / 2 + edge < src->UdpL_Y2 ?
			dscn - edge / 2  + edge : dst->UdpL_Y2;

	    bpxl = dpel - edge / 2 > src->UdpL_X1 ?
			dpel - edge / 2 : dst->UdpL_X1;

	    epxl = dpel - edge / 2 + edge < src->UdpL_X2 ?
			dpel - edge / 2  + edge : dst->UdpL_X2;
            /*	 
            **  Initialize scanline pointer and sum.
            */	 
	    sscnptr = (float *)src->UdpA_Base +
		      src->UdpL_Pos / (sizeof(float) * 8) + 
		      (bscn - src->UdpL_Y1) * sscnstr +
		      (bpxl - src->UdpL_X1) * spelstr;
	    count = (escn - bscn + 1) * (epxl - bpxl + 1);
	    sum = 0.0;
	    sumsqr = 0.0;

            for (sscn = bscn;  sscn <= escn;  sscn++)
                {
		spelptr = sscnptr;
                for (spel = bpxl;  spel <= epxl;  spel++)
                    {
		    sum += *spelptr;
		    sumsqr += *spelptr * *spelptr;
                    }
		sscnptr += sscnstr;
                }
            /*	 
            **  Store the local standard deviation in the destination UDP 
	    **	buffer, and update destination pointer.
            */
	    *dpelptr = 
		(sumsqr-((sum*sum)/(float)count))/(float)(count-1);
	    dpelptr += dpelstr;
            }
	dscnptr += dscnstr;
        }

    return( Success );
}

static	int   _AreaVarV(src,dst,edge)
UdpPtr		     src;
UdpPtr		     dst;
int		     edge;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _AreaCppMin*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the minimum of pixel
**	values surrounding each point in the source which has a bit set in the
**	corresponding position in the CPP. The size of the area calculated over
**	is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	cpp	    - CPP data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int   _AreaCppMinB(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMinW(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMinL(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMinF(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMinV(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _AreaCppMax*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the maximum of pixel
**	values surrounding each point in the source which has a bit set in the
**	corresponding position in the CPP. The size of the area calculated over
**	is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	cpp	    - CPP data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int   _AreaCppMaxB(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMaxW(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMaxL(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMaxF(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMaxV(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _AreaCppMean*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the mean of pixel values
**	surrounding each point in the source which has a bit set in the
**	corresponding position in the CPP. The size of the area calculated over
**	is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	cpp	    - CPP data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int   _AreaCppMeanB(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMeanW(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMeanL(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMeanF(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppMeanV(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _AreaCppStdDev*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the standard deviation of
**	pixel values surrounding each point in the source which has a bit set
**	in the corresponding position in the CPP. The size of the area
**	calculated over is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	cpp	    - CPP data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int   _AreaCppStdDevB(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppStdDevW(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppStdDevL(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppStdDevF(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int   _AreaCppStdDevV(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _AreaCppVar*
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines fill the destination UDP with the variance of pixel
**	values surrounding each point in the source which has a bit set in the
**	corresponding position in the CPP. The size of the area calculated over
**	is determined by the edge parameter.
**
**  FORMAL PARAMETERS:
**
**      src	    - source data descriptor.
**	dst	    - destination data descriptor.
**	cpp	    - CPP data descriptor.
**	edge	    - size of source area to calculate over.
**
**  FUNCTION VALUE:
**
**      success
**
*****************************************************************************/
static	int	    _AreaCppVarB(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int	    _AreaCppVarW(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int	    _AreaCppVarL(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int	    _AreaCppVarF(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}

static	int	    _AreaCppVarV(src,dst,cpp,edge)
UdpPtr		     src;
UdpPtr		     dst;
UdpPtr		     cpp;
int		     edge;
{
    return( BadImplementation );
}
