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
static char *rcsid = "@(#)$RCSfile: scu_misc.c,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/11/23 23:10:40 $";
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
 * File:	scu_misc.c
 * Author:	Robin T. Miller
 * Date:	December 13, 1990
 *
 * Description:
 *	This file contains miscellaneous support functions.
 */

#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/ioctl.h>

#include <sys/mtio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io/common/iotypes.h>
#include <io/cam/rzdisk.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_special.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"

/*
 * Externel References:
 */
extern int errno;
extern char *malloc_palign(int size);
extern void free_palign(char *pa_addr);
extern void CloseFile (FILE **fp);
extern void DumpSenseData (struct all_req_sns_data *sdp);
extern void Fprintf(), Perror(), Printf();
extern void FreeDefaultPages (struct scu_device *scu);
extern int GetDeviceType (struct cmd_entry *ce, struct key_entry *ke,
			  struct ccb_getdev *ccb, struct all_inq_data *inquiry,
			  struct scu_device *scu);
extern void Pclose (FILE **fp);
extern FILE *Popen (char *pstr, char *mode);

extern int scmn_SpecialCmd (struct special_args *sap);
extern int scmn_FreeArgsBuffer (struct special_args *sap);
extern struct special_args *scmn_GetArgsBuffer (int bus, int target, int lun);
extern u_long xpt_action (struct ccb_header *ccbh);

/*
 * Forward References:
 */
int DoDeviceSetup (struct scu_device *scu);
void FreeDeviceEntry (struct scu_device *scu);
struct scu_device *MakeDeviceEntry();
void PrintSwitchEntry (struct scu_device *scu);

#define NUMBER_BUFFER_SIZE	24

static char number_buffer[NUMBER_BUFFER_SIZE];

