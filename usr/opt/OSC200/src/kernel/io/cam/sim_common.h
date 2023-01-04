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
/*
 * @(#)$RCSfile: sim_common.h,v $ $Revision: 1.1.10.5 $ (DEC) $Date: 1993/10/06 13:44:54 $
 */
#ifndef _SIM_COMM_
#define _SIM_COMM_

/* ---------------------------------------------------------------------- */

/* sim_common.h		Version 1.11			Nov. 20, 1991 */

/*  This file contains the definitions common to all the SIM related
    source files.

Modification History

	1.11	11/20/91	janet
	Modified SC_ENABLE_TMO to use sim_default_timeout.

	1.10	11/15/91	janet
	"sim_module()" is always defined.

	1.09	11/13/91	janet
	Removed functional queue macro.

	1.08	10/22/91	janet
	o Added SIM_MODULE() macro.
	o Added SIM_PRINTD() macro.
	o Replaced all PRINTD's with SIM_PRINTD's.
	o Added SC_UPDATE_MSGIN() macro for target mode support.
	o Added SC_ADD_FUNC() macro for accessing the functional queue.

	1.07 	09/11/91	janet
	Modified SC_ENABLE_TMO to round up.

	1.06	06/18/91	janet
	Modified to use new state machine structure

	1.05	06/12/91	janet
	Added macro to get "disc" bit from identify message

	1.04	05/29/91	janet
	Added SC_GET_IT_NEXUS() macro.

	1.03	03/26/91	janet
	Updated after code review.

	1.02	12/10/90	rln
	Add SC_(UN)FREEZE macros.

	1.01	12/10/90	rln
	Add SC_GET_SOFTC macro.

	1.00	12/04/90	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */

/*
 * PRINTD for the SIM modules.
 */
#define SIM_MODULE(name) static char *module = "name"
#ifdef CAMDEBUG
#define TOASCII(i) ((i)+48)
#define SIM_PRINTD(B, T, L, flags, string)				\
    PRINTD(B, T, L, flags, ("[%c/%c/%c] (%s): ",			\
			    B == NOBTL ? 'b' : TOASCII(B),		\
			    T == NOBTL ? 't' : TOASCII(T),		\
			    L == NOBTL ? 'l' : TOASCII(L),		\
			    module));					\
    PRINTD(B, T, L, flags, string);
#else
#define SIM_PRINTD(B, T, L, flags, string);
#endif

/*
 * Maximum number of lost arbitrations.
 */
#define SIM_MAX_LOSTARB 10

/*
 * Actions used by sc_gen_msg().
 */
#define SC_MSG_START		((U32)0x0001)
#define SC_MSG_IDENTIFY		((U32)0x0002)
#define SC_MSG_SYNC		((U32)0x0004)
#define SC_MSG_TAG		((U32)0x0008)

/*
 * Decode values for the identify message.
 */
#define SC_IDENTIFY_LUN		0x07
#define SC_IDENTIFY_DISC	0x40

/*
 * Defines for accessing the HBA specific functions within the
 * SIM_SOFTC.
 */
#define SC_HBA_INIT(sc, csr) ((sc)->hba_init)(sc, csr)
#define SC_HBA_GO(sc, sws) ((sc)->hba_go)(sws)
#define SC_HBA_SM(sc, ssm) ((sc)->hba_sm)(ssm)
#define SC_HBA_BUS_RESET(sc) ((sc)->hba_bus_reset)(sc)
#define SC_HBA_SEND_MSG(sc, sws) ((sc)->hba_send_msg)(sws)
#define SC_HBA_XFER_INFO(sc, sws, buf, cnt, dir)			\
    ((sc)->hba_xfer_info)(sws, buf, cnt, dir)
