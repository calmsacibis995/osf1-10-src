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
static char	*sccsid = "@(#)$RCSfile: fgetws.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:51:30 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: fgetws 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * com/lib/c/io/fgetws.c, bos320 2/26/91 17:54:08
 */

/*LINTLIBRARY*/
/*
 * This version reads directly from the buffer rather than looping on getwc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak fgetws = __fgetws
#endif
#endif
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "stdiom.h"
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)	return(TS_FUNLOCK(filelock), (wchar_t *)(val))

#else
#define	RETURN(val)	return(val)
#endif	/* _THREAD_SAFE */


extern int __filbuf(),_wcfilbuf();

wchar_t 	*
fgetws(wchar_t *s, int n, FILE *stream)
{
	wchar_t *nextstart;
	char  *retptr;
	int converted,err;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if ((stream->_flag&(_IOREAD|_IORW))==0) {  /* not open for reading */
		_Seterrno(EBADF);
		stream->_flag |= _IOERR;
		RETURN(NULL);
	}
	nextstart=s;
	for(n--;;) {
		/* If the buffer was empty try reading some more from the file.
		 */
		if(stream->_cnt <= 0 ) {
			if(__filbuf(stream) == EOF)  {
				if(nextstart == s )  {
					RETURN(NULL);
				}
				break; 
			}
			stream->_ptr--;
			stream->_cnt++;
		}

		/* Try converting the bytes in the buffer to process code.
		 * __mbstopcs (
		 *	wchar_t *  -  start storing the converted wchar here.
		 *	size_t     -  upto a maximum of these many wchars.
		 *	char *     -  start reading the bytestring from here.
		 *	size_t     -  upto a maximum of these many bytes.
		 *	uchar *    -  stop converting if this byte encountered.
		 *	char **    -  pointer in bytestring after byte where 
		 *		      conversion stopped.
		 *	int *      -  Has the return error indicator
		 *		      0 => success.
		 *		      -1 => illegal character.
		 *		      >0 => not enough characters in the buffer
		 *			    to form a valid character.Need at
		 *			    least these many.
		 *	)
		 *
		 * __mbstopcs() returns number of wide characters converted 
		 * before stopping or -1 if it is not supported for this code
		 * set.
		*/
		if((converted=__mbstopcs(nextstart,n,stream->_ptr,stream->_cnt,
					'\n',&retptr,&err)) > 0 ) {
			stream->_cnt-=retptr - (char *)stream->_ptr;
			stream->_ptr=(unsigned char *)retptr;
			nextstart += converted;
			if(((n-=converted) <= 0) || retptr[-1] == '\n')
				break;
			if(! err )
				continue;
		} else {
			if(converted == -1) {
				/* __mbstopcs not supported for this 
				 * code set.  Implement a version 
				 * using mbstowcs() or getwc() here.
				 * For now this is a stub.
				 */
				stream->_flag |= _IOERR;
				_Seterrno(EILSEQ);
				RETURN(NULL);
			}
		}


		if(err == -1 || _wcfilbuf(stream,err) == EOF) {

			/* Invalid character found .
			 * We are here because either __mbstowcs returned a -1
			 * in err or we got a character straddling the buffer
			 * boundary and _wcfilbuf() was not able to read 
			 * sufficient number of bytes.
			 */

			*nextstart = '\0';	/* should be (wchar _t) 0 */
			stream->_flag |= _IOERR;
			_Seterrno(EILSEQ);
			RETURN(NULL);
		}

	}

	*nextstart='\0';
	RETURN(s);
}
