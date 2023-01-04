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

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/regdef.h>
#include "assym.s"

#define SelfPC	ra
#define ATsave  t1
#define FromPC	t2
#define ToIndex t3
#define Top	t4
#define PrevTop	t5
#define Tos     t6
#define Tmp1    t7
#define Tmp2    t8

LEAF(_mcount)
#if PROFTYPE >= 2 && PROFTYPE <= 4
	.data
	IMPORT(profiling,4)			# Flag indicating on/off
	IMPORT(s_lowpc,8)			# Lowest .text address in kernel
	IMPORT(froms,8)				# Pointer to array of u_short
	IMPORT(tos,8)				# Pointer to structure
	.text
	.set		noat
	mov		AT,ATsave		# Save callers return address
	.set		at			# Assembler can again use AT
	ldl		Tmp1,profiling		# Is profiling turned on?
	bne		Tmp1,DONE

	addl		Tmp1,1,Tmp2
	stl		Tmp2,profiling
#else
	mov		AT,ATsave		# Save callers return address
#endif /* PROFTYPE >= 2 && PROFTYPE <= 4 */
#if PROFTYPE == 2 || PROFTYPE == 3
/* We need to fill this in sometime */
#elif PROFTYPE == 4

	ldq		Tmp1,s_lowpc
	subq		SelfPC,Tmp1,FromPC	# FromPC = SelfPC - s_lowpc
	ldq		Tmp1,s_textsize
	cmpult		Tmp1,FromPC,Tmp2	# FromPC > s_textize
	bne		Tmp2,OUT		# Not in range of interrest

	ldq		Tmp1,froms
#if HASHFRACTION > 1
	divqu		FromPC,HASHFRACTION
#endif
	addq		FromPC,Tmp1,FromPC	# FromPC = &froms[FromPC/hash]
	ldwu		ToIndex,0(FromPC)	# ToIndex = *FromPC
	/* The following 3 instructions are used on both sides of the branch */
	ldq		Tos,tos
	mulq		ToIndex,TOS_SIZE,Tmp1
	addq		Tmp1,Tos,Top		# Top = &tos[ToIndex]
	beq		ToIndex,NEWARC		# ToIndex == 0

	ldq		Tmp1,TOS_SELFPC(Top)
	subq		ATsave,Tmp1,Tmp2	# Top->selfpc != ATsave
	bne		Tmp2,LOOP		# Isn't at the front of chain

	ldq		Tmp1,TOS_COUNT(Top)
	addq		Tmp1,1,Tmp2		# ++(Top->count)
	stq		Tmp2,TOS_COUNT(Top)
	br		zero,OUT

LOOP:
	ldwu		Tmp1,TOS_LINK(Top)
	beq		Tmp1,NEWARC		# Top->link == 0

	mov		Top,PrevTop		# PrevTop = Top
	mulq		Tmp1,TOS_SIZE,Tmp2
	addq		Tmp2,Tos,Top		# Top = &tos[Top->link]
	ldq		Tmp2,TOS_SELFPC(Top)
	subq		ATsave,Tmp2,Tmp1	# Top->selfpc == ATsave
	bne		Tmp1,LOOP

	ldq		Tmp1,TOS_COUNT(Top)
	addq		Tmp1,1,Tmp2		# ++(Top->count)
	stq		Tmp2,TOS_COUNT(Top)
	ldl		ToIndex,TOS_LINK(PrevTop)
	ldwu		Tmp1,TOS_LINK(Top)
	stw		Tmp1,TOS_LINK(PrevTop)	# PrevTop->link = Top->link
	ldwu		Tmp1,0(FromPC)
	stw		Tmp1,TOS_LINK(Top)	# Top->link = *FromPC
	stw		ToIndex,0(FromPC)	# *FromPC = ToIndex
	br		zero,OUT

NEWARC:
	ldwu		Tmp1,TOS_LINK(Tos)
	addq		Tmp1,1,Tmp2		# ++(tos->link)
	mov		Tmp2,ToIndex		# ToIndex = tos->link
	stw		Tmp2,TOS_LINK(Tos)
	ldq		Tmp1,tolimit
	cmplt		ToIndex,Tmp1,Tmp2	# ToIndex >= tolimit
	beq		Tmp2,NOROOM

	mulq		ToIndex,TOS_SIZE,Tmp1
	addq		Tmp1,Tos,Top		# Top = &tos[ToIndex]
	stq		ATsave,TOS_SELFPC(Top)	# Top->selfpc = ATsave
	ldiq		Tmp1,1
	stq		Tmp1,TOS_COUNT(Top)	# Top->count = 1
	ldwu		Tmp1,0(FromPC)
	stw		Tmp1,TOS_LINK(Top)	# Top->link = *FromPC
	stw		ToIndex,0(FromPC)	# *FromPC = ToIndex
#endif /* PROFTYPE == 4 */
#if PROFTYPE >= 2 && PROFTYPE <= 4
OUT:
	ldl		Tmp1,profiling
	subl		Tmp1,1,Tmp2
	stl		Tmp2,profiling
DONE:
#endif /* PROFTYPE >= 2 && PROFTYPE <= 4 */
	ret		(ATsave)		# Return to caller

#if PROFTYPE == 4
NOROOM:
	ldil		Tmp1,3
	stl		Tmp1,profiling		# We are full. Mark it so
        PRINTF("_mcount: tos overflow\n")
	br		zero,OUT
#endif /* PROFTYPE == 4 */

END(_mcount)
