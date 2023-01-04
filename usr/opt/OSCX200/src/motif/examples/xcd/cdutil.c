/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1993, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  ALL RIGHTS RESERVED
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN DIGITAL
*  EQUIPMENT CORPORATION OR ITS THIRD PARTY SUPPLIERS  
*  
*  DIGITAL EQUIPMENT CORPORATION AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND DIGITAL EQUIPMENT CORPORATION EXPRESSLY DISCLAIMS ALL IMPLIED 
*  WARRANTIES, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
*  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*  
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*
 * File:	cdutil.c
 * Description:
 *	This file contains various utility functions.
 *
 * Modified:	December 4, 1993
 *
 * Remove Sony vendor unique commands used with RRD42, since the new
 * CD-ROM's are Toshiba drives (RRD43 & RRD44).  This required removing
 * the SetAddrFormatMsf() & GetPlaybackStatus() functions, and rewriting
 * the SetVolume() function to avoid using the Playback Control command.
 * Setting of volume is now done with a Mode Select of the Audio Control
 * Page.  NOTE:  Neither drive supports saving mode page parameters.
 *
 */

#include <stdio.h>
#include <sys/types.h>

#ifdef __osf__
#include <io/cam/rzdisk.h>
#else
#include <sys/rzdisk.h>
#endif

#include "cdutil.h"

/* Make this a global so that it doesn't have to be passed to all of
 * the routines.
 */
int CdFd;	/* File descriptor for CDROM */

struct PageHeader {
	struct mode_parameter_header parameter_header;
	struct mode_block_descriptor block_descriptor;
	struct scsi_page_header page_header;
};

static u_char PageCode = AUDIO_CONTROL_PAGE;
static u_char SaveModeParameters = TRUE;

struct audio_params {				/* Audio Control page.	*/
	struct mode_page_header header;
	struct scsi_audio_control page0e;
};

static struct audio_params de_audio_params;	/* Default parameters.	*/

/************************************************************************
 *									*
 * DoIoctl()	Do An I/O Control Command.				*
 *									*
 * Description:								*
 *	This function issues the specified I/O control command to the	*
 * xpr psuedo device driver.						*
 *									*
 * Inputs:	cmd = The I/O control command.				*
 *		argp = The command argument to pass.			*
 *		msgp = The message to display on errors.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int DoIoctl
#ifdef _NO_PROTO
(cmd, argp, msgp)
    int 	cmd;
    char *	argp;
    char *	msgp;
#else
(
    int		cmd,
    char *	argp,
    char *	msgp
)
#endif /* _NO_PROTO */
{
	int status;

	if(CdFd) {
		status = ioctl (CdFd, cmd, argp);
		return (status);
	} else
		return(-1);
}

int StopUnit()
{
	int cmd = SCSI_STOP_UNIT;
	char * argp = NULL;
	char * msgp = "SCSI_STOP_UNIT";

	return (DoIoctl (cmd, argp, msgp));
}

int EjectUnit()
{
	int cmd = CDROM_EJECT_CADDY;
	char * argp = NULL;
	char * msgp = "CDROM_EJECT_CADDY";

	return (DoIoctl (cmd, argp, msgp));
}

int PausePlay()
{
	int cmd = CDROM_PAUSE_PLAY;
	char * argp = NULL;
	char * msgp = "CDROM_PAUSE_PLAY";

	return (DoIoctl (cmd, argp, msgp));
}

int ResumePlay()
{
	int cmd = CDROM_RESUME_PLAY;
	char * argp = NULL;
	char * msgp = "CDROM_RESUME_PLAY";

	return (DoIoctl (cmd, argp, msgp));
}

int PreventRemoval()
{
	int cmd = SCSI_PREVENT_REMOVAL;
	char * argp = NULL;
	char * msgp = "SCSI_PREVENT_REMOVAL";

	return (DoIoctl (cmd, argp, msgp));
}

