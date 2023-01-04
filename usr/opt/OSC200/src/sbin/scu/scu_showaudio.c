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
static char *rcsid = "@(#)$RCSfile: scu_showaudio.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 08:55:05 $";
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
 * File:	scu_showaudio.c
 * Author:	Robin T. Miller
 * Date:	August 8, 1991
 *
 * Description:
 *	This file contains functions for showing audio parameters.
 */

#include <sys/types.h>
#include <io/cam/cdrom.h>
#include <io/cam/rzdisk.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"
#include "scu_pages.h"
/*
 * Local Functions (forward reference):
 */
void PrintAddress(), PrintControl();
void PrintSUBCHeader(), PrintTOCHeader(), PrintTOCEntry();
char *PrintTOCTime();

/*
 * External Functions:
 */
extern u_long CvtMSFtoLBA (int minute_units,
			   int second_units, int frame_units);
extern void DumpHex();
extern int OpenPager (char *pager);
extern void Printf();
extern void PrintHeader (char *header);
extern int ShowAudioControl(), ShowErrorRecovery();
extern int ShowDiscoReco(), ShowDirectAccess(), ShowDiskGeometry();
extern int ShowDecSpecific(), ShowReadAhead();

#define TIME_BUFFER_SIZE	25	/* Size of time buffer.	*/

static char time_buffer[TIME_BUFFER_SIZE];
static char *time_bufptr = time_buffer;

static char *toch_str =			"Table Of Contents Information";
static char *tocs_str =			"Table of Contents Summary";
static char *data_len_str =		"Data Length";
static char *starting_track_str =	"Starting Track Number";
static char *ending_track_str =		"Ending Track Number";

static char *toce_str =			"Track Entry Information";
static char *track_str =		"Track Number";
static char *index_str =		"Index Number";
static char *addr_str =			"Address Type Information";
static char *control_str =		"Track Control Attributes";
static char *abs_LBA_str =		"Absolute Logical Block Address";
static char *abs_M_units_str =		"Absolute Minute-Units";
static char *abs_S_units_str =		"Absolute Second-Units";
static char *abs_F_units_str =		"Absolute Frame-Units";
static char *rel_LBA_str =		"Relative Logical Block Address";
static char *rel_M_units_str =		"Relative Minute-Units";
static char *rel_S_units_str =		"Relative Second-Units";
static char *rel_F_units_str =		"Relative Frame-Units";

static char *rhdr_str =			"Read Header Information";
static char *data_mode_str =		"Data Mode";

static char *data_mode_table[] = {
	"All Bytes Zero",
	"User Data With L-EC Symbols",
	"All User Data",
	"Reserved"
};

static char *psh_str =			"Playback Status Information";
static char *lbamsf_str =		"Address Format";
static char *audio_len_str =		"Audio Status Data Length";
char *chan0_select_str =		"Channel 0 Output Selection";
char *chan0_volume_str =		"Channel 0 Volume Level";
char *chan1_select_str =		"Channel 1 Output Selection";
char *chan1_volume_str =		"Channel 1 Volume Level";
char *chan2_select_str =		"Channel 2 Output Selection";
char *chan2_volume_str =		"Channel 2 Volume Level";
char *chan3_select_str =		"Channel 3 Output Selection";
char *chan3_volume_str =		"Channel 3 Volume Level";

static char *lbamsf_table[] = {
	"Logical Block Address (LBA)",
	"Minute/Second/Frame (MSF)"
};

char *port_select_table[] = {
	"Output Port Muted",					/* 0x00 */
	"Connect Channel 0 to this port",			/* 0x01 */
	"Connect Channel 1 to this port",			/* 0x02 */
	"Connect Channel 0 & 1 to this port"			/* 0x03 */
};

static char *addr_type_table[] = {
	"Sub-Channel Q Information Not Supplied",		/* 0x00 */
	"Sub-Channel Q Encodes Current Position Data",		/* 0x01 */
	"Sub-Channel Q Encodes Media Catalog Number",		/* 0x02 */
	"Sub-Channel Q Encodes ISRC Number"			/* 0x03 */
};