/************************************************************************
 *									*
 * DoDeviceSetup() - Get & Sets Up The Device Information.		*
 *									*
 * Description:								*
 *	This function opens the specified device, gets device info	*
 * using the DEVIOCGET I/O control command, and then sets up some of	*
 * the initial device information (device type, block size, etc).	*
 *									*
 * Note:  This function purposely does NOT issue any SCSI commands to	*
 *	  obtain capacity, page parameters, etc.  This is done so the	*
 *	  user has direct control over all SCSI commands.  This means	*
 *	  that any 'scu' commands requiring specific device information	*
 *	  is responsible for requesting it from the device.		*
 *									*
 *	  The device specified is purposely left open to serve as a	*
 *	  locking mechanism against other users.  This can be bypassed	*
 *	  by using /dev/cam and setting the appropriate nexus info.	*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoDeviceSetup (scu)
register struct scu_device *scu;
{
	register struct devget *devget = &scu->scu_devget;
	struct cmd_entry *ce;
	struct key_entry *ke;
	int status;

	/*
	 * Validate the device name specified.
	 */
	if ((status = ValidateDevice (scu->scu_device_entry)) != SUCCESS) {
	    return (status);
	}

	(void) bzero ((char *) devget, sizeof(*devget));
	/*
	 * We open the device just to get driver device information.
	 */
	if ( (status = OpenScuDevice (scu)) != SUCCESS) {
	    return (status);
	}
	if ((status = ioctl (scu->scu_fd, DEVIOCGET, (char *) devget)) < 0) {
	    Perror ("DEVIOCGET failed on device '%s'", scu->scu_device_entry);
	    (void) close (scu->scu_fd);
	    scu->scu_fd = -1;
	    ExitStatus = status;
	    return (status);
	} else if (devget->bus != DEV_SCSI) {
	    Fprintf ("Device '%s' is not attached to a SCSI bus.",
						scu->scu_device_entry);
	    (void) close (scu->scu_fd);
	    scu->scu_fd = -1;
	    status = FAILURE;
	    ExitStatus = status;
	    return (status);
	}

	/*
	 * Setup the initial device information.
	 */
	scu->scu_device_name = devget->device;
	if ( EQ (DEV_UAGT, devget->device) ) {
	    scu->scu_bus = NEXUS_NOT_SET;
	    scu->scu_target = NEXUS_NOT_SET;
	    scu->scu_lun = NEXUS_NOT_SET;
	    scu->scu_unit = NEXUS_NOT_SET;
	} else {
#ifdef notdef
	    scu->scu_bus = devget->ctlr_num;
	    scu->scu_target = devget->slave_num;
	    if ( (EQ ("rz", devget->dev_name)) ||
		 (EQ ("tz", devget->dev_name)) ) {
		scu->scu_lun = 0;
	    } else {
		scu->scu_lun = devget->unit_num & 0x07;
	    }
#endif /* notdef */
	    scu->scu_bus = devget->bus_num;
	    scu->scu_target = (devget->slave_num >> 3) & 0x07;
	    scu->scu_lun = devget->slave_num & 0x07;
	    scu->scu_unit = devget->unit_num;
	    if ((ce = FindCmdEntry (C_SHOW)) == (struct cmd_entry *) 0) {
		return (FAILURE);
	    }
	    if ((ke = FindKeyEntry (ce, 0, K_DEVICE_TYPE)) == NULL) {
		return (FAILURE);
	    }
	    status = GetDeviceType (ce, ke, NULL, scu->scu_inquiry, scu);
	    if (status != SUCCESS) {
		return (status);
	    }
	    /*
	     * Setup the default device block size (0 = variable).
	     */
	    if (scu->scu_device_type != ALL_DTYPE_SEQUENTIAL) {
		scu->scu_device_size = DEFAULT_BLOCK_SIZE;
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * FreeDeviceEntry() - Deallocate Device Entry & Buffer Resources.	*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
FreeDeviceEntry (scu)
register struct scu_device *scu;
{
	register struct scu_device *shdr = ScuDeviceList;
	register struct scu_device *sptr = shdr;

	if (scu->scu_sense_buffer != NULL) {
	    free_palign ((caddr_t) scu->scu_sense_buffer);
	    scu->scu_sense_buffer = NULL;
	}

	if (scu->scu_inquiry != NULL) {
	    free_palign ((caddr_t) scu->scu_inquiry);
	    scu->scu_inquiry = NULL;
	}

	if (scu->scu_data_buffer != NULL) {
	    free_palign ((caddr_t) scu->scu_data_buffer);
	    scu->scu_data_buffer = NULL;
	}

	if (scu->scu_tmp_buffer != NULL) {
	    free_palign ((caddr_t) scu->scu_tmp_buffer);
	    scu->scu_tmp_buffer = NULL;
	}

	if (scu->scu_iop_buffer != NULL) {
	    free_palign ((caddr_t) scu->scu_iop_buffer);
	    scu->scu_iop_buffer = NULL;
	}

	if (scu->scu_sap != NULL) {
	    (void) scmn_FreeArgsBuffer (scu->scu_sap);
	    scu->scu_sap = NULL;
	}

	/*
	 * Remove this device entry from the doubly linked list:
	 *
	 *     +------------------------------------------------+
	 *     v   +-------+       +-------+       +-------+    |
	 * Hdr---->| flink |------>| flink |------>| flink |--->|
	 *         |-------|\      |-------|\      |-------|\
	 *     |<--| blink | \<----| blink | \<----| blink | \<-+
	 *     |   +-------+       +-------+       +-------+    |
	 *     +------------------------------------------------+
	 */
	do {
	    if (sptr->scu_flink == scu) {
		scu->scu_flink->scu_blink = sptr;
		sptr->scu_flink = scu->scu_flink;
	    }
	} while ( (sptr = sptr->scu_flink) != shdr);

	free_palign ((caddr_t) scu);
}

/************************************************************************
 *									*
 * MakeDeviceEntry() - Allocate A Device Entry.				*
 *									*
 * Inputs:	None.							*
 *									*
 * Return Value:							*
 *		Returns pointer to the device entry structure.		*
 *									*
 ************************************************************************/
struct scu_device *
MakeDeviceEntry()
{
	register struct scu_device *scu;
	register struct all_req_sns_data *sdp;
	register struct scu_device *shdr = ScuDeviceList;
	register struct scu_device *sptr;

	scu = (struct scu_device *) malloc_palign (sizeof(*scu));
	if (scu == (struct scu_device *) 0) {
	    errno = ENOMEM;
	    Perror ("Failed to allocate scu_device structure.");
	    return (scu);
	}
	scu->scu_fd = -1;
	scu->scu_bus = NEXUS_NOT_SET;
	scu->scu_target = NEXUS_NOT_SET;
	scu->scu_lun = NEXUS_NOT_SET;
	scu->scu_unit = NEXUS_NOT_SET;
	scu->scu_cam_flags = CamFlags;
	scu->scu_block_size = DefaultBlockSize;
	scu->scu_compare_flag = DefaultCompareFlag;
	scu->scu_data_pattern = DefaultDataPattern;
	scu->scu_error_limit = DefaultErrorLimit;
	scu->scu_pass_limit = DefaultPassLimit;
	scu->scu_recovery_flag = TRUE;
	scu->scu_retry_limit = DefaultRetryLimit;
	scu->scu_verify_flag = TRUE;
	scu->scu_verify_length = DefaultVerifyLength;

	/*
	 * Initialize Special Argument Fields:
	 */
	scu->scu_sap = scmn_GetArgsBuffer (-1, -1, -1);
	scu->scu_device_name = DEV_UNKNOWN;
	scu->scu_device_type = ALL_DTYPE_NONE;
	scu->scu_file_flags = UagtFileFlags;
	scu->scu_retry_limit = DEFAULT_RETRY_LIMIT;

	/*
	 * Allocate the I/O parameters buffer.
	 */
	scu->scu_iop_length = IOP_BUFFER_SIZE;
	scu->scu_iop_buffer = (caddr_t) malloc_palign (scu->scu_iop_length);
	if (scu->scu_iop_buffer == (caddr_t) 0) {
	    FreeDeviceEntry (scu);
	    errno = ENOMEM;
	    Perror ("Failed to allocate I/O parameters buffer.");
	    return ((struct scu_device *) 0);
	}

	/*
	 * Allocate a default I/O data buffer.
	 */
	scu->scu_data_length = DATA_BUFFER_SIZE;
	scu->scu_data_buffer = (caddr_t) malloc_palign (scu->scu_data_length);
	if (scu->scu_data_buffer == (caddr_t) 0) {
	    FreeDeviceEntry (scu);
	    errno = ENOMEM;
	    Perror ("Failed to allocate default I/O data buffer.");
	    return ((struct scu_device *) 0);
	}

	/*
	 * Allocate a temporary I/O data buffer (for common functions).
	 */
	scu->scu_tmp_length = DATA_BUFFER_SIZE;
	scu->scu_tmp_buffer = (caddr_t) malloc_palign (scu->scu_tmp_length);
	if (scu->scu_tmp_buffer == (caddr_t) 0) {
	    FreeDeviceEntry (scu);
	    errno = ENOMEM;
	    Perror ("Failed to allocate temporary I/O data buffer.");
	    return ((struct scu_device *) 0);
	}

	/*
	 * Allocate the inquiry data buffer.
	 */
	scu->scu_inquiry =
	    (struct all_inq_data *) malloc_palign (sizeof(*scu->scu_inquiry));
	if (scu->scu_inquiry == (struct all_inq_data *) 0) {
	    FreeDeviceEntry (scu);
	    errno = ENOMEM;
	    Perror ("Failed to allocate inquiry data structure.");
	    return ((struct scu_device *) 0);
	}

	/*
	 * Allocate the sense buffer.
	 */
	scu->scu_sense_length = sizeof(*sdp);
	scu->scu_sense_buffer = malloc_palign (sizeof(*sdp));
	if (scu->scu_sense_buffer == NULL) {
	    FreeDeviceEntry (scu);
	    errno = ENOMEM;
	    Perror ("Failed to allocate sense data structure.");
	    return ((struct scu_device *) 0);
	}

	/*
	 * Add this device entry to our doubly linked list:
	 *
	 *     +------------------------------------------------+
	 *     v   +-------+       +-------+       +-------+    |
	 * Hdr---->| flink |------>| flink |------>| flink |--->|
	 *         |-------|\      |-------|\      |-------|\
	 *     |<--| blink | \<----| blink | \<----| blink | \<-+
	 *     |   +-------+       +-------+       +-------+    |
	 *     +------------------------------------------------+
	 */
	if (shdr == (struct scu_device *) 0) {
	    ScuDeviceList = shdr = scu;
	    shdr->scu_flink = scu;
	    shdr->scu_blink = scu;
	} else {
	    sptr = shdr->scu_blink;
	    sptr->scu_flink = scu;
	    scu->scu_blink = sptr;
	    scu->scu_flink = shdr;
	    shdr->scu_blink = scu;
	}
	return (scu);
}

/************************************************************************
 *									*
 * ResetDeviceEntry() - Reset Volitile Device Entry Information.	*
 *									*
 * Description:								*
 *	This function re-initializes the volitile device entry fields	*
 * which may change when the device nexus ot device parameters change.	*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *		level = Soft for parameter changes (zero), 		*
 *		Hard (non-zero) for nexus changes.			*
 *									*
 * Return Value:							*
 *		Returns pointer to the device entry structure.		*
 *									*
 ************************************************************************/
void
ResetDeviceEntry (scu, level)
register struct scu_device *scu;
u_char level;
{

        if (level == HARD) {
	    scu->scu_unit = NEXUS_NOT_SET;
	    scu->scu_device_entry = NULL;
	    scu->scu_device_name = DEV_UNKNOWN;
	}
	
	scu->scu_device_size = 0;
	scu->scu_device_capacity = 0;
	scu->scu_error_control = 0;
	FreeDefaultPages (scu);
}

/************************************************************************
 *									*
 * SwitchDeviceEntry() - Switch to a New or Existing Device Entry.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SwitchDeviceEntry (ce)
register struct cmd_entry *ce;
{
	register struct scu_device *shdr = ScuDeviceList;
	register struct scu_device *scu = shdr;
	char *device_name = (char *) *(long *) ce->c_argp;
	int status;

	/*
	 * If a device name isn't specified, switch to previous entry.
	 */
	if (device_name == NULL) {
	    if (ScuPrevious != (struct scu_device *) 0) {
		scu = ScuPrevious;
		ScuPrevious = ScuDevice;
		ScuDevice = scu;
		PrintSwitchEntry (scu);
		return (SUCCESS);
	    } else {
		Fprintf ("Warning, no previous device entry to switch to.");
		return (FAILURE);
	    }
	}

	/*
	 * See if we already have an entry for this device.
	 */
	do {
	    if (scu->scu_device_entry == NULL) {
		continue;	/* Not setup if using default /dev/cam.	*/
	    }
	    if ( EQSC (scu->scu_device_entry, device_name) ||
		 EQSC (scu->scu_device_name, device_name) ) {
		ScuPrevious = ScuDevice;
		ScuDevice = scu;
		PrintSwitchEntry (scu);
		return (SUCCESS);
	    }
	} while ((scu = scu->scu_flink) != shdr);

	/*
	 * Validate the device name specified.
	 */
	if ((status = ValidateDevice (device_name)) != SUCCESS) {
	    return (status);
	}

	if ((scu = MakeDeviceEntry()) == (struct scu_device *) 0) {
	    return (FAILURE);
	}

	*(long *)ce->c_argp = NULL;		/* To keep allocation.	*/
	scu->scu_device_entry = device_name;
	if ((status = DoDeviceSetup (scu)) != SUCCESS) {
	    FreeDeviceEntry (scu);
	    return (status);
	}

	ScuPrevious = ScuDevice;		/* Save previous entry.	*/
	ScuDevice = scu;			/* Set default entry.	*/
	PrintSwitchEntry (scu);
	return (status);
}

/************************************************************************
 *									*
 * PrintSwitchEntry() - Display Device Entry Switching To.		*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
PrintSwitchEntry (scu)
register struct scu_device *scu;
{
	if (DisplayVerbose) {
	    Printf ("Switching to device entry %s (%s)...\n",
			scu->scu_device_entry, scu->scu_device_name);
	}
}

/************************************************************************
 *									*
 * ValidateDevice() - Validate The Device Name Specified.		*
 *									*
 * Inputs:	device_name = The device name pointer.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ValidateDevice (device_name)
register char *device_name;
{
	struct stat statbuf;
	register struct stat *sb = &statbuf;
	int status;

	/*
	 * Check for a valid device is being specified.
	 */
	if ((status = stat (device_name, sb) < 0)) {
	    Perror ("stat");
	    return (status);
	}

	if ((sb->st_mode & S_IFMT) != S_IFCHR) {
	    Fprintf ("Device '%s' is NOT a character device entry.",
							device_name);
	    return (FAILURE);
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * DoIoctl()	Do An I/O Control Command.				*
 *									*
 * Description:								*
 *	This function issues the specified I/O control command to the	*
 * device driver.							*
 *									*
 * Inputs:	cmd = The I/O control command.				*
 *		argp = The command argument to pass.			*
 *		msgp = The message to display on errors.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoIoctl (cmd, argp, msgp)
int cmd;
caddr_t argp;
caddr_t msgp;
{
	register struct scu_device *scu = ScuDevice;
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;
	register struct all_req_sns_data *sdp;

	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;
	bzero ((char *) sp, sizeof(*sp));
	bzero ((char *) sdp, sizeof(*sdp));
	sp->sp_sub_command = cmd;
#ifdef notdef
	sp->sp_sense_length = sizeof(*sdp);
	sp->sp_sense_buffer = (caddr_t) sdp;
#endif notdef
	sp->sp_iop_length = ((cmd & ~(IOC_INOUT|IOC_VOID)) >> 16);
	sp->sp_iop_buffer = argp;
	return (DoSpecial (sp, msgp));
}

/************************************************************************
 *									*
 * DoSpecial()	Do A Special I/O Control Command.			*
 *									*
 * Description:								*
 *	This function issues the specified special I/O control command	*
 * to the device driver.						*
 *									*
 * Inputs:	sp = The special I/O arguments.				*
 *		msgp = The message to display on errors.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoSpecial (sp, msgp)
register struct scsi_special *sp;
caddr_t msgp;
{
	register struct scu_device *scu = ScuDevice;
	register struct special_args *sap = scu->scu_sap;
	register struct all_req_sns_data *sdp;
	register struct mtop *mtop;
	int status;

	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;
	(void) bzero ((char *) sdp, sizeof(*sdp));
	sp->sp_sense_length = sizeof(*sdp);
	sp->sp_sense_buffer = (caddr_t) sdp;
#if defined(IOCTL)
	if (sp->sp_sub_command == MTIOCTOP) {
	    mtop = (struct mtop *) sp->sp_iop_buffer;
	    status = ioctl (scu->scu_fd, MTIOCTOP, mtop);
	} else {
	    status = ioctl (scu->scu_fd, SCSI_SPECIAL, sp);
	}
	if (status != SUCCESS) {
#else /* !defined(IOCTL) */
	(void) bzero ((char *) sap, sizeof(*sap));
	sap->sa_flags = SpecialFlags;
	if (CamRetrysFlag == FALSE) sap->sa_flags |= SA_NO_ERROR_RECOVERY;
	sap->sa_bus = scu->scu_bus;
	sap->sa_target = scu->scu_target;
	sap->sa_lun = scu->scu_lun;
	sap->sa_device_name = scu->scu_device_name;
	sap->sa_device_type = scu->scu_device_type;
	sap->sa_file_flags = scu->scu_file_flags;
	sap->sa_retry_limit = scu->scu_retry_limit;
	sap->sa_sense_length = scu->scu_sense_length;
	sap->sa_sense_buffer = scu->scu_sense_buffer;
	if ( (sap->sa_timeout = scu->scu_timeout) == 0) {
	    sap->sa_timeout = (int) CamTimeout;
	}
	sap->sa_start = (int (*)()) xpt_action;

	/*
	 * Check for magtape I/O operations and map acordingly.
	 */
	if (sp->sp_sub_command == MTIOCTOP) {
	    mtop = (struct mtop *) sp->sp_iop_buffer;
	    sap->sa_ioctl_cmd = MTIOCTOP;
	    sap->sa_ioctl_data = (caddr_t) mtop;
	    sap->sa_ioctl_scmd = mtop->mt_op;
	} else {
	    sap->sa_ioctl_cmd = SCSI_SPECIAL;
	    sap->sa_ioctl_data = (caddr_t) sp;
	}

	errno = 0;
	if ((status = scmn_SpecialCmd (sap)) != SUCCESS) {
	    if (errno == 0) errno = status;
	    status = FAILURE;
#endif /* defined(IOCTL) */
	    /*
	     * At the present time, the User Agent returns EIO when the
	     * user aborts the request via an interrupt signal.  This is
	     * being changed to EINTR in the next CAM release.
	     */
	    if (errno != EINTR) {
		/*
		 * When the PerrorFlag is TRUE, we map recovered errors to
		 * success.  If the PerrorFlag is FALSE, we presume a test
		 * command is in progress, and the diagnostic function will
		 * properly decode and handle the recovered error condition.
		 */
		if ( (PerrorFlag == TRUE) && (errno == EIO) &&
		     (sdp->sns_key == ALL_RECOVER_ERR) ) {
		    status = SUCCESS;
		} else {
		    Perror ("'%s' failed", msgp);
		    if ( ((PerrorFlag == TRUE) && !CmdInterruptedFlag &&
			 (sdp->sns_key != ALL_NO_SENSE)) || DebugFlag) {
			DumpSenseData (sdp);
		    }
		}
	    }
	} else {
	    if (DebugFlag && (sp->sp_xfer_resid)) {
		Printf ("Command '%s' completed with residual count of %d.\n",
						msgp, sp->sp_xfer_resid);
	    }
	}
	scu->scu_xfer_resid = sp->sp_xfer_resid;
	return (status);
}

/************************************************************************
 *									*
 * CloseFile() - Close an Input/Output File.				*
 *									*
 * Inputs:	fp = Pointer to file pointer.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
CloseFile (fp)
FILE **fp;
{
	if (*fp != (FILE *) 0) {
	    (void) fclose (*fp);
	    *fp = (FILE *) 0;
	}
}

/************************************************************************
 *									*
 * OpenStartupFile() - Open a Startup Initialization File.		*
 *									*
 * Description:								*
 *	This function attempts to open the initialization file in the	*
 * current directory and the home directory.				*
 *									*
 * Inputs:	fp = Pointer to file pointer.				*
 *		file = The input file name.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
OpenStartupFile (fp, file)
FILE **fp;
char *file;
{
	struct stat statbuf;
	register struct stat *sb = &statbuf;
	register char *rcfile = WrkBufPtr;
	int status;

	(void) sprintf (rcfile, ".%src", file);	
	if ( (status = stat (rcfile, sb)) == FAILURE) {
	    (void) sprintf (rcfile, "%s/.%src", Home, file);	
	    if ( (status = stat (rcfile, sb)) == FAILURE) {
		return (status);
	    }
	}

	return (OpenInputFile (fp, rcfile));
}

/************************************************************************
 *									*
 * SourceInputFile() - Source Input from a File.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SourceInputFile (ce)
struct cmd_entry *ce;
{
	char filename_buffer[MAXPATHLEN];
	struct stat statbuf;
	register struct stat *sb = &statbuf;
	register char *fnp = filename_buffer;
	register char *sourcefile = (char *) *(long *) ce->c_argp;
	int status;

	/*
	 * Logic:
	 *   o	If .scu extension was specified, then attempt to locate
	 *	the specified database file.
	 *   o	If .scu was not specified, then attempt to locate file
	 *	with .scu extension first, and if that fails, attempt to
	 *	locate the file without .scu extension.
	 */
	if ( EQSC (sourcefile, ".scu") ) {
	    (void) strcpy (fnp, sourcefile);
	    status = stat (fnp, sb);
	} else {
	    (void) sprintf (fnp, "%s.scu", sourcefile);
	    if ( (status = stat (fnp, sb)) == FAILURE) {
		(void) strcpy (fnp, sourcefile);
		status = stat (fnp, sb);
	    }
	}

	if (status == FAILURE) {
	    Perror ("Unable to locate source file '%s'", fnp);
	    return (status);
	}

	return (OpenInputFile (&InputFp, fnp));
}

