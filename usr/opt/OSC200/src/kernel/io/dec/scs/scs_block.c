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
static char *rcsid = "@(#)$RCSfile: scs_block.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 15:19:55 $";
#endif
/*
 * derived from scs_block.c	4.1  (ULTRIX)        7/2/90";
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Systems Communication Services
 *
 *   Abstract:	This module contains Systems Communication Services( SCS )
 *		block data communication service functions.
 *
 *   Creator:	Todd M. Katz	Creation Date:	May 27, 1985
 *
 *   Function/Routines:
 *
 *   scs_map_buf		Map Local Buffer
 *   scs_unmap_buf		Unmap Local Buffer
 *   scs_req_data		Request Block Data
 *   scs_send_data		Send Block Data
 *
 *   Modification History:
 *
 *   15-Mar-1991	Brian Nadeau
 *	Port to OSF
 *
 *   11-Mar-1989	Todd M. Katz		TMK0003
 *	Include header file smp_lock.h.
 *
 *   06-Mar-1989	Todd M. Katz		TMK0002
 *	Include header file ../vaxmsi/msisysap.h.
 *
 *   02-Jun-1988     Ricky S. Palmer
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
#include		<sys/buf.h>
#include		<dec/binlog/errlog.h>
#include		<kern/lock.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>

/* External Variables.
 */
extern	SCSIB		lscs;
extern	CBVTDB		*scs_cbvtdb;

/*   Name:	scs_map_buf	- Map Local Buffer
 *
 *   Abstract:	This function maps a local buffer.  Lack of resources may
 *		prevent mapping.  SCS makes NO attempt to notify SYSAPs when
 *		mapping resources are again available.
 *
 *		Access mode checking of buffers by local ports is NOT currently
 *		implemented.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   csb			- Communication Services Block pointer
 *	connid			-  Identification of logical SCS connection
 *	ov2.sbh			-  System buffer handle pointer
 *	    ( sbh->b_bcount )	-   Size of buffer to map
 *   lscs			- Local system permanent information
 *   scs_cbvtdb			- CB vector table database pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *	cinfo.bytes_mapped	- Number of bytes mapped
 *   csb			- Communication Services Block pointer
 *	lbhandle		-  Local buffer handle
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Local buffer is mapped
 *   RET_INVCONNID		- Invalid connection identification number
 *   RET_INVCSTATE		- Connection in invalid state
 *   RET_NORESOURCES		- No system-wide resources available
 *
 *   SMP:	The CB is locked to synchronize access and prevent deletion.
 *		It is indirectly locked through its CBVTE.
 */
u_long
scs_map_buf( csb )
    CSB			*csb;
{
    CB			*cb;
    CBVTE		*cbvte;
    u_long		status;

    /* The steps involved in mapping a buffer are as follows:
     *
     * 1. Lock and retrieve the CB.
     * 2. Invoke a PD specific function to map the local buffer.
     * 3. Perform connection bookkeeping if mapping is successful.
     * 4. Unlock the CB.
     * 5. Return an appropriate status.
     */
    Check_connid( csb->connid, cbvte, cb )
    if( cb->cinfo.cstate != CS_OPEN ) {
	status = RET_INVCSTATE;
    } else if(( status = ( *cb->Map_buf )( cb->pccb,
					   &csb->lbhandle,
					   csb->Sbh,
					   U_int( cb->cinfo.rconnid )))
		== RET_SUCCESS ) {
	Data_counter( cb->cinfo.bytes_mapped, csb->Sbh->b_bcount )
    } else {
	status = RET_NORESOURCES;
    }
    Unlock_cbvte( cbvte );
    return( status );
}

/*   Name:	scs_unmap_buf	- Unmap Local Buffer
 *
 *   Abstract:	This function unmaps a local buffer releasing the associated
 *		mapping resources.  No attempt is made to notify SYSAPs that
 *		these mapping resources are now available.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   csb			- Communication Services Block pointer
 *	connid			-  Identification of logical SCS connection
 *	lbhandle		-  Local buffer handle
 *	ov2.sbh			-  System buffer handle pointer
 *   lscs			- Local system permanent information
 *   scs_cbvtdb			- CB vector table database pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   csb			- Communication Services Block pointer
 *	lbhandle		-  0
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Local buffer is unmapped
 *   RET_INVCONNID		- Invalid connection identification number
 *   RET_INVCSTATE		- Connection in invalid state
 *
 *   SMP:	The CB is locked to synchronize access and prevent deletion.
 *		It is indirectly locked through its CBVTE.
 */
