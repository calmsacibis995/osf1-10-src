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
static char *rcsid = "@(#)$RCSfile: np_error.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 17:45:53 $";
#endif
/*
 * derived from np_error.c	5.2	(ULTRIX)	10/16/91";
 */
/************************************************************************
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		error processing routines and functions.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Function/Routines:
 *
 *   np_cleanup_port		Clean up Local CI Port
 *   np_console_log		Log CI Events to Console Terminal
 *   np_log_badport		Log Bad Port Numbers in CI Packets
 *   np_log_dev_attn		Log CI Device Attention Events
 *   np_log_initerr		Log CI Port Initialization Fatal Errors
 *   np_log_packet		Log CI Packet Related Events
 *   np_map_port		Map Local CI Port
 *   np_unmap_port		Unmap Local CI Port
 *   npsc_disable		N_PORT Single Channel Disable Local Port
 *
 *   Modification History:
 *
 *   31-Oct-1991	Peter Keilty
 *	Ported to OFS/1
 *
 *   16-Oct-1991	Brian Nadeau
 *	Updates/bug fixes
 *
 *   23-Jul-1991	Brian Nadeau
 *	New NPORT module.
 */

/*	      Protecting Register Accesses Against Machine Checks
 *
 * Machine checks are relatively rare events which may occur for many reasons.
 * The vast majority have absolutely no CI involvement; however, a few result
 * from CI port driver attempts to access inaccessible addresses in CI adapter
 * I/O space.  It is these machine checks the CI port driver is specifically
 * concerned about and wants to protect against.
 * 
 * The only CI adapter I/O space addresses the CI port driver directly accesses
 * are those corresponding to adapter local port registers.  These registers
 * are accessed by the driver during the following events:
 *
 * 1. Local CI port initialization.
 * 2. Local CI port notification of command and free datagram/message buffer
 *    availability.
 * 3. Processing of local CI port interrupts.
 * 4. Local CI port notification of the continued functioning of host software.
 * 5. Logging of certain local CI port errors.
 * 6. Error recovery following failure of a local CI port.
 *
 * Events 2-4 take place quite frequently.  The remaining events occur
 * extremely rarely.  Any scheme for port driver protection against extraneous
 * machine checks must take the frequencies of these events into account.
 *
 * To summarize, the only machine checks the CI port driver needs to guard
 * against are those that result from driver attempts to access inaccessible
 * local port registers.  Normally, all registers are of course fully
 * accessible.  Machine checks would be extremely common if this was not the
 * case.  However, a number of well-defined circumstances do exist under which
 * registers become inaccessible.  The most likely of these is loss of power,
 * either system-wide or just isolated to the bus on which the CI adapter
 * resides.  Certain CI hardware port types( CI750, CIBCI ) are also subject to
 * completely independent losses of power.  The adapters for these port types
 * are located within their own physically separate cabinets.  This makes them
 * vulnerable not only to separate losses of power but to becoming uncabled
 * from the busses on which they reside.  Either unfortunate circumstances
 * results in inaccessibility of most, but not all, local port registers.  The
 * registers of a sufficiently broken local port may also become inaccessible.
 *
 * The best most idealistic machine check protection strategy for the CI port
 * driver would be for it to always provide transparent protection against
 * machine checks during all register accesses.  Unfortunately, this is not
 * possible because the only protective mechanisms currently available are
 * expensive and are not transparent.
 *
 * The next best strategy is a defensive one: the CI port driver only protects
 * against machine checks on local CI ports known to have lost power, become
 * physically uncabled, or to have suffered a sufficient hardware failure.  In
 * other words, protective mechanisms are employed only for those local ports
 * determined to be at risk.  At all other times the driver makes no attempt to
 * protect against machine checks during register accesses.
 *
 * Implementation of this strategy is also unfortunately not possible.  This is
 * because for it to succeed the CI port driver would have to meet the
 * following two requirements:
 *
 * 1. It would have to be immediately notified when any of the events which
 *    could result in inaccessible local CI port registers occurs.
 * 2. It would have to immediately protect all subsequent register accesses on
 *    notification of local port machine check vulnerability.
 *
 * The first requirement can not be met because the driver is never notified of
 * system-wide or bus associated power loss.  Ironically ULTRIX crashes with a
 * machine check when such power loss occurs!  Furthermore, while the driver is
 * notified of independent CI port power loss, uncabling, and fatal errors
 * through interrupts; handling of such interrupts maybe temporarily blocked
 * postponing notification.  Such blockage is due to processor IPL level in
 * single processor environments and current unavailability of critical locks
 * in SMP environments.  The end result is the same, inability to immediately
 * notify the driver of local port machine check vulnerability on register
 * accesses.
 *
 * The second requirement for implementation of the next best strategy can also
 * not be met.  To meet it requires CI port driver determination of whether a
 * local CI port is currently vulnerable to machine checks prior to each
 * register access and to employ protective mechanisms only if it is.  Meeting
 * this requirement is much too costly given the frequency with which certain
 * register accesses are made.
 *
 * The machine check protection strategy actually employed by the CI port
 * driver is a realistic one and possesses the following two basic
 * characteristics:
 *
 * 1. It never attempts to protect against machine checks during register
 *    accesses that take place frequently( Events 2-4 above ).
 * 2. It considers a local port to always be at risk and protects against
 *    machine checks during all infrequent register accesses( Events 1,5-6
 *    above ).
 * 3. The first checks it makes during interrupt processing are for uncabled
 *    or powerless local ports before allowing any subsequent register accesses
 *    by the current thread to occur.
 *
 * At first glance it may seem that this strategy is not all that rigorous,
 * and that better ones could be designed, and to some extent this is true.
 * However, this strategy is actually the best one available given the tools
 * which are currently provided by the Ultrix kernel.
 *
 * The basic tool employed for machine check protection is the BADADDR() macro.
 * This macro determines whether a specified byte, word, or longword address is
 * addressable and "returns" 1 if it is not.  It is used in the machine check
 * protection strategy to determine whether a register is accessible before
 * attempting to access it.  Therefore, the actual register accesses are never
 * themselves actually protected.  
 *
 * This mechanism is as defensive as it is possible to be without being
 * paranoid and without seriously affecting performance of mainline code paths.
 * It will serve until it is possible to transparently protect against machine
 * during all register accesses.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<sys/systm.h>
#include		<sys/vm.h>
#include		<dec/binlog/errlog.h>
#include		<machine/pmap.h>
#include		<machine/cpu.h>
#include		<hal/cpuconf.h>
#include		<io/dec/xmi/xmireg.h>
#include		<io/dec/scs/sca.h>
#include		<io/dec/scs/scaparam.h>
#include		<io/dec/scs/scamachmac.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/scs/scs.h>
#include                <io/dec/ci/cippd.h>
#include                <io/dec/np/npport.h>
#include                <io/dec/np/npadapter.h>

#ifdef __alpha
#include		<io/common/devdriver.h>
#include		<io/dec/mbox/mbox.h>
#endif

/* External Variables and Routines.
 */
extern	SCSIB		lscs;
extern	u_short		np_severity;
extern	u_short		np_errlog;

#ifdef (vax | mips)
extern	u_int		np_bhole_pfn;
extern  u_char		*np_black_hole;
#endif

extern  scaaddr         scs_system_id;
extern	CLSTAB		np_cltab[ ES_FE + 1 ][ ESC_PPD + 1 ];
extern	u_char		scs_node_name[];
extern	void		np_dealloc_pkt(), np_log_packet(), np_unmap_port(),
			np_unmapped_isr(), cippd_stop(),
			lamb_disable_errors(), lamb_enable_errors();

