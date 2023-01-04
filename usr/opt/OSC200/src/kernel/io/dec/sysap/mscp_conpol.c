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
static char *rcsid = "@(#)$RCSfile: mscp_conpol.c,v $ $Revision: 1.1.11.2 $ (DEC) $Date: 1993/07/13 16:25:59 $";
#endif
/*
 * derived from mscp_conpol.c	2.5	(ULTRIX)	1/19/90";
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Disk Class Driver
 *
 *   Abstract:	This module contains all the routines associated with
 *		connection management.
 *
 *   Author:	David E. Eiche	Creation Date:	September 30, 1985
 *
 *   History:
 *
 *   31-Oct-1991	Pete Keilty
 *	Changed memeory allocation to OSF/1 memory zones. 
 *	Routines are in scs_subr.c.
 *	Modified routines to use new OSF configuration structures bus, 
 *	controller & device.
 *
 *   19-Feb-1991	Tom Tierney
 *	ULTRIX to OSF/1 port:
 *
 * 
 *   Dec-1990		Matthew Sacks
 *	Made the handler SMP safe.  Changes to each routine are
 *	documented at the beginning of the routine.  For a discussion
 *	of all the changes, see the comments at the end of mscp_defs.h.
 *
 *   04-Dec-1990        Pete Keilty
 *      Modified mscp_system_poll() added Splscs synchronization
 *      because it is called from timeout routine now also.
 *      Modified mscp_consetretry() to set polling for adapter
 *      fail over.
 *      Modified mscp_consubr() to set lport_name, rport_addr & sysid
 *      from the connection block.
 *
 *   03-July-1990	Matthew Sacks
 *	Modified the below change to special case the KDB50.  This is
 *	because they have an init process which is both very slow and
 *	which blocks out the other KDB50's.  The goal is to prevent one
 *	KDB50's init from causing host access timeouts to another KDB50.
 *
 *   28-Dec-1989	David E. Eiche		DEE0081
 *	Change mscp_constconem() to identify controllers that support
 *	dual porting and to adjust the host timeout interval to prevent
 *	spurious host timeouts caused by timer processing delays.  Change
 *	mscp_constrstcon() to use the processing overhead factor in
 *	computing a default controller timeout.
 *
 *   17-Mar-1989	Tim Burke
 *	Changed queue manipulations to use the following macros:
 *	insque ..... Insert_entry
 *	remque ..... Remove_entry
 *	remqck ..... Remove_entry and check to see if any elements on queue.
 *
 *   12-Feb-1989	Todd M. Katz		TMK0003
 *	1. Modify TMK0002 to use the shorthand notation Lproc_name when
 *	   refering to the corresponding MSB field.
 *	2. Include header file ../vaxmsi/msisysap.h.
 *	3. Use the ../machine link to refer to machine specific header files.
 *
 *   17-Oct-1988	Pete Keilty
 *	Fix concleanup save off wrp->flink for later use, wrp insque to
 *	restart queue.
 *
 *   28-Sep-1988	David E. Eiche		DEE0058
 *	Fix host timeout logic so that controllers that are initialized
 *	to have host timeout disabled retain that characteristic after
 *	connection recovery.
 *
 *   28-Sep-1988	David E. Eiche		DEE0057
 *	Fix panic message formats to make them consistent.
 *
 *   14-Sep-1988	Todd M. Katz		TMK0002
 *	Pass the local SYSAP name in the MSB to SCS when crashing a path.
 *
 *   09-Sep-1988	David E. Eiche		DEE0056
 *	Move initialization of request state from mscp_conqrestart
 *	to mscp_restart_next in mscp_subr.c.  This fixes a bug in
 *	which the connection management request block was having
 *	its state erroneously initialized.
 *
 *   06-Sep-1988	David E. Eiche		DEE0054
 *	Fix code which caused disks to hang when a second loss of
 *	connection occurred while recovering from the first.
 *
 *   06-Sep-1988	David E. Eiche		DEE0053
 *	Change host/controller timeout code to provide a default
 *	controller timeout period if the controller returns a zero
 *	controller timeout.  This is done in conjunction with a
 *	change to the host timeout values in the controller model
 *	table.
 *
 *   17-Aug-1988	David E. Eiche		DEE0051
 *	Change state definitions to use ST_CMN_INITIAL so that recovery
 *	can rely on the initial state in any state table to be the same.
 *	Also modify mscp_polinit to set and mscp_polgtuntem to clear the
 *	upoll_busy flag in the connection block.
 *
 *   17-July-1988	David E. Eiche		DEE0047
 *	Change parameters used to call mscp_get_unitb, so that it
 *	can be called from attention message processing.  Move the
 *	calls to mscp_find_controller and mscp_find_device for the
 *	same reason.
 *
 *   17-July-1988	David E. Eiche		DEE0046
 *	Change parameters used in calling mscp_get_connb, so that it
 *	can be called from the new path section of the control routine.
 *	This allows the new system event to be processed directly rather
 *	than by invocation of system polling.
 *
 *   17-July-1988	David E. Eiche		DEE0045
 *	Change references to the model table to conform to the new
 *	format and to use the connection block fields where appropriate.
 *
 *   29-June-1988	David E. Eiche		DEE0044
 *	Change mscp_conresynch to always call scs_crash_path so that
 *	all SYSAPs connected to a server are quickly notified of path
 *	failure.  This change is intended to prevent multiple resynch's
 *	of a controller as the result of a single failure.
 *
 *   29-June-1988	David E. Eiche		DEE0043
 *	Change mscp_condisccmplt to dispatch EV_ERRECOV instead of
 *	EV_INITIAL.  Change mscp_concleanup to clarify the event
 *	redispatch logic and improve the code style.
 *
 *   27-June-1988	David E. Eiche		DEE0042
 *	Add code to mscp_constrstcon to supply a default maximum transfer
 *	length value if the controller does not supply the field.  This
 *	change is in accordance with the MSCP specification and is required
 *	by the mscp_ioctl routine to implement the bad block scan ioctl.
 *	
 *   16-June-1988	Larry Cohen
 *	Add global variable mscp_polls that is incremented for each
 *	connection attempt and decremented for each connection completion.
 *	The system waits in init_main for mscp_polls to go to zero so that
 *	autoconfigure output can be delayed until the disks/tapes have
 *	been sized.
 *
 *   02-Jun-1988     Ricky S. Palmer
 *      Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   20-May-1988	David E. Eiche		DEE0038
 *	Remove mscp_conreturn routine.  All references to mscp_conreturn
 *	had previously been changed to use mscp_noaction.
 *
 *   15-May-1988	David E. Eiche		DEE0034
 *	Change mscp_conresynch to correctly bracket if's.
 *	Add routine mscp_condealmsg to deallocate a connection
 *	management service message that arrives after the connection
 *	is restarted.
 *
 *   15-Jan-1988	Todd M. Katz		TMK0001
 *	Include new header file ../vaxmsi/msisysap.h.
 *
 *   25-Jan-1988	Ricky S. Palmer
 *	Added some "node_name" based code to determine whether a dssc or an hsc
 *	is involved. This needs to be done using the hardware port type.
 *
 */

