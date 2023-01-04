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
static char *rcsid = "@(#)$RCSfile: np_init.c,v $ $Revision: 1.1.10.4 $ (DEC) $Date: 1993/09/21 21:54:39 $";
#endif
/*
 * derived from np_init.c	5.2	(ULTRIX)	10/16/91";
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		initialization routines and functions.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Function/Routines:
 *
 *   np_init_port		Initialize a Local CI Port
 *   np_probe			Probe a Local CI Port
 *   np_setup_port		Prepare Local CI Port for Initialization
 *   np_test_port		Test Operational Status of Local CI Port
 *   npsc_start			Start a Local N_PORT Single Channel Family Port
 *
 *   Modification History:
 *
 *   31-Oct-1991	Peter Keilty
 *	Ported to OFS/1
 *	Changed memeory allocation to OSF/1 memory zones. 
 *	Routines are in scs_subr.c.
 *	Added new svatophys routine from OSF/1 code base.
 *
 *   16-Oct-1991	Brian Nadeau
 *      Updates/bug fixes.
 *
 *   23-Jul-1991	Brian Nadeau
 *	New NPORT module.
 *
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<sys/systm.h>
#include		<dec/binlog/errlog.h>
#include		<machine/pmap.h>
#include		<machine/cpu.h>
#include		<machine/nexus.h>
#include		<io/common/devdriver.h>
#include		<io/dec/xmi/xmireg.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/np/npport.h>
#include		<io/dec/np/npadapter.h>
#include		<io/dec/ci/cippd.h>
#include		<io/dec/mbox/mbox.h>

/* External Variables and Routines.
 */
extern	int		cpu;
extern	SCSIB		lscs;
extern	PDT		np_pdt;
extern	NPISR		ci_isr[];
extern	NPBDDB		*np_bddb;
extern	int		*np_ucode;
extern	struct slock	lk_scadb;
extern	pccbq		scs_lport_db;
extern	MRLTAB		cimna_mrltable[], citca_mrltable[];
extern	SCSH		*np_alloc_dg(), *np_alloc_msg();
extern	u_long		*cimna_regoff[], *citca_regoff[];
extern	u_short		np_cippdburst, np_cippdcontact, ci_first_port,
			np_max_reinits, np_maint_intrvl, np_maint_timer,
			np_ucode_type, cippd_max_port, np_nosanity_chk;
extern	void		np_crash_lport(), np_dealloc_pkt(), np_init_port(),
			np_log_dev_attn(), np_log_initerr(), np_notify_port(),
			np_unmap_port(), np_unmapped_isr(),
			npsc_disable(), cippd_start(), np_dealloc_dg(),
			np_dealloc_msg();
extern	u_int		np_crctable[], np_max_bds,
			scs_dg_size, scs_msg_size, np_intr_holdoff;
extern  u_long		np_setup_port(), np_test_port(), npsc_start(),
			np_mna_start(), np_initialize(), scs_initialize();

/*   Name:	np_init_port	- Initialize a Local CI Port
 *
 *   Abstract:	This routine directs the initialization of a local CI port.  It
 *		also oversees the port's initial initialization.  It is always
 *		invoked by forking to it.
 *
 *		The local port must be completely disabled whenever execution
 *		of this routine is scheduled including disablement of:
 *
 *		1. Port interrupts.
 *		2. Port boot/sanity timer.
 *		3. CI PPD finite state machine activity on the local port.
 *
 *		The local CI port must also be in the UNINITIALIZED port state.
 *
 *		This routine may also be scheduled to permanently shutdown
 *		local ports.  Such ports are always unmapped and marked broken
 *		in addition to having been disabled.  This distinguishes them
 *		from local ports to be initialized which are mapped and not
 *		marked broken.
 *
 *		NOTE: This is a mandatory PD function( Init_port ) for use by
 *		      the CI PPD.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   np_cippdburst		- CI PPD port polling burst size
 *   np_cippdcontact		- CI PPD port polling contact frequency
 *   ci_first_port		- Port number of first local CI port
 *   np_maint_intrvl		- CI port maintenance timer interval
 *   np_maint_timer             - CI port maintenance timer enable flag
 *   np_max_reinits		- CI max number consecutive reinitializations
 *   pccb			- Port Command and Control Block pointer
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.cleanup	-   1
 *	    fsmstatus.fkip	-   1
 *	    fsmstatus.online	-   0
 *   lk_scadb			- SCA database lock structure
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   ci_first_port		- Port number of first local CI port
 *   pccb			- Port Command and Control Block pinter
 *	lpinfo.addr		-  Local port station address
 *	lpinfo.nreinits		-  Number of local port re-initializations
 *	lpinfo.ppd.cippd	-  CI PPD specific local port information
 *	    max_port		-   Maximum hardware remote port number
 *	pd.gvp.pqb.type.ci	-  CI specific PQB fields
 *	    keepalive		-   Variable maintenance timer interval cell
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    devattn.cirevlev	-   Port microcode information
 *	    lbcrc		-   Loopback CRC
 *	    lbstatus		-   0
 *	    lpstatus		-   Local port status flags
 *	        init		-    First time initialization status flag
 *		connectivity	-    0
 *	    reinit_tries	-   Number consecutive re-initializations left
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    burst		-   Port polling burst size
 *	    contact		-   Port polling contact frequency
 *	    fsmstatus.broken	-   Port is broken status flag
 *	    fsmstatus.cleanup	-   0
 *	    fsmstatus.fkip	-   Fork operation in progress status flag
 *
 *   SMP:	The SCA database is locked for deletion of the PCCB following
 *		exhaustion of consecutive re-initialization attempts without a
 *		successful initial port initialization.
 *
 *		The PCCB is locked to synchronize access and as required by
 *		np_log_dev_attn() in case event logging becomes necessary.
 *		PCCB locking is probably unnecessary because of lack of
 *		conflict for the PCCB due to the single threadedness of port
 *		clean up and initialization.  It is done anyway to guarantee
 *		unrestricted access and because the CI PPD interval timer may
 *		still be active.  PCCB addresses are always valid because these
 *		data structures are never deleted( except by this function on
 *		exhaustion of consecutive re-initialization attempts without a
 *		successful initial port initialization ).
 *
 *		Access to port queues is by means of memory interlocking
 *		queuing instructions.
 */
