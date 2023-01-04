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
static char *rcsid = "@(#)$RCSfile: scu_special.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1993/01/22 17:30:20 $";
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

/************************************************************************
 *									*
 * File:	scu_special.c	(Created from cam_special.c)		*
 * Date:	April 23, 1991						*
 * Author:	Robin Miller						*
 *									*
 * Description:								*
 *	Functions to process special I/O control commands.		*
 *									*
 * Modification History:						*
 *									*
 * December 4, 1991 by Robin Miller.					*
 *	Modified scmn_AbortCCB() & scmn_TerminateCCB() functions to	*
 *	sleep at non-interruptable priority.  Previously, the user	*
 *	could interrupt these CCB's while waiting for them to complete	*
 *	which resulted in the cam_flags of the active SCSI I/O CCB not	*
 *	being updated to reflect the SIM Q was frozen. Thus, the SIM Q	*
 *	would be left in a frozen state.				*
 *									*
 *	Modified logic in function scmn_CheckError() so the command	*
 *	does not get retried on "Busy" conditions unless the CAM status	*
 *	is CAM_REQ_CMP_ERR.  Previously, if the command timed out and	*
 *	was aborted by the SIM via a BDR or RESET and the SCSI status	*
 *	was SCSI_STAT_BUSY, the command was being retried.		*
 *									*
 *	Modified function scmd_AbortCCB() to not issue a Terminate CCB	*
 *	if the Abort CCB failed.  The SCSI terminate message phase is	*
 *	an optional cmd which is not properly handled by all devices.	*
 *									*
 * November 19, 1991 by Robin Miller.					*
 *	Remove code for filling in the LUN field of the SCSI CDB's.	*
 *	In SCSI-2, the LUN is sent out as part of the message phases	*
 *	and is not required within the CDB (must be zero).		*
 *	Also added extern declarations for CAM debug routines used.	*
 *									*
 * August 24, 1991 by Robin Miller.					*
 *	In the scmn_SetupCCB() function, ensure the command code gets	*
 *	copied from the CDB pointer before calculating the CDB length.	*
 *	This is done to allow the CDB opcode to come from a proto-type	*
 *	CDB, or to be filled in by the make CDB function invoked.  This	*
 *	allows a single make CDB function to handle creation of both 6	*
 *	and 10 byte CDB's (i.e., for READ & WRITE commands).		*
 *	Also setup the LUN field of the CDB (always zero before).	*
 *									*
 * August 3, 1991 by Robin Miller.					*
 *	Converted command table list to true doubly linked list.	*
 *	Always add new command table to end of command table list.	*
 *	Pass back cam_sense_resid field on SCSI_SPECIAL I/O requests.	*
 *	Check command entry device type field if type is specified.	*
 *	Removed peripheral driver pointer from call to ccmn_ccbwait()	*
 *	function.  This is now obtained from the cam_pdrv_ptr field	*
 *	of the CCB passed into ccmn_ccbwait().				*
 *									*
 * July 30, 1991 by Robin Miller.					*
 *	Fix problem in SetupSpecial() function to always copy the I/O	*
 *	parameters buffer to kernel allocated I/O parameters buffer.	*
 *									*
 * July 29, 1991 by Maria Vella.					*
 *	Added peripheral device structure pointer on call to function	*
 *	ccmn_ccbwait().							*
 *									*
 * July 1, 1991 by Robin Miller.					*
 *	Modify code associated with SCSI_SPECIAL I/O control command,	*
 *	to allow use of this command with system requests.		*
 *									*
 * June 29, 1991 by Robin Miller.					*
 *	Added sanity check to ensure user buffer isn't allocated on the	*
 *	kernel stack for system requests.  Since the kernel stack is	*
 *	allocated as part of the u area pages, I/O buffers can't exist	*
 *	in this space since process context isn't guarenteed during the	*
 *	I/O processing (u area could be mapped to another process).	*
 *									*
 * June 27, 1991 by Robin Miller.					*
 *	Change SCSI structures & operation codes to new definitions.	*
 *									*
 ************************************************************************/

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_special.h>
#include <io/cam/scsi_status.h>
#include <io/cam/cam_debug.h>

#define mprintf Printf

/*
 * User Settable CAM CCB Flags Mask (See h/cam.h for definitions).
 */
#define CAM_CCB_FLAGS_MASK	\
  (CAM_DIS_DISCONNECT | CAM_INITIATE_SYNC | CAM_DIS_SYNC | CAM_SIM_QHEAD | \
   CAM_SIM_QFREEZE | CAM_SIM_QFRZDIS | CAM_ENG_SYNC )

/*
 * External Declarations:
 */
extern char *malloc_palign (int size);
extern void free_palign (char *pa_addr);
extern void CAMerror (struct ccb_header *ccbh);
extern u_long CamFlags;
extern long CmdInterruptedFlag, DumpFlag;
extern void Printf();


/*
 * Local Declarations:
 */
#define CSM		CAM_STATUS_MASK	/* CAM status mask (short form)	*/

/*
 * Note:  These are patchable values since the defaults may not
 *	  be adequate for all applications.
 */
int scmn_retry_limit;		  	/* The default retry limit.	*/
int scmn_retry_interval;		/* The default retry interval.	*/

#if defined(KERNEL)

#define RETRY_LIMIT	30		/* The retryable error limit.	*/
#define RETRY_INTERVAL	(hz * 2)	/* Retryable errors interval.	*/

/*
 * Macro to Check for Click Locked:
 */
#define MISLOCKED(c)	((c)->c_lock)

/*
 * External Declarations:
 */
extern int hz;
extern int copyin(), copyout(), geterror();
extern int vslock(), vsunlock(), useracc();
extern int splimp(), timeout(), wakeup();
extern struct pte *vtopte();

extern struct buf *ccmn_get_bp();
extern void ccmn_rel_bp();
extern u_char *ccmn_get_dbuf();
extern void ccmn_rel_dbuf();
extern int ccmn_ccbwait();

extern CCB_HEADER *xpt_ccb_alloc();
extern u_long xpt_ccb_free();
extern u_long xpt_action();

#endif /* defined(KERNEL) */

extern struct special_header *cam_SpecialCmds;
extern struct special_header *cam_SpecialHdrs[];

void scmn_SpecialInit(), scmn_AddSpecialCmds(), scmn_SpecialCleanup();
int scmn_FindSpecialCmd(), scmn_FindCmdEntry();

#if defined(CAMDEBUG)
/*
 * Declare External CAM Debugging Routines Used.
 */
extern caddr_t cdbg_CamStatus(), cdbg_SystemStatus();
extern void cdbg_DumpCCBHeader();
extern void cdbg_DumpSCSIIO(), cdbg_DumpABORT(), cdbg_DumpTERMIO();
extern void cdbg_DumpBuffer(), cdbg_DumpSenseData();

#endif /* defined(CAMDEBUG) */

/************************************************************************
 *									*
 * scmn_SpecialInit() - Initialize Special Command Tables.		*
 *									*
 * Description:								*
 *	This function creates a linked list of the initial special	*
 * command tables.							*
 *									*
 * Return Value:  Void.							*
 *									*
 ************************************************************************/
void
scmn_SpecialInit()
{
	register struct special_header **sptr = cam_SpecialHdrs;
	register struct special_header *sph;

	PRINTD (NOBTL, NOBTL, NOBTL, (CAMD_INOUT | CAMD_FLOW),
	    ("[b/t/l] scmn_SpecialInit: Initializing Command Tables...\n"));
	/*
	 * Insert the initial command tables.
	 */
	while (sph = *sptr) {
	    scmn_AddSpecialCmds (sph);
	    sptr++;
	}

#if defined(KERNEL)
	/*
	 * If not already set, setup the default retry limit/interval.
	 */
	if (scmn_retry_limit == 0) {
	    scmn_retry_limit = RETRY_LIMIT;	  /* Retry limit. */
	}
	if (scmn_retry_interval == 0) {
	    scmn_retry_interval = RETRY_INTERVAL; /* Retry interval. */
	}
#endif /* defined(KERNEL) */
}

/************************************************************************
 *									*
 * scmn_AddSpecialCmds() - Add Special Commands Table.			*
 *									*
 * Description:								*
 *	This function is used to add a special commands table to the	*
 * doubly linked list of command tables.				*
 *									*
 * Inputs:	sph = The command table header to add.			*
 *									*
 * Return Value:  Void.							*
 *									*
 ************************************************************************/
void
scmn_AddSpecialCmds (sph)
register struct special_header *sph;
{
	register struct special_header *shdr = cam_SpecialCmds;
	register struct special_header *sptr;

	PRINTD (NOBTL, NOBTL, NOBTL, (CAMD_INOUT | CAMD_FLOW),
	    ("[b/t/l] scmn_AddSpecialCmds: '%s Table' at sph = 0x%lx\n",
					sph->sph_table_name, sph));
	/*
	 * Add table to doubly linked list:
	 *
	 *     +------------------------------------------------+
	 *     v   +-------+       +-------+       +-------+    |
	 * Hdr---->| flink |------>| flink |------>| flink |--->|
	 *         |-------|\      |-------|\      |-------|\
	 *     |<--| blink | \<----| blink | \<----| blink | \<-+
	 *     |   +-------+       +-------+       +-------+    |
	 *     +------------------------------------------------+
	 */
	sptr = shdr->sph_blink;
	sptr->sph_flink = sph;
	sph->sph_blink = sptr;
	sph->sph_flink = shdr;
	shdr->sph_blink = sph;
}

/************************************************************************
 *									*
 * scmn_RemoveSpecialCmds() - Remove Special Commands Table.		*
 *									*
 * Description:								*
 *	This function is used to remove a special commands table from	*
 * the linked list of command tables.					*
 *									*
 * Inputs:	sph = The command table header to remove.		*
 *									*
 * Return Value:							*
 *		Return SUCCESS if table was found and removed,		*
 *		  otherwise return FAILURE if table was not found.	*
 *									*
 ************************************************************************/
