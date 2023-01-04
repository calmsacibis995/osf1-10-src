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
static char *rcsid = "@(#)$RCSfile: np_msg.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 17:47:04 $";
#endif
/*
 * derived from np_msg.c	5.2	(ULTRIX)	10/16/91";
 */
/************************************************************************
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		message communication service functions.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Function/Routines:
 *
 *   np_alloc_msg		Allocate Message Buffer
 *   np_dealloc_msg		Deallocate Message Buffer
 *   np_add_msg			Add Message Buffer to Free Queue
 *   np_remove_msg		Remove Message Buffer from Free Queue
 *   np_send_msg		Send Message
 *
 *   Modification History:
 *
 *   31-Oct-1991	Peter Keilty
 *	Ported to OFS/1
 *	Changed memeory allocation to OSF/1 memory zones. 
 *	Routines are in scs_subr.c.
 *
 *
 *   16-Oct-1991	Brian Nadeau
 *	Updates/bug fixes.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<sys/systm.h>
#include		<sys/vm.h>
#include		<dec/binlog/errlog.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/np/npport.h>
#include		<io/dec/np/npadapter.h>

#ifdef __alpha
#include		<io/common/devdriver.h> 
#include		<io/dec/mbox/mbox.h>
#endif

/* External Variables and Routines.
 */
extern  NPBH		*np_alloc_pkt();

/*   Name	np_alloc_msg	- Allocate Message Buffer
 *
 *   Abstract:	This function allocates a port specific message buffer from
 *		dynamic kernel memory.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   Return Values:
 *
 *   Address of SCS header in message buffer on success
 *   Otherwise NULL
 *
 *   SMP:	No locks are required.  PCCB addresses are always valid
 *		allowing access to static fields because these data structures
 *		are never deleted once their corresponding ports have been
 *		initialized.
 */
SCSH *
np_alloc_msg( pccb )
    PCCB		*pccb;
{
    HDR			*npbhp;

    SCA_KM_ALLOC( npbhp, HDR *, pccb->lpinfo.Msg_size, KM_SCABUF, KM_NOWAIT )
    if( npbhp ) {
        CARRIERH	*carhp;
        CARRIER		*carp;

	Alloc_np_carrier( carhp, carp, pccb )
        if( carhp == NULL ) {
	    SCA_KM_FREE(( char * )npbhp, pccb->lpinfo.Msg_size, KM_SCABUF )
	    return( NULL );
        }
	npbhp->carh_ptr = carhp;
	npbhp->size = ( u_int )( pccb->lpinfo.Msg_size );
	npbhp->type = ( u_int )( DYN_NPMSG << 16 );
	return( Pd_to_scs( Nphd_to_npbp( npbhp ), pccb ));
    } else {
	return( NULL );
    }
}

/*   Name:	np_dealloc_msg	- Deallocate Message Buffer
 *
 *   Abstract:	This function deallocates a port specific message buffer to
 *		dynamic kernel memory.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *   scsbp			- Address of SCS header in message buffer
 *
 *   Outputs:	
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	No locks are required.  PCCB addresses are always valid
 *		allowing access to static fields because these data structures
 *		are never deleted once their corresponding ports have been
 *		initialized.
 */
void
np_dealloc_msg( pccb, scsbp )
    PCCB		*pccb;
    SCSH		*scsbp;
{
    NPBH		*npbp = Scs_to_pd( scsbp, pccb );
    CARRIERH		*carhp = Npbp_to_nphd( npbp )->carh_ptr;

    SCA_KM_FREE(( char * )carhp, carhp->Chdr_size, KM_SCABUF )
    SCA_KM_FREE(( char * )Npbp_to_nphd( npbp ), Npbp_to_nphd( npbp)->size,
			  KM_SCABUF )
}

