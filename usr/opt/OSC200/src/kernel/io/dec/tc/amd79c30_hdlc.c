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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: amd79c30_hdlc.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/15 17:39:58 $";
#endif

/*
 * OLD HISTORY
 * Revision 1.1.2.6  92/11/18  10:36:40  Narayan_Mohanram
 * 	"Changes for HDLC and bug fixes"
 * 
 * Revision 1.1.4.2  92/11/11  14:46:15  Narayan_Mohanram
 * 	Fixed the bits that is robbed in 56KB-narayan
 * 
 * Revision 1.1.2.5  92/08/19  16:08:12  Narayan_Mohanram
 * 	Updated for alpha changes (None..)
 * 	[92/08/19  15:57:05  Narayan_Mohanram]
 * 
 * Revision 1.1.2.4  92/08/03  14:04:28  Narayan_Mohanram
 * 	Fixing ODE branch screw-ups -narayan
 * 	[92/08/03  13:49:47  Narayan_Mohanram]
 * 
 * Revision 1.1.3.3  92/06/22  10:59:42  Narayan_Mohanram
 * 	Added support for 56KB connections -- narayan
 * 
 * Revision 1.1.3.2  92/06/22  10:53:15  Narayan_Mohanram
 * 	Added software hdlc algorithms for 56KB connections -- narayan
 * 
 * Revision 1.1.2.2  92/04/19  17:06:55  Ron_Bhanukitsiri
 * 	"BL2 - Bellcore Certification"
 * 	[92/04/19  17:05:50  Ron_Bhanukitsiri]
 */



#ifdef TESTING
#include <sys/types.h>
#include "hdlc.h"
#else
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <io/dec/tc/amd79c30_hdlc.h>
#endif

extern unsigned short hdlc_fcstab_bba [];
struct hdlc_debug {
	int in_flags;
	int frame_abort;
	int in_flags_seq;
	int reset;
	int frame_complete;
	int short_frame;
	int bad_fcs;
	int good_fcs;
	int out_ones;
	int out_zeros;
	int in_ones;
	int in_zeros;
} hdlc_bba_stats;
#define INCREMENT(x) (hdlc_bba_stats.x++)
#define H_SEND_BADFRAMES 1
#define H_SEND_ALL 2
int bba_hdlc_debug, hdlc_bba_custom;
/*
 * FCS routine for incoming packets. Look at it 1 octet at a time
 */
unsigned short
hdlc_fcs_bba (fcs, cp, len)
	register unsigned char *cp;
	register int len;
	register unsigned short fcs;
{

	while (len-- > 0) {
		fcs = (fcs >> 8) ^ hdlc_fcstab_bba [(fcs ^ *cp++) & 0xff];
		fcs &= 0xffff;
	}
	return fcs;
}
/*
 * Reset. probably due to incoming frame abort.
 */
void
hdlc_reset_bba (hi)
	struct hdlc_info *hi;
{
	hi->state = HDLCS_FLAG_SEARCH;
	hi->num_ones = hi->len = 0;
	INCREMENT(reset);
}
/*
 * Frame flag was received. This may be only the start of a frame
 * or it may be just channell idle and sending `flags's to idle.
 */
