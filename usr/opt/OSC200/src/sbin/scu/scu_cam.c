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
static char *rcsid = "@(#)$RCSfile: scu_cam.c,v $ $Revision: 1.1.8.5 $ (DEC) $Date: 1993/12/15 20:56:56 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu_cam.c
 * Author:	Robin T. Miller
 * Date:	October 1, 1991
 *
 * Description:
 *	This file contains routines to support CAM specific commands.
 */

#include <stdio.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/uagt.h>
#include <io/cam/cam_debug.h>

#undef SUCCESS
#undef FAILURE
#include "scu.h"
#include "scu_device.h"

/*
 * External Declarations:
 */
extern char *GetDeviceName();
extern int CheckRootAccess(), CheckWriteAccess();
extern void Fprintf(), FprintC(), Perror(), Printf();
extern int IS_Mounted (struct scu_device *scu);
extern int OpenPager (char *pager);
extern void PrintInquiry(), PrintPathInquiry();
extern void PrintHeader (char *header);
extern void PrintHex (char *field_str, int numeric_value, int nl_flag);
extern void PrintNumeric (char *field_str, int numeric_value, int nl_flag);
extern void ResetDeviceEntry (struct scu_device *scu, u_char level);

extern void cdbg_DumpCCBHeader();
extern void cdbg_DumpGETDEV(), cdbg_DumpPATHINQ(), cdbg_DumpSETDEV();
extern caddr_t cdbg_CamFunction(), cdbg_CamStatus();
/*
 * Local Declarations:
 */
#define CSM		CAM_STATUS_MASK	/* CAM status mask (short form)	*/

/*
 * Forward References:
 */
I32 xpt_action (struct ccb_header *ccbh);
void CAMerror (struct ccb_header *ccbh);
void PrintEdtEntry (struct ccb_getdev *ccb, struct all_inq_data *inquiry);
void SetupDeviceInfo (struct scu_device *scu);
void SetupNexus (struct cmd_entry *ce,
		struct ccb_header *ccbh, struct scu_device *scu);

/*
 * Message Strings:
 */
static char *edt_str =	"CAM Equipment Device Table (EDT) Information";

static char *path_str =			"Path Inquiry Information";
static char *version_num_str =		"Version Number For The SIM/HBA";
static char *hba_inquiry_str =		"SCSI HBA Capabilities Flags";
static char *target_sprt_str =		"Target Mode Support Flags";
static char *hba_misc_str =		"Miscellaneous HBA Feature Flags";
static char *hba_eng_cnt_str =		"The HBA Engine Count";
static char *vuhba_flags_str =		"Vendor Unique Capabilities";
static char *sim_priv_str =		"Size of SIM Private Data Area";
static char *async_flags_str =		"Async Callback Capabilities";
static char *hpath_id_str =		"Highest HBA Path ID Assigned";
static char *initiator_id_str =		"SCSI Device ID of Initiator";
static char *sim_vid_str =		"The Vendor ID of the SIM";
static char *hba_vid_str =		"The Vendor ID of the HBA";
static char *osd_usage_str =		"The OSD Usage Pointer";

/************************************************************************
 *									*
 * GetDeviceType() - Get the CAM Device Type Information.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		ccb = The Get Device Type CCB (optional).		*
 *		inquiry = The Inquiry data buffer.			*
 *		scu = The selected SCSI device unit.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
GetDeviceType (ce, ke, ccb, inquiry, scu)
struct cmd_entry *ce;
struct key_entry *ke;
struct ccb_getdev *ccb;
register struct all_inq_data *inquiry;
register struct scu_device *scu;
{
	struct ccb_getdev getdev_ccb;
	register struct ccb_header *ccbh;
	int status;

	/*
	 * Most of the time, we don't want the ccb information.
	 */
	if (ccb == (struct ccb_getdev *) 0) {
	    ccb = &getdev_ccb;
	}

	ccbh = (struct ccb_header *) ccb;
	(void) bzero ((char *) ccb, sizeof(*ccb));
	(void) bzero ((char *) inquiry, sizeof(*inquiry));

	ccbh->my_addr = (struct ccb_header *) ccb;
	ccbh->cam_ccb_len = (u_short) sizeof (*ccb);
	ccbh->cam_func_code = XPT_GDEV_TYPE;
	ccb->cam_inq_data = (char *) inquiry;
	SetupNexus (ce, ccbh, scu);

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_CMD_EXP, cdbg_DumpGETDEV(ccb));

	if ((status = xpt_action ((struct ccb_header *) ccb)) != SUCCESS) {
	    Perror ("'%s' failed", "get device type");
	} else {
	    CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_CMD_EXP, cdbg_DumpGETDEV(ccb));
	    if (IS_SelectedDevice (scu, ccbh) == TRUE) {
	        scu->scu_device_type = inquiry->dtype;
	    }
	}

	return (status);
}

