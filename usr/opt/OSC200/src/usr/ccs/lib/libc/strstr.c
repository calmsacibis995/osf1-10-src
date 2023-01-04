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
static char	*sccsid = "@(#)$RCSfile: strstr.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/21 14:34:32 $";
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
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strstr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * strstr.c	1.12  com/lib/c/str,3.1,8943 10/20/89 13:03:57
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <string.h>

/*
 * FUNCTION: Locates the first occurrence in the string pointed to by s1
 *	     of the sequence of characters, excluding the terminating null
 *	     character, in the string pointed to by s2.  If s2 points to
 *	     a zero length string, the value of s1 is returned.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS:
 *	     char *s1 - string to search
 *	     char *s2 - string to find
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer; the location of the found
 *	     string, or NULL if the string was not found, or the value of
 *	     s1 if s2 points to a zero length string.
 */
/*LINTLIBRARY*/


char	*
strstr(const char *s1, const char *s2)
{
  unsigned char *p, *q, *r;
  register int pp, qq, rr;

  if( *s2 == '\0' )
    return((char *)s1);

  q = (unsigned char *)s1;
  while (TRUE) {
        qq = *q;
        if (qq != '\0') {
                r = q;
                p = (unsigned char *)s2;
                while (TRUE) {
                        rr = *r;
                        pp = *p;
                        if (rr != '\0' && pp != '\0') {
                                if (pp != rr)
                                        break;
                                r++;
                                p++;
                        }
                        else
                                break;
                }
                if (pp == '\0')
                        break;
                q++;
        }
        else
                break;
  }

  if (qq)
      return((char *)q);
  else
      return(NULL);
}
