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
static char *rcsid = "@(#)$RCSfile: scs_error.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 15:21:36 $";
#endif
/*
 * derived from scs_error.c	4.1	(ULTRIX)	7/2/90";
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Systems Communication Services
 *
 *   Abstract:	This module contains Systems Communication Services( SCS )
 *		error processing routines and functions.
 *
 *   Creator:	Todd M. Katz	Creation Date:	February 10, 1989
 *
 *   Function/Routines:
 *
 *   scs_abort_conn		Abort Establishment of SCS Connection
 *   scs_console_log		Log SCS Events to Console Terminal
 *   scs_log_event		Log Local SCS SYSAP/Connection Specific Events
 *
 *   Modification History:
 *
 *   31-Oct-1991	Pete Keilty
 *	Add OSF ifdef UERF around errlog ealloc calls.
 *
 *   15-Mar-1991	Brian Nadeau
 *	Port to OSF
 *
 *   19-Sep-1989	Pete Keilty
 *	Added SCS errlogging level support.
 *
 *   13-May-1989	Todd M. Katz		TMK0003
 *	Include header file smp_lock.h and rename external variable
 *	sca_lk_db -> lk_scadb.
 *
 *   22-Mar-1989	Todd M. Katz		TMK0002
 *	Fix several problems within the routine scs_log_event():
 *	1. Correctly position to optional SCS event portions of error log
 *	   records.
 *	2. Correctly initialize remote system node name, remote system
 *	   identification number, and remote station address fields within
 *	   optional SCS connection information portions of error log records.
 *
 *   11-Feb-1989	Todd M. Katz		TMK0001
 *	Add support for SCS event logging.  Log local SYSAP initiated abortions
 *	of SCS connection establishment.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<dec/binlog/errlog.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>

/* External Variables and Routines.
 */
extern	SCSIB		lscs;
extern	struct slock	lk_scadb;
extern	CBVTDB		*scs_cbvtdb;
extern	u_short		scs_severity;
extern	u_short		scs_errlog;
extern	CLSTAB		scs_cltab[ ES_W + 1 ];
extern	void		scs_console_log(), scs_dealloc_cb(), scs_init_cmsb(),
			scs_log_event();

/*   Name:	scs_abort_conn	- Abort Establishment of SCS Connection
 *
 *   Abstract:	This routine completes the abortion of logical SCS connection.
 *		The SYSAP initiating and aborting the connection is
 *		asynchronously notified of the abortion of connection
 *		establishment through invocation of its control event routine.
 *
 *		An abortion is triggered only by a SYSAP requesting termination
 *		of a connection it had initiated before its counterpart has
 *		responded by accepting or rejecting the connection request.
 *		This routine is always invoked by forking to it. 
 *
 *		Connection abortion involves CB removal from all internal
 *		resource queues and the system-wide configuration database and
 *		its deallocation along with all associated resources.  The
 *		CBVTE referencing the CB is also deallocated.
 *
 *   Inputs:
 *
 *   IPL_SOFTCLOCK		- Interrupt processor level
 *   cb				- Connection Block pointer
 *	cinfo.cstate		-  CS_CONN_SNT or CS_CONN_ACK
 *	cinfo.status.abort_fork	-  1
 *   lk_scadb			- SCA database lock structure
 *
 *   Outputs:
 *
 *   IPL_SOFTCLOCK		- Interrupt processor level
 *   pb				- Path Block pointer
 *	pinfo.nconns		-  Number of connections
 *
 *   SMP:	The SCA database is locked for CB removal from the system-wide
 *		configuration database and CBVTE deallocation.
 *
 *		The CB is locked to synchronize access and for deletion.  It is
 *		indirectly locked through its CBVTE.  Locking the CB also
 *		prevents PB deletion as required by scs_init_cmsb().
 */