void
np_init_port( pccb )
    PCCB		*pccb;
{
    MRLTAB		*mrltab;
    u_long		status;
    u_long 		save_ipl = Splscs();

    /* The steps involved in port initialization are as follows:
     *
     *  1. The PCCB is locked.
     *  2. Port clean up is marked completed.
     *  3. The existence of port power is verified( non-broken ports only ).
     *
     *  - PORT RE-INITIALIZATION ONLY:
     *  4. All port queues are flushed and all flushed packets are deallocated.
     *  5. The local port maintenance timer interval, CI PPD port polling burst
     *	   size, and CI PPD port polling contact frequency are reset.
     *  6. Local port loopback is enabled and loopback status is cleared.
     *
     *  - ALL INITIALIZATIONS:
     *  7. Presence of an operational port is verified.
     * 10. An initial number of free datagrams and messages are allocated and
     *	   and placed on the appropriate port queues.
     * 12. Both port interrupts and the port itself are enabled.
     * 13. The port is notified of free message and datagram buffer
     *	   availability.
     *
     * - INITIAL PORT INITIALIZATION ONLY:
     * 14. The local port number is retrieved and stored as the port number of
     *	   the first local CI port encountered( when appropriate ).
     * 15. The local port loopback datagram CRC is computed.
     * 16. The maximum addressable port number is retrieved and verified.
     * 17. Microcode revision levels( ram/functional and prom/self-test ) are
     *	   verified.
     *
     *  - ALL INITIALIZATIONS:
     * 18. The initialization of the local CI port is logged.
     * 19. The local CI port maintenance timer is engaged provided CI port
     *	   maintenance timers have been optionally enabled.
     * 20. The PCCB is unlocked.
     * 21. The CI PPD is notified of successful port initialization.
     *
     * Local ports can not be initialized when they are without power.  This
     * presents no problems for those instances of power failure which involve
     * the entire system.  It also poses no problems for local CITCA/CIMNA
     * ports which can not independently power failure.  Unfortunately,
     * potential problems do exist for power failure of local CI750/CI780/CIBCI
     * ports.  Such ports can power failure independently.  Worse yet, power
     * loss and gain on such ports can be discrete events separated in time.
     * This allows for the local port disablement and clean associated with the
     * loss of port power to complete and local port initialization to commence
     * before power is restored to the port.  It is also the reason why
     * verification of port power occurs( Step 3 ) before proceeding further
     * with port initialization, to insure that initialization is postponed
     * while local ports are without power.  For a more extensive discussion of
     * possible power failure recovery scenarios consult np_crash_lport().
     *
     * All port queues are flushed immediately prior to port re-initialization(
     * Step 4 ) to insure their pristine state.  Logically this action should
     * occur as part of port clean up.  However, it is done immediately prior
     * to port re-initialization in order to be able to meet the following
     * requirements:
     *
     * 1. Port queues can not be flushed until all possibility of further
     *	  packet queuing to them has ceased.
     *
     * 2. Port queue flushing must involve determination of whether or not the
     *	  queues were left permanently locked by failure of the port( Discovery
     *    of a locked queue or any failure to obtain a queue interlock during
     *	  queue flushing results in zeroing of the offending queue and
     *	  permanent loss of all packets residing on it ).
     *
     * Neither of these requirements can be easily met during port clean up.
     * Nothing can prevent SYSAP or SCS motivated insertion of packets into
     * port queues without seriously affecting performance until all paths
     * associated with the failed port have been terminated.  This condition is
     * not reached until the virtual end of port clean up.  Furthermore, until
     * port re-initialization begins, multiple simultaneously active threads
     * may exist to complicate the determination of queues left permanently
     * locked by port failure.  It is only during re-initialization that a
     * single thread with access to port queues exists, the thread responsible
     * for the re-initialization itself.  Therefore, port queues are flushed
     * during port re-initialization instead of during port clean up mainly
     * because it is easier to meet these requirements and because it is just
     * safer and more straight forward to do so.
     *
     * Local port maintenance timer interval, CI PPD port polling burst size,
     * and CI PPD port polling contact frequency are all reset( Step 5 ) using
     * configuration variables.  This allows their values to change following
     * failure and re-initialization of a local port.  The values for CI PPD
     * port polling burst size and contact frequency actually change much more
     * frequently.  They are reset on a per local port basis following the
     * completion of each CI PPD polling sweep( by np_test_lpconn()).
     *
     * An error in Steps 7,9 or exhaustion of all consecutive port
     * initialization attempts permanently aborts port initialization.  This is
     * also the fate of ports marked broken.
     *
     * Errors in Steps 10-12 abort the current port initialization attempt.  A
     * consecutive port initialization attempt is scheduled and the PCCB is
     * unlocked.
     *
     * Retrieval of the maximum addressable port number( Step 16 ) is dependent
     * upon the format of the CI port parameter register.  There are currently
     * two possible formats easily distinguishable by means of the diagnostic
     * information field.  One format implements this field while for the other
     * it is always 0.  Failure to retrieve a valid maximum addressable port
     * number crashes the local port and shuts it down permanently.
     *
     * Failure to verify the microcode revision level( Step 17 ) results either
     * in logging of an appropriate warning message or crashing of the local
     * port.  The former occurs when the revision level is known, just out of
     * date.  The port is allowed to continue to function.  The latter occurs
     * when the revision level is both unknown and does NOT exceed the known
     * maximum level for the hardware port type.  The local port is permanently
     * shut down.  No action is taken when an unknown level is found to exceed
     * the known maximum revision level for the hardware port type.  It is just
     * assumed that the port driver has not yet been updated to recognize a new
     * maximum microcode revision level.
     *
     * Permanent abortion of port initialization proceeds as follows:
     *
     * 1. The port is left disabled and unmapped.
     * 2. All associated resources are deallocated.
     * 3. The local port is marked broken.
     * 4. A console message is printed.
     * 5. The PCCB is removed from the system-wide local port database.
     * 6. The PCCB is unlocked.
     * 7. The PCCB is deallocated( only when the initial port initialization
     *    attempts fail ).
     *
     * The local port is marked broken( Step 3 ) only when it was previously
     * NOT marked broken.  It is also unmapped at this time.  This situation
     * exists only when all consecutive port initialization attempts have been
     * exhausted.
     *
     * The PCCB is deallocated( Step 7 ) only when the initial port
     * initialization attempts fails.  The SCA database is always locked prior
     * to PCCB removal and unlocked following it.  In order to preserve the SCA
     * locking hierarchy the PCCB lock is first released and then re-obtained
     * after the SCA database is locked.  Cached PCCB addresses remain valid at
     * this point because PCCBs are only deleted by this routine and such
     * action has not yet occurred.
     * 
     * NOTE: Broken local ports are unmapped at the time of their disablement.
     *	     This is not the case for those local ports which are to be
     *	     shutdown because they have exhausted their consecutive
     *	     initialization attempts.  Such ports are explicitly unmapped by
     *	     this routine during shutdown.
     */
    Lock_pccb( pccb )
    Pccb_fork_done( pccb, PANIC_PCCBFB )
    pccb->Fsmstatus.cleanup = 0;
    if( !pccb->Fsmstatus.broken && !pccb->Lpstatus.power ) {
	Unlock_pccb( pccb )
        ( void )splx( save_ipl );
	return;
    } else if( !pccb->Lpstatus.init     &&
	       !pccb->Fsmstatus.broken  &&
	       pccb->Reinit_tries == np_max_reinits ) {
        ++pccb->lpinfo.nreinits;
	{
	HDR	        *npbhp;
	npbq       	*carhp;
	long		i = ADAP_QUE_STP + CHNL_QUE_STP;

	while(( npbhp = ( HDR * )pccb->Dfreeq.flink ) != 
	      ( HDR * )&pccb->Dfreeq.flink ) {
            Remove_entry( npbhp->flink )
            SCA_KM_FREE(( char * )npbhp, npbhp->size, KM_SCABUF )
	}
	while(( npbhp = ( HDR * )pccb->Mfreeq.flink ) != 
	      ( HDR * )&pccb->Mfreeq.flink ) {
            Remove_entry( npbhp->flink )
            SCA_KM_FREE(( char * )npbhp, npbhp->size, KM_SCABUF )
	}
	while(( npbhp = ( HDR * )pccb->Binuseq.flink ) != 
	      ( HDR * )&pccb->Binuseq.flink ) {
            Remove_entry( npbhp->flink )
            SCA_KM_FREE(( char * )npbhp, npbhp->size, KM_SCABUF )
	}
	while(( carhp = ( npbq * )pccb->Cinuseq.flink ) != 
	      ( npbq * )&pccb->Cinuseq.flink ) {
            Remove_entry( carhp->flink )
	    if( i-- > 0 ) {
    	        Insert_entry( carhp->flink, pccb->Cfreeq )
	    } else {
                SCA_KM_FREE(( char * )carhp,((CARRIERH *)carhp)->Chdr_size, KM_SCABUF )
	    }
	}
        Init_adap_ques( pccb )
        Init_chnl_ques( pccb, pccb->C_idx )
	}
	pccb->Keepalive = ( u_int )np_maint_intrvl;
	pccb->Burst = np_cippdburst;		/* poll all the nodes first */
	pccb->Contact = np_cippdcontact;
	pccb->Lpstatus.connectivity = 0;
	*( u_char * )&pccb->Lbstatus = 0;
    }
    if( !pccb->Fsmstatus.broken					      &&
	 ( status = np_setup_port( pccb )) == RET_SUCCESS	      &&
	 ( pccb->Load_ucode == NULL 	||
	   ( status = ( *pccb->Load_ucode )( pccb )) == RET_SUCCESS ) &&
	 ( status = ( *pccb->Start_port )( pccb )) == RET_SUCCESS ) {
	status = 0;
	if( pccb->Lpstatus.init ) {

	    pccb->Lpstatus.init = 0;
	    pccb->Devattn.cirevlev.ci_romlev = pccb->Rom_level;
	    pccb->Devattn.cirevlev.ci_ramlev = pccb->Fn_level;
	    ( void )np_log_dev_attn( pccb, I_LPORT_INIT, LOG_NOREGS );
	    if( ci_first_port == 0 ) {
		ci_first_port = Scaaddr_low( pccb->lpinfo.addr );
	    }    
	    Compute_lbcrc( pccb )
	    switch( Cluster_size( pccb )) {
		    case 0: pccb->lpinfo.Max_port = CSZ_16 - 1; break;
		    case 1: pccb->lpinfo.Max_port = CSZ_32 - 1; break;
		    case 2: pccb->lpinfo.Max_port = CSZ_64 - 1; break;
		    case 3: pccb->lpinfo.Max_port = CSZ_128 - 1; break;
		    case 5: pccb->lpinfo.Max_port = CSZ_8 - 1; break;
		    default: pccb->lpinfo.Max_port = 0; break;
	    }
	    if( pccb->lpinfo.Max_port ) {
		if( pccb->lpinfo.Max_port > CIPPD_MAXPATHS ) {
		    pccb->lpinfo.Max_port = CIPPD_MAXPATHS;
		}
		for( mrltab = pccb->Mrltab; mrltab->ram; mrltab++ ) {
		    if( mrltab->ram == pccb->Fn_level &&
			 mrltab->rom == pccb->Rom_level ) {
			if( mrltab->warn == UCODE_WARNING ) {
			    ( void )np_log_dev_attn( pccb,
						     W_UCODE_WARN,
						     LOG_NOREGS );
			}
			break;
		    }
		}
		if( mrltab->ram == 0 && pccb->Fn_level <= pccb->Max_fn_level ) {
/* Commented OUT for DEBUGGING 
		     status = FE_BADUCODE;
*/
		}
	    } else {
		status = FE_BADMAXPORT;
	    }
	    if( status ) {
		pccb->Fsmstatus.broken = 1;
		( void )np_crash_lport( pccb, status, NULL );
	    }
	} else {
	    ( void )np_log_dev_attn( pccb, I_LPORT_REINIT, LOG_NOREGS );
	}
	if( status == 0 && np_maint_timer ) {
	    ( void )np_notify_port( pccb );
	}
	Unlock_pccb( pccb )
	if( status == 0 ) {
	    ( void )cippd_start( pccb );
	}
    } else if( !pccb->Fsmstatus.broken && --pccb->Reinit_tries ) {
	Pccb_fork( pccb, np_init_port, PANIC_PCCBFB )
	Unlock_pccb( pccb )
    } else {
	{
	CARRIERH       *carhp;
	HDR            *npbhp;

	while(( npbhp = ( HDR * )pccb->Dfreeq.flink ) != 
	      ( HDR * )&pccb->Dfreeq.flink ) {
            Remove_entry((( npbq * )npbhp )->flink )
            SCA_KM_FREE(( char * )npbhp, npbhp->size, KM_SCABUF )
	}
	while(( npbhp = ( HDR * )pccb->Mfreeq.flink ) != 
	      ( HDR * )&pccb->Mfreeq.flink ) {
            Remove_entry((( npbq * )npbhp )->flink )
            SCA_KM_FREE(( char * )npbhp, npbhp->size, KM_SCABUF )
	}
	while(( npbhp = ( HDR * )pccb->Binuseq.flink ) != 
	      ( HDR * )&pccb->Binuseq.flink ) {
            Remove_entry((( npbq * )npbhp )->flink )
            SCA_KM_FREE(( char * )npbhp, npbhp->size, KM_SCABUF )
	}
	while(( carhp = ( CARRIERH * )pccb->Cinuseq.flink ) != 
	      ( CARRIERH * )&pccb->Cinuseq.flink ) {
            Remove_entry((( npbq * )carhp )->flink )
            SCA_KM_FREE(( char * )carhp, carhp->Chdr_size, KM_SCABUF )
	}
	while(( carhp = ( CARRIERH * )pccb->Cfreeq.flink ) != 
	      ( CARRIERH * )&pccb->Cfreeq.flink ) {
            Remove_entry((( npbq * )carhp )->flink )
            SCA_KM_FREE(( char * )carhp, carhp->Chdr_size, KM_SCABUF )
	}
	}
	if( !pccb->Fsmstatus.broken ) {
	    pccb->Fsmstatus.broken = 1;
	    ( void )np_unmap_port( pccb );
	}
	Unlock_pccb( pccb )
	Lock_scadb()
	Lock_pccb( pccb )
	Remove_entry( pccb->flink )
	Unlock_pccb( pccb )
	Unlock_scadb()
	if( pccb->Lpstatus.init ) {
	    ( void )printf( "%4s\t- permanently offline( local port ? )\n",
			    ( u_char * )&pccb->lpinfo.name );
	    pccb->Npadap->pccb[pccb->C_idx] = NULL;
	    pccb->Ciisr->pccb = NULL;
	    SCA_KM_FREE(( char * )pccb, sizeof( PCCB ), KM_SCA )
	} else {
	    printf( "%4s\t- permanently shutting down( local port %u )\n",
		    ( u_char * )&pccb->lpinfo.name,
		    Scaaddr_low( pccb->lpinfo.addr ));
	}
    }
    ( void )splx( save_ipl );
}

