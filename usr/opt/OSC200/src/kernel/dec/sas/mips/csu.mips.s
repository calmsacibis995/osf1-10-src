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
 *	@(#)$RCSfile: csu.mips.s,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 01:37:58 $
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
 * derived from csu.mips.s	3.1	(ULTRIX/OSF)	2/28/91
 */
/*		4.2	csu.mips.s
 *
 * Copyright 1985 by MIPS Computer Systems, Inc.
 *
 * csu.s -- standalone io library startup code
 */

/*
 * Revision History:
 *
 * jas - commented out reloading of a2.  environ is never loaded, so
 *       loading a2 with environ results in a2 being loaded with trash.
 */

#include <machine/regdef.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/entrypt.h>
#include "setjmp.h"
#include "mips_saio.h"

	.text

STARTFRM=	EXSTKSZ			# leave room for fault stack
NESTED(start, STARTFRM, zero)
	la	gp,_gp
	subu	v0,sp,4*4		# leave room for argsaves
/*
	sw	v0,_fault_sp		# small stack for fault handling
*/
	bne	a3,zero,1f		# no return
	la	t0,jb
	sw	ra,JB_PC*4(t0)		# ra and sp to get back to execer
	sw	sp,JB_SP*4(t0)
1:
	subu	sp,STARTFRM		# fault stack can grow to here + 16
	sw	zero,STARTFRM-4(sp)	# keep debuggers happy
	sw	a0,STARTFRM(sp)		# home args
	sw	a1,STARTFRM+4(sp)
	sw	a3,retflag		# return or exit flag
	lw	a0,STARTFRM+4(sp)	# copy strings out of prom area
	lw	a0,STARTFRM(sp)		# reload argc, argv, environ
	lw	a1,STARTFRM+4(sp)
/*	lw	a2,environ 		# don't do, it trashes a2. */
	jal	main
	lw	v1,retflag
	beq	v1,zero,return
	move	v0,a0
	jal	_exit
	END(start)

LEAF(_exit)
	lw	v1,retflag
	bne	v1,zero,promexit
	move	v0,a0
return:
	la	t0,jb
	lw	ra,JB_PC*4(t0)
	lw	sp,JB_SP*4(t0)
	j	ra

promexit:
	li	ra,+PROM_RESTART
	j	ra
	END(_exit)

	BSS(environ,4)			# environment pointer
	LBSS(retflag,4)			# return or exit flag
	BSS(jb,JB_SIZE*4)		# return jump_buf