void
scs_abort_conn( id )
    u_long		id;
{
    CMSB		cmsb;
    CB			*cb;
    CBVTE		*cbvte;
    void		( *control )();
    u_long		save_ipl = Splscs();

    /* The steps involved in aborting SCS connection establishment are:
     *
     *  1. IPL is synchronized to IPL_SCS( IPL was lowered through forking ).
     *  2. Lock the SCA database.
     *  3. Retrieve and lock the CB.
     *  4. Log abortion of SCS connection establishment.
     *  5. Close the aborted connection.
     *  6. Remove the CB from the system-wide configuration database.
     *  7. Deallocate the CB along with all associated resources.
     *  8. Unlock the CB.
     *  9. Unlock the SCA database.
     * 10. Notify the local SYSAP of the abortion of connection establishment.
     * 11. Restore IPL before returning.
     *
     * Failure to retrieve the CB is not treated as an error.  The path over
     * which the connection was to be established is assumed to have failed and
     * the CB deallocated during the PB clean up following path failure.
     */
    Lock_scadb();
    if((( CONNID * )&id )->index > ( lscs.max_conns - 1 )) {
	( void )panic( SCSPANIC_ABORT );
    }
    cbvte = Get_cbvte((*( CONNID * )&id));
    Lock_cbvte( cbvte );
    if((( CONNID * )&id )->seq_num == cbvte->connid.seq_num ) {
	cb = Get_cb( cbvte );
	if( !cb->cinfo.status.abort_fork ||
	     ( cb->cinfo.cstate != CS_CONN_SNT &&
		cb->cinfo.cstate != CS_CONN_ACK )) {
	    ( void )panic( SCSPANIC_NABORT );
	}
	( void )scs_init_cmsb( CRE_CONN_DONE,
			       ADR_DISCONN,
			       &cmsb,
			       cb,
			       cb->pb,
			       0 );
	control = cb->control;
	( void )scs_log_event( cb, W_ABORT_FCONN, CONN_EVENT );
	Remove_cb( cb, cb->pb )
    } else {
	cb = NULL;
    }
    Unlock_cbvte( cbvte )
    Unlock_scadb()
    if( cb ) {
	( void )( *control )( CRE_CONN_DONE, &cmsb );
    }
    ( void )splx( save_ipl );
}

/*   Name:	scs_console_log	- Log SCS Events to Console Terminal
 *
 *   Abstract:	This routine logs SCS events to the console terminal.  The
 *		event is always one of the following types:
 *
 *		CONN_EVENT	- Connection specific event
 *		LSYSAP_EVENT	- Local SYSAP specific event
 *
 *		Explicit formatting information must be provided for each
 *		event.  This requires updating of the following tables each
 *		time a new event is defined:
 *
 *		1. The appropriate entry within the SCS console logging table(
 *		   scs_cltab[] ) must be updated to reflect the new maximum
 *		   code represented within the associated format table.
 *
 *		2. The associated format table itself( scs_cli[], scs_clw[] )
 *		   must be updated with both the class of variable information
 *		   and exact text to be displayed.  However, the appropriate
 *		   table should be updated with a NULL entry when SCS is
 *		   specifically NOT to log the new event.  Currently, no such
 *		   events fall into this category.
 *
 *		NOTE: Console logging of events is bypassed whenever the event
 *		      severity does not warrant console logging according to
 *		      the current SCS severity level( scs_severity ).  Such
 *		      bypassing is overridden when the ECLAWAYS bit is set in
 *		      the event code indicating that the event is always to be
 *		      logged regardless of the current severity level.
 *
 *		NOTE: This routine does not log path specific error or severe
 *		      error events, all of which are candidates for application
 *		      of the  path crash severity modifier( ESM_PC ).  Logging
 *		      of such events is currently the responsibility of the
 *		      individual port drivers.  
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *   event			- Event logging code
 *   event_type			- CONN_EVENT, LSYSAP_EVENT
 *   scs_cltab			- SCS Console logging formatting table
 *   scs_severity		- SCS console logging severity level
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	The CB is locked( EXTERNALLY ) to synchronize access and
 *		prevent premature deletion.  It is locked through its CBVTE.
 *
 *		PBs and PCCBs do NOT require locking when provided because only
 *		static fields are accessed and because PBs are prevented from
 *		deletion be EXTERNAL CB locks.  SBs NEVER require locking.
 */
