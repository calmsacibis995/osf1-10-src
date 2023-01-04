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
static char *rcsid = "@(#)$RCSfile: scu_tests.c,v $ $Revision: 1.1.10.4 $ (DEC) $Date: 1993/11/23 23:11:25 $";
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
 * File:	scu_tests.c
 * Author:	Robin T. Miller
 * Date:	December 26, 1990
 *
 * Description:
 *	This file contains functions for performing tests.
 */

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <io/common/iotypes.h>
#include <io/cam/cdrom.h>
#include <io/cam/rzdisk.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#include <io/cam/scsi_special.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"
#include "scsipages.h"

/*
 * Data transfers lengths MUST be restricted to the maximum 32-bit
 * positive value, since lengths are defined as int's through much
 * of the code.  A negative value sign extends on Alpha to 64 bits,
 * which causes all kinds of problems.
 */
#define MAX_DATA_LENGTH		0x7fffffff

/*
 * External References:
 */
extern char *malloc_palign(int size);
extern void free_palign(char *pa_addr);
extern int OpenInputtFile (FILE **fp, char *file);
extern int EnableRecovery (struct scu_device *scu);
extern int DisableRecovery (struct scu_device *scu);
extern void Fprintf(), Perror(), Printf();

/*
 * Forward References:
 */
U32 GetMemorySize (int offset, u_char mode, u_char buf_id);
static int DoDirectSetup (struct cmd_entry *ce, struct scu_device *scu);
void DumpBuffer (u_char *buffer, int size);
void FillBuffer (u_char *buffer, int count, U32 pattern);
int VerifyBuffer (u_char *buffer, int count, U32 pattern);

/*
 * Local Variables:
 */
static int send_cmd = SCSI_SEND_DIAGNOSTIC;
static char *send_msg = "SCSI_SEND_DIAGNOSTIC";

static int receive_cmd = SCSI_RECEIVE_DIAGNOSTIC;
static char *receive_msg = "SCSI_RECEIVE_DIAGNOSTIC";

static char *read_buffer_msg = "SCMD_READ_BUFFER";
static char *write_buffer_msg = "SCMD_WRITE_BUFFER";

static char *self_test_str =			"Self-Test";
static char *controller_str =			"Controller";
static char *drive_control_str =		"Drive Control";
static char *rom_str =				"ROM";
static char *ram_str =				"RAM";
static char *data_buffer_str =			"Data Buffer";
static char *interface_str =			"Interface";
static char *diag_str =				"Diagnostic";
static char *failed_str =			"FAILED!!!";
static char *success_str =			"Completed SUCCESSFULLY";
static char *unknown_str =			"Returned Unknown Value";
static char *memory_str =			"Memory";

#if defined(CAMDEBUG)

extern void cdbg_DumpBuffer (char *buffer, int size);
extern u_long cdbg_DumpLimit;

#else /* !defined(CAM_DEBUG) */

u_long cdbg_DumpLimit = 512;		/* Patchable dump buffer limit.	*/

#endif /* defined(CAMDEBUG) */

/*
 * Data patterns used for testing controller memory.
 */
U32 data_patterns[] = {
	DEFAULT_PATTERN,
	0x00ff00ff,
	0x0f0f0f0f,
	0xc6dec6de,
	0x6db6db6d,
	0x00000000,
	0xffffffff,
	0xaaaaaaaa,
	0x33333333,	/* Continuous worst case pattern (media defects) */
	0x26673333,	/* Frequency burst worst case pattern #1.	 */
	0x66673326,	/* Frequency burst worst case pattern #2.	 */
	0x71c7c71c	/* Dibit worst case data pattern.		 */
};
int npatterns = sizeof(data_patterns) / sizeof(U32);

/************************************************************************
 *									*
 * DoAllTests() - Do All The CD-ROM Diagnostics.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
DoAllTests (ce)
struct cmd_entry *ce;
{
	int status;

	/*
	 * Don't do the self-test by default, since it doesn't release
	 * the bus during these tests.  Takes approximatly 5 seconds.
	 */
	if (DebugFlag) {
	    if ((status = DoSelfTests()) != SUCCESS) {
		return (status);
	    }
	}
	if ((status = ControllerTests()) != SUCCESS) {
		return (status);
	}
	if ((status = DriveTests()) != SUCCESS) {
		return (status);
	}
	return (status);
}

