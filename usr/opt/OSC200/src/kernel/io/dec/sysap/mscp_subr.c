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
static char *rcsid = "@(#)$RCSfile: mscp_subr.c,v $ $Revision: 1.1.13.2 $ (DEC) $Date: 1993/07/13 16:28:09 $";
#endif
/*
 * derived from mscp_subr.c	2.7	(ULTRIX)	3/13/90";
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Disk Class Driver
 *
 *   Abstract:	This module contains all routines necessary to
 *		implement the disk MSCP.
 *
 *   Author:	David E. Eiche	Creation Date:	September 30, 1985
 *
 *   History:
 *
 *   31-Oct-1991	Pete Keilty
 *	Changed memeory allocation to OSF/1 memory zones. 
 *	Routines are in scs_subr.c.
 *	Added OSF ifdef UERF around errlog ealloc routine.
 *	Added syslog calls to logerr.
 *	Change the use of NRSPID to nrspid define in mscp_data.c
 *
 *   19-Feb-1991	Tom Tierney
 *	ULTRIX to OSF/1 port:
 *	- Updated to pick up include files from /sys
 *	- ifdef'd out error logging code and replaced with temporary
 *	  code to log as much info as possible.  When ULTRIX-style
 *	  error logging is ported, this code will be re-enabled.
 *
 *   DEC-1990		Matthew S Sacks
 *	Made the driver SMP safe.  Each routine has SMP comments at
 *	its beginnings.  For a general explanation, see the comments
 *	at the end of mscp_defs.h.
 *
 *   04-DEC-90		Pete Keilty
 *      Changed mscp_control() checking of CRE_NEW_PATH event.
 *      Added new routine mscp_adapt_check which checks if this
 *      controller should be attached to this adapter as specified
 *      in the config file. This is an attempt to static load
 *      balance over multiple adapters. Also if the connection is
 *      not the this adapter set a system_poll in timeout for 60sec.
 *      If the other connection does not come in use what is there.
 *
 *   13-Mar-1990	David E. Eiche		DEE0083
 *	Fix mscp_dealloc_msg() to get the address of the next waiting
 *	request block.  The code got mangled during the ISIS merge.
 *
 *   21-Jul-1989	David E. Eiche		DEE0070
 *	Change reference from MSLG_FM_DSK_TRN to MSLG_FM_DISK_TRN.
 *	Change reference from MSLG_FM_BUS_ADR to MSLG_FM_BUS_ADDR.
 *
 *   11-Jul-1989	Tim Burke
 *	Modify mscp_poll_wait to wait at least one 15 second interval before
 *	concluding that all controllers have been identified.
 *
 *   16-May-1989	Pete Keilty
 *	Change mscp_restart_next to check restart queue is empty by
 *	comparing restart.flink with restart.
 *
 *   05-May-1989	Tim Burke	
 *	Merged 3.1 changes into the pu and isis pools.
 *
 *   31-Mar-1989	Pete Keilty
 *	Fixed alloc rspid Remove_entry to use rtp->flink;
 *
 *   15-Mar-1989	Tim Burke
 *	Changed splx( IPL_SCS ) to Splscs();
 *	Changed queue manipulations to use the following macros:
 *	insque ..... Insert_entry
 *	remque ..... Remove_entry
 *	remqck ..... Remove_entry and check to see if any elements on queue.
 *
 *   07-Mar-1989	Todd M. Katz		TMK0002
 *	1. Include header file ../vaxmsi/msisysap.h.
 *	2. Use the ../machine link to refer to machine specific header files.
 *
 *   28-Dec-1988	Tim Burke
 *	Added the new error log format MSLG_FM_IBMSENSE to mscp_logerr() to
 *	handle informational error logs from the TA90.
 *
 *   20-Oct-1988	Pete Keilty
 *	Change mscp_poll_wait to use timeout() to do timing
 *	instead of DELAY.  DELAY raises IPL on CVAX processors
 *	and blocks interrupts for an extended period.
 *
 *   17-Oct-1988	Pete Keilty
 *	Added untimeout to mscp_alloc_reqb().
 *
 *   28-Sep-1988	David E. Eiche		DEE0057
 *	Change panic message formats to make them consistent and
 *	eliminate unnecessary panic messages.
 *
 *   09-Sep-1988	David E. Eiche		DEE0056
 *	Move initialization of request state from mscp_conqrestart
 *	in mscp_conpol.c to mscp_restart_next.  This fixes a bug in
 *	which the connection management request block was having
 *	its state erroneously initialized.
 *
 *   07-Sep-1988	Larry Cohen
 *	Add 15 second delay to mscp_poll_wait routine so that installation
 *	catches more disks.  Need to come up with a better way to wait.
 *
 *   06-Sep-1988	David E. Eiche		DEE0054
 *	Fix code which caused disks to hang when a second loss of
 *	connection occurred while recovering from the first.
 *
 *   27-Jul-1988	Pete Keilty
 *	Changed datagram routine to check for bbr busy and transfer
 *	datagram to bbr code. Also added new logerr routine.
 *
 *   17-Jul-1988	David E. Eiche		DEE0047
 *	Add routine mscp_avail_attn to process available attention
 *	messages.  Call it from mscp_message.
 *
 *   17-Jul-1988	David E. Eiche		DEE0046
 *	Change mscp_control to call mscp_get_connb directly, so
 *	that new path events can be responded to directly.
 *
 *   17-July-1988	David E. Eiche		DEE0045
 *	Move mscp_find_model to mscp_config.c.
 *
 *
 *   08-Jul-1988	Pete Keilty
 *	Added case MSLG_FM_REPLACE for errlogging in datagram routine.
 *
 *   23-Jun-1988	David E. Eiche		DEE0041
 *	Fixed mscp_send_msg to put the connection management request
 *	block in the active queue when there are requests waiting for credits.
 *
 *   16-June-1988	Larry Cohen
 *	Add global variable mscp_polls that is incremented for each
 *	connection attempt and decremented for each connection completion.
 *	The system waits in init_main for mscp_polls to go to zero so that
 *	autoconfigure output can be delayed until the disks/tapes have
 *	been sized.
 *
 *   08-Jun-1988	David E. Eiche		DEE0040
 *	Added a new routine mscp_service_bufferq which is called periodically
 *	from mscp_timer when the buffer wait queue on a connection is not
 *	empty.  The routine attempts to allocate buffers for requests waiting
 *	on the queue.
 *
 *   02-Jun-1988     Ricky S. Palmer
 *      Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   02-Apr-1988	David E. Eiche		DEE0021
 *	Move sleep out of KM_ALLOC into mscp_alloc_reqb	to
 *	eliminate a sleep on interrupt stack double panic.
 *
 *   12-Feb-1988	David E. Eiche		DEE0014
 *	Change references to Insq and Remq macros to call the
 *	underlying insque and remqck functions directly.
 *
 *   02-Feb-1988	David E. Eiche		DEE0011
 *	Remove buf structure queuing code from mscp_alloc_reqb.
 *	Also rearrange mscp_timer code that detects waiting map
 *	requests.
 *
 *   15-Jan-1988	Todd M. Katz		TMK0001
 *	Include new header file ../vaxmsi/msisysap.h.
 */
/**/

/* Libraries and Include Files.
 */
#include	<labels.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<dec/binlog/errlog.h>
#include	<sys/syslog.h>
#include	<sys/errno.h>
#include	<io/dec/scs/sca.h>
#include	<io/dec/sysap/sysap.h>
#include	<io/dec/sysap/mscp_bbrdefs.h>

/**/

/* External variables and routines.
 */
extern	u_long			scs_alloc_msg();
extern	u_long			scs_dealloc_msg();
extern	u_long			scs_map_buf();
extern	u_long			scs_send_msg();
extern	u_long			scs_queue_dgs();
extern	u_long			scs_unmap_buf();
extern	int			wakeup();
extern  CONNB *			mscp_get_connb();
extern	UNITB *			mscp_get_unitb();
extern	RSPID_TBL		mscp_rspid_tbl[];
extern	LISTHD			mscp_rspid_lh;
extern	LISTHD			mscp_rspid_wait_lh;
extern	QE			mscp_reqbq;
extern	int			mscp_reqb_free;
extern	int			mscp_reqb_hold;
extern	u_long			mscp_gbl_flags;
extern	int			hz;
extern	int			nrspid;
extern	int			nreqbs;
extern  caddr_t			sca_zalloc(), sca_zget();
extern  int			sca_zfree(), sca_zones_init(), 
				sca_zones_initialized;
