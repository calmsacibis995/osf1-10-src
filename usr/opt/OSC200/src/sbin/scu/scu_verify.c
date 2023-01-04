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
static char *rcsid = "@(#)$RCSfile: scu_verify.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/12/15 20:57:17 $";
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
 * File:	scu_verify.c
 * Author:	Robin T. Miller
 * Date:	September 2, 1991
 *
 * Description:
 *	This file contains routines to support the SCSI Verify Command.
 */

#include <stdio.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>

#include <io/common/iotypes.h>
#include <io/cam/rzdisk.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#include <io/cam/scsi_special.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"

/*
 * External References:
 */
extern int DoTestSetup (struct cmd_entry *ce, struct scu_device *scu);
extern int DoTestCleanup (struct cmd_entry *ce, struct scu_device *scu);
extern void Fprintf(), Printf();
extern int ReportDirectError (struct scu_device *scu, char *op, U32 *lbn);
extern int ReportSequentialError (struct scu_device *scu, char *op, U32 *resid);

/*
 * Forward References:
 */
static int TestVerifyDirect (struct cmd_entry *ce, struct scu_device *scu);
static int TestVerifySequential (struct cmd_entry *ce, struct scu_device *scu);
static int VerifySequential (struct scu_device *scu, U32 length);
int DoVerifySequential (struct scu_device *scu, U32 length);

/************************************************************************
 *									*
 * VerifyMedia() - Verify Device Media using SCSI Verify Command.	*
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
VerifyMedia (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;
	int saved_PerrorFlag, status;

	scu->scu_test_control = (TC_NO_BUFFER | TC_NO_STATISTICS);
	if ( (status = DoTestSetup (ce, scu)) != SUCCESS) {
	    return (status);
	}

	saved_PerrorFlag = PerrorFlag;
	PerrorFlag = FALSE;

	/*
	 * Do verify data for each device type.
	 */
	switch (scu->scu_device_type) {

            case ALL_DTYPE_DIRECT:
	    case ALL_DTYPE_OPTICAL:
	    case ALL_DTYPE_RODIRECT:
		status = TestVerifyDirect (ce, scu);
		break;

	    case ALL_DTYPE_SEQUENTIAL:
		status = TestVerifySequential (ce, scu);
		break;

	    default:
		Fprintf ("Verify data not implemented for %s device type.",
					GetDeviceName (scu->scu_device_type));
		status = WARNING;
		break;
	}

	PerrorFlag = saved_PerrorFlag;
	(void) DoTestCleanup (ce, scu);
	return (status);
}

/************************************************************************
 *									*
 * TestVerifyDirect() - Test Verify Data for Direct Access Devices.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
TestVerifyDirect (ce, scu)
struct cmd_entry *ce;
register struct scu_device *scu;
{
	u_long total_blocks;
	U32 blocks, data_blocks;
	U32 lba = 0;
	int status;

	/*
	 * Setup the number of data blocks per verify request.
	 */
	if ( ISSET_KOPT (ce, K_BLOCK_SIZE) ) {
	    data_blocks = howmany (scu->scu_block_size, scu->scu_device_size);
	    data_blocks = MIN (MAX_VERIFY_LENGTH, data_blocks);
	} else {
	    data_blocks = MIN (MAX_VERIFY_LENGTH, scu->scu_verify_length);
	}

	/*
	 * Verify the disk blocks.
	 */
	do {
	  lba = scu->scu_starting_lba;
	  total_blocks = scu->scu_block_limit;
	  if (DisplayVerbose) {
	    Printf ("Verifying %u block%s on %s (%s), please be patient...\n",
			total_blocks, (total_blocks == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name);
	  }
	  while ( total_blocks &&
		  (scu->scu_error_count < scu->scu_error_limit) ) {
	    if (CmdInterruptedFlag == TRUE) break;
	    blocks = MIN (total_blocks, data_blocks);
	    if (WatchProgress) {
		if (blocks > 1) {
		    Printf ("Verifying blocks [ %u through %u ]...\n",
						lba, (lba + blocks - 1) );
		} else {
		    Printf ("Verifying block %u...\n", lba);
		}
	    }
	    if ( (status = DoVerifyDirect (scu, lba, blocks)) != SUCCESS) {
		break;
	    }
	    lba += (U32) blocks;
	    total_blocks -= blocks;
	  }
	} while ( (status == SUCCESS) &&
		  (CmdInterruptedFlag == FALSE) &&
		  (++scu->scu_pass_count < scu->scu_pass_limit) &&
		  (scu->scu_error_count < scu->scu_error_limit) );

	if ( (CmdInterruptedFlag == TRUE) ||
	     (scu->scu_error_count >= scu->scu_error_limit) ) {
	    status = FAILURE;
	}
	return (status);
}

