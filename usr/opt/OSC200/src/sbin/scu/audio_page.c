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
static char *rcsid = "@(#)$RCSfile: audio_page.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/12/15 20:56:49 $";
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
 * File:	audio_page.c
 * Author:	Robin T. Miller
 * Date:	December 17, 1990
 *
 * Description:
 *	This file contains the functions for the audio control page.
 */

#include <sys/types.h>
#include <io/cam/rzdisk.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_pages.h"
#include "scsipages.h"

/*
 * External References:
 */
extern void FieldNotChangeable (char *str);

/*
 * Forward References:
 */
int	SetAudioPage(), ShowAudioPage(), ChangeAudioPage(),
	VerifyAudioPage(), SenseAudioPage(), SelectAudioPage();

static u_char PageCode = AUDIO_CONTROL_PAGE;

struct mode_page_funcs audio_page_funcs = {
	AUDIO_CONTROL_PAGE,		/* The mode page code.		*/
	SetAudioPage,			/* Set mode page function.	*/
	ShowAudioPage,			/* Show mode page function.	*/
	ChangeAudioPage,		/* Change mode page function.	*/
	VerifyAudioPage,		/* Verify mode page function.	*/
	SenseAudioPage,			/* Sense mode page function.	*/
	SelectAudioPage			/* Select mode page function.	*/
};

struct audio_params {				/* Audio Control page.	*/
	struct mode_page_header header;
	struct scsi_audio_control page0e;
};

static char *hdr_str =			"Audio Control";
static char *page_str =			"Page E";
static char *immed_str =		"Immediate Bit";
static char *sotc_str =			"Stop On Track Crossing";

extern char *chan0_select_str;
extern char *chan0_volume_str;
extern char *chan1_select_str;
extern char *chan1_volume_str;
extern char *chan2_select_str;
extern char *chan2_volume_str;
extern char *chan3_select_str;
extern char *chan3_volume_str;
extern char *port_select_table[];

/************************************************************************
 *									*
 * ShowAudioPage() - Show The CD-ROM Audio Control Parameters.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		ph = The page header (if any).				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
static int
ShowAudioPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct audio_params ms_audio_params;
	register struct audio_params *ms = &ms_audio_params;
	register struct scsi_audio_control *ac = &ms_audio_params.page0e;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseAudioPage (ms, PageControlField)) != 0) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    ac = (struct scsi_audio_control *) ph;
	}

	ShowPageHeader (&ac->page_header);

	PrintAscii (immed_str, yesno_table[ac->ac_immed], PNL);

	PrintAscii (sotc_str, yesno_table[ac->ac_sotc], PNL);

	PrintAscii (chan0_select_str,
			port_select_table[ac->ac_chan0_select&0x3], PNL);
	PrintNumeric (chan0_volume_str, ac->ac_chan0_volume, PNL);

	PrintAscii (chan1_select_str,
			port_select_table[ac->ac_chan1_select&0x3], PNL);
	PrintNumeric (chan1_volume_str, ac->ac_chan1_volume, PNL);

	PrintAscii (chan2_select_str,
			port_select_table[ac->ac_chan2_select&0x3], PNL);
	PrintNumeric (chan2_volume_str, ac->ac_chan2_volume, PNL);

	PrintAscii (chan3_select_str,
			port_select_table[ac->ac_chan3_select&0x3], PNL);
	PrintNumeric (chan3_volume_str, ac->ac_chan3_volume, PNL);

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetAudioPage() - Set The CD-ROM Audio Control Parameters.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		ph = The page header (if any).				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
static int
SetAudioPage (ce, ke, ph)
register struct cmd_entry *ce;
register struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct audio_params *de;
	register struct scsi_audio_control *ac;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct audio_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct audio_params *) 0) {
	    return (FAILURE);
	}
	ac = &de->page0e;

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyAudioPage (ce, ke, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	return (SelectAudioPage (de, SaveModeParameters));
}

/************************************************************************
 *									*
 * SetVolume()  Set CD-ROM Volume Control.				*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetVolume (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	register struct audio_params *ms;
	register struct scsi_audio_control *ac;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	ms = (struct audio_params *) GetDefaultPage (PageCode, sizeof(*ms));
	if (ms == (struct audio_params *) 0) {
	    return (FAILURE);
	}
	ac = &ms->page0e;

	/*
	 * Get the current mode page values first.
	 */
	if ((status = SenseAudioPage (ms, PCF_CURRENT)) != SUCCESS) {
	    return (status);
	}

	if (ISSET_KOPT (ce, K_CHAN0_SELECT)) {
	    ac->ac_chan0_select = Channel_0_Select;
	}

	if (ISSET_KOPT (ce, K_CHAN0_VOLUME)) {
	    ac->ac_chan0_volume = Channel_0_Volume;
	}

	if (ISSET_KOPT (ce, K_CHAN1_SELECT)) {
	    ac->ac_chan1_select = Channel_1_Select;
	}

	if (ISSET_KOPT (ce, K_CHAN1_VOLUME)) {
	    ac->ac_chan1_volume = Channel_1_Volume;
	}

	if (ISSET_KOPT (ce, K_VOLUME_LEVEL)) {
	    ac->ac_chan0_volume = VolumeLevel;
	    ac->ac_chan1_volume = VolumeLevel;
	}

	return (SelectAudioPage (ms, SaveModeParameters));
}

