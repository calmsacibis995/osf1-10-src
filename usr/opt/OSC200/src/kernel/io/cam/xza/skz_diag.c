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
 *
 * Log History from before file was renamed
 *
 * Revision 1.1.2.5  92/06/26  14:00:35  Ronald_Hegli
 * 	Changed to reflect CAM header files moving from sys to io/cam.
 * 	[92/06/26  13:54:36  Ronald_Hegli]
 * 
 * Revision 1.1.2.4  92/06/18  15:50:12  Ronald_Hegli
 * 	Minor changes to reflect data structure changes.
 * 	[92/06/18  15:42:49  Ronald_Hegli]
 * 
 * Revision 1.1.2.3  92/06/07  10:42:19  Ronald_Hegli
 * 	Small changes so that this file will continue to build, resulting
 * 	from code review changes made in other files.
 * 	[92/06/07  10:34:37  Ronald_Hegli]
 * 
 * Revision 1.1.2.2  92/03/31  17:28:47  Ronald_Heglie
 * 	Initial Submission for LASER/Ruby XZA/SCSI Support
 * 	[92/03/31  17:26:46  Ronald_Heglie]
 * 
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: skz_diag.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/01/08 17:50:31 $";
#endif


/************************************************************************
 *									*
 * File:	skz_diag.c						*
 * Date:	January 10, 1992					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *	This file contains routines which implement the XZA diagnostic  *
 * functions, including EEPROM updates and reading various error and    *
 * parameter information from the adapter.				*
 *									*
 ************************************************************************/

/*
** Include Files
*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/fcntl.h>
#include <sys/proc.h>
#include <sys/buf.h>

#include <vm/vm_kern.h>
#include <mach/kern_return.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>
#include <machine/rpb.h>

#include <io/common/iotypes.h>
#include <io/cam/cam.h>

#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim_common.h>
#include <io/cam/sim.h>

#include <io/cam/xza/skz_params.h>
#include <io/cam/xza/skz_error.h>
#include <io/cam/xza/skz_xza.h>
#include <io/cam/xza/skz_nport.h>
#include <io/cam/xza/skz_xza_qb.h>
#include <io/cam/xza/skz.h>


#include <io/common/devdriver.h>
#include <io/dec/xmi/xmireg.h>
#include <io/dec/mbox/mbox.h>


extern XZA_SOFTC* xza_softc_directory[];

skz_read_errors ( SIM_WS* , u_char );


/************************************************************************
 *
 *  ROUTINE NAME:  skz_diag_command()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	All CAM requests directed to the XZA's scsi id come here,
 *	as almost always ( except during installation ) these are
 *	diag commands sent by scu
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

skz_diag_command ( SIM_WS* sws )

{
    u_int		skz_status = CAM_REQ_CMP;
    CCB_SCSIIO*		ccb;

    ccb = sws -> ccb;

    switch ( ccb -> opcode ) {

	case RDCNT:

	    skz_read_count ( sws );
	    break;

	case SETDIAG:

	    switch ( ccb -> diag_opcode ) {
	
		case EE$WriteParam:
		    skz_write_scsi_id ( sws );
		    break;

		case EE$ReadParam:
		    skz_read_params ( sws );
		    break;

		case EE$ReadErrHrd:
		    skz_read_errors ( sws, (u_char) EE$ReadErrHrd );
		    break;

		case EE$ReadErrSoft:
		    skz_read_errors ( sws, (u_char) EE$ReadErrSoft );
		    break;

		case EE$WriteImage:
		    skz_write_image ( sws );
		    break;

		case EE$ReadImage:
		    skz_read_image ( sws );
		    break;

		case EE$WriteEEPROM:
		    skz_write_eeprom ( sws );
		    break;

		case EE$ReadEEPROM:
		    skz_read_eeprom ( sws );
		    break;

		case NCR$ReadReg:
		    skz_read_ncr_regs ( sws );
		    break;

		default:
	    	    /*
	    	    ** It must be a regular I/O directed at the XZA's id
	    	    ** Set the status to 'target invalid' and complete the 
	    	    ** command.
	    	    */
	    	    skz_status = CAM_TID_INVALID;

		    printf ( "\nInvalid diag opcode" );
		    break;

	    } /* switch */

	    break;

	default:

	    /*
	    ** It must be a regular I/O directed at the XZA's id
	    ** Set the status to 'target invalid' and complete the 
	    ** command.
	    */
	    skz_status = CAM_TID_INVALID;

	    break;

    } /* switch */

    return ( skz_status );
}



