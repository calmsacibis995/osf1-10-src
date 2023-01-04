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
static char	*sccsid = "@(#)$RCSfile: NLscanf.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/06/08 01:18:12 $";
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

/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: NLscanf, NLfscanf, NLsscanf 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sccsid[] = "NLscanf.c 1.15  com/lib/c/io,3.1,9021 1/18/90 09:49:39";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NLfscanf = __NLfscanf
#pragma weak NLscanf = __NLscanf
#pragma weak NLsscanf = __NLsscanf
#endif
#include <stdio.h>
#ifdef  _THREAD_SAFE
#include "stdio_lock.h"
#endif
#include <stdarg.h>

/* 
 * EXTERNAL PROCEDURE CALLED
 */

extern int _doscan();

/*
 * NAME: NLscanf 
 *
 * FUNCTION: Reads character data including NLchars from stdin,    
 *           according to fmt and save data to va_alist. 
 *
 * NOTE:     extern _doscan() does all the scan and match.
 *
 * RETURN VALUE DESCRIPTION: Returns the number of matches found between
 *           fmt and va_alist, and EOF on the end of the input. 
 */

int
NLscanf(unsigned char *fmt, ...) 
{
#ifdef  _THREAD_SAFE
        register int rc;
        register filelock_t filelock;
#endif
        va_list ap;     /* the argument list */ 

	va_start(ap, fmt);	/* initialize ap */ 
#ifdef  _THREAD_SAFE
        filelock = _flockfile(stdin);
        rc = _doscan(stdin, fmt, ap);
        _funlockfile(filelock);

        return(rc);
#else
	return(_doscan(stdin, fmt, ap));
#endif
}

/*
 * NAME: NLfscanf 
 *
 * FUNCTION: Reads character data including NLchars from stream iop,   
 *           according to fmt and save data to va_alist. 
 *
 * NOTE:     extern _doscan() does all the scan and match.
 *
 * RETURN VALUE DESCRIPTION: Returns the number of matches found between
 *           fmt and va_alist, and EOF on the end of the input. 
 */

int
NLfscanf(FILE *iop, unsigned char *fmt, ...)
/* FILE *iop		 an input file */
/* char *fmt       	 a conversion specifications */
{
#ifdef  _THREAD_SAFE
        register int rc;
        register filelock_t filelock;
#endif
        va_list ap;	/* argument list */

	va_start(ap, fmt);   /* initialize ap */
#ifdef  _THREAD_SAFE
        filelock = _flockfile(stdin);
        rc = _doscan(iop, fmt, ap);
        _funlockfile(filelock);

        return(rc);
#else
        return(_doscan(iop, fmt, ap));
#endif
}

/*
 * NAME: NLsscanf 
 *
 * FUNCTION: Read character data including NLchars from str according
 *           to fmt and save data to va_alist.  
 *
 * NOTE:     extern _doscan() does all the scan and match.
 *
 * RETURN VALUE DESCRIPTION: Returns the number of matches found between
 *           fmt and va_alist, and EOF on the end of the input. 
 */

int
NLsscanf(unsigned char *str, unsigned char *fmt, ...)
/* char *str	 	an input string */
/* char *fmt           the conversion specifications */  
{
	va_list ap;	/* argument list */
	FILE strbuf;    /* temporary file */

	va_start(ap, fmt);	/* initialize ap */

/*  Convert data type from char to FILE */ 
	strbuf._flag = (_IOREAD | _IONOFD);
	strbuf._ptr = strbuf._base = (unsigned char*)str;
	strbuf._cnt = strlen(str);
	strbuf._file = _NFILE;
	strbuf._lock = NULL;
	return(_doscan(&strbuf, fmt, ap));
}
