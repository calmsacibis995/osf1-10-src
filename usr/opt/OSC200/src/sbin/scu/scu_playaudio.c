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
static char *rcsid = "@(#)$RCSfile: scu_playaudio.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/05/07 20:40:56 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu_playaudio.c - 'scu' Play Audio Functions.
 * Author:	Robin T. Miller
 * Date:	August 8, 1991
 *
 * Description:
 *	This file contains functions for playing audio.
 */

#include "scu.h"
#include <io/cam/cdrom.h>

/*
 * External References:
 */
extern u_long CvtMSFtoLBA (int minute_units,
			   int second_units, int frame_units);
extern struct cd_toc_entry *FindTOCEntry (int track_number);
extern void Printf();

/************************************************************************
 *									*
 * PlayAudio()  Play Audio LBA Format.					*
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
PlayAudio (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct cd_play_audio play_audio;
	register struct cd_play_audio *pa = &play_audio;
	u_long lba = *((u_long *) ke->k_argp);
	int status;

	/*
	 * Seek to the specified LBA first, and check for an audio track,
	 * since the Play Audio LBA command fails on data tracks.
	 */
	if ( (status = SeekPosition (lba)) != SUCCESS) {
		return (status);
	}
	if ( (status = CheckAudioTrack (TRUE, ke->k_msgp)) != TRUE) {
		return ( (status == FALSE) ? WARNING : status);
	}
	bzero ((char *) pa, sizeof(*pa));
	pa->pa_lba = lba;
	pa->pa_length = TransferLength;
	status = DoIoctl (ke->k_cmd, (caddr_t)pa, ke->k_msgp);
	return (status);
}

/************************************************************************
 *									*
 * PlayAudioMSF()  Play Audio Minute/Second/Frame Format.		*
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
PlayAudioMSF (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct cd_play_audio_msf play_audio_msf;
	register struct cd_play_audio_msf *msf = &play_audio_msf;
	u_long lba;
	int status;

	lba = CvtMSFtoLBA (Starting_M_Unit, Starting_S_Unit, Starting_F_Unit);
	/*
	 * Seek to the starting track LBA, and check for an audio track,
	 * since the Play Audio Track command fails on data tracks.
	 */
	if ( (status = SeekPosition (lba)) != SUCCESS) {
		return (status);
	}
	if ( (status = CheckAudioTrack (TRUE, ke->k_msgp)) != TRUE) {
		return ( (status == FALSE) ? WARNING : status);
	}
	bzero ((char *) msf, sizeof(*msf));
	msf->msf_starting_M_unit = Starting_M_Unit;
	msf->msf_starting_S_unit = Starting_S_Unit;
	msf->msf_starting_F_unit = Starting_F_Unit;
	msf->msf_ending_M_unit = Ending_M_Unit;
	msf->msf_ending_S_unit = Ending_S_Unit;
	msf->msf_ending_F_unit = Ending_F_Unit;
	status = DoIoctl (ke->k_cmd, (caddr_t)msf, ke->k_msgp);
	return (status);
}

