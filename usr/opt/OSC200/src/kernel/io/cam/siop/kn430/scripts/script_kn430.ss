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

/* 		THIS IS THE COBRA NCR 53C710 SCRIPT 		*/
/*
 * This SCRIPT is downloaded for each SIOP on the I/O board.
 * It can be no longer than 26214 (0x6666) bytes in length (1/5 RAM).
 * As many address as possible should be relative.
 * All other addresses must be relocated before the SCRIPT is downloaded
 * to the SCRIPTS RAM.
 * This file must be passed through the C preprocessor before it is
 * passed to the SCRIPTS Compiler.
 */

/* The layout of SCRIPTS RAM is that the RAM is split into 5 equal size
 * portions, each 26214 bytes in length.  Each SIOP uses it's own
 * portion of the RAM.
 * The addresses for each SIOP are:
 *		SIOP 0 		0x0 - 0x65ff
 *		SIOP 1		0x6600 - 0xcbff
 *		SIOP 2		0xcc00 - 0x131ff
 *		SIOP 3		0x13200 - 0x197ff
 *		SIOP 4		0x19800 - 0x1fdff
 *		BOOT LOADER	0x1ffe0 - 0x1ffff
 *
 * Each area of memory is organized as follows:
 *		0:		main jump vector for cold start
 *		8:		main entry for warm start
 *		16:		parity error jump vector
 *		24:		data phase mismatch restart vector
 *		32:		msg phase mismatch restart vector
 *		40:		general entry point to abort a request
 *		48:		entry point to continue processing a job
 *			(add more jumps here)
 *		0xNN		SCRIPTS CODE
 */

/* since this must go through the preprocessor, #includes are permitted
 * as are #defines and macros (I modified the SCRIPTS compiler to
 * accept ';' and the end of a statement).
 */

#define _INSCRIPTS_

#include "scripthost.h"
#include "siopdefs.h"
#include "scriptram_kn430.h"
#include "siopdefs_kn430.h"
#include "scsi_phases.h"

#define ENABLE_SS	move DCNTL | 0x10 to DCNTL
#define DISABLE_SS	move DCNTL & ~0x10 to DCNTL

/* Define SIOP_BURST if the SCRIPTS should try to do burst transfers
 * on the local bus.
 */
#define SIOP_BURST


#ifdef SIOP_BURST

/* Enable burst transfers.  Only a 16 byte threshold seems to work.
 * This may not be a problem since the chip will only perform a 16
 * byte transfer per bus ownership anyway.
 */

#define BURST_ON 	move DMODE | (DMODE_BL1) to DMODE

#define BURST_OFF 	move DMODE & ~(DMODE_BL1|DMODE_BL0) to DMODE


/* copy data between SCRIPTS RAM and host memory with the maximum workable
 * burst size (currently 8 bytes on Cobra).
 */

#define copyout(F,T,S) \
	move memory SIZEOF_U32, T, pc+28 ; \
	move DMODE | (DMODE_BL0) to DMODE ; \
	move memory (S), F, 0 ; \
	move DMODE & ~(DMODE_BL0|DMODE_BL1) to DMODE

#define copyin(F,T,S) \
	move memory SIZEOF_U32, F, pc+24 ; \
	move DMODE | (DMODE_BL0) to DMODE ; \
	move memory (S), 0, T ; \
	move DMODE & ~(DMODE_BL0|DMODE_BL1) to DMODE

#else	/* SIOP_BURST */

#define BURST_ON
#define BURST_OFF

#define copyout(F,T,S) \
	move memory SIZEOF_U32, T, pc+20 ; \
	move memory (S), F, 0 

#define copyin(F,T,S) \
	move memory SIZEOF_U32, F, pc+16 ; \
	move memory (S), 0, T 
#endif /* SIOP_BURST */


/* Entry points:  The host starts SCRIPTS execution at one of the
 * following addresses.  These are known to the host.
 */

ORIGIN	0x80000000
start:
	jump REL (main)
	jump REL (warm_start)
	jump REL (parity_error)
	jump REL (data_phase_mismatch)
	jump REL (msg_phase_mismatch)
	jump REL (do_abort)
	jump REL (mainloop)

/* put any more entry points here */

		/* main part of SCRIPT code */

/* The main entry point simply clears out the current job status and
 * performs all processing to enter the IDLE state.
 * This is the main REENTRY point for the SCRIPTS so it must not
 * destroy any global information.
 */
main:
	BURST_OFF
	/* first, clear out the current job field */
	move 0 to SFBR
	move SFBR to SCRATCH0
	move SFBR to SCRATCH1
	move SFBR to SCRATCH2
	move SFBR to SCRATCH3
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, zero
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, zero+SIZEOF_U32
	move memory SIZEOF_U64, zero, zero+SIZEOF_U64
	move memory 16, zero, zero+16
	move memory 32, zero, zero+32
	move memory SIZEOF_U64, zero, scsijobptr

	/* clear out the job table */
	move memory 64, zero, SCRIPT_RAM_JOBTABLE
	move memory 64, zero, SCRIPT_RAM_JOBTABLE+64
	move memory 64, zero, SCRIPT_RAM_JOBTABLE+128
	move memory 64, zero, SCRIPT_RAM_JOBTABLE+192

	/* clear out the tag cache */
	move memory 64, zero, SCRIPT_RAM_TAG_CACHE
	move memory 64, zero, SCRIPT_RAM_TAG_CACHE+64
	move memory 128, SCRIPT_RAM_TAG_CACHE, SCRIPT_RAM_TAG_CACHE+128
	move memory 256, SCRIPT_RAM_TAG_CACHE, SCRIPT_RAM_TAG_CACHE+256
	move memory 512, SCRIPT_RAM_TAG_CACHE, SCRIPT_RAM_TAG_CACHE+512
	move memory 1024, SCRIPT_RAM_TAG_CACHE, SCRIPT_RAM_TAG_CACHE+1024
	move memory 2048, SCRIPT_RAM_TAG_CACHE, SCRIPT_RAM_TAG_CACHE+2048
	move memory 4096, SCRIPT_RAM_TAG_CACHE, SCRIPT_RAM_TAG_CACHE+4096

	/* now clear the host's idea of the current job */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_CURRENTJOB to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U64, zero, 0

	/* now set up the move instruction for the setdsa subroutine */
	move memory SIZEOF_U32, siopjob_datai, SSIOP_REG_SCRATCH0
	move memory SIZEOF_U32, pc-8, SSIOP_REG_SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, setdsa+9
	move SCRATCH2 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, setdsa+17

	/* clear out the transfer parameters table */
	move memory 64, zero, SCRIPT_RAM_TDTABLE

	/* check the version number of the scripthost structure to sanity */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_VERSION to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, temp
	move memory SIZEOF_U8, temp, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (warm_start), if SIOP_SH_VERSION

	/* if the version number is not sane then kill the 710.
	 * Just have the 710 reset it's self.
	 */
	move ISTAT_RST to ISTAT

	/* NOT REACHED */

warm_start:
	BURST_OFF
	/* delay a bit */
	move 250 to SFBR
	call rel (delay)

goidle:
	/* set Don't PASS parity mode (rev 1+ chips only) */
	move 0x20 to CTEST0	
	/* clear the FIFO's */
	move CTEST8 | CTEST8_CLF to CTEST8

	/* clear out the SIGP bit */
	move CTEST2 TO SFBR	/* the read resets the SIGP bit */

	/* test jobrequest to see if it's 0.
	 *
	 * 	if(SCRIPT_RAM_SH->sh_jobrequest)
	 *	    goto newjob
	 *	else
	 *	{
	 *	    SCRIPT_RAM_SH->sh_intstatus = GOING_IDLE;
	 *	    int GOING_IDLE;
	 *	}
	 */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_JOBREQUEST to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, scsijobptr

	move memory SIZEOF_U32, scsijobptr, SSIOP_REG_SCRATCH0
	call REL (is_zero_32)
	jump REL (newjob), if not 0

	/* now do a GOING_IDLE interrupt */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+28
	move GOING_IDLE to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	int GOING_IDLE

	/* When we get here the host has finished processing the 
  	 * GOING_IDLE and has returned some value in jobrequest.
 	 */

isnewjob:
	/* test jobrequest to see if it's 0.
	 *
	 *	if(SCRIPT_RAM_SH->sh_jobrequest == 0)
	 *		goto idle;
	 */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_JOBREQUEST to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, scsijobptr

	move memory SIZEOF_U32, scsijobptr, SSIOP_REG_SCRATCH0
	call REL (is_zero_32)
	jump REL (idle), if 0

	/* we got a new job to start up, it's address is in SCRIPT_RAM_JOB */
	/*
	 *	scsijobptr = SCRIPT_RAM_SH->sh_jobrequest;
	 *	memcpy(*scsijobptr,siopjob);
	 *	SCRIPT_RAM_SH->sh_currentjob = siopjob.sj_me;
	 *	if(siopjob.sj_version != SIOP_SJ_VERSION)
	 *		goto finishjob;
	 */
newjob:
	/* first get the scsijob structure from the host */
	copyin(scsijobptr,siopjob,SJ_COPYIN_SIZE)

	/* now set up the current job reference in the scripthost struct */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_CURRENTJOB to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U64, siopjob_me, 0

	/* check the structure version number for sanity */
	move memory SIZEOF_U8, siopjob_version, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (abortcheck), if SIOP_SJ_VERSION
	move CAM_REQ_INVALID to SFBR
	jump rel (finishjob)

abortcheck:
	/* if the term or abort flags are set just return the job with
	 * error.
	 *
	 *	if(siopjob.sj_term)
	 *		goto finishjob;
	 */
	move memory SIZEOF_U8, siopjob_term, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (abortcheck1), if 0
	move CAM_REQ_ABORTED to SFBR
	jump rel (finishjob)

	/*
	 * if(siopjob.sj_abort)
	 *	goto finishjob;
	 */
abortcheck1:
	move memory SIZEOF_U8, siopjob_abort, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (set_tid), if 0
	move CAM_REQ_ABORTED to SFBR
	jump rel (finishjob)