int
scmn_RemoveSpecialCmds (sph)
register struct special_header *sph;
{
	register struct special_header *shdr = cam_SpecialCmds;
	register struct special_header *sptr = shdr;

	PRINTD (NOBTL, NOBTL, NOBTL, (CAMD_INOUT | CAMD_FLOW),
	    ("[b/t/l] scmn_RemoveSpecialCmds: '%s Table' at sph = 0x%lx\n",
					sph->sph_table_name, sph));

	while (sptr->sph_flink != sph) {
	    sptr = sptr->sph_flink;
	    if (sptr == shdr) {
		return (FAILURE);
	    }
	}
	/*
	 * Remove table from doubly linked list:
	 *
	 *     +------------------------------------------------+
	 *     v   +-------+       +-------+       +-------+    |
	 * Hdr---->| flink |------>| flink |------>| flink |--->|
	 *         |-------|\      |-------|\      |-------|\
	 *     |<--| blink | \<----| blink | \<----| blink | \<-+
	 *     |   +-------+       +-------+       +-------+    |
	 *     +------------------------------------------------+
	 */
	sph->sph_flink->sph_blink = sptr;
	sptr->sph_flink = sph->sph_flink;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_FindSpecialCmd() - Find Command Entry in Command Tables.	*
 *									*
 * Description:								*
 *	This function is used to loop through the command table list	*
 * to find a matching command table entry.				*
 *									*
 * Inputs:	sap = Pointer to special command argments.		*
 *		cmd = The I/O control command code.			*
 *		scmd = The I/O control sub-command code.		*
 *									*
 * Return Value:							*
 *		Returns SUCCESS/FAILURE = Entry Found/Not Found.	*
 *									*
 ************************************************************************/
int
scmn_FindSpecialCmd (sap, cmd, scmd)
register struct special_args *sap;
register u_int cmd, scmd;
{
	register struct special_header *shdr = cam_SpecialCmds;
	register struct special_header *sptr = shdr;
	register int status = FAILURE;

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
	("[%d/%d/%d] scmn_FindSpecialCmd: sap = 0x%lx, cmd = 0x%x, scmd = 0x%x\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, sap, cmd, scmd));

	if (shdr->sph_flink == shdr) {
	    scmn_SpecialInit();
	}

	while ((sptr = sptr->sph_flink) != shdr) {
	    status = scmn_FindCmdEntry (sap, sptr, cmd, scmd);
	    if (status == SUCCESS) break;
	}
	return (status);
}

/************************************************************************
 *									*
 * scmn_FindCmdEntry() - Find Command Entry in Command Tables.		*
 *									*
 * Description:								*
 *	This function is used to loop through the command table list	*
 * to find a matching command table entry.				*
 *									*
 * Inputs:	sap = Pointer to special command arguments.		*
 *		sph = Pointer to special command table header.		*
 *		cmd = The I/O control command code.			*
 *		scmd = The I/O control sub-command code.		*
 *									*
 * Return Value:							*
 *		Returns SUCCESS/FAILURE = Entry Found/Not Found.	*
 *									*
 ************************************************************************/
int
scmn_FindCmdEntry (sap, sph, cmd, scmd)
register struct special_args *sap;
register struct special_header *sph;
register u_int cmd, scmd;
{
	register struct special_cmd *spc = sph->sph_cmd_table;
	register int entry_found = 0;
	int status = FAILURE;

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
	("[%d/%d/%d] scmn_FindCmdEntry: Searching '%s Table' at 0x%lx\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
		sph->sph_table_name, sph));

	/*
	 * Check command table for correct device type.
	 */
	if (ISCLR(sph->sph_device_type, sap->sa_device_type)) {
	    return (status);
	}
	/*
	 * Search the special command table for a match.
	 */
	do {
	    if ( (sph->sph_table_flags & SPH_SUB_COMMAND) ||
		 (spc->spc_cmd_flags & SPC_SUB_COMMAND) &&
		 ( ISSET(spc->spc_device_type, sap->sa_device_type) ||
		   (spc->spc_device_type == 0) ) ) {
		if ( (spc->spc_ioctl_cmd == cmd) &&
		     (spc->spc_sub_command == scmd) ) {
		    entry_found++;
		    break;
		}
	    } else {
		if ( ISSET(spc->spc_device_type, sap->sa_device_type) ||
		     (spc->spc_device_type == 0) ) {
		    if ( (spc->spc_ioctl_cmd == cmd) ||
			 (scmd && (spc->spc_sub_command == scmd)) ) {
			entry_found++;
			break;
		    }
		}
	    }
	    spc++;
	} while (spc->spc_ioctl_cmd != END_OF_CMD_TABLE);

	if (entry_found) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
	    ("[%d/%d/%d] scmn_FindCmdEntry: Found '%s' in '%s Table'.\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				spc->spc_cmdp, sph->sph_table_name));
	    sap->sa_spc = spc;
#ifdef notdef
	    sap->sa_sph = sph;
#endif notdef
	    status = SUCCESS;
	}
	return (status);
}

/************************************************************************
 *									*
 * scmn_GetArgsBuffer() - Allocate Special Arguments Buffer.		*
 *									*
 * Inputs:	bus = The SCSI bus number.				*
 *		target = The SCSI target ID.				*
 *		lun = The logical unit number.				*
 *									*
 * Return Value:							*
 *		Returns pointer to special_args structure or NULL if a	*
 *		buffer couldn't be allocated.				*
 *									*
 ************************************************************************/
struct special_args *
scmn_GetArgsBuffer (bus, target, lun)
int bus, target, lun;
{
	struct special_args *sap;

#if defined(KERNEL)
	sap = (struct special_args *) ccmn_get_dbuf (sizeof(*sap));
#else /* !defined(KERNEL) */
	sap = (struct special_args *) malloc_palign (sizeof(*sap));
	(void) bzero ((caddr_t) sap, sizeof(*sap));
#endif /* defined(KERNEL) */

	if (sap == (struct special_args *) 0) {
	    return (sap);
	}

	PRINTD (bus, target, lun, CAMD_FLOW,
  ("[%d/%d/%d] scmn_GetArgsBuffer: Allocated args buffer at 0x%lx of %d bytes.\n",
					bus, target, lun, sap, sizeof(*sap)));
	sap->sa_bus = (u_char) bus;
	sap->sa_target = (u_char) target;
	sap->sa_lun = (u_char) lun;
	sap->sa_unit = (u_char) target;
#if defined(KERNEL)
	sap->sa_retry_limit = scmn_retry_limit;
	sap->sa_start = (int (*)()) xpt_action;
#endif /* defined(KERNEL) */
	return (sap);
}

/************************************************************************
 *									*
 * scmn_FreeArgsBuffer() - Free Special Arguments Buffer.		*
 *									*
 * Inputs:	sap = The special argments buffer.			*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_FreeArgsBuffer (sap)
struct special_args *sap;
{
	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
  ("[%d/%d/%d] scmn_FreeArgsBuffer: Freeing args buffer at 0x%lx of %d bytes.\n",
	    sap->sa_bus, sap->sa_target, sap->sa_lun, sap, sizeof(*sap)));
#if defined(KERNEL)
	ccmn_rel_dbuf (sap);
#else /* !defined(KERNEL) */
	free_palign ((caddr_t) sap);
#endif /* defined(KERNEL) */
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SpecialCmd() - Process Special I/O Control Commands.		*
 *									*
 * Description:								*
 *	This function is used to process various I/O control commands.	*
 * Although the process performed here may appear complex and possibly	*
 * an over kill, the sets are very deterministic & consistent for each	*
 * command.  The design of this special interface offers the peripheral	*
 * writers much flexibility and handles most of the tedious operations	*
 * which every driver would otherwise implement in private code.	*
 *									*
 * Inputs:	sap = Pointer to special command argments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SpecialCmd (sap)
register struct special_args *sap;
{
	register struct special_cmd *spc;
	register struct scsi_special *sp;
	register CCB_SCSIIO *ccb;
	register u_int cmd;
	u_int scmd;
	register caddr_t data;
	int status = SUCCESS;

	/*
	 * Do the initial sanity checks...So not to crash...
	 */
	if (sap == (struct special_args *) 0) {
	    return (EINVAL);
	}

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
		("[%d/%d/%d] scmn_SpecialCmd: ENTER - sap = 0x%lx\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, sap));

	/*
	 * Additional setup for new SCSI special I/O command.
	 */
	if (sap->sa_ioctl_cmd == SCSI_SPECIAL) {
	    sp = (struct scsi_special *) sap->sa_ioctl_data;
	    if ( (status = scmn_SetupSpecial (sap, sp)) != SUCCESS) {
		return (status);
	    }
	    /*
	     * If the sub-command is an I/O control command, then we're
	     * emulating a backwards compatibilty command.
	     */
	    if (sp->sp_sub_command & (IOC_VOID | IOC_INOUT)) {
		cmd = sp->sp_sub_command;
		scmd = 0;		/* Show there's no sub-command.	*/
	    } else {
		cmd = sap->sa_ioctl_cmd;/* Set the I/O control command.	*/
		scmd = sp->sp_sub_command; /* And the sub-command code.	*/
	    }
	    data = sap->sa_iop_buffer;	/* Pointer to the command data.	*/
	} else {
	    sp = (struct scsi_special *) 0;
	    cmd = sap->sa_ioctl_cmd;	/* The I/O control command.	*/
	    scmd = sap->sa_ioctl_scmd;	/* The sub-command (if any).	*/
	    data = sap->sa_ioctl_data;	/* The parameter data (if any).	*/
	}
	ccb = sap->sa_ccb; 		/* Pointer to the CCB header.	*/

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
    ("[%d/%d/%d] scmn_SpecialCmd: cmd = 0x%x, scmd = 0x%x, data = 0x%lx\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, cmd, scmd, data));

	/*
	 * Find the special command tables entry.
	 */
	if (sap->sa_sph != (struct special_header *) 0) {
	    status = scmn_FindCmdEntry (sap, sap->sa_sph, cmd, scmd);
	} else {
	    status = scmn_FindSpecialCmd (sap, cmd, scmd);
	}
	if (status != SUCCESS) {
	    return (ENXIO);		/* Special command was NOT found. */
	}
	spc = sap->sa_spc;

	/*
	 * Ensure there's appropriate access for this command.
	 */
	if ( (status = scmn_CheckCmdAccess (sap)) != SUCCESS) {
	    return (status);
	}

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
	("[%d/%d/%d] scmn_SpecialCmd: Processing '%s' command...\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, spc->spc_cmdp));

	/*
	 * Calculate the I/O parameters length (if any).  A kernel I/O
	 * parameters buffer is allocated later if a user or data buffer
	 * isn't setup prior to a data in/out command.  Otherwise, the
	 * user parameters are accessible since all functions are called
	 * in process context.
	 */
    if (sap->sa_ioctl_cmd != SCSI_SPECIAL) {
	sap->sa_iop_length = ((cmd & ~(IOC_INOUT|IOC_VOID)) >> 16);
	sap->sa_iop_buffer = data;
#ifdef notdef
	if (sap->sa_iop_length) {
	    sap->sa_iop_buffer = (char *) ccmn_get_dbuf (sap->sa_iop_length);
	    if (sap->sa_iop_buffer == NULL) {
		return (ENOMEM);
	    }
	    sap->sa_flags |= SA_ALLOCATED_IOP;
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
  ("[%d/%d/%d] scmn_SpecialCmd: Allocated I/O parameters buffer at 0x%lx of %d (0x%x) bytes.\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
		sap->sa_iop_buffer, sap->sa_iop_length, sap->sa_iop_length));
#if defined(KERNEL)
	    if (cmd & IOC_IN) {
		bcopy (data, sap->sa_iop_buffer, sap->sa_iop_length);
	    }
#endif /* defined(KERNEL) */
	}
#endif notdef
    }

	/*
	 * Allocate a CAM Control Block (if necessary).
	 */
	if (sap->sa_ccb == (CCB_SCSIIO *) 0) {
#if defined(KERNEL)
	    ccb = sap->sa_ccb = (CCB_SCSIIO *) xpt_ccb_alloc();
#else /* !defined(KERNEL) */
	    ccb = sap->sa_ccb = (CCB_SCSIIO *) malloc_palign (sizeof(*ccb));
	    (void) bzero ((caddr_t) ccb, sizeof(*ccb));
#endif /* defined(KERNEL) */
	    sap->sa_flags |= SA_ALLOCATED_CCB;
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
	    ("[%d/%d/%d] scmn_SpecialCmd: Allocated CCB buffer at 0x%lx.\n",
		    sap->sa_bus, sap->sa_target, sap->sa_lun, sap->sa_ccb));
	}

	/*
	 * Do the special command setup (if required).
	 */
	sap->sa_cmd_flags = spc->spc_cmd_flags;
	if (spc->spc_ioctl_cmd != SCSI_SPECIAL) {
	    sap->sa_cmd_parameter = spc->spc_cmd_parameter;
	}
	if ( (spc->spc_data_length < 0) &&
	     (spc->spc_setup == (int (*)()) 0) ) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
    ("[%d/%d/%d] scmn_SpecialCmd: Command '%s' requires a setup function.\n",
		    sap->sa_bus, sap->sa_target, sap->sa_lun, spc->spc_cmdp));
		status = EINVAL;
	} else if (spc->spc_setup != (int (*)()) 0) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
    ("[%d/%d/%d] scmn_SpecialCmd: Calling '%s' setup function @ 0x%lx...\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun,
			spc->spc_cmdp, spc->spc_setup));
	    status = (*spc->spc_setup)(sap, data);
	}

	/*
	 * Sanity check some of the special command arguments.
	 */
	if (status == SUCCESS) {
	    if ( (sap->sa_user_length < 0) ||
		 ( sap->sa_user_length &&
		   (sap->sa_user_buffer == (caddr_t) 0)) ||
#if defined(KERNEL)
		( sap->sa_user_length &&
		  ((sap->sa_cmd_flags & SPC_DATA_INOUT) == 0)) ||
		( (sap->sa_cmd_flags & SPC_DATA_INOUT) &&
		  ((sap->sa_flags & SA_SYSTEM_REQUEST) == 0) &&
		  !CAM_IS_KUSEG(sap->sa_user_buffer) ) ) {
#else /* !defined(KERNEL) */
		( sap->sa_user_length &&
		  ((sap->sa_cmd_flags & SPC_DATA_INOUT) == 0)) ) {
#endif /* defined(KERNEL) */
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_ERRORS,
    ("[%d/%d/%d] scmn_SpecialCmd: Invalid special arguments, aborting...\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun));
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
("[%d/%d/%d] scmn_SpecialCmd: cmd_flags = 0x%x, user_buffer = 0x%lx, user_length = 0x%d\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
		sap->sa_cmd_flags, sap->sa_user_buffer, sap->sa_user_length));
		status = EINVAL;
	    }
	}

	/*
	 * If there's a failure, then free resources & abort the request.
	 */
	if (status != SUCCESS) {
	    scmn_SpecialCleanup (sap);
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
    ("[%d/%d/%d] scmn_SpecialCmd: Completing '%s' command with ERROR status %d (%s).\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun,
			spc->spc_cmdp, status, cdbg_SystemStatus(status)));
	    return (status);
	}

	/*
	 * If there's a data buffer length, then allocate a kernel buffer.
	 * This buffer will be used for the actual I/O for commands which
	 * don't return all of the data from a particular command.
	 */
	if ( (sap->sa_data_length = spc->spc_data_length) > 0) {
#if defined(KERNEL)
	    sap->sa_data_buffer = (char *) ccmn_get_dbuf (sap->sa_data_length);
#else /* !defined(KERNEL) */
	    sap->sa_data_buffer = (char *) malloc_palign (sap->sa_data_length);
	    (void) bzero (sap->sa_data_buffer, sap->sa_data_length);
#endif /* defined(KERNEL) */d
	    if (sap->sa_data_buffer == NULL) {
		scmn_SpecialCleanup (sap);
		return (ENOMEM);
	    }
	    sap->sa_flags |= SA_ALLOCATED_DATA;
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
 ("[%d/%d/%d] scmn_SpecialCmd: Allocated data buffer at 0x%lx of %d (0x%x) bytes.\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
		sap->sa_data_buffer, sap->sa_data_length, sap->sa_data_length));
	    /*
	     * If there isn't a user buffer setup, then we'll presume this
	     * data buffer will be used for the I/O.
	     */
	    if ((sap->sa_user_buffer == (caddr_t) 0) &&
		(sap->sa_user_length == 0) ) {
		sap->sa_user_buffer = sap->sa_data_buffer;
		sap->sa_user_length = sap->sa_data_length;
	    }
	}

	/*
	 * If this command requires an I/O buffer, and a user buffer isn't
	 * already setup, then we'll presume the data resides in the I/O
	 * parameters buffer.  Therefore, a kernel buffer is allocated for
	 * data movement since the kernel buffer allocated on the stack is
	 * mapped to the users' stack.
	 */
	if ( (sap->sa_cmd_flags & SPC_DATA_INOUT) &&
	     (sap->sa_user_buffer == (caddr_t) 0) &&
	     (sap->sa_user_length == 0) ) {
	    if (sap->sa_iop_length == 0) {
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
		("[%d/%d/%d] scmn_SpecialCmd: A data buffer is required!!!\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun));
		scmn_SpecialCleanup (sap);
		return (EINVAL);
	    }
#if defined(KERNEL)
	    if ((sap->sa_flags & SA_ALLOCATED_IOP) == 0) {
		sap->sa_iop_buffer = (char *) ccmn_get_dbuf (sap->sa_iop_length);
		if (sap->sa_iop_buffer == NULL) {
		    scmn_SpecialCleanup (sap);
		    return (ENOMEM);
		}
		sap->sa_flags |= SA_ALLOCATED_IOP;
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
 ("[%d/%d/%d] scmn_SpecialCmd: Allocated I/O parameters buffer at 0x%lx of %d (0x%x) bytes.\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
		sap->sa_iop_buffer, sap->sa_iop_length, sap->sa_iop_length));
		if (cmd & IOC_IN) {
		    bcopy (data, sap->sa_iop_buffer, sap->sa_iop_length);
		}
	    }
#endif /* defined(KERNEL) */
	    sap->sa_user_buffer = sap->sa_iop_buffer;
	    sap->sa_user_length = sap->sa_iop_length;
	}

	/*
	 * The buffer to use for data movement has been placed in the
	 * user buffer field so the low level routines have a common
	 * field to access when setting up the SCSI I/O CCB.
	 */
	if (sap->sa_user_buffer != (caddr_t) 0) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
 ("[%d/%d/%d] scmn_SpecialCmd: Using user buffer at 0x%lx of %d (0x%x) bytes.\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
		sap->sa_user_buffer, sap->sa_user_length, sap->sa_user_length));
	}

