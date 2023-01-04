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
static char *rcsid = "@(#)$RCSfile: skz_debug.c,v $ $Revision: 1.1.2.10 $ (DEC) $Date: 1992/10/06 16:57:21 $";
#endif

/************************************************************************
 *									*
 * File:	skz_debug.c						*
 * Date:	December 30, 1991					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *	This file contains routines which print out structure contents	*
 *	for debugging purposes.						*
 *									*
 ************************************************************************/
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
 *  ROUTINE NAME:  skz_dump_xza_softc()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine dumps the contents of the driver's xza_softc 
 *	controlling data structure.
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

skz_dump_xza_softc ( XZA_SOFTC* xza_softc )

{
    printf ( "\nxza_softc -> xza_base = %x", xza_softc -> xza_base );
    printf ( "\nxza_softc -> flags = %x", xza_softc -> flags );
    printf ( "\nxza_softc -> xza_xmi_node = %d", xza_softc -> xza_xmi_node );
    printf ( "\nxza_softc -> lamb_xmi_node = %d", xza_softc -> lamb_xmi_node );
    printf ( "\nxza_softc -> c0_misc_intr_vector = %x", xza_softc -> c0_misc_intr_vector );
    printf ( "\nxza_softc -> c1_misc_intr_vector = %x", xza_softc -> c1_misc_intr_vector );
    printf ( "\nxza_softc -> resp_intr_vector = %x", xza_softc -> resp_intr_vector );
    printf ( "\nxza_softc -> ab = %x", xza_softc -> ab );
    printf ( "\nxza_softc -> np_qb_pool = %x", xza_softc -> np_qb_pool );
    printf ( "\nxza_softc -> np_carrier_pool = %x", xza_softc -> np_carrier_pool );
    printf ( "\nxza_softc -> ddfq.head_ptr = %x", xza_softc -> ddfq.head_ptr );
    printf ( "\nxza_softc -> ddfq.tail_ptr = %x", xza_softc -> ddfq.tail_ptr );
    printf ( "\nxza_softc -> channel[0].state = %d", xza_softc -> channel[0].state );
    printf ( "\nxza_softc -> channel[1].state = %d", xza_softc -> channel[1].state );
    printf ( "\nxza_softc -> channel[0].scsi_id = %d", xza_softc -> channel[0].scsi_id.targid );
    printf ( "\nxza_softc -> channel[1].scsi_id = %d", xza_softc -> channel[1].scsi_id.targid );
    printf ( "\nxza_softc -> channel[0].chan_num = %d", xza_softc -> channel[0].chan_num );
    printf ( "\nxza_softc -> channel[1].chan_num = %d", xza_softc -> channel[1].chan_num );
    printf ( "\nxza_softc -> channel[0].commands_sent = %d", xza_softc -> channel[0].commands_sent );
    printf ( "\nxza_softc -> channel[1].commands_sent = %d", xza_softc -> channel[1].commands_sent );
    printf ( "\nxza_softc -> channel[0].flags = %x", xza_softc -> channel[0].flags );
    printf ( "\nxza_softc -> channel[1].flags = %x", xza_softc -> channel[1].flags );

    return ( 1 );
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_dump_scsi_qb_hdr()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine dumps the contents of a SNDSCSI Q Buffer header.
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

skz_dump_scsi_qb_hdr ( SCSI_QB_HEADER* qb_hdr )

{
    printf ( "\nheader -> opc = %x", qb_hdr -> opc );
    printf ( "\nheader -> chnl_idx = %d", qb_hdr -> chnl_idx );
    printf ( "\nheader -> flags = %x", qb_hdr -> flags );
    printf ( "\nheader -> status.fail = %x", qb_hdr -> status.fail );
    printf ( "\nheader -> status.type = %x", qb_hdr -> status.type );
    printf ( "\nheader -> status.scsi_status = %x", 
					qb_hdr -> status.scsi_status );
    printf ( "\nheader -> scsi_opc = %x", qb_hdr -> scsi_opc );
    printf ( "\nheader -> scsi_flags = %x", qb_hdr -> scsi_flags );
    printf ( "\nheader -> dst_xport.targid = %x", qb_hdr -> dst_xport.targid );
    printf ( "\nheader -> dst_xport.lun = %x", qb_hdr -> dst_xport.lun );
    printf ( "\nheader -> src_xport = %x", qb_hdr -> src_xport );
    return ( CAM_REQ_CMP );
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_dump_scsi_qb_body()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine dumps the contents of a SNDSCSI Q Buffer body.
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

skz_dump_scsi_qb_body ( SNDSCSI_CMD* qb_body_ptr, QB* qb_ptr )

{
    vm_offset_t		as_buf = 0;

    printf ( "\nbody -> xfer_len = %d", qb_body_ptr -> xfer_len );
    printf ( "\nbody -> xfer_offset = %d", qb_body_ptr -> xfer_offset );
    printf ( "\nbody -> ret_len = %d", qb_body_ptr -> ret_len );
    printf ( "\nbody -> cdb_len = %d", qb_body_ptr -> cdb_len );
    printf ( "\nbody -> buf_root_ptr_0 = %x", qb_body_ptr -> buf_root_ptr_0 );
    printf ( "\nbody -> buf_root_ptr_1 = %x", qb_body_ptr -> buf_root_ptr_1 );

    bcopy ( qb_body_ptr -> autosense_ptr, &as_buf, 
		sizeof ( qb_body_ptr -> autosense_ptr ) );
    printf ( "\nbody -> as_ptr = %x", as_buf );

    printf ( "\nbody -> as_len = %x", qb_body_ptr -> as_len );
    printf ( "\nbody -> cmd_tmo = %x", qb_body_ptr -> cmd_tmo );
    printf ( "\nSCSI CDB:" );
    skz_dump_element ( qb_body_ptr -> cdb, 14, qb_ptr );
}

/************************************************************************
 *
 *  ROUTINE NAME:  skz_dump_data()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine prints sent or returned data, whether it was sent 
 *	via scatter-gather list or not.  It uses skz_dump_element.
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

skz_dump_data ( SIM_WS* sws, QB* qb_ptr )

{
    u_int	i;

    CCB_SCSIIO*	ccb;
    SG_ELEM*	sg_elem;

    vm_offset_t data_ptr;
    u_int	data_len;

    ccb = sws -> ccb;

    printf ( "\nTransfer Size = %d", ccb -> cam_dxfer_len );
    printf ( "\nData Buffer Address = %x", ccb -> cam_data_ptr );

    if ( !(ccb -> cam_dxfer_len > 0) )
	return ( 1 );

    if ( sws -> cam_flags & CAM_SCATTER_VALID )
    {
	sg_elem = (SG_ELEM *) ccb -> cam_data_ptr;
        for ( i=0; i < ccb -> cam_sglist_cnt; i++ )
        {
            /*
            ** Walk down each element to break them up per page
            */
            data_ptr = (vm_offset_t) sg_elem -> cam_sg_address;
            data_len = (u_int) sg_elem -> cam_sg_count;

	    skz_dump_element ( data_ptr, data_len, qb_ptr );

            sg_elem++;
        }
    }
    else
    {
	skz_dump_element ( ccb -> cam_data_ptr, 
			   (u_int) ccb -> cam_dxfer_len,
			   qb_ptr );
    }

    return ( 1 );

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_dump_element()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine prints bytes of data with a space between, given
 *	a pointer and a length.
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