/*   Name:	np_cleanup_port	- Clean up Local CI Port
 *
 *   Abstract:	This routine directs the second stage of local CI port clean
 *		up.  It is always invoked by forking to it.
 *
 *		Failed local CI ports are cleaned up in two stages.  The first
 *		stage consists of those actions which should be performed
 *		immediately following port disablement and are insensitive to
 *		processor state. The second stage consists of those activities
 *		which need not be performed immediately following port
 *		disablement and should be executed within a constant
 *		well-defined processor state.  This routine direct this second
 *		stage of port clean up and the constant environment necessary
 *		for its proper execution is provided by always forking to it.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	lpinfo.reason		-  Reason for port failure
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.cleanup	-   1
 *	    fsmstatus.fkip	-   1
 *	    fsmstatus.online	-   0
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.fkip	-   0
 *
 *   SMP:	The PCCB is locked to synchronize access.  PCCB locking is
 *		probably unnecessary because of lack of conflict for the PCCB
 *		due to single threading of port clean up and re-initialization.
 *		It is done anyway to guarantee unrestricted access and because
 *		the CI PPD interval timer may still be active.  PCCB addresses
 *		are always valid because these data structures are never
 *		deleted once their corresponding ports have been initialized.
 */
void
np_cleanup_port( pccb )
    PCCB		*pccb;
{
    NPBH		**lp, **ep, *cibp;
    u_int		code, pf_reason;
    u_int		save_ipl = Splscs();

    /* The steps involved in the second stage of local port clean up include:
     *
     * 1. Locking the PCCB.
     * 3. Map the specific local port crash reason into a generic one.
     * 4. Unlocking the PCCB.
     * 5. Notifying the CI PPD of failure of the local port.
     *
     * CI ports logout all internalized queue entries to the appropriate area
     * only when power failure is detected.  Both logout areas are scanned in
     * their entirety and any packets found are deallocated( Step 3 ).  The
     * logout areas are also re-initialized to NPBH_FREE( -1 ) to be able to
     * differentiate logged packets from empty logout area slots on subsequent
     * power failures.
     *
     * The CI PPD completes the clean up of its portion of the local port
     * including the failure and clean up all paths, both formative and fully
     * established, originating at the port( Step 5 ).  Clean up of the last
     * path triggers scheduling of port re-initialization by the CI PPD.  The
     * PCCB lock is released( Step 4 ) prior to notifying the CI PPD of local
     * port failure instead of after it as required by the SCA architecture.
     */
    Lock_pccb( pccb )
    Pccb_fork_done( pccb, PANIC_PCCBFB )
    if( Eseverity( pccb->lpinfo.reason ) == ES_FE ) {
	pf_reason = PF_FATALERROR;
    } else if( Ecode( pccb->lpinfo.reason ) == SE_POWER ) {
	pf_reason = PF_POWER;
    } else {
	pf_reason = PF_PORTERROR;
    }
    Unlock_pccb( pccb )
    ( void )cippd_stop( pccb, pf_reason );
    ( void )splx( save_ipl );
}

/*   Name:	np_console_log	- Log CI Events to Console Terminal
 *
 *   Abstract:	This routine logs CI events to the console terminal.  The event
 *		is always one of the following types:
 *
 *		PATH_EVENT	- Path specific event
 *		RPORT_EVENT	- Remote port specific event
 *		LPORT_EVENT	- Local port specific event
 *
 *		Explicit formatting information must be provided for each
 *		event.  This requires updating of the following tables each
 *		time a new event is defined:
 *
 *		1. The appropriate entry within the CI console logging table(
 *		   np_cltab[][] ) must be updated to reflect the new maximum
 *		   code represented within the associated format table.
 *
 *		2. The associated format table itself( np_cli[], np_clw[],
 *		   np_clre[], np_cle[], np_clse[], np_clfe[], np_clppdse[] )
 *		   must be updated with both the class of variable information
 *		   and exact text to be displayed.  However, the appropriate
 *		   table should be updated with a NULL entry whenever the CI
 *		   port driver is specifically NOT to log a new event.  This
 *		   applies only to np_clppdse[] when a new CI PPD severe error
 *		   event is to be specifically logged only by the CI PPD and
 *		   not by appropriate port drivers such as the CI port driver.
 *
 *		NOTE: Console logging of events is bypassed whenever the event
 *		      severity does not warrant console logging according to
 *		      the current CI severity level( np_severity ).  Such
 *		      bypassing is overridden when the ECLAWAYS bit is set in
 *		      the event code indicating that the event is always to be
 *		      logged regardless of the current severity level.
 *
 *		NOTE: This routine does NOT log events arising external to the
 *		      CI port driver with the exception of those CI PPD severe
 *		      error events which are candidates for application of the
 *		      local port crash severity modifier( ESM_LPC ).
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   np_cltab			- CI console logging table
 *   np_severity		- CI console logging severity level
 *   cibp			- Address of CI port header( OPTIONAL )
 *   event			- Event code
 *   event_type			- PATH_EVENT, LPORT_EVENT, RPORT_EVENT
 *   cpu			- CPU type code
 *   cpusw			- CPU switch structure
 *   pb				- Path Block pointer( OPTIONAL )
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    devattn.cicpurevlev	-   Out-of-revision CPU microcode logging info
 *	    devattn.cirevlev	-   Port microcode information
 *	    max_fn_level	-   Maximum functional microcode revision level
 *	    max_rom_level	-   Maximum PROM/self-test microcode rev level
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) to synchronize access and
 *		prevent premature PB deletion when a PB is provided.
 *
 *		PBs do NOT require locking when provided because only static
 *		fields are accessed.  SBs NEVER require locking.
 */