/************************************************************************
 *									*
 * PlayRange()	Play A Range of Tracks on the CD.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
PlayRange (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_play_audio_ti play_audio_ti;
	register struct cd_play_audio_ti *ti = &play_audio_ti;
	int status;

	bzero ((char *) ti, sizeof(*ti));
	if (EndingTrack < 0) {
	    EndingTrack = GetEndingTrack();
	}
	if ( (status =
	    CheckTrackRange (StartingTrack, EndingTrack, TRUE)) != SUCCESS) {
		return (status);
	}
	if ( (status =
	    CheckAudioRange (StartingTrack, EndingTrack, TRUE, ke->k_msgp))
	    != TRUE) {
		return ( (status == FALSE) ? WARNING : status);
	}
	ti->ti_starting_track = StartingTrack;
	ti->ti_ending_track = EndingTrack;
	ti->ti_starting_index = StartingIndex;
	ti->ti_ending_index = EndingIndex;
	EndingTrack = -1;
	return (DoIoctl (ce->c_cmd, (caddr_t)ti, ce->c_msgp));
}

/************************************************************************
 *									*
 * PlayTrack()	Play A Single Track on the CD.				*
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
PlayTrack (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct cd_play_audio_ti play_audio_ti;
	register struct cd_play_audio_ti *ti = &play_audio_ti;
	register struct cd_toc_entry *te;
	int status;

	if ( (status =
	    CheckTrackRange (TrackNumber, TrackNumber, TRUE)) != SUCCESS) {
		return (status);
	}
	if ((te = FindTOCEntry (TrackNumber)) == (struct cd_toc_entry *) 0) {
		return (FAILURE);
	}
	if ((status = CheckAudioEntry (te, TRUE, ke->k_msgp)) != TRUE) {
		return (FAILURE);
	}
	bzero ((char *) ti, sizeof(*ti));
	ti->ti_starting_track = TrackNumber;
	ti->ti_ending_track = TrackNumber;
	ti->ti_starting_index = StartingIndex;
	ti->ti_ending_index = EndingIndex;
	status = DoIoctl (ke->k_cmd, (caddr_t)ti, ke->k_msgp);
	return (status);
}

/************************************************************************
 *									*
 * PlayVTrack() - Play Track on the CD (Vendor Unique Command).		*
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
PlayVTrack (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct cd_play_track play_track;
	register struct cd_play_track *pt = &play_track;
	register struct cd_toc_entry *te;
	int status;

	if ( (status =
	    CheckTrackRange (TrackNumber, TrackNumber, TRUE)) != SUCCESS) {
		return (status);
	}
	if ((te = FindTOCEntry (TrackNumber)) == (struct cd_toc_entry *) 0) {
		return (FAILURE);
	}
	if ((status = CheckAudioEntry (te, TRUE, ke->k_msgp)) != TRUE) {
		return (FAILURE);
	}
	bzero ((char *) pt, sizeof(*pt));
	pt->pt_starting_track = TrackNumber;
	pt->pt_starting_index = StartingIndex;
	pt->pt_number_indexes = NumberIndexes;
	status = DoIoctl (ke->k_cmd, (caddr_t)pt, ke->k_msgp);
	return (status);
}

/************************************************************************
 *									*
 * SkipTracks() - Skip Backward/Forward Track(s) on the CD-ROM.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SkipTracks (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_toc_header toc_header;
	register struct cd_subc_information *sci;
	register struct cd_subc_header *sh;
	register struct cd_subc_position *scp;
	register struct cd_toc_header *th = &toc_header;
	register struct cmd_entry *pce;
	register struct key_entry *pke;
	int status, skip_count;

	sci = (struct cd_subc_information *) TmpBufPtr;
	sh = &sci->sci_header;
	scp = &sci->sci_scp;
	if ( (status = GetPlayPosition (sci)) != SUCCESS) {
		return (status);
	}
	if ( !((sh->sh_audio_status == AS_PLAY_IN_PROGRESS) ||
	       (sh->sh_audio_status == AS_PLAY_PAUSED)) ) {
		Printf ("Audio Play is NOT in progress...\n");
		return (WARNING);
	}
	if ( (status = GetTOCHeader (th)) != SUCCESS) {
		return (FAILURE);
	}
	skip_count = *((int *) ke->k_argp);
	if (ISSET_KOPT (ce, K_FORWARD_TRACK)) {
	    StartingTrack = (scp->scp_track_number + skip_count);
	    if (StartingTrack > th->th_ending_track) {
		StartingTrack = th->th_ending_track;
	    }
	} else {
	    StartingTrack = (scp->scp_track_number - skip_count);
	    if (StartingTrack < th->th_starting_track) {
		StartingTrack = th->th_starting_track;
	    }
	}

	/*
	 * Find the "PLAY range" command/keyword entrys, and go re-issue
	 * the play range command.
	 */
	if ((pce = FindCmdEntry (C_PLAY)) == (struct cmd_entry *) 0) {
		return (FAILURE);
	}
	if ((pke = FindKeyEntry (pce, 0, K_STARTING_TRACK)) == NULL) {
		return (FAILURE);
	}
	return (PlayRange (pce, pke));
}
