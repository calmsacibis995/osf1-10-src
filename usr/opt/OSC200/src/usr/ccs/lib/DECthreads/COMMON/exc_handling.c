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
 * @(#)$RCSfile: exc_handling.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/05/11 22:03:30 $
 */
/*
 *  FACILITY:
 *
 *	EXC services
 *
 *  FILENAME:
 *
 *	EXC_HANDLING.C
 *
 *  ABSTRACT:
 *
 *	This module implements exception handling for C.
 *
 *
 *  AUTHORS:
 *
 *	Eric Roberts
 *
 *  CREATION DATE:
 *
 *	15 March 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	R. Conti	2 August 1989
 *		Added a faster "non-debugging" implementation.  It is more
 *		efficient in the usual case of no exception being raised.
 *	002	R. Conti	1 December 1989
 *		Replace call to exit with abort. Remove old SRC code. Add
 *		partial support for system-specific status values.
 *	003	R. Conti	6 December 1989
 *		Complete the support for system status codes. Pop the context
 *		block stack here rather than in the client code to save time
 *		and space in the  client code.
 *	004	R. Conti	6 December 1989
 *		Fix bug: forgot to pop exception context at  ENDTRY for
 *		normal fall through.  Added routines to push and pop the
 *		context block in the current thread.
 *	006	R. Conti	8 December 1989
 *		Add cma_exc_raise_status to raise a status directly. Connect
 *		to VMS condition handling.
 *	007	Dave Butenhof	8 December 1989
 *		Implement per-thread exception stack.
 *	008	Dave Butenhof	8 December 1989
 *		Delete internal cma_e_uninitexc.
 *	009	Bob Conti	8 December 1989
 *		Restore internal cma_e_uninitexc. And restore the possibility
 *		of this package being standalone outside of CMA. NOTE: This
 *		module must be able to be used with CMA multithreading,
 *		without CMA multithreading, and by CMA multithreading itself.
 *	010	Dave Butenhof	2 March 1990
 *		Integrate Kevin Ackley's changes for Apollo M68000 port.
 *	011	Webb Scales	23 March 1990
 *		Replace call to C-RTL "abort" with a CMA wrapper.
 *	012	Bob Conti	6 April 1990
 *		Add code to resignal on VMS if the condition is SS$_DEBUG.
 *		CMA's exception handler is not intended to catch  continuable
 *		exceptions.  This fix had to be implemented because SS$_DEBUG
 *		is a continuable condition with a SEVERE severity. must never
 *		be used to handle
 *	013	Dave Butenhof	10 April 1990
 *		Integrate Kevin Ackley's Apollo changes.
 *	014	Dave Butenhof	29 May 1990
 *		Clean up a little by adding definition for sys$putmsg.
 *	015	Webb Scales	15 August 1990
 *		Clean up a little, accomodate new platforms.
 *	016	Dave Butenhof	21 August 1990
 *		Fix typo in conditional for defining cma___print_status_error
 *		(should be if != APOLLO && != VMS, rather than != APOLLO &&
 *		== VMS).
 *	017	Paul Curtin	15 October 1990
 *		changed cma$_ values to cma$s_
 *	018	Dave Butenhof	15 November 1990
 *		Fix cma_exc_report to report address exception properly;
 *		since the exception object may have been copied, it must
 *		print the exc->match.address field, rather than the actual
 *		address of the object.
 *	019	Dave Butenhof	27 December 1990
 *		Rearrange exception names.
 *	020	Dave Butenhof	22 January 1991
 *		More exception name stuff
 *	021	Dave Butenhof	25 March 1991
 *		Bring message text into alignment with VMS message file, and
 *		remove most CMA-specific names.
 *	022	Paul Curtin	8 April 1991
 *		Changed call to cma__abort to cma__abort_process
 *	023	Dave Butenhof	02 May 1991
 *		Use new exc_longjmp macro rather than longjmp() function, so
 *		that exc_handling.h can configure use of longjmp() or
 *		_longjmp() as appropriate to the platform.
 *	024	Paul Curtin	12 May 1991
 *		Converted over to use Unix messages on unix platforms.
 *	025	Dave Butenhof	01 July 1991
 *		Flush stdout before writing error in exc_report()
 *	026	DECthreads team	    22 July 1991
 *		VMS handler now uses cma__restore_exc_context
 *	027	Dave Butenhof, Webb Scales	30 July 1991
 *		Change exc_handler() to call frame's previous handler before
 *		resignalling.
 *	028	Paul Curtin & Webb Scales   6 August 1991
 *		Fix nested TRYs
 *	029	Paul Curtin	20 August 1991
 *		Conditionalized out the include of stdio.h on VMS.
 *	030	Dave Butenhof	18 September 1991
 *		Integrate Apollo CMA5 reverse drop; remove some Apollo PFM
 *		conditionals.
 *	031	Dave Butenhof	30 October 1991
 *		Fix bug in exc_handler; it should unwind to depth-1, unless
 *		depth is 0.
 *	032	Dave Butenhof	06 November 1991
 *		Several uses of _CMA_ config symbols have crept in: fix them.
 *	033	Dave Butenhof	18 November 1991
 *		Fix test for call-through to previous handler (VMS only). It
 *		needs to check address against transfer vector, not internal
 *		function.
 *	034	Paul Curtin	18 November 1991
 *		Changes to support Alpha VMS.
 *	035	Dave Butenhof	20 November 1991
 *		Change format of DCE message codes (all U*IX platforms) to
 *		integrate with DCE message printing.
 *	036	Dave Butenhof	25 November 1991
 *		Conditionalize cma_message.h inclusion (not used on VMS).
 *	037	Dave Butenhof	05 December 1991
 *		Pass controlled arguments to $putmsg to avoid printing stuff
 *		from the exc_report stack frame!
 *	038	Dave Butenhof	16 December 1991
 *		Don't use cma_s_exccop prefix condition for codes that
 *		DECthreads signals; it's just clutter. Also, back off on the
 *		separate "kind" codes for new exceptions; it would break
 *		CATCH clauses in old object modules, which is unacceptable at
 *		this point (use less reliable "sentinel" value, which isn't
 *		allocated in old exceptions, instead).
 *	039	Dave Butenhof	17 December 1991
 *		Another "oops". Remove exc_reraise() function, since it
 *		"reraises" wrong exception if within a TRY nested in a CATCH.
 *	056	Dave Butenhof	18 December 1991
 *		Adjust Alpha mechanism array access.
 *	057	Paul Curtin	20 December 1991
 *		Remove include of starlet.h on VAX/VMS
 *	058	Dave Butenhof	08 January 1992
 *		Prepend exception reports with "Exception: " to indicate that
 *		the report is a DECthreads exception (rather than a DCE
 *		status report). This will affect all messages from
 *		exc_report, rather than just DECthreads codes.
 *	059	Paul Curtin	22 January 1992
 *		Changed a couple of int casts to unsigned int casts.
 *	060	Paul Curtin	06 February 1992
 *		Added individual ifndef checks for certain Alpha hardware
 *		exceptions.
 *	061	Dave Butenhof	07 February 1992
 *		Clean up the exc___resignal macro to restore the mechanism
 *		array depth field on return from user handler.
 *	062	Dave Butenhof	10 February 1992
 *		Remove depth modifications for VMS... Webb's experimentation
 *		indicates that depth really is just distance between
 *		establisher frame and the frame where the exception occurred;
 *		number of "recursive handlers" on top of that is irrelevant.
 *	063	Dave Butenhof	31 March 1992
 *		Implement 'interest' model for exceptions -- exc_raise() will
 *		search the exc context stack for "exc_v3ctx_c" version blocks
 *		that express interest in the exception. Older blocks are
 *		always assumed to express interest, for compatibility.
 *	064	Dave Butenhof	31 March 1992
 *		Separate VMS exception interest (into exc_handler rather than
 *		exc_raise).
 *	065	Dave Butenhof	20 April 1992
 *		The exception interest implementatation imposes too much
 *		overhead at the TRY/CATCH macro level to be acceptable at
 *		this time: so withdraw the feature for now.
 *	066	Webb Scales	15 January 1993
 *		Cache the address of the ctx-stack-top in a local variable to
 *		reduce calls to get-self-tcb.
 *	067	Dave Butenhof	22 March 1993
 *		Fix logic that checks for errno exception in exc_report() [it
 *		makes little-endian 32 bit assumptions].
 *	068	Dave Butenhof	26 April 1993
 *		Remove _EXC_ symbol tests.
 */

/*
 *  INCLUDE FILES
 */

#include <pthread_exc.h>	/* Includes pthread.h (getting cma.h) */
#include <exc_handling.h>
#include <cma_util.h>
#include <cma_init.h>
#if _CMA_OS_ != _CMA__VMS
# include <signal.h>
# include <errno.h>
# include <cma_message.h>
#endif
#undef exc_raise
#undef exc_raise_status
#undef exc_report
#undef exc_push_ctx
#undef exc_pop_ctx
#undef exc_handler

#if _CMA_OS_ == _CMA__UNIX
# include <stdio.h>
# include <cma_signal.h>
#else
# undef NULL
# define NULL (void *) 0
#endif

#include <cma_stack.h>			/* cma__get_self_tcb () */
#include <cma_tcb_defs.h>		/* (cma__t_int_tcb)->exc_stack */

#if _CMA_VENDOR_ == _CMA__APOLLO
# include <apollo/base.h>
# include <apollo/error.h>
#endif

#if _CMA_OS_ == _CMA__VMS
# include <cma_assem.h>
# include <chfdef.h>
# if _CMA_HARDWARE_ == _CMA__VAX
#  define frame_field	chf$l_mch_frame
#  define depth_field	chf$l_mch_depth
#  define savr0_field	chf$l_mch_savr0
# else
#  include <starlet.h>
#  define frame_field	chf$q_mch_frame
#  define depth_field	chf$q_mch_depth
#  define savr0_field	chf$q_mch_savr0
# endif
#endif

/*
 * GLOBAL DATA
 */

#if _CMA_OS_ == _CMA__VMS
# ifndef STS$M_FAC_NO
#  include <stsdef.h>
# endif
# ifndef SS$_NORMAL
#  include <ssdef.h>
# endif
extern int lib$stop ();
extern int lib$callg ();
#endif

#if (_CMA_OS_ == _CMA__UNIX) && (_CMA_VENDOR_ != _CMA__APOLLO)
extern char	*sys_errlist[];
extern int	sys_nerr;
#endif

/*
 * LOCAL MACROS
 */

#if _CMA_OS_ == _CMA__VMS
typedef int (*exc___t_hndlr) _CMA_PROTOTYPE_((
	unsigned int sigargs[],
	struct chf$mech_array *mechargs));

# if _CMA_HARDWARE_ == _CMA__VAX
    /*
     * Define a macro to encapsulate "return SS$_RESIGNAL" inside the
     * condition handler. If a frame containing a TRY block previously
     * contained another condition handler, we save the handler in the TRY
     * context block. This macro allows that handler to have a chance at the
     * condition before we resignal. The return value of the saved handler is
     * returned. Before calling the handler, increment the mechanism array
     * depth field (number of frames to the handler's establisher), since the
     * exc_handler() frame is now intervening.
     */
#  define exc___resignal(ctx,sig,mech,est) { \
    int			_usesav = 0; \
    exc_context_t	*_rectx = (exc_context_t *)(ctx); \
    while (_rectx != (exc_context_t *)0) { \
	if ((exc_address_t)_rectx->current_frame == (exc_address_t)(est)) \
	    break; \
	_rectx = (exc_context_t *)_rectx->link; \
	} \
    while (_rectx != (exc_context_t *)0) { \
	if ((exc_address_t)_rectx->current_frame != (exc_address_t)(est)) \
	    break; \
	if ((exc_address_t)_rectx->old_handler != (exc_address_t)cma$exc_handler \
		&& (exc_address_t)_rectx->old_handler != (exc_address_t)0) { \
	    _usesav = 1; \
	    break; \
	    } \
	_rectx = (exc_context_t *)_rectx->link; \
	} \
    if (_usesav) \
	return ((exc___t_hndlr)_rectx->old_handler)((sig), (mech)); \
    else \
	return SS$_RESIGNAL; \
    }