extern  struct zone 		*sca_zone[];

void				mscp_avail_attn();
void				mscp_dealloc_all();
void				mscp_dispatch();
void				mscp_reserve_credit();
void				mscp_restart_next();
void				mscp_service_mapq();
void				mscp_service_bufferq();
void				mscp_service_creditq();
void				mscp_unstall_unit();

/**/

/*
 *
 *   Name:	mscp_dispatch - Finite state machine dispatcher.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect access to the request's state table and state.
 *		
 *
 *   Return	NONE
 *   Values:
 */


void mscp_dispatch( event, rp )
    u_long	    	event;
    REQB	    	*rp;
{
    u_long	    	state;
    STATE	    	*sp;

    /* The following "do forever" allows redispatching when a non-NULL
     * event is returned by an action routine.
     */
    for( ;; ) {

	state = rp->state;
	sp = rp->state_tbl + ( state * ( EV_MAXEVENT + 1 ) + event );
	rp->state = sp->new_state;

	/* If the event code returned by the action routine is NULL,
	 * break out of the "do forever" and exit.  Otherwise, dispatch
	 * the new event.
	 */
	if(( event = ( *sp->action_rtn )( event, rp )) == EV_NULL )
	    break;

    }
    return;
}

/**/

/*
 *
 *   Name:	mscp_invevent - Process invalid event.
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

u_long mscp_invevent( event, rp )
    u_long		event;
    REQB		*rp;
{
    u_long		state = rp->state;

    printf( "mscp_invevent:  invalid event %d in state %d, reqb %x\n",
	    event,
	    state,
	    rp );
    panic( "mscp_invevent: fatal mscp error\n" );
}


/**/

/*
 *
 *   Name:	mscp_noaction - Null action routine.
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

u_long mscp_noaction( event, rp )
    u_long		event;
    REQB		*rp;

{
    return( EV_NULL );
}
/**/

/*
 *
 *   Name:	mscp_timer - provide connection/resource timing services.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Lock the class block to protect its linked list of
 *		know connections.  But do not keep any locks across
 *		subroutine calls.  So, theoretically, the list could
 *		change while we are using it - but it will always be
 *		defined.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_timer( clp )
    CLASSB		*clp;
{
    CONNB		*cp;
    int			s;

    /* Raise the IPL to synchronize with SCS.
     */
    s = Splscs();

    /* If system polling is requested, start up the poller.
     */
    Lock_classb (clp);		/* mscp_system_poll will assume this lock */
    if( clp->flags.need_poll )
	mscp_system_poll( clp );

    /* Scan through the list of known connections.
     */
    for( cp = clp->flink;
	 cp != ( CONNB * )&clp->flink;
	 cp = cp->flink ) {

	/* If command timeouts are active on the connection and the timeout
	 * interval has expired, call connection management with a timeout
	 * event.
	 */
	if(( cp->cmdtmo_intvl != 0 ) && ( --cp->cmdtmo_intvl == 0 )) {
	    Unlock_classb (clp);
	    mscp_dispatch( EV_TIMEOUT, &cp->timeout_reqb );
	    Lock_classb (clp);
	    }
	
	/* If the resource wait queue timing threshhold has been reached and
	 * the either the buffer or mapping wait queue is not empty, service
	 * the waiting requests and reset the timeout interval.
	 */
	Lock_connb (cp);
	if( cp->rsrctmo_intvl == 0 ) {
	    Unlock_connb (cp);
	    Unlock_classb (clp);
	    mscp_service_bufferq( cp );
	    mscp_service_mapq( cp );
	    Lock_classb (clp);
	    Lock_connb (cp);
	    cp->rsrctmo_intvl = RSRC_WAIT_TMO;
	} else {
	    --cp->rsrctmo_intvl;
	}
	Unlock_connb (cp);
    }

    Unlock_classb (clp);

    /* Restore the entry IPL, start another timer interval, and exit.
     */
    ( void )splx( s );
    ( void )timeout( mscp_timer, ( caddr_t )clp, hz );
    return;
}

/**/

/*
 *
 *   Name:	mscp_control - Process asynchronous connection events.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Lock the class block before calling mscp_get_connb.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_control( event, cmsp )
    u_short		event;
    CMSB		*cmsp;
{
    CONNB		*cp;
    REQB		*rp;
    CLASSB		*clp;

    /* Store the CMSB address in the request block and dispatch
     * on asynchronous event type.  Note that cmsp->aux contains the
     * class block address in the new system case, but contains the
     * connection block address in all other cases.
     */
    if( event == CRE_NEW_PATH ) {
	clp = ( CLASSB * )cmsp->aux;
	if( mscp_adapt_check( cmsp )) {
		Lock_classb (clp);
		cp = mscp_get_connb( clp,
			     &cmsp->sysid,
			     &cmsp->rport_addr,
			     cmsp->lport_name );
		if( cp == NULL ) {
	    		clp->flags.need_poll = 1;
		}
		Unlock_classb (clp);
        } else {
            (void) timeout( mscp_system_poll, (caddr_t)clp, hz * 60);
        }
    } else {
	cp = ( CONNB * )cmsp->aux;
	rp = &cp->timeout_reqb;
	rp->aux = ( u_char * )cmsp;
	switch( event ) {
	    case	CRE_CONN_DONE:
		mscp_dispatch( EV_CONCOMPLETE, rp );
		break;
	    case	CRE_PATH_FAILURE:
		Lock_connb (cp);
		cp->flags.path_fail = 1;
		Unlock_connb (cp);
		/* Fall through */
	    case	CRE_DISCONN_REC:
		mscp_dispatch( EV_PATHFAILURE, rp );
		break;
	    case	CRE_DISCONN_DONE:
		mscp_dispatch( EV_DISCOMPLETE, rp );
		break;
	    case	CRE_CREDIT_AVAIL:
		Lock_connb (cp);
		if (! cp->flags.top_serv) {
			Unlock_connb (cp);
			mscp_service_creditq( rp );
			}
			else Unlock_connb (cp);
		break;

	    /* None of the following events should occur.
	     */
	    case	CRE_ACCEPT_DONE:
	    case	CRE_BLOCK_DONE:
	    case	CRE_CONN_REC:
	    case	CRE_REJECT_DONE:
	    default:
	    panic( "mscp_control: unexpected connection management event" );
	}
    }
    return;
}

/**/

