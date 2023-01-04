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
static char *rcsid = "@(#)$RCSfile: scs_maint.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/07/13 15:23:06 $";
#endif
/*
 *	scs_maint.c	4.1  (ULTRIX)        7/2/90
 */

/*
 *
 *   Facility:	Systems Communication Architecture
 *		Systems Communication Services
 *
 *   Abstract:	This module contains Systems Communication Services( SCS )
 *		maintenance service functions.
 *
 *   Creator:	Todd M. Katz	Creation Date:	May 27, 1985
 *
 *   Function/Routines:
 *   
 *   scs_crash_path		Crash Path
 *   scs_reset			Reset Remote Port and System
 *   scs_restart		Restart Remote Port and System
 *   scs_shutdown		Inform Known Systems of Local Shutdown
 *
 *   Modification History:
 *
 *   15-Mar-1991	Brian Nadeau
 *	Port to OSF
 *
 *   06-Apr-1989	Pete Keilty
 *	Added include file smp_lock.h
 *
 *   13-Feb-1989	Todd M. Katz		TMK0004
 *	1. Modify TMK0003 to use the shorthand notation Lproc_name when
 *	   refering to the corresponding MSB field.
 *	2. Include header file ../vaxmsi/msisysap.h.
 *
 *   20-Aug-1988	Todd M. Katz		TMK0003
 *	1. SCA event codes have been completed revised.  All former SCS path
 *	   crash codes are now defined as either error events or severe error
 *	   events.  The path crash attribute is only applied by the individual
 *	   port driver routines responsible for crashing paths and only when
 *	   the crashed path is currently open.
 *	2. Modify scs_path_crash() to pass the name of the local SYSAP
 *	   responsible for crashing the path to the appropriate port specific
 *	   routine.  The local SYSAP is required to pass its name within a new
 *	   MSB field( lproc_name ).  This name is passed to the appropriate
 *	   port specific routine in place of the SCS header of a
 *	   datagram/message buffer, the normal occupant of that arguement
 *	   position.
 *
 *   24-Jun-1988	Todd M. Katz		TMK0002
 *	Due to a typo scs_crash_path() was not returning the status
 *	RET_INVPSTATE when directed to crash existing paths in inappropriate
 *	states.  It does now.
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
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>

/* External Variables and Routines.
 */
extern	struct slock	lk_scadb;
extern	pccbq		scs_lport_db;

/*   Name:	scs_crash_path	- Crash Path
 *
 *   Abstract:	This function initiates port specific termination of a specific
 *		path to a known system.  The path is disabled, cleaned up, and
 *		all SYSAPs with connections established across the explicitly
 *		failed path are notified of its failure.  Notification occurs
 *		through asynchronous invocation of their connections' control
 *		event routines.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   msb			- Maintenance Services Block pointer
 *	lport_name		-  Local port device name
 *	ov1.lproc_name		-  Local SYSAP name( blank filled )
 *	rport_addr		-  Remote port station address
 *   lk_scadb			- SCA database lock structure
 *   scs_lport_db		- System-wide local port database queue head
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Initiated path termination
 *   RET_INVPSTATE		- Path in invalid state
 *   RET_NOPATH			- Path not exist
 *
 *   SMP:	The SCA database is locked to postpone potential modifications
 *		to the system-wide local port database while it is being
 *		traversed.  It also allows PB retrieval while preventing
 *		premature PB deletion as required by PD routines which crash
 *		paths.
 */
u_long
scs_crash_path( msb )
    MSB	*msb;
{
    PB		*pb;
    pccbq	*qp;
    PCCB	*pccb;
    u_long	status = RET_NOPATH;

    /* The steps involved in crashing a path are as follows:
     *
     * 1. Lock the SCA database.
     * 2. Search the system-wide local port database for the specified port.
     * 3. Retrieve the PB for the specified path.
     * 4. Invoke a PD specific routine to initiate crashing of the path.
     * 5. Unlock the SCA database.
     * 6. Return an appropriate status.
     */
    Lock_scadb();
    for( qp = scs_lport_db.flink; qp != &scs_lport_db; qp = qp->flink ) {
	pccb = Pos_to_pccb( qp, flink );
	if( pccb->lpinfo.name == msb->lport_name ) {
	    if(( pb = ( *pccb->Get_pb )( pccb,
					 ( SCSH * )&msb->rport_addr,
					 NO_BUF ))) {
		if( pb->pinfo.state == PS_OPEN ||
		     pb->pinfo.state == PS_PATH_FAILURE ) {
		    ( void )( *pb->Crash_path )( pccb,
						 pb,
						 E_SYSAP,
						 RETURN_BUF,
						 ( SCSH * )msb->Lproc_name );
		    status = RET_SUCCESS;
		} else {
		    status = RET_INVPSTATE;
		}
	    }
	    break;
	}
    }
    Unlock_scadb();
    return( status );
}

