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
static char *rcsid = "@(#)$RCSfile: cma_kernel.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/04/13 21:32:14 $";
#endif
/*
 * FACILITY:
 *
 *	CMA services
 *
 * ABSTRACT:
 *
 *	This module defines the interface for locking and unlocking the kernel
 *	scheduling database.
 *
 * AUTHORS:
 *
 *	Dave Butenhof
 *
 * CREATION DATE:
 *
 *	14 June 1990
 *
 * MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	09 April 1991
 *		Use new type for "atomic bit" operation target
 *	002	Dave Butenhof	10 May 1991
 *		Add new global data for debug, and recording kernel
 *		functions.
 *	003	Dave Butenhof	13 May 1991
 *		Add recording "tryenter_kernel" function.
 *	004	Dave Butenhof	17 May 1991
 *		Have enter kernel functions check for multiple entry.
 *	005	Dave Butenhof	11 June 1991
 *		Add formatting function for kernel trace array.
 *	006	Dave Butenhof	25 July 1991
 *		Support debug build without VAXCRTL on VMS by using internal
 *		*printf emulation functions (#defined to real functions on
 *		UNIX platforms).
 *	007	Paul Curtin	20 August 1991
 *		Conditionalized out the include of stdio.h on VMS.
 *	008	Dave Butenhof	17 September 1991
 *		Fix cma__format_karray() to close the file.
 *	009	Dave Butenhof	10 October 1991
 *		Change format karray routine to go to stdout.
 *	010	Webb Scales	13 February 1992
 *		Perform undeferrals on enter-kernel.
 *	011	Dave Butenhof	24 March 1992
 *		Modify format karray to use redirectable I/O
 *	012	Dave Butenhof	19 August 1992
 *		Upgrade _record versions of mP kernel lock to use
 *		_CMA_SPINLOCKYIELD_.
 *	013	Dave Butenhof	30 March 1993
 *		Replace "%08x" formatting with "0x%lx" for 64-bit safety.
 */

/*
 * INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_kernel.h>
#include <cma_stack.h>
#include <cma_tcb_defs.h>
#if _CMA_TRACE_KERNEL_
# if _CMA_OS_ == _CMA__UNIX
#  include <stdio.h>
# endif
#endif

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */

#ifdef _CMA_TRACE_KERNEL_
typedef struct CMA___T_KARRAY {
    cma_t_integer	eline;
    char		*efile;
    cma__t_int_tcb	*ethd;
    cma_t_integer	eseq;
    cma_t_boolean	tryenter;
    cma_t_boolean	locked;
    cma_t_integer	xline;
    char		*xfile;
    cma__t_int_tcb	*xthd;
    cma_t_integer	xseq;
    cma_t_boolean	unlock;
    cma_t_boolean	unset;
    } cma___t_karray;
#endif

/*
 * GLOBAL DATA
 */

/*
 * Set when scheduling database is locked by a thread
 */
cma__t_atomic_bit	cma__g_kernel_critical = cma__c_tac_static_clear;

/*
 * LOCAL DATA
 */

#ifdef _CMA_TRACE_KERNEL_
static cma_t_integer	cma___g_kidx = 0;
static cma___t_karray	cma___g_karray[_CMA_TRACE_KERNEL_];
#endif

/*
 * INTERNAL INTERFACES
 */

#ifdef _CMA_TRACE_KERNEL_

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine locks the kernel; normally, the inline macro version
 *	(cma__enter_kernel) is used instead, but this function is called when
 *	running in a non-production (!defined(NDEBUG)) environment with
 *	kernel lock recording (_CMA_TRACE_KERNEL_) enabled.  Mostly just
 *	because the code is sufficiently large and slow that the call
 *	probably doesn't matter: but also because it provides a way to set a
 *	breakpoint at kernel entry and exit (as well as being able to
 *	redefine the recording code without recompiling every DECthreads
 *	module).
 *
 *  FORMAL PARAMETERS:
 *
 *	line		Line number of call
 *	file		File name of call
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__enter_kern_record
#ifdef _CMA_PROTO_
	(
	cma_t_integer	line,
	char		*file)
#else	/* no prototypes */
	(line, file)
	cma_t_integer	line;
	char		*file;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self = cma__get_self_tcb ();


#if _CMA_UNIPROCESSOR_
    if (cma__kernel_set (&cma__g_kernel_critical))
	cma__bugcheck ("enter_kernel: deadlock");
