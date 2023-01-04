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
static char *rcsid = "@(#)$RCSfile: cma_init.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:47:51 $";
#endif
/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	Initialize CMA
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	19 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	11 September 1989
 *		Implement cma_init external routine
 *	002	Hans Oser	26 September 1989
 *		Additions for thread and dispatch initialisation
 *	003	Dave Butenhof	30 October 1989
 *		Initialize timeslicing (unconditional) and nonblocking I/O
 *		(on unix systems).
 *	004	Dave Butenhof	1 November 1989
 *		Initialize known CV & Mutex queues in init_static to avoid
 *		problems with init dependencies.
 *	005	Webb Scales	7 November 1989
 *		Added cma__init_assem for VMS and enabled timeslicing for
 *		MIPS
 *	006	Webb Scales	10 November 1989
 *		Moved cma__init_assem to earlier in the initialization
 *		sequence  as it must be called before any (including null)
 *		threads are created.
 *	007	Dave Butenhof	17 November 1989
 *		Include cma_condition.h explicitly, since cma_tcb.h no longer
 *		does so.
 *	008	Dave Butenhof	8 December 1989
 *		Change cma_init to cma_shr_init (primary cma_init will now be
 *		linked into client image to do exception initialization as
 *		well as old "cma_init" even when CMA is embedded in a
 *		shareable image.
 *	009	Webb Scales	10 December 1989
 *		Remove references to (defunct) low-level thread I/O stuff.
 *	010	Bob Conti	10 December 1989
 *		Replace call to cma__init_exceptions with call to
 *		cma__init_impl_exc, which just initializes those exceptions
 *		that the CMA implementation can RAISE.   The CMA
 *		implementation must use the CMA__ERROR calls rather  than
 *		raising exceptions directly.  (Keeping the unused exception
 *		objects uninitialized will catch accidental use.) Added
 *		one-time flag to prevent accidental re-initialization  in the
 *		middle of a running program; this makes cma_init modular.
 *	011	Dave Butenhof	11 December 1989
 *		Change name of cma_client_init.h to cma_client.h, and change
 *		back to using cma__init_exceptions so CMA can CATCH
 *		exceptions.
 *	012	Dave Butenhof	26 January 1990
 *		Change initialization of stack chain (it's now a list, not a
 *		queue).
 *	013	Dave Butenhof	2 March 1990
 *		Integrate Kevin Ackley's changes for Apollo M68000 port.
 *	014	Webb Scales	22 March 1990
 *		Add signal module initialization.
 *	015	Dave Butenhof	9 April 1990
 *		New encapsulation for known object structures.
 *	016	Dave Butenhof	31 May 1990
 *		Translate environment variables at initialization.
 *	017	Dave Butenhof	7 June 1990
 *		Change environment variables to upper case.
 *	018	Dave Butenhof	27 June 1990
 *		Remove call to cma__init_vm (no longer needed).
 *	019	Bob Conti	5 July 1990
 *		Adapt cma_shr_init to changes made in CMA_CLIENT.C
 *	020	Paul Curtin     1 August 1990
 *		Added memory initialization routine: cma__init_memory
 *		replaced atoi w/ cma__atoi
 *	021	Webb Scales	16 August 1990
 *		Changed #ifdef's to #if's
 *	022	Dave Butenhof	28 August 1990
 *		Change cma_shr_init to cma__shr_init, since it's really an
 *		internal interface (even though it's called from the client
 *		image and must be in the transfer vector, no client should
 *		call it directly).
 *	023	Webb Scales	29 August 1990
 *		Replace instance of RAISE with call to cma__error.
 *	024	Bob Conti	1 October 1990
 *		Move insertion of main tcb on known threads queue to cma_tcb
 *	025	Dave Butenhof	9 October 1990
 *		Add (null) argument to cma__init_exceptions call, since it's
 *		also used as a cma_once init routine (from cma_init()), and
 *		therefore expects to be passed an argument (which it doesn't
 *		use).
 *	026	Dave Butenhof	22 January 1991
 *		Merge cma_init back into this module (should have been done a
 *		while ago when we removed the requirement that client link
 *		against cma_client.obj).
 *	027	Paul Curtin	24 January 1991
 *		Remove include of cma_client.h
 *	028	Dave Butenhof	24 January 1991
 *		Fix up initialization of exceptions (some status names use
 *		"cma" where they should use "exc").
 *	029	Dave Butenhof	18 March 1991
 *		Define cma_g_debug for UNIX systems.
 *	030	Dave Butenhof	27 March 1991
 *		Change initialization of exceptions to use new names.
 *	031	Dave Butenhof	12 April 1991
 *		Support OSF/1 init, including providing synchronization
 *		functions and per-thread errno to reentrant libraries.
 *	032	Dave Butenhof	24 April 1991
 *		Add function prefix to cma__trace messages.
 *	033	Paul Curtin	 7 May 1991
 *		Reworked cma_init to operate thru a macro: cma__int_init
 *	034	Paul Curtin	06 June 1991
 *		Added to init process, cma__init_atfork.
 *	035	Webb Scales and Dave Butenhof	    10 June 1991
 *		Conditionalize inclusion of I/O stuff.
 *	036	Dave Butenhof	16 September 1991
 *		To make the UNIX file I/O wrappers more efficient, the
 *		(possibly large) mask arrays are allocated dynamically at TCB
 *		creation. For this to work, cma_init_tcb() needs to know the
 *		correct size; since cma_init_thread_io() is usually run
 *		later, move the size calculation (getdtablesize()) here.
 *	037	Dave Butenhof	04 October 1991
 *		Clean up use of _CMA_UNIPROCESSOR_
 *	038	Dave Butenhof	16 October 1991
 *		Add a typecast in cma___get_self_tcb().
 *	039	Dave Butenhof	05 November 1991
 *		Integrate Dave Weisman's changes; build DECthreads on OSF/1
 *		without Mach threads or libc_r.a
 *	040	Dave Butenhof	25 November 1991
 *		Add cma_g_debug_cmd pointer.
 *	041	Dave Butenhof	26 November 1991
 *		Change PLATFORM conditional to OS, to support Alpha VMS.
 *	042	Dave Butenhof	18 December 1991
 *		MIPS C just decided to complain about cma_g_debug_cmd typing;
 *		it's right, so I'll fix it.
 *	043	Dave Butenhof	18 December 1991
 *		Change interface to cma__getenv to make it cleanly reentrant.
 *	044	Dave Butenhof	24 January 1992
 *		Disable declaration of extern exceptions in exc_handling.h so
 *		that all exceptions may be statically initialized as
 *		exc_kind_status_t structures without type conflicts.
 *	045	Dave Butenhof	12 March 1992
 *		Fix trace sequencing around atfork init.
 *	046	Dave Butenhof	19 March 1992
 *		Add debugging code to check libc_r.a mutex access on OSF/1.
 *	047	Dave Butenhof	02 April 1992
 *		Use _pthread_init_routine hook for automatic crt0.o init on
 *		OSF/1.
 *	048	Dave Butenhof	17 April 1992
 *		Include a version string from cma_version.h
 *	049	Webb Scales	24 April 1992
 *		Unconditionalize signal module init call.
 *	050	Paul Curtin	20 May 1992
 *		Add cma__find_persistent_frame for OpenVMS Alpha
 *	051	Paul Curtin	21 May 1992
 *		Make include for libicb.h OpenVMS Alpha specific.
 *	052	Webb Scales	21 May 1992
 *		Add call to cma_tis_support module init routine; move the
 *		TIS/libc_r wrapper routines to the cma_tis_support module.
 *	053	Paul Curtin	11 June 1992
 *		Removed cma__initialize routine.
 *	054	Paul Curtin & Webb Scales	20 July 1992
 *		Removed a reference to malloc.
 *	055	Dave Butenhof	30 July 1992
 *		Initialize memory manager (single-thread) prior to
 *		initializing "environment" -- which may open a file, which
 *		needs VM.
 *	056	Dave Butenhof	13 August 1992
 *		For Alpha OSF/1, initialize main thread's PAL unique value.
 *	057	Dave Butenhof	25 August 1992
 *		cma__int_init() is now vapor on VMS and OSF/1, so cma_init is
 *		better off not using it (since both LIB$INITIALIZE and
 *		_pthread_init point to cma_init!)
 *	058	Webb Scales	 3 September 1992
 *		Add scheduler module initialization.
 *	059	Dave Butenhof	15 September 1992
 *		Get rid of "sequence object" (it's in known object
 *		structure).
 *	060	Dave Butenhof	 2 October 1992
 *		On systems with _CMA_RT4_KTHREAD_ defined, set up extern
 *		variables for the cma_c_prio_* symbols, since we need to call
 *		the system to find the value. Using externs seems better than
 *		#defining the symbols to call sched_get_priority_{min|max}()
 *		directly.
 *	061	Dave Butenhof	 5 October 1992
 *		Initialize defer bits directly to make sure they're set by
 *		the first enter_kernel operation.
 *	062	Paul Curtin	 6 October 1992
 *		Merge in work Cactus stack changes from EVMS.
 *	063	Dave Butenhof	15 October 1992
 *		Call cma__init_sched even on DEC OSF/1 AXP -- it creates the
 *		default VP structure.
 *	064	Dave Butenhof	20 November 1992
 *		Upgrade cma_c_prio* symbols to cma_t_integer (long).
 *	065	Webb Scales	23 November 1992
 *		Initialize TIS before dispatch for errno handling on VMS.
 *	066	Dave Butenhof	25 November 1992
 *		Remove externs for OSF/1 RT priorities, since they're really
 *		constants anyway.
 *	067	Dave Butenhof	10 December 1992
 *		Fix pointer type problem (Mach).
 *	068	Webb Scales	 8 February 1993
 *		Add defer initialization for non-Kthread, U*ix platforms
 *	069	Dave Butenhof	 1 March 1993
 *		Add support for a new internal baselevel release procedure:
 *		users must define a logical name (env. variable) explicitly
 *		confirming that they will not pass on the copy.
 *	070	Dave Butenhof	 2 March 1993
 *		Modify 069 -- use exit(-1) instead of aborting with a signal
 *		on UNIX.
 *	071	Dave Butenhof	 4 March 1993
 *		Fix 069 comment (logical name check goes in /final, not
 *		/debug).
 *	072	Dave Butenhof	 9 March 1993
 *		Fix sprintf error check from 069.
 *	073	Brian Keane	22 April 1993
 *		Make cma___init_env() compile only if not NDEBUG.
 *	074	Dave Butenhof	 3 May 1993
 *		Initialize cma__g_last_thread with cma__g_known_threads.queue
 *	075	Dave Butenhof	 4 May 1993
 *		OK, OK -- so 074 only caught the more obscure of the two
 *		places where last_thread should be initialized (fake lib_init
 *		for CMA_NOINIT).
 *	076	Dave Butenhof	 5 May 1993
 *		Integrate invariant QUIPU logical analyzer support
 *	078	Dave Butenhof	14 May 1993
 *		Track first dead thread rather than last living thread
 *		(cma__g_last_thread is now cma__g_dead_zone) -- otherwise,
 *		destroy_tcb and free_tcb would have to enter kernel and check
 *		pointer to avoid moving last_thread onto cache or pool list!
 *	079	Dave Butenhof	21 May 1993
 *		Set unique value at cma_init() on DEC OSF/1, mostly so QUIPU
 *		code will work.
 *	080	Dave Butenhof	23 June 1993
 *		On OSF/1, don't init values that aren't used!
 */

/*
 *  INCLUDE FILES
 */

#define _EXC_NO_EXCEPTIONS_		/* tell exc_handling.h to skip exports */
#include <cma.h>
#undef _EXC_NO_EXCEPTIONS_
#include <cma_assem.h>
#include <cma_defs.h>
#include <cma_defer.h>
#include <cma_init.h>
#include <cma_condition.h>
#include <cma_once.h>
#include <cma_tcb.h>
#include <cma_context.h>
#include <cma_dispatch.h>
#include <cma_timer.h>
#include <cma_stack.h>
#include <cma_queue.h>
#include <cma_list.h>
#include <cma_vm.h>
#include <cma_deb_core.h>
#include <cma_util.h>
#include <cma_int_errno.h>
#include <cma_version.h>
#include <cma_sched.h>
#include <cma_tis_sup.h>

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
# include <cma_thread_io.h>
#endif

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
# include <libicb.h>
#endif

#if _CMA_PLATFORM_ != _CMA__VAX_VMS
# include <pthread.h>
#endif

#ifdef _CMA_QUIPU_
# include <fcntl.h>
# include <sys/types.h>
# include <stdio.h>
# include <sys/mman.h>
# include <errno.h>
#endif

/*
 *  LOCAL MACROS
 */

/*
 *  GLOBAL DATA
 */


cma__t_atomic_bit	cma__g_init_started = cma__c_tac_static_clear;
cma__t_atomic_bit	cma__g_init_done = cma__c_tac_static_clear;

#ifdef _CMA_QUIPU_
int			cma__g_quipu_fudge;
int			*cma__g_quipu = &cma__g_quipu_fudge;
#endif

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
cma_t_address   cma__g_base_frame;
#endif

#if _CMA_PLATFORM_ != _CMA__VAX_VMS
/*
 * Define the CMA and pthread symbols which can be referenced by the client
 * program.  On OpenVMS VAX, these symbols are defined in the shareable image
 * transfer vector, not here, but the OpenVMS AXP symbol vector requires them
 * to be defined here.
 */

/*
 * Define pointer to cma_debug; this ensures that cma_debugger.o is included
 * in client a.out files, and that the user can call it manually from the
 * debugger.
 */
_CMA_EXPORT_ void (*cma_g_debug) _CMA_PROTOTYPE_ ((void)) = cma_debug;
_CMA_EXPORT_ void (*cma_g_debug_cmd) _CMA_PROTOTYPE_ ((char *)) = cma_debug_cmd;

/*
 * Predefined handles used by CMA and clients: the CMA "null handle", and the
 * POSIX 1003.4a default attributes object handles.
 */

_CMA_EXPORT_ cma_t_handle		cma_c_null 		  = {0, 0};
_CMA_EXPORT_ pthread_attr_t		pthread_attr_default 	  = {0, 0};
_CMA_EXPORT_ pthread_mutexattr_t	pthread_mutexattr_default = {0, 0};
_CMA_EXPORT_ pthread_condattr_t		pthread_condattr_default  = {0, 0};

/* 
 * CMA exceptions
 */

_CMA_EXPORT_ exc_kind_status_t
    exc_e_uninitexc = {exc_kind_status_c, exc_s_uninitexc, {exc_newexc_c, exc_v2exc_c}},

    exc_e_illaddr = {exc_kind_status_c, exc_s_illaddr, {exc_newexc_c, exc_v2exc_c}},
    exc_e_exquota = {exc_kind_status_c, exc_s_exquota, {exc_newexc_c, exc_v2exc_c}},
    exc_e_insfmem = {exc_kind_status_c, exc_s_insfmem, {exc_newexc_c, exc_v2exc_c}},
    exc_e_nopriv = {exc_kind_status_c, exc_s_nopriv, {exc_newexc_c, exc_v2exc_c}},
    exc_e_illinstr = {exc_kind_status_c, exc_s_illinstr, {exc_newexc_c, exc_v2exc_c}},
    exc_e_resaddr = {exc_kind_status_c, exc_s_resaddr, {exc_newexc_c, exc_v2exc_c}},
    exc_e_privinst = {exc_kind_status_c, exc_s_privinst, {exc_newexc_c, exc_v2exc_c}},
    exc_e_resoper = {exc_kind_status_c, exc_s_resoper, {exc_newexc_c, exc_v2exc_c}},
    exc_e_SIGTRAP = {exc_kind_status_c, exc_s_SIGTRAP, {exc_newexc_c, exc_v2exc_c}},
    exc_e_SIGIOT = {exc_kind_status_c, exc_s_SIGIOT, {exc_newexc_c, exc_v2exc_c}},
    exc_e_SIGEMT = {exc_kind_status_c, exc_s_SIGEMT, {exc_newexc_c, exc_v2exc_c}},
    exc_e_aritherr = {exc_kind_status_c, exc_s_aritherr, {exc_newexc_c, exc_v2exc_c}},
    exc_e_SIGSYS = {exc_kind_status_c, exc_s_SIGSYS, {exc_newexc_c, exc_v2exc_c}},
    exc_e_SIGPIPE = {exc_kind_status_c, exc_s_SIGPIPE, {exc_newexc_c, exc_v2exc_c}},
    exc_e_excpu = {exc_kind_status_c, exc_s_excpu, {exc_newexc_c, exc_v2exc_c}},
    exc_e_exfilsiz = {exc_kind_status_c, exc_s_exfilsiz, {exc_newexc_c, exc_v2exc_c}},
    exc_e_intovf = {exc_kind_status_c, exc_s_intovf, {exc_newexc_c, exc_v2exc_c}},
    exc_e_intdiv = {exc_kind_status_c, exc_s_intdiv, {exc_newexc_c, exc_v2exc_c}},
    exc_e_fltovf = {exc_kind_status_c, exc_s_fltovf, {exc_newexc_c, exc_v2exc_c}},
    exc_e_fltdiv = {exc_kind_status_c, exc_s_fltdiv, {exc_newexc_c, exc_v2exc_c}},
    exc_e_fltund = {exc_kind_status_c, exc_s_fltund, {exc_newexc_c, exc_v2exc_c}},
    exc_e_decovf = {exc_kind_status_c, exc_s_decovf, {exc_newexc_c, exc_v2exc_c}},
    exc_e_subrng = {exc_kind_status_c, exc_s_subrng, {exc_newexc_c, exc_v2exc_c}},

    cma_e_alerted = {exc_kind_status_c, cma_s_alerted, {exc_newexc_c, exc_v2exc_c}},
    cma_e_assertion = {exc_kind_status_c, cma_s_assertion, {exc_newexc_c, exc_v2exc_c}},
    cma_e_badparam = {exc_kind_status_c, cma_s_badparam, {exc_newexc_c, exc_v2exc_c}},
    cma_e_bugcheck = {exc_kind_status_c, cma_s_bugcheck, {exc_newexc_c, exc_v2exc_c}},
    cma_e_exit_thread = {exc_kind_status_c, cma_s_exit_thread, {exc_newexc_c, exc_v2exc_c}},
    cma_e_existence = {exc_kind_status_c, cma_s_existence, {exc_newexc_c, exc_v2exc_c}},
    cma_e_in_use = {exc_kind_status_c, cma_s_in_use, {exc_newexc_c, exc_v2exc_c}},
    cma_e_use_error = {exc_kind_status_c, cma_s_use_error, {exc_newexc_c, exc_v2exc_c}},
    cma_e_wrongmutex = {exc_kind_status_c, cma_s_wrongmutex, {exc_newexc_c, exc_v2exc_c}},
    cma_e_stackovf = {exc_kind_status_c, cma_s_stackovf, {exc_newexc_c, exc_v2exc_c}},
    cma_e_nostackmem = {exc_kind_status_c, cma_s_nostackmem, {exc_newexc_c, exc_v2exc_c}},
    cma_e_notcmastack = {exc_kind_status_c, cma_s_notcmastack, {exc_newexc_c, exc_v2exc_c}},
    cma_e_unimp = {exc_kind_status_c, cma_s_unimp, {exc_newexc_c, exc_v2exc_c}},
    cma_e_inialrpro = {exc_kind_status_c, cma_s_inialrpro, {exc_newexc_c, exc_v2exc_c}},
    cma_e_defer_q_full = {exc_kind_status_c, cma_s_defer_q_full, {exc_newexc_c, exc_v2exc_c}},
    cma_e_signal_q_full = {exc_kind_status_c, cma_s_signal_q_full, {exc_newexc_c, exc_v2exc_c}},
    cma_e_alert_nesting = {exc_kind_status_c, cma_s_alert_nesting, {exc_newexc_c, exc_v2exc_c}};
#endif					/* if not VAX/VMS */

/*
 * FIX-ME:  These should be declared in their own internal header files.
 */

#if _CMA_OS_ != _CMA__VMS
extern void
cma__init_atfork _CMA_PROTOTYPE_ ((void));
#endif

cma__t_env	cma__g_env[cma__c_env_count] = {
    /*
     * The following highwater marks determine the maximum number of objects
     * which can be cached (of each type) on a particular attributes object;
     * if the number of cached objects exceeds the "max" value, objects will
     * be destroyed until the number is reduced to the "min" value.
     */
    {"CMA_MAXATTR", cma__c_env_type_int, 20},	/* If more attributes than this, */
    {"CMA_MINATTR", cma__c_env_type_int, 5},	/* . purge back to this */
    {"CMA_MAXTHREAD", cma__c_env_type_int, 20},	/* If more threads than this */
    {"CMA_MINTHREAD", cma__c_env_type_int, 5},	/* . purge back to this */
    /*
     * End of per-attributes object watermarks
     */
    {"CMA_TRACE", cma__c_env_type_file, 0}	/* Enable tracing in debug build */
    };

char	*cma__g_version = cma__c_version;

/*
 *  LOCAL DATA
 */

static char		*cma___g_version = cma__c_vref;
static cma_t_boolean	cma___g_fake_libinit = cma_c_false;

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
# define cma___c_persistent     1
#endif

/*
 * LOCAL FUNCTIONS
 */

#ifndef NDEBUG
static void
cma___init_env _CMA_PROTOTYPE_ ((void));	/* Get environment variables */
#endif


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize the CMA shared library.
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
cma_init
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {

    if (!cma__tac_isset(&cma__g_init_started)) {

	if (!cma__test_and_set (&cma__g_init_started)) {
	    cma__init_static ();
	    cma__test_and_set (&cma__g_init_done);
	    }
	else if (!cma__tac_isset (&cma__g_init_done))
	    cma__error (cma_s_inialrpro);

	}

    }

#if _CMA_OS_ == _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Called from OpenVMS lib$initialize to conditionally init DECthreads
 *	at image activation time, unless the logical CMA_NOINIT is defined.
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
cma_lib_init
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    char		translation[512], *result;
    cma__t_int_tcb	*tcb;


    /*
     * Translate the logical name -- only if it's not defined do we actually
     * make the call to initialize DECthreads. Otherwise, we rely on the main
     * program to call it later.
     */
    translation[0] = '\0';
    result = cma__getenv ("CMA_NOINIT", translation, 512);

    if (!result)
	cma_init ();
    else {
	cma___g_fake_libinit = cma_c_true;
#if _CMA_THREAD_IS_VP_
# if _CMA_HARDWARE_ == _CMA__ALPHA && _CMA_OSIMPL_ == _CMA__OS_OSF
	cma__set_unique ((long int)&cma__g_def_tcb);
# endif
#endif
#if !_CMA_THREAD_IS_VP_
	cma__tac_set (&cma__g_mgr_wake);
	cma__tac_set (&cma__g_cv_defer);
#endif
	cma__queue_init (&cma__g_known_atts.queue);
	cma__queue_init (&cma__g_known_cvs.queue);
	cma__queue_init (&cma__g_known_mutexes.queue);
	cma__queue_init (&cma__g_known_threads.queue);
	cma__g_dead_zone = &cma__g_known_threads.queue;
	cma__list_init (&cma__g_stack_clusters);
	cma__g_known_mutexes.sequence = 1;
	cma__init_memory ();		/* Initialize VM queues! (only) */
#ifndef NDEBUG
	cma___init_env ();
#endif
	/*
	 * Do enough with default TCB to satisfy DEBUG until a GO
	 */
	tcb = &cma__g_def_tcb;
	tcb->header.sequence 	= 1;
	tcb->header.type	= cma__c_obj_tcb;
	tcb->state		= cma__c_state_ready;
	tcb->prolog.sentinel    = cma_c_tcb_sentinel;
	tcb->prolog.reserved1   = cma_c_null_ptr;
	tcb->prolog.client_key  = (cma_t_key)0;
	tcb->debug.on_hold	= cma_c_false;
	tcb->debug.activated	= cma_c_true;
	tcb->debug.did_preempt	= cma_c_false;
	tcb->debug.start_pc	= (cma_t_address)0;
	tcb->debug.object_addr  = (cma_t_address)0;
	tcb->debug.substate	= cma__c_substa_normal;
	tcb->debug.notify_debugger = cma_c_false;
	tcb->context_count	= 0;
	tcb->mutex		= (cma__t_int_mutex *)0;
	tcb->wait_cv		= (cma__t_int_cv *)cma_c_null_ptr;
	tcb->wait_mutex		= (cma__t_int_mutex *)cma_c_null_ptr;
	tcb->exc_stack		= (exc_context_t *)cma_c_null_ptr;
	tcb->contexts		= (cma__t_context_list)0;
	cma__tac_set (&tcb->alert_pending);
	tcb->alert.g_enable = cma_c_true;
	tcb->alert.a_enable = cma_c_false;
	tcb->alert.count = 0;
	cma__queue_zero (&tcb->threads);
	cma__queue_zero (&tcb->header.queue);
	cma__queue_init (&tcb->stack);
	cma__object_to_handle (
	    (cma__t_object *)tcb,
	    (cma_t_handle *)&(tcb->prolog.client_thread));
	cma__trace ((cma__c_trc_init, "(cma_lib_init) Done with fake init"));
	}

    }
