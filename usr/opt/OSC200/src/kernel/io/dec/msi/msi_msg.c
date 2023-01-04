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
static char *rcsid = "@(#)$RCSfile: msi_msg.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 17:41:21 $";
#endif
/*
 * derived from msi_msg.c	4.1	(ULTRIX)	7/2/90";
 */
/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1989                              *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************
 *
 *
 *   Facility:	Systems Communication Architecture
 *		Mayfair Storage Interconnect Port Driver
 *
 *   Abstract:	This module contains Mayfair Storage Interconnect Port
 *		Driver( MSI ) message communication service functions
 *		and routines.
 *
 *   Creator:	Todd M. Katz	Creation Date:	December 21, 1988
 *
 *   Function/Routines:
 *
 *   msi_alloc_msg		Allocate MSI Port Specific Message Buffer
 *   msi_dealloc_msg		Deallocate MSI Port Specific Message Buffer
 *   msi_add_msg		Add MSI Port Message Buffer to Free Queue
 *   msi_remove_msg		Remove MSI Port Message Buffer from Free Queue
 *   msi_send_msg		Send MSI Port Specific Message
 *
 *   Modification History:
 *
 *   31-Oct-1991	Pete Keilty
 *	Ported to OSF/1.
 *	Changed memeory allocation to OSF/1 memory zones. 
 *	Routines are in scs_subr.c.
 *
 *   14-Jun-1989	Pete Keilty
 *	Add include file smp_lock.h
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
extern  int		shutting_down;
extern  int		dsaisr_thread_init, dsaisr_thread_off;
extern  fbq		dsa_fbq;
extern	void		msi_xfp(), dsaisr_thread();
extern  caddr_t		sca_zalloc(), sca_zget();
extern  int		sca_zfree(), sca_zones_init(), sca_zones_initialized;
extern  struct zone 	*sca_zone[];

/*   Name:	msi_alloc_msg	- Allocate MSI Port Specific Message Buffer
 *
 *   Abstract:	This function allocates a MSI port specific message buffer
 *		from dynamic kernel memory.
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
msi_alloc_msg( pccb )
    PCCB		*pccb;
{
    MSIB		*msibp;

    SCA_KM_ALLOC( msibp, MSIB *, pccb->lpinfo.Msg_size, KM_SCABUF, KM_NOWAIT )
    if( msibp ) {
	Format_msib( msibp, pccb->lpinfo.Msg_size, DYN_MSIMSG )
	return(( SCSH * )msibp->Msg.text );
    } else {
	return( NULL );
    }
}

/*   Name:	msi_dealloc_msg	- Deallocate MSI Port Specific Message Buffer
 *
 *   Abstract:	This function deallocates a MSI port specific message buffer
 *		to dynamic kernel memory.
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
msi_dealloc_msg( pccb, scsbp )
    PCCB		*pccb;
    SCSH		*scsbp;
{
    MSIB		*msibp = Scs_to_pd( pccb, scsbp );

    SCA_KM_FREE(( u_char * )msibp, msibp->Size, KM_SCABUF )
}

/*   Name:	msi_add_msg	- Add MSI Port Message Buffer to Free Queue
 *
 *   Abstract:	This function adds a MSI port specific message buffer to a
 *		specific local MSI port message free queue.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *   scsbp			- Address of SCS header in message buffer
 *
 *   Outputs:
 *   pccb			- Port Command and Control Block pointer
 *	pd.msi			-  MSI specific PCCB fields
 *	    mfreeq		-   MSIB message free queue
 *
 *   IPL_SCS			- Interrupt processor level
 *
 *   SMP:	The PCCB specific MFREEQ is locked allowing exclusive access to
 *		the corresponding message free queue.
 *
 *		PCCB addresses are always valid allowing access to static
 *		fields because these data structures are never deleted once
 *		their corresponding ports have been initialized.
 */
void
msi_add_msg( pccb, scsbp )
    PCCB		*pccb;
    SCSH		*scsbp;
{
    MSIB		*msibp = Scs_to_pd( pccb, scsbp );

    Reset_msib( msibp )
    Insert_mfreeq( pccb, msibp )
}

