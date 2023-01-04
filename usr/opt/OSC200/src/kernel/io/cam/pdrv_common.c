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
static char *rcsid = "@(#)$RCSfile: pdrv_common.c,v $ $Revision: 1.1.16.9 $ (DEC) $Date: 1993/12/15 20:50:50 $";
#endif

/************************************************************************
 *
 *  pdrv_common.c		Version 1.00 
 *
 *  This file contains the CAM common routines which handle the 
 *  commands which are supported for all SCSI device types.
 *
 *  MODIFICATION HISTORY:
 *
 *  VERSION  DATE	WHO	REASON
 *
 *  1.00     02/22/91	maria	Created from CAM PDRV Common Func. Spec.
 *
 *  1.01     06/07/91	maria	Fixed PRINTD statements to print b/t/l
 *				information.
 *				Added to ccmn_open_unit, the setting of
 *				pd_dev field to the dev on first
 *				open of the device.
 *
 *  1.02     06/13/91   janet   Fixed ipl bug in ccmn_open_unit()
 *
 *  1.03     06/14/91   dallas	Fixed unknown entry bug in get_dev_desc_entry()
 *
 *  1.04     06/17/91   maria	Added a call to KM_ALLOC in ccmn_open_unit()
 *				to allocate the saved sense buffer.
 *
 *  1.05     06/19/91   maria	Removed ccmn_cdb_build().
 *				Added ccmn_minphys().
 *				Added call arguments to PRINTDs.
 *				Added INISR check when KM_ALLOCing
 *				in ccmn_get_bp() and ccmn_get_dbuf().
 *
 *  1.06     06/25/91   maria	Added ccmn_DoSpecialCmd().
 *
 *  1.07     07/03/91   jag	Replaced the INISR() macro with the cam_inisr()
 *				routine call.
 *
 *  1.08     07/3/91	robin	Added ccmn_SysSpecialCmd() to allow drivers
 *				to issue I/O control commands.  This also
 *				changed ccmn_DoSpecialCmd() to permit one
 *				additional argument to set special flags.
 *				Setup peripheral driver structure in CCB &
 *				in the special argument structure.
 *
 *  1.09     07/29/91   maria	Replaced SMP locking macros.
 *				SMP locked ccmn_ccbwait().
 *				Added check for end of device
 *				descriptor list in get_dev_desc().
 *				Removed call to ccmn_sdev_ccb_bld() in
 *				ccmn_open_unit() when the device types
 *				do not match.
 *
 *  1.10     07/31/91   dallas  Added the cmmn_errlog function
 *				for the peripheral drivers
 *
 *  1.11     08/06/91   maria	Removed peripheral driver structure
 *				pointer from ccmn_ccbwait() function.
 *
 *  1.12     08/13/91   maria	Fixed bug in get_dev_desc_entry() for
 *				unknown devices.
 *
 *  1.13     08/21/91	dallas	Bug fixes to ccmn_errlog. Found when
 *				I started looking into the log file 
 *				itself.
 *
 *  1.14     09/03/91   maria	Removed ccmn_minphys().
 *				General Cleanup.
 *
 *  1.15     09/05/91   maria	Lint fixes.
 *
 *  1.16     09/23/91   maria	Added declaration for unit table lock
 *				and fixed bug in get_dev_desc_entry in 
 *				copying idstring.
 *
 *  1.17     09/30/91   maria	Fixed locking sequence in ccmn_ccbwait().
 *
 *  1.18     10/23/91	maria	General clean up for SSB and set open
 *				count to zero in ccmn_close_unit().
 *
 *  1.19     10/29/91	maria	Ipl/locking bug fix on ccmn_abort_que()
 *				and ccmn_term_que().
 *  1.20     11/19/91   dallas  Changed ccmn_logger based on review
 *				commments. Also instead of defines 
 *				for the the error reports limits
 *				added externs found in cam_data.c
 *				In ccmn_open_unit fixed ccb release
 *				problem.
 *  1.21     12/11/91   maria	Added buf structure pool.
 *  1.22     12/19/91   maria	Added check in ccmn_open_unit for MAX_UNITS.
 *  1.23     01/24/92   maria	Removed the setting of the lun in the cdb
 *				in ccmn_start_unit(), ccmn_tur(), and 
 *				ccmn_mode_select().
 *
 ************************************************************************/

/************************* Include Files ********************************/

#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <kern/thread.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/parallel.h>
#include <vm/vm_kern.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <io/common/devio.h>
#include <sys/map.h>
#include <sys/file.h>
#include <sys/disklabel.h>
#include <kern/queue.h>
#include <ufs/fs.h>
#include <io/common/pt.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#include <io/cam/scsi_special.h>
#include <dec/binlog/errlog.h>	/* UERF errlog defines */
#include <io/cam/cam_logger.h>
#include <io/cam/pdrv.h>
#include <vm/vm_kern.h>
#include <io/cam/cam_errlog.h>
#include <io/cam/cam_debug.h>
#include <io/cam/cam_disk.h>
#include <io/cam/cam_tape.h>
#include <io/cam/ccfg.h>
#include <io/common/devdriver.h>
#include <io/common/devdriver_loadable.h>

/************************* Local Defines ********************************/

	
static DEV_DESC *get_dev_desc_entry();
CCB_HEADER 	*ccmn_get_ccb();
CCB_SCSIIO 	*ccmn_io_ccb_bld();
CCB_GETDEV 	*ccmn_gdev_ccb_bld();
CCB_SETDEV 	*ccmn_sdev_ccb_bld();
CCB_SETASYNC 	*ccmn_sasy_ccb_bld();
CCB_RELSIM 	*ccmn_rsq_ccb_bld();
CCB_PATHINQ 	*ccmn_pinq_ccb_bld();
CCB_ABORT 	*ccmn_abort_ccb_bld();
CCB_TERMIO 	*ccmn_term_ccb_bld();
CCB_RESETDEV 	*ccmn_bdr_ccb_bld();
CCB_RESETBUS 	*ccmn_br_ccb_bld();
CCB_SCSIIO 	*ccmn_tur();
CCB_SCSIIO 	*ccmn_start_unit();
CCB_SCSIIO 	*ccmn_mode_select();
struct buf	*ccmn_get_bp();
void		ccmn_rel_bp();
u_char		*ccmn_get_dbuf();
U32		ccmn_ccb_status();
U32		ccmn_send_ccb();
I32		ccmn_open_unit();
void		ccmn_init();
void		ccmn_close_unit();
void		ccmn_rem_cbb();
U32		ccmn_abort_que();
void		ccmn_term_que();
void		ccmn_rel_ccb();
void		ccmn_rel_dbuf();
void            ccmn_errlog();
int		ccmn_DoSpecialCmd();
int		ccmn_SysSpecialCmd();
struct buf * 	ccmn_alloc_bp_pool();
void 		ccmn_free_bp_pool();
struct device   *ccmn_find_device();
void		ccmn_disperse_que();

/********************** External Declarations ***************************/

extern CCB_HEADER   *xpt_ccb_alloc();
extern U32       xpt_ccb_free();
extern U32       xpt_action();
extern I32         cam_inisr();	/* for determining interrupt context */
extern void         cam_logger();
extern caddr_t      cdbg_ScsiStatus();
extern struct bus *ldbl_find_bus();
extern struct controller *ldbl_find_ctlr();

extern int scmn_SpecialCmd();
extern void scmn_SpecialCleanup();
extern struct special_args *scmn_GetArgsBuffer();

extern I32   num_dev_desc;		/* No. of device descriptor entries */
extern I32   num_unknown_dev_desc;	/* No. of unknown device */
					/* descriptor entries */
extern int nCAMBUS;
extern int hz;

extern DEV_DESC	  cam_devdesc_tab[];	/* Device descriptor table */
extern DEV_DESC	  dev_desc_unknown[];	/* Unknown device descriptor table */
extern PDRV_UNIT_ELEM pdrv_unit_table[];/* Statically allocated unit table */
extern char	  *cdbg_SenseKeyTable[];
extern U32	  cam_harderr_limit;	/* In cam_data.c number of harderrs */
					/* in total to report per device. */
extern U32	  cam_softerr_limit;	/* In cam_data.c number of harderrs */
					/* in total to report per device. */
extern U32 ccmn_bp_pool_size;	/* Buf structure pool size */
extern U32 ccmn_bp_high_water;	/* Buf structure pool high water mark */
extern U32 ccmn_bp_low_water;	/* Buf structure pool low water mark */
extern U32 ccmn_bp_increment;	/* Buf structure pool incerment */

/****************** Initialized and Unitialized Data ********************/

/*
 * Unit Table Lock Structure
 * There will be one global lock for the unit table which will
 * lock the entire table.  There is no need for locking the
 * individual entries.  This will be a sleep lock since it is
 * only accessed at open() or close() which are not at interrupt context
 */
lock_data_t lk_pdrv_unit_table;

/*
 * Buf Structure Pool Lock Structure
 * There will be one global lock for the buf structure pool.
 */
lock_data_t lk_ccmn_bp_pool;

/*
 * Data pool lock when there aren't any
 */
lock_data_t lk_ccmn_data_pool;

static u_char ccmn_first_call = 1;	/* Indicates whether ccmn_init() */
					/* has already been called */

/*
 * Header for buf structure pool.
 */
struct bp_head {
	int 	num_bp;			/* number of free buf structures */
	struct buf *bp_list;		/* Pointer to first buf */
					/* structure on free list */
	U32	bp_wait_cnt;		/* Count of resource wait */ 
}  ccmn_bp_head;

struct ccmn_data_alloc {
	U32	alloc_wait_cnt;		/* Number of who is waiting */
} ccmn_data_alloc;


/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_init()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle the common initialization for the
 *	CAM peripheral drivers.
 *           If this is the first time called:
 *		- Initialize the unit table lock structure.
 *          	- Call xpt_init().
 *          	- Indicate initialization has been completed.
 *		- Allocate the buf structure pool.
 *
 *  FORMAL PARAMETERS:
 *		None.
 *
 *  IMPLICIT INPUTS:
 *	ccmn_first_call is checked.
 *
 *  IMPLICIT OUTPUTS:
 *	ccmn_first_call is set on first call.
 *
 *  RETURN VALUE:
 *		None.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
ccmn_init()
{
	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_COMMON,
		("[b/t/l] ccmn_init: entry\n"));

	if (ccmn_first_call)  {
		int i;
		struct buf *bp;

		PDU_INIT_LOCK();
		/*
		 * Call the XPT initialization routine.
		 */
		xpt_init();

		/*
		 * Allocate the bp pool and initialize its lock.
		 */
		CCMN_BP_INIT_LOCK();
		ccmn_alloc_bp_pool(ccmn_bp_pool_size);

		/* 
		 * Init the data allocation lock
		 */
		CCMN_DATA_INIT_LOCK();
		ccmn_first_call = 0;
	}
	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_COMMON,
		("[b/t/l] ccmn_init: exit\n"));
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_open_unit()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle the common open  for all the CAM
 *	peripheral drivers.  It must be called for every open before
 *	any device specific code is executed.
 *		-  Check whether the device type was configured.
 *		-  If this is the first open or the peripheral device
 *		   structure was not allocated:
 *			- Allocate the peripheral device structure
 *			- Initialize the peripheral device lock
 *			- Issue the Get Device Type ccb to obtain
 *			  inquiry data.  If the device types do not
 *			  match then return ENXIO.
 *			- Allocate the device specific structure.
 *			- Obtain a pointer to the device descriptor entry.
 *			- Allocate the saved sense data area.
 *		- If the device types do not match, return ENXIO.
 *		- Increment the open count.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	scsi_dev_type	- SCSI device type (DTYPE_DIRECT ...).
 *	flag		- Indicates whether open for exclusive use.
 *	dev_size;	- Size of device specific structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *        0       -  Success.
 *        EBUSY   -  Device already opened - excluse use.
 *        ENXIO   -  Device does not exist, even after rescan.
 *        EFAULT  -  Device requested will go beyond the size of 
 *		     the pdrv_unit_table.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

