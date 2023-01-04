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
static char *rcsid = "@(#)$ $ (DEC) $";
#endif
/*
 * derived from scs_subr.c	4.1	(ULTRIX)	7/2/90";
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Systems Communication Services
 *
 *   Abstract:	This module contains Systems Communication Services( SCS )
 *		miscellaneous routines and functions.  Some of these functions
 *		are invokable only by port drivers while others are for use
 *		only by SCS.  Still others may be invoked by anyone( SYSAPs,
 *		SCS, PDs ).
 *
 *   Creator:	Todd M. Katz	Creation Date:	July 7, 1985
 *
 *   Function/Routines:
 *
 *   scs_alloc_buf		Allocate all Message and Datagram Buffers
 *   scs_choose_path		Choose Path for New Connection
 *   scs_dealloc_buf		Deallocate all Message and Datagram Buffers
 *   scs_dispose_msg		Dispose of Application Message Buffer
 *   scs_init_cmsb		Initialize CMSB for Notifying SYSAP of Event
 *   scs_initialize		Initialize SCS
 *   scs_unix_to_vms		Convert UNIX Time to VMS Time
 *
 *   sca_zones_init		Initialize SCA memory zones
 *   sca_zalloc			Allocate memory zone
 *   sca_zget			Allocate memory zone
 *   sca_zfree			Free memory zone
 *
 *   Modification History:
 *
 *   31-Oct-1991	Pete Keilty
 *	Added new OSF/1 memory zone allocation routines. 
 *
 *   15-Mar-1991	Brian Nadeau
 *	Port to OSF
 *
 *   14-Sep-1989	Pete Keilty
 *	1. Add bcopy for strings instead of cast u_long assign.
 *	   Mips cpu's needed this for alignment.
 *	2. Removed ifdef mips and ifdef vax which where no longer needed.
 *
 *   30-May-89		Darrell A. Dunnuck
 *	Added include of ../../machine/common/cpuconf.h -- cpu types
 *	were moved there.  Removed the include of ../machine/cpu.h.
 *
 *   06-Apr-1989	Pete Keilty
 *	Added include file smp_lock.h
 *
 *   03-Mar-1989	Todd M. Katz		TMK0003
 *	1. Modify scs_init_cmsb() to always return the connection
 *	   identification number regardless of event status.
 *	2. Include header file ../vaxmsi/msisysap.h.
 *	3. Use the ../machine link to refer to machine specific header files.
 *
 *   05-Jul-1988	Todd M. Katz		TMK0002
 *	1. Include limits.h from ../h instead of from /usr/include.
 *	2. Macros Copy_name() and Copy_data() have been renamed to Move_name()
 *	   and Move_data() respectively.
 *
 *   02-Jun-1988	Ricky S. Palmer
 *	Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   08-Jan-1988	Todd M. Katz		TMK0001
 *	Formated module, revised comments, increased robustness, restructured
 *	code paths, and added SMP support.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<dec/binlog/errlog.h>
#include		<kern/thread.h>
#include 		<vm/vm_kern.h>
#include		<kern/sched_prim.h>
#include		<hal/cpuconf.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<machine/rpb.h>
#include 	        <hal/kn7aa.h>

#define DEBUG_PRINT

/* External Variables and Routines.
 */
extern	struct timezone	tz;
extern	int		cpu;
extern	SCSIB		lscs;
extern	struct utsname	utsname;
extern	struct timeval  boottime;
extern	struct slock	lk_scadb;
extern	CBVTDB		*scs_cbvtdb;
extern	scaaddr		scs_system_id;
extern	void		( *scs_disable )(), scs_shutdown(), scs_unix_to_vms();
extern	u_char		*cpu_types[], scs_node_name[];
extern	u_long		scs_cushion, scs_dg_size, scs_max_conns, scs_msg_size;
extern  caddr_t		sca_zalloc(), sca_zget();
extern  int		sca_zfree(), sca_zones_init(), sca_zones_initialized;
extern  struct zone 	*sca_zone[];
extern  fbq		dsa_fbq;

