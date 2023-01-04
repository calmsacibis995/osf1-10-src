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
static char *rcsid = "@(#)$RCSfile: direct_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:09:26 $";
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
 * File:	direct_page.c
 * Author:	Robin T. Miller
 * Date:	December 19, 1990
 *
 * Description:
 *	This file contains the functions for the direct-access device
 * format page.
 */
#include <stdio.h>
#include <sys/types.h>
#include <io/cam/rzdisk.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"
#include "scu_pages.h"
#include "scsipages.h"

/*
 * External References:
 */
extern void FieldNotChangeable (char *str);
extern int IS_Mounted (struct scu_device *scu);

/*
 * Forward References:
 */
int	SetDirectPage(), ShowDirectPage(), ChangeDirectPage(),
	VerifyDirectPage(), SenseDirectPage(), SelectDirectPage();

static u_char PageCode = DIRECT_ACCESS_PAGE;

struct mode_page_funcs direct_page_funcs = {
	DIRECT_ACCESS_PAGE,		/* The mode page code.		*/
	SetDirectPage,			/* Set mode page function.	*/
	ShowDirectPage,			/* Show mode page function.	*/
	ChangeDirectPage,		/* Change mode page function.	*/
	VerifyDirectPage,		/* Verify mode page function.	*/
	SenseDirectPage,		/* Sense mode page function.	*/
	SelectDirectPage		/* Select mode page function.	*/
};

struct direct_params {				/* Direct Access Format	*/
	struct mode_page_header header;
	struct scsi_direct_access page3;
};

static char *hdr_str =			"Direct-Access Device Format";
static char *page_str =			"Page 3";
char *tracks_per_zone_str =		"Tracks per Zone";
char *alt_sectors_zone_str =		"Alternate Sectors per Zone";
static char *alt_tracks_zone_str =	"Alternate Tracks per Zone";
char *alt_tracks_vol_str =		"Alternate Tracks per Volume";
static char *sectors_track_str =	"Sectors per Track";
static char *data_sector_str =		"Data Bytes per Physical Sector";
static char *interleave_str =		"Interleave";
static char *track_skew_str =		"Track Skew";
static char *cylinder_skew_str =	"Cylinder Skew";
static char *surface_addressing_str =	"Surface Addressing";
static char *removable_media_str =	"Removable Media";
static char *hard_sector_format_str =	"Hard Sector Format";
static char *soft_sector_format_str =	"Soft Sector Format";

#ifdef notdef
static char *warn_sectors_track_str =
	"Specified sectors/track not equal to vendor supplied value.\n";

/*
 * Table of different sparring modes.
 */
static char *sparring_list[] = {
	"cylinder",
	"track",
	"drive",
	"host",
	NULL,
};
#endif notdef

/************************************************************************
 *									*
 * ShowDirectPage() - Show The Direct-Access Device Format Page.	*
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
ShowDirectPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct direct_params ms_direct_params;
	register struct direct_params *ms = &ms_direct_params;
	register struct scsi_direct_access *da = &ms_direct_params.page3;
	u_short tmp;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseDirectPage (ms, PageControlField)) != SUCCESS) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    da = (struct scsi_direct_access *) ph;
	}

	ShowPageHeader (&da->page_header);

	tmp = (u_short) ( (da->da_tracks_per_zone_1 << 8) +
			  (da->da_tracks_per_zone_0) );
	PrintNumeric (tracks_per_zone_str, tmp, PNL);

	tmp = (u_short) ( (da->da_alt_sectors_zone_1 << 8) +
			  (da->da_alt_sectors_zone_0) );
	PrintNumeric (alt_sectors_zone_str, tmp, PNL);

	tmp = (u_short) ( (da->da_alt_tracks_zone_1 << 8) +
			  (da->da_alt_tracks_zone_0) );
	PrintNumeric (alt_tracks_zone_str, tmp, PNL);

	tmp = (u_short) ( (da->da_alt_tracks_vol_1 << 8) +
			  (da->da_alt_tracks_vol_0) );
	PrintNumeric (alt_tracks_vol_str, tmp, PNL);

	tmp = (u_short) ( (da->da_sectors_track_1 << 8) +
			  (da->da_sectors_track_0) );
	PrintNumeric (sectors_track_str, tmp, PNL);

	tmp = (u_short) ( (da->da_data_sector_1 << 8) +
			  (da->da_data_sector_0) );
	PrintNumeric (data_sector_str, tmp, PNL);

	tmp = (u_short) ( (da->da_interleave_1 << 8) +
			  (da->da_interleave_0) );
	PrintNumeric (interleave_str, tmp, PNL);

	tmp = (u_short) ( (da->da_track_skew_1 << 8) +
			  (da->da_track_skew_0) );
	PrintNumeric (track_skew_str, tmp, PNL);

	tmp = (u_short) ( (da->da_cylinder_skew_1 << 8) +
			  (da->da_cylinder_skew_0) );
	PrintNumeric (cylinder_skew_str, tmp, PNL);

	PrintNumeric (surface_addressing_str, da->da_surf, PNL);

	PrintAscii (removable_media_str,
			yesno_table[da->da_rmb], PNL);

	/*
	 * Since the drive should only be setup for hard or soft sector
	 * format, we only display the message if the mode is enabled.
	 */
	if (da->da_hsec) {
	    PrintAscii (hard_sector_format_str,
			yesno_table[da->da_hsec], PNL);
	}

	if (da->da_ssec) {
	    PrintAscii (soft_sector_format_str,
			yesno_table[da->da_ssec], PNL);
	}

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetDirectPage() - Set The Direct-Access Device Format Page.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		ph = The page header (if any).				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SetDirectPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct direct_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct direct_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct direct_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyDirectPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	if ((status = SelectDirectPage (de, SaveModeParameters)) != SUCCESS) {
	    return (status);
	}
	if (DisplayVerbose && (PageControlField == PCF_SAVED) && !Formatting) {
	    Printf ("Parameters are only saved after a FORMAT UNIT command.\n");
	}
	return (status);
}

