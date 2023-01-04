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
static char *rcsid = "@(#)$RCSfile: uagt.c,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/12/15 20:46:25 $";
#endif

/************************************************************************
 *
 *  uagt.c		Version 1.04
 *
 *  This file contains the CAM SCSI User Agent (pass-thru) driver.
 *
 *  MODIFICATION HISTORY:
 *
 *  VERSION  DATE	WHO	REASON
 *
 *  1.00     11/12/90	maria	Created from User Agent Func. Spec.
 *  1.01     03/26/91	janet	Changed name of scsi_chap6.h to scsi_status.h
 *  1.02     03/26/91	robin	Added synchronization where appropriate,
 *				& support the DEVIOCGET ioctl() command.
 *  1.03     03/31/91	jag	Change unix_bp to req_map, Rev 2.3 of cam.h
 *  1.04     04/08/91	jag	In cleanup, check for QFRZN in cam_status.
 *  1.05     04/08/91	maria	In ioctl, when a signal is caught and a
 *				terminate ccb is sent, added sleep to wait for
 *				the original request to complete.
 *				In ioctl, added check for valid XPT function
 *				code.
 *  1.06     04/10/91	maria	In ioctl, added check for target id and
 *				lun being within the legal range.
 *  1.07     05/24/91	maria	Moved the ccmn_ccbwait function to the
 *				peripheral common file.
 *  1.08     06/20/91	robin	Moved CamStatus() function to cam_debug.c
 *				& changed uprintf to not use this function.
 *  1.09     07/26/91	maria	Created uagt_ccbwait() since common
 *				routine requires a peripheral device structure
 * 				pointer to SMPize the routine.
 *  1.10     09/03/91	maria	Removed the use of "uagt_num_open" on
 *				close to determine when to unfreeze the
 *				SIM qeueue since close will only be called
 *				on the very last close.
 *  1.11     10/22/91	janet	Replaced "register status" with "register
 *				int status"
 *  1.12     11/19/91	maria	Added new ioctls for scanning a bus or nexus.
 *				Added use of user address for GET DEVICE TYPE 
 *				ccb data with lock and unlock.
 *				Allow sleeps at a non-interruptable
 *				priority if requested.
 *				Added code to save the orignal values of
 *				fields in the ccb which get changed within 
 *				the user agent.  These fields are set to
 *				original value when returning to user.
 *				Added support for the TERMINATE I/O and
 *				ABORT ccbs.
 *  1.13     12/03/91	maria	Send an Abort ccb instead of terminate
 *				I/O for CNTL-C.
 *				Lock cdb area if a pointer.
 *
 *  1.14     12/09/91	maria	Allocate a system buffer for CDB's which
 *				are pointers and copy them into system space.
 *
 ************************************************************************/

/************************* Include Files ********************************/

#include <io/common/iotypes.h>
#include <sys/types.h>
#include <io/common/devio.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <mach/vm_param.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/cam_debug.h>
#include <io/cam/uagt.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <io/cam/pdrv.h>
#include <io/cam/ccfg.h>		/* CAM configuration include file */
#include <vm/vm_kern.h>

/************************* Local Defines ********************************/

#ifndef NULL
#define NULL 0
#endif

#define MAX_UAGT_REQ	25	/* max oustanding requests in UA */
				/* may need to make this a system */
				/* tunable parameter */

#define UAGT_ABORT_INPROG  1	/* indicate abort in progress */
#define UAGT_DONE	   2	/* indicate request was completed */
#define UAGT_SIMQ_FRZ	   4	/* indicate SIM Queue is frozen */

#define DEV_UAGT	"UAGT"		/* CAM User Agent driver.	*/

/********************** External Declarations ***************************/

extern	CCB_HEADER 	*xpt_ccb_alloc();
extern 	U32 		xpt_ccb_free();
extern 	U32		xpt_action();
extern	int		copyin();
extern	int		copyout();
extern	int		wakeup();
extern	int		vslock();
extern	int		vsunlock();
extern	int		useracc();
extern  caddr_t		cdbg_CamStatus();

/****************** Initialized and Unitialized Data ********************/

/* User Agent request structure */
typedef struct uagt_req  {
	UAGT_CAM_CCB 	*uccb;		/* UA CCB structure passed to ioctl */ 
	CCB_HEADER 	*ccb;		/* users CCB structure */
	struct uagt_req	*next;		/* pointer to next UA request */
					/* in freelist or uagt_pend_list */
	U32	   	flags;		/* indicates aborting or done */
	u_char 		bus;		/* needed for Release SIM */
	u_char 		target;		/* Queue CCB which is issued */
	u_char 		lun;		/* on close if queue left frozen */
	struct buf 	bp;		/* Buf ptr needed for DME */
	char *		u_data;		/* user ptr for data */
	u_char *	cdb_ptr;	/* ptr for cdb */
	void		(*cbfcnp)();	/* Users callback function */
	U32		cam_flags;	/* Users cam flags value */
	CCB_HEADER 	*abort_term_ccb;/* users CCB structure */
} UAGT_REQ;

UAGT_REQ uagt_req[MAX_UAGT_REQ];

UAGT_REQ *uagt_freelist;	/* free list of UA request structures */

UAGT_REQ *uagt_pendlist;	/* I/O pending list of UA request structures */

UAGT_REQ *uagt_locklist;	/* List of frozen queues */

u_char	 uagt_initialized=0;	/* used to initialize the uagt_freelist */

UAGT_REQ *uagt_ins_queue();
UAGT_REQ *uagt_get_queue_entry();
void     uagt_rem_queue();
void     uagtcomplete();	/* UAgt driver callback completion routine */