/************************************************************************
 *									*
 * DoVerifyDirect() - Verify Direct Access Device Blocks.		*
 *									*
 * Inputs:	scu    = The device unit structure.			*
 *		lba    = The starting logical block number.		*
 *		blocks = The number of blocks to verify.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
DoVerifyDirect (scu, lba, blocks)
struct scu_device *scu;
U32 lba;
U32 blocks;
{
	struct verify_params verify_data;
	register struct verify_params *vp = &verify_data;
	int cmd = SCSI_VERIFY_DATA;
	U32 lbn;
	int status;

	vp->vp_lbn = lba;		/* Start at first disk block.	*/
	/* TODO: vp_length MUST change to U32. */
	vp->vp_length = (u_short)blocks;/* Number of blocks to verify.	*/
	do {
	    status = DoIoctl (cmd, (caddr_t)vp, "verify data");
	    if (CmdInterruptedFlag == TRUE) return (FAILURE);
	    if (status == SUCCESS) break;
	    /*
	     * We've had an error, determine the type of failure.
	     */
	    status = ReportDirectError (scu, "Verify", &lbn);
	    if ( (status == SUCCESS) || (status == FAILURE) ) {
		return (status);	/* No sense or recoverable. */
	    }
	    /*
	     * Calculate remaining blocks to process.
	     */
	    lbn++;			/* Point past erroring block. */
	    status = SUCCESS;		/* Don't abort verify...  */
	    vp->vp_lbn = lbn;		/* Point past erroring block. */
	    /* TODO: vp_length MUST change to U32. */
	    if ( (vp->vp_length = (blocks - (lbn - lba))) == 0) {
		return (status);
	    }
	} while (scu->scu_error_count < scu->scu_error_limit);

	return (status);
}

