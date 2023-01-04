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
static char *rcsid = "@(#)$RCSfile: cam_tape.c,v $ $Revision: 1.1.22.5 $ (DEC) $Date: 1993/10/14 20:20:23 $";
#endif

#define CAMERRLOG 

/* ---------------------------------------------------------------------- */

/* cam_tape.c	Version 1.00	    Mar. 11, 1991 

    This module is the upper layer (class) for SCSI tapes. The
    module is written to the CAM spec (rev 2.3) and the SCSI 2
    spec for sequential access devices.

Modification History

    Version    Date	Who    Reason

    1.00    03/11/91    dallas    Creation date.  

    1.02    06/04/91	dallas	  Changed scsi include file names to 
				  reflect new ones.

    1.03    06/05/91	dallas    Fixed eom handling with respect to 
				  BSR FSR WFM's etc......

    1.04    06/17/91	dallas	  Included rmillers slave and attach
				  fixes (the miller spec). Updated 
				  density selection and EOM handling
				  for older devices.
				  Added Orphan cmds... and Position
				  lost state.

    1.05    06/19/91	maria 	  Removed extern for ccmn_cdb_build.

    1.06    06/21/91    dallas    Fixed in ioctl devget a pointer to
				  wrong member of dev_desc.

    1.7     07/31/91	dallas    Fixed printd's to conform to the 
				  miller spec. Added full error
				  logging. Got rid of routines
				  nolonger needed because error
				  logging has been implemented.
				  Added macro's as suggested Fixed
				  multi volume tar problem.

    1.8     08/02/91	dallas	  Fixed panic's in ctape_ioctl 
				  and ctape_async due to improper
				  use of macro's for wrappers.

    1.9     08/30/91	dallas	  Added scsi busy retries in the
				  done routines. Added "DAS" loaded
				  support, this was a major addition.

    2.0     09/13/91	dallas    Added Rescans for devices that were
				  not up at boot. Added sync on opens.
				  Added loader bit for DEVGET ioctl.
				  Name change of cam_tape_define.h to
				  cam_tape.h. Lint changes

    2.1	    09/16/91	dallas    Fix for retry on busy scsi status.
				  Using ccmn_ccbwait. 

    2.2	    09/19/91	dallas	  In ctape_online changed the MTBSR 
				  back to MTREW because of tk50's
				  don't do the right thing...

    2.3     10/24/91	dallas	  Fixed bug in interuptable move_tape
				  commands. Moved the call to 
				  ctape_compress_set to ctape_online.
				  Fixed bug in ctape_compress_set.
    2.4     11/19/91	dallas	  Added new densities for scsi tapes.
				  TZ30 fix (ctape_mode_sns). Turned
				  on read/write counters. Added fix
				  for ABORTED CCB's not done by driver.
				  Copy slast sense to pdrv_device
				  buffer.
    2.5     12/02/91	dallas	  Added fix in ctape_online for fixed
				  block exabyte tapes. Changed name
				  reported in devget for the wrapper
				  Fixed controller number in devget.
    2.5     12/03/91	dallas	  Tightened up the TZ30 problem of
				  no sense bits but also no data.
				  Fixed up the calls to ctape_online
				  from ctape_ioctl to pass cam_dev instead
				  of dev.
    2.6     12/06/91	dallas	  Fixed call to DoSpecialCmd in ctape_ioctl
				  The number of args to the routine has
				  changed.
    2.7     01/27/91	dallas	  In ctape_attach issue BDR's for tz30 and
				  tlz04's because of problems with those
				  devices. Increased error control 
				  granularity, this involved changes to
				  ctape_ccb_chkcond for greater error
				  return granularity and ctape_move_tape
				  additional arg. To allow caller control
				  of what check condition types to report
				  to the error log.
*/		    

/* ---------------------------------------------------------------------- */

/* Include files. */

#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <io/common/devdriver.h>
#include <sys/mtio.h>
#include <io/common/devio.h>
#include <sys/errno.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam_debug.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_sequential.h>
#include <io/cam/pdrv.h>
#include <io/cam/ccfg.h>
#include <io/cam/cam_errlog.h>
#include <io/cam/cam_tape.h>
#include <io/cam/cam_logger.h>		/* The cam error log file	*/

/*
 * End error defines and includes....
 */


/* ---------------------------------------------------------------------- */

/* Local defines. */

void ctape_done();
void ctape_orphan_done();
void ctape_async();
void ctape_iodone();
void ccmn_minphys();
void ctape_strategy();
void ctape_ready();
void ctape_open_sel();
void ctape_density_set();
void ctape_compress_set();
void ctape_move_tape();
void ctape_wfm();
void ctape_mode_sns();
void ctape_load_tape();
void ctape_minphys();
void ctape_retry();
U32 ctape_ccb_chkcond();


/* ---------------------------------------------------------------------- */

/* External declarations. */
extern int lbolt;
extern int hz;
extern int timeout();
extern void ccmn_minphys();
extern void ccmn_init();
extern I32 ccmn_open_unit();
extern void ccmn_close_unit();
extern U32 ccmn_send_ccb();
extern U32 ccmn_send_ccb_wait();
extern void ccmn_rem_ccb();
extern void ccmn_abort_que();
extern void ccmn_term_que();
extern CCB_HEADER *ccmn_getccb(); 
extern void ccmn_rel_ccb();
extern CCB_SCSIIO *ccmn_io_ccb_bld();
extern CCB_GETDEV *ccmn_gdev_ccb_bld();
extern CCB_SETDEV *ccmn_sdev_ccb_bld();
extern CCB_SETASYNC *ccmn_sasy_ccb_bld();
extern CCB_RELSIM *ccmn_rsq_ccb_bld();
extern CCB_PATHINQ *ccmn_pinq_ccb_bld();
extern CCB_ABORT *ccmn_abort_ccb_bld();
extern CCB_TERMIO *ccmn_term_ccb_bld();
extern CCB_RESETDEV *ccmn_bdr_ccb_bld();
extern CCB_RESETBUS *ccmn_br_ccb_bld();
extern CCB_SCSIIO *ccmn_tur();
extern CCB_SCSIIO *ccmn_mode_select();
extern U32 ccmn_ccb_status();
extern U32 ccfg_edtscan();
extern struct buf *ccmn_get_bp();
extern void ccmn_rel_bp();
extern u_char *ccmn_get_dbuf();
extern void ccmn_rel_dbuf();
extern int ccmn_DoSpecialCmd();
extern void printf();
extern void uprintf();
extern int nCAMBUS;
extern u_long ctape_io_base_timo;
extern u_long ctape_move_timo;
extern u_long ctape_wfm_base_timo;
extern u_long ctape_boot_bdr;
extern int density_entrys;
extern DENS_TBL_ENTRY density_table[];

/* 
 * Error reporting and conversion routines..
 */
extern void ccmn_errlog(); 

/*
 * End of error routines
 */
extern struct device *camdinfo[];
extern struct controller *camminfo[];

extern PDRV_UNIT_ELEM pdrv_unit_table[];


/* ---------------------------------------------------------------------- */

/* Initialized and uninitialized data. */


static void (*local_errlog)() = ccmn_errlog; 




/* ---------------------------------------------------------------------- */
/* Function description.
 *
 * Routine name ctape_slave
 *
 *	This routine gets called at boot to find out if there are any
 *	devices at this BUS/TARGET/LUN. 
 *
 * Call syntax
 *  ctape_slave(attach )
 * 	struct device *attach		Pointer to the device struct in OSF or
 *      struct uba_device *attach	Pointer to uba_device struct in BSD
 *	caddr_t		   reg		Virtual address of controller
 *					DO NOT USE
 * 
 * Implicit inputs
 *	NONE
 *
 * Implicit outputs 
 *	NONE
 *	
 * Return values
 *	PROBE_FAILURE
 *	PROBE_SUCCESS
 *
 */

int
ctape_slave( attach, reg )
	struct device *attach;		/* Pointer to uba device struct	*/
 	caddr_t		   reg;		/* Virtual address of controller
 					 * DO NOT USE
					 */

{

    /*
     * Local variables
     */
    U32	unit;		/* Unit number				*/
    dev_t	dev;
    static u_char	module[] = "ctape_slave"; /* Module name	*/

    /*
     * The UBA_UNIT_TO_DEV_UNIT macro assumes ui_unit has bits 0-2 = lun, 
     * bits 3-5 = target id, and 6-7 = bus num.
     */
    dev = makedev(0, MAKEMINOR(UBA_UNIT_TO_DEV_UNIT(attach), 0));
    unit = DEV_UNIT(dev);

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

    /*
     * Call init because we don't know if the subsystem has
     * been init'ed
     */
    ccmn_init();


    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), ("[%d/%d/%d] %s: cntl = %d unit = %d\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
	module, attach->ctlr_num, (attach->unit & 0x3f)));

    if( unit > MAX_UNITS){
	/*
	 * They screwed up this unit is greater then max.
	 */
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE |CAMD_INOUT), 
		("[%d/%d/%d] %s: Unit number to large %d\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
		module, unit));
	return(PROBE_FAILURE);
    }
    /*
     * Set the configured bit in our unit table
     */
    pdrv_unit_table[unit].pu_config |= ( 1 << ALL_DTYPE_SEQUENTIAL);

    /*
     * Now call the open unit routine to see if a device is there.
     * We must shift the unit number left be 4 to get over the
     * device specific bits like density and norewind
     */
    if( ccmn_open_unit( dev, ALL_DTYPE_SEQUENTIAL, 
		CCMN_EXCLUSIVE, sizeof(TAPE_SPECIFIC)) != (I32)NULL){
	/*
	 * Could not open unit.
	 */
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE |CAMD_INOUT), 
		("[%d/%d/%d] %s: ccmn_open_unit failed\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));
	return(PROBE_FAILURE);
    }

    /* Set the logical unit number for indexing camdinfo struct */
    pdrv_unit_table[unit].pu_device->pd_log_unit = attach->logunit;

    /*
     * Close out the unit.
     */
    ccmn_close_unit(dev);

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n",
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));


    return(PROBE_SUCCESS);
}



/* ---------------------------------------------------------------------- */
/* Function description.
 *
 * Routine name ctape_attach
 *
 *	This routine gets called at boot to find out if there are any
 *	devices at this BUS/TARGET/LUN. Output the identification string.
 *
 * Call syntax
 *  ctape_attach(attach)
 * 	struct device *attach		Pointer to the device struct in OSF or
 *      struct uba_device *attach	Pointer to uba_device struct in BSD
 * 
 * Implicit inputs
 *	NONE
 *
 * Implicit outputs 
 *	NONE
 *	
 * Return values
 *	PROBE_FAILURE
 *	PROBE_SUCCESS
 *
 */

int
ctape_attach( attach )
    struct device *attach;		/* Pointer to uba device struct	*/
{

    /* Local Varibles */

    PDRV_DEVICE 	*pd;	/* Pointer to periheral device structure */
    dev_t		dev;	/* For the PRINTD's			*/
    ALL_INQ_DATA 	*inqp; /* Pointer to inquiry data		*/
    DEV_DESC            *dev_desc;      /* Ptr to our device descriptor */
    CCB_RESETDEV 	*bdr_ccb;	/* For TZ30's HACK		*/
    char 		idstring[IDSTRING_SIZE+REV_STRING_SIZE+2];
    static u_char	module[] = "ctape_attach"; /* Module name	*/

    /*
     * The UBA_UNIT_TO_DEV_UNIT macro assumes ui_unit has bits 0-2 = lun, 
     * bits 3-5 = target id, and 6-7 = bus num.
     */
    dev = makedev(0, MAKEMINOR(UBA_UNIT_TO_DEV_UNIT(attach), 0));

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	
    /*
     * Determine whether a device exists at this address by calling
     * ccmn_open_unit which will check the EDT table.
     */
    if( ccmn_open_unit(dev, (U32)ALL_DTYPE_SEQUENTIAL,
		CCMN_EXCLUSIVE, (U32)sizeof(TAPE_SPECIFIC)) != 0L)   {
        PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: ccmn_open_unit failed\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	return(PROBE_FAILURE);
    }

    if( (pd = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL)  {
	ccmn_close_unit(dev);
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: No peripheral device structure allocated\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	return(PROBE_FAILURE);
    }

    /*
     * Start of TZ30 hack.... When the config driver goes and probes
     * the bus mwhen the tz30 is probed and we hit a lun other
     * then 0 it drives the unit insane if a tape is loaded....
     * The fix is to clear the condition buy issueing a bdr to the
     * unit when we see it at boot.........
     * The tlz04 also needs a bdr.... The is because it eats the
     * unit attention on boot because of inquiry to luns using
     * sync negotiations causes the device to break...
     */
    dev_desc = pd->pd_dev_desc;

    /*
     * For machines that don't reset the bus on boot (ALPHA's)
     * We must try to determine position. Policy is presently to
     * issue a BDR for all tape devices until we decide how to 
     * implement multi-initiators for tape. Multi-initiators
     * for tapes must be thought out. What we decide will be around
     * for a long time.
     */

    if( (strcmp( DEV_TZ30, dev_desc->dd_dev_name ) == NULL) ||
	(strcmp( DEV_TLZ04, dev_desc->dd_dev_name ) == NULL) ||
	(ctape_boot_bdr != NULL)){

	bdr_ccb = ccmn_bdr_ccb_bld( dev, (U32)CAM_DIR_NONE);
	/* 
	 * don't even wory about status.........
	 */
	CHK_RELEASE_QUEUE(pd, bdr_ccb);

	ccmn_rel_ccb(bdr_ccb);
    }

    /* 
     * Output the identification string
     */
    inqp = (ALL_INQ_DATA *)pd->pd_dev_inq;
    bcopy((caddr_t)inqp->vid, idstring, VID_STRING_SIZE);
    bcopy((caddr_t)inqp->pid, &idstring[VID_STRING_SIZE], PID_STRING_SIZE);
    idstring[VID_STRING_SIZE + PID_STRING_SIZE] = ' ';
    bcopy((caddr_t)inqp->revlevel, 
     	&idstring[VID_STRING_SIZE + PID_STRING_SIZE+1], REV_STRING_SIZE);
    idstring[VID_STRING_SIZE + PID_STRING_SIZE+ REV_STRING_SIZE +1] = '\0';

    printf(" (%s)", idstring);


    /*
     * Decrement the open count back to 0.
     */
    ccmn_close_unit(dev);

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n",
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));

    return(PROBE_SUCCESS);
}


/* ---------------------------------------------------------------------- */
/* Function description.
 *
 * Routine name ctape_open
 *
 *	This routine is a pass thru routine that calls ctape_online.
 *	All work for opening and bringing the unit online
 *	is done in online unit.
 *
 * Implicit outputs 
 *	None
 * Implicit outputs 
 *	None
 *
 * Return values
 *	CTAPE_SUCCESS
 *	EBUSY	Device reserved by another initiator
 *	ENOMEM  Resource problem
 *	EINVAL  CCB problems
 *	ENXIO	Device path problems.
 *	EIO	Device check conditions
 *
 */


int 
ctape_open(dev, flags, fmt)


    dev_t dev;		/*  major and minor pair   		 	*/
    int flags;		/* Flags RDONLY READ WRITE FNDELAY etc.	*/

{
    static u_char	module[] = "ctape_open"; /* Module name	*/

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

    return( ctape_online( dev, flags, OPEN_UNIT));
    
}

/* ---------------------------------------------------------------------- */
/* Function description.
 *
 * Routine name ctape_online
 *
 *	This routine handles opening the unit. 
 *	For flags of FNDELAY and errors other the reservation conflicts
 *	we always return success.
 *	
 *	First do a TEST unit ready command to see if the device is 
 *	ready to use.
 *	
 *	If we don't have any unit attentions or resets fall
 *	down to bottom and return success.
 *	
 *	Else set the density up (mode select )
 *	Then move tape to see if the cartridge has been written to
 *	determine density.
 *	
 *
 * Call syntax
 *  ctape_online( dev, flags, online_flags )
 * 
 * Implicit inputs
 * 	Flags of CTAPE_RESEST_STATE, CTAPE_UNIT_ATTEN_STATE
 *
 * Implicit outputs 
 *	Flags of UNIT_ATTEN_STATE and AUTO_DENSITY_VALID_STATE
 *
 * Return values
 *	CTAPE_SUCCESS
 *	EBUSY	Device reserved by another initiator
 *	ENOMEM  Resource problem
 *	EINVAL  CCB problems
 *	ENXIO	Device path problems.
 *	EIO	Device check conditions
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */


int 
ctape_online(dev, flags, online_flags)


    dev_t dev;		/*  major and minor pair   		 	*/
    int flags;		/* Flags RDONLY READ WRITE FNDELAY etc.	*/
    I32 online_flags;	/* Tells us how to bring unit online, if we
			 * are going thru full open or thru loader
			 * type onlines..
			 */

