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
static char *rcsid = "@(#)$RCSfile: cma_tis_sup.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/08/18 14:53:21 $";
#endif
/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	This module contains the routines which make the CMA$TIS_SHR image
 *	and the OSF/1 reentrant libraries work with DECthreads.
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	19 May 1992
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	26 May 1992
 *		Correct definition of JMP instruction.
 *	002	Webb Scales	28 May 1992
 *		Added conditional condition variables.
 *	003	Brian Keane	09 June 1992
 *	        Fixup compile problem in libc_r stuff.  Reorder some of
 * 		init_tis so that the necessary mutexes are in place.
 *	004	Dave Butenhof, Webb Scales	09 June 1992
 *		Try to fix MIPS compile errors. MIPS cpp doesn't like a macro
 *		that doesn't expand all it's arguments (and ...dbg_obj_search
 *		accidentally didn't).
 *	005	Webb Scales	9 July 1992
 *		Added support for EVMS.
 *	006	Paul Curtin & Webb Scales	20 July 1992
 *		Rework cma___redirect_tis_func macro on Alpha/VMS.
 *	007	Webb Scales	27 July 1992
 *		Fix inter-platform incompatibilities.
 *	008	Dave Butenhof	03 August 1992
 *		Fix 64-bit type problems (like returning "int" for TCB
 *		pointer).
 *	009	Dave Butenhof	13 August 1992
 *		Fix compilation warning on Mach platforms.
 *	010	Dave Butenhof	28 August 1992
 *		Change TIS condition wait to call cma__int_wait rather than
 *		(obsolete) cma__wait macro.
 *	011	Brian Keane	04 November 1992
 *		Preallocate enough pool so that libc_r initialization can take
 *		place without needing to sbrk (which can't be done while
 *		initialization is in progress since it needs mutexes that
 *		are not yet created).
 *	012	Webb Scales	 9 November 1992
 *		Set DEC C RTL reentrancy level in the cell in the TIS image.
 *	013	Webb Scales	19 November 1992
 *		- Added errno-handling functions to the redirected TIS functions
 *		- Added call to TIS for the addresses of the errno tables.
 *	014	Dave Butenhof	10 December 1992
 *		Clear the default thread's errno (on OSF/1) during
 *		initialization, using seterrno() -- this pulls in cma_errno.o
 *		so that OSF/1 thread programs can be linked with -lpthreads
 *		-lc_r rather than duplicating -lpthreads.
 *	015	Brian Keane	20 January 1993
 *		For shared libraries, disable loader lazy text evaluation,
 *		since it is not reentrant.
 *	016	Dave Butenhof	28 January 1993
 *		Disable TIS synchronization on a DECthreads bugcheck, so we
 *		can do file output without inconvenient blocking (bugcheck
 *		runs "single threaded" anyway).
 *	017	Brian Keane	26 February 1993
 *		Fix 014 - actually use seterrno() to access errno rather
 *		than cma_set_errno() which doesn't quite do what we thought.
 *		Reorder assertion checks in debug macros so the
 *		cma___g_dbg_tis_m mutex is unlocked prior to making the
 *		assertion.  This prevents cascading assertions.
 *	018	Webb Scales	 1 March 1993
 *		Fix compiler prototype warnings on ALPHA.
 *	019	Dave Butenhof	31 March 1993
 *		Add formatted name strings to TIS sync. objects.
 *	020	Dave Butenhof	12 April 1993
 *		Add argument to cma__int[_timed]_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	021	Dave Butenhof	 6 May 1993
 *		Add cma__g_tis_disable flag to replace use of
 *		cma__g_bugchecked, so lock/unlock can be disabled during TIS
 *		initialization (mostly important for cma__trace during init)
 *		and during cma_debug().
 *	022	Dave Butenhof	 2 June 1993
 *		Avoid an annoying CMA_TRACE artifact by allowing TIS mutex
 *		lock to dynamically create the lock when not NDEBUG.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_mutex.h>
#include <cma_condition.h>
#include <cma_attr.h>
#include <cma_int_errno.h>
#include <cma_vm.h>
#include <cma_tis.h>
#include <cma_tis_int.h>
#include <cma_tis_sup.h>

#if _CMA_OS_ == _CMA__VMS
# include <cma_tis_jt.h>
#endif


/*
 *  LOCAL MACROS
 */

/*
 * Add an object to the list of TIS-created objects.
 */
#ifndef NDEBUG
# define cma___tis_dbg_obj_add(obj_ptr) { \
    cma__int_lock (cma___g_dbg_tis_m); \
    if (cma___g_dbg_tis_c < 1024) { \
	cma___g_dbg_tis_v[cma___g_dbg_tis_c++] = *obj_ptr; \
	cma__int_unlock (cma___g_dbg_tis_m); } \
    else { \
	cma__int_unlock (cma___g_dbg_tis_m); \
	cma__assert_fail (0, "Too many TIS objects");} \
    }