/**/

/* Libraries and Include Files.
 */
#include	<labels.h>
#include	<sys/types.h>
#include	<dec/binlog/errlog.h>
#include	<io/dec/scs/sca.h>
#include	<io/dec/sysap/sysap.h>
#include	<io/dec/sysap/mscp_bbrdefs.h>

/**/
/* #define DEBUG_PRINT */

/* External variables and routines.
 */
extern	struct timeval		boottime;
extern	int			dkn;
extern	int 			mscp_polls;

extern	u_long			scs_connect();
extern	u_long			scs_crsh_path();
extern	u_long			scs_info_system();
extern	u_long			scs_reset();
extern	u_long			scs_restart();
extern	void			scs_unix_to_vms();

extern	void			mscp_bbr_init();
extern	void			mscp_conqrestart();
extern	void			mscp_dealloc_all();
extern	void			mscp_dispatch();
extern	void			mscp_find_controller();
extern	void			mscp_find_device();
extern	void			mscp_find_model();
extern	void			mscp_recycle_rspid();
extern	void			mscp_reserve_credit();
extern	void			mscp_restart_next();
extern	u_long			mscp_send_msg();

extern  UNITB			*mscp_unit_tbl[];
extern	STATE			mscp_con_states[];
extern	STATE			mscp_pol_states[];
extern	STATE			mscp_rec_states[];
extern	LISTHD			mscp_rspid_wait_lh;

extern  caddr_t			sca_zalloc(), sca_zget();
extern  int			sca_zfree(), sca_zones_init(), 
				sca_zones_initialized;
extern  struct zone 		*sca_zone[];
/* Forward routine references.
 */
void				mscp_consubr();
void				mscp_confmtstcon();
void				mscp_constrstcon();
CONNB *				mscp_get_connb();
UNITB *				mscp_get_unitb();



/**/

/*
 *
 *   Name:	mscp_system_poll - Find and connect to known (sub-)systems.
 *
 *   Abstract:	Find all systems currently known by SCS and attempt to
 *   		initiate a connection to each of them.
 *
 *   Inputs:	Class block is locked.
 *
 *   Outputs:   Class block is locked.
 *
 *   SMP:	Access to flags in the class block is safe because the
 *		the caller has locked the class block.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_system_poll( clp )
    CLASSB		*clp;
{
    CONNB		*cp;
    ISB			isb;
    SIB			sib;
    CMSB		cmsb;
    int                 saved_ipl;

    saved_ipl = Splscs();
    clp->flags.need_poll = 0;

    /* Poll SCS for each system that it knows about, and attempt to
     * establish connections to all known systems.  If an SCS service
     * returns any error status or if sufficient memory is not available
     * to allocate a connection block, abort the current polling cycle
     * and reschedule polling.
     */
    Zero_scaaddr( isb.next_sysid );
    do {
	cp = NULL;
	if( scs_info_system( &isb, &sib ) == RET_SUCCESS ) {
	    isb.next_lport_name = 0;
	    if( scs_info_path( &isb, NULL ) == RET_SUCCESS ) {
		cp = mscp_get_connb( clp,
				     &isb.sysid,
				     &isb.rport_addr,
				     isb.lport_name );
	    }
	}
	if( cp == NULL ) {
	    clp->flags.need_poll = 1;
	    break;
	}
    } while ( !Test_scaaddr( isb.next_sysid )); 

    /* If the polling cycle completed successfully and the listening
     * connection has not yet been established, issue a listen request
     * to SCS.  If the listen does not succeed, reschedule polling,
     * as it is then the only way to find out about new systems.
     */
    if(( clp->flags.need_poll == 0 ) && ( clp->flags.listen == 0 )) {
	Lock_connb (cp);
 	cmsb = clp->cmsb;
	cmsb.aux = ( u_char * )clp;
	if( scs_listen( &cmsb ) == RET_SUCCESS ) {
	    clp->flags.listen = 1;
	} else {
	    clp->flags.need_poll = 1;
	}
	Unlock_connb (cp);
    }
    splx( saved_ipl);
    return;
}

/**/

/*
 *
 *   Name:	mscp_coninit - Connect to the target system.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect connection block.
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_coninit( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;

    Lock_connb (cp);
    cp->retry_count = CONNECT_RETRIES;
    Unlock_connb (cp);
    mscp_consubr( rp );
    return( EV_NULL );
}
/**/

/*
 *
 *   Name:	mscp_concomplete - Process a connection completed response.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	protect queue manipulations and the connection block.
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_concomplete( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CMSB		*cmsp;
    u_long		status = EV_NULL;

#define ADR_EVMS_DISK	0
#define ADR_EVMS_TAPE	0x2020

    Lock_connb (cp);

    /* Turn off the connection timeout.
     */
    cp->cmdtmo_intvl = 0;

    /* A connect complete event was received.  Analyze the completion status.
     */
    if( event == EV_CONCOMPLETE ) {

	cmsp = ( CMSB * )rp->aux;
	
	switch( cmsp->status ) {

	    /* The connection succeeded.  Store the connection ID and
	     * redispatch.
	     */
	    case ADR_SUCCESS:
		Move_connid( cmsp->connid, cp->connid );
		status = EV_CONACTIVE;
		Unlock_connb (cp);
		break;

	    /* The server does not support the requested service.
	     * Vaporize the connection block and go away.
	     */
	    case ADR_NOSUPPORT:
	    case ADR_NOLISTENER:
	    case ADR_EVMS_DISK:
	    case ADR_EVMS_TAPE:
#ifdef DEBUG_PRINT
printf(">>> mscp_concomplete: ADR_NOSUPPORT or ADR_NOLISTENER\n");
#endif /* DEBUG_PRINT */
		Remove_entry( cp->flink );
		Unlock_connb (cp);
		SCA_KM_FREE( cp, sizeof( CONNB ), KM_SCA );
		mscp_polls--;
		break;
    
	    /* A retryable error occurred.  If retries remain, reissue
	     * the connect.
	     */
	    case ADR_DISCONN:
	    case ADR_NOCREDIT:
	    case ADR_PATH_FAILURE:
	    case ADR_NORESOURCE:
#ifdef DEBUG_PRINT
printf(">>> mscp_concomplete: retryable error occurred (0x%x)\n", cmsp->status);
#endif /* DEBUG_PRINT */
		if( cp->retry_count-- ) {
		    Unlock_connb (cp);
		    mscp_consubr( rp );
		    }
		else 
		    {
		    status = EV_EXRETRY;
		    Unlock_connb (cp);
		    }
		break;

	    /* These errors should not occur.  If they do, panic.
	     */
	    case ADR_BUSY:
		Unlock_connb (cp);
		panic( "mscp_concomplete:  connect to an active server\n" );
	    default:
#ifdef DEBUG_PRINT
                printf("mscp_concomplete: status = %x\n",cmsp->status);
                printf("mscp_concomplete: sysid = %lx\n", cmsp->sysid);
                printf("mscp_concomplete: rport_addr = %lx\n", cmsp->rport_addr);
#endif /* DEBUG_PRINT */
		Unlock_connb (cp);
		panic( "mscp_concomplete:  unrecognized completion status\n" );

	}

    /* The connect timed out.  If retries remain, reissue the connect.
     */
    } else {
#ifdef DEBUG_PRINT
printf(">>> mscp_concomplete: connection timed out\n");
#endif /* DEBUG_PRINT */
	if( cp->retry_count-- ) {
	    Unlock_connb (cp);
	    mscp_consubr( rp );
	    }
	else {
	    status = EV_EXRETRY;
	    Unlock_connb (cp);
	    }
    }

    return( status );

}

