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
static char *rcsid = "@(#)$RCSfile: mbswidth.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 23:28:51 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: mbswidth
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.1  com/lib/c/nls/mbswidth.c, 9115320a, bos320 4/1/91 11:23:04
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak mbswidth = __mbswidth
#endif
#endif
#include <mbstr.h>
#include <string.h>
#include <stdlib.h>

/*
 *
 * FUNCTION: mbswidth
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */

int mbswidth(const char *s, size_t n)
{
    int len;
    wchar_t *wcs;
    int rc;
    char *str_ptr;

    if ((s == (char *)NULL) || (*s == '\0'))
	return ((int)NULL);

    /**********
      get the space for the process code.  There cannot be more process
      codes than characters
    **********/
    if ((wcs = (wchar_t *) malloc ((n+1) * sizeof(wchar_t))) == (wchar_t *)NULL){
	perror("mbswidth:malloc");
	return ((int)NULL);
    }

    /**********
      get space for a temp string
    **********/
    if ((str_ptr = (char *) malloc (n+1)) == (char *)NULL) {
	perror("mbswidth:malloc");
	free(wcs);
	return((int)NULL);
    }

    /**********
      copy s into the temp string
    **********/
    strncpy(str_ptr, s, n);
    str_ptr[n] = '\0';

    rc = mbstowcs(wcs, str_ptr, n+1);

    /**********
      was there an invalid character found
    **********/ 
    if (rc == -1)
	len = -1;
    else
	len = wcswidth(wcs, rc+1);

    /*********
      free up the malloced space
    ********/
    free(wcs);
    free(str_ptr);

    return(len);
}
