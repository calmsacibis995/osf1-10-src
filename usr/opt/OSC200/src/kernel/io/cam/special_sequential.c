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
static char *rcsid = "@(#)$RCSfile: special_sequential.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 17:53:42 $";
#endif

/************************************************************************
 *									*
 * File:	special_sequential.c					*
 * Date:	May 5, 1991						*
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
#include <sys/mtio.h>

#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_special.h>
#include <io/cam/scsi_sequential.h>
#include <io/cam/cam_debug.h>

/************************************************************************
 *									*
 * scmn_MakeMtSpace() - Make 'mt' Space Files/Records SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeMtSpace (sap, cdbp)
register struct special_args *sap;
register struct seq_space_cdb6 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct mtop *mtop;
	register daddr_t count;
	register u_char code;

	mtop = (struct mtop *) sap->sa_ioctl_data;
	count = mtop->mt_count;
	switch (mtop->mt_op) {

	    case MTBSF:			/* Backward Space Files. */
		code = SEQ_SPACE_FILEMARKS;
		count = -count;
		break;

	    case MTBSR:			/* Backward Space Records. */
		code = SEQ_SPACE_BLOCKS;
		count = -count;
		break;

	    case MTFSF:			/* Forward Space Files. */
		code = SEQ_SPACE_FILEMARKS;
		break;

	    case MTFSR:			/* Forward Space Records. */
		code = SEQ_SPACE_BLOCKS;
		break;
	}
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->code = code;
	cdbp->count2 = LTOB(count,2);
	cdbp->count1 = LTOB(count,1);
	cdbp->count0 = LTOB(count,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeMtWriteFileMark() - Make 'mt' Write File Mark SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeMtWriteFileMark (sap, cdbp)
register struct special_args *sap;
register struct seq_writemarks_cdb6 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct mtop *mtop;
	register daddr_t count;

	mtop = (struct mtop *) sap->sa_ioctl_data;
	count = mtop->mt_count;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->trans_len2 = LTOB(count,2);
	cdbp->trans_len1 = LTOB(count,1);
	cdbp->trans_len0 = LTOB(count,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeVerifySequential() - Make Verify Tape SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeVerifySequential (sap, cdbp)
register struct special_args *sap;
register struct seq_verify_cdb6 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct data_transfer_params *dt;
	register u_long length = sap->sa_cmd_parameter;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->fixed = (dt->dt_block_size) ? 1 : 0;
	cdbp->bytcmp = 0;
	cdbp->immed = 0;
	if (dt->dt_block_size) {
	    u_long blocks = howmany (length, dt->dt_block_size);
	    cdbp->verflen2 = LTOB(blocks,2);
	    cdbp->verflen1 = LTOB(blocks,1);
	    cdbp->verflen0 = LTOB(blocks,0);
	} else {
	    cdbp->verflen2 = LTOB(length,2);
	    cdbp->verflen1 = LTOB(length,1);
	    cdbp->verflen0 = LTOB(length,0);
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeReadSequential() - Make Read (Sequential) SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeReadSequential (sap, cdbp)
register struct special_args *sap;
register struct seq_read_cdb6 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct data_transfer_params *dt;
	register u_long length = sap->sa_user_length;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->fixed = (dt->dt_block_size) ? 1 : 0;
	if (dt->dt_block_size) {
	    u_long blocks = howmany (length, dt->dt_block_size);
	    SEQTRANS_TO_READ6 (blocks, cdbp);
	} else {
	    SEQTRANS_TO_READ6 (length, cdbp);
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeWriteSequential() - Make Write (Sequential) SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeWriteSequential (sap, cdbp)
struct special_args *sap;
register struct seq_write_cdb6 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct data_transfer_params *dt;
	register u_long length = sap->sa_user_length;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->fixed = (dt->dt_block_size) ? 1 : 0;
	if (dt->dt_block_size) {
	    u_long blocks = howmany (length, dt->dt_block_size);
	    SEQTRANS_TO_WRITE6 (blocks, cdbp);
	} else {
	    SEQTRANS_TO_WRITE6 (length, cdbp);
	}
	return (SUCCESS);
}
