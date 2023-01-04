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
static char *rcsid = "@(#)$RCSfile: cma_errors.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:47:33 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	This module is the implementation of
 * 	the platform-specific error reporting mechanism.
 *
 *  AUTHORS:
 *
 *	Robert A. Conti
 *
 *  CREATION DATE:
 *
 *	6 October 1989
 *
 *  MODIFICATION HISTORY:
 * 
 *	001	Webb Scales	20 October 1989
 *		ifdef globalvalue & delete call to cma__error(cma$_bugcheck)
 *	002	Webb Scales	20 October 1989
 *		ifdef the refernences to lib$stop, too!
 *	003	Bob Conti	10 December 1989
 *		Move cma__unimplemented here from cma_exception.c
 *	004	Bob Conti	10 December 1989
 *		Hook up to portable exceptions package for the non-VMS case.
 *	005	Dave Butenhof	14 December 1989
 *		Add cma__report_error to generate error message for an
 *		exception.
 *	006	Dave Butenhof	15 December 1989
 *		On UNIX, cma__report_error should report exception status
 *		value, not address, if it's status type (even though the
 *		value doesn't really MEAN anything on UNIX).
 *	007	Webb Scales	11 January 1990
 *		Added assertion that cma_error is called _outside_ the kernel.
 *		If we call cma_error while we are inside the kernel, the entire
 *		process will hang when we try to raise the exception, because
 *		we will attempt to reenter the kernel to get the thread's tcb.
 *	008	Dave Butenhof	29 May 1990
 *		Clean up by removing #include for starlet.h on VMS (it's not
 *		needed anymore, and complicates SCA searching).
 *	009	Webb Scales	15 August 1990
 *		Replaced an #ifdef unix with an #if _CMA_OS_ == _CMA__UNIX
 *	010	Paul Curtin	15 October 1990
 *		Made cma$_bugcheck cma$s_bugcheck
 *	011	Dave Butenhof	25 March 1991
 *		Change from cma_exception to exc_handling
 *	012	Dave Butenhof	09 April 1991
 *		Don't reference kernel_critical flag directly
 *	013	Dave Butenhof	01 May 1991
 *		Add string argument to cma__bugcheck() (written out before
 *		raising exception).
 *	014	Dave Butenhof	08 May 1991
 *		Use new macro to test for kernel already locked, which
 *		evaporates under multiprocessor implementation.
 *	015	Dave Butenhof	11 June 1991
 *		Change cma__bugcheck() to call formatting functions for
 *		kernel and semaphore tracing if the appropriate conditionals
 *		are set.
 *	016	Paul Curtin	18 November 1991
 *		Added the use of an Alpha switch.
 *	017	Dave Butenhof	27 November 1991
 *		Remove globalvalue of cma$s_bugcheck.
 *	018	Dave Butenhof	16 December 1991
 *		Add null argument count to cma__error's lib$stop.
 *	019	Dave Butenhof	27 January 1992
 *		Add more nulls to cma__error's lib$stop, to match
 *		exc_handling's exc_raise_status.
 *	020	Dave Butenhof	28 January 1992
 *		Modify lib$stop local prototype to avoid comp. errors in 019.
 *	021	Dave Butenhof	17 March 1992
 *		Major revision to bugcheck: abort execution immediately
 *		rather than raising an exception (which obscures the state
 *		information and also won't work if the bugcheck occurred
 *		while the DECthreads kernel is locked!) This is only the
 *		beginning: there should be a full state report, optionally to
 *		an output file, as well as the core dump for more detailed
 *		poking about.
 *	022	Dave Butenhof	23 March 1992
 *		Change bugcheck to support argument list.
 *	023	Dave Butenhof	01 April 1992
 *		Change text of bugcheck message -- instead of calling it an
 *		"internal error", call it an "internal problem". The DTM
 *		test set thinks any output with "error" is a failure, making
 *		it difficult to pass a bugcheck test.
 *	024	Dave Butenhof	17 April 1992
 *		Add DECthreads version to bugcheck message.
 *	025	Dave Butenhof	08 June 1992
 *		Add current thread to bugcheck trace file.
 *	026	Dave Butenhof	10 June 1992
 *		Add #include for cma_kernel.h to get try_enter_kernel().
 *	027	Dave Butenhof	16 June 1992
 *		Webb pointed out that I screwed sense of try_enter_kernel
 *		test, and he's right. So tempting to believe things return
 *		"true" when they succeed -- but it don't.
 *	028	Dave Butenhof	16 June 1992
 *		Add list of attributes objects to bugcheck dump.
 *	029	Brian Keane	17 July 1992
 *		Minor fix to avoid accvio in list of attributes objects.
 *	030	Dave Butenhof	03 August 1992
 *		Modify current thread printout on bugcheck to use full 64
 *		bits on Alpha OSF/1.
 *	031	Dave Butenhof	19 August 1992
 *		Add a static flag to prevent recursive bugchecking -- once is
 *		really enough! Also, get fancy and add better configuration
 *		info to the bugcheck report -- like hardware and O/S from the
 *		cma_config flags.
 *	032	Dave Butenhof	04 September 1992
 *		Final name for "Alpha" architecture -- "Alpha AXP"
 *	033	Dave Butenhof	24 September 1992
 *		Instead of locking kernel for bugcheck, disable timeslicing
 *		(on uniprocessor) as well as shutting down other threads
 *		(which it already does on k-thread versions). Always UNLOCK
 *		the kernel to ensure that mutex operations can work during
 *		bugcheck reporting.
 *	034	Dave Butenhof	16 October 1992
 *		Get rid of %0*lx -- 0-filling on 64-bits is ugly.
 *	035	Dave Butenhof	19 January 1993
 *		If unable to open cma_dump.log, write log to stderr.
 *	036	Dave Butenhof	28 January 1993
 *		Add global cma__g_bugchecked variable to record that a bugcheck
 *		is in progress -- this will disable TIS synchronization
 *		blocks to ensure that bugcheck can use the C runtime without
 *		spurious blocks due to stopped or hosed threads.
 *	037	Dave Butenhof	 6 May 1993
 *		cma_tis_sup has been generalized to use new
 *		cma__g_tis_disable instead of sharing this module's
 *		cma__g_bugchecked: so set the new flag when starting to
 *		bugcheck. (Note that cma__g_bugchecked still exists, but now
 *		is only used to detect a recursive attempt to bugcheck).
 *		Also, while I'm here, add the new "stack" command to the
 *		output.
 *	038	Dave Butenhof	10 May 1993
 *		Add 0x prefix to current thread address, and include -f on
 *		attributes command.
 *	039	Dave Butenhof	13 May 1993
 *		Use "-z" in "thread" command on bugcheck.
 *	040	Dave Butenhof	 1 June 1993
 *		On OSF/1, report state of VP spinlock on entering bugcheck.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_errors.h>
