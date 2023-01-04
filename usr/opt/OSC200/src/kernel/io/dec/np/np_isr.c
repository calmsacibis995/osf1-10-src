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
static char *rcsid = "@(#)$RCSfile: np_isr.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/09/21 21:54:47 $";
#endif
/*
 * derived from np_isr.c	5.2	(ULTRIX)	10/16/91";
 */
/************************************************************************
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		NPORT interrupt service routines and functions.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Function/Routines:
 *
 *   np_rsp_handler		Process All CI Port Responses
 *   np_unmapped_isr		Unmapped CI Adapter Interrupt Service Routine
 *   cimna_isr			CIMNA Interrupt Service Routine
 *   citca_isr			CITCA Interrupt Service Routine
 *
 *   Modification History:
 *
 *   31-Oct-1991	Pete Keilty
 * 	Ported to OSF/1.
 *
 *   16-Oct-1991	Brian Nadeau
 *	Updates/bug fixes.
 *
 *   23-Jul-1991	Brian Nadeau
 *	New NPORT module.
 *
 */

/*			Interrupt Processing Philosophy
 *
 * The CI port driver's interrupt processing philosophy is to optimize its
 * primary interrupt service routines for response processing.  This is because
 * the placement of responses by local CI ports onto previously empty response
 * queues is the most like cause of interrupts.  Response processing is
 * optimized for by minimizing the number of checks made for errors.  It is
 * necessary to make some checks because most port detected errors represent
 * very serious situations which can only be recovered from by crashing and
 * re-initialization of the affected port.  In such instances responses are
 * never processed.  Instead, additional time is spent determining the exact
 * cause of the error before the local port is crashed.
 *
 * One major consequence of the minimization of error detection arising from
 * the optimization of response processing is the use of separate routines as
 * interrupt service handlers by different local CI ports and for all CI ports
 * when they are unmapped.  Different hardware port types require different
 * checks to be made for errors and the requirements for processing of
 * interrupts by unmapped local ports are radically different from the
 * requirements of mapped ones.  Whenever processing requirements sufficiently
 * diverge the CI port driver employs a different routine as interrupt service
 * handler for the local port.  
 */