#define SC_HBA_SEL_MSGOUT(sc, sws) ((sc)->hba_sel_msgout)(sws)
#define SC_HBA_MSGOUT_PEND(sc, sws) ((sc)->hba_msgout_pend)(sws)
#define SC_HBA_MSGOUT_CLEAR(sc, sws) ((sc)->hba_msgout_clear)(sws)
#define SC_HBA_MSG_ACCEPT(sc, sws) ((sc)->hba_msg_accept)(sws)
#define SC_HBA_SETUP_SYNC(sc, sws) ((sc)->hba_setup_sync)(sws)
#define SC_HBA_DISCARD_DATA(sc, sws) ((sc)->hba_discard_data)(sws)
#define SC_SCHED_START(sc, sws) ((sc)->sched_start)(sws)
#define SC_SCHED_RUN_SM(sc) ((sc)->sched_run_sm)(sc)
#define SC_SCHED_ABORT(sc) ((sc)->sched_abort)(sc)
#define SC_SCHED_TERMIO(sc) ((sc)->sched_termio)(sc)
#define SC_SCHED_BDR(sc) ((sc)->sched_bdr)(sc)

/*
 * Determine the minimum and maximum of two values.
 */
#define SC_GET_MIN(a, b) ((a) > (b) ? (b) : (a))
#define SC_GET_MAX(a, b) ((a) > (b) ? (a) : (b))

/* ---------------------------------------------------------------------- */
/*
 * The following macros are provided for accessing the functional
 * circular queue.
 */
#ifdef SIM_FUNCQ_ON
#define SC_ADD_FUNC(sws, msg);						\
    if ((sws) != (SIM_WS *)NULL) {					\
         CIRQ_SET_CURR((sws)->funcq, (sws)->funcq_buf, (msg));          \
	 if ((sws)->sim_sc != (SIM_SOFTC *)NULL) {			\
             CIRQ_SET_CURR((sws)->sim_sc->funcq,			\
                           (sws)->sim_sc->funcq_buf, (msg));		\
	 }								\
    }
#else
#define SC_ADD_FUNC(sws, msg);
#endif

/*
 * Macro: SC_FREEZE_QUEUE
 *
 * Description:
 * 	This macro is used to increase the freeze count of the nexus
 *	queue to prevent other SIM_WS's from being serviced.
 */
#define SC_FREEZE_QUEUE(sim_ws) (sim_ws)->nexus->freeze_count++
    
/*
 * Macro: SC_UNFREEZE_QUEUE
 *
 * Description:
 *	This macro is used to reduce the freeze count of the nexus
 *	queue.  When it reaches 0, I/O's will be allowed to continue. 
 */
#define SC_UNFREEZE_QUEUE(sim_ws)					\
	if ( (--(sim_ws)->nexus->freeze_count) < 0 ) {			\
		(sim_ws)->nexus->freeze_count = 0;			\
	}

/*
 * Macro: SC_WS_REMOVE and SC_WS_INSERT
 *
 * Description :
 *	These macros are used by the SIM to insert and remove SIM_WS's
 *	from the NEXUS queues.
 */
#define SC_WS_REMOVE(sws) remque((void *)(sws))
#define SC_WS_INSERT(sws, where) insque((void *)(sws), (void *)(where))

/*
 * Macro: SC_GET_IT_NEXUS
 *
 * Description:
 * 	This macro is used by SIM to return a IT_NEXUS pointer given a
 *	SIM_SOFTC and a target id.
 */
#define SC_GET_IT_NEXUS(sc, id)					\
    &((SIM_SOFTC *)(sc))->it_nexus[(id)]

/*
 * Macro: SC_GET_NEXUS
 *
 * Description:
 * 	This macro is used by SIM to return a NEXUS pointer given a
 *	SIM_SOFTC, target id, and lun.
 */
#define SC_GET_NEXUS(sc, id, lun)					\
    &((SIM_SOFTC *)(sc))->nexus[(id)][(lun)]

/*
 * Macro: SC_GET_WS
 *
 * Description:
 *	Given the CCB address this macro will return the associated
 *	SIM_WS pointer.  This macro depends on the placement of
 *	the SIM_WS within the DEC CAM packet.
 */
#define SC_GET_WS(ccb_hdr) \
    (SIM_WS *)(((U_WORD)(ccb_hdr)) - ((U_WORD)sizeof(SIM_WS)))

/*
 * Macro: SC_GET_SOFTC
 *
 * Description :
 *	Return the address of the SIM softc that is used for the 
 *	pathid.
 *
 */
#define SC_GET_SOFTC(pathid) softc_directory[(pathid)]

/*
 * Macro:  SC_DECODE_IDENTIFY
 *
 * Description :
 *	SC_DECODE_IDENTIFY() will break up an identify message into
 *	a LUN value.  The LUN will be returned in the form 0 to 7.
 */