#include <cma_kernel.h>
#include <cma_dispatch.h>
#include <cma_stack.h>
#include <cma_init.h>
#include <cma_debugger.h>
#include <cma_timer.h>
#include <cma_tis_sup.h>
#include <cma_vp.h>
#if _CMA_OS_ == _CMA__UNIX
# include <stdio.h>
#endif
#include <cma_util.h>
#ifdef _CMA_PROTO_
# include <stdarg.h>
#else
# include <varargs.h>
#endif

/*
 * GLOBALS
 */

cma_t_boolean	cma__g_bugchecked = cma_c_false;

/*
 * LOCAL DATA
 */

static cma__t_file	cma___g_dumpfile;

/*
 * LOCAL FUNCTIONS
 */


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is called to report that a major internal inconsistency
 *	has been detected in CMA during execution of the production version.
 *
 *      The user action associated with a call to this routine is to submit
 *  	an SPR, so this routine should only be used when an internal bug
 *	is suspected, rather than a client-caused error.
 *
 *	Note that this routine cannot be "macro-ized" as it is called from
 *	assembly code.
 *
 *  FORMAL PARAMETERS:
 *
 *	text	Text describing error circumstances
 *	args	Variable printf-style arglist for text (format string).
 *
 *  IMPLICIT INPUTS:
 *
 *	NONE
 *
 *  IMPLICIT OUTPUTS:
 *
 *	NONE
 *
 *  FUNCTION VALUE:
 *
 *	NONE
 *
 *  SIDE EFFECTS:
 *
 *	The bugcheck is reported as appropriate to the current host.
 *	Typically, an exception is raised in the current thread.
 */
static void cma___dump_eol
#ifdef _CMA_PROTO_
	(char	*buffer)
#else
	(buffer)
	char	*buffer;
#endif
    {
    cma__int_fprintf (cma___g_dumpfile, "%s\n", buffer);
#if _CMA_OS_ == _CMA__UNIX
    fflush (cma___g_dumpfile);
#endif
    }

