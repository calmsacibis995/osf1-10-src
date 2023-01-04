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
static char *rcsid = "@(#)$RCSfile: mbsinvalid.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 23:27:52 $";
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
 * FUNCTIONS: mbsinvalid
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.1  com/lib/c/nls/mbsinvalid.c, 9115320a, bos320 4/1/91 11:23:00
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak mbsinvalid = __mbsinvalid
#endif
#endif
#include <mbstr.h>
#include <stdlib.h>

/*
 *
 * FUNCTION: mbsinvalid
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */

char *mbsinvalid(const char *s)
{
    int len;
    int rc;
    
    if (s == (char *)NULL)
	return ((char *)NULL);

    /**********
      if an invalid character is encountered, return
      the pointer to it
    **********/
    while (*s) {
	if ((rc = mblen(s, MB_CUR_MAX)) == -1)
	    return((char *)s);
	s += rc;
    }

    /**********
      if we made it here, all of the characters are valid
    **********/
    return ((char *)NULL);
}
	    

    
    
    
    
