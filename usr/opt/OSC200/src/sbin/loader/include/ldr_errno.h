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
 *	@(#)$RCSfile: ldr_errno.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:36:47 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_errno.h
 * error number declarations for loader
 *
 * OSF/1 Release 1.0
 */

/* Loader errors are divided into two groups:
 *  1) those corresponding 1-1 with errno's, so that errno's coming in
 *     from system calls (eg. ENOENT, EIO) can be translated into
 *     loader errors and back again.  These have the same global meaning
 *     as the errnos;
 *  2) internal loader errors, possibly many to 1 with errno's.
 *
 * This is handled as follows:
 *
 *  - Loader errnos from (-1) to (-SYSTEM_ERRNO_MAX) are resesrved for 1-1
 *    translation to and from system errno's.  They must have the same
 *    global meaning as system errno's.
 *  - Loader errno's from (-SYSTEM_ERRNO_MAX-1) to (-BIG) are internal loader
 *    errors.  No system errno ever translates to one of these statuses; they
 *    are all generated internally.  Each internal status translates to a
 *    single system errno, via a translation table.
 */

#ifndef	_H_LDR_ERRNO
#define	_H_LDR_ERRNO

#include <errno.h>

#define	LDR_SUCCESS	0
#define	LDR_EPERM	ldr_errno_to_status(EPERM)
#define	LDR_ENOENT	ldr_errno_to_status(ENOENT)
#define	LDR_ESRCH	ldr_errno_to_status(ESRCH)
#define	LDR_EINTR	ldr_errno_to_status(EINTR)
#define	LDR_EIO		ldr_errno_to_status(EIO)
#define	LDR_ENXIO	ldr_errno_to_status(ENXIO)
#define	LDR_E2BIG	ldr_errno_to_status(E2BIG)
#define	LDR_ENOEXEC	ldr_errno_to_status(ENOEXEC)
#define	LDR_EBADF	ldr_errno_to_status(EBADF)
#define	LDR_ECHILD	ldr_errno_to_status(ECHILD)
#define	LDR_EAGAIN	ldr_errno_to_status(EAGAIN)
#define	LDR_ENOMEM	ldr_errno_to_status(ENOMEM)
#define	LDR_EACCES	ldr_errno_to_status(EACCES)
#define	LDR_EFAULT	ldr_errno_to_status(EFAULT)
#define	LDR_ENOTBLK	ldr_errno_to_status(ENOTBLK)
#define	LDR_EBUSY	ldr_errno_to_status(EBUSY)
#define	LDR_EEXIST	ldr_errno_to_status(EEXIST)
#define	LDR_EXDEV	ldr_errno_to_status(EXDEV)
#define	LDR_ENODEV	ldr_errno_to_status(ENODEV)
#define	LDR_ENOTDIR	ldr_errno_to_status(ENOTDIR)
#define	LDR_EISDIR	ldr_errno_to_status(EISDIR)
#define	LDR_EINVAL	ldr_errno_to_status(EINVAL)
#define	LDR_ENFILE	ldr_errno_to_status(ENFILE)
#define	LDR_EMFILE	ldr_errno_to_status(EMFILE)
#define	LDR_ENOTTY	ldr_errno_to_status(ENOTTY)
#define	LDR_ETXTBSY	ldr_errno_to_status(ETXTBSY)
#define	LDR_EFBIG	ldr_errno_to_status(EFBIG)
#define	LDR_ENOSPC	ldr_errno_to_status(ENOSPC)
#define	LDR_ESPIPE	ldr_errno_to_status(ESPIPE)
#define	LDR_EROFS	ldr_errno_to_status(EROFS)
#define	LDR_EMLINK	ldr_errno_to_status(EMLINK)
#define	LDR_EPIPE	ldr_errno_to_status(EPIPE)
#define	LDR_EDOM	ldr_errno_to_status(EDOM)
#define	LDR_ERANGE	ldr_errno_to_status(ERANGE)
#define	LDR_EDUPPKG	ldr_errno_to_status(EDUPPKG)
#define LDR_EVERSION	ldr_errno_to_status(EVERSION)
#define LDR_ENOPKG	ldr_errno_to_status(ENOPKG)
#define LDR_ENOSYM	ldr_errno_to_status(ENOSYM)

/* All loader internal errno's have magnitudes > SYSTEM_ERRNO_MAX */

#define	SYSTEM_ERRNO_MAX	0x3fffffff


/* Loader internal errno's */

#define LDR_ENOMODULE	ldr_errno_to_status(SYSTEM_ERRNO_MAX+1)
#define LDR_ENOMAIN	ldr_errno_to_status(SYSTEM_ERRNO_MAX+2)
#define LDR_EALLOC	ldr_errno_to_status(SYSTEM_ERRNO_MAX+3)

#define	LDR_MAXSTATUS	4

/* Routines to translate system errno's to loader error statuses and
 * vice-versa.
 */

/* Translate a system errno to a loader error status.  Assumes that all
 * system errno's are positive and less than SYSTEM_ERRNO_MAX.  Can't fail.
 *
 *   int errno_to_status(int err_no);
 */

#define	ldr_errno_to_status(err)	(-(err))


#ifndef _NO_PROTO
#include <stdarg.h>
#endif

/* Routines to log error information and to print error messages
 * to the user (currently via /dev/tty)
 */

/* Log information about the current error to the static loader
 * error logging buffer.  Information from the buffer can be
 * retrieved later by using the %B format character to ldr_error().
 * Arguments are like simplified printf (supports %x, %d, %u, %o,
 * %c, %s, field widths), plus loader-specific format-characters:
 *   %B		include error log buffer as a string (don't use this
 *		in ldr_log())
 *   %E		format loader error message string
 */

extern void
ldr_log __((const char *fmt, ...));

/* Display an error message to the user.  Currently this simply opens
 * /dev/tty and blurts the message out.  In the future we may try to
 * do something more graceful.  Arguments are same as ldr_log() above.
 */

extern void
ldr_msg __((const char *fmt, ...));

/* Simple-minded sprintf.  Supports the same formats as ldr_log.
 * No return value.  Unlike real sprintf, it's safe (takes a
 * buffer size).
 */

extern void
ldr_sprintf __((char *buffer, int bufsize, const char *fmt, ...));

/* Display an already formatted message to the user. We don't use
 * any resources which may not be available yet (e.g. heap) so the
 * routine can always be called.
 */

extern void
ldr_puts __((char *s));

#endif	/* _H_LDR_ERRNO */
