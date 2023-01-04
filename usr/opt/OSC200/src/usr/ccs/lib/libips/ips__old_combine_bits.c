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
**	This module replaces the _ImgCombineBits.MAR module used in VMS.
**
**      _ImgCombineBits replaces the bit string described by LEN/DSTADR/DSTPOS
**      with the result of applying the combination rule COMBO to SRC,DST,and
**      MASK data.
**
**  ENVIRONMENT:
**
**	Ultrix V3.0  
**
**  AUTHOR(S):
**
**	Written for Ultrix by Brian M. Stevens 29-Sep-1988
**	Clean-up and put in library by Michael D. O'Connor 29-Nov-1988
**	Modified for V3.0  by John Poltrack 28-Sep-1989
**	
************************************************************************/

/*
** Table of Contents
*/
#ifdef NODAS_PROTO
int _IpsCombineBits();
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
**
**  FUNCTIONAL DESCRIPTION:
**
**      _IpsCombineBits replaces the bit string described by LEN/DSTADR/DSTPOS
**      with the result of applying the combination rule COMBO to SRC,DST,and
**      MASK data.
**
**  FORMAL PARAMETERS:
**
    long		size;		* number of bits to combine	     *
    long		src_addr;	* base address of source buf         *
    long		src_pos;	* starting offset in source buf      *
    long		dst_addr;	* base address of destination buf    *
    long		dst_pos;	* starting offset in destination buf *
    long		mask;		* source buffer mask bits            *
    long		rule;		* combination rule                   *
**
**  IMPLICIT INPUTS:
**
**	None
**
**  IMPLICIT OUTPUTS:
**
**	None
**
**  FUNCTION VALUE:
**
**	None
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

int _IpsCombineBits(size, src_pos, src_addr, dst_pos, dst_addr, mask, rule)
    long		size;		/* number of bits to combine	      */
    long		src_pos;	/* starting offset in source buf      */
    unsigned char	*src_addr;	/* base address of source buf         */
    long		dst_pos;	/* starting offset in destination buf */
    unsigned char	*dst_addr;	/* base address of destination buf    */
    long		mask;		/* source buffer mask bits            */
    long		rule;		/* combination rule                   */
{
    unsigned int	*src;		/* current source buf word            */
    unsigned int	*dst;		/* current destination buf word       */
    unsigned int	*dst2;		/* saved destination buf word         */
    int			nbits;		/* number of bits to copy this time   */
    unsigned int	dmask;		/* for forming destination word       */
    unsigned int	src_word;	/* the masked aligned source word     */

    /*
     * obtain pointers to the source and destination buffers
     */

    dst = (unsigned int *) dst_addr + dst_pos / LONG_SIZE;
    src = (unsigned int *) src_addr + src_pos / LONG_SIZE;
    src_pos = src_pos % LONG_SIZE;
    dst_pos = dst_pos % LONG_SIZE;

    /*
     * combine the largest portion of the longword possible until
     * the entire bit string is combined
     */

    while (size > 0)
    {
	/*
	 * determine the maximum number of bits that can be copied this time
	 */

        nbits = LONG_SIZE - (src_pos <= dst_pos ? dst_pos : src_pos);
        nbits = (nbits <= size ? nbits : size);

	/*
	 * form the dmask
	 */

	dmask = hi_mask[dst_pos] | lo_mask[dst_pos + nbits];

	/*
	 * save the destination word pointer before the pointers are updated
	 */

	dst2 = dst;

	/*
	 * calculate the masked aligned source word and
	 * update source and destination word pointers and offsets
	 */

        if (src_pos >= dst_pos)
	{
	    if (src_addr == 0)
		src_word = -1 & mask;
	    else
	        src_word = (*src >> src_pos - dst_pos) & mask;
	    dst += (dst_pos + nbits) / LONG_SIZE;
	    dst_pos = (dst_pos + nbits) % LONG_SIZE;
	    src_pos = 0;
	    src++;
	}
	else
	{
	    if (src_addr == 0)
		src_word = -1 & mask;
	    else
	        src_word = (*src << dst_pos - src_pos) & mask;
	    src_pos += nbits;
	    dst_pos = 0;
	    dst++;
	}

	/*
	 * decrease the amount of bits left to be copied
	 */

        size -= nbits;

	/*
	 * apply the rule
	 */

        switch (rule)
        {
	    case 1:
    
	        /*
	         * 0
	         */

	        *dst2 = (*dst2 & dmask) | (~dmask & 0);
		break;

	    case 2:
    
	        /*
	         * src AND dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (src_word & *dst2));
		break;
    
	    case 3:
    
	        /*
	         * src AND NOT dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (src_word & ~*dst2));
		break;
    
	    case 4:
    
	        /*
	         * src
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (src_word));
		break;
    
	    case 5:
    
	        /*
	         * NOT src AND dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (~src_word & *dst2));
		break;
    
	    case 6:
    
	        /*
	         * dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (*dst2));
		break;
    
	    case 7:
    
	        /*
	         * src XOR dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (src_word ^ *dst2));
		break;
    
	    case 8:
    
	        /*
	         * src OR dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (src_word | *dst2));
		break;
    
	    case 9:
    
	        /*
	         * NOT src AND NOT dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (~src_word & ~*dst2));
		break;
    
	    case 10:
    
	        /*
	         * NOT src XOR dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (~src_word ^ *dst2));
		break;
    
	    case 11:
    
	        /*
	         * NOT dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (~*dst2));
		break;
    
	    case 12:
    
	    	/*
	         * src OR NOT dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (src_word | ~*dst2));
		break;
    
	    case 13:
    
	        /*
	         * NOT src
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (~src_word));
		break;
    
	    case 14:
    
	        /*
	         * NOT src OR dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (~src_word | *dst2));
		break;
    
	    case 15:
    
	        /*
	         * NOT src OR NOT dst
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask & (~src_word | ~*dst2));
		break;
    
	    case 16:
    
	        /*
	         * 1
	         */
    
	        *dst2 = (*dst2 & dmask) | (~dmask);
		break;
    
	    default:
	        break;
        }
    }
}
