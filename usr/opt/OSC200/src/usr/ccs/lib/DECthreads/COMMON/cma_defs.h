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
 * @(#)$RCSfile: cma_defs.h,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/08/18 14:46:41 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	General DECthreads core definitions
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	24 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	15 August 1989
 *		Add cma__t_short type, and common object header
 *	002	Dave Butenhof	16 August 1989
 *		Remove boolean true/false and null pointer constants to
 *		CMA.h, since they're externally useful, too.
 *	003	Dave Butenhof	24 August 1989
 *		Modify object header to include generic queue element.
 *	004	Dave Butenhof	15 September 1989
 *		Add "attr_revision" count to object header for queue
 *		validation.
 *	005	Dave Butenhof	22 September 1989
 *		Remove erroneous semicolons in #defines (hmmm...)
 *	006	Bob Conti 	6 October 1989
 *		Add assertions interfaces to make them easy to use.
 *	007	Dave Butenhof	19 October 1989
 *		Add new macro cma__structure_base for backing off from
 *		embedded queue elements to structure head.
 *	008	Bob Conti 	4 November 1989
 *		Make cma_host.h an implicit include file for all modules.
 *	009	Webb Scales	19 November 1989
 *		Move cma__zero from cma_thread_io_low.h to here.
 *	010	Webb Scales	11 January 1990
 *		Add cma__copy macro.
 *	011	Dave Butenhof	10 April 1990
 *		Add object name field to object header, and macro to set it.
 *	012	Dave Butenhof	18 June 1990
 *		Add macros for clearing and testing header.name (clean up
 *		build for NDEBUG).
 *	013	Paul Curtin	3 July 1990
 *		Removed stack __c_ constants to cma_stack_int.h
 *	014	Paul Curtin	24 July 1990
 *		Adding generic cma__roundup macro.
 *	015	Paul Curtin	06 August 1990
 *		Replaced memcpy w/ cma__memcpy. Replaced memset w/
 *		cma__memset
 *	016	Paul Curtin	22 August 1990
 *		Added cma__max macro: return greater of two inputs
 *	017	Paul Curtin	11 September 1990
 *		Added cma__min macro: return lesser of two inputs
 *	018	Bob Conti	19 September 1990
 *		Add VAX Debug support.
 *	019	Webb Scales	10 October 1990
 *		Moved "idle" scheduling class constants here from cma.h
 *	020	Webb Scales	 6 December 1990
 *		Added HP page size constant.
 *	021	Dave Butenhof	14 December 1990
 *		Change cma_assertions.h to cma_assert.h (shorten length)
 *	022	Dave Butenhof	24 April 1991
 *		Fix cma__trace macro, and add some documentation for it.
 *	023	Dave Butenhof	03 May 1991
 *		Add new trace class, and allow this module to be included
 *		from assembly code (for trace class defs).
 *	024	Dave Butenhof	13 May 1991
 *		Add new cma__trace_kernel() to be used for tracing when
 *		inside the kernel (it will exit the kernel to prevent a
 *		possible deadlock if the thread needs to block).
 *	025	Paul Curtin	 5 June 1991
 *		Added new constants for reinit routines.
 *	026	Webb Scales	23 July 1991
 *		Added VMS versions of the printf routines.
 *	027	Dave Butenhof	25 July 1991
 *		Add vsprintf and vfprintf versions to 024, and replace by
 *		#defines to the "real" *printf functions on non-VMS systems.
 *	028	Dave Butenhof	24 September 1991
 *		When the object name field was first added, it evaporated in
 *		NDEBUG builds; therefore, names weren't set for NDEBUG
 *		builds. Since the name field long since became unconditional
 *		to solve alignment problems, we could have made the names
 *		unconditional, but didn't. Rectify that oversight now.
 *	029	Dave Butenhof	14 October 1991
 *		Fix comments about cma__t_header structure that incorrectly
 *		state it "can't be changed": the restriction is only for VMS
 *		systems, where VMS Debugger integration relies on fixed
 *		offsets.
 *	030	Dave Butenhof	30 January 1992
 *		Remove cma__trace_kernel, which isn't used much and is really
 *		too dangerous to leave around.
 *	031	Dave Butenhof	12 March 1992
 *		Add trace class to cover fork wrapper.
 *	032	Brian Keane	11 July 1992
 *		Fix cast of pointer to 32 bit int in cma__base.  Use
 *		cma_t_integer instead so Alpha OSF/1 will work.
 *	033	Dave Butenhof	30 July 1992
 *		Add cma__c_hexchars -- number of hex characters in a "long
 *		int" value.
 *	034	Dave Butenhof	19 August 1992
 *		Enhance the cma__spinlock() macro to yield on non-mP
 *		machines.
 *	035	Dave Butenhof	24 August 1992
 *		Use "swtch_pri(0)" directly in Mach spinlock -- save a few
 *		cycles.
 *	036	Dave Butenhof	15 September 1992
 *		Add cma__tryspinlock.
 *	037	Dave Butenhof	21 September 1992
 *		Change sequence # to unsigned short.
 *	038	Webb Scales	23 September 1992
 *		Change idle scheduling policy constant value to keep the
 *		others contiguous (with the addition of ada-rtb).
 *	039	Dave Butenhof	24 September 1992
 *		Add 'owner' field in object header for debug name.
 *	040	Dave Butenhof	 4 November 1992
 *		Add cma__mp_barrier macro. On MP hardware, this calls
 *		cma__memory_barrier(), which must be defined in the
 *		cma_host.h file (either as a macro or prototype for assembly
 *		code).
 *	041	Dave Butenhof	25 November 1992
 *		the idle scheduling policy needs to be in line with the DEC
 *		OSF/1 RT values on appropriate systems.
 *	042	Dave Butenhof	22 February 1993
 *		Fix typo in 040!
 *	043	Dave Butenhof	 1 March 1993
 *		Just cleaning up -- remove cma__c_hexchars (a bad idea that's
 *		no longer used anyway).
 *	044	Dave Butenhof	 5 May 1993
 *		Integrate invariant QUIPU logical analyzer support
 *	045	Dave Butenhof	12 May 1993
 *		Add trace class for zombie-related magic
 *	046	Dave Butenhof	21 May 1993
 *		Integrate changes to permanent QUIPU support.
 */

#ifndef CMA_DEFS
#define CMA_DEFS

/*
 *  INCLUDE FILES
 */

#ifdef _CMA_QUIPU_
/*
 * Currently, this ONLY works for DEC OSF/1 AXP!!!
 */
# ifdef __LANGUAGE_ASSEMBLY__
   .extern cma__g_quipu
#  define LOG_CMA(_v_) \
    or $31, $0, $24; \
    call_pal PAL_rduniq; \
    ldq_u $22, 16($0); \
    extwl $22, $0, $22; \
    sll $22, 0xc, $22; \
    or $22, _v_, $22; \
    ldq $23, cma__g_quipu; \
    stl $22, 0($23); \
    or $31, $24, $0; \
    mb
# else
   extern int *cma__g_quipu;
#  define LOG_CMA(v) \
     (*cma__g_quipu = (v | (cma__rduniq()->sequence << 12)),asm("mb"))
# endif
# include <cma_quipu.h>
#endif

#if !defined(__LANGUAGE_ASSEMBLY__) && !defined(LANGUAGE_ASSEMBLY)

# include <cma_queue.h>
# include <cma_host.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * Enumerated type for objects (only defined as constants since ENUMS can't
 * be stored in a word).
 */
