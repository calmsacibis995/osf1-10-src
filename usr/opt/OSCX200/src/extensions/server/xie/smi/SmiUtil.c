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
**      This module contains various utility routines.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**
**  CREATION DATE:
**
**      Sat Mar 31 13:45:36 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Definition to let include files know who's calling.
*/
#define _SmiUtil

/*
**  Include files
*/
#include <stdio.h>
#include <XieDdx.h>                     /* XIE device dependant definitions */
#include <XieUdpDef.h>			/* UDP definitions		    */

/*
**  Table of contents
*/
int	SmiCmpPreferred();
void	SmiCopyBits();
int	SmiResolveUdp();
int	SmiUsingCL();

/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/
#define	    Success	0
#define	    NoIntersect	1
#define     FALSE       0
#define     TRUE        1
/*
**  External References
*/

/*
**	Local Storage
*/

/****************************************************************************
**  SmiCmpPreferred
**
**  FUNCTIONAL DESCRIPTION:
**
**     Answers the question: Does this DDX prefer to keep permanent data 
**     of the specified compression type in compressed form?
**
**     Compressed form is preferred when the storage and conversion
**     overhead of uncompressed data exceeds the overhead of decompressing
**     the data each time it must be used.
**
**
**  FORMAL PARAMETERS:
**	type  - compression type
**
**  FUNCTION VALUE:
**	TRUE or FALSE
**
*****************************************************************************/
int  SmiCmpPreferred( type )
    int type;
{
    switch (type)
	{

         case XieK_G31D :
         case XieK_G32D :
         case XieK_G42D :
         case XieK_DCT :
	     return( FALSE );

         default:
	     return( FALSE );
	  
	  }
}

/*****************************************************************************
**  SmiCopyBits
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine copies (in a semi-optimized way) a string of bits from
**      one arbitrary location to another.
**
**  FORMAL PARAMETERS:
**
**      in	- input buffer pointer
**      inofs   - bit offset into input buffer
**	out	- output buffer pointer
**      outofs  - bit offset into output buffer
**      count   - number of bits to copy
**
*****************************************************************************/
void	    SmiCopyBits(in, inofs, out, outofs, count)
    char *in;
    long inofs;
    char *out;
    long outofs;
    long count;
{
    char *src, *dst;
    int in_align_bits;
    int out_align_bits;
    unsigned int out_align_mask;

    int remainder;

    char *limit;
    unsigned char temp;

    /* Compute amount by which bit streams are offset from being
     * byte aligned.
     */

    in_align_bits = inofs & 7;
    out_align_bits = outofs & 7;

    if (in_align_bits == 0) {

	if (out_align_bits == 0){

	    /* This is the easy case - both sides are byte aligned */
	    remainder = count & 7;

	    in+=(inofs>>3);
	    inofs = 0;
	    
	    out+=(outofs>>3);
	    outofs = 0;

	    limit = in + (count >> 3);

	    while (in < limit)
		*out++ = *in++;
	} else {

	    /* The input is byte aligned. Fetch bytes, and store
	     * unaligned 8 bits chunks. */
	    remainder = count & 7;

	    in+=(inofs>>3);
	    inofs = 0;
	    
	    limit = in + (count >> 3);

	    while (in < limit ) {
		PUT_VALUE_(out, outofs, *in++, 0xff);
		outofs+=8;
	    }
		
	}

    } else {
	/* Input is not byte aligned */

	if (out_align_bits != 0) {

	    /* Neither stream is byte aligned.  Align the output stream */
	    out_align_mask = (1<<(8 - out_align_bits)) -1;
	    temp = GET_VALUE_(in, inofs, out_align_mask);
	    PUT_VALUE_(out, outofs, temp, out_align_mask);

	    inofs += (8-out_align_bits);
	    outofs += (8-out_align_bits);
	    count -= (8-out_align_bits);
	}

	/* The output is (now) byte aligned.  Fetch 8 bits, store bytes */
	remainder = count & 7;

	out += (outofs>>3);
	outofs = 0;

	limit = out + (count >> 3);

	while (out < limit ) {
	    temp = GET_VALUE_( in, inofs, 0xff);
	    *out++ = temp;
	    inofs+=8;
	}
    }

    /* Move the leftover (not multiple of a byte) bits.
     * At this point in:inofs have been advanced to point to
     * the source of the remaining bits, and out:outofs  have been
     * been advanced to point to the destination */
    if (remainder > 0) {
	temp = GET_VALUE_(in, inofs, ((1<<remainder)-1));
	PUT_VALUE_(out, outofs, temp, ((1<<remainder)-1));
    }
}				    /* end SmiCopyBits */

