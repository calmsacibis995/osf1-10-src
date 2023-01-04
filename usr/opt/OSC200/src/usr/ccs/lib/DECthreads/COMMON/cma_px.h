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
 *	@(#)$RCSfile: cma_px.h,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/08/06 17:38:50 $
 */

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for POSIX wrapper routines
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	18 May 1990
 *
 *  MODIFIED BY:
 *
 *	Dave Butenhof
 *	Paul Curtin
 *	Webb Scales
 */


#ifndef CMA_PX
#define CMA_PX

/*
 *  INCLUDE FILES
 */

/*
 * CONSTANTS AND MACROS
 */

#if !_CMA_REENTRANT_CLIB_
# ifndef _CMA_NOWRAPPERS_
#  if !_CMA_THREAD_IS_VP_
#   define sigaction cma_sigaction   
#  endif
# endif
#endif

/*
 * TYPEDEFS
 */

#if _CMA_OSIMPL_ == _CMA__OS_OSF
# include <sys/timers.h>
#else
# ifndef _TIMESPEC_T_
# define _TIMESPEC_T_
typedef struct timespec {
    unsigned long	tv_sec;		/* seconds */
    long		tv_nsec;	/* and nanoseconds */
    } timespec_t;
# endif
#endif

/*
 * INTERFACES
 */

#endif  /* CMA_PX */