/************************************************************************
 *									*
 * DoSelfTests() - Do the CD-ROM Default Self-test Diagnostics.		*
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
DoSelfTests (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct diagnostic_params diag_params;
	register struct diagnostic_params *dp = &diag_params;
	int status;

	if (DisplayVerbose) {
	    Printf ("Performing %s %ss...\n", self_test_str, diag_str);
	}
	(void) bzero ((char *)dp, sizeof(*dp));
	dp->dp_control = DP_SELF_TEST;
	if ( (status = DoIoctl (send_cmd, (char *)dp, send_msg)) != 0) {
	    Printf ("The %s %s %s\n", self_test_str, diag_str, failed_str);
	} else if (DebugFlag) {
	    Printf ("The %s %s %s\n", self_test_str, diag_str, success_str);
	}
	return (status);
}

/************************************************************************
 *									*
 * ControllerTests() - Do the CD-ROM Controller Diagnostics.		*
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
ControllerTests (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct diagnostic_params diag_params;
	register struct diagnostic_params *dp = &diag_params;
	register struct cd_diagnostic_data *dd;
	struct scu_device *scu = ScuDevice;
	int status;

	if (DisplayVerbose) {
	    Printf ("Performing %s %ss...\n", controller_str, diag_str);
	}
	(void) bzero ((char *)dp, sizeof(*dp));
	dd = (struct cd_diagnostic_data *) scu->scu_data_buffer;
	(void) bzero ((char *)dd, sizeof(*dd));
	dp->dp_length = sizeof(*dd);
	dp->dp_buffer = (caddr_t) dd;
	dd->dd_rom_diag = CDROM_CONTROLLER;
	dd->dd_ram_diag = CDROM_CONTROLLER;
	dd->dd_data_buffer_diag = CDROM_CONTROLLER;
	dd->dd_interface_diag = CDROM_CONTROLLER;
	if ( (status = DoIoctl (send_cmd, (char *)dp, send_msg)) != 0) {
		return (status);
	}
	(void) bzero ((char *)dd, sizeof(*dd));
	if ( (status = DoIoctl (receive_cmd, (char *)dp, receive_msg)) != 0) {
		return (status);
	}
	return (CheckDiagResults (dd));
}

/************************************************************************
 *									*
 * DriveTests() - Do the CD-ROM Drive Control Diagnostics.		*
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
DriveTests (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct diagnostic_params diag_params;
	register struct diagnostic_params *dp = &diag_params;
	register struct cd_diagnostic_data *dd;
	struct scu_device *scu = ScuDevice;
	int status;

	if (DisplayVerbose) {
	    Printf ("Performing %s %ss...\n", drive_control_str, diag_str);
	}
	(void) bzero ((char *)dp, sizeof(*dp));
	dd = (struct cd_diagnostic_data *) scu->scu_data_buffer;
	(void) bzero ((char *)dd, sizeof(*dd));
	dp->dp_length = sizeof(*dd);
	dp->dp_buffer = (caddr_t) dd;
	dd->dd_rom_diag = CDROM_DRIVE_CONTROL;
	dd->dd_ram_diag = CDROM_DRIVE_CONTROL;
	dd->dd_data_buffer_diag = CDROM_DRIVE_CONTROL;
	dd->dd_interface_diag = CDROM_DRIVE_CONTROL;
	if ( (status = DoIoctl (send_cmd, (char *)dp, send_msg)) != 0) {
		return (status);
	}
	(void) bzero ((char *)dd, sizeof(*dd));
	if ( (status = DoIoctl (receive_cmd, (char *)dp, receive_msg)) != 0) {
		return (status);
	}
	return (CheckDiagResults (dd));
}

/************************************************************************
 *									*
 * CheckDiagResults() - Check The CD-ROM Diagnostic Results.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
CheckDiagResults (dd)
register struct cd_diagnostic_data *dd;
{
	int status = SUCCESS;

	/*
	 * Check ROM Diagnostic Test Results.
	 */
	if (dd->dd_rom_diag != CDROM_DIAG_SUCCESS) {
	    if (dd->dd_rom_diag == CDROM_CONTROLLER) {
		Printf ("The %s %s %s %s\n",
			controller_str, rom_str, diag_str, failed_str);
	    } else if (dd->dd_rom_diag == CDROM_DRIVE_CONTROL) {
		Printf ("The %s %s %s %s\n",
			drive_control_str, rom_str, diag_str, failed_str);
	    } else {
		Printf ("The %s %s %s %#x\n",
			rom_str, diag_str, unknown_str, dd->dd_rom_diag);
	    }
	    status = FAILURE;
	} else if (DebugFlag) {
	    Printf ("The %s %s %s\n", rom_str, diag_str, success_str);
	}

	/*
	 * Check RAM Diagnostic Test Results.
	 */
	if (dd->dd_ram_diag != CDROM_DIAG_SUCCESS) {
	    if (dd->dd_ram_diag == CDROM_CONTROLLER) {
		Printf ("The %s %s %s %s\n",
			controller_str, ram_str, diag_str, failed_str);
	    } else if (dd->dd_ram_diag == CDROM_DRIVE_CONTROL) {
		Printf ("The %s %s %s %s\n",
			drive_control_str, ram_str, diag_str, failed_str);
	    } else {
		Printf ("The %s %s %s %#x\n",
			ram_str, diag_str, unknown_str, dd->dd_ram_diag);
	    }
	    status = FAILURE;
	} else if (DebugFlag) {
	    Printf ("The %s %s %s\n", ram_str, diag_str, success_str);
	}

	/*
	 * Check Data Buffer Diagnostic Test Results.
	 */
	if (dd->dd_data_buffer_diag != CDROM_DIAG_SUCCESS) {
	    if (dd->dd_data_buffer_diag == CDROM_CONTROLLER) {
		Printf ("The %s %s %s %s\n",
			controller_str, data_buffer_str, diag_str, failed_str);
	    } else if (dd->dd_data_buffer_diag == CDROM_DRIVE_CONTROL) {
		Printf ("The %s %s %s %s\n",
		drive_control_str, data_buffer_str, diag_str, failed_str);
	    } else {
		Printf ("The %s %s %s %#x\n",
		data_buffer_str, diag_str, unknown_str, dd->dd_data_buffer_diag);
	    }
	    status = FAILURE;
	} else if (DebugFlag) {
	    Printf ("The %s %s %s\n", data_buffer_str, diag_str, success_str);
	}

	/*
	 * Check Interface Diagnostic Test Results.
	 */
	if (dd->dd_interface_diag != CDROM_DIAG_SUCCESS) {
	    if (dd->dd_interface_diag == CDROM_CONTROLLER) {
		Printf ("The %s %s %s %s\n",
			controller_str, interface_str, diag_str, failed_str);
	    } else if (dd->dd_interface_diag == CDROM_DRIVE_CONTROL) {
		Printf ("The %s %s %s %s\n",
			drive_control_str, interface_str, diag_str, failed_str);
	    } else {
		Printf ("The %s %s %s %#x\n",
			interface_str, diag_str, unknown_str, dd->dd_interface_diag);
	    }
	    status = FAILURE;
	} else if (DebugFlag) {
	    Printf ("The %s %s %s\n", interface_str, diag_str, success_str);
	}

	return (status);
}