I32
ccmn_open_unit(dev, scsi_dev_type, flag, dev_size)
dev_t dev;
U32 scsi_dev_type;
U32 flag;
U32 dev_size;
{
	PDRV_UNIT_ELEM 	*pdu;
	PDRV_DEVICE 	*pd;
	CCB_GETDEV 	*ccb;
	int 		spl;
	char 		*ptr;
	char		*tmp_inq;
	int 		t_spl;
	u_long		status;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev), 
	   (CAMD_COMMON | CAMD_INOUT), 
	   ("[%d/%d/%d] cdisk_open_unit: entry dev=0x%x dev_type=0x%x flag=0x%x dev_size=%d\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, scsi_dev_type, flag, dev_size));
	/*
	 * Check that we don't go beyond the unit table.
	 */
	if( DEV_BUS_ID(dev) >= nCAMBUS)
		return(EFAULT);
	/* 
	 * SMP lock the unit table
	 */
	PDU_IPLSMP_LOCK(LK_ONCE , spl);

	pdu = GET_PDRV_UNIT_ELEM(dev);

	/* 
	 * Check to see what type of open is wanted
	 */
	if ((pdu->pu_exclusive &  CCMN_EXCLUSIVE)){
		/* 
		 * Someone already has exclusive on this device fail it..
		 */
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
		   (CAMD_COMMON | CAMD_ERRORS),
		   ("[%d/%d/%d] ccmn_open_unit: Device already opened exclusively\n",
		   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		PDU_IPLSMP_UNLOCK(spl); /* unlock unit table */
		return(EBUSY);
	}
	if ((pdu->pu_opens != 0) && (flag & CCMN_EXCLUSIVE))  {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
		   (CAMD_COMMON | CAMD_ERRORS),
		   ("[%d/%d/%d] ccmn_open_unit: Device already opened\n",
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		PDU_IPLSMP_UNLOCK(spl);	/* unlock unit table */
		return(EBUSY);
	}

	if ((pdu->pu_opens == 0) && (pdu->pu_device == NULL)) {
		/*
		 * Allocate the peripheral device structure.
		 */
		ptr = (char *)ccmn_get_dbuf(sizeof(PDRV_DEVICE));
		if (ptr == NULL)  {
			panic("Not enough memory for Peripheral Device");
		}
		pdu->pu_device = (PDRV_DEVICE *)ptr;
		pd = pdu->pu_device;

		/*
		 * Must check to see if at boot time.. Can't scan
		 * with IPL lock 
		 */
		if( cam_at_boottime()){
		    PDU_IPLSMP_UNLOCK(spl); /* unlock unit table */
		    status = ccfg_edtscan(EDT_SINGLESCAN, DEV_BUS_ID(dev),
			     DEV_TARGET(dev), DEV_LUN(dev)); 
		    PDU_IPLSMP_LOCK(LK_ONCE, spl);
		}
		else {
		    status = ccfg_edtscan(EDT_SINGLESCAN, DEV_BUS_ID(dev),
				DEV_TARGET(dev), DEV_LUN(dev));
		}
		if( status != CAM_REQ_CMP) {
			     
		    ccmn_rel_dbuf((char *)pdu->pu_device, sizeof(PDRV_DEVICE));
		    /* 
		     * Null out unit table
		     */
		    pdu->pu_device = NULL;
		    /*
		     * Unlock unit table and return enxio
		     */
		    PDU_IPLSMP_UNLOCK(spl); /* unlock unit table */
		    return(ENXIO);
		}
		/* 
		 * SMP lock the peripheral device structure.
	 	 */
		PDRV_INIT_LOCK(pd);
		PDRV_IPLSMP_LOCK(pd, LK_ONCE , t_spl);

		/* 
		 * Issue the GET DEVICE TYPE CCB
		 * Device must report connected to this target/lun or can be
		 * connected at a later time.
		 */
		ccb = ccmn_gdev_ccb_bld(dev, (U32)0, pd->pd_dev_inq);
		if ((ccb->cam_ch.cam_status & CAM_STATUS_MASK) != CAM_REQ_CMP){
			PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   	   	   DEV_LUN(dev), (CAMD_COMMON | CAMD_ERRORS), 
			   ("[%d/%d/%d] ccmn_open_unit: Get Dev failed\n",
	   	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
			/* Maria - could issue rescan */
			PDRV_IPLSMP_UNLOCK(pd, t_spl);
			ccmn_rel_dbuf((char *)pdu->pu_device,
					sizeof(PDRV_DEVICE));
			pdu->pu_device = NULL;
			PDU_IPLSMP_UNLOCK(spl);	/* unlock unit table */
			/*
			 * return the CCB to the pool.
			 */
			ccmn_rel_ccb((CCB_HEADER *)ccb);
			return(ENXIO);
		}
		/*
		 * Device present or may be present later on
		 */
		if ((scsi_dev_type != ccb->cam_pd_type) && ((scsi_dev_type |
				 (ALL_PQUAL_NOT_CONN << 5)) != ccb->cam_pd_type)) {
			PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   	   	   DEV_LUN(dev), (CAMD_COMMON | CAMD_ERRORS), 
			   ("[%d/%d/%d] ccmn_open_unit: Get Dev failed/Types don't match\n",
	   	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
			/* Maria - could issue rescan */
			PDRV_IPLSMP_UNLOCK(pd, t_spl);
			ccmn_rel_dbuf((char *)pdu->pu_device,
					sizeof(PDRV_DEVICE));
			pdu->pu_device = NULL;
			PDU_IPLSMP_UNLOCK(spl);	/* unlock unit table */
			/*
			 * return the CCB to the pool.
			 */
			ccmn_rel_ccb((CCB_HEADER *)ccb);
			return(ENXIO);
		}
		/* 
		 * Initialize the forward and backward pointers for the
		 * active and pending lists.
		 */
		pd->pd_active_list.flink = pd->pd_active_list.blink = 
			(PDRV_WS *)&pd->pd_active_list;
		pd->pd_pend_list.flink = pd->pd_pend_list.blink = 
			(PDRV_WS *)&pd->pd_pend_list;

		pdu->pu_type = ccb->cam_pd_type;
		/*
		 * return the CCB to the pool.
		 */
		ccmn_rel_ccb((CCB_HEADER *)ccb);
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   	   DEV_LUN(dev), (CAMD_COMMON | CAMD_FLOW), 
		   ("[%d/%d/%d] ccmn_open_unit: pu_type=0x%x\n",
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev),
		   DEV_LUN(dev), pdu->pu_type));
		/*
		 * Allocate the device specific structure.
		 */
		if (dev_size)   {
		    pd->pd_specific = (char *)ccmn_get_dbuf(dev_size);
		    if (pd->pd_specific == NULL)   {
			panic("Cannot allocate periph specific\n");
		    }
		}
		pd->pd_spec_size = dev_size;
		pd->pd_dev = dev;
		pd->pd_bus = DEV_BUS_ID(dev);
		pd->pd_target = DEV_TARGET(dev);
		pd->pd_lun = DEV_LUN(dev);
		/*
		 * Find the device descriptor entry.
		 */
		if ((pd->pd_dev_desc = get_dev_desc_entry(pd, scsi_dev_type)) 
			== (DEV_DESC *)NULL)
			panic("Cannot find device descriptor entry");
		/*
		 * Allocate the peripheral saved sense data buffer.
		 * Must sure the Flag indicates the user wants this
		 * and the size is non zero
		 */
		if( (pd->pd_dev_desc->dd_valid & DD_REQSNS_VAL ) && 
			( pd->pd_dev_desc->dd_req_sense_len != 0)){

		    ptr = (char *)ccmn_get_dbuf(pd->pd_dev_desc->
				dd_req_sense_len);
		    if (ptr == NULL)   {
			panic("Cannot allocate periph sense buffer\n");
		    }
		    pd->pd_sense_ptr = ptr;
		    pd->pd_sense_len = pd->pd_dev_desc->dd_req_sense_len;
		}
		else {
		    ptr = (char *)ccmn_get_dbuf(DEC_AUTO_SENSE_SIZE);
		    if (ptr == NULL) {
			panic("Cannot allocate periph sense buffer\n");
		    }
		    pd->pd_sense_ptr = ptr;
		    pd->pd_sense_len = DEC_AUTO_SENSE_SIZE;
		}


		/*
		 * Unlock peripheral device structure.
		 */
		PDRV_IPLSMP_UNLOCK(pd, t_spl);
	}
	/* 
	 * There is a peripherial device structure... get to see
	 * if the device has changed underneath us.
	 * Issue a rescan for this one and compare it against what
	 * was there..
	 */
	else if ((pdu->pu_opens == 0) && (pdu->pu_device != NULL)) {

		/*
		 * Allocate a temp area to hold inquiry data
		 */
		tmp_inq = (char *)ccmn_get_dbuf(INQLEN);
		if (tmp_inq == NULL)  {
			PDU_IPLSMP_UNLOCK(spl); /* unlock unit table */
			return(ENOMEM);
		}
		pd = pdu->pu_device;

		/*
		 * Must check to see if at boot time.. Can't scan
		 * with IPL lock 
		 */
		if( cam_at_boottime()){
		    PDU_IPLSMP_UNLOCK(spl); /* unlock unit table */
		    status = ccfg_edtscan(EDT_SINGLESCAN, DEV_BUS_ID(dev),
			     DEV_TARGET(dev), DEV_LUN(dev)); 
		    PDU_IPLSMP_LOCK(LK_ONCE, spl);
		}
		else {
		    status = ccfg_edtscan(EDT_SINGLESCAN, DEV_BUS_ID(dev),
				DEV_TARGET(dev), DEV_LUN(dev));
		}
		if( status != CAM_REQ_CMP) {
			     
		    ccmn_rel_dbuf( tmp_inq, INQLEN);
		    /*
		     * Unlock unit table and return enxio
		     */
		    PDU_IPLSMP_UNLOCK(spl); /* unlock unit table */
		    return(ENXIO);
		}

		/* 
		 * SMP lock the peripheral device structure.
	 	 */
		PDRV_IPLSMP_LOCK(pd, LK_ONCE , t_spl);

		/* 
		 * Issue the GET DEVICE TYPE CCB
		 * Device must report connected to this target/lun or can be
		 * connected at a later time.
		 */
		ccb = ccmn_gdev_ccb_bld(dev, (U32)0, tmp_inq);
		if ((ccb->cam_ch.cam_status & CAM_STATUS_MASK) != CAM_REQ_CMP){
			PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   	   	   DEV_LUN(dev), (CAMD_COMMON | CAMD_ERRORS), 
			   ("[%d/%d/%d] ccmn_open_unit: Get Dev failed\n",
	   	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
			

		        ccmn_rel_dbuf( tmp_inq, INQLEN);

			/*
			 * return the CCB to the pool.
			 */
			ccmn_rel_ccb((CCB_HEADER *)ccb);

			PDRV_IPLSMP_UNLOCK(pd, t_spl);
		        PDU_IPLSMP_UNLOCK(spl); /* unlock unit table */

			return(ENXIO);
		}
		/*
		 * See if this device is what we want.
		 */
		if ((scsi_dev_type != ccb->cam_pd_type) && ((scsi_dev_type |
				 (ALL_PQUAL_NOT_CONN << 5)) != ccb->cam_pd_type)) {
			PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   	   	   DEV_LUN(dev), (CAMD_COMMON | CAMD_ERRORS), 
			   ("[%d/%d/%d] ccmn_open_unit: Get Dev failed/Types don't match\n",
	   	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

		        ccmn_rel_dbuf( tmp_inq, INQLEN);
			/* 
			 * Unlock both the dev struct and the unit table.
			 */
			PDRV_IPLSMP_UNLOCK(pd, t_spl);
			PDU_IPLSMP_UNLOCK(spl);	/* unlock unit table */
			/*
			 * return the CCB to the pool.
			 */
			ccmn_rel_ccb((CCB_HEADER *)ccb);
			return(ENXIO);
		}
		/*
		 * Now check to see if the dev that was seen is the 
		 * same type as before.
		 */
		if((pdu->pu_type & DEV_TYPE_MASK) != (ccb->cam_pd_type &
					DEV_TYPE_MASK)) {
		    /* 
		     * Remove the following lines of code which 
		     * will return a open failure and remove ifdefs
		     * to allow tear down.. Also note a check
		     * should be done on VID and PID.
		     */
		    ccmn_rel_dbuf( tmp_inq, INQLEN);
		    /* 
		     * Unlock both the dev struct and the unit table.
		     */
		    PDRV_IPLSMP_UNLOCK(pd, t_spl);
		    PDU_IPLSMP_UNLOCK(spl);	/* unlock unit table */
		    /*
		     * return the CCB to the pool.
		     */
		    ccmn_rel_ccb((CCB_HEADER *)ccb);
		    return(ENXIO);
/* HOTSWAP Future feature */
#ifdef HOTSWAP
		    /* 
		     * the devices don't match up.. Tear down the
		     * specific struct and redo for this one.
		     */
		    if( pd->pd_spec_size  && pd->pd_specific ){
			ccmn_rel_dbuf((char *)pd->pd_specific, pd->pd_spec_size);
		    }
		    /* 
		     * Release the previous sense buf 
		     */
		    if( pd->pd_sense_ptr && pd->pd_sense_len){
			    ccmn_rel_dbuf((char *)pd->pd_sense_ptr ,
				pd->pd_sense_len);
		    }
		    /* 
		     * We are committed now zero out a good portion 
		     * of the pdrv struct... Don't touch the lock...
		     */
		    bzero( pd, (sizeof(PDRV_DEVICE) - sizeof(lock_data_t)));

		    if (dev_size)   {
		        pd->pd_specific = (char *)ccmn_get_dbuf(dev_size);
		        if (pd->pd_specific == NULL)   {
		            ccmn_rel_dbuf( tmp_inq, INQLEN);
		            /* 
		             * Release sense buf 
		             */
		            if( pd->pd_sense_ptr && pd->pd_sense_len){
			        ccmn_rel_dbuf((char *)pd->pd_sense_ptr ,
					pd->pd_sense_len);
			    }
			    /* 
			     * Unlock both the dev struct and the unit table.
			     */
			    PDRV_IPLSMP_UNLOCK(pd, t_spl);
			    ccmn_rel_dbuf((char *)pdu->pu_device,
					sizeof(PDRV_DEVICE));
			    pdu->pu_device = NULL;
			    PDU_IPLSMP_UNLOCK(spl);	/* unlock unit table */
			    /*
			     * return the CCB to the pool.
			     */
			    ccmn_rel_ccb((CCB_HEADER *)ccb);
			    return(ENOMEM);
		        }
		    }
		    /* 
		     * Initialize the forward and backward pointers for the
		     * active and pending lists.
		     */
		    pd->pd_active_list.flink = pd->pd_active_list.blink = 
				(PDRV_WS *)&pd->pd_active_list.flink;
		    pd->pd_pend_list.flink = pd->pd_pend_list.blink = 
				(PDRV_WS *)&pd->pd_pend_list.flink;

		    pdu->pu_type = ccb->cam_pd_type;
		    /*
		     * return the CCB to the pool.
		     */
		    ccmn_rel_ccb((CCB_HEADER *)ccb);
		    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   	       DEV_LUN(dev), (CAMD_COMMON | CAMD_FLOW), 
		       ("[%d/%d/%d] ccmn_open_unit: pu_type=0x%x\n",
	   	       DEV_BUS_ID(dev), DEV_TARGET(dev),
		       DEV_LUN(dev), pdu->pu_type));


		    /* 
		     * Since the old and new does not match up 
		     * Redo the pdrv struct 
		     */
		    
		    pd->pd_spec_size = dev_size;
		    pd->pd_dev = dev;
		    pd->pd_bus = DEV_BUS_ID(dev);
		    pd->pd_target = DEV_TARGET(dev);
		    pd->pd_lun = DEV_LUN(dev);
		    /* 
		     * Copy in the new inquiry data
		     */
		    bcopy(tmp_inq, pd->pd_dev_inq, INQLEN);

		    /*
		     * Find the device descriptor entry.
		     */
		    if ((pd->pd_dev_desc = get_dev_desc_entry(pd, scsi_dev_type)) 
			== (DEV_DESC *)NULL)
			panic("Cannot find device descriptor entry");

		    /*
		     * Allocate the peripheral saved sense data buffer.
		     * Must sure the Flag indicates the user wants this
		     * and the size is non zero
		     */
		    if( (pd->pd_dev_desc->dd_valid & DD_REQSNS_VAL ) && 
			    ( pd->pd_dev_desc->dd_req_sense_len != 0)){

		        ptr = (char *)ccmn_get_dbuf(pd->pd_dev_desc->
				dd_req_sense_len);
		        if (ptr == NULL)   {
			    panic("Cannot allocate periph sense buffer\n");
		        }
		        pd->pd_sense_ptr = ptr;
		        pd->pd_sense_len = pd->pd_dev_desc->dd_req_sense_len;
		    }
		    else {
		        ptr = (char *)ccmn_get_dbuf(DEC_AUTO_SENSE_SIZE);
		        if (ptr == NULL) {
			    panic("Cannot allocate periph sense buffer\n");
		        }
		        pd->pd_sense_ptr = ptr;
		        pd->pd_sense_len = DEC_AUTO_SENSE_SIZE;
		    }

#endif HOTSWAP

		}
		/*
		 * The following line should be removed once hotswap
		 * is implemented. The dev types match but the upper
		 * 3 bits could say not connected or connected now (RAID).
		 * so we just copy the bits over for now.
		 */
		pdu->pu_type = ccb->cam_pd_type;
		ccmn_rel_dbuf( tmp_inq, INQLEN);
		ccmn_rel_ccb((CCB_HEADER *)ccb);
		PDRV_IPLSMP_UNLOCK(pd, t_spl);
	}

	/*
	 * Check if the device type does not match 
	 */
	else if ((scsi_dev_type != pdu->pu_type) && ((scsi_dev_type | 
			(ALL_PQUAL_NOT_CONN << 5)) !=  pdu->pu_type)) {
		PDU_IPLSMP_UNLOCK(spl);	/* unlock unit table */
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   	   DEV_LUN(dev), (CAMD_COMMON | CAMD_ERRORS), 
		   ("[%d/%d/%d] ccmn_open_unit: Types do not match pu_type=0x%x\n", 
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev),
		   DEV_LUN(dev), pdu->pu_type));
		return(ENXIO);
	}
	/* 
	 * Get the pdrv_device ptr
	 */
	pd = pdu->pu_device;

	if (pdu->pu_opens == 0){ 
		pdu->pu_device->pd_dev = dev;
		/* Need pu_opens++ before calling ccmn_disperse_que */
	        pdu->pu_opens++;		/* increment open count */
		/*
		 * If the device descriptor indicates that this device 
		 * allows the que depth to be dispersed across all LUNS
		 * then do so.  Otherwise,
		 * Act as if only half the real queue depth exists so
		 * that other drivers are not starved in a
		 * multi-initiator environment.
		 */
		if (pd->pd_dev_desc->dd_que_depth) {
			if (pd->pd_dev_desc->dd_flags & SZ_DISPERSE_QUE) {
				ccmn_disperse_que(dev, 
					(pd->pd_dev_desc->dd_que_depth/2));
			}
			else {
				pd->pd_que_depth = 
					pd->pd_dev_desc->dd_que_depth / 2;
			}
		}

	}
	else {
	    pdu->pu_opens++;		/* increment open count */
	}

	/* 
	 * assign flags
	 */
	pdu->pu_exclusive = flag;

	PDU_IPLSMP_UNLOCK(spl);	/* unlock unit table */

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_open_unit: exit - success\n", 
   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

	return(0);		/* Success */
}

/************************************************************************
 *
 *  ROUTINE NAME:  get_dev_desc_entry()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will search the device descriptor table to
 *	find the entry for the device by matching on the product and
 *	vendor ID in the Inquiry data and the device type.
 *		- If the vendor id and product id field are NULL, check
 *		  the qualifier field to determine if the device is a
 *		  TZ30 or TZK50.  
 *		- Search the cam_devdesc_tab for a match on the device
 *		  type and product id and vendor id string.
 *		- If no entry is found then the device entry in the
 *		  unknown device descriptor table is used.
 *
 *  FORMAL PARAMETERS:
 *	pd		- Pointer to the peripheral device structure.
 *	scsi_dev_type 	- SCSI device type.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	DEV_DESC *ddp	- Pointer to the device descriptor entry.
 *	NULL		- No entry exists even in the unknown table (fatal).
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/


static DEV_DESC *
get_dev_desc_entry(pd, scsi_dev_type)
PDRV_DEVICE *pd;
U32 scsi_dev_type;
{
	DEV_DESC *ddp;
	char idstring[IDSTRING_SIZE+1];
	ALL_INQ_DATA *inqp = (ALL_INQ_DATA *)pd->pd_dev_inq;
	int i=0;
	I32 found_it = 0;	/* For unknown entry at end of table */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] get_dev_desc: entry pd=0x%x dev_type=0x%x\n", 
   	   pd->pd_bus, pd->pd_target, pd->pd_lun, pd, scsi_dev_type));

	/*
	 * HACK for tk50's and tk30's since they don't return
	 * anything in the vid and pid fields.  But they do
	 * return in the qualifier field a 0x50 and 0x30 respectively.
	 */
	if((inqp->dtype == ALL_DTYPE_SEQUENTIAL) && (inqp->vid[0] == NULL ) && 
			(inqp->pid[0] == NULL)) {
	    PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_COMMON,
		("[%d/%d/%d] get_dev_desc: Zero pid and vid dmodify=0x%x\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun, inqp->dmodify));
	    if(inqp->dmodify == 0x50){ /* tzk50 */
		bcopy("DEC     " , inqp->vid, VID_STRING_SIZE);
		bcopy("TZK50           ", inqp->pid, PID_STRING_SIZE);
	    }
	    else if(inqp->dmodify == 0x30){ /* tzk30 */
		bcopy("DEC     " , inqp->vid, VID_STRING_SIZE);
		bcopy("TZ30           ", inqp->pid, PID_STRING_SIZE);
	    }
	    else {
		/*
	 	 * The entry in the UNKNOWN table is returned for this device.
		 */
	    	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
		   (CAMD_COMMON | CAMD_ERRORS),
		   ("[%d/%d/%d] get_dev_desc_entry: Zero'ed inquiry vid and pid fields and not a TZK50 or TZK30\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	    }
	}

	/*
	 * Get a local copy of the product and vendor ID string used to
	 * match with entry in device descriptor table.
	 */
	bcopy((caddr_t)inqp->vid, idstring, VID_STRING_SIZE);
	bcopy((caddr_t)inqp->pid, &idstring[VID_STRING_SIZE], PID_STRING_SIZE);
	idstring[PID_STRING_SIZE + VID_STRING_SIZE] = '\0';

	for(ddp = cam_devdesc_tab; i < num_dev_desc; i++,ddp++)   {
		/*
		 * Check for end of list.
		 */
		if( (ddp->dd_pv_name[0] == NULL) 
		   && (ddp->dd_dev_name[0] == NULL)
		   && (ddp->dd_device_type == 0) )
			break;
		if (((ddp->dd_device_type >> DTYPE_SHFT) & 0xF) 
		   == scsi_dev_type)  {
			if (strncmp(idstring, (caddr_t)ddp->dd_pv_name,
			   ddp->dd_length) == 0){
				found_it++;
				break;		/* found it */
			    }
		}
	}

	/*
	 * Check whether we found the entry. If not  use the UNKNOWN entry
	 * for the device type.
	 */
	if ((found_it == 0) || (ddp == (DEV_DESC *)NULL))   {
		/*
		 * Make sure we don't go beyond the unknown table.
		 */
		if(scsi_dev_type <= num_unknown_dev_desc)
			ddp = &dev_desc_unknown[scsi_dev_type];
		else
			ddp = (DEV_DESC *)NULL;
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] get_dev_desc: exit - success\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ddp);
}


/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_close_unit()
 *
 *  FUNCTIONAL DESCRIPTION:
 *           This function will handle the common close for the CAM
 *           peripheral drivers.  It will decrement the open count.
 *
 *  FORMAL PARAMETERS:
 *	dev	- Major/minor device number.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
ccmn_close_unit(dev)
dev_t dev;
{
	PDRV_UNIT_ELEM 	*pdu;
	PDRV_DEVICE	*pd;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_close_unit: entry dev=0x%x\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev), dev));

	pdu = GET_PDRV_UNIT_ELEM(dev);
	/*
	 * SMP lock the unit table.
	 */
	PDU_IPLSMP_LOCK(LK_ONCE, spl);   /* lock unit table */

	if((pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL){
	    /* 
	     * This really shouldn't happen....
	     */
	    return;
	}

	/*
	 * Set the open count to 0 - only called on last close.
	 */
	if (pdu->pu_opens) {
		pdu->pu_opens = 0;
		pdu->pu_exclusive = 0;
		/*
		 * If the device descriptor indicates that this device 
		 * allows the que depth to be dispersed across all LUNS
		 * then take the que depth from the unit being closed
		 * and disperse to remaining open LUNS.
		 * Also zero out the que depth for this unit.
		 */
		if (pd->pd_dev_desc->dd_flags & SZ_DISPERSE_QUE) {
			ccmn_disperse_que(dev, 
				(pd->pd_dev_desc->dd_que_depth/2));
			pd->pd_que_depth = 0;
		}
	}
	else  {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
		   (CAMD_COMMON | CAMD_ERRORS),
	      	   ("[%d/%d/%d] ccmn_close_unit: open count is 0\n",
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	}
	/*
	 * SMP unlock the unit table.
	 */
	PDU_IPLSMP_UNLOCK(spl);   /* lock unit table */

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_close_unit: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_send_ccb()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle placing the CCB in the appropriate
 *	queue and calling the XPT layer  to start the request if
 *	appropriate.  For tagged requests a high water mark of half
 *	the queue depth for the device will be used so that other
 *	drivers will not be starved.
 *		- If the ccb is a SCSI I/O ccb and not a retry
 *			- If queuing is enabled and the queue depth has
 *			  been reached, then the ccb is placed on the pending 
 *			  list, the pending count is incremented and
 *			  CAM_REQ_INPROG is returned.
 *			- Place the request on the active list and
 *			  increment the active count.
 *		- Call xpt_action().
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure.
 *	ccb	- Pointer to CCB to send to XPT.
 *	retry	- Indicates whether this CCB is a retry (retry=1).
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Return value from xpt_action().
 *	CAM_REQ_INPROG - for tagged requests placed on pending queue.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This routine must be called SMP safe!
 *
 ************************************************************************/

U32
ccmn_send_ccb(pd, ccb, retry)
PDRV_DEVICE *pd;
CCB_HEADER *ccb;
u_char retry;	/* indicates whether this is a retry of a CCB that's */
		/* already on the queue */
{
	CCB_HEADER *pend_ccb;
	PDRV_UNIT_ELEM	*pdu;
	U32 ret;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_send_ccb: entry pd=0x%x ccb=0x%x retry=%d\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun, pd, ccb, retry));


	/*
	 * Check whether this is a SCSI I/O CCB.
	 */
	if ((ccb->cam_func_code == XPT_SCSI_IO) && !(retry))	{
		CCB_SCSIIO *ioccb = (CCB_SCSIIO *)ccb;
		/*
	 	 * Check whether request is tagged and the queue depth
		 * count has been reached.
	 	 */
		if ((ccb->cam_flags & CAM_QUEUE_ENABLE) && !( ccb->cam_flags &
				CAM_SIM_QHEAD)){
		    if (pd->pd_que_depth <= pd->pd_active_ccb)  {
			/*
			 * Place the CCB on the pending list and
			 * set the cam_status .
			 */
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
		           (CAMD_COMMON | CAMD_FLOW),
			   ("[%d/%d/%d] ccmn_send_ccb: CCB 0x%x placed on pending list\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun, ccb));
			ccb->cam_status = CAM_REQ_INPROG;
			/*
			 * Goes to tail of que....
			 */
			enqueue_tail( &pd->pd_pend_list, ioccb->cam_pdrv_ptr);
			++pd->pd_pend_ccb;
			return(CAM_REQ_INPROG);
		   }
		}
		/*
		 * Place the SCSI I/O CCB on the active list and
		 * increment the active count.
		 */
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
		   (CAMD_COMMON | CAMD_FLOW),
		   ("[%d/%d/%d] ccmn_send_ccb: CCB 0x%x placed on active list\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, ccb));
		enqueue_tail(&pd->pd_active_list, ioccb->cam_pdrv_ptr);
		++pd->pd_active_ccb;
	}

	/*
	 * Send the request to the XPT layer. 
	 */
	ret = xpt_action(ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_send_ccb: exit ret=0x%x\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun, ret));

	return(ret);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_rem_ccb()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle removing a SCSI I/O CCB from the
 *	active list and starting a new tagged request if a tagged
 *	CCB is pending.
 *		- Remove the CCB from the active list.
 *		- Decrement the active count.
 *		- If queuing is enabled and the there is a ccb pending:
 *			- Remove the ccb from the pending list and
 *			  decrement the pending count.
 *			- Place the ccb on the active list and increment
 *			  the active count.
 *			- Call xpt_action().
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure.
 *	ccb	- Pointer to CCB to send to XPT.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
ccmn_rem_ccb(pd, ccb)
PDRV_DEVICE *pd;
CCB_SCSIIO *ccb;	/* Only SCSI I/O CCBs are on queue */
{
	CCB_SCSIIO 	*next_ccb;
	PDRV_WS		*pws;		/* Working set pointer */
	int		act_q = 0;
	int 		spl;
	U32 		status;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_rem_ccb: entry pd=0x%x ccb=0x%x\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun, pd, ccb));

	/*
	 * SMP lock the peripheral device structure.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);

	/* 
	 * Find out if on pending queues or active queue
	 */
	pws = pd->pd_active_list.flink;
	while(  pws != (PDRV_WS *)&pd->pd_active_list.flink){
	    if( pws->pws_ccb == ccb){
	        act_q = 1;
	        break;
	    }
	    else {
		pws = pws->pws_flink;
	    }
	}
	if( act_q == 0 ){ /* Not found on active queues */
	    pws = pd->pd_pend_list.flink;
	    while( pws != (PDRV_WS *)&pd->pd_pend_list.flink ){
	        if( pws->pws_ccb == ccb){
	            act_q = 2;
	            break;
	        }
	        else {
	    	    pws = pws->pws_flink;
	        }
	    }
	}
	if(act_q == 0 ){ /* Not found on any list */
	    PDRV_IPLSMP_UNLOCK(pd, spl);
	    return;
	}


	/*
	 * Remove the request from the active queue and decrement 
	 * the active count.
	 */
	remque(pws);

	if( act_q == 1){
	    pd->pd_active_ccb--;
	}
	else {
	    pd->pd_pend_ccb--;
	}
	/*
	 * If this was a tagged request, check if another tagged request is
 	 * waiting on the pending queue to be sent to the XPT and send it.
	 */
	if ((ccb->cam_ch.cam_flags & CAM_QUEUE_ENABLE) && pd->pd_pend_ccb && 
		    (pd->pd_que_depth > pd->pd_active_ccb))  {
	    while( (pd->pd_que_depth > pd->pd_active_ccb) && 
			(pd->pd_pend_ccb != 0)){

		/* 
		 * Remove request from pending queue.
		 */
		pws = (PDRV_WS *)dequeue_head(&pd->pd_pend_list);
		if(pws == (PDRV_WS *)NULL){
		    break; /* Nothing is there */
		}
		--pd->pd_pend_ccb;
 		/* 
		 * Place on active queue.
		 */
		enqueue_tail( &pd->pd_active_list, pws);
		++pd->pd_active_ccb;
		/*
		 * Send the request to the XPT.  If it did not get 
		 * queued to the SIM (IN_PROG) then call the completion
		 * routine if there is one.
		 */
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_COMMON,
	   	   ("[%d/%d/%d] ccmn_rem_ccb: pending ccb 0x%x sent to XPT\n", 
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, ccb));
		next_ccb = pws->pws_ccb;
		status = xpt_action((CCB_HEADER*)next_ccb);
		if (status != CAM_REQ_INPROG) 	{
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
		   	   (CAMD_COMMON | CAMD_FLOW),
			   ("ccmn_rem_ccb: pending CCB 0x%x status 0x%x\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun,
			   next_ccb, next_ccb->cam_ch.cam_status));
			if (!(next_ccb->cam_ch.cam_flags & CAM_DIS_CALLBACK)){
				/* 
				 * Always unlock it before calling the 
				 * completion function.
				 */
				PDRV_IPLSMP_UNLOCK(pd, spl);
				next_ccb->cam_cbfcnp(next_ccb);
				PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			}
		}
	    }
	}
	/*
	 * SMP unlock the peripheral device structure.
	 */
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_rem_ccb: exit - success\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_abort_que()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle sending an ABORT CCB for each SCSI
 *	I/O CCB on the active list.
 *		- If no request are active then return.
 *		- For each request not marked as retry send the abort ccb.
 *		- Release the Abort ccb.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE: Count of number of ccbs aborted
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This routine must be called SMP safe!
 *
 ************************************************************************/

