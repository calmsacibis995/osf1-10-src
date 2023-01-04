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
static char	*sccsid = "@(#)$RCSfile: libc_locks.c,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1993/10/05 21:21:10 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *  This file contains the declarations of all locks used in libc.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak ldr_declare_lock_functions = __ldr_declare_lock_functions
#pragma weak libc_locks_init = __libc_locks_init
#pragma weak libc_locks_reinit = __libc_locks_reinit
#endif
#endif
#include "rec_mutex.h"
#include <stdio.h>
#include "glue.h"

extern void  __sia_init_thread_mutexes(); 
extern void  __sia_reinit_thread_mutexes(); 

extern FILE _iob[_NIOBRW];

/* Locks for stdio.  These are the iob locks */
struct rec_mutex _iobptr_rmutex;
struct rec_mutex _stdio_buf_rmutex[3];
struct rec_mutex _prnt_rmutex;


/* Locks for compat-4.1 */
struct rec_mutex _rand_rmutex;

/* Locks for compat-sys5 */
struct  rec_mutex _clock_rmutex;
struct  rec_mutex _tmpnam_rmutex;
struct 	rec_mutex _tempnam_rmutex;

/* Locks for gen */
struct	rec_mutex _ctime_rmutex;
struct	rec_mutex _domain_rmutex;
struct	rec_mutex _fstab_rmutex;
struct	rec_mutex _getttyent_rmutex;
struct	rec_mutex _getusershell_rmutex;
struct  rec_mutex _group_rmutex;
struct	rec_mutex _utmp_rmutex;
struct  rec_mutex _passwd_rmutex;
struct	rec_mutex _random_rmutex;
struct	rec_mutex _environ_rmutex;
struct	rec_mutex _resolv_rmutex;
struct	rec_mutex _nis_rmutex;

/* Locks for n16/gen */
struct	rec_mutex _abort_rmutex;

/* locks for internationalization */
struct rec_mutex _locale_rmutex;
struct rec_mutex _catalog_rmutex;

struct rec_mutex _exec_rmutex;
struct rec_mutex _exit_rmutex;
struct rec_mutex _nice_rmutex;
struct rec_mutex _alarm_rmutex;

struct rec_mutex _nanotimer_rmutex;

struct rec_mutex _popen_rmutex;
struct rec_mutex _ldr_rmutex;
struct rec_mutex _brk_rmutex;
struct rec_mutex _atof_rmutex;

/* locks for exception handling */
struct rec_mutex _exc_cr_mutex;
struct rec_mutex _exc_write_mutex;
struct rec_mutex _exc_read_mutex;
struct rec_mutex _exc_read_access_mutex;

void
libc_locks_init()
{
	/* Locks for stdio.  These are the iob locks */
	rec_mutex_init(&_iobptr_rmutex);
        rec_mutex_init(&_prnt_rmutex);
	rec_mutex_init(&_stdio_buf_rmutex[0]);

	rec_mutex_init(&_stdio_buf_rmutex[1]);
	rec_mutex_init(&_stdio_buf_rmutex[2]);

	/* Locks for compat-4.1 */
	rec_mutex_init(&_rand_rmutex);

	/* Locks for compat-sys5 */
	rec_mutex_init(&_clock_rmutex);
	rec_mutex_init(&_tmpnam_rmutex);
	rec_mutex_init(&_tempnam_rmutex);

	/* Locks for gen */
	rec_mutex_init(&_domain_rmutex);
	rec_mutex_init(&_ctime_rmutex);
	rec_mutex_init(&_fstab_rmutex);
	rec_mutex_init(&_getttyent_rmutex);
	rec_mutex_init(&_getusershell_rmutex);
	rec_mutex_init(&_group_rmutex);
	rec_mutex_init(&_utmp_rmutex);
	rec_mutex_init(&_passwd_rmutex);
	rec_mutex_init(&_random_rmutex);
	rec_mutex_init(&_environ_rmutex);
	rec_mutex_init(&_resolv_rmutex);
	rec_mutex_init(&_nis_rmutex);

	/* Locks for n16/gen */
	rec_mutex_init(&_abort_rmutex);

	/* lock for interationalization */
	rec_mutex_init(&_locale_rmutex);
	rec_mutex_init(&_catalog_rmutex);

	rec_mutex_init(&_exec_rmutex);
	rec_mutex_init(&_exit_rmutex);
	rec_mutex_init(&_nice_rmutex);
	rec_mutex_init(&_alarm_rmutex);

	rec_mutex_init(&_nanotimer_rmutex);
	rec_mutex_init(&_popen_rmutex);

        /*
         * The loader is not re-entrant high level
         * lock for loader routines.
         */
	rec_mutex_init(&_ldr_rmutex);

       /* 
        * sbrk and brk mutex
        */
        rec_mutex_init(&_brk_rmutex);
	/*
	 * atof mutex
	 */
	rec_mutex_init(&_atof_rmutex);

       /*
	* exception handling mutexes
	*/
	rec_mutex_init(&_exc_cr_mutex);
	rec_mutex_init(&_exc_write_mutex);
	rec_mutex_init(&_exc_read_mutex);
	rec_mutex_init(&_exc_read_access_mutex);


       /*  
        * Fill the lock fields in 
        * the _iob data structures.
        */

        _iob[0]._lock = &_stdio_buf_rmutex[0];
        _iob[1]._lock = &_stdio_buf_rmutex[1];
        _iob[2]._lock = &_stdio_buf_rmutex[2];

        /*
         * Initialize locks used by sia
         */ 
        __sia_init_thread_mutexes(); 
}

