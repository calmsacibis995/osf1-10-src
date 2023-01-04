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
static char *rcsid = "@(#)$RCSfile: skz_dme.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/11/17 22:41:02 $";
#endif

/************************************************************************
 *									*
 * File:	skz_dme.c						*
 * Date:	November 10, 1991					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *	This file contains routines which are analogous to the DME      *
 * 	(data mover engine) routines found in other SIMs.  		*
 *									*
 *		1) skz_dme_init - initializes the scatter-gather array  *
 *				  free queue				*
 *				  called by xzaprobe()			*
 *									*
 *		2) skz_dme_setup - maps the data for transfer		*
 *				   called by skz_go()			*
 *									*
 *		3) skz_dme_end - frees up resources used in a transfer	*
 *				 called by skz_command_complete		*
 *									*
 ************************************************************************/

/* #define SKZ_DUMP */

/*
** Include Files
*/
#include <sys/types.h>
#include <sys/lock.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/buf.h>

#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/kern_return.h>
#include <kern/sched_prim.h>
#include <machine/rpb.h>
#include <machine/machparam.h>

#include <kern/thread.h>
#include <kern/lock.h>

#include <io/dec/mbox/mbox.h>
#include <io/common/devdriver.h>
#include <io/dec/xmi/xmireg.h>

#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>

#include <io/cam/xza/skz_params.h>
#include <io/cam/xza/skz_error.h>
#include <io/cam/xza/skz_xza.h>
#include <io/cam/xza/skz_nport.h>
#include <io/cam/xza/skz_xza_qb.h>
#include <io/cam/xza/skz.h>

/************************************************************************
 *
 *  ROUTINE NAME:  skz_dme_init()
 *
 *  FUNCTIONAL DESCRIPTION:  This routine allocates a pool of memory for
 *	scatter-gather array descriptors and their n_port queue carriers.
 *	For each scatter-gather descriptor, a page is allocated in 
 *	memory for the scatter-gather array itself, some initialization
 *	of the scatter-gather descriptor is done, and the scatter-gather
 *	descriptors are put onto a scatter-gather free queue.
 *	The number of free scatter-gather arrays allocated initially
 *	is set by a #def in skz_params.h
 *
 *	This routine is called from xzaprobe, very early in the game,
 *	to be sure we can allocate the memory we need before we go too
 *	far.
 *
 *  FORMAL PARAMETERS:
 *	xza_softc - pointer to the controlling data structure, which is
 *		    used to hold the memory pool starting addresses.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS: Raw I/O is possible after this routine is called.
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_dme_init ( XZA_SOFTC* xza_softc )

{
    u_int			skz_status;
    kern_return_t		kern_status;
    u_int			i;

    u_int			num_ptes;

    SKZ_DME_SG_DSC*		sg_dsc_ptr;
    CARRIER*			car_ptr;

    SIM_MODULE(skz_dme_init);

    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );
    /*
    ** Allocate a sg descriptor pool and the carrier pool,
    ** so that we can put the sg descriptors onto an N_Port
    ** queue
    */
    xza_softc -> dme_sg_dsc_pool = (SKZ_DME_SG_DSC *)
			kmem_alloc ( kernel_map, 
				SKZ_DME_SG_ARRAYS * sizeof ( SKZ_DME_SG_DSC ));
    xza_softc -> dme_sg_dsc_car_pool = (CARRIER *)
			kmem_alloc ( kernel_map, 
				(SKZ_DME_SG_ARRAYS + 1) * sizeof ( CARRIER ) );

    sg_dsc_ptr = xza_softc -> dme_sg_dsc_pool;
    car_ptr = xza_softc -> dme_sg_dsc_car_pool;

    /*
    ** Initialize the sg free queue, DDSGQ
    */
    np_init_queue (	DRIVER_DRIVER,
			&(xza_softc -> ddsgq),
			car_ptr++ );

    /*
    ** For each sg descriptor, allocate the sg array, initialize
    ** the sg descriptor, and put the sg descriptor onto the 
    ** sg free queue, DDSGQ
    */
    for ( i=0; i<SKZ_DME_SG_ARRAYS; i++ )
    {
	/*
	** Allocate the 1 page scatter-gather array
	*/
	sg_dsc_ptr -> sg_array_ptr = kmem_alloc ( kernel_map, 
						XZA_PORT_PAGE_SIZE );
	if ( sg_dsc_ptr -> sg_array_ptr == NULL )
	    return ( SKZ_MEM_ALLOC_ERR );

	/*
	** Pre-calculate the physical address of the array, so
	** we won't have to do it again
	*/
	kern_status = kvtop ( sg_dsc_ptr -> sg_array_ptr,
			     &(sg_dsc_ptr -> sg_phys_ptr) );
	if ( kern_status != KERN_SUCCESS )
	    return ( SKZ_PHYS_CONV_ERR );

	sg_dsc_ptr -> sg_elem_cnt = 0;

	/*
	** insert the array onto the sg free queue, DDSGQ
	*/
	skz_status = skz_put_queue ( DDSGQ, 
				     sg_dsc_ptr, 
				     car_ptr, 
				     xza_softc );

	sg_dsc_ptr++;
	car_ptr++;
    }

    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_dme_setup()
 *
 *  FUNCTIONAL DESCRIPTION: This function is responsible for setting up
 *	for a data transfer.  It initializes the dme descriptor in the
 *	driver private portion of the Q Buffer.  It initializes the buffer
 *	pointers in the Q Buffer structure based on the data being 
 *	transferred.  The rules are:
 *
 *	1 TYP0 pointer - for transfers up to 8KB that do not span
 *			 page boundaries.
 *
 *	2 TYP0 pointers - for transfer up to 8KB that cross a page 
 *			  boundary.
 *
 *	1 TYP3 pointer - for transfer 8KB < x <= 8MB, or for mapping
 *			 a CAM sg list to an XZA sg list
 *
 *	2 TYP3 pointers - for transfers 8MB < x <= 16MB, or for mapping
 *			  a CAM sg list to an XZA sg list.
 *
 *	The XZA firmware does not implement the TYP1 or TYP2 N_Port
 *	buffer pointer types.  TYP3 is an XZA-specific type implemented
 *	for scatter-gather lists.
 *
 * 	pmap_extract is used to get physical translations of user
 *	virtual addresses and svatophys is used for kernel addresses.
 *
 *  FORMAL PARAMETERS:
 *	xza_softc - a pointer to the controlling structure for the driver.
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