U32
ccmn_abort_que(pd)
PDRV_DEVICE *pd;
{
	CCB_ABORT 	*abort_ccb = (CCB_ABORT *)NULL;
	PDRV_WS 	*pws;
	U32		count = 0;	

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_abort_que: entry pd=0x%x\n", 
	   pd->pd_bus, pd->pd_target, pd->pd_lun, pd));

	/*
	 * Check if there is anything to abort.
	 */
	if ((pws = pd->pd_active_list.flink) == (PDRV_WS *)pd)  {
		return;
	}

	while (pws != (PDRV_WS *)pd)  {

		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_COMMON,
		   ("[%d/%d/%d] ccmn_abort_que: aborting CCB 0x%x\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, pws->pws_ccb));

		/*
		 * CCBs marked with PWS_RETRY are just sitting on the 
		 * PDRV queue waiting to be resent.
		 */
		if (!(pws->pws_flags & (PWS_RETRY | PWS_CALLBACK)))  {
			count++;
			if (!abort_ccb)  {
				abort_ccb = ccmn_abort_ccb_bld(pd->pd_dev,
				    	(U32)0, (CCB_HEADER *)pws->pws_ccb);
			} else  {
				abort_ccb->cam_abort_ch = (CCB_HEADER *)pws->pws_ccb;
				(void)ccmn_send_ccb(pd, (CCB_HEADER *)abort_ccb, RETRY);
			}
		}
		pws = pws->pws_flink;
	}

	/*
	 * Return the CCB to the XPT.
	 * Only if something is on the queue NOT PWS_RETRY
	 */
	if( abort_ccb != (CCB_ABORT *)NULL){
	    ccmn_rel_ccb((CCB_HEADER *)abort_ccb);
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_abort_que: exit - success\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return( count);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_term_que()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle sending a Terminate I/O CCB for
 *	each SCSI I/O CCB on the active list.
 *		- If no request are active then return.
 *		- For each request not marked as retry send the terminate ccb.
 *		- Release the Terminate I/O ccb.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This routine must be called SMP safe!
 *
 ************************************************************************/

void
ccmn_term_que(pd)
PDRV_DEVICE *pd;
{
	PDRV_WS 	*pws;
	CCB_TERMIO 	*term_ccb = (CCB_TERMIO *)NULL;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_term_que: entry pd=0x%x\n", 
	   pd->pd_bus, pd->pd_target, pd->pd_lun, pd));

	/*
	 * Check if there is anything to terminate.
	 */
	if ((pws = pd->pd_active_list.flink) == (PDRV_WS *)pd)  {
		return;
	}

	while (pws != (PDRV_WS *)pd)  {

		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_COMMON,
		   ("[%d/%d/%d] ccmn_term_que: terminating CCB 0x%x\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, pws->pws_ccb));

		/*
		 * CCBs marked with PWS_RETRY are just sitting on the 
		 * PDRV queue waiting to be resent.
		 */
		if (!(pws->pws_flags & PWS_RETRY)) {
			if (!term_ccb)  {
				term_ccb = ccmn_term_ccb_bld(pd->pd_dev,
				    	(U32)0, (CCB_HEADER *)pws->pws_ccb);
			} else  {
				term_ccb->cam_termio_ch = (CCB_HEADER *)pws->pws_ccb;
				(void)ccmn_send_ccb(pd, (CCB_HEADER *)term_ccb, RETRY);
			}
		}
		pws = pws->pws_flink;
	}

	/*
	 * Return the CCB to the XPT.
	 * Only if something is on the queue NOT PWS_RETRY
	 */
	if( term_ccb != (CCB_TERMIO *)NULL){
	    ccmn_rel_ccb((CCB_HEADER *)term_ccb);
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_term_que: exit - success\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_get_ccb()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle allocating a CCB by calling
 *	xpt_ccb_alloc() and filling in the common portion of the CCB.
 *
 *  FORMAL PARAMETERS:
 *	dev	   - Indicates b/t/l
 *	func_code  - XPT function code
 *	cam_flags  - cam_flags field of the CCB header
 *	ccb_len    - CCB length
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	CCB_HEADER pointer.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_HEADER *
ccmn_get_ccb(dev, func_code, cam_flags, ccb_len)
dev_t   dev;		/* indicates b/t/l */
u_char  func_code;	/* XPT function code */
U32  cam_flags;	/* cam_flags field of the CCB header */
u_short ccb_len;	/* CCB length */
{
	CCB_HEADER *ccb;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_get_ccb: entry dev=0x%x func=0x%x cam_flags=0x%x ccb_len=%d\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, func_code, cam_flags, ccb_len));

	/*
	 * Allocate a CCB from the XPT layer.
	 */
	ccb = xpt_ccb_alloc();
	/*
	 * Fill in the common fields.
	 */
	ccb->cam_ccb_len = ccb_len;
	ccb->cam_func_code = func_code;
	ccb->cam_path_id = DEV_BUS_ID(dev);
	ccb->cam_target_id = DEV_TARGET(dev);
	ccb->cam_target_lun = DEV_LUN(dev);
	ccb->cam_flags = cam_flags;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_get_ccb: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_rel_ccb()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will return the CCB to the XPT pool and return
 *	the sense data buffer if allocated.
 *
 *  FORMAL PARAMETERS:
 8	ccb	- Pointer to the CCB to be released.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
ccmn_rel_ccb(ccb)
CCB_HEADER *ccb;
{
	PRINTD(ccb->cam_path_id, ccb->cam_target_id,
	   ccb->cam_target_lun, (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_rel_ccb: entry ccb=0x%x\n", ccb->cam_path_id,
	   ccb->cam_target_id, ccb->cam_target_lun, ccb));

	/*
	 * Check if a sense buffer was allocated.
	 */
	if (ccb->cam_func_code == XPT_SCSI_IO)   {
		if (((CCB_SCSIIO *)ccb)->cam_sense_len > DEC_AUTO_SENSE_SIZE) {
		    ccmn_rel_dbuf((u_char *)((CCB_SCSIIO *)ccb)->cam_sense_ptr,
					    ((CCB_SCSIIO *)ccb)->cam_sense_len);
		}
	}
	xpt_ccb_free(ccb);

	PRINTD(ccb->cam_path_id, ccb->cam_target_id,
	   ccb->cam_target_lun, (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_rel_ccb: exit - success\n", ccb->cam_path_id,
	   ccb->cam_target_id, ccb->cam_target_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_io_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating the SCSI I/O CCB.
 *		- Call cmmn_get_ccb(0 to obtain a ccb with the header
 *		  portion filled in.
 *		- Fill in the SCS I/O specific fields.
 *		- If more sense data is needed that allocated then call
 *		  ccmn_get_dbuf().
 *		- Set up the working set fields of the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	data_addr	- Pointer to data area.
 *	data_len	- Length of data transfer.
 *	sense_len	- Length of request sense data.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *	comp_func	- Pointer to callback completion function.
 *	tag_action	- Action for tagged commands.
 *	timeout		- Command timeout.
 *	bp		- Pointer to buf structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to a SCSI I/O CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	CDB information is not filled in by this routine.
 *	A sense buffer will be allocated for devices which have a
 *	sense data length that is greater than the reserved sense buffer
 *	can hold.
 *
 ************************************************************************/

CCB_SCSIIO *
ccmn_io_ccb_bld(dev, data_addr, data_len, sense_len, cam_flags, comp_func,
tag_action, timeout,  bp)
dev_t   dev;
u_char  *data_addr;
U32  data_len;
u_short sense_len;
U32  cam_flags;
void    (*comp_func)();
u_char  tag_action;
U32  timeout;
struct  buf *bp;
{
	CCB_SCSIIO *ccb;
	PDRV_WS *pws;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_io_ccb_bld: entry dev=0x%x data_addr=0x%x data_len=%d",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), dev,
	   data_addr, data_len));

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   (" sense_len=%d flags=0x%x tag_action=0x%x bp=0%x\n",
	   sense_len, cam_flags, tag_action, bp));
	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_SCSIIO *)ccmn_get_ccb(dev, XPT_SCSI_IO, cam_flags,
					(u_short)sizeof(CCB_SCSIIO));
	/*
	 * Fill in the SCSI I/O specific fields.
	 */
	ccb->cam_cbfcnp      = comp_func;
	ccb->cam_data_ptr    = data_addr;
	ccb->cam_dxfer_len   = data_len;
	ccb->cam_sense_len   = sense_len;
	ccb->cam_req_map     = (u_char *)bp;
	ccb->cam_tag_action  = tag_action;
	ccb->cam_timeout     = timeout;
	/*
	 * Check whether we need to allocate a request sense buffer.
	 */
	if (sense_len > DEC_AUTO_SENSE_SIZE)	{
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev), CAMD_COMMON,
		   ("[%d/%d/%d] ccmn_io_ccb_bld: allocating sense buf len=%d for CCB 0x%x\n",
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
		   sense_len, ccb));
		ccb->cam_sense_ptr = ccmn_get_dbuf(sense_len);
	}  else  {
		ccb->cam_sense_ptr = 
			((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_sense_buf;
	}
	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pws->pws_pdrv = (u_char *)GET_PDRV_PTR(dev);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_io_ccb_bld: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_gdev_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the GET
 *	DEVICE TYPE CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *	inq_addr	- Pointer to location for Inquiry data.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to a GET DEVICE TYPE CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_GETDEV *
ccmn_gdev_ccb_bld(dev, cam_flags, inq_addr)
dev_t dev;
U32 cam_flags;
u_char *inq_addr;
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_GETDEV 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_gdev_ccb_bld: entry dev=0x%x flags=0x%x inq_addr=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags, inq_addr));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_gdev_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_GETDEV *)NULL);
	}
	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_GETDEV *)ccmn_get_ccb(dev, XPT_GDEV_TYPE, cam_flags,
					(u_short)sizeof(CCB_GETDEV));

	ccb->cam_inq_data = (char *)inq_addr;
	/*
	 * Issue it to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_gdev_ccb_bld: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_sdev_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the SET
 *	DEVICE TYPE CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *	dev_type	- Device type.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to a SET DEVICE TYPE CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_SETDEV *
ccmn_sdev_ccb_bld(dev, cam_flags, dev_type)
dev_t dev;
U32 cam_flags;
U32 dev_type;
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_SETDEV 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_sdev_ccb_bld: entry dev=0x%x flags=0x%x dev_type=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags, dev_type));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_sdev_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_SETDEV *)NULL);
	}

	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_SETDEV *)ccmn_get_ccb(dev, XPT_SDEV_TYPE, cam_flags,
					(u_short)sizeof(CCB_SETDEV));

	ccb->cam_dev_type = (u_char)dev_type;
	/*
	 * Issue it to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_sdev_ccb_bld: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_sasy_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the SET
 *	ASYNCHRONOUS CALLBACK CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *	async_flags	- Asynchronous flags.
 *	callb_func	- Asynchronous callback function.
 *	buf		- Peripheral driver buffer pointer.
 *	buflen		- Length of peripheral driver buffer.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to a SET ASYNCHRONOUS CALLBACK CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_SETASYNC *
ccmn_sasy_ccb_bld(dev, cam_flags, async_flags, callb_func, buf, buflen)
dev_t  dev;
U32 cam_flags;
U32 async_flags;
void   (*callb_func)();
u_char *buf;
u_char buflen;
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_SETASYNC 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_sasy_ccb_bld: entry dev=0x%x cam_flags=0x%x",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags));

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   (" async_flags=0x%x buf=0x%x buflen=0x%x\n",
	   async_flags, buf, buflen));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_sasy_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_SETASYNC *)NULL);
	}
	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_SETASYNC *)ccmn_get_ccb(dev, XPT_SASYNC_CB, cam_flags,
					(u_short)sizeof(CCB_SETASYNC));

	ccb->cam_async_flags = async_flags;
	ccb->cam_async_func = callb_func;
	ccb->pdrv_buf = buf;
	ccb->pdrv_buf_len = buflen;
	/*
	 * Issue it to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_sasy_ccb_bld: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_rsq_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the RELEASE
 *	SIM QUEUE CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to a RELEASE SIM QUEUE CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_RELSIM *
ccmn_rsq_ccb_bld(dev, cam_flags)
dev_t  dev;
U32 cam_flags;
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_RELSIM 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_rsq_ccb_bld: entry dev=0x%x flags=0x%x \n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_rsq_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_RELSIM *)NULL);
	}
	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_RELSIM *)ccmn_get_ccb(dev, XPT_REL_SIMQ, cam_flags,
					(u_short)sizeof(CCB_RELSIM) );
	/*
	 * Send the request to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_rsq_ccb_bld: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);

}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_pinq_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the PATH
 *	INQUIRY CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to a PATH INQUIRY CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_PATHINQ *
ccmn_pinq_ccb_bld(dev, cam_flags)
dev_t  dev;
U32 cam_flags;
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_PATHINQ 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_pinq_ccb_bld: entry dev=0x%x flags=0x%x \n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_pinq_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_PATHINQ *)NULL);
	}

	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_PATHINQ *)ccmn_get_ccb(dev, XPT_PATH_INQ, cam_flags,
					(u_short)sizeof(CCB_PATHINQ));

	/*
	 * Send the request to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_pinq_ccb_bld: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_abort_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the ABORT CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *	abort_ccb	- Pointer to SCSI I/O CCB to be aborted.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to an ABORT CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_ABORT *
ccmn_abort_ccb_bld(dev, cam_flags, abort_ccb)
dev_t  dev;
U32 cam_flags;
CCB_HEADER *abort_ccb;	/* pointer to CCB to abort */
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_ABORT 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_abort_ccb_bld: entry dev=0x%x flags=0x%x ccb=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags, abort_ccb));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_abort_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_ABORT *)NULL);
	}
	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_ABORT *)ccmn_get_ccb(dev, (u_char)XPT_ABORT, cam_flags,
					(u_short)sizeof(CCB_ABORT));
	ccb->cam_abort_ch = abort_ccb;

	/*
	 * Send the request to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_abort_ccb_bld: exit\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_term_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the TERMINATE CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *	term_ccb	- Pointer to SCSI I/O CCB to be terminated.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to the TERMINATE I/O CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_TERMIO *
ccmn_term_ccb_bld(dev, cam_flags, term_ccb)
dev_t  dev;
U32 cam_flags;
CCB_HEADER *term_ccb;	/* pointer to CCB to terminate */
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_TERMIO 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_term_ccb_bld: entry dev=0x%x flags=0x%x ccb=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags, term_ccb));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_term_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_TERMIO *)NULL);
	}

	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_TERMIO *)ccmn_get_ccb(dev, XPT_TERM_IO, cam_flags,
					(u_short)sizeof(CCB_TERMIO));
	ccb->cam_termio_ch = term_ccb;

	/*
	 * Send the request to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), CAMD_COMMON,
 	   ("[%d/%d/%d] ccmn_term_ccb_bld: exit - succes\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_bdr_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the BUS
 *	DEVICE RESET CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to the BUS DEVICE RESET CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_RESETDEV  *
ccmn_bdr_ccb_bld(dev, cam_flags)
dev_t  dev;
U32 cam_flags;
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_RESETDEV 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_bdr_ccb_bld: entry dev=0x%x flags=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_bdr_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_RESETDEV *)NULL);
	}
	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_RESETDEV *)ccmn_get_ccb(dev, XPT_RESET_DEV, cam_flags,
					(u_short)sizeof(CCB_RESETDEV));

	/*
	 * Send the request to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_bdr_ccb_bld: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_br_ccb_bld()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating and sending the BUS RESET CCB.
 *		- Call ccmn_get_ccb() to obtain a ccb with the 
 *		  header portion filled in.
 *		- Call ccmn_send_ccb().
 *		- Return the ccb.
 *
 *  FORMAL PARAMETERS:
 *	dev		- Major/minor device number.
 *	cam_flags	- Cam_flags field of the CCB_HEADER.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to the BUS RESET CCB.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

CCB_RESETBUS  *
ccmn_br_ccb_bld(dev, cam_flags)
dev_t  dev;
U32 cam_flags;
{
	PDRV_DEVICE 	*pd = GET_PDRV_PTR(dev);
	CCB_RESETBUS 	*ccb;
	int 		spl;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_br_ccb_bld: entry dev=0x%x flags=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cam_flags));

	if ( pd == (PDRV_DEVICE *)NULL )   {
	   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev),  DEV_LUN(dev),
	      (CAMD_COMMON | CAMD_ERRORS),
	      ("[%d/%d/%d] ccmn_br_ccb_bld: No peripheral device struct\n",
	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	   return((CCB_RESETBUS *)NULL);
	}

	/*
	 * Obtain a CCB with the header portion filled in.
	 */
	ccb = (CCB_RESETBUS *)ccmn_get_ccb(dev, XPT_RESET_BUS, cam_flags,
					(u_short)sizeof(CCB_RESETBUS));

	/*
	 * Send the request to the XPT.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_br_ccb_bld: exit - success\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev)));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_tur()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating a SCSI I/O CCB for the
 *	TEST UNIT READY command and send it to the XPT for
 *	processing.
 *		- Call ccmn_io_ccb_bdl() to allocate and fill in a SCSI
 *		  I/O ccb.
 *		- Fill in the cdb.
 *		- Call ccmn_send_ccb to send the request to the XPT.
 *
 *  FORMAL PARAMETERS:
 *	pd		- Pointer to peripheral device structure.
 *	sense_len	- Sense data length.
 *	cam_flags	- Cam_flags field of CCB_HEADER.
 *	comp_func	- Pointer to the callback completion function.
 *	tag_action	- Action for tagged requests.
 *	timeout		- Command timeout.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to SCSI I/O ccb.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This routine may be called from interrupt context.
 *
 ************************************************************************/

