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
static char *rcsid = "@(#)$RCSfile: skz_nport.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/11/17 22:41:30 $";
#endif
                                                                                
/************************************************************************
 *									*
 * File:	skz_nport.c						*
 * Date:	November 7, 1991					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 * 	This file contains routines that manipulate N_Port queues       *
 *									*
 ************************************************************************/

/* #define CAMDEBUG */

/*
** Include Files
*/
#include <sys/types.h>
#include <sys/lock.h>
#include <mach/kern_return.h>
#include <kern/lock.h>

#include <kern/thread.h>

#include <io/common/iotypes.h>

#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>

#include <io/cam/xza/skz_params.h>
#include <io/cam/xza/skz_xza.h>
#include <io/cam/xza/skz_nport.h>
#include <io/cam/xza/skz_xza_qb.h>
#include <io/cam/xza/skz_error.h>
#include <io/cam/xza/skz.h>

/************************************************************************
 *
 *  ROUTINE NAME: np_insert_da_q() 
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine inserts a q buffer into a driver-adapter queue
 *
 *  FORMAL PARAMETERS:
 *	q_tail_ptr - address of tail of queue to be manipulated
 *	car_ptr	- address of carrier structure
 *	qb_ptr - address of queue buffer structure
 *
 *  IMPLICIT INPUTS:
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *	The queue will have had an entry inserted at the tail.
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned long np_insert_da_q (	CARRIER**	q_tail_ptr,
				CARRIER*	car_ptr,
				QB*		qb_ptr	)
{
    NP_QB_HEADER*	np_qb_hdr;

    vm_offset_t		phys_addr;
    kern_return_t	kern_status;

    SIM_MODULE(np_insert_da_q);

    SIM_PRINTD	(	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			( "\nEntering %s", module ) );

    np_qb_hdr = &(qb_ptr -> qb_header.np_qb_hdr);

    car_ptr->next_ptr = ADAPTER_STOPPER;
    car_ptr->car_token = (long) car_ptr;		/* save virt addr */
    car_ptr->qb_ptr = 0;
    car_ptr->qb_token = 0;

    np_qb_hdr->token = (long) qb_ptr;		/* save virt addr */

    kern_status = kvtop ( qb_ptr, &phys_addr );
    (*q_tail_ptr)->qb_ptr = phys_addr;		/* use phys ptrs d-a */
    (*q_tail_ptr)->qb_token = (long) qb_ptr;	/* save virt address */
    (*q_tail_ptr)->car_token = (long) (*q_tail_ptr);
    mb();	/* memory barrier, so stopper shows up after next instr */

    kern_status = kvtop ( car_ptr, &phys_addr );
    (*q_tail_ptr)->next_ptr = phys_addr + 1;	/* Set <0> to make valid */
    mb();	/* memory barrier, before changing tail ptr */

    *q_tail_ptr = car_ptr;

    SIM_PRINTD	(	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			( "\nLeaving %s", module ) );
    return ( 0 );
}