/************************************************************************
 *									*
 * OpenInputFile() - Open an Input File to Read Commands From.		*
 *									*
 * Inputs:	fp = Pointer to file pointer.				*
 *		file = The input file name.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
OpenInputFile (fp, file)
FILE **fp;
char *file;
{
	if (*fp != (FILE *) 0) {
	    CloseFile (fp);
	}

	if ( (*fp = fopen (file, "r")) == (FILE *) 0) {
	    Perror ("Unable to open input file '%s'", file);
	    return (FAILURE);
	}
	(void) setlinebuf (*fp);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * OpenOutputFile() - Open an Output File.				*
 *									*
 * Inputs:	fp = Pointer to file pointer.				*
 *		file = Pointer to output file name.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
OpenOutputFile (fp, file)
FILE **fp;
char *file;
{
	if (*fp != (FILE *) 0) {
	    CloseFile (fp);
	}

	if ( (*fp = fopen (file, "w")) == (FILE *) 0) {
	    Perror ("Unable to open output file '%s'", file);
	    return (FAILURE);
	}
	(void) setlinebuf (*fp);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * OpenPager() - Open a Pipe to a Screen Paging Filter.			*
 *									*
 * Inputs:	pager = The output pager to display output.		*
 *			"" or NULL = Use the default pager.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / WARNING / FAILURE.		*
 *									*
 ************************************************************************/
