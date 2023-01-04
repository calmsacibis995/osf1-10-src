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
 * @(#)$RCSfile: sim_xpt.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/10/06 13:45:06 $
 */
#ifndef _SIM_XPT_
#define _SIM_XPT_

/**
 * FACILITY:
 *
 *	ULTRIX SCSI CAM SUBSYSTEM
 *
 * ABSTRACT:
 *
 * This module contains those macros and definitions that are specific
 * to the SIM XPT component of the ULTRIX CAM SCSI Subsystem..
 *
 * AUTHOR:
 *
 *	Richard L. Napolitano, 12-Dec-1990
 *
 * MODIFIED BY:
 * 
 *	Richard L. Napolitano  12-Dec-1990
 *	Original entry.
 *
 **/


/**
 * GET_TIME_SEC
 *
 * FUNCTIONAL DESCRIPTION: This macro is used by SIM to return the current
 * time in seconds, the time structure is a global kernel structure allocate
 * in kernel.h. 
 *
 * FORMAL PARAMETERS:   None.
 *
 * IMPLICIT INPUTS:     None.
 *
 * IMPLICIT OUTPUTS:    None.
 *		     
 **/
#define GET_TIME_SEC() (time.tv_sec) 

/**
 * LOCK_NEXUS/UNLOCK_NEXUS -
 *
 * FUNCTIONAL DESCRIPTION: These macros are used by the SIM to synchronize
 * access to the nexus queues in an SMP environment.
 *
 * FORMAL PARAMETERS:   TBD, for now none.
 *
 * IMPLICIT INPUTS:     None.
 *
 * IMPLICIT OUTPUTS:    None.
 *		     
 **/
#define LOCK_NEXUS(s,lock_handle); {			\
		(s) = splbio();				\
}
#define UNLOCK_NEXUS(s,lock_handle); {			\
		splx(s);				\
}

/**
 * XPT_SIM_LOADED
 *
 * FUNCTIONAL DESCRIPTION: This macro is used by SIM to notify the XPT of
 * asynchronous events.
 *
 * FORMAL PARAMETERS:   See below.
 *
 * IMPLICIT INPUTS:     None.
 *
 * IMPLICIT OUTPUTS:    None.
 *		     
 **/
#define XPT_ASYNCH_CALLBACK(opcode,pathid,targetid,lun,bufptr,cnt)\
(\
xpt_async((opcode),(pathid),(targetid),(lun),(bufptr),(cnt))\
)


/**
 * SC_FIND_WS -
 *
 * FUNCTIONAL DESCRIPTION: This macro when invoked will return the value
 * of the sim_ws whose tag matches the arguments to this macro.
 *
 * FORMAL PARAMETERS:      U32 id  - Target ID
 *                         U32 lun - Logical Unit Number
 *                         U32 tag - SCSI command tag
 *
 * IMPLICIT INPUTS:     None.
 *
 * IMPLICIT OUTPUTS:    None.
 *		     
 **/
#define SC_FIND_WS(softc,id,lun,tag) (\
				       \
				       \
/* need to handle tagged or untagged, look at first entry in nexus queue to*/\
/*determine whether it is tagged or untagged for non tagged return first one*/\
				       sim_softc->ASA[tag]\
				       )

/**
 * XPT_SIM_LOADED
 *
 * FUNCTIONAL DESCRIPTION: This macro is used by SIM to notify the XPT that
 * a SIM was loaded and to register the CAM_SIM_ENTRY with the XPT.
 *
 * FORMAL PARAMETERS:   CAM_SIM_ENTRY sim_entry - Address of CAM SIM ENTRY
 *
 * IMPLICIT INPUTS:     None.
 *
 * IMPLICIT OUTPUTS:    None.
 *		     
 **/
#define XPT_SIM_LOADED(sim_entry)\
(\
printf("Notify the XPT that a sim was loaded with a SIM_ENTRY of :%x.\n",(sim_entry))\
/* xpt_bus_register((sim_entry)) */\
) /* XPT_SIM_LOADED */
    
    
/**
 * SX_ENABLE_TMO -
 *
 * FUNCTIONAL DESCRIPTION: This macro is invoked to setup HBA
 * specific time-outs. Typically before expaecting an interrupt the
 * SIMH will call this routine to setup a time-out before waiting
 * on an event. With cooperation between this routine and the SIM
 * timer, stalled I/O requests are timed-out. The SIM_WScontains
 * two fields that are used during time-outs. The sim_tmo field
 * determines when the sim_ws should be timed-out and the
 * tmo_routine field determines where to go when a time-out occurs.
 * If a time-out occurs this routine  will invoke the tmo routine
 * provided by the SIM  HBA.
 *
 * 
 * Setup the timeout routine address and argument and then add the
 * current time to the CCB timeout time to enable a timeout.
 *
 * The SIM timer will expire at periodic intervals and look for sim_ws
 * that needs to be timed out. 
 *
 *
 * FORMAL PARAMETERS:    SIM_WS  sim_ws      - Address of  sim_ws.
 *                       void    tmo_routine - Routine to be called if time-out
 *					      occurs
 * 			caddr_t arg	    - Parameter to be returned
 *                       U32  delta_time  - Time-out value in seconds. 
 *					      If delta_time is zero then there
 *					      is an infinite timeout
 * 
 *
 *
 * CALL CONVENTION:
 *
 *   SX_ENABLE_TMO(sim_ws,sh_phase_tmo,sim_ws,\
 *                        sim_ws->ccb(CCB_SCSIIO)->cam_timeout)
 *
 **/
#define SX_ENABLE_TMO(sim_ws,tmo_routine,arg,delta_time) \
(\
 (SIM_WS*)(sim_ws)->tmo_fn  = tmo_routine;\
 (SIM_WS*)(sim_ws)->tmo_arg = (void *)arg;\
 (SIM_WS*)(sim_ws)->time.tv_sec = GET_TIME_SEC() + delta_time\
 )
    
/**
 * SX_DISABLE_TMO -
 *
 * FUNCTIONAL DESCRIPTION: This macro when invoked clears any
 * pending time-outs on this sim_ws. The SIMH time-out fields for
 * this sim_ws are  disabled.
 *
 * FORMAL PARAMETERS:    SIM_WS*	 sim_ws   - Address of sim_ws.
 *
 * CALLING CONVENTION:
 *
 *	SX_DISABLE_TMO(sim_ws)
 **/
    /* Clear timer to prevent timeouts on this SIM_WS */
#define SX_DISABLE_TMO(sim_ws) \
(\
 (sim_ws)->time.tv_sec = 0\
 )

#endif /* _SIM_XPT_ */