# define cma__c_obj_attr 	1
# define cma__c_obj_cv		2
# define cma__c_obj_mutex	3
# define cma__c_obj_tcb		4
# define cma__c_obj_stack	5
/*
 * Define the number of object classes.  Note that it's actually one HIGHER
 * than the maximum object number... this allows an array to be indexed by
 * the actual type constants (which start at 1).  Too bad C doesn't allow
 * non-0-based array indices.
 */
# define cma__c_obj_num		6

# if _CMA_VENDOR_ == _CMA__HP
#  define cma__c_page_size	NBPG	/* Page size in bytes */
# else
#  define cma__c_page_size	512	/* Page size in bytes */
# endif

/*
 * Private scheduling policy
 */
# define cma_c_prio_idle_min     0
# define cma_c_prio_idle_mid     0
# define cma_c_prio_idle_max     0
# define cma_c_sched_idle	 (cma_c_sched_ada_rtb + 1)

/*
 * This macro computes the base of a structure, given a pointer to some field
 * within the structure, the name of the field, and the type of the
 * structure.  For example, it can be used to find the beginning of a TCB
 * structure when a TCB is removed from a timer queue.
 */
# define cma__base(pointer, field, type) \
    (type  *)((char *)(pointer) - (char *)(&(((type *)0)->field)))

