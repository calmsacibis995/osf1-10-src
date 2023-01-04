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
 *	@(#)$RCSfile: cma_int_errno.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:32:05 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	Header file for internal DECthreads errno functions
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	12 April 1991
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	30 October 1991
 *		OSF DCE wants to use DECthreads on OSF/1 without Mach threads
 *		and without libc_r.a. The _errno() function (and others)
 *		shouldn't be declared under these circumstances, but are
 *		currently compiled whenever built on an OSF/1-based system.
 *	002	Dave Butenhof	27 November 1991
 *		Under DEC C, use the library's get/set errno access functions
 *		rather than global errno.
 *	003	Webb Scales	25 February 1992
 *		Reorganized errno handling.
 *	004	Webb Scales	20 April 1992
 *		Temporarily returned errno dependency to VAXCRTL for VAX VMS.
 *	005	Webb Scales	19 November 1992
 *		Provided for TIS list of errno cells on VMS.
 *	006	Webb Scales	24 November 1992
 *		Unconditionalize tis-set-errno-value prototype.
 *	007	Webb Scales	 9 December 1992
 *		Return to VAXCRTL dependency for errno on OpenVMS VAX.
 *	008	Webb Scales	 1 March 1993
 *		Use internal function on EVMS.
 */

#ifndef CMA_INT_ERRNO
#define CMA_INT_ERRNO

/*
 *  INCLUDE FILES
 */
#include <errno.h>
#include <cma_defs.h>
#include <cma_tcb_defs.h>
#include <cma_stack.h>

#ifndef EDEADLK
# define EDEADLK	EWOULDBLOCK
#endif

/*
 * CONSTANTS AND MACROS
 */

/*
 * This symbol determines what sort of errno-processing DECthreads does:
 *	User-program references to "errno" result in:
 *	    STATIC_CELL -   a direct reference to a static cell
 *	    EXTERN_FUNC -   a call to a function which references a static cell
 *	    INTERN_FUNC -   a call to cma_errno() which references the TCB
 */
#define _CMA__STATIC_CELL	1
#define _CMA__EXTERN_FUNC	2
#define _CMA__INTERN_FUNC	3

/*
 * Errno functions
 *
 * FIX-ME:  _CMA_ERRNO_TYPE_ should be _CMA__INTERN_FUNC on VAX VMS starting
 *          with V6.0 (and should be _CMA__STATIC_CELL for earlier versions).
 */

#ifndef _CMA_ERRNO_TYPE_
/*
 * This should be used for BLADE/DELTA code:
 * # if 0
 * This should be used OpenVMS AXP V1.0/OpenVMS VAX pre-V6.0 code:
 * # if ((_CMA_OS_ == _CMA__VMS) && (_CMA_HARDWARE_ == _CMA__ALPHA))
 */
# if 0
#  define _CMA_ERRNO_TYPE_	_CMA__EXTERN_FUNC
# else
/*
 * This should be used for BLADE/DELTA code:
 * #  if _CMA_REENTRANT_CLIB_ || (_CMA_OS_ == _CMA__VMS)
 * The below is for OpenVMS AXP V1.0/OpenVMS VAX pre-V6.0 code:
 * #  if _CMA_REENTRANT_CLIB_
 */
#  if _CMA_REENTRANT_CLIB_ || (_CMA_PLATFORM_ == _CMA__ALPHA_VMS)
#   define _CMA_ERRNO_TYPE_	_CMA__INTERN_FUNC
#  else
#   define _CMA_ERRNO_TYPE_	_CMA__STATIC_CELL
#  endif
# endif
#endif


#if _CMA_ERRNO_TYPE_ == _CMA__INTERN_FUNC
# define cma__get_errno()		(cma__get_self_tcb ()->thd_errno)
# if _CMA_OS_ == _CMA__VMS
#  define cma__set_errno(value)	\
	(*cma__g_errno_tbl \
	    ? (cma__tis_set_errno_value (value), 0) \
	    : (cma__get_self_tcb ()->thd_errno = value, 0))
# else
#  define cma__set_errno(value)	(cma__get_self_tcb ()->thd_errno = value)
# endif
# define cma__get_thderrno(thd)		(thd->thd_errno)
# define cma__set_thderrno(thd,value)	(thd->thd_errno = value)
#else
# if (_CMA_ERRNO_TYPE_ == _CMA__STATIC_CELL)
#  undef errno
# endif
# define cma__get_errno()		(errno)
# define cma__set_errno(value)		(errno = value)
# define cma__get_thderrno(thd)		(errno)
# define cma__set_thderrno(thd,value)	(errno = value)
#endif


/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */
#if _CMA_OS_ == _CMA__VMS
extern cma_t_errno **cma__g_errno_tbl;
extern cma_t_errno **cma__g_vmserrno_tbl;
#endif

/*
 * INTERNAL INTERFACES
 */

extern void 
cma__tis_set_errno_value _CMA_PROTOTYPE_ ((cma_t_errno value));

#if (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
extern int
geterrno _CMA_PROTOTYPE_ ((void));

extern void
seterrno _CMA_PROTOTYPE_ ((int value));

extern int *
_errno _CMA_PROTOTYPE_ ((void));
#endif

#endif
