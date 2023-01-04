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
 * @(#)$RCSfile: kn_wbflush.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:22:22 $
 */

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/regdef.h>
#include <hal/kn02ba.h>		/* for KN02BA_SIRM_K1ADDR */

/*
 * ifdef-ing instructions:
 *	(1) add ifdef/endif DSxxxx around the per-platform routine;
 *	    logically OR multiple DSxxxx's if 1 routine serves
 *	    more than 1 platform.
 *	(2) add ifdef/endif HAL_LIBRARY within the ifdef DSxxxx
 *	    so that the per-platform function name is used for the HAL
 *	    library (used by cpu-switch to the function), and the 
 *	    "wbflush" name is used for the per-platform library.
 */

/*
 * Write buffer flush routine for PMAX and 3MAX.  Routine waits until
 * the write buffer is empty before returning.
 */
#if defined(DS3100) || defined(DS5000)
#ifdef HAL_LIBRARY
LEAF(kn01wbflush)
XLEAF(kn02wbflush)
#else /* per-platform library name */
LEAF(wbflush)
#endif /* HAL_LIBRARY */
1:	
 	bc0f	1b
 	j	ra
#ifdef HAL_LIBRARY
	END(kn01wbflush)
#else /* per-platform library name */
	END(wbflush)
#endif /* HAL_LIBRARY */
#endif /* DS3100 || DS5000 */


/* 
 * Write buffer flush routine for MIPSFAIR 1 and MIPSMATE.
 */
#if defined(DS5100) || defined(DS5400)
#ifdef HAL_LIBRARY
LEAF(kn210wbflush)		/* WB flush routine for R3000	*/
#else /* per-platform library name */
LEAF(wbflush)
#endif /* HAL_LIBRARY */
	.set noreorder
	mfc0	v0,C0_SR	/* v0 = status register */
	li	t0,0x80000000	/* set CU3 bit */
	or	v1,v0,t0	/* v1 = v0 | 0x80000000 */
	nop
	mtc0	v1,C0_SR	/* status register = v1 */
	nop			/* both these nops are needed */
	nop			/* both these nops are needed */
1:	
 	bc3f	1b		/* wait till write buffer empty */
	nop			/* this no op too		*/
	mtc0	v0,C0_SR	/* restore old status register */
	nop
 	j	ra
	nop
	.set reorder
#ifdef HAL_LIBRARY
	END(kn210wbflush)
#else /* per-platform library name */
	END(wbflush)
#endif /* HAL_LIBRARY */
#endif /* DS5100 || DS5400 */


/* 
 * Write buffer flush routine for MIPSFAIR 2.
 *
 * A read of any location that is buffered in the write buffer
 * forces a full write buffer flush.
 */
#ifdef DS5500

/* bss area location that kn220 uses to force a write buffer flush */
LBSS(kn220wbflush_loc,4)

#ifdef HAL_LIBRARY
LEAF(kn220wbflush)		/* WB flush routine for R3000	*/
#else /* per-platform library name */
LEAF(wbflush)
#endif /* HAL_LIBRARY */
	.set noreorder
	la	t0,kn220wbflush_loc
	li	t1, 0xa0000000  /* Make kseg1 address   */
	or      t0,t1
	sw	t1,0(t0)
	lw	t1,0(t0)
	nop
 	j	ra
	nop
	.set reorder
#ifdef HAL_LIBRARY
	END(kn220wbflush)
#else /* per-platform library name */
	END(wbflush)
#endif /* HAL_LIBRARY */
#endif /* DS5500 */


/*
 * Write buffer flush routine for DS5000_100/3MIN & 
 *				  DSPERSONAL_DECSTATION/MAXine
 * A read will flush all writes
 */
#if defined(DS5000_100) || defined(DSPERSONAL_DECSTATION)
#ifdef HAL_LIBRARY
LEAF(kn02ba_wbflush)
XLEAF(kn02ca_wbflush)
#else /* per-platform library name */
LEAF(wbflush)
#endif /* HAL_LIBRARY */
	lw	v0, KN02BA_SIRM_K1ADDR
	j	ra
#ifdef HAL_LIBRARY
	END(kn02ba_wbflush)
#else /* per-platform library name */
	END(wbflush)
#endif /* HAL_LIBRARY */
#endif /* DS5000_100 || DSPERSONAL_DECSTATION */


/* 
 * Write buffer flush routine for DS5000_300/3MAX+/BIGMAX
 *
 * A read will flush all writes
 */
#ifdef DS5000_300
#ifdef HAL_LIBRARY
LEAF(kn03_wbflush)
#else /* per-platform library name */
LEAF(wbflush)
#endif /* HAL_LIBRARY */
	lw	v0, 0xbf840000 
	j	ra
#ifdef HAL_LIBRARY
	END(kn03_wbflush)
#else /* per-platform library name */
	END(wbflush)
#endif /* HAL_LIBRARY */
#endif /* DS5000_300 */
