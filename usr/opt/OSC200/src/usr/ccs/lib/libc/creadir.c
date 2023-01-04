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
static char	*sccsid = "@(#)$RCSfile: creadir.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:42:48 $";
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
 * FUNCTIONS: creadir
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * creadir.c	1.9  com/lib/c/gen,3.1,8943 10/19/89 16:57:40
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak creadir = __creadir
#endif
#include <sys/types.h>				/* for uid_t / gid_t	*/
#include <sys/stat.h>				/* for struct stat	*/
#include <sys/errno.h>				/* errno defines	*/
#include "ts_supp.h"                            /* Thread support macros*/

extern int chown();				/* system calls		*/

/*
 * NAME:	creadir
 *                                                                    
 * FUNCTION:	create a directory, specifying owner, group, modes
 *                                                                    
 * NOTES:	Searches for the directory given in 'dir', and if not
 *		there, creates it with 'owner', group' and 'mode'.
 *
 * RETURN VALUE DESCRIPTION:	0 if we succeed or if the directory
 *		already exists, else -1
 */  

int creadir(char *path, uid_t owner, gid_t group, int mode)
{
	int old_umask;			/* saved umask			*/
	struct stat path_info;		/* stat buf			*/

	/* stat to see if directory already exists */
	if(stat(path,&path_info) < 0)
	{
		/* does errno indicate a non-existent file? */
		if( TS_GETERR() == ENOENT )
		{
			/* umask(0) so that 'mode' is absolute */
			old_umask = umask((mode_t)0);

			/* attempt to make the directory */
			if(mkdir(path, (mode_t)mode) < 0)
				return(-1);

			/* attempt to chown it over */
			if(chown(path, owner, group) < 0)
				return(-1);

			/* reset the umask back to the original value */
			(void) umask((mode_t)old_umask);
		}

		else
			/* strange errno... */
			return(-1);
	}

	return(0);
}