static char *with_pre_str =		"Audio With Pre-Emphasis";
static char *without_pre_str =		"Audio Without Pre-Emphasis";	
static char *copy_prohibited_str =	"Digital Copy Prohibited";
static char *copy_permitted_str =	"Digital Copy Permitted";
static char *audio_track_str =		"Audio Track";
static char *data_track_str =		"Data Track";
static char *two_chan_str =		"Two Channel Audio";
static char *four_chan_str =		"Four Channel Audio";

static char *mcval_str =		"Media Catalog Valid";
static char *mcnum_str =		"Media Catalog Number";
static char *tcval_str =		"Track Code Valid";
static char *tcnum_str =		"Track ISRC Number";

static char *subc_header_str =		"CD-ROM Sub-Channel Header";
static char *audio_status_str =		"Audio Status";

static char *data_format_table[] = {
	"Sub-Channel Data Information",				/* 0x00 */
	"Current Position Information",				/* 0x01 */
	"Media Catalog Number Information",			/* 0x02 */
	"ISRC Information"					/* 0x03 */
};

static char *audio_status_table[] = {
	"Audio Status Not Supported",				/* 0x00 */
	"0x01",							/* 0x01 */
	"0x02",							/* 0x02 */
	"0x03",							/* 0x03 */
	"0x04",							/* 0x04 */
	"0x05",							/* 0x05 */
	"0x06",							/* 0x06 */
	"0x07",							/* 0x07 */
	"0x08",							/* 0x08 */
	"0x09",							/* 0x09 */
	"0x0a",							/* 0x0a */
	"0x0b",							/* 0x0b */
	"0x0c",							/* 0x0c */
	"0x0d",							/* 0x0d */
	"0x0e",							/* 0x0e */
	"0x0f",							/* 0x0f */
	"0x10",							/* 0x10 */
	"Audio Play Operation In Progess",			/* 0x11 */
	"Audio Play Operation Paused",				/* 0x12 */
	"Audio Play Completed",					/* 0x13 */
	"Audio Play Stopped By Error",				/* 0x14 */
	"No Current Audio Status"				/* 0x15 */
};

static char *playback_status_table[] = {
	"Audio Play Operation In Progess",			/* 0x00 */
	"Audio Pause Operation In Progress",			/* 0x01 */
	"Audio Muting On",					/* 0x02 */
	"Audio Play Operation Successfully Completed",		/* 0x03 */
	"Error Occurred During Audio Play Operation",		/* 0x04 */
	"Audio Play Operation Not Requested",			/* 0x05 */
	"0x06",							/* 0x06 */
	"0x07"							/* 0x07 */
};