#else
# define cma___tis_dbg_obj_add(obj_ptr)
#endif

/*
 * Search for a TIS-created object, and optionally perform some operation.
 * An assertion failure occurs if the object is not found.  This macro is
 * used only in debugging -- it expands vaccuously in a production build.
 */
#ifndef NDEBUG
# define cma___tis_dbg_obj_search(obj_ptr,_function_,_found_processing_) { \
    int	i, done = cma_c_false; \
    cma__int_lock (cma___g_dbg_tis_m); \
    for (i = 0; i < cma___g_dbg_tis_c; i++) \
	if (cma___g_dbg_tis_v[i] == *(obj_ptr)) { \
	    done = cma_c_true; \
	    break; \
	    } \
    if (done) { \
        (_found_processing_); \
        cma__int_unlock (cma___g_dbg_tis_m); } \
    else { \
        cma__int_unlock (cma___g_dbg_tis_m); \
        cma__assert_fail (done, "Attempt to _function_ a non-existent TIS obj_ptr"); } \
    }
#else
# define cma___tis_dbg_obj_search(obj_ptr,_function_,_found_processing_)
#endif

/*
 * Macro which redirects synchronization functions into DECthreads
 */
#if _CMA_PLATFORM_ == _CMA__VAX_VMS

/*
 * On VAX VMS, the CMA$TIS_SHR image contains a writeable transfer vector.
 * This macro gets a pointer to the transfer vector entry by prefixing 
 * "func" with "cma$tis_".  It then copies the appropriate entry mask into
 * the transfer vector entry, followed by a JMP instruction, followed by
 * the address of the first instruction in the DECthreads routine.  Note,
 * that an REI instruction must be executed before any of these newly
 * written transfer vector entries to ensure proper visibility, according
 * to the SRM.
 */
# define JMP 0x9F17;
# define cma___redirect_tis_func(func, entry_pt) { \
	cma___t_tis_tve *tve_ptr, *rtn_ptr; \
	tve_ptr = (cma___t_tis_tve *)(cma$tis_/**/func); \
	rtn_ptr = (cma___t_tis_tve *)(entry_pt); \
	tve_ptr->mask = rtn_ptr->mask; \
	tve_ptr->inst = JMP; \
	tve_ptr->dst = &(rtn_ptr->inst); \
	}

#elif _CMA_PLATFORM_ == _CMA__ALPHA_VMS

/*
 * On ALPHA VMS, the CMA$TIS_SHR image contains a jump table, accessible
 * via universal pointers and #define'd symbolic indexes.  The redirection
 * is done by overwriting the default values in the table with the pointers
 * to entries in DECthreads.
 */
# define cma___redirect_tis_func(func, entry_pt) \
	tis_vector_begin[func] = (int (*)())(entry_pt)

#elif (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_

/*
 * On OSF/1, a jump table is constructed and then passed to libc_r.  The
 * addresses of the functions are copied into the table appropriate
 * field in the table.
 */
# define cma___redirect_tis_func(func, entry_pt) \
    cma___g_libcr_funcs.func = (entry_pt)

#else

/*
 * On our other platforms, this functionality isn't required (yet), so do
 * nothing.
 */
# define cma___redirect_tis_func(func, entry_pt)

#endif


/*
 *  GLOBAL DATA
 */

#if (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
/*
 * crt0.o function, for implicit initialization (?)
 */
 void (*_pthread_init_routine)(void) = cma_init;
#endif

/*
 * Addresses of the errno tables in the TIS image.
 */
#if _CMA_OS_ == _CMA__VMS
 cma_t_errno **cma__g_errno_tbl;
 cma_t_errno **cma__g_vmserrno_tbl;
#endif

/*
 * This flag allows the TIS interlocking functions to be disabled during
 * certain points when the lock state may reasonably be inconsistent: for
 * example, during debugging, during a DECthreads bugcheck, and during
 * the process of initializing TIS.
 */
cma_t_boolean	cma__g_tis_disable = cma_c_false;	/* Start enabled */

/*
 *  LOCAL DATA
 */