/*   Name:	scs_alloc_buf	- Allocate all Message and Datagram Buffers
 *
 *   Abstract:	This function allocates all requested message and datagram
 *		buffers and adds them to the appropriate port's free message
 *		and datagram pools.
 *
 *		Partial allocation of either datagram or message buffers may
 *		occur.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *   ndgs			- Number of datagrams to allocate and add
 *   nmsgs			- Number of messages to allocate and add
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *	cinfo.dg_credit		-  Number of datagram credits
 *	cinfo.pend_rec_credit	-  Number of credits pending extension
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Successful buffer allocation
 *   RET_ALLOCFAIL		- Unsuccessful buffer allocation
 *
 *   SMP:	The CB is locked( EXTERNALLY ) to synchronize access and
 *		prevent premature deletion whenever it formerly resided within
 *		the system-wide configuration database.  It is locked through
 *		its CBVTE.
 *
 *		The PB itself must NOT be EXTERNALLY locked as required by the
 *		PD routines which add datagram and message buffers to the
 *		appropriate local port datagram and message free pools.
 */
u_long
scs_alloc_buf( cb, nmsgs, ndgs )
    CB			*cb;
    u_long		nmsgs;
    u_long		ndgs;
{
    SCSH		*scsbp;

    /* Allocate the specified number of message and datagram buffers and add
     * them to the appropriate port's free message and datagram pools
     * respectively.  Return immediately on allocation errors without removing
     * allocated buffers from the appropriate pools and deallocating them.
     */
    for( ; nmsgs; --nmsgs, ++cb->cinfo.pend_rec_credit ) {
	if(( scsbp = ( *cb->Alloc_msg )( cb->pccb ))) {
	    ( void )( *cb->Add_msg )( cb->pccb, scsbp );
	} else {
	    return( RET_ALLOCFAIL );
	}
    }
    for( ; ndgs; --ndgs, ++cb->cinfo.dg_credit ) {
	if(( scsbp = ( *cb->Alloc_dg )( cb->pccb ))) {
	    ( void )( *cb->Add_dg )( cb->pccb, scsbp );
	} else {
	    return( RET_ALLOCFAIL );
	}
    }
    return( RET_SUCCESS );
}

/*   Name:	scs_choose_path	- Choose Path for New Connection
 *
 *   Abstract:	This function chooses the path over which an attempt is made to
 *		establish a new logical SCS connection.  The current algorithm
 *		is to choose an open path to the target system with the least
 *		number of logical SCS connections.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   sb				- System Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   Return Values:
 *
 *   Address of Path Block representing the chosen path
 *   Otherwise NULL if path could not be chosen
 *
 *   SMP:	The SCA database is locked( EXTERNALLY ) allowing for traversal
 *		of the system-wide configuration database.
 */
PB *
scs_choose_path( sb )
    SB			*sb;
{
    pbq			*pb;
    PB			*save = NULL;

    for( pb = sb->pbs.flink; pb != &sb->pbs; pb = pb->flink ) {
	if( Pb->pinfo.state == PS_OPEN ) {
	    if( save == NULL || save->pinfo.nconns > Pb->pinfo.nconns ) {
		save = Pb;
	    }
	}
    }
    return( save );
}

/*   Name:	scs_dealloc_buf	- Deallocate all Message and Datagram Buffers
 *
 *   Abstract:	This function deallocates all of a CB's datagram and message
 *		buffers which it removes from the appropriate port's free
 *		datagram and message pools.  It also removes and deallocates
 *		one message buffer for each block data transfer currently in
 *		progress.
 *
 *		Partial removal of either datagram or message buffers may
 *		occur.
 *
 *		NOTE: The appropriate port drivers are always responsible for
 *		      the clean up of all port specific resources associated
 *		      with failed local ports including free datagram and
 *		      message buffers.  SCS must never attempt to dispose of
 *		      such resources during clean up of the paths and
 *		      connections associated with such failed ports.
 *		
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *	cinfo.dg_credit		-  0
 *	cinfo.ntransfers	-  0
 *	cinfo.pend_rec_credit	-  0
 *	cinfo.rec_credit	-  0
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Successful deallocation of all buffers
 *   RET_FAILURE		- Error encountered deallocating buffers
 *
 *   SMP:	The CB is locked( EXTERNALLY ) to synchronize access and
 *		prevent premature deletion.  It is locked through its CBVTE.
 *
 *		The PB itself must NOT be EXTERNALLY locked as required by the
 *		PD routines which remove datagram and message buffers from the
 *		appropriate local port datagram and message free pools.
 */
