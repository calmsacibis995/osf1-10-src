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
static char	*sccsid = "@(#)$RCSfile: setbuffer.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:37:13 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * FUNCTIONS: setbuffer, setlinebuf 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * setbuffer.c 1.5  com/lib/c/io,3.1,9013 10/18/89 15:08:42
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak setbuffer = __setbuffer
#pragma weak setlinebuf = __setlinebuf
#endif
#include <stdio.h>
#include <malloc.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */


/*                                                                    
 * FUNCTION: Assigns buffering to a stream.
 *
 * RETURN VALUE DESCRIPTION: None.
 *
 * setbuffer - setup stdio stream buffering after opening, but before
 *	it is read or written.
 */
void
setbuffer(FILE *stream, char *buf, int size)
{
	/* we just call the sysV setvbuf(3) */
	(void)setvbuf( stream, buf, _IOFBF, size );
	/* BSD returns indeterminate value */
}


#ifdef _THREAD_SAFE
#define	FFLUSH	fflush_unlocked
#define	SETVBUF	setvbuf_unlocked
#else
#define	FFLUSH	fflush
#define	SETVBUF	setvbuf
#endif	/* _THREAD_SAFE */

/*
 * setlinebuf - change stdio stream buffering from block or unbuffered to
 *	line buffered, may be used even after reading or writting.
 */
void setlinebuf(FILE *stream)
{
	char	*buf;		/* ptr for tmp usage */

	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	FFLUSH(stream);				      /* force out all output */
	SETVBUF(stream,(char *)NULL,_IONBF,(size_t)0);/* close down buffering */
	buf = (char *)malloc((size_t)BUFSIZ);	      /* get a new buffer */
	if( buf != NULL ) {
		(void)SETVBUF(stream, buf, _IOLBF, (size_t)BUFSIZ);
		/* say this buffer belongs to stdio */
		stream->_flag |= _IOMYBUF;
	}

	TS_FUNLOCK(filelock);

	/* BSD returns indeterminate value */
}