#if defined(KERNEL)
	/*
	 * If there's a user buffer, check it's access & lock it in memory.
	 */
	if (sap->sa_user_length && CAM_IS_KUSEG(sap->sa_user_buffer)) {
	    if ((status = scmn_LockPages (sap, sap->sa_user_buffer,
		    sap->sa_user_length, sap->sa_cmd_flags)) != SUCCESS) {
		scmn_SpecialCleanup (sap);
		return (status);
	    }
	    sap->sa_flags |= SA_USER_LOCKED;
	}

	/*
	 * If there's a sense buffer, check it's access & lock it in memory.
	 */
	if (sap->sa_sense_length && CAM_IS_KUSEG(sap->sa_sense_buffer)) {
	    if ((status = scmn_PagesLocked (sap->sa_sense_buffer,
					sap->sa_sense_length)) != SUCCESS) {
		/*
		 * Need function to check sense buffer page being mapped
		 * by the user buffer (this is valid... just don't lock).
		 * Sense buffer MUST be wholly contained within user buffer
		 * pages to allow this (can't cross segments).
		 */
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_ERRORS,
    ("[%d/%d/%d] scmn_SpecialCmd: Sense buffer already locked, aborting...\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun));
		scmn_SpecialCleanup (sap);
		return (status);
	    }
	    if ((status = scmn_LockPages (sap, sap->sa_sense_buffer,
			sap->sa_sense_length, SPC_DATA_IN)) != SUCCESS) {
		scmn_SpecialCleanup (sap);
		return (status);
	    }
	    sap->sa_flags |= SA_SENSE_LOCKED;
	} else if ( (sap->sa_sense_length == 0) &&
		    ((ccb->cam_ch.cam_flags & CAM_DIS_AUTOSENSE) == 0) ) {
	    register PDRV_WS *pws = (PDRV_WS *)ccb->cam_pdrv_ptr;
	    /*
	     * If there's a peripheral driver working set pointer, then
	     * use the sense buffer contained within it.  Otherwise, we'll
	     * allocate a sense buffer for this I/O request.
	     */
	    if (pws != (PDRV_WS *) 0) {
		sap->sa_sense_length = DEC_AUTO_SENSE_SIZE;
		sap->sa_sense_buffer = (caddr_t) pws->pws_sense_buf;
	    } else {
		sap->sa_sense_length = sizeof(struct all_req_sns_data);
		sap->sa_sense_buffer = (char *) ccmn_get_dbuf (sap->sa_sense_length);
		sap->sa_flags |= SA_ALLOCATED_SENSE;
	    }
	}
#endif /* defined(KERNEL) */

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_FLOW,
  ("[%d/%d/%d] scmn_SpecialCmd: Using sense buffer at 0x%lx of %d (0x%x) bytes.\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
		sap->sa_sense_buffer, sap->sa_sense_length, sap->sa_sense_length));

	/*
	 * Process The Special Command.
	 */
	if (status == SUCCESS) {
	    if (spc->spc_docmd != (int (*)()) 0) {
		status = (*spc->spc_docmd)(sap, data);
	    } else {
		status = scmn_DoCmd (sap);
	    }
	}

	/*
	 * If the command was successful and data was read into the I/O
	 * parameters buffer, then copy this onto the user stack so it'll
	 * be copied back to the user by the kernel ioctl() via copyout.
	 */
	if ( (status == SUCCESS) &&
	     (sap->sa_ioctl_cmd != SCSI_SPECIAL) &&
	     (sap->sa_user_buffer == sap->sa_iop_buffer) &&
	     ((cmd & IOC_OUT) && sap->sa_iop_length) ) {
	    int copy_length = (sap->sa_iop_length - sap->sa_xfer_resid);
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
    ("[%d/%d/%d] scmn_SpecialCmd: bcopy(iop_buffer (0x%lx), data (0x%lx), length (0x%x))\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				sap->sa_iop_buffer, data, copy_length));
	    bcopy (sap->sa_iop_buffer, data, copy_length);
	} else if (sap->sa_ioctl_cmd == SCSI_SPECIAL) {
	    (void) scmn_FinishSpecial (sap, sp);
	}

	/*
	 * If we didn't wait for the I/O to complete, then it's the
	 * users' responsibility to free the resources we allocated.
	 */
	if ( (sap->sa_flags & SA_NO_WAIT_FOR_IO) == 0) {
	    scmn_SpecialCleanup (sap);
	}

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
    ("[%d/%d/%d] scmn_SpecialCmd: Completing '%s' command with status %d (%s).\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun,
			spc->spc_cmdp, status, cdbg_SystemStatus(status)));
	return (status);
}