/**/


/*
 *
 *   Name:	mscp_constconcm - Send an MSCP STCON command message.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Lock the relevent connection block before calling
 *		mscp_confmtstcon
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_constconcm( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;

    /* Format a set controller characteristics message, and attempt
     * to send it out on the connection.  Note that the host timeout
     * field in this message is zero, which means no host timeout is
     * desired for this controller (for the moment).
     */
    Lock_connb (cp);
    mscp_confmtstcon( rp, 0 );
    Unlock_connb (cp);

    /* The send message has been queued successfully.  Exit to wait for
     * the end message to come back.
     */

    return( mscp_send_msg( rp ) );
}

/**/
u_short mscp_htmo_overhead = HTMO_OVERHEAD;
u_short mscp_htmo_overhead_kdb50 = HTMO_OVERHEAD_KDB50;
/*
 *
 *   Name:	mscp_constconem - Process an MSCP STCON end message.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	all set
 *
 *   Return	NONE
 *   Values:
 */
u_long
mscp_constconem( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    u_long		state = rp->state;

    /* Store the controller characteristics and recycle the RSPID.
     * Note also that the end message buffer address has been stored
     * in the REQB so that it can be used to send the next connection
     * management command.  Then look up the controller model information
     * and store a pointer to the model table entry in the CONNB.
     */
    mscp_constrstcon( rp );
    mscp_recycle_rspid( rp );

    /* If the connection state is STCON1 and the controller supports
     * dual port operation, the host timeout is calculated as the larger
     * of the compiled-in host dual port timeout and the controller
     * timeout that was returned in the STCON end message plus a 
     * processing overhead factor.  A message will be sent to the
     * controller at least once per controller timeout period, so no
     * host timeout should occur while the host is operational.
     *
     * For controllers that do not support dual port operation, the host
     * timeout is disabled by setting it to zero.  Since zero was sent in
     * the first STCON command, it is not necessary to send a second command.
     */

    Lock_connb (cp);
    if( state == ST_CN_STCON1 ) {
	mscp_find_model( cp );
	mscp_reserve_credit( rp );
	if(( cp->cnt_flgs & MSCP_CF_MLTHS )		||
	   ( cp->cnt_id.model == MSCP_CM_UDA50 )	||
	   ( cp->cnt_id.model == MSCP_CM_UDA50A )	||
	   ( cp->cnt_id.model == MSCP_CM_KDA50 )	||
	   ( cp->cnt_id.model == MSCP_CM_KDB50 )	||
	   ( cp->cnt_id.model == MSCP_CM_KDM70 ))	{
	    cp->hst_tmo = HTMO_DUALPORT;
	    if ((cp->cnt_tmo + mscp_htmo_overhead) > cp->hst_tmo ) {
		if (cp->cnt_id.model == MSCP_CM_KDB50)
			cp->hst_tmo = cp->cnt_tmo + mscp_htmo_overhead_kdb50;
		else
			cp->hst_tmo = cp->cnt_tmo + mscp_htmo_overhead;
	    }

	    mscp_confmtstcon( rp, cp->hst_tmo );

	    Unlock_connb (cp);

	    /* If the message is queued successfully, exit to wait for the
	     * end message or timeout.  Otherwise, resynch the controller.
	     */
	    if( mscp_send_msg( rp ) == EV_NULL ) {
		return( EV_NULL );
	    } else {
		return( EV_EXRETRY );
	    }
	}
    }
    

    /* The the controller characteristics have been set successfully.
     * If the controller requires host initiated bad block replacement,
     * allocate and initialize the BBR work area (BBRB).
     */

    if( cp->classb->flags.disk && !( cp->cnt_flgs & MSCP_CF_REPLC )) {
	Unlock_connb (cp);
	mscp_bbr_init( cp );
    }
    else Unlock_connb (cp);
     
    return( EV_CONACTIVE );
    
}


/**/

/*
 *
 *   Name:	mscp_conmarkopen - Mark the connection open.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect connection block.
 *
 *   Return	NONE
 *   Values:
 */
u_long
mscp_conmarkopen( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;

    /* Initiate polling for units attached to the server on this
     * connection, and exit.
     */
    Lock_connb (cp);
    mscp_find_controller( cp );
    cp->polling_reqb.state_tbl = mscp_pol_states;
    cp->polling_reqb.state = ST_CMN_INITIAL;
    Unlock_connb (cp);
    mscp_dispatch( EV_INITIAL, &cp->polling_reqb );
    return( EV_NULL );
}
/**/

/*
 *
 *   Name:	mscp_conrestore - Restore the unit states.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect the connection block.
 *
 *   Return	NONE
 *   Values:
 */
u_long
mscp_conrestore( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CLASSB		*clp = rp->classb;

    /* If connection recovery is in progress, initiate a thread to
     * restore the state of any units that were online when the
     * connection was lost.
     */
    Lock_connb (cp);
    if( cp->flags.restart ) {
	cp->polling_reqb.state_tbl = clp->recov_states;
	cp->polling_reqb.state = ST_CMN_INITIAL;
	Unlock_connb (cp);
	mscp_dispatch( EV_INITIAL, &cp->polling_reqb );
    }
    else {
	 Unlock_connb (cp);
    }
    mscp_polls--;
    return( EV_NULL );
}
/**/

u_short	mscp_force_reset = 0;
/*
 *
 *   Name:	mscp_conwatchdog - Check activity of an open connection.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	We lock the connection block's timeout-block-lock (tmob).
 *		Within the scope of this lock we will know if we are
 *		timing out the connection or not, and if we are we set
 *		the host-time-out-of-the-controller flag (ctrl_tout).
 *		This flag is checked by mscp_conendmsg() so that 
 *		mscp_conendmsg() is synchronized with this routine,
 *		this is neccesary in case an end message for the
 *		connection which we are timing out comes in via 
 *		another processor.  The flag is cleared by mscp_conresynch().
 *
 *   Return	NONE
 *   Values:
 */
