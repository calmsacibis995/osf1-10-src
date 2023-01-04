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
 *	@(#)$RCSfile: exc_handling.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/11/23 23:43:03 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads exception services
 *
 *  FILENAME:
 *
 * 	EXC_HANDLING.H
 *
 *  ABSTRACT:
 *
 *	Header file for exception handling in C
 *
 *  AUTHORS:
 *
 *	Eric Roberts
 *	Bob Conti
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	15 March 1989
 *
 *  MODIFIED BY:
 *
 *	Dave Butenhof
 *	Bob Conti
 *	Paul Curtin
 *	Webb Scales
 */


#ifndef EXC_HANDLING
# define EXC_HANDLING	1

#ifdef __cplusplus
    extern "C" {
#endif

/*
 *  INCLUDE FILES
 */

#ifndef CMA_CONFIG
# if defined(vms) || defined(__vms) || defined (VMS) || defined(__VMS) || defined(__vms__)
#  include <cma_config.h>
# else
#  include <dce/cma_config.h>
# endif
#endif

#if ((_CMA_COMPILER_ == _CMA__DECC) || (_CMA_COMPILER_ == _CMA__DECCPLUS)) && _CMA_OS_ == _CMA__VMS
# pragma __extern_model __save
# pragma __extern_model __strict_refdef
#elif _CMA_COMPILER_ == _CMA__VAXC
# pragma nostandard
#endif

/*
 * NOTE: on U*IX systems, these status codes must be kept unique from
 * "Enums".  We do this arbitrarily by setting some high order bits which
 * happen to be the same as we use on VMS. Apollo systems use different
 * error numbering scheme, and override this.
 */
#if _CMA_VENDOR_ == _CMA__APOLLO
# define exc_facility_c	0x03c0000
# define _CMA_STATUS_(val, sev) ((exc_int_t)(exc_facility_c + (val)))
#endif
#if _CMA_OS_ == _CMA__VMS
# define exc_facility_c	00020100000
# define _CMA_STATUS_(val, sev) \
	((exc_int_t)(exc_facility_c | ((val) << 3) | (sev)))
#endif
#ifndef _CMA_STATUS_
# define _CMA_DCE_PREFIX_	0x177db000
# define exc_facility_c		_CMA_DCE_PREFIX_
# define _CMA_STATUS_(val, sev) \
	((exc_int_t)(_CMA_DCE_PREFIX_ | (val)))
#endif

#include <setjmp.h>

/*
 * Define a symbol that specifies whether exception handling should use the
 * standard setjmp() and longjmp() functions, or the alternate _setjmp() and
 * _longjmp().  The latter are faster, as they don't save/restore the signal
 * mask (and therefore require no kernel calls).  However, _setjmp() and
 * _longjmp() are not standard, and therefore may not be available
 * everywhere. Also, there may be some platforms where failing to save signal
 * state could break exception handling. For both reasons, we enable use of
 * the optimized functions only where we know for sure they are both
 * available and appropriate.
 */
#ifndef _CMA_BAR_JMP_
# if (_CMA_VENDOR_ == _CMA__APOLLO) || ((_CMA_VENDOR_ == _CMA__DIGITAL) && (_CMA_OS_ == _CMA__UNIX))
#  define _CMA_BAR_JMP_	1
# endif
# ifndef _CMA_BAR_JMP_
#  define _CMA_BAR_JMP_	0
# endif
#endif

#if _CMA_OS_ == _CMA__VMS
# if defined(_CMA_SUPPRESS_EXTERNALS_) || defined (_CMA_CORE_BUILD_)
#  define cma_save_exc_context	cma__save_exc_context
# else
#  define cma_save_exc_context	cma$$save_exc_context
# endif
# if _CMA_HARDWARE_ == _CMA__VAX
   typedef int cma__t_jmp_buf[14];
   extern int cma_save_exc_context (_CMA_VOLATILE_ int *);
# else
#  include <ints.h>
   typedef uint64 cma__t_jmp_buf[(14+8+3)];
   extern int cma_save_exc_context (_CMA_VOLATILE_ uint64 *);
# endif
# define exc_setjmp(__env)	cma_save_exc_context((__env))
#else
  typedef jmp_buf cma__t_jmp_buf;
# if _CMA_BAR_JMP_
#  if _CMA_OSIMPL_ == _CMA__OS_OSF
    /*
     * OSF/1 already provides prototypes for _setjmp and _longjmp in
     * /usr/include/setjmp.h; the prototypes here should be compatible, so
     * we'll just cast the volatile jump buffer inside the exc_setjmp macro
     * instead of defining an appropriate prototype as we do elsewhere.
     */
    extern int _setjmp _CMA_PROTOTYPE_ ((jmp_buf));
    extern void _longjmp _CMA_PROTOTYPE_ ((jmp_buf, int));
#   define exc_setjmp(__env)		_setjmp ((exc_int_t *)(__env))
#   define exc_longjmp(__env,__val)	_longjmp((exc_int_t *)(__env),(__val))
#  else
    extern int _setjmp _CMA_PROTOTYPE_ ((_CMA_VOLATILE_ int *));
    extern void _longjmp _CMA_PROTOTYPE_ ((_CMA_VOLATILE_ int *, int));
#   define exc_setjmp(__env)		_setjmp ((__env))
#   define exc_longjmp(__env,__val)	_longjmp((__env),(__val))
#  endif
# else
#  define exc_setjmp(__env)		setjmp ((__env))
#  define exc_longjmp(__env,__val)	longjmp((__env),(__val))
# endif
#endif

typedef char *exc_address_t;

/*
 * Use the most efficient code available to determine the address of the
 * current procedure frame on VAX VMS systems (which we need to integrate
 * well with native VAX VMS condition handling).
 *
 * - VAX C under VAX VMS supports instruction "builtins" to access general
 * registers. Since it requires a "#pragma", which some old cpp versions
 * can't parse, it's hidden in a separate file.
 *
 * - GCC supports an "asm" statement that generates inline assembly code.
 *
 * - Otherwise, declare an extern function (part of DECthreads' assembly
 * code) that will return the value.
 */
#if _CMA_OS_ == _CMA__VMS
# if _CMA_HARDWARE_ == _CMA__VAX
#  if _CMA_COMPILER_ == _CMA__VAXC
#   pragma builtins
#   define exc_fetch_fp() ((exc_address_t)_READ_GPR (13))
#  elif _CMA_COMPILER_ == _CMA__GCC
#   define exc_fetch_fp() \
    ({ int frameptr; \
    asm volatile ("movl fp, %0" : "=g" (frameptr)); \
    frameptr; })
#  else
#   if defined(_CMA_SUPPRESS_EXTERNALS_) || defined (_CMA_CORE_BUILD_)
     extern exc_address_t cma__fetch_fp (void);
#    define exc_fetch_fp  cma__fetch_fp
#   else
     extern exc_address_t cma$exc_fetch_fp (void);
#    define exc_fetch_fp  cma$exc_fetch_fp
#   endif
#  endif
# else
#  if defined(_CMA_SUPPRESS_EXTERNALS_) || defined (_CMA_CORE_BUILD_)
    extern exc_address_t cma__fetch_fp (void);
#   define exc_fetch_fp  cma__fetch_fp
#  else
    extern exc_address_t cma$exc_fetch_fp (void);
#   define exc_fetch_fp  cma$exc_fetch_fp
#  endif
# endif
#endif

/*
 * Define all of the status codes used by DECthreads.
 *
 * For VMS, these must remain in synch with the CMA_MESSAGE.GNM message file.
 *
 * These values cannot be altered after they have shipped in some DECthreads
 * release.
 */

/*
 * EXC facility messages
 */
#define exc_s_exception         _CMA_STATUS_(1, 4)
#define exc_s_exccop            _CMA_STATUS_(2, 4)
#define exc_s_uninitexc         _CMA_STATUS_(3, 4)
#define exc_s_unkstatus		_CMA_STATUS_(128, 4)
#define exc_s_exccoplos		_CMA_STATUS_(129, 4)

/*
 * These should be set to match with underlying system exception codes on
 * platforms where that is appropriate (e.g., ss$_ codes on VMS).
 */
#if _CMA_OS_ == _CMA__VMS
/*
 * A few of these codes are somewhat imaginary, since VMS doesn't support
 * condition codes that very closely approximate the sense of some UNIX
 * signals.  SIGTRAP, SIGIOT, and SIGEMT have no clear parallels, and the
 * values chosen are fairly arbitrary.  For two others, we chose what seemed
 * close equivalents: SIGPIPE becomes "no mailbox", and SIGXFSZ becomes "disk
 * quota exceeded".
 */
# define exc_s_illaddr		12	/* ss$_accvio */
# define exc_s_exquota		28	/* ss$_exquota */
# define exc_s_insfmem		292	/* ss$_insfmem */
# define exc_s_nopriv		36	/* ss$_nopriv */
# define exc_s_normal		1	/* ss$_normal */
# define exc_s_illinstr		1084	/* ss$_opcdec */
# define exc_s_resaddr		1100	/* ss$_radrmod */
# define exc_s_privinst		1084	/* ss$_opcdec */
# define exc_s_resoper		1108	/* ss$_roprand */
# define exc_s_SIGTRAP		1044	/* ss$_break */
# define exc_s_SIGIOT		44	/* ss$_abort */
# define exc_s_SIGEMT		1068	/* ss$_compat */
# define exc_s_aritherr		1164	/* ss$_fltovf */
# define exc_s_SIGSYS		20	/* ss$_badparam */
# define exc_s_SIGPIPE		628	/* ss$_nombx */
# define exc_s_excpu		8364	/* ss$_excputim */
# define exc_s_exfilsiz		1004	/* ss$_exdiskquota */
# define exc_s_intovf		1148	/* ss$_intovf */
# define exc_s_intdiv		1156	/* ss$_intdiv */
# define exc_s_fltovf		1164	/* ss$_fltovf */
# define exc_s_fltdiv		1172	/* ss$_fltdiv */
# define exc_s_fltund		1180	/* ss$_fltund */
# define exc_s_decovf		1188	/* ss$_decovf */
# define exc_s_subrng		1196	/* ss$_subrng */
#else
# define exc_s_illaddr		_CMA_STATUS_(5, 4)
# define exc_s_exquota		_CMA_STATUS_(6, 4)
# define exc_s_insfmem		_CMA_STATUS_(7, 4)
# define exc_s_nopriv		_CMA_STATUS_(8, 4)
# define exc_s_normal		_CMA_STATUS_(9, 1)
# define exc_s_illinstr		_CMA_STATUS_(10, 4)
# define exc_s_resaddr		_CMA_STATUS_(11, 4)
# define exc_s_privinst		_CMA_STATUS_(12, 4)
# define exc_s_resoper		_CMA_STATUS_(13, 4)
# define exc_s_SIGTRAP		_CMA_STATUS_(14, 4)
# define exc_s_SIGIOT		_CMA_STATUS_(15, 4)
# define exc_s_SIGEMT		_CMA_STATUS_(16, 4)
# define exc_s_aritherr		_CMA_STATUS_(17, 4)
# define exc_s_SIGSYS		_CMA_STATUS_(18, 4)
# define exc_s_SIGPIPE		_CMA_STATUS_(19, 4)
# define exc_s_excpu		_CMA_STATUS_(20, 4)
# define exc_s_exfilsiz		_CMA_STATUS_(21, 4)
# define exc_s_intovf		_CMA_STATUS_(22, 4)
# define exc_s_intdiv		_CMA_STATUS_(23, 4)
# define exc_s_fltovf		_CMA_STATUS_(24, 4)
# define exc_s_fltdiv		_CMA_STATUS_(25, 4)
# define exc_s_fltund		_CMA_STATUS_(26, 4)
# define exc_s_decovf		_CMA_STATUS_(27, 4)
# define exc_s_subrng		_CMA_STATUS_(28, 4)
#endif

/*
 * Define alias names
 */
#define exc_s_accvio		exc_s_illaddr
#define exc_s_SIGILL		exc_s_illinstr
#define exc_s_SIGFPE		exc_s_aritherr
#define exc_s_SIGBUS		exc_s_illaddr
#define exc_s_SIGSEGV		exc_s_illaddr
#define exc_s_SIGXCPU		exc_s_excpu
#define exc_s_SIGXFSZ		exc_s_exfilsiz

/*
 * DECthreads facility (CMA) messages
 */
#define cma_s_alerted           _CMA_STATUS_(48, 4)
#define cma_s_assertion         _CMA_STATUS_(49, 4)
#define cma_s_badparam          _CMA_STATUS_(50, 4)
#define cma_s_bugcheck          _CMA_STATUS_(51, 4)
#define cma_s_exit_thread       _CMA_STATUS_(52, 4)
#define cma_s_existence         _CMA_STATUS_(53, 4)
#define cma_s_in_use            _CMA_STATUS_(54, 4)
#define cma_s_use_error         _CMA_STATUS_(55, 4)
#define cma_s_wrongmutex	_CMA_STATUS_(56, 4)
#define cma_s_stackovf          _CMA_STATUS_(57, 4)
#define cma_s_nostackmem        _CMA_STATUS_(58, 4)
#define cma_s_notcmastack       _CMA_STATUS_(59, 4)
#define cma_s_timed_out         _CMA_STATUS_(60, 4)
#define cma_s_unimp             _CMA_STATUS_(61, 4)
#define cma_s_inialrpro         _CMA_STATUS_(62, 4)
#define cma_s_defer_q_full      _CMA_STATUS_(63, 4)
#define cma_s_signal_q_full	_CMA_STATUS_(64, 4)
#define cma_s_alert_nesting	_CMA_STATUS_(65, 4)

/*
 * Synonyms for convenience
 */
#define cma_s_normal		exc_s_normal

/*
 * TYPEDEFS
 */

/*
 * Constants for the kind of an exception object.
 *
 * There are *currently* only two kinds.  In the address-kind, the identity
 * of an exception object is its address; in the value-kind, the
 * identity of an exception object is an integer, typically, 
 * a system-defined-status-value. These coded kinds also
 * serve as sentinels to help detect uninitialized exceptions.
 */
typedef enum EXC_KIND_T {
    exc_kind_address_c	= 0x02130455,  
    exc_kind_status_c	= 0x02130456
    }			exc_kind_t;

/*
 * Internal contents of an exception object.
 */
typedef long int exc_int_t;

typedef struct EXC_EXT_T {
    exc_int_t		sentinel;
    exc_int_t		version;
    exc_address_t	extend;
    unsigned int	*args;
    } exc_ext_t;

typedef struct EXC_KIND_V1ADDR_T {
    exc_kind_t		kind;
    exc_address_t	address;
    exc_int_t		filler[6];
    } exc_kind_v1addr_t;

typedef struct EXC_KIND_V1STATUS_T {
    exc_kind_t		kind;
    exc_int_t		status;
    exc_int_t		filler[6];
    } exc_kind_v1status_t;

typedef struct EXC_KIND_ADDRESS_T {
    exc_kind_t		kind;
    exc_address_t	address;
    exc_ext_t		ext;
    } exc_kind_address_t;

typedef struct EXC_KIND_STATUS_T {
    exc_kind_t		kind;
    exc_int_t		status;
    exc_ext_t		ext;
    } exc_kind_status_t;

typedef union EXC_EXCEPTION_T	{
    exc_kind_t		kind;
    exc_kind_v1status_t	v1status;
    exc_kind_v1addr_t	v1address;
    exc_kind_status_t	status;
    exc_kind_address_t	address;
    } EXCEPTION;

/*
 * Constants for the state of handling in the current TRY clause.
 * 
 * The state is "none" when no exception has been raised, "active" when
 * one has been raised but has not yet been caught by a CATCH clause, and
 * "handled" after the exception has been caught by some CATCH clause.
 */
typedef enum EXC_STATE_T {
    exc_active_c	= 0, /* This must be the 0 state, see pop_ctx */
    exc_none_c		= 1,
    exc_handled_c	= 2,
    exc_popped_c	= 3
    }			exc_state_t;

/*
 * Structure of a context block.
 *
 * A context block is allocated in the current stack frame for each
 * TRY clause.  These context blocks are linked to form a stack of
 * all current TRY blocks in the current thread.  Each context block
 * contains a jump buffer for use by setjmp and longjmp.  
 *
 */
#define exc_excargs_c	40

typedef struct EXC_CONTEXT_T {
    cma__t_jmp_buf	jmp;		/* Jump buffer */
    _CMA_VOLATILE_ struct EXC_CONTEXT_T
			*link;		/* Link to context block stack */
    EXCEPTION		cur_exception;	/* Copy of the current exception */
    exc_state_t		exc_state;	/* State of handling for this TRY */
#if _CMA_OS_ == _CMA__VMS
    exc_address_t	current_frame;	/* Address of current stack frame */
# if _CMA_PLATFORM_ == _CMA__VAX_VMS
    exc_address_t	old_handler;	/* Address of previous handler */
# endif
#endif
    exc_int_t		sentinel;	/* Identify as "new" ctx block */
    exc_int_t		version;	/* Client context version */
    unsigned int	exc_args[exc_excargs_c];
    } exc_context_t;

/*
 *  GLOBAL DATA
 */

#if _CMA_OS_ == _CMA__VMS && !defined(_CMA_SUPPRESS_EXTERNALS_) && !defined(_CMA_CORE_BUILD_)
/*
 * On VMS, use the VMS calling standard ("$") interface, to avoid pulling in
 * the open interface image (cma$open_rtl) unless the client code uses it.
 */
extern void cma$exc_push_ctx _CMA_PROTOTYPE_ ((	/* Push a context block */
	_CMA_VOLATILE_	exc_context_t *cb));
extern void cma$exc_pop_ctx _CMA_PROTOTYPE_ ((	/* Pop a context block */
	_CMA_VOLATILE_	exc_context_t *cb));
extern void cma$exc_raise _CMA_PROTOTYPE_ ((	/* Raise an exception */
	EXCEPTION *exc));
extern void cma$exc_raise_status _CMA_PROTOTYPE_ ((	/* Raise a status as exception*/
	exc_int_t	status));
extern void cma$exc_report _CMA_PROTOTYPE_ ((	/* Report an exception */
	EXCEPTION *exc));
# define exc_push_ctx		cma$exc_push_ctx
# define exc_pop_ctx		cma$exc_pop_ctx
# define exc_raise		cma$exc_raise
# define exc_raise_status	cma$exc_raise_status
# define exc_report		cma$exc_report
#else
extern void exc_push_ctx _CMA_PROTOTYPE_ ((	/* Push a context block */
	_CMA_VOLATILE_	exc_context_t *cb));
extern void exc_pop_ctx _CMA_PROTOTYPE_ ((	/* Pop a context block */
	_CMA_VOLATILE_	exc_context_t *cb));
extern void exc_raise _CMA_PROTOTYPE_ ((	/* Raise an exception */
	EXCEPTION *exc));
extern void exc_raise_status _CMA_PROTOTYPE_ ((	/* Raise a status as exception*/
	exc_int_t	status));
extern void exc_report _CMA_PROTOTYPE_ ((	/* Report an exception */
	EXCEPTION *exc));
#endif

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
# if defined(_CMA_SUPPRESS_EXTERNALS_)
   extern int  exc_handler (/* sargs, margs*/);	/* System condition handler */
# else
   extern int  cma$exc_handler (/* sargs, margs*/);	/* System condition handler */
#  define exc_handler	cma$exc_handler
# endif
#endif

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
# if defined(_CMA_SUPPRESS_EXTERNALS_) || defined (_CMA_CORE_BUILD_)
    extern int  exc_handler (/* sargs, margs*/);	/* System condition handler */
# else
   extern int  cma$exc_handler (/* sargs, margs*/);	/* System condition handler */
#  define exc_handler	cma$exc_handler
# endif
#endif

/*
 * Define the exception variables
 *
 * NOTE: it does not make sense to turn all DECthreads status codes into
 * exceptions as some are never raised.  Those are:
 * 
 *	cma_s_normal	-- never signalled
 *	cma_s_exception	-- internal to the implementation of exceptions
 * 	cma_s_exccop	-- internal to the implementation of exceptions
 *	cma_s_timed_out -- returned as status value from timed condition wait
 */

#if _CMA_OS_ == _CMA__VMS && !defined (_CMA_CORE_BUILD_)
# if defined (__STDC__) || _CMA_COMPILER_ == _CMA__DECCPLUS
#  define _CMA_EXCNAME(name)	cma$e_##name
#  define _CMA_CMANAME(name)	cma$e_##name
# else
#  define _CMA_EXCNAME(name)	cma$e_/**/name
#  define _CMA_CMANAME(name)	cma$e_/**/name
# endif
#else
# if defined (__STDC__) || _CMA_COMPILER_ == _CMA__DECCPLUS
#  define _CMA_EXCNAME(name)	exc_e_##name
#  define _CMA_CMANAME(name)	cma_e_##name
# else
#  define _CMA_EXCNAME(name)	exc_e_/**/name
#  define _CMA_CMANAME(name)	cma_e_/**/name
# endif
#endif

#if !defined (_EXC_NO_EXCEPTIONS_) && !defined (_CMA_SUPPRESS_EXTERNALS_)
_CMA_IMPORT_ EXCEPTION
    _CMA_EXCNAME (uninitexc),
    _CMA_EXCNAME (illaddr),
    _CMA_EXCNAME (exquota),
    _CMA_EXCNAME (insfmem),
    _CMA_EXCNAME (nopriv),
    _CMA_EXCNAME (illinstr),
    _CMA_EXCNAME (resaddr),
    _CMA_EXCNAME (privinst),
    _CMA_EXCNAME (resoper),
    _CMA_EXCNAME (SIGTRAP),
    _CMA_EXCNAME (SIGIOT),
    _CMA_EXCNAME (SIGEMT),
    _CMA_EXCNAME (aritherr),
    _CMA_EXCNAME (SIGSYS),
    _CMA_EXCNAME (SIGPIPE),
    _CMA_EXCNAME (excpu),
    _CMA_EXCNAME (exfilsiz),
    _CMA_EXCNAME (intovf),
    _CMA_EXCNAME (intdiv),
    _CMA_EXCNAME (fltovf),
    _CMA_EXCNAME (fltdiv),
    _CMA_EXCNAME (fltund),
    _CMA_EXCNAME (decovf),
    _CMA_EXCNAME (subrng),
    _CMA_CMANAME (alerted),
    _CMA_CMANAME (assertion),
    _CMA_CMANAME (badparam),
    _CMA_CMANAME (bugcheck),
    _CMA_CMANAME (exit_thread),
    _CMA_CMANAME (existence),
    _CMA_CMANAME (in_use),
    _CMA_CMANAME (use_error),
    _CMA_CMANAME (wrongmutex),
    _CMA_CMANAME (stackovf),
    _CMA_CMANAME (nostackmem),
    _CMA_CMANAME (notcmastack),
    _CMA_CMANAME (unimp),
    _CMA_CMANAME (inialrpro),
    _CMA_CMANAME (defer_q_full),
    _CMA_CMANAME (signal_q_full),
    _CMA_CMANAME (alert_nesting);

#if _CMA_OS_ == _CMA__VMS && !defined (_CMA_CORE_BUILD_)
    /*
     * Define exc_e_ or cma_e_ aliases for the cma$e_ names. The purpose of
     * all this is to use the "VMS calling standard" names (cma$rtl)
     * regardless of whether the client code uses the VMS or portable
     * DECthreads interfaces, so that use of exceptions never requires the
     * cma$open_rtl image.
     */
# define exc_e_uninitexc	cma$e_uninitexc
# define exc_e_illaddr		cma$e_illaddr
# define exc_e_exquota		cma$e_exquota
# define exc_e_insfmem		cma$e_insfmem
# define exc_e_nopriv		cma$e_nopriv
# define exc_e_illinstr		cma$e_illinstr
# define exc_e_resaddr		cma$e_resaddr
# define exc_e_privinst		cma$e_privinst
# define exc_e_resoper		cma$e_resoper
# define exc_e_SIGTRAP		cma$e_SIGTRAP
# define exc_e_SIGIOT		cma$e_SIGIOT
# define exc_e_SIGEMT		cma$e_SIGEMT
# define exc_e_aritherr		cma$e_aritherr
# define exc_e_SIGSYS		cma$e_SIGSYS
# define exc_e_SIGPIPE		cma$e_SIGPIPE
# define exc_e_excpu		cma$e_excpu
# define exc_e_exfilsiz		cma$e_exfilsiz
# define exc_e_intovf		cma$e_intovf
# define exc_e_intdiv		cma$e_intdiv
# define exc_e_fltovf		cma$e_fltovf
# define exc_e_fltdiv		cma$e_fltdiv
# define exc_e_fltund		cma$e_fltund
# define exc_e_decovf		cma$e_decovf
# define exc_e_subrng		cma$e_subrng
# define cma_e_alerted		cma$e_alerted
# define cma_e_assertion	cma$e_assertion
# define cma_e_badparam		cma$e_badparam
# define cma_e_bugcheck		cma$e_bugcheck
# define cma_e_exit_thread	cma$e_exit_thread
# define cma_e_existence	cma$e_existence
# define cma_e_in_use		cma$e_in_use
# define cma_e_use_error	cma$e_use_error
# define cma_e_wrongmutex	cma$e_wrongmutex
# define cma_e_stackovf		cma$e_stackovf
# define cma_e_nostackmem	cma$e_nostackmem
# define cma_e_notcmastack	cma$e_notcmastack
# define cma_e_unimp		cma$e_unimp
# define cma_e_inialrpro	cma$e_inialrpro
# define cma_e_defer_q_full	cma$e_defer_q_full
# define cma_e_signal_q_full	cma$e_signal_q_full
# define cma_e_alert_nesting	cma$e_alert_nesting
#endif

/*
 * Define aliased exceptions
 */
# define exc_e_accvio		_CMA_EXCNAME (illaddr)
# define exc_e_SIGILL		_CMA_EXCNAME (illinstr)
# define exc_e_SIGFPE		_CMA_EXCNAME (aritherr)
# define exc_e_SIGBUS		_CMA_EXCNAME (illaddr)
# define exc_e_SIGSEGV		_CMA_EXCNAME (illaddr)
# define exc_e_SIGXCPU		_CMA_EXCNAME (excpu)
# define exc_e_SIGXFSZ		_CMA_EXCNAME (exfilsiz)

/*
 * The following are pthread exception names.
 */

# define exc_uninitexc_e	_CMA_EXCNAME (uninitexc)
# define exc_illaddr_e		_CMA_EXCNAME (illaddr)
# define exc_exquota_e		_CMA_EXCNAME (exquota)
# define exc_insfmem_e		_CMA_EXCNAME (insfmem)
# define exc_nopriv_e		_CMA_EXCNAME (nopriv)
# define exc_illinstr_e		_CMA_EXCNAME (illinstr)
# define exc_resaddr_e		_CMA_EXCNAME (resaddr)
# define exc_privinst_e		_CMA_EXCNAME (privinst)
# define exc_resoper_e		_CMA_EXCNAME (resoper)
# define exc_SIGTRAP_e		_CMA_EXCNAME (SIGTRAP)
# define exc_SIGIOT_e		_CMA_EXCNAME (SIGIOT)
# define exc_SIGEMT_e		_CMA_EXCNAME (SIGEMT)
# define exc_aritherr_e		_CMA_EXCNAME (aritherr)
# define exc_SIGSYS_e		_CMA_EXCNAME (SIGSYS)
# define exc_SIGPIPE_e		_CMA_EXCNAME (SIGPIPE)
# define exc_excpu_e		_CMA_EXCNAME (excpu)
# define exc_exfilsiz_e		_CMA_EXCNAME (exfilsiz)
# define exc_intovf_e		_CMA_EXCNAME (intovf)
# define exc_intdiv_e		_CMA_EXCNAME (intdiv)
# define exc_fltovf_e		_CMA_EXCNAME (fltovf)
# define exc_fltdiv_e		_CMA_EXCNAME (fltdiv)
# define exc_fltund_e		_CMA_EXCNAME (fltund)
# define exc_decovf_e		_CMA_EXCNAME (decovf)
# define exc_subrng_e		_CMA_EXCNAME (subrng)

# define pthread_cancel_e	_CMA_CMANAME (alerted)
# define pthread_assertion_e	_CMA_CMANAME (assertion)
# define pthread_badparam_e	_CMA_CMANAME (badparam)
# define pthread_bugcheck_e	_CMA_CMANAME (bugcheck)
# define pthread_exit_thread_e	_CMA_CMANAME (exit_thread)
# define pthread_existence_e	_CMA_CMANAME (existence)
# define pthread_in_use_e	_CMA_CMANAME (in_use)
# define pthread_use_error_e	_CMA_CMANAME (use_error)
# define pthread_wrongmutex_e	_CMA_CMANAME (wrongmutex)
# define pthread_stackovf_e	_CMA_CMANAME (stackovf)
# define pthread_nostackmem_e	_CMA_CMANAME (nostackmem)
# define pthread_notstack_e	_CMA_CMANAME (notcmastack)
# define pthread_unimp_e	_CMA_CMANAME (unimp)
# define pthread_inialrpro_e	_CMA_CMANAME (inialrpro)
# define pthread_defer_q_full_e	_CMA_CMANAME (defer_q_full)
# define pthread_signal_q_full_e _CMA_CMANAME (signal_q_full)

# define exc_accvio_e		_CMA_EXCNAME (accvio)
# define exc_SIGILL_e		_CMA_EXCNAME (SIGILL)
# define exc_SIGFPE_e		_CMA_EXCNAME (SIGFPE)
# define exc_SIGBUS_e		_CMA_EXCNAME (SIGBUS)
# define exc_SIGSEGV_e		_CMA_EXCNAME (SIGSEGV)
# define exc_SIGXCPU_e		_CMA_EXCNAME (SIGXCPU)
# define exc_SIGXFSZ_e		_CMA_EXCNAME (SIGXFSZ)
#endif

/*
 * CONSTANTS AND MACROS
 */

/*
 * This constant helps to identify a context block or exception created with
 * DECthreads BL9 or later; the new structures include a version field to
 * better manage future changes.
 */
#define exc_newexc_c	0x45586732	/* Identify ctx block with version */

/*
 * Define a version constant to be put into exception structures.
 */
#define exc_v2exc_c	2

/*
 * Define "keyword" to initialize an exception. Note: all exceptions *must*
 * be initialized using this macro.
 */
#define EXCEPTION_INIT(e)   (	\
    (e).address.address = (exc_address_t)&(e),	\
    (e).address.kind = exc_kind_address_c, \
    (e).address.ext.sentinel = exc_newexc_c, \
    (e).address.ext.version = exc_v2exc_c, \
    (e).address.ext.extend = (exc_address_t)0, \
    (e).address.ext.args = (unsigned int *)0)

/*
 * Define "routine" to equivalence an exception to an integer
 * (typically a system-defined status value).
 */
#define exc_set_status(e,s) ( \
    (e)->status.status = (s), \
    (e)->status.kind = exc_kind_status_c)