u_long
scs_dealloc_buf( cb )
    CB			*cb;
{
    u_long		nbs;
    SCSH		*scsbp,
			*( *remove )() = cb->Remove_msg;
    void		( *dealloc )() = cb->Dealloc_msg;
    u_long		status = RET_SUCCESS;

    for( nbs = ( cb->cinfo.rec_credit	   +
		 cb->cinfo.pend_rec_credit +
		 cb->cinfo.ntransfers ),
	 cb->cinfo.rec_credit = 0,
	 cb->cinfo.pend_rec_credit = 0,
	 cb->cinfo.ntransfers = 0;
	 nbs;
	 --nbs ) {
	if(( scsbp = ( *remove )( cb->pccb ))) {
            if( scsbp != ( SCSH * )RET_SUCCESS )
                ( void )( *dealloc )( cb->pccb, scsbp );
	} else {
	    status = RET_FAILURE;
	    break;
	}
    }
    for( remove = cb->Remove_dg,
	 dealloc = cb->Dealloc_dg,
	 nbs = cb->cinfo.dg_credit,
	 cb->cinfo.dg_credit = 0;
	 nbs;
	 --nbs ) {
	if(( scsbp = ( *remove )( cb->pccb ))) {
            if( scsbp != ( SCSH * )RET_SUCCESS )
                ( void )( *dealloc )( cb->pccb, scsbp );
	} else {
	    status = RET_FAILURE;
	    break;
	}
    }
    return( status );
}

/*   Name:	scs_dispose_msg	- Dispose of Application Message Buffer
 *
 *   Abstract:	This function disposes of an application message buffer on a
 *		specific logical SCS connection according to the connection's
 *		current credit situation.  The buffer is either deallocated or
 *		converted into a receive message buffer for the connection.
 *
 *		Conversion of the application message buffer into a receive
 *		message buffer may trigger explicit extension of send credits
 *		across the logical SCS connection.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *	cinfo.cstate		-  CS_OPEN
 *   lk_scadb			- SCA database lock structure
 *   scs_cbvtdb			- CB vector table database pointer
 *   scs_cushion		- SCS Send credit cushion
 *   scsbp			- Address of SCS header of message buffer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *	cinfo.cstate		-  Connection state
 *	cinfo.pend_rec_credit	-  Number of credits pending extension
 *
 *   SMP:	The SCA database is locked ONLY for explicit extension of send
 *		credits following conversion of application sequenced messages
 *		into receive message buffers.
 *
 *		The CB is locked( EXTERNALLY ) to synchronize access and
 *		prevent deletion.  It is locked through its CBVTE.  Locking the
 *		CB also prevents PB deletion as required when explicit
 *		extension of send credits due to message buffer conversion
 *		becomes necessary.
 *
 *		The PB itself must NOT be EXTERNALLY locked in case the message
 *		buffer is added to the appropriate local port message free
 *		pool.
 */
void
scs_dispose_msg( cb, scsbp )
    CB			*cb;
    SCSH		*scsbp;
{
    u_long		sn;
    CBVTE		*cbvte;

    /* Deallocate the application message buffer if the connection's current
     * number of receive message buffer's exceeds or is equal to its maximum
     * number of such buffers.  Otherwise, add the message buffer to the
     * appropriate port's free message pool and increment the number of credits
     * pending extension across the connection.
     *
     * Explicit extension of send credits is requested whenever the following
     * conditions are met:
     *
     * 1. SCS is currently not in the extending credits across the connection.
     * 2. The number of send credits held by the counterpart is less than its
     *	  minimum credit requirement plus a local system-wide credit cushion.
     *
     * Explicit send credit extensions require the SCA database to be locked.
     * In order to preserve the SCA locking hierarchy, the CB lock is first
     * released and then re-obtained after the SCA database is locked.  Note
     * that releasing the CB lock allows the connection state to change and the
     * the CB to be deallocated.  Credits are NOT extended when this occurs.
     */
    if(( cb->cinfo.pend_rec_credit +
	  cb->cinfo.rec_credit  ) >= cb->cinfo.init_rec_credit ) {
	( void )( *cb->Dealloc_msg )( cb->pccb, scsbp );
    } else {
	( void )( *cb->Add_msg )( cb->pccb, scsbp );
	++cb->cinfo.pend_rec_credit;
	if((( cb->cinfo.min_rec_credit +
	       scs_cushion ) > cb->cinfo.rec_credit ) &&
	    cb->cinfo.cbstate != CB_CREDIT_PEND ) {
	    cbvte = Get_cbvte( cb->cinfo.lconnid );
	    sn = cbvte->connid.seq_num;
	    Unlock_cbvte( cbvte )
	    Lock_scadb();
	    Lock_cbvte( cbvte );
	    if( cbvte->connid.seq_num == sn && cb->cinfo.cstate == CS_OPEN ) {
		( void )scs_request( CB_CREDIT_PEND, cb, cb->pb, NULL );
	    }
	    Unlock_scadb()
	}
    }
}

