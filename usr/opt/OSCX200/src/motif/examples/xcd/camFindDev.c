/******************************************************************************
*******************************************************************************
*
*   Copyright (c) 1993 by Digital Equipment Corporation
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose and without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies, and that
*   the name of Digital Equipment Corporation not be used in advertising or
*   publicity pertaining to distribution of the document or software without
*   specific, written prior permission.
*
*   Digital Equipment Corporation makes no representations about the
*   suitability of the software described herein for any purpose.  It is
*   provided "as is" without express or implied warranty.
*  
*  DEC is a registered trademark of Digital Equipment Corporation
*  DIGITAL is a registered trademark of Digital Equipment Corporation
*  X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*
 * File:	camFindDev.c
 * Author:	Henry R. Tumblin
 * Date:	April 15,1993
 *
 * Description:
 *	Generic cam scan for devices routine.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>

#include        <io/common/iotypes.h>
#include        <io/cam/cam.h>
#include        <io/cam/uagt.h> 
#include        <io/cam/dec_cam.h>
#include        <io/cam/scsi_all.h>  

#include "camFindDev.h"

camDevTbl camFindDev(dt)
u_char dt;
{
 
/* Local variables and structures */
 
u_char bus, target;
int fd;
int nbrFound = 0;

UAGT_CAM_CCB *ua;
CCB_SCSIIO *ccb;
ALL_INQ_CDB *inq;
ALL_INQ_DATA * buf;
camDevTbl ct = NULL, tmpct;

/* Allocate the buffers		*/

    ua = (UAGT_CAM_CCB *) calloc (1,sizeof(UAGT_CAM_CCB));
    ccb = (CCB_SCSIIO *) calloc (1,sizeof(CCB_SCSIIO));
    inq = (ALL_INQ_CDB *) ccb->cam_cdb_io.cam_cdb_bytes;
    buf = (ALL_INQ_DATA *) malloc (sizeof(ALL_INQ_DATA));

/* open up the user agent cam driver */

    if ((fd = open("/dev/cam", O_RDONLY, 0))< 0){
	perror("Unable to open /dev/cam");
	return(NULL);
    }

/* Setup the user agent ccb		*/

    ua->uagt_ccb = (CCB_HEADER *) ccb;
    ua->uagt_ccblen = sizeof(CCB_SCSIIO);
    ua->uagt_buffer = (u_char *)buf;
    ua->uagt_buflen = sizeof (ALL_INQ_DATA);

/* Setup the scsi ccb			*/

    ccb->cam_ch.my_addr = (CCB_HEADER *) ccb;
    ccb->cam_ch.cam_ccb_len = sizeof(CCB_SCSIIO);
    ccb->cam_ch.cam_func_code = XPT_SCSI_IO;

    ccb->cam_ch.cam_flags = CAM_DIR_IN | CAM_DIS_AUTOSENSE;

    ccb->cam_data_ptr = (u_char *)buf;
    ccb->cam_dxfer_len = sizeof (ALL_INQ_DATA);
    ccb->cam_timeout = CAM_TIME_DEFAULT;
    ccb->cam_cdb_len = sizeof(ALL_INQ_CDB);

    inq->opcode = ALL_INQ_OP;
    inq->alloc_len = sizeof(ALL_INQ_DATA);

/*	Now loop through all bus,target,and 		*/

    for (bus=0; bus < 8; bus++){
	ccb->cam_ch.cam_path_id = bus;
	for(target=0; target<8; target++){
	    ccb->cam_ch.cam_target_id = target;
	    ccb->cam_ch.cam_target_lun = 0;
	    inq->evpd = 0;
	    inq->lun = 0;
	    inq->page = 0;
	    inq->control = 0;
	    if (ioctl (fd,UAGT_CAM_IO,(caddr_t)ua) <0) {
		close (fd);
		return (NULL);
	    }
	    if (ccb->cam_ch.cam_status == CAM_REQ_CMP &&
		buf->dtype == dt ){
		nbrFound++;
		tmpct = (camDevTbl)  calloc (nbrFound, sizeof(_camdevtbl));
		if (ct != NULL){
		    memcpy (tmpct,ct,(sizeof(_camdevtbl)*(nbrFound-1)));
		    free(ct);
		}
	        ct = tmpct;

	        ct[nbrFound-1].bus = bus;
	        ct[nbrFound-1].target = target;
	        ct[nbrFound-1].lun = 0;
 		if (buf->dtype == ALL_DTYPE_DIRECT ||
        	    buf->dtype == ALL_DTYPE_RODIRECT){
		    ct[nbrFound-1].name = (char *)malloc(16);
		    sprintf (ct[nbrFound-1].name, "/dev/rz%d",bus*8+target);
		}
		ct[nbrFound-1].inq = (ALL_INQ_DATA *) malloc(sizeof(ALL_INQ_DATA));
		memcpy (ct[nbrFound-1].inq,buf,sizeof(ALL_INQ_DATA));
	    } 
	}
    }
    close (fd);
    if (nbrFound){
	tmpct = (camDevTbl) calloc (nbrFound+1,sizeof(_camdevtbl));
	memcpy (tmpct,ct,sizeof(_camdevtbl)*nbrFound);
	ct = tmpct;
	ct[nbrFound].bus = 0xff;
    }
    return ct;
}

