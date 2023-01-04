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
static char *rcsid = "@(#)$RCSfile: scu_audio.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 08:53:47 $";
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
 * File:	scu_audio.c - 'scu' Audio Functions.
 * Author:	Robin T. Miller
 * Date:	August 8, 1991
 *
 * Description:
 *	This file contains the audio support functions.
 */

#include <stdio.h>
#include <io/cam/cdrom.h>

#include "scu.h"
#include "scu_device.h"

/*
 * External References:
 */
extern void Fprintf();
extern void PrintNumeric (char *field_str, int numeric_value, int nl_flag);

/*
 * Forward References:
 */
u_long CvtMSFtoLBA (int minute_units, int second_units, int frame_units);

/************************************************************************
 *									*
 * CheckAudioEntry() - Check TOC Entry for being an Audio Track.	*
 *									*
 * Inputs:	te = Pointer to the track entry.			*
 *		warn_user = Flag to control warning message.		*
 *		cmd_name = Pointer to command name text.		*
 *									*
 * Return Value:							*
 *		Returns TRUE (audio) / FALSE (data)			*
 *									*
 ************************************************************************/
int
CheckAudioEntry (te, warn_user, cmd_name)
register struct cd_toc_entry *te;
int warn_user;
caddr_t cmd_name;
{
	int status;

	if ((te->te_control & CDROM_DATA_TRACK) == 0) {
	    status = TRUE;			/* Audio Track */
	} else {
	    status = FALSE;			/* Data Track */
	    if (warn_user) {
		fprintf (stderr,
		    "%s: Command '%s' is illegal on data track %d.\n",
				OurName, cmd_name, te->te_track_number);
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * CheckDataEntry() - Check TOC Entry for being a Data Track.		*
 *									*
 * Inputs:	te = Pointer to the track entry.			*
 *		warn_user = Flag to control warning message.		*
 *		cmd_name = Pointer to command name text.		*
 *									*
 * Return Value:							*
 *		Returns TRUE (data) / FALSE (audio)			*
 *									*
 ************************************************************************/
int
CheckDataEntry (te, warn_user, cmd_name)
register struct cd_toc_entry *te;
int warn_user;
caddr_t cmd_name;
{
	int status;

	if (te->te_control & CDROM_DATA_TRACK) {
	    status = TRUE;			/* Data Track */
	} else {
	    status = FALSE;			/* Audio Track */
	    if (warn_user) {
		fprintf (stderr,
		    "%s: Command '%s' is illegal on audio track %d.\n",
				OurName, cmd_name, te->te_track_number);
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * CheckAudioTrack() - Check Current Position being an Audio Track.	*
 *									*
 * Inputs:	warn_user = Flag to control warning message.		*
 *		cmd_name = Pointer to command name text.		*
 *									*
 * Return Value:							*
 *		Returns TRUE (audio) / FALSE (data) / FAILURE (error)	*
 *									*
 ************************************************************************/
int
CheckAudioTrack (warn_user, cmd_name)
int warn_user;
caddr_t cmd_name;
{
	register struct cd_subc_information *sci;
	register struct cd_subc_position *scp;
	struct scu_device *scu = ScuDevice;
	int status;

	sci = (struct cd_subc_information *) scu->scu_tmp_buffer;
	scp = &sci->sci_scp;
	if ( (status = GetPlayPosition (sci)) == SUCCESS) {
	    if ( (scp->scp_control & CDROM_DATA_TRACK) == 0) {
		status = TRUE;			/* Audio Track */
	    } else {
		status = FALSE;			/* Data Track */
		if (warn_user) {
		    fprintf (stderr,
			"%s: Command '%s' is illegal on data track %d.\n",
				OurName, cmd_name, scp->scp_track_number);
		}
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * CheckDataTrack() - Check Current Position being a Data Track.	*
 *									*
 * Inputs:	warn_user = Flag to control warning message.		*
 *		cmd_name = Pointer to command name text.		*
 *									*
 * Return Value:							*
 *		Returns TRUE (audio) / FALSE (data) / FAILURE (error)	*
 *									*
 ************************************************************************/
int
CheckDataTrack (warn_user, cmd_name)
int warn_user;
caddr_t cmd_name;
{
	register struct cd_subc_information *sci;
	register struct cd_subc_position *scp;
	struct scu_device *scu = ScuDevice;
	int status;

	sci = (struct cd_subc_information *) scu->scu_tmp_buffer;
	scp = &sci->sci_scp;
	if ( (status = GetPlayPosition (sci)) == SUCCESS) {
	    if (scp->scp_control & CDROM_DATA_TRACK) {
		status = TRUE;			/* Data Track */
	    } else {
		status = FALSE;			/* Audio Track */
		if (warn_user) {
		    fprintf (stderr,
			"%s: Command '%s' is illegal on audio track %d.\n",
				OurName, cmd_name, scp->scp_track_number);
		}
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * CheckAudioRange() - Check Range of Track for being Audio Tracks.	*
 *									*
 * Inputs:	starting_track = The starting track number.		*
 *		ending_track = The ending track number.			*
 *		warn_user = Flag to control warning message.		*
 *		cmd_name = Pointer to command name text.		*
 *									*
 * Return Value:							*
 *		Returns TRUE (audio) / FALSE (data) / FAILURE (error)	*
 *									*
 ************************************************************************/
int
CheckAudioRange (starting_track, ending_track, warn_user, cmd_name)
register int starting_track, ending_track;
int warn_user;
caddr_t cmd_name;
{
	struct cd_toc Toc;
	struct cd_toc_header toc_header;
	register struct cd_toc *toc = &Toc;
	register struct cd_toc_header *th = &toc_header;
	register struct cd_toc_entry *te;
	register struct key_entry *ke;
	struct scu_device *scu = ScuDevice;
	int entrys, length, status;
	caddr_t bp;

	if ( (status = GetTOCHeader (th)) != SUCCESS) {
		return (status);
	}
	length = (int) ( ( (th->th_data_len1 << 8) +
			  th->th_data_len0) & 0xffff) + 2;
	bp = scu->scu_tmp_buffer;
	toc->toc_address_format = AddressFormat;
	toc->toc_starting_track = 0;
	toc->toc_alloc_length = (u_short) length;
	toc->toc_buffer = bp;

	if ((ke = FindKeyEntry (NULL, C_SHOW, K_TOC)) == NULL) {
		return (FAILURE);
	}
	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)toc, ke->k_msgp)) != 0) {
		return (FAILURE);
	}

	th = (struct cd_toc_header *) bp;
	te = (struct cd_toc_entry *) (bp + sizeof(*th));
	entrys = ((length - sizeof(*th)) / sizeof(*te));

	/*
	 * Loop through all entrys checking the track range.
	 */
	while (entrys--) {
	    if ( (te->te_track_number >= starting_track) &&
		 (te->te_track_number <= ending_track) ) {
		if (CheckAudioEntry (te, warn_user, cmd_name) == FALSE) {
			return (FALSE);
		}
	    }
	    if (te->te_track_number > ending_track) {
		break;
	    } else {
		te++;
	    }
	}
	return (TRUE);
}

/************************************************************************
 *									*
 * CheckTrackRange() - Check Track Number Range.			*
 *									*
 * Inputs:	starting_track = The starting track number.		*
 *		ending_track = The ending track number.			*
 *		warn_user = Flag to control warning message.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
CheckTrackRange (starting_track, ending_track, warn_user)
register int starting_track, ending_track;
int warn_user;
{
	struct cd_toc_header toc_header;
	register struct cd_toc_header *th = &toc_header;
	int status;

	if (ending_track < starting_track) {
		if (warn_user) {
		    fprintf (stderr,
	"%s: Ending track of %d MUST be greater than starting track of %d.\n",
				OurName, ending_track, starting_track);
		}
		return (FAILURE);
	}
	if ( (status = GetTOCHeader (th)) != SUCCESS) {
		return (status);
	}
	if ( ((starting_track < th->th_starting_track) ||
	      (starting_track > th->th_ending_track)) ||
	     ((ending_track < th->th_starting_track) ||
	      (ending_track > th->th_ending_track)) ) {
		if (warn_user) {
		    fprintf (stderr,
			"%s: Track number(s) MUST be in range (%d - %d).\n",
			OurName, th->th_starting_track, th->th_ending_track);
		}
		return (FAILURE);
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * DoCdromCmd()	Setup & Do a CDROM I/O Control Command.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoCdromCmd (ce)
register struct cmd_entry *ce;
{
	int status;

	switch (ce->c_token) {

	    case C_PAUSE_PLAY:
	    case C_RESUME_PLAY:
	    case C_START_UNIT:
	    case C_STOP_UNIT:
	    case C_EJECT_CADDY:
	    case C_ALLOW_REMOVAL:
	    case C_PREVENT_REMOVAL:
		status = DoIoctl (ce->c_cmd, ce->c_argp, ce->c_msgp);
		break;

	    case C_PLAY: {
		register struct key_entry *ke;

		if ((ke = FindKeyEntry (ce, 0, K_STARTING_TRACK)) == NULL) {
			return (FAILURE);
		}
		return (PlayRange (ce, ke));
		/* NOT REACHED */
	    }
	    case C_SKIP: {
		register struct key_entry *ke;

		if ((ke = FindKeyEntry (ce, 0, K_FORWARD_TRACK)) == NULL) {
			return (FAILURE);
		}
		SET_KOPT (ce, ke->k_token);
		return (SkipTracks (ce, ke));
		/* NOT REACHED */
	    }
	    default:
		Fprintf (
		    "Command '%s', I/O Control command 0x%x, is incomplete.",
						ce->c_name, ce->c_cmd);
		status = FAILURE;
	}
	return (status);
}

/************************************************************************
 *									*
 * FindTOCEntry() - Find A CD-ROM Table Of Contents Entry.		*
 *									*
 * Inputs:	track_number = The track number to find.		*
 *									*
 * Return Value:							*
 *		Returns the toc entry or 0 if not found.		*
 *									*
 ************************************************************************/
struct cd_toc_entry *
FindTOCEntry (track_number)
int track_number;
{
	struct cd_toc Toc;
	struct cd_toc_header toc_header;
	register struct cd_toc *toc = &Toc;
	register struct cd_toc_header *th = &toc_header;
	register struct cd_toc_entry *te;
	register struct key_entry *ke;
	struct scu_device *scu = ScuDevice;
	int entrys, length;
	caddr_t bp;

	if (GetTOCHeader (th) != SUCCESS) {
		return ((struct cd_toc_entry *) 0);
	}
	length = (int) ( ( (th->th_data_len1 << 8) +
			  th->th_data_len0) & 0xffff) + 2;
	bp = scu->scu_tmp_buffer;
	toc->toc_address_format = AddressFormat;
	toc->toc_starting_track = 0;
	toc->toc_alloc_length = (u_short) length;
	toc->toc_buffer = bp;

	if ((ke = FindKeyEntry (NULL, C_SHOW, K_TOC)) == NULL) {
		return ((struct cd_toc_entry *) 0);
	}
	if (DoIoctl (ke->k_cmd, (caddr_t)toc, ke->k_msgp) != SUCCESS) {
		return ((struct cd_toc_entry *) 0);
	}

	th = (struct cd_toc_header *) bp;
	te = (struct cd_toc_entry *) (bp + sizeof(*th));
	entrys = ((length - sizeof(*th)) / sizeof(*te));

	/*
	 * Loop through entrys looking for desired track number.
	 */
	while (entrys--) {
		if (te->te_track_number == track_number) {
			return (te);
		} else {
			te++;
		}
	}
	return ((struct cd_toc_entry *) 0);
}

/************************************************************************
 *									*
 * GetAddressFormat() - Get The CD-ROM Default Address Format.		*
 *									*
 * Inputs:	None.
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
GetAddressFormat()
{
	struct cd_playback_status playback_status;
	register struct cd_playback_status *ps = &playback_status;
	int status = SUCCESS;

	if (AddressFormat < 0) {
	    AddressFormat = CDROM_LBA_FORMAT;	/* Set default format. */
	    /*
	     * Ensure the current position is an audio track, otherwise
	     * the playback status command will fail.
	     */
	    if ( (status = CheckAudioTrack (FALSE, "")) != TRUE) {
		return ( (status == FALSE) ? WARNING : status);
	    }
	    bzero ((char *)ps, sizeof(*ps));
	    if ( (status = GetPlaybackStatus (ps)) == SUCCESS) {
		AddressFormat = ps->ps_lbamsf;	/* LBA or MSF format. */
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * GetEndingTrack() - Get Ending Track of CD-ROM.			*
 *									*
 * Inputs:	None.							*
 *									*
 * Return Value:							*
 *		Returns ending track number from Table of Contents.	*
 *									*
 ************************************************************************/
int
GetEndingTrack()
{
	struct cd_toc_header toc_header;
	register struct cd_toc_header *th = &toc_header;
	int status;

	if ( (status = GetTOCHeader (th)) != SUCCESS) {
		return (status);
	}
	return (th->th_ending_track);
}

/************************************************************************
 *									*
 * GetTOCHeader() - Get The CD-ROM Table Of Contents Header.		*
 *									*
 * Inputs:	th = Pointer to TOC header structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
GetTOCHeader (th)
register struct cd_toc_header *th;
{
	register struct key_entry *ke;

	if ((ke = FindKeyEntry (NULL, C_SHOW, K_TOC_HEADER)) == NULL) {
		return (FAILURE);
	}
	return (DoIoctl (ke->k_cmd, (caddr_t)th, ke->k_msgp));
}

/************************************************************************
 *									*
 * GetPlaybackStatus() - Get The CD-ROM Playback Status.		*
 *									*
 * Inputs:	ps = Pointer to playback status structure.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
GetPlaybackStatus (ps)
register struct cd_playback_status *ps;
{
	struct cd_playback playback;
	register struct cd_playback *pb = &playback;
	register struct key_entry *ke;

	if ((ke = FindKeyEntry (NULL, C_SHOW, K_STATUS)) == NULL) {
		return (FAILURE);
	}
	bzero ((char *)pb, sizeof(*pb));
	pb->pb_alloc_length = sizeof(*ps);
	pb->pb_buffer = (caddr_t) ps;
	return (DoIoctl (ke->k_cmd, (caddr_t)pb, ke->k_msgp));
}

/************************************************************************
 *									*
 * GetPlayPosition() - Get Current Play CD-ROM Position Information.	*
 *									*
 * Inputs:	sci = Pointer to sub-channel information structure.	*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
GetPlayPosition (sci)
register struct cd_subc_information *sci;
{
	struct cd_sub_channel sub_channel;
	register struct cd_sub_channel *sch = &sub_channel;
	register struct key_entry *ke;
	int length;

	if ((ke = FindKeyEntry (NULL, C_SHOW, K_PLAY_POSITION)) == NULL) {
		return (FAILURE);
	}
	length = sizeof(struct cd_subc_header) +
				 sizeof(struct cd_subc_position);
	bzero ((char *)sch, sizeof(*sch));
	bzero ((char *)sci, sizeof(*sci));
	sch->sch_address_format = AddressFormat;
	sch->sch_data_format = CDROM_CURRENT_POSITION;
	sch->sch_track_number = 0;
	sch->sch_alloc_length = (u_short) length;
	sch->sch_buffer = (caddr_t) sci;
	return (DoIoctl (ke->k_cmd, (caddr_t)sch, ke->k_msgp));
}

/************************************************************************
 *									*
 * CvtMSFtoLBA() - Convert MSF Address to an Logical Block Address.	*
 *									*
 * Inputs:	minute_units = The M-Units value.			*
 *		second_units = The S-Units value.			*
 *		frame_units = The F-Units value.			*
 *									*
 * Return Value:							*
 *		Returns the calculated LBA.				*
 *									*
 ************************************************************************/
u_long
CvtMSFtoLBA (minute_units, second_units, frame_units)
register int minute_units, second_units, frame_units;
{
	u_long lba;

	lba = ( ( ( (minute_units * S_Units_per_M_Unit) +
		     second_units) * F_Units_per_S_Unit) +
		     frame_units ) * BlocksPerCDBlock;
	return (lba);
}