int AllowRemoval()
{
	int cmd = SCSI_ALLOW_REMOVAL;
	char * argp = NULL;
	char * msgp = "SCSI_ALLOW_REMOVAL";

	return (DoIoctl (cmd, argp, msgp));
}

int PlayTrack
#ifdef _NO_PROTO
(starting_track, ending_track)
    int starting_track, ending_track;
#else
(
    int starting_track,
    int ending_track
)
#endif /* _NO_PROTO */
{
	struct cd_play_audio_ti play_audio_ti;
	register struct cd_play_audio_ti *ti = &play_audio_ti;
	int cmd = CDROM_PLAY_AUDIO_TI;
	char * argp = (char *) ti;
	char * msgp = "CDROM_PLAY_AUDIO_TI";

	BZERO ((char *) ti, sizeof(*ti));
	ti->ti_starting_track = starting_track;
	ti->ti_ending_track = ending_track;
	ti->ti_starting_index = 1;
	ti->ti_ending_index = 1;
	return (DoIoctl (cmd, argp, msgp));
}

int PlayTime
#ifdef _NO_PROTO
(starting_time, ending_time)
    union cd_address starting_time, ending_time;
#else
(
    union cd_address starting_time,
    union cd_address ending_time
)
#endif /* _NO_PROTO */
{
	struct cd_play_audio_msf play_audio_msf;
	register struct cd_play_audio_msf *msf = &play_audio_msf;
	int cmd = CDROM_PLAY_AUDIO_MSF;
	char * argp = (char *) msf;
	char * msgp = "CDROM_PLAY_AUDIO_MSF";

	BZERO ((char *) msf, sizeof(*msf));
	msf->msf_starting_M_unit = starting_time.msf.m_units;
	msf->msf_starting_S_unit = starting_time.msf.s_units;
	msf->msf_starting_F_unit = starting_time.msf.f_units;
	msf->msf_ending_M_unit = ending_time.msf.m_units;
	msf->msf_ending_S_unit = ending_time.msf.s_units;
	msf->msf_ending_F_unit = ending_time.msf.f_units;
	return (DoIoctl (cmd, argp, msgp));
}

/************************************************************************
 *									*
 * GetTotalTracks() - Get Total Tracks on the CD-ROM.			*
 *									*
 * Inputs:	None.							*
 *									*
 * Return Value:							*
 *		Returns total tracks Table of Contents.			*
 *									*
 ************************************************************************/
int GetTotalTracks()
{
	register struct cd_toc_header toc_header;
	register struct cd_toc_header *th = &toc_header;
	int status;

	if ( (status = GetTOCHeader (th)) != CD_SUCCESS) {
		return (FAILURE);
	}
	return ((th->th_ending_track - th->th_starting_track) + 1);
}

/************************************************************************
 *									*
 * GetPlayPosition() - Get Current Play CD-ROM Position Information.	*
 *									*
 * Inputs:	sci = Pointer to sub-channel information structure.	*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int GetPlayPosition
#ifdef _NO_PROTO
(sci)
    register struct cd_subc_information *sci;
#else
(
    register struct cd_subc_information *sci
)
#endif /* _NO_PROTO */
{
	struct cd_sub_channel sub_channel;
	register struct cd_sub_channel *sch = &sub_channel;
	register struct cd_subc_header *sh = &sci->sci_header;
	register struct cd_subc_position *scp = &sci->sci_scp;
	int cmd = CDROM_READ_SUBCHANNEL;
	char * argp = (char *) sch;
	char * msgp = "CDROM_READ_SUBCHANNEL";
	int length, err;

	length = sizeof(*sh) + sizeof(*scp);
	BZERO ((char *)sch, sizeof(*sch));
	sch->sch_address_format = CDROM_MSF_FORMAT;
	sch->sch_data_format = CDROM_CURRENT_POSITION;
	sch->sch_track_number = 0;
	sch->sch_alloc_length = (u_short) length;
	sch->sch_buffer = (char *) sci;
	return (DoIoctl (cmd, argp, msgp));
}

