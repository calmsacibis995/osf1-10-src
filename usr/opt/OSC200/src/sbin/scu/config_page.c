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
static char *rcsid = "@(#)$RCSfile: config_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:05:41 $";
#endif

#include <sys/types.h>
#include <io/cam/rzdisk.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_pages.h"
#include "scsipages.h"

/*
 * Forward References:
 */
int	SetConfigPage(), ShowConfigPage(), ChangeConfigPage(),
	VerifyConfigPage(), SenseConfigPage(), SelectConfigPage();

static u_char PageCode = DEVICE_CONFIG_PAGE;

struct mode_page_funcs device_config_page_funcs = {
	DEVICE_CONFIG_PAGE,		/* The mode page code.		*/
	SetConfigPage,			/* Set mode page function.	*/
	ShowConfigPage,			/* Show mode page function.	*/
	ChangeConfigPage,		/* Change mode page function.	*/
	VerifyConfigPage,		/* Verify mode page function.	*/
	SenseConfigPage,		/* Sense mode page function.	*/
	SelectConfigPage		/* Select mode page function.	*/
};

struct dc_params {				/* Device Configuration Page.*/
	struct mode_page_header header;
	struct scsi_device_config page10;
};

static char *hdr_str =			"Device Configuration";
static char *page_str =			"Page 10";
static char *act_fmt_str =		"Active Format";
static char *caf_str =			"Change Active Format (CAF)";
static char *cap_str =			"Change Active Partition (CAP)";
static char *act_part_str =		"Active Partition";
static char *wb_full_str =		"Write Buffer Full Ratio";
static char *rb_empty_str =		"Read Buffer Empty Ratio";
static char *wt_delay_str =		"Write Delay Time";
static char *rew_str =			"Report Early Warning (REW)";
static char *rbo_str =			"Recover Buffer Order (RBO)";
static char *socf_str =			"Stop On Consecutive Filemarks";
static char *avc_str =			"Automatic Velocity COntrol (AVC)";
static char *rsmk_str =			"Report Setmarks (RSMK)";
static char *bis_str =			"Block Identifiers Supported (BIS)";
static char *dbr_str =			"Data Buffer Recovery (DBR)";
static char *gap_str =			"Gap Size";
static char *sew_str =			"Sync At Early Warning (SEW)";
static char *eeg_str =			"Early EOD Generation (EEG)";
static char *eod_str =			"EOD Definition";
static char *buff_size_str =		"Buffer Size At Early Warning";
static char *sel_dc_str =		"Data Compression Algorithm";

/************************************************************************
 *									*
 * ShowConfigPage() - Show The Device Configuration Page.		*
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
ShowConfigPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct dc_params ms_dc_params;
	register struct dc_params *ms = &ms_dc_params;
	register struct scsi_device_config *dc = &ms_dc_params.page10;
	u_short tmp;
	u_long tmp1;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseConfigPage (ms, PageControlField)) != SUCCESS) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    dc = (struct scsi_device_config *) ph;
	}

	ShowPageHeader (&dc->page_header);

	PrintNumeric (act_fmt_str, dc->dc_act_fmt, PNL);

	PrintNumeric (caf_str, dc->dc_ch_act_fmt, PNL);

	PrintNumeric (cap_str, dc->dc_ch_act_part, PNL);
	     
	PrintNumeric (act_part_str, dc->dc_act_part, PNL);

	PrintNumeric (wb_full_str, dc->dc_wt_buff_full, PNL);

	PrintNumeric (rb_empty_str, dc->dc_rd_buff_empty, PNL);

	tmp = (u_short) ( (dc->dc_wt_delay_time_1 << 8) +
			  (dc->dc_wt_delay_time_0) );
	PrintNumeric (wt_delay_str, tmp, PNL);

	if (PageControlField == PCF_CHANGEABLE) {
	     PrintNumeric (rew_str, dc->dc_rew, PNL);
	} else {
	     PrintAscii (rew_str, yesno_table[dc->dc_rew], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {	     
	     PrintNumeric (rbo_str, dc->dc_rbo, PNL);
	} else {
	     PrintAscii (rbo_str, yesno_table[dc->dc_rbo], PNL);
	}
	
	PrintNumeric (socf_str, dc->dc_socf, PNL);

	if (PageControlField == PCF_CHANGEABLE) {
	     PrintNumeric (avc_str, dc->dc_avc, PNL);
	} else {
	     PrintAscii (avc_str, yesno_table[dc->dc_avc], PNL);
	}
	     
	if (PageControlField == PCF_CHANGEABLE) {
	     PrintNumeric (rsmk_str, dc->dc_rsmk, PNL);
	} else {
	     PrintAscii (rsmk_str, yesno_table[dc->dc_rsmk], PNL);
	}
	
	if (PageControlField == PCF_CHANGEABLE) {
	     PrintNumeric (bis_str, dc->dc_bis, PNL);
	} else {
	     PrintAscii (bis_str, yesno_table[dc->dc_bis], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {
	     PrintNumeric (dbr_str, dc->dc_dbr, PNL);
	} else {
	     PrintAscii (dbr_str, yesno_table[dc->dc_dbr], PNL);
	}

	PrintNumeric (gap_str, dc->dc_gap_size, PNL);

	if (PageControlField == PCF_CHANGEABLE) {
	     PrintNumeric (sew_str, dc->dc_sew, PNL);
	} else {
	     PrintAscii (sew_str, yesno_table[dc->dc_sew], PNL);
	}
	
	if (PageControlField == PCF_CHANGEABLE) {
	     PrintNumeric (eeg_str, dc->dc_eeg, PNL);
	} else {
	     PrintAscii (eeg_str, yesno_table[dc->dc_eeg], PNL);
	}
	
	PrintNumeric (eod_str, dc->dc_eod_defined, PNL);

	tmp1 = (u_long) (dc->dc_buff_sz_at_ew_2);
	tmp1 = tmp1 << 16;
	tmp1 |= (u_long) ( (dc->dc_buff_sz_at_ew_1 << 8) +
			   (dc->dc_buff_sz_at_ew_0) );
	PrintNumeric (buff_size_str, tmp1, PNL);

	PrintNumeric (sel_dc_str, dc->dc_sel_data_comp, PNL);

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetConfigPage() - Set The Device Configuration Page.			*
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
SetConfigPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct dc_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct dc_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct dc_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyConfigPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Set the device configuration parameters.
	 */
	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	return (SelectConfigPage (de, SaveModeParameters));
}