void
np_console_log( pccb, pb, cibp, event, event_type )
    PCCB		*pccb;
    PB			*pb;
    NPBH		*cibp;
    u_int		event;
    u_int		event_type;
{
    u_int		fcode, severity = Eseverity( event );

    /* Events are logged according to their type and the class of variable
     * information they display.  Console messages for path specific events(
     * PATH_EVENT ) display the local and remote port station addresses and
     * remote system name by default when the event is of severity level error(
     * ES_E ) or severe error( ES_SE ).  Console messages for local port
     * specific events( LPORT_EVENT ) display the local port station address by
     * default when they are of severity level error( ES_E ) or higher.
     * Console messages for remote port specific events( RPORT_EVENT ) do not
     * display any information by default.
     *     
     * The following classes of variable information currently exist:
     *
     * 1. Remote CI port station address.
     * 2. Local CI port station address.
     * 3. CI port parameter register( PPR ) only.
     * 4. CI port registers.
     * 5. BI registers.
     * 6. CI packet fields.
     * 7. Appropriate CI port microcode revision levels.
     * 8. CPU microcode revision levels.
     * 9. Initial local CI port initialization information.
     *
     * Certain events may also be logged without displaying any variable
     * information.
     *
     * Which CI port registers are selected for console logging( Class 3 ) is
     * dependent upon hardware port type:
     *
     * CIXCD		XBER, PMCSR, and PSR.
     *
     * In the case of certain specific events the PESR is also logged.
     *
     * Only the CIBCI logs events which fall into the BI register class( Class
     * 5 ).  The CIBCA does NOT.  Registers displayed include: BICSR, BER, and
     * CNFR.
     *
     * The accessibility of each register location is always checked prior to
     * accessing because the local port is itself suspect whenever this routine
     * is invoked and requested to display register contents.  Inability to
     * access a location implies unavailability of the port itself( ie - brain
     * damage ) and the specific access is bypassed.  A more extensive
     * explanation of why and when register accesses require protection may be
     * found at the front of this module.
     *
     * A panic occurs whenever the CI port driver is not prepared to log the
     * event due to detection of any of the following circumstances:
     *
     * 1. The event type is unknown.
     * 2. The event is a SCS specific event.
     * 3. The severity level of the event is invalid.
     * 4. The code of the event exceeds the current maximum known code for the
     *	  class( CI or CI PPD ) and severity level of the event.
     * 5. The event is not represented within the appropriate console logging
     *    formatting table( indicating that the CI port driver should never
     *	  have been asked to log it in the first place ).
     * 6. The class of variable information associated with the event is
     *	  unknown.
     *
     * None of these circumstances should ever occur.
     *
     * NOTE: Events represented within console logging format tables by NULL
     *	     entries are events which are to be logged only by the CI PPD and
     *	     never by individual port drivers like the CI port driver.
     *	     Currently, only certain path specific CI PPD severe error events
     *	     fall into this category.
     */
#ifdef CI_DEBUG
    printf("np_console_log: entered\n");
    printf("np_console_log: pccb = %lx, pb= %lx, cibp = %lx\n",pccb,pb,cibp);
    printf("np_console_log: event = %x, event_type = %x\n",event,event_type);
#endif /* CI_DEBUG */
    if(( event_type < PATH_EVENT || event_type > LPORT_EVENT ) ||
	 Test_scs_event( event )			       ||
	 severity > ES_FE				       ||
	 Ecode( event ) > Clog_maxcode( np_cltab, event )      ||
	 Clog_tabmsg( np_cltab, event ) == NULL ) {
	( void )panic( PANIC_UNKCODE );
    } else if(( fcode = Clog_tabcode( np_cltab, event )) &&
	      ( fcode < CF_RPORT || fcode > CF_INIT )) {
	( void )panic( PANIC_UNKCF );
    } else if( np_severity > severity && !Test_cloverride( event )) {
	return;
    }
    ( void )printf( "%4s\t- ", &pccb->lpinfo.name );
    switch( event_type ) {

	case PATH_EVENT:
	    if( severity == ES_E || severity == ES_SE ) {
		if( severity == ES_E ) {
		    ( void )printf( "error on path" );
		} else {
		    ( void )printf( "severe error on path" );
		}
		( void )printf( "( local/remote port: %u/",
				 Scaaddr_low( pccb->lpinfo.addr ));
		if( pb ) {
		    ( void )printf( "%u, remote system: ",
				     Scaaddr_low( pb->pinfo.rport_addr ));
		    if( pb->sb ) {
			( void )printf( "%8s )\n\t- ",
					 pb->sb->sinfo.node_name );
		    } else {
			( void )printf( "? )\n\t- " );
		    }
		} else {
		    ( void )printf( "?, remote system: ? )\n\t- " );
		}
	    }
	    break;

	case LPORT_EVENT:
	    if( severity >= ES_E ) {
		if( Test_lpc_event( event )) {
		    ( void )printf( "port failing, " );
		}
		switch( severity ) {

		    case ES_E:
			( void )printf( "error( local port %u )\n\t- ",
					 Scaaddr_low( pccb->lpinfo.addr ));
			break;

		    case ES_SE:
			( void )printf( "severe error( local port %u )\n\t- ",
					 Scaaddr_low( pccb->lpinfo.addr ));
			break;

		    case ES_FE:
			( void )printf( "fatal error( local port %u )\n\t- ",
					 Scaaddr_low( pccb->lpinfo.addr ));
			break;
		}
	    }
	    break;
    }
    ( void )printf( "%s", Clog_tabmsg( np_cltab, event ));
    switch( fcode ) {

	case CF_NONE:
	    ( void )printf( "\n" );
	    break;

	case CF_RPORT:
	    if( pb ) {
		( void )printf( "( remote port: %u )\n",
				 Scaaddr_low( pb->pinfo.rport_addr ));
	    } else {
		( void )printf( "( remote port: ? )\n" );
	    }
	    break;

	case CF_LPORT:
	    ( void )printf( "( local port %u )\n",
			     Scaaddr_low( pccb->lpinfo.addr ));
	    break;

	case CF_PPR:
	    ( void )printf( "\n\t- cluster size: 0x%08lx\n", Cluster_size( pccb ));
	    break;

	case CF_REGS:
	    switch( pccb->lpinfo.type.hwtype ) {

		case HPT_CIMNA:
		    ( void )printf( "\n\t- xber/amcsr/csr:\n\t0x%08lx", Get_reg( pccb->Mna_ber ));
		    break;

		case HPT_CITCA:
		    ( void )printf( "\n\t- tcber/amcsr/csr:\n\t0x%08lx", Get_reg( pccb->Tca_ber ));
		    break;

		default:
		    ( void )panic( PANIC_HPT );
	    }
	    ( void )printf( "/0x%08lx", Get_reg( pccb->Amcsr ));
	    ( void )printf( "/0x%08lx\n", Get_reg( pccb->Csr ));
	    break;

	case CF_REGS2:
	    switch( pccb->lpinfo.type.hwtype ) {

		case HPT_CIMNA:
		    ( void )printf( "\n\t- xber/amcsr/csr/cesr/cfar:\n\t0x%08lx", Get_reg( pccb->Mna_ber ));
		    break;

		case HPT_CITCA:
		    ( void )printf( "\n\t- tcber/amcsr/csr/cesr/cfar:\n\t0x%08lx/0x%08lx", Get_reg( pccb->Tca_ber ));
		    break;

		default:
		    ( void )panic( PANIC_HPT );
	    }
	    ( void )printf( "/0x%08lx", Get_reg( pccb->Amcsr ));
	    ( void )printf( "/0x%08lx", Get_reg( pccb->Csr ));
	    ( void )printf( "/0x%08lx", Get_reg( pccb->Cesr ));
	    ( void )printf( "/0x%08lx\n", Get_reg( pccb->Cfar ));
	    break;

	case CF_PKT:
	    ( void )printf( "\n\t- opc/c_idx/flags/status/ci_opc/ci_flags/port:\n"); 
	    if( cibp ) {
		( void )printf( "\t0x%02x/0x%02x/0x%04x/0x%04x/0x%02x/0x%02x/0x%02x\n",
				 ( u_int )cibp->opc,
				 ( u_int )cibp->c_idx,
				 ( u_int )*( u_short * )&cibp->flags,
				 ( u_int )*( u_short * )&cibp->status,
				 ( u_int )cibp->ci_opc,
				 ( u_int )*( u_char * )&cibp->ci_flags,
				 ( u_int )Get_pgrp( pccb, cibp ));
	    } else {
		( void )printf( "0x??/0x??/0x??/0x??\n" );
	    }
	    break;

	case CF_UCODE:
	    if( severity < ES_SE ) {
		( void )printf( "( local port %u )",
				 Scaaddr_low( pccb->lpinfo.addr ));
	    }
	    switch( pccb->lpinfo.type.hwtype ) {

		case HPT_CIMNA:
		case HPT_CITCA:
		    ( void )printf( "\n\t- current functional/self-test " );
		    ( void )printf( "microcode levels:   %u/%u",
				     pccb->Devattn.cirevlev.ci_ramlev,
				     pccb->Devattn.cirevlev.ci_romlev );
		    ( void )printf( "\n\t- supported functional/self-test " );
		    ( void )printf( "microcode levels: %u/%u\n",
				     pccb->Max_fn_level,
				     pccb->Max_rom_level );
		    break;

		default:
		    ( void )panic( PANIC_HPT );
	    }
	    break;

	case CF_CPU:
	    ( void )printf( "\n\t- current/minimum %4s CPU microcode ",
			     ( u_char * )&pccb->Devattn.cicpurevlev.ci_hwtype);
	    ( void )printf( "revision level is: %u/%u\n",
			     pccb->Devattn.cicpurevlev.ci_currevlev,
			     pccb->Devattn.cicpurevlev.ci_mincpurev );
	    break;

	case CF_INIT:
	    ( void )printf( "( local port %u )",
			     Scaaddr_low( pccb->lpinfo.addr ));
	    switch( pccb->lpinfo.type.hwtype ) {

		case HPT_CIMNA:
		case HPT_CITCA:
		    printf("\n\t- functional/self-test microcode levels: " );
		    break;

		default:
		    ( void )panic( PANIC_HPT );
	    }
	    ( void )printf( "%u/%u\n",
			     pccb->Devattn.cirevlev.ci_ramlev,
			     pccb->Devattn.cirevlev.ci_romlev );
	    break;
    }
}