CCB_SCSIIO *
ccmn_tur(pd, sense_len, cam_flags, comp_func, tag_action, timeout)
PDRV_DEVICE *pd;
u_short sense_len;
U32 cam_flags;
void (*comp_func)();
u_char tag_action;
U32 timeout;
{
	CCB_SCSIIO 	*ccb;
	ALL_TUR_CDB 	*cdb;
	int 		spl;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_tur: entry pd=0x%x sense_len=%d flags=0x%x tag_action=0x%x timeout=%d\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun,
	   pd, sense_len, cam_flags, tag_action, timeout));
	/*
	 * Obtain a SCSI I/O CCB.
	 */
	ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)0, (U32)0, sense_len, 
		cam_flags, comp_func, tag_action, timeout, (struct buf *)0);

	/*
	 * Fill in the CDB for the Test Unit Ready command.
	 */
	cdb = (ALL_TUR_CDB *)ccb->cam_cdb_io.cam_cdb_bytes;
	cdb->opcode = ALL_TUR_OP;
	ccb->cam_cdb_len = sizeof(ALL_TUR_CDB);

	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	(void)ccmn_send_ccb_wait(pd, (CCB_HEADER *)ccb, NOT_RETRY, PZERO);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_tur: exit - success\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_start_unit()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle creating a SCSI I/O CCB for the
 *	START/STOP UNIT command and send it to the XPT for
 *	processing.
 *		- Call ccmn_io_ccb_bdl() to allocate and fill in a SCSI
 *		  I/O ccb.
 *		- Fill in the cdb.
 *		- Call ccmn_send_ccb to send the request to the XPT.
 *
 *  FORMAL PARAMETERS:
 *	pd		- Pointer to peripheral device structure.
 *	sense_len	- Sense data length.
 *	cam_flags	- Cam_flags field of CCB_HEADER.
 *	comp_func	- Pointer to the callback completion function.
 *	tag_action	- Action for tagged requests.
 *	timeout		- Command timeout.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to SCSI I/O ccb.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This routine may be called from interrupt context.
 *
 ************************************************************************/

