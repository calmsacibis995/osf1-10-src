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
/*
 * @(#)$RCSfile: ffbcpu.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:08:39 $
 */
/*
 */
/****************************************************************************
 *                         CPU Hardware Definitions                         *
 ***************************************************************************/

/* Which CPU are we talking about? */
#if defined(ALPHA) && !defined(__alpha)
#define __alpha
#endif



/* Do we have a single big page pointing at the frame buffer?  If not, writing
   into the frame buffer may cause TLB faults, and for lines and narrow spans
   it's better to use an extra bus write to the address register.

   MIPS R2000, R3000 have small pages and take lots of faults.
   MIPS R4000, R6000, BIPS, and Alpha all have big pages.
*/

#ifdef mips
#   define TLBFAULTS
#endif


/*
 * WRITE-BUFFER INFORMATION
 *
 *	mips:	write-buffer does not merge.
 *	alpha:	too complicated(lazy) to document here.
 */
#ifdef mips
#   ifdef SOFTWARE_MODEL
#       define CPU_WB_WORDS     8
#   else
#       define CPU_WB_WORDS     0
#   endif
#elif defined(__alpha)
#   define CPU_WB_WORDS		8
#endif

#define CPU_WB_LINES		3	
#define CPU_WB_BYTES		(CPU_WB_WORDS << 2)
#define CPU_WB_LINEMASK		(CPU_WB_BYTES -1)



/* Do we have direct partial word over the bus?  If not, we'll want to avoid
   a read/modify/write cycle by using TRANSPARENTSTIPPLE mode.  MIPS over
   TURBOChannel has byte writes and halfword writes, BIPS and Alpha don't. 

   Don't define PARTIALWRITES for a 32 bit system, because 8-bit unpacked mode
   won't work.  Don't define if using write buffer model. */ 
#if defined(mips) && (FFBPIXELBITS != 32) && (CPU_WB_WORDS == 0)
#   define PARTIALWRITES
#endif



/* Do shifts work by modulo the wordsize, as with MIPS and Alpha?  Or do they
   zero the word if shift amount is large, like VAX?
*/

#if defined(mips) || defined(__alpha)
#   define MODULOSHIFTS
#endif

/* What is natural word size of machine?  (This means register size.) */
#if defined(mips)
#   define WORDBITS     32
#elif defined(__alpha)
#   define WORDBITS     64
#endif

#ifdef __alpha
#   ifdef VMS
#       include "ints.h"
#       define Bits64  uint64 
#       define Pixel64 uint64
#   else /* osf */
#       define Bits64  unsigned long 
#       define Pixel64 unsigned long 
#   endif
#endif 

/*
 * HISTORY
 */