set_tid:
	/* Set up the information needed by the SELECT instruction */
	/* WARNING: WATCH THE OFFSETS ON THE FOLLOWING MOVES */
	/*
	 * siopjob.tid |= (SCRIPT_RAM_TABLE[siopjob.tid]&0xff00)
	 */
	move memory SIZEOF_U8, siopjob_tid+2, SSIOP_REG_SCRATCH2
	move SCRATCH2 to SFBR
	jump REL (pc+20), if not 1
	move memory SIZEOF_U8, SCRIPT_RAM_TDTABLE+1, siopjob_tid+1
	jump REL (pc+20), if not 2
	move memory SIZEOF_U8, SCRIPT_RAM_TDTABLE+5, siopjob_tid+1
	jump REL (pc+20), if not 4
	move memory SIZEOF_U8, SCRIPT_RAM_TDTABLE+9, siopjob_tid+1
	jump REL (pc+20), if not 8
	move memory SIZEOF_U8, SCRIPT_RAM_TDTABLE+13, siopjob_tid+1
	jump REL (pc+20), if not 0x10
	move memory SIZEOF_U8, SCRIPT_RAM_TDTABLE+17, siopjob_tid+1
	jump REL (pc+20), if not 0x20
	move memory SIZEOF_U8, SCRIPT_RAM_TDTABLE+21, siopjob_tid+1
	jump REL (pc+20), if not 0x40
	move memory SIZEOF_U8, SCRIPT_RAM_TDTABLE+25, siopjob_tid+1
	jump REL (pc+20), if not 0x80
	move memory SIZEOF_U8, SCRIPT_RAM_TDTABLE+29, siopjob_tid+1
	jump REL (loadtid), if not 0

	/* if we get here then we have an error:  illegal TID in request.
	 * Send back an Invalid Path ID status.
	 */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+20
	move memory SH_OFFSET_CURRENTJOB, zero, 0

	move CAM_PATH_INVALID to SFBR
	jump REL (finishjob)

loadtid:
	move memory SIZEOF_U32, zero, errcount

try_select:
	/* now do attempt a SELECTION */
	call rel (setdsa)
	select atn from (SJ_OFFSET_TID-SJ_COPYOUT_SIZE), REL (collision)

	/* At this point the chip has started the selections, but
	 * has not waited for BUSY to get asserted by the target.
	 * To allow for slow devices (such as the SDS-3 with the ZADIAN
	 * Host Basher Software) a timeout of more than 250 msec should
	 * be implemented.  This should not be too long however.
	 * BTW, the 250 msec timeout specified in the spec is a MINIMUM
	 * timeout.
	 *
	 *    	for(SCRATCH0 = 255; SCRACTH0 ; SCRATCH0--)
	 *	    if(SBCL & SBCL_BUSY)
	 *		goto isbusy;
	 */
	move 255 to SCRATCH0
busy_wait:
	move 255 to SFBR
	call rel (delay)
	move SBCL & SBCL_BSY to SFBR
	jump rel (isbusy), if not 0
	move SCRATCH0 to SFBR
	move SFBR + 255 to SCRATCH0
	jump rel (busy_wait), if not 0

	/* The target has not yet asserted BUSY, so start the chip's
	 * selection timeout so the host gets an STO in another 250 msec.
	 * Of course, if the target responds within this period then
	 * everything will be OK.
	 */
	move CTEST7 & 0xef  to CTEST7

	/* The 710 does not handle the case in which the target 
	 * disconnects during a selection.  No UDC interrupt is
	 * generated.  Thus, the following loop spins to wait
	 * for the transition to the MSG_OUT phase (this MUST
	 * be the next phase since ATN was asserted on the selection)
	 * or for the target to disconnect.
	 * When the target disconnects the connection will be retried
	 * a few more times before the request is returned to the
	 * host with an error.
	 *
	 *	while(SBCL & SBCL_BSY)
	 *	    if(MSG_OUT)
	 *		goto jobstarted;
	 *	goto sel_discon;
	 */
isbusy:
	move SBCL & SBCL_BSY to SFBR
	jump rel (sel_discon), if 0
	jump rel (jobstarted), if MSG_OUT	/* synchronize */
	jump rel (isbusy)

	/*  Target disconnected after selection, but before first
	 * phase change.  Since the chip won't generate a UDC for
	 * this condition, it must be handled manually.
	 *
	 *	if(errcount++ != 2)
	 *	    goto try_select;
	 *	SCRIPT_RAM_SH->sh_jobrequest = 0;
	 *	finishjob(CAM_UNEXP_BUSFREE);
	 */
sel_discon:
	move memory SIZEOF_U32, errcount, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	move SFBR + 1 to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, errcount
	jump rel (try_select), if not 2

	/* The target keeps disconnecting during selection.  Fail the
	 * request. Clear out the jobrequest field first.
	 */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_JOBREQUEST to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U32, zero, 0

	move CAM_UNEXP_BUSFREE to SFBR
	jump rel (finishjob)

jobstarted:
	/* set up a bad termination status in case the status byte is
	 * not received.
	 *
	 *	status_received = CAM_REQ_CMP_ERR;
	 *	temp = 0;
	 *	SCRIPT_RAM_SH->sh_jobrequest = 0;
	 */
	move CAM_REQ_CMP_ERR to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, status_received

	move memory 4, zero, temp
	/* disable timeout interrupts once selected */
	move CTEST7 | 0x10 to CTEST7

	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_JOBREQUEST to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U32, zero, 0

	/* enter the job in the job table (for reconnections).
	 * This is only done for untagged requests since there is
	 * not enough local memory to hold all tagged requests and
	 * insufficient SIOP compute power to hash/lookup/etc.
	 * The host must handle the tagged requests.
	 *
	 *	if(siopjob.tagged == 0)
	 *	    SCRIPT_RAM_JOBTABLE[siopjob.lun * 4] = scsijobptr;
	 */
	move memory SIZEOF_U32, siopjob_tagged, SSIOP_REG_SCRATCH0
	call rel (is_zero_32)
	jump rel (setup_tag_cache), if not 0

	move memory SIZEOF_U32, siopjob_lun, SSIOP_REG_SCRATCH0

	/* get the word offset into the job table (multiply by 4)  */
	move SCRATCH0 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH0 + 0 to SCRATCH0
	move SCRATCH0 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH0 + 0 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U32, scsijobptr, SCRIPT_RAM_JOBTABLE
	jump rel (setup_pointers)

	/* place the entry in the tag cache.  If there is already info
	 * there then don't bother.  This will prevent the possible situation
	 * where each new requests overwrites the old cache line before
	 * the old job reselects, causing cache misses.  At least this
	 * will assure that at least one reconnecting request is in
	 * the cache.
	 *
	 *	else if(*siopjob.tagged == 0)
	 *	    *siopjob.tagged = scsijobptr | (siopjob.lun & 7);
	 */	    
setup_tag_cache:
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, SSIOP_REG_SCRATCH0
	call rel (is_zero_32)
	jump rel (setup_pointers), if not 0

	/* Set the cache line tag to the LUN.
	 * This is the lower three bits of the cache line entry.
	 */
	move memory SIZEOF_U8, siopjob_lun, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, scsijobptr, SSIOP_REG_SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH0 | 0 to SCRATCH0
	move SCRATCH0 to SFBR

	/* move the cache line entry to the cache  */
	move memory SIZEOF_U32, scsijobptr, SSIOP_REG_SCRATCH0
	move SFBR to SCRATCH0
	move memory SIZEOF_U32, siopjob_tagged, pc+20
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, 0

setup_pointers:
	/* set up the current MSGOUT pointer.
	 *
	 *	siopjob.msgoptr = siopjob.smsgoptr;
	 *	current_datai = siopjob.sdatai;
	 *	cirrent_datap = 0;
	 *	current_offset = 0;
	 */
	move memory SIZEOF_DATAI, siopjob_smsgoptr, siopjob_msgoptr

	/* set up the current data pointer */
	move memory SIZEOF_DATAI, siopjob_ldatai, current_datai
	move memory SIZEOF_U32, zero, current_datap
	move memory SIZEOF_U32, zero, current_offset


	/* Main loop.  Just test for different phases and take the
	 * appropriate action.
	 *
	 *	switch(scsi phase)
	 */
mainloop:
	BURST_OFF
	jump rel (msgout), when MSG_OUT
	jump rel (msgin), if MSG_IN
	jump rel (dataio), if DATA_OUT
	jump rel (dataio), if DATA_IN
	jump rel (scb), if CMD
	jump rel (ssi), if STATUS


	/* Handle all incomming messages from the target.
	 * First, read the first byte of the message to figure
	 * out what the message is.  If it is one that is
	 * not handled by the initiator then this allows ATN
	 * to be asserted immediately.
	 * Otherwise, determine the message type and jump to
	 * the appropriate handler.
	 *
	 *	msg_parity = msg_mpe;
	 *	receive on message byte
	 *	switch(message byte)
	 */
msgin:
	move memory SIZEOF_U8, msg_mpe, msg_parity
	move SIZEOF_U8, msgibuf, when MSG_IN
check_msg_type:
	/* take action on message by testing SFBR */
	jump rel (do_reselect), if 0x80, and mask 0x7f
	jump rel (got_disconnect), if SCSI_DISCONNECT
	jump rel (reject_msg), if SCSI_ABORT
	jump rel (save_ptr), if SCSI_SAVE_DATA_POINTER
	jump rel (restore_ptr), if SCSI_RESTORE_POINTERS
	jump rel (reject_msg), if SCSI_ABORT_TAG
	jump rel (reject_msg), if SCSI_INITIATOR_DETECTED_ERROR
	jump rel (reject_msg), if SCSI_MESSAGE_PARITY_ERROR
	jump rel (reject_msg), if SCSI_LINKED_COMMAND_COMPLETE
	jump rel (reject_msg), if SCSI_LINKED_COMMAND_COMPLETE_WFLAG
	jump rel (reject_msg), if SCSI_BUS_DEVICE_RESET
	jump rel (reject_msg), if SCSI_CLEAR_QUEUE
	jump rel (reject_msg), if SCSI_INITIATE_RECOVERY
	jump rel (reject_msg), if SCSI_RELEASE_RECOVERY
	jump rel (reject_msg), if SCSI_TERMINATE_IO_PROCESS
	jump rel (gottag), if SCSI_SIMPLE_QUEUE_TAG
	jump rel (reject_msg), if SCSI_HEAD_OF_QUEUE_TAG
	jump rel (reject_msg), if SCSI_ORDERED_QUEUE_TAG
	jump rel (reject_msg), if SCSI_IGNORE_WIDE_RESIDUE
	jump rel (got_reject), if SCSI_MESSAGE_REJECT
	jump rel (jobdone), if SCSI_COMMAND_COMPLETE
	jump rel (extended), if SCSI_EXTENDED_MESSAGE
	jump rel (noop), if SCSI_NO_OPERATION
	jump rel (reject_msg)

noop:
	clear ack
	jump rel (mainloop)

	/* Send as many of the outgoing bytes as the target wants.
	 * If the target is requesting bytes and the host has none
	 * to send then just send NOPs.
	 *
	 *	if(siopjob.sj_msgoptr.di_count == 0)
	 *	    send NOP
	 *	else
	 *	{
	 *	    setdsa();
	 *	    send message bytes
	 *	}
	 */