CCB_SCSIIO *
ccmn_start_unit(pd, sense_len, cam_flags, comp_func, tag_action, timeout)
PDRV_DEVICE *pd;
u_short sense_len;
U32 cam_flags;
void (*comp_func)();
u_char tag_action;
U32 timeout;
{
	CCB_SCSIIO     *ccb;
	int  	       spl;
	DIR_START_CDB6 *cdb;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_start_unit: entry pd=0x%x sense_len=%d flags=0x%x tag_action=0x%x timeout=%d\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun,
	   pd, sense_len, cam_flags, tag_action, timeout));
	/*
	 * Obtain a SCSI I/O CCB.
	 */
	ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)0, (U32)0, sense_len, 
			cam_flags, comp_func, tag_action, timeout, 
			(struct buf *)0);

	/*
	 * Fill in the CDB for the Start Unit command.
	 */
	cdb = (DIR_START_CDB6 *)ccb->cam_cdb_io.cam_cdb_bytes;
	cdb->opcode = DIR_START_OP;
	cdb->start = 1;
	ccb->cam_cdb_len = sizeof(DIR_START_CDB6);

	PDRV_IPLSMP_LOCK(pd, LK_ONCE, spl);
	(void)ccmn_send_ccb_wait(pd, (CCB_HEADER *)ccb, NOT_RETRY, PZERO);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_start_unit: exit - success\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_mode_select()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will create and send a  SCSI I/O CCB for the
 *	six byte MODE SELECT command for a particular page in the
 *	mode select table indexed by the ms_index parameter.
 *		- Call ccmn_io_ccb_bdl() to allocate and fill in a SCSI
 *		  I/O ccb.
 *		- Fill in the cdb.
 *		- Call ccmn_send_ccb_wait to send the request to the XPT.
 *
 *  FORMAL PARAMETERS:
 *	pd		- Pointer to peripheral device structure.
 *	sense_len	- Sense data length.
 *	cam_flags	- Cam_flags field of CCB_HEADER.
 *	comp_func	- Pointer to the callback completion function.
 *	tag_action	- Action for tagged requests.
 *	timeout		- Command timeout.
 *	ms_index	- Index into the Mode Select table for this
 *			  request.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to SCSI I/O ccb.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This routine may be called from interrupt context.
 *
 ************************************************************************/

CCB_SCSIIO *
ccmn_mode_select(pd, sense_len, cam_flags, comp_func, tag_action,
timeout, ms_index)
PDRV_DEVICE *pd;
u_short     sense_len;
U32      cam_flags;
void        (*comp_func)();
u_char      tag_action;
U32      timeout;
unsigned    ms_index;	/*  Index into Mode Select table */
{
	CCB_SCSIIO 	*ccb;
	struct ms_entry *ms;
	int 		spl;
	ALL_MODE_SEL_CDB6 *cdb;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
	   ("[%d/%d/%d] ccmn_start_unit: entry pd=0x%x sense_len=%d flags=0x%x tag_action=0x%x timeout=%d ms_index=%d\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun,
	   pd, sense_len, cam_flags, tag_action, timeout, ms_index));

	ms = &pd->pd_dev_desc->dd_modesel_tbl->ms_entry[ms_index];
	/*
	 * Obtain a SCSI I/O CCB.
	 */
	ccb = ccmn_io_ccb_bld(pd->pd_dev, ms->ms_data, (U32)ms->ms_data_len,
		sense_len, cam_flags, comp_func, tag_action, timeout, 
		(struct buf *)0);

	/*
	 * Fill in the CDB for the Mode Select command.
	 */
	cdb = (ALL_MODE_SEL_CDB6 *)ccb->cam_cdb_io.cam_cdb_bytes;
	cdb->opcode = ALL_MODE_SEL6_OP;
	if (ms->ms_ent_sp_pf & MSEL_SAVE_PAGE)
		cdb->sp = 1;
	if ( ms->ms_ent_sp_pf & MSEL_SCSI2)
		cdb->pf = 1;
	cdb->param_len = ms->ms_data_len;
	ccb->cam_cdb_len = sizeof(ALL_MODE_SEL_CDB6);

	ccb->cam_data_ptr = ms->ms_data;
	ccb->cam_dxfer_len = ms->ms_data_len;

	PDRV_IPLSMP_LOCK(pd, LK_ONCE, spl);
	(void)ccmn_send_ccb_wait(pd, (CCB_HEADER *)ccb, NOT_RETRY, PZERO);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_COMMON | CAMD_INOUT),
 	   ("[%d/%d/%d] ccmn_mode_select: exit - success\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ccb);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_ccb_status()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will boil down the numerous CAM status values
 *	into a few generic classifications. (Refer to cam.h).  The
 *	following table shows the returned category for each of the
 *	CAM status values:
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	CAT_INPROG      The request is in progress.
 *	CAT_CMP         The request has completed without error.
 *	CAT_CMP_ERR     The request has completed with error.
 *	CAT_ABORT       The request has been aborted or terminate or
 *       		cannot be aborted or terminated.
 *	CAT_BUSY	CAM is busy.
 *	CAT_SCSI_BUSY	SCSI is busy.
 *	CAT_NO_DEVICE   No device at request address.
 *	CAT_DEVICE_ERR  Bus/device problems.
 *	CAT_BAD_AUTO    Invalid autosense data.
 *	CAT_CCB_ERR     Invalid CCB.
 *	CAT_RESET       The unit/bus has detected a reset condition.
 *	CAT_UNKNOWN     Invalid CAM status.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

U32
ccmn_ccb_status(ccb)
CCB_HEADER *ccb;
{
	U32 retval;

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun,
	   	(CAMD_COMMON | CAMD_INOUT),
		("[%d/%d/%d] ccmn_ccb_status: entry ccb=0x%x status=0x%x\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun,
		ccb, ccb->cam_status));

	switch( ccb->cam_status & CAM_STATUS_MASK) {

	case CAM_REQ_INPROG:
		retval =  CAT_INPROG;
		break;

	case CAM_REQ_CMP:
		retval =  CAT_CMP;
		break;

	case CAM_REQ_ABORTED:
	case CAM_UA_ABORT:
	case CAM_UA_TERMIO:
	case CAM_REQ_TERMIO:
		retval =  CAT_ABORT;
		break;

	case CAM_REQ_CMP_ERR:
		retval = CAT_CMP_ERR;
		break;

	case CAM_BUSY:
		retval = CAT_BUSY;
		break;

	case CAM_SCSI_BUSY:
		retval = CAT_SCSI_BUSY;
		break;

	case CAM_REQ_INVALID:
	case CAM_CCB_LEN_ERR:
	case CAM_PROVIDE_FAIL:
	case CAM_FUNC_NOTAVAIL:
		retval = CAT_CCB_ERR;
		break;

	case CAM_PATH_INVALID:
	case CAM_DEV_NOT_THERE:
	case CAM_NO_HBA:
	case CAM_LUN_INVALID:
	case CAM_TID_INVALID:
	case CAM_NO_NEXUS:
	case CAM_IID_INVALID:
		retval = CAT_NO_DEVICE;
		break;

	case CAM_SEL_TIMEOUT:
	case CAM_CMD_TIMEOUT:
	case CAM_MSG_REJECT_REC:
	case CAM_UNCOR_PARITY:
	case CAM_DATA_RUN_ERR:
	case CAM_UNEXP_BUSFREE:
	case CAM_SEQUENCE_FAIL:
		retval = CAT_DEVICE_ERR;
		break;

	case CAM_AUTOSENSE_FAIL:
		retval = CAT_BAD_AUTO;
		break;

	case CAM_BDR_SENT:
	case CAM_SCSI_BUS_RESET:
		retval = CAT_RESET;
		break;

	default:
		retval = CAT_UNKNOWN;
		break;
	}

	PRINTD(ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun,
	   	(CAMD_COMMON | CAMD_INOUT),
    		("[%d/%d/%d] ccmn_ccb_status: exit - retval=%d\n",
		ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun,
		retval));

	return( retval );
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_get_bp()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will allocate a buf structure by calling
 *	KM_ALLOC().
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Buf structure pointer.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/


struct buf *
ccmn_get_bp()
{
	struct buf *bp_ptr;
	int spl;

	PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_COMMON | CAMD_INOUT),
		("[b/t/l] ccmn_get_bp: entry\n"));

	CCMN_BP_IPLSMP_LOCK(LK_RETRY, spl);

	/*
	 * Check whether we need to increase the pool.
	 */
	if(ccmn_bp_head.num_bp <= ccmn_bp_low_water)  {
		ccmn_alloc_bp_pool(ccmn_bp_increment);
	}
	while(( bp_ptr = ccmn_bp_head.bp_list) == (struct buf *)NULL){
		CCMN_BP_WAIT(spl);
	}
	ccmn_bp_head.bp_list = bp_ptr->b_actf;
	ccmn_bp_head.num_bp--;

	CCMN_BP_IPLSMP_UNLOCK(spl);

	PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_COMMON | CAMD_INOUT),
		("[b/t/l] ccmn_get_bp: exit bp=0x%x\n", bp_ptr));

	return(bp_ptr);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_rel_bp()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will deallocate a buf structure by calling
 *	KM_FREE().
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
ccmn_rel_bp(bp)
struct buf *bp;
{
	int spl;

	PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_COMMON | CAMD_INOUT),
		("[b/t/l] ccmn_rel_bp: entry bp=0x%x\n", bp));



	CCMN_BP_IPLSMP_LOCK(LK_RETRY, spl);

	/*
	 * Is there anyone waiting This is a timeout routine call.
	 */
	if(bp == (struct buf *)NULL){
		if(ccmn_bp_head.bp_wait_cnt != NULL) {
			thread_wakeup((vm_offset_t)&ccmn_bp_head.bp_wait_cnt);
			/* 
			 * Do it again in 2 seconds
			 */
			timeout(ccmn_rel_bp, NULL, (2 * hz));
		}
		return;
	}
		

	bzero(bp, sizeof(struct buf));

	bp->b_actf = ccmn_bp_head.bp_list;
	ccmn_bp_head.bp_list = bp;
	ccmn_bp_head.num_bp++;

	if((ccmn_bp_head.num_bp >= ccmn_bp_high_water) && 
			(ccmn_bp_head.bp_wait_cnt == NULL))
		ccmn_free_bp_pool(ccmn_bp_increment);
	/*
	 * Anyone waiting
	 */
	if( ccmn_bp_head.bp_wait_cnt ) {
	    thread_wakeup((vm_offset_t)&ccmn_bp_head.bp_wait_cnt);
	}

	CCMN_BP_IPLSMP_UNLOCK(spl);

	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_COMMON,
		("[b/t/l] ccmn_rel_bp: exit - success\n"));
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_alloc_bp_pool()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will allocate a pool of buf structures by calling
 *	KM_ALLOC() and link them together.
 *
 *  FORMAL PARAMETERS:
 *	num_bp - Number of buf structures to allocate.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to first buf structure in linked list.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This function must not be called at interrupt context.
 *
 ************************************************************************/