/* 		    Interrupt Processing in SMP Environments
 *
 * In a SMP environment it is conceivable that multiple threads may be brought
 * into existence to simultaneously process interrupts.  For example, a local
 * port may signal an interrupt to report response availability.  While
 * processing of this initial interrupt is proceeding the same port may signal
 * another interrupt to again report response availability.  This scenario is
 * possible because the port status register in which response availability is
 * reported is released prior to initiating response processing.  It is also
 * possible that a second or even third interrupt may be signaled by the same
 * port to report an error either within the port status register or a bus
 * specific error register.  The end result is the same, each interrupt
 * potentially activates a new thread, each thread executes the same
 * ( appropriate ) interrupt service routine, and all such threads may be
 * simultaneously active( on separate processors ).
 *
 * The existence of multiple threads all executing the same interrupt service
 * routine never poses a problem provided all interrupts were signaled by the
 * local port to report response availability.  Only one caveat exists: it may
 * not be assumed that the absence of errors implies availability of responses
 * awaiting processing.  The existence of responses is always explicitly
 * checked for before the port status register is released and responses are
 * processed.
 *
 * The situation is not so straightforward when adapter reported errors are
 * involved.  The major source of complexity originates in the following facts:
 *
 * 1. Not all adapter reported errors are equal, an error hierarchy actually
 *    exists: soft error -> port( PSR ) error -> bus( non-port ) error in
 *    ascending order.
 * 2. Occurrence of a new interrupt wipes out the state of previous interrupts(
 *    interrupt state is reported within bus and port registers ).
 *
 * The error hierarchy allows an interrupt reporting response availability, to
 * be followed by an interrupt reporting a soft error, to be followed by an
 * interrupt reporting a port error, to be followed by an interrupt reporting a
 * bus error with the occurrence of each interrupt wiping out the state of
 * prior interrupts.  This sequence of ever more serious error interrupts would
 * never pose a problem in a single processor environment where only one
 * interrupt is processed at a time and is processed to completion.
 * Unfortunately, this is not the case in a SMP environment.  Multiple
 * interrupts may be processed simultaneously, some signaled to report
 * response availability while others to report errors.  Each new interrupt
 * potentially affects how previous interrupts are processed by completely
 * independent and concurrent threads.  Even the relative order in which
 * interrupts are processed to completion may not reflect the order in which
 * they were initially signaled due to temporary suspension of some threads
 * for handling of higher priority interrupts.
 * 
 * While processing of error interrupts is potentially a very complicated
 * situation in SMP environments it is by no means a hopeless one.  It is made
 * manageable by having the appropriate interrupt service routine execute the 
 * following general steps once it detects an adapter reported error:
 *
 * 1. The PCCB is locked to synchronize access to port registers.
 * 2. Bus and port register register contents are cached.
 * 3. A second determination is made as to whether an error is being reported
 *    by the adapter using the cached register contents.
 * 4. Interrupt processing is allowed to proceed( after unlocking the PCCB ) if
 *    no error is detected on the second check for errors.
 * 4. Soft errors are cleared or ignored and interrupt processing is allowed to
 *    proceed( after unlocking the PCCB ) as if no error had been detected.
 * 5. Hard errors crash the local port and terminate interrupt processing(
 *    after unlocking the PCCB ).
 *
 * The major benefit derived from executing this general sequence is that
 * threads complete error processing in the order in which they obtain the
 * appropriate PCCB lock after detecting an error.  This adds back a
 * deterministic element to error processing in SMP environments.
 *
 * A second benefit derived from this sequence is the reduction in the effect
 * new error interrupts have on existing threads attempting to process previous
 * adapter reported errors.  This reduction is achieved through utilization of
 * cached register contents in the determination of the nature of the reported
 * error.  It allows error processing to proceed in a consistent fashion.  New
 * error interrupts may still occur and still change register contents;
 * however, processing of new interrupts by newly spawned threads no longer has
 * major affect on error processing by existing threads.
 *
 * Use of cached register contents is not without its disadvantages though.
 * While determination of the nature of the error proceeds from cached
 * contents, logging of the event utilizes the contents of the registers
 * themselves.  Thus, registers logged may bear no relation to the event logged
 * when multiple virtually simultaneous( ie - one after the other ) errors
 * occur.  Such synchronicity should be extremely rare and does not outweigh
 * the advantages of using cached register contents.
 *
 * Use of cached register contents also does not completely eliminate the
 * effect of new error interrupts on processing of previous interrupts by
 * completely independent and concurrent threads.  This can best be observed
 * through use of a hypothetical example.  Consider the following set of time
 * sequential virtually simultaneous interrupts originated on the same local
 * port:
 *
 * 1. Interrupt to report response availability occurs.
 * 2. Interrupt to report soft error occurs.
 * 3. Interrupt to report a fatal error occurs.
 *
 * The following scenario may be concocted in order to demonstrate that while
 * side effects on interrupt processing resulting from new error interrupts in
 * SMP environments may be minimized, it may never be completely eliminated.
 *
 * 1. The interrupt reporting response availability occurs and is received by
 *    Processor 1.  Before the thread processing the interrupt( T1 ) has the
 *    opportunity to check for errors it is pre-empted by an interrupt with a
 *    higher priority.
 * 2. The interrupt reporting the soft error occurs and is received by
 *    Processor 2.  The thread processing the interrupt( T2 ) detects the
 *    error, locks the appropriate PCCB, and caches register contents.
 * 3. The interrupt reporting the fatal error occurs and is received by
 *    Processor 3.  The thread processing the interrupt( T3 ) detects the error
 *    but is unable to lock the appropriate PCCB because it is locked by T2.
 *    It spins waits waiting for the PCCB lock to become available.
 * 4. T1 resumes execution, checks for errors, notices that an error exists,
 *    and attempts to lock the appropriate PCCB.  It too spin waits waiting for
 *    the PCCB lock( locked by T2 ) to become available.
 * 5. T2 checks its cached register contents for errors, discovers the soft
 *    error, clears/ignores it, unlocks the PCCB, and proceeds to determine
 *    whether responses are available for processing.  As there are responses
 *    available, it proceeds to process them even though a fatal error has
 *    occurred since response availability was initially signaled.  Thread
 *    execution is terminated following processing of all available responses.
 * 6. T1 wins the race with T3 for the PCCB lock.  After obtaining the lock, it
 *    caches register contents, checks them, and determines that a fatal error
 *    has occurred.  It proceeds to crash the local port, unlock the PCCB and
 *    terminate thread execution.
 * 7. T3 obtains the PCCB lock, caches register contents, checks them, and
 *    discovers the lack of an error.  It does not find an error because
 *    crashing of the local port by T1 involved disabling it, and the act of
 *    disabling a local port resets its registers.  Thus, when T3 caches
 *    register contents, it caches contents which do not display an error.  T3
 *    proceeds to unlock the PCCB and continue as if no error had been
 *    initially detected.  When T3 checks explicitly for response availability
 *    it finds none, once again begin the registers have been reset, and the
 *    interrupt is dismissed.
 *
 * Many scenarios such as this one are possible.  What is evident in all of
 * them is that a potential for side effects is introduced when error
 * interrupts occur while prior interrupts are currently being processed.
 * There is no way to eliminate them short of always locking the PCCB when an
 * interrupt occurs and this choice is unacceptable for performance reasons.
 * Fortunately, the impact of such side effects may be minimized and that has
 * been one of the guiding principles during development of the methodology for
 * processing errors in SMP environments.
 * 
 * One final note, differences do exist in how individual interrupt service
 * routines go about executing the general steps outlined above on initial
 * detection of errors.  This is to be expected because of the differences in
 * port interfaces between the various hardware port types.  If such
 * differences did not exist there would be no need for separate interrupt
 * service routines in the first place.  However, every CI port driver
 * interrupt service which must be prepared for execution within SMP
 * environments( cibnp_isr(), cibca_isr(), cixcd_isr()) does follow these
 * general steps on detection of adapter reported errors.
 *
 * NOTE: This discussion pertains only to processing in SMP environments by the
 *	 primary interrupt service routines( 
 *	 cixcd_isr(), citca_isr() ).  It does not pertain to any processing by
 *	 the special routine( np_unmapped_isr()) used as interrupt service
 *	 handler by local ports temporarily without power( CI750/CI780//CIBCI
 *	 only ) or marked broken and permanently shutdown.  The special routine
 *	 itself contains a full description of any additional processing
 *	 requirements brought about by the introduction of SMP environments.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<sys/systm.h>
#include		<sys/vm.h>
#include		<dec/binlog/errlog.h>
#include		<io/dec/xmi/xmireg.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/np/npport.h>
#include		<io/dec/np/npadapter.h>
#include		<io/dec/ci/cippd.h>
#include		<io/dec/uba/ubavar.h>

#include		<io/dec/mbox/mbox.h>
#include		<io/common/devdriver.h>

/* External Variables and Routines.
 */
extern	int		dsaisr_thread_init, dsaisr_thread_off, shutting_down;
extern  fbq		dsa_fbq;
extern	PB		*cippd_get_pb();
extern	u_long		np_map_port();
extern	void		np_crash_lport(), np_dealloc_pkt(), np_init_port(),
			np_log_dev_attn(), np_log_packet(), np_rsp_handler(),
			np_unmap_lport(), np_update_cable(), cippd_crash_pb(),
			cippd_receive(), cippd_reqid_snt(), np_add_dg(),
			np_add_msg(), scs_data_done(), scs_dg_rec(),
			scs_msg_rec(), scs_msg_snt(), dsaisr_thread();