#define SC_DECODE_IDENTIFY(message) ((message) & SC_IDENTIFY_LUN)

/*
 * Macro:  SC_GET_IDENTIFY_DISC
 *
 * Description :
 *	SC_GET_IDENTIFY_DISC() will break up an identify message and
 *	return a non-zero value if the disconnect bit is set in the
 *	identify message.  Zero will be returned if the bit is not set.
 */
#define SC_GET_IDENTIFY_DISC(message) ((message) & SC_IDENTIFY_DISC)

/* ---------------------------------------------------------------------- */
/*
 * The following macros will be provided to access the state machine
 * circular queue.  These macros will use the circular queue macros.
 */

/*
 * Macro: SC_SM
 *
 * Description :
 *	Will determine the past and current phase and use these
 *	values to access the appropriate SCSI state machine function
 *	to handle the current phase.
 *
 *	Will also determine if error recovery is needed.  If so,
 *	the function sim_err_sm() will be called.
 *
 */
#define SC_SM(sws);							\
{									\
    SIM_MODULE(SC_SM);							\
    if ((sws)->error_recovery) {					\
    	SIM_PRINTD((sws)->cntlr, (sws)->targid, (sws)->lun, CAMD_SM,	\
	       ("error recovery, phase 0x%x\n",				\
		SC_GET_CURR_PHASE(sws)));				\
    	sim_err_sm(sws);						\
    }									\
    else {								\
    	SIM_PRINTD((sws)->cntlr, (sws)->targid, (sws)->lun, CAMD_SM,	\
	       ("prev phase 0x%x, curr phase 0x%x\n",			\
		SC_GET_PREV_PHASE(sws), SC_GET_CURR_PHASE(sws)));	\
    	(scsi_sm[SC_GET_PREV_PHASE(sws)][SC_GET_CURR_PHASE(sws)])(sws);	\
    }									\
}

/*
 * Macro: SC_GET_SM_BUF
 *
 * Description :
 *	Will get a buffer from the State Machine's circular queue.  This
 *	buffer never leaves the queue.
 */
#define SC_GET_SM_BUF(sm, ssm);						\
{									\
    (ssm) = &((sm)->sm_data[CIRQ_ADJUST_INDEX((sm)->sm_queue,		\
					      (sm)->sm_queue.curr,	\
					      (sm)->sm_queue.curr_cnt)]); \
    (sm)->sm_queue.curr_cnt++;						\
}

/*
 * Macro: SC_GET_SM
 *
 * Description :
 *	Will get a pointer to the next request on the State Machine's
 *	queue which should be processed.  If the queue is empty, -1
 *	will be returned.
 */
#define SC_GET_SM(sm, ssm);						\
{									\
    SIM_MODULE(SC_GET_SM);						\
    if (CIRQ_CURR_SZ((sm)->sm_queue) == 0) {				\
        ssm = (SIM_SM_DATA *) -1;					\
        SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_SM,			\
	       ("Empty queue\n"));					\
    }									\
    else {								\
	ssm = &(CIRQ_GET_BYTE((sm)->sm_queue, CIRQ_CURR((sm)->sm_queue),\
			      (sm)->sm_data, 0));			\
	CIRQ_UPDATE_SEQ((sm)->sm_queue, 1);				\
    }									\
}

/* ---------------------------------------------------------------------- */
/*
 * The following macros will be provided to access the phase
 * circular queue and phase summary.  These macros will use the
 * circular queue macros.
 */

/*
 * Macro: SC_NEW_PHASE
 *
 * Description:
 * 	Use the SIM_WS pointer (sws) to access the phase queue.  Put
 *	the "phase" on this queue.
 */
#define SC_NEW_PHASE(sws, phase);					\
    CIRQ_SET_CURR((sws)->phaseq, (sws)->phaseq_buf, phase);

/*
 * Macro: SC_GET_PREV_PHASE
 *
 * Description:
 *	Use the SIM_WS pointer (sws) to access the phase queue.
 *	Determine the previous phase.
 */
#define SC_GET_PREV_PHASE(sws)						\
    CIRQ_GET_PREV((sws)->phaseq, (sws)->phaseq_buf)

/*
 * Macro: SC_GET_CURR_PHASE
 *
 * Description:
 *	Use the SIM_WS pointer (sws) to access the phase queue.
 *	Determine the current phase.
 */