typedef	cma__t_atomic_bit   *cma__int_tis_spinlock_t;

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
/*
 * Structure representing a routine or transfer vector entry point.
 */
typedef struct CMA___T_TIS_TVE {
	unsigned short	mask;	    /* Routine entry mask */
	unsigned short 	inst;	    /* First instruction w/ addressing mode */
	unsigned long	dst;	    /* First operand */
	} cma___t_tis_tve;

#elif (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
/*
 * Table of synchronization functions used by libc_r.
 */
typedef	int	(*cma___t_libcr_mutex_func)(cma_tis_mutex_t *mutex);
typedef	int	(*cma___t_libcr_spin_func)(cma__int_tis_spinlock_t *spinner);
typedef	int	(*cma___t_libcr_cond_func)(cma_tis_cond_t *cond);
typedef	int	(*cma___t_libcr_condwait_func)
			(cma_tis_cond_t *cond, cma_tis_mutex_t *mutex);

typedef cma_tis_thread_t	(*cma___t_libcr_id_func)(void);
typedef cma_t_errno *		(*cma___t_libcr_get_errno_func)(void);
typedef void			(*cma___t_libcr_set_errno_func)(cma_t_errno);

typedef struct CMA___T_LIBCR_LOCK_FUNC_TABLE {
	/* Start of libc_r table */
	cma___t_libcr_mutex_func	mutex_create;
	cma___t_libcr_mutex_func	mutex_delete;
	cma___t_libcr_mutex_func	mutex_lock;
	cma___t_libcr_mutex_func	mutex_unlock;
	cma___t_libcr_mutex_func	mutex_trylock;
	cma___t_libcr_spin_func		spin_create;
	cma___t_libcr_spin_func		spin_delete;
	cma___t_libcr_spin_func		spin_lock;
	cma___t_libcr_spin_func		spin_unlock;
	cma___t_libcr_spin_func		spin_trylock;
	cma___t_libcr_id_func		thread_get_self;
	/* End of libc_r table */
	cma___t_libcr_cond_func		cond_create;
	cma___t_libcr_cond_func		cond_delete;
	cma___t_libcr_cond_func		cond_broadcast;
	cma___t_libcr_cond_func		cond_signal;
	cma___t_libcr_condwait_func	cond_wait;
	cma___t_libcr_get_errno_func	errno_get_addr;
	cma___t_libcr_get_errno_func	vmserrno_get_addr;
	cma___t_libcr_set_errno_func	errno_set_value;
	cma___t_libcr_set_errno_func	vmserrno_set_value;
	} cma___t_libcr_lock_func_table;

static cma___t_libcr_lock_func_table	cma___g_libcr_funcs;
#endif

#ifndef NDEBUG
/*
 * Debugging data structures
 */
static cma__t_int_mutex		*cma___g_dbg_tis_m;
static cma_tis_mutex_t		cma___g_dbg_tis_v[1024];
static int			cma___g_dbg_tis_c;
#endif

static char			*cma___g_tis_obj = "for TIS (0x%lx)";

/*
 * LOCAL FUNCTIONS
 */

static cma_t_errno *cma___tis_get_errno_addr _CMA_PROTOTYPE_ ((void));
static cma_t_errno *cma___tis_get_vmserrno_addr _CMA_PROTOTYPE_ ((void));
static void cma___tis_set_vmserrno_value _CMA_PROTOTYPE_ ((cma_t_errno value));
static int cma___tis_cond_create _CMA_PROTOTYPE_ ((cma_tis_cond_t *cond));
static int cma___tis_cond_delete _CMA_PROTOTYPE_ ((cma_tis_cond_t *cond));
static int cma___tis_cond_broadcast _CMA_PROTOTYPE_ ((cma_tis_cond_t *cond));
static int cma___tis_cond_signal _CMA_PROTOTYPE_ ((cma_tis_cond_t *cond));
static int cma___tis_cond_wait 
	_CMA_PROTOTYPE_ ((cma_tis_cond_t *cond, cma_tis_mutex_t *mutex));
static int cma___tis_mutex_create _CMA_PROTOTYPE_ ((cma_tis_mutex_t *mutex));
static int cma___tis_mutex_delete _CMA_PROTOTYPE_ ((cma_tis_mutex_t *mutex));
static int cma___tis_mutex_lock _CMA_PROTOTYPE_ ((cma_tis_mutex_t *mutex));
static int cma___tis_mutex_unlock _CMA_PROTOTYPE_ ((cma_tis_mutex_t *mutex));
static int cma___tis_mutex_trylock _CMA_PROTOTYPE_ ((cma_tis_mutex_t *mutex));
static int cma___tis_spin_create 
	_CMA_PROTOTYPE_ ((cma__int_tis_spinlock_t *spinner));
