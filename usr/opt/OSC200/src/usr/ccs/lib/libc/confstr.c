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
static char *rcsid = "@(#)$RCSfile: confstr.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 22:42:30 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: confstr
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.2  com/lib/c/gen/confstr.c, 9113320, bos320 2/26/91 14:17:19
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak confstr = __confstr
#endif
#endif
#include <unistd.h>
#include <string.h>
#include <errno.h>

size_t
confstr(int name, char *buf, size_t len)
{ 
	switch (name)
	{
		case _CS_PATH:
#ifndef _CSPATH
			/*  If _CSPATH is not defined, return zero */
			return(0);
#else
		       /* 
                        *  If the buffer is large enough to hold the _CSPATH
                        *  value + NULL, copy _CSPATH into buffer and return
			*  the size of the buffer to indicate success.
                        */

			if (len > strlen(_CSPATH))  {
				strcpy(buf, _CSPATH);
			}

		       /*  
                        *  If the buffer is not large enough, truncate
			*  the _CSPATH value to len - 1 and copy into buffer.
                        */

			if ((len != 0) && (buf != (char *)0)) {
				strncpy(buf, _CSPATH, --len);
				buf += len;
				*buf = '\0';
			}

			/*
                         *  return the size buffer needed in all cases.
                         */
			return(strlen(_CSPATH)+1);
#endif

		/* If name is not valid, set errno and exit */
		default:
			_Seterrno(EINVAL);
			return(0);
	}

}  