/*   Name:	scs_init_cmsb	- Initialize CMSB for Notifying SYSAP of Event
 *
 *   Abstract:	This routine initialized a CMSB in preparation for asynchronous
 *		local SYSAP notification of the occurrence of a specific event
 *		on a specific connection.
 *
 *		The events SYSAPs may be asynchronously notified of are:
 *
 *		 1. Received connection request over listening SCS connection.
 *		 2. Completed attempt to establish logical SCS connection.
 *		 3. Completed attempt to accept connection request.
 *		 4. Completed attempt to reject connection request.
 *		 5. Received disconnection request over logical SCS connection.
 *		 6. Completed disconnection of logical SCS connection.
 *		 7. Connection disconnected due to path failure.
 *		 8. Discovered path to unknown system.
 *		 9. Completed block data transfer.
 *		10. Extended send credits over logical SCS connection.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   blockid			- Block data transfer identification
 *   cb				- Connection Block pointer
 *   cmsb			- Connection Management Service Block pointer
 *   event			- Event type
 *   pb				- Path Block pointer
 *   status			- Event status
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cmsb			- Connection Management Service Block pointer
 *				   ( Initialized according to event type )
 *
 *   SMP:	No locks are required; however, the CB must EXTERNALLY be
 *		prevented from deletion.  This in turn prevents the deletion of
 *		the corresponding PB and SB which is also required.
 */
void
scs_init_cmsb( event, status, cmsb, cb, pb, blockid )
    u_long		event;
    u_long		status;
    CMSB		*cmsb;
    CB			*cb;
    PB			*pb;
    u_long		blockid;
{
    /* The CMSB is formated according to the specific event the SYSAP is to be
     * asynchronously notified of.  Panic occurs on unknown events.
     *
     * NOTE: Block transfer completions are processed outside the switch
     *	     structure for performance reasons.
     */
    if( event == CRE_BLOCK_DONE ) {
	cmsb->aux = cb->aux;
	cmsb->status = ADR_SUCCESS;
	cmsb->blockid = blockid;
	Move_connid( cb->cinfo.lconnid, cmsb->connid )
    } else {
	switch( event ) {

	    case CRE_CONN_REC:
	    case CRE_CONN_DONE:
	    case CRE_ACCEPT_DONE:
	    case CRE_REJECT_DONE:
		if(  status == ADR_SUCCESS ) {
		    Move_data( cb->cinfo.rconn_data, cmsb->conn_data )
		    cmsb->min_snd_credit = cb->cinfo.min_rec_credit;
		    cmsb->Init_snd_credit = cb->cinfo.snd_credit;
		}

		/* Fall through.
		 */
	    case CRE_DISCONN_DONE:
	    case CRE_PATH_FAILURE:
	    case CRE_DISCONN_REC:
		Move_name( cb->cinfo.rproc_name, cmsb->rproc_name )

		/* Fall through.
		 */
	    case CRE_NEW_PATH:
		Move_scaaddr( pb->sb->sinfo.sysid, cmsb->sysid )
		Move_scaaddr( pb->pinfo.rport_addr, cmsb->rport_addr )
		cmsb->lport_name = pb->pinfo.lport_name;
		Move_name( cb->cinfo.lproc_name, cmsb->lproc_name )

		/* Fall through.
		 */
	    case CRE_CREDIT_AVAIL:
		cmsb->aux = cb->aux;
		cmsb->status = status;
		Move_connid( cb->cinfo.lconnid, cmsb->connid )
		break;

	    default:
		( void )panic( SCSPANIC_EVENT );
	}
    }
}