static int cma___tis_spin_delete 
	_CMA_PROTOTYPE_ ((cma__int_tis_spinlock_t *spinner));
static int cma___tis_spin_lock 
	_CMA_PROTOTYPE_ ((cma__int_tis_spinlock_t *spinner));
static int cma___tis_spin_unlock 
	_CMA_PROTOTYPE_ ((cma__int_tis_spinlock_t *spinner));
static int cma___tis_spin_trylock 
	_CMA_PROTOTYPE_ ((cma__int_tis_spinlock_t *spinner));
static cma_tis_thread_t cma___tis_thread_get_self _CMA_PROTOTYPE_ ((void));


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize this module.  Redirect synchronization functions in the
 *	CMA$TIS_SHR image and the OSF/1 reentrant libraries to point at 
 *	DECthreads routines.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	Symbols in the TIS image, and the routines in this module.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The CMA$TIS_SHR image transfer vector is rewritten.
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	All calls to CMA$TIS_SHR image and all locking functions in libc_r
 *	will call into DECthreads.
 */
void
cma__init_tis
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    int (**tis_vector_begin)(), (**tis_vector_end)();
    _CMA_IMPORT_    cma$decc$g_reentrancy;


    cma__tis_vector_get_bounds (
	    (void **)&tis_vector_begin, 
	    (void **)&tis_vector_end);

    cma__tis_etable_get_addr (
	    (void **)&cma__g_errno_tbl,
	    (void **)&cma__g_vmserrno_tbl);

    /*
     * Skip the default errno cell.  We'll be using a per-thread cell instead.
     */
    cma__g_errno_tbl++;
    cma__g_vmserrno_tbl++;

    /*
     * Make sure that the TIS image contains the appropriate number of 
     * entry points.
     * 
     * FIX-ME: this test should probably be more rigorous...
     */
    cma__assert_fail (
	tis_vector_end - tis_vector_begin > 0,
	"Mismatch with TIS vector");

# if _CMA_PLATFORM_ == _CMA__VAX_VMS
    cma__assert_fail (
	    sizeof(cma___t_tis_tve) == 8,
	    "cma___t_tis_tve is the wrong size");
# endif

    if (cma$$tis_objects_exist())
	cma__bugcheck ("Activation of DECthreads attempted after creating TIS mutexes.");

# define decc_tolerant	    0
# define decc_AST	    1
# define decc_multithread   2
# define decc_none	    4
# define decc_default	    0x80000000
# define decc_mask \
	    (decc_tolerant | decc_AST | decc_multithread | decc_none)

    /*
     * If the present reentrancy setting is a default value, or if it is not
     * currently set to a higher level than decc_multithread, then set it
     * to decc_multithread.
     */
    if (((cma$decc$g_reentrancy & decc_default) != 0) ||
	((cma$decc$g_reentrancy & ~decc_mask) == 0)) {
	cma$decc$g_reentrancy = decc_multithread;
	}
#endif

    cma__g_tis_disable = cma_c_true;	/* Disable locking for init */
#ifndef NDEBUG
    cma___g_dbg_tis_c = 0;
    cma___g_dbg_tis_m = cma__get_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma___g_dbg_tis_m, "TIS debug mutex");
#endif

    cma___redirect_tis_func (errno_get_addr, cma___tis_get_errno_addr);
    cma___redirect_tis_func (vmserrno_get_addr, cma___tis_get_vmserrno_addr);
    cma___redirect_tis_func (errno_set_value, cma__tis_set_errno_value);
    cma___redirect_tis_func (vmserrno_set_value, cma___tis_set_vmserrno_value);
    cma___redirect_tis_func (cond_create, cma___tis_cond_create);
    cma___redirect_tis_func (cond_delete, cma___tis_cond_delete);
    cma___redirect_tis_func (cond_broadcast, cma___tis_cond_broadcast);
    cma___redirect_tis_func (cond_signal, cma___tis_cond_signal);
    cma___redirect_tis_func (cond_wait, cma___tis_cond_wait);
    cma___redirect_tis_func (mutex_create, cma___tis_mutex_create);
    cma___redirect_tis_func (mutex_delete, cma___tis_mutex_delete);
    cma___redirect_tis_func (mutex_lock, cma___tis_mutex_lock);
    cma___redirect_tis_func (mutex_trylock, cma___tis_mutex_trylock);
    cma___redirect_tis_func (mutex_unlock, cma___tis_mutex_unlock);
    cma___redirect_tis_func (spin_create, cma___tis_spin_create);
    cma___redirect_tis_func (spin_delete, cma___tis_spin_delete);
    cma___redirect_tis_func (spin_lock, cma___tis_spin_lock);
    cma___redirect_tis_func (spin_trylock, cma___tis_spin_trylock);
    cma___redirect_tis_func (spin_unlock, cma___tis_spin_unlock);
    cma___redirect_tis_func (thread_get_self, cma___tis_thread_get_self);

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
    cma__do_rei();	    /* Required before executing dynamic code. */
