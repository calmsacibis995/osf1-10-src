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
static char *rcsid = "@(#)$RCSfile: special_direct.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/11/23 23:07:40 $";
#endif

/************************************************************************
 *									*
 * File:	special_direct.c					*
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
 * January 17, 1992 by Robin Miller.					*
 *	Modify the read defect data function to use the gdl & mdl bits	*
 *	(Grown/Manufactures Defect lists) passed in parameter list.	*
 *									*
 * August 24, 1991 by Robin Miller.					*
 *	Added functions for making read & write CDB's (6 & 10 byte).	*
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
#include <io/cam/rzdisk.h>

#include <io/cam/dec_cam.h>
#ifdef _KERNEL
#include <mach/vm_param.h>
#endif /* _KERNEL */
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#include <io/cam/scsi_special.h>
#include <io/cam/cam_debug.h>


/************************************************************************
 *									*
 * scmn_MakeSeekPosition() - Make Seek Position (Direct) SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeSeekPosition (sap, cdbp)
register struct special_args *sap;
register struct dir_seek_cdb10 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register U32 seek_address;

	seek_address = sap->sa_cmd_parameter;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->lbn3 = LTOB(seek_address,3);
	cdbp->lbn2 = LTOB(seek_address,2);
	cdbp->lbn1 = LTOB(seek_address,1);
	cdbp->lbn0 = LTOB(seek_address,0);
	return (SUCCESS);
}


/************************************************************************
 *									*
 * scmn_SetupSeekPosition() - Setup Seek Position Parameters.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupSeekPosition (sap, data)
register struct special_args *sap;
register caddr_t data;
{
	register U32 seek_address;

	seek_address = *(U32 *) data;
	sap->sa_cmd_parameter = seek_address;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeFormatUnit() - Make Format Unit Command Descriptor Block.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeFormatUnit (sap, cdbp)
register struct special_args *sap;
register struct dir_format_cdb6 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct format_params *fp;

	fp = (struct format_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	if (fp->fp_defects == VENDOR_DEFECTS) {
	    cdbp->fmt_data = 1;
	    cdbp->cmp_list = 1;
	} else if (fp->fp_defects == KNOWN_DEFECTS) {
	    cdbp->fmt_data = 1;
	    cdbp->cmp_list = 0;
	} else if (fp->fp_defects == NO_DEFECTS) {
	    cdbp->fmt_data = 0;
	    cdbp->cmp_list = 0;
	}
	cdbp->defect_list_fmt = fp->fp_format;
	cdbp->vendor_specific = fp->fp_pattern;
	cdbp->interleave1 = 0;
	cdbp->interleave0 = fp->fp_interleave;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupFormatUnit() - Setup Format Unit Parameters.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupFormatUnit (sap, data)
register struct special_args *sap;
caddr_t data;
{
	struct form2_defect_list_header defect_header;
	register struct form2_defect_list_header *ddh = &defect_header;
	register struct format_params *fp; 

	fp = (struct format_params *) data;
	sap->sa_user_buffer = (caddr_t) fp->fp_addr;

	/*
	 * For diskettes, there are no defect lists.
	 */
	if ( ((sap->sa_user_length = fp->fp_length) == 0) &&
	     (fp->fp_defects == NO_DEFECTS) ) {
	    sap->sa_cmd_flags &= ~(SPC_INOUT | SPC_DATA_INOUT);
	    return (SUCCESS);
	}

#ifdef _KERNEL
	/*
	 * Ensure the defect list address is valid (user address).
	 */
	if ( ((sap->sa_flags & SA_SYSTEM_REQUEST) == 0) &&
		!CAM_IS_KUSEG(fp->fp_addr) ) {
	    return (EINVAL);
	}

	/*
	 * The format parameters structure isn't set up with the length
	 * of the defect lists like it should.  Therefore, we must copyin
	 * the defect list header then calculate the defect list length.
	 */
	if (copyin ((caddr_t)fp->fp_addr, (caddr_t)ddh, sizeof(*ddh)) != 0) {
	    return (EFAULT);
	}
#else /* _KERNEL */
	(void) bcopy ((caddr_t)fp->fp_addr, (caddr_t)ddh, sizeof(*ddh));
#endif /* _KERNEL */

	sap->sa_user_length = (int) (  (ddh->defect_len1 << 8) +
					ddh->defect_len0 + sizeof(*ddh) );

	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeReadDefectData() - Make Read Defect Data SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeReadDefectData (sap, cdbp)