/************************************************************************
 *									*
 * ShowTOC() - Show Default CD-ROM Table of Contents.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowTOC (ce, ke)
register struct cmd_entry *ce;
struct key_entry *ke;
{
	if ( ISCLR_KOPT(ce, K_TOC_ENTRY) && ISCLR_KOPT(ce, K_TOC_HEADER) &&
	     ISCLR_KOPT(ce, K_TOC_FULL) && ISCLR_KOPT(ce, K_TOC_SUMMARY) ) {
		return (ShowTOCSummary (ce, ke));
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * ShowTOCHeader() - Show The CD-ROM Table of Contents Header.		*
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
ShowTOCHeader (ce, ke)
struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_toc_header toc_header;
	register struct cd_toc_header *th = &toc_header;
	int status;

	(void) bzero ((char *)th, sizeof(*th));
	if ( (status = DoIoctl (ke->k_cmd, (caddr_t) th, ke->k_msgp)) != 0) {
		return (status);
	}

	PrintTOCHeader (th);

	return (status);
}

/************************************************************************
 *									*
 * ShowTOCEntry() - Show An Entry In The CD-ROM Table of Contents.	*
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
ShowTOCEntry (ce, ke)
struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_toc Toc;
	register struct cd_toc *toc = &Toc;
	register struct cd_toc_entry *te;
	register u_char track_number;
	struct scu_device *scu = ScuDevice;
	int length, status;
	caddr_t bp;

	track_number = *((u_char *) ke->k_argp);
	if ( (status =
	    CheckTrackRange (track_number, track_number, TRUE)) != SUCCESS) {
		return (status);
	}
	(void) GetAddressFormat();
	length = sizeof(struct cd_toc_header) + sizeof(*te);
	bp = scu->scu_data_buffer;
	toc->toc_address_format = AddressFormat;
	toc->toc_starting_track = track_number;
	toc->toc_alloc_length = (u_short) length;
	toc->toc_buffer = bp;
	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)toc, ke->k_msgp)) != 0) {
		return (status);
	}
	te = (struct cd_toc_entry *)
			( (u_long)bp + sizeof(struct cd_toc_header) );

	PrintHeader (toce_str);

	PrintTOCEntry (te);

	return (status);
}

/************************************************************************
 *									*
 * ShowTOCFull() - Show Full CD-ROM Table of Contents.			*
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
ShowTOCFull (ce, ke)
struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_toc Toc;
	struct cd_toc_header toc_header;
	register struct cd_toc *toc = &Toc;
	register struct cd_toc_header *th = &toc_header;
	register struct cd_toc_entry *te;
	struct scu_device *scu = ScuDevice;
	int entrys, length, status;
	caddr_t bp;

	(void) GetAddressFormat();
	if ( (status = GetTOCHeader (th)) != SUCCESS) {
		return (FAILURE);
	}
	length = (int) ( ( (th->th_data_len1 << 8) +
			  th->th_data_len0) & 0xffff) + 2;
	bp = scu->scu_data_buffer;
	toc->toc_address_format = AddressFormat;
	toc->toc_starting_track = 0;
	toc->toc_alloc_length = (u_short) length;
	toc->toc_buffer = bp;

	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)toc, ke->k_msgp)) != 0) {
		return (status);
	}

	th = (struct cd_toc_header *) bp;
	te = (struct cd_toc_entry *) (bp + sizeof(*th));
	entrys = ((length - sizeof(*th)) / sizeof(*te));
	(void) OpenPager (NULL);
	PrintTOCHeader (th);

	while (entrys--) {
		Printf ("\n");
		PrintTOCEntry (te);
		te++;
	}
	return (status);
}

/************************************************************************
 *									*
 * ShowTOCSummary() - Show Summary of CD-ROM Table of Contents.		*
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
ShowTOCSummary (ce, ke)
struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_toc Toc;
	struct cd_toc_header toc_header;
	register struct cd_toc *toc = &Toc;
	register struct cd_toc_header *th = &toc_header;
	register struct cd_toc_entry *te, *fte, *cte;
	struct scu_device *scu = ScuDevice;
	int entrys, length, status;
	caddr_t bp;
	char *str;
	u_long starting_lba, ending_lba;
	int audio_tracks = 0, data_tracks = 0;

	if ( (status = GetTOCHeader (th)) != SUCCESS) {
		return (FAILURE);
	}
	length = (int) ( ( (th->th_data_len1 << 8) +
			  th->th_data_len0) & 0xffff) + 2;
	bp = scu->scu_data_buffer;
	toc->toc_address_format = CDROM_MSF_FORMAT;
	toc->toc_starting_track = 0;
	toc->toc_alloc_length = (u_short) length;
	toc->toc_buffer = bp;

	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)toc, ke->k_msgp)) != 0) {
		return (status);
	}

	th = (struct cd_toc_header *) bp;
	fte = te = (struct cd_toc_entry *) (bp + sizeof(*th));
	entrys = (th->th_ending_track - th->th_starting_track) + 1;
	MSFtoLBA_Adjust = CvtMSFtoLBA (te->te_msf.m_units,
				te->te_msf.s_units, te->te_msf.f_units);
	(void) OpenPager (NULL);
	PrintHeader (tocs_str);

	while (entrys--) {
		if (te->te_control & CDROM_DATA_TRACK) {
			data_tracks++;
			str = data_track_str;
		} else {
			audio_tracks++;
			str = audio_track_str;
		}
		starting_lba = CvtMSFtoLBA (te->te_msf.m_units,
				te->te_msf.s_units, te->te_msf.f_units);
		cte = te; te++;
		ending_lba = CvtMSFtoLBA (te->te_msf.m_units,
				te->te_msf.s_units, te->te_msf.f_units);
		if (cte->te_track_number < 10) {
			Printf ("    ");
		} else {
			Printf ("   ");
		}
		Printf ("Track %d: %s, Time: %s, LBA: %6d, Length: %d\n",
				cte->te_track_number, str,
				PrintTOCTime (cte, te),
				(starting_lba - MSFtoLBA_Adjust),
				(ending_lba - starting_lba) + 1);
	}
	Printf ("\nThere are %d audio track%s and %d data track%s,",
		audio_tracks, ((audio_tracks - 1) == 0) ? "" : "s",
		data_tracks, ((data_tracks - 1) == 0) ? "" : "s");
	Printf (" with a total time of %s.\n", PrintTOCTime (fte, te));
	return (status);
}

/************************************************************************
 *									*
 * ShowLBAHeader() - Show Logical Block Address Data Header.		*
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
ShowLBAHeader (ce, ke)
struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_read_header read_header;
	register struct cd_read_header *rh = &read_header;
	register struct cd_read_header_data *rhd;
	struct scu_device *scu = ScuDevice;
	u_long lba = *((u_long *) ke->k_argp);
	int status;

	(void) GetAddressFormat();
	/*
	 * Seek to the specified LBA first, and check for a data track,
	 * since the Read Header command fails on audio tracks.
	 */
	if ( (status = SeekPosition (lba)) != SUCCESS) {
		return (status);
	}
	if ( (status = CheckDataTrack (TRUE, ke->k_msgp)) != TRUE) {
		return ( (status == FALSE) ? WARNING : status);
	}

	rhd = (struct cd_read_header_data *) scu->scu_data_buffer;
	bzero ((char *)rh, sizeof(*rh));
	bzero ((char *)rhd, sizeof(*rhd));
	rh->rh_address_format = AddressFormat;
	rh->rh_lba = *((u_long *) ke->k_argp);
	rh->rh_alloc_length = sizeof(*rhd);
	rh->rh_buffer = (caddr_t) rhd;

	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)rh, ke->k_msgp)) != 0) {
		return (status);
	}

	PrintHeader (rhdr_str);

	PrintAscii (data_mode_str,
			data_mode_table[rhd->rhd_data_mode&0x3], PNL);

	PrintAddress (&rhd->rhd_absaddr, ABSOLUTE_ADDRESS, AddressFormat);

	return (status);
}

