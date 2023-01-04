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
static char *rcsid = "@(#)$RCSfile: flexible_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:14:22 $";
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
 * File:	flexible_page.c
 * Author:	Robin T. Miller
 * Date:	January 16, 1991
 *
 * Description:
 *	This file contains the functions for the flexible disk page.
 */

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
int	SetFlexiblePage(), ShowFlexiblePage(), ChangeFlexiblePage(),
	VerifyFlexiblePage(), SenseFlexiblePage(), SelectFlexiblePage();

static u_char PageCode = FLEXIBLE_DISK_PAGE;

struct mode_page_funcs flexible_page_funcs = {
	FLEXIBLE_DISK_PAGE,		/* The mode page code.		*/
	SetFlexiblePage,		/* Set mode page function.	*/
	ShowFlexiblePage,		/* Show mode page function.	*/
	ChangeFlexiblePage,		/* Change mode page function.	*/
	VerifyFlexiblePage,		/* Verify mode page function.	*/
	SenseFlexiblePage,		/* Sense mode page function.	*/
	SelectFlexiblePage		/* Select mode page function.	*/
};

struct flexible_params {			/* Flexible Disk Page.	*/
	struct mode_page_header header;
	struct scsi_flexible_disk page5;
};

static char *hdr_str =			"Flexible Disk";
static char *page_str =			"Page 5";
static char *transfer_rate_str =	"Transfer Rate";
static char *heads_str =		"Number of Heads";
static char *sectors_track_str =	"Sectors per Track";
static char *cylinders_str =		"Number of Cylinders";
static char *data_sector_str =		"Data Bytes per Physical Sector";
static char *precomp_cyl_str =		"Write Precomp Starting Cylinder";
static char *current_cyl_str =		"Reduced Write Current Cylinder";
static char *step_rate_str =		"Drive Step Rate";
static char *step_pulse_width_str =	"Drive Step Pulse Width";
static char *head_settle_delay_str =	"Head Settle Delay";
static char *motor_on_delay_str =	"Motor On Delay";
static char *motor_off_delay_str =	"Motor Off Delay";
static char *mo_str =			"Motor On Bit (MO)";
static char *ssn_str =			"Starting Sector Number (SSN)";
static char *trdy_str =			"True Ready Indicator (TRDY)";
static char *spc_str =			"Step Pulses Per Cylinder (SPC)";
static char *write_compensation_str =	"Write Compensation";
static char *head_load_delay_str =	"Head Load Delay";
static char *head_unload_delay_str =	"Head Unload Delay";
static char *pin_1_str =		"Pin 1 Definition";
static char *pin_2_str =		"Pin 2 Definition";
static char *pin_4_str =		"Pin 4 Definition";
static char *pin_34_str =		"Pin 34 Definition";
static char *rotate_rate_str =		"Medium Rotate Rate";

