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
*/

/************************************************************************
**  IPS__MOVE_BITS.C
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	This module contains a function that moves by UDP from a source
**	to a destination a pixel at a time.  It is identical in functionality
**	to the VAX MACRO function of the same name. This function will 
**	select the most optimal way of moving the data based on the alignment.
**
**  ENVIRONMENT:
**
**	UNIX
**
**  AUTHOR(S):
**
**	Karen D'Intino
**
**  CREATION DATE:
**
**	25-JULY-1991
**
************************************************************************/

/*
**  Table of contents:
**
**	Global routines
*/

#ifdef NODAS_PROTO
long	_IpsMoveBits();           /* Main routine. Dispatch to optimal move*/

/* Dispatch routines which determine the most optimal rtn to handle the move  */

void	_IpsAlignedToAligned();   /* Moving aligned data to aligned data plane*/
void	_IpsAlignedToUnaligned(); /* Moving aligned to an unaligned data plane*/
void	_IpsUnalignedToAligned(); /* Moving unaligned data to aligned         */
long	_IpsUnalignedToUnaligned();/* Moving unaligned data to unaligned      */

/* Aligned to Aligned */
void    _IpsAlignedByteToAlignedByte(); /* Move Byte to Byte aligned format */
void    _IpsAlignedByteToAlignedWord(); /* Move Byte to Word aligned format */
void    _IpsAlignedByteToAlignedLong(); /* Move Byte to Long aligned format */
void    _IpsAlignedWordToAlignedWord(); /* Move Word to Word aligned format */
void    _IpsAlignedWordToAlignedLong(); /* Move Word to Long aligned format */
void    _IpsAlignedLongToAlignedLong(); /* Move Long to Long aligned format */

/* Aligned to Unaligned */
void	_IpsAlignedByteToUnaligned(); /* Move aligned byte to unaligned format*/
void    _IpsAlignedWordToUnaligned(); /* Move Aligned word to unaligned format*/
void    _IpsAlignedLongToUnaligned(); /* Move aligned long to unaligned format*/

/* 
** Aligned to Unaligned (but just padded or pos shifted).
** A special case for moving interleaved by pixel format to native.
*/
void	_IpsAlignedByteToUnalignedPad(); /* Move aligned byte to unal. padded*/
void    _IpsAlignedWordToUnalignedPad(); /* Move Aligned word to unal. padded*/
void    _IpsAlignedLongToUnalignedPad(); /* Move aligned long to unal. padded*/
				
/* Unaligned to Aligned */
void	_IpsUnalignedToAlignedByte();    /* Move Unaligned to Aligned byte */
void    _IpsUnalignedToAlignedWord();    /* Move Unaligned to Aligned word */
void    _IpsUnalignedToAlignedLong();    /* Move Unaligned to Aligned long */

/* 
** Unaligned (but just padded or pos shifted) to Aligned 
** A special case for interleaved by pixel data moving to native.
*/
void	_IpsUnalignedPadToAlignedByte(); /* Move Unal. with pad to Aligned byte */
void	_IpsUnalignedPadToAlignedWord(); /* Move  "  with pad to Aligned word */
void	_IpsUnalignedPadToAlignedLong(); /* Move  "  with pad to Aligned long */

/* Unaligned to Unaligned */
void	_IpsMoveBitByBit();      /* Move data a bit at a time (for unaligned)*/
#endif
void	_IpsBytePaddedToPadded();/* Aligned but just padded pixel(byte to byte) */


/*
**  Include files:
*/

#include <IpsDef.h>
#include <IpsStatusCodes.h>
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif

/*
**  External References:
**
**	none
*/

/*
**  Local Storage:
**
**	Bit mask tables used for moving individual bits.
*/
static char	highbits_mask[9] = { 
			0xFF, 0xFE, 0xFC, 0xF8,
			0xF0, 0xE0, 0xC0, 0x80,
			0x00 };

static char	lowbits_mask[9] = { 
			0x00, 0x01, 0x03, 0x07,
			0x0F, 0x1F, 0x3F, 0x7F,
			0xFF };

static int	lowbits_longmask[33] = { 
			0x00000000, 0x00000001, 0x00000003, 0x00000007,
			0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
			0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
			0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
			0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF,
			0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
			0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF,
			0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 
			0xFFFFFFFF  };

static short int lowbits_wordmask[17] = { 
			0x00000000, 0x00000001, 0x00000003, 0x00000007,
			0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
			0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
			0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
			0x0000FFFF  };

static short int	highbits_wordmask[17] = { 
			0x0000FFFF, 0x0000FFFE, 0x0000FFFC, 0x0000FFF8,
			0x0000FFF0, 0x0000FFE0, 0x0000FFC0, 0x0000FF80,
			0x0000FF00, 0x0000FE00, 0x0000FC00, 0x0000F800,
			0x0000F000, 0x0000E000, 0x0000C000, 0x00008000,
			0x00000000 };

static int	        highbits_longmask[33] = { 
			0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF8,
			0xFFFFFFF0, 0xFFFFFFE0, 0xFFFFFFC0, 0xFFFFFF80,
			0xFFFFFF00, 0xFFFFFE00, 0xFFFFFC00, 0xFFFFF800,
			0xFFFFF000, 0xFFFFE000, 0xFFFFC000, 0xFFFF8000,
			0xFFFF0000, 0xFFFE0000, 0xFFFC0000, 0xFFF80000,
			0xFFF00000, 0xFFE00000, 0xFFC00000, 0xFF800000,
			0xFF000000, 0xFE000000, 0xFC000000, 0xF8000000,
			0xF0000000, 0xE0000000, 0xC0000000, 0x80000000,
			0x00000000 };


/******************************************************************************
**  _IpsMoveBits()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Global interface to the portable version of the function
**	that moves bits from a source to a destination a bitfield
**	at a time. This main routine will determine if there is any
**	optimal way to move the data. The move can be optimized if
**      either the destination or the source is aligned.
** 
**	The (Unaligned -> Aligned) Move and the (Aligned -> Unaligned)
**      cases occur frequently as our conversion services call this 
**      routine to do the Native format conversion copy. 
**      Special case handling is provided for these types of moves. 
**
**	This routine will classify the move operation into one of the
**	following categories:
**
**		Aligned --> Aligned
**		Aligned --> Unaligned
**  
**		Unaligned --> Aligned
**		Unaligned --> Unaligned
**
**	Once the data is classified this routine will call the 
**	appropriate subroutine to further classify the type of move.
**	Once the most optimal method is determined, the move will be
**	executed.
** 
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**      long returns status
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

long  _IpsMoveBits( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    long dst_aligned = FALSE;	    /* flag to indicate if dest is aligned */
    long src_aligned = FALSE;	    /* flag to indicate if src is aligned  */
    long status = IpsX_SUCCESS;
    /* Determine if the source or destination is Aligned */
    /* To be considered aligned, everything must be aligned */

    /* If all but the  pixel length is aligned, 
    ** that is taken into consideration later to optimize the move.
    */
    if (((srcudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
        && ((srcudp->UdpL_PxlStride % BYTE_SIZE ) == 0)
        && ((srcudp->UdpW_PixelLength % BYTE_SIZE ) == 0) 
        && ((srcudp->UdpL_Pos % BYTE_SIZE ) == 0))
        src_aligned = TRUE;

    if (((dstudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
        && ((dstudp->UdpW_PixelLength % BYTE_SIZE ) == 0)
        && ((dstudp->UdpL_PxlStride % BYTE_SIZE ) == 0)
        && ((dstudp->UdpL_Pos % BYTE_SIZE ) == 0))
        dst_aligned = TRUE;

    if (src_aligned)
        {
        if (dst_aligned)
            _IpsAlignedToAligned(srcudp,dstudp);
	else 
            _IpsAlignedToUnaligned(srcudp,dstudp);
	}
    else
        {
        if (dst_aligned)
            _IpsUnalignedToAligned(srcudp,dstudp);
	else 
	    /* The only routine which currently returns status */
            status = _IpsUnalignedToUnaligned(srcudp,dstudp);
        }
    return (status);
    } /* End of _IpsMoveBits */

/******************************************************************************
**  _IpsAlignedToAligned()
**
**  FUNCTIONAL DESCRIPTION:
**
**	The Source and the destination data is Aligned. 
**      This routine determines the type of Destination data that we have. 
**      It will branch off to the appropriate optimal subroutine to handle 
**      the move based on the destination data format.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _IpsAlignedToAligned( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    /* Note:
    ** When the data types are the same, they will be moved by
    ** plane. If there is padding on the source or destination scanline,
    ** the data will be moved by line. If there is padding on the
    ** pixels themselves, the move will be done in this module.
    */
    /* The data can only be moved to an equal or greater length and
    ** this has already been checked in IPS__COPY_UTILS. 
    */
    switch (srcudp->UdpW_PixelLength)
        {
        case BYTE_SIZE:
	    switch (dstudp->UdpW_PixelLength)
	        {
	        case BYTE_SIZE:
	            /* 
		    ** Moving aligned byte to aligned byte, for
	            ** the case where the pixels are padded and therefore
		    ** cannot be moved by plane or line.
	            */
		    _IpsAlignedByteToAlignedByte(srcudp,dstudp); 
		    break;
		   
	        case WORD_SIZE:
		    _IpsAlignedByteToAlignedWord(srcudp,dstudp); 
		    break;
		    
		case LONG_SIZE:
		    _IpsAlignedByteToAlignedLong(srcudp,dstudp);
		    break;

	        default:
		    break;
		}
            break;

        case WORD_SIZE:
	    switch (dstudp->UdpW_PixelLength)
	       {
	       /* Moving from aligned word to either word or
	       ** long aligned. 
	       */
	       case WORD_SIZE:
	            /* Moving aligned word to aligned word for
	            ** the case where the pixels are padded 
		    ** and therefore cannot be moved by plane or by line.
		    */
		    _IpsAlignedWordToAlignedWord(srcudp,dstudp); 
		    break;

	       case LONG_SIZE:
	            _IpsAlignedWordToAlignedLong(srcudp,dstudp);
		    break;

	       default:
		    break;
	       }
            break;

        case LONG_SIZE:
	        _IpsAlignedLongToAlignedLong(srcudp,dstudp);
            break;
        default:
            break;
        }
    }
/* End of  _IpsAlignedToAligned */

/******************************************************************************
**  _IpsAlignedByteToAlignedByte()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves data from an aligned byte to an aligned byte plane.
**      Handles the pixels which are padded out to a byte boundary.
**      If the data is not padded, it would have been moved by line
**      or by plane.
** 
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _IpsAlignedByteToAlignedByte( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned char *src_byte_ptr;    /* Source byte pointer         */
    unsigned char *dst_byte_ptr;    /* Destination byte pointer    */
    long src_pixel_stride_bytes;    /* Source pixel stride         */
    long dst_pixel_stride_bytes;    /* Destination pixel stride    */
    long src_pad;		    /* Source scanline padding     */
    long dst_pad;		    /* Destination scanline padding*/
    long iy;			    /* Scanline loop count variable*/
    long ix;			    /* Pixel loop count variable   */

    /* Determine the start of the source bytes */

    src_byte_ptr = (unsigned char *)srcudp->UdpA_Base + ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> BYTE_SHIFT);

    /* Determine the start of the destination bytes */

    dst_byte_ptr = (unsigned char *)dstudp->UdpA_Base + 
		    ((dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> BYTE_SHIFT);

    /* get the source scanline pad in bytes */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Get the source pixel stride in bytes. */
    src_pixel_stride_bytes = srcudp->UdpL_PxlStride >> BYTE_SHIFT;
    
    /* Get the destination pad in bytes. */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Get the destination pixel stride in bytes. */
    dst_pixel_stride_bytes = dstudp->UdpL_PxlStride >> BYTE_SHIFT; 

    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0;iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
	    *dst_byte_ptr = *src_byte_ptr; /* get the first byte      */
	    dst_byte_ptr += dst_pixel_stride_bytes; /* add in the stride & pad */
	    src_byte_ptr += src_pixel_stride_bytes; /*	    "		 */
	    }
        src_byte_ptr += src_pad; /* add in the scanline pad if any */
        dst_byte_ptr += dst_pad; /*		"		   */
        }
    } /* end of _IpsAlignedByteToAlignedByte*/

