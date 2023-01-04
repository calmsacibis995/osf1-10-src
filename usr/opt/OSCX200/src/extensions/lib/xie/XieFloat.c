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

/******************************************************************************
**++
**  FACILITY:
**
**      X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module provides a machine independant mechanism for transporting
**	floating point numbers using the X11 Image Extension protocol.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	Robert NC Shelley
**
**
**  CREATION DATE:     25-Aug-1989
**
*******************************************************************************/

/*
**  INCLUDE FILES
**/
#ifndef VXT
#include <math.h>
#else
#include <vxtmath.h>
#endif
#include <XieFloat.h>			/* XIE device dependant definitions */

/*
**  Table of Contents
*/
void     MiEncodeDouble();
double   MiDecodeDouble();



/*
**  Macro definitions
*/
    /*
    **	To convert the mantissa of a 'double' (always < 1.0) to/from the
    **	'XieFloatRec.mantissa' we multiply/divide by this factor
    **	( 2^n, where n is the size in bits of 'XieFloatRec.mantissa' less 1).
    **	On the wire the mantissa is shifted left one place for compatibililty
    **	with previous protocol versions.
    */
#define Convert_Mantissa_(xiefloatptr) \
			    pow(2.0, 8.0*sizeof(xiefloatptr->mantissa)-1.0)

/*
**  External routines
*/

/*
**  Equated symbols
*/

/*
**  External symbol definitions.
*/

/*****************************************************************************
**  MiEncodeDouble
**
**  FUNCTIONAL DESCRIPTION:
**
**      Encode a double value as XieFloat format.
**
**  FORMAL PARAMETERS:
**
**      ptr	- pointer to XieFloatRec
**	value	- double value to be encoded
**
*****************************************************************************/
void MiEncodeDouble( ptr, value )
 XieFloat   ptr;
 double	    value;
{
    double mantissa;
    int	   exponent;

    /*
    **  Separate out the mantissa and exponent.
    */
    mantissa = frexp(value, &exponent);

    /*
    **  Encode the mantissa in XieFloat format.
    */
    ptr->m_sign   = (unsigned char) (mantissa >= 0.0);
    ptr->mantissa = (unsigned long) (Convert_Mantissa_(ptr)
				  * (ptr->m_sign ? mantissa : -mantissa) + 0.5);

    /*
    **  Check/correct for overflow on round-up.
    */
    if( (int)(ptr->mantissa) < 0 )
	{
	exponent++;
	ptr->mantissa = (unsigned long) Convert_Mantissa_(ptr);
	}
    else
	ptr->mantissa <<= 1;

    /*
    **  Encode the exponent in XieFloat format.
    */
    ptr->e_sign   = (unsigned char)  (exponent >= 0);
    ptr->exponent = (unsigned short) (ptr->e_sign ? exponent : -exponent);
}

/*****************************************************************************
**  MiDecodeDouble
**
**  FUNCTIONAL DESCRIPTION:
**
**      Decode a double value from XieFloat format.
**
**  FORMAL PARAMETERS:
**
**      ptr	- pointer to XieFloatRec
**
**  FUNCTION VALUE:
**
**	decoded double value
**
*****************************************************************************/
double MiDecodeDouble( ptr )
 XieFloat   ptr;
{
    /*
    **  Reassemble the XieFloatRec into a double value.
    */
    return( pow( 2.0, (ptr->e_sign ? 1.0 : -1.0) * ptr->exponent )
		    * (ptr->m_sign ? 1.0 : -1.0)
		    * (ptr->mantissa >> 1) /  Convert_Mantissa_(ptr) );
}