struct buf *
ccmn_alloc_bp_pool(num_bp)
int num_bp;
{
	int i;
	struct buf *first_bp, *bp;
	caddr_t ptr;

	/*
	 * Allocate the bp pool - create a linked list of buf structures.
	 */
	for(i=0; i<num_bp; i++)  {

	        ptr = (char *)cam_zalloc(sizeof(struct buf));

		if(( ptr == (char *)NULL) && (i == 0 )){
			return;
		}

		if(i==0)  {
			first_bp = (struct buf *)ptr;
			bp = (struct buf *)ptr;
		} else if ( ptr == (char *)NULL) {
			break;

		} else {
			bp->b_actf = (struct buf *)ptr;
			bp = (struct buf *)ptr;
		}
	}

	/* 
	 * The last buffer will point to the beginning of the free bp pool.
	 */
	bp->b_actf = ccmn_bp_head.bp_list;

	ccmn_bp_head.bp_list = first_bp;
	ccmn_bp_head.num_bp  += i;
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_free_bp_pool()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will deallocate buf structures by calling
 *	KM_FREE().
 *
 *  FORMAL PARAMETERS:
 *	num_bp - Number of buf structures to deallocate.
 *	bp_list - Pointer to first buf structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
ccmn_free_bp_pool(num_bp)
int num_bp;
{
	int i;
	struct buf *tmp_bp, *bp;

	/*
	 * Free some buf structures from the bp pool.
	 */
	bp = ccmn_bp_head.bp_list;

	for(i=0; i<num_bp; i++)  {
		tmp_bp = bp->b_actf;
		cam_zfree((char *)bp,sizeof( struct buf));
		bp = tmp_bp;
	}
	ccmn_bp_head.num_bp -= num_bp;
	ccmn_bp_head.bp_list = bp;
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_get_dbuf()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will allocate a data buffer area for the size
 *	specified  by calling KM_ALLOC().
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	Pointer to allocated space.
 *
 *  SIDE EFFECTS:
 *	Pointer to kernel data space.
 *	NULL - no buffers are available and cannot allocate more.
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

u_char *
ccmn_get_dbuf(size)
U32 size;
{
	caddr_t dbuf;
	int spl;

	PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_COMMON | CAMD_INOUT),
		("[b/t/l] ccmn_get_dbuf: entry size=%d\n", size));
    
	CCMN_DATA_IPLSMP_LOCK( LK_RETRY, spl);

	/* 
	 * Are we being called by timeout
	 */
	if( size == NULL){
		/* 
		 * Is anyone waiting
		 */
		if(ccmn_data_alloc.alloc_wait_cnt){
			thread_wakeup((vm_offset_t)&ccmn_data_alloc.alloc_wait_cnt);
			timeout( ccmn_get_dbuf, NULL, (2 * hz));
		}
		return((u_char *)NULL);
	}

	while((dbuf = (char *)cam_zalloc(size)) == (caddr_t)NULL){
	    CCMN_DATA_WAIT( spl );
	}

	CCMN_DATA_IPLSMP_UNLOCK( spl );
	PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_COMMON | CAMD_INOUT),
		("[b/t/l] ccmn_get_dbuf: exit - dbuf=0x%x\n", dbuf));

	return((u_char *)dbuf);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_rel_dbuf()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will deallocate a data buffer by calling
 *	KM_FREE().
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void
ccmn_rel_dbuf(addr, size)
u_char *addr;
u_int	size;
{
	int spl;

	PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_COMMON | CAMD_INOUT),
		("[b/t/l] ccmn_rel_dbuf: entry addr=0x%x\n", addr));

	CCMN_DATA_IPLSMP_LOCK( LK_RETRY, spl);


	cam_zfree((char *)addr,size);

	if(ccmn_data_alloc.alloc_wait_cnt){
		thread_wakeup((vm_offset_t)&ccmn_data_alloc.alloc_wait_cnt);
	}

	CCMN_DATA_IPLSMP_UNLOCK(spl);

	PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_COMMON | CAMD_INOUT),
		("[b/t/l] ccmn_rel_dbuf: exit - success\n"));
}

/************************************************************************
 *
 *  ROUTINE NAME: ccmn_send_ccb_wait()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function sends a ccb and waits for it to complete.
 *		- If the status returned from ccmn_send_ccb() is CAM_REQ_INPROG:
 *			- If the priority requires checking for signals
 *			  then sleep at a brekable priority.
 *			- Otherwise sleep at a non-breakable priority.
 *
 *  FORMAL PARAMETERS:
 *	pd	 - Peripheral device structure.
 *	ccb	 - Pointer to CCB to wait on.
 *	retry	 - Indiactes whether the ccb is already on the queue.
 *	sleep_pri - Software priority to sleep at.
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
 *	This function must be called with the peripheral device
 *	structure lopcked.
 *
 ************************************************************************/

int
ccmn_send_ccb_wait(pd, ccb, retry, sleep_pri)
PDRV_DEVICE *pd;
CCB_HEADER *ccb;
u_char retry;	
int sleep_pri;
{
	u_long status;

	/*
	 * We are done.
	 */
	if( (status = ccmn_send_ccb(pd, ccb, retry)) != CAM_REQ_INPROG) {
		return(status);
	}

	if (sleep_pri> PZERO)  {
		/*
		 * We sleep on address of ccb checking for signals.
		 */
		if (PDRV_SMP_SLEEPUNLOCK(ccb, (sleep_pri | PCATCH), pd))  {
		    PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
			   (CAMD_COMMON | CAMD_INOUT),
	    	    	("[%d/%d/%d] ccmn_send_ccb_wait: interrupted sleep for CCB 0x%x\n",
			pd->pd_bus, pd->pd_target, pd->pd_lun, ccb));
		    return(EINTR);
		}
	} else {
		/*
		 * We sleep on address of ccb but NON interruptable.
		 */
		PDRV_SMP_SLEEPUNLOCK(ccb, sleep_pri, pd);
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
		(CAMD_COMMON | CAMD_INOUT),
		("[%d/%d/%d] ccmn_send_ccb_wait: exit - success\n",
		pd->pd_bus, pd->pd_target, pd->pd_lun));

	return (0);
}

/************************************************************************
 *
 *  ROUTINE NAME: ccmn_ccbwait()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function waits for a CCB request to complete.
 *		- While the cam status is CAM_REQ_INPROG:
 *			- If the priority requires checking for signals
 *			  then sleep at a brekable priority.
 *			- Otherwise sleep at a non-breakable priority.
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
ccmn_ccbwait(ccb, priority)
CCB_SCSIIO *ccb;	/* Pointer to CCB to wait for */
int priority;		/* Sleep priority */
{
	int 	spl;

	PDRV_WS *pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	PDRV_DEVICE *pd = (PDRV_DEVICE *)pws->pws_pdrv;

	PRINTD(ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id, 
		ccb->cam_ch.cam_target_lun, (CAMD_COMMON | CAMD_INOUT),
		("[%d/%d/%d] ccmn_ccbwait: entry ccb=0x%x pri=0x%x\n",
		ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
	 	ccb->cam_ch.cam_target_lun, ccb, priority));

	PDRV_IPLSMP_LOCK(pd, LK_ONCE, spl);
	while (CAM_STATUS(ccb) == CAM_REQ_INPROG) {
	    if (priority > PZERO)  {
		/*
		 * We sleep on address of ccb checking for signals.
		 */
		if (PDRV_SMP_SLEEPUNLOCK(ccb, (priority | PCATCH), pd))  {
		    PDRV_LOWER_IPL(spl);
		    PRINTD(ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id, 
			ccb->cam_ch.cam_target_lun, CAMD_COMMON,
	    	    	("[%d/%d/%d] ccmn_ccbwait: interrupted sleep for CCB 0x%x\n",
			ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
	 		ccb->cam_ch.cam_target_lun, ccb));
		    return(EINTR);
		}
	    } else {
		/*
		 * We sleep on address of ccb but NON interruptable.
		 */
		PDRV_SMP_SLEEPUNLOCK(ccb, priority, pd);
		/*
		 * Get the lock again.
		 */
		PDRV_SMP_LOCK(pd);
	    }
	}
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id, 
		ccb->cam_ch.cam_target_lun, (CAMD_COMMON | CAMD_INOUT),
		("[%d/%d/%d] ccmn_ccbwait: exit - success\n",
		ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
	 	ccb->cam_ch.cam_target_lun));

	return (0);
}

/************************************************************************
 *
 *  ROUTINE NAME:	ccmn_SysSpecialCmd()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	The purpose of this function is to permit a system request to
 *  issue SCSI I/O commands to the special I/O interface.  This permits
 *  existing SCSI commands to be issued from within kernel code.
 *
 *  FORMAL PARAMETERS:
 *	dev	= The device major/minor #.
 *	cmd	= The SCSI command code.
 *	data	= The data buffer address.
 *	flags	= The file open flags.
 *	sflags	= The special I/O flags.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	Returns 0 for SUCCESS, or error code on failures.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/
int
ccmn_SysSpecialCmd (dev, cmd, data, flags, ccb, sflags)
dev_t dev;
int cmd;
caddr_t data;
int flags; 
CCB_SCSIIO *ccb;
int sflags;
{
	int status;
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
		(CAMD_COMMON | CAMD_INOUT),
	("[%d/%d/%d] ccmn_SysSpecialCmd: dev = 0x%x, cmd = 0x%x, data = 0x%x\n",
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   	dev, cmd, data));
	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   	(CAMD_COMMON | CAMD_INOUT),
	("[%d/%d/%d] ccmn_SysSpecialCmd: flags = 0x%x, ccb = 0x%x, sflags = 0x%x\n",
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   	flags, ccb, sflags));
	
	bzero ((char *) sp, sizeof(*sp));
	sp->sp_flags = (sflags & SA_USER_FLAGS_MASK);
	sp->sp_sub_command = cmd;
	sp->sp_iop_length = ((cmd & ~(IOC_INOUT|IOC_VOID)) >> 16);
	sp->sp_iop_buffer = data;
	status = ccmn_DoSpecialCmd (dev, SCSI_SPECIAL, sp,
					flags, ccb, SA_SYSTEM_REQUEST);
	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   	(CAMD_COMMON | CAMD_INOUT),
		("[%d/%d/%d] ccmn_SysSpecialCmd: EXIT - status = %d (%s)\n",
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
		status, cdbg_SystemStatus(status)));
	return (status);
}

/************************************************************************
 *
 *  ROUTINE NAME:	ccmn_DoSpecialCmd()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine is used to prepare for and issue special commands.
 *  It provides a simplified interface to the special command routine.
 *
 *  FORMAL PARAMETERS:
 *	dev	= The device major/minor #.
 *	cmd	= I/O control command.
 *	data	= User data buffer.
 *	flags	= The file open flags.
 *	ccb	= Pointer to SCSI I/O CCB (optional).
 *	sflags	= Special request flags.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	Returns 0 for SUCCESS, or error code on failures.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/
int
ccmn_DoSpecialCmd (dev, cmd, data, flags, ccb, sflags)
register dev_t dev;
int cmd;
caddr_t data;
int flags;
CCB_SCSIIO *ccb;
int sflags;
{
	PDRV_DEVICE *pd;
	DEV_DESC *dd;
	PDRV_WS *pws;
	struct special_args *sap;
	ALL_INQ_DATA *inq;
	int bus = DEV_BUS_ID(dev);
	int target = DEV_TARGET(dev);
	int lun = DEV_LUN(dev);
	int status;

	PRINTD(bus, target, lun, (CAMD_INOUT | CAMD_COMMON),
	("[%d/%d/%d] ccmn_DoSpecialCmd: dev = 0x%x, cmd = 0x%x, data = 0x%x\n",
	   	bus, target, lun, dev, cmd, data));
	PRINTD(bus, target, lun, (CAMD_INOUT | CAMD_COMMON),
	("[%d/%d/%d] ccmn_DoSpecialCmd: flags = 0x%x, ccb = 0x%x, sflags = 0x%x\n",
	   	bus, target, lun, flags, ccb, sflags));
	
	if ( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *) 0) {
	    PRINTD(bus, target, lun, (CAMD_COMMON | CAMD_ERRORS),
	("[%d/%d/%d] ccmn_DoSpecialCmd: No peripheral device structure.\n",
		bus, target, lun));
	    return (ENODEV);
	}
	dd = pd->pd_dev_desc;
	inq = (ALL_INQ_DATA *)pd->pd_dev_inq;

	/*
	 * Obtain a special argument buffer for the request.
	 */
	sap = scmn_GetArgsBuffer (bus, target, lun);
	if (sap == (struct special_args *) 0) {
	    return (ENOMEM);
	}

	/*
	 * Setup the initial special arguments.
	 */
	sap->sa_flags = (sflags & (SA_SYSTEM_REQUEST | SA_USER_FLAGS_MASK));
	sap->sa_dev = dev;
	sap->sa_ioctl_cmd = cmd;
	sap->sa_ioctl_data = data;
	sap->sa_ioctl_scmd = 0;
	sap->sa_file_flags = flags;
	sap->sa_device_name = (caddr_t)dd->dd_dev_name;
	sap->sa_device_type = inq->dtype;
	sap->sa_sense_length = pd->pd_sense_len;
	sap->sa_sense_buffer = pd->pd_sense_ptr;
	sap->sa_bp = (struct buf *) 0;
	/*
	 * Allocate a CAM Control Block (if necessary).
	 */
	if (ccb == (CCB_SCSIIO *) 0) {
	    ccb = (CCB_SCSIIO *) xpt_ccb_alloc();
	    sap->sa_flags |= SA_ALLOCATED_CCB;
	    PRINTD (bus, target, lun, (CAMD_COMMON | CAMD_CMD_EXP),
	    ("[%d/%d/%d] ccmn_DoSpecialCmd: Allocated CCB buffer at 0x%x.\n",
		    bus, target, lun, ccb));
	}
	sap->sa_ccb = ccb;
	/*
	 * Setup pointer to the peripheral driver structure.
	 */
	if ( (pws = (PDRV_WS *) ccb->cam_pdrv_ptr) != (PDRV_WS *) 0) {
	    pws->pws_pdrv = (u_char *) pd;
	    sap->sa_specific = (caddr_t) pd;
	}
	sap->sa_start = (int (*)()) xpt_action;

	status = scmn_SpecialCmd (sap);

	/*
	 * Clean up all resources if not already freed.
	 */
	scmn_SpecialCleanup(sap);

	(void) scmn_FreeArgsBuffer (sap);

	PRINTD(bus, target, lun, (CAMD_INOUT | CAMD_COMMON),
	    ("[%d/%d/%d] ccmn_DoSpecialCmd: EXIT - status = %d (%s)\n",
	   	bus, target, lun, status, cdbg_SystemStatus(status)));

	return (status);
}


/************************************************************************
 *
 *  ROUTINE NAME:	ccmn_errlog()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine reports error conditions for the drivers.
 * 	The routine is passed a pointer to a string (func_name that
 * 	detetected the error, optional string (Diagnostic string)
 * 	flags (what type of error hard soft informational etc.),
 * 	a CCB pointer, and dev number.
 *
 *  FORMAL PARAMETERS:
 *	func_str	Pointer to func name string
 *	opt_str		Pointer to optional string
 *	flags		Hard, soft, informational
 *	ccb;	 	Pointer to the ccb struct	
 *	dev		Major/minor pair
 *	unused		Unused argument.. CAM_ERROR macro
 *
 *  IMPLICIT INPUTS:
 * 	From the ccb or dev we get the pdrv_device struct
 * 	and the Device specific struct. Build strings based
 *	on error conditions.
 *
 *  IMPLICIT OUTPUTS:
 *	None.
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

/* 
 * define for number of entries for stack
 */
#define CCMN_MAX_ENTRIES	15	
/*
 * error type strings
 */
static u_char	HardError[] = "Hard Error Detected";
static u_char	SoftError[] = "Soft Error Detected (recovered)";
static u_char	InforError[] = "Information Message Detected (recovered)";
static u_char	SoftwareError[] = 
		"Possible Software Problem - Impossible Cond Detected";