# else
#  define exc___resignal(ctx,sig,mech,est)	return SS$_RESIGNAL;
# endif
#endif

#define exc___ctx_stack_top	((cma__get_self_tcb ())->exc_stack)

/*
 * LOCAL DATA
 */

#if _CMA_OS_ == _CMA__VMS
typedef struct EXC___T_SYSARGS {
    int		msgid;
    int		argcnt;
    } exc___t_sysargs;

# if _CMA_HARDWARE_ == _CMA__ALPHA
#  define exc___c_hwexcs	35
# else
#  define exc___c_hwexcs 32
# endif

# define FACID(name)	(((name) & STS$M_FAC_NO) >> STS$V_FAC_NO)
# define MSGID(name)	(((name) & STS$M_MSG_NO) >> STS$V_MSG_NO)

# ifndef SS$_VARITH
#  define SS$_VARITH	1252
# endif

# ifndef SS$_ILLVECOP
#  define SS$_ILLVECOP	1260
# endif

# ifndef SS$_VECALIGN
#  define SS$_VECALIGN	1268
# endif

# ifndef SS$_VECDIS
#  define SS$_VECDIS	1244
# endif

# ifndef SS$_HPARITH
#  define SS$_HPARITH	1276
# endif

# ifndef SS$_ALIGN
#  define SS$_ALIGN	1284
# endif