/************************************************************************
 *									*
 * DoMemoryTests() - Test the Controller Memory.			*
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
DoMemoryTests (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct scu_device *scu = ScuDevice;
	U32 bytes, data_bytes, total_bytes;
	struct all_buffer_header *bh;
	caddr_t bp;
	U32 bufsize, count, memory_size;
	int offset = 0, status;
	u_char buf_id, mode;
	U32 pattern;

	/*
	 * Check for mounted file systems on disks.
	 */
	if ( (scu->scu_device_type == ALL_DTYPE_DIRECT) ||
	     (scu->scu_device_type == ALL_DTYPE_RODIRECT) ||
	     (scu->scu_device_type == ALL_DTYPE_OPTICAL) ) {
	    /*
	     * Don't permit memory testing if a file system is mounted.
	     */
	    if ((status = IS_Mounted (scu)) != FALSE ) {
	        return (FAILURE);
	    }
	}

	scu->scu_test_control = (TC_NO_BUFFER |
				 TC_NO_SETUP |
				 TC_NO_STATISTICS);
	if ( (status = DoTestSetup (ce, scu)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Performing %s %s %ss...\n",
				controller_str, memory_str, diag_str);
	}

	/*
	 * Setup default or user specified parameters.
	 */
	if ( ISSET_KOPT (ce, K_BUFFER_ID) ) {
	    buf_id = BufferId;
	} else {
	    buf_id = DEFAULT_BUFFER_ID;
	}
	mode = scu->scu_buffer_mode;

	/*
	 * Don't allow testing anything except buffer ID 0, which is
	 * normally the data buffer, so we don't destroy anything else.
	 */
	if (BufferId != 0) {
	    Fprintf ("Testing is restricted to a Buffer ID of 0.");
	    return (FAILURE);
	}

	if ( ISSET_KOPT (ce, K_BUFFER_MODE) ) {
	    mode = BufferMode;
	}
	if ( (memory_size = GetMemorySize(offset, mode, buf_id)) == FAILURE) {
	    return (FAILURE);
	}

	if ( ISSET_KOPT (ce, K_BUFFER_OFFSET) ) {
	    offset = BufferOffset;
	} else {
	    offset = DEFAULT_BUFFER_OFFSET;
	}

	bufsize = memory_size + sizeof(*bh);	/* Must R/W buffer hdr.	*/
	if ( (bh = (struct all_buffer_header *) malloc_palign (bufsize)) == NULL) {
	    Fprintf ("Unable to allocate buffer of %u bytes.", bufsize);
	    return (FAILURE);
	}
	if (mode == ALL_RDBUF_COMBINED_HEADER) {
	    bp = (caddr_t) bh + sizeof(*bh);
	} else {
	    bp = (caddr_t) bh;			/* Don't need a header. */
	}
	data_bytes = memory_size;		/* Pure data byte size. */

	/*
	 * Test the controller memory.
	 */
	if (DisplayVerbose) {
	    Printf ("Testing %s %s of %u bytes (Mode %d, Offset %u)\n",
		    controller_str, memory_str, memory_size, mode, offset);
	}
	do {
	  if ( ISSET_KOPT (ce, K_DATA_LIMIT) ) {
	    total_bytes = MIN (memory_size, scu->scu_data_limit);
	  } else if ( ISSET_KOPT (ce, K_BLOCK_SIZE) ) {
	    total_bytes = MIN (memory_size, scu->scu_block_size);
	  } else {
	    total_bytes = memory_size;
	  }
	  if ( ((total_bytes + offset) > memory_size) &&
		(offset < total_bytes) ) {
	    total_bytes -= offset;		/* Adjust accordingly. */
	  }
	  if ( (offset > memory_size) ||
	       ((total_bytes + offset) > memory_size) ) {
		Fprintf (
		"Offset of %u causes memory size of %u to be exceeded.",
						offset, memory_size);
		status = FAILURE;
		break;
	  }
	  if ( ISSET_KOPT (ce, K_DATA_PATTERN) ) {
	    pattern = scu->scu_data_pattern;	/* Users' data pattern. */
	  } else {
	    pattern = data_patterns[scu->scu_pass_count % npatterns];
	  }
	  if (DisplayVerbose) {
	    if (scu->scu_compare_flag == TRUE) {
		Printf ("Testing %u byte%s on %s (%s) using pattern 0x%08x...\n",
			total_bytes, (total_bytes == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name, pattern);
	    } else {
	        Printf ("Testing %u byte%s on %s (%s)...\n",
			total_bytes, (total_bytes == 1) ? "" : "s",
			scu->scu_device_entry, scu->scu_device_name);
	    }
	  }
	  while ( total_bytes &&
		  (scu->scu_error_count < scu->scu_error_limit) ) {
	    if (CmdInterruptedFlag == TRUE) break;
	    bytes = MIN (total_bytes, data_bytes);
	    if (mode == ALL_RDBUF_COMBINED_HEADER) {
		bufsize = bytes + sizeof(*bh);	/* Must R/W buffer hdr.	*/
		(void) bzero ((caddr_t) bh, sizeof(*bh)); /* Zero hdr.	*/
	    } else {
		bufsize = bytes;		/* Send just pure data. */
	    }
	    FillBuffer ((u_char *) bp, bytes, pattern);
	    count = DoWriteBuffer ((caddr_t)bh, bufsize, offset, mode, buf_id);
	    if (count == FAILURE) {
		status = FAILURE;
		break;
	    }
	    if (CmdInterruptedFlag == TRUE) break;
	    /*
	     * Fill the buffer with the inverted pattern to ensure the
	     * data read actually overwrites the entire data buffer.
	     */
	    FillBuffer ((u_char *) bp, bytes, ~pattern);
	    count = DoReadBuffer ((caddr_t)bh, bufsize, offset, mode, buf_id);
	    if (count == FAILURE) {
		status = FAILURE;
		break;
	    }
	    if (CmdInterruptedFlag == TRUE) break;
	    if (scu->scu_compare_flag == TRUE) {
	        int vstatus = VerifyBuffer ((u_char *) bp, bytes, pattern);
		if (vstatus != SUCCESS) {
		    scu->scu_error_count++;
		}
	    }
	    total_bytes -= bytes;
	  }
	} while ( (status == SUCCESS) &&
		  (CmdInterruptedFlag == FALSE) &&
		  (++scu->scu_pass_count < scu->scu_pass_limit) &&
		  (scu->scu_error_count < scu->scu_error_limit) );

	if ( (CmdInterruptedFlag == TRUE) ||
	     (scu->scu_error_count >= scu->scu_error_limit) ) {
	    status = FAILURE;
	}
	free_palign ((char *) bh);
	(void) DoTestCleanup (ce, scu);
	return (status);
}

/************************************************************************
 *									*
 * DownloadMicrocode() - Download & Optionally Save Microcode.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DownloadMicrocode (ce)
struct cmd_entry *ce;
{
	register struct scu_device *scu = ScuDevice;
	char *fnp = (char *) *(long *) ce->c_argp;
	struct stat sb;
	FILE *fp = (FILE *) 0;
	caddr_t bp;
	int fd, size;
	off_t file_size;
	ssize_t count;
	int offset, status;
	u_char buf_id, mode;

	/*
	 * If segmented download requested, use alternate function.
	 */
	if ( ISSET_KOPT (ce, K_SEGMENT_SIZE) ) {
	    return ( SegmentDownloadMicrocode (ce) );
	}

	/*
	 * Check for mounted file systems on disks.
	 */
	if ( (scu->scu_device_type == ALL_DTYPE_DIRECT) ||
	     (scu->scu_device_type == ALL_DTYPE_RODIRECT) ||
	     (scu->scu_device_type == ALL_DTYPE_OPTICAL) ) {
	    /*
	     * Don't permit downloading if a file system is mounted.
	     */
	    if ((status = IS_Mounted (scu)) != FALSE ) {
	        return (FAILURE);
	    }
	}
	
	/*
	 * Setup default or user specified parameters.
	 */
	if ( ISSET_KOPT (ce, K_BUFFER_OFFSET) ) {
	    offset = BufferOffset;
	} else {
	    offset = DEFAULT_BUFFER_OFFSET;
	}
	if ( ISSET_KOPT (ce, K_BUFFER_ID) ) {
	    buf_id = BufferId;
	} else {
	    buf_id = DEFAULT_BUFFER_ID;
	}

	if ( (status = OpenInputFile (&fp, fnp)) != SUCCESS) {
	    return (status);
	}
	fd = fileno (fp);
	if ( (status = fstat (fd, &sb)) < 0) {
	    Perror ("fstat");
	    (void) fclose (fp);
	    return (status);
	}

	/*
	 * Only expecting regular files at this time.
	 */
	if ((sb.st_mode & S_IFMT) != S_IFREG) {
	    Fprintf ("File '%s' is NOT a regular file.", fnp);
	    (void) fclose (fp);
	    return (FAILURE);
	}

	if ( (file_size = sb.st_size) == (off_t) 0) {
	    Fprintf ("File '%s' is zero length, nothing to download.", fnp);
	    (void) fclose (fp);
	    return (FAILURE);
	}

	/*
	 * To simplify the download operation, we will allocate and read
	 * in the entire image to be downloaded (as opposed to segments).
	 * [ NOTE:  All devices appear to support full download cmds. ]
	 */
	if ( (bp = malloc_palign (file_size)) == NULL) {
	    Fprintf ("Failed to allocate file buffer of %lu bytes.", file_size);
	    (void) fclose (fp);
	    return (FAILURE);
	}

	if ( (count = read (fd, bp, file_size)) != file_size) {
	    Fprintf ("Error occured reading download file.");
	    if (count == FAILURE) {
		Perror ("read");
	    } else {
		Fprintf ("Attempted to read %lu bytes, read only %lu bytes.",
							file_size, count);
	    }
	    (void) fclose (fp);
	    free_palign (bp);
	    return (FAILURE);
	}

	if (DisplayVerbose) {
	    Printf ("Downloading%s Firmware File '%s' of %lu bytes...\n",
		      ( ISSET_KOPT (ce, K_SAVE_DATA) ) ? " & Saving" : "",
							fnp, file_size);
	}
	if ( ISSET_KOPT (ce, K_SAVE_DATA) ) {
	    mode = ALL_WTBUF_DOWNLOAD_SAVE;
	} else {
	    mode = ALL_WTBUF_DOWNLOAD_MICROCODE;
	}
	size = DoWriteBuffer (bp, (int)file_size, offset, mode, buf_id);
	if (size == file_size) {
	    status = SUCCESS;			/* Map to success. */
	} else {
	    status = FAILURE;			/* Set failure status. */
	}
	(void) fclose (fp);
	free_palign (bp);
	return (status);
}

