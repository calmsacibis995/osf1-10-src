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
static char *rcsid = "@(#)$RCSfile: scu_xzadiag.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/07/21 14:03:40 $";
#endif

/****************************************************************************
 *
 * File:	scu_xzadiag.c
 * Author:	Ron Hegli
 * Date:	June 29, 1992	
 *
 * Description:
 *	This file contains routines which make use of the CAM User Agent
 *	Driver to send XZA Diagnostic commands to the adapter.  
 *	
 *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#include <io/common/iotypes.h>
#include <io/cam/rzdisk.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"

#include <io/cam/xza/skz_xza.h>
#include <io/cam/xza/skz_xza_qb.h>

/************************************************************************
 *									*
 * XZASoftErrs() - 							*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZASoftErrs (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	int 		status;
	int 		adapt_id;

	char*		xza_err_buf;

	CCB_SCSIIO	*ccb;

	/*
	** Allocate the error buffer
	*/
	xza_err_buf = (char *) malloc_palign ( XZA_ERR_BUF_SIZE );
 	bzero ( xza_err_buf, XZA_ERR_BUF_SIZE );

        /*
	** Get the Adapter's scsi id
	*/
	adapt_id = XZAGetAdaptId();

	/*
	** Initialize the diag CCB
	*/
	ccb = (CCB_SCSIIO *) malloc_palign ( sizeof ( CCB_SCSIIO ) );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_data_ptr = (u_char *) xza_err_buf;
	ccb -> cam_dxfer_len = sizeof ( xza_err_buf );

	ccb -> cam_ch.cam_flags |= CAM_DIR_IN;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;
	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$ReadErrSoft;

	status = xpt_action ( ccb );
	
	if ( status == SUCCESS )
	    XZAPrintErrs ( xza_err_buf );

	/*
	** Free allocated memory
	*/
	free_palign ( ccb );
	free_palign ( xza_err_buf );


	return ( 0 );
}


/************************************************************************
 *									*
 * XZAHardErrs() - 							*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAHardErrs (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	int 		status;
	int		adapt_id;

	char*		xza_err_buf;

	CCB_SCSIIO	*ccb;

	/*
	** Allocate the error buffer
	*/
	xza_err_buf = (char *) malloc_palign ( XZA_ERR_BUF_SIZE );
 	bzero ( xza_err_buf, XZA_ERR_BUF_SIZE );

        /*
	** Get the adapter's scsi id
	*/
	adapt_id = XZAGetAdaptId();

	/*
	** Initialize the diag CCB
	*/
	ccb = (CCB_SCSIIO *) malloc_palign ( sizeof ( CCB_SCSIIO ) );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_data_ptr = (u_char *) xza_err_buf;
	ccb -> cam_dxfer_len = sizeof ( xza_err_buf );

	ccb -> cam_ch.cam_flags |= CAM_DIR_IN;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;
	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$ReadErrHrd;

	status = xpt_action ( ccb );
	
	if ( status == SUCCESS )
	    XZAPrintErrs ( xza_err_buf );

	/*
	** Free allocated memory
	*/
	free_palign ( ccb );
	free_palign ( xza_err_buf );

	return ( 0 );
}


/************************************************************************
 *									*
 * XZAReadCnt() - 							*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAReadCnt (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	u_int 		status;
	int		adapt_id;

	char*		rdcnt_data_ptr;

	CCB_SCSIIO	*ccb;		/* XZA diagnostic ccb */

	/*
	** Allocate data buffer
	*/
	rdcnt_data_ptr = (char *) malloc_palign ( sizeof ( RDCNT_CMD ) );
	bzero ( rdcnt_data_ptr, sizeof ( RDCNT_CMD ) );

        /*
	** Get the adapter's scsi id
	*/
	adapt_id = XZAGetAdaptId();

	/*
	** Initialize the diag CCB
	*/
	ccb = (CCB_SCSIIO *) malloc_palign ( sizeof ( CCB_SCSIIO ) );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;
	ccb -> cam_ch.cam_target_lun = 0;
	ccb -> cam_ch.cam_status = 0;

	ccb -> cam_data_ptr = (u_char *) rdcnt_data_ptr;
	ccb -> cam_dxfer_len = sizeof ( RDCNT_CMD );

	ccb -> cam_ch.cam_flags |= CAM_DIR_IN;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;
	ccb -> cam_cdb_len = 6;

	ccb -> opcode = RDCNT;


	status = xpt_action ( ccb );

	if ( status == SUCCESS )
	    XZAPrintCntrData ( rdcnt_data_ptr );

	/*
	** Free allocated memory
	*/
	free_palign ( ccb );
	free_palign ( rdcnt_data_ptr );

	return (status);
}