#elif (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
    /*
     * Allocate/free enough memory so that declaring (thus creating)
     * the locks doesn't cause calls to sbrk.
     *
     * FIX-ME: This is a sorry hack
     */
    cma__free_mem (cma__alloc_mem ((cma_t_natural) cma__c_pool_size));

    /*
     * DECthreads programs have needed to be linked with arcane incantations
     * such as "-lpthreads -lc_r -lpthreads" because there are circular
     * dependencies. DECthreads calls libc_declare_lock_functions(), and
     * libc_r in turn references seterrno() and _errno() that need to be
     * resolved from DECthreads. If the second -lpthreads is omitted, the
     * symbols may be unresolved (or resolved from the wrong place).
     *
     * Paula Long just pointed out an elegant solution -- if _pthread_init()
     * [which is really cma_init()] references the object module with those
     * symbols, they'll be there waiting for libc_r, and the second
     * -lpthreads shouldn't be necessary -- and it will also remove the
     * danger of resolving the symbols from the wrong place.
     *
     * Of course, adding the errno functions to the "lock function" vector
     * (as we did for TIS on OpenVMS) would be a far better solution! I also
     * thought of just moving seterrno() and _errno() to this module (which
     * would really be more appropriate)... but this change is simpler :-)
     */
    seterrno(0);			/* Pull in cma_errno.o */
    libc_declare_lock_functions (&cma___g_libcr_funcs);
    ldr_declare_lock_functions (&cma___g_libcr_funcs);

    /*
     * FIX-ME:
     * 
     * The loader's lazy text evaluation is not reentrant.  We disable it
     * with this little routine.  Note the routine only exists in libc_r.so,
     * not libc_r.a. This should be removed when (if) the loader is fixed!
     */
# ifdef _CMA_SHLIB_
     __init_shared_libs_for_threads();
# endif
#endif
    cma__g_tis_disable = cma_c_false;	/* Enable use of TIS */
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Define a set of functions that the OSF/1 reentrant libraries use for
 *	synchronization and which are called from the CMA$TIS_SHR image on
 *	VMS.  
 *
 *  FORMAL PARAMETERS:
 *
 *	mutexes, spin-locks, or condition variables, by reference
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
 *	-1 on memory allocation errors (errno is set), zero on success.  
 *	(Exceptions are raised for the remaining errors.)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */

/*
 * Fetch errno address
 */
static cma_t_errno *
cma___tis_get_errno_addr
#ifdef _CMA_PROTO_
	(void)
#else
	()
#endif
    {
    return &(cma__get_self_tcb ()->thd_errno);
    }


/*
 * Fetch vms-specific-errno address
 */
static cma_t_errno *
cma___tis_get_vmserrno_addr
#ifdef _CMA_PROTO_
	(void)
#else
	()
#endif
    {
#if _CMA_OS_ == _CMA__VMS
    return &(cma__get_self_tcb ()->thd_vmserrno);
#else
    return (cma_t_errno *)cma_c_null_ptr;
#endif
    }


/*
 * Set errno to a specific value
 */
void 
cma__tis_set_errno_value
#ifdef _CMA_PROTO_
	(cma_t_errno value)
#else
	(value)
	cma_t_errno value;
#endif
    {
    cma_t_errno **ptr;


    /*
     * Set the per-thread cell.
     */
    cma__get_self_tcb ()->thd_errno = value;

#if _CMA_OS_ == _CMA__VMS
    /*
     * Set the extern cells.
     */
    for (ptr = cma__g_errno_tbl; *ptr; ptr++)
	**ptr = value;
#endif
    }


/*
 * Set vms-specific-errno to a specific value
 */
static void 
cma___tis_set_vmserrno_value
#ifdef _CMA_PROTO_
	(cma_t_errno value)
