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
static char *rcsid = "@(#)$RCSfile: scu_sense.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/11/03 21:22:05 $";
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
 * File:	scu_sense.c - 'scu' Request Sense Functions.
 * Author:	Robin T. Miller
 * Date:	December10, 1990
 *
 * Description:
 *	This file contains functions for displaying the request sense
 * data after an error occurs.
 */

#include <sys/types.h>
#include <io/common/iotypes.h>
#include <io/cam/rzdisk.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>

#undef SUCCESS
#undef FATAL_ERROR

#include "scu.h"
#include "scu_device.h"

/*
 * External Declarations:
 */
extern caddr_t cdbg_SenseMessage();
extern char *cdbg_SenseKeyTable[];
extern void Fprintf(), FprintC(), Printf();
extern void PrintHeader (char *header);

/*
 * Forward References:
 */
void DumpSenseData (struct all_req_sns_data *sdp);
void DumpSenseBrief (struct all_req_sns_data *sdp);
void DumpSenseFull (struct all_req_sns_data *sdp);

/************************************************************************
 *									*
 * ReportDirectError() - Report Direct Access Device Error Information.	*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *		op  = The operation type.				*
 *		lbn = Address to store logical block number.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 / 1 = SUCCESS / FAILURE / WARNING	*
 *									*
 ************************************************************************/
int
ReportDirectError (scu, op, lbn)
struct scu_device *scu;
char *op;
U32 *lbn;
{
	struct all_req_sns_data *sdp;
	U32 lba = 0;

	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;
	if (sdp->sns_key == ALL_NO_SENSE) {
	    scu->scu_error_count++;
	    return (FAILURE);		/* No sense, presume CCB error.	*/
	}

	if (sdp->valid) {
	    lba = (U32)( ((U32)sdp->info_byte3 << 24) +
			 ((U32)sdp->info_byte2 << 16) +
			 ((U32)sdp->info_byte1 << 8) +
			  (sdp->info_byte0) );
	}

	*lbn = lba;

	/*
	 * We expect only recoverable or medium errors, all others are
	 * considered fatal (abort the command).
	 */
	if ( (sdp->sns_key != ALL_RECOVER_ERR) &&
	     (sdp->sns_key != ALL_MEDIUM_ERR) ) {
	    DumpSenseData (sdp);
	    scu->scu_error_count++;
	    return (FAILURE);
	}

	/*
	 * The valid bit indicates the information bytes are valid.
	 */
	if (sdp->valid) {
	    Fprintf ("%s error at logical block number %u (%#x).",
							op, lba, lba);
	    DumpSenseBrief (sdp);
	} else if (sdp->sns_key != ALL_MEDIUM_ERR) {
	    DumpSenseData (sdp);
	    scu->scu_error_count++;
	    return (FAILURE);
	}

	/*
	 * Don't count recoverable errors.
	 */
	if (sdp->sns_key == ALL_RECOVER_ERR) {
	    return (SUCCESS);
	} else {
	    scu->scu_error_count++;
	}
	return (WARNING);	/* Using warning to flag LBA error. */
}

/************************************************************************
 *									*
 * ReportSequentialError() - Report Sequential Access Device Error.	*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *		op  = The operation type.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 / 1 = SUCCESS / FAILURE / WARNING.	*
 *									*
 ************************************************************************/