#else
	{
	int limit = _CMA_SPINLOCKYIELD_;

	while (cma__kernel_set (&cma__g_kernel_critical))
	    if (limit <= 0) {
		cma__vp_yield ();
		limit = _CMA_SPINLOCKYIELD_;
		}
	    else
		--limit;
	}
#endif

    cma__assert_fail (
	    !cma___g_karray[cma___g_kidx].locked,
	    "enter kernel succeeded when kernel was already locked");

    cma___g_karray[cma___g_kidx].locked = cma_c_true;
    cma___g_karray[cma___g_kidx].eline = line;
    cma___g_karray[cma___g_kidx].efile = file;
    cma___g_karray[cma___g_kidx].ethd = self;
    cma___g_karray[cma___g_kidx].eseq = self->header.sequence;
    cma___g_karray[cma___g_kidx].xline = 0;
    cma___g_karray[cma___g_kidx].xfile = (char *)cma_c_null_ptr;
    cma___g_karray[cma___g_kidx].xthd = (cma__t_int_tcb *)cma_c_null_ptr;
    cma___g_karray[cma___g_kidx].eseq = 0;
    cma___g_karray[cma___g_kidx].tryenter = cma_c_false;
    cma___g_karray[cma___g_kidx].unlock = cma_c_false;
    cma___g_karray[cma___g_kidx].unset = cma_c_false;

    cma__undefer ();
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine unlocks the kernel; normally, the inline macro version
 *	(cma__enter_kernel) is used instead, but this function is called when
 *	running in a non-production (!defined(NDEBUG)) environment with
 *	kernel lock recording (_CMA_TRACE_KERNEL_) enabled.  Mostly just
 *	because the code is sufficiently large and slow that the call
 *	probably doesn't matter: but also because it provides a way to set a
 *	breakpoint at kernel entry and exit (as well as being able to
 *	redefine the recording code without recompiling every DECthreads
 *	module).
 *
 *  FORMAL PARAMETERS:
 *
 *	line		Line number of call
 *	file		File name of call
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__exit_kern_record
#ifdef _CMA_PROTO_
	(
	cma_t_integer	line,
	char		*file)
