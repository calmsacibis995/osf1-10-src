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
 *       @(#)$RCSfile: exc.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/01/28 17:32:29 $
 */
#ifndef	_EXC_H_
#define	_EXC_H_

/* Module exc */

#include <mach/kern_return.h>
#if	(defined(__STDC__) || defined(c_plusplus)) || defined(LINTLIBRARY)
#include <mach/port.h>
#include <mach/message.h>
#endif

#ifndef	mig_external
#define mig_external extern
#endif

mig_external void init_exc
#if	(defined(__STDC__) || defined(c_plusplus))
    (port_t rep_port);
#else
    ();
#endif
#include <mach/std_types.h>

/* Routine exception_raise */
mig_external kern_return_t exception_raise
#if	defined(LINTLIBRARY)
    (exception_port, clear_port, thread, task, exception, code, subcode)
	port_t exception_port;
	port_t clear_port;
	port_t thread;
	port_t task;
	int exception;
	int code;
	int subcode;
{ return exception_raise(exception_port, clear_port, thread, task, exception, code, subcode); }
#else
#if	(defined(__STDC__) || defined(c_plusplus))
(
	port_t exception_port,
	port_t clear_port,
	port_t thread,
	port_t task,
	int exception,
	int code,
	int subcode
);
#else
    ();
#endif
#endif

#endif	/* _exc */
