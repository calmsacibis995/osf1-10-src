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
static char *rcsid = "@(#)$RCSfile: sim_common.c,v $ $Revision: 1.1.17.2 $ (DEC) $Date: 1994/01/11 22:10:08 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_common.c		Version 1.15			Dec. 2, 1991

	The SIM Common source file (sim_common.c) will contain
	functions which are shared by other SIM modules.

Modification History:

	1.15	12/01/91	janet
	Modified to not attempt to break up long error logs.

	1.14	11/20/91	janet
	Modified the CAM_CALC_ENTRY_SZ() macro.

	1.13	11/13/91	janet
	Removed functional queues.  Added error logging functions.

	1.12	10/22/91	janet
	o Added the functions sc_free() and sc_alloc() for freeing
	  and allocating memory.
	o Replaced all PRINTD's with SIM_PRINTD's.
	o Added SIM_MODULE() to every function.
	o Make sure that autosense is not performed on the temporary
	  working set.

	1.11	09/19/91	janet
	Modified sc_get_ws, when working with a tag, to make sure that
	the tag was in use and that the target and lun match the SIM_WS.

	1.10	09/05/91	janet
	Modified sc_get_ws to use NEXUS ata

	1.09	07/26/91	janet
	Added functions and pointers reserved for debugging

	1.08	07/08/91	janet
	The function sc_lost_arb() was removed.

        1.07    06/28/91        rps
	Added new configuration code.

	1.06	06/20/91	janet
	Fixed sc_setup_ws() to setup it_nexus. Modified sc_lost_arb() to
	be called at high IPL.

	1.05	06/07/91	maria
	Added get_scsiid() routine.

	1.04	06/04/91	janet
	Ignore the return value from DME_END()

	1.03	05/29/91	janet
	sc_gen_msg() uses sync info kept in the IT_NEXUS.

	1.02	03/26/91	janet
	Updated after code review.

	1.01	02/12/91	janet
	Updated after debug.

	1.00	11/08/90	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */
/* Local defines.
 */
#define CAMERRLOG
    
/* ---------------------------------------------------------------------- */
/* Include files.
 */
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_all.h>
#include <dec/binlog/errlog.h>	/* UERF errlog defines */
#include <io/cam/cam_logger.h>
#include <vm/vm_kern.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/cam_errlog.h>

/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern void	ss_resched_request();
extern void	ss_finish();
extern U32	sim_init();
extern SIM_SOFTC *softc_directory[];

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
void		sc_gen_msg();
SIM_WS		*sc_find_ws();
void		sc_setup_ws();
void		sc_init_wsq();
void		sc_sel_timeout();
void		sim_logger();
void		sc_free();
char		*sc_alloc();
void		sc_logger();
void		sc_add_log_entry();
caddr_t 	cam_zget();
caddr_t		cam_zalloc();
static SIM_WS	*sc_sort_list();
static void (*local_errlog)() = sim_logger; 
    
/* ---------------------------------------------------------------------- */
/* Functions and pointers reserved for debugging
 */
SIM_SOFTC	*gl_sim_softc;
NEXUS		*gl_nexus;
IT_NEXUS	*gl_it_nexus;
SIM_SM		*gl_sim_sm;
SIM_SM_DATA	*gl_sim_sm_data;
SIM_WS		*gl_sws;
sim_break() {}

/*
 * Routine Name : sc_gen_msg
 *
 * Function Description :
 *	The function sc_gen_msg() will be passed a SIM_WS and a
 *	requested action.  A message sequence will then be created
 *	and stored in the message out queue of the SIM_WS.
 *
 * Call Syntax :
 *	sc_gen_msg(sws, action)
 *
 * Arguments:
 *	SIM_WS *sws;
 *	U32 action;
 *
 * Return Value :  none
 */