/*
 *
 *   Name:	mscp_message - Message input routine
 *
 *   Abstract:	
 *		This routine handles end messages and attention
 *		messages arriving on a connection.  End messages are
 *		distinguished from attention messages by the presence
 *		of the end flag in the mscp opcode.
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	Protect connection block.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_message( csp )
    CSB			*csp;
{
    CONNB		*cp = ( CONNB * )csp->Aux;
    MSCP		*mp = ( MSCP * )csp->buf;
    REQB		*rp;
    u_short		index, seq_no;

    /* If the message is an end message, interpret the command reference
     * number as a RSPID and break it into its component parts.
     */
    if( mp->mscp_opcode & MSCP_OP_END ) {
	index = (( RSPID * )&mp->mscp_cmd_ref )->index;
	seq_no = (( RSPID * )&mp->mscp_cmd_ref )->seq_no;

	/* If the RSPID index is out of range or the sequence number is not
	 * current, deallocate the message without further processing.
	 */
	if(( index >= nrspid ) ||
	   ( seq_no != mscp_rspid_tbl[ index ].rspid.seq_no )) {
	    printf( "mscp_message: dropping msg:  mp %x, op %x, RSPID %x\n",
		mp, mp->mscp_opcode, mp->mscp_cmd_ref );
	    if( scs_dealloc_msg( csp ) != RET_SUCCESS )
		panic( "mscp_message: scs_dealloc_msg failed\n" );
	   return;
	} else {

	    /* Get the request block pointer that corresponds to the RSPID,
	     * remove the request block from the connection active queue,
	     * store the MSCP message buffer address and size in the REQB,
	     * and call back the thread that was waiting for message completion.
	     */
	    rp = mscp_rspid_tbl[ index ].reqb;

	    if( mp->mscp_cmd_ref != *( u_int * )&rp->rspid ) {
		printf( "mscp_message: end msg rspid %x != rp rspid %x\n",
		    mp->mscp_cmd_ref, *( u_int * )&rp->rspid );
		panic( "mscp_message: invalid rspid\n");
	    }

	    /* If the RSPID represents the oldest command outstanding in
	     * the controller, clear the oldest command field in the
	     * connection block to let connection management know that work
	     * is progressing.  Recall that the connection block is
	     * locked a this moment.
	     */
	    Lock_connb (cp);
	    if( mp->mscp_cmd_ref == *( u_int * )&cp->old_rspid )
		*( u_int * )&cp->old_rspid = NULL;
	    Unlock_connb (cp);

    	    Lock_connb_actq (cp);
	    Remove_entry( rp->flink );
	    Unlock_connb_actq (cp);

	    rp->flink = NULL;
	    rp->blink = NULL;
	    rp->msgptr = mp;
	    rp->msgsize = csp->size;
	    mscp_dispatch( EV_ENDMSG, rp );
	}

    /* The message is not an end message.  Treat it as an attention message
     * and dispatch on MSCP opcode.
     */
    } else {
	switch( mp->mscp_opcode ) {
	    default:
		printf( "mscp_message:  unknown attention message 0x%2x\n",
			mp->mscp_opcode );
	    case MSCP_OP_AVATN:
		mscp_avail_attn( csp );
	    case MSCP_OP_DUPUN:
	    case MSCP_OP_ACPTH:
		if( scs_dealloc_msg( csp ) != RET_SUCCESS )
		    panic( "mscp_message: scs_dealloc_msg failed\n" );
	}
    }

    return;
}
/**/

/*
 *
 *   Name:	mscp_avail_attn- Process available attention messages
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_avail_attn( csp )
    CSB			*csp;
{
    CONNB		*cp = ( CONNB * )csp->Aux;
    MSCP		*mp = ( MSCP * )csp->buf;
    UNITB		*up;

    up = mscp_get_unitb( cp, mp, csp->size );
}

/**/

/*
 *
 *   Name:	mscp_datagram - Datagram input routine
 *
 *   Abstract:	
 *		This routine handles error logging datagrams.
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	Be wary that if we are processing a last fail packet,
 *		we are bringing up a connection, and scs may hold the
 *		LK_SCADB lock.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_datagram( csp )
    CSB			*csp;
{
    CONNB		*cp = (CONNB *)csp->Aux;
    MSLG 		*mp = (MSLG *)csp->buf;
    REQB		*rp;    /* bbr request cmd */
    MSLG		*bbrmp; /* bbr mslg */
    void 		mscp_logerr();

 	/* avoid boolean short circuit problem */
	if (cp->bbrb != 0) {
		rp = &(cp->bbrb->bbr_reqb);		/* bbr request cmd */
		bbrmp = &(cp->bbrb->bbr_mslg); 		/* bbr mslg */
	}

	/* if error durning bbr cmd - log via bbr code */
	if ((cp->bbrb != 0) && (cp->bbrb->flags.bit.busy == 1) &&
	    	(mp->mslg_cmd_ref == *(u_int *)&rp->rspid)) {

	    	    if (cp->bbrb->flags.bit.logerr == 0) {
		         *(bbrmp) = *(mp);
	    	         }
	        }
	        else {
	            mscp_logerr( cp, mp , csp->size);
		}

        csp->Nbufs = 0;

        scs_queue_dgs( csp );

        return;
}

/**/

/*
 *   Name:	mscp_logerr - mscp log error routine
 *
 *   Abstract:	
 *		This routine logs datagram error packets to the system
 *		error log.
 *
 *   Inputs:	cp - connection pointer
 *		mp - mslg packet pointer
 *		size - mslg packet size
 *
 *   Outputs:
 *
 *   SMP	all set
 *
 *   Return	NONE
 *   Values:
 */
 