/*   Name:	np_rsp_handler	- Process All CI Port Responses
 *
 *   Abstract:	This routine is responsible for processing all entries on a CI
 *		port's response queue.  It is invoked by a CI port interrupt
 *		service routine whenever a port places a response onto a
 *		previously empty response queue and requests an interrupt to
 *		notify the port driver of the queue transition.
 *
 *		There are several types of responses including:
 *
 *		1. Received packets.
 *		2. Locally initiated port commands with requested statuses.
 *		3. Locally initiated port commands with error statuses.
 *		4. Local port initiated commands with error statuses.
 *		5. Remotely initiated port commands with error statuses.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pb				- Path Block pointer
 *	ppd.cippd		-  CI PPD specific PB fields
 *	    pstatus.path_closed	-   Path already closed by port flag
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.pqb.rspq		-  Port response queue
 *
 *   SMP:	The PCCB is locked for PB retrieval and prevents premature PB
 *		deletion.  This is required only when processing packet
 *		reported errors and when updating cable statuses following the
 *		reception of identification requests and loopback response
 *		packets.  PCCB addresses are always valid because these data
 *		structures are never deleted once their corresponding ports
 *		have been initialized.
 *
 *		PBs are locked to synchronize access and prevent premature
 *		deletion.  This is required only when processing packet
 *		reported errors and when updating cable statuses following the
 *		reception of identification requests.
 *
 *		Access to port response queues is by means of memory
 *		interlocking queuing instructions.
 */