msgout:
	move memory SIZEOF_U32, siopjob_msgoptr, SSIOP_REG_SCRATCH0
	move 0 to SCRATCH3
	call rel (is_zero_32)
	jump rel (send_nop), if 0

	call rel (setdsa)
	move from (SJ_OFFSET_MSGOPTR-SJ_COPYOUT_SIZE), when MSG_OUT
	jump rel (mainloop), when not MSG_OUT

send_nop:
	clear atn
	move SIZEOF_U8, msgnop, when MSG_OUT
	jump rel (mainloop), when not MSG_OUT

	/*  There was a phase mismatch on message transmission.
	 *  The host processed the M/A interrupt and reset the
	 *  msgoptr.  Copy it back in and continue processing the request.
	 *
	 *	siopjob.sj_msgoptr = scsijob->sj_msgoptr;
	 *	goto mainloop;
	 */	
msg_phase_mismatch:
	move memory SIZEOF_U32, scsijobptr, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SJ_OFFSET_MSGOPTR to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_DATAI, 0, siopjob_msgoptr

	jump rel (mainloop)

	/* Get/send data to/from target.
	 *
	 * 	siopjob.ldatai = current_datai;
	 *	siopjob.datap = current_datap;
	 *	siopjob.offset = current_offset;
	 *	if(siopjob.sj_datai[current_datap].di_count == 0)
	 *	    goto overflow;
	 *	msg_parity = msg_ide;
	 *	setdsa();
	 *	DSA += current_datap;
	 *	if(DATA_IN)
	 *	    datain();
	 *	else if(DATA_OUT)
	 *	    dataout();
	 *	if((++current_datap%SIOP_DMA_WINDOW_SIZE) == 0)
	 *	    goto datawrap;
	 *	setdsa();
	 *	current_datai = siopjob.sj_datai[current_datap];
	 *	current_offset = siopjob.sj_offset[current_datap];
	 *	siopjob.sj_datai = current_datai;
	 *	siopjob.sj_datap = current_datap;
	 *	siopjob.sj_offset = current_offset;
	 *	if(command phase)
	 *	    goto do_abort;
	 *	goto mainloop;
	 */
dataio:

	/* set the host's idea of the current data pointer in case
	 * a phase mismatch interrupt occurrs.
	 */
	move memory SIZEOF_DATAI, current_datai, siopjob_ldatai
	move memory SIZEOF_U32, current_datap, siopjob_ldatap
	move memory SIZEOF_U32, current_offset, siopjob_loffset

	move memory SIZEOF_U8, current_datap, SSIOP_REG_SCRATCH0

	/* check the transfer count to make sure it's non-zero */
	call rel (setdsa)
	move DSA1 to SFBR
	move SFBR to SCRATCH1
	move DSA2 to SFBR
	move SFBR to SCRATCH2
	move 0x80 to SCRATCH3
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, SSIOP_REG_SCRATCH0

	call rel (is_zero_32)
	jump rel (overflow), if 0

	move memory SIZEOF_U8, current_datap, SSIOP_REG_SCRATCH0

	move memory SIZEOF_U8, msg_ide, msg_parity

	/* load the DSA register with the base of the SCSIJOB struct */
	call rel (setdsa)

	/* "add" the offset to the current datai elemement.  This
	 * is really fast and easy, but restricts the size of the datai
	 * array to 32 entries.
	 */
	move SCRATCH0 to SFBR
	move SFBR to DSA0

	BURST_ON
	call rel (datain), if DATA_IN
	call rel (dataout), if DATA_OUT
	BURST_OFF

	/* check for wraparound of the current datap. */
	move memory SIZEOF_U32, current_datap, SSIOP_REG_SCRATCH0
	move SCRATCH0 + 8 to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, current_datap
	move SCRATCH0 to SFBR
	jump rel (datawrap), if 0

	/* make the next datai element the current one.
	 * This assures that is a phase change occurs at this point
	 * the currect information will be in the ldatai struct
	 * to properly save/restore the data pointer.
	 */
	call rel (setdsa)
	move DSA1 to SFBR
	move SFBR to SCRATCH1
	move DSA2 to SFBR
	move SFBR to SCRATCH2
	move 0x80 to SCRATCH3
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_DATAI, 0, current_datai

	/* and the current offset */
	move SCRATCH1 + 1 to SCRATCH1
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U64, 0, current_offset

	move memory SIZEOF_DATAI, current_datai, siopjob_ldatai
	move memory SIZEOF_U32, current_datap, siopjob_ldatap
	move memory SIZEOF_U32, current_offset, siopjob_loffset

	jump rel (do_abort), when CMD
	jump rel (mainloop)

	/* Ok, all the data mapped in the datai array has been transferred.
	 * We must ask the host to remap so more data can be moved.
	 * This is done via an RPC.
	 * This has the unfortuneate side effect of causing an extra
	 * interrupts on requests that have exactly 32 entries in
	 * the scatter/gather list.
	 *
	 *	siopjob.sj_sdatai.mbz = 0xff;
	 *	copyout(&siopjob,scsijobptr,SJ_COPYOUT_SIZE);
	 *	SCRIPT_RAM_SH->sh_intstatus = RPC_REQUEST;
	 *	SCRIPT_RAM_SH->sh_rpcreq = RPC_MORE_DATA;
	 *	int RPC_REQUEST
	 *	
	 *	copyin(scsijobptr,&siopjob,SJ_COPYIN_SIZE);
	 *	current_datai = siopjob.sj_ldatai;
	 *	current_offset = siopjob.sj_loffset;
	 *	current_datap = siopjob.sj_datap;
	 *	if(command phase)
	 *	    goto do_abort;
	 *	goto mainloop;
	 */
datawrap:
	/* mark the saved data pointer as invalid since the following RPC
	 * will result in the DMA window moving.
	 */
	move 0xff to SCRATCH3
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH3, siopjob_sdatai+3

	/* First, the siopjob structure must be copied out to maintain 
	 * any values changed by the SCRIPTS.  This is not too expensive
	 * since only a small part of the structure gets copied out.
	 */
	copyout(siopjob,scsijobptr,SJ_COPYOUT_SIZE)

	/* set the interrupt status */
        move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+28
        move RPC_REQUEST to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0

	/* set the RPC request */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREQ to SCRATCH0
        move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+28
        move RPC_MORE_DATA to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	int RPC_REQUEST

	/* don't really need to check the result since an error
	 * only exists when the target tries to use a null entry
	 * not when one is received.
	 */

	/* now transfer in the new scsijob structure */
	copyin(scsijobptr,siopjob,SJ_COPYIN_SIZE)

	move memory SIZEOF_DATAI, siopjob_ldatai, current_datai
	move memory SIZEOF_U64, siopjob_loffset, current_offset
	move memory SIZEOF_U64, siopjob_ldatap, current_datap

	jump rel (do_abort), when CMD
	jump rel (mainloop)

	/* a SCSI bus phase sequence failure */
bad_seq:
	move CAM_SEQUENCE_FAIL to SFBR
	jump rel (abort_job)

	/* The target has attempted to transfer more data than the
	 * host is prepared to receive.  In this case the command
	 * should be aborted and any further data tossed.  Data
	 * trasfer phases must be services since the target will
	 * respond to the ATN when it ready, and not before.
	 * If the case of a read the host will get the requested
	 * data, but should be informed that the read overflowed.
	 * On writes, the abort should be sent to prevent media
	 * corruption.
	 *
	 *	siopjob.sj_err = SFBR;
	 *	copyout(&siopjob,*scsijobptr,SJ_COPYOUT_SIZE);
	 *	if(siopjob.sj_tagged)
	 *	    badselloop(SIOP_KILL_AT);
	 *	else
	 *	    badselloop(SIOP_KILL_ABORT);
	 */
overflow:
	move CAM_DATA_RUN_ERR to SFBR

abort_job:
	/* set the error status */
	move SFBR to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, siopjob_err

	/* now copy out the job */
	copyout(siopjob,scsijobptr,SJ_COPYOUT_SIZE)

	/* a JOB_DONE RPC isn't required since th job will be picked
	 * up as the result of the upcoming UDC or RESET interrupt.
	 * The job must be the current job.
	 */

	move memory SIZEOF_U8, siopjob_tagged, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	call rel (is_zero_32)
	jump rel (abort_tagged), if not 0

	move SIOP_KILL_ABORT to SCRATCH0
	jump rel (badselloop)

abort_tagged:
	move SIOP_KILL_AT to SCRATCH0
	jump rel (badselloop)

	/* a general entry point to abort requests.
	 * If the target goes to status phase then deal with it.
	 * Otherwise, abort the request in case the target does
	 * something stupid like continue processing with an incomplete
	 * command.
	 */
do_abort:
	BURST_OFF
	jump rel (abort_it), when DATA_IN
	jump rel (abort_it), when DATA_OUT
	jump rel (abort_it), when MSG_OUT
	jump rel (abort_it), when CMD
	jump rel (ssi), when STATUS

	/* the phase is a MSG_IN phase.  If the message is anything other
	 * than RESTORE_POINTER, then abort the request.
	 */
	move SIZEOF_U8, msgibuf, when MSG_IN
	jump rel (restore_ptr), if SCSI_RESTORE_POINTERS
	set atn
	clear ack
	jump rel (abort_it)

abort_it:
	set atn
	move CAM_REQ_ABORTED to SFBR
	jump rel (abort_job)

	/* move data from the target to host memory */
datain:
	move from 0, when DATA_IN
	return

	/* move data from host memory to the target */
dataout:
	move from 0, when DATA_OUT
	return

	/* handle phase mismatches.
	 * The host gets an interrupt and saves the DNAD and DBC
	 * registers in siopjob.ldata.
	 * This must be read back in from the host.
	 * everything else can be done here.
	 */
data_phase_mismatch:
	BURST_OFF
	/* copy back in the ldatai, loffset, and sdatai */
	move memory SIZEOF_U32, scsijobptr, pc+16
	move memory 24, 0, siopjob

	move memory SIZEOF_DATAI, siopjob_ldatai, current_datai
	move memory SIZEOF_U64, siopjob_loffset, current_offset
	jump rel (mainloop)


scb:	/* send command to target */
	call rel (setdsa)
	move from (SJ_OFFSET_CMDPTR-SJ_COPYOUT_SIZE), when CMD
	jump rel (mainloop), when not CMD	/* synchronize return */

	/* oops, another command phase!  Abort the request! */
	jump rel (do_abort)