void
mscp_logerr( cp, mp, size)
    CONNB		*cp;
    MSLG		*mp;
    long		size;
{

    switch (mp->mslg_format) {
    case MSLG_FM_CNT_ER:
      if (mp->mslg_cnt_id[7] == 3) 
        log(LOG_ERR,"MSCP tape controller error, event code 0x%x\n",
		mp->mslg_event);
      else
        log(LOG_ERR,"MSCP disk controller error, event code 0x%x\n",
		mp->mslg_event);
  
      break;

    case MSLG_FM_BUS_ADDR:
        log(LOG_ERR,"MSCP bus address error, event code 0x%x\n",
		mp->mslg_event);
      break;

    case MSLG_FM_DISK_TRN:
      log(LOG_ERR,"MSCP disk transfer error, unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;
    case MSLG_FM_SDI:
      log(LOG_ERR,"MSCP disk sdi error, unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;

    case MSLG_FM_SML_DSK:
      log(LOG_ERR,"MSCP small disk error, unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;

    case MSLG_FM_REPLACE:
      log(LOG_ERR,"MSCP disk bad block replace, unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;

    case MSLG_FM_TAPE_TRN:
      log(LOG_ERR,"MSCP tape transfer error, unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;

    case MSLG_FM_STI_ERR:
      log(LOG_ERR,"MSCP tape sti error, unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;

    case MSLG_FM_STI_DEL:
      log(LOG_ERR,"MSCP tape sti del error, unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;

    case MSLG_FM_STI_FEL:
      log(LOG_ERR,"MSCP tape sti fel error,  unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;

    case MSLG_FM_IBMSENSE:
      log(LOG_ERR,"MSCP tape sence data, unit #%d event code 0x%x\n",
		mp->mslg_unit,mp->mslg_event);
      break;

    default:
      log(LOG_ERR,"MSCP unknown format error x%x, event code 0x%x\n",
		mp->mslg_format,mp->mslg_event);
      break;
 
    }

	{
	struct	el_rec	*elp;
	int	class, type, devtype, unitnum, subidnum;

	if ((elp = ealloc((sizeof(struct el_bdev)), EL_PRIHIGH)) == EL_FULL)
		return;

	switch (mp->mslg_format) {

	case MSLG_FM_CNT_ER:
	case MSLG_FM_BUS_ADDR:

		class = ELCT_DCNTL;	    /* Device controller class	*/
		if (mp->mslg_cnt_id[7] == 3)
		    type = ELTMSCP_CNTRL;   /* TMSCP controller type 	*/
		else
		    type = ELMSCP_CNTRL;    /* MSCP controller type 	*/

		unitnum = cp->cnt_number;    /* Controller number	*/
		devtype = mp->mslg_cnt_id[6] & 0xFF;

		/* Need to find  adpt or bus # from ? */
		subidnum = EL_UNDEF; 	   /* adpt or bus controller #	*/

		break;

	case MSLG_FM_DISK_TRN:
	case MSLG_FM_SDI:
	case MSLG_FM_SML_DSK:
	case MSLG_FM_REPLACE:

		class = ELCT_DISK;		/* Disk class		*/
		type = ELDEV_MSCP;		/* MSCP disk type	*/
		devtype = (mp->mslg_unit_id[1] >> 16) & 0xFF;
		unitnum = mp->mslg_unit;	/* Unit number		*/
		subidnum = cp->cnt_number;      /* Controller number	*/
		break;

	case MSLG_FM_TAPE_TRN:
	case MSLG_FM_STI_ERR:
	case MSLG_FM_STI_DEL:
	case MSLG_FM_STI_FEL:
	case MSLG_FM_IBMSENSE:

		class = ELCT_TAPE;		/* Disk class		*/
		type = ELDEV_MSCP;		/* MSCP disk type	*/
		devtype = (mp->mslg_unit_id[1] >> 16) & 0xFF;
		unitnum = mp->mslg_unit;	/* Unit number		*/
		subidnum = cp->cnt_number;      /* Controller number	*/
		break;

	default:

		class = EL_UNDEF;		/* Unknown		*/
		type = EL_UNDEF;		/* Unknown		*/
		devtype = EL_UNDEF;		/* Unknown		*/
		unitnum = EL_UNDEF;		/* Unknown		*/
		subidnum = EL_UNDEF;		/* Unknown		*/
		break;
	}

	LSUBID(elp, class, type, devtype, subidnum, unitnum, 
		  (u_int)mp->mslg_format)

	/* What should go in here?   */
	elp->el_body.elbdev.eldevhdr.devhdr_dev = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_flags = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_bcount = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_blkno = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_retrycnt = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_herrcnt = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_serrcnt = EL_UNDEF;
	elp->el_body.elbdev.eldevhdr.devhdr_csr = EL_UNDEF;

	elp->el_body.elbdev.eldevdata.elmslg.mslg_len = size;
	elp->el_body.elbdev.eldevdata.elmslg.mscp_mslg = *(mp);
	EVALID(elp);
	}

}

/**/

/*
 *
 *   Name:	mscp_alloc_rspid - Allocate a Response ID
 *
 *   Abstract:	Allocate a Response ID (RSPID) for use as a command
 *		reference number.  If there are no RSPIDs available,
 *		insert the REQB onto the tail of the RSPID wait queue.
 *
 *   Inputs:	rp			Request block pointer
 *		connb		Connection block pointer
 *		classb		Class block pointer
 *
 *   Outputs:   rp
 *		rspid		The allocated RSPID
 *
 *   SMP	protect access to rspid stuff
 *
 *   Return	NONE
 *   Values:
 */

/* ** TO DO ** Put the RSPID table or a pointer to it in the CLASSB
 * so that it doesn't have to be referenced as a global variable.
 */

u_long
mscp_alloc_rspid( event, rp )
    u_long		event;
    REQB		*rp;
{
    RSPID_TBL		*rtp;
    u_long		new_event = EV_RSPID;
 
    /* ** TEMP **  Panic if this reqb already has a RSPID.
     */
    if( *( u_int * )&rp->rspid ) {
	printf(" rp %x has non-zero RSPID %x\n", rp, rp->rspid );
	panic( "mscp_alloc_rspid: double RSPID allocation\n" );
    }

    Lock_rspid_db ();
    /* If the RSPID wait queue is empty and there is an available RSPID,
     * store the REQB address in the RSPID table and copy the RSPID from
     * the RSPID table into the REQB.
     */
    if(( mscp_rspid_wait_lh.flink == ( QE * )&mscp_rspid_wait_lh.flink ) &&
       ((rtp = (RSPID_TBL *)mscp_rspid_lh.flink) != 
	( RSPID_TBL *)&mscp_rspid_lh )) {
	    Remove_entry( rtp->flink );
	    rtp->reqb = rp;
	    rp->rspid = rtp->rspid;
    	    Unlock_rspid_db(); 

    /* If the wait queue is not empty or there are no available RSPIDs,
     * thread the REQB into the RSPID wait queue and bump the wait count
     * to stall requests on the unit.
     */
    } else {
	Insert_entry( rp->flink, mscp_rspid_wait_lh );
    	Unlock_rspid_db(); 
	Incr_rwait( rp );
	new_event = EV_NULL;
    }

    return( new_event );
}

/**/

/*
 *
 *   Name:	mscp_recycle_rspid - Recycle a Response ID
 *
 *   Abstract:	Recycle a response ID by updating its sequence number
 *		field.  Recycling has no effect on threads in the RSPID
 *		wait queue.
 *
 *   Inputs:	rp			Request block pointer
 *		    rspid		RSPID
 *		rtp			RSPID table pointer
 *		    rspid		RSPID
 *
 *   Outputs:	rp			Request block pointer
 *		    rspid		Updated RSPID.
 *
 *   SMP	Protect the rspid database.  Caller may have the relevent
 *		unit block locked.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_recycle_rspid( rp )
    REQB		*rp;
{
    RSPID_TBL		*rtp;

    Lock_rspid_db ();
    /* Locate the RSPID table entry that corresponds to the input RSPID.
     */
    rtp = &mscp_rspid_tbl[ rp->rspid.index ];
 
    /* If the sequence numbers don't agree, something has been corrupted.
     * Panic.
     */
    if( rtp->rspid.seq_no != rp->rspid.seq_no) {
	Unlock_rspid_db ();
	panic( "mscp_recycle_rspid - sequence number error.\n" );
    }
    /* Update the sequence number and copy the RSPID into the REQB.
     */
    else {
	if( ++rtp->rspid.seq_no == 0 )
	    ++rtp->rspid.seq_no;
	rp->rspid = rtp->rspid;
	Unlock_rspid_db ();
	return;
    }
}

/**/

/*
 *
 *   Name:	mscp_dealloc_rspid	- deallocate Response ID
 *
 *   Abstract:	Return a RSPID entry to the free queue and activate
 *		the first thread on the RSPID wait queue, if any.
 *
 *   Inputs:	rp			Request block pointer
 *		    rspid		Response ID
 *		rtp			RSPID table pointer
 *		    rspid		Response ID
 *		    flink		Forward link
 *
 *   Outputs:
 *
 *   SMP	protect rspid stuff
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_dealloc_rspid( rp )
    REQB		*rp;
{
    RSPID_TBL		*rtp;
    REQB		*wrp;

    Lock_rspid_db ();

    /* Locate the RSPID table entry that corresponds to the input RSPID.
     */
    rtp = &mscp_rspid_tbl[ rp->rspid.index ];
 
    /* If the sequence numbers don't agree, something has been corrupted.
     * Panic.
     */
    if( rtp->rspid.seq_no != rp->rspid.seq_no) {
	printf("mscp_dealloc_rspid: error rtp->rspid %x, rp->rspid %x\n",
	    *( u_int * )&rtp->rspid, *( u_int * )&rp->rspid);
        Unlock_rspid_db ();
	panic( "mscp_dealloc_rspid:  sequence number mismatch\n" );

    /* Update the sequence number in the RSPID table and zero the
     * RSPID field in the request block as a safety precaution.
     */
    } else {
	if( ++rtp->rspid.seq_no == 0 )
	    ++rtp->rspid.seq_no;
	rp->rspid.index = 0;
	rp->rspid.seq_no = 0;

	/* If there is a request block waiting for a RSPID, do the
	 * required bookkeeping and dispatch the waiting thread.
	 */
        if(( wrp = (REQB *)mscp_rspid_wait_lh.flink ) != (REQB *)&mscp_rspid_wait_lh ) {
            Remove_entry( wrp->flink );
	    rtp->reqb = wrp;
	    wrp->rspid = rtp->rspid;
	    Unlock_rspid_db ();
	    mscp_dispatch( EV_RSPID, wrp );
	    Decr_rwait( wrp );		    /* might call mscp_unstall_unit */

	/* No waiters.  Clear the request block pointer in the RSPID
	 * table and put the deallocated entry in the free list.
	 */
	} else {
	    rtp->reqb = NULL;
	    Insert_entry( rtp->flink, mscp_rspid_lh );
	    Unlock_rspid_db ();
	}

	return;
    }
}

/**/

/*
 *
 *   Name:	mscp_alloc_msg - Allocate a sequenced message buffer
 *
 *   Abstract:	Allocate a sequenced message buffer via SCS.  If
 *		the allocation fails because of a shortage of
 *		buffers, insert the REQB on the buffer wait queue.
 *		If the allocation fails for any other reason, panic.
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP	Protect manipulaton of the buffer queue in the connection
 *		block, but do not keep locks across subroutine calls
 *
 *		NOTE:
 *			(a) We do not lock around the check to see if
 *			buff_wq is empty, in order to avoid the expense
 *			of taking the lock.  Although the test could
 *			be false, the only effect of the bad test is
 *			that the request goes onto the queue and is
 *			serviced later.
 *
 *			(b) This routine assumes that interrupts are not
 *			distributed; if they were, then the access to
 *			the buffwq needs to be synchronized with 
 *			mscp_service_buffq().
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_alloc_msg( event, rp )
    u_long		event;
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    u_long		new_event = EV_MSGBUF;
    int			alloc_succ = 0;

    /* ** TEMP **  Panic if this reqb already has a msg buffer.
     */
    if( rp->msgptr ) {
	printf(" rp %x has non-zero msgptr %x\n", rp, rp->msgptr );
	panic( "mscp_alloc_msg: double msg buffer allocation\n" );
    }

    /* Store the connection ID in the CSB.  If the message wait queue
     * is empty, call SCS to allocate a message buffer.  If the allocation
     * is successful, fill in the message buffer pointer in the request
     * block, and return a message buffer available event to the caller.
     */
    Move_connid( cp->connid, csp->connid );
    if ( cp->buffer_wq.flink == ( REQB * )&cp->buffer_wq.flink ) {
	if ( scs_alloc_msg( csp ) == RET_SUCCESS ) {
	    rp->msgptr = ( MSCP * )csp->buf;
	    alloc_succ = 1;
	}
    }

    /* If the wait queue is not empty or if allocation fails for any
     * reason, insert the REQB at the tail of the message buffer wait
     * queue for the connection, increment the wait reasons counter to
     * stall new activity, and return a null event to the caller.
     */
    if (! alloc_succ) {
    	Lock_connb_buffwq (cp);
	Insert_entry( rp->flink, cp->buffer_wq );
	Unlock_connb_buffwq (cp);
	Incr_rwait( rp );
	new_event = EV_NULL;
    }

    return( new_event );
}
/**/

/*
 *
 *   Name:	mscp_dealloc_msg - Deallocate a sequenced message buffer
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect the buffer queue manipulation in the connection block,
 *		but do not keep locks across subroutine calls.  
 *
 *		NOTE:
 *			(a) We do not lock around the check to see if
 *			buff_wq is empty, in order to avoid the expense
 *			of taking the lock.  Although the test could
 *			be false, the only effect of the bad test is
 *			that the request goes onto the queue and is
 *			serviced later.
 *
 *			(b) This routine assumes that interrupts are not
 *			distributed; if they were, then the access to
 *			the buffwq needs to be synchronized with 
 *			mscp_service_buffq().
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_dealloc_msg( rp )
    REQB		*rp;
{
    REQB		*wrp;
    CONNB		*cp = rp->connb;
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    u_long		scs_status;

    Move_connid( cp->connid, csp->connid );

    /* Return the buffer to SCS.
     */

    csp->buf = ( u_char * )rp->msgptr;
    rp->msgptr = NULL;
    if(( scs_status = scs_dealloc_msg( csp )) != RET_SUCCESS ) {
	printf( "mscp_dealloc_msg: scs_status %x\n", scs_status );
	panic( "mscp_dealloc_msg: bad connection state or ID\n" );
    }

    /* If there is a request packet waiting for a buffer, try to get
     * the buffer that was just given back.  If the allocation request
     * succeeds, restart the waiting thread with a message buffer 
     * available event.
     */
    if(( wrp = cp->buffer_wq.flink ) != ( REQB * )&cp->buffer_wq.flink ) {
	if(( scs_status = scs_alloc_msg( csp )) == RET_SUCCESS ) {
	    Lock_connb_buffwq (cp);
	    Remove_entry( wrp->flink );
	    Unlock_connb_buffwq (cp);
	    wrp->msgptr = ( MSCP * )csp->buf;
	    mscp_dispatch( EV_MSGBUF, wrp );
	    Decr_rwait( wrp );		/* might call mscp_unstall_unit */
    	    }
    }

    return;
}
/**/

/*
 *
 *   Name:	mscp_service_bufferq - get buffers for waiting requests
 *
 *   Abstract:	This routine is called from mscp_timer to attempt to
 *		service requests that are waiting for message buffers.
 *		It is possible for a shared message buffer resource to
 *		become available on one connection without notification
 *		of waiting requests on other connections; this routine is
 *		periodically invoked to deal with that eventuality.
 *
 *   Inputs:    cp		    Connection block pointer.
 *		    buffer_wq.flink Map wait queue of request blocks.
 *
 *   Outputs:	
 *		rp		    Request block pointer.
 *		    msgptr
 *
 *   SMP:	protect manipulations of the connection block's buffer
 *		wait queue.
 *		These lock manipulations, the ones in mscp_service_mapq,
 *		and the ones in mscp_timer, are	coordinated with each other.
 *
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_service_bufferq( cp )
    CONNB		*cp;
{
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    REQB		*wrp;
    u_long		status;
    int			is_locked = 1;


    /* Issue an allocate message request to SCS for each waiting REQB
     * in turn until the queue is empty or until a request fails.  For
     * each successful buffer allocation, do the wait queue bookkeeping
     * and dispatch the REQB thread with the message buffer pointer
     * in hand.
     */
    Move_connid( cp->connid, csp->connid );

    Lock_connb_buffwq (cp);
    for(wrp = cp->buffer_wq.flink;
	 wrp != (REQB * )&cp->buffer_wq.flink;
	 wrp = cp->buffer_wq.flink ) {
	Unlock_connb_buffwq (cp);

	if( scs_alloc_msg( csp ) == RET_SUCCESS ) {
	    Lock_connb_buffwq (cp);
	    Remove_entry( wrp->flink );
	    Unlock_connb_buffwq (cp);
	    wrp->msgptr = ( MSCP * )csp->buf;
	    mscp_dispatch( EV_MSGBUF, wrp );
	    Decr_rwait( wrp );		/* might call mscp_unstall_unit */
	    Lock_connb_buffwq (cp);

	} else {
	    is_locked = 0;
	    break;
	}
    }

    if (is_locked) Unlock_connb_buffwq (cp);

    return;
}

/**/

/*
 *
 *   Name:	mscp_map_buffer - map a data buffer
 *
 *   Abstract:	Allocate mapping resources for an MSCP data transfer
 *		operation.
 *
 *   Inputs:    rp			Request block pointer.
 *
 *   Outputs:	rp			Request block pointer.
 *		    lbhandle		Local buffer handle.
 *
 *   SMP	Protect manipulaton of the map_wq queue in the connection
 *		block, but do not keep locks across subroutine calls
 *
 *		NOTE:
 *			(a) We do not lock around the check to see if
 *			map_wq is empty, in order to avoid the expense
 *			of taking the lock.  Although the test could
 *			be false, the only effect of the bad test is
 *			that the request goes onto the queue and is
 *			serviced later.
 *
 *			(b) This routine assumes that interrupts are not
 *			distributed; if they were, then the access to
 *			the buffwq needs to be synchronized with 
 *			mscp_service_mapwq().
 *
 *   Return	NONE
 *   Values:
 */

u_long
mscp_map_buffer( event, rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    u_long		new_event = EV_MAPPING;
    int			map_succ = 0;

    /* ** TEMP **  Panic if this reqb already has a buffer handle.
     */
    if( !Test_bhandle( rp->lbhandle )) {
	printf(" rp %x has non-zero buffer handle\n", rp );
	panic( "mscp_map_buffer: double buffer handle allocation\n" );
    }


    /* *** TEMP ***
     * Zero the buffer handle while we figure out who should really do it.
     */
    Zero_bhandle( csp->lbhandle );

    /* Store the connection ID and the buf structure pointer in the CSB.
     * If the map wait queue is empty, call SCS to map the buffer.  If
     * the map request succeeds, store the local buffer handle in the
     * request block and return a MAPPING event to the caller.
     */
    csp->Sbh = rp->bufptr;
    Move_connid( cp->connid, csp->connid );

    if ( cp->map_wq.flink == ( REQB * )&cp->map_wq.flink ) {
	if ( scs_map_buf( csp ) == RET_SUCCESS ) {
	    rp->lbhandle = csp->lbhandle;
	    map_succ = 1;
	}
    }

    /* If the map queue is not empty or the request fails for any reason,
     * queue the request block, stall incoming requests and return a NULL
     * event to the caller.
     */
    if (! map_succ) {
	Lock_connb_mapwq (cp);
	Insert_entry( rp->flink, cp->map_wq );
	Unlock_connb_mapwq (cp);
	Incr_rwait( rp );
	new_event = EV_NULL;
    }

    return( new_event );
}
/**/

/*
 *
 *   Name:	mscp_unmap_buffer - unmap a data buffer
 *
 *   Abstract:	Deallocate mapping resources after completion of
 *		an MSCP data transfer operation.
 *
 *   Inputs:    rp			Request block pointer.
 *		    lbhandle		Local buffer handle.
 *
 *   Outputs:	rp			Request block pointer.
 *
 *   SMP:	Protect access to the connection block's mapping
 *		wait queue.  Do not keep locks across subroutine
 *		calls.
 *
 *		NOTE:
 *			(a) We do not lock around the check to see if
 *			map_wq is empty, in order to avoid the expense
 *			of taking the lock.  Although the test could
 *			be false, the only effect of the bad test is
 *			that the request goes onto the queue and is
 *			serviced later.
 *
 *			(b) This routine assumes that interrupts are not
 *			distributed; if they were, then the access to
 *			the buffwq needs to be synchronized with 
 *			mscp_service_mapwq().
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_unmap_buffer( rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    REQB		*wrp;
    u_long		status;
    int			is_locked = 1;

    Move_connid( cp->connid, csp->connid );

    csp->lbhandle = rp->lbhandle;
    csp->Sbh = rp->bufptr;

    if(( status = scs_unmap_buf( csp )) == RET_SUCCESS ) {

      /* Zero the buffer handle to prevent its inadvertant reuse.
       */
      Zero_bhandle( rp->lbhandle );

	/* If there are REQBs waiting for mapping resources, issue a map 
	 * request for each one in turn until the wait queue is empty or
	 * until a map request fails.
	 */
      if (cp->map_wq.flink != (REQB *)&cp->map_wq.flink) {

	  Lock_connb_mapwq (cp);
	  for( wrp = cp->map_wq.flink;
	     wrp != (REQB * )&cp->map_wq.flink;
	     wrp = cp->map_wq.flink ) {

	     Unlock_connb_mapwq (cp);

	     csp->Sbh = wrp->bufptr;

	    /* If the map request succeeds, do the wait queue bookkeeping
	     * and dispatch the REQB thread with the local buffer handle
	     * in hand.
	     */
	    if( scs_map_buf( csp ) == RET_SUCCESS ) {
		Lock_connb_mapwq (cp);
	        Remove_entry( wrp->flink );
		Unlock_connb_mapwq (cp);
		wrp->lbhandle = csp->lbhandle;
		mscp_dispatch( EV_MAPPING, wrp );
		Decr_rwait( wrp );	   /* might call mscp_unstall_unit */
	        Lock_connb_mapwq (cp);

	    /* If the map request failed, break out of the for loop.
	     */
	    } else {
		is_locked = 0;
		break;
		}
	}
	if (is_locked) Unlock_connb_mapwq (cp);
      }

        return;

    } else
	panic( "mscp_unmap_buffer: bad connection state or ID\n" );
}
/**/

/*
 *
 *   Name:	mscp_service_mapq - attempt to map waiting requests
 *
 *   Abstract:	This routine is called from mscp_timer to attempt to
 *		service requests that are waiting for mapping resources.
 *		It is possible for a shared mapping resource to become
 *		available on one connection without notification of
 *		waiting requests on other connections; this routine is
 *		periodically invoked to deal with that eventuality.
 *
 *   Inputs:    cp		    Connection block pointer.
 *		    map_wq.flink    Map wait queue of request blocks.
 *		        lbhandle    Local buffer handle.
 *
 *   Outputs:	
 *
 *   SMP:	lock the connection block's map wait queue in order to
 *		protects its map wait queue.  Do not keep locks across
 *		subroutine calls.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_service_mapq( cp )
    CONNB		*cp;
{
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    REQB		*wrp;
    u_long		status;
    int			is_locked = 1;


    Move_connid( cp->connid, csp->connid );

    Lock_connb_mapwq (cp);
    /* Issue a map request for each waiting REQB in turn until the queue
     * is empty or until a map request fails.
     */
    for( wrp = cp->map_wq.flink;
	 wrp != (REQB * )&cp->map_wq.flink;
	 wrp = cp->map_wq.flink ) {

	 Unlock_connb_mapwq (cp);

	 csp->Sbh = wrp->bufptr;

	/* If the map request succeeds, do the wait queue bookkeeping
	 * and dispatch the REQB thread with the local buffer handle
	 * in hand.
	 */
	if( scs_map_buf( csp ) == RET_SUCCESS ) {
	    Lock_connb_mapwq (cp);
	    Remove_entry( wrp->flink );
	    Unlock_connb_mapwq (cp);
	    wrp->lbhandle = csp->lbhandle;
	    mscp_dispatch( EV_MAPPING, wrp );
	    Decr_rwait( wrp );		/* might call mscp_unstall_unit */
	    Lock_connb_mapwq (cp);
	/* If the map request failed, break out of the for loop.
	 */
	} else
	    {
	    is_locked = 0;
	    break;
	    }
    }
    if (is_locked) Unlock_connb_mapwq (cp);
    return;
}

