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
static char *rcsid = "@(#)$RCSfile: fp_ieee.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/12/01 16:37:44 $";
#endif

/*
ieee, ieee_set_fp_control, ieee_get_fp_control, ieee_set_state_in_handler, ieee_ignore_state_at_signal (3) - libc ieee trap enables support routines.
*/

#ifdef KERNEL
#include <sys/user.h>
#endif
#include <arch/alpha/fpu.h>


#ifndef KERNEL
static unsigned long	ieee_fp_control;
static unsigned long    __ieee_set_state_at_signal;
static unsigned long    __ieee_fp_control_at_signal;
static unsigned long    __ieee_fpcr_at_signal;
#else /* KERNEL */
/* variables are thread private in kernel */
#define	ieee_fp_control			u.u_ieee_fp_control
#define __ieee_set_state_at_signal	u.u_ieee_set_state_at_signal
#define __ieee_fp_control_at_signal	u.u_ieee_fp_control_at_signal
#define __ieee_fpcr_at_signal		u.u_ieee_fpcr_at_signal
#endif /* KERNEL */

unsigned long
ieee_get_fp_control()
{
    return ieee_fp_control;
} /* ieee_get_fp_control */

void
ieee_set_fp_control(
			unsigned long		fp_control
)
{
    ieee_fp_control = fp_control;
} /* ieee_set_fp_control */

void
ieee_ignore_state_at_signal()
{
    __ieee_set_state_at_signal = 0;
} /* ieee_ignore_state_at_signal */

void
ieee_set_state_at_signal(
			unsigned long		fp_control,
			unsigned long		hardware_fpcr
)
{
    __ieee_set_state_at_signal = 1;
    __ieee_fp_control_at_signal = fp_control;
    __ieee_fpcr_at_signal = hardware_fpcr;
} /* ieee_set_state_at_signal */

unsigned long
ieee_get_state_at_signal(
			unsigned long		*fp_control,
			unsigned long		*hardware_fpcr
)
{
    if (__ieee_set_state_at_signal == 0)
	return __ieee_set_state_at_signal;
    if (fp_control)
	*fp_control = __ieee_fp_control_at_signal;
    if (hardware_fpcr)
	 *hardware_fpcr = __ieee_fpcr_at_signal;
    return __ieee_set_state_at_signal;
} /* ieee_set_state_at_signal */

#ifdef manpage

These routines support implementing the ieee floating standard and
managing floating point information on Alpha OSF systems.

The ieee standard says that user programs need the ability to control
the kind of floating points traps the program expects to get. The
default is that all of the floating point traps are not enabled (i.e. not
delivered to the user). The ieee standard specifies default result values
for operations which cause traps which are not user enabled.
The Alpha hardware leaves it up to software to fixup the result values
for traps which are not enabled.

The following constants from ieee_fp.h may be used to
set the bits in the fp_control arguments which enable floating point
traps:

	#define ieee_trap_enable_INV	0x001	/* invalid operation */
	#define ieee_trap_enable_DZE	0x002	/* divide by 0 */
	#define ieee_trap_enable_OVF	0x004	/* overflow */
	#define ieee_trap_enable_UNF	0x008	/* underflow */
	#define ieee_trap_enable_INE	0x010	/* inexact */

The actions when a floating point trap occurs depends on three
variables: whether the user has set one of the above flags,
whether the user has created resumption safe code, and
what the user has specified for the signal SIGFPE.

Resumption safe code is important because it provides a mechanism
for software to supply default ieee result values for operations
which cause traps which are not enabled by software. Resumption safe
code requires some discussion about how exceptions occur on Alpha OSF.

On Alpha when a floating point exception occurs, the program counter
or sc_pc (referred to as the exception pc) found in the 
sigcontext (see signal.h) may not be the address of the instruction
which caused the trap (referred to as the trigger pc). The
instructions between the trigger pc and the exception pc are called
the trap shadow. The Alpha Architecture Handbook specifies conventions
for creating the trap shadow so that software could provide a
replacement (e.g. ieee) result value for an operation and continue execution.
The architecture provides a way for software to mark instructions
which abide by the conventions and user may request this of
the compiler driver (cc(1), etc.) by specifying the '-resumption_safe' 
argument on the command line.