ssi:	/* get status from target
	 *
	 *	get status byte
	 *	siopjob_status[3] = 1;
	 */
	move memory SIZEOF_U8, msg_ide, msg_parity

	move SIZEOF_U8, siopjob_status, when STATUS

	/* once the status byte has been received flag it */
	move 0 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, status_received
	move 1 to SCRATCH3
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH3, siopjob_status+3

	/* make sure that the transitions from STATUS are legal.
	 * The only legal phases after a STATUS message are MSGIN and
	 * MSGOUT.
	 */
status_change:
	jump rel (msgin), when MSG_IN
	jump rel (msgout), if MSG_OUT
	move CAM_SEQUENCE_FAIL to SFBR
	jump rel (abort_job), if not STATUS

	set atn
	/* eat extra status bytes */
status_eat:
	move SIZEOF_U8, discard, when STATUS
	jump rel (status_eat), when STATUS
	move CAM_SEQUENCE_FAIL to SFBR
	jump rel (abort_job), if not MSG_IN
	
	move SIZEOF_U8, msgibuf, when MSG_IN

	clear ack

status_spin:
	move SBCL & SBCL_BSY to SFBR
	jump rel (jobdone1), if 0
	jump rel (abort_it), if MSG_OUT
	jump rel (status_spin)


	/* Received a disconnect message.
	 * Just copy out the job and wait for a disconnect.
	 */
got_disconnect:
	clear ack
	wait disconnect 

	/* now copy out the job */
	copyout(siopjob,scsijobptr,SJ_COPYOUT_SIZE)

	jump rel (goidle)

	/* A job finished with no status byte received.  This is a sequence
	 * failure since the target has sent the COMMAND COMPLETE message.
	 * Assert ATN and try to abort the request.
	 */
no_status:
	move CAM_SEQUENCE_FAIL to SFBR
	set atn
	clear ack
	jump rel (abort_job)

	/* A job has completed because a COMMAND COMPLETE message
	 * has been received.
	 *
	 *	if(!siopjob_status[3])
	 *	    goto no_status;
	 *	clear ACK
	 *	wait DISCONNECT
	 *	siopjob_err = status_received;
	 *	if(siopjob_tagged)
	 *	{
	 *	   if((*siopjob_tagged & 7) == (siopjob_lun & 7))
	 *		*siopjopb_tagged = 0;
	 *	}
	 *	else
	 *	    SCRIPT_RAM_JOBTABLE[siopjob_lun] = 0;
	 *	copyout(*siopjob,scsijobptr,SJ_COPYOUT_SIZE);
	 *	SCRIPT_RAM_SH->sh_jobdone = siopjob_me;
	 *	SCRIPT_RAM_SH->sh_intstatus = JOB_DONE;
	 *	int JOBDONE;
	 *	clear SIGP
	 */
jobdone:
	move memory SIZEOF_U8, siopjob_status+3, SSIOP_REG_SCRATCH3
	move SCRATCH3 to SFBR
	jump rel (no_status), if 0

	clear ack
	wait disconnect

	/* change the error byte only if it's 0 */
jobdone1:
	move memory SIZEOF_U8, status_received, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
finishjob:
	/* set the error status */
	move SFBR to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, siopjob_err

	/* clear out the tag cache entry but only if this is the correct
	 * job for the entry (compare the cache line tags).
	 */
	move memory SIZEOF_U32, siopjob_tagged, SSIOP_REG_SCRATCH0
	call rel (is_zero_32)
	jump rel (finish_untagged), if 0

	/* get the cache line entry, make sure it non-zero */
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U8, 0, SSIOP_REG_SCRATCH0
	call rel (is_zero_32)
	jump rel (finish_copy), if 0

	move SCRATCH0 & 7 to SFBR
	move memory SIZEOF_U8, siopjob_lun, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, pc+12
	jump rel (finish_copy), if not 0

	/* zero out the entry if it's ours */
	move memory SIZEOF_U32,  siopjob_tagged, SSIOP_REG_SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U32, zero, 0
	jump rel (finish_copy)

finish_untagged:
	/* zero out the entry in the job table */
	move memory SIZEOF_U8, siopjob_lun, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH0 + 0 to SCRATCH0
	move SCRATCH0 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH0 + 0 to SCRATCH0

	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U32, zero, SCRIPT_RAM_JOBTABLE

finish_copy:
	/* now copy out the job */
	copyout(siopjob,scsijobptr,SJ_COPYOUT_SIZE)

	/* set the interrupt status */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+28
        move JOB_DONE to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0

	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_JOBDONE to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U64, siopjob_me, 0
	int JOB_DONE

	/* clear SIGP */
	move memory SIZEOF_U8, SSIOP_REG_CTEST2, discard+(SIOP_REG_CTEST2 & 3)
	jump rel (isnewjob)


collision:
	/* disable STO timeouts.
	 *
	 *	disable timeouts
	 *	SCRIPT_RAM_SH->sh_currentjob = 0;
	 *	status_received = 0;
	 *	clear TARGET
	 *	WAIT RESELECT
	 */
	move CTEST7 | 0x10 to CTEST7

	/* clear the currentjob pointer */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_CURRENTJOB to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U64, zero, 0
	move memory SIZEOF_U32, zero, scsijobptr
	/* the main idle state.
	 * Just wait for a reselection.
	 * If the host hits us with a SIGP then the branch will be
	 * taken.
	 * If we're selected then we also go to the alternate address.
	 */
idle:
	/* clear the status status */
	move CAM_REQ_CMP_ERR to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, status_received

	clear target
	WAIT RESELECT REL (selected)

	/* disable timeout interrupts once selected.
	 *
	 *	disable timeouts
	 *	if(target mode)
	 *	    goto idle
	 *	if(!connected)
	 *	    goto idle;
	 *	SCRATCH0 = KILL_ABORT;
	 *	if(! MSG_IN)
	 *	    goto badselect;
	 *	id_reselect[0] = LCRC;
	 *	msg_parity = msg_mpe;
	 *	get message in byte;
	 *	if(!(msgibuf[0] & 0x80))
	 *	    goto bad_identify;
	 *	SCRAYCH0 = id_reselect;
	 *	if(count_bits() != 2)
	 *	    goto bad_identify;
	 *	clear ack;
	 *	goto do_reselect;
	 */
	move CTEST7 | 0x10 to CTEST7

	move SCNTL0 & SCNTL0_TRG to SFBR
	jump rel (idle), if not 0
	move ISTAT & ISTAT_CON to SFBR
	jump rel (goidle), if 0

	move SIOP_KILL_ABORT to SCRATCH0
	jump rel (badreselect), when not MSG_IN
	/* OK, we've been reselected.  Try to figure out the target/lun
	 * of the reselection.
	 */

	/* save the selection bits */
	move LCRC to SFBR
	move SFBR to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect

	move memory SIZEOF_U8, msg_mpe, msg_parity

	/* get the identify message */
	move SIZEOF_U8, msgibuf, when MSG_IN

	/* make sure it's an identify message */
	jump rel (bad_identify), if not 0x80, and mask 0x7f
	/* check reserved bits */

	jump rel (bad_identify), if 0x18, and mask 0x18

	/* now check the reselection ID not make sure exactly two
	 * bits are set.
	 */
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	call rel (count_bits)
	jump rel (bad_identify), if not 2
	jump rel (do_reselect)

	/* Either the selection bits are bad or the identify message is
	 * bad.
	 */
bad_identify:
	clear ack
	move SIOP_KILL_ABORT to SCRATCH0
	set atn
	jump rel (badreselect)

	
	/*	Figure out which target is reselecting.
	 *
	 *	switch(id_reselect[0])
	 *	{
	 *	    case 1:	goto target 0; break;
	 *	    case 2:	goto target 1; break;
	 *	    case 4:	goto target 2; break;
	 *	    case 8:	goto target 3; break;
	 *	    case 16:	goto target 4; break;
	 *	    case 32:	goto target 5; break;
	 *	    case 64:	goto target 6; break;
	 *	    case 128:	goto target 7; break;
	 *	    default:
	 *		SCRATCH0 = SIOP_KILL_ABORT;
	 *		goto bad_reselect;
	 *		break;
	 *	}
	 */
do_reselect:
	/* save the IDENTIFY message for debugging */
	move memory SIZEOF_U8, msgibuf, identify
	move memory SIZEOF_U8, msgibuf, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, id_reselect+1

	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (target0), if 1, and mask 0xfe
check1:
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (target1), if 2, and mask 0xfd
check2:
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (target2), if 4, and mask 0xfb
check3:
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (target3), if 8, and mask 0xf7
check4:
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (target4), if 0x10, and mask 0xef
check5:
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (target5), if 0x20, and mask 0xdf
check6:
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (target6), if 0x40, and mask 0xbf
check7:
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (target7), if 0x80, and mask 0x7f
	move SIOP_KILL_ABORT to SCRATCH0
	jump rel (badreselect)

	/*
	 *	if(SCID & 1)
	 *	    goto check1;
	 *	id_reselect[0] = 0;
	 *	SCRATCH0 = identify & 7;
	 *	temp = SCRIPT_RAM_TDTABLE[0];
	 *	goto findjob;
	 */
target0:
	move SCID to SFBR
	jump rel (check1), if 1
	/* now get the identify message */
	move 0 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect
	move memory SIZEOF_U8, identify, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move memory 1, SCRIPT_RAM_TDTABLE+1, temp+1
	jump rel (findjob)

	/*
	 *	if(SCID & 2)
	 *	    goto check2;
	 *	id_reselect[0] = 1;
	 *	SCRATCH0 = identify & 7;
	 *	temp = SCRIPT_RAM_TDTABLE[1];
	 *	goto findjob;
	 */
target1:
	move SCID to SFBR
	jump rel (check2), if 2
	/* now get the identify message */
	move 1 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect
	move memory SIZEOF_U8, identify, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move SCRATCH0 | 8 to SCRATCH0
	move memory 1, SCRIPT_RAM_TDTABLE+5, temp+1
	jump rel (findjob)

	/*
	 *	if(SCID & 4)
	 *	    goto check3;
	 *	id_reselect[0] = 2;
	 *	SCRATCH0 = identify & 7;
	 *	temp = SCRIPT_RAM_TDTABLE[2];
	 *	goto findjob;
	 */
target2:
	move SCID to SFBR
	jump rel (check3), if 4
	/* now get the identify message */
	move 2 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect
	move memory SIZEOF_U8, identify, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move SCRATCH0 | 16 to SCRATCH0
	move memory 1, SCRIPT_RAM_TDTABLE+9, temp+1
	jump rel (findjob)

	/*
	 *	if(SCID & 8)
	 *	    goto check4;
	 *	id_reselect[0] = 3;
	 *	SCRATCH0 = identify & 7;
	 *	temp = SCRIPT_RAM_TDTABLE[3];
	 *	goto findjob;
	 */