u_long
mscp_conwatchdog( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    REQB		*hrp = cp->active.flink;
    MSCP		*mp = rp->msgptr;
    u_long		new_event = EV_NULL;


    /* We take out the timeout block lock in the connection block in order
     * to synchronize this routine with the mscp_conendmsg() routine.  We
     * are assuming that the caller appropriately raised IPL.
     */
    Lock_tmob(cp);

    if( mscp_force_reset ) {
	mscp_force_reset = 0;
	cp->flags.ctrl_tout = 1;
	Unlock_tmob (cp);
#ifdef DEBUG_PRINT
printf(">>> mscp_conwatchdog: mscp_force_reset != 0\n");
#endif /* DEBUG_PRINT */
	return( EV_EXRETRY );
    }
	
    /* Check the controller time out flag.  If it is already on, we
     * have already decided to time out the connection and we must kill
     * this thread of execution.
     */
    if (cp->flags.ctrl_tout) {
	new_event =  EV_NULL;
        Unlock_tmob (cp);
#ifdef DEBUG_PRINT
printf(">>> mscp_conwatchdog: already timing out connection :-(\n");
#endif /* DEBUG_PRINT */
    }
    /* If the message pointer is NULL, the controller never responded
     * to the previous GTUNT or GTCMD message.  Redispatch to
     * resynchronize the connection.
     */
    else if( mp == NULL ) {
	new_event =  EV_EXRETRY;
	cp->flags.ctrl_tout = 1;   /* flag that we timing out controller */
        Unlock_tmob (cp);
#ifdef DEBUG_PRINT
printf(">>> mscp_conwatchdog: msg==NULL so time out connection :-(\n");
#endif /* DEBUG_PRINT */
    }
    /* Check to see if there are any requests outstanding in
     * the active queue.
     */
    else if( hrp != ( REQB * )&cp->active.flink ) {

	Lock_connb (cp);
	/* Check to see if the command reference number (RSPID) of the oldest
	 * active command has changed during the timeout interval.  If the
	 * RSPIDs are different, at least one command must have completed in
	 * the interval.  Update the oldest active RSPID field in the CONNB,
	 * and re-init the command status.
	 */
	if( *( u_int * )&hrp->rspid != *( u_int *)&cp->old_rspid ) {
		    cp->old_rspid = hrp->rspid;
		    cp->old_cmd_sts = 0xffffffff;
		    Unlock_connb (cp);
		    Unlock_tmob (cp);

	/* The RSPIDs are the same, indicating that the oldest command did not
	 * complete within the previous timeout interval.  Check to see if any
	 * progress has been made on the execution of the oldest command by
	 * sending a Get Command Status message out on the connection.
	 */
	} else {
	    Init_msg( mp, rp->rspid, NULL );
	    if( hrp->unitb != NULL )
		mp->mscp_unit = hrp->unitb->unit;
	    Unlock_connb (cp);
	    Unlock_tmob (cp);
	    mp->mscp_opcode = MSCP_OP_GTCMD;
	    mp->mscp_out_ref = *( u_int * )&cp->old_rspid;
#ifdef DEBUG_PRINT
printf(">>> mscp_conwatchdog: no command completed so send GTCMD msg\n");
#endif /* DEBUG_PRINT */
	    new_event = mscp_send_msg( rp );
	}

    /* No messages are active on the connection.  In order to keep
     * the controller from timing the host (us) out, send a Get
     * Unit Status command for unit 0 to give the controller a little
     * work to do.
     */
    } else {
        Unlock_tmob (cp);
	Init_msg( mp, rp->rspid, 0 );
	mp->mscp_opcode = MSCP_OP_GTUNT;
#ifdef DEBUG_PRINT
printf(">>> mscp_conwatchdog: no messages active so send GTUNT msg\n");
#endif /* DEBUG_PRINT */
	new_event = mscp_send_msg( rp );
    }

    /* Begin another timeout interval and exit.  Any errors encountered
     * above will cause redispatch to resynchronize the port.
     */
    cp->cmdtmo_intvl = cp->cnt_tmo;

#ifdef DEBUG_PRINT
printf(">>> mscp_conwatchdog: set cmdtmo_intvl to 0x%x\n", cp->cmdtmo_intvl);
#endif /* DEBUG_PRINT */

    return( new_event );

}

/**/

/*
 *
 *   Name:	mscp_conendmsg - Process a GTCMD end message.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	We lock the connection-time-out-lock and the ctrl_tout
 *		flag to synchronize this routine with mscp_conwatchdog ().
 *		If the flag is on, this means we have already decided to
 *		time out the connection; in this case we want, regardless
 *		of the end message status, to just return EV_NULL and
 *		abort this thread.  If there is no progress on the
 *		oldest outstanding command and thus we are going to
 *		return EV_EXRETRY, we must set ctrl_tout.
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_conendmsg( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    MSCP		*mp = rp->msgptr;


    /* If the command was a GTCMD, check the progress indicator in
     * the end message.
     */
    if( mp->mscp_endcode == ( MSCP_OP_GTCMD | MSCP_OP_END )) {

        Lock_tmob (cp);

	/* if the controller time out flag is set then we do not want
 	to do anything but kill off this thread. */
	if (! cp->flags.ctrl_tout) {

	Lock_connb (cp);

	/* If no progress has been made, error log the end message and 
	 * redispatch to break the connection.  Otherwise, update the
	 * command status.
	 */
	if( cp->old_cmd_sts <= mp->mscp_cmd_sts ) {
	    /* Error log the end message */

	    /* Since we are going to time out the connection, we set
	     * the ctrl_tout flag.
 	     */
	    cp->flags.ctrl_tout = 1;
	    Unlock_connb (cp);
	    Unlock_tmob (cp);
	    return( EV_EXRETRY );
	    } else {
	           cp->old_cmd_sts = mp->mscp_cmd_sts;
	           Unlock_connb (cp);
	           Unlock_tmob (cp);
	           }
	}  /* end if (! cp->flags.ctrl_tout) */ 
	else
	    {
	    Unlock_tmob (cp);
	    }
    }

    mscp_recycle_rspid( rp );
    return( EV_NULL );
}
/**/