/*   Name:	np_probe	- Probe a Local CI Port
 *
 *   Abstract:	This routine probes a newly discovered CI port culminating in
 *		its first initialization.  Any encountered errors abort the CI
 *		port probing.
 *
 *		The following CI hardware port types are currently supported
 *		under N_PORT:
 *
 *		CIXCD, CITCA
 *
 *   Inputs:
 *
 *   0				- Interrupt processor level
 *   SBR			- System Page Table Address processor register
 *   SLR			- System Page Table Length processor register
 *   np_adap			- Vector of CI Adapter Interface Blocks
 *   np_cippdburst		- CI PPD port polling burst size
 *   np_cippdcontact		- CI PPD port polling contact frequency
 *   np_maint_intrvl		- CI port maintenance timer interval
 *   np_max_reinits		- CI max number consecutive reinitializations
 *   np_pdt			- CI Port Dispatch Table
 *   np_ucode_type		- CI functional microcode type
 *   cinum			- CI adapter number
 *   cixcd_mrltable		- CIXCD Functional Microcode Revision Table
 *   cixcd_regoff		- CIXCD register offsets
 *   np_bddb			- Generic Vaxport Buffer Descriptor Database
 *   np_max_bds		- Size of system-wide buffer descriptor table
 *   hpt			- Hardware port type
 *   interconnect		- Interconnect type
 *   ioaddr			- Adapter I/O space virtual address
 *   scs_dg_size		- Maximum application datagram size
 *   scs_lport_db		- System-wide local port database queue head
 *   scs_msg_size		- Maximum application message size
 *   Sysmap			- System Page Table
 *
 *   Outputs:
 *
 *   0				- Interrupt processor level
 *   np_adap			- Vector of CI Adapter Interface Blocks
 *   np_ucode_type		- CI functional microcode type
 *   pccb			- Port Command and Control Block pointer
 *				   ( INITIALIZED as required )
 *   scs_lport_db		- System-wide local port database queue
 *
 *   SMP:	No locks are required even though the PCCB and SCA database are
 *		manipulated.  This function is only called during system
 *		initialization and at that time only the processor executing
 *		this code is operational.  This guarantees uncompromised access
 *		to all data structures without locking the PCCB or the SCA
 *		database.
 *
 *		PCCB lock structure is initialized.
 */
#ifdef __alpha
PCCB *
np_probe( cinum, ioaddr, interconnect, hpt, chnlnum, npadap, bus )
    u_int			cinum;
    volatile struct xmi_reg	*ioaddr;	/* adapter I/O space VA */
    u_int			interconnect;
    u_int			hpt, chnlnum;
    NPADAP			*npadap;
    struct bus			*bus;
{
    PCCB		*pccb;
    u_int		status = 0;
    static u_int	initialized = 0;
    u_int		save_ipl = Splscs();
    u_long		**regoffptr;

# else
PCCB *
np_probe( cinum, ioaddr, interconnect, hpt, chnlnum, npadap, bus )
    u_int		cinum;
    u_char		*ioaddr;
    u_int		interconnect;
    u_int		hpt, chnlnum;
    NPADAP		*npadap;
    struct bus		*bus;
{
    PCCB		*pccb;
    u_long		status = 0;
    static u_long	initialized = 0;
    u_long		save_ipl = Splscs();
    u_long		**regoffptr;
#endif

