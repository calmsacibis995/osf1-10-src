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
static char *rcsid = "@(#)$RCSfile: special_audio.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 17:53:14 $";

/************************************************************************
 *									*
 * File:	special_audio.c						*
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
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_rodirect.h>
#include <io/cam/cam_debug.h>

/*
 * External Declarations:
 */
extern int scmn_DoCmd();

/************************************************************************
 *									*
 * scmn_DoCdTocHeader() - Process CD-ROM Read TOC Header Command.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_DoCdTocHeader (sap, data)
register struct special_args *sap;
register caddr_t data;
{
	register struct cd_toc *toc;
	register struct cd_toc_header *th;
	register int status = SUCCESS;

	toc = (struct cd_toc *) sap->sa_iop_buffer;
	th = (struct cd_toc_header *) sap->sa_data_buffer;
	toc->toc_starting_track = 0;
	toc->toc_alloc_length = sizeof(*th);
	if ( (status = scmn_DoCmd (sap)) == SUCCESS) {
	    (void) bcopy ((char *)th, data, sizeof(*th));
	}
	return (status);
}

/************************************************************************
 *									*
 * scmn_MakeCdReadTOC() - Make Read Table of Contents SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdReadTOC (sap, cdbp)
register struct special_args *sap;
register struct CdReadTOC_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_toc *toc;

	toc = (struct cd_toc *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	if (toc->toc_address_format == CDROM_MSF_FORMAT) {
	    cdbp->msf = 1;
	}
	cdbp->starting_track = toc->toc_starting_track;
	cdbp->alloc_len1 = LTOB(toc->toc_alloc_length,1);
	cdbp->alloc_len0 = LTOB(toc->toc_alloc_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupCdTocEntrys() - Setup CD-ROM Read TOC Entrys Parameters.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupCdTocEntrys (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct cd_toc *toc;

	toc = (struct cd_toc *) data;
	sap->sa_user_buffer = toc->toc_buffer;
	sap->sa_user_length = toc->toc_alloc_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeCdReadSubChannel() - Make Read SubChannel SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdReadSubChannel (sap, cdbp)
register struct special_args *sap;
register struct CdReadSubChannel_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_sub_channel *sch;

	sch = (struct cd_sub_channel *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	if (sch->sch_address_format == CDROM_MSF_FORMAT) {
	    cdbp->msf = 1;
	}
	cdbp->subQ = 1;
	cdbp->data_format = sch->sch_data_format;
	cdbp->track_number = sch->sch_track_number;
	cdbp->alloc_len1 = LTOB(sch->sch_alloc_length,1);
	cdbp->alloc_len0 = LTOB(sch->sch_alloc_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupCdReadSubChannel() - Setup CD-ROM Read SubChannel Params.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupCdReadSubChannel (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct cd_sub_channel *sch;

	sch = (struct cd_sub_channel *) data;
	sap->sa_user_buffer = sch->sch_buffer;
	sap->sa_user_length = sch->sch_alloc_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_DoCdSetAddressFormat() - Process CD-ROM Set Address Format Cmd.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_DoCdSetAddressFormat (sap, data)
register struct special_args *sap;
register caddr_t data;
{
	register int address_format;

	address_format = *(int *) data;
	sap->sa_cmd_parameter = address_format;
	return (scmn_DoCmd (sap));
}

/************************************************************************
 *									*
 * scmn_MakeCdSetAddressFormat() - Make Set Address Format SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdSetAddressFormat (sap, cdbp)
register struct special_args *sap;
register struct CdSetAddressFormat_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;

	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->lbamsf = sap->sa_cmd_parameter;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeCdReadHeader() - Make Read Header SCSI CDB.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdReadHeader (sap, cdbp)
register struct special_args *sap;
register struct CdReadHeader_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_read_header *rh;

	rh = (struct cd_read_header *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	if (rh->rh_address_format == CDROM_MSF_FORMAT) {
	    cdbp->msf = 1;
	}
	cdbp->lbaddr3 = LTOB(rh->rh_lba,3);
	cdbp->lbaddr2 = LTOB(rh->rh_lba,2);
	cdbp->lbaddr1 = LTOB(rh->rh_lba,1);
	cdbp->lbaddr0 = LTOB(rh->rh_lba,0);
 	cdbp->alloc_len1 = LTOB(rh->rh_alloc_length,1);
	cdbp->alloc_len0 = LTOB(rh->rh_alloc_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupCdReadHeader() - Setup CD-ROM Read Header Parameters.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupCdReadHeader (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct cd_read_header *rh;

	rh = (struct cd_read_header *) data;
	sap->sa_user_buffer = rh->rh_buffer;
	sap->sa_user_length = rh->rh_alloc_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeCdPlayback() - Make Playback Control/Status SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdPlayback (sap, cdbp)
register struct special_args *sap;
register struct CdPlayback_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_playback *pb;

	pb = (struct cd_playback *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->alloc_len1 = LTOB(pb->pb_alloc_length,1);
	cdbp->alloc_len0 = LTOB(pb->pb_alloc_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_SetupCdPlayback() - Setup CD Playback Control/Status Parameters	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		data = The address of input/output arguments.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_SetupCdPlayback (sap, data)
register struct special_args *sap;
caddr_t data;
{
	register struct cd_playback *pb;

	pb = (struct cd_playback *) data;
	sap->sa_user_buffer = pb->pb_buffer;
	sap->sa_user_length = pb->pb_alloc_length;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeCdPlayAudioLBA() - Make Play Audio LBA SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdPlayAudioLBA (sap, cdbp)
register struct special_args *sap;
register struct CdPlayAudioLBA_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_play_audio *pa;

	pa = (struct cd_play_audio *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->lbaddr3 = LTOB(pa->pa_lba,3);
	cdbp->lbaddr2 = LTOB(pa->pa_lba,2);
	cdbp->lbaddr1 = LTOB(pa->pa_lba,1);
	cdbp->lbaddr0 = LTOB(pa->pa_lba,0);
	cdbp->xferlen1 = LTOB(pa->pa_length,1);
	cdbp->xferlen0 = LTOB(pa->pa_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeCdPlayAudioMSF() - Make Play Audio MSF SCSI CDB.		*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdPlayAudioMSF (sap, cdbp)
register struct special_args *sap;
register struct CdPlayAudioMSF_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_play_audio_msf *msf;

	msf = (struct cd_play_audio_msf *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->starting_M_unit = msf->msf_starting_M_unit;
	cdbp->starting_S_unit = msf->msf_starting_S_unit;
	cdbp->starting_F_unit = msf->msf_starting_F_unit;
	cdbp->ending_M_unit = msf->msf_ending_M_unit;
	cdbp->ending_S_unit = msf->msf_ending_S_unit;
	cdbp->ending_F_unit = msf->msf_ending_F_unit;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeCdPlayAudioTI() - Make Play Audio Track/Index SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdPlayAudioTI (sap, cdbp)
register struct special_args *sap;
register struct CdPlayAudioTI_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_play_audio_ti *ti;

	ti = (struct cd_play_audio_ti *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->starting_track = ti->ti_starting_track;
	cdbp->starting_index = ti->ti_starting_index;
	cdbp->ending_track = ti->ti_ending_track;
	cdbp->ending_index = ti->ti_ending_index;
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeCdPlayAudioTR() - Make Play Audio Track Relative SCSI CDB.	*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdPlayAudioTR (sap, cdbp)
register struct special_args *sap;
register struct CdPlayAudioTR_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_play_audio_tr *tr;

	tr = (struct cd_play_audio_tr *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->lbaddr3 = LTOB(tr->tr_lba,3);
	cdbp->lbaddr2 = LTOB(tr->tr_lba,2);
	cdbp->lbaddr1 = LTOB(tr->tr_lba,1);
	cdbp->lbaddr0 = LTOB(tr->tr_lba,0);
	cdbp->starting_track = tr->tr_starting_track;
 	cdbp->xfer_len1 = LTOB(tr->tr_xfer_length,1);
	cdbp->xfer_len0 = LTOB(tr->tr_xfer_length,0);
	return (SUCCESS);
}

/************************************************************************
 *									*
 * scmn_MakeCdPlayTrack() - Make Play Track SCSI CDB.			*
 *									*
 * Inputs:	sap = Special command argument block pointer.		*
 *		cdbp = Pointer to command descriptor block.		*
 *									*
 * Return Value:							*
 *		Returns 0 for SUCCESS, or error code on failures.	*
 *									*
 ************************************************************************/
int
scmn_MakeCdPlayTrack (sap, cdbp)
register struct special_args *sap;
register struct CdPlayTrack_CDB *cdbp;
{
	register struct special_cmd *spc = sap->sa_spc;
	register struct cd_play_track *pt;

	pt = (struct cd_play_track *) sap->sa_iop_buffer;
	cdbp->opcode = (u_char) spc->spc_cmd_code;
	cdbp->starting_track = pt->pt_starting_track;
	cdbp->starting_index = pt->pt_starting_index;
	cdbp->number_indexes = pt->pt_number_indexes;
	return (SUCCESS);
}