{

    /*
     * LOCAL VARIABLES 
     */

	
    PDRV_DEVICE		*pdrv_dev;	/* Ptr to our device struct	*/

    DEV_DESC		*dev_desc;	/* Ptr to our device descriptor	*/

    MODESEL_TBL		*modsel_tab;	/* Ptr to our mode select 
					 * table for what we do on opens.
					 */


    TAPE_SPECIFIC	*ts_spec;	/* Ptr to our device specific
					 * struct
					 */
    
    CCB_SETASYNC	*ccb_async;	/* Cam set async ccb		*/
    CTAPE_ACTION	action;		/* What all the subroutines use
					 * to communicate back to caller 
					 */
    
    DENSITY_TBL		*dens;		/*
					 * Pointer to density table for 
					 * this device
					 */
    
    I32		idx;		/* Index into the density desc	*/


    I32		ret_val;	/* return value from sub-routines */

    U32		ready_time;	/* 
					 * Time it takes for this type
					 * unit to come ready (seconds)
					 */
    U32		state_flags;	/* Saved state 			*/
    I32		success;	/* Test unit ready loop indicator */
    I32		fndelay;	/* Test unit ready loop indicator */
    I32		fatal;		/* Test unit ready loop indicator */

    int			i;		/* Various uses			*/
    int			s;		/* For our saved ipl		*/
    int			s1;		/* Throw away IPL		*/
    static u_char	module[] = "ctape_online"; /* Module name	*/

			

    /*
     *END OF LOCAL VARIABLES 
     */

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

    /* 
     * If online flags say open unit.
     * Call generic peripherial driver support routine to
     * open the device for us. This routine will lock
     * the unit table an make sure everything matches
     * Arguments are dev, device_type (tape ,disk,scanner),
     * size of your device specific struct (TAPE_SPECIFIC,
     * and whether exclusive use or not.
     *
     * Plese refer to ccmn_open_unit() for a full description.
     *
     * If the return == ENXIO then issue a rescan to see if the
     * device has been powered up......
     */
    if(( online_flags & OPEN_UNIT ) != NULL) {
        for( i = 0; i < 2; i++){

            ret_val = ccmn_open_unit( dev, ALL_DTYPE_SEQUENTIAL, 
		    CCMN_EXCLUSIVE, sizeof(TAPE_SPECIFIC) );

            if ( ret_val == ENXIO ){
	        /*
	         * Based on return value return a ERRNO 
	         * EBUSY - Device is already opened exclusive use 
	         * EINVAL- Device types do not match 
	         * ENXIO - Device does not exist even after rescan.  
	         */

    	        PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Issuing rescan for device\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	
		if(ccfg_edtscan(EDT_SINGLESCAN, DEV_BUS_ID(dev),
	             DEV_TARGET(dev), DEV_LUN(dev)) != CAM_REQ_CMP) {
	             PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
			(CAMD_TAPE | CAMD_ERRORS),
	   	        ("[%d/%d/%d] %s: ccfg_edtscan failed\n",
	                DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			module));
		     return(ENXIO);
		}
            }
	    else {
		break; /* Break out of for loop....*/
	    }
        }
        if ( ret_val != NULL ){
	    /*
	     * Based on return value return a ERRNO 
	     * EBUSY - Device is already opened exclusive use 
	     * EINVAL- Device types do not match 
	     * ENXIO - Device does not exist even after rescan.  
	     */

    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Dev failed ccmn_open_unit dev = %d\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module,
		    dev));

	    return( ret_val );
	
        }
    }
    /* 
     * WE now have everything we need to start operations 
     * Can now set up structure pointers
     */

    /* 
     * Get our peripherial device pointer 
     */
    if( (pdrv_dev = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL){
	/*
	 * This should not happen no pdrv_device struct... 
	 */
	CAM_ERROR( module, "No device struct", CAM_SOFTWARE, 
			(CCB_HEADER *)NULL, dev, (u_char *)NULL);
	return(ENOMEM);
    }


    /* 
     * Get our pointer to our device description table
     */
    dev_desc = pdrv_dev->pd_dev_desc;

    /* 
     * Get our pointer to our mode select table 
     */
    modsel_tab = dev_desc->dd_modesel_tbl;
    

    /* 
     * Get our device specific pointer 
     */

    if( (ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific) == 
		(TAPE_SPECIFIC *)NULL){
        PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
	CAM_ERROR( module, "No tape specific struct", CAM_SOFTWARE,
			(CCB_HEADER *)NULL, dev, (u_char *)NULL);
        PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	return(ENOMEM);
    }

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE ), 
	("[%d/%d/%d] %s: state flags = %X\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module,
	ts_spec->ts_state_flags));

    /* 
     * Check to see if a command is still inprogress from last open/close
     * if so sleep and wait for it to complete......
     */
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
    while(( ts_spec->ts_state_flags & CTAPE_ORPHAN_CMD_STATE ) != NULL ){
	/*
    	 * Sleep on address of ts_spec->ts_state_flags but interruptable
    	 */
    	if( PDRV_SMP_SLEEPUNLOCK( &ts_spec->ts_state_flags, 
			((PZERO + 1) | PCATCH )  , pdrv_dev) != NULL){
	    /* 
	     * USER has interrupted our sleep they must not want to wait
	     */
	    PDRV_LOWER_IPL( s );
	    ccmn_close_unit(dev);
	    return (EINTR);
   	} 
	/* 
    	 * Get the lock again
    	 */
    	PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s1 );
    }


    /* 
     * Do not unlock here since we are going to be mucking with flags
     */

    /*
     * INIT our state flags and regular flags
     * We want to notice a few of them accross opens.
     */

    state_flags = ts_spec->ts_state_flags;

    if((state_flags & 
		(CTAPE_UNIT_ATTEN_STATE | CTAPE_RESET_STATE )) != NULL) {

	/*
	 * Known position is beginning of media if unit atten or reset
	 */
	ts_spec->ts_flags = CTAPE_BOM;
    }
    else {
	/*
	 * We want position indicators where they where left.
	 * Do not need or want anything else.
	 */
	ts_spec->ts_flags &= (CTAPE_BOM | CTAPE_EOM);

    }
    if((ts_spec->ts_state_flags & 
		(CTAPE_UNIT_ATTEN_STATE | CTAPE_RESET_STATE )) != NULL) {

	/*
	 * Reset or unit attention clear all states
	 */
	ts_spec->ts_state_flags = 0; 
    }

    else {
	/*
	 * leave auto density valid alone.. tape has not change
	 * since last open so we leave it alone.
	 */
	ts_spec->ts_state_flags &= (CTAPE_AUTO_DENSITY_VALID_STATE |
		CTAPE_POSITION_LOST_STATE ); 
    }

    PDRV_IPLSMP_UNLOCK( pdrv_dev, s);




    /*
     * If online flags say open unit register for the async call backs.  
     * We can now register for an async call back 
     * The events we want to know about are:  
     * 
     * Bus Device resets, SCSI_Attens, Bus Resets 
     */ 
     
    if(( online_flags & OPEN_UNIT ) != NULL) {
        ccb_async = ccmn_sasy_ccb_bld( dev, 
			(U32)CAM_DIR_NONE, (AC_SENT_BDR |
			AC_SCSI_AEN | AC_BUS_RESET), ctape_async,
			(u_char *)NULL, NULL);

        /*
         * This is an immediate command so status should be valid
         * If not then things are messed up down below
         */
        if( CAM_STATUS(ccb_async) != CAM_REQ_CMP ){
	    /* 
	     * We can't register our async callback
	     * If FNDELAY set continue
	     */
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Can't set async ccb status = %x\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module,
		    ccb_async->cam_ch.cam_status));

	    CAM_ERROR(module, "Can't set async callback",CAM_SOFTWARE,
			(CCB_HEADER *)ccb_async, dev, (u_char *)NULL);


	    /*
	     * Release the ccb
	     */
	    ccmn_rel_ccb((CCB_HEADER *)ccb_async );

	    if((flags & (FNDELAY|FNONBLOCK)) == NULL){
	        CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
	        ccmn_close_unit(dev);
	        return(EIO);
	    }
        } /* end of if status != CAM_REQ_CMP for set async */

        /*
         * Just release the ccb
         */
        else {

	    /*
	     * Release the ccb
	     */
	    ccmn_rel_ccb((CCB_HEADER *)ccb_async );

        } /* end of else (set async ccb status == CAM_REQ_CMP) */
    } /* end of if OPEN_UNIT */

    

    /*
     * WE now have everything we need to start operations 
     * First we must see if the unit is ready to accept commands
     * DO a test unit ready. We rely on the auto sense feature
     * to do the request sense for us.
     * We check to see if the online flag bit oof NO_TIME is
     * set. If the bit is see we set ready time to 3 seconds
     * We are being called from the strategy routine..
     * Unit was opened with the fndelay flag set originally
     * and was not ready.. tar and dump both do this.
     *
     * Some tape drives require  quite a bit of time to come
     * ready after a tape change or they can be rewinding
     * the tape which can take an extremely long time.
     * Gaze into the dev descriptor an get the device ready
     * time in seconds. If null take the default of 45 seconds
     */

    if((online_flags & NO_TIME) != NULL){
	ready_time = 3;
    }
    else {
        ready_time = dev_desc->dd_ready_time;

        if( ready_time == NULL){
	    ready_time = 45;
        }
    }

    /* 
     * The following 3 variable are VERY important. They direct
     * actions at the bottom of the for loop. If success is non
     * zero then the TUR succeeded with no errors. If fndelay
     * is non zero the TUR failed but the FNDELAY flag was set
     * either way get out of for loop;
     * If fatal is ever positive then either the unit is reserved
     * to another initiator or driver problem...
     */

    success = 0;
    fndelay = 0;
    fatal = 0;


    /* 
     * Do the for loop looking for the device to come ready
     * We take into account the FNDELAY flag and busy SCSI
     * status.
     */
    for ( i = 0; i < ready_time; i++) {
	/* 
	 * Zero out action structure
	 */
	bzero( &action, sizeof(CTAPE_ACTION));

	/*
	 * Do a test unit ready. We rely on the auto sense 
	 * feature to do the request sense for us.
	 */

	ctape_ready( pdrv_dev, &action, ctape_done, CTAPE_SLEEP);

	if( action.act_ccb == NULL ) {
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: TUR, CCB_IO = NULL\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	    /*
	     * Resource problem???? If so get out
	     */
	    if(( action.act_fatal & ACT_RESOURCE ) != NULL){
		fatal++;
		break;
	    }
	    /* 
	     * Some type of other gross error
	     */
	    else if(( flags & (FNDELAY|FNONBLOCK) ) != NULL){
		fndelay++;
		break;
	
	    }
	    else {
		fatal++;
		break;
	    }

	}

	/*
	 * At this point we have a completed ccb
	 * Check to see if it completed successfully
	 */ 
	if(action.act_ccb_status == CAT_CMP) {

    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: TUR, SUCCESS\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	    success++;

	} /* end if status == CAT_CMP */

	/*
	 * The test unit ready failed find out why
	 * The only error that will cause an open with
	 * the FNDELAY flag set is EBUSY (reservation 
	 * conflict) to return. 
	 */
	else {
	    /* 
	     * If the ccb status does not equal 
	     * CAT_CMP_ERR , Then fail this open.
	     */
	    if( action.act_ccb_status != CAT_CMP_ERR ){
		fatal++;
	    }

	    /*
	     *  Check for Reservation conflict
	     */
	    else if( action.act_scsi_status == SCSI_STAT_RESERVATION_CONFLICT ){
		fatal++;
	    }

		

	    /*
	     * Make sure we are not busy from prev. command &&
	     * Try to walk out unit atten's for fndelay
	     */
	    if( (action.act_scsi_status != SCSI_STAT_BUSY ) &&
			(( flags & (FNDELAY|FNONBLOCK) ) != NULL) && ( i > 1 )) {
		fndelay++;
	    }
	}

	/*
	 * At this point in time we are done with this ccb
	 * we release it back to the pools
	 */
	CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	/*
	 * Release the ccb
	 */
	ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	/* 
	 * Check for posix nodelay or sucessful open or fatal error
	 */
	if((fndelay != NULL) || (success != NULL) ||(fatal != NULL)){
	    /* 
	     * Break out of for loop
	     */
	    break;
	}


	/* 
	 * since we don't have success yet sleep 
	 */

	if( mpsleep(&lbolt, (PCATCH|(PZERO+1)), "Zzzzzz",0,(void *)0,0)) {
	    /* 
	     * Set for interrupable sleeps ... if
	     * non zero comes back we have had a signal
	     * delivered.
	     */
	    CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
	    ccmn_close_unit(dev);
	    return( EINTR );
	}


    } /* end of for tur loop */

    /*
     * Check to see it fatal is set (reservation conflict)
     */
    if ( fatal != NULL ){
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: TUR, FATAL\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
	ccmn_close_unit(dev);
	return(action.act_ret_error);
    }	
    else if( fndelay != NULL ){
	/* 
	 * We got out of loop because of some failure
	 * and the FNDELAY flag is set.
	 * Set the flag saying not set up.
	 */
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: TUR, FNDELAY\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
		(state_flags | CTAPE_NOT_READY_STATE));

	return(CTAPE_SUCCESS);
    }
    else if ( success == NULL){
	/*
	 * The TUR never completed successfully and FNDELAY
	 * flag WAS NOT set return the last error value
	 */
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: TUR, NO_SUCCESS\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags );
	ccmn_close_unit(dev);
	return(EIO);
    }



    /*
     * If there has been a reset or unit atten go do
     * the user defined mode select tables
     */

    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    /* 
     * The if checks to see if we had a reset when the unit was closed
     * and if one has occurred while in the opening state.
     */
    if(((state_flags & (CTAPE_UNIT_ATTEN_STATE | CTAPE_RESET_STATE)) !=
		NULL ) || ((ts_spec->ts_state_flags & (CTAPE_RESET_STATE |
		CTAPE_UNIT_ATTEN_STATE)) != NULL)) {

	/*
	 * We have had a unit attention or reset must set the
	 * mode select pages 
	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

	/* 
	 * Now lets do the mode select table for this
	 * device. We we run down the list passing the index
	 * of the page we want to set.
	*/


	if(modsel_tab != NULL) {
	    /*
	     * There is a table lets do what the user/sys admin
	     * wants us to do for this device.
	     */
	    for( i = 0; (modsel_tab->ms_entry[i].ms_data != NULL) && 
				( i < MAX_OPEN_SELS); i++) {
		/*
		 * Zero out the action structure
		 */
		bzero( &action, sizeof(CTAPE_ACTION));
		ctape_open_sel( pdrv_dev, &action, i, 
			ctape_done, CTAPE_SLEEP);

		if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    		    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			(CAMD_TAPE ), 
			("[%d/%d/%d] %s: MODSEL, CCB = NULL\n", 
			DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			module));

		    if(( action.act_fatal & ACT_RESOURCE ) != NULL ){
		    	/* 
		    	 * Could not get resources needed (ccb's);
		    	 */

			CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
		    	ccmn_close_unit(dev);
		    	return(action.act_ret_error); /* driver/resource 
						       * problem 
						       */
		    }

		    if( (flags & (FNDELAY|FNONBLOCK)) == NULL ) {
			/*
			 * close the unit and return errno
			 */
			CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
			ccmn_close_unit(dev);
			return( action.act_ret_error );
		    }

		    /*
		     * The user has set fndelay must return success
		     */
		    else {
			CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				( state_flags | CTAPE_NOT_READY_STATE));

			return(CTAPE_SUCCESS);

		    }

		}

		/*
		 * At this point we have a completed ccb
		 * Check to see if it completed successfully
		 */ 
		if(action.act_ccb_status == CAT_CMP){

		    /*
		     * At this point in time we are done with this ccb
		     * we release it back to the pools
		     */
		    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

		    /*
		     * Release the ccb
		     */
		    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

		    /* do next page if any */
		    continue;
		}

		/*
		 * The mode select for this page failed find out why
		 * The only error that will cause an open with
		 * the FNDELAY flag set is EBUSY (reservation 
		 * conflict) to return. 
		 */

		else {
    		    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			(CAMD_TAPE ), 
			("[%d/%d/%d] %s: MODSEL FAILED index = 0x%x\n", 
			DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			module, i));
		    
		    /*
		     * At this point in time we are done with this ccb
		     * we release it back to the pools
		     */
		    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

		    /*
		     * Release the ccb
		     */
		    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

		    
		    /* 
		     * If reservation conflict or fndelay == NULL then error 
		     * This open out
		     */
		    if((action.act_scsi_status == 
				SCSI_STAT_RESERVATION_CONFLICT) || 
				((flags & (FNDELAY|FNONBLOCK)) == NULL) ){
			/*
			 * close the unit and return EBUSY
			 */
			CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
			ccmn_close_unit(dev);
			return( action.act_ret_error );
		    }

		    /*
		     * The user has set fndelay must return success
		     */
		    else {
			CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				(state_flags | CTAPE_NOT_READY_STATE));

			return(CTAPE_SUCCESS);

		    }
		}
	    } /* end of for loop */
	} /* End of if modsel != NULL */
    } /* End of reset or unit atten */

    /*
     * Just unlock the struct
     */
    else {
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
    }

    /* 
     * Now we must look to see if a reset or a unit atten has come by.
     * If one of them has, then do auto density.
     */
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s  );

    /* 
     * The if checks to see if we had a reset when the unit was closed
     * and if one has occurred while in the opening state.
     */
    if(((state_flags & (CTAPE_UNIT_ATTEN_STATE | CTAPE_RESET_STATE)) !=
		NULL ) || ((ts_spec->ts_state_flags & (CTAPE_RESET_STATE |
		CTAPE_UNIT_ATTEN_STATE)) != NULL) || ((ts_spec->ts_flags &
		CTAPE_BOM) != NULL)) {

	/*
	 * We have had a unit attention or reset do auto density
	 */

	/* 
	 * Make sure eom, tape mark and tape mark pending are cleared
	 * We have to notice them after we move tape.
	 */

	ts_spec->ts_flags &= ~(CTAPE_EOM | CTAPE_TPMARK | 
				CTAPE_TPMARK_PENDING );

	/*
	 * Move tape 1 record forward this will allow the
	 * Tape units to determine density
	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s);

	/*
	 * Clear out action struct
	 */
	bzero(&action, sizeof(CTAPE_ACTION));

	/* 
	 * Pass down the err flags we don't want reported to Error log
	 */
	ctape_move_tape(pdrv_dev, &action, ctape_done, 
		MTFSR, 1, NULL, CTAPE_SLEEP, 
		(U32)(ERR_EOM | ERR_FILEMARK | ERR_MEDIUM | ERR_HARDWARE |
		ERR_BLANK_CHK));


	if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: MTFSR, CCB = NULL\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	    if(( action.act_fatal & ACT_RESOURCE ) != NULL ){
	    	/* 
	    	 * Could not get resources needed (ccb's);
	    	 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);

	    	ccmn_close_unit(dev);
	    	return(action.act_ret_error); /* driver/resource problem */
	    }

	    if( (flags & (FNDELAY|FNONBLOCK)) == NULL ) {
		/*
		 * close the unit and return errno
		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);

		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	    /*
	     * The user has set fndelay must return success
	     */
	    else {
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				(state_flags | CTAPE_NOT_READY_STATE));

		return(CTAPE_SUCCESS);

	    }

	}


	/*
	 * At this point we have a completed ccb
	 * Check to see if it completed successfully
	 */ 

	if(action.act_ccb_status == CAT_CMP) {
	    /*
	     * At this point in time we are done with this ccb
	     * we release it back to the pools
	     */
	    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	    /*
	     * Now release the i/o ccb
	     */

	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	} /* end of if status == CAT_CMP */

	/*
	 * The move tape for density failed find out why
	 * The only error that will cause an open with
	 * the FNDELAY flag set is EBUSY (reservation 
	 * conflict) to return. 
	 */

	else {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Move tape fwd failed flags = 0x%X\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    module, ts_spec->ts_flags));
	    /*
	     * At this point in time we are done with this ccb
	     * we release it back to the pools
	     */
	    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);


	    /*
	     * Now release the i/o ccb
	     */

	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	    if( (action.act_scsi_status == SCSI_STAT_RESERVATION_CONFLICT) ){
		/* 
		 * close the unit and return ERROR
 		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	    /* 
	     * We could have hit a filemark eom blank check etc.....
	     * If we did just clear the flags....
	     */
	    PDRV_IPLSMP_LOCK( pdrv_dev,LK_RETRY, s);

	    if((action.act_chkcond_error == CHK_EOM) ||
			( action.act_chkcond_error == CHK_FILEMARK) ||
			( action.act_chkcond_error == CHK_MEDIUM_ERR ) ||
			( action.act_chkcond_error == CHK_HARDWARE) ||
			( action.act_chkcond_error == CHK_BLANK_CHK) ||
			( action.act_chkcond_error == CHK_RECOVER ) ||
			( action.act_chkcond_error == CHK_INFORMATIONAL )){
		/*
		 * Thats cool just clear them
		 */
		ts_spec->ts_flags &= ~(CTAPE_EOM | CTAPE_TPMARK | 
				CTAPE_TPMARK_PENDING | CTAPE_HARDERR );
		PDRV_IPLSMP_UNLOCK( pdrv_dev, s);
	    }


	    else if(( flags & (FNDELAY|FNONBLOCK) ) != NULL) {
		/*
		 * FORCE OPEN POSIX 
		 */

    	    	ts_spec->ts_state_flags |= (state_flags | 
						CTAPE_NOT_READY_STATE);

		PDRV_IPLSMP_UNLOCK( pdrv_dev, s);

		/*
		 * try to get back to BOM
		 */
	        bzero(&action, sizeof(CTAPE_ACTION));

		ctape_move_tape(pdrv_dev, &action, ctape_done, 
			MTREW, 1, NULL, CTAPE_SLEEP, (U32)NULL);

	        CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);


	        /*
	         * Now release the i/o ccb
	         */

	        ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

		return(CTAPE_SUCCESS);

	    }
	    else {
		/* 
		 * Some other error
		 * close the unit and return ERROR
		 */
    	    	ts_spec->ts_state_flags |= state_flags;
		PDRV_IPLSMP_UNLOCK( pdrv_dev, s);

		/*
		 * try to get back to BOM
		 */
	        bzero(&action, sizeof(CTAPE_ACTION));

		ctape_move_tape(pdrv_dev, &action, ctape_done, 
			MTREW, 1, NULL, CTAPE_SLEEP, (U32)NULL);

	        CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);


	        /*
	         * Now release the i/o ccb
	         */

	        ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	} /* End of if != SUCCESS */


	/*
	 * Lets get back to the beinging of tape/
	 */
	/*
	 * Clear out action struct
	 */
	bzero(&action, sizeof(CTAPE_ACTION));

	ctape_move_tape(pdrv_dev, &action, ctape_done, 
		MTREW, 1, NULL, CTAPE_SLEEP, (U32)NULL);

	if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: MTREW, CCB = NULL\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	    if(( action.act_fatal & ACT_RESOURCE ) != NULL ){
	    	/* 
	    	 * Could not get resources needed (ccb's);
	    	 */

		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
	    	ccmn_close_unit(dev);
	    	return(action.act_ret_error); /* driver/resource problem */
	    }

	    if( (flags & (FNDELAY|FNONBLOCK)) == NULL ) {
		/*
		 * close the unit and return errno
		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	    /*
	     * The user has set fndelay must return success
	     */
	    else {
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				(state_flags | CTAPE_NOT_READY_STATE));

		return(CTAPE_SUCCESS);

	    }

	}


	if(action.act_ccb_status == CAT_CMP) {
	    /*
	     * At this point in time we are done with this ccb
	     * we release it back to the pools
	     */
	    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	    /*
	     * Now release the i/o ccb
	     */

	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	}

	/*
	 * The REW failed 
	 */

	else {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Move tape rew failed flags = 0x%X\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    module, ts_spec->ts_flags));

	    /*
	     * At this point in time we are done with this ccb
	     * we release it back to the pools
	     */
	    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	    /*
	     * Now release the i/o ccb
	     */

	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	    if( (action.act_scsi_status == SCSI_STAT_RESERVATION_CONFLICT) || 
			((flags & (FNDELAY|FNONBLOCK)) == NULL) ){
		/* 
		 * close the unit and return ERROR
 		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	    else { /* FNDELAY AND NOT reservation conflixct */
		/* 
		 * FORCE OPEN POSIX
		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				(state_flags | CTAPE_NOT_READY_STATE));

		return(CTAPE_SUCCESS);

	    }

	} /* End of if != SUCCESS */

	/*
	 * Now we have to see what density and blocking
	 * we are at. Do a mode sense command. This routine
	 * returns to us a completed ccb_io (sent to xpt). 
	 * It is up to caller to determine what the status
	 * of the command is.
	 */
	/*
	 * Clear out action struct
	 */
	bzero(&action, sizeof(CTAPE_ACTION));

	ctape_mode_sns( pdrv_dev,&action, ctape_done, SEQ_NO_PAGE, 
			ALL_PCFM_CURRENT, CTAPE_SLEEP);

	if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: MODSNS, CCB = NULL\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	    if(( action.act_fatal & ACT_RESOURCE ) != NULL ){
	    	/* 
	    	 * Could not get resources needed (ccb's);
	    	 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);

	    	ccmn_close_unit(dev);
	    	return(action.act_ret_error); /* driver/resource problem */
	    }

	    if( (flags & (FNDELAY|FNONBLOCK)) == NULL ) {
		/*
		 * close the unit and return errno
		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	    /*
	     * The user has set fndelay must return success
	     */
	    else {
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				(state_flags | CTAPE_NOT_READY_STATE));

		return(CTAPE_SUCCESS);

	    }

	}


	/*
	 * At this point we have a completed ccb
	 * Check to see if it completed successfully
	 */ 
	if((action.act_ccb_status == CAT_CMP) || ((action.act_fatal &
			ACT_FAILED) == NULL)){

	    /*
	     * Now what we have to do is see what density the is set at
	     * and the blocking factor. Gaze into mode sense data in 
	     * the block descriptor section and extract the info needed.
	     */
	    PDRV_IPLSMP_LOCK( pdrv_dev,LK_RETRY, s);

	    ts_spec->ts_density = 
			((SEQ_MODE_DATA6 *)action.act_ccb->cam_data_ptr)->
					sel_desc.density_code;
	 
	    if (ts_spec->ts_density <= density_entrys){
	       ts_spec->ts_block_size = density_table[ts_spec->ts_density]
					.den_blocking;
	    }
	    else { 
	
	       /* default to variable blocking factor */
	  
	       ts_spec->ts_block_size = density_table[0].den_blocking;
	       ts_spec->ts_density = density_table[0].dens_code;
	    }
	    /*
	     * Set auto density valid
	     */

	    ts_spec->ts_state_flags |= CTAPE_AUTO_DENSITY_VALID_STATE;


	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s);
	    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	    CTAPE_REL_MEM(action.act_ccb);

	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	}

	/*
	 * The mode sense for density failed find out why
	 * The only error that will cause an open with
	 * the FNDELAY flag set is EBUSY (reservation 
	 * conflict) to return. 
	 */

	else {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: MODE SENSE FAILED of density\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	    /*
	     * At this point in time we are done with this ccb
	     * we release it back to the pools
	     */
	    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);


	    /*
	     * Now release the i/o ccb and memory gotten.....
	     */
	    CTAPE_REL_MEM(action.act_ccb);

	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	    if( (action.act_scsi_status == SCSI_STAT_RESERVATION_CONFLICT) || 
			((flags & (FNDELAY|FNONBLOCK)) == NULL) ){
		/* 
		 * close the unit and return ERROR
 		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	    else { /* FNDELAY AND NOT reservation conflixct */
		/* 
		 * FORCE OPEN POSIX
		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				(state_flags | CTAPE_NOT_READY_STATE));

		return(CTAPE_SUCCESS);

	    }


	} /* End of if != SUCCESS */
          

    } /* end of if reset or unit attention for auto density */	    

    else {
	/*
	 * Just unlock it
	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s);
    }

    /*
     * Set our density up if unit atten or reset.
     */

    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s  );
    
    /* 
     * The if checks to see if we had a reset when the unit was closed
     * and if one has occurred while in the opening state, We also
     * allow the setting of density at BOM.
     */
    if(((state_flags & (CTAPE_UNIT_ATTEN_STATE | CTAPE_RESET_STATE)) !=
		NULL ) || ((ts_spec->ts_state_flags & (CTAPE_RESET_STATE |
		CTAPE_UNIT_ATTEN_STATE)) != NULL) || ((ts_spec->ts_flags &
		CTAPE_BOM) != NULL)) {

	/*
	 * We have had a unit attention or reset must set the
	 * density up 
	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s);

	bzero(&action, sizeof(CTAPE_ACTION));
        
        ctape_density_set(pdrv_dev, &action, ctape_done, CTAPE_SLEEP, CTAPE_READ); 

	/* 
	 * Start checking the results.
	 */
	if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: Density_set, CCB = NULL\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	    if(( action.act_fatal & ACT_RESOURCE ) != NULL ){
	    	/* 
	    	 * Could not get resources needed (ccb's);
	    	 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);

	    	ccmn_close_unit(dev);
	    	return(action.act_ret_error); /* driver/resource problem */
	    }

	    if((flags & (FNDELAY|FNONBLOCK)) == NULL){
		/*
		 * close the unit and return errno
		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	    /*
	     * The user has set fndelay must return success
	     */
	    else {
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				(state_flags | CTAPE_NOT_READY_STATE));

		return(CTAPE_SUCCESS);

	    }

	}


	if(action.act_ccb_status == CAT_CMP) {
	    /*
	     * At this point in time we are done with this ccb
	     * we release it back to the pools
	     */
	    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	    /*
	     * Now release the i/o ccb and memory gotten.....
	     */

	    CTAPE_REL_MEM( action.act_ccb );
	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	} /* end if status == CAT_CMP */

	/*
	 * The modsel for density failed find out why
	 * The only error that will cause an open with
	 * the FNDELAY flag set is EBUSY (reservation 
	 * conflict) to return. 
	 */

	else {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: DENS_SET Failed\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	    /*
	     * At this point in time we are done with this ccb
	     * we release it back to the pools
	     */
	    CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	    /*
	     * Now release the i/o ccb and memory gotten.....
	     */

	    CTAPE_REL_MEM(action.act_ccb);
	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	    /* 
	     * If reservation conflict or fndelay == NULL then error 
	     * This open out
	     */
	    if((action.act_scsi_status == SCSI_STAT_RESERVATION_CONFLICT) || 
			((FNDELAY|FNONBLOCK)  != NULL)) {
		/*
		 * close the unit and return EBUSY
		 */
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, state_flags);
		ccmn_close_unit(dev);
		return( action.act_ret_error );
	    }

	    /*
	     * The user has set fndelay must return success
	     */
	    else {
		CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, 
				(state_flags | CTAPE_NOT_READY_STATE));

		return(CTAPE_SUCCESS);

	    }


	} /* End of if != SUCCESS */

    } /* end of if reset or unit attention */	    

    else {
	/*
	 * Just unlock it
	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
    }


    /* 
     * We have the unit open..
     * We check to see if a reset 
     * One last thing before we return set up our flags
     * for a consistent state.
     */
    PDRV_IPLSMP_LOCK( pdrv_dev,LK_RETRY, s);
    if((ts_spec->ts_state_flags & (CTAPE_RESET_STATE | 
			CTAPE_RESET_PENDING_STATE)) != NULL ) {
        PDRV_IPLSMP_UNLOCK( pdrv_dev, s);
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: RESET is detected s_flags = 0x%X\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		module, ts_spec->ts_state_flags));
	/*
	 * A reset has come by while we where trying to open
	 * We will retry next version. This one just fail it
	 */
	if(( flags & (FNDELAY|FNONBLOCK) ) != NULL) {
	    /* 
	     * FORCE OPEN POSIX
	     */

	    ts_spec->ts_state_flags |= CTAPE_NOT_READY_STATE;
	    return(CTAPE_SUCCESS);
	}
	else {
	    /* 
	     * close the unit and return ERROR
 	     */
	    ccmn_close_unit(dev);
	    return( EIO );
	}
    }

    ts_spec->ts_flags &=  (CTAPE_BOM | CTAPE_EOM | CTAPE_WRT_PROT );
    ts_spec->ts_state_flags &=  (CTAPE_AUTO_DENSITY_VALID_STATE |
		CTAPE_POSITION_LOST_STATE);
    ts_spec->ts_state_flags |=  CTAPE_OPENED_STATE;

    PDRV_IPLSMP_UNLOCK( pdrv_dev, s);


    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n",
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));
	    
    return(CTAPE_SUCCESS);
} /* End of ctape_online() */




/* ---------------------------------------------------------------------- */
/* Function description.
 *
 * Routine name ctape_close
 *
 * 	This routine handles the closing out of the unit.
 *	If Written write file marks Based of density struct
 *	direction flags. 
 *	
 *
 * Call syntax
 *  ctape_close( dev, flags )
 * 
 * Implicit inputs
 * 	Flag of CTAPE_TPMARK_PENDINING
 *	Density struct direction flags
 *	Device rewind bit.
 *
 * Implicit outputs 
 *	NONE	
 *
 * Return values
 *	CTAPE_SUCCESS
 *	ENOMEM	Resource problem
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */


int 
ctape_close(dev, flags)


    dev_t dev;		/*  major and minor pair   		 	*/
    int flags;		/* Flags RDONLY READ WRITE FNDELAY etc.	*/

{

    /*
     * LOCAL VARIABLES 
     */

	
    PDRV_DEVICE		*pdrv_dev;	/* Ptr to our device struct	*/



    DENSITY_TBL		*dens;		/* For tapes we have to know what
		 			 * density we are going to write 
					 * at.. Reads go thru auto_density
					 */

    TAPE_SPECIFIC	*ts_spec;	/* Ptr to our device specific
					 * struct
					 */
    CTAPE_ACTION	action;		/* Our action struct for sub-routines
					 */


    U32		s;		/* For our saved ipl		*/

    I32		idx;		/* Density table index		*/

    I32		fm_cnt;		/* Number of file marks to do	*/
    static u_char	module[] = "ctape_close"; /* Module name	*/

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

    /* 
     * Get our peripherial device pointer 
     */
    if( (pdrv_dev = GET_PDRV_PTR(dev)) == (PDRV_DEVICE *)NULL){
        /*
         * This should not happen no pdrv_device struct...
         */
         CAM_ERROR( module, "No device struct", CAM_SOFTWARE,
			     (CCB_HEADER *)NULL, dev, (u_char *)NULL);
         return(ENOMEM);
    }



    /* 
     * Get our device specific pointer 
     */

    ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;
    if( (ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific) ==
			    (TAPE_SPECIFIC *)NULL){
        PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
	CAM_ERROR( module, "No tape specific struct", CAM_SOFTWARE,
			 (CCB_HEADER *)NULL, dev, (u_char *)NULL);
        PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	return(ENOMEM);
    }

    /*
     * Set the pointer to our density struct
     */
    dens = pdrv_dev->pd_dev_desc->dd_density_tbl;
								      


    /*
     * Check to see if a unit attention, reset , or if the unit
     * is in a not ready state.
     * Don't do anything just close down the unit.
     */
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    if((ts_spec->ts_state_flags & (CTAPE_UNIT_ATTEN_STATE | 
		CTAPE_RESET_STATE | CTAPE_NOT_READY_STATE)) != NULL ){
	
	/*
	 * Just close it down we can't know where position is
	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	ccmn_close_unit(dev);
	return(CTAPE_SUCCESS);

    }


    /*
     * Now check to see if they had write permissions and they
     * wrote the tape. If wriiten we have to write file marks 
     * and we don't have position lost.
     */
    if((( flags & FWRITE ) != NULL) && ((ts_spec->ts_flags & 
			CTAPE_WRITTEN) !=NULL) && ((ts_spec->ts_state_flags &
			CTAPE_POSITION_LOST_STATE) == NULL)) { 

	PDRV_IPLSMP_UNLOCK( pdrv_dev, s); 

	/*
	 * For this density check the number file marks we write on close
	 */
	idx = DEV_TO_DENS_IDX(dev);	
	if((dens->density[idx].den_flags & ONE_FM) != NULL) {
	    fm_cnt = 1;
	}
	else {
	    fm_cnt = 2;
	}
	/*
	 * Clear out action struct
	 */
	bzero(&action, sizeof(CTAPE_ACTION));


	ctape_wfm( pdrv_dev, &action, ctape_done, fm_cnt, NULL,
				CTAPE_SLEEP); 

	if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: resource problem for wfm\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	    ccmn_close_unit(dev);
	    return(action.act_ret_error); /* driver/resource problem */
	}

	/*
	 * At this point we have a completed ccb
	 * Check to see if it completed successfully
    	 * At this point in time we are done with this ccb 
	 * we release it back to the pools 
	 */
	CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

    	ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );



	/*
	 * The write file mark has failed clean up after ourselves
	 * Do we set UNIT_ATTEN for next open do a rewind ????
	 */
	if(action.act_ccb_status != CAT_CMP) {

    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: wfm failed\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));


	    /*
	     * We had some sort of condition reported
	     * It could be eom or an informational message.
	     * Check to see if it failed.
	     */
	    if(( action.act_fatal & ACT_FAILED ) != NULL){

	        /*
	         * Close unit down
	         */
    	        ccmn_close_unit(dev);

	        return( action.act_ret_error );
	    }

	} /* End of if != SUCCESS */

	/* 
	 * Well we wrote the file mark's now  lets see if
	 * we need to back space over one of them.
	 */
	if( fm_cnt == 2 ) {

	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_move_tape(pdrv_dev, &action, ctape_done, 
				MTBSF, 1, NULL, CTAPE_SLEEP, (U32)NULL);

	    if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			(CAMD_TAPE ), 
			("[%d/%d/%d] %s: BSF NULL CCB\n", 
			DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			module));
	        ccmn_close_unit(dev);
	        return(action.act_ret_error); /* driver/resource problem */
	    }


	    /*
	     * At this point we have a completed ccb
	     * Check to see if it completed successfully
	     * At this point in time we are done with this ccb
	     * we release it back to the pools
	     */
            CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	    /*
	     * Now release the i/o ccb
	     */

	    ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );


	    /*
	     * The back space over the file mark failed......
	     */
	    if(action.act_ccb_status != CAT_CMP) {
		/* 
		 * We check to see of the command really failed
		 */
		if((action.act_fatal & ACT_FAILED) != NULL) {
		    /* 
		     * Some other error
		     * close the unit and return ERROR
 		     */
    		    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			    (CAMD_TAPE ), 
			    ("[%d/%d/%d] %s: BSF failed flags = 0x%X\n", 
			    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			    module, ts_spec->ts_flags));

		    ccmn_close_unit(dev);
		    return( action.act_ret_error );
		}


		/* 
		 * check of tape flags.... We could have hit a filemark
		 * eom blank check etc.....Do we really want to do this
		 */
		PDRV_IPLSMP_LOCK( pdrv_dev,LK_RETRY, s);
		if((ts_spec->ts_flags & (CTAPE_EOM | CTAPE_TPMARK |
			    CTAPE_TPMARK_PENDING )) != NULL ) {
		    /*
		     * Thats cool just clear them
		     */
		    ts_spec->ts_flags &= ~(CTAPE_EOM | CTAPE_TPMARK | 
				CTAPE_TPMARK_PENDING );
		    PDRV_IPLSMP_UNLOCK( pdrv_dev, s);
		}


	    } /* End of if != SUCCESS */

	} /* End if fm_cnt == 2 */

    } /* end of if tape written */
    /*
     * Tape has not been written,
     */

    else {
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s);
    }

    /*
     * Now check if we need to rewind the unit..
     */
    if((dev & CTAPE_NOREWIND_DEV ) == NULL ) {
	/* 
	 * we now issue a rewind cmd
	 */
	/*
	 * Clear out action struct
	 */
	bzero(&action, sizeof(CTAPE_ACTION));

	
	/*
	 * Please note..... That We will not sleep after the command 
	 * has been issued...We set the state flag of ORPHANE_CMD_STATE.
	 * In the open we block on that flag until it is cleared...
	 * This solves the problem of a unit rewinding ....but the tur
	 * unit ready cmd returns success if we then issue any other cmd
	 * some units will take it but disconnect and wait for the rewind
	 * to complete.. We get hosed if that happens with command timeouts.
	 * NO CCB CHECKING IS DONE HERE.....
	 */
	CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, CTAPE_ORPHAN_CMD_STATE);

	ctape_move_tape(pdrv_dev, &action, ctape_orphan_done, 
		MTREW, 1, NULL, CTAPE_NOSLEEP, (U32)NULL);

	if(action.act_ccb == (CCB_SCSIIO *)NULL)  {
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: REW NULL CCB\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

    	    PDRV_IPLSMP_LOCK( pdrv_dev,LK_RETRY, s);
	    /*
	     * Can't leave flag set......
	     */

	    ts_spec->ts_state_flags &= ~CTAPE_ORPHAN_CMD_STATE;

	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s);

	    ccmn_close_unit(dev);
	    return(action.act_ret_error); /* driver/resource problem */
	}

    } /* End if rewind

    /*
     * Well no rewind, check to see if a tape mark is pending
     * We will move the tape over the tape mark to maintain
     * position across closes.
     */

    PDRV_IPLSMP_LOCK( pdrv_dev,LK_RETRY, s);

    if((( ts_spec->ts_flags & CTAPE_TPMARK_PENDING) != NULL) &&  
		(dev & CTAPE_NOREWIND_DEV ) != NULL ) {

	/*
	 * Clear TPMARK_PENDING.....
	 */
	ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;

	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	/*
	 * Clear out action struct
	 */
	bzero(&action, sizeof(CTAPE_ACTION));


	ctape_move_tape(pdrv_dev, &action, ctape_done, 
		MTBSF, 1, NULL, CTAPE_SLEEP, (U32)NULL);

	if(action.act_ccb == (CCB_SCSIIO *)NULL) {

    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: NULL CCB for bsf over tpmark pending\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	    ccmn_close_unit(dev);
	    return(action.act_ret_error); /* driver/resource problem */
	}

	/*
	 * At this point we have a completed ccb
	 * Check to see if it completed successfully
         * At this point in time we are done with this ccb
         * we release it back to the pools
         */
        CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

        /*
         * Now release the i/o ccb
         */

        ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );


	if(action.act_ccb_status != CAT_CMP) {
	    /*
	     * The move tape for tape mark pending failed
	     */
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: tapemark pending bsf failed flags\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));


	    if((action.act_fatal & ACT_FAILED) != NULL) {
	        /* 
	         * Some other error
	         * close the unit and return ERROR
 	         */
    		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			(CAMD_TAPE ), 
			("[%d/%d/%d] %s: BSF failed flags = 0x%X\n", 
			DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			module, ts_spec->ts_flags));

	        ccmn_close_unit(dev);
	        return( action.act_ret_error );
	    }


	} /* End of if != SUCCESS */
	/*
	 * Now clear the flags 
	 */

	PDRV_IPLSMP_LOCK( pdrv_dev,LK_RETRY, s);

	ts_spec->ts_flags &= ~(CTAPE_EOM | CTAPE_TPMARK | 
				CTAPE_TPMARK_PENDING );

	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

    } /* end if Tape mark pending */
    else {

	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    }


    /*
     * If we have gotten to this point just close down
     * the unit
     */
    ccmn_close_unit(dev);
    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n",
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));
    return( CTAPE_SUCCESS );

} /* End of tape_close() */




/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * Routine ctape_read
 *
 * Functional Description:
 * 	This routine handles user process's synchronous read requests.
 * 	This is a pass through function, that gets a bp and then passes
 *	the work to ctape_strategy
 *
 * Call syntax
 *  ctape_read( dev, uio)
 *	dev_t		dev;		 Major/minor pair	
 *	struct 		*uio		 Pointer to the uio struct
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	NONE
 *
 * Return values
 *	Passes return from physio()
 *
 * TO DO:
 */




int
ctape_read( dev, uio)
	dev_t		dev;		/* Major/minor pair		*/
	struct uio	*uio;		/* Pointer to the uio struct	*/

{
    /*
     * Local variables
     */
    int		ret_val;		/* What we will return		*/
    struct buf	*bp;			/* Our allocated bp		*/
    static u_char	module[] = "ctape_read"; /* Module name	*/

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
    /* 
     * Get our struct bp allocated
     */
    bp = ccmn_get_bp();

    if( bp == NULL ){
	CAM_ERROR(module, "Can't get a bp struct", CAM_SOFTWARE,
		(CCB_HEADER *)NULL, dev, (u_char *)NULL);
	return (ENOMEM);
    }

	
    ret_val =  physio(ctape_strategy, bp, dev, B_READ, ctape_minphys, uio);

    /* 
     * Release the bp
     */
    ccmn_rel_bp( bp );

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), ("[%d/%d/%d] %s: exit\n",
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));

    return( ret_val );

} /* end of ctape_read */





