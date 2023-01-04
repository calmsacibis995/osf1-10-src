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
static char	*sccsid = "@(#)$RCSfile: findiop.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/08/18 21:36:06 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * findiop.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * FUNCTIONS: _findiop 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * findiop.c	1.8  com/lib/c/io,3.1,8943 10/18/89 10:52:42
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#include "ts_supp.h"
#include "glue.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#include "rec_mutex.h"

extern struct rec_mutex	_iobptr_rmutex;

#endif /* _THREAD_SAFE */

FILE *
_findiop(void)
{
	register FILE *fp, **fpp;
	register int i, size;
	int	cur_row, num_rows;
	TS_FDECLARELOCK(filelock)

	TS_LOCK(&_iobptr_rmutex);

	for(;;) {

		/* Look through the iobptr list for a free iob.
		 * Start at the hint supplied by freefile.
		 */
		for (i = _glued.freefile; i < _glued.nfiles; i++) {
			if (!inuse(fp = (FILE *)&_glued.iobptr[i>>4][i&0xf])) {
#ifdef _THREAD_SAFE
				/* Make sure next thread through doesn't use
				 * this iob. Check the file lock. If null,
				 * allocate it. If it's non-null, testlock().  
				 * If we get it, it's ours.
				 */
				if (fp->_lock == NULL)
					(void)_rec_mutex_alloc((rec_mutex_t *)
								&fp->_lock);
				if (filelock = _ftestfile(fp)) {	
					if (!inuse(fp)) { /* found one */
						fp->_flag = _IOINUSE;
						TS_FUNLOCK(filelock);
						_glued.freefile = i + 1;
						if (i > _glued.lastfile)
							_glued.lastfile = i;
						TS_UNLOCK(&_iobptr_rmutex);
						return (fp);
					}
					/* not free, give it back */
					TS_FUNLOCK(filelock);
				}	/* lock failed, must be in use */
			}
#else	/* _THREAD_SAFE */
				_glued.freefile = i + 1;
				if (i > _glued.lastfile)
					_glued.lastfile = i;
				return (fp);
			}
#endif	/* _THREAD_SAFE */
		}

		/* There are no free iobs in the current list.
		 * Grow the list to the next size up and repopulate.
		 */
		if (_glued.nrows < _NROWEXTEND)
			num_rows = _NROWEXTEND;
		else
			num_rows =  _glued.nrows << 2;
		if (!(fpp = (FILE **)malloc(size =
					    sizeof(FILE **) * num_rows))) {
				TS_UNLOCK(&_iobptr_rmutex);
				return (NULL);
		}

		/* Copy the old list into the new and free the old if it
		 * is not the initial one (which is static storage).
		 */
		memset(fpp, 0, size);
		memcpy((void *)fpp, (void *)_glued.iobptr,
		       sizeof(FILE **) * _glued.nrows);
		if (_glued.nrows > _NROWSTART)
			free(_glued.iobptr);

		/* Reset counters.
		 */
		_glued.nfiles =  num_rows * _NIOBRW;
		_glued.iobptr = fpp;
		_glued.nrows = num_rows;
		cur_row = _glued.crow + 1;
		_glued.crow = num_rows - 1;

		/* Set up new iob rows.
		 */
		fpp = (FILE **)&_glued.iobptr[cur_row];
		while (cur_row++ <  num_rows) {
			if (!(*fpp = (FILE *)malloc(_NROWSIZE))) {
				TS_UNLOCK(&_iobptr_rmutex);
				return (NULL);
			}
			memset(*fpp++, 0, _NROWSIZE);
		}

		/* Now retry the list.
		 */
	}
}