void
np_rsp_handler( pccb )
    PCCB		*pccb;
{
    PB			*pb;
    NPBH		*cibp;
    u_long		save_ipl;

    /* Each response is removed from the specified port's response queue and
     * checked for errors.  These errors fall into one of the following
     * categories based upon the level of error recovery required.
     *
     * 1. Informational Errors.
     *	  The error is processed and response processing continues as if the
     *	  error had never been reported.  This may involve placing the response
     * 	  onto the appropriate free queue if this is what the port would have
     *	  done itself had it not needed to report the error.  Errors classified
     *	  as informational include:
     *
     *		- Cable Failed but Port Command Succeeded.  This status is
     *		  returned whenever a cable fails during the successful
     *		  execution of those local port commands which initiate packet
     *		  transmission and do not explicitly specify the cable to be
     *		  used.  Error processing consists of updating PB cable status
     *		  and logging any good->bad cable transition.  Such processing
     *		  is required only for fully established paths.
     *
     *		- Lack of a Good Cable Forced Local Port Command Failure.  This
     *		  status may be returned by locally executed REQID and SNDLB
     *		  port commands.  Both commands initiate packet transmission
     *		  over specific cables; however, the meaning of this status is
     *		  different for each command as is the error processing on
     *		  detection of this status.
     *
     *		  SNDLB: A NOPATH status indicates failure to loopback to the
     *			 local port over the specified cable.  No error
     *			 processing is required and the packet is immediately
     *			 disposed of.  Note that final disposition of the
     *			 packet is NOT the same as if the error had never
     *			 occurred.  There is no need to queue the packet to
     *			 the appropriate local port datagram free queue for
     *			 reception of a loopback response because no response
     *			 is forthcoming due to failure of the port command.
     *			 Instead the packet is just deallocated.
     *
     *		  REQID: A NOPATH status indicates failure of the specified
     *		  	 cable and is considered only informational in nature
     *		  	 whenever no record of the path currently exists within
     *		  	 the system-wide configuration database, the path is
     *			 not established, or at least one other operational
     *		  	 cable is still known.  Error processing consists of
     *			 updating the PB's cable status, logging any good->bad
     *		  	 cable transition, and forcing transmission of a REQID
     *		  	 over the other cable to determine if it too is bad.
     *			 Such processing is required only for fully established
     *			 paths.
     *
     *		- Unrecognized Packet.  This status is generated whenever the
     *		  port receives a SNDSTRT or SNDRST maintenance packet while in
     *		  the enabled state.  Such statuses are not totally unexpected.
     *		  Remote MSCP clients request resetting and restarting of local
     *		  ports whenever their logical SCS connections to the local
     *		  MSCP server times out and local ports are always to be found
     *		  in the enabled state.  Error processing consists of just
     *		  disposing of the response.
     *
     * 2. Severe Errors.  
     *    The error is logged, the appropriate path is crashed and the response
     *	  is disposed of either by deallocating the buffer( locally executed
     *	  command packets with the R bit set ) or adding it to the appropriate
     *	  port free queue( received packets, local port generated port command
     *	  packets and locally executed port command packets with the R bit
     *	  clear ).  Response processing continues with the next response on the
     *	  queue.  Errors classified as severe include:
     *
     *		- Virtual Circuit Closed by Previous Command.  This status may
     *		  be returned by locally executed port commands which initiate
     *		  packet transmission over a virtual circuit.  The current
     *		  response is just disposed of.  There is no need to log the
     *		  event or crash the path because these actions have already
     *		  taken place.
     *
     *		- Lack of a Good Cable Forced Local Port Command Failure.  This
     *		  status may be returned by any locally executed port commands
     *		  which initiate packet transmission.  The occurrence of such
     *		  an error may close the associated virtual circuit.  The path
     *		  is crashed provided it has not already failed.  Additional
     *		  error processing is also required in the case of those REQIDs
     *		  which had attempted packet transmission over specific cables
     *		  previously thought to be good.  Such processing includes
     *		  updating the PB's cable status and logging any good->bad
     *		  cable transition before logging the error and crashing the
     *		  path.
     *
     *		- Buffer Memory System Error Occurred.  This status may be
     *		  returned within block data transfer packets which return
     *		  status about block data transfer operations.  The occurrence
     *		  of such an error also automatically closes the associated
     *		  virtual circuit.
     *
     *		- Packet Size Violation.  This status may be returned by
     *		  remotely initiated locally executed port commands which
     *		  initiate packet transmission and also within received
     *		  packets.  The occurrence of such an error may also
     *		  automatically close the associated virtual circuit.
     *
     *		- Unrecognized Packet.  This status is generated whenever the
     *		  port receives a packet with an illegal opcode or flags field,
     *		  or an invalid source source address, or a maintenance packet
     *		  when the port is in the enabled state.
     *
     *		- Out-of-Sequence Message Received.  This status may only be
     *		  returned within received packets.  It MAY now be obsolete.
     *		  It is not documented and might just have been used in some
     *		  interim CI ucode for debugging purposes.  Presumably, the
     *		  occurrence of such an error also automatically closes the
     *		  associated virtual circuit.
     *
     *		- Message Received with Closed Virtual Circuit Descriptor.
     *		  This status may only be returned only within received
     *		  packets.  It MAY now be obsolete.  It is not documented and
     *		  might just have been used in some interim CI ucode for
     *		  debugging purposes.  Presumably, the occurrence of such an
     *		  error also automatically closes the associated virtual
     *		  circuit.
     *
     * 3. Fatal Errors.
     *	  The local port is crashed and response processing is immediately
     *	  terminated.  The response is disposed of and any responses remaining
     *	  on the queue are flushed during crashing of the port.  Errors
     *    classified as fatal include:
     *
     *		- Invalid Local Buffer Name.  This status may be returned by
     *		  local block data transfer port commands.  The occurrence of
     *		  such an error also automatically closes the associated
     *		  virtual circuit.
     *
     *		- Local Buffer Length Violation.  This status may be returned
     *		  by local block data transfer port commands.  The occurrence
     *		  of such an error also automatically closes the associated
     *		  virtual circuit.
     *
     *		- Access Control Violation during Local Buffer Access.  This
     *		  status may be returned by local block data transfer port
     *		  commands.  The occurrence of such an error also automatically
     *		  closes the associated virtual circuit.
     *
     *		- Packet Size Violation.  This status may be returned by
     *		  locally executed port commands which initiate packet
     *		  transmission.  In addition, loopback responses( LBREC ) may
     *		  also return this status.  These responses are both locally
     *		  initiated and executed because of how loopback is used by the
     *		  CI port driver.  The occurrence of such an error may also
     *		  automatically close the associated virtual circuit.
     *
     *		- Invalid Destination Port.  This status may be returned by
     *		  those locally and remotely initiated, locally executed port
     *		  commands which require specification of a destination port.
     *		  The occurrence of such an error may also automatically close
     *		  the associated virtual circuit.
     *
     *		- Unknown Local Port Command.  This status is generated
     *		  whenever a local port executes a command with an invalid or
     *		  unimplemented opcode field, invalid flags field, or non-zero
     *		  status field.
     *
     *		- Aborted Local Port Command Encountered.  Packets with this
     *		  status should never be encountered.  Only those port commands
     *		  currently being processed by the port when it transitions
     *		  into the disabled state can have this status.  However once
     *		  a port enters the disabled state it either is in the process
     *		  of being crashed or shortly will be, and all packets on all
     *		  port queues are flushed during crashing of the port.
     *
     *		- All Other( Unknown ) Error Statuses.
     *
     * If a response does not contain any errors, or the error is only
     * informational in nature, then the response is itself processed according
     * to its port operation code.  Processing then continues with the next
     * response on the port's response queue.  There is one exception to this
     * rule: detection of a NOPATH status in a SNDLB packet.  The detection of
     * this informational error forces deallocation of the packet instead of
     * queuing it to the appropriate local port datagram free queue as would
     * have automatically taken place had not the error occurred.  Deallocation
     * of the packet is forced by explicit setting of the response bit within
     * the flags field of the generic Vaxport driver header.
     *
     * Response processing terminates either when this queue is exhausted or a
     * fatal error is detected triggering crashing of the port.  The port is
     * also crashed if the queue interlock can not be obtained.
     *
     * Only cable statuses of open paths are ever updated.
     */

    save_ipl = Splscs();
    Lock_pccb( pccb )
    if( Lpstatus_npisrfork( pccb )) {
	Clear_npisrfork( pccb ) ;
    } else {
	( void )panic( "np_rsp_handler: fork block flag not set" );
    }
    Unlock_pccb( pccb )

    do	{
	Remqh_rspq( pccb, cibp )
	if( cibp == NULL ) {
	    break;
	}
	pccb = pccb->Npadap->pccb[cibp->c_idx];
	if( cibp->status.failure ) {
	    u_long	crash_lport = 0;
	    u_long	dispatch = 0, event = 0;

	    Lock_pccb( pccb )
	    if(( pb = cippd_get_pb( pccb, Pd_to_scs( cibp, pccb ), BUF ))) {
		Lock_pb( pb )
	    }
	    switch( cibp->status.type ) {

		case T_OK:
		    dispatch = 1;
		    if( pb && pb->pinfo.state == PS_OPEN ) {
			( void )np_update_cable( pccb, pb, cibp, CABLE_GB );
		    }
		    break;

		case T_PSVIO:
		    if( cibp->opc == PMREC ) {
			event = SE_INVRPKTSIZE;
			if( cibp->ci_opc != DGREC && pb ) {
			    pb->Fsmpstatus.path_closed = 1;
			}
		    } else {
			crash_lport = 1;
			event = SE_INVLPKTSIZE;
		    }
		    break;

		case T_INVBNAME:
		    crash_lport = 1;
		    event = SE_INVBNAME;
		    break;

		case T_INVBSIZE:
		    crash_lport = 1;
		    event = SE_INVBSIZE;
		    break;

		case T_NOPATH:
		    if( cibp->ci_opc == IDREQ ) {
			dispatch = 1;
			if( Cselect( cibp ) != CS_AUTO ) {
			    if( pb && pb->pinfo.state == PS_OPEN ) {
				( void )np_update_cable( pccb,
							 pb,
							 cibp,
							 CABLE_GB );
				if( pb->Pstatus.cable0 && pb->Pstatus.cable1 ){
				    event = E_NOCABLES;

				/* The response IDREQ packet is not re-used for
				 * the transmission of a REQID over the other
				 * cable.  Its processing must continue in case
				 * it represents the REQID currently being
				 * sanity checked.
				 */
				} else {
				    u_long	save_cable;

				    save_cable = pccb->Poll_cable;
				    if( Cselect( cibp ) == CS_CABLE0 ) {
					pccb->Poll_cable = CS_CABLE1;
				    } else {
					pccb->Poll_cable = CS_CABLE0;
				    }
				    ( void )np_send_reqid( pccb,
							   NULL,
						 	   Get_pgrp( pccb,cibp),
							   DEALLOC_BUF );
				    pccb->Poll_cable = save_cable;
				}
			    }
			} else if( pb && pb->pinfo.state != PS_PATH_FAILURE ) {
			    event = E_NOCABLES;
			}
		    } else if( cibp->ci_opc == LBSNT ) {
			cibp->flags.rsp = 1;
		    } else if( cibp->ci_opc == DGSNT   ||
				cibp->ci_opc == RSTSNT ||
				cibp->ci_opc == STRTSNT ) {
			if( pb && pb->pinfo.state != PS_PATH_FAILURE ) {
			    event = E_NOCABLES;
			}
		    } else if( pb ) {
			pb->Fsmpstatus.path_closed = 1;
			if( pb->pinfo.state != PS_PATH_FAILURE ) {
			    event = E_NOCABLES;
			}
		    }
		    break;

		case T_UPKT:
		    event = SE_UNRECPKT;
		    break;

		case T_VCC:
		    if( cibp->ci_opc == IDREQ ) {
			dispatch = 1;
		    }
		    break;

		case T_DPORT:
		    crash_lport = 1;
		    event = SE_INVDPORT;
		    break;

		case T_UCMD:
		    crash_lport = 1;
		    event = SE_UNKCMD;
		    break;

		case T_INVCGN:
		    crash_lport = 1;
		    event = SE_INVCGN;
		    break;

		case T_INVSN:
		    crash_lport = 1;
		    event = SE_INVSN;
		    break;

		case T_IRESVCD:
		    crash_lport = 1;
		    event = SE_IRESVCD;
		    break;

		case T_IRESEQ:
		    crash_lport = 1;
		    event = SE_IRESEQ;
		    break;

		case T_DISCVCPKT:
		    crash_lport = 1;
		    event = SE_DISCVCPKT;
		    break;

		case T_INVDDL:
		    crash_lport = 1;
		    event = SE_INVDDL;
		    break;

		default:
		    crash_lport = 1;
		    event = SE_UNKSTATUS;
		    break;
	    }
	    if( event ) {
		if( crash_lport == 0 ) {
		    ( void )np_log_packet( pccb, pb, cibp, event, PATH_EVENT );
		    ( void )cippd_crash_pb( pccb,
					    pb,
					    (( Eseverity( event ) == ES_E )
						? E_PD : SE_PD ),
					    0,
					    NULL );
		} else {
		    ( void )np_crash_lport( pccb,
					    event,
					    Pd_to_scs( cibp, pccb ));
					    
		}
	    }
	    if( pb ) {
		Unlock_pb( pb )
	    }
	    Unlock_pccb( pccb )
	    if( crash_lport ) {
    		( void )splx( save_ipl );
		return;
	    }
	    if( !cibp->flags.rsp ) {
		if( cibp->ci_opc == IDREC ||
		     cibp->ci_opc == IDREQ ||
		     cibp->ci_opc == DGSNT ||
		     cibp->ci_opc == DGREC ||
		     cibp->ci_opc == LBSNT ||
		     cibp->ci_opc == LBREC ||
		     ( cibp->status.type == T_UPKT )) {
		    ( void )np_add_dg( pccb, Pd_to_scs( cibp, pccb ));
		} else {
		    ( void )np_add_msg( pccb, Pd_to_scs( cibp, pccb ));
		}
		continue;
	    } else if( !dispatch ) {
		( void )np_dealloc_pkt( cibp, pccb );
		continue;
	    }
	}

	/* Responses with good or informational status are processed according
	 * to their port operation codes as follows:
	 *
	 * Response		Processing
	 *
	 * Received/Transmitted	- SCS processes and disposes of response.
	 *  Sequence Message
	 *
	 * Completed Block Data	- SCS is notified of block data transfer
	 *  Transfer		  completion.
	 *			- Port driver disposes of response.
	 *
	 * Received Datagram 	- SCS processes and disposes of datagram(
	 *			  application datagrams only ).
	 *			- CI PPD processes and disposes of datagram(
	 *			  all other datagrams ).
	 *
	 * Invalidate		- Port driver disposes of response.
	 *  Translation Cache
	 *  Command
	 * Transmitted Datagram
	 * Reset Request
	 * Start Request
	 * Set Circuit Command
	 *
	 * Transmitted		- CI PPD is notified of request identification
	 *  Identification	  command completion.
	 *  Request		- Port driver disposes of response.
	 *
	 * Received		- The appropriate PB's cable status is updated(
	 *  Identification	  open paths only ).
	 *  Packet		- Log all crossed->uncrossed,
	 *			  uncrossed->crossed, and bad->good cable
	 *			  transitions( open paths only ).
	 *			- CI PPD processes and disposes of response.
	 *
	 * Received		- The appropriate PCCB's loopback cable status
	 *  Loopback Response	  is updated.
         *			- Log all loopback bad->good transitions.
	 *			- Port driver disposes of response.
	 *
	 * All other port	- The local port is crashed.
	 *  operation codes	- Response processing is terminated.
	 *
         * Received sequenced messages, datagrams, and block data transfer
         * completion packets are checked for and processed BEFORE dispatching
         * on the operation code contained within the response.  Such special
         * handling results in significantly improved performance.
	 */
	{
	SCSH		*scsbp = Pd_to_scs( cibp, pccb );
	NPPPDH		*cippdbp = Pd_to_ppd( cibp, pccb );
        NPBH		*npbp;
    	RETQEH  	*bp;

	if( cibp->opc == PMREC ) {
	    if( cibp->ci_opc == MSGREC ) {
	        ( void )scs_msg_rec( pccb, scsbp, Appl_size( cippdbp ));
	        continue;
	    } else if( cibp->ci_opc == CNFREC || cibp->ci_opc == DATREC ) {
	        ( void )scs_data_done( pccb, scsbp, &Cnfrec( cippdbp )->xctid );
	    } else if ( cibp->ci_opc == DGREC ) {
	        if( cippdbp->mtype == SCSDG ) {
		    ( void )scs_dg_rec( pccb, scsbp, Appl_size( cippdbp ));
	        } else {
		    ( void )cippd_receive( pccb, cippdbp,
				          ( u_long )cippdbp->mtype );
	        }
	        continue;
	    } else {
	        switch( cibp->ci_opc ) {

		case IDREC:
		    Lock_pccb( pccb )
		    if(( pb = cippd_get_pb( pccb, scsbp, BUF ))) {
			Lock_pb( pb )

			/* Crash the path if the remote port is not in an
			 * appropriate state.
			 */
			if( pb->pinfo.state == PS_OPEN ) {
			    if( Idrec( cippdbp )->port_state != PS_ENAB &&
				Idrec( cippdbp )->port_state != PS_ENAB_MAINT){
				( void )np_log_packet( pccb,
						       pb,
						       cibp,
						       E_RPORTSTATE,
						       PATH_EVENT );
				( void )cippd_crash_pb( pccb,
							pb,
							E_PD,
							0,
							NULL );
				Unlock_pb( pb )
				Unlock_pccb( pccb )
				( void )np_add_dg( pccb, scsbp );
				continue;
			    } else {
				( void )np_update_cable( pccb,
							 pb,
							 cibp,
							 CABLE_CROSSED );
				( void )np_update_cable( pccb,
							 pb,
							 cibp,
							 CABLE_BG );
			    }
			}
			Unlock_pb( pb )
		    }
		    Unlock_pccb( pccb )
		    ( void )cippd_receive( pccb, cippdbp, CNFE_ID_REC );
		    continue;
/* NO longer needed for N_PORT (from silver bl7 merge)
		case LBREC:
		    Lock_pccb( pccb )
		    ( void )np_update_cable( pccb, NULL, cibp, CABLE_LB_BG );
		    Unlock_pccb( pccb )
*/		    break;

		case DATREQ0:	case DATREQ2:
		case MDATREQ: 	case MCNFREC:	case MDATREC:	
		    ( void )np_crash_lport( pccb, SE_INVOPCODE, scsbp );
    		    ( void )splx( save_ipl );
		    return;

		default:
		    ( void )np_crash_lport( pccb, SE_UNKOPCODE, scsbp );
    		    ( void )splx( save_ipl );
		    return;
	        }
	    }
	} else if( cibp->opc == PMSNT ) {
	    switch( cibp->ci_opc ) {

		case DGSNT:
		case RSTSNT:
		case STRTSNT:
		    break;

		case MSGSNT:
		    ( void )scs_msg_snt( pccb, scsbp, Appl_size( cippdbp ));
		    continue;

		case IDREQ:
		    ( void )cippd_reqid_snt( pccb, Get_pgrp( pccb, cibp ));
		    break;

		case LBSNT:	case MDATSNT:
		    ( void )np_crash_lport( pccb, SE_INVOPCODE, scsbp );
    		    ( void )splx( save_ipl );
		    return;

		default:
		    ( void )np_crash_lport( pccb, SE_UNKOPCODE, scsbp );
    		    ( void )splx( save_ipl );
		    return;
	        }
	} else {
	    switch( cibp->opc ) {

		case QPURG:
		case TCINV:
		case CHNLSET:
		case CKTSET:
		    break;

		case QERET:
		    {
		    NPBH	*npbp;
    		    RETQEH  	*bp;

    		    bp = ( RETQEH * )cippdbp;
		    while( bp->qe_ret-- ) {
		        if( Dgfqe( cibp )) {
			    Remqh_dfreeq( pccb, npbp )
		        } else {
			    Remqh_mfreeq( pccb, npbp )
		        }
		        if( npbp ) {
		            ( void )np_dealloc_pkt( npbp, pccb );
		        }
		    }
		    break;
		    }
		case CNTRD:
		    break;

		default:
		    ( void )np_crash_lport( pccb, SE_UNKOPCODE, scsbp );
    		    ( void )splx( save_ipl );
		    return;
	    }
        }
	( void )np_dealloc_pkt( cibp, pccb );
	}
    }	while( 1 );
    ( void )splx( save_ipl );
}