/**/

/*
 *
 *   Name:	mscp_send_msg - Send an MSCP sequenced message
 *
 *   Abstract:	Fill in the appropriate CSB fields, and send a 
 *		MSCP sequenced message across a connection.
 *
 *   Inputs:	rp			Request block pointer
 *
 *   Outputs:
 *
 *   SMP:	lock the connection block to protect the manipulations
 *		of its credit wait queue; but do not keep locks across
 *		subroutine calls; this means the queue may change but
 *		it is always defined.
 *
 *		Also, lock the unit block structure when manipulating
 *		the resource wait counter; the Decr_wait macro has the
 *		lock call in it.
 *
 *   Return	
 *   Values:
 */
u_long
mscp_send_msg( rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    u_long		status = RET_FAILURE;
    u_long		new_event = EV_NULL;

    Move_connid( cp->connid, csp->connid);

    /* Fill in the Communications services block portion of the
     * request block with the connection ID, a pointer to the
     * MSCP message buffer, the maximum MSCP command message size,
     * and the message buffer disposition code.
     */
    csp->buf = ( u_char * )rp->msgptr;
    csp->size = sizeof( MSCP_CMDMSG );
    csp->Disposal = RECEIVE_BUF;

    /* If the credit wait queue is empty or the request is a connection
     * management immediate message, call SCS to send the message.
     */

    Lock_connb_credwq (cp);
    if(( cp->credit_wq.flink == ( REQB * )&cp->credit_wq.flink ||
	 rp->flags.nocreditw )) {
	Unlock_connb_credwq (cp);
	status = scs_send_msg(csp);
    }
    else
	Unlock_connb_credwq (cp);

    /*
     * If scs_send_msg returned success, queue the request to the active
     * queue, and zero out the message buffer pointer to prevent its
     * inadvertent reuse.
     */

    if( status == RET_SUCCESS ) {
		Lock_connb_actq (cp);
		Insert_entry( rp->flink, cp->active );
		Unlock_connb_actq (cp);
		rp->msgptr = NULL;
    }
    else if( rp->flags.nocreditw ) {

    /* If the request represents a connection management message, and
     * we are in this block of code, then we can conclude that the
     * scs_send_msg() did not succeed.  Call SCS to add back the credit
     * reserved for connection management, and call SCS to try again to
     * send the message.  (The reserved credit must be added back here
     * rather than above in order to avoid using the last credit for a
     * non-immediate command, which would violate the MSCP spec.)  If
     * either call fails, return a no credits event.  Otherwise, add the
     * request to the active queue, and zero out the message buffer pointer
     * to prevent its inadvertent reuse.
     */
		if( scs_add_credit( csp ) == RET_SUCCESS  && 
	    		( status = scs_send_msg( csp )) == RET_SUCCESS ) {
			Lock_connb_actq (cp);
	    		Insert_entry( rp->flink, cp->active );
			Unlock_connb_actq (cp);
			Lock_connb (cp);
	    		cp->flags.need_cr = 1;
		        Unlock_connb (cp);
	    		rp->msgptr = NULL;
		} else {
	    		new_event = EV_NOCREDITS;
		}
	} /* end if (rp->flags.nocreditw) */

    /* The credit wait queue isn't empty or scs_send_msg returned an
     * error on a non privileged request. Add the request to the credit
     * wait queue and increment the resource wait count for the unit.
     * Then try to service credit the queue.
     */
        else {
		Lock_connb_credwq (cp);
		Insert_entry( rp->flink, cp->credit_wq );
		Unlock_connb_credwq (cp);

		Incr_rwait(rp);

	/* Since we know there are requests on the credit wait queue,
	 * we call service_creditq.  Note that this call, unlike the
	 * the interrupt driven one, can happen on a secondary cpu.
	 */
		Lock_connb (cp);
		cp->flags.top_serv = 1;
		Unlock_connb (cp);
		mscp_service_creditq( rp );
		Lock_connb (cp);
		cp->flags.top_serv = 0;
		Unlock_connb (cp);
    	}

    /* Return status to the caller.
     */
    return( new_event );

}
/**/