/******************************************************************************
**  _IpsAlignedByteToAlignedWord()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves data from an aligned byte to an aligned word plane.
**	Promotes the data from byte to word.
** 
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsAlignedByteToAlignedWord( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned char *src_byte_ptr;	/* Source Byte Address         */
    unsigned short int *dst_word_ptr;	/* Destination Word Address    */
    long src_pixel_stride_bytes;	/* Source pixel stride         */
    long dst_pixel_stride_words;	/* Destination pixel stride    */
    long src_pad;			/* Source scanline padding     */
    long dst_pad;			/* Destination scanline padding*/
    long iy;				/* Scanline count variable     */
    long ix;				/* Pixel count variable	       */

    /* Get the address of the start of the byte data */
    src_byte_ptr = (unsigned char *)srcudp->UdpA_Base + ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> BYTE_SHIFT);

    /* Get the word address of where to put the data in the destination */
    dst_word_ptr = (unsigned short int *)dstudp->UdpA_Base + 
		    ((dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> WORD_SHIFT);

    /* Get the source pixel stride in bytes */
    src_pixel_stride_bytes = srcudp->UdpL_PxlStride >> BYTE_SHIFT;

    /* Get the destination pixel stride in words  */
    dst_pixel_stride_words = dstudp->UdpL_PxlStride >> WORD_SHIFT; 

    /* Get the source line padding in bytes */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Get the destination pad in words */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> WORD_SHIFT); 

    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0;iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
	    *dst_word_ptr = (unsigned short int)*src_byte_ptr;  /* Move the byte to word    */
	    dst_word_ptr += dst_pixel_stride_words;  /* Advance to the next word */
	    src_byte_ptr += src_pixel_stride_bytes;  /* Advance to the next byte */
	    }
        src_byte_ptr += src_pad;    /* Add pad if any to get next scanline */
        dst_word_ptr += dst_pad;    /*		    "			   */
        }
    } /* end of _IpsAlignedByteToAlignedWord*/

/******************************************************************************
**  _IpsAlignedByteToAlignedLong()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves data from an aligned byte to an aligned longword plane.
**	Promotes the data from byte to long.
** 
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _IpsAlignedByteToAlignedLong( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned char *src_byte_ptr;    /* Address of source byte aligned data */
    unsigned int  *dst_long_ptr;    /* Address of destination long aligned */
    long src_pixel_stride_bytes;    /* Source pixel stride                 */
    long dst_pixel_stride_longs;    /* Destination pixel stride            */
    long src_pad;		    /* Source scanline padding             */
    long dst_pad;		    /* Destination scanline padding        */
    long iy;			    /* Scanline loop counter variable      */
    long ix;			    /* Pixel loop counter variable         */

    /* Get start of source byte data. */
    src_byte_ptr = (unsigned char *)srcudp->UdpA_Base + ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> BYTE_SHIFT);

    /* Get start of where data will be moved in destination. */
    dst_long_ptr = (unsigned int *)dstudp->UdpA_Base + 
		    ((dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> LONG_SHIFT);

    /* Source pixel stride in bytes */
    src_pixel_stride_bytes = srcudp->UdpL_PxlStride >> BYTE_SHIFT;

    /* Destination pixel stride in long words. */
    dst_pixel_stride_longs = dstudp->UdpL_PxlStride >> LONG_SHIFT;      

    /* Get the source scanline pad in bytes */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Get the destination scanline pad in longwords */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);
 
    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0;iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
	    *dst_long_ptr = *src_byte_ptr;  /* Move byte to long     */
	    dst_long_ptr += dst_pixel_stride_longs;  /* Advance to next pixel */
	    src_byte_ptr += src_pixel_stride_bytes;  /*         "	     */
	    }
        src_byte_ptr += src_pad;  /* Advance to next scanline */
        dst_long_ptr += dst_pad;  /*		"	      */
        }
    } /* end of _IpsAlignedByteToAlignedLong*/


/******************************************************************************
**  _IpsAlignedWordToAlignedWord()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves data from an aligned word to another aligned word plane.
**	This routine handles the special case where the pixel data is padded
**	out to a byte boundary. If it is not padded, the move would have
**	either been done by line or by plane.
** 
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _IpsAlignedWordToAlignedWord( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned short int *src_word_ptr;	/* Source data word address     */
    unsigned short int *dst_word_ptr;	/* Destination word address     */
    long src_pixel_stride_words;	/* Source pixel stride          */
    long dst_pixel_stride_words;	/* Destination pixel stride     */
    long src_pad;			/* Source scanline padding      */
    long dst_pad;			/* Destination scanline padding */
    long iy;				/* Scanline loop count variable */
    long ix;				/* Pixel loop count variable    */

    /* Get Source word data address */
    src_word_ptr = (unsigned short int *)srcudp->UdpA_Base + 
		    ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> WORD_SHIFT);

    /* Get Destination word data address */
    dst_word_ptr = (unsigned short int *)dstudp->UdpA_Base + 
		    ((dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> WORD_SHIFT);

    /* Get the source and destination pixel padding in words */
    src_pixel_stride_words = srcudp->UdpL_PxlStride >> WORD_SHIFT;
    dst_pixel_stride_words = dstudp->UdpL_PxlStride >> WORD_SHIFT; 

    /* Get the source scanline padding in words */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);

    /* Get the destination scanline padding in words */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);
 
    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0;iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
	    *dst_word_ptr = *src_word_ptr;   /* move the pixel value       */
	    dst_word_ptr += dst_pixel_stride_words;/* advance to the next pixel  */
	    src_word_ptr += src_pixel_stride_words;/*		"		   */
	    }
        src_word_ptr += src_pad;	    /* Add in line padding if any */
        dst_word_ptr += dst_pad;	    /*		"		  */
        }
    } /* end of _IpsAlignedWordToAlignedWord */


/******************************************************************************
**  _IpsAlignedWordToAlignedLong()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves data from an aligned word to another aligned longword plane.
**	Promotes the data from word to long.
** 
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsAlignedWordToAlignedLong( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned short int *src_word_ptr;	/* Source word data pointer      */
    unsigned int *dst_long_ptr;	/* Destination long data pointer */
    long src_pixel_stride_words;	/* Source pixel stride           */
    long dst_pixel_stride_longs;	/* Destination pixel stride      */
    long src_pad;			/* Source scanline padding       */
    long dst_pad;			/* Destination scanline padding  */
    long iy;				/* Scanline loop count variable  */
    long ix;				/* Pixel loop count variable     */

    /* Get start of source word data */
    src_word_ptr = (unsigned short int *)srcudp->UdpA_Base + 
		    ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> WORD_SHIFT);

    /* Get start of where to put the destination data */
    dst_long_ptr = (unsigned int *)dstudp->UdpA_Base + 
		    ((dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> LONG_SHIFT);

    /* Determine the source pixel stride in words */
    src_pixel_stride_words = srcudp->UdpL_PxlStride >> WORD_SHIFT;

    /* Determine the destination pixel stride in longwords */
    dst_pixel_stride_longs = dstudp->UdpL_PxlStride >> LONG_SHIFT; 

    /* Get the source line pad in words */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);

    /* Get the destination scanline pad in longwords */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);
 
    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0;iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
	    *dst_long_ptr = *src_word_ptr;  /* move the word         */
	    dst_long_ptr += dst_pixel_stride_longs;  /* Advance to next pixel */
	    src_word_ptr += src_pixel_stride_words;  /*          "            */
	    }
        src_word_ptr += src_pad; /* Advance past any line padding */
        dst_long_ptr += dst_pad; /*		"		  */
        }
    } /* end of _IpsAlignedWordToAlignedLong*/