/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * Routine ctape_write
 *
 * Functional Description:
 * 	This routine handles user process's synchronous write requests.
 * 	This is a pass through function, that gets a bp and then passes
 *	the work to ctape_strategy
 *
 * Call syntax
 *  ctape_write( dev, uio)
 *	dev_t		dev;		 Major/minor pair	
 *	struct 		*uio		 Pointer to the uio struct
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	NONE
 *
 * Return values
 *	Passes return from physio()
 *
 * TO DO:
 */




int 
ctape_write( dev, uio)
	dev_t		dev;		/* Major/minor pair		*/
	struct uio	*uio;		/* Pointer to the uio struct	*/

{
    /*
     * Local variables
     */
    int		ret_val;		/* What we will return		*/
    struct buf	*bp;			/* Our allocated bp		*/
    static u_char	module[] = "ctape_write"; /* Module name	*/

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

    /* 
     * Get our struct bp allocated
     */
    bp = ccmn_get_bp();

    if( bp == NULL ){
	CAM_ERROR(module, "Can't get a bp struct", CAM_SOFTWARE,
		(CCB_HEADER *)NULL, dev, (u_char *)NULL);
	return (ENOMEM);
    }

	
    ret_val =  physio(ctape_strategy, bp, dev, B_WRITE, ctape_minphys, uio);

    /* 
     * Release the bp
     */
    ccmn_rel_bp( bp );

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n",
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));

    return( ret_val );

} /* end of ctape_write */





/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 *	This rouitne handles all I/O request for user processes.
 *	A number of checks all made based on whether the request
 *	is synchronous or asynchronous. Based on that information
 *	tape status is checked to determine if the request is 
 *	allowed or not.
 *
 * Call syntax
 *  ctape_strategy( bp )
 *		struct	buf	*bp 	Pointer to the struct buf
 *
 * 
 * Implicit inputs
 *	In the bp, whether the request is a read or write, synchronous
 *	or asynchronous.
 *	Various state conditions, in the tape specific structure.
 *	Various tape conditions, in the tape specific structure.
 *
 * Implicit outputs 
 *	None.
 *
 * Return values
 *
 * TO DO:
 *
 */


void
ctape_strategy( bp )
	
	struct 	buf	*bp;	/* Pointer to the struct buf		*/

{

    /* 
     * Local variables.
     */
    PDRV_DEVICE		*pdrv_dev;	/* Our device descriptor struct	*/

    DEV_DESC		*dev_desc;	/* Device descriptor struct	*/

    TAPE_SPECIFIC	*ts_spec;	/* Pointer to the tape specific	*/
    CTAPE_ACTION   	action;  	/* What all the subroutines use
					 * to communicate back to the
                			 * caller
  					 */

    DENSITY_TBL 	*dens;		/* Pointer to density table 
					 * for this device 
  					 */

    I32			idx; 		/* Index into the density
                                         * desc 
					 */  


    CCB_SCSIIO		*ccb_io;	/* The Cam ccb for I/O		*/

    U32		ccb_flags;	/* The flags we will set in the 
					 * ccb
					 */
    I32		count;		/* Used for count to cdb	*/
    SEQ_READ_CDB6	*rd_cdb;	/* Pointer in ccb for cdb	*/
    SEQ_WRITE_CDB6	*wt_cdb;	/* Pointer in ccb for cdb	*/
    U32		send_stat;	/* What is returned from send ccb */

    static u_char	module[] = "ctape_strategy"; /* Module name	*/

    int			s;		/* Saved IPL			*/
    dev_t 		dev;		/* Major/minor (CAM) device pair.	*/
    u_char		sense_size;	/* The request sense size	*/

    /*
     * Get our device major/minor pair 
     */
    dev = bp->b_dev;

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

    /* 
     * get our device struct pointer
     */
    pdrv_dev = GET_PDRV_PTR(dev);

    /* 
     * get our device descriptor struct pointer
     */
    dev_desc = pdrv_dev->pd_dev_desc;
    /* 
     * get our tape specific pointer
     */
    ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;

    /*
     * Lock the struct now because we notice items out of it.
     */
    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);


    /* 
     * Check to see if the device was opened with the FNDELAY
     * and it was not ready and we have not tried to bring
     * it online one last time.
     */
    if((( ts_spec->ts_state_flags & CTAPE_NOT_READY_STATE ) != NULL ) &&
		((ts_spec->ts_state_flags & CTAPE_TRIED_ONLINE_STATE ) != 
		NULL)){

    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: NOT ready state flags = %0xX\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		module, ts_spec->ts_state_flags));

	 /*
	  * Do not allow I/O operations to the unit
	  */
	 PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	 CTAPE_BERROR(bp, bp->b_bcount, EINVAL);
	 biodone( bp );
	 return;
    }
    /* 
     * If not ready try to bring it online..
     */
    if(( ts_spec->ts_state_flags & CTAPE_NOT_READY_STATE ) != NULL ) { 
	/*
	 * Set the tried online state ...
	 */
	ts_spec->ts_state_flags |= CTAPE_TRIED_ONLINE_STATE;

	/* 
	 * We must unlock first
	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

	if( ctape_online(dev, (int)NULL, NO_TIME ) != CTAPE_SUCCESS){
	    CTAPE_BERROR(bp, bp->b_bcount, EINVAL);
	    biodone( bp );
	    return;
	}
	/* 
	 * Well the unit is now ready. We did everything needed
	 * like setting the density etc...
	 * relock ourselfs
	 */

    	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
    }


    /*
     * Check to see if position is lost. If so we can't allow
     * any operations that deal with data.
     */

    if(( ts_spec->ts_state_flags & CTAPE_POSITION_LOST_STATE ) != NULL ){
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: POSITION_LOST IS SET stateflags = 0x%X\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		module, ts_spec->ts_state_flags));

	 /*
	  * Do not allow I/O operations to the unit
	  */
	 PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	 CTAPE_BERROR(bp, bp->b_bcount, EIO);
	 biodone( bp );
	 return;
    }
	

    /*
     * Check to see if we are at end of media and we have not
     * disabled notification.
     */

    if((( ts_spec->ts_flags & CTAPE_EOM) != NULL) && ((ts_spec->ts_state_flags
		& CTAPE_DISEOT_STATE ) == NULL )){
	
	/* 
	 * We have EOM and we have NOT disabled eot notification
	 */
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: EOM  flags = 0x%X\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		module, ts_spec->ts_flags));

	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	CTAPE_BERROR(bp, bp->b_bcount, ENOSPC);
	biodone( bp );
	return;
    }

#ifndef OSF
    /*
     * Check to see if hard error has occurred on raw async I/O
     */
    if((( BOP_IS_RAWASYNC_SET(bp->b_flags) ) != NULL ) && ((ts_spec->ts_flags &
			CTAPE_HARDERR) != NULL ) ){ 
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: HARDERR  flags = 0x%X\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		module, ts_spec->ts_flags));

	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	CTAPE_BERROR(bp, bp->b_bcount, EIO);
	biodone( bp );
	return;
    }

    /*
     * We check for any abort state for nbuf I/O
     */
    if((( BOP_IS_RAWASYNC_SET(bp->b_flags)) != NULL ) &&
        ((ts_spec->ts_state_flags & CTAPE_ABORT_TPPEND_STATE) != NULL )){ 
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: ABORT TP stateflags = 0x%X\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		module, ts_spec->ts_state_flags));

	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	CTAPE_BERROR(bp, bp->b_bcount, EIO);
	biodone( bp );
	return;
    }
#endif /* !OSF */

    /*
     * Special checks for read cases
     */
    if(( bp->b_flags & B_READ ) != NULL ){

	ccb_flags =  (U32)CAM_DIR_IN; 
	
#ifndef OSF
	/*
	 * Check if tape mark and raw async 
	 */
	if((( BOP_IS_RAWASYNC_SET(bp->b_flags)) != NULL ) &&
	   ((ts_spec->ts_flags & CTAPE_TPMARK) != NULL )){ 

    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Read tpmark, flags = 0x%X\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    module, ts_spec->ts_flags));

	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	    CTAPE_BERROR(bp, bp->b_bcount, EIO);
	    biodone( bp );
	    return;
	}
#endif /* !OSF */

	/* 
	 * Check for a tape mark pending (fixed block units ) 
	 * For both Sync and async....If there was any thing
	 * on the queues when the tape mark was detected for 
	 * fixed block tapes it is handled by the tpmark_pending
	 * abort state in ctape_iodone and above in this routine.
	 */
	if((ts_spec->ts_flags & CTAPE_TPMARK_PENDING ) != NULL ){ 
	 
	 /*
	if((( BOP_IS_RAWASYNC_SET(bp->b_flags) ) == NULL ) &&
	   ((ts_spec->ts_flags & CTAPE_TPMARK_PENDING ) != NULL )){ 
	   */

    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Read Tapemark pending flags = 0x%X\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    module, ts_spec->ts_flags));

	    ts_spec->ts_flags |= CTAPE_TPMARK;
	    ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;
	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	    /*
	     * DO NOT CTAPE_BERROR this Not an error
	     */
	    bp->b_resid = bp->b_bcount;
	    biodone( bp );
	    return;
	}

	/* 
	 * One last check.... Make sure we have valid density info
	 */
	if((ts_spec->ts_state_flags & CTAPE_AUTO_DENSITY_VALID_STATE ) == NULL){
	    printf("ctape_strat: READ case and density info not valid....\n");
	}

	/*
	 * Clear the written flag and TPMARK if set.....
	 */
	ts_spec->ts_flags &= ~(CTAPE_WRITTEN | CTAPE_TPMARK);
    }
    /*
     * Must be a write
     * For writes we validate the CTAPE_AUTO_VALID_STATE and the blocking
     * factor for this density....
     */
    else {

	ccb_flags =  (U32)CAM_DIR_OUT; 
         
	/* 
	 * Tape could have been written before... we only change
	 * densities at BOM.........
	 */


        if((ts_spec->ts_flags & CTAPE_BOM) != NULL) {

          bzero(&action, sizeof(CTAPE_ACTION));

          /*
           * Get the index into the density table
           */
          idx = DEV_TO_DENS_IDX(pdrv_dev->pd_dev);

          /*
           * Set the pointer to our density struct
           */
          dens = dev_desc->dd_density_tbl;

          if( (dens == NULL) || ((dens->density[idx].den_flags &
                DENS_VALID) == NULL)) {
            /*
             * The density struct for this device null or the density
             * is not valid for it (the dev number)
             */

            PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
            CAM_ERROR(module, "Density structure wrong", CAM_HARDERR,
                    (CCB_HEADER *)NULL, pdrv_dev->pd_dev,
                    (u_char *)NULL);
            PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
            CTAPE_BERROR(bp, bp->b_bcount, EINVAL);
   	    biodone( bp );
	    return;
          } 
       	  
          /*
           * Check to see if we need to set compression for
           * this density/selection.
           */
          if(( dens->density[idx].den_flags & DENS_COMPRESS_VALID ) != NULL) {
                ctape_compress_set( pdrv_dev, &action, ctape_done,
                                        CTAPE_SLEEP );
          }

          /*
           * Just a normal tape
           */
          else {
              ctape_density_set(pdrv_dev, &action, ctape_done, CTAPE_SLEEP, CTAPE_WRITE);

          }

          /*
           * Start checking the results.
           */
          if(action.act_ccb == (CCB_SCSIIO *)NULL) {
              PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
               (CAMD_TAPE ),
               ("[%d/%d/%d] %s: Density_set, CCB = NULL\n",
               DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
   
               CTAPE_BERROR(bp, bp->b_bcount, action.act_ret_error);
               biodone( bp );
               return;

                /* driver/resource problem */
          }

	  /*
           * At this point we have a completed ccb
           * Check to see if it completed successfully
           */

          if(action.act_ccb_status == CAT_CMP) {
            /*
             * At this point in time we are done with this ccb
             * we release it back to the pools
             */
            CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);
       
            /*
             * Now release the i/o ccb and memory gotten.....
             */

            CTAPE_REL_MEM( action.act_ccb );
            ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

          } /* end if status == CAT_CMP */

          /*
           * The modsel for density failed find out why
           * The only error that will cause an open with
           * the FNDELAY flag set is EBUSY (reservation
           * conflict) to return.
           */

          else {
             PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
                  (CAMD_TAPE ),
                  ("[%d/%d/%d] %s: DENS_SET Failed\n",
                  DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
             /*
              * At this point in time we are done with this ccb
              * we release it back to the pools
              */
             CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

             /*
              * Now release the i/o ccb and memory gotten.....
              */

             CTAPE_REL_MEM(action.act_ccb);
             ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );
    
             CTAPE_BERROR(bp, bp->b_bcount, action.act_ret_error)     
             biodone( bp );
             return;
   
          } /* End of if != SUCCESS */
         
          ts_spec->ts_state_flags |= CTAPE_AUTO_DENSITY_VALID_STATE;
          ts_spec->ts_block_size = dev_desc->dd_density_tbl->density
                            [DEV_TO_DENS_IDX(dev)].den_blocking;
          ts_spec->ts_density = dev_desc->dd_density_tbl->density
                            [DEV_TO_DENS_IDX(dev)].den_density_code;

        } /* End of if CTAPE_BOM != NULL */
        
        else {
          /*
           * Just unlock it
           */
          PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

        }  /* End of if != BOM */
       
        ts_spec->ts_flags |= CTAPE_WRITTEN;

    } /* if write */    
      
 
    /*
     * Check if we are running in a fix block mode and if the user
     * is doing non multiples of block size
     */
    if( ts_spec->ts_block_size != NULL ){
	/*
	 * Running in fixed block
	 */
	if(( bp->b_bcount % ts_spec->ts_block_size ) != NULL ) {

    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Block and not multiple\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	    uprintf("Tape is fixed block and byte count is not a multiple of block size.\n"); 
	    uprintf("Blocking factor of tape is %D\n", ts_spec->ts_block_size);
	    CTAPE_BERROR(bp, bp->b_bcount, EINVAL);
	    biodone( bp );
	    return;
	}
    }
			
    /* 
     * See if the user has set the request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    if( (dev_desc->dd_valid & DD_REQSNS_VAL ) != NULL){
	sense_size = dev_desc->dd_req_sense_len;
    }
    else {
	sense_size = DEC_AUTO_SENSE_SIZE;
    }
    /*
     * Need to LOOK AT timeout values...
     */
    /*
     * Get an I/O ccb
     */
    ccb_io = ccmn_io_ccb_bld( dev, (u_char *)bp->b_un.b_addr, 
		(U32)bp->b_bcount, 
		sense_size, ccb_flags, ctape_iodone,(u_char)NULL,
		(( bp->b_bcount/10000) + ctape_io_base_timo), bp); 
    
    if( ccb_io == NULL){
    	/* 
    	 * We can't get any memory
    	 * ERROR LOG it
    	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: CCB = NULL\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

    	CAM_ERROR(module, "NULL CCB returned", CAM_SOFTWARE, 
		(CCB_HEADER *)NULL, dev, (u_char *)NULL);	
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	CTAPE_BERROR(bp, bp->b_bcount, ENOMEM);
	biodone( bp );
	return;
    }

    /* 
     * Clear BOM bit.. 
     */
    ts_spec->ts_flags &= ~CTAPE_BOM;
    /* 
     * Build the cdb (SCSI Command )
     */
    if(( bp->b_flags & B_READ ) != NULL){
	rd_cdb = (SEQ_READ_CDB6 *)ccb_io->cam_cdb_io.cam_cdb_bytes;

	rd_cdb->opcode = SEQ_READ_OP;
	rd_cdb->lun = 0;

        if( ts_spec->ts_block_size != NULL ){
	    /*
	     * Running in fixed block
	     */
	    count =  bp->b_bcount/ts_spec->ts_block_size; 
	    rd_cdb->fixed = 1;
	    SEQTRANS_TO_READ6( count, rd_cdb );
        }
	else {
	    SEQTRANS_TO_READ6( bp->b_bcount, rd_cdb );
	
	}
	/* 
	 * Set the length of the cdb
	 */
	ccb_io->cam_cdb_len = sizeof(SEQ_READ_CDB6);
    }
    /*
     * Must be write
     */
    else {
	wt_cdb = (SEQ_WRITE_CDB6 *)ccb_io->cam_cdb_io.cam_cdb_bytes;

	wt_cdb->opcode = SEQ_WRITE_OP;

	wt_cdb->lun = 0;

        if( ts_spec->ts_block_size != NULL ){
	    /*
	     * Running in fixed block
	     */
	    count =  bp->b_bcount/ts_spec->ts_block_size; 
	    wt_cdb->fixed = 1;
	    SEQTRANS_TO_WRITE6( count, wt_cdb );
        }
	else {
	    SEQTRANS_TO_WRITE6( bp->b_bcount, wt_cdb );
	
	}
	/* 
	 * Set the length of the cdb
	 */
	ccb_io->cam_cdb_len = sizeof(SEQ_WRITE_CDB6);
    }


    /*
     * Send it down...
     */

    send_stat = ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)ccb_io, NOT_RETRY);

    /*
     * If the ccb is not inprogress then we blew it...
     */
    if((send_stat & CAM_STATUS_MASK) != CAM_REQ_INPROG){
	/* 
	 * The ccb has been returned to us and has not gone thru
	 * ctape_iodone... call it and return.
	 */
    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		(CAMD_TAPE ), 
		("[%d/%d/%d] %s: send status NOT inprog\n", 
		DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	ctape_iodone(ccb_io);
	return;
    }
	
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);


    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n",
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));

    return;

} /* end of ctape_strategy */





/* ---------------------------------------------------------------------- */
/* Function description.
 *
 * Routine name ctape_ioctl
 *
 * 	This routine handles specific requests for actions other
 *	then read/write. The routine handles moving of tape and
 *	the reporting of tape status.
 *
 * Call syntax
 *  ctape_ioctl( dev, cmd, data, flags )
 * 
 * Implicit inputs
 * 	flag of CTAPE_TPMARK_PENDINING
 *
 * Implicit outputs 
 *	Various and flags based on command status.
 *	
 *
 * Return values
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */

