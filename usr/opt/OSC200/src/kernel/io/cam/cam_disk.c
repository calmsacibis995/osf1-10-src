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
static char *rcsid = "@(#)$RCSfile: cam_disk.c,v $ $Revision: 1.1.52.20 $ (DEC) $Date: 1994/01/03 17:46:05 $";
#endif

#define CAMERRLOG

/************************************************************************
 *
 *  cam_disk.c		Version 1.00 
 *
 *  This file contains the CAM disk driver routines.  This driver will
 *  support both Direct Access and Read Only Direct Access devices
 *  (CD-ROM).  This driver also contains the RZ wrapper kernel entry
 *  points which allow access to the CAM driver using the BSD naming scheme.
 *
 *  MODIFICATION HISTORY:
 *
 *  VERSION  DATE	WHO	REASON
 *
 *  1.00     01/26/91	maria	Created from CAM Disk Driver Func. Spec.
 *
 *  1.01     05/29/91	maria	Added some fixes for BBR.
 *
 *  1.02     06/04/91	maria	Added call to release the CCB for the
 *				Set Asynch callback CCB in cdisk_open.
 *
 *  1.03     06/07/91	maria	Added the rz wrapper routines to allow
 *				access to devices using the old dev_t.
 *				Reversed the functionality of cdisk_slave
 *				and cdisk_attach.
 *				Fixed cdisk_strategy to add in the starting
 *				block number of a partition to the
 *				requested block when building the cdb.
 *
 *  1.04     06/14/91	maria	Fixed bug in cdisk_ioctl to set slave_num
 *				to the target id - used when dumping.
 *				Changed printf to cprintf in attach.
 *
 *  1.05     06/17/91	maria	Removed underscore in driver entry point
 *				routine names.
 *				Fixed dev_t mapping macro to allow
 *				access to other controllers.
 *
 *  1.06     06/19/91	maria	Changed location of cam_disk.h.
 *				Added call to ccmn_minphys().
 *				Cleaned up cdiskslave().
 *				Fixed bug in rzioctl when indexing
 *				camdinfo.
 *
 *  1.07     06/25/91	maria	Added check in cdisk_open for the
 *				B_OLD_DEV bit set in flag from rzopen() 
 *				wrapper.  The rsblk routine must be called 
 *				with the correct dev number.
 *				Added calls to the cdrom driver in the
 *				rz wrapper routines.
 *				Added call to ccmn_DoSpecialCmd() to
 *				handle SCSI cmds (for backwards
 *				compatability) in cdisk_ioctl().
 *				Moved rz wrapper mapping macro to pdrv.h
 *
 *  1.08     06/25/91	maria	Added save of sense data in
 *				cdisk_check_sense().
 *				Allow start unit to be issued only twice
 *				in cdisk_op_spin().
 *
 *  1.09     07/26/91	maria	Made change of REASSIGN_OP to reflect
 *				change made in scsi_direct.h.
 *
 *  1.10     07/29/91	maria	Major modifications for FT2.
 *				Added BBR and recovery support.
 *				Added rescan of an address.
 *				Added CD-ROM support.
 *				Changed RZ wrapper scheme.
 *
 *  1.11     07/29/91	maria	Removed peripheral device structure
 *				pointer in calls to ccmn_ccbait().
 *
 *  1.12     08/13/91	maria	Fixed bugs involving multiple reset recovery.
 *				Recovery is started on removables on 
 *				power up and resets.
 *				Added special code for tagged devices
 *				in cdisk_op_spin.
 *				
 *  1.13     09/02/91	maria	Reworked the recovery scheme to handle 
 *				multiple resets during recovery.
 *				Added special code for tagged devices
 *				in cdisk_op_spin and recovery.
 *				Added error logging.
 *				Added prevent/allow media removal capability.
 *				Fixed bug in recovery superblock read cdb.
 *				Added OSF specific code.
 *				Added checks for sufficient sense data.
 *
 *  1.14     09/05/91	maria	Moved setting of PD_ONLINE to end of
 *				cdisk_open().
 *				Added prevent media removal cmd to
 *				recovery code.
 *				Fixed initiate sync bug in recovery code.
 *				Lint fixes and cleanup.
 *
 *  1.15     09/11/91   maria	Added timeout for scsi status of busy
 *				in cdisk_complete().
 *				Check for SZ_NOSYNC instead of
 *				SZ_TRY_SYNC.
 *
 *  1.16     09/16/91   maria	Changed the tag action for tagged write
 *				requests to be ordered.
 *				Added timeout in recovery code for start
 *				unit command.
 *				Added start of recovery for tagged
 *				devices on power cycle.
 *
 *  1.17     09/19/91   maria	Fixed bugs in recovery code in issuing 
 *				mode select data (cdisk_rec_tur_done &
 *				cdisk_rec_rel_ccb).
 *
 *  1.18     10/02/91   maria	Fixed bug in cdisk_ioctl when string comparing
 *				to "RX33" and cdisk_bbr in checking one
 *				ASQC.
 *
 *  1.20     10/02/91   maria	Fixed bug in cdisk_ioctl for the
 *				DEVGETGEOM command for floppies.
 *
 *  1.21     10/24/91   maria	Modifications for SSB:
 *				Split up recovery routines.
 *				Added check for SZ_NO_TAG.
 *				Call error logger for prevent/allow failure.
 *				Added check for correct number of bytes
 *				in mode sense pages accessed in cdisk_ioctl
 *				and request correct number of bytes to include
 *				mode header and descriptor.
 *				BBR code cleaned up.
 *				Increased CD-ROM comamnd timeouts. 
 *
 *  1.22     10/28/91   maria	Bug fix in cdisk_complete for CAT_DEVICE_ERROR
 *				to retry the request if not tagged.
 *				Bug fix in BBR code to handle requests
 *				which are not retryable.
 *
 *  1.23     10/29/91   maria	Unlock peripheral device structure after
 *				calling ccmn_abort_que.
 *
 *  1.24     11/19/91   maria	Save sense data in peripheral device
 *				struct on open.
 *				Moved defines to cam_disk.h.
 *				Added lock before calling CAM_ERROR().
 *
 *  1.25     11/20/91   janet   Changed SIM_DEFAULT_TIMEOUT to CAM_TIME_DEFAULT
 *
 *  1.26     11/22/91   maria	Fixed bug in cdisk_strategy in determining
 *				when to use a 10 byte cdb.
 *
 *  1.27     11/29/91   maria	Fixed cdisk_open() to not set PD_NO_DEVICE if 
 *				device responds with NOT READY. (SoftPC fix).
 *
 *  1.28     12/04/91   maria	Bug fix to release the prevent removal
 *				ccb in the recovery completion code.
 *
 *  1.29     12/14/91   maria	Clear ccb before retry in cdisk_reassign.
 *				Add error logs for set async callback failure 
 *				and invalid block size after read capacity in 
 *				cdisk_open and in cdisk_minphys.
 *				Increase timeout for Start Unit cmd.
 *				Add locks around incrementing hard error
 *				counts in cdisk_check_sense.
 *				Retry all ccbs in recovery path.
 *				Allow 3 retries on start unit.
 *				Fixed bug in cdisk_open to call ccmn_open_unit()
 *				with both device types instead of inquiry data.
 *
 *  1.30     12/14/91   maria	Bug fix - removed early assignment in 
 *				cdisk_attach().
 *  1.31     12/18/91   maria	Bug fix in cdisk_op_find_device() for
 *				opening a device which was not configured.
 *
 *  1.32     12/19/91   maria	Set dkn to -1 for devices found after boot
 *				and increased timeout for reassign command.
 *
 *  1.33     12/20/91   maria	Check whether removable before logging error
 *				for start unit failure and increase command
 *				timeouts for all removables.
 *
 ************************************************************************/

/************************* Include Files ********************************/

#include <io/common/iotypes.h>
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <labels.h>
#include <sys/poll.h>
#include <sys/syslog.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/buf.h>
#include <io/common/devio.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/disklabel.h>
#include <sys/sysinfo.h>
#include <io/cam/rzdisk.h>
#include <sys/user.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#include <io/cam/cam_logger.h>
#include <io/cam/pdrv.h>
#include <hal/cpuconf.h>
#include <ufs/fs.h>
#include <io/common/pt.h>
#include <io/common/devdriver.h>
#include <io/cam/ccfg.h>
#include <io/cam/cam_disk.h>
#include <io/cam/cam_errlog.h>
#include <io/cam/cam_debug.h>
#include <io/cam/dme.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim.h>
#include <io/common/srvc.h>
#include <io/cam/cam_nstd_raid.h>
#include <io/dec/tc/tc.h>

/************************* Local Defines ********************************/

#define DEV_TYPE_SZ 	8
#define DEV_NAME_SZ 	8
/*
 * The following defines are used in the callback completion routine
 * to indicate the action to be taken for the completing ccb.
 */
#define CD_IO_CONT	0	/* Leave request on queue and return */
#define CD_IO_DONE	1	/* Request completed sucessfully */
#define CD_IO_FAILURE	2	/* Request completed with error */
#define CD_IO_RETRY 	3	/* Request must be retried */

#define LUN_ZERO	0
#define HSZ10_PAR_CHK   0x40	/* vendor unique for control byte */
#define CDISK_RETRY_LIMIT 5	/* Number of times we retry the I/O */
#define CDISK_REC_RETRY 15	/* Number of timesfor recovery retry */

extern u_long cdisk_to_def;
extern u_long cdisk_io_def;
extern u_long cdisk_io_rmb;

#define CDISK_TO_DEF	cdisk_to_def
#define CDISK_IO_DEF	cdisk_io_def
#define CDISK_IO_RMB	cdisk_io_rmb
				

#define CD_ALLOW_REMOVAL	0
#define CD_PREVENT_REMOVAL	1

#define CD_MODE_HEADER_DESC					\
	(U32)(sizeof(ALL_MODE_HEAD6) + sizeof(ALL_MODE_DESC))

/*
 * The following defines are used in cdisk_ioctl in determining that the
 * correct number of bytes are contained in the mode sense data for each of
 * the pages accessed.
 */
#define CD_MIN_GEOM_DATA	 5	/* Minimum bytes required for  */
					/* drive geometry page */
#define CD_MIN_FLEXI_DATA	10	/* Minimum bytes requireed for */
					/* flexible disk page */
#define CD_MIN_FORMAT_DATA	12	/* Minimum bytes required for */
					/* the format device page */

#define CD_NEEDED_SENSE_BYTES	15	/* Minimum number of sense */
					/* bytes needed to determine */
					/* the reason for Check Cond */
/*
 * Check whether we have the minimum required sense data bytes.
 */
#define CHECK_SENSE_RESID(ccb)					\
( ((ccb)->cam_sense_len - (ccb)->cam_sense_resid) 		\
	< CD_NEEDED_SENSE_BYTES )				\


/*
 * Check whether the device is a SCSI-2 device to determine  
 * if it supports tagged queueing and set the defaults.
 * Check whether the device does not allow disconnects and
 * set as default.
 */
#define SET_PD_DEFAULTS(pd)						\
	if ( (((ALL_INQ_DATA *)pd->pd_dev_inq)->ansi == ALL_RDF_SCSI2)	\
	   && (((ALL_INQ_DATA *)pd->pd_dev_inq)->cmdque) 		\
	   && (!(pd->pd_dev_desc->dd_flags & SZ_NO_TAG)) ) {		\
		CCB_PATHINQ *ccb_inq;					\
		if ( ccb_inq = ccmn_pinq_ccb_bld(dev, 0x0) ) {		\
		    if ( ccb_inq->cam_hba_inquiry & PI_TAG_ABLE) {	\
			pd->pd_cam_flags =  CAM_QUEUE_ENABLE |		\
						CAM_SIM_QFRZDIS;	\
			pd->pd_tag_action = CAM_SIMPLE_QTAG;		\
		    }							\
		    ccmn_rel_ccb((CCB_HEADER *)ccb_inq);		\
		}							\
	}								\
	if (pd->pd_dev_desc->dd_flags & SZ_NO_DISC)			\
		pd->pd_cam_flags |= CAM_DIS_DISCONNECT;			
/*
 * Macro which determines the timeout for a ccb based on whether it is
 * a CDROM device or direct access.
 */
#define GET_IO_TIMEOUT(pd)  				\
	(((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb == 1 	\
	? CDISK_IO_RMB : CDISK_IO_DEF)

#define GET_TIMEOUT(pd)					\
	((pd)->pd_tag_action != CAM_SIMPLE_QTAG		\
	? CDISK_TO_DEF : (CDISK_TO_DEF * 2))

/*
 * Handles an error return from the cdisk_strategy routine.
 */
#define IO_STRATEGY_ERR(bp, err)  {				\
	bp->b_flags |= B_ERROR;					\
	bp->b_resid = bp->b_bcount;				\
	bp->b_error = err;					\
	(void) biodone(bp);					\
}
	
/*
 * Fill in the appropriate fields in the CCB for the Mode Select command.
 */
#define CCB_MODE_SEL(pd, ccb, msep)  {				\
	ALL_MODE_SEL_CDB6 *cdb;					\
	cdb = (ALL_MODE_SEL_CDB6 *)(ccb)->cam_cdb_io.cam_cdb_bytes;\
	cdb->opcode = ALL_MODE_SEL6_OP;				\
	if ((msep)->ms_ent_sp_pf & MSEL_SAVE_PAGE)		\
		cdb->sp = 1;					\
	if ((msep)->ms_ent_sp_pf & MSEL_SCSI2)			\
		cdb->pf = 1;					\
	cdb->param_len = (msep)->ms_data_len;			\
	(ccb)->cam_cdb_len = sizeof(ALL_MODE_SEL_CDB6);		\
	(ccb)->cam_data_ptr = (msep)->ms_data;			\
	(ccb)->cam_dxfer_len = (msep)->ms_data_len;		\
}

/*
 * The following macro will issue a recovery ccb to the XPT.  
 * First it will check whether recovery must be restarted.
 */
#define ISSUE_REC_CCB(pd, ccb, retry, recov_state)  {		\
	U32 cam_status;						\
	int pri;						\
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, pri);			\
	CHK_RECOVERY_PENDING(pd, pri);				\
	pd->pd_flags |= recov_state;				\
	cam_status = ccmn_send_ccb(pd, (CCB_HEADER *)ccb, retry);\
	PDRV_IPLSMP_UNLOCK(pd, pri);				\
	/*							\
	 * Make sure the request was queued.			\
	 */							\
	if( cam_status != CAM_REQ_INPROG )			\
		ccb->cam_cbfcnp(ccb);				\
	else  {							\
		/*						\
		 * We know the queue is frozen here.		\
		 */						\
		RELEASE_QUEUE(pd);				\
	}							\
}								\

/*
 * This macro is called during the recovery process to determine
 * whether the recovery process needs to be restarted.  It will 
 * check the recovery pending bit, release the already allocated 
 * recovery ccbs and restart the recovery process by calling
 * cdisk_recovery.  This can occur when a reset occurs during
 * the recovery process.
 */
#define CHK_RECOVERY_PENDING(pd, saved_spl)  {			\
	if( ((pd)->pd_flags & PD_REC_PEND) )  {			\
		if((pd)->pd_flags & PD_REC_TIMEOUT)  {		\
			PDRV_IPLSMP_UNLOCK((pd), (saved_spl));	\
			return;					\
		}						\
		(pd)->pd_flags &= ~(PD_REC_START | PD_REC_PEND);\
		PDRV_IPLSMP_UNLOCK((pd), (saved_spl))		\
		/* 						\
		 * Remove all recovery CCBs from queue and 	\
		 * release them.				\
		 */						\
		cdisk_rec_rel_ccb((pd));			\
		cdisk_recovery((pd));				\
		return;						\
	}							\
}								\

#define LUN_ZERO        0

/*
 * The following are the CAM disk driver kernel entry points.
 */
int		cdisk_read();
int		cdisk_write();
int		cdisk_open();
int		cdisk_close();
int		cdisk_ioctl();
void 		cdisk_strategy();
daddr_t		cdisk_size();
void 		cdisk_async_cb();
void 		cdisk_complete();
/*
 * The following are the CAM disk driver internal routines.
 */
void 		cdisk_minphys();
static void 	cdisk_reset_hand();
static void 	cdisk_atn_hand();
static void 	cdisk_ccb_retry();
static void 	cdisk_recovery();
static void 	cdisk_rec_retry();
static void 	cdisk_rec_tur_done();
static void 	cdisk_rec_tur();
static void 	cdisk_rec_start();
static void 	cdisk_rec_start_done();
static void 	cdisk_rec_modesel();
static void 	cdisk_rec_modesel_done();
static void 	cdisk_rec_read_cap();
static void 	cdisk_rec_read_cap_done();
static void 	cdisk_rec_read();
static void 	cdisk_rec_read_done();
static void 	cdisk_rec_prevent();
static void 	cdisk_rec_error();
static void 	cdisk_reset_rec_err();
static void 	cdisk_rec_done();
static void 	cdisk_rec_rel_ccb();
static void 	cdisk_bbr_initiate();
static void 	cdisk_bbr();
static void 	cdisk_bbr_read();
static void 	cdisk_bbr_reassign();
static void 	cdisk_bbr_write();
static void 	cdisk_bbr_done();
static void 	cdisk_reassign();
static void 	cdisk_raid_complete();
static void	cdisk_raid_cmd_error();
static void     cdisk_close_dev();
static void     cdisk_raid_wrt_verify();
static void 	srvc_unlock();
CCB_SCSIIO 	*cdisk_build_raid_ccb();
static u_short 	cdisk_check_sense();
static U32 	cdisk_mode_sense();
static U32	cdisk_send_ccb();
static U32	cdisk_send_ccb_wait();


/********************** External Declarations ***************************/

/*
 * The following are the peripheral common functions.
 */
extern CCB_PATHINQ	*ccmn_pinq_ccb_bld();
extern CCB_SCSIIO 	*ccmn_io_ccb_bld();
extern CCB_SETASYNC 	*ccmn_sasy_ccb_bld();
extern CCB_RELSIM 	*ccmn_rsq_ccb_bld();
extern CCB_RESETDEV 	*ccmn_bdr_ccb_bld();
extern CCB_SCSIIO 	*ccmn_tur();
extern CCB_SCSIIO 	*ccmn_start_unit();
extern struct buf	*ccmn_get_bp();
extern u_char		*ccmn_get_dbuf();
extern void		ccmn_init();
extern void		ccmn_rem_ccb();
extern void		ccmn_rel_dbuf();
extern void		ccmn_rel_bp();
extern void		ccmn_rel_ccb();
extern U32		ccmn_abort_que();
extern U32		ccmn_send_ccb();
extern U32		ccmn_send_ccb_wait();
extern U32		ccmn_ccb_status();
extern I32		ccmn_open_unit();
extern void		ccmn_close_unit();
extern int		ccmn_DoSpecialCmd();
extern void		ccmn_errlog();
extern struct controller *ccmn_find_ctrl();
extern struct device     *ccmn_find_device();

/*
 * Kernel External Declarations:
 */
extern int 		biodone();
extern int 		physio();
extern int 		ptcmp();
extern int 		ssblk();
#ifndef SEC_BASE
extern int 		suser();
#endif
extern int 		timeout();
extern int 		wakeup();
extern void		conn_device();

extern PDRV_UNIT_ELEM pdrv_unit_table[];

extern SIM_SOFTC 	*softc_directory[];
extern struct device *camdinfo[];
extern struct controller *camminfo[];
extern long dk_wds[];		/* Used for statistical data */
extern long dk_xfer[];		/* Used for statistical data */
extern int hz;
extern int nCAMBUS;
extern int CAM_log_label_info;

#ifdef OSF
extern int cpu;
#endif
extern int lbolt;
extern int partial_dump;
extern int partial_dumpmag;
extern int dumpsize;

/****************** Initialized and Unitialized Data ********************/

static void (*local_errlog)() = ccmn_errlog; /* use common error logger */

static u_char no_useracc_str[] = "User access to data area failed";
static u_char invalid_data_str[] = "Invalid data area size";
static u_char invalid_senseoff_str[] = "Invalid sense data offset";
static u_char invalid_sense_data_str[] = "Invalid sense data size";
static u_char invalid_len_str[] = "Length of data struct too small";
static u_char dir_str[]="Direction Flags don't match Sub operation code.";
static u_char open_str[]="ccmn_open_unit Can't open device.";
static u_char mem_str[]="Can't obtain memory in device driver, please try again.";
static u_char data_str[]="Can't find driver data struct, (driver problem)";
static u_char cfg_subop_str[]="Invalid configuration sub opcode.";

static u_char maint_subop_str[] = "Invalid maintenance sub opcode.";
static u_char notsup_str[] = "This sub operation is not supported currently.";
static u_char busy_str[]="Device currently is opened and this operation is not allowed presently.";
static u_char maint_pass_str[]="Maintenance pass operation but lun is not markedas in Maintenance mode";

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_slave()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles the slave call for CAM disks/cdroms.  
 *	Call the common init routine to initialize the CAM subsystem.
 *	We indicate that a direct access and read only direct access  
 *	device types were configured at this address and call the
 *	ccmn_open_unit() routine to determine whether a direct
 *	access or read only direct access device exists.  
 *	If a device was found, we register the device for asynchronous
 *	callbacks with the CAM subsystem.  
 *
 *  FORMAL PARAMETERS:
 *	uba	- Pointer to UBA device structure
 *	vaddr	- Virtual register address - not used
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	PROBE_SUCCESS
 *	PROBE_FAILURE
 *		- ccmn_open_unit() indicates a device was not found.
 *		- unit number is invalid.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

cdisk_slave(attach, vaddr)
struct device *attach;
caddr_t vaddr;			/* Virtual register address - unused */
{
	PDRV_DEVICE 	*pd;	/* Pointer to peripheral device structure */
	int 		unit;	/* Unit index into unit table */
	CCB_SETASYNC  	*ccb;	/* Pointer to Set Asynch Callback ccb */
	dev_t dev;		/* CAM dev_t */

	/*
	 * The UBA_UNIT_TO_DEV_UNIT macro assumes ui_unit has bits 0-2 = lun, 
	 * bits 3-5 = target id, and 6-7 = bus num.
	 */
	dev = makedev(0, MAKEMINOR(UBA_UNIT_TO_DEV_UNIT(attach), 0));
	unit = DEV_UNIT(dev);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	   (CAMD_DISK | CAMD_INOUT), 
	   ("[%d/%d/%d] cdisk_slave: entry unit=0x%x\n", DEV_BUS_ID(dev),
	   DEV_TARGET(dev), DEV_LUN(dev), unit));

	/*
 	 * Call the common init routine to initialize the CAM subsystem.
	 */
	ccmn_init();

 	if (unit > MAX_UNITS)   {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		   (CAMD_DISK | CAMD_ERRORS), 
		   ("[%d/%d/%d] cdisk_slave: Unit number 0x%x too large\n", 
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), unit));
		return(PROBE_FAILURE);
	}
	
	/*
	 * Indicate that a direct access device was configured at this
	 * address.  We support both direct access and cdrom devices.
	 */
	pdrv_unit_table[unit].pu_config |= (1 << ALL_DTYPE_DIRECT);
	pdrv_unit_table[unit].pu_config |= (1 << ALL_DTYPE_RODIRECT);

	/*
	 * Determine whether a direct access device exists at this address by 
	 * calling ccmn_open_unit which will check the EDT table.
	 */
	if( ccmn_open_unit(dev, (U32)ALL_DTYPE_DIRECT,
	   (U32)0, (U32)sizeof(DISK_SPECIFIC)) != (U32)0)   {
		/*
		 * Determine whether a read only direct access device
		 * exists at this address.
		 */
		if( ccmn_open_unit(dev, (U32)ALL_DTYPE_RODIRECT,
	   	    (U32)0, (U32)sizeof(DISK_SPECIFIC)) != (U32)0)   {
		   PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		      (CAMD_DISK | CAMD_ERRORS), 
		      ("[%d/%d/%d] cdisk_slave: ccmn_open_unit failed\n",
   	   	      DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		   return(PROBE_FAILURE);
		}
	}

	/*
	 * Sanity check - should never happen!
	 */
	if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
		ccmn_close_unit(dev);
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		   (CAMD_DISK | CAMD_ERRORS), 
		   ("[%d/%d/%d]cdisk_slave: No peripheral device structure\n",
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		return(PROBE_FAILURE);
	}

	/* Set the logical unit number for indexing camdinfo struct */
	pd->pd_log_unit = attach->logunit;
	/*
	 * Check whether the device supports tagged queueing or
	 * does not allow disconnects and set the defaults.
	 */
	SET_PD_DEFAULTS(pd);


	/*
	 * Decrement the open count back to 0.
	 */
	ccmn_close_unit(dev);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	   (CAMD_DISK | CAMD_INOUT), 
	   ("[%d/%d/%d] cdisk_slave: exit - success\n",
   	    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

	return(PROBE_SUCCESS);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_attach()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles the attach call for the disk/cdrom driver.
 *	This routine will output the identify string for the device
 *	and save the device index for statistical information.
 *
 *  FORMAL PARAMETERS:
 *	uba	- Pointer to UBA device structure 
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	PROBE_SUCCESS
 *	PROBE_FAILURE
 *		- unit number is invalid.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

cdisk_attach(attach)
struct device *attach;
{
	PDRV_DEVICE 	*pd;	/* Pointer to periheral device structure */
	DISK_SPECIFIC 	*ds;	/* Pointer to disk specific struct */
	int 		unit;	/* Index into unit table */
	dev_t 		dev;
	char 		idstring[IDSTRING_SIZE+REV_STRING_SIZE+2];
	ALL_INQ_DATA 	*inqp;

	/*
	 * The UBA_UNIT_TO_DEV_UNIT macro assumes ui_unit has bits 0-2 = lun, 
	 * bits 3-5 = target id, and 6-7 = bus num.
	 */
	dev = makedev(0, MAKEMINOR(UBA_UNIT_TO_DEV_UNIT(attach), 0));
	unit = DEV_UNIT(dev);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   DEV_LUN(dev), (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_attach: entry unit=0x%x\n",
   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), unit));

	if (unit > MAX_UNITS)   {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
		   DEV_LUN(dev), (CAMD_DISK | CAMD_ERRORS),
		   ("[%d/%d/%d] cdisk_slave: Unit number 0x%x too large\n", 
   	  	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), unit));
		return(PROBE_FAILURE);
	}

	/*
	 * Sanity check - this should never happen.
	 */
	if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
		   DEV_LUN(dev), (CAMD_DISK | CAMD_ERRORS),
		   ("[%d/%d/%d]cdisk_attach: No peripheral device structure\n",
   	    	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		return(PROBE_FAILURE);
	}

	/*
	 * Save device index for statistical data for iostat.
	 */
	ds = (DISK_SPECIFIC *)pd->pd_specific;
	perf_init(attach);
	ds->ds_dkn = (int)attach->perf;

	/* 
	 * Output the identification string
	 */
	inqp = (ALL_INQ_DATA *)pd->pd_dev_inq;

	/*
	 * Is the device connected 
	 */
	bcopy((caddr_t)inqp->vid, idstring, VID_STRING_SIZE);
	bcopy((caddr_t)inqp->pid, &idstring[VID_STRING_SIZE], PID_STRING_SIZE);
	idstring[VID_STRING_SIZE + PID_STRING_SIZE] = ' ';
	bcopy((caddr_t)inqp->revlevel, 
	     &idstring[VID_STRING_SIZE + PID_STRING_SIZE+1], REV_STRING_SIZE);
	idstring[VID_STRING_SIZE + PID_STRING_SIZE+ REV_STRING_SIZE +1] = '\0';
	if( inqp->pqual == ALL_PQUAL_CONN){
	    printf(" (%s)", idstring);
	}
	else if( inqp->pqual == ALL_PQUAL_NOT_CONN){
	    printf(" (%s NOT CONNECTED)", idstring);
	}
	else {
	    return(PROBE_FAILURE);
	}


	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   DEV_LUN(dev), (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_attach: exit - success\n",
   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

	return(PROBE_SUCCESS);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_open()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine is the entry point for the upper layers. This routine
 *	calls cdisk_online to formally open the device and bring it online.
 *	Returns what cdisk_online returns.
 *	
 *
 *  FORMAL PARAMETERS:
 *	dev	- major/minor device number.
 *	flag	- unused.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	ENXIO	- Drive does not become ready.
 *	   	- Failure in sending mode select pages.
 *	   	- Failure in sending read capacity command.
 *		- Invalid CAM status.
 *		- Invalid sense key on check condition SCSI status for
 *		  the command sent on first open.
 *		- A direct access or cd-rom device does not exist at 
 *		  this address.
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	- If the device is open with the F_NODELAY flag bit set, all
 *	  errors are ignored.
 *
 ************************************************************************/

cdisk_open(dev, flag, fmt)
dev_t dev;
int flag;
int fmt;
{
    u_long type_open = 0;    /* What type of open - exclusive or normal */ 

    /* 
     * always allow open on this one... it's not really
     * there
     */
    if( minor(dev) == RAID_MINOR){
	return(0);
    }
    return( cdisk_online(dev, flag, fmt, type_open ));

}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_online()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine will handle the open of a disk/cd-rom device.  First
 *	the common open routine is called to determine if the device was
 *	configured and that it is the same type.  Then we obtain our
 *	peripheral device structure pointer.  If none exists, then
 *	cdisk_op_find_device is called to determine if the device now exists
 *	even though it was not there at power up.  The cdisk_op_spin()
 *	function is called to make sure the device is online. If the device
 *	has just been brought online, each page in the mode select table 
 *	contained in the device descriptor entry for the device is issued
 *	by calling cdisk_op_msel() and the Read Capacity command is
 *	issued by calling cdisk_read_capacity.  Then a read is issued to
 *	obtain the partition table.  Also a prevent media removable
 *	command is issued for removable devices which require this as
 *	indicated in the device descriptor table.
 *	
 *
 *  FORMAL PARAMETERS:
 *	dev	- major/minor device number.
 *	flag	- unused.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	ENXIO	- Drive does not become ready.
 *	   	- Failure in sending mode select pages.
 *	   	- Failure in sending read capacity command.
 *		- Invalid CAM status.
 *		- Invalid sense key on check condition SCSI status for
 *		  the command sent on first open.
 *		- A direct access or cd-rom device does not exist at 
 *		  this address.
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	- If the device is open with the F_NODELAY flag bit set, all
 *	  errors are ignored.
 *
 ************************************************************************/

cdisk_online(dev, flag, fmt, type_open)
dev_t dev;
int flag;
int fmt;
u_long type_open;
{
	PDRV_DEVICE   	*pd;	/* Pointer to peripheral device struct */
	DISK_SPECIFIC 	*dsp;	/* Pointer to disk specific struct */
	DEV_DESC 	*dd;	/* Pointer to device descriptor struct */
	PDRV_UNIT_ELEM  *pdu;   /* Pointer to unit table slot */
	struct controller *ctlr; /* Controller struct pointer */
	CCB_SETASYNC  	*ccb;	/* Pointer to Set Asynch Callback ccb */
	caddr_t		msen_data = 0; /* For raid level */
	int 		status;		/* Open return value */
	int 		dev_ready=0;	/* Flag to indicate whether the */
					/* device is ready */
	int 		spl, spl1;	/* priority level */
	int		part;
	int 		partmask;
	int		nbytes;
	static u_char	module[] = "cdisk_online";

	part = CD_GET_PARTITION(dev);
	partmask = 1 << part;
	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), (CAMD_DISK
	   | CAMD_INOUT), ("[%d/%d/%d] cdisk_open: entry dev=0x%x flag=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), dev,flag));

	PDU_IPLSMP_LOCK(LK_ONCE , spl);
	pdu = GET_PDRV_UNIT_ELEM(dev);
	pd = GET_PDRV_PTR(dev);


	if(( pd == (PDRV_DEVICE *)NULL) || ( pdu->pu_opens == NULL)){

	    /*
	     * Have to check for both device types  existing since we can't
	     * tell whether this is an open for a disk or a cdrom.
	     * Was this seen at boot....
	     */
	    if( (ctlr = ccmn_find_ctrl(dev)) != (struct controller *)NULL){
		struct device *device;
		u_char *space;

	        /* 
	         * Find the device struct if found saw it before else
	         * must connect one up
	         */
	        if((device = ccmn_find_device( ctlr, dev)) == ( struct device *)
			NULL){

		    PDU_IPLSMP_UNLOCK(spl);

	 	    /*
		     * The following should only be executed for devices
	 	     * which were not seen at boot.
		     */
		    if ( (status = cdisk_op_find_device(dev, type_open)) != 0)
			return(ENXIO);
	   	    pd = GET_PDRV_PTR(dev);

		    /*
		     * Check whether the device supports tagged queueing or
		     * does not allow disconnects and set the defaults.
		     */
		    SET_PD_DEFAULTS(pd);
		    /*
		     * Cannot have iostat statistics for devices added
 		     * after boot.
		     */
		    ((DISK_SPECIFIC *)pd->pd_specific)->ds_dkn = -1;
		    /*
		     * Allocate a device structure, fill it in, and attach it 
		     * to controller structure.
		     */
		    space = ccmn_get_dbuf(sizeof(struct device) 
			+ DEV_TYPE_SZ + DEV_NAME_SZ);
		    device = (struct device *)space;
		    device->ctlr_hd = ctlr;
		    device->dev_type = (char *)((vm_offset_t)space 
			+ sizeof(struct device));
		    bcopy ("disk", device->dev_type, 4);
		    device->dev_name = (char *)((vm_offset_t)device->dev_type  
			+ DEV_TYPE_SZ);
		    bcopy ("rz", device->dev_name, 2);
		    device->logunit = (DEV_BUS_ID(dev) << 3) + DEV_TARGET(dev);
		    device->unit = DEV_UNIT(dev);
		    device->ctlr_name = ctlr->ctlr_name;
		    device->ctlr_num = ctlr->ctlr_num;
		    device->alive = ALV_ALIVE;
		    conn_device(ctlr, device);
		}
		else { /* device struct found so something is there */
		    PDU_IPLSMP_UNLOCK(spl);
	    	    /*
	     	     * Call the common open routine.
	     	     */
   	    	     if( (status = ccmn_open_unit(dev, (U32)ALL_DTYPE_DIRECT,
		     	     (U32)type_open, (U32)sizeof(DISK_SPECIFIC))) != 0) {
   			if( (status = ccmn_open_unit(dev, (U32)ALL_DTYPE_RODIRECT,
		       	    	(U32)type_open, 
				(U32)sizeof(DISK_SPECIFIC))) != 0) {
		    	    return(ENXIO);
			}
	    	    }
	    	    /* 
	    	     * Get the pdrv_device struct ptr
	    	     */
	    	    pd = GET_PDRV_PTR(dev);
		}
	    }
	    else { /* No controller struct this should not happen */
		PDU_IPLSMP_UNLOCK(spl);
		return(ENXIO);

	    }
	}  else  {
	    /*
	     * Call the common open routine.
	     */
   	    if( (status = ccmn_open_unit(dev, (U32)ALL_DTYPE_DIRECT,
		     (U32)type_open, (U32)sizeof(DISK_SPECIFIC))) != 0)  {
   		if( (status = ccmn_open_unit(dev, (U32)ALL_DTYPE_RODIRECT,
		       	    (U32)type_open, (U32)sizeof(DISK_SPECIFIC))) != 0) 
		    return(ENXIO);
	    }
	    /* 
	     * Get the pdrv_device struct ptr
	     */
	    pd = GET_PDRV_PTR(dev);
	}
	/* 
	 * Register the device for asynch callbacks on Bus Reset,
	 * Bus Device Reset, and SCSI Asynchronous Event Notification
	 */
	ccb = ccmn_sasy_ccb_bld(dev, (U32)0,
		(U32)(AC_SENT_BDR | AC_SCSI_AEN | AC_BUS_RESET),
		cdisk_async_cb, (u_char *)NULL, 0);

    	if( CAM_STATUS(ccb) != CAM_REQ_CMP)	{
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		CAM_ERROR(module, "Set Async Callback Failed", 
			CAM_SOFTERR, (CCB_HEADER *)ccb, 
			pd->pd_dev, (u_char *)NULL);
		PDRV_IPLSMP_UNLOCK(pd, spl);
	}
	ccmn_rel_ccb((CCB_HEADER *)ccb);

	dsp = (DISK_SPECIFIC *)pd->pd_specific;
	dd = (DEV_DESC *)pd->pd_dev_desc;

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	pd->pd_flags &= ~(PD_NO_DEVICE | PD_OFFLINE);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	/* 
	 * Stall if recovery is happening
	 */
	PDRV_IPLSMP_LOCK( pd, LK_RETRY, spl);

	while(( pd->pd_flags & PD_REC_INPROG ) != 0 ){
	    PDRV_SMP_SLEEPUNLOCK( &pd->pd_flags, PZERO, pd );
	    PDRV_IPLSMP_LOCK( pd, LK_RETRY, spl1);
	}
	/* 
	 * We have opened the unit so clean out our recovery flags
	 */
	pd->pd_fatal_rec = 0;
	pd->pd_recovery_cnt = 0;

	PDRV_IPLSMP_UNLOCK( pd, spl ); /* restore to orignal spl */

	/*
	 * Try to bring the drive on line.  If it fails but opened
	 * with FNDELAY just keep going.
	 */

	if( (status = cdisk_op_spin(pd)) != CAM_SUCCESS)  {
		pd->pd_flags |= PD_OFFLINE;
		if( !(flag & (FNDELAY|FNONBLOCK)) ) {
			cdisk_close_dev(pd, pdu, dsp, dev);
			return(status);
		}
	}  else {
		dev_ready = 1;
	}
	/*
	 * If the device exists but is off-line, isssue the Mode Select
	 * and Read Capacity commands.
	 */
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	if ( !(pd->pd_flags & PD_ONLINE) && !(pd->pd_flags & PD_NO_DEVICE) )  {

		DIR_READ_CAP_DATA *rdcp_data;

		PDRV_IPLSMP_UNLOCK(pd, spl);
		/*
		 * Issue the Mode Select comamnd for all the Mode
		 * Select pages.
		 */
		if( (status = cdisk_op_msel(pd)) != CAM_SUCCESS)  {
			if( !(flag & (FNDELAY|FNONBLOCK)) ) {
				cdisk_close_dev(pd, pdu, dsp, dev);
				return(status);
			}
			else
				dev_ready = 0;
		}

		/* Allocate space for Read Capacity data */
		rdcp_data = (DIR_READ_CAP_DATA *)
			    ccmn_get_dbuf((U32)sizeof(DIR_READ_CAP_DATA));

		/*
		 * Issue the Read Capacity comamnd.
		 */
		if( (status = cdisk_read_capacity(pd, rdcp_data)) != 
		     CAM_SUCCESS)  {
			if( !(flag & (FNDELAY|FNONBLOCK)) ) {
				ccmn_rel_dbuf((u_char *)rdcp_data,
			    	     (U32)sizeof(DIR_READ_CAP_DATA));
				cdisk_close_dev(pd, pdu, dsp, dev);
				return(status);
			}
			else
				dev_ready = 0;
		}  else  {
			U32 lbn, blk_len;
			/*
	 		 * RDCAP returns the address of the last LBN.
	 		 * We must add one to get the number of LBNs.
	 		 */
	    	    	BTOL(&rdcp_data->lbn3, lbn);
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   (CAMD_DISK | CAMD_FLOW),
			   ("[%d/%d/%d] cdisk_open: read_cap:lbn=0x%x\n", 
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun, lbn));
			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			dsp->ds_tot_size = (lbn + (U32)1);
			PDRV_IPLSMP_UNLOCK(pd, spl);
			/*
	 		 * Verify block size matches device desc data.
	 		 */
			BTOL(&rdcp_data->block_len3, blk_len);
			/*
			 * No longer needed... Block size is always
			 * from the device.......
			if ( blk_len != dd->dd_block_size ) {
				PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
				CAM_ERROR(module, "Invalid Block Size", 
					CAM_SOFTERR, NULL, 
					pd->pd_dev, (u_char *)NULL);
				PDRV_IPLSMP_UNLOCK(pd, spl);
                                if( !(flag & (FNDELAY|FNONBLOCK)) ) {
					cdisk_close_dev(pd, pdu, dsp, dev);
					ccmn_rel_dbuf((u_char *)rdcp_data,
			    	            (U32)sizeof(DIR_READ_CAP_DATA));
					return(EIO);
				}
				else
					dev_ready = 0;

			}  else 
			*/
	    		dsp->ds_block_size = blk_len;
		}
		/* Deallocate the space for the Read Capacity data */
		ccmn_rel_dbuf((u_char *)rdcp_data, 
					(U32)sizeof(DIR_READ_CAP_DATA));
		/* 
		 * Check to see if is a RAID device, 
		 * Determine what raid level running..
		 * The block size in the descriptor Table tells us
		 * Page code byte offset and size of page.
		 * byte 0 page num, byte 1 offset within page
		 * byte 2 & 3 page size.
		 */
		if((dd->dd_device_type & SZ_RAID) != 0){
		    /*
		     * Do a mode sense of of the page to
		     * tells us what raid level
		     */
		    if(dd->dd_block_size != 0){
			nbytes = ((dd->dd_block_size >> 16) & 0xffff) + 
				CD_MODE_HEADER_DESC;
				
			msen_data = (caddr_t)ccmn_get_dbuf( nbytes );
			if(msen_data != (char *)NULL){
		    	    if( cdisk_mode_sense(pd, msen_data, 
				    (dd->dd_block_size & 0xff), nbytes) == 0){
				dsp->ds_raid_level = *(msen_data + 
				    ((dd->dd_block_size >> 8) & 0xff) +
				    CD_MODE_HEADER_DESC);
			    }
		    	    ccmn_rel_dbuf((u_char *)msen_data, nbytes);
		        }
		    }
		}


	} else
	
		PDRV_IPLSMP_UNLOCK(pd, spl);
	/*
	 * See if we need to read in the partition table from the disk.
	 * The conditions in which we will have to read from the disk are
 	 * if the partition table valid bit is not set or the device is
	 * is being powered on. Assume the default values before trying to
	 * see if the partition tables are on the disk. The 
	 * strategy routine expects a partition table.
	 */
#ifdef LABELS
	if (dsp->ds_openpart == 0) {
		cdisk_read_label(dev, pd, dsp, cdisk_strategy, part);
	}
#else	/* LABELS */
	if ( (dsp->ds_pt.pt_valid == 0) || !(pd->pd_flags & PD_ONLINE) )  {
		int i;
		struct pt_info *ptp = dd->dd_def_partition;

		for( i = 0; i < CD_NUM_PARTITIONS; i++ ) {
			dsp->ds_pt.pt_part[i].pi_nblocks = ptp[i].pi_nblocks;
			dsp->ds_pt.pt_part[i].pi_blkoff = ptp[i].pi_blkoff;
		}
		dsp->ds_pt.pt_valid = PT_VALID;	
		/*
		 * Default partitions are now set. Call rsblk to set
		 * the driver's partition table.
		 */
		rsblk(cdisk_strategy, dev, &dsp->ds_pt);
	}
#endif	/* LABELS */

	/*
	 * Indicate that this partition is opened.
	 */
	dsp->ds_openpart |= partmask;

#ifdef LABELS
	/* 
	 * Track openness of each partition.
	 */
	switch (fmt) {
	   case S_IFCHR:
		dsp->ds_copenpart |= partmask;
		break;
	   case S_IFBLK:
		dsp->ds_bopenpart |= partmask;
		break;
	}
#endif	/* LABELS */

	/*
	 * Issue a Prevent Media Removal command for removable 
	 * devices if required.
	 */
	if ( !(pd->pd_flags & PD_OFFLINE) 
	    && (((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb)
	    && (pd->pd_dev_desc->dd_scsi_optcmds & SZ_PREV_ALLOW)
	    && (fmt == S_IFBLK) )
	        if(cdisk_media_remove(pd, CD_PREVENT_REMOVAL) != 0)
		    PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			(CAMD_DISK | CAMD_ERRORS),
	   		("[%d/%d/%d] cdisk_open: Prevent Removal failed\n",
	   		pd->pd_bus, pd->pd_target, pd->pd_lun));

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	if (dev_ready)	
		pd->pd_flags |= PD_ONLINE;
	else
		/* here if FNODELAY is set */
		pd->pd_flags |= PD_NO_DEVICE;

	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_open: exit - success\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return (CAM_SUCCESS);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_read_label()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *  FORMAL PARAMETERS:
 *	dev	- major/minor device number.
 *	flag	- unused.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

#ifdef 	LABELS

int
cdisk_read_label(dev, pd, dsp, strat_func, part)
dev_t dev;
PDRV_DEVICE *pd;
DISK_SPECIFIC *dsp;
int (*strat_func)();
int part;
{
    struct pt p_t;
    struct pt *part_table = &p_t;
    struct disklabel *lp = &dsp->ds_label;
    int i;
    struct pt_info *ptp = pd->pd_dev_desc->dd_def_partition;
    char 		*msg;
    char		*readdisklabel();
    int    partmask = 1 << part;

    PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_read_label: begin\n",
	    pd->pd_bus, pd->pd_target, pd->pd_lun));

    /*
     * First read 4.3T disk label, if it exists.
     * Set up default sizes until we have the label, or longer
     * if there is none.  Set secpercyl, as readdisklabel wants
     * to compute b_cylin (although we do not need it), and set
     * nsectors in case diskerr is called.
     */
    lp->d_magic = 0;
    lp->d_secsize = DEV_BSIZE;
    lp->d_secperunit = dsp->ds_tot_size;

    lp->d_secpercyl = 1;
    lp->d_nsectors = lp->d_secperunit;
    lp->d_npartitions = 1;
    lp->d_partitions[0].p_size = lp->d_secperunit;
    lp->d_partitions[0].p_offset = 0;

    if ((msg = readdisklabel((dev & ~(CD_GET_PARTITION(dev))), strat_func, lp)) 	!= NULL)  {
	/*
	 * Only log messages to syslog if the site config explicitly
	 * requests them...
	 */
	if (CAM_log_label_info) {
          log(LOG_ERR, "rz%d: error reading disk label -- %s\n",
                        pd->pd_log_unit, msg);
	}

	/* No disk label. Try for Ultrix part table */
	lp->d_magic = 0;
	for( i = 0; i < CD_NUM_PARTITIONS; i++ ) {
	    dsp->ds_pt.pt_part[i].pi_nblocks = ptp[i].pi_nblocks;
	    dsp->ds_pt.pt_part[i].pi_blkoff = ptp[i].pi_blkoff;
	}
	dsp->ds_pt.pt_valid = PT_VALID;	
	/*
	 * Default partitions are now set. Call rsblk to set
	 * the driver's partition table.
	 */
	if(rsblk(strat_func, dev, &dsp->ds_pt))   {
	    /*
	     * Only log messages to syslog if the site config explicitly
	     * requests them...
	     */
	    if (CAM_log_label_info) {
                log(LOG_ERR,
	            "rz%d: no partition info on disk - using default values.\n",
		    pd->pd_log_unit);
	    }
	    pd->pd_flags |= PD_OPENRAW;
        } else {
	    /*
	     * NOTE: We'll still spew a message about ULTRIX file systems,
	     * just so the user doesn't accidentally trash their disk.
	     * Eventually, this should be put under "CAM_log_label_info"
	     * also...
	     */
            log(LOG_ERR,"rz%d: using ULTRIX partition info found on disk.\n",
                pd->pd_log_unit);
        }
	/*
	 * Copy the Ultrix partition table to the disk label.
	 */
	lp->d_npartitions = 8;
	for(i=0; i<8; i++)   {
	    int size = dsp->ds_pt.pt_part[i].pi_nblocks;
	    if(size == -1)  {
		size = dsp->ds_tot_size;
		size -= dsp->ds_pt.pt_part[i].pi_blkoff;
	    }
	    lp->d_partitions[i].p_size = size;
	    lp->d_partitions[i].p_offset =
		dsp->ds_pt.pt_part[i].pi_blkoff;
	}
    }

    PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_read_label: end\n",
	    pd->pd_bus, pd->pd_target, pd->pd_lun));
}
#endif	/* LABELS */

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_close()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle the close of the disk driver.  It will
 *	call the common close routine to decrement the open count for
 *	the device.  For removable media which require prevent media
 *	removal, we issue the allow media removal command if all 
 *	partitions are closed.
 *
 *  FORMAL PARAMETERS:
 *	dev	- major/minor device number.
 *	flag	- unused.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