int
ReportSequentialError (scu, op, resid)
struct scu_device *scu;
char *op;
U32 *resid;
{
	struct all_req_sns_data *sdp;
	U32 xfer_resid = 0;

	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;

	/*
	 * The valid bit indicates the information bytes are valid.
	 */
	if (sdp->valid) {
	    xfer_resid = (U32)( ((U32)sdp->info_byte3 << 24) +
				((U32)sdp->info_byte2 << 16) +
				((U32)sdp->info_byte1 << 8) +
				 (sdp->info_byte0) );
	}

	*resid = xfer_resid;

	/*
	 * File marks and end of media are expected errors, but since we
	 * want to terminate further requests, we return warning status.
	 */
	if ( (sdp->sns_key == ALL_NO_SENSE) &&
	     (sdp->eom || sdp->filemark) ) {
	    return (WARNING);
	}

	/*
	 * If the error was an illegal length indicator, return success to
	 * accept the data transferred based on the residual count.
	 */
	if ( (sdp->sns_key == ALL_NO_SENSE) && sdp->ili) {
	    return (SUCCESS);
	}

	if (sdp->sns_key == ALL_NO_SENSE) {
	    scu->scu_error_count++;
	    return (FAILURE);		/* No sense, presume CCB error.	*/
	}

	/*
	 * We expect only recoverable or medium errors, all others are
	 * considered fatal (abort the command).
	 */
	if ( (sdp->sns_key != ALL_RECOVER_ERR) &&
	     (sdp->sns_key != ALL_MEDIUM_ERR) ) {
	    DumpSenseData (sdp);
	    scu->scu_error_count++;
	    return (FAILURE);
	}

	DumpSenseBrief (sdp);

	/*
	 * Return SUCCESS on recoverable errors, to continue normally.
	 * Otherwise return FAILURE to abort the current operation.
	 */
	if (sdp->sns_key == ALL_RECOVER_ERR) {
	    return (SUCCESS);
	} else {
	    scu->scu_error_count++;
	    return (FAILURE);
	}
}

/************************************************************************
 *									*
 * ShowSense() - Show The Request Sense Data.				*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Outputs:	None.							*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
ShowSense (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct scu_device *scu = ScuDevice;
	register struct all_req_sns_data *sdp;

	/*
	 * Simply display previously returned sense data.
	 */
	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;
	DumpSenseFull (sdp);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * DumpSenseData() - Dump The Request Sense Data.			*
 *									*
 * Description:								*
 *	This function simply dispatches to the appropriate funtion to	*
 * dump sense data in either brief or full format (user controls this).	*
 *									*
 * Inputs:	sdp = Sense data pointer.				*
 *									*
 * Outputs:	None.							*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
DumpSenseData (sdp)
register struct all_req_sns_data *sdp;
{
	if (FullErrorReport || DebugFlag) {
	    DumpSenseFull (sdp);
	} else {
	    DumpSenseBrief (sdp);
	}
}

