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
static char *rcsid = "@(#)$RCSfile: kdebug_kern.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/09/27 14:18:49 $";
#endif

/*
 * This file contains routines and data structures that are part of
 * the kernel even when kdebug is not loaded.
 */

#include <sys/kdebug.h>
#include <machine/reg.h>

KdebugInfo kdebug_info;
long (*kdebug_handler)();
extern int numcpus;
extern kdebug_error();

kdebug_if_trap(
    u_long a0,
    u_long a1,
    u_long a2,
    u_long code,
    u_long *exc_frame)
{
    int s;
    int i;

    if (!kdebug_handler) {
	panic("kdebug_trap: kdebug not loaded");
    }

    s = kdebug_ipl(SPLEXTREME);

#if NCPUS > 1
    /*
     * prevent more than one processor from processing a breakpoint
     */
    while (test_and_set_l(0, &kdebug_info.lock))
	kdebug_wait();
#endif
    
    kdebug_info.state |= KDEBUG_BRK;
    kdebug_info.exc_frame = exc_frame;

#if NCPUS > 1
    /*
     * stop all other processors
     */
    for (i=0; i < NCPUS; i++) {
	if (machine_slot[i].is_cpu == TRUE && machine_slot[i].running == TRUE) {
	    if (cpu_number != i)
		intrpt_cpu(i, IPI_KDEBUG);
	}
    }
#endif

    (*kdebug_handler)(KDEBUG_REQ_BRKPT);

    /*
     * kdebug has fiddled with the instruction stream
     */
    kdebug_imb();

    /*
     * restart all processors that caught the IPI_KDEBUG
     * or are blocked in their lock routines
     */
    kdebug_info.state &= ~KDEBUG_BRK;
    kdebug_mb();

    /*
     * unblock any processors that want to process a breakpoint
     */
    kdebug_info.lock = 0L;
    (void) kdebug_ipl(s);
}

kdebug_mm_trap(
    u_long a0,
    u_long a1,
    u_long a2,
    u_long code,
    u_long *exc_frame)
{
    if (!kdebug_handler) {
	panic("kdebug_trap: kdebug not loaded");
    }

    if (kdebug_info.nofault) {
	exc_frame[EF_PC] = (u_long) kdebug_error;
    } else {
	printf("kdebug memory fault: va 0x%16lx, pc 0x%16lx, ra 0x%16lx\n",
	       a0, exc_frame[EF_PC], exc_frame[EF_RA]);
	panic("kdebug memory fault");
    }
}

/*
 * kdebug_bootstrap:
 *
 * Called from the kernel to initialize kdebug if necessary.
 * Perform any needed kdebug initialization that can be done at this time.
 */

void
kdebug_bootstrap(argc, argv, mode)
    int argc;
    char *argv[];
    int mode;
{
    kdebug_info.magic = KDEBUG_MAGIC;
    kdebug_info.state = KDEBUG_DISABLED;

    if (!kdebug_handler) {
        /*
         * nothing to do if the kdebug subsystem is not loaded
         */
	return;
    }

    (*kdebug_handler)(KDEBUG_REQ_INIT, argc, argv);

    if (mode) {
	/*
	 * We must use the k flag (boot_osflags) if the catching of an
	 * early breakpoint is desired.  This allows any breakpoints in
	 * .dbxinit to be set.
	 * TODO: right now we must use the k flag to use kdebug at all.
	 * Once init_saio is able to reinit the debug line, we can attach
	 * dynamically (by somehow calling kdebug_fakebreak() even after the
	 * lines are initialized by the kernel.
	 */
        (*kdebug_handler)(KDEBUG_REQ_BRKPT);
    }
}

/*
 * kdebug_state:
 *
 * Called from the kernel to tell if kdebug is active.
 */

unsigned long
kdebug_state()
{
    if (kdebug_info.magic == KDEBUG_MAGIC)
        return kdebug_info.state;
    else
	return KDEBUG_DISABLED;
}

/*
 * kdebug_wait:
 *
 * Called from any cpu which should block until the cpu processing a 
 * breakpoint is finished.
 */

void
kdebug_wait()
{
    int s;

    /*
     * see if we need to wait
     */
    if (!(kdebug_info.state & KDEBUG_BRK))
	return;

    if (numcpus == 1)
	panic("kdebug_wait: numcpus = 1");

    /*
     * freeze the clock
     */
    s = kdebug_ipl(SPLEXTREME);

    /*
     * save the state of the thread into the pcb so that dbx can examine it
     */
    save_context_all();

    /*
     * spin wait until the cpu handling the breakpoint is finished
     */
    while (kdebug_info.state & KDEBUG_BRK)
	;

    (void) kdebug_ipl(s);
}

#if !PROFILING
/*
 * dummy procedure so that dbx 'call' facility will work
 */
_mcount()
{
}
#endif