#define SC_GET_CURR_PHASE(sws)						\
    CIRQ_GET_CURR((sws)->phaseq, (sws)->phaseq_buf)

/*
 * Macro: SC_ADD_PHASE_BIT(sws, phase)
 *
 * Description:
 *	Use the SIM_WS pointer (sws) to access the "phase_sum" field.
 *	Translate the "phase" into a bit value and "or" this value into
 *	the current "phase_sum."
 */
#define SC_ADD_PHASE_BIT(sws, phase)					\
    (sws)->phase_sum |= SCSI_PHASEBIT(phase)

/* ---------------------------------------------------------------------- */
/*
 * The following macros are provided to access the message
 * in and message out circular queues.  These macros will use
 * the circular queue macros contained in the file sim_cirq.h.
 *
 * The message out queue will be used in "sequence" mode.  This
 * means that bytes will be added to the current sequence.  The
 * current sequence becomes the previous sequence by calling
 * SC_UPDATE_MSGOUT.  Before a message out queue can be used in
 * this mode the macro CIRQ_USE_SEQ must be called.
 */

/*
 * Macro: SC_ADD_MSGOUT
 *
 * Description:
 *	Add the specified message to the SIM_WS's message out
 *	queue.  The number of message out bytes will be incremented
 *	by one.
 */
#define SC_ADD_MSGOUT(sws, msg)						\
    CIRQ_ADD_BYTE(((sws)->msgoutq), (sws)->msgoutq_buf, msg)

/*
 * Macro: SC_GET_MSGOUT
 *
 * Description:
 *	Get a message byte from the current message out data sequence.
 *	Use the index value to determine which message byte to
 *	retreive.
 */
#define SC_GET_MSGOUT(sws, index)					\
    CIRQ_GET_BYTE((sws)->msgoutq, CIRQ_CURR((sws)->msgoutq),		\
		  (sws)->msgoutq_buf, (sws)->msgout_sent + (index))

/*
 * Macro: SC_GET_MSGOUT_LEN
 *
 * Description:
 *	Determine the number of message out bytes which are ready
 *	to be sent to the target.
 */
#define SC_GET_MSGOUT_LEN(sws)						\
    (CIRQ_CURR_SZ((sws)->msgoutq) - (sws)->msgout_sent)

/*
 * Macro: SC_GET_PREV_MSGOUT
 *
 * Description:
 *	Get a message byte from the previous message out data sequence.
 *	Use the index value to determine which message byte to
 *	retreive.
 */
#define SC_GET_PREV_MSGOUT(sws, index)					\
    CIRQ_GET_BYTE((sws)->msgoutq, CIRQ_PREV((sws)->msgoutq),		\
		  (sws)->msgoutq_buf, index)

/*
 * Macro: SC_UPDATE_MSGOUT
 *
 * Description:
 *	This macro should be called after the message bytes in the
 *	message out queue have been sent out.  This macro will update
 *	the current and previous message out pointers.
 */
#define SC_UPDATE_MSGOUT(sws, count);					\
    if (((sws)->msgout_sent += (count)) >= SC_GET_MSGOUT_LEN(sws)) {	\
        CIRQ_UPDATE_SEQ((sws)->msgoutq, (sws)->msgout_sent);		\
	(sws)->msgout_sent = 0;						\
    }

/*
 * Macro: SC_GET_PREV_MSGOUT_LEN
 *
 * Description:
 *	Determine the number of bytes that were sent out during
 *	the last message out phase.
 */
#define SC_GET_PREV_MSGOUT_LEN(sws)					\
    CIRQ_PREV_SZ((sws)->msgoutq)

/*
 * Macro: SC_RETRY_MSGOUT
 *
 * Description:
 *	During error recovery a message byte may have to
 *	be resent.  This macro will set the current message out
 *	pointer to the previous message sent.
 */
#define SC_RETRY_MSGOUT(sws);						\
{									\
    if ((sws)->msgout_sent == 0) {					\
    	(sws)->msgoutq.curr = (sws)->msgoutq.prev;			\
    	(sws)->msgoutq.curr_cnt = (sws)->msgoutq.prev_cnt;		\
    } else 								\
	(sws)->msgout_sent--;						\
}