/******************************************************************************
**  _IpsAlignedLongToAlignedLong()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves data from an aligned Long to another aligned longword plane.
**	This routine moves the data that is pixel padded. The padding
**	is aligned to a byte boundary.
** 
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _IpsAlignedLongToAlignedLong( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned int *src_long_ptr;    /* Source longword address */
    unsigned int *dst_long_ptr;    /* Destination longword address */
    long src_pixel_stride_longs;    /* Source pixel stride	    */
    long dst_pixel_stride_longs;    /* Destination pixel stride     */
    long src_pad;		    /* Source scanline padding      */
    long dst_pad;		    /* Destination scanline padding */
    long iy;			    /* Scanline count loop variable */
    long ix;			    /* Pixel count loop variable    */

    /* Get the address of the start of data in the source structure */
    src_long_ptr = (unsigned int *)srcudp->UdpA_Base + 
		    ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> LONG_SHIFT);

    /* Get the address of the destination location for data placement */
    dst_long_ptr = (unsigned int *)dstudp->UdpA_Base + 
		    ((dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> LONG_SHIFT);

    /* Determine the pixel stride in longwords */
    src_pixel_stride_longs = srcudp->UdpL_PxlStride >> LONG_SHIFT;
    dst_pixel_stride_longs = dstudp->UdpL_PxlStride >> LONG_SHIFT;      

    /* Get the scanline paddings in longwords. */

    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);

    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);
 
    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0;iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
	    *dst_long_ptr = *src_long_ptr; /* Move the longword           */
	    dst_long_ptr += dst_pixel_stride_longs; /* Advance to next pixel if any */ 
	    src_long_ptr += src_pixel_stride_longs; /*              "              */
	    }
        src_long_ptr += src_pad;    /* Advance past any line padding. */
        dst_long_ptr += dst_pad;    /*		    "		      */
        }
    } /* end of _IpsAlignedLongToAlignedLong*/