/************************************************************************
 *									*
 * GetTOCHeader() - Get The CD-ROM Table Of Contents Header.		*
 *									*
 * Inputs:	th = Pointer to TOC header structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int GetTOCHeader
#ifdef _NO_PROTO
(th)
    register struct cd_toc_header *th;
#else
(
    register struct cd_toc_header *th
)
#endif /* _NO_PROTO */
{
	int cmd = CDROM_TOC_HEADER;
	char * argp = (char *) th;
	char * msgp = "CDROM_TOC_HEADER";
	int status;

	return (DoIoctl (cmd, argp, msgp));
}

/************************************************************************
 *									*
 * GetTOC() - Get The Table Of Contents.				*
 *									*
 * Inputs:	toc_buffer = Pointer to TOC buffer.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int GetTOC
#ifdef _NO_PROTO
(toc_buffer)
    struct cd_toc_head_and_entries *toc_buffer;
#else
(
    struct cd_toc_head_and_entries *toc_buffer
)
#endif /* _NO_PROTO */
{
	int status, length;
	struct cd_toc Toc;
	int cmd = CDROM_TOC_ENTRYS;
	char * argp = (char *) &Toc;
	char * msgp = "CDROM_TOC_ENTRYS";
	struct cd_toc_header *th;

	th = &toc_buffer->cdth;
	if ( (status = GetTOCHeader (th)) != CD_SUCCESS) {
		return (FAILURE);
	}
	length = (int) ( ( (th->th_data_len1 << 8) +
			  th->th_data_len0) & 0xffff) + 2;
	Toc.toc_address_format = CDROM_MSF_FORMAT;
	Toc.toc_starting_track = 0;
	Toc.toc_alloc_length = (u_short) length;
	Toc.toc_buffer = (char *)toc_buffer;

	if ( (status = DoIoctl (cmd, argp, msgp)) != 0) {
		return (status);
	}

	return (status);
}

void set_fd
#ifdef _NO_PROTO
(fd)
    int fd;
#else
(
    int fd
)
#endif /* _NO_PROTO */
{

	CdFd = fd;
}

/************************************************************************
 *									*
 * SetVolume()  Set CD-ROM Volume Control.				*
 *									*
 * Inputs:	left_channel_volume = Left channel volume level.	*
 *		right_channel_volume = Right channel volume level.	*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int SetVolume
#ifdef _NO_PROTO
(left_channel_volume, right_channel_volume)
    int left_channel_volume, right_channel_volume;
#else
(
    int left_channel_volume, 
    int right_channel_volume
)
#endif /* _NO_PROTO */
{
	struct cd_subc_information sci;
	register struct audio_params *ms = &de_audio_params;
	register struct scsi_audio_control *ac = &ms->page0e;
	int status;

	if ( (status = GetPlayPosition(&sci)) != CD_SUCCESS) {
	    return (status);
	}

	/*
	 * Get the current mode page values first.
	 */
	if ((status = SenseAudioPage (ms, PCF_CURRENT)) != CD_SUCCESS) {
	    return (status);
	}

	ac->ac_chan0_volume = left_channel_volume;
	ac->ac_chan1_volume = right_channel_volume;

	return (SelectAudioPage (ms, SaveModeParameters));
}

/************************************************************************
 *									*
 * SenseAudioPage() - Sense The CD-ROM Audio Control Page.		*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseAudioPage (ms, pcf)
struct audio_params *ms;
u_char pcf;
{
    return (SensePage (ms, pcf, PageCode, sizeof *ms));
}

/************************************************************************
 *									*
 * SelectAudioPage() - Select The CD-ROM Audio Control Page.		*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectAudioPage (ms, smp)
struct audio_params *ms;
u_char smp;
{
	struct audio_params se_audio_params;
	register struct audio_params *se = &se_audio_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}

/************************************************************************
 *									*
 * SensePage() - Sense A Particular SCSI Mode Page.			*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *		page = The mode page to sense.				*
 *		size = Size of mode sense structure.			*
 *									*
 * Returns:	Returns 0 / -1 / 1 = CD_SUCCESS / FAILURE / WARNING	*
 *									*
 ************************************************************************/
