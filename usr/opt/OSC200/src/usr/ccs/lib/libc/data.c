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
 *	@(#)$RCSfile: data.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:44:32 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/*
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * FUNCTIONS: 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * data.c	1.6  com/lib/c/io,3.1,8943 9/13/89 09:13:03
 */

/* 
 * This module should not be included in libc_r while libc_r is
 * dependent on libc.  The reason for this is because in the shareable
 * case it is possible to get multiple versions of the global data defined in
 * this module.  One in libc_r.so and one in libc.so.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifndef _THREAD_SAFE
/*LINTLIBRARY*/
#include <stdio.h>

/* some slop is allowed at the end of the buffers in case an upset in
 * the synchronization of _cnt and _ptr (caused by an interrupt or other
 * signal) is not immediately detected.
 */
unsigned char _sibuf[BUFSIZ+8+2 * MB_LEN_MAX];
unsigned char _sobuf[BUFSIZ+8+2 * MB_LEN_MAX];
/*
 * Ptrs to start of preallocated buffers for stdin, stdout.
 */
unsigned char *_stdbuf[] = { _sibuf, _sobuf };

unsigned char _smbuf[3+1][_SBFSIZ + 2 * MB_LEN_MAX];

#include "glue.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _stdio_buf_rmutex[3];

FILE _iob[_NIOBRW] = {
	{0, NULL, NULL, 0, _IOREAD, 0, 0, &_stdio_buf_rmutex[0], NULL},
	{0, NULL, NULL, 0, _IOWRT,  1, 0, &_stdio_buf_rmutex[1], NULL},
	{0, _smbuf[2]+2*MB_LEN_MAX, _smbuf[2]+2*MB_LEN_MAX, _SBFSIZ, 
			_IOWRT+_IONBF, 2, 0, &_stdio_buf_rmutex[2],
			_smbuf[2]+2*MB_LEN_MAX+_SBFSIZ}
};
#else /* _THREAD_SAFE */
FILE _iob[_NIOBRW] = {
	{0, NULL, NULL, 0, _IOREAD, 0, 0, 0, NULL},
	{0, NULL, NULL, 0, _IOWRT,  1, 0, 0, NULL},
	{0, _smbuf[2]+2*MB_LEN_MAX, _smbuf[2]+2*MB_LEN_MAX, _SBFSIZ,
			_IOWRT+_IONBF, 2, 0,
			0, _smbuf[2]+2*MB_LEN_MAX+_SBFSIZ}
};
#endif /* _THREAD_SAFE */

FILE _iob1[_NIOBRW], _iob2[_NIOBRW], _iob3[_NIOBRW];
static FILE *_iobptr[_NROWSTART] = { _iob,_iob1,_iob2,_iob3 };

struct glued _glued = { 2,	/* lastfile	*/
			3,	/* freefile	*/
			64,	/* nfiles	*/
			4,	/* nrows	*/
			3,	/* crow		*/
			(FILE **)_iobptr
			};
#endif