/************************************************************************
 *									*
 * TestVerifySequential() - Test Verify Data for Sequential Devices.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
TestVerifySequential (ce, scu)
struct cmd_entry *ce;
register struct scu_device *scu;
{
	u_long total_bytes;
	struct all_req_sns_data *sdp;
	U32 bytes, count, verify_bytes;
	int status;

	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;
	/*
	 * Setup the number of data bytes per verify request.
	 */
	if ( ISSET_KOPT (ce, K_BLOCK_SIZE) ) {
	    verify_bytes = scu->scu_block_size;
	} else {
	    verify_bytes = scu->scu_verify_length;
	    /*
	     * For fixed block tapes, verify length is in blocks.
	     */
	    if (scu->scu_device_size) {
		verify_bytes = (verify_bytes * scu->scu_device_size);
	    }
	}

	/*
	 * For fixed block tapes, check verify length in blocks.
	 */
	if (scu->scu_device_size) {
	    u_long blocks = howmany (verify_bytes, scu->scu_device_size);
	    blocks = MIN (MAX_VERIFY_TAPE_LENGTH, blocks);
	    verify_bytes = (blocks * scu->scu_device_size);
	} else {
	    verify_bytes = MIN (MAX_VERIFY_TAPE_LENGTH, verify_bytes);
	}
	if (verify_bytes == 0) verify_bytes = MAX_VERIFY_TAPE_LENGTH;

	/*
	 * Verify the tape bytes/records.
	 */
	do {
	  scu->scu_record_count = 0L;
	  total_bytes = scu->scu_block_limit;
	  if (DisplayVerbose) {
	    Printf ("Verifying %lu byte%s on %s (%s), please be patient...\n",
			total_bytes, (total_bytes == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name);
	  }
	  while ( total_bytes &&
		  (scu->scu_error_count < scu->scu_error_limit) ) {
	    if (CmdInterruptedFlag == TRUE) break;
	    bytes = MIN (total_bytes, verify_bytes);
#ifdef notdef
	    if (WatchProgress) {
		Printf ("Verifying %u byte%s...\n",
					bytes, (bytes == 1) ? "" : "s");
	    }
#endif
	    if ( (status = VerifySequential (scu, bytes)) == FAILURE) {
		break;
	    }
	    count = (bytes - scu->scu_xfer_resid);
	    total_bytes -= count;
	    if (status == WARNING) break;	/* File Mark or EOM. */
	    if ( scu->scu_record_limit &&
		 (scu->scu_record_count >= scu->scu_record_limit) ) {
		    break;
	    }
	  }
	  /*
	   * Status set to warning on file marks and end of media.
	   */
	  if (status == WARNING) {
	      if (sdp->eom) {
		  break;			/* End of media detected. */
	      } else {
	          status = SUCCESS;		/* Continue on file marks. */
	      }
	  }
	} while ( (status == SUCCESS) &&
		  (CmdInterruptedFlag == FALSE) &&
		  (++scu->scu_pass_count < scu->scu_pass_limit) &&
		  (scu->scu_error_count < scu->scu_error_limit) );

	if ( (CmdInterruptedFlag == TRUE) ||
	     (scu->scu_error_count >= scu->scu_error_limit) ) {
	    status = FAILURE;
	} else if ( (status != FAILURE) && total_bytes) {
	    Fprintf ("Warning, only verified %lu bytes of requested %lu bytes.",
				scu->scu_total_bytes, scu->scu_block_limit);
	    status = WARNING;
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifySequential() - Verify Records from Sequential Access Device.	*
 *									*
 * Inputs:	scu    = The device unit structure.			*
 *		length = The length of the transfer (bytes).		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
VerifySequential (scu, length)
struct scu_device *scu;
U32 length;
{
	int status;
	U32 bytes, count, xfer_resid;

	bytes = length;
	do {
	    count = DoVerifySequential (scu, bytes);
	    if (CmdInterruptedFlag == TRUE) return (FAILURE);
	    scu->scu_record_count++;
	    if (count == bytes) {	/* Full transfer completed. */
		scu->scu_total_bytes += count;
		return (SUCCESS);
	    }
	    count = (bytes - scu->scu_xfer_resid);
	    scu->scu_total_bytes += count;
	    /*
	     * Error occured or partial number of bytes written.
	     */
	    status = ReportSequentialError (scu, "Verify", &xfer_resid);
	    if (status == FAILURE) {
		return (status);	/* No sense or fatal error. */
	    }
	    /*
	     * Report partial records, unless file mark or eom detected.
	     */
	    if ( count && (bytes != count) ) {
		scu->scu_partial_records++;
		if (DebugFlag || DisplayVerbose) {
		    Fprintf(
	"Warning: Record #%lu, attempted to verify %u bytes, verified only %u bytes.",
				scu->scu_record_count, bytes, count);
		}
	    }
	    /*
	     * Recoverable error or partial transfer occured.
	     */
	    if ( (int)(bytes -= count) <= 0) {
		return (SUCCESS);
	    }
	    if ( scu->scu_record_limit &&
		 (scu->scu_record_count >= scu->scu_record_limit) ) {
		    break;
	    }
	    /*
	     * Status set to warning on file marks and end of media.
	     */
	    if (status == WARNING) return (status);
	    /*
	     * For variable length tapes, don't loop so the specified
	     * (full) block size gets used for the next request.
	     */
	    if (!scu->scu_device_size) break;
	} while (scu->scu_error_count < scu->scu_error_limit);

	return (status);
}

/************************************************************************
 *									*
 * DoVerifySequential() - Verify Sequential Access Device Blocks.	*
 *									*
 * Inputs:	scu    = The device unit structure.			*
 *		length = The number of bytes to verify.			*
 *									*
 * Return Value:							*
 *		Returns # of Bytes Written / -1 = SUCCESS / FAILURE.	*
 *									*
 ************************************************************************/
int
DoVerifySequential (scu, length)
struct scu_device *scu;
U32 length;
{
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;
	register struct data_transfer_params *dt;
	register struct all_inq_data *inquiry = scu->scu_inquiry;
	struct all_req_sns_data *sdp;
	U32 xfer_resid;
	int status;

	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;
	/*
	 * Setup the direct access device data transfer parameters.
	 */
	dt = (struct data_transfer_params *) scu->scu_iop_buffer;
	(void) bzero ((char *) dt, sizeof(*dt));
	dt->dt_inq_dtype = inquiry->dtype;
	dt->dt_inq_pqual = inquiry->pqual;
	dt->dt_inq_rdf = inquiry->rdf;
	dt->dt_block_size = scu->scu_device_size;

	/*
	 * Prepare Special I/O Command:
	 */
	(void) bzero ((char *) sp, sizeof(*sp));
	sp->sp_sub_command = SCMD_VERIFY_TAPE;
	sp->sp_iop_length = sizeof(*dt);
	sp->sp_iop_buffer = (caddr_t) dt;
	sp->sp_cmd_parameter = length;
	status = DoSpecial (sp, "verify tape");
	/*
	 * Since this command does not transfer any data, we must
	 * calculate the residual count from the sense information.
	 * The valid bit indicates the information bytes are valid.
	 */
	if (sdp->valid) {
	    xfer_resid = (U32)( ((U32)sdp->info_byte3 << 24) +
				((U32)sdp->info_byte2 << 16) +
				((U32)sdp->info_byte1 << 8) +
				 (sdp->info_byte0) );
	    /*
	     * For fixed blocks, residual is a block count.
	     */
	    if (scu->scu_device_size) {
		xfer_resid = (xfer_resid * scu->scu_device_size);
	    } else {
		/*
		 * For variable length tapes, a short read of a larger
		 * block returns a negative residual count.
		 */
		if ((I32)xfer_resid < 0) {
		    if (DebugFlag || DisplayVerbose) {
			Fprintf (
	"Warning: Record #%lu, only verified %u bytes of %u record bytes.",
	(scu->scu_record_count + 1), length, (length - (I32)xfer_resid));
		    }
		    xfer_resid = 0;
		}
	    }
	    /*
	     * Sanity check the residual count, since some tape drives
	     * do not report the correct information bytes (which ones?).
	     */
	    if (xfer_resid <= length) {
		scu->scu_xfer_resid = xfer_resid;
	    } else {
		scu->scu_xfer_resid = 0;
	    }
	}

	if (status != SUCCESS) {
	    return (status);
	} else {
	    return (length - scu->scu_xfer_resid);
	}
}
