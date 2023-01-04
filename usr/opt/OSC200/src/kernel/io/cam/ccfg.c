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
static char *rcsid = "@(#)$RCSfile: ccfg.c,v $ $Revision: 1.1.12.4 $ (DEC) $Date: 1993/11/23 21:54:16 $";
#endif

/* ---------------------------------------------------------------------- */

/* ccfg.c		Version 1.15			Dec. 11, 1991 

This file contains the routines and functional descriptions that makeup the
Configuration driver, (CDrv), in the CAM subsystem.

The CDrv contains 2 major parts.  The parts are Boot time configuration,
and CCB handling.  The CDrv is responsible for scanning the all the
SCSI buses to find target/LUNs.  When a target/LUN is found the Inquiry
data strings are stored in the cam_edt[] structure.  The CDrv supports
the CCBs that access the cam_edt[] structure.

Modification History

	Version	  Date		Who	Reason

	1.00	12/19/90	jag	Creation date. Created from the CDrv
					notes and functional spec.
	1.01	02/03/91	jag	Modified from last Func. spec. review,
					removed all Autosense code, minor mods
					to the EDT CCB routines.
	1.02	02/25/91	jag	Added the scanning/callback code.
	1.03	03/25/91	jag	Updated the sleep and locking code.
					Added the boot time configuration 
					pieces.  Changed names to ccfg.
	1.04	04/04/91	jag	Fixed a few bugs, the callback code
					will clear SIM_QFRZN, simattach()
					now expects a CAM status value returned
					from edtscan().
	1.05	04/07/91	jag	Added the SMP lock init code.
	1.06	05/15/91	dallas	ccfg_getinq() bcopy of inq data
					changed to reflect what was actually
					transferred.
	1.07	05/31/91	janet	Remove two sets of 
					#ifdef JAG_NOT_IHV_KIT
        1.08    06/28/91        rps     Added new config calls.
	1.09	07/03/91	jag	Added locking around the cam_conftbl[]
					for loaded SIMs.  Changes from the CDrv
					code review.
	1.10	08/06/91	jag	Added the checking for Initiator ID in
					the EDT scanning. 
	1.11	08/28/91	jag	Implemented timeout code for the probe
					scanning.  If the inital INQUIRY SCSI
					command failed, I reissued the command
					without the INITIATE_SYNC flag, for
					older SCSI-1 devices.
					Removed the PWS autosence init line.
	1.12	10/17/91	jag	Added the code to detecte when a new
					device is found.  The xpt_async()
					routine is called.
	1.13	11/15/91	janet   in ccfg_getinq() check 
					ccfg_use_sdtr_inquiry to decide if
					CAM_INITIATE_SYNC should be set.
	1.14	11/15/91	jag	Added Error logger support.
	1.15	12/11/91	jag	The probe timeout parameters now come
					from the cam_data.c file.
*/

/* ---------------------------------------------------------------------- */
/* Include files. */

#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam_debug.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/pdrv.h>
#include <vm/vm_kern.h>
#include <io/common/devdriver.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/xpt.h>
#include <dec/binlog/errlog.h>	/* UERF errlog defines */
#include <io/cam/cam_logger.h>		/* CAM error logging definess */
#include <io/cam/ccfg.h>
#include <io/cam/cam_errlog.h>
#include <io/cam/cam_config.h>


#define CAMERRLOG		/* Turn on the error logging code */

/* ---------------------------------------------------------------------- */
/* Local defines. */

#define CCFG_ERR_CNT	4		/* error entry list count */

/* ---------------------------------------------------------------------- */
/* Function type declarations for internal module routines. */

U32			ccfg_simattach();
int			ccfg_slave();
int			ccfg_attach();
U32			ccfg_action();
static void		ccfg_callback();
U32			ccfg_initialize();
U32			ccfg_gettype();
U32			ccfg_setasync();
U32			ccfg_settype();
U32			ccfg_edtscan();
static U32		ccfg_getinq();
static void		ccfg_ccb_wait();
static U32		ccfg_getsimid();
static void		ccfg_ccbtimeout();
static U32		ccfg_ok_toretry();
static void		ccfg_errlog();
static EDT *		ccfg_alloc_edt();
static ASYNC_INFO *	ccfg_alloc_async();
static void		ccfg_free_async();

/* ---------------------------------------------------------------------- */
/* External declarations. */

/* The EDT array is a multi layered grid of targets x luns.  It
will be primarly accessed via the edt_dir pointer array.  Each
element of the edt_dir[] will point to one of the "layers" of the
The edt_dir[] structure is indexed by the SIM-pathid. */

extern EDT *edt_dir[];			/* ptrs for EDT grid per HBA */
extern u_long N_edt_dir;		/* indicating the number of entries */

extern CAM_SIM_ENTRY *cam_conftbl[];	/* array containing the SIM entries */
extern U32 N_cam_conftbl;		/* indicating the number of entries */

extern struct cam_peripheral_driver cam_peripheral_drivers[];
extern int cam_pdrv_entries;

extern CAM_SIM_ENTRY xpt_sim_entry;	/* XPT place holder */
extern XPT_CTRL cam_conf_ctrl;		/* cam_conftbl[] control struct */
extern U32 ccfg_inquiry_retry_limit;	/* number of retries to do */
extern U32 ccfg_use_sdtr_inquiry;	/* flag for the EDT scanning */
extern U32 ccfg_wait_delay_loop;	/* how long to wait for the EDT CCBs */

extern I32 xpt_initialize();		/* initialize the XPT module */
extern I32 xpt_action();		/* issues the CCB to the SIMs */
extern CCB_HEADER *xpt_ccb_alloc();	/* gives a CCB to use */
extern void xpt_ccb_free();		/* frees a CCB nolonger needed */
extern I32 xpt_async();		/* XPT Async callback handler */
extern I32 cam_inisr();		/* for determining interrupt context */
extern I32 cam_at_boottime();		/* for determining probe context */
extern void cam_logger();		/* the error log handler for CAM */

extern void init_cam_components();	/* init the PDrv component lists */
extern void init_sim_components();	/* init the SIM component lists */

extern void panic();			/* general purpose "Aieeee" routine */
extern void bcopy();			/* gereral purpose copy routine */
extern void bzero();			/* gereral purpose 0 fill routine */
extern void wakeup();			/* wakes up sleeping processes */

extern caddr_t cdbg_CamStatus();	/* support func to report CAM status */

/* ---------------------------------------------------------------------- */
/* Initialized and uninitialized data. */

U32 cam_subsystem_ready = 0;		/* global flag to signal all is ready */

static volatile CCFG_CTRL ccfg_ctrl;	/* module control structure */
static CCFG_QHEAD ccfg_qhead;	/* header for the callback CCB list */

static void (*local_errlog)() = ccfg_errlog;	/* ptr for the logging Macro */

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_simattach()

Functional Description :
    This routine is used during the bootup configuration process in
Ultrix/BSD.  The SIM HBA code is called by the system configuration
code with system information.  The SIM HBA code will call this routine
with it's SIM_ENTRY and path id, (bus #) for the cam_conftbl[]
structure.  This routine will make sure that the CDrv and XPT are
initialized, update the cam_conftbl[] structure using the passed parameters,
call the SIM's init rouine via the SIM_ENTRY, and finally updated the
EDT structure with the inquiry data from the targets on the bus.

Formal Parameters : 
    A pointer to the SIM's SIM_ENTRY structure.
    The path ID for the SIM to control.

Implicit Inputs : 
    The CAM cam_conftbl[] structure and the EDT structure.
    The CAM subsystem initialized global flag.

Implicit Outputs :
    The CAM cam_conftbl[] structure and the EDT structure.

Return Value :
    A long value of CAM_SUCCESS or CAM_FAILURE for the ability to "attach"
the SIM to the subsystem.

Side Effects :
    The SCSI bus, from path ID is scanned, SCSI I/O CCBs are allocated and sent
to the SIM.  The scanning code may call sleep() or DELAY() depending on the
system boot state.

Additional Information :
    This code is Ultrix/BSD specific, it will have to be replaces/updated for
the Ultrix/OSF port and the new I/O configuration rules.

*/
static char *ccfg_simattach_func = "ccfg_simattach()";	/* func name */