/*   Name:	np_unmapped_isr	- Unmapped CI Adapter Interrupt Service Routine
 *
 *   Abstract:
 *
 *   Inputs:
 *
 *   IPL_CI			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    lpstatus.mapped	-   0
 *	    lpstatus.power	-   Port has power status flag
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   Port is broken status flag
 *	    fsmstatus.online	-   0
 *
 *   Outputs:
 *
 *   IPL_CI			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    lpstatus.power	-   Port has power status flag
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   Port is broken status flag
 *	    fsmstatus.cleanup	-   Cleanup in progress status flag
 *
 *   SMP:
 */
void
np_unmapped_isr( pccb )
    PCCB		*pccb;
{
    u_long		cnfr, event, log_regs = LOG_REGS, save_ipl;

    /* The steps involved in processing an interrupt from an unmapped local CI
     * port are as follows:
     *
     * 1. IPL is synchronized to IPL_SCS.
     * 2. The PCCB is locked.
     * 3. The mapped state of the local port is verified.
     * 4. The local port is mapped.
     * 5. The interrupt is processed according to its nature.
     * 6. The interrupt is logged.
     * 7. The local port is disabled.
     * 8. The PCCB is unlocked.
     * 9. IPL is restored.
     *
     * The local port is always unmapped on entry into this routine; however it
     * may be found to be mapped following locking of the PCCB( Step 2 ).
     * There is only one way this can occur, if another thread were to process
     * an interrupt notifying the port driver of power availability on the
     * local port.  Such processing leaves the local port mapped because the
     * severe error which caused the port to be unmapped in the first place(
     * power loss ) has been alleviated.  It also removes any reason for
     * continued execution of the current thread.   Either the power up
     * interrupt processed by the other thread was originally designated for
     * the current one, or the interrupt designated for the current thread was
     * superseded by the power up interrupt processed by the other one.  In any
     * event, there is no reason for the current thread to continue execution
     * when it finds the local port mapped, and this is why the mapped state of
     * the local port is verified( Step 3 ) after the PCCB is unlocked.
     *
     * This routine services interrupts only when the local port is temporarily
     * without power or marked marked broken and permanently shutdown.  A panic
     * is issued in any other circumstance.
     *
     * All interrupts on broken local ports are considered to be stray.
     * Processing of these stray interrupts includes re-mapping of the local
     * port adapter I/O space( Step 4 ).  This allows access to local port
     * registers for the purpose of logging the interrupt( Step 6 ) and
     * disabling the local port( Step 7 ).  Note that only the adapter I/O
     * space is re-mapped.  The interrupt service handler for the local port is
     * NOT restored to the routine appropriate for the local port hardware port
     * type.  All interrupts on this local port continue to be serviced by this
     * special routine.  Furthermore, this re-mapping is only temporary.  The
     * local port is automatically returned to the unmapped state in which it
     * was found at the beginning of interrupt processing during disablement of
     * the local port( Step 7 ).
     *
     */
    if( pccb == NULL ) {
	return;
    }
    save_ipl = Splscs();
    Lock_pccb( pccb )
    if( pccb->Lpstatus.mapped ) {
	Unlock_pccb( pccb )
	( void )splx( save_ipl );
	return;
    }
    if( pccb->Fsmstatus.broken ) {
	( void )np_map_port( pccb, MAP_REGS );
	event = W_STRAY;
    } else {
	( void )panic( PANIC_BADUNMAP );
    }
    ( void )np_log_dev_attn( pccb, event, LOG_REGS );
    ( void )( *pccb->Disable_port )( pccb, PS_UNINIT );
    Unlock_pccb( pccb )
    ( void )splx( save_ipl );
}

