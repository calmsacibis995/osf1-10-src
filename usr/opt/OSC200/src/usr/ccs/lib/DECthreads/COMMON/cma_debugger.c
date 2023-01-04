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
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Debug functions
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	Halloween 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	1 November 1989
 *		Enhance debugging: make external entry a parser.
 *	002	Webb Scales	3 November 1989
 *		Remove semaphore.lock field references
 *	003 	Bob Conti	4 November 1989
 *		Remove superfluous include of cma_host
 *	004	Webb Scales	19 November 1989
 *		Changed thd->thread_context.SP to thd->static_ctx.sp
 *	005	Dave Butenhof	20 November 1989
 *		Add some commands and switches
 *	006	Dave Butenhof	22 November 1989
 *		Use new "owner" field of mutex to report current owner.
 *	007	Dave Butenhof	1 December 1989
 *		Add scheduling policy to thread output.
 *	008	Dave Butenhof	4 December 1989
 *		Fix unconditional include of (VMS-only) stdlib.h
 *	009	Dave Butenhof	5 December 1989
 *		Remove initial_test attribute.
 *	010	Dave Butenhof	11 December 1989
 *		Change wording of thread display (alert "disabled" to "asynch
 *		disabled").
 *	011	Dave Butenhof	9 February 1990
 *		Enhance thread report to include full stack information.
 *	012	Dave Butenhof	12 February 1990
 *		Fix pointer arithmetic compiler complaint introduced by 011.
 *	013	Dave Butenhof	15 February 1990
 *		Handle default stack as special case (no stack object, so
 *		don't try to look at it).
 *	014	Dave Butenhof	26 February 1990
 *		Add more information for null thread.
 *	015	Dave Butenhof	26 February 1990
 *		Enhance stack information, handle new policy symbols.
 *	016	Dave Butenhof	9 April 1990
 *		Use new known object structures, with locks.
 *	017	Dave Butenhof	14 June 1990
 *		Replace cma__test_and_set by cma__kernel_set, and cma__unset
 *		by cma__kernel_unset, to match redefinitions done to allow
 *		mutex optimizations.
 *	018	Dave Butenhof	18 June 1990
 *		Use macro to test value of name field (doesn't exist for
 *		NDEBUG build).
 *	019	Webb Scales	15 June 1990
 *		Added priority scheduling, rearranged part of the tcb.
 *	020	Dave Butenhof	27 June 1990
 *		Support friendly mutexes.
 *	021	Dave Butenhof	6 July 1990
 *		On machines without interlocked instructions, locking a mutex
 *		involves entering the kernel, but the debugger runs with the
 *		kernel locked to prevent timeslices and such.  Therefore, on
 *		such machines, listing a friendly mutex (which attempts to
 *		lock the mutex's internal mutex) hangs. "Fix" this by reading
 *		the nesting count without locking the internal mutex and
 *		hoping for the best (it's OK on uniprocessors).
 *	022	Webb Scales	26 July 1990
 *		Added policy strings for new scheduling policies.
 *	023	Dave Butenhof	03 August 1990
 *		Fix semaphore stuff
 *	024	Paul Curtin	06 August 1990
 *		Replaced gets, strlen, strncmp, and strtok with  cma__*
 *		versions.  Replace printf with cma__put* functions.
 *	025	Webb Scales	30 August 1990
 *		Removed the put_eol from the debug prompt, and added a prompt
 *		string to _gets.
 *	026	Webb Scales	4 September 1990
 *		Fixed policy to print as a string instead of an int.
 *	027	Dave Butenhof	25 October 1990
 *		Fix up print formatting.
 *	028	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	029	Dave Butenhof	7 February 1991
 *		Changes to alert state bits.
 *	030	Dave Butenhof	12 February 1991
 *		Support nonrecursive mutexes, and change from "friendly" to
 *		"recursive".
 *	031	Dave Butenhof	4 March 1991
 *		Add new command 'set', and new thread selection switches '-h'
 *		for hold, '-n' for "nohold", '-s<exp>' for priority
 *	032	Dave Butenhof	11 March 1991
 *		Add thread -f for brief, and display thread using
 *		cma__deb_show_thread function.
 *	033	Dave Butenhof	12 March 1991
 *		Fix MIPS CC compilation warnings.
 *	034	Webb Scales	14 March 1991
 *		Fix more MIPS CC compilation warnings.
 *	035	Dave Butenhof	18 March 1991
 *		Make several static functions extern so that Ada debug code
 *		can chain on ours. Also add a switch to "set" to act on all
 *		known threads (primarily for hold/nohold).
 *	036	Dave Butenhof	09 April 1991
 *		Don't reference "atomic bits" directly
 *	037	Dave Butenhof	16 May 1991
 *		Add new cma_debug_cmd entry to take command lines as args.
 *	038	Dave Butenhof	21 June 1991
 *		Add casts to cma__puthex calls.
 *	039	Dave Butenhof	01 July 1991
 *		Add mutex type to attribute object display.
 *	040	DECthreads team	    22 July 1991
 *		Changed calls to atol to cma__atol
 *	041	Dave Butenhof	18 September 1991
 *		Modify "thread -i -f" display to show the internal synch.
 *		objects. Also don't try to format "processor" structure (for
 *		"-v") unless it actually exists.
 *	042	Dave Butenhof	19 September 1991
 *		Integrate HPUX CMA5 reverse drop: some additional type casts
 *		make for a happy compiler.
 *	043	Dave Butenhof	04 October 1991
 *		Clean up use of _CMA_UNIPROCESSOR_
 *	044	Dave Butenhof	10 October 1991
 *		Add hooks to print out trace array info (if built) and VM
 *		stats. Change "set" command to "tset" (thread set) since it
 *		hasn't been documented yet, and I'd prefer to save "set" for
 *		more general setup (like to make nonrecursive mutexes the
 *		default, or turn on core-dump-on-synch-term-signal mode).
 *	045	Dave Butenhof	10 December 1991
 *		Fix bug in stack display on "thread -i": for current thread,
 *		it reported value in static_context structure rather than
 *		current live SP.
 *	046	Dave Butenhof	10 February 1992
 *		Add new global cma__g_sbrk_align to memory management report
 *		(number of bytes lost to align sbrk chunks).
 *	047	Dave Butenhof	05 March 1992
 *		Add "show -v" to call cma__vp_dump(). Enhance "thread -i -v"
 *		to show "real" vp state (cma__vp_get_state()).
 *	048	Dave Butenhof	16 June 1992
 *		Implement internal queue operations that don't test link
 *		validity with assertions (this could cause an infinite loop,
 *		since assert bugchecks, which calls debug functions to dump
 *		the current state).
 *	049	Dave Butenhof	24 July 1992
 *		Fix report of "thread not on stack" added in 048 -- it was
 *		backwards.
 *	050	Dave Butenhof	30 July 1992
 *		Change cma__puthex_8 to cma__puthex_long, and any remaining
 *		uses of "%08x" to ("%0*lx", cma__c_hexchars) to make field 8
 *		chars wide for 32 bit long int or 16 chars wide for 64 bit.
 *	051	Dave Butenhof	31 July 1992
 *		Add "handle" command to decode a user's DECthreads object
 *		handle.
 *	052	Dave Butenhof	25 August 1992
 *		Don't report VP stuff for a terminated thread -- it's stale.
 *	053	Dave Butenhof	26 August 1992
 *		Remove references to semaphores.
 *	054	Dave Butenhof	11 September 1992
 *		Add -f to condition variable display; show defer & pend bits.
 *	055	Dave Butenhof	14 September 1992
 *		Display count of deferred signals & broadcasts, plus pending
 *		wakes, for each CV (under !NDEBUG only). Also, clean up
 *		column counting with new cma__put* interfaces (which return
 *		formatted string size). Convert several sequences of older
 *		cma__put* calls into cma__putformat.
 *	056	Dave Butenhof	21 September 1992
 *		Webb changed the null & mgr threads from negative sequence
 *		numbers to large positive. So use the "long form" logic to
 *		detect them by checking thd->kind (slightly more expensive,
 *		but it won't care what the numbers become in the future).
 *	057	Dave Butenhof	24 September 1992
 *		Add name for new sched. policy, and support for 'owner' field
 *		in object header.
 *	058	Dave Butenhof	 7 October 1992
 *		Fix a typo in thread -i.
 *	059	Dave Butenhof	13 October 1992
 *		Remove calls to sched_ functions.
 *	060	Dave Butenhof	 5 November 1992
 *		Remove some cma__cv_if_waiters calls, which are susceptible
 *		to queue corruption; use cma___dbg_q_empty instead, which
 *		reports corruption gracefully.
 *	061	Dave Butenhof	11 November 1992
 *		Fix misplaced "}" that broke NDEBUG build.
 *	062	Dave Butenhof	20 November 1992
 *		Fix vp_interrupt call.
 *	063	Dave Butenhof	25 November 1992
 *		Modify to handle DEC OSF/1 RT policy better.
 *	064	Dave Butenhof	 3 December 1992
 *		OSF/1 policy/priority is skewed from uniprocessor values, so
 *		make the string arrays correct!
 *	065	Dave Butenhof	 7 December 1992
 *		cma_debug_cmd() doesn't work on OSF/1 dbx -- it doesn't
 *		handle the va list. So change it to ';'-separated command
 *		strings in single argument.
 *	066	Dave Butenhof	17 February 1993
 *		Fix a bug in thread -i output -- alert nesting scope is
 *		formatted wrong.
 *	067	Dave Butenhof	26 February 1993
 *		Suspend other kernel threads (DEC OSF/1) when entering
 *		cma_debug.
 *	068	Dave Butenhof	 3 March 1993
 *		Make 067 literal -- I stopped 'em on cma_debug_cmd() as well,
 *		and I think perhaps it shouldn't (at least have one way to
 *		look at threads undisturbed, and it should work OK anyway
 *		since it doesn't need to prompt).
 *	069	Dave Butenhof	10 March 1993
 *		Move VP & Mach thread state strings to VP code.
 *	070	Dave Butenhof	15 April 1993
 *		Use extern VM queue names
 *	071	Dave Butenhof	30 April 1993
 *		Add 'versions' command
 *	072	Dave Butenhof	30 April 1993
 *		Look for terminated threads on zombie list.
 *	073	Dave Butenhof	 3 May 1993
 *		Reverse 072 to preserve VMS DEBUG interface: delete zombie
 *		list and keep all threads on known list.
 *	074	Dave Butenhof	 4 May 1993
 *		It's nice to use cma_debug_cmd() to turn VPs on and off, but
 *		the "kernel is locked" warning can block and let other
 *		threads go. Don't give messages for cma_debug_cmd.
 *	075	Dave Butenhof	 5 May 1993
 *		Unconditionalize some of the "show -m" counters, since
 *		they're fairly cheap-to-keep and may be useful.
 *	076	Dave Butenhof	 7 May 1993
 *		Add -a to "show -s" to include Ada (or "all") policies.
 *	077	Dave Butenhof	10 May 1993
 *		Clean up some formatting (add 0x prefix to mutex address, and
 *		always display sequence number before address). Also, only
 *		show cached object count on attributes with -f.
 *	078	Dave Butenhof	13 May 1993
 *		Add zombie state
 *	079	Dave Butenhof	17 May 1993
 *		VAX ULTRIX can't handle comparisons other than "==" and "!="
 *		on enums, so cast new "<" and ">=" comparisons to (int)
 *		[due to addition of "zombie" state].
 *	080	Dave Butenhof	27 May 1993
 *		Enhancements tend to occur while I'm waiting for builds to
 *		finish during the process of fixing aggravating bugs. Thus,
 *		I'm improving the text introducing the object upon which a
 *		thread is currently blocked so that it's obvious whether the
 *		culprit is a condition or mutex.
 *	081	Dave Butenhof	28 May 1993
 *		More minor enhancements -- like reporting spindle state for
 *		VP cv & mutex (under -f).
 *	082	Dave Butenhof	26 July 1993
 *		Add a "follow queue" command to aid in debugging an OSF/1
 *		problem.
 *	083	Dave Butenhof	27 July 1993
 *		Fix minor formatting bug in squeue help.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_queue.h>
#include <cma_dispatch.h>
#include <cma_init.h>
#include <cma_tcb_defs.h>
#include <cma_condition.h>
#include <cma_mutex.h>
#include <cma_attr.h>
#include <cma_stack.h>
#include <cma_debugger.h>
#include <cma_deb_core.h>
#include <cma_util.h>
#include <cma_vp.h>
#include <cma_assem.h>
#include <cma_vm.h>
#include <cma_tis_sup.h>
#if _CMA_OS_ == _CMA__UNIX
# include <limits.h>
# include <sys/utsname.h>
#endif
#if _CMA_OS_ == _CMA__VMS
# include <starlet.h>			/* sys$getsyi */
# include <syidef.h>
#endif
#if _CMA_KTHREADS_ == _CMA__MACH
# include <mach.h>
# include <mach_error.h>
#endif

/*
 * LOCAL MACROS
 */

/*
 * Status values for internal queue operations
 */

#define cma___c_dbg_q_empty	0
#define cma___c_dbg_q_full	1
#define cma___c_dbg_q_corrupt	-1

/*
 * Flags for thread display bitmask
 */
#define DISPLAY_FULL		0x00000001
#define DISPLAY_VP		0x00000002

/*
 * Flags for thread select bitmask
 */
#define SELECT_ID		0x00000001
#define SELECT_INTERNAL		0x00000002
#define SELECT_BLOCKED		0x00000004
#define SELECT_TERMINATED	0x00000008
#define SELECT_CURRENT		0x00000010
#define SELECT_NULL		0x00000020
#define SELECT_READY		0x00000040
#define SELECT_DETACHED		0x00000080
#define SELECT_HOLD		0x00000100
#define SELECT_ME		0x00000200
#define SELECT_NOHOLD		0x00000400
#define SELECT_SCHED		0x00000800
#define SELECT_ZOMBIE		0x00001000

/*
 * Flags for queue tracing
 */

#define Q_IS_OBJ		0x00000001
#define Q_IS_THREAD		0x00000002

/*
 * Flags to control priorty/policy matching
 */
#define cma___c_cmp_eql 0
#define cma___c_cmp_neq 1
#define cma___c_cmp_gtr 2
#define cma___c_cmp_geq 3
#define cma___c_cmp_lss 4
#define cma___c_cmp_leq 5

#define cma___putname(out,obj) cma__putformat( \
    (out),(cma__obj_null_name(obj)?user_obj:((cma__t_object *)(obj))->name), \
    ((cma__t_object *)(obj))->owner)

#define cma___test_mutex(m) \
    (cma__tac_isset (&(m)->lock) \
	? ((m)->mutex_kind != cma_c_mutex_fast \
	    ? (m)->owner != (cma__t_int_tcb *)cma_c_null_ptr \
	    : cma_c_true) \
	: cma_c_false)

/*
 * GLOBAL DATA
 */

char	*cma__g_hardware =
#if _CMA_HARDWARE_ == _CMA__MIPS
	"MIPS Rxx00";
#elif _CMA_HARDWARE_ == _CMA__VAX
	"VAX";
#elif _CMA_HARDWARE_ == _CMA__M68K
	"M680x0";
#elif _CMA_HARDWARE_ == _CMA__HPPA
	"HPPA";
#elif _CMA_HARDWARE_ == _CMA__RIOS
	"RS/6000";
#elif _CMA_HARDWARE_ == _CMA__ALPHA
	"AXP";
#elif _CMA_HARDWARE_ == _CMA__SPARC
	"Sparc";
#else
	"<hdwr?>";
#endif

char	*cma__g_os =
#if _CMA_OSIMPL_ == _CMA__OS_OSF
# if _CMA_VENDOR_ == _CMA__DIGITAL
	"DEC OSF/1";
# else
	"OSF/1";
# endif
#elif _CMA_OSIMPL_ == _CMA__OS_BSD
# if _CMA_VENDOR_ == _CMA__DIGITAL
	"ULTRIX";
# elif _CMA_VENDOR_ == _CMA__APOLLO
	"DOMAIN";
# elif _CMA_VENDOR_ == _CMA__HP
	"HPUX";
# elif _CMA_VENDOR_ == _CMA__SUN
	"SunOS";
# else
	"BSD UNIX";
# endif
#elif _CMA_OSIMPL_ == _CMA__OS_SYSV
# if _CMA_VENDOR_ == _CMA__SUN
	"Solaris";
# else
	"UNIX";
# endif
#elif _CMA_OSIMPL_ == _CMA__OS_VMS
	"OpenVMS";
#elif _CMA_OSIMPL_ == _CMA__OS_AIX
	"AIX";
#else
	"<os?>";
#endif

/*
 * LOCAL DATA
 */

static long int		active_regs[17] = {0};

static cma__t_string	user_obj = "<USER>";

static cma__t_string	enabled_string = "enabled";
static cma__t_string	disabled_string = "disabled";

typedef enum CMA___T_CMD_TYPE {
    cma___c_cmd_normal,
    cma___c_cmd_modifier,
    cma___c_cmd_hidden,
    cma___c_cmd_exit
    } cma___t_cmd_type;

typedef struct CMA___T_CMD_INFO {
    char		*verb;
    cma___t_cmd_type	type;
    void		(*routine) ();
    char		*brief_desc;
    char		**full_desc;
    } cma___t_cmd_info;

/*
 * Caution: The layout of this must match cma_t_state in cma_tcb_defs.h
 */
static char 	*state[cma__c_state__dim] = {
    "running",
    "ready",
    "blocked",
    "terminated",
    "zombie"
    };

static char	*status[3] = {
    "error",
    "normal",
    "alert"
    };

static char	*policy[cma__c_max_policies] = {
#if _CMA_RT4_KTHREAD_
    "<undefined>",
#endif
    "fifo",
    "rr",
    "throughput",
    "background",
    "ada_low",
    "ada_rtb",
    "idle"
    };

#define cma___c_num_pris 3
static char	*pri_names[cma___c_num_pris] = {
    "min",
    "mid",
    "max"
    };

static char	*mutex_type[3] = {
    "fast",
    "recursive",
    "nonrecursive"
    };

static char	*obj_type[cma__c_obj_num] = {
    "<unused>",
    "attributes object",
    "condition variable",
    "mutex",
    "thread",
    "stack"
    };

static char	*debug_attr_help[] = {
    " attributes [id] -f",
    "   Print information about attributes objects.",
    "	id	Attributes object number",
    "	-f	Show additional information (cached objects)",
    cma_c_null_ptr};

static char	*debug_cond_help[] = {
    " condition [id] -fwq",
    "   Print information about condition variable objects.",
    "	id	Condition variable object number",
    "	-f	Show 'defer' and 'pending' states:",
    "		[sbp] := deferred signal, deferred broadcast, pending wake",
    "	-w	List only condition variables with waiting threads",
    "	-q	Show the queue of waiting threads (if any)",
    cma_c_null_ptr};

static char	*debug_hand_help[] = {
    " handle <address>",
    "   Format address (<decimal>, 0<octal>, or 0x<hex>) as DECthreads handle",
    cma_c_null_ptr};

static char	*debug_help_help[] = {
    " help [<topic>]",
    "   With no arguments, print a summary of all commands.",
    "   If you specify an argument, the help command will print detailed",
    "   descriptions of each command with an initial string matching the",
    "   argument. For example, 'help t' will describe the 'thread' and",
    "   'tset' commands. 'help *' will give help on all commands.",
    cma_c_null_ptr};

static char	*debug_exit_help[] = {
    " exit, quit",
    "   Exit from DECthreads debugger.",
    cma_c_null_ptr};

static char	*debug_mutex_help[] = {
    " mutex [id] -lq",
    "   Print information about mutex objects.",
    "	id	Mutex object number",
    "	-l	List only mutexes which are locked",
    "	-q	Show the queue of waiting threads (if any)",
    cma_c_null_ptr};

static char	*debug_queue_help[] = {
    " squeue <address>",
    "   Treat address (<decimal>, 0<octal>, or 0x<hex>) as an element in a",
    "   double-linked circular queue with absolute addresses",
    "   (void *flink, void *blink) and follow the entire queue.",
    "	-h	Assume address is queue header: don't format",
    "	-o	Assume (non-thread) DECthreads queue, format type and sequence",
    "	-t	Assume known thread queue, format header",
    cma_c_null_ptr};

static char	*debug_show_help[] = {
#ifdef _CMA_TRACE_KERNEL_
    " show -ckmpsv",
#else
    " show -cmpsv",
#endif
    "   Print information about DECthreads.",
    "	-c	Show current thread.",
#ifdef _CMA_TRACE_KERNEL_
    "	-k	Show trace log of kernel lock operations.",
#endif
    "	-m	Show internal memory management statistics.",
    "	-p	Show internal memory pool.",
    "	-s	Show scheduling ranges.",
    "	-v	Show virtual processors.",
    cma_c_null_ptr};

static char	*debug_stack_help[] = {
    " stack [sp] -af",
    "   Print information about stacks.",
    "	sp	Stack pointer value (decimal, 0octal, 0xhex)",
    "	-a	Include free stack segments",
    "	-f	Display base and guard range of in-use stacks",
    cma_c_null_ptr};

static char	*debug_thread_help[] = {
    " thread [id] -frbtcdahn [-s <sched>]",
    "   Print information about thread objects.",
    "	id	Thread object number",
    "	-f	Display full thread information",
    "	-i	Display using alternate format",
    "	-r	Select only threads that are ready to run",
    "	-b	Select only threads that are blocked",
    "	-t	Select only terminated threads",
    "	-c	Select only currently running threads",
    "	-d	Select only detached threads",
    "	-a	Include \"invisible\" threads owned by DECthreads",
    "	-h	Select only threads which are on hold",
    "	-m	Select only calling thread",
    "	-n	Select only threads which are not on hold",
    "	-s	Select by scheduling information. The syntax of the",
    "		value is \"policy[<op>priority]\". \"policy\" is the name",
    "		of a DECthreads scheduling policy. <op> is one of the C",
    "		relational operators ==, !=, >, <, >=, or <=. \"priority\"",
    "		may be an integer, or one of the keywords \"min\", \"mid\",",
    "		or \"max\" designating the minimum, midrange, or maximum",
    "		priority of the specified scheduling policy. Threads are",
    "		selected if they have the specified policy and a priority",
    "		which is equal to (==), greater than (>), and so forth, with",
    "		respect to the specified priority",
    "	-z	Include \"zombie\" threads",
    cma_c_null_ptr};

static char	*debug_tset_help[] = {
    " tset id -chnav [-s <sched>]",
    "   Set thread object state. If \"id\" is specified as \"*\", then all",
    "   threads will be modified; only -h, -n, and -s may be used with \"*\".",
    "	id	Mutex object number or \"*\"",
    "	-c	Cancel thread (at next cancellation point)",
    "	-h	Put the thread on hold; the thread cannot run until set",
    "		to -n, \"nohold\"",
    "	-n	Release a thread that was put on hold",
    "	-a	Make the thread active: it will be the next thread to run",
    "	-v	Make the thread visible so that its state may be examined",
    "	-s	Set scheduling information, \"[policy]:priority\". The",
    "		\"policy\" may be any of the DECthreads scheduling policies,",
    "		fifo, rr, throughput, background, ada_low, or idle. The",
    "		\"priority\" may be an integer, or one of the keywords",
    "		\"min\", \"mid\", or \"max\" designating the minimum,",
    "		midrange, or maximum priority for the specified policy",
    cma_c_null_ptr};

static char	*debug_vers_help[] = {
    " versions",
    "   Display the version of the DECthreads library and system",
    cma_c_null_ptr};

#if !_CMA_UNIPROCESSOR_
static char	*debug_vp_help[] = {
    " vp -rs",
    "   Suspend or resume virtual processors that are in use by other",
    "   threads within the process.",
    "	-r	Resume all VPs that have been held",
    "	-s	Suspend all VPs except the current one",
    cma_c_null_ptr};
#endif

static char	*debug_help_short[1] = {"help"};

#if !_CMA_UNIPROCESSOR_
# define cma___c_command_count	14
#else
# define cma___c_command_count	13
#endif
#define cma___c_max_args	10
static cma___t_cmd_info	cmds[cma___c_command_count] = {
    {"attributes", cma___c_cmd_normal, cma__debug_list_atts,
	"[id] -f : list attributes objects", debug_attr_help},
    {"condition", cma___c_cmd_normal, cma__debug_list_cvs,
	"[id] -fwq : list condition variables", debug_cond_help},
    {"handle", cma___c_cmd_normal, cma__debug_format_handle,
	"address : format as DECthreads object handle", debug_hand_help},
    {"help [topic]", cma___c_cmd_normal, cma__debug_help,
	": display help information", debug_help_help},
    {"exit", cma___c_cmd_exit, (void (*)())0,
	": exit from DECthreads debugger", debug_exit_help},
    {"mutex", cma___c_cmd_normal, cma__debug_list_mutexes,
	"[id] -lq : list mutexes", debug_mutex_help},
    {"quit", cma___c_cmd_exit, (void (*)())0,
	": exit from DECthreads debugger", debug_exit_help},
    {"show", cma___c_cmd_normal, cma__debug_show,
	"-cm{k}psv : show stuff", debug_show_help},
    {"squeue", cma___c_cmd_normal, cma__debug_queue,
	"address -hot: format queue", debug_queue_help},
    {"stack", cma___c_cmd_normal, cma__debug_list_stacks,
	"[sp] -af : list stacks", debug_stack_help},
    {"thread", cma___c_cmd_normal, cma__debug_list_threads,
	"[id] -firbtcdahmniz -s<sched> : list threads", debug_thread_help},
    {"tset", cma___c_cmd_modifier, cma__debug_set_thread,
	"{id | *} -s<sched> -chnav : set state of thread", debug_tset_help},
    {"versions", cma___c_cmd_normal, cma__debug_versions,
	": display versions", debug_vers_help}
#if !_CMA_UNIPROCESSOR_
    ,{"vp", cma___c_cmd_normal, cma__debug_vp,
	"-sr : suspend or resume all other VPs", debug_vp_help}
#endif
    };

/*
 * LOCAL FUNCTIONS
 */

static int
cma___dbg_q_empty _CMA_PROTOTYPE_ ((cma__t_queue *qh));

static int
cma___dbg_q_next _CMA_PROTOTYPE_ ((cma__t_queue *pq, cma__t_queue **nq));

static void
cma___dbg_travthd _CMA_PROTOTYPE_ ((
	cma__t_queue		*qhead,
	cma_t_integer		select_flags,
	cma_t_integer		display_flags,
	cma_t_integer		select_id,
	cma_t_integer		sched_comparison,
	cma_t_sched_policy	sched_policy,
	cma_t_priority		sched_priority));

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Debugging routine: parse commands and dispatch to execution code.
 *
 *  FORMAL PARAMETERS:
 *
 *	cmd	Command line string (may be a list, ending with 0).
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
 *	Many and varied.
 */
extern void
cma_debug_cmd
#ifdef _CMA_PROTO_
	(
	char	*cmd)
#else	/* no prototypes */
	(cmd)
	char	*cmd;
#endif	/* prototype */
    {
    cma_t_boolean	lock, semi, tis_state;
    char		*cmdline, *sc;


    cmdline = cmd;
    tis_state = cma__g_tis_disable;
    cma__g_tis_disable = cma_c_true;
    lock = cma__debug_trylock (cma_c_false);

    while (*cmdline != '\0') {

	for (sc = cmdline; *sc != '\0' && *sc != ';'; sc++);

	if (*sc == ';') {
	    *sc = '\0';			/* If we found ';', make it null */
	    semi = cma_c_true;
	    }
	else
	    semi = cma_c_false;

	if (cma__strlen (cmdline) > 0) {

	    if (cma__debug_parse (cmdline, lock)) break;

	    }

	if (semi)
	    cmdline = &sc[1];		/* Next command starts after null */
	else
	    break;

	}

    if (!lock)
	cma__unset_kernel ();

    cma__g_tis_disable = tis_state;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Main debugging routine: parse commands and dispatch to execution code.
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
 *	Many and varied.
 */
extern void
cma_debug
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    char			buffer[512];
    char			*ptr;
    cma_t_boolean		locked, tis_state;


    tis_state = cma__g_tis_disable;
    cma__g_tis_disable = cma_c_true;
#if _CMA_KTHREADS_ == _CMA__MACH
    cma__vp_suspend_others ();		/* Stop other kernel threads */
#endif

    /*
     * Lock the kernel, to prevent scheduling database changes while in
     * the debug loop.
     */
    locked = cma__debug_trylock (cma_c_true);

    while (1) {
	ptr = (cma_t_address) cma__gets ("DECthreads debug> ", &buffer[0]);

	if (ptr == cma_c_null_ptr) break;

	if (cma__strlen (ptr) == 0) continue;

	if (cma__debug_parse (ptr, locked)) break;

	}

    /*
     * When done, unblock kernel threads, unlock the kernel (if we actually
     * locked it), and return.
     */

#if _CMA_KTHREADS_ == _CMA__MACH
    cma__vp_resume_others ();		/* Resume stopped kernel threads */
#endif

    if (!locked)
	cma__unset_kernel ();

    cma__g_tis_disable = tis_state;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Format a DECthreads handle
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	Number of arguments
 *
 *	argv	array of argument strings
 *
 *  IMPLICIT INPUTS:
 *
 *	arrays to decode enum types.
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
cma__debug_format_handle
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    char		output[cma__c_buffer_size];
    cma__t_int_handle	*handle;
    cma__t_object	*object;
    cma_t_integer	sequence, type;
    cma_t_boolean	error = cma_c_false;


    output[0] = '\0';
    if (argc == 1) {
	cma__putstring (output, "Command 'handle' requires address argument");
	cma__puteol (output);
	return;
	}

    handle = (cma__t_int_handle *)cma__strtoul (
	    argv[1],
	    (char **)cma_c_null_ptr,
	    0);

    TRY {
	object = (cma__t_object *)handle->pointer;
	sequence = handle->sequence;
	type = handle->type;
	}
    CATCH_ALL {
	cma__putformat (output, "0x%lx is not a valid address", handle);
	cma__puteol (output);
	error = cma_c_true;
	}
    ENDTRY

    if (error) return;

    if (object == (cma__t_object *)cma_c_null_ptr
	    && type == 0 && sequence == 0) {
	cma__putformat (output, "0x%lx is a null handle", handle);
	cma__puteol (output);
	return;
	}

    TRY {
	if (object->type != type || object->sequence != sequence)
	    error = cma_c_true;
	}
    CATCH_ALL {
	error = cma_c_true;
	}
    ENDTRY

    if (error) {
	cma__putformat (output, "0x%lx is not a valid handle", handle);
	cma__puteol (output);
	return;
	}

    cma__putformat (
	    output,
	    "0x%lx is %s #%d",
	    handle,
	    obj_type[type],
	    sequence);

    if (object->name) {
	cma__putstring (output, "; ");
	cma___putname (output, object);
	}

    cma__puteol (output);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Format and display an object
 *
 *  FORMAL PARAMETERS:
 *
 *	qel		Address of object
 *	flags		Bitmask of flags to control display
 *
 *  IMPLICIT INPUTS:
 *
 *	arrays to decode enum types.
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
cma__debug_format_object
#ifdef _CMA_PROTO_
	(
	cma__t_queue	*qel,
	cma_t_integer	flags)
#else	/* no prototypes */
	(qel, flags)
	cma__t_queue	*qel;
	cma_t_integer	flags;
#endif	/* prototype */
    {
    char		output[cma__c_buffer_size];

    TRY {
	if (flags & (Q_IS_OBJ | Q_IS_THREAD)) {
	    cma__t_object	*obj;
	    cma_t_integer	type;
	    cma_t_integer	sequence;


	    if (flags & Q_IS_THREAD)
		obj = (cma__t_object *)cma__base (qel, threads, cma__t_int_tcb);
	    else
		obj = (cma__t_object *)qel;

	    type = obj->type;

	    if (type <= 0 || type >= cma__c_obj_num) {
		cma__putformat (
			output,
			"%lx:[%lx,%lx] object type %d; maybe the queue head?",
			qel, qel->flink, qel->blink,
			type);
		}
	    else {
		sequence = obj->sequence;

		cma__putformat (
			output,
			"%lx:[%lx,%lx] %s %d",
			qel, qel->flink, qel->blink,
			obj_type[type],
			sequence);
		}

	    }
	else
	    cma__putformat (
		    output,
		    "%lx:[%lx,%lx]",
		    qel, qel->flink, qel->blink);

	cma__puteol (output);
	}
    CATCH_ALL {
	cma__putformat (
		output,
		"%lx: bad address",
		qel);
	cma__puteol (output);
	}
    ENDTRY
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Format and display a TCB
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb		Address of TCB
 *	display_flags	Bitmask of flags to control display
 *
 *  IMPLICIT INPUTS:
 *
 *	arrays to decode enum types.
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
cma__debug_format_thread
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*thd,
	cma_t_integer	display_flags)
#else	/* no prototypes */
	(thd, display_flags)
	cma__t_int_tcb	*thd;
	cma_t_integer	display_flags;
#endif	/* prototype */
    {
    char		output[cma__c_buffer_size];
    int			qstat;
    cma__t_int_stack	*stack;
    cma__t_queue	*sq;
    cma__t_int_tcb	*self;
    long		sp, limit;
    cma_t_boolean	on_sp = cma_c_false;


    output[0] = '\0';
    cma__putformat (
	    output,
	    "%sThread ",
	    (cma__g_dead_zone == &thd->threads ? "(*)" : ""));
    cma___putname (output, thd);
    cma__putformat (
	    output,
	    " %d (0x%lx) : %s",
	    thd->header.sequence,
	    (cma_t_integer)thd,
	    state[(int)thd->state]);
    cma__puteol (output);

    if ((display_flags & (DISPLAY_VP | DISPLAY_FULL)) != 0) {

	if (thd->sched.processor == (cma__t_vp *)cma_c_null_ptr) {
	    cma__putstring (output, "  No current vp");
	    cma__puteol (output);
	    }
	else {
#if !_CMA_UNIPROCESSOR_
	    cma__t_vp_state	state;
	    cma__t_vp_status	st;


	    if (thd->sched.processor->vp_id == (cma__t_vpid)cma_c_null_ptr) {
		cma__putstring (output, "  vp has been deallocated");
		cma__puteol (output);
		}
	    else {
		cma__putstring (output, "  Current vp is ");
		cma__putint (
			output,
			(cma_t_integer)thd->sched.processor->vp_id->vp);
		cma__putstring (output, ", port is ");
		cma__putint (
			output,
			(cma_t_integer)thd->sched.processor->vp_id->synch);
		cma__puteol (output);

		if ((st = cma__vp_get_state (
			    thd->sched.processor->vp_id,
			    &state)) == cma__c_vp_normal) {
		    cma__putstring (output, "   VP state is ");
		    cma__putstring (
			output,
			cma__g_vp_statestr[state.run_state]);

		    if (state.suspend_count != 0)
			cma__putformat (
				output,
				"; Suspended[%d], ",
				state.suspend_count);

		    if (thd->sched.processor->vp_id->flags & cma__c_vp_hold)
			cma__putstring (output, " (hold)");

		    cma__puteol (output);
# if _CMA_KTHREADS_ == _CMA__MACH
		    cma__putformat (
			    output,
			    "    Mach policy %s, priority %d",
			    policy[(int)state.policy],
			    state.priority);
		    cma__puteol (output);
		    cma__putstring (output, "    Mach state: ");
		    cma__putstring (
			output,
			cma__g_mach_statestr[state.mach_state]);
		    cma__putstring (output, ".");
		    cma__puteol (output);
# endif
		    }
		else {
# if _CMA_KTHREADS_ == _CMA__MACH
		    cma__putstring (output, "   (Can't get VP state: ");
		    cma__putstring (
			output,
			mach_error_string ((kern_return_t)st));
		    cma__putstring (output, ")");
# else
		    cma__putstring (output, "   (Can't get VP state)");
# endif
		    cma__puteol (output);
		    }

		}
#else
	    cma__putformat (
		    output,
		    "  Current vp is 0x%lx",
		    thd->sched.processor->vp_id);
	    cma__puteol (output);
#endif
	    }

	}

    if ((display_flags & DISPLAY_FULL) != 0) {
	cma__putformat (
		output,
		"  Join: mutex %d (0x%lx), cv %d (0x%lx)",
		((cma__t_int_mutex *)thd->mutex)->header.sequence,
		(long)thd->mutex,
		((cma__t_int_cv *)thd->term_cv)->header.sequence,
		(long)thd->term_cv);
	cma__puteol	(output);
	cma__putformat (
		output,
		"  Sync. wait: mutex %d (0x%lx), cv %d (0x%lx)",
		((cma__t_int_mutex *)thd->tswait_mutex)->header.sequence,
		(long)thd->tswait_mutex,
		((cma__t_int_cv *)thd->tswait_cv)->header.sequence,
		(long)thd->tswait_cv);
	cma__puteol	(output);
	cma__putstring	(output, "  Thread's start function ");

	if ((cma_t_address)thd->start_code == cma_c_null_ptr)
	    cma__putstring (output, "is unknown");
	else {
	    cma__putformat (output,
		    "0x%lx (0x%lx)",
		    thd->start_code,
		    thd->start_arg);
	    }

	cma__puteol	(output);
	cma__putstring	(output, "  Thread's last errno was ");
	cma__putint	(output, thd->thd_errno);
	cma__puteol	(output);
	}

    if ((cma_t_integer)thd->state >= (cma_t_integer)cma__c_state_terminated) {
	cma__putformat (
		output,
		"  Exit status %s, result 0x%lx",
		status[(int)thd->exit_status],
		(long)thd->return_value);
	cma__puteol	    (output);
	cma__putformat (
		output,
		"  %d joiner%s",
		thd->joiners,
		(thd->joiners == 1 ? "." : "s."));
	cma__puteol	    (output);
	}

    if (thd->state == cma__c_state_blocked) {

	if (thd->wait_cv != (cma__t_int_cv *)cma_c_null_ptr) {
	    cma__putformat (
		    output,
		    "  Waiting on condition variable %d (0x%lx)",
		    ((cma__t_int_cv *)thd->wait_cv)->header.sequence,
		    (long)thd->wait_cv);
	    cma__puteol     (output);
	    }
	else if (thd->wait_mutex != (cma__t_int_mutex *)cma_c_null_ptr ) {
	    cma__putformat (
		    output,
		    "  Waiting to lock mutex %d (0x%lx)",
		    ((cma__t_int_mutex *)thd->wait_mutex)->header.sequence,
		    (long)thd->wait_mutex);
	    cma__puteol     (output);
	    }
	else {
	    cma__putstring	(output, "  Blocked on indeterminate object");
	    cma__puteol	(output);
	    }

	}

    self = cma__get_self_tcb ();
    cma__putstring  (output, "  Scheduling: priority ");
    cma__putint	    (output, thd->sched.priority);
    cma__putstring  (output, ", policy ");
    cma__putstring  (output, policy[(int)thd->sched.policy]);
    cma__puteol     (output);

    if (self == thd)
	sp = (long)cma__fetch_sp ();
    else
	sp = (long)thd->static_ctx.sp;

    if (cma___dbg_q_empty (&thd->stack) == cma___c_dbg_q_empty) {
	cma__putformat (output, "  Stack: 0x%lx (default stack)", sp);
	cma__puteol (output);

	if (sp >= cma__c_def_stack_min && sp <= cma__c_def_stack_max)
	    on_sp = cma_c_true;

	}
    else {
	int		select;
	static char	*inrange[2] = {") [<-SP]", ")"};


	qstat = cma___dbg_q_next (&thd->stack, &sq);

	if (qstat == cma___c_dbg_q_corrupt) {
	    cma__putformat (
		output,
		"  <<CORRUPT STACK QUEUE HEAD AT %lx: (%lx,%lx)>>",
		&thd->stack,
		thd->stack.flink,
		thd->stack.blink);
	    cma__puteol (output);
	    return;
	    }

	if (qstat == cma___c_dbg_q_empty) {
	    cma__putformat (
		output,
		"  <<UNEXPECTEDLY EMPTY STACK QUEUE HEAD AT %lx>>",
		&thd->stack);
	    cma__puteol (output);
	    return;
	    }

	stack = (cma__t_int_stack *)sq;

#ifdef _CMA_UPSTACK_
	limit = (long)stack->yellow_zone + cma__c_yellow_size;

	if ((sp >= (long)stack->stack_base) && (sp <= limit)) {
	    on_sp = cma_c_true;
	    select = 0;
	    }
	else
	    select = 1;

#else
	limit = (long)stack->yellow_zone - cma__c_yellow_size;

	if ((sp <= (long)stack->stack_base) && (sp >= limit)) {
	    on_sp = cma_c_true;
	    select = 0;
	    }
	else
	    select = 1;

#endif
	cma__putformat (
		output,
		"  Stack: 0x%lx (base = 0x%lx, guard = 0x%lx%s",
		sp,
		stack->stack_base,
		limit,
		inrange[select]);
	cma__puteol	(output);

	qstat = cma___dbg_q_next (sq, &sq);

	if (qstat != cma___c_dbg_q_full) {
	    cma__putformat (
		output,
		"  <<CORRUPT STACK QUEUE AT %lx: (%lx,%lx)>>",
		sq,
		sq->flink,
		sq->blink);
	    cma__puteol (output);
	    return;
	    }

	if (sq != &thd->stack) {
	    cma__putstring  (output, "  Additional stack segments: ");
	    cma__puteol	    (output);

	    while (sq != &thd->stack) {
		stack = (cma__t_int_stack *)sq;
		qstat = cma___dbg_q_next (sq, &sq);
		on_sp = cma_c_false;

		if (qstat != cma___c_dbg_q_full) {
		    cma__putformat (
			output,
			"  <<CORRUPT STACK QUEUE AT %lx: (%lx,%lx)>>",
			sq,
			sq->flink,
			sq->blink);
		    cma__puteol (output);
		    break;		/* Go on to next thread */
		    }

#ifdef _CMA_UPSTACK_
		limit = (long)stack->yellow_zone + cma__c_yellow_size;

		if ((sp >= (long) stack->stack_base) && (sp <= limit)) {
		    on_sp = cma_c_true;
		    select = 0;
		    }
		else
		    select = 1;
#else
		limit = (long)stack->yellow_zone - cma__c_yellow_size;

		if ((sp <= (long)stack->stack_base) && (sp >= limit)) {
		    on_sp = cma_c_true;
		    select = 0;
		    }
		else
		    select = 1;
#endif
		cma__putformat (
			output,
			"   base = 0x%lx, guard = 0x%lx%s",
			stack->stack_base,
			limit,
			inrange[select]);
		cma__puteol (output);
		}

	    }

	}

    if (! on_sp) {
	cma__putstring (output, " !! Thread is not on stack !!");
	cma__puteol (output);
	}

    if (thd->detached) {
	cma__putstring	(output, "  Detached");
	cma__puteol	(output);
	}

    cma__putstring  (output, "  Alerts: ");

    if (thd->alert_pending)		/* Note: state is NEGATIVE */
	cma__putstring (output, "none ");

    cma__putstring (output, "pending, ");
    cma__putstring (output, "general delivery ");

    if (thd->alert.g_enable)
	cma__putstring (output, enabled_string);
    else
	cma__putstring (output, disabled_string);

    cma__putstring (output, ", asynch delivery ");

    if (thd->alert.a_enable)
	cma__putstring (output, enabled_string);
    else
	cma__putstring (output, disabled_string);

    if (thd->alert.count > 0) {
	cma__putstring (output, ",");
	cma__puteol (output);

	cma__putformat (
		output,
		"   %d nested alert scope%s active",
		thd->alert.count,
		(thd->alert.count == 1 ? "" : "s"));
	}

    cma__puteol (output);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get the system type and version
 *
 *  FORMAL PARAMETERS:
 *
 *	buffer		address of buffer to place result
 *
 *	length		length of buffer
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
cma__debug_get_system
#ifdef _CMA_PROTO_
	(
	char	*buffer,
	int	length)
#else	/* no prototypes */
	(buffer, length)
	char	*buffer;
	int	length;
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__UNIX
    struct utsname	name;

    if (uname (&name) == 0) {
	cma__int_sprintf (
		buffer,
		"%s %s %s(%s)",
		name.sysname,
		name.machine,
		name.release,
		name.version);
	}
    else {
	cma__int_sprintf (buffer, "<unknown>");
	}
#elif _CMA_OS_ == _CMA__VMS
    char		model[32];
    char		version[16];
    unsigned short	mlen, vlen;
    struct {
	unsigned short	buffer_length;
	unsigned short	item_code;
	char		*buffer_address;
	unsigned short	*return_length_address;
	} itmlst[3];
    double  iosb;


    itmlst[0].buffer_length = 31;
    itmlst[0].item_code = SYI$_HW_NAME;
    itmlst[0].buffer_address = model;
    itmlst[0].return_length_address = &mlen;
    itmlst[1].buffer_length = 8;
    itmlst[1].item_code = SYI$_VERSION;
    itmlst[1].buffer_address = version;
    itmlst[1].return_length_address = &vlen;
    itmlst[2].buffer_length = 0;
    itmlst[2].item_code = 0;
    itmlst[2].buffer_address = 0;
    itmlst[2].return_length_address = 0;
    sys$getsyiw ( 0, 0, 0, &itmlst, &iosb, 0, 0 );
    model[mlen] = '\0';
    for (vlen = 0; version[vlen] != ' ' && vlen < 8; vlen++);
    version[vlen] = '\0';
    cma__int_sprintf (buffer, "OpenVMS %s (%s)\n", version, model);
#else
    buffer[0] = '\0';
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Print help message
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
 *	Print information on each command
 */
extern void
cma__debug_help
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    char		output[cma__c_buffer_size];
    cma_t_integer	i;
    cma_t_boolean	helped = cma_c_false, asterisk;
    char		*keyword, *kwptr, **hline;
    int			compare_len;


    output[0] = '\0';
    compare_len = 0;

    if (argc > 1) {
	kwptr = argv[1];
	keyword = kwptr;
	for (; *kwptr != '\0' && *kwptr != '*'; kwptr++, compare_len++);
	asterisk = (*kwptr == '*');
	}
    else
	asterisk = cma_c_false;

    if (compare_len == 0 && !asterisk) {

	for (i = 0; i < cma___c_command_count; i++)
	    if (cmds[i].type != cma___c_cmd_hidden) {
		cma__putstring(output, "    ");
		cma__putstring(output, cmds[i].verb);
		cma__putstring(output, " ");
		cma__putstring(output, cmds[i].brief_desc);
		cma__puteol (output);
		}

	cma__putstring (
		output,
		" All keywords may be abbreviated: ");
	cma__putstring (
		output,
		"if the abbreviation is ambiguous, the first");
	cma__puteol (output);
	cma__putstring (
		output,
		" match will be used. ");
	cma__putstring (
		output,
		"Exit from the debug function by entering");
	cma__putstring (
		output,
#if _CMA_OS_ == _CMA__VMS
		" ^Z, or a");
#else
		" ^D, or a");
#endif
	cma__putstring (
		output,
		" blank line. For more help, type 'help <topic>'");
	cma__putstring (output, ", or 'help *' for everything");
	cma__puteol (output);
	}
    else {
	/*
	 * Give detailed help on matching topics
	 */

	for (i = 0; i < cma___c_command_count; i++)
	    if (cma__strncmp (argv[1], cmds[i].verb, compare_len) == 0)
		for (hline = cmds[i].full_desc; *hline != (char *)0; hline++) {
		    cma__putstring (output, *hline);
		    cma__puteol (output);
		    helped = cma_c_true;
		    }

	if (!helped) {
	    cma__putformat (
		    output,
		    "Sorry, no topics starting with \"%s\"",
		    argv[1]);
	    cma__puteol (output);
	    }

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Print the status of all attributes objects.
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	argument count
 *
 *	argv	argument array
 *
 *  IMPLICIT INPUTS:
 *
 *	known attributes object queue.
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
 *	Print information on each known attributes object.
 */
extern void
cma__debug_list_atts
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma__t_queue	*ptr;
    cma_t_integer	select_id = 0;
    cma_t_boolean	show_full = cma_c_false;
    char		output[cma__c_buffer_size];
    int			arg, i;
    int			qstat;


    output[0] = '\0';

    for (arg = 1; arg < argc; arg++) {

	if (argv[arg][0] == '-')	/* If a switch */
	    for (i = 1; argv[arg][i] != 0; i++) {
		switch (argv[arg][i]) {
		    case 'f' : {
			show_full = cma_c_true;
			break;
			}
		    }
		}
	else
	    select_id = cma__atol (argv[arg]);

	}

    if (cma___test_mutex (cma__g_known_atts.mutex)) {
	cma__putstring (output, ">>Warning: known attributes list is currently locked;");
	cma__puteol    (output);
	cma__putstring (output, ">>state may be inconsistent (continuing anyway).");
	cma__puteol    (output);
	}

    qstat = cma___dbg_q_next (&cma__g_known_atts.queue, &ptr);

    if (qstat == cma___c_dbg_q_corrupt) {
	cma__putformat (
	    output,
	    "  <<KNOWN ATTRIBUTES QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
	    &cma__g_known_atts.queue,
	    cma__g_known_atts.queue.flink,
	    cma__g_known_atts.queue.blink);
	cma__puteol (output);
	return;
	}

    while (ptr != &cma__g_known_atts.queue) {
	cma__t_int_attr	*att;

	att = (cma__t_int_attr *)ptr;

	if (select_id == 0 || select_id == att->header.sequence) {
	    cma__putstring  (output, "Attributes object ");
	    cma___putname   (output, att);
	    cma__putformat  (output,
		    " %d (0x%lx)",
		    att->header.sequence,
		    (cma_t_integer)att);
	    cma__puteol	    (output);

	    cma__putstring  (output, "  reference count ");
	    cma__putint	    (output, att->refcnt);
	    cma__putstring  (output, (char *) 
		(att->delete_pending ? ", delete pending" : ""));
	    cma__puteol	    (output);

	    if (show_full) {
		cma__putformat  (output,
			"  cached objects: %d thread%s, %d attributes object%s",
			att->cache[cma__c_obj_tcb].count,
			(att->cache[cma__c_obj_tcb].count == 1?"":"s"),
			att->cache[cma__c_obj_attr].count,
			(att->cache[cma__c_obj_attr].count == 1?"":"s"));
		cma__puteol     (output);
		}

	    cma__putstring  (output, "  scheduling: (priority ");
	    cma__putint	    (output, att->priority);
	    cma__putstring  (output, ", policy ");
	    cma__putstring  (output, policy [(int)att->policy]);
	    cma__putstring  (output, "); ");
	    cma__putstring  (output, (char *) 
		(att->inherit_sched ? "inherit scheduling" : "use default"));
	    cma__puteol	    (output);

	    cma__putstring  (output, "  stack size ");
	    cma__putint	    (output, att->stack_size);
	    cma__putstring  (output, ", guard size ");
	    cma__putint	    (output, att->guard_size);
	    cma__puteol	    (output);

	    cma__putstring  (output, "  mutex type ");
	    cma__putstring  (output, mutex_type [(int)att->mutex_kind]);
	    cma__puteol	    (output); 
	    }

	qstat = cma___dbg_q_next (ptr, &ptr);

	if (qstat != cma___c_dbg_q_full) {
	    cma__putformat (
		output,
		"  <<KNOWN ATTRIBUTES QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
		ptr,
		ptr->flink,
		ptr->blink);
	    cma__puteol (output);
	    return;
	    }

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Print the status of all condition variables.
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	argument count
 *
 *	argv	argument array
 *
 *  IMPLICIT INPUTS:
 *
 *	known cv queue.
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
 *	Print information on each known condition variable.
 */
extern void
cma__debug_list_cvs
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma__t_queue	*ptr;
    cma_t_integer	select_id = 0, arg;
    cma_t_boolean	select_all = cma_c_true;
    cma_t_boolean	select_waiters = cma_c_false;
    cma_t_boolean	show_queue = cma_c_false;
    cma_t_boolean	show_full = cma_c_false;
    char		output[cma__c_buffer_size];
    cma_t_integer	cols;
    int			qstat, i;


    output[0] = '\0';
    for (arg = 1; arg < argc; arg++) {

	if (argv[arg][0] == '-')	/* If a switch */
	    for (i = 1; argv[arg][i] != 0; i++) {
		switch (argv[arg][i]) {
		    case 'f' : {
			show_full = cma_c_true;
			break;
			}
		    case 'q' : {
			show_queue = cma_c_true;
			break;
			}
		    case 'w' : {
			select_all = cma_c_false;
			select_waiters = cma_c_true;
			break;
			}
		    }
		}
	else
	    select_id = cma__atol (argv[arg]);

	}

    if (cma___test_mutex (cma__g_known_cvs.mutex)) {
	cma__putstring (output, ">>Warning: known conditions list is currently locked;");
	cma__puteol(output);
	cma__putstring (output, ">>state may be inconsistent (continuing anyway).");
	cma__puteol(output);
	}

    qstat = cma___dbg_q_next (&cma__g_known_cvs.queue, &ptr);

    if (qstat == cma___c_dbg_q_corrupt) {
	cma__putformat (
	    output,
	    "  <<KNOWN CONDITION QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
	    &cma__g_known_cvs.queue,
	    cma__g_known_cvs.queue.flink,
	    cma__g_known_cvs.queue.blink);
	cma__puteol (output);
	return;
	}

    while (ptr != &cma__g_known_cvs.queue) {
	cma__t_int_cv	*cv;

	cv = (cma__t_int_cv *)ptr;

	if (
		((select_id == 0) || (select_id == cv->header.sequence))
		&& (select_all || (select_waiters
		&& (cma___dbg_q_empty (&cv->queue) != cma___c_dbg_q_empty)))) {
	    cma__t_queue	*que;
	    cma_t_integer	count;
	    cma_t_boolean	oneout = cma_c_false;


	    cols = cma__putstring (output, "Condition variable ");
	    cols += cma___putname (output, cv);
	    cols += cma__putformat (
		    output,
		    " %d (0x%lx)",
		    cv->header.sequence,
		    (long)cv);

	    if (show_full) {

		if (!cv->defer_sig) {

		    if (cols > 60) {
			cma__puteol (output);
			cma__putstring (output, "  ");
			cols = 2;
			}

		    cols += cma__putstring (output, " [set: sig");
		    oneout = cma_c_true;
		    }

		if (!cv->defer_bro) {

		    if (oneout)
			cols += cma__putstring (output, ",");
		    else {

			if (cols > 60) {
			    cma__puteol (output);
			    cma__putstring (output, "  ");
			    cols = 2;
			    }

			cols += cma__putstring (output, " [set: ");
			oneout = cma_c_true;
			}

		    cols += cma__putstring (output, "bro");
		    }

		if (!cv->nopend) {

		    if (oneout)
			cols += cma__putstring (output, ",");
		    else {

			if (cols > 60) {
			    cma__puteol (output);
			    cma__putstring (output, "  ");
			    cols = 2;
			    }

			cols += cma__putstring (output, " [set: ");
			oneout = cma_c_true;
			}

		    cols += cma__putstring (output, "pend");
		    }

#if !_CMA_UNIPROCESSOR_
		if (cv->spindle) {

		    if (oneout)
			cols += cma__putstring (output, ",");
		    else {

			if (cols > 60) {
			    cma__puteol (output);
			    cma__putstring (output, "  ");
			    cols = 2;
			    }

			cols += cma__putstring (output, " [set: ");
			oneout = cma_c_true;
			}

		    cols += cma__putstring (output, "lock");
		    }
#endif

		if (oneout)
		    cols += cma__putstring (output, "]");

		}

#ifndef NDEBUG
	    if (show_full && (cv->defsig_cnt > 0 || cv->defbro_cnt > 0
		    || cv->pend_cnt > 0)) {

		if (cols > 40) {
		    cma__puteol (output);
		    cma__putstring (output, "  ");
		    cols = 2;
		    }

		cols += cma__putformat (
			output,
			"{counters: %d sig, %d bro; %d pend wakes}",
			cv->defsig_cnt,
			cv->defbro_cnt,
			cv->pend_cnt);
		}
#endif

	    if (cma___dbg_q_empty (&cv->queue) != cma___c_dbg_q_empty) {
		qstat = cma___dbg_q_next (&cv->queue, &que);

		if (qstat != cma___c_dbg_q_full) {
		    cma__putformat (
			output,
			"  <<CONDITION WAIT QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
			&cv->queue,
			cv->queue.flink,
			cv->queue.blink);
		    cma__puteol (output);
		    }
		else {
		    for (count = 0; que != &cv->queue; count++) {
			qstat = cma___dbg_q_next (que, &que);

			if (qstat != cma___c_dbg_q_full) {
			    cma__putformat (
				output,
				"  <<CONDITION WAIT QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
				&que,
				que->flink,
				que->blink);
			    cma__puteol (output);
			    return;
			    }

			}

		    cols += cma__putstring (output, ", ");

		    if (cols > 76) {
			cma__puteol (output);
			cma__putstring (output, "  ");
			cols = 2;
			}

		    cols += cma__putint (output, count);

		    if (count == 1)
			cols += cma__putstring (output, " waiter");
		    else
			cols += cma__putstring (output, " waiters");

		    if (show_queue) {
			cma__t_int_tcb	*thd;
			cma_t_boolean	wol = cma_c_false;


			qstat = cma___dbg_q_next (&cv->queue, &que);

			if (qstat != cma___c_dbg_q_full) {
			    cma__putformat (
				output,
				"  <<CONDITION WAIT QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
				&cv->queue,
				cv->queue.flink,
				cv->queue.blink);
			    cma__puteol (output);
			    return;
			    }

			thd = (cma__t_int_tcb *)que;

			cols += cma__putstring (output, ": ");

			while (que != &cv->queue) {

			    if (wol)
				cols += cma__putstring (output, ",");

			    if (cols >= 76) {
				cma__puteol (output);
				cma__putstring (output, "  ");
				cols = 2;
				wol = cma_c_false;
				}

			    cols += cma__putint (output, thd->header.sequence);
			    wol = cma_c_true;

			    qstat = cma___dbg_q_next (que, &que);

			    if (qstat != cma___c_dbg_q_full) {
				cma__putformat (
				    output,
				    "  <<CONDITION WAIT QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
				    &que,
				    que->flink,
				    que->blink);
				cma__puteol (output);
				return;
				}

			    thd = (cma__t_int_tcb *)que;
			    }

			}

		    }

		}

	    if (cols > 2)
		cma__puteol (output);

	    }

	qstat = cma___dbg_q_next (ptr, &ptr);

	if (qstat != cma___c_dbg_q_full) {
	    cma__putformat (
		output,
		"  <<KNOWN CONDITION QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
		ptr,
		ptr->flink,
		ptr->blink);
	    cma__puteol (output);
	    return;
	    }

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Print the status of all mutexes
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	argument count
 *
 *	argv	argument array
 *
 *  IMPLICIT INPUTS:
 *
 *	known mutex queue.
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
 *	Print information on each known mutex.
 */
extern void
cma__debug_list_mutexes
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma__t_queue	*ptr;
    cma_t_integer	select_id = 0, arg;
    cma_t_boolean	select_all = cma_c_true;
    cma_t_boolean	select_locked = cma_c_false;
    cma_t_boolean	show_full = cma_c_false;
    cma_t_boolean	show_queue = cma_c_false;
    char		output[cma__c_buffer_size];
    cma_t_integer	cols = 3;
    int			qstat, i;


    output[0] = '\0';
    for (arg = 1; arg < argc; arg++) {

	if (argv[arg][0] == '-')	/* If a switch */
	    for (i = 1; argv[arg][i] != 0; i++) {
		switch (argv[arg][i]) {
		    case 'f' : {
			show_full = cma_c_true;
			break;
			}
		    case 'l' : {
			select_all = cma_c_false;
			select_locked = cma_c_true;
			break;
			}
		    case 'q' : {
			show_queue = cma_c_true;
			break;
			}
		    }
		}
	else
	    select_id = cma__atol (argv[arg]);

	}

    if (cma___test_mutex (cma__g_known_mutexes.mutex)) {
	cma__putstring(output, ">>Warning: known mutex list is currently locked;");
	cma__puteol(output);
	cma__putstring(output, ">>state may be inconsistent (continuing anyway).");
	cma__puteol(output);
	}

    qstat = cma___dbg_q_next (&cma__g_known_mutexes.queue, &ptr);

    if (qstat == cma___c_dbg_q_corrupt) {
	cma__putformat (
	    output,
	    "  <<KNOWN MUTEX QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
	    &cma__g_known_mutexes.queue,
	    cma__g_known_mutexes.queue.flink,
	    cma__g_known_mutexes.queue.blink);
	cma__puteol (output);
	return;
	}

    while (ptr != &cma__g_known_mutexes.queue) {
	cma__t_int_mutex	*mutex;

	mutex = (cma__t_int_mutex *)ptr;

	if (
		((select_id == 0) || (select_id == mutex->header.sequence))
		&&
		(select_all || (select_locked && cma___test_mutex (mutex)))
		) {
	    cols = cma__putstring (output, "Mutex ");
	    cols += cma___putname (output, mutex);
	    cols += cma__putformat (
		    output,
		    " %d (0x%lx), type %s",
		    mutex->header.sequence,
		    (long)mutex,
		    mutex_type [(int)mutex->mutex_kind]);

	    if (!cma___test_mutex (mutex))
		cols += cma__putstring (output, ", unlocked");
	    else {

		if (mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr)
		    cols += cma__putstring (output, ", locked");
		else
		    cols += cma__putformat (
			    output,
			    ", locked by %d",
			    mutex->owner->header.sequence);

		if (mutex->mutex_kind == cma_c_mutex_recursive) {

		    if (cols >= 67) {
			cma__puteol (output);
			cols = cma__putstring (output, "  (depth ");
			}
		    else
			cols += cma__putstring (output, " (depth ");

		    cols += cma__putint (output, mutex->nest_count);
		    cols += cma__putstring (output, ")");
		    }

		if (show_full) {
#if !_CMA_UNIPROCESSOR_
		    if (mutex->spindle) {

			if (cols > 60) {
			    cma__puteol (output);
			    cma__putstring (output, "  ");
			    cols = 2;
			    }

			cols += cma__putstring (output, "[locked]");
			}
#endif
		    }

		if (cma___dbg_q_empty (&mutex->queue) != cma___c_dbg_q_empty) {
		    cma__t_queue	*que;
		    cma_t_integer	count;


		    qstat = cma___dbg_q_next (&mutex->queue, &que);

		    if (qstat != cma___c_dbg_q_full) {
			cma__putformat (
			    output,
			    "  <<MUTEX WAIT QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
			    &mutex->queue,
			    mutex->queue.flink,
			    mutex->queue.blink);
			cma__puteol (output);
			}
		    else {

			for (count = 0; que != &mutex->queue; count++) {
			    qstat = cma___dbg_q_next (que, &que);

			    if (qstat != cma___c_dbg_q_full) {
				cma__putformat (
				    output,
				    "  <<MUTEX WAIT QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
				    &que,
				    que->flink,
				    que->blink);
				cma__puteol (output);
				return;
				}

			    }

			if (cols >= 69) {
			    cma__putstring (output, ",");
			    cma__puteol (output);
			    cma__putstring (output, "  ");
			    cols = 2;
			    }
			else {
			    cma__putstring (output, ", ");
			    cols += 2;
			    }

			cols += cma__putint (output, count);
			cols += cma__putstring (output, " waiting");

			if (show_queue) {
			    cma_t_boolean	wol = cma_c_false;
			    cma__t_int_tcb	*thd;


			    qstat = cma___dbg_q_next (&mutex->queue, &que);

			    if (qstat != cma___c_dbg_q_full) {
				cma__putformat (
				    output,
				    "  <<MUTEX WAIT QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
				    &mutex->queue,
				    mutex->queue.flink,
				    mutex->queue.blink);
				cma__puteol (output);
				return;
				}

			    thd = (cma__t_int_tcb *)que;

			    cols += cma__putstring (output, ": ");

			    while (que != &mutex->queue) {

				if (wol)
				    cols += cma__putstring (output, ",");

				if (cols >= 76) {
				    cma__puteol (output);
				    cma__putstring (output, "  ");
				    cols = 2;
				    wol = cma_c_false;
				    }

				cols += cma__putint (output, thd->header.sequence);
				wol = cma_c_true;

				qstat = cma___dbg_q_next (que, &que);

				if (qstat != cma___c_dbg_q_full) {
				    cma__putformat (
					output,
					"  <<MUTEX WAIT QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
					&que,
					que->flink,
					que->blink);
				    cma__puteol (output);
				    return;
				    }

				thd = (cma__t_int_tcb *)que;
				}

			    }

			}

		    }

		}

	    if (cols > 2)
		cma__puteol (output);

	    }

	qstat = cma___dbg_q_next (ptr, &ptr);

	if (qstat != cma___c_dbg_q_full) {
	    cma__putformat (
		output,
		"  <<KNOWN MUTEX QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
		ptr,
		ptr->flink,
		ptr->blink);
	    cma__puteol (output);
	    return;
	    }

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Print the status of all threads.
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	argument count
 *
 *	argv	argument array
 *
 *  IMPLICIT INPUTS:
 *
 *	known thread queue.
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
 *	Print information on each known thread.
 */
extern void
cma__debug_list_threads
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma__t_queue	*ptr;
    cma_t_boolean	suppress_header = cma_c_false;
    cma_t_integer	arg;
    cma_t_integer	select_id = 0;
    cma_t_integer	select_flags = 0;
    cma_t_integer	display_flags = 0;
    cma_t_integer	sched_comparison = cma___c_cmp_geq;
    cma_t_boolean	operator_found = cma_c_false;
    cma_t_sched_policy	sched_policy = cma_c_sched_throughput;
    cma_t_priority	sched_priority;
    char		output[cma__c_buffer_size];
    int			qstat, j;


    output[0] = '\0';
    for (arg = 1; arg < argc; arg++) {

	if (argv[arg][0] == '-') {	/* If a switch */
	    int ta = arg;

	    for (j = 1; argv[ta][j] != 0; j++) {
		switch (argv[ta][j]) {
		    case 'f' : {
			display_flags |= DISPLAY_FULL;
			break;
			}
		    case 'r' : {
			select_flags |= SELECT_READY;
			break;
			}
		    case 'b' : {
			select_flags |= SELECT_BLOCKED;
			break;
			}
		    case 't' : {
			select_flags |= SELECT_TERMINATED;
			break;
			}
		    case 'z' : {
			select_flags |= SELECT_ZOMBIE;
			break;
			}
		    case 'c' : {
			select_flags |= SELECT_CURRENT;
			break;
			}
		    case 'i' : {
			select_flags |= SELECT_INTERNAL;
			break;
			}
		    case 'd' : {
			select_flags |= SELECT_DETACHED;
			break;
			}
		    case 'a' : {
			select_flags |= SELECT_NULL;
			break;
			}
		    case 'h' : {
			select_flags |= SELECT_HOLD;
			break;
			}
		    case 'm' : {
			select_flags |= SELECT_ME;
			break;
			}
		    case 'n' : {
			select_flags |= SELECT_NOHOLD;
			break;
			}
		    case 's' : {
			char		*ch, *str;
			cma_t_integer	len;


			/*
			 * Parse a scheduling selector:
			 *
			 *	    <policy><op><priority>
			 *
			 * Where <policy> is "fifo", "rr", "throughput",
			 * "background", "ada_low", or "idle". It may be
			 * abbreviated.
			 *
			 * Where <op> is "==", "!=", "<", ">", "<=", or ">=",
			 * and controls how threads are selected against the
			 * priority (not the policy) specified.
			 *
			 * Where <priority> is an integer, or "min", or
			 * "mid", or "max".  The keywords are interpreted
			 * relative to the specified scheduling policy.
			 */
			select_flags |= SELECT_SCHED;
			ch = str = argv[++arg];
			len = 0;

			while (*ch != '\0'
				&& *ch != '>'
				&& *ch != '<'
				&& *ch != '='
				&& *ch != '!') {
			    len++;
			    ch++;
			    }

			if (len > 0) {
			    cma_t_integer	i;
			    cma_t_boolean	found = cma_c_false;


			    for (i = 0; i < cma__c_max_policies; i++) {

				if (cma__strncmp (str, policy[i], len) == 0) {
				    sched_policy = (cma_t_sched_policy)i;
				    found = cma_c_true;
				    break;
				    }

				}

			    if (!found) {
				cma__putstring (output, "%Bad policy: ");
				cma__putstring (output, str);
				cma__puteol (output);
				output[0] = '\0';
				return;
				}

			    }

			if (*ch == '!' || *ch == '=' || *ch == '>' || *ch == '<') {
			    cma_t_boolean	bad_operator = cma_c_false;


			    switch (*ch++) {
				case '!' : {
				    if (*ch == '=') {
					ch++;
					sched_comparison = cma___c_cmp_neq;
					}
				    else
					bad_operator = cma_c_true;
				    break;
				    }
				case '=' : {
				    if (*ch == '=') {
					ch++;
					sched_comparison = cma___c_cmp_eql;
					}
				    else
					bad_operator = cma_c_true;
				    break;
				    }
				case '>' : {
				    if (*ch == '=') {
					ch++;
					sched_comparison = cma___c_cmp_geq;
					}
				    else
					sched_comparison = cma___c_cmp_gtr;
				    break;
				    }
				case '<' : {
				    if (*ch == '=') {
					ch++;
					sched_comparison = cma___c_cmp_leq;
					}
				    else
					sched_comparison = cma___c_cmp_lss;
				    break;
				    }
				default : {
				    bad_operator = cma_c_true;
				    break;
				    }

				}

			    if (bad_operator) {
				cma__putstring (output, "%Bad operator in ");
				cma__putstring (output, str);
				cma__puteol (output);
				return;
				}
			    else
				operator_found = cma_c_true;

			    }
			else if (*ch != '\0') {	/* Junk at end of switch */
			    cma__putstring (output, "%Bad scheduling selector: ");
			    cma__putstring (output, str);
			    cma__puteol (output);
			    }

			/*
			 * Priority may be a keyword: "min" for the minimum
			 * value of the current policy, "mid" for the
			 * midrange, or "max" for the maximum.
			 */
			if (*ch >= 'a' && *ch <= 'z') {	/* Priority is a keyword */
			    cma_t_integer	i;
			    cma_t_boolean	found = cma_c_false;
			    char		*pri_start;


			    len = 0;
			    pri_start = ch;

			    while (*ch >= 'a' && *ch <= 'z') {
				ch++;
				len++;
				}

			    for (i = 0; i < cma___c_num_pris; i++)

				if (cma__strncmp (
					    pri_start,
					    pri_names[i],
					    len) == 0) {
				    sched_priority =
					cma__g_pri_range[sched_policy][i];
				    found = cma_c_true;
				    }

			    if (!found) {
				cma__putstring (output, "%Bad priority: ");
				cma__putstring (output, pri_start);
				cma__puteol (output);
				output[0] = '\0';
				return;
				}

			    }
			else if (*ch >= '0' && *ch <= '9') {
			    sched_priority = cma__atol (ch);

			    if (sched_priority
					< cma__g_pri_range[sched_policy][0]
				    || sched_priority
					> cma__g_pri_range[sched_policy][2]) {
				cma__putstring (output, "%Priority ");
				cma__putint (output, sched_priority);
				cma__putstring (output, " is not valid for the ");
				cma__putstring (
					output,
					policy[sched_policy]);
				cma__putstring (output, " policy; range is ");
				cma__putint (
					output,
					cma__g_pri_range[sched_policy][0]);
				cma__putstring (output, " to ");
				cma__putint (
					output,
					cma__g_pri_range[sched_policy][2]);
				cma__putstring (output, ".");
				cma__puteol (output);
				return;
				}

			    }
			else if (*ch == '\0' && !operator_found)
			    /*
			     * The default operator is ">=" and the default
			     * priority is "min"; this combination results in
			     * displaying all threads in the specified
			     * policy.
			     */
			    sched_priority = cma__g_pri_range[sched_policy][0];
			else {
			    cma__putstring (output, "%Bad priority in ");
			    cma__putstring (output, str);
			    cma__puteol (output);
			    return;
			    }

			break;
			}

		    case 'v' : {
			display_flags |= DISPLAY_VP;
			break;
			}

		    default : {
			cma__putstring (output, "%Unrecognized switch: ");
			cma__putstring (output, &argv[arg][j]);
			cma__puteol (output);
			output[0] = '\0';
			return;
			}

		    }

		}

	    }
	else
	    select_id = cma__atol (argv[arg]);

	}

    if (select_flags & SELECT_READY) {
	cma_t_integer	i;


        for (i = cma__c_prio_n_tot - 1; i >= 0; i--) {
	    qstat = cma___dbg_q_next (&cma__g_ready_list[i], &ptr);

	    if (qstat == cma___c_dbg_q_corrupt) {
		cma__putformat (
		    output,
		    "  <<READY LIST %d QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
		    i,
		    &cma__g_ready_list[i],
		    cma__g_ready_list[i].flink,
		    cma__g_ready_list[i].blink);
		cma__puteol (output);
		return;
		}

	    while (ptr != &cma__g_ready_list[i]) {
		if (select_flags & SELECT_INTERNAL)
 		    cma__debug_format_thread (
			    (cma__t_int_tcb *)ptr,
			    display_flags);
		else {
		    cma__deb_show_thread (
			    (cma__t_int_tcb *)ptr,
			    ((display_flags & DISPLAY_FULL) != 0),
			    suppress_header,
			    active_regs,
			    cma_c_null_ptr,
			    cma_c_null_ptr,
			    cma_c_null_ptr);
		    suppress_header = cma_c_true;
		    }

		qstat = cma___dbg_q_next (ptr, &ptr);

		if (qstat != cma___c_dbg_q_full) {
		    cma__putformat (
			output,
			"  <<READY LIST %d QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
			i,
			ptr,
			ptr->flink,
			ptr->blink);
		    cma__puteol (output);
		    return;
		    }

		}

	    }

	}
    else {

	cma___dbg_travthd (
		&cma__g_known_threads.queue,
		select_flags,
		display_flags,
		select_id,
		sched_comparison,
		sched_policy,
		sched_priority);

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Process a single debug command
 *
 *  FORMAL PARAMETERS:
 *
 *	cmd	The command string
 *	locked	Previous state of kernel lock before cma_debug()
 *
 *  IMPLICIT INPUTS:
 *
 *	lots of stuff
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	cma_c_true if command was "exit"
 *
 *  SIDE EFFECTS:
 *
 *	possibly just about anything
 */
extern cma_t_boolean
cma__debug_parse
#ifdef _CMA_PROTO_
	(
	char		*cmd,
	cma_t_boolean	locked)
#else	/* no prototypes */
	(cmd, locked)
	char		*cmd;
	cma_t_boolean	locked;
#endif	/* prototype */
    {
    char			*argv[cma___c_max_args];
    char			*ptr, *argptr;
    cma_t_boolean		found;
    cma_t_integer		argc;
    cma_t_integer		i;
    cma_t_integer		str_len;
    cma_t_integer		length;
    static char			argstr[cma___c_max_args * 30];
    char			output[cma__c_buffer_size];


    ptr = cmd;
    argptr = &argstr[0];
    argc = 0;
    found = cma_c_false;

    for (i = 0; i < cma___c_max_args; i++) argv[i] = cma_c_null_ptr;

    while (argc < cma___c_max_args && *ptr != '\0' && *ptr != '\n') {

	/*
	 * Skip any leading whitespace
	 */
	while ((*ptr == ' ' || *ptr == '\t')
		&& (*ptr != '\n') && (*ptr != '\0'))
	    ptr++;

	argv[argc++] = argptr;		/* Set start of next arg */

	/*
	 * Find end of argument string
	 */
	while (*ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\0')
	    *argptr++ = *ptr++;

	*argptr++ = '\0';		/* Terminate the argument */
	}

    if (argc == 0) return cma_c_false;	/* If no args, get new line */

    length = cma__strlen (argv[0]);

    for (i = 0; i < cma___c_command_count; i++) {

	if (cma__strncmp (argv[0], cmds[i].verb, length) == 0) {
	    found = cma_c_true;

	    if (cmds[i].type == cma___c_cmd_exit)
		return cma_c_true;

	    if (cmds[i].type == cma___c_cmd_modifier && locked) {
		output[0] = '\0';
		cma__putstring (output, "%Modifier commands not allowed: ");
		cma__putstring (output, "threads library is active.");
		cma__puteol (output);
		}
	    else
		(cmds[i].routine) (argc, argv);

	    break;
	    }

	}

    if (! found) {
	output[0] = '\0';
	cma__putstring (output, "Verb ");
	cma__putstring (output, argv[0]);
	cma__putstring (output, " not recognized, try one of:");
	cma__puteol (output);
	cma__debug_help (1, debug_help_short);
	}

    return cma_c_false;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Display a queue
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	Number of arguments to command
 *	argv	Array of argument strings
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
cma__debug_queue
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma__t_queue	*sptr = (cma__t_queue *)0, *ptr;
    int			arg, j;
    int			flags = 0;
    cma_t_boolean	addr_is_head = cma_c_false;
    char		output[cma__c_buffer_size];
    int			qstat;


    output[0] = '\0';

    if (argc < 2) {
	cma__putstring (output, "%Too few arguments");
	cma__puteol (output);
	return;
	}

    for (arg = 1; arg < argc; arg++) {

	if (argv[arg][0] == '-') {	/* If a switch */
	    int ta = arg;

	    for (j = 1; argv[ta][j] != 0; j++) {
		switch (argv[ta][j]) {
		    case 'h' : {
			addr_is_head = cma_c_true;
			break;
			}
		    case 'o' : {
			flags |= Q_IS_OBJ;
			break;
			}
		    case 't' : {
			flags |= Q_IS_THREAD;
			break;
			}
		    }
		}
	    }
	else
	    sptr = (cma__t_queue *)cma__strtoul (
		argv[arg],
		(char **)cma_c_null_ptr,
		0);

	}

    if (sptr == (cma__t_queue *)0) {
	cma__putstring (output, "Command 'queue' requires address argument");
	cma__puteol (output);
	return;
	}

    if (!addr_is_head)
	cma__debug_format_object (sptr, flags);

    qstat = cma___dbg_q_next (sptr, &ptr);

    if (qstat == cma___c_dbg_q_corrupt) {
	cma__putformat (
		output,
		"%lx: corrupt",
		sptr);
	cma__puteol (output);
	return;
	}

    while (ptr != sptr) {
	cma__debug_format_object (ptr, flags);
	qstat = cma___dbg_q_next (ptr, &ptr);

	if (qstat != cma___c_dbg_q_full) {
	    cma__putformat (
		output,
		"%lx: bad queue element",
		ptr);
	    cma__puteol (output);
	    return;
	    }

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set thread attributes
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	Number of arguments to command
 *	argv	Array of argument strings
 *
 *  IMPLICIT INPUTS:
 *
 *	Known thread list
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
cma__debug_set_thread
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma_t_integer	thd_num = -1, arg;
    cma__t_int_tcb	*tcb;
    cma__t_queue	*ptr;
    char		output[cma__c_buffer_size];
    cma_t_boolean	set_cancel = cma_c_false;
    cma_t_boolean	set_sched = cma_c_false;
    cma_t_boolean	set_hold = cma_c_false;
    cma_t_boolean	set_nohold = cma_c_false;
    cma_t_boolean	set_active = cma_c_false;
    cma_t_boolean	set_visible = cma_c_false;
    cma_t_boolean	conflicting = cma_c_false;
    cma_t_boolean	boolsetbuf;
    cma_t_sched_policy	sched_policy;
    cma_t_boolean	policy_present = cma_c_false;
    cma_t_priority	sched_priority;
    cma_t_boolean	sched_prio_is_key = cma_c_false;
    cma_t_integer	sched_prio_key;
    cma_t_boolean	do_all_threads = cma_c_false;
    int			qstat, j;


    output[0] = '\0';

    if (argc < 2) {
	cma__putstring (output, "%Too few arguments");
	cma__puteol (output);
	return;
	}

    for (arg = 1; arg < argc; arg++) {

	if (argv[arg][0] == '-') {	/* If a switch */
	    int ta = arg;

	    for (j = 1; argv[ta][j] != 0; j++) {
		switch (argv[ta][j]) {
		    case 'c' : {
			set_cancel = cma_c_true;
			break;
			}
		    case 's' : {
			char		*ch, *str;
			cma_t_integer	len;


			set_sched = cma_c_true;

			/*
			 * Parse a scheduling specifier:
			 *
			 *	    [<policy>]:<priority>
			 *
			 * Where <policy> is "fifo", "rr", "throughput",
			 * "background", "ada_low", or "idle". It may be
			 * abbreviated.
			 *
			 * Where <priority> is an integer, or "min", or
			 * "mid", or "max".  The keywords are interpreted
			 * relative to the specified scheduling policy.
			 */
			ch = str = argv[++arg];
			len = 0;

			while (*ch != '\0' && *ch != ':') {
			    len++;
			    ch++;
			    }

			if (len > 0) {
			    cma_t_integer	i;


			    for (i = 0; i < cma__c_max_policies; i++) {

				if (cma__strncmp (str, policy[i], len) == 0) {
				    sched_policy = (cma_t_sched_policy)i;
				    policy_present = cma_c_true;
				    break;
				    }

				}

			    }

			/*
			 * The policy is optional, but the priority is
			 * required; so if there's no colon, complain and
			 * quit.
			 */
			if (*ch++ != ':') {
			    cma__putstring (output, "%Priority is required for -s");
			    cma__puteol (output);
			    return;
			    }

			/*
			 * Priority may be a keyword: "min" for the minimum
			 * value of the current policy, "mid" for the
			 * midrange, or "max" for the maximum.
			 */
			if (*ch >= 'a' && *ch <= 'z') {	/* Priority is a keyword */
			    cma_t_integer	i;
			    char		*pri_start;
			    cma_t_boolean	found = cma_c_false;


			    sched_prio_is_key = cma_c_true;
			    len = 0;
			    pri_start = ch;

			    while (*ch >= 'a' && *ch <= 'z') {
				ch++;
				len++;
				}

			    for (i = 0; i < cma___c_num_pris; i++)

				if (cma__strncmp (
					    pri_start,
					    pri_names[i],
					    len) == 0) {

				    if (policy_present)
					sched_priority
					    = cma__g_pri_range[sched_policy][i];
				    else
					sched_prio_key = i;

				    found = cma_c_true;
				    }

			    if (!found) {
				cma__putstring (output, "%Bad priority: ");
				cma__putstring (output, pri_start);
				cma__puteol (output);
				return;
				}

			    }
			else {		/* Priority is an integer */
			    sched_priority = cma__atol (ch);

			    if (policy_present) {

				if (sched_priority
					    < cma__g_pri_range[sched_policy][0]
					|| sched_priority
					    > cma__g_pri_range[sched_policy][2]) {
				    cma__putstring (output, "%Priority ");
				    cma__putint (output, sched_priority);
				    cma__putstring (
					    output,
					    " is not valid for the ");
				    cma__putstring (
					    output,
					    policy[(int)sched_policy]);
				    cma__putstring (
					    output,
					    " policy; range is ");
				    cma__putint (
					    output,
					    cma__g_pri_range[sched_policy][0]);
				    cma__putstring (output, " to ");
				    cma__putint (
					    output,
					    cma__g_pri_range[sched_policy][2]);
				    cma__putstring (output, ".");
				    cma__puteol (output);
				    return;
				    }

				}

			    }

			break;
			}
		    case 'h' : {
			if (set_nohold)
			    conflicting = cma_c_true;
			else
			    set_hold = cma_c_true;

			break;
			}
		    case 'n' : {
			if (set_hold)
			    conflicting = cma_c_true;
			else
			    set_nohold = cma_c_true;

			break;
			}
		    case 'a' : {
			set_active = cma_c_true;
			break;
			}
		    case 'v' : {
			set_visible = cma_c_true;
			break;
			}

		    }

		}

	    }
	else {

	    if (argv[arg][0] == '*' && argv[arg][1] == '\0')
		do_all_threads = cma_c_true;
	    else
		thd_num = cma__atol (argv[arg]);

	    }

	}

    if (thd_num < 0 && !do_all_threads) {
	output[0] = '\0';
	cma__putstring (output, "%Thread number is required");
	cma__puteol (output);
	return;
	}

    if (do_all_threads && (set_active || set_visible || set_cancel))
	conflicting = cma_c_true;

    if (conflicting) {
	cma__putstring (output, "%Conflicting switches were specified:");
	cma__putstring (output, " some have been ignored.");
	cma__puteol (output);
	}

    tcb = (cma__t_int_tcb *)cma_c_null_ptr;

    qstat = cma___dbg_q_next (&cma__g_known_threads.queue, &ptr);

    if (qstat == cma___c_dbg_q_corrupt) {
	cma__putformat (
	    output,
	    "  <<KNOWN THREADS QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
	    &cma__g_known_threads.queue,
	    cma__g_known_threads.queue.flink,
	    cma__g_known_threads.queue.blink);
	cma__puteol (output);
	return;
	}

    while (ptr != &cma__g_known_threads.queue) {
	cma__t_int_tcb	*thd = (cma__t_int_tcb *)ptr;


	thd = cma__base (ptr, threads, cma__t_int_tcb);	/* Get TCB base */

	if (do_all_threads && (thd->kind != cma__c_thkind_null)) {

	    if (set_hold) {
		boolsetbuf = cma_c_true;
		cma__deb_set (
			thd,
			cma_c_debset_hold,
			(cma_t_address)&boolsetbuf,
			(cma_t_integer)(sizeof (cma_t_boolean)));
		}

	    if (set_nohold) {
		boolsetbuf = cma_c_false;
		cma__deb_set (
			thd,
			cma_c_debset_hold,
			(cma_t_address)&boolsetbuf,
			(cma_t_integer)(sizeof (cma_t_boolean)));
		}

	    if (set_sched) {

		if (policy_present)
		    cma__deb_set (
			    thd,
			    cma_c_debset_policy,
			    (cma_t_address)&sched_policy,
			    (cma_t_integer)(sizeof (cma_t_sched_policy)));

		cma__deb_set (
			thd,
			cma_c_debset_priority,
			(cma_t_address)&sched_priority,
			(cma_t_integer)(sizeof (cma_t_priority)));
		}
	    
	    }
	else if (thd->header.sequence == thd_num) {
	    tcb = thd;
	    break;
	    }

	qstat = cma___dbg_q_next (ptr, &ptr);

	if (qstat != cma___c_dbg_q_full) {
	    cma__putformat (
		output,
		"  <<KNOWN THREADS QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
		ptr,
		ptr->flink,
		ptr->blink);
	    cma__puteol (output);
	    return;
	    }

	}

    /*
     * If we're changing all threads, then we did everything during the
     * traversal.
     */
    if (do_all_threads)
	return;

    if (tcb == (cma__t_int_tcb *)cma_c_null_ptr) {
	output[0] = '\0';
	cma__putstring (output, "%There is no thread with number ");
	cma__putint (output, thd_num);
	cma__puteol (output);
	return;
	}

    if (tcb->kind == cma__c_thkind_null) {
	cma__putstring (output, "%Null thread cannot be modified.");
	cma__puteol (output);
	}

    if (set_cancel)
	(void)cma__deb_set_alert (tcb);

    if (set_hold) {
	boolsetbuf = cma_c_true;
	cma__deb_set (
		tcb,
		cma_c_debset_hold,
		(cma_t_address)&boolsetbuf,
		(cma_t_integer)(sizeof (cma_t_boolean)));
	}

    if (set_nohold) {
	boolsetbuf = cma_c_false;
	cma__deb_set (
		tcb,
		cma_c_debset_hold,
		(cma_t_address)&boolsetbuf,
		(cma_t_integer)(sizeof (cma_t_boolean)));
	}

    if (set_active) {
#if _CMA_THREAD_IS_VP_
	/*
	 * On a one-to-one VP implementation, all threads are concurrently
	 * active, so this switch can't do anything useful.
	 */
#else
	cma__deb_set_next_thread (tcb);
#endif
	}

    if (set_visible) {
#if !_CMA_THREAD_IS_VP_
	cma__putstring (output, "'set visible' not implemented; -v ignored");
	cma__puteol (output);
#else
	cma__vp_interrupt (
		tcb->sched.processor->vp_id,
		(cma__t_vp_handler)cma__do_break,
		cma_c_null_ptr,
		cma__vp_get_sequence (tcb->sched.processor->vp_id));
#endif
	}

    if (set_sched) {

	if (policy_present)
	    cma__deb_set (
		    tcb,
		    cma_c_debset_policy,
		    (cma_t_address)&sched_policy,
		    (cma_t_integer)(sizeof (cma_t_sched_policy)));
	else {
	    /*
	     * If policy wasn't specified on command line, then we need to do
	     * keyword interpretation or range checking.
	     */
	    sched_policy = tcb->sched.policy;	/* Fetch actual policy */

	    if (sched_prio_is_key)
		sched_priority
		    = cma__g_pri_range[sched_policy][sched_prio_key];
	    else {

		if (sched_priority < cma__g_pri_range[sched_policy][0]
			|| sched_priority > cma__g_pri_range[sched_policy][2]) {
		    cma__putstring (output, "%Priority ");
		    cma__putint (output, sched_priority);
		    cma__putstring (output, " is not valid for the ");
		    cma__putstring (output, policy[(int)sched_policy]);
		    cma__putstring (output, " policy; range is ");
		    cma__putint (output, cma__g_pri_range[sched_policy][0]);
		    cma__putstring (output, " to ");
		    cma__putint (output, cma__g_pri_range[sched_policy][2]);
		    cma__putstring (output, ".");
		    cma__puteol (output);
		    return;
		    }

		}

	    }

	cma__deb_set (
		tcb,
		cma_c_debset_priority,
		(cma_t_address)&sched_priority,
		(cma_t_integer)(sizeof (cma_t_priority)));
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Show DECthreads information
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	Number of arguments to command
 *	argv	Array of argument strings
 *
 *  IMPLICIT INPUTS:
 *
 *	All sorts of random externs and stuff
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
cma__debug_show
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma_t_integer	arg, i;
    cma_t_boolean	show_current = cma_c_false;
    cma_t_boolean	show_mem = cma_c_false;
    cma_t_boolean	show_kernel = cma_c_false;
    cma_t_boolean	show_sched = cma_c_false;
    cma_t_boolean	show_sched_ada = cma_c_false;
    cma_t_boolean	show_pool = cma_c_false;
    cma_t_boolean	show_vp = cma_c_false;
    char		buffer[120];


    buffer[0] = '\0';

    if (argc < 2) {
	cma__putstring (buffer, "Too few arguments");
	cma__puteol (buffer);
	return;
	}

    for (arg = 1; arg < argc; arg++) {

	if (argv[arg][0] == '-')	/* If a switch */
	    for (i = 1; argv[arg][i] != 0; i++) {
		switch (argv[arg][i]) {
		    case 'a' : {

			if (show_sched)
			    show_sched_ada = cma_c_true;

			break;
			}
		    case 'c' : {
			show_current = cma_c_true;
			break;
			}
		    case 'm' : {
			show_mem = cma_c_true;
			break;
			}
		    case 'p' : {
			show_pool = cma_c_true;
			break;
			}
#ifdef _CMA_TRACE_KERNEL_
		    case 'k' : {
			show_kernel = cma_c_true;
			break;
			}
#endif
		    case 's' : {
			show_sched = cma_c_true;
			break;
			}

		    case 'v' : {
			show_vp = cma_c_true;
			break;
			}

		    }

		}

	}

    if (show_current) {
	cma__t_int_tcb	*self = cma__get_self_tcb ();


	/*
	 * Display the current thread
	 */
	cma__putformat (
		buffer,
		"Current thread is %d (0x%lx)",
		self->header.sequence,
		self);
	cma__puteol (buffer);
	}

    if (show_mem) {
	/*
	 * We're doing this without locking the VM database, since we don't
	 * know what state we're in. Watch it!
	 */
	cma__putstring (buffer, "DECthreads internal VM manager statistics:");
	cma__puteol (buffer);
	for (i = 0; i < cma__c_pool; i++) {
	    cma__putformat (
		    buffer,
		    " %s (%d bytes): %d allocated, %d free",
		    cma__g_vm_names[i],
		    cma__g_memory[i].size,
		    cma__g_memory[i].total,
		    cma__g_memory[i].count);
	    cma__puteol (buffer);
	    }

	cma__putformat (
		buffer,
		" %d thing%s currently on general pool list",
		cma__g_memory[cma__c_pool].count,
		(cma__g_memory[cma__c_pool].count == 1 ? "" : "s"));
	cma__puteol (buffer);
#ifndef NDEBUG
	cma__putformat (
		buffer,
		" %d zeroed allocations (%d bytes)",
		cma__g_pool_stat.zero_allocs,
		cma__g_pool_stat.zero_bytes);
	cma__puteol (buffer);
#endif
	cma__putformat (
		buffer,
		" %d syscalls for %d bytes (%d exact size), %d attempts failed",
		cma__g_pool_stat.allocs,
		cma__g_pool_stat.bytes_allocd,
		cma__g_pool_stat.exact_allocs,
		cma__g_pool_stat.failures);
	cma__puteol (buffer);
	cma__putformat (
		buffer,
		" %d pool extractions; %d split from larger packets",
		cma__g_pool_stat.extractions,
		cma__g_pool_stat.breakups);
	cma__puteol (buffer);
	cma__putformat (
		buffer,
		" %d pool returns; %d merged with previous, %d with next",
		cma__g_pool_stat.returns,
		cma__g_pool_stat.merge_befores,
		cma__g_pool_stat.merge_afters);
	cma__puteol (buffer);
	cma__putformat (
		buffer,
		" Lookaside scrounging: %d small, %d medium, %d large",
		cma__g_pool_stat.scrounges[0],
		cma__g_pool_stat.scrounges[1],
		cma__g_pool_stat.scrounges[2]);
	cma__puteol (buffer);
#if _CMA_OS_ == _CMA__UNIX
	cma__putformat (buffer, " %d sbrk alignments", cma__g_sbrk_align);
	cma__puteol (buffer);
#endif
	}

    if (show_pool) {
	cma__t_queue	*p;
	cma_t_integer	width;
	int		qstat;


	cma__putstring (buffer, "General pool element list:");
	cma__puteol (buffer);
	qstat = cma___dbg_q_next (&cma__g_memory[cma__c_pool].queue, &p);

	if (qstat == cma___c_dbg_q_corrupt) {
	    cma__putformat (
		buffer,
		"  <<GENERAL POOL QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
		&cma__g_memory[cma__c_pool].queue,
		cma__g_memory[cma__c_pool].queue.flink,
		cma__g_memory[cma__c_pool].queue.blink);
	    cma__puteol (buffer);
	    return;
	    }

	width = 0;

	while (p != &cma__g_memory[cma__c_pool].queue) {
	    width += 16;

	    if (width >= 80) {
		cma__puteol (buffer);
		width = 16;
		}

	    cma__putformat (
		    buffer,
		    " 0x%lx(%d)",
		    p,
		    *(cma_t_natural *)((cma_t_natural)p - cma__c_mem_tag));
	    qstat = cma___dbg_q_next (p, &p);

	    if (qstat != cma___c_dbg_q_full) {
		cma__putformat (
		    buffer,
		    "  <<GENERAL POOL QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
		    p,
		    p->flink,
		    p->blink);
		cma__puteol (buffer);
		return;
		}

	    }

	cma__puteol (buffer);
	}

#ifdef _CMA_TRACE_KERNEL_
    if (show_kernel)
	cma__format_karray ();
#endif

    if (show_sched) {
	int	pol, max;


	/*
	 * Display the priority range of each scheduling policy.
	 */

	if (show_sched_ada)
	    max = cma_c_sched_ada_rtb;
	else
	    max = cma_c_sched_background;

	for (pol = cma_c_sched_fifo; pol <= max; pol++) {
	    cma__putformat (
		    buffer,
		    "Policy %s: low %d, mid %d, high %d",
		    policy[pol],
		    cma__g_pri_range[pol][0],
		    cma__g_pri_range[pol][1],
		    cma__g_pri_range[pol][2]);
#if _CMA_RT4_KTHREAD_

	    if (pol <= cma_c_sched_rr)
		cma__putstring (buffer, ", max. allowed: 63");

#endif
	    cma__puteol (buffer);
	    }

	}

    if (show_vp)
	cma__vp_dump ();

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Lock the kernel (if possible) and do other setup for main debug entry.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	Kernel lock
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	previous state of the kernel lock
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_boolean
cma__debug_trylock
#ifdef _CMA_PROTO_
	(cma_t_boolean message)
#else	/* no prototypes */
	(message)
	cma_t_boolean	message;
#endif	/* prototype */
    {
    cma_t_boolean	locked;
    char		output[cma__c_buffer_size];


    locked = cma__tryenter_kernel ();	/* Enter krnl */

    if (locked && message) {
	/*
	 * If kernel was already locked, print warning, but continue anyway
	 */
	output[0] = '\0';
	cma__putstring (
		output,
		"  WARNING: The cma_debug() function has been called while");
	cma__puteol (output);
	cma__putstring (
		output,
		"  the threads library was active, and the state of thread");
	cma__puteol (output);
	cma__putstring (
		output,
		"  library objects may not be consistent. You may continue");
	cma__puteol (output);
	cma__putstring
		(output,
		"  and examine the current state if you wish, but you may");
	cma__puteol (output);
	cma__putstring
		(output,
		"  not change anything. If possible, please exit from the");
	cma__puteol (output);
	cma__putstring
		(output,
		"  cma_debug() function now, and step (or set a breakpoint)");
	cma__puteol (output);
	cma__putstring
		(output,
		"  outside of the thread library before entering cma_debug().");
	cma__puteol (output);
	cma__puteol (output);
	}

    active_regs[14] = (long int)cma__fetch_sp ();
    return locked;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Print version information
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	Number of arguments to command
 *	argv	Array of argument strings
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
cma__debug_versions
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    char		buffer[100];
    char		output[cma__c_buffer_size];


    output[0] = '\0';
    cma__debug_get_system (buffer, 100);
    cma__putformat (
	    output,
	    "DECthreads version %s, %s %s [%s]",
	    cma__g_version,
	    cma__g_os,
	    cma__g_hardware,
	    buffer);
    cma__puteol (output);
    }
#if !_CMA_UNIPROCESSOR_

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Suspend or resume all other VPs.
 *
 *  FORMAL PARAMETERS:
 *
 *	argc	Number of arguments to command
 *	argv	Array of argument strings
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
cma__debug_vp
#ifdef _CMA_PROTO_
	(
	cma_t_integer	argc,
	char		*argv[])
#else	/* no prototypes */
	(argc, argv)
	cma_t_integer	argc;
	char		*argv[];
#endif	/* prototype */
    {
    cma_t_boolean	suspend_all = cma_c_false;
    cma_t_boolean	resume_all = cma_c_false;
    cma_t_integer	arg, i;
    char		output[cma__c_buffer_size];


    output[0] = '\0';

    for (arg = 1; arg < argc; arg++)
	if (argv[arg][0] == '-')	/* If a switch */
	    for (i = 1; argv[arg][i] != 0; i++) {
		switch (argv[arg][i]) {
		    case 'r' : {
			resume_all = cma_c_true;
			break;
			}
		    case 's' : {
			suspend_all = cma_c_true;
			break;
			}
		    }
		}

    if (suspend_all && resume_all) {
	cma__putstring (output, "%Conflicting switches");
	cma__puteol (output);
	return;
	}

    if (suspend_all)
	(void)cma__vp_suspend_others ();
    else if (resume_all)
	(void)cma__vp_resume_others ();

    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Check whether a queue is empty -- it will also report queue
 *	corruptions (if it can detect them).
 *
 *  FORMAL PARAMETERS:
 *
 *	qh	Queue header
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
 *	cma___c_dbg_q_empty:	queue is empty
 *
 *	cma___c_dbg_q_full:	queue is not empty
 *
 *	cma___c_dbg_q_corrupt:	queue is corrupt
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static int
cma___dbg_q_empty
#ifdef _CMA_PROTO_
	(cma__t_queue	*qh)
#else	/* no prototypes */
	(qh)
	cma__t_queue	*qh;
#endif	/* prototype */
    {
    int	status;

    TRY {
	if (qh->blink == 0 || qh->flink == 0)
	    status = cma___c_dbg_q_corrupt;
	else if (qh->blink == qh->flink && qh == qh->flink)
	    status = cma___c_dbg_q_empty;
	else
	    status = cma___c_dbg_q_full;
	}
    CATCH_ALL {
	status = cma___c_dbg_q_corrupt;
	}
    ENDTRY

    return status;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Return the next element of a queue -- report whether queue link appears
 *	to be corrupt.
 *
 *  FORMAL PARAMETERS:
 *
 *	pq	IN Previous (current) queue element
 *
 *	nq	OUT Next queue element
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
 *	cma___c_dbg_q_empty:	queue is empty
 *
 *	cma___c_dbg_q_full:	queue is not empty
 *
 *	cma___c_dbg_q_corrupt:	queue is corrupt
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static int
cma___dbg_q_next
#ifdef _CMA_PROTO_
	(cma__t_queue	*pq,
	cma__t_queue	**nq)
#else	/* no prototypes */
	(pq, nq)
	cma__t_queue	*pq;
	cma__t_queue	**nq;
#endif	/* prototype */
    {
    int			status;
    cma__t_queue	*flink, *blink;

    TRY {
	flink = pq->flink;
	blink = pq->blink;

	if (flink->blink != pq || blink->flink != pq
		|| flink->flink == 0 || blink->blink == 0)
	    status = cma___c_dbg_q_corrupt;
	else if (flink == blink && flink == pq) {
	    *nq = flink;
	    status = cma___c_dbg_q_empty;
	    }
	else {
	    *nq = flink;
	    status = cma___c_dbg_q_full;
	    }

	}
    CATCH_ALL {
	status = cma___c_dbg_q_corrupt;
	}
    ENDTRY

    return status;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Traverse a thread queue and list appropriate threads.
 *
 *  FORMAL PARAMETERS:
 *
 *	lots
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
static void
cma___dbg_travthd
#ifdef _CMA_PROTO_
	(cma__t_queue		*qhead,
	cma_t_integer		select_flags,
	cma_t_integer		display_flags,
	cma_t_integer		select_id,
	cma_t_integer		sched_comparison,
	cma_t_sched_policy	sched_policy,
	cma_t_priority		sched_priority)
#else	/* no prototypes */
	(qhead, select_flags, display_flags,
	select_id, sched_comparison, sched_policy, sched_priority)
	cma__t_queue		*qhead;
	cma_t_integer		select_flags;
	cma_t_integer		display_flags;
	cma_t_integer		select_id;
	cma_t_integer		sched_comparison;
	cma_t_sched_policy	sched_policy;
	cma_t_priority		sched_priority;
#endif	/* prototype */
    {
    char		output[cma__c_buffer_size];
    int			qstat;
    cma__t_queue	*ptr;
    cma_t_boolean	suppress_header = cma_c_false;


    /*
     * The known thread list & zombie list are protected by the kernel lock;
     * the debug main routine already owns the lock if possible, and has
     * given a warning if it was locked.  There's not much more we can do
     * here.
     */
    qstat = cma___dbg_q_next (qhead, &ptr);

    if (qstat == cma___c_dbg_q_corrupt) {
	cma__putformat (
	    output,
	    "  <<THREADS QUEUE HEAD CORRUPT AT %lx: (%lx,%lx)>>",
	    qhead,
	    qhead->flink,
	    qhead->blink);
	cma__puteol (output);
	return;
	}

    while (ptr != qhead) {
	cma__t_int_tcb	*thd;
	cma_t_boolean	null, skip = cma_c_false;


	thd = cma__base (ptr, threads, cma__t_int_tcb);

	if (thd->kind == cma__c_thkind_null
		|| thd->kind == cma__c_thkind_mgr) {

	    if (!(select_flags & SELECT_NULL)
		    && select_id != thd->header.sequence)
		skip = cma_c_true;

	    }

	if (select_id != 0 && select_id != thd->header.sequence)
	    skip = cma_c_true;

	/*
	 * Reject if switches are set and thread doesn't match (however, we
	 * ignore switches if a thread ID is specified)
	 */
	if (select_flags != 0 && select_id == 0 && ! skip) {

	    if ((select_flags & SELECT_READY) && thd->state != cma__c_state_ready)
		skip = cma_c_true;
	    if ((select_flags & SELECT_BLOCKED) && thd->state != cma__c_state_blocked)
		skip = cma_c_true;
	    if ((select_flags & SELECT_TERMINATED) && thd->state != cma__c_state_terminated)
		skip = cma_c_true;
	    if (!(select_flags & SELECT_ZOMBIE) && thd->state == cma__c_state_zombie)
		skip = cma_c_true;
	    if ((select_flags & SELECT_CURRENT) && thd->state != cma__c_state_running)
		skip = cma_c_true;
	    if ((select_flags & SELECT_DETACHED) && !thd->detached)
		skip = cma_c_true;
	    if ((select_flags & SELECT_HOLD) && !thd->debug.on_hold)
		skip = cma_c_true;
	    if ((select_flags & SELECT_ME) && (thd != cma__get_self_tcb()))
		skip = cma_c_true;
	    if ((select_flags & SELECT_NOHOLD) && thd->debug.on_hold)
		skip = cma_c_true;

	    if ((select_flags & SELECT_SCHED) && !skip) {

		if (thd->sched.policy != sched_policy)
		    skip = cma_c_true;

		switch (sched_comparison) {
		    case cma___c_cmp_eql : {
			if (thd->sched.priority != sched_priority)
			    skip = cma_c_true;
			break;
			}
		    case cma___c_cmp_neq : {
			if (thd->sched.priority == sched_priority)
			    skip = cma_c_true;
			break;
			}
		    case cma___c_cmp_lss : {
			if (thd->sched.priority >= sched_priority)
			    skip = cma_c_true;
			break;
			}
		    case cma___c_cmp_gtr : {
			if (thd->sched.priority <= sched_priority)
			    skip = cma_c_true;
			break;
			}
		    case cma___c_cmp_leq : {
			if (thd->sched.priority > sched_priority)
			    skip = cma_c_true;
			break;
			}
		    case cma___c_cmp_geq : {
			if (thd->sched.priority < sched_priority)
			    skip = cma_c_true;
			break;
			}

		    }

		}

	    }

	if (! skip) {
	    if (select_flags & SELECT_INTERNAL)
		cma__debug_format_thread (
			thd,
			display_flags);
	    else {
		cma__deb_show_thread (
			thd,
			((display_flags & DISPLAY_FULL) != 0),
			suppress_header,
			active_regs,
			cma_c_null_ptr,
			cma_c_null_ptr,
			cma_c_null_ptr);
		suppress_header = cma_c_true;
		}
	    }

	qstat = cma___dbg_q_next (ptr, &ptr);

	if (qstat != cma___c_dbg_q_full) {
	    cma__putformat (
		output,
		"  <<THREADS QUEUE CORRUPT AT %lx: (%lx,%lx)>>",
		ptr,
		ptr->flink,
		ptr->blink);
	    cma__puteol (output);
	    return;
	    }

	}

    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEBUGGER.C */
/*  *67   28-JUL-1993 06:11:10 BUTENHOF "Minor formatting" */
/*  *66   26-JUL-1993 13:34:33 BUTENHOF "Add queue walker" */
/*  *65   28-MAY-1993 12:17:36 BUTENHOF "Show spindle on mu/cv" */
/*  *64   27-MAY-1993 14:32:01 BUTENHOF "Clarify thread blocking state" */
/*  *63   17-MAY-1993 09:57:24 BUTENHOF "Fix VAX ULTRIX compilation error" */
/*  *62   14-MAY-1993 15:54:35 BUTENHOF "Fix 'thread' output bug" */
/*  *61   10-MAY-1993 14:10:53 BUTENHOF "Clean up some formatting" */
/*  *60    7-MAY-1993 14:23:37 BUTENHOF "Add 'show -sa'" */
/*  *59    6-MAY-1993 19:06:43 BUTENHOF "Add to NDEBUG VM stats" */
/*  *58    4-MAY-1993 11:38:07 BUTENHOF "Don't type long message on debug_cmd" */
/*  *57    3-MAY-1993 13:44:02 BUTENHOF "Merge zombies with known threads" */
/*  *56   30-APR-1993 18:13:32 BUTENHOF "Use uname(), etc" */
/*  *55   16-APR-1993 13:02:46 BUTENHOF "Use extern VM names" */
/*  *54    1-APR-1993 14:32:41 BUTENHOF "Add show -s" */
/*  *53   10-MAR-1993 13:56:02 BUTENHOF "Move Mach & VP states to vp module" */
/*  *52    4-MAR-1993 14:33:13 BUTENHOF "Don't stop/start VPs in debug_cmd" */
/*  *51    3-MAR-1993 17:15:41 BUTENHOF "Use new global policy range table" */
/*  *50   26-FEB-1993 08:31:33 BUTENHOF "Stop kernel threads on cma_debug()" */
/*  *49   17-FEB-1993 14:35:11 BUTENHOF "Fix thread output" */
/*  *48   28-JAN-1993 14:41:52 BUTENHOF "thread -if should list terminated thread stack" */
/*  *47   11-DEC-1992 13:45:45 BUTENHOF "Fix cma_debug_cmd" */
/*  *46    8-DEC-1992 15:14:42 BUTENHOF "Change cma_debug_cmd()" */
/*  *45    4-DEC-1992 12:41:20 BUTENHOF "Get array right for OSF/1 policy/priority" */
/*  *44    1-DEC-1992 14:05:00 BUTENHOF "OSF/1 RT" */
/*  *43   20-NOV-1992 11:18:43 BUTENHOF "Fix vp_interrupt call" */
/*  *42   11-NOV-1992 05:52:00 BUTENHOF "Fix NDEBUG bugs" */
/*  *41    6-NOV-1992 12:50:23 BUTENHOF "Fix an assertion error" */
/*  *40   13-OCT-1992 14:19:55 BUTENHOF "remove sched_ calls" */
/*  *39    7-OCT-1992 12:46:58 BUTENHOF "Fix typo" */
/*  *38   28-SEP-1992 11:48:49 BUTENHOF "Add 'owner sequence' field" */
/*  *37   21-SEP-1992 13:31:20 BUTENHOF "Skip null & mgr threads" */
/*  *36   21-SEP-1992 08:24:39 BUTENHOF "Add help info" */
/*  *35   15-SEP-1992 13:49:13 BUTENHOF "Report pend/defer in CV" */
/*  *34    2-SEP-1992 16:24:18 BUTENHOF "Remove semaphores from mutexes" */
/*  *33   25-AUG-1992 11:48:02 BUTENHOF "Don't report VP for terminated thread" */
/*  *32    3-AUG-1992 09:52:13 BUTENHOF "Fix internal q_empty check" */
/*  *31   31-JUL-1992 15:33:12 BUTENHOF "Fix hex formatting for 64bit" */
/*  *30   24-JUL-1992 06:16:24 BUTENHOF "Fix stack message" */
/*  *29   10-JUL-1992 22:39:18 BUTENHOF "Display attr. delete_pending" */
/*  *28   16-APR-1992 12:40:31 BUTENHOF "Fix access to NOWRT strings" */
/*  *27   24-MAR-1992 13:45:49 BUTENHOF "Put bugcheck output in file" */
/*  *26   17-MAR-1992 09:55:41 BUTENHOF "Fix formatting of a message" */
/*  *25    5-MAR-1992 12:06:23 BUTENHOF "Add to VP debugging" */
/*  *24   19-FEB-1992 07:16:02 BUTENHOF "Fix show -p problem" */
/*  *23   18-FEB-1992 15:28:17 BUTENHOF "Add sbrk() alignment to UNIX memory report" */
/*  *22   13-DEC-1991 09:53:35 BUTENHOF "Fix SP report" */
/*  *21   22-NOV-1991 11:55:13 BUTENHOF "Type a cast" */
/*  *20   14-OCT-1991 13:38:02 BUTENHOF "Refine/fix use of config symbols" */
/*  *19   24-SEP-1991 16:26:37 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *18   26-JUL-1991 15:53:17 CURTIN "Removed last of CRTL dependence on VMS" */
/*  *17    2-JUL-1991 16:47:03 BUTENHOF "Show mutex type on attributes object" */
/*  *16   21-JUN-1991 11:59:06 BUTENHOF "Add cast to puthex calls" */
/*  *15   10-JUN-1991 18:18:35 SCALES "Add sccs headers for Ultrix" */
/*  *14    5-JUN-1991 17:31:08 BUTENHOF "Fix bug in call to cma__vp_interrupt" */
/*  *13   29-MAY-1991 16:59:13 BUTENHOF "Accept command line as argument" */
/*  *12   14-MAY-1991 13:42:59 BUTENHOF "Integrate changes lost in disk crash" */
/*  *11   12-APR-1991 23:35:04 BUTENHOF "Fix access to atomic bits" */
/*  *10   21-MAR-1991 09:25:59 BUTENHOF "extern some static functions for Ada" */
/*  *9    14-MAR-1991 19:04:44 SCALES "Fix MIPS prototype problem" */
/*  *8    12-MAR-1991 17:24:01 BUTENHOF "Fix some MIPS compilation warnings" */
/*  *7    11-MAR-1991 19:06:56 BUTENHOF "Add new commands" */
/*  *6     8-MAR-1991 18:49:41 BUTENHOF "Add new commands" */
/*  *5    13-FEB-1991 17:54:36 BUTENHOF "Change mutex attribute name symbols" */
/*  *4    12-FEB-1991 23:09:42 BUTENHOF "Recursive/nonrecursive mutexes" */
/*  *3    12-FEB-1991 01:28:49 BUTENHOF "Change to alert control bits" */
/*  *2    14-DEC-1990 00:55:24 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:43:45 BUTENHOF "Debug support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEBUGGER.C */