/*   Name:	np_add_msg	- Add Message Buffer to Free Queue
 *
 *   Abstract:	This function adds a port specific message buffer to a
 *		specific port's message free queue and notifies the port when
 *		the queue was previously empty.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *   scsbp			- Address of SCS header in message buffer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	No locks are required.  PCCB addresses are always valid
 *		allowing access to static fields because these data structures
 *		are never deleted once their corresponding ports have been
 *		initialized.
 *
 *		Locks lower than the PCCB in the SCA locking hierarchy may NOT
 *		be held EXTERNALLY without also holding the PCCB lock in case
 *		the port requires crashing.
 */
void
np_add_msg( pccb, scsbp )
    PCCB		*pccb;
    SCSH		*scsbp;
{
    NPBH		*npbp = Scs_to_pd( scsbp, pccb );

    Insqt_mfreeq( npbp, pccb )
}

/*   Name:	np_remove_msg	- Remove Message Buffer from Free Queue
 *
 *   Abstract:	This function removes a port specific message buffer from a
 *		specific port's message free queue.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   Return Values:
 *
 *   Address of SCS header of removed message buffer if successful
 *   Otherwise NULL
 *
 *   SMP:	No locks are required.  PCCB addresses are always valid
 *		allowing access to static fields because these data structures
 *		are never deleted once their corresponding ports have been
 *		initialized.
 *
 *		Locks lower than the PCCB in the SCA locking hierarchy may NOT
 *		be held EXTERNALLY without also holding the PCCB lock in case
 *		the port requires crashing.
 */
SCSH *
np_remove_msg( pccb )
    PCCB		*pccb;
{
    NPBH		*npbp;
    RETQEH    		*bp;

    if( npbp = np_alloc_pkt( pccb )) {
	bp = ( RETQEH * )Pd_to_ppd( npbp, pccb );
	bp->qe_req = 1;
        Format_nph( pccb, npbp, RETQE, 0, 0, DEALLOC_BUF )
    	Insqt_dccq1( npbp, pccb )
	return(( SCSH * )RET_SUCCESS );
    }
    return( NULL );
}

/*   Name:	np_send_msg	- Send Message
 *
 *   Abstract:	This function initiates transmission of a port specific message
 *		over a specific path.  Transmission is initiated by placing a
 *		SNDMSG command packet onto the second highest priority port
 *		command queue and notifying the port when the queue was
 *		previously empty.
 *
 *		Two options exist for disposal of the buffer following
 *		transmission of the message:
 *
 *		1. Add the buffer to the port's message free queue.
 *		2. Return the buffer to SCS for deallocation.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   disposal			- DEALLOC_BUF or RECEIVE_BUF
 *   pb				- Path Block pointer
 *   pccb			- Port Command and Control Block pointer
 *   scsbp			- Address of SCS header in message buffer
 *   size			- Size of application data
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	No locks are required; however, the PB must EXTERNALLY be
 *		prevented from deletion.  PCCB addresses are always valid
 *		because these data structures are never deleted once their
 *		corresponding ports have been initialized.
 *
 *		Locks lower than the PCCB in the SCA locking hierarchy may NOT
 *		be held EXTERNALLY without also holding the PCCB lock in case
 *		the port requires crashing.
 */
void
np_send_msg( pccb, pb, scsbp, size, disposal )
    PCCB		*pccb;
    PB			*pb;
    SCSH		*scsbp;
    u_long		size;
    u_long		disposal;
{
    NPBH		*npbp;
    NPPPDH		*npppdbp;

    /* Both the port-to-port and N_port driver header must be
     * initialized before initiating message transmission.
     */
    npppdbp = Scs_to_ppd( scsbp );
    Format_npppdh( npppdbp, SCSMSG, size )
    npbp = Ppd_to_pd( npppdbp, pccb );
    Format_nph( pccb, npbp, SNDPM, SNDMSG, 
		Scaaddr_low( pb->pinfo.rport_addr ), disposal )
    Insqt_dccq1( npbp, pccb )
}
