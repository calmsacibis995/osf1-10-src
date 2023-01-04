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
static char *rcsid = "@(#)$RCSfile: np_dg.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 17:45:33 $";
#endif
/*
 * derived from np_dg.c	5.2	(ULTRIX)	10/16/91";
 */
/************************************************************************
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		datagram communication service functions.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Function/Routines:
 *
 *   np_alloc_dg		Allocate Datagram Buffer
 *   np_dealloc_dg		Deallocate Datagram Buffer
 *   np_add_dg			Add Datagram Buffer to Free Queue
 *   np_remove_dg		Remove Datagram Buffer from Free Queue
 *   np_send_dg			Send Datagram
 *
 *   Modification History:
 *
 *   31-Oct-1991	Peter Keilty
 *	Ported to OFS/1
 *	Changed memeory allocation to OSF/1 memory zones. 
 *	Routines are in scs_subr.c.
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
#include		<machine/pmap.h>
#include		<machine/cpu.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/np/npport.h>
#include		<io/dec/uba/uqscs.h>
#include		<io/dec/np/npadapter.h>

#ifdef __alpha
#include		<io/common/devdriver.h>
#include		<io/dec/mbox/mbox.h>
#endif

/* External Variables and Routines.
 */
extern	NPBH 		*np_alloc_pkt();

/*   Name:	np_alloc_dg	- Allocate Datagram Buffer
 *
 *   Abstract:	This function allocates a port specific datagram buffer from
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
 *   Address of SCS header in datagram buffer on success
 *   Otherwise NULL
 *
 *   SMP:	No locks are required.  PCCB addresses are always valid
 *		allowing access to static fields because these data structures
 *		are never deleted once their corresponding ports have been
 *		initialized.
 */
SCSH *
np_alloc_dg( pccb )
    PCCB		*pccb;
{
    HDR			*npbhp;

    SCA_KM_ALLOC( npbhp, HDR *, pccb->lpinfo.Dg_size, KM_SCABUF, KM_NOWAIT )
    if( npbhp ) {
        CARRIERH	*carhp;
        CARRIER		*carp;

	Alloc_np_carrier( carhp, carp, pccb )
        if( carhp == NULL ) {
	    SCA_KM_FREE(( char * )npbhp, pccb->lpinfo.Dg_size, KM_SCABUF )
	    return( NULL );
        }
	npbhp->carh_ptr = carhp;
	npbhp->size = ( u_int )( pccb->lpinfo.Dg_size );
	npbhp->type = ( u_int )( DYN_NPDG << 16 );
	return( Pd_to_scs( Nphd_to_npbp( npbhp ), pccb ));
    } else {
	return( NULL );
    }
}

/*   Name:	np_dealloc_dg	- Deallocate Datagram Buffer
 *
 *   Abstract:	This function deallocates a port specific datagram buffer to
 *		dynamic kernel memory.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *   scsbp			- Address of SCS header in datagram buffer
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
np_dealloc_dg( pccb, scsbp )
    PCCB		*pccb;
    SCSH		*scsbp;
{
    NPBH		*npbp = Scs_to_pd( scsbp, pccb );
    CARRIERH		*carhp  = Npbp_to_nphd( npbp )->carh_ptr;

    SCA_KM_FREE(( char * )carhp, carhp->Chdr_size, KM_SCABUF )
    SCA_KM_FREE(( char * )Npbp_to_nphd( npbp ), Npbp_to_nphd( npbp)->size, 
			  KM_SCABUF )
}

/*   Name:	np_add_dg	- Add Datagram Buffer to Free Queue
 *
 *   Abstract:	This function adds a port specific datagram buffer to a
 *		specific port's datagram free queue and notifies the port when
 *		the queue was previously empty.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *   scsbp			- Address of SCS header in datagram buffer
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
np_add_dg( pccb, scsbp )
    PCCB		*pccb;
    SCSH		*scsbp;
{
    NPBH		*npbp = Scs_to_pd( scsbp, pccb );

    Insqt_dfreeq( npbp, pccb )
}

/*   Name:	np_remove_dg	- Remove Datagram Buffer from Free Queue
 *
 *   Abstract:	This function removes a port specific datagram buffer from a
 *		specific port's datagram free queue.
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
 *   Address of SCS header in removed datagram buffer if successful
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
np_remove_dg( pccb )
    PCCB		*pccb;
{
    NPBH		*npbp;
    RETQEH     		*bp;

    if( npbp = np_alloc_pkt( pccb )) {
	bp = ( RETQEH * )Pd_to_ppd( npbp, pccb );
	bp->qe_req = 1;
        Format_nph( pccb, npbp, RETQE, 0, 0, DEALLOC_BUF )
        Dgfqe( npbp ) = 1;
   	Insqt_dccq1( npbp, pccb )
        return(( SCSH * )RET_SUCCESS );
    }
    return( NULL );
}

/*   Name:	np_send_dg	- Send Datagram
 *
 *   Abstract:	This function initiates transmission of a port specific
 *		datagram over a specific path.  Transmission is initiated by
 *		placing a SNDDG command packet onto the second highest priority
 *		port command queue and notifying the port when the queue was
 *		previously empty.
 *
 *		Two options exist for disposal of the buffer following
 *		transmission of the datagram:
 *
 *		1. Add the buffer to the port's datagram free queue.
 *		2. Deallocate the buffer.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   disposal			- DEALLOC_BUF or RECEIVE_BUF
 *   mtype			- SCSDG, START, STACK, ACK, or STOP
 *   pb				- Path Block pointer
 *   pccb			- Port Command and Control Block pointer
 *   scsbp			- Address of SCS header in datagram buffer
 *   size			- Size of application data
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	No locks are required; however, the PB must EXTERNALLY be
 *		prevented from deletion.  PCCBs addresses are always valid
 *		because these data structures are never deleted once their
 *		corresponding ports have been initialized.
 *
 *		Locks lower than the PCCB in the SCA locking hierarchy may NOT
 *		be held EXTERNALLY without also holding the PCCB lock in case
 *		the port requires crashing.
 */
void
np_send_dg( pccb, pb, scsbp, size, disposal, mtype )
    PCCB		*pccb;
    PB			*pb;
    SCSH		*scsbp;
    u_long		size;
    u_long		disposal;
    u_long		mtype;
{
    NPBH		*npbp;
    NPPPDH		*npppdbp;

    /* Both the port-to-port and nport driver header must be
     * initialized before initiating datagram transmission.
     */
    npppdbp = Scs_to_ppd( scsbp );
    Format_npppdh( npppdbp, mtype, size )
    npbp = Ppd_to_pd( npppdbp, pccb );
    Format_nph( pccb, npbp, SNDPM, SNDDG, Scaaddr_low( pb->pinfo.rport_addr ),
                 disposal )
    Insqt_dccq1( npbp, pccb )
}