/************************************************************************
 *									*
 * ShowChannel() - Show Sub-Q Channel Data Information.			*
 *									*
 * NOTE: The SONY CDU-541 doesn't support Data Format Code 0x00 which	*
 *	 is normally used to return all Sub-Q Channel Data.		*
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
ShowChannel (ce, ke)
register struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct key_entry *ske;
	int status;

	if ((ske = FindKeyEntry (ce, 0, K_PLAY_POSITION)) == NULL) {
		return (FAILURE);
	}
	if ( (status = ShowPlayPosition (ce, ske)) != SUCCESS) {
		return (status);
	}

	if ((ske = FindKeyEntry (ce, 0, K_CATALOG)) == NULL) {
		return (FAILURE);
	}
	if ( (status = ShowCatalog (ce, ske)) != SUCCESS) {
		return (status);
	}

	if ((ske = FindKeyEntry (ce, 0, K_ISRC)) == NULL) {
		return (FAILURE);
	}
	if ( (status = ShowISRC (ce, ske)) != SUCCESS) {
		return (status);
	}

	return (status);
}

/************************************************************************
 *									*
 * ShowPlayPosition() - Show Current CD-ROM Play Position Information.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowPlayPosition (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_sub_channel sub_channel;
	register struct cd_sub_channel *sch = &sub_channel;
	register struct cd_subc_header *sh;
	register struct cd_subc_position *scp;
	struct scu_device *scu = ScuDevice;
	int length, status;
	caddr_t bp;

	(void) GetAddressFormat();
	length = sizeof(*sh) + sizeof(*scp);
	bp = scu->scu_data_buffer;
	bzero ((char *)sch, sizeof(*sch));
	sch->sch_address_format = AddressFormat;
	sch->sch_data_format = CDROM_CURRENT_POSITION;
	sch->sch_track_number = 0;
	sch->sch_alloc_length = (u_short) length;
	sch->sch_buffer = bp;
	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)sch, ke->k_msgp)) != 0) {
		return (status);
	}
	sh = (struct cd_subc_header *) bp;
	scp = (struct cd_subc_position *) (bp + sizeof(*sh));

	if (ISSET_KOPT (ce, K_CHANNEL)) {
		Printf ("\n");
	} else {
		PrintSUBCHeader (sh);
	}

	PrintHeader (data_format_table[scp->scp_data_format&0x03]);

	PrintNumeric (track_str, scp->scp_track_number, PNL);

	PrintNumeric (index_str, scp->scp_index_number, PNL);

	PrintAddress (&scp->scp_absaddr, ABSOLUTE_ADDRESS, AddressFormat);

	PrintAddress (&scp->scp_reladdr, RELATIVE_ADDRESS, AddressFormat);

	PrintControl (scp->scp_control);

	PrintAscii (addr_str,
			addr_type_table[scp->scp_addr_type&0x3], PNL);

	return (status);
}

/************************************************************************
 *									*
 * ShowCatalog() - Show The Media Catalog Number Information.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowCatalog (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_sub_channel sub_channel;
	register struct cd_sub_channel *sch = &sub_channel;
	register struct cd_subc_header *sh;
	register struct cd_subc_media_catalog *smc;
	struct scu_device *scu = ScuDevice;
	int length, status;
	caddr_t bp;
	char mc_number[sizeof(smc->smc_mc_number)+1];

	(void) GetAddressFormat();
	length = sizeof(*sh) + sizeof(*smc);
	bp = scu->scu_data_buffer;;
	bzero ((char *)sch, sizeof(*sch));
	sch->sch_address_format = AddressFormat;
	sch->sch_data_format = CDROM_MEDIA_CATALOG;
	sch->sch_track_number = 0;
	sch->sch_alloc_length = (u_short) length;
	sch->sch_buffer = bp;
	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)sch, ke->k_msgp)) != 0) {
		return (status);
	}
	sh = (struct cd_subc_header *) bp;
	smc = (struct cd_subc_media_catalog *) (bp + sizeof(*sh));

	if (ISSET_KOPT (ce, K_CHANNEL)) {
		Printf ("\n");
	} else {
		PrintSUBCHeader (sh);
	}

	PrintHeader (data_format_table[smc->smc_data_format&0x03]);

	PrintAscii (mcval_str, yesno_table[smc->smc_mc_valid], PNL);

	if (smc->smc_mc_valid) {
	    strncpy (mc_number, smc->smc_mc_number, sizeof(smc->smc_mc_number));
	    mc_number[sizeof(smc->smc_mc_number)] = '\0';
	    if (isprint (mc_number[0])) {
		PrintAscii (mcnum_str, mc_number, PNL);
	    } else {
		PrintAscii (mcnum_str, "", DNL);
		DumpHex (smc->smc_mc_number, sizeof(smc->smc_mc_number));
	    }
	}

	return (status);
}

/************************************************************************
 *									*
 * ShowISRC() - Show International Standard Recording Code Information.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowISRC (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_sub_channel sub_channel;
	register struct cd_sub_channel *sch = &sub_channel;
	register struct cd_subc_header *sh;
	register struct cd_subc_isrc_data *sid;
	struct scu_device *scu = ScuDevice;
	int length, status;
	caddr_t bp;
	char tc_number[sizeof(sid->sid_tc_number)+1];

	(void) GetAddressFormat();
	length = sizeof(*sh) + sizeof(*sid);
	bp = scu->scu_data_buffer;
	bzero ((char *)sch, sizeof(*sch));
	sch->sch_address_format = AddressFormat;
	sch->sch_data_format = CDROM_ISRC;
	sch->sch_track_number = ISRCTrackNumber;
	sch->sch_alloc_length = (u_short) length;
	sch->sch_buffer = bp;
	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)sch, ke->k_msgp)) != 0) {
		return (status);
	}
	sh = (struct cd_subc_header *) bp;
	sid = (struct cd_subc_isrc_data *) (bp + sizeof(*sh));

	if (ISSET_KOPT (ce, K_CHANNEL)) {
		Printf ("\n");
	} else {
		PrintSUBCHeader (sh);
	}

	PrintHeader (data_format_table[sid->sid_data_format&0x03]);

	PrintNumeric (track_str, sid->sid_track_number, PNL);

	PrintAscii (tcval_str, yesno_table[sid->sid_tc_valid], PNL);

	if (sid->sid_tc_valid) {
	    strncpy (tc_number, sid->sid_tc_number, sizeof(sid->sid_tc_number));
	    tc_number[sizeof(sid->sid_tc_number)] = '\0';
	    if (isprint (tc_number[0])) {
		PrintAscii (tcnum_str, tc_number, PNL);
	    } else {
		PrintAscii (tcnum_str, "", DNL);
		DumpHex (sid->sid_tc_number, sizeof(sid->sid_tc_number));
	    }
	}

	return (status);
}

/************************************************************************
 *									*
 * ShowPlayStatus() - Show the CD-ROM Playback Status.			*
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
ShowPlayStatus (ce, ke)
struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct cd_playback playback;
	register struct cd_playback *pb = &playback;
	register struct cd_playback_status *ps;
	struct scu_device *scu = ScuDevice;
	int length, status;

	/*
	 * Check for an audio track, since the Playback Status command
	 * fails on data tracks.
	 */
	if ( (status = CheckAudioTrack (TRUE, ke->k_msgp)) != TRUE) {
		return ( (status == FALSE) ? WARNING : status);
	}
	ps = (struct cd_playback_status *) scu->scu_data_buffer;;
	bzero ((char *)pb, sizeof(*pb));
	bzero ((char *)ps, sizeof(*ps));
	pb->pb_alloc_length = sizeof(*ps);
	pb->pb_buffer = (caddr_t) ps;

	if ( (status = DoIoctl (ke->k_cmd, (caddr_t)pb, ke->k_msgp)) != 0) {
		return (status);
	}

	PrintHeader (psh_str);

	PrintAscii (audio_status_str,
			playback_status_table[ps->ps_audio_status&0x07], PNL);

	length = (int) ( (ps->ps_data_len1 << 8) +
			  ps->ps_data_len0) & 0xffff;
	PrintNumeric (audio_len_str, length, PNL);

	PrintControl (ps->ps_control);

	PrintAscii (lbamsf_str, lbamsf_table[ps->ps_lbamsf], PNL);

	PrintAddress (&ps->ps_absaddr, ABSOLUTE_ADDRESS, ps->ps_lbamsf);

	PrintAscii (chan0_select_str,
			port_select_table[ps->ps_chan0_select&0x3], PNL);
	PrintNumeric (chan0_volume_str, ps->ps_chan0_volume, PNL);

	PrintAscii (chan1_select_str,
			port_select_table[ps->ps_chan1_select&0x3], PNL);
	PrintNumeric (chan1_volume_str, ps->ps_chan1_volume, PNL);

	PrintAscii (chan2_select_str,
			port_select_table[ps->ps_chan2_select&0x3], PNL);
	PrintNumeric (chan2_volume_str, ps->ps_chan2_volume, PNL);

	PrintAscii (chan3_select_str,
			port_select_table[ps->ps_chan3_select&0x3], PNL);
	PrintNumeric (chan3_volume_str, ps->ps_chan3_volume, PNL);

	return (status);
}