int
OpenPager (pager)
char *pager;
{
	static int check_pager_flag = TRUE;
	char *default_pager = "more -d";
	char *pstr;
	int status;

	/*
	 * Only page the output if stdout is a terminal device.
	 */
	if ( (PagingFlag == FALSE) || (isatty (fileno (stdout)) == FALSE) ) {
	    return (WARNING);
	}

	if ( (pager == NULL) || (strlen(pager) == 0) ) {
	    if ( (pstr = OutputPager) == NULL) {
		pstr = default_pager;
	    }
	} else {
	    pstr = pager;
	}

	/*
	 * Do a simple test to ensure both the shell and the pager
	 * exist before opening a pipe, since popen() does not fail
	 * on these errors.  This is needed for single-user mode.
	 */
	if (check_pager_flag == TRUE) {
	    char scmd[STRING_BUFFER_SIZE];
	    (void) sprintf (scmd, "sh -c %s </dev/null >/dev/null 2>&1", pstr);
	    if ((status = system (scmd)) != SUCCESS) {
		if (status == FAILURE) {
		    Perror ("system() failed checking pager");
		} else if (DebugFlag) {
		    Fprintf (
	"Failed to locate shell or pager '%s', disabling paging...", pstr);
		}
		PagingFlag = FALSE;
		return (WARNING);
	    }
	    check_pager_flag = FALSE;
	}

	PipeFp = Popen (pstr, "w");

	return ( (PipeFp != NULL) ? SUCCESS : FAILURE);
}

