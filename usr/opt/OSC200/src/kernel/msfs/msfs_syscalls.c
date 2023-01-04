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
static char *rcsid = "@(#)$RCSfile: msfs_syscalls.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/05/14 19:23:00 $";
#endif

/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1992                                  *
 *                                                                          *
 *      This is an unpublished work which was created in the indicated      *
 *      year, which contains confidential and secret information,  and      *
 *      which is protected under the copyright laws.  The existence of      *
 *      the copyright notice is not to be construed as an admission or      *
 *      presumption that publication has occurred. Reverse engineering      *
 *      and unauthorized copying is strictly prohibited.   All  rights      *
 *      reserved.                                                           *
 *                                                                          *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *      MegaSafe Storage System
 *
 * Abstract:
 *
 *      MegaSafe system call interface.
 *
 * Author:
 *
 *      Pete Stoppani
 *
 * Date:
 *
 *      Tue Jul 23 15:32:49 1991
 *
 * Revision History:
 *
 *      See end of file.
 */


#if     MACH
#include <mach_nbc.h>
#endif
#include <mach_ldebug.h>
#include <rt_preempt.h>

#include <sys/secdefines.h>
#if     SEC_BASE
#include <sys/security.h>
#endif
#if     SEC_ARCH
#include <sys/secpolicy.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/macro_help.h>
#include <sys/lock_types.h>
#if     MACH
#include <kern/zalloc.h>
#include <kern/assert.h>
#else
#include <sys/malloc.h>
#endif


#if     UNIX_LOCKS
#include <sys/ucred.h>
#endif


/*
 ** msfsSyscallp
 *
 * This is a function pointer to the MegaSafe system call handler.  It 
 * exists so that we can have msfs_syscall() in the kernel and have no
 * real syscall (as is the case when the customer has not installed
 * MegaSafe).  When MegaSafe is installed it initializes the syscall
 * function pointer to point to the syscall handler.
 */

int
(* MsfsSyscallp)( 
    int     opType,
    void    *parmBuf,
    int     parmBufLen
    ) = NULL;

/*
 * msfs_syscall
 *
 * Provides a kernel interface to the MegaSafe system call handler.
 *
 * Returns zero if successful, EINVAL if not.  The returned value
 * is placed into the thread's errno by the syscall mechanism.
 *
 * If MegaSafe is not installed/initialized then ENOSYS is returned.
 */

int
msfs_syscall(
    struct proc *p,     /* in - ?? */
    void *args,         /* in - pointer system call arguments */
    int *retval         /* out - system call return value */
    )
{
    /* 
     * Define a pointer to a struct that contains the user args 
     */
    register struct args {
        int  opType;
        void *parmBuf;
        int  parmBufLen;
    } *uap = (struct args *) args;  /* User Args Pointer */

    if (MsfsSyscallp == NULL) {
        /*
         ** MegaSafe is not installed/initialized.
         */
        *retval = -1;
        return( ENOSYS );
    } else {

        /*
         ** Call the MegaSafe syscall handler.  Note that we never set
         ** errno when MegaSafe is installed.  The syscall return value
         ** is either zero (EOK) or an errno, or a MegaSafe status.  We
         ** don't use errno to indicate an error; it is too limited.
         */
        *retval = MsfsSyscallp( uap->opType, uap->parmBuf, uap->parmBufLen );
        return( 0 );
    }
}

/* end msfs_syscall.c */
