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
static char *rcsid = "@(#)$RCSfile: scu_write.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/11/03 21:28:33 $";
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
 * File:	scu_write.c
 * Author:	Robin T. Miller
 * Date:	February 3, 1992
 *
 * Description:
 *	This file contains routines for performing write testing.
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
extern int IS_Mounted (struct scu_device *scu);
extern int ReportDirectError (struct scu_device *scu, char *op, U32 *lbn);
extern int ReportSequentialError (struct scu_device *scu, char *op, U32 *resid);
extern int VerifyBuffer (u_char *buffer, int count, U32 pattern);
extern void Fprintf(), Printf();

extern U32 data_patterns[];
extern int npatterns;

/*
 * Forward References:
 */
int DoWriteDirect (caddr_t buffer, U32 length, U32 lba);
static int TestWriteDirect (struct cmd_entry *ce, struct scu_device *scu);
int DoWriteSequential (struct scu_device *scu, caddr_t buffer, U32 length);

/*
 * Local Variables:
 */
static char *write_msg = "SCMD_WRITE";

/************************************************************************
 *									*
 * WriteMedia() - Write Data Blocks to Device Media.			*
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
WriteMedia (ce, ke)
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

	/*
	 * Do verify data for each device type.
	 */
	switch (scu->scu_device_type) {

	    case ALL_DTYPE_DIRECT:
	    case ALL_DTYPE_OPTICAL:
		/*
		 * Don't permit writing if a file system is mounted.
		 */
		if ((status = IS_Mounted (scu)) != FALSE) {
		    return (FAILURE);
		}
		PerrorFlag = FALSE;
		status = TestWriteDirect (ce, scu);
		break;

	    case ALL_DTYPE_SEQUENTIAL:
		PerrorFlag = FALSE;
		status = TestWriteSequential (ce, scu);
		break;

	    default:
		Fprintf ("Write data not implemented for %s device type.",
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
 * TestWriteDirect() - Test Write Data for Direct Access Device Types.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
TestWriteDirect (ce, scu)
register struct cmd_entry *ce;
register struct scu_device *scu;
{
	register caddr_t buffer;
	u_long total_blocks;
	U32 bytes, blocks, data_blocks, lba = 0, pattern;
	int status;

	buffer = scu->scu_aligned_bptr;

	/*
	 * Calculate the number of data blocks & bytes per I/O request.
	 */
	data_blocks = howmany (scu->scu_block_size, scu->scu_device_size);
	bytes = (data_blocks * scu->scu_device_size);

	/*
	 * Write the disk blocks.
	 */
	do {
	  lba = scu->scu_starting_lba;
	  total_blocks = scu->scu_block_limit;
	  if ( ISSET_KOPT (ce, K_DATA_PATTERN) ) {
	    pattern = scu->scu_data_pattern;	/* Users' pattern. */
	  } else {
	    pattern = data_patterns[scu->scu_pass_count % npatterns];
	  }
	  FillBuffer ((u_char *) buffer, (int) bytes, pattern);
	  if (DisplayVerbose) {
	    Printf ("Writing %lu block%s on %s (%s) with pattern 0x%08x...\n",
			total_blocks, (total_blocks == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name, pattern);
	  }
	  while ( total_blocks &&
		  (scu->scu_error_count < scu->scu_error_limit) ) {
	    if (CmdInterruptedFlag == TRUE) break;
	    blocks = MIN (total_blocks, data_blocks);
	    if (WatchProgress) {
		if (blocks > 1) {
		    Printf ("Writing blocks [ %u through %u ]...\n",
						lba, (lba + blocks - 1) );
		} else {
		    Printf ("Writing block %u...\n", lba);
		}
	    }
	    if ( (status = WriteDirect (scu, buffer, lba, blocks)) != SUCCESS) {
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
 * WriteDirect() - Write Range of Blocks for Direct Access Device.	*
 *									*
 * Inputs:	scu    = The device unit structure.			*
 *		buffer = The data buffer to write.			*
 *		lba    = The starting logical block number.		*
 *		blocks = The number of blocks to write.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
WriteDirect (scu, buffer, lba, blocks)
struct scu_device *scu;
caddr_t buffer;
U32 lba;
U32 blocks;
{
	U32 lbn;
	int status;
	U32 bytes, count;

	bytes = (blocks * scu->scu_device_size);
	do {
	    count = DoWriteDirect (buffer, bytes, lba);
	    if (CmdInterruptedFlag == TRUE) return (FAILURE);
	    if (count == bytes) {	/* Full transfer completed. */
		return (SUCCESS);
	    }
	    /*
	     * Failure or partial transfer occurred.
	     */
	    status = ReportDirectError (scu, "Write", &lbn);
	    if ( (status == SUCCESS) || (status == FAILURE) ) {
		return (status);	/* No sense or recoverable. */
	    }
	    /*
	     * Calculate remaining blocks to process.
	     */
	    lbn++;			/* Point past erroring block. */
	    status = SUCCESS;		/* Don't abort writing... */
	    buffer = (caddr_t)
		    ( (u_long) buffer +
		      (u_long) ((lbn - lba) * scu->scu_device_size) );
	    if ( (long)(bytes -= ((lbn - lba) * scu->scu_device_size)) <= 0L) {
		return (status);
	    } else {
		lba = lbn;		/* Set the starting lba. */
	    }
	} while (scu->scu_error_count < scu->scu_error_limit);

	return (status);
}

/************************************************************************
 *									*
 * DoWriteDirect() - Setup & Do Write to a Direct Access Device.	*
 *									*
 * Inputs:	buffer = The buffer to write from.			*
 *		length = The length of the transfer (bytes).		*
 *		lba    = The starting logical block address.		*
 *									*
 * Return Value:							*
 *		Returns # of Bytes Written / -1 = SUCCESS / FAILURE.	*
 *									*
 ************************************************************************/
int
DoWriteDirect (buffer, length, lba)
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

	/*
	 * Prepare the Special I/O Command.
	 */
	(void) bzero ((char *) sp, sizeof(*sp));
	sp->sp_sub_command = SCMD_WRITE;
	sp->sp_iop_length = sizeof(*dt);
	sp->sp_iop_buffer = (caddr_t) dt;
	sp->sp_user_length = length;
	sp->sp_user_buffer = buffer;
	if ( (status = DoSpecial (sp, write_msg)) != SUCCESS) {
	    return (status);
	}
	return (length - sp->sp_xfer_resid);
}

/************************************************************************
 *									*
 * TestWriteSequential() - Test Write Data for Sequential Device Types.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
TestWriteSequential (ce, scu)
register struct cmd_entry *ce;
register struct scu_device *scu;
{
	caddr_t buffer;
	U32 bytes, data_bytes;
	u_long total_bytes;
	struct all_req_sns_data *sdp;
	U32 pattern;
	int status;

	buffer = scu->scu_aligned_bptr;
	data_bytes = MIN (MAX_SEQ_DATA_LENGTH, scu->scu_block_size);
	sdp = (struct all_req_sns_data *) scu->scu_sense_buffer;

	/*
	 * Write the tape bytes/records.
	 */
	do {
	  total_bytes = scu->scu_block_limit;
	  if ( ISSET_KOPT (ce, K_DATA_PATTERN) ) {
	    pattern = scu->scu_data_pattern;	/* Users' pattern. */
	  } else {
	    pattern = data_patterns[scu->scu_pass_count % npatterns];
	  }
	  FillBuffer ((u_char *) buffer, (int) data_bytes, pattern);
	  if (DisplayVerbose) {
	    Printf ("Writing %lu byte%s on %s (%s) with pattern 0x%08x...\n",
			total_bytes, (total_bytes == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name, pattern);
	  }
	  while ( total_bytes &&
		  (scu->scu_error_count < scu->scu_error_limit) ) {
	    if (CmdInterruptedFlag == TRUE) break;
	    bytes = MIN (total_bytes, data_bytes);
#ifdef notdef
	    if (WatchProgress) {
		Printf ("Writing %lu byte%s...\n",
					bytes, (bytes == 1) ? "" : "s");
	    }
#endif notdef
	    if ( (status = WriteSequential (scu, buffer, bytes)) != SUCCESS) {
		break;
	    }
	    total_bytes -= bytes;
	  }
	  /*
	   * Status set to warning on file marks and end of media.
	   */
	  if (status == WARNING) {
	      if (sdp->eom) {			/* End of media detected. */
		break;
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
	    Fprintf ("Warning, only wrote %lu bytes of requested %lu bytes.",
				scu->scu_total_bytes, scu->scu_block_limit);
	    status = WARNING;
	}
	return (status);
}

/************************************************************************
 *									*
 * WriteSequential() - Write Records to Sequential Access Device.	*
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
WriteSequential (scu, buffer, length)
struct scu_device *scu;
caddr_t buffer;
U32 length;
{
	int status;
	U32 count, xfer_resid;

	count = DoWriteSequential (scu, buffer, length);
	if (CmdInterruptedFlag == TRUE) return (FAILURE);
	if (count == length) {		/* Full transfer completed. */
	    scu->scu_total_bytes += count;
	    return (SUCCESS);
	}
	count = (length - scu->scu_xfer_resid);
	scu->scu_total_bytes += count;
	if (length - count) scu->scu_partial_records++;

	/*
	 * Error occured or partial number of bytes written.
	 */
	status = ReportSequentialError (scu, "Write", &xfer_resid);
	return (status);
}

/************************************************************************
 *									*
 * DoWriteSequential() - Write to Sequential Access Device.		*
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
DoWriteSequential (scu, buffer, length)
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
	sp->sp_sub_command = SCMD_WRITE;
	sp->sp_iop_length = sizeof(*dt);
	sp->sp_iop_buffer = (caddr_t) dt;
	sp->sp_user_length = length;
	sp->sp_user_buffer = buffer;
	if ( (status = DoSpecial (sp, write_msg)) != SUCCESS) {
	    return (status);
	}
	return (length - sp->sp_xfer_resid);
}