/************************************************************************
 *									*
 * scmn_SpecialCleanup() - Special Command Cleanup.			*
 *									*
 * Description:								*
 *	This functions' purpose is to free any allocated resources that	*
 * were previously obtained to process a special command.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
static void
scmn_SpecialCleanup (sap)
register struct special_args *sap;
{
	if (sap->sa_flags & SA_ALLOCATED_CCB) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
	    ("[%d/%d/%d] scmn_SpecialCleanup: Freeing CCB buffer at 0x%lx.\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, sap->sa_ccb));
#if defined(KERNEL)
	    (void) xpt_ccb_free (sap->sa_ccb);
#else /* !defined(KERNEL) */
	    free_palign ((caddr_t) sap->sa_ccb);
#endif /* defined(KERNEL) */
	    sap->sa_ccb = (CCB_SCSIIO *) 0;
	    sap->sa_flags &= ~SA_ALLOCATED_CCB;
	}

	/*
	 * Free the data buffer (if we allocated one).
	 */
	if (sap->sa_flags & SA_ALLOCATED_DATA) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
    ("[%d/%d/%d] scmn_SpecialCleanup: Freeing data buffer at 0x%lx of %d (0x%x) bytes.\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
	        sap->sa_data_buffer, sap->sa_data_length, sap->sa_data_length));
#if defined(KERNEL)
	    ccmn_rel_dbuf (sap->sa_data_buffer);
#else /* !defined(KERNEL) */
	    free_palign ((caddr_t) sap->sa_data_buffer);
#endif /* defined(KERNEL) */
	    sap->sa_data_length = 0;
	    sap->sa_data_buffer = (caddr_t) 0;
	    sap->sa_flags &= ~SA_ALLOCATED_DATA;
	}

	/*
	 * Free the sense buffer (if we allocated one).
	 */
	if (sap->sa_flags & SA_ALLOCATED_SENSE) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
    ("[%d/%d/%d] scmn_SpecialCleanup: Freeing sense buffer at 0x%lx of %d (0x%x) bytes.\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
	        sap->sa_sense_buffer, sap->sa_sense_length, sap->sa_sense_length));
#if defined(KERNEL)
	    ccmn_rel_dbuf (sap->sa_sense_buffer);
#else /* !defined(KERNEL) */
	    free_palign ((caddr_t) sap->sa_sense_buffer);
#endif /* defined(KERNEL) */
	    sap->sa_sense_length = 0;
	    sap->sa_sense_buffer = (caddr_t) 0;
	    sap->sa_flags &= ~SA_ALLOCATED_SENSE;
	}

	/*
	 * Free the I/O Parameters buffer (if any).
	 */
	if (sap->sa_flags & SA_ALLOCATED_IOP) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
("[%d/%d/%d] scmn_SpecialCleanup: Freeing I/O parameters buffer at 0x%lx of %d (0x%x) bytes.\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
		sap->sa_iop_buffer, sap->sa_iop_length, sap->sa_iop_length));
#if defined(KERNEL)
	    ccmn_rel_dbuf (sap->sa_iop_buffer);
#else /* !defined(KERNEL) */
	    free_palign ((caddr_t) sap->sa_iop_buffer);
#endif /* defined(KERNEL) */
	    sap->sa_iop_length = 0;
	    sap->sa_iop_buffer = (caddr_t) 0;
	    sap->sa_flags &= ~SA_ALLOCATED_IOP;
	}

#if defined(KERNEL)
	/*
	 * If there's a user buffer, unlock the previously locked pages.
	 */
	if (sap->sa_flags & SA_USER_LOCKED) {
	    (void) scmn_UnlockPages (sap, sap->sa_user_buffer,
				sap->sa_user_length, sap->sa_cmd_flags);
	    sap->sa_flags &= ~SA_USER_LOCKED;
	}

	/*
	 * If there's a sense buffer, unlock the previously locked pages.
	 */
	if (sap->sa_flags & SA_SENSE_LOCKED) {
	    (void) scmn_UnlockPages (sap, sap->sa_sense_buffer,
					sap->sa_sense_length, SPC_DATA_IN);
	    sap->sa_flags &= ~SA_SENSE_LOCKED;
	}
#endif /* defined(KERNEL) */
}

/************************************************************************
 *									*
 * scmn_SetupSpecial() - Setup For SCSI Special I/O Control Command.	*
 *									*
 * Description:								*
 *	This functions' purpose is to copy parameters from the SCSI	*
 * special command structure to the special argument structure.  This	*
 * permits a common I/O control structure to be used with all commands.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		sp = SCSI special I/O control command pointer.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
static int
scmn_SetupSpecial (sap, sp)
register struct special_args *sap;
register struct scsi_special *sp;
{
	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
		("[%d/%d/%d] scmn_SetupSpecial: sap = 0x%lx, sp = 0x%lx\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, sap, sp));

	sap->sa_flags |= (sp->sp_flags & SA_USER_FLAGS_MASK);
	sap->sa_cmd_parameter = sp->sp_cmd_parameter;

	/*
	 * If the sub-command is an existing I/O control command which
	 * requires I/O parameters, then ensure they're specified.
	 */
	if (sp->sp_sub_command & IOC_INOUT) {
	    if ( (sp->sp_iop_length == 0) ||
	         (sp->sp_iop_buffer == (caddr_t) 0) ) {
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_ERRORS,
    ("[%d/%d/%d] scmn_SetupSpecial: I/O parameter buffer & length required.\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun));
		return (EINVAL);
	    }
	}

	/*
	 * Setup the I/O parameters buffer & length (if any).
	 */
	if (sp->sp_iop_length && sp->sp_iop_buffer) {
	    sap->sa_iop_length = sp->sp_iop_length;
#if !defined(KERNEL)
	    sap->sa_iop_buffer = sp->sp_iop_buffer;
#else /* defined(KERNEL) */
	    if (sap->sa_flags & SA_SYSTEM_REQUEST) {
		sap->sa_iop_buffer = sp->sp_iop_buffer;
	    } else {
		 if (!CAM_IS_KUSEG(sp->sp_iop_buffer)) {
		    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_ERRORS,
	("[%d/%d/%d] scmn_SetupSpecial: User I/O parameter buffer required.\n",
		    sap->sa_bus, sap->sa_target, sap->sa_lun));
		    return (EINVAL);
		}
		/*
		 * Copy the users' I/O parameter buffer to a kernel
		 * buffer to avoid access and/or lock problems.
		 */
		sap->sa_iop_buffer = (char *) ccmn_get_dbuf (sap->sa_iop_length);
		if (sap->sa_iop_buffer == NULL) {
		    return (ENOMEM);
		}
		sap->sa_flags |= SA_ALLOCATED_IOP;
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
  ("[%d/%d/%d] scmn_SetupSpecial: Allocated I/O parameters buffer at 0x%lx of %d (0x%x) bytes.\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
		sap->sa_iop_buffer, sap->sa_iop_length, sap->sa_iop_length));
		/*
		 * Copy the I/O parameters buffer (if necessary).
		 */
		if ( ((sp->sp_sub_command & IOC_IN) ||
		      (sp->sp_sub_command & (IOC_VOID | IOC_INOUT)) == 0) ) {
		    if (copyin (sp->sp_iop_buffer, sap->sa_iop_buffer,
						sap->sa_iop_length) != 0) {
			scmn_SpecialCleanup (sap);
			return (EFAULT);
		    }
		}
	    }