#endif

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__find_persistent_frame - Walks default stack until it finds
 *	the base (a frame/descriptor with a saved FP of 0) and then from
 *	there determines frame/descriptor which is persistent in regards
 *	to the debugger's handler.
 *
 *  FORMAL PARAMETERS:
 *
 *	None
 *
 *  IMPLICIT INPUTS:
 *
 *	Invocation context of current call (Frame/Descriptor).
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Address of persistent frame. (cma__g_base_frame).
 *
 *  FUNCTION VALUE:
 *
 *	Void
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma__find_persistent_frame
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    struct invo_context_blk     invocation_context;
    cma_t_integer               depth;
    cma_t_integer               index;
                                          

    /*
     * Grab out current invocation context.
     */
    lib$get_current_invo_context (&invocation_context);

    /*
     * Find the bottom of the stack.
     */
    depth = 0;
    while (lib$get_previous_invo_context (&invocation_context) != 0) {
	depth = depth + 1;
	}

    /*
     * Grab out current invocation context, again.
     */
    lib$get_current_invo_context (&invocation_context);

    /*
     * Find the frame/descriptor holding the saved FP we desire.
     *
     * FIX-ME*** - At the moment this algorithm is hardwired to
     * to make the persistent frame be the second frame on the stack -
     * presumably IMGSTA.  When the OS groups gets around to implementing
     * the base frame bit in image start up then this algorithm should
     * make the persistent frame be that which has the base frame bit
     * set.  Note, this will not conflict with our thread stacks having
     * the base frame bit set because this routine should only be called 
     * during initialization prior to the creation of any of our stacks.
     */


    while (depth > cma___c_persistent) {
	    lib$get_previous_invo_context (&invocation_context);
	    depth--;
	}

    /*
     * Place the FP pointing at the persistent frame/descriptor in
     * a global.
     */

    cma__g_base_frame = 
		(cma_t_address)(invocation_context.libicb$q_ireg[29]);


   }