/*
 *
 *   Name:	mscp_service_creditq - service requests in credit wait
 *
 *   Abstract:	This routine is called from mscp_control to service
 *		requests in the credit wait queue.
 *
 *   Inputs:    cp		    Connection block pointer.
 *		    credit_wq.flink Map wait queue of request blocks.
 *
 *   Outputs:	
 *
 *   SMP:	lock the connection block in order to protect manipulations
 *		of the credit wait queue.
 *		The Decr_rwait macro does its own locking of the unit block.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_service_creditq( rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    u_long		status;
    int			is_locked = 1;

    Lock_connb (cp);

    if (! cp->flags.serv_cred)
    {
    cp->flags.serv_cred = 1;
    
    /* If the connection management reserved credit has been expended,
     * reserve it again.
     */
    if( cp->flags.need_cr ) 
	mscp_reserve_credit( rp );

    Unlock_connb (cp);

    /* Issue a send message for each waiting REQB in turn until the queue
     * is empty or until a send message fails.
     */
    Lock_connb_credwq (cp);
    for( rp = cp->credit_wq.flink;
	 rp != (REQB * )&cp->credit_wq.flink;
	 rp = cp->credit_wq.flink ) {
	 
	Unlock_connb_credwq (cp);

	/* Fill in the CSB request block with the connection ID,
	 * a pointer to the MSCP message buffer, the maximum MSCP
	 * command message size, and the message buffer disposition
	 * code.
	 */
	Move_connid( cp->connid, csp->connid);

	csp->buf = ( u_char * )rp->msgptr;
	csp->size = sizeof( MSCP_CMDMSG );
	csp->Disposal = RECEIVE_BUF;

	/* Attempt to send the sequenced message. If scs_send_msg
	 * returns successfully, remove the request from the credit
	 * wait queue, decrement the wait counter, queue the request
	 * to the controller active queue, and zero out the message
	 * buffer pointer to prevent its inadvertent reuse.
	 */
	if(( status = scs_send_msg( csp )) == RET_SUCCESS ) {
	    Lock_connb_credwq (cp);
	    Remove_entry( rp->flink );
	    Unlock_connb_credwq (cp);
	    Decr_rwait( rp );		/* might call mscp_unstall_unit */
	    Lock_connb_actq (cp);
	    Insert_entry( rp->flink, cp->active );
	    Unlock_connb_actq (cp);
	    Lock_connb_credwq (cp);
	    rp->msgptr = NULL;

	/* If scs_send_message returned an error, break out of the loop.
	 */
	} else {
	    is_locked = 0;
	    break;
	}

    }

    if (is_locked) Unlock_connb_credwq (cp);

    Lock_connb (cp);
    cp->flags.serv_cred = 0;
    Unlock_connb (cp);

    }
    else Unlock_connb (cp);

    return;
}