/************************************************************************
 *									*
 * ChangeAudioPage() - Change The CD-ROM Audio Control Parameters.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		ph = The page header (if any).				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
static int
ChangeAudioPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct audio_params ch_audio_params;
	struct audio_params ms_audio_params;
	register struct audio_params *ch = &ch_audio_params;
	register struct audio_params *de;
	register struct audio_params *ms = &ms_audio_params;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct audio_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct audio_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseAudioPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Use user selected pcf, unless set to changeable parameters.
	 */
	if ((pcf = PageControlField) == PCF_CHANGEABLE) {
	    pcf = PCF_SAVED;
	}

	/*
	 * Get the page parameters for defaults to prompts.
	 */
	if ((status = SenseAudioPage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page0e, &ms->page0e)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	chbits = (u_long) ch->page0e.ac_immed;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_immed & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, immed_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_immed = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (immed_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_sotc;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_sotc & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, sotc_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_sotc = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (sotc_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_chan0_select;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_chan0_select & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, chan0_select_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_chan0_select = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (chan0_select_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_chan0_volume;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_chan0_volume & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, chan0_volume_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_chan0_volume = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (chan0_volume_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_chan1_select;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_chan1_select & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, chan1_select_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_chan1_select = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (chan1_select_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_chan1_volume;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_chan1_volume & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, chan1_volume_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_chan1_volume = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (chan1_volume_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_chan2_select;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_chan2_select & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, chan2_select_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_chan2_select = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (chan2_select_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_chan2_volume;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_chan2_volume & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, chan2_volume_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_chan2_volume = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (chan2_volume_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_chan3_select;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_chan3_select & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, chan3_select_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_chan3_select = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (chan3_select_str);
	    }
	}

	chbits = (u_long) ch->page0e.ac_chan3_volume;
	if (chbits) {
	    deflt = (u_long) (ms->page0e.ac_chan3_volume & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, chan3_volume_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0e.ac_chan3_volume = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (chan3_volume_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectAudioPage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyAudioPage() - Verify The CD-ROM Audio Control Parameters.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		se = The mode page to set (if any).			*
 *		verify_verbose = Display verification messages.		*
 *									*
 * Implicit Outputs:							*
 *		Default page parameters are setup.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
static int
VerifyAudioPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_audio_control *se;
int verify_verbose;
{
	struct audio_params ch_audio_params;
	struct audio_params vd_audio_params;
	register struct audio_params *ch = &ch_audio_params;
	register struct audio_params *de;
	register struct audio_params *vd = &vd_audio_params;
	int status, field_changed;
	u_long ch_field;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct audio_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct audio_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseAudioPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Use user selected pcf, unless set to changeable parameters.
	 */
	if ((pcf = VerifyPcf) == PCF_CHANGEABLE) {
	    pcf = PCF_CURRENT;
	}

	/*
	 * Get the default page parameters (unless already done).
	 */
	if (de->page0e.page_header.page_code != PageCode) {
	    if ((status = SenseAudioPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_audio_control *) 0) {
	    register struct scsi_audio_control *ac = &de->page0e;

	    *ac = *se;
	}

	if ((status = SetupChangeable (&ch->page0e, &de->page0e)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Get the vendor default values for sanity checks.
	 */
	if ((status = SenseAudioPage (vd, PCF_DEFAULT)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Verify the page parameters:
	 *
	 *			+---------------+  No	+---------------+
	 *   |----------------->| More Fields?  |------>|    Finished   |
	 *   |			+---------------+	+---------------+
	 *   |			       | Yes
	 *   |			       v
	 *   |	  		+---------------+
	 *   |		+-------|  Changeable?	|-------+
	 *   |		|  No	+---------------+  Yes	|
	 *   |		|				|
	 *   |		v				v
	 *   |   +--------------+	    No	+---------------+
	 *   |	 | Set To Zero	|	+-------|   Specified?	|
	 *   |	 +--------------+	|	+---------------+
	 *   |		|		|		|
	 *   |<---------+		|		|
	 *   |				|		v
	 *   |	 +--------------+	v   No	+---------------+
	 *   |	 | Set Default	|<--------------| Within Range?	|
	 *   |	 |      or	|		+---------------+
	 *   |	 | Use Default	|			|
	 *   |	 +--------------+			| Yes
	 *   |		|				|
	 *   |		v				v
	 *   |	 +--------------+		+---------------+
	 *   |	 | If verbose,	|		| Set New Value	|
	 *   |	 |   tell user.	|		+---------------+
	 *   |	 +--------------+			|
	 *   |		|				|
	 *   |		v				v
	 *   |<-----------------------------------------+
	 *
	 */

	if (verify_verbose) {
	    ShowParamHeader (verifying_str, hdr_str, page_str, pcf);
	}

	/*
	 * Verify "Immediate Bit" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_immed;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					immed_str, de->page0e.ac_immed);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (immed_str);
	    }
	}

	/*
	 * Verify "Stop On Track Crossing Bit" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_sotc;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					sotc_str, de->page0e.ac_sotc);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (sotc_str);
	    }
	}

	/*
	 * Verify "Channel 0 Output Selection" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_chan0_select;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				chan0_select_str, de->page0e.ac_chan0_select);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (chan0_select_str);
	    }
	}

	/*
	 * Verify "Channel 0 Volume Level" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_chan0_volume;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				chan0_volume_str, de->page0e.ac_chan0_volume);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (chan0_volume_str);
	    }
	}

	/*
	 * Verify "Channel 1 Output Selection" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_chan1_select;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				chan1_select_str, de->page0e.ac_chan1_select);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (chan1_select_str);
	    }
	}

	/*
	 * Verify "Channel 1 Volume Level" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_chan1_volume;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				chan1_volume_str, de->page0e.ac_chan1_volume);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (chan1_volume_str);
	    }
	}

	/*
	 * Verify "Channel 2 Output Selection" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_chan2_select;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				chan2_select_str, de->page0e.ac_chan2_select);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (chan2_select_str);
	    }
	}

	/*
	 * Verify "Channel 2 Volume Level" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_chan2_volume;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				chan2_volume_str, de->page0e.ac_chan2_volume);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (chan2_volume_str);
	    }
	}

	/*
	 * Verify "Channel 3 Output Selection" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_chan3_select;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				chan3_select_str, de->page0e.ac_chan3_select);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (chan3_select_str);
	    }
	}

	/*
	 * Verify "Channel 3 Volume Level" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page0e.ac_chan3_volume;
	if (ch_field) {
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				chan3_volume_str, de->page0e.ac_chan3_volume);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (chan3_volume_str);
	    }
	}

	return (SUCCESS);
}

/************************************************************************
 *									*
 * SenseAudioPage() - Sense The CD-ROM Audio Control Page.		*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
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
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
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