/************************************************************************
 *									*
 * PrintAddress() - Print The CD-ROM Address Value.			*
 *									*
 * Inputs:	addr = The CD-ROM address.				*
 *		address_type = The address type (Absolute or Relative).	*
 *		address_format = The address format (LBA or MSF).	*
 *									*
 * Return Value: Void.							*
 *									*
 ************************************************************************/
void
PrintAddress (addr, address_type, address_format)
register union cd_address *addr;
int address_type;
int address_format;
{
	char *m_units_str, *s_units_str,*f_units_str, *lba_str;

	if (address_type == ABSOLUTE_ADDRESS) {
		m_units_str = abs_M_units_str;
		s_units_str = abs_S_units_str;
		f_units_str = abs_F_units_str;
		lba_str = abs_LBA_str;
	} else {
		m_units_str = rel_M_units_str;
		s_units_str = rel_S_units_str;
		f_units_str = rel_F_units_str;
		lba_str = rel_LBA_str;
	}

	if (address_format == CDROM_MSF_FORMAT) {
		PrintNumeric (m_units_str, addr->msf.m_units, PNL);
		PrintNumeric (s_units_str, addr->msf.s_units, PNL);
		PrintNumeric (f_units_str, addr->msf.f_units, PNL);
	} else {
		u_long lba;

		lba = (u_long)( ((u_long)addr->lba.addr3 << 24) +
				((u_long)addr->lba.addr2 << 16) +
				((u_long)addr->lba.addr1 << 8) +
				(addr->lba.addr0) );
		PrintNumeric (lba_str, lba, PNL);
	}
}