/*   Name:	np_log_badport	- Log Bad Port Numbers in CI Packets
 *
 *   Abstract:	This routine logs bad port numbers in CI packets.  For a port
 *		number within a packet to be considered bad it must exceed the
 *		hardware maximum port number of the specified local port.
 *
 *		NOTE: This is a mandatory PD routine( Log_badportnum ) for use
 *		      by the CI PPD.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cippdbp			- Address of CI PPD header
 *   pccb			- Port Command and Control Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) as required by
 *		np_log_packet(), the routine which performs the actual logging.
 */
void
np_log_badport( pccb, cippdbp )
    PCCB		*pccb;
    NPPPDH		*cippdbp;
{
    ( void )np_log_packet( pccb,
			   NULL,
			   Ppd_to_pd( cippdbp, pccb ),
			   SE_BADPORTNUM,
			   LPORT_EVENT );
}

/*   Name:	np_log_dev_attn	- Log CI Device Attention Events
 *
 *   Abstract:	This routine logs CI device attention events.  Such events are
 *		detected directly from a specific local CI port as opposed to
 *		those events ascertained indirectly from a CI port packet.  The
 *		event is also optionally logged to the console.
 *
 *		Nine classes of events are currently logged by this routine:
 *
 *		1. Software errors detected during port initialization.
 *		2. Software detected hardware problems.
 *		3. Software detected invalid hardware CI port types.
 *		4. Software detected local port failures.
 *		5. CPU or port microcode problems.
 *		6. Explicit hardware errors.
 *		7. Stray interrupts.
 *		8. Failures to obtain access to port queue memory interlocks.
 *		9. Local port initializations.
 *
 *		Many of these events represent serious errors and are logged to
 *		save relevant information before drastic steps are attempted to
 *		resolve them.  Others are less serious and are logged only to
 *		give a warning or for informational purposes only.
 *
 *		NOTE: Stray interrupts are specific interrupts processed by the
 *		      special routine np_unmapped_isr().  This routine serves
 *		      as the interrupt service handler for local ports without
 *		      power or marked broken and permanently shutdown.  Refer
 *		      to it for a more extensive explanation of what interrupts
 *		      are considered stray.
 *
 *		NOTE: This routine does NOT log events arising external to the
 *		      CI port driver.  It currently does NOT even log those CI
 *		      PPD events which are candidates for application of the
 *		      local port crash severity modifier( ESM_LPC ).
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   event			- Event code
 *   cpu			- CPU type code
 *   cpusw			- CPU switch structure
 *   lscs			- Local system permanent information
 *   pccb			- Port Command and Control Block pointer
 *      pd.gvp.type.ci          -  CI specific PCCB fields
 *	    devattn.cicpurevlev	-   Out-of-revision CPU microcode information
 *	    devattn.cirevlev	-   Port microcode information
 *	    devattn.ciucode	-   Faulty microcode information
 *   regs			- LOG_REGS or LOG_NOREGS
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	lpinfo.nerrs		-  Number of errors on port
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) to synchronize access and as
 *		required by np_console_log(), the routine responsible for
 *		logging events to the console terminal.
 */