int
ctape_ioctl( dev, cmd, data, flag )
	dev_t		dev;		/* Major/minor device pair	*/
	int		cmd;		/* The command we are doing	*/
	caddr_t		data;		/* 
					 * Ptr to kernel's copy of user
					 * request struct
					 */
	int		flag;		/* user flags			*/
{


    /*
     * Local Variables
     */

    PDRV_DEVICE 	*pdrv_dev;	/* Ptr to device struct		*/
    TAPE_SPECIFIC	*ts_spec;	/* Ptr to tape specifc struct	*/
    DEV_DESC            *dev_desc;	/* ptr to device desc		*/
    SEQ_MODE_DATA6	*msdp;		/* Mode sense data ptr		*/
    CTAPE_ACTION	action;		/* The action struct		*/
    struct mtop         *mtop;          /* For tape operations 		*/ 
    struct mtget        *mtget;         /* Status operations            */ 
    struct devget       *devget;        /* Device get ioctl             */ 
    struct device	*dinfo;		/* Used for devget only         */
    struct controller   *minfo;		/* Used for devget only         */
    I32                retries;        /* The number of time we try to
					 * do a mode sense for devget	
					 */
    int			s;		/* Saved IPL			*/
					/* Device unit number		*/
    dev_t		cam_dev;	/* For wrapper			*/
    static u_char	module[] = "ctape_ioctl"; /* Module name	*/


    cam_dev = dev;

    PRINTD(DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), DEV_LUN(cam_dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), DEV_LUN(cam_dev), module));


    /* 
     * Get our pointers
     */
    pdrv_dev = GET_PDRV_PTR(cam_dev);
    if( pdrv_dev == (PDRV_DEVICE *)NULL) {
	/* 
	 * There is no device struct
	 */
	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
	CAM_ERROR(module,"No PDRV_DEVICE struct", CAM_SOFTWARE,
		(CCB_HEADER *)NULL, cam_dev, (u_char *)NULL);
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	return(ENXIO);
    }

    dinfo = camdinfo[pdrv_dev->pd_log_unit];
    if(dinfo != (struct device *)NULL)
	minfo = camminfo[dinfo->ctlr_num];
    else
	minfo = (struct controller *)0;
 
    ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;
    if( ts_spec == (TAPE_SPECIFIC *)NULL){
	/*
	 * No tape specific struct
	 */
	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
	CAM_ERROR(module,"No TAPE_SPECIFIC struct", CAM_SOFTWARE,
		(CCB_HEADER *)NULL, cam_dev, (u_char *)NULL);
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	return(ENXIO);
    }

    dev_desc = pdrv_dev->pd_dev_desc;

    /*
     * Find out what we have to do. Look at command
     */
    switch (cmd) {

    /* 
     * Tape operations - if opened with FNDELAY no operations
     * are permitted.
     */
    case MTIOCTOP:

	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);

    	/* 
    	 * Check to see if the device was opened with the FNDELAY
    	 * and it was not ready and we have not tried to bring
    	 * it online one last time.
    	 */
    	if((( ts_spec->ts_state_flags & CTAPE_NOT_READY_STATE ) != NULL ) &&
		((ts_spec->ts_state_flags & CTAPE_TRIED_ONLINE_STATE ) != 
		NULL)){

    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: NOT ready state flags = %0xX\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    module, ts_spec->ts_state_flags));

	     /*
	      * Do not allow I/O operations to the unit
	      */
	     PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	     return(EINVAL);
    	}
    	/* 
    	 * If not ready try to bring it online..
    	 */
    	if(( ts_spec->ts_state_flags & CTAPE_NOT_READY_STATE ) != NULL ) { 
	    /*
	     * Set the tried online state ...
	     */
	    ts_spec->ts_state_flags |= CTAPE_TRIED_ONLINE_STATE;

	    /* 
	     * We must unlock first
	     */
	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

	    if( ctape_online(cam_dev, (int)NULL, NO_TIME ) != CTAPE_SUCCESS){
	        return(EINVAL);
	    }
	/* 
	 * Well the unit is now ready. We did everything needed
	 * like setting the density etc...
	 * relock ourselves
	 */

    	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
        }


        mtop = (struct mtop *)data;

	/*
	 * Check to see if position is lost........
	 */

	if( (ts_spec->ts_state_flags & CTAPE_POSITION_LOST_STATE) != NULL){
		/* 
		 * Only allow certain commands to come thru.
		 */
		switch( mtop->mt_op )
		{
		case MTREW:
		case MTOFFL:
		case MTRETEN:
			/* DO NOT UNLOCK HERE..... */
			break;

		default:
			PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
			return(EIO);
			/*NOTREACHED*/
			break;
		}

	}
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);



	/*
	 * Find out the tape command
	 */
        switch (mtop->mt_op) {

        case MTWEOF:
	    /*
	     * Write end of file 
	     */

	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_wfm( pdrv_dev, &action, ctape_done, mtop->mt_count, 
				NULL, CTAPE_SLEEP); 

            break;

        case MTFSF: 
        case MTBSF:
        case MTFSR:
        case MTBSR:
	case MTSEOD:
	    /*
	     * Space file marks/records
	     */
	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_move_tape( pdrv_dev, &action, ctape_done, 
			(I32)mtop->mt_op, (I32)mtop->mt_count, NULL, 
			CTAPE_SLEEP, (U32)NULL); 


            break;


        case MTOFFL:
	case MTUNLOAD:
	    /* 
	     * Issue an unload tape
	     * Set not ready state so we can bring it online..
	     */
	    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
	    ts_spec->ts_state_flags |= CTAPE_NOT_READY_STATE;
	    ts_spec->ts_state_flags &= ~CTAPE_TRIED_ONLINE_STATE;
	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_load_tape( pdrv_dev, &action, ctape_done,
			 LOAD_CMD_UNLOAD, CTAPE_SLEEP); 
	    break;

        case MTONLINE:
	case MTLOAD:
	    /* 
	     * Issue an unload tape
	     * Set not ready state so we can bring it online..
	     */
	    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
	    ts_spec->ts_state_flags |= CTAPE_NOT_READY_STATE;
	    ts_spec->ts_state_flags &= ~CTAPE_TRIED_ONLINE_STATE;
	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_load_tape( pdrv_dev, &action, ctape_done,
			 LOAD_CMD_LOAD, CTAPE_SLEEP); 
	    break;

        case MTREW:
	    /*
	     * Rewind 
	     */
	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_move_tape( pdrv_dev, &action, ctape_done,
			(I32)mtop->mt_op, 1, NULL , CTAPE_SLEEP, (U32)NULL); 


            break;

        case MTNOP:
            return(0);
        case MTCACHE:
        case MTNOCACHE:
            return(ENXIO);

        case MTFLUSH:
	    /*
	     * Done issuing space file mark count == 0.
	     */
	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_move_tape( pdrv_dev, &action, ctape_done, MTFSF,
				NULL, NULL, CTAPE_SLEEP, (U32)NULL); 

            break;

        case MTCSE:
            /*
             * Clear Serious Exception, used by tape utilities
             * to clean up after Nbuf I/O and end of media.
             */
	    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
	    ts_spec->ts_flags &= ~(CTAPE_TPMARK | CTAPE_EOM |CTAPE_HARDERR );
	    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
            return(0);
	    /*NOTREACHED*/
	    break;

        case MTCLX: case MTCLS:
            return(0);
	    /*NOTREACHED*/
	    break;

        case MTENAEOT:
	    /*
	     * Enable EOM detection
	     * Just clear the bit
	     */
	    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
            ts_spec->ts_state_flags &= ~CTAPE_DISEOT_STATE;
	    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
            return(0);
	    /*NOTREACHED*/
	    break;

        case MTDISEOT:
	    /*
	     * Disable EOM detection
	     * Clear the EOM bit
	     */
	    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
            ts_spec->ts_state_flags |= CTAPE_DISEOT_STATE;
	    ts_spec->ts_flags &= ~CTAPE_EOM;
	    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
            return(0);
	    /*NOTREACHED*/
	    break;

        case MTRETEN:
	    /*
	     * Issue a load command with retension.
	     */
	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_load_tape( pdrv_dev, &action, ctape_done,
			(LOAD_CMD_RET | LOAD_CMD_LOAD), CTAPE_SLEEP); 


            break;

        default:
            return (ENXIO);
	    /*NOTREACHED*/
	    break;
        }

	/* 
	 * If we got here have submitted an action commnad check 
	 * results.......
	 */

	if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    	    PRINTD(DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), DEV_LUN(cam_dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: mtop NULL CCB\n", 
		    DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), 
		    DEV_LUN(cam_dev), module));
	    return(action.act_ret_error); /* driver/resource problem */
	}

	/*
	 * At this point in time we are done with this ccb
	 * we release it back to the pools
	 */
        CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	/*
	 * Now release the i/o ccb
	 */

	ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );

	/*
	 * Check if command completed 
	 * without error...
	 */

	if(action.act_ccb_status == CAT_CMP) {
	    /* 
	     * For the offl load commands, if we have success
	     * try to bring it online.. do not set the tried online
	     * state, will allow it to happen again if it fails.
	     */
	    switch( mtop->mt_op )
	    {
	    case MTOFFL:
	    case MTUNLOAD:
	    case MTONLINE:
	    case MTLOAD:
		ctape_online( cam_dev, (int)NULL, NO_TIME);
		break;
	    }

	    return(CTAPE_SUCCESS);

	} /* end of if status == CAT_CMP */

	/*
	 * The mtop failed
	 */
	else {


	    /* 
	     * Since these are all tape movement for the most part....
	     * Check to see if the command actually failed could be 
	     * a eom detect of successful fsf.... ie we did
	     * everything asked of us....
	     */
	    if((action.act_fatal & ACT_FAILED) == NULL) {
		return(CTAPE_SUCCESS);
	    }

	    else {
		/* 
		 * Some other error
		 * return ERROR
 		 */
		return( action.act_ret_error );
	    }

	} /* End of if != SUCCESS */

	/*NOTREACHED*/
	break; /* Break for MTIOCTOP */

    /* 
     * Get tape status.......
     */
    case MTIOCGET:                /* tape status */
        mtget = (struct mtget *)data;
	/*
	 * Copy over flags...
	 */
        mtget->mt_dsreg = 0; 
        mtget->mt_erreg = 0;

	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
        mtget->mt_resid = (short)ts_spec->ts_resid; /* LAST resid 	*/
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

        mtget->mt_type = MT_ISSCSI;
	return(0);
	/*NOTREACHED*/
        break;

    case DEVIOCGET:             /* device status */
        devget = (struct devget *)data;
        bzero(devget,sizeof(struct devget));
        devget->category = DEV_TAPE;
        devget->bus = DEV_SCSI;
        bcopy(DEV_SCSI_GEN, devget->interface, strlen(DEV_SCSI_GEN));
        bcopy(dev_desc->dd_dev_name, devget->device, DEV_SIZE);

	if( minfo == (struct controller *)NULL) {
	    devget->adpt_num = 0; /* FAKE it out */
	}
	else {
            devget->adpt_num = minfo->slot;
	}
	if( dinfo == (struct device *)NULL) {
	    devget->ctlr_num = DEV_BUS_ID(dev);
            devget->rctlr_num = 0;
  	    devget->unit_num = DEV_UNIT(dev);
	}  else  {
	    devget->ctlr_num = dinfo->ctlr_num;
            devget->rctlr_num = dinfo->ctlr_hd->rctlr;
  	    devget->unit_num = dinfo->logunit;
	}
        devget->nexus_num = 0;
	devget->bus_num = DEV_BUS_ID(dev);
        devget->slave_num = DEV_UNIT(dev) ;
        bcopy("tz", devget->dev_name, 3);

	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
        devget->soft_count = pdrv_dev->pd_soft_err;
        devget->hard_count = pdrv_dev->pd_hard_err;

	/* We only want the lower byte */
        devget->stat = ts_spec->ts_flags & 0xFF;
        /* 
         * we only want the lower 4 bits at this time 
         * the rest is density which gets filled in later 
         */
        devget->category_stat = ((ts_spec->ts_flags >> 12 )& 0X0F);
	/*
	 * Check to see if a "DAS" is present
	 */
	if(( dev_desc->dd_device_type & SZ_LOADER) != NULL){
	    /*
 	     * "DAS" is present............
	     */
	     devget->category_stat |= DEV_LOADER;
	}
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

        /*
         * Do a mode sense to check for write locked drive.
         * First one can fail due to unit attention.
         */

	retries = 0;
	do {

	    /*
	     * Issue a mode sense command 
	     */
	    /*
	     * Clear out action struct
	     */
	    bzero(&action, sizeof(CTAPE_ACTION));

	    ctape_mode_sns( pdrv_dev, &action, ctape_done,SEQ_NO_PAGE,
				ALL_PCFM_CURRENT, CTAPE_SLEEP); 

	    if(action.act_ccb == (CCB_SCSIIO *)NULL) {
    		PRINTD(DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), 
			DEV_LUN(cam_dev), (CAMD_TAPE ), 
			("[%d/%d/%d] %s: devget NULL CCB\n", 
			DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), 
			DEV_LUN(cam_dev), module));

		/* Must return 0 for devget */
	        return(0); 
	    }

	    if(action.act_ccb_status == CAT_CMP ){

		/*
		 *  GOOD Status  fill in rest of devget struct 
		 */ 
		msdp = (SEQ_MODE_DATA6 *)action.act_ccb->cam_data_ptr; 
		
		if( msdp->sel_head.wp != NULL ){
		    /*
		     * Tape is write locked.  
		     */ 
		    devget->stat |= DEV_WRTLCK; 
	 	} 
		/*
		 * Setup the tape density.  
		 */ 
	 	if (msdp->sel_desc.density_code <= density_entrys) {
		    devget->category_stat |=
				density_table[msdp->sel_desc.density_code]
                                             .dens_code;
		} 
		/*
		 * Setup default density codes.  
		 */ 
		else if (msdp->sel_desc.density_code == CTAPE_DENS_DEFAULT) {

		    if( strcmp( "TZ05", dev_desc->dd_dev_name ) ==
		    						NULL) {
			devget->category_stat |= DEV_1600BPI; 
		    } 
		    else if( strcmp( "TZ07", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_6250BPI; 
		    } 
		    else if( strcmp( "TZ30", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_6666BPI; 
		    } 
		    else if( strcmp( "TK50", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_6666BPI; 
		    } 
		    else if( strcmp( "TZK10", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_16000_BPI; 
		    } 
		    else if( strcmp( "TLZ04", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_61000_BPI; 
		    } 
		    else if( strcmp( "TLZ06", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_61000_BPI; 
		    } 
		    else if( strcmp( "TZ857", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_42500_BPI; 
		    } 
		    else if( strcmp( "TZ85", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_42500_BPI; 
		    } 
		    else if( strcmp( "TZK08", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_54000_BPI; 
		    } 
		    else if( strcmp( "TKZ60", dev_desc->dd_dev_name ) ==
								NULL) {
			devget->category_stat |= DEV_38000BPI; 
		    } 
		    else if(dev_desc->dd_device_type & SZ_QIC_CLASS) {
			devget->category_stat |= DEV_10000_BPI; 
		    }

		    else if (dev_desc->dd_device_type & SZ_9TRK_CLASS) {
			devget->category_stat |= DEV_6250BPI; 
		    }
		    else if (dev_desc->dd_device_type & SZ_RDAT_CLASS) {
			devget->category_stat |= DEV_61000_BPI; 
		    }
		    else if (dev_desc->dd_device_type & SZ_8MM_CLASS) {
			devget->category_stat |= DEV_54000_BPI; 
		    }
		    else if (dev_desc->dd_device_type & SZ_3480_CLASS) {
			devget->category_stat |= DEV_38000BPI; 
		    }
		    else {
			devget->category_stat |= DEV_6666BPI;
		    }


		} 
		/* End 'else if (msdp->sel_desc.density_code ==
		 * CTAPE_DENS_DEFAULT)' 
		 */
		else {
		    /*
		     * This density not listed as of yet or
		     * vendor unique.. Default to DEV_6666BPI
		     */
		    devget->category_stat |= DEV_6666BPI;
		}

	    }


	    /*
	     * At this point in time we are done with this ccb 
	     * we release it back to the pools  and memory gotten.....
	     */
	    if( action.act_ccb != (CCB_SCSIIO *)NULL) {
	    	CHK_RELEASE_QUEUE(pdrv_dev, action.act_ccb);

	    	CTAPE_REL_MEM( action.act_ccb );

                ccmn_rel_ccb((CCB_HEADER *)action.act_ccb );
	    }

	    retries++;
        
	} while( (retries < 3) && (action.act_ccb_status != CAT_CMP));
	
	/*
	 * since this is a devget... we always return success
	 */
    	PRINTD(DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), DEV_LUN(cam_dev), 
		(CAMD_TAPE |CAMD_INOUT), 
		("[%d/%d/%d] %s: exit\n",
		DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), DEV_LUN(cam_dev),
		module));
	return(0);
	/*NOTREACHED*/
	break;


    default:
        return (ccmn_DoSpecialCmd(cam_dev, cmd, data, flag, 
			(CCB_SCSIIO *)NULL, (int)NULL));
	/*NOTREACHED*/
        break;
    }
    return (0);

} /* end of ctape_ioctl */


	    



    


/* ---------------------------------------------------------------------- */
/*
 * Done routines
 */


/* ---------------------------------------------------------------------- */
/* Function description.
 *
 *
 * Routine Name: ctape_done()
 *
 * Functional Description:
 *
 *	Entry point for all NON user I/O requests.
 *	If the ccb does not contain a struct bp pointer in the
 *	working set then issue a wake up on the address of the 
 *	ccb. 
 *
 *
 *
 * Call Syntax:
 *
 *	ctape_done( ccb )
 *
 *	CCB_SCSIIO *ccb;
 *
 *
 *
 *
 *
 * Returns :
 *	None
 */


void
ctape_done (ccb)
	CCB_SCSIIO	*ccb;	/* The completed ccb		*/
{

    /*
     * Local variable
     */
    PDRV_DEVICE		*pdrv_dev;	/* Our device struct 	*/
    PDRV_WS		*pw;		/* Our working set	*/


    int			s;		/* Saved IPL		*/
    static u_char	module[] = "ctape_done"; /* Module name	*/



    pdrv_dev = (PDRV_DEVICE *)((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_pdrv; 

    pw = (PDRV_WS *)ccb->cam_pdrv_ptr;

    if( pdrv_dev == NULL ){
	panic("ctape_done: NULL PDRV_DEVICE pointer");
	return;
    }
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));


    /* 
     * To prevent race conditions on smp machines....
     */
    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);


    /* 
     * Check to see if bp is filled in. It should not be for this done
     * routine.
     */
    if( (struct buf *)ccb->cam_req_map == NULL){

	/* 
	 * This is not an user io ccb 
	 */
	/*
	 * Check to see if the ccb status says the scsi bus
	 * is busy or if the scsi status says the device is busy.
	 * If either one is the case the just just retry up to the
	 * retry limit.....
	 */
	if( CAM_STATUS(ccb) ==  CAM_REQ_CMP) {
	    /*
    	     * SIGNAL that we have received it.....
    	     */
    	    ((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_flags |= PWS_CALLBACK; 
	    wakeup(ccb);
    	    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
			("[%d/%d/%d] %s: exit\n", 
			DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), module));
	    return;
	}
	    
		
	else if(( CAM_STATUS(ccb) == CAM_REQ_CMP_ERR) && 
		(((CCB_SCSIIO *)ccb)->cam_scsi_status == SCSI_STAT_BUSY ) &&
		(pw->pws_retry_cnt < CTAPE_RETRY_LIMIT)){
	    CLEAR_CCB(ccb);
	    /*
	     * Want ccb back to head of sim with the sim enabled
	     */
	    ccb->cam_ch.cam_flags |=  CAM_SIM_QHEAD; 

	    /* 
	     * Increment retry count...
	     */
	    pw->pws_retry_cnt++;

	    /*
	     * Now unlock schedule timeout for 1 second to 
	     * resend the ccb.
	     */
    	    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

	    timeout( ctape_retry, ccb, hz);

    	    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
			("[%d/%d/%d] %s: exit\n", 
			DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), module));
	    return;
	}

	else {
	    /*
    	     * SIGNAL that we have received it.....
    	     */
    	    ((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_flags |= PWS_CALLBACK; 
	    wakeup(ccb);
    	    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
			("[%d/%d/%d] %s: exit\n", 
			DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), module));
	    return;
	}


    }
    else {

	CAM_ERROR(module,"CCB cam_req_map NON NULL", CAM_SOFTWARE,
		(CCB_HEADER *)ccb, pdrv_dev->pd_dev, (u_char *)NULL);

    	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	/*
	 * Remove from active lists
	 */
    	ccmn_rem_ccb( pdrv_dev, ccb );
	/*
    	 * SIGNAL that we have received it.....
    	 */
    	((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_flags |= PWS_CALLBACK; 
	wakeup(ccb);
    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
		("[%d/%d/%d] %s: exit\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module));
	return;
    }

    
} /* end of ctape_done */



/* ---------------------------------------------------------------------- */
/* Function description.
 *
 *
 * Routine Name: ctape_orphan_done()
 *
 * Functional Description:
 *
 *	Entry point for all NON user I/O requests, that have been
 *	orphaned. The only one so far is rewind on close...
 * 	
 *	If the ccb does not contain a struct bp pointer in the
 *	working set then issue a wake up on the address of the 
 *	ccb. 
 *
 *
 *
 * Call Syntax:
 *
 *	ctape_done( ccb )
 *
 *	CCB_SCSIIO *ccb;
 *
 *
 *
 *
 *
 * Returns :
 *	None
 * TODO:
 *	ADD MORE CMDS That can be orphaned.. only rewind is done now.
 */


void
ctape_orphan_done (ccb)
	CCB_SCSIIO	*ccb;	/* The completed ccb		*/
{

    /*
     * Local variable
     */
    PDRV_DEVICE		*pdrv_dev;	/* Our device struct 	*/
    TAPE_SPECIFIC	*ts_spec;	/* Our tape specific struct */
    PDRV_WS		*pw;		/* Our working set pointer */
    U32		ccb_status;	/* The status of the ccb */
    U32		chkcond_error;	/* Check condition status */
    int			s;		/* Saved IPL		*/
    static u_char	module[] = "ctape_orphan_done"; /* Module name	*/


    pdrv_dev = (PDRV_DEVICE *)((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_pdrv; 
    pw = (PDRV_WS *)ccb->cam_pdrv_ptr;


    if( pdrv_dev == NULL ){
	panic("ctape_orphan_done: NULL PDRV_DEVICE pointer");
	return;
    }
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;


    /* 
     * To prevent race conditions on smp machines....
     */
    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);

    /* 
     * Check to see if bp is filled in. It should not be for this done
     * routine.
     */
    if( (struct buf *)ccb->cam_req_map == NULL){

	/* 
	 * This is not an user io ccb ... Check to make sure that the
	 * ORPHAN_CMD_STATE bit is set if not report it.....
	 *
	 */
	if((ts_spec->ts_state_flags & CTAPE_ORPHAN_CMD_STATE) == NULL){
	    CAM_ERROR(module,"CTAPE_ORPHAN_CMD_STATE bit NOT set", 
			CAM_SOFTWARE, (CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	}


        ccb_status = ccmn_ccb_status((CCB_HEADER *)ccb);
    
        switch( ccb_status ) {
    
        case CAT_CMP:
    
	    /* 
     	     *  GOOD Status just return;
      	     */
	    break;
    
         case CAT_CMP_ERR:
                    
	    /* 
 	     * Had some sort of scsi status other then good
 	     * must look at each one.
 	     */
          
	     /* now we find out why ... either a check
 	      * condition or reservation conflict....
 	      */
	     switch( ccb->cam_scsi_status)
	     {
	     default:
	     case SCSI_STAT_GOOD:
	     case SCSI_STAT_CONDITION_MET:
	     case SCSI_STAT_INTERMEDIATE:
	     case SCSI_STAT_INTER_COND_MET:
	     case SCSI_STAT_COMMAND_TERMINATED:
	     case SCSI_STAT_QUEUE_FULL:
	         /* 
 	          * For all the above something is
 	          * really messed up.. Since the commands
 	          * are single threaded and not running 
 	          * tagged commands.
 	          */
		 ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		 CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	    
	         break;

	     case SCSI_STAT_BUSY:
		/* 
		 * Can we retry???
		 */
		if( pw->pws_retry_cnt < CTAPE_RETRY_LIMIT){
	    	    CLEAR_CCB(ccb);
		    /* 
		     * Want ccb back to head of sim with the sim enabled
		     */
		    ccb->cam_ch.cam_flags |= CAM_SIM_QHEAD; 

		    /* 
		     * Increment retry count...
		     */
		    pw->pws_retry_cnt++;

		    /*
		     * Now unlock schedule timeout for 1
		     * second to handle ccb.
		     */
    	    	    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

		    timeout( ctape_retry, ccb, hz);

    	    	    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
			("[%d/%d/%d] %s: exit\n", 
			DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), module));
	    	    return;
		}

		else {
		    ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    CAM_ERROR(module,"scsi status busy retry count exceeded", 
			CAM_HARDERR,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;
		}
		/*NOTREACHED*/
		break;
    
	     case SCSI_STAT_RESERVATION_CONFLICT:
	         /* this unit reserved by another
 	          * initiator , this should not
 	          * happen
 	          */
		 ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
	         break;
    
	     case SCSI_STAT_CHECK_CONDITION:
    
	         /* call ctape_ccb_chkcond() 
 	          * to handle the check condition 
 	          */
	         chkcond_error = ctape_ccb_chkcond(ccb, pdrv_dev);        
                     /* 
                      * Now determine what to do.
                      */
                     switch ( chkcond_error ) {
                    
                     case CHK_RECOVER:
		 	CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
				(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
			break;
		     case CHK_INFORMATIONAL:
		 	CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
				(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
			break;

		     /* 
		      * Everything else flag position lost.
		      */
                     default:
		 	ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		 	CAM_ERROR(module,(u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		         break;
                     } /* end of switch for check condition */
    
                 break; /* end of scsi_status check condition */
    
                 } /* end of switch of scsi status */

	     break; /* End of CAM_CMP_ERR */
    
             case CAT_RESET:
             case CAT_BUSY:
		 CAM_ERROR(module,"Reset occuring", CAM_INFORMATIONAL,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		 ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
	         break;

             case CAT_SCSI_BUSY:
             case CAT_INPROG:
             case CAT_UNKNOWN:
             case CAT_CCB_ERR:
             case CAT_BAD_AUTO:
             case CAT_DEVICE_ERR:
             case CAT_NO_DEVICE:
             case CAT_ABORT:
		 ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;

		 CAM_ERROR(module,"Bad ccb status", CAM_SOFTWARE,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	         break;
             default:
                 /* 
                  * Error log this we should never get 
                  */
		 ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		 CAM_ERROR(module,"Unknown ccb status", CAM_SOFTWARE,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
                 break;
    
             } /* end switch on cam status */
    
	/*
	 * clear ORPHAN_CMD_STATE
	 */
	ts_spec->ts_state_flags &= ~CTAPE_ORPHAN_CMD_STATE;

        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

        /*
         * Remove from active lists
         */
        ccmn_rem_ccb( pdrv_dev, ccb );

	CHK_RELEASE_QUEUE(pdrv_dev, ccb);

	CTAPE_REL_MEM( ccb );

	ccmn_rel_ccb((CCB_HEADER *)ccb );
	wakeup(&ts_spec->ts_state_flags);
    }
    else {
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
        /*
         * Remove from active lists
         */
        ccmn_rem_ccb( pdrv_dev, ccb );

	CAM_ERROR(module,"CCB cam_req_map NON NULL", CAM_SOFTWARE,
		(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
		(u_char *)NULL);


	/* 
	 * Issue a wake up on the ccb and the addr. of the state
	 * flags  so we don't have a hanging process...we 
	 * really don't want to panic...... DO we error out the bp
	 * just incase....... 
	 */
	CHK_RELEASE_QUEUE(pdrv_dev, ccb);
    	/*
    	 * SIGNAL that we has received it.....
    	 */
    	((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_flags |= PWS_CALLBACK; 
	wakeup(ccb);
	wakeup(&ts_spec->ts_state_flags);
    }

    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));
    return;
    
} /* end of ctape_orphan_done */


/* ---------------------------------------------------------------------- */
/* Function description.
 *
 * Routine Name: ctape_iodone
 *
 * Functional description:
 *
 *	This routine is called by lower levels when a user I/O request
 *	has been acted on by the lower levels. There is no error 
 *	recovery attempted for tapes in this version of the tape driver. 
 *
 *	Due the targets buffered mode operation the target can return
 *	good status without transferring the data to media. Notifcation
 *	of media error occurs sometime later. Due to the complexity of
 *	recovery code it has been decided to not to support retries at 
 *	this time.
 *
 * Side Effects:
 *	Since the tape driver does not operate in tag queueing 
 *	mode and any errors returned in the cam status the que
 *	is FROZEN below. This guanantees that nothing will be 
 *	coming up from the lower levels completed.
 *
 *	Based on cam status the user buffer struct is modified to
 *	reflect either successfull completion of the I/O tansfer 
 *	or error status.
 *
 *	Flags are set in the tape specific structure to reflect 
 *	End of Media, Filemark, Short Record, state reset etc.
 *
 * Call Syntax
 *	ctape_iodone( ccb )
 *		CCB_SCSIIO * ccb;
 *
 *
 * Returns:
 *	None
 *
 */

void
ctape_iodone( ccb )

	CCB_SCSIIO *ccb;

{
    PDRV_DEVICE		*pdrv_dev;	/* our device structure		*/
    TAPE_SPECIFIC	*ts_spec;	/* pointer to specific struct	*/
    PDRV_WS		*pw;		/* Our working set pointer */
    struct buf		*bp;		/* our user I/O bp ptr		*/
    U32		ccb_status; 	/* result of ccb status		*/
    U32		chk_status;	/* result of chk condition status */
    int			s;		/* Saved IPL			*/
    dev_t		dev;		/* Major/minor pair		*/
    static u_char	module[] = "ctape_iodone"; /* Module name	*/
	

	
    /* 
     * Our device struct, and specific struct
     */
    pdrv_dev = (PDRV_DEVICE *)((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_pdrv;
	
    pw = (PDRV_WS *)ccb->cam_pdrv_ptr;

    ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;

    dev = pdrv_dev->pd_dev;

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
    /* 
     * We lock to prevent race conditions for nbuf I/O
     */
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s);


    /* 
     * Get our user bp 
     */
    bp = (struct buf *)ccb->cam_req_map;
    if( bp == (struct buf *)NULL) {
	/*
	 * We really should have a bp if we are in this routine....
	 */
	
	CAM_ERROR(module,"CBB cam_req_map is NULL", CAM_SOFTWARE,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	/* 
	 * Issue a wake up on this ccb
    	 * SIGNAL that we have received it.....
    	 */
    	((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_flags |= PWS_CALLBACK; 
	wakeup( ccb );
        PDRV_IPLSMP_UNLOCK( pdrv_dev, s);
	return;
    }

    /*
     * Update read/write counters
     */
    if( ccb->cam_cdb_io.cam_cdb_bytes[0] == SEQ_READ_OP){
	/*
	 * Cmd is a read...
	 */
	pdrv_dev->pd_read_count++;
	pdrv_dev->pd_read_bytes += (bp->b_bcount - ccb->cam_resid);
    }
    else { /* write */
	pdrv_dev->pd_write_count++;
	pdrv_dev->pd_write_bytes += (bp->b_bcount - ccb->cam_resid);
    }



    /* Get our complettion Status */

    ccb_status = ccmn_ccb_status( (CCB_HEADER *)ccb );

    /*
     * Save away residual counts
     */
    bp->b_resid = ccb->cam_resid;
    ts_spec->ts_resid = ccb->cam_resid;

    switch( ccb_status ) {

    case CAT_CMP:

	/* 
	 * The cam reside flag tells us
	 * how many bytes where not transferred 
	 * If non NULL The device is hosed.....I hope..
	 */
	if( ccb->cam_resid != NULL){
	    CAM_ERROR(module,"Status = CMP but resid not NULL", CAM_SOFTWARE,
		(CCB_HEADER *)ccb, dev,
		(u_char *)NULL);
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Status = CMP but resid not NULL\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	    CTAPE_BERROR(bp, bp->b_bcount, EIO);

	    /* 
	     * Check if async I/O aka nbuf. If so
	     * we must abort the queues 
	     */

	    CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);
	}

	break;

    case CAT_CMP_ERR:

	/*
	 * Had some sort of scsi status other then good 
	 * must look at each one.  
	 */

	/* now we find out why ... either a check
	 * condition or reservation conflict....  
	 */
	switch(ccb->cam_scsi_status) { 
	default:  
	case SCSI_STAT_GOOD:
	case SCSI_STAT_CONDITION_MET:  
	case SCSI_STAT_INTERMEDIATE:
	case SCSI_STAT_INTER_COND_MET:  
	case SCSI_STAT_COMMAND_TERMINATED:  
	case SCSI_STAT_QUEUE_FULL:
	    /*
	     * For all the above something is 
	     * really messed up..  Since the commands 
	     * are single threaded and not running 
	     * tagged commands.  
	     */ 
	    CTAPE_BERROR(bp, bp->b_bcount, EIO);


	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
			(CAMD_TAPE ), 
			("[%d/%d/%d] %s: default SCSI STATUS = 0x%x\n",
			DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),
			module, ccb->cam_scsi_status));

	    CAM_ERROR(module,"Unexpected SCSI status", CAM_HARDERR,
			(CCB_HEADER *)ccb, dev,
			(u_char *)NULL);

	    /* 
	     * Check if async I/O aka nbuf. If so
	     * we must abort the queues 
	     */

	    CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

	    break;

	
	case SCSI_STAT_BUSY:
	    /* 
	     * Can we retry???
	     */
	    if( pw->pws_retry_cnt < CTAPE_RETRY_LIMIT){
		CLEAR_CCB(ccb);
		/* 
		 * Want ccb back to head of sim with the sim enabled
		 */
		ccb->cam_ch.cam_flags |=  CAM_SIM_QHEAD; 

		pw->pws_retry_cnt++;

		/*
		 * Now unlock schedule timeout for 1
		 * second to handle ccb.
		 */
    	    	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

		timeout( ctape_retry, ccb, hz);

    		PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
			("[%d/%d/%d] %s: exit\n", 
			DEV_BUS_ID(pdrv_dev->pd_dev), 
			DEV_TARGET(pdrv_dev->pd_dev), 
			DEV_LUN(pdrv_dev->pd_dev), module));
		return;
	    }

	    else {
	    	CTAPE_BERROR(bp, bp->b_bcount, EIO);
    	    	PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: device BUSY STATUS\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));

	    	CAM_ERROR(module,"BUSY SCSI status", CAM_HARDERR,
			(CCB_HEADER *)ccb, dev,
			(u_char *)NULL);

	    	/* 
	    	 * Check if async I/O aka nbuf. If so
	    	 * we must abort the queues 
	    	 */

	    	CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

	    }
	    break;

	case SCSI_STAT_RESERVATION_CONFLICT:
	    /* this unit reserved by another
	     * initiator , this should not
	     * happen
	     */
	    CTAPE_BERROR(bp, bp->b_bcount, EBUSY);

	    CAM_ERROR(module,"Reservation Conflict SCSI status", CAM_HARDERR,
			(CCB_HEADER *)ccb, dev,
			(u_char *)NULL);
    	    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
		    (CAMD_TAPE ), 
		    ("[%d/%d/%d] %s: Reservation conflict.\n", 
		    DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), module));
	    /* 
	     * Check if async I/O aka nbuf. If so
	     * we must abort the queues 
	     */
	    CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

      	    break;

	case SCSI_STAT_CHECK_CONDITION:

	    /* call ctape_io_chkcond() 
	     * to handle the check condition 
	     * this handles the bp fields
	     */
	    chk_status = ctape_ccb_chkcond(ccb, pdrv_dev);	

	    /* 
	     * Now determine what to do.
	     */
	    switch ( chk_status ) {
	    /*
	     * Look at common conditions first.
	     * Please note that the ts_spec->ts_resid is handled 
	     * down in the check condition.....
	     */
	    case CHK_BLANK_CHK:
	    case CHK_EOM :
		/*
		 * This has been broke for a long time is the
		 * 4.X releases.... In those releases we did not
		 * take into consideration the fact that data
		 * could have been transfered on the read/write
		 * cases.... We now will take a look at the 
		 * cam_resid field to tell use if any data
		 * when across.
		 */
		if((ccb->cam_resid == NULL) || (bp->b_bcount != 
				ccb->cam_resid)){
		    /*
		     * Data has gone across ...
		     */

		    if((ts_spec->ts_state_flags & CTAPE_DISEOT_STATE ) == 
					NULL ){
			/*
			 * They have not disabled eot detect/notification
			 * we must abort all pending I/O at this point 
			 * But NOTICE WE DO NOT ERROR THIS BP BECAUSE
			 * DATA IS ON TAP........
			 */
	    		CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);
		    }
		

		}
		else {
		    /* 
		     * No data has gone......
		     * Must notify process.
		     */
		    
		    if((ts_spec->ts_state_flags & CTAPE_DISEOT_STATE ) == 
					NULL ){
			CTAPE_BERROR( bp, bp->b_bcount, ENOSPC);
	    		CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);
		    }
		}
		break;

	    case CHK_FILEMARK:
		/*
		 * Now for some strangness.. Due to the nature
		 * of fixed block operation it will read a file
		 * mark while reading data and we MUST take that
		 * into consideration.
		 */
		if((ccb->cam_resid != NULL) && (bp->b_bcount != 
				ccb->cam_resid)){
		    /*
		     * Data has gone across ... This should
		     * only be the case for fixed block tapes
		     */
		    if(ts_spec->ts_block_size != NULL ) {
			/* 
			 * FIXED blocks.... clear tpmark
			 * and set tpmark pending
			 */
			ts_spec->ts_flags &= ~CTAPE_TPMARK;
			ts_spec->ts_flags |= CTAPE_TPMARK_PENDING;

			/*
			 * Check to see if anything is on our
			 * queues if so set abort_tppend_state
			 * to tell strategy to fail all nbuf
			 * requests that come in.
			 */
			if( pdrv_dev->pd_active_list.flink !=
				 (PDRV_WS *)pdrv_dev) {
			    /*
			     * There is something on the queue
			     * Set the state flag for strategy and
			     * abort the queue.
			     * If there is a stuff on the queue
			     * tape mark pending is handled in this
			     * routine after the ccb's come back
			     * to this routine.. else if nothing on the
			     * queue tape mark pending is handled in the
			     * strategy routine.
			     */
			    ts_spec->ts_state_flags |= 
					CTAPE_ABORT_TPPEND_STATE;
	    		    CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);
			}
		    }
		    else {
			/* 
			 * DATA has been transferred but in variable 
			 * block mode... this should not happen
			 */
			CAM_ERROR(module,
			    "TPMARK detected, Transfer of data, variable blk",
			    CAM_HARDERR, (CCB_HEADER *)ccb, dev,
			    (u_char *)NULL);
				
    			PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
				(CAMD_TAPE ), 
				("[%d/%d/%d] %s: Tape mark, unit not fixed\n", 
				DEV_BUS_ID(dev), DEV_TARGET(dev), 
				DEV_LUN(dev), module));

			CTAPE_BERROR( bp ,bp->b_bcount, EIO);
	    		CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);
		    }
		}
		else {
		    /*
		     * Looks like a regular tape... just
		     * zero bytes to user
		     */
		    bp->b_resid = ccb->cam_resid;
		    /*
		     * Check to see if anything is on our
		     * queues 
		     */
	    	    CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);


		}
		break; /* End of if CHK_FILEMARK */

	    /* 
	     * Do nothing 
	     */
	    case CHK_ILI:
		break;
		
	    case CHK_RECOVER:

	    	CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)ccb, dev,
			(u_char *)NULL);
    		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			(CAMD_TAPE ), 
			("[%d/%d/%d] %s: ILI/SOFTERR cam_resid = 0x%X\n", 
			DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			module, ccb->cam_resid));
    		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			(CAMD_TAPE ), 
			("[%d/%d/%d] %s: sense info = 0x%x 0x%x 0x%x 0x%x\n", 
			DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			module,
			((ALL_REQ_SNS_DATA *)ccb->cam_sense_ptr)->info_byte3,
			((ALL_REQ_SNS_DATA *)ccb->cam_sense_ptr)->info_byte2,
			((ALL_REQ_SNS_DATA *)ccb->cam_sense_ptr)->info_byte1,
			((ALL_REQ_SNS_DATA *)ccb->cam_sense_ptr)->info_byte0));

		break;

    	    case CHK_INFORMATIONAL:

		/* 
		 * The cam resid field tells us
		 * how many bytes where not transferred 
		 * If non NULL The device is hosed.....I hope..
		 * We had an informational message make sure all the
		 * data went in/out of the dme
		 */
		if( ccb->cam_resid != NULL){
		    CAM_ERROR(module,
			"Checkcondition Informational but cam_resid NON NULL",
			CAM_HARDERR, ccb, dev,
			(u_char *)NULL);

    		    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			    (CAMD_TAPE ), 
			    ("[%d/%d/%d] %s: Informational but resid != 0\n", 
			    DEV_BUS_ID(dev), DEV_TARGET(dev), 
			    DEV_LUN(dev), module));
	    	    CTAPE_BERROR(bp, bp->b_bcount, EIO);

	    	    /* 
	    	     * Check if async I/O aka nbuf. If so
	    	     * we must abort the queues 
	    	     */
	    	    CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

		}
		else{
		    CAM_ERROR(module, (u_char *)NULL,
			CAM_INFORMATIONAL, ccb, dev,
			(u_char *)NULL);
			
		}


		break;
	
	    /* 
	     * The rest just mark error and abort que.
	     */
	    case CHK_CHK_NOSENSE:
	    case CHK_SENSE_NOT_VALID:
	    case CHK_NOSENSE_BITS:
	    case CHK_NOT_READY:
	    case CHK_MEDIUM_ERR:
	    case CHK_HARDWARE:
	    case CHK_ILL_REQ:
	    case CHK_VENDOR_SPEC:
	    case CHK_COPY_ABORT:
	    case CHK_EQUAL:
	    case CHK_VOL_OFLOW:
	    case CHK_MIS_CMP:
	    case CHK_UNIT_ATTEN:
	    case CHK_DATA_PROT:
	    case CHK_CMD_ABORTED:
	    case CHK_UNKNOWN_KEY:
	    default:
		
		CAM_ERROR(module,(u_char *)NULL, CAM_HARDERR,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    		PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
			(CAMD_TAPE ), 
			("[%d/%d/%d] %s: Fatal I/O error\n", 
			DEV_BUS_ID(dev), DEV_TARGET(dev), 
			DEV_LUN(dev), module));

		CTAPE_BERROR( bp, bp->b_bcount,EIO);
	    	
		CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

		break;
	    } /* end of switch for check condition */

	    break; /* end of scsi_status check condition */

	} /* end of switch of scsi status */

	break;


    case CAT_INPROG:
    case CAT_UNKNOWN:
    case CAT_CCB_ERR:
	/* errlog this 
	 * Lower level problem..
	 * Once the driver is up and running
	 * panic??????
	*/


	CAM_ERROR(module,"Unexpected CCB Status", CAM_HARDERR,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);

	CTAPE_BERROR( bp, bp->b_bcount, EIO);

	CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

	break;

    case CAT_RESET:
    case CAT_BUSY:
	
	/* 
	 * We should only get busy stat's 
	 * Don't have to abort they will all be returned to us.
	 * Don't errlog this Because we will fill up the the errlog
	 * with reset pending messages....
	 */
	if( CAM_STATUS(ccb) == CAM_BUSY) {
	    ts_spec->ts_state_flags |= CTAPE_RESET_PENDING_STATE;
	}

	CTAPE_BERROR( bp, bp->b_bcount, EIO);

	break;

    case CAT_SCSI_BUSY:
    case CAT_BAD_AUTO:
    case CAT_DEVICE_ERR:
	/* 
	 * Error log this 
	 */
	CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);

	CTAPE_BERROR( bp, bp->b_bcount, EIO);

	CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

	ts_spec->ts_resid = ccb->cam_resid;
	break;


    case CAT_NO_DEVICE:
	/* 
	 * Error log this 
	 */
	
	CAM_ERROR(module,"Lower level has lost device", CAM_HARDERR,
			(CCB_HEADER *)ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	
	CTAPE_BERROR(bp, bp->b_bcount, ENXIO);

	CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

	break;

    case CAT_ABORT:

	/* 
	 * Return is a result of us walking our
	 * active lists and aborting the ccb's
	*/

	/* 
	 * Check to see if we are expecting this abort....
	 */
	if((ts_spec->ts_state_flags & CTAPE_IO_ABORT_STATE) == NULL){
	    /*
	     * We were not expecting this ccb to has been aborted..
	     * The sim has done it for some reason.....
	     * Report the unexpected abort
	     * Error out bp...
	     */
	    CAM_ERROR(module,"Unexpected Aborted CCB (SIM has aborted)", 
		CAM_HARDERR,
		(CCB_HEADER *)ccb, dev,
		(u_char *)NULL);

	    CTAPE_BERROR(bp, bp->b_bcount, EIO);

	    /*
	     * If anything is on our queues abort them now....
	     */
	    CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

	    break;
	}

	/* 
	 * If we got here check to see how many on queue and
	 * if this is the last one then clear the flag.
	 */
	if( pdrv_dev->pd_active_ccb <= 1 ){
	    ts_spec->ts_state_flags &= ~CTAPE_IO_ABORT_STATE;
	}

	if( CAM_STATUS( ccb ) == CAM_REQ_ABORTED ){
	    /* 
	     * We can have two conditions either a eom has been
	     * reported. tpmark_pending or tpmark... we
	     * have to take each case separately.
	     */
	    if(( ts_spec->ts_flags & CTAPE_TPMARK_PENDING) != NULL){
		/*
		 * First one set tpmark and signal user
		 */
		ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;
		ts_spec->ts_flags |= CTAPE_TPMARK;
		ts_spec->ts_state_flags &= ~CTAPE_ABORT_TPPEND_STATE;
		bp->b_resid = bp->b_bcount;
	    }
	    else if(( ts_spec->ts_flags & CTAPE_TPMARK ) != NULL ){ 

		CTAPE_BERROR( bp, bp->b_bcount, EIO);
	    }
	    else if(( ts_spec->ts_flags & CTAPE_EOM ) != NULL ){ 

		CTAPE_BERROR( bp, bp->b_bcount, ENOSPC);
	    }
	    else { 

		CTAPE_BERROR( bp, bp->b_bcount, EIO);
	    }
	}
	else if( CAM_STATUS( ccb ) == CAM_UA_ABORT ){

	    /* 
	     * Error log this we should never get 
	     */
	    CAM_ERROR(module,"Unexpected CBB status", CAM_SOFTWARE,
		(CCB_HEADER *)ccb, dev,
		(u_char *)NULL);

	    CTAPE_BERROR( bp, bp->b_bcount, EIO);
	}

	else if( CAM_STATUS( ccb ) == CAM_UA_TERMIO ){
	    /* 
	     * Error log this we should never get 
	     */
	    CAM_ERROR(module,"Unexpected CBB status", CAM_SOFTWARE,
		(CCB_HEADER *)ccb, dev,
		(u_char *)NULL);

	    CTAPE_BERROR( bp, bp->b_bcount, EIO);

	}

	else if( CAM_STATUS( ccb ) == CAM_REQ_TERMIO ){
	    /* 
	     * Error log this we should never get 
	     */
	    CAM_ERROR(module,"Unexpected CBB status", CAM_SOFTWARE,
		(CCB_HEADER *)ccb, dev,
		(u_char *)NULL);

	    CTAPE_BERROR( bp, bp->b_bcount, EIO);

	
	}
	else {
	    /* 
	     * Error log this we should never get 
	     */
	    CAM_ERROR(module,"Unexpected CBB status", CAM_SOFTWARE,
		(CCB_HEADER *)ccb, dev,
		(u_char *)NULL);

	    CTAPE_BERROR( bp, bp->b_bcount, EIO);
	
	}
	break;


    default:
	/* 
	 * Error log this we should never get 
	 */
	CAM_ERROR(module,"Unknown CBB status", CAM_SOFTWARE,
		(CCB_HEADER *)ccb, dev,
		(u_char *)NULL);

	CTAPE_BERROR( bp, bp->b_bcount, EIO);
	/*
	 * Abort async queues
	 */
	CTAPE_RAWASYNC_ABORT( bp, pdrv_dev);

	
	break;

    } /* end switch on cam status */

    /*
     * Now unlock
     */
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s)

    /* at this time all flags are set we now
     * call iodone on this bp........
    */
    iodone( bp );

    /* 
     * Remove this ccb from the active list 
     */
    ccmn_rem_ccb( pdrv_dev, ccb );

    /* 
     * Release any data buffer we have
     * NOT IMPLEMENTED FOR IO
     * You can't release a user buffer Will cause a panic if you do....
     */

    /*
     * At this point in time we are done with this ccb
     * we release it back to the pools
     */
    CHK_RELEASE_QUEUE(pdrv_dev, ccb);

    /*
     * Release the ccb
     */
    ccmn_rel_ccb((CCB_HEADER *)ccb );

    PRINTD(DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev), 
	(CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n",
	DEV_BUS_ID(dev), DEV_TARGET(dev), DEV_LUN(dev),module));

    return;

} /* end of ctape_iodone */




