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
static char	*sccsid = "@(#)$RCSfile: wcswcs.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/08 20:44:07 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: wcswcs
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * wcswcs.c	1.4  com/lib/c/nls,3.1,9013 2/27/90 21:46:08
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wcswcs = __wcswcs
#endif
#endif
#include <sys/types.h>

/*
 * NAME: wcswcs
 *
 * FUNCTION: Locate the first occurrence in the string pointed to by
 *  string1 of the sequence of wchar_t characters in the string pointed
 *  to by string2.
 *
 * PARAMETERS:
 *	wchar_t *string1	-	the wide character string
 *	wchar_t *string2	-	the wide character string
 *
 * RETURN VALUE DESCRIPTION: the pointer to the located string or NULL if
 *  the string is not found.  If string2 points to a string with zero length
 *  the function returns string1.
 */
wchar_t *
wcswcs(wchar_t *string1,wchar_t *string2)
{
	register wchar_t *p;
	register wchar_t *q;
	register wchar_t *r;

  if( *string2 == '\0' )
    return(string1);

  for(q=string1; *q != '\0'; q++) 
  {
    for(r=q, p=string2; *r != '\0' && *p != '\0'; r++, p++)
    {
      if( *p != *r )
	break;
    }
    if( *p == '\0' )
      break;
  }
  if (*q)
      return(q);
  return(NULL);
}
