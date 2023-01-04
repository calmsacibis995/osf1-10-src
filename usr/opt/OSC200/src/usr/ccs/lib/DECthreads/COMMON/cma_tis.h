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
 * @(#)$RCSfile: cma_tis.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/08/06 17:41:03 $
 */

/*
 *  FACILITY:
 *
 *	Thread-Independent Services, a subsidiary of DECthreads
 *
 *  ABSTRACT:
 *
 *	External definitions for CMA TIS services
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 * 
 *	29 April 1992
 *
 *  MODIFIED BY:
 *
 *	Webb Scales
 *	Dave Butenhof
 */
#ifndef CMA_TIS_INCLUDE
#define CMA_TIS_INCLUDE


/*
 * The following routines comprise the DECthreads "Thread Independent Services".
 * These routines provide the indicated functionality when used in a program
 * or application in which threads are present.  In the absence of threads,
 * these functions return immediately, imposing the minimum possible overhead
 * on their caller.
 *
 * TIS objects (e.g., mutexes) can be used in the same program or application
 * which uses DECthreads objects; however, TIS objects are referenced by
 * address instead while DECthreads objects are referenced by handle.  TIS 
 * objects, therefore, may not be interoperable (i.e., interchangeable) with 
 * DECthreads objects.
 */


/*
 * TIS data types.
 */

#if defined(__STDC__) || defined(VAXC) || defined(__mips)
# define _TIS_PROTO_(proto) proto
  typedef void	*cma_tis_addr_t;
#else
# define _TIS_PROTO_(proto) ()
  typedef char	*cma_tis_addr_t;
#endif

/*
 * TIS condition variable: the address of an internal condition variable object.
 */
typedef cma_tis_addr_t	cma_tis_cond_t;

/*
 * TIS mutex: the address of an internal mutex object.
 */
typedef cma_tis_addr_t	cma_tis_mutex_t;

/*
 * TIS thread-ID: the address of a internal thread object.
 */
typedef cma_tis_addr_t	cma_tis_thread_t;


#ifdef vms
  /*
   * On VMS, these routines must begin with "cma$" not "cma_", so use a 
   * macro to translate on those platforms.
   */
# define cma_tis_errno_get_addr		cma$tis_errno_get_addr
# define cma_tis_vmserrno_get_addr	cma$tis_vmserrno_get_addr
# define cma_tis_cond_create		cma$tis_cond_create
# define cma_tis_cond_delete		cma$tis_cond_delete
# define cma_tis_cond_broadcast		cma$tis_cond_broadcast
# define cma_tis_cond_signal		cma$tis_cond_signal
# define cma_tis_cond_wait		cma$tis_cond_wait
# define cma_tis_mutex_create		cma$tis_mutex_create
# define cma_tis_mutex_delete		cma$tis_mutex_delete
# define cma_tis_mutex_lock		cma$tis_mutex_lock
# define cma_tis_mutex_trylock		cma$tis_mutex_trylock
# define cma_tis_mutex_unlock		cma$tis_mutex_unlock
# define cma_tis_thread_get_self	cma$tis_thread_get_self
#endif

/*
 * TIS functions.
 */

/*
 * Get the address of errno 
 */
extern int *cma_tis_errno_get_addr _TIS_PROTO_ ((void));

/*
 * Get the address of the VMS-specific errno
 */
extern int *cma_tis_vmserrno_get_addr _TIS_PROTO_ ((void));

/*
 * Create a TIS condition variable.
 *
 * "cond" is a pointer to an abstract condition variable, passed by reference, 
 * which receives the address of the new condition variable object.
 */
extern int cma_tis_cond_create _TIS_PROTO_ ((cma_tis_cond_t *cond));

/*
 * Delete a TIS condition variable
 *
 * "cond" is the address of the abstract condition variable, passed by 
 * reference, which is to be deleted.
 */
extern int cma_tis_cond_delete _TIS_PROTO_ ((cma_tis_cond_t *cond));

/*
 * Broadcast on a TIS condition variable
 *
 * "cond" is the address of the abstract condition variable, passed by 
 * reference, on which to broadcast.
 */
extern int cma_tis_cond_broadcast _TIS_PROTO_ ((cma_tis_cond_t *cond));

/*
 * Signal a TIS condition variable
 *
 * "cond" is the address of the abstract condition variable, passed by 
 * reference, on which to signal.
 */
extern int cma_tis_cond_signal _TIS_PROTO_ ((cma_tis_cond_t *cond));

/*
 * Wait on a TIS condition variable
 *
 * "cond" is the address of the abstract condition variable, passed by 
 * reference, on which to wait.
 */
extern int cma_tis_cond_wait _TIS_PROTO_ ((cma_tis_cond_t *cond, 
					   cma_tis_mutex_t *mutex));

/*
 * Create a TIS mutex
 *
 * "mutex" is a pointer to an abstract mutex, passed by reference, which 
 * receives the address of the new mutex object.
 */
extern int cma_tis_mutex_create _TIS_PROTO_ ((cma_tis_mutex_t *mutex));

/*
 * Delete a TIS mutex
 *
 * "mutex" is the address of the abstract mutex, passed by reference, which is
 * to be deleted.
 */
extern int cma_tis_mutex_delete _TIS_PROTO_ ((cma_tis_mutex_t *mutex));

/*
 * Lock a TIS mutex
 *
 * "mutex" is the address of the abstract mutex, passed by reference, which is
 * to be locked.
 */
extern int cma_tis_mutex_lock _TIS_PROTO_ ((cma_tis_mutex_t *mutex));

/*
 * Non-blocking TIS mutex lock
 *
 * "mutex" is the address of the abstract mutex, passed by reference, which is
 * to be locked.
 *
 * The function returns a boolean value indicating if the mutex was 
 * successfully locked.
 */
extern int cma_tis_mutex_trylock _TIS_PROTO_ ((cma_tis_mutex_t *mutex));

/*
 * Unlock a TIS mutex
 *
 * "mutex" is the address of the abstract mutex, passed by reference, which is
 * to be unlocked.
 */
extern int cma_tis_mutex_unlock _TIS_PROTO_ ((cma_tis_mutex_t *mutex));

/*
 * Get current thread "ID".  (Returns zero if there are no threads.)
 */
extern cma_tis_thread_t cma_tis_thread_get_self _TIS_PROTO_ ((void));
#endif