/*
 * Define "routine" to return the status of an exception. Returns 0 if status
 * kind (and value of status in *s), or -1 if not status kind.
 */
#define exc_get_status(e,s) ( \
    (e)->kind == exc_kind_status_c ? \
	(*(s) = (e)->status.status, 0) : \
	-1)

/*
 * Define "routine" to determine if two exceptions match.
 */
#define exc_matches(e1,e2) \
    ((e1)->kind == (e2)->kind \
    && (e1)->address.address == (e2)->address.address)

/*
 * Define "statement" for clients to use to raise an exception.
 */
#define RAISE(e) exc_raise(&(e))

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
/*
 * For VAX VMS, try to integrate peacefully with native VMS condition
 * handling. Save the previous handler for the frame, and restore it on
 * ENDTRY. The DECthreads condition handler will call the saved handler
 * before resignalling a condition that we don't want to handle, unless
 * it is the DECthreads condition handler (to avoid infinite recursion).
 */
# define exc_establish(_exc_ctx_) ( \
        (_exc_ctx_)->current_frame = ((exc_address_t)exc_fetch_fp()), \
	(_exc_ctx_)->old_handler = \
		*((exc_address_t *)(_exc_ctx_)->current_frame), \
        *(exc_address_t *)(_exc_ctx_)->current_frame = \
		((exc_address_t)exc_handler))