extern void
cma__bugcheck
#ifdef _CMA_PROTO_
	(char		*text,
	...)
#else	/* no prototypes */
	(va_alist)
	va_dcl
#endif	/* prototype */
    {
    cma__t_file		errfile;
    cma__t_int_tcb	*self;
    cma__t_eol_routine	save_eol;
    cma_t_boolean	kernel_was_locked;
    va_list		ap;
    char		buffer[512], buffer1[512], sysbuf[100];
    static char		*log_file = "cma_dump.log";
    static char		*attcmd[2] = {"attributes","-f"};
    static char		*thdcmd[2] = {"thread","-iafz"};
    static char		*mutcmd[2] = {"mutex","-q"};
    static char		*concmd[2] = {"condition","-qf"};
    static char		*stkcmd[2] = {"stack","-af"};
    static char		*shocmd[2] = {"show","-mp"};
    static char		*shkcmd[2] = {"show","-k"};
    static char		*shscmd[2] = {"show","-s"};
    static char		*shvcmd[2] = {"show","-v"};
#ifdef _CMA_PROTO_
    va_start (ap, text);
#else
    char		*text;


    va_start (ap);
    text = va_arg (ap, char*);
#endif

    /*
     * Try to lock the kernel -- but continue anyway. Since we're
     * bugchecking, even if this is a multiprocessor we can't count on
     * another thread ever RELEASING the kernel (plus, we can't tell whether
     * the current thread might hold the lock). Ignoring an existing MP lock
     * is potentially dangerous -- but we're bugchecking anyway, so what are
     * we supposed to do but our best?
     */
    kernel_was_locked = cma__tryenter_kernel ();

    /*
     * Check whether this is the first time we've tried to bugcheck. If not,
     * then something must have gone wrong in trying to report the first
     * bugcheck (since we never return from a bugcheck and keep going!). So
     * don't try to report it -- just dump cookies and die.
     */
    if (cma__g_bugchecked) {
	cma__unset_kernel ();
#if _CMA_OS_ == _CMA__VMS
	cma__abort ();
#else
	fflush (cma___g_dumpfile);	/* Flush anything that was written */
	cma__abort_process (SIGIOT);	/* Abort with core dump */
#endif
	}
    else
	cma__g_bugchecked = cma_c_true;

    /*
     * Turn off TIS locking functions (only relevant for OpenVMS and OSF/1 at
     * this point, but does no harm on ULTRIX).
     */
    cma__g_tis_disable = cma_c_true;

#if !_CMA_UNIPROCESSOR_
    cma__vp_suspend_others ();
#else
    cma__disable_timer ();
#endif

    /*
     * Now unlock the kernel, even if we didn't actually lock it, to minimize
     * the risk of blocking somewhere (for example, the debug routines may
     * lock the kernel). We've disabled timeslicing, and suspended any other
     * kernel threads (if applicable), so we're as safe as reasonably
     * possible.
     */
    cma__unset_kernel ();

    if (text != (char *)cma_c_null_ptr)
	cma__int_vsprintf (buffer, text, ap);
    else
	cma__int_sprintf (buffer, "unknown.");

    va_end (ap);
    errfile = cma__int_fopen ("stderr", "w");
    cma__int_fprintf (
	    errfile,
	    "%%Internal DECthreads problem (version %s), terminating execution.\n",
	    cma__g_version);
    cma__debug_get_system (sysbuf, 100);
    cma__int_sprintf (
	    buffer1,
	    "%%running on %s %s [%s]\n",
	    cma__g_os,
	    cma__g_hardware,
	    sysbuf);
    cma__int_fprintf (errfile, "%s", buffer1);
    cma__int_fprintf (errfile, "%% Reason: %s\n", buffer);

    cma___g_dumpfile = cma__int_fopen (log_file, "w");

    if (cma___g_dumpfile == (cma__t_file)0) {
	cma___g_dumpfile = errfile;
	cma__int_fprintf (
		errfile,
		"%% Unable to write '%s'; state information follows:\n",
		log_file);
	}
    else {
	cma__int_fprintf (
		errfile,
		"%% See '%s' for state information.\n",
		log_file);
	cma__int_fprintf (
		cma___g_dumpfile,
		"%%Internal DECthreads problem (version %s), terminating execution.\n",
		cma__g_version);
	cma__int_fprintf (cma___g_dumpfile, "%s", buffer1);
	cma__int_fprintf (cma___g_dumpfile, "%% Reason: %s\n", buffer);
	}

    self = cma__get_self_tcb ();
    cma__int_fprintf (
	    cma___g_dumpfile,
	    "The current thread is %d (address 0x%lx)\n",
	    self->header.sequence,
	    self);

    if (kernel_was_locked)
	cma__int_fprintf (
		cma___g_dumpfile,
		"DECthreads scheduling database is locked.\n");

#if !_CMA_UNIPROCESSOR_
    if (cma__g_vp_lock)
	cma__int_fprintf (
		cma___g_dumpfile,
		"DECthreads VP database is locked.\n");
#endif

    cma__set_eol_routine (cma___dump_eol, &save_eol);
    cma__int_fprintf (cma___g_dumpfile, "Current attributes objects:\n");
    cma__debug_list_atts (sizeof(attcmd)/sizeof (char *), attcmd);
    cma__int_fprintf (cma___g_dumpfile, "Current threads:\n");
    cma__debug_list_threads (sizeof(thdcmd)/sizeof (char *), thdcmd);
    cma__int_fprintf (cma___g_dumpfile, "Mutexes:\n");
    cma__debug_list_mutexes (sizeof(mutcmd)/sizeof (char *), mutcmd);
    cma__int_fprintf (cma___g_dumpfile, "Condition variables:\n");
    cma__debug_list_cvs (sizeof (concmd)/sizeof (char *), concmd);
    cma__int_fprintf (cma___g_dumpfile, "Stacks:\n");
    cma__debug_list_stacks (sizeof (stkcmd)/sizeof (char *), stkcmd);
    cma__debug_show (sizeof (shocmd)/sizeof (char *), shocmd);
#ifdef _CMA_TRACE_KERNEL_
    cma__int_fprintf (cma___g_dumpfile, "Kernel trace info:\n");
    cma__debug_show (sizeof (shkcmd)/sizeof (char *), shkcmd);
#endif
#ifdef _CMA_TRACE_SEM_
    cma__int_fprintf (cma___g_dumpfile, "Semaphore trace info:\n");
    cma__debug_show (sizeof (shscmd)/sizeof (char *), shscmd);
#endif
#if _CMA_KTHREADS_ != _CMA__NONE
    cma__int_fprintf (cma___g_dumpfile, "Kernel threads:\n");
    cma__debug_show (sizeof (shvcmd)/sizeof (char *), shvcmd);
#endif
    cma__set_eol_routine (save_eol, (cma__t_eol_routine *)0);
#if _CMA_OS_ == _CMA__VMS
    cma__abort ();
#else
    fflush (cma___g_dumpfile);
    cma__abort_process (SIGIOT);	/* Abort with core dump */
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is called to report that a client-caused error
 *	has been detected in CMA during execution of the production version.
 * 	In other words, to report errors that are defined by the CMA
 *	architecture specification.
 *
 *      The user action associated with a call to this routine depends on
 *  	the particular status code passed in.  Be sure to use the
 *	appropriate status code and/or modify the manual text in the file
 *	CMA_MESSAGES*.GNM so that when the client gets the error it is
 *	clear what action is to be taken.
 *
 *  FORMAL PARAMETERS:
 *
 *	code  	An integer "status code" in a format appropriate to the
 *		current host.
 *
 *  IMPLICIT INPUTS:
 *
 *	NONE
 *
 *  IMPLICIT OUTPUTS:
 *
 *	NONE
 *
 *  FUNCTION VALUE:
 *
 *	NONE
 *
 *  SIDE EFFECTS:
 *
 *	The error is reported as appropriate to the current host.
 *	Typically, an exception is raised in the current thread.
 */
extern void
cma__error
#ifdef _CMA_PROTO_
	(
	cma_t_status	code)
#else	/* no prototypes */
	(code)
	cma_t_status	code;
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    extern void
    lib$stop _CMA_PROTOTYPE_ ((
	cma_t_status	code,
	cma_t_integer	count,
	cma_t_integer	fill1,
	cma_t_integer	fill2));

    /*
     * Signal the error, adding several 0s. The first specifies that the
     * condition code has no arguments. The second specifies a secondary
     * condition code of "0" (with sys$putmsg will ignore); otherwise,
     * sys$putmsg might find non-zero codes in whatever happens to follow the
     * argument list on the stack. The third should be unnecessary, but
     * provides a safety net (either an addition null status code, or an
     * argument count of 0 to the first null status code, depending on
     * implementation).
     */
    lib$stop (code, 0, 0, 0);
#else
    cma__assert_not_kernel ();
    exc_raise_status (code);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This function is used by the transfer vector to report unimplemented
 *	entry points.
 *
 *  FORMAL PARAMETERS:
 *
 * 	none
 *
 *  IMPLICIT INPUTS:
 *
 * 	none
 *
 *  IMPLICIT OUTPUTS:
 *
 * 	none
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
cma__unimplemented
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__error (cma_s_unimp);
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ERRORS.C */
/*  *35    1-JUN-1993 11:08:07 BUTENHOF "Handle VP spinlock on debug/bugcheck" */
/*  *34   14-MAY-1993 15:55:37 BUTENHOF "Use -z in output" */
/*  *33   10-MAY-1993 14:11:06 BUTENHOF "Clean up bugcheck formatting" */
/*  *32    6-MAY-1993 19:06:55 BUTENHOF "Use TIS disable" */
/*  *31   30-APR-1993 18:14:08 BUTENHOF "Use uname(), etc" */
/*  *30   26-FEB-1993 10:06:49 BUTENHOF "Fix AXP compilation error" */
/*  *29   28-JAN-1993 14:42:08 BUTENHOF "Turn TIS locking off on bugcheck" */
/*  *28   19-JAN-1993 12:56:35 BUTENHOF "Handle dump file open error" */
/*  *27   16-OCT-1992 12:19:42 BUTENHOF "Get rid of 0-filled addresses" */
/*  *26   28-SEP-1992 11:49:12 BUTENHOF "Disable timeslicing rather than locking kernel" */
/*  *25    4-SEP-1992 14:42:19 BUTENHOF "Change 'Alpha' to 'Alpha AXP'" */
/*  *24   21-AUG-1992 13:41:58 BUTENHOF "Prevent recursive bugcheck" */
/*  *23    4-AUG-1992 11:04:52 BUTENHOF "64 bit address printout" */
/*  *22   17-JUL-1992 17:31:23 KEANE "Fix report attr. object display" */
/*  *21   10-JUL-1992 07:59:41 BUTENHOF "Report attr. objs on bucheck" */
/*  *20   16-JUN-1992 06:49:00 BUTENHOF "Fix bugcheck code" */
/*  *19   10-JUN-1992 14:03:04 BUTENHOF "Fix enter kernel code" */
/*  *18    9-JUN-1992 12:10:46 BUTENHOF "Add current thread on bugcheck" */
/*  *17   17-APR-1992 11:11:02 BUTENHOF "Add version number string" */
/*  *16    1-APR-1992 07:53:52 BUTENHOF "Remove ""error"" from bugcheck -- for DTM" */
/*  *15   24-MAR-1992 13:46:19 BUTENHOF "Put bugcheck output in file" */
/*  *14   17-MAR-1992 09:55:59 BUTENHOF "Abort process on bugcheck" */
/*  *13   28-JAN-1992 06:40:17 BUTENHOF "Fix compilation error" */
/*  *12   27-JAN-1992 05:50:00 BUTENHOF "Pad lib$stop in cma__error with nulls" */
/*  *11   16-DEC-1991 12:55:19 BUTENHOF "Specify 0 argument count on cma__error()" */
/*  *10   27-NOV-1991 11:03:35 BUTENHOF "Remove globalvalue on cma$s_bugcheck" */
/*  *9    18-NOV-1991 11:07:55 CURTIN "Added the use of an Alpha switch." */
/*  *8    11-JUN-1991 17:16:55 BUTENHOF "Add & use functions to dump kernel/sem trace arrays" */
/*  *7    10-JUN-1991 18:21:44 SCALES "Add sccs headers for Ultrix" */
/*  *6    29-MAY-1991 17:14:38 BUTENHOF "Put some meat on the bugcheck printout" */
/*  *5    10-MAY-1991 17:52:01 BUTENHOF "Use new macro to test for kernel locked" */
/*  *4     2-MAY-1991 13:58:12 BUTENHOF "Add string argument to cma__bugcheck()" */
/*  *3    12-APR-1991 23:35:49 BUTENHOF "Change references to kernel_critical" */
/*  *2     1-APR-1991 18:08:29 BUTENHOF "QAR 93, exception text" */
/*  *1    12-DEC-1990 21:45:26 BUTENHOF "Error handling" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ERRORS.C */
