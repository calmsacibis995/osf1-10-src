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
static char *rcsid = "@(#)$RCSfile: partition_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:18:28 $";
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
int	SetPartitionPage1(), ShowPartitionPage1(), ChangePartitionPage1(),
	VerifyPartitionPage1(), SensePartitionPage1(), SelectPartitionPage1();

static u_char PageCode = MEDIUM_PART_PAGE1;

struct mode_page_funcs medium_part_page1_funcs = {
	MEDIUM_PART_PAGE1,		/* The mode page code.		*/
	SetPartitionPage1,		/* Set mode page function.	*/
	ShowPartitionPage1,		/* Show mode page function.	*/
	ChangePartitionPage1,		/* Change mode page function.	*/
	VerifyPartitionPage1,		/* Verify mode page function.	*/
	SensePartitionPage1,		/* Sense mode page function.	*/
	SelectPartitionPage1		/* Select mode page function.	*/
};

struct mp_params {			/* Medium Partition Page.*/
	struct mode_page_header header;
	struct scsi_medium_part page11;
};

static char *hdr_str =			"Medium Partition Page(1)";
static char *page_str =			"Page 11";
static char *max_parts_str =		"Maximum Additional Partitions";
static char *add_parts_str =		"Additional Partitions";
static char *psum_str =			"Partition Size Unit of Measure";
static char *idp_str =			"Initiator Defined Partitions";
static char *sdp_str =			"Select Data Partitions";
static char *fdp_str =			"Fixed Data Partitions";
static char *med_fmt_str =		"Medium Format Recognition";
static char *psd_hdr_str =		"Numer of Partitions";
static char *psd_str =			"Partition Size";

/************************************************************************
 *									*
 * ShowPartitionPage1() - Show The Medium Partition Page(1).		*
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
ShowPartitionPage1 (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct mp_params ms_mp_params;
	register struct mp_params *ms = &ms_mp_params;
	register struct scsi_medium_part *mp = &ms_mp_params.page11;
	u_long tmp;
	int status = SUCCESS;
	int index;
	int parts_to_disp;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SensePartitionPage1 (ms, PageControlField)) != SUCCESS) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    mp = (struct scsi_medium_part *) ph;
	}

	ShowPageHeader (&mp->page_header);

	PrintNumeric (max_parts_str, mp->mp_max_parts, PNL);

	PrintNumeric (add_parts_str, mp->mp_add_parts, PNL);

	PrintNumeric (psum_str, mp->mp_psum, PNL);

	PrintNumeric (idp_str, mp->mp_idp, PNL);

	PrintNumeric (sdp_str, mp->mp_sdp, PNL);

	PrintNumeric (fdp_str, mp->mp_fdp, PNL);

	PrintNumeric (med_fmt_str, mp->mp_med_fmt, PNL);

	if ( parts_to_disp = (mp->page_header.page_length - 
			      MEDIUM_PART_BASE_SIZE)/2 ) {
	    PrintNumeric (psd_hdr_str, parts_to_disp, PNL);
	    for ( index = 0; index < parts_to_disp; index++ ) {
		tmp = (u_long) ( ((u_long)mp->mp_psdesc[index].mp_psd1 << 8) +
			      ((u_long)mp->mp_psdesc[index].mp_psd0) );
		if (OutputRadix == HEX_RADIX) {
		    Printf ("%31.31s %d: %#x\n", psd_str, index, tmp);
		} else {
		    Printf ("%31.31s %d: %d\n", psd_str, index, tmp);
		}
	    }
        }

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetPartitionPage1() - Set The Medium Partition Page(1).		*
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
SetPartitionPage1 (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct mp_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct mp_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct mp_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyPartitionPage1 (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Set the device configuration parameters.
	 */
	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	return (SelectPartitionPage1 (de, SaveModeParameters));
}

/************************************************************************
 *									*
 * ChangePartitionPage1() - Change The Medium Partition Page(1).	*
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
ChangePartitionPage1 (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct mp_params ch_mp_params;
	struct mp_params ms_mp_params;
	register struct mp_params *ch = &ch_mp_params;
	register struct mp_params *de;
	register struct mp_params *ms = &ms_mp_params;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct mp_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct mp_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SensePartitionPage1 (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SensePartitionPage1 (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page11, &ms->page11)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	if (ch->page11.mp_max_parts) {
	    deflt = ms->page11.mp_max_parts;
	    range.pr_max = ch->page11.mp_max_parts;
	    if ((status = PromptUser (pc, TYPE_VALUE, max_parts_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page11.mp_max_parts = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (max_parts_str);
	    }
	}

	if (ch->page11.mp_add_parts) {
	    deflt = ms->page11.mp_add_parts;
	    range.pr_max = ch->page11.mp_add_parts;
	    if ((status = PromptUser (pc, TYPE_VALUE, add_parts_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page11.mp_add_parts = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (add_parts_str);
	    }
	}

	if (ch->page11.mp_psum) {
	    deflt = ms->page11.mp_psum;
	    range.pr_max = ch->page11.mp_psum;
	    if ((status = PromptUser (pc, TYPE_VALUE, psum_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page11.mp_psum = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (psum_str);
	    }
	}

	if (ch->page11.mp_idp) {
	    deflt = ms->page11.mp_idp;
	    range.pr_max = ch->page11.mp_idp;
	    if ((status = PromptUser (pc, TYPE_VALUE, idp_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page11.mp_idp = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (idp_str);
	    }
	}

	if (ch->page11.mp_sdp) {
	    deflt = ms->page11.mp_sdp;
	    range.pr_max = ch->page11.mp_sdp;
	    if ((status = PromptUser (pc, TYPE_VALUE, sdp_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page11.mp_sdp = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (sdp_str);
	    }
	}

	if (ch->page11.mp_fdp) {
	    deflt = ms->page11.mp_fdp;
	    range.pr_max = ch->page11.mp_fdp;
	    if ((status = PromptUser (pc, TYPE_VALUE, fdp_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page11.mp_fdp = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (fdp_str);
	    }
	}

	if (ch->page11.mp_med_fmt) {
	    deflt = ms->page11.mp_med_fmt;
	    range.pr_max = ch->page11.mp_med_fmt;
	    if ((status = PromptUser (pc, TYPE_VALUE, med_fmt_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page11.mp_med_fmt = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (med_fmt_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectPartitionPage1 (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyPartitionPage1() - Verify The Partition Control Page(1).	*
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
VerifyPartitionPage1 (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_medium_part *se;
int verify_verbose;
{
	struct mp_params ch_mp_params;
	register struct mp_params *ch = &ch_mp_params;
	register struct mp_params *de;
	int status, field_changed;
	u_long ch_field, de_field;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct mp_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct mp_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SensePartitionPage1 (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page11.page_header.page_code != PageCode) {
	    if ((status = SensePartitionPage1 (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_medium_part *) 0) {
	    register struct scsi_medium_part *mp = &de->page11;

	    *mp = *se;
	}

	if ((status = SetupChangeable (&ch->page11, &de->page11)) != SUCCESS) {
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
 * SensePartitionPage1() - Sense The Partition Control Page(1).		*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SensePartitionPage1 (ms, pcf)
struct mp_params *ms;
u_char pcf;
{
	return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectPartitionPage1() - Select The Medium Partition Page(1).	*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectPartitionPage1 (ms, smp)
struct mp_params *ms;
u_char smp;
{
	struct mp_params se_mp_params;
	register struct mp_params *se = &se_mp_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}