/************************************************************************
 *									*
 * PrintControl() - Print The CD-ROM Control Information.		*
 *									*
 * Inputs:	control = The control information field.		*
 *									*
 * Return Value: Void.							*
 *									*
 ************************************************************************/
void
PrintControl (control)
register int control;
{
	if (control & CDROM_DATA_TRACK) {
		PrintAscii (control_str, data_track_str, PNL);
	} else {
		PrintAscii (control_str, audio_track_str, PNL);
	}
	if (control & CDROM_AUDIO_PREMPH) {
		PrintAscii ("", with_pre_str, PNL);
	} else {
		PrintAscii ("", without_pre_str, PNL);
	}
	if (control & CDROM_COPY_PERMITTED) {
		PrintAscii ("", copy_permitted_str, PNL);
	} else {
		PrintAscii ("", copy_prohibited_str, PNL);
	}
	if (control & CDROM_FOUR_CHAN_AUDIO) {
		PrintAscii ("", four_chan_str, PNL);
	} else {
		PrintAscii ("", two_chan_str, PNL);
	}
}

/************************************************************************
 *									*
 * PrintSUBCHeader() - Print the CD-ROM Sub-Channel Header.		*
 *									*
 * Inputs:	sh = The Sub-Channel header structure.			*
 *									*
 * Return Value: Void.							*
 *									*
 ************************************************************************/
