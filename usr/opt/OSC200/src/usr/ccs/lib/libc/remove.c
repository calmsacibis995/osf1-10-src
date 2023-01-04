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
static char	*sccsid = "@(#)$RCSfile: remove.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:18:55 $";
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
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: remove 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * remove.c	1.9  com/lib/c/io,3.1,9013 2/16/90 08:28:07
 */

/*                                                                    
 * FUNCTION: The remove() function causes the file, whose name is pointed
 *	     to by filename, to be removed
 *
 * PARAMETERS: char *filename - file to be removed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      0 if successful
 *	      non-0 if not successful
 *
 */
/* The remove() function includes all the POSIX requirements */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/stat.h>
int	
remove(const char *filename)
{
	struct stat	stbuf;

	if (lstat((char *)filename, &stbuf) < 0)
		return (-1);
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		return(rmdir(filename));
	}
	else {
		return(unlink(filename));
	}
}