void
libc_locks_reinit()
{
	/* Locks for stdio.  These are the iob locks */
	rec_mutex_reinit(&_iobptr_rmutex);
	rec_mutex_reinit(&_prnt_rmutex);
	rec_mutex_reinit(&_stdio_buf_rmutex[0]);

	rec_mutex_reinit(&_stdio_buf_rmutex[1]);
	rec_mutex_reinit(&_stdio_buf_rmutex[2]);

	/* Locks for compat-4.1 */
	rec_mutex_reinit(&_rand_rmutex);

	/* Locks for compat-sys5 */
	rec_mutex_reinit(&_clock_rmutex);
	rec_mutex_reinit(&_tmpnam_rmutex);
	rec_mutex_reinit(&_tempnam_rmutex);

	/* Locks for gen */
	rec_mutex_reinit(&_ctime_rmutex);
	rec_mutex_reinit(&_fstab_rmutex);
	rec_mutex_reinit(&_getttyent_rmutex);
	rec_mutex_reinit(&_getusershell_rmutex);
	rec_mutex_reinit(&_group_rmutex);
	rec_mutex_reinit(&_utmp_rmutex);
	rec_mutex_reinit(&_passwd_rmutex);
	rec_mutex_reinit(&_random_rmutex);
	rec_mutex_reinit(&_environ_rmutex);
	rec_mutex_reinit(&_resolv_rmutex);
	rec_mutex_reinit(&_nis_rmutex);

	/* Locks for n16/gen */
	rec_mutex_reinit(&_abort_rmutex);

	/* lock for interationalization */
	rec_mutex_reinit(&_locale_rmutex);
	rec_mutex_reinit(&_catalog_rmutex);

	rec_mutex_reinit(&_exec_rmutex);
	rec_mutex_reinit(&_exit_rmutex);
	rec_mutex_reinit(&_nice_rmutex);
	rec_mutex_reinit(&_alarm_rmutex);

	rec_mutex_reinit(&_nanotimer_rmutex);
	rec_mutex_reinit(&_popen_rmutex);

        /*
         * The loader is not re-entrant high level
         * lock for loader routines.
         */
	rec_mutex_reinit(&_ldr_rmutex);

       /* 
        * sbrk and brk mutex
        */
        rec_mutex_reinit(&_brk_rmutex);

       /*
	* exception handling mutexes
	*/
	rec_mutex_reinit(&_exc_cr_mutex);
	rec_mutex_reinit(&_exc_write_mutex);
	rec_mutex_reinit(&_exc_read_mutex);
	rec_mutex_reinit(&_exc_read_access_mutex);


       /*  
        * re-init stdio locks.
        * 
        */

        rec_mutex_reinit(&_stdio_buf_rmutex[0]);
        rec_mutex_reinit(&_stdio_buf_rmutex[1]);
        rec_mutex_reinit(&_stdio_buf_rmutex[2]);
        __sia_reinit_thread_mutexes(); 
}

void ldr_declare_lock_functions()
{

}