/************************************************************************
 *									*
 * XZAShowParams() - 							*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAShowParams (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	u_int 		status;
	int		adapt_id;

	READPARAM_CMD*	params_ptr;

	CCB_SCSIIO	*ccb;		/* XZA diagnostic ccb */

        /*
	** Get the adapter's scsi id
	*/
	adapt_id = XZAGetAdaptId();

	/*
	** Allocate space for for the parameter data
	*/
	params_ptr = (READPARAM_CMD *) malloc_palign (sizeof ( READPARAM_CMD ));
	bzero ( params_ptr, sizeof ( READPARAM_CMD ) );

	/*
	** Initialize the diag CCB
	*/
	ccb = (CCB_SCSIIO *) malloc_palign ( sizeof ( CCB_SCSIIO ) );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_ch.cam_flags |= CAM_DIR_IN;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;

	ccb -> cam_data_ptr = (u_char *) params_ptr;
	ccb -> cam_dxfer_len = sizeof ( READPARAM_CMD );
	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$ReadParam;


	status = xpt_action ( ccb );
	if ( status == SUCCESS )
	    XZAPrintParams ( params_ptr );

	/*
	** Free allocated memory
	*/
	free_palign ( ccb );
	free_palign ( params_ptr );

	return (status);
}


/************************************************************************
 *									*
 * XZASetParams() - 							*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZASetId (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	u_int 		status;
	int		adapt_id;

	CCB_SCSIIO	*ccb;		/* XZA diagnostic ccb */

        /*
	** Get the adapter's scsi id
	*/
	adapt_id = XZAGetAdaptId();

	/*
	** Initialize the diag CCB
	*/
	ccb = (CCB_SCSIIO *) malloc_palign ( sizeof ( CCB_SCSIIO ) );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_ch.cam_flags |= CAM_DIR_IN;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;

	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$WriteParam;
	ccb -> xza_scsi_id = UserSCSIID;

	/*
	** Send the CCB to the User Agent driver
	*/
	status = xpt_action ( ccb );

	/*
	** Free allocated memory
	*/
	free_palign ( ccb );

	return (status);
}



/************************************************************************
 *									*
 * XZAShowNCRRegs() - 							*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAShowNCRRegs (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	u_int 		status;
	int		adapt_id;

	char*		ncr_reg_data;

	CCB_SCSIIO	*ccb;	


	/*
	** Allocate data buffer
	*/
	ncr_reg_data = (char *) malloc_palign ( sizeof ( READREG_CMD ) );
	bzero ( ncr_reg_data, sizeof ( READREG_CMD ) );

	/*
	** Get the adapter's scsi id
	*/
	adapt_id = XZAGetAdaptId();

	/*
	** Initialize the diag CCB
	*/
	ccb = (CCB_SCSIIO *) malloc_palign ( sizeof ( CCB_SCSIIO ) );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_data_ptr = (u_char *) ncr_reg_data;
	ccb -> cam_dxfer_len = sizeof ( READREG_CMD );

	ccb -> cam_ch.cam_flags |= CAM_DIR_IN;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;

	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = NCR$ReadReg;


	status = xpt_action ( ccb );

	if ( status == SUCCESS )
	    XZAPrintNCRRegs ( ncr_reg_data );

	/*
	** Free allocated memory
	*/
	free_palign ( ccb );
	free_palign ( ncr_reg_data );

	return (status);
}




