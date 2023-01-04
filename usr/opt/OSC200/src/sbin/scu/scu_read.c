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
static char *rcsid = "@(#)$RCSfile: scu_read.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/11/03 21:27:24 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1992 by				*
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
 * File:	scu_read.c
 * Author:	Robin T. Miller
 * Date:	February 6, 1992
 *
 * Description:
 *	This file contains routines for performing read testing.
 */

#include <stdio.h>
#include <sys/errno.h>
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
extern void Fprintf(), Perror(), Printf();
extern void FillBuffer (u_char *buffer, int count, U32 pattern);
extern int ReportDirectError (struct scu_device *scu, char *op, U32 *lbn);
extern int ReportSequentialError (struct scu_device *scu, char *op, U32 *resid);
extern U32 data_patterns[];
extern int npatterns;

/*
 * Forward References:
 */
int DoReadDirect (caddr_t buffer, U32 length, U32 lba);
static int TestReadDirect (struct cmd_entry *ce, struct scu_device *scu);
int DoReadSequential (struct scu_device *scu, caddr_t buffer, U32 length);
static int TestReadSequential (struct cmd_entry *ce, struct scu_device *scu);

/*
 * Local Variables:
 */
static char *read_msg = "SCMD_READ";

/************************************************************************
 *									*
 * ReadMedia() - Read Data Blocks from Device Media.			*
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
ReadMedia (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;
	int saved_PerrorFlag, status;

	scu->scu_test_control = (TC_NO_DEFAULT);
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
		status = TestReadDirect (ce, scu);
		break;

	    case ALL_DTYPE_SEQUENTIAL:
		status = TestReadSequential (ce, scu);
		break;

	    default:
		Fprintf ("Read data not implemented for %s device type.",
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
 * TestReadDirect() - Test Read Data for Direct Access Device Types.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
TestReadDirect (ce, scu)
register struct cmd_entry *ce;
register struct scu_device *scu;
{
	caddr_t buffer;
	u_long total_blocks;
	U32 bytes, blocks, data_blocks, lba = 0, pattern;
	int status;

	buffer = scu->scu_aligned_bptr;

	/*
	 * Calculate the number of data blocks & bytes per I/O request.
	 */
	data_blocks = howmany (scu->scu_block_size, scu->scu_device_size);

	/*
	 * Read the disk blocks.
	 */
	do {
	  lba = scu->scu_starting_lba;
	  total_blocks = scu->scu_block_limit;
	  /*
	   * For reads, we don't know the last pattern actually written,
	   * so the user must specify or we'll presume the default pattern.
	   */
	  if ( ISSET_KOPT (ce, K_DATA_PATTERN) ) {
	    pattern = scu->scu_data_pattern;	/* Users' pattern. */
	  } else {
	    pattern = data_patterns[0];		/* Default pattern. */
	  }
	  if (DisplayVerbose) {
	    if (scu->scu_compare_flag == TRUE) {
	        Printf ("Reading %lu block%s on %s (%s) using pattern 0x%08x...\n",
			total_blocks, (total_blocks == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name, pattern);
	    } else {
	        Printf ("Reading %lu block%s on %s (%s)...\n",
			total_blocks, (total_blocks == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name);
	    }
	  }
	  while ( total_blocks &&
		  (scu->scu_error_count < scu->scu_error_limit) ) {
	    if (CmdInterruptedFlag == TRUE) break;
	    blocks = MIN (total_blocks, data_blocks);
	    bytes = (blocks * scu->scu_device_size);
	    FillBuffer ((u_char *) buffer, bytes, ~pattern);
	    if (WatchProgress) {
		if (blocks > 1) {
		    Printf ("Reading blocks [ %u through %u ]...\n",
						lba, (lba + blocks - 1) );
		} else {
		    Printf ("Reading block %u...\n", lba);
		}
	    }
	    if ( (status = ReadDirect (scu, buffer, lba, blocks)) != SUCCESS) {
		break;
	    }
	    if (CmdInterruptedFlag == TRUE) break;
	    if (scu->scu_compare_flag == TRUE) {
	        status = VerifyBuffer ((u_char *) buffer, (int) bytes, pattern);
		if (status != SUCCESS) {
		    scu->scu_error_count++;
		}
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
 * ReadDirect() - Read Range of Blocks for Direct Access Device.	*
 *									*
 * Inputs:	scu    = The device unit structure.			*
 *		buffer = The data buffer to write.			*
 *		lba    = The starting logical block number.		*
 *		blocks = The number of blocks to read.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ReadDirect (scu, buffer, lba, blocks)
struct scu_device *scu;
caddr_t buffer;
U32 lba;
U32 blocks;
{
	U32 lbn, status;
	U32 bytes, count;

	bytes = (blocks * scu->scu_device_size);
	do {
	    count = DoReadDirect (buffer, bytes, lba);
	    if (CmdInterruptedFlag == TRUE) return (FAILURE);
	    scu->scu_record_count++;
	    if (count == bytes) {	/* Full transfer completed. */
		return (SUCCESS);
	    }
	    /*
	     * Failure or partial transfer occurred.
	     */
	    status = ReportDirectError (scu, "Read", &lbn);
	    if ( (status == SUCCESS) || (status == FAILURE) ) {
		return (status);	/* No sense or recoverable. */
	    }
	    /*
	     * Calculate remaining blocks to process.
	     */
	    lbn++;			/* Point past erroring block. */
	    status = SUCCESS;		/* Don't abort reading... */
	    buffer = (caddr_t)
		    ( (u_long) buffer +
		      (u_long) ((lbn - lba) * scu->scu_device_size) );
	    if ( (int)(bytes -= ((lbn - lba) * scu->scu_device_size)) <= 0) {
		return (status);
	    } else {
		lba = lbn;		/* Set the starting lba. */
	    }
	} while (scu->scu_error_count < scu->scu_error_limit);

	return (status);
}

/************************************************************************
 *									*
 * DoReadDirect() - Setup & Do Read from a Direct Access Device.	*
 *									*
 * Inputs:	buffer = The buffer to read into.			*
 *		length = The length of the transfer (bytes).		*
 *		lba    = The starting logical block address.		*
 *									*
 * Return Value:							*
 *		Returns # of Bytes Read / -1 = SUCCESS / FAILURE.	*
 *									*
 ************************************************************************/
int
DoReadDirect (buffer, length, lba)
caddr_t buffer;
U32 length;
U32 lba;
{
	struct scu_device *scu = ScuDevice;
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;
	register struct data_transfer_params *dt;
	register struct all_inq_data *inquiry = scu->scu_inquiry;
	int status;

	/*
	 * Setup the direct access device data transfer parameters.
	 */
	dt = (struct data_transfer_params *) scu->scu_iop_buffer;
	(void) bzero ((char *) dt, sizeof(*dt));
	dt->dt_inq_dtype = inquiry->dtype;
	dt->dt_inq_pqual = inquiry->pqual;
	dt->dt_inq_rdf = inquiry->rdf;
	dt->dt_lba = lba;
	dt->dt_block_size = scu->scu_device_size;

	(void) bzero (buffer, length);

	/*
	 * Prepare Special I/O Command:
	 */
	(void) bzero ((char *) sp, sizeof(*sp));
	sp->sp_sub_command = SCMD_READ;
	sp->sp_iop_length = sizeof(*dt);
	sp->sp_iop_buffer = (caddr_t) dt;
	sp->sp_user_length = length;
	sp->sp_user_buffer = buffer;
	if ( (status = DoSpecial (sp, read_msg)) != SUCCESS) {
	    return (status);
	}
	return (length - sp->sp_xfer_resid);
}

/************************************************************************
 *									*
 * TestReadSequential() - Test Read Data for Sequential Device Types.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
TestReadSequential (ce, scu)
register struct cmd_entry *ce;
register struct scu_device *scu;
{
	caddr_t buffer;
	u_long total_bytes;
	U32 bytes, data_bytes;
	struct all_req_sns_data *sdp;
	U32 pattern;
	int count, status;

	buffer = scu->scu_aligned_bptr;
	data_bytes = MIN (MAX_SEQ_DATA_LENGTH, scu->scu_block_size);
	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;

	/*
	 * Read the tape bytes/records.
	 */
	do {
	  scu->scu_record_count = 0L;
	  total_bytes = scu->scu_block_limit;
	  if ( ISSET_KOPT (ce, K_DATA_PATTERN) ) {
	    pattern = scu->scu_data_pattern;	/* Users' pattern. */
	  } else {
	    pattern = data_patterns[scu->scu_pass_count % npatterns];
	  }
	  if (DisplayVerbose) {
	    if (scu->scu_compare_flag == TRUE) {
		Printf ("Reading %lu byte%s on %s (%s) using pattern 0x%08x...\n",
			total_bytes, (total_bytes == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name, pattern);
	    } else {
	        Printf ("Reading %lu byte%s on %s (%s)...\n",
			total_bytes, (total_bytes == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name);
	    }
	  }
	  while ( total_bytes &&
		  (scu->scu_error_count < scu->scu_error_limit) ) {
	    if (CmdInterruptedFlag == TRUE) break;
	    bytes = MIN (total_bytes, data_bytes);
	    FillBuffer ((u_char *) buffer, bytes, ~pattern);
#ifdef notdef
	    if (WatchProgress) {
		Printf ("Reading %lu byte%s...\n",
					bytes, (bytes == 1) ? "" : "s");
	    }
#endif notdef
	    if ( (status = ReadSequential (scu, buffer, bytes)) == FAILURE) {
		break;
	    }
	    if (CmdInterruptedFlag == TRUE) break;
	    count = ((int) bytes - scu->scu_xfer_resid);
	    if (scu->scu_compare_flag == TRUE) {
	        int vstatus = VerifyBuffer ((u_char *) buffer, count, pattern);
		if (vstatus != SUCCESS) {
		    scu->scu_error_count++;
		}
	    }
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
	    Fprintf ("Warning, only read %lu bytes of requested %lu bytes.",
				scu->scu_total_bytes, scu->scu_block_limit);
	    status = WARNING;
	}

	return (status);
}

/************************************************************************
 *									*
 * ReadSequential() - Read Records from Sequential Access Device.	*
 *									*
 * Inputs:	scu    = The device unit structure.			*
 *		buffer = The buffer to write from.			*
 *		length = The length of the transfer (bytes).		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ReadSequential (scu, buffer, length)
struct scu_device *scu;
caddr_t buffer;
U32 length;
{
	int status;
	U32 bytes, count, xfer_resid;

	bytes = length;
	do {
	    count = DoReadSequential (scu, buffer, bytes);
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
	    status = ReportSequentialError (scu, "Read", &xfer_resid);
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
	"Warning: Record #%lu, attempted to read %u bytes, read only %u bytes.",
					scu->scu_record_count, bytes, count);
		}
	    }
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
	"Warning: Record #%lu, only read %u bytes of %u record bytes.",
		scu->scu_record_count, length, (length - (I32)xfer_resid));
		    }
		    xfer_resid = 0;
		}
	    }
	    /*
	     * Sanity check the residual counts (CAM's vs. ours from sense).
	     ( FYI: Field scu_xfer_resid gets value from CCB's cam_resid).
	     */
	    if (scu->scu_xfer_resid != xfer_resid) {
		if (DebugFlag || DisplayVerbose) {
		    Fprintf (
	"Error: Residual count mismatch, CCB resid = %u, sense resid = %u",
					scu->scu_xfer_resid, xfer_resid);
		}
		scu->scu_error_count++;
	    }
	    /*
	     * Recoverable error or partial transfer occured.
	     */
	    buffer = (caddr_t) ( (u_long) buffer + count);
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
	    if (status == WARNING) break;
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
 * DoReadSequential() - Write to Sequential Access Device.		*
 *									*
 * Inputs:	scu    = The device unit structure.			*
 * 		buffer = The buffer to write from.			*
 *		length = The length of the transfer (bytes).		*
 *									*
 * Return Value:							*
 *		Returns # of Bytes Written / -1 = SUCCESS / FAILURE.	*
 *									*
 ************************************************************************/
int
DoReadSequential (scu, buffer, length)
struct scu_device *scu;
caddr_t buffer;
U32 length;
{
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;
	register struct data_transfer_params *dt;
	register struct all_inq_data *inquiry = scu->scu_inquiry;
	int status;

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
	sp->sp_sub_command = SCMD_READ;
	sp->sp_iop_length = sizeof(*dt);
	sp->sp_iop_buffer = (caddr_t) dt;
	sp->sp_user_length = length;
	sp->sp_user_buffer = buffer;
	if ( (status = DoSpecial (sp, read_msg)) != SUCCESS) {
	    return (status);
	}
	return (length - sp->sp_xfer_resid);
}