void
scs_console_log( cb, event, event_type )
    CB			*cb;
    u_long		event;
    u_long		event_type;
{
    SB			*sb;
    PB			*pb;
    u_long		severity = Eseverity( event );

    /* Console messages for connection specific SCS events always display
     * the local and remote SYSAP names, local port name, local and remote port
     * station addresses, and remote system name by default.  Console messages
     * for local SYSAP specific events always display the local SYSAP name by
     * default.
     *
     * A panic occurs whenever SCS is not prepared to log the event due to
     * detection of any of the following circumstances:
     *
     * 1. The event type is unknown.
     * 2. The event is not a SCS specific event.
     * 3. The severity level of the event is invalid.
     * 4. The code of the event exceeds the current maximum known code for the
     *	  severity level of the event.
     * 5. The event is not represented within the appropriate console logging
     *    formatting table( indicating that SCS should never have been asked to
     *	  log it in the first place ).
     * 6. The class of variable information associated with the event is
     *	  unknown.
     *
     * None of these circumstances should ever occur.
     *
     * NOTE: Currently all SCS events are logged without displaying any
     *	     variable information.
     *
     * NOTE: Events represented within console logging format tables by NULL
     *	     entries are events which are always to be logged only by
     *	     individual port drivers and never by SCS.  Currently, no such
     *	     warning events fall within this category.
     */
    if( event_type > LSYSAP_EVENT				||
	 !Test_scs_event( event )				||
	 severity > ES_W					||
	 Ecode( event ) > Scs_clmaxcode( scs_cltab, event )	||
	 Scs_cltabmsg( scs_cltab, event ) == NULL ) {
	( void )panic( SCSPANIC_UNKCOD );
    } else if( Scs_cltabcode( scs_cltab, event ) > CF_NONE ) {
	( void )panic( SCSPANIC_UNKCF );
    } else if( scs_severity > severity && !Test_cloverride( event )) {
	return;
    }
    ( void )printf( "scs\t- %s\n", Scs_cltabmsg( scs_cltab, event ));

    switch( event_type ) {

	case CONN_EVENT:
	    ( void )printf( "\t- local/remote sysap: %16s/%16s\n",
			     cb->cinfo.lproc_name,
			     cb->cinfo.rproc_name );
	    if(( pb = cb->pb )) {
		sb = pb->sb;
		( void )printf( "\t- path( local( %4s )/remote port: %u/%u",
				 &pb->pccb->lpinfo.name,
				 Scaaddr_low( pb->pccb->lpinfo.addr ),
				 Scaaddr_low( pb->pinfo.rport_addr ));
	    } else {
		sb = NULL;
		( void )printf( "\t- path( local( ? )/remote port: ?/?" );
	    }
	    if( sb ) {
		( void )printf( ", remote system: %8s", sb->sinfo.node_name );
	    } else {
		( void )printf( ", remote system: ?" );
	    }
	    break;

	case LSYSAP_EVENT:
	    ( void )printf( "\t- local sysap: %16s", cb->cinfo.lproc_name );
	    break;
    }
    switch( Scs_cltabcode( scs_cltab, event )) {

	case CF_NONE:
	    ( void )printf( "\n" );
	    break;
    }
}