/************************************************************************
 *									*
 * ShowFlexiblePage() - Show The Flexible Disk Page.			*
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
ShowFlexiblePage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct flexible_params ms_flexible_params;
	register struct flexible_params *ms = &ms_flexible_params;
	register struct scsi_flexible_disk *fd = &ms_flexible_params.page5;
	u_long tmp;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseFlexiblePage (ms, PageControlField)) != 0) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    fd = (struct scsi_flexible_disk *) ph;
	}

	ShowPageHeader (&fd->page_header);

	tmp = (u_long) ( (fd->fd_transfer_rate_1 << 8) +
			 (fd->fd_transfer_rate_0) );
	PrintNumeric (transfer_rate_str, tmp, PNL);

	PrintNumeric (heads_str, fd->fd_heads, PNL);

	PrintNumeric (sectors_track_str, fd->fd_sectors_track, PNL);

	tmp = (u_long) ( (fd->fd_cylinders_1 << 8) +
			 (fd->fd_cylinders_0) );
	PrintNumeric (cylinders_str, tmp, PNL);

	tmp = (u_long) ( (fd->fd_data_sector_1 << 8) +
			 (fd->fd_data_sector_0) );
	PrintNumeric (data_sector_str, tmp, PNL);

	tmp = (u_long) ( (fd->fd_precomp_cyl_1 << 8) +
			 (fd->fd_precomp_cyl_0) );
	PrintNumeric (precomp_cyl_str, tmp, PNL);

	tmp = (u_long) ( (fd->fd_current_cyl_1 << 8) +
			 (fd->fd_current_cyl_0) );
	PrintNumeric (current_cyl_str, tmp, PNL);

	tmp = (u_long) ( (fd->fd_step_rate_1 << 8) +
			 (fd->fd_step_rate_0) );
	PrintNumeric (step_rate_str, tmp, PNL);

	PrintNumeric (step_pulse_width_str, fd->fd_step_pulse_width, PNL);

	tmp = (u_long) ( (fd->fd_head_settle_delay_1 << 8) +
			 (fd->fd_head_settle_delay_0) );
	PrintNumeric (head_settle_delay_str, tmp, PNL);

	PrintNumeric (motor_on_delay_str, fd->fd_motor_on_delay, PNL);

	PrintNumeric (motor_off_delay_str, fd->fd_motor_off_delay, PNL);

	PrintNumeric (mo_str, fd->fd_mo, PNL);

	PrintNumeric (ssn_str, fd->fd_ssn, PNL);

	PrintNumeric (trdy_str, fd->fd_trdy, PNL);

	PrintNumeric (spc_str, fd->fd_spc, PNL);

	PrintNumeric (write_compensation_str, fd->fd_write_compensation, PNL);

	PrintNumeric (head_load_delay_str, fd->fd_head_load_delay, PNL);

	PrintNumeric (head_unload_delay_str, fd->fd_head_unload_delay, PNL);

	PrintNumeric (pin_1_str, fd->fd_pin_1, PNL);

	PrintNumeric (pin_2_str, fd->fd_pin_2, PNL);

	PrintNumeric (pin_4_str, fd->fd_pin_4, PNL);

	PrintNumeric (pin_34_str, fd->fd_pin_34, PNL);

	tmp = (u_long) ( (fd->fd_rotate_rate_1 << 8) +
			 (fd->fd_rotate_rate_0) );
	PrintNumeric (rotate_rate_str, tmp, PNL);

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetFlexiblePage() - Set The Flexible Disk Page.			*
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
SetFlexiblePage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct flexible_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct flexible_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct flexible_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyFlexiblePage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters Page (%s)\n", hdr_str, page_str);
	}
	if ((status = SelectFlexiblePage (de, SaveModeParameters)) != SUCCESS) {
	    return (status);
	}
	if (DisplayVerbose && (PageControlField == PCF_SAVED) && !Formatting) {
	    Printf ("Parameters are only saved after a FORMAT UNIT command.\n");
	}
	return (status);
}

/************************************************************************
 *									*
 * ChangeFlexiblePage() - Change The Flexible Disk Page.		*
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
ChangeFlexiblePage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct scu_device *scu = ScuDevice;
	struct flexible_params ch_flexible_params;
	struct flexible_params ms_flexible_params;
	register struct flexible_params *ch = &ch_flexible_params;
	register struct flexible_params *de;
	register struct flexible_params *ms = &ms_flexible_params;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	u_long answer, chbits, deflt;
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
	de = (struct flexible_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct flexible_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseFlexiblePage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseFlexiblePage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	(void) SetupChangeable (&ch->page5, &ms->page5);

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	chbits = (u_long) ( (ch->page5.fd_transfer_rate_1 << 8) +
			    (ch->page5.fd_transfer_rate_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page5.fd_transfer_rate_1 << 8) +
				(ms->page5.fd_transfer_rate_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			transfer_rate_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_transfer_rate_1 = (u_char) (answer >> 8);
	    ms->page5.fd_transfer_rate_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (transfer_rate_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_heads;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_heads & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, heads_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_heads = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (heads_str);
	    }
	}


	chbits = (u_long) ch->page5.fd_sectors_track;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_sectors_track & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, sectors_track_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_sectors_track = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (sectors_track_str);
	    }
	}

	chbits = (u_long) ( (ch->page5.fd_data_sector_1 << 8) +
			    (ch->page5.fd_data_sector_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page5.fd_data_sector_1 << 8) +
				(ms->page5.fd_data_sector_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			data_sector_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_data_sector_1 = (u_char) (answer >> 8);
	    ms->page5.fd_data_sector_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (data_sector_str);
	    }
	}

	chbits = (u_long) ( (ch->page5.fd_cylinders_1 << 8) +
			    (ch->page5.fd_cylinders_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page5.fd_cylinders_1 << 8) +
				(ms->page5.fd_cylinders_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			cylinders_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_cylinders_1 = (u_char) (answer >> 8);
	    ms->page5.fd_cylinders_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (cylinders_str);
	    }
	}

	chbits = (u_long) ( (ch->page5.fd_precomp_cyl_1 << 8) +
			    (ch->page5.fd_precomp_cyl_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page5.fd_precomp_cyl_1 << 8) +
				(ms->page5.fd_precomp_cyl_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			precomp_cyl_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_precomp_cyl_1 = (u_char) (answer >> 8);
	    ms->page5.fd_precomp_cyl_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (precomp_cyl_str);
	    }
	}

	chbits = (u_long) ( (ch->page5.fd_current_cyl_1 << 8) +
			    (ch->page5.fd_current_cyl_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page5.fd_current_cyl_1 << 8) +
				(ms->page5.fd_current_cyl_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			current_cyl_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_current_cyl_1 = (u_char) (answer >> 8);
	    ms->page5.fd_current_cyl_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (current_cyl_str);
	    }
	}

	chbits = (u_long) ( (ch->page5.fd_step_rate_1 << 8) +
			    (ch->page5.fd_step_rate_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page5.fd_step_rate_1 << 8) +
				(ms->page5.fd_step_rate_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			step_rate_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_step_rate_1 = (u_char) (answer >> 8);
	    ms->page5.fd_step_rate_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (step_rate_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_step_pulse_width;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_step_pulse_width & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, step_pulse_width_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_step_pulse_width = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (step_pulse_width_str);
	    }
	}

	chbits = (u_long) ( (ch->page5.fd_head_settle_delay_1 << 8) +
			    (ch->page5.fd_head_settle_delay_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page5.fd_head_settle_delay_1 << 8) +
				(ms->page5.fd_head_settle_delay_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			head_settle_delay_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_head_settle_delay_1 = (u_char) (answer >> 8);
	    ms->page5.fd_head_settle_delay_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (head_settle_delay_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_motor_on_delay;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_motor_on_delay & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, motor_on_delay_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_motor_on_delay = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (motor_on_delay_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_motor_off_delay;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_motor_off_delay & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, motor_off_delay_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_motor_off_delay = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (motor_off_delay_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_mo;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_mo & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, mo_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_mo = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (mo_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_ssn;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_ssn & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, ssn_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_ssn = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (ssn_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_trdy;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_trdy & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, trdy_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_trdy = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (trdy_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_spc;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_spc & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, spc_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_spc = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (spc_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_write_compensation;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_write_compensation & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, write_compensation_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_write_compensation = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (write_compensation_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_head_load_delay;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_head_load_delay & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, head_load_delay_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_head_load_delay = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (head_load_delay_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_head_unload_delay;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_head_unload_delay & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, head_unload_delay_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_head_unload_delay = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (head_unload_delay_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_pin_1;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_pin_1 & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, pin_1_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_pin_1 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (pin_1_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_pin_2;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_pin_2 & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, pin_2_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_pin_2 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (pin_2_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_pin_4;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_pin_4 & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, pin_4_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_pin_4 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (pin_4_str);
	    }
	}

	chbits = (u_long) ch->page5.fd_pin_34;
	if (chbits) {
	    deflt = (u_long) (ms->page5.fd_pin_34 & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, pin_34_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_pin_34 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (pin_34_str);
	    }
	}

	chbits = (u_long) ( (ch->page5.fd_rotate_rate_1 << 8) +
			    (ch->page5.fd_rotate_rate_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page5.fd_rotate_rate_1 << 8) +
				(ms->page5.fd_rotate_rate_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			rotate_rate_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page5.fd_rotate_rate_1 = (u_char) (answer >> 8);
	    ms->page5.fd_rotate_rate_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (rotate_rate_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectFlexiblePage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyFlexiblePage() - Verify The Flexible Disk Page.		*
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
VerifyFlexiblePage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_flexible_disk *se;
int verify_verbose;
{
	struct scu_device *scu = ScuDevice;
	struct flexible_params ch_flexible_params;
	struct flexible_params vd_flexible_params;
	register struct flexible_params *ch = &ch_flexible_params;
	register struct flexible_params *de;
	register struct flexible_params *vd = &vd_flexible_params;
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
	de = (struct flexible_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct flexible_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseFlexiblePage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page5.page_header.page_code != PageCode) {
	    if ((status = SenseFlexiblePage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_flexible_disk *) 0) {
	    register struct scsi_flexible_disk *fd = &de->page5;

	    *fd = *se;
	}

	if ((status = SetupChangeable (&ch->page5, &de->page5)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Get the vendor default values for sanity checks.
	 */
	if ((status = SenseFlexiblePage (vd, PCF_DEFAULT)) != SUCCESS) {
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

	return (status);
}