/************************************************************************
 *
 *  ROUTINE NAME:  skz_read_count()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine issues an XZA RDCNT command and returns the 
 * 	various counters stored in the XZA adapter.
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

skz_read_count( SIM_WS* sws )

{
    unsigned int  skz_status;

    QB*         	qb_ptr;
    NP_QB_HEADER*	qb_hdr_ptr;
    RDCNT_CMD*		qb_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_CHANNEL*        xza_chan;
    XZA_SOFTC*          xza_softc;

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    qb_hdr_ptr = (NP_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (RDCNT_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = RDCNT;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;

    /*
    ** Issue command by inserting onto the adapter's command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
        return ( CAM_PROVIDE_FAIL );

    return ( CAM_REQ_CMP );
                                                                                
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_read_errors()
 *
 *  FUNCTIONAL DESCRIPTION:
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

skz_read_errors ( SIM_WS* sws, u_char code )

{
    u_int 		skz_status;

    QB*         	qb_ptr;
    SETDIAG_QB_HEADER*	qb_hdr_ptr;
    READERRSOFT_CMD*	qb_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_CHANNEL*        xza_chan;
    XZA_SOFTC*          xza_softc;

    SKZ_DME_DSC*	dme_dsc_ptr;

    SIM_MODULE(skz_read_errors);

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    qb_hdr_ptr = (SETDIAG_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (READERRSOFT_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );
    dme_dsc_ptr = &(qb_driver_ptr -> dme_dsc);

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SETDIAG;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */
    qb_hdr_ptr -> diag_opc = code;

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;


    skz_status = skz_dme_setup ( sws, dme_dsc_ptr,  xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
        skz_error ( module,
                    SKZ_ERROR,
                    skz_status,
                    NULL,
                    xza_softc );
        /*
        ** attempt to put the Q Buffer back on the free queue, and
        ** ignore the return status for now
        */
        skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

        return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** Fill in the qb_body information
    */
    qb_body_ptr -> ptr_0 = dme_dsc_ptr -> buf_root_ptr_0;
    qb_body_ptr -> ptr_1 = dme_dsc_ptr -> buf_root_ptr_1;
    qb_body_ptr -> buf_offset = dme_dsc_ptr -> xfer_offset;
    qb_body_ptr -> buf_len = dme_dsc_ptr -> xfer_len;
                                                                             
    /*
    ** Issue command by inserting onto the adapter's command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
        return ( CAM_PROVIDE_FAIL );

    return ( CAM_REQ_CMP );
                                                                                
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_write_image()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine udpates the XZA firmware
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

skz_write_image ( SIM_WS* sws )

{
    u_int 		skz_status;

    QB*         	qb_ptr;
    SETDIAG_QB_HEADER*	qb_hdr_ptr;
    WRITEIMAGE_CMD*	qb_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_CHANNEL*        xza_chan;
    XZA_SOFTC*          xza_softc;

    SKZ_DME_DSC*	dme_dsc_ptr;

    SIM_MODULE(skz_write_image);

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    qb_hdr_ptr = (SETDIAG_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (WRITEIMAGE_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );
    dme_dsc_ptr = &(qb_driver_ptr -> dme_dsc);

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SETDIAG;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */
    qb_hdr_ptr -> diag_opc = EE$WriteImage;

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;

    /*
    ** Map the data for transfer
    */
    skz_status = skz_dme_setup ( sws, dme_dsc_ptr,  xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
        skz_error ( module,
                    SKZ_ERROR,
                    skz_status,
                    NULL,
                    xza_softc );
        /*
        ** attempt to put the Q Buffer back on the free queue, and
        ** ignore the return status for now
        */
        skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

        return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** Fill in the qb_body information
    */
    qb_body_ptr -> buf_ptr = dme_dsc_ptr -> buf_root_ptr_0;
    qb_body_ptr -> buf_offset = dme_dsc_ptr -> xfer_offset;
    qb_body_ptr -> buf_len = dme_dsc_ptr -> xfer_len;
                                                                             
    /*
    ** Issue command by inserting onto the adapter's command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
        return ( CAM_PROVIDE_FAIL );

    return ( CAM_REQ_CMP );

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_read_image()
 *
 *  FUNCTIONAL DESCRIPTION:
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

skz_read_image ( SIM_WS* sws )

{
    u_int 		skz_status;

    QB*         	qb_ptr;
    SETDIAG_QB_HEADER*	qb_hdr_ptr;
    READIMAGE_CMD*	qb_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_CHANNEL*        xza_chan;
    XZA_SOFTC*          xza_softc;

    SKZ_DME_DSC*	dme_dsc_ptr;

    SIM_MODULE(skz_read_image);

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    qb_hdr_ptr = (SETDIAG_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (READIMAGE_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );
    dme_dsc_ptr = &(qb_driver_ptr -> dme_dsc);

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SETDIAG;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */
    qb_hdr_ptr -> diag_opc = EE$ReadImage;

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;

    /*
    ** Map the data for transfer
    */
    skz_status = skz_dme_setup ( sws, dme_dsc_ptr,  xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
        skz_error ( module,
                    SKZ_ERROR,
                    skz_status,
                    NULL,
                    xza_softc );
        /*
        ** attempt to put the Q Buffer back on the free queue, and
        ** ignore the return status for now
        */
        skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

        return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** Fill in the qb_body information
    */
    qb_body_ptr -> buf_ptr = dme_dsc_ptr -> buf_root_ptr_0;
    qb_body_ptr -> buf_offset = dme_dsc_ptr -> xfer_offset;
    qb_body_ptr -> buf_len = dme_dsc_ptr -> xfer_len;
                                                                             
    /*
    ** Issue command by inserting onto the adapter's command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    return ( CAM_REQ_CMP );

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_write_eeprom()
 *
 *  FUNCTIONAL DESCRIPTION:
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

skz_write_eeprom ( SIM_WS* sws )

{
    u_int 		skz_status;

    QB*         	qb_ptr;
    SETDIAG_QB_HEADER*	qb_hdr_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_CHANNEL*        xza_chan;
    XZA_SOFTC*          xza_softc;

    SIM_MODULE(skz_write_eeprom);

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    qb_hdr_ptr = (SETDIAG_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SETDIAG;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */
    qb_hdr_ptr -> diag_opc = EE$WriteEEPROM;

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;

    /*
    ** Issue command by inserting onto the adapter's command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    return ( CAM_REQ_CMP );

}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_read_eeprom()
 *
 *  FUNCTIONAL DESCRIPTION:
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

skz_read_eeprom ( SIM_WS* sws )

{
    u_int 		skz_status;

    QB*         	qb_ptr;
    SETDIAG_QB_HEADER*	qb_hdr_ptr;
    READEEPROM_CMD*	qb_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_CHANNEL*        xza_chan;
    XZA_SOFTC*          xza_softc;

    SKZ_DME_DSC*	dme_dsc_ptr;

    SIM_MODULE(skz_read_eeprom);

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free queue
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    qb_hdr_ptr = (SETDIAG_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (READEEPROM_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );
    dme_dsc_ptr = &(qb_driver_ptr -> dme_dsc);

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SETDIAG;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */
    qb_hdr_ptr -> diag_opc = EE$ReadEEPROM;

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;

    /*
    ** Map the data for transfer
    */
    skz_status = skz_dme_setup ( sws, dme_dsc_ptr,  xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
        skz_error ( module,
                    SKZ_ERROR,
                    skz_status,
                    NULL,
                    xza_softc );
        /*
        ** attempt to put the Q Buffer back on the free queue, and
        ** ignore the return status for now
        */
        skz_status = skz_put_queue ( DDFQ, qb_ptr, car_ptr, xza_softc );

        return ( CAM_PROVIDE_FAIL );
    }

    /*
    ** Fill in the qb_body information
    */
    qb_body_ptr -> buf_ptr = dme_dsc_ptr -> buf_root_ptr_0;
    qb_body_ptr -> buf_offset = dme_dsc_ptr -> xfer_offset;
    qb_body_ptr -> buf_len = dme_dsc_ptr -> xfer_len;
                                                                             
    /*
    ** Issue command by inserting onto the adapter's command queue
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( CAM_PROVIDE_FAIL );

    return ( CAM_REQ_CMP );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_write_eeprom_complete()
 *
 *  FUNCTIONAL DESCRIPTION:
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

skz_write_eeprom_complete ( XZA_SOFTC* xza_softc )

{
    u_int	skz_status;

    u_int	delay;
    vm_offset_t	wait_event;
    int		s;

    XBE_REG	xbe;
    XPUD_REG	xpud;

    SIM_MODULE(skz_write_eeprom_complete);


    /*
    ** Wait for 10 seconds
    */
    DELAY ( SKZ_ONE_SECOND_DELAY * 10 );

    /*
    ** Poll for xpud.unin
    */
    /*
    ** First read of XPUD ( ASR )
    */
    skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( skz_status );

    delay = 0;

    /*
    ** Poll for xpud.unin a little differently if we're threaded
    */
    if ( xza_softc -> flags & SKZ_THREADS_ACTIVE )
    {
	if ( SKZ_ALREADY_WAITING )
	    SKZ_SAVE_WAIT;

	while ( !(xpud.unin) && (delay < XZA_ADAPTER_HALT_TIME) )
	{
	    /*
	    ** Must 'assert_wait' before calling thread_set_timeout.
	    */
	    assert_wait((vm_offset_t)delay, TRUE );

	    /* wait one second */
            thread_set_timeout ( hz );
            thread_block();
	    delay++;

	    /*
	    ** RE-read the register
	    */
	    skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
            if ( skz_status != CAM_REQ_CMP )
	 	return ( skz_status );
	}

	SKZ_RESTORE_WAIT;
    }
    else
    {
	while ( !(xpud.unin) && (delay < XZA_ADAPTER_HALT_TIME) )
	{
	    DELAY ( SKZ_ONE_SECOND_DELAY );
	    delay++;

	    /*
	    ** RE-read the register
	    */
	    skz_status = skz_read_reg ( XPUD, &xpud, xza_softc );
            if ( skz_status != CAM_REQ_CMP )
	 	return ( skz_status );

	}

    }

    SIM_PRINTD (NOBTL,
		NOBTL,
		NOBTL,
		CAMD_FLOW,
		("\n%s: XPUD = %x", module, xpud) );
    /*
    ** Verify timeout or state change
    */
    if ( !(xpud.unin) )
	return ( CAM_REQ_CMP_ERR );

    /*
    ** Set the adapter state to UNINITIALIZED
    */
    SKZ_LOCK ( xza_softc -> lock, s );
    xza_softc -> channel[0].state = UNINITIALIZED;
    xza_softc -> channel[1].state = UNINITIALIZED;
    SKZ_UNLOCK ( xza_softc -> lock, s );

    /*
    ** Reset the adapter
    */
    skz_status = skz_reset_adapter ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    /*
    ** Read the XBE and check for self-test errors
    */
    skz_status = skz_read_reg ( XBE, &xbe, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    if ( xbe.stf )
    {
	/* freak out */
    }


    /*
    ** Re-initialize the adapter
    */
    skz_status = skz_init ( xza_softc );
    if ( skz_status != CAM_REQ_CMP )
    {
	skz_error (	module,
			SKZ_ERROR,
			skz_status,
			NULL,
			xza_softc );
	return ( CAM_REQ_CMP_ERR );
    }

    return ( CAM_REQ_CMP );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_read_params()
 *
 *  FUNCTIONAL DESCRIPTION:
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

skz_read_params ( SIM_WS* sws )

{
    u_int	  	skz_status;

    QB*			qb_ptr;
    SETDIAG_QB_HEADER*  qb_hdr_ptr;
    READPARAM_CMD*    	qb_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_SOFTC*		xza_softc;
    XZA_CHANNEL*	xza_chan;

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free list
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( 0 );

    /*
    ** Initialize the Q_Buffer for the setdiag ee$writeimage command
    */
    qb_hdr_ptr = (SETDIAG_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (READPARAM_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SETDIAG;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */
    qb_hdr_ptr -> diag_opc = EE$ReadParam;

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;

    /*
    ** Insert the Q_Buffer onto the channel's command queue
    ** Pick channel 0, since either will do
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( 0 );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_write_scsi_id()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine udpates the XZA eeprom parameters
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

skz_write_scsi_id ( SIM_WS* sws )

{
    u_int		skz_status;

    CCB_SCSIIO*		ccb;

    QB*         	qb_ptr;
    SETDIAG_QB_HEADER*  qb_hdr_ptr;
    WRITEPARAM_CMD*    	qb_body_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_SOFTC*		xza_softc;
    XZA_CHANNEL*	xza_chan;

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    ccb = sws -> ccb;

    /*
    ** Get a Q_Buffer from the free list
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( 0 );

    /*
    ** Initialize the Q_Buffer for the setdiag ee$writeimage command
    */
    qb_hdr_ptr = (SETDIAG_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_body_ptr = (WRITEPARAM_CMD *) ( qb_ptr -> qb_body );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SETDIAG;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */
    qb_hdr_ptr -> diag_opc = EE$WriteParam;

    /*
    ** Initialize the qb body
    */
    qb_body_ptr -> eeprm_fcn = xza_softc -> eeprm_fcn;

    if ( qb_hdr_ptr -> chnl_idx == 0 )
    {
	qb_body_ptr -> eeprm_fcn.uc0id = 1;
	qb_body_ptr -> eeprm_fcn.uc1id = 0;
	qb_body_ptr -> c0_id = ccb -> xza_scsi_id;
	qb_body_ptr -> c1_id = xza_softc -> channel[1].scsi_id.targid;
    }
    else
    {
	qb_body_ptr -> eeprm_fcn.uc1id = 1;
	qb_body_ptr -> eeprm_fcn.uc0id = 0;
	qb_body_ptr -> c1_id = ccb -> xza_scsi_id;
	qb_body_ptr -> c0_id = xza_softc -> channel[0].scsi_id.targid;
    }

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;

    /*
    ** Insert the Q_Buffer onto the channel's command queue
    ** Pick channel 0, since either will do
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( 0 );

    return ( 1 );
}


/************************************************************************
 *
 *  ROUTINE NAME:  skz_read_ncr_regs()
 *
 *  FUNCTIONAL DESCRIPTION:
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

skz_read_ncr_regs ( SIM_WS* sws )

{
    u_int	  	skz_status;

    QB*			qb_ptr;
    SETDIAG_QB_HEADER*  qb_hdr_ptr;
    QB_DRIVER*  	qb_driver_ptr;

    CARRIER*    	car_ptr;

    XZA_SOFTC*		xza_softc;
    XZA_CHANNEL*	xza_chan;

    /*
    ** setup
    */
    xza_chan = (XZA_CHANNEL *) sws -> sim_sc -> hba_sc;
    xza_softc = (XZA_SOFTC *) xza_chan -> xza_softc;

    /*
    ** Get a Q_Buffer from the free list
    */
    skz_status = skz_get_queue ( DDFQ, &qb_ptr, &car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( 0 );

    /*
    ** Initialize the Q_Buffer for the setdiag NCR$ReadReg command
    */
    qb_hdr_ptr = (SETDIAG_QB_HEADER *) &( qb_ptr -> qb_header );
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    /*
    ** Initialize Q_Buffer fields
    */
    qb_hdr_ptr -> opc = SETDIAG;
    qb_hdr_ptr -> chnl_idx = xza_chan -> chan_num;
    qb_hdr_ptr -> flags.r = 1;                  /* always get response */
    qb_hdr_ptr -> diag_opc = NCR$ReadReg;

    /*
    ** Initialize the Q_Buffer driver portion
    */
    qb_driver_ptr -> xza_chan_ptr = (vm_offset_t) xza_chan;
    qb_driver_ptr -> sws = (vm_offset_t) sws;

    /*
    ** Insert the Q_Buffer onto the channel's command queue
    ** Pick channel 0, since either will do
    */
    skz_status = skz_put_queue ( DCCQ0, qb_ptr, car_ptr, xza_softc );
    if ( skz_status != CAM_REQ_CMP )
	return ( 0 );
}



/************************************************************************
 *
 *  ROUTINE NAME:  skz_copyout()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine copies Q Buffer contents into a user's data buffer.
 *	It's needed because we need to copy data that isn't mapped in 
 *	skz_dme_setup() from a QB in kernel space into a user's data
 *	buffer from a kernel thread context.  The normal copyout won't
 *	work..
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

skz_copyout ( QB* qb_ptr )

{
    u_int		skz_status;

    char*		qb_body_ptr;
    QB_DRIVER*		qb_driver_ptr;

    CCB_SCSIIO*		ccb;

    vm_offset_t		phys_addr;

    /*
    ** Set up
    */
    qb_body_ptr = (char *)  qb_ptr -> qb_body;
    qb_driver_ptr = (QB_DRIVER *) &( qb_ptr -> qb_driver );

    ccb = ((SIM_WS *) qb_driver_ptr -> sws) -> ccb;

    /*
    ** Get physical address of user's buffer
    */
    phys_addr = pmap_extract (
      ((struct buf *) ccb -> cam_req_map) -> b_proc -> task -> map -> vm_pmap,
        ccb -> cam_data_ptr );
    if ( phys_addr == NULL )
        return ( SKZ_PHYS_CONV_ERR );

    /*
    ** Copy to the user buffer
    */
    copy_to_phys ( qb_body_ptr, phys_addr, ccb -> cam_dxfer_len);

    return ( CAM_REQ_CMP );
}