/************************************************************************
 *
 *  ROUTINE NAME: uagtopen()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine handles the open system call for the user agent
 *	driver.  Its functions are to create the free list of user 
 *	agent request structures on the very first open.
 *
 *  FORMAL PARAMETERS:
 *	dev - major/minor device number.
 *	flag - unused.
 *
 *  IMPLICIT INPUTS:
 *	uagt_initialized - indicates whether queues initialized
 *	uagt_req - array of user agent request structures.
 *
 *  IMPLICIT OUTPUTS:
 *	uagt_freelist - points to first element in the free list of
 *		      user agent reqeust structures.
 *
 *  RETURN VALUE:
 *	0	- indicates success.
 *
 *  SIDE EFFECTS:
 *	User agent request free list is created on first open.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
int
uagtopen(dev, flag)
register dev_t dev;
register int flag;
{
	register int status = 0;

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
		("[b/t/l] uagtopen: enter\n"));

	/* 
	 * Create linked free list.
	 */
	if (!uagt_initialized)	{
		int i;

		uagt_locklist = NULL;
		uagt_pendlist = NULL;
		uagt_freelist = &uagt_req[0];
		for(i=0; i<(MAX_UAGT_REQ-1); i++)
			uagt_req[i].next = &uagt_req[i+1];
		uagt_req[i].next = NULL;

		uagt_initialized = 1;	/* indicate opened */
	}

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
		("[b/t/l] uagtopen: exit\n"));

	return (status);
}

/************************************************************************
 *
 *  ROUTINE NAME: uagtclose()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine handles the close system call for the user agent
 *	driver.  It ensures the SIM queues is released incase it's frozen
 *	due to an error condition.
 *
 *  FORMAL PARAMETERS:
 *	dev - major/minor device number.
 *	flag - unused.
 *
 *  IMPLICIT INPUTS:
 *	uagt_locklist - list of frozen queues.
 *
 *  IMPLICIT OUTPUTS:
 *	none
 *
 *  RETURN VALUE:
 *	0	- indicates success.
 *
 *  SIDE EFFECTS:
 *	All SIM queues for the user agent are released.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
int
uagtclose(dev, flag)
dev_t dev;
int flag;
{
	register CCB_RELSIM *rsq_ccb;
	register UAGT_REQ *uarp;
	register int status = 0;

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
		("[b/t/l] uagtclose: enter\n"));

	/*
	 * Check whetehr we have any frozen SIM Queues lying around and
	 * unfreeze them.
	 */
	if( uagt_locklist )  {
		rsq_ccb = (CCB_RELSIM *)xpt_ccb_alloc();
		rsq_ccb->cam_ch.my_addr = (CCB_HEADER *)rsq_ccb;
		rsq_ccb->cam_ch.cam_ccb_len = (u_short)sizeof(CCB_RELSIM);
		rsq_ccb->cam_ch.cam_func_code = (u_char)XPT_REL_SIMQ;

		while( uagt_locklist )  {
			rsq_ccb->cam_ch.cam_path_id = uagt_locklist->bus;
			rsq_ccb->cam_ch.cam_target_id = uagt_locklist->target;
			rsq_ccb->cam_ch.cam_target_lun = uagt_locklist->lun;
			/*
		 	 * Send to XPT. Release SIM Queue CCB should always
		 	 * return success.
		 	 */
			PRINTD(uagt_locklist->bus, uagt_locklist->target,
			    uagt_locklist->lun, CAMD_FLOW,
			    ("[%d/%d/%d] uagtclose: Send RELSIMQ\n",
				uagt_locklist->bus,
				uagt_locklist->target,
				uagt_locklist->lun));
			xpt_action(rsq_ccb);
			if( (rsq_ccb->cam_ch.cam_status&CAM_STATUS_MASK) 
				!= CAM_REQ_CMP )
				PRINTD(uagt_locklist->bus,uagt_locklist->target,
			    	   uagt_locklist->lun, CAMD_ERRORS,
				   ("[%d/%d/%d] uagtclose: REL SIMQ failed\n",
				   uagt_locklist->bus,
				   uagt_locklist->target, uagt_locklist->lun));
			/*
			 * Get next SIM Queue to be released.
			 */
			uarp = uagt_locklist;
			uagt_locklist = uagt_locklist->next;
			/*
			 * Null out the request structure and put it back
			 * on free list.
			 */
			bzero(uarp, sizeof(UAGT_REQ));
			uarp->next = uagt_freelist;
			uagt_freelist = uarp;
		}

		xpt_ccb_free(rsq_ccb);
	}

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("[b/t/l] uagtclose: exit\n"));

	return (status);
}

/************************************************************************
 *
 *  ROUTINE NAME: uagtioctl()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles the ioctl() system call for the user 
 *	agent driver.  The following commands are supported:
 *	    DEVIOCGET   returns a devget structure for the user agent.
 *	    UAGT_CAM_IO which will allow the pass thru of a user created CCB
 *		 	to the CAM XPT layer for processing.  For SCSI I/O
 *		 	and GET DEVICE TYPE ccbs, the user data area is locked
 *			before passing the CCB to the XPT.  The user agent
 *			will sleep waiting for the I/O to complete and will
 *			issue a TERMINATE I/O CCB if a signal is caught while
 *			sleeping. For ABORT and TERMINATE I/O ccbs from
 *			the user, the ccb address is checked.  If the address
 *			is a kernel address it will be used.  If the address
 *			is a user level address, the corresponding kernel 
 *			address will be used.
 *	    UAGT_CAM_FULL_SCAN  calls the configuration driver to scan a
 *				controller.
 *	    UAGT_CAM_SINGLE_SCAN  calls the configuration driver to scan a
 *				  b/t/l.
 *
 *  FORMAL PARAMETERS:
 *  	dev	- major/minor device number.
 *	cmd	- the ioctl command.
 *	data	- pointer to user data structure.
 *	flag	- unused.
 *
 *  IMPLICIT INPUTS:
	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	EFAULT	-  Copy to/from user space failed.
 *		-  User does not have access to data area indicated
 *		   in CCB.
 *	EINVAL	-  An unsupported cmd value was passed to ioctl().
 *		-  An invalid XPT function code was contained in the CCB.
 *		-  An out of range target id or lun.
 *	EBUSY	-  The maximum allowable requests in user agent requests
 *		   has been reached (MAX_UAGT_REQ).
 *	0	-  Success.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	This routine assumes that the xpt_ccb_alloc() routine will never
 *	return NULL!
 *
 ************************************************************************/