/*
 * Copy N bytes of memory from one address to another
 */
# define cma__copy(src, dst, cnt) cma__memcpy ((char *)(dst), (char *)(src), (cnt))

/*
 * Zero a given data structure
 */
# define cma__zero(p)	cma__memset ((char *)(p), 0, sizeof (*(p)))

/*
 * Set the name of an object
 */
# define cma__obj_set_name(o,string) (((cma__t_object *)(o))->name = (string))
# define cma__obj_set_owner(o,value) (((cma__t_object *)(o))->owner = (value))

/*
 * Clear the name of an object
 */
# define cma__obj_clear_name(o) \
    (((cma__t_object *)(o))->name = (cma__t_string)cma_c_null_ptr)

/*
 * Test whether name is null
 */
# define cma__obj_null_name(o) \
    (((cma__t_object *)(o))->name == (cma__t_string)cma_c_null_ptr)

/*
 * Round first value (a) upto a value fully divisible by second value (b).
 */
# define cma__roundup(a,b)   ((((a) + ((b) - 1))/(b)) * (b))

/*
 * Return the greater of (a) and (b).
 */
# define cma__max(a,b) (((a) > (b)) ? (a) : (b))

/*
 * Return the lesser of (a) and (b).
 */
# define cma__min(a,b) (((a) < (b)) ? (a) : (b))

/*
 * Define simple spinlock based on test-and-set from cma_host.h
 */
# if _CMA_UNIPROCESSOR_
#  define cma__spinlock(lock) while (cma__test_and_set (lock)) cma_yield ()
# else
#  if _CMA_SPINLOCKYIELD_ > 0
#   define cma__spinlock(lock) {int __limit__ = _CMA_SPINLOCKYIELD_; \
	while (cma__test_and_set (lock)) { \
	    if (__limit__ <= 0) {swtch_pri (0); \
		__limit__ = _CMA_SPINLOCKYIELD_;} \
	    else --__limit__; }}
#  else
#   define cma__spinlock(lock) while (cma__test_and_set (lock)) {swtch_pri (0);}
#  endif
# endif
# define cma__tryspinlock(lock) (!cma__test_and_set (lock))
# define cma__spinunlock(lock)	cma__tac_clear (lock)

# if _CMA_MP_HARDWARE_
#  define cma__mp_barrier()	cma__memory_barrier ()
# else
#  define cma__mp_barrier()
# endif

#endif					/* __LANGUAGE_ASSEM__ */

/*
 * Print trace message (if CMA_TRACE environment variable is defined to a
 * filepath, and if DECthreads is built without NDEBUG symbol).
 *
 * By convention, begin the trace message with the function name (minus the
 * "cma__" prefix) in parens. For example,
 *
 * 	cma__trace ((cma__c_trc_vp, "(vp_create) created vp %d", vpid->vp));
 */