void
ClosePager()
{
	if (PipeFp != NULL) {
	    Pclose (&PipeFp);
	}
}

/************************************************************************
 *									*
 * OpenScuDevice() - Open a SCSI Device Entry.				*
 *									*
 * Inputs:	scu = Pointer to SCSI device entry to open.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
OpenScuDevice (scu)
register struct scu_device *scu;
{
	int status = SUCCESS;
	int mode;

	scu->scu_file_flags = (FREAD | FWRITE);
	mode = (O_RDWR | O_NDELAY);
	if ( (scu->scu_fd = open (scu->scu_device_entry, mode)) < 0) {
	    mode = (O_RDONLY | O_NDELAY);
	    if ( (scu->scu_fd = open (scu->scu_device_entry, mode)) < 0) {
		Perror ("Unable to open device '%s'", scu->scu_device_entry);
		status = FAILURE;
	    } else {
		scu->scu_file_flags = FREAD;
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * itoa()	Convert Integer to ASCII String.			*
 *									*
 * Inputs:	num = The number to convert.				*
 *									*
 * Return Value:							*
 *		Returns pointer to converted number.			*
 *									*
 ************************************************************************/
char *
itoa (number)
register int number;
{
	(void) sprintf (number_buffer, "%ld", number);
	return (number_buffer);
}

/************************************************************************
 *									*
 * SeekPosition() - Seek To A Logical Block Address.			*
 *									*
 * Inputs:	lba = Logical Block Address to seek to.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SeekPosition (lba)
u_long lba;
{
	register struct key_entry *ke;

	if ((ke = FindKeyEntry (NULL, C_SET, K_SEEK_POSITION)) == NULL) {
		return (FAILURE);
	}
	return (DoIoctl (ke->k_cmd, (caddr_t)&lba, ke->k_msgp));
}

/************************************************************************
 *									*
 * SleepTime() - Sleep for Number of Seconds.				*
 *									*
 * Implicit Inputs:							*
 *	SleepValue = Number of seconds to sleep.			*
 *									*
 * Return Value:							*
 *	If the sleep() function returns because the requested time	*
 * has elapsed, 0 (zero) is returned.  If the sleep() function returns	*
 * because a signal was caught, the amount of time still remaining to	*
 * be "slept" is returned.						*
 *									*
 ************************************************************************/
int
SleepTime (struct cmd_entry *ce)
{
	return (sleep ((unsigned int )SleepValue));
}