# ifndef SS$_UNALIGN_SP_LOAD
#  define SS$_UNALIGN_SP_LOAD 1292
# endif

 static exc___t_sysargs	exc___g_sysargs[exc___c_hwexcs] = {
    {MSGID(SS$_ACCVIO), 4}
    ,{MSGID(SS$_MCHECK), 2}
    ,{MSGID(SS$_ASTFLT), 6}
    ,{MSGID(SS$_BREAK), 2}
    ,{MSGID(SS$_CMODSUPR), 3}
    ,{MSGID(SS$_CMODUSER), 3}
    ,{MSGID(SS$_COMPAT), 3}
    ,{MSGID(SS$_OPCCUS), 2}
    ,{MSGID(SS$_OPCDEC), 2}
    ,{MSGID(SS$_PAGRDERR), 4}
    ,{MSGID(SS$_RADRMOD), 2}
    ,{MSGID(SS$_ROPRAND), 2}
    ,{MSGID(SS$_SSFAIL), 3}
    ,{MSGID(SS$_TBIT), 2}
    ,{MSGID(SS$_DEBUG), 3}
    ,{MSGID(SS$_ARTRES), 2}
    ,{MSGID(SS$_INTOVF), 2}
    ,{MSGID(SS$_INTDIV), 2}
    ,{MSGID(SS$_FLTOVF), 2}
    ,{MSGID(SS$_FLTDIV), 2}
    ,{MSGID(SS$_FLTUND), 2}
    ,{MSGID(SS$_DECOVF), 2}
    ,{MSGID(SS$_SUBRNG), 2}
    ,{MSGID(SS$_FLTOVF_F), 2}
    ,{MSGID(SS$_FLTDIV_F), 2}
    ,{MSGID(SS$_FLTUND_F), 2}
    ,{MSGID(SS$_INHCHMK), 3}
    ,{MSGID(SS$_INHCHME), 3}
    ,{MSGID(SS$_VARITH), 4}
    ,{MSGID(SS$_ILLVECOP), 3}
    ,{MSGID(SS$_VECALIGN), 4}
    ,{MSGID(SS$_VECDIS), 3}
# if _CMA_HARDWARE_ == _CMA__ALPHA
    ,{MSGID(SS$_HPARITH), 5}
    ,{MSGID(SS$_ALIGN), 4}
    ,{MSGID(SS$_UNALIGN_SP_LOAD), 3}
# endif
    };