/*   Name:	 scs_initialize	- Initialize SCS
 *
 *   Abstract:	This function initializes SCS and all of SCS's data structures.
 *		Each port driver may call this function once during its own
 *		initialization.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   SID			- CPU system identification register
 *   boottime			- Time of last system boot
 *   cpu			- CPU type identification code
 *   lk_scadb			- SCA database lock structure
 *   scs_cbvtdb			- CB vector table database pointer
 *   scs_dg_size		- Maximum application datagram size
 *   scs_disable		- Address of SCS shutdown routine
 *   scs_max_conns		- Maximum number of logical SCS connections
 *   scs_msg_size		- Maximum application sequenced message size
 *   scs_node_name		- SCS node name
 *   scs_system_id		- SCS system identification number
 *   utsname.release		- Current Ultrix version
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   lscs			- Local system permanent information
 *				  ( INITIALIZED )
 *   lk_scadb			- SCA database lock structure
 *   scs_cbvtdb			- CB vector table database pointer
 *				  ( INITIALIZED )
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- SCS successfully initialized
 *   RET_ALLOCFAIL		- Storage allocation failure occurred
 *   RET_ZEROSYSID		- scs_system_id has not been assigned
 *
 *   SMP:	No locks are required even though many shared data structures
 *		are manipulated.  This function is only called during system
 *		initialization and at that time only the processor executing
 *		this code is operational.  This guarantees uncompromised access
 *		to all SCS data structures without locking the SCA database.
 *
 *		SCA database and CBVTE lock structures are initialized.
 */
u_long
scs_initialize()
{
    long		index;
    u_long		size;
    CBVTE		*cbvte;

    /* Immediately return success if SCS has already been initialized.  Return
     * an error if scs_system_id has not been initialized.  Otherwise,
     * initialize SCS as follows:
     *
     * 1. Allocate and zero the dynamic memory required for the CB vector table
     *	  database.  Immediately return if insufficient dynamic memory is
     *	  available.
     * 2. Initialize the local system information.
     * 3. Initialize the CB vector table database and all CBVTEs.  Place all of
     *	  the CBVTEs onto the CBVTE free list.
     * 4. Initialize the SCA database lock structure.
     * 5. Initialize the global variable containing the address of the SCS
     *	  shutdown routine.
     *
     * NOTE: The system identification number is zero by default.  By
     *	     convention this number is not allowed to be 0.
     */
    if( lscs.system.swtype ) {
#ifdef DEBUG_PRINT_OFF
    printf(">>> scs_initialize: lscs.system.swtype SUCCESS\n");
#endif /* DEBUG_PRINT ALPHA */
	return( RET_SUCCESS );
    } else if( Test_scaaddr( scs_system_id )) {
#ifdef DEBUG_PRINT
    printf(">>> scs_initialize: scs_system_id ZEROSYSID\n");
#endif /* DEBUG_PRINT ALPHA */
	return( RET_ZEROSYSID );
    }
    size = ( sizeof( CBVTDB ) + ( sizeof( CBVTE ) * scs_max_conns ));
    while( size > PAGE_SIZE ) { /* OSF/MIPS contigous mem size, fine for now */
	size -= sizeof( CBVTE );/* I don't think this will happen but be safe*/
	scs_max_conns--;
    }
    SCA_KM_ALLOC( scs_cbvtdb, CBVTDB *, size, KM_SCA, KM_NOW_CL_CA )
    if( scs_cbvtdb ){
	U_int( scs_cbvtdb->size ) = size;
	scs_cbvtdb->type = DYN_CBVTDB;
	scs_cbvtdb->cbvt = ( CBVTE * )( scs_cbvtdb + 1 );
    } else {
#ifdef DEBUG_PRINT
    printf(">>> scs_initialize: scs_cbvtdb ALLOCFAIL\n");
#endif /* DEBUG_PRINT ALPHA */
	return( RET_ALLOCFAIL );
    }
    lscs.max_conns = scs_max_conns;
    Move_scaaddr( scs_system_id, lscs.system.sysid )
    lscs.system.max_dg = scs_dg_size;
    lscs.system.max_msg = scs_msg_size;
    bcopy( "OSF ", ( char * )&lscs.system.swtype, 4 );
    bcopy( "V1.3", (char * )&lscs.system.swver, 4 );
    ( void )scs_unix_to_vms( &boottime, &lscs.system.swincrn );
    if( cpu <= CPU_MAX ) {
#ifdef __alpha
	if ( (cpu == DEC_3000_500) || (cpu == DEC_7000) ) 
		bcopy( "ALPH ", ( char * )&lscs.system.hwtype, 4 );
		if ( cpu == DEC_7000 ) {   /* RUBY */
/* this need to be updated when more than one cpu is supported on Ruby */
			switch ((rpb->rpb_sysvar & SV_STS_MASK) >> 10) {
				case DEC7000_SYSVAR:
   				     lscs.system.hwver.val[ 0 ] = 3;
    				     lscs.system.hwver.val[ 1 ] = 0;
				     lscs.system.hwver.val[ 2 ] = 0x04010000; 
				     break;
				case DEC10000_SYSVAR:
   				     lscs.system.hwver.val[ 0 ] = 3;
    				     lscs.system.hwver.val[ 1 ] = 0;
				     lscs.system.hwver.val[ 2 ] = 0x04050000; 
				     break;
			}
		}
		else {  /* FLAMINGO */
   			lscs.system.hwver.val[ 0 ] = 4;
    			lscs.system.hwver.val[ 1 ] = 0;
			lscs.system.hwver.val[ 2 ] = 0x04090000; 
	}
#else /* __mips */
	if ( cpu == DS_5000_300 ) {		/* Bigmax */
		bcopy( "BIGM", (char * )&lscs.system.hwtype, 4);
/* this hwver needs to be changed for VMS show cluster command if VMS 
   decides ever to support MIPS machines */ 
    		bcopy("5000_300    ", ( char * )&lscs.system.hwver, 12 );
	}
#endif
    } else {
	bcopy( "????", ( char * )&lscs.system.hwtype, 4 );
    	lscs.system.hwver.val[ 0 ] = cpu;
    	lscs.system.hwver.val[ 1 ] = 0;
	lscs.system.hwver.val[ 2 ] = 0; 
    }

    Move_node( scs_node_name, lscs.system.node_name )
    for( index = ( lscs.max_conns - 1 ), cbvte = ( scs_cbvtdb->cbvt + index );
	 index >= 0;
	 --index, --cbvte ) {
	Init_cbvte_lock( cbvte )
	cbvte->connid.index = index;
	cbvte->connid.seq_num = 1;
	cbvte->ov1.cbvte = scs_cbvtdb->free_cbvte;
	scs_cbvtdb->free_cbvte = cbvte;
    }
    Init_scadb_lock()
    scs_disable = scs_shutdown;

#ifdef DEBUG_PRINT_OFF
    printf(">>> scs_initialize: complete SUCCESS\n");
#endif /* DEBUG_PRINT ALPHA */

    return( RET_SUCCESS );
}