    /* The steps involved in probing a CI port are as follows:
     *
     *  1. IPL is synchronized to IPL_SCS.
     *  2. SCS is initialized.
     *  3. The N_PORT Buffer discriptor table is initialized.
     *  5. The CI functional microcode type is ascertained.
     *  6. A PCCB is allocated.
     *  8. The PCCB is initialized.
     *  9. The CI port is disabled.
     * 10. The PCCB is inserted into the system-wide local port database.
     * 11. Initialization of the CI port is scheduled through forking.
     * 12. IPL is restored.
     *
     * Steps 2-5 constitute CI port driver initialization and are only executed
     * during probing of the very first local CI port encountered during device
     * auto-configuration.  The remaining steps constitute the actual probing
     * of the specified local CI port and are executed whenever this routine is
     * invoked.  Any errors( Steps 2-7 ) encountered during either portion of
     * this routine immediately abort probing of the current local CI port
     * following logging of the fatal event.  The interrupt service routine for
     * the local CI port is also switched to the np_unmapped_isr(), the routine
     * responsible for handling interrupts on unmapped local CI ports.  Any
     * subsequent interrupts on the inoperative local port are discarded.
     *
     * NOTE: Certain CI hardware port types( CIBCA-BA, CIXCD ) do not require
     *	     functional microcode to be loaded.  All needed microcode is
     *	     onboard.  Lack of onboard functional microcode in CI ports of
     *	     these hardware types is regarded as a fatal error.  The event is
     *	     logged and probing of the local CI port is aborted.
     *
     * NOTE: All PCCB pointers to adapter registers which are not used by a
     *	     local port because of its hardware port type are zeroed.  This
     *	     includes:
     *
     *		1. Pointers to the maintenance address and maintenance data
     *		   port registers( not required by CIBCA-BA and CIXCD local
     *		   ports because they possess onboard functional microcode ).
     *		2. Pointers to the configuration register( not required by
     *		   CIBCA and CIXCD local ports because they do not define it ).
     *
     * NOTE: The PCCB fork block is only used for clean up and initialization
     *	     of the local port.  Therefore, it should always be available for
     *	     the port's first initialization.
     */
#ifdef CI_DEBUG 
    printf(">>> np_probe: entry\n");
#endif /* CI_DEBUG */
    if( initialized == 0 ) {
	if(( status = scs_initialize()) != RET_SUCCESS ||
	   ( status = np_initialize()) != RET_SUCCESS ) {
	    status = ( status == RET_ALLOCFAIL )
				? FE_INIT_NOMEM : FE_INIT_ZEROID;
	    ( void )np_log_initerr( cinum, interconnect, hpt, status );
	    ci_isr[ cinum ].isr = np_unmapped_isr;
	    ( void )splx( save_ipl );
	    return( NULL );
	} else {
	   status = 0;
	   initialized = 1;
	}
    }
    SCA_KM_ALLOC( pccb, PCCB *, sizeof( PCCB ), KM_SCA, KM_NOW_CL_CA )
#ifdef CI_DEBUG 
    printf("np_probe: pccb = %lx\n", pccb);
#endif /* CI_DEBUG */
    if( pccb ) {
	ci_isr[ cinum ].pccb = pccb;
	pccb->Npadap = npadap;
	pccb->Ciisr = &ci_isr[ cinum ];
    } else {
#ifdef CI_DEBUG 
	printf("np_init: failed to allocate pccb\n");
#endif /* CI_DEBUG */
	( void )np_log_initerr( cinum, interconnect, hpt, FE_INIT_NOMEM );
	ci_isr[ cinum ].isr = np_unmapped_isr;
	( void )splx( save_ipl );
	return( NULL );
    }
    switch(( pccb->Interconnect = interconnect )) {
	case ICT_XMI: {
#ifdef __alpha
	    volatile struct xmi_reg	*xmireg = ioaddr;
#else
	    struct xmi_reg	*xmireg = ( struct xmi_reg * )ioaddr;
#endif
	    CARRIERH		*carhp;
	    CARRIER		*carp;
	    int			i;

	    switch( hpt ) {

		case HPT_CIMNA:
	    	    pccb->Mna_dev = ( u_long * )&xmireg->xmi_dtype;
	    	    pccb->Mna_ber = ( u_long * )&xmireg->xmi_xbe;
	    	    pccb->Mna_fadrl = ( u_long * )&xmireg->xmi_fadr;
	    	    pccb->Mna_fadrh = 
				( u_long * )(( u_long )xmireg + CIMNA_XFAER);
	    	    pccb->Mna_aidr = 
				( u_long * )(( u_long )xmireg + CIMNA_AIDR );
	    	    pccb->Mna_amivr = 
				( u_long * )(( u_long )xmireg + CIMNA_AMIVR );
	    	    pccb->Mna_acivr = 
				( u_long * )(( u_long )xmireg + CIMNA_ACIVR );
	    	    pccb->Mna_asnr = 
				( u_long * )(( u_long )xmireg + CIMNA_ASNR );
	    	    pccb->Mna_pdcsr = 
				( u_long * )(( u_long )xmireg + CIMNA_PDCSR );
	    	    pccb->Mna_asubr = 
				( u_long * )(( u_long )xmireg + CIMNA_ASUBR );

		    regoffptr = cimna_regoff;
		    pccb->Lpstatus.onboard = 1;
		    pccb->Dg_cache = CI_DG_CACHE;
		    pccb->Msg_cache = CI_MSG_CACHE;
		    pccb->Start_port = np_mna_start;
		    pccb->Disable_port = npsc_disable;
		    pccb->Max_fn_level = CIMNA_MAXFN;
		    pccb->Max_rom_level = CIMNA_MAXST;
		    pccb->Mrltab = cimna_mrltable;
		    pccb->Load_ucode = NULL;

    		    Init_queue( pccb->Cfreeq )
		    for( i = ADAP_QUE_STP + CHNL_QUE_STP; i > 0; --i ) {
			Alloc_np_carrier( carhp, carp, pccb )
			if( carhp == NULL ) {
		    	    status = FE_INIT_NOMEM;
			    break;
			}
    	        	Insert_entry((( HDR * )carhp )->flink, pccb->Cfreeq )
		    }
		    break;

		default:
		    status = FE_INIT_UNKHPT;
	    }
	    break;
	}
	case ICT_TC: {
	    struct _tca_reg	*tcreg = ( struct _tca_reg * )ioaddr;
	    CARRIERH		*carhp;
	    CARRIER		*carp;
	    int			i;

	    switch( hpt ) {

		case HPT_CITCA:
	    	    pccb->Tca_dev = ( u_long * )&tcreg->tca_dtype;
	    	    pccb->Tca_ber = ( u_long * )&tcreg->tca_ber;

	    	    pccb->Tca_pdcsr = 
				( u_long * )(( u_long )tcreg + CITCA_PDCSR );
	    	    pccb->Tca_asnr = 
				( u_long * )(( u_long )tcreg + CITCA_ASNR );

		    regoffptr = citca_regoff;
		    pccb->Lpstatus.onboard = 1;
		    pccb->Dg_cache = CI_DG_CACHE;
		    pccb->Msg_cache = CI_MSG_CACHE;
		    pccb->Start_port = npsc_start;
		    pccb->Disable_port = npsc_disable;
		    pccb->Max_fn_level = CITCA_MAXFN;
		    pccb->Max_rom_level = CITCA_MAXST;
		    pccb->Mrltab = citca_mrltable;
		    pccb->Load_ucode = NULL;

    		    Init_queue( pccb->Cfreeq )
    		    for( i = ADAP_QUE_STP + CHNL_QUE_STP; i > 0; --i ) {
			Alloc_np_carrier( carhp, carp, pccb )
			if( carhp == NULL ) {
    	    		    status = FE_INIT_NOMEM;
	    		    break;
			}
    	        	Insert_entry((( HDR * )carhp )->flink, pccb->Cfreeq )
    		    }
	    	    break;

		default:
		    status = FE_INIT_UNKHPT;
	    }
	    break;
        }
	default:
	    ( void )panic( PANIC_IC );
    }
    if( status ) {
	SCA_KM_FREE(( char * )pccb, sizeof( PCCB ), KM_SCA )
	( void )np_log_initerr( cinum, interconnect, hpt, status );
	ci_isr[ cinum ].pccb = NULL;
	ci_isr[ cinum ].isr = np_unmapped_isr;
	( void )splx( save_ipl );
	return( NULL );
    }
    pccb->size = sizeof( PCCB );
    pccb->type = DYN_PCCB;
    pccb->pdt = &np_pdt;
    Init_pccb_lock( pccb )
    pccb->lpinfo.type.hwtype = hpt;
    pccb->lpinfo.type.swtype = SPT_CI;
    pccb->lpinfo.type.ictype = interconnect;
    pccb->lpinfo.type.dual_path = 1;
    pccb->lpinfo.name = Ctrl_from_num( "ci  ", cinum );
    {
    u_long	**regptr, **end;
    for( regptr = ( u_long ** )&pccb->Npregptrs,
	 end = ( u_long ** )( &pccb->Npregptrs + 1 );
	 regptr != end;
	 ++regptr, ++regoffptr ) {
	*regptr = ( u_long * )(( u_long )ioaddr + U_long( *regoffptr ));
    }
    }
    pccb->Bus = bus;	/* put ptr to bus structure in PCCB for Mbox calls */
    switch( pccb->lpinfo.type.hwtype ) {

	case HPT_CITCA:
	case HPT_CIMNA:
            pccb->lpinfo.flags.expl = 1;
            pccb->lpinfo.Ovhd_pd = Nph_size_exp;
	    if(( pccb->Pgsize = NBPG ) < NPORT_PGSIZE ) {
	        pccb->Drv_param |= SHORT_DATA_PAGE;
		pccb->Typ0_max = TYP0_4K_MAX;
		pccb->Typ1_max = TYP1_4K_MAX;
	    } else {
		pccb->Pgsize = NPORT_PGSIZE;
		pccb->Typ0_max = TYP0_8K_MAX;
		pccb->Typ1_max = TYP1_8K_MAX;
	    }
	    break;

	default:
	    ( void )panic( PANIC_HPT );
    }
    pccb->Lpstatus.init = 1;
    pccb->Lpstatus.power = 1;
    pccb->Lpstatus.mapped = 1;
    pccb->Reinit_tries = np_max_reinits;
    pccb->Pkt_size = ( pccb->lpinfo.Ovhd_pd + Nphdr_size + sizeof( SETCKTH ));
    {
    u_char	*c;
    int		n;

    for( c = &pccb->Lbdata[ 0 ], n = 0; n < LBDSIZE; n++, c++ ) {
	*c = ( u_char )n;
    }
    }
    pccb->C_idx = chnlnum;
    pccb->Npadap->pccb[chnlnum] = pccb;
    pccb->Dqe_len = pccb->lpinfo.Ovhd_pd + Nphdr_size +
			sizeof( NPPPDH ) +
			sizeof( SCSH ) +
			scs_dg_size;
    pccb->Mqe_len = pccb->lpinfo.Ovhd_pd + Nphdr_size +
			sizeof( NPPPDH ) +
			sizeof( SCSH ) +
			scs_msg_size;
    pccb->Keepalive = ( u_int )np_maint_intrvl;
    svatophys( np_bddb->bdlt, &pccb->Bdlt_base );
    pccb->Bdlt_len = sizeof( BDLT );
    pccb->Intr_holdoff = ( u_int )np_intr_holdoff;
    pccb->Dccq0 = &pccb->Chnl_cmdq[pccb->C_idx].dccq0;
    pccb->Dccq1 = &pccb->Chnl_cmdq[pccb->C_idx].dccq1;
    pccb->Dccq2 = &pccb->Chnl_cmdq[pccb->C_idx].dccq2;

