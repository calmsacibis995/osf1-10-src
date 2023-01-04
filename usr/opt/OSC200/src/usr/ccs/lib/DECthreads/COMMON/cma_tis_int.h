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
 * @(#)$RCSfile: cma_tis_int.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1992/09/14 12:19:23 $
 */

/*
 *  FACILITY:
 *
 *	Thread-Independent Services, a subsidiary of DECthreads
 *
 *  ABSTRACT:
 *
 *	Internal definitions for CMA TIS services
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 * 
 *	9 July 1992
 *
 *  MODIFIED BY:
 *
 *	001	Webb Scales	31 July 1992
 *		Moved the jump table definitions to cma_tis_jt.h
 *		Added functions to retrieve the data that was available via
 *		universal symbols, and removed the universal symbols.
 */

#ifndef CMA_TIS_INT
# define CMA_TIS_INT

/*
 * Currently only OpenVMS Alpha actually uses the jump table definitions, 
 * however it won't hurt on VAX, so pull it in on either VMS platform to
 * get the function protos.
 */
# if defined(vms) || defined(__vms) || defined(VMS) || defined(__VMS) || defined(__vms__)
#  include <cma_tis_jt.h>
# endif


/*
 * Unpublished TIS data types
 */

typedef cma_tis_addr_t	cma_tis_spinlock_t;


/*
 * Unpublished TIS entry points
 */

# if defined(vms) || defined(__vms) || defined(VMS) || defined(__VMS) || defined(__vms__)
/*
 * On VMS, these routines must begin with "cma$" not "cma_", so use a 
 * macro to translate on those platforms.
 */
#  define cma_tis_spin_create		cma$tis_spin_create
#  define cma_tis_spin_delete		cma$tis_spin_delete
#  define cma_tis_spin_lock		cma$tis_spin_lock
#  define cma_tis_spin_trylock		cma$tis_spin_trylock
#  define cma_tis_spin_unlock		cma$tis_spin_unlock
#  define cma_tis_errno_set_addr	cma$tis_errno_set_addr
#  define cma_tis_vmserrno_set_addr	cma$tis_vmserrno_set_addr
#  define cma__tis_objects_exist	cma$$tis_objects_exist
#  define cma__tis_vector_get_bounds	cma$$tis_vector_get_bounds
# endif

extern int cma_tis_spin_create _TIS_PROTO_((cma_tis_spinlock_t *spinlock));
extern int cma_tis_spin_delete _TIS_PROTO_((cma_tis_spinlock_t *spinlock));
extern int cma_tis_spin_lock _TIS_PROTO_((cma_tis_spinlock_t *spinlock));
extern int cma_tis_spin_trylock _TIS_PROTO_((cma_tis_spinlock_t *spinlock));
extern int cma_tis_spin_unlock _TIS_PROTO_((cma_tis_spinlock_t *spinlock));

#endif
