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
static char *rcsid = "@(#)$RCSfile: psx4_shm.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 09:25:25 $";
#endif



/*****************************************************************************
 *++
 *
 * Facility:
 *
 *	POSIX 1003.4/D11 Memory Mapped File and Shared Memory
 *      run-time library routines
 *
 * Abstract:
 *
 *	This module contains the routines to open or delete a shared
 *      memory object. 
 *
 *      Routines:
 *		shm_open() and shm_unlink()
 *
 *		
 * Author:
 *
 *	
 *
 * Modified by:
 *
 *   modifier's name,	dd-mmm-yyyy,  VERSION: svv.u-ep
 *   01 - modification description
 *
 *--
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/errno.h>


/* 
 * Constants and Macros
 */


#define SUCCESS  0
#define FAILURE -1


#define	SETERR(err)	errno = err
 
/*
 *--
 * shm_open()
 *
 * Functional description:
 *
 *	This function opens the shared memory object named by the pathname,
 *    	creating the shared memory object if necessary.
 *
 * Inputs:
 *
 *	path	- Pathname
 *	oflag	- File status flag
 *	mode	- File access mode
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	fd - The lowest numbered unused file descriptor
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */
int shm_open(const char *path, int oflag, mode_t mode)
{

int	fd;


if((oflag & O_APPEND) || (oflag & O_NONBLOCK) || (oflag & O_NOCTTY)){
	SETERR(EINVAL);
	return(FAILURE);
}


if((fd = open(path, oflag, mode)) == FAILURE )
	return(FAILURE);

(void) fcntl(fd, F_SETFD, FD_CLOEXEC);

return(fd);
}



/*
 *++++++++
 *
 * shm_unlink()
 *
 * Functional description:
 *
 *	This function deletes the shared memory object named by the pathname.
 *    	
 *
 * Inputs:
 *
 *	path	- Pathname
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	None
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *++++++++
 */


int shm_unlink(const char *path)
{
	return(unlink(path));
}