u_long
scs_unmap_buf( csb )
    CSB			*csb;
{
    CB			*cb;
    CBVTE		*cbvte;
    u_long		status = RET_SUCCESS;

    /* The steps involved in unmapping a buffer are as follows:
     *
     * 1. Lock and retrieve the CB.
     * 2. Invoke a PD specific routine to unmap the local buffer.
     * 3. Unlock the CB.
     * 4. Return an appropriate status.
     */
    Check_connid( csb->connid, cbvte, cb )
    if( cb->cinfo.cstate == CS_OPEN         ||
	 cb->cinfo.cstate == CS_DISCONN_REC ||
	 cb->cinfo.cstate == CS_PATH_FAILURE ) {  
	( void )( *cb->Unmap_buf )( cb->pccb, &csb->lbhandle, csb->Sbh );
	Zero_bhandle( csb->lbhandle )
    } else {
	status = RET_INVCSTATE;
    }
    Unlock_cbvte( cbvte );
    return( status );
}

/*   Name:	scs_req_data	- Request Block Data
 *
 *   Abstract:	This function initiates transfer of block data from a mapped
 *		remote buffer to a mapped local buffer over a specific logical
 *		SCS connection.  During transfer the SYSAP is prevented from
 *		terminating the logical SCS connection.  The SYSAP is notified
 *		of transfer completion through asynchronous invocation of the
 *		connection's control event routine.
 *
 *		A send credit is temporarily consumed during block data
 *		transfer.  It is returned after the transfer completes.  The
 *		lack of send credits prevents transfer of block data.  The
 *		SYSAP is notified of send credit availability through
 *		asynchronous invocation of the connection's control event
 *		routine.
 *
 *		Third party I/O is NOT currently supported by this function.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   csb			- Communication Services Block pointer
 *	connid			-  Identification of logical SCS connection
 *	lbhandle		-  Local buffer handle
 *	ov1.blockid		-  Identification number of block data transfer
 *	ov2.rboff		-  Remote buffer transfer offset
 *	ov3.lboff		-  Local buffer transfer offset
 *	rbhandle		-  Remote buffer handle
 *	size			-  Application data transfer size
 *   lscs			- Local system permanent information
 *   scs_cbvtdb			- CB vector table database pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *	cinfo.bytes_req		-  Number of bytes requested
 *	cinfo.ntransfers	-  Number of block data transfers in progress
 *	cinfo.snd_credit	-  Number of send credits
 *	cinfo.rdatas_snt	-  Number of request datas sent
 *	cinfo.status.cwait	-  SYSAP waiting for send credits flag
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Successfully initiated block data transfer
 *   RET_ALLOCFAIL		- Failed to allocate a local port buffer
 *   RET_INVCONNID		- Invalid connection identification number
 *   RET_INVCSTATE		- Connection in invalid state
 *   RET_NOCREDITS		- No send credits available
 *   RET_NOSUPPORT		- Function unsupported on SCS connection
 *
 *   SMP:	The CB is locked to synchronize access and prevent deletion.
 *		It is indirectly locked through its CBVTE.  Locking the CB also
 *		prevents PB deletion as required by PD routines which request
 *		block data.
 */
u_long
scs_req_data( csb )
    CSB			*csb;
{
    TID			tid;
    CB			*cb;
    CBVTE		*cbvte;
    u_long		( *request )(), status;

    /* The steps involved in requesting block data are as follows:
     *
     * 1. Lock and retrieve the CB.
     * 2. Invoke a PD specific function to request transfer of data.
     * 3. Perform connection bookkeeping and temporarily decrement the number
     *	  of available send credits if transfer is successful initiated.
     * 4. Unlock the CB.
     * 5. Return an appropriate status.
     * 
     * The lack of send credits is remembered if none are available for this
     * transfer.  The SYSAP is notified when send credits are again available.
     */
    Check_connid( csb->connid, cbvte, cb );
    if( cb->cinfo.cstate != CS_OPEN ) {
	status = RET_INVCSTATE;
    } else if(( request = cb->Request_data ) == NULL ) {
	status = RET_NOSUPPORT;
    } else if(( int )--cb->cinfo.snd_credit >= 0 ) {
	tid.blockid = csb->Blockid;
	Move_connid( cb->cinfo.lconnid, tid.lconnid )
	if(( status = ( *request )( cb->pccb,
			 	    cb->pb,
				    csb->size,
				    &tid,
				    &csb->lbhandle,
				    csb->Lboff,
				    &csb->rbhandle,
				    csb->Rboff )) == RET_SUCCESS ) {
	    ++cb->cinfo.ntransfers;
	    Event_counter( cb->cinfo.rdatas_snt )
	    Data_counter( cb->cinfo.bytes_req, csb->size )
	} else {
	    ++cb->cinfo.snd_credit;
	}
    } else {
	cb->cinfo.snd_credit = 0;
	cb->cinfo.status.cwait = 1;
	status = RET_NOCREDITS;
    }
    Unlock_cbvte( cbvte );
    return( status );
}