void
np_log_dev_attn( pccb, event, regs )
    PCCB		*pccb;
    u_int		event;
    u_int		regs;
{
    u_int		size;
    u_char		*celp;
    struct el_rec	*elp;
    u_int		severity = Eseverity( event );

    /* The steps involved in logging device attention events include:
     *
     * 1. Logging the event to the console.
     * 2. Incrementing the counter of local port errors.
     * 3. Computing the size of the application portion of the event log
     *	  record.
     * 4. Allocating an event log record and initializing the record's sub id
     *	  packet fields.
     * 5. Initializing the portion of the record common to all CI events.
     * 6. Initializing the portion of the record reserved for register
     *    contents.
     * 7. Moving any optional device attention information into the record.
     * 8. Validating the event log record.
     *
     * The ability of this routine to log the event is validated during console
     * logging( Step 1 ).  A panic occurs whenever the CI port driver is not
     * prepared to log the reported event.
     *
     * Not all events increment the counter of errors which have occurred on
     * the specified local port( Step 2 ).  Only those events with severity
     * level equal to or greater than error( ES_E ) increment the counter.
     *
     * This routine immediately terminates on failure to allocate an event log
     * record( Step 4 ).
     *
     * Not all device attention events request logging of device registers(
     * Step 6 ).  Sufficient space is reserved within the event log record for
     * register contents only for those that do.  Both CI port registers and
     * interconnect specific registers are logged when register logging is
     * requested.
     *
     * Accessibility of register locations are always checked prior to
     * accessing because the local port is itself suspect whenever this routine
     * is invoked and requested to log register contents.  Accessibility to
     * each CI port register is explicitly tested before accessing while only
     * general accessibility to the interconnect registers, and not to each
     * separate one, is tested before accessing.  Inability to access a
     * location implies unavailability of the port itself( ie - brain damage )
     * and the specific access is bypassed.  A more extensive explanation of
     * why and when register accesses require protection may be found at the
     * front of this module.
     *
     * The following types of mutually exclusive optional device attention
     * information may require logging( Step 7 ):
     *
     * 1. Microcode revision levels.
     * 2. CPU microcode revision levels.
     * 3. Information about improperly loaded functional microcode.
     *
     * Sufficient space is reserved within the event record for such optional
     * information only for those events which require its logging.  Logging of
     * microcode revision levels is required by four events: I_LPORT_INIT,
     * I_LPORT_REINIT, FE_BADUCODE and W_UCODE_WARN.  Logging of CPU microcode
     * revision levels is required by a single event: FE_CPU.  Logging of
     * information about improperly loaded functional microcode is also
     * required only by a single event: E_UCODE_LOAD.
     *
     * NOTE: Requests for logging of register contents and requirements for
     *	     logging of optional device attention information are orthogonal.
     *	     In other words, an event may request register content logging and
     *	     require logging of optional information, or neither, or any 
     *	     combination in between.
     *
     * NOTE: Fields reserved for registers which are either unused by local CI
     *	     ports of a specific hardware port type or are currently
     *	     inaccessible are initialized to EL_UNDEF.
     */
    ( void )np_console_log( pccb, NULL, NULL, event, LPORT_EVENT );
    if( Eseverity( event ) >= ES_E ) {
	Event_counter( pccb->lpinfo.nerrs )
    }
    if( np_errlog > severity && 
	!Test_cloverride( event ) &&
	np_errlog < SCA_ERRLOG3 ) {
	return;
    }
    size = sizeof( struct ci_common );
    if( regs == LOG_REGS ) {
	size += sizeof( struct ci_regs );
	switch( pccb->Interconnect ) {

	    case ICT_TC:
		size += sizeof( struct _tca_reg );
		break;

	    case ICT_XMI:
		size += sizeof( struct cixmi_regs );
		break;

	    default:
		( void )panic( PANIC_IC );
	}
    }
    switch( Mask_esevmod( event )) {

	case I_LPORT_REINIT:
	case I_LPORT_INIT:
	case E_SHORT_DPAGE:
	case E_ENABLE_STATE:
	    size += sizeof( struct ci_npparms );
	    break;
	case W_UCODE_WARN:
	case FE_BADUCODE:
	    size += sizeof( struct ci_revlev );
	    break;

	case E_UCODE_LOAD:
	    size += sizeof( struct ci_ucode );
	    break;

	case FE_CPU:
	    size += sizeof( struct ci_cpurevlev );
	    break;
    }
    if(( elp = ealloc( size, EL_PRIHIGH )) == EL_FULL ) {
	return;
    }
    LSUBID( elp,
	    ELCT_DCNTL,
	    ELCI_ATTN,
	    pccb->lpinfo.type.hwtype,
	    Ctrl_from_name( pccb->lpinfo.name ),
	    EL_UNDEF,
	    event )
    Elcicommon( elp )->ci_optfmask1 = 0;
    Elcicommon( elp )->ci_optfmask2 = 0;
    Elcicommon( elp )->ci_evpktver = CI_EVPKTVER;
    U_int( *Elcicommon( elp )->ci_lpname ) = pccb->lpinfo.name;
    Move_node( lscs.system.node_name, Elcicommon( elp )->ci_lname )
    Move_scaaddr( lscs.system.sysid, *Elcicommon( elp )->ci_lsysid )
    Move_scaaddr( pccb->lpinfo.addr, *Elcicommon( elp )->ci_lsaddr )
    Elcicommon( elp )->ci_nerrs = pccb->lpinfo.nerrs;
    Elcicommon( elp )->ci_nreinits = pccb->lpinfo.nreinits;
    celp = ( u_char * )Elcidattn( elp );
    if( regs == LOG_REGS ) {
	Elcicommon( elp )->ci_optfmask1 |= CI_REGS;
	Elciciregs( celp )->ci_cnfr = EL_UNDEF;
	Elciciregs( celp )->ci_pmcsr = Get_reg( pccb->Amcsr );
	Elciciregs( celp )->ci_psr = Get_reg( pccb->Csr );
	Elciciregs( celp )->ci_pfaddr = Get_reg( pccb->Cfar );
	Elciciregs( celp )->ci_pesr = Get_reg( pccb->Cesr );
	Elciciregs( celp )->ci_ppr = EL_UNDEF;
	celp += sizeof( struct ci_regs );
	switch( pccb->Interconnect ) {

	    case ICT_TC:
		Elcicommon( elp )->ci_optfmask1 |= CI_TCREGS;
		if( !Bad_reg( pccb->Tca_dev )) {
		    Elcitcaregs( celp )->tcdev = *pccb->Tca_dev;
		    Elcitcaregs( celp )->tcber = *pccb->Tca_ber;
		} else {
		    Elcitcaregs( celp )->tcdev = EL_UNDEF;
		    Elcitcaregs( celp )->tcber = EL_UNDEF;
		}
		celp += sizeof( struct citca_regs );
		break;

	    case ICT_XMI:
		Elcicommon( elp )->ci_optfmask1 |= CI_XMIREGS;
		if( !Bad_reg( pccb->Mna_dev )) {
#ifdef __alpha
		    Elcixmiregs( celp )->xdev =
			RDCSR(LONG_32,pccb->Bus,pccb->Mna_dev);
		    Elcixmiregs( celp )->xbe =
			RDCSR(LONG_32,pccb->Bus,pccb->Mna_ber);
		    Elcixmiregs( celp )->xfadrl =
			RDCSR(LONG_32,pccb->Bus,pccb->Mna_fadrl);
		    Elcixmiregs( celp )->xfadrh =
			RDCSR(LONG_32,pccb->Bus,pccb->Mna_fadrh);
		    if( pccb->lpinfo.type.hwtype == HPT_CIMNA ) {
			Elcixmiregs( celp )->pidr =
				RDCSR(LONG_32,pccb->Bus,pccb->Mna_aidr);
			Elcixmiregs( celp )->pvr =
				RDCSR(LONG_32,pccb->Bus,pccb->Mna_amivr);
		    } else {
			Elcixmiregs( celp )->pidr = EL_UNDEF;
			Elcixmiregs( celp )->pvr = EL_UNDEF;
		    }
#else
		    Elcixmiregs( celp )->xdev = *pccb->Mna_dev;
		    Elcixmiregs( celp )->xbe = *pccb->Mna_ber;
		    Elcixmiregs( celp )->xfadrl = *pccb->Mna_fadrl;
		    Elcixmiregs( celp )->xfadrh = *pccb->Mna_fadrh;
		    if( pccb->lpinfo.type.hwtype == HPT_CIMNA ) {
		        Elcixmiregs( celp )->pidr = *pccb->Mna_aidr;
		        Elcixmiregs( celp )->pvr = *pccb->Mna_amivr;
		    } else {
		        Elcixmiregs( celp )->pidr = EL_UNDEF;
		        Elcixmiregs( celp )->pvr = EL_UNDEF;
		    }
#endif /* __alpha */
		} else {
		    Elcixmiregs( celp )->xdev = EL_UNDEF;
		    Elcixmiregs( celp )->xbe = EL_UNDEF;
		    Elcixmiregs( celp )->xfadrl = EL_UNDEF;
		    Elcixmiregs( celp )->xfadrh = EL_UNDEF;
		    Elcixmiregs( celp )->pidr = EL_UNDEF;
		    Elcixmiregs( celp )->pvr = EL_UNDEF;
		}
		celp += sizeof( struct cixmi_regs );
		break;

	    default:
		( void )panic( PANIC_IC );
	}
    }
    switch( Mask_esevmod( event )) {

	case I_LPORT_REINIT:
	case I_LPORT_INIT:
	case E_SHORT_DPAGE:
	case E_ENABLE_STATE:
	    Elcicommon( elp )->ci_optfmask1 |= CI_NPPARMS;
	    *Elcinpapb( celp ) = *Elcinpapb( &pccb->Adap_pb );
	    celp += sizeof( struct ci_npapb );
	    *Elcinpcpb( celp ) = *Elcinpcpb( &pccb->Chnl_pb[pccb->C_idx] );
	    celp += sizeof( struct ci_npcpb );
	    break;
	case W_UCODE_WARN:
	case FE_BADUCODE:
	    Elcicommon( elp )->ci_optfmask1 |= CI_REVLEV;
	    *Elcirevlev( celp ) = pccb->Devattn.cirevlev;
	    break;

	case E_UCODE_LOAD:
	    Elcicommon( elp )->ci_optfmask1 |= CI_UCODE;
	    *Elciucode( celp ) = pccb->Devattn.ciucode;
	    break;

	case FE_CPU:
	    Elcicommon( elp )->ci_optfmask1 |= CI_CPUREVLEV;
	    *Elcicpurevlev( celp ) = pccb->Devattn.cicpurevlev;
	    break;
    }
    EVALID( elp )
}

/*   Name:	np_log_initerr	- Log CI Port Initialization Fatal Errors
 *
 *   Abstract:	This routine logs a special type of CI device attention event:
 *		software errors detected during probing of local CI ports.
 *		These fatal error events are logged as device attentions
 *		because they pertain to a specific local CI port.  However,
 *		they are considered special because they pre-date allocation of
 *		a PCCB for the local port, and therefore, may not make use of
 *		it for event logging purposes.  The following special events
 *		are currently defined:
 *
 *		1. FE_INIT_NOMEM   - Insufficient dynamic memory
 *		2. FE_INIT_ZEROID  - Uninitialized system identification num
 *		3. FE_INIT_NOUCODE - CI microcode absent
 *		4. FE_INIT_UNKHPT  - Unknown hardware port type
 *		5. FE_INIT_MISMTCH - Mismatched CI Port ucode & hw port types
 *
 *		All such events represent fatal errors.  None ever have the
 *		local port crash severity modified( ESM_LPC ) applied.
 *
 *		The occurrence of an port initialization fatal event is also
 *		automatically logged to the console, but without variable
 *		information.
 *
 *		Explicit formatting information must be provided for each
 *		event.  This requires updating of the following tables each
 *		time a new CI fatal error event is defined:
 *
 *		1. The CI fatal error event table( np_clfe[]) must be updated
 *		   with the exact text to be displayed and the console logging
 *		   format code CF_NONE( CI fatal error events discovered during
 *		   port probing currently never display variable information ).
 *
 *		2. The fatal error event entry within the CI console logging
 *		   table( np_cltab[][] ) must be updated to reflect the new
 *		   maximum fatal error event code.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   np_cltab			- CI console logging table
 *   cinum			- CI adapter number
 *   event			- Fatal error event code
 *   hpt			- Hardware port type
 *   interconnect		- Interconnect type
 *   scs_node_name		- SCS node name
 *   scs_system_id		- SCS system identification number
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	No locks are required.
 */