/* ---------------------------------------------------------------------- */
/*
 * ASYNC notification routine.
 */

/* ---------------------------------------------------------------------- */
/* Function description.
 *	This routine is called when a AEN, BDR, or Bus reset has
 *	occurred. This routine sets CTAPE_RESET_STATE and clears
 *	CTAPE_RESET_PED_STATE for BDR's and Bus resets. For AEN's
 *	set CTAPE_UNIT_ATTEN_STATE.
 * 
 *
 * Call syntax
 *  ctape_async( opcode, path_id, target, lun, buf_ptr, data_cnt)
 *	u_I32		opcode;		 Reason why called	
 *	u_char		path_id;	 Bus number
 *	u_char		target;		 Target number
 *	u_char		lun;		 Logical unit number
 *	cadd_t		buf_ptr;	 Buffer address AEN's
 *	u_char		data_cnt;	 Number of bytes valid;
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	Setting and clearing of state flags
 *
 * Return values
 *	NONE
 *
 * TO DO:
 *	Recovery for unit.
 */

void
ctape_async( opcode, path_id, target, lun, buf_ptr, data_cnt)
	U32		opcode;		 /* Reason why called		*/
	u_char		path_id;	 /* Bus number			*/
	u_char		target;		 /* Target number		*/
	u_char		lun;		 /* Logical unit number		*/
	caddr_t		buf_ptr;	 /* Buffer address AEN's	*/
	u_char		data_cnt;	 /* Number of bytes valid;	*/

{

    /* 
     * Local Variables
     */

    PDRV_DEVICE		*pdrv_dev;	/* Pointer to device struct	*/

    TAPE_SPECIFIC	*ts_spec;	/* Pointer to our device
				 	 * specific struct
					 */

    dev_t		dev;		/* device number		*/
    int			s;		/* Saved ipl 			*/
    static u_char	module[] = "ctape_async"; /* Module name	*/

    PRINTD(path_id, target, lun, (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	path_id, target, lun, module));

    /* 
     * Get device number
     */
    dev = makedev(0, MAKEMINOR(MAKE_UNIT(path_id, target, lun), 0));


    pdrv_dev = GET_PDRV_PTR(dev);

    /*
     * If pdrv_device == NUll then the device has never been opened
     * and we should not be here
     */
    if( pdrv_dev == (PDRV_DEVICE *)NULL){
	CAM_ERROR(module,"No PDRV_DEVICE struct", CAM_SOFTWARE,
		(CCB_HEADER *)NULL, dev,
		(u_char *)NULL);
        PRINTD(path_id, target, lun, (CAMD_TAPE |CAMD_INOUT), 
	    ("[%d/%d/%d] %s: pdrv_dev == 0\n", 
	    path_id, target, lun, module));
	return;
    }

    /*
     * The same holds true for the specific struct
     */

    ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;

    if( ts_spec == (TAPE_SPECIFIC *)NULL){
	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
	CAM_ERROR(module,"No TAPE_SPECIFIC struct", CAM_SOFTWARE,
		(CCB_HEADER *)NULL, dev,
		(u_char *)NULL);
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	return;
    }

    /* 
     * Find out why we are here
     */
    if((opcode & AC_SENT_BDR ) != NULL){
	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
	ts_spec->ts_state_flags |= CTAPE_RESET_STATE;
	ts_spec->ts_state_flags &= ~CTAPE_RESET_PENDING_STATE;
	CAM_ERROR(module,"Device reset notification", CAM_HARDERR,
		(CCB_HEADER *)NULL, dev,
		(u_char *)NULL);
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    }
    if((opcode & AC_BUS_RESET ) != NULL){
	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
	ts_spec->ts_state_flags |= CTAPE_RESET_STATE;
	ts_spec->ts_state_flags &= ~CTAPE_RESET_PENDING_STATE;
	CAM_ERROR(module,"Bus reset notification", CAM_HARDERR,
		(CCB_HEADER *)NULL, dev,
		(u_char *)NULL);
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    }
    if((opcode & AC_SCSI_AEN ) != NULL){
	CTAPE_LOCK_OR_STATE(pdrv_dev, ts_spec, CTAPE_UNIT_ATTEN_STATE);
    }

    PRINTD(path_id, target, lun, (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	path_id, target, lun, module));
    return;

} /* End of ctape_async() */


/* ---------------------------------------------------------------------- */

/*
 * COMMAND SUPPORT ROUTINES
 */



/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * This routine will issue a TEST UNIT READY SCSI command
 * to the unit. 
 *
 * The varible sleep_action for this version will always be TRUE, which
 * directs the code to sleep waiting for comand status. If the
 * varible sleep_action is FALSE then we are in a recovery state (NEXT 
 * VERSION. With it being false then the code acts as a state machine
 * and steps  the machine
 *
 * Call syntax
 *  ctape_ready( pdrv_dev, action, done, sleep_action)
 *	PDRV_DEVICE	*pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 Pointer to the communication struct
 *	void		(*done)();	 Complettion routine
 *	U32		sleep_action;	 Whether we sleep
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	The various status into the callers action struct.
 *
 * Return values
 *	NONE	
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 *	
 */


void
ctape_ready( pdrv_dev, action, done, sleep_action)
	PDRV_DEVICE	*pdrv_dev;	/* Pointer to the device struct	*/
	CTAPE_ACTION	*action;	/* The caller action struct ptr	*/
 	void		(*done)();	/* Complettion routine		*/
	U32		sleep_action;	/* Whether we sleep		*/

{
    
    /*
     * LOCAL variables
     */
    
    int			s;		/* Saved IPL			*/
    int			s1;		/* Throw away IPL		*/
    U32		cam_flags;	/* Cam direction flags		*/
    u_char		sense_size;	/* reguest sense buffer size	*/
    static u_char	module[] = "ctape_ready"; /* Module name	*/
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));
    
    /* 
     * See if the user has set the request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    sense_size = GET_SENSE_SIZE( pdrv_dev );

    /*
     * Check to see if device can do synchronous transfers
     */ 
    cam_flags = CAM_DIR_NONE; 
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
    if (((((TAPE_SPECIFIC *)pdrv_dev->pd_specific)->ts_state_flags & 
		CTAPE_SYNC_STATE) == NULL) && 
    		((pdrv_dev->pd_dev_desc->dd_flags & SZ_NOSYNC) == NULL)){

	cam_flags |= CAM_INITIATE_SYNC; 
	((TAPE_SPECIFIC *)pdrv_dev->pd_specific)->ts_state_flags |= CTAPE_SYNC_STATE;
    }
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

    /*
     * Call the common routine to do the test unit ready for us
     * It will return an already being processed ccb
     */
    action->act_ccb = ccmn_tur(pdrv_dev, sense_size, cam_flags, done,
		(u_char)NULL, CTAPE_TIME_5);

   /* 
    * Check if ccb is NULL if so the macro fills out
    * the Error logs it and fills out action return values. 
    */
    if(action->act_ccb == (CCB_SCSIIO *)NULL){
    	CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	return;
    }
    
    /*
     * Check to see if we don't sleep
     * STEP AT THIS POINT NEXT VERSION
     */
    if( sleep_action != CTAPE_SLEEP){
    	return;
    }
    	
    
    /*
     * Remove from active lists
     */
    ccmn_rem_ccb( pdrv_dev, action->act_ccb );

    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    /* 
     * If we got here the command has been sent down and completed
     * Now check for status.
     */


    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
    switch( action->act_ccb_status ) {
    
    case CAT_CMP:
    
	/* 
         *  GOOD Status just return;
         */
        break;
    
    case CAT_CMP_ERR:
                    
	/* 
 	 * Had some sort of scsi status other then good
 	 * must look at each one.
	 */
            
	/* now we find out why ... either a check
	 * condition or reservation conflict....
	 */

	action->act_scsi_status = action->act_ccb->cam_scsi_status;
	switch(action->act_scsi_status)
            {
            default:
            case SCSI_STAT_GOOD:
            case SCSI_STAT_CONDITION_MET:
            case SCSI_STAT_BUSY:
            case SCSI_STAT_INTERMEDIATE:
            case SCSI_STAT_INTER_COND_MET:
            case SCSI_STAT_COMMAND_TERMINATED:
            case SCSI_STAT_QUEUE_FULL:
                /* 
                 * For all the above something is
                 * really messed up.. Since the commands
                 * are single threaded and not running 
                 * tagged commands.
                 */
		CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |=  ACT_FAILED;
	    	action->act_ret_error = EIO;
                break;
    
            case SCSI_STAT_RESERVATION_CONFLICT:
                /* this unit reserved by another
                 * initiator , this should not
                 * happen
                 */
		CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |=  ACT_FAILED;
		action->act_ret_error = EBUSY;
                break;
    
            case SCSI_STAT_CHECK_CONDITION:
    
                /* call ctape_ccb_chkcond() 
                 * to handle the check condition 
                 */
                action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, 
				pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) {

		case CHK_UNIT_ATTEN:
		case CHK_NOT_READY:
		    /*
		     * Unit attentions we just walk out...
		     * Not ready is not really an error
		     */
		    break;

		case CHK_INFORMATIONAL:
		    /* 
		     * Just informational message do nothing..
		     */
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_RECOVER:
		    CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                /*
                 * Look at error conditions.
                 */
                case CHK_EOM :
                case CHK_FILEMARK:
                case CHK_ILI:
                case CHK_CHK_NOSENSE:
                case CHK_SENSE_NOT_VALID:
                case CHK_NOSENSE_BITS:
                case CHK_HARDWARE:
		case CHK_MEDIUM_ERR:
		case CHK_ILL_REQ:
		case CHK_BLANK_CHK:
		case CHK_VENDOR_SPEC:
		case CHK_COPY_ABORT:
		case CHK_EQUAL:
		case CHK_VOL_OFLOW:
		case CHK_MIS_CMP:
                case CHK_DATA_PROT:
                case CHK_CMD_ABORTED:
                case CHK_UNKNOWN_KEY:
                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |=  ACT_FAILED;
		    action->act_ret_error = EIO;
                    
                } /* end of switch for check condition */
    
	    break; /* end of scsi_status check condition */
    
	    } /* end of switch of scsi status */

	break; /* End of CAM_CMP_ERR */
    
    case CAT_INPROG:
    case CAT_UNKNOWN:
    case CAT_CCB_ERR:
    case CAT_RESET:
    case CAT_BUSY:
    case CAT_SCSI_BUSY:
    case CAT_BAD_AUTO:
    case CAT_DEVICE_ERR:
    case CAT_NO_DEVICE:
    case CAT_ABORT:
	action->act_fatal |=  ACT_FAILED;
	action->act_ret_error = EIO;
        /* 
         * Error log this we should never get 
         */
	CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	break;
    default:
    /* 
     * Error log this we should never get 
     */
	action->act_fatal |=  ACT_FAILED;
	action->act_ret_error = EIO;
	CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
	break;
    
    } /* end switch on cam status */
    
    /*
     * Now unlock
     */
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));
    
    return;
    
} /* End of ctape_ready() */
    



/* ---------------------------------------------------------------------- */
/* Function description.
 * This routine runs down the modsel table for this device if
 * one is defined. 
 * The varible sleep_action for this version will always be TRUE, which
 * directs the code to sleep waiting for comand status. If the
 * varible sleep_action is FALSE then we are in a recovery state (NEXT 
 * VERSION. With it being false then the code acts as a state machine
 * and steps  the machine
 *
 * Call syntax
 *  ctape_open_sel( pdrv_dev, action,index, done, sleep_action)
 *	PDRV_DEVICE	pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 Pointer to callers action struct
 * 	I32		index;		 The index of the table
 *	void		(*done)();	 Complettion routine
 *	U32		sleep_action;	 Whether we sleep
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	Return values of status of command placed in the action struct.
 *
 * Return values
 *	NONE	
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */


void
ctape_open_sel( pdrv_dev, action, index, done, sleep_action)
	PDRV_DEVICE	*pdrv_dev;	 /* Pointer to the device struct*/	
	CTAPE_ACTION	*action;	 /* Pointer to callers action 
					  * Action struct
					  */
	void		(*done)();	 /* Complettion routine		*/
	U32		sleep_action;	 /* Whether we sleep		*/

{

    /* 
     * Local Variables
     */
    
    MODESEL_TBL		*mod_tbl = pdrv_dev->pd_dev_desc->dd_modesel_tbl;
    				/*
    				 * Pointer to mode select descriptor
    				 */
    
    
    int			s;		/* Saved IPL			*/
    int			s1;		/* Throw away IPL		*/
    u_char		sense_size;	/* Request sense buffer size	*/
    static u_char	module[] = "ctape_open_sel"; /* Module name	*/
    
    
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    /* 
     * Now lets do the mode select table for this
     * device. We we run down the list passing the index
     * of the page we want to set.
     */
    if(( index >= MAX_OPEN_SELS)||(mod_tbl->ms_entry[index].ms_data == NULL)){
    
    	/*
    	 * The caller screwed up
    	 */
	
    	PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
	CAM_ERROR(module,"User defined mode select table wrong", CAM_HARDERR,
			(CCB_HEADER *)NULL, pdrv_dev->pd_dev,
			(u_char *)NULL);
    	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE ), 
		("[%d/%d/%d] %s: Data pointer 0 or excede OPEN_SELS\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module));
	
	action->act_fatal |= (ACT_PARAMETER| ACT_FAILED);
	action->act_ret_error = EINVAL;
    	return;
    }

    /* 
     * See if the user has set there request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    sense_size = GET_SENSE_SIZE( pdrv_dev );

    action->act_ccb = ccmn_mode_select( pdrv_dev, sense_size, (U32)CAM_DIR_OUT,
		done, (u_char)NULL, CTAPE_TIME_5, index);
    
   /* 
    * Check if ccb is NULL if so the macro fills out
    * the Error logs it and fills out action return values. 
    */
    if(action->act_ccb == (CCB_SCSIIO *)NULL){
    	CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	return;
    }
    
    /* 
     * Check to see if we should sleep ...
     * ADD State step for error recovery
     */
    if( sleep_action == CTAPE_NOSLEEP ){
    	return;
    }
    
    
    /*
     * Remove from active lists
     */
    ccmn_rem_ccb( pdrv_dev, action->act_ccb );

    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
    switch( action->act_ccb_status ) {
    
    case CAT_CMP:
    
	/* 
         *  GOOD Status just return;
         */
        break;
    
    case CAT_CMP_ERR:
                    
	/* 
 	 * Had some sort of scsi status other then good
 	 * must look at each one.
	 */
            
	/* now we find out why ... either a check
	 * condition or reservation conflict....
	 */

	action->act_scsi_status = action->act_ccb->cam_scsi_status;
	switch(action->act_scsi_status)
            {
            default:
            case SCSI_STAT_GOOD:
            case SCSI_STAT_CONDITION_MET:
            case SCSI_STAT_BUSY:
            case SCSI_STAT_INTERMEDIATE:
            case SCSI_STAT_INTER_COND_MET:
            case SCSI_STAT_COMMAND_TERMINATED:
            case SCSI_STAT_QUEUE_FULL:
                /* 
                 * For all the above something is
                 * really messed up.. Since the commands
                 * are single threaded and not running 
                 * tagged commands.
                 */
		CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |=  ACT_FAILED;
	    	action->act_ret_error = EIO;
                break;
    
            case SCSI_STAT_RESERVATION_CONFLICT:
                /* this unit reserved by another
                 * initiator , this should not
                 * happen
                 */
		CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |=  ACT_FAILED;
		action->act_ret_error = EBUSY;
                break;
    
            case SCSI_STAT_CHECK_CONDITION:
    
                /* call ctape_ccb_chkcond() 
                 * to handle the check condition 
                 */
                action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, 
				pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) {
                /*
                 * Look at conditions.
                 */

		case CHK_INFORMATIONAL:
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_RECOVER:
		    CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_EOM :
                case CHK_FILEMARK:
                case CHK_ILI:
                case CHK_CHK_NOSENSE:
                case CHK_SENSE_NOT_VALID:
                case CHK_NOSENSE_BITS:
                case CHK_NOT_READY:
                case CHK_UNIT_ATTEN:
                case CHK_DATA_PROT:
		case CHK_MEDIUM_ERR:
		case CHK_HARDWARE:
		case CHK_ILL_REQ:
		case CHK_BLANK_CHK:
		case CHK_VENDOR_SPEC:
		case CHK_COPY_ABORT:
		case CHK_EQUAL:
		case CHK_VOL_OFLOW:
		case CHK_MIS_CMP:
                case CHK_CMD_ABORTED:
                case CHK_UNKNOWN_KEY:
                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |=  ACT_FAILED;
		    action->act_ret_error = EIO;
		    break;
                    
                } /* end of switch for check condition */
    
	    break; /* end of scsi_status check condition */
    
	    } /* end of switch of scsi status */

	break; /* End of CAM_CMP_ERR */
    
    case CAT_INPROG:
    case CAT_UNKNOWN:
    case CAT_CCB_ERR:
    case CAT_RESET:
    case CAT_BUSY:
    case CAT_SCSI_BUSY:
    case CAT_BAD_AUTO:
    case CAT_DEVICE_ERR:
    case CAT_NO_DEVICE:
    case CAT_ABORT:
    	action->act_fatal |=  ACT_FAILED;
	action->act_ret_error = EIO;
        /* 
         * Error log this we should never get 
         */
	CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	break;
    default:
    /* 
     * Error log this we should never get 
     */	
    	action->act_fatal |=  ACT_FAILED;
	action->act_ret_error = EIO;
	CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
	break;
    
    } /* end switch on cam status */
    
    /*
     * Now unlock
     */
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));
    
    return;

} /* End of ctape_open_sel() */
    






/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * This routine will issue a MODE SELECT SCSI command
 * to the unit,to set up the density. The varible sleep_action for this 
 * version will  always be TRUE, which directs the code to sleep 
 * waiting for  command status. If the varible sleep_action is FALSE then 
 * we are in a  recovery state (NEXT * VERSION. With it being false 
 * then the code acts as a state machine and steps the machine
 * We return a SCSIIO CCB or NULL if we can't get the resources
 *
 * Call syntax
 *  ctape_density_set( pdrv_dev, action, done, sleep_action, rd_wr_flag)
 *	PDRV_DEVICE	pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 Callers action struct pointer
 *	void		(*done)();	 Complettion routine
 *	U32		sleep_action;	 Whether we sleep
 *      U32             rd_wr_flag;	 Setting density for read or write
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 * 	The filling in of the action structure for the caller
 *	State flag of RESET_PENDING_STATE is set if one is detected.
 *	ctape_ccb_chkcond() will set various flags based on the check
 *	condition status
 *
 * Return values
 *	NONE
 *
 * TO DO:
 *	No sleep and state step
 *
 *	Interrupted sleeps
 */



void 
ctape_density_set( pdrv_dev, action, done, sleep_action, rd_wr_flag)
	PDRV_DEVICE	*pdrv_dev;	/* Pointer to the device struct	*/
	CTAPE_ACTION	*action;	/* Pointer to the callers action
					 * structure
					 */
	void		(*done)();	/* Complettion routine		*/
	U32		sleep_action;	/* Whether we sleep		*/
        U32             rd_wr_flag;	/* Setting density for read or write */
{
    
    
    /* 
     * Local Variables
     */
    
    DEV_DESC		*dev_desc = pdrv_dev->pd_dev_desc;
				/*
				 * Pointer to our device descriptor 
				 */
    TAPE_SPECIFIC 	*ts_spec; 	/* Ptr to our device specific struct */

    DENSITY_TBL		*dens;
				/*
				 * Pointer to density table for this device
				 */
    
    
    
    ALL_MODE_SEL_CDB6	*mod_cdb;	/* Mode sel cdb pointer		*/
    
    I32		idx;		/* Index into the density desc	*/

    u_char		*data_buf;	/* Pointer to data buffer	*/

    U32		data_buf_size; 	/* Size of data buffer	*/
    u_char		sense_size; 	/* Size of request sense buffer	*/

    int			s;		/* Saved IPL			*/
    int			s1;		/* Throw away  IPL		*/
    static u_char	module[] = "ctape_density_set"; /* Module name	*/
    
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

       
    /*
     * Get our device specific pointer
     */

    if( (ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific) ==
                (TAPE_SPECIFIC *)NULL){
        PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
        CAM_ERROR( module, "No tape specific struct", CAM_SOFTWARE,
                        (CCB_HEADER *)NULL, pdrv_dev->pd_dev, (u_char *)NULL);
        PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
        action->act_ret_error = EINVAL;
        return;
    }
     
    if ( rd_wr_flag == CTAPE_WRITE ) {
 
       /*
        * Get the index into the density table
        */
       idx = DEV_TO_DENS_IDX(pdrv_dev->pd_dev);
       
       /*
        * Set the pointer to our density struct
        */
       dens = dev_desc->dd_density_tbl;
       
       if( (dens == NULL) || ((dens->density[idx].den_flags &
       		DENS_VALID) == NULL)) {
       
          /* 
    	   * The density struct for this device null or the density
    	   * is not valid for it (the dev number)
    	   */

    	  PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
          CAM_ERROR(module, "Density structure wrong", CAM_HARDERR,
		(CCB_HEADER *)NULL, pdrv_dev->pd_dev,
		(u_char *)NULL);
    	  PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
    
	  action->act_fatal |= (ACT_PARAMETER | ACT_FAILED);
	  action->act_ret_error = EINVAL;
    	  return;
       }
    } /* End of rd_wr_flag == WRITE */


    /* 
     * Get our data_buffer for the modesel command since
     * we are only setting the density and block size
     * we will use the 6 byte mode select cdb . This
     * has a 4 byte parameter header and a 8 byte descriptor.
     */
    data_buf_size = sizeof(SEQ_MODE_HEAD6) + sizeof(SEQ_MODE_DESC);
  
    if(( data_buf = ccmn_get_dbuf(data_buf_size)) == (u_char *)NULL){

     	PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
        CAM_ERROR(module, "Can't get memory", CAM_SOFTWARE,
	       	  (CCB_HEADER *)NULL, pdrv_dev->pd_dev,
		  (u_char *)NULL);
        PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	
        action->act_fatal |= (ACT_RESOURCE | ACT_FAILED);
        action->act_ret_error = ENOMEM;
    
        return;
    }

    /* 
     * See if the user has set there request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    sense_size = GET_SENSE_SIZE( pdrv_dev );
    
    /*
     * Get an I/O ccb
     */
    action->act_ccb = ccmn_io_ccb_bld( pdrv_dev->pd_dev,data_buf, 
		data_buf_size, sense_size,
    		(U32)CAM_DIR_OUT, done,(u_char)NULL, CTAPE_TIME_5,
		(struct buf *)NULL); 
    
   /* 
    * Check if ccb is NULL if so the macro 
    * Error logs it and fills out action return values. 
    */
    if(action->act_ccb == (CCB_SCSIIO *)NULL){
    	CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	/*
	 * Must release data buffer gotten
	 */
	ccmn_rel_dbuf( data_buf, data_buf_size);
	return;
    }
    
    /*
     * Build our command mode select command 6 byte
     */

    mod_cdb = (ALL_MODE_SEL_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;
    mod_cdb->opcode = ALL_MODE_SEL6_OP;
    mod_cdb->lun = 0;
    if( ((ALL_INQ_DATA *)pdrv_dev->pd_dev_inq)->ansi == ALL_SCSI2 ) {
	mod_cdb->pf = 1; /* SCSI 2 devices need this bit */
    }
    mod_cdb->param_len = (char)data_buf_size;

    /*
     * Set cdb lenght
     */
    action->act_ccb->cam_cdb_len = sizeof(ALL_MODE_SEL_CDB6);

    
    /*
     * zero out the the data block
     */
    bzero( data_buf, data_buf_size);
    
    
    /*
     * set our length To null since mode select.......
     */
    ((SEQ_MODE_DATA6 *)data_buf)->sel_head.mode_len = (char)NULL;

    if (rd_wr_flag == CTAPE_WRITE ) {

       /*
        * Is there a speed setting
        */
       if((dens->density[idx].den_flags & DENS_SPEED_VALID) != NULL) {
      
            ((SEQ_MODE_DATA6 *)data_buf)->sel_head.speed = 
     		  dens->density[idx].den_speed_setting;
       }
    
       /*
        * Buffered mode
        */
       if((dens->density[idx].den_flags & DENS_BUF_VALID) != NULL) {
      
       	   ((SEQ_MODE_DATA6 *)data_buf)->sel_head.buf_mode = 
    		  dens->density[idx].den_buffered_setting;
       }
    } /* if rd_wr_flag == WRITE */ 
    
    /*
     * Set our block descriptor length
     */
    ((SEQ_MODE_DATA6 *)data_buf)->sel_head.blk_desc_len = 
   		  (char)sizeof(SEQ_MODE_DESC);
  
    /* 
     * now work on the block descriptor
     */
    
    if (rd_wr_flag == CTAPE_WRITE ) { 

       /* 
        * Set the density code up
        */
       ((SEQ_MODE_DATA6 *)data_buf)->sel_desc.density_code = 
       		dens->density[idx].den_density_code;
      
       /*
        * Set our blocking factor NULL means varible (9 trk etc.)
        */
       SEQBLK_SIZE_TO_DESC(dens->density[idx].den_blocking,
       		&((SEQ_MODE_DATA6 *)data_buf)->sel_desc);
 
    } /* if rd_wr_flag == WRITE */ 

    else {
       /*
        * Set the density code up 
        */
       
       ((SEQ_MODE_DATA6 *)data_buf)->sel_desc.density_code =
       ts_spec->ts_density;
 
       SEQBLK_SIZE_TO_DESC(ts_spec->ts_block_size,
                 &((SEQ_MODE_DATA6 *)data_buf)->sel_desc);
	/* 
	 * For reads this is a don't care but we have to 
	 * do it because of the python 4mm isn't SCSI 2
	 * complient. It barf's if we give it a zero...
	 */
       ((SEQ_MODE_DATA6 *)data_buf)->sel_head.buf_mode = 1;
    } 
 
    /* 
     * Now ready to send the density selection down
     */
    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);

    if( sleep_action != CTAPE_NOSLEEP) {

        ccmn_send_ccb_wait( pdrv_dev, (CCB_HEADER *)action->act_ccb, NOT_RETRY, 
		PRIBIO);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    }
    
    /*
     * The caller has said don't sleep they will handle the ccb
     */
    else {
        ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)action->act_ccb,NOT_RETRY);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	return;
    }
    

    /*
     * Remove from active lists
     */
    ccmn_rem_ccb( pdrv_dev, action->act_ccb );

    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
    switch( action->act_ccb_status ) {
    
    case CAT_CMP:
    
	/* 
         *  GOOD Status just return;
         */
        break;
    
    case CAT_CMP_ERR:
                    
	/* 
 	 * Had some sort of scsi status other then good
 	 * must look at each one.
	 */
            
	/* now we find out why ... either a check
	 * condition or reservation conflict....
	 */

	action->act_scsi_status = action->act_ccb->cam_scsi_status;
	switch(action->act_scsi_status)
            {
            default:
            case SCSI_STAT_GOOD:
            case SCSI_STAT_CONDITION_MET:
            case SCSI_STAT_BUSY:
            case SCSI_STAT_INTERMEDIATE:
            case SCSI_STAT_INTER_COND_MET:
            case SCSI_STAT_COMMAND_TERMINATED:
            case SCSI_STAT_QUEUE_FULL:
                /* 
                 * For all the above something is
                 * really messed up.. Since the commands
                 * are single threaded and not running 
                 * tagged commands.
                 */
		CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);

		action->act_fatal |= ACT_FAILED;
	    	action->act_ret_error = EIO;
                break;
    
            case SCSI_STAT_RESERVATION_CONFLICT:
                /* this unit reserved by another
                 * initiator , this should not
                 * happen
                 */
		CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |= ACT_FAILED;
		action->act_ret_error = EBUSY;
                break;
    
            case SCSI_STAT_CHECK_CONDITION:
    
                /* call ctape_ccb_chkcond() 
                 * to handle the check condition 
                 */
                action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, 
				pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) {
                /*
                 * Look at conditions
                 */

		case CHK_INFORMATIONAL:
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_RECOVER:
		    CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_EOM :
                case CHK_FILEMARK:
                case CHK_ILI:
                case CHK_CHK_NOSENSE:
                case CHK_SENSE_NOT_VALID:
                case CHK_NOSENSE_BITS:
                case CHK_NOT_READY:
                case CHK_UNIT_ATTEN:
                case CHK_DATA_PROT:
                case CHK_CMD_ABORTED:
		case CHK_MEDIUM_ERR:
		case CHK_HARDWARE:
		case CHK_ILL_REQ:
		case CHK_BLANK_CHK:
		case CHK_VENDOR_SPEC:
		case CHK_COPY_ABORT:
		case CHK_EQUAL:
		case CHK_VOL_OFLOW:
		case CHK_MIS_CMP:
                case CHK_UNKNOWN_KEY:
                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    break;
                    
                } /* end of switch for check condition */
    
	    break; /* end of scsi_status check condition */
    
	    } /* end of switch of scsi status */

	break; /* End of CAM_CMP_ERR */
    
    case CAT_INPROG:
    case CAT_UNKNOWN:
    case CAT_CCB_ERR:
    case CAT_RESET:
    case CAT_BUSY:
    case CAT_SCSI_BUSY:
    case CAT_BAD_AUTO:
    case CAT_DEVICE_ERR:
    case CAT_NO_DEVICE:
    case CAT_ABORT:
	action->act_fatal |= ACT_FAILED;
	action->act_ret_error = EIO;
        /* 
         * Error log this we should never get 
         */
	CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	break;
    default:
    /* 
     * Error log this we should never get 
     */
	action->act_fatal |= ACT_FAILED;
	action->act_ret_error = EIO;
	CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
	break;
    
    } /* end switch on cam status */
    
    /*
     * Now unlock
     */
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));
    
    return;

} /* end of ctape_density_set */