On Alpha, in order to determine which exception occured the software must
back up instructions and look for the trigger pc. Once the trigger
pc is found, the software may have to re-execute or emulate the 
trigger instruction in order to determine which trap it actually 
caused (see the Alpha Architecture Handbook for more info).

The sigcontext will contain fields which will contain the trigger pc,
trigger execption and the trigger destination register. The sigcontext
will also contain a copy of the current ieee trap flags at the time of 
the exception. If the software cannot determine the trigger pc,
A read-only bit in the trap flags will be set:

	#define ieee_invalid_shadow	0x060	/* shadow invalid */

A signal will be raised when an ieee_invalid_shadow is encountered
if the following flag is set in the trap flags:

	#define ieee_signal_imprecise	0x020	/* deliver on invalid shadow */

Once we have determined the validity of the trap shadow and the trigger
exception the software can then decide what to do when an exception
occurs based on the three variables mentioned above:


SIGFPE		Trap enable shadow	actions
--------------	----------- -------	--------------
SIG_IGN|SIG_DEF	   clear    invalid	continue at exception_pc+4

SIG_IGN|SIG_DEF	   clear     safe	supply default ieee result value,
					continue at trigger_pc+4

SIG_IGN		    set	    invalid	continue at exception_pc+4

SIG_DEF		    set	    invalid	cause core dump

SIG_IGN		    set	     safe	supply default ieee result value,
					continue at trigger_pc+4

SIG_DEF		    set	     safe	cause core dump

user handler	   clear    invalid	continue at exception+4

user handler	   clear     safe	supply default ieee result value,
					continue at trigger_pc+4

user handler	    set	    invalid	deliver SIGFPE to user,
					sc_pc == sc_trigger_pc,
					set ieee_invalid_shadow

user handler	    set	     safe	deliver SIGFPE to user,
					possibly sc_pc != sc_trigger_pc


See signal(4) for information on default actions for SIGFPE.

User programs may set the ieee trap enable flags by 
calling 'ieee_set_fp_control' with a mask set appropriately. Each
thread will have one of these masks and the mask is inhereted
when creating a new thread or forking a new process, 
but does not live over exec(2).
Users should be careful to remember that setting the trap_enables,
like setting signal handlers, will affect other code within
their thread. All libraries will be compiled resumption safe.

We will also supply some common trap actions as additional trap
flags. The base trap enable must be set for the software to
even look at these (one for now):

	#define ieee_trap_enable_UMZ	0x040	/* underflow maps to 0 */

User programs may get the current ieee trap enables by calling
'ieee_get_fp_control'.

The 'jmp_buf' argument for setjmp(3) which uses struct sigcontext 
from signal(4) as an overlay will contain an 'sc_ieee_fp_control'
argument and when users longjmp(3) or sigreturn(2) the sc_ieee_trap_flag
will become the current the set of trap flags.

'ieee_set_state_at_signal' allows users to easily modify the trap
enables or rounding modes without worrying about their affect on
code executed from signal handlers (which may be third party code).
This routine will modify the floating point control register and
the ieee trap flags before calling the user handler for a signal.
If the user returns using the sigcontext from the handler, the
values from before the exception will be restored. By default,
the state at the time of the exception will be the state at the time
of calling a signal handler. User's may restore this behavior by calling
'ieee_ignore_state_at_signal'. This functionality is also per thread where
thread 0 determines the actions for taskwide (asynchronous) signals.
This functionality is inherited when creating a new thread or forking
a process, but does not live over exec(2).

[ NOTES for jeff:
[	- we have not decided whether this support will be implemented
[		at user level or kernel level so I have worded it that
[		the 'software' will do this or that. I expect the
[		actions and interface to remain 99% same either way.
[	- I saw no man pages on sigcontext-- in fact signal says the
[		handler 'can' be defined to only receive a 'signal' argument.
[		this needs to be remedied for this man page to make sense.

RELATED INFO

exec(2), signal(4), setjmp(3), sigreturn(2), longjmp(3), cc(1),
The Alpha Architecture Handbook

#endif
