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
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	Bit copying routines.
**
**  ENVIRONMENT:
**
**	Ultrix V3.0
**
**  AUTHOR(S):
**
**	Written in C by Brian M. Stevens  27-Sep-1988
**
**	Clean-up an put in library by Michael D. O'Connor  5-Dec-1988
**
**  CREATION DATE:
**
**	5-Dec-1988
**
************************************************************************/

/*
** Table of Contents
*/
#ifdef NODAS_PROTO
int _IpsMovv5();
#endif

/*
**  Include files
*/
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif
#include <ips_bitmasks.h>

/*
**  MACRO definitions
*/

/*
** Global Symbols
*/

/*
** Equated Symbols
*/

/*
** External References
*/

/************************************************************************
**
**  _IpsMovv5_LONG - move bit string of arbitrary length
**
**  FUNCTIONAL DESCRIPTION:
**
**	Move a bit string of any length to any destination
**
**  FORMAL PARAMETERS:
**
    long		size;		* number of bits to copy	     *
    long		src_addr;	* base address of source buf         *
    long		src_pos;	* starting offset in source buf      *
    long		dst_addr;	* base address of destination buf    *
    long		dst_pos;	* starting offset in destination buf *
**
**  IMPLICIT INPUTS:
**
**
**
**  IMPLICIT OUTPUTS:
**
**
**
**  FUNCTION VALUE:
**
**
**
**  SIGNAL CODES:
**
**      None
**
**  SIDE EFFECTS:
**
**      None
**
************************************************************************/

int _IpsMovv5(size, src_pos, src_addr, dst_pos, dst_addr)
    long		size;		/* number of bits to copy	      */
    unsigned char	*src_addr;	/* base address of source buf         */
    long		src_pos;	/* starting offset in source buf      */
    unsigned char	*dst_addr;	/* base address of destination buf    */
    long		dst_pos;	/* starting offset in destination buf */
{
    unsigned char	*src;		/* current source buf word            */
    unsigned char 	*dst;		/* current destination buf word       */
    int			nbits;		/* number of bits to copy this time   */
    unsigned int	mask;		/* for forming destination word       */

    /*
     * obtain pointers to the source and destination buffers
     */

    dst = (unsigned char *) dst_addr + dst_pos / BYTE_SIZE;
    src = (unsigned char *) src_addr + src_pos / BYTE_SIZE;
    src_pos = src_pos % BYTE_SIZE;
    dst_pos = dst_pos % BYTE_SIZE;

    /*
     * copy the largest portion of the longword possible until
     * the entire bit string is copied
     */

    while (size > 0)
    {
	/*
	 * determine the maximum number of bits that can be copied this time
	 */

        nbits = BYTE_SIZE - (src_pos <= dst_pos ? dst_pos : src_pos);
        nbits = (nbits <= size ? nbits : size);

	/*
	 * form the mask
	 */

	mask = hi_mask[dst_pos] | lo_mask[dst_pos + nbits];

	/*
	 * check the alignment of the source and destination buffers
	 */

        if (src_pos >= dst_pos)
        {
	    /*
	     * must shift the source buffer right to align it with the
	     * destination buffer
	     */

	    *dst = (*dst & mask) | ((*src >> src_pos - dst_pos) & ~mask);
	    dst += (dst_pos + nbits) / BYTE_SIZE;
	    dst_pos = (dst_pos + nbits) % BYTE_SIZE;
	    src_pos = 0;
	    src++;
        }
        else
        {
	    /*
	     * must shift the source buffer left to align it with the
	     * destination buffer
	     */

	    *dst = (*dst & mask) | ((*src << dst_pos - src_pos) & ~mask);
	    src_pos += nbits;
	    dst_pos = 0;
	    dst++;
        }

	/*
	 * decrease the amount of bits left to be copied
	 */

        size -= nbits;
    }
}