/*   Name:	msi_remove_msg	- Remove MSI Message Buffer from Free Queue
 *
 *   Abstract:	This function removes a MSI port specific message buffer from
 *		a specific local MSI port message free queue.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   pccb			- Port Command and Control Block pointer
 *	pd.msi			-  MSI specific PCCB fields
 *	    mfreeq		-   MSIB message free queue
 *
 *   Return Values:
 *
 *   Address of SCS header in removed message buffer if successful
 *   Otherwise NULL
 *
 *   SMP:	The PCCB specific MFREEQ is locked allowing exclusive access to
 *		the corresponding message free queue.
 *
 *		PCCB addresses are always valid allowing access to static
 *		fields because these data structures are never deleted once
 *		their corresponding ports have been initialized.
 */
SCSH *
msi_remove_msg( pccb )
    PCCB		*pccb;
{
    msibq		*msibp;

    Remove_mfreeq( pccb, msibp )
    if( msibp ) {
	return(( SCSH * )Msibp->Msg.text );
    } else {
	return( NULL );
    }
}

/*   Name:	msi_send_msg	- Send MSI Port Specific Message
 *
 *   Abstract:	This function initiates transmission of a MSI port specific
 *		message over a specific path.
 *
 *		Two options exist for disposal of the buffer following
 *		transmission of the message:
 *
 *		1. Add the buffer to the appropriate port message free queue.
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
 *   pccb			- Port Command and Control Block pointer
 *	pd.msi			-  MSI specific PCCB fields
 *	    comql		-   MSIB low priority command queue
 *	    lpstatus.xfork	-   1
 *	    xforkb		-   Transmit Fork Process fork block
 *
 *   SMP:	The PCCB is locked INTERNALLY whenever it was not locked
 *		EXTERNALLY prior to routine invocation.  Locking the PCCB
 *		allows exclusive access to PCCB contents.  PCCB addresses are
 *		always valid because these data structures are never deleted
 *		once their corresponding ports have been initialized.
 *
 *		The PCCB specific COMQL is locked allowing exclusive access to
 *		the corresponding low priority command queue
 *
 *		The PB must be EXTERNALLY prevented from deletion.
 */
void
msi_send_msg( pccb, pb, scsbp, size, disposal )
    PCCB		*pccb;
    PB			*pb;
    SCSH		*scsbp;
    u_long		size;
    u_long		disposal;
{
    MSIB		*msibp = Scs_to_pd( pccb, scsbp );

    /* Only the Transmit Fork Process( XFP ) interfaces directly with the SII
     * chip for the purpose of processing outgoing MSI packets.  All other
     * driver routines must interface with the XFP.  This is accomplished by:
     *
     * 1. Formatting the MSIB buffer.
     * 2. Inserting the MSIB buffer onto the appropriate command queue.
     * 3. Requesting the XFP to process outgoing MSI packets.
     *
     * Formatting the MSIB buffer( Step 1 ) consists of formatting the:
     *
     * MSIB buffer header - information needed by XFP for packet transmission
     * MSI port header    - common MSI packet fields
     * MSI packet body	  - message specific MSI packet fields
     *
     * Message specific MSI packet fields requiring formatting by this routine
     * include only the CI PPD header( PPD message type field only ).  The text
     * portion of the message packet is always formatted prior to routine
     * invocation.
     *
     * MSIB buffers containing messages are always inserted onto the
     * appropriate low priority command queue( Step 2 ).  The PCCB specific
     * COMQL is locked immediately prior to insertion and unlocked immediately
     * afterwards.  This guarantees exclusive access to the queue.
     *
     * The XFP is requested to begin processing of outgoing MSI packets( Step
     * 3 ) by scheduling its asynchronous execution.  This step is bypassed
     * whenever XFP execution has already been scheduled but has not yet
     * commenced.  The PCCB is locked immediately prior to scheduling and
     * unlocked immediately afterwards provided it was not locked EXTERNALLY.
     * This guarantees scheduling of only 1 asynchronous XFP thread at any
     * given moment.
     *
     * NOTE:	It is possible for a XFP thread to be currently active.  This
     *		does not prevent scheduling of asynchronous XFP execution.
     *		However, the new thread does not begin to process outgoing MSI
     *		packets until the currently active thread completes.  Also, no
     *		other additional XFP threads are scheduled until the new thread
     *		begins processing outgoing MSI packets.
     */
    Format_msibh( msibp,
		  Scaaddr_lob( pb->pinfo.rport_addr ),
		  ( size + pccb->Msg_ovhd ),
		  ( disposal | MSI_VC ))
    Format_msih( msibp, MSG )
    Format_msippdh( msibp, SCSMSG )
    Insert_comql( pccb, msibp )
    Xstart_xfp( pccb )
}
