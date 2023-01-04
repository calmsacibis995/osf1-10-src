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
static char	*sccsid = "@(#)$RCSfile: string.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:22:50 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * ABSTRACT:
 *   String utility programs to allocate and copy
 *   or concatenate, free and return constant strings.
 */

#define	EXPORT_BOOLEAN
#include <mach/boolean.h>
#include <sys/types.h>
#include "error.h"
#include "alloc.h"
#include "string.h"

char	charNULL = 0;

string_t
strmake(string)
    char *string;
{
    register string_t saved;

    saved = malloc((u_int) (strlen(string) + 1));
    if (saved == strNULL)
	fatal("strmake('%s'): %s", string, unix_error_string(errno));
    return (string_t)strcpy(saved, string);
}

string_t
strconcat(left, right)
    string_t left, right;
{
    register string_t saved;

    saved = malloc((u_int) (strlen(left) + strlen(right) + 1));
    if (saved == strNULL)
	fatal("strconcat('%s', '%s'): %s",
	      left, right, unix_error_string(errno));
    return (string_t)strcat(strcpy(saved, left), right);
}

void
strfree(string)
    string_t string;
{
    free(string);
}

char *
strbool(bool)
    boolean_t bool;
{
    if (bool)
	return "TRUE";
    else
	return "FALSE";
}

char *
strstring(string)
    string_t string;
{
    if (string == strNULL)
	return "NULL";
    else
	return string;
}