/************************************************************************
 *									*
 * DumpSenseBrief() - Dump Request Sense Data in Brief Format.		*
 *									*
 * Inputs:	sdp = Sense data pointer.				*
 *									*
 * Outputs:	None.							*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
DumpSenseBrief (sdp)
register struct all_req_sns_data *sdp;
{
	char *ascq_msg;

	if ( (ascq_msg = cdbg_SenseMessage(sdp)) != (char *) 0) {
	    Fprintf ("Sense Key = %#x = %s,",
			sdp->sns_key, cdbg_SenseKeyTable[sdp->sns_key]);
	    FprintC ("Sense Code/Qualifier = (%#x, %#x) = %s",
			sdp->asc, sdp->asq, ascq_msg);
	} else {
	    Fprintf ("Sense Key = %d = %s, Sense Code/Qualifier = (%#x, %#x)",
			sdp->sns_key, cdbg_SenseKeyTable[sdp->sns_key],
			sdp->asc, sdp->asq);
	}
}

/************************************************************************
 *									*
 * DumpSenseFull() - Dump All Request Sense Data.			*
 *									*
 * Inputs:	sdp = Sense data pointer.				*
 *									*
 * Outputs:	None.							*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
DumpSenseFull (sdp)
register struct all_req_sns_data *sdp;
{
	register int sense_length = (int) sdp->addition_len + 7;

	PrintHeader ("Request Sense Information");

	PrintHex ("Error Code", (sdp->error_code & 0x0f), PNL);
	PrintHex ("Error Class", (sdp->error_code >> 4) & 0x07, PNL);
	PrintHex ("Valid Bit", sdp->valid, PNL);
	PrintHex ("Segment Number", sdp->segment, PNL);
	PrintHex ("Sense Key", sdp->sns_key, DNL);
	Printf (" - %s\n", cdbg_SenseKeyTable[sdp->sns_key]);
	PrintHex ("Illegal Length", sdp->ili, PNL);
	PrintHex ("End Of Media", sdp->eom, PNL);
	PrintHex ("File Mark", sdp->filemark, PNL);
	PrintHex ("Information Byte 3", sdp->info_byte3, PNL);
	PrintHex ("Information Byte 2", sdp->info_byte2, PNL);
	PrintHex ("Information Byte 1", sdp->info_byte1, PNL);
	PrintHex ("Information Byte 0", sdp->info_byte0, PNL);
	PrintHex ("Additional Sense Length", sdp->addition_len, PNL);

	if ( (sense_length -= 8) > 0) {
	    PrintHex ("Command Information Byte 3", sdp->cmd_specific3, PNL);
	    PrintHex ("Command Information Byte 2", sdp->cmd_specific2, PNL);
	    PrintHex ("Command Information Byte 1", sdp->cmd_specific1, PNL);
	    PrintHex ("Command Information Byte 0", sdp->cmd_specific0, PNL);
	    sense_length -= 4;
	}

	if (sense_length > 0) {
	    char *ascq_msg;

	    PrintHex ("Additional Sense Code", sdp->asc, PNL);
	    PrintHex ("Sense Code Qualifier", sdp->asq, PNL);

	    ascq_msg = cdbg_SenseMessage (sdp);
	    PrintAscii ("Sense Code/Qualifier Message",
			( (ascq_msg) ? ascq_msg : "Unknown"), PNL);
	    sense_length -= 2;
	}

	if (sense_length-- > 0) {
	    PrintHex ("Field Replaceable Unit Code", sdp->fru, PNL);
	}

	/*
	 * Sense specific sense data.
	 */
	if (sense_length > 0) {
	    struct all_sks_ill_req *sksi;
	    sksi = (struct all_sks_ill_req *)&sdp->sense_specific.sks_ill_req;
	    if (sksi->sksv && (sdp->sns_key == ALL_ILLEGAL_REQ) ) {
		PrintHex ("Bit Pointer to Field in Error",
						sksi->bit_pointer, PNL);
		PrintHex ("Bit Pointer Field Valid (BPV)", sksi->bpv, PNL);
		PrintHex ("Error Field Command/Data (C/D)", sksi->c_or_d, PNL);
		PrintHex ("Field Pointer Field (MSB)", sksi->field_ptr1, PNL);
		PrintHex ("Field Pointer Field (LSB)", sksi->field_ptr0, PNL);
		PrintHex ("Sense-Key Specific Field Valid", sksi->sksv, PNL);
	    } else if (sksi->sksv && ( (sdp->sns_key == ALL_RECOVER_ERR) ||
				       (sdp->sns_key == ALL_HARDWARE_ERR) ||
				       (sdp->sns_key == ALL_MEDIUM_ERR) ) ) {
		struct all_sks_retry_cnt *sksr;
		sksr = (struct all_sks_retry_cnt *) sksi;
		PrintHex ("Sense-key Specific Field Valid", sksr->sksv, PNL);
		PrintHex ("Actual Retry Count (MSB)", sksr->retry_cnt1, PNL);
		PrintHex ("Actual Retry Count (LSB)", sksr->retry_cnt0, PNL);
	    } else {
		int sks_length = sizeof(*sksi);
		u_char *ssbp = (u_char *) sksi;
		PrintAscii ("Sense Specific Bytes", "", DNL);
		do {
		    Printf ("%#x ", *ssbp++);
		} while (--sks_length);
		Printf ("\n");
	    }
	    sense_length -= 3;
	}
	/*
	 * Additional sense bytes (if any);
	 */
	if (sense_length > 0) {
	    u_char *asbp = (u_char *)sdp->additional_sense.other_sns;
	    PrintAscii ("Additional Sense Bytes", "", DNL);
	    do {
		Printf ("%#x ", *asbp++);
	    } while (--sense_length);
	    Printf ("\n");
	}
}