hdlc_frame_complete_bba (hi, func, arg)
    register struct hdlc_info *hi;
    int (*func) ();
    caddr_t arg;
{
	register int crc;
	/*
	 * Each frame must contain at least 32 bits for it to be valid
	 * Dump the 7 bits of flag that was accumulated so far.
	 */
	INCREMENT (frame_complete);
	if ((hi->len -= (BITS_PER_OCTET - 1)) < 32) {
		BBA_HDLC_DEBUG (HD_BADFRAME,
			("bba_framecomplete: %d bits(short frame)\n", hi->len));
		INCREMENT (short_frame);
		goto done;
	}
	/*
	 * Frame is complete. We have more than 32 bits, and it must
	 * be validated and sent to the host.
	 */
	BBA_HDLC_DEBUG (HD_COMPLETE,
		("bbaFrameCompete len %d bits (%d bytes) %s Boundary\n",
		  hi->len, hi->len >> 3, (hi->len & 7) ? "Bad" : "OK"));
	/*
	 * Check and make sure that the FCS is OK
	 */
	if ((crc = hdlc_fcs_bba ((unsigned short)HDLC_INITFCS, hi->base, 
			     hi->len / 8)) != HDLC_GOODFCS)
	{
		BBA_HDLC_DEBUG (HD_BADFRAME, ("bba:Bad input Fcs %x\n", crc));
		INCREMENT (bad_fcs);
		if ((hdlc_bba_custom & H_SEND_BADFRAMES) == 0)
			goto done;
		else
			goto Send;
	} else {
		BBA_HDLC_DEBUG (HD_FRAME, ("bba:Got Good Fcs %x\n", crc));
		INCREMENT (good_fcs);
	}
	hi->len -= 16; /* Dump the FCS */
#ifdef XTESTING
	/*
	 * Signal the host that we have a frame ready for them.
	 * Note that the CRC is the last 2 octets of this frame.
	 */
	{
		write (2, &hi->base [0], hi->len/8);
	}
#else
	/*
	 * Receive Done.. Send this up to the user..
	 * If the sender could not alloc a new
	 * buffer, then we have to abort..
	 */
	Send:
	if ((*func) (arg) == 0)
		return 0;
#endif /* TESTING */
	done:
	hi->len = hi->num_ones = 0;
	hi->state = HDLCS_GOT_FLAG;
	return 1;
}
/*
 * Got len octets from the remote. Add this to the bits we have
 * accumulated so far. If we have a flag, deal with frame completion
 * This routine is optimized for the MAXine memory map. Function calls
 * are expanded in line, Also the function is more convoluted in terms
 * of creating masks... just so that fast code is generated. This is
 * NOT optimized for space.
 * The input src has one octet every 4 bytes..
 */
void
hdlc_recv_bba (hi, src, len, func, arg)
    register struct hdlc_info *hi;
    register char *src;
    register int len;
    int (*func) ();
    caddr_t arg;
{
	register int c, mask, in_mask, bit, last_oct = 0, is_flag = 0;
	register char *dst;

	/*
	 * On the MAXine we get the data which was memory mapped
	 * This data is available every 4th byte for this specific channell
	 */
	dst = (char *)hi->base + (hi->len >> 3);
	if (hi->len == 0)
		*dst = 0;
	CREAT_BBA_IN_BIT_MASK (hi, in_mask);
	BBA_HDLC_DEBUG (HD_RECV, ("hdlc_recv_bba: hi->len %d cp %x num-ones %d len %d mask %x <%02x><%02x><%02x><%02x>\n",
		hi->len, src, hi->num_ones, len, in_mask,
		src [0], src [4], src [8], src [12]));
	if (hdlc_bba_custom & H_SEND_ALL) {
		hi->len = (len * 8)*4;
		bcopy (src, dst, len *4); /* Take all the 4 bytes */
		(*func) (arg);
		return;

	}
	while (len-- > 0) {
		c = *src & 0xff;
		src += 4;
		if (is_flag) {
			if (c == last_oct) {
				INCREMENT (in_flags_seq);
				goto Next_Oct;
			} else {
				/*
				 * This is changing from flag to data..
				 * the hi->len already contains part of
				 * the last octet of the started the
				 * flags (after hdlc_frame_complete
				 * we processed the rest of the octet)
				 */
				is_flag = 0;
			} 
		}
		last_oct = c;
		/*
		 * Scan from low-order-bit to high-order bit. Keep a mask
		 * to create the output-octet. This octet is stored Low-Order
		 * bit to high order bit. The AM79C30A can transmit LSB first.
		 */
		for (mask = 0x80, bit = 0; mask; mask >>= 1, bit++) {
			if (c & mask) {
				/*
				 * Got a 1 bit.
				 */
				if (hi->state == HDLCS_FLAG_SEARCH)
					/*
					 * Looking for a frame start.
					 * This is not it.
					 */
					continue;
				if (++hi->num_ones >= 7) {
					INCREMENT (frame_abort);
					hdlc_reset_bba (hi); /* Frame Abort.. */
					is_flag = 0;
					in_mask = 1;
					*(dst = (char *)hi->base) = 0;
				} else if (hi->state != HDLCS_GOT_ZERO) {
					IN_BBA_DST_MASK_BIT(hi,dst,in_mask);
				}
			} else {
				/*
				 * Got a zero bit. If we are searching for a
				 * zero to start a flag (first time and after a
				 * frame abort) specify that we got a zero.
				 * If we have 6 ones aleady, then we now have a
				 * flag. frame_complete will deal with both
				 * silence (flags) and complete frames.
				 */
				if (hi->state == HDLCS_FLAG_SEARCH) {
					hi->state = HDLCS_GOT_ZERO;
				} else	if (hi->num_ones == 6) {
					/*
					 * Got a frame start/end. Construct the
					 * next flag that can follow this..
					 */
					if (!hdlc_frame_complete_bba (hi, 
								 func, arg))
					{
						hdlc_reset_bba (hi);
						return;
					}
					/*
					 * The new buffer is on a new boundary
					 * So we have to reset the mask..
					 */
					*(dst = (char *)hi->base) = 0;
					in_mask = 1;
					is_flag = 1;/* for next time */
					last_oct = (0x7e7e >> (bit+1))
						& 0xff;
				} else if (hi->num_ones != 5 && 
					   hi->state == HDLCS_GOT_FLAG)
				{
					/*
					 * We already have a flag, and are
					 * gathering a new frame. We dump a
					 * zero bit after 5 ones
					 */
					IN_BBA_DST_NOT_MASK_BIT (hi, dst,
								 in_mask);
				}
				hi->num_ones = 0;
			} /** if (c & mask) **/
		} /*** for (mask= 0x80) ***/
		While_Cont:
		if (hi->len >= HDLC_MAX_FRAME) {
			BBA_HDLC_DEBUG (HD_BADFRAME,
				("bba:Frame Too long aborting\n"));
			hdlc_reset_bba (hi);
		}
		Next_Oct:;
	} /* while len-- */
}
/*
 * This is for adding flags (No bit stuffing here). All 8 bits are added to
 * the end of the bits we have so far accumulated.
 */