skz_dump_element ( char* data_ptr, u_int data_len, QB* qb_ptr )

{
    kern_return_t	kern_status;
    u_int	error;

    u_int	i;
    u_char*	kva;
    u_int	klen;

    static	vm_offset_t kva_dup;
    static	first_time = 1;

    pmap_t	map;
    QB_DRIVER*	qb_driver_ptr;
    SKZ_DME_DSC*	dme_dsc_ptr;

    if ( first_time )
    {
	kva_dup = vm_alloc_kva ( XZA_PORT_PAGE_SIZE * 2 );
	first_time = 0;
    }

    qb_driver_ptr = &(qb_ptr -> qb_driver);
    dme_dsc_ptr = &(qb_driver_ptr -> dme_dsc);


    if ( IS_SYS_VA(data_ptr) )
    {
	printf ( "\ndump_element: kernel address" );
	kva = (u_char *) data_ptr;
	klen = data_len;
    }
    else
    {
	printf ( "\nUser space address" );

	if ( data_len > XZA_PORT_PAGE_SIZE*2 )
	    klen = XZA_PORT_PAGE_SIZE*2;
	else
	    klen = data_len;

	map = ((struct buf *) dme_dsc_ptr -> bp) -> b_proc -> task -> map -> vm_pmap;
	kern_status = pmap_dup ( map, data_ptr, klen, kva_dup, 
			VM_PROT_WRITE, TB_SYNC_LOCAL );
	printf ( "\nkern_status from pmap_dup = %x", kern_status );

 	kva = (u_char *) kva_dup;

    }

    

    for ( i=0; i<klen; i++ )
    {
	if ( i % 8 == 0 )
	    printf ( "\n" );

 	printf ( "%02x ", *kva );
	kva++;
    }

    return ( 1 );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_dump_eb()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine dumps the contents of an skz error buffer
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

skz_dump_eb ( SKZ_ERR_BUF* eb_ptr )

{

    printf ( "\neb_ptr -> error_code = %d", eb_ptr -> error_code );
    printf ( "\neb_ptr -> error_flags = %d", eb_ptr -> error_flags );
    printf ( "\neb_ptr -> reg_mask = %d", eb_ptr -> reg_mask );

    printf ( "\neb_ptr -> xza_softc = %x", eb_ptr -> xza_softc );
    printf ( "\neb_ptr -> reg_entry.xdev = %x", eb_ptr -> reg_entry.xdev );
    printf ( "\neb_ptr -> reg_entry.asr = %x", eb_ptr -> reg_entry.asr );

    return ( 1 );
}