/************************************************************************
 *
 *  ROUTINE NAME: np_insert_dd_q() 
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine inserts a q buffer into a driver-driver queue
 *
 *  FORMAL PARAMETERS:
 *	q_tail_ptr - address of tail of queue to be manipulated
 *	car_ptr	- address of carrier structure
 *	qb_ptr - address of queue buffer structure
 *
 *  IMPLICIT INPUTS:
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *	The queue will have had an entry inserted at the tail.
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned long np_insert_dd_q (	CARRIER**	q_tail_ptr,
				CARRIER*	car_ptr,
				NP_QB_HEADER*	qb_ptr	)
{
    vm_offset_t		phys_addr;
    kern_return_t	kern_status;

    SIM_MODULE(np_insert_dd_q);

    SIM_PRINTD	(	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			( "\nEntering %s", module ) );

    car_ptr->next_ptr = DRIVER_STOPPER;
    car_ptr->car_token = (long) car_ptr;	/* save virt! addr */
    car_ptr->qb_token = 0;
    car_ptr->qb_ptr = 0;

    qb_ptr->token = (long) qb_ptr;		/* save phys addr */

    (*q_tail_ptr)->qb_ptr = (long) qb_ptr;	/* use phys ptrs d-d */
    (*q_tail_ptr)->qb_token = (long) qb_ptr; 	/* save vir, like adapt */
    (*q_tail_ptr)->car_token = (long) (*q_tail_ptr);

    mb();	/* memory barrier, so stopper shows up after next instr */

    (*q_tail_ptr)->next_ptr = (vm_offset_t) car_ptr;
    mb();	/* memory barrier, before changing tail ptr */

    *q_tail_ptr = car_ptr;

    mb();

    SIM_PRINTD	(	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			( "\nLeaving %s", module ) );

    return ( NP_SUCCESS );
}


/************************************************************************
 *
 *  ROUTINE NAME: np_insert_dd_q_head() 
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine inserts a q buffer into a driver-drvier queue, at the
 *      head of the queue.
 *
 *  FORMAL PARAMETERS:
 *	q_head_ptr - address of head of queue to be manipulated
 *	car_ptr	- address of carrier structure
 *	qb_ptr - address of queue buffer structure
 *
 *  IMPLICIT INPUTS:
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *	The queue will have had an entry inserted at the head.
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned long np_insert_dd_q_head (	CARRIER**	q_head_ptr,
					CARRIER*	car_ptr,
					QB*		qb_ptr	)
{
	NP_QB_HEADER*	np_qb_hdr;

	np_qb_hdr = &(qb_ptr -> qb_header.np_qb_hdr);

	car_ptr->next_ptr = (vm_offset_t) *q_head_ptr;
	car_ptr->car_token = (long) car_ptr;	/* save phys addr */	
	car_ptr->qb_ptr = (vm_offset_t) qb_ptr;

	np_qb_hdr->token = (long) qb_ptr;		/* save phys addr */
	car_ptr->qb_token = (long) qb_ptr;

	mb();	/* memory barrier, before changing head ptr */

	*q_head_ptr = (CARRIER *) car_ptr;	/* use virt ptrs d-d */

	return ( 0 );
}



/************************************************************************
 *
 *  ROUTINE NAME: np_rem_drv_q_entry() 
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine removes a q buffer from a driver queue
 *
 *  FORMAL PARAMETERS:
 *	q_tail_ptr - address of head of queue to be manipulated
 *	car_ptr	- address of carrier structure
 *	qb_ptr - address of queue buffer structure
 *
 *  IMPLICIT INPUTS:
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *	The queue will have had an entry removed from the head.
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned long np_rem_drv_q_entry (	CARRIER**	q_head_ptr,
					CARRIER**	car_ptr,
					QB**		qb_ptr	)
{
    SIM_MODULE(np_rem_drv_q_entry);

    SIM_PRINTD	(	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_FLOW,
			( "\nEntering %s", module ) );

    if ( ( (*q_head_ptr)->next_ptr & 1L ) == DRIVER_STOPPER ) 
	return ( NP_FAILURE );
    else
    {
        SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_FLOW,
			( "\n%s: Queue Not Empty", module ) );

	(*car_ptr) = (*q_head_ptr);
	mb();
	(*q_head_ptr) = (CARRIER *) (*car_ptr)->next_ptr;
	(*qb_ptr) = (QB *) (*car_ptr)->qb_token;

 	mb();

        SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_FLOW,
			( "\nLeaving %s", module ) );

	return ( NP_SUCCESS );
    }


}



/************************************************************************
 *
 *  ROUTINE NAME: np_rem_adapt_q_entry() 
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine removes a q buffer from an adapter queue
 *
 *  FORMAL PARAMETERS:
 *	q_head_ptr - address of head of queue to be manipulated
 *	car_ptr	- address of carrier structure
 *	qb_ptr - address of queue buffer structure
 *
 *  IMPLICIT INPUTS:
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *	The queue will have had an entry removed from the head.
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

unsigned long np_rem_adapt_q_entry (	CARRIER**	q_head_ptr,
					CARRIER**	car_ptr,
					QB**		qb_ptr	)
{
/*	*car_ptr = (CARRIER *) ptov ( *q_head_ptr ); */
	*car_ptr = 
	(CARRIER *) ((CARRIER *) *q_head_ptr) -> car_token; /* gets virt addr? */
	if ( ( (*car_ptr)->next_ptr & 1L ) == ADAPTER_STOPPER ) 
		return ( NP_FAILURE );
	else
	{
		*qb_ptr = (QB *) (*car_ptr)->qb_token;
		*q_head_ptr = (CARRIER *) (*car_ptr)->next_ptr;
		return ( NP_SUCCESS );
	}
}


