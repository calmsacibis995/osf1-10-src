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
/****************************************************************************
**                                                                          *
**                       COPYRIGHT (c) 1990, 1991 BY                        *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/

/* Modes to operate SFB in */
#define SIMPLE		    0
#define OPAQUESTIPPLE       1
#define OPAQUELINE	    2
#define TRANSPARENTSTIPPLE  5
#define TRANSPARENTLINE     6
#define COPY		    7

/* Offsets of SFB registers from base */
#define SFBPIXELMASK	    0x2c
#define SFBMODE		    0x30
#define SFBSHIFT	    0x38
#define SFBADDRESS	    0x3c
#define SFBBRES1	    0x40
#define SFBBRES2	    0x44
#define SFBBRES3	    0x48
#define SFBBRESCONTINUE	    0x4c
#define SFBSTART	    0x54

#include "sfbparams.h"
#include "cpu.h"

#define SFBPIXELBYTES    (SFBPIXELBITS / 8)

#if SFBPIXELBITS == 8
#define SFBLINESHIFT		(16 + 0)
#define SFBPIXELSTOBYTES(r)	/* Nothing */
#define SFBBYTESTOPIXELS(r)	/* Nothing */
#elif SFBPIXELBITS == 16
#define SFBLINESHIFT		(16 + 1)
#define SFBPIXELSTOBYTES(r)	sll	r, 1
#define SFBBYTESTOPIXELS(r)	sra	r, 1
#elif SFBPIXELBITS == 32
#define SFBLINESHIFT		(16 + 2)
#define SFBPIXELSTOBYTES(r)	sll	r, 2
#define SFBBYTESTOPIXELS(r)	sra	r, 2
#endif

#if SFBSTIPPLEBITS == 16
#define SFBSTIPPLEALL1      0x0000ffff
#elif SFBSTIPPLEBITS == 32
#define SFBSTIPPLEALL1      0xffffffff
#endif

#define SFBSTIPPLEBITSMASK  (SFBSTIPPLEBITS - 1)
#define SFBSTIPPLEBYTESDONE (SFBSTIPPLEBITS * SFBPIXELBYTES)

#define SFBBUSBITSMASK      (SFBBUSBITS - 1)
#define SFBALIGNMENT	    (SFBVRAMBITS / 8)
#define SFBALIGNMASK	    (SFBALIGNMENT - 1)


#if defined(MODULOSHIFTS) && (SFBSTIPPLEBITS == WORDBITS)
#define SFBLEFTSTIPPLEMASK(mask, align, ones)		\
	sll     mask, ones, align

#define SFBRIGHTSTIPPLEMASK(mask, alignedWidth, ones)   \
	negu    mask, alignedWidth;			\
	srl     mask, ones, mask

#else /* use longer sequences */

#define SFBLEFTSTIPPLEMASK(mask, align, ones)		\
	andi    mask, align, SFBSTIPPLEBITSMASK;	\
	sll     mask, ones, mask

#define SFBRIGHTSTIPPLEMASK(mask, alignedWidth, ones)   \
	negu    mask, alignedWidth;			\
	andi    mask, SFBSTIPPLEBITSMASK;		\
	srl     mask, ones, mask
#endif

#if SFBSTIPPLEBITS == SFBBUSBITS
#define StippleEntireWord(p, srcbits)			\
	sw      srcbits, 0(p)

#elif 2*SFBSTIPPLEBITS == SFBBUSBITS
#define StippleEntireWord(p, srcbits)			\
	sw      srcbits, 0*SFBSTIPPLEBYTESDONE(p);	\
	srl     srcbits, SFBSTIPPLEBITS;		\
	sw      srcbits, 1*SFBSTIPPLEBYTESDONE(p)

#elif 4*SFBSTIPPLEBITS == SFBBUSBITS
#define StippleEntireWord(p, srcbits)			\
	sw      srcbits, 0*SFBSTIPPLEBYTESDONE(p);	\
	srl     srcbits, SFBSTIPPLEBITS;		\
	sw      srcbits, 1*SFBSTIPPLEBYTESDONE(p);      \
	srl     srcbits, SFBSTIPPLEBITS;		\
	sw      srcbits, 2*SFBSTIPPLEBYTESDONE(p);	\
	srl     srcbits, SFBSTIPPLEBITS;		\
	sw      srcbits, 3*SFBSTIPPLEBYTESDONE(p)

#endif