/************************************************************************
 *									*
 * GetPathInquiry() - Get the CAM Path Inquiry Information.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		ccb = The Path Inquiry CCB.				*
 *		scu = The selected SCSI device unit.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
GetPathInquiry (ce, ke, ccb, scu)
struct cmd_entry *ce;
struct key_entry *ke;
struct ccb_pathinq *ccb;
register struct scu_device *scu;
{
	register struct ccb_header *ccbh = (struct ccb_header *) ccb;
	int status;

	(void) bzero ((char *) ccb, sizeof(*ccb));

	ccbh->my_addr = (struct ccb_header *) ccb;
	ccbh->cam_ccb_len = (u_short) sizeof (*ccb);
	ccbh->cam_func_code = XPT_PATH_INQ;
	SetupNexus (ce, ccbh, scu);

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_CMD_EXP, cdbg_DumpCCBHeader(ccb));

	if ((status = xpt_action ((struct ccb_header *) ccb)) != SUCCESS) {
	    Perror ("'%s' failed", ke->k_msgp);
	} else {
	    CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_CMD_EXP, cdbg_DumpPATHINQ(ccb));
	}

	return (status);
}

/************************************************************************
 *									*
 * SetCamFlags() - Set CAM CCB Flags into Selected Device Structure.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
SetCamFlags (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct scu_device *scu = ScuDevice;
	u_long cam_flags = *(u_long *) ke->k_argp;

	scu->scu_cam_flags = ( cam_flags & CAM_CCB_FLAGS_MASK );
	return (SUCCESS);
}

/************************************************************************
 *									*
 * SetDeviceType() - Set the CAM Device Type Information.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetDeviceType (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct ccb_setdev setdev_ccb;
	struct ccb_setdev *ccb = &setdev_ccb;
	register struct ccb_header *ccbh = (struct ccb_header *) ccb;
	register struct scu_device *scu = ScuDevice;
	int status;

	/*
	 * This command alters the CAM EDT, so restrict to root access.
	 */
	if ((status = CheckRootAccess (ke, scu)) != SUCCESS) {
	    return (status);
	}

	(void) bzero ((char *) ccb, sizeof(*ccb));

	ccbh->my_addr = (struct ccb_header *) ccb;
	ccbh->cam_ccb_len = (u_short) sizeof (*ccb);
	ccbh->cam_func_code = XPT_SDEV_TYPE;
	ccb->cam_dev_type = (u_char) UserDeviceType;
	SetupNexus (ce, ccbh, scu);

	if ( (IS_SelectedDevice (scu, ccbh) == TRUE) &&
	     ( (scu->scu_device_type == ALL_DTYPE_DIRECT) ||
	       (scu->scu_device_type == ALL_DTYPE_OPTICAL) ||
	       (scu->scu_device_type == ALL_DTYPE_RODIRECT) ) ) {
	    /*
	     * Ensure there are no mounted file systems.
	     */
	    if ((status = IS_Mounted (scu)) != FALSE) {
		return (FAILURE);
	    }
	}

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_CMD_EXP, cdbg_DumpSETDEV(ccb));

	if ((status = xpt_action ((struct ccb_header *) ccb)) != SUCCESS) {
	    Perror ("'%s' failed", ke->k_msgp);
	} else {
	    if ( IS_SelectedDevice (scu, ccbh) ) {
		scu->scu_device_type = (u_char) UserDeviceType;
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * SetDeviceNexus() - Set Device Nexus (Bus/Target/LUN) Information.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetDeviceNexus (ce, ke)
register struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;
	int status;

	if (ISSET_KOPT (ce, K_BUS)) {
	    scu->scu_bus = (u_char) UserBus;
	}

	if (ISSET_KOPT (ce, K_TARGET)) {
	    scu->scu_target = (u_char) UserTarget;
	}

	if (ISSET_KOPT (ce, K_LUN)) {
	    scu->scu_lun = (u_char) UserLun;
	}

	if ( (status = CheckDeviceNexus (ce, ke)) == SUCCESS) {
	    size_t size;
	    caddr_t s;

	    /*
	     * Updates inquiry data which contains the device type.
	     */
	    status = GetDeviceType (ce, ke, NULL, scu->scu_inquiry, scu);

	    /*
             * Reset device parameters since we're changing devices.
	     */
	    ResetDeviceEntry (scu, HARD);
	    if (status == SUCCESS) {
		SetupDeviceInfo (scu);
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * SetupDeviceInfo() - Setup New Device Information.			*
 *									*
 * Description:								*
 *	This function sets up the device entry, device name, & default	*
 * device block size when switching to a new device via "set nexus".	*
 *									*
 * Inputs:	scu = The SCSI device unit.				*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
#define NEXUS_ALLOC	12		/* # of byte for nexus string. */

void
SetupDeviceInfo (struct scu_device *scu)
{
	register caddr_t s;
	size_t size;

	/*
	 * Setup psuedo device entry/device name for this nexus.
	 */
	if (scu->scu_device_entry = (caddr_t) malloc (NEXUS_ALLOC)) {
	    (void) sprintf (scu->scu_device_entry, "[%d/%d/%d]",
		scu->scu_bus, scu->scu_target, scu->scu_lun);
	}

	/*
	 * Since we don't have a device name, use the product name.
	 */
	size = sizeof(scu->scu_inquiry->pid) + 1;
	s = (caddr_t) malloc (size);
	strncpy (s, scu->scu_inquiry->pid, size);
	while ( isspace(*s) ) {
	    s++, size--;
	}
	scu->scu_device_name = s;
	while (size && !isspace(*s)) {
	    s++, size--;
	}
	*s = '\0';

	/*
	 * Setup the default device block size (0 = variable).
	 */
	if (scu->scu_device_type != ALL_DTYPE_SEQUENTIAL) {
	    scu->scu_device_size = DEFAULT_BLOCK_SIZE;
	}
}

/************************************************************************
 *									*
 * ShowDeviceNexus() - Show Device Nexus (Bus/Target/LUN) Information.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
ShowDeviceNexus (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;

	Printf (
	"Device Nexus: Bus: %d, Target: %d, Lun: %d, Device Type: %s\n",
		    	scu->scu_bus, scu->scu_target, scu->scu_lun,
			GetDeviceName (scu->scu_device_type));

	return (SUCCESS);
}

/************************************************************************
 *									*
 * ScanEdtEntrys() - Force a Scan to Update the Equipment Device Table.	*
 *									*
 * Description:								*
 *	This function is used to force a full or partial scan of a SCSI	*
 * bus to update the CAM Equipment Device Table (EDT).  This action is	*
 * necessary if the device was not found during booting, or if a set	*
 * device type CCB was issued to modify the EDT of an existing device.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
ScanEdtEntrys (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct uagt_cam_scan cam_scan;
	struct uagt_cam_scan *ucs = &cam_scan;
	struct scu_device *scu = ScuDevice;
	u_long uagt_cmd;
	int status;

	(void) bzero ((char *)ucs, sizeof(*ucs));

	/*
	 * The user specified bus/target/lun controls the scanning.
	 */
	if (ISSET_KOPT (ce, K_BUS)) {
	    ucs->ucs_bus = (u_char) UserBus;
	} else {
	    if ( (ucs->ucs_bus = scu->scu_bus) == NEXUS_NOT_SET) {
		ucs->ucs_bus = 0;
	    }
	}

	if (ISSET_KOPT (ce, K_TARGET)) {
	    ucs->ucs_target = (u_char) UserTarget;
	} else {
	    if ( (ucs->ucs_target = scu->scu_target) == NEXUS_NOT_SET) {
		ucs->ucs_target = 0;
	    }
	}

	if (ISSET_KOPT (ce, K_LUN)) {
	    ucs->ucs_lun = (u_char) UserLun;
	} else {
	    if ( (ucs->ucs_lun = scu->scu_lun) == NEXUS_NOT_SET) {
		ucs->ucs_lun = 0;
	    }
	}

	if ( (ISCLR_KOPT (ce, K_TARGET)) && (ISCLR_KOPT (ce, K_LUN)) &&
	     (ISSET_KOPT (ce, K_BUS)) ) {
	    uagt_cmd = UAGT_CAM_FULL_SCAN;
	} else if ( (scu->scu_target == NEXUS_NOT_SET) ||
		    (scu->scu_lun == NEXUS_NOT_SET) ) {
	    uagt_cmd = UAGT_CAM_FULL_SCAN;
	} else {
	    uagt_cmd = UAGT_CAM_SINGLE_SCAN;
	}

	if (DisplayVerbose || DebugFlag) {
	    if (uagt_cmd == UAGT_CAM_FULL_SCAN) {
		Printf ("Scanning bus %d, please be patient...\n",
							ucs->ucs_bus);
	    } else {
		Printf ("Scanning bus %d, target %d, lun %d, please be patient...\n",
			ucs->ucs_bus, ucs->ucs_target, ucs->ucs_lun);
	    }
	}

	CmdInProgressFlag = TRUE;
	if ( (status = ioctl (UagtFd, uagt_cmd, ucs)) != SUCCESS) {
	    Perror ("'%s' failed", ce->c_msgp);
	}
	CmdInProgressFlag = FALSE;

	return (status);
}

/************************************************************************
 *									*
 * ShowEdtEntrys() - Show the CAM Equipment Device Table Entrys.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowEdtEntrys (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct ccb_getdev getdev_ccb;
	struct ccb_getdev *ccb = &getdev_ccb;
	struct ccb_pathinq pathinq_ccb;
	struct ccb_pathinq *pccb = &pathinq_ccb;
	struct all_inq_data inquiry_data;
	register struct all_inq_data *inquiry = &inquiry_data;
	register struct scu_device *scu = ScuDevice;
	register int bus = 0, target = 0, lun = 0;
	int ini_bus, ini_target, ini_lun, max_bus, max_target, max_lun;
	int device_count = 0, saved_CamErrorFlag, saved_PerrorFlag;
	int status = SUCCESS;

	/*
	 * The user specified bus/target/lun limits the EDT display.
	 */
	if ( ISSET_KOPT (ce, K_BUS) ) {
	    ini_bus = UserBus;
	    max_bus = UserBus;
	} else {
	    ini_bus = UserBus = 0;
	    max_bus = MAX_SCSI_BUS;
	    SET_KOPT (ce, K_BUS);
	}

	if ( ISSET_KOPT (ce, K_TARGET) ) {
	    ini_target = UserTarget;
	    max_target = UserTarget;
	} else {
	    ini_target = UserTarget = 0;
	    max_target = MAX_SCSI_TARGET;
	    SET_KOPT (ce, K_TARGET);
	}

	if ( ISSET_KOPT (ce, K_LUN) ) {
	    ini_lun = UserLun;
	    max_lun = UserLun;
	} else {
	    ini_lun = UserLun = 0;
	    max_lun = MAX_SCSI_LUN;
	    SET_KOPT (ce, K_LUN);
	}

	saved_CamErrorFlag = CamErrorFlag;
	saved_PerrorFlag = PerrorFlag;
	CamErrorFlag = PerrorFlag = FALSE;
	(void) OpenPager (NULL);

	/*
	 * Loop through all (or selected) nexus combinations:
	 */
	for (bus = ini_bus; bus <= max_bus; bus++) {
	    if (CmdInterruptedFlag == TRUE) break;
	    UserBus = bus; UserTarget = 0; UserLun = 0;
	    /*
	     * Get the path inquiry information to obtain the HBA Target ID.
	     */
	    if (GetPathInquiry (ce, ke, pccb, scu) != SUCCESS) {
		continue;	/* HBA does not exist, try next bus.	*/
	    }
	    for (target = ini_target; target <= max_target; target++) {
		if (CmdInterruptedFlag == TRUE) break;
		UserTarget = target;
		/*
		 * Skip the initiator (HBA) target ID.
		 */
		if (pccb->cam_initiator_id == target) {
		    continue;
		}
		for (lun = ini_lun; lun <= max_lun; lun++) {
		    if (CmdInterruptedFlag == TRUE) break;
		    UserLun = lun;
		    status = GetDeviceType (ce, ke, ccb, inquiry, scu);
		    if (status == SUCCESS) {
			if (device_count == 0) {
			    PrintHeader (edt_str);
			}
			if ( ISSET_KOPT (ce, K_REPORT_FULL) ) {
			    PrintInquiry (ccb, inquiry);
			    Printf ("\n");
			} else {
			    PrintEdtEntry (ccb, inquiry);
			}
			device_count++;
		    }
		}
	    }
	}
	CamErrorFlag = saved_CamErrorFlag;
	PerrorFlag = saved_PerrorFlag;
	return ( (device_count == 0) ? FAILURE : SUCCESS);
}

/************************************************************************
 *									*
 * PrintEdtEntry() - Print The CAM EDT Information.			*
 *									*
 * Inputs:	ccb = The Get Device Type CCB.				*
 *		inquiry = Pointer to inquiry data.			*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
PrintEdtEntry (ccb, inquiry)
struct ccb_getdev *ccb;
register struct all_inq_data *inquiry;
{
	register struct ccb_header *ccbh = (struct ccb_header *) ccb;

	Printf ("    Bus: %d, Target: %d, Lun: %d, Device Type: %s\n",
		ccbh->cam_path_id, ccbh->cam_target_id,
		ccbh->cam_target_lun, GetDeviceName(inquiry->dtype));
}

/************************************************************************
 *									*
 * ShowDeviceType() - Show the CAM Device Type Information.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowDeviceType (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct ccb_getdev getdev_ccb;
	struct ccb_getdev *ccb = &getdev_ccb;
	register struct scu_device *scu = ScuDevice;
	int status;

	status = GetDeviceType (ce, ke, ccb, scu->scu_inquiry, scu);

	if (status == SUCCESS) {
	    PrintInquiry (ccb, scu->scu_inquiry);
	}
	return (status);
}

/************************************************************************
 *									*
 * ShowPathInquiry() - Show the CAM Path Inquiry Information.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowPathInquiry (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct ccb_pathinq pathinq_ccb;
	struct ccb_pathinq *ccb = &pathinq_ccb;
	register struct scu_device *scu = ScuDevice;
	int status;

	if ( (status = GetPathInquiry (ce, ke, ccb, scu)) != SUCCESS) {
	    return (status);
	}

	PrintPathInquiry (ccb);

	return (status);
}

/************************************************************************
 *									*
 * PrintPathInquiry() - Print the CAM Path Inquiry Information.		*
 *									*
 * Inputs:	ccb = The Path Inquiry CCB to print.			*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
static void
PrintPathInquiry (ccb)
struct ccb_pathinq *ccb;
{
	char hba_vid[SIM_ID + 1];
	char sim_vid[HBA_ID + 1];
	char tmp_buffer[80];
	char *tmp = tmp_buffer;
	register int i;

	PrintHeader (path_str);

	(void) sprintf (tmp, "%d.%d", (ccb->cam_version_num >> 4) & 0x0f,
				      (ccb->cam_version_num & 0x0f) );
	PrintAscii (version_num_str, tmp, PNL);

	PrintHex (hba_inquiry_str, ccb->cam_hba_inquiry, PNL);
	if (ccb->cam_hba_inquiry) {
	    if (ccb->cam_hba_inquiry & PI_MDP_ABLE) {
		(void) sprintf (tmp, "%#x = Supports MDP message.",
							PI_MDP_ABLE);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_hba_inquiry & PI_WIDE_32) {
		(void) sprintf (tmp, "%#x = Supports 32 bit wide SCSI.",
							PI_WIDE_32);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_hba_inquiry & PI_WIDE_16) {
		(void) sprintf (tmp, "%#x = Supports 16 bit wide SCSI.",
							PI_WIDE_16);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_hba_inquiry & PI_SDTR_ABLE) {
		(void) sprintf (tmp, "%#x = Supports SDTR message.",
							PI_SDTR_ABLE);
		PrintAscii ("", tmp, PNL);
	    }

	    if (ccb->cam_hba_inquiry & PI_LINKED_CDB) {
		(void) sprintf (tmp, "%#x = Supports linked CDBs.",
							PI_LINKED_CDB);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_hba_inquiry & PI_TAG_ABLE) {
		(void) sprintf (tmp, "%#x = Supports tag queue message.",
							PI_TAG_ABLE);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_hba_inquiry & PI_SOFT_RST) {
		(void) sprintf (tmp, "%#x = Supports soft reset.",
							PI_SOFT_RST);
		PrintAscii ("", tmp, PNL);
	    }
	}
 	PrintHex (target_sprt_str, ccb->cam_target_sprt, PNL);
	if (ccb->cam_target_sprt) {
	    if (ccb->cam_target_sprt & PIT_PROCESSOR) {
		(void) sprintf (tmp, "%#x = Target mode processor mode.",
							PIT_PROCESSOR);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_target_sprt & PIT_PHASE) {
		(void) sprintf (tmp, "%#x = Target mode phase cognizant mode.",
							PIT_PHASE);
		PrintAscii ("", tmp, PNL);
	    }
	}

	PrintHex (hba_misc_str, ccb->cam_hba_misc, PNL);
	if (ccb->cam_hba_misc) {
	    if (ccb->cam_hba_misc & PIM_SCANHILO) {
		(void) sprintf (tmp, "%#x = Bus scans from ID 7 to ID 0.",
							PIM_SCANHILO);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_hba_misc & PIM_NOREMOVE) {
		(void) sprintf (tmp, "%#x = Removable device not included in scan.",
							PIM_NOREMOVE);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_hba_misc & PIM_NOINQUIRY) {
		(void) sprintf (tmp, "%#x = Inquiry data not kept by XPT.",
							PIM_NOINQUIRY);
		PrintAscii ("", tmp, PNL);
	    }
	}

	PrintNumeric (hba_eng_cnt_str, ccb->cam_hba_eng_cnt, PNL);

	PrintHex (vuhba_flags_str, ccb->cam_vuhba_flags[0], DNL);
	for (i = 1; i < VUHBA; i++) {
	    Printf (" %#x", ccb->cam_vuhba_flags[i]);
	}
	Printf ("\n");

	PrintNumeric (sim_priv_str, ccb->cam_sim_priv, PNL);

	PrintHex (async_flags_str, ccb->cam_async_flags, DNL);
	Printf (" Reasons for generating async event:\n");
	if (ccb->cam_async_flags) {
	    if (ccb->cam_async_flags & AC_FOUND_DEVICES) {
		(void) sprintf (tmp, "%#x = New devices found during rescan.",
							AC_FOUND_DEVICES);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_async_flags & AC_SIM_DEREGISTER) {
		(void) sprintf (tmp, "%#x = SIM module DE-registered.",
							AC_SIM_DEREGISTER);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_async_flags & AC_SIM_REGISTER) {
		(void) sprintf (tmp, "%#x = SIM module Registered.",
							AC_SIM_REGISTER);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_async_flags & AC_SENT_BDR) {
		(void) sprintf (tmp, "%#x = Sent Bus Device Reset to Target.",
							AC_SENT_BDR);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_async_flags & AC_SCSI_AEN) {
		(void) sprintf (tmp, "%#x = SCSI AEN Received.",
							AC_SCSI_AEN);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_async_flags & AC_UNSOL_RESEL) {
		(void) sprintf (tmp, "%#x = Unsolicited Reselection.",
							AC_UNSOL_RESEL);
		PrintAscii ("", tmp, PNL);
	    }
	    if (ccb->cam_async_flags & AC_BUS_RESET) {
		(void) sprintf (tmp, "%#x = Unsolicited SCSI Bus Reset.",
							AC_BUS_RESET);
		PrintAscii ("", tmp, PNL);
	    }
	}

	PrintNumeric (hpath_id_str, ccb->cam_hpath_id, PNL);

	PrintNumeric (initiator_id_str, ccb->cam_initiator_id, PNL);

	strncpy (sim_vid, ccb->cam_sim_vid, SIM_ID);
	sim_vid[SIM_ID] = '\0';
	PrintAscii (sim_vid_str, sim_vid, PNL);

	strncpy (hba_vid, ccb->cam_hba_vid, HBA_ID);
	hba_vid[HBA_ID] = '\0';
	PrintAscii (hba_vid_str, hba_vid, PNL);

	PrintHex (osd_usage_str, (long)ccb->cam_osd_usage, PNL);
};

/************************************************************************
 *									*
 * ResetBus() - Reset the SCSI Bus of the Selected SCSI Device.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ResetBus (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct ccb_resetbus resetbus_ccb;
	struct ccb_resetbus *ccb = &resetbus_ccb;
	register struct ccb_header *ccbh = (struct ccb_header *) ccb;
	register struct scu_device *scu = ScuDevice;
	int status;

	/*
	 * This command only requires a bus number, so we check for it here.
	 */
	if ( (ISCLR_KOPT (ce, K_BUS)) && (scu->scu_bus == NEXUS_NOT_SET) ) {
	    return (CheckDeviceNexus (ce, ke));
	}

	/*
	 * Since this can cause loss of data, restrict to root access.
	 */
	if ((status = CheckRootAccess (ke, scu)) != SUCCESS) {
	    return (status);
	}

	(void) bzero ((char *) ccb, sizeof(*ccb));

	ccbh->my_addr = (struct ccb_header *) ccb;
	ccbh->cam_ccb_len = (u_short) sizeof (*ccb);
	ccbh->cam_func_code = XPT_RESET_BUS;
	SetupNexus (ce, ccbh, scu);

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_CMD_EXP, cdbg_DumpCCBHeader(ccb));

	if ((status = xpt_action ((struct ccb_header *) ccb)) != SUCCESS) {
	    Perror ("'%s' failed", ke->k_msgp);
	}
	return (status);
}

