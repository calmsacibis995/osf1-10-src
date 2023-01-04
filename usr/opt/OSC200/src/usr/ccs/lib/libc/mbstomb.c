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
static char *rcsid = "@(#)$RCSfile: mbstomb.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 23:28:20 $";
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
 * FUNCTIONS: mbstomb
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.1  com/lib/c/nls/mbstomb.c, 9115320a, bos320 4/1/91 11:23:02
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak mbstomb = __mbstomb
#endif
#endif
#include <mbstr.h>
#include <stdlib.h>

/*
 *
 * FUNCTION: mbstomb
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */

mbchar_t mbstomb(const char *s)
{
    int len;
    mbchar_t rc = 0;
    
    if (s == (char *)NULL)
	return ((mbchar_t)NULL);

    /**********
      if an invalid character is encountered, return
      the pointer to it
    **********/
    if ((len = mblen(s, MB_CUR_MAX)) == -1)
	return(0);

    while (len--) {
	rc = (rc << 8) | (unsigned char)*s++;
    }

    return (rc);
}
	    

    
    
    
    