/************************************************************************
 *									*
 * SegmentDownloadMicrocode() - Download & Optionally Save Microcode.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SegmentDownloadMicrocode (ce)
struct cmd_entry *ce;
{
	register struct scu_device *scu = ScuDevice;
	char *fnp = (char *) *(long *) ce->c_argp;
	struct stat sb;
	FILE *fp = (FILE *) 0;
	caddr_t bp;
	int fd, size;
	off_t file_size;
	ssize_t count;
	int offset, status;
	u_char buf_id, mode;
	int segment_size;

	/*
	 * Check for mounted file systems on disks.
	 */
	if ( (scu->scu_device_type == ALL_DTYPE_DIRECT) ||
	     (scu->scu_device_type == ALL_DTYPE_RODIRECT) ||
	     (scu->scu_device_type == ALL_DTYPE_OPTICAL) ) {
	    /*
	     * Don't permit downloading if a file system is mounted.
	     */
	    if ((status = IS_Mounted (scu)) != FALSE ) {
	        return (FAILURE);
	    }
	}
	
	/*
	 * Setup default or user specified parameters.
	 */
	if ( ISSET_KOPT (ce, K_BUFFER_OFFSET) ) {
	    offset = BufferOffset;
	} else {
	    offset = DEFAULT_BUFFER_OFFSET;
	}
	if ( ISSET_KOPT (ce, K_BUFFER_ID) ) {
	    buf_id = BufferId;
	} else {
	    buf_id = DEFAULT_BUFFER_ID;
	}
	if ( ISSET_KOPT (ce, K_SEGMENT_SIZE) ) {
	    if (SegmentSize) {
		segment_size = SegmentSize;
		SegmentSize = 0;
	    } else {
		segment_size = DefaultSegmentSize;
	    }
	} else {
	    segment_size = DefaultSegmentSize;
	}

	if ( (status = OpenInputFile (&fp, fnp)) != SUCCESS) {
	    return (status);
	}
	fd = fileno (fp);
	if ( (status = fstat (fd, &sb)) < 0) {
	    Perror ("fstat");
	    (void) fclose (fp);
	    return (status);
	}

	/*
	 * Only expecting regular files at this time.
	 */
	if ((sb.st_mode & S_IFMT) != S_IFREG) {
	    Fprintf ("File '%s' is NOT a regular file.", fnp);
	    (void) fclose (fp);
	    return (FAILURE);
	}

	if ( (file_size = sb.st_size) == (off_t) 0) {
	    Fprintf ("File '%s' is zero length, nothing to download.", fnp);
	    (void) fclose (fp);
	    return (FAILURE);
	}

	/*
	 * The download operation is performed segments, since large images
	 * can exceed the maximum parameter list length support by the device.
	 */
	if ( (bp = malloc_palign (segment_size)) == NULL) {
	    Fprintf ("Failed to allocate file buffer of %d bytes.", segment_size);
	    (void) fclose (fp);
	    return (FAILURE);
	}

	if (DisplayVerbose) {
	    Printf ("Downloading File '%s' of %ld bytes in %d byte segments...\n",
						fnp, file_size, segment_size);
	}

	/*
	 * The method used to download the firmware is as follows:
	 *    o  download image in segments w/Download Microcode (mode 4).
	 *    o  after image is downloaded, issue Save Microcode (mode 5).
	 */
	if (scu->scu_device_type == ALL_DTYPE_DIRECT) {
	    mode = ALL_WTBUF_DOWNLOAD_SAVE;
	} else {
	    mode = ALL_WTBUF_DOWNLOAD_MICROCODE;
	}
	/*
	 * The default mode to use is *very* vendor specific.  The TZK11 will
	 * only accept segmented downloads using mode 4, while DEC disks will
	 * only accept segmented downloads using mode 5.  In any case, we'll
	 * allow the user to specify any mode (even vendor specific modes).
	 */
	if ( ISSET_KOPT (ce, K_BUFFER_MODE) ) {
	    mode = BufferMode;			/* Allow way to override... */
	}
	while (file_size) {
	    size = (size_t) MIN (file_size, segment_size);
	    if ( (count = read (fd, bp, (size_t)size)) != (ssize_t)size) {
		Fprintf ("Error occured reading download file '%s'.", fnp);
		if (count == FAILURE) {
		    Perror ("read");
		} else {
		    Fprintf ("Attempted to read %d bytes, read only %d bytes.",
								size, count);
		}
		status = FAILURE;
		break;
	    }
	    if (DebugFlag) {
		Printf ("Downloading %d bytes to offset %d...\n",
							size, offset);
	    }
	    status = DoWriteBuffer (bp, (int)size, offset, mode, buf_id);
	    if (status == size) {
		status = SUCCESS;		/* Map to success. */
	    } else {
		status = FAILURE;		/* Set failure status. */
		break;
	    }
	    file_size -= size;
	    offset += size;
	}
	if ( (status == SUCCESS) && (file_size == 0) ) {
	    if (DisplayVerbose) {
		Printf ("Download completed successfully%s\n",
			( ISSET_KOPT (ce, K_SAVE_DATA) )
			? ", now saving the microcode..." : "." );
	    }
	    if ( ISSET_KOPT (ce, K_SAVE_DATA) ) {
		/*
		 * All firmware was downloaded, now just save it.
		 */
		mode = ALL_WTBUF_DOWNLOAD_SAVE;
	        status = DoWriteBuffer ((caddr_t)0, 0, 0, mode, buf_id);
	    }
	} else if ( (status == SUCCESS) && file_size) {
	    /*
	     * This is an *important* operation, ensure all bytes done.
	     */
	    Fprintf ("Only %d bytes of %ld total bytes were downloaded.",
				(sb.st_size - file_size), sb.st_size);
	    status = FAILURE;
	}
	(void) fclose (fp);
	free_palign (bp);
	return (status);
}