#endif


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize all CMA static data at startup time.
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
cma__init_static
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_THREAD_IS_VP_
# if _CMA_HARDWARE_ == _CMA__ALPHA && _CMA_OSIMPL_ == _CMA__OS_OSF
    cma__set_unique ((long int)&cma__g_def_tcb);
# endif
#endif
#ifdef _CMA_QUIPU_
    {
    int		fd;
    size_t	size;


    /*
     * Load up a "fudge" address to make sure geterrno, seterrno, and stuff
     * like that don't blow up before we get started.
     */
    size = (u_long)getpagesize ();

    /*
     * Open the logic analyzer 'lad'.
     * The 'lad' device must be manually created...
     *      > mknod /dev/lad c 34 0
     */
    fd = open ("/dev/lad", O_RDWR);

    if (fd == (int *)-1) {
	fprintf (stderr, "***********************\n");
	fprintf (stderr, "* unable to open...\n");
	fprintf (stderr, "* %d %s\n",errno,sys_errlist[errno]);
	fprintf (stderr, "* using internal fudge...\n");
	fprintf (stderr, "***********************\n");
	}
    else {

	/*
	 * Map the 'lad'. 'cma__g_quipu' is the mapped address used to access
	 * the logic analyzer.
	 */
	cma__g_quipu = (int *)mmap(
		(caddr_t)0,
		size,
		PROT_READ|PROT_WRITE,MAP_FILE|MAP_SHARED,
		fd,
		(off_t)0);

	if (cma__g_quipu == (int *)-1) {
	    cma__g_quipu = &cma__g_quipu_fudge;	/* Restore the fudge */
	    fprintf (stderr, "***********************\n");
	    fprintf (stderr, "* unable to mmap...\n");
	    fprintf (stderr, "* %d %s\n",errno,sys_errlist[errno]);
	    fprintf (stderr, "* using internal fudge...\n");
	    fprintf (stderr, "***********************\n");
	    }

	close(fd);
	}

    }