/*   Name:	scs_log_event	- Log Local SYSAP/Connection Specific Events
 *
 *   Abstract:	This routine logs SCS local SYSAP and connection specific
 *		events.  Local SYSAP events are the result of specific SYSAP
 *		actions.  Connection specific events may result either from
 *		direct or indirect SYSAP actions; or, as a consequence of
 *		port driver related events.  The event is also optionally
 *		logged to the console.
 *
 *		Two classes of events are currently logged by this routine:
 *
 *		1. Changes in connection state.
 *		2. Local SYSAP actions.
 *
 *		None of these events are serious in nature.  They are logged
 *		to give a warning or for informational purposes only.
 *
 *		NOTE: This routine does not log path specific error or severe
 *		      error events, all of which are candidates for application
 *		      of the  path crash severity modifier( ESM_PC ).  Logging
 *		      of such events is currently the responsibility of the
 *		      individual port drivers.  
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *	errlogopt.rreason	-  SYSAP/SCS connection rejection reason
 *   event			- Event code
 *   event_type			- CONN_EVENT, LSYSAP_EVENT
 *   lscs			- Local system permanent information
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	The CB is locked( EXTERNALLY ) to synchronize access and
 *		prevent premature deletion.  It is locked through its CBVTE.
 *
 *		PBs and PCCBs do NOT require locking when provided because only
 *		static fields are accessed and because PBs are prevented from
 *		deletion be EXTERNAL CB locks.  SBs NEVER require locking.
 */