# define exc_unestablish(_exc_ctx_) \
	*(exc_address_t *)(_exc_ctx_)->current_frame = (_exc_ctx_)->old_handler;
#else
# if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
    /*
     * Workaround: early Alpha DEC C baselevels require including the header
     * file vaxcshr.h to direct C RTL calls to the proper DEC C entry points.
     * This header erroneously defines "vaxc$establish" to "decc$establish",
     * which doesn't exist and isn't properly translated into defining a
     * condition handler in the procedure descriptor. So defeat the #define
     * manually.
     */
#  ifdef vaxc$establish
#   undef vaxc$establish
#  endif
#  define exc_establish(_exc_ctx_) \
	(_exc_ctx_)->current_frame = ((exc_address_t)exc_fetch_fp()); \
	lib$establish (exc_handler);
#  define exc_unestablish(_exc_ctx_)
# else
#  define exc_establish(_exc_ctx_)
#  define exc_unestablish(_exc_ctx_)
# endif
#endif

/*
 * Constants to define versions of the context block:
 */
#define exc_v2ctx_c	2

/* 
 * Start a new TRY block, which may contain exception handlers
 * 
 *   Allocate a context block on the stack to remember the current
 *   exception. Push it on the context block stack.  Initialize
 *   this context block to indicate that no exception is active. Do a SETJMP
 *   to snapshot this environment (or return to it).  Then, start
 *   a block of statements to be guarded by the TRY clause.
 *   This block will be ended by one of the following: a CATCH, CATCH_ALL,
 *   or the ENDTRY macros.
 */