void
np_log_initerr( cinum, interconnect, hpt, event )
    u_int		cinum;
    u_int		interconnect;
    u_int		hpt;
    u_int		event;
{
    struct el_rec	*elp;

    /* Logging of fatal errors detected during CI port probings proceeds as
     * follows:
     *
     * 1. The fatal error is validated.
     * 2. The fatal error event and a local port permanently offline message
     *	  are both printed on the console.
     * 3. An event log record is allocated and the record's sub id packet
     *	  fields are initialized.
     * 4. The portion of record common to all CI events is initialized.
     * 5. The event log record is validated.
     *
     * This routine panics if validation of the fatal error event( Step 1 )
     * fails indicating inability of the CI port driver to log the reported
     * event.
     *
     * Event logging is bypassed on failures to allocate an event log record(
     * Step 3 ).
     *
     * Note that no other CI specific information common to device attention
     * events is logged and that no variable information is displayed within
     * logged console messages.
     */
#ifdef CI_DEBUG
printf(">>>np_log_initerr: entry\n");
#endif /* CI_DEBUG */
    switch( event ) {

	case FE_INIT_NOMEM:
	case FE_INIT_ZEROID:
	case FE_INIT_NOUCODE:
	case FE_INIT_UNKHPT:
	case FE_INIT_MISMTCH:
	    break;

	default:
	    ( void )panic( PANIC_UNKCODE );
    }
    ( void )printf( "ci%u\t- fatal error( local port ? )\n\t- %s\n",
		     cinum, Clog_tabmsg( np_cltab, event ));
    ( void )printf( "ci%u\t- permanently offline( local port ? )\n", cinum );
    if(( elp = ealloc( sizeof( struct ci_common ), EL_PRIHIGH )) == EL_FULL ) {
	LSUBID( elp, ELCT_DCNTL, ELCI_ATTN, hpt, cinum, EL_UNDEF, event )
	Elcicommon( elp )->ci_optfmask1 = 0;
	Elcicommon( elp )->ci_optfmask2 = 0;
	Elcicommon( elp )->ci_evpktver = CI_EVPKTVER;
	U_int( *Elcicommon( elp )->ci_lpname ) = Ctrl_from_num( "ci  ",cinum);
	Move_node( scs_node_name, Elcicommon( elp )->ci_lname )
	Move_scaaddr( scs_system_id, *Elcicommon( elp )->ci_lsysid )
	U_short( Elcicommon( elp )->ci_lsaddr[ 0 ]) = EL_UNDEF;
	U_short( Elcicommon( elp )->ci_lsaddr[ 2 ]) = EL_UNDEF;
	U_short( Elcicommon( elp )->ci_lsaddr[ 4 ]) = EL_UNDEF;
	Elcicommon( elp )->ci_nerrs = 1;
	Elcicommon( elp )->ci_nreinits = 0;
	EVALID( elp )
    }
}

/*   Name:	np_log_packet	- Log CI Packet Related Events
 *
 *   Abstract:	This routine logs CI packet related events.  Such events are
 *		ascertained indirectly from a CI port packet as opposed to
 *		those events detected directly from a specific local CI port.
 *		The event is also optionally logged to the console.
 *
 *		Five classes of events are currently logged by this routine:
 *
 *		1. Hardware detected errors during port command processing.
 *		2. Software detected invalid remote port states.
 *		3. Sofware detected invalid CI packet port numbers.
 *		4. All changes in cable states.
 *		5. Reception of packets over software non-existent paths.
 *
 *		Many of these events represent serious errors and are logged to
 *		save relevant information before drastic steps are attempted to
 *		resolve them.  Others are less serious and are logged only to
 *		give a warning or for informational purposes only.
 *
 *		NOTE: While all events logged therein arise indirectly from CI
 *		      port packets, the logging of each event does not
 *		      necessarily involve logging of the packet itself.
 *
 *		NOTE: This routine does NOT log events arising external to the
 *		      CI port driver with the exception of those CI PPD events
 *		      which are candidates for application of the local port
 *		      crash severity modifier( ESM_LPC ).
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cibp			- Address of CI port header( OPTIONAL )
 *   event			- Event code
 *   event_type			- PATH_EVENT, LPORT_EVENT, RPORT_EVENT
 *   lscs			- Local system permanent information
 *   pb				- Path Block pointer( OPTIONAL )
 *   pccb			- Port Command and Control Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	lpinfo.nerrs		-  Number of errors on port
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) to synchronize access, to
 *		prevent premature PB deletion when a PB is provided, and as
 *		required by np_console_log(), the routine responsible for
 *		logging events to the console terminal.
 *
 *		PBs do NOT require locking when provided because only static
 *		fields are accessed.  SBs NEVER require locking.
 */
void
np_log_packet( pccb, pb, cibp, event, event_type )
    PCCB		*pccb;
    PB			*pb;
    NPBH		*cibp;
    u_int		event;
    u_int		event_type;
{
    struct el_rec	*elp;
    u_int		opt_size;
    u_int		severity = Eseverity( event );

    /* The steps involved in logging packet related events include:
     *
     * 1. Logging the event to the console.
     * 2. Incrementing the counter of local port errors.
     * 3. Computing the size of the application portion of the event log
     *	  record.
     * 4. Allocating an event log record and initializing the record's sub id
     *	  packet fields.
     * 5. Initializing the portion of the record common to all CI events.
     * 6. Initializing the portion of the record common to all CI packet
     *	  related events.
     * 7. Moving any optional logged packet information into the record.  
     * 8. Validating the event log record.
     *
     * The ability of this routine to log the event is validated during console
     * logging( Step 1 ).  A panic occurs whenever the CI port driver is not
     * prepared to log the reported event.
     *
     * Not all events increment the counter of errors which have occurred on
     * the specified local port( Step 2 ).  Only those events with severity
     * level equal to or greater than error( ES_E ) increment the counter.
     *
     * This routine immediately terminates on failure to allocate an event log
     * record( Step 4 ).
     *
     * The size of the application portion of each event log record( Step 3 )
     * is dependent upon the presence or absence of optional information to be
     * included within the record( Step 7 ).  The following types of mutually
     * exclusive optional information may be logged:
     *
     * 1. The CI port packet responsible for the logged packet related event.
     *
     * Optional CI port packets are associated with many different events and
     * vary widely in size based upon their port operation codes.  Such packets
     * require truncation whenever logging their full size would exceed the
     * maximum size of an event log record.
     */
    ( void )np_console_log( pccb, pb, cibp, event, event_type );
    if( Eseverity( event ) >= ES_E ) {
	Event_counter( pccb->lpinfo.nerrs )
    }
    if( np_errlog > severity && 
	!Test_cloverride( event ) &&
	np_errlog < SCA_ERRLOG3 ) {
	return;
    }
    {
    u_int	size = sizeof( struct ci_common ) + sizeof( struct ci_lcommon );

    if( cibp ) {
	opt_size = pccb->lpinfo.Ovhd_pd;
        if( cibp->opc > 1 ) {
	    switch( cibp->ci_opc ) {

	    case SNDDG:   case SNDMSG:
		opt_size += ( sizeof( NPPPDH ) +
			      Appl_size( Pd_to_ppd( cibp, pccb )));
		break;

	    case SNDDAT:  case REQDAT1:  case REQDAT0:  case REQDAT2:
	    case RETDAT:
		opt_size += sizeof( SNDDATH );
		break;

	    case CNFREC:   case REQID: 	 case SNDRST:
		opt_size += sizeof( REQIDH );
		break;

	    case IDREC:
		opt_size += sizeof( IDRECH );
		break;

	    case SNDSTRT:
		opt_size += sizeof( SNDSTRTH );
		break;

	    case SNDLB:
		opt_size += sizeof( SNDLBH );
		break;

	    default:
		opt_size += ( sizeof( IDRECH ) - pccb->lpinfo.Ovhd_pd );
		break;
	    }
	} else {
	    switch( cibp->opc ) {

	    case SETCKT:
		opt_size += sizeof( SETCKTH );
		break;

	    case INVTC:
		break;

	    }
        }
	if(( size += opt_size ) > EL_MAXAPPSIZE ) {
	    opt_size += ( EL_MAXAPPSIZE - size );
	    size = EL_MAXAPPSIZE;
	}
    }
    if(( elp = ealloc( size, EL_PRIHIGH )) == EL_FULL ) {
	return;
    }
    LSUBID( elp,
	    ELCT_DCNTL,
	    ELCI_LPKT,
	    pccb->lpinfo.type.hwtype,
	    Ctrl_from_name( pccb->lpinfo.name ),
	    EL_UNDEF,
	    event )
    }
    Elcicommon( elp )->ci_optfmask1 = CI_LCOMMON;
    Elcicommon( elp )->ci_optfmask2 = 0;
    Elcicommon( elp )->ci_evpktver = CI_EVPKTVER;
    U_int( *Elcicommon( elp )->ci_lpname ) = pccb->lpinfo.name;
    Move_scaaddr( pccb->lpinfo.addr, *Elcicommon( elp )->ci_lsaddr )
    Move_scaaddr( lscs.system.sysid, *Elcicommon( elp )->ci_lsysid )
    Move_node( lscs.system.node_name, Elcicommon( elp )->ci_lname )
    Elcicommon( elp )->ci_nerrs = pccb->lpinfo.nerrs;
    Elcicommon( elp )->ci_nreinits = pccb->lpinfo.nreinits;
    {
    SB		*sb;

    if( pb ) {
	Move_scaaddr( pb->pinfo.rport_addr, *Elcilcommon( elp )->ci_rsaddr )
	sb = pb->sb;
    } else {
	U_short( Elcilcommon( elp )->ci_rsaddr[ 0 ]) = EL_UNDEF;
	U_short( Elcilcommon( elp )->ci_rsaddr[ 2 ]) = EL_UNDEF;
	U_short( Elcilcommon( elp )->ci_rsaddr[ 4 ]) = EL_UNDEF;
	sb = NULL;
    }
    if( sb ) {
	Move_scaaddr( sb->sinfo.sysid, *Elcilcommon( elp )->ci_rsysid )
	Move_node( sb->sinfo.node_name, Elcilcommon( elp )->ci_rname )
    } else {
	U_short( Elcilcommon( elp )->ci_rsysid[ 0 ])  = EL_UNDEF;
	U_short( Elcilcommon( elp )->ci_rsysid[ 2 ])  = EL_UNDEF;
	U_short( Elcilcommon( elp )->ci_rsysid[ 4 ])  = EL_UNDEF;
	U_int( Elcilcommon( elp )->ci_rname[ 0 ]) = EL_UNDEF;
	U_int( Elcilcommon( elp )->ci_rname[ 4 ]) = EL_UNDEF;
    }
    }
    if( cibp ) {
	Elcicommon( elp )->ci_optfmask1 |= ( CI_PACKET | CI_NPPKT );
	Elcinppacket( elp )->size = opt_size;
	( void )bcopy( &cibp->opc, &Elcinppacket( elp )->ci_opc, opt_size );
    }
    EVALID( elp )
}