#else
	(value)
	cma_t_errno value;
#endif
    {
    cma_t_errno **ptr;


#if _CMA_OS_ == _CMA__VMS
    /*
     * Set the per-thread cell.
     */
    cma__get_self_tcb ()->thd_vmserrno = value;

    /*
     * Set the extern cells.
     */
    for (ptr = cma__g_vmserrno_tbl; *ptr; ptr++)
	**ptr = value;
#endif
    }


/*
 * condition variable create
 */
static int
cma___tis_cond_create
#ifdef _CMA_PROTO_
	(cma_tis_cond_t *cond)
#else
	(cond)
	cma_tis_cond_t	*cond;
#endif
    {
    *cond = (cma_tis_cond_t)cma__get_cv (&cma__g_def_attr);

    if (*cond == (cma_tis_cond_t)0) {
	cma__set_errno (ENOMEM);
	return -1;
	}
    else {
	cma__obj_set_name (*cond, cma___g_tis_obj);
	cma__obj_set_owner (*cond, (cma_t_integer)cond);
	}

    cma___tis_dbg_obj_add(cond);

    return 0;
    }


/*
 * condition variable delete
 */
static int
cma___tis_cond_delete
#ifdef _CMA_PROTO_
	(cma_tis_cond_t *cond)
#else
	(cond)
	cma_tis_cond_t	*cond;
#endif
    {

    if (cond == (cma_tis_cond_t *)cma_c_null_ptr) {
	cma__set_errno (EINVAL);
	return -1;
	}

    if (*cond == (cma_tis_cond_t)cma_c_null_ptr) {
	cma__set_errno (EINVAL);
	return -1;
	}

    cma___tis_dbg_obj_search (cond, delete, (cma___g_dbg_tis_v[i] = (cma_tis_cond_t)0));
    cma__free_cv ((cma__t_int_cv *)*cond);
    *cond = (cma_tis_cond_t)cma_c_null_ptr;

    return 0;
    }


/*
 * condition variable broadcast
 */
static int
cma___tis_cond_broadcast
#ifdef _CMA_PROTO_
	(cma_tis_cond_t *cond)
#else
	(cond)
	cma_tis_cond_t	*cond;
#endif
    {
    cma___tis_dbg_obj_search (cond, broadcast, 0);
    cma__int_broadcast (*cond);
    return 0;
    }


/*
 * condition variable signal
 */
static int
cma___tis_cond_signal
#ifdef _CMA_PROTO_
	(cma_tis_cond_t *cond)
#else
	(cond)
	cma_tis_cond_t	*cond;
#endif
    {
    cma___tis_dbg_obj_search (cond, signal, 0);
    cma__int_signal (*cond);
    return 0;
    }


/*
 * condition variable wait
 */
static int
cma___tis_cond_wait
#ifdef _CMA_PROTO_
	(cma_tis_cond_t *cond, cma_tis_mutex_t *mutex)
#else
	(cond, mutex)
	cma_tis_cond_t	*cond;
	cma_tis_mutex_t	*mutex;
#endif
    {
    cma___tis_dbg_obj_search (cond, cv_wait, 0);
    cma___tis_dbg_obj_search (mutex, cv_wait, 0);
    cma__int_wait (
	    (cma__t_int_cv *)*cond,
	    (cma__t_int_mutex *)*mutex,
	    cma__get_self_tcb ());
    return 0;
    }

/*
 * mutex create
 */
static int
cma___tis_mutex_create
#ifdef _CMA_PROTO_
	(cma_tis_mutex_t *mutex)
#else
	(mutex)
	cma_tis_mutex_t	*mutex;
#endif
    {
    *mutex = (cma_tis_mutex_t)cma__get_mutex (&cma__g_def_attr);

    if (*mutex == (cma_tis_mutex_t)0) {
	cma__set_errno (ENOMEM);
	return -1;
	}
    else {
	cma__obj_set_name (*mutex, cma___g_tis_obj);
	cma__obj_set_owner (*mutex, (cma_t_integer)mutex);
	}

    cma___tis_dbg_obj_add(mutex);

    return 0;
    }

/*
 * mutex delete
 */
static int
cma___tis_mutex_delete
#ifdef _CMA_PROTO_
	(cma_tis_mutex_t *mutex)
#else
	(mutex)
	cma_tis_mutex_t	*mutex;