int
uagtioctl(dev, cmd, data, flag)
dev_t dev;
unsigned cmd;
caddr_t data;
int flag;
{
	UAGT_CAM_CCB *uccb;	/* UA CAM CCB pointed to by data */
	CCB_HEADER   *ccb;	/* pointer to CCB obtained from XPT */
	UAGT_REQ     *uarp;	/* pointer to UA request entry */
	PDRV_WS	     *updrv_ws, *pdrv_ws;  /* Pointer to ccb working set area */
	int          status;	/* status return value */
	int	     s;

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, 
	    ("[b/t/l] uagtioctl: enter cmd = 0x%x, data = 0x%lx\n", cmd, data));

	/* 
	 * Decode the I/O command.
	 */
	switch (cmd) {

	    case DEVIOCGET: {				/* Device Status */
		register struct devget *devget;

		devget = (struct devget *)data;
		bzero (devget, sizeof(struct devget));
		devget->category = DEV_SPECIAL; 	/* special xpr	*/
		devget->bus = DEV_SCSI;			/* SCSI bus 	*/
		bcopy (DEV_SCSI_GEN, devget->interface,
			strlen(DEV_SCSI_GEN));		/* generic SCSI	*/
		bcopy (DEV_UAGT, devget->device,
			strlen(DEV_UAGT));		/* uagt dev.	*/
		devget->adpt_num = -1;			/* n/a		*/
		devget->nexus_num = -1; 		/* n/a		*/
		devget->bus_num = -1;			/* n/a		*/
		devget->ctlr_num = -1;			/* n/a		*/
		devget->slave_num = -1; 		/* n/a		*/
		bcopy (DEV_UAGT, devget->dev_name,
			strlen(DEV_UAGT));		/* Ultrix name	*/
		devget->unit_num = 0;			/* always 0	*/
		devget->soft_count = 0; 		/* always 0	*/
		devget->hard_count = 0; 		/* always 0	*/
		devget->stat = 0;			/* always 0	*/
		devget->category_stat = 0;		/* always 0	*/
		return (0);
		/*NOTREACHED*/
	    }
	    case UAGT_CAM_FULL_SCAN:
	    {
		UAGT_CAM_SCAN *ucs = (UAGT_CAM_SCAN *)data;
		ccfg_edtscan(EDT_FULLSCAN, ucs->ucs_bus, 0, 0);
		return(0);
		/*NOTREACHED*/
	    }
	    case UAGT_CAM_SINGLE_SCAN:
	    {
		UAGT_CAM_SCAN *ucs = (UAGT_CAM_SCAN *)data;
		ccfg_edtscan(EDT_SINGLESCAN, ucs->ucs_bus, 
	             ucs->ucs_target, ucs->ucs_lun);
		return(0);
		/*NOTREACHED*/
	    }
	    case UAGT_CAM_IO:
		break;				/* Handled below. */

	    default:
		return (EINVAL);
		/*NOTREACHED*/
	}

	/* 
	 * Allocate CCB from XPT layer.
	 */
	ccb = xpt_ccb_alloc();

	uccb = (UAGT_CAM_CCB *)data;

	/*
 	 * Save the pdrv_ws ptr.....
	 */
	pdrv_ws = (PDRV_WS *)((CCB_SCSIIO *)ccb)->cam_pdrv_ptr;

	/*
	 * Copyin user's CCB structure to kernel allocated CCB structure.
	 */
	if ( (uccb->uagt_ccb == (CCB_HEADER *)NULL)
	   || (copyin(uccb->uagt_ccb, ccb, uccb->uagt_ccblen)) )  {
		/*
		 * User's ccb pointer is bad.
		 */
		xpt_ccb_free(ccb);
		PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS, 
			("[b/t/l] uagtioctl: copyin() failed\n"));
		return(EFAULT);
	}
	/* 
	 * Save away users working set........
	 */
	updrv_ws = (PDRV_WS *)((CCB_SCSIIO *)ccb)->cam_pdrv_ptr;

	/*
	 * Check for a valid target and lun id.
	 */
	if( (ccb->cam_target_id >= NDPS) ||
   	    (ccb->cam_target_lun >= NLPT) )	{
   		xpt_ccb_free(ccb);
		PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS, 
			("[b/t/l] uagtioctl: Invalid target/lun ID\n"));
		return(EINVAL);
	}

	switch (ccb->cam_func_code)   {
		case XPT_SCSI_IO:
		{
			CCB_SCSIIO *ioccb = (CCB_SCSIIO *)ccb;

			/* 
			 * Stuff back in after copy the working set pointer....
			 */
			(caddr_t)((CCB_SCSIIO *)ccb)->cam_pdrv_ptr = (caddr_t)pdrv_ws;

			/* 
			 * Verify user access and lock user pages 
			 * if needed.
			 */
			if( !uagt_lock(uccb) )  {
				return(EFAULT);
			}

			/* 
			 * Insert into UA request queue.
			 */
			if ( (uarp = uagt_ins_queue(uccb, ccb)) == (UAGT_REQ *)NULL )  {
			    PRINTD(ccb->cam_path_id, ccb->cam_target_id,
			       ccb->cam_target_lun, CAMD_ERRORS, 
			       ("[%d/%d/%d] uagtioctl: uagt_ins_queue failed\n",
			       ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));
			    xpt_ccb_free(ccb);
			    uagt_unlock(uccb);
			    return(EBUSY);	/* reached max allowed limit */
			}

			/*
			 * If the CDB is pointer allocate a system
			 * buffer and copy CDB data into it.
			 */
			if (ioccb->cam_ch.cam_flags & CAM_CDB_POINTER)   {
			    int error;
			    uarp->cdb_ptr = ioccb->cam_cdb_io.cam_cdb_ptr;
			    ioccb->cam_cdb_io.cam_cdb_ptr =
				    (u_char *)cam_zalloc(ioccb->cam_cdb_len);
			    error = copyin(uarp->cdb_ptr, 
						ioccb->cam_cdb_io.cam_cdb_ptr,
						ioccb->cam_cdb_len);
			    if (error) {
				uagt_unlock(uarp->uccb);
				uagt_rem_queue(ccb);
				xpt_ccb_free(ccb);
				return (error);
			    }
			}

			CALLD (ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, 
				CAMD_CMD_EXP, cdbg_DumpSCSIIO(ccb));

			uarp->cbfcnp = ioccb->cam_cbfcnp;
			uarp->cam_flags = ioccb->cam_ch.cam_flags;
			/*
			 * Make sure UAgt driver gets called back and
			 * set the driver callback completion routine.
			 */
			ioccb->cam_ch.cam_flags &= ~CAM_DIS_CALLBACK;
			ioccb->cam_cbfcnp = uagtcomplete;

			break;
		}
		case XPT_REL_SIMQ:
		{
			UAGT_REQ *temp, *tuarp;

			temp = tuarp = uagt_locklist;
			while (tuarp)	{
				if( (tuarp->bus == ccb->cam_path_id) &&
				    (tuarp->target == ccb->cam_target_id) &&
			    	    (tuarp->lun == ccb->cam_target_lun) )  {
					/*
			 		 * Check if it is the first element
					 * and remove from list.
			 		 */
					if( tuarp == uagt_locklist )
						uagt_locklist = tuarp->next;
					else
						temp->next = tuarp->next;
					break;
				}  else  {
					temp = tuarp;
					tuarp = tuarp->next;
				}
			}
			/*
			 * Place request back on free list 
			 */
			if( tuarp )  {	
				bzero(tuarp, sizeof(UAGT_REQ));
				tuarp->next = uagt_freelist;
				uagt_freelist = tuarp;
			}
			/* 
			 * Insert Release Queue CCB into UA request queue.
			 */
			if ( (uarp = uagt_ins_queue(uccb, ccb)) 
			    == (UAGT_REQ *)NULL )  {
			    PRINTD(ccb->cam_path_id, ccb->cam_target_id,
			      ccb->cam_target_lun, CAMD_ERRORS, 
			      ("[%d/%d/%d] uagtioctl: uagt_ins_queue failed\n",
			      ccb->cam_path_id,ccb->cam_target_id, ccb->cam_target_lun));
			    xpt_ccb_free(ccb);
			    return(EBUSY);	/* reached max allowed limit */
			}
			uarp->cam_flags = ccb->cam_flags;
			break;
		}
		case XPT_ABORT:
		{
			UAGT_REQ *ureq;

			/*
			 * If this is a user ccb then find its
			 * corresponding kernel ccb.
			 */
			if(CAM_IS_KUSEG(((CCB_ABORT *)ccb)->cam_abort_ch))  {
			   if( (ureq = uagt_get_queue_entry( 
				((CCB_ABORT *)ccb)->cam_abort_ch)) 
				== (UAGT_REQ *)NULL)  {
				return(EINVAL);
			   }
			   ureq->abort_term_ccb = 
				(CCB_HEADER *)((CCB_ABORT *)ccb)->cam_abort_ch;
			   ((CCB_ABORT *)ccb)->cam_abort_ch  = ureq->ccb;
			}
			/* 
			 * Insert into UA request queue.
			 */
			if ( (uarp = uagt_ins_queue(uccb, ccb)) 
			    == (UAGT_REQ *)NULL )  {
			    PRINTD(ccb->cam_path_id, ccb->cam_target_id,
			      ccb->cam_target_lun, CAMD_ERRORS, 
			      ("[%d/%d/%d] uagtioctl: uagt_ins_queue failed\n",
			      ccb->cam_path_id,ccb->cam_target_id, ccb->cam_target_lun));
			    xpt_ccb_free(ccb);
			    return(EBUSY);	/* reached max allowed limit */
			}
			uarp->cam_flags = ccb->cam_flags;
			break;
		}
		case XPT_TERM_IO:
		{
			UAGT_REQ *ureq;
		
			/*
			 * If this is a user ccb then find its
			 * corresponding kernel ccb.
			 */
			if(CAM_IS_KUSEG(((CCB_TERMIO *)ccb)->cam_termio_ch))  {
			   if( (ureq = uagt_get_queue_entry( 
				((CCB_TERMIO *)ccb)->cam_termio_ch)) 
				== (UAGT_REQ *)NULL)  {
				return(EINVAL);
			   }
			   ((CCB_TERMIO *)ccb)->cam_termio_ch  = ureq->ccb;
			}
			/* 
			 * Insert into UA request queue.
			 */
			if ( (uarp = uagt_ins_queue(uccb, ccb)) 
			    == (UAGT_REQ *)NULL )  {
			    PRINTD(ccb->cam_path_id, ccb->cam_target_id,
			      ccb->cam_target_lun, CAMD_ERRORS, 
			      ("[%d/%d/%d] uagtioctl: uagt_ins_queue failed\n",
			      ccb->cam_path_id,ccb->cam_target_id, ccb->cam_target_lun));
			    xpt_ccb_free(ccb);
			    return(EBUSY);	/* reached max allowed limit */
			}
			uarp->cam_flags = ccb->cam_flags;
			break;
		}
		case XPT_GDEV_TYPE:
			/* 
			 * Verify user access and lock user page
			 * in which to copy inquiry data.
			 */
			if( !uagt_lock(uccb) )  {
				return(EFAULT);
			}
			/* FALL THRU */
		case XPT_PATH_INQ:
		case XPT_SASYNC_CB:
		case XPT_SDEV_TYPE:
		case XPT_RESET_BUS:
		case XPT_RESET_DEV:
		case XPT_NOOP:
			/* 
			 * Insert into UA request queue.
			 */
			if ( (uarp = uagt_ins_queue(uccb, ccb)) 
			    == (UAGT_REQ *)NULL )  {
			    PRINTD(ccb->cam_path_id, ccb->cam_target_id,
			      ccb->cam_target_lun, CAMD_ERRORS, 
			      ("[%d/%d/%d] uagtioctl: uagt_ins_queue failed\n",
			      ccb->cam_path_id,ccb->cam_target_id, ccb->cam_target_lun));
			    xpt_ccb_free(ccb);
			    return(EBUSY);	/* reached max allowed limit */
			}
			uarp->cam_flags = ccb->cam_flags;
			break;
		default:
			xpt_ccb_free(ccb);
			return(EINVAL);
	}	/* switch */

	/* 
	 * Issue request to the XPT and wait
	 */
	if( uccb->uagt_flags & UAGT_NO_INT_SLEEP )
		status = uagt_send_ccb_wait(ccb, PZERO);
	else
		status = uagt_send_ccb_wait(ccb, PZERO+1);

	/* 
	 * Check if a signal was caught and issue an Abort Request.
	 */
	if( status == EINTR )  {

		CCB_ABORT  *abort_ccb;
		UAGT_REQ   *uarp;
		PDRV_WS    *pws;

		PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS, 
			("[b/t/l] uagtioctl: signal caught\n"));

		/*
		 * Allocate ABORT CCB.
		 */
		abort_ccb = (CCB_ABORT *)xpt_ccb_alloc();

		if ( (uarp = uagt_get_queue_entry(ccb)) == (UAGT_REQ *)NULL)  {
			PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS, 
			("[b/t/l] uagtioctl: No CCB to terminate\n"));
			return(0);
		}

		uarp->flags |= UAGT_ABORT_INPROG;

		/* 
		 * Copy CCB header from ccb to be terminated.
		 */
		bcopy(ccb, &abort_ccb->cam_ch, sizeof(CCB_HEADER));

		abort_ccb->cam_ch.my_addr = (CCB_HEADER *)abort_ccb;
		abort_ccb->cam_ch.cam_ccb_len = (u_short)sizeof(CCB_ABORT);
		abort_ccb->cam_ch.cam_func_code = (u_char)XPT_ABORT;
		abort_ccb->cam_abort_ch = uarp->ccb;

		/*
		 * Send the Abort CCB to the XPT.
		 */
		xpt_action((CCB_HEADER *)abort_ccb); 

		/*
		 * CAM status must be complete for ABORT CCB.
		 */
		if( CAM_STATUS(abort_ccb) != CAM_REQ_CMP )  {
		    PRINTD(abort_ccb->cam_ch.cam_path_id,
			abort_ccb->cam_ch.cam_target_id,
		    	abort_ccb->cam_ch.cam_target_lun, CAMD_ERRORS, 
			("[%d/%d/%d] uagtioctl: Terminate I/O CCB failed, cam_status = 0x%x (%s)\n",
		    	abort_ccb->cam_ch.cam_path_id,
		    	abort_ccb->cam_ch.cam_target_id,
		    	abort_ccb->cam_ch.cam_target_lun,
			ccb->cam_status,
			cdbg_CamStatus(ccb->cam_status, CDBG_BRIEF)));
		}

		/*
		 * Return the ABORT  CCB to XPT.
		 */
		xpt_ccb_free((CCB_HEADER *)abort_ccb);

		s = splbio();

		/*
		 * Go back to sleep non-interruptible to wait for the 
		 * request to complete if the callback has not already
		 * occurred. The CCB can only be an CCB_SCSIIO.
		 */
		pws = (PDRV_WS *)((CCB_SCSIIO *)ccb)->cam_pdrv_ptr;
		if(( pws->pws_flags & PWS_CALLBACK) == NULL )   {
			(void) mpsleep(ccb, PZERO, "Zzzzzz", 0, 
				(void *)0, 0);
		}
		splx(s);

	}

	ccb->my_addr = uccb->uagt_ccb;
	/*
	 * Clean up the ccb before copying to user.
	 */
	if (ccb->cam_func_code == XPT_SCSI_IO)  {
		CCB_SCSIIO *io_ccb = (CCB_SCSIIO *)ccb;
		/*
 		 * Restore users working set....
		 */
		(caddr_t)((CCB_SCSIIO *)ccb)->cam_pdrv_ptr = (caddr_t)updrv_ws;

		uarp->cam_flags = uarp->cam_flags;
		io_ccb->cam_cbfcnp = uarp->cbfcnp;
		io_ccb->cam_req_map = 0;
		if (io_ccb->cam_ch.cam_flags & CAM_CDB_POINTER)  {
		        cam_zfree((char *)io_ccb->cam_cdb_io.cam_cdb_ptr,
				  io_ccb->cam_cdb_len);
			io_ccb->cam_cdb_io.cam_cdb_ptr = (u_char *)uarp->cdb_ptr;
		}
		CALLD (ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, 
				CAMD_CMD_EXP, cdbg_DumpSCSIIO(ccb));
	}  
	else if( (ccb->cam_func_code == XPT_TERM_IO)
	    || (ccb->cam_func_code == XPT_ABORT) )  {
	    if(uarp->abort_term_ccb != (CCB_HEADER *)NULL)
	        ((CCB_ABORT *)ccb)->cam_abort_ch = uarp->abort_term_ccb;
	}
	

	/* 
	 * Copy ccb back to user space.
	 */
	if (copyout((caddr_t)ccb, (caddr_t)uccb->uagt_ccb, 
	    uccb->uagt_ccblen) )  {
		PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, 
		   CAMD_ERRORS, ("[%d/%d/%d] uagtioctl: copyout() failed\n",
		   ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));
		(void)uagt_cleanup(ccb);
		return(EFAULT);
	}

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("[b/t/l] uagtioctl: exiting\n"));
	/*
	 * Handle all UA driver cleanup.
	 */
	(void)uagt_cleanup(ccb);
	return(status);
}