void
hdlc_add_flags_bba (hi, c)
    struct hdlc_info *hi;
    unsigned int c;
{
	register int nbits;
	register char *dst;

	c &= 0xff;
	dst = (char *)&BBA_OCTET_CHANGED (hi);
	BBA_HDLC_DEBUG (HD_FLAG_ENCODE, 
		("hdlc_add_flags_bba:c %x len %d dst %x\n", c, hi->len, dst));
	if (nbits = (hi->len & 7)) {
		/*
		 * The bits are going to overflow into the next octet.
		 * so put in the (8 - bits_remaining) bits into the
		 * first octet, and the rest into the next octet.
		 * Remember that this flag gets reversed..
		 */
		dst [0] |= (c >> nbits);
		dst [1] = (c << (8 - nbits));
	} else
		*dst = c;
	hi->len += 8;
}
/*
 * Encode a buf with `len' specified in octets. The resultant outgoing
 * buffer has:
 * 8 bits of flag 0x7e (not bit-stuffed)
 * n bits of the buf (bit-stuffed)
 * FCS 16 bits but they are bit-stuffed.
 * 8 bits of flag (not bit-stuffed)
 * All this is written to a linear buffer.
 */
void
hdlc_encode_bba (hi, src, len, add_header, no_trailer)
    register struct hdlc_info *hi;
    register unsigned char *src;
    register int len;
    int add_header, no_trailer;
{
	register int c, mask, out_shift;
	register unsigned short crc;
	unsigned char frame_trailer [4];
	register char *dst;

	if (add_header) {
		hdlc_add_flags_bba (hi, HDLC_FLAG, 1);
		crc = HDLC_INITFCS; /* Initial value.. */
		hi->num_ones = 0;
	} else {
		crc = hi->fcs;
	}
	dst = (char *)&BBA_OCTET_CHANGED (hi);
	out_shift = 0x80 >> (hi->len & 7);
	BBA_HDLC_DEBUG (HD_ENCODE,
		("bba_encode:hi %x len %d dst %x hi->len %d head %d trail %d",
		  hi, len, dst, hi->len, add_header, no_trailer));
	BBA_HDLC_DEBUG (HD_ENCODE,
		(" num_ones %d shift %x\n", hi->num_ones, out_shift));
	BBA_HDLC_DEBUG (HD_ENCODE,
		("bba_encode:SRC %x.%x.%x.%x\n",src[0],src[1],src[2],src[3]));
	while (len-- > 0) {
		/*
		 * Add each byte and accumulate the CRC
		 */
		c = *src++;
		/*
		 * Expanded the hdlc_add_output here (to reduce call overhead)
		 * We have to tranmit it low-order(LSbit) to high order
		 * On the AM79C30 however each octet is transmitted High Order
		 * Bit first. So we have to scan from LSB to HSB and
		 * Construct it on the outgoing memory as HSB to MSB
		 */
		for (mask = 0x1; mask != 0x100; mask <<= 1) {
			/*
			 * Examine this octet 1 bit at a time.
			 */
			if (c & mask) {
				/*
				 * Got a 1 bit.
				 */
				OUT_BBA_MASK_BIT (hi, dst, out_shift)
				if (++hi->num_ones == 5) {
					OUT_BBA_NOT_MASK_BIT (hi,dst,out_shift);
					hi->num_ones = 0;
				}
			} else {
				/*
				 * Got a zero bit.
				 */
				OUT_BBA_NOT_MASK_BIT (hi, dst, out_shift);
				hi->num_ones = 0;
			}
		}
		/*
		 * Accumulate the CRC.
		 */
		crc = (crc >> 8) ^ hdlc_fcstab_bba [(crc ^ c) & 0xff];
	}
	hi->fcs = crc;
	if (no_trailer)
		return;
	/*
	 * Stuff the CRC for this buffer at the end. The CRC goes out
	 * low order octet first!! (go figure)
	 * The CRC is bit-stuffed obviously, but not the flag.
	 */
	crc ^= 0xffff;
	frame_trailer [0] = crc; frame_trailer [1] = crc >> 8;
	hdlc_encode_bba(hi, &frame_trailer[0], 2, 0/*no-head*/, 1/*no-trailer*/);
	/*
	 * Now add 2 flags at the end..(So that we know the fill pattern latter)
	 */
	hdlc_add_flags_bba (hi, HDLC_FLAG, 1);
	hdlc_add_flags_bba (hi, HDLC_FLAG, 1);
	hi->num_ones = 0;
	hi->flag = (0x7e7e >> (hi->len % 8)) & 0xff;
}
/*
 * Got len octets from the remote. Add this to the bits we have
 * accumulated so far. If we have a flag, deal with frame completion
 * This routine is optimized for the MAXine memory map. Function calls
 * are expanded in line, Also the function is more convoluted in terms
 * of creating masks... just so that fast code is generated. This is
 * NOT optimized for space.
 * The input src has one octet every 4 bytes..
 * We can only use 7 bits out of every octet.
 */