/*
 *
 *   Name:	mscp_conresynch - Resynchronize with a failed (sub-)system.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect the connection block.  Also take out the tmob
 *		lock in order to synchronize the call to scs_crash_path ()
 *		with the decisions made by mscp_conwatchdog() and
 *		mscp_conendmsg().  To see why, see the discussion in the
 *		SMP comments at the end of mscp_defs.h
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_conresynch( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    MSB			*msp = ( MSB * )&cp->classb->msb;
    u_long		status;

    printf( "mscp\t- resynching " );
    if( cp->cnt_name != NULL ) {
	printf( "controller %s%d\n", cp->cnt_name, cp->cnt_number );
    } else {
	printf( "controller ***** at local port %4s ", &cp->lport_name );
	printf( "remote port %x\n", cp->rport_addr );
    }

    Lock_tmob (cp);
    Lock_connb (cp);   /* SMP protection */

    /* Turn off connection timeouts and fill in the MSB.
     */
    cp->cmdtmo_intvl = 0;
    msp->lport_name = cp->lport_name;
    Move_scaaddr( cp->rport_addr, msp->rport_addr );
    Move_name( cp->classb->cmsb.lproc_name, msp->Lproc_name )

    /* Crash the path to the remote system. If the remote system is a
     * hardware server, reset and restart it.  If the reset or restart
     * fails, check the reason for the failure.  RET_NOLPORT and RET_NOPATH
     * represent internal errors and result in panic.  Other errors may
     * represent transient lack of a resource and are retried indefinitely.
     */

    status = scs_crash_path( msp );

    if( cp->cnt_id.model != MSCP_CM_VMS &&
	cp->cnt_id.model != MSCP_CM_TOPS &&
	cp->cnt_id.model != MSCP_CM_ULTRIX32 ) {

	msp->Force = 1;
	if(( status = scs_reset( msp )) != RET_SUCCESS ) {
	    if( status == RET_NOLPORT || status == RET_NOPATH ) {
		cp->flags.ctrl_tout = 0;
		Unlock_connb (cp);
		Unlock_tmob (cp);
		panic( "mscp_conresynch: scs_reset failure.\n" );
	    } else {
		cp->cmdtmo_intvl = MAINT_TMO;
		cp->flags.ctrl_tout = 0;
		Unlock_connb (cp);
		Unlock_tmob (cp);
	        }

	} else {
	    msp->Startaddr = 0;
	    if(( status =  scs_restart( msp )) != RET_SUCCESS ) {
		if( status == RET_NOLPORT || status == RET_NOPATH ) {
		    cp->flags.ctrl_tout = 0;
		    Unlock_connb (cp);
		    Unlock_tmob (cp);
		    panic( "mscp_conresynch: scs_restart failure.\n" );
		} else {
		    cp->cmdtmo_intvl = MAINT_TMO;
		    cp->flags.ctrl_tout = 0;
		    Unlock_connb (cp);
		    Unlock_tmob (cp);
		}
	    }
	    else {
		 cp->flags.ctrl_tout = 0;
		 Unlock_connb (cp);
		 Unlock_tmob (cp);
		 }
	}
    }
    else
    {
    cp->flags.ctrl_tout = 0;
    Unlock_connb (cp);
    Unlock_tmob (cp);
    }

    return( EV_NULL );
}

/**/

/*
 *
 *   Name:	mscp_concleanup - Clean up after a lost connection.
 *
 *   Abstract:	This routine is entered when connection management is notified
 *		that a virtual path has failed or a disconnect has been
 *		received.  Deallocate all resources held by REQBs on the
 *		connection, and funnel the REQBs onto the restart queue.  Then
 *		attempt to restore the connection.
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	protect the queue manipulations
 *
 *		Lock the unit block structure whenever the unit's resource
 *		wait counter is manipulated.
 *
 *   Return	NONE
 *   Values:
 */
u_long
mscp_concleanup( event, rp )
    u_long		event;
    REQB		*rp;
{
    UNITB		*up = rp->unitb;
    CONNB		*cp = rp->connb;
    REQB		*wrp, *savwrp;
    CLASSB		*clp = rp->classb;
    CMSB		cmsb;
    u_long		status;
    u_long		new_event = EV_NULL;

#ifdef DEBUG_PRINT_OFF
printf(">>> mscp_concleanup: entry\n");
#endif /* DEBUG_PRINT */

    Lock_connb (cp);

    /* Turn off connection timeouts.
     */
    cp->cmdtmo_intvl = 0;

    /* Bump the resource wait reason count for each UNITB attached to
     * this CONNB in order to stall all new requests on the connection.
     */
    for( up = cp->unit.flink;
	 ( up != ( UNITB * )&cp->unit.flink );
	 up = up->flink ) {

	Lock_unitb (up);
		/* break if we have already bumped for this unit */
	if (up->flags.wait_bump) {
		Unlock_unitb (up);
		break;
	}
	up->flags.wait_bump = 1;
	up->rwaitct++;
	Unlock_unitb (up);
    }

    Unlock_connb (cp);

    /* Return resources held by the permanent request blocks.
     */
    mscp_dealloc_all( &cp->timeout_reqb );
    mscp_dealloc_all( &cp->polling_reqb );

    /* ***** NEED TO ADD CODE TO SCAN BBR WAIT QUEUE *****
     */

    /* Scan the RSPID wait queue looking for REQBs for the current 
     * connection.  Requeue each REQB in the restart queue.
     */
    Lock_rspid_db ();
    for( wrp = ( REQB * )mscp_rspid_wait_lh.flink;
	 wrp != ( REQB * )&mscp_rspid_wait_lh;
	 wrp = savwrp ) {

	/* save off flink because of wrp requeue on restart queue */
	savwrp = wrp->flink;

	/* If the REQB is for the current connection, remove it from
	 * the wait queue, decrement the wait reasons counter, and
	 * queue it on the restart queue.
	 */
	if( cp == wrp->connb ) {
	    Remove_entry( wrp->flink );
	    Decr_rwait( wrp );
	    Unlock_rspid_db ();
	    mscp_conqrestart( wrp );
	    Lock_rspid_db ();
	}
    }

    Unlock_rspid_db ();

    /* Drain the buffer, credit, and mapping wait queues into the
     * restart queue.  For each REQB encountered, decrement the
     * appropriate resource wait reasons counter before requeueing.
     */
    Lock_connb_buffwq (cp);
    for( wrp = cp->buffer_wq.flink;
	 wrp != (REQB *)&cp->buffer_wq;
	 wrp = cp->buffer_wq.flink ) {
	Remove_entry( wrp->flink );
	Decr_rwait( wrp );
	Unlock_connb_buffwq (cp);
	mscp_conqrestart( wrp );
	Lock_connb_buffwq (cp);
    }
    Unlock_connb_buffwq (cp);

    Lock_connb_credwq (cp);
    for( wrp = cp->credit_wq.flink;
	 wrp != (REQB *)&cp->credit_wq;
	 wrp = cp->credit_wq.flink ) {
	Remove_entry( wrp->flink );
	Decr_rwait( wrp );
	Unlock_connb_credwq (cp);
	mscp_conqrestart( wrp );
	Lock_connb_credwq (cp);
    }
    Unlock_connb_credwq (cp);

    Lock_connb_mapwq (cp);
    for( wrp = cp->map_wq.flink;
	 wrp != (REQB *)&cp->map_wq;
	 wrp = cp->map_wq.flink ) {
	Remove_entry( wrp->flink );
	Decr_rwait( wrp );
	Unlock_connb_mapwq (cp);
	mscp_conqrestart( wrp );
	Lock_connb_mapwq (cp);
    }
    Unlock_connb_mapwq (cp);

    /* Drain the queue of requests active in the controller into
     * the restart queue.
     */
    Lock_connb_actq (cp);
    for( wrp = cp->active.flink;
	 wrp != (REQB *)&cp->active;
	 wrp = cp->active.flink ) {
	Remove_entry( wrp->flink );
	Unlock_connb_actq (cp);
	mscp_conqrestart( wrp );
	Lock_connb_actq (cp);
    }
    Unlock_connb_actq (cp);
    
    /* Issue an SCS disconnect.  If the disconnect succeeds, and the
     * path had failed, the disconnect completes synchronously, so it
     * is possible to redispatch immediately to restart the connection.
     * Otherwise, the disconnect occurs asynchronously, in which case
     * we exit to await disconnect completion.
     */
    cmsb = clp->cmsb;
    cmsb.Reason = ADR_SUCCESS;
    Lock_connb (cp);
    Move_connid( cp->connid, cmsb.connid );
    if(( status = scs_disconnect( &cmsb )) == RET_SUCCESS ) {
	if( cp->flags.path_fail ) {
	    cp->flags.path_fail = 0;
	    cp->flags.restart = 1;
	    new_event = EV_ERRECOV;
	}
    /* The disconnect failed.  Shouldn't happen.  Panic.
     */
    } else {
	panic( "mscp_concleanup: disconnect failed\n" );
    }

    Unlock_connb (cp);

    return( new_event );
}