/************************************************************************
 *									*
 * ChangeConfigPage() - Change The Device Configuration Page.		*
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
ChangeConfigPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct dc_params ch_dc_params;
	struct dc_params ms_dc_params;
	register struct dc_params *ch = &ch_dc_params;
	register struct dc_params *de;
	register struct dc_params *ms = &ms_dc_params;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct dc_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct dc_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseConfigPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseConfigPage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page10, &ms->page10)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);


	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	if (ch->page10.dc_act_fmt) {
	    deflt = ms->page10.dc_act_fmt;
	    range.pr_max = ch->page10.dc_act_fmt;
	    if ((status = PromptUser (pc, TYPE_VALUE, act_fmt_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_act_fmt = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (act_fmt_str);
	    }
	}

	if (ch->page10.dc_ch_act_fmt) {
	    deflt = ms->page10.dc_ch_act_fmt;
	    range.pr_max = ch->page10.dc_ch_act_fmt;
	    if ((status = PromptUser (pc, TYPE_VALUE, caf_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_ch_act_fmt = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (caf_str);
	    }
	}

	if (ch->page10.dc_ch_act_part) {
	    deflt = ms->page10.dc_ch_act_part;
	    range.pr_max = ch->page10.dc_ch_act_part;
	    if ((status = PromptUser (pc, TYPE_VALUE, cap_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_ch_act_part = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (cap_str);
	    }
	}

	if (ch->page10.dc_act_part) {
	    deflt = ms->page10.dc_act_part;
	    range.pr_max = ch->page10.dc_act_part;
	    if ((status = PromptUser (pc, TYPE_VALUE, act_part_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_act_part = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (act_part_str);
	    }
	}

	if (ch->page10.dc_wt_buff_full) {
	    deflt = ms->page10.dc_wt_buff_full;
	    range.pr_max = ch->page10.dc_wt_buff_full;
	    if ((status = PromptUser (pc, TYPE_VALUE, wb_full_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_wt_buff_full = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (wb_full_str);
	    }
	}

	if (ch->page10.dc_rd_buff_empty) {
	    deflt = ms->page10.dc_rd_buff_empty;
	    range.pr_max = ch->page10.dc_rd_buff_empty;
	    if ((status = PromptUser (pc, TYPE_VALUE, rb_empty_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_rd_buff_empty = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (rb_empty_str);
	    }
	}

	chbits = (u_long) ( ((u_long)ch->page10.dc_wt_delay_time_1 << 8) +
			    ((u_long)ch->page10.dc_wt_delay_time_0) );
	if (chbits) {
	    deflt = (u_long) ( (((u_long)ms->page10.dc_wt_delay_time_1 << 8) +
			        ((u_long)ms->page10.dc_wt_delay_time_0) ) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			wt_delay_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_wt_delay_time_1 = (u_char) (answer >> 8);
	    ms->page10.dc_wt_delay_time_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (wt_delay_str);
	    }
	}

	if (ch->page10.dc_rew) {
	    deflt = ms->page10.dc_rew;
	    range.pr_max = ch->page10.dc_rew;
	    if ((status = PromptUser (pc, TYPE_VALUE, rew_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_rew = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (rew_str);
	    }
	}

	if (ch->page10.dc_rbo) {
	    deflt = ms->page10.dc_rbo;
	    range.pr_max = ch->page10.dc_rbo;
	    if ((status = PromptUser (pc, TYPE_VALUE, rbo_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_rbo = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (rbo_str);
	    }
	}

	if (ch->page10.dc_socf) {
	    deflt = ms->page10.dc_socf;
	    range.pr_max = ch->page10.dc_socf;
	    if ((status = PromptUser (pc, TYPE_VALUE, socf_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_socf = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (socf_str);
	    }
	}

	if (ch->page10.dc_avc) {
	    deflt = ms->page10.dc_avc;
	    range.pr_max = ch->page10.dc_avc;
	    if ((status = PromptUser (pc, TYPE_VALUE, avc_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_avc = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (avc_str);
	    }
	}

	if (ch->page10.dc_rsmk) {
	    deflt = ms->page10.dc_rsmk;
	    range.pr_max = ch->page10.dc_rsmk;
	    if ((status = PromptUser (pc, TYPE_VALUE, rsmk_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_rsmk = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (rsmk_str);
	    }
	}

	if (ch->page10.dc_bis) {
	    deflt = ms->page10.dc_bis;
	    range.pr_max = ch->page10.dc_bis;
	    if ((status = PromptUser (pc, TYPE_VALUE, bis_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_bis = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (bis_str);
	    }
	}

	if (ch->page10.dc_dbr) {
	    deflt = ms->page10.dc_dbr;
	    range.pr_max = ch->page10.dc_dbr;
	    if ((status = PromptUser (pc, TYPE_VALUE, dbr_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_dbr = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (dbr_str);
	    }
	}

	if (ch->page10.dc_gap_size) {
	    deflt = ms->page10.dc_gap_size;
	    range.pr_max = ch->page10.dc_gap_size;
	    if ((status = PromptUser (pc, TYPE_VALUE, gap_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_gap_size = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (gap_str);
	    }
	}

	if (ch->page10.dc_sew) {
	    deflt = ms->page10.dc_sew;
	    range.pr_max = ch->page10.dc_sew;
	    if ((status = PromptUser (pc, TYPE_VALUE, sew_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_sew = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (sew_str);
	    }
	}

	if (ch->page10.dc_eeg) {
	    deflt = ms->page10.dc_eeg;
	    range.pr_max = ch->page10.dc_eeg;
	    if ((status = PromptUser (pc, TYPE_VALUE, eeg_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_eeg = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (eeg_str);
	    }
	}

	if (ch->page10.dc_eod_defined) {
	    deflt = ms->page10.dc_eod_defined;
	    range.pr_max = ch->page10.dc_eod_defined;
	    if ((status = PromptUser (pc, TYPE_VALUE, eod_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_eod_defined = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (eod_str);
	    }
	}

	chbits = (u_long) ( ((u_long)ch->page10.dc_buff_sz_at_ew_2 << 16) +
			    ((u_long)ch->page10.dc_buff_sz_at_ew_1 << 8) +
			    ((u_long)ch->page10.dc_buff_sz_at_ew_0) );
	if (chbits) {
	    deflt = (u_long) ( (((u_long)ms->page10.dc_buff_sz_at_ew_2 << 16) +
			        ((u_long)ms->page10.dc_buff_sz_at_ew_1 << 8) +
			        ((u_long)ms->page10.dc_buff_sz_at_ew_0) ) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			buff_size_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_buff_sz_at_ew_2 = (u_char) (answer >> 16);
	    ms->page10.dc_buff_sz_at_ew_1 = (u_char) (answer >> 8);
	    ms->page10.dc_buff_sz_at_ew_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (buff_size_str);
	    }
	}

	if (ch->page10.dc_sel_data_comp) {
	    deflt = ms->page10.dc_sel_data_comp;
	    range.pr_max = ch->page10.dc_sel_data_comp;
	    if ((status = PromptUser (pc, TYPE_VALUE, sel_dc_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page10.dc_sel_data_comp = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (sel_dc_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectConfigPage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyConfigPage() - Verify The Config Control Page.		*
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
static int
VerifyConfigPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_device_config *se;
int verify_verbose;
{
	struct dc_params ch_dc_params;
	register struct dc_params *ch = &ch_dc_params;
	register struct dc_params *de;
	int status, field_changed;
	u_long ch_field, de_field;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct dc_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct dc_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseConfigPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page10.page_header.page_code != PageCode) {
	    if ((status = SenseConfigPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_device_config *) 0) {
	    register struct scsi_device_config *dc = &de->page10;

	    *dc = *se;
	}

	if ((status = SetupChangeable (&ch->page10, &de->page10)) != SUCCESS) {
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
#endif notdef
	return (status);
}

/************************************************************************
 *									*
 * SenseConfigPage() - Sense The Config Control Page.			*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseConfigPage (ms, pcf)
struct dc_params *ms;
u_char pcf;
{
	return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectConfigPage() - Select The Config Control Page.			*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectConfigPage (ms, smp)
struct dc_params *ms;
u_char smp;
{
	struct dc_params se_dc_params;
	register struct dc_params *se = &se_dc_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
