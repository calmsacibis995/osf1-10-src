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
static char *rcsid = "@(#)$RCSfile: kdebug_brkpt.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/10/12 13:31:13 $";
#endif

/*
 * brkpt.c -- support routines for single stepping
 */

#include <sys/kdebug.h>

static struct brkpt {
    unsigned long addr;		/* address where brkpt is installed */
    unsigned int inst;		/* previous instruction */
} brkpt1, brkpt2;

#define BREAK_INST ((unsigned int) 0x80)

/*
 * install_brkpts -- install breakpoints in the kernel
 */
void
install_brkpts()
{
    if (brkpt1.addr) {
	brkpt1.inst = *(unsigned int *)brkpt1.addr;
	*(unsigned int *)brkpt1.addr = BREAK_INST;
    }

    if (brkpt2.addr) {
	brkpt2.inst = *(unsigned int *)brkpt2.addr;
	*(unsigned int *)brkpt2.addr = BREAK_INST;
    }
}

/*
 * remove_brkpts -- remove breakpoints from the kernel
 */
void
remove_brkpts()
{
    if (brkpt1.addr) {
	*(unsigned int *)brkpt1.addr = brkpt1.inst;
	brkpt1.addr = 0;
    }

    if (brkpt2.addr) {
	*(unsigned int *)brkpt2.addr = brkpt2.inst;
	brkpt2.addr = 0;
    }
}

void
step()
{
    unsigned long pc;
    unsigned int next_inst;
    unsigned long branch_pc;

    reg_from_exc(R_EPC, &pc);

    next_inst = *(int *)pc;
    if (kdebug_isa_branch(next_inst)) {
	branch_pc = kdebug_branch_target(next_inst, pc);
	if (branch_pc == pc) {
	    kprintf(DBG_ERROR, "can't step over self-branch at 0x%x", pc);
	    quit();
	}
	brkpt1.addr = branch_pc;

	if (!kdebug_isa_uncond_branch(next_inst))
            brkpt2.addr = pc + 4L;
    } else {
        brkpt2.addr = pc + 4L;
    }
}