#endif /* !defined(KERNEL) */
	}

	/*
	 * Setup the sense buffer and length (if any).
	 */
	if (sp->sp_sense_length && sp->sp_sense_buffer) {
#if defined(KERNEL)
	    if ( ((sap->sa_flags & SA_SYSTEM_REQUEST) == 0) &&
		 (!IS_KUSEG(sp->sp_sense_buffer)) ) {
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_ERRORS,
		("[%d/%d/%d] scmn_SetupSpecial: User sense buffer required.\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun));
		return (EINVAL);
	    }
#endif /* defined(KERNEL) */
	    sap->sa_sense_length = sp->sp_sense_length;
	    sap->sa_sense_buffer = sp->sp_sense_buffer;
	}

	/*
	 * Setup the user buffer & length (if any).
	 */
	if (sp->sp_user_length && sp->sp_user_buffer) {
#if defined(KERNEL)
	    if ( ((sap->sa_flags & SA_SYSTEM_REQUEST) == 0) &&
		 (!IS_KUSEG(sp->sp_user_buffer)) ) {
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_ERRORS,
		("[%d/%d/%d] scmn_SetupSpecial: User data buffer required.\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun));
		return (EINVAL);
	    }
#endif /* defined(KERNEL) */
	    sap->sa_user_length = sp->sp_user_length;
	    sap->sa_user_buffer = sp->sp_user_buffer;
	}

	/*
	 * Setup the retry & timeout fields if defaults are overridden.
	 */
	if (sp->sp_retry_limit) {
	    sap->sa_retry_limit = sp->sp_retry_limit;
	}
	if (sp->sp_timeout) {
	    sap->sa_timeout = sp->sp_timeout;
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_FinishSpecial() - Finish SCSI Special I/O Control Command.	*
 *									*
 * Description:								*
 *	This functions' purpose is to pass back necessary parameters	*
 * when completing the SCSI special I/O control command.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		sp = SCSI special I/O control command pointer.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
static int
scmn_FinishSpecial (sap, sp)
register struct special_args *sap;
register struct scsi_special *sp;
{
	int status = SUCCESS;

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
		("[%d/%d/%d] scmn_FinishSpecial: sap = 0x%lx, sp = 0x%lx\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, sap, sp));

#if defined(KERNEL)
	/*
	 * If there's an I/O parameters buffer, copy the kernel buffer
	 * to the users' I/O parameter buffer (if necessary).
	 */
	if ( (sp->sp_iop_length && sp->sp_iop_buffer) &&
	     (sp->sp_sub_command & IOC_OUT) ) {
	    int copy_length = (sap->sa_iop_length - sap->sa_xfer_resid);
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
  ("[%d/%d/%d] scmn_FinishSpecial: bcopy/copyout(iop_buffer (0x%lx), data (0x%lx), length (0x%x))\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun,
			sap->sa_iop_buffer, sp->sp_iop_buffer, copy_length));
	    if (sap->sa_flags & SA_SYSTEM_REQUEST) {
		bcopy (sap->sa_iop_buffer, sp->sp_iop_buffer, copy_length);
	    } else {
		if (copyout (sap->sa_iop_buffer, sp->sp_iop_buffer,
							copy_length) != 0) {
		    return (EFAULT);
		}
	    }
	}
#endif /* defined(KERNEL) */
	sp->sp_retry_count = sap->sa_retry_count;
	sp->sp_sense_resid = sap->sa_sense_resid;
	sp->sp_xfer_resid = sap->sa_xfer_resid;
	return (status);
}

/************************************************************************
 *									*
 * scmn_CheckCmdAccess() - Check Command Access Requirments.		*
 *									*
 * Description:								*
 *	This functions' purpose is to ensure the user has the proper	*
 * file access, system access, and that the command being issued is	*
 * valid for the current device type.					*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
static int
scmn_CheckCmdAccess (sap)
register struct special_args *sap;
{
	register struct special_cmd *spc = sap->sa_spc;
	register int file_flags = sap->sa_file_flags;
	register int status = SUCCESS;

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
("[%d/%d/%d] scmn_CheckCmdAccess: sap = 0x%lx, spc = 0x%lx, file_flags = 0x%x\n",
	sap->sa_bus, sap->sa_target, sap->sa_lun, sap, spc, file_flags));

	/*
	 * Ensure the device was open with the proper access mode.
	 * For example, if a disk device isn't open with FWRITE access,
	 * the special open code DOESN'T check for mounted file systems.
	 */
	if ( (spc->spc_file_flags & file_flags) == 0) {
	    if (spc->spc_file_flags & FWRITE) {
		return (EROFS);		/* Requires write access. */
	    } else {
		return (EACCES);	/* Requires other access. */
	    }
	}

	/*
	 * See if command requires super-user privilege.
	 */
#if defined(KERNEL)
	if ( (sap->sa_cmd_flags & SPC_SUSER) && !suser() ) {
	    return (EACCES);
	}
#else /* !define(KERNEL) */
	if ( (sap->sa_cmd_flags & SPC_SUSER) && getuid() ) {
	    return (EACCES);
	}
#endif /* defined(KERNEL) */

	/*
	 * If this is a CD-ROM device, a flag needs checked to determine
	 * if the drive supports audio commands.  This flag should be set
	 * in the open routine... this code doesn't exist at this time.
	 */
	return (status);
}

/************************************************************************
 *									*
 * scmn_DoCmd() - Prepare & Issue A Special Command.			*
 *									*
 * Description:								*
 *	This function queues the special command, then checks for and	*
 * reports an error via mprintf() if error logging is enabled.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_DoCmd (sap)
register struct special_args *sap;
{
	register CCB_SCSIIO *ccb = sap->sa_ccb;
	register int status;

	do {
	    (void) bzero (sap->sa_sense_buffer, sap->sa_sense_length);
	    /*
	     * If the command succeeded, or was interrupted by a signal,
	     * or we didn't wait for completion, then return the status.
	     */
	    if ( ((status = scmn_QueueCCB (sap)) == SUCCESS) ||
		 (status == EINTR) ||
		 (sap->sa_flags & SA_NO_WAIT_FOR_IO) ) {
		return (status);
	    }

	    /*
	     * If error recovery is disabled, then report the error
	     * and simply return.
	     */
	    if (sap->sa_flags & SA_NO_ERROR_RECOVERY) {
		status = scmn_ReportError (sap, ccb);
		return (status);
	    }

	    /*
	     * If this is a retryable error, then reissue the command.
	     * Otherwise, report the error condition and return if not
	     * a retryable error.
	     */
	    if ((status = scmn_CheckError (sap, ccb)) != RETRYABLE) {
		if (status == EINTR) {
		    break;		/* Sleep aborted by signal. */
		}
		/*
		 * Report the error condition.
		 */
		if ((status = scmn_ReportError (sap, ccb)) != RETRYABLE) {
		    return (status);
		}
	    }

	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
		("[%d/%d/%d] scmn_DoCmd: Retrying '%s' command...\n",
	    sap->sa_bus, sap->sa_target, sap->sa_lun, sap->sa_spc->spc_cmdp));

	    sap->sa_retry_count++;
	} while (sap->sa_retry_count < sap->sa_retry_limit);

#if !defined(KERNEL)
	CAMerror ((struct ccb_header *) ccb); /* Report error after retrys. */
#endif /* !defined(KERNEL) */

	return (EIO);
}

/************************************************************************
 *									*
 * scmn_CheckError() - Check for Retryable Error Conditions.		*
 *									*
 * Description:								*
 *	This function checks for various retryable error conditions.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		ccb = The SCSI I/O CAM Control Block which errored.	*
 *									*
 * Return Value:							*
 *		Returns RETRYABLE/NOT_RETRYABLE status codes.		*
 *									*
 ************************************************************************/
int
scmn_CheckError (sap, ccb)
register struct special_args *sap;
register CCB_SCSIIO *ccb;
{
	register CCB_HEADER *ccbh = (CCB_HEADER *) ccb;
	register struct all_req_sns_data *sense;

	/*
	 * If the SCSI device status is "BUSY" or the CAM status is
	 * "SELECTION TIMEOUT", then delay & then retry the command.
	 */
	switch (ccbh->cam_status&CSM) {

	    case CAM_REQ_CMP_ERR:
		if ( (ccb->cam_scsi_status != SCSI_STAT_BUSY) &&
		     (ccb->cam_scsi_status != SCSI_STAT_RESERVATION_CONFLICT) ) {
			break;
		}
		/*FALLTHROUGH*/
	    case CAM_SEL_TIMEOUT: {
#if defined(KERNEL)
		int priority = (PCATCH | (PZERO + 1));
		(void) timeout (wakeup, (caddr_t)ccb, scmn_retry_interval);
		if (SLEEP ((caddr_t)ccb, priority)) {
		    return (EINTR);
		} else {
		    return (RETRYABLE);
		}
#else /* !defined(KERNEL) */
		(void) sleep (2);
		if (CmdInterruptedFlag) {
		    return (EINTR);
		} else {
		    return (RETRYABLE);
		}
#endif /* defined(KERNEL) */
		/*NOTREACHED*/
		break;
	    }
	    default:
		break;
	}

	sense = (struct all_req_sns_data *) sap->sa_sense_buffer;
	/*
	 * If the sense key is 'Unit Attention' (target reset), then
	 * retry the command without logging an error.
	 */
	if ( (ccbh->cam_status & CAM_AUTOSNS_VALID) &&
	     (sense->sns_key == ALL_UNIT_ATTEN) ) {
	    return (RETRYABLE);
	}

	return (NOT_RETRYABLE);
}

/************************************************************************
 *									*
 * scmn_ReportError() - Report An Error Message for Failed Command.	*
 *									*
 * Description:								*
 *	This function reports an error message, if logging is enabled.	*
 * Either the supplied error routine is invoked to handle/report the	*
 * error or a generic error message is reported.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		ccb = The SCSI I/O CAM Control Block which errored.	*
 *									*
 * Return Value:							*
 *		Returns SUCCESS for non-fatal/expected errors,		*
 *			RETRYABLE if command should be retried,		*
 *				or					*
 *			error code on failures.				*
 *									*
 ************************************************************************/
int
scmn_ReportError (sap, ccb)
register struct special_args *sap;
register CCB_SCSIIO *ccb;
{
	register CCB_HEADER *ccbh = (CCB_HEADER *) ccb;
	register struct special_cmd *spc = sap->sa_spc;
	register struct all_req_sns_data *sense;
	register int status = EIO;

	/*
	 * Log the error (unless logging is disabled).
	 */
	if ((sap->sa_flags & SA_NO_ERROR_LOGGING) == 0) {
	    sense = (struct all_req_sns_data *) sap->sa_sense_buffer;
	    /*
	     * Invoke the drivers' error function (if specified).
	     */
	    if (sap->sa_error != (int (*)()) 0) {
		status = (*sap->sa_error)(ccb, sense);
	    } else {
		/*
		 * Generalized Error Reporter.
		 */
		if (ccbh->cam_status & CAM_AUTOSNS_VALID) {
    mprintf ("[%d/%d/%d] scmn_ReportError: %s: %s failed, sense key = 0x%x\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun,
			sap->sa_device_name, spc->spc_cmdp, sense->sns_key);
		} else if ((ccbh->cam_status&CSM) != CAM_REQ_CMP_ERR) {
    mprintf ("[%d/%d/%d] scmn_ReportError: %s: %s failed, cam status = 0x%x\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun,
			sap->sa_device_name, spc->spc_cmdp,
			(ccbh->cam_status&CSM));
		} else {
    mprintf ("[%d/%d/%d] scmn_ReportError: %s: %s failed, scsi status = 0x%x\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun,
			sap->sa_device_name, spc->spc_cmdp,
			ccb->cam_scsi_status);
		}
	    }
#if defined(STOP_DEBUG_ON_ERRORS)
	    camdbg_flag = 0;
#endif
	}
	return (status);
}

#if defined(KERNEL)
/************************************************************************
 *									*
 * scmn_CompleteCCB() - CCB Completion Handler.				*
 *									*
 * Description:								*
 *	This function is the completion handler for CCB.  It's called	*
 * by the CAM sub-system at HBA interrupt level to complete a CCB I/O	*
 * request.								*
 *									*
 * Inputs:	ccb = The CCB being completed.				*
 *									*
 * Return Value:							*
 *		Void.							*
 *									*
 ************************************************************************/