void
PrintSUBCHeader (sh)
register struct cd_subc_header *sh;
{
	int length;

	PrintHeader (subc_header_str);

	PrintAscii (audio_status_str,
			audio_status_table[sh->sh_audio_status], PNL);

	length = (int) ( (sh->sh_data_len1 << 8) +
			  sh->sh_data_len0) & 0xffff;
	PrintNumeric (data_len_str, length, PNL);

	Printf ("\n");
}

/************************************************************************
 *									*
 * PrintTOCHeader() - Print the CD-ROM Table of Contents Header.	*
 *									*
 * Inputs:	th = The TOC header structure.				*
 *									*
 * Return Value: Void.							*
 *									*
 ************************************************************************/
void
PrintTOCHeader (th)
register struct cd_toc_header *th;
{
	int length;

	PrintHeader (toch_str);

	length = (int) ( (th->th_data_len1 << 8) +
			  th->th_data_len0) & 0xffff;
	PrintNumeric (data_len_str, length, PNL);

	PrintNumeric (starting_track_str, th->th_starting_track, PNL);

	PrintNumeric (ending_track_str, th->th_ending_track, PNL);
}

/************************************************************************
 *									*
 * PrintTOCEntry() - Print a CD-ROM Table of Contents Entry.		*
 *									*
 * Inputs:	te = The TOC entry structure.				*
 *									*
 * Return Value: Void.							*
 *									*
 ************************************************************************/
void
PrintTOCEntry (te)
register struct cd_toc_entry *te;
{
	PrintNumeric (track_str, te->te_track_number, PNL);

	PrintAddress (&te->te_absaddr, ABSOLUTE_ADDRESS, AddressFormat);

	PrintControl (te->te_control);

	PrintAscii (addr_str,
			addr_type_table[te->te_addr_type&0x3], PNL);
}

/************************************************************************
 *									*
 * PrintTOCTime() - Print Time From CD-ROM Table of Contents Entrys.	*
 *									*
 * Inputs:	te = Pointer to current TOC entry.			*
 *		nte = Pointer to next TOC entry.			*
 *									*
 * Return Value: Void.							*
 *									*
 ************************************************************************/
char *
PrintTOCTime (te, nte)
register struct cd_toc_entry *te, *nte;
{
	int next_track_seconds, this_track_seconds;
	int track_seconds;

	this_track_seconds = (  (te->te_msf.m_units * S_Units_per_M_Unit) +
				(te->te_msf.s_units) +
			( (te->te_msf.f_units * 2) / F_Units_per_S_Unit) );

	next_track_seconds = (  (nte->te_msf.m_units * S_Units_per_M_Unit) +
				(nte->te_msf.s_units) +
			( (te->te_msf.f_units * 2) / F_Units_per_S_Unit) );

	track_seconds = (next_track_seconds - this_track_seconds);

	(void) sprintf (time_bufptr, "%02d:%02d",
				(track_seconds / 60), (track_seconds % 60));

	return (time_bufptr);
}