/*   Name:	cimna_isr	- CIMNA Interrupt Service Routine
 *
 *   Abstract:	This routine is the primary interrupt service routine for the
 *		CIMNA local port.  It services all interrupts except when a
 *		local port is marked broken and permanently shutdown.  At such
 *		times the local port is unmapped and all interrupts are
 *		serviced instead by np_unmapped_isr().
 *
 *   Inputs:
 *
 *   IPL_CI			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	lpinfo.type.hwtype	-  HPT_CIMNA
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    lpstatus.mapped	-   1
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   0
 *
 *   Outputs:
 *
 *   IPL_CI			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    ciregptrs.csrcr	-   Port status release control reg pointer
 *	    ic.xmi.xbe		-   XMI bus error register pointer
 *
 *   SMP:	The PCCB is locked to synchronize access to adapter register
 *		contents but only during error processing.  PCCB addresses are
 *		always valid because these data structures are never deleted
 *		once their corresponding ports have been initialized.
 *
 *		No locks are required for normal( ie - non-error ) interrupt
 *		processing for the following reasons:
 *
 *			1. No changes to interlocked structures are made.
 *			2. PCCB references are only to static fields.
 *			3. Port registers either indicate no errors at time of
 *		   	   access or can not change until released.
 */
cimna_isr( pccb )
    PCCB		*pccb;
{
    u_long		save_ipl;
    u_int		lpcrash, csr, xbe; 

    /* The steps involved in servicing CIMNA interrupts are as follows:
     *
     * 1. IPL is synchronized to IPL_SCS.
     * 2. The XMI bus error status register is checked for serious errors.
     * 3. Responses on the port response queue are processed after releasing
     *    the port status register.
     * 4. IPL is restored.
     *
     * CIMNA errors reported within the XMI bus error register include:
     *
     * 1. Soft errors( CRD and CC ).
     * 2. Parity errors( PE ).
     * 3. Node specific errors( NSES ).
     * 4. Miscellaneous errors( WSE, RIDNAK, WDNAK, NRR, RSE, RER, CNAK,
     *    TTO; or undefined errors ).
     *
     * The existence of a node specific error requires querying the port status
     * register in order to determine its exact identity.  CIMNA port status
     * register errors include:
     *
     * 1. Memory system errors.
     * 2. Parity errors( reported within AMCSR ).
     * 3. Sanity time expiration.
     * 4. Data structure error.
     * 5. Message free queue empty.
     * 6. Miscellaneous errors( MISC; or undefined errors ).
     *
     * XMI soft errors are cleared and then ignored.  Processing of the
     * interrupt proceeds as if they had never occurred.  The existence of any
     * other error forces bypassing of response processing and triggers
     * crashing, clean up, and re-initialization of the local port.  The PCCB
     * is locked while any and all errors are being processed.
     *
     * NOTE: Consult the beginning of this module for a more in-depth
     *	     discussion on the processing of interrupts, especially interrupts
     *	     to report errors, in SMP environments.
     */
    save_ipl = Splscs();
#ifdef __alpha
    MB();
    xbe = RDCSR(LONG_32,pccb->Bus,pccb->Mna_ber);
    csr = RDCSR(LONG_32,pccb->Bus,pccb->Csr);
#else
    csr = *pccb->Csr;
    xbe = *pccb->Mna_ber;
#endif
    if(( xbe & CIMNA_XBE_ERRS ) || ( csr & CSR_ERRS )) {
	Lock_pccb( pccb )
	if((( xbe & CIMNA_XBE_HARDE ) > 0 ) || ( csr & CSR_CME )) {
	    if(( xbe & XMI_PE ) || ( xbe & XMI_NSES )) {
	        lpcrash = SE_PARITY;
            } else {
	        lpcrash = SE_PORTERROR;
            }
	} else if( csr & CSR_ERRS ) {
	    if( csr & CSR_STE ) { 
		lpcrash = SE_SANITYTIMER;
	    } else if( csr & CSR_DSE ) {
		lpcrash = SE_DSE;
	    } else if( csr & CSR_MSE ) {
		lpcrash = SE_MSE;
	    } else if( csr & CSR_AMFQE ) {
		lpcrash = SE_MFQE;
	    } else if( csr & CSR_ADFQE ) {
		lpcrash = SE_DFQE;
	    } else {
		lpcrash = SE_PORTERROR;
	    }
	} else  { /* soft error */
#ifdef __alpha
	    WRTCSR(LONG_32,pccb->Bus,pccb->Csrcr,CSRCR_CSRC);
#else
	    *pccb->Csrcr = CSRCR_CSRC;
#endif
	    Unlock_pccb( pccb )
	    lpcrash = 0;
	}
	if( lpcrash ) {
	    ( void )np_crash_lport( pccb, lpcrash, NULL );
	    Unlock_pccb( pccb )
	    ( void )splx( save_ipl );
	    return;
	}
    } else if ( csr == NULL ) {
#ifdef __alpha
        WRTCSR(LONG_32,pccb->Bus,pccb->Csrcr,CSRCR_CSRC);
#else
        *pccb->Csrcr = CSRCR_CSRC;
#endif
	Npstart_isrfp( pccb ) ;
    }
    ( void )splx( save_ipl );
}