#endif

    /*
     * Initialize the environment first, specifically to get the value of the
     * trace file (if any). This assumes that cma___init_env isn't dependent
     * on ANY DECthreads features other than single-thread VM.
     */

    /*
     * If we ran a "fake" lib$initialize (cma_lib_init) due to CMA_NOINIT
     * being defined, skip the things we faked to allow debug to start up.
     */
    if (!cma___g_fake_libinit) {
#if !_CMA_THREAD_IS_VP_
	cma__tac_set (&cma__g_mgr_wake);
	cma__tac_set (&cma__g_cv_defer);
#endif
	cma__queue_init (&cma__g_known_atts.queue);
	cma__queue_init (&cma__g_known_cvs.queue);
	cma__queue_init (&cma__g_known_mutexes.queue);
	cma__queue_init (&cma__g_known_threads.queue);
	cma__g_dead_zone = &cma__g_known_threads.queue;
	cma__list_init (&cma__g_stack_clusters);
	cma__g_known_mutexes.sequence = 1;
	cma__init_memory ();		/* Initialize VM queues! (only) */
#ifndef NDEBUG
	cma___init_env ();
#endif
	}

    cma__trace ((
	    cma__c_trc_init | cma__c_trc_vm,
	    "(init_static) page size = %ld bytes",
	    cma__g_page_size));