target3:
	move SCID to SFBR
	jump rel (check4), if 8
	/* now get the identify message */
	move 3 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect
	move memory SIZEOF_U8, identify, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move SCRATCH0 | 24 to SCRATCH0
	move memory 1, SCRIPT_RAM_TDTABLE+13, temp+1
	jump rel (findjob)

	/*
	 *	if(SCID & 0x10)
	 *	    goto check5;
	 *	id_reselect[0] = 4;
	 *	SCRATCH0 = identify & 7;
	 *	temp = SCRIPT_RAM_TDTABLE[4];
	 *	goto findjob;
	 */
target4:
	move SCID to SFBR
	jump rel (check5), if 0x10
	/* now get the identify message */
	move 4 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect
	move memory SIZEOF_U8, identify, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move SCRATCH0 | 32 to SCRATCH0
	move memory 1, SCRIPT_RAM_TDTABLE+17, temp+1
	jump rel (findjob)

	/*
	 *	if(SCID & 0x20)
	 *	    goto check6;
	 *	id_reselect[0] = 5;
	 *	SCRATCH0 = identify & 7;
	 *	temp = SCRIPT_RAM_TDTABLE[5];
	 *	goto findjob;
	 */
target5:
	move SCID to SFBR
	jump rel (check6), if 0x20
	/* now get the identify message */
	move 5 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect
	move memory SIZEOF_U8, identify, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move SCRATCH0 | 40 to SCRATCH0
	move memory 1, SCRIPT_RAM_TDTABLE+21, temp+1
	jump rel (findjob)

	/*
	 *	if(SCID & 0x40)
	 *	    goto check7;
	 *	id_reselect[0] = 6;
	 *	SCRATCH0 = identify & 7;
	 *	temp = SCRIPT_RAM_TDTABLE[6];
	 *	goto findjob;
	 */
target6:
	move SCID to SFBR
	jump rel (check7), if 0x40
	/* now get the identify message */
	move 6 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect
	move memory SIZEOF_U8, identify, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move SCRATCH0 | 48 to SCRATCH0
	move memory 1, SCRIPT_RAM_TDTABLE+25, temp+1
	jump rel (findjob)

	/*
	 *	if(SCID & 0x80)
	 *	    goto badreselect;
	 *	id_reselect[0] = 7;
	 *	SCRATCH0 = identify & 7;
	 *	temp = SCRIPT_RAM_TDTABLE[7];
	 */
target7:
	move SCID to SFBR
	move SIOP_KILL_ABORT to SCRATCH0
	jump rel (badreselect), if 0x80
	/* now get the identify message */
	move 7 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, id_reselect
	move memory SIZEOF_U8, identify, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 7 to SCRATCH0
	move SCRATCH0 | 56 to SCRATCH0
	move memory 1, SCRIPT_RAM_TDTABLE+29, temp+1

	/* we have a reselection and and identify message.  See if
	 * the job is in our tables.
	 *
	 * the target/lun of the reconnecting request in in SCRATCH0.
	 *
	 *	SXFER = temp[1] | SXFER_DHP;
	 *	scsijobptr = SCRIPT_RAM_JOBTABLE[<target_id>]
	 *	if(scsijobptr)
	 *	    goto getjob;
	 *	if(MSG_IN)
	 *	    goto gettag;
	 *	SCRATCH0 = SIOP_KILL_ABORT;
	 *	goto badreselect;
	 */
findjob:
	/* restore the synchonous transfer parameters */
	move memory 1, temp+1, SSIOP_REG_SCRATCH1
	move SCRATCH1 | SXFER_DHP to SFBR
	move SFBR to SXFER

	/* get the word offset into the job table (multiply by 4)  */
	move SCRATCH0 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH0 + 0 to SCRATCH0
	move SCRATCH0 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH0 + 0 to SCRATCH0

	/* now get the pointer from the table */
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, SCRIPT_RAM_JOBTABLE, scsijobptr
	move memory SIZEOF_U32, scsijobptr, SSIOP_REG_SCRATCH0

	/* clear the ACK here AFTER the job has been located and
	 * the synchonous transfer parameters have been loaded.
	 * Clearing the ACK any earlier could allow the target
	 * to start transferring data before the proper synchonous
	 * transfer parameters are set.
	 */
	clear ack

	/* At this point the reconnecting request can either be tagged
	 * or untagged.  If it is untagged there will be a pointer
	 * to the job in the JOBTABLE.  Otherwise it is either a bogus
	 * reconnect or it is tagged.  This test is valid because
	 * tagged and untagged requests can't be mixed.
	 * The can be some abiguity concerning contengent allegience
	 * and untagged requests.  This is handled if a tag message
	 * is received.
	 */
	call rel (is_zero_32)
	jump rel (getjob), if not 0

	jump rel (gettag), when MSG_IN

	/* Ok, there are no more message bytes, if there was no job
	 * for the reconnecting request assert ATN and prepare to send
	 * and ABORT message.
	 */

	move SIOP_KILL_ABORT to SCRATCH0
	jump rel (badreselect)

	/* Ok, this is a valid reselection.  Read in the SIOPJOB
	 * structure and proceed.
	 *
	 *	copyin(scsijobptr,&siopjob,SJ_COPYIN_SIZE);
	 *	SCRIPT_RAM_SH->sh_currentjob = scsijobptr;
	 *	if(siopjob.sj_abort)
	 *	    goto dont_reselect;
	 *	if(!siopjob.sj_term)
	 *	    goto restore_ptr;
	 *	if(STATUS)
	 *	    goto restore_ptr;
	 *	
	 */
getjob:
	copyin(scsijobptr,siopjob,SJ_COPYIN_SIZE)

	/* set up the current job field in the scripthost struct */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_CURRENTJOB to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U64, siopjob_me, 0

	/* test term and abort and kill the job if necessary */
	move memory SIZEOF_U8, siopjob_abort, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (dont_reselect), if not 0

	/* test term to see if the request should be terminated */
	move memory SIZEOF_U8, siopjob_term, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (restore_ptr), if 0

	/* if the request is just reconnecting to transfer the STATUS
	 * byte and finish the request, then go ahead and do it.
	 * This is permissable since the CAM abort is not gauranteed
	 * to work.
	 */
	jump rel (restore_ptr), when STATUS

	/* A request is to be terminated.
	 * Set ATN and wait for a MSG_OUT phase.  If there is a data
	 * phase or a command phase then reset the bus because the
	 * target isn't responding to ATN.
	 * Send the TERMINATE message and wait for a STATUS and 
	 * a COMMAND COMPLETE message.  If any non-completion
	 * message arrives perform the abort sequence.
	 */

termjob:
	set atn

termloop:
	jump rel (termout), when MSG_OUT
	jump rel (termin), if MSG_IN
	jump rel (bus_reset), if DATA_IN
	jump rel (bus_reset), if DATA_OUT
	jump rel (term_ssi), if STATUS
	jump rel (bus_reset), if CMD

termout:
	move SIZEOF_U8, msgterminate, when MSG_OUT
	jump rel (termloop)

term_ssi:
	call rel (setdsa)
	move SIZEOF_U8, siopjob_status, when STATUS

	/* mark the status as received */
	move 0 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, status_received

	jump rel (termloop)

termin:
	move SIZEOF_U8, msgibuf, when MSG_IN
	jump rel (jobdone), if SCSI_COMMAND_COMPLETE
	set atn
	clear ack
	move CAM_REQ_TERMIO to SFBR
	jump rel (abort_job)


	/* Get the next byte of a message.  It may be a SIMPLE QUEUE
	 * message.  If so, deal with it.  If not and there is no
	 * current job, assert ATN and abort the request.  If there
	 * is a job, continue processing normally.
	 *
	 *	if(msgibuf[0] ==  SCSI_SIMPLE_QUEUE_TAG)
	 *	    goto gottag;
	 *	clear ack
	 *	SCRATCH0 = SIOP_KILL_ABORT;
	 *	goto badreselect;
	 */
gettag:
	move SIZEOF_U8, msgibuf, when MSG_IN
	jump rel (gottag), if SCSI_SIMPLE_QUEUE_TAG

	clear ack

	move SIOP_KILL_ABORT to SCRATCH0
	jump rel (badreselect)

	/* it's a tagged request, read in the tag and then call the host
	 * with a LOOKUP RPC
	 * Before doing the final lookup, clear any current job.
	 *
	 *	scsijobptr = 0;
	 *	clear ack;
	 *	if(! MSG_IN)
	 *	    goto badreselect;
	 *	id_reselect[2] = msgibuf[0];
	 *	if(tag_cache_lookup())
	 *	    goto getjob;
	 *	SCRIPT_RAM_SH->sh_rpcdata = id_reselect;
	 *	SCRIPT_RAM_SH->sh_rpc = RPC_LOOK_TAG;
	 *	SCRIPT_RAM_SH->sh_intstatus = RPC_REQUEST;
	 *	int RPC_REQUEST;
	 *	scsijobptr = SCRIPT_RAM_SH->sh_rpcreply;
	 *	clear ack;
	 *	if(scsijobptr)
	 *	    goto getjob;
	 *	SCRATCH0 = SIOP_KILL_AT;
	 *	goto badreselect;
	 */
gottag:
	move memory SIZEOF_U32, zero, scsijobptr

	clear ack

	move SIOP_KILL_ABORT to SCRATCH0
	jump rel (badreselect), when not MSG_IN
	move SIZEOF_U8, msgibuf, when MSG_IN
	move SFBR to SCRATCH2
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH2, id_reselect+2

	/* all the target supplied info has come in.
	 * Use the tid/lun/tag to do a lookup in the tag cache.
	 * If the cache hits then the RPC is not needed.
	 */
	jump rel (tag_cache_lookup)
tcl_return:
	call rel (is_zero_32)
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, scsijobptr
	jump rel (getjob), if not 0

	/* now do the RPC */
	/* copy the offset to the rpcdata field */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCDATA to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U32, id_reselect, 0

	/* set the interrupt status */
        move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+28
        move RPC_REQUEST to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0

	/* set the RPC request */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREQ to SCRATCH0
        move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+28
        move RPC_LOOK_TAG to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	int RPC_REQUEST

	/* if the host can't find the request set ATN and then clear
	 * ACK.  The target should then go to MSG_OUT so the ABORT
	 * TAG message can be sent.
	 */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREPLY to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, SSIOP_REG_SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, scsijobptr

	clear ack

	call rel (is_zero_32)
	jump rel (getjob), if not 0

	move SIOP_KILL_AT to SCRATCH0

	/* at this point the target should disconnect */
	jump rel (badreselect)

	/* Just perform the abort sequence for the aborted request.
	 * if the reconnecting request is sending a STATUS byte back,
	 * then the request has completed so just finish up normally
	 * and ignore the abort.  This is permissable since the CAM
	 * aborts are not guaranteed to work.
	 */