/************************************************************************
 *									*
 * ShowMemorySize() - Show Size of Controllers' Memory.			*
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
ShowMemorySize (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	int offset;
	u_char buf_id, mode;
	U32 memory_size;

	offset = DEFAULT_BUFFER_OFFSET;
	if ( ISSET_KOPT (ce, K_BUFFER_ID) ) {
	    buf_id = BufferId;
	} else {
	    buf_id = DEFAULT_BUFFER_ID;
	}

	if ( ISSET_KOPT (ce, K_BUFFER_MODE) ) {
	    mode = BufferMode;
	} else {
	    mode = DefaultBufferMode;
	}

	if ( (memory_size = GetMemorySize(offset, mode, buf_id)) == FAILURE) {
	    return (FAILURE);
	}
	if (mode || buf_id) {
	    Printf ("The Memory Size for Mode %d, ID %d is %u (0x%x) bytes.\n",
				mode, buf_id, memory_size, memory_size);
	} else {
	    Printf ("The Controller Memory Size is %u (0x%x) bytes.\n",
						memory_size, memory_size);
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * GetMemorySize() - Get Memory Size of Selected Buffer.		*
 *									*
 * Inputs:	offset = Offset into memory buffer.			*
 *		mode   = The write buffer mode.				*
 *		buf_id = The buffer ID value.				*
 *									*
 * Return Value:							*
 *		Returns Memory Size or / -1 for FAILURE.		*
 *									*
 ************************************************************************/
U32
GetMemorySize (int offset, u_char mode, u_char buf_id)
{
	register struct all_buffer_header *bh;
	struct scu_device *scu = ScuDevice;
	int memory_size;
	int status;

	bh = (struct all_buffer_header *) scu->scu_iop_buffer;
	status = DoReadBuffer ((caddr_t) bh, sizeof(*bh), offset, mode, buf_id);
	if (status == FAILURE) {
	    return (status);
	}
	memory_size = (U32) ( ((U32) bh->buffer_capacity2 << 16) +
			      ((U32) bh->buffer_capacity1 << 8) +
			       (U32) bh->buffer_capacity0 );
	return (memory_size);
}

/************************************************************************
 *									*
 * DoReadBuffer() - Do Read Buffer Command.				*
 *									*
 * Inputs:	buffer = The buffer to read into.			*
 *		length = The length of the transfer.			*
 *		offset = Offset into memory buffer.			*
 *		mode   = The write buffer mode.				*
 *		buf_id = The buffer ID value.				*
 *									*
 * Return Value:							*
 *		Returns # of Bytes Read / -1 = SUCCESS / FAILURE.	*
 *									*
 ************************************************************************/
int
DoReadBuffer (buffer, length, offset, mode, buf_id)
caddr_t buffer;
int length, offset;
u_char mode, buf_id;
{
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;
	struct diagnostic_params diag_params;
	register struct diagnostic_params *dp = &diag_params;
	int status;

	(void) bzero ((char *)dp, sizeof(*dp));
	(void) bzero (buffer, length);
	dp->dp_mode   = mode;  
	dp->dp_id     = buf_id;
	dp->dp_length = length;
	dp->dp_buffer = buffer;
	dp->dp_offset = offset;

	/*
	 * Prepare Special I/O Command:
	 */
	(void) bzero ((char *) sp, sizeof(*sp));
	sp->sp_sub_command = SCMD_READ_BUFFER;
	sp->sp_iop_length = sizeof(*dp);
	sp->sp_iop_buffer = (caddr_t) dp;
	sp->sp_user_length = length;
	sp->sp_user_buffer = buffer;
	if ( (status = DoSpecial (sp, read_buffer_msg)) != SUCCESS) {
	    return (status);
	}
	return (length - sp->sp_xfer_resid);
}

/************************************************************************
 *									*
 * DoWriteBuffer() - Do Write Buffer Command.				*
 *									*
 * Inputs:	buffer = The buffer to write from.			*
 *		length = The length of the transfer.			*
 *		offset = Offset into memory buffer.			*
 *		mode   = The write buffer mode.				*
 *		buf_id = The buffer ID value.				*
 *									*
 * Return Value:							*
 *		Returns # of Bytes Written / -1 = SUCCESS / FAILURE.	*
 *									*
 ************************************************************************/
int
DoWriteBuffer (buffer, length, offset, mode, buf_id)
caddr_t buffer;
int length, offset;
u_char mode, buf_id;
{
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;
	struct diagnostic_params diag_params;
	register struct diagnostic_params *dp = &diag_params;
	int status;

	(void) bzero ((char *)dp, sizeof(*dp));
	dp->dp_mode = mode;
	dp->dp_id = buf_id;
	dp->dp_length = length;
	dp->dp_buffer = buffer;
	dp->dp_offset = offset;

	/*
	 * Prepare Special I/O Command:
	 */
	(void) bzero ((char *) sp, sizeof(*sp));
	sp->sp_sub_command = SCMD_WRITE_BUFFER;
	sp->sp_iop_length = sizeof(*dp);
	sp->sp_iop_buffer = (caddr_t) dp;
	sp->sp_user_length = length;
	sp->sp_user_buffer = buffer;
	if ( (status = DoSpecial (sp, write_buffer_msg)) != SUCCESS) {
	    return (status);
	}
	return (length - sp->sp_xfer_resid);
}

/************************************************************************
 *									*
 * FillBuffer() - Fill Buffer With A Data Pattern.			*
 *									*
 * Description:								*
 *	If a pattern_buffer exists, then this data is used to fill the	*
 * buffer instead of the data pattern.					*
 *									*
 * Inputs:	buffer = Pointer to buffer to fill.			*
 *		count = Number of bytes to fill.			*
 *		pattern = Data pattern to fill buffer with.		*
 *		invert = Fills the buffer with inverted data.		*
 *									*
 * Outputs:	None.							*
 *									*
 ************************************************************************/