#else	/* no prototypes */
	(line, file)
	cma_t_integer	line;
	char		*file;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self = cma__get_self_tcb ();


    cma___g_karray[cma___g_kidx].xline = line;
    cma___g_karray[cma___g_kidx].xfile = file;
    cma___g_karray[cma___g_kidx].xthd = self;
    cma___g_karray[cma___g_kidx].xseq = self->header.sequence;

    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "cma_exit_kernel:  kernel critical already unlocked");

    cma___g_karray[cma___g_kidx].unlock = cma_c_true;
    cma___g_karray[cma___g_kidx].locked = cma_c_false;

    if (cma___g_kidx >= _CMA_TRACE_KERNEL_-1)
	cma___g_kidx = 0;
    else
	cma___g_kidx++;

    cma__undefer ();
    cma__kernel_unset (&cma__g_kernel_critical);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine formats the entries in cma___g_karray to assist in
 *	debugging.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__format_karray
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer	i, empty = -1;
    char		buffer[256];


    buffer[0] = '\0';
    cma__putstring (buffer, "Dump of kernel trace array:");
    cma__puteol (buffer);
    cma__puteol (buffer);
    cma__putformat (buffer, "\tCurrent active index is %d", cma___g_kidx);
    cma__puteol (buffer);

    for (i = 0; i < _CMA_TRACE_KERNEL_; i++) {

	if (!cma___g_karray[i].locked && !cma___g_karray[i].unlock) {
	    /*
	     * If this entry is empty, then just skip it. If it's the first
	     * in a series of empty entries, record the index.
	     */
	    if (empty == -1) empty = i;
	    }
	else {
	    /*
	     * If we've found a non-empty entry, then display it. First, if
	     * we just completed a series of empty entries, report the
	     * range.
	     */
	    if (empty != -1) {
		cma__putformat (
			buffer,
			"[[Entries %d to %d are empty]]",
			empty,
			i-1);
		cma__puteol (buffer);
		empty = -1;
		}

	    cma__putformat (
		    buffer,
		    "[%03d] %sentry thd 0x%lx (%d) from \"%s\":%d,",
		    i,
		    (cma___g_karray[i].tryenter ? "try" : ""),
		    cma___g_karray[i].ethd,
		    cma___g_karray[i].eseq,
		    cma___g_karray[i].efile,
		    cma___g_karray[i].eline);
	    cma__puteol (buffer);

	    if (cma___g_karray[i].locked)
		cma__putstring (buffer, "  never unlocked.");
	    else
		cma__putformat (
			buffer,
			"  %s thd 0x%lx (%d) from \"%s\":%d.",
			(cma___g_karray[i].unset ? "unset" : "exit"),
			cma___g_karray[i].xthd,
			cma___g_karray[i].xseq,
			cma___g_karray[i].xfile,
			cma___g_karray[i].xline);

	    cma__puteol (buffer);
	    }

	}

    if (empty != -1) {
	cma__putformat (
		buffer,
		"[[Entries %d to %d are empty]]",
		empty,
		_CMA_TRACE_KERNEL_-1);
	cma__puteol (buffer);
	empty = -1;
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine locks the kernel; normally, the inline macro version
 *	(cma__set_kernel) is used instead, but this function is called when
 *	running in a non-production (!defined(NDEBUG)) environment with
 *	kernel lock recording (_CMA_TRACE_KERNEL_) enabled.  Mostly just
 *	because the code is sufficiently large and slow that the call
 *	probably doesn't matter: but also because it provides a way to set a
 *	breakpoint at kernel entry and exit (as well as being able to
 *	redefine the recording code without recompiling every DECthreads
 *	module).
 *
 *  FORMAL PARAMETERS:
 *
 *	line		Line number of call
 *	file		File name of call
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__set_kern_record
#ifdef _CMA_PROTO_
	(
	cma_t_integer	line,
	char		*file)
#else	/* no prototypes */
	(line, file)
	cma_t_integer	line;
	char		*file;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self = cma__get_self_tcb ();


#if _CMA_UNIPROCESSOR_
    if (cma__kernel_set (&cma__g_kernel_critical))
	cma__bugcheck ("enter_kernel: deadlock");
#else
	{
	int limit = _CMA_SPINLOCKYIELD_;

	while (cma__kernel_set (&cma__g_kernel_critical))
	    if (limit <= 0) {
		cma__vp_yield ();
		limit = _CMA_SPINLOCKYIELD_;
		}
	    else
		--limit;
	}
#endif

    cma__assert_fail (
	    !cma___g_karray[cma___g_kidx].locked,
	    "set kernel succeeded when kernel was already locked");

    cma___g_karray[cma___g_kidx].locked = cma_c_true;
    cma___g_karray[cma___g_kidx].eline = line;
    cma___g_karray[cma___g_kidx].efile = file;
    cma___g_karray[cma___g_kidx].ethd = self;
    cma___g_karray[cma___g_kidx].eseq = self->header.sequence;
    cma___g_karray[cma___g_kidx].xline = 0;
    cma___g_karray[cma___g_kidx].xfile = (char *)cma_c_null_ptr;
    cma___g_karray[cma___g_kidx].xthd = (cma__t_int_tcb *)cma_c_null_ptr;
    cma___g_karray[cma___g_kidx].eseq = 0;
    cma___g_karray[cma___g_kidx].tryenter = cma_c_false;
    cma___g_karray[cma___g_kidx].unlock = cma_c_false;
    cma___g_karray[cma___g_kidx].unset = cma_c_false;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine attempts to lock the kernel and returns the previous
 *	state; normally, the inline macro version (cma__enter_kernel) is used
 *	instead, but this function is called when running in a non-production
 *	(!defined(NDEBUG)) environment with kernel lock recording
 *	(_CMA_TRACE_KERNEL_) enabled.  Mostly just because the code is
 *	sufficiently large and slow that the call probably doesn't matter:
 *	but also because it provides a way to set a breakpoint at kernel
 *	entry and exit (as well as being able to redefine the recording code
 *	without recompiling every DECthreads module).
 *
 *  FORMAL PARAMETERS:
 *
 *	line		Line number of call
 *	file		File name of call
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_boolean
cma__tryenter_kern_record
#ifdef _CMA_PROTO_
	(
	cma_t_integer	line,
	char		*file)
#else	/* no prototypes */
	(line, file)
	cma_t_integer	line;
	char		*file;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self = cma__get_self_tcb ();


    if (cma__kernel_set (&cma__g_kernel_critical))
	return cma_c_true;		/* Return TRUE if already locked */
    else {
	cma__assert_fail (
		!cma___g_karray[cma___g_kidx].locked,
		"enter kernel succeeded when kernel was already locked");

	cma___g_karray[cma___g_kidx].locked = cma_c_true;
	cma___g_karray[cma___g_kidx].eline = line;
	cma___g_karray[cma___g_kidx].efile = file;
	cma___g_karray[cma___g_kidx].ethd = self;
	cma___g_karray[cma___g_kidx].eseq = self->header.sequence;
	cma___g_karray[cma___g_kidx].xline = 0;
	cma___g_karray[cma___g_kidx].xfile = (char *)cma_c_null_ptr;
	cma___g_karray[cma___g_kidx].xthd = (cma__t_int_tcb *)cma_c_null_ptr;
	cma___g_karray[cma___g_kidx].xseq = 0;
	cma___g_karray[cma___g_kidx].tryenter = cma_c_true;
	cma___g_karray[cma___g_kidx].unlock = cma_c_false;
	cma___g_karray[cma___g_kidx].unset = cma_c_false;
	return cma_c_false;
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine unlocks the kernel without processing deferrals;
 *	normally, the inline macro version (cma__kernel_unset) is used
 *	instead, but this function is called when running in a non-production
 *	(!defined(NDEBUG)) environment with kernel lock recording
 *	(_CMA_TRACE_KERNEL_) enabled.  Mostly just because the code is
 *	sufficiently large and slow that the call probably doesn't matter:
 *	but also because it provides a way to set a breakpoint at kernel
 *	entry and exit (as well as being able to redefine the recording code
 *	without recompiling every DECthreads module).
 *
 *  FORMAL PARAMETERS:
 *
 *	line		Line number of call
 *	file		File name of call
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__unset_kern_record
#ifdef _CMA_PROTO_
	(
	cma_t_integer	line,
	char		*file)
#else	/* no prototypes */
	(line, file)
	cma_t_integer	line;
	char		*file;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self = cma__get_self_tcb ();


    cma___g_karray[cma___g_kidx].xline = line;
    cma___g_karray[cma___g_kidx].xfile = file;
    cma___g_karray[cma___g_kidx].xthd = self;
    cma___g_karray[cma___g_kidx].xseq = self->header.sequence;

    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "cma_exit_kernel:  kernel critical already unlocked");

    cma___g_karray[cma___g_kidx].unlock = cma_c_true;
    cma___g_karray[cma___g_kidx].unset = cma_c_true;	/* released by unset */
    cma___g_karray[cma___g_kidx].locked = cma_c_false;

    if (cma___g_kidx >= _CMA_TRACE_KERNEL_-1)
	cma___g_kidx = 0;
    else
	cma___g_kidx++;

    cma__kernel_unset (&cma__g_kernel_critical);
    }