#if _CMA_OS_ == _CMA__VMS
    cma__find_persistent_frame ();
#endif

    /*
     * WARNING:
     *
     * Do not EVER change the order of these calls unless you are ABSOLUTELY
     * certain that you know exactly what every one does!  There are many
     * interdependencies, and some (particularly the first few) use special
     * "bootstrap" versions of some calls to get their job done.  Changing
     * the order could be MUCH more complicated than it might seem!
     *
     * In general, new initializations should be added at the END; if this
     * won't do, be VERY, VERY, VERY CAREFUL!  Exception: cma__init_timer
     * should always remain at the END, to ensure that timer events can't
     * occur while other modules are still initializing!
     */
#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
    cma__trace ((cma__c_trc_init, "(init_static) Get file table size"));
    cma__g_mx_file = getdtablesize ();
    cma__g_nspm = ((cma__g_mx_file + cma__c_nbpm - 1)/cma__c_nbpm);
    cma__trace ((
	    cma__c_trc_init,
	    "(init_static)  %d file descriptors; mask is %d ints",
	    cma__g_mx_file,
	    cma__g_nspm));
    cma__tac_set (&cma__g_defer_avail);
#endif
#if _CMA_PLATFORM_ == _CMA__VAX_VMS
    cma__trace ((cma__c_trc_init, "(init_static) VAX/VMS assembler code"));
    cma__init_assem ();