/************************************************************************
 *									*
 * XZAWriteImage() - Write firmware image to adapter memory		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
XZAWriteImage (ce)
struct cmd_entry *ce;
{
	register struct scu_device *scu = ScuDevice;
	char *fnp = (char *) *(long *) ce->c_argp;
	struct stat sb;
	FILE *fp = (FILE *) 0;
	caddr_t bp, verify_bp;
	int count, fd, size;
	int offset = 0, status;
	u_char buf_id = 0, mode;

	int		adapt_id;
	CCB_SCSIIO	*ccb;		/* XZA diagnostic ccb */


	
	if ( (status = OpenInputFile (&fp, fnp)) != SUCCESS) {
	    return (status);
	}
	fd = fileno (fp);
	if ( (status = fstat (fd, &sb)) < 0) {
	    Perror ("fstat");
	    (void) fclose (fp);
	    return (status);
	}

	/*
	 * Only expecting regular files at this time.
	 */
	if ((sb.st_mode & S_IFMT) != S_IFREG) {
	    Fprintf ("File '%s' is NOT a regular file.", fnp);
	    (void) fclose (fp);
	    return (FAILURE);
	}
	size = sb.st_size;
	/*
	 * To simplify the download operation, we will allocate and read
	 * in the entire image to be downloaded (as opposed to segments).
	 */
	if ( (bp = (caddr_t) malloc_palign (size)) == NULL) {
	    Fprintf ("Failed to allocate file buffer of %d bytes.", size);
	    (void) fclose (fp);
	    return (FAILURE);
	}

	if ( (count = read (fd, bp, size)) != size) {
	    Fprintf ("Error occured reading download file.");
	    if (count == FAILURE) {
		Perror ("read");
	    } else {
		Fprintf ("Attempted to read %d bytes, read only %d bytes.",
								size, count);
	    }
	    (void) fclose (fp);
	    free_palign (bp);
	    return (FAILURE);
	}

	CloseFile (fp);

	if (DisplayVerbose) {
	    Printf ("Downloading File '%s' of %d bytes...\n", fnp, size);
	}

	/*
	** Contruct SCSIIO CCB to transfer the Image
	*/
        /*
	** Get the adapter's scsi id
	*/
	adapt_id = XZAGetAdaptId();

	/*
	** Initialize the diag CCB
	*/
	ccb = (CCB_SCSIIO *) malloc_palign ( sizeof ( CCB_SCSIIO ) );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_ch.cam_flags |= CAM_DIR_OUT;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;

	ccb -> cam_data_ptr = (u_char *) bp;
	ccb -> cam_dxfer_len = size;
	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$WriteImage;

	/*
	** Send the CCB to the User Agent driver
	*/
	status = xpt_action ( ccb );

	if ( status != SUCCESS )
	    return ( FAILURE );

	/*
	** Read the image back from adapter memory and verify that it
	** was transferred correctly
	**
	** Issue the EE$ReadImage command to copy the image from
	** adapter memory to host memory
	*/

	/*
	** Allocate another buffer to read the image into
	*/
	if ( (verify_bp = (caddr_t) malloc_palign (size)) == NULL) {
	    Fprintf ("Failed to allocate file buffer of %d bytes.", size);
	    return (FAILURE);
	}
	bzero ( verify_bp, size );


	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_ch.cam_flags |= CAM_DIR_IN;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;

	ccb -> cam_data_ptr = (u_char *) verify_bp;
	ccb -> cam_dxfer_len = size;
	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$ReadImage;

	/*
	** Send the CCB to the User Agent driver
	*/
	status = xpt_action ( ccb );

	if ( status != SUCCESS )
	    return ( FAILURE );

	/*
	** Validate the image
	*/
	status = XZAValidateImage ( bp, verify_bp );

	if ( status != SUCCESS )
	{
	    printf ( "\nAdapter Memory Image failed to validate" );
	    return ( FAILURE );
	}


	/*
	** Issue the EE$WriteEEPROM command to copy the image from
	** adapter memory to EEPROM
	*/
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_ch.cam_flags |= CAM_DIR_OUT;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;

	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$WriteEEPROM;

	/*
	** Send the CCB to the User Agent driver
	*/
	status = xpt_action ( ccb );

	if ( status != SUCCESS )
	    return ( FAILURE );


	/*
	** Read the image back from the adapter's EEPROM and verify that it
	** was transferred correctly
	**
	** Issue the EE$ReadEEPROM command to copy the image from
	** EEPROM to host memory.
	*/
	bzero ( verify_bp, size );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_ch.cam_flags |= CAM_DIR_OUT;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;

	ccb -> cam_data_ptr = (u_char *) verify_bp;
	ccb -> cam_dxfer_len = size;
	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$ReadEEPROM;

	/*
	** Send the CCB to the User Agent driver
	*/
	status = xpt_action ( ccb );

	if ( status != SUCCESS )
	    return ( FAILURE );

	status = XZAValidateImage ( bp, verify_bp );
	
	if ( status != SUCCESS )
	    printf ( "\nEEPROM Image Failed Validation" );

	/*
	** Free allocated memory
	*/
	free_palign ( ccb );
	free_palign (bp);

	return (status);
}


/************************************************************************
 *									*
 * XZAReadImage() - Read Firmware image from adapter memory		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
XZAReadImage (ce)
struct cmd_entry *ce;
{

	u_int 		status;
	int		adapt_id;

	caddr_t		bp;

	CCB_SCSIIO	*ccb;		/* XZA diagnostic ccb */


	/*
	** Allocate a buffer to read the image into
	*/
	if ( (bp = (caddr_t) malloc_palign (XZA_FW_IMAGE_LEN)) == NULL) {
	    Fprintf ("Failed to allocate file buffer of %d bytes.", 
			XZA_FW_IMAGE_LEN );
	    return (FAILURE);
	}

        /*
	** Get the adapter's scsi id
	*/
	adapt_id = XZAGetAdaptId();

	/*
	** Initialize the diag CCB
	*/
	ccb = (CCB_SCSIIO *) malloc_palign ( sizeof ( CCB_SCSIIO ) );
	bzero ( ccb, sizeof ( CCB_SCSIIO ) );

	ccb -> cam_ch.my_addr = (CCB_HEADER *) ccb;
	ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_SCSIIO );
	ccb -> cam_ch.cam_func_code = XPT_SCSI_IO;
	ccb -> cam_ch.cam_path_id = UserBus;
	ccb -> cam_ch.cam_target_id = adapt_id;

	ccb -> cam_ch.cam_flags |= CAM_DIR_IN;
	ccb -> cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;

	ccb -> cam_data_ptr = (u_char *) bp;
	ccb -> cam_dxfer_len = XZA_FW_IMAGE_LEN;
	ccb -> cam_cdb_len = 6;

	ccb -> opcode = SETDIAG;
	ccb -> diag_opcode = EE$ReadImage;

	/*
	** Send the CCB to the User Agent driver
	*/
	status = xpt_action ( ccb );

	if ( status != SUCCESS )
	    return ( FAILURE );

	/*
	** Write the image out onto the file
	*/




	/*
	** Free allocated memory
	*/
	free_palign ( ccb );
	free_palign ( bp );

}



