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
 *	@(#)$RCSfile: cma_sigwait.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/08/06 17:39:45 $
 */

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for the CMA implementation of POSIX sigwait routine
 *
 *  AUTHORS:
 *
 *	Paul Curtin
 *
 *  CREATION DATE:
 *
 *	10 December 1990
 *
 *  MODIFIED BY:
 *
 *	Dave Butenhof
 *	Paul Curtin
 */


#ifndef CMA_SIGWAIT
#define CMA_SIGWAIT

/*
 *  INCLUDE FILES
 */
#include <signal.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * Note:  sigwait is currently only defined in the POSIX spec and nowhere 
 *	else (ie, it is not in Section 2 of the Unix Manual, it is not part of
 *	the C RTL, and it is not part of the CMA specification) so it is an 
 *	orphan.  It is documented here until it is given its rightful place.
 */
# if !defined(_CMA_NOWRAPPERS_) && (_CMA_HARDWARE_ != _CMA__HPPA)
#  define sigwait cma_sigwait
# endif

/*
 * TYPEDEFS
 */

# if _CMA_OS_ == _CMA__VMS
#  ifndef _SIGSET_T_
#  define _SIGSET_T_
typedef	int	sigset_t;		/* For sigwait */
#  endif
# endif

/*
 *  GLOBAL DATA
 */

/*
 *  PROTOTYPES
 */
# if _CMA_OS_ != _CMA__VMS
#  if _CMA_HARDWARE_ != _CMA__HPPA
extern int
cma_sigwait _CMA_PROTOTYPE_ ((
        sigset_t    *set));
#  else
extern cma_t_integer
sigwait _CMA_PROTOTYPE_ ((
        sigset_t    *set));
#  endif
# endif

#endif