void
sc_gen_msg(sws, action)
register SIM_WS *sws;
U32 action;       /* type of message sequence to create */
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    u_char msg;
    SIM_MODULE(sc_gen_msg);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    switch(action) {

    case SC_MSG_START:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("SC_MSG_START\n"));

	/*
	 * SC_MSG_START.  Will set-up a sequence of messages to
	 * perform a SCSI "connection."  This connection may include:
	 * an identify message and a tag message sequence.
	 */

	/*
	 * Call sc_gen_msg() with an action of SC_MSG_IDENTIFY.
	 */
	sc_gen_msg(sws, SC_MSG_IDENTIFY);

	/*
	 * If SZ_TAGGED is set in the SIM_WS "flags" field, call
	 * sc_gen_msg() with an action of SC_MSG_TAG.
	 */
	if (sws->flags & SZ_TAGGED) {
	    sc_gen_msg(sws, SC_MSG_TAG);
	}
	break;

    case SC_MSG_IDENTIFY:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("SC_MSG_IDENTIFY\n"));

	/*
	 * SC_MSG_IDENTIFY.  Will set-up a one byte identify message.
	 * The following will be "or-ed" together to create the
	 * message:
	 *	SCSI_IDENTIFY
	 *	0x40 (if SZ_NO_DISCON is not set in the NEXUS "flags" field)
	 *	"lun"
	 */
	msg = SCSI_IDENTIFY;
	if (!(sws->nexus->flags & SZ_NO_DISCON)) {
	    msg |= 0x40;
	}
	msg |= sws->lun;
	SC_ADD_MSGOUT(sws, msg);
	break;

    case SC_MSG_SYNC: {

	u_char period, offset;
	
	/*
	 * Synchronous information is kept per target, not
	 * put LUN, so the nexus structure will always use
	 * a LUN of 0.
	 * Version 1.03
	 */

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("SC_MSG_SYNC\n"));

	/*
	 * SC_MSG_SYNC.  Will set-up a synchronous message negotiation
	 * sequence.  This call should be made after the device has
	 * gone to message out.  The following bytes will be put on
	 * the message out queue:
	 *	SCSI_EXTENDED_MESSAGE
	 *	0x03 (message length)
	 *	SCSI_SYNCHRONOUS_XFER
	 *	synchronous period
	 *	synchronous offset
	 */
	SC_ADD_MSGOUT(sws, SCSI_EXTENDED_MESSAGE);
	SC_ADD_MSGOUT(sws, 0x03);
	SC_ADD_MSGOUT(sws, SCSI_SYNCHRONOUS_XFER);

	/*
	 * If SZ_SYNC_NEEDED is set, use the period and offset from
	 * the SIM_SOFTC.  This means that we are initiating the
	 * negotiation.
	 *
	 * If SZ_SYNC_CLEAR is set, use a zero period and offset.
	 * This means that we are initiating the negotiation.
	 *
	 * If SZ_SYNC_NEG is set, use the period and offset stored in
	 * the IT_NEXUS.  This means that the target initiated the 
	 * negotiation.
	 */
	if (sws->it_nexus->flags & (SZ_SYNC_NEEDED | SZ_SYNC_CLEAR)) {
	    if (sws->it_nexus->flags & SZ_SYNC_CLEAR) {
		period = 0;
		offset = 0;
		sws->it_nexus->flags &= ~SZ_SYNC_CLEAR;
	    }
	    else {
		period = sc->sync_period;
		offset = sc->sync_offset;
	    }
	    sws->it_nexus->flags &= ~SZ_SYNC_NEEDED;
	    sws->it_nexus->flags |= SZ_SYNC_NEG;
	}

	/*
	 * Is this to be the final sync message?  If so, SZ_SYNC_NEG
	 * will be set.  Use the period and offset from the IT_NEXUS.
	 * Clear the SZ_SYNC_NEG flag and set the SZ_SYNC flag.
	 * After this message is sent, data phases should be running
	 * in synchronous.
	 */
	else if (sws->it_nexus->flags & SZ_SYNC_NEG) {
	    period = sws->it_nexus->sync_period;
	    offset = sws->it_nexus->sync_offset;
	    sws->it_nexus->flags &= ~SZ_SYNC_NEG;
	    sws->it_nexus->flags |= SZ_SYNC;
	}
	SC_ADD_MSGOUT(sws, period);
	SC_ADD_MSGOUT(sws, offset);
	break;
    }

    case SC_MSG_TAG:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("SC_MSG_TAG\n"));

	/*
	 * SC_MSG_TAG.  Will set-up a two byte tag message sequence.
	 * The following bytes will be put on the message out queue:
	 * the queue message code (taken from the "tag action"
	 * field of the CCB) and the tag (taken from the SIM_WS "tag"
	 * field).
	 *
	 * Make sure that the CCB isn't NULL.
	 */
	if (sws->ccb != (CCB_SCSIIO *)NULL) {
	    SC_ADD_MSGOUT(sws, sws->ccb->cam_tag_action);
	    SC_ADD_MSGOUT(sws, sws->tag);
	}
	break;

    default:
	/*
	 * Invalid action request.
	 */
	CAM_ERROR(module,
		  "Invalid action request",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sc_find_ws
 *
 * Function Description :
 *	The function sc_find_ws() will return the active SIM_WS
 *	which corresponds to the given target and lun.  This function
 *	should be called with SOFTC SMP locked.
 *
 * Call Syntax :
 *	sc_find_ws(sc, targid, lun)
 *
 * Arguments:
 *	SIM_SOFTC *sc
 *	U32 targid
 *	U32 lun
 *
 * Return Value :  Pointer to SIM_WS which was found.  If not found,
 *		   a pointer to the SIM_SOFTC's tmp_ws will be returned.
 */
SIM_WS *
sc_find_ws(sc, targid, lun, tag)
register SIM_SOFTC *sc;
U32 targid, lun;
I32 tag;
{
    register NEXUS *nexus = &sc->nexus[targid][lun];
    register SIM_WS *sws;
    SIM_MODULE(sc_find_ws);

    /*
     * If we were running untagged, then the SIM_WS should be
     * the first in the NEXUS queue.
     */
    if (nexus->flags & SZ_UNTAGGED) {

	/*
	 * If the NEXUS queue is empty, then there was a problem.
	 * Use the SIM_SOFTC tmp_ws.
	 */
	if (nexus->flink == (SIM_WS *)nexus) {
	    return(&sc->tmp_ws);
	}

	/*
	 * Check the "cam_status" of the SIM_WS.  It should be
	 * CAM_REQ_INPROG.  If it isn't then return a pointer
	 * to tmp_ws.
	 */
	if (nexus->flink->cam_status != CAM_REQ_INPROG) {
	    return(&sc->tmp_ws);
	}

	/*
	 * The first SIM_WS within the NEXUS queue is the one
	 * that we want.
	 */
	return(nexus->flink);
    }

    /*
     * If a valid tag was provided, use it to access the NEXUS's ata.
     */
    if ((tag > -1) && (tag < sc->max_tag)) {
	if (nexus->ata[tag].flags & SZ_TAG_ELEMENT_INUSE) {
	    sws = nexus->ata[tag].sim_ws;
	    if ((sws->targid == targid) && (sws->lun == lun))
		return(sws);
	}
    }

    /*
     * If the device is using tagged requests, check the value
     * of "tag."  If -1 then we can't find the SIM_WS, so return
     * the tmp_ws.
     */
    if (tag == -1) {
	return (&sc->tmp_ws);
    }

    /*
     * If we made it here, there was a problem.
     * Return a pointer to tmp_ws.
     */
    return(&sc->tmp_ws);
}

/*
 * Routine Name : sc_setup_ws
 *
 * Function Description :
 *	Sc_setup_ws() will initialize the given SIM_WS, setup
 *	its SIM_SOFTC and NEXUS pointers, and setup its target and lun.
 *
 * Call Syntax :
 *	sc_setup_ws(sc, sws, targid, lun)
 *
 * Arguments:
 *	SIM_SOFTC *sc
 *	SIM_WS *sws
 *	U32 targid
 *	U32 lun
 *
 * Return Value :  None
 */
void
sc_setup_ws(sc, sws, targid, lun)
SIM_SOFTC *sc;
SIM_WS *sws;
U32 targid, lun;
{
    SIM_MODULE(sc_setup_ws);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    bzero(sws, sizeof(SIM_WS));
    sc_init_wsq(sws);
    sws->nexus = &sc->nexus[targid][lun];
    sws->it_nexus = &sc->it_nexus[targid];
    sws->sim_sc = (void *)sc;
    sws->cntlr = sc->cntlr;
    sws->targid = targid;
    sws->lun = lun;

    /*
     * Make sure that autosense is NOT performed on this working set.
     */
    sws->cam_flags |= CAM_DIS_AUTOSENSE;

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sc_init_wsq
 *
 * Function Description :
 *	Sc_init_wsq() will initialize the SIM_WS's circular queues.
 *
 * Call Syntax :
 *	sc_init_wsq(sws)
 *
 * Arguments:
 *	SIM_WS *sws
 *
 * Return Value :  None
 */
void
sc_init_wsq(sws)
SIM_WS *sws;
{
    SIM_MODULE(sc_init_wsq);
    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    CIRQ_INIT_Q(sws->msgoutq);
    CIRQ_INIT_Q(sws->msginq);
    CIRQ_INIT_Q(sws->phaseq);

    CIRQ_SET_DATA_SZ(sws->msgoutq, SIM_MAX_MSGOQ);
    CIRQ_SET_DATA_SZ(sws->msginq, SIM_MAX_MSGIQ);
    CIRQ_SET_DATA_SZ(sws->phaseq, SIM_MAX_PHASEQ);

    CIRQ_USE_SEQ(sws->msgoutq);

#ifdef SIM_FUNCQ_ON
    CIRQ_INIT_Q(sws->funcq);
    CIRQ_SET_DATA_SZ(sws->funcq, SIM_MAX_FUNCQ);
#endif

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name :  sc_sel_timeout
 *
 * Functional Description :
 *	Sc_sel_timeout() is called when a selection timeout is
 *	detected.  It will set the "cam_status" of the SIM_WS
 *	to CAM_SEL_TIMEOUT.
 * 
 * Call Syntax:
 *	sc_sel_timeout(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sc_sel_timeout(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sc_sel_timeout);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));
    /*
     * Set CAM status to CAM_SEL_TIMEOUT.
     */
    sws->cam_status = CAM_SEL_TIMEOUT;

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name :  sc_logger
 *
 * Functional Description :
 *	This function logs the common SIM structures.  It is recursive.
 * 
 * Call Syntax:
 *	sc_logger(hdr, size, sc, sws, flags)
 *
 * Arguments:
 *	CAM_ERR_HDR *hdr	Pointer to log header, already filled in
 *	U32 size		Number of entrie space in the log
 *	SIM_SOFTC *sc		SIM_SOFTC pointer, may be NULL
 *	SIM_WS *sws		SIM_WS pointer, may be NULL
 *	U32 flags		flags described in sim.h
 *
 * Return Value :  None
 */
void
sc_logger(hdr, size, sc, sws, flags)
CAM_ERR_HDR *hdr;
U32 size;
SIM_SOFTC *sc;
SIM_WS *sws;
U32 flags;
{
    SIM_SOFTC *tsc;
    NEXUS *tnexus;
    IT_NEXUS *titn;
    SIM_WS *tsws;
    DME_STRUCT *tdme;
    TAG_ELEMENT *tata;
    extern U32 sm_queue_sz;
    extern SIM_SM_DATA sm_data[];
    extern SIM_SM sim_sm;
    int i;
    SIM_MODULE(sc_logger);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * Log the SIM_WS structure which was passed in as an argument.
     */
    if (flags & SIM_LOG_SIM_WS) {
	if (sws != (SIM_WS *)NULL) {

	    /*
	     * Log the SIM_WS.
	     */
	    sc_add_log_entry(hdr, size, sc, sws, (U32)ENT_SIM_WS,
			     (U32)sizeof(SIM_WS), (U32)SIM_WS_VERS,
			     (u_char *)sws, (U32)PRI_FULL_REPORT);

	    /*
	     * If SIM_LOG_FOLLOW_LINKS is set, follow the links
	     * for linked commands.
	     */
	    if (flags & SIM_LOG_FOLLOW_LINKS) {
		sc_logger(hdr, size, sc, sws->linked_ws,
			  SIM_LOG_SIM_WS | SIM_LOG_FOLLOW_LINKS |
			  SIM_LOG_NOLOG);
	    }
	}
	flags &= ~SIM_LOG_SIM_WS;
    }

    /*
     * Log the NEXUS structures.
     */
    if (flags & SIM_LOG_NEXUS) {
	tnexus = (NEXUS *)NULL;
	if (sws != (SIM_WS *)NULL)
	    tnexus = sws->nexus;

	if (tnexus != (NEXUS *)NULL) {
	    /*
	     * Log the NEXUS.
	     */
	    sc_add_log_entry(hdr, size, sc, sws, (U32)ENT_NEXUS,
			     (U32)sizeof(NEXUS), (U32)NEXUS_VERS,
			     (u_char *)tnexus, (U32)PRI_FULL_REPORT);

	    /*
	     * If SIM_LOG_FOLLOW_LINKS is set, follow the SIM_WS
	     * linked list.
	     */
	    if (flags & SIM_LOG_FOLLOW_LINKS) {
		for (tsws = tnexus->flink; tsws != (SIM_WS *)tnexus;
		     tsws = tsws->flink)
		    sc_logger(hdr, size, sc, tsws,
			      SIM_LOG_SIM_WS | SIM_LOG_FOLLOW_LINKS |
			      SIM_LOG_NOLOG);
	    }

	    /*
	     * Do we need to log the TAG_ELEMENT array for this NEXUS?
	     */
	    if (flags & SIM_LOG_TAG_ELEMENT) {
		sc_logger(hdr, size, sc, sws,
			  SIM_LOG_TAG_ELEMENT | SIM_LOG_NOLOG);
		flags &= ~SIM_LOG_TAG_ELEMENT;
	    }
	}
	flags &= ~SIM_LOG_NEXUS;
    }

    /*
     * Log the IT_NEXUS structures.
     */
    if (flags & SIM_LOG_IT_NEXUS) {
	titn = (IT_NEXUS *)NULL;
	if (sws != (SIM_WS *)NULL)
	    titn = sws->it_nexus;

	if (titn != (IT_NEXUS *)NULL) {
	    /*
	     * Log the IT_NEXUS.
	     */
	    sc_add_log_entry(hdr, size, sc, sws, (U32)ENT_IT_NEXUS,
			     (U32)sizeof(IT_NEXUS),
			     (U32)IT_NEXUS_VERS,
			     (u_char *)titn, (U32)PRI_FULL_REPORT);
	}
	flags &= ~SIM_LOG_IT_NEXUS;
    }

    /*
     * Log the SIM_SOFTC structures.
     */
    if (flags & SIM_LOG_SIM_SOFTC) {
	tsc = (SIM_SOFTC *)NULL;
	if (sws != (SIM_WS *)NULL)
	    tsc = sws->sim_sc;
	else
	    tsc = sc;
	
	if (tsc != (SIM_SOFTC *)NULL) {
	    /*
	     * Log the SIM_SOFTC.
	     */
	    sc_add_log_entry(hdr, size, sc, sws, (U32)ENT_SIM_SOFTC,
			     (U32)sizeof(SIM_SOFTC),
			     (U32)SIM_SOFTC_VERS,
			     (u_char *)tsc, (U32)PRI_FULL_REPORT);

	    /*
	     * Do we need to log the TAG_ELEMENT array for the SIM_SOFTC?
	     */
	    if (flags & SIM_LOG_TAG_ELEMENT) {
		sc_logger(hdr, size, sc, sws,
			  SIM_LOG_TAG_ELEMENT | SIM_LOG_NOLOG);
		flags &= ~SIM_LOG_TAG_ELEMENT;
	    }

	    /*
	     * Do we need to log the DME_STRUCT for the SIM_SOFTC?
	     */
	    if (flags & SIM_LOG_DME_STRUCT) {
		sc_logger(hdr, size, sc, sws,
			  SIM_LOG_DME_STRUCT | SIM_LOG_NOLOG);
		flags &= ~SIM_LOG_DME_STRUCT;
	    }
	}
	flags &= ~SIM_LOG_SIM_SOFTC;
    }

    /*
     * Log the TAG_ELEMENT array.
     */
    if (flags & SIM_LOG_TAG_ELEMENT) {
	tsc = (SIM_SOFTC *)NULL;
	tata = (TAG_ELEMENT *)NULL;
	if (sc != (SIM_SOFTC *)NULL) {
	    tsc = sc;
	    tata = sc->ata;
	}
	else if (sws != (SIM_WS *)NULL)
	    if (sws->sim_sc != (SIM_SOFTC *)NULL) {
		tsc = sws->sim_sc;
		tata = sws->sim_sc->ata;
	    }

	if ((tsc != (SIM_SOFTC *)NULL) && (tata != (TAG_ELEMENT *)NULL)) {
	    for (i=0; i < tsc->max_tag; i++) {
		    
		/*
		 * Log each TAG_ELEMENT structure.
		 */
		sc_add_log_entry(hdr, size, sc, sws,
				 (U32)ENT_TAG_ELEMENT,
				 (U32)sizeof(TAG_ELEMENT),
				 (U32)TAG_ELEMENT_VERS,
				 (u_char *)&tata[i],
				 (U32)PRI_FULL_REPORT);
	    }
	}
	flags &= ~SIM_LOG_TAG_ELEMENT;
    }

    /*
     * Log the DME_STRUCT.
     */
    if (flags & SIM_LOG_DME_STRUCT) {
	tdme = (DME_STRUCT *)NULL;
	if (sc != (SIM_SOFTC *)NULL)
	    tdme = sc->dme;
	else if (sws != (SIM_WS *)NULL)
	    if (sws->sim_sc != (SIM_SOFTC *)NULL)
		tdme = sws->sim_sc->dme;
	if (tdme != (DME_STRUCT *)NULL) {

	    /*
	     * Log the DME_STRUCT structure.
	     */
	    sc_add_log_entry(hdr, size, sc, sws,
			     (U32)ENT_DME_STRUCT,
			     (U32)sizeof(DME_STRUCT),
			     (U32)DME_STRUCT_VERS,
			     (u_char *)tdme,
			     (U32)PRI_FULL_REPORT);
	}
	flags &= ~SIM_LOG_DME_STRUCT;
    }

    /*
     * Do we log the SIM_SM_DATA array or the SIM_SM struct?
     */
    if (flags & (SIM_LOG_SIM_SM_DATA | SIM_LOG_SIM_SM)) {

	/*
	 * If SIM_LOG_SIM_SM or if SIM_LOG_SIM_SM_DATA is set,
	 * always log the SIM_SM struct.
	 */
	sc_add_log_entry(hdr, size, sc, sws,
			 (U32)ENT_SIM_SM,
			 (U32)sizeof(SIM_SM),
			 (U32)SIM_SM_VERS,
			 (u_char *)&sim_sm,
			 (U32)PRI_FULL_REPORT);
	flags &= ~SIM_LOG_SIM_SM;

	/*
	 * If SIM_LOG_SIM_SM_DATA is set, log the sm_data array.
	 */
	if (flags & SIM_LOG_SIM_SM_DATA) {
	    sc_add_log_entry(hdr, size, sc, sws,
			     (U32)ENT_SIM_SM_DATA,
			     (U32)sm_queue_sz * sizeof(SIM_SM_DATA),
			     (U32)SIM_SM_DATA_VERS,
			     (u_char *)sm_data,
			     (U32)PRI_FULL_REPORT);
	    flags &= ~SIM_LOG_SIM_SM_DATA;
	}
    }

    /*
     * Call the CAM error logger.
     */
    if (!(flags & SIM_LOG_NOLOG)) {
	sim_call_cam_logger(hdr, sc, sws);
    }

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

sim_call_cam_logger(hdr, sc, sws)
CAM_ERR_HDR *hdr;
SIM_SOFTC *sc;
SIM_WS *sws;
{
    char bus = -1, targid = -1, lun = -1;

    if (sws != (SIM_WS *)NULL) {
	bus = (char)sws->cntlr;
	targid = (char)sws->targid;
	lun = (char)sws->lun;
    }
    else if (sc != (SIM_SOFTC *)NULL)
	bus = (char)sc->cntlr;

    cam_logger(hdr, bus, targid, lun);
}

void
sc_add_log_entry(hdr, log_size, sc, sws, type, size, vers, data, pri)
CAM_ERR_HDR *hdr;
U32 log_size;
SIM_SOFTC *sc;
SIM_WS *sws;
U32 type;
U32 size;
U32 vers;
u_char *data;
U32 pri;
{
    CAM_ERR_ENTRY *entry;

    /*
     * Will this entry fit in the list?
     */
    if (hdr->hdr_entries >= log_size) {
	char *msg = "CAM error log continued...";

	/*
	 * If not, call cam_logger to log the current
	 * header contents.
	 */
	sim_call_cam_logger(hdr, sc, sws);

	/*
	 * Now, set the count back to two (module and message)
	 * and set the size to zero.
	 */
	hdr->hdr_entries = 2;
	hdr->hdr_size = 0;

	/*
	 * Change the log string to be "CAM error log continued..."
	 */
	hdr->hdr_list[1].ent_size = strlen(msg) + 1;
	hdr->hdr_list[1].ent_data = (u_char *) msg;
    }

    /*
     * Add the entry.
     */
    entry = &hdr->hdr_list[hdr->hdr_entries++];
    entry->ent_type = type;
    entry->ent_size = size;
    entry->ent_vers = vers;
    entry->ent_data = data;
    entry->ent_pri = pri;
}

/*
 * Routine Name :  sim_logger
 *
 * Functional Description :
 *	This function is the common error logging function for the
 *	sim modules.
 * 
 * Call Syntax:
 *	sim_logger(func, msg, sc, sws, unused, flags)
 *
 * Arguments:
 *	u_char *func	calling module name, may be NULL
 *	u_char *msg	error description, may be NULL
 *	SIM_SOFTC *sc	SIM_SOFTC pointer, may be NULL
 *	SIM_WS *sws	SIM_WS pointer, may be NULL
 *	U32 flags	flags described in sim.h
 *
 * Return Value : None
 */
void
sim_logger(func, msg, flags, sc, sws, unused)
u_char *func;
u_char *msg;
U32 flags;
SIM_SOFTC *sc;
SIM_WS *sws;
U32 unused;
{
    SIM_MODULE(sim_logger);
    static CAM_ERR_HDR hdr;
    static CAM_ERR_ENTRY entrys[SIM_LOG_SIZE];

    hdr.hdr_type = CAM_ERR_PKT;
    hdr.hdr_size = 0;
    hdr.hdr_class = CLASS_DEC_SIM;
    hdr.hdr_subsystem = SUBSYS_DEC_SIM;
    hdr.hdr_entries = 0;
    hdr.hdr_list = entrys;
    if (flags & SIM_LOG_PRISEVERE)
	hdr.hdr_pri = EL_PRISEVERE;
    else if (flags & SIM_LOG_PRIHIGH)
	hdr.hdr_pri = EL_PRIHIGH;
    else
	hdr.hdr_pri = EL_PRILOW;

    /*
     * Log the module name.
     */
    if (func != (u_char *)NULL) {
	sc_add_log_entry(&hdr, (I32)SIM_LOG_SIZE, sc, sws,
			 (U32)ENT_STR_MODULE,
			 (U32)strlen(func) + 1,
			 (U32)0,
			 (u_char *)func,
			 (U32)PRI_BRIEF_REPORT);
    }

    /*
     * Log the message.
     */
    if (msg != (u_char *)NULL) {
	sc_add_log_entry(&hdr, (I32)SIM_LOG_SIZE, sc, sws,
			 (U32)ENT_STRING,
			 (U32)strlen(msg) + 1,
			 (U32)0,
			 (u_char *)msg,
			 (U32)PRI_BRIEF_REPORT);
    }

    /*
     * Call sc_logger to log the common structures.
     */
    sc_logger(&hdr, SIM_LOG_SIZE, sc, sws, flags);
}

extern int default_scsiid;
#define CNTLR_INDEX	6	/* loc of controller char in the string */
#define ASCII_0		0x30	/* add to binary # to get ACSII equivilent */

/*
 * Routine Name :  get_scsiid
 *
 * Functional Description :
 *	This function is needed to determine the SCSC ID of the
 *	processor.
 * 
 * Call Syntax:
 *	get_scsiid(cntlr)
 *
 * Arguments:
 *	ctlr - SCSI bus number.
 *
 * Return Value :  Processor target id.
 */

int 
get_scsiid( cntlr )
int cntlr;			/* controller/bus number on the system */
{
    char *env;			/* ptr for the NVR string */
    int nvr_id;			/* converted ID # from NVR */
    char *idstring = "scsiid?";	/* string to search for */
    SIM_MODULE(get_scsiid);

    /*
     * Build an id string from our controller #, i.e., scsiid0, 1, etc.  
     * The ID string can be reused. 
     */

    idstring[ CNTLR_INDEX ] = (char)((cntlr & 0xff) + ASCII_0);

    env = (char *)prom_getenv( idstring );

    if (env != NULL) {
	nvr_id = xtob(env);		/* convert ACSII hex to binary */

	/* Is the ID a valid #, ID's on the SCSI bus can only be [0-7]. */
	if ((nvr_id >= 0) && (nvr_id <= 7)) {
	    return( nvr_id );
	}
    }
    
    /* 
     * The SCSI bus ID conversion failed, return the default value to be used
     * for this controller. 
     */
    return( default_scsiid );		/* return the default */
}

/*
 * Routine Name :  sc_alloc
 *
 * Functional Description :
 *	This function is used to allocate memory.
 * 
 * Call Syntax:
 *	p = sc_alloc(size)
 *
 * Arguments:
 *	size - number of byte to allocate.
 *
 * Return Value :  char pointer to the memory.
 */
char *
sc_alloc(size)
int size;
{
    char *c;
    SIM_MODULE(sc_alloc);

    c = (char *)cam_zalloc(size);

    return(c);
}

/*
 * Routine Name :  sc_free
 *
 * Functional Description :
 *	This function is used to free memory allocated with sc_alloc().
 * 
 * Call Syntax:
 *	sc_free(p, size)
 *
 * Arguments:
 *	p - char pointer to buffer.
 *	size - size of buffer.
 *
 * Return Value :  none.
 */
void
sc_free(c, size)
char 	*c;
u_int	size;
{
    SIM_MODULE(sc_free);

    cam_zfree((char *)c,size);
}

/*
 * Macro: SC_GET_TIME_SEC
 *
 * Description:
 * 	This macro is used to return the current time in seconds.
 *	The time structure is a global kernel structure.
 */
I32
sc_get_time_sec()
{
    struct timeval tv;
    SIM_MODULE(sc_get_time_sec);
    microtime(&tv);
    return (tv.tv_sec);
}
U32
sc_get_time_usec()
{
    struct timeval tv;
    U32 usec;
    SIM_MODULE(sc_get_time_usec);
    microtime(&tv);
    usec = tv.tv_sec * 1000000;
    usec += tv.tv_usec;
    return (usec);
}

/* 
 * CAM memory zone code
 * Temp. If we want to use zones
 * The zones can be changed to would ever sizes is best.
 */
#define CAM_ZONE_MAX 6
struct zone *cam_zone[CAM_ZONE_MAX];
static char *cam_zone_name[CAM_ZONE_MAX] = {
	"camzone.64",
	"camzone.128",
	"camzone.512",
	"camzone.1024",
	"camzone.2048",
	"camzone.4096",
};
unsigned I32 cam_zone_size[CAM_ZONE_MAX] = {
      64,		/*     64 Byte  */
      128,		/*    128 Byte  */
      512,		/*    512 Byte  */
      1024,		/*   1024 Byte  */
      2048,		/*   2048 Byte  */
      4096,		/*   4096 Byte  */
};
unsigned I32 cam_zone_max[CAM_ZONE_MAX] = {
      4096,		/*     64 Byte  */
      4096,		/*    128 Byte  */
      4096,		/*    512 Byte  */
      4096,	        /*   1024 Byte  */
      4096,		/*   2048 Byte  */
      4096,		/*   4096 Byte  */
};

int cam_zones_initialized = 0;

cam_zones_init()
{
    int 		i, size;
    vm_offset_t 	cam_zone_addr;

    if (cam_zones_initialized)
	return;

    cam_zones_initialized++;

    for( i = 0; i < CAM_ZONE_MAX; ++i ) {
	cam_zone[i] = zinit( cam_zone_size[i], 
			     cam_zone_max[i] * cam_zone_size[i],
			     PAGE_SIZE, 
			     cam_zone_name[i]);
/*
** This call was removed to enable use of large configurations on 
** Ruby systems.  The settings in this call to zchange are the same
** as the defaults set via zinit except for argument 4, the
** "exhaustible" boolean.  Set to true means that when the max
** size of the zone is reached, zalloc returns NULL.  This causes
** havoc in the memory allocation routines in xpt.c and causes the
** xpt_ccb_alloc routine to return a NULL pointer.
**
** Returning NULL from xpt_ccb_alloc results in a system panic because
** routines like ccmn_get_ccb() don't check the return value and cause
** a memory management exception upon referencing the NULL ccb pointer.
**
** Removing this call gives the zone the default behavior, which is to 
** expand the zone if the "max" size, set via the zinit call above, 
** is reached.
*/
/*	zchange( cam_zone[i],FALSE,FALSE,TRUE,TRUE); */
    }
}

caddr_t
cam_zalloc(size)
I32 		size;
{
    int 		zindex;
    caddr_t 		addr;

    if (!cam_zones_initialized) cam_zones_init();

	/* compute the size of the block that we will actually allocate */

    for( zindex = 0; zindex < CAM_ZONE_MAX; ++zindex ) {
	if( size <= cam_zone_size[zindex] ) {
	    break;
	}
    }

	/*
	 * If our size is still small enough, check the queue for that size
	 * and allocate.
	 */

    if ((size < PAGE_SIZE) && (zindex < CAM_ZONE_MAX)) {
	addr = (caddr_t) zalloc(cam_zone[zindex]);
    } else {
	addr = (caddr_t) kmem_alloc(kernel_map, size);
    }

    if (addr != (caddr_t)NULL)
	bzero(addr, size);

    return(addr);
}

caddr_t
cam_zget(size)
    I32 		size;
{
    int 		zindex;
    caddr_t 		addr;

    if (!cam_zones_initialized) cam_zones_init();

    /* compute the size of the block that we will actually allocate */

    for( zindex = 0; zindex < CAM_ZONE_MAX; ++zindex ) {
	if( size <= cam_zone_size[zindex] ) {
	    break;
	}
    }

    /*
     * If our size is still small enough, check the queue for that size
     * and allocate.
     */

    if (size < PAGE_SIZE) {
	addr = (caddr_t) zget(cam_zone[zindex]);
    } else {
	addr = (caddr_t) kmem_alloc(kernel_map, size);
	/* This will never work, so we might as well panic 
	panic("cam_get: oversized request");
	*/
    }

    if (addr == (caddr_t)NULL) panic("cam_zget:  No space left");

    if (addr != (caddr_t)NULL)
	bzero(addr, size);

    return(addr);
}

cam_zfree(data, size)
    caddr_t		data;
    I32		size;
{
    int 		zindex;

    for( zindex = 0; zindex < CAM_ZONE_MAX; ++zindex ) {
	if( size <= cam_zone_size[zindex] ) {
	    break;
	}
    }
    if ((size < PAGE_SIZE) && (zindex < CAM_ZONE_MAX)) {
        zfree(cam_zone[zindex], data);
    } else {
        kmem_free(kernel_map, data, size);
    }
}

/*
 * Routine Name :  sc_sws_sort
 *
 * Functional Description :
 *
 *	This function will sort the SIM_WS's in the NEXUS list based
 *	on the "cam_sort" value.
 *
 *	The sorting algrithim is a basic C-scan.  SIM_WS's with a
 *	cam_sort value of 0 are considered to be ordered and no
 *	inserting or sorting will be performed in front of them.
 *
 *	This function is only called if the PDRV has registered
 *	for sorting on the NEXUS of this SIM_WS.
 *
 *	This function keeps two pointers into the NEXUS list, the
 *	curr_list and the next_list.
 *
 *	The "curr_list" is the active list.  The first SIM_WS in
 *	this list has been sent to the device and is considered active.
 *
 *	The "next_list" is being built for the next pass on the device.
 *	If a SIM_WS can't be added to the "curr_list" then it will
 *	be added to the "next_list."
 *
 *	SIM_WS's are added to the end of the NEXUS list unsorted
 *	until the list contains sim_min_sort_depth entries.  Once
 *	this has occured the list will be sorted.  After the list
 *	has been sorted and as long as the list contains at
 *	least sim_min_sort_depth SIM_WS's, SIM_WS will be inserted
 *	into this list in sorted order.
 *	
 * 
 * Call Syntax:
 *	sc_sws_sort(sws)
 *
 * Arguments:
 *	sws	- SIM_WS to be sorted
 *
 * Return Value :  none.
 */
U32
sc_sws_sort(sws)
SIM_WS *sws;
{
    SIM_WS *tsws;
    NEXUS *nexus = sws->nexus;
    extern U32 sim_sort_age_time;
    extern U32 sim_allow_io_sorting;
    extern U32 sim_allow_io_priority_sorting;
    extern U32 sim_min_sort_depth;
    int i;
    SIM_MODULE(sc_sws_sort);

    /*
     * Has I/O sorting been disabled?
     */
    if (!sim_allow_io_sorting)
        sws->cam_sort = 0;

    /*
     * Has I/O priority sorting been disabled?
     */
    if (!sim_allow_io_priority_sorting)
        sws->cam_priority = 0;

    /*
     * Should this SIM_WS be sorted?  If "cam_sort" 
     * and "cam_priority" are zero then don't sort it.
     */
    if (!sws->cam_sort && !sws->cam_priority) {
	
	/*
	 * A non-sorted SIM_WS is considered to be ordered, therefore
	 * the SIM_WS will be added to the end of the NEXUS queue and
	 * no further sorting will be performed in front of this SIM_WS.
	 */
	SC_WS_INSERT((void *)sws, sws->nexus->blink);
	nexus->curr_list = (SIM_WS *)NULL;
	nexus->next_list = (SIM_WS *)NULL;
	nexus->curr_cnt = 0;
	nexus->next_cnt = 0;
	return;
    }

    /*
     * Assign a time stamp to the SIM_WS.
     */
    sws->time_stamp = sc_get_time_usec();

    /*
     * Determine if any SIM_WS's in the curr_list or in the next_list
     * have been waiting too long.  If so, don't add any more I/O
     * in front of them.  "sim_sort_age_time" is the maximum number
     * of microseconds that an I/O will be allowed to wait.
     */
    if (nexus->curr_cnt && ((sws->time_stamp - nexus->curr_time) >
                                                    sim_sort_age_time)) {
        nexus->curr_list = (SIM_WS *)NULL;
	nexus->curr_cnt = 0;
    }
    if (nexus->next_cnt && ((sws->time_stamp - nexus->next_time) >
                                                    sim_sort_age_time)) {
	nexus->curr_list = (SIM_WS *)NULL;
	nexus->curr_cnt = 0;
	nexus->next_list = (SIM_WS *)NULL;
	nexus->next_cnt = 0;
    }

    /*
     * At this point the SIM_WS must have a non-zero "cam_sort" field.
     * Therefore this SIM_WS is to be sorted.
     *
     * Determine if this SIM_WS can be added to the "curr_list."
     * If the "curr_list" contains less than sim_min_sort_depth
     * entries then it can't be added to the "curr_list."
     */
    if (nexus->curr_cnt >= sim_min_sort_depth) {

	/*
	 * The "curr_list" must be sorted.  Determine if this SIM_WS
	 * can be added to the "curr_list."  It can be added to
	 * the current list if it's "cam_sort" value is greater or
	 * equal to that of the head of the "curr_list."
         *
         * It can also be added to the list if the priority is
         * less than or greater than that of the head of the
         * current list.
	 */
	if ((nexus->curr_list->cam_sort <= sws->cam_sort) ||
            (nexus->curr_list->cam_priority != sws->cam_priority)) {

	    /*
	     * Go through the "curr_list" looking for a SIM_WS which
	     * has a "cam_sort" field greater than that of the incoming
	     * SIM_WS.
	     *
 	     * Since the head of the current list is always active,
	     * start with the next entry in the list to prevent any
	     * insertions in front of the head.
             *
	     * Also sort on the cam_priority.  The higher the priority
	     * the closer to the head.
	     */
	    for (i=1, tsws = nexus->curr_list->flink;
		 (i < nexus->curr_cnt) &&
                 (sws->cam_priority < tsws->cam_priority);
		 i++, tsws = tsws->flink);
            for (; (i < nexus->curr_cnt) &&
                 (sws->cam_sort >= tsws->cam_sort) &&
                 (sws->cam_priority == tsws->cam_priority);
                 i++, tsws = tsws->flink);

	    /*
	     * Insert the SIM_WS in front of tsws.
	     */
	    SC_WS_INSERT(sws, tsws->blink);

	    /*
	     * Increment the count of SIM_WS's in the current list.
	     */
	    nexus->curr_cnt++;

	    /*
	     * The SIM_WS has been inserted in the curr_list.
	     */
	    return;
	}
    }

    /*
     * The SIM_WS wasn't added to the curr_list it will therefore
     * be added to the next list.
     *
     * If the next_list is empty add it to the end of the NEXUS
     * list and initialize the "next_list."
     */
    if (nexus->next_cnt == 0) {

	SC_WS_INSERT(sws, nexus->blink);
	nexus->next_list = sws;
	nexus->next_cnt++;
	
	/*
	 * Update the "next_list"'s time stamp with that of
	 * SIM_WS.
	 */
        nexus->next_time = sws->time_stamp;

	/*
	 * The SIM_WS has been added to the next list.
	 */
	return;
    }

    /*
     * The next list already has SIM_WS's in it.  If it contains
     * less than sim_min_sort_depth, then the list is unsorted.
     * Just add the SIM_WS to the end of the NEXUS list.
     */
    if (nexus->next_cnt < sim_min_sort_depth) {

	/*
	 * Insert the SIM_WS at the end of the list.
	 */
	SC_WS_INSERT(sws, nexus->blink);
	
	/*
	 * If we reach our unsorted limit, sort the list.
	 */
	if (++nexus->next_cnt >= sim_min_sort_depth) {

	    nexus->next_list = sc_sort_list(nexus->next_list,
					    nexus->next_cnt);
	}

	/*
	 * The SIM_WS has been added to the next list.
	 */
	return;
    }

    /*
     * The next list has already been sorted.  Insert this SIM_WS
     * into the "next_list" in sorted order.
     *
     * Go through the "curr_list" looking for a SIM_WS which
     * has a "cam_sort" field greater than that of the incoming
     * SIM_WS.
     *
     * Also check the cam_priority field.  The higher the priority
     * the closer to the head of the list.
     */
    for (i=0, tsws = nexus->next_list;
	 (i < nexus->next_cnt) &&
         (sws->cam_priority < tsws->cam_priority);
	 i++, tsws = tsws->flink);
    for (; (i < nexus->next_cnt) &&
         (sws->cam_sort >= tsws->cam_sort) &&
         (sws->cam_priority == tsws->cam_priority);
         i++, tsws = tsws->flink);

    /*
     * Add the SIM_WS in front of tsws.
     */
    SC_WS_INSERT(sws, tsws->blink);
    if (tsws == nexus->next_list)
	nexus->next_list = sws;
    
    /*
     * Increment the count of SIM_WS's in the "next_list."
     */
    nexus->next_cnt++;
}
     
/*
 * Routine Name :  sc_sort_list
 *
 * Functional Description :
 *
 *	This funtion is used to sort a unsorted list.  It
 *	uses the basic bubble-sort algrithim.
 *
 * Call Syntax:
 *	sc_sort_list(hd, cnt)
 *
 * Arguments:
 *	SIM_WS *hd	Specifies the head of the list.
 *	U32 cnt		Specifies the number of entries in the list.
 *
 * Return Value :  SIM_WS *, head of the sorted list
 */
static SIM_WS *
sc_sort_list(hd, cnt)
SIM_WS *hd;
U32 cnt;
{
    SIM_WS *t, *t1, *t2;
    U32 i, j;

    /*
     * This is a bubble sort.  Start with the first entry
     * and compare it with the rest of the list.
     * Then move on to the second entry and compare it with
     * the rest of the list...
     */
    for (i=0, t1=hd; i < (cnt - 1); i++, t1=t1->flink) {

	for (j=i, t2=t1->flink; j < (cnt - 1); j++, t2=t2->flink) {

	    /*
	     * Do the SIM_WS's need to be swapped?
	     */
  	    if ((t1->cam_priority < t2->cam_priority) ||
                ((t1->cam_priority == t2->cam_priority) &&
                 (t1->cam_sort > t2->cam_sort))) {

		/*
		 * Remove the smaller SIM_WS.
		 */
                t = t2->blink;
		SC_WS_REMOVE(t2);

		/*
		 * Insert it in front of the larger one.
		 */
		SC_WS_INSERT(t2, t1->blink);

		/*
		 * Update the t1 and t2 pointers.
		 */
		t1 = t2;
		t2 = t;
	    }
	}

	/*
	 * The hd could have changed on the first time through.
	 * Check for this.
	 */
	if (i == 0)
	    hd = t1;
    }

    /*
     * Return the head of the list.
     */
    return(hd);
}

sc_set_curr_age_time(n)
NEXUS *n;
{
    U32 i;
    U32 time = 0xffffffff;
    SIM_WS *sws = n->curr_list;

    for (i=0; i < n->curr_cnt; i++) {
        time = SC_GET_MIN(time, sws->time_stamp);
        sws = sws->flink;
    }

    n->curr_time = time;
}

sc_set_next_age_time(n)
NEXUS *n;
{
    U32 i;
    U32 time = 0xffffffff;
    SIM_WS *sws = n->next_list;

    for (i=0; i < n->next_cnt; i++) {
        time = SC_GET_MIN(time, sws->time_stamp);
        sws = sws->flink;
    }

    n->next_time = time;
}