dont_reselect:
	jump rel (restore_ptr), when STATUS
	set atn
	move CAM_REQ_ABORTED to SFBR
	jump rel (abort_job)

	/* The following code is general to handle aborting a request on
	 * the bus.  The code does not depend on a current siopjob so
	 * that it may be used for illegal reconnects.  This also
	 * implies that an active siopjob must have been discarded
	 * before this routine is called.
	 *
	 * The basic premise is that we try to get the targets attention
	 * and send it ever escalating messages until it finally decides
	 * to go away (at which time the host will get a UDC and
	 * restart the SCRIPTS from the main entry point.
	 * The caller decided where to begin by setting SCRATCH0 to one
	 * of the following:
	 *
	 *	3	send an ABORT TAG
	 *	2	send an ABORT
	 *	1	send a DEVICE RESET
	 *	0	reset the bus
	 *
	 * each time one of these actions is taken SCRATCH0 is decremented
	 * by 1 so that if the target doesn't respond as expected the next
	 * action is taken until the bus is finally reset.
	 */
badreselect:
	move memory SIZEOF_U32, zero, scsijobptr
	set atn

badselloop:
	jump rel (badmsgout), when MSG_OUT
	jump rel (badmsgin), if MSG_IN
	/* never send any invalid data to a target */
	jump rel (bus_reset), if DATA_OUT
	jump rel (baddatain), if DATA_IN
	jump rel (badcmd), if CMD
	jump rel (bus_reset), if STATUS
	jump rel (badselloop)

	/* there is no associated request, so just read any message bytes
	 * and toss them.  We're waiting for the target to respond to
	 * the ATN.
	 */
badmsgin:
	move SIZEOF_U8, msgibuf, when MSG_IN
	jump rel (bus_reset), if SCSI_COMMAND_COMPLETE
	jump rel (bus_reset), if SCSI_DISCONNECT
	jump rel (bus_reset), if SCSI_LINKED_COMMAND_COMPLETE
	jump rel (bus_reset), if SCSI_LINKED_COMMAND_COMPLETE_WFLAG
	jump rel (bus_reset), if SCSI_INITIATE_RECOVERY
	jump rel (bus_reset), if SCSI_RELEASE_RECOVERY
	set atn
	clear ack
	jump rel (badselloop)

badmsgout:
	move SCRATCH0 to SFBR
	move SFBR + 255 to SCRATCH0
	jump rel (badat), if SIOP_KILL_AT
	jump rel (badabort), if SIOP_KILL_ABORT
	jump rel (baddr), if SIOP_KILL_DRESET
	jump rel (bus_reset)

badabort:
	move SIZEOF_U8, msgabort, when MSG_OUT
	jump rel (badselloop)

badat:
	move SIZEOF_U8, msg_abort_tag, when MSG_OUT
	jump rel (badselloop)

baddr:
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, temp
	/* now tell the host we're about to do a device reset.
	 * This is ok since if the device reset doesn't take we'll be
	 * doing a bus reset.
	 */

	/* set the interrupt status */
        move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+28
        move RPC_REQUEST to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0

	/* copy the target ID to the RPC DATA field */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCDATA to SCRATCH0
	move SCRATCH0 + 2 to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U8, SSIOP_REG_SDID, 0

	/* set the RPC request */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREQ to SCRATCH0
        move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+28
        move RPC_DEVICE_RESET to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	int RPC_REQUEST

	/* now do the reset.  If the reset results in any phase other
	 * than a bus free, nuke the entire bus.
	 */
	
	jump rel (bus_reset), when not MSG_OUT

	move SIZEOF_U8, msg_device_reset, when MSG_OUT
	move memory SIZEOF_U32, temp, SSIOP_REG_SCRATCH0
	jump rel (badselloop)

baddatain:
	set atn
	move SIZEOF_U8, temp, when DATA_IN
	jump rel (badselloop)

badcmd:
	set atn
	move 1, zero, when CMD		/* 0 is really a TUR */
	jump rel (badselloop)

	

	/* Well, we've either been selected (bad) or we have been
	 * interrupted by the host (SIGP).
	 * Test SIGP first.
	 * If this isn't set then we've been selected.  In this case,
	 * set target mode and do an immediate disconnect.
	 */
selected:
	/* read the SIGP bit from CTEST2 */
	move CTEST2 & 0x40 to SFBR
	jump REL (goidle), if not 0
	set target
	disconnect
	clear target
	jump REL (idle)

	/* handle extended messages from the target.
	 * First, read in the extended message length byte.
	 * Then read in the extended message code byte.
	 * If the message code is not handled, reject the message
	 * here.
	 * If the message is handled, read in the remained of the
	 * message and do something with it.
	 */
extended:
	clear ack
	jump rel (bad_seq), when not MSG_IN
	move SIZEOF_U8, extended_msg_size, when MSG_IN
	clear ack
	jump rel (bad_seq), when not MSG_IN
	move SIZEOF_U8, extended_msg_type, when MSG_IN
	jump rel (got_sync), if SCSI_SYNCHRONOUS_XFER
	jump rel (reject_msg), if SCSI_WIDE_XFER

	/* process a MODIFY DATA POINTERS message.
	 * This is done via an RPC to the host.
	 * Check the return value to make sure the request was sane.
	 * If not, reject the mssage.
	 */
	move memory SIZEOF_U8, extended_msg_size, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (reject_msg), if not 5		/* message length must be 5 */
	clear ack
	/* Transfer each byte separately so phase mismatch errors are
	 * not generated and any bogus phase change can be detected here.
	 */
	jump rel (bad_seq), when not MSG_IN
	move SIZEOF_U8, extended_msg, when MSG_IN
	clear ack
	jump rel (bad_seq), when not MSG_IN
	move SIZEOF_U8, extended_msg+1, when MSG_IN
	clear ack
	jump rel (bad_seq), when not MSG_IN
	move SIZEOF_U8, extended_msg+2, when MSG_IN
	clear ack
	jump rel (bad_seq), when not MSG_IN
	move SIZEOF_U8, extended_msg+3, when MSG_IN

	/* the pointer offset comes in off the bus MSB first.  This must
	 * be fixed to be little endian by the host (it's easier for the
	 * host that for the SCRIPTS).
	 *
	 * Set up the RPC.
	 *
	 *	copyout(&siopjob,scsijobptr,SJ_COPYOUT_SIZE);
	 *	SCRIPT_RAM_SH->sh_rpc_data = extended_msg;
	 *	SCRIPT_RAM_SH->sh_rpcreq = RPC_MODIFY_PTR;
	 *	SCRIPT_RAM_SH->sh_intstatus = RPC_REQUEST;
	 *	int RPC_REQUEST;
	 */

	/* copy out the current pointer and SCRIPTS modifyable data */
	copyout(siopjob,scsijobptr,SJ_COPYOUT_SIZE)

	/* copy the offset to the rpcdata field */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCDATA to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U32, extended_msg, 0

	/* set the interrupt status */
        move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+28
        move RPC_REQUEST to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0

	/* set the RPC request */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREQ to SCRATCH0
        move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+28
        move RPC_MODIFY_PTR to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	int RPC_REQUEST

	/* now check the result to make sure it's OK.
	 *
	 *	if(SCRIPT_RAM_SH->sh_rpcreply)
	 *	    goto reject_msg;
	 *	clear ack
	 *	copyin(scsijobptr,&siopjob,SIOP_COPYIN_SIZE);
	 *	current_datai = siopjob.sj_ldatai;
	 *	current_datap = siopjob.sj_datap;
	 *	current_offset = siopjob.sj_offset;
	 *	goto mainloop;
	 */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREPLY to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, SSIOP_REG_SCRATCH0
	call rel (is_zero_32)
	jump rel (reject_msg), if not 0

	/* everything went ok.
	 * Ack the last byte and return to the main loop
	 */
	clear ack

	/* copy in the new data pointers */
	copyin(scsijobptr,siopjob,SJ_COPYIN_SIZE)

	/* set up current data pointers */
	move memory SIZEOF_DATAI, siopjob_ldatai, current_datai
	move memory SIZEOF_U64, siopjob_loffset, current_offset
	move memory SIZEOF_U32, siopjob_ldatap, current_datap

	jump rel (mainloop)

	/* code to process a received SDTR message.  This can happen
	 * either in response to a SDTR we sent or because the target
	 * wants to renegociate the transfer parameters.
	 * Either way, the RPC call is required for the host to
	 * check the validity of the parameters.
	 * The host will communicate the appropriate action to take
	 * by setting bits in the rpc_reply member.
	 */
got_sync:
	/* first, validate the message size */
	move memory SIZEOF_U8, extended_msg_size, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	jump rel (reject_msg), if not 3		/* message length must be 3 */
	clear ack
	/* read each byte separately to prevent a phase mismatch */
	jump rel (bad_seq), when not MSG_IN
	move SIZEOF_U8, extended_msg, when MSG_IN
	clear ack
	jump rel (bad_seq), when not MSG_IN
	move SIZEOF_U8, extended_msg+1, when MSG_IN

	/* now set up the RPC */

	/* copy out the current pointer and SCRIPTS modifyable data */
	copyout(siopjob,scsijobptr,SJ_COPYOUT_SIZE)

	/* copy the transfer parameters into the rpc_data field */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCDATA to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U16, extended_msg, 0

	/* set the interrupt status */
        move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+28
        move RPC_REQUEST to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0

	/* set the RPC request */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREQ to SCRATCH0
        move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+28
        move RPC_CAL_SYNC to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	int RPC_REQUEST

	/* the reply will have bits set for each action we are to take.
	 * This includes clearing the ACK and rejecting the message.
	 */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREPLY to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, SSIOP_REG_SCRATCH0

	move SCRATCH3 & RPC_REPLY_ATN to SFBR
	jump rel (sync_ack), if 0
	/* raise attn */
	set atn

sync_ack:
	move SCRATCH3 & RPC_REPLY_ACK to SFBR
	jump rel (sync_error), if 0
	clear ack

	/* if the host flagged and error then ATN was also flagged.
	 * Send a reject message.
	 */