#define TRY \
    { \
	_CMA_VOLATILE_ exc_context_t exc_ctx; \
	exc_ctx.sentinel = exc_newexc_c; \
	exc_ctx.version = exc_v2ctx_c; \
	exc_ctx.exc_args[0] = 0; \
	exc_push_ctx (&exc_ctx);\
	exc_establish (&exc_ctx);\
        if (!exc_setjmp (exc_ctx.jmp)) {
/*		{ user's block of code goes here } 	*/

/* 
 * Define an CATCH(e) clause (or exception handler).
 *
 *   First, end the prior block.  Then, check if the current exception
 *   matches what the user is trying to catch with the CATCH clause.
 *   If there is a match, a variable is declared to support lexical
 *   nesting of RERAISE statements, and the state of the current
 *   exception is changed to "handled".
 */
#define CATCH(e) \
            } \
            else if (exc_matches(&exc_ctx.cur_exception, &(e))) { \
		EXCEPTION *THIS_CATCH = (EXCEPTION *)&exc_ctx.cur_exception;\
		exc_ctx.exc_state = exc_handled_c;
/*		{ user's block of code goes here } 	*/

/* 
 * Define an CATCH_ALL clause (or "catchall" handler).
 *
 *   First, end the prior block.  Then, unconditionally,
 *   let execution enter into the catchall code.  As with a normal
 *   catch, a variable is declared to support lexical
 *   nesting of RERAISE statements, and the state of the current
 *   exception is changed to "handled".
 */
#define CATCH_ALL \
            } \
            else { \
		EXCEPTION *THIS_CATCH = (EXCEPTION *)&exc_ctx.cur_exception;\
		exc_ctx.exc_state = exc_handled_c;
/*		{ user's block of code goes here } 	*/

/* 
 * Define a RERAISE statement.
 * 
 *   This "statement" is valid only if lexically nested in
 *   a CATCH or CATCH_ALL clause. Reraise the current lexically visible 
 *   exception.
 */
#define RERAISE exc_raise(THIS_CATCH)

/* 
 * Define a FINALLY clause
 *
 *   This "keyword" starts a FINALLY clause.  It must appear before
 *   an ENDTRY.  A FINALLY clause will be entered after normal exit
 *   of the TRY block, or if an unhandled exception tries to propagate
 *   out of the TRY block.  
 *
 *   Unlike Modula 3's TRY clause, we do not expend overhead trying to
 *   enforce that FINALLY be mutually exclusive with CATCH clauses.  Currently, 
 *   if they are used together, then control will drop into a FINALLY clause 
 *   under the following conditions:
 *	o normal exit from TRY, 
 *	o an exception is raised and no CATCH is present (recommended usage) 
 *	o CATCH's are present but none matches the exception.
 *	o CATCH's are present and one matches the exception, but it
 *	  does not raise any exception.  
 *   That is, FINALLY is always entered after TRY unless a CATCH clause raises 
 *   (or re-raises) an exception.
 *
 *			** WARNING **
 *   You should *avoid* using FINALLY with CATCH clauses, that is, use it 
 *   only as TRY {} FINALLY {} ENDTRY.  Source code that combines CATCHes
 *   with FINALLY in the same TRY clause is considered "unsupported"
 *   -- that is, such code may be broken by a future version of this
 *   package.  
 *
 *   There are several reasons this restriction is necessary:
 *	o FINALLY may be added to C++ and its combination with CATCH
 *	  clauses may have different semantics than implemented by these macros.
 *	o The restriction is consistant with the same restriction in Modula 3
 *	o It allows the use of the 2-phase or "debugging" implementation 
 *	  technique of the SRC exception package for these same macros.
 */
#define FINALLY   } \
	if (exc_ctx.exc_state == exc_none_c) \
	    exc_pop_ctx (&exc_ctx);\
	{
/*		{ user's block of code goes here } 	*/

/* 
 * End the whole TRY clause
 */
#define ENDTRY \
	} \
    exc_unestablish (&exc_ctx); \
    if (exc_ctx.exc_state == exc_none_c \
	    || exc_ctx.exc_state == exc_active_c) { \
	exc_pop_ctx (&exc_ctx); \
	} \
    }

#if ((_CMA_COMPILER_ == _CMA__DECC) || (_CMA_COMPILER_ == _CMA__DECCPLUS)) && _CMA_OS_ == _CMA__VMS
# pragma __extern_model __restore
#elif _CMA_COMPILER_ == _CMA__VAXC
# pragma standard
#endif

#ifdef __cplusplus
    }
#endif

#endif
