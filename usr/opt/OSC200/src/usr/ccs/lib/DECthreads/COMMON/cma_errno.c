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
 * @(#)$RCSfile: cma_errno.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/08/18 14:47:17 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Services to manage per-thread UNIX(R) errno
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	12 February 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	12 April 1991
 *		Adapt for OSF/1, which has latent support for per-thread
 *		errno in the standard errno.h, but requires help from the
 *		thread library (us) to implement it.
 *	002	Paul Curtin	18 November 1991
 *		Made cma_errno return value volatile on Alpha platform.
 *	003	Dave Butenhof	27 November 1991
 *		Add code for Alpha DEC C.
 *	004	Webb Scales	25 February 1992
 *		Reorganized errno handling, added a vms-specific routine
 *		for fetching the vms-specific errno.
 *	005	Dave Butenhof	24 July 1992
 *		Errno type is "cma_t_errno" now -- change cma_errno() type
 *		appropriately.
 *	006	Webb Scales	24 November 1992
 *		Add typecast to _errno routine return value.
 *	007	Brian Keane	23 June 1993
 *		Cleanup ANSI namespace for the libc routines we preempt.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_int_errno.h>

/*
 * GLOBAL DATA
 */

/*
 * LOCAL DATA
 */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Return address of per-thread errno variable
 *
 *  FORMAL PARAMETERS:
 *
 *	none
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
 *	address of errno (int *)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_errno *
cma_errno
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_ERRNO_TYPE_ != _CMA__INTERN_FUNC
    /*
     * This should really be a per-thread errno cell in the TCB.  However, on
     * uniprocessor implementations we'd like to coexist with old libraries
     * that don't know about threads, without a lot of complication. While
     * it's possible to make a per-thread errno coexist with the global, it
     * can't be done without a set of constructs like "geterrno" and
     * "seterrno".
     *
     * Therefore, errno will be treated like a machine register; it will be
     * context switched.  This is WRONG, as it cannot work on a true
     * multiprocessor; however, it's good enough for a uniprocessor
     * implementation.
     */
    return (cma_t_errno *)&errno;
#else
    return &(cma__get_self_tcb ()->thd_errno);
#endif
    }


#if _CMA_OS_ == _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Return address of per-thread, VMS-specific errno variable
 *
 *  FORMAL PARAMETERS:
 *
 *	none
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
 *	address of vms-errno (int *)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_integer *
cma_vms_errno
# ifdef _CMA_PROTO_
	(void)
# else	/* no prototypes */
	()
# endif	/* prototype */
    {
# if _CMA_ERRNO_TYPE_ != _CMA__INTERN_FUNC
    /*
     * This should really be a per-thread errno cell in the TCB.  However, on
     * uniprocessor implementations we'd like to coexist with old libraries
     * that don't know about threads, without a lot of complication. While
     * it's possible to make a per-thread errno coexist with the global, it
     * can't be done without a set of constructs like "geterrno" and
     * "seterrno".
     *
     * Therefore, errno will be treated like a machine register; it will be
     * context switched.  This is WRONG, as it cannot work on a true
     * multiprocessor; however, it's good enough for a uniprocessor
     * implementation.
     */
    return (cma_t_integer *)&vaxc$errno;
# else
    return &(cma__get_self_tcb ()->thd_vmserrno);
# endif
    }
#endif

#if _CMA_OSIMPL_ == _CMA__OS_OSF
/*
 * Declare the functions used by the runtime libraries for errno. Note that
 * the seterrno function also sets the global errno; this is useless for
 * multithreaded applications, but allows single threaded applications that
 * somehow pull in DECthreads (but don't actually create threads) to continue
 * to operate "the old way".
 */
#pragma weak geterrno = __geterrno
int
__geterrno (void)
    {
    return (cma__get_errno ());
    }

#pragma weak seterrno = __seterrno
void
__seterrno (int value)
    {
    cma__set_errno (value);

	/*
	 * Open a new block so that we can position the "undef" below after
	 * the "set-errno" above, to keep it from interfering.
	 */
	{
# undef errno
	extern int	errno;

	errno = value;
	}
    }

int *
_errno (void)
    {
    return (int *)(&(cma__get_self_tcb ()->thd_errno));
    }
#endif					/* _CMA__OS_OSF */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ERRNO.C */
/*  *10   29-JUN-1993 17:28:06 KEANE "Fix libc replacement routines for ANSI name compliance" */
/*  *9    24-NOV-1992 12:19:59 SCALES "Add typecast" */
/*  *8    24-JUL-1992 15:00:31 BUTENHOF "Change type of cma_errno" */
/*  *7    26-FEB-1992 19:15:08 SCALES "Integrate with errno image" */
/*  *6    27-NOV-1991 11:03:30 BUTENHOF "Use geterrno" */
/*  *5    18-NOV-1991 11:06:36 CURTIN "Made cma_errno return value volatile on Alpha platform" */
/*  *4    14-OCT-1991 13:38:44 BUTENHOF "Modify for uniprocessor OSF/1" */
/*  *3    10-JUN-1991 18:21:39 SCALES "Add sccs headers for Ultrix" */
/*  *2    12-APR-1991 23:35:43 BUTENHOF "Change errno access for OSF/1" */
/*  *1    12-DEC-1990 21:45:11 BUTENHOF "Per-thread errno" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ERRNO.C */