/*   Name:	citca_isr	- CITCA Interrupt Service Routine
 *
 *   Abstract:	This routine is the primary interrupt service routine for the
 *		CITCA local port.  It services all interrupts except when a
 *		local port is marked broken and permanently shutdown.  At such
 *		times the local port is unmapped and all interrupts are
 *		serviced instead by np_unmapped_isr().
 *
 *   Inputs:
 *
 *   IPL_CI			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	lpinfo.type.hwtype	-  HPT_CITCA
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    lpstatus.mapped	-   1
 *	ppd.cippd		-  CI PPD specific PCCB fields
 *	    fsmstatus.broken	-   0
 *
 *   Outputs:
 *
 *   IPL_CI			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.gvp.type.ci		-  CI specific PCCB fields
 *	    ciregptrs.csrcr	-   Port status release control reg pointer
 *	    ic.tc.tber		-   TC bus error register pointer
 *
 *   SMP:	The PCCB is locked to synchronize access to adapter register
 *		contents but only during error processing.  PCCB addresses are
 *		always valid because these data structures are never deleted
 *		once their corresponding ports have been initialized.
 *
 *		No locks are required for normal( ie - non-error ) interrupt
 *		processing for the following reasons:
 *
 *			1. No changes to interlocked structures are made.
 *			2. PCCB references are only to static fields.
 *			3. Port registers either indicate no errors at time of
 *		   	   access or can not change until released.
 */