/**/

/*
 *
 *   Name:	mscp_reserve_credit - reserve a send credit
 *
 *   Abstract:	Reserve a send credit for use by connection management.
 *
 *   Inputs:    rp			Request block pointer.
 *
 *   Outputs:	
 *
 *   SMP:	all set here, but the connection block may be locked by
 *		the caller.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_reserve_credit( rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    CSB			csb;
    CSB			*csp = ( CSB * )&csb;
    u_long		scs_status;

    Move_connid( cp->connid, csp->connid );
    if(( scs_status = scs_rsv_credit( csp )) == RET_SUCCESS ) {
	cp->flags.need_cr = 0;
    }
}
/**/

/*
 *
 *   Name:	mscp_unstall_unit - activate stalled requests on a unit
 *
 *   Abstract:	This routine dispatches requests which were stalled on
 *		a unit's request queue.
 *
 *   Inputs:    up			Unit block pointer.
 *
 *   Outputs:	NONE.
 *
 *   SMP:	protect manipulations of the unit's requests-waitng
 *		queue, but do not keep locks across subroutine calls
 *		so, theoretically, the queue may change but it is always
 *		defined.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_unstall_unit( up )
    UNITB		*up;
{
    REQB		*rp;
    int			save_ipl;

    Lock_unitb (up);
    while(( up->rwaitct == 0) &&
           (( rp = up->request.flink ) != ( REQB * )&up->request )) {
        Remove_entry( rp->flink );
	Unlock_unitb (up);
	mscp_dispatch( EV_INITIAL, rp );
	Lock_unitb (up);
    }
    Unlock_unitb (up);
}

/**/

/*
 *
 *   Name:	mscp_alloc_reqb - allocate a request block
 *
 *   Abstract:	Allocate and initialize a request block and return its
 *		address to the caller.  If no request block is available,
 *		return NULL to the caller.
 *
 *   Inputs:	up			Unit block pointer.
 *		bp			Buf structure pointer or NULL.
 *		stbl			State table used by request.
 *		p1			Function-dependent parameter 1. 
 *		p2			Function-dependent parameter 2. 
 *
 *   Outputs:	NONE
 *
 *   SMP:	Protect the unit block.  Protect the class block when we
 *		increment the operation count.
 *		Initialize the request block smp lock.
 *
 *   Return	NONE
 *   Values:
 */

int mscp_reqb_wait;

REQB *
mscp_alloc_reqb( up, bp, stbl, p1, p2 )
    UNITB		*up;
    struct buf		*bp;
    STATE		*stbl;
    u_long		p1;
    u_long		p2;

{
    REQB		*rp;
    CLASSB		*clp = up->connb->classb;
    int			s;

    /* Allocate a request block (sleeping until it becomes available),
     * clear and format it.  Then pass control to a functional routine
     * to start a sequence of MSCP operations or queue the request if
     * activity on the unit is stalled.
     */
    s = Splscs();

    while( 1 ) {
	if(( rp = ( REQB * )mscp_reqbq.flink ) != ( REQB * )&mscp_reqbq ) {
    	    Remove_entry( rp->flink );
            --mscp_reqb_free;
            break;
	} else {
	    mscp_reqb_wait = 1;
	    sleep(( caddr_t )&mscp_reqb_wait, PSWP+1 );
	} 
   }

    rp->unitb = up;
    rp->connb = up->connb;
    rp->classb = clp;
    rp->bufptr = bp;
    rp->p1 = p1;
    rp->p2 = p2;
    Lock_classb (clp);
    rp->op_seq_num = clp->operation_ct++;
    Unlock_classb (clp);
    rp->rwaitptr = &up->rwaitct;
    rp->state_tbl = stbl;

    Lock_unitb (up);
    if( up->rwaitct == 0 ) {
        Unlock_unitb (up);
	mscp_dispatch( EV_INITIAL, rp );
    } else {
	Insert_entry( rp->flink, up->request );
        Unlock_unitb (up);
    }

    splx (s);

    return( rp );
}
/**/

/*
 *
 *   Name:	mscp_dealloc_reqb - deallocate a request block
 *
 *   Abstract:	Deallocate a request block and all of the resources
 *		it holds.
 *
 *   Inputs:    rp			Request block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP	Protect the connection block.
 *
 *   Return	
 *   Values:
 */

void
mscp_dealloc_reqb( rp )
    REQB		*rp;
{
    CONNB		*cp = rp->connb;
    struct buf		*bp;

    /* Deallocate all resources held by the request block.
     */
    mscp_dealloc_all( rp );

    /* If there is a buf structure pointer associated with the operation,
     * fill in the residual byte count and issue an iodone on the buffer.
     */
    if( bp = rp->bufptr ) {
	bp->b_resid = rp->p1;
	iodone( bp );
	rp->bufptr = NULL;
    } else {
	wakeup(( caddr_t )rp );
    }

    /* If the request block is not permanently allocated, deallocate it.
     */
    if( !rp->flags.perm_reqb ) {
	    bzero(( caddr_t )&rp->flink, sizeof( REQB ));
	    Insert_entry( rp->flink, mscp_reqbq );
	    ++mscp_reqb_free;
	    if (mscp_reqb_wait) {
		mscp_reqb_wait = 0;
		wakeup(( caddr_t )&mscp_reqb_wait );
	    }
    }

    /* If the connection is in single stream mode, attempt to restart
     * the next request on the restart queue.
     */
    Lock_connb (cp);
    if( cp->flags.sngl_strm ) {
	Unlock_connb (cp);
	mscp_restart_next( cp ); 
    } else {
	Unlock_connb (cp);
    }

    return;
}