    Init_queue( pccb->Cinuseq )
    Init_queue( pccb->Binuseq )
    Init_queue( pccb->Dfreeq )
    Init_queue( pccb->Mfreeq )
    Init_adap_ques( pccb )
    Init_chnl_ques( pccb, chnlnum )
    Init_queue( pccb->Form_pb )
    pccb->Burst = cippd_max_port;		/* poll all nodes first */
    pccb->Contact = np_cippdcontact;
    pccb->Fsmstatus.nosanity_chk = np_nosanity_chk;
    pccb->Max_cables = MAX_CABLES;
    pccb->lpinfo.Dg_size = pccb->Dqe_len;
    pccb->lpinfo.Msg_size = pccb->Mqe_len;
    pccb->lpinfo.Ovhd =  pccb->lpinfo.Ovhd_pd + sizeof( NPPPDH );
    pccb->lpinfo.Protocol = CIPPD_VERSION;
    ( void )( *pccb->Disable_port )( pccb, PS_UNINIT );
    Insert_entry( pccb->flink, scs_lport_db );
    Pccb_fork( pccb, np_init_port, PANIC_PCCBFB )
    ( void )splx( save_ipl );
#ifdef CI_DEBUG 
	printf("np_probe: exit\n");
#endif /* CI_DEBUG */
    return( pccb );
}

/*   Name:	np_setup_port	- Prepare Local CI Port for Initialization
 *
 *   Abstract:	This function prepares local CI ports for initialization and is
 *		invoked only during local port initialization.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   SID			- System Identification processor register
 *   cpu			- CPU type code
 *   lscs			- Local system permanent information
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *          lpstatus.mapped     -   1
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   0
 *	    fsmstatus.online	-   0
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pinter
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    devattn.cicpurevlev	-   Out-of-revision CPU microcode information
 *	    dfreeq		-   Port datagram free queue
 *	    mfreeq		-   Port message free queue
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   Port is broken status flag
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Local port successfully prepared
 *   RET_FAILURE		- CI adapter is absent & can't be initialized
 *   				- Insufficient memory for port initialization
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) allowing unrestricted access.
 *		This is probably unnecessary because of lack of conflict for
 *		the PCCB due to the single threadedness of port clean up and
 *		initialization.  It is done anyway to guarantee unrestricted
 *		access and because the CI PPD interval timer may still be
 *		operational.
 *
 *		Access to port free queues is by means of memory interlocking
 *		queuing instructions.
 */
u_long
np_setup_port( pccb )
    PCCB		*pccb;
{
    u_long		n, pfn;
    SCSH		*scsbp;
    CARRIER		*carp;
    u_long		status = 0;
    u_long		log_regs;

    /* The steps involved in preparing a CI port for initialization are as
     * follows:
     *
     * 1. Presence of a local operational port is verified.
     * 2. An initial number of free datagrams and messages, sufficient to fill
     *	  the port's internal cache plus 1, are allocated and inserted into the
     *    appropriate local port queues.
     *
     * Preparations for local port initialization are aborted on detection of
     * any errors.  Errors in Steps 1,3 permanently abort port initialization.
     * The fatal event is logged and the local port is taken permanently
     * offline by marking it broken and unmapping it.  Errors in Step 4 abort
     * only the current consecutive port initialization attempt.  The event is
     * logged and all local port free queues are flushed with all flushed
     * buffers being deallocated.
     *
     * Allocation of messages and datagrams and their insertion into
     * appropriate local port free queues( Step 4 ) is bypassed whenever such
     * buffers had been previously allocated.  This can only occur when
     * preparation of the local port during a previous consecutive
     * initialization succeeded although all subsequent attempts at starting of
     * the port itself failed.
     *
     * NOTE: The port is not informed of the presence of free datagram and
     *	     message buffers even though the queues have been previously empty.
     *	     The port is disabled and therefore is in no condition to take
     *	     notice.  The port is notified of free queue entry availability
     *	     only after its successful enabling.
     */
#ifdef CI_DEBUG
    printf("np_setup_port: entered\n");
#endif /* CI_DEBUG */
    if( np_test_port( pccb ) != RET_SUCCESS ) {
        status = FE_NOCI;
        log_regs = LOG_REGS;
        pccb->Fsmstatus.broken = 1;
    } else {
        svatophys( pccb->Dadfq.T_ptr, &pfn );
        if(( caddr_t )pfn == pccb->Dadfq.H_ptr ) {
        for( n = pccb->Dg_cache + 1; n; --n ) {
            if(( scsbp = np_alloc_dg( pccb )) == NULL ) {
                status = E_NOMEM;
                log_regs = LOG_NOREGS;
                break;
           }
            Insert_entry((( npbq * )Npbp_to_nphd( Scs_to_pd( scsbp, pccb )))->flink, pccb->Dfreeq )
            Insqt( pccb, Scs_to_pd( scsbp, pccb ), &pccb->Dadfq, carp )
        }
        if( status == 0 ) {
            for( n = pccb->Msg_cache + 1; n; --n ) {
                if(( scsbp = np_alloc_msg( pccb )) == NULL ) {
                    status = E_NOMEM;
                    log_regs = LOG_NOREGS;
                    break;
                }
                Insert_entry((( npbq * )Npbp_to_nphd( Scs_to_pd( scsbp, pccb )))->flink, pccb->Mfreeq )
                Insqt( pccb, Scs_to_pd( scsbp, pccb ), &pccb->Damfq, carp )
            }
        }
        if( status ) {
            {
            HDR             *npbhp;
            npbq            *carhp, *next;

            svatophys( pccb->Dadfq.T_ptr, &pccb->Dadfq.H_ptr );
            svatophys( pccb->Damfq.T_ptr, &pccb->Damfq.H_ptr );
            while(( npbhp = ( HDR * )pccb->Dfreeq.flink ) !=
                  ( HDR * )&pccb->Dfreeq.flink ) {
                Remove_entry( npbhp->flink )
                SCA_KM_FREE(( char * )npbhp, npbhp->size, KM_SCABUF )
            }
            while(( npbhp = ( HDR * )pccb->Mfreeq.flink ) !=
                  ( HDR * )&pccb->Mfreeq.flink ) {
                Remove_entry( npbhp->flink )
                SCA_KM_FREE(( char * )npbhp, npbhp->size, KM_SCABUF )
            }
            for( carhp = ( npbq * )pccb->Cinuseq.flink;
                 carhp != ( npbq * )&pccb->Cinuseq.flink;
                 carhp = next ) {
                next = carhp->flink;
                carp = Carhd_to_car( carhp );
                if( carp->Cbuftoken != 0 ) {
                    Remove_entry( carhp->flink )
                    SCA_KM_FREE(( char * )carhp,((CARRIERH *)carhp)->Chdr_size,
                                  KM_SCABUF )
                }
            }
            }
        }
        }
    }

    if( status == 0 ) {
        status = RET_SUCCESS;
    } else {
        ( void )np_log_dev_attn( pccb, status, log_regs );
        if( pccb->Fsmstatus.broken ) {
            ( void )np_unmap_port( pccb );
        }
        status = RET_FAILURE;
    }
    return( status );
}

/*   Name:      np_test_port    - Test Operational Status of Local CI Port
 *
 *   Abstract:  This function checks for the presence of a local CI port and
 *              clears all bus specific errors which require clearing if the
 *              port is both present and operational.  It is invoked only
 *              during local port initialization.
 *
 *              CITCA/CIMNA local ports are always present though they
 *              may be broken.
 *
 *   Inputs:
 *
 *   IPL_SCS                    - Interrupt processor level
 *   cpu                        - CPU type code
 *   cpusw                      - CPU switch structure
 *   pccb                       - Port Command and Control Block pointer
 *      pd.gvp.type.ci          -  CI specific PCCB fields
 *          lpstatus.mapped     -   1
 *      ppd.cippd               -  CI PPD specific PCCB fields
 *          fsmstatus.broken    -   0
 *          fsmstatus.online    -   0
 *
 *   Outputs:
 *
 *   IPL_SCS                    - Interrupt processor level
 *   pccb                       - Port Command and Control Block pointer
 *      pd.gvp.type.ci          -  CI specific PCCB fields
 *          ciregptrs.cnfr      -   Configuration register pointer
 *
 *   Return Values:
 *
 *   RET_SUCCESS                - Local port present and accessible
 *   RET_FAILURE                - Local port absent
 *
 *   SMP:       No locks are required.  PCCB addresses are always valid
 *              allowing access to static fields because these data structures
 *              are never deleted once their corresponding ports have been
 *              initialized( The PCCB is locked EXTERNALLY anyway ).
 */
u_long
np_test_port( pccb )
    PCCB                *pccb;
{
    u_long              status = RET_SUCCESS;

    /* The local CI port is declared absent in the following cases:
     *
     * 1. CITCA/CIMNA failed its self-test.
     * 2. CITCA/CIMNA bus specific device type register is unaccessible.
     * 3. Verification of hardware port type fails.
     *
     * NOTE: There is never a need to clear errors in the BIIC/XMI error
     *       register because resetting of the node during port disablement
     *       accomplishes that task.
     */
#ifdef CI_DEBUG
    printf("np_test_port: entered\n");
#endif /* CI_DEBUG */
    switch( pccb->lpinfo.type.hwtype ) {

	case HPT_CIMNA:
#ifdef __alpha
	    if( Bad_reg( pccb->Mna_dev )	  ||
		((RDCSR(LONG_32,pccb->Bus,pccb->Mna_ber)) & XMI_STF) ||
		(( (RDCSR(LONG_32,pccb->Bus,pccb->Mna_dev)) & 
			XMIDTYPE_TYPE ) != XMI_CIMNA )) {
		status = RET_FAILURE;
		}

#else
	    if( Bad_reg( pccb->Mna_dev )	  ||
		 ( *pccb->Mna_ber & XMI_STF ) ||
		 (( *pccb->Mna_dev & XMIDTYPE_TYPE ) != XMI_CIMNA )) {
		status = RET_FAILURE;
		}
#endif
	    break;

	case HPT_CITCA:
	    if( Bad_reg( pccb->Tca_dev )	  ||
		 ( *pccb->Tca_ber & CITCA_TC_STF ) ||
		 (( *pccb->Tca_dev & TC_DTYPE ) != TC_CITCA )) {
		status = RET_FAILURE;
	    }
	    break;

	default:
	    ( void )panic( PANIC_HPT );
    }
    return( status );
}

/*   Name:	npsc_start	- Start a Local N_PORT Single Chnl Family Port
 *
 *   Abstract:	This function starts a CITCA/CIMNA port during local
 *		port initialization.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *          lpstatus.mapped     -   1
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   0
 *	    fsmstatus.online	-   0
 *   Sysmap			- System Page Table
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    ciregptrs.pdfqcr	-   Port dg free queue control register pointer
 *	    ciregptrs.pecr	-   Port enable control register pointer
 *	    ciregptrs.picr	-   Port initialization control reg pointer
 *	    ciregptrs.pmcsr	-   Port maintenance cntl & status reg pointer
 *	    ciregptrs.pmfqcr	-   Port message free queue control reg pointer
 *	    ciregptrs.pqbbase	-   PQB base register pointer
 *	    ciregptrs.psrcr	-   Port status release control reg pointer
 *	    ram_level		-   Functional microcode revision level
 *	    rom_level		-   Self-test microcode revision level
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Local port successfully started
 *   RET_FAILURE		- CI port microcode did not pass verification
 *   				- CI port can not be started
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) to synchronize access.  This
 *		is probably unnecessary because of lack of conflict for the
 *		PCCB due to the single threadedness of port clean up and
 *		initialization.  It is done anyway to guarantee unrestricted
 *		access and because the CI PPD interval timer may still be
 *		active.
 */
u_long
npsc_start( pccb )
    PCCB		*pccb;
{
    u_long		event, status = RET_SUCCESS;
    
    /* The steps involved in starting a N_PORT single channel family port 
     *	are as follows:
     *
     * 1. Start the port micro-engine sequencing.
     * 2. Disable port interrupts and the port maintenance timer.
     * 3. Initialize the port by transitioned it into the disabled state.
     * 4. Retrieve port functional/self-test microcode revision levels( initial
     *	  initialization only ).
     * 5. Load port PQB physical address into the appropriate CI port register.
     * 6. Release the port status register.
     * 7. Enable interrupts on the port.
     * 8. Enable the port by transitioning it into the enabled state.
     * 9. Notify the port of free message and datagram buffer availability.
     *
     * Starting the port micro-engine sequencing( Step 1 ) clears all port
     * errors.  Unfortunately, it also enables both port interrupts and the
     * port maintenance timer requiring their explicit disablement( Step 2 ).
     *
     * Transitioning a CI port into the disabled state( Step 3 ) triggers a
     * port interrupt.  As port interrupts are currently disabled, the expected
     * interrupt must be manually checked for.  An error is declared if either
     * an interrupt is not detected within a fixed period of time or the port
     * fails to transition into the disabled state.  Such errors abort the
     * current consecutive attempt at port initialization.  The event is logged
     * and the port itself is disabled to flush out any loaded microcode and to
     * return the port to a known state.
     */
    u_int	amcsr, csr;
    u_long	abbr;
#ifdef __alpha
    amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
    WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,(amcsr | AMCSR_MIN));
#else
    *pccb->Amcsr |= AMCSR_MIN;
#endif

DELAY( 10000 );
/* This should be commented out until a new version of CIMNA ucode supports
   the UNIN bit in the CSR */
/*
    Adap_wait_unin();
    if( *pccb->Csr & CSR_UNIN ) {
*/
    	switch( pccb->lpinfo.type.hwtype ) {
	    case HPT_CITCA:
                *pccb->Amcsr &= ~CITCA_PMCS_IE;
	        break;
	    case HPT_CIMNA:
#ifdef __alpha
               amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
	       WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,(amcsr & ~CIMNA_PMCS_IE));
	       WRTCSR(LONG_32,pccb->Bus,pccb->Mna_aidr,pccb->Npadap->Mna_aid);
	       WRTCSR(LONG_32,pccb->Bus,pccb->Mna_amivr,pccb->Npadap->Mna_amiv);
	       WRTCSR(LONG_32,pccb->Bus,pccb->Mna_acivr,pccb->Npadap->Mna_aciv);
#else
                *pccb->Amcsr &= ~CIMNA_PMCS_IE;
		*pccb->Mna_aidr = pccb->Npadap->Mna_aid;
		*pccb->Mna_amivr = pccb->Npadap->Mna_amiv;
		*pccb->Mna_acivr = pccb->Npadap->Mna_aciv;
#endif
	    default:
	        ( void )panic( PANIC_HPT );
	}
