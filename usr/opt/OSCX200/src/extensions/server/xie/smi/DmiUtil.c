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

/*******************************************************************************
**  Copyright (c) Digital Equipment Corporation, 1991 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**
**  Derived from work bearing the following copyright and permissions:
**
**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**  
**                          All Rights Reserved
**  
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**  
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**  
*******************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      X Imaging Extension
**      Digital Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains various utility routines.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**
**  CREATION DATE:
**
**      Tue Mar  5 17:47:44 1991
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Definition to let include files know who's calling.
*/
#define _DmiUtil

/*
**  Include files
*/
#include <stdio.h>

#if defined(VMS) && !defined(VXT)
#include <lmfdef.h>
#include <descrip.h>
#include <ssdef.h>
#endif

#include <XieDdx.h>                     /* XIE device dependant definitions */
#include <XieUdpDef.h>			/* UDP definitions		    */

/*
**  Table of contents
*/
int	    DmiCmpPreferred();
void	    DmiCopyBits();
int	    DmiCheckLicense();
int	    DmiResolveUdp();
int	    DmiUsingCL();



/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/
#define	    Success	0
#define	    NoIntersect	1
#define	    FALSE	0
#define	    TRUE	1
/*
**  External References
*/

/*
**	Local Storage
*/

/****************************************************************************
**  DmiCmpPreferred
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
int  DmiCmpPreferred( type )
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
**  DmiCopyBits
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
**      none
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
void	    DmiCopyBits(in, inofs, out, outofs, count)
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
}				    /* end DmiCopyBits */

/****************************************************************************
**  
**  DmiCheckLicense
**
**  FUNCTIONAL DESCRIPTION:
**
**	Verify that this system has a properly registered license for
**      the Xie server extension.
**  
**      If the license check fails, log an error message in the
**      server error log.
**
**  FORMAL PARAMETERS:
**  
**	None.
**
**  IMPLICIT INPUTS:
**
**	None.
**  
**  IMPLICIT OUTPUTS:
**
**	None.
**  
**  FUNCTION VALUE:
**  
**	0     = A valid license was found.
**	non 0 = Xie is not licensed on this system.
**	
**  SIGNAL CODES:
**
**	None.
**
**  SIDE EFFECTS:
**	
**	None.
**  
*****************************************************************************/
int
DmiCheckLicense(name, producer, reldate, major, minor)
char *name;				/* LMF product name */
char *producer;				/* LMF producer name */
char *reldate;				/* LMF produce release date */
int major;				/* Product major version */
int minor;				/* Product minor version */
{

#if defined(VMS) && !defined(VXT)
    long	status;
    long	lmf_flag = 1;
    long	bindate[2];		/* Quadword for binary time */
    struct {
	unsigned short minor;		/* minor version */
	unsigned short major;		/* major version */
    } version;

    struct {
					/* Canonical VMS item list */
	unsigned short length;		/* Length */
	unsigned short code;		/* Item code */
	unsigned char *address;		/* Buffer address */
	unsigned char *ret_address;	/* Return address */
    } itemlist[] = {
    			{sizeof(long) * 2, LMF$_PROD_DATE, bindate, 0},
		        {sizeof(long), LMF$_PROD_VERSION, &version, 0},
		        {0, 0, 0, 0}
		    };

    $DESCRIPTOR(name_dsc, " ");
    $DESCRIPTOR(producer_dsc, " ");
    $DESCRIPTOR(reldate_dsc, " ");

    long SYS$BINTIM();
    long SYS$LOOKUP_LICENSE();

    version.major = major;
    version.minor = minor;

    name_dsc.dsc$w_length = strlen(name);
    name_dsc.dsc$a_pointer = name;

    producer_dsc.dsc$w_length = strlen(producer);
    producer_dsc.dsc$a_pointer = producer;

    reldate_dsc.dsc$w_length = strlen(reldate);
    reldate_dsc.dsc$a_pointer = reldate;

    /* Convert the product data to binary format */
    status = SYS$BINTIM(&reldate_dsc, bindate);
    if (status != SS$_NORMAL)
	return( 1 );

    /* Oh license gods, we beseech thee, find us worthy of running
     * this humble program. */
    status = SYS$LOOKUP_LICENSE(&name_dsc, itemlist, &producer_dsc,
				&lmf_flag, 0, 0);
    if (status == SS$_NORMAL)
	return( 0 );
    else {
	ErrorF("XieExtensionInit: license for this product is not active\n");
	return( 1 );
    }
	
    
    return( (status == SS$_NORMAL) ? 0 : 1 );
				
#else
    return( 0 );			/* Only check on VMS for now. */
#endif
}

/*****************************************************************************
**  DmiResolveUdp
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
**      status, Zero,		success.
**		NoIntersect,	ROIs don't intersect.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
int	DmiResolveUdp(isudp,idudp,irudp,osudp,odudp,orudp)
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
**  DmiUsingCL
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function is the DMI version of the changelist support check
**      used in XieTransport.c to determine if the DDX does changelists.
**      The DMI DDX does.
**
**  FORMAL PARAMETERS:
**
**      None.
**
**  FUNCTION VALUE:
**
**      FALSE  ==  no changelist support
**	TRUE   ==  changelists done here
**
*****************************************************************************/
int     DmiUsingCL()
{
    /* 
    ** This is a vile, ugly hack to allow the DIX to tell if the underlying
    ** DDX includes changelist support.  This avoids having to rebuild the
    ** DIX routine when using different versions of the DDX code.
    */
    static UdpRec udp = {0, UdpK_DTypeCL, UdpK_ClassCL};

    if (DdxGetTyp_(&udp) == XieK_DATA_TYPE_COUNT)
	return FALSE;
    else
	return TRUE;
}