/**/

/*
 *
 *   Name:	mscp_condisccmplt - process completed disconnect.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect the conntection block.
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_condisccmplt( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;

    /* Mark the connection block as being restarted and redispatch to
     * attempt reconnection.
     */
    Lock_connb (cp);
    cp->flags.restart = 1;
    Unlock_connb (cp);
    return( EV_ERRECOV );
}
/**/

/*
 *
 *   Name:	mscp_condealmsg - Deallocate message buffers during restart.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	protect the connection block.
 *
 *   Return	NONE
 *   Values:
 */
 
u_long
mscp_condealmsg( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    u_long		scs_status;

    /* Return the buffer to SCS ignoring any error that may occur.
     */
    Lock_connb (cp);
    Move_connid( cp->connid, csp->connid );
    csp->buf = ( u_char * )rp->msgptr;
    Unlock_connb (cp);
    scs_status = scs_dealloc_msg( csp );
    rp->msgptr = NULL;
    return( EV_NULL );

}
/**/

/*
 *
 *   Name:	mscp_consubr - Issue scs_connect and analyze return status
 *
 *   Abstract:	This routine attempts to make an SCS connection and
 *		analyzes the return status.  If the connect attempt
 *		starts successfully, the connection state is updated.
 *
 *   Inputs:	rp			Request block
 *		    connb		Connection block pointer
 *
 *   Outputs:	cp			Connection block
 *		    state		Connection state
 *
 *   SMP:	Protect the connection block.
 *
 *   Return
 *   Values:	We don't actually return a value of the scs_connect ().
 *		The connect will either be successful, or else the connection
 *		time out mechansim will handle the failed connect.
 *
 */
void
mscp_consubr( rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CMSB		cmsb;
    CMSB		*cmsp = &cmsb;
    u_long		status;

    /* Set up the connection timeout period and fill in the CMSB.
     */
    Lock_connb (cp);
    cp->cmdtmo_intvl = CONNECT_TMO;
    cmsb = cp->classb->cmsb;
    cmsp->lport_name = cp->lport_name;
    Move_scaaddr( cp->rport_addr, cmsp->rport_addr );
    Move_scaaddr( cp->sysid, cmsp->sysid );
    Unlock_connb (cp);
    cmsp->aux = ( u_char * )cp;
    /* Attempt to make a connection via SCS. Exit to wait for the connection
     * to complete or for the connection timeout to occur.
     */
    status = scs_connect( cmsp );

    return;
}
/**/

/*
 *
 *   Name:	mscp_confmtstcon - Format a a STCON command message.
 *
 *   Abstract:	Initialize a MSCP message buffer as a set controller
 *		characteristics message and set up a temporary command
 *		timeout.
 *
 *   Inputs:	rp			Request block pointer
 *		    msgptr		Message buffer pointer
 *		    connb		Connection block pointer
 *
 *   Outputs:	mp->mscp_		Message buffer pointer
 *		    cmd_ref		Response ID value
 *		    opcode		STCON opcode value
 *		    version		MSCP_VERSION (currently 0)
 *		    cnt_flgs		Controller flags from connb
 *		    time		Boot time in VMS format
 *		Other message buffer fields are zeroed.
 *
 *   SMP:	Assume that the relevent connection block has been
 *		locked.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_confmtstcon( rp, hst_tmo )
    REQB		*rp;
    u_short		hst_tmo;
{
    CONNB		*cp = rp->connb;
    MSCP		*mp = rp->msgptr;

    Init_msg( mp, rp->rspid, NULL );
    mp->mscp_opcode = MSCP_OP_STCON;
    mp->mscp_version = MSCP_VERSION;
    mp->mscp_cnt_flgs = cp->cnt_flgs;
    mp->mscp_hst_tmo = hst_tmo;
    scs_unix_to_vms(( struct timeval * )&boottime,
		    ( u_int * )&mp->mscp_time[ 0 ] );
    if(( cp->cmdtmo_intvl = cp->cnt_tmo ) == 0 )
	cp->cmdtmo_intvl = IMMEDIATE_TMO;
    return;
}

/**/

/*
 *
 *   Name:	mscp_constrstcon - Store results of STCON end message.
 *
 *   Abstract:	Store the contents of the set controller characteristics
 *		end message in the controller block.
 *
 *   Inputs:	rp		Request block pointer
 *		    msgptr	MSCP end message pointer
 *		    connb	Controller block pointer
 *		    csb		Communications services block pointer
 *
 *   Outputs:
 *
 *   SMP:	Protect the connection block.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_constrstcon( rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    MSCP		*mp = rp->msgptr;

    /* Pad the STCON end message out to its full length, then
     * copy its contents into the connb.  If the controller
     * timeout returned in the end message is 0, store a default
     * timeout in the connection block.  If the optional maximum
     * byte count field is supplied, use it.  Otherwise, store the
     * MSCP specification default for disk or tape, depending on
     * the type of device being served.
     */
    Pad_msg( mp, rp->msgsize );
    Lock_connb (cp);
    cp->version = mp->mscp_version;
    cp->cnt_flgs = mp->mscp_cnt_flgs;
    if(( cp->cnt_tmo = mp->mscp_cnt_tmo ) == 0 ) {
	cp->cnt_tmo = 255 - mscp_htmo_overhead;
    }
    cp->cnt_svr = mp->mscp_cnt_svr;
    cp->cnt_hvr = mp->mscp_cnt_hvr;
    cp->cnt_id = *( UNIQ_ID * )&mp->mscp_cnt_id[ 0 ];
    if(( cp->max_bcnt = mp->mscp_max_bcnt ) == 0 ) {
	if( cp->classb->flags.disk ) {
	    cp->max_bcnt = ( 1 << 24 );
	} else {
	    cp->max_bcnt = (( 1 << 16 ) - 1 );
	}
    }
    Unlock_connb (cp);
    return;
}