#endif
    {
    if (mutex == (cma_tis_mutex_t *)cma_c_null_ptr) {
	cma__set_errno (EINVAL);
	return -1;
	}

    if (*mutex == (cma_tis_mutex_t)cma_c_null_ptr) {
	cma__set_errno (EINVAL);
	return -1;
	}

    cma___tis_dbg_obj_search (mutex, delete, (cma___g_dbg_tis_v[i] = (cma_tis_mutex_t)0));

    cma__free_mutex ((cma__t_int_mutex *)*mutex);
    *mutex = (cma_tis_mutex_t)cma_c_null_ptr;

    return 0;
    }


/*
 * mutex lock
 */
static int
cma___tis_mutex_lock
#ifdef _CMA_PROTO_
	(cma_tis_mutex_t *mutex)
#else
	(mutex)
	cma_tis_mutex_t	*mutex;
#endif
    {

    if (!cma__g_tis_disable) {

#ifndef NDEBUG
	if (*mutex == (cma_tis_mutex_t)0) {
	    cma_t_boolean	state = cma__g_tis_disable;
	    cma_t_integer	stat;

	    /*
	     * Since the mutex "voted most likely to be accessed before TIS
	     * is initialized" (three years running) is the stdio mutex
	     * required by CMA_TRACE support, and creating a mutex may
	     * generate tracing, it's not an extraordinarily good idea to
	     * allow things to proceed upon their normal course. So we
	     * disable TIS temporarily while doing this dynamic creation.
	     */
	    cma__g_tis_disable = cma_c_true;
	    stat = cma___tis_mutex_create (mutex);
	    cma__g_tis_disable = state;

	    if (stat == -1)
		return -1;

	    }
	else
#endif
	    cma___tis_dbg_obj_search (mutex, lock, 0);

	cma__int_lock ((cma__t_int_mutex *)*mutex);
	}

    return 0;
    }


/*
 * mutex unlock
 */
static int
cma___tis_mutex_unlock
#ifdef _CMA_PROTO_
	(cma_tis_mutex_t *mutex)
#else
	(mutex)
	cma_tis_mutex_t	*mutex;
#endif
    {

    if (!cma__g_tis_disable) {
	cma___tis_dbg_obj_search (mutex, unlock, 0);
	cma__int_unlock ((cma__t_int_mutex *)*mutex);
	}

    return 0;
    }


/*
 * mutex try-lock
 */
static int
cma___tis_mutex_trylock
#ifdef _CMA_PROTO_
	(cma_tis_mutex_t *mutex)
#else
	(mutex)
	cma_tis_mutex_t	*mutex;
#endif
    {

    if (cma__g_tis_disable)
	return 1;			/* Pretend it's OK */
    else {

#ifndef NDEBUG
	if (*mutex == (cma_tis_mutex_t)0) {
	    cma_t_boolean	state = cma__g_tis_disable;
	    cma_t_integer	stat;

	    /*
	     * Since the mutex "voted most likely to be accessed before TIS
	     * is initialized" (three years running) is the stdio mutex
	     * required by CMA_TRACE support, and creating a mutex may
	     * generate tracing, it's not an extraordinarily good idea to
	     * allow things to proceed upon their normal course. So we
	     * disable TIS temporarily while doing this dynamic creation.
	     */
	    cma__g_tis_disable = cma_c_true;
	    stat = cma___tis_mutex_create (mutex);
	    cma__g_tis_disable = state;

	    if (stat == -1)
		return -1;

	    }
	else
#endif
	    cma___tis_dbg_obj_search (mutex, trylock, 0);

	return (cma__int_try_lock ((cma__t_int_mutex *)*mutex));
	}

    }

/*
 * spin-lock create
 */
static int
cma___tis_spin_create
#ifdef _CMA_PROTO_
	(cma__int_tis_spinlock_t	*spinner)
#else
	(spinner)
	cma__int_tis_spinlock_t		*spinner;
#endif
    {
    int	status;

    *spinner = cma__alloc_object (cma__t_atomic_bit);

    if ((cma_t_address)*spinner != cma_c_null_ptr) {
	cma__tac_clear (*spinner);
	status = 0;
	}
    else {
	status = -1;
	cma__set_errno (ENOMEM);
	}

    return status;
    }


/*
 * spin-lock delete
 */
static int
cma___tis_spin_delete
#ifdef _CMA_PROTO_
	(cma__int_tis_spinlock_t	*spinner)
#else
	(spinner)
	cma__int_tis_spinlock_t		*spinner;
