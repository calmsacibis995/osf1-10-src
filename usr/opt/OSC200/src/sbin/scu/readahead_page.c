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
static char *rcsid = "@(#)$RCSfile: readahead_page.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/06/25 10:19:53 $";
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
 * File:	readahead_page.c
 * Author:	Robin T. Miller
 * Date:	December 19, 1990
 *
 * Description:
 *	This file contains the functions for the read-ahead control page.
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

/*
 * Forward References:
 */
int	SetReadAheadPage(), ShowReadAheadPage(), ChangeReadAheadPage(),
	VerifyReadAheadPage(), SenseReadAheadPage(), SelectReadAheadPage();

static u_char PageCode = READAHEAD_CONTROL_PAGE;

struct mode_page_funcs readahead_page_funcs = {
	READAHEAD_CONTROL_PAGE,		/* The mode page code.		*/
	SetReadAheadPage,		/* Set mode page function.	*/
	ShowReadAheadPage,		/* Show mode page function.	*/
	ChangeReadAheadPage,		/* Change mode page function.	*/
	VerifyReadAheadPage,		/* Verify mode page function.	*/
	SenseReadAheadPage,		/* Sense mode page function.	*/
	SelectReadAheadPage		/* Select mode page function.	*/
};
struct rac_params {				/* Read-Ahead Control.	*/
	struct mode_page_header header;
	struct scsi_readahead_control page38;
};

static char *hdr_str =			"Read-Ahead Control";
static char *page_str =			"Page 38";
static char *cache_enable_str =		"Cache Enable";
static char *cache_table_size_str =	"Cache Table Size";
static char *ramd_str =			"Read-Ahead w/Mechanical Delay";
static char *prefetch_threshold_str =	"Prefetch Threshold";
static char *min_prefetch_mult_str =	"Minimum Prefetch Multiplier";
static char *max_prefetch_mult_str =	"Maximum prefetch multiplier";
static char *min_prefetch_str =		"Miniumum Prefetch";
static char *max_prefetch_str =		"Maximum Prefetch";

/************************************************************************
 *									*
 * ShowReadAheadPage() - Show The Read-Ahead Control Page.		*
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
ShowReadAheadPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct rac_params ms_rac_params;
	register struct rac_params *ms = &ms_rac_params;
	register struct scsi_readahead_control *ra = &ms_rac_params.page38;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseReadAheadPage (ms, PageControlField)) != 0) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    ra = (struct scsi_readahead_control *) ph;
	}

	ShowPageHeader (&ra->page_header);

	PrintAscii (cache_enable_str, yesno_table[ra->ra_cache_enable], PNL);

	PrintNumeric (cache_table_size_str, ra->ra_table_size, PNL);

	PrintAscii (ramd_str, yesno_table[ra->ra_ramd], PNL);

	PrintNumeric (prefetch_threshold_str, ra->ra_threshold, PNL);

	PrintNumeric (min_prefetch_mult_str, ra->ra_min_multiplier, PNL);

	PrintNumeric (max_prefetch_mult_str, ra->ra_max_multiplier, PNL);

	PrintNumeric (min_prefetch_str, ra->ra_min_prefetch, PNL);

	PrintNumeric (max_prefetch_str, ra->ra_max_prefetch, PNL);

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetReadAheadPage() - Set The Read-Ahead Control Page.		*
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
SetReadAheadPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct rac_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct rac_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct rac_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyReadAheadPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Set the read-ahead control parameters.
	 */
	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	return (SelectReadAheadPage (de, SaveModeParameters));
}