register struct special_args *sap;
register struct dir_read_defect_cdb10 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	struct read_defect_params *rdp;

	rdp = (struct read_defect_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	/*
	 * If neither defect list is specified, then pass back both
	 * defect lists so we don't break 'rzdisk' functionality.
	 */
	if (rdp->rdp_mdl || rdp->rdp_gdl) {
	    cdbp->plist = rdp->rdp_mdl;
	    cdbp->glist = rdp->rdp_gdl;
        } else {
	    cdbp->plist = 1;
	    cdbp->glist = 1;
        }
	cdbp->defect_list_fmt = rdp->rdp_format;
	cdbp->alloc_len1 = (u_char) (rdp->rdp_alclen >> 8);
	cdbp->alloc_len0 = (u_char) rdp->rdp_alclen;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupReadDefectData() - Setup Read Defect Data Parameters.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupReadDefectData (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct read_defect_params *rdp; 

	rdp = (struct read_defect_params *) data;
	sap->sa_user_buffer = (caddr_t) rdp->rdp_addr;
	sap->sa_user_length = (int) rdp->rdp_alclen;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupReassignBlocks() - Setup Reassign Blocks Parameters.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupReassignBlocks (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct reassign_params *rp; 

	rp = (struct reassign_params *) data;
	sap->sa_user_buffer = (caddr_t) sap->sa_iop_buffer;
	sap->sa_user_length = (int) (  (rp->rp_header.defect_len1 << 8) +
					rp->rp_header.defect_len0 +
					sizeof(rp->rp_header) );
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupReadLong() - Setup Read Long Parameters.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupReadLong (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct io_uxfer *iox;

	iox = (struct io_uxfer *) data;
	sap->sa_user_buffer = (caddr_t) iox->io_addr;
	sap->sa_user_length = iox->io_cnt;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupWriteLong() - Setup Write Long Parameters.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupWriteLong (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct io_uxfer *iox;

	iox = (struct io_uxfer *) data;
	sap->sa_user_buffer = (caddr_t) iox->io_addr;
	sap->sa_user_length = iox->io_cnt;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeVerifyDirect() - Make Verify Data (Direct) SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeVerifyDirect (sap, cdbp)
register struct special_args *sap;
register struct dir_verify_cdb10 *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct verify_params *vp;

	vp = (struct verify_params *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->reladr = 0;
	cdbp->bytchk = 0;
	cdbp->lbaddr3 = LTOB(vp->vp_lbn,3);
	cdbp->lbaddr2 = LTOB(vp->vp_lbn,2);
	cdbp->lbaddr1 = LTOB(vp->vp_lbn,1);
	cdbp->lbaddr0 = LTOB(vp->vp_lbn,0);
	cdbp->verflen1 = LTOB(vp->vp_length,1);
	cdbp->verflen0 = LTOB(vp->vp_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeReadDirect() - Make Read (Direct) SCSI CDB.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeReadDirect (sap, cdbp)
struct special_args *sap;
caddr_t *cdbp;
{
	struct data_transfer_params *dt;
	U32 blocks;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	blocks = howmany (sap->sa_user_length, dt->dt_block_size);
	/*
	 * Dispatch to 6 or 10 byte CDB depending on LBA.
	 */
	if ( (dt->dt_lba > DIR_MAX_LBA_CDB6) || (blocks > DIR_MAX_LEN_CDB6)
	     || (dt->dt_inq_dtype == ALL_DTYPE_OPTICAL) ) {
	    return (scmn_MakeReadDirect10 (sap, cdbp));
	} else {
	    return (scmn_MakeReadDirect6 (sap, cdbp));
	}
}

int
scmn_MakeReadDirect6 (sap, cdbp)
register struct special_args *sap;
register struct dir_read_cdb6 *cdbp;
{
	register struct data_transfer_params *dt;
	register U32 blocks;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	blocks = howmany (sap->sa_user_length, dt->dt_block_size);
	cdbp->opcode = DIR_READ6_OP;
	DIRLBN_TO_READ6 (dt->dt_lba, cdbp);
	DIRTRANS_TO_READ6 (blocks, cdbp);
	return (SUCCESS);
}

int
scmn_MakeReadDirect10 (sap, cdbp)
register struct special_args *sap;
register struct dir_read_cdb10 *cdbp;
{
	register struct data_transfer_params *dt;
	register U32 blocks;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	blocks = howmany (sap->sa_user_length, dt->dt_block_size);
	cdbp->opcode = DIR_READ10_OP;
	DIRLBN_TO_READ10 (dt->dt_lba, cdbp);
	DIRTRANS_TO_READ10 (blocks, cdbp);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeWriteDirect() - Make Write (Direct) SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeWriteDirect (sap, cdbp)
struct special_args *sap;
caddr_t *cdbp;
{
	struct data_transfer_params *dt;
	U32 blocks;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	blocks = howmany (sap->sa_user_length, dt->dt_block_size);
	/*
	 * Dispatch for 6 or 10 byte CDB depending on LBA.
	 */
	if ( (dt->dt_lba > DIR_MAX_LBA_CDB6) || (blocks > DIR_MAX_LEN_CDB6)
	     || (dt->dt_inq_dtype == ALL_DTYPE_OPTICAL) ) {
	    return (scmn_MakeWriteDirect10 (sap, cdbp));
	} else {
	    return (scmn_MakeWriteDirect6 (sap, cdbp));
	}
}

int
scmn_MakeWriteDirect6 (sap, cdbp)
register struct special_args *sap;
register struct dir_write_cdb6 *cdbp;
{
	register struct data_transfer_params *dt;
	register U32 blocks;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	blocks = howmany (sap->sa_user_length, dt->dt_block_size);
	cdbp->opcode = DIR_WRITE6_OP;
	DIRLBN_TO_WRITE6 (dt->dt_lba, cdbp);
	DIRTRANS_TO_WRITE6 (blocks, cdbp);
	return (SUCCESS);
}

int
scmn_MakeWriteDirect10 (sap, cdbp)
register struct special_args *sap;
register struct dir_write_cdb10 *cdbp;
{
	register struct data_transfer_params *dt;
	register U32 blocks;

	dt = (struct data_transfer_params *) sap->sa_iop_buffer;
	blocks = howmany (sap->sa_user_length, dt->dt_block_size);
	cdbp->opcode = DIR_WRITE10_OP;
	DIRLBN_TO_WRITE10 (dt->dt_lba, cdbp);
	DIRTRANS_TO_WRITE10 (blocks, cdbp);
	return (SUCCESS);
}