sync_error:
	move SCRATCH3 & RPC_REPLY_ERROR to SFBR
	jump rel (rejectit), if not 0

	/* if ATN is set but error isn't then we're transmitting
	 * a reply back to the target (it must have been the target
	 * that initiated the negotiation.
	 * Otherwise the negotiation was successful and the reply
	 * values should be used for all future data transfers (the
	 * host should have left this in byte 1 of the reply).
	 * The host should place the table offset*4 in byte 0 of the
	 * reply.
	 */
        move SCRATCH3 & RPC_REPLY_ATN to SFBR
	jump rel (sync_reply), if not 0

	/* the target's ACK was ok, just load the new parameters
	 * and continue with the request.
	 */
	move SCRATCH0 + 1 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, SCRIPT_RAM_TDTABLE
	move SCRATCH1 | SXFER_DHP to SFBR
	move SFBR to SXFER

	jump rel (mainloop)

	/* send a reply to a sync negotiation request from a target.
	 * The target MUST transition to message in.
	 * The information needed to form the message is provided by
	 * the host.
	 */
sync_reply:
	jump rel (send_sync_reply), when MSG_OUT

	set atn
	move CAM_DATA_RUN_ERR to SFBR
	jump rel (abort_job)

send_sync_reply:
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, temp
	/* create the outgoing message from the data in the RPC reply. */
	move SCRATCH1 & 0xf to SFBR
	move SFBR to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, extended_msg+4
	move SCRATCH2 to SFBR
	move SFBR to SCRATCH3
	move 1 to SCRATCH0
	move 1 to SCRATCH2
	move 3 to SCRATCH1
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, extended_msg

	move 5, extended_msg, when MSG_OUT

	/* if all went well then the transfer parameters can be set
	 * up.  Otherwise, the target rejected the message and
	 * no new parameters will get set up (this is the default).
	 * If the message was rejected then the move command will get
	 * a phase mismatch and the SCRIPTS will be re-entered from
	 * one of the error entry points.
	 */
	move memory SIZEOF_U32, temp, SSIOP_REG_SCRATCH0
	move SCRATCH0 + 1 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, SCRIPT_RAM_TDTABLE
	move SCRATCH1 | SXFER_DHP to SFBR
	move SFBR to SXFER

	jump rel (mainloop), when not MSG_IN

	/* If the target rejects the reply then default to async */
	move memory SIZEOF_U8, msg_mpe, msg_parity
	move SIZEOF_U8, msgibuf, when MSG_IN
	jump rel (check_msg_type), if not SCSI_MESSAGE_REJECT

	clear ack
	move memory SIZEOF_U32, temp, SSIOP_REG_SCRATCH0
	move 0 to SCRATCH1
	move SCRATCH0 + 1 to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, SCRIPT_RAM_TDTABLE
	move SCRATCH1 | SXFER_DHP to SFBR
	move SFBR to SXFER
	jump rel (mainloop)

reject_msg:
	set atn		/* Get the target's attention (let's hope) */
	clear ack

	/* read in any more bytes */
	jump rel (rejectit), when MSG_OUT

eloop:
	move SIZEOF_U8, discard, when MSG_IN
	clear ack
	jump rel (eloop), when MSG_IN

	/* If the next phase is not MSG_OUT then the target is not
	 * responding to the ATN.  This is bad.  When this happens
	 * the only choice is to reset the bus.
	 * Make sure the instructions synchronize the phases.
	 */
rejectit:
	jump rel (bus_reset), when not MSG_OUT
	move SIZEOF_U8, msgreject, when MSG_OUT

	/* the reject has now been sent.  Make sure that the target does
	 * not reject the reject.  If this happens, then we have to
	 * do whatever is necessary to stop the target from further processing
	 * the request.
	 *
	 * If the next phase is not a MSG_IN phase then the target
	 * either accepted or ignored the REJECT.
	 */
	jump rel (mainloop), when not MSG_IN

	/* OOPS, the target switched to MSG_IN.  This may not be too
	 * good.  Read the first msg byte.  If it's a reject then
	 * try to stop the target.  If it's something else, then
	 * go to the main message processing code.
	 */
	move SIZEOF_U8, msgibuf, when MSG_IN
	jump rel (check_msg_type), if not SCSI_MESSAGE_REJECT

	/* It was a reject message.  The first step here is to try
	 * to send an abort.
	 */

	set atn

	clear ack

	move CAM_REQ_ABORTED to SFBR

	jump rel (abort_job)

bus_reset:
	/* RESET THE SCSI BUS.
	 * This must be done when a target fails to respond to an ATN.
	 * In this case, the target must be immediately stopped from going
	 * going any further.
	 *
	 * The trick here is to control the interrupting of the host since
	 * the host will re-enter the SCRIPTS at main on a bus reset.
	 * To releive the host from doing the timeout of the reset,
	 * do it here.  Disable the reset interrupt until the reset
	 * is finished.  The host will remove the reset condition.
	 */
	move SIEN & ~SIEN_RST to SIEN
	move SCNTL1 | SCNTL1_RST to SCNTL1

	/* the bus is now in reset, time it for 25 usec */
	move 0x20 to SFBR
	call rel (delay)

	/* clear the FIFO's */
	move CTEST8 | CTEST8_CLF to CTEST8

	move SIEN | SIEN_RST to SIEN
	int 0
	jump rel (main)

got_reject:
	/* We've received a reject message.  This means we should
	 * have just taken a phase mismatch interrupt at the byte
	 * at which the target changed phases.
	 * Thus, the offset to the offending byte is in the rpcdata
	 * field.  All we have to do is to complete the reject
	 * processing by doing an RPC to the host.
	 */
	/* set the interrupt status */
        move RPC_REQUEST to SCRATCH0 
        move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+20
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	
	/* set the RPC request */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREQ to SCRATCH0
        move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+28
        move RPC_MSG_REJECT to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	int RPC_REQUEST

	/* now test the RPC result.
	 * If the result is RPC_REPLY_ERROR then the bus should be reset
	 * from here (the host can't do it since it would screw up the
	 * state of the SCRIPTS from the RPC interrupt.
	 */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREPLY to SCRATCH0
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+16
	move memory SIZEOF_U32, 0, SSIOP_REG_SCRATCH0

	move SCRATCH3 to SFBR

	/* make sure ATN is set if needed before clearing ACK */
	jump rel (clear_reply_atn), if not RPC_REPLY_ATN, and mask 0xbf

	set atn
	jump rel (set_reply_ack)

clear_reply_atn:
	clear atn

set_reply_ack:

	clear ack

	/* at this point if there is an error and the job is to be
	 * aborted (with varying degrees of prejudice) then SCRATCH0
	 * will contain the level at which to start the abort process.
	 * At this point we can forget about the job, it's history.
	 */
	jump rel (badreselect), if RPC_REPLY_ERROR, and mask 0x7f

	jump rel (msg_phase_mismatch)

	/* Restore Data Pointers.  Copy the saved data pointer to
	 * the current data pointer.
	 *
	 * First, check the DMA window to determine if the current
	 * window is at the saved pointer.  If the window has moved
	 * since the pointer was saved then sdatai.di_mbz will be 
	 * non-zero.  If this is the case then an RPC_REST_PNTR
	 * RPC must be done to get the host to rebuild the DMA
	 * window at the saved data pointer.
	 *
	 *	clear ack
	 *	if(siopjob.sj_sdatai.mbz == 0)
	 *	    goto copyptr;
	 *	copyout(&siopjob,scsijobptr,SJ_COPYOUT_SIZE);
	 *	SCRIPT_RAM_SH->sh_intstatus = RPC_REQUEST;
	 *	SCRIPT_RAM_SH->sh_rpcreq = RPC_REST_PNTR;
	 *	int RPC_REQUEST;
	 *	copying(scsijobptr,&siopjob,SJ_COPYIN_SIZE);
	 */
restore_ptr:
	clear ack

	move memory SIZEOF_U8, siopjob_sdatai+3, SSIOP_REG_SCRATCH3
	move SCRATCH3 to SFBR
	jump rel (copyptr), if 0

	/* First, the siopjob structure must be copied out to maintain 
	 * any values changed by the SCRIPTS.  This is not too expensive
	 * since only a small part of the structure gets copied out.
	 */
	copyout(siopjob,scsijobptr,SJ_COPYOUT_SIZE)

	/* set the interrupt status */
        move memory SIZEOF_U32, SCRIPT_RAM_SH, pc+28
        move RPC_REQUEST to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0

	/* set the RPC request */
	move memory SIZEOF_U32, SCRIPT_RAM_SH, SSIOP_REG_SCRATCH0
	move SCRATCH0 + SH_OFFSET_RPCREQ to SCRATCH0
        move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+28
        move RPC_REST_PNTR to SCRATCH0 
        move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, 0
	int RPC_REQUEST

	/* don't really need to check the result since an error
	 * can not occur since the data pointer has been at this
	 * value before (it's a know good value).
	 */

	/* now transfer in the new scsijob structure */
	copyin(scsijobptr,siopjob,SJ_COPYIN_SIZE)

copyptr:
	/* restore the data pointers.
	 *
	 *	current_datai = siopjob.sj_sdatai;
	 *	current_offset = siopjob.sj_soffset;
	 *	current_datap = siopjob.sj_sdatap;
	 *	siopjob.sj_datai[curent_datap] = current_datai;
	 *	siopjob.sj_msgoptr = siopjob.sj_smsgoptr;
	 */
	move memory SIZEOF_DATAI, siopjob_sdatai, current_datai
	move memory SIZEOF_U64, siopjob_soffset, current_offset
	move memory SIZEOF_U32, siopjob_sdatap, current_datap

	/* copy ldatai to the current datai array entry to the data transfer
	 * picks up right from where it left off.
	 */
	move memory SIZEOF_U32, current_datap, SSIOP_REG_SCRATCH0
	call rel (setdsa)
	move DSA1 to SFBR
	move SFBR to SCRATCH1
	move DSA2 to SFBR
	move SFBR to SCRATCH2
	move 0x80 to SCRATCH3
	move memory SIZEOF_U32, SSIOP_REG_SCRATCH0, pc+20
	move memory SIZEOF_DATAI, current_datai, 0

	/* restore message pointers */
	move memory SIZEOF_DATAI, siopjob_smsgoptr, siopjob_msgoptr

	/* command pointers should never change */

	jump rel (mainloop)

	/* Save the Data pointer.  This is done by making a copy of the
	 * current ldatai and the associated doffset value of that entry.
	 * This will identify the starting offset (into the host buffer)
	 * of the scatter/gather entry and the number of bytes transferred
	 * so far.
	 */