/************************************************************************
 *									*
 * XZAGetAdaptId() - 							*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAGetAdaptId ()

{
	u_int		status;
	int		adapt_id;

  	CCB_PATHINQ	*pi_ccb;

        /*
	** First issue a PATHINQ ccd to find out what the xza's scsi id
	** is
	*/
	pi_ccb = (CCB_PATHINQ *) malloc_palign ( sizeof ( CCB_PATHINQ ) );
	bzero ( pi_ccb, sizeof ( CCB_PATHINQ ) );

	pi_ccb -> cam_ch.my_addr = (CCB_HEADER *) pi_ccb;
	pi_ccb -> cam_ch.cam_ccb_len = sizeof ( CCB_PATHINQ );
	pi_ccb -> cam_ch.cam_path_id = UserBus;	
	pi_ccb -> cam_ch.cam_func_code = XPT_PATH_INQ;

	status = xpt_action ( pi_ccb );

	/*
	** Check status
	*/
	if ( status == SUCCESS )
	    adapt_id = pi_ccb -> cam_initiator_id;
	else
	    adapt_id = -1;
	
	/*
	** Free allocated memory
	*/
	free_palign ( pi_ccb );


	return ( adapt_id );
}


/************************************************************************
 *									*
 * XZAValidateImage() - 						*
 *	This routine is used to validate a firmware image previously	*
 *	written to the adapter and read back out via XZAReadImage	*
 *	against a known good image residing on disk			*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAValidateImage ( char* bp, char* verify_bp )

{
	/*
	** Need to determine the method to use..
	** Probably just need to do a checksum,
	** could compare each byte.....
	*/


	return ( SUCCESS );
}




/*
**
** Print Routines
**
*/