/**/

/*
 *
 *   Name:	mscp_conqrestart - Queue a REQB on the restart queue
 *
 *   Abstract:	Queue a request block on the restart queue in sequence
 *		number order, after first deallocating all the resources
 *		that it holds.
 *
 *   Inputs:	rp			REQB pointer.
 *
 *   Outputs:
 *
 *   SMP:	Protect manipulations of the restart queue by locking
 *		the relevent connection block. 
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_conqrestart( rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    REQB		*qrp;

    /* Deallocate all the resources held by this REQB.
     */
    mscp_dealloc_all( rp );


    /* Bypass queuing of the driver internal (permanent) request blocks.
     */
    if( !rp->flags.perm_reqb ) {

    Lock_connb (cp);

	/* Find a REQB with a sequence number not less than the one we
	 * wish to insert.
	 */
	for( qrp = cp->restart.flink;
	     qrp != ( REQB * )&cp->restart &&
	     rp->op_seq_num >= qrp->op_seq_num;
	     qrp = qrp->flink )
	    ;

	/* Sequence numbers should not be duplicated, so if we find two
	 * that are equal, panic.
	 */
	if( rp->op_seq_num == qrp->op_seq_num ) {
	    panic( "mscp_conqrestart: duplicate sequence numbers\n" );
	    }

	/* Insert the REQB in the restart queue in sequence number order.
	 */
	Insert_entry( rp->flink, *qrp );

    Unlock_connb (cp);

    }

    return;

}

/**/

/*
 *
 *   Name:	mscp_consetretry - Reinitialize the connection retry count.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect the connection block.  Protect the class block.
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_consetretry( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CLASSB              *clp = rp->classb;
 

    Lock_classb (clp);
    clp->flags.need_poll = 1;
    Unlock_classb (clp);

    /* Set a long timeout interval, reset the connection retry count, and
     * call back the routine that started this connection management thread.
     */
    Lock_connb (cp);
    cp->cmdtmo_intvl = DEAD_TMO;
    cp->retry_count = CONNECT_RETRIES;
    Unlock_connb (cp);

    return( EV_NULL );
}
/**/

/*
 *
 *   Name:	mscp_polinit - Format and send first GTUNT/next command.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect connection block.
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_polinit( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    MSCP		*mp = rp->msgptr;

    /* Set the unit polling in progress flag in the connection block,
     * initialize the mscp message buffer setting the unit number to 1 and
     * format it as a get unit status message, then queue it to be sent out
     * on the connection.  
     */
    Lock_connb (cp);
    cp->flags.upoll_busy = 1;
    Init_msg( mp, rp->rspid, 1 );
    mp->mscp_opcode = MSCP_OP_GTUNT;
    mp->mscp_modifier = MSCP_MD_NXUNT;
    /* Send the GTUNT command message and wait for the end message to
     * come back.
     */
    Unlock_connb (cp);

    return(mscp_send_msg(rp));
}
/**/

/*
 *
 *   Name:	mscp_polgtuntem - Find all units attached to a system
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect the connection block.
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_polgtuntem( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    MSCP		*mp = rp->msgptr;
    UNITB		*up;
    u_long		code, subcode;

    Lock_connb (cp);

    /* Pad the GTUNT end message out to the maximum message size,
     * and store the updated unit number in the connection block.
     */
    Pad_msg( mp, rp->msgsize );
    cp->cur_unit = mp->mscp_unit;

    /* Break apart the end message status for analysis.
     */
    code = mp->mscp_status & MSCP_ST_MASK;
    subcode = mp->mscp_status >> MSCP_ST_SBBIT;

    /* If the unit is in a state in which we can potentially
     * do operations to it, find/allocate and initialize a unit
     * block for the unit.
     */
    if( code == MSCP_ST_SUCC  ||
	code == MSCP_ST_AVLBL ||
	code == MSCP_ST_DRIVE ||
	( code == MSCP_ST_OFFLN &&
	    ( subcode == MSCP_SC_NOVOL || subcode == MSCP_SC_EXUSE ))) {
	if(( up = mscp_get_unitb( cp, mp, rp->msgsize )) != NULL ) {

	    /* Provide a unit block pointer for the connection permanent
	     * request blocks.
	     */
	    cp->timeout_reqb.unitb = up;
	    cp->polling_reqb.unitb = up;

	/* The unit block could not be allocated.  Set a flag to cause unit
	 * polling to be retried after a few seconds.
	 */
	} else {
	    printf("mscp_polgtuntem: couldn't allocate a unitb\n");
	    cp->flags.need_upoll = 1;
	}
    }

    /* If the updated unit number is not zero, we have more units to poll.
     * Increment the unit number, initialize the mscp message buffer and format
     * it as a get unit status message, then queue it to be sent out on the 
     * connection.  
     */

    if( cp->cur_unit != 0 && cp->flags.need_upoll == 0 ) {
	Unlock_connb (cp);
	mscp_recycle_rspid( rp );
	Init_msg( mp, rp->rspid, ++cp->cur_unit );
	mp->mscp_opcode = MSCP_OP_GTUNT;
	mp->mscp_modifier = MSCP_MD_NXUNT;

	/* Send the GTUNT command message and wait for the end message to
	 * come back.
	 */
	return( mscp_send_msg( rp ));

    /* All units on the connection have been polled.  Deallocate all
     * the resources held by the request block, reset the unit polling
     * in progress flag in the connection block, dispatch connection
     * management with a polling complete event, and terminate the polling
     * thread.
     */
    } else {
	cp->flags.upoll_busy = 0;
	Unlock_connb (cp);
	mscp_dealloc_all( rp );
	mscp_dispatch( EV_POLLCOMPLETE, ( REQB * )&cp->timeout_reqb );
	return( EV_NULL );
    }
}
/**/

/*
 *
 *   Name:	mscp_get_connb - Find or allocate/initialize connection block
 *
 *   Abstract:	
 *
 *   Inputs:	Class block is locked.
 *
 *   Outputs:	Class block is locked.
 *
 *   SMP:	when we make a new connection block, initialize its
 *		connb and tmob smp locks.  Also, assume that the caller
 *		has locked the class block.
 *
 *   Return	NONE
 *   Values:
 */

/* ?? is there some reason to update the s/w incarnation ?? */