#endif
    cma__trace ((cma__c_trc_init, "(init_static) attributes"));
    cma__init_attr ();
    cma__trace ((cma__c_trc_init, "(init_static) mutexes"));
    cma__init_mutex ();
    cma__trace ((cma__c_trc_init, "(init_static) memory management locks"));
    cma__init_mem_locks ();
    cma__trace ((cma__c_trc_init, "(init_static) per-thread context"));
    cma__init_context ();
    cma__trace ((cma__c_trc_init, "(init_static) condition variables"));
    cma__init_cv ();
#if _CMA_OS_ != _CMA__VMS
    cma__trace ((cma__c_trc_init, "(init_static) atfork init"));
    cma__init_atfork ();
#endif
    cma__trace ((cma__c_trc_init, "(init_static) one-time init"));
    cma__init_once ();
    cma__trace ((cma__c_trc_init, "(init_static) stacks"));
    cma__init_stack ();
#if _CMA_KTHREADS_ != _CMA__NONE
    cma__trace ((cma__c_trc_init, "(init_static) VP layer"));
    cma__init_vp ();
#endif
    cma__trace ((cma__c_trc_init, "(init_static) tcb management"));
    cma__init_tcb ();
    cma__trace ((cma__c_trc_init, "(init_static) TIS hooks"));
    cma__init_tis();
    cma__trace ((cma__c_trc_init, "(init_static) dispatcher"));
    cma__init_dispatch ();
    cma__trace ((cma__c_trc_init, "(init_static) scheduler"));
    cma__init_sched ();