#ifdef __alpha
    	amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
    	WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,(amcsr | AMCSR_STD));
	/* N_PORT spec says shift >> 5, but CITCA/CIMNA spec DON'T */
	/* svatophys will put the phys. addr. into abbr */
        svatophys( pccb->Npadap, &abbr );
	WRTCSR(LONG_32,pccb->Bus,pccb->Abbr,abbr);
	MB();
	WRTCSR(LONG_32,pccb->Bus,pccb->Cicr,CICR_CIC);
        Chnl_wait_pic();
	if( (RDCSR(LONG_32,pccb->Bus,pccb->Csr)) & CSR_CIC ) {
#else
        *pccb->Amcsr |= AMCSR_STD;
	/* N_PORT spec says shift >> 5, but CITCA/CIMNA spec DON'T */
        svatophys( pccb->Npadap, pccb->Abbr );
        *pccb->Cicr = CICR_CIC;
        Chnl_wait_pic();
        if( *pccb->Csr & CSR_CIC ) {
#endif
            switch( pccb->lpinfo.type.hwtype ) {
                case HPT_CITCA:
                    Clean_dcache_addr(& (pccb->Npadap->npab), 512, 0x1ff)
                    Clean_dcache_addr(& (pccb->Chnl_pb[pccb->C_idx]), 512, 0x1ff)
                    break;
                default:
                    break;
            }
	    if( pccb->Lpstatus.init ) {
		Scaaddr_low( pccb->lpinfo.addr ) = Imp_lport_addr( pccb );
    		if(( pccb->Pkt_mult = (( Aibuf_len( pccb )) >> 9 )) > 6 ) {
       		    pccb->Pkt_mult = 6;
    		}
		if( Aramp( pccb )) {
		
/* Read Ramp info in param block and alloc memory - Ampb
*/
		}
	        switch( pccb->lpinfo.type.hwtype ) {
		    case HPT_CIMNA:
                        pccb->Fn_level = Cimna_fn_level( pccb );
		        break;
		    case HPT_CITCA:
                        pccb->Fn_level = Citca_fn_level( pccb );
		        break;
		    default:
		        ( void )panic( PANIC_HPT );
	        }
	    }
	    if(( pccb->Pgsize < NPORT_PGSIZE ) && ( !Atyp_sdp( pccb ))) {
		event =  E_SHORT_DPAGE;
	        status = RET_FAILURE;
	    } else {
#ifdef __alpha
		WRTCSR(LONG_32,pccb->Bus,pccb->Csrcr,CSRCR_CSRC);
#else
	        *pccb->Csrcr = CSRCR_CSRC;
#endif
    		switch( pccb->lpinfo.type.hwtype ) {
		    case HPT_CITCA:
                	*pccb->Amcsr |= CITCA_PMCS_IE;
	    	        break;
		    case HPT_CIMNA:
#ifdef __alpha
    			amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
    			WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,
					(amcsr | CIMNA_PMCS_IE));
#else
                	*pccb->Amcsr |= CIMNA_PMCS_IE;
#endif
	    	        break;
		    default:
	    	        ( void )panic( PANIC_HPT );
		}
#ifdef __alpha
		WRTCSR(LONG_32,pccb->Bus,pccb->Cecr,CECR_CEC);
                Chnl_wait_pic_clear();
		csr = RDCSR(LONG_32,pccb->Bus,pccb->Csr);
		if (( csr & CSR_CEC ) && !( csr & CSR_CIC )) {
		    WRTCSR(LONG_32,pccb->Bus,pccb->Csrcr,CSRCR_CSRC);
	            Svatophy_shiftR5(pccb->Dadfq.T_ptr,pccb->Adfqir,pccb->Bus);
	            Svatophy_shiftR5(pccb->Damfq.T_ptr,pccb->Amfqir,pccb->Bus);

#else
	        *pccb->Cecr = CECR_CEC;
                Chnl_wait_pic_clear();
                if(( *pccb->Csr & CSR_CEC ) && !( *pccb->Csr & CSR_CIC )) {
	    	    *pccb->Csrcr = CSRCR_CSRC;
	            Svatophy_shiftR5( pccb->Dadfq.T_ptr, *pccb->Adfqir );
	            Svatophy_shiftR5( pccb->Damfq.T_ptr, *pccb->Amfqir );
#endif
                } else {
		    event = E_ENABLE_STATE;
	            status = RET_FAILURE;
                }
            }
        } else {
	    event = E_DISABLE_STATE;
	    status = RET_FAILURE;
        }
/*
    } else {
	event = E_UCODE_START;
	status = RET_FAILURE;
    }
*/
    if( status == RET_FAILURE ) {
	if( pccb->Reinit_tries <= 1 )  {
	    event = FE_UCODE_START;
	}
	( void )np_log_dev_attn( pccb, event, LOG_REGS );
	( void )( *pccb->Disable_port )( pccb, PS_UNINIT );
    }
    return( status );
}