#endif					/* VMS */

static void exc___copy_args _CMA_PROTOTYPE_ ((
	unsigned int	*sigarg,
	unsigned int	*ctxarg,
	exc_int_t	ctxlen));

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is called to raise an exception.
 *
 * 	Its job is to ensure that control is transferred to the
 * 	innermost active TRY clause in the current thread.  If a CATCH or 
 * 	CATCH_ALL clause is present and the exception matches, then the
 *	corresponding block of user code executed.  
 *
 *  FORMAL PARAMETERS:
 *
 *	Address of an exception object
 *
 *  IMPLICIT INPUTS:
 *
 *	The stack of context blocks for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	An error is reported if the context block stack is empty
 *	(no active TRY clauses).  If the exception passed in is
 *      uninitialized, then we raise a special exception of our own.
 *
 *  FUNCTION VALUE:
 *
 *	NONE
 *
 *  SIDE EFFECTS:
 *
 *	Control is transferred to outer scopes.
 */
extern void exc_raise
#ifdef _CMA_PROTO_
	(EXCEPTION *exc)
#else
	(exc)
	EXCEPTION *exc;
#endif
    {
    exc_context_t *ctx;
    exc_context_t **stack_top = (exc_context_t **)&exc___ctx_stack_top;


    /* 
     * Check the sentinel in the exception to see if the exception 
     * passed to us is uninitialized. If so, raise an implementation-defined 
     * exception.
     */
    if ((exc->kind != exc_kind_address_c) 
	    && (exc->kind != exc_kind_status_c))
	RAISE(exc_e_uninitexc);

#if _CMA_OS_ == _CMA__VMS
    /* 
     * If the target is VMS, then signal the exception as a VMS condition.
     * There are two cases, either the exception is that address of
     * an exception object or the exception is a status value that has
     * been imported (or copied) into the domain of the C exception
     * package.  These must be signalled differently.  In the first
     * case, the address of the exception is an FAO argument.  In the
     * the second case, the status is packaged as a chained condition
     * under a special CMA-defined condition (note: this prevents other
     * software from accidentally interpreting the status as a "real"
     * VMS condition; that would be wrong, since there are no FAO
     * arguments or chained conditions that a condition handler would
     * typically expect; also, the PC and PSL are of the "re-raise" 
     * not necessarily the original raise).  In the second case, we
     * pass a 0 FAO count and several zeros to try to get PUTMSG to
     * avoid finding accidental FAO parameters to display.  We should
     * fix PUTMSG to act on zero FAO parameters.
     */
    if (exc->kind == exc_kind_address_c)
	lib$stop (exc_s_exception, 1, exc->address.address);
    else {
	
	/*
	 * If we have a "V2" exception structure, and it points to a list of
	 * exception arguments (saved by the DECthreads exception handler),
	 * then use those arguments when raising the condition. Otherwise,
	 * just use the exception's status as the secondary (after the
	 * standard DECthreads "exception copied" status), and pad the
	 * argument list with some zeros in case the status calls for FAO
	 * arguments.
	 */
	if (exc->status.ext.sentinel == exc_newexc_c
		&& exc->status.ext.args != (unsigned int *)0)
	    lib$callg (exc->status.ext.args, lib$stop);
	else
	    lib$stop (exc_s_exccop, 0, exc->status.status, 0, 0, 0, 0, 0);

	}
#else
    /* 
     * Compute a pointer to the top context block on the context block stack
     * If the context block stack is empty (no active TRY), then abort
     * the program.
     */
    ctx = *stack_top;

    if (ctx == (exc_context_t *)0) {
	exc_report (exc);
	cma__abort_process (SIGIOT);
        }

    /*
     * Copy the exception value into the current active context block,
     * set its exception state to indicate that an exception is
     * active (i.e. raised but not yet handled), pop this context block
     * from the context block stack, and transfer control to the selected 
     * context block using longjump.
     */
    ctx->cur_exception = *exc;
    ctx->exc_state = exc_active_c;

    *stack_top = (exc_context_t *)ctx->link;
    exc_longjmp (ctx->jmp, 1);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is called to raise a system-defined status as 
 * 	an exception.
 *
 * 	The status is re-packaged as an exception and then 
 * 	exc_raise is called.
 *
 *  FORMAL PARAMETERS:
 *
 *	System status code, by value.
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
 *	Control is transferred to outer scopes.
 */
extern void exc_raise_status
#ifdef _CMA_PROTO_
	(exc_int_t	s)
#else
	(s)
	exc_int_t	s;
#endif
    {
#if _CMA_OS_ != _CMA__VMS
    /*
     * Declare a local exception, initialize it, convert it to a status,
     * and then raise it.
     */
    EXCEPTION temp_exc;


    EXCEPTION_INIT (temp_exc);
    exc_set_status (&temp_exc, s);
    exc_raise (&temp_exc);
#else
    lib$stop (exc_s_exccop, 0, s, 0, 0, 0, 0, 0);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is called to push an exception context in
 *	the current thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	Address of an exception context block.
 *
 *  IMPLICIT INPUTS:
 *
 *	The stack of context blocks for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The stack of context blocks for the current thread.
 *
 *  FUNCTION VALUE:
 *
 *	NONE
 *
 *  SIDE EFFECTS:
 *
 *	NONE
 */
extern void
exc_push_ctx
#ifdef _CMA_PROTO_
	(_CMA_VOLATILE_ exc_context_t *ctx)
#else
	(ctx)
	_CMA_VOLATILE_ exc_context_t *ctx;
#endif
    {
    exc_context_t **stack_top = (exc_context_t **)&exc___ctx_stack_top;


    /*
     * Perform DECthreads initialization if necessary: this allows TRY to be
     * used in the pthread exception interface prior to any direct DECthreads
     * calls.
     */
    cma__int_init ();

    /* 
     * Push the context block on the context stack
     */
    ctx->link = *stack_top;
    *stack_top = (exc_context_t *)ctx;

    /* 
     * Initialize the state of the context block
     */
    ctx->exc_state = exc_none_c;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is called to pop an exception context from
 *	the current thread.
 *
 *  FORMAL PARAMETERS:
 *
 * 	The address of the context block to pop.
 *
 *  IMPLICIT INPUTS:
 *
 *	The stack of context blocks for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The stack of context blocks for the current thread.
 *
 *  FUNCTION VALUE:
 *
 *	NONE
 *
 *  SIDE EFFECTS:
 *
 *	NONE
 */
extern void exc_pop_ctx
#ifdef _CMA_PROTO_
	(_CMA_VOLATILE_ exc_context_t *ctx)
#else
	(ctx)
	_CMA_VOLATILE_ exc_context_t *ctx;
#endif
    {
    /* 
     * Note: this routine is idempotent... can be repeated infinitely for the 
     * active context block with no ill effects.
     */

    /* 
     * Pop the context block.  Don't bother checking if the
     * context block is at the top... this is time critical code and
     * had better just be called correctly.  (Note this is not
     * necessary if an exception was raised since the stack was
     * already popped by RAISE, but it can't hurt.)
     */
    exc___ctx_stack_top = (exc_context_t *)ctx->link;

    /* 
     * If there is an active (unhandled) exception in this block,
     * then we must reraise it.  
     * Note: this code depends on exc_active_c being identically 0.
     */
    if (!ctx->exc_state) {
	exc_raise ((EXCEPTION *)&ctx->cur_exception);
        }

    /*
     * Mark as popped, so ENDTRY macro won't pop again.
     */
    ctx->exc_state = exc_popped_c;
    }

#if _CMA_OS_ == _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is a VMS condition handler to map VMS conditions
 * 	into the exceptions understood by this package.
 *
 *  FORMAL PARAMETERS:
 *
 * 	Standard VMS sigargs and mechargs.
 *
 *  IMPLICIT INPUTS:
 *
 * 	The stack of context blocks for the current thread.
 *
 *  IMPLICIT OUTPUTS:
 *
 * 	The stack of context blocks for the current thread.
 *
 *  FUNCTION VALUE:
 *
 *	Standard return values: SS$_RESIGNAL SS$_CONTINUE
 *
 *  SIDE EFFECTS:
 *
 *	May perform a longjump
 */
extern int exc_handler
#ifdef _CMA_PROTO_
	(unsigned int sigargs[], struct chf$mech_array *mechargs)
#else
	(sigargs, mechargs)
	unsigned int sigargs[];
	struct chf$mech_array *mechargs;
#endif
    {
    _CMA_VOLATILE_ exc_context_t	*ctx;
    exc_context_t	   **stack_top = (exc_context_t **)&exc___ctx_stack_top;
    exc_address_t 			establisher_frame;
    EXCEPTION 				*exc;
    int 				primary, depth;


    /* Layout of sigargs:
     *		0 argcount
     *		1 primary condition
     *		2 FAO count of primary /or/ secondary condition
     *		3 FAO parameter        /or/ ...
     */
    primary = sigargs[1];

    /* Layout of mechargs:
     *		0 argcount
     *		1 frame
     *		2 depth
     *		3 save R0
     *		4 save R1
     */
    establisher_frame = (pthread_addr_t)(mechargs->frame_field);

    /*
     * If there is no active TRY clause, then resignal so that some
     * other VMS condition handler has a chance to handle the error.
     */
    ctx = (_CMA_VOLATILE_ exc_context_t *)*stack_top;

    if (ctx == NULL) 
	return SS$_RESIGNAL; 	
     
    /* 
     * If unwinding the stack, or the debugger has been asynchronously
     * signalled, or the primary condition does not have a severe
     * status (low 3 bits = 4), then resignal.
     * 
     * Note: this is compatible with components that
     * signal conditions to print a message and expect continuation
     * because the VMS catchall handler continues execution on
     * all non-severes.  Hence, any component that signals non-severes
     * *must* be prepared to expect continuation... unless it has a prior 
     * agreement with some caller... which is non-modular anyway and with 
     * which our resignalling does not interfere anyway.
     *
     * We catch all SEVERE's, however, as they are "terminating" 
     * error cases, just like the "terminating exceptions for C" that
     * we are implementing.  On all such errors, and only such errors,
     * we allow any active TRY clause to gain control.
     *
     */
    if ((primary == SS$_UNWIND) || (primary == SS$_DEBUG) 
	    || ((primary & 7) != 4))
	exc___resignal (ctx, sigargs, mechargs, establisher_frame);
	
    /*
     * Here on out, we have a severe error and we are not unwinding.
     */

    /*
     * If the current active TRY clause is not in the same frame we were
     * established in, then resignal. (This is necessary to ensure optimal
     * integration with VMS condition handling.  It ensures that the actual
     * VMS condition propagates out of the frame rather than a C longjump
     * condition.)
     */
    if (ctx->current_frame != establisher_frame) 
	exc___resignal (ctx, sigargs, mechargs, establisher_frame);

    /*
     * Repackage the VMS condition as an exception
     */

    exc = (EXCEPTION *)&ctx->cur_exception;	/* Bind to context's exception */

    /*
     * Initialize the exception. If the context block is version 2, then use
     * a version 2 exception; otherwise initialize a version 1 exception.
     * (Can't use EXCEPTION_INIT, since it would set fields V1 exceptions
     * don't have.)
     */
    if (ctx->sentinel == exc_newexc_c) {
	EXCEPTION_INIT (*exc);
	exc->kind = exc_kind_status_c;
	}
    else
	exc->kind = exc_kind_status_c;

    if (primary == exc_s_exception) {
	exc->kind = exc_kind_address_c;
	exc->address.address = (exc_address_t)sigargs[3];
	}
    else if (sigargs[1] == exc_s_exccop || sigargs[1] == exc_s_exccoplos)
	exc->status.status = sigargs[3];
    else
	exc->status.status = sigargs[1];

    /*
     * Copy the argument vector for resignalling or reporting.
     */
    if (ctx->sentinel == exc_newexc_c) {
	exc___copy_args (&sigargs[0], &ctx->exc_args[0], exc_excargs_c);
	exc->status.ext.args = &ctx->exc_args[0];
	}

    /*
     * Set the exception state of the context block to indicate that an
     * exception is active (i.e. raised but not yet handled), pop the
     * context block from the context block stack, and unwind the stack to
     * the frame of the TRY; transfer control to the cma__restore_exc_context
     * code to restore registers.
     */
    ctx->exc_state = exc_active_c;
    *stack_top = ctx->link;
    mechargs->savr0_field = (cma_t_integer)&ctx->jmp;
    depth = (mechargs->depth_field > 0 ? mechargs->depth_field - 1 : 0);

    return sys$unwind (&depth, cma__restore_exc_context);
    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This function is used to report exceptions to the user.  It should be
 *	adapted to use native environment reporting mechanisms were they
 *	exist (on VMS, it uses the $putmsg system service).
 *
 *  FORMAL PARAMETERS:
 *
 *	exception	Address of an exception structure (EXCEPTION *)
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
exc_report
#ifdef _CMA_PROTO_
	(EXCEPTION	*exc) 
#else
	(exc)
	EXCEPTION	*exc;
#endif
    {
#if _CMA_OS_ == _CMA__VMS
    unsigned int	msgvec[6], *mvp, old;


    if (exc->status.ext.sentinel != exc_newexc_c)
	old = 1;
    else
	old = 0;

    /*
     * If it's an old-format exception structure, or a new one that doesn't
     * have any argument list, then fake a message vector.
     */
    if (old || exc->status.ext.args == (unsigned int *)0) {

	if (exc->kind == exc_kind_status_c) {
	    msgvec[0] = 1;
	    msgvec[1] = exc->status.status;
	    msgvec[2] = 0;
	    msgvec[3] = 0;
	    msgvec[4] = 0;
	    msgvec[5] = 0;
	    mvp = &msgvec[0];
	    }
	else {
	    msgvec[0] = 3;
	    msgvec[1] = exc_s_exception;
	    msgvec[2] = 1;
	    msgvec[3] = (int)exc->address.address;
	    mvp = &msgvec[0];
	    }

	}
    else
	mvp = exc->status.ext.args;
    
    sys$putmsg (mvp, 0, 0, 0);
    }
#else
    (void)fflush (stdout);		/* Flush output stream, first */

    if (exc->kind == exc_kind_status_c) {
# if _CMA_VENDOR_ == _CMA__APOLLO
        status_$t st;
        static char *msg = "Exception: ";
        st.all = exc->status.status;
        error_$print_name(st, msg, strlen(msg));
# else
	unsigned char	buffer[256], *stuff;
	int		status, arg = 0, len;
	long		statmask;


	cma__int_sprintf ((char *)buffer, "Exception: ");
	stuff = &buffer[0] + cma__strlen ((char *)buffer);

	/*
	 * Try to support using errno values as status codes in exceptions;
	 * if the high word of the code is 0, assume it's an errno.
	 * Otherwise, it better be a DCE status code.
	 */
	statmask = exc->status.status & (~0xffff);

	if (statmask == 0) {

	    if (exc->status.status < sys_nerr) {
		cma__int_sprintf (
			(char *)stuff,
			"%s",
			sys_errlist[exc->status.status]);
		status = 0;
		}
	    else {
		cma__error_inq_text (exc_s_unkstatus, stuff, &status);
		arg = exc->status.status;
		}

	    }
	else {
	    cma__error_inq_text (exc->status.status, stuff, &status);
	    }

	if (status != -1) {
	    len = cma__strlen ((char *)buffer);
	    buffer[len] = '\n';
	    buffer[len+1] = (char)0;
	    cma__int_fprintf (stderr, (char *)buffer, arg, 0);
	    }
# endif
	}
    else {
	unsigned char	buffer[256], *stuff;
	int		status, len;


	cma__int_sprintf ((char *)buffer, "Exception: ");
	stuff = &buffer[0] + cma__strlen ((char *)buffer);
	cma__error_inq_text (exc_s_exception, stuff, &status);

	if (status != -1) {
	    len = cma__strlen ((char *)buffer);
	    buffer[len] = '\n';
	    buffer[len+1] = (char)0;
	    cma__int_fprintf (
		    stderr,
		    (char *)buffer,
		    exc->address.address,
		    0);
	    }

	}

    }
#endif

#if _CMA_OS_ == _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This function is used on VMS systems to copy a condition signal
 *	vector (or message vector) from one place to another. It copies
 *	arguments that may be stack addresses as 0, and pre-pends the
 *	exc_s_exccop condition if it's not already there.
 *
 *  FORMAL PARAMETERS:
 *
 *	sigvec		Address of signal vector
 *	ctxvec		Address of output vector
 *	ctxlen		Size (in ints) of output vector
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
exc___copy_args
#ifdef _CMA_PROTO_
	(unsigned int	*sigarg,
	unsigned int	*ctxarg,
	exc_int_t	ctxlen)
#else
	(sigarg, ctxarg, ctxlen)
	unsigned int	*sigarg;
	unsigned int	*ctxarg;
	exc_int_t	ctxlen;
#endif
    {
    exc_int_t		count, facility, lost;
    unsigned int	condition;
    exc_int_t		in, out, size, lasthdw;


    if (sigarg[0] >= ctxlen)
	count = ctxlen - 1;
    else
	count = sigarg[0] - 2;

    lost = 0;				/* So far, no data lost */
    lasthdw = 0;			/* So far, no hardware exceptions */
    size = ctxlen - 3;			/* Actual size minus count, pc, ps */
    in = 1;
    facility = sigarg[1] & STS$M_FAC_NO;	/* Get unshifted facility */

    /*
     * If the signal vector already starts with a DECthreads facility
     * message, copy it as is (this includes foreign messages that have
     * already been prefixed by "exception copied" or "exception copied with
     * lost data" codes). Otherwise, add the prefix.
     */
    if (facility == (exc_facility_c & STS$M_FAC_NO))
	out = 1;
    else {
	out = 3;
	ctxarg[1] = exc_s_exccop;
	ctxarg[2] = 0;
	count += 2;			/* We added two fields */
	size -= 2;
	}

    ctxarg[0] = count;

    while (out <= count) {
	exc_int_t	argcnt, argzero;


	condition = sigarg[in];
	facility = FACID (condition);
	
	if (facility == 0) {
	    exc_int_t	i, msgid;


	    switch (condition) {
		case SS$_FLTOVF_F : {
		    condition = SS$_FLTOVF;
		    break;
		    }
		case SS$_FLTDIV_F : {
		    condition = SS$_FLTDIV;
		    break;
		    }
		case SS$_FLTUND_F : {
		    condition = SS$_FLTUND;
		    break;
		    }
		default : {
		    break;
		    }

		}

	    msgid = MSGID (condition);
	    argcnt = 0;
	    argzero = 0;
	    lasthdw = 0;

	    for (i = 0; i < exc___c_hwexcs; i++) {

		if (msgid == exc___g_sysargs[i].msgid) {
    		    argcnt = exc___g_sysargs[i].argcnt;
		    lasthdw = 1;	/* It's a hardware exception! */
		    break;
		    }

		}
		    
	    }
	else if (facility == 1) {
	    argcnt = 1;
	    argzero = 0;
	    lasthdw = 0;
	    }
	else if (facility == (exc_facility_c >> 16)) {
	    argcnt = sigarg[in+1] + 1;	/* Copy count and any args */
	    argzero = 0;		/* Assume our stuff is copyable! */
	    lasthdw = 0;
	    }
	else {
	    argcnt = 1;			/* Copy count */
	    argzero = sigarg[in+1];
	    lost = 1;			/* Set the "lost" flag */
	    lasthdw = 0;
	    }

	/*
	 * If there's enough room, copy the condition into the output vector.
	 * If there's not enough room for the whole thing, don't copy any of
	 * it, since $putmsg might freak out.
	 */
	if (size >= argcnt + argzero + 1) {
	    size -= (argcnt + argzero + 1);
	    ctxarg[out++] = condition;	/* Copy it */
	    in++;

	    /*
	     * If we need to copy arguments, do so
	     */
	    while (argcnt > 0) {
		ctxarg[out++] = sigarg[in++];
		argcnt--;
		}

	    /*
	     * We need to zero any user condition arguments: that's a shame,
	     * but they might be stack addresses that would do something
	     * weird when we try to print the message later.
	     */
	    while (argzero > 0) {
		ctxarg[out++] = 0;
		in++;
		argzero--;
		}

	    }
	else {
	    lost = 1;
	    break;
	    }
	
	}

    /*
     * If the last condition was a hardware type, and we already copied the
     * PC and PS to the two slots we reserved for them (which would take us
     * beyond the expected count, since we'd dropped the PC/PSL), then add
     * them to the count. Otherwise, copy two zeros, so something will be
     * there just in case a user condition code does something odd.
     */
    if (lasthdw && out > (count + 1))
	ctxarg[0] += 2;			/* Admit to the extra stuff in count */
    else {
	ctxarg[out++] = 0;
	ctxarg[out++] = 0;
	}

    /*
     * If we lost any information, then change the primary condition code
     * from exc_s_exccop to exc_s_exccoplos so the user will know.
     */
    if (lost && ctxarg[1] == exc_s_exccop)
	ctxarg[1] = exc_s_exccoplos;

    }
#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element EXC_HANDLING.C */
/*  *40   27-APR-1993 12:12:05 BUTENHOF "Remove _EXC_ symbols" */
/*  *39   23-MAR-1993 08:52:39 BUTENHOF "Fix test for 'errno' exception" */
/*  *38    1-MAR-1993 12:06:01 BUTENHOF "Remove '$'" */
/*  *37   15-JAN-1993 16:25:17 SCALES "Cache the stack-top value to decrease calls to get-self-tcb" */
/*  *36   20-APR-1992 07:29:06 BUTENHOF "Remove exception interest" */
/*  *35   17-APR-1992 11:12:16 BUTENHOF "Improve TRY/CATCH performance" */
/*  *34   31-MAR-1992 15:12:18 BUTENHOF "Fix VMS exception interest" */
/*  *33   31-MAR-1992 13:31:33 BUTENHOF "Implement exception 'interest' model" */
/*  *32   10-FEB-1992 08:51:01 BUTENHOF "Drop depth modifications on VMS" */
/*  *31    7-FEB-1992 10:31:09 BUTENHOF "Clean up VAX VMS resignal" */
/*  *30    6-FEB-1992 10:24:50 CURTIN "Added individual ifndef's for Alpha hardware exc's" */
/*  *29   22-JAN-1992 17:40:10 CURTIN "added a few casts" */
/*  *28    9-JAN-1992 10:27:38 BUTENHOF "Move ""Exception:"" text from message catalog" */
/*  *27   23-DEC-1991 14:06:04 CURTIN " remove include of starlet.h on vax" */
/*  *26   20-DEC-1991 07:30:01 BUTENHOF "Alpha exception work" */
/*  *25   18-DEC-1991 06:45:44 BUTENHOF "Remove exc_reraise function" */
/*  *24   16-DEC-1991 12:55:26 BUTENHOF "Update previous exception changes" */
/*  *23   13-DEC-1991 09:54:05 BUTENHOF "Clear message args on putmsg" */
/*  *22   27-NOV-1991 11:03:46 BUTENHOF "Fix for DEC C" */
/*  *21   26-NOV-1991 11:19:12 BUTENHOF "More Alpha changes" */
/*  *20   25-NOV-1991 14:00:15 BUTENHOF "Make cma_message.h conditional" */
/*  *19   22-NOV-1991 11:57:45 BUTENHOF "Integrate dce message formatting" */
/*  *18   18-NOV-1991 10:24:29 BUTENHOF "Fix test for ""resignal""" */
/*  *17    6-NOV-1991 09:06:55 BUTENHOF "Fix config macro use" */
/*  *16   31-OCT-1991 12:40:27 BUTENHOF "Unwind to called frame if depth != 0" */
/*  *15   24-SEP-1991 16:30:13 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *14   21-AUG-1991 16:46:33 CURTIN "Removed VMS include of stdio.h" */
/*  *13    6-AUG-1991 17:03:47 CURTIN "fix nested TRYs" */
/*  *12   31-JUL-1991 18:40:13 BUTENHOF "Improve VMS condition integration" */
/*  *11   26-JUL-1991 15:56:57 CURTIN "Use internal routines instead of setjmp/longjmp" */
/*  *10    2-JUL-1991 16:47:29 BUTENHOF "Make exc_report flush stdout" */
/*  *9    13-JUN-1991 18:03:10 CURTIN "Converted to use Unix messages on Unix platforms" */
/*  *8    10-JUN-1991 18:25:36 SCALES "Add sccs headers for Ultrix" */
/*  *7     2-MAY-1991 14:00:08 BUTENHOF "Utilize _longjmp where appropriate" */
/*  *6    15-APR-1991 15:54:40 CURTIN "clean up previous addition" */
/*  *5     8-APR-1991 20:32:26 CURTIN "changed cma__abort to cma__process_abort" */
/*  *4     1-APR-1991 18:10:08 BUTENHOF "QAR 93, exception text" */
/*  *3    24-JAN-1991 00:35:30 BUTENHOF "Fix exception name references" */
/*  *2    28-DEC-1990 00:04:49 BUTENHOF "Change exception names" */
/*  *1    12-DEC-1990 21:56:12 BUTENHOF "Exception support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element EXC_HANDLING.C */