#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_KERNEL.C */
/*  *17    1-APR-1993 14:32:50 BUTENHOF "Remove use of %08x formatting" */
/*  *16    2-SEP-1992 16:25:13 BUTENHOF "Fix undeferral" */
/*  *15   21-AUG-1992 13:42:03 BUTENHOF "Limit spinlock by config symbol" */
/*  *14   24-MAR-1992 14:47:18 BUTENHOF "Use redirectable I/O for kernel trace" */
/*  *13   19-FEB-1992 13:50:17 SCALES "Undefer on enter-kernel" */
/*  *12   14-OCT-1991 13:39:06 BUTENHOF "Change trace dumps to go to stdout" */
/*  *11   17-SEP-1991 13:21:15 BUTENHOF "Fix format debug array functions" */
/*  *10   21-AUG-1991 16:42:37 CURTIN "Removed VMS include of stdio.h" */
/*  *9    25-JUL-1991 13:53:39 BUTENHOF "Use cma__int_*printf functions" */
/*  *8    11-JUN-1991 17:17:02 BUTENHOF "Add & use functions to dump kernel/sem trace arrays" */
/*  *7    10-JUN-1991 18:22:11 SCALES "Add sccs headers for Ultrix" */
/*  *6    29-MAY-1991 17:14:44 BUTENHOF "Add multiple entry checks to enter_kernel" */
/*  *5    14-MAY-1991 13:43:26 BUTENHOF "Add global data for kernel trace" */
/*  *4    10-MAY-1991 16:43:35 BUTENHOF "Add global data for kernel trace" */
/*  *3    10-MAY-1991 16:18:46 BUTENHOF "Add global data for kernel trace" */
/*  *2    12-APR-1991 23:35:59 BUTENHOF "Change type of internal locks" */
/*  *1    12-DEC-1990 21:46:50 BUTENHOF "Kernel lock support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_KERNEL.C */