#if _CMA_OS_ == _CMA__UNIX
# if !_CMA_THREAD_SYNC_IO_
    cma__trace ((cma__c_trc_init, "(init_static) thread synchronous I/O"));
    cma__init_thread_io ();
# endif
# if !_CMA_THREAD_IS_VP_
    cma__trace ((cma__c_trc_init, "(init_static) deferral module"));
    cma__init_defer ();
# endif
#endif
    cma__trace ((cma__c_trc_init, "(init_static) signal handlers"));
    cma__init_signal ();
    cma__trace ((cma__c_trc_init, "(init_static) debug code"));
    cma__init_debug ();			/* Must be after mutexes */
    cma__trace ((cma__c_trc_init, "(init_static) timers"));
    cma__init_timer ();
    cma__trace ((cma__c_trc_init, "(init_static) initialization complete"));

#ifdef _CMA_RESTRICTED_USE_
    /*
     * If this is a final build, check for the environment variable (or VMS
     * logical name) "CMA_RESTRICTED_DISTRIBUTION". It must be defined to the
     * text "DECthreads baselevel %s, for Digital internal use only" (where
     * %s is the baselevel version string). If not defined properly, print an
     * error message and exit.
     */
    {
    char		match[512], translation[512];
    char		*trans;
    cma_t_boolean	okflag = cma_c_true;


    if ((int)cma__int_sprintf (
	    match,
	    "DECthreads baselevel %s, for Digital internal use only",
	    cma__c_version) == -1)
	cma__bugcheck ("Unable to format restriction string with sprintf");

    if (!cma__getenv ("CMA_RESTRICTED_DISTRIBUTION", translation, 512))
	okflag = cma_c_false;

    if (okflag) {
	if (cma__strncmp (match, translation, cma__strlen (match)) != 0)
	    okflag = cma_c_false;
	}

    if (!okflag) {
	cma__t_file		errfile;
	
	errfile = cma__int_fopen ("stderr", "w");
	cma__int_fprintf (
		errfile,
		"\tThis is experimental baselevel %s of the DECthreads\n",
		cma__c_version);
	cma__int_fprintf (
		errfile,
		"\tlibrary. This baselevel may be used only for experimental\n");
	cma__int_fprintf (
		errfile,
		"\tand pre-release testing within Digital Equipment Corporation,\n");
	cma__int_fprintf (
		errfile,
		"\tand only with specific permission from the DECthreads\n");
	cma__int_fprintf (
		errfile,
		"\tdevelopment group. To affirm that you accept these\n");
	cma__int_fprintf (
		errfile,
		"\tconditions and have received permission, define the\n");
#if _CMA_OS_ == _CMA__VMS
	cma__int_fprintf (
		errfile,
		"\tlogical name CMA_RESTRICTED_DISTRIBUTION to the value\n");
#else
	cma__int_fprintf (
		errfile,
		"\tenvironment variable CMA_RESTRICTED_DISTRIBUTION to the value\n");
#endif
	cma__int_fprintf (
		errfile,
		"\t\"%s\"\n",
		match);
	cma__int_fprintf (
		errfile,
		"\tbefore running any program utilizing this library.\n");

#if _CMA_OS_ == _CMA__VMS
	cma__abort ();
#elif _CMA_OS_ == _CMA__UNIX
	exit (-1);
#endif	
	}

    }
#endif
    }

#ifndef NDEBUG
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get the value of all CMA environment variables to customize caching
 *	characteristics (and other things).
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	cma__g_env array (names of variables)
 *
 *  IMPLICIT OUTPUTS:
 *
 *	cma__g_env array (value of variables)
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static void
cma___init_env
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer	i;
    char		*stringvalue, buffer[256];

    for (i = 0; i < cma__c_env_count; i++) {
	stringvalue = cma__getenv (cma__g_env[i].name, buffer, 256);

	if (stringvalue != cma_c_null_ptr) {

	    switch (cma__g_env[i].type) {
		case cma__c_env_type_int : {
		    cma__g_env[i].value = cma__atoi (stringvalue);
		    break;
		    }
		case cma__c_env_type_file : {
		    if (i == cma__c_env_trace)
			cma__init_trace (stringvalue);
		    break;
		    }

		}

	    }

	}

    }
#endif