/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * This routine will issue a 2 MODE SENSES and a MODE SELECT SCSI command
 * to the unit,to set up the density and compression. The varible sleep_action 
 * for this version will  always be TRUE, which directs the code to sleep 
 * waiting for  command status. If the varible sleep is FALSE then 
 * we are in a  recovery state (NEXT * VERSION. With it being false 
 * then the code acts as a state machine and steps the machine
 * We return a SCSIIO CCB or NULL if we can't get the resources
 *
 * Call syntax
 *  ctape_compress_set( pdrv_dev, action, done, sleep_action)
 *	PDRV_DEVICE	pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 Callers action struct pointer
 *	void		(*done)();	 Complettion routine
 *	U32		sleep_action;	 Whether we sleep
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 * 	The filling in of the action structure for the caller
 *	State flag of RESET_PENDING_STATE is set if one is detected.
 *	ctape_ccb_chkcond() will set various flags based on the check
 *	condition status
 *
 * Return values
 *	NONE
 *
 * TO DO:
 *	No sleep and state step
 *
 *	Interrupted sleeps
 */



void 
ctape_compress_set( pdrv_dev, action, done, sleep_action)
	PDRV_DEVICE	*pdrv_dev;	/* Pointer to the device struct	*/
	CTAPE_ACTION	*action;	/* Pointer to the callers action
					 * structure
					 */
	void		(*done)();	/* Complettion routine		*/
	U32		sleep_action;	/* Whether we sleep		*/
{
    
    
    /* 
     * Local Variables
     */
    
    DEV_DESC		*dev_desc = pdrv_dev->pd_dev_desc;
				/*
				 * Pointer to our device descriptor 
				 */
    DENSITY_TBL		*dens;
				/*
				 * Pointer to density table for this device
				 */
    
    
    
    ALL_MODE_SEL_CDB6	*mod_cdb;	/* Mode sel cdb pointer		*/
    ALL_MODE_SENSE_CDB6	*sns_cdb;	/* Mode sense cdb pointer	*/
    
    I32		idx;		/* Index into the density desc	*/

    u_char		*data_buf;	/* Pointer to data buffer	*/

    U32		data_buf_size; 	/* Size of data buffer		*/

    CCB_SCSIIO		*ccbs[3];	/* saved ccbs			*/
    SEQ_DEV_CONF_PG	*pg;		/* page data pointer		*/
    U32		i, j;		/* Loop counters		*/
    u_char		sense_size; 	/* Size of request sense buffer	*/


    int			s;		/* Saved IPL			*/
    int			s1;		/* Throw away  IPL		*/
    static u_char	module[] = "ctape_compress_set"; /* Module name	*/
    
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    /*
     * Get the index into the density table
     */
    idx = DEV_TO_DENS_IDX(pdrv_dev->pd_dev);
    
    /*
     * Set the pointer to our density struct
     */
    dens = dev_desc->dd_density_tbl;
    /* 
     * See if the user has set there request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    sense_size = GET_SENSE_SIZE( pdrv_dev );
    
    /* 
     * Make sure our saved ccb's are zero before we start
     */
    for( i = 0; i < 3 ; i++){
	ccbs[i] = (CCB_SCSIIO *)NULL;
    }
	
    for( i = 0; i < 3 && (action->act_ret_error == NULL) ; i++){

	bzero( *action, sizeof(CTAPE_ACTION));

	/* 
         * Get our data_buffer for the mode sense/select command since
      	 * we are only setting the density and block size
         * we will use the 6 byte mode select cdb . This
         * has a 4 byte parameter header and a 8 byte descriptor.
	 * plus the size of page 10hex
         */
        data_buf_size = sizeof(SEQ_MODE_HEAD6) + sizeof(SEQ_MODE_DESC) +
			sizeof( SEQ_DEV_CONF_PG) ;
    
        if(( data_buf = ccmn_get_dbuf(data_buf_size)) == (u_char *)NULL){

    	    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
	    CAM_ERROR(module, "Can't get memory", CAM_SOFTWARE,
		(CCB_HEADER *)NULL, pdrv_dev->pd_dev,
		(u_char *)NULL);
    	    PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	
	    action->act_fatal |= (ACT_RESOURCE | ACT_FAILED);
	    action->act_ret_error = ENOMEM;

	    /* 
	     * Get out of for loop 
	     */
	    break;
	}

    
	/*
	 * Get an I/O ccb
	 */
	if( i < 2){ /* sense */
	    action->act_ccb = ccmn_io_ccb_bld( pdrv_dev->pd_dev,data_buf, 
		data_buf_size, sense_size,
    		(U32)CAM_DIR_IN, done,(u_char)NULL, CTAPE_TIME_5,
		(struct buf *)NULL); 
	}
	else { /* select */
           action->act_ccb = ccmn_io_ccb_bld( pdrv_dev->pd_dev,data_buf, 
		data_buf_size, sense_size,
    		(U32)CAM_DIR_OUT, done,(u_char)NULL, CTAPE_TIME_5,
		(struct buf *)NULL); 
	}
    
       /* 
        * Check if ccb is NULL if so the macro 
        * Error logs it and fills out action return values. 
        */
       if(action->act_ccb == (CCB_SCSIIO *)NULL){
	    CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	    /* 
	     * Release this data buffer
	     */
	    ccmn_rel_dbuf( data_buf, data_buf_size);

	    /*
	     * Get out of for loop 
	     */
	    break;
        }

	/*
	 * Save away this ccb
	 */
	ccbs[i] = action->act_ccb;
    
	/* 
	 * Are we doing the mode senses or a select
	 */
	if( i < 2 ){
	    /*
             * Build our mode sense command 6 byte
             */

       	    sns_cdb = (ALL_MODE_SENSE_CDB6 *)action->act_ccb->cam_cdb_io.
							cam_cdb_bytes;
       	    sns_cdb->opcode = ALL_MODE_SENSE6_OP;
       	    sns_cdb->lun = 0;
	    sns_cdb->page_code = SEQ_PGM_DEV_CONF; 
	    /*
	     * Now figure out if we are doing current or changeable
	     */
	    if( i == 0 ){
		sns_cdb->pc = ALL_PCFM_CURRENT;
	    }
	    else {
		sns_cdb->pc = ALL_PCFM_CHANGEABLE;
	    }

       	    sns_cdb->alloc_len = (char)data_buf_size;


	    /*
	     * Set cdb lenght 
	     */ 
	    action->act_ccb->cam_cdb_len = sizeof(ALL_MODE_SENSE_CDB6); 
	} 
	else {
	    /*
	     * doing the mode select 
	     */

	    /*
             * Build our mode select command 6 byte
	     * Donot issue the save pages bit....
             */

       	    mod_cdb = (ALL_MODE_SEL_CDB6 *)action->act_ccb->cam_cdb_io.
							cam_cdb_bytes;
       	    mod_cdb->opcode = ALL_MODE_SEL6_OP;
       	    mod_cdb->lun = 0;
    	    if( ((ALL_INQ_DATA *)pdrv_dev->pd_dev_inq)->ansi == ALL_SCSI2 ) {
		mod_cdb->pf = 1; /* SCSI 2 devices need this bit */
    	    }

       	    mod_cdb->param_len = (char)data_buf_size;

	    /*
	     * Set cdb lenght
	     */
       	    action->act_ccb->cam_cdb_len = sizeof(ALL_MODE_SEL_CDB6);

    
	    /*
	     * zero out the the data block
	     */
	    bzero( data_buf, data_buf_size);
    
    
	    /*
	     * set our length To null since mode select.......
	     */
	    ((SEQ_MODE_DATA6 *)data_buf)->sel_head.mode_len = (char)NULL;
	    /*
	     * Is there a speed setting
	     */
	    if((dens->density[idx].den_flags & DENS_SPEED_VALID) != NULL) {
    
		((SEQ_MODE_DATA6 *)data_buf)->sel_head.speed = 
				dens->density[idx].den_speed_setting;
	    }
    
	    /*
	     * Buffered mode
	     */
	    if((dens->density[idx].den_flags & DENS_BUF_VALID) != NULL) {
    
		((SEQ_MODE_DATA6 *)data_buf)->sel_head.buf_mode = 
				dens->density[idx].den_buffered_setting;
	    }
    
	    /*
	     * Set our block descriptor length
	     */
	    ((SEQ_MODE_DATA6 *)data_buf)->sel_head.blk_desc_len = 
					(char)sizeof(SEQ_MODE_DESC);
    
	    /* 
	     * now work on the block descriptor
	     */
	    /* 
	     * Set the density code up
	     */
	    ((SEQ_MODE_DATA6 *)data_buf)->sel_desc.density_code = 
					dens->density[idx].den_density_code;
    
	    /*
	     * Set our blocking factor NULL means varible (9 trk etc.)
	     */
	    SEQBLK_SIZE_TO_DESC(dens->density[idx].den_blocking,
				&((SEQ_MODE_DATA6 *)data_buf)->sel_desc);
    
	    /*
	     * Get the current setting over to the what we are
	     * going to send down.
	     */
	    for( j = 0; j < sizeof( SEQ_DEV_CONF_PG); j++ ) {
		((SEQ_MODE_DATA6 *)data_buf)->page_data[j] = 
		    ((SEQ_MODE_DATA6 *)ccbs[0]->cam_data_ptr)->page_data[j]; 
	    }

	    /*
	     * Now look at the page data.
	     */

	     pg = (SEQ_DEV_CONF_PG *)((u_char *)data_buf +  
			sizeof(SEQ_MODE_HEAD6) + 
			sizeof(SEQ_MODE_DESC));
	    /* 
	     * We really should check to see if compr_algo 
	     * is changable
	     */

	     /*
	      * Compression code
	      */
	    pg->compr_algo = dens->density[idx].den_compress_code;

	    /*
	     * Clear the ps bit 
	     */
	    pg->pg_head.ps = 0;


	} /* end of mode select */

	/* 
	 * Now ready to send the density selection down
	 */
    	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);

    	if( sleep_action != CTAPE_NOSLEEP) {

            ccmn_send_ccb_wait( pdrv_dev, (CCB_HEADER *)action->act_ccb, 
			NOT_RETRY, PRIBIO);
            PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	}
    
        /*
    	 * The caller has said don't sleep they will handle the ccb
    	 */
    	else {
	    ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)action->act_ccb,NOT_RETRY);
            PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	    return;
	}
    
    	/*
    	 * Remove from active lists
    	 */
    	ccmn_rem_ccb( pdrv_dev, action->act_ccb );

	PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);

        action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
        switch( action->act_ccb_status ) {
    
        case CAT_CMP:
    
	    /* 
             *  GOOD Status
	     */
           break;
    
       case CAT_CMP_ERR:
                    
	   /* 
 	    * Had some sort of scsi status other then good
 	    * must look at each one.
	    */
            
	   /* now we find out why ... either a check
	    * condition or reservation conflict....
	    */

	   action->act_scsi_status = action->act_ccb->cam_scsi_status;
	   switch(action->act_scsi_status)
            {
            default:
            case SCSI_STAT_GOOD:
            case SCSI_STAT_CONDITION_MET:
            case SCSI_STAT_BUSY:
            case SCSI_STAT_INTERMEDIATE:
            case SCSI_STAT_INTER_COND_MET:
            case SCSI_STAT_COMMAND_TERMINATED:
            case SCSI_STAT_QUEUE_FULL:
                /* 
                 * For all the above something is
                 * really messed up.. Since the commands
                 * are single threaded and not running 
                 * tagged commands.
                 */
		CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);

		action->act_fatal |= ACT_FAILED;
	    	action->act_ret_error = EIO;
                break;
    
            case SCSI_STAT_RESERVATION_CONFLICT:
                /* 
		 * This unit reserved by another
                 * initiator , this should not
                 * happen
                 */
		CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |= ACT_FAILED;
		action->act_ret_error = EBUSY;
                break;
    
            case SCSI_STAT_CHECK_CONDITION:
    
                /* call ctape_ccb_chkcond() 
                 * to handle the check condition 
                 */
                action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, 
				pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) 
		{
                /*
                 * Look at conditions
                 */

		case CHK_INFORMATIONAL:
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_RECOVER:
		    CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_EOM :
                case CHK_FILEMARK:
                case CHK_ILI:
                case CHK_CHK_NOSENSE:
                case CHK_SENSE_NOT_VALID:
                case CHK_NOSENSE_BITS:
                case CHK_NOT_READY:
                case CHK_UNIT_ATTEN:
                case CHK_DATA_PROT:
                case CHK_CMD_ABORTED:
		case CHK_MEDIUM_ERR:
		case CHK_HARDWARE:
		case CHK_ILL_REQ:
		case CHK_BLANK_CHK:
		case CHK_VENDOR_SPEC:
		case CHK_COPY_ABORT:
		case CHK_EQUAL:
		case CHK_VOL_OFLOW:
		case CHK_MIS_CMP:
                case CHK_UNKNOWN_KEY:
                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    break;
                    
                } /* end of switch for check condition */
    
	        break; /* end of scsi_status check condition */
    
	    } /* end of switch of scsi status */

	    break; /* End of CAM_CMP_ERR */
    
        case CAT_INPROG:
        case CAT_UNKNOWN:
        case CAT_CCB_ERR:
        case CAT_RESET:
        case CAT_BUSY:
        case CAT_SCSI_BUSY:
        case CAT_BAD_AUTO:
        case CAT_DEVICE_ERR:
        case CAT_NO_DEVICE:
        case CAT_ABORT:
	    action->act_fatal |= ACT_FAILED;
	    action->act_ret_error = EIO;
            /* 
             * Error log this we should never get 
             */
	    CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	    break;
        default:
            /* 
             * Error log this we should never get 
             */
	    action->act_fatal |= ACT_FAILED;
	    action->act_ret_error = EIO;
	    CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
	    break;
    
        } /* end switch on cam status */
    
        /*
         * Now unlock
         */
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    
    
    } /* End of for loop */

    /* 
     * We always return the last ccb whether it had and error or not.
     * But in this case since we are dealing with multiple ccb's
     * we free the previous ones but no the last.....
     */
    if( i > 0) { /* Have more then one. */
	for( j = 0; (j < (i - 1)) && (ccbs[j] != (CCB_SCSIIO *)NULL); j++){ 
	    ccmn_rel_dbuf( ccbs[j]->cam_data_ptr, ccbs[j]->cam_dxfer_len);
	    ccmn_rel_ccb((CCB_HEADER *)ccbs[j]);
	}
    }
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	    DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	    ("[%d/%d/%d] %s: exit\n", 
	    DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	    DEV_LUN(pdrv_dev->pd_dev), module));

    return;
	

} /* end of ctape_compress_set */





/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * This routine will issue a MODE Sense SCSI command
 * to the unit. We return to caller a CAM_SCSIIO ccb and it is
 * up to caller to determine status of the command.
 * The varible sleep_action for this version will always be TRUE, which
 * directs the code to sleep waiting for comand status. If the
 * varible sleep_action is FALSE then we are in a recovery state (NEXT 
 * VERSION. With it being false then the code acts as a state machine
 * and steps  the machine
 *
 * Call syntax
 *  ctape_mode_sns( pdrv_dev, action, done, page_code, page_cntl, sleep_action)
 *	PDRV_DEVICE	*pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 Pointer to callers action struct
 *	void		(*done)();	 Complettion routine
 *	u_char		page_code;	 The page we want
 *	u_char		page_cntl;	 The page control field
 *	U32		sleep_action;	 Whether we sleep
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	NONE
 *
 * Return values
 * 	NULL could not do what was requested
 *	CAM_SCSIIO ccb pointer 
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */


void
ctape_mode_sns( pdrv_dev, action, done, page_code, page_cntl, sleep_action)
	PDRV_DEVICE	*pdrv_dev;	/* Pointer to the device struct	*/	
	CTAPE_ACTION	*action;	/* Pointer to callers action struct */
	void		(*done)();	/* Complettion routine		*/
	u_char		page_code;	/* The page we want		*/
	u_char		page_cntl;	/* The page control field	*/
	U32		sleep_action;	/* Whether we sleep		*/

{


    /* 
     * Local Variables
     */

    ALL_MODE_SENSE_CDB6	*mod_cdb;	/* Mode sense cdb pointer	*/
    u_char	    *data_buf;		/* Our data buffer		*/
    U32	    data_buf_size; 	/* The size of the data buffer	*/
    int		    s;			/* Saved IPL			*/
    int		    s1;			/* Throw away IPL		*/
    DEV_DESC	         *dev_desc = pdrv_dev->pd_dev_desc;
    u_char	    sense_size; 	/* Size of request sense buffer	*/
    static u_char	module[] = "ctape_mode_sns"; /* Module name	*/


    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));
    /* 
     * Get our data_buffer size for the mode sense command since
     * we will use the 6 byte mode select cdb . This
     * has a 4 byte parameter header and a 8 byte descriptor.
     */
    data_buf_size = sizeof(SEQ_MODE_HEAD6) + sizeof(SEQ_MODE_DESC);
    /*
     * TZ30 hack......The TZ30 tries to send 14 bytes if we
     * ask for 12... So set the transfer to 14.....
     */
    if( strcmp( DEV_TZ30, dev_desc->dd_dev_name ) == NULL){
	data_buf_size += 2;
    }

     /* 
      * Now get our page size 
      */
    switch( page_code ) {

    case SEQ_NO_PAGE:
	/* 
	 * This is no page we only want the mode head and
	 * mode descriptor.
	 */
	break;
    /* 
     * Case on generic pages first.
     */

    case ALL_PGM_DISCO_RECO:
	data_buf_size += sizeof( ALL_DISC_RECO_PG);
	break;

    case ALL_PGM_PERIPH_DEVICE:
	data_buf_size += sizeof( ALL_PERIPH_DEV_PG);
	break;

    case ALL_PGM_CONTROL_MODE:
	data_buf_size += sizeof( ALL_CONTROL_PG);
	break;

    /*
     * The sequenial pages (tapes).
     */
    case SEQ_PGM_ERR_RECOV:
	data_buf_size += sizeof( SEQ_ERR_RECOV_PG);
	break;

    case SEQ_PGM_DEV_CONF:
	data_buf_size += sizeof( SEQ_DEV_CONF_PG);
	break;

    case SEQ_PGM_PART1:
	data_buf_size += sizeof( SEQ_PART1_PG);
	break;

    case SEQ_PGM_PART2:
	data_buf_size += sizeof( SEQ_PART1_PG);
	break;

    case SEQ_PGM_PART3:
	data_buf_size += sizeof( SEQ_PART1_PG);
	break;

    case SEQ_PGM_PART4:
	data_buf_size += sizeof( SEQ_PART1_PG);
	break;

    default:
	/* 
	 * Invalid PAGE code.
	 */
    	PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
	CAM_ERROR(module, "Invalid page", CAM_HARDERR,
		(CCB_HEADER *)NULL, pdrv_dev->pd_dev,
		(u_char *)NULL);
    	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );
	
	action->act_fatal |= (ACT_PARAMETER | ACT_FAILED);
	action->act_ret_error = EINVAL;
	return;
	/*NOTREACHED*/
	break;

    } /* end switch */

    if(( data_buf = ccmn_get_dbuf(data_buf_size)) == (u_char *)NULL){
	/* 
	 *  Log the error
	 */
    	PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
	CAM_ERROR(module, "Can't get memory", CAM_HARDERR,
		(CCB_HEADER *)NULL, pdrv_dev->pd_dev,
		(u_char *)NULL);
    	PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

	
	action->act_fatal |= ( ACT_RESOURCE | ACT_FAILED);
	action->act_ret_error = ENOMEM;
	return;
    }
    /* 
     * See if the user has set there request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    sense_size = GET_SENSE_SIZE( pdrv_dev );

    /*
     * Get an I/O ccb
     */
    action->act_ccb = ccmn_io_ccb_bld( pdrv_dev->pd_dev,data_buf, 
		data_buf_size, sense_size,
		(U32)CAM_DIR_IN, done,(u_char)NULL, (CTAPE_TIME_5 + 15) ,
		(struct buf *)NULL); 

   /* 
    * Check if ccb is NULL if so the macro 
    * Error logs it and fills out action return values. 
    */
    if(action->act_ccb == (CCB_SCSIIO *)NULL){
    	CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	/*
	 * Must release data buffer gotten
	 */
	ccmn_rel_dbuf( data_buf, data_buf_size);
	return;
    }

    /*
     * Build our command mode select command 6 byte
     */
    mod_cdb = (ALL_MODE_SENSE_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;
    mod_cdb->opcode = ALL_MODE_SENSE6_OP;
    mod_cdb->lun = 0;
    mod_cdb->page_code = page_code;
    mod_cdb->pc = page_cntl;
    mod_cdb->alloc_len = data_buf_size;
    
    /* 
     * set our cdb lenght
     */
    action->act_ccb->cam_cdb_len = sizeof(ALL_MODE_SENSE_CDB6);


    /* 
     * Now ready to send the mode sense command down 
     */
    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
    if( sleep_action != CTAPE_NOSLEEP) {

        ccmn_send_ccb_wait( pdrv_dev, (CCB_HEADER *)action->act_ccb, 
			NOT_RETRY, PRIBIO);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    }
    
    /*
     * The caller has said don't sleep they will handle the ccb
     */
    else {
	ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)action->act_ccb,NOT_RETRY);
	PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	return;
    }

    /*
     * Remove from active lists
     */
    ccmn_rem_ccb( pdrv_dev, action->act_ccb );

    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
    switch( action->act_ccb_status ) {
    
    case CAT_CMP:
    
	/* 
         *  GOOD Status just return;
         */
        break;
    
    case CAT_CMP_ERR:
                    
	/* 
 	 * Had some sort of scsi status other then good
 	 * must look at each one.
	 */
            
	/* now we find out why ... either a check
	 * condition or reservation conflict....
	 */

	action->act_scsi_status = action->act_ccb->cam_scsi_status;
	switch(action->act_scsi_status)
            {
            default:
            case SCSI_STAT_GOOD:
            case SCSI_STAT_CONDITION_MET:
            case SCSI_STAT_BUSY:
            case SCSI_STAT_INTERMEDIATE:
            case SCSI_STAT_INTER_COND_MET:
            case SCSI_STAT_COMMAND_TERMINATED:
            case SCSI_STAT_QUEUE_FULL:
                /* 
                 * For all the above something is
                 * really messed up.. Since the commands
                 * are single threaded and not running 
                 * tagged commands.
                 */
		CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |=  ACT_FAILED;
	    	action->act_ret_error = EIO;
                break;
    
            case SCSI_STAT_RESERVATION_CONFLICT:
                /* this unit reserved by another
                 * initiator , this should not
                 * happen
                 */
		CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |=  ACT_FAILED;
		action->act_ret_error = EBUSY;
                break;
    
            case SCSI_STAT_CHECK_CONDITION:
    
                /* call ctape_ccb_chkcond() 
                 * to handle the check condition 
                 */
                action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, 
				pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) {
                /*
                 * Look at conditions.
                 */

		case CHK_INFORMATIONAL:
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;
                case CHK_RECOVER:
		    CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_NOT_READY:
                case CHK_UNIT_ATTEN:
		    action->act_fatal |=  ACT_FAILED;
		    action->act_ret_error = EIO;
		    /* 
		     * Not really an error for devget ioctl's
		     */
		    break;

                case CHK_EOM:
                case CHK_FILEMARK:
                case CHK_ILI:
                case CHK_CHK_NOSENSE:
                case CHK_SENSE_NOT_VALID:
                case CHK_NOSENSE_BITS:
                case CHK_DATA_PROT:
                case CHK_CMD_ABORTED:
		case CHK_MEDIUM_ERR:
		case CHK_HARDWARE:
		case CHK_ILL_REQ:
		case CHK_BLANK_CHK:
		case CHK_VENDOR_SPEC:
		case CHK_COPY_ABORT:
		case CHK_EQUAL:
		case CHK_VOL_OFLOW:
		case CHK_MIS_CMP:
                case CHK_UNKNOWN_KEY:
                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |=  ACT_FAILED;
		    action->act_ret_error = EIO;
		    break;
                    
                } /* end of switch for check condition */
    
	    break; /* end of scsi_status check condition */
    
	    } /* end of switch of scsi status */

	break; /* End of CAM_CMP_ERR */
    
    case CAT_INPROG:
    case CAT_UNKNOWN:
    case CAT_CCB_ERR:
    case CAT_RESET:
    case CAT_BUSY:
    case CAT_SCSI_BUSY:
    case CAT_BAD_AUTO:
    case CAT_DEVICE_ERR:
    case CAT_NO_DEVICE:
    case CAT_ABORT:
	action->act_fatal |=  ACT_FAILED;
	action->act_ret_error = EIO;
        /* 
         * Error log this we should never get 
         */
	CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	break;
    default:
        /* 
         * Error log this we should never get 
         */
	action->act_fatal |=  ACT_FAILED;
	action->act_ret_error = EIO;
	CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
	break;
    
    } /* end switch on cam status */
    
    /*
     * Now unlock
     */
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    return;

}
    


