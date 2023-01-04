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
/* @(#)$RCSfile: coreregs.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:08:00 $ */
/*
 */

#ifndef _COREREGS_H_
#define _COREREGS_H_
#include <sys/types.h>
#include <machine/reg.h>		/* Defines EF_SIZE  */
#include <machine/mips_ptrace.h>	/* Defines NFP_REGS */

/*
 * Layout of exception frame and fp registers section of core files
 */
struct core_regs {
	ulong_t    ef_regs[EF_SIZE/sizeof(ulong_t)];	/* Exception frame */
	ulong_t    fp_regs[NFP_REGS];			/* FP regs         */
	ulong_t    fp_csr;				/* FP csr	   */
};
#endif /* _COREREGS_H_ */

