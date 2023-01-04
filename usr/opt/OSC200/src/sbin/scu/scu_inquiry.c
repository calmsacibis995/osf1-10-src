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
static char *rcsid = "@(#)$RCSfile: scu_inquiry.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 10:31:25 $";
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
 * File:	scu_inquiry.c
 * Author:	Robin T. Miller
 * Date:	August 8, 1991
 *
 * Description:
 *	This file contains functions for the Inquiry command.
 */

#include <sys/types.h>
#include <io/common/iotypes.h>
#include <io/cam/rzdisk.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"
#include "scu_pages.h"

/*
 * External References:
 */
extern char *GetDeviceName (int device_type);
extern void PrintHeader (char *header);

/*
 * Forward References:
 */
void PrintInquiry();

/*
 * Strings for Inquiry Data Fields:
 */
static char *bus_str =			"SCSI Bus ID";
static char *target_str =		"SCSI Target ID";
static char *target_lun_str =		"SCSI Target LUN";

static char *inquiry_str =		"Inquiry Information";
static char *dtype_str =		"Peripheral Device Type";
static char *pqual_str =		"Peripheral Qualifier";
static char *dqual_str =		"Device Type Qualifier";
static char *rmb_str =			"Removable Media";
static char *ansi_str =			"ANSI Version";
static char *ecma_str =			"ECMA Version";
static char *iso_str =			"ISO Version";
static char *rdf_str =			"Response Data Format";
static char *trmiop_str =		"Terminate I/O Process";
static char *aenc_str =			"Asynchronous Notification";
static char *addlen_str =		"Additional Length";
static char *sftre_str =		"Soft Reset Support";
static char *cmdque_str =		"Command Queuing Support";
static char *linked_str =		"Linked Command Support";
static char *sync_str =			"Synchronous Data Transfers";
static char *wbus16_str =		"Support for 16 Bit Transfers";
static char *wbus32_str =		"Support for 32 Bit Transfers";
static char *reladdr_str =		"Relative Addressing Support";

static char *pqual_table[] = {
	"Peripheral Device Connected",				/* 0x0 */
	"Peripheral Device NOT Connected",			/* 0x1 */
	"Reserved",						/* 0x2 */
	"No Physical Device Support"				/* 0x3 */
};

static char *ansi_table[] = {
	"Level 0",						/* 0x0 */
	"SCSI-1 Compliant",					/* 0x1 */
	"SCSI-2 Compliant",					/* 0x2 */
	"Reserved"					  /* 0x3 - 0x7 */
};

static char *rdf_table[] = {
	"ANSI SCSI-1",						/* 0x00 */
	"CCS",							/* 0x01 */
	"ANSI SCSI-2",						/* 0x02 */
	"Reserved"					 /* 0x03 - 0x1F */
};

/************************************************************************
 *									*
 * GetInquiry() - Get The Device Inquiry Data.				*
 *									*
 * Inputs:	inquiry = Pointer to inquiry data buffer.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
GetInquiry (inquiry)
register struct all_inq_data *inquiry;
{
	register struct key_entry *ke;
	struct mode_sel_sns_params msp;

	/*
	 * Find & execute Inquiry command to obtain device type.
	 */
	if ((ke = FindKeyEntry (NULL, C_SHOW, K_INQUIRY)) == NULL) {
	    return (FAILURE);
	}
	bzero ((char *)inquiry, sizeof(*inquiry));
	bzero ((char *)&msp, sizeof(msp));
	msp.msp_addr = (caddr_t)inquiry;
	msp.msp_length = sizeof(*inquiry);

	return (DoIoctl (ke->k_cmd, (caddr_t)&msp, ke->k_msgp));
}

/************************************************************************
 *									*
 * ShowInquiry() - Show The Device Inquiry Data.			*
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
ShowInquiry (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;
	register struct all_inq_data *inquiry = scu->scu_inquiry;
	int status;

	if ( (status = GetInquiry (inquiry)) != SUCCESS) {
		return (status);
	}

	PrintInquiry ((struct ccb_header *) 0, inquiry);

	return (status);
}

/************************************************************************
 *									*
 * PrintInquiry() - Print The Device Inquiry Data.			*
 *									*
 * Inputs:	ccbh = Pointer to CCB header (optional).		*
 *		inquiry = Pointer to inquiry data.			*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
PrintInquiry (ccbh, inquiry)
register struct ccb_header *ccbh;
register struct all_inq_data *inquiry;
{
	char	vid[sizeof(inquiry->vid)+1];
	char	pid[sizeof(inquiry->pid)+1];
	char	revlevel[sizeof(inquiry->revlevel)+1];

	PrintHeader (inquiry_str);

	if (ccbh != (struct ccb_header *) 0) {
	    PrintDecimal (bus_str, ccbh->cam_path_id, PNL);
	    PrintDecimal (target_str, ccbh->cam_target_id, PNL);
	    PrintDecimal (target_lun_str, ccbh->cam_target_lun, PNL);
	}

	PrintAscii (dtype_str, GetDeviceName(inquiry->dtype), PNL);

	if (inquiry->pqual & PQUAL_VENDOR_SPECIFIC) {
		PrintHex (pqual_str, inquiry->pqual, DNL);
		Printf (" (Vendor Specific)\n");
	} else {
		PrintAscii (pqual_str, pqual_table[inquiry->pqual], PNL);
	}

	PrintDecimal (dqual_str, inquiry->dmodify, PNL);

	PrintAscii (rmb_str, yesno_table[inquiry->rmb], PNL);

	PrintAscii (ansi_str, ansi_table[inquiry->ansi&0x03], PNL);

	PrintDecimal (ecma_str, inquiry->ecma, PNL);

	PrintDecimal (iso_str, inquiry->iso, PNL);

	PrintAscii (rdf_str, rdf_table[inquiry->rdf&0x03], PNL);

	if (inquiry->rdf == ALL_RDF_SCSI2) {
	    PrintDecimal (trmiop_str, inquiry->trmiop, PNL);
	    PrintDecimal (aenc_str, inquiry->aenc, PNL);
	}

	PrintDecimal (addlen_str, inquiry->addlen, PNL);

	if (inquiry->rdf == ALL_RDF_SCSI2) {
	    PrintDecimal (sftre_str, inquiry->sftre, PNL);
	    PrintDecimal (cmdque_str, inquiry->cmdque, PNL);
	    PrintDecimal (linked_str, inquiry->linked, PNL);
	    PrintDecimal (sync_str, inquiry->sync, PNL);
	    PrintDecimal (wbus16_str, inquiry->wbus16, PNL);
	    PrintDecimal (wbus32_str, inquiry->wbus32, PNL);
	    PrintDecimal (reladdr_str, inquiry->reladdr, PNL);
	}

	strncpy (vid, inquiry->vid, sizeof(inquiry->vid));
	vid[sizeof(inquiry->vid)] = '\0';
	PrintAscii ("Vendor Identification", vid, PNL);

	strncpy (pid, inquiry->pid, sizeof(inquiry->pid));
	pid[sizeof(inquiry->pid)] = '\0';
	PrintAscii ("Product Identification", pid, PNL);

	strncpy (revlevel, inquiry->revlevel, sizeof(inquiry->revlevel));
	revlevel[sizeof(inquiry->revlevel)] = '\0';
	PrintAscii ("Firmware Revision Level", revlevel, PNL);
}