#define cma__c_trc_assem	0x00000001	/* Assembly code trace */
#define cma__c_trc_attr		0x00000002	/* Attributes changes */
#define cma__c_trc_bug		0x00000004	/* More info before bugcheck */
#define cma__c_trc_cache	0x00000008	/* Object cache changes */
#define cma__c_trc_deb		0x00000010	/* Debug operations */
#define cma__c_trc_disp		0x00000020	/* Dispatcher operation */
#define cma__c_trc_io		0x00000040	/* I/O wrappers */
#define cma__c_trc_init		0x00000080	/* Initialization */
#define cma__c_trc_null		0x00000100	/* Null thread */
#define cma__c_trc_obj		0x00000200	/* Object creation/deletion */
#define cma__c_trc_sched	0x00000400	/* Scheduling changes */
#define cma__c_trc_stack	0x00000800	/* Trace stacks */
#define cma__c_trc_sync		0x00001000	/* Synchronization functions */
#define cma__c_trc_time		0x00002000	/* Timer operations */
#define cma__c_trc_tmp		0x00004000	/* Temporary debug traces */
#define cma__c_trc_vm		0x00008000	/* Virtual memory */
#define cma__c_trc_vp		0x00010000	/* Virtual Processors */
#define cma__c_trc_wrp		0x00020000	/* Assorted wrappers */
#define cma__c_trc_zombie	0x00040000	/* Zombie magic */
#define cma__c_trc_manager	0x00080000	/* Manager thread */

#if !defined(__LANGUAGE_ASSEMBLY__) && !defined(LANGUAGE_ASSEMBLY)

# ifndef NDEBUG
#  define cma__trace(args)		cma__trace_print args
# else
#  define cma__trace(args)
# endif

/*
 * reinit routine constants (flags).
 */
# define cma__c_reinit_prefork_lock	0
# define cma__c_reinit_postfork_unlock	1
# define cma__c_reinit_postfork_clear	2

/*
 * TYPEDEFS
 */

typedef char *		cma__t_string;
typedef unsigned short	cma__t_short;

/*
 * Common header for all CMA objects
 *
 * NOTE for VMS support: This structure affects the positioning of the
 * standard prolog at the front of all TCBs. The VMS Debugger integration
 * requires that certain information be present at 32 byte offset in the
 * thread control structure; if changes are made here, compensating changes
 * must be made in cma__t_tcb_pad in file cma_tcb_def.h, and the maximum size
 * this structure can ever become is 32 bytes. This restriction affects VMS
 * only.
 */
typedef struct CMA__T_OBJECT {
    cma__t_queue	queue;		/* Queue element MUST BE FIRST */
    cma__t_short	sequence;	/* Sequence number */
    cma__t_short	type;		/* Type of object */
    cma_t_natural	revision;	/* Revision count of attr. obj */
    cma__t_string	name;		/* Name of object for debugging */
    cma_t_integer	owner;		/* 'owner' field for debugging */
    } cma__t_object;

/*
 * This is included at the end because it's just an "implicit include" for
 * modules including cma_defs.h, and it depends on definitions that are
 * made here.
 */
# include <cma_assert.h>

/*
 * INTERNAL INTERFACES
 */

# ifndef NDEBUG
extern void
cma__trace_print _CMA_PROTOTYPE_ ((
	int	type,
	char	*reason,
	...));
# endif

# if _CMA_OS_ == _CMA__VMS
extern cma_t_integer
cma__int_fprintf _CMA_PROTOTYPE_ ((
	cma_t_address 	file,
	char		*format,
	...));

extern cma_t_integer
cma__int_printf _CMA_PROTOTYPE_ ((
	char	*format,
	...));

extern cma_t_integer
cma__int_sprintf _CMA_PROTOTYPE_ ((
	char	*dst,
	char	*format,
	...));

extern cma_t_integer
cma__int_vfprintf _CMA_PROTOTYPE_ ((
	cma_t_address 	file,
	char		*format,
	cma_t_address	args));

extern cma_t_integer
cma__int_vprintf _CMA_PROTOTYPE_ ((
	char		*format,
	cma_t_address	args));

extern cma_t_integer
cma__int_vsprintf _CMA_PROTOTYPE_ ((
	char		*dst,
	char		*format,
	cma_t_address	args));