/************************************************************************
 *
 *  ROUTINE NAME: uagtcomplete()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine is the call back on completion handler for the user
 *	agent driver.  Its only functions are to indicate the request
 *	is DONE and call wakeup() to wake the process sleeping on the
 *	completion of the request.  This routine is called at the HBA
 *	interrupt level.
 *
 *  FORMAL PARAMETERS:
 *	ccb - CCB pointer of the completed request.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
void
uagtcomplete(ccb) 
register CCB_SCSIIO *ccb;
{
	register UAGT_REQ *uarp;
	PDRV_WS *pws;

	PRINTD(ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
		ccb->cam_ch.cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagtcomplete: enter, ccb = 0x%lx\n", 
		ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
		ccb->cam_ch.cam_target_lun, ccb));

	if ( (uarp = uagt_get_queue_entry(ccb)) == (UAGT_REQ *)NULL) 
		PRINTD(ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id, 
			ccb->cam_ch.cam_target_lun, CAMD_ERRORS,
			("[%d/%d/%d] uagtcomplete: No entry for ccb = 0x%lx\n",
			ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id, 
			ccb->cam_ch.cam_target_lun, ccb));

	if ( uarp != (UAGT_REQ *)NULL)
		uarp->flags |= UAGT_DONE;	/* indicate done */

	/*
	 * Indicate this ccb has had a callback.
 	 */
	((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_flags |= PWS_CALLBACK;

	(void)wakeup(ccb);

	PRINTD(ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
		ccb->cam_ch.cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagtcomplete: exit\n",
		ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
		ccb->cam_ch.cam_target_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME: uagt_cleanup()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles the clean up for the user agent driver for
 *	requests which have returned from the CAM subsystem.  For SCSI
 *	I/O GET DEVICE TYPE  requests, the data area is unlocked. For all
 *	CCBs, the entry in the request queue(uagt_pend_list) is removed and 
 *	the CCB is deallocated to the XPT layer.
 *
 *  FORMAL PARAMETERS:
 *	ccb - CCB pointer which has completed.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	EFAULT	- Copyout to user space fails for Get Device Type CCB.
 *	ENXIO	- No entry for request in uagt_pend_list.
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
int
uagt_cleanup(ccb)
register CCB_HEADER *ccb;
{
	register UAGT_REQ *uarp;

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagt_cleanup: enter, ccb = 0x%lx\n", 
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, ccb));

	if( (uarp = uagt_get_queue_entry(ccb)) == (UAGT_REQ *)NULL)  {
		PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, 
			CAMD_ERRORS, 
			("[%d/%d/%d] uagt_cleanup: No entry for ccb=0x%lx\n",
			ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, ccb));
		xpt_ccb_free(ccb);
		return(ENXIO);
	}

	/*
	 * For the Get Device Type CCB, we need to unlock the inquiry data
	 * area in user space.
	 */
	if ( ccb->cam_func_code == XPT_GDEV_TYPE )  {
		uagt_unlock(uarp->uccb);
	}

	/*
	 * Unlock data area and sense area for SCSI I/O CCB.
	 */
	if (ccb->cam_func_code == XPT_SCSI_IO)  {
		uagt_unlock(uarp->uccb);
		/*
		 * Check if the SIM queue is frozen which means we may
		 * need to issue a Release SIM Queue CCB on close.
		 */
		if( ccb->cam_status & CAM_SIM_QFRZN )  {
			/*
			 * Save away  b/t/l info
			 */
			uarp->bus = ccb->cam_path_id;
			uarp->target = ccb->cam_target_id;
			uarp->lun = ccb->cam_target_lun;
			uarp->flags = UAGT_SIMQ_FRZ;
		}
	}

	/*
	 * Remove request from UA queue.
	 */
	uagt_rem_queue(ccb);

	/*
	 * Return CCB to XPT layer.
	 */
	xpt_ccb_free(ccb);

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagt_cleanup: exit\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));

	return(0);
}