void
scmn_CompleteCCB (ccb)
register CCB_HEADER *ccb;
{
	PRINTD (ccb->cam_path_id, ccb->cam_target_id,
		ccb->cam_target_lun, CAMD_INOUT,
    ("[%d/%d/%d] scmn_CompleteCCB: Completing I/O request for CCB at 0x%lx\n",
	ccb->cam_path_id, ccb->cam_target_id, ccb->cam_target_lun, ccb));

	(void) wakeup (ccb);
}
#endif /* defined(KERNEL) */

/************************************************************************
 *									*
 * scmn_QueueCCB() - Prepare And Queue A CCB I/O Request.		*
 *									*
 * Description:								*
 *	This function handles filling in of the CCB request packet and	*
 * queuing the request to the XPT action routine.  The iowait flag	*
 * controls waiting for completion of the I/O request.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_QueueCCB (sap)
register struct special_args *sap;
{
	register CCB_SCSIIO *ccb = sap->sa_ccb;
	register CCB_HEADER *ccbh = (CCB_HEADER *)sap->sa_ccb;
#if defined(KERNEL)
	register struct buf *bp;
	register int status = SUCCESS, priority;
#else /* !defined(KERNEL) */
	register int status = SUCCESS;
#endif /* defined(KERNEL) */

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
	("[%d/%d/%d] scmn_QueueCCB: Queuing I/O request for CCB at 0x%lx\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun, ccb));

	if ( (status = scmn_SetupCCB (sap)) != SUCCESS) {
	    return (status);
	}
#if defined(KERNEL)
	bp = sap->sa_bp;	/* Incase dynamically allocated. */
#endif /* defined(KERNEL) */

	/*
	 * Dump The Outgoing Data Buffer.
	 */
	if (sap->sa_cmd_flags & SPC_DATA_OUT) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
	("[%d/%d/%d] scmn_QueueCCB: Data sent for '%s' command:\n",
	    sap->sa_bus, sap->sa_target, sap->sa_lun, sap->sa_spc->spc_cmdp));
	    CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
		cdbg_DumpBuffer(ccb->cam_data_ptr,ccb->cam_dxfer_len));
	}

	/*
	 * Start the I/O request via the CCB start routine.
	 */
	(void) (*sap->sa_start)(ccb);

#if defined(KERNEL)
	/*
	 * If requested, wait for the I/O request to complete.
	 */
	if ( ((sap->sa_flags & SA_NO_WAIT_FOR_IO) == 0) &&
	     ((ccbh->cam_flags & CAM_DIS_CALLBACK) == 0) ) {
	    if ( (sap->sa_flags & SA_NO_SLEEP_INTR) ||
		 (sap->sa_cmd_flags & SPC_NOINTR) ) {
		priority = PRIBIO;	/* Don't interrupt the request.	*/
	    } else {
		priority = (PZERO + 1);	/* Allow signals to interrupt.	*/
	    }
	    if ((status = ccmn_ccbwait (ccb, priority)) == SUCCESS) {
		if ( (ccbh->cam_status&CSM) != CAM_REQ_CMP) {
		    PRINTD (sap->sa_bus, sap->sa_target,
			    sap->sa_lun, CAMD_ERRORS,
	("[%d/%d/%d] scmn_QueueCCB: '%s' failed, CAM status = 0x%x (%s)\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				sap->sa_spc->spc_cmdp, ccbh->cam_status,
				cdbg_CamStatus(ccbh->cam_status,CDBG_FULL)));
		    status = EIO;
		    if (ccbh->cam_status & CAM_AUTOSNS_VALID) {
			CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_ERRORS,
				cdbg_DumpSenseData (sap->sa_sense_buffer));
		    }
		} else {
		    /*
		     * Dump The Incoming Data Buffer.
		     */
		    if (sap->sa_cmd_flags & SPC_DATA_IN) {
			PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
		("[%d/%d/%d] scmn_QueueCCB: Data received for '%s' command:\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, sap->sa_spc->spc_cmdp));
			CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
    cdbg_DumpBuffer(ccb->cam_data_ptr,(ccb->cam_dxfer_len-ccb->cam_resid)));
		    }
		}
		sap->sa_sense_resid = ccb->cam_sense_resid;
		sap->sa_xfer_resid = (int) ccb->cam_resid;
		/*
		 * An I/O buffer (bp) is only required if data movement is
		 * done to a user buffer (not required for a kernel buffer).
		 */
		if (bp != (struct buf *) 0) {
		    bp->b_flags &= ~(B_BUSY | B_WANTED | B_PHYS);
		}
	    }

	    /*
	     * If the CCB sleep was interrupted via a signal, then we'll
	     * abort the CCB which is still active.
	     */
	    if (status == EINTR) {
		(void) scmn_AbortCCB (ccb);
	    }

	    /*
	     * We do I/O to kernel buffers without a request buffer.
	     */
	    if (bp != (struct buf *) 0) {
		if (bp->b_flags & B_PHYS) {
		    CLEAR_P_VM(u.u_procp, SPHYSIO);
		}
	    }

	    CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
							cdbg_DumpSCSIIO(ccb));
	    /*
	     * If permitted, release the SIMQ if it's frozen.
	     */
	    if ( (ccbh->cam_status & CAM_SIM_QFRZN) &&
			((sap->sa_flags & SA_NO_SIMQ_THAW) == 0) ) {
		(void) scmn_ReleaseSIMQ (ccb);
	    }

	    if (sap->sa_flags & SA_ALLOCATED_BP) {
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_FLOW,
	("[%d/%d/%d] scmn_QueueCCB: Freeing I/O request buffer at 0x%lx\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun, bp));
		ccmn_rel_bp (bp);
		sap->sa_bp = (struct buf *) 0;
		sap->sa_flags &= ~SA_ALLOCATED_BP;
	    }
	} /* End 'if ( ((sap->sa_flags & SA_NO_WAIT_FOR_IO) == 0) &&' */

#else /* !defined(KERNEL) */

	if ( (ccbh->cam_status&CSM) != CAM_REQ_CMP) {
	    PRINTD (sap->sa_bus, sap->sa_target,
			    sap->sa_lun, CAMD_ERRORS,
	("[%d/%d/%d] scmn_QueueCCB: '%s' failed, CAM status = 0x%x (%s)\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				sap->sa_spc->spc_cmdp, ccbh->cam_status,
				cdbg_CamStatus(ccbh->cam_status,CDBG_FULL)));
	    status = EIO;
	    if (ccbh->cam_status & CAM_AUTOSNS_VALID) {
		CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_ERRORS,
				cdbg_DumpSenseData (sap->sa_sense_buffer));
	    }
	    CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
							cdbg_DumpSCSIIO(ccb));
	    /*
	     * If permitted, release the SIMQ if it's frozen.
	     */
	    if ( (ccbh->cam_status & CAM_SIM_QFRZN) &&
			((sap->sa_flags & SA_NO_SIMQ_THAW) == 0) ) {
		(void) scmn_ReleaseSIMQ (ccb);
	    }
	} else {
	    /*
	     * Dump The Incoming Data Buffer.
	     */
	    if (sap->sa_cmd_flags & SPC_DATA_IN) {
		u_long data_length;
		if (DumpFlag) {
		    data_length = ccb->cam_dxfer_len;
		} else {
		    data_length = (ccb->cam_dxfer_len - ccb->cam_resid);
		}
#if defined(lint)
		data_length = data_length;
#endif /* defined(lint) */
		PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
		("[%d/%d/%d] scmn_QueueCCB: Data received for '%s' command:\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun, sap->sa_spc->spc_cmdp));
			CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
			cdbg_DumpBuffer(ccb->cam_data_ptr, data_length));
	    }
	    CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
							cdbg_DumpSCSIIO(ccb));

	}
	sap->sa_sense_resid = ccb->cam_sense_resid;
	sap->sa_xfer_resid = (int) ccb->cam_resid;
#endif /* defined(KERNEL) */

	return (status);
}

#if defined(KERNEL)
/************************************************************************
 *									*
 * scmn_AbortCCB() - Prepare And Queue An Abort CCB.			*
 *									*
 * Description:								*
 *	This function handles allocating and filling in of an Abort CCB	*
 * request packet to abort an active I/O request.			*
 *									*
 * Inputs:	ccb = The CAM Control Block to terminate.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_AbortCCB (ccb)
register CCB_SCSIIO *ccb;
{
	register CCB_HEADER *ccbh = (CCB_HEADER *) ccb;
	register CCB_ABORT *accb;
	int priority = PRIBIO;
	int status = SUCCESS;

	PRINTD (ccbh->cam_path_id, ccbh->cam_target_id,
		ccbh->cam_target_lun, CAMD_ERRORS,
		("[%d/%d/%d] scmn_AbortCCB: Aborting CCB at 0x%lx...\n",
	ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun, ccbh));

	/*
	 * Allocate a CCB to Abort the I/O.
	 */
	accb = (CCB_ABORT *) xpt_ccb_alloc();

	/* 
	 * Setup the CCB header from SCSI I/O CCB.
	 */
	accb->cam_ch.cam_ccb_len = (u_short) sizeof(*accb);
	accb->cam_ch.cam_func_code = (u_char) XPT_ABORT;
	accb->cam_ch.cam_path_id = ccbh->cam_path_id;
	accb->cam_ch.cam_target_id = ccbh->cam_target_id;
	accb->cam_ch.cam_target_lun = ccbh->cam_target_lun;
	accb->cam_abort_ch = (CCB_HEADER *) ccb;

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_ERRORS, cdbg_DumpABORT(accb));
	/*
	 * Issue the Abort CCB.
	 */
	xpt_action ((CCB_HEADER *) accb); 

	/*
	 * CAM status must be complete for an Abort CCB.
	 */
	if ( (accb->cam_ch.cam_status&CSM) != CAM_REQ_CMP) {
	    PRINTD (ccbh->cam_path_id, ccbh->cam_target_id,
		    ccbh->cam_target_lun, CAMD_ERRORS,
		("[%d/%d/%d] scmn_AbortCCB: Abort CCB failed...\n",
		ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun));

	    CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_ERRORS, cdbg_DumpABORT(accb));
	    status = !SUCCESS;
	} else {
	    /*
	     * After issuing the Abort CCB, wait for the CCB to complete.
	     */
	    status = ccmn_ccbwait (ccb, priority);
	}
	(void) xpt_ccb_free ((CCB_HEADER *) accb);

	return (status);
}
#endif /* defined(KERNEL) */