static u_char	Unknown[] = "Unknown error type";
static u_char	ActiveCCB[] = "Active CCB at time of error";
static u_char	Active_List[] = "Active List of CCB's";
static u_char	Active_List_Cont[] = "Active List of CCB's Continued";
static u_char	Pend_List[] = "Pending List of CCB's";
static u_char	Pend_List_Cont[] = "Pending List of CCB's Continued";
static u_char	NoPdrv_struct[] = "ccmn_errlog: Has detected no pdrv_device struct ";



void
ccmn_errlog( func_str, opt_str, flags, ccb, dev, unused )
u_char		*func_str;	/* Pointer to func name string		*/
u_char		*opt_str;	/* Pointer to optional string		*/
U32		flags;		/* Hard, soft, informational		*/
CCB_HEADER	*ccb;		/* Pointer to the ccb struct		*/
dev_t		dev;		/* Major/minor pair			*/
u_char		*unused;	/* Unused argument.. CAM_ERROR macro	*/

{

    /* Local Variables	*/

    PDRV_DEVICE		*pdrv_dev;	/* The device struct		*/
    PDRV_WS		*pdrv_ws;	/* The working set pointer 	*/
    ALL_INQ_DATA	*inq;		/* Inquiry data			*/
    CAM_ERR_HDR		err_hdr;	/* The error header struct	*/
    CAM_ERR_ENTRY	err_ent[CCMN_MAX_ENTRIES]; /* Error entry list	*/
    U32		ent = 0;	/* Entry number			*/
    U32 first_time;
    I32		i;


    bzero( &err_hdr, sizeof(CAM_ERR_HDR)); 
    
    err_hdr.hdr_type = CAM_ERR_PKT;
    
    /*
     * Get address of list
     */
    err_hdr.hdr_list = err_ent;
    
    /*
     * Init number of entries
     */
    err_hdr.hdr_entries = ent;

    /* 
     * Init the entry list
     */
    for( i = 0; i < CCMN_MAX_ENTRIES; i++){
	bzero( &err_ent[i], sizeof(CAM_ERR_ENTRY));
    }

    
    /*
     * Starting filling in entry list.....
     */

    /*
     * Function string is alway first....
     */
    if( func_str != NULL){
    	err_ent[ent].ent_type = ENT_STR_MODULE;
    	err_ent[ent].ent_data = func_str;
    	err_ent[ent].ent_size = strlen(func_str) + 1;
    	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
    	/*
    	 * set to next sequential entry
    	 */
    	ent++;
    }
    if( opt_str != NULL){
    	err_ent[ent].ent_type = ENT_STRING;
    	err_ent[ent].ent_data = opt_str;
    	err_ent[ent].ent_size = strlen(opt_str) + 1;
    	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
    	/*
    	 * set to next sequential entry
    	 */
    	ent++;
    }
    
    /*
     * What type of error is it
     */
    if(( flags & CAM_SOFTWARE ) != NULL){
        err_ent[ent].ent_type = ENT_STR_SOFTWARE_ERROR;
        err_ent[ent].ent_data = SoftwareError;
        err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
        /*
         * set to next sequential entry
         */
        ent++;
    }
    else if(( flags & CAM_HARDERR ) != NULL){
        err_ent[ent].ent_type = ENT_STR_HARD_ERROR;
        err_ent[ent].ent_data = HardError;
        err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
        err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
        /*
         * set to next sequential entry
         */
        ent++;
    }
    else if(( flags & CAM_SOFTERR ) != NULL){
        err_ent[ent].ent_type = ENT_STR_SOFT_ERROR;
        err_ent[ent].ent_data = SoftError;
        err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
        /*
         * set to next sequential entry
         */
        ent++;
    }
    else if(( flags & CAM_INFORMATIONAL ) != NULL){
        err_ent[ent].ent_type = ENT_STR_INFOR_ERROR;
        err_ent[ent].ent_data = InforError;
        err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
        /*
         * set to next sequential entry
         */
        ent++;
    }
    else {
        err_ent[ent].ent_type = ENT_STR_UNKNOWN_ERROR;
        err_ent[ent].ent_data = Unknown;
        err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
        /*
         * set to next sequential entry
         */
        ent++;
    }

    /* 
     * Get our pointers
     */
    if( (pdrv_dev = GET_PDRV_PTR( dev )) == NULL) {
	/* 
	 * Things are hosed we have no PDRV_DEVICE pointer
	 * Log what we can.......
	 */
    
        
        /*
         * Since both the class and subsytem follows the scsi
         * device type just copy them over.. But since we don't
	 * have a pdrv_device structure... set to unknown
         */
        err_hdr.hdr_class = CLASS_UNKNOWN;
        err_hdr.hdr_subsystem = SUBSYS_UNKNOWN;
    
    
        /* 
         * Get the priority of this entry to severe 
         */
    	err_hdr.hdr_pri = CAM_ERR_SEVERE;
    
        /*
         * Set string to no pdrv_device struct...badness
         */
        err_ent[ent].ent_type = ENT_STRING;
        err_ent[ent].ent_data = NoPdrv_struct;
        err_ent[ent].ent_size = strlen(NoPdrv_struct) + 1;
    	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
        /*
         * set to next sequential entry
         */
        ent++;
    
        /* 
         * If there is a ccb
         */
        if( ccb != NULL){
    
    	    /*
    	     * String Active CCB 
    	     */
            err_ent[ent].ent_type = ENT_STRING;
            err_ent[ent].ent_data = ActiveCCB;
            err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	    err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
            /*
             * set to next sequential entry
             */
            ent++;
    
    	    /* DUMP the CCB..... function codes map entry codes....*/
            err_ent[ent].ent_type = (U32)ccb->cam_func_code;
            err_ent[ent].ent_data = (u_char *)ccb;
            err_ent[ent].ent_size = (U32)ccb->cam_ccb_len;
    	    err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
    	    /*
    	     * Assign the cam version number
    	     */
    	    err_ent[ent].ent_vers = CAM_VERSION;
    
            /*
             * set to next sequential entry
             */
            ent++;
    
            /*
             * Now lets see if there is sense data.....
             */
            if( ccb->cam_func_code == XPT_SCSI_IO ){
    		/*
    		 * well it's a I/O request.... see if 
    		 * sense data is valid....
    		 */
    		if((((CCB_SCSIIO *)ccb)->cam_ch.cam_flags & 
			CAM_AUTOSNS_VALID) != NULL){
    
    	    	    /*
    	    	     * Dump the sense buffer now.
    	    	     */
        	    err_ent[ent].ent_type = ENT_SENSE_DATA;
        	    err_ent[ent].ent_data = ((CCB_SCSIIO *)ccb)->cam_sense_ptr;
        	    err_ent[ent].ent_size = 
				(U32)((CCB_SCSIIO *)ccb)->cam_sense_len;
    	    	    err_ent[ent].ent_pri = PRI_FULL_REPORT; 
    	   	    /*
    	    	     * Sense data does not have a version number.....
        	     * set to next sequential entry
        	     */
        	    ent++;
    		}
            }
	}
        /*
         * Now set the number of entries and send it down...
         * to the cam_logger().
         */
        err_hdr.hdr_entries = ent;

    	cam_logger( &err_hdr, DEV_BUS_ID(dev), DEV_TARGET(dev),
			DEV_LUN(dev));
	return;

    } /* End of if no pdrv_device structure */

    /*
     * Now Check if limits are reached for this open
     * Must change once we define how to set limits....
     * We don't check the limits first because of check
     * to see if pdrv_struct is there....
     */
    if(((flags & CAM_HARDERR) != NULL) && (pdrv_dev->pd_hard_err > 
		cam_harderr_limit)){
	pdrv_dev->pd_hard_err++;
	return;
    }
    if(((flags & CAM_SOFTERR) != NULL) && (pdrv_dev->pd_soft_err > 
		cam_softerr_limit)){
	pdrv_dev->pd_soft_err++;
	return;
    }
    if(((flags & CAM_INFORMATIONAL) != NULL) && (pdrv_dev->pd_soft_err > 
		cam_softerr_limit)){
	pdrv_dev->pd_soft_err++;
	return;
    }

    inq = (ALL_INQ_DATA *)pdrv_dev->pd_dev_inq;

    
    /*
     * Since both the class and subsytem follows the scsi
     * device type just copy them over..
     */
    err_hdr.hdr_class = inq->dtype;
    err_hdr.hdr_subsystem = inq->dtype;


    /* 
     * Get the priority of this entry and increment error count
     */
    if(( flags & CAM_SOFTWARE ) != NULL){
	err_hdr.hdr_pri = CAM_ERR_SEVERE;
    }
    else if(( flags & CAM_HARDERR ) != NULL){
    	err_hdr.hdr_pri = CAM_ERR_HIGH;
	pdrv_dev->pd_hard_err++;
    }
    else if(( flags & CAM_SOFTERR ) != NULL){
    	err_hdr.hdr_pri = CAM_ERR_LOW;
	pdrv_dev->pd_soft_err++;
    }
    else {
	err_hdr.hdr_pri = CAM_ERR_LOW;
    }


    /*
     *  Fill in rest of entry list.....
     */

    
    /*
     * Get our device name...........  
     */ 
    err_ent[ent].ent_type = ENT_STR_DEV; 
    err_ent[ent].ent_data = pdrv_dev->pd_dev_desc->dd_pv_name; 
    err_ent[ent].ent_size = strlen(pdrv_dev->pd_dev_desc->dd_pv_name) + 1; 
    err_ent[ent].ent_pri = PRI_BRIEF_REPORT; /* strings are brief */
    /*
     * set to next sequential entry 
     */ 
    ent++;


    /* 
     * If there is a ccb
     */
    if( ccb != NULL){

	/*
	 * String Active CCB 
	 */
    	err_ent[ent].ent_type = ENT_STRING;
    	err_ent[ent].ent_data = ActiveCCB;
    	err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; 
        /*
         * set to next sequential entry
         */
        ent++;

	/*
	 * String Active CCB Status
	 */
    	err_ent[ent].ent_type = ENT_STRING;
    	err_ent[ent].ent_data = (u_char *)cdbg_CamStatus(ccb->cam_status, 
						CDBG_FULL);
    	err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; 
        /*
         * set to next sequential entry
         */
        ent++;

	/* DUMP the CCB..... function codes map entry codes....*/
    	err_ent[ent].ent_type = (U32)ccb->cam_func_code;
    	err_ent[ent].ent_data = (u_char *)ccb;
    	err_ent[ent].ent_size = (U32)ccb->cam_ccb_len;
	err_ent[ent].ent_pri = PRI_FULL_REPORT; 
	/*
	 * Assign the cam version number
	 */
	err_ent[ent].ent_vers = CAM_VERSION;

    	/*
    	 * set to next sequential entry
    	 */
    	ent++;

        /*
         * Now lets see if there is sense data.....
         */
        if( ccb->cam_func_code == XPT_SCSI_IO ){
	    /*
	     * well it's a I/O request.... see if 
	     * sense data is valid....
	     */
	    if(( ((CCB_SCSIIO *)ccb)->cam_ch.cam_status & CAM_AUTOSNS_VALID) !=
			    NULL){
		/*
		 * GET the scsi status string
		 */
    	        err_ent[ent].ent_type = ENT_STRING;
    	        err_ent[ent].ent_data = (u_char *)cdbg_ScsiStatus(
			((CCB_SCSIIO *)ccb)->cam_scsi_status, CDBG_FULL);
    	        err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	    	err_ent[ent].ent_pri = PRI_BRIEF_REPORT; 
	        /*
	         * Strings do not have a version number.....
    	         * set to next sequential entry
    	         */
    	        ent++;

		/*
		 * If check condition get sense key string
		 */
		if(((CCB_SCSIIO *)ccb)->cam_scsi_status == 
				SCSI_STAT_CHECK_CONDITION){


    	            err_ent[ent].ent_type = ENT_STRING;
    	            err_ent[ent].ent_data = 
			(u_char *)cdbg_SenseKeyTable[((ALL_REQ_SNS_DATA *)
			((CCB_SCSIIO *)ccb)->cam_sense_ptr)->sns_key];
    	            err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	    	    err_ent[ent].ent_pri = PRI_BRIEF_REPORT; 
	            /*
	             * Strings do not have a version number.....
    	             * set to next sequential entry
    	             */
    	            ent++;
		}

	        /*
	         * Dump the sense buffer now.
	         */
    	        err_ent[ent].ent_type = ENT_SENSE_DATA;
    	        err_ent[ent].ent_data = ((CCB_SCSIIO *)ccb)->cam_sense_ptr;
    	        err_ent[ent].ent_size = 
			(U32)((CCB_SCSIIO *)ccb)->cam_sense_len;
    	    	err_ent[ent].ent_pri = PRI_FULL_REPORT; 
	        /*
	         * Sense data does not have a version number.....
    	         * set to next sequential entry
    	         */
    	        ent++;
	    }
        }
    }
    /*
     * Now check the flags to see if we need to the dump
     * the pdrv_device structs and the specific structs.
     * and all the queues
     */
    if((( flags & CAM_DUMP_ALL ) != NULL) || (( flags & CAM_SOFTWARE ) !=
		NULL)){
        /*
         * PDRV_DEVICE struct
         */
        err_ent[ent].ent_type = ENT_PDRV_DEVICE;
        err_ent[ent].ent_data = (u_char *)pdrv_dev;
        err_ent[ent].ent_size = sizeof(PDRV_DEVICE);
    	err_ent[ent].ent_pri = PRI_FULL_REPORT; 
	err_ent[ent].ent_vers = PDRV_DEVICE_VERS;
        /*
         * set to next sequential entry
         */
        ent++;

	/*
	 * The device specific struct 
	 */
	if( (inq->dtype == ALL_DTYPE_DIRECT) || (inq->dtype == 
		ALL_DTYPE_RODIRECT)) {
	    /*
	     * DISK od CDROM
	     */
    	    err_ent[ent].ent_type = ENT_DISK_SPECIFIC;
	    err_ent[ent].ent_vers = DISK_SPECIFIC_VERS;
    	}
	else if(inq->dtype == ALL_DTYPE_SEQUENTIAL) {
	    err_ent[ent].ent_type = ENT_TAPE_SPECIFIC;
	    err_ent[ent].ent_vers = TAPE_SPECIFIC_VERS;
        }
        else {
	    err_ent[ent].ent_type = ENT_STRUCT_UNKNOWN;;
	}

    	err_ent[ent].ent_data = (u_char *)pdrv_dev->pd_specific;
    	err_ent[ent].ent_size = (U32)pdrv_dev->pd_spec_size;
    	err_ent[ent].ent_pri = PRI_FULL_REPORT; 
    	/*
     	 * set to next sequential entry
    	 */
	ent++;


    }
    /*
     * Now set the number of entries and send it down...
     * to the cam_logger().
     */
    err_hdr.hdr_entries = ent;
    
    cam_logger( &err_hdr, DEV_BUS_ID(dev), DEV_TARGET(dev),
		DEV_LUN(dev));


    /* 
     * Now dump the active queues
     */

    err_hdr.hdr_entries = 0;

    first_time = 1;
     

    /*
     * If statement reads as follows
     * If either DUMP_ALL or SOFTWARE flags set and there are ccbs on the
     * active list
     */
    if(((( flags & CAM_DUMP_ALL ) != NULL) || (( flags & CAM_SOFTWARE ) !=
		NULL)) && (pdrv_dev->pd_active_ccb != NULL) ){
	/*
	 * Get pointer to first thing on active list if any
	 */
	pdrv_ws = pdrv_dev->pd_active_list.flink;

	for( ent = 0; ; ent++) {
	    /*
	     * Check if anything is on list.....
	     */
	    if( (ent == 0) && (pdrv_ws == 
			(PDRV_WS *)pdrv_dev)) {
		break;
	    }
	    /*
	     * Check if we have to send down
	     */
	    if( (ent >= CCMN_MAX_ENTRIES ) || (pdrv_ws ==
			(PDRV_WS *)pdrv_dev)) {
		/* 
		 * Assign number of entries and send
		 */
		err_hdr.hdr_entries = ent;
    		cam_logger( &err_hdr, DEV_BUS_ID(dev), DEV_TARGET(dev),
				DEV_LUN(dev));
		/*
		 * Now if not at end of list continue
		 * the for loop
		 */
		if(pdrv_ws == (PDRV_WS *)pdrv_dev) {
		    break;
		}
		else {
		    ent = 0; /* set back to begining */
		    /*
		     * set pointer to next in list
		     */
	    	    pdrv_ws = pdrv_ws->pws_flink;
		    first_time++; /* so we don't get same message */
		    err_hdr.hdr_entries = 0;
		    continue;
		}
	    }

	    if( ent == 0 ){
    		/* 
     		 * Init the entry list
    		 */
    		for( i = 0; i < CCMN_MAX_ENTRIES; i++){
		    bzero( &err_ent[i], sizeof(CAM_ERR_ENTRY));
    		}
		/* 
		 * First time around ??
		 */
		if( first_time == 1) { 
    		    err_ent[ent].ent_type = ENT_STRING;
    		    err_ent[ent].ent_data = Active_List;
    		    err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	    	    err_ent[ent].ent_pri = PRI_FULL_REPORT; 
		}
		else { 
    		    err_ent[ent].ent_type = ENT_STRING;
    		    err_ent[ent].ent_data = Active_List_Cont;
    		    err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	    	    err_ent[ent].ent_pri = PRI_FULL_REPORT; 
		}
	    }
		
	    /*
	     * Get the ccb and and set the entry up
	     */

    	    err_ent[ent].ent_type = 
			(U32)pdrv_ws->pws_ccb->cam_ch.cam_func_code;

    	    err_ent[ent].ent_data = (u_char *)pdrv_ws->pws_ccb;

    	    err_ent[ent].ent_size = 
			(U32)pdrv_ws->pws_ccb->cam_ch.cam_ccb_len;

	    err_ent[ent].ent_vers = CAM_VERSION;
    	    err_ent[ent].ent_pri = PRI_FULL_REPORT; 


	    /*
	     * Now assign the next wroking set on the que
	     */
	    pdrv_ws = pdrv_ws->pws_flink;

	} /* end of for */
    } /* end of if DUMP_ALL or SOFTWARE */

    /* 
     * Now Check if anything is on the pending list....
     */
    /*
     * If statement reads as follows
     * If either DUMP_ALL or SOFTWARE flags set and there are ccbs on the
     * pending list
     */
    if(((( flags & CAM_DUMP_ALL ) != NULL) || (( flags & CAM_SOFTWARE ) !=
		NULL)) && (pdrv_dev->pd_pend_ccb != NULL) ){
	/*
	 * Get pointer to first thing on active list if any
	 */
	pdrv_ws = pdrv_dev->pd_pend_list.flink;

	for( ent = 0; ; ent++) {
	    /*
	     * Check if anything is on list.....
	     */
	    if( (ent == 0) && (pdrv_ws == 
			(PDRV_WS *)pdrv_dev)) {
		break;
	    }
	    /*
	     * Check if we have to send down
	     */
	    if( (ent >= CCMN_MAX_ENTRIES ) || (pdrv_ws ==
			(PDRV_WS *)pdrv_dev)) {
		/* 
		 * Assign number of entries and send
		 */
		err_hdr.hdr_entries = ent;
    		cam_logger( &err_hdr, DEV_BUS_ID(dev), DEV_TARGET(dev),
				DEV_LUN(dev));
		/*
		 * Now if not at end of list continue
		 * the for loop
		 */
		if(pdrv_ws == (PDRV_WS *)pdrv_dev) {
		    break;
		}
		else {
		    ent = 0; /* set back to begining */
		    /*
		     * set pointer to next in list
		     */
	    	    pdrv_ws = pdrv_ws->pws_flink;
		    first_time++; /* so we don't get same message */
		    err_hdr.hdr_entries = 0;
		    continue;
		}
	    }

	    if( ent == 0 ){
    		/* 
     		 * Init the entry list
    		 */
    		for( i = 0; i < CCMN_MAX_ENTRIES; i++){
		    bzero( &err_ent[i], sizeof(CAM_ERR_ENTRY));
    		}
		/* 
		 * First time around ??
		 */
		if( first_time == 1) { 
    		    err_ent[ent].ent_type = ENT_STRING;
    		    err_ent[ent].ent_data = Pend_List;
    		    err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	    	    err_ent[ent].ent_pri = PRI_FULL_REPORT; 
		}
		else { 
    		    err_ent[ent].ent_type = ENT_STRING;
    		    err_ent[ent].ent_data = Pend_List_Cont;
    		    err_ent[ent].ent_size = strlen(err_ent[ent].ent_data) + 1;
    	    	    err_ent[ent].ent_pri = PRI_FULL_REPORT; 
		}
	    }
		
	    /*
	     * Get the ccb and and set the entry up
	     */

    	    err_ent[ent].ent_type = 
			(U32)pdrv_ws->pws_ccb->cam_ch.cam_func_code;

    	    err_ent[ent].ent_data = (u_char *)pdrv_ws->pws_ccb;
    	    err_ent[ent].ent_size = 
			(U32)pdrv_ws->pws_ccb->cam_ch.cam_ccb_len;

	    err_ent[ent].ent_vers = CAM_VERSION;
    	    err_ent[ent].ent_pri = PRI_FULL_REPORT; 


	    /*
	     * Now assign the next wroking set on the que
	     */
	    pdrv_ws = pdrv_ws->pws_flink;

	} /* end of for */
    } /* end of if DUMP_ALL or SOFTWARE  for pending list */


} /* end of ccmn_errlog */

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_check_idle()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will check that a device does not have any opens.
 *	This function is used for unloading CAM peripheral drivers.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This function must be called with the pdrv_unit_table SMP locked.
 *
 ************************************************************************/