/************************************************************************
 *
 *  ROUTINE NAME: uagt_lock()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles the locking down of pages of the SCSI 
 *	I/O CCB for the the data area and sense data area and the
 *	inquiry data area for the GET DEVICE TYPE CCB.
 *
 *  FORMAL PARAMETERS:
 *	uccb	- user agent CAM CCB request pointer.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *		0	- User does not have read/write access to address
 *			  specified for the data buffer or sense buffer.
 *		1	- Success.
 *
 *  SIDE EFFECTS :
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
int
uagt_lock(uccb)
UAGT_CAM_CCB *uccb;
{
	int direct;	/* direction of transfer */

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("[b/t/l] uagt_lock: enter\n"));

	if (uccb->uagt_ccb->cam_func_code == XPT_GDEV_TYPE)   {
		CCB_GETDEV *get_ccb = (CCB_GETDEV *)uccb->uagt_ccb;
		/*
		 * Check user has access to inquiry data area.
		 */
		if( !CAM_VM_USERACC(get_ccb->cam_inq_data, INQLEN, B_WRITE) ) {
			PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS, 
				("[b/t/l] uagt_lock: useracc inquiry data failed\n"));
				return(0);	
		}
		/*
		 * Lock inquiry data area.
		 */
		CAM_VM_LOCK(get_ccb->cam_inq_data, INQLEN);
		return(1);
	}
	/* 
	 * Verify user access and lock user pages if needed.
	 */
	if( (uccb->uagt_buffer != NULL) && (uccb->uagt_buflen != 0) )  {
		/*
		 * Verify that the data direction bit is set.
		 */
		if( uccb->uagt_ccb->cam_flags & CAM_DIR_IN )
			direct = B_WRITE;
		else 
		if( uccb->uagt_ccb->cam_flags & CAM_DIR_OUT )
			direct = B_READ;
		else  {
			PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS,
				("[b/t/l] uagt_lock: No direction in flags\n"));
				return(0);	
		}
		/*
		 * Check user has access to data area.
		 */
		if( !CAM_VM_USERACC(uccb->uagt_buffer, uccb->uagt_buflen,
				    direct) ) {
			PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS, 
				("[b/t/l] uagt_lock: useracc data failed\n"));
				return(0);	
		}
		/*
		 * Lock data area.
		 */
		CAM_VM_LOCK(uccb->uagt_buffer, uccb->uagt_buflen);
	}
	/*
	 * Check that autosense is enabled.
	 */
	if( !(uccb->uagt_ccb->cam_flags & CAM_DIS_AUTOSENSE) )  {
		if( (uccb->uagt_snsbuf == NULL) || (uccb->uagt_snslen == 0) ) {
			PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS,
				("[b/t/l/] uagt_lock: No sense values\n"));
			return(0);	
		}
		/*
		 * Check user has access to sense data area.
		 */
		if( !CAM_VM_USERACC((u_char *)uccb->uagt_snsbuf,
		    	(U32)uccb->uagt_snslen, B_WRITE) )  {
			PRINTD(NOBTL, NOBTL, NOBTL, CAMD_ERRORS, 
				("[b/t/l] uagt_lock: useracc sense data failed\n"));
			return(0);
		}
		/*
		 * Lock sense data area.
		 */
		CAM_VM_LOCK(uccb->uagt_snsbuf, uccb->uagt_snslen);
	}

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("[b/t/l] uagt_lock: exit\n"));

	return(1);	/* success */
}

