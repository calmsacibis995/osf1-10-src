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
static char *rcsid = "@(#)$RCSfile: __wcscoll_std.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:22:22 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcscoll_std
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.6  com/lib/c/str/__wcscoll_std.c, libcstr,bos320, 9132320m 8/11/91 17:47:07
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

/*
 * FUNCTION: Compares the strings pointed to by wcs1 and wcs2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If wcs1 is less than wcs2
 *		Equal to 0	If wcs1 is equal to wcs2
 *		Greater than 0	If wcs1 is greater than wcs2.
 *
 *	     The comparison is based on the collating sequence specified
 *	     by the locale category LC_COLLATE affected by the setlocale
 *	     function.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *wcs1 - first string
 *	     char *wcs2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */
/**********
  __wcscoll_std is calling strcoll due to the fact that regular expressions
  cannot handle process code.
**********/

int
__wcscoll_std(const wchar_t *wcs1, const wchar_t *wcs2, _LC_collate_t *hdl)
{
    char *str1;
    char *str2;
    int  len1;
    int  len2;
    int  rc;
    
    /**********
      malloc the space for the multi-byte wcs1
    **********/
    if ((str1 = (char *)malloc( (len1=wcslen(wcs1)*MB_CUR_MAX + 1) )) == NULL) {
	perror("__wcscoll_std:malloc");
	_exit (-1);
    }
    if ((str2 = (char *)malloc( (len2=wcslen(wcs2)*MB_CUR_MAX + 1) )) == NULL) {
	perror("__wcscoll_std:malloc");
	_exit (-1);
    }

    if (wcstombs(str1, wcs1, len1) == -1) 
	_Seterrno(EINVAL);
    if (wcstombs(str2, wcs2, len2) == -1) 
	_Seterrno(EINVAL);

    rc = strcoll(str1, str2);

    /**********
      free up the malloced space and return
    **********/
    (void)free(str1);
    (void)free(str2);
    return(rc);

}