/**/

/*
 *
 *   Name:	mscp_restart_next - Restart next single streamed request
 *
 *   Abstract:	This routine is called during connection recovery to
 *		restart the next request block on the restart queue.
 *		Requests on the restart queue are retried one at a time
 *		in order to isolate and eliminate the command(s) which
 *		may have caused the connection failure.
 *
 *   Inputs:    cp			Connection block pointer.
 *
 *   Outputs:	NONE
 *
 *   SMP:	Lock the connection block to protect manipulations of its
 *		requests-to restart queue; but do not keep locks across
 *		subroutine calls, so, theoretically, the queue may change
 *		but is always defined.
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_restart_next( cp )
    CONNB		*cp;
{
    REQB		*rp;
    struct buf		*bp;
    UNITB		*up;

    /* Get the next request on the restart queue, if any.
     */
    Lock_connb (cp);

    while(( rp = cp->restart.flink ) != ( REQB * )&cp->restart ) {

        Remove_entry( rp->flink );

	/* If this is the first time through restart for this request,
	 * set the single stream flag, update the pointer to the request
	 * being restarted and set the restart command retry count to its
	 * maximum value.
	 */
	if( !cp->flags.sngl_strm || rp != cp->restart_reqb ) {
	    cp->flags.sngl_strm = 1;
	    cp->restart_reqb = rp;
	    cp->restart_count = COMMAND_RETRIES;

	/* This request has been through restart before.  Decrement the
	 * restart retry count.
	 */
	} else {
	    --cp->restart_count;
	}

	/* If any retries remain, set the state to INITIAL, dispatch
	 * the request and break out of the while loop.
	 */
	if( cp->restart_count ) {
	    rp->state = ST_CMN_INITIAL;
	    Unlock_connb (cp);
	    mscp_dispatch( EV_INITIAL, rp );
	    Lock_connb (cp);
	    break;

	/* This request has repeatedly caused the connection to be lost.
	 * If the request has a buf structure associated with it, fill
	 * in the appropriate fields in the buf structure.  Then deallocate
	 * the request block and associated resources.
	 */
	} else
	    {
	    if( bp = rp->bufptr ) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		rp->p1 = bp->b_bcount;
	        }

	    Unlock_connb (cp);
	    mscp_dealloc_reqb( rp );
    	    Lock_connb (cp);
	}
    }

    /* If the restart queue is empty, reset single stream mode then scan
     * the list of unit blocks on the connection and decrement the wait
     * count for each, unstalling the units as appropriate.  When the
     * last unit has been seen, reset the connection restart flag.
     */
    if( cp->restart.flink == ( REQB * )&cp->restart ) {
	cp->flags.sngl_strm = 0;
	for( up = cp->unit.flink;
	     up != ( UNITB * )&cp->unit.flink;
	     up = up->flink ) {
	     
	    Lock_unitb (up);

	    if( up->flags.wait_bump ) {
		up->flags.wait_bump = 0;
		--up->rwaitct;
	    }
	    if( up->rwaitct == 0 ) {
		Unlock_unitb (up);
		Unlock_connb (cp);
		mscp_unstall_unit( up );
		Lock_connb (cp);
		Lock_unitb (up);
		}

	    Unlock_unitb (up);
	}
	cp->flags.restart = 0;
	Unlock_connb (cp);
    }
    else
    {
    Unlock_connb (cp);
    }

    return;
}


/**/

/*
 *
 *   Name:	mscp_dealloc_all - Deallocate all resources held by REQB
 *
 *   Abstract:	Deallocate any resource that is associated with the
 *		input REQB.  Resources may include RSPID, message
 *		buffer or mapping information (local buffer handle).
 *
 *   Inputs:	rp			REQB pointer
 *		    rspid		RSPID
 *		    msgptr		message buffer pointer
 *		    lbhandle		local buffer handle
 *
 *   Outputs:
 *
 *   SMP:	all set here 
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_dealloc_all( rp )
    REQB		*rp;
{
    /* If there is a RSPID in the request block, deallocate it.
     */
    if( *( u_int * )&rp->rspid )
	mscp_dealloc_rspid( rp );

    /* If there is a non-NULL message buffer pointer in the REQB,
     * deallocate the message buffer.
     */
    if( rp->msgptr )
	mscp_dealloc_msg( rp );

    /* If there is a non-NULL local buffer handle in the REQB,
     * deallocate it.
     */
    if( !Test_bhandle( rp->lbhandle ))
	mscp_unmap_buffer( rp );
    return;
}
/**/

/*
 *
 *   Name:	mscp_media_to_ascii - Convert MSCP media code to ASCII.
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

void
mscp_media_to_ascii( media, ascii )
    u_long		media;
    u_char		*ascii;
{
    u_long		temp;

    *ascii++ = (( media >> 17 ) & 0x1f ) + 'A' - 1;
    if( temp = (( media >> 12 ) & 0x1f )) {
	*ascii++ = ( temp + 'A' - 1 );
	if( temp = (( media >> 7 ) & 0x1f ))
	    *ascii++ = ( temp + 'A' - 1 );
    }
    *ascii++ = (( media & 0x7f ) / 10 ) + '0';
    *ascii++ = (( media & 0x7f ) % 10 ) + '0';
    *ascii = '\0';
}

/**/

/*
 *
 *   Name:	mscp_common_init - Do tape and disk common initialization.
 *
 *   Abstract:	
 *
 *   Inputs:
 *
 *   Outputs:
 *
 *   SMP:	Protect the rspid stuff
 *
 *   Return	NONE
 *   Values:
 */

void
mscp_common_init()
{
    int			i, s;
    RSPID_TBL		*rtp;
    REQB		*rp;

    s = Splscs();
    if( !mscp_gbl_flags ) {
	Lock_rspid_db ();
	for( i = 0, rtp = mscp_rspid_tbl;
	     i < nrspid;
	     i++, rtp++ ) {
	    Insert_entry( rtp->flink, mscp_rspid_lh );
	    rtp->reqb = NULL;
	    rtp->rspid.index = i;
	    rtp->rspid.seq_no = 1;
	}
	/*
	 * We will attempt to allocate request blocks so that
	 * we will have "nreqbs" number of request blocks for
	 * use by the MSCP subsystem (see mscp_data.c).
	 */
	for ( i = 0; i < nreqbs; i++) {
	    SCA_KM_ALLOC( rp, REQB *, sizeof( REQB ), KM_CDRP, KM_NOW_CL );
            if( rp != NULL ) {
                Insert_entry( rp->flink, mscp_reqbq );
                ++mscp_reqb_free;
	    }
	}
	mscp_gbl_flags = 1;
	Unlock_rspid_db ();
    }
    splx( s );
    return;
}

/**/

/*
 *
 *   Name:	mscp_poll_wait 
 *
 *   Abstract:	- wait up to 15*count seconds for mscp polling to complete. 
 *      	Global variable mscp_polls is incremented for each
 *      	connection attempt and decremented for each connection 
 *		completion.  If there are no known controllers left to
 *		poll (mscp_polls == 0) then delay one more iteration to
 *		allow recognition of controllers which are slow in making
 *		their presence known.
 *
 *   Inputs:    count - specifies the number of 15 second intervals which are
 *		granted to allow controllers time to report their presence.
 *
 *   Outputs:
 *
 *   SMP:	all set
 *
 *   Return	NONE
 *   Values:
 */

#ifdef __vax
int mscp_polls = 0;
int pollwait = 0;
#else
volatile int do_mscp_poll_wait = 0;
volatile int mscp_polls = 0;
volatile int pollwait = 0;
#endif /* __vax */

void
mscp_poll_wait(  count )
    int 		count;
{
    int			s;

    do {
	s = splsoftclock();
	( void )timeout( wakeup, ( caddr_t )&mscp_polls, hz*15 );
	( void )sleep(( caddr_t )&mscp_polls, PSWP+1 );
	splx(s);
    } while(( mscp_polls > 0 ) && ( count-- > 0 ));
}
