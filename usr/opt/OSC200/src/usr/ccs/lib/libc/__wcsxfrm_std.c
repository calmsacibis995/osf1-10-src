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
static char *rcsid = "@(#)$RCSfile: __wcsxfrm_std.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/12 22:39:40 $";
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
 * FUNCTIONS: __wcsxfrm_std.c
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.6  com/lib/c/str/__wcsxfrm_std.c, libcstr, bos320, 9138320 9/13/91 11:46:19
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <wchar.h>
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <machine/endian.h>
#include "collation.h"

/*
 * FUNCTION: Compares the strings pointed to by wcs_in and wcs_out, returning an
 *	     integer as follows:
 *
 *		Less than 0	If wcs_in is less than wcs_out
 *		Equal to 0	If wcs_in is equal to wcs_out
 *		Greater than 0	If wcs_in is greater than wcs_out.
 *
 *	     The comparison is based on the collating sequence specified
 *	     by the locale category LC_COLLATE affected by the setlocale
 *	     function.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *wcs_in - first string
 *	     char *wcs_out - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */
/**********
  __wcsxfrm_std is calling strxfrm due to the fact that regular expressions
  cannot handle process code.
**********/

size_t
__wcsxfrm_std(wchar_t *wcs_out, const wchar_t *wcs_in, size_t n, _LC_collate_t *hdl)
{
    char *str_in;
    int  len_in;
    int  rc;
    
    /**********
      malloc the space for the multi-byte wcs_in
    **********/
    if ((str_in = (char *)malloc( (len_in=wcslen(wcs_in)*MB_CUR_MAX + 1) )) == NULL) {
	_Seterrno(ENOMEM);
	return (size_t) -1;
    }

    /**********
      convert the process code to file code
    **********/
    if (wcstombs(str_in, wcs_in, len_in) == -1) {
	(void)free(str_in);
	_Seterrno(EINVAL);
	return (size_t) -1;
    }

    rc = strxfrm((char *) wcs_out, str_in, n*sizeof(wchar_t));

    /**********
      since only one null is placed at the end of wcs_out, always
      put a null wchar null
    **********/
    rc /= sizeof(wchar_t);

    if (rc >= n)
	wcs_out[n-1] = 0x00;
    else
	wcs_out[rc] = 0x00;

    /**********
      free up the malloced space and return
    **********/
    (void)free(str_in);

    /*
     * May need to reverse byte-order for little-endian stuff
     */
#if BYTE_ORDER == LITTLE_ENDIAN
    while(*wcs_out) {
	*wcs_out = byte_string_to_collation(*wcs_out);
	wcs_out++;
    }
#endif /* BYTE_ORDER */

    return(rc);

}