/************************************************************************
 *
 *  ROUTINE NAME: uagt_unlock()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles the unlocking of pages for the SCSI 
 *	I/O CCB for the the data area and sense data area or the inquiry
 *	data area for the GET DEVICE TYPE CCB.
 *
 *  FORMAL PARAMETERS:
 *	uccb	- user agent CAM CCB request pointer.
 *
 *  FORMAL PARAMETERS:
 *	uccb	- Pointer to user Agent CAM CCB passed from user.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
int
uagt_unlock(uccb)
UAGT_CAM_CCB *uccb;
{
	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("[b/t/l] uagt_unlock: enter\n"));

	if (uccb->uagt_ccb->cam_func_code == XPT_GDEV_TYPE)   {
		CAM_VM_UNLOCK(((CCB_GETDEV *)uccb->uagt_ccb)->cam_inq_data, 
			INQLEN, B_WRITE);
		return;
	}
	/*
	 * Check whether we need to unlock data area for SCSI I/O ccb.
	 */
	if( (uccb->uagt_buffer != NULL) && (uccb->uagt_buflen != 0) ) {
		if( uccb->uagt_ccb->cam_flags & CAM_DIR_IN ) {
		   CAM_VM_UNLOCK(uccb->uagt_buffer, uccb->uagt_buflen, B_WRITE);
		} else {
		   CAM_VM_UNLOCK(uccb->uagt_buffer, uccb->uagt_buflen, B_READ);
		}
	}
	/* 
	 * Check whether we need to unlock sense data area.
	 */
	if( !(uccb->uagt_ccb->cam_flags & CAM_DIS_AUTOSENSE) )  {
		CAM_VM_UNLOCK(uccb->uagt_snsbuf, uccb->uagt_snslen, B_WRITE);
	}

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("[b/t/l] uagt_unlock: exit\n"));
}