/******************************************************************************
**  _IpsAlignedToUnaligned()
**
**  FUNCTIONAL DESCRIPTION:
**
**	The Source data is Aligned. This routine determines the type of
**	Unaligned Destination data that we have. It will branch off to 
**      the appropriate optimal subroutine to handle the move based on 
**      the destination data format.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _IpsAlignedToUnaligned( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    /* Dispatch to the appropriate routine */
    switch (srcudp->UdpW_PixelLength)
        {
	/* Note: For cases where the destination is simply padded 
	** can be special cased (as is done for the case where the
	** source is simply padded).
	*/

        case BYTE_SIZE:
            if (((dstudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
		&& (srcudp->UdpW_PixelLength == dstudp->UdpW_PixelLength)
                && ((dstudp->UdpL_PxlStride % BYTE_SIZE ) == 0) &&
	        (((srcudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= BYTE_SIZE) &&
	        (((dstudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= BYTE_SIZE))
				
	        /* This test determines if the destination data
		** is Unaligned but just padded out. If the data is 
		** of type 3,3,2 for example, moving via _IpsAlignedByteto
		** UnalignedPad will be faster because all the data
		** falls within the byte boundary. 
		** 
		** The _IpsAlignedByteToUnalignedPad routine doesn't
		** need to handle data that falls outside the first byte.
		** 
		** This test verifies that the bit offset + pixel length 
		** is less than a byte.
	        */
		{
	        _IpsAlignedByteToUnalignedPad( srcudp, dstudp );
		}
	    else
	        {
	        _IpsAlignedByteToUnaligned(srcudp,dstudp); 
		}
            break;

        case WORD_SIZE:
	        if( ((dstudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
                && ((dstudp->UdpL_PxlStride % BYTE_SIZE ) == 0) &&
		(srcudp->UdpW_PixelLength == dstudp->UdpW_PixelLength) && 
	        (((srcudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= WORD_SIZE) &&
	        (((dstudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= WORD_SIZE))

	        /* Aligned data, just non-aligned pixel length */
	        /* Pos can be unaligned too because that's what is used
	        ** to get to the next components.
	        */
		{
	        _IpsAlignedWordToUnalignedPad( srcudp, dstudp );
		}
	    else
	        {
  	        _IpsAlignedWordToUnaligned(srcudp,dstudp);
		}
            break;

        case LONG_SIZE:
		if (((dstudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
                && ((dstudp->UdpL_PxlStride % BYTE_SIZE ) == 0) && 
		(srcudp->UdpW_PixelLength == dstudp->UdpW_PixelLength) && 
	        (((srcudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= LONG_SIZE) &&
	        (((dstudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= LONG_SIZE))

	        /* Aligned data, just non-aligned pixel length */
	        /* Pos can be unaligned too because that's what is used
	        ** to get to the next components.
	        */
		{
	        _IpsAlignedLongToUnalignedPad( srcudp, dstudp );
		}
	    else
	        {
	        _IpsAlignedLongToUnaligned(srcudp,dstudp); 
		}
            break;

	default:
            break;
        }
    } /* End of  _IpsAlignedToUnaligned */

/******************************************************************************
**  _IpsAlignedByteToUnaligned()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a Aligned format source to an unaligned destination a 
**	bitfield at a time.
**
**	Note: This routine only moves 8 bits (pixel length is always 8).
**	    It is known then, that the bit offset will always remain the
**	    same regardless of the stride. The pixel stride can be greater
**	    than 8 but it must be aligned. It is also implied that once 
**	    the bits from the first part of the byte have been moved, 
**	    there remain (8 - bit_offset) bits to move in the higher part 
**	    of the byte.
** 
**	For Example:
** 
**	    src: |________|
**		 7	  0				    
**
**                    
**			Destination  |________|________|
**				     15       7        0
**						    <--| bit offset = 3
**                                                     ^
**						    Nearest Byte
**
** 
**	1) move the high bits of interest out of the source into the dest.
**
**
**		d_nearest_byte & lowbits_mask[d_bit_offset]
**
**			  00000111 (Mask out low bits to offset in the dest.
**				    so that these bits are preserved in the
**				    new destination byte)
**		|--------|--------|
**		15	 7	  0
**
**			or with:
**
**
**			 |--------|	Source
**                       7        0
**		Shifted left by bit offset << (3)
**		&         11111000	    (highbitmask bit offset)
**
**	    Since it is known that there are 8 bits to move it is not
**	    necessary to mask out the destination high bits in this move.
**
** 
**  d_nearest_byte_ptr++;
**  2)	Get the remaining bits (there will be d_bit_offset bits remaining)
**	      Src_byte_ptr & highbits_mask [ 8 - d_bit_offset]
**		|________| & 11100000	Shifted right by 5 when 
**		7	 0		    placed in the new dest.
**		        
**	or'd with:
** 
**	    d_nearest_byte & highbits_mask[d_bit_offset]
**		|________| & 11111000
**
** 
** 
**	        The result looks like this in the dest bytes:
**			  |new byte|<---offset = 3   
**		    |________|________|
**		   15        7        0
**
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsAlignedByteToUnaligned( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned char *d_base = dstudp->UdpA_Base;	/* Destination base pointer */
    unsigned char *src_byte_ptr;	    /* Source byte pointer          */
    unsigned char *d_nearest_byte;	    /* Destination nearest byte     */
    long d_line_offset;			    /* Destination line offset      */
    long d_pixel_offset;		    /* Destination pixel offset     */
    long d_bit_offset;			    /* Destination bit offset       */
    long src_pixel_stride_bytes;	    /* Src Pixel stride in bytes    */
    long src_pad;			    /* Source scanline padding      */
    long iy;				    /* Scanline loop count variable */
    long ix;				    /* Pixel loop count variable    */
    long bits_remaining;
    unsigned long value = 0;		    /* The Pixel value to be copied */

    /* Calculate the bit offset to the data to be copied. */

    d_bit_offset = (dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride));

    /* Position the byte pointer to the first source byte */

    src_byte_ptr = (unsigned char *)srcudp->UdpA_Base + 
		    ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> BYTE_SHIFT);

    /* Get the source pixel stride in bytes */
    src_pixel_stride_bytes = srcudp->UdpL_PxlStride >> BYTE_SHIFT;      

    /* Get the scanline padding in bytes */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Set the line offset to the current bit offset */
    d_line_offset = d_bit_offset;
 
    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
	/* Set destination pixel offset to the current line offset. */
        d_pixel_offset = d_line_offset;
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
            bits_remaining = dstudp->UdpW_PixelLength;
	    value = ((long)*src_byte_ptr) & 0xFF;

	    /* Get the nearest byte address where data is to be 
	    ** moved to, then get the bit offset into that byte. 
	    */
            d_nearest_byte = d_base + (d_pixel_offset >> BYTE_SHIFT);

	    /* 
	    ** Get the bit offset to the beginning of the pixel data
	    ** into the current byte.
	    */
            d_bit_offset = d_pixel_offset & 7;
  
	    /* Mask out the destination low bits so they are not lost */
	    /* Shift by the offset into the destination */
	    /* Mask out any high bits  */

            *d_nearest_byte = (*d_nearest_byte & lowbits_mask[d_bit_offset])|
		((value << d_bit_offset) & highbits_mask[d_bit_offset]);

	    bits_remaining -= (8 - d_bit_offset);

	    /* 
	    ** Advance the destination pointer to the next byte to get 
	    ** any remaining bits in the pixel.
	    */
	    *d_nearest_byte++;

	    /*
	    ** Put the bits from the lower portion of the last byte.
	    */
	    value >>= (BYTE_SIZE - d_bit_offset);

	    while (bits_remaining >= BYTE_SIZE)
	        {
		*d_nearest_byte++ = (unsigned char)value;
		value >>= BYTE_SIZE;
		bits_remaining -= 8;
		}

	    if (bits_remaining > 0)
	        *d_nearest_byte = (*d_nearest_byte & 
				highbits_mask[ d_bit_offset ]) + value;

	    /* Advance to the next source pixel */
            src_byte_ptr += src_pixel_stride_bytes;

	    /* Advance the destination pixel offset to the next pixel pos.*/
	    d_pixel_offset += dstudp->UdpL_PxlStride;
	    }

	/* Advance past any padding to the next scanline. */
        src_byte_ptr += src_pad;
	/* Advance the destination line offset to the next line */
        d_line_offset += dstudp->UdpL_ScnStride;
        }
    } /* end of _IpsAlignedByteToUnaligned*/


/******************************************************************************
**  _IpsAlignedByteToUnalignedPad()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a Aligned format source to an unaligned (just padded)
**      destination a bitfield at a time. The destination data is aligned on
**	it's pixel boundaries, it is just padded out OR it starts on a non-
**      byte boundary (i.e. the pos is not aligned ).
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsAlignedByteToUnalignedPad( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned char *d_base = dstudp->UdpA_Base;	/* Destination base pointer */
    unsigned char *src_byte_ptr;	    /* Source byte pointer          */
    unsigned char *d_nearest_byte;	    /* Destination nearest byte     */
    long dst_pixel_stride_bytes;	    /* Dest. Pixel stride in bytes  */
    long src_pixel_stride_bytes;	    /* Src   Pixel stride in bytes  */
    long d_pixel_offset;		    /* Destination pixel offset     */
    long d_bit_offset;			    /* Destination bit offset       */
    long src_pad;			    /* Source scanline padding      */
    long dst_pad;			    /* Destination scanline padding */
    long iy;				    /* Scanline loop count variable */
    long ix;				    /* Pixel loop count variable    */

    /* Calculate the bit offset to the data to be copied. */

    d_bit_offset = (dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride));

    /* Position the byte pointer to the first source byte */

    src_byte_ptr = (unsigned char *)srcudp->UdpA_Base + 
		    ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> BYTE_SHIFT);


    src_pixel_stride_bytes = srcudp->UdpL_PxlStride >> BYTE_SHIFT;      

    /* Get the scanline padding in bytes */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    dst_pixel_stride_bytes = dstudp->UdpL_PxlStride >> BYTE_SHIFT;      

    /* Get the scanline padding in bytes */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Set the line offset to the current bit offset */
    d_pixel_offset = d_bit_offset >> BYTE_SHIFT;
    d_bit_offset = d_bit_offset & 7;
    d_nearest_byte = d_base + d_pixel_offset; 

    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
            *d_nearest_byte = (*d_nearest_byte & lowbits_mask[d_bit_offset])|
		((*src_byte_ptr << d_bit_offset) & highbits_mask[d_bit_offset])|
                 (*d_nearest_byte & highbits_mask[d_bit_offset]);

	    d_nearest_byte += dst_pixel_stride_bytes;/* Advance to next pixel */
            src_byte_ptr += src_pixel_stride_bytes;	  /*        "              */
	    }
        src_byte_ptr   += src_pad;    /* Add in scanline padding if any */
        d_nearest_byte += dst_pad;    /*             "                  */
        }
    } /* End of _IpsAlignedByteToUnalignedPad */

/******************************************************************************
**  _IpsAlignedWordToUnaligned()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a Aligned format word source to an unaligned 
**      destination a bitfield at a time.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
******************************************************************************/

void _IpsAlignedWordToUnaligned( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned short int *src_word_ptr;	  /* Source word pointer              */
    unsigned short int *d_nearest_word;	  /* Destination nearest word pointer */
    unsigned short int *d_base;		  /* Destination base pointer         */
    long d_bit_offset = 0;		  /* Destination bit offset to data   */
    long d_line_offset	= 0;		  /* Destination line offset          */
    long d_pixel_offset	= 0;		  /* Destination pixel offset         */
    unsigned long store_value;		  /* Temporary long to contain pixel  */
    long src_pixel_stride_words;	  /* source pixel stride in words     */
    long src_pad;			  /* Source scanline padding          */
    long iy;				  /* Scanline loop counter variable   */
    long ix;				  /* Pixel loop counter variable      */
    long bits_remaining;		  /* The number of bits in the dest   */
 
    store_value = 0;

    d_base = (unsigned short int *)dstudp->UdpA_Base;

    /* Calculate the bit offset to the data to be copied. */

    d_bit_offset = (dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride));

    /* Initialize  destination line offset to the dest bit offset */
    d_line_offset = d_bit_offset;

    /* Position the byte pointer to the first source byte */
    src_word_ptr = (unsigned short int *)srcudp->UdpA_Base + 
		    ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> WORD_SHIFT);


    /* Get the source stride in words */
    /* Determine the line pad if there is any */

    src_pixel_stride_words = srcudp->UdpL_PxlStride >> WORD_SHIFT;      

    /* Get the scanline padding in words */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);
 
    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
        d_pixel_offset = d_line_offset;
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
	    /* 
	    ** Position to nearest word and get current bit offset 
	    ** into the word.
	    */
            d_nearest_word = d_base + (d_pixel_offset >> WORD_SHIFT);
            d_bit_offset = d_pixel_offset & 15;

            bits_remaining = dstudp->UdpW_PixelLength;  

	    /* 
	    ** Get the Pixel value.
	    */
	    store_value = *src_word_ptr;

	    /* Put whatever bits will fit into the first word of storage */

	    *d_nearest_word = (*d_nearest_word & 
				    lowbits_wordmask[d_bit_offset]) |
			    (( store_value << d_bit_offset) & 
				    highbits_wordmask[d_bit_offset]);

	    /* Position the store value to the bits remaining */
	    store_value >>= (WORD_SIZE - d_bit_offset);

	    /* Advance to the next word to move any remaining bits */
	    *d_nearest_word++;
	
	    /* Put whatever is left into the last word of storage.
	    ** Note that the high bits should be preserved to properly
	    ** emulate INSV. They are preserved by masking them out and
	    ** adding them back into the final bits in value.
	    */
	    while (bits_remaining >= WORD_SIZE)
		{
	        *d_nearest_word++ = (unsigned short int)store_value;
		store_value >>= WORD_SIZE;
		bits_remaining -= WORD_SIZE;
		}

	    if (bits_remaining > 0)
	        *d_nearest_word = (*d_nearest_word &
				 highbits_wordmask[d_bit_offset]) + 
				   store_value;

	    /* Advance to the next destination pixel */
	    d_pixel_offset += dstudp->UdpL_PxlStride;
	    src_word_ptr += src_pixel_stride_words;  /* Advance to next pixel */
	    }
	/* Advance the source pointer past the pad (if any) to the 
	** next scanline 
	*/
        src_word_ptr += src_pad;
	/* Advance the destination line offset pointer to the next line */
        d_line_offset += dstudp->UdpL_ScnStride;
        }
    } /* End of _IpsAlignedWordToUnaligned*/

/******************************************************************************
**  _IpsAlignedWordToUnalignedPad()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a Aligned format source to an unaligned (just padded)
**      destination a bitfield at a time. The destination data is aligned on
**	it's pixel boundaries, it is just padded out OR it starts on a non-
**      byte boundary (i.e. the pos is not aligned ).
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsAlignedWordToUnalignedPad( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned short int *d_base;		    /* Destination base pointer */
    unsigned short int *src_word_ptr;	    /* Source word pointer          */
    unsigned short int *d_nearest_word;	    /* Destination nearest word     */
    long d_pixel_offset;		    /* Destination pixel offset     */
    long d_bit_offset;			    /* Destination bit offset       */
    long src_pixel_stride_words;	    /* Source pixel stride          */
    long src_pad;			    /* Source scanline padding      */
    long dst_pad;			    /* Destination scanline padding */
    long iy;				    /* Scanline loop count variable */
    long ix;				    /* Pixel loop count variable    */
    long dst_pixel_stride_words;	    /* Dest. pxl stride in words    */

    d_base  = (unsigned short int *)dstudp->UdpA_Base;

    /* Calculate the bit offset to the data to be copied. */

    d_bit_offset = (dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride));

    /* Position the  pointer to the first source word */

    src_word_ptr = (unsigned short int *)srcudp->UdpA_Base + 
		    ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> WORD_SHIFT);

    /* Get the source pixel stride in words */
    src_pixel_stride_words = srcudp->UdpL_PxlStride >> WORD_SHIFT;      

    /* Get the scanline padding in words */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);

    /* get the pixel stride in words */
    dst_pixel_stride_words = dstudp->UdpL_PxlStride >> WORD_SHIFT;      

    /* Get the scanline padding in words */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);


    /* Set the line offset to the current bit offset */
    d_pixel_offset = d_bit_offset >> WORD_SHIFT;
    d_bit_offset = d_bit_offset & 15;
    d_nearest_word = d_base + d_pixel_offset; 

    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
            *d_nearest_word = (*d_nearest_word & lowbits_wordmask[d_bit_offset])|
		((*src_word_ptr
			    << d_bit_offset) & highbits_wordmask[d_bit_offset])|
              (*d_nearest_word & highbits_wordmask[d_bit_offset]);

	    d_nearest_word += dst_pixel_stride_words;
            src_word_ptr += src_pixel_stride_words;
	    }

        src_word_ptr += src_pad;
        d_nearest_word += dst_pad;
        }
    } /* End of _IpsAlignedWordToUnalignedPad */