/************************************************************************
 *									*
 * scmn_ReleaseSIMQ() - Prepare And Queue A Release SIM Q CCB.		*
 *									*
 * Description:								*
 *	This function handles allocating and filling in of a release	*
 * SIM CCB request packet to unfreeze the SIM Q.  This must be done to	*
 * allow the next request to be processed by the SIM.			*
 *									*
 * Inputs:	ccb = The CAM Control Block which errored.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_ReleaseSIMQ (ccb)
register CCB_SCSIIO *ccb;
{
	register CCB_HEADER *ccbh = (CCB_HEADER *) ccb;
	register CCB_RELSIM *rccb;
	int status = SUCCESS;

	PRINTD (ccbh->cam_path_id, ccbh->cam_target_id,
		ccbh->cam_target_lun, CAMD_ERRORS,
	("[%d/%d/%d] scmn_ReleaseSIMQ: Releasing SIM Q for CCB at 0x%lx\n",
	ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun, ccbh));

	/*
	 * Allocate CCB for Release SIM Q.
	 */
#if defined(KERNEL)
	rccb = (CCB_RELSIM *) xpt_ccb_alloc();
#else /* !defined(KERNEL) */
	rccb = (CCB_RELSIM *) malloc_palign (sizeof(*rccb));
	(void) bzero ((char *) rccb, sizeof(*rccb));
#endif /* defined(KERNEL) */

	/* 
	 * Setup the CCB header from SCSI I/O CCB.
	 */
	rccb->cam_ch.cam_ccb_len = (u_short) sizeof(*rccb);
	rccb->cam_ch.cam_func_code = (u_char) XPT_REL_SIMQ;
	rccb->cam_ch.cam_path_id = ccbh->cam_path_id;
	rccb->cam_ch.cam_target_id = ccbh->cam_target_id;
	rccb->cam_ch.cam_target_lun = ccbh->cam_target_lun;

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_ERRORS, cdbg_DumpCCBHeader(rccb));
	/*
	 * Issue the Release SIM Q CCB.
	 */
	xpt_action ((CCB_HEADER *) rccb); 

	/*
	 * CAM status must be complete for Release SIM Q CCB.
	 */
	if ( (rccb->cam_ch.cam_status&CSM) != CAM_REQ_CMP) {
	    PRINTD (ccbh->cam_path_id, ccbh->cam_target_id,
		    ccbh->cam_target_lun, CAMD_ERRORS,
		("[%d/%d/%d] scmn_ReleaseSIMQ: Release SIM Q CCB failed...\n",
		ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun));

	    CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
				CAMD_ERRORS, cdbg_DumpCCBHeader(rccb));
	    status = EIO;
	}
#if defined(KERNEL)
	(void) xpt_ccb_free ((CCB_HEADER *) rccb);
#else /* !defined(KERNEL) */
	free_palign ((char *) rccb);
#endif /* defined(KERNEL) */

	return (status);
}

#if defined(KERNEL)
/************************************************************************
 *									*
 * scmn_TerminateCCB() - Prepare And Queue A Terminate CCB.		*
 *									*
 * Description:								*
 *	This function handles allocating and filling in of a terminate	*
 * CCB request packet to abort an active I/O request.			*
 *									*
 * Inputs:	ccb = The CAM Control Block to terminate.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_TerminateCCB (ccb)
register CCB_SCSIIO *ccb;
{
	register CCB_HEADER *ccbh = (CCB_HEADER *) ccb;
	register CCB_TERMIO *tccb;
	int priority = PRIBIO;
	int status = SUCCESS;

	PRINTD (ccbh->cam_path_id, ccbh->cam_target_id,
		ccbh->cam_target_lun, CAMD_ERRORS,
	("[%d/%d/%d] scmn_TerminateCCB: Terminating CCB at 0x%lx\n",
	ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun, ccbh));

	/*
	 * Allocate a CCB to Terminate the I/O.
	 */
	tccb = (CCB_TERMIO *) xpt_ccb_alloc();

	/* 
	 * Setup the CCB header from SCSI I/O CCB.
	 */
	tccb->cam_ch.cam_ccb_len = (u_short) sizeof(*tccb);
	tccb->cam_ch.cam_func_code = (u_char) XPT_TERM_IO;
	tccb->cam_ch.cam_path_id = ccbh->cam_path_id;
	tccb->cam_ch.cam_target_id = ccbh->cam_target_id;
	tccb->cam_ch.cam_target_lun = ccbh->cam_target_lun;
	tccb->cam_termio_ch = (CCB_HEADER *) ccb;

	CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_ERRORS, cdbg_DumpTERMIO(tccb));
	/*
	 * Issue the Terminate I/O CCB.
	 */
	xpt_action ((CCB_HEADER *) tccb); 

	/*
	 * CAM status must be complete for Terminate I/O CCB.
	 */
	if ( (tccb->cam_ch.cam_status&CSM) != CAM_REQ_CMP) {
	    PRINTD (ccbh->cam_path_id, ccbh->cam_target_id,
		    ccbh->cam_target_lun, CAMD_ERRORS,
	("[%d/%d/%d] scmn_TerminateCCB: Terminate I/O CCB failed...\n",
	    ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun));

	    CALLD (ccbh->cam_path_id, ccbh->cam_target_id, ccbh->cam_target_lun,
					CAMD_ERRORS, cdbg_DumpTERMIO(tccb));
	    status = !SUCCESS;
	} else {
	    /*
	     * After issuing the Terminate CCB, wait for the CCB to complete.
	     */
	    status = ccmn_ccbwait (ccb, priority);
	}
	(void) xpt_ccb_free ((CCB_HEADER *) tccb);

	return (status);
}

/************************************************************************
 *									*
 * scmn_LockPages() - Check Access & Lock User Pages in Memory.		*
 *									*
 * Description:								*
 *	This function checks the user buffer access and if valid, locks	*
 * the user pages in memory. This allows direct I/O to the user buffer.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		user_buffer = The user buffer address.			*
 *		user_length = The user buffer length.			*
 *		data_direction = The data direction (IN or OUT).	*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_LockPages (sap, user_buffer, user_length, data_direction)
register struct special_args *sap;
caddr_t user_buffer;
int user_length;
int data_direction;
{
	/*
	 * Check read/write access and lock the pages.
	 */
	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
    ("[%d/%d/%d] scmn_LockPages: User buffer at 0x%lx with buffer length of %d.\n",
	sap->sa_bus, sap->sa_target, sap->sa_lun, user_buffer, user_length));

	if ((data_direction & SPC_DATA_INOUT) == 0) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
    ("[%d/%d/%d] scmn_LockPages: The I/O direction flags are NOT setup.\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun));
	    return (EINVAL);
	}

	/*
	 * Check access to user buffer pages:
	 *    o  Data direction IN, requires write access to pages.
	 *    o  Data direction OUT, requires read access from pages.
	 */
	if ( CAM_VM_USERACC(user_buffer, user_length,
	    (data_direction & SPC_DATA_IN) ? B_WRITE : B_READ) == 0) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
	("[%d/%d/%d] scmn_LockPages: User pages aren't accessible for %s...\n",
		sap->sa_bus, sap->sa_target, sap->sa_lun,
		(data_direction & SPC_DATA_IN) ? "Writing" : "Reading"));
	    return (EFAULT);
	}

	/*
	 * Lock the user buffer pages in memory.
	 */
	CAM_VM_LOCK(user_buffer, user_length);

	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_UnlockPages() - Unlock Previous Locked User Pages.		*
 *									*
 * Description:								*
 *	This function unlocks the user pages previously locked.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		user_buffer = The user buffer address.			*
 *		user_length = The user buffer length.			*
 *		data_direction = The data direction (IN or OUT).	*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_UnlockPages (sap, user_buffer, user_length, data_direction)
register struct special_args *sap;
caddr_t user_buffer;
int user_length;
int data_direction;
{
	/*
	 * Unlock the user buffer pages.
	 */
	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
    ("[%d/%d/%d] scmn_UnlockPages: User buffer at 0x%lx of %d (0x%x) bytes..\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				user_buffer, user_length, user_length));
	CAM_VM_UNLOCK(user_buffer, user_length,
		      (data_direction & SPC_DATA_IN) ? B_READ : B_WRITE);

	return (SUCCESS);
}
#endif /* defined(KERNEL) */

/************************************************************************
 *									*
 * scmn_SetupCCB() - Setup the CCB for this SCSI I/O Command.		*
 *									*
 * Description:								*
 *	This function handles the setup of the CAM Control Block fields	*
 * which includes the Command Descriptor Block (CDB) within the CCB.	*
 * The CDB is either setup from the prototype CDB defined in special	*
 * command block or via the make CDB function.				*
 *									*
 * Inputs:	sap = The special command argument pointer.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupCCB (sap)
struct special_args *sap;
{
	register CCB_SCSIIO *ccb = sap->sa_ccb;
	register CCB_HEADER *ccbh = (CCB_HEADER *)sap->sa_ccb;
	register struct special_cmd *spc = sap->sa_spc;
	register u_char *cdbp;
	int status = SUCCESS;
	u_char cmd;

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
	    ("[%d/%d/%d] scmn_SetupCCB: ENTER - sap = 0x%lx, ccb = 0x%lx\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun, sap, ccb));

	ccbh->cam_func_code = (u_char) XPT_SCSI_IO;
	ccbh->cam_flags |= spc->spc_cam_flags;
#if !defined(KERNEL)
	/*
	 * Or in the User enabled CAM CCB flags.
	 */
	ccbh->cam_flags |= ( CamFlags & CAM_CCB_FLAGS_MASK );
#endif /* defined(KERNEL) */
	ccbh->cam_ccb_len = sizeof(CCB_SCSIIO);
	ccbh->cam_path_id = sap->sa_bus;
	ccbh->cam_target_id = sap->sa_target;
	ccbh->cam_target_lun = sap->sa_lun;
	ccb->cam_data_ptr = (u_char *) sap->sa_user_buffer;
	ccb->cam_dxfer_len = (u_long) sap->sa_user_length;
	ccb->cam_sense_ptr = (u_char *) sap->sa_sense_buffer;

	/*
	 * If there isn't a sense length, then disable auto-sense.
	 */
	if ((ccb->cam_sense_len = sap->sa_sense_length) == 0) {
	    ccbh->cam_flags |= CAM_DIS_AUTOSENSE;
	}

	/*
	 * Setup the command timeout: user / command table / default.
	 */
	if ((ccb->cam_timeout = sap->sa_timeout) == 0) {
	    if ((ccb->cam_timeout = spc->spc_timeout) == 0) {
		ccb->cam_timeout = CAM_TIME_DEFAULT;
	    }
	}

	/*
	 * If the user buffer length is zero, then ensure the CAM data
	 * direction flags are set to 'no data'.  This check is necessary
	 * to handle commands which have optional data (for example, a
	 * Format Unit command with/without a defect list).
	 */
	if ( (sap->sa_user_buffer == (caddr_t) 0) &&
	     (sap->sa_user_length == 0) ) {
	    ccbh->cam_flags |= CAM_DIR_NONE;
	}

#if defined(KERNEL)
	/*
	 * A kernel I/O request buffer is needed if this command requires
	 * data movement by the DME and it's a user buffer address.
	 */
	if ( ( (sap->sa_cmd_flags & SPC_DATA_INOUT) &&
		CAM_IS_KUSEG(sap->sa_user_buffer) ) ||
	     ( (sap->sa_sense_length) &&
	       CAM_IS_KUSEG(sap->sa_sense_buffer) ) ) {
	    if ( (status = scmn_SetupRequest (sap)) != SUCCESS) {
		return (status);
	    }
	}
	ccb->cam_req_map = (u_char *) sap->sa_bp;

	/*
	 * If user didn't setup completion function, use ours.
	 */
	if (ccb->cam_cbfcnp == (void (*)()) 0) {
	    ccb->cam_cbfcnp = scmn_CompleteCCB;
	}
#endif /* defined(KERNEL) */

	/*
	 * Setup the Command Descriptor Block (CDB).
	 */
	if (ccbh->cam_flags & CAM_CDB_POINTER) {
	    cdbp = ccb->cam_cdb_io.cam_cdb_ptr;
	} else {
	    cdbp = (u_char *) ccb->cam_cdb_io.cam_cdb_bytes;
	}
	sap->sa_cdb_pointer = (caddr_t) cdbp;
	sap->sa_cdb_length = ccb->cam_cdb_len;

	/*
	 * Logic:
	 *   o	If there's a prototype CDB, then just copy it.
	 *   o	If there's a make CDB function, then call it.
	 *   o  Otherwise, just fill in the CDB command code.
	 */
	if (spc->spc_cdbp != (caddr_t) 0) {
	    ccbh->cam_flags |= CAM_CDB_POINTER;
	    cdbp = (u_char *) spc->spc_cdbp;
	    ccb->cam_cdb_io.cam_cdb_ptr = cdbp;
	    sap->sa_cdb_pointer = (caddr_t) cdbp;
	} else if (spc->spc_mkcdb != (int (*)()) 0) {
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_FLOW,
	("[%d/%d/%d] scmn_SetupCCB: Calling '%s' make CDB function @ 0x%lx...\n",
			sap->sa_bus, sap->sa_target, sap->sa_lun,
			spc->spc_cmdp, spc->spc_mkcdb));
	    status = (*spc->spc_mkcdb)(sap, cdbp);
	} else {
	    (u_char) cdbp[0] = spc->spc_cmd_code;
	}
	cmd = (u_char) cdbp[0];