void
FillBuffer (buffer, count, pattern)
u_char *buffer;
int count;
U32 pattern;
{
	register u_char *bptr;
	register int i;
	union {
		u_char pat[sizeof(U32)];
		U32 pattern;
	} p;

	/*
	 * Initialize the buffer with a data pattern.  We move a byte
	 * at a time to account for non-modulo longword counts.
	 */
	bptr = buffer;
	p.pattern = pattern;
	for (i = 0; i < count; i++) {
	    *bptr++ = p.pat[i & (sizeof(U32) - 1)];
	}
}

/************************************************************************
 *									*
 * VerifyBuffer() - Verify pattern in a buffer.				*
 *									*
 * Inputs:	buffer = Pointer to data to verify.			*
 *		count = Number of bytes to compare.			*
 *		pattern = Data pattern to compare against.		*
 *									*
 * Outputs:	Returns SUCCESS/FAILURE = Data Ok/Compare Error.	*
 *									*
 ************************************************************************/
int
VerifyBuffer (buffer, count, pattern)
u_char *buffer;
int count;
U32 pattern;
{
	register int i;
	register u_char *vptr;
	union {
		u_char pat[sizeof(U32)];
		U32 pattern;
	} p;

	p.pattern = pattern;
	vptr = buffer;
	for (i = 0; i < count; i++, vptr++) {
	    if (*vptr != p.pat[i & (sizeof(U32) - 1)]) {
		Printf ("%s: Data compare error at byte position %u\n",
								OurName, i);
		Printf ("%s: Data expected = %#x, data found = %#x\n",
					OurName, p.pat[i & 3], *vptr);
		if (DumpFlag) {
		    Printf ("%s: Data buffer pointer = %#lx\n", OurName, vptr);
		    Printf ("\nData Buffer (%u bytes):\n\n", count);
		    DumpBuffer (buffer, count);
		}
		return (FAILURE);
	    }
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * DumpBuffer() Dump Data Buffer in Hex Bytes.				*
 *									*
 * Inputs:	buffer = Pointer to buffer to dump.			*
 *		size   = The size of the buffer to dump.		*
 *									*
 * Outputs:	None.							*
 *									*
 ************************************************************************/
void
DumpBuffer (buffer, size)
u_char *buffer;
int size;
{
#if !defined(CAMDEUG)
	register int count;
	register int limit = MIN(size, (int)cdbg_DumpLimit);
	register int field_entrys = 16;
	register u_char *bptr = (u_char *)buffer;

	/*
	 * Displaying a header is left to the caller.
	 */
	for (count = 0; count < limit; count++) {
	    if ((count % field_entrys) == 0) {
		Printf ("\n0x%lx ", bptr);
	    }
	    Printf (" %02x", *bptr++);
	}
	if (count) Printf ("\n");
#else /* defined(CAM_DEBUG) */
	cdbg_DumpBuffer ((char *) buffer, size);
#endif /* defined(CAMDEBUG) */

}

/************************************************************************
 *									*
 * DoSimpleCommand() - Do A Simple SCSI Command (No Parameters).	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoSimpleCommand (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;

	(void) bzero ((char *) sp, sizeof(*sp));
	sp->sp_sub_command = ke->k_cmd;
	return (DoSpecial (sp, ke->k_msgp));
}

/************************************************************************
 *									*
 * DoTestUnitReady() - Do A Test Unit Ready Command.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoTestUnitReady (ce)
struct cmd_entry *ce;
{
	struct scsi_special special_cmd;
	register struct scsi_special *sp = &special_cmd;

	(void) bzero ((char *) sp, sizeof(*sp));
	sp->sp_sub_command = SCMD_TEST_UNIT_READY;
	return (DoSpecial (sp, ce->c_msgp));
}

/************************************************************************
 *									*
 * DoTestSetup() - Do Common Test Setup for Devices.			*
 *									*
 * Description:								*
 *	This function is responsible for setting up the initial test	*
 * parameters.  This includes resetting various variables, disabling	*
 * error recovery, etc.							*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoTestSetup (ce, scu)
register struct cmd_entry *ce;
register struct scu_device *scu;
{
	u_long data_length;
	int status = SUCCESS;

	/*
	 * Setup the test parameters based on user values or defaults.
	 */
	if ( ISSET_KOPT (ce, K_ALIGN_OFFSET) ) {
	    scu->scu_align_offset = AlignOffset;
	} else {
	    scu->scu_align_offset = DefaultAlignOffset;
	}

	scu->scu_block_count = 0L;
	scu->scu_block_limit = 0L;
	if ( ISSET_KOPT (ce, K_BLOCK_SIZE) ) {
	    scu->scu_block_size = BlockSize;
	} else {
	    scu->scu_block_size = DefaultBlockSize;
	}
	if ( (scu->scu_block_size == 0L) ||
	     (scu->scu_block_size > MAX_DATA_LENGTH) ) {
	    Fprintf("The block size must be positive and non-zero.");
	    return (FAILURE);
	}

	if ( ISSET_KOPT (ce, K_BUFFER_MODE) ) {
	    scu->scu_buffer_mode = (BufferMode & 0x3);
	} else {
	    scu->scu_buffer_mode = DefaultBufferMode;
	}

	if ( ISSET_KOPT (ce, K_SEGMENT_SIZE) ) {
	    if (SegmentSize) {
		scu->scu_segment_size = SegmentSize;
		SegmentSize = 0;
	    } else {
		scu->scu_segment_size = DefaultSegmentSize;
	    }
	} else {
	    scu->scu_segment_size = DefaultSegmentSize;
	}

	/*
	 * Ensure current data buffer is large enough.
	 */
	if ( (scu->scu_test_control & TC_NO_BUFFER) == 0) {
	  data_length = (scu->scu_block_size + scu->scu_align_offset);
	  if (data_length > MAX_DATA_LENGTH) {
	    Fprintf("The data length must be positive and non-zero.");
	    return (FAILURE);
	  }
	  if (data_length > scu->scu_data_length) {
	    free_palign ((caddr_t) scu->scu_data_buffer);
	    scu->scu_data_length = data_length;
	    scu->scu_data_buffer = (caddr_t) malloc_palign (data_length);
	    if (scu->scu_data_buffer == (caddr_t) 0) {
		scu->scu_data_length = 0L;
		errno = ENOMEM;
	        Perror ("Failed to allocate I/O data buffer of %u bytes.",
								data_length);
		return (FAILURE);
	    }
	  }
	}

	/*
	 * Setup the aligned data buffer address.
	 */
	scu->scu_aligned_bptr = scu->scu_data_buffer + scu->scu_align_offset;

	if ( ISSET_KOPT (ce, K_COMPARE_FLAG) ) {
	    scu->scu_compare_flag = CompareFlag;
	} else {
	    scu->scu_compare_flag = DefaultCompareFlag;
	}

	scu->scu_data_bytes = 0L;
	if ( ISSET_KOPT (ce, K_DATA_LIMIT) ) {
	    scu->scu_data_limit = DataLimit;
	} else {
	    scu->scu_data_limit = DefaultDataLimit;
	}

	if ( ISSET_KOPT (ce, K_DATA_PATTERN) ) {
	    scu->scu_data_pattern = DataPattern;
	} else {
	    scu->scu_data_pattern = DefaultDataPattern;
	    data_patterns[0] = scu->scu_data_pattern;
	}

	if ( ISSET_KOPT (ce, K_DELAY_VALUE) ) {
	    scu->scu_delay_value = DelayValue;
	} else {
	    scu->scu_delay_value = DefaultDelayValue;
	}

	if ( ISSET_KOPT (ce, K_ENDING_LBA) ) {
	    scu->scu_ending_lba = EndingLogicalBlock;
	} else {
	    scu->scu_ending_lba = 0L;
	}

	scu->scu_error_code = 0L;
	scu->scu_error_count = 0L;
	if ( ISSET_KOPT (ce, K_ERROR_LIMIT) ) {
	    scu->scu_error_limit = ErrorLimit;
	} else {
	    scu->scu_error_limit = DefaultErrorLimit;
	}

	if ( ISSET_KOPT (ce, K_INCREMENT_COUNT) ) {
	    scu->scu_incr_count = IncrementCount;
	} else {
	    scu->scu_incr_count = DefaultIncrementCount;
	}

	scu->scu_seek_position = 0L;
	scu->scu_last_position = 0L;

	if ( ISSET_KOPT (ce, K_LBA) ) {
	    scu->scu_logical_block = LogicalBlock;
	} else {
	    scu->scu_logical_block = 0L;
	}

	scu->scu_pass_count = 0L;
	if ( ISSET_KOPT (ce, K_PASS_LIMIT) ) {
	    scu->scu_pass_limit = PassLimit;
	} else {
	    scu->scu_pass_limit = DefaultPassLimit;
	}

	if ( ISSET_KOPT (ce, K_RANDOM_FLAG) ) {
	    scu->scu_random_flag = RandomFlag;
	} else {
	    scu->scu_random_flag = DefaultRandomFlag;
	}

	if ( ISSET_KOPT (ce, K_RANDOMIZING_KEY) ) {
	    scu->scu_randomizing_key = RandomizingKey;
	} else {
	    scu->scu_randomizing_key = DefaultRandomizingKey;
	}

	scu->scu_record_count = 0L;
	scu->scu_partial_records = 0L;
	if ( ISSET_KOPT (ce, K_RECORD_LIMIT) ) {
	    scu->scu_record_limit = RecordLimit;
	} else {
	    scu->scu_record_limit = 0L;
	}

	if ( ISSET_KOPT (ce, K_RECOVERY_FLAG) ) {
	    scu->scu_recovery_flag = TestRecoveryFlag;
	} else {
	    scu->scu_recovery_flag = RecoveryFlag;
	}

	scu->scu_retry_count = 0L;
	if ( ISSET_KOPT (ce, K_RETRY_LIMIT) ) {
	    scu->scu_retry_limit = (u_char) RetryLimit;
	} else {
	    scu->scu_retry_limit = (u_char) DefaultRetryLimit;
	}

	if ( ISSET_KOPT (ce, K_STARTING_LBA) ) {

	    scu->scu_starting_lba = StartingLogicalBlock;
	} else {
	    scu->scu_starting_lba = 0L;
	}

	if ( ISSET_KOPT (ce, K_SEEK_COUNT) ) {
	    scu->scu_seek_count = SeekCount;
	} else {
	    scu->scu_seek_count = DefaultSeekCount;
	}

	if ( ISSET_KOPT (ce, K_SKIP_COUNT) ) {
	    scu->scu_skip_count = SkipCount;
	} else {
	    scu->scu_skip_count = DefaultSkipCount;
	}

	if ( ISSET_KOPT (ce, K_STEP_VALUE) ) {
	    scu->scu_step_value = StepValue;
	} else {
	    scu->scu_step_value = DefaultStepValue;
	}

	scu->scu_total_bytes = 0L;
	scu->scu_total_errors = 0L;
	scu->scu_total_records = 0L;
	scu->scu_warning_errors = 0L;

	if ( ISSET_KOPT (ce, K_VERIFY_FLAG) ) {
	    scu->scu_verify_flag = VerifyFlag;
	} else {
	    scu->scu_verify_flag = DefaultVerifyFlag;
	}

	if ( ISSET_KOPT (ce, K_VERIFY_LENGTH) ) {
	    scu->scu_verify_length = VerifyLength;
	} else {
	    scu->scu_verify_length = DefaultVerifyLength;
	    if (scu->scu_device_type == ALL_DTYPE_SEQUENTIAL) {
		if ((int)scu->scu_verify_length < 0) {
		    scu->scu_verify_length = VERIFY_TAPE_LENGTH;
		}
	    } else {
		if ((int)scu->scu_verify_length < 0) {
		    scu->scu_verify_length = VERIFY_LENGTH;
		}
	    }
	}

	if ( ISSET_KOPT (ce, K_LENGTH) ) {
	    scu->scu_xfer_length = TransferLength;
	}

	/*
	 * Do special setup for each device type (if necessary).
	 */
	if ( (scu->scu_test_control & TC_NO_SETUP) == 0) {
	  switch (scu->scu_device_type) {

	    case ALL_DTYPE_DIRECT:
	    case ALL_DTYPE_OPTICAL:
	    case ALL_DTYPE_RODIRECT:
		status = DoDirectSetup (ce, scu);
		break;

	    case ALL_DTYPE_SEQUENTIAL:
		status = DoSequentialSetup (ce, scu);
		break;
	  }
	}

	if (status == SUCCESS) {
	    if ( ISSET_KOPT (ce, K_RECOVERY_FLAG) ) {
		if (scu->scu_recovery_flag == TRUE) {
		    status = EnableRecovery (scu);
		} else {
		    status = DisableRecovery (scu);
		}
	    }
	}

	return (status);
}

/************************************************************************
 *									*
 * DoTestCleanup() - Do Test Cleanup.					*
 *									*
 * Description:								*
 *	This function is invoked after a test completes to cleanup.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoTestCleanup (ce, scu)
register struct cmd_entry *ce;
register struct scu_device *scu;
{
	int status;

	/*
	 * TODO: Display statistics & timers.
	 */

	/*
	 * If error recovery was disabled, then re-enable recovery now.
	 */
	if ( ISSET_KOPT (ce, K_RECOVERY_FLAG) &&
             (scu->scu_recovery_flag != RecoveryFlag) ) {
	    if (RecoveryFlag == TRUE) {
		status = EnableRecovery (scu);
	    } else {
		status = DisableRecovery (scu);
	    }
	}

	return (status);
}

