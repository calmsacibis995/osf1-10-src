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
 *                         CPU Hardware Definitions                         *
 ***************************************************************************/

/* ||| All these guys need more detailed ifdefs than they have here. */

/* Do we have a single big page pointing at the frame buffer?  If not, writing
   into the frame buffer may cause TLB faults, and for lines and narrow spans
   it's better to use an extra bus write to the address register.

   MIPS R2000, R3000 have small pages and take lots of faults.
   MIPS R4000, R6000, BIPS, and Alpha all have big pages.
*/

#ifdef mips
#define TLBFAULTS
#endif


/* Do we have direct partial word over the bus?  If not, we'll want to avoid
   a read/modify/write cycle by using TRANSPARENTSTIPPLE mode.  MIPS over
   TURBOChannel has byte writes and halfword writes, BIPS and Alpha don't. 
*/

#ifdef mips
#define PARTIALWRITES
#endif


/* Do shifts work by modulo the wordsize, as with MIPS and Alpha?  Or do they
   zero the word if shift amount is large, like VAX?
*/

#if defined(mips) || defined(__alpha)
#define MODULOSHIFTS
#endif

/* What is natural word size of machine?  (This means register size.) */
#ifndef LONG_BIT
#define LONG_BIT	32
#endif
#if defined(mips)
#define WORDBITS     	32
#else
#define WORDBITS     	LONG_BIT
#endif


/* We want to ensure that write gets to sfb before reading data that
   depends upon the write getting there.  We define two macros:
   SFBFLUSHMODE really writes the mode in all cases; SFBFASTFLUSH does it
   if it is efficient.  (If it isn't efficient to flush, the sfb code
   isn't used; the slower cfb code is called instead.) */


/* ||| This needs to get info from kernel driver. */
#if defined(mips)
/* On MIPS, only 3MAX performs read-around-writes. */
#define SFBFLUSHMODE(sfb, data)					\
{								\
    int i;							\
    SFBMODE(sfb, data);						\
    /* Now make sure it gets out there.	*/			\
    i = ((ws_cpu == DS_5000) ? 6 : 0);				\
    /* ||| Want write to uncached mem but use this for now */   \
    while (i != 0) {						\
	SFBMODE(sfb, data);					\
	i--;							\
    }								\
} /* SFBFLUSHMODE */

#define SFBFASTFLUSH()

#elif defined(__alpha)
#define SFBFLUSHMODE(sfb, data)		SFBMODE(sfb, data);WBFLUSH()
/* SFBFASTFLUSH is only used in the DRAIN code. DRAIN always needs 
 * a flush to start to make sure that stuff is down in the registers
 * before we read them.
 */
#define SFBFASTFLUSH()			WBFLUSH()
#endif	


/* Is the CPU really fast?  At some point we hit the breakover point in text
from painting a fixed number of glyphs each iteration (which bounce all over
in alignment), to painting a variable number of glyphs (which always use 32
bits aligned to 8 bytes).  Don't even know where breakover point is right
now. */


