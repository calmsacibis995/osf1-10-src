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
static char *rcsid = "@(#)$RCSfile: scu_format.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/06/25 10:28:45 $";
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
 * File:	scu_format.c
 * Author:	Robin T. Miller
 * Date:	September 2, 1991
 *
 * Description:
 *	This file contains routines to support the SCSI Format Command.
 */

#include <stdio.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <io/cam/rzdisk.h>
#include <sys/param.h>
#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"
#include "scu_pages.h"
#include "scsipages.h"

/*
 * External Declarations:
 */
extern struct mode_page_funcs *mode_page_table[];
extern int IS_Mounted (struct scu_device *scu);
extern void Printf();
extern void ResetDeviceEntry (struct scu_device *scu, u_char level);

/*
 * Page Parameters for Diskette Format Operation:
 */
struct direct_params {				/* Direct Access Format	*/
	struct mode_page_header header;
	struct scsi_direct_access page3;
};

struct flexible_params {			/* Flexible Disk Page.	*/
	struct mode_page_header header;
	struct scsi_flexible_disk page5;
};

/*
 * Table of Supported Diskette Densities:
 */
struct density_params density_table[] = {
    {	/* RX50 Density Parameters */
	/* heads = */		1,	/* sector/track = */	10,
	/* data_sector = */	512,	/* cylinders = */	80,
	/* step_pulse_cyl = */	0,	/* transfer_rate = */	250
    },
    {	/* Low Density (LD) Parameters */
	/* heads = */		2,	/* sector/track = */	9,
	/* data_sector = */	512,	/* cylinders = */	40,
	/* step_pulse_cyl = */	0,	/* transfer_rate = */	250
    },
    {	/* Double Density (DD) Parameters */
	/* heads = */		2,	/* sector/track = */	9,
	/* data_sector = */	512,	/* cylinders = */	80,
	/* step_pulse_cyl = */	0,	/* transfer_rate = */	250
    },
    {	/* High Density (HD 3.5") Parameters */
	/* heads = */		2,	/* sector/track = */	18,
	/* data_sector = */	512,	/* cylinders = */	80,
	/* step_pulse_cyl = */	0,	/* transfer_rate = */	500
    },
    {	/* High Density (HD 5.25") Parameters */
	/* heads = */		2,	/* sector/track = */	15,
	/* data_sector = */	512,	/* cylinders = */	80,
	/* step_pulse_cyl = */	0,	/* transfer_rate = */	500
    },
    {	/* Extra Density (ED) Parameters */
	/* heads = */		2,	/* sector/track = */	36,
	/* data_sector = */	512,	/* cylinders = */	80,
	/* step_pulse_cyl = */	0,	/* transfer_rate = */	1000
    },
    {	/* Other Density (?D) Parameters */
	/* heads = */		2,	/* sector/track = */	0,
	/* data_sector = */	512,	/* cylinders = */	0,
	/* step_pulse_cyl = */	0,	/* transfer_rate = */	0
    }
};

/************************************************************************
 *									*
 * FormatUnit() - Formats A Disk or Diskette Device.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
FormatUnit (ce)
register struct cmd_entry *ce;
{
	struct scu_device *scu = ScuDevice;
	int status;

	/*
	 * Don't permit formatting if a file system is mounted.
	 */
	if ((status = IS_Mounted (scu)) != FALSE) {
	    return (FAILURE);
	}

	/*
	 * Must attempt to read flexible page to make distinction.
	 */
	Formatting = TRUE;
	if ( IS_FlexibleDisk() == TRUE ) {
	    status = FormatFlexible (ce, scu);
	} else {
	    status = FormatDisk (ce, scu);
	}
	Formatting = FALSE;

	/*
         * Formatting changes device parameters, so reset current ones.
	 */
	ResetDeviceEntry (scu, SOFT);

	return (status);
}

/************************************************************************
 *									*
 * FormatDisk() - Formats Hard Disk Drives.				*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
FormatDisk (ce, scu)
register struct cmd_entry *ce;
struct scu_device *scu;
{
	struct defect_descriptors dd;
	struct format_params format_params;
	register struct format_params *fp = &format_params;

	(void) bzero ((char *)&dd, sizeof(dd));
	(void) bzero ((char *)fp, sizeof(*fp));

	fp->fp_format = BLK_FORMAT;
	fp->fp_interleave = 0;
	fp->fp_pattern = 0;
	fp->fp_length = 0;
	fp->fp_addr = (u_char *)&dd;

	/*
	 * Determine the defect list(s) for formatting.
	 */
	if ( ISSET_KOPT (ce, K_VENDOR_DEFECTS) ) {
	    fp->fp_defects = VENDOR_DEFECTS;
	} else if ( ISSET_KOPT (ce, K_NO_DEFECTS) ) {
	    fp->fp_defects = NO_DEFECTS;
	    fp->fp_addr = (u_char *) 0;
	} else {
	    fp->fp_defects = KNOWN_DEFECTS;
	}

	/*
	 * Initialize the defect header and the defect list. Note 
 	 * that there is no defect list being sent and therefore 
	 * the defect list length is set to zero (0).
	 */
	dd.dd_header.fu_hdr.vu = 0;
	dd.dd_header.fu_hdr.dcrt = 0;
	dd.dd_header.fu_hdr.dpry = 0;

	if (DisplayVerbose) {
	    Printf ("Formatting device %s (%s), please be patient...\n",
		    		scu->scu_device_entry, scu->scu_device_name);
	}

	return (DoIoctl (ce->c_cmd, (caddr_t)fp, ce->c_msgp));
}