/************************************************************************
 *									*
 * DoDirectSetup() - Do Direct Access Test Setup.			*
 *									*
 * Description:								*
 *	This function is responsible for setting up the initial test	*
 * parameters for direct access devices.				*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
DoDirectSetup (ce, scu)
register struct cmd_entry *ce;
register struct scu_device *scu;
{
	struct dir_read_cap_data capacity_data;
	register struct dir_read_cap_data *capacity = &capacity_data;
	U32 logical_blocks, block_length;
	U32 ending_lba, starting_lba = 0L, total_blocks;
	int status = SUCCESS;

	if (scu->scu_device_capacity == 0L) {
	    if ( (status = GetCapacity (capacity)) != SUCCESS) {
		return (status);
	    }

	    logical_blocks = (U32) ( ((U32)capacity->lbn3 << 24) +
				     ((U32)capacity->lbn2 << 16) +
				     ((U32)capacity->lbn1 << 8) +
				      (capacity->lbn0) + 1 );

	    scu->scu_device_capacity = logical_blocks;

	    block_length =  (U32)( (U32)capacity->block_len3 << 24) +
				   ((U32)capacity->block_len2 << 16) +
				   ((U32)capacity->block_len1 << 8) +
				    (capacity->block_len0);

	    scu->scu_device_size = block_length;
	}

	logical_blocks = (scu->scu_device_capacity - 1);
	if ( (scu->scu_test_control & TC_NO_DEFAULT) == 0) {
	    ending_lba = logical_blocks;
	} else {
	    ending_lba = -1;
	}

	/*
	 * Check the starting logical block address.
	 */
	if ( ISSET_KOPT (ce, K_LBA) ) {
	    scu->scu_starting_lba = starting_lba = scu->scu_logical_block;
	    ending_lba = starting_lba;
	}

	if ( ISSET_KOPT (ce, K_STARTING_LBA) ) {
	    starting_lba = scu->scu_starting_lba;
	    ending_lba = logical_blocks;
	}

	if (starting_lba > logical_blocks) {
	    Fprintf ("Starting block %u exceeds capacity range of (0 - %u).",
					starting_lba, (logical_blocks - 1));
	    return (FAILURE);
	}

	/*
	 * Check the ending logical block address.
	 */
	if ( ISSET_KOPT (ce, K_ENDING_LBA) ) {
	    if ( (ending_lba = scu->scu_ending_lba) < starting_lba) {
		Fprintf ("Ending block %u is less than starting block %u.",
						ending_lba, starting_lba);
		return (FAILURE);
	    }
	    total_blocks = (ending_lba - starting_lba) + 1;
	} else if ( ISSET_KOPT (ce, K_LENGTH) ) {
	    /*
	     * Length is expressed in the number of blocks.
	     */
	    total_blocks = scu->scu_xfer_length;
	    ending_lba = (starting_lba + total_blocks) - 1;
	} else if ( ISSET_KOPT (ce, K_DATA_LIMIT) ) {
	    U32 blocks;
	    /*
	     * Limit is expressed in the number of bytes.
	     */
	    total_blocks = (logical_blocks - starting_lba) + 1;
	    blocks = howmany (scu->scu_data_limit, scu->scu_device_size);
            if (blocks < total_blocks) {
		total_blocks = blocks;
	    }
	    ending_lba = (starting_lba + total_blocks) - 1;
	} else if ( ISSET_KOPT (ce, K_RECORD_LIMIT) ) {
	    U32 blocks;
	    /*
	     * Records means this many records of the block size.
	     */
	    blocks = howmany (scu->scu_block_size, scu->scu_device_size);
	    total_blocks = (blocks * scu->scu_record_limit);
	    ending_lba = (starting_lba + total_blocks) + 1;
	} else {
	    total_blocks = (ending_lba - starting_lba) + 1;
	}

	if ((I32) ending_lba < 0) {
    Fprintf ("No defaults, please specify test parameters for transfer...");
	    status = WARNING;
	} else if (ending_lba > logical_blocks) {
	    Fprintf ("Ending block %u exceeds disk capacity of %u blocks.",
						ending_lba, logical_blocks);
	    status = FAILURE;
	} else {
	    scu->scu_ending_lba = ending_lba;
	    scu->scu_starting_lba = starting_lba;
	    scu->scu_block_limit = total_blocks;
	}
	return (status);
}

