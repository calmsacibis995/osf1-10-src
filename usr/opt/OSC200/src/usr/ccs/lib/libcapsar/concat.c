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
/*LINTLIBRARY*/

#ifndef lint
static char *sccsid = "@(#)$RCSfile: concat.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/03 15:27:28 $";
#endif lint

/* concat.c - concatenate a bunch of strings in managed memory */

#include <stdio.h>
#include <varargs.h>


/* VARARGS */

char   *concat (va_alist)
va_dcl
{
    register char  *cp,
                   *dp,
                   *sp;
    register unsigned   len;
    register    va_list list;
    char	   *copy();

    len = 1;
    va_start (list); 
    while (cp = va_arg (list, char *))
	len += strlen (cp);
    va_end (list);

    dp = sp = (char *)malloc (len);
    if (dp == NULL)
	return(NULL);

    va_start (list); 
    while (cp = va_arg (list, char *))
	sp = copy (cp, sp);
    va_end (list);

    return dp;
}