/******************************************************************************
**  _IpsAlignedLongToUnaligned()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a Aligned format long source to an unaligned 
**      destination a bitfield at a time.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
******************************************************************************/

void _IpsAlignedLongToUnaligned( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned char value[4];
    unsigned char *src_byte_ptr;
    unsigned char *d_nearest_byte;
    unsigned char *d_base = dstudp->UdpA_Base;
    long d_bit_offset	= (dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride));
    long d_line_offset	= 0;
    long d_pixel_offset	= 0;
    long bits_remaining;
    long src_stride;
    long src_pad;
    long iy;
    long ix;
    unsigned long store_value = 0;

    /* 
    ** Could be Pixel stride aligned.
    ** Need to special case this.
    ** Can move the  bits and bump by word.
    */
    *((unsigned long*)value) = 0;

    /*
    ** Set up the scanline line and pixel loops, and move the bits.
    */
    d_line_offset = d_bit_offset;

    src_byte_ptr = (unsigned char *)srcudp->UdpA_Base + ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> BYTE_SHIFT);

    /* Get the source PAD in bytes */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);
    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0;iy < srcudp->UdpL_ScnCnt; iy++)
        {
        d_pixel_offset = d_line_offset;
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
            d_nearest_byte = d_base + (d_pixel_offset >> BYTE_SHIFT);
            d_bit_offset = d_pixel_offset & 7;
  
    	    bits_remaining = dstudp->UdpW_PixelLength;

 	    value[0] = *src_byte_ptr++;
 	    value[1] = *src_byte_ptr++;
 	    value[2] = *src_byte_ptr++;
 	    value[3] = *src_byte_ptr++;
	    store_value = *(unsigned long *)value;
				   
	    /* Put whatever bits will fit into the first byte of storage */

	    *d_nearest_byte = (*d_nearest_byte & 
			lowbits_mask[d_bit_offset]) |
		    (( store_value << d_bit_offset) & 
			    highbits_mask[d_bit_offset]);

	    *d_nearest_byte++;
	    bits_remaining -= (BYTE_SIZE - d_bit_offset);
	    store_value >>= (BYTE_SIZE - d_bit_offset);

	    while (bits_remaining >= BYTE_SIZE)
		{
	        *d_nearest_byte++ = (unsigned char )store_value;
	        store_value >>= BYTE_SIZE;
	        bits_remaining -= BYTE_SIZE;		
		}

	    /* Put whatever is left into the last byte of storage.
	    ** Note that the high bits should be preserved to properly
	    ** emulate INSV. They are preserved by masking them out and
	    ** adding them back into the final bits in value.
	    */
	    if (bits_remaining > 0)
	        *d_nearest_byte = (*d_nearest_byte &
				 highbits_mask[d_bit_offset]) + 
				   store_value;

	    d_pixel_offset += dstudp->UdpL_PxlStride;
	    }
        src_byte_ptr += src_pad;
        d_line_offset += dstudp->UdpL_ScnStride;
        }
    } /* End of _IpsAlignedLongToUnaligned*/

/******************************************************************************
**  _IpsAlignedLongToUnalignedPad()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a Aligned format source to an unaligned (just padded)
**      destination a bitfield at a time. The destination data is aligned on
**	it's pixel boundaries, it is just padded out OR it starts on a non-
**      byte boundary (i.e. the pos is not aligned ).
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsAlignedLongToUnalignedPad( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned int *d_base;		    /* Destination base pointer */
    unsigned int *src_long_ptr;	    /* Source long pointer          */
    unsigned int *d_nearest_long;	    /* Destination nearest long     */
    long d_pixel_offset;		    /* Destination pixel offset     */
    long d_bit_offset;			    /* Destination bit offset       */
    long src_pixel_stride_longs;	    /* Source pixel stride          */
    long src_pad;			    /* Source scanline padding      */
    long dst_pixel_stride_longs;	    /* Destination pixel  stride    */
    long dst_pad;			    /* Destination scanline padding */
    long iy;				    /* Scanline loop count variable */
    long ix;				    /* Pixel loop count variable    */

    d_base = (unsigned int *)dstudp->UdpA_Base;
    /* Calculate the bit offset to the data to be copied. */

    d_bit_offset = (dstudp->UdpL_Pos +
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride));

    /* Position the byte pointer to the first source longword */
    src_long_ptr = (unsigned int *)srcudp->UdpA_Base + 
		    ((srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride)) >> LONG_SHIFT);

    /* Get the source pixel stride in longwords*/
    src_pixel_stride_longs = srcudp->UdpL_PxlStride >> LONG_SHIFT;      

    /* Get the scanline padding in long words */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);

    /* Get the destination pixel stride in longwords*/
    dst_pixel_stride_longs = dstudp->UdpL_PxlStride >> LONG_SHIFT;      

    /* Get the scanline padding in long words */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);

    /* Set the line offset to the current bit offset */
    d_pixel_offset = d_bit_offset >> LONG_SHIFT;
    d_bit_offset = d_bit_offset & 31;
    d_nearest_long = d_base + d_pixel_offset; 
    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
            *d_nearest_long = (*d_nearest_long & lowbits_longmask[d_bit_offset])|
		((*src_long_ptr << d_bit_offset) & 
		    highbits_longmask[d_bit_offset])|
              (*d_nearest_long & highbits_longmask[d_bit_offset]);

	    d_nearest_long += dst_pixel_stride_longs;/* Advance to next pixel */
            src_long_ptr   += src_pixel_stride_longs;/*          "            */
	    }
        src_long_ptr += src_pad;    /* Add in scanline padding if any */
        d_nearest_long += dst_pad;  /*                "               */
        }
    } /* End of _IpsAlignedLongToUnalignedPad */

/******************************************************************************
**  _IpsUnalignedPadToAlignedByte()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a data plane which is aligned (except that the 
**	Pixel Stride is padded out) to an aligned byte udp.
**	This type of data format is special cased because this is
**	the format that is used when converting from interleaved by
**	pixel to interleaved by plane.
** 
**      This routine handles special cases such as 3,3,2 data where
**      each pixel is aligned on a byte boundary and the data is less
**      than 8 bits and is found within the byte.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsUnalignedPadToAlignedByte( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    { 
    unsigned char *s_base = srcudp->UdpA_Base; /* Source base data pointer */
    unsigned char *dst_byte_ptr;	       /* Destination byte pointer */
    unsigned char *s_nearest_byte;	       /* Source nearest byte      */
    long s_bit_offset;			       /* Source bit offset        */
    long dst_pad;			       /* Destination padding      */
    long src_pad;			       /* Source padding           */
    long s_pixel_offset;		       /* Source pixel offset      */
    long iy;				       /* Scanline loop counter    */
    long ix;				       /* Pixel loop counter       */
    long src_pixel_stride_bytes;	       /* Src pixel stride in bytes*/
    long dst_pixel_stride_bytes;	       /* Destination pixel stride */
    /* Get Destination byte pointer */

    dst_byte_ptr = (unsigned char *)dstudp->UdpA_Base + ((dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> BYTE_SHIFT);

    /* Get source bit offset to start of data */
    s_bit_offset = (srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride));


    /* Get the source pixel stride in bytes */
    src_pixel_stride_bytes = srcudp->UdpL_PxlStride >> BYTE_SHIFT;      

    /* Get the scanline padding in bytes */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Get the destination pixel stride in bytes */
    dst_pixel_stride_bytes = dstudp->UdpL_PxlStride >> BYTE_SHIFT;      

    /* Get the scanline padding in bytes  */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Get the pixel offset in bytes */
    s_pixel_offset = s_bit_offset >> BYTE_SHIFT;
    /* Get the offset into the byte where the pixel data to be moved starts */
    s_bit_offset = s_bit_offset & 7;
            
    /* Get the source nearest byte */
    s_nearest_byte = s_base + s_pixel_offset;
    /*
    ** Outer loop that steps through lines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
	    /*
	    ** Shift the (high) bits of interest out of the byte 
	    ** and into the destination.
	    */
	    *dst_byte_ptr = ((*s_nearest_byte >> s_bit_offset)
				& lowbits_mask[ srcudp->UdpW_PixelLength]);

	    dst_byte_ptr += dst_pixel_stride_bytes;/* Advance to next pixel */
	    s_nearest_byte += src_pixel_stride_bytes; /*          "            */
	    }
	/* Advance source & dest byte pointers past any scanline padding */
        dst_byte_ptr += dst_pad;
        s_nearest_byte += src_pad;
        }
    } /* end of _IpsUnalignedPadToAlignedByte*/