ccmn_check_idle(start_unit, num_units, cmajor, bmajor, spec_size)
U32 start_unit;
U32 num_units;
dev_t cmajor;
dev_t bmajor;
U32 spec_size;
{
	PDRV_DEVICE *pd;
	int i;

	/*
	 * Make sure we do not go beyond the size of the unit table.
	 */
	if( (start_unit + num_units) > MAX_UNITS)
		return(-1);

	for(i=start_unit; i<start_unit+num_units; i++)   {
		/*
		 * Check whether a device was found at this address and
		 * whether the major numbers match.
		 */
		if( (pd = pdrv_unit_table[i].pu_device) ) {
		    if( (major(pd->pd_dev) == cmajor) ||
		        (major(pd->pd_dev) == bmajor) ) {
			/*
			 * Check whether the driver thinks this device
			 * is open.
			 */
		        if(pdrv_unit_table[i].pu_opens)  {
				return(-1);
			}
		    }
		}
	}
	/* 
	 * Now that we know no devices owned by the driver to be
	 * unloaded are opened, we need to make a second pass and release
	 * the resources allocated by the driver.
	 */
	for(i=start_unit; i<start_unit+num_units; i++)   {
		/*
		 * Check whether a device was found at this address and
		 * whether the major numbers match.
		 */
		if( (pd = pdrv_unit_table[i].pu_device) ) {
		    if( (major(pd->pd_dev) == cmajor) ||
		        (major(pd->pd_dev) == bmajor) ) {
			/*
			 * Release the device specific structure, sense 
			 * data area and the peripheral device structure.
			 */
		    	if(pd->pd_specific)
		    	    ccmn_rel_dbuf(pd->pd_specific, spec_size);
			if(pd->pd_sense_ptr)
		    	    ccmn_rel_dbuf(pd->pd_sense_ptr, pd->pd_sense_len);
		    	ccmn_rel_dbuf(pd , sizeof(PDRV_DEVICE));
		    	pdrv_unit_table[i].pu_device = (PDRV_DEVICE *)NULL;
		    }
		}
	}
	return(0);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_find_ctlr()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will find the controller structure that corresponds 
 *	to the SCSI controller that the device must be attached to. 
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This function must be called with the pdrv_unit_table SMP locked.
 *
 ************************************************************************/


struct controller *
ccmn_find_ctrl(dev)
dev_t dev;
{
   char done = 0;
   struct device *device;
   struct controller *ctlr;
   struct bus *bus;
   extern struct bus *system_bus;
   extern char cam_ctlr_string[];

   /*
    * We must find the controller for the device but first we must find
    * the bus structure.  We start with the system bus.
    */
    bus = ldbl_find_bus(system_bus, LDBL_SEARCH_ALL, LDBL_WILDNAME, 
	LDBL_WILDNUM);


    while( (bus != NULL) && (!done) )  {

    	ctlr = ldbl_find_ctlr(bus->ctlr_list, LDBL_SEARCH_ALL,
    	      LDBL_WILDNAME, DEV_BUS_ID(dev), LDBL_WILDNUM);

        while( (ctlr != NULL) && (!done) )  {
	    /*
	     * If it's a SCSI/CAM controller there will be special data
	     * in the private area of the controller structure.
	     */
     	    if( (ctlr->private[0] != NULL) && 
     	        strcmp(ctlr->private[0], cam_ctlr_string) == 0)  {
		    if(ctlr->ctlr_num == DEV_BUS_ID(dev))
                	done = 1;	/* found it */
            } else {
		/* keep searching */
    		ctlr = ldbl_find_ctlr(ctlr, LDBL_SEARCH_NOINCLUDE,
	    	      LDBL_WILDNAME, DEV_BUS_ID(dev), LDBL_WILDNUM);
            }
	}

        if(!done)
    	    bus = ldbl_find_bus(bus, LDBL_SEARCH_NOINCLUDE, 
		LDBL_WILDNAME, LDBL_WILDNUM);
    }

    return(ctlr);
}
/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_find_device()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will find the device structure that corresponds 
 *	to the SCSI device  for this controller.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This function must be called with the pdrv_unit_table SMP locked.
 *
 ************************************************************************/


struct device *
ccmn_find_device(ctrl, dev)

struct controller *ctrl;
dev_t dev;

{
   char done = 0;
   struct device *device;
   int unit = DEV_UNIT(dev);
   int log_unit = (DEV_BUS_ID(dev) << 3) + DEV_TARGET(dev);


   /*
    * We must walk the device list for this controller
    * to find our device if one is there.
    */
    device = ctrl->dev_list;

    while(( device != (struct device *)NULL) && (done == 0) )  {
	if((device->unit == unit ) && (device->logunit == log_unit)) {
	    done++;
	}
	else {
	    device = device->nxt_dev;
	}
    }
    if( done != 0 ){
	return( device );
    }
    else {
	return( (struct device *)NULL);
    }
	
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_attach_device()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This function will find the controller structure for a device,
 *	fill in the device structure, and attach it to the controller structure.
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

ccmn_attach_device(dev, dev_type, dev_name)
dev_t dev;
caddr_t dev_type;
caddr_t dev_name;
{
   PDRV_DEVICE *pd;
   struct controller *ctlr;
   struct device *device;
   static u_char module[] = "ccmn_attach_device";

   if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
	ccmn_errlog(module, "No peripheral device structure\n",
		CAM_SOFTWARE, NULL, dev, (u_char *)NULL);
	return(-1);
   }
   if( (ctlr = ccmn_find_ctrl(dev)) == (struct controller *)NULL) {
	ccmn_errlog(module, "Can't find controller structure\n",
		CAM_SOFTWARE, NULL, pd->pd_dev, (u_char *)NULL);
	return(-1);
   }
   /*
    * Allocate a device structure and some space to hold strings.
    */
   device = (struct device *)ccmn_get_dbuf(sizeof(struct device));
   if( device == (struct device *)NULL)   {
	ccmn_errlog(module, "Can't alloc device structure\n",
		CAM_SOFTWARE, NULL, pd->pd_dev, (u_char *)NULL);
	return(-1);
   }
   /* 
    * Fill in device structure.
    */
   device->ctlr_hd = ctlr;
   device->dev_name = dev_name;
   device->dev_type = dev_type;
   device->logunit =  0;/*UNIT(dev); /* pd->pd_logunit */
   device->unit = 0;/*UNIT(dev); /* ?? */
   device->ctlr_name = ctlr->ctlr_name;
   device->ctlr_num = ctlr->ctlr_num;
   device->alive = ALV_ALIVE;
   /*
    * Attach device structure to controller structure.
    */
   conn_device(ctlr, device);
   return(0);
}

/************************************************************************
 *
 *  ROUTINE NAME:  ccmn_disperse_que()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will spread the queue depth (passed as a parameter)
 *	across all LUNS connected (open) on the target.
 *
 *  FORMAL PARAMETERS:
 *	dev - major/minor number 
 *	que_depth - the queue depth to  be dispersed among all
 *		    LUNs connected to the target.
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
 *
 *  ADDITIONAL INFORMATION:
 *	This function must be called with the pdrv_unit_table SMP locked.
 *
 ************************************************************************/
 void
 ccmn_disperse_que(dev, que_depth)
 dev_t dev;
 U32 que_depth;
 {
	PDRV_DEVICE	*pd;
	PDRV_UNIT_ELEM  *pdu, *tmp_pdu;
	U32 unit;
	U32 i, lun_count;
	U32 que_per_lun, remainder_que;

	/*
	 * Must convert dev into a unit number that reflects lun 0
	 */
	unit = DEV_UNIT(dev);
	unit &= ~LUN_MASK;
	/*
	 * Get the element ptr
	 */
	pdu = &pdrv_unit_table[unit];
	/*
	 * Make a temp copy of the base pdu cause we'll be
	 * incrementing the pointer for each possible LUN
	 */
	tmp_pdu = pdu;
	
	lun_count = 0;
	que_per_lun = 0;
	remainder_que = 0;
	
	/*
	 * Determine how many LUNs are present by checking the open
	 * count in the PDRV_UNIT_TABLE
	 */
	for (i = 0; i < NLPT; i++, tmp_pdu++) {
		if (tmp_pdu->pu_opens) {
			lun_count++;
		}
	}
	/*
	 * Calculate the queue depth per lun based on the number of
	 * LUNs found on target.  Also get the remainder, if any,
	 * to disperse.
	 */
	if( lun_count){ 
		que_per_lun = que_depth / lun_count;
		remainder_que = que_depth % lun_count;
	}
	else {
		return;
	}
	/*
	 * Reset the PDRV_UNIT_ELEM pointer to LUN 0
	 */
	tmp_pdu = pdu;
	/*
	 * Now, go back thru the LUNs and assign the que_per_lun value.
	 * If there is any remainder_que, distribute 1 to each LUN until
	 * it's gone.
	 */
	for (i = 0; i < NLPT; i++, tmp_pdu++) {
		if (tmp_pdu->pu_opens) {
			pd = tmp_pdu->pu_device;
			if (que_per_lun != 0) {
				pd->pd_que_depth = que_per_lun;
			}
			if (remainder_que != 0) {
				pd->pd_que_depth += 1;
				remainder_que -= 1;
			}
		}
	}
 }