/************************************************************************
 *
 *  ROUTINE NAME: np_drv_q_entry_present() 
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine determines whether an entry is present on a
 *      driver queue.
 *
 *  FORMAL PARAMETERS:
 *	q_head_ptr - address of head of queue to be manipulated
 *
 *  IMPLICIT INPUTS:
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

np_drv_q_entry_present (	CARRIER**	q_head_ptr )

{
	return ( (((*q_head_ptr)->next_ptr & 1L) != DRIVER_STOPPER )
		? NP_SUCCESS : NP_FAILURE );
}


/************************************************************************
 *
 *  ROUTINE NAME:  np_adapt_q_entry_present()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine determines whether an entry is present on an
 * 	adapter queue.
 *
 *  FORMAL PARAMETERS:
 *	q_head_ptr
 *	
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

np_adapt_q_entry_present ( CARRIER* q_head_ptr )

{
/*
	return ( ( (((CARRIER *) ptov(q_head_ptr))->next_ptr & 1L) != ADAPTER_STOPPER )
*/
	return ( ( (((CARRIER *) q_head_ptr -> car_token)->next_ptr & 1L) != ADAPTER_STOPPER )


		? NP_SUCCESS : NP_FAILURE );
}


/************************************************************************
 *
 *  ROUTINE NAME:  np_init_queue()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine initialized an N_Port queue.
 *
 *  FORMAL PARAMETERS:
 *	q_head_ptr
 *	
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

void np_init_queue (	QUEUE_TYPE	queue_type,
			NP_Q*		queue,
			CARRIER*	car_ptr )

{
    vm_offset_t		phys_addr;
    kern_return_t	kern_status;

    kern_status = kvtop ( car_ptr, &phys_addr );

	switch ( queue_type ) {

		case DRIVER_ADAPTER:
			car_ptr->next_ptr = ADAPTER_STOPPER;
			car_ptr->car_token = (long) car_ptr;
			queue->head_ptr = (CARRIER *) phys_addr;
			queue->tail_ptr = car_ptr;
			break;

		case ADAPTER_DRIVER:
			car_ptr->next_ptr = DRIVER_STOPPER;
			car_ptr->car_token = (long) car_ptr;
			queue->head_ptr = car_ptr;
			queue->tail_ptr = (CARRIER *) phys_addr;
			break;

		case DRIVER_DRIVER:
			car_ptr->next_ptr = DRIVER_STOPPER;
			car_ptr->car_token = (long) car_ptr;
			queue->head_ptr = car_ptr;
			queue->tail_ptr = car_ptr;
			break;

		case ADAPTER_ADAPTER:
			car_ptr->next_ptr = ADAPTER_STOPPER;
			car_ptr->car_token = (long) car_ptr;
			queue->head_ptr = (CARRIER *) phys_addr;
			queue->tail_ptr = (CARRIER *) phys_addr;
			break;
		}
}