int
SensePage (ms, pcf, page, size)
register struct scsi_page_params *ms;
u_char pcf;
u_char page;
u_char size;
{
	struct mode_sel_sns_params mode_sense_params;
	struct mode_sel_sns_params *msp = &mode_sense_params;
	int status;

	BZERO ((char *) ms, size);
	BZERO ((char *) msp, sizeof(msp));
	msp->msp_addr = (caddr_t) ms;
	msp->msp_length = size;
	msp->msp_pgcode = page;
	msp->msp_pgctrl = (pcf >> PCF_SHIFT);
	status = DoIoctl (SCSI_MODE_SENSE, (char *) msp, "mode sense");
	if (status == CD_SUCCESS) {
	    struct mode_parameter_header *mph;
	    struct scsi_page_header *ph;
	    int hdr_length;
	    mph = &ms->mode_header.parameter_header;
	    hdr_length = sizeof(*mph) + mph->mph_block_desc_length;
	    ph = (struct scsi_page_header *) ((caddr_t) mph + hdr_length);
	    /*
	     * A page length of zero means that either the page does not
	     * exist, or that the page is not changeable.
	     */
	    if (ph->page_length == 0) {
		status = WARNING;
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * SelectPage() - Do Mode Select On A SCSI Page.			*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		size = Size of mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Returns:	Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SelectPage (ms, size, smp)
register struct scsi_page_params *ms;
u_char size;
u_char smp;
{
	struct mode_sel_sns_params mode_select_params;
	struct mode_sel_sns_params *msp = &mode_select_params;
	struct PageHeader *ph = (struct PageHeader *) ms;
	struct mode_parameter_header *mph = &ms->mode_header.parameter_header;
	int page = ph->page_header.page_code;
	int page_savable = ph->page_header.ps;
	int mode_length = (mph->mph_data_length + 1);
	int status;

	if ((status = SetupHeaders (ms)) != CD_SUCCESS) {
		return (status);
	}
	BZERO ((char *) msp, sizeof(msp));
	msp->msp_addr = (caddr_t) ms;
	msp->msp_length = mode_length;
	msp->msp_pgcode = page;
	msp->msp_setps = (smp && page_savable) ? TRUE : FALSE;
	return (DoIoctl (SCSI_MODE_SELECT, (char *) msp, "mode select"));
}

/************************************************************************
 *									*
 * SetupHeaders() - Setup Headers For Mode Select Command.		*
 *									*
 * Description:								*
 *	Function to setup the mode select headers.  Since we're using	*
 * the mode sense header, so certain fields must be zeroed before the	*
 * mode select is issued.						*
 *									*
 * Inputs:	ms = The Mode Select Header.				*
 *									*
 * Returns:	Returns 0 / -1 = CD_SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetupHeaders (ms)
struct PageHeader *ms;
{
	register struct mode_parameter_header *mph = &ms->parameter_header;
	register struct mode_block_descriptor *mbd = &ms->block_descriptor;
	int status = CD_SUCCESS;

	mph->mph_reserved_0 = 0;
	mph->mph_reserved_1 = 0;
	mph->mph_reserved_2 = 0;
	mbd->mbd_reserved_0 = 0;

	if (mph->mph_block_desc_length != sizeof(struct mode_block_descriptor)) {
	    mph->mph_block_desc_length = sizeof(struct mode_block_descriptor);
	}

	ms->page_header.ps = 0;		/* Reserved field for mode select. */
	ms->page_header.reserved = 0;
	return (status);
}
