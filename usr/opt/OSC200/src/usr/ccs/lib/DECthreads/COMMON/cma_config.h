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
 * @(#)$RCSfile: cma_config.h,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/11/23 23:42:44 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Configuration header file to set up control symbols.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	26 January 1990
 *
 *  MODIFIED BY:
 *
 *	Dave Butenhof
 *	Bob Conti
 *	Paul Curtin
 *	Webb Scales
 */

#ifndef CMA_CONFIG
#define CMA_CONFIG

/*
 *  INCLUDE FILES
 */

/*
 * CONSTANTS AND MACROS
 */

/*
 * Quick reference guide to DECthreads configuration symbols:
 *
 *	_USER_THREADS_
 *			If this has been defined, then cma_config.h will
 *			override the default calculation of the
 *			_CMA_UNIPROCESSOR_ config. symbol, forcing use of
 *			user threads even if the system supports kernel
 *			threads.
 *	_CMA_COMPILER_		(CC, VAXC, DECC, CFRONT, GCC,
 *				DECCPLUS, GCPLUS, MSC)
 *			Each compiler has its own quirks, and DECthreads
 *			tries to do as much with each as possible. For
 *			example, it instantiates function prototypes where
 *			possible, even if the compiler is not fully ANSI C
 *			compliant.
 *	_CMA_HARDWARE_		(MIPS, VAX, M68K, HPPA, IBMR2, ALPHA, X86)
 *			The computer on which DECthreads will run.
 *	_CMA_OS_		(UNIX, VMS, NT)
 *			The operating system on which DECthreads will run;
 *			generally, all UNIX-descended systems have certain
 *			similarities which need to be considered as a group.
 *	_CMA_VENDOR_		(APOLLO, DIGITAL, HP, IBM, OSF, SUN, MICROSOFT)
 *			The company supplying the O/S, since each have
 *			slightly different behavior and requirements.
 *	_CMA_PLATFORM_		(MIPS/UNIX, VAX/VMS, VAX/UNIX, M68K/UNIX,
 *				HPPA/UNIX, RS6000/UNIX, ALPHA/UNIX ALPHA/VMS,
 *				X86/NT, ALPHA/NT)
 *			This is a convenience to test both hardware and O/S
 *			variants together.
 *	_CMA_PROTECT_MEMORY_	(0, 1)
 *			This is TRUE (1) if the environment supports
 *			protecting memory pages (DECthreads will set stack
 *			guard pages to no access to trap stack overflows).
 *	_CMA_PROTO_	(1 or undefined)
 *			This is set if the compiler supports function
 *			prototypes (if __STDC__ is true, or if any of a
 *			number of specific compilers is being used). Note:
 *			this must be tested by an #ifdef.
 *	_CMA_OSIMPL_		(AIX, OSF, BSD, SYSV, NT)
 *			The closest ancestor of the operating system, since
 *			each family inherits common characteristics
 *			(particularly in signal behaviors).
 *	_CMA_STACK_TRACE_	(0, 1)
 *			This can be set to cause DECthreads to generate trace
 *			messages when stack management operations are called.
 *			It triggers conditional compilation in cma_stack.c.
 *	_CMA_VOID_		(0, 1)
 *			This is TRUE (1) if the compiler supports the use of
 *			"void *" types. Most __STDC__ compilers do (except
 *			that a version of MIPS C had a bug that prevented its
 *			use). If _CMA_VOID_ is FALSE (0), DECthreads will use
 *			"char *" instead.
 *	_CMA_VOLATILE_		("volatile" or null)
 *			DECthreads requires "volatile" storage class in
 *			several places, including exception handling. Since
 *			some compilers do not support "volatile", DECthreads
 *			uses this symbol in place of the "volatile" keyword.
 *			Note that DECthreads runtimes built with compilers
 *			that do not support volatile may show incorrect
 *			behavior under certain circumstances (particularly
 *			during delivery of exceptions). Because these
 *			situations tend to occur only when errors have
 *			already occurred, it is hoped that the absence of
 *			volatile will not prevent normal DECthreads
 *			operation.
 *	_CMA_UPSTACK_		(0, 1)
 *			This specifies the direction of stack growth on the
 *			target platform. If TRUE (1), then a stack "push"
 *			(e.g., for a function call) causes the stack pointer
 *			value to INCREASE. If FALSE (0), a stack "push"
 *			causes the stack pointer value to DECREASE.
 *	_CMA_KTHREADS_		(NONE, MACH, NTTHREADS)
 *			This specifies the type of kernel threads (if any)
 *			supported by the target. In general, DECthreads will
 *			map user threads onto kernel threads if they are
 *			available.
 *	_CMA_UNIPROCESSOR_	(0, 1)
 *			A value of TRUE (1) means that kernel threads are not
 *			supported and threads are a purely user-mode
 *			abstraction. This allows certain optimizations (for
 *			example, "current thread" can be implemented as a
 *			fetch from a global variable rather than a search for
 *			the stack pointer).
 *	_CMA_MULTIPLEX_		(0, 1)
 *			If TRUE (1) then kernel threads are supported, and
 *			DECthreads additionally will multiplex multiple user
 *			threads on each kernel thread. This balances some of
 *			the advantages and disadvantages of each (user mode
 *			context switching is faster, but use of kernel
 *			threads is more robust since kernel functions
 *			generally block only the calling kernel thread).
 *	_CMA_THREAD_IS_VP_	(0, 1)
 *			A convenience, meaning that kernel threads are being
 *			used, and no multiplexing is being done. It is
 *			computed from _CMA_MULTIPLEX_ and _CMA_KTHREADS_.
 *	_CMA_RT_KTHREAD_	(0, 1)
 *			The kernel thread implementation support POSIX
 *			scheduling policies.
 *	_CMA_THREAD_SYNC_IO_	(0, 1)
 *			If TRUE (1) then the system supports "thread
 *			synchronous I/O", and DECthreads does not have to
 *			emulate it. This is automatically set when DECthreads
 *			is not configured for a uniprocessor OR for
 *			multiplexing on kernel threads, but can be overridden
 *			if the O/S supports thread sync. I/O on user-mode
 *			threads.
 *	_CMA_PER_THD_SYNC_SIGS_	(0, 1)
 *			If TRUE (1) then the system supports per-thread
 *			synchronous signals.  Currently, this is set only for
 *			one-to-one thread mapping on OSF/1 based systems.
 *	_CMA_REENTRANT_CLIB_	(0, 1)
 *			If TRUE (1) then the C library functions are thread
 *			reentrant.
 *	_CMA_MP_HARDWARE_	(0, 1)
 *			Generally, this is the same as "_CMA_KTHREADS_ !=
 *			_CMA__NONE" (kernel threads are assumed to run on
 *			multiprocessor hardware), but should be overridden if
 *			kernel threads are being used and it isn't desirable
 *			to assume MP hardware (setting this for non-MP
 *			hardware may degrade performance).
 *	_CMA_SPINLOOP_		(0, n)
 *			For MP hardware, DECthreads can be configured to
 *			"spin" on a mutex lock for some time before giving up
 *			and blocking the thread. If set to 0, DECthreads will
 *			not spin. Otherwise, _CMA_SPINLOOP_ determines the
 *			number of times DECthreads will try to acquire the
 *			lock before blocking.
 *	_CMA_SPINLOCKYIELD_		(0, n)
 *			The "cma__spinlock" primitive spins for a time and
 *			then yields. On uniprocessors, there's not much point
 *			to spinning, so we want to yield immediately.
 *	_CMA_NO_POSIX_SIGNAL_	(1 or undefined)
 *			If defined, the target platform doesn't support a
 *			POSIX-compatible sigaction() function; DECthreads
 *			will use sigvec() instead. NOTE: must be tested with
 *			#ifdef.
 *	_CMA_IMPORT_, _CMA_EXPORT_
 *			Specifies the keywords used for EXPORTing variables
 *			from DECthreads, or IMPORTing those variables to
 *			client code. For normal UNIX systems, IMPORT is
 *			usually "extern" and EXPORT is usually null. Because
 *			of oddities in the VAX C implementation of extern,
 *			DECthreads uses "globaldef" for EXPORT and
 *			"globalref" for IMPORT.
 *	_CMA_TRACE_KERNEL_	(n or undefined)
 *			If defined, DECthreads will allocate an array of "n"
 *			elements and trace information relating to the use of
 *			the "kernel critical" lock (the user mode scheduling
 *			lock). This can be examined from the debugger, or
 *			printed by cma__format_karray(). It shows the module,
 *			line number, and thread ID of the last n kernel
 *			lock/unlock operations.
 *	_CMA_TRACE_SEM_		(n or undefined)
 *			If defined, DECthreads will allocate an array of "n"
 *			elements and trace information relating to the use of
 *			internal semaphores (the basic blocking mechanism
 *			used for mutexes and condition variables). This can
 *			be examined from the debugger, or printed by
 *			cma__sem_format_array(). It shows the module, line
 *			number, thread ID, and semaphore opcode of the
 *			last n semaphore operations.
 *	_CMA_NOWRAPPERS_	(1 or undefined)
 *			If defined, DECthreads will not use its I/O and C
 *			library wrapper functions. Generally, this is set to
 *		    	1 for building DECthreads, and undefined for building
 *			client code.
 *	_CMA_VSSCANF_		(0, 1)
 *			If set to 1, the DECthreads stdio wrappers will
 *			include the scanf family. The wrappers cannot be
 *			built without the real "v*scanf" function, which don't
 *			exist on most platforms; but we're prepared if we
 *			ever find a platform that does support them. Note that
 *			we assume that using one symbol implicitly assumes
 *			that if one of the v*scanf family is present, they
 *			all will be.
 *	_CMA_CONST_	("const" or "")
 *			if __STDC__ is true, This is set to "const", otherwise
 *			it is an empty definition.  This is used for the 
 *			I/O wrappers to make compliant with POSIX and ANSI C.
 *	_CMA_AUTO_INIT_	(0, 1)
 *			Set if the current system supports automatic
 *			initialization of the DECthreads library (currently
 *			VMS, OSF/1, and HPUX with _HP_LIBC_R set). This
 *			allows the runtime init functions to evaporate
 *			without even checking the init flags.
 *	_CMA_RT4_KTHREAD_	(0, 1)
 *			Set if O/S supports POSIX scheduling policies on
 *			kernel threads.
 *	_CMA_POSIX_SCHED_	(0, 1)
 *			Set if DECthreads should support POSIX scheduling
 *			policy & priorities. Set for all user-mode builds;
 *			clear for kernel-thread builds unless O/S threads can
 *			handle it.
 */

/*
 * NOTE: all configuration symbols are set up such that they can be
 * overridden by a -D switch on the command line if desired (but be sure
 * that you know what you're doing).
 */

/*
 * Name of the platform C compiler
 */
#define _CMA__CC	1
#define _CMA__VAXC	2
#define _CMA__DECC	3
#define _CMA__CFRONT	4
#define _CMA__GCC	5
#define _CMA__DECCPLUS	6
#define _CMA__GCPLUS	7
#define _CMA__MSC	8

/*
 * Test for C++ compilers before C compilers because Glockenspiel C++ also
 * defines symbols for the VAX C compiler and this could be the case for
 * other C++/C compiler combinations
 */
#ifndef _CMA_COMPILER_
# if defined(__cplusplus)		/* test for other C++ compilers first */
#  if defined(__DECCXX)
#   define _CMA_COMPILER_	_CMA__DECCPLUS
#  else
#   define _CMA_COMPILER_	_CMA__CFRONT
#  endif
# elif defined(__decc) || defined(__DECC)
#  define _CMA_COMPILER_	_CMA__DECC
# elif defined(vaxc) || defined(VAXC) || defined(__vaxc) || defined(__VAXC)
#  define _CMA_COMPILER_	_CMA__VAXC
# elif defined(__GNUC__) || defined(__GNUC) || defined(__gnuc)
#  define _CMA_COMPILER_	_CMA__GCC
# elif defined(_MSDOS)
#  define _CMA_COMPILER_        _CMA__MSC
# else
#  define _CMA_COMPILER_	_CMA__CC
# endif
#endif

#if _CMA_COMPILER_ == _CMA__MSC
# define __STDC__ 1
#endif

/*
 * Name of the hardware platform
 */
#define	_CMA__MIPS	1
#define	_CMA__VAX	2
#define _CMA__M68K	3
#define _CMA__HPPA	4
#define _CMA__IBMR2     5
#define _CMA__ALPHA	6
#define _CMA__SPARC	7
#define _CMA__X86	8

#ifndef	_CMA_HARDWARE_
# if defined(vax) || defined (VAX) || defined(__vax) || defined(__VAX)
#  define	_CMA_HARDWARE_	_CMA__VAX
# endif
# if defined(mips) || defined(MIPS) || defined(__mips) || defined(__MIPS)
#  define	_CMA_HARDWARE_	_CMA__MIPS
# endif
# if defined(m68k) || defined(m68000) || defined(_ISP__M68K) || defined(M68000) || defined(mc68000) 
#  define	_CMA_HARDWARE_	_CMA__M68K
# endif
# if defined(hp9000s300) || defined(__hp9000s300)
#  define	_CMA_HARDWARE_	_CMA__M68K
# endif
# if defined(__hppa)
#  define	_CMA_HARDWARE_	_CMA__HPPA
# endif
# if defined(_IBMR2)
#  define _CMA_HARDWARE_	_CMA__IBMR2
# endif
# if defined(__ALPHA) || defined(__alpha)
#  define _CMA_HARDWARE_	_CMA__ALPHA
# endif
# if defined(_M_IX86)
#  define _CMA_HARDWARE_        _CMA__X86
# endif
# ifndef _CMA_HARDWARE_
   !!!Error: _CMA_HARDWARE_ not set
# endif
#endif

/*
 * Name of the software platform
 */
#define	_CMA__UNIX	1
#define	_CMA__VMS	2
#define _CMA__NT	3

#ifndef	_CMA_OS_
# if defined(unix) || defined(__unix) || defined(_AIX) || defined(__OSF__) || defined(__osf__)
#  define	_CMA_OS_	_CMA__UNIX
# endif
# if defined(vms) || defined(__vms) || defined(VMS) || defined(__VMS) || defined(__vms__)
#  define	_CMA_OS_	_CMA__VMS
# endif
# if defined(_MSDOS)
#  define       _CMA_OS_        _CMA__NT
# endif
# ifndef _CMA_OS_
   !!!Error: _CMA_OS_ not set
# endif
#endif

/*
 * Name of the software vendor
 */
#define _CMA__APOLLO	1
#define _CMA__DIGITAL	2
#define _CMA__HP        3
#define _CMA__IBM       4
#define _CMA__OSF	5
#define _CMA__SUN       6
#define _CMA__MICROSOFT 7

#ifndef _CMA_VENDOR_
# ifdef apollo
#  define	_CMA_VENDOR_	_CMA__APOLLO
# endif
# if _CMA_OS_ == _CMA__VMS
#  define	_CMA_VENDOR_	_CMA__DIGITAL
# endif
# if defined(ultrix) || defined(__ULTRIX) || defined (__ultrix)
#  define	_CMA_VENDOR_	_CMA__DIGITAL
# endif
# if defined(__osf__) || defined(__OSF__)
#  define	_CMA_VENDOR_	_CMA__DIGITAL
# endif
# if defined(hpux) || defined(__hpux)
#  define	_CMA_VENDOR_	_CMA__HP
# endif
# ifdef _IBMR2
#  define 	_CMA_VENDOR_	_CMA__IBM
# endif
# ifdef sun
#  define	_CMA_VENDOR_	_CMA__SUN
# endif
# if _CMA_OS_ == _CMA__NT
#  define       _CMA_VENDOR_    _CMA__MICROSOFT
# endif
# ifndef _CMA_VENDOR_
   !!!Error: _CMA_VENDOR_ not set
# endif
#endif

/*
 * Combined platform (OS + hardware)
 */
#define	_CMA__MIPS_UNIX		1
#define	_CMA__VAX_VMS		2
#define _CMA__VAX_UNIX		3
#define _CMA__M68K_UNIX		4
#define _CMA__HPPA_UNIX		5
#define _CMA__IBMR2_UNIX	6
#define _CMA__ALPHA_UNIX	7
#define _CMA__ALPHA_VMS		8
#define _CMA__ALPHA_NT          9
#define _CMA__X86_NT            10

#ifndef	_CMA_PLATFORM_
# if _CMA_OS_ == _CMA__UNIX
#  if _CMA_HARDWARE_ == _CMA__MIPS
#   define _CMA_PLATFORM_	_CMA__MIPS_UNIX
#  endif
#  if _CMA_HARDWARE_ == _CMA__VAX
#   define _CMA_PLATFORM_	_CMA__VAX_UNIX
#  endif
#  if _CMA_HARDWARE_ == _CMA__M68K
#   define _CMA_PLATFORM_	_CMA__M68K_UNIX
#  endif
#  if _CMA_HARDWARE_ == _CMA__HPPA
#   define _CMA_PLATFORM_	_CMA__HPPA_UNIX
#  endif
#  if _CMA_HARDWARE_ == _CMA__IBMR2
#   define _CMA_PLATFORM_	_CMA__IBMR2_UNIX
#  endif
#  if _CMA_HARDWARE_ == _CMA__ALPHA
#   define _CMA_PLATFORM_	_CMA__ALPHA_UNIX
#  endif
# endif
# if _CMA_OS_ == _CMA__VMS
#  if _CMA_HARDWARE_ == _CMA__VAX
#   define _CMA_PLATFORM_	_CMA__VAX_VMS
#  endif
#  if _CMA_HARDWARE_ == _CMA__ALPHA
#   define _CMA_PLATFORM_	_CMA__ALPHA_VMS
#  endif
# endif
# if _CMA_OS_ == _CMA__NT
#  if _CMA_HARDWARE_ == _CMA__X86
#   define _CMA_PLATFORM_       _CMA__X86_NT
#  endif
#  if _CMA_HARDWARE_ == _CMA__ALPHA
#   define _CMA_PLATFORM_       _CMA__ALPHA_NT
#  endif
# endif
# ifndef _CMA_PLATFORM_
   !!!Error: _CMA_PLATFORM_ not set
# endif
#endif

#define _CMA__OS_AIX	1
#define _CMA__OS_OSF	2
#define _CMA__OS_BSD	3
#define _CMA__OS_SYSV	4
#define _CMA__OS_VMS	5
#define _CMA__OS_NT     6

/*
 * MIPS C on DEC OSF/1 sets __osf__ but not __OSF__; but gcc on "raw" OSF/1
 * sets __OSF__ but not __osf__. This little ditty provides a bridge.
 */
#if defined (__OSF__) && !defined (__osf__)
# define __osf__
#endif

#ifndef _CMA_OSIMPL_
# if _CMA_OS_ == _CMA__VMS
#  define _CMA_OSIMPL_		_CMA__OS_VMS
# elif _CMA_OS_ == _CMA__NT
#  define _CMA_OSIMPL_          _CMA__OS_NT
# else
#  if defined (__osf__)
#   define _CMA_OSIMPL_		_CMA__OS_OSF
#  else
#   if _CMA_VENDOR_ == _CMA__IBM
#    define _CMA_OSIMPL_	_CMA__OS_AIX
#   else
#    if _CMA_VENDOR_ == _CMA__SUN
#     define _CMA_OSIMPL_	_CMA__OS_SYSV
#    else
#     define _CMA_OSIMPL_	_CMA__OS_BSD
#    endif
#   endif
#  endif
# endif
#endif

/*
 * Set to 1 if system supports setting memory page protection (see
 * cma_stack.c for use of page protection routines; "cma_vmprot.h" defines
 * interface to generic jacket routines "cma__set_noaccess" and
 * "cma__set_access").
 */
#ifndef _CMA_PROTECT_MEMORY_
# if _CMA_PLATFORM_ == _CMA__VAX_VMS
#  define _CMA_PROTECT_MEMORY_	1
# endif
# if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
#  define _CMA_PROTECT_MEMORY_	1
# endif
# if _CMA_PLATFORM_ == _CMA__MIPS_UNIX
#  define _CMA_PROTECT_MEMORY_  1
# endif
# if _CMA_PLATFORM_ == _CMA__ALPHA_UNIX
#  define _CMA_PROTECT_MEMORY_  1
# endif
# if _CMA_PLATFORM_ == _CMA__VAX_UNIX
#  define _CMA_PROTECT_MEMORY_  1
# endif
# if _CMA_PLATFORM_ == _CMA__X86_NT
#  define _CMA_PROTECT_MEMORY_  0
# endif
# if _CMA_PLATFORM_ == _CMA__ALPHA_NT
#  define _CMA_PROTECT_MEMORY_  0
# endif
#endif

/*
 * This controls whether ANSI C function prototypes are used for CMA
 * interfaces.
 */
#ifndef	_CMA_PROTO_
# ifdef __STDC__
#  define _CMA_PROTO_		1
# endif
# if _CMA_COMPILER_ == _CMA__DECC
#  define _CMA_PROTO_		1
# endif
# if _CMA_COMPILER_ == _CMA__GCC
#  define _CMA_PROTO_		1
# endif
# if _CMA_COMPILER_ == _CMA__DECCPLUS
#  define _CMA_PROTO_		1
# endif
# if _CMA_COMPILER_ == _CMA__GCPLUS
#  define _CMA_PROTO_		1
# endif
# if _CMA_COMPILER_ == _CMA__CFRONT
#  define _CMA_PROTO_		1
# endif
# if _CMA_COMPILER_ == _CMA__VAXC
#  define _CMA_PROTO_		1
# endif
# if _CMA_OSIMPL_ == _CMA__OS_OSF
#  define _CMA_PROTO_		1
# endif
# if _CMA_PLATFORM_ == _CMA__MIPS_UNIX
#  define _CMA_PROTO_		1
# endif
# if _CMA_VENDOR_ == _CMA__APOLLO
#  define _CMA_PROTO_		1
# endif
# if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
#  define _CMA_PROTO_		1
# endif
# if _CMA_PLATFORM_ == _CMA__X86_NT
#  define _CMA_PROTO_           1
# endif
# if _CMA_PLATFORM_ == _CMA__ALPHA_NT
#  define _CMA_PROTO_           1
# endif
/* Otherwise, _CMA_PROTO_ is undefined, which means do not use prototypes. */
#endif

#ifdef _CMA_PROTO_
# define _CMA_PROTOTYPE_(arg)	arg
#else
# define _CMA_PROTOTYPE_(arg)	()
#endif

/*
 * The stack manager module (cma_stack.c) can printf messages which can be
 * useful for debugging (if changes are made to stack management for a
 * platform).
 */
#ifndef _CMA_STACK_TRACE_
# define _CMA_STACK_TRACE_	0	/* DEBUG (report stack management) */
#endif

/*
 * Define whether to use "void *" or "char *" pointers, based on whether the
 * compiler can support them.
 */
#ifndef	_CMA_VOID_
# ifdef __STDC__
#  define _CMA_VOID_		1
# endif
# if _CMA_COMPILER_ == _CMA__VAXC 
#  define _CMA_VOID_	        1
# endif
# if _CMA_COMPILER_ == _CMA__DECCPLUS
#  define _CMA_VOID_		1
# endif
# if _CMA_COMPILER_ == _CMA__GCPLUS
#  define _CMA_VOID_		1
# endif
# if _CMA_COMPILER_ == _CMA__CFRONT
#  define _CMA_VOID_		1
# endif
# if _CMA_COMPILER_ == _CMA__DECC
#  define _CMA_VOID_		1
# endif
# if _CMA_OSIMPL_ == _CMA__OS_OSF
#  define _CMA_VOID_		1
# endif
# if _CMA_VENDOR_ == _CMA__SUN
#  define _CMA_VOID_            1
# endif
# if _CMA_VENDOR_ == _CMA__HP
#  define _CMA_VOID_            1
# endif
# if _CMA_COMPILER_ == _CMA__MSC
#  define _CMA_VOID_            1
# endif
# ifndef _CMA_VOID_
#  define _CMA_VOID_		0
# endif
#endif

/*
 * Certain structures within CMA (especially in exception handling) should be
 * marked "volatile", however some C compilers do not support "volatile" on
 * structures.  Set this to "volatile" unless using such a deficient
 * compiler, in which case it should be "".
 */
#ifndef	_CMA_VOLATILE_
# if _CMA_PLATFORM_ == _CMA__VAX_UNIX
#  if _CMA_COMPILER_ == _CMA__CC
#   define _CMA_VOLATILE_
#  endif
# endif
# if _CMA_VENDOR_ == _CMA__SUN
#  define _CMA_VOLATILE_
# endif
# if _CMA_COMPILER_ == _CMA__CFRONT
#  define _CMA_VOLATILE_
# endif
#ifndef	_CMA_VOLATILE_
#  define _CMA_VOLATILE_  		volatile
# endif
#endif

/*
 * This symbol defines whether stacks grow towards lower addresses or higher
 * addresses (_CMA_UPSTACK_ is defined if the stack grows up).
 */
#ifndef _CMA_UPSTACK_
# if _CMA_HARDWARE_ == _CMA__HPPA
#  define _CMA_UPSTACK_ 1
# endif
#endif

/*
 * If the platform supports kernel threads, then the DECthreads VP layer can
 * provide parallel computation. This symbol defines the variety of kernel
 * threads supported by the platform.
 */
#define _CMA__NONE	0
#define _CMA__MACH	1
#define _CMA__NTTHREADS 2

#ifndef _CMA_KTHREADS_
# if _CMA_OSIMPL_ == _CMA__OS_OSF
#  define _CMA_KTHREADS_	_CMA__MACH
# elif _CMA_OSIMPL_ == _CMA__OS_NT
#  define _CMA_KTHREADS_        _CMA__NTTHREADS
# else
#  define _CMA_KTHREADS_	_CMA__NONE
# endif
#endif

/*
 * This symbol, if defined, provides for some shortcuts that can be made when
 * only uniprocessor hardware is supported (for example, using a fixed entry
 * for "current thread" instead of search stack clusters).
 *
 * If the builder has specifically defined _CMA_UNIPROCESSOR_ to 1, then
 * build a non-kernel-thread version even if the system supports kernel
 * threads, by setting _CMA_KTHREADS_ to _CMA__NONE.
 *
 * Note that if _USER_THREADS_ is set, then _CMA_UNIPROCESSOR_ is explicitly
 * defined to "TRUE" regardless of the platform. That overrides the default
 * user/kernel threads decision, forcing use of user threads. If kernel
 * threads have been determined to be present, they are turned off.
 */
#ifdef _USER_THREADS_
# define _CMA_UNIPROCESSOR_	1
#endif

#ifndef _CMA_UNIPROCESSOR_
# if _CMA_KTHREADS_ == _CMA__NONE
#  define _CMA_UNIPROCESSOR_	1
# else
#  define _CMA_UNIPROCESSOR_	0
# endif
#elif _CMA_UNIPROCESSOR_
# undef _CMA_KTHREADS_
# define _CMA_KTHREADS_		_CMA__NONE
#endif

/*
 * Specify whether this implementation will multiplex on top of VPs
 */
#ifndef _CMA_MULTIPLEX_
# if !_CMA_UNIPROCESSOR_
#  define _CMA_MULTIPLEX_	0	/* No support yet! */
# endif
# ifndef _CMA_MULTIPLEX_
#  define _CMA_MULTIPLEX_	0
# endif
#endif

/*
 * _CMA_THREAD_IS_VP_ is true IFF the platform supports kernel threads and
 * each DECthreads thread is permanently bound to a specific kernel thread
 * throughout its life: in other words, it is a kernel thread platform and
 * threads are not multiplexed in user mode.
 */
#ifndef _CMA_THREAD_IS_VP_
# if _CMA_MULTIPLEX_ || (_CMA_KTHREADS_ == _CMA__NONE)
#  define _CMA_THREAD_IS_VP_	0
# else
#  define _CMA_THREAD_IS_VP_	1
# endif
#endif

/*
 * Set a symbol if we're building a kernel thread version on a system that
 * supports POSIX.4 scheduling semantics for kernel threads.
 */
#ifndef _CMA_RT4_KTHREAD_
# if (_CMA_OSIMPL_ == _CMA__OS_OSF) && (_CMA_VENDOR_ == _CMA__DIGITAL) && _CMA_THREAD_IS_VP_
#  define _CMA_RT4_KTHREAD_	1
# else
#  define _CMA_RT4_KTHREAD_	0
# endif
#endif

/*
 * _CMA_THREAD_SYNC_IO_ is true IFF a blocking I/O function (e.g., read())
 * will block only the DECthreads thread that issued the call. It should
 * generally be false if _CMA_MULTIPLEX_ || _CMA_UNIPROCESSOR_, but can be
 * controlled separately. If _CMA_THREAD_SYNC_IO_ is true, the DECthreads I/O
 * wrapper functions will not be compiled.
 */
#ifndef _CMA_THREAD_SYNC_IO_
# if _CMA_MULTIPLEX_ || _CMA_UNIPROCESSOR_
#  define _CMA_THREAD_SYNC_IO_	0
# else
#  define _CMA_THREAD_SYNC_IO_	1
# endif
#endif

/*
 * _CMA_PER_THD_SYNC_SIGS_ is true IFF the system supports per-thread
 * synchronous signal actions for DECthreads threads. Generally, that means
 * that kernel threads support per-thread sync. signals, and DECthreads is
 * mapped one-to-one on kernel threads (_CMA_THREAD_IS_VP_).
 */
#ifndef _CMA_PER_THD_SYNC_SIGS_
# if _CMA_THREAD_IS_VP_ && (_CMA_OSIMPL_ == _CMA__OS_OSF)
#  define _CMA_PER_THD_SYNC_SIGS_	1
# else
#  define _CMA_PER_THD_SYNC_SIGS_	0
# endif
#endif

/*
 * _CMA_REENTRANT_CLIB_ is true IFF the platform's C library is reentrant.
 * This is generally true on a platform where _CMA_THREAD_IS_VP_ is true, but
 * may also be true for multiplexed threads if the C library is designed
 * properly (the OSF/1 libc_r library and OpenVMS tis facility allow the
 * thread library to provide the correct locks "transparently").
 */
#ifndef _CMA_REENTRANT_CLIB_
# if _CMA_THREAD_IS_VP_
#  define _CMA_REENTRANT_CLIB_	1
# elif defined(_POSIX_REENTRANT_FUNCTIONS) && (_CMA_HARDWARE_ == _CMA__HPPA)
#  define _CMA_REENTRANT_CLIB_	1
# elif (_CMA_COMPILER_ == _CMA__DECC) || (_CMA_COMPILER_ == _CMA__DECCPLUS)
#  define _CMA_REENTRANT_CLIB_	1
# elif _CMA_OS_ == _CMA__NT
#  define _CMA_REENTRANT_CLIB_  1
# else
#  define _CMA_REENTRANT_CLIB_	0
# endif
#endif

/*
 * _CMA_POSIX_SCHED_ is true if not kernel threads, or if it's a specific
 * type of kernel threads that supports POSIX scheduling.
 */
#ifndef _CMA_POSIX_SCHED_
# if _CMA_UNIPROCESSOR_ || _CMA_RT4_KTHREAD_
#  define _CMA_POSIX_SCHED_	1
# else
#  define _CMA_POSIX_SCHED_	0
# endif
#endif

/*
 * _CMA_MP_HARDWARE_ defines whether DECthreads is being built to support
 * actual multiprocessor hardware, not merely kernel threads. In most cases,
 * the important distinction is kernel threads vs. user multiplexing.
 * However, there are some decisions that ought to be based on whether the
 * kernel threads may actually run on different CPUs concurrently; so we
 * might as well have this convenient symbol.
 */
#ifndef _CMA_MP_HARDWARE_
# if _CMA_KTHREADS_ != _CMA__NONE
#  define _CMA_MP_HARDWARE_	0	/* No mP kernel thread configs yet */
# else
#  define _CMA_MP_HARDWARE_	0	/* Assume NO if no kernel threads */
# endif
#endif

/*
 * _CMA_SPINLOOP_ controls the number of times a thread will spin (in a tight
 * loop) attempting to lock a mutex before it gives up and blocks itself. The
 * _CMA_SPINLOCKYIELD_ symbol is similar, but is used for spin locks (and for
 * the kernel lock).
 *
 * On a uniprocessor configuration, these symbols should be defined to zero;
 * spinning won't accomplish anything but to waste the rest of the thread's
 * timeslice. Even when kernel threads are supported on uniprocessor
 * hardware, these symbols still have little value, since the thread may
 * still spin (wasting CPU) until the kernel performs a thread context
 * switch.
 */
#ifndef _CMA_SPINLOOP_
# if _CMA_MP_HARDWARE_
#  define	_CMA_SPINLOOP_		100
# else
#  define	_CMA_SPINLOOP_		0
# endif
#endif

#ifndef _CMA_SPINLOCKYIELD_
# if _CMA_MP_HARDWARE_
#  define	_CMA_SPINLOCKYIELD_	100
# else
#  define	_CMA_SPINLOCKYIELD_	0
# endif
#endif

/*
 * Some UNIX vendors don't yet provide POSIX compatible sigaction().
 * In this case, use sigvec() instead.
 */
#ifndef _CMA_NO_POSIX_SIGNAL_
# if _CMA_VENDOR_ == _CMA__SUN
#  define _CMA_NO_POSIX_SIGNAL_         1
# endif
#endif

/*
 * Define the symbols used to "import" and "export" symbols for the client
 * interface.  Note that these shouldn't be used for symbols shared only
 * between CMA modules; it's for those symbols which are "imported" in the
 * cma.h (or pthread.h) header files, such as cma_c_null,
 * pthread_attr_default, and the exception names.
 *
 * On most platforms (with well-integrated C compilers), "import" should be
 * "extern", and "export" should be "".
 */
#if ((_CMA_COMPILER_ == _CMA__DECC) || (_CMA_COMPILER_ == _CMA__DECCPLUS)) && _CMA_OS_ == _CMA__VMS
# pragma __extern_model __save		/* restored at the end of cma.h */
# pragma __extern_model __strict_refdef
# define _CMA_IMPORT_ extern
# define _CMA_EXPORT_
#elif _CMA_COMPILER_ == _CMA__VAXC
# pragma nostandard			/* restored at the end of cma.h */
# define _CMA_IMPORT_ globalref
# define _CMA_EXPORT_ globaldef
#else
# define _CMA_IMPORT_ extern
# define _CMA_EXPORT_
#endif

/*
 * Some systems provide hooks so that DECthreads can be automatically
 * initialized when a program using it starts -- VMS lib$initialize psects
 * and the OSF/1 _pthread_init call from crt0.o for example. On these
 * systems, checking for initialization at runtime (which is done in pthread
 * "create" functions and cma_init()) is a waste of time. This symbol allows
 * such checks to be evaporated.
 */
#ifndef _CMA_AUTO_INIT_
# if _CMA_OS_ == _CMA__VMS || _CMA_OSIMPL_ == _CMA__OS_OSF || defined(_HP_LIBC_R)
#  define _CMA_AUTO_INIT_	1
# else
#  define _CMA_AUTO_INIT_	0
# endif
#endif

/*
 * The following macros are used with the I/O wrappers.
 * ANSI C defines some parameters using the const modifier. Use this macro
 * to make the function signatures ANSI and POSIX compliant for STDIO and
 * thread I/O. wrappers. Size_t is defined by including stdio.h which is
 * included in cma_stdio.h and cma_stdio.c
 */
#ifdef __STDC__
#define _CMA_CONST_ const
#define	_CMA_SIZE_T size_t
#else
#define _CMA_CONST_
#define _CMA_SIZE_T unsigned int
#endif

/*
 * The DECthreads stdio formatting wrappers (printf & scanf family) depend on
 * the existence of the stdarg variety of those functions (vsprintf &
 * vsscanf), since DECthreads must pass on the client's variable argument
 * list. Most systems have vsprintf, however we haven't seen one with
 * vsscanf, although it seems a logical extension. The scanf family wrappers
 * are coded, but can't be built without vsscanf: for a system which does
 * supply this elusive function, turn on the _CMA_VSSCANF_ config symbol and
 * rebuild DECthreads.
 */
#ifndef _CMA_VSSCANF_
# define _CMA_VSSCANF_	0
#endif

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#endif