U32
ccfg_simattach( se, bus_id )
    CAM_SIM_ENTRY *se;			/* SIM entry for the cam_conftbl[] */
    U32 bus_id;			/* path ID for the entry */
{
    U32 ccfg_initialize();		/* for setting up local/XPT modules */
    U32 ccfg_edtscan(); 		/* for INQUIRY scanning of the bus */
    EDT *ccfg_alloc_edt();		/* allocates a EDT structure */ 

    I32 j, k;				/* loop counters */
    CAM_SIM_ENTRY *cse;			/* pointer for SIM entries */
    EDT *eg;				/* pointer for the EDT grid */
    CAM_EDT_ENTRY *ee;			/* for accessing the EDT elements */
    int s;				/* for the SMP/IPL locking */

    PRINTD( bus_id, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	("[%d/t/l] ccfg_simattach: Enter.\n", bus_id));

    /* Check the ready flag for the subsystem in the event that this is the 
    first call the CDrv.  If the flag is FALSE call the initialize routine
    to handle the local variables and the XPT module. */

    if( cam_subsystem_ready != CAM_TRUE )	/* make sure it not ready */
    {
	PRINTD( bus_id, NOBTL, NOBTL, (CAMD_FLOW | CAMD_CONFIG),
	("[%d/t/l] ccfg_simattach: subsysem not ready, calling local init\n",
	bus_id));

	if( ccfg_initialize() == CAM_FAILURE )
	{
	    return( CAM_FAILURE );		/* Signal nothing worked */
	}
    }

    /* Check out the cam_conftbl[] structure, if the selected entry is empty
    load the SIM_ENTRY pointer and call the (*sim_init)() routine. */

    if( bus_id >= N_cam_conftbl )		/* is there room ? */
    {
	return( CAM_FAILURE );		/* Signal nothing done */
    }

    XCTRL_IPLSMP_LOCK( s, &cam_conf_ctrl );	/* lock the cam_conftbl[] */

    if( cam_conftbl[ bus_id ] != NULL )
    {
	/* Check to see if the entry is the XPT "place holder".  If the
	entry is being held by the XPT, it is exptecting to have this
	entry over write the place holder.  This allows the XPT
	bus_{de,}register() routines to "play" with the cam_conftbl[]. */

	if( cam_conftbl[ bus_id ] != &xpt_sim_entry )
	{
	    /* There already is a valid SIM entry for the path ID. */

	    XCTRL_IPLSMP_UNLOCK( s, &cam_conf_ctrl );	/* release the lock */

	    CAM_ERROR( ccfg_simattach_func,
		"Attach Request For An Already Existing SIM Entry.",
		(CAM_ERR_SEVERE), bus_id, cam_conftbl, (void *)NULL ); 

	    PRINTD( bus_id, NOBTL, NOBTL, (CAMD_ERRORS | CAMD_CONFIG),
		("[%d/t/l] ccfg_simattach: existing SIM entry for bus id\n",
		bus_id) );

	    return( CAM_FAILURE );		/* Signal nothing done */
	}
    }

    /* Update the table and call the SIM init routine with it's pathid
    and verify it's return status value. */

    cam_conftbl[ bus_id ] = se;		/* formally put it in the table */
    cse = cam_conftbl[ bus_id ];	/* formally copy it out */

    XCTRL_IPLSMP_UNLOCK( s, &cam_conf_ctrl );	/* release the lock */

    PRINTD( bus_id, NOBTL, NOBTL, (CAMD_FLOW | CAMD_CONFIG),
	("[%d/t/l] ccfg_simattach: calling SIM init via cam_conftbl[]\n",
	bus_id));

    if( cse->sim_init != NULL )
    {
	/* The SIM routines use the same defines from the CAM spec. for thier
	function return values. */

	if( (*cse->sim_init)( bus_id ) == CAM_REQ_CMP_ERR )
	{
	    CAM_ERROR( ccfg_simattach_func,
		"SIM Attach Failed, sim_init() reported FAILURE.",
		(CAM_ERR_SEVERE), bus_id, cam_conftbl, (void *)NULL ); 

	    return( CAM_FAILURE );
	}
    }
    else
    {
	/* For the indexed entry there is no valid init routine. */

	PRINTD( bus_id, NOBTL, NOBTL, (CAMD_ERRORS | CAMD_CONFIG),
	    ("[%d/t/l] ccfg_simattach: no valid sim_init\n", bus_id) );
    }

    /* Allocate the EDT grid for the bus, put it into the directory array.
    Run the scan to fill it.  It is ASSUMED that if the code has made it
    to here going through the cam_conftbl[] checks that the slot in the
    EDT directory array is available. */

    eg = ccfg_alloc_edt();		/* one EDT per HBA */

    if( eg == (EDT *)NULL )		/* Failure on allocation */
    {
	CAM_ERROR( ccfg_simattach_func,
	    "Memory Allocation Failure, on EDT Request.",
	    (CAM_ERR_SEVERE), (U32)NO_ERRVAL, (void *)NULL, (void *)NULL ); 
	return( CAM_FAILURE );
    }

    eg->edt_flags = 0;			/* clear out the flag bits */
    eg->edt_scan_count = 0;		/* clear out the counter */
    EDT_INIT_LOCK( eg );		/* init the SMP lock */

    for( j = 0; j < NDPS; j++ )
    {
	for( k = 0; k < NLPT; k++ )
	{
	    ee = &(eg->edt[j][k]);

	    ee->cam_tlun_found = CAM_FALSE;		/* no target yet */
	    ee->cam_ainfo = NULL;			/* no async list */
	    ee->cam_owner_tag = 0;			/* not used */
	    bzero( ee->cam_inq_data, INQLEN );		/* 0 fill the array */
	}
    }

    /* After the initialization of the allocated EDT grid for the bus,
    put it into the directory array. */

    edt_dir[ bus_id ] = eg;		/* now it can be used by others */

    /* Call the scanning code to fill in the EDT for this bus. */

    PRINTD( bus_id, NOBTL, NOBTL, (CAMD_FLOW | CAMD_CONFIG),
	("[%d/t/l] ccfg_simattach: SIM ready, calling EDT scanning code\n",
	bus_id));

    if( ccfg_edtscan( (I32)EDT_FULLSCAN, (I32)bus_id, (I32)0, (I32)0 )
	!= CAM_REQ_CMP )
    {
	return( CAM_FAILURE );		/* scan failed */
    }

    PRINTD( bus_id, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	("[b/t/l] ccfg_simattach: Exit.\n", bus_id));

    return( CAM_SUCCESS );		/* The attach went well */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_slave()

Functional Description :

Configuration Driver Slave Routine.
     This routine is called at auto-configuration time to determine
if a slave exists or not.  During the auto-configuration process,
this routine locates the configured driver in the peripheral driver
table.  If located, the peripheral drivers' slave routine is called
with the standard arguments, to allow it to perform its' own slave
initialization.

OSF Formal Parameters :

    d = Pointer to the "device" structure information.

ULTRIX Formal Parameters : 

    ui = Pointer to unit information structure.
    csr = Virtual address of the CSR address.

Implicit Inputs :

    The peripheral driver lookup table, cam_peripheral_drivers.

Implicit Outputs : None

Return Value : 
    Returns 0 / 1 = Slave isn't alive / Slave is alive.

Side Effects : TBD

Additional Information :
    This routine is only needed for ULTRIX configuration process and won't
be required for OSF configuration.

*/

int
ccfg_slave (attach, csr)
struct device *attach;
caddr_t csr;
{
    register struct cam_peripheral_driver *cpd;
    register int i;

    PRINTD(attach->ctlr_num, NOBTL, NOBTL,(CAMD_INOUT|CAMD_CONFIG),
	("[%d/t/l] ccfg_slave: Enter.\n", attach->ctlr_num));

    for(cpd = cam_peripheral_drivers, i = 0; i < cam_pdrv_entries; cpd++, i++)
    {
	if (strcmp (cpd->cpd_name, attach->dev_name) == 0)
	{
	    return ((*cpd->cpd_slave)(attach, csr));
	    /*NOTREACHED*/
	}
    }
    return (0);
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_attach()

Functional Description :

Configuration Driver Attach Routine.

    This routine is called by the auto-configuration code after a 
slave has been found.  The purpose of this routine is to locate the
configured driver in the peripheral driver table.  Once located,
the peripheral drivers' attach routine is called with the standard
arguments, to allow it to perform its' own attach initialization.

OSF Formal Parameters :

    d = Pointer to the "device" structure information.

ULTRIX Formal Parameters : 

    ui = Pointer to unit information structure.

Implicit Inputs : 

    The peripheral driver lookup table, cam_peripheral_drivers.

Implicit Outputs : None

Return Value :

    Returns 0 / 1 = Attach Failed / Attach Successful.
    Note: The return value is ignored by the system auto-configuration code.

Side Effects : TBD

Additional Information :

    This routine is only needed for ULTRIX configuration process and
won't be required for OSF configuration

*/

int
ccfg_attach (attach)
struct device *attach;
{
    register struct cam_peripheral_driver *cpd;
    register int i;

    PRINTD(attach->ctlr_num, NOBTL, NOBTL, (CAMD_INOUT|CAMD_CONFIG),
	("[%d/t/l] ccfg_attach: Enter.\n", attach->ctlr_num));

    for(cpd = cam_peripheral_drivers, i = 0; i < cam_pdrv_entries; cpd++, i++)
    {
	if(strcmp (cpd->cpd_name, attach->dev_name) == 0)
	{
	    return ((*cpd->cpd_attach)(attach));

	    /*NOTREACHED*/
	}
    }
    return (0);
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_action()

Functional Description :
This routine is called with a CCB that has anything to do with the
cam_edt[].  It is called from the xpt_action() routine.  Using the
opcode in the CCB header it will call the internal routines that deal
with the particular CCBs.  The CAM function codes supported here are,
XPT_GDEV_TYPE, XPT_SASYNC_CB, and XPT_SDEV_TYPE.  The internal routines
used to support this are, ccfg_gettype(), ccfg_setasync(), and ccfg_settype()
respectivly.

Formal Parameters :
A pointer to the CCB header.

Implicit Inputs :
The CAM cam_edt[] data structure.

Implicit Outputs :
The CAM cam_edt[] data structure.

Return Value :
    A valid CAM status also loaded into the CAM status field.

Side Effects :
The cam_edt[] may be updated, depending on the particular CCB.

Additional Information :
It is expected that a series of cascadding "if"s will be used to give priority
to the GET DEVICE TYPE CCB call.
*/

U32
ccfg_action( ch )
    CCB_HEADER *ch;
{
    PRINTD( ch->cam_path_id, ch->cam_target_id, ch->cam_target_lun, CAMD_INOUT,
	("[%d/%d/%d] ccfg_action: Enter, opcode: %d.\n", ch->cam_path_id,
	ch->cam_target_id, ch->cam_target_lun, ch->cam_func_code));

    /* Validate the pathid.  If the pathid in the header is too large, past
    the end of the cam_conftbl[] structure return the error. */

    if( ch->cam_path_id >= N_cam_conftbl )	/* invalid path id */
    {
	ch->cam_status = CAM_PATH_INVALID;
	return( CAM_PATH_INVALID );
    }

    /* Check out the opcode in the header and call the correct routine to
    deal with the CCB. */

    if( ch->cam_func_code == XPT_GDEV_TYPE )
    {
	return( ccfg_gettype( (CCB_GETDEV *)ch ) );
    }
    else if( ch->cam_func_code == XPT_SDEV_TYPE )
    {
	return( ccfg_settype( (CCB_SETDEV *)ch ) );
    }
    else if( ch->cam_func_code == XPT_SASYNC_CB )
    {
	return( ccfg_setasync( (CCB_SETASYNC *)ch ) );
    }

    /* In the event of a fall through, return CAM_REQ_INVALID. */

    ch->cam_status = CAM_REQ_INVALID;
    return( CAM_REQ_INVALID );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_callback()

Functional Description :
This routine is used to support the scanning of the SCSI buses.  When
an INQUIRY CCB goes down to the SIM this routine is the callback
handler.  The SIM issues the CCB callback to here.  This rouitne has
little to do with the interpretation of the CCB results.  It will set
the CCB_RECEIVED flag in the working set.  It will also check the
ISSUE_WAKEUP to see if the scanning code is currently sleeping waiting
for this CCB to complete.  If the ISSUE_WAKEUP bit is set the callback
routine will call the wakeup routine on the working set.  Along with
setting the CCB_RECEIVED flag the callback routine is also responsible
for releasing the SIM queue in the event of a non-good CAM status.  The
release is done here because the assumption is that for the EDT INQUIRY
scan there will be no error recovery.  The SIM Q frozen bit in the CAM
status field is also cleared, when the code that is waiting for the
CCB_RECEIVED signal it will not have to worry about the SIM Q needing
to be released.

Formal Parameters :
A CCB pointer for the INQUIRY SCSI I/O CCB.

Implicit Inputs :
None

Implicit Outputs :
Setting the CCB_RECEIVED bit in the working set.

Return Value :
None

Side Effects :
The SIM Q may be frozen and released.  This allows the SIM Q to be returned
to it's state from before the INQUIRY CCB.

Additional Information :
The PDrv working set is used as part of the callback handling.
*/

static void
ccfg_callback( cio )
    CCB_SCSIIO *cio;		/* the completed INQUIRY CCB */
{
    PDRV_WS *pws;		/* pointer for the working set */
    CCB_RELSIM *rsq;		/* pointer for the release SIM Q CCB */
    I32 cflags;		/* temp copy of the working set flags */
    int s;			/* for the IPL settings */


    PRINTD( cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	cio->cam_ch.cam_target_lun, CAMD_INOUT,
	("[%d/%d/%d] ccfg_callback: Enter, IO CCB: 0x%x.\n",
	cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	cio->cam_ch.cam_target_lun, cio));

    /* First things first, check to see if the SIM Q is frozen.  If it is
    send the release CCB down.  Let the Q get back to normal, there will
    be no error recovery. */

    if( (cio->cam_ch.cam_status & CAM_SIM_QFRZN) != 0 )
    {
	PRINTD( cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	    cio->cam_ch.cam_target_lun, (CAMD_FLOW | CAMD_ERRORS),
	   ("[%d/%d/%d] ccfg_callback: IO CCB: 0x%x, SIM Q Fzn Status: 0x%x.\n",
	    cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	    cio->cam_ch.cam_target_lun, cio, cio->cam_ch.cam_status));

	/* Get a CCB from the XPT, copy the B/T/L information and relesase
	the Q. */

	rsq = (CCB_RELSIM *)xpt_ccb_alloc();	/* get a raw CCB */

	rsq->cam_ch.cam_ccb_len = sizeof( CCB_RELSIM );	/* how many bytes */
	rsq->cam_ch.cam_func_code = XPT_REL_SIMQ;	/* what to do */
	rsq->cam_ch.cam_status = 0;			/* not checked */
	rsq->cam_ch.cam_path_id    = cio->cam_ch.cam_path_id;
	rsq->cam_ch.cam_target_id  = cio->cam_ch.cam_target_id;
	rsq->cam_ch.cam_target_lun = cio->cam_ch.cam_target_lun;
	rsq->cam_ch.cam_flags = 0;			/* no flags needed */

	/* Send the CCB to the XPT and the SIM.  When it returns the SIM
	Q is released and the CCB can be returned to the XPT. */

	(void)xpt_action( (CCB_HEADER *)rsq );	/* issue the CCB */
	xpt_ccb_free( rsq );		/* return the CCB to the pool */

	/* Clear the original CCB's SIM Q Frozen bit in the CAM status.
	The Q is nolonger frozen. */

	cio->cam_ch.cam_status &= ~(CAM_SIM_QFRZN);
    }

    /* Now that the CCB has completed, cam_status/AUTOSENSE/INQUIRY data,
    simply set the CCB_RECEIVED bit in the working set to indicate that
    it is back. */

    pws = (PDRV_WS *)cio->cam_pdrv_ptr;		/* get the working set */

    PRINTD( cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	cio->cam_ch.cam_target_lun, CAMD_FLOW,
   ("[%d/%d/%d] ccfg_callback: IO CCB: 0x%x, signaling RECEIVED, flags 0x%x.\n",
	cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	cio->cam_ch.cam_target_lun, cio, pws->pws_flags));

    QHEAD_IPLSMP_LOCK( s, &ccfg_qhead );	/* lock on the Q struct */
    cflags = pws->pws_flags;			/* grab a local copy */

    pws->pws_flags |= CCB_RECEIVED;		/* signal it's there */
    QHEAD_IPLSMP_UNLOCK( s, &ccfg_qhead );

    /* Check to local copy of the working set flags to see if the scanning
    code is waiting for a wakeup() call. */

    (void)wakeup( pws );		/* the sleeper must awaken */

    /* All done, now simply return back to the caller. */

    return;
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_initialize()

Functional Description :

This routine is responsible for the initialization of the CDrv, XPT,
and PDrv/SIM lookup lists in the CAM subsystem.  The CDrv will
initialize the local variables, then call the XPT, and then call the
initialization routines for the lists.

Formal Parameters :

At this time it is not sure just what will be passed as formal parameters.
The system level configuration process is *** TDB ***.  For now there will
be no arguments.

Implicit Inputs :

The xpt_initialize(), and init list  routines.

Implicit Outputs :

The global cam_subsystem_ready flag is set to TRUE or FALSE, depending on the
success of the *_init() calls.

Return Value :
    A long value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :

    There are many, all the initialization points in the modules are called.
The SMP lock structure are initialized.

Additional Information :
*/

static char *ccfg_initialize_func = "ccfg_initialize()";	/* Func name */

U32
ccfg_initialize()
{
    PDRV_WS *cqh;			/* ptr for the CCB queue header */

    PRINTD(NOBTL, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	("[b/t/l] ccfg_initialize: Enter.\n"));

    /* Initialize the CDrv local variables. */

    cam_subsystem_ready = CAM_FALSE;	/* make sure it not ready */

    /* Initialize the Q Head structure. */

    cqh = &ccfg_qhead.qws;	/* set the ptr to the working set */

    cqh->pws_flink = NULL;	/* NULL pointer : empty queue */
    cqh->pws_blink = NULL;	/* NULL pointer : empty queue */
    cqh->pws_ccb = NULL;	/* the head does not store a CCB */
    cqh->pws_flags = 0;		/* clear all flags */
    cqh->pws_retry_cnt = 0;	/* use retry cnt for Q count */

    QHEAD_INIT_LOCK( &ccfg_qhead );	/* Init the SMP lock */

    /* Initialize the control structure. */

    ccfg_ctrl.ccfg_flags = 0;		/* controlling flags */
    CTRL_INIT_LOCK( &ccfg_ctrl );	/* init the SMP lock */

    /* Call the XPT initialize routine.  If the XPT modules returns
    CAM_FAILURE, the subsystem is concidered not ready. */

    if( xpt_initialize() == CAM_FAILURE )
    {
	CAM_ERROR( ccfg_initialize_func,
	    "CAM subsystem Error, XPT init() Reported Failure.",
	    (CAM_ERR_SEVERE), (U32)NO_ERRVAL, (void *)NULL, (void *)NULL );

	return( CAM_FAILURE );
    }

    /* Call the configuration list initalization routines. */

    init_cam_components();		/* setup the PDrv list */
    init_sim_components();		/* setup the SIM list */

    /* All has gone well, signal the readiness of the subsystem. */

    cam_subsystem_ready = CAM_TRUE;	/* set to signal it's all ready */
    return( CAM_SUCCESS );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_gettype()

Functional Description :
This routine is responsible for implementing the GET DEVICE TYPE CCB.  The 
inquiry information from the selected B/T/L from the cam_edt[] is passed to
the PDrv requesting the information.

Formal Parameters :
A pointer to the GET DEVICE TYPE CCB.

Implicit Inputs :
The cam_edt[] data structure.

Implicit Outputs : 
The inquiry data is returned to the PDrv.

Return Value :
A valid CAM status, also contained in the CAM status field.
The following CAM status values are returned:

CAM_REQ_CMP		The device was there and the return fields are valid.
CAM_DEV_NOT_THERE	At the selected path ID there is no EDT, or no
			Target/LUN was found.

Side Effects :
None

Additional Information :
At this time there is only support for the PDrv pointer for the inquiry 
information to be a kernel virtual address.
*/

U32
ccfg_gettype( cg )
    CCB_GETDEV *cg;
{
    EDT *eg;				/* for accessing the EDT struct */
    CAM_EDT_ENTRY *ee;			/* for accessing EDT elements */
    ALL_INQ_DATA *idata;		/* for the inquiry data */
    int s;				/* for IPL/Locking */

    PRINTD( cg->cam_ch.cam_path_id, cg->cam_ch.cam_target_id,
	cg->cam_ch.cam_target_lun, CAMD_INOUT,
	("[%d/%d/%d] ccfg_gettype: Enter IO CCB: 0x%x.\n",
	cg->cam_ch.cam_path_id, cg->cam_ch.cam_target_id,
	cg->cam_ch.cam_target_lun, cg));

    /* Access the EDT via the directory array.  If the found flag is not set
    then no target/LUN was detected during the initial boot scan. */

    if( (eg = edt_dir[ cg->cam_ch.cam_path_id ]) == NULL )
    {
	cg->cam_ch.cam_status = CAM_DEV_NOT_THERE;	/* No HBA/SIM */
	return( CAM_DEV_NOT_THERE );
    }

    EDT_IPLSMP_LOCK( s, eg );			/* lock on the EDT */

    ee = &(eg->edt[cg->cam_ch.cam_target_id][cg->cam_ch.cam_target_lun]);

    /* Check the EDT entry.  If the entry is valid, update the device type
    field and copy the inquiry information to the PDrv. */

    if( ee->cam_tlun_found == CAM_FALSE )
    {
	EDT_IPLSMP_UNLOCK( s, eg );			/* remove the lock */
	cg->cam_ch.cam_status = CAM_DEV_NOT_THERE;	/* No target/LUN */
	return( CAM_DEV_NOT_THERE );
    }

    /* Copy the Device type bits from the Inquiry data field. */
    cg->cam_pd_type = ((ALL_INQ_DATA *)(&ee->cam_inq_data[0]))->dtype;

    if( cg->cam_inq_data != NULL )
    {
	/* Copy the stored INQUIRY data to the area pointed to in the CCB. */

	bcopy( ee->cam_inq_data, cg->cam_inq_data, INQLEN );
    }

    /* Signal all went well. */

    EDT_IPLSMP_UNLOCK( s, eg );			/* remove the lock */
    cg->cam_ch.cam_status = CAM_REQ_CMP;	/* All done */
    return( CAM_REQ_CMP );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_setasync()

Functional Description :
This routine is responsible for implementing the SET ASYNC CALLBACK
CCB.  The flags and callback handler address is stored in the cam_edt[]
structure.  Multiple PDrvs registering for the same B/T/L will have the
information stored in linked lists of ASYNC_INFO structures.  This same
routine can "de-register" and previously registered PDrv, provided that
the passed handler address matches the one in the CCB with no flags
enabled.

Formal Parameters :
A pointer to the SET ASYNC CALLBACK CCB.

Implicit Inputs :
The CAM cam_edt[] data structure.

Implicit Outputs :
The CAM cam_edt[] data structure will be updated.

Return Value :
A valid CAM status, also contained in the CAM status field.

Side Effects :
If a PDrv sends down another SET ASYNC CALLBACK CCB for the same
bus/target/lun the previous event flags will be superceeded, replaced,
by the new settings in the CCB just received.  This will allow a PDrv
to "de-register" a couple of events without removing it's entry
initially.

Additional Information :
    The callback handler address is the only link the CDrv has to the
different PDrvs.  The handler address is used to detect the same PDrv
changing it's previous event flag registration.  If a PDrv uses
differet handlers to deal with the different events, the matching
handler must be used to make any changes to the event flag(s).  There
is some limited error checking done on the CCB.  The pointer field must
contain a non-NULL value.  And if any of the enabled bits require data
to be passed back to the PDrv the buffer pointer and length is also
checked. (JAG-V1: not checked)

    This routine will pre-alloc the ASYNC structure, incase it needs it.
At the end if the block was not needed it will free the block.  This is
kind of round about, however it gets around the allocation w/in a lock
unlock.

    In the event that the XPT is in the process of scanning the EDT
during async callbacks, the ASYNC_CB_INPROG bit in the EDT is set,
this routine will sleep until the XPT finishes.  */

U32
ccfg_setasync( ca )
    CCB_SETASYNC *ca;
{
    volatile EDT *eg;			/* for accessing the EDT struct */
    volatile CAM_EDT_ENTRY *ee;		/* for accessing EDT elements */
    ASYNC_INFO *p, *q;			/* pointers for info list */
    ASYNC_INFO *ap;			/* pointer for allocated packet */
    I32 free_ap = CAM_FALSE;		/* flag for "freeing" the space */
    int s;				/* for IPL/Locking */

    PRINTD( ca->cam_ch.cam_path_id, ca->cam_ch.cam_target_id,
	ca->cam_ch.cam_target_lun, CAMD_INOUT,
	("[%d/%d/%d] ccfg_setasync: Enter IO CCB: 0x%x.\n",
	ca->cam_ch.cam_path_id, ca->cam_ch.cam_target_id,
	ca->cam_ch.cam_target_lun, ca));

    /* First check the CCB for a valid callback handler address.  Any address
    that is not NULL is valid.  If the field is NULL return a CAM status
    of CAM_REQ_CMP_ERR is returned. */

    if( ca->cam_async_func == NULL )
    {
	ca->cam_ch.cam_status = CAM_REQ_CMP_ERR;	/* bad handler addr */
	return( CAM_REQ_CMP_ERR );
    }

    /* Make sure that this routine is not called on the Interrupt stack.  It
    needs to allocate a block of memory. */

    if( cam_inisr() )
    {
	ca->cam_ch.cam_status = CAM_PROVIDE_FAIL;	/* can't do it now */
	return( CAM_PROVIDE_FAIL );
    }

    /* Access the EDT via the directory array.  There is no need to check for
    the found flag.  It is possible that a PDrv will register on a non-found
    ID instead of all of them inorder to get notification of an event only
    once. */

    if( (eg = edt_dir[ ca->cam_ch.cam_path_id ]) == NULL )
    {
	ca->cam_ch.cam_status = CAM_NO_HBA;	/* No HBA/SIM */
	return( CAM_NO_HBA );
    }

    /* Pre-alloc the ASYNC info structure.  Check the async flags to see if
    the PDrv wishes to de-register, there will be no need to get a packet. */

    if( ca->cam_async_flags != 0 )	/* this is a register request */
    {
	ap = ccfg_alloc_async();	/* get the packet, NULL check later */
    }

    EDT_IPLSMP_LOCK( s, eg );			/* lock on the EDT */

    /* Check the ASYNC_CB_INPROG flag.  If the XPT is using the EDT for 
    an async callback request this routine can not disturb any of the 
    async info lists.  The process will be put to sleep and wait for the
    XPT to finish. */

    while( (eg->edt_flags & ASYNC_CB_INPROG) != 0 )
    {
	/* There is a XPT Async calback scan in progress. */
	EDT_SMP_SLEEPUNLOCK( eg, PRIBIO, eg );
	EDT_SMP_LOCK( eg );		/* relock on the control struct */
    }

    /* Locate the EDT element selected and start the work on the info list */

    ee = &(eg->edt[ca->cam_ch.cam_target_id][ca->cam_ch.cam_target_lun]);

    /* Walk along the Async info linked list, watch for an callback address
    match on the callback request.  If a match is found also break out of
    the loop. */

    q = NULL;
    p = ee->cam_ainfo;		/* set the pointers */

    while( p != NULL )
    {
	if( p->cam_async_func == ca->cam_async_func )	/* same async routine */
	    break;

	q = p;					/* save current */
	p = p->cam_async_next;			/* get next */
    }

    /* After falling out of the loop the pointers are at the place where
    some work has to be done, (this could also be at the beginning.
    If a match is found check to see what event flags the PDrv wishes
    to enable.  The PDrv can also "de-register" by not setting any of
    the event flags. */

    if( ca->cam_async_flags == 0 )	/* de-register the PDrv */
    {
	/* If p == NULL, just return, the PDrv can consider itself 
	de-registered.  Otherwise the current ASYNC_INFO structure has to
	be removed. */

	if( p != NULL )
	{
	    /* Where on the list is it, check for making it empty. */

	    if( q == NULL )		/* at the front */
	    {
		ee->cam_ainfo = p->cam_async_next;	/* pop off front */
	    }
	    else			/* inside the list */
	    {
		q->cam_async_next = p->cam_async_next;	/* pop it */
	    }

	    /* The ap pointer can be "over-used" here.  In this if()
	    code path there was no ASYNC_INFO structure pre-allocated.
	    By using the ap pointer and the free_ap flag the common
	    "free" code at the bottom can be used. */

	    ap = p;			/* same the ptr to "free" at the end */
	    free_ap = CAM_TRUE;		/* signal to call de-alloc */
	}
    }
    else				/* update the EDT information */
    {
	/* If p points to a valid structure, replace the previous event flags,
	else a new structure has to be added to the list. */

	if( p != NULL )			/* replace the current settings  */
	{
	    p->cam_event_enable = ca->cam_async_flags;
	    free_ap = CAM_TRUE;		/* didn't need the packet */
	}
	else				/* add to the end */
	{
	    /* Now check the packet pointer for NULL, if the
	    pre-allocation failed don't attempt to use the space and fail
	    the request. */

	    if( ap != NULL )
	    {
		/* Use the pre-allocated storage for a new ASYNC_INFO
		structure.  Move the information from the CCB into it
		and add it to the list. */

		ap->cam_async_next = NULL;			/* new end */
		ap->cam_event_enable = ca->cam_async_flags;	/* events */
		ap->cam_async_func = ca->cam_async_func;	/* handler */
		ap->cam_async_blen = ca->pdrv_buf_len;		/* buffer len */
		ap->cam_async_ptr = ca->pdrv_buf;		/* buffer loc */

		/* Put the new registration at the end of the list. */

		if( q == NULL )		/* The first one */
		{
		    ee->cam_ainfo = ap;
		}
		else			/* at the end */
		{
		    q->cam_async_next = ap;
		}

		free_ap = CAM_FALSE;	/* don't free it, it's in use */
	    }
	    else			/* signal allocation failure */
	    {
		EDT_IPLSMP_UNLOCK( s, eg );		/* remove the lock */
		ca->cam_ch.cam_status = CAM_PROVIDE_FAIL; /* had an error */
		return( CAM_PROVIDE_FAIL );
	    }
	}
    }

    /* Signal all went well. */

    EDT_IPLSMP_UNLOCK( s, eg );			/* remove the lock */

    /* Check the free_ap flag, if set call the de-alloc code to release the
    packet.  It is no longer needed, the system alloc code can be called 
    after the SMP lock is released. */

    if( free_ap == CAM_TRUE )
    {
	ccfg_free_async( ap );	/* free up the nolonger needed space */
    }

    ca->cam_ch.cam_status = CAM_REQ_CMP;	/* All done */
    return( CAM_REQ_CMP );
}
/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_settype()

Functional Description :
This routine is responsible for implementing the SET DEVICE TYPE CCB.  The 
inquiry information-the device type byte, in the selected B/T/L entry in
the cam_edt[] is updated to the value passed by the PDrv.  The inquiry data
array is cleared to NULL.

Formal Parameters :
A pointer to the SET DEVICE TYPE CCB.

Implicit Inputs :
The cam_edt[] data structure.

Implicit Outputs : 
The inquiry data is modified to the value passed by the PDrv.

Return Value :
A valid CAM status, also contained in the CAM status field.

The following CAM status values are returned:

CAM_REQ_CMP_ERR		There is no available EDT to update.
CAM_REQ_CMP		The EDT is updated.

Side Effects :
There could be many or there could be none.  The CDrv will allow a PDrv to
override what is in the cam_edt[] strucuture.  This could effect any subsequent
GET DEVICE TYPE queries.

Additional Information :
It is expected that a bit in the flags field will be used to indicate that the
inquiry data stored in the cam_edt[] can be suspect.  Also the Inquiry string
needs to be signaled as nolonger valid.

What should be done on a SET call to a device that was not found during the
initial scan ?  Perhaps a "trust me" bit also in the flags field ?
*/

U32
ccfg_settype( cs )
    CCB_SETDEV *cs;
{
    EDT *eg;				/* for accessing the EDT struct */
    CAM_EDT_ENTRY *ee;			/* for accessing EDT elements */
    int s;				/* for IPL/Locking */

    PRINTD( cs->cam_ch.cam_path_id, cs->cam_ch.cam_target_id,
	cs->cam_ch.cam_target_lun, CAMD_INOUT,
	("[%d/%d/%d] ccfg_settype: Enter IO CCB: 0x%x.\n",
	cs->cam_ch.cam_path_id, cs->cam_ch.cam_target_id,
	cs->cam_ch.cam_target_lun, cs));

    /* Access the EDT via the directory array.  If the found flag is not set
    then no target/LUN was detected during the initial boot scan. */

    if( (eg = edt_dir[ cs->cam_ch.cam_path_id ]) == NULL )
    {
	cs->cam_ch.cam_status = CAM_REQ_CMP_ERR;	/* No HBA/SIM space */
	return( CAM_REQ_CMP_ERR );
    }

    EDT_IPLSMP_LOCK( s, eg );			/* lock on the EDT */

    ee = &(eg->edt[cs->cam_ch.cam_target_id][cs->cam_ch.cam_target_lun]);

    /* Check the EDT entry.  It will not matter if the entry is valid,
    update the device found field and copy the device type information
    from the PDrv. */

    ee->cam_tlun_found = CAM_TRUE;		/* indicate a pseudo found */
    bzero( ee->cam_inq_data, INQLEN );	 	/* clear the INQUIRY space */

    ee->cam_inq_data[0] = (char)cs->cam_dev_type;  /* first byte is PTYPE */

    /* Signal the set has been done. */  

    EDT_IPLSMP_UNLOCK( s, eg );			/* remove the lock */
    cs->cam_ch.cam_status = CAM_REQ_CMP;	/* All done */
    return( CAM_REQ_CMP );
}

/* ---------------------------------------------------------------------- */

/*
Routine Name : ccfg_edtscan

Functional Description :

This routine is responsible for issuing SCSI inquiry commands to all the
SCSI targets and LUNs attached to the buses.  It uses the CAM subsystem
in the normal manner, the CCB is sent to the XPT and then to the SIM who 
"owns" the SCSI bus.  All the modules must be initialized and interrupts
for the system enabled.  The returned Inquiry data is stored in the EDT
structures.

Formal Parameters : 

The SCSI bus to scan, and the flag indicating the level of the scan.
There are two versions of the scan, a full scan or partial scan.  A
full scan will walk through the entire EDT sending an Inquiry command
to all the targets and LUNs.  A partial scan will only send an Inquiry
command to targets and LUNs that are flagged as "not found".

Implicit Inputs :

The EDT structure will be queried for not found targets, in the event of a 
partial scan.  If there is no EDT structure for a bus, this code will not
allocate and fill one.  This code requires that the EDT structures already
be setup and ready to be filled.

Implicit Outputs :

The EDT structures will be updated with the Inquiry information and the
cam_tlun_found flag set.

Return Value :
    A long value, indicating CAM_SUCCESS or CAM_FAILURE.

Side Effects :

A full scan has the potential to "un-find" all the devices that were set
by the PDrvs using the XPT_SETTYPE CCB.

Additional Information :

All the SCSI I/O - Inquiry CCBs are sent with Autosense enabled.
According to the SCSI specs issuing an Inquiry command to a device
should not affect any pending UNIT ATTENTION information.  The scanning
behavior for targets and LUNs will be consistant.  All potential SCSI
IDs will be selected and LUN 0 will be the initial LUN queried.  If an
ID responds to selection all LUNs, 0 - 7 will be queried.  It is not
assumed that all the LUNs in an ID are populated.  However it is
assumed that if an ID does not respond to selection that there are no
LUNs there also. 

TO DO:
 1) Retry the ccfg_getinq() calls on other failures.
*/

U32
ccfg_edtscan( scan_type, bus, target, lun  ) 
    I32 scan_type;			/* type of scan FULL/PARTIAL/SINGLE */
    I32 bus;				/* bus number for EDT grid */
    I32 target;			/* the target number */
    I32 lun;				/* the lun number */
{
    EDT *eg;				/* for accessing the EDT struct */
    CAM_EDT_ENTRY *ee;			/* for accessing EDT elements */
    int s;				/* for IPL/Locking */
    U32 scan_status;			/* holder for return value */
    U32 sim_id;			/* the initiator ID of the SIM */
    U32 new_dev_found;		/* signal for new "found" devices */

    PRINTD( bus, target, lun, (CAMD_INOUT | CAMD_CONFIG),
	("[%d/%d/%d] ccfg_edtscan: Bus scan type %d.\n",
	bus, target, lun, scan_type));

    /* Make sure that there is an EDT grid for the selected bus.  Both the
    bounds and contents of the directory is checked for.  */

    if( bus >= N_edt_dir )		/* is it past the end of the array */
    {
	return( CAM_PATH_INVALID );     /* Signal unable to scan the bus */
    }

    if( (eg = edt_dir[ bus ]) == NULL )
    {
	return( CAM_PATH_INVALID );	/* Signal unable to scan the bus */
    }

    /* If this routine is called while in Interrupt context fail the 
    request. */

    if( cam_inisr() )
    {
	return( CAM_PROVIDE_FAIL );	/* Signal unable to scan the bus */
    }

    /* Check the current scanning state flags in the control structure.  If
    there is a scan inprogress, call sleep() and wait for the current scan
    to complete and call wakeup(). */ 

    CTRL_IPLSMP_LOCK( s, &ccfg_ctrl );	/* lock on the control struct */

    while( (ccfg_ctrl.ccfg_flags & EDT_INSCAN) != 0 )
    {
	/* There is a scan already in progress. */
	CTRL_SMP_SLEEPUNLOCK( &ccfg_ctrl, PRIBIO, &ccfg_ctrl );
	CTRL_SMP_LOCK( &ccfg_ctrl );	/* relock on the control struct */
    }

    /* Set the scan bit, this routine is ready to do the scan.  Unlock
    the control structure the code now has the control structure used in
    the scan. */

    ccfg_ctrl.ccfg_flags |= EDT_INSCAN;	

    CTRL_IPLSMP_UNLOCK( s, &ccfg_ctrl );

    /* Query the SIM to get it's Initiator ID.  This ID will be skipped 
    during the bus scan.   If the XPT_PATH_ID CCB fails the returned ID
    will not be a valid SCSI ID, 0-7, the scan will still take place. */

    sim_id = ccfg_getsimid( bus );	/* Query the SIM */

    /* Now run through the SCAN to get/find all the devices out there. */

    new_dev_found = CAM_FALSE;		/* no devices found yet */
    scan_status = CAM_REQ_CMP;		/* initialize the return status */
    if( scan_type != EDT_SINGLESCAN )
    {
	/* Set the loops for a "full" scan.  The actual checking for
	the partial scan flag will be done just prior to the
	ccfg_getinq() call. */

	for( target = 0; target < NDPS; target++ )
	{
	    /* This Target loop has to skip the SIM's initiator ID. */

	    if( target == sim_id )
		continue;		/* go to the next target */

	    /* LUN 0, has some special conditions around it.  If it is
	    not found by the getinq() routine, it will be assumed that
	    there are no other LUNs 1 - 7, at that target.  The target
	    loop will continue to the next target. */

	    ee = &(eg->edt[ target ][ 0 ]);	/* get the element for LUN 0 */

	    /* Don't bother to send INQUIRY if PARTIAL and FOUND, just
	    keep going to the next LUN. */

	    if( !((scan_type == EDT_PARTSCAN) &&
		(ee->cam_tlun_found == CAM_TRUE)) )
	    {
		if( ccfg_getinq( bus, target, 0, eg, &new_dev_found )
		    == CAM_SEL_TIMEOUT )
		{
		    continue;	/* if not successfull skip to next target */
		}
	    }

	    /* Scan the rest of the LUNs for this target.  All the LUNs are
	    scanned to allow for holes in the LUNs. */

	    for( lun = 1; lun < NLPT; lun++ )
	    {
		ee = &(eg->edt[ target ][ lun ]); /* get the EDT element */

		if( (scan_type == EDT_PARTSCAN) &&
		    (ee->cam_tlun_found == CAM_TRUE) )
		{
		    continue;		/* skip to the next one */
		}

		/* check this B/T/L */
		(void)ccfg_getinq( bus, target, lun, eg, &new_dev_found );
	    }
	}
    }
    else			/* issue inquiry to a single nexus */
    {
	/* Check out only one B/T/L */

	scan_status = ccfg_getinq( bus, target, lun, eg, &new_dev_found );
    }

    /* Now that the scan in done, wakeup whomever may be waiting for
    another scan.  Clear the scanning bit and issue the wakeup() call. */

    CTRL_IPLSMP_LOCK( s, &ccfg_ctrl );	/* lock on the control struct */
    ccfg_ctrl.ccfg_flags &= ~(EDT_INSCAN);	/* signal completed */
    CTRL_IPLSMP_UNLOCK( s, &ccfg_ctrl );	/* unlock the ctrl structure */

    wakeup( &(ccfg_ctrl) );		/* wakeup anyone waiting */

    /* Check out the new device found flag that the ccfg_getinq() routine
    has updated if a new device was discovered during the scans. */

    if( new_dev_found == CAM_TRUE )
    {
	/* Call the XPT Async handler to notify any callers that new 
	devices were found. */

	(void)xpt_async( AC_FOUND_DEVICES, bus, ASYNC_WILDCARD, ASYNC_WILDCARD, 
	    (char *)NULL, (I32)0 );
    }

    return( scan_status );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_getinq

Functional Description :
This routine is responsible for issuing an SCSI inquiry command to the
selected SCSI bus, target, and LUN, passed as arguments.  This routine
will get a CCB, load it and send it to the subsystem.  It uses the CAM
subsystem in the normal manor.  All the modules must be initialized and
interrupts for the system enabled.  The returned Inquiry data is stored
in the EDT structure.  If the device that has responded to the inquiry
command was not found from a previous scan a CAM_TRUE value is stored
in the location indicated by the caller.

Formal Parameters : 
The bus, target, and LUN that are to be issued the INQUIRY command.
A pointer to the EDT grid that contains the B/T/L for locking.
A pointer to a BOOLEAN new device found variable. 

Implicit Inputs : None.

Implicit Outputs :
The EDT structures will be updated with the Inquiry information and the
cam_tlun_found flag set.  If there is no EDT structure at the selected
bus this routine will return failure.

Return Values :
    A long value with the following settings:

    CAM_REQ_CMP  		The inquiry went fine
    CAM_SEL_TIMEOUT		The selection failed, no device found
    Any thing else		Something bad happened

Side Effects :
This routine has the potential to "un-find" a device that was set
by the PDrvs using the XPT_SETTYPE CCB.

Additional Information :
The SCSI I/O - Inquiry CCB is sent with Autosence disabled.
According to the SCSI specs issuing an Inquiry command to a device
should not affect any of the sense information. 

Implementation Note: This routine can not be called while in ISR
context.  It uses code that will issue a sleep() call while waiting for
the INQUIRY CCB to complete. 
*/

static U32
ccfg_getinq( b, t, l, eg, nptr )
    I32 b;				/* the bus number */
    I32 t;				/* the target number */
    I32 l;				/* the LUN number */
    EDT *eg;				/* for accessing EDT elements */
    U32 *nptr;				/* pointer for the BOOLEAN flag */
{
    PDRV_WS *cqh;			/* for the CDrv Q head structure */
    PDRV_WS *pws;			/* for the working set */
    CCB_SCSIIO *cio;			/* for the I/O inquiry CCB */
    ALL_INQ_CDB *inq;			/* for the inquiry CDB */
    CAM_EDT_ENTRY *e;			/* for the T/L element */
    u_char rtn_cam_status;		/* to store the returned CAM status */
    extern U32 ccfg_use_sdtr_inquiry;	/* try sync or not */
    int s;				/* IPL value for the SMP macro */
    U32 prev_found;			/* temp storage for the old found */

    PRINTD( b, t, l, (CAMD_INOUT | CAMD_CONFIG),
	("[%d/%d/%d] ccfg_getinq: Enter.\n", b, t, l));

    /* Allocate a CCB from the xpt pool, and fill it in with the necessary
    values for the Inquiry command. */

    cio = (CCB_SCSIIO *)xpt_ccb_alloc();

    cio->cam_ch.cam_ccb_len = sizeof( CCB_SCSIIO );
    cio->cam_ch.cam_func_code = XPT_SCSI_IO;
    cio->cam_ch.cam_path_id = b;
    cio->cam_ch.cam_target_id = t;
    cio->cam_ch.cam_target_lun = l;

    /* If "ccfg_use_sdtr_inquiry" is not CAM_TRUE, then don't
     do the inquiry with the CAM_INITIATE_SYNC set.  Rev 1.12 */

    cio->cam_ch.cam_flags =
	(CAM_DIR_IN | CAM_DIS_AUTOSENSE | CAM_SIM_QFRZDIS |
	((ccfg_use_sdtr_inquiry == CAM_TRUE) ? CAM_INITIATE_SYNC : 0 ));

    cio->cam_cbfcnp = ccfg_callback;
    cio->cam_data_ptr = (u_char *)&ccfg_ctrl.inq_buf;
    cio->cam_dxfer_len = INQLEN;
    cio->cam_cdb_len = sizeof( ALL_INQ_CDB );
    inq = (ALL_INQ_CDB *)&cio->cam_cdb_io.cam_cdb_bytes[0];

    inq->opcode = ALL_INQ_OP;			/* inquiry command */
    inq->evpd = 0;				/* no product data */
    inq->lun = 0;				/* not used in SCSI-2 */
    inq->page = 0;				/* no product pages */
    inq->alloc_len = INQLEN;			/* for the EDT array */
    inq->control = 0;				/* no control flags */

    /* If the system is currently in probe context, do not run with 
    timeouts enabled.  Otherwise use the SIM default. */

    if( cam_at_boottime() )			/* w/in the boot state */
    {
	cio->cam_timeout = CAM_TIME_INFINITY;	/* disable timers */
    }
    else
    {
	cio->cam_timeout = CAM_TIME_DEFAULT;	/* use system default */
    }

    cio->cam_vu_flags = 0;			/* no VU stuff */
    cio->cam_tag_action = CAM_HEAD_QTAG;	/* just in case */
    cio->cam_req_map = NULL;			/* no associated bp */

    /* Get the working set. */

    pws = (PDRV_WS *)cio->cam_pdrv_ptr;		/* pre-filled wset ptr */

    /* The retry flag and retry count are cleared for the retry code. */

    pws->pws_flags &= ~(CCFG_RETRY);		/* not in retry state */
    pws->pws_retry_cnt = 0;			/* no retries yet 

    /* Set the pointer to the Q header's working set */

    cqh = &ccfg_qhead.qws;

    /* Attach the CCB/working set to the front of the pending Q. */

    QHEAD_IPLSMP_LOCK( s, &ccfg_qhead );
    pws->pws_flink = cqh->pws_blink;	/* attach to old front */
    if( pws->pws_flink != NULL )	/* incase the pending Q is empty */
    {
	(pws->pws_flink)->pws_blink = pws;/* attach the old front back ptr */
    }

    cqh->pws_blink = pws;		/* bump up the pending Q */
    cqh->pws_retry_cnt++;		/* increment the number pending pkts */
    QHEAD_IPLSMP_UNLOCK( s, &ccfg_qhead );

    /* The CCB is filled in and ready to go to the selected b/t/l.
    Start the retry loop for the scanning and send the CCB to the CAM
    subsystem and wait for it to complete. */

    while( ccfg_ok_toretry( pws ) )
    {
	/* Clear out the inquiry buffer in the control structure.  This will
	make sure that there are no residual bytes in the there. */

	bzero( &ccfg_ctrl.inq_buf, INQLEN );	/* 0 fill the area */

	/* Clear the CCB_RECEIVED and ISSUE_WAKEUP bits in the flags field.
	They will be used again in the ccfg_ccb_wait() routine. */

	pws->pws_flags &= ~(CCB_RECEIVED | ISSUE_WAKEUP);

	(void)xpt_action( (CCB_HEADER *)cio );	/* Send it down to the device */

	if( (pws->pws_flags & CCB_RECEIVED) == 0 )
	{
	    ccfg_ccb_wait( pws );		/* sleep or spin */
	}

	/* If the CCB completed fine, then break out of the loop and
	finish up the inquiry request. */

	if( cio->cam_ch.cam_status == CAM_REQ_CMP )
	{
	    break;				/* finish up */
	}

	PRINTD( b, t, l, (CAMD_INOUT | CAMD_CONFIG),
	    ("[%d/%d/%d] ccfg_getinq: Retry INQ. try # %d\n", b, t, l,
	    pws->pws_retry_cnt));
    }

    /* Now that the CCB has completed, good or otherwise, pop it off
    the Q and check out the returned CAM status to determine if the
    inquiry went ok. */

    QHEAD_IPLSMP_LOCK( s, &ccfg_qhead );
    
    if( pws->pws_blink == NULL )	/* at the front of the pending Q */
    {
	cqh->pws_blink = pws->pws_flink;	/* point to new front */
    }
    else
    {
	(pws->pws_blink)->pws_flink = pws->pws_flink;	/* normal middle */
    }

    if( pws->pws_flink != NULL )	/* not the end of the pending Q */
    {
	(pws->pws_flink)->pws_blink = pws->pws_blink;       /* normal middle */
    }
    cqh->pws_retry_cnt--;		/* decrement the number of pending */

    QHEAD_IPLSMP_UNLOCK( s, &ccfg_qhead );

    /* This routine will return three values, Found, Error condition, and
    selection timeout.  It can be possible to treat the timeout and any
    error condition as: "The device is not there". */

    e = &(eg->edt[ t ][ l ]);	/* get the EDT element */

    EDT_IPLSMP_LOCK( s, eg );			/* lock on the EDT */

    prev_found = e->cam_tlun_found;		/* save the old state */
    e->cam_tlun_found = CAM_FALSE;		/* assume no device */

    if( cio->cam_ch.cam_status == CAM_REQ_CMP )
    {
	/* All went ok copy the data from the control struct into the 
	selected EDT entry. */

	e->cam_tlun_found = CAM_TRUE;		/* found a device */

	/* Check the saved value of found, if it was CAM_FALSE then
	a new device was found.  Report it back to the caller via the
	passed pointer. */

	if( prev_found == CAM_FALSE )
	{
	    *nptr = CAM_TRUE;		/* signal a new devicd found */
	}

	/* 
	 * 1.06
	 * Fixed - make sure only copy what is actually transferred.
	 */
	bcopy( &ccfg_ctrl.inq_buf, e->cam_inq_data, (cio->cam_dxfer_len - 
			cio->cam_resid) );	/* store it */
    }

    EDT_IPLSMP_UNLOCK( s, eg );			/* remove the lock */

    /* Store the CAM status from the CCB, and return the CCB back to the
    XPT CCB Pool. */

    rtn_cam_status = cio->cam_ch.cam_status;	/* keep a local copy */
    xpt_ccb_free( cio );			/* give back the CCB */

    /* Simply return the CAM status, and let the calling code determine 
    what to do with it. */

    PRINTD( b, t, l, (CAMD_INOUT | CAMD_CONFIG),
	("[%d/%d/%d] ccfg_getinq: Exit, cam_status 0x%x.\n", b, t, l,
	rtn_cam_status));

    return( (U32)rtn_cam_status );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_ccb_wait

Functional Description :
This routine is responsible for dealing with the waiting issue during the
EDT scans.  Because a scan can occur during boot and run time this code
will need to wait two different ways.  During boot the DELAY() macro will
be used, and sleep() called when at run time.  This routine "returns" to
the scanning code once the INQUIRY CCB is back w/in the CDrv module.

Formal Parameters : 
A pointer to the working set for the INQUIRY CCB to wait for.

Implicit Inputs :
System support code/macros to indicate if the system is currently running
at "boot" time or "run" time.  The CCB_RECEIVED bit in the working set
flags, this bit is set by the callback routine when it is called.

Implicit Outputs :
The working set for the CCB may have the ISSUE_WAKEUP bit set.

Return Value :
None

Side Effects :
This routine may issue a sleep on the process, while waiting for the CCB to
return via the callback handler.  If the code is unable the issue a sleep()
call it will poll on the CCB_RECEIVED bit.

Additional Information :
OSF Porting issue - sleep/DELAY and checking for boot/run time.
*/

static void
ccfg_ccb_wait( pws )
    volatile PDRV_WS *pws;	/* the working set for the INQUIRY CCB */
{
    int s;			/* for IPL storage */
    I32 i;			/* loop counter */

    PRINTD( pws->pws_ccb->cam_ch.cam_path_id,
	pws->pws_ccb->cam_ch.cam_target_id,
	pws->pws_ccb->cam_ch.cam_target_lun, (CAMD_INOUT|CAMD_CONFIG),
	("[%d/%d/%d] ccfg_ccbwait: Enter, IO CCB: 0x%x.\n",
	pws->pws_ccb->cam_ch.cam_path_id, pws->pws_ccb->cam_ch.cam_target_id,
	pws->pws_ccb->cam_ch.cam_target_lun, pws->pws_ccb));


    /* Check to see what "state" the system is in, if at boot time take
    the DELAY() path else take the sleep() path. */

    if( cam_at_boottime() )			/* w/in the boot state */
    {

	/* Stay w/in a while loop waiting for the CCB_RECEIVED bit to be
	set by the callback handler. */

	for( i = 0; i < ccfg_wait_delay_loop; i++ )	/* a 2 second loop */
	{
	    if( (pws->pws_flags & CCB_RECEIVED) != 0 )
	    {
		break;
	    }
	    DELAY( WAIT_DELAY );		/* wait a bit */
	}

	/* Did the CCB come back or did it time out. */

	if( (pws->pws_flags & CCB_RECEIVED) == 0 ) /* is the SIM done */
	{
	    PRINTD( pws->pws_ccb->cam_ch.cam_path_id,
		pws->pws_ccb->cam_ch.cam_target_id,
		pws->pws_ccb->cam_ch.cam_target_lun,
		(CAMD_INOUT|CAMD_CONFIG|CAMD_ERRORS),
		("[%d/%d/%d] ccfg_ccbwait: timeout reached, IO CCB: 0x%x.\n",
		pws->pws_ccb->cam_ch.cam_path_id,
		pws->pws_ccb->cam_ch.cam_target_id,
		pws->pws_ccb->cam_ch.cam_target_lun, pws->pws_ccb));

	    ccfg_ccbtimeout( pws );		/* get the CCB back */
	}
    }
    else				/* w/in the running state */
    {
    	QHEAD_IPLSMP_LOCK( s, &ccfg_qhead );	/* lock on the Q struct */
	/* Set the ISSUE_WAKEUP bit, set the priority and go to sleep. */

	pws->pws_flags |= ISSUE_WAKEUP;

	while( (pws->pws_flags & CCB_RECEIVED) == 0 )
	{
	    /* The CCB has not yet returned. */
	    QHEAD_SMP_SLEEPUNLOCK( pws, PRIBIO, &ccfg_qhead );
	    QHEAD_SMP_LOCK( &ccfg_qhead );	/* relock on the Q struct */
	}

	/* Unlock the Q structure the CCB has arrived. */

	QHEAD_IPLSMP_UNLOCK( s, &ccfg_qhead );
    }

    /* By now the CCB has returned to the CDrv module, return back to the
    scanning code. */

    return;
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_getsimid

Functional Description :

This routine will use the CAM XPT_PATH_INQ CCB to query the SIM what it's 
Initiator ID is.  If the CCB fails in any way this routine will return a
-1 for an error condition.

Formal Parameters :

The SIM pathid to use in the XPT_PATH_INQ CCB.

Implicit Inputs : None

Implicit Outputs : None

Return Value : 

A valid SIM initiator ID, 0 - 7, or -1 for an failure status.

Side Effects : 

This routine will issue an alloced CCB to the selected SIM.  It is expected
that the SIM is already at a state where the XPT_PATH_INQ CCB will 
correctly work.

Additional Information :

*/

static U32
ccfg_getsimid( pathid )
    u_char pathid;			/* The Path ID for the selected SIM */
{
    CCB_PATHINQ *cpi;			/* for the path inquiry CCB */
    I32 saved_id;			/* the initiator id to return */

    PRINTD( pathid, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	("[%d/t/l] ccfg_getsimid: Enter.\n", pathid ));

    /* Allocate a CCB from the xpt pool, and fill it in with the necessary
    values for the Inquiry command. */

    cpi = (CCB_PATHINQ *)xpt_ccb_alloc();

    /* Fill in the CCB with the information needed to get the initiator id. */

    cpi->cam_ch.cam_ccb_len = sizeof( CCB_PATHINQ );
    cpi->cam_ch.cam_func_code = XPT_PATH_INQ;
    cpi->cam_ch.cam_path_id = pathid;

    cpi->cam_ch.cam_target_id = 0;		/* not needed */
    cpi->cam_ch.cam_target_lun = 0;		/* not needed */
    cpi->cam_ch.cam_flags = 0;			/* not needed */

    /* Now that the CCB is filled in send it to the CAM subsystem and wait
    for it to complete. */

    (void)xpt_action( (CCB_HEADER *)cpi );	/* Send it down to the device */

    /* Check the returned CAM status.  If the CCB failed then the "failed"
    ID, if not return the cam_initiator_id field. */

    if( cpi->cam_ch.cam_status != CAM_REQ_CMP )
    {
	PRINTD( pathid, NOBTL, NOBTL, (CAMD_ERRORS | CAMD_CONFIG),
	("[%d/t/l] ccfg_getsimid: PATHINQ CCB failed: CAM status 0x%x (%s)\n",
	pathid, cpi->cam_ch.cam_status,
	(char *)cdbg_CamStatus(cpi->cam_ch.cam_status, CDBG_BRIEF) ));

	saved_id = (-1);			/* save the error id */
    }
    else
    {
	saved_id = (I32)cpi->cam_initiator_id;	/* save the good id */
    }

    /* Return the CCB back to the pool. */

    xpt_ccb_free( (CCB_HEADER *)cpi );

    /* Return the saved SIM initiator ID to the caller. */

    PRINTD( pathid, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	("[%d/t/l] ccfg_getsimid: Exit, SIM ID %d\n", pathid, saved_id ));

    return( (U32)saved_id );			/* return the ID */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_ccbtimeout()

Functional Description :

This routine will do what ever is necessary to get the CCB back from the
SIM/SCSI BUS.  The caller routine, ccfg_getinq(), has timed out this 
CCB and now this code has to "get it back".  An XPT_ABORT CCB will be
issued and the CCB waited for.  If the CCB does not come back in time
a XPT_RESET_BUS CCB is issued.  After the ABORT/RESET it is assumed that
the CCB will have returned by then.  

Formal Parameters : 

    The Working set of the CCB that needs to be returned.

Implicit Inputs : None

Implicit Outputs :  None

Return Value : None

Side Effects : 

This routine will issue an ABORT to the device via the SIM.  It could also
issue a SCSI BUS RESET to the SIM.  The RESET could have some impact on 
the other devieces on the bus.

Additional Information :

Following the SCSI BUS RESET a 250 msec wait loop is entered.  This time
is needed to follow the SCSI-2 spec that defines 250 msecs for BUS
RESET to selection timing value.  So in the event the CCB does return
"immediately" there will be no bus activity until the 250 msec wait loop
has finished.

*/

static void
ccfg_ccbtimeout( pws )
    volatile PDRV_WS *pws;	/* the working set for the CCB to timeout */
{
    U32 i;			/* loop counter */
    CCB_HEADER *ch;		/* for the ABORT/RESET CCBs */

    PRINTD( pws->pws_ccb->cam_ch.cam_path_id,
	pws->pws_ccb->cam_ch.cam_target_id,
	pws->pws_ccb->cam_ch.cam_target_lun, (CAMD_INOUT|CAMD_CONFIG),
	("[%d/%d/%d] ccfg_ccbtimeout: Enter, IO CCB: 0x%x.\n",
	pws->pws_ccb->cam_ch.cam_path_id, pws->pws_ccb->cam_ch.cam_target_id,
	pws->pws_ccb->cam_ch.cam_target_lun, pws->pws_ccb));

    /* Allocate a CCB from the XPT to sent the ABORT. */

    ch = xpt_ccb_alloc();	/* get one from the pool */

    /* Copy the information from the header of the original CCB.  This
    information should not change for the rest of the routine. */

    ch->cam_path_id    = pws->pws_ccb->cam_ch.cam_path_id;	/* SIM */
    ch->cam_target_id  = pws->pws_ccb->cam_ch.cam_target_id;
    ch->cam_target_lun = pws->pws_ccb->cam_ch.cam_target_lun;

    /* Update the length and opcode fields. */

    ch->cam_func_code = XPT_ABORT;		/* first try an abort */
    ch->cam_ccb_len = sizeof( CCB_ABORT );	/* the right # of bytes */

    /* Load the address of the CCB from the working set. */

    ((CCB_ABORT *)ch)->cam_abort_ch = (CCB_HEADER *)pws->pws_ccb;

    PRINTD( ch->cam_path_id, ch->cam_target_id, ch->cam_target_lun,
	(CAMD_INOUT|CAMD_CONFIG|CAMD_ERRORS),
	("[%d/%d/%d] ccfg_ccbtimeout: Sending ABORT for IO CCB: 0x%x.\n",
	ch->cam_path_id, ch->cam_target_id, ch->cam_target_lun, pws->pws_ccb));

    /* Send the ABORT CCB to the SIM and wait for it to return. */

    pws->pws_flags |= ABORT_SENT;		/* keep track of events */
    (void)xpt_action( ch );			/* send it to the SIM */

    for( i = 0; i < ccfg_wait_delay_loop; i++ )	/* a 2 second loop */
    {
	if( (pws->pws_flags & CCB_RECEIVED) != 0 )
	{
	    break;
	}
	DELAY( WAIT_DELAY );		/* wait a bit */
    }

    /* Did the CCB come back or did it time out. */

    if( (pws->pws_flags & CCB_RECEIVED) != 0 )	/* has lassie come home ? */
    {
	xpt_ccb_free( ch );		/* return the ABORT CCB */
	return;				/* let the scanning code deal w/it */
    }

    /* The timeout loop counted down with out the CCB being returned.  The
    next step is to issue a SCSI bus reset to really make it come back. */

    /* Update the length and opcode fields. */

    ch->cam_func_code = XPT_RESET_BUS;		/* now try a reset */
    ch->cam_ccb_len = sizeof( CCB_RESETBUS );	/* the right # of bytes */

    PRINTD( ch->cam_path_id, ch->cam_target_id, ch->cam_target_lun,
	(CAMD_INOUT|CAMD_CONFIG|CAMD_ERRORS),
	("[%d/%d/%d] ccfg_ccbtimeout: Sending SCSI RESET for IO CCB: 0x%x.\n",
	ch->cam_path_id, ch->cam_target_id, ch->cam_target_lun, pws->pws_ccb));

    /* Send the RESET CCB to the SIM and wait for the CCB to return. */

    pws->pws_flags |= RESET_SENT;		/* keep track of events */
    (void)xpt_action( ch );			/* send it to the SIM */

    for( i = 0; i < WAIT_RESET_LOOP; i++ )	/* force a 250 msec loop */
    {
	DELAY( WAIT_DELAY );			/* wait a bit */
    }

    /* Now wait for the CCB to come back up from the SIM. */ 

    while( (pws->pws_flags & CCB_RECEIVED) == 0 )	/* where is lassie ? */
    {
	DELAY( WAIT_DELAY );			/* wait a bit */
    }

    xpt_ccb_free( ch );				/* return the ABORT/RESET CCB */
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_ok_toretry

Functional Description :

    This routine uses the information from the "just" completed CCB to 
    determine if it should be reissued to to the SIM for the b/t/l.  The
    external ccfg_inquiry_retry_limit variable, in cam_data.c, is also
    used for the user supplied limit count. 

Formal Parameters : 

    The PDrv working set pointer for the CCB.

Implicit Inputs : 

    The unsigned long ccfg_inquiry_retry_limit in cam_data.c

Implicit Outputs : None

Return Value : 

    This routine returns the BOOLEAN values CAM_TRUE or CAM_FALSE.

Side Effects :

    The PDrv working set is updated with the retry counts.  The returned
    information fields in the CCB are cleared during the retries.  The
    values from the last retry will remain to be used by the caller.

Additional Information :
    There is some, limited, attempts to deal with older SCSI devices.  The 
    CAM flag to initiate SDTR may be turned off depending on some of the
    CAM status values returned.  This will help to allow some of the 
    devices to be "found" that do not know how to deal with SDTR.

*/

static U32
ccfg_ok_toretry( pws )
    volatile PDRV_WS *pws;		/* working set for the CCB */
{
    U32 retry_return;		/* holder for the return value */
    CCB_SCSIIO *cio;			/* for the INQUIRY CCB */

    cio = pws->pws_ccb;			/* point to the IO CCB */

    PRINTD( cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	cio->cam_ch.cam_target_lun, (CAMD_INOUT|CAMD_CONFIG),
	("[%d/%d/%d] ccfg_ok_toretry: Enter, IO CCB: 0x%x.\n",
	cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	cio->cam_ch.cam_target_lun, cio));

    /* Check the retry limit, if the limit has been reached return 
    CAM_FALSE to the caller. */

    if( (pws->pws_flags & CCFG_RETRY) != 0 )
    {
	if( pws->pws_retry_cnt >= ccfg_inquiry_retry_limit )
	{
	    return( (U32)CAM_FALSE );	/* no more retries */
	}
    }

    /* Update the working set retry information. */

    pws->pws_flags |= CCFG_RETRY;	/* set the retry flag */
    pws->pws_retry_cnt++;		/* update the count */

    /* Check the CAM status returned from the I/O request, the switch
    statement is used to "lump" the different status values together. */

    switch( (int)cio->cam_ch.cam_status )
    {
	/* Do not allow any more retries on these status values. */

	case CAM_REQ_CMP:
	case CAM_REQ_INVALID:
	case CAM_PATH_INVALID:
	case CAM_DEV_NOT_THERE:
	case CAM_SEL_TIMEOUT:
	case CAM_NO_HBA:
	case CAM_CCB_LEN_ERR:
	case CAM_PROVIDE_FAIL:

	    retry_return = CAM_FALSE;		/* not ok to retry */

	break;

	/* Try more retries without attempting SDTR. */

	case CAM_MSG_REJECT_REC:
	case CAM_UNEXP_BUSFREE:
	case CAM_SEQUENCE_FAIL:

	    cio->cam_ch.cam_flags &= ~(CAM_INITIATE_SYNC); /* clear the flag */
	    retry_return = CAM_TRUE;		/* ok to retry */

	break;

	/* Try again. */

	default:
	case CAM_REQ_INPROG:
	case CAM_REQ_ABORTED:
	case CAM_UA_ABORT:
	case CAM_REQ_CMP_ERR:
	case CAM_BUSY:
	case CAM_UA_TERMIO:
	case CAM_CMD_TIMEOUT:
	case CAM_SCSI_BUS_RESET:
	case CAM_UNCOR_PARITY:
	case CAM_AUTOSENSE_FAIL:
	case CAM_DATA_RUN_ERR:
	case CAM_BDR_SENT:
	case CAM_REQ_TERMIO:

	    retry_return = CAM_TRUE;		/* ok to retry */

	break;
    }

    /* Clear out the CAM returned info fields for the retry. */

    if( retry_return == CAM_TRUE )
    {
	cio->cam_ch.cam_status = CAM_REQ_INPROG;
	cio->cam_scsi_status = NULL;
	cio->cam_resid = 0;
	cio->cam_sense_resid = 0;
    }

    /* Now let the caller know what to do on the retry. */

    PRINTD( cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	cio->cam_ch.cam_target_lun, (CAMD_INOUT|CAMD_CONFIG),
	("[%d/%d/%d] ccfg_ok_toretry: Exit status for IO CCB: 0x%x. is %s\n",
	cio->cam_ch.cam_path_id, cio->cam_ch.cam_target_id,
	cio->cam_ch.cam_target_lun, cio,
	((retry_return == CAM_FALSE)? "CAM_FALSE" : "CAM_TRUE" )));

    return( retry_return );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_errlog

Functional Description :

Local error logging routine for the CDrv.  Using the arguments from the 
caller a system error log packet is filled with the error information and 
then stored within the system.

Formal Parameters :
The six arguments via the macro:

    fs : The Function name string
    ms : The message string
    ef : Relivant error flags
    id : The bus/SIM if not NO_ERRVAL
    ct : The CAM cam_conftbl addr if not NULL
    cc : The CDrv control pointer if not NULL

Implicit Inputs : None

Implicit Outputs : Error Log Structures.

Return Value : None

Side Effects : A Lot of Error log work.

Additional Information :
*/

/* ARGSUSED */
static void
ccfg_errlog( fs, ms, ef, id, ct, cc )
    char *fs;				/* Function Name string */
    char *ms;				/* The message string */
    U32 ef;				/* Relivant error flags */
    I32 id;				/* The bus/SIM if not NO_ERRVAL */
    CAM_SIM_ENTRY *ct;			/* The cam_conftbl addr if not NULL */
    CCFG_CTRL *cc;			/* The CDrv control ptr if not NULL */
{
    CAM_ERR_HDR err_hdr;		/* Local storage for the header */
    CAM_ERR_HDR *eh;			/* pointer for accessing it */

    CAM_ERR_ENTRY err_ent[CCFG_ERR_CNT];/* the local error entry list */
    CAM_ERR_ENTRY *cee;			/* error entry pointer */

    U32 ent;				/* current entry number in the list */
    U32 i;				/* loop counter */

    /* Setup the local variables. */

    ent = 0;				/* first entry */

    /* Clear out the header structure and the Error structures. */

    bzero( &err_hdr, sizeof(CAM_ERR_HDR) );	/* Zero fill the header */

    for( i = 0; i < CCFG_ERR_CNT; i++ )
    {
	bzero( &err_ent[i], sizeof(CAM_ERR_ENTRY) );	/* fill the entry */
    }

    /* Setup the Error log header. */

    eh = &err_hdr;			/* pointer for the header */

    eh->hdr_type = CAM_ERR_PKT;		/* A CAM error packet */
    eh->hdr_class = CLASS_CCFG;		/* the CDrv is the head of the class */
    eh->hdr_subsystem = SUBSYS_CCFG;	/* fill in the subsystem field */
    eh->hdr_entries = ent;		/* set the entries */
    eh->hdr_list = err_ent;		/* point to the entry list */
    eh->hdr_pri = ef;			/* the priority is passed in */

    /* Setup the First Error entry on the list.  This entry will contain the
    function string. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( fs != (char *)NULL )			/* make sure there is one */
    {
	cee->ent_type = ENT_STR_MODULE;		/* the entry is a string */
	cee->ent_size = strlen(fs) +1;		/* how long it is */
	cee->ent_vers = 1;			/* Version 1 for strings */
	cee->ent_data = (u_char *)fs;		/* point to it */
	cee->ent_pri = PRI_BRIEF_REPORT;	/* strings are brief */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain the
    message string. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( ms != (char *)NULL )			/* make sure there is one */
    {
	cee->ent_type = ENT_STRING;		/* the entry is a string */
	cee->ent_size = strlen(ms) +1;		/* how long it is */
	cee->ent_vers = 1;			/* Version 1 for strings */
	cee->ent_data = (u_char *)ms;		/* point to it */
	cee->ent_pri = PRI_BRIEF_REPORT;	/* strings are brief */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain the
    address of the CAM conftbl[] array if present. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( ct != (CAM_SIM_ENTRY *)NULL )		/* make sure there is one */
    {
	cee->ent_type = ENT_CCFG_CONFTBL;	/* this entry is the Q-head */
	cee->ent_size =				/* how big it is */
	    (sizeof(CAM_SIM_ENTRY *)) * N_cam_conftbl;
	cee->ent_vers = CAM_VERSION;		/* current version */
	cee->ent_data = (u_char *)ct;		/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Setup the Next Error entry on the list.  This entry will contain 
    the CDrv Control structure if present. */

    cee = &err_ent[ ent ];		/* set the pointer */

    if( cc != (CCFG_CTRL *)NULL )		/* make sure there is one */
    {
	cee->ent_type = ENT_CCFG_CTRL;		/* this entry is the wrk set */
	cee->ent_size = sizeof(CCFG_CTRL);	/* how long it is */
	cee->ent_vers = CCFG_CTRL_VERS;		/* current version */
	cee->ent_data = (u_char *)cc;		/* point to it */
	cee->ent_pri = PRI_FULL_REPORT;		/* structures are full */
	ent++;					/* on to the next entry */
    }

    /* Call the CAM Error logger handler with the error structures. */

    eh->hdr_entries = ent;		/* signal how many valid entries */

    cam_logger( eh, (char)id, (char)NO_ERRVAL, (char)NO_ERRVAL ); 

    return;				/* all done */
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* This code will have to change for the ULTRIX/BSD to ULTRIX/OSF port !! */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_alloc_edt()

Functional Description :

    This routine will dynamicly allocate the necessary physical memory for
one EDT grid.

Formal Parameters : None

Implicit Inputs : 

    System allocation interfaces.

Implicit Outputs : None

Return Value :

    A pointer the data area.

Side Effects : TBD

Additional Information :

    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

/* ARGSUSED */
static EDT *
ccfg_alloc_edt()
{
    EDT *e;

    e = (EDT *)cam_zalloc(sizeof(EDT));

    return( e );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_alloc_async()

Functional Description :

    This routine will dynamicly allocate the necessary physical memory for
one async callback data stucture.

Formal Parameters : None

Implicit Inputs : 

    System allocation interfaces.

Implicit Outputs : None

Return Value :

    A pointer the data area.

Side Effects : TBD

Additional Information :

    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

static ASYNC_INFO *
ccfg_alloc_async()
{
    ASYNC_INFO *a;

    a = (ASYNC_INFO *)cam_zalloc(sizeof(ASYNC_INFO));

    return( a );
}

/* ---------------------------------------------------------------------- */
/*
Routine Name : ccfg_free_async()

Functional Description :

    This routine will free an ASYNC_INFO buffer back to the system memory
pool.

Formal Parameters : 

    The pointer to a ASYNC_INFO structure.

Implicit Inputs : None

Implicit Outputs : 

    System allocation interfaces.

Return Value : None

Side Effects : TBD

Additional Information :

    This routine will be one of the Ultrix/BSD -> Ultrix/OSF port problems.
*/

static void
ccfg_free_async( a )
    ASYNC_INFO *a;
{
    cam_zfree((char *)a, sizeof(ASYNC_INFO));
}
/* ---------------------------------------------------------------------- */

/*
END OF FILE
r !make ccfg.o
*/

