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
static char *rcsid = "@(#)$RCSfile: wctype.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 20:45:41 $";
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
 * FUNCTIONS: wctype
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/chr/wctype.c, libcchr, bos320, 9130320 7/17/91 15:15:57
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wctype = __wctype
#endif
#endif
#include <sys/localedef.h>
#include <ctype.h>
#include <string.h>


wctype_t 
__wctype_std( char *name, _LC_ctype_t *hdl)
{
    int i;
    int rc = -1;

    /**********
      search thru the list of names and try to
      find a match
    **********/
    for (i=0; i<hdl->nclasses &&
	      (rc=strcmp(name, hdl->classnms[i].name)) > 0; i++);

    /**********
      if a match was found, return the mask
    **********/
    if (rc==0)
	return(hdl->classnms[i].mask);
    
    else
	return(0);
}


wctype_t 
wctype(char *name)
{
	if (METHOD(__lc_ctype,wctype) == NULL)
		return __wctype_std(name, __lc_ctype);
	else
		return METHOD(__lc_ctype,wctype)(name, __lc_ctype);
}