/*   Name:	scs_reset	- Reset Remote Port and System
 *
 *   Abstract:	This function initiates the resetting of a remote port and
 *		system in a port specific fashion.  The remote system does not
 *		have to be known.  Such resettings are best efforts.  Remote
 *		ports and systems are not guaranteed to have been reset.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   msb			- Maintenance Services Block pointer
 *	ov1.force		-  Force resetting of remote port and system
 *	lport_name		-  Local port device name
 *	rport_addr		-  Remote port station address
 *   lk_scadb			- SCA database lock structure
 *   scs_lport_db		- System-wide local port database queue head
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Initiated remote port and system resetting
 *   RET_ALLOCFAIL		- Failed to allocate a local port buffer
 *   RET_INVLPSTATE		- Local port in invalid state
 *   RET_NOLPORT		- Local port not exist
 *   RET_NOPATH			- Invalid remote port station address
 *
 *   SMP:	The SCA database is locked to postpone potential modifications
 *		to the system-wide local port database while it is being
 *		traversed.
 */
u_long
scs_reset( msb )
    MSB	*msb;
{
    pccbq	*qp;
    PCCB	*pccb;
    u_long	status = RET_NOLPORT;

    /* The resetting of a remote port and system proceeds as follows:
     *
     * 1. Lock the SCA database.
     * 2. Search the system-wide local port database for the target port.
     * 4. Invoke a PD specific function to initiate resetting of the remote
     *	  port and system.
     * 5. Unlock the SCA database.
     * 6. Return an appropriate status.
     */
    Lock_scadb();
    for( qp = scs_lport_db.flink; qp != &scs_lport_db; qp = qp->flink ) {
	pccb = Pos_to_pccb( qp, flink );
	if( pccb->lpinfo.name == msb->lport_name ) {
	    status = ( *pccb->Remote_reset )( pccb,
					      &msb->rport_addr,
					      msb->Force );
	    break;
	}
    }
    Unlock_scadb();
    return( status );
}

/*   Name:	scs_restart	- Restart Remote Port and System
 *
 *   Abstract:	This function initiates the restarting of a remote port and
 *		system in a port specific fashion.  A reset must have been
 *		previously issued from the the local to the remote port.
 *
 *		The remote system does not have	to be known.  Such restartings
 *		are best efforts.  Remote ports and systems are not guaranteed
 *		to have been restart.
 *
 *		The remote system may optionally be started at a SYSAP supplied
 *		address.  Otherwise, the remote system is started at its
 *		default start address.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   msb			- Maintenance Services Block pointer
 *	lport_name		-  Local port device name
 *	ov1.startaddr		-  Remote system starting address( OPTIONAL )
 *	rport_addr		-  Remote port station address
 *   lk_scadb			- SCA database lock structure
 *   scs_lport_db		- System-wide local port database queue head
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Initiated remote port and system restarting
 *   RET_ALLOCFAIL		- Failed to allocate a local port buffer
 *   RET_INVLPSTATE		- Local port in invalid state
 *   RET_NOLPORT		- Local port not exist
 *   RET_NOPATH			- Invalid remote port station address
 *
 *   SMP:	The SCA database is locked to postpone potential modifications
 *		to the system-wide local port database while it is being
 *		traversed.
 */
u_long
scs_restart( msb )
    MSB	*msb;
{
    pccbq	*qp;
    PCCB	*pccb;
    u_long	status = RET_NOLPORT;

    /* The restarting of a remote port and system proceeds as follows:
     *
     * 1. Lock the SCA database.
     * 2. Search the system-wide local port database for the target port.
     * 4. Invoke a PD specific function to initiate restarting of the remote
     *	  port and system.
     * 5. Unlock the SCA database.
     * 6. Return an appropriate status.
     */
    Lock_scadb();
    for( qp = scs_lport_db.flink; qp != &scs_lport_db; qp = qp->flink ) {
	pccb = Pos_to_pccb( qp, flink );
	if( pccb->lpinfo.name == msb->lport_name ) {
	    status = ( *pccb->Remote_start )( pccb,
					      &msb->rport_addr,
					      msb->Startaddr );
	    break;
	}
    }
    Unlock_scadb();
    return( status );
}

/*   Name:	scs_shutdown	- Inform Known Systems of Local Shutdown
 *
 *   Abstract:	This function initiates notification of all known systems of
 *		local system shutdown.  Such notifications are best efforts.
 *		Systems are not guaranteed to have been notified.
 *
 *   Inputs:
 *
 *   IPL_POWER			- Interrupt processor level
 *   scs_lport_db		- System-wide local port database queue head
 *
 *   Outputs:
 *
 *   IPL_POWER			- Interrupt processor level
 *
 *   SMP:	No locks are required.  This routine is only called from
 *		panic() and it is invoked at maximum IPL.  Only the processor
 *		executing this code is operational and this routine can not be
 *		interrupted on this processor.  This guarantees uncompromised
 *		access to the system-wide local port database without locking
 *		the SCA database.
 */
void
scs_shutdown()
{
    pccbq	*qp;
    PCCB	*pccb;
    void	( *down )();

    /* Traverse the system-wide local port database and invoke each local
     * port's PD specific routine to notify those remote systems with paths
     * originating at the port of local system shutdown.
     */
    for( qp = scs_lport_db.flink; qp != &scs_lport_db; qp = qp->flink ) {
	pccb = Pos_to_pccb( qp, flink );
	if(( down = pccb->Shutdown )) {
	    ( void )( *down )( pccb );
	}
    }
}