/************************************************************************
 *
 *  ROUTINE NAME: uagt_ins_queue()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will allocate and fill in a UA request entry.  It
 *	will also insert the newly allocated entry onto the pending list
 *	(uagt_pendlist).
 *
 *  FORMAL PARAMETERS:
 *	uccb  -  user agent CAM request.
 *	ccb   -  pointer to CCB which will be sent to XPT.
 *
 *  IMPLICIT INPUTS:
 *	User agent freelist (us_free_list) is referenced.
 *
 *  IMPLICIT OUTPUTS:
 *	User agent pending lust (uagt_pendlist) is modififed.
 *	User agent freelist (us_free_list) is modified.
 *
 *  RETURN VALUE:
 *	UAGT_REQ  - Pointer to entry in UA request pending list.
 *	NULL	  - Maximum exceeded.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/

UAGT_REQ *
uagt_ins_queue(uccb, ccb)
register UAGT_CAM_CCB *uccb;
register CCB_HEADER *ccb;
{
	register UAGT_REQ *uarp;
	register struct buf *bp;
	register int s;

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagt_ins_queue: enter\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));

	s = splbio();
	/* 
	 * Check if there is an entry available.
	 */
	if (uagt_freelist == (UAGT_REQ *)NULL)	{
	        splx(s);
		return((UAGT_REQ *)NULL);
	}

	uarp = uagt_freelist;	    /* get first entry on free list */

	uagt_freelist = uagt_freelist->next;  /* point to next entry in free list */

	uarp->uccb = uccb;
	uarp->ccb = ccb;
	uarp->flags = 0;
	uarp->next = uagt_pendlist;
	/*
	 * Get a buffer header from system and fill in appropriate
	 * fields for the DME.
	 */
	if ( ccb->cam_func_code == XPT_SCSI_IO )   {
		CCB_SCSIIO *io_ccb = (CCB_SCSIIO *)ccb;

		uarp->u_data = (char *)io_ccb->cam_data_ptr;
		bp = &uarp->bp;
		PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, 
			CAMD_FLOW, 
			("[%d/%d/%d] uagt_ins_queue: Got a buffer bp=0x%lx\n", 
			ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, bp));
		bp->b_bcount = uccb->uagt_buflen;
		bp->b_proc = u.u_procp;
		bp->b_un.b_addr = (caddr_t)uccb->uagt_buffer;
		bp->b_flags = B_PHYS | B_BUSY;
    		io_ccb->cam_req_map = (u_char *)bp;
	}

	uagt_pendlist = uarp;

	splx(s);
	
	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagt_ins_queue: exit\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));

	return(uarp);
}