/*   Name:	np_mna_start	- Start a Local N_PORT Single Chnl Family Port
 *
 *   Abstract:	This function starts a CITCA/CIMNA port during local
 *		port initialization.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *          lpstatus.mapped     -   1
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   0
 *	    fsmstatus.online	-   0
 *   Sysmap			- System Page Table
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    ciregptrs.pdfqcr	-   Port dg free queue control register pointer
 *	    ciregptrs.pecr	-   Port enable control register pointer
 *	    ciregptrs.picr	-   Port initialization control reg pointer
 *	    ciregptrs.pmcsr	-   Port maintenance cntl & status reg pointer
 *	    ciregptrs.pmfqcr	-   Port message free queue control reg pointer
 *	    ciregptrs.pqbbase	-   PQB base register pointer
 *	    ciregptrs.psrcr	-   Port status release control reg pointer
 *	    ram_level		-   Functional microcode revision level
 *	    rom_level		-   Self-test microcode revision level
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Local port successfully started
 *   RET_FAILURE		- CI port microcode did not pass verification
 *   				- CI port can not be started
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) to synchronize access.  This
 *		is probably unnecessary because of lack of conflict for the
 *		PCCB due to the single threadedness of port clean up and
 *		initialization.  It is done anyway to guarantee unrestricted
 *		access and because the CI PPD interval timer may still be
 *		active.
 */
#ifdef __alpha
u_long
np_mna_start( pccb )
    PCCB		*pccb;
{
    u_long		event, status = RET_SUCCESS;

    /* The steps involved in starting a N_PORT single channel family port 
     *	are as follows:
     *
     * 1. Start the port micro-engine sequencing.
     * 2. Disable port interrupts and the port maintenance timer.
     * 3. Initialize the port by transitioned it into the disabled state.
     * 4. Retrieve port functional/self-test microcode revision levels( initial
     *	  initialization only ).
     * 5. Load port PQB physical address into the appropriate CI port register.
     * 6. Release the port status register.
     * 7. Enable interrupts on the port.
     * 8. Enable the port by transitioning it into the enabled state.
     * 9. Notify the port of free message and datagram buffer availability.
     *
     * Starting the port micro-engine sequencing( Step 1 ) clears all port
     * errors.  Unfortunately, it also enables both port interrupts and the
     * port maintenance timer requiring their explicit disablement( Step 2 ).
     *
     * Transitioning a CI port into the disabled state( Step 3 ) triggers a
     * port interrupt.  As port interrupts are currently disabled, the expected
     * interrupt must be manually checked for.  An error is declared if either
     * an interrupt is not detected within a fixed period of time or the port
     * fails to transition into the disabled state.  Such errors abort the
     * current consecutive attempt at port initialization.  The event is logged
     * and the port itself is disabled to flush out any loaded microcode and to
     * return the port to a known state.
     */
    u_int       amcsr, csr;
    u_long	abbr;

#ifdef CI_DEBUG  
    printf("np_mna_start: entered\n");
#endif /* CI_DEBUG */
    amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
    WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,(amcsr | AMCSR_MIN));