/*   Name:	np_map_port	- Map Local CI Port
 *
 *   Abstract:	This routine maps a specified local CI port.  It is invoked
 *		only during processing of interrupts by the special routine(
 *		np_unmapped_isr()) which handles all interrupts, regardless of
 *		hardware port type, whenever local ports are unmapped.  Local
 *		ports maybe unmapped because they are temporarily without power
 *		or because they are marked broken and are permanently shutdown.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cpu			- CPU type code
 *   cpusw			- CPU switch structure
 *   map_extent			- MAP_REGS or MAP_FULL
 *   pccb			- Port Command and Control Block pointer
 *      pd.gvp.type.ci          -  CI specific PCCB fields
 *	    lpstatus.mapped	-   0
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    lpstatus.mapped	-   1
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Adapter I/O space successfully mapped
 *   RET_FAILURE		- Adapter I/O space unsuccessfully mapped
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) to synchronize access to the
 *		PCCB and provide exclusive access to the CIADAP vector element
 *		corresponding to the local CI port.
 */
u_int
np_map_port( pccb, map_extent )
    PCCB		*pccb;
    u_int		map_extent;
{
    NPADAP		*npadap;
    u_char		*vaddr;
    u_int		status = RET_SUCCESS;

    /* Mapping a local CI port includes:
     *
     * 1. Mapping local port adapter I/O space system PTEs back to their
     *	  original physical addresses, invalidating the system translation
     *	  buffers for each address mapped.
     * 2. Changing the interrupt service handler for the local port back to the
     *	  routine normally employed for this hardware port type.
     * 3. Verifying accessibility of the local port.
     *
     * The interrupt service handler for the local port is restored back to the
     * routine normally employed for this hardware port type only if full
     * mapping( map_extent == MAP_FULL ) was requested( Step 2 ).  Otherwise,
     * only the adapter I/O space is mapped.
     *
     * Failure is returned whenever the adapter can not be successfully
     * accessed following restoration of local port adapter I/O space system
     * PTEs( Step 3 ).  A panic occurs if this routine is invoked to map an
     * already mapped adapter.
     */
    if( !pccb->Lpstatus.mapped ) {
	pccb->Lpstatus.mapped = 1;
	if( map_extent == MAP_FULL ) {
	    npadap->isr[pccb->C_idx] = npadap->mapped_isr;
	    pccb->Ciisr->isr = npadap->mapped_isr;
	}
	switch( pccb->Interconnect ) {

	    case ICT_XMI:
		if( Bad_reg( pccb->Mna_dev )) {
		    status = RET_FAILURE;
		}
		break;

	    case ICT_TC:
		if( Bad_reg( pccb->Tca_dev )) {
		    status = RET_FAILURE;
		}
		break;

	    default:
		( void )panic( PANIC_IC );
	}
    } else {
	( void )panic( PANIC_MAP );
    }
    return( status );
}

/*   Name:	np_unmap_port	- Unmap Local CI Port
 *
 *   Abstract:	This routine unmaps a specified local CI port.  CI ports are
 *		unmapped only under the following circumstances:
 *
 *		1. Consecutive attempts exhausted without success during local
 *		   port initialization.
 *		2. Local port is determined to be broken.
 *
 *		The local port is permanently unmapped in the first two
 *		circumstances while it is only temporarily unmapped until power
 *		is restored in the last circumstance.
 *
 *		Unmapping local ports provides the means without impacting
 *		normal interrupt processing performance for handling unexpected
 *		interrupts which may occur while ports are without power or are
 *		permanently shutdown.  
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   np_bhole_pfn		- CI black hole mapping page page frame number
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    lpstatus.mapped	-   1
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    lpstatus.mapped	-   0
 *
 *   SMP:	The PCCB is locked( EXTERNALLY ) to synchronize access to the
 *		PCCB and provide exclusive access to the CIADAP vector element
 *		corresponding to the local CI port.
 */
void
np_unmap_port( pccb )
    PCCB		*pccb;
{

    /* Unmapping a local CI port includes:
     *
     * 1. Unmapping local port adapter I/O space system PTEs to the black hole
     *	  page, invalidating the system translation buffers for each page
     *	  unmapped.
     * 2. Changing the interrupt service handler for the local port to a
     *	  special routine( np_unmapped_isr()) for fielding of all future
     *	  interrupts while the adapter I/O space is unmapped.
     *
     * A panic occurs if this routine is invoked to unmap an already unmapped
     * adapter.
     */
    if( pccb->Lpstatus.mapped ) {
	pccb->Lpstatus.mapped = 0;
	pccb->Npadap->isr[pccb->C_idx] = np_unmapped_isr;
	pccb->Ciisr->isr = np_unmapped_isr;
    } else {
	( void )panic( PANIC_UNMAP );
    }
}