/************************************************************************
 *									*
 * XZAPrintCntrData() - 						*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
XZAPrintCntrData ( RDCNT_CMD* rdcnt_data_ptr )

{
    /*
    ** Print count data
    */

    /*
    ** Command Counters
    */
    printf ( "\nCommand Counters" );
    printf ( "\n================" );
    printf ( "\nSNDSCSI : %d", 
			rdcnt_data_ptr -> sndscsi_cmd_count );
    printf ( "\nSNDVNDR : %d", 
			rdcnt_data_ptr -> sndvndr_cmd_count );
    printf ( "\nABRTCMD : %d", 
			rdcnt_data_ptr -> abrtcmd_cmd_count );
    printf ( "\nMRKABRT : %d", 
			rdcnt_data_ptr -> mrkabrt_cmd_count );
    printf ( "\nBDRST : %d", 
			rdcnt_data_ptr -> bdrst_cmd_count );
    printf ( "\nPURGQ : %d", 
			rdcnt_data_ptr -> purgq_cmd_count );
    printf ( "\nSETNEX : %d", 
			rdcnt_data_ptr -> setnex_cmd_count );
    printf ( "\nRDCNT : %d", 
			rdcnt_data_ptr -> rdcnt_cmd_count );
    printf ( "\nSETDIAG : %d", 
			rdcnt_data_ptr -> setdiag_cmd_count );
    printf ( "\nRETQE : %d", 
			rdcnt_data_ptr -> retqe_cmd_count );
    printf ( "\n" );
    printf ( "\nDIRECT-CLASS : %d", 
			rdcnt_data_ptr -> direct_class_cmd_count );
    printf ( "\nREAD-CLASS : %d", 
			rdcnt_data_ptr -> read_class_cmd_count );
    printf ( "\nWRITE-CLASS : %d", 
			rdcnt_data_ptr -> write_class_cmd_count );
    printf ( "\n" );
    printf ( "\nGCQIR Writes : %d", 
			rdcnt_data_ptr -> gcqir_write_count );
    printf ( "\nQIR Writes : %d", 
			rdcnt_data_ptr -> qir_write_count );
    printf ( "\nInquiries Received : %d", 
			rdcnt_data_ptr -> inq_rec_count );
    printf ( "\nCommands Aborted : %d", 
			rdcnt_data_ptr -> aborted_count );
    printf ( "\nAutosense Count : %d", 
			rdcnt_data_ptr -> autosense_count );


    /*
    ** Read/Write byte counters
    */
    printf ( "\n\nRead Byte Counts\tWrite Byte Counts" );
    printf ( "\n======================================================" );
    printf ( "\nNode 0 : %ld\t\t%ld", 
			rdcnt_data_ptr -> node_0_read_byte_cnt,
			rdcnt_data_ptr -> node_0_write_byte_cnt );
    printf ( "\nNode 1 : %ld\t\t%ld", 
			rdcnt_data_ptr -> node_1_read_byte_cnt,
			rdcnt_data_ptr -> node_1_write_byte_cnt );
    printf ( "\nNode 2 : %ld\t\t%ld", 
			rdcnt_data_ptr -> node_2_read_byte_cnt,
			rdcnt_data_ptr -> node_2_write_byte_cnt );
    printf ( "\nNode 3 : %ld\t\t%ld", 
			rdcnt_data_ptr -> node_3_read_byte_cnt,
			rdcnt_data_ptr -> node_3_write_byte_cnt );
    printf ( "\nNode 4 : %ld\t\t%ld", 
			rdcnt_data_ptr -> node_4_read_byte_cnt,
			rdcnt_data_ptr -> node_4_write_byte_cnt );
    printf ( "\nNode 5 : %ld\t\t%ld", 
			rdcnt_data_ptr -> node_5_read_byte_cnt,
			rdcnt_data_ptr -> node_5_write_byte_cnt );
    printf ( "\nNode 6 : %ld\t\t%ld", 
			rdcnt_data_ptr -> node_6_read_byte_cnt,
			rdcnt_data_ptr -> node_6_write_byte_cnt );
    printf ( "\nNode 7 : %ld\t\t%ld", 
			rdcnt_data_ptr -> node_7_read_byte_cnt,
			rdcnt_data_ptr -> node_7_write_byte_cnt );


    /*
    ** SCSI Event Counters
    */
    printf ( "\n\nSCSI Event Counters:" );
    printf ( "\n===================" );
    printf ( "\nBus Parity Errors : %d", 
			rdcnt_data_ptr -> bus_parity_err_cnt );
    printf ( "\nUnsolicited Reselections : %d", 
			rdcnt_data_ptr -> unsol_resel_cnt );
    printf ( "\nIllegal Phase Errors : %d", 
			rdcnt_data_ptr -> ill_phase_err_cnt );
    printf ( "\nCommand Timeouts : %d", 
			rdcnt_data_ptr -> cmd_timeout_cnt );
    printf ( "\nSelection Timeouts : %d", 
			rdcnt_data_ptr -> select_timeout_cnt );
    printf ( "\nUnexpected Disconnects : %d", 
			rdcnt_data_ptr -> unexp_discon_cnt );
    printf ( "\nBus Resets Sent : %d", 
			rdcnt_data_ptr -> bus_rst_sent_cnt );
    printf ( "\nBus Resets Received : %d", 
			rdcnt_data_ptr -> bus_rst_recv_cnt );
    printf ( "\nBus Device Resets Sent : %d", 
			rdcnt_data_ptr -> bus_dev_rst_sent_cnt );
    printf ( "\nBus Device Resets Received : %d", 
			rdcnt_data_ptr -> bus_dev_rst_recv_cnt );


    printf ( "\n\n" );

}


/************************************************************************
 *									*
 * XZAPrintErrs() - 							*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrintErrs ( char err_buf[] )
{
    u_int	i;
    u_int	invalid_records = 0;
    EB*		eb_ptr;

    eb_ptr = (EB *) &err_buf[0];

    for ( i = 0; i < XZA_MAX_ERROR_RECORDS; i++ )
    {

	switch ( (int) eb_ptr -> eb_header.err_type ) {

	    case XZA_ET_MACH_CHECK:
	
		XZAPrintErrHdr ( &( eb_ptr -> eb_header ) );
		XZAPrint_MC_NH (&(eb_ptr->eb_body.mc_nh_body));
		break;

	    case XZA_ET_NODE_HALT:

		XZAPrintErrHdr ( &( eb_ptr -> eb_header ) );
		XZAPrint_MC_NH (&(eb_ptr->eb_body.mc_nh_body));
		break;

	    case XZA_ET_AME:

		XZAPrintErrHdr ( &( eb_ptr -> eb_header ) );

		switch ( (int) eb_ptr -> eb_header.sub_type ) {

		    case XZA_EST_PEEK_TRANS:
			XZAPrint_Peek_Trans(&(eb_ptr->eb_body.peek_trans_body));
			break;

		    case XZA_EST_DATAMOVE:
			XZAPrint_Datamove(&(eb_ptr->eb_body.datamove_body));
			break;

		    case XZA_EST_HOST_INTR:
			XZAPrint_Host_Intr(&(eb_ptr->eb_body.host_intr_body));
			break;

		    case XZA_EST_XBE_RELATED:
			XZAPrint_XBE_Rel(&(eb_ptr->eb_body.xbe_related_body));
			break;

		    case XZA_EST_SIOP:
			XZAPrint_SIOP(&(eb_ptr->eb_body.siop_body));
			break;

		    default:
			printf ( "\nInvalid XZA Error Sub-Type\n" );
			break;

		} /* switch */

		break;

	    case XZA_ET_FW_UPDATE:

		XZAPrintErrHdr ( &( eb_ptr -> eb_header ) );
		XZAPrint_FW_Update(&(eb_ptr->eb_body.fw_update_body));
		break;

	    default:
		printf ( "\nIncrementing invalid_records" );
		invalid_records++;
	    	break;

	} /* switch */

	eb_ptr++;

    } /* for */

    if ( invalid_records == XZA_MAX_ERROR_RECORDS )
	printf ( "\nNo Valid Error Records Found\n\n" );

}