/*****************************************************************************
**  SmiResolveUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine accepts a source, destination, and ROI UDP as input and
**	sets a second set of UDPs to include only the minimum common area
**	described by the input UDPs.
**
**  FORMAL PARAMETERS:
**
**	isudp - input source UDP pointer
**	idudp - input destination UDP pointer
**	irudp - input ROI UDP pointer, may be NULL if no ROI
**	osudp - output source UDP pointer
**	odudp - output destination UDP pointer
**	orudp - output ROI UDP pointer, if irudp == NULL, this is not used.
**
**  FUNCTION VALUE:
**
**      status, Zero,		success.
**		NoIntersect,	ROIs don't intersect.
**
*****************************************************************************/
int	SmiResolveUdp(isudp,idudp,irudp,osudp,odudp,orudp)
UdpPtr	isudp;
UdpPtr	idudp;
UdpPtr	irudp;
UdpPtr	osudp;
UdpPtr	odudp;
UdpPtr	orudp;
{
    int	X1,Y1,X2,Y2;
    /*
    **	Get minimum common region between source and destination.
    */
    X1 = isudp->UdpL_X1 > idudp->UdpL_X1 ? isudp->UdpL_X1 : idudp->UdpL_X1;
    Y1 = isudp->UdpL_Y1 > idudp->UdpL_Y1 ? isudp->UdpL_Y1 : idudp->UdpL_Y1;
    X2 = isudp->UdpL_X2 < idudp->UdpL_X2 ? isudp->UdpL_X2 : idudp->UdpL_X2;
    Y2 = isudp->UdpL_Y2 < idudp->UdpL_Y2 ? isudp->UdpL_Y2 : idudp->UdpL_Y2;
    /*
    **	If a ROI was also specified, intersect that with what we have thus
    **	far.
    */
    if (irudp != NULL)
    {
	X1 = X1 > irudp->UdpL_X1 ? X1 : irudp->UdpL_X1;
	Y1 = Y1 > irudp->UdpL_Y1 ? Y1 : irudp->UdpL_Y1;
	X2 = X2 < irudp->UdpL_X2 ? X2 : irudp->UdpL_X2;
	Y2 = Y2 < irudp->UdpL_Y2 ? Y2 : irudp->UdpL_Y2;
    }
    /*
    **	Check to make sure these UDPs do actually have an intersection.
    */
    if (X1 > X2 || Y1 > Y2)
    {
        return NoIntersect;
    }
    /*
    **	Now, adjust the output source UDP to describe only the bounding region.
    */
    *osudp = *isudp;

    osudp->UdpL_Pos +=	(Y1 - osudp->UdpL_Y1) * osudp->UdpL_ScnStride +
			(X1 - osudp->UdpL_X1) * osudp->UdpL_PxlStride;
    osudp->UdpL_PxlPerScn = X2 - X1 + 1;
    osudp->UdpL_ScnCnt = Y2 - Y2 + 1;
    osudp->UdpL_X1 = X1;
    osudp->UdpL_Y1 = Y1;
    osudp->UdpL_X2 = X2;
    osudp->UdpL_Y2 = Y2;
    /*
    **	Same for destination...
    */
    *odudp = *idudp;

    odudp->UdpL_Pos +=	(Y1 - odudp->UdpL_Y1) * odudp->UdpL_ScnStride +
			(X1 - odudp->UdpL_X1) * odudp->UdpL_PxlStride;
    odudp->UdpL_PxlPerScn = X2 - X1 + 1;
    odudp->UdpL_ScnCnt = Y2 - Y2 + 1;
    odudp->UdpL_X1 = X1;
    odudp->UdpL_Y1 = Y1;
    odudp->UdpL_X2 = X2;
    odudp->UdpL_Y2 = Y2;
    /*
    **	If ROI specified, then that too...
    */
    if (irudp != NULL)
    {
	*orudp = *irudp;

	orudp->UdpL_Pos +=  (Y1 - orudp->UdpL_Y1) * orudp->UdpL_ScnStride +
			    (X1 - orudp->UdpL_X1) * orudp->UdpL_PxlStride;
	orudp->UdpL_PxlPerScn = X2 - X1 + 1;
	orudp->UdpL_ScnCnt = Y2 - Y2 + 1;
	orudp->UdpL_X1 = X1;
	orudp->UdpL_Y1 = Y1;
	orudp->UdpL_X2 = X2;
	orudp->UdpL_Y2 = Y2;
    }

    return Success;
}

/****************************************************************************
**  SmiUsingCL
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function is the SMI version of the changelist support check
**      used in XieTransport.c to determine if the DDX does changelists.
**      The SMI DDX does not, so this function just returns false.
**
**  FORMAL PARAMETERS:
**
**      None.
**
**  FUNCTION VALUE:
**
**      FALSE  ==  no change list support
**
*****************************************************************************/
int     SmiUsingCL()
{
    return( FALSE );
}