/************************************************************************
 *									*
 * ResetDevice() - Reset the Selected SCSI Device.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ResetDevice (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct ccb_resetdev resetdev_ccb;
	struct ccb_resetdev *ccb = &resetdev_ccb;
	register struct ccb_header *ccbh = (struct ccb_header *) ccb;
	register struct scu_device *scu = ScuDevice;
	int status;

	(void) bzero ((char *) ccb, sizeof(*ccb));

	ccbh->my_addr = (struct ccb_header *) ccb;
	ccbh->cam_ccb_len = (u_short) sizeof (*ccb);
	ccbh->cam_func_code = XPT_RESET_DEV;
	SetupNexus (ce, ccbh, scu);

	if (IS_SelectedDevice (scu, ccbh) == TRUE) {
	    if ((status = CheckWriteAccess (ke, scu)) != SUCCESS) {
		return (status);
	    }
	} else {
	    /*
	     * Since this can cause loss of data, restrict to root access.
	     */
	    if ((status = CheckRootAccess (ke, scu)) != SUCCESS) {
		return (status);
	    }
	}

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_CMD_EXP, cdbg_DumpCCBHeader(ccb));

	if ((status = xpt_action ((struct ccb_header *) ccb)) != SUCCESS) {
	    Perror ("'%s' failed", ke->k_msgp);
	}
	return (status);
}