void
hdlc_recv_bba_56KB (hi, src, len, func, arg)
    register struct hdlc_info *hi;
    register char *src;
    register int len;
    int (*func) ();
    caddr_t arg;
{
	register int c, mask, in_mask;
	register char *dst;

	/*
	 * On the MAXine we get the data which was memory mapped
	 * This data is available every 4th byte for this specific channell
	 */
	dst = (char *)hi->base + (hi->len >> 3);
	if (hi->len == 0)
		*dst = 0;
	CREAT_BBA_IN_BIT_MASK (hi, in_mask);
	BBA_HDLC_DEBUG (HD_RECV, ("hdlc_recv_bba: hi->len %d cp %x num-ones %d len %d mask %x <%02x><%02x><%02x><%02x>\n",
		hi->len, src, hi->num_ones, len, in_mask,
		src [0], src [4], src [8], src [12]));
	if (hdlc_bba_custom & H_SEND_ALL) {
		hi->len = len * 8;
		while (len > 0) {
			*dst++ = *src;
			src += 4;
			len--;
		}
		(*func) (arg);
		return;

	}
	while (len-- > 0) {
		c = *src & 0xff;
		src += 4;
		/*
		 * Scan from low-order-bit to high-order bit. Keep a mask
		 * to create the output-octet. This octet is stored Low-Order
		 * bit to high order bit. The AM79C30A can transmit LSB first.
		 */
		for (mask = HDLC_56KB_HIGHBIT; 
			mask != HDLC_56KB_LOWBIT; mask >>= 1)
		{
			if (c & mask) {
				/*
				 * Got a 1 bit.
				 */
				if (hi->state == HDLCS_FLAG_SEARCH)
					/*
					 * Looking for a frame start.
					 * This is not it.
					 */
					continue;
				if (++hi->num_ones >= 7) {
					INCREMENT (frame_abort);
					hdlc_reset_bba (hi); /* Frame Abort.. */
					*(dst = (char *)hi->base) = 0;
					hi->state = HDLCS_FLAG_SEARCH;
					in_mask = 1;
				} else if (hi->state != HDLCS_GOT_ZERO) {
					IN_BBA_DST_MASK_BIT(hi,dst,in_mask);
				}
			} else {
				/*
				 * Got a zero bit. If we are searching for a
				 * zero to start a flag (first time and after a
				 * frame abort) specify that we got a zero.
				 * If we have 6 ones aleady, then we now have a
				 * flag. frame_complete will deal with both
				 * silence (flags) and complete frames.
				 */
				if (hi->state == HDLCS_FLAG_SEARCH) {
					hi->state = HDLCS_GOT_ZERO;
				} else	if (hi->num_ones == 6) {
					/*
					 * Got a frame start/end. All frames
					 * must have at least 32 bits.
					 */
					if (hi->len < (32+(BITS_PER_OCTET -1)))
					{
						hi->len = hi->num_ones = 0;
						hi->state = HDLCS_GOT_FLAG;
						
					} else if (!hdlc_frame_complete_bba
							(hi, func, arg))
					{
						hdlc_reset_bba (hi);
						return;
					}
					/*
					 * The new buffer is on a new boundary
					 * So we have to reset the mask..
					 */
					*(dst = (char *)hi->base) = 0;
					in_mask = 1;
				} else if (hi->num_ones != 5 && 
					   hi->state == HDLCS_GOT_FLAG)
				{
					/*
					 * We already have a flag, and are
					 * gathering a new frame. We dump a
					 * zero bit after 5 ones
					 */
					IN_BBA_DST_NOT_MASK_BIT (hi, dst,
								 in_mask);
				}
				hi->num_ones = 0;
			} /** if (c & mask) **/
		} /*** for (mask= 0x80) ***/
		While_Cont:
		if (hi->len >= HDLC_MAX_FRAME) {
			BBA_HDLC_DEBUG (HD_BADFRAME,
				("bba:Frame Too long aborting\n"));
			hdlc_reset_bba (hi);
		}
		Next_Oct:;
	} /* while len-- */
}
/*
 * This is for adding flags (No bit stuffing here). All 8 bits are added to
 * the end of the bits we have so far accumulated.
 */