skz_dme_setup ( SIM_WS* sws, SKZ_DME_DSC* dme_dsc_ptr, XZA_SOFTC* xza_softc )

{
    u_int		skz_status;
    u_int		i;

    vm_offset_t		phys_addr;
    kern_return_t	kern_status;

    XZA_SG_ELEM*	sg_array_ptr;
    vm_offset_t		kern_data_ptr;

    SG_ELEM*		sg_elem;

    u_char*		data_ptr;
    u_int		data_len;
    u_int		remain_page_len;

    CCB_SCSIIO*		ccb;

    SIM_MODULE(skz_dme_setup);


    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );


    /*
    ** Initialize the DME descriptor
    */
    ccb = (CCB_SCSIIO *) sws -> ccb;

    dme_dsc_ptr -> data_ptr = (vm_offset_t) ccb -> cam_data_ptr;
    dme_dsc_ptr -> xfer_len = ccb -> cam_dxfer_len;
    dme_dsc_ptr -> bp = (struct buf *) ccb -> cam_req_map;

    if ( sws -> cam_flags & CAM_SCATTER_VALID )
    {
        dme_dsc_ptr -> flags |= SKZ_SG;
        dme_dsc_ptr -> cam_sg_count = ccb -> cam_sglist_cnt;
    }

    /*
    ** Check transfer size..
    ** If more that we can handle, return error
    ** If zero, we're done already
    */
    if ( dme_dsc_ptr -> xfer_len > XZA_MAX_TRANSFER )
	return ( SKZ_MAX_TRANSFER_ERR );

    if ( dme_dsc_ptr -> xfer_len == 0 )
	return ( CAM_REQ_CMP );

    /*
    ** Have we been given a scatter-gather list?
    ** If we have, map into an XZA sg list
    **
    ** Otherwise, depending upon the transfer size, either use
    ** one or two TYP0 buffer pointers or one or two TYP3 (SG) pointers
    */
    if ( dme_dsc_ptr -> flags & SKZ_SG )
    {
	/*
	** CAM has given us a scatter-gather list
	*/
	/*
	** Map each SG Element from CAM land to XZA land
	*/
	sg_elem = (SG_ELEM *) dme_dsc_ptr -> data_ptr;

	for ( i=0; i < dme_dsc_ptr -> cam_sg_count; i++ )
	{
	    /*
	    ** Walk down each element to break them up per page
	    */
	    data_ptr = (u_char *) sg_elem -> cam_sg_address;
	    data_len = (u_int) sg_elem -> cam_sg_count;

	    skz_status = skz_sg_map ( 	dme_dsc_ptr,
					data_ptr,
			 		data_len,
					xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
		return ( skz_status );

	    sg_elem++;

	}

 	/*
	** Set up the buffer pointer(s)
	*/

	/*
	** First TYP3 pointer
	*/
	if ( dme_dsc_ptr -> flags & SKZ_DME_SG_1_VALID )
	{
	    phys_addr = dme_dsc_ptr -> sg1_dsc -> sg_phys_ptr;

	    dme_dsc_ptr -> buf_root_ptr_0.pa = phys_addr >> 5;
	    dme_dsc_ptr -> buf_root_ptr_0.pax = phys_addr >> 32;
	    dme_dsc_ptr -> buf_root_ptr_0.count = 
		dme_dsc_ptr -> sg1_dsc -> sg_elem_cnt;
	    dme_dsc_ptr -> buf_root_ptr_0.typ = TYP3;
	}

	/*
	** Is there a second TYP3 pointer to set up ( for transfers > 8MB )
	*/
	if ( dme_dsc_ptr -> flags & SKZ_DME_SG_2_VALID )
	{
	    phys_addr = dme_dsc_ptr -> sg2_dsc -> sg_phys_ptr;

	    dme_dsc_ptr -> buf_root_ptr_1.pa = phys_addr >> 5;
	    dme_dsc_ptr -> buf_root_ptr_1.pax = phys_addr >> 32;
	    dme_dsc_ptr -> buf_root_ptr_1.count = 
		dme_dsc_ptr -> sg2_dsc -> sg_elem_cnt;
	    dme_dsc_ptr -> buf_root_ptr_1.typ = TYP3;
	}

    }
    else
    {
	/*
	** Check the size of the transfer
	*/
	remain_page_len =  ( NEXT_PAGE_ADDR ( dme_dsc_ptr -> data_ptr ) - 
					      dme_dsc_ptr -> data_ptr );

	/*
	** If the transfer is less than 8K and does not cross
	** a port page boundary, use a single TYP0 pointer
	*/
	if ( remain_page_len >= dme_dsc_ptr -> xfer_len ) 
	{ 
	    /*
	    ** Set up single TYP0 pointer
	    */
	    phys_addr = 0;

	    /*
	    ** Calculate the physical address of the data using
	    ** either svatophys, for kernel va's, or pmap_extract,
	    ** for user space addresses.
	    */
	    if ( IS_SYS_VA ( dme_dsc_ptr -> data_ptr ) )
	    {
	        kern_status = kvtop ( THIS_PAGE_ADDR(dme_dsc_ptr -> data_ptr), 
				     &phys_addr );
	        if ( kern_status != KERN_SUCCESS )
		    return ( SKZ_PHYS_CONV_ERR );
	    }
	    else
	    {
		phys_addr = pmap_extract (  
			dme_dsc_ptr -> bp -> b_proc -> task -> map -> vm_pmap,
			THIS_PAGE_ADDR(dme_dsc_ptr -> data_ptr) );
		if ( phys_addr == NULL )
		    return ( SKZ_PHYS_CONV_ERR );
	    }

	    /*
	    ** Initialize the TYP0 pointer
	    */
   	    dme_dsc_ptr -> buf_root_ptr_0.pa = phys_addr >> 5;
   	    dme_dsc_ptr -> buf_root_ptr_0.pax = phys_addr >> 32;
	    dme_dsc_ptr -> buf_root_ptr_0.typ = TYP0;

	    /*
	    ** Calculate the offset into the page to where the
	    ** data to be transferred starts
	    */
	    dme_dsc_ptr -> xfer_offset = dme_dsc_ptr -> data_ptr - 
			THIS_PAGE_ADDR( dme_dsc_ptr -> data_ptr ); 

	}

	/*
	** if the transfer spans two port pages but still is only
	** 8K or less, use two TYP0 pointers
	** This capability to use two TYP0's was provided by the XZA
	** architecture for efficiency since this is a very common case.
	*/
	else if ( ( remain_page_len + XZA_PORT_PAGE_SIZE > 
			dme_dsc_ptr -> xfer_len )
	       && ( dme_dsc_ptr -> xfer_len <= XZA_PORT_PAGE_SIZE ) )
	{
	    /*
	    ** Set up two TYP0 pointers
	    */
	    phys_addr = 0;

	    /*
	    ** Use svatophys for kernel virtual address and pmap_extract
	    ** for user space addresses to calculate the physical address
	    ** of the data.
	    */
	    if ( IS_SYS_VA ( dme_dsc_ptr -> data_ptr ) )
	    {
	        kern_status = kvtop ( THIS_PAGE_ADDR(dme_dsc_ptr -> data_ptr), 
				     &phys_addr );
	        if ( kern_status != KERN_SUCCESS )
		    return ( SKZ_PHYS_CONV_ERR );
	    }
	    else
	    {
		phys_addr = pmap_extract (  
			dme_dsc_ptr -> bp -> b_proc -> task -> map -> vm_pmap,
			THIS_PAGE_ADDR(dme_dsc_ptr -> data_ptr) );
		if ( phys_addr == NULL )
		    return ( SKZ_PHYS_CONV_ERR );
	    }

	    /* 
	    ** Initialize the first TYP0 pointer
	    */
   	    dme_dsc_ptr -> buf_root_ptr_0.pa = phys_addr >> 5;
   	    dme_dsc_ptr -> buf_root_ptr_0.pax = phys_addr >> 32;
	    dme_dsc_ptr -> buf_root_ptr_0.typ = TYP0;

	    /* 
	    ** Calculate the offset into the first page of the data
	    ** to be transferred
	    */
	    dme_dsc_ptr -> xfer_offset = dme_dsc_ptr -> data_ptr - 
				THIS_PAGE_ADDR ( dme_dsc_ptr -> data_ptr );

	    phys_addr = 0;

	    /*
	    ** If the data pointer is a kernel virtual address,
	    ** get the physical address via svatophys.  If it's a
	    ** user space address, then get the physical address
	    ** using pmap_extract
	    */
	    if ( IS_SYS_VA ( dme_dsc_ptr -> data_ptr ) )
	    {
	        kern_status = kvtop ( NEXT_PAGE_ADDR(dme_dsc_ptr -> data_ptr), 
				     &phys_addr );
	        if ( kern_status != KERN_SUCCESS )
		    return ( SKZ_PHYS_CONV_ERR );
	    }
	    else
	    {
		phys_addr = pmap_extract (  
			dme_dsc_ptr -> bp -> b_proc -> task -> map -> vm_pmap,
			NEXT_PAGE_ADDR(dme_dsc_ptr -> data_ptr) );
		if ( phys_addr == NULL )
		    return ( SKZ_PHYS_CONV_ERR );
	    }

	    /*
	    ** Initialize the second TYP0 buffer pointer
	    */
   	    dme_dsc_ptr -> buf_root_ptr_1.pa = phys_addr >> 5;
   	    dme_dsc_ptr -> buf_root_ptr_1.pax = phys_addr >> 32;
	    dme_dsc_ptr -> buf_root_ptr_1.typ = TYP0;

	}
	else
	{
	    /*
	    ** Set up our own scatter gather array, one TYP3 pointer
	    */
	    /*
	    ** Call skz_sg_map - this will map all of the data to 1 or 2
	    ** scatter-gather arrays.  If the transfer is greater than
	    ** 8KB and up to 8MB, one scatter gather array is required.
	    ** For transfer greater than 8MB up to 16MB, two scatter gather
	    ** arrays will be allocated.
	    */
	    skz_status = skz_sg_map (   dme_dsc_ptr, 
			 		dme_dsc_ptr -> data_ptr,
			 		dme_dsc_ptr -> xfer_len,
			 		xza_softc );
	    if ( skz_status != CAM_REQ_CMP )
	  	return ( skz_status );

	    phys_addr = 0;

	    /*
	    ** First TYP3 pointer, set up the boffer pointer in the
	    ** Q Buffer.
	    */
	    if ( dme_dsc_ptr -> flags & SKZ_DME_SG_1_VALID )
	    {
		/*
		** Use pre-calculated physical address of array
		*/
		phys_addr = dme_dsc_ptr -> sg1_dsc -> sg_phys_ptr;

   	        dme_dsc_ptr -> buf_root_ptr_0.pa = phys_addr >> 5;
   	        dme_dsc_ptr -> buf_root_ptr_0.pax = phys_addr >> 32;
	        dme_dsc_ptr -> buf_root_ptr_0.count = 
			dme_dsc_ptr -> sg1_dsc -> sg_elem_cnt;
	        dme_dsc_ptr -> buf_root_ptr_0.typ = TYP3;
	    }
	 
	    /*
	    ** If the mapping required a second scatter-gather array,
	    ** set up the second TYP3 pointer in the Q Buffer
	    */
	    if ( dme_dsc_ptr -> flags & SKZ_DME_SG_2_VALID )
	    {
		/*
		** Use pre-calculated physical address of the array
		*/
		phys_addr = dme_dsc_ptr -> sg2_dsc -> sg_phys_ptr;

   	        dme_dsc_ptr -> buf_root_ptr_1.pa = phys_addr >> 5;
   	        dme_dsc_ptr -> buf_root_ptr_1.pax = phys_addr >> 32;
	        dme_dsc_ptr -> buf_root_ptr_1.count =
			dme_dsc_ptr -> sg2_dsc -> sg_elem_cnt;
	        dme_dsc_ptr -> buf_root_ptr_1.typ = TYP3;
	    }

	    /*
	    ** Remember what we did so we can undo it
	    */
	    dme_dsc_ptr -> flags |= SKZ_SG;
	}
    }
	
    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_dme_end()
 *
 *  FUNCTIONAL DESCRIPTION: This routine frees up DME resources used 
 *	during the transfer.  It puts any scatter-gather arrays used,
 *	either one or two, back onto the DDSGQ, the scatter-gather
 *	array free queue.
 *
 *  FORMAL PARAMETERS:
 *	qb_ptr - a pointer to the completed Q Buffer
 *	xza_softc - pointer to the driver controlling data structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *
 *  SIDE EFFECTS:
 *	Scatter-gather elements used in a transfer are freed up and 
 *	put back on the DDSGQ queue.
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_dme_end ( QB* qb_ptr, XZA_SOFTC* xza_softc )

{
    u_int		skz_status;

    QB_DRIVER*		qb_driver_ptr;
    SKZ_DME_DSC*	dme_dsc_ptr;

    SIM_MODULE(skz_dme_end);

    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nEntering %s", module) );

    qb_driver_ptr = &(qb_ptr -> qb_driver);
    dme_dsc_ptr = &(qb_driver_ptr -> dme_dsc);

    /*
    ** If the dme descriptor has allocated scatter-gather arrays,
    ** initialize them and put them back on the DDSGQ for re-use
    ** using skz_put_queue.
    */
    if ( dme_dsc_ptr -> flags & SKZ_DME_SG_1_VALID )
    {
        dme_dsc_ptr -> sg1_dsc -> sg_elem_cnt = 0;
        bzero ( dme_dsc_ptr -> sg1_dsc -> sg_array_ptr, XZA_PORT_PAGE_SIZE );

        skz_status = skz_put_queue ( DDSGQ, 
				     dme_dsc_ptr -> sg1_dsc, 
				     dme_dsc_ptr -> car1_ptr, 
				     xza_softc );
    }

    if ( dme_dsc_ptr -> flags & SKZ_DME_SG_2_VALID )
    {
        dme_dsc_ptr -> sg2_dsc -> sg_elem_cnt = 0;
        bzero ( dme_dsc_ptr -> sg2_dsc -> sg_array_ptr, XZA_PORT_PAGE_SIZE );

        skz_status = skz_put_queue ( DDSGQ, 
				     dme_dsc_ptr -> sg2_dsc, 
				     dme_dsc_ptr -> car2_ptr, 
				     xza_softc );
    }

    SIM_PRINTD (	NOBTL,
			NOBTL,
			NOBTL,
			CAMD_INOUT,
			("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );
}



/************************************************************************
 *
 *  ROUTINE NAME:  skz_sg_map()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine creates scatter-gather elements for a given range of 
 * 	virtual addresses, breaking up the data by page.
 *
 *  FORMAL PARAMETERS:
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

skz_sg_map ( 	SKZ_DME_DSC*	dme_dsc_ptr,
		u_char*		data_ptr,
		u_int		data_len,
		XZA_SOFTC*	xza_softc )

{
    u_int		skz_status;
    vm_offset_t		phys_addr;
    kern_return_t	kern_status;

    int 		i;
    unsigned short	remain_page_len;
    unsigned short	sg_elem_len;

    XZA_SG_ELEM*	sg_array_ptr;
    SKZ_DME_SG_DSC*	sg_dsc_ptr;

    SIM_MODULE(skz_sg_map);


    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nEntering %s", module) );

    /*
    ** Do we have a scatter-gather array allocated?  If not,
    ** allocate one first.  The flags SKZ_DME_SG_?_VALID are used
    ** to indicate the scatter-gather arrays that have been allocated.
    ** There can only be one or two sg arrays ( XZA Architecture, for
    ** up to 8MB transfers or 8-16MB transfers )
    */
    if ( !( dme_dsc_ptr -> flags & SKZ_DME_SG_1_VALID ) )
    {
	skz_status = skz_alloc_sg_array ( dme_dsc_ptr, xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );

	sg_dsc_ptr = dme_dsc_ptr -> sg1_dsc;
    }
    else if ( dme_dsc_ptr -> sg1_dsc -> sg_elem_cnt <
		XZA_MAX_SG_ELEMENTS )
	sg_dsc_ptr = dme_dsc_ptr -> sg1_dsc;

    else if ( !(dme_dsc_ptr -> flags & SKZ_DME_SG_2_VALID) )
    {
	skz_status = skz_alloc_sg_array ( dme_dsc_ptr, xza_softc );
	if ( skz_status != CAM_REQ_CMP )
	    return ( skz_status );

	sg_dsc_ptr = dme_dsc_ptr -> sg2_dsc;
    }
    else if ( dme_dsc_ptr -> sg2_dsc -> sg_elem_cnt <
		XZA_MAX_SG_ELEMENTS )
        sg_dsc_ptr = dme_dsc_ptr -> sg2_dsc;
	
    else
	return ( SKZ_MAX_TRANSFER_ERR );

    /*
    ** Calculate where we are in the array.  We may have part of the
    ** transfer from a CAM scatter-gather element and have come back
    ** in here to continue with the next CAM sg element.  This calculation
    ** gives us the pointer into the array that we're working on so that
    ** we can continue to fill it up.
    */
    sg_array_ptr = (XZA_SG_ELEM *) ( sg_dsc_ptr -> sg_array_ptr +
	(sg_dsc_ptr -> sg_elem_cnt * sizeof (XZA_SG_ELEM)) );

    /*
    ** while data has not been mapped into the scatter-gather array
    */
    while ( data_len > 0 )
    {

        for ( i = sg_dsc_ptr -> sg_elem_cnt; 
	      (i < XZA_MAX_SG_ELEMENTS) && (data_len > 0); 
	      i++ )
        {
	    remain_page_len = NEXT_PAGE_ADDR ( data_ptr ) - 
				(vm_offset_t) data_ptr;

	    sg_elem_len = ( data_len <= remain_page_len ) ? 
			    data_len : remain_page_len;

	    phys_addr = 0;

 	    /*
	    ** If the data_ptr is a kernel virtual address, convert
	    ** to physical using svatophys(), otherwise get the physical
	    ** address using pmap_extract
	    */
	    if ( IS_SYS_VA ( data_ptr ) )
	    {
	        kern_status = kvtop ( data_ptr, &phys_addr );
	        if ( kern_status != KERN_SUCCESS )
	            return ( SKZ_PHYS_CONV_ERR );
	    }
	    else
	    {
		phys_addr = pmap_extract (  
			dme_dsc_ptr -> bp -> b_proc -> task -> map -> vm_pmap,
			data_ptr );
		if ( phys_addr == NULL )
		    return ( SKZ_PHYS_CONV_ERR );
	    }

	    /*
	    ** Set up the sg array element with this page's data
	    */
	    sg_array_ptr -> pa = phys_addr;
	    sg_array_ptr -> pax = phys_addr >> 32;
	    sg_array_ptr -> length = sg_elem_len;

	    sg_dsc_ptr -> sg_elem_cnt++;

	    /*
	    ** Advance to the next sg element
	    */
	    data_len -= sg_elem_len;
	    data_ptr += sg_elem_len;
	    sg_array_ptr++;

        } /* for */

	/*
	** if both arrays are full and we've still got data
	** that is not mapped, we're trying to transfer more data
	** than the XZA can handle
	*/
        if ( ((dme_dsc_ptr -> flags & SKZ_DME_SG_1_VALID) &&
	      (dme_dsc_ptr -> sg1_dsc -> sg_elem_cnt == XZA_MAX_SG_ELEMENTS)) &&
	     ((dme_dsc_ptr -> flags & SKZ_DME_SG_2_VALID) &&
	      (dme_dsc_ptr -> sg2_dsc -> sg_elem_cnt == XZA_MAX_SG_ELEMENTS)) &&
	     data_len != 0 )
	    return ( SKZ_MAX_TRANSFER_ERR );

	/*
	** If this sg array is full, determine whether we need
	** to allocate a second array and, if so, allocate it.
	*/
        else if ( (i == XZA_MAX_SG_ELEMENTS) && (data_len != 0) )
        {
	    if ( !( dme_dsc_ptr -> flags & SKZ_DME_SG_2_VALID ) )
	        skz_status = skz_alloc_sg_array ( dme_dsc_ptr, xza_softc );
	  	if ( skz_status != CAM_REQ_CMP )
		    return ( skz_status );

	    sg_dsc_ptr = dme_dsc_ptr -> sg2_dsc;

    	    sg_array_ptr = (XZA_SG_ELEM *) ( sg_dsc_ptr -> sg_array_ptr +
		(sg_dsc_ptr -> sg_elem_cnt * sizeof (XZA_SG_ELEM)) );

	}

    } /* While data_len > 0 */

    SIM_PRINTD ( NOBTL,
		 NOBTL,
		 NOBTL,
		 CAMD_INOUT,
		 ("\nLeaving %s", module) );

    return ( CAM_REQ_CMP );
}



/************************************************************************
 *
 *  ROUTINE NAME:  skz_alloc_sg_array()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine takes a scatter-gather array off of the DDSGQ.
 *	It sets up the dme descriptor for the new sg array, as number one
 *	two depending upon whether the dme descriptor already has a sg
 *	array or not.  
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *	CAM_REQ_CMP
 *	SKZ_SG_ALLOC_ERR
 *
 *  SIDE EFFECTS:
 *	The DDSGQ has one less element on it, the dme_descriptor passed
 *	in now points to the element.
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

skz_alloc_sg_array ( SKZ_DME_DSC* dme_dsc_ptr, XZA_SOFTC* xza_softc )

{
    u_int	skz_status;

    SKZ_DME_SG_DSC*	sg_dsc_ptr = 0;
    CARRIER*		car_ptr = 0;

    SIM_MODULE(skz_alloc_sg_array);

    /* 
    ** If scatter-gather array 1 has not been allocated yet, then
    ** allocate it, otherwise allocate scatter-gather array 2
    */
    if ( !(dme_dsc_ptr -> flags & SKZ_DME_SG_1_VALID) )
    {
	skz_status = skz_get_queue ( DDSGQ,
				     &sg_dsc_ptr,
				     &car_ptr,
				     xza_softc );
	if ( skz_status == CAM_REQ_CMP )
	{
	    dme_dsc_ptr -> sg1_dsc = sg_dsc_ptr;
	    dme_dsc_ptr -> car1_ptr = (vm_offset_t) car_ptr;
	    dme_dsc_ptr -> flags |= SKZ_DME_SG_1_VALID;
	}
	else
	{
	    return ( SKZ_SG_ALLOC_ERR );
	}
    }
    else if ( !(dme_dsc_ptr -> flags & SKZ_DME_SG_2_VALID) )
    {
	skz_status = skz_get_queue ( DDSGQ,
				     &sg_dsc_ptr,
				     &car_ptr,
				     xza_softc );
	if ( skz_status == CAM_REQ_CMP )
	{
	    dme_dsc_ptr -> sg2_dsc = sg_dsc_ptr;
	    dme_dsc_ptr -> car2_ptr = (vm_offset_t) car_ptr;
	    dme_dsc_ptr -> flags |= SKZ_DME_SG_2_VALID;
	}
	else
	    return ( SKZ_SG_ALLOC_ERR );
    }

    return ( CAM_REQ_CMP );

}