#endif
    {

    if (spinner == (cma__int_tis_spinlock_t *)cma_c_null_ptr) {
	cma__set_errno (EINVAL);
	return -1;
	}

    if (*spinner == (cma__int_tis_spinlock_t)cma_c_null_ptr) {
	cma__set_errno (EINVAL);
	return -1;
	}

    cma__free_mem ((cma_t_address)*spinner);
    *spinner = (cma__int_tis_spinlock_t)cma_c_null_ptr;
    return 0;
    }


/*
 * spin-lock lock
 */
static int
cma___tis_spin_lock
#ifdef _CMA_PROTO_
	(cma__int_tis_spinlock_t	*spinner)
#else
	(spinner)
	cma__int_tis_spinlock_t		*spinner;
#endif
    {

    if (!cma__g_tis_disable) {
	cma__spinlock (*spinner);
	}

    return 0;
    }


/*
 * spin-lock unlock
 */
static int
cma___tis_spin_unlock
#ifdef _CMA_PROTO_
	(cma__int_tis_spinlock_t	*spinner)
#else
	(spinner)
	cma__int_tis_spinlock_t		*spinner;
#endif
    {

    if (!cma__g_tis_disable) {
	cma__spinunlock (*spinner);
	}

    return 0;
    }


/*
 * spin-lock try-lock
 */
static int
cma___tis_spin_trylock
#ifdef _CMA_PROTO_
	(cma__int_tis_spinlock_t	*spinner)
#else
	(spinner)
	cma__int_tis_spinlock_t		*spinner;
#endif
    {

    if (cma__g_tis_disable)
	return 0;
    else
	return cma__test_and_set (*spinner);

    }


/*
 * get thread ID
 */
static cma_tis_thread_t
cma___tis_thread_get_self
#ifdef _CMA_PROTO_
	(void)
#else
	()
#endif
    {
    /*
     * NOTE: this will work even on a uniprocessor build where
     * cma__get_self_tcb() is a macro.
     */
    return (cma_tis_thread_t)cma__get_self_tcb ();
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TIS_SUP.C */
/*  *24    3-JUN-1993 14:15:31 BUTENHOF "Dynamically create mutex on tis_lock under !NDEBUG" */
/*  *23    6-MAY-1993 19:07:15 BUTENHOF "Use TIS disable" */
/*  *22   16-APR-1993 13:06:31 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *21    1-APR-1993 14:33:35 BUTENHOF "Use formatting for TIS sync objects" */
/*  *20    1-MAR-1993 17:25:38 SCALES "Fix warnings on ALPHA" */
/*  *19    1-MAR-1993 09:48:57 KEANE "Fix cascading assertions; Fix seterrno() initialization" */
/*  *18   28-JAN-1993 14:42:16 BUTENHOF "Turn TIS locking off on bugcheck" */
/*  *17   25-JAN-1993 09:00:20 KEANE "Add final touches for shared libraries" */
/*  *16   10-DEC-1992 14:34:31 BUTENHOF "Reference _errno() in OSF/1 init" */
/*  *15   24-NOV-1992 01:53:10 SCALES "Add support for TIS multiple errno table" */
/*  *14   12-NOV-1992 11:55:03 SCALES "Set DEC C RTL reentrancy to multithread" */
/*  *13    5-NOV-1992 09:43:05 KEANE "Preallocate pool for libc_r initialization" */
/*  *12    2-SEP-1992 16:26:48 BUTENHOF "Change cma__wait" */
/*  *11   13-AUG-1992 14:44:25 BUTENHOF "Fix compile errors" */
/*  *10   12-AUG-1992 17:08:38 SCALES "Replace universal symbol data ref's with function calls" */
/*  *9     4-AUG-1992 11:05:01 BUTENHOF "Fix types for 64-bit" */
/*  *8    27-JUL-1992 18:01:24 SCALES "Fix OSF and MIPS problems" */
/*  *7    20-JUL-1992 16:10:04 CURTIN "Fix a macro" */
/*  *6    20-JUL-1992 16:00:23 SCALES "Add EVMS support" */
/*  *5    10-JUN-1992 13:20:44 BUTENHOF "Fix VAX ULTRIX compile errors" */
/*  *4     9-JUN-1992 12:11:00 BUTENHOF "Try the good 'ol shotgun debugging paradigm" */
/*  *3     9-JUN-1992 09:09:37 KEANE "Fix compile problem for pmax & alpha" */
/*  *2     3-JUN-1992 11:38:27 BUTENHOF "Update to latest version" */
/*  *1    25-MAY-1992 14:25:36 SCALES "CMA$TIS_SHR and libc_r support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TIS_SUP.C */