/************************************************************************
 *									*
 * ReleaseSIMQ() - Prepare And Queue A Release SIM Q CCB.		*
 *									*
 * Description:								*
 *	This function handles allocating and filling in of a release	*
 * SIM CCB request packet to unfreeze the SIM Q.  This must be done to	*
 * allow the next request to be processed by the SIM.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
ReleaseSIMQ (ce)
struct cmd_entry *ce;
{
	struct ccb_relsim release_simq_ccb;
	struct ccb_relsim *ccb = &release_simq_ccb;
	register struct ccb_header *ccbh = (struct ccb_header *) ccb;
	register struct scu_device *scu = ScuDevice;
	int status;

	(void) bzero ((char *) ccb, sizeof(*ccb));

	ccbh->cam_ccb_len = (u_short) sizeof(*ccb);
	ccbh->cam_func_code = XPT_REL_SIMQ;
	SetupNexus (ce, ccbh, scu);

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_CMD_EXP, cdbg_DumpCCBHeader(ccb));

	if ((status = xpt_action ((struct ccb_header *) ccb)) != SUCCESS) {
	    Perror ("'%s' failed", ce->c_msgp);
	}
	return (status);
}

/************************************************************************
 *									*
 * SetupNexus() - Setup the Nexus (Bus/Target/Lun) for a CCB.		*
 *									*
 * Description:								*
 *	This function sets up the nexus fields of the CCB.  The bus,	*
 * target, and lun fields get initialized either from the selected SCSI	*
 * device, or from the user specified values.				*
 *									*
 * Inputs:	ce = The command entry.					*
 *		ccbh = The CAM Control Block Header.			*
 *		scu = The selected SCSI device unit.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
SetupNexus (ce, ccbh, scu)
register struct cmd_entry *ce;
register struct ccb_header *ccbh;
register struct scu_device *scu;
{
	if (ISSET_KOPT (ce, K_BUS)) {
	    ccbh->cam_path_id = (u_char) UserBus;
	} else {
	    if ( (ccbh->cam_path_id = scu->scu_bus) == NEXUS_NOT_SET) {
		ccbh->cam_path_id = 0;
	    }
	}

	if (ISSET_KOPT (ce, K_TARGET)) {
	    ccbh->cam_target_id = (u_char) UserTarget;
	} else {
	    if ( (ccbh->cam_target_id = scu->scu_target) == NEXUS_NOT_SET) {
		ccbh->cam_target_id = 0;
	    }
	}

	if (ISSET_KOPT (ce, K_LUN)) {
	    ccbh->cam_target_lun = (u_char) UserLun;
	} else {
	    if ( (ccbh->cam_target_lun = scu->scu_lun) == NEXUS_NOT_SET) {
		ccbh->cam_target_lun = 0;
	    }
	}
}

/************************************************************************
 *									*
 * IS_SelectedDevice() - Determines if CCB is for Selected Device.	*
 *									*
 * Inputs:	scu = The selected SCSI device unit.			*
 *		ccbh = The CCB header to check.				*
 *									*
 * Return Value:							*
 *		Returns TRUE if CCB is for selected device.		*
 *		   "    FALSE if device is for a different device.	*
 *									*
 ************************************************************************/