/*   Name:	scs_unix_to_vms	- Convert UNIX Time to VMS Time
 *
 *   Abstract:	This subroutine converts the given UNIX time structure to
 *		64-bit format.  The time is returned in 100-nanosecond units
 *		from the VMS system base time.  The VMS system base time is
 *		00:00 o'clock November 17, 1858 ( the Smithsonian base date and
 *		time for the astronomical calendar ) local time.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   unix_time			- UNIX time value source pointer
 *   vms_time			- VMS time value destination pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   vms_time			- VMS time value destination pointer
 *
 *   SMP:	No locks are required.
 */
void
scs_unix_to_vms( unix_time, vms_time )
    struct timeval		*unix_time;
    u_quad			*vms_time;
{
    unsigned int		*r11;
    struct timeval		*r10;
    struct timeval		scratch;

    /* The following steps convert UNIX time to VMS time:
     *
     * 1. Convert the Greenwich UNIX time to local UNIX time( VMS time is kept
     *    relative to local time ).
     * 2. Convert the micro-seconds and seconds to 100-nanosecond units and add
     *    them together.
     * 3. Add the difference between 00:00 o'clock Jan. 1, 1970 and 00:00
     *    o'clock November 17, 1858 to form the VMS time value.
     */
    scratch = *unix_time;
    scratch.tv_sec -= tz.tz_minuteswest * 60;
    scratch.tv_usec *= 10;
    r10 = &scratch;
    r11 = ( unsigned int * )vms_time;

    /* VAX inline asm; MIPS asm routine */
#ifdef __mips
    unixtovms(r10,r11);
#endif /* JAY_SAYS_NOT_YET FOR ALPHA */
}