/******************************************************************************
**  _IpsUnalignedPadToAlignedWord()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a data plane which is aligned (except that the 
**	Pixel Stride is padded out) to an aligned Word udp.
**	This type of data format is special cased because this is
**	the format that is used when converting from interleaved by
**	pixel to interleaved by plane.
** 
**      This routine handles special cases such as 3,3,2 data where
**      each pixel is aligned on a byte boundary and the data is less
**      than WORD_SIZE bits and is found within the word.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsUnalignedPadToAlignedWord( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    { 
    unsigned short int *s_base; /* Source base data pointer */
    unsigned short int *dst_word_ptr;	       /* Destination word pointer */
    unsigned short int *s_nearest_word;	       /* Source nearest word      */
    long s_bit_offset;			       /* Source bit offset        */
    long dst_pixel_stride_words;	       /* Destination pixel stride */
    long dst_pad;			       /* Destination padding      */
    long src_pixel_stride_words;	       /* Source pixel stride      */
    long src_pad;			       /* Source padding           */
    long s_pixel_offset;		       /* Source pixel offset      */
    long iy;				       /* Scanline loop counter    */
    long ix;				       /* Pixel loop counter       */

    s_base = (unsigned short int *)srcudp->UdpA_Base;

    /* Get Destination word pointer */
    dst_word_ptr = (unsigned short int *)dstudp->UdpA_Base + ((dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> WORD_SHIFT);

    /* Get source bit offset to start of data */
    s_bit_offset = (srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride));

    /* Get the source pixel stride in words*/
    src_pixel_stride_words = srcudp->UdpL_PxlStride >> WORD_SHIFT;      

    /* Get the scanline padding in words */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);

    /* Get the destination pixel stride in words*/
    dst_pixel_stride_words = dstudp->UdpL_PxlStride >> WORD_SHIFT;      

    /* Get the scanline padding in words */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);

    /* Get the pixel offset in words */
    s_pixel_offset = s_bit_offset >> WORD_SHIFT;
    /* Get the offset into the byte where the pixel data to be moved starts */
    s_bit_offset = s_bit_offset & 15;
            
    /* Get the source nearest word */
    s_nearest_word = s_base + s_pixel_offset;
    /*
    ** Outer loop that steps through lines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
	    /*
	    ** Shift the (high) bits of interest out of the word
	    ** and into the destination.
	    */
	    *dst_word_ptr = ((*s_nearest_word >> s_bit_offset)
				& lowbits_wordmask[ srcudp->UdpW_PixelLength]);

	    dst_word_ptr += dst_pixel_stride_words;/* Advance to next pixel */
	    s_nearest_word += src_pixel_stride_words; /*          "            */
	    }
	/* Advance source & dest word pointers past any scanline padding */
        dst_word_ptr += dst_pad;
        s_nearest_word += src_pad;
        }

    }/* End of _IpsUnalignedPadToAlignedWord */

/******************************************************************************
**  _IpsUnalignedPadToAlignedLong()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits from a data plane which is aligned (except that the 
**	Pixel Stride is padded out) to an aligned Long udp.
**	This type of data format is special cased because this is
**	the format that is used when converting from interleaved by
**	pixel to interleaved by plane.
** 
**      This routine handles special cases such as 3,3,2 data where
**      each pixel is aligned on a byte boundary and the data is less
**      than 32 bits and is found within the longword.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsUnalignedPadToAlignedLong( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    { 
    unsigned int *s_base;		       /* Source base data pointer */
    unsigned int  *dst_long_ptr;	       /* Destination long pointer */
    unsigned int  *s_nearest_long;	       /* Source nearest long      */
    long s_bit_offset;			       /* Source bit offset        */
    long dst_pad;			       /* Destination padding      */
    long src_pad;			       /* Source padding           */
    long s_pixel_offset;		       /* Source pixel offset      */
    long iy;				       /* Scanline loop counter    */
    long ix;				       /* Pixel loop counter       */
    long src_pixel_stride_longs;	       /* Src pixel stride in longs*/
    long dst_pixel_stride_longs;	       /* Dst pixel stride in longs*/

    s_base = (unsigned int *)srcudp->UdpA_Base;

    /* Get Destination long pointer */
    dst_long_ptr = (unsigned int *)dstudp->UdpA_Base + ((dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> LONG_SHIFT);

    /* Get source bit offset to start of data */
    s_bit_offset = (srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride));


    /* Get the source pixel stride in longwords*/
    src_pixel_stride_longs = srcudp->UdpL_PxlStride >> LONG_SHIFT;      

    /* Get the scanline padding in long words */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);

    /* Get the destination pixel stride in longwords*/
    dst_pixel_stride_longs = dstudp->UdpL_PxlStride >> LONG_SHIFT;      

    /* Get the scanline padding in long words */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);

    /* Get the pixel offset in words */
    s_pixel_offset = s_bit_offset >> LONG_SHIFT;
    /* Get the offset into the byte where the pixel data to be moved starts */
    s_bit_offset = s_bit_offset & 31;
            
    /* Get the source nearest longword */
    s_nearest_long = s_base + s_pixel_offset;
    /*
    ** Outer loop that steps through lines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
	    /*
	    ** Shift the (high) bits of interest out of the word
	    ** and into the destination.
	    */
	    *dst_long_ptr = ((*s_nearest_long >> s_bit_offset)
				& lowbits_longmask[ srcudp->UdpW_PixelLength]);

	    dst_long_ptr += dst_pixel_stride_longs;/* Advance to next pixel */
	    s_nearest_long += src_pixel_stride_longs; /*          "            */
	    }
	/* Advance source & dest word pointers past any scanline padding */
        dst_long_ptr += dst_pad;
        s_nearest_long += src_pad;
        }
    }/* End of _IpsUnalignedPadToAlignedLong */