int
IS_SelectedDevice (scu, ccbh)
register struct scu_device *scu;
register struct ccb_header *ccbh;
{
	if ( (scu->scu_bus == ccbh->cam_path_id) &&
	     (scu->scu_target == ccbh->cam_target_id) &&
	     (scu->scu_lun == ccbh->cam_target_lun) ) {
		return (TRUE);
	} else {
		return (FALSE);
	}
}

/************************************************************************
 *									*
 * xpt_action() - Prepare CCB For The User Agent Driver.		*
 *									*
 * Description:								*
 *	This function takes an already initialized CAM CCB, and issues	*
 * it to the User Agent Driver for processing.				*
 *									*
 * Inputs:	ccbh = The CAM Control Block Header.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
I32
xpt_action (ccbh)
register struct ccb_header *ccbh;
{
	struct uagt_cam_ccb uagt_request;
	register struct uagt_cam_ccb *uagt = &uagt_request;
	int status;

	(void) bzero ((char *) uagt, sizeof(*uagt));
	uagt->uagt_ccb = (struct ccb_header *) ccbh;

	switch (ccbh->cam_func_code) {

	    case XPT_SCSI_IO: {
		register CCB_SCSIIO *ccb = (CCB_SCSIIO *)ccbh;

		uagt->uagt_ccblen = sizeof(*ccb);
		uagt->uagt_buffer = ccb->cam_data_ptr;
		uagt->uagt_buflen = ccb->cam_dxfer_len;
		uagt->uagt_snsbuf = ccb->cam_sense_ptr;
		uagt->uagt_snslen = ccb->cam_sense_len;
		break;
	    }
	    case XPT_GDEV_TYPE: {
		register CCB_GETDEV *ccb = (CCB_GETDEV *)ccbh;

		uagt->uagt_ccblen = sizeof(*ccb);
		uagt->uagt_buffer = (u_char *) ccb->cam_inq_data;
		uagt->uagt_buflen = INQLEN;
		break;
	    }
	    case XPT_PATH_INQ: {
		uagt->uagt_ccblen = sizeof(CCB_PATHINQ);
		break;
	    }
	    case XPT_REL_SIMQ: {
		uagt->uagt_ccblen = sizeof(CCB_RELSIM);
		break;
	    }
	    case XPT_SDEV_TYPE: {
		uagt->uagt_ccblen = sizeof(CCB_SETDEV);
		break;
	    }
	    case XPT_ABORT: {
		uagt->uagt_ccblen = sizeof(CCB_ABORT);
		break;
	    }
	    case XPT_RESET_BUS: {
		uagt->uagt_ccblen = sizeof(CCB_RESETBUS);
		break;
	    }
	    case XPT_RESET_DEV: {
		uagt->uagt_ccblen = sizeof(CCB_RESETDEV);
		break;
	    }
	    case XPT_TERM_IO: {
		uagt->uagt_ccblen = sizeof(CCB_TERMIO);
		break;
	    }
	    default:
		return (EINVAL);
	}

	CmdInProgressFlag = TRUE;
	errno = 0;
	status = ioctl (UagtFd, UAGT_CAM_IO, uagt);
	CmdInProgressFlag = FALSE;

	/*
	 * If the I/O command succeeded, then check the CCB status.
	 */
	if ( (status == SUCCESS) && ((ccbh->cam_status&CSM) != CAM_REQ_CMP) ) {
	    /*
	     * Report any unexpected SCSI I/O CCB errors.
	     */
	    if ( (ccbh->cam_func_code != XPT_SCSI_IO) ||
		 ( (ccbh->cam_func_code == XPT_SCSI_IO) &&
		   ((ccbh->cam_status&CSM) != CAM_REQ_CMP_ERR) ) ) {
		CAMerror (ccbh);
	    }
	    errno = EIO;
	    status = FAILURE;
	}
	return (status);
}

/************************************************************************
 *									*
 * CAMerror() - Common Function to Print CAM Error Messages.		*
 *									*
 * Implicit Inputs:							*
 *	ccbh = Pointer to the CCB header which failed.			*
 *									*
 * Return Value:							*
 *	Void.								*
 * 									*
 ************************************************************************/
void
CAMerror (ccbh)
register struct ccb_header *ccbh;
{
	if (CamErrorFlag == TRUE) {
	    Fprintf ("[%d/%d/%d] CAM CCB function '%s' failed,",
		ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
		cdbg_CamFunction(ccbh->cam_func_code, CDBG_BRIEF),
		cdbg_CamStatus(ccbh->cam_status,CDBG_BRIEF),
		ccbh->cam_status, cdbg_CamStatus(ccbh->cam_status,CDBG_FULL));
	    FprintC ("CAM status = %s (%#x) - %s",
		cdbg_CamStatus(ccbh->cam_status,CDBG_BRIEF),
		ccbh->cam_status, cdbg_CamStatus(ccbh->cam_status,CDBG_FULL));
	}
}