citca_isr( pccb )
    PCCB		*pccb;
{
    u_int		lpcrash, csr, tber, save_ipl;

    /* The steps involved in servicing CITCA interrupts are as follows:
     *
     * 1. IPL is synchronized to IPL_SCS.
     * 2. The TC bus error status register is checked for serious errors.
     * 3. Responses on the port response queue are processed after releasing
     *    the port status register.
     * 4. IPL is restored.
     *
     * CITCA errors reported within the TC bus error register include:
     *
     * 1. Parity errors( MVATPE, CMDTPE, RSPTPE ).
     * 2. Node specific errors( NSES; reported in AMCSR ).
     * 3. TC errors( MVAMAC, MVATER, MVBMAC, MVBTER, CMDMAC, CMDTER ).
     *
     * The existence of a node specific error requires querying the port status
     * register in order to determine its exact identity.  CITCA port status
     * register errors include:
     *
     * 1. Memory system errors.
     * 2. Parity errors( reported within AMCSR ).
     * 3. Sanity time expiration.
     * 4. Data structure error.
     * 5. Message free queue empty.
     * 6. datagram free queue empty.
     * 7. Abnormal condition errors( AC; or undefined errors ).
     *
     * TC soft errors are cleared and then ignored.  Processing of the
     * interrupt proceeds as if they had never occurred.  The existence of any
     * other error forces bypassing of response processing and triggers
     * crashing, clean up, and re-initialization of the local port.  The PCCB
     * is locked while any and all errors are being processed.
     *
     * NOTE: Consult the beginning of this module for a more in-depth
     *	     discussion on the processing of interrupts, especially interrupts
     *	     to report errors, in SMP environments.
     */
    save_ipl = Splscs();
    csr = *pccb->Csr;
    if( csr & CSR_ERRS ) {
	Lock_pccb( pccb )
        csr = *pccb->Csr;
	if( csr & CSR_CME ) {
            tber = *pccb->Tca_ber;
	    if(( tber & CITCA_TC_PE ) || ( tber & CITCA_TC_NSES )) {
	        lpcrash = SE_PARITY;
            } else {
	        lpcrash = SE_PORTERROR;
            }
	} else {
	    if( csr & CSR_STE ) { 
		lpcrash = SE_SANITYTIMER;
	    } else if( csr & CSR_DSE ) {
		lpcrash = SE_DSE;
	    } else if( csr & CSR_MSE ) {
		lpcrash = SE_MSE;
	    } else if( csr & CSR_AMFQE ) {
		lpcrash = SE_MFQE;
	    } else if( csr & CSR_ADFQE ) {
		lpcrash = SE_DFQE;
	    } else if( csr & CSR_CAC ) {
		lpcrash = SE_PORTERROR;
	    } else if( csr & CSR_UNIN ) {
		lpcrash = SE_PORTERROR;
	    } else {
	        Unlock_pccb( pccb )
		lpcrash = 0;
	    }
	}
	if( lpcrash ) {
	    ( void )np_crash_lport( pccb, lpcrash, NULL );
	    Unlock_pccb( pccb )
	    ( void )splx( save_ipl );
	    return;
	}
    }
    if( csr & CSR_ASIC ) {
        *pccb->Csrcr = CSRCR_CSRC;
	Npstart_isrfp( pccb ) ;
    }
    ( void )splx( save_ipl );
}

/*   Name:	cirsp_isr	- CI Responce Interrupt Service Routine
 *
 *   Abstract:	This routine is the responce ONLY interrupt service routine 
 *		for the CI local port. It services all responce interrupts.
 *		All error interrupts are handled by the specific local
 *		interrupt routine.
 *
 *		This routine is used for OSF ISR thread start up if
 *		dsaisr_thread_off = 0, otherwise call the np_rsp_handler
 *		directly.
 *
 *		This routine address will be place in the busses interrupt
 *		vector area for handling responce interrupts ONLY.
 *		No registers need be read thus NO mailbox reads occur
 *		on ALPHA systems. Performance is increase because of this.
 *
 *		Ports that have responce interrupts are CIMNA.
 *
 *   Inputs:
 *
 *   IPL_CI			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *
 *   Outputs:
 *
 *   IPL_CI			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.np.type.ci		-  CI specific PCCB fields
 *
 *   SMP: 	No locks are required for normal( ie - non-error ) interrupt
 *		processing for the following reasons:
 *
 *			1. No changes to interlocked structures are made.
 *			2. PCCB references are only to static fields.
 *			3. Port registers either indicate no errors at time of
 *		   	   access or can not change until released.
 */
cirsp_isr( pccb )
    PCCB		*pccb;
{
/* It is not necessary to write the CSRCR register for CIMNA completion 
   interrupts. However, this write is necessary for turbochannel if CITCA
   gets supported on Flamingo */

    Npstart_isrfp( pccb ) ;
}