void
hdlc_add_flags_bba_56KB (hi, num)
    struct hdlc_info *hi;
    register int num;
{
	register int out_shift;
	register char *dst;

	dst = (char *)&BBA_OCTET_CHANGED_56KB (hi);
	out_shift = 1 << BIT_LOCATION_56KB (hi);
	BBA_HDLC_DEBUG (HD_FLAG_ENCODE, 
		("hdlc_add_flags_bba:len %d dst %x out_shift %2x numflags %d\n",
			 hi->len, dst, out_shift, num));
	while (num-- > 0) {
		OUT_BBA_56KB_NOT_MASK_BIT (hi, dst, out_shift);
		OUT_BBA_56KB_MASK_BIT (hi, dst, out_shift) /* 1 */
		OUT_BBA_56KB_MASK_BIT (hi, dst, out_shift) /* 2 */
		OUT_BBA_56KB_MASK_BIT (hi, dst, out_shift) /* 3 */
		OUT_BBA_56KB_MASK_BIT (hi, dst, out_shift) /* 4 */
		OUT_BBA_56KB_MASK_BIT (hi, dst, out_shift) /* 5 */
		OUT_BBA_56KB_MASK_BIT (hi, dst, out_shift) /* 6 */
		OUT_BBA_56KB_NOT_MASK_BIT (hi, dst, out_shift);
	}
	hi->num_ones = 0;
}
/*
 * Encode a buf with `len' specified in octets. The resultant outgoing
 * buffer has:
 * 8 bits of flag 0x7e (not bit-stuffed)
 * n bits of the buf (bit-stuffed)
 * FCS 16 bits but they are bit-stuffed.
 * 8 bits of flag (not bit-stuffed)
 * All this is written to a linear buffer.
 */