/************************************************************************
 *
 *  ROUTINE NAME: uagt_rem_queue()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will deallocate an UA request entry from the UA
 *	pending list (uagt_pendlist) to the UA freelist (uagt_freelist).
 *
 *  FORMAL PARAMETERS:
 *	uccb  -  user agent CAM request.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	User agent pending list (uagt_pendlist) is modififed.
 *	User agent freelist (us_free_list) is modified.
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
void
uagt_rem_queue(ccb)
register CCB_HEADER *ccb;
{
	register int s;
	register UAGT_REQ *uarp, *temp;

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagt_rem_queue: enter\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));

	/*
	 * Locate and remove the request structure from the pending list
	 */
	s = splbio();

	uarp = temp = uagt_pendlist;
	while (uarp)	{
		if (uarp->ccb == ccb)   {  /* found it */
			/*
			 * Check if it is the first element.
			 */
			if( uarp == uagt_pendlist )
				uagt_pendlist = uarp->next;
			else
				temp->next = uarp->next;
			break;
		}  else  {
			temp = uarp;
			uarp = uarp->next;
		}
	}

	if(uarp != NULL)  {
		/*
		 * If the SIM Queue has been frozen keep track of it.
		 */
		if( uarp->flags == UAGT_SIMQ_FRZ )  {
			uarp->next = uagt_locklist;
			uagt_locklist = uarp;
		} else  {
			/*
			 * Null out the request structure and put it back
			 * on free list.
			 */
			bzero(uarp, sizeof(UAGT_REQ));
			uarp->next = uagt_freelist;
			uagt_freelist = uarp;
		}
	}
	else
		PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, 
		   CAMD_ERRORS,
		   ("[%d/%d/%d] uagt_rem_queue: NO entry in queue for ccb=0x%lx\n", 
		   ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, ccb));

	splx(s);

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagt_rem_queue: exit\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME: uagt_get_queue_entry()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function retrieves the UA request entry for ccb from the
 *	pending list (uagt_pendlist).
 *
 *  FORMAL PARAMETERS:
 *	ccb	- pointer to CCB to locate entry.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	UAGT_REQ-	pointer to UA request entry.
 *	NULL	-	no entry found for ccb.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/
UAGT_REQ *
uagt_get_queue_entry(ccb)
register CCB_HEADER *ccb;
{
	register int s;
	register UAGT_REQ *uarp;

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, CAMD_INOUT,
		("[%d/%d/%d] uagt_get_queue: enter\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));

        s = splbio();

	uarp = uagt_pendlist;

	while (uarp)	{
		if (uarp->ccb == ccb)	/* found it */
			break;
		else
			uarp = uarp->next;
	}

	splx(s);

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, CAMD_INOUT, 
		("[%d/%d/%d] uagt_get_queue: exit\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun));
	return(uarp);
}
/************************************************************************
 *
 *  ROUTINE NAME: uagt_send_ccb_wait()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function sends a CCB to the XPT and waits for it to complete.
 *
 *  FORMAL PARAMETERS:
 *	ccb	 - Pointer to CCB to wait on.
 *	priority - Software priority to sleep at.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	Return 0 for SUCCESS, or EINTR if sleep interrupted by signal.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/

int
uagt_send_ccb_wait(ccb, priority)
CCB_HEADER *ccb;
int priority;
{
	u_long status;
	int s;

	s = splbio();

	/*
	 * Check xpt return status to determine if we are done.
	 */
	if( (status = xpt_action(ccb)) != CAM_REQ_INPROG) {
		splx(s);
		return(0);
	}


	if (priority > PZERO)  {
		if (mpsleep(ccb, priority | PCATCH, "Zzzzzz", 0, (void *)0, 0)){
		    splx(s);
		    return(EINTR);
		}
	} else {
		(void) mpsleep(ccb, priority, "Zzzzzz", 0, (void *)0, 0);
	}

	splx(s);

	return (0);
}