/************************************************************************
 *									*
 * ChangeDirectPage() - Change The Direct-Access Device Format Page.	*
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
ChangeDirectPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct scu_device *scu = ScuDevice;
	struct direct_params ch_direct_params;
	struct direct_params ms_direct_params;
	register struct direct_params *ch = &ch_direct_params;
	register struct direct_params *de;
	register struct direct_params *ms = &ms_direct_params;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct direct_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct direct_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Don't permit changing this page if a file system is mounted.
	 */
	if ((status = IS_Mounted (scu)) != FALSE) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseDirectPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseDirectPage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page3, &ms->page3)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	chbits = (u_long) ( (ch->page3.da_tracks_per_zone_1 << 8) +
			    (ch->page3.da_tracks_per_zone_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_tracks_per_zone_1 << 8) +
				(ms->page3.da_tracks_per_zone_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			tracks_per_zone_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_tracks_per_zone_1 = (u_char) (answer >> 8);
	    ms->page3.da_tracks_per_zone_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (tracks_per_zone_str);
	    }
	}

	chbits = (u_long) ( (ch->page3.da_alt_sectors_zone_1 << 8) +
			    (ch->page3.da_alt_sectors_zone_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_alt_sectors_zone_1 << 8) +
				(ms->page3.da_alt_sectors_zone_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			alt_sectors_zone_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_alt_sectors_zone_1 = (u_char) (answer >> 8);
	    ms->page3.da_alt_sectors_zone_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (alt_sectors_zone_str);
	    }
	}

	chbits = (u_long) ( (ch->page3.da_alt_tracks_zone_1 << 8) +
			    (ch->page3.da_alt_tracks_zone_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_alt_tracks_zone_1 << 8) +
				(ms->page3.da_alt_tracks_zone_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			alt_tracks_zone_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_alt_tracks_zone_1 = (u_char) (answer >> 8);
	    ms->page3.da_alt_tracks_zone_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (alt_tracks_zone_str);
	    }
	}

	chbits = (u_long) ( (ch->page3.da_alt_tracks_vol_1 << 8) +
			    (ch->page3.da_alt_tracks_vol_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_alt_tracks_vol_1 << 8) +
				(ms->page3.da_alt_tracks_vol_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			alt_tracks_vol_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_alt_tracks_vol_1 = (u_char) (answer >> 8);
	    ms->page3.da_alt_tracks_vol_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (alt_tracks_vol_str);
	    }
	}

	chbits = (u_long) ( (ch->page3.da_sectors_track_1 << 8) +
			    (ch->page3.da_sectors_track_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_sectors_track_1 << 8) +
				(ms->page3.da_sectors_track_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			sectors_track_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_sectors_track_1 = (u_char) (answer >> 8);
	    ms->page3.da_sectors_track_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (sectors_track_str);
	    }
	}

	chbits = (u_long) ( (ch->page3.da_data_sector_1 << 8) +
			    (ch->page3.da_data_sector_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_data_sector_1 << 8) +
				(ms->page3.da_data_sector_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			data_sector_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_data_sector_1 = (u_char) (answer >> 8);
	    ms->page3.da_data_sector_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (data_sector_str);
	    }
	}

	chbits = (u_long) ( (ch->page3.da_interleave_1 << 8) +
			    (ch->page3.da_interleave_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_interleave_1 << 8) +
				(ms->page3.da_interleave_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			interleave_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_interleave_1 = (u_char) (answer >> 8);
	    ms->page3.da_interleave_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (interleave_str);
	    }
	}

	chbits = (u_long) ( (ch->page3.da_track_skew_1 << 8) +
			    (ch->page3.da_track_skew_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_track_skew_1 << 8) +
				(ms->page3.da_track_skew_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			track_skew_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_track_skew_1 = (u_char) (answer >> 8);
	    ms->page3.da_track_skew_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (track_skew_str);
	    }
	}

	chbits = (u_long) ( (ch->page3.da_cylinder_skew_1 << 8) +
			    (ch->page3.da_cylinder_skew_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page3.da_cylinder_skew_1 << 8) +
				(ms->page3.da_cylinder_skew_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			cylinder_skew_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_cylinder_skew_1 = (u_char) (answer >> 8);
	    ms->page3.da_cylinder_skew_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (cylinder_skew_str);
	    }
	}

	chbits = (u_long) ch->page3.da_surf;
	if (chbits) {
	    deflt = (u_long) (ms->page3.da_surf & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, surface_addressing_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_surf = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (surface_addressing_str);
	    }
	}

	chbits = (u_long) ch->page3.da_hsec;
	if (chbits) {
	    deflt = (u_long) (ms->page3.da_hsec & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, hard_sector_format_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_hsec = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (hard_sector_format_str);
	    }
	}

	chbits = (u_long) ch->page3.da_ssec;
	if (chbits) {
	    deflt = (u_long) (ms->page3.da_ssec & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, soft_sector_format_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page3.da_ssec = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (soft_sector_format_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectDirectPage (ms, SaveModeParameters)) == SUCCESS) {
	    *de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyDirectPage() - Verify The Direct-Access Device Format Page.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		se = The mode page to set (if any).			*
 *		verify_verbose = Display verification messages.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
static int
VerifyDirectPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_direct_access *se;
int verify_verbose;
{
	struct scu_device *scu = ScuDevice;
	struct direct_params ch_direct_params;
	struct direct_params vd_direct_params;
	register struct direct_params *ch = &ch_direct_params;
	register struct direct_params *de;
	register struct direct_params *vd = &vd_direct_params;
	int status;
	u_char pcf;

	/*
	 * Don't permit changing this page if a file system is mounted.
	 */
	if ((status = IS_Mounted (scu)) != FALSE) {
	    return (FAILURE);
	}

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct direct_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct direct_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseDirectPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Use user selected pcf, unless set to changeable parameters.
	 */
	if ((pcf = VerifyPcf) == PCF_CHANGEABLE) {
	    pcf = PCF_DEFAULT;
	}

	/*
	 * Get the default page parameters (unless already done).
	 */
	if (de->page3.page_header.page_code != PageCode) {
	    if ((status = SenseDirectPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_direct_access *) 0) {
	    register struct scsi_direct_access *da = &de->page3;

	    *da = *se;
	}

	if ((status = SetupChangeable (&ch->page3, &de->page3)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Get the vendor default values for sanity checks.
	 */
	if ((status = SenseDirectPage (vd, PCF_DEFAULT)) != SUCCESS) {
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
#ifdef notdef
	if (verify_verbose) {
	    ShowParamHeader (verifying_str, hdr_str, page_str, pcf);
	}

	/*
	 * Verify "Tracks per Zone" parameter.
	 */
	field_changed = 0;
	ch_field = (u_long) ( (ch->page3.da_tracks_per_zone_1 << 8) +
			      (ch->page3.da_tracks_per_zone_0) );
	if (ch_field) {
	    se_field = (u_long) ( ((se->da_tracks_per_zone_1 << 8) +
				   (se->da_tracks_per_zone_0)) );
	    if (se_field <= ch_field) {
		field_changed = 1;
		de->page3.da_tracks_per_zone_1 = (u_char) (se_field >> 8);
		de->page3.da_tracks_per_zone_0 = (u_char) se_field;
	    } else {
		if (verify_verbose) {
		    Printf ("%s %s\n", tracks_per_zone_str, too_large_str);
		}
	    }
	    if (verify_verbose) {
		de_field = (u_long) ( ( (de->page3.da_tracks_per_zone_1 << 8) +
					(de->page3.da_tracks_per_zone_0)) );
		Printf ( (field_changed) ? setting_str : leaving_str,
					tracks_per_zone_str, de_field);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (tracks_per_zone_str);
	    }
	}

	/*
	 * Verify "Alternate Sectors per Zone" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page3.da_alt_sectors_zone);
	if (ch_field) {
	    if (dt->dtype_options & SUP_ASECT) {
		if (dt->dtype_asect <= ch_field) {
		    field_changed = 1;
		    de->page3.da_alt_sectors_zone = htoccs2(dt->dtype_asect);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", alt_sectors_zone_str, too_large_str);
		    }
		}
	    }
	    if (!field_changed) {
		field_changed = 1;
		de->page3.da_alt_sectors_zone = htoccs2(DEF_ALT_SECTORS_ZONE);
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					alt_sectors_zone_str,
					ccstoh2(de->page3.da_alt_sectors_zone));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (alt_sectors_zone_str);
	    }
	}

	/*
	 * Verify "Alternate Tracks per Zone" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page3.da_alt_tracks_zone);
	if (ch_field) {
	    if (dt->dtype_options & SUP_ATRKS_ZONE) {
		if (dt->dtype_atrks_zone <= ch_field) {
		    field_changed = 1;
		    de->page3.da_alt_tracks_zone = htoccs2(dt->dtype_atrks_zone);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", alt_tracks_zone_str, too_large_str);
		    }
		}
	    }
	    /*
	     * Field not specified, set default or use vendors' default.
	     */
	    if (ccstoh2(de->page3.da_alt_tracks_zone) != 0) { /* Expect zero */
		de->page3.da_alt_tracks_zone = htoccs2(DEF_ALT_TRACKS_ZONE);
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					alt_tracks_zone_str,
					ccstoh2(de->page3.da_alt_tracks_zone));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (alt_tracks_zone_str);
	    }
	}

	/*
	 * Verify "Alternate Tracks per Volume" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page3.da_alt_tracks_vol);
	if (ch_field) {
	    if (dt->dtype_options & SUP_ATRKS) {
		if (dt->dtype_atrks <= ch_field) {
		    field_changed = 1;
		    de->page3.da_alt_tracks_vol = htoccs2(dt->dtype_atrks);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", alt_tracks_vol_str, too_large_str);
		    }
		}
	    }
	    if (!field_changed) {
		field_changed = 1;
		de->page3.da_alt_tracks_vol = htoccs2(DEF_ALT_TRACKS_VOLUME);
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					alt_tracks_vol_str,
					ccstoh2(de->page3.da_alt_tracks_vol));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (alt_tracks_vol_str);
	    }
	}

	/*
	 * Determine the sparring method being used to decide on how many
	 * sectors/track there should be.  Although there are 4 types of
	 * sparring, only cylinder oriented and track oriented sparring
	 * are expected.
	 *
	 * Sparring	  Tracks/	Alt. Sectors	Alt. Tracks
	 *   Mode	   Zone		  Per Zone	 Per Volume
	 * --------	  -------	------------	-----------
	 * Cylinder	 # of heads	     >3		 1-3 Cyls.
	 *  Track	     1		     1		 1-3 Cyls.
	 *  Drive	     1		     0		 1-3 Cyls.
	 *  Host	     1		     0		     0
	 *
	 * The alternate tracks per volume typically depends on the size
	 * of the disk.  On smaller drives such as the CDC Wren IV, this
	 * value is normally (2 * # heads = 18 tracks).  On larger drives
	 * such as the Maxtor-8760S and Micropols 1588/1598, this value
	 * is normally (3 * # heads = 45 tracks).
	 *
	 * Note:  The number of sectors specified in the format data
	 *	  file doesn't include the spare sector count.  This
	 *	  sector count gets used in logical block number (LBN)
	 *	  and maximum size calculations by system utilities.
	 *	  I'd like to change this but... history is in place.
	 */
	if (ccstoh2(de->page3.da_track_per_zone) == cur_dtype->dtype_nhead) {
	    sparring_method = CYLINDER_SPARRING;
	    sectors_per_track = dt->dtype_nsect;
	} else {
	    sparring_method = TRACK_SPARRING;
	    sectors_per_track = dt->dtype_nsect +
					ccstoh2(de->page3.da_track_per_zone);
	}
	if (verify_verbose) {
	    Printf ("Using %s oriented sparring mode.\n",
					sparring_list[sparring_method]);
	}

	/*
	 * Verify "Sectors per Track" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page3.da_sectors_track);
	if (ch_field) {
	    if (dt->dtype_options & SUP_NSECT) {
		if (sectors_per_track <= ch_field) {
		    field_changed = 1;
		    /*
		     * Critical parameter, do sanity checking.
		     */
		    if (sectors_per_track != ccstoh2(de->page3.da_sectors_track)) {
			if (verify_verbose) Printf (warn_sectors_track_str);
			if (sectors_per_track > ccstoh2(de->page3.da_sectors_track)) {
			    status = FAILURE;
			}
		    }
		    de->page3.da_sectors_track = htoccs2(sectors_per_track);
		} else {
		    status = FAILURE;		/* Critical parameter. */
		    Printf ("%s %s\n", sectors_track_str, too_large_str);
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				sectors_track_str,
				ccstoh2(de->page3.da_sectors_track));
	    }
	} else {
	    /*
	     * Critical parameter, do sanity checking.
	     */
	    if (sectors_per_track != ccstoh2(vd->page3.da_sectors_track)) {
		if (verify_verbose) Printf (warn_sectors_track_str);
		if (sectors_per_track > ccstoh2(vd->page3.da_sectors_track)) {
		    status = FAILURE;
		}
	    }
	    de->page3.da_sectors_track = 0;
	    if (verify_verbose) {
		FieldNotChangeable (sectors_track_str);
	    }
	}

	/*
	 * Verify "Data Bytes per Physical Sector" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page3.da_data_sector);
	if (ch_field) {
	    if (dt->dtype_options & SUP_DATA_SECT) {
		if (dt->dtype_data_sector <= ch_field) {
		    field_changed = 1;
		    de->page3.da_data_sector = htoccs2(dt->dtype_data_sector);
		} else {
		    status = FAILURE;		/* Critical parameter. */
		    Printf ("%s %s\n", data_sector_str, too_large_str);
		}
	    }
	    /*
	     * Only one sector size currently supported.
	     */
	    if (!field_changed ||
		(ccstoh2(de->page3.da_data_sector) != DEF_DATA_SECTOR) ) {
		field_changed = 1;
		de->page3.da_data_sector = htoccs2(DEF_DATA_SECTOR);
		}
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					data_sector_str,
					ccstoh2(de->page3.da_data_sector));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (data_sector_str);
	    }
	}

	/*
	 * Verify "Interleave" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page3.da_interleave);
	if (ch_field) {
	    if (dt->dtype_options & SUP_DATA_SECT) {
		if (dt->dtype_interleave <= ch_field) {
		    field_changed = 1;
		    de->page3.da_interleave = htoccs2(dt->dtype_interleave);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", interleave_str, too_large_str);
		    }
		}
	    }
	    if (!field_changed) {
		field_changed = 1;
		de->page3.da_interleave = htoccs2(DEF_INTERLEAVE);
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					interleave_str,
					ccstoh2(de->page3.da_interleave));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (interleave_str);
	    }
	}

	/*
	 * Verify "Track Skew" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page3.da_track_skew);
	if (ch_field) {
	    if (dt->dtype_options & SUP_TRACK_SKEW) {
		if (dt->dtype_track_skew <= ch_field) {
		    field_changed = 1;
		    de->page3.da_track_skew = htoccs2(dt->dtype_track_skew);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", track_skew_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				track_skew_str,
				ccstoh2(de->page3.da_track_skew));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (track_skew_str);
	    }
	}

	/*
	 * Verify "Cylinder Skew" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page3.da_cylinder_skew);
	if (ch_field) {
	    if (dt->dtype_options & SUP_CYLINDER_SKEW) {
		if (dt->dtype_cylinder_skew <= ch_field) {
		    field_changed = 1;
		    de->page3.da_cylinder_skew = htoccs2(dt->dtype_cylinder_skew);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", cylinder_skew_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
				cylinder_skew_str,
				ccstoh2(de->page3.da_cylinder_skew));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (cylinder_skew_str);
	    }
	}
#endif notdef
	return (status);
}

/************************************************************************
 *									*
 * SenseDirectPage() - Sense The Direct-Access Device Format Page.	*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseDirectPage (ms, pcf)
struct direct_params *ms;
u_char pcf;
{
	return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectDirectPage() - Select The Direct-Access Device Format Page.	*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectDirectPage (ms, smp)
struct direct_params *ms;
u_char smp;
{
	struct direct_params se_direct_params;
	register struct direct_params *se = &se_direct_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
