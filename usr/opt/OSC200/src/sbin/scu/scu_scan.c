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
static char *rcsid = "@(#)$RCSfile: scu_scan.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/11/03 21:27:30 $";
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
 * File:	scu_scan.c
 * Author:	Robin T. Miller
 * Date:	February 6, 1992
 *
 * Description:
 *	This file contains routines for performing read/write testing.
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
extern int IS_Mounted (struct scu_device *scu);
extern int ReadDirect (struct scu_device *scu, caddr_t buffer,
					U32 lba, U32 blocks);
extern int VerifyBuffer (u_char *buffer, int count, U32 pattern);
extern int WriteDirect (struct scu_device *scu, caddr_t buffer,
					U32 lba, U32 blocks);
extern U32 data_patterns[];
extern int npatterns;

/*
 * Forward References:
 */
static int TestScanDirect (struct cmd_entry *ce, struct scu_device *scu);

/************************************************************************
 *									*
 * ScanMedia() - Scan (Read/Write) Device Media on Selected Device.	*
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
ScanMedia (ce, ke)
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
		status = TestScanDirect (ce, scu);
		break;

	    default:
		Fprintf ("Scan data not implemented for %s device type.",
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
 * TestScanDirect() - Test Scan Media for Direct Access Device Types.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
TestScanDirect (ce, scu)
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
	  if (DisplayVerbose) {
	    Printf ("Scanning %lu block%s on %s (%s) with pattern 0x%08x...\n",
			total_blocks, (total_blocks == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name, pattern);
	  }
	  while ( total_blocks &&
		  (scu->scu_error_count < scu->scu_error_limit) ) {
	    if (CmdInterruptedFlag == TRUE) break;
	    blocks = MIN (total_blocks, data_blocks);
	    bytes = (blocks * scu->scu_device_size);
	    if (WatchProgress) {
		if (blocks > 1) {
		    Printf ("Scanning blocks [ %u through %u ]...\n",
						lba, (lba + blocks - 1) );
		} else {
		    Printf ("Scanning block %u...\n", lba);
		}
	    }
	    FillBuffer ((u_char *) buffer, (int) bytes, pattern);
	    if ( (status = WriteDirect (scu, buffer, lba, blocks)) != SUCCESS) {
		break;
	    }
	    if (CmdInterruptedFlag == TRUE) break;
	    FillBuffer ((u_char *) buffer, bytes, ~pattern);
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

	return (status);
}
