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
static char	*sccsid = "@(#)$RCSfile: ftok.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/06/29 14:04:40 $";
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
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: ftok
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
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
 * ftok.c	1.6  com/lib/c/gen,3.1,8943 9/8/89 08:47:54
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak ftok = __ftok
#endif
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
/* #include <sys/sysmacros.h> */

/*
 * NAME:	ftok
 *                                                                    
 * FUNCTION:	generate a standard ipc key
 *                                                                    
 * NOTES:	Ftok generates a key based on 'path' and 'id'.  'Path'
 *		is the pathname of a currently existing file that is
 *		accessible to the process.  'Id' is a character that
 *		can cause different keys to be generated for the same
 *		'path'.
 *
 * RETURN VALUE DESCRIPTION:	-1 if the file does not exist or is
 *		not accessible to the process, -1 if 'id' is '\0',
 *		else the key
 */  

key_t
ftok(path, id)
char *path;			/* pathname to base key on */
char id;			/* unique id per application */
{
	struct stat st;

	/* Leave keys 0 to (2^24-1) for SNA, other use */
	if(id == '\0')
		return((key_t)-1);

	/*
	 * if the file is not accessible or doesn't exist, return -1.
	 * else use a combo of 'id', the minor device number of the
	 * device the file lives on, and the file's inode number to
	 * compute the key...
	 */
	return(stat(path, &st) < 0 ? (key_t)-1 :
            (key_t)((key_t)id << 24 |
                   ((unsigned)minor(st.st_dev)<<16 & 0x00ff0000) |
                   (unsigned)st.st_ino));
}
