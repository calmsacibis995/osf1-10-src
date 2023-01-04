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
 *	@(#)$RCSfile: fp_control.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:09:36 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * fp_control.s
 *
 *	Revision History:
 *
 * 28-Apr-91	Fred Canter
 *	Undo LANGUAGE_C hack.
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * This file contains routines to get and set the floating point control
 * registers.
 */


#include <mips/regdef.h>
#include <mips/asm.h>
#include <mips/fpu.h>
#include <float.h>

/*
 *  Notes for read_rnd and write_rnd:
 *
 *  Floating point co-processors provide the ability to specify the rounding
 *  mode for floating point operations.  For the MIPS R2010 Floating-Point 
 *  Accelerator, the rounding mode is specified by bits 0 and 1 in the FPA's
 *  control/status register.
 *
 * 31                                               2 1 0   Bit #
 *  _______________________________________________ _ _ _
 * |_______________________________________________|_|_|_|
 *   F P A  C O N T R O L / S T A T U S  R E G I S T E R
 *
 *  The bit patterns and corresponding rounding modes for the R2010 are:
 *	00	RN	Round toward nearest representable value
 *	01	RZ	Round toward zero
 *	10	RP	Round toward positive infinity
 *	11	RM	Round toward minus infinity
 *
 *  The bit patterns and corresponding rounding modes as specified by ANSI are:
 *	00	RZ	Round toward zero
 *	01	RN	Round toward nearest representable value
 *	10	RP	Round toward positive infinity
 *	11	RM	Round toward minus infinity
 */

/*
 * read_rnd returns the current rounding mode of the floating point unit.
 * A performance approach was taken when coding this routine (rather than
 * a portability one).  This routine makes use of the knowledge that both
 * the ANSI and the R2010 bit encodings for rounding to plus and rounding
 * to minus infinity are the same.
 */
LEAF(read_rnd)
	cfc1	v1,fpc_csr	/* get fpu's control status register contents */
	and	v0,v1,3		/*clear out all but csr rounding mode ctl bits*/
	bne	v0,ROUND_TO_NEAREST,rr_skip1
	ori	v0,zero,FP_RND_RN
	j	ra
rr_skip1:
	ori	v1,zero,ROUND_TO_ZERO
	bne	v0,v1,rr_skip2
	ori	v0,zero,FP_RND_RZ
rr_skip2:
	j	ra
	END(read_rnd)

/**************************
 * read_rnd returns the current rounding mode of the floating point unit.
 * A more portable approach relying on labels - not making any assumptions
 * about which ANSI bit patterns are identical to which fpu bit patterns.
LEAF(read_rnd)
	cfc1	v1,fpc_csr
	andi	v0,v1,3	
	ori	v1,zero,ROUND_TO_NEAREST
	bne	v0,v1,wr_skip1
	ori	v0,zero,FP_RND_RN
	j	ra
wr_skip1:
	ori	v1,zero,ROUND_TO_ZERO
	bne	v0,v1,wr_skip2
	ori	v0,zero,FP_RND_RZ
	j	ra
wr_skip2:
	ori	v1,zero,ROUND_TO_PLUS_INFINITY
	bne	v0,v1,wr_skip3
	ori	v0,zero,FP_RND_RP
	j	ra
wr_skip3:
	ori	v0,zero,FP_RND_RM
	j	ra
	END(read_rnd)
 **************************************/

/*
 * write_rnd sets the current rounding mode of the floating point unit and
 * returns the rounding mode in effect before the set.
 * A performance approach was taken when coding this routine (rather than
 * a portability one).  This routine makes use of the knowledge that both
 * the ANSI and the R2010 bit encodings for rounding to plus and rounding
 * to minus infinity are the same.
 */
LEAF(write_rnd)
	li	v1,3
	cfc1	a3,fpc_csr	/* get fpu's control status register contents */
	and	a0,a0,v1	/* make sure just parameter's lower 2 bits set*/
	and	v0,a3,v1	/*clear out all but csr rounding mode ctl bits*/
	not	v1
	and	a2,a3,v1	/* clear out rounding mode control bits */
	bne	a0,ROUND_TO_NEAREST,wr_skip1
	or	a2,FP_RND_RN	/* prepare value to write to fpu */
	b	wr_done
wr_skip1:
	bne	a0,ROUND_TO_ZERO,wr_skip2
	or	a2,FP_RND_RZ
	b	wr_done
wr_skip2:
	or	a2,a2,a0	/* ANSI bit pattern for infins. same as fpu's */
wr_done:
	ctc1	a2,fpc_csr	/* set fpu's control status register contents */
	j	ra
	END(write_rnd)

/*
 * get_fpc_csr returns the fpc_csr.
 */
LEAF(get_fpc_csr)
	cfc1	v0,fpc_csr
	j	ra
	END(get_fpc_csr)

/*
 * set_fpc_csr sets the fpc_csr and returns the old fpc_csr.
 */
LEAF(set_fpc_csr)
	cfc1	v0,fpc_csr
	ctc1	a0,fpc_csr
	j	ra
	END(set_fpc_csr)

/*
 * get_fpc_irr returns the fpc_irr.
 */
LEAF(get_fpc_irr)
	cfc1	v0,fpc_irr
	j	ra
	END(get_fpc_irr)

/*
 * set_fpc_led sets the floating board leds.
 */
LEAF(set_fpc_led)
	ctc1	a0,fpc_led
	j	ra
	END(set_fpc_led)

/*
 * get_fpc_eir returns the fpc_eir.
 */
LEAF(get_fpc_eir)
	cfc1	v0,fpc_eir
	j	ra
	END(get_fpc_eir)