CONNB *
mscp_get_connb( clp, sysid_ptr, rport_addr_ptr, lport_name )
    CLASSB		*clp;
    c_scaaddr		*sysid_ptr;
    c_scaaddr		*rport_addr_ptr;
    u_long		lport_name;
{
    CONNB		*cp;

    /* Check to see if a connection block for the system exists in the 
     * data base already.  If so, use it.
     */
    for( cp = clp->flink; cp != ( CONNB * )&clp->flink; cp = cp->flink ) {
	if( Comp_scaaddr( cp->sysid, *sysid_ptr ))
	    break;
    }

    /* If the system isn't already known, attempt to allocate and
     * fill in a new connection block.
     */
    if( cp == ( CONNB * )&clp->flink ) {
	SCA_KM_ALLOC( cp, CONNB *, sizeof( CONNB ), KM_SCA, KM_NOW_CL_CA );
	if( cp != NULL ) {
	    mscp_polls++;
	    cp->classb = clp;
	    cp->unit.flink = ( UNITB * )&cp->unit.flink;
	    cp->unit.blink = ( UNITB * )&cp->unit.flink;
	    cp->active.flink = ( REQB * )&cp->active.flink;
	    cp->active.blink = ( REQB * )&cp->active.flink;
	    cp->restart.flink = ( REQB * )&cp->restart.flink;
	    cp->restart.blink = ( REQB * )&cp->restart.flink;
	    cp->credit_wq.flink = ( REQB * )&cp->credit_wq.flink;
	    cp->credit_wq.blink = ( REQB * )&cp->credit_wq.flink;
	    cp->buffer_wq.flink = ( REQB * )&cp->buffer_wq.flink;
	    cp->buffer_wq.blink = ( REQB * )&cp->buffer_wq.flink;
	    cp->map_wq.flink = ( REQB * )&cp->map_wq.flink;
	    cp->map_wq.blink = ( REQB * )&cp->map_wq.flink;
	    cp->flags.ctrl_tout = 0;
	    cp->flags.serv_cred = 0;
	    cp->flags.top_serv =  0;
	    cp->cnt_flgs = MSCP_CF_ATTN | MSCP_CF_MISC | MSCP_CF_THIS;
	    cp->timeout_reqb.classb = clp;
	    cp->timeout_reqb.connb = cp;
	    cp->timeout_reqb.state = ST_CN_INITIAL;
	    cp->timeout_reqb.state_tbl = mscp_con_states;
	    cp->timeout_reqb.flags.perm_reqb = 1;
	    cp->timeout_reqb.flags.nocreditw = 1;
	    cp->polling_reqb.classb = clp;
	    cp->polling_reqb.connb = cp;
	    cp->polling_reqb.state = ST_UP_INITIAL;
	    cp->polling_reqb.state_tbl = mscp_pol_states;
	    cp->polling_reqb.flags.perm_reqb = 1;
	    Init_tmob_lock (cp);
	    Init_connb_mapwq_lock (cp);
	    Init_connb_buffwq_lock (cp);
	    Init_connb_credwq_lock (cp);
	    Init_connb_actq_lock (cp);
	    Init_connb_lock (cp);

	    Insert_entry( cp->flink, *clp );
	    ++clp->system_ct;
	}
    }

    /* If we have a connection block in hand, start up a connection
     * management thread for it.
     */
    if( cp != NULL ) {
	Move_scaaddr( *sysid_ptr, cp->sysid );
	Move_scaaddr( *rport_addr_ptr, cp->rport_addr );
	cp->lport_name = lport_name;
	mscp_dispatch( EV_INITIAL, ( REQB * )&cp->timeout_reqb );
    }

    return( cp );
}

/**/

/*
 *
 *   Name:	mscp_get_unitb - Find or allocate a unit block.
 *
 *   Abstract:	Scan the unit list for a connection looking for a unit number
 *		that matches cur_unit.  If there is no	match, allocate and
 *		initialize a new UNITB and thread it into the unit list (which
 *		is maintained in ascending sequence number order.)  Return the
 *		address of the UNITB to the caller.  If the UNITB could not be
 *		allocated, return NULL to the caller.
 *
 *   Inputs:	cp			Connection block pointer
 *		mp			MSCP message pointer
 *		msg_size		MSCP message size
 *		connection block is smp locked.
 *
 *   Outputs:	connection block remains locked.
 *
 *   SMP:	If we make a new unit block, initialize the unit block's
 *		SMP lock.  Also, assume that the caller has locked the
 *		connection block.
 *
 *   Side	A new unit block may be initialized and threaded into the
 *   Effects:	unit list.
 *
 *   Return	up			Unit block pointer if UNITB was found
 *   Values:				or successfully allocated; else, NULL.
 */

UNITB *
mscp_get_unitb( cp, mp, msg_size )
    CONNB		*cp;
    MSCP		*mp;
    u_long		msg_size;
{
    UNITB		*up, *next_up;

    /* Search for an existing unit block with a unit number equal to the
     * cur_unit field in the connection block.  Note that when the for loop
     * completes, the unit block pointer is pointing at the listhead or at
     * an entry whose unit number is >= the search argument.
     */
    for( up = cp->unit.flink;
	 up != ( UNITB * )&cp->unit.flink && mp->mscp_unit > up->unit;
	 up = up->flink ) {}

    /* If the unit queue is empty or no matching unit block was found,
     * save the address of the next unit block and attempt to allocate
     * and initialize a new unit block.
     */
    if( up == ( UNITB * )&cp->unit.flink || mp->mscp_unit != up->unit ) {
	next_up = up;
	SCA_KM_ALLOC( up, UNITB *, sizeof( UNITB ), KM_SCA, KM_NOW_CL_CA );
	if( up != NULL ) {
	    up->connb = cp;
	    up->request.flink = ( REQB * )&up->request.flink;
	    up->request.blink = ( REQB * )&up->request.flink;
	    up->unit = mp->mscp_unit;

	    up->mult_unt = mp->mscp_mult_unt;
	    up->unt_flgs = mp->mscp_unt_flgs;
	    up->unit_id = *( UNIQ_ID * )mp->mscp_unit_id;
	    up->media_id = mp->mscp_media_id;
	    up->unit_svr = mp->mscp_unit_svr;
	    up->unit_hvr = mp->mscp_unit_hvr;
	    up->unt_flgs = mp->mscp_unt_flgs & MSCP_UF_RMVBL;
	    up->rawb_busy = 0;
	    Init_unitb_lock (up);

	
	    /* Link the new unit block into the connection queue in unit
	     * number sequence.
	     */
	    Insert_entry( up->flink, *next_up );
 
	    /* If the controller is present in the configuration tables,
	     * try to connect the unit to the configuration tables.
	     */
	    if( cp->ubctlr ) {
		mscp_find_device( up );
	    }
	}
    }

    /* Return the address of the unit block (or NULL) to the caller.
     */
    return( up );
}
