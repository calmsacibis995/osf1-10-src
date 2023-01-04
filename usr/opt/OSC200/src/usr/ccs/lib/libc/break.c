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
static char	*sccsid = "@(#)$RCSfile: break.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/06/07 19:47:24 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES)
#pragma weak brk = __brk
#pragma weak sbrk = __sbrk
#endif
#endif
#include <sys/types.h>
#include <errno.h>


#ifndef	_SHARED_LIBRARIES

#error "Compatibility brk code not yet supported; use brk.o and sbrk.o instead"

#include <sys/mman.h>

/* minbrk is the  minimum allowable break address; it's initialized to "_end".
 * curbrk is the current break; it's initially set to minbrk, and it varies
 * as brk() and sbrk() are performed.
 */

extern	char	_end[];
static	char	*minbrk = _end;
static	char	*curbrk = _end;


#define	round(x, s)	(((unsigned)(x) + (unsigned)(s) - 1) & ~((unsigned)(s) - 1))
#define	round_page(addr) ((void *)round((addr), getpagesize()))


char *
#ifndef _NO_PROTO
brk(char *addr)
#else
brk(addr)
char *addr;
#endif

/* Set the current break to the specified address,
 * Grows or shrinks the break area (by using mmap() and munmap())
 * so that the break area ends at the specified address.
 */
{
	char 		*newbrk;

	addr = round_page(addr);

	if ((char *)addr < (char *)curbrk) {

		/* Shrinking */

		if ((char *)addr < (char *)minbrk)
			addr = minbrk;
		if (munmap(addr, ((char *)curbrk - (char *)addr)) < 0)
			return((char *)(-1));

	} else if ((char *)addr > (char *)curbrk) {

		/* Expanding */

		if ((newbrk = mmap(curbrk, ((char *)addr - (char *)curbrk),
				   PROT_READ|PROT_WRITE|PROT_EXEC,
				   MAP_ANON|MAP_PRIVATE|MAP_FIXED,
				   -1, (off_t)0)) == (caddr_t)(-1))
			return((char *)(-1));

	}

	curbrk = addr;
	return((char *)0);
}


char *
#ifndef _NO_PROTO
sbrk(ssize_t incr)
#else
sbrk(incr)
ssize_t incr;
#endif

/* Add the specified (signed) quantity to the current break.
 * If shrinking, unmap the space being removed.  If growing,
 * map more space.  Returns the old break.
 */
{
	char		*obrk;
	char		*newbrk;
	char		*tmp;

	obrk = (char *)curbrk;
	newbrk = (char *)curbrk + incr;
	newbrk = round_page(newbrk);

	if (incr < 0) {

		/* Shrinking */

		if ((char *)newbrk < (char *)minbrk)
			newbrk = minbrk;
		if (munmap(newbrk, -incr) < 0)
			return((char *)(-1));

	} else if (incr > 0) {

		/* Expanding */

		if ((tmp = mmap(curbrk, incr, 
				PROT_READ|PROT_WRITE|PROT_EXEC,
				MAP_ANON|MAP_PRIVATE|MAP_FIXED,
				-1, (off_t)0)) == (char *)(-1))
			return((char *)(-1));
	}

	curbrk = newbrk;
	return(obrk);
}

#else	/* _SHARED_LIBRARIES */

/* This version of brk.c simply calls the standalone loader for 
 * brk() and sbrk().
 */
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ldr_rmutex;
#endif

#include <loader.h>
#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>

char *
#ifndef _NO_PROTO
brk(char *addr)
#else
brk(addr)
char *addr;
#endif

{
	int	rc;
        
        TS_LOCK(&_ldr_rmutex);
	if ((rc = ldr_brk(addr)) < 0) {
		TS_SETERR(ldr_status_to_errno(rc));
		rc = -1;
	}
        TS_UNLOCK(&_ldr_rmutex);
	return((char *)rc);
}


char *
#ifndef _NO_PROTO
sbrk(ssize_t incr)
#else
sbrk(incr)
ssize_t incr;
#endif

{
	char	*obrk;
	int	rc;

        TS_LOCK(&_ldr_rmutex);
	if ((rc = ldr_sbrk(incr, &obrk)) < 0) {
		TS_SETERR(ldr_status_to_errno(rc));
                TS_UNLOCK(&_ldr_rmutex);
		return((char *)-1);
	}
        TS_UNLOCK(&_ldr_rmutex);
	return((char *)obrk);
}

#endif	/* _SHARED_LIBRARIES */