/************************************************************************
 *									*
 * XZAPrintErrHdr() - 							*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrintErrHdr ( XZA_EB_HEADER* eb_ptr )
{
    printf ( "\n" );

	printf ( "\nErr Type = %x", eb_ptr -> err_type );
	printf ( "\nErr Sub Type = %x", eb_ptr -> sub_type );

    printf ( "\nUptime: %x", eb_ptr -> uptime_100ms );
    printf ( "\nXDEV: %x", eb_ptr -> xdev_reg );
    printf ( "\nXBE: %x", eb_ptr -> xbe_reg );
    printf ( "\nXFADR: %x", eb_ptr -> xfadr_reg );
    printf ( "\nXFAER: %x", eb_ptr -> xfaer_reg );
    printf ( "\nASR: %x", eb_ptr -> asr_reg );
    printf ( "\nError PC: %x", eb_ptr -> fw_pc );
    printf ( "\nGASCR: %x", eb_ptr -> gacsr_reg );
    printf ( "\nXZA Diag Reg: %x", eb_ptr -> diag_reg);

}


/************************************************************************
 *									*
 * XZAPrint_MC_NH() - 							*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrint_MC_NH ( XZA_MC_NH_BODY* eb_body_ptr )
{
    printf ( "\n\n" );
 
    printf ( "\nR0: %x", eb_body_ptr -> r0 );
    printf ( "\nR1: %x", eb_body_ptr -> r1 );
    printf ( "\nR2: %x", eb_body_ptr -> r2 );
    printf ( "\nR3: %x", eb_body_ptr -> r3 );
    printf ( "\nR4: %x", eb_body_ptr -> r4 );
    printf ( "\nR5: %x", eb_body_ptr -> r5 );
    printf ( "\nR6: %x", eb_body_ptr -> r6 );
    printf ( "\nR7: %x", eb_body_ptr -> r7 );
    printf ( "\nR8: %x", eb_body_ptr -> r8 );
    printf ( "\nR9: %x", eb_body_ptr -> r9 );
    printf ( "\nR10: %x", eb_body_ptr -> r10 );
    printf ( "\nR11: %x", eb_body_ptr -> r11 );
    printf ( "\nAP: %x", eb_body_ptr -> ap );
    printf ( "\nFP: %x", eb_body_ptr -> fp );
    printf ( "\nSP: %x", eb_body_ptr -> sp );
    printf ( "\nSP4: %x", eb_body_ptr -> sp4 );
    printf ( "\nSP8: %x", eb_body_ptr -> sp8 );
    printf ( "\nSP12: %x", eb_body_ptr -> sp12 );
    printf ( "\nSP16: %x", eb_body_ptr -> sp16 );
    printf ( "\nSP20: %x", eb_body_ptr -> sp20 );
    printf ( "\nSP24: %x", eb_body_ptr -> sp24 );
    printf ( "\nSP28: %x", eb_body_ptr -> sp28 );

}

/************************************************************************
 *									*
 * XZAPrint_Peek_Trans() -	 					*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrint_Peek_Trans ( XZA_PEEK_TRANS_BODY* eb_body_ptr )
{

	printf ( "\n\n" );

	printf ( "\nPKXMIL0 : %x", eb_body_ptr -> pkxmil0_reg );
	printf ( "\nPKXMIH0 : %x", eb_body_ptr -> pkxmih0_reg );
	printf ( "\nPKDATA0 : %x", eb_body_ptr -> pkdata0_reg );
	printf ( "\nPKDATB0 : %x", eb_body_ptr -> pkdatb0_reg );
	printf ( "\nPKXMIL1 : %x", eb_body_ptr -> pkxmil1_reg );
	printf ( "\nPKXMIH1 : %x", eb_body_ptr -> pkxmih1_reg );
	printf ( "\nPKDATA1 : %x", eb_body_ptr -> pkdata1_reg );
	printf ( "\nPKDATB1 : %x", eb_body_ptr -> pkdatb1_reg );

}

/************************************************************************
 *									*
 * XZAPrint_Datamove() - 						*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrint_Datamove ( XZA_DATAMOVE_BODY* eb_body_ptr )
{

	printf ( "\n\n" );

	printf ( "\nDMPOR0 : %x", eb_body_ptr -> dmpor0_reg );
	printf ( "\nDMCSR0 : %x", eb_body_ptr -> dmcsr0_reg );
	printf ( "\nDMXMI0 : %x", eb_body_ptr -> dmxmi0_reg );
	printf ( "\nDMNPA0 : %x", eb_body_ptr -> dmnpa0_reg );
	printf ( "\nDMPOR1 : %x", eb_body_ptr -> dmpor1_reg );
	printf ( "\nDMCSR1 : %x", eb_body_ptr -> dmcsr1_reg );
	printf ( "\nDMXMI1 : %x", eb_body_ptr -> dmxmi1_reg );
	printf ( "\nDMNPA1 : %x", eb_body_ptr -> dmnpa1_reg );
	printf ( "\nDMPOR2 : %x", eb_body_ptr -> dmpor2_reg );
	printf ( "\nDMCSR2 : %x", eb_body_ptr -> dmcsr2_reg );
	printf ( "\nDMXMI2 : %x", eb_body_ptr -> dmxmi2_reg );
	printf ( "\nDMNPA2 : %x", eb_body_ptr -> dmnpa2_reg );
	printf ( "\nDMPOR3 : %x", eb_body_ptr -> dmpor3_reg );
	printf ( "\nDMCSR3 : %x", eb_body_ptr -> dmcsr3_reg );
	printf ( "\nDMXMI3 : %x", eb_body_ptr -> dmxmi3_reg );
	printf ( "\nDMNPA3 : %x", eb_body_ptr -> dmnpa3_reg );

}

/************************************************************************
 *									*
 * XZAPrint_Host_Intr() - 						*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrint_Host_Intr ( XZA_HOST_INTR_BODY* eb_body_ptr )
{

	printf ( "\n\n" );

	printf ( "\nGAHIR : %x", eb_body_ptr -> gahir_reg );
	printf ( "\nGAIVR : %x", eb_body_ptr -> gaivr_reg );
	printf ( "\nGATMR : %x", eb_body_ptr -> gatmr_reg );

}

/************************************************************************
 *									*
 * XZAPrint_XBE_Rel() - 						*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrint_XBE_Rel ( XZA_XBE_RELATED_BODY* eb_body_ptr )
{
}

/************************************************************************
 *									*
 * XZAPrint_SIOP() - 							*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrint_SIOP ( XZA_SIOP_BODY* eb_body_ptr )
{

	printf ( "\n\n" );

	printf ( "\nChannel Mask : %x", eb_body_ptr -> channel_mask );
	printf ( "\nDSTAT : %x", eb_body_ptr -> dstat_reg );
	printf ( "\nISTAT : %x", eb_body_ptr -> istat_reg );

}

/************************************************************************
 *									*
 * XZAPrint_FW_Update() - 						*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
XZAPrint_FW_Update ( XZA_FW_UPDATE_BODY* eb_body_ptr )
{

	printf ( "\n\n" );

	printf ( "\nFW Image Rev : %4s", eb_body_ptr->ascii_fw_image_rev );
	printf ( "\nFW Image Rev Date : %12s", eb_body_ptr->ascii_fw_rev_date );

}


/************************************************************************
 *									*
 * XZAPrintParams() - 							*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
XZAPrintParams ( READPARAM_CMD* read_param_data_ptr )

{

    /*
    ** Print Parameters
    */
    printf ( "\n\nXZA Adapter Parameter Data" );
    printf ( "\n==========================\n" );

    printf ( "\nChan 0 ID: %d", read_param_data_ptr -> c0_id );
    printf ( "\nChan 1 ID: %d", read_param_data_ptr -> c1_id );

    printf ( "\n\nEEPROM Function bits:" );
    printf ( "\n=====================" );
    printf ( "\nLocal Console : %s", 
	read_param_data_ptr -> eeprm_fcn.lce ? "ENABLED" : "DISABLED" );
    printf ( "\nLog Self-Test Errors : %s", 
	read_param_data_ptr -> eeprm_fcn.lste ? "ENABLED" : "DISABLED" );
    printf ( "\nLog NCR 53C710 RBD Errors : %s", 
	read_param_data_ptr -> eeprm_fcn.lncre ? "ENABLED" : "DISABLED" );
    printf ( "\nLog XMI RBD Errors : %s", 
	read_param_data_ptr -> eeprm_fcn.lxmie ? "ENABLED" : "DISABLED" );
    printf ( "\nGlobal RBD Error Logging : %s", 
	read_param_data_ptr -> eeprm_fcn.lxzae ? "DISABLED" : "ENABLED" );
    printf ( "\nHard Error Frame Overflow : %s", 
	read_param_data_ptr -> eeprm_fcn.hefo ? "YES" : "NO" );
    printf ( "\nSoft Error Frame Overflow : %s", 
	read_param_data_ptr -> eeprm_fcn.sefo ? "YES" : "NO" );
    printf ( "\nRBD Error Frame Overflow : %s", 
	read_param_data_ptr -> eeprm_fcn.refo ? "YES" : "NO" );
    printf ( "\nFW Update Frame Overflow : %s", 
	read_param_data_ptr -> eeprm_fcn.lce ? "YES" : "NO" );

    printf ( "\n" );

    printf ( "\nSerial Number: %12s", read_param_data_ptr -> mod_ser_no );
    printf ( "\nFirmware Revision: %4s", read_param_data_ptr -> fw_image_rev );
    printf ( "\nFirmware Rev Date: %12s", read_param_data_ptr -> fw_rev_date );

    printf ( "\n\n" );

}