struct zone *sca_zone[SCA_ZONE_MAX];
static char *sca_zone_name[SCA_ZONE_MAX] = {
	"scazone.64",
	"scazone.256",
	"scazone.512",
	"scazone.1024",
	"scazone.2048",
};
unsigned int sca_zone_size[SCA_ZONE_MAX] = {
      64,		/*     64 Byte  */
      256,		/*    256 Byte  */
      512,		/*    512 Byte  */
      1024,		/*   1024 Byte  */
      2048,		/*   2048 Byte  */
};

extern int sca_zone_max[SCA_ZONE_MAX];	/* actual #zone entries in scs_data */

struct sca_zone_cnt {
    int total;
    int free;
    int inuse;
    int highwater;
} sca_zone_history[SCA_ZONE_MAX];

long sca_nonzone_allocs = 0;

/* MIPS HACK!! until gold when vm has a contiguous memory allocator */
/* 8k contiguous needed for N_PORT and UQSSP ports data structures */
caddr_t dsa_physical_bg  = NULL;
caddr_t dsa_physical_end = NULL;
struct dsa_physzone_qe {
    struct phys_zone_qe *flink;
    struct phys_zone_qe *blink;
} dsa_physzone_qh = { 
    (struct phys_zone_qe *)&dsa_physzone_qh, 
    (struct phys_zone_qe *)&dsa_physzone_qh 
};
struct dsa_physzone_buf {
    struct phys_zone_qe *flink;
    struct phys_zone_qe *blink;
    caddr_t		ptr;
};


int sca_zones_initialized = 0;

sca_zones_init()
{
    int 		i, size;
    vm_offset_t 	sca_zone_addr;

    if (sca_zones_initialized) return;
	sca_zones_initialized++;

    for( i = 0; i < SCA_ZONE_MAX; ++i ) {
	sca_zone[i] = zinit( sca_zone_size[i], 
			     sca_zone_max[i] * sca_zone_size[i],
			     PAGE_SIZE, 
			     sca_zone_name[i]);
	zchange( sca_zone[i],FALSE,FALSE,FALSE,FALSE);
        if(( sca_zone_addr = kmem_alloc( kernel_map,
			        sca_zone_max[i] * sca_zone_size[i])) == NULL )
            panic ("scs_subr: sca zone mem alloc failed.\n");
        zcram(sca_zone[i], sca_zone_addr, sca_zone_max[i] * sca_zone_size[i]);
	sca_zone_history[i].total = sca_zone_max[i];
	sca_zone_history[i].free  = sca_zone_max[i];
	sca_zone_history[i].inuse = 0;
        sca_zone_history[i].highwater = 0;
    }

	/* HACK!! until gold when vm has a contiguous memory allocator */
	/* 8k contiguous need for N_PORT and UQSSP ports data structures */
    {
    struct dsa_physzone_buf *bufp;

    if( dsa_physical_bg != dsa_physical_end ) {
	do {
	    bufp = (struct dsa_physzone_buf *)dsa_physical_bg;
	    bufp->ptr = dsa_physical_bg;
            Insert_entry( bufp->flink, dsa_physzone_qh );
	    dsa_physical_bg += 8192;
	} while( dsa_physical_bg != dsa_physical_end );
    }
    }
}

caddr_t sca_zalloc(size)
    long 		size;
{
    int 		zindex;
    caddr_t 		addr;

    if (!sca_zones_initialized) sca_zones_init();

	/* compute the size of the block that we will actually allocate */

    for( zindex = 0; zindex < SCA_ZONE_MAX; ++zindex ) {
	if( size <= sca_zone_size[zindex] ) {
	    break;
	}
    }

	/*
	 * If our size is still small enough, check the queue for that size
	 * and allocate.
	 */

    if( zindex < SCA_ZONE_MAX ) {
	addr = (caddr_t) zalloc(sca_zone[zindex]);
    } else {
	addr = (caddr_t) kmem_alloc(kernel_map, size);
    }
    return(addr);
}

