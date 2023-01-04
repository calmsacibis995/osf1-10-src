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
static char *rcsid = "@(#)$RCSfile: srvc_open.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/27 18:08:55 $";
#endif

/************************* Include Files ********************************/

#include <io/common/iotypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ufs/dinode.h>
#include <io/common/devio.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/buf.h>
#include <sys/user.h>
#include <sys/param.h>
#include <io/cam/cam_nstd_raid.h>
#include <io/common/srvc.h>


/************************************************************************
 *
 *  ROUTINE NAME: rd_srvc_open()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine handles the open system call for the services 
 *	driver.  Its function is to provide a mechanism which allows
 *      a user process to gain information about and communicate  
 *	with an underlying I/O subsystem without a physical device
 *      present.
 *
 *	This routine first checks to see whether the user has superuser
 *      authority.  If not, returns EACCES.  Next, we try to open the
 *      device special file "/dev/srvc" for r/w access.  If we receive
 *      ENOENT on the open(), create the special file (character, major
 *      number = rz's #, minor number = all one's).
 *
 *  FORMAL PARAMETERS:
 *      None.
 *      
 *  IMPLICIT INPUTS:
 *      None.	
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	valid file descriptor, fd.
 *      EACCES  - Not superuser.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
int
rd_srvc_open()
{
	register int status = 0;
        int fd;
	dev_t	dev;	/* major/minor number for this special file */

        /* check for superuser privilege */

	if ((uid_t)geteuid() != (uid_t)0){
	  errno = EACCES;
	  return(-1);

	}
        /* Open the device special file with r/w access */

	fd = open ("/dev/srvc", O_RDWR);

        /* If the file descriptor is -1 an error occured, check errno */ 

        if (fd == -1)
        {

            /* If the file doesn't exist make it and open it */

            if (errno == ENOENT)
            {
		/*
		 * The major/minor number for this special file is
		 * calculated as follows:
		 * bit 20 = major number = 8 = SCSI device
		 * bits 0-19 = minor number = bus, target, lun, partition 
		 * bus, target, lun, and partition fields are set to all 
		 * ones for this special file.
		 */
		dev = ((8 << 20) | 0xfffff); 
                mknod ("/dev/srvc", (IFCHR | 0600), dev); 
		fd = open ("/dev/srvc", O_RDWR);
		if (fd == -1)
			return(-1);
		else
                	return (fd);
            }
	    return(-1);
        }
        else
           return (fd); 

}

