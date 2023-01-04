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
 * @(#)$RCSfile: kd_thread_support.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/13 18:57:50 $
 */
/*
 *  This file contains data structure declaration for device driver
 *  kernel thread startup support.
 */
#ifndef _KD_THREAD_SUPPORT_INCL_
#define _KD_THREAD_SUPPORT_INCL_

/*
 *              NOTICE NOTICE NOTICE
 * You must place the lock in  kern/lockinfo.c
 * May be in data/lockinfo.c later on
 */

/*
 * One single LOCK structure for the thread list is all that is
 * needed.
 */
typedef struct kd_lock_thread {
        simple_lock_data_t      kd_lock;
        /* lock_info            kd_lock_info; */
} KD_LOCK_THREAD;

/*
 * LOCK INIT macro
 */
#define KD_LOCK_THREAD_LIST_INIT()                              \
{                                                               \
        simple_lock_init( &kd_lock_thread.kd_lock);             \
        /*                                                      \
         * Remove lock_init() and uncomment out next            \
         * usimple_lock_setup( &kd_lock_thread.kd_lock,         \
         * &kd_lock_thread.kd_lock_info)                        \
         */                                                     \
}

/* 
 * LOCK AND UNLOCK macros
 */
#define KD_LOCK_THREAD_LIST()                                   \
{                                                               \
        /* uncomment for SMP                                    \
         * simple_lock( &kd_lock_thread.kd_lock);               \
         */                                                     \
}

#define KD_UNLOCK_THREAD_LIST()                         	\
{                                                               \
        /* uncomment for SMP                                    \
         * simple_unlock( &kd_lock_thread.kd_lock);     	\
         */                                                     \
}

typedef struct kd_thread_list {
        queue_head_t kd_start_threads;  /* List of threads to be started */
} KD_THREAD_LIST;


/*
 * kd_flags definitions
 */
#define KD_INITIALIZED          0x00000001      /* Structure inited     */
#define KD_STARTED              0x00000002      /* Call from init_main done */


/*
 *
 */

typedef struct kd_thread_elt {
        KD_THREAD_LIST kd_queue;
        int (*dt_func)();
        caddr_t dt_arg;
} KD_THREAD_ELT;

#endif /* _KD_THREAD_SUPPORT_INCL_ */
