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
static char *rcsid = "@(#)$RCSfile: srvc_req.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/27 18:09:40 $";
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
 *  ROUTINE NAME: rd_srvc_req()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles the processing of Non-standard RAID 
 *      service requests.  
 *
 *      A SRVC_REQ packet is created and filled in with the appropriate
 *      from the NSTD_RAID packet passed from the user.  The SRVC_REQ
 *      packet is passed as part of the IOCTL call to the CAM disk
 *      driver.  Upon completion, status is checked.  If an error
 *      occured, the SRVC_FLAGS are checked to see if error bit or
 *      string valid bit is set.  
 *
 *  FORMAL PARAMETERS:
 *      fd	  - open file descriptor.
 *	nstd_raid - pointer to nstd_raid data structure.
 *      len       - length of nstd_raid data structure (bytes).
 *	msg_buf	  - pointer to user buffer area to pass information
 *                  back to user process. 
 *	flag	  - pointer to flag that indicates a message is available
 *                  in msg_buf.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	0	-  Success.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *      The user process must check the flag parameter to see if there's
 *      an informational message in the msg_buf.
 *
 ************************************************************************/
int
rd_srvc_req(fd, nstd_raid, len, msg_buf, flag)
int fd;
NSTD_RAID *nstd_raid;
u_long len;
u_char *msg_buf;
int *flag;
{
	int          status;	/* status return value */
        SRVC_REQ *srvc_req;

        /*
         * Allocate a service request structure
         */
        srvc_req = (SRVC_REQ *)malloc(sizeof(SRVC_REQ));
	if(srvc_req == (SRVC_REQ *)NULL){
	    return(-1);
	}
        /* 
         * Fill in service request packet to be sent to the IOCTL
         * Currently support NSTD_RAID service type, SCSI IO subsystem,
         * and DISK driver.
         */
	bzero( srvc_req, sizeof(SRVC_REQ));
	srvc_req->srvc_op = SRVC_DISPATCH;
	strcpy(srvc_req->srvc_name, NSTD_RAID_SERVICE);
	strcpy(srvc_req->srvc_io, SCSI_IO_SUB);
        strcpy(srvc_req->srvc_drvr, DISK_DRIVER);
        srvc_req->srvc_buffer = (char *)nstd_raid;
        srvc_req->srvc_len = len;

        /*
         * Call the CAM DISK IOCTL
         */
        status = ioctl(fd, SRVC_REQUEST, srvc_req);

        if (status == -1)
	{
	        free(srvc_req);
		return(status);
	}
	else
	{
		/*
		 * Check to see if the IOCTL has an informational
		 * message to return to the user process
		 * Set the flag which indicates a message is available
		 * in the msg_buf 
		 */
		if (srvc_req->srvc_flags & STR_MSG_VALID)
		{
			bcopy(srvc_req->srvc_str, msg_buf, 128);
			*flag = 1; 
		}
	}
	/*
	 * Deallocate the srvc_req buffer
	 */
	 free(srvc_req);

	return(status);
}
