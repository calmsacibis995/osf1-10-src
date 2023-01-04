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
static char *rcsid = "@(#)$RCSfile: scu_setaudio.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/12/15 20:57:04 $";
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
 * File:	scu_setauduio.c
 * Author:	Robin T. Miller
 * Date:	August 8, 1991
 *
 * Description:
 *	This file contains functions for setting audio parameters.
 */

#include "scu.h"
#include "scu_device.h"
#include <io/cam/cdrom.h>

/************************************************************************
 *									*
 * SetAddressFormat()  Set CD-ROM Default Address Format.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetAddressFormat (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	int status = SUCCESS;

	if (ce->c_token == C_SET) {
	    status = DoIoctl (ke->k_cmd, ke->k_argp, ke->k_msgp);
	}
	return (status);
}

/************************************************************************
 *									*
 * SetSonyVolume()  Set CD-ROM Volume Control (Sony Vendor Unique).	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetSonyVolume (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_playback playback;
	struct cd_playback_status playback_status;
	register struct cd_playback *pb = &playback;
	register struct cd_playback_status *ps = &playback_status;
	register struct cd_playback_control *pc;
	struct scu_device *scu = ScuDevice;
	int status;

	/*
	 * Check for an audio track, since the Playback Control command
	 * fails on data tracks.
	 */
	if ( (status = CheckAudioTrack (TRUE, ke->k_msgp)) != TRUE) {
		return ( (status == FALSE) ? WARNING : status);
	}
	pc = (struct cd_playback_control *) scu->scu_data_buffer;
	bzero ((char *)pb, sizeof(*pb));
	bzero ((char *)pc, sizeof(*pc));
	bzero ((char *)ps, sizeof(*ps));
	pb->pb_alloc_length = sizeof(*pc);
	pb->pb_buffer = (caddr_t) pc;

	if ( (status = GetPlaybackStatus(ps)) != SUCCESS) {
	    return (status);
	}

	/*
	 * NOTE: The current firmware only supports channels 0 & 1.
	 */
	if (ISSET_KOPT (ce, K_CHAN0_SELECT)) {
	    pc->pc_chan0_select = Channel_0_Select;
	} else {
	    pc->pc_chan0_select = ps->ps_chan0_select;
	}

	if (ISSET_KOPT (ce, K_CHAN0_VOLUME)) {
	    pc->pc_chan0_volume = Channel_0_Volume;
	} else {
	    pc->pc_chan0_volume = ps->ps_chan0_volume;
	}

	if (ISSET_KOPT (ce, K_CHAN1_SELECT)) {
	    pc->pc_chan1_select = Channel_1_Select;
	} else {
	    pc->pc_chan1_select = ps->ps_chan1_select;
	}

	if (ISSET_KOPT (ce, K_CHAN1_VOLUME)) {
	    pc->pc_chan1_volume = Channel_1_Volume;
	} else {
	    pc->pc_chan1_volume = ps->ps_chan1_volume;
	}

	if (ISSET_KOPT (ce, K_VOLUME_LEVEL)) {
	    pc->pc_chan0_volume = VolumeLevel;
	    pc->pc_chan1_volume = VolumeLevel;
	}

	return (DoIoctl (ke->k_cmd, (caddr_t)pb, ke->k_msgp));
}