/************************************************************************
 *									*
 * XZAPrintNCRRegs() - 							*
 *									*
 * Inputs:								*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
XZAPrintNCRRegs ( READREG_CMD* reg_data_ptr )

{

    /*
    ** Print Register Values
    */

    printf ( "\n\nNCR SIOP Register Values:" );
    printf ( "\n=========================" );
    printf ( "\n" );

    printf ( "\nSCNTL0 : %x\tSFBR : %x", 
		reg_data_ptr -> scntl0, reg_data_ptr -> sfbr );
    printf ( "\nSCNTL1 : %x\tSIDL : %x", 
		reg_data_ptr -> scntl1, reg_data_ptr -> sidl );
    printf ( "\nSDID : %x\tSBDL : %x", 
		reg_data_ptr -> sdid, reg_data_ptr -> sbdl );
    printf ( "\nSIEN : %x\tSBCL : %x", 
		reg_data_ptr -> sien, reg_data_ptr -> sbcl );
    printf ( "\nSCID : %x\tDSTAT : %x", 
		reg_data_ptr -> scid, reg_data_ptr -> dstat );
    printf ( "\nSXFER : %x\tSSTAT0 : %x", 
		reg_data_ptr -> sxfer, reg_data_ptr -> sstat0 );
    printf ( "\nSODL : %x\tSSTAT1 : %x", 
		reg_data_ptr -> sodl, reg_data_ptr -> sstat1 );
    printf ( "\nSOCL : %x\tSSTAT2 : %x", 
		reg_data_ptr -> socl, reg_data_ptr -> sstat2 );
    printf ( "\n" );

    printf ( "\nDSA : %x", reg_data_ptr -> dsa );
    printf ( "\nCTEST0 : %x", reg_data_ptr -> ctest0 );
    printf ( "\nCTEST1 : %x", reg_data_ptr -> ctest1 );
    printf ( "\nCTEST2 : %x", reg_data_ptr -> ctest2 );
    printf ( "\nCTEST3 : %x", reg_data_ptr -> ctest3 );
    printf ( "\n" );

    printf ( "\nCTEST4 : %x\tDFIFO : %x", 
		reg_data_ptr -> ctest4, reg_data_ptr -> dfifo );
    printf ( "\nCTEST5 : %x\tISTAT : %x", 
		reg_data_ptr -> ctest5, reg_data_ptr -> istat );
    printf ( "\nCTEST6 : %x\tCTEST8 : %x", 
		reg_data_ptr -> ctest6, reg_data_ptr -> ctest8 );
    printf ( "\nCTEST7 : %x\tLCRC : %x", 
		reg_data_ptr -> ctest7, reg_data_ptr -> lcrc );
    printf ( "\nTEMP : %x\tDBC/DCMD : %x", 
		reg_data_ptr -> temp, reg_data_ptr -> dbc_dcmd );
    printf ( "\n" );


    printf ( "\nDNAD : %x\tDSPS : %x", 
		reg_data_ptr -> dnad, reg_data_ptr -> dsps );
    printf ( "\nDSP : %x\t\tSCRATCH : %x", 
		reg_data_ptr -> dsp, reg_data_ptr -> scratch );
    printf ( "\n" );

    printf ( "\nDMODE : %x", reg_data_ptr -> dmode );
    printf ( "\nDIEN : %x", reg_data_ptr -> dien );
    printf ( "\nDWT : %x", reg_data_ptr -> dwt );
    printf ( "\nDCNTL : %x", reg_data_ptr -> dcntl );
    printf ( "\nADDER : %x", reg_data_ptr -> adder );
    printf ( "\n\n" );

}

