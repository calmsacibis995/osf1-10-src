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
static char *rcsid = "@(#)$RCSfile: special_generic.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/11/23 23:07:52 $";
#endif

/************************************************************************
 *									*
 * File:	special_generic.c					*
 * Date:	April 5, 1991						*
 * Author:	Robin Miller						*
 *									*
 * Description:								*
 *	Functions to process a special command.  Since these commands	*
 * require either special setup before or after the command is issued,	*
 * there must be functions to handle these commands.			*
 *									*
 * Modification History:						*
 *									*
 * August 13, 1991 by Robin Miller.					*
 *	Modify #include directives to permit user applications to use	*
 *	this data file (changed "../h/..." format to <sys/...>).	*
 *									*
 * July 30, 1991 by Robin Miller.					*
 *	Added functions to support read & write buffer commands.	*
 *									*
 * June 27, 1991 by Robin Miller.					*
 *	Change SCSI structures & operation codes to new definitions.	*
 *									*
 ************************************************************************/

#include <io/common/iotypes.h>
#include <sys/param.h>
#include <io/common/devio.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <io/cam/cdrom.h>
#include <io/cam/rzdisk.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam_debug.h>

/************************************************************************
 *									*
 * scmn_DoGetSense() - Process Get Sense Command.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_DoGetSense (sap, data)
register struct special_args *sap;
register caddr_t data;
{
	register struct extended_sense *es;
	register int target = sap->sa_target;

#ifdef notdef
	(void) bcopy (sap->sa_sense_buffer, data, sap->sa_sense_length);
	(void) bzero (sap->sa_sense_buffer, sap->sa_sense_length);
	return (SUCCESS);
#endif notdef
	es = (struct extended_sense *) data;
	(void) bcopy (sap->sa_sense_buffer, es, sizeof(*es));
	(void) bzero (sap->sa_sense_buffer, sap->sa_sense_length);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeModeSense() - Make Mode Sense Command Descriptor Block.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeModeSense (sap, cdbp)
register struct special_args *sap;
register struct all_mode_sense_cdb6 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct mode_sel_sns_params *msp;

	msp = (struct mode_sel_sns_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->page_code = msp->msp_pgcode;
	cdbp->pc = msp->msp_pgctrl;
	cdbp->alloc_len = msp->msp_length;
	return (SUCCESS);
}


/************************************************************************
 *									*
 * scmn_SetupModeSense() - Setup Mode Sense Parameters.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupModeSense (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct mode_sel_sns_params *msp; 

	msp = (struct mode_sel_sns_params *) data;
	sap->sa_user_buffer = msp->msp_addr;
	sap->sa_user_length = msp->msp_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeModeSelect() - Make Mode Select Command Descriptor Block.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeModeSelect (sap, cdbp)
register struct special_args *sap;
register struct all_mode_sel_cdb6 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct mode_sel_sns_params *msp;

	msp = (struct mode_sel_sns_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->param_len = msp->msp_length;
	cdbp->sp = msp->msp_setps;
	cdbp->pf = 1;			/* Comes from SCSI device table. */
/*									*/
/* Current driver sets this bit later on based on SCSI_MODSEL_PF bit.	*/
/* This bit is normally set for SCSI-2 Compliant controllers.		*/
/*									*/
	return (SUCCESS);
}


/************************************************************************
 *									*
 * scmn_SetupModeSelect() - Setup Mode Select Parameters.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupModeSelect (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct mode_sel_sns_params *msp; 

	msp = (struct mode_sel_sns_params *) data;
	sap->sa_user_buffer = msp->msp_addr;
	sap->sa_user_length = msp->msp_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeInquiry() - Make Inquiry Command Descriptor Block (CDB).	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeInquiry (sap, cdbp)
register struct special_args *sap;
register struct all_inq_cdb *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;

	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->alloc_len = (u_char) sap->sa_user_length;
	return (SUCCESS);
}


/************************************************************************
 *									*
 * scmn_SetupInquiry() - Setup Inquiry Parameters.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupInquiry (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct mode_sel_sns_params *msp; 

	msp = (struct mode_sel_sns_params *) data;
	sap->sa_user_buffer = msp->msp_addr;
	sap->sa_user_length = msp->msp_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeSendDiagnostic() - Make Send Diagnostic SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeSendDiagnostic (sap, cdbp)
register struct special_args *sap;
register struct all_send_diagnostic_cdb *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct diagnostic_params *dp;

	dp = (struct diagnostic_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->control = dp->dp_control;
	cdbp->param_len1 = LTOB(dp->dp_length,1);
	cdbp->param_len0 = LTOB(dp->dp_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupSendDiagnostic() - Setup Send Diagnostic Parameters.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupSendDiagnostic (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct diagnostic_params *dp;

	dp = (struct diagnostic_params *) data;
	sap->sa_user_buffer = dp->dp_buffer;
	sap->sa_user_length = dp->dp_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeReceiveDiagnostic() - Make Receive Diagnostic SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeReceiveDiagnostic (sap, cdbp)
register struct special_args *sap;
register struct all_receive_diagnostic_cdb *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct diagnostic_params *dp;

	dp = (struct diagnostic_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->alloc_len1 = LTOB(dp->dp_length,1);
	cdbp->alloc_len0 = LTOB(dp->dp_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupReceiveDiagnostic() - Setup Receive Diagnostic Parameters.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupReceiveDiagnostic (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct diagnostic_params *dp;

	dp = (struct diagnostic_params *) data;
	sap->sa_user_buffer = dp->dp_buffer;
	sap->sa_user_length = dp->dp_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeReadBuffer() - Make Read Buffer Diagnostic SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeReadBuffer (sap, cdbp)
register struct special_args *sap;
register struct all_read_buffer_cdb *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct diagnostic_params *dp;

	dp = (struct diagnostic_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->mode = dp->dp_mode;
	cdbp->id = dp->dp_id;
	cdbp->offset2 = LTOB(dp->dp_offset,2);
	cdbp->offset1 = LTOB(dp->dp_offset,1);
	cdbp->offset0 = LTOB(dp->dp_offset,0);
	cdbp->alloc_len2 = LTOB(dp->dp_length,2);
	cdbp->alloc_len1 = LTOB(dp->dp_length,1);
	cdbp->alloc_len0 = LTOB(dp->dp_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeWriteBuffer() - Make Write Buffer Diagnostic SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeWriteBuffer (sap, cdbp)
register struct special_args *sap;
register struct all_write_buffer_cdb *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct diagnostic_params *dp;

	dp = (struct diagnostic_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->mode = dp->dp_mode;
	cdbp->id = dp->dp_id;
	cdbp->offset2 = LTOB(dp->dp_offset,2);
	cdbp->offset1 = LTOB(dp->dp_offset,1);
	cdbp->offset0 = LTOB(dp->dp_offset,0);
	cdbp->param_len2 = LTOB(dp->dp_length,2);
	cdbp->param_len1 = LTOB(dp->dp_length,1);
	cdbp->param_len0 = LTOB(dp->dp_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupWriteBuffer() - Setup Write Buffer Parameters.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupWriteBuffer (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct diagnostic_params *dp;

	dp = (struct diagnostic_params *) data;
	/*
	 * If there's no data being transferred, clear the data out flag.
	 */
	if (dp->dp_length == 0) {
	    sap->sa_cmd_flags &= ~(SPC_DATA_OUT);
	}
	return (SUCCESS);
}