/************************************************************************
 *									*
 * FormatFlexible() - Formats Flexible Device Media.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
FormatFlexible (ce, scu)
register struct cmd_entry *ce;
struct scu_device *scu;
{
	struct format_params format_params;
	register struct format_params *fp = &format_params;
	struct dir_read_cap_data capacity_data;
	register struct dir_read_cap_data *capacity = &capacity_data;
	int saved_PerrorFlag, status;

	/*
	 * First force media access for floppy, this will allow it
	 * to pick up media changes on the RX26 (others?).
	 */
	saved_PerrorFlag = PerrorFlag;
	PerrorFlag = FALSE;
	status = GetCapacity (capacity);
	PerrorFlag = saved_PerrorFlag;
	
	(void) bzero ((char *)fp, sizeof(*fp));

	/*
	 * This check is being done so if the user did not specify a
	 * particular density type, we will format using the defaults.
	 */
	if (ANY_KOPTS(ce)) {
	    if ((status = SetupDensity (ce)) != SUCCESS) {
		return (status);
	    }
	}
	fp->fp_format = BLK_FORMAT;
	fp->fp_interleave = 0;
	fp->fp_pattern = 0;
	fp->fp_defects = NO_DEFECTS;
	fp->fp_length = 0;
	fp->fp_addr = 0;

	if (DisplayVerbose) {
	    Printf ("Formatting device %s (%s), please be patient...\n",
		    		scu->scu_device_entry, scu->scu_device_name);
	}

	return (DoIoctl (ce->c_cmd, (caddr_t)fp, ce->c_msgp));
}

/************************************************************************
 *									*
 * SetupDensity() - Sets Up Flexible Diskette Density Parameters.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SetupDensity (ce)
register struct cmd_entry *ce;
{
	struct direct_params ms_direct_params;
	struct direct_params *dp = &ms_direct_params;
	register struct scsi_direct_access *da = &ms_direct_params.page3;
	struct flexible_params ms_flexible_params;
	struct flexible_params *fp = &ms_flexible_params;
	register struct scsi_flexible_disk *fd = &ms_flexible_params.page5;
	register struct density_params *dpt;
	register struct mode_page_funcs *pf;
	int status;

	/*
	 * Handle "other" diskette parameters by prompting the user
	 * for the direct access & flexible disk parameter pages.
	 *
	 * Note:  I purposely did not emulate 'rzdisk' for "other" so
	 *	  the user can enter all page parameters and have complete
	 *	  control instead of only a small subset of the parameters.
	 */
	if (DensityType == V_FORMAT_OTHER) {
	    OperationType = CHANGE_OPERATION;
	    pf = mode_page_table[DIRECT_ACCESS_PAGE];
	    /*
	     * Change the direct access device page parameters.
	     */
	    if ((status = (*pf->pf_changepage)(ce, NULL, NULL)) != SUCCESS) {
		return (status);
	    }

	    pf = mode_page_table[FLEXIBLE_DISK_PAGE];
	    /*
	     * Change the flexible disk page parameters.
	     */
	    status = (*pf->pf_changepage)(ce, NULL, NULL);
	    return (status);
	}

	dpt = &density_table[DensityType];
	pf = mode_page_table[DIRECT_ACCESS_PAGE];
	/*
	 * Obtain and setup the Direct Access Page.
	 */
	if ((status = (*pf->pf_sensepage)(dp, PCF_DEFAULT)) != SUCCESS) {
	    return (status);
	}
	da->da_sectors_track_1 = 0;
	da->da_sectors_track_0 = dpt->dp_sectors_track;
	da->da_data_sector_1 = (u_char) (dpt->dp_data_sector >> 8);
	da->da_data_sector_0 = (u_char) dpt->dp_data_sector;

	if ((status = (*pf->pf_setpage)(ce, NULL, da)) != SUCCESS) {
	    return (status);
	}

	pf = mode_page_table[FLEXIBLE_DISK_PAGE];
	/*
	 * Obtain and setup the Direct Flexible Disk Page.
	 */
	if ((status = (*pf->pf_sensepage)(fp, PCF_DEFAULT)) != SUCCESS) {
	    return (status);
	}
	fd->fd_heads = dpt->dp_heads;
	fd->fd_cylinders_1 = (u_char) (dpt->dp_cylinders >> 8);
	fd->fd_cylinders_0 = (u_char) dpt->dp_cylinders;
	fd->fd_data_sector_1 = (u_char) (dpt->dp_data_sector >> 8);
	fd->fd_data_sector_0 = (u_char) dpt->dp_data_sector;
	fd->fd_sectors_track = dpt->dp_sectors_track;
	fd->fd_spc = dpt->dp_step_pulse_cyl;
	fd->fd_transfer_rate_1 = (u_char) (dpt->dp_transfer_rate >> 8);
	fd->fd_transfer_rate_0 = (u_char) dpt->dp_transfer_rate;

	if ((status = (*pf->pf_setpage)(ce, NULL, fd)) != SUCCESS) {
	    return (status);
	}

	return (status);
}
