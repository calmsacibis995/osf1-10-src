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
	.rdata
	.asciiz "@(#)$RCSfile: nofault.s,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/09/27 14:18:31 $"
	.text

#include <machine/asm.h>
#include <machine/regdef.h>
#include <machine/trap.h>
#include <sys/errno.h>
#include <machine/reg.h>

/*
 * copyin()/copyout() nofault handler.
 *
 * Note: If the bcopy routine changes, make sure that BCOPYFRM
 *       matches the new frame.
 */
#define	BCOPYFRM	0		/* We get here from bcopy() */
NESTED(cerror, BCOPYFRM, zero)
	ldiq	v0,EFAULT
/**	lda	sp,BCOPYFRM(sp) **/
	RET
	END(cerror)

/*
 * kdebug nofault handler.  Returns -1.
 */
LEAF(kdebug_error)
	ornot	zero,zero,v0
	ret	zero,(ra)
	END(kdebug_error)

/*
 * user unaligned-address fixup nofault handler.
 */
#define TMPSPACE	8		/* must match value from locore.s */
LEAF(uaerror)
	lda	sp,TMPSPACE(sp)		# realign sp with that in _XentUna

	ldq	a0,EF_PC*8(sp)		# get the offending PC
	subq	a0,4,a0			# back it up to point to bad instr
	stq	a0,EF_PC*8(sp)		# store back to exception frame
	ldq	a0,nofault_badvaddr	# offending virtual address
	bis	zero,zero,a1
	ldq	a2,nofault_cause
	ldiq	a3,T_MMANG		# code in a3
	bis	sp,zero,a4		# exception frame
	jsr	ra,trap			# trap(a0,a1,a2,trap_code,ef)
	br	zero,exception_exit
	END(uaerror)

/*
 * nofault handler for the fu/su routines, return a -1
 */
LEAF(uerror)
	ornot	zero,zero,v0	# -1
	ret	zero,(ra)
	END(uerror)

/*
 * nofault handler for the ieee emulation routines, return v0 set by caller. 
 */
LEAF(emulator_handler)
	ret	zero,(ra) 	#  return result in v0
	END(emulator_handler)


/*
 * nofault handler for the pmap_lw_wire, return a -1
 * may want to add splx and unlock pmap, or can do in lw_wire
 */

/* for pmap_lw_wire */
LEAF(lwerror)
	bis     t2, t2, a0
	call_pal PAL_swpipl
	ldq	ra, 0(sp)
	lda	sp, 128(sp)	# realign sp with that in lw_wire
	ornot	zero,zero,v0	# -1
	ret	zero,(ra)
	END(lwerror)

#define PMAPLW_UN_SPACE 160

/* for pmap_lw_unwire_ass */
LEAF(lwunerror)
	ldq	a0, 72(sp)
	call_pal PAL_swpipl
	ornot	zero,zero,v0
	ldq	ra, 0(sp)
	ldq	s0, 8(sp)
	ldq	s1, 16(sp)
	ldq	s2, 24(sp)
	ldq	s3, 32(sp)
	ldq	s4, 40(sp)
	ldq	s5, 48(sp)
	ldq	s6, 56(sp)
	lda	sp, 192(sp)
	ret	zero,(ra)
	END(lwunerror)

/* for pmap_lw_unwire_aud */
LEAF(lwunerror_aud)

	ldq	a0, 104(sp)
	call_pal PAL_swpipl
	ldq	ra, 0(sp)
	ldq	s1, 16(sp)
	ldq	s2, 24(sp)
	ldq	s0, 8(sp)
	ldq	s3, 32(sp)
	ldq	s5, 48(sp)
	ldq	s4, 40(sp)
	lda	sp, 160(sp)
	ornot	zero,zero,v0	# -1
	ret	zero,(ra)
	END(lwunerror_aud)

/* for pmap_lw_wire_ass */
LEAF(lwerror_ass)
	ldq	a0, 72(sp)
	call_pal PAL_swpipl
	ornot	zero,zero,v0
	ldq	ra, 0(sp)
	ldq	s0, 8(sp)
	ldq	s1, 16(sp)
	ldq	s2, 24(sp)
	ldq	s3, 32(sp)
	ldq	s4, 40(sp)
	ldq	s5, 48(sp)
	ldq	s6, 56(sp)
	lda	sp, 192(sp)
	ret	zero,(ra)
	END(lwerror_ass)