/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * This routine will issue a MODE select SCSI command
 * to the unit. We return to caller a CAM_SCSIIO ccb and it is
 * up to caller to determine status of the command.
 * The varible sleep_action for this version will always be TRUE, which
 * directs the code to sleep waiting for comand status. If the
 * varible sleep_action is FALSE then we are in a recovery state (NEXT 
 * VERSION. With it being false then the code acts as a state machine
 * and steps  the machine
 *
 * Call syntax
 *  ctape_mode_sel( pdrv_dev, action, done, data_buf, length, sleep_action)
 *	PDRV_DEVICE	*pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 Pointer to callers action struct
 *	u_char 		*data_buf;	 The page we want to send	
 *	U32		length;		 Size of data buffer
 *	void		(*done)();	 Complettion routine
 *	U32		sleep_action;	 Whether we sleep
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	NONE
 *
 * Return values
 * 	NULL could not do what was requested
 *	CAM_SCSIIO ccb pointer 
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */


void
ctape_mode_sel( pdrv_dev, action, done, data_buf, length, sleep_action)
	PDRV_DEVICE	*pdrv_dev;	/* Pointer to the device struct	*/	
	CTAPE_ACTION	*action;	/* Pointer to callers action struct */
	void		(*done)();	/* Complettion routine		*/
	u_char 		*data_buf;	/* The page we want to send	*/
	U32		length;		/* Size of data buffer		*/
	U32		sleep_action;	/* Whether we sleep		*/

{


    /* 
     * Local Variables
     */

    ALL_MODE_SEL_CDB6	*mod_cdb;	/* Mode sense cdb pointer	*/
    int		    s;			/* Saved IPL			*/
    int		    s1;			/* Throw away IPL		*/
    u_char	    sense_size; 	/* Size of request sense buffer	*/
    static u_char	module[] = "ctape_mode_sel"; /* Module name	*/


    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    /* 
     * See if the user has set there request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    sense_size = GET_SENSE_SIZE( pdrv_dev );

    /*
     * Get an I/O ccb
     */
    action->act_ccb = ccmn_io_ccb_bld( pdrv_dev->pd_dev,data_buf, 
		length, sense_size,
		(U32)CAM_DIR_OUT, done,(u_char)NULL, CTAPE_TIME_5,
		(struct buf *)NULL); 

   /* 
    * Check if ccb is NULL if so the macro 
    * Error logs it and fills out action return values. 
    */
    if(action->act_ccb == (CCB_SCSIIO *)NULL){
    	CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	return;
    }

    /*
     * Build our command mode select command 6 byte
     */
    mod_cdb = (ALL_MODE_SEL_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;
    mod_cdb->opcode = ALL_MODE_SEL6_OP;
    mod_cdb->lun = 0;
    mod_cdb->param_len = length;

    /* Change need define */
    if( ((ALL_INQ_DATA *)pdrv_dev->pd_dev_inq)->ansi == 2 ) {
	mod_cdb->pf = 1; /* SCSI 2 devices need this bit */
    }
    
    /* 
     * set our cdb lenght
     */
    action->act_ccb->cam_cdb_len = sizeof(ALL_MODE_SEL_CDB6);

    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
    if( sleep_action != CTAPE_NOSLEEP) {

        ccmn_send_ccb_wait( pdrv_dev, (CCB_HEADER *)action->act_ccb, 
			NOT_RETRY, PRIBIO);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    }
    
    /*
     * The caller has said don't sleep they will handle the ccb
     */
    else {
	ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)action->act_ccb,NOT_RETRY);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	return;
    }

    /*
     * Remove from active lists
     */
    ccmn_rem_ccb( pdrv_dev, action->act_ccb );

    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
    switch( action->act_ccb_status ) {
    
    case CAT_CMP:
    
	/* 
         *  GOOD Status just return;
         */
        break;
    
    case CAT_CMP_ERR:
                    
	/* 
 	 * Had some sort of scsi status other then good
 	 * must look at each one.
	 */
            
	/* now we find out why ... either a check
	 * condition or reservation conflict....
	 */

	action->act_scsi_status = action->act_ccb->cam_scsi_status;
	switch(action->act_scsi_status)
            {
            default:
            case SCSI_STAT_GOOD:
            case SCSI_STAT_CONDITION_MET:
            case SCSI_STAT_BUSY:
            case SCSI_STAT_INTERMEDIATE:
            case SCSI_STAT_INTER_COND_MET:
            case SCSI_STAT_COMMAND_TERMINATED:
            case SCSI_STAT_QUEUE_FULL:
                /* 
                 * For all the above something is
                 * really messed up.. Since the commands
                 * are single threaded and not running 
                 * tagged commands.
                 */
		CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |=  ACT_FAILED;
	    	action->act_ret_error = EIO;
                break;
    
            case SCSI_STAT_RESERVATION_CONFLICT:
                /* this unit reserved by another
                 * initiator , this should not
                 * happen
                 */
		CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |=  ACT_FAILED;
		action->act_ret_error = EBUSY;
                break;
    
            case SCSI_STAT_CHECK_CONDITION:
    
                /* call ctape_ccb_chkcond() 
                 * to handle the check condition 
                 */
                action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, 
				pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) {
                /*
                 * Look at conditions.
                 */

		case CHK_INFORMATIONAL:
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;
                case CHK_RECOVER:
		    CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_EOM :
                case CHK_FILEMARK:
                case CHK_ILI:
                case CHK_CHK_NOSENSE:
                case CHK_SENSE_NOT_VALID:
                case CHK_NOSENSE_BITS:
                case CHK_NOT_READY:
                case CHK_UNIT_ATTEN:
                case CHK_DATA_PROT:
		case CHK_MEDIUM_ERR:
		case CHK_HARDWARE:
		case CHK_ILL_REQ:
		case CHK_BLANK_CHK:
		case CHK_VENDOR_SPEC:
		case CHK_COPY_ABORT:
		case CHK_EQUAL:
		case CHK_VOL_OFLOW:
		case CHK_MIS_CMP:
                case CHK_CMD_ABORTED:
                case CHK_UNKNOWN_KEY:
                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |=  ACT_FAILED;
		    action->act_ret_error = EIO;
		    break;
                    
                } /* end of switch for check condition */
    
	    break; /* end of scsi_status check condition */
    
	    } /* end of switch of scsi status */

	break; /* End of CAM_CMP_ERR */
    
    case CAT_INPROG:
    case CAT_UNKNOWN:
    case CAT_CCB_ERR:
    case CAT_RESET:
    case CAT_BUSY:
    case CAT_SCSI_BUSY:
    case CAT_BAD_AUTO:
    case CAT_DEVICE_ERR:
    case CAT_NO_DEVICE:
    case CAT_ABORT:
	action->act_fatal |=  ACT_FAILED;
	action->act_ret_error = EIO;
        /* 
         * Error log this we should never get 
         */
	CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	break;
    default:
        /* 
         * Error log this we should never get 
         */
	action->act_fatal |=  ACT_FAILED;
	action->act_ret_error = EIO;
	CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
	break;
    
    } /* end switch on cam status */
    
    /*
     * Now unlock
     */
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    
    
    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    return;

}
    


