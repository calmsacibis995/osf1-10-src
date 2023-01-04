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
 *	@(#)$RCSfile: aio.h,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/12/15 22:12:32 $
 */
#ifdef _POSIX_4SOURCE

#ifndef __AIO_H

#define __AIO_H

#ifndef POSIX_4D10

#include <sys/sysaio.h>

/*
 * redefine close
 */
#ifdef close
#undef close
#endif

#define close(f) aio_close(f)

#ifdef _NO_PROTO

int aio_read();
int aio_write();
int lio_listio();
int aio_cancel();
int aio_suspend();
int aio_return();
int aio_close();

#else

_BEGIN_CPLUSPLUS
int aio_read(struct aiocb *);
int aio_write(struct aiocb *);
int lio_listio(int, struct aiocb *[], int, struct sigevent *);
int aio_cancel(int, struct aiocb *);
int aio_suspend(int, const struct aiocb *[]);
int aio_return(struct aiocb *);
int aio_close(int);
_END_CPLUSPLUS

#endif /*_NO_PROTO*/

#else /* POSIX_4D10 */

/*
 * POSIX_4D10: provides source-level compatibility for POSIX 1003.4/Draft 10
 * applications using AIO. Draft 10 and Draft 11 calls and data structures
 * *cannot* be mixed: it's either one draft or the other. Compatibility
 * is provided by redefining the D10 calls to the form *_D10; for example,
 * aio_read() is redefined to aio_read_D10(). These calls will be resolved
 * to D10-compatibile object code in /usr/ccs/lib/libaio.a.
 *
 *				NOTE WELL
 *	Support for this D10-to-D11 compatibility will be removed with the
 *	next release of DEC OSF/1.
 */

#include <signal.h>		/* get sigevent definition */
#include <sys/types.h>		/* get size_t */

/*
 * define the AIO/LIO constants
 */
#define AIO_EVENT 1

#define AIO_CANCELED 0		/* all operations cancelled */
#define AIO_NOTCANCELED 1	/* not all operations cancelled */
#define AIO_ALLDONE 2		/* all operations were completed */

#define LIO_WAIT 1		/* wait for all operations to complete */
#define LIO_ASYNC 2		/* notify when all operations done */
#define LIO_NOWAIT 3		/* no notification */

#define LIO_READ 4
#define LIO_WRITE 5
#define LIO_NOP 0

#define _POSIX_PRIORITIZED_IO

#ifdef AIO_SUSPEND
typedef struct AIO_SUSPEND *aio_suspendp;
#else
typedef void *aio_suspendp;
#endif

struct aio_handle {
	int aio_errno;
	size_t aio_result;
};

typedef struct aio_handle *aiohandle_t;

struct aiocb {
	int aio_whence;
	off_t aio_offset;
	volatile char *aio_buf;
	size_t aio_nbytes;
	int aio_reqprio;
	struct sigevent aio_event;
	int aio_flag;
	aiohandle_t aio_handle;

	/* implementation extensions */
	struct aio_handle aio_handle_body;
	int aio_opcode;
	int aio_fildes;
	struct aiocb *aio_next;
	aio_suspendp aio_suspended;
};

struct liocb {
	int lio_opcode;
	int lio_fildes;
	struct aiocb lio_aiocb;
};

/*
 * redefine close
 */
#ifdef close
#undef close
#endif

/*
 * Redefine Draft 10 calls to point to Draft 10 code.
 */
#define aio_read	aio_read_D10
#define aio_write	aio_write_D10
#define lio_listio	lio_listio_D10
#define aio_cancel	aio_cancel_D10
#define aio_suspend	aio_suspend_D10
#define aio_close	aio_close_D10

/*
 * These are macros, not functions.
 */
#define aio_return(handle) ((handle)->aio_result)
#define aio_error(handle)  ((handle)->aio_errno)

#define close(f) aio_close_D10(f)

#ifdef _NO_PROTO

int aio_read_D10();
int aio_write_D10();
int lio_listio_D10();
int aio_cancel_D10();
int aio_suspend_D10();
int aio_close_D10();
#else

_BEGIN_CPLUSPLUS
int aio_read_D10(int, struct aiocb *);
int aio_write_D10(int, struct aiocb *);
int lio_listio_D10(int, struct liocb *[], int, struct sigevent *);
int aio_cancel_D10(int, struct aiocb *);
int aio_suspend_D10(int const, struct aiocb *[]);
int aio_close_D10(int);
_END_CPLUSPLUS

#endif /*_NO_PROTO*/

#endif /* POSIX_4D10 */

#endif /*__AIO_H*/

#endif /*_POSIX_4SOURCE*/
