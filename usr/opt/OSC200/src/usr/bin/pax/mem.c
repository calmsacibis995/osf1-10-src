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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: mem.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 19:57:54 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* 
 * mem.c - memory allocation and manipulation functions
 *
 * DESCRIPTION
 *
 *	These routines are provided for higher level handling of the UNIX
 *	memory allocation functions.
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Revision 1.2  89/02/12  10:04:53  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:17  mark
 * Initial revision
 * 
 */

/* Headers */

#include "pax.h"


/* mem_get - allocate memory
 *
 * DESCRIPTION
 *
 *	Mem_get attempts to allocate a block of memory using the malloc
 *	function call.  In the event that the memory is not available, 
 *	mem_get will display an "Out of memory" message for the user
 *	the first time it encounters the an out of memory situation.
 *	Subsequent calls to mem_get may fail, but no message will be
 *	printed.
 *
 * PARAMETERS
 *
 *	uint len	- The amount of memory to allocate
 *
 * RETURNS
 *
 *	Normally returns the pointer to the newly allocated memory.  If
 *	an error occurs, NULL is returned, and an error message is
 *	printed.
 *
 * ERRORS
 *
 *	ENOMEM	No memory is available 
 */


char *mem_get(uint len)

{
    char           *mem;
    static short    outofmem = 0;

    if ((mem = (char *)malloc(len)) == (char *)NULL && !outofmem) {
	outofmem++;
	warn("mem_get()", MSGSTR(NOMEM, "Out of memory"));
    }
    return (mem);
}


/* mem_str - duplicate a string into dynamic memory
 *
 * DESCRIPTION
 *
 *	Mem_str attempts to make a copy of string.  It allocates space for
 *	the string, and if the allocation was successfull, copies the old
 *	string into the newly allocated space.
 *
 * PARAMETERS
 *
 *	char *str 	- string to make a copy of 
 *
 * RETURNS
 *
 *	Normally returns a pointer to a new string at least as large
 *	as strlen(str) + 1, which contains a copy of the the data 
 *	passed in str, plus a null terminator.  Returns (char *)NULL 
 *	if enough memory to make a copy of str is not available.
 */


char *mem_str(char *str)

{
    char           *mem;

    if (mem = mem_get((uint) strlen(str) + 1)) {
	strcpy(mem, str);
    }
    return (mem);
}