/*   Name:	npsc_disable	- Disable a Local Single Channel CITCA 
 *				  Family Port
 *
 *   Abstract:	This routine completely disables a local CITCA family port
 *		CITCA.  There are five occasions when this routine is
 *		invoked:
 *
 *		1. Prior to the initial initialization of a CITCA family port.
 *		2. During crashing of a CITCA family port.
 *		3. Following failure to initialize a CITCA family port.
 *		4. During disablement of a CITCA family port as part of system
 *		   shutdown.
 *		5. During processing of interrupts from CITCA family ports by
 *		   the special routine( np_unmapped_isr()) used as an interrupt
 *		   service handler by unmapped local ports marked broken and
 *		   permanently shutdown.
 *
 *		During port disablement various port registers are accessed.
 *		The accessibility of each register location is checked prior to
 *		accessing because the port is itself suspect whenever this
 *		routine is invoked.  Inability to access a location implies
 *		unavailability of the port itself( ie - brain damage ).  The
 *		specific access and all subsequent register accesses are
 *		bypassed.  A more extensive explanation of why and when
 *		register accesses require protection may be found at the front
 *		of this module.
 *
 *		NOTE: Local ports may be marked broken for many reasons other
 *		      than inability to access port or bus specific registers.
 *		      The disabling of such ports is crucial because they are
 *		      in the process of being permanently shutdown and may
 *		      still be currently active.  This is why disabling of a
 *		      local port is always attempted even when the port is
 *		      marked broken.  Only after a register access fails is the
 *		      local port regarded as truely dead, and inaccessible, and
 *		      all subsequent register accesses are bypassed.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cpu			- CPU type code
 *   cpusw			- CPU switch structure
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    lpstatus.mapped	-   1
 *   state			- Port state
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    ciregptrs.pmcsr	-   Port maintenance cntl & status reg pointer
 *	    ic.xmi.pidr		-   Port interrupt destination register pointer
 *	    ic.xmi.pvr		-   Port vector register pointer
 *	    lpstatus.mtimer	-   0
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   Port is broken status flag
 *
 *   SMP:	No locks are required.  However exclusive access to the PCCB
 *		must be guaranteed EXTERNALLY.  It may be guaranteed by locking
 *		the PCCB or by guaranteeing that only the processor executing
 *		this routine has access to it.
 */
void
npsc_disable( pccb, state )
    PCCB		*pccb;
    u_int		state;
{
    u_int		status = 0;

    /* Disabling a CITCA family port includes:
     *
     * - ENABLED PORTS ONLY:
     * 1. Disabling all interrupts on the port.
     *
     * - ALL PORTS:
     * 3. Disabling the port by transitioning it into the Uninitialized port
     *	  state.
     * 4. Resetting bus specific register contents.
     * 5. Halting the port microsequencer.
     * 6. Disabling the port maintenance timer.
     *
     * - BROKEN PORTS ONLY:
     * 7. Unmapping the local port adapter I/O space system PTEs to the black
     *	  hole page, invalidating the system translation buffers for each page
     *    unmapped.
     * 8. Changing the interrupt service handler for the local port to a
     *	  special routine( np_unmapped_isr()) for fielding of all future
     *	  interrupts while the adapter I/O space is unmapped.
     *
     * Gracefully shutting down a CI port( Step 2 ) aborts all port commands
     * currently undergoing processing.  It also triggers a port interrupt
     * following transitioning of the port into the disabled state.  As port
     * interrupts are currently disabled( Step 1 ), the expected interrupt
     * must be manually checked for.  The cause of any observed interrupt is
     * never ascertained, it is just assumed to be shutdown completion.
     * Shutdown is aborted whenever an interrupt is not detected within a
     * fixed period of time.
     *
     * CITCA family ports are disabled in a bus specific fashion( Step 3 ) by
     * resetting the node at which the port resides and waiting up to a fixed
     * period of time for the node reset to complete and the appropriate status
     * bit to be cleared.
     *
     * Bus specific register contents must be reset after disabling the port(
     * Step 4 ).  This is because executing a node reset( Step 3 ) to disable
     * the port clears their contents.
     *
     * The port microsequencer must be halted( Step 5 ) to allow access to
     * adapter control store because the act of resetting the node( Step 3 )
     * automatically started it.
     *
     * The port maintenance timer must be explicitly disabled( Step 6 ) because
     * the act of disabling the port( Step 3 ) automatically triggers
     * activation of the timer.  This step must always be performed even if the
     * maintenance timer had been previously disabled.
     *
     * Steps 7-8 constitute unmapping of the local CI port and are executed by
     * np_unmap_port().  These steps are only executed when the local port is
     * marked broken and either has been or is in the process of being
     * permanently shutdown.
     *
     * Inability to access a bus specific or CI port register location at any
     * time( Steps 1-6 ) results in bypassing of all subsequent register
     * accesses and execution of the following actions:
     *
     * 1. The absence of the local port is logged.
     * 2. The local port is marked broken.
     * 3. The local port is permanently shut down.
     *
     * Event logging( Action 1 ) of the local port is bypassed whenever the
     * local port was already marked broken indicating that its absence is
     * already known of and was previously logged.  Shutdown of the local
     * port( Action 3 ) is also bypassed under such circumstances.  Local ports
     * marked broken have already been shut down.
     *
     * It is not necessary for this routine to directly shutdown the local
     * port( Action 3 ).  Marking the local port broken( Action 2 ) is
     * sufficient to force local port shutdown.  This is because an attempt to
     * initialize the local port always follows port disablement and local port
     * initialization is inherently designed to shutdown broken local ports.
     *
     * One occassion does exist when local port initialization does not follow
     * local port disablement, and shutdown of broken local ports never takes
     * place.  This is when local port disablement occurs as part of system
     * shutdown.  Failure to shutdown the local port in this case does not
     * present any problems because the entire system is being shutdown and no
     * need exists to separately shutdown one isolated part of it.
     */
#ifdef __alpha
    u_int       amcsr;
#endif /* __alpha */
    if( state == PS_ENAB ) {
        if( !Bad_reg( pccb->Amcsr )) {
            switch( pccb->lpinfo.type.hwtype ) {
	
                case HPT_CIMNA:
#ifdef __alpha
                    amcsr = RDCSR(LONG_32,pccb->Bus,pccb->Amcsr);
                    WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,
                                        (amcsr & ~CIMNA_PMCS_IE));
#else
	            *pccb->Amcsr &= ~CIMNA_PMCS_IE;
#endif
	            break;
	        case HPT_CITCA:
	            *pccb->Amcsr &= ~CITCA_PMCS_IE;
	            break;
		default:
                    ( void )panic( PANIC_HPT );
	    }
	DELAY(20000);
	} else {
	    status = FE_NOCI;
	}
    }
    if( status == 0 ) {
	switch( pccb->lpinfo.type.hwtype ) {
	    case HPT_CIMNA:
		if( !Bad_reg( pccb->Mna_ber )) {
       		/* Notify LAMB to ignore errors that are generated 
		   	during CI selftest */
		    lamb_disable_errors();
		    if( xmisst( pccb->Mna_dev, pccb->Bus ) == 0 ) {
		        status = FE_NOCI;
		    }
		} else { 
		    status = FE_NOCI;
		}
		lamb_enable_errors();
	        break;

	    case HPT_CITCA:
		if( !Bad_reg( pccb->Tca_ber )) {
/*
		    if( tcsst( pccb->Tca_dev ) == 0 ) {
		        status = FE_NOCI;
		    }
*/
		} else { 
	    	    status = FE_NOCI;
		}
	        break;
	    default:
                ( void )panic( PANIC_HPT );
        }
    }
    if( status == 0 ) {
	if( !Bad_reg( pccb->Amcsr )) {
		if( !pccb->Lpstatus.init ) {
#ifdef __alpha
	    	WRTCSR(LONG_32,pccb->Bus,pccb->Amcsr,AMCSR_STD);
#else
	    	*pccb->Amcsr = AMCSR_STD;
#endif
		}
	    pccb->Lpstatus.mtimer = 0;
	} else {
	    status = FE_NOCI;
	}
    }
    if( status && !pccb->Fsmstatus.broken ) {
	pccb->Fsmstatus.broken = 1;
	( void )np_log_dev_attn( pccb, status, LOG_REGS );
    }
    if( pccb->Fsmstatus.broken ) {
	( void )np_unmap_port( pccb );
    }
}