void
scs_log_event( cb, event, event_type )
    CB			*cb;
    u_long		event;
    u_long		event_type;
{
    PCCB		*pccb;
    u_char		*celp;
    struct el_rec	*elp;
    u_long	        severity = Eseverity( event );

    /* The steps involved in logging non-path specific SCS events include:
     *
     * 1. Logging the event to the console.
     * 2. Computing the size of the application portion of the event log
     *	  record.
     * 3. Allocating an event log record and initializing the record's sub id
     *	  packet fields.
     * 4. Initializing the portion of the record common to all non-path related
     *	  SCS events.
     * 5. Moving any optional information into the record.
     * 6. Validating the event log record.
     *
     * The ability of SCS to log the event is validated during console logging(
     * Step 1 ).  A panic occurs whenever SCS is not prepared to log the
     * reported event.
     *
     * This routine immediately terminates on failure to allocate an event log
     * record( Step 3 ).
     *
     * The size of the application portion of each event log record( Step 2 )
     * is dependent upon the presence or absence of optional information to be
     * included within the record( Step 5 ).  The following types of optional
     * information may be logged:
     *
     * 1. SCS connection information.
     * 2. Local directory identification number.
     * 3. Remote SYSAP/SCS connection reject reason.
     *
     * Sufficient space is reserved within the event record for such optional
     * information only for those event which require its logging.  Logging of
     * SCS connection information is required by all connection specific
     * events.  Logging of the local directory identification number is
     * currently required by all local SYSAP specific events.  Logging of the
     * remote SYSAP/SCS connection reject reason is required by two events:
     * W_REJECT_FCONN and W_TERM_FCONN.
     */
    ( void )scs_console_log( cb, event, event_type );
    if( scs_errlog > severity && 
        !Test_cloverride( event ) &&
	scs_errlog < SCA_ERRLOG3 ) {
	    return;
    }
    {
    u_long		size;

    size = sizeof( struct scs_common );
    if( event_type == CONN_EVENT ) {
	pccb = cb->pccb;
	size += sizeof( struct scs_conn );
	if( event == W_TERM_FCONN || event == W_REJECT_FCONN ) {
	    size += sizeof( u_int );
	}
    } else {
	pccb = NULL;
	size += sizeof( u_short );
    }
    if(( elp = ealloc( size, EL_PRIHIGH )) == EL_FULL ) {
	return;
    }
    LSUBID( elp,
	    ELSW_SCS,
	    EL_UNDEF,
	    (( pccb ) ? pccb->lpinfo.type.hwtype : EL_UNDEF ),
	    (( pccb ) ? Ctrl_from_name( pccb->lpinfo.name ) : EL_UNDEF ),
	    EL_UNDEF,
	    event )
    Elscscommon( elp )->scs_optfmask1 = 0;
    Elscscommon( elp )->scs_optfmask2 = 0;
    Elscscommon( elp )->scs_evpktver = SCS_EVPKTVER;
    Move_name( cb->cinfo.lproc_name, Elscscommon( elp )->scs_lsysap )
    Move_name( cb->cinfo.lconn_data, Elscscommon( elp )->scs_lconndata )
    Move_connid( cb->cinfo.lconnid, Elscscommon( elp )->scs_lconnid )
    Move_node( lscs.system.node_name, Elscscommon( elp )->scs_lname )
    Move_scaaddr( lscs.system.sysid, *Elscscommon( elp )->scs_lsysid )
    Elscscommon( elp )->scs_cstate = cb->cinfo.cstate;
    celp = ( u_char * )( Elscscommon( elp ) + 1 );
    if( event_type == CONN_EVENT ) {
	PB	*pb = cb->pb;

	Elscscommon( elp )->scs_optfmask1 |= ( SCS_CLTDEVTYP |
					       SCS_CLTDEVNUM |
					       SCS_CONN );
	Move_name( cb->cinfo.rproc_name, Elscsconn( celp )->scs_rsysap )
	Move_name( cb->cinfo.rconn_data, Elscsconn( celp )->scs_rconndata )
	Move_connid( cb->cinfo.rconnid, Elscsconn( celp )->scs_rconnid )
	if( pb && pb->sb ) {
	    Move_node( pb->sb->sinfo.node_name, Elscsconn( celp )->scs_rname )
	    Move_scaaddr( pb->sb->sinfo.sysid, *Elscsconn( celp )->scs_rsysid )
	} else {
	    U_int( Elscsconn( celp )->scs_rname[ 0 ]) = EL_UNDEF;
	    U_int( Elscsconn( celp )->scs_rname[ 4 ]) = EL_UNDEF;
	    U_short( Elscsconn( celp )->scs_rsysid[ 0 ]) = EL_UNDEF;
	    U_short( Elscsconn( celp )->scs_rsysid[ 2 ]) = EL_UNDEF;
	    U_short( Elscsconn( celp )->scs_rsysid[ 4 ]) = EL_UNDEF;
	}
	if( pb ) {
	    Move_scaaddr( pb->pinfo.rport_addr, *Elscsconn( celp )->scs_rsaddr)
	    Elscsconn( celp )->scs_nconns = pb->pinfo.nconns;
	} else {
	    U_short( Elscsconn( celp )->scs_rsaddr[ 0 ]) = EL_UNDEF;
	    U_short( Elscsconn( celp )->scs_rsaddr[ 2 ]) = EL_UNDEF;
	    U_short( Elscsconn( celp )->scs_rsaddr[ 4 ]) = EL_UNDEF;
	    Elscsconn( celp )->scs_nconns = EL_UNDEF;
	}
	if( pccb ) {
	    U_int( *Elscsconn( celp )->scs_lpname ) = pccb->lpinfo.name;
	    Move_scaaddr( pccb->lpinfo.addr, *Elscsconn( celp )->scs_lsaddr )
	} else {
	    U_int( *Elscsconn( celp )->scs_lpname ) = EL_UNDEF;
	    U_short( Elscsconn( celp )->scs_lsaddr[ 0 ]) = EL_UNDEF;
	    U_short( Elscsconn( celp )->scs_lsaddr[ 2 ]) = EL_UNDEF;
	    U_short( Elscsconn( celp )->scs_lsaddr[ 4 ]) = EL_UNDEF;
	}
	if( event == W_TERM_FCONN || event == W_REJECT_FCONN ) {
	    celp += sizeof( struct scs_conn );
	    Elscscommon( elp )->scs_optfmask1 |= SCS_RREASON;
	    *Elscsrreason( celp ) = cb->errlogopt.rreason;
	}
    } else {
	Elscscommon( elp )->scs_optfmask1 |= SCS_LDIRID;
	*Elscsldirid( celp ) = cb->cinfo.Dirid;
    }
    EVALID( elp )
    }
}