/*   Name:	scs_send_data	- Send Block Data
 *
 *   Abstract:	This function initiates transfer of block data from a mapped
 *		local buffer to a mapped remote buffer over a specific logical
 *		SCS connection.  During transfer the SYSAP is prevented from
 *		terminating the logical SCS connection.  The SYSAP is notified
 *		of transfer completion through asynchronous invocation of the
 *		connection's control event routine.
 *
 *		A send credit is temporarily consumed during block data
 *		transfer.  It is returned after the transfer completes.  The
 *		lack of send credits prevents transfer of block data.  The
 *		SYSAP is notified of send credit availability through
 *		asynchronous invocation of the connection's control event
 *		routine.
 *
 *		Third party I/O is NOT currently supported by this function.
 *
 *   Inputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   csb			- Communication Services Block pointer
 *	connid			-  Identification of logical SCS connection
 *	lbhandle		-  Local buffer handle
 *	ov1.blockid		-  Identification number of block data transfer
 *	ov2.rboff		-  Remote buffer transfer offset
 *	ov3.lboff		-  Local buffer transfer offset
 *	rbhandle		-  Remote buffer handle
 *	size			-  Application data transfer size
 *   lscs			- Local system permanent information
 *   scs_cbvtdb			- CB vector table database pointer
 *
 *   Outputs:
 *
 *   IPL_SCS			- Interrupt processor level
 *   cb				- Connection Block pointer
 *	cinfo.bytes_snt		-  Number of bytes sent
 *	cinfo.ntransfers	-  Number of block data transfers in progress
 *	cinfo.snd_credit	-  Number of send credits
 *	cinfo.sdatas_snt	-  Number of send datas sent
 *	cinfo.status.cwait	-  SYSAP waiting for send credits flag
 *
 *   Return Values:
 *
 *   RET_SUCCESS		- Successfully initiated block data transfer
 *   RET_ALLOCFAIL		- Failed to allocate a local port buffer
 *   RET_INVCONNID		- Invalid connection identification number
 *   RET_INVCSTATE		- Connection in invalid state
 *   RET_NOCREDITS		- No send credits available
 *   RET_NOSUPPORT		- Function unsupported on SCS connection
 *
 *   SMP:	The CB is locked to synchronize access and prevent deletion.
 *		It is indirectly locked through its CBVTE.  Locking the CB also
 *		prevents PB deletion as required by PD routines which send
 *		block data.
 */
u_long
scs_send_data( csb )
    CSB			*csb;
{
    TID			tid;
    CB			*cb;
    CBVTE		*cbvte;
    u_long		( *send )(), status;

    /* The steps involved in sending block data are as follows:
     *
     * 1. Lock and retrieve the CB.
     * 2. Invoke a PD specific function to transfer data.
     * 3. Perform connection bookkeeping and temporarily decrement the number
     *	  of available send credits if transfer is successful initiated.
     * 4. Unlock the CB.
     * 5. Return an appropriate status.
     * 
     * The lack of send credits is remembered if none are available for this
     * transfer.  The SYSAP is notified when send credits are again available.
     */
    Check_connid( csb->connid, cbvte, cb );
    if( cb->cinfo.cstate != CS_OPEN ) {
	status = RET_INVCSTATE;
    } else if(( send = cb->Send_data ) == NULL ) {
	status = RET_NOSUPPORT;
    } else if(( int )--cb->cinfo.snd_credit >= 0 ) {
	tid.blockid = csb->Blockid;
	Move_connid( cb->cinfo.lconnid, tid.lconnid )
	if(( status = ( *send )( cb->pccb,
				 cb->pb,
				 csb->size,
				 &tid,
				 &csb->lbhandle,
				 csb->Lboff,
				 &csb->rbhandle,
				 csb->Rboff )) == RET_SUCCESS ) {
	    ++cb->cinfo.ntransfers;
	    Event_counter( cb->cinfo.sdatas_snt )
	    Data_counter( cb->cinfo.bytes_snt, csb->size )
	} else {
	    ++cb->cinfo.snd_credit;
	}
    } else {
	cb->cinfo.snd_credit = 0;
	cb->cinfo.status.cwait = 1;
	status = RET_NOCREDITS;
    }
    Unlock_cbvte( cbvte );
    return( status );
}