/************************************************************************
 *									*
 * ChangeReadAheadPage() - Change The Read-Ahead Control Page.		*
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
ChangeReadAheadPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct rac_params ch_rac_params;
	struct rac_params ms_rac_params;
	register struct rac_params *ch = &ch_rac_params;
	register struct rac_params *de;
	register struct rac_params *ms = &ms_rac_params;
	register struct parser_control *pc = &ParserControl;
	register struct scu_device *scu = ScuDevice;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct rac_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct rac_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseReadAheadPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseReadAheadPage (ms, pcf)) != SUCCESS) {
		return (status);
	}

	if ((status = SetupChangeable (&ch->page38, &ms->page38)) != SUCCESS) {
		return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	chbits = ch->page38.ra_table_size;
	if (chbits) {
	    deflt = ch->page38.ra_table_size & chbits;
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   cache_table_size_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page38.ra_table_size = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (cache_table_size_str);
	    }
	}

	if (ch->page38.ra_cache_enable) {
	    deflt = ms->page38.ra_cache_enable;
	    range.pr_max = ch->page38.ra_cache_enable;
	    if ((status = PromptUser (pc, TYPE_VALUE, 
			  cache_enable_str, ':',
			  &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page38.ra_cache_enable = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (cache_enable_str);
	    }
	}

	if (ch->page38.ra_ramd) {
	    deflt = ms->page38.ra_ramd;
	    range.pr_max = ch->page38.ra_ramd;
	    if ((status = PromptUser (pc, TYPE_VALUE,
			  ramd_str, ':',
			  &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page38.ra_ramd = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (ramd_str);
	    }
	}

	chbits = (u_long) ( ch->page38.ra_threshold );
	if (chbits) {
	    deflt = (u_long) ( ch->page38.ra_threshold & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   prefetch_threshold_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page38.ra_threshold = (u_char) (answer);
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (prefetch_threshold_str);
	    }
	}

	chbits = (u_long) ( ch->page38.ra_max_prefetch );
	if (chbits) {
	    deflt = (u_long) ( ch->page38.ra_max_prefetch & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   max_prefetch_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page38.ra_max_prefetch = (u_char) (answer);
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (max_prefetch_str);
	    }
	}

	chbits = (u_long) ( ch->page38.ra_max_multiplier );
	if (chbits) {
	    deflt = (u_long) ( ch->page38.ra_max_multiplier & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   max_prefetch_mult_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page38.ra_max_multiplier = (u_char) (answer);
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (max_prefetch_mult_str);
	    }
	}

	chbits = (u_long) ( ch->page38.ra_min_prefetch );
	if (chbits) {
	    deflt = (u_long) ( ch->page38.ra_min_prefetch & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   min_prefetch_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page38.ra_min_prefetch = (u_char) (answer);
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (min_prefetch_str);
	    }
	}

	chbits = (u_long) ( ch->page38.ra_min_multiplier );
	if (chbits) {
	    deflt = (u_long) ( ch->page38.ra_min_multiplier & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   min_prefetch_mult_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page38.ra_min_multiplier = (u_char) (answer);
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (min_prefetch_mult_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectReadAheadPage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyReadAheadPage() - Verify The Read-Ahead Control Page.		*
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
VerifyReadAheadPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_readahead_control *se;
int verify_verbose;
{
	struct rac_params ch_rac_params;
	register struct rac_params *ch = &ch_rac_params;
	register struct rac_params *de;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct rac_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct rac_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseReadAheadPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page38.page_header.page_code != PageCode) {
	    if ((status = SenseReadAheadPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_readahead_control *) 0) {
	    register struct scsi_readahead_control *ra = &de->page38;

	    *ra = *se;
	}

	if ((status = SetupChangeable (&ch->page38, &de->page38)) != SUCCESS) {
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
	 * Verify "Read-Ahead Control Mode" parameters.
	 */
	if (dt->dtype_options & SUP_READAHEAD) {
	    ch_field = ch->page38.ra_cache_enable;
	    if (ch_field) {
		de->page38.ra_cache_enable =
		    (cur_dtype->dtype_rac & RAC_CACHE_ENABLE) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str,
				cache_enable_str, de->page38.ra_cache_enable);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (cache_enable_str);
		}
	    }
	    ch_field = ch->page38.ra_table_size;
	    if (ch_field) {
		de->page38.ra_table_size =
				(cur_dtype->dtype_rac & RAC_TABLE_SIZE);
		if (verify_verbose) {
		    Printf (setting_str,
				cache_table_size_str, de->page38.ra_table_size);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (cache_table_size_str);
		}
	    }
	    ch_field = ch->page38.ra_ramd;
	    if (ch_field) {
		de->page38.ra_ramd =
		    (cur_dtype->dtype_rac & RAC_RAMD) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, ramd_str, de->page38.ra_ramd);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (ramd_str);
		}
	    }
	}

	/*
	 * Verify "Prefetch Threshold" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page38.ra_threshold;
	if (ch_field) {
	    if (dt->dtype_options & SUP_PREFETCH) {
		if (dt->dtype_threshold <= ch_field) {
		    field_changed = 1;
		    de->page38.ra_threshold = dt->dtype_threshold;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n",
				    prefetch_threshold_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					prefetch_threshold_str,
					de->page38.ra_threshold);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (prefetch_threshold_str);
	    }
	}

	/*
	 * Verify "Maximum Prefetch" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page38.ra_max_prefetch;
	if (ch_field) {
	    if (dt->dtype_options & SUP_PREFETCH_MAX) {
		if (dt->dtype_prefetch_max <= ch_field) {
		    field_changed = 1;
		    de->page38.ra_max_prefetch = dt->dtype_prefetch_max;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n",
				    max_prefetch_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					max_prefetch_str,
					de->page38.ra_max_prefetch);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (max_prefetch_str);
	    }
	}

	/*
	 * Verify "Minimum Prefetch" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page38.ra_min_prefetch;
	if (ch_field) {
	    if (dt->dtype_options & SUP_PREFETCH_MIN) {
		if (dt->dtype_prefetch_min <= ch_field) {
		    field_changed = 1;
		    de->page38.ra_min_prefetch = dt->dtype_prefetch_min;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n",
				    min_prefetch_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					min_prefetch_str,
					de->page38.ra_min_prefetch);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (min_prefetch_str);
	    }
	}
#endif notdef
	return (status);
}

/************************************************************************
 *									*
 * SenseReadAheadPage() - Sense The Read-Ahead Control Page.		*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseReadAheadPage (ms, pcf)
struct rac_params *ms;
u_char pcf;
{
	return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectReadAheadPage() - Select The Read-Ahead Control Page.		*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectReadAheadPage (ms, smp)
struct rac_params *ms;
u_char smp;
{
	struct rac_params se_rac_params;
	register struct rac_params *se = &se_rac_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