/*
 * Macro: SC_ADD_MSGIN
 *
 * Description:
 *	Use the SIM_WS pointer (sws) to access the message in queue
 *	(msginq).  Put the message, "msg", onto this queue.
 */
#define SC_ADD_MSGIN(sws, msg);						\
    CIRQ_ADD_BYTE((sws)->msginq, (sws)->msginq_buf, msg);

/*
 * Macro: SC_SET_MSGIN_LEN(sws, len)
 *
 * Description:
 *	Use the SIM_WS pointer (sws) to access the message in queue
 *	(msginq).  Set the "needed" field of this queue to "len."
 *	This macro is used for multi-byte message
 */
#define SC_SET_MSGIN_LEN(sws, len)					\
    CIRQ_SET_NEEDED((sws)->msginq, len)

/*
 * Macro: SC_GET_MSGIN
 *
 * Description:
 *	Get a message byte from the current message in data sequence.
 *	Use the index value to determine which message byte to
 *	retrieve.
 */
#define SC_GET_MSGIN(sws, index)					\
    CIRQ_GET_BYTE((sws)->msginq, CIRQ_CURR((sws)->msginq),		\
		  (sws)->msginq_buf, index)

/*
 * Macro: SC_GET_MSGIN_LEN
 *
 * Description:
 *	Determine the number of message in bytes which are in the
 *	message in queue.
 */
#define SC_GET_MSGIN_LEN(sws)						\
    CIRQ_CURR_SZ((sws)->msginq)

/*
 * Macro: SC_UPDATE_MSGIN
 *
 * Description:
 */
#define SC_UPDATE_MSGIN(sws, count);					\
    CIRQ_UPDATE_SEQ((sws)->msginq, (count));

/* ---------------------------------------------------------------------- */
/*
 * The following macros are provided for setting up and disabling
 * command timeouts.
 */

/*
 * Macro: SC_ENABLE_TMO
 *
 * Description:
 * 	This macro is invoked to setup command specific timeouts.
 *	Typically just before starting a command, this macro will
 *	be called to setup a timeout.  With cooperation between this
 *	macro and the SIM Scheduler's timer, stalled I/O requests
 *	are timed out. The SIM_WS contains two fields that are used
 *	during timeouts. The "sim_tmo" field determines when the SIM_WS
 *	should be timed out and the "tmo_routine" field determines
 *	where to go when a timeout occurs.
 * 
 *	This macro will determine the current time (in seconds) and
 *	add to it the timeout value specified in the SIM_WS.  If the
 *	specified timeout value is CAM_TIME_INFINITY, no command 
 *	timeout will be scheduled.  If the specified timeout value
 *	is CAM_TIME_DEFAULT, the default timeout of "sim_default_timeout"
 *	(which is set in cam_data.c) will be used.
 *
 *	Also check sim_disable_timeout.  This is a back door for users
 *	to disable all SIM timeouts.  It can only be modified on a
 *	running system with a debugger.
 */
#define SC_ENABLE_TMO(sws, tmo_routine, arg, delta_time);		\
    if (((delta_time) != CAM_TIME_INFINITY) && !sim_disable_timeout) {	\
        (sws)->tmo_fn = (void (*)())(tmo_routine);			\
        (sws)->tmo_arg = (void *)(arg);					\
	if ((delta_time) == CAM_TIME_DEFAULT)				\
            (sws)->time.tv_sec = 1 + sc_get_time_sec() + sim_default_timeout;\
	else								\
	    (sws)->time.tv_sec = 1 + sc_get_time_sec() + (delta_time);	\
    }

/*
 * Macro: SC_DISABLE_TMO
 *
 * Description:
 * 	This macro will clear any pending timeouts on the specified
 *	SIM_WS.
 */
#define SC_DISABLE_TMO(sws) (sws)->time.tv_sec = 0

/*
 * Common phase defines for HBA's.  These defines are based on the
 * MSG, C/D, and I/O lines of the SCSI bus.
 */
#define SC_PHASE_DATAOUT	0x0
#define SC_PHASE_DATAIN		0x1
#define SC_PHASE_COMMAND	0x2
#define SC_PHASE_STATUS		0x3
#define SC_PHASE_MSGOUT		0x6
#define SC_PHASE_MSGIN		0x7

#endif /* _SIM_COMM_ */