caddr_t sca_zget(size)
    long 		size;
{
    int 		zindex;
    caddr_t 		addr;

    if (!sca_zones_initialized) sca_zones_init();

    /* compute the size of the block that we will actually allocate */

    for( zindex = 0; zindex < SCA_ZONE_MAX; ++zindex ) {
	if( size <= sca_zone_size[zindex] ) {
	    break;
	}
    }

    /*
     * If our size is still small enough, check the queue for that size
     * and allocate.
     */

    if( zindex < SCA_ZONE_MAX ) {
	addr = (caddr_t) zget(sca_zone[zindex]);
	if( addr == NULL ) {
	/* Informational message only. Use for debug */
/*  	 printf("sca_zget: NO mem in %s: free=%d inuse=%d\n",
	        sca_zone_name[zindex],
	        sca_zone_history[zindex].free,
	        sca_zone_history[zindex].inuse );  */
	} else {
	    sca_zone_history[zindex].free--;
	    sca_zone_history[zindex].inuse++;
	    if( sca_zone_history[zindex].inuse > 
		sca_zone_history[zindex].highwater ) {
		sca_zone_history[zindex].highwater = 
		sca_zone_history[zindex].inuse;
	    }
	}
    } else if( PAGE_SIZE == 4096 && size == 8192 ) {
	/* MIPS HACK!! until gold when vm has a contiguous memory allocator */
	/* 8k contiguous needed for N_PORT and UQSSP ports data structures */
	{
    	struct dsa_physzone_buf *bufp;
        if(( bufp = (struct dsa_physzone_buf *)dsa_physzone_qh.flink ) != 
		    (struct dsa_physzone_buf *)&dsa_physzone_qh) {
            Remove_entry( bufp->flink );
	    addr = bufp->ptr;
	} else {
	    addr = NULL;
	}
	}
    } else {
	addr = (caddr_t) kmem_alloc(kernel_map, size);
        sca_nonzone_allocs++;
    }
    return(addr);
}

sca_zfree(data, size)
    caddr_t		data;
    long		size;
{
    int 		zindex;

    for( zindex = 0; zindex < SCA_ZONE_MAX; ++zindex ) {
	if( size <= sca_zone_size[zindex] ) {
	    break;
	}
    }
    if( zindex < SCA_ZONE_MAX ) {
        zfree(sca_zone[zindex], data);
	sca_zone_history[zindex].free++;
	sca_zone_history[zindex].inuse--;
    } else if( PAGE_SIZE == 4096 && size == 8192 ) {
	/* MIPS HACK!! until gold when vm has a contiguous memory allocator */
	/* 8k contiguous needed for N_PORT and UQSSP ports data structures */
	{
    	struct dsa_physzone_buf *bufp;
	bufp = (struct dsa_physzone_buf *)data;
	bufp->ptr = data;
        Insert_entry( bufp->flink, dsa_physzone_qh );
	}
    } else {
        kmem_free(kernel_map, data, size);
    }
}


/*
 * Routine Name :  dsaisr_thread
 *
 * Functional Description :
 *
 * Call Syntax : dsaisr_thread()
 *
 * Arguments :	None
 *
 * Return Value :  None
 */
void
dsaisr_thread()
{
        thread_t thread = current_thread();
	long	 once;

        thread_swappable(thread, FALSE);
/*
 * RT_SCHED: Change hardcoded priority to a constant.  Always valid.
 * The constant is defined in kern/sched.h.
 * Stomping on sched_pri here won't hurt, because the thread is running and
 * therefore not on a run queue.
 */
        thread->priority = thread->sched_pri = BASEPRI_HIGHEST;

        (void) spl0();

        for ( ;; ) {
            assert_wait((vm_offset_t)dsaisr_thread, FALSE);
	    {
		u_long			save_ipl;
		struct kschedblk 	*fbp;
		
		save_ipl = splbio();
		/* Lock needed for fbq SMP */
                if((( fbq *)fbp = dsa_fbq.flink ) == ( fbq *)&dsa_fbq ) {
    		    ( void )splx( save_ipl );
		} else {
		    Remove_entry( fbp->flink )
    		    ( void )splx( save_ipl );
		    clear_wait(thread,THREAD_AWAKENED,FALSE);
		    ( *fbp->func )( fbp->arg );
		}	
	    }
            thread_block();
        }
        /* NOTREACHED */
}

int     dsaisr_thread_init = 0;

/*
 * Routine Name :  dsaisr_init
 *
 * Functional Description : Initialize the dsaisr_thread.
 *
 * Call Syntax : dsaisr_init()
 *
 * Arguments:	None
 *
 * Return Value :  None
 */
void
dsaisr_init()
{
        extern task_t first_task;

	if (dsaisr_thread_init)
                return;
        dsaisr_thread_init++;

        (void) kernel_thread(first_task, dsaisr_thread);
}