int
cdisk_close(dev, flag, fmt)
dev_t dev;
int flag;
int fmt;
{
	PDRV_DEVICE *pd;
	DISK_SPECIFIC *ds;
	CCB_SETASYNC *ccb;
	int   partmask;
	int spl;



        /* 
         * always allow close  on this one... it's not really
         * there
         */
        if( minor(dev) == RAID_MINOR){
	    return(0);
        }

	partmask = 1 << CD_GET_PARTITION(dev);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), (CAMD_DISK
	   | CAMD_INOUT), ("[%d/%d/%d] cdisk_close: entry\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

	if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
		   DEV_LUN(dev), (CAMD_DISK | CAMD_ERRORS),
		   ("[%d/%d/%d] cdisk_close: No peripheral device structure\n",
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		return(ENODEV);
	}
	ds = (DISK_SPECIFIC *)pd->pd_specific;


	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	/* 
	 * Track openness of each partition.
	 */
	switch (fmt) {
	   case S_IFCHR:
		ds->ds_copenpart &= ~partmask;
		break;
	   case S_IFBLK:
		ds->ds_bopenpart &= ~partmask;
		break;
	}
	ds->ds_openpart = ds->ds_bopenpart | ds->ds_copenpart;
	if( ds->ds_openpart == 0 )  {
		ds->ds_wlabel = 0;
	}

	/*
	 * If there is still open references to this LUN
	 * just return.
	 */
	if( ds->ds_bopenpart || ds->ds_copenpart ) {
	    PDRV_IPLSMP_UNLOCK(pd, spl);
	    return(CAM_SUCCESS);
	}

	/*
	 * Issue an Allow Media Removal command for removable 
	 * devices if no partitions are open and the device 
	 * supports it.
	 */
	if(  !(pd->pd_flags & PD_OFFLINE)
	   && (ds->ds_bopenpart == 0) && ((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb)
	    if(pd->pd_dev_desc->dd_scsi_optcmds & SZ_PREV_ALLOW)
	    	if(cdisk_media_remove(pd, CD_ALLOW_REMOVAL) != 0)
		    PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			(CAMD_DISK | CAMD_ERRORS),
	   		("[%d/%d/%d] cdisk_open: Allow Removal failed\n",
	   		pd->pd_bus, pd->pd_target, pd->pd_lun));

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), (CAMD_DISK
	   | CAMD_INOUT), ("[%d/%d/%d] cdisk_close: exit\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
	
	/* 
	 * Clear out the online and set offline to force 
	 * the unit to reread capacity info... A must for raid
	 */
	pd->pd_flags |= PD_OFFLINE;
	pd->pd_flags &= ~PD_ONLINE;

	PDRV_IPLSMP_UNLOCK(pd, spl);
	/*
	 * Deregister for call back..
	 */
        ccb = ccmn_sasy_ccb_bld(dev, (U32)0,
		(U32)0, cdisk_async_cb, (u_char *)NULL, 0);


        ccmn_rel_ccb((CCB_HEADER *)ccb);

	ccmn_close_unit(dev);

	return(CAM_SUCCESS);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_strategy()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle reads and writes to the device.  The
 *	request is validated by checking that the partition table is valid
 * 	and that the request fits into the partition.  A SCSI I/O ccb
 *	is created for the read/write command.  If the request is not a
 *	multiple of the block size of the device, then a scatter gather map is
 *	created using an allocated system buffer for the remaining bytes
 *	of the transfer.  The request is placed on the peripheral driver
 *	queue by calling cdisk_send_ccb() and sent to the XPT/SIM.
 *
 *  FORMAL PARAMETERS:
 *	bp	- Buf structure pointer.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	The bp is returned via biodone with b_error set to:
 *		ENOSPC - Request is not valid.
 *		ENXIO - If opened with FNDELAY and the device did not
 *			exist or the partition table is invalid.
 *		ENODEV - No peripheral device structure is allocated. -
 *			 This should never happen!
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	If recovery is in progress, th bp is saved and will be reissued
 *	to strategy when recovery completes.
 *
 ************************************************************************/

void
cdisk_strategy(bp)
register struct buf *bp;
{
	CCB_SCSIIO 	*ccb;		/* Pointer to created SCSI I/O CCB */
	daddr_t 	part_size;	/* Size of parition in blocks */
	U32 		resid=0;	/* Residual count of transfers which */
					/* are not a multiple of block size */
	U32 		nblocks;	/* Number of blocks for transfer */
	U32 		end_blk;	/* Last block number of transfer */
	U32 		start_blk;	/* Starting block number of transfer */
	int 		part;		/* Partition table index */
	struct pt 	*pt;		/* Pointer to disk partition table */
	PDRV_DEVICE 	*pd;		/* Peripheral device pointer */
	DISK_SPECIFIC 	*ds;		/* Disk Specific structure ptr */
	int 		spl;		/* Saved priority */
	U32 		cam_flags;	/* CAM flags including direction */
	U32 		status;		/* Return value from xpt_action */
	dev_t 		dev;
	struct partition *pp;
	u_char		tag_action;
	static u_char	module[] = "cdisk_strategy";

	dev = bp->b_dev;


	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
	   DEV_LUN(dev), (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_strategy: entry bp=0x%x blk=0x%x count=%d\n", 
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   bp, bp->b_blkno, bp->b_bcount));

	/*
	 * Sanity check - this should never happen!
	 */
	if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
		CAM_ERROR(module, "No device struct", CAM_SOFTWARE,
			(CCB_HEADER *)NULL, dev, (u_char *)NULL);
		IO_STRATEGY_ERR(bp, ENODEV);
		return;
	}
	/*
	 * Check to see if we have posted a fatal recovery error
	 */
	if( (pd->pd_recovery_cnt > CDISK_REC_RETRY) || pd->pd_fatal_rec ){
		IO_STRATEGY_ERR(bp, EIO);
		return;
	}

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	ds = (DISK_SPECIFIC *)pd->pd_specific;


	/*
	 * If recovery is in progress, save away new requests until
	 * the recovery process completes.
	 */
	if (pd->pd_flags & PD_REC_INPROG) {
		bp->b_actf = ds->ds_bufhd;
		ds->ds_bufhd = bp;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
	  	   ("[%d/%d/%d] cdisk_strategy: Recovery in progress bp=0x%x\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, bp));
		return;
	}
	/*
	 * If PD_NO_DEVICE is set, the device was opened
	 * with FNDELAY, but the device didn't respond.
	 */
	if( pd->pd_flags & PD_NO_DEVICE ) {
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
		   ("[%d/%d/%d] cdisk_strategy: No device - opened with F_NODELAY\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		IO_STRATEGY_ERR(bp, ENXIO);
		return;
	}

        if(CAM_IS_HWRELOC_SET(bp->b_flags)) {
		IO_STRATEGY_ERR(bp, EINVAL);
		return;
	}

	/*
	 * Check if the partition table is valid.
	 */
#if !LABELS
	pt = &ds->ds_pt;
	if ( ds->ds_pt.pt_valid != PT_VALID )  {
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		  (CAMD_DISK | CAMD_ERRORS),
		   ("[%d/%d/%d] cdisk_strategy: Invalid Partition Table\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));

		IO_STRATEGY_ERR(bp, ENXIO);
		return;
	}
#endif	/* !LABELS */

	PDRV_IPLSMP_UNLOCK(pd, spl);

	/*
	 * Check that the transfer does not go beyond the
	 * bounds of the partition.
	 */
	part = CD_GET_PARTITION(dev);
	nblocks = (bp->b_bcount + (DEV_BSIZE - 1)) >> DEV_BSHIFT;
	end_blk = bp->b_blkno + nblocks;
#if LABELS
	pp = &ds->ds_label.d_partitions[part];
	end_blk = bp->b_blkno + nblocks + pp->p_offset;
	start_blk = bp->b_blkno + pp->p_offset;
	/*
	 * Should not return EROFS for zero sized partitions.
	 */
	if ( ((bp->b_blkno + pp->p_offset) <= LABELSECTOR) &&
#if LABELSECTOR != 0
	   ((bp->b_blkno + pp->p_offset + nblocks) <= LABELSECTOR) &&
#endif
	   ((bp->b_flags & B_READ) == 0) &&
	   (pp->p_size != 0) &&
	   (ds->ds_label.d_magic == DISKMAGIC) &&
	   (ds->ds_wlabel == 0))   {
		IO_STRATEGY_ERR(bp, EROFS);
		return;
	}
	if( (bp->b_blkno < 0) || (bp->b_blkno >= pp->p_size) 
	   || (pp->p_offset >= ds->ds_tot_size) ) {
	   /* 
	    * Needed for POSIX compliance - indicate no bytes read 
	    * but no error on reads at or beyond EOP.
	    */
	   if( bp->b_flags & B_READ ) {
		bp->b_resid = bp->b_bcount;				
		(void) biodone(bp);
	   } else {
		/* Writes are still an error */
		IO_STRATEGY_ERR(bp, ENOSPC);
	   }
	   return;
	}
	/*
	 * Transfer up to end of partition.
	 */
	if( (bp->b_blkno + nblocks) > pp->p_size )  {
		/* Save the original count in case of failure */
		bp->b_resid = bp->b_bcount;
		bp->b_bcount = (pp->p_size - bp->b_blkno) << DEV_BSHIFT;
	} else  {
		bp->b_resid = 0;
	}
#else	/* LABELS */
	start_blk = bp->b_blkno + pt->pt_part[part].pi_blkoff;
	if( pt->pt_part[part].pi_nblocks == -1 )
		part_size = ds->ds_tot_size - pt->pt_part[part].pi_blkoff;
	else
		part_size = pt->pt_part[part].pi_nblocks;
	if ((bp->b_blkno < 0) || (end_blk > part_size) ||
	    (pt->pt_part[part].pi_blkoff >= ds->ds_tot_size)) {
		IO_STRATEGY_ERR(bp, ENOSPC);
		return;
	}
	end_blk = bp->b_blkno + nblocks + pt->pt_part[part].pi_blkoff;
#endif	/* LABELS */

	tag_action = pd->pd_tag_action;

	/* Set the direction bit */
	if( bp->b_flags & B_READ )
		cam_flags = pd->pd_cam_flags | CAM_DIR_IN;
	else   {
		cam_flags = pd->pd_cam_flags | CAM_DIR_OUT;
	}
	/*
	 * Obtain a SCSI I/O CCB.
 	 */
	ccb = ccmn_io_ccb_bld(dev, (u_char *)bp->b_un.b_addr, 
				(U32)bp->b_bcount,
				GET_SENSE_SIZE(pd), 
				cam_flags, cdisk_complete, 
				tag_action, GET_IO_TIMEOUT(pd), bp);
	/*
	 * If B_ASYNC is not set in b_flags, then allow the SIM
	 * to give this CCB priority.  Assign a priority level
	 * of DEC_CAM_LOW_PRIOR.  The default is DEC_CAM_ZERO_PRIOR.
	 */
	if (!(bp->b_flags & B_ASYNC))
	    ccb->cam_vu_flags |= DEC_CAM_LOW_PRIOR;

	/*
	 * Check if not a multiple of the disk block size - if so
	 * create the scatter gather list.
	 */
	if( (resid = (bp->b_bcount % ds->ds_block_size)) != 0 )  {
		SG_ELEM *sg;
		u_char *addr;

		resid = ds->ds_block_size - resid;

		/* Get space for scatter gather list */
		sg = (SG_ELEM *)ccmn_get_dbuf((U32)(sizeof(SG_ELEM) * 2));

		/* Get space for data to go */
		addr = ccmn_get_dbuf(ds->ds_block_size);

		/*
		 * Change CCB to indicate there is a valid scatter/gather 
		 * list for this request.
		 */
		ccb->cam_dxfer_len  = bp->b_bcount + resid;
		ccb->cam_sglist_cnt = 2;
		ccb->cam_ch.cam_flags |= CAM_SCATTER_VALID;
		ccb->cam_data_ptr = (u_char *)sg;

		/* Fill in the scatter gather list entries */
		sg->cam_sg_address = (u_char *)bp->b_un.b_addr;
		sg->cam_sg_count = bp->b_bcount;
		sg++;
		sg->cam_sg_address = addr;
		sg->cam_sg_count = resid;

	}

	/*
	 * Build the CDB, check whether we need a 10 byte cdb.
	 */
	nblocks = (resid+bp->b_bcount)/ds->ds_block_size;
	if( bp->b_flags & B_READ )	{
		if ( (end_blk > DIR_MAX_LBA_CDB6)	
		    || (nblocks > DIR_MAX_LEN_CDB6) )  {
			DIR_READ_CDB10 *cdb;
			cdb = (DIR_READ_CDB10 *)ccb->cam_cdb_io.cam_cdb_bytes;
			cdb->opcode = DIR_READ10_OP;

			/* 
			 * Can sorting be performed on this device?
			 * If so, load cam_sort with the LBN.
			 */
			if (pd->pd_dev_desc->dd_flags & SZ_REORDER)
			    ccb->cam_sort = start_blk;

			DIRLBN_TO_READ10(start_blk, cdb);
			DIRTRANS_TO_READ10(nblocks, cdb);
			ccb->cam_cdb_len = sizeof(DIR_READ_CDB10);
		}  else  {
			DIR_READ_CDB6 *cdb;
			cdb = (DIR_READ_CDB6 *)ccb->cam_cdb_io.cam_cdb_bytes;
			cdb->opcode = DIR_READ6_OP;

			/* 
			 * Can sorting be performed on this device?
			 * If so, load cam_sort with the LBN.
			 */
			if (pd->pd_dev_desc->dd_flags & SZ_REORDER)
			    ccb->cam_sort = start_blk;

			DIRLBN_TO_READ6(start_blk, cdb);
			DIRTRANS_TO_READ6(nblocks, cdb);
			ccb->cam_cdb_len = sizeof(DIR_READ_CDB6);
		}
	} else {
		if ( (end_blk > DIR_MAX_LBA_CDB6)	
		    || (nblocks > DIR_MAX_LEN_CDB6) )  {
			DIR_WRITE_CDB10 *cdb;
			cdb = (DIR_WRITE_CDB10 *)ccb->cam_cdb_io.cam_cdb_bytes;
			cdb->opcode = DIR_WRITE10_OP;

			/* 
			 * Can sorting be performed on this device?
			 * If so, check b_flags for B_PHYS.  If not set,
			 * then the I/O is from the file system and is
			 * safe to reorder.
			 */
			if ((pd->pd_dev_desc->dd_flags & SZ_REORDER) &&
			    !(bp->b_flags & B_PHYS))
			    ccb->cam_sort = start_blk;

			DIRLBN_TO_WRITE10(start_blk, cdb);
			DIRTRANS_TO_WRITE10(nblocks, cdb);
			ccb->cam_cdb_len = sizeof(DIR_WRITE_CDB10);
		} else {
			DIR_WRITE_CDB6 *cdb;
			cdb = (DIR_WRITE_CDB6 *)ccb->cam_cdb_io.cam_cdb_bytes;
			cdb->opcode = DIR_WRITE6_OP;

			/* 
			 * Can sorting be performed on this device?
			 * If so, check b_flags for B_PHYS.  If not set,
			 * then the I/O is from the file system and is
			 * safe to reorder.
			 */
			if ((pd->pd_dev_desc->dd_flags & SZ_REORDER) &&
			    !(bp->b_flags & B_PHYS))
			    ccb->cam_sort = start_blk;

			DIRLBN_TO_WRITE6(start_blk, cdb);
			DIRTRANS_TO_WRITE6(nblocks, cdb);
			ccb->cam_cdb_len = sizeof(DIR_WRITE_CDB6);
		}
	}

	/*
	 * Place the request on the queue and send to XPT
	 */
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	status = cdisk_send_ccb(pd, (CCB_HEADER *)ccb, NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	/*
	 * Need to check return status - some status values will not
	 * place the ccb on the SIM queue so we need to call the
	 * callback completion function ourseleves.
	 */ 
	if( status != CAM_REQ_INPROG )
		cdisk_complete(ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_strategy: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_size()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	Obtains the size of a partition.  This function is called by the
 *	kernel and is an entry in the bdevsw table.
 *
 *  FORMAL PARAMETERS:
 *	dev - Major/minor device number.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	-1	- No device exists at the specified address.
 *	partition size - Success.
 *
 *  SIDE EFFECTS:
 *	This function will panic as do all other size functions in
 *	drivers if the partition table is invalid.
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

daddr_t
cdisk_size(dev)
dev_t dev;
{
	DISK_SPECIFIC *ds;	/* Pointer to disk specific struct */
	PDRV_DEVICE *pd;	/* Pointer to peripheral device struct */
	int part;		/* Index into partition table */
	daddr_t psize;		/* Returned partition size */

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	   (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_size: entry\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

	/* Check whether a device exists at this address */
	if ( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	   	   (CAMD_DISK | CAMD_ERRORS),
	   	   ("[%d/%d/%d] cdisk_size: No device struct\n",
	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		return((daddr_t)-1);
	}

	ds = (DISK_SPECIFIC *)pd->pd_specific;
	part = CD_GET_PARTITION(dev);

#if LABELS
	if( part >= ds->ds_label.d_npartitions )
		return(-1);
	psize = ds->ds_label.d_partitions[part].p_size;
#else	/* LABELS */
	if ( ds->ds_pt.pt_valid != PT_VALID ) 
		panic("cdisk_size: Invalid parition table\n");

	if (ds->ds_pt.pt_part[part].pi_nblocks != -1)
		psize = ds->ds_pt.pt_part[part].pi_nblocks;
	else
		psize = ds->ds_tot_size - ds->ds_pt.pt_part[part].pi_blkoff;
#endif	/* LABELS */

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	   (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_size: exit size = 0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), psize));

	return(psize);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_read()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle a raw read to a device.  A buf
 *	structure is obtained from the common pool and used for the transfer.
 *	The buf structure is released when the transfer completes.
 *
 *  FORMAL PARAMETERS:
 *	dev	- major/minor device number.
 *	uio	- Pointer to UIO structure.
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

cdisk_read(dev, uio)
register dev_t dev;		/* major/minor number */
register struct uio *uio;	/* Pointer to uio structure */
{
	struct buf *bp;		/* Pointer to allocated buf structure */
	int 	   ret_val;	/* Physio() return value */

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
   	   (CAMD_DISK | CAMD_INOUT), ("[%d/%d/%d] cdisk_read: entry dev=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), dev));

	if( (bp = ccmn_get_bp()) == (struct buf *)NULL )
		return(ENOMEM);

	ret_val = physio(cdisk_strategy, bp, 
			dev, B_READ, cdisk_minphys, uio);

	ccmn_rel_bp(bp);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_DISK | CAMD_INOUT), ("[%d/%d/%d] cdisk_read: exit\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

	return(ret_val);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_write()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle a raw write to a device.  A buf
 *	structure is obtained from the common pool and used for the transfer.
 *	The buf structure is released when the transfer completes.
 *
 *  FORMAL PARAMETERS:
 *	dev	- major/minor device number.
 *	uio	- Pointer to uio structure
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

cdisk_write(dev, uio)
register dev_t dev;		/* major/minor number */
register struct uio *uio;	/* Pointer to uio structure */
{
	struct buf *bp;		/* Pointer to allocated buf structure */
	int 	   ret_val;	/* Physio() return value */

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_DISK | CAMD_INOUT), ("[%d/%d/%d] cdisk_write: entry dev=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), dev));

	if( (bp = ccmn_get_bp()) == (struct buf *)NULL )
		return(ENOMEM);

	ret_val = physio(cdisk_strategy, bp, 
			dev, B_WRITE, cdisk_minphys, uio);

	ccmn_rel_bp(bp);

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_DISK | CAMD_INOUT), ("[%d/%d/%d] cdisk_write: exit\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

	return(ret_val);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_ioctl()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle the following ioctl commands:
 *		DIOCGETPT - Get the partition table.
 *		DIOCDGTPT - Get the default partition table.
 *		DIOCSETPT - Set the partition table.
 *		DEVIOCGET - Get device status information.
 *		DEVGETGEOM - Get disk geometry information.
 *		SRVC_REQUEST - Perform a service request (nstd RAID Only).
 *	All other SCSI commands will be handled via the special command 
 *	interface, ccmn_DoSpecialCmd().
 *		
 *
 *  FORMAL PARAMETERS:
 *	dev	- major/minor device number.
 *	cmd	- Ioctl command.
 *	data	- User data buffer - already copied in.
 *	flag	- unused.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	EACCESS	- Not superuser.
 *	EIO	- For the DEVGETGEOM command if the read capacity fails
 *		  or there is no geometry information available.
 *	ENXIO	- Invalid ioctl command.
 *	ENODEV	- No peripheral device structure alloacted. - This 
 *		  should never happen!
 *	EROFS 	- For DIOCSETPT on cd-rom devices.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

cdisk_ioctl(dev, cmd, data, flag)
dev_t dev;		/* major/minor nunber */
register int cmd;	/* Ioctl command */
caddr_t data;		/* User data buffer - already copied in */
int flag;		/* unused */
{
	PDRV_DEVICE 	*pd;	/* Pointer to peripheral device struct */
	DEV_DESC 	*dd;	/* Pointer to device descriptor entry */
	DISK_SPECIFIC 	*ds;	/* Pointer to device specific struct */
	int 		spl, spl1;	/* priority level */
	int 		error = 0;
#if     LABELS
	struct disklabel *lp;
#endif	/* LABELS */
	static u_char	module[] = "cdisk_ioctl";

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_DISK | CAMD_INOUT), 
	   ("[%d/%d/%d] cdisk_ioctl: entry dev=0x%x cmd=0x%x data=0x%x\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   dev, cmd, data));


	if((cmd != SRVC_REQUEST) && (minor(dev) != RAID_MINOR)){
	    /*
	     * Sanity check - this should never happen!
	     */
	    if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
		    CAM_ERROR(module, "No device struct", CAM_SOFTWARE,
			(CCB_HEADER *)NULL, dev, (u_char *)NULL);
		return(ENODEV);
	    }
	    PDRV_IPLSMP_LOCK( pd, LK_RETRY, spl);
	    /*
	     * Check to see if we have posted a fatal recovery error
	     */
	    if( (pd->pd_recovery_cnt > CDISK_REC_RETRY) || pd->pd_fatal_rec ){
	        PDRV_IPLSMP_UNLOCK( pd, spl ); /* restore to orignal spl */
		return(EIO);
	    }
	    dd = pd->pd_dev_desc;
	    ds = (DISK_SPECIFIC *)pd->pd_specific;

	    /* 
	     * Stall if recovery is happening
	     */

	    while(( pd->pd_flags & PD_REC_INPROG ) != 0 ){
	        PDRV_SMP_SLEEPUNLOCK( &pd->pd_flags, PZERO, pd );
	        PDRV_IPLSMP_LOCK( pd, LK_RETRY, spl1);
	    }
	    PDRV_IPLSMP_UNLOCK( pd, spl ); /* restore to orignal spl */
#if 	LABELS
	    lp = &ds->ds_label;
#endif	/* LABELS */
	}


	switch (cmd) {
	/* Start of 4.3-style disk label support */

#if	LABELS
	    case DIOCGDINFO:
		/* 
		 * If this disk doesn't have a label, pretend that we don't
		 * implement them.
		 */
		if (ds->ds_label.d_magic != DISKMAGIC)
			return(EINVAL);
		else
			*(struct disklabel *)data = *lp;
		break;

	    case DIOCGPART:
		if (lp->d_magic != DISKMAGIC)
			return(EINVAL);
		else {
			((struct partinfo *)data)->disklab = lp;
			((struct partinfo *)data)->part =
			    &lp->d_partitions[CD_GET_PARTITION(dev)];
		}
		break;

	    case DIOCSDINFO:
#if	SEC_PRIV
		if (!privileged(SEC_FILESYS, 0))
			return(EACCES);
#endif
		if ((flag & FWRITE) == 0)
			return(EBADF);
		else if ( pd->pd_flags & PD_OPENRAW )
			error = setdisklabel(lp, (struct disklabel *)data, 0);
		else
			error = setdisklabel(lp, (struct disklabel *)data, ds->ds_openpart);
		if(error != 0)
			return(error);
		break;

	    case DIOCWLABEL:
#if	SEC_PRIV
		if (!privileged(SEC_FILESYS, 0))
			return(EACCES);
#endif
		if ((flag & FWRITE) == 0)
			return(EBADF);
		else
			ds->ds_wlabel = *(int *)data;
		break;

	    case DIOCWDINFO:
		if ((flag & FWRITE) == 0)
			return(EBADF);
		else if ((error = setdisklabel(lp, (struct disklabel *)data,
		    	(pd->pd_flags & PD_OPENRAW) ?
			0 : ds->ds_openpart)) == 0) {
			int wlab;

			/* 
			 * Simulate opening partition 0 so write succeeds.
			 */
			ds->ds_openpart |= (1 << 0);
			wlab = ds->ds_wlabel;
			ds->ds_wlabel = 1;
			error = writedisklabel(dev, cdisk_strategy, lp);
			ds->ds_openpart = ds->ds_copenpart | ds->ds_bopenpart;
			ds->ds_wlabel = wlab;
		}
		if (error != 0)
			return(error);
		break;

	    case DIOCGDEFPT:
	    {
		struct pt_tbl *ptp = (struct pt_tbl *)data;
		struct pt_info *def_pt = dd->dd_def_partition;
 		int i,size;
 
		/*
		 * We walk through the default partition for this device
		 * and copy size and offset of each supported partition.
		 */
		for(i=0; i<8; i++)   {
		    size = def_pt[i].pi_nblocks;
		    if(size == -1)  {
		        size = ds->ds_tot_size;
		        size -= def_pt[i].pi_blkoff;
		    }
		    ptp->d_partitions[i].p_size = size;
		    ptp->d_partitions[i].p_offset = def_pt[i].pi_blkoff;
		}
		break;
	    }

	    case DIOCGCURPT:
	    {
		*(struct pt_tbl *)data = *(struct pt_tbl *)lp->d_partitions;
		break;
	    }

#endif	/* LABELS */

#ifndef OSF
	    /* 
	     * Return partition table to user 
	     */
	    case DIOCGETPT:
	    /* 
	     * Return default partition table
	     */
	    case DIOCDGTPT:
	    {
		struct pt *pt = (struct pt *)data;
		int i;

		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

		if (cmd == DIOCGETPT) {
		    struct pt *cur_pt = &ds->ds_pt;
		    /*
		     * Copy pt structure into user's data area.
		     */
		    *pt = *cur_pt;
		}
		else {
		    struct pt_info *def_pt = dd->dd_def_partition;
		    /*
		     * Copy the default partition table to user's data area.
		     */
		    for (i=0; i<CD_NUM_PARTITIONS; i++) {
			pt->pt_part[i].pi_nblocks = def_pt[i].pi_nblocks;
			pt->pt_part[i].pi_blkoff = def_pt[i].pi_blkoff;
		    }
		}

		PDRV_IPLSMP_UNLOCK(pd, spl);
		/*
		 * Change all -1 nblocks to disk unit size.
		 */
		for (i=0; i<CD_NUM_PARTITIONS; i++) {
		   if (pt->pt_part[i].pi_nblocks == -1)
			pt->pt_part[i].pi_nblocks =
			    ds->ds_tot_size - pt->pt_part[i].pi_blkoff;
		}
		pt->pt_magic = PT_MAGIC;
		break;

	    }
	    /* TODO1: what if user does this with open no delay? */
	    /* 
	     * Set the driver partition tables
	     */
	    case DIOCSETPT:
	    {
		struct pt *pt = (struct pt *)data;
		int error;
		/*
		 * Only super users can set the partition table.
		 */
#ifdef 	OSF
#if 	SEC_BASE
		if (!privileged(SEC_FILESYS, 0))
		    return(EACCES);
#else	/* SEC_BASE */
		if (suser(u.u_cred, &u.u_acflag))
		    return(EACCES);
#endif	/* SEC_BASE */

#else	/* OSF */
		if (!suser())
		    return(EACCES);
#endif	/* OSF */
		/*
		 * CD-ROMS are read only disks  
		 */
		if( ((ALL_INQ_DATA *)pd->pd_dev_inq)->dtype == DTYPE_RODIRECT )
		    return(EROFS);

		/*
		 * Before we set the new partition table make sure
		 * that it will not corrupt any of the kernel data
		 * structures.
		 */
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

		error = ptcmp(dev, &ds->ds_pt, pt);

		PDRV_IPLSMP_UNLOCK(pd, spl);

		if (error != 0)
		    return(error);

		/*
		 * Use the user's data to set the partition table.
		 */
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		ds->ds_pt = *pt;

		/*
		 * See if we need to update the superblock of the
		 * "a" partition of this disk.
		 */
		ssblk(dev, pt);

		PDRV_IPLSMP_UNLOCK(pd, spl);

		/*
		 * Just make sure that we set the valid bit.
		 */
		ds->ds_pt.pt_valid = PT_VALID;
		break;
	    }
#endif	/* ! OSF*/

	    /* 
	     * Device status
	     */
	    case DEVIOCGET:
	    {
		DIR_MODE_DATA6 *msen_data;
		DIR_READ_CAP_DATA *read_cap_data;
		ALL_INQ_DATA *inqp = (ALL_INQ_DATA *)pd->pd_dev_inq;
		struct devget 	*devget;
		struct device *device = (struct device *)NULL;
		struct controller *cont = (struct controller *)NULL;
		U32 nblocks;

		device = camdinfo[pd->pd_log_unit];
		if( device != (struct device *)NULL)
		        cont = camminfo[device->ctlr_num];
		devget = (struct devget *)data;
		bzero((caddr_t)devget,sizeof(struct devget));
		devget->category = DEV_DISK;
		devget->bus = DEV_SCSI;
		bcopy(DEV_SCSI_GEN, devget->interface, strlen(DEV_SCSI_GEN));
		bcopy(dd->dd_dev_name, devget->device, DEV_SIZE);
		if( cont != (struct controller *)NULL)
		    if( (int)cont->boot_slot == -1 ) {
			/* get the boot slot from the parent bus */
			devget->adpt_num = cont->bus_hd->slot;
		    } else {
			devget->adpt_num = cont->slot;
		    }
		else
			devget->adpt_num = 0;
		if( device != (struct device *)NULL) {
		   	devget->ctlr_num = device->ctlr_num;
			devget->rctlr_num = device->ctlr_hd->rctlr;
	  		devget->unit_num = device->logunit;
		}  else {
		   	devget->ctlr_num = DEV_BUS_ID(dev);
			devget->rctlr_num = 0;
	  		devget->unit_num = pd->pd_log_unit;
		}
		devget->nexus_num = 0;
		devget->bus_num = DEV_BUS_ID(dev);
		/* return physical unit number */
		devget->slave_num = DEV_UNIT(dev);
		bcopy("rz", devget->dev_name, 3);
		devget->soft_count = pd->pd_soft_err;
		devget->hard_count = pd->pd_hard_err;
		if( !(pd->pd_flags & PD_OFFLINE) )
			devget->stat = DEV_DONE;
		else
			devget->stat = DEV_OFFLINE;
		devget->category_stat = CD_GET_PARTITION(dev);
		/*
		 * Do a mode sense  to see if the device is
		 * write locked. - contained in mode param header.
		 * NOTE: RX23 fails if we don't ask for a page.
		 */
		msen_data = 
			(DIR_MODE_DATA6 *)ccmn_get_dbuf((U32)sizeof(DIR_MODE_DATA6));
		if( cdisk_mode_sense(pd, (u_char *)msen_data, DIR_PG_ERR_RECOV, 
			(U32)(sizeof(DIR_ERR_RECOV_PG) +
			CD_MODE_HEADER_DESC)) == 0)   {
			if (msen_data->sel_head.wp)
			    devget->stat |= DEV_WRTLCK;
		}
		ccmn_rel_dbuf((u_char *)msen_data, (U32)sizeof(DIR_MODE_DATA6));
		if( inqp->dtype == DTYPE_RODIRECT )  {
		    devget->stat |= DEV_RDONLY;
		    devget->category_stat |= DEV_MC_COUNT;
		    devget->category_stat |= 
		    		(ds->ds_media_changes << 16);
		    break;
		}

		/*
		 * We're done if non-removable.
		 */
		if( !inqp->rmb )
		    break;

		/*
		 * Issue the Read Capacity command to determine the 
		 * type of diskette inserted.
		 */
		read_cap_data = (DIR_READ_CAP_DATA *)ccmn_get_dbuf((U32)sizeof(DIR_READ_CAP_DATA));
		if( cdisk_read_capacity(pd, read_cap_data) == 0)	{
			BTOL(&read_cap_data->lbn3, nblocks);
		    	/*
		     	 * RDCAP returns the address of the last LBN.
		     	 * We must add one to get the number of LBNs.
			 */
			nblocks++;
		    	switch(nblocks) {
		    		case 5760:
					devget->category_stat |= DEV_3_ED2S;
					break;
		    		case 2880:
					devget->category_stat |= DEV_3_HD2S;
					break;
		    		case 1440:
					if(strcmp("RX33", dd->dd_dev_name) == 0)
			    		    devget->category_stat |= DEV_5_DD2S;
					else
			    		    devget->category_stat |= DEV_3_DD2S;
					break;
		    		case 2400:
					devget->category_stat |= DEV_5_HD2S;
					break;
		    		case 800:
					devget->category_stat |= DEV_5_DD1S;
					break;
		    		case 720:
					devget->category_stat |= DEV_5_LD2S;
					break;
		    		default:
					devget->category_stat |= DEV_X_XXXX;
					break;
		    	}	/* switch */
		}  else
		    devget->category_stat |= DEV_X_XXXX;

		ccmn_rel_dbuf((u_char *)read_cap_data, (U32)sizeof(DIR_READ_CAP_DATA));
		devget->category_stat |= DEV_MC_COUNT;
		devget->category_stat |= (ds->ds_media_changes << 16);
		break;
	    }

	    /* 
	     * Disk geometry info.
	     */
	    case DEVGETGEOM:
	    {
		DEVGEOMST *devgeom = (DEVGEOMST *)data;
		DIR_READ_CAP_DATA *read_cap_data;
		DIR_MODE_DATA6 *msen_data;
		DIR_FORMAT_PG *msen_fp;
		DIR_GEOM_PG *msen_gp;
		DIR_FLEXI_PG *msen_flp;
		ALL_INQ_DATA *inqp = (ALL_INQ_DATA *)pd->pd_dev_inq;
		U32 nblocks;

		/*
		 * CD-ROMs do not have geometry info.
		 */
		if( ((ALL_INQ_DATA *)pd->pd_dev_inq)->dtype == DTYPE_RODIRECT )
		    return(EIO);

		bzero((caddr_t)devgeom, sizeof(DEVGEOMST));

		if( inqp->rmb )
		    devgeom->geom_info.attributes |= DEVGEOM_REMOVE;

		/*
		 * HSX00 and HSX01 FIB RAID devices are flagged as having
		 * "dynamic geometry" because the geometry of the underlying
		 * device can change depending on the configuration of units.
		 */
		if ( dd->dd_flags & DEVGEOM_DYNAMIC != 0)
		    devgeom->geom_info.attributes |= DEVGEOM_DYNAMIC;

		/*
		 * Get disk size via read capacity command.
		 */
		read_cap_data = (DIR_READ_CAP_DATA *)
			ccmn_get_dbuf((U32)sizeof(DIR_READ_CAP_DATA));
		if( cdisk_read_capacity(pd, read_cap_data) == 0)	{
			BTOL(&read_cap_data->lbn3, nblocks);
		    	/*
		     	 * RDCAP returns the address of the last LBN.
		     	 * We must add one to get the number of LBNs.
			 */
		    	devgeom->geom_info.dev_size = ++nblocks;
			ccmn_rel_dbuf((u_char *)read_cap_data,
			    (U32)sizeof(DIR_READ_CAP_DATA));
		}  else  {
			ccmn_rel_dbuf((u_char *)read_cap_data,
			    (U32)sizeof(DIR_READ_CAP_DATA));
		    	return(EIO);
		}
		/*
		 * Get disk geometry from some combination of mode sense
		 * pages 3, 4, and 5. Normally 3 and 4 for hard disks,
		 * 5 for floppy disks. We get the current values.
		 */
		msen_data = (DIR_MODE_DATA6 *)
			ccmn_get_dbuf((U32)sizeof(DIR_MODE_DATA6));
		/*
		 * Get the Format Device Page (3).
		 */
		if( cdisk_mode_sense(pd, (u_char *)msen_data, DIR_PG_FORMAT,
			(U32)(sizeof(DIR_FORMAT_PG) +
			CD_MODE_HEADER_DESC)) == 0)   {
			msen_fp = (DIR_FORMAT_PG *)msen_data->page_data;
			if((msen_fp->pg_head.page_length >= CD_MIN_FORMAT_DATA)
			  && (msen_fp->pg_head.page_code == DIR_PG_FORMAT) ) {
				devgeom->geom_info.nsectors =
					((msen_fp->secs_track1 & 0xFF) << 8)  |
					(msen_fp->secs_track0 & 0xFF);
			}
		}
		bzero((caddr_t)msen_data, sizeof(DIR_MODE_DATA6 *));

		/*
		 * Get the Rigid Disk Drive Geometry Page (4).
		 */
		if( cdisk_mode_sense(pd, (u_char *)msen_data, DIR_PG_GEOM,
		   (U32)(sizeof(DIR_GEOM_PG) + CD_MODE_HEADER_DESC)) == 0) {
		   msen_gp = (DIR_GEOM_PG *)msen_data->page_data;
		   if((msen_fp->pg_head.page_length >= CD_MIN_GEOM_DATA)
		        && (msen_gp->pg_head.page_code == DIR_PG_GEOM) )   {
			devgeom->geom_info.ncylinders =
				((msen_gp->num_cyl2 & 0xFF) << 16)  |
				((msen_gp->num_cyl1 & 0xFF) << 8)  |
				(msen_gp->num_cyl0 & 0xFF);
			devgeom->geom_info.ntracks =
				(msen_gp->num_heads & 0xFF);
			}
		}
		bzero((caddr_t)msen_data, sizeof(DIR_MODE_DATA6 *));

		/*
		 * Get the Flexible Disk Page (5) for removable devices.
		 */
		if( inqp->rmb )	{
			if( cdisk_mode_sense(pd, (u_char *)msen_data,
				DIR_PG_FLEXI, (U32)sizeof(DIR_FLEXI_PG) +
				CD_MODE_HEADER_DESC) == 0)   {
				msen_flp = (DIR_FLEXI_PG *)msen_data->page_data;
				if((msen_fp->pg_head.page_length >= CD_MIN_FLEXI_DATA)
				   && (msen_flp->pg_head.page_code == DIR_PG_FLEXI) )   {
					devgeom->geom_info.nsectors =
						(msen_flp->sec_trk & 0xFF);
					devgeom->geom_info.ncylinders =
						((msen_flp->num_cyl1 & 0xFF) << 8)  |
						(msen_flp->num_cyl0 & 0xFF);
					devgeom->geom_info.ntracks =
						(msen_flp->num_heads & 0xFF);
				}
			}
		}

		/*
		 * Release the mode sense data buffer.
		 */
		ccmn_rel_dbuf((u_char *)msen_data, (U32)sizeof(DIR_MODE_DATA6));


		/* fail if no geometry info available (RRD40 does this) */
		if ((devgeom->geom_info.ntracks == 0) ||
		    (devgeom->geom_info.nsectors == 0) ||
		    (devgeom->geom_info.ncylinders == 0))
			return(EIO);

	    }
	    break;

	    case SRVC_REQUEST:	/* Service Request ie RAID */
	    {
		/* 
		 * Call the decoded to handle this request. Currently RAID only
		 */
		return( cdisk_srvc_req( dev, (SRVC_REQ *)data));
	    }
	    break;


	    default:
		/*
		 * All other commands (SCSI specific) will be handled
		 * by the special command interface.
		 */
		return(ccmn_DoSpecialCmd(dev, cmd, data, flag, 
					(CCB_SCSIIO *)NULL, 0));
	}

	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   (CAMD_DISK | CAMD_INOUT), ("[%d/%d/%d] cdisk_ioctl: exit\n",
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));

	return (CAM_SUCCESS);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_op_spin()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine is called at open to bring the drive online.  A Test 
 *	Unit Ready command is issued.  If the device comes back as Not Ready
 *	then we will issue a Start Unit command and reissue the Test
 *	Unit Ready command  We try for ready time seconds (contained in
 *	the device descriptor entry for the device) to get the drive
 *	ready. 
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to the peripheral device structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	EIO	- The maximum retry count for the Test Unit Ready
 *		  command was reached and the drive is still not ready.
 *	ENXIO	- Invalid CAM status.
 *		- Invalid sense key on check condition SCSI status.
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *	- Since the Test Unit Ready command is the first command issued to
 *	  the drive, we will initiate synchronous if supported.
 *	- For tagged devices, if a tagged Test Unit Ready command is 
 *	  rejected then the Test Unit Ready and Start Unit commands are
 *	  issued as non-tagged.  (For tagged devides the drive must
 *	  be spun up before operating in tagged mode.)
 *	- The media change counter is incremented for removable devices.
 *
 *  ADDITIONAL INFORMATION:
 *	The Test Unit Ready command will be retried:
 *		- for the categorized completion status values of
 *		  CAT_SCSI_BUSY, and CAT_DEVICE_ERR.
 *		- for the categorized completion status value of 
 *		  CAT_CMP_ERR and a sense key value of unit attention
 *		  or not ready
 *		- Invalid sense data.
 *		- SCSI status other than GOOD or Check Condition.
 *	The following will return an error:
 *		- All categorized completion values other than
 *		  CAT_SCSI_BUSY, CAT_CMP, CAT_DEVICE_ERR, or CAT_CMP_ERR.
 *		- for the categorized completion status value of 
 *		  CAT_CMP_ERR and sense keys other than unit attention,
 *		  not ready, and no sense.
 *		- Retry count is exhausted.
 *
 ************************************************************************/

static 
cdisk_op_spin(pd)
PDRV_DEVICE *pd;
{
	CCB_SCSIIO *tur_ccb;	/* Pointer to SCSI I/O ccb for */
				/* Test Unit Ready command */
	CCB_SCSIIO *start_ccb = (CCB_SCSIIO *)NULL;
				/* Pointer to SCSI I/O ccb for */
				/* Start Unit command */
	int 	    done = 0;	/* Indicates whether done */
	int 	    retry;	/* Retry counter */
	int 	    ret = 0;	/* Return value */
	int 	    spl;	/* Priority level */
	U32 	    cat;	/* CAM status category */
	U32 	    flags;	/* CAM flags */
	int	    ready_time;
	int	    start_unit_cnt = 0;
	static u_char	   module[] = "cdisk_op_spin";

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_op_spin: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	/*
	 * Only initiate synchronous on the very first open of the device.
	 * Make sure the device supports it.
	 */
	PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
	flags = pd->pd_cam_flags | CAM_DIR_NONE; 
	if (!(pd->pd_flags & PD_SYNCH))   {
	    /*
	     * Check whether we must not try to negotiate synchronous.
	     */
	    if( !(pd->pd_dev_desc->dd_flags & SZ_NOSYNC) )
		flags |= CAM_INITIATE_SYNC; 
	    pd->pd_flags |= PD_SYNCH;
	}
	PDRV_IPLSMP_UNLOCK(pd, spl);
		
	/*
	 * Create and send the Test Unit Ready CCB.
	 */
	tur_ccb = ccmn_tur(pd, GET_SENSE_SIZE(pd), flags,
		  cdisk_complete, pd->pd_tag_action, 
		  GET_TIMEOUT(pd));
		
	ready_time = pd->pd_dev_desc->dd_ready_time;
	if (ready_time <= 0)
		ready_time  = SZ_READY_DEF;

	for( retry=0; retry<ready_time; retry++ ) {

		if( retry != 0)	{
			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			CLEAR_CCB(tur_ccb);
			(void)cdisk_send_ccb_wait(pd, (CCB_HEADER *)tur_ccb, 
				RETRY, PZERO);
			PDRV_IPLSMP_UNLOCK(pd, spl);
		}

		CHK_RELEASE_QUEUE(pd, tur_ccb);	

		/*
		 * Special check for tagged devices.  If the tag was
 		 * rejected, reissue without tagging enabled.
		 */
		if( tur_ccb->cam_ch.cam_flags & CAM_QUEUE_ENABLE) {
		    if( (CAM_STATUS(tur_ccb) == CAM_MSG_REJECT_REC) ||
				(CAM_STATUS(tur_ccb) == CAM_UNEXP_BUSFREE) )  {
			tur_ccb->cam_ch.cam_flags &= ~CAM_QUEUE_ENABLE;
			continue;  /* retry the request */
		    }
		}
	  	cat = ccmn_ccb_status((CCB_HEADER *)tur_ccb);

	  	switch( cat ) {
		   case CAT_CMP:
			done = 1;
			break;		/* success */
		   case CAT_SCSI_BUSY:
		   case CAT_DEVICE_ERR:
			break;		/* retry */
		   case CAT_CMP_ERR:

			/*
			 * Now if it was because of a reservation_conflict,
			 * then we don't need to continue trying over and
			 * over again.  So, lets just exit now.
			 */
			if (tur_ccb->cam_scsi_status ==
					SCSI_STAT_RESERVATION_CONFLICT) {
			    PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
			    CAM_ERROR(module, "Unit Reserved", CAM_HARDERR,
				  (CCB_HEADER *)tur_ccb, pd->pd_dev,
				  (u_char *)NULL);
			    PDRV_IPLSMP_UNLOCK(pd, spl);
			    done = 1;
			    ret = EIO;
			    break;
			}
	   	  	if( !(tur_ccb->cam_ch.cam_status & CAM_AUTOSNS_VALID)
			  || CHECK_SENSE_RESID(tur_ccb) 
	  		  || (tur_ccb->cam_scsi_status != 
			      SCSI_STAT_CHECK_CONDITION) ) 
				break;	/* retry */

			/*
			 * Save sense data in the peripheral driver structure.
			 */
			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			pd->pd_sense_len = ((u_short)tur_ccb->cam_sense_len - 
				tur_ccb->cam_sense_resid);
			bcopy ((caddr_t)tur_ccb->cam_sense_ptr, 
				pd->pd_sense_ptr, pd->pd_sense_len );
			PDRV_IPLSMP_UNLOCK(pd, spl);

			switch (SENSEKEY(tur_ccb))   {
			   case ALL_NO_SENSE:
				done = 1;	/* success */
				break;
			   case ALL_UNIT_ATTEN:
			   {
				DISK_SPECIFIC *ds = (DISK_SPECIFIC *)pd->pd_specific;
				u_short code;
				PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
				pd->pd_flags &= ~PD_ONLINE;
		    		if( ((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb ){	
				    /*
				     * Check for a power down, reset, bdr.
				     */
				    ASCQ_TO_USHORT(tur_ccb->cam_sense_ptr, code);
				    if(code != ASCQ_PON_RESET)
				    	ds->ds_media_changes++;
				}
				PDRV_IPLSMP_UNLOCK(pd, spl);
				break;	/* reissue TUR cmd */
			   }
			   case ALL_NOT_READY:
				PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
				   CAMD_DISK,
				   ("[%d/%d/%d] cdisk_op_spin: Issue Start Unit cmd\n",
	   			   pd->pd_bus, pd->pd_target, pd->pd_lun));
				/*
				 * Check whether this is removable - if
				 * so check whether media is present.
				 */
				if( ((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb)  {
				   if( ((ALL_REQ_SNS_DATA *)
				       tur_ccb->cam_sense_ptr)->asc 
					== CD_NO_MEDIA) {
					done = 1;
					ret = EIO;
					break;
				   }
				}
			    	/*
				 * Issue the Start Unit command.
			    	 * Check whether this a retry of the
				 * start unit command.
			    	 */
				if( start_ccb == (CCB_SCSIIO *)NULL ) {
					start_ccb = ccmn_start_unit(pd,
					   GET_SENSE_SIZE(pd), 
					   tur_ccb->cam_ch.cam_flags, 
					   cdisk_complete, pd->pd_tag_action,
					   ready_time);
				}  else  {
					if(start_unit_cnt > CD_RETRY_START_UNIT)
					{
						done = 1;
						ret = EIO;
						break;
					}
					PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
					CLEAR_CCB(start_ccb);
					(void)cdisk_send_ccb_wait(pd, (CCB_HEADER *)
					    start_ccb, RETRY, PZERO);
					PDRV_IPLSMP_UNLOCK(pd, spl);
				}
				start_unit_cnt++;
				CHK_RELEASE_QUEUE(pd, start_ccb);	
				if(CAM_STATUS(start_ccb) != CAM_REQ_CMP)  {
					timeout(wakeup, &start_ccb, hz);
					sleep((caddr_t)&start_ccb, PZERO+1);
				}
				break;  /* reissue TUR cmd */
			   default:
				PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
				   (CAMD_DISK | CAMD_ERRORS),
				   ("[%d/%d/%d] cdisk_op_spin: Invalid sense key=0x%x\n",
	   			   pd->pd_bus, pd->pd_target, pd->pd_lun,
				   SENSEKEY(tur_ccb)));
				PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
				pd->pd_flags |= PD_NO_DEVICE;
				PDRV_IPLSMP_UNLOCK(pd, spl);
				done = 1;
				ret = ENXIO;
			}  /* switch */
			break;
	    
		   default:
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   (CAMD_DISK | CAMD_ERRORS),
			   ("[%d/%d/%d] cdisk_op_spin: Invalid cam status=0x%x\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun,
			   CAM_STATUS(tur_ccb)));
			done = 1;
			ret = ENXIO;
		}  /* switch */

		if( done ) 
			break;		/* break out of for loop */
		sleep(&lbolt, PZERO);

		/* 
		 * Schedule a wakeup for 1 sec later. Can't use
		 * lbolt because at boot time lbolt wakeups are not
		 * scheduled yet.
		 */
		timeout(wakeup, pd, hz);  
		sleep(pd, PZERO);


	} 	/* for */

	if (retry >= pd->pd_dev_desc->dd_ready_time)  {
		PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
		CAM_ERROR(module, "Device Not Ready", CAM_HARDERR,
			(CCB_HEADER *)tur_ccb, pd->pd_dev, (u_char *)NULL);
		PDRV_IPLSMP_UNLOCK(pd, spl);
		ret = EIO;
	}
	if(start_unit_cnt > CD_RETRY_START_UNIT) {
		if( ((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb == 0)  {
		   PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);
		   CAM_ERROR(module, "Start Unit failed", CAM_HARDERR,
			(CCB_HEADER *)start_ccb, pd->pd_dev, (u_char *)NULL);
		   PDRV_IPLSMP_UNLOCK(pd, spl);
		}
		ret = EIO;
	}
	/* 
	 * Remove from queue and release all allocated CCBs
	 */
	ccmn_rem_ccb(pd, tur_ccb);
	ccmn_rel_ccb((CCB_HEADER *)tur_ccb);
	if( start_ccb != (CCB_SCSIIO *)NULL )	{
		ccmn_rem_ccb(pd, start_ccb);
		ccmn_rel_ccb((CCB_HEADER *)start_ccb);
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_op_spin: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ret);
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_op_msel()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine will handle issuing the mode select pages contained
 *	in the mode select table in the device descriptor entry for the
 *	device on first open of a device.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to the peripheral device structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	EIO	- The maximum retry count has been reached in sending
 *		  one of the Mode Select pages.
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

static
cdisk_op_msel(pd)
PDRV_DEVICE *pd;
{
	MODESEL_TBL *mstp = pd->pd_dev_desc->dd_modesel_tbl;
				/* Pointer to mode select table */
	struct 	    ms_entry *msep;
				/* Pointer to mode select entries */
	CCB_SCSIIO  *ccb;	/* Pointer to SCSI I/O ccb */
	int 	    indx = 0;	/* Mode select table index */
	int 	    retry;	/* Retry counter */
	int 	    ret = 0;	/* Return value */
	int 	    spl;	/* Priority level */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_op_msel: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
	/*
	 * Check whether there are any pages to send.
	 */
	if( !mstp )
		return(0);
	if( mstp->ms_entry[indx].ms_data == (u_char *)NULL )
		return(0);

	/*
	 * Create a SCSI I/O CCB for the Mode Select command.
	 */
	ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)NULL,  (U32)0,
			GET_SENSE_SIZE(pd), 
			(pd->pd_cam_flags | CAM_DIR_OUT), 
			cdisk_complete, pd->pd_tag_action,
			GET_TIMEOUT(pd), (struct buf *)NULL);

	/*
	 * For each page entry in the Mode Select Table, send
	 * the MODE SELECT command.
	 */
	while( (mstp->ms_entry[indx].ms_data != (u_char *)NULL)
	   && (indx < MAX_OPEN_SELS) )  {
	   	msep = (struct ms_entry *)&mstp->ms_entry[indx];

		/*
		 * Fill in the specifics for the Mode Select command
		 */
		CCB_MODE_SEL(pd, ccb, msep);

		for(retry=0; retry<CD_RETRY_MODE_SEL; retry++)    {

			CLEAR_CCB(ccb);

			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			if( (retry == 0) && (indx == 0) )
			   (void)cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, 
				NOT_RETRY, PZERO);
			else
			   (void)cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, 
				RETRY, PZERO);
			PDRV_IPLSMP_UNLOCK(pd, spl);

			CHK_RELEASE_QUEUE(pd, ccb);

			/* Check if completed successfully */
	    		if( CAM_STATUS(ccb) == CAM_REQ_CMP )  {
				break;
			}
		}	/* for */


		if( retry == CD_RETRY_MODE_SEL )   {
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   (CAMD_DISK | CAMD_ERRORS),
			   ("[%d/%d/%d] cdisk_op_msel: Max Retry\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun));
			ret = EIO;
			break;
		}

		indx++;		/* get next entry in the mode select table */

	}	/* while */

	/* Remove the CCB from peripheral driver queue */
	ccmn_rem_ccb(pd, ccb);

	/* Return the CCB to the XPT */
	ccmn_rel_ccb((CCB_HEADER *)ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_op_msel: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ret);
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_media_remove()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine will issue the Prevent/Allow Medium Removal command.
 *	This rouitne is called on open to issue the prevent media
 *	removal command and on close to issue the allow media removal command.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to the peripheral device structure.
 *	cmd	- 0 = allow removal 1 = prevent removal.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	EIO	- The maximum retry count has been reached is sending
 *		  the Read Capacity command.
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

static
cdisk_media_remove(pd, cmd)
PDRV_DEVICE *pd;
u_short cmd;
{
	CCB_SCSIIO 	   *ccb;	/* Pointer to SCSI I/O ccb */
	DIR_PREVENT_CDB6   *cdb;	/* Pointer to Read Capacity CDB */
	int  	   	   ret=0;	/* Return value */
	int 		   spl;		/* Priority level */
	int 		   retry;	/* Retry counter */
	static u_char	   module[] = "cdisk_media_remove";

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_media_remove: entry cmd=0x%d\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun, cmd));

	/* Construct the CCB */
	ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)NULL, 
				(U32)0,
				GET_SENSE_SIZE(pd), 
				(pd->pd_cam_flags | CAM_DIR_NONE), 
				cdisk_complete, pd->pd_tag_action,
				GET_TIMEOUT(pd), (struct buf *)NULL);

	/* Fill in the CDB */
	cdb = (DIR_PREVENT_CDB6 *)ccb->cam_cdb_io.cam_cdb_bytes;
	cdb->opcode = DIR_PREVENT_OP;
	cdb->prevent = cmd;
	ccb->cam_cdb_len = sizeof(DIR_PREVENT_CDB6);

	for(retry=0; retry<CD_RETRY_IO; retry++)   {
		/* 
		 * Place the request on the queue and send to XPT.
		 */
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		if( !retry )
			(void)cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, 
				NOT_RETRY, PZERO);
		else  {
			CLEAR_CCB(ccb);
			(void)cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, 
				RETRY, PZERO);
		}
		PDRV_IPLSMP_UNLOCK(pd, spl);

		CHK_RELEASE_QUEUE(pd, ccb);

		if( CAM_STATUS(ccb) == CAM_REQ_CMP )
			break;
	}

	if( retry == CD_RETRY_IO) 	{
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		CAM_ERROR(module, "Prevent/Allow Removal Failed", CAM_HARDERR,
			(CCB_HEADER *)ccb, pd->pd_dev, (u_char *)NULL);
		PDRV_IPLSMP_UNLOCK(pd, spl);
		ret = EIO;
	}

	/* Remove the CCB from the peripheral driver queue. */
	ccmn_rem_ccb(pd, ccb);

	/* Return the CCB. */
	ccmn_rel_ccb((CCB_HEADER *)ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_media_remove: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ret);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_read_capacity()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine will issue the Read Capacity command to
 *	determine the size of the disk.
 *
 *  FORMAL PARAMETERS:
 *	pd	   - Pointer to the peripheral device structure.
 *	read_cap_data - Pointer for read capacity data.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	EIO	- The maximum retry count has been reached is sending
 *		  the Read Capacity command.
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

static
cdisk_read_capacity(pd, rdcp_data)
PDRV_DEVICE *pd;
DIR_READ_CAP_DATA *rdcp_data;
{
	CCB_SCSIIO 	   *ccb;	/* Pointer to SCSI I/O ccb */
	DIR_READ_CAP_CDB10 *cdb;	/* Pointer to Read Capacity CDB */
	int  	   	   ret=0;	/* Return value */
	int 		   spl;		/* Priority level */
	int 		   retry;	/* Retry counter */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_read_capacity: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	/* Construct the CCB */
	ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)rdcp_data, 
				(U32)sizeof(DIR_READ_CAP_DATA),
				GET_SENSE_SIZE(pd), 
				(pd->pd_cam_flags | CAM_DIR_IN), 
				cdisk_complete, pd->pd_tag_action,
				GET_IO_TIMEOUT(pd), (struct buf *)NULL);

	/* Fill in the CDB */
	cdb = (DIR_READ_CAP_CDB10 *)ccb->cam_cdb_io.cam_cdb_bytes;
	cdb->opcode = DIR_READCAP_OP;
	ccb->cam_cdb_len = sizeof(DIR_READ_CAP_CDB10);

	for(retry=0; retry<CD_RETRY_READ_CAP; retry++)   {
		/* 
		 * Place the request on the queue and send to XPT.
		 */
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		if( !retry )
			(void)cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb,
				NOT_RETRY, PZERO);
		else  {
			CLEAR_CCB(ccb);
			(void)cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, 
				RETRY, PZERO);
		}
		PDRV_IPLSMP_UNLOCK(pd, spl);

		CHK_RELEASE_QUEUE(pd, ccb);

		if( CAM_STATUS(ccb) == CAM_REQ_CMP )
			break;
	   	if( (CAM_STATUS(ccb) == CAM_REQ_CMP_ERR) &&
		   (ccb->cam_scsi_status == SCSI_STAT_CHECK_CONDITION)  &&
		   (SENSEKEY(ccb) == ALL_UNIT_ATTEN) )  {
		   u_short code;
		   /*
		    * Check for a power down, reset, or bus device reset.
		    */
		   ASCQ_TO_USHORT(ccb->cam_sense_ptr, code);
		   if(code != ASCQ_PON_RESET)
		      /*
		       * Increment media change counter for the cdrom and the
		       * floppy disk devices.
		       */
		      if(((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb) {
			((DISK_SPECIFIC *)pd->pd_specific)-> ds_media_changes++;
		      }
	   	}
	}

	if( retry == CD_RETRY_READ_CAP ) 	{
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
		   ("[%d/%d/%d] cdisk_read_capacity: Reached Max Retry status=0x%x\n", 
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun,
		   CAM_STATUS(ccb)));
		ret = EIO;
	}

	/* Remove the CCB from the peripheral driver queue. */
	ccmn_rem_ccb(pd, ccb);

	/* Return the CCB. */
	ccmn_rel_ccb((CCB_HEADER *)ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_read_capacity: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ret);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_op_find_device()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called on an open of a device which did not
 *	previously exist.  If neither a direct access or cd-rom device
 *	exists we will issue a rescan for this address and reissue the
 *	open for both device types.
 *
 *  FORMAL PARAMETERS:
 *	dev	- Major/minor number.
 *	type_open - Regular open Exclusive or Semi exclusive
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	ENXIO	- The rescan failed.
 *	0	- Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

static
cdisk_op_find_device(dev, type_open)
dev_t dev;
u_long type_open;
{
   int done = 0;		/* Indicates whether completed */
   I32 status;

   PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_DISK | CAMD_INOUT), 
   	("[%d/%d/%d] cdisk_op_find_device: entry\n", DEV_BUS_ID(dev),
   	DEV_TARGET(dev), DEV_LUN(dev)));

   do   {
	/*
	 * Check if we have a direct access disk device
	 */
	if( (status = ccmn_open_unit(dev, (U32)ALL_DTYPE_DIRECT,
	    (U32)type_open, (U32)sizeof(DISK_SPECIFIC))) != 0)   {
	    /* 
	     * Check whether the requested device is on a non existant 
	     * controller.
	     */
	    if(status == EFAULT)
		     return(ENXIO);
	    /* 
	     * Check whether it is a cdrom
	     */
	    if( (status = ccmn_open_unit(dev, (U32)ALL_DTYPE_RODIRECT,
	       	(U32)type_open, (U32)sizeof(DISK_SPECIFIC))) != 0)   {
		/*
		 * Check whether we have already done a scan.
		 */
		if (done)
		     return(status);
	        done++;
		/* 
		 * Lets issue a rescan of this address.
		 */
		if(ccfg_edtscan(EDT_SINGLESCAN, DEV_BUS_ID(dev),
	             DEV_TARGET(dev), DEV_LUN(dev)) != CAM_REQ_CMP) {
	             PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
			(CAMD_DISK | CAMD_ERRORS),
	   	        ("[%d/%d/%d] cdisk_op_find_device: ccfg_edtscan failed\n",
	                DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		     return(ENXIO);
		}
	    }
	}  /* if (status == ENXIO) */

   }  while (status != 0);

   PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_DISK | CAMD_INOUT), 
   	("[%d/%d/%d] cdisk_op_find_device: exit status=0x%x\n", DEV_BUS_ID(dev),
   	DEV_TARGET(dev), DEV_LUN(dev), status));

   return(status);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_mode_sense()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine will issue the Mode Sense command.
 *
 *  FORMAL PARAMETERS:
 *	pd		- Pointer to peripheral device structure
 *	msen_data	- Pointer to Mode Sense data area
 *	page_code	- Mode Sense page code
 *	nbytes		- Expected number of bytes for page
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	EIO	- Retry count is exhausted.
 *	0	- Success. 
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

static U32
cdisk_mode_sense(pd, msen_data, page_code, nbytes)
PDRV_DEVICE *pd;	/* Pointer to peripheral device structure */
u_char *msen_data;	/* Pointer to Mode Sense data area */
u_char page_code;	/* Mode Sense page code */
U32 nbytes;		/* Expected number of bytes for page */
{
	CCB_SCSIIO 	    *ccb;	/* Pointer to SCSI I/O ccb */
	ALL_MODE_SENSE_CDB6 *cdb;	/* Pointer to mode sense cdb */
	int 		    spl;	/* Priority level */
	int 		    retry;	/* Retry counter */
	U32 		    ret = 0;	/* Return value */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_mode_sense: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	/* Construct the CCB */
	ccb = ccmn_io_ccb_bld(pd->pd_dev, msen_data, 
			nbytes, GET_SENSE_SIZE(pd), 
			(pd->pd_cam_flags | CAM_DIR_IN), cdisk_complete,
			pd->pd_tag_action, GET_TIMEOUT(pd), (struct buf *)NULL);

	/* Construct the CDB */
	cdb = (ALL_MODE_SENSE_CDB6 *)ccb->cam_cdb_io.cam_cdb_bytes;
	cdb->opcode = ALL_MODE_SENSE6_OP;
	cdb->page_code = page_code;
	cdb->alloc_len = nbytes;
	ccb->cam_cdb_len = sizeof(ALL_MODE_SENSE_CDB6);

	for(retry=0; retry<CD_RETRY_MODE_SEN; retry++)    {

	   PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	   if( !retry )
		(void) cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, NOT_RETRY,
			PZERO);
	   else	{
		CLEAR_CCB(ccb);
		(void) cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, RETRY, PZERO);
	   }
	   PDRV_IPLSMP_UNLOCK(pd, spl);

	   CHK_RELEASE_QUEUE(pd, ccb);

    	   if( CAM_STATUS(ccb) == CAM_REQ_CMP )
		break;
	}  /* for(retry=0; retry<CD_RETRY_MODE_SEN; retry++)  */ 

	if( retry == CD_RETRY_MODE_SEN ) 	{
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		  (CAMD_DISK | CAMD_ERRORS),
		   ("[%d/%d/%d] cdisk_mode_sense: Reached Max Retry status=0x%x\n", 
	      	   pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));
		ret = EIO;
	}

	/* Remove request from peripheral driver queue */
	ccmn_rem_ccb(pd, ccb);

	/* Release the CCB */
	ccmn_rel_ccb((CCB_HEADER *)ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_mode_sense: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(ret);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_complete()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This is the callback completion function for all R/W SCSI I/O CCBs
 *	which are created by this driver which are not involved in BBR
 *	or error recovery.
 *	- If no buf structure pointer (bp) exists then we issue a
 *	  wakeup.
 *	- If the request completed successfully, then we save
 *	  statistical info and call biodone.
 *	- If the cam status indicates busy or reset then we will start
 *	  the recovery process.
 *	- All other CAM status values will result in a retry of the
 *	  request.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to completed SCSI I/O CCB.
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
cdisk_complete(ccb)
CCB_SCSIIO *ccb;	/* pointer to completed CCB */
{
	PDRV_WS 	*pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
				/* Peripheral working set pointer */
	PDRV_DEVICE 	*pd = (PDRV_DEVICE *)pws->pws_pdrv;
				/* Pointer to peripheral device structure */
	DISK_SPECIFIC 	*ds;	/* Pointer to disk specific struct */
	struct buf 	*bp;	/* Pointer to buf struct */
	U32 		cat;	/* CAM status category */
	int 		spl;	/* Priority level */
	caddr_t		cdb;	/* Pointer to the cdb (RAID) */
	u_short 	action;	/* action to be taken for request */
	int		xfer_count;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_complete: entry status=0x%x\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));

	/*
	 * If the ccb did not complete successfully, call the kernel error 
	 * event notification function to indicate the error to anyone 
	 * regsitered for error notification on this device. The registered
	 * process will determine whether this is an error that it cares about.
	 */
	if(CAM_STATUS(ccb) != CAM_REQ_CMP)   {
		event_notify(pd->pd_dev, ccb);
	}

	action = CD_IO_RETRY;

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	/*
	 * Indicate that the ccb has been received 
	 */
	pws->pws_flags |= PWS_CALLBACK;

	/* 
	 * Has this been marked as a total failure if so 
	 * notify the process
	 */
	if(pws->pws_flags & PWS_FATAL_ERR){
	    PDRV_IPLSMP_UNLOCK(pd, spl);
	}
	/*
	 * If recovering, call the recovery handler which will mark the
	 * request as retryable.
	 */
	else if( pd->pd_flags & PD_REC_INPROG )   {
	   	PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
   		   ("[%d/%d/%d] cdisk_complete: Recovery in Progress ccb=0x%x\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, ccb));
		if(pd->pd_recov_hand == (void *)NULL) {
			pws->pws_flags |= PWS_RETRY;
	   		PDRV_IPLSMP_UNLOCK(pd, spl);
		}  else  {
			pws->pws_flags |= PWS_RETRY;
	   		PDRV_IPLSMP_UNLOCK(pd, spl);
			pd->pd_recov_hand(pd, ccb);
		}
		CHK_RELEASE_QUEUE(pd, ccb);
		return;
	} else
	   	PDRV_IPLSMP_UNLOCK(pd, spl);

	/*
	 * Check whether we have a bp pointer. If not we are sleeping so
	 * we must issue a wakeup.
	 */
	if( !ccb->cam_req_map )	{
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK
		   | CAMD_FLOW),("[%d/%d/%d] cdisk_complete: call wakeup ccb=0x%x\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, ccb));
		wakeup(ccb);
		return;
	}
	ds = (DISK_SPECIFIC *)pd->pd_specific;

	if( pws->pws_flags & PWS_FATAL_ERR){
	    /* 
	     * Set the action so we fail it
	     */
	    action = CD_IO_FAILURE;
	}
	else {
	    cat = ccmn_ccb_status((CCB_HEADER *)ccb);

	    switch( cat )	{
		case CAT_CMP:
			/*
			 * We assume the residual count should be zero
			 * in order for the request to have completed
			 * successfully.
			 */
			if( ccb->cam_resid != 0 )  {
			   action = CD_IO_FAILURE;
			   PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
				(CAMD_DISK | CAMD_ERRORS),
			      	("[%d/%d/%d] cdisk_complete: bad resid\n",
	   			pd->pd_bus, pd->pd_target, pd->pd_lun));
			} else
			   action = CD_IO_DONE;
			break;
		case CAT_DEVICE_ERR:
			/*
			 * This category will simply default to a retry
			 * except when running tagged.
			 * If a power cycle occurrs the next request
			 * will fail  due to rejecting the tag
			 * message so if it was running tagged 
			 * then we know we have a reset.
			 */
			if( !(ccb->cam_ch.cam_flags & CAM_QUEUE_ENABLE))
				break;
			if( (ccb->cam_ch.cam_flags & CAM_QUEUE_ENABLE)
			    && (CAM_STATUS(ccb) != CAM_MSG_REJECT_REC)
			    && (CAM_STATUS(ccb) != CAM_UNEXP_BUSFREE))
				break;
			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			/*
			 * Indicate that this request should be retried later.
			 */
			pws->pws_flags |= PWS_RETRY;
			/*
			 * Indicate recovery is in progress.
		 	 */
			pd->pd_flags |= PD_REC_INPROG;
			/*
			 * Check if this was the only active CCB and call the
			 * recovery start routine.  Otherwise abort all 
			 * outstanding requests on this device.
		 	 */
			if( pd->pd_active_ccb == 1 )   {
				PDRV_IPLSMP_UNLOCK(pd, spl);
				cdisk_recovery(pd);
			} else {
				pd->pd_recov_hand = cdisk_atn_hand;
				pd->pd_abort_cnt = ccmn_abort_que(pd);
				PDRV_IPLSMP_UNLOCK(pd, spl);
			}
			action = CD_IO_CONT;
			break;
		/*
		 * Check whether a reset is occurring.
	 	 */
		case CAT_BUSY:
		case CAT_RESET:
			/*
			 * Indicate recovery started and set reset recovery
			 * handler which will simply mark the CCB
			 * as retryable. - The asynch callback will
			 * actually start the recovery process.
			 */
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   (CAMD_DISK | CAMD_ERRORS),
			   ("[%d/%d/%d] cdisk_complete: Reset In progress\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun));
			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			pd->pd_recov_hand = cdisk_reset_hand;
			pd->pd_flags |= PD_REC_INPROG;
			PDRV_IPLSMP_UNLOCK(pd, spl);
			cdisk_reset_hand(pd, ccb);
			action = CD_IO_CONT;
			break;
		case CAT_CMP_ERR:
			if (ccb->cam_scsi_status == SCSI_STAT_CHECK_CONDITION) {
				action = cdisk_check_sense(pd, ds, ccb);
				break;
			}
			if (ccb->cam_scsi_status == SCSI_STAT_BUSY) {
			    if( pws->pws_retry_cnt < CD_RETRY_IO )  {
				timeout(cdisk_ccb_retry, (caddr_t)ccb, hz);
				action = CD_IO_CONT;
				break;
			    }  else  {
				action = CD_IO_FAILURE;
				break;
			    }
				
			}
			/* fall thru */
		default:
			action = CD_IO_RETRY;
			break;
	    }	/* switch */
	} /* end of else */
	/*
	 * For hsz10's and 15's if write cmd must change to
	 * write verify parity check
	 */
	cdb = (caddr_t )((CCB_SCSIIO *)ccb)->cam_cdb_io.cam_cdb_bytes;

	/*
	 * Leave the request on the peripheral driver queue - it will be
	 * handled at a later time.
	 */
	if( action == CD_IO_CONT) {
		if((cdb[0] == DIR_WRITE6_OP) || (cdb[0] ==  DIR_WRITE10_OP)){  
		    cdisk_raid_wrt_verify(ccb);
		}
		CHK_RELEASE_QUEUE(pd, ccb);
		return;
	}

	if( action == CD_IO_RETRY)   {
		if((cdb[0] == DIR_WRITE6_OP) || (cdb[0] ==  DIR_WRITE10_OP) ||
			(cdb[0] == DIR_WRITE_VRFY_10_OP)){
		    cdisk_raid_wrt_verify(ccb);
		}
		if( pws->pws_retry_cnt >= CD_RETRY_IO )  {
			action = CD_IO_FAILURE;
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   (CAMD_DISK | CAMD_ERRORS),
			   ("[%d/%d/%d] cdisk_complete: Reached Retry count\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun));
		}  else	{
			U32 	status;
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   (CAMD_DISK | CAMD_ERRORS),
   			   ("[%d/%d/%d] cdisk_complete: Retry request ccb=0x%x status=0x%x\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   ccb, CAM_STATUS(ccb)));
			CHK_RELEASE_QUEUE(pd, ccb);
			CLEAR_CCB(ccb);
			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			pws->pws_retry_cnt++;
			status = cdisk_send_ccb(pd, (CCB_HEADER *)ccb, RETRY);
			PDRV_IPLSMP_UNLOCK(pd, spl);
			if( status != CAM_REQ_INPROG )
				ccb->cam_cbfcnp(ccb);
			return;
		}
	}

	/* Remove the request from the peripheral driver queue. */
	ccmn_rem_ccb(pd, ccb);

	/* Fill in the buf structure. */
	bp = (struct buf *)ccb->cam_req_map;
	if( action == CD_IO_FAILURE )   {
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		/*
		 * Check whether this was an EOP truncated transfer 
		 * (b_resid != 0). If so b_resid is already set up, just 
		 * put b_bcount back to what it was.
		 */
		if(bp->b_resid)
			bp->b_bcount = bp->b_resid;
		else
			bp->b_resid = bp->b_bcount;
		/* need to handle partial transfers */
	} else {
		xfer_count = bp->b_bcount - ccb->cam_resid;
		/*
		 * Check whether this was an EOP truncated transfer.
		 */
		if(bp->b_resid) {
			bp->b_bcount = bp->b_resid; /* get orig count */
			bp->b_resid -= xfer_count;
		}  else  {
			bp->b_resid = ccb->cam_resid;
		}
	}

	/* The queue should not be frozen here! */
	CHK_RELEASE_QUEUE(pd, ccb);

	/* 
	 * Check whether this transfer was scatter/gather and
	 * deallocate all resources.
	 */
	if( ccb->cam_ch.cam_flags & CAM_SCATTER_VALID)   {
		SG_ELEM *sg;
		sg = (SG_ELEM *)ccb->cam_data_ptr;
		sg++;
		ccmn_rel_dbuf(sg->cam_sg_address, ds->ds_block_size);
		ccmn_rel_dbuf(ccb->cam_data_ptr, (U32)(sizeof(SG_ELEM) * 2));
	}

	/* Return the CCB to the XPT */
	ccmn_rel_ccb((CCB_HEADER *)ccb);

	/*
	 * Save statistical info.
	 */
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	if( action != CD_IO_FAILURE )   {
	   if (ds->ds_dkn >= 0)  {
		dk_xfer[ds->ds_dkn]++;
		dk_wds[ds->ds_dkn] += xfer_count >> 6;
	   }
	   if(bp->b_flags & B_READ)  {
		pd->pd_read_count++;
		pd->pd_read_bytes += xfer_count;
	   }  else  {
		pd->pd_write_count++;
		pd->pd_write_bytes += xfer_count;
	   }
	}
	PDRV_IPLSMP_UNLOCK(pd, spl);

	biodone(bp);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_complete: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_check_sense()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will examine sense data for read and write
 *	requests and return the next action which should be taken for
 *	the request.  It will start the BBR process for recoverable errors.
 *	It will also start the recovery process for the NOT_READY and
 *	UNIT_ATTENTION sense key values with the proper sense codes.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure
 *	ds	- Pointer to disk specific structure
 *	ccb	- Pointer to completed CCB
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *	The sense data is saved in the peripheral device structure.
 *
 *  RETURN VALUE:
 *	CD_IO_DONE	- Indicates the request has completed.
 *		  (No Sense sense key).
 *	CD_IO_CONT	- Indicates the request is still being processed.
 *		  (Not Ready and Unit Attention sense keys - recovery 
 *		  will be started).
 *	CD_IO_RETRY  - Indicates the request should be retried.
 *		    (Command Aborted sense key value).
 *	CD_IO_FAILURE - Indicates the request has failed.
 *		     (All other sense keys and invalid or insufficient
 *		     sense data).
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	- The hard error counters are incremented for invalid and
 *	  insufficient auto sense data and aborted requests.
 *	- The soft error counter is incremented for recovered errors.
 *	- The media change counter is incremented for removable devices
 *	  which have a Unit Attention sense key which is not a result of
 *	  reset or power up.
 *
 ************************************************************************/

static u_short
cdisk_check_sense(pd, ds, ccb)
PDRV_DEVICE *pd;	/* Pointer to peripheral device structure */
DISK_SPECIFIC *ds;	/* Pointer to disk specific structure */
CCB_SCSIIO *ccb;	/* Pointer to completed CCB */
{
	int	spl;	/* Priority level */
	u_short status;	/* Return value */
	ALL_INQ_DATA *inq = (ALL_INQ_DATA *)pd->pd_dev_inq;
	static u_char	module[] = "cdisk_check_sense";

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_check_sense: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	/*	
	 * If the sense data is not valid then presume hard error.
	 */
	if ( !(ccb->cam_ch.cam_status & CAM_AUTOSNS_VALID) )  {
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
     		   ("[%d/%d/%d] cdisk_check_sense: Invalid sense data - HARD ERR\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		pd->pd_hard_err++;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		return(CD_IO_FAILURE);
	}

	/*	
	 * If we didn't get enough sense data then presume hard error.
	 */
	if( CHECK_SENSE_RESID(ccb) )  {
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
     		   ("[%d/%d/%d] cdisk_check_sense: Not enough sense data - HARD ERR\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		pd->pd_hard_err++;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		return(CD_IO_FAILURE);
	}

	/*
	 * Save sense data in the peripheral driver structure.
	 */
	pd->pd_sense_len = ((u_short)ccb->cam_sense_len - ccb->cam_sense_resid);
	bcopy ((caddr_t)ccb->cam_sense_ptr, pd->pd_sense_ptr,
		pd->pd_sense_len );
	PDRV_IPLSMP_UNLOCK(pd, spl);

	switch( SENSEKEY(ccb) )   {
	   case ALL_NO_SENSE:
		status = CD_IO_DONE;
		break;
	   case ALL_ABORTED_CMD:
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		pd->pd_hard_err++;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		status = CD_IO_RETRY;
		break;
	   case ALL_UNIT_ATTEN:
	   {
		u_short code;
		/*
		 * Check for a power down, reset, or bus device reset.
		 */
		ASCQ_TO_USHORT(ccb->cam_sense_ptr, code);
		if(code != ASCQ_PON_RESET)
		    /*
		     * Increment media change counter for the cdrom and the
		     * floppy disk devices.
		     */
		    if( inq->rmb )	{
			ds->ds_media_changes++;
			status = CD_IO_FAILURE;
			break;
		    }
		/*
		 * Otherwise we start the recovery process. - fall thru.
		 */
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
		   ("[%d/%d/%d/] cdisk_check_sense: UNIT ATN Start Recovery\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
	   }
	   case ALL_NOT_READY:
	   {
		PDRV_WS *pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
		u_short code;

		/*
		 * Do not start recovery on removable devices unless
		 * the asc = 0x29 which indicates power on, reset or bus
		 * device reset.
		 */
		if(SENSEKEY(ccb) == ALL_NOT_READY)  {
		   ALL_INQ_DATA *inqp = (ALL_INQ_DATA *)pd->pd_dev_inq;
		   ASCQ_TO_USHORT(ccb->cam_sense_ptr, code);
		   if(code != ASCQ_PON_RESET)
		      if( inqp->rmb ) {
		   	  status = CD_IO_FAILURE;
			  break;
		      } else  {
		      	  PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		      	  (CAMD_DISK | CAMD_ERRORS),
		      	  ("[%d/%d/%d/] cdisk_check_sense: Start Recovery\n",
	   	      	  pd->pd_bus, pd->pd_target, pd->pd_lun));
		      }
		}

		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		/*
		 * Indicate that this request should be retried later.
		 */
		pws->pws_flags |= PWS_RETRY;
		/*
		 * Indicate recovery is in progress.
		 */
		pd->pd_flags |= PD_REC_INPROG;
		/*
		 * Check if this was the only active CCB and call the
		 * recovery start routine.  Otherwise abort all 
		 * outstanding requests on this device.
		 */
		if( pd->pd_active_ccb == 1 )   {
			PDRV_IPLSMP_UNLOCK(pd, spl);
			cdisk_recovery(pd);
		} else {
			pd->pd_recov_hand = cdisk_atn_hand;
			pd->pd_abort_cnt = ccmn_abort_que(pd);
			PDRV_IPLSMP_UNLOCK(pd, spl);
		}
		status = CD_IO_CONT;
		break;
	   }
	   case ALL_RECOVER_ERR:
		/*
		 * Increment the soft error counter and start BBR processing.
		 */
		pd->pd_soft_err++;
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_INOUT),
		   ("[%d/%d/%d] cdisk_check_sense:SOFT ERROR - BBR\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		if(inq->dtype == DTYPE_RODIRECT)  {
		   status = CD_IO_DONE;
		   break;
		}
		cdisk_bbr_initiate(ccb);
		status = CD_IO_CONT;
		break;
	   case ALL_HARDWARE_ERR:
	   case ALL_MEDIUM_ERR:
   	   {
		U32 	bad_block;
		static u_char	log_str[100];
		static u_char	bad_block_str[20];

		/*
		 * Reassign the block on writes for non-removables.
		 */
		if( (inq->rmb == 0)
		   && (ccb->cam_ch.cam_flags & CAM_DIR_OUT))   {
			cdisk_bbr_initiate(ccb);
			status = CD_IO_CONT;
			break;
		}
       		BTOL(&((ALL_REQ_SNS_DATA *)(ccb->cam_sense_ptr))->info_byte3, 
		    bad_block);
		itoa(bad_block, bad_block_str);
		if( SENSEKEY(ccb) == ALL_HARDWARE_ERR)  
		    strcpy(&log_str[0], " Hardware Error bad block number: ");
		else
		    strcpy(&log_str[0], " Medium Error bad block number: ");
		strcpy(&log_str[strlen(log_str)], bad_block_str); 
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		pd->pd_hard_err++;
		CAM_ERROR(module, log_str, CAM_HARDERR, (CCB_HEADER *)ccb, 
			pd->pd_dev, (u_char *)NULL);
		PDRV_IPLSMP_UNLOCK(pd, spl);
		status = CD_IO_FAILURE;
		break;
	   }
	   case ALL_BLANK_CHECK:
	   case ALL_ILLEGAL_REQ:
	   {
		u_short code;
		/*
		 * If the error is "Illegal Mode for this Track" (0x64, 0x00),
		 * then don't log this error.  This is caused by open() trying
		 * to read the partition table on an audio disk.
		 */
		if( ((ALL_INQ_DATA *)pd->pd_dev_inq)->dtype 
		   == ALL_DTYPE_RODIRECT)  {
		   ASCQ_TO_USHORT(ccb->cam_sense_ptr, code);
		   if(code == ASCQ_ILL_MODE_TRK)  {
		   	status = CD_IO_FAILURE;
		   	break;
		   }
		}
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
   		   ("[%d/%d/%d] cdisk_check_sense: Blank Check or Illegal Request\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));

		/* fall thru to default case */
	   }
	   default:
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		pd->pd_hard_err++;
		CAM_ERROR(module, "Hard Error\n",
			CAM_HARDERR, (CCB_HEADER *)ccb, pd->pd_dev, 
			(u_char *)NULL);
		PDRV_IPLSMP_UNLOCK(pd, spl);
		status = CD_IO_FAILURE;
		break;
	}	/* switch */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_check_sense: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	return(status);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_ccb_retry()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called to handle resending ccbs which
 *	must be retried after a timeout usually due to a BUSY SCSI 
 *	status being returned from the device.  This is called from the
 *	timeout function.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O ccb to be retried.
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

static void
cdisk_ccb_retry(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_DEVICE 	*pd;		/* Pointer to peripheral struct */
	PDRV_WS 	*pws;		/* Pointer to working set */
	int 		spl;		/* Priority level */
	U32 		status;		/* XPT return status */

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK| CAMD_INOUT),
		("[%d/%d/%d] cdisk_ccb_retry: enter\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));

	CLEAR_CCB(ccb);

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	pws->pws_retry_cnt++;
	status = cdisk_send_ccb(pd, (CCB_HEADER *)ccb, RETRY);

	PDRV_IPLSMP_UNLOCK(pd, spl);

	if( status != CAM_REQ_INPROG )
		ccb->cam_cbfcnp(ccb);

	/* We know the queue is frozen here */
	RELEASE_QUEUE(pd);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK | CAMD_INOUT),
		("[%d/%d/%d] cdisk_ccb_retry: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_async_cb()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handle all asynchronous callbacks for which 
 *	this driver has registered.  The recovery process is started 
 *	on all resets.
 *
 *  FORMAL PARAMETERS:
 *	opcode	- Asynchrnous callbcak indication
 *	pathid	- Bus number
 *	target	- Target id
 *	lun	- LUN
 *	buffer	- Peripheral buffer pointer
 *	data_cnt- Length of peripheral buffer
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
cdisk_async_cb(opcode, pathid, target, lun, buffer, data_cnt)
U32 opcode;		/* Asynchrnous callbcak indication */
U32 pathid;		/* Bus number */
U32 target;		/* Target id */
U32 lun;		/* LUN */
caddr_t buffer;		/* Periperal buffer pointer */
U32 data_cnt;	/* Length of peripheral buffer */
{
	PDRV_DEVICE *pd;	/* Pointer to periheral device struct */
	int 	    spl;	/* Priority level */
	dev_t       dev;	/* major/minor number */


	dev = makedev(0, MAKEMINOR(MAKE_UNIT(pathid, target, lun), 0));

	/*
	 * Sanity check - this should never happen!
	 */
	if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL )  {
		PRINTD(pathid, target, lun, (CAMD_DISK | CAMD_ERRORS), 
   		  ("[%d/%d/%d] cdisk_async_cb: No peripheral device struct\n",
		   pathid, target, lun));
		return;
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_async_cb: entry opcode = 0x%x\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun, opcode));

	switch( opcode )   {

		case AC_BUS_RESET:
		case AC_SENT_BDR:
			/*
			 * Start the recovery process for this device
			 */
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
			   (CAMD_DISK_REC | CAMD_ERRORS), 
   		  	   ("[%d/%d/%d] cdisk_async_cb: Start Recovery\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun));
			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			pd->pd_flags |= PD_REC_INPROG;
			PDRV_IPLSMP_UNLOCK(pd, spl);
			cdisk_recovery(pd);
			break;

		case AC_SCSI_AEN:
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
			   (CAMD_DISK_REC | CAMD_ERRORS), 
   		  	   ("[%d/%d/%d] cdisk_async_cb: AEN\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun));
			break;

		default:
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
			   (CAMD_DISK_REC | CAMD_ERRORS), 
   	   		   ("[%d/%d/%d] cdisk_async_cb: Unknown opcode = 0x%x\n",
	    		   pd->pd_bus, pd->pd_target, pd->pd_lun, opcode));
			break;
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_async_cb: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_reset_hand()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called before the recovery process is started
 *	for CCBs which are returned with a CAM status of Bus Reset or 
 *	Bus Device Reset.  The first CCB which is returned will cause the
 *	PD_REC_INPROG bit to be set in pd_flags of the peripeheral device
 *	structure and set the pd_recov_hand to be this routine.  Therefore all
 *	subsequent CCBs returned will cause this routine to be called until
 *	the asynch callback routine is called indicating all CCBs have been
 *	returned after the reset and recovery may begin.
 *
 *	This routines only function is to mark the CCB as retryable.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure.
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_reset_hand(pd, ccb)
PDRV_DEVICE *pd;
CCB_SCSIIO *ccb;
{
	PDRV_WS *pws = (PDRV_WS *)ccb->cam_pdrv_ptr;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_reset_hand: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	pws->pws_flags |= PWS_RETRY;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_reset_hand: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_atn_hand()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	For read and write requests which return a Not Ready or Unit 
 *	Attention sense key, we will calculate the number of request on
 *	the peripheral driver queue which need to be aborted.  This routine will
 *	be called as these aborted requests return to the driver.  It will set
 *	the retry bit for the request and decrement the abort count. 
 *	If all requests have been returned, we will start the recovery
 *	process by calling cdisk_recovery.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure.
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_atn_hand(pd, ccb)
PDRV_DEVICE *pd;
CCB_SCSIIO *ccb;
{
	PDRV_WS	*pws;	/* Pointer to working set */
	int 	spl;	/* Priority level */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_atn_hand: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	pws->pws_flags |= PWS_RETRY;
	pd->pd_abort_cnt--;
	/*
	 * Check whether all requests have been aborted - so that
	 * we may start the recovery process.
	 */
	if( pd->pd_abort_cnt == 0 )   {
		PDRV_IPLSMP_UNLOCK(pd, spl);
		cdisk_recovery(pd);
		return;
	}
	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DISK | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_atn_hand: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_bbr_initiate()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine will either initiate a BBR operation or queue the
 *	operation for a later time.  Since the BBR state machine is single
 *	threaded, we can only do one BBR per unit at a time.  This routine
 *	will add the currently requested BBR onto the bbr_wait queue for
 *	later processing if a BBR is currently in process.
 *
 *	When BBR processing completes a request in cdisk_bbr_done, the
 *	first entry on the queue will be removed and started.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
 *
 *  IMPLICIT INPUTS:
 *	pws	- The peripheral driver working set
 *	pd	- The peripheral device structure
 *	dsp	- The disk specific structure
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

static void
cdisk_bbr_initiate(ccb)
CCB_SCSIIO *ccb;	/* Pointer to completed CCB */
{
	PDRV_DEVICE 	*pd;		/* Pointer to Peripheral device struc */
	PDRV_WS 	*pws;		/* CCB working set pointer */
	PDRV_WS 	*next_wait;	/* Next BBR waiting working set */
	DISK_SPECIFIC 	*dsp;		/* Pointer to disk specific structure */
	int 		spl;		/* Priority level */

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd  = (PDRV_DEVICE *)pws->pws_pdrv;
	dsp = (DISK_SPECIFIC *)pd->pd_specific;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_bbr: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

        /*
         * SMP lock the peripheral device structure.
         */
        PDRV_IPLSMP_LOCK(pd, LK_RETRY, spl);

	/*
	 * Add this request to the end of the bbr wait queue.  We must also
	 * leave it on the active queue in case a reset of any kind happens.
	 * Reset will need to re-start all I/Os on the active queue.
	 */
	next_wait = dsp->ds_bbr_wait_queue;
	if (next_wait) {
	    for (;;) {
		if (next_wait->wait_flink == 0) {
		    /* when finally at the end, put this one there */
		    next_wait->wait_flink = pws;
		    break;
		}
		/* we're not at the end yet, so keep following the links */
		next_wait = next_wait->wait_flink;
	    }
	} else {
	    /* The queue is empty, so make this the first in the queue */
	    dsp->ds_bbr_wait_queue = pws;
	}

	PDRV_IPLSMP_UNLOCK(pd, spl);

	/*
	 * Placement of items in the bbr_wait_queue is done "locked".  This
	 * means only 1 cpu at a time will be adding things to the queue.
	 * So, if our pws is the one at the head of the queue, then we
	 * just put it there, and we must get it started.  If our pws is
	 * not at the head of the queue, then it will get started later
	 * (when the other ones in the queue finally get completed).
	 */
	if ( pws == dsp->ds_bbr_wait_queue ) {
	    cdisk_bbr( ccb );
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_bbr()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called to start Bad Block Recovery processing.
 *	It will first check whether BBR is disabled for this device and
 *	that the error is an ECC correctable one.  If we will attempt
 *	BBR, we allocate a buffer for the read command and reassign
 *	data.  We create and send a ccb to read the bad block.
 *	BBR processing will not be started if BBR is disabled, the sense
 *	data is invalid or insufficient, or the error is not an ECC
 *	correctable one.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS:
 *	The original ccb is saved in the working set of the read/write ccb.
 *
 *  ADDITIONAL INFORMATION:
 *	This function serves as the callback completion function for all
 *	the CCBs invlolved in the BBR process.
 *
 ************************************************************************/

static void
cdisk_bbr(ccb)
CCB_SCSIIO *ccb;	
{
	CCB_SCSIIO 	*rw_ccb;	/* Pointer to CCB to use for BBR */
	PDRV_DEVICE 	*pd;		/* Pointer to Peripheral device struc */
	PDRV_WS 	*pws;		/* CCB working set pointer */
	DISK_SPECIFIC 	*dsp;		/* Pointer to disk specific structure */
	u_char 		*rdwr_buf;	/* Pointer to data buffer */
	DIR_DEFECT_LIST *reas_buf;	/* Pointer to reassign buffer */
	ALL_REQ_SNS_DATA *rsen;		/* Pointer to sense data */
	U32 		status;		/* XPT return status */
	int 		spl;		/* Priority level */


	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd  = (PDRV_DEVICE *)pws->pws_pdrv;
	dsp = (DISK_SPECIFIC *)pd->pd_specific;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_bbr: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
	/*
	 * Determine if BBR has not started for this nexus.
	 */
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	if( !dsp->ds_bbr_state )   {
		DEV_DESC *dd = pd->pd_dev_desc;
		U32 lbn;
		u_short codes;

		/*
		 * Check whether BBR is disabled for this device.
		 */
		dd  = (DEV_DESC *)pd->pd_dev_desc;
		if( !(dd->dd_flags & SZ_BBR) )	{
			PDRV_IPLSMP_UNLOCK(pd, spl);
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_DBBR,
			   ("[%d/%d/%d] cdisk_bbr: BBR disabled\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun));
			cdisk_bbr_done(pd, ccb, CD_BBR_NO_ERROR, NOT_RETRY,
			   	"cdisk_bbr: BBR disabled");
			return;
		}
		PDRV_IPLSMP_UNLOCK(pd, spl);

		rsen = (ALL_REQ_SNS_DATA *)ccb->cam_sense_ptr;
		if(SENSEKEY(ccb) == ALL_RECOVER_ERR)  {
		   /*
		    * Check the additional sense code and additional sense
		    * code qualifier. Only handle ECC correctable errors.
		    */
		   if( rsen->asc != CD_ECC_CORRECTABLE )  {
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   (CAMD_DBBR | CAMD_ERRORS),
			   ("[%d/%d/%d] cdisk_bbr: Not ECC Correctable Error asc=0x%x asq=0x%x\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun,
			   rsen->asc, rsen->asq));
			cdisk_bbr_done(pd, ccb, CD_BBR_NO_ERROR, NOT_RETRY,
			   	"cdisk_bbr: Not ECC Correctable Error");
			return;
		   }
		}

		/*
		 * Get a buffer for the read/write and reassign commands
		 * First half is for read/write data and second half is
		 * for reassign data.
		 */
		rdwr_buf = ccmn_get_dbuf((U32)(dsp->ds_block_size * 2));
		/* Fill in the reassign data */
		reas_buf = (DIR_DEFECT_LIST *)((vm_offset_t)rdwr_buf + dsp->ds_block_size);
		reas_buf->defect_header.list_len0 = 4;
		reas_buf->defect_desc[0].lbn3 = rsen->info_byte3;
		reas_buf->defect_desc[0].lbn2 = rsen->info_byte2;
		reas_buf->defect_desc[0].lbn1 = rsen->info_byte1;
		reas_buf->defect_desc[0].lbn0 = rsen->info_byte0;

		/* 
		 * Save the original CCB and BBR data buffer.
		 */
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		dsp->ds_bbr_origccb = ccb;
		dsp->ds_bbr_buf = rdwr_buf;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		/*
		 * For medium and hardware errors we do the reassign now.
		 */
		if(SENSEKEY(ccb) != ALL_RECOVER_ERR)  {
			cdisk_reassign(pd, ccb, reas_buf);
			return;
		}
		/*
		 * Construct the CCB for the read of the bad block.
		 */
		rw_ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)rdwr_buf, 
			dsp->ds_block_size,
			GET_SENSE_SIZE(pd), 
			(pd->pd_cam_flags | CAM_DIR_IN | CAM_SIM_QHEAD | CAM_SIM_QFREEZE), 
			cdisk_bbr, pd->pd_tag_action, GET_IO_TIMEOUT(pd),
			(struct buf *)NULL);

		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		/* 
		 * Save the Read/Write CCB pointer
		 */
		dsp->ds_bbr_rwccb = rw_ccb;

		PDRV_IPLSMP_UNLOCK(pd, spl);
		/*
		 * Construct the cdb for the read command.
		 */
		rw_ccb->cam_cdb_len = ccb->cam_cdb_len;
		if (ccb->cam_cdb_len == 10)   {
			DIR_READ_CDB10 *cdb;
			cdb = (DIR_READ_CDB10 *)rw_ccb->cam_cdb_io.cam_cdb_bytes;
			cdb->opcode = DIR_READ10_OP;
			BTOL(&rsen->info_byte3, lbn);
			DIRLBN_TO_READ10(lbn, cdb);
			cdb->tran_len0 = 1;
		}   else   {
			DIR_READ_CDB6 *cdb;
			cdb = (DIR_READ_CDB6 *)rw_ccb->cam_cdb_io.cam_cdb_bytes;
			cdb->opcode = DIR_READ6_OP;
			BTOL(&rsen->info_byte3, lbn);
			DIRLBN_TO_READ6(lbn, cdb);
			cdb->trans_len = 1;
		}

		/* Send the Read request */
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		dsp->ds_bbr_state = CD_BBR_READ;
		status = cdisk_send_ccb(pd, (CCB_HEADER *)rw_ccb, NOT_RETRY);
		PDRV_IPLSMP_UNLOCK(pd, spl);

		if( status != CAM_REQ_INPROG )
			cdisk_bbr_read(pd, rw_ccb);
		else  {
			CHK_RELEASE_QUEUE(pd, ccb);
		}
		return;
	} else
		PDRV_IPLSMP_UNLOCK(pd, spl);

	/* 
	 * Here after BBR has been started.
	 */

	switch( dsp->ds_bbr_state )   {
		case CD_BBR_READ:
			cdisk_bbr_read(pd, ccb);
			break;
		case CD_BBR_REASSIGN:
		case CD_BBR_REASSIGN_NO_READ:
			cdisk_bbr_reassign(pd, ccb);
			break;
		case CD_BBR_WRITE:
			cdisk_bbr_write(pd, ccb);
			break;
		case CD_BBR_DONE:
			break;
		default:
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			   (CAMD_DBBR | CAMD_ERRORS),
			   ("cdisk_bbr: Invalid BBR state=0x%x",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun,
			   dsp->ds_bbr_state));
			cdisk_bbr_done(pd, ccb, CD_BBR_ERROR, NOT_RETRY,
			   	"cdisk_bbr: Invalid BBR state");
			break;
	}  /* switch */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_bbr_read()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine is called during BBR processing when the read request
 *	for a recovered block has completed.  If the block has gone
 *	from bad to good, then BBR has completed.  If the retry count for the
 *	read is reached, then we will create and send the ccb for the
 *	Reassign Block command.  Otherwise, we retry the read request.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Peripheral device structure pointer.
 *	ccb	- Pointer to SCSI I/O CCB.
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
 *	The hard error counter is incremented if a Medium error sense
 *	key is returned.
 *
 ************************************************************************/

static void
cdisk_bbr_read(pd, ccb)
PDRV_DEVICE *pd;
CCB_SCSIIO *ccb;
{
	PDRV_WS 	*pws;	/* Pointer to working set */
	DISK_SPECIFIC 	*dsp;	/* Pointer to disk specific struct */
	U32 		status;	/* XPT return status */
	U32 		cat;	/* CAM status category */
	int 		spl;	/* Priority level */

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	dsp = (DISK_SPECIFIC *)pd->pd_specific;
	cat = ccmn_ccb_status((CCB_HEADER *)ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr_read: entry\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));

	switch( cat )	{
	/*
	 * Check if the block has gone from bad to good.
	 */
	case CAT_CMP:
		cdisk_bbr_done(pd, ccb, CD_BBR_NO_ERROR, RETRY,
			"cdisk_bbr_read: Bad block ok no BBR action");
		return;

	case CAT_CCB_ERR:
	case CAT_NO_DEVICE:
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		    (CAMD_DBBR | CAMD_ERRORS),
		    ("BBR failed - Invalid Cam Status\n",
	   	    pd->pd_bus, pd->pd_target, pd->pd_lun));
		cdisk_bbr_done(pd, ccb, CD_BBR_ERROR, NOT_RETRY,
			"cdisk_bbr_read: Invalid CAM status");
		return;

	case CAT_CMP_ERR:
		if( (ccb->cam_scsi_status == SCSI_STAT_CHECK_CONDITION)
	           && (ccb->cam_ch.cam_status & CAM_AUTOSNS_VALID)
		   && (CHECK_SENSE_RESID(ccb) == 0) )  {
			if( SENSEKEY(ccb) == ALL_NO_SENSE )	{
			     	cdisk_bbr_done(pd, ccb, CD_BBR_NO_ERROR,
				   RETRY, 
				   "cdisk_bbr_read: No Sense sense key");
				return;
			}
			/*
			 * The block has gone completely bad.
			 */
			if( SENSEKEY(ccb) == ALL_MEDIUM_ERR )  {
				PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		    		   (CAMD_DBBR | CAMD_ERRORS),
		    		   ("BBR failed - Medium Error\n",
	   	    		   pd->pd_bus, pd->pd_target, pd->pd_lun));
				pd->pd_hard_err++;
				cdisk_bbr_done(pd, ccb, CD_BBR_ERROR, NOT_RETRY,
					"cdisk_bbr_read: Medium Error");
				return;
			} else if( SENSEKEY(ccb) != ALL_RECOVER_ERR )  {
				PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		    		   (CAMD_DBBR | CAMD_ERRORS),
		    		   ("BBR failed - Invalid Sense Key\n",
	   	    		   pd->pd_bus, pd->pd_target, pd->pd_lun));
				cdisk_bbr_done(pd, ccb, CD_BBR_ERROR, 
				   NOT_RETRY, 
				   "cdisk_bbr_read: Invalid sense key");
				return;
			}
			
		}
		/*
		 * All other SCSI status values and invalid sense data
		 * will fall through here and result in a retry.
		 */
	default:
		break;
	} /* switch */
	/*
	 * All other CAM status values will result in a retry. If the
 	 * retry count has not been reached then resend the read request.
	 */
	if( ++pws->pws_retry_cnt < CD_BBR_RETRY )    {
		CLEAR_CCB(ccb);
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		status = cdisk_send_ccb(pd, (CCB_HEADER *)ccb, RETRY);
		PDRV_IPLSMP_UNLOCK(pd, spl);
		if( status != CAM_REQ_INPROG )	
			cdisk_bbr_read(pd, ccb);
		else  {
			/*
			 * We know the queue is frozen here.
			 */
			RELEASE_QUEUE(pd);
		}
		return;
	}

	/*
	 * Clear the counter which counts the number of times we will issue
	 * a Reassign Blocks command.
	 */
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	dsp->ds_bbr_retry = 0;

	PDRV_IPLSMP_UNLOCK(pd, spl);
	/*
	 * Issue the Reassign Block command.
	 */
	cdisk_reassign(pd, ccb, 
		(u_char *)((vm_offset_t)ccb->cam_data_ptr + dsp->ds_block_size));

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr_read: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_reassign()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during BBR processing to create and send
 *	the Reassign Blocks command.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Peripheral device structure pointer.
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_reassign(pd, ccb, addr)
PDRV_DEVICE *pd;
CCB_SCSIIO *ccb;
u_char *addr;
{
	DISK_SPECIFIC 	  *dsp;		/* Pointer to disk specific struct */
	U32 		  status;	/* XPT return status */
	int 		  spl;		/* Priority level */
	CCB_SCSIIO 	  *reas_ccb;	/* Pointer Reassign ccb */
	DIR_REASSIGN_CDB6 *cdb;		/* Pointer to cdb for reassign */
	u_char 		  retry;	/* Indicates whether request */
					/* already on peripheral queue */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_reassign: entry\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));

	dsp = (DISK_SPECIFIC *)pd->pd_specific;

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	/*
	 * Check whether we came here without doing a read.
	 */
	if( !dsp->ds_bbr_state)
		dsp->ds_bbr_state = CD_BBR_REASSIGN_NO_READ;
	else
		dsp->ds_bbr_state = CD_BBR_REASSIGN;

	PDRV_IPLSMP_UNLOCK(pd, spl);

	/*
	 * Construct the CCB for the REASSIGN command.
	 */
	if (!dsp->ds_bbr_reasccb)	{
	    reas_ccb = ccmn_io_ccb_bld(pd->pd_dev, addr,
		sizeof(DIR_REAS_DEFECT_HEADER)  + sizeof(DIR_DEFECT_DESC),
		GET_SENSE_SIZE(pd), 
		(pd->pd_cam_flags|CAM_DIR_OUT|CAM_SIM_QHEAD|CAM_SIM_QFREEZE), 
		cdisk_bbr, pd->pd_tag_action, (GET_IO_TIMEOUT(pd) *  5),
		(struct buf *)NULL);
	    /*
	     * Fill in the cdb for the REASSIGN BLOCKS command.
	     */
	    cdb = (DIR_REASSIGN_CDB6 *)reas_ccb->cam_cdb_io.cam_cdb_bytes;
	    cdb->opcode = DIR_REASSIGN_OP;
	    reas_ccb->cam_cdb_len = sizeof(DIR_REASSIGN_CDB6);
	    dsp->ds_bbr_reasccb = reas_ccb;
	    retry = NOT_RETRY;
	}  else  {
	    retry = RETRY;
	    reas_ccb = dsp->ds_bbr_reasccb;
	    CLEAR_CCB(reas_ccb);
	}

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	status = cdisk_send_ccb(pd, (CCB_HEADER *)reas_ccb, retry);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	/*
	 * If not queued in SIM then call the completion routine
	 * for this CCB.
	 */
	if( status != CAM_REQ_INPROG )
		cdisk_bbr_reassign(pd, reas_ccb);
	else
		RELEASE_QUEUE(pd);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_reassign: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_bbr_reassign()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during BBR processing when a Reassign
 *	Block command has completed.  If the reassignment was
 *	successful, then create and send a ccb to write the original
 *	data to the reassigned block.  If the reassignment failed, call
 *	the BBR completion function indicating the failure.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Peripheral device structure pointer.
 *	ccb	- Pointer to SCSI I/O CCB for reassign command.
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

static void
cdisk_bbr_reassign(pd, reas_ccb)
PDRV_DEVICE *pd;
CCB_SCSIIO *reas_ccb;
{
	PDRV_WS 	*pws;		/* Pointer to working set */
	DISK_SPECIFIC 	*dsp;		/* Pointer to disk specific struct */
	U32 		status;		/* XPT return status */
	int 		spl;		/* Priority level */
	CCB_SCSIIO 	*wr_ccb;	/* Pointer to write ccb */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr_reassign: entry\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
	/*
	 * Check if the Reassign command failed.
	 */
	if( CAM_STATUS(reas_ccb) != CAM_REQ_CMP )  {
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
		   (CAMD_DBBR | CAMD_ERRORS),
		   ("[%d/%d/%d] cdisk_reassign: Reassign failed status=0x%x\n",
	  	   pd->pd_bus, pd->pd_target, pd->pd_lun,
		   CAM_STATUS(reas_ccb)));
		cdisk_bbr_done(pd, reas_ccb, CD_BBR_ERROR, NOT_RETRY,
			"cdisk_bbr_reassign: Reassign Cmd failed");
		return;
	}

	dsp = (DISK_SPECIFIC *)pd->pd_specific;
	if(dsp->ds_bbr_state == CD_BBR_REASSIGN_NO_READ)  {
		cdisk_bbr_done(pd, reas_ccb, CD_BBR_NO_ERROR, RETRY,
			"cdisk_bbr_reassign: BBR complete");
		return;
	}
	/*
 	 * Construct the write request. - Use the CCB used for the reads.
	 */
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	wr_ccb = dsp->ds_bbr_rwccb;
	dsp->ds_bbr_state = CD_BBR_WRITE;
	PDRV_IPLSMP_UNLOCK(pd, spl);

	pws = (PDRV_WS *)wr_ccb->cam_pdrv_ptr;
	pws->pws_retry_cnt = 0;

	/* Adjust the cdb for the write command */
	if (wr_ccb->cam_cdb_len == 10)   {
		DIR_WRITE_CDB10 *cdb;
		cdb = (DIR_WRITE_CDB10 *)wr_ccb->cam_cdb_io.cam_cdb_bytes;
		cdb->opcode = DIR_WRITE10_OP;
	} else {
		DIR_WRITE_CDB6 *cdb;
		cdb = (DIR_WRITE_CDB6 *)wr_ccb->cam_cdb_io.cam_cdb_bytes;
		cdb->opcode = DIR_WRITE6_OP;
	}
	wr_ccb->cam_ch.cam_flags &= ~CAM_DIR_IN;
	wr_ccb->cam_ch.cam_flags |= CAM_DIR_OUT;

	CLEAR_CCB(wr_ccb);

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	status = cdisk_send_ccb(pd, (CCB_HEADER *)wr_ccb, RETRY);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	if( status != CAM_REQ_INPROG )
		cdisk_bbr_write(pd, wr_ccb);
	else
		CHK_RELEASE_QUEUE(pd, reas_ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr_reassign: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_bbr_write()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during BBR processing with the completed
 *	ccb for the write command.  If the write completed successfully,
 *	then BBR has completed.  If the write failed, we will issue a
 *	retry.  If the retry count is reached, then we will retry the Reassign
 *	Block command.  If the reassignment retry count has been reached, then
 *	BBR has completed unsuccessfully.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Peripheral device structure pointer.
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_bbr_write(pd, ccb)
PDRV_DEVICE *pd;
CCB_SCSIIO *ccb;
{
	PDRV_WS 	*pws;		/* Pointer to working set */
	DISK_SPECIFIC 	*ds;		/* Pointer to disk specific struct */
	U32 		status;		/* XPT return status */
	int 		spl;		/* Priority level */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr_write: entry\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	ds = (DISK_SPECIFIC *)pd->pd_specific;
	/*
	 * Check if we have finished.
	 */
	if( CAM_STATUS(ccb) == CAM_REQ_CMP )  {
		cdisk_bbr_done(pd, ccb, CD_BBR_NO_ERROR, RETRY,
			"cdisk_bbr_write: BBR complete");
		return;
	}

	/*
	 * If the retry count has not been reached then resend
	 * the write request.
	 */
	if( ++pws->pws_retry_cnt < CD_BBR_RETRY )    {
		CLEAR_CCB(ccb);

		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		status = cdisk_send_ccb(pd, (CCB_HEADER *)ccb, RETRY);
		PDRV_IPLSMP_UNLOCK(pd, spl);

		RELEASE_QUEUE(pd);

		if( status != CAM_REQ_INPROG )
			cdisk_bbr(ccb);

	} else if( ++ds->ds_bbr_retry < CD_BBR_RETRY )  {
		/*
		 * If the BBR retry count for the reassignment has not
		 * been reached then Reissue the Reassign Command.
		 */
		cdisk_reassign(pd, ccb,
		    (u_char *)((vm_offset_t)ccb->cam_data_ptr + ds->ds_block_size)); 
	}  else   {
		/*
		 * All retry counts have been exhausted.
		 */
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
			(CAMD_DBBR | CAMD_ERRORS),
			("[%d/%d/%d] cdisk_bbr_write: Reached BBR Max Retry\n",
	   		pd->pd_bus, pd->pd_target, pd->pd_lun));
		cdisk_bbr_done(pd, ccb, CD_BBR_ERROR, NOT_RETRY,
			"cdisk_bbr_write: BBR retry exhausted");
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr_write: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_bbr_done()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called to handle the successful or failure
 *	completion of BBR processing.  The BBR ccbs are deallocated
 *	and the original request is reissued.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Peripheral device structure pointer.
 *	ccb	- Pointer to SCSI I/O CCB.
 *	error 	- Indiactes whether BBR has completed with/without
 *		  error.
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

static void
cdisk_bbr_done(pd, ccb, error, retry, string)
PDRV_DEVICE *pd;
CCB_SCSIIO *ccb;
u_short error;
u_short retry;
caddr_t *string;
{
	PDRV_WS 	*pws;		/* Pointer to working set */
	DISK_SPECIFIC 	*dsp;		/* Pointer to disk specific struct */
	U32		status;		/* XPT return status */
	int		spl;		/* Priority level */
	CCB_SCSIIO 	*orig_ccb;	/* Pointer to original ccb */
	struct buf 	*bp;		/* Pointer to buf structure */
					/* for original command */
	U32 		bad_block;
	static u_char	module[] = "cdisk_bbr_done";
	static u_char	log_str[100];
	static u_char	bad_block_str[20];

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr_done: entry\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
	/*
	 * Indicate BBR has completed.
	 */
	dsp = (DISK_SPECIFIC *)pd->pd_specific;

	/*
	 * Get the original ccb which had the recovered error.
	 * Then get the right PDRV working set.
	 */
	if (dsp->ds_bbr_origccb) {
		orig_ccb = dsp->ds_bbr_origccb;
	}
	else {
		orig_ccb = ccb;
	}

	pws = (PDRV_WS *)orig_ccb->cam_pdrv_ptr;

	/*
	 * Have we completed the one we were waiting for?
	 */
	if( pws != dsp->ds_bbr_wait_queue ) {
		/* if not, then panic */
		panic("cdisk_bbr_done: Finished pws != waiting pws.\n");
	}

	/*
	 * Remove it from the bbr wait queue.
	 */
	dsp->ds_bbr_wait_queue = dsp->ds_bbr_wait_queue->wait_flink;

	/*
	 * Log a message indicating what has happened.
	 */
       	BTOL(&((ALL_REQ_SNS_DATA *)(orig_ccb->cam_sense_ptr))->info_byte3, 
		bad_block);
	itoa(bad_block, bad_block_str);
	strcpy(&log_str[0], string);
	strcpy(&log_str[strlen(string)], " bad block number: ");
	strcpy(&log_str[strlen(log_str)], bad_block_str); 
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	CAM_ERROR(module, log_str, CAM_SOFTERR,
		(CCB_HEADER *)ccb, pd->pd_dev, (u_char *)NULL);
	PDRV_IPLSMP_UNLOCK(pd, spl);

	/*
	 * If any BBR action was taken we need to get the original ccb
	 * (which was saved in the working set of the read/write ccb)
	 * and free resources.
	 */
	if( dsp->ds_bbr_state )  {
		if(dsp->ds_bbr_rwccb) {
			ccmn_rem_ccb(pd, dsp->ds_bbr_rwccb);
			ccmn_rel_ccb((CCB_HEADER *)dsp->ds_bbr_rwccb);
		}
		if (dsp->ds_bbr_reasccb)   {
			ccmn_rem_ccb(pd, dsp->ds_bbr_reasccb);
			ccmn_rel_ccb((CCB_HEADER *)dsp->ds_bbr_reasccb);
		}
		if (dsp->ds_bbr_buf)   {
			ccmn_rel_dbuf(dsp->ds_bbr_buf, 
				(U32)(dsp->ds_block_size * 2));
		}
		dsp->ds_bbr_rwccb = (CCB_SCSIIO *)NULL;
		dsp->ds_bbr_reasccb = (CCB_SCSIIO *)NULL;
		dsp->ds_bbr_origccb = (CCB_SCSIIO *)NULL;
		dsp->ds_bbr_buf = (u_char *)NULL;
		dsp->ds_bbr_state = 0;
	}

	/*
	 * If BBR failed - then deallocate resources and return the 
	 * original request as an error. If BBR succeeded - then resend
 	 * the original request.
	 */
	if(retry == NOT_RETRY)  {
		bp = (struct buf *)orig_ccb->cam_req_map;
		if (error == CD_BBR_ERROR)  {
			bp->b_flags |= B_ERROR;
			bp->b_error = EIO;
		}  else  {
			bp->b_resid = orig_ccb->cam_resid;
		}
		CHK_RELEASE_QUEUE(pd, orig_ccb);
		ccmn_rem_ccb(pd, orig_ccb);
		ccmn_rel_ccb((CCB_HEADER *)orig_ccb);
		biodone(bp);
	 }  else {
		CLEAR_CCB(orig_ccb);
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		status = cdisk_send_ccb(pd, (CCB_HEADER *)orig_ccb, RETRY);
		PDRV_IPLSMP_UNLOCK(pd, spl);
		if(status != CAM_REQ_INPROG)   {
			orig_ccb->cam_cbfcnp(orig_ccb);
		}   else   {
			RELEASE_QUEUE(pd);
		}
	}

	if( (pws = dsp->ds_bbr_wait_queue) != (PDRV_WS *)NULL ) {
		/*
		 * We just finished with a BBR, and we have another one
		 * to start, so get it going.
		 */
		cdisk_bbr( pws->pws_ccb );
	}

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, (CAMD_DBBR | CAMD_INOUT),
		("[%d/%d/%d] cdisk_bbr_done: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_recovery()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called to start the recovery process.  If the 
 *	device was never opened then no recovery occurrs.  If recovery
 *	is already in progress, we set a bit indicating that recovery
 *	must be restarted and return.  The rest of the recovery code will check
 *	this bit.  Otherwise we set a bit indicating recovery has started 
 *	cam call cdisk_rec_tur() to issue the Test Unit Ready command.
 *	
 *  FORMAL PARAMETERS:
 *	pd		- Pointer to peripheral device structure.
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

static void
cdisk_recovery(pd)
PDRV_DEVICE *pd;
{
	int 		spl, spl1;		/* Priority level */



	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_DISK_REC | CAMD_INOUT), ("[%d/%d/%d] cdisk_recovery: entry\n", 
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	/*
	 * Increment the recovery counter if greater then limit
	 * Fail it.
	 */

 	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	pd->pd_recovery_cnt++;

	if( (pd->pd_recovery_cnt > CDISK_REC_RETRY) || pd->pd_fatal_rec ){
		pd->pd_fatal_rec++;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		cdisk_reset_rec_err(pd);
		return;
	}


	pd->pd_recov_hand = (void *)NULL;
	/*
	 * Make sure that this is a disk device or cd..
	 */
	if(((pd->pd_dev_inq[0] & DEV_TYPE_MASK) != ALL_DTYPE_DIRECT) &&
		((pd->pd_dev_inq[0] & DEV_TYPE_MASK) != ALL_DTYPE_RODIRECT)){
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   	   (CAMD_DISK_REC | CAMD_FLOW),
	   	   ("[%d/%d/%d] cdisk_recovery: Type not of disk\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		return;
	}

	/*
	 * Check whether recovery is already in progress.
 	 * If so, indicate that the recovery process must be started
	 * again.  The rest of the recovery code will check the
	 * PD_REC_PEND bit before continuing with recovery.
	 */
	if( pd->pd_flags & PD_REC_START )  {
		pd->pd_flags |= PD_REC_PEND;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   	   (CAMD_DISK_REC | CAMD_FLOW),
	   	   ("[%d/%d/%d] cdisk_recovery: pending\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		return;
	}
	/* 
	 * Indicate recovery has started.
	 */
	pd->pd_flags |= PD_REC_START;

	PDRV_IPLSMP_UNLOCK(pd, spl);

	cdisk_rec_tur(pd);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
		("[%d/%d/%d] cdisk_recovery: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));

}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_status()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This routine is called during the recovery process when a 
 * 	recovery CCB is returned from the SIM.  If the CAM status
 * 	indicates a reset has occurred then we must start the
 * 	recovery process all over again.  The recovery start bit and
 * 	recovery pending bits are cleared.  The recovery process will be
 * 	restarted when the asynch callback is received.
 *	
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure.
 *	pws	- Pointer to peripheral workign set in ccb.
 *	ccb	- Pointer to completed recovery ccb.
 *	retry	- Retry count for the ccb.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	-1	- Indicates recovery will be restarted.
 *	0	- Indicates to caller to proceed with recovery.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

cdisk_rec_status(pd, pws, ccb, retry)
PDRV_DEVICE *pd;
PDRV_WS *pws;
CCB_SCSIIO *ccb;
U32 retry;
{
	int spl;

	/*
	 * Check whether this ccb's cam status indicates that	
	 * a new recovery will be started once the async 	
	 * callback comes in.
	 */
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	if((CAM_STATUS(ccb) == CAM_SCSI_BUS_RESET) 	
	  || (CAM_STATUS(ccb) == CAM_BDR_SENT)  
	  || (CAM_STATUS(ccb) == CAM_BUSY))  {
		/* 
		 * If a timeout will happen we can handle it there.
		 */
		if(pd->pd_flags & PD_REC_TIMEOUT){
			PDRV_IPLSMP_UNLOCK(pd, spl);
			return(-1);	
		}
		pd->pd_flags &= ~(PD_REC_START | PD_REC_PEND);
		PDRV_IPLSMP_UNLOCK(pd, spl);
		cdisk_rec_rel_ccb(pd);
		return(-1);	
	}
	PDRV_IPLSMP_UNLOCK(pd, spl);
	/*
	 * Check whether we have exhausted our retries.
	 */
	if( pws->pws_retry_cnt >= retry )  {
		cdisk_rec_error(ccb);
		return(-1);	
	}
	/*
	 * Check whether we should retry the ccb.  If so set up a 
	 * timeout for the retry.
	 */
	if ( (CAM_STATUS(ccb) == CAM_SCSI_BUSY) 
	      || (CAM_STATUS(ccb) == CAM_SEL_TIMEOUT) 
	      || ((CAM_STATUS(ccb) == CAM_REQ_CMP_ERR)
		 && (ccb->cam_scsi_status == SCSI_STAT_BUSY)) )   {
			pd->pd_flags |= PD_REC_TIMEOUT;
			timeout(cdisk_rec_retry, (caddr_t)ccb, hz);
			return(-1);
	}
	return(0);
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_retry()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called to retry a ccb used during the
 *	recovery process.  If a new recovery is pending, handle it
 *	else reissue the CCB.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- SCSI I/O CCB for the Test Unit Ready command.
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

static void
cdisk_rec_retry(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_DEVICE 	*pd;		/* Pointer to peripheral struct */
	PDRV_WS 	*pws;		/* Pointer to working set */
	int 		spl;		/* Priority level */
	U32 		status;		/* XPT return status */

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
		("[%d/%d/%d] cdisk_rec_retry: enter\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));


	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	CLEAR_CCB(ccb);

	pd->pd_flags &= ~PD_REC_TIMEOUT;

	CHK_RECOVERY_PENDING(pd, spl);

	pws->pws_retry_cnt++;
	status = ccmn_send_ccb(pd, (CCB_HEADER *)ccb, RETRY);

	PDRV_IPLSMP_UNLOCK(pd, spl);

	if( status != CAM_REQ_INPROG )
		ccb->cam_cbfcnp(ccb);

	/* We know the queue is frozen here */
	RELEASE_QUEUE(pd);


	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
		("[%d/%d/%d] cdisk_rec_retry: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}


/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_tur()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called  during the recovery process to create a ccb
 *	for the Test Unit Ready command and send it to the XPT.
 *	
 *  FORMAL PARAMETERS:
 *	pd		- Pointer to peripheral device structure.
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

static void
cdisk_rec_tur(pd)
PDRV_DEVICE *pd;
{
	DISK_SPECIFIC 	*ds;		/* Pointer to disk specific struct */
	CCB_SCSIIO 	*ccb;		/* CCB for TUR command */
	U32		cam_flags;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_DISK_REC | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_rec_tur: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	ds = (DISK_SPECIFIC *)pd->pd_specific;

	/*
	 * Do not use tags at this point since the drive must be
	 * spun up before running tagged.
	 */
	cam_flags = pd->pd_cam_flags
		 | CAM_DIR_NONE | CAM_SIM_QFREEZE | CAM_SIM_QHEAD;
	if( !(pd->pd_dev_desc->dd_flags & SZ_NOSYNC) )
		cam_flags |= CAM_INITIATE_SYNC; 
	cam_flags &= ~CAM_QUEUE_ENABLE;

	/*
	 * Build a SCSI I/O CCB for the Test Unit Ready command.
	 * Must initiate synchronous here.
	 */
	ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)NULL, 
		(U32)0, GET_SENSE_SIZE(pd), cam_flags,
		cdisk_rec_tur_done, pd->pd_tag_action, 
		GET_TIMEOUT(pd), (struct buf *)NULL);
	ccb->cam_cdb_len = sizeof(ALL_TUR_CDB);
	ds->ds_tur_ccb = ccb;

	/*
	 * Send it to XPT.
	 */
	ISSUE_REC_CCB(pd, ccb, NOT_RETRY, PD_REC_TUR);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_DISK_REC | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_rec_tur: exit\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));
}


/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_tur_done()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This is the callback completion function for the Test Unit Ready
 *	command during the recovery process.  If the TUR command
 *	completed, a Mode Select command is issued for the first entry in the
 *	Mode Select table.  If no entries exist, then the Read Capacity 
 *	command is issued. If the TUR command failed, and the sense key
 *	is not ready, then the Start Unit command is issued. If the sense key is
 *	Unit Attention then the TUR command is retried.  For all other cases,
 *	recovery has failed. 
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_tur_done(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_DEVICE 	*pd;		/* Pointer to peripheral struct */
	PDRV_WS 	*pws;		/* Pointer to working set */
	int	    	ready_time;

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_DISK_REC | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_rec_tur_done: entry status=0x%x\n",
   	   pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));

	ready_time = 5;

	/*
	 * Removables increase our time since they are slow.
	 */
	if( ((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb)  {
	    ready_time = 30;
	}

	if(cdisk_rec_status(pd, pws, ccb, ready_time) == -1)
		return;
	/*
	 * If the Test Unit Ready command was successful,
	 * send the mode select command for the first entry in the
	 * Mode Select table.  If no entry exists, send the read
	 * capacity command.
	 */
	if( CAM_STATUS(ccb) == CAM_REQ_CMP )  {
		MODESEL_TBL *mstp = pd->pd_dev_desc->dd_modesel_tbl;
		if( (mstp == (MODESEL_TBL *)NULL) ||
		    (mstp->ms_entry[0].ms_data == (u_char *)NULL) )  {
			cdisk_rec_read_cap(pd);
			return;
		} else
			cdisk_rec_modesel(pd);
			return;
	}

	if( (CAM_STATUS(ccb) == CAM_REQ_CMP_ERR)
	   && (ccb->cam_scsi_status == SCSI_STAT_CHECK_CONDITION) 
	   && (CHECK_SENSE_RESID(ccb) == 0 )
	   &&  (ccb->cam_ch.cam_status & CAM_AUTOSNS_VALID) )   {
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_DISK_REC,
		   ("[%d/%d/%d] cdisk_rec_tur_done: TUR sense key=0x%x\n",
	  	   pd->pd_bus, pd->pd_target, pd->pd_lun, SENSEKEY(ccb)));
		if( SENSEKEY(ccb)  == ALL_NOT_READY)  {
			/*
			 * Check whether this is removable - if
			 * so check whether media is present.
			 */
			if( ((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb)  {
			   if( ((ALL_REQ_SNS_DATA *) 
				ccb->cam_sense_ptr)->asc == CD_NO_MEDIA) {
					cdisk_rec_done(ccb);	
					return;
			   }
			}
			/*
			 * Issue the Start Unit command.
			 */
			cdisk_rec_start(pd, ccb);
			return;
		}
	}
	/*
	 * Resend the Test Unit Ready command.
	 */
	cdisk_rec_retry(ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
		("[%d/%d/%d] cdisk_rec_tur_done: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_start()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during the recovery process to create
 *	and issue the Start Unit command.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Peripheral device structure pointer.
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_start(pd, ccb)
PDRV_DEVICE *pd;
CCB_SCSIIO *ccb;
{
	DISK_SPECIFIC 	*ds;		/* Pointer to disk specific struct */
	int 		spl;		/* Priority level */
	CCB_SCSIIO 	*start_ccb;	/* Pointer to Start Unit ccb */
	u_char 		retry;		/* Indicates whether the request */
					/* is already on the peripheral queue */
	U32		cam_flags;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
		("[%d/%d/%d] cdisk_rec_start: entry\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));

	ds = (DISK_SPECIFIC *)pd->pd_specific;

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	/*
	 * Construct the ccb for the Start Unit command.
	 */
	if (!ds->ds_start_ccb) {
		DIR_START_CDB6 *cdb;
		int ready_time;

		/*
		 * Can't use tags until spun up.	
		 */
		cam_flags = (pd->pd_cam_flags | CAM_DIR_NONE | 
			  CAM_SIM_QFREEZE | CAM_SIM_QHEAD),
		cam_flags &= ~CAM_QUEUE_ENABLE;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		ready_time = pd->pd_dev_desc->dd_ready_time;
		if (ready_time <= 0)
			ready_time  = SZ_READY_DEF;
		start_ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)NULL,
			  (U32)0, GET_SENSE_SIZE(pd), cam_flags,
			  cdisk_rec_start_done,pd->pd_tag_action, 
			  ready_time, (struct buf *)NULL);
		retry = NOT_RETRY;
		/* Build the CDB */
		cdb = (DIR_START_CDB6 *)start_ccb->cam_cdb_io.cam_cdb_bytes;
		bzero((caddr_t)cdb, sizeof(DIR_START_CDB6));
		cdb->opcode = DIR_START_OP;
		cdb->start = 1;
		start_ccb->cam_cdb_len = sizeof(DIR_START_CDB6);
		ds->ds_start_ccb = start_ccb;
	}  else  {
		PDRV_IPLSMP_UNLOCK(pd, spl);
		start_ccb = ds->ds_start_ccb;
		CLEAR_CCB(start_ccb);
		retry = RETRY;
	}

	ISSUE_REC_CCB(pd, start_ccb, retry, PD_REC_ST_UNIT);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
		("[%d/%d/%d] cdisk_rec_start: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_start_done()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during the recovery process when 
 *	the Start Unit command has completed.  The Test Unit Ready
 *	command is reissued.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_start_done(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_WS 	*pws;		/* Pointer to working set */
	PDRV_DEVICE 	*pd;		/* Pointer to peripheral struct */

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_start_done: entry status=0x%x\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));

	/*
 	 * If the start unit failed set up a 1 second timer to retry
	 * the Test Unit Ready command.  Otherwise just retry the Test
	 * Unit Ready command.
	 */
	if(CAM_STATUS(ccb) != CAM_REQ_CMP)
		timeout(cdisk_rec_retry,
		       ((DISK_SPECIFIC *)pd->pd_specific)->ds_tur_ccb,
			hz);
	else
		cdisk_rec_retry(((DISK_SPECIFIC *)pd->pd_specific)->ds_tur_ccb);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_start_done: exit\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_modesel()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during the recovery process to issue the
 *	Mode Select command for one of the pages in the Mode Select
 *	table.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Peripheral device structure pointer.
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

static void
cdisk_rec_modesel(pd)
PDRV_DEVICE *pd;
{
	DISK_SPECIFIC *ds = (DISK_SPECIFIC *)pd->pd_specific;
	MODESEL_TBL *mstp = pd->pd_dev_desc->dd_modesel_tbl;
	struct ms_entry *msep;
	CCB_SCSIIO *mdsel_ccb;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_DISK_REC | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_rec_modesel: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	/*
	 * Build a SCSI I/O CCB for the Mode Select command.
	 */
	msep = (struct ms_entry *)&mstp->ms_entry[0];
	mdsel_ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)NULL, 
	   (U32)0, GET_SENSE_SIZE(pd),
	   (pd->pd_cam_flags | CAM_DIR_OUT | CAM_SIM_QFREEZE | CAM_SIM_QHEAD),
	   cdisk_rec_modesel_done, pd->pd_tag_action, GET_TIMEOUT(pd), 
	   (struct buf *)NULL);

	/* 
	 * Construct the Mode Select CCB and cdb using the first
	 * entry in the Mode Select table.
	 */
	CCB_MODE_SEL(pd, mdsel_ccb, msep);
	ds->ds_mdsel_ccb = mdsel_ccb;
	pd->pd_ms_index = 0;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
	   (CAMD_DISK_REC | CAMD_FLOW),
   	   ("[%d/%d/%d] cdisk_rec_modesel: exit\n",
   	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	ISSUE_REC_CCB(pd, mdsel_ccb, NOT_RETRY, PD_REC_MODE_SEL);
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_modesel_done()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during the recovery process after a Mode
 *	Select command has completed for an entry in the Mode Select table.  If
 *	the request completed successfully and there is another entry in the
 *	Mode Select table we will reissue the command for the next entry.
 *	Otherwise we will issue the Read Capacity command.  If the Mode Select
 *	failed we will retry the request.  If the retry count is reached then
 *	recovery has failed.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_modesel_done(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_WS 	*pws;		/* Pointer to working set */
	PDRV_DEVICE 	*pd;		/* Pointer to peripheral struct */
	int 		spl;		/* Priority level */

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_modesel_done: entry status=0x%x\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));

	/*
	 * Check if a new recovery is pending.
	 */
	if(cdisk_rec_status(pd, pws, ccb, CD_RETRY_RECOVERY) == -1)
		return;

	switch( CAM_STATUS(ccb) )   {
	    case CAM_REQ_CMP:
	    {
		struct ms_entry *msep;

		msep = (struct ms_entry *)
		  &pd->pd_dev_desc->dd_modesel_tbl->ms_entry[++pd->pd_ms_index];

		/*
		 * Check if there are more pages to send. If not
		 * we are ready to issue the Read Capacity command.
		 */
		if( (msep == (struct ms_entry *)NULL)
		    || (msep->ms_data == (u_char *)NULL)
		    || (pd->pd_ms_index >= MAX_OPEN_SELS) )
			break;

		/* Construct the Mode Select CDB */
		CCB_MODE_SEL(pd, ccb, msep);

		CLEAR_CCB(ccb);

		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		pws->pws_retry_cnt = 0;
		PDRV_IPLSMP_UNLOCK(pd, spl);

		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_DISK_REC,
	   	   ("[%d/%d/%d] cdisk_rec_modesel_done: Issue next page\n",
   		   pd->pd_bus, pd->pd_target, pd->pd_lun));

		ISSUE_REC_CCB(pd, ccb, RETRY, PD_REC_MODE_SEL);
		return;
	    }
	    default:
		cdisk_rec_retry(ccb);
		return;
	}	/* switch */

	/*
	 * Issue the read capacity command.
	 */
	cdisk_rec_read_cap(pd);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_modesel_done: exit\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_read_cap()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during the recovery process to create
 *	and send the Read Capacity command.
 *
 *  FORMAL PARAMETERS:
 *	pd	- Pointer to peripheral device structure.
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

static void
cdisk_rec_read_cap(pd)
PDRV_DEVICE *pd;
{
	DISK_SPECIFIC 	   *ds;		/* Pointer to disk specific struct */
	CCB_SCSIIO 	   *rdcp_ccb;	/* Pointer to Read Capacity ccb */
	DIR_READ_CAP_DATA  *rdcp_data;	/* Pointer to Read Capacity data */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_read_cap: entry\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));

	ds = (DISK_SPECIFIC *)pd->pd_specific;

	/* Allocate space for Read Capacity data */
	rdcp_data = (DIR_READ_CAP_DATA *)
		ccmn_get_dbuf((U32)sizeof(DIR_READ_CAP_DATA));

	rdcp_ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)rdcp_data, 
		(U32)sizeof(DIR_READ_CAP_DATA),
		GET_SENSE_SIZE(pd), 
		(pd->pd_cam_flags | CAM_DIR_IN | CAM_SIM_QFREEZE | CAM_SIM_QHEAD),
		cdisk_rec_read_cap_done, pd->pd_tag_action, 
		GET_TIMEOUT(pd), (struct buf *)NULL);

	/* Construct the CDB */
	rdcp_ccb->cam_cdb_io.cam_cdb_bytes[0] = DIR_READCAP_OP;
	rdcp_ccb->cam_cdb_len = sizeof(DIR_READ_CAP_CDB10);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,  CAMD_DISK_REC,
	   ("[%d/%d/%d] cdisk_rec_read_cap:Sending Read Capacity cmd\n",
 	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	ds->ds_rdcp_ccb = rdcp_ccb;

	ISSUE_REC_CCB(pd, rdcp_ccb, NOT_RETRY, PD_REC_READ_CAP);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_read_cap: exit\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_read_cap_done()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during the recovery process when 
 *	the Read Capacity command has completed.  If the Read Capacity
 *	fails, the block size does not match, or the total disk size does not
 *	match, then reocvery has failed.  Otherwise, create and send the read
 *	ccb for the partition table.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_read_cap_done(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_WS 	  *pws;		/* Pointer to working set */
	PDRV_DEVICE 	  *pd;		/* Pointer to peripheral struct */
	int 		  spl;		/* Priority level */
	DIR_READ_CAP_DATA *rdcp_data;	/* Pointer to Read Capacity data */
	U32 		  blk_len;	/* block size */
	U32 		  lbn;		/* last block number */
	DISK_SPECIFIC 	  *ds;		/* Pointer to disk specific struct */

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;
	ds = (DISK_SPECIFIC *)pd->pd_specific;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_read_cap_done: entry status=0x%x\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));

	if(cdisk_rec_status(pd, pws, ccb, CD_RETRY_RECOVERY) == -1)
		return;

	if( CAM_STATUS(ccb) != CAM_REQ_CMP )   {
		cdisk_rec_retry(ccb);
		return;
	}
	/*
 	 * RDCAP returns the address of the last LBN.
	 * We must add one to get the number of LBNs.
	 */
	rdcp_data = (DIR_READ_CAP_DATA *)ccb->cam_data_ptr;
       	BTOL(&rdcp_data->lbn3, lbn);

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	if (ds->ds_tot_size != (lbn + (U32)1))	{
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_DISK_REC,
		   ("[%d/%d/%d] cdisk_rec_read_cap_done: Not same disk size\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		cdisk_rec_error(ccb);
		return;
	} 
	/*
	 * Verify block size matches device desc data.
	 */
	BTOL(&rdcp_data->block_len3, blk_len);
	if ( blk_len != ds->ds_block_size ) {
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, CAMD_DISK_REC,
   		   ("[%d/%d/%d] cdisk_rec_read_cap_done: Invalid Block size\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		cdisk_rec_error(ccb);
		return;
	}

	PDRV_IPLSMP_UNLOCK(pd, spl);

	/*
	 * Read in the partition table.
	 */
	cdisk_rec_read(pd);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
		("[%d/%d/%d] cdisk_rec_read_cap_done: exit\n",
	   	pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_read()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during the recovery process to create a
 *	ccb for the read command for the partition table.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_read(pd)
PDRV_DEVICE *pd;
{
	CCB_SCSIIO 	  *rd_ccb;	/* Pointer to read ccb */
	DIR_READ_CDB6 	  *cdb;		/* Pointer to cdb for read cmd */
	DISK_SPECIFIC 	  *ds;		/* Pointer to disk specific struct */
	u_char 		  *buffer;	/* Pointer for read data */

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
	   (CAMD_DISK_REC | CAMD_INOUT),
	   ("[%d/%d/%d] cdisk_rec_read: entry\n",
	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	ds = (DISK_SPECIFIC *)pd->pd_specific;
	/*
	 * Issue the Read for the partition table.
	 */
	buffer = ccmn_get_dbuf((U32)SBSIZE);

	rd_ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)buffer, 
		(U32)SBSIZE,
		GET_SENSE_SIZE(pd), 
		(pd->pd_cam_flags | CAM_DIR_IN | CAM_SIM_QFREEZE | CAM_SIM_QHEAD),
		cdisk_rec_read_done, pd->pd_tag_action, 
		GET_TIMEOUT(pd), (struct buf *)NULL);

	/* Construct the CDB */
	cdb = (DIR_READ_CDB6 *)rd_ccb->cam_cdb_io.cam_cdb_bytes;
	cdb->opcode = DIR_READ6_OP;
	DIRLBN_TO_READ6(SBLOCK, cdb);
	DIRTRANS_TO_READ6((SBSIZE/ds->ds_block_size), cdb);
	rd_ccb->cam_cdb_len = sizeof(DIR_READ_CDB6);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
  		("[%d/%d/%d] cdisk_rec_read: Issue super block read\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));
	ds->ds_read_ccb = rd_ccb;

	ISSUE_REC_CCB(pd, rd_ccb, NOT_RETRY, PD_REC_READ);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
   	   	("[%d/%d/%d] cdisk_rec_read: exit\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_read_done()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called to handle the completion of
 *	the super block read during the recovery process.
 *	If a new recovery is pending then handle it.  If the read
 *	then  call cdisk_rec_error since recovery has failed.
 *	If the device is removable and the Prevent Media Removal
 *	command must be issued, the cdisk_rec_prev_remove() is called to
 *	issue the CCB for the command.  Otherwise recovery has completed
 *	successfully and call cdisk_rec_done().
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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
static void
cdisk_rec_read_done(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_WS 	*pws;		/* Pointer to working set */
	PDRV_DEVICE 	*pd;		/* Pointer to peripheral struct */
	DISK_SPECIFIC 	*ds;		/* Pointer to disk specific struct */
	U32 		status;		/* XPT return status */
	int 		spl;		/* Priority level */
	struct pt 	*pt;		/* Set to current partition table */
	register union sblock  {	/* Superblock layout */
		struct fs fs;
		char pad[MAXBSIZE];
	} *sblk_addr;

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;
	ds = (DISK_SPECIFIC *)pd->pd_specific;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
   	   	("[%d/%d/%d] cdisk_rec_read_done: entry status=0x%x\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));

	if(cdisk_rec_status(pd, pws, ccb, CD_RETRY_RECOVERY) == -1)
		return;

	if( CAM_STATUS(ccb) != CAM_REQ_CMP )   {
		cdisk_rec_retry(ccb);
		return;
	}

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	pt = &ds->ds_pt; 
	sblk_addr = (union sblock *)ccb->cam_data_ptr;
	/* 
	 * Check whether we have a valid superblock.
	 */
#ifndef OSF
	if (sblk_addr->fs.fs_magic == FS_MAGIC)	{
		struct pt *dpt;
		dpt = (struct pt*)&sblk_addr->pad[SBSIZE - sizeof(struct pt)];
		/*
		 * Check whether we have a valid partition table.
		 */
		if (dpt->pt_magic == PT_MAGIC)   {
			*pt = *dpt;
			pt->pt_valid = PT_VALID;
		}
	}

	if (pt->pt_valid != PT_VALID)	{
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK_REC | CAMD_ERRORS),
   		   ("[%d/%d/%d] cdisk_rec_done: Invalid partition table\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun));
		cdisk_rec_error(ccb);
		return;
	}
#endif  /* OSF */
	PDRV_IPLSMP_UNLOCK(pd, spl);
	/*
	 * Check whether we need to issue the Prevent Media Removal
	 * command.
	 */
	if( (((ALL_INQ_DATA *)pd->pd_dev_inq)->rmb)
	    && (pd->pd_dev_desc->dd_scsi_optcmds & SZ_PREV_ALLOW)
	    && ds->ds_bopenpart)  {
		cdisk_rec_prevent(pd);
	}  else  {
		cdisk_rec_done(ccb);	/* recovery is done */
	}
	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
   	   	("[%d/%d/%d] cdisk_rec_read_done: exit\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_prevent()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called during the recovery process to create a
 *	ccb for the Prevent Media Removale coammnd for removable devices
 *	which support this command.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_prevent(pd)
PDRV_DEVICE *pd;
{
	CCB_SCSIIO *prev_ccb;
	DIR_PREVENT_CDB6 *cdb;
	DISK_SPECIFIC *ds;

	ds = (DISK_SPECIFIC *)pd->pd_specific;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
	   (CAMD_DISK_REC | CAMD_INOUT),
      	   ("[%d/%d/%d] cdisk_rec_prevent: entry\n",
   	   pd->pd_bus, pd->pd_target, pd->pd_lun));

	/* Construct the CCB */
	prev_ccb = ccmn_io_ccb_bld(pd->pd_dev, (u_char *)NULL, 
			(U32)0, GET_SENSE_SIZE(pd),
			(pd->pd_cam_flags | CAM_DIR_NONE | CAM_SIM_QFREEZE 
			| CAM_SIM_QHEAD),
			cdisk_rec_done, pd->pd_tag_action,
			GET_TIMEOUT(pd), (struct buf *)NULL);

	/* Fill in the CDB */
	cdb = (DIR_PREVENT_CDB6 *)prev_ccb->cam_cdb_io.cam_cdb_bytes;
	cdb->opcode = DIR_PREVENT_OP;
	cdb->prevent = CD_PREVENT_REMOVAL;
	prev_ccb->cam_cdb_len = sizeof(DIR_PREVENT_CDB6);
	ds->ds_prev_ccb = prev_ccb;
	ISSUE_REC_CCB(pd, prev_ccb, NOT_RETRY, PD_REC_PREV_ALLOW);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
   	   	("[%d/%d/%d] cdisk_rec_prevent: exit\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_done()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called to handle the completion of
 *	the recovery process when either the read of the partition table
 *	has completed or the prevent media removal command has completed.  
 *	completed.  If a new recovery is pending then handle it.
 *	If the ccb indicates failure, then recovery has failed.  Otherwise,
 *	the ccbs used during the recovery process are removed from the
 *	peripheral queue and returned to the XPT.  All CCBs on the peripheral
 *	queue marked as retryable are reissued.  Also, any requests which came
 *	into the strategy routine during the execution of the recovery process
 *	are resent to the strategy routine.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_done(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_WS 	*pws;		/* Pointer to working set */
	PDRV_DEVICE 	*pd;		/* Pointer to peripheral struct */
	DISK_SPECIFIC 	*ds;		/* Pointer to disk specific struct */
	U32 		status;		/* XPT return status */
	int 		spl;		/* Priority level */
	struct buf	*bp, *tmp_bp;

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;
	ds = (DISK_SPECIFIC *)pd->pd_specific;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
   	   	("[%d/%d/%d] cdisk_rec_done: entry status=0x%x\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));


	if(cdisk_rec_status(pd, pws, ccb, CD_RETRY_RECOVERY) == -1)
		return;

	if( CAM_STATUS(ccb) != CAM_REQ_CMP )   {
		cdisk_rec_retry(ccb);
		return;
	}

	CHK_RELEASE_QUEUE(pd, ccb);	/* release the queue */

	/*
	 * Check for a pending recovery.
	 */
	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	CHK_RECOVERY_PENDING(pd, spl);
	/* 
	 * Remove all recovery CCBs from queue and release them.
	 */
	cdisk_rec_rel_ccb(pd);
	/*
	 * Indicate recovery has completed.
	 */
	pd->pd_flags &= ~(PD_REC_INPROG | PD_REC_START | PD_REC_PEND);
	pd->pd_abort_cnt = 0;
	/*
	 * Resend all CCBs on the queue marked as retryable.
	 */
	pws = pd->pd_active_list.flink;
	while( pws != (PDRV_WS *)pd )  {
		if( pws->pws_flags & PWS_RETRY )	{
			PDRV_WS *next_pws = pws->pws_flink;
			CLEAR_CCB((CCB_SCSIIO *)pws->pws_ccb)
			pws->pws_flags &= ~PWS_RETRY;
			status = ccmn_send_ccb(pd, (CCB_HEADER *)pws->pws_ccb, RETRY);
			PDRV_IPLSMP_UNLOCK(pd, spl);
			PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   	   (CAMD_DISK_REC | CAMD_ERRORS),
			   ("[%d/%d/%d] cdisk_rec_done: Reissue ccb=0x%x\n",
   	   	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, pws->pws_ccb));
			if( status != CAM_REQ_INPROG )
				pws->pws_ccb->cam_cbfcnp(pws->pws_ccb);
			PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
			pws = next_pws;
		} else 
			pws = pws->pws_flink;
	}
		
	/*
 	 * Start any requests which came into the strategy routine while
	 * recovery was in progress.
	 */
	bp = ds->ds_bufhd;
	while( bp != (struct buf *)NULL)  {
		tmp_bp = bp->b_actf;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK_REC | CAMD_INOUT),
	   	   ("[%d/%d/%d] cdisk_rec_done: call strat bp=0x%x\n",
   		   pd->pd_bus, pd->pd_target, pd->pd_lun, bp));
		cdisk_strategy(bp);
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		bp = tmp_bp;
	}
	ds->ds_bufhd = NULL;

	/*
	 * Indicated that we have  recovery 
	 */
	pd->pd_fatal_rec = 0;
	pd->pd_recovery_cnt = 0;
	/*
	 * Issue Wakeup on the address of flags if anyone was stalled
	 * waiting for recovery done.
	 */
	wakeup( &pd->pd_flags);

	PDRV_IPLSMP_UNLOCK(pd, spl);

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_done: exit\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_error()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called when an error is encountered during the
 *	recovery process.  First we check whether there is a recovery
 *	pending and start it.  We remove the recovery ccbs from the
 *	queue and deallocate them.  We return all ccb's marked as
 *	retryable from the peripheral queue to their completion
 *	functions with status of the failing ccb.
 *
 *  FORMAL PARAMETERS:
 *	ccb	- Pointer to SCSI I/O CCB.
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

static void
cdisk_rec_error(ccb)
CCB_SCSIIO *ccb;
{
	PDRV_WS 	*pws;	/* Pointer to working set */
	PDRV_DEVICE 	*pd;	/* Ponter to peripheral struct */
	DISK_SPECIFIC 	*ds;	/* Pointer to disk specific struct */
	U32 		status;	/* XPT return status */
	int 		spl, spl1;	/* Priority level */
	struct buf	*bp;
	struct buf	*tmp_bp;
	static u_char 	module[] = "cdisk_rec_error";

	pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	pd = (PDRV_DEVICE *)pws->pws_pdrv;
	ds = (DISK_SPECIFIC *)pd->pd_specific;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_error: entry status=0x%x\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));

	/*
	 * Check whether the queue was left frozen and release it.
	 */
	CHK_RELEASE_QUEUE(pd, ccb);	

	status = ccb->cam_ch.cam_status;

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
	CHK_RECOVERY_PENDING(pd, spl);

	/* 
	 * Remove recovery CCBs from queue and release them.
	 */
	cdisk_rec_rel_ccb(pd);

	/*
	 * Indicated that we have failed recovery and must start
	 * a new (a clean open).
	 */
	pd->pd_fatal_rec++;

	PDRV_IPLSMP_UNLOCK(pd, spl);

	/* 
	 * cdisk_reset_rec_err() to fail all of our pending and
	 * Active io..
	 */

	cdisk_reset_rec_err(pd); 
}

/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_reset_rec_err()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function is called when the max recovery reset limit is reached.  
 *	We remove the recovery ccbs from the queue and deallocate them.  
 *	We return all ccb's marked as retryable from the peripheral queue to 
 *	their completion functions with status of the failing ccb.
 *
 *  FORMAL PARAMETERS:
 *	pd - Pointer to PDRV_DEVICE struct.
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



static void 
cdisk_reset_rec_err(pd) 
PDRV_DEVICE *pd; 
{
	PDRV_WS         *pws;   /* Pointer to working set */
	DISK_SPECIFIC   *ds;    /* Pointer to disk specific struct */
	int 		spl, spl1; /* Priority level */ 
	struct buf      *bp;
	struct buf      *tmp_bp; 
	static u_char   module[] = "cdisk_reset_rec_err";


	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_error: entry status=0x%x\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun, CAM_STATUS(ccb)));

	PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);

	ds = (DISK_SPECIFIC *)pd->pd_specific;

	RELEASE_QUEUE(pd);	

	CAM_ERROR(module, "Recovery failed", CAM_SOFTERR,
			(CCB_HEADER *)NULL, pd->pd_dev, (u_char *)NULL);
	/* 
	 * Remove recovery CCBs from queue and release them.
	 */
	cdisk_rec_rel_ccb(pd);
	/*
	 * Remove all pending (TAGGED) CCBs from queue and call their 
	 * callback completion functions. We can't call the completion 
	 * function since it will be sent on down...
	 */
	while( pd->pd_pend_ccb != 0){
	    /*
	     * Remove request from pending queue.
	     */
	    pws = (PDRV_WS *)dequeue_head(&pd->pd_pend_list);
	    if(pws == (PDRV_WS *)NULL){
		pd->pd_pend_ccb == 0;
		break; /* Nothing is there */
	    }
	    --pd->pd_pend_ccb;

	    pws->pws_flags |= PWS_FATAL_ERR;
	    pws->pws_ccb->cam_ch.cam_status = CAM_REQ_ABORTED;

	    if( !pws->pws_ccb->cam_req_map ) {
		wakeup(pws->pws_ccb);
		continue;
	    }
	    bp = (struct buf *)pws->pws_ccb->cam_req_map;
	    /* 
	     * Fail this io back to user now....
	     */
	    IO_STRATEGY_ERR(bp, EIO);

	    /*
	     * Now release the ccb back to the pools
	     * we are done with it...
	     */
	    ccmn_rel_ccb( pws->pws_ccb );
	}

	pws = pd->pd_active_list.flink;
	/*
	 * Remove all waiting CCBs from queue and call their 
	 * callback completion functions.
	 */
	while( pws != (PDRV_WS *)pd )  {
		pws->pws_flags &= ~PWS_RETRY;
		pws->pws_flags |= PWS_FATAL_ERR;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK_REC | CAMD_ERRORS),
	   	   ("[%d/%d/%d] cdisk_rec_error: Returning ccb=0x%x\n",
   		   pd->pd_bus, pd->pd_target, pd->pd_lun,
		   pws->pws_ccb));
		pws->pws_ccb->cam_cbfcnp(pws->pws_ccb);
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		pws = pd->pd_active_list.flink;
	}

	/*
	 * Return all requests which came into the stratgey routine while
	 * recovery was in progress with error.
	 */
	bp = ds->ds_bufhd;
	while( bp != (struct buf *)NULL)  {
		tmp_bp = bp->b_actf;
		PDRV_IPLSMP_UNLOCK(pd, spl);
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		   (CAMD_DISK_REC | CAMD_INOUT),
	   	   ("[%d/%d/%d] cdisk_rec_error: call strat bp=0x%x\n",
   		   pd->pd_bus, pd->pd_target, pd->pd_lun, bp));
		IO_STRATEGY_ERR(bp, EIO);
		PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl);
		bp = tmp_bp;
	}
	ds->ds_bufhd = NULL;

	pd->pd_flags &= ~(PD_REC_INPROG | PD_REC_TIMEOUT | PD_REC_START 
			| PD_REC_PEND);


	wakeup( &pd->pd_flags);
	PDRV_IPLSMP_UNLOCK(pd, spl);
}
/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_rec_rel_ccb()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This function is called to remove the recovery CCBs from the peripheral
 * 	queue and release them.
 *
 *  FORMAL PARAMETERS:
 *	pd -	Pointer to peripheral device structure.
 *	ds -	Pointer to disk specific structure.
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

static void
cdisk_rec_rel_ccb(pd)
PDRV_DEVICE *pd;
{
	DISK_SPECIFIC *ds = (DISK_SPECIFIC *)pd->pd_specific;

	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_rel_ccb: enter\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));

	if (ds->ds_tur_ccb)  {
		if(pd->pd_flags & PD_REC_TUR)  {
			ccmn_rem_ccb(pd, ds->ds_tur_ccb);
			pd->pd_flags &= ~PD_REC_TUR;
		}
		ccmn_rel_ccb((CCB_HEADER *)ds->ds_tur_ccb);
		ds->ds_tur_ccb = NULL;
	}
	if (ds->ds_start_ccb)	{
		if(pd->pd_flags & PD_REC_ST_UNIT)  {
			ccmn_rem_ccb(pd, ds->ds_start_ccb);
			pd->pd_flags &= ~PD_REC_ST_UNIT;
		}
		ccmn_rel_ccb((CCB_HEADER *)ds->ds_start_ccb);
		ds->ds_start_ccb = NULL;
	}
	if (ds->ds_mdsel_ccb) {
		if(pd->pd_flags & PD_REC_MODE_SEL)  {
			ccmn_rem_ccb(pd, ds->ds_mdsel_ccb);
			pd->pd_flags &= ~PD_REC_MODE_SEL;
		}
		ccmn_rel_ccb((CCB_HEADER *)ds->ds_mdsel_ccb);
		ds->ds_mdsel_ccb = NULL;
	}
	if (ds->ds_rdcp_ccb)	{
		if(pd->pd_flags & PD_REC_READ_CAP)  {
			ccmn_rem_ccb(pd, ds->ds_rdcp_ccb);
			pd->pd_flags &= ~PD_REC_READ_CAP;
		}
		if(ds->ds_rdcp_ccb->cam_data_ptr)
			ccmn_rel_dbuf(ds->ds_rdcp_ccb->cam_data_ptr,
				(U32)sizeof(DIR_READ_CAP_DATA));
		ccmn_rel_ccb((CCB_HEADER *)ds->ds_rdcp_ccb);
		ds->ds_rdcp_ccb = NULL;
	}
	if (ds->ds_read_ccb)	{
		if(pd->pd_flags & PD_REC_READ)  {
			ccmn_rem_ccb(pd, ds->ds_read_ccb);
			pd->pd_flags &= ~PD_REC_READ;
		}
		if(ds->ds_read_ccb->cam_data_ptr)
			ccmn_rel_dbuf(ds->ds_read_ccb->cam_data_ptr, 
			   	(U32)SBSIZE);
		ccmn_rel_ccb((CCB_HEADER *)ds->ds_read_ccb);
		ds->ds_read_ccb = NULL;
	}
	if (ds->ds_prev_ccb)	{
		if(pd->pd_flags & PD_REC_PREV_ALLOW)  {
			ccmn_rem_ccb(pd, ds->ds_prev_ccb);
			pd->pd_flags &= ~PD_REC_PREV_ALLOW;
		}
		ccmn_rel_ccb((CCB_HEADER *)ds->ds_prev_ccb);
		ds->ds_prev_ccb = NULL;
	}
	PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun, 
		(CAMD_DISK_REC | CAMD_INOUT),
	   	("[%d/%d/%d] cdisk_rec_rel_ccb: exit\n",
   		pd->pd_bus, pd->pd_target, pd->pd_lun));
}

/************************************************************************
 *
 *  ROUTINE NAME: cdisk_minphys()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will compare the b_bcount field in the buf
 *	structure with the maximum transfer limit for the device
 *	(dd_max_record) in the device descriptor structure. The
 *	count is adjusted if it is greater than the limit.
 *
 *  FORMAL PARAMETERS:
 *	bp	 - Buf structure pointer.
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	Modified b_bcount field of buf structure.
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
cdisk_minphys(bp)
struct buf *bp;
{

    PDRV_DEVICE *pd;		/* Our pdrv device struct		*/
    DEV_DESC 	*dd;		/* Our device descriptor 		*/
    dev_t 	dev;		/* RZ dev_t */
    static u_char module[] = "cdisk_minphys";

    dev = bp->b_dev;

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), CAMD_DISK, 
	   ("[%d/%d/%d] cdisk_minphys: entry bp=%xx bcount=%xx\n", 
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	   bp, bp->b_bcount));

    if ( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
	CAM_ERROR(module, "No device struct", CAM_SOFTWARE,
		(CCB_HEADER *)NULL, dev, (u_char *)NULL);
	return;
    }

    dd = pd->pd_dev_desc;
	
    /*
     * Get the maximun transfer size for this device. If b_bcount
     * is greater then adjust it. 
     */
    if (bp->b_bcount > dd->dd_max_record ){
	bp->b_bcount = dd->dd_max_record;
    }
    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), CAMD_DISK, 
	   ("[%d/%d/%d] cdisk_minphys: exit - success\n", 
	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
}
/*
 * This isn't a generic SCSI dump routine.  It calls the prom
 * code to do the dump. For MIPS, hacked to support rex - burns.
 */

#define ONE_MEGABYTE    0x00100000

rzdump()
{
#ifdef OSF
        cdisk_dump();
#endif
}

#ifdef OSF
#ifdef __alpha
/************************************************************************
 *
 *  ROUTINE NAME: cdisk_dump(dump_req)
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	This is called from dumpsys through the bdevsw to fetch the 
 *	info needed to dump that is device specific. 
 * 	It allocates a protocol block, fills in the SCSI specific data,
 * 	and links the protocol structure to the generic dump structure.
 *
 *  FORMAL PARAMETERS:
 *	dump_req - Generic dump info structure
 *
 *  IMPLICIT INPUTS:
 *	None.
 *
 *  IMPLICIT OUTPUTS:
 *	scsidump_req - linked into the dump_req structure and returns
 *			info specific to the device needed to dump to
 *			this device.
 *
 *  RETURN VALUE(s):
 *	ENXIO, Success(0)
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/

/*
 * This is called from dumpsys to fetch the info needed to dump
 * that is device specific. It allocates a protocol block, fills 
 * in the SCSI specific data, and links the protocol structure to 
 * the generic dump structure.
 * 
 */
#define ONE_MEGABYTE    0x00100000
#include "io/dec/mbox/mbox.h"

cdisk_dump(dump_req)
	register struct dump_request *dump_req;
{
	register dev_t dev;
	PDRV_DEVICE *pd;
	DISK_SPECIFIC *ds;

#if 	LABELS
	int part;
	struct partition *pp;
#else
	struct pt *pt;
#endif	/* LABELS */

	dev = dump_req->dump_dev;
	if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
		   DEV_LUN(dev), CAMD_ERRORS,
		   ("[%d/%d/%d]cdisk_dump: No peripheral device structure\n",
   	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		return(ENXIO);
	}

	dump_req->device = camdinfo[pd->pd_log_unit];
	ds = (DISK_SPECIFIC *)pd->pd_specific;

#if 	LABELS
	part = CD_GET_PARTITION(dev);
	pp = &ds->ds_label.d_partitions[part];
	dump_req->blk_offset = pp->p_offset + dump_req->blk_offset;
#else
	pt = &ds->ds_pt;
	dump_req->blk_offset = pt->pt_part[part].pi_blkoff + dump_req->blk_offset;
#endif	/* LABELS */

	return(ENOSYS);
}

#else /*alpha*/

extern rex_base;
extern daddr_t dumplo;
extern dev_t dumpdev;

cdisk_dump()
{
	register daddr_t blkcnt, maxsz, blk_off;
	int part, status, slot, i;
	PDRV_DEVICE *pd;
	DISK_SPECIFIC *ds;
	struct device *device;
	int scsi_cntlr;
	extern  struct tc_slot tc_slot[8];
#if 	LABELS
	struct partition *pp;
#else
	struct pt *pt;
#endif	/* LABELS */
	char prom_name[80];
	int dump_io;
	char *start;
	register int blk, bc;
	register char *cp;
	dev_t dev;

	dev = dumpdev;

	if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
		   DEV_LUN(dev), CAMD_ERRORS,
		   ("[%d/%d/%d]cdisk_dump: No peripheral device structure\n",
   	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		return(ENXIO);
	}

	if (pd->pd_flags & PD_NO_DEVICE) {
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
		   DEV_LUN(dev), CAMD_ERRORS,
		   ("[%d/%d/%d]cdisk_dump: Unit is offline\n",
   	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		return(ENXIO);
	}
	ds = (DISK_SPECIFIC *)pd->pd_specific;

	part = CD_GET_PARTITION(dev);
	blkcnt = ctod(dumpsize);
	if(partial_dump){
		  blkcnt += dumpsize/(DEV_BSIZE/sizeof(vm_offset_t));
		  if(blkcnt * (DEV_BSIZE/sizeof(vm_offset_t)) != dumpsize)
		          blkcnt++;
	}

#if 	LABELS
	pp = &ds->ds_label.d_partitions[part];
	if ((dumplo + blkcnt > pp->p_size) ||
		((pp->p_offset + dumplo + blkcnt) >= ds->ds_tot_size)) {
#else	
	pt = &ds->ds_pt;
	if( pt->pt_part[part].pi_nblocks == -1 )
		maxsz = ds->ds_tot_size - pt->pt_part[part].pi_blkoff;
	else
		maxsz = pt->pt_part[part].pi_nblocks;
	if( ((dumplo + blkcnt) > maxsz) ||
	    (pt->pt_part[part].pi_blkoff >= ds->ds_tot_size)) {
#endif	/* LABELS */
		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), 
		   DEV_LUN(dev), CAMD_ERRORS,
		   ("[%d/%d/%d]cdisk_dump: Dump device too small\n",
   	   	   DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev)));
		return(ENOSPC);
	}

	/* 
	 * Open a channel for the Prom code.
	 */
	cp = prom_name;
	i = 0;
	if (!rex_base) {
		/* 
		 * The dump device is an 'rz' device.
		 */
		cp[i++]='r';
		cp[i++]='z';
	} else {
		cp[i++]='b';
		cp[i++]='o';
		cp[i++]='o';
		cp[i++]='t';
		cp[i++]=' ';
	}
	if (!rex_base) {
		cp[i++]='(';
	}
	/* 
	 * Fill in the dump ctlr part of the string. We need to process
	 * one char at a time and the number may be bigger than one digit.  
	 */

	device = camdinfo[pd->pd_log_unit];
	if ((device == 0) || (device->alive == 0)) {
	   printf("Can't Open SCSI Dump Device 0x%x\n", dumpdev);
	   return (ENXIO);
        } else {
	    scsi_cntlr = device->ctlr_num;
	}

	if (!rex_base) {
	    i = i + itoa(scsi_cntlr,&cp[i]);
	} else {
	    slot = 0;
	    while((tc_slot[slot].unit != scsi_cntlr) ||
		  (strcmp(tc_slot[slot].devname,"asc") != 0)) {
		      slot++;
	    }
	    i = i + itoa(tc_slot[slot].slot,&cp[i]);
	    cp[i++]='/';
	    cp[i++]='r';
	    cp[i++]='z';
	}

	if (!rex_base) {
		cp[i++] = ',';	/* comma seperator */
	}

	/*
	 * Now load up the unit number part of the dump device string.
	 */

	if (!rex_base) {
		i = i + itoa(pd->pd_target, &cp[i]);
		cp[i++] = ',';	/* comma seperator */
	} else {
		i = i + itoa(pd->pd_target, &cp[i]);
	}

	/*
	 * Horses in midstream - Third arg to console used to
	 * be partition, now is a block offset for the device.
	 * The console will no longer have knowledge of 'partitions'.
	 * Here we assume that PMAX and MIPSfair will not change
	 * to the new format and that everything else will.
	 */
	if (!rex_base) {
		if ((cpu == DS_3100) || (cpu == DS_5400))
			cp[i++] = 'c';	/* make it the C partition */
		else
			cp[i++] = '0';	/* give it a zero offset */
	  
		cp[i++] = ')';	/* end string here, don't */
		cp[i++] = '\0';	/* need the file name part*/
       } else {
		cp[i++] = '/';
		cp[i++] = 'l';         /* pick any old name! */
		cp[i++] = ' ';
		cp[i++] = '-';
		cp[i++] = 'N';
		cp[i++] = 'o';
		cp[i++] = 'b';
		cp[i++] = 'o';
		cp[i++] = 'o';
		cp[i++] = 't';
		cp[i] = '\0';
	}

	if (rex_base) {
		/*
		 * Check here to see if this is the boot disk.
		 * If not boot disk we must do more setup,
		 */

		*(int *)(rex_base + 0x54) = 0;
		rex_execute_cmd(cp);
		if(*(int *)(rex_base + 0x54) == 0) {
	 		printf("Console dump initialization failed\n");
			return(EIO);
		}
		if (rex_bootinit() < 0) {
			printf("can't init console boot device\n");
			return(EIO);
		}
	} else {
		/*
		 * Open the 'c' partition so that we have full access
		 * to the device.  We will provide the offset through
		 * prom_lseek.
		 * Printing the following will only scare people
		 * when they see the 'c' partition. It is for debug.
		 */
		if ((dump_io = prom_open(prom_name, 2)) < 0) {
			printf("can't open dump device\n");
	 		return (EIO);
	 	}
	}

#if 	LABELS
	blk_off = pp->p_offset + dumplo;
#else
	blk_off = pt->pt_part[part].pi_blkoff + dumplo;
#endif	/* LABELS */

	if (!rex_base) {
		if (prom_lseek(dump_io,blk_off * DEV_BSIZE,0) < 0) {
			printf("can't lseek to dump device\n");
			prom_close(dump_io);
			return(EIO);
		}
	}

	start = (char *)0x80000000;

	/*
	 * Dump the kernel.
	 */
	if(partial_dump){
	        vm_offset_t blocks[DEV_BSIZE/sizeof(vm_offset_t)], *ptr;
	        int count, i, total, num;

	        total = 0;
	        bc = ctob(1);
	        ptr = &blocks[1];
	        num = DEV_BSIZE/sizeof(vm_offset_t) - 1;
	        blocks[0] = (vm_offset_t) partial_dumpmag;
	        while((count = get_next_page(ptr, num)) != 0){
	                for(i=count;i<num;i++) blocks[i+(ptr-blocks)] = 0;
	                if (rex_base)
	                        status = (rex_bootwrite(blk_off, blocks,
							DEV_BSIZE)
					  == DEV_BSIZE) ? 1 : 0; 
	                else
	                        status = (prom_write(dump_io, blocks,
						     DEV_BSIZE) == DEV_BSIZE)
		                  ? 1 :0; 
	                if (status & 1) blk_off++;
	                else {
	                        dprintf("dump i/o error: bn = %d, ", blk_off);
	                        if(!rex_base) prom_close(dump_io);
	                        return(EIO);
	                }
	                for(i=ptr-blocks;i<count+(ptr-blocks);i++){
	                        if (rex_base)
		                        status = (rex_bootwrite(blk_off,
								blocks[i], bc)
						  == bc) ? 1 : 0;
	                        else
		                        status = (prom_write(dump_io,
							     blocks[i], bc)
						  == bc) ? 1 :0; 
	                        if (status & 1) blk_off++;
	                        else {
		                        dprintf("dump i/o error: bn = %d, ",
						blk_off);
		                        if(!rex_base) prom_close(dump_io);
		                        return(EIO);
	                        }
				if(((total + i) * NBPG) % ONE_MEGABYTE == 0)
				        dprintf(".");
	                }
	                total += count;
	                ptr = blocks;
	                num = DEV_BSIZE/sizeof(vm_offset_t);
	        }
	        if(total != dumpsize){
	                dprintf("Mismatched dump size : Expected %d got %d\n",
				dumpsize, total);
	        }  	
	}
	else {
	        while (blkcnt) {
		        blk = blkcnt > DBSIZE ? DBSIZE : blkcnt;
		        bc  = blk * DEV_BSIZE;
		        if (rex_base)
			        status = (rex_bootwrite(blk_off, start, bc) == bc) ? 1 : 0; 
		        else
			        status = (prom_write(dump_io, start, bc) == bc) ? 1 :0; 
		        if (status & 1) {
			        start += bc;
			        blkcnt -= blk;
			        blk_off += blk;
		        }
		        else {
			        dprintf("dump i/o error: bn = %d, ", blk_off);
			        if(!rex_base)
				        prom_close(dump_io);
			        return(EIO);
		        }
		        if ((int)start % ONE_MEGABYTE == 0) {
			        dprintf("."); /* Give watcher a warm feeling */
			}
		}
	}
	return(0);
}
#endif /* __alpha */
#endif	/* OSF */

#ifndef __alpha
/* NOTE: this moved to io/common/driver_support.c for Alpha */
/*
 *  Name:		
 *
 *  Abstract:
 *
 *  Inputs:
 *
 *	dev	device name (major/minor number)
 *  Outputs:
 *
 *
 *  Return
 *  Values:
 *
 */
asyncsel(dev, events, revents, scanning)
	dev_t dev;
	short *events, *revents;
        int scanning;
{
#if	0
	/*
	 * XXX - This code needs work - conversion to BUF_LOCKS - before
	 * it can be activated. - XXX
	 */
	register struct buf *bp;
	register struct abuf *dp;
	register int s;
	register int doown = 0;
	
	s = splbio();
	XPR(XPR_BIO, ("enter asyncsel",0,0,0,0));
	if(async_bp == (struct abuf *)0)
		return(EINVAL);	
	if (scanning) {
		for(dp = async_bp->b_forw; dp != async_bp; dp = dp->b_forw) {
			bp = &dp->bp;
			if(bp->b_dev != dev)
				continue;
			if(bp->b_proc != u.u_procp)
				continue;
			doown++;
			if(bp->b_flags&B_BUSY)
				continue;
			splx(s);
			*revents = *events; /* Found a non-busy buffer */
			return(0);
		}
		if(bp->b_proc->p_wchan == (caddr_t) &selwait)
			if (*events & POLLNORM)
				bp->b_selflags |= ASYNC_RCOLL;
		if (*events & POLLOUT)
			bp->b_selflags |= ASYNC_WCOLL;
		if(!doown) /* None owned so must be ok */
			*revents = *events;
	}
	splx(s);
#endif
	return(0);	
}
#endif /* !__alpha */
/*
 * Return the disk unit number and name for table TBL_DKINFO
 */
char *
szinfo(dkn, unit)
int dkn;
int *unit; /* return */
{
    PDRV_DEVICE 	*pd;	/* Pointer to peripheral device structure */
    int i;

    for(i=0; i<MAX_UNITS; i++)   {
	if( (pdrv_unit_table[i].pu_device) &&
		    ((pdrv_unit_table[i].pu_type == ALL_DTYPE_DIRECT) ||
		    (pdrv_unit_table[i].pu_type == ALL_DTYPE_RODIRECT))) {
	    pd = pdrv_unit_table[i].pu_device;
	    if( ((DISK_SPECIFIC *)pd->pd_specific) ) {
		if( ((DISK_SPECIFIC *)pd->pd_specific)->ds_dkn == dkn) {
		    /* 
		     * NOTE: This needs to be changed when wrapper code
		     *       is removed 
		     */
		    *unit = (pd->pd_bus * NDPS) + pd->pd_target;
		    return((char *)"rz");
		}
	    }
	}
    }
    return (NULL);
}

/************************************************************************
 *
 *  ROUTINE NAME: cdisk_srvc_req()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles all service requests. The call is from 
 *	cdisk_ioctl by detecting that this is a service request ioctl. 
 *	The functionality that this routine does is to decode what service 
 *	is requested by the user.
 *
 *  FORMAL PARAMETERS:
 *	dev	 - Dev number.
 *	srvc - Pointer to service request structure.
 *
 *  IMPLICIT INPUTS:
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *	 None
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
cdisk_srvc_req( dev, srvc ) 
	dev_t		dev;		/* Device opened		*/
	SRVC_REQ	*srvc;		/* Ptr to service struct	*/
{

    /*
     * Local declarations
     */

    PDRV_DEVICE		*pd;	/* Ptr to pdrv struct 			*/
    DEV_DESC		*dd;	/* Ptr to device descripor		*/
    char		*cp;	/* character pointer			*/
    U32 		i;
    U32			found;

    /* 
     * Before comparing strings make sure they are null terminated.
     */
    cp = srvc->srvc_name;
    for( i = 0, found = 0; i < SRVC_NAME_SIZE; i++){
	if( cp[i] == NULL){
	    found++;
	    break;
	}
    }
    if( found == 0){
	return(EINVAL);
    }
    cp = srvc->srvc_io;
    for( i = 0, found = 0; i < SRVC_NAME_SIZE; i++){
	if( cp[i] == NULL){
	    found++;
	    break;
	}
    }
    if( found == 0){
	return(EINVAL);
    }
    cp = srvc->srvc_drvr;
    for( i = 0, found = 0; i < SRVC_NAME_SIZE; i++){
	if( cp[i] == NULL){
	    found++;
	    break;
	}
    }
    if( found == 0){
	return(EINVAL);
    }
	


    /* 
     * Checks to make sure this is for us.
     */
    if( strcmp( srvc->srvc_io, SCSI_IO_SUB) != NULL){
	cp = srvc->srvc_io;
	 return(EINVAL);
    }
    if( strcmp( srvc->srvc_drvr,  DISK_DRIVER) != NULL){
	 return(EINVAL);
    }

    /*
     * Now check the service classification, we only support RAID    
     */
    if( strcmp( srvc->srvc_name, NSTD_RAID_SERVICE) == NULL){
	return(cdisk_raid_srvc( dev, srvc, (NSTD_RAID *)srvc->srvc_buffer));
    }
    else {
	return(EINVAL);
    }


}



/************************************************************************
 *
 *  ROUTINE NAME: cdisk_raid_srvc()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function handles all RAID service requests. The call 
 *	is from cdisk_srvc by detecting that this is a RAID service request 
 *	ioctl. The functionality that this routine does is to decode what 
 *	RAID service is requested by the user.
 *
 *  FORMAL PARAMETERS:
 *	dev	 - Dev number.
 *	srvc - Pointer to service request structure.
 *	raid -  Pointer to RAID request structure.
 *
 *  IMPLICIT INPUTS:
 *	RAID structure members.
 *
 *  IMPLICIT OUTPUTS:
 *	 RAID structure members.
 *
 *  RETURN VALUE:
 *	Always Success.
 *
 *  SIDE EFFECTS:
 *	None.
 *
 *  ADDITIONAL INFORMATION:
 *	None.
 *
 ************************************************************************/

cdisk_raid_srvc( dev, srvc, raid)

    dev_t	dev;	/* device major minor		*/
    SRVC_REQ	*srvc;	/* Pointer to service struct.	*/
    NSTD_RAID	*raid;	/* Pointer to RAID service struct. */

{
    /* Local variables */

    /* 
     * Check to make sure that the RAID struct is ok (size, access)
     */
    if( cdisk_check_buffer(srvc, raid) != CAM_SUCCESS){
	/* 
	 * We return SUCCESS so the error info is copied back to user...
	 */
	return(CAM_SUCCESS);
    }
    /*
     * RAID service struct is ok and data areas locked down....
     * Lets find out what we need to do... look at opcode....
     */
    switch( raid->nstd_opcode ) {
	case  NSTD_GET_CTRL_NUM:
	case NSTD_GET_CTRL_LIST:
	    get_num_ctrl( srvc, raid );
	    break;
	case NSTD_CONFIG:
	    cdisk_raid_config(  srvc, raid );
	    break;
	    
	case NSTD_MAINT:
	    cdisk_raid_maint(  srvc, raid );
	    break;
    
    } /* end of switch for opcode. */

    /* 
     * unlock the users buffer...
     */
    (void) srvc_unlock(raid);
    return(CAM_SUCCESS);
}

/************************************************************************
 *
 *  ROUTINE NAME: cdisk_close_dev(pd, pdu, pds, dev)
 *
 *  FUNCTIONAL DESCRIPTION:
 *	
 *	Detemines if a close of the unit is necessary and then closes
 *	unit.  This routine gets called by the RAID service IOCTL routines
 *	cidsk_raid_config() and cdisk_raid_maint().
 *
 *  FORMAL PARAMETERS:
 *      pd - Pointer to PDRV_DEVICE structure.
 *      pdu -  Pointer to the Unit table slot.
 *	pds - Pointer to disk specific struct
 *	dev - dev number of the device.
 *
 *  IMPLICIT INPUTS:
 *      None
 *
 *  IMPLICIT OUTPUTS:
 *       None
 *
 *  RETURN VALUE:
 *      None.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
 ************************************************************************/

void
cdisk_close_dev(pd, pdu, pds, dev)
    PDRV_DEVICE	*pd;	/* Pointer to pdrv struct	*/
    PDRV_UNIT_ELEM *pdu; /* Pointer to unit table slot	*/
    DISK_SPECIFIC *pds;	/* Pointer to disk specific struct */
    dev_t dev;		/* Major/minor pair		*/
{
    /*
     * Local variables
     */
    CCB_SETASYNC  *ccb;
    int spl, spl_pd;	/* spl's for locks		*/

    /* 
     * Do the locks
     */
    PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl_pd); 
    PDU_IPLSMP_LOCK(LK_ONCE, spl);   /* lock unit table */
    if (( pdu->pu_opens == 1) && (pds->ds_openpart == NULL)) {
	PDU_IPLSMP_UNLOCK(spl);
	PDRV_IPLSMP_UNLOCK(pd, spl_pd);
	/*
	 * Deregister for call back..
	 */
        ccb = ccmn_sasy_ccb_bld(dev, (U32)0,
		(U32)0, cdisk_async_cb, (u_char *)NULL, 0);
	ccmn_close_unit(dev);
        ccmn_rel_ccb((CCB_HEADER *)ccb);
    }
    else{
	PDRV_IPLSMP_UNLOCK(pd, spl_pd);
	PDU_IPLSMP_UNLOCK(spl);
    }
}


/************************************************************************
 *
 *  ROUTINE NAME: cdisk_raid_config()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles all RAID config service requests. 
 *	The call is from cdisk_raid_srvc by detecting that this is a 
 *	config service request ioctl. All checking is done here for these 
 *	commands. (Allowability)
 *	
 *
 *  FORMAL PARAMETERS:
 *      srvc - Pointer to service request structure.
 *      raid -  Pointer to RAID request structure.
 *
 *  IMPLICIT INPUTS:
 *      RAID structure members.
 *
 *  IMPLICIT OUTPUTS:
 *       RAID structure members.
 *
 *  RETURN VALUE:
 *      Sucess.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
 ************************************************************************/

int
cdisk_raid_config( srvc, raid)
    SRVC_REQ    *srvc;   /* Pointer to service struct.   */
    NSTD_RAID   *raid;   /* Pointer to RAID service struct. */
{
    /* Local Variables */
    PDRV_DEVICE	*pd;	/* Peripheral device struct of effected lun */
    PDRV_DEVICE	*dir_pd;	/* Peripheral device struct cmd set to	*/
    DISK_SPECIFIC *pds; /* Disk specific struct	of effected lun	*/
    DISK_SPECIFIC *dir_pds; /* Disk specific struct cmd sent to  */
    PDRV_UNIT_ELEM *pdu, *dir_pdu; /* Unit table pointers */ 
    CCB_SCSIIO	*ccb;	/* ccb that we use. */
    dev_t	dev;	/* Make up dev number based bus target lun */
    dev_t 	dir_dev; /* Make up dev number based on bus target 
			  * and directed_lun */
    U32		open_flags, dir_open_flags; /* Semi-exlcusive or normal */
    int 	spl, spl_pd; /* for locks		*/
    int		status;
    U32		open_dir = 0; /* Flag that syas open directed lun */
    U32		do_online = 0; /* Flag to indicate to call online. */

    /*
     * Figure out what type command this is ..
     */
    switch ( raid->nstd_subop )
    {
    /* 
     * Commands that only require that answer up.
     */
    /*
     * DATA IN or none 
     */
    case SUB_GET_PHYS:
    case SUB_GET_LUN_STAT:
    case SUB_GET_LV_STAT:
    case SUB_GET_SPARE_STAT:
	if((( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_IN) &&
		(( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_NONE)){

	    strcpy(srvc->srvc_str, dir_str); /* copy user string up */
	    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	    srvc->srvc_errno = EINVAL;
	    return( EINVAL );
	}
	open_flags = 0;
        break;

    /* 
     * Commands that require we have semi exclusive access
     */

    case SUB_SET_PHYS:
    case SUB_CREATE_LV:
    case SUB_DELETE_LV:
    case SUB_MODIFY_LV:
    case SUB_RECON_LV:
    case SUB_CREATE_SPARE_SET:
    case SUB_DELETE_SPARE_SET:
    case SUB_MODIFY_SPARE_SET:
	if((( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_OUT) &&
		(( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_NONE)){

	    strcpy(srvc->srvc_str, dir_str); /* copy user string up */
	    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	    srvc->srvc_errno = EINVAL;
	    return( EINVAL );
	}
	open_flags = 0;
        break;

    /* 
     * Commands that require we have semi exclusive access
     */
    case SUB_CREATE_LV_LUN:
    case SUB_CREATE_LUN:
    case SUB_DELETE_LV_LUN:
    case SUB_DELETE_LUN:
    case SUB_MODIFY_LUN:
	if((( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_OUT) &&
		(( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_NONE)){

	    strcpy(srvc->srvc_str, dir_str); /* copy user string up */
	    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	    srvc->srvc_errno = EINVAL;
	    return( EINVAL );
	    break;
	}
	if( raid->nstd_lun == raid->nstd_directed_lun) {
	    open_flags = CCMN_EXCLUSIVE;
	}
	else {
	    dir_open_flags = CCMN_EXCLUSIVE;
	    open_dir = 1;
	}

	break;
    default:
	strcpy(srvc->srvc_str, cfg_subop_str); /* copy user string up */
	srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	srvc->srvc_errno = EINVAL;
	return( EINVAL );
	break;

    } /* End of switch */

    /* 
     * Get our dev_t's
     */
    dev = makedev(0, MAKEMINOR(MAKE_UNIT( raid->nstd_bus,  raid->nstd_target,  
		raid->nstd_lun), 0));
    dir_dev = makedev(0, MAKEMINOR(MAKE_UNIT( raid->nstd_bus,  raid->nstd_target,  
		raid->nstd_directed_lun), 0));

    /* 
     * Open our unit that the cmd is sent to 
     */
    if(( status = ccmn_open_unit(dev, (U32)ALL_DTYPE_DIRECT,
	    open_flags, (U32)sizeof(DISK_SPECIFIC))) != (U32)0){
	if( status == EBUSY ){
	    strcpy(srvc->srvc_str, busy_str); /* copy user string up */
            srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
            srvc->srvc_errno = EBUSY;
            return( EBUSY );
	}
	else {
	    strcpy(srvc->srvc_str, open_str); /* copy user string up */
            srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
            srvc->srvc_errno = ENODEV;
            return( ENODEV );
	}
    }
    pdu = GET_PDRV_UNIT_ELEM(dev);

    if((pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL){
        strcpy(srvc->srvc_str, data_str); /* copy user string up */
        srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
        srvc->srvc_errno = ENOMEM;
        /* 
         * since we don't have a pdrv can't close unit
         */
        return( ENOMEM );
    }	    
    if(( pds = (DISK_SPECIFIC *)pd->pd_specific) == (DISK_SPECIFIC *)NULL){
        strcpy(srvc->srvc_str, data_str); /* copy user string up */
        srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
        srvc->srvc_errno = ENOMEM;
        return( ENOMEM );
    }
    /* 
     * open the lun 
     */
    if (open_dir != NULL ){
        if((status = ccmn_open_unit(dir_dev, 
		(U32)ALL_DTYPE_DIRECT,
	         dir_open_flags, (U32)sizeof(DISK_SPECIFIC))) != (U32)0) {
	    if( status == EBUSY ){
	        strcpy(srvc->srvc_str, busy_str); /* copy user string up */
                srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                srvc->srvc_errno = EBUSY;
	    }
	    else {
                strcpy(srvc->srvc_str, open_str); /* copy user string up */
                srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                srvc->srvc_errno = ENODEV;
	    }
            /*
             * Must close other unit...
             */
	    cdisk_close_dev( pd, pdu, pds, dev);
	    if(status == EBUSY){
		return(EBUSY);
	    }
            return( ENODEV );

        }
        /* 
	 * Get our structure pointers
         */
        dir_pdu = GET_PDRV_UNIT_ELEM(dir_dev);



	if((dir_pd = GET_PDRV_PTR(dir_dev)) == (PDRV_DEVICE *)NULL) {
	    strcpy(srvc->srvc_str, data_str); /* copy user string up */
	    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	    srvc->srvc_errno = ENOMEM; 
	    /*
	     * Must close other unit...  
	     */
	    cdisk_close_dev( pd, pdu, pds, dev);
	    return( ENOMEM ); 
	}

	if(( dir_pds = (DISK_SPECIFIC *)dir_pd->pd_specific) ==
			(DISK_SPECIFIC *)NULL){
	    strcpy(srvc->srvc_str, data_str); /* copy user string up */
	    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	    srvc->srvc_errno = ENOMEM; 
	    /*
	     * Must close other unit...  
	     */
	    cdisk_close_dev( pd, pdu, pds, dev);
	    return( ENOMEM ); 
	}


    } /* end of if open_dir  */


    /* 
     * Lets do the command.....
     */
    ccb = cdisk_build_raid_ccb( srvc, raid, dev, pd, cdisk_raid_complete); 
    if( ccb == (CCB_SCSIIO *)NULL ){
	/* 
	 * No CCB's 
	 */
        strcpy(srvc->srvc_str, data_str); /* copy user string up */
        srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
        srvc->srvc_errno = ENOMEM;
        /*
         * Must close both luns (if needed)...
         */
	cdisk_close_dev( pd, pdu, pds, dev);
    
        if( open_dir ){
            /*
             * Since the effected lun and the one we send the cmd
	     * to is different.. must close out the one we send to.
             */
	    cdisk_close_dev( dir_pd, dir_pdu, dir_pds, dir_dev);
	
        }
        return( srvc->srvc_errno );
    }

    /* 
     * well we have a built ccb
     * do the command
     */
    PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl_pd);
    (void) cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, NOT_RETRY,
	     PZERO);
    PDRV_IPLSMP_UNLOCK(pd, spl_pd);

    /* 
     * If we got here then the ccb came back to us..
     * Check for good completion... And maybe retry for
     * UNIT ATTEN"S.
     */

    if( CAM_STATUS(ccb) == CAM_REQ_CMP ){
        /* release resources */

        cdisk_raid_rel_res( ccb ); 

    }
    else {
        cdisk_raid_cmd_error( srvc, raid, ccb);
        cdisk_raid_rel_res( ccb );
    }

    /*
     * Done the command for better or worse..
     * Must close both luns (if needed)...
     */
    cdisk_close_dev( pd, pdu, pds, dev);
    
    if( open_dir ){
        /*
         * Close out the effected lun
         */
        cdisk_close_dev( dir_pd, dir_pdu, dir_pds, dir_dev);
    }
    /* 
     * Bring it online....
     */

     /*
      * raid->nstd_error will return 0 (initialized state) unless
      *	an error occured.  Then cdisk_raid_cmd_error will fill in
      * the nstd_error field.
      */
    return( raid->nstd_error );
}


/************************************************************************
 *
 *  ROUTINE NAME: cdisk_raid_maint()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles all RAID maint service requests. 
 *	The call is from cdisk_raid_srvc by detecting that this is a maint 
 *	service request ioctl. All checking is done here for these commands. 
 *	(Allowability)
 *	
 *
 *  FORMAL PARAMETERS:
 *      srvc - Pointer to service request structure.
 *      raid -  Pointer to RAID request structure.
 *
 *  IMPLICIT INPUTS:
 *      RAID structure members.
 *
 *  IMPLICIT OUTPUTS:
 *       RAID structure members.
 *
 *  RETURN VALUE:
 *      Sucess.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
 ************************************************************************/

int
cdisk_raid_maint( srvc, raid)
    SRVC_REQ    *srvc;   /* Pointer to service struct.   */
    NSTD_RAID   *raid;   /* Pointer to RAID service struct. */
{
    /* Local Variables */
    PDRV_DEVICE	*pd;	/* Peripheral device struct of effected lun */
    PDRV_DEVICE	*dir_pd;	/* Peripheral device struct cmd set to	*/
    DISK_SPECIFIC *pds; /* Disk specific struct	of effected lun	*/
    DISK_SPECIFIC *dir_pds; /* Disk specific struct cmd set to  */
    PDRV_UNIT_ELEM *pdu, *dir_pdu; /* Unit table pointers */ 
    CCB_SCSIIO	*ccb;	/* ccb that we use. */
    dev_t	dev;	/* Make up dev number based bus target lun */
    dev_t 	dir_dev; /* Make up dev number based on bus target 
			  * and directed_lun */
    U32		open_flags, dir_open_flags; /* Semi-exlcusive or normal */
    int 	spl, spl_pd; /* for locks		*/
    int		status;
    U32		open_dir = 0; /* Flag that syas open directed lun */
    U32		do_online = 0; /* Flag to indicate to call online. */

    /*
     * Figure out what type command this is ..
     */
    switch ( raid->nstd_subop )
    {
    /* 
     * Commands that only require that answer up.
     */
    /*
     * DATA IN or none 
     */
    case SUB_GET_DEVICE_STAT:
    case SUB_GET_CTRL_STAT:
	if((( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_IN) &&
		(( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_NONE)){

	    strcpy(srvc->srvc_str, dir_str); /* copy user string up */
	    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	    srvc->srvc_errno = EINVAL;
	    return( EINVAL );
	}
	open_flags = 0;
        break;

    /* 
     * Commands that require we have semi exclusive access
     */

    case SUB_SET_DEVICE_STAT:
	if((( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_OUT) &&
		(( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_NONE)){

	    strcpy(srvc->srvc_str, dir_str); /* copy user string up */
	    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	    srvc->srvc_errno = EINVAL;
	    return( EINVAL );
	}
	open_flags = 0;
        break;

    /* 
     * Commands that require we have semi exclusive access
     */
    case SUB_FORMAT_LV:
	if((( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_OUT) &&
		(( raid->nstd_flags & NSTD_DATA_MASK) !=  NSTD_DATA_NONE)){

	    strcpy(srvc->srvc_str, dir_str); /* copy user string up */
	    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	    srvc->srvc_errno = EINVAL;
	    return( EINVAL );
	    break;
	}
	if( raid->nstd_lun == raid->nstd_directed_lun) {
	    open_flags = CCMN_EXCLUSIVE;
	}
	else {
	    dir_open_flags = CCMN_EXCLUSIVE;
	    open_dir = 1;
	}

	break;

    case SUB_SET_PHYS_DEV_MAINT:
    case SUB_CLEAR_PHYS_DEV_MAINT:
    case SUB_MAINT_PASS:
	if( raid->nstd_lun == raid->nstd_directed_lun) {
	    open_flags = CCMN_EXCLUSIVE;
	}
	else {
	    dir_open_flags = CCMN_EXCLUSIVE;
	    open_dir = 1;
	}

	break;

    case SUB_SET_CTRL_STAT:
    case SUB_MARK_CTRL_FAIL:
    case SUB_MARK_CTRL_ALIVE:
    case SUB_GET_REDUND_CTRL:
    case SUB_SET_REDUND_CTRL:
	strcpy(srvc->srvc_str, notsup_str); /* copy user string up */
	srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	srvc->srvc_errno = ENOTSUP;
	return( ENOTSUP );
	break;

    default:
	strcpy(srvc->srvc_str, maint_subop_str); /* copy user string up */
	srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
	srvc->srvc_errno = EINVAL;
	return( EINVAL );
	break;

    } /* End of switch */

    /* 
     * Get our dev_t's
     */
    dev = makedev(0, MAKEMINOR(MAKE_UNIT( raid->nstd_bus,  raid->nstd_target,  
		raid->nstd_lun), 0));
    dir_dev = makedev(0, MAKEMINOR(MAKE_UNIT( raid->nstd_bus,  raid->nstd_target,  
		raid->nstd_directed_lun), 0));

    /* 
     * Open our unit that the cmd is sent to  only if not
     * passthru or clearing the maint bit.
     */
    if(( raid->nstd_subop != SUB_CLEAR_PHYS_DEV_MAINT) && (raid->nstd_subop !=
		SUB_MAINT_PASS)){
        if((status = ccmn_open_unit(dev, (U32)ALL_DTYPE_DIRECT,
	        open_flags, (U32)sizeof(DISK_SPECIFIC))) != (U32)0) {
	    if( status == EBUSY ){
	        strcpy(srvc->srvc_str, busy_str); /* copy user string up */
                srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                srvc->srvc_errno = EBUSY;
	    }
	    else {
	        strcpy(srvc->srvc_str, open_str); /* copy user string up */
                srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                srvc->srvc_errno = ENODEV;
	    }
	    if(status == EBUSY){
	        return( EBUSY );
	    }
            return( ENODEV );
        }
    }
    pdu = GET_PDRV_UNIT_ELEM(dev);

    if((pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL){
        strcpy(srvc->srvc_str, data_str); /* copy user string up */
        srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
        srvc->srvc_errno = ENOMEM;
        /* 
         * since we don't have a pdrv can't close unit
         */
        return( ENOMEM );
    }	    
    if(( pds = (DISK_SPECIFIC *)pd->pd_specific) == (DISK_SPECIFIC *)NULL){
        strcpy(srvc->srvc_str, data_str); /* copy user string up */
        srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
        srvc->srvc_errno = ENOMEM;
        return( ENOMEM );
    }
    /* 
     * open the lun  if not a passthru or clearing of maint 
     */
    if(( raid->nstd_subop != SUB_CLEAR_PHYS_DEV_MAINT) && (raid->nstd_subop !=
		SUB_MAINT_PASS)){
        if (open_dir != NULL ){
            if((status = ccmn_open_unit(dir_dev, (U32)ALL_DTYPE_DIRECT,
	             dir_open_flags, (U32)sizeof(DISK_SPECIFIC)) != (U32)0))   {
	        if( status == EBUSY ){
	            strcpy(srvc->srvc_str, busy_str); /* copy user string up */
                    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                    srvc->srvc_errno = EBUSY;
	        }
	        else {
                    strcpy(srvc->srvc_str, open_str); /* copy user string up */
                    srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                    srvc->srvc_errno = ENODEV;
	        }
                /*
                 * Must close other unit...
                 */
    	        cdisk_close_dev( pd, pdu, pds, dev);
	        if(status == EBUSY){
	            return( EBUSY );
	        }
                return( ENODEV );
            }
            /* 
	     * Get our structure pointers
             */
            dir_pdu = GET_PDRV_UNIT_ELEM(dir_dev);

            if((dir_pd = GET_PDRV_PTR(dir_dev)) == (PDRV_DEVICE *)NULL){
                strcpy(srvc->srvc_str, data_str); /* copy user string up */
                srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                srvc->srvc_errno = ENOMEM;
                /*
                 * Must close other unit...
                 */
    	        cdisk_close_dev( pd, pdu, pds, dev);
                return( ENOMEM );
            }
	    
            if(( dir_pds = (DISK_SPECIFIC *)dir_pd->pd_specific) == 
			(DISK_SPECIFIC *)NULL){
                strcpy(srvc->srvc_str, data_str); /* copy user string up */
                srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                srvc->srvc_errno = ENOMEM;
                /*
                 * Must close other unit...
                 */
    	        cdisk_close_dev( pd, pdu, pds, dev);
                return( ENOMEM );
            }
	        
    
        } /* end of if open_dir  */
    }

    /* 
     * For cmds that are maint pass thru..
     */
    if( raid->nstd_subop == SUB_SET_PHYS_DEV_MAINT){
	/* 
	 * set the maint bit for this directed lun.
	 */
	if( open_dir ){
	    /* Must lock the PDRV */
	    PDRV_IPLSMP_LOCK(dir_pd,LK_RETRY, spl_pd); 
	    dir_pd->pd_flags |= PD_MAINT;
	    PDRV_IPLSMP_UNLOCK(pd_dir, spl_pd);
	}
	else {
	    /* Must lock the PDRV */
	    PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl_pd); 
	    pd->pd_flags |= PD_MAINT;
	    PDRV_IPLSMP_UNLOCK(pd, spl_pd);
	}
        return( raid->nstd_error );
    }
    if( raid->nstd_subop == SUB_CLEAR_PHYS_DEV_MAINT){
	/* 
	 * clear the maint bit for this directed lun.
	 */
	if( open_dir ){
	    /* Must lock the PDRV */
	    PDRV_IPLSMP_LOCK(dir_pd,LK_RETRY, spl_pd); 
	    dir_pd->pd_flags &= ~PD_MAINT;
	    PDRV_IPLSMP_UNLOCK(dir_pd, spl_pd);
	    /*
	     * Close the directed lun and then close where 
	     * we sent the commnad to
	     */
            dir_pdu = GET_PDRV_UNIT_ELEM(dir_dev);
            dir_pd = GET_PDRV_PTR(dir_dev);
            dir_pds = (DISK_SPECIFIC *)dir_pd->pd_specific;
    	    cdisk_close_dev( dir_pd, dir_pdu, dir_pds, dir_dev);
    	    cdisk_close_dev( pd, pdu, pds, dev);
	}
	else {
	    /* Must lock the PDRV */
	    PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl_pd); 
	    pd->pd_flags &= ~PD_MAINT;
	    PDRV_IPLSMP_UNLOCK(pd, spl_pd);
    	    cdisk_close_dev( pd, pdu, pds, dev);
	}
        return( raid->nstd_error );
    }
    if( raid->nstd_subop == SUB_MAINT_PASS){
	/* 
	 * set the check the maint. bit for this directed lun.
	 */
	if( open_dir ){

            dir_pdu = GET_PDRV_UNIT_ELEM(dir_dev);
            dir_pd = GET_PDRV_PTR(dir_dev);
            dir_pds = (DISK_SPECIFIC *)dir_pd->pd_specific;

	    /* Must lock the PDRV */
	    PDRV_IPLSMP_LOCK(dir_pd,LK_RETRY, spl_pd); 
	    /*
	     * Check to see if they have set maintenance  up....
	     */
	    if(( dir_pd->pd_flags & PD_MAINT ) == NULL){
                /*
                 * Since the effected lun and the one we send the cmd
	         * to is different.. 
                 */
		PDRV_IPLSMP_UNLOCK(dir_pd, spl_pd);
                strcpy(srvc->srvc_str, maint_pass_str); 
                srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                srvc->srvc_errno = EINVAL;
		return(EINVAL);
	    }
	    else {
		 PDRV_IPLSMP_UNLOCK(dir_pd, spl_pd);
	    }


	}
	else {
	    /* Must lock the PDRV */
	    PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl_pd); 
	    if(( pd->pd_flags & PD_MAINT ) == NULL){
	        PDRV_IPLSMP_UNLOCK(pd, spl_pd);
                strcpy(srvc->srvc_str, maint_pass_str); 
                srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
                srvc->srvc_errno = EINVAL;
		return(EINVAL);
	    }
	    else {
		 PDRV_IPLSMP_UNLOCK(pd, spl_pd);
	    }
	}
    }
    /* 
     * Lets do the command.....
     */
    ccb = cdisk_build_raid_ccb( srvc, raid, dev, pd, cdisk_raid_complete); 
    if( ccb == (CCB_SCSIIO *)NULL ){
	/* 
	 * No CCB's 
	 */
        strcpy(srvc->srvc_str, data_str); /* copy user string up */
        srvc->srvc_flags |=  (STR_MSG_VALID | SRVC_ERROR);
        srvc->srvc_errno = ENOMEM;

	/*
	 * If maint passthru don't close luns
	 */
        if( raid->nstd_subop == SUB_MAINT_PASS){
	    return(srvc->srvc_errno );
	}

        /*
         * Must close both luns (if needed)...
         */
    	cdisk_close_dev( pd, pdu, pds, dev);
    
        if( open_dir ){
            /*
             * Since the effected lun and the one we send the cmd
	     * to is different.. must close out the one we send to.
             */
    	    cdisk_close_dev( dir_pd, dir_pdu, dir_pds, dir_dev);

        }
	return(srvc->srvc_errno );
    }

    /* 
     * well we have a built ccb
     * do the command
     */
    PDRV_IPLSMP_LOCK(pd,LK_RETRY, spl_pd);
    (void) cdisk_send_ccb_wait(pd, (CCB_HEADER *)ccb, NOT_RETRY,
	     PZERO);
    PDRV_IPLSMP_UNLOCK(pd, spl_pd);

    /* 
     * If we got here then the ccb came back to us..
     * Check for good completion... And maybe retry for
     * UNIT ATTEN"S.
     */

    if( CAM_STATUS(ccb) == CAM_REQ_CMP ){
        /* release resources */

        cdisk_raid_rel_res( ccb ); 
    }
    else {
        cdisk_raid_cmd_error( srvc, raid, ccb);
        cdisk_raid_rel_res( ccb );
    }
    /*
     * If maint passthru don't close luns
     */
    if( raid->nstd_subop == SUB_MAINT_PASS){
        return( raid->nstd_error );
    }

    /*
     * Done the command for better or worse..
     * Must close both luns (if needed)...
     */
    cdisk_close_dev( pd, pdu, pds, dev);
    
    if( open_dir ){
        /*
         * Close out the effected lun
         */
        cdisk_close_dev( dir_pd, dir_pdu, dir_pds, dir_dev);
    }
    /* 
     * Bring it online....
     */
    return( raid->nstd_error );
}

/************************************************************************
 *
 *  ROUTINE NAME: cdisk_raid_rel_res( ccb )
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine will release the bp back to the pools if non null
 *	then release the the simq if frozen then the ccb.
 *	
 *
 *  FORMAL PARAMETERS:
 *      ccb - Pointer the CCB_SCSIIO 
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *       Release of the queue if frozen. 
 *
 *  RETURN VALUE:
 *      None.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
 ************************************************************************/

cdisk_raid_rel_res( ccb )
    CCB_SCSIIO	*ccb;	/* The ccb pointer	*/
{
    /* 
     * Local variables
     */
    PDRV_WS *pws;

    pws = (PDRV_WS *)ccb->cam_pdrv_ptr;

    ccmn_rem_ccb((PDRV_DEVICE *)pws->pws_pdrv, ccb);

    if( ccb->cam_req_map != NULL){
	ccmn_rel_bp((struct buf *)ccb->cam_req_map);
    }
    CHK_RELEASE_QUEUE( (PDRV_DEVICE *)pws->pws_pdrv, ccb);

    ccmn_rel_ccb((CCB_HEADER *)ccb);

}

 /************************************************************************
 *
 *  ROUTINE NAME:  get_num_ctrl()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	If the nstd_opcode is NSTD_GET_CTRL_NUM then this routine
 *      will determine and return the number of non-standard RAID
 *      controllers seen in the system.
 *
 *      If the nstd_opcode is NSTD_GET_CTRL_LIST then this routine
 *      will determine and return the bus, target, vid, and pid for
 *      each non-standard RAID controller seen in the system.
 *
 *  FORMAL PARAMETERS:
 *	srvc_req - Pointer to service request structure
 *	nstd_raid - Pointer to non-standard RAID structure 
 *
 *  IMPLICIT INPUTS:
 *      pdrv_unit_table
 *	pdrv_device
 *      disk_specific
 *
 *  IMPLICIT OUTPUTS:
 *  	If the nstd_opcode is NSTD_GET_CTRL_NUM then the 
 *      nstd_data field of nstd_raid structure will contain the number
 *      of non-standard RAID controllers seen.
 *
 *  	If the nstd_opcode is NSTD_GET_CTRL_LIST then the 
 *      nstd_data field of nstd_raid structure will contain  an array
 *	of NSTD_CTRL structures.  There is one NSTD_CTRL structure for
 *	each non-standard RAID controller seen.   The nstd_data_resid
 *	field indicates the number of NSTD_CTRL structures in the data
 *	area.  The nstd_sense_resid field will indicate the number of
 *	NSTD_CTRL structures NOT in the data area due to the data area
 *	not being large enough.
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

int
get_num_ctrl(srvc_req, nstd_raid)
	SRVC_REQ *srvc_req;
	NSTD_RAID *nstd_raid;
{

	PDRV_DEVICE	*pd;	/* Pointer to peripheral device struct */
	DISK_SPECIFIC	*dsp;	/* Pointer to disk specific struct */
	DEV_DESC	*dd;	/* Pointer to device descriptor struct */
	CCB_PATHINQ	*ccb;	/* Pointer to Path Inquiry struct */
	U32		target, bus, slot; /* Step values... */
	int		status;	/* Return status */
	caddr_t		data_area; /* tmp pointer */
	U32		nctrl = 0;	/* Number of controllers found */
	int		LUN = 0; /* LUN stays at zero.  Just looking for */
				 /* targets */
	dev_t		dev;	/* CAM dev_t */
	int		spl, t_spl;	/* priority level */

	static u_char	module[] = "get_num_ctrl";

	/*
	 * For each bus in the system...
	 */
	for (bus = 0; bus < nCAMBUS; bus++)
	{
		/*
		 * Scan the controller to see if any additional devices
		 * are now present
		 */
		ccfg_edtscan(EDT_FULLSCAN, bus, 0, 0);

		/*
		 * For each device on the bus...
		 */
		for (target = 0; target < NDPS; target++)
		{
			/*
			 * Calculate the unit table index using B,T,L
			 * Lun is always zero because we're only
			 * looking for targets
			 */
			slot =  (((bus * NDPS) * NLPT) + 
				 ((target * NLPT) + LUN));
			/*
			 * If a peripheral device structure exists and
			 * the device type is DIRECT ACCESS
			 */
			if ((pdrv_unit_table[slot].pu_device) &&
			   (pdrv_unit_table[slot].pu_type == ALL_DTYPE_DIRECT))
                        {
				int t_spl;

				/*
				 * Get the pointer to the peripheral
				 * device structure and lock it
				 */
				pd =  pdrv_unit_table[slot].pu_device;
				PDRV_IPLSMP_LOCK(pd, LK_ONCE, t_spl);
				/*
				 * Get the pointer to the device
				 * descriptor structure
				 */
				dd = pd->pd_dev_desc;
				/* 
				 * If it's a RAID controller
				 * increment the count and get the
				 * controller information if the
				 * nstd opcode indicates so
				 */
				if (dd->dd_device_type & SZ_RAID)
				{
					nctrl++;
					if (nstd_raid->nstd_opcode == 
						NSTD_GET_CTRL_LIST) 
					{
						get_ctrl_info(nstd_raid,
							nctrl, pd);
					}
				}
				/*
				 * All done with this pd structure so
				 * unlock it
				 */
				PDRV_IPLSMP_UNLOCK(pd, t_spl);
                        }
                        else 
			{
				/* If the peripheral device structure 
				 * for this B,T,L is not configured then
				 * try to see if something's there...
				 */
				if (pdrv_unit_table[slot].pu_device == NULL)
				{
					/*
				 	 * Need a dev for the open  so...
					 * make one.
					 */
					dev = makedev(0, 
						MAKEMINOR(
						MAKE_UNIT(bus, target, 0),
						0));
					/*
					 * We're looking for DIRECT
					 * ACCESS devices
					 */
					if (status = ccmn_open_unit(dev, 
						ALL_DTYPE_DIRECT, 0,
				        	sizeof(DISK_SPECIFIC)) != 0)
						continue;

					/* 
					 * We've got a direct access
					 * device...get the pointer to 
					 * the peripheral device struct
					 * and lock it
					 */
					pd =  pdrv_unit_table[slot].pu_device;
					PDRV_IPLSMP_LOCK(pd, LK_ONCE, 
							t_spl);
					/*
					 * Get a pointer to the device
					 * descriptor struct
					 */
					dd = pd->pd_dev_desc;
					/* 
					 * If it's a RAID controller
					 * increment the count and get the
					 * controller information if the
					 * nstd opcode indicates so
					 */
					if (dd->dd_device_type & SZ_RAID)
					{
						nctrl++;
						if (nstd_raid->nstd_opcode == 
							NSTD_GET_CTRL_LIST)
						{
							get_ctrl_info(nstd_raid,
								nctrl,
								pd);
						}
					}
					/*
					 * We're all done with
					 * this pd so unlock it
					 */
					PDRV_IPLSMP_UNLOCK(pd, t_spl);
					/*
					 * close the opened
					 * device
					 */
					cdisk_close_dev(pd, &pdrv_unit_table[slot], 
						pd->pd_specific, dev);
				}		
                        }
		} /* end for target... */
	} /* end for bus... */
	/*
	 * If the nstd_raid opcode is NSTD_GET_NUM_CTRL, get the number of 
	 * controllers, then fill the number of controllers found into the 
	 * data area.
	 */
 	if (nstd_raid->nstd_opcode == NSTD_GET_CTRL_NUM)
	{
		data_area = (caddr_t)nstd_raid->nstd_data_area;
		*data_area = nctrl;
		/*
		(U32 *)(nstd_raid)->nstd_data_area = nctrl;
		*/
	}

	return(CAM_SUCCESS);
}

 /************************************************************************
 *
 *  ROUTINE NAME:  get_ctrl_info()
 *      This routines fills in the controller information which is
 *      stored in the pd & dd into the RAID_CTRL structure.  Info
 *      includes bus, target, VID, PID.
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *  FORMAL PARAMETERS:
 *	nstd_raid - Pointer to non-standard RAID structure 
 *	num_ctrl - number of controllers found at this function call
 *	pd - pointer to the peripheral device structure
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *	nstd_data_area field will contain information about the
 *	controller (bus, target, vid, pid)
 *
 *	If the data area is large enough to contain this controller's
 *	information, nstd_data_resid will be incremented by one.
 *	Else, nstd_sense_resid will be incremented by one.
 *
 *  RETURN VALUE:
 *	CAM_SUCCESS
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

int
get_ctrl_info(nstd_raid, num_ctrl, pd)
	NSTD_RAID *nstd_raid;
	int num_ctrl;
	PDRV_DEVICE *pd;
{
	DEV_DESC	*dd;	/* Pointer to device descriptor struct */
	int		offset;	/* offset into data area */
	RAID_CTRL	*raid_ctrl;	/* Pointer to RAID controller */
					/* struct */

	if ((num_ctrl * (sizeof(RAID_CTRL)) <= nstd_raid->nstd_data_len)) 
	{
		offset = ((num_ctrl-1) * sizeof(RAID_CTRL));
		raid_ctrl = ((RAID_CTRL *) &(nstd_raid->nstd_data_area[offset]));
		raid_ctrl->ctrl_bus = pd->pd_bus;
		raid_ctrl->ctrl_target = pd->pd_target;
		dd = pd->pd_dev_desc;
		bcopy(dd->dd_pv_name, raid_ctrl->ctrl_vid, VID_STRING_SIZE);
		bcopy(&(dd->dd_pv_name[VID_STRING_SIZE]),
			raid_ctrl->ctrl_pid,
			PID_STRING_SIZE);
		nstd_raid->nstd_data_resid += 1;
	}
	else
	{
		nstd_raid->nstd_sense_resid += 1;
	}
	return(CAM_SUCCESS);
}
 /************************************************************************
 *
 *  ROUTINE NAME:  cdisk_check_buffer()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This routine validates the user buffer supplied in the
 *      rd_srvc_req() call.
 *
 *  FORMAL PARAMETERS:
 *	srvc_req - Pointer to service request structure
 *	nstd_raid - Pointer to non-standard RAID structure 
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	EFAULT - User doesn't have r/w access to data area
 *	EINVAL -  Invalid data in packet
 *      CAM_SUCCESS - success
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

int 
cdisk_check_buffer(srvc_req, nstd_raid)
	SRVC_REQ *srvc_req;
	NSTD_RAID *nstd_raid;
{
	/*
	 * Check to make sure the length of the nstd RAID is the
	 * size reflected in the srvc_req.
	 */
	if (srvc_req->srvc_len < sizeof(NSTD_RAID))
	{
		srvc_req->srvc_flags |= (STR_MSG_VALID | SRVC_ERROR) ;
		srvc_req->srvc_errno = EINVAL;
		 return (EINVAL);
	}
	/* 
	 * Make sure that sense area is less then 256 bytes 0 - 255
	 * max that can be decribed in char. SCSI restriction
	 */
	if ( nstd_raid->nstd_sns_len >= ALL_MAX_PAGE_SIZE )
	{
		strcpy(srvc_req->srvc_str, invalid_sense_data_str);
		srvc_req->srvc_flags |= (STR_MSG_VALID | SRVC_ERROR) ;
		srvc_req->srvc_errno = EINVAL;
		return (EINVAL);
	}

	/* Check to make sure the combined data and sense data
	 * areas are not greater than the size of the total data
	 * area.
	 */
	if ((nstd_raid->nstd_data_len + nstd_raid->nstd_sns_len) > 
	    nstd_raid->nstd_total_data_area)
	{
		strcpy( srvc_req->srvc_str, invalid_data_str);
		srvc_req->srvc_flags |= (STR_MSG_VALID | SRVC_ERROR) ;
		srvc_req->srvc_errno = EINVAL;
		return (EINVAL);
	}
	/* Check to make sure the sense offset plus the sense length
	 * is within the total data area.
	 */
	if ((nstd_raid->nstd_sns_offset + nstd_raid->nstd_sns_len) > 
	   nstd_raid->nstd_total_data_area)
	{
		strcpy(srvc_req->srvc_str, invalid_sense_data_str);
		srvc_req->srvc_flags |= (STR_MSG_VALID | SRVC_ERROR) ;
		srvc_req->srvc_errno = EINVAL;
		return (EINVAL);
	}
	/* Verify user access and lock user pages if needed.
	 * Fill in descriptive error string in srvc_req packet.
	 * Set flag indicating valid string present.  
	 */
	if ( !srvc_lock(nstd_raid))
	{
		strcpy(  srvc_req->srvc_str, no_useracc_str);
		srvc_req->srvc_flags |= (STR_MSG_VALID | SRVC_ERROR) ;
		srvc_req->srvc_errno = EINVAL;
		return(EFAULT);
	}
	return(CAM_SUCCESS);

}

 /**********************************************************************
 * ROUTINE NAME: srvc_lock()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the locking down of pages of the 
 *      non-standard RAID IOCTL command for the the data area and
 *	sense data area 
 *
 *  FORMAL PARAMETERS:
 *	nstd_raid - a pointer to the non-standard RAID packet
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  RETURN VALUE:
 *              0       - User does not have read/write access to address
 *                        specified for the data buffer or sense buffer.
 *              1       - Success.
 *
 *  SIDE EFFECTS :
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
  ************************************************************************/
int
srvc_lock(nstd_raid)
	NSTD_RAID *nstd_raid;
{

  int error;
	if (nstd_raid->nstd_total_data_area != 0)
	{
		if ( !CAM_VM_USERACC(nstd_raid->nstd_data_area, 
			nstd_raid->nstd_total_data_area, B_WRITE))
		{
	       		return(0);
		}
		/*
		 * Lock the total data area 
		 */ 
		error = CAM_VM_LOCK(nstd_raid->nstd_data_area, 
			nstd_raid->nstd_total_data_area);
		if( error != 0 ){	
		    return(0);
		}
		    
        }
	return(1);	/* success */
}


 /**********************************************************************
 * ROUTINE NAME: srvc_unlock()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the unlocking of pages of the NSTD_RAID 
 *      IOCTL command for the the data area and sense data area.  It
 *	should be noted that this routine unlocks the pages that were
 *	locked from the cdisk_check_buffer().
 *
 *  FORMAL PARAMETERS:
 *	nstd_raid - a pointer to the non-standard RAID packet
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS :
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
  ************************************************************************/
void 
srvc_unlock(nstd_raid)
	NSTD_RAID *nstd_raid;
{
  int error;
	if (nstd_raid->nstd_total_data_area != 0) {
		 error = CAM_VM_UNLOCK(nstd_raid->nstd_data_area,
			nstd_raid->nstd_total_data_area, B_WRITE);

		if( error != 0 ){
		    panic("Can't unlock already locked memory\n");
		}

	}
}

 /**********************************************************************
 * ROUTINE NAME: cdisk_build_raid_ccb()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      This routine builds a CCB for the srvc_req.  The srvc_req
 *      contains the nstd_raid packet data passed via the rd_srvc_req()
 *      call from the user.
 *
 *  FORMAL PARAMETERS:
 *	srvc_req - a pointer to the service request packet
 *	nstd_raid - a pointer to the non-standard RAID packet
 *	dev - major/minor number of the device
 *	pd - a pointer to the peripheral device structure
 *	cdisk_raid_complete - a pointer to the callback completion function 
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  RETURN VALUE:
 *	ccb - a pointer to the ccb built for the srvc_req
 *
 *  SIDE EFFECTS :
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
  ************************************************************************/
CCB_SCSIIO * 
cdisk_build_raid_ccb(srvc_req, nstd_raid, dev, pd, cdisk_raid_complete)
	SRVC_REQ *srvc_req;
	NSTD_RAID *nstd_raid;
	dev_t dev;
	PDRV_DEVICE *pd;
	void (*cdisk_raid_complete)();
{
	CCB_SCSIIO *ccb;	/* Pointer to created SCSI IO CCB */
	u_char	tag_action;	/* What our default action is */
	struct buf *bp;		/* Pointer to allocated buf structure */
	U32 cam_flags;		/* temp holder of the flags	*/
	static u_char no_mem_str[] = "Can't get driver memory, please try again later.";

	static u_char module[] = "cdisk_build_raid_ccb";

	/*
	 * Get a buf structure
	 */
	if ((bp = ccmn_get_bp()) == (struct buf *)NULL)
	{
		strcpy(srvc_req->srvc_str, no_mem_str  );
		srvc_req->srvc_flags |= (STR_MSG_VALID | SRVC_ERROR);
		srvc_req->srvc_errno = ENOMEM;
		return(NULL);
	}

	/*
	 * Fill in the buf struct
	 */
	bp->b_bcount = nstd_raid->nstd_data_len;
	bp->b_proc = u.u_procp;
	bp->b_un.b_addr = (caddr_t)nstd_raid->nstd_data_area;
	bp->b_flags = B_PHYS | B_BUSY;

	tag_action = pd->pd_tag_action;

	/* 
	 * Set the data direction
	 */
        if ((nstd_raid->nstd_flags & NSTD_DATA_MASK) == NSTD_DATA_IN)
		cam_flags = pd->pd_cam_flags | CAM_DIR_IN;
	else
		if ((nstd_raid->nstd_flags & NSTD_DATA_MASK) == NSTD_DATA_OUT)
			cam_flags = pd->pd_cam_flags | CAM_DIR_OUT;
		else
			cam_flags = pd->pd_cam_flags | CAM_DIR_NONE;
	/*
	 * Obtain a SCSI I/O CCB
	 */
	ccb = ccmn_io_ccb_bld(dev, (u_char *)bp->b_un.b_addr,
				(U32)bp->b_bcount,
				nstd_raid->nstd_sns_len,
				cam_flags, cdisk_raid_complete,
				tag_action, nstd_raid->nstd_cmd_timeo, bp);
	if (ccb == NULL)
	{
		/*
		 * Release the bp
		 */
		ccmn_rel_bp(bp);
		nstd_raid->nstd_error = ENOMEM;
		return(NULL);
	}

	/*
	 * Fill in the cdb 
	 */
	bcopy(nstd_raid->nstd_cmd, ccb->cam_cdb_io.cam_cdb_bytes, nstd_raid->nstd_cmd_len);
	ccb->cam_cdb_len = nstd_raid->nstd_cmd_len;
	ccb->cam_req_map = (u_char *)bp;

	return(ccb);

}

 /**********************************************************************
 * ROUTINE NAME: cdisk_raid_cmd_error()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This routine performs the actions specific for RAID when a CCB
 *      completes with an error.  Actions include things like filling
 *      the sense data into the data area to be returned to the user,
 *      filling in error numbers, and passing more descriptive ascii
 *      string error information back to the user.
 *	
 *
 *  FORMAL PARAMETERS:
 *	srvc_req - a pointer to the service request packet
 *	nstd_raid - a pointer to the non-standard RAID packet
 *	ccb - a pointer to the ccb containing an error
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS :
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
  ************************************************************************/
void 
cdisk_raid_cmd_error(srvc_req, nstd_raid, ccb)
	SRVC_REQ *srvc_req;
	NSTD_RAID *nstd_raid;
	CCB_SCSIIO *ccb;

{
	int cat;	/* CAM status */
	int offset;	/* Sense offset in nstd_raid structure data area */

	static u_char busy_str[] = "Retry limit on busy status exhausted.";

	/*
	 * Get the ccb status
	 */
	cat = ccmn_ccb_status(ccb);
	
	switch (cat)
	{
		case CAT_SCSI_BUSY:
			nstd_raid->nstd_error = EIO;
			strcpy(srvc_req->srvc_str, busy_str);
			srvc_req->srvc_flags = (STR_MSG_VALID | SRVC_ERROR);
			break;
		case CAT_CMP_ERR:
			if ((ccb->cam_scsi_status == 
				SCSI_STAT_CHECK_CONDITION) && 
				(ccb->cam_ch.cam_status & CAM_AUTOSNS_VALID))
			{
				offset = (nstd_raid->nstd_sns_offset);
				nstd_raid->nstd_sense_resid = 
						ccb->cam_sense_resid;
				bcopy((caddr_t)ccb->cam_sense_ptr,
					&nstd_raid->nstd_data_area[offset],
					nstd_raid->nstd_sns_len);
			}
			nstd_raid->nstd_scsi_stat = ccb->cam_scsi_status;
			break;
		default:
			nstd_raid->nstd_error = ENXIO;
			strcpy(srvc_req->srvc_str, cdbg_CamStatus( (ccb->cam_ch.cam_status & CAM_STATUS_MASK),
				 CDBG_FULL));
			srvc_req->srvc_flags = (STR_MSG_VALID | SRVC_ERROR);
			break;
	} /* switch */
}


 /**********************************************************************
 * ROUTINE NAME: cdisk_raid_retry()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine is called by the timeout code.  It is passed
 *	a CCB it will send on done.  Used for retries.
 *
 *  FORMAL PARAMETERS:
 *	ccb - a pointer to a ccb to send 
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS :
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
  ************************************************************************/
void 
cdisk_raid_retry( ccb )
	CCB_SCSIIO *ccb;
{
	PDRV_DEVICE	*pdrv_dev;	/* pointer to peripheral device struct */
	int		spl;		/* Saved IPL */
	static u_char	module[] = "cdisk_raid_retry"; /* Module name */

	/*
	 * Get the peripheral device structure
	 */
	pdrv_dev = (PDRV_DEVICE *)((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_pdrv;

	/*
	 * This shouldn't happen...
	 */
	if (pdrv_dev == NULL)
	{
		panic("cdisk_raid_retry: NULL PDRV_DEVICE pointer");
		return;
	}

	/*
	 * Lock the peripheral device structure
	 */
	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, spl);

	/*
	 * If the ccb can't be re-sent, call it's done routine.
	 * It has been cleared.
	 */
	if (cdisk_send_ccb(pdrv_dev, (CCB_HEADER *)ccb, RETRY) !=
							CAM_REQ_INPROG)
	{
		((CCB_SCSIIO *)ccb)->cam_cbfcnp(ccb);
	}

	/*
	 * Unlock the peripheral device structure
	 */
	PDRV_IPLSMP_UNLOCK(pdrv_dev, spl);

	/*
	 * Release the frozen SIM QUEUE
	 */
	CHK_RELEASE_QUEUE(pdrv_dev, ccb);

	return;
}


 /**********************************************************************
 * ROUTINE NAME: cdisk_raid_wrt_verify()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This routine modifies a write command to a write verify
 *      command.  This routine only gets called if an error
 *      occured on a write command.  This routine first checks to
 *      see if the device supports the write verify command.  If not,
 *      it returns immediately.  If so, the CDB is modified to reflect
 *      the write verify command.
 *
 *  FORMAL PARAMETERS:
 *	ccb - a pointer to the ccb containing an error
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  RETURN VALUE:
 *	NONE
 *
 *  SIDE EFFECTS :
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
  ************************************************************************/
void
cdisk_raid_wrt_verify(ccb)
  CCB_SCSIIO *ccb;
{
    caddr_t cdb;		/* pointer to CDB */
    DEV_DESC *dd;		/* Pointer to the device descriptor */
    DISK_SPECIFIC *ds;		/* Pointer to the disk specific struct */
    u_short code; 
    DIR_WRITE_VRFY_CDB10 tmp_cdb;/* Where we build the write verify cmd */

    /* 
     * Determine if this box supports a write verify parity check
     */
    dd = (DEV_DESC *)((PDRV_DEVICE *)((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_pdrv)
				->pd_dev_desc;
    if(( dd->dd_scsi_optcmds & SZ_WR_VERI_PAR) == 0){
	return;
    }

    /*
     * Get the disk specific struct pointer.
     */
    ds = (DISK_SPECIFIC *)((PDRV_DEVICE *)((PDRV_WS *)ccb->cam_pdrv_ptr)->
	pws_pdrv)->pd_specific;

    if(( ds->ds_raid_level != 3 ) && ( ds->ds_raid_level != 5 )){
	return;
    }

    if(( ccb->cam_cdb_io.cam_cdb_bytes[0] == DIR_WRITE_VRFY_10_OP) &&
	(ccb->cam_ch.cam_status & CAM_AUTOSNS_VALID) && ( SENSEKEY(ccb) ==
	ALL_HARDWARE_ERR ) && ( ((ALL_REQ_SNS_DATA *)ccb->cam_sense_ptr)->
	asc == 86)){
	/* Note the asc code is vendor specific (NCR) */
	/* 
	 * If we have been here before then we can't do the Write
	 * Verify because of degrade condition.
	 * Covert back to a write.
	 */
	bcopy( ccb->cam_cdb_io.cam_cdb_bytes, &tmp_cdb,
			sizeof(DIR_WRITE_VRFY_CDB10)); 
	cdb = (caddr_t)(DIR_WRITE_CDB10 *)ccb->cam_cdb_io.cam_cdb_bytes; 
	bzero( cdb, sizeof( DIR_WRITE_CDB10 ));
	((DIR_WRITE_CDB10 *)cdb)->opcode = DIR_WRITE10_OP;
	((DIR_WRITE_CDB10 *)cdb)->reladr = tmp_cdb.reladr;
	((DIR_WRITE_CDB10 *)cdb)->dpo = tmp_cdb.dpo;
	((DIR_WRITE_CDB10 *)cdb)->lbn0 = tmp_cdb.lbn0;
	((DIR_WRITE_CDB10 *)cdb)->lbn1 = tmp_cdb.lbn1;
	((DIR_WRITE_CDB10 *)cdb)->lbn2 = tmp_cdb.lbn2;
	((DIR_WRITE_CDB10 *)cdb)->lbn3 = tmp_cdb.lbn3;
	((DIR_WRITE_CDB10 *)cdb)->tran_len0 = tmp_cdb.tran_len0;
	((DIR_WRITE_CDB10 *)cdb)->tran_len1 = tmp_cdb.tran_len1;
	
	return;
    }

    if( ccb->cam_cdb_io.cam_cdb_bytes[0] == DIR_WRITE_VRFY_10_OP){
	/*
	 * Write verify failed for some other reason try again
	 */
	return;
    }

    /*
     * Well its a write command see if it is a result of a time out...
     */
    if( CAM_STATUS(ccb) != CAM_CMD_TIMEOUT) {
	return;
    }


    /* 
     * Zero out the tmp_cdb 
     */
    bzero( &tmp_cdb, sizeof(DIR_WRITE_VRFY_CDB10));

    if( ccb->cam_cdb_io.cam_cdb_bytes[0] == DIR_WRITE6_OP) 
    {
	cdb = (caddr_t)(DIR_WRITE_CDB6 *)ccb->cam_cdb_io.cam_cdb_bytes; 
	tmp_cdb.opcode = DIR_WRITE_VRFY_10_OP;
	tmp_cdb.lbn0 =((DIR_WRITE_CDB6 *)cdb)->lbn0;
	tmp_cdb.lbn1 =((DIR_WRITE_CDB6 *)cdb)->lbn1;
	tmp_cdb.lbn2 =(((DIR_WRITE_CDB6 *)cdb)->lbn2 & 0X1F);
	tmp_cdb.tran_len0 = ((DIR_WRITE_CDB6 *)cdb)->trans_len;
	/*
	 * HSZ10 uses a reserved bit for the parity check
	 */
        tmp_cdb.resv2 =  1;
    }
    else /* 10 byte cdb */ 
    {
	cdb = (caddr_t)(DIR_WRITE_CDB10 *)ccb->cam_cdb_io.cam_cdb_bytes; 
	tmp_cdb.opcode = DIR_WRITE_VRFY_10_OP;
	tmp_cdb.reladr = ((DIR_WRITE_CDB10 *)cdb)->reladr;
	tmp_cdb.dpo = ((DIR_WRITE_CDB10 *)cdb)->dpo;
	tmp_cdb.lbn0 =((DIR_WRITE_CDB10 *)cdb)->lbn0;
	tmp_cdb.lbn1 =((DIR_WRITE_CDB10 *)cdb)->lbn1;
	tmp_cdb.lbn2 =((DIR_WRITE_CDB10 *)cdb)->lbn2;
	tmp_cdb.lbn3 =((DIR_WRITE_CDB10 *)cdb)->lbn3;
	tmp_cdb.tran_len0 = ((DIR_WRITE_CDB10 *)cdb)->tran_len0;
	tmp_cdb.tran_len1 = ((DIR_WRITE_CDB10 *)cdb)->tran_len1;
	/*
	 * HSZ10 uses a reserved bit for the parity check
	 */
        tmp_cdb.resv2 =  1;
    }
    bcopy(&tmp_cdb, cdb, sizeof(DIR_WRITE_VRFY_CDB10));
    ccb->cam_cdb_len = sizeof(DIR_WRITE_VRFY_CDB10);
    return;

}


 /**********************************************************************
 * ROUTINE NAME: cdisk_raid_complete()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine is the call back on completion handler for the
 *	Service interface (non-standard RAID).  Its only function
 *	call wakeup() to wake the process sleeping on the completion 
 *	of the request.
 *
 *  FORMAL PARAMETERS:
 *	ccb - a pointer to the completed ccb 
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  RETURN VALUE:
 *	None.
 *
 *  SIDE EFFECTS :
 *      None.
 *
 *  ADDITIONAL INFORMATION:
 *      None.
 *
  ************************************************************************/
void 
cdisk_raid_complete(ccb)
	CCB_SCSIIO *ccb;
{
	PDRV_DEVICE	*pdrv_dev;		/* Device structure */
	PDRV_WS		*pw;			/* Working set */
	int		spl;			/* Saved IPL */
	static u_char	module[] = "cdisk_raid_complete"; /* Module name */

	/*
	 * Get the peripheral device structure
	 */
	pdrv_dev = (PDRV_DEVICE *)((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_pdrv;

	/*
	 * Get the working set
	 */
	pw = (PDRV_WS *)ccb->cam_pdrv_ptr;

	/*
	 * This shouldn't happen...a null pd
	 */
	if ( pdrv_dev == NULL)
	{
		panic("cdisk_raid_complete: NULL PDRV_DEVICE pointer");
		return;
	}

	/*
	 * Lock the peripheral device structure
	 */
	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, spl);

	/*
	 * Indicated that the ccb has been received 
	 */
	pw->pws_flags |= PWS_CALLBACK;

	/*
	 * If recovering, call the recovery handler which will mark the
	 * request as retryable.
	 */
	if( pdrv_dev->pd_flags & PD_REC_INPROG )   {
		PRINTD(pdrv_dev->pd_bus, pdrv_dev->pd_target, pdrv_dev->pd_lun, 
		   (CAMD_DISK | CAMD_ERRORS),
   		   ("[%d/%d/%d] cdisk_complete: Recovery in Progress ccb=0x%x\n",
	   	   pdrv_dev->pd_bus, pdrv_dev->pd_target, pdrv_dev->pd_lun, ccb));
		if(pdrv_dev->pd_recov_hand == (void *)NULL) {
			pw->pws_flags |= PWS_RETRY;
	   		PDRV_IPLSMP_UNLOCK(pdrv_dev, spl);
		}  else  {
			pw->pws_flags |= PWS_RETRY;
	   		PDRV_IPLSMP_UNLOCK(pdrv_dev, spl);
			pdrv_dev->pd_recov_hand(pdrv_dev, ccb);
		}
		CHK_RELEASE_QUEUE(pdrv_dev, ccb);
		return;
	} else {
	   	PDRV_IPLSMP_UNLOCK(pdrv_dev, spl);
	}

	/*
	 * Check to see if bp is filled in.  It should not be for this
	 * done routine.
	 */
	if ((struct buf *)ccb->cam_req_map != NULL)
	{
		/*
		 * Check the ccb status
		 */
		if (CAM_STATUS(ccb) == CAM_REQ_CMP)
		{
			/*
			 * Indicate that we've received it...
			 */
			wakeup(ccb);
			PDRV_IPLSMP_UNLOCK(pdrv_dev, spl);
			return;
		}
		/*
		 * If the status is bus busy or the device is busy  or the
		 * queue is full and the retry count is less than the limit...
		 * retry
		 */
		else if ((CAM_STATUS(ccb) == CAM_REQ_CMP_ERR) &&
			((((CCB_SCSIIO *)ccb)->cam_scsi_status == SCSI_STAT_BUSY)
			|| (((CCB_SCSIIO *)ccb)->cam_scsi_status == SCSI_STAT_QUEUE_FULL))
			&& (pw->pws_retry_cnt < CDISK_RETRY_LIMIT))
		{
			CLEAR_CCB(ccb);
			/* 
			 * Place CCB at head of SIM queue 
			 */
			ccb->cam_ch.cam_flags |= CAM_SIM_QHEAD;
			/*
			 * Increment the retry count
			 */
			pw->pws_retry_cnt++;
			/* 
			 * Unlock the peripheral device structure
			 */
			PDRV_IPLSMP_UNLOCK(pdrv_dev, spl);
			/*
			 * Schedule timeout for 1 second to resend the ccb
			 */
			timeout( cdisk_raid_retry, ccb, hz);
			return;
		}
		else
		{
			/*
			 * Indicate that we've received it...
			 */
			((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_flags |= PWS_CALLBACK;
			wakeup(ccb);
			PDRV_IPLSMP_UNLOCK(pdrv_dec, spl);
			return;
		}
	}
	/*
	 * We've got a null bp
	 */
	else
	{
		/* 
		 * Unlock the peripheral device structure
		 */
		PDRV_IPLSMP_UNLOCK(pdrv_dev, spl);
		((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_flags |= PWS_CALLBACK;
		wakeup(ccb);   
		return;
	}

}


/************************************************************************
 *
 *  ROUTINE NAME:  cdisk_send_ccb()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will handles the error recovery functionality
 *	queue manipulation and calling the common routine layer to 
 *	start the request if appropriate.  For tagged requests a high 
 *	water mark of half the queue depth for the device will be used 
 *	so that other drivers will not be starved.
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
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *	This routine must be called SMP safe!
 *
 ************************************************************************/

U32
cdisk_send_ccb(pd, ccb, retry)
PDRV_DEVICE *pd;
CCB_HEADER *ccb;
u_char retry;	/* indicates whether this is a retry of a CCB that's */
		/* already on the queue */
{
    CCB_HEADER *pend_ccb;
    U32 ret = CAM_REQ_INPROG;

    PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
       (CAMD_DISK | CAMD_INOUT),
       ("[%d/%d/%d] cdisk_send_ccb: entry pd=0x%x ccb=0x%x retry=%d\n",
       pd->pd_bus, pd->pd_target, pd->pd_lun, pd, ccb, retry));

    /*
     * Recovery from Error 
     */
    if(( pd->pd_flags & PD_REC_INPROG ) != 0){

	/*
	 * Check whether this is a SCSI I/O CCB.
	 */
        if ((ccb->cam_func_code == XPT_SCSI_IO) && !(retry)){
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
			   ("[%d/%d/%d] cdisk_send_ccb: CCB 0x%x placed on pending list\n",
	   		   pd->pd_bus, pd->pd_target, pd->pd_lun, ccb));
			ccb->cam_status = CAM_REQ_INPROG;
			insque(ioccb->cam_pdrv_ptr, &pd->pd_pend_list);
			pd->pd_pend_ccb++;
			return(ret);
		   }
		}
		/*
		 * Place the SCSI I/O CCB on the active list and
		 * increment the active count.
		 */
		((PDRV_WS *)ioccb->cam_pdrv_ptr)->pws_flags |= PWS_RETRY;
		PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
		   (CAMD_COMMON | CAMD_FLOW),
		   ("[%d/%d/%d] cdisk_send_ccb: CCB 0x%x placed on active list\n",
	   	   pd->pd_bus, pd->pd_target, pd->pd_lun, ccb));
		insque(ioccb->cam_pdrv_ptr, &pd->pd_active_list);
		pd->pd_active_ccb++;
		return(ret);
	    }
        if ((ccb->cam_func_code == XPT_SCSI_IO) && retry){
	    /* 
	     * Already on que just return
	     */
	    return(ret);
	}

	/* 
	 * Note if not an I/O ccb then fall thru and just send it
	 */
    }
    /*
     * Send the request to the common layer. 
     */
    ret = ccmn_send_ccb(pd, ccb, retry)

    PRINTD(pd->pd_bus, pd->pd_target, pd->pd_lun,
       (CAMD_DISK | CAMD_INOUT),
       ("[%d/%d/%d] cdisk_send_ccb: exit ret=0x%x\n",
       pd->pd_bus, pd->pd_target, pd->pd_lun, ret));

    return(ret);
}
/************************************************************************
 *
 *  ROUTINE NAME: cdisk_send_ccb_wait()
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

U32
cdisk_send_ccb_wait(pd, ccb, retry, sleep_pri)
PDRV_DEVICE *pd;
CCB_HEADER *ccb;
u_char retry;	
int sleep_pri;
{
	u_long status;

	/*
	 * We are done.
	 */
	if( (status = cdisk_send_ccb(pd, ccb, retry)) != CAM_REQ_INPROG) {
		return(status);
	}

	if (sleep_pri> PZERO)  {
		/*
		 * We sleep on address of ccb checking for signals.
		 */
		if (PDRV_SMP_SLEEPUNLOCK(ccb, (sleep_pri | PCATCH), pd))  {
		    PRINTD(ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id, 
			ccb->cam_ch.cam_target_lun, CAMD_DISK,
	    	    	("[%d/%d/%d] cdisk_send_ccb_wait: interrupted sleep for CCB 0x%x\n",
			ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
	 		ccb->cam_ch.cam_target_lun, ccb));
		    return(EINTR);
		}
	} else {
		/*
		 * We sleep on address of ccb but NON interruptable.
		 */
		PDRV_SMP_SLEEPUNLOCK(ccb, sleep_pri, pd);
	}

	PRINTD(ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id, 
		ccb->cam_ch.cam_target_lun, CAMD_COMMON,
		("[%d/%d/%d] cdisk_send_ccb_wait: exit - success\n",
		ccb->cam_ch.cam_path_id, ccb->cam_ch.cam_target_id,
	 	ccb->cam_ch.cam_target_lun));

	return (0);
}