/************************************************************************
 *									*
 * DoSequentialSetup() - Do Sequential Access Test Setup.		*
 *									*
 * Description:								*
 *	This function is responsible for setting up the initial test	*
 * parameters for sequential access devices.				*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
DoSequentialSetup (ce, scu)
register struct cmd_entry *ce;
register struct scu_device *scu;
{
	struct mode_page_header mode_parameters;
	register struct mode_page_header *mh = &mode_parameters;
	register struct mode_parameter_header *mph;
	register struct mode_block_descriptor *mbd;
	u_long total_bytes;
	u_char pcf = PCF_CURRENT, page_code = MODE_PARAMETERS_PAGE;
	int status;

	status = SensePage (mh, pcf, page_code, sizeof(*mh));
	if (status == FAILURE) return (status);
	else if (status == WARNING) status = SUCCESS;

	/*
	 * For fixed block devices, pickup the tape block size.
	 */
	mph = &mh->parameter_header;
	if (mph->mph_block_desc_length) {
	    U32 block_length;
	    mbd = &mh->block_descriptor;
	    block_length = (U32) (
				 ((U32)mbd->mbd_block_length_2 << 16) +
				  (mbd->mbd_block_length_1 << 8) +
				  (mbd->mbd_block_length_0) );
	    scu->scu_device_size = block_length;
	}

	total_bytes = 0L;		/* Force user to specify amount. */

	/*
	 * Check for parameters which specify the bytes to transfer.
	 */
	if ( ISSET_KOPT (ce, K_LENGTH) ) {
	    /*
	     * Length is expressed in the number of bytes or blocks.
	     */
	     if (scu->scu_device_size) {
		total_bytes = (scu->scu_xfer_length * scu->scu_device_size);
	     } else {
		total_bytes = scu->scu_xfer_length;
	     }
	} else if ( ISSET_KOPT (ce, K_DATA_LIMIT) ) {
	    /*
	     * Limit is expressed in the number of bytes.
	     */
	    total_bytes = scu->scu_data_limit;
	} else if ( ISSET_KOPT (ce, K_RECORD_LIMIT) ) {
	    /*
	     * Records means this many records of the block size.
	     */
	     if (scu->scu_device_size) {
		U32 blocks, total_blocks;
		blocks = howmany (scu->scu_block_size, scu->scu_device_size);
		total_blocks = (blocks * scu->scu_record_limit);
		total_bytes = (total_blocks * scu->scu_device_size);
	     } else {
		total_bytes = (scu->scu_block_size * scu->scu_record_limit);
	     }
	}

	if ((total_bytes == 0L) && !(ISSET_KOPT(ce, K_MEMORY_TESTS))) {
    Fprintf ("No defaults, please specify test parameters for transfer...");
	    status = WARNING;
	} else {
	    scu->scu_block_limit = total_bytes;
	}
	return (status);
}