# else
#  define cma__int_fprintf	fprintf
#  define cma__int_printf	printf
#  define cma__int_sprintf	sprintf
#  define cma__int_vfprintf	vfprintf
#  define cma__int_vprintf	vprintf
#  define cma__int_vsprintf	vsprintf
# endif

#endif					/* __LANGUAGE_ASSEMBLY__ */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEFS.H */
/*  *36   27-MAY-1993 14:32:16 BUTENHOF "Change QUIPU interface" */
/*  *35   14-MAY-1993 15:55:14 BUTENHOF "Add trace class" */
/*  *34    6-MAY-1993 19:06:52 BUTENHOF "Add permanent QUIPU init support" */
/*  *33    1-MAR-1993 11:10:42 BUTENHOF "Remove hexchars symbol" */
/*  *32   22-FEB-1993 11:39:16 BUTENHOF "Fix typo" */
/*  *31    4-DEC-1992 12:41:35 BUTENHOF "Change OSF/1 yield" */
/*  *30    1-DEC-1992 14:05:19 BUTENHOF "Change sched_idle on OSF/1 RT" */
/*  *29    5-NOV-1992 14:24:44 BUTENHOF "Add cma__mp_barrier macro" */
/*  *28   28-SEP-1992 11:49:09 BUTENHOF "add 'owner' field to header" */
/*  *27   24-SEP-1992 08:56:28 SCALES "Change idle sched. policy constant to keep the others contiguous" */
/*  *26   21-SEP-1992 13:31:31 BUTENHOF "Sequence is unsigned" */
/*  *25   15-SEP-1992 13:49:40 BUTENHOF "Add tryspinlock" */
/*  *24   25-AUG-1992 11:48:11 BUTENHOF "Adjust Mach yield operations" */
/*  *23   21-AUG-1992 13:41:43 BUTENHOF "Limit spinlock by config symbol" */
/*  *22   31-JUL-1992 15:31:44 BUTENHOF "Define cma__c_hexchars" */
/*  *21   13-JUL-1992 17:21:28 KEANE "Fix 64 bit glitch in cma__base" */
/*  *20   13-MAR-1992 14:08:35 BUTENHOF "Add tracing" */
/*  *19   30-JAN-1992 11:55:43 BUTENHOF "Get rid of trace_kernel" */
/*  *18   14-OCT-1991 13:38:21 BUTENHOF "Fix some misleading comments" */
/*  *17   24-SEP-1991 16:26:50 BUTENHOF "Unconditionalize object names" */
/*  *16   25-JUL-1991 13:59:44 BUTENHOF "Remove debug line in previous change" */
/*  *15   25-JUL-1991 13:53:32 BUTENHOF "Use cma__int_*printf functions" */
/*  *14   23-JUL-1991 20:19:58 SCALES "Add cma-printf routine protos" */
/*  *13    1-JUL-1991 16:58:09 SCALES "Put back SCCS header" */
/*  *12   17-JUN-1991 15:45:53 BUTENHOF "Make cma__trace_kernel a block" */
/*  *11   11-JUN-1991 17:16:37 BUTENHOF "cma__trace_kernel should do unset, not exit" */
/*  *10   10-JUN-1991 19:52:40 SCALES "Convert to stream format for ULTRIX build" */
/*  *9    10-JUN-1991 19:20:35 BUTENHOF "Fix the sccs headers" */
/*  *8    10-JUN-1991 18:21:26 SCALES "Add sccs headers for Ultrix" */
/*  *7     5-JUN-1991 16:12:51 CURTIN "fork work" */
/*  *6    14-MAY-1991 13:43:14 BUTENHOF "Integrate changes lost in disk crash" */
/*  *5    10-MAY-1991 17:15:53 BUTENHOF "Add ""assem"" trace class, allow as include" */
/*  *4     2-MAY-1991 13:58:07 BUTENHOF "Fix cma__trace macro" */
/*  *3    12-APR-1991 23:35:26 BUTENHOF "Add simple spinlock macro" */
/*  *2    14-DEC-1990 00:55:30 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:44:33 BUTENHOF "General definitions" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEFS.H */