void
hdlc_encode_bba_56KB (hi, src, len, add_header, no_trailer)
    register struct hdlc_info *hi;
    register unsigned char *src;
    register int len;
    int add_header, no_trailer;
{
	register int c, mask, out_shift;
	register unsigned short crc;
	unsigned char frame_trailer [4];
	register char *dst;

	if (add_header) {
		hi->base[0] = 0;
		hdlc_add_flags_bba_56KB (hi, 1);
		crc = HDLC_INITFCS; /* Initial value.. */
		hi->num_ones = 0;
	} else {
		crc = hi->fcs;
	}
	dst = (char *)&BBA_OCTET_CHANGED (hi);
	out_shift = 0x80 >> (hi->len & 7);
	BBA_HDLC_DEBUG (HD_ENCODE,
		("bba_encode:hi %x len %d dst %x hi->len %d head %d trail %d",
		  hi, len, dst, hi->len, add_header, no_trailer));
	BBA_HDLC_DEBUG (HD_ENCODE,
		(" num_ones %d shift %x\n", hi->num_ones, out_shift));
	BBA_HDLC_DEBUG (HD_ENCODE,
		("bba_encode:SRC %x.%x.%x.%x\n",src[0],src[1],src[2],src[3]));
	while (len-- > 0) {
		/*
		 * Add each byte and accumulate the CRC
		 */
		c = *src++;
		/*
		 * Expanded the hdlc_add_output here (to reduce call overhead)
		 * We have to tranmit it low-order(LSbit) to high order
		 * On the AM79C30 however each octet is transmitted High Order
		 * Bit first. So we have to scan from LSB to HSB and
		 * Construct it on the outgoing memory as HSB to MSB
		 */
		for (mask = 0x1; mask != 0x100; mask <<= 1) {
			/*
			 * Examine this octet 1 bit at a time.
			 */
			if (c & mask) {
				/*
				 * Got a 1 bit.
				 */
				OUT_BBA_56KB_MASK_BIT (hi, dst, out_shift)
				if (++hi->num_ones == 5) {
					OUT_BBA_56KB_NOT_MASK_BIT (hi,dst,out_shift);
					hi->num_ones = 0;
				}
			} else {
				/*
				 * Got a zero bit.
				 */
				OUT_BBA_56KB_NOT_MASK_BIT (hi, dst, out_shift);
				hi->num_ones = 0;
			}
		}
		/*
		 * Accumulate the CRC.
		 */
		crc = (crc >> 8) ^ hdlc_fcstab_bba [(crc ^ c) & 0xff];
	}
	hi->fcs = crc;
	if (no_trailer)
		return;
	/*
	 * Stuff the CRC for this buffer at the end. The CRC goes out
	 * low order octet first!! (go figure)
	 * The CRC is bit-stuffed obviously, but not the flag.
	 */
	crc ^= 0xffff;
	frame_trailer [0] = crc; frame_trailer [1] = crc >> 8;
	hdlc_encode_bba_56KB(hi, &frame_trailer[0], 2, 
		0/*no-head*/, 1/*no-trailer*/);
	/*
	 * Now add 2 flags at the end..(So that we know the fill pattern latter)
	 */
	hdlc_add_flags_bba_56KB (hi, 3);
	hi->flag = (0x7e7e >> (hi->len % 8)) & 0xff;
}
unsigned short hdlc_fcstab_bba [] = {
/*   0 */	0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
/*   8 */	0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
/*  16 */	0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
/*  24 */	0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
/*  32 */	0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
/*  40 */	0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
/*  48 */	0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
/*  56 */	0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
/*  64 */	0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
/*  72 */	0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
/*  80 */	0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
/*  88 */	0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
/*  96 */	0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
/* 104 */	0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
/* 112 */	0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
/* 120 */	0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
/* 128 */	0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
/* 136 */	0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
/* 144 */	0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
/* 152 */	0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
/* 160 */	0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
/* 168 */	0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
/* 176 */	0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
/* 184 */	0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
/* 192 */	0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
/* 200 */	0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
/* 208 */	0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
/* 216 */	0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
/* 224 */	0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
/* 232 */	0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
/* 240 */	0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
/* 248 */	0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78,
};