/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * This routine will issue a SCSI SPACE/REWIND command baseed on 
 * count and cmd to the unit. 
 * The varible sleep_action for this version will always be TRUE, which
 * directs the code to sleep waiting for comand status. If the
 * varible sleep_action is FALSE then we are in a recovery state (NEXT 
 * VERSION. With it being false then the code acts as a state machine
 * and steps  the machine. The varible err_report are flags for which
 * check condition errors not to report.
 *
 * Call syntax
 *  ctape_move_tape( pdrv_dev, action, done, cmd, count, flags, 
		sleep_action, err_report)
 *	PDRV_DEVICE	pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 The callers action struct ptr
 *	void		(*done)();	 Complettion routine
 *	I32		cmd;		 MTFSR, MTREW, etc,
 *	I32		count;		 How many 
 *	I32		flags;		 Command specific flags
 *	U32		sleep_action;	 Whether we sleep
 *	U32		err_report	 Which chk errors not to log.
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	Status is placed into the action struct for the caller
 *
 * Return values
 *	NONE
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */


void
ctape_move_tape( pdrv_dev, action, done, cmd, count, flags, 
		 sleep_action, err_report)
	PDRV_DEVICE	*pdrv_dev;	/* Pointer to the device struct */	
	CTAPE_ACTION	*action;	/* The callers action struct ptr */
	void		(*done)();	/* Complettion routine		*/
	I32		cmd;		/* MTFSR, MTREW, etc,		*/
	I32		count;		/* How many			*/
	I32		flags;		/* Command specific flags	*/
	U32		sleep_action;	/* Whether we sleep		*/
	U32		err_report;	/* Which err's not to report	*/

{

    /* 
     * Local Variables
     */

    TAPE_SPECIFIC	*ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;
				/*
				 * pointer to our device
				 * specific struct
				 */

    SEQ_SPACE_CDB6	*spacep; /* Will cast out the union in cdb	*/
    SEQ_REWIND_CDB6	*rewp; 	 /* Will cast out the union in cdb	*/
    CCB_ABORT		*abort_ccb;	/*  For interrupted commands	*/
    int			s;		/* Saved IPL			*/
    int			s1;		/* Throw away IPL		*/
    u_char		sense_size; /* Request sense size		*/
    static u_char	module[] = "ctape_move_tape"; /* Module name	*/


    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    /*
     * Make sure count is positive
     */
    if( count < NULL ){
	action->act_fatal |= ACT_PARAMETER;
	action->act_ret_error = EINVAL;
	return;
    }

    /* 
     * See if the user has set there request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    sense_size = GET_SENSE_SIZE( pdrv_dev );

    /*
     * Get an I/O ccb
     */
    action->act_ccb = ccmn_io_ccb_bld( pdrv_dev->pd_dev,(caddr_t)NULL, 
			(U32)NULL, sense_size,
			(U32)CAM_DIR_NONE, done,(u_char)NULL, 
			ctape_move_timo , (struct buf *)NULL); 

   /* 
    * Check if ccb is NULL if so the macro 
    * Error logs it and fills out action return values. 
    */
    if(action->act_ccb == (CCB_SCSIIO *)NULL){
    	CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	return;
    }


    /*
     * since we will be mucking with flags lock the specific
     * struct. It is unlocked in each case.
     */
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    /*
     * Since we are moving tape clear written flags and BOM flags
     */
    if( count != NULL){
    	ts_spec->ts_flags &= ~(CTAPE_WRITTEN | CTAPE_BOM);
    }

    /* 
     * Find out what our command is 
     * Please note that we adjust the count if a TAPE_MARK is
     * pending. This is for fixed block tapes.
     */
    switch( cmd ) {

    case MTFSF:  /* FORWARD SPACE FILE */
    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE ), 
		("[%d/%d/%d] %s: MTFSF count = 0x%X, flags = 0x%X\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module,
		count, ts_spec->ts_flags));

	if(( ts_spec->ts_flags & CTAPE_TPMARK_PENDING) != NULL){

	    count = count - 1;

	    if (count < 0 ){
		count = 0;
	    }

	    if( count == 0){
		ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;
		ts_spec->ts_flags |= CTAPE_TPMARK;

	    }

	    else {
		ts_spec->ts_flags= ~CTAPE_TPMARK_PENDING;
	    }
	}


	/*
	 * Build our command space command 6 byte
	 */
	spacep = (SEQ_SPACE_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;

	spacep->opcode = SEQ_SPACE_OP;		/* opcode */
	spacep->lun = 0;			/* ONLY scsi 2 */
	spacep->code = SEQ_SPACE_FILEMARKS; 
	/*
	 * count of how many to the cdb
	 */
	SEQ_COUNT_TO_SPACECDB6( count, spacep);

    	/* 
    	 * set our cdb lenght
    	 */
    	action->act_ccb->cam_cdb_len = sizeof(SEQ_SPACE_CDB6);

	break;

    case MTBSF:	/* BACK SPACE FILE */

    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE ), 
		("[%d/%d/%d] %s: MTBSF count = 0x%X, flags = 0x%X\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module,
		count, ts_spec->ts_flags));

	if(( ts_spec->ts_flags & CTAPE_TPMARK_PENDING) != NULL){
	    count = count + 1;
	    ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;
	}


	/*
	 * Build our command space command 6 byte
	 */
	spacep = (SEQ_SPACE_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;

	spacep->opcode = SEQ_SPACE_OP;		/* opcode */
	spacep->lun = 0;			/* ONLY scsi 2 */
	spacep->code = SEQ_SPACE_FILEMARKS; 
	/*
	 * count of how many to the cdb, Since basckspace
	 * Must be negative 2's compliment
	 */
	SEQ_COUNT_TO_SPACECDB6( (~count + 1), spacep);
    	/* 
    	 * set our cdb lenght
    	 */
    	action->act_ccb->cam_cdb_len = sizeof(SEQ_SPACE_CDB6);

	/* 
	 * Clear eom..... will notice one if we hit it.
	 */
	ts_spec->ts_flags &= ~CTAPE_EOM;

	 break;

    case MTFSR: /* FORWARD SPACE RECORD */
    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE ), 
		("[%d/%d/%d] %s: MTFSR count = 0x%X, flags = 0x%X\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module,
		count, ts_spec->ts_flags));

	if(( ts_spec->ts_flags & CTAPE_TPMARK_PENDING) != NULL){
	    count = count - 1;

	    if (count < 0 ){
		count = 0;
	    }

	    if( count == 0){
		ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;
		ts_spec->ts_flags |= CTAPE_TPMARK;
	    }
		
	    else {
		ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;
	    }
	}


	/*
	 * Build our command space command 6 byte
	 */
	spacep = (SEQ_SPACE_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;

	spacep->opcode = SEQ_SPACE_OP;		/* opcode */
	spacep->lun = 0;			/* ONLY scsi 2 */
	spacep->code = SEQ_SPACE_BLOCKS; 
	/*
	 * count of how many to the cdb
	 */
	SEQ_COUNT_TO_SPACECDB6( count, spacep);

    	/* 
    	 * set our cdb lenght
    	 */
    	action->act_ccb->cam_cdb_len = sizeof(SEQ_SPACE_CDB6);

	break;

    case MTBSR: /* BACK SPACE RECORD */

    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE ), 
		("[%d/%d/%d] %s: MTBSR count = 0x%X, flags = 0x%X\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module,
		count, ts_spec->ts_flags));

	if(( ts_spec->ts_flags & CTAPE_TPMARK_PENDING) != NULL){
	    count = count + 1;
	    ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;
	}


	/*
	 * Build our command space command 6 byte
	 */
	spacep = (SEQ_SPACE_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;

	spacep->opcode = SEQ_SPACE_OP;		/* opcode */
	spacep->lun = 0;			/* ONLY scsi 2 */
	spacep->code = SEQ_SPACE_BLOCKS; 
	/*
	 * count of how many to the cdb, Since basckspace
	 * Must be negative 2's compliment
	 */
	SEQ_COUNT_TO_SPACECDB6( (~count + 1), spacep);

    	/* 
    	 * set our cdb lenght
    	 */
    	action->act_ccb->cam_cdb_len = sizeof(SEQ_SPACE_CDB6);

	/* 
	 * Clear eom..... will notice one if we hit it.
	 */
	ts_spec->ts_flags &= ~CTAPE_EOM;

	break;

    case MTSEOD: /* SPACE TO END OF DATA */

    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE ), 
		("[%d/%d/%d] %s: MTSEOD count = 0x%X, flags = 0x%X\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module,
		count, ts_spec->ts_flags));

	ts_spec->ts_flags &= ~CTAPE_TPMARK_PENDING;
	ts_spec->ts_flags &= ~CTAPE_TPMARK;


	/*
	 * Build our command space command 6 byte
	 */
	spacep = (SEQ_SPACE_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;

	spacep->opcode = SEQ_SPACE_OP;		/* opcode */
	spacep->lun = 0;			/* ONLY scsi 2 */
	spacep->code = SEQ_SPACE_ENDDATA; 

    	/* 
    	 * set our cdb lenght
    	 */
    	action->act_ccb->cam_cdb_len = sizeof(SEQ_SPACE_CDB6);

	break;

    case MTREW:

    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE ), 
		("[%d/%d/%d] %s: MTREW count = 0x%X, flags = 0x%X\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module,
		count, ts_spec->ts_flags));

	/*
	 * Clear 
	 */
	ts_spec->ts_flags &= ~(CTAPE_EOM | CTAPE_CSE | CTAPE_SOFTERR |
		CTAPE_HARDERR | CTAPE_TPMARK | CTAPE_TPMARK_PENDING |
		CTAPE_SHRTREC | CTAPE_RDOPP );

	ts_spec->ts_state_flags &= ~CTAPE_POSITION_LOST_STATE;

	/*
	 * SET
	 */
	ts_spec->ts_flags |= (CTAPE_BOM | CTAPE_REWINDING );


	/*
	 * Build our command space command 6 byte
	 */
	rewp = (SEQ_REWIND_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;

	rewp->opcode = SEQ_REWIND_OP;
	rewp->lun = 0;
	if(( flags &  CMD_IMED ) != NULL) {
	    /* 
	     * They want return to be immediate
	     */
	    rewp->immed = 1;
	}

    	/* 
    	 * set our cdb lenght
    	 */
    	action->act_ccb->cam_cdb_len = sizeof(SEQ_REWIND_CDB6);

	break;

    default:
	/*
	 * Must release the ccb
	 */
	PDRV_IPLSMP_UNLOCK( pdrv_dev, s);
	ccmn_rel_ccb((CCB_HEADER *)action->act_ccb );
	action->act_fatal |= (ACT_PARAMETER | ACT_FAILED);
	action->act_ret_error = EINVAL;
	action->act_ccb = (CCB_SCSIIO *)NULL;
    	PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE ), 
		("[%d/%d/%d] %s: default count = 0x%X, flags = 0x%X\n", 
		DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
		DEV_LUN(pdrv_dev->pd_dev), module,
		count, ts_spec->ts_flags));

	return;
	/*NOTREACHED*/
	break;
	

    } /* end of switch cmd	*/


    /* 
     * Now ready to send the command down
     * This routine requires it to be locked before we call.
     */
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
    if( sleep_action != CTAPE_NOSLEEP) {
	/* 
	 * This is interruptable so we must handle it
	 */
        if(( ccmn_send_ccb_wait( pdrv_dev, (CCB_HEADER *)action->act_ccb, 
			NOT_RETRY, (PZERO + 1))) == EINTR ){

            PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	    abort_ccb = ccmn_abort_ccb_bld( pdrv_dev->pd_dev, CAM_DIR_NONE,
			action->act_ccb);
	    ccmn_rel_ccb((CCB_HEADER *)abort_ccb );
	    action->act_fatal |= (ACT_INTERRUPTED | ACT_FAILED);
	    action->act_ret_error = EINTR;
	    /*
	     * Wait for the ccb to come back to us
	     */
    	    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );
	    if( (((PDRV_WS *)action->act_ccb->cam_pdrv_ptr)->pws_flags & 
			PWS_CALLBACK) == NULL){	
	        PDRV_SMP_SLEEPUNLOCK(action->act_ccb, PRIBIO, pdrv_dev);
	    }
	    /* 
	     * Must unlock (mpsleep) after thread block will relock.
	     */
            PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

    	    /*
	     * Remove from active lists
    	     */
    	    ccmn_rem_ccb( pdrv_dev, action->act_ccb );
	    /*
	     * Get our status
	     */
    	    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
	    /* 
	     * QUESTION FOR GROUP..... Do we set position lost here....
	     */

	    /* 
	     * Return no need to check.
	     */

	    return;
	}
	else {
	    /* 
	     * Just unlock it
	     */
            PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
	}
    }
    
    /*
     * The caller has said don't sleep they will handle the ccb
     */
    else {
	ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)action->act_ccb,NOT_RETRY);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	return;
    }

    /*
     * Remove from active lists
     */
    ccmn_rem_ccb( pdrv_dev, action->act_ccb );


    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    /* 
     * If we got here the command has been sent down and completed
     * Now check for status.
     */


    if( cmd == MTREW ){
	ts_spec->ts_flags &= ~CTAPE_REWINDING;
    }
    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
    switch( action->act_ccb_status ) {
    
    case CAT_CMP:
    
	/* 
 	 *  GOOD Status just return;
 	 */

	break;
    
     case CAT_CMP_ERR:
                    
	/* 
 	 * Had some sort of scsi status other then good
 	 * must look at each one.
 	 */
          
	/* now we find out why ... either a check
 	 * condition or reservation conflict....
 	 */
	action->act_scsi_status = action->act_ccb->cam_scsi_status;
	switch(action->act_scsi_status)
	{
	default:
	case SCSI_STAT_GOOD:
	case SCSI_STAT_CONDITION_MET:
	case SCSI_STAT_BUSY:
	case SCSI_STAT_INTERMEDIATE:
	case SCSI_STAT_INTER_COND_MET:
	case SCSI_STAT_COMMAND_TERMINATED:
	case SCSI_STAT_QUEUE_FULL:
	    /* 
 	     * For all the above something is
 	     * really messed up.. Since the commands
 	     * are single threaded and not running 
 	     * tagged commands.
 	     */
	    CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	    
	    action->act_fatal |= ACT_FAILED;
	    action->act_ret_error = EIO;
	    break;
    
	case SCSI_STAT_RESERVATION_CONFLICT:
	    /* this unit reserved by another
 	     * initiator , this should not
 	     * happen
 	     */
	    CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	    action->act_fatal |= ACT_FAILED;
	    action->act_ret_error = EBUSY;
	    break;
    
	case SCSI_STAT_CHECK_CONDITION:
    
	    /* call ctape_ccb_chkcond() 
 	     * to handle the check condition 
 	     */
	    action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) {
                /*
                 * Look at common conditions first.
                 */
                case CHK_EOM :
		    /* 
		     * We can hit eom on forward space bsck space etc....
		     * Check resid to see if we are not where we think
		     * we are...The only command that moves tape that
		     * we don't care about is space to end of data...
		     * We expect eom on that one....
		     */
		    if(cmd == MTSEOD) {
			break;
		    }
		    else if(cmd == MTREW) {
			if(( err_report & ERR_EOM) == NULL){
		    		CAM_ERROR(module, "Hit EOM on rewind", 
					CAM_HARDERR, 
					(CCB_HEADER *)action->act_ccb, 
					pdrv_dev->pd_dev,
					(u_char *)NULL);
			}
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
			action->act_fatal |= ACT_FAILED;
		    	action->act_ret_error = EIO;
			break;
		    }	
		    /* 
		     * This is a strange one..... If we are doing back space
		     * ops an we hit bom ... we get a check condition media
		     * error with the eom bit set..... we must clear eom and
		     * set bom... then check resid field....
		     */
		    if(( cmd == MTBSR) || (cmd == MTBSF)){
			ts_spec->ts_flags &= ~CTAPE_EOM;
			ts_spec->ts_flags |= CTAPE_BOM;
			if(ts_spec->ts_resid != NULL) {
			    action->act_fatal |= ACT_FAILED;
			    action->act_ret_error = EIO;
			}
		    }

                    break;
    
                case CHK_FILEMARK:
                    /*
		     * File marks are not an error for the most part
		     * If the comand is a record command we must 
		     * Tell the user that they hit one..return 
		     * indication.
                     */
		    if(((cmd == MTFSR) || (cmd == MTBSR)) && 
				(ts_spec->ts_resid != NULL)) {

			action->act_fatal |= ACT_FAILED;
			action->act_ret_error = EIO;
		    }
                    break; 
    
                case CHK_ILI:
		    /* 
		     * We really should not be getting an Illegal 
		     * lenght error..... fail it.
		     */
		    if(( err_report & ERR_ILI) == NULL){
		    	CAM_ERROR(module, "ILI on a move tape command", 
				CAM_HARDERR, (CCB_HEADER *)action->act_ccb, 
				pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;
		    
                /* 
                 * Do nothing 
                 */
                case CHK_RECOVER:
		    if(( err_report & ERR_RECOV) == NULL){
		        CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			    (CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			    (u_char *)NULL);
		    }
		    break;
		case CHK_INFORMATIONAL:
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;
    
            
                /* 
                 * The rest just mark error and abort que.
                 */
                case CHK_CHK_NOSENSE:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_SENSE_NOT_VALID:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_NOSENSE_BITS:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_NOT_READY:
		    if(( err_report & ERR_NOT_RDY) == NULL){
		    	CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_MEDIUM_ERR:
		    if(( err_report & ERR_MEDIUM) == NULL){
		        CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

		case CHK_HARDWARE:
		    if(( err_report & ERR_HARDWARE) == NULL){
		    	CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

		case CHK_ILL_REQ:
		    if(( err_report & ERR_ILL_REQ) == NULL){
		        CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_UNIT_ATTEN:
		    if(( err_report & ERR_UNIT_ATTEN) == NULL){
		        CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_DATA_PROT:
		    if(( err_report & ERR_DATA_PROT) == NULL){
		        CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

		case CHK_BLANK_CHK:
		    if(( err_report & ERR_BLANK_CHK) == NULL){
		        CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

		case CHK_VENDOR_SPEC:
		    if(( err_report & ERR_VENDOR_SPEC ) == NULL){
		        CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_COPY_ABORT:
		    if(( err_report & ERR_COPY_ABORT) == NULL){
		        CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_CMD_ABORTED:
		    if(( err_report & ERR_CMD_ABORTED) == NULL){
		    	CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

		case CHK_EQUAL:
		    if(( err_report & ERR_EQUAL) == NULL){
		    	CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

		case CHK_VOL_OFLOW:
		    if(( err_report & ERR_VOL_OFLOW) == NULL){
		    	CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

		case CHK_MIS_CMP:
		    if(( err_report & ERR_MIS_CMP) == NULL){
		    	CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    }
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                case CHK_UNKNOWN_KEY:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;

                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
		    if(cmd == MTREW) {
			ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    }	
		    break;
                    
                } /* end of switch for check condition */
    
            break; /* end of scsi_status check condition */
    
            } /* end of switch of scsi status */

	break; /* End of CAM_CMP_ERR */
    
    
        case CAT_INPROG:
        case CAT_UNKNOWN:
        case CAT_CCB_ERR:
        case CAT_RESET:
        case CAT_BUSY:
        case CAT_SCSI_BUSY:
        case CAT_BAD_AUTO:
        case CAT_DEVICE_ERR:
        case CAT_NO_DEVICE:
        case CAT_ABORT:
	    action->act_fatal |= ACT_FAILED;
	    action->act_ret_error = EIO;
            /* 
             * Error log this we should never get 
             */
	    CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	    break;
        default:
            /* 
             * Error log this we should never get 
             */
	    action->act_fatal |= ACT_FAILED;
	    action->act_ret_error = EIO;
	    CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
            break;
    
        } /* end switch on cam status */
    
        /*
         * Now unlock
         */
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    
    
    


    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    return;


} /* End of ctape_move_tape() */





/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * This routine will issue a SCSI load/unload command based on 
 * count and cmd to the unit. 
 * The varible sleep_action for this version will always be TRUE, which
 * directs the code to sleep waiting for comand status. If the
 * varible sleep_action is FALSE then we are in a recovery state (NEXT 
 * VERSION. With it being false then the code acts as a state machine
 * and steps  the machine
 *
 * Call syntax
 *  ctape_load_tape( pdrv_dev, action, done, flags, sleep_action)
 *	PDRV_DEVICE	pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 Callers action struct pointer
 *	void		(*done)();	 Complettion routine
 *	I32		flags;		 Command specific flags
 *	U32		sleep_action;	 Whether we sleep
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	CTAPE_ACTION struct filled in.
 *
 * Return values
 *	NONE
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */


void
ctape_load_tape( pdrv_dev, action, done, flags, sleep_action)
	PDRV_DEVICE	*pdrv_dev;	/* Pointer to the device struct */	
	CTAPE_ACTION	*action;	/* Callers action struct pointer */
	void		(*done)();	/* Complettion routine		*/
	I32		flags;		/* Command specific flags	*/
	U32		sleep_action;	/* Whether we sleep		*/

{

    /* 
     * Local Variables
     */

    TAPE_SPECIFIC    *ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;
				/*
				 * pointer to our device
				 * specific struct
				 */

    SEQ_LOAD_CDB6     *loadp;   /* Will cast out the union in cdb	*/
    int		      s;	/* Saved IPL				*/
    int		      s1;	/* Throw away IPL			*/
    u_char	      sense_size;/* Request sense size			*/
    static u_char	module[] = "ctape_load_tape"; /* Module name	*/


    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    /* 
     * See if the user has set there request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    sense_size = GET_SENSE_SIZE( pdrv_dev );

    /*
     * Get an I/O ccb
     */
    action->act_ccb = ccmn_io_ccb_bld( pdrv_dev->pd_dev,(caddr_t)NULL, 
    		(U32)NULL, sense_size, (U32)CAM_DIR_NONE, 
		done, (u_char)NULL, ctape_move_timo,
		(struct buf *)NULL); 

   /* 
    * Check if ccb is NULL if so the macro 
    * Error logs it and fills out action return values. 
    */
    if(action->act_ccb == (CCB_SCSIIO *)NULL){
    	CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	return;
    }

    /*
     * Clear the position lost
     */
    ts_spec->ts_state_flags &= ~CTAPE_POSITION_LOST_STATE;

    /*
     * Set EOM or BOM based on flag.
     */
    if((flags & LOAD_CMD_EOT) == NULL){
	ts_spec->ts_flags |= CTAPE_BOM;
    }
    else {
	ts_spec->ts_flags |= CTAPE_EOM;
    }

    loadp = (SEQ_LOAD_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;

    loadp->opcode = SEQ_LOAD_OP;		/* opcode */
    loadp->lun = 0;			/* ONLY scsi 2 */
    if(( flags & CMD_IMED ) != NULL){
	loadp->immed = 1;
    }
    if(( flags & LOAD_CMD_UNLOAD ) != NULL){
	loadp->load = 0;
    }
    if(( flags & LOAD_CMD_LOAD ) != NULL){
	loadp->load = 1;
    }
    if(( flags & LOAD_CMD_RET ) != NULL){
	loadp->reten = 1;
    }
    if(( flags & LOAD_CMD_EOT ) != NULL){
	loadp->eot = 1;
    }

    /* 
     * set our cdb lenght
     */
    action->act_ccb->cam_cdb_len = sizeof(SEQ_LOAD_CDB6);


    /*
     * Since we will be mucking with flags lock the specific
     * struct. 
     */
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );


    /* 
     * Clear out the flags.........
     */
    ts_spec->ts_flags &= ~(CTAPE_TPMARK_PENDING | CTAPE_TPMARK | 
	CTAPE_HARDERR | CTAPE_SOFTERR );


    /* 
     * Now ready to send the command down
     * This routine requires it to be locked before we call.
     */
    if( sleep_action != CTAPE_NOSLEEP) {

        ccmn_send_ccb_wait( pdrv_dev, (CCB_HEADER *)action->act_ccb, 
			NOT_RETRY, PRIBIO);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    }
    
    /*
     * The caller has said don't sleep they will handle the ccb
     */
    else {
	ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)action->act_ccb,NOT_RETRY);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	return;
    }

    /*
     * Remove from active lists
     */
    ccmn_rem_ccb( pdrv_dev, action->act_ccb );

    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );


    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
    switch( action->act_ccb_status ) {
    
    case CAT_CMP:
    
	/* 
         *  GOOD Status just return;
         */
        break;
    
    case CAT_CMP_ERR:
                    
	/* 
 	 * Had some sort of scsi status other then good
 	 * must look at each one.
	 */
            
	/* now we find out why ... either a check
	 * condition or reservation conflict....
	 */

	action->act_scsi_status = action->act_ccb->cam_scsi_status;
	switch(action->act_scsi_status)
            {
            default:
            case SCSI_STAT_GOOD:
            case SCSI_STAT_CONDITION_MET:
            case SCSI_STAT_BUSY:
            case SCSI_STAT_INTERMEDIATE:
            case SCSI_STAT_INTER_COND_MET:
            case SCSI_STAT_COMMAND_TERMINATED:
            case SCSI_STAT_QUEUE_FULL:
                /* 
                 * For all the above something is
                 * really messed up.. Since the commands
                 * are single threaded and not running 
                 * tagged commands.
                 */
		CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |= ACT_FAILED;
	    	action->act_ret_error = EIO;
                break;
    
            case SCSI_STAT_RESERVATION_CONFLICT:
                /* this unit reserved by another
                 * initiator , this should not
                 * happen
                 */
		CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |= ACT_FAILED;
		action->act_ret_error = EBUSY;
                break;
    
            case SCSI_STAT_CHECK_CONDITION:
    
                /* call ctape_ccb_chkcond() 
                 * to handle the check condition 
                 */
                action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, 
				pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) {

                case CHK_RECOVER:
		    CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;
		case CHK_INFORMATIONAL:
		    /*
		     * Do nothing informational message....
		     */
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;


                /*
                 * Look at common conditions.
                 */
                case CHK_EOM :
		    /* 
		     * LOAD at eot???
		     */
    		    if(( flags & LOAD_CMD_EOT ) != NULL){
			break;
    		    }
                case CHK_FILEMARK:
                case CHK_ILI:
                case CHK_CHK_NOSENSE:
                case CHK_SENSE_NOT_VALID:
                case CHK_NOSENSE_BITS:
                case CHK_NOT_READY:
                case CHK_UNIT_ATTEN:
                case CHK_DATA_PROT:
		case CHK_MEDIUM_ERR:
		case CHK_HARDWARE:
		case CHK_ILL_REQ:
		case CHK_BLANK_CHK:
		case CHK_VENDOR_SPEC:
		case CHK_COPY_ABORT:
		case CHK_EQUAL:
		case CHK_VOL_OFLOW:
		case CHK_MIS_CMP:
                case CHK_CMD_ABORTED:
                case CHK_UNKNOWN_KEY:
                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    ts_spec->ts_state_flags |= CTAPE_POSITION_LOST_STATE;
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
                    
                } /* end of switch for check condition */
    
	    break; /* end of scsi_status check condition */
    
	    } /* end of switch of scsi status */

	break; /* End of CAM_CMP_ERR */
    
    case CAT_INPROG:
    case CAT_UNKNOWN:
    case CAT_CCB_ERR:
    case CAT_RESET:
    case CAT_BUSY:
    case CAT_SCSI_BUSY:
    case CAT_BAD_AUTO:
    case CAT_DEVICE_ERR:
    case CAT_NO_DEVICE:
    case CAT_ABORT:
	action->act_fatal |= ACT_FAILED;
	action->act_ret_error = EIO;
        /* 
         * Error log this we should never get 
         */
	CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	break;
    default:
    	/* 
	 * Error log this we should never get 
    	 */
	action->act_fatal |= ACT_FAILED;
	action->act_ret_error = EIO;
	CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
	break;
    
    } /* end switch on cam status */
    
    /*
     * Now unlock
     */
    PDRV_IPLSMP_UNLOCK( pdrv_dev, s );

    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    return;


} /* End of ctape_load() */



/* ---------------------------------------------------------------------- */
/* Function description.
 * 
 * This routine will issue a SCSI Write file/set marks command baseed on 
 * count and cmd to the unit. 
 * The varible sleep_action for this version will always be TRUE, which
 * directs the code to sleep waiting for comand status. If the
 * varible sleep_action is FALSE then we are in a recovery state (NEXT 
 * VERSION. With it being false then the code acts as a state machine
 * and steps  the machine
 *
 * Call syntax
 *  ctape_wfm( pdrv_dev, action, done, count, flags, sleep_action)
 *	PDRV_DEVICE	*pdrv_dev;	 Pointer to the device struct	
 *	CTAPE_ACTION	*action;	 Callers action struct pointer
 *	void		(*done)();	 Complettion routine
 *	I32		count;		 How many 
 *	I32		flags;		 Command specific flags
 *	U32		sleep_action;	 Whether we sleep
 * 
 * Implicit inputs
 * 	NONE
 *
 * Implicit outputs 
 *	Action structure is filled in basaed on command status.
 *
 * Return values
 *	NONE 
 *
 * TO DO:
 *	No sleep and state step
 * 	Interrupted sleeps
 */


void
ctape_wfm( pdrv_dev, action, done, count, flags, sleep_action)
	PDRV_DEVICE	*pdrv_dev;	/* Pointer to the device struct */	
 	CTAPE_ACTION	*action;	/* Callers action struct pointer */
	void		(*done)();	/* Complettion routine		*/
	I32		count;		/* How many			*/
	I32		flags;		/* Command specific flags	*/
	U32		sleep_action;	/* Whether we sleep		*/

{

    /* 
     * Local Variables
     */

    TAPE_SPECIFIC	*ts_spec = (TAPE_SPECIFIC *)pdrv_dev->pd_specific;
				/*
				 * pointer to our device
				 * specific struct
				 */
    DEV_DESC	         *dev_desc = pdrv_dev->pd_dev_desc;
				/*
				 * Pointer to device descriptor
				 */

    SEQ_WRITEMARKS_CDB6  *wrtp;		/* Will cast out the union in cdb*/
    int			  s;		/* Saved IPL			*/
    int			  s1;		/* Throw away IPL		*/
    u_char		  sense_size;	/* Request sense size		*/
    static u_char	module[] = "ctape_wfm"; /* Module name	*/


    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));
    /*
     * Make sure count is positive
     */
    if( count < NULL ){
	action->act_fatal |= (ACT_PARAMETER |ACT_FAILED);
	action->act_ret_error = EINVAL;
	return;
    }

    /* 
     * See if the user has set there request sense
     * size. This is for auto sense. If there is an error
     * the lower levels will do a request sense for us.
     */
    if( (dev_desc->dd_valid & DD_REQSNS_VAL ) != NULL){
	sense_size = dev_desc->dd_req_sense_len;
    }
    else {
	sense_size = DEC_AUTO_SENSE_SIZE;
    }

    /*
     * Get an I/O ccb
     */
    action->act_ccb = ccmn_io_ccb_bld( pdrv_dev->pd_dev,(caddr_t)NULL, 
			(U32)NULL, sense_size,
			(u_long)CAM_DIR_NONE, done,(u_char)NULL, 
			count + ctape_wfm_base_timo,
			(struct buf *)NULL); 

   /* 
    * Check if ccb is NULL if so the macro 
    * Error logs it and fills out action return values. 
    */
    if(action->act_ccb == (CCB_SCSIIO *)NULL){
    	CTAPE_NULLCCB_ERR(action, pdrv_dev, module);
	return;
    }

    wrtp = (SEQ_WRITEMARKS_CDB6 *)action->act_ccb->cam_cdb_io.cam_cdb_bytes;

    wrtp->opcode = SEQ_WRITEMARKS_OP;		/* opcode */
    wrtp->lun = 0;			/* ONLY scsi 2 */
    if(( flags & CMD_IMED ) != NULL){
	wrtp->immed = 1;
    }

    /*
     * Count of how many to the cdb.
     */
    SEQ_COUNT_TO_WFM_CDB6( count, wrtp);

    /* 
     * set our cdb lenght
     */
    action->act_ccb->cam_cdb_len = sizeof(SEQ_LOAD_CDB6);

    /*
     * Since we will be mucking with flags lock the specific
     * struct. 
     */
    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );


    /* 
     * Clear out the flags.........
     */
    ts_spec->ts_flags &= ~(CTAPE_TPMARK_PENDING | CTAPE_TPMARK | CTAPE_EOM |
		CTAPE_WRITTEN | CTAPE_BOM);


    /* 
     * Now ready to send the command down
     * This routine requires it to be locked before we call.
     */
    if( sleep_action != CTAPE_NOSLEEP) {

        ccmn_send_ccb_wait( pdrv_dev, (CCB_HEADER *)action->act_ccb, 
			NOT_RETRY, PRIBIO);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    }
    
    /*
     * The caller has said don't sleep they will handle the ccb
     */
    else {
	ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)action->act_ccb,NOT_RETRY);
        PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    	return;
    }

    /*
     * Remove from active lists
     */
    ccmn_rem_ccb( pdrv_dev, action->act_ccb );


    PDRV_IPLSMP_LOCK( pdrv_dev, LK_RETRY, s );

    action->act_ccb_status = ccmn_ccb_status((CCB_HEADER *)action->act_ccb);
    
    switch( action->act_ccb_status ) {
    
    case CAT_CMP:
    
	/* 
         *  GOOD Status just return;
         */
        break;
    
    case CAT_CMP_ERR:
                    
	/* 
 	 * Had some sort of scsi status other then good
 	 * must look at each one.
	 */
            
	/* now we find out why ... either a check
	 * condition or reservation conflict....
	 */

	action->act_scsi_status = action->act_ccb->cam_scsi_status;
	switch(action->act_scsi_status)
            {
            default:
            case SCSI_STAT_GOOD:
            case SCSI_STAT_CONDITION_MET:
            case SCSI_STAT_BUSY:
            case SCSI_STAT_INTERMEDIATE:
            case SCSI_STAT_INTER_COND_MET:
            case SCSI_STAT_COMMAND_TERMINATED:
            case SCSI_STAT_QUEUE_FULL:
                /* 
                 * For all the above something is
                 * really messed up.. Since the commands
                 * are single threaded and not running 
                 * tagged commands.
                 */
		CAM_ERROR(module,"Unexpected scsi status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |= ACT_FAILED;
	    	action->act_ret_error = EIO;
                break;
    
            case SCSI_STAT_RESERVATION_CONFLICT:
                /* this unit reserved by another
                 * initiator , this should not
                 * happen
                 */
		CAM_ERROR(module,"Reservation conflict", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		action->act_fatal |= ACT_FAILED;
		action->act_ret_error = EBUSY;
                break;
    
            case SCSI_STAT_CHECK_CONDITION:
    
                /* call ctape_ccb_chkcond() 
                 * to handle the check condition 
                 */
                action->act_chkcond_error = ctape_ccb_chkcond(action->act_ccb, 
					pdrv_dev);        
    
                /* 
                 * Now determine what to do.
                 */
                switch ( action->act_chkcond_error ) {
                /*
                 * Look at common conditions first.
                 */
                case CHK_EOM:
		    /*
		     * We can hit and eom of write of marks.... Make 
		     * sure that everything we wanted got to tape....
		     */
		    if( ts_spec->ts_resid != NULL){
			action->act_fatal |= ACT_FAILED;
			action->act_ret_error = EIO;
		    }
		    break;
	
                case CHK_RECOVER:
		    CAM_ERROR(module,(u_char *)NULL, CAM_SOFTERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;
		case CHK_INFORMATIONAL:
		    /* 
		     * Informational message do nothing
		     */
		    CAM_ERROR(module,(u_char *)NULL, CAM_INFORMATIONAL,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
		    break;

                case CHK_FILEMARK:
                case CHK_ILI:
                case CHK_CHK_NOSENSE:
                case CHK_SENSE_NOT_VALID:
                case CHK_NOSENSE_BITS:
                case CHK_NOT_READY:
                case CHK_UNIT_ATTEN:
                case CHK_DATA_PROT:
		case CHK_MEDIUM_ERR:
		case CHK_HARDWARE:
		case CHK_ILL_REQ:
		case CHK_BLANK_CHK:
		case CHK_VENDOR_SPEC:
		case CHK_COPY_ABORT:
		case CHK_EQUAL:
		case CHK_VOL_OFLOW:
		case CHK_MIS_CMP:
                case CHK_CMD_ABORTED:
                case CHK_UNKNOWN_KEY:
                default:
		    CAM_ERROR(module, (u_char *)NULL, CAM_HARDERR,
				(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
				(u_char *)NULL);
		    action->act_fatal |= ACT_FAILED;
		    action->act_ret_error = EIO;
                    
                } /* end of switch for check condition */
    
	    break; /* end of scsi_status check condition */
    
	    } /* end of switch of scsi status */

	break; /* End of CAM_CMP_ERR */
    
    case CAT_INPROG:
    case CAT_UNKNOWN:
    case CAT_CCB_ERR:
    case CAT_RESET:
    case CAT_BUSY:
    case CAT_SCSI_BUSY:
    case CAT_BAD_AUTO:
    case CAT_DEVICE_ERR:
    case CAT_NO_DEVICE:
    case CAT_ABORT:
	action->act_fatal |= ACT_FAILED;
	action->act_ret_error = EIO;
        /* 
         * Error log this we should never get 
         */
	CAM_ERROR(module,"Unexpected CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
	break;
    default:
    	/* 
    	 * Error log this we should never get 
    	 */
	action->act_fatal |= ACT_FAILED;
	action->act_ret_error = EIO;
	CAM_ERROR(module,"Unknown CCB status", CAM_HARDERR,
			(CCB_HEADER *)action->act_ccb, pdrv_dev->pd_dev,
			(u_char *)NULL);
    
	break;
    
    } /* end switch on cam status */
    
    /*
     * Now unlock
     */
    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);
    
    

    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    return;


} /* End of ctape_wfm() */

    
/* ---------------------------------------------------------------------- */
/*
 * Error checking routines
 */



/* ---------------------------------------------------------------------- */
/* Function description. */

/*
 * ctape_ccb_chkcond()
 * 
 * Routine Name : ctape_ccb_chkcond
 * 
 * Functional Description:
 * 
 * 
 * 	This routine handles the sns_data sense data for the tape driver
 * 	and returns the appropriate status back to the caller.  The
 * 	routine gets called when a CAM_SCSIIO ccb is returned with a
 * 	CAM_STATUS of CAM_REQ_CMP_ERR ( request completed with error).
 * 	and the cam_scsi_status equals SCSI_CHECK_CONDITION.  The
 * 	routine also will set the appropriate flags in the device
 * 	drivers tape specific structure. 
 *	NOTE... This routine must be called with the device SMP LOCKED.
 * 
 * 	Call Syntax:
 * 
 * 		ctape_ccb_chkcond( ccb, pdrv_device )
 * 			PDRV_DEVICE *pdrv_device;
 * 			CCB_SCSIIO  *ccb;
 * 
 * 	Return Values:
 * 		int:
 * 
 * 		CHK_CHK_NOSENSE
 * 			Lower levels could not get the request sense to
 * 			complete without error. Sense buffer not
 * 			valid.
 * 
 * 		CHK_SENSE_NOT_VALID
 * 			The valid bit in the sense buffer is not set
 * 			sense data is useless. 
 * 	
 * 		CHK_EOM
 * 			End of media detected.
 * 
 * 		CHK_FILEMARK
 * 			Filemark detected.
 * 
 * 		CHK_ILI
 * 			Incorrect lenght detected.
 * 		
 * 		CHK_NOSENSE_BITS
 * 			Sense key equals no sense but there
 * 			are no bits set in byte 2 of sense
 * 			data.
 * 		
 * 		CHK_RECOVER
 * 			Soft error detected corrected by the
 * 			unit.
 * 	
 * 		CHK_NOT_READY
 * 			The unit is not ready.
 *
 * 		CHK_MEDIUM_ERR
 * 			The unit has detected a hard medium error.
 * 
 * 		CHK_HARDWARE
 * 			The unit has detected a hard device error.
 * 
 * 		CHK_ILL_REQ
 * 			The unit has rejected this cmd. Unit may
 *			not support.
 * 
 * 		CHK_UNIT_ATTEN
 * 			The unit has either had a tape change or
 * 			just powered up.
 * 
 * 		CHK_DATA_PROT
 * 			The unit is write protected.
 * 
 * 		CHK_BLANK_CHK
 * 			The tape is blank erased at this point
 * 
 * 		CHK_VENDOR_SPEC
 * 			This is a vendor specific key (no idea)
 * 
 * 		CHK_COPY_ABORT
 * 			Copy cmd has been aborted.
 * 
 * 		CHK_CMD_ABORTED
 * 			The unit aborted this command.
 * 
 * 		CHK_EQUAL
 * 			Search cmd has been satisfied
 * 
 * 		CHK_VOL_OFLOW
 *			Physical end of media and data in buffer.
 * 
 * 		CHK_MIS_CMP
 *			Src != Media
 * 
 *		CHK_INFORMATIONAL
 *			Unit is reporting an informational message
 *
 * 		CHK_UNKNOWN_KEY
 * 			The unit has returned a sense key that
 * 			is not supported by the SCSI 2 spec.
 * 
 */


 




U32 
ctape_ccb_chkcond( ccb, pdrv_device )

    PDRV_DEVICE *pdrv_device;   /* Pointer to our device struct */ 
    CCB_SCSIIO  *ccb;           /* The ccb that had the check condition */ 

{

    /*
     * Local declarations
     */
 
    /* pointer to our tape specific structure				*/
    TAPE_SPECIFIC *ts_spec = (TAPE_SPECIFIC *)pdrv_device->pd_specific;
  
    /* Pointer to the sense data					*/
    ALL_REQ_SNS_DATA *sns_data = (ALL_REQ_SNS_DATA *)ccb->cam_sense_ptr;
  
    int		ret_val; 	/* What we return			*/
    static u_char	module[] = "ctape_ccb_chkcond"; /* Module name	*/
  
	

  
	
    PRINTD(DEV_BUS_ID(pdrv_device->pd_dev), DEV_TARGET(pdrv_device->pd_dev), 
	DEV_LUN(pdrv_device->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_device->pd_dev), DEV_TARGET(pdrv_device->pd_dev), 
	DEV_LUN(pdrv_device->pd_dev), module));

    /* 
     * Check to see if we have valid sense data 
     */
    if(( ccb->cam_ch.cam_status & CAM_AUTOSNS_VALID) == NULL){
	/* 
	 * since our sns_data sense is not valid
	 * return CHK_CHK_NOSENSE
	 */
	return( CHK_CHK_NOSENSE );
    }
    if( sns_data == NULL ) {
	panic("ctape_ccb_chkcond: CCB-AUTOSNS_VALID but data pointer = NULL");
	return(CHK_CHK_NOSENSE);
    }
    /*
     * Make sure buffer is long enougth
     */
    pdrv_device->pd_sense_len = ((u_short)ccb->cam_sense_len - 
					ccb->cam_sense_resid);
    if( pdrv_device->pd_sense_len < CTAPE_MIN_SENSE){
	/* 
	 * Sense buffer is really not large enough for
	 * use.... Error this.
	 */
	 return(CHK_CHK_NOSENSE);
    }
    /* 
     * Copy it to the last sense save area
     */
    bcopy ((caddr_t)ccb->cam_sense_ptr, pdrv_device->pd_sense_ptr,
					pdrv_device->pd_sense_len );

    /* 
     * We have valid sense data lets find out why
     * and report it.
     */

    PRINTD(DEV_BUS_ID(pdrv_device->pd_dev), 
	DEV_TARGET(pdrv_device->pd_dev), 
	DEV_LUN(pdrv_device->pd_dev), (CAMD_TAPE ), 
	("[%d/%d/%d] %s: error_code, 0x%x sense_key 0x%x asc 0x%x asq 0x%x\n", 
	DEV_BUS_ID(pdrv_device->pd_dev), 
	DEV_TARGET(pdrv_device->pd_dev), 
	DEV_LUN(pdrv_device->pd_dev), module,
	sns_data->error_code, sns_data->sns_key, sns_data->asc, 
	sns_data->asq));


    CTAPE_INFO(sns_data, ts_spec->ts_resid);
    /*
    printf("error_code, 0x%x sense_key 0x%x asc 0x%x asq 0x%x\n", 
	sns_data->error_code, sns_data->sns_key, sns_data->asc, 
	sns_data->asq);
    printf("ili 0x%x, eom 0x%x, filemark 0x%x info 0x%X\n", 
	sns_data->ili, sns_data->eom, sns_data->filemark,
	ts_spec->ts_resid);
    */



    /*
     * Make sure correct error code if 0x71 (deferred error) 
     * What do we do... punt????
     */
    if(( sns_data->error_code != 0x70) && 
		( sns_data->error_code != 0x71 )){

        return( CHK_SENSE_NOT_VALID );
    }
    
    /* 
     * Get the sense key and case it 
     */

    switch(sns_data->sns_key ){

    case ALL_NO_SENSE:
        /* 
         * Must look at the bit fields
         */
        if( sns_data->filemark != NULL){
	    /* 
	     * Set our flag 
	     */
	    ts_spec->ts_flags |= CTAPE_TPMARK;
	    ret_val = CHK_FILEMARK;

	    break;
        }
        else if( sns_data->eom != NULL){
	    /* 
	     * Set our flag 
	     */
	    ts_spec->ts_flags |= CTAPE_EOM;
	    ret_val = CHK_EOM;

	    break;
        }
        else if( sns_data->ili != NULL){
	    /* 
	     * Set our flag 
	     */
	    ts_spec->ts_flags |= CTAPE_SHRTREC;
	    ret_val = CHK_ILI;
    
	    break;
        }
        else {
	    /* 
	     * Since nothing is set  A warning has been sent
	     * We must return an indication
	     * Check to see if there is a reside count.... Fail it
	     * If there is... ret_val == CHK_NOSENSE_BITS else
	     * CHK_INFORMATIONAL
	     */


	    if( ccb->cam_resid != NULL ){
		ret_val = CHK_NOSENSE_BITS;
	    }
	    else if(ts_spec->ts_resid != NULL){
	    	ret_val = CHK_NOSENSE_BITS;
	    }
	    else {
		ret_val = CHK_INFORMATIONAL;
	    }
    
	    break;
	}
    
    case ALL_VOL_OVERFLOW:
	    /* 
	     * We are at end of media set the flag
	     */
	    ts_spec->ts_flags |= CTAPE_EOM;
	    ret_val = CHK_VOL_OFLOW;
    
	    break;
    

    case ALL_BLANK_CHECK:
	    /* 
	     * We are at end of media set the flag
	     */
	    ts_spec->ts_flags |= CTAPE_EOM;
	    ret_val = CHK_BLANK_CHK;

	    break;

    case ALL_RECOVER_ERR:
	    /* 
	     * Soft error 
	     */
	    ts_spec->ts_flags |= CTAPE_SOFTERR;
	    ret_val = CHK_RECOVER;
    
	    break;
    
    case ALL_NOT_READY:
	    ts_spec->ts_flags |= CTAPE_OFFLINE;
	    ret_val = CHK_NOT_READY;
    
	    break;
    
    case ALL_MEDIUM_ERR:
	    /*
	     * Some units will report medium error with the eom bit set
	     * when it hits either bom or eom..... We will return CHK_EOM
	     * and let the upper levels decide whether it is an eom or
	     * bom....
	     */

            if( sns_data->eom != NULL){
		/* 
		 * Set our flag 
		 */
		ts_spec->ts_flags |= CTAPE_EOM;
		ret_val = CHK_EOM;

		break;
	    }
	    /* 
	     * Hard error on the device 
	     */
	    ts_spec->ts_flags |= CTAPE_HARDERR;
	    ret_val = CHK_MEDIUM_ERR;
    
	    break;

    case ALL_HARDWARE_ERR:
	    /* 
	     * Hard error on the device 
	     */
	    ts_spec->ts_flags |= CTAPE_HARDERR;
	    ret_val = CHK_HARDWARE;
    
	    break;
    	
    case ALL_ILLEGAL_REQ:
	    /* 
	     * Hard error on the device 
	     */
	    ts_spec->ts_flags |= CTAPE_HARDERR;
	    ret_val = CHK_ILL_REQ;
    
	    break;
    	
    case ALL_COPY_ABORT:
	    /* 
	     * Hard error on the device 
	     */
	    ts_spec->ts_flags |= CTAPE_HARDERR;
	    ret_val = CHK_COPY_ABORT;
    
	    break;
    	
    case ALL_MISCOMPARE:
	    /* 
	     * Hard error on the device 
	     */
	    ts_spec->ts_flags |= CTAPE_HARDERR;
	    ret_val = CHK_MIS_CMP;
    
	    break;
    	
    case ALL_UNIT_ATTEN:
	    /* 
	     * Unit has had a tape change or has
	     * come up thru a power up.
	     */
	    ts_spec->ts_state_flags |= CTAPE_UNIT_ATTEN_STATE;
	    ts_spec->ts_state_flags &= ~(CTAPE_POSITION_LOST_STATE |CTAPE_SYNC_STATE);
	    ts_spec->ts_flags |= CTAPE_BOM;
	    ts_spec->ts_flags &= ~CTAPE_EOM;
	    ret_val = CHK_UNIT_ATTEN;
    
	    break;
    
    case ALL_DATA_PROTECT:
	    /* 
	     * Unit is write protected
	     */
	    ts_spec->ts_flags |= CTAPE_WRT_PROT;
	    ret_val = CHK_DATA_PROT;
    
	    break;
     
    /* 
     *These are not supported 
     */
    case ALL_VENDOR_SPEC:
	    ts_spec->ts_flags |= CTAPE_HARDERR;
	    ret_val = CHK_VENDOR_SPEC;
    
	    break;
    
    case ALL_EQUAL:
	    ret_val = CHK_EQUAL;
    
	    break;
    
    case ALL_ABORTED_CMD:
	    ts_spec->ts_flags |= CTAPE_HARDERR;
	    ret_val = CHK_CMD_ABORTED;
    
	    break;
    
    default:
	    /* 
	     * We have an unknown sense key 
	     */
	    ts_spec->ts_flags |= CTAPE_HARDERR;
	    ret_val = CHK_UNKNOWN_KEY;
    
	    break;
    }
    
    /* 
     * Return what happened 
     */
    PRINTD(DEV_BUS_ID(pdrv_device->pd_dev), DEV_TARGET(pdrv_device->pd_dev), 
	DEV_LUN(pdrv_device->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_device->pd_dev), DEV_TARGET(pdrv_device->pd_dev), 
	DEV_LUN(pdrv_device->pd_dev), module));

    
    return(ret_val);
    
    
}


/* ---------------------------------------------------------------------- */
/* Function description.
 *
 *
 * Routine Name: ctape_retry()
 *
 * Functional Description:
 *
 *	This routine is called by the timeout code. It is passed 
 *	a CCB will to send on done. Used for retries.
 *
 *
 *
 * Call Syntax:
 *
 *	ctape_retry( ccb )
 *
 *	CCB_SCSIIO *ccb;
 *
 *
 *
 *
 *
 * Returns :
 *	None
 */


void
ctape_retry (ccb)
	CCB_SCSIIO	*ccb;	/* The ccb to send		*/
{

    /*
     * Local variable
     */
    PDRV_DEVICE		*pdrv_dev;	/* Our device struct 	*/

    int			s;		/* Saved IPL		*/
    static u_char	module[] = "ctape_retry"; /* Module name	*/



    pdrv_dev = (PDRV_DEVICE *)((PDRV_WS *)ccb->cam_pdrv_ptr)->pws_pdrv; 


    if( pdrv_dev == NULL ){
	panic("ctape_retry: NULL PDRV_DEVICE pointer");
	return;
    }

    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: entry\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));


    PDRV_IPLSMP_LOCK(pdrv_dev, LK_RETRY, s);
    if( ccmn_send_ccb( pdrv_dev, (CCB_HEADER *)ccb, RETRY) != CAM_REQ_INPROG) {
	/* 
	 * The ccb can't be resent for what ever reason call it's done routine.
	 */
	
	((CCB_SCSIIO *)ccb)->cam_cbfcnp(ccb);
    }

    PDRV_IPLSMP_UNLOCK(pdrv_dev, s);

    /*
     * Now Release the SIM QUEUE.. Was frozen.
     */
    RELEASE_QUEUE(pdrv_dev);

    PRINTD(DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), (CAMD_TAPE |CAMD_INOUT), 
	("[%d/%d/%d] %s: exit\n", 
	DEV_BUS_ID(pdrv_dev->pd_dev), DEV_TARGET(pdrv_dev->pd_dev), 
	DEV_LUN(pdrv_dev->pd_dev), module));

    return;
}


/*
 * ctape_minphys will go away once the wrappers go....
 */


/************************************************************************
 *
 *  ROUTINE NAME: ctape_minphys()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will compare the b_bcount field in the buf
 *	structure with the maximum transfer limit for the device
 *	(dd_max_record) in the device descriptor structure. The
 *	count is adjusted if it is greater than the limit for
 *	reads, and sets B_ERROR in the bp to signify once around.
 *	For writes set reside to b_count , B_ERROR and b_error
 *	to say this request must be failed to physio.
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
ctape_minphys(bp)
struct buf *bp;
{

    PDRV_DEVICE *pdrv_dev;		/* Our pdrv device struct	*/
    DEV_DESC 	*dev_desc;		/* Our device descriptor 	*/
    dev_t	cam_dev;		/* Cam dev_t			*/

    cam_dev = bp->b_dev;

    PRINTD(DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), 
	   DEV_LUN(cam_dev), CAMD_TAPE, 
	   ("[%d/%d/%d] ctape_minphys: entry bp=%xx bcount=%xx\n", 
	   DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), DEV_LUN(cam_dev),
	   bp, bp->b_bcount));

    if ( (pdrv_dev = GET_PDRV_PTR(cam_dev)) == (PDRV_DEVICE *)NULL)  {
        PRINTD(DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), 
	   DEV_LUN(cam_dev), CAMD_TAPE, 
	   ("[%d/%d/%d] ctape_minphys: No periheral devcie struct\n", 
	   DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), DEV_LUN(cam_dev)));
	return;
    }

    dev_desc = pdrv_dev->pd_dev_desc;
	
    /*
     * Get the maximun transfer size for this device. If b_bcount
     * is greater then adjust it. For read only.. 
     * For anything greater then the device can do set b_flags to error
     * For writes set resid.
     */
    /* 
     * Indicate only once around in physio()
     */
    bp->b_flags |= B_ERROR;
    bp->b_resid = NULL;

    if (bp->b_bcount > dev_desc->dd_max_record ){

	/*
	 * If read allow it to read up to max device record size
	 * If write fail it
	 */
	if(( bp->b_flags & B_READ ) != NULL){
	    bp->b_bcount = dev_desc->dd_max_record;
	}
	else {
	    /*
	     * Indicate to physio not to do anything
	     */
	    bp->b_resid = bp->b_bcount;
	    bp->b_error = EINVAL;
	}

    }
    PRINTD(DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), 
	   DEV_LUN(cam_dev), CAMD_TAPE, 
	   ("[%d/%d/%d] ctape_minphys: exit - success\n", 
	   DEV_BUS_ID(cam_dev), DEV_TARGET(cam_dev), DEV_LUN(cam_dev)));
}