/************************************************************************
 *									*
 * IS_FlexibleDisk() - Checks for Flexible Disk Device.			*
 *									*
 * Inputs:	None.							*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = FALSE / TRUE				*
 *									*
 ************************************************************************/
int
IS_FlexibleDisk()
{
	struct flexible_params ms_flexible_params;
	register struct flexible_params *ms = &ms_flexible_params;
	register struct mode_parameter_header *mph = &ms->header.parameter_header;
	int saved_PerrorFlag;

	saved_PerrorFlag = PerrorFlag;
	PerrorFlag = FALSE;

	if (SenseFlexiblePage (ms, PCF_CURRENT) != SUCCESS) {
	    PerrorFlag = saved_PerrorFlag;
	    return (FALSE);
	}

	PerrorFlag = saved_PerrorFlag;

	/*
	 * Some hard disks return this page with a length of zero to
	 * specify the page does not exist, so... additional check.
	 */
	if (mph->mph_data_length == 0) {
	    return (FALSE);
	} else {
	    return (TRUE);
	}
}

/************************************************************************
 *									*
 * SenseFlexiblePage() - Sense The Flexible Disk Page.			*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseFlexiblePage (ms, pcf)
struct flexible_params *ms;
u_char pcf;
{
	return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectFlexiblePage() - Select The Flexible Disk Page.		*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectFlexiblePage (ms, smp)
struct flexible_params *ms;
u_char smp;
{
	struct flexible_params se_flexible_params;
	register struct flexible_params *se = &se_flexible_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