#ifdef notdef
	(u_char) cdbp[1] |= (sap->sa_lun << 5);
#endif notdef
	if (sap->sa_cdb_length == 0) {
	    sap->sa_cdb_length = (u_char) scmn_cdb_length (cmd);
	}

	/*
	 * If the CDB length exceeds the space allocated in the SCSIIO
	 * CCB, then a pointer to the CDB should have been setup.
	 */
	if ( (status == SUCCESS) &&
	     ((ccb->cam_cdb_len = sap->sa_cdb_length) > IOCDBLEN) ) {
	    if ((ccbh->cam_flags & CAM_CDB_POINTER) == 0) {
		printf
  ("[%d/%d/%d] scmn_SetupCCB: CDB length (%d) > IOCDBLEN (%d) w/o CAM_CDB_POINTER set.\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				sap->sa_cdb_length, IOCDBLEN);
		status = EINVAL;
	    }
	}

#if defined(KERNEL)
	/*
	 * If there's a pointer to the CDB, it MUST be a kernel address.
	 */
	if ( (status == SUCCESS) &&
	     (ccbh->cam_flags & CAM_CDB_POINTER) &&
	     (CAM_IS_KUSEG((caddr_t)ccb->cam_cdb_io.cam_cdb_ptr)) ) {
	    printf
  ("[%d/%d/%d] scmn_SetupCCB: CDB pointer of 0x%lx is NOT kernel address.\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				ccb->cam_cdb_io.cam_cdb_ptr);
	    status = EINVAL;
	}
#endif /* defined(KERNEL) */

	CALLD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
						cdbg_DumpSCSIIO(ccb));

	PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_INOUT,
    ("[%d/%d/%d] scmn_SetupCCB: EXIT - cmd = 0x%x (%s), cdb length = %d\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				cmd, spc->spc_cmdp, sap->sa_cdb_length));
	return (status);
}

#if defined(KERNEL)
/************************************************************************
 *									*
 * scmn_SetupRequest() - Setup the Buffer Request for this I/O.		*
 *									*
 * Description:								*
 *	This function handles the setup of an I/O request buffer.  Any	*
 * command which involves data movement to a user data of sense buffer,	*
 * must have a kernel request buffer allocated for it since the process	*
 * structure (pointed to from within the bp) is required to double map	*
 * the user address.							*
 *									*
 * NOTE:	Since the user structure is accessed, this function	*
 *		must be called in process context.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupRequest (sap)
register struct special_args *sap;
{
	register struct buf *bp;
	int rw = (sap->sa_cmd_flags & SPC_DATA_OUT) ? B_WRITE : B_READ;

	if ((bp = sap->sa_bp) == (struct buf *) 0) {
	    bp = ccmn_get_bp();
	    if ((sap->sa_bp = bp) == (struct buf *) 0) {
		return (ENOMEM);
	    }
	    sap->sa_flags |= SA_ALLOCATED_BP;
	    PRINTD (sap->sa_bus, sap->sa_target, sap->sa_lun, CAMD_CMD_EXP,
  ("[%d/%d/%d] scmn_SetupRequest: Allocated I/O request buffer at 0x%lx of %d bytes.\n",
				sap->sa_bus, sap->sa_target, sap->sa_lun,
				bp, sizeof(*bp)));
	}
	bp->b_flags = (B_BUSY | rw);
	bp->b_dev = sap->sa_dev;
	bp->b_bcount = sap->sa_user_length;
	bp->b_un.b_addr = sap->sa_user_buffer;
	bp->b_blkno = 0;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	if (sap->sa_user_buffer && CAM_IS_KUSEG(sap->sa_user_buffer)) {
	    bp->b_flags |= B_PHYS;
	    SET_P_VM(u.u_procp, SPHYSIO);
	}
	return (SUCCESS);
}
#endif /* defined(KERNEL) */

/************************************************************************
 *									*
 * scmn_cdb_length() - Calculate the Command Descriptor Block length.	*
 *									*
 * Description:								*
 *	This function is used to determine the SCSI CDB length.  This	*
 * is done by checking the command group code.  The command specified	*
 * is expected to be the actual SCSI command byte, not a psuedo command	*
 * byte.  There should be tables for vendor specific commands, since	*
 * there is no way of determining the length of these commands.		*
 *									*
 * Inputs:	cmd_code = The SCSI command code.			*
 *									*
 * Return Value:							*
 *		Returns the CDB length.					*
 *									*
 ************************************************************************/
int
scmn_cdb_length (cmd_code)
register u_char cmd_code;
{
	register int cdb_length = 0;

	/*
	 * Calculate the size of the SCSI command.
	 */
	switch (cmd_code & SCSI_GROUP_MASK) {

	    case SCSI_GROUP_0:
		cdb_length = 6;			/* 6 byte CDB. */
		break;

	    case SCSI_GROUP_1:
	    case SCSI_GROUP_2:
		cdb_length = 10;		/* 10 byte CDB. */
		break;

	    case SCSI_GROUP_5:
		cdb_length = 12;		/* 12 byte CDB. */
		break;

	    case SCSI_GROUP_3:
	    case SCSI_GROUP_4:
		cdb_length = 6;			/* Reserved group. */
		break;

	    case SCSI_GROUP_6:
	    case SCSI_GROUP_7:
		cdb_length = 10;		/* Vendor unique. */
		break;
	}
	return (cdb_length);
}

#if defined(KERNEL)
/************************************************************************
 *									*
 * scmn_PagesLocked() - Check Virtual Address Range for Locked Pages.	*
 *									*
 * Description:								*
 *	This functions' purpose is to ensure no pages are locked prior	*
 * to attemping to lock them.  The kernel vslock() function will put	*
 * the calling process to sleep if a page in the virtual address range	*
 * specified is already locked.  This will create a deadlock situation	*
 * if the process specified multiple buffers within the same page in an	*
 * I/O control command structure (i.e., data buffer & sense buffer).	*
 *									*
 * Inputs:	base = The virtual base address.			*
 *		length = The address length to check.			*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS,					*
 *			or						*
 *		the error code EDEADLK if deadlock would occur.		*
 *									*
 ************************************************************************/
int
scmn_PagesLocked (base, length)
caddr_t base;
int length;
{
	register unsigned v;
	register int npf;
	register struct pte *pte = 0;
	register struct cmap *c;
	register struct proc *p = u.u_procp;
	int s, status = SUCCESS;

	PRINTD (NOBTL, NOBTL, NOBTL, (CAMD_INOUT | CAMD_FLOW),
	("[b/t/l] scmn_PagesLocked: Checking address at 0x%lx of length %d.\n",
							base, length));
	v = btop (base);
	npf = btoc (length + ((int)base & CLOFSET));
	s = splimp();
	smp_lock (&lk_cmap, LK_RETRY);
	while (npf > 0) {
	    if ( ( ((int)pte & PGOFSET) < CLSIZE*sizeof(struct pte) ) ||
		 ( (pte->pg_pfnum == 0) && (pte->pg_v == 0) ) ) {
		pte = vtopte(p, v);	/* Handles crossing segments. */
	    }
	    /*
	     * Only valid pages can be locked.
	     */
	    if (pte->pg_v) {
		c = &cmap[pgtocm(pte->pg_pfnum)];
		if (MISLOCKED(c)) {
		    status = EDEADLK;	/* Show would cause dead-lock. */
		    break;
		}
	    }
	    pte += CLSIZE;
	    v += CLSIZE;
	    npf -= CLSIZE;
	}
	smp_unlock (&lk_cmap);
	(void) splx (s);

	PRINTD (NOBTL, NOBTL, NOBTL, (CAMD_INOUT | CAMD_FLOW),
	("[b/t/l] scmn_PagesLocked: EXIT - status = 0x%x\n", status));

	return (status);
}
#endif /* defined(KERNEL) */