/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_INIT.C */
/*  *64   26-JUL-1993 13:34:53 BUTENHOF "cma_init destroys thread ID!" */
/*  *63    2-JUL-1993 14:37:54 BUTENHOF "Don't init mgr_wake on OSF/1" */
/*  *62   27-MAY-1993 14:32:25 BUTENHOF "Change QUIPU interface" */
/*  *61   14-MAY-1993 15:55:45 BUTENHOF "Change 'last_thread' to 'dead_zone'" */
/*  *60    6-MAY-1993 19:07:01 BUTENHOF "Add permanent QUIPU init support" */
/*  *59    4-MAY-1993 11:38:28 BUTENHOF "Init last_thread in right place" */
/*  *58    3-MAY-1993 13:44:31 BUTENHOF "Init last_thread at start" */
/*  *57   22-APR-1993 10:01:18 KEANE "Fix NDEBUG problem" */
/*  *56   16-APR-1993 13:03:47 BUTENHOF "Allow lib$init override" */
/*  *55    9-MAR-1993 13:02:29 BUTENHOF "Fix sprintf" */
/*  *54    4-MAR-1993 09:27:21 BUTENHOF "Fix comment" */
/*  *53    2-MAR-1993 13:08:20 BUTENHOF "RESTRICTED_DISTRIBUTION: use exit(-1) on UNIX" */
/*  *52    1-MAR-1993 13:11:57 BUTENHOF "Check for enabling logical name" */
/*  *51   23-FEB-1993 16:32:45 SCALES "Fix two wake-up waiting races in the null thread" */
/*  *50   10-DEC-1992 14:34:18 BUTENHOF "Fix pointer type problem (cma_debug_cmd)" */
/*  *49    1-DEC-1992 14:05:22 BUTENHOF "OSF/1 scheduling" */
/*  *48   24-NOV-1992 01:52:57 SCALES "Initialize TIS earlier" */
/*  *47   20-NOV-1992 11:18:54 BUTENHOF "Upgrade some ints to long" */
/*  *46   15-OCT-1992 13:11:53 BUTENHOF "Call init_sched on alpha" */
/*  *45    6-OCT-1992 16:29:27 CURTIN "Update Cactus stacking" */
/*   36A2  6-OCT-1992 16:24:11 CURTIN "Update lib$ call usage" */
/*  *44    5-OCT-1992 15:07:46 BUTENHOF "Init defer flags straight-off" */
/*  *43    2-OCT-1992 16:12:17 BUTENHOF "Improve OSF/1 RT support" */
/*  *42   15-SEP-1992 13:49:43 BUTENHOF "Change sequencing" */
/*  *41    3-SEP-1992 21:28:29 SCALES "Add re/initialization calls for scheduler module" */
/*  *40   25-AUG-1992 11:48:21 BUTENHOF "Don't use cma__int_init for cma_init!" */
/*  *39   13-AUG-1992 14:44:01 BUTENHOF "Set Alpha OSF/1 unique value ASAP" */
/*  *38   31-JUL-1992 15:03:27 BUTENHOF "Init memory before env for trace file" */
/*  *37   20-JUL-1992 15:52:36 CURTIN "remove reference to malloc" */
/*   36A1 16-JUL-1992 15:43:48 CURTIN "Turn off cma__init_tis call" */
/*  *36   11-JUN-1992 06:55:16 CURTIN "Removed cma__initialize routine" */
/*  *35   22-MAY-1992 17:43:31 SCALES "Integrate TIS support" */
/*  *34   21-MAY-1992 13:46:34 CURTIN "" */
/*  *33   21-MAY-1992 09:38:40 CURTIN "Made include for libicb.h OpenVMS Alpha specific" */
/*  *32   20-MAY-1992 20:45:57 SCALES "Convert to stream format for ULTRIX build" */
/*  *31   20-MAY-1992 17:06:49 CURTIN "Added cma__find_persistent_frame for OpenVMS Alpha" */
/*  *30   24-APR-1992 17:04:11 SCALES "Unconditionalize signal module init" */
/*  *29   17-APR-1992 11:11:19 BUTENHOF "Add version number string" */
/*  *28   16-APR-1992 06:02:02 BUTENHOF "Add string template for version" */
/*  *27    2-APR-1992 21:30:18 BUTENHOF "Use crt0.o hooks on OSF/1" */
/*  *26   19-MAR-1992 13:17:25 BUTENHOF "Add debugging hooks for libc_r.a mutex code" */
/*  *25   13-MAR-1992 14:08:46 BUTENHOF "Fix trace sequence" */
/*  *24   18-FEB-1992 15:29:11 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *23   24-JAN-1992 10:02:16 BUTENHOF "Since I made it possible for Alpha open rtl, do static exception init" */
/*  *22   19-DEC-1991 13:08:28 BUTENHOF "Fix trace file init" */
/*  *21   18-DEC-1991 08:55:01 BUTENHOF "Fix a type problem" */
/*  *20   18-DEC-1991 06:45:16 BUTENHOF "Fix exception initialization" */
/*  *19   26-NOV-1991 16:15:27 BUTENHOF "Fix some conditionals" */
/*  *18   25-NOV-1991 14:00:06 BUTENHOF "Add cma_g_debug_cmd" */
/*  *17    5-NOV-1991 14:58:50 BUTENHOF "Integrate Dave Weisman's changes" */
/*  *16   16-OCT-1991 10:00:42 BUTENHOF "Add a typecast" */
/*  *15   14-OCT-1991 13:38:49 BUTENHOF "Refine/fix use of config symbols" */
/*  *14   24-SEP-1991 16:27:18 BUTENHOF "Get count of file descriptors early" */
/*  *13   17-SEP-1991 13:23:36 BUTENHOF "Get count of file descriptors early" */
/*  *12   10-JUN-1991 18:22:01 SCALES "Add sccs headers for Ultrix" */
/*  *11   10-JUN-1991 17:54:36 SCALES "Conditionalize inclusion of I/O stuff" */
/*  *10    6-JUN-1991 11:20:38 CURTIN "Added to init process, cma__init_atfork" */
/*  *9     7-MAY-1991 10:06:13 CURTIN "Reworked cma_init to use new cma__int_init macro" */
/*  *8     2-MAY-1991 13:58:22 BUTENHOF "Standardize trace messages" */
/*  *7    12-APR-1991 23:35:52 BUTENHOF "Mach support" */
/*  *6     1-APR-1991 18:08:53 BUTENHOF "Exception changes" */
/*  *5    21-MAR-1991 09:26:19 BUTENHOF "Fix up cma_debug access" */
/*  *4    24-JAN-1991 20:50:50 BUTENHOF "Fix status names in exception initializers" */
/*  *3    24-JAN-1991 11:40:27 CURTIN "removed include of cma_client.h" */
/*  *2    24-JAN-1991 00:34:54 BUTENHOF "Get rid of cma_client module" */
/*  *1    12-DEC-1990 21:46:23 BUTENHOF "CMA initializer" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_INIT.C */