DELAY( 1000 );
/* This should be commented out until a new version of CIMNA ucode supports
   the UNIN bit in the CSR */
/*
    Adap_wait_unin();
    if( *pccb->Csr & CSR_UNIN ) {
*/
        amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
        WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,(amcsr & ~CIMNA_PMCS_IE));
        WRTCSR(LONG_32,pccb->Bus,pccb->Mna_aidr,pccb->Npadap->Mna_aid);
        WRTCSR(LONG_32,pccb->Bus,pccb->Mna_amivr,pccb->Npadap->Mna_amiv);
        WRTCSR(LONG_32,pccb->Bus,pccb->Mna_acivr,pccb->Npadap->Mna_aciv);
        amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
        WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,(amcsr | AMCSR_STD));

	/* N_PORT spec says shift >> 5, but CITCA/CIMNA spec DON'T */
	/* svatophys will put the phys. addr. into abbr */
        svatophys( pccb->Npadap, &abbr );
	WRTCSR(LONG_32,pccb->Bus,pccb->Abbr,abbr);
	MB();
#ifdef CI_DEBUG
	printf("np_mna_start: pccb->Abbr = %lx\n", 
		RDCSR(LONG_32,pccb->Bus,pccb->Abbr));
#endif /* CI_DEBUG */
	WRTCSR(LONG_32,pccb->Bus,pccb->Cicr,CICR_CIC);
	MB();
	Chnl_wait_pic();
	if( (RDCSR(LONG_32,pccb->Bus,pccb->Csr)) & CSR_CIC ) {
#ifdef CI_DEBUG
            printf("np_mna_start: CIMNA in disabled state\n");
#endif /* CI_DEBUG */
	    if( pccb->Lpstatus.init ) {
		Scaaddr_low( pccb->lpinfo.addr ) = Imp_lport_addr( pccb );
    		if(( pccb->Pkt_mult = (( Aibuf_len( pccb )) >> 9 )) > 6 ) {
       		    pccb->Pkt_mult = 6;
    		}
		if( Aramp( pccb )) {
		
/* Read Ramp info in param block and alloc memory - Ampb
 * CITCA/CIMNA doesn't request memory.
 */
		}
                pccb->Fn_level = Cimna_fn_level( pccb );
	    }
            WRTCSR(LONG_32,pccb->Bus,pccb->Csrcr,CSRCR_CSRC);
	    MB();
	    amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
            WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,(amcsr|CIMNA_PMCS_IE));
	    MB();
#ifdef CI_DEBUG
            printf("np_mna_start: XMI interrupts enabled\n");
#endif /* CI_DEBUG */
	    WRTCSR(LONG_32,pccb->Bus,pccb->Cecr,CECR_CEC);
/* wait for IC bit to clear */
            Chnl_wait_pic_clear();
            csr = RDCSR(LONG_32,pccb->Bus,pccb->Csr);
            if (( csr & CSR_CEC ) && !( csr & CSR_CIC )) {
#ifdef CI_DEBUG
                    printf("np_mna_start: initialization complete; CIMNA in enabled state\n");
#endif /* CI_DEBUG */
                    WRTCSR(LONG_32,pccb->Bus,pccb->Csrcr,CSRCR_CSRC);
                    Svatophy_shiftR5(pccb->Dadfq.T_ptr,pccb->Adfqir,pccb->Bus);
                    Svatophy_shiftR5( pccb->Damfq.T_ptr,pccb->Amfqir,pccb->Bus);
            } else {
		    event = E_ENABLE_STATE;
	            status = RET_FAILURE;
            }
        } else {
	    event = E_DISABLE_STATE;
	    status = RET_FAILURE;
        }
/*
    } else {
	event = E_UCODE_START;
	status = RET_FAILURE;
    }
*/
    if( status == RET_FAILURE ) {
	if( pccb->Reinit_tries <= 1 )  {
	    event = FE_UCODE_START;
	}
	( void )np_log_dev_attn( pccb, event, LOG_REGS );
	( void )( *pccb->Disable_port )( pccb, PS_UNINIT );
    }
#ifdef CI_DEBUG
    printf("np_mna_start: exit; status = %x\n", status);
#endif /* CI_DEBUG */
    return( status );
}

#else
u_long
np_mna_start( pccb )
    PCCB                *pccb;
{
    u_long               event, status = RET_SUCCESS;

    /* The steps involved in starting a N_PORT single channel family port
     *  are as follows:
     *
     * 1. Start the port micro-engine sequencing.
     * 2. Disable port interrupts and the port maintenance timer.
     * 3. Initialize the port by transitioned it into the disabled state.
     * 4. Retrieve port functional/self-test microcode revision levels( initial
     *    initialization only ).
     * 5. Load port PQB physical address into the appropriate CI port register.
     * 6. Release the port status register.
     * 7. Enable interrupts on the port.
     * 8. Enable the port by transitioning it into the enabled state.
     * 9. Notify the port of free message and datagram buffer availability.
     *
     * Starting the port micro-engine sequencing( Step 1 ) clears all port
     * errors.  Unfortunately, it also enables both port interrupts and the
     * port maintenance timer requiring their explicit disablement( Step 2 ).
                                                                         *
     * Transitioning a CI port into the disabled state( Step 3 ) triggers a
     * port interrupt.  As port interrupts are currently disabled, the expected
     * interrupt must be manually checked for.  An error is declared if either
     * an interrupt is not detected within a fixed period of time or the port
     * fails to transition into the disabled state.  Such errors abort the
     * current consecutive attempt at port initialization.  The event is logged
     * and the port itself is disabled to flush out any loaded microcode and to
     * return the port to a known state.
     */
    *pccb->Amcsr |= AMCSR_MIN;
DELAY( 1000 );
/*
    Adap_wait_unin();
    if( *pccb->Csr & CSR_UNIN ) {
*/
        *pccb->Amcsr &= ~CIMNA_PMCS_IE;
        *pccb->Mna_aidr = pccb->Npadap->Mna_aid;
        *pccb->Mna_amivr = pccb->Npadap->Mna_amiv;
        *pccb->Mna_acivr = pccb->Npadap->Mna_aciv;
        *pccb->Amcsr |= AMCSR_STD;
       /* N_PORT spec says shift >> 5, but CITCA/CIMNA spec DON'T */
        svatophys( pccb->Npadap, pccb->Abbr );
        *pccb->Cicr = CICR_CIC;
        Chnl_wait_pic();
        if( *pccb->Csr & CSR_CIC ) {
            if( pccb->Lpstatus.init ) {
                Scaaddr_low( pccb->lpinfo.addr ) = Imp_lport_addr( pccb );
                if(( pccb->Pkt_mult = (( Aibuf_len( pccb )) >> 9 )) > 6 ) {
                    pccb->Pkt_mult = 6;
                }
                if( Aramp( pccb )) {

/* Read Ramp info in param block and alloc memory - Ampb
 * CITCA/CIMNA doesn't request memory.
 */
                }
                pccb->Fn_level = Cimna_fn_level( pccb );
            }
            *pccb->Csrcr = CSRCR_CSRC;
            *pccb->Amcsr |= CIMNA_PMCS_IE;
            *pccb->Cecr = CECR_CEC;
            Chnl_wait_pic_clear();
            if(( *pccb->Csr & CSR_CEC ) && !( *pccb->Csr & CSR_CIC )) {
                    *pccb->Csrcr = CSRCR_CSRC;
                    Svatophy_shiftR5( pccb->Dadfq.T_ptr, *pccb->Adfqir );
                    Svatophy_shiftR5( pccb->Damfq.T_ptr, *pccb->Amfqir );
            } else {
                    event = E_ENABLE_STATE;
                    status = RET_FAILURE;
            }
        } else {
            event = E_DISABLE_STATE;
            status = RET_FAILURE;
        }
/*
    } else {
        event = E_UCODE_START;
        status = RET_FAILURE;
    }
*/
    if( status == RET_FAILURE ) {
        if( pccb->Reinit_tries <= 1 )  {
            event = FE_UCODE_START;
        }
        ( void )np_log_dev_attn( pccb, event, LOG_REGS );
        ( void )( *pccb->Disable_port )( pccb, PS_UNINIT );
    }
    return( status );
}
#endif      