/******************************************************************************
**  _IpsUnalignedToAligned()
**
**  FUNCTIONAL DESCRIPTION:
** 
**	The Destination is Aligned. 
**
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _IpsUnalignedToAligned( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    /* Dispatch to the appropriate routine */
    switch (dstudp->UdpW_PixelLength)
        {
        case BYTE_SIZE:
            /* if the destination is byte aligned bytes */
            /* move < = 8 bits to the byte aligned byte destination */
            /* 
            ** Could be 3,3,2 image. (Pixel stride aligned).
            ** Need to special case this.
            ** Can move the three bits and bump by byte.
            */

            if (((srcudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
                && ((srcudp->UdpL_PxlStride % BYTE_SIZE ) == 0) &&
	        (((srcudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= BYTE_SIZE) &&
	        (((dstudp->UdpL_Pos % BYTE_SIZE)+ srcudp->UdpW_PixelLength) <= BYTE_SIZE))
	        /* byte aligned, just non-aligned pixel length */
	        /* Pos can be unaligned too because that's what is used
	        ** to get to the second and third plane data 
	        */
		{
	        _IpsUnalignedPadToAlignedByte( srcudp, dstudp );
		}
	    else	    
		{
    	        _IpsUnalignedToAlignedByte(srcudp,dstudp);
		}
            break;

        case WORD_SIZE:
            /* if the destination is word aligned words */

            if (((srcudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
                && ((srcudp->UdpL_PxlStride % BYTE_SIZE ) == 0) && 
	        (((srcudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= WORD_SIZE) &&
	        (((dstudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= WORD_SIZE))
	        /* byte aligned, just non-aligned pixel length */
	        /* Pos can be unaligned too because that's what is used
	        ** to get to the second and third plane data 
	        */
		{
	        _IpsUnalignedPadToAlignedWord( srcudp, dstudp );
		}
	    else
		{
	        _IpsUnalignedToAlignedWord(srcudp,dstudp);
		}
            break;

        case LONG_SIZE:
            /* if the destination is long aligned long */
            /* move < = 32 bits to the long aligned long destination */
            if (((srcudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
                && ((srcudp->UdpL_PxlStride % BYTE_SIZE ) == 0) &&
	        (((srcudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= LONG_SIZE) &&
	        (((dstudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= LONG_SIZE))
	        /* byte aligned, just non-aligned pixel length */
	        /* Pos can be unaligned too because that's what is used
	        ** to get to the second and third plane data 
	        */
		{
	        _IpsUnalignedPadToAlignedLong( srcudp, dstudp );
		}
	    else	    
	        {
	        _IpsUnalignedToAlignedLong(srcudp,dstudp);
		}
            break;

	default:
            break;
        }
    }/* End of routine _IpsUnalignedToAligned */


/******************************************************************************
**  _IpsUnalignedToAlignedByte()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits to a Aligned format destination a bitfield at a time.
**
**
**	For Example:
**			pixel length = 8
**	           11111111<-- Bit offset = 3
**	    |________|________|
**	    15	     7	     0				    
**                    
**	 Destination aligned  |________|
**		              7        0
**
** 
**	1) Get as many bits as we can from the first byte.
**
**		start off with bits remaining = 8;
**		Source byte shifted right by three (src bit offset), 
**		then anded with lowbits mask.
**
**			      11111
**			 |--------|
**			 7	  0
**
**  src_nearest_byte_ptr++; Increment the source to next byte to get
**			    the remaining bits.
** 
**  bits_remaining -= 8 - bit_offset; 
**  
** 
**  2)	Get the remaining bits.
**	      Src_byte_ptr & lowbits_mask [bits_remaining (3)]
**		|_____111| & 00000111	Shifted left by 5 when 
**		15	 7		    placed in the new dest.
**		        
**	or'd with:
** 
**	    destination byte |___11111|
**			     7        0
**  Gives:	 |11111111|
**               7        0
** 
***********************************************************************
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsUnalignedToAlignedByte( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    { 
    unsigned char *s_base = srcudp->UdpA_Base; /* Source base address         */
    unsigned char *dst_byte_ptr;	       /* Destination byte pointer    */
    unsigned char *s_nearest_byte;	       /* Source nearest byte ptr     */
    long s_bit_offset;			       /* Source bit offset           */
    long dst_pixel_stride_bytes;	       /* Destination pixel stride    */
    long dst_pad;			       /* Destination padding         */
    long s_line_offset;			       /* Source line offset          */
    long s_pixel_offset;		       /* Source pixel offset         */
    long bits_remaining;		       /* Number of bits remaining    */
    long iy;				       /* Scanline loop count variable*/
    long ix;				       /* Pixel loop count variable   */

    /* Source pixel bit offset */
    s_bit_offset = (srcudp->UdpL_Pos +	
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride));
    
    /* Destination pixel byte pointer */
    dst_byte_ptr = (unsigned char *)dstudp->UdpA_Base + ((dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> BYTE_SHIFT);

    /* Initialize the source line offset to be the bit offset */
    s_line_offset = s_bit_offset;


    /* Get the dst pixel stride in bytes */
    dst_pixel_stride_bytes = dstudp->UdpL_PxlStride >> BYTE_SHIFT;      

    dst_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /*
    ** Outer loop that steps through lines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
	/* Initialize the source pixel offset to be the line offset */
        s_pixel_offset = s_line_offset;
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
            /*
            ** Calculate the address of the byte that the bitfield starts in,
            ** and the offset in bits to the start of the field.
            */
            s_nearest_byte = s_base + (s_pixel_offset >> BYTE_SHIFT);
            s_bit_offset = s_pixel_offset & 7;

	    /* Pixel length is the number of bits to be moved */
	    bits_remaining = srcudp->UdpW_PixelLength;

	    /*
	    ** Shift the (high) bits of interest out of the byte 
	    ** and into the destination.
	    */
	    *dst_byte_ptr = ((*s_nearest_byte++ >> s_bit_offset)
				& lowbits_mask[ bits_remaining ]);
	    /*
	    ** How many bits are remaining, and where in the return value
	    ** is the next portion to go?
	    */
	    bits_remaining -= (BYTE_SIZE - s_bit_offset);

	    /*
	    ** Put the bits from the lower portion of the last byte
	    */
	    if ( bits_remaining >= 0 )
	        *dst_byte_ptr |= (*s_nearest_byte 
			 & lowbits_mask[ bits_remaining ]) << 
			    (BYTE_SIZE - s_bit_offset);

	    dst_byte_ptr += dst_pixel_stride_bytes;
	    s_pixel_offset += srcudp->UdpL_PxlStride;
	    }
        dst_byte_ptr += dst_pad;
        s_line_offset += srcudp->UdpL_ScnStride;
        }
    } /* end of _IpsUnalignedToAlignedByte*/

/******************************************************************************
**  _IpsUnalignedToAlignedWord()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits to a Word Aligned format destination a bitfield at a time.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsUnalignedToAlignedWord( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    { 
    /* 
    ** Could be Pixel stride aligned.
    ** Need to special case this case as well.
    ** Can move the  bits and bump by word.
    */
    /* i.e. 12 bits padded.*/

    unsigned short int *s_base;			/* Source base address      */
    unsigned short int *dst_word_ptr;		/* Destination word address */
    unsigned short int *s_nearest_word;	        /* Source Nearest word      */
    long s_bit_offset;				/* Source bit offset        */
    long dst_pixel_stride_words;	/* Destination pixel stride         */
    long dst_pad;			/* Destination scanline pad         */
    long s_line_offset;			/* Source scanline offset           */
    long s_pixel_offset;		/* Source pixel offset              */
    long bits_remaining;		/* Number of bits remaining of pixel*/
    long iy;				/* Scanline loop counter variable   */
    long ix;				/* Pixel loop counter variable      */

    s_base = (unsigned short int *)srcudp->UdpA_Base;

    /* Get the offset in bits to the first pixel */
    s_bit_offset = (srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride));

    /* Get the destination word pointer of where to place the data */
    dst_word_ptr = (unsigned short int *)dstudp->UdpA_Base + 
		    ((dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> WORD_SHIFT);

    /* Initialize the source line offset to be the bit offset to first pixel */
    s_line_offset = s_bit_offset;

    /* Get the destination pixel stride in words*/
    dst_pixel_stride_words = dstudp->UdpL_PxlStride >> WORD_SHIFT;      

    /* Get the scanline padding in words */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> WORD_SHIFT);


    /*
    ** Outer loop that steps through lines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
	/* Initialize the pixel offset to be the current line offset */
        s_pixel_offset = s_line_offset;
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
            /*
            ** Calculate the address of the byte that the bitfield starts in,
            ** and the offset in bits to the start of the field.
            */
            s_nearest_word = s_base + (s_pixel_offset >> WORD_SHIFT);
            s_bit_offset = s_pixel_offset & 7;

	    bits_remaining = srcudp->UdpW_PixelLength;

	    /*
	    ** Shift the (high) bits of interest out of the byte 
	    ** and into the destination.
	    */
	    *dst_word_ptr = ((*s_nearest_word++ >> s_bit_offset)
				& lowbits_wordmask[ bits_remaining ]);
	    /*
	    ** How many bits are remaining, and where in the return value
	    ** is the next portion to go?
	    */
	    bits_remaining -= (WORD_SIZE - s_bit_offset);

	    /*
	    ** Put the bits from the lower portion of the last byte
	    */
	    if ( bits_remaining >= 0 )
	        *dst_word_ptr |= (*s_nearest_word
			 & lowbits_wordmask[ bits_remaining ]) << 
			    (WORD_SIZE - s_bit_offset);

	    dst_word_ptr += dst_pixel_stride_words;  /* Advance to next pixel */
	    s_pixel_offset += srcudp->UdpL_PxlStride;/*         "            */

	    }
	/* Advance to next scanline */
        dst_word_ptr += dst_pad;
        s_line_offset += srcudp->UdpL_ScnStride;
        }
    } /* end of _IpsUnalignedToAlignedWord*/

/******************************************************************************
**  _IpsUnalignedToAlignedLong()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Moves bits to a Long Aligned format destination a bitfield at a time.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsUnalignedToAlignedLong( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    { 
    unsigned int *s_base;
    unsigned int *dst_long_ptr;
    unsigned int *s_nearest_long;
    long s_bit_offset	= (srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride));
    long dst_pixel_stride_longs;
    long dst_pad;
    long s_line_offset;
    long s_pixel_offset;
    long bits_remaining;
    long iy;
    long ix;
    
    s_base = (unsigned int *)srcudp->UdpA_Base;

    dst_long_ptr = (unsigned int *)dstudp->UdpA_Base + 
		    ((dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride)) >> LONG_SHIFT);
    /*
    ** Set up the scanline line and pixel loops.
    */
    s_line_offset = s_bit_offset;

    /* Get the destination pixel stride in longwords*/
    dst_pixel_stride_longs = dstudp->UdpL_PxlStride >> LONG_SHIFT;      

    /* Get the scanline padding in long words */
    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> LONG_SHIFT);

    /*
    ** Outer loop that steps through lines
    */

    for (iy = 0;iy < srcudp->UdpL_ScnCnt; iy++)
        {
        s_pixel_offset = s_line_offset;
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
            /*
            ** Calculate the address of the byte that the bitfield starts in,
            ** and the offset in bits to the start of the field.
            */
            s_nearest_long = 
			s_base + (s_pixel_offset >> LONG_SHIFT);
            s_bit_offset = s_pixel_offset & 7;

	    bits_remaining = srcudp->UdpW_PixelLength;

	    /*
	    ** Shift the (high) bits of interest out of the byte 
	    ** and into the destination.
	    */
	    *dst_long_ptr = ((*s_nearest_long++ >> s_bit_offset)
				& lowbits_longmask[ bits_remaining ]);
	    /*
	    ** How many bits are remaining, and where in the return value
	    ** is the next portion to go?
	    */
	    bits_remaining -= (LONG_SIZE - s_bit_offset);

	    if ( bits_remaining >= 0 )
	        *dst_long_ptr |= (*s_nearest_long
			 & lowbits_longmask[ bits_remaining ]) << 
			    (LONG_SIZE - s_bit_offset);

	    /* Advance to next pixel */
	    dst_long_ptr += dst_pixel_stride_longs;
	    s_pixel_offset += srcudp->UdpL_PxlStride;

	    }
	/* Advance to next scanline */
        dst_long_ptr += dst_pad;
        s_line_offset += srcudp->UdpL_ScnStride;
        }
    } /* end of _IpsUnalignedToAlignedLong*/

/******************************************************************************
**  _IpsUnalignedToUnaligned()
**
**  FUNCTIONAL DESCRIPTION:
** 
**	The Destination is Unaligned. 
**
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	long (status)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _IpsUnalignedToUnaligned( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    long status = IpsX_SUCCESS;
    /* 
    ** Could be Pixel stride aligned.
    ** Need to special case this.
    ** Can move the  bits and bump by dtype.
    */
    /* 
    ** Could be padded source and destination pixels.
    ** i.e.  3,3,2 destination image, and interleaved by plane source.
    ** (Pixel stride aligned).
    ** Need to special case this.
    ** Can move the three bits and bump by byte.
    */
    if (((dstudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
        && (dstudp->UdpL_PxlStride == BYTE_SIZE) &&
	 ((srcudp->UdpL_ScnStride % BYTE_SIZE ) == 0)
        && (srcudp->UdpL_PxlStride == BYTE_SIZE) &&
	(srcudp->UdpW_PixelLength == dstudp->UdpW_PixelLength ) &&
	(((srcudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= BYTE_SIZE) &&
	(((dstudp->UdpL_Pos % BYTE_SIZE) + srcudp->UdpW_PixelLength) <= BYTE_SIZE))

        /* byte aligned, just non-aligned pixel length */
        /* Pos can be unaligned too because that's what is used
        ** to get to the second and third plane data
        */       
	{
	_IpsBytePaddedToPadded( srcudp, dstudp );
	}
    else
	{

	/* If this is not running on a little endian machine. 
	** This won't work for data pixel length greater than a byte.
	*/
/* 
#ifndef __MIPSEL
        if (srcudp->UdpL_PxlStride > BYTE_SIZE)
            return (IpsX_UNSOPTION);
#endif
*/
        _IpsMoveBitByBit(srcudp,dstudp);
	}
    return (status);
    } /* End of _IpaUnalignedToUnaligned */

/******************************************************************************
**  _IpsBytePaddedToPadded()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Move data from source udp which has pixel padding to destination
**	with pixel padding.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

void _IpsBytePaddedToPadded( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned char *s_base = srcudp->UdpA_Base;  /* Source base pointer      */
    unsigned char *d_base = dstudp->UdpA_Base;	/* Destination base pointer */
    unsigned char *s_nearest_byte;		/* Source nearest byte      */
    unsigned char *d_nearest_byte;		/* Destination nearest byte */
    long s_bit_offset;		      /* Source bit offset to pixel to move */
    long d_bit_offset;		      /* Destination bit offset to pixel    */
    long d_pixel_offset;	      /* Destination pixel offset           */
    long s_pixel_offset;	      /* Source pixel offset                */
    long src_stride;		      /* Source scanline stride             */
    long src_pad;		      /* Source scanline padding            */
    long dst_stride;		      /* Destination scanline stride        */
    long dst_pad;		      /* Destination scanline padding       */
    long iy;			      /* Scanline loop counter variable     */
    long ix;			      /* Pixel loop counter variable        */
    long src_pixel_stride_bytes;      /* Src pixel stride in bytes         */
    long dst_pixel_stride_bytes;      /* Dst pixel stride in bytes         */

    /* Get the source bit offset of the first pixel */
    s_bit_offset = (srcudp->UdpL_Pos +	
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride));

    /* Get the bit offset of where to put the first pixel in the destination */
    d_bit_offset = (dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride));

    /* Get the scanline padding in bytes  */
    src_pad = ((srcudp->UdpL_ScnStride - 
		(srcudp->UdpL_PxlStride * srcudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    dst_pad = ((dstudp->UdpL_ScnStride - 
		(dstudp->UdpL_PxlStride * dstudp->UdpL_PxlPerScn) ) >> BYTE_SHIFT);

    /* Get the destination pixel offset in bytes */
    d_pixel_offset = d_bit_offset >> BYTE_SHIFT;

    /* Get the pixel offset in bits */
    d_bit_offset = d_bit_offset & 7;

    /* Get the source pixel offset in bytes */
    s_pixel_offset = s_bit_offset >> BYTE_SHIFT;

    /* Get the source bit offset */
    s_bit_offset = s_bit_offset & 7;

    /* Get the nearest byte addresses of the source and destination */
    d_nearest_byte = d_base + d_pixel_offset;
    s_nearest_byte = s_base + s_pixel_offset; 

    dst_pixel_stride_bytes = (dstudp->UdpL_PxlStride >> BYTE_SHIFT);
    src_pixel_stride_bytes = (srcudp->UdpL_PxlStride >> BYTE_SHIFT);

    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0; iy < srcudp->UdpL_ScnCnt; iy++)
        {
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
            {
	    /*
	    ** Shift the (high) bits of interest out of the byte 
  	    ** and into the destination.
	    */
            *d_nearest_byte = (*d_nearest_byte & lowbits_mask[d_bit_offset])|
		 ((((*s_nearest_byte >> s_bit_offset) 
		    & lowbits_mask[srcudp->UdpW_PixelLength])
		 << d_bit_offset) & highbits_mask[d_bit_offset]) |
	    	(*d_nearest_byte & 
		    highbits_mask[srcudp->UdpW_PixelLength + d_bit_offset]);

	    /* Advance the source and destination pixel bytes */
	    s_nearest_byte += src_pixel_stride_bytes;
	    d_nearest_byte += dst_pixel_stride_bytes;
	    }
	/* Advance the source and destination by the padding if any. */
        s_nearest_byte += src_pad;
        d_nearest_byte += dst_pad;
        }
    } /* end of _IpsBytePaddedToPadded*/

/******************************************************************************
**  _IpsMoveBitByBit()
**
**  FUNCTIONAL DESCRIPTION:
**
**	There is no Aligned format involved, we must move the data
**      bit by bit. If the data is longer than a byte, it will move
**	the middle byte by byte when possible.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source data, by UDP.
**
**	dstudp	    Destination data, by UDP.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _IpsMoveBitByBit( srcudp, dstudp )
struct UDP  *srcudp;
struct UDP  *dstudp;
    {
    unsigned char *d_base = dstudp->UdpA_Base;  /* destination base pointer */
    unsigned char *s_base = srcudp->UdpA_Base;	/* source base pointer      */
    unsigned long return_value = 0;		/* temp. var. for pixel val.*/
    unsigned char *d_nearest_byte;		/* dest nearest byte ptr    */
    unsigned char *s_nearest_byte;		/* source nearest byte ptr  */
    long d_bit_offset;			        /* destination bit offset   */
    long s_bit_offset;				/* source bit offset        */
    long d_line_offset	= 0;			/* destination line offset  */
    long d_pixel_offset	= 0;			/* destination pixel offset */
    long s_line_offset	= 0;			/* source line offset       */
    long s_pixel_offset	= 0;			/* source pixel offset      */
    long s_bits_remaining;			/* source bits remaining    */
    long d_bits_remaining;			/* destination bits remaining*/
    long retval_offset;				/* return value offset       */
    long ix;					/* Pixel count loop variable */
    long iy;					/* Line count loop variable  */


    /* Get the bit offset to the first pixel in the destination */
    d_bit_offset = (dstudp->UdpL_Pos + 
                    (dstudp->UdpL_ScnStride * dstudp->UdpL_Y1) +
                    (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride));

    /* Get the bit offset to the first pixel in the source */
    s_bit_offset = (srcudp->UdpL_Pos +
                    (srcudp->UdpL_ScnStride * srcudp->UdpL_Y1) +
                    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride));

    /* Initialize the source and destination line offsets to the bit offset */
    d_line_offset = d_bit_offset;
    s_line_offset = s_bit_offset;

    /*
    ** Outer loop that steps through scanlines
    */
    for (iy = 0;iy <  srcudp->UdpL_ScnCnt; iy++)
        {
	/* Initialize the pixel offsets to the current line offset */
        d_pixel_offset = d_line_offset;
        s_pixel_offset = s_line_offset;
        /*
        ** Inner loop that steps through pixels
        */
        for ( ix = 0; ix < srcudp->UdpL_PxlPerScn; ix++ )
	    {
            /*
            ** Calculate the address of the byte that the bitfield starts in,
            ** and the offset in bits to the start of the field.
            */
            d_nearest_byte = d_base + (d_pixel_offset >> BYTE_SHIFT);
            d_bit_offset = d_pixel_offset & 7;

            s_nearest_byte = s_base + (s_pixel_offset >> BYTE_SHIFT);
            s_bit_offset = s_pixel_offset & 7;

	    s_bits_remaining = srcudp->UdpW_PixelLength;
	    d_bits_remaining = dstudp->UdpW_PixelLength;

	    /*
	    ** Shift the (high) bits of interest out of the byte 
	    ** and into the destination.
	    */
	    if (s_bits_remaining > BYTE_SIZE)
	        return_value = (*s_nearest_byte++ >> s_bit_offset)
                                & lowbits_mask[ BYTE_SIZE];
	    else
	        return_value = (*s_nearest_byte++ >> s_bit_offset)
                                & lowbits_mask[ s_bits_remaining ];

                             
	    /*
	    ** How many bits are remaining, and where in the return value
	    ** is the next portion to go?
	    */
	    s_bits_remaining -= (BYTE_SIZE - s_bit_offset);
	    retval_offset = (BYTE_SIZE - s_bit_offset);

	    while ( s_bits_remaining >= BYTE_SIZE )
     	        {
		return_value |= *s_nearest_byte++ << retval_offset;
	        retval_offset += BYTE_SIZE;
	        s_bits_remaining -= BYTE_SIZE;
	        }

	    /*
	    ** Put the bits from the lower portion of the last byte
	    */
            if ( s_bits_remaining >= 0 )
		return_value |= (*s_nearest_byte
                         & lowbits_mask[ s_bits_remaining ]) << retval_offset;

	    /* now stuff the return value in the dest */

	    *d_nearest_byte = (*d_nearest_byte & lowbits_mask[d_bit_offset]) |
	      ((return_value << d_bit_offset) & highbits_mask[d_bit_offset]) | 
	      (*d_nearest_byte & 
		highbits_mask[(d_bits_remaining + d_bit_offset) >= BYTE_SIZE ? BYTE_SIZE : 
		    (d_bits_remaining + d_bit_offset)]);

	    *d_nearest_byte++;

	    d_bits_remaining -= (BYTE_SIZE - d_bit_offset);
	    return_value >>= (BYTE_SIZE - d_bit_offset);

	    while (d_bits_remaining >= BYTE_SIZE)
	        {
		*d_nearest_byte++ = (unsigned char)return_value;
		return_value >>= BYTE_SIZE;
		d_bits_remaining -= BYTE_SIZE;
		}

	    if (d_bits_remaining > 0)
		*d_nearest_byte = (*d_nearest_byte & 
		    highbits_mask[d_bits_remaining]) + return_value;

	    d_pixel_offset += dstudp->UdpL_PxlStride;
	    s_pixel_offset += srcudp->UdpL_PxlStride;
	    }
        d_line_offset += dstudp->UdpL_ScnStride;
        s_line_offset += srcudp->UdpL_ScnStride;
        }
    } /* end of _IpsMoveBitByBit */