save_ptr:
	clear ack

	move memory SIZEOF_DATAI, siopjob_ldatai, siopjob_sdatai
	move memory SIZEOF_U32, siopjob_ldatap, siopjob_sdatap
	move memory SIZEOF_U64, siopjob_loffset, siopjob_soffset

	jump rel (mainloop)


	/* When a parity error occurs the host will vector the SIOP
	 * here.  Continue to process data phases (the target may
	 * not respond to the ATN until a block boundry).  If any
	 * unexpected phases occur then the target is not responding
	 * to the ATN so the bus should be reset.
	 * Once the target enters the DATA_OUT phase, transfer the
	 * appropriate message and wait for the target's response.
	 * If the response is a reject then abort the request.  If
	 * the response is any other message or a status phase then
	 * things are probably ok. If the target goes to a data
	 * phase before going to a MSGIN phase then reset the
	 * bus because the target is screwed up (it must not
	 * transfer any more data until it has restored the data
	 * pointers.
	 */

parity_error:
	BURST_OFF
	set atn
	clear ack

par_svcloop:
	jump rel (par_msgout), when MSG_OUT
	jump rel (bus_reset), if DATA_OUT
	jump rel (bus_reset), if MSG_IN
	jump rel (par_datain), if DATA_IN
	jump rel (bus_reset), if CMD
	jump rel (bus_reset), if STATUS

par_msgout:
	move SIZEOF_U8, msg_parity, when MSG_OUT
	jump rel (par_replyloop), when not MSG_OUT

par_datain:
	move SIZEOF_U8, discard, when DATA_IN
	jump rel (par_svcloop)

par_replyloop:
	jump rel (par_msgin), when MSG_IN
	jump rel (par_baddata), if MSG_OUT
	jump rel (par_baddata), if DATA_OUT
	jump rel (par_baddata), if DATA_IN
	jump rel (par_baddata), if CMD
	jump rel (ssi), if STATUS

par_baddata:
	set atn
	move CAM_UNCOR_PARITY to SFBR
	jump rel (abort_job)

par_msgin:
	move SIZEOF_U8, msgibuf, when MSG_IN
	jump rel (check_msg_type), if not SCSI_MESSAGE_REJECT
	set atn
	clear ack
	move CAM_UNCOR_PARITY to SFBR
	jump rel (abort_job)


/**********************************************************************/
	/* SUBROUTINES:
	 * the memory move instruction CAN NOT be used in any subroutine
	 */

	/* determine if a 32 bit value in the SCRATCH register is 0.
	 * return 0 in the SFBR if it is, and non-0 if it isnt.
	 */
is_zero_32:
	move SCRATCH0 to SFBR
	return if not 0
	move SCRATCH1 to SFBR
	return if not 0
	move SCRATCH2 to SFBR
	return if not 0
	move SCRATCH3 to SFBR
	return 

	/* set up the DSA register.  This requires that the location
	 * of the siopjob be a constant.
	 * The lowest and highest order bytes are known to be 0.
	 * This leaves the middle two bytes, which depend on the SIOP
	 * on which we're running.
	 * These are set at starup based on the base address of the
	 * SCRIPTS.
	 */
setdsa:
	move 0 to DSA0
	move 0 to DSA1
	move 0 to DSA2
	move 0x80 to DSA3
	return

	/* A general delay loop.  Delay up to 256 loops.
	 * The caller passes in the loop count in the SFBR.
	 * Just loop, decrementing SFBR until it's 0.
	 */

delay:
	return if 0
	move SFBR + 255 to SFBR
	NOP
	NOP
	NOP
	NOP
	jump rel (delay)

	/* count the number of bits set in the SFBR.
	 * The number of bits set is returned in the SFBR.
	 */
count_bits:
	move 0 to SCRATCH0
	jump rel (bittest1), if not 1, and mask 0xfe
	move SCRATCH0 + 1 to SCRATCH0
bittest1:
	jump rel (bittest2), if not 2, and mask 0xfd
	move SCRATCH0 + 1 to SCRATCH0
bittest2:
	jump rel (bittest3), if not 4, and mask 0xfb
	move SCRATCH0 + 1 to SCRATCH0
bittest3:
	jump rel (bittest4), if not 8, and mask 0xf7
	move SCRATCH0 + 1 to SCRATCH0
bittest4:
	jump rel (bittest5), if not 0x10, and mask 0xef
	move SCRATCH0 + 1 to SCRATCH0
bittest5:
	jump rel (bittest6), if not 0x20, and mask 0xdf
	move SCRATCH0 + 1 to SCRATCH0
bittest6:
	jump rel (bittest7), if not 0x40, and mask 0xbf
	move SCRATCH0 + 1 to SCRATCH0
bittest7:
	jump rel (bittest_done), if not 0x80, and mask 0x7f
	move SCRATCH0 + 1 to SCRATCH0
bittest_done:
	move SCRATCH0 to SFBR
	return


	/* This very long routine does the tagged request cache lookup
	 * for a reconnecting tagged request.
	 * This really isn't a subroutine but is here because it's so
	 * long that I didn't want to put it in line.
	 * See the comment in scriptram.h for the cache algorithm.
	 *
	 * Right now this is only called by one place so the return address
	 * is known.
	 * The tid/lun/tag is in id_reselect.
	 * The SIOPJOB structure pointer is returned in SCRATCH0 on a hit,
	 * and 0 is returned on a miss.
	 *
	 * Id_reselect is used to get info about the reconnecting request:
	 *	bits 0-7:	target id
	 *	bits 8-15:	identify message (lun)
	 *	bits 16-23:	tag
	 * 
	 */

#define SHIFT	\
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13 ; \
	move SCRATCH1 + 0 to SCRATCH1

tag_cache_lookup:
	move memory SIZEOF_U32, SCRIPT_RAM_TAG_CACHE, SSIOP_REG_SCRATCH0
	move memory SIZEOF_U32, pc-8, cache_addr

	/* calculate the table address bits 8-12.
	 * ((tid<<10) + ((tag&3)<<8))
	 *	or
	 * ((tid<<2) + (tag &3)) 
	 * when doing byte operations on bits 8-15 of the address.
	 */
	move memory SIZEOF_U8, id_reselect, SSIOP_REG_SCRATCH0
	move SCRATCH0 to SFBR
	move SFBR to SCRATCH1
	/* (tid<<2) */
	SHIFT
	SHIFT

	/* tag & 3 */
	move memory SIZEOF_U8, id_reselect+2, SSIOP_REG_SCRATCH2
	move SCRATCH2 & 3 to SCRATCH2

	/* ((tid<<2) + (tag&3) */
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH2 | 0 to SCRATCH2
	move SCRATCH2 & 0x1f to SCRATCH2

	/* addr<15:8> + ((tid<<2) + (tag&3)) */
	move memory SIZEOF_U8, cache_addr+1, SSIOP_REG_SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, pc+13
	move SCRATCH2 + 0 to SCRATCH2
	move SCRATCH2 to SFBR
	move SFBR to SCRATCH1
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH1, cache_addr+1

	/* now get address bits 0-7.
	 *
	 *	tag & 0xfc
	 *
	 * since the lower two bits must be 0 since this will be a word
	 * address.
	 */
	move memory SIZEOF_U8, id_reselect+2, SSIOP_REG_SCRATCH2
	move SCRATCH2 & 0xfc to SFBR
	move SFBR to SCRATCH0
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, cache_addr

	/* the cache line address in now in cache_addr.
	 * Get the cache line.
	 */
	move memory SIZEOF_U32, cache_addr, pc+16
	move memory SIZEOF_U32, 0, cache_line

	/* make sure that the cache line is valid.
	 * Of course, this assumes that both the host address and tag
	 * are not 0.  Even if they are 0 (which is really improbable)
	 * then the worst that will happen is an extra RPC to the host.
	 */
	move memory SIZEOF_U32, cache_line, SSIOP_REG_SCRATCH0
	call rel (is_zero_32)
	jump rel (tcl_return), if 0

	/* now check the cache tag against the incoming tag.  Do a
	 * direct compare by modifying SCRIPTS instructions.
	 * The lower three bits of the cache line must be shifted
	 * 5 bits for comparison against the upper three bits
	 * of the tag.
	 *
	 *	if((siopjob.sj_lun & 7) == (cache_line&7))
	 *		return cache_line&~7;
	 *	else
	 *		return 0;
	 */
	move memory SIZEOF_U8, id_reselect+1, SSIOP_REG_SCRATCH1
	move SCRATCH0 & 7 to SCRATCH0
	move SCRATCH1 & 7 to SFBR
	move memory SIZEOF_U8, SSIOP_REG_SCRATCH0, pc+12
	jump rel (cache_miss), if not 0

cache_hit:
	move memory SIZEOF_U32, cache_line, SSIOP_REG_SCRATCH0
	move SCRATCH0 & 0xf8 to SCRATCH0
	jump rel (tcl_return)

cache_miss:
	move memory SIZEOF_U32, zero, SSIOP_REG_SCRATCH0
	jump rel (tcl_return)




/***************************************************************************
 *			DATA SPACE
 *	Put all initialized data here.
 ***************************************************************************/

ALIGN
/* the host physical address of the currently active job (pointer to the
 * active SIOPJOB structure in host memory.
 */
scsijobptr:
DD		0

/* current data pointers */
current_datai:
DD		0, 0
current_datap:
DD		0
current_offset:
DD		0

/* Always 0.  Used in zeroing memory */
zero:
DD		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

/* predefined outgoing messages */
msgreject:
DB		SCSI_MESSAGE_REJECT
msgabort:
DB		SCSI_ABORT
msg_device_reset:
DB		SCSI_BUS_DEVICE_RESET
msg_ide:
DB		SCSI_INITIATOR_DETECTED_ERROR
msg_mpe:
DB		SCSI_MESSAGE_PARITY_ERROR
msg_abort_tag:
DB		SCSI_ABORT_TAG
msg_parity:
DB		0			/* set to appropriate message type */
msgterminate:
DB		SCSI_TERMINATE_IO_PROCESS
msgnop:
DB		SCSI_NO_OPERATION

/* a place to put incoming bytes to be discarded */
discard:
DD		0, 0, 0, 0

/* Size byte of incoming extended message */
extended_msg_size:
DB		0

/* message type byte of incoming extended message */
extended_msg_type:
DB		0

/* extended message buffer */
extended_msg:
DD		0, 0, 0, 0

/* Used to store the reselection information for used in tagged request
 * lookup.  When the reselection sequence is complete this word
 * will contain the following:
 *	bits 0-7:	reconnecting target ID
 *	bits 8-15:	reselecting LUN (ID message byte)
 *	bits 16-23:	tag
 */
id_reselect:
DD		0
identify:
DB		0

/* incoming message byte buffer */
msgibuf:
DD		0

/* general scratch area */
temp:
DD		0

/* just a general error counter */
errcount:
DD		0

/* flag set to 1 when status byte is received, zeroed when job started */
status_received:
DD		0

/* Variable for tag cache lookup */
cache_addr:
DD		0
cache_line:
DD		0
