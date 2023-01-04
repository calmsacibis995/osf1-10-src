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
static char *rcsid = "@(#)$RCSfile: cache_page.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/11/23 23:08:10 $";
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
 * File:	cache_page.c
 * Author:	Robin T. Miller
 * Date:	December 19, 1990
 *
 * Description:
 *	This file contains the functions for the cache control page.
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
 * Forward References:
 */
int	SetCachePage(), ShowCachePage(), ChangeCachePage(),
	VerifyCachePage(), SenseCachePage(), SelectCachePage();

static u_char PageCode = CACHE_CONTROL_PAGE;

struct mode_page_funcs cache_page_funcs = {
	CACHE_CONTROL_PAGE,		/* The mode page code.		*/
	SetCachePage,			/* Set mode page function.	*/
	ShowCachePage,			/* Show mode page function.	*/
	ChangeCachePage,		/* Change mode page function.	*/
	VerifyCachePage,		/* Verify mode page function.	*/
	SenseCachePage,			/* Sense mode page function.	*/
	SelectCachePage			/* Select mode page function.	*/
};

struct cac_params {				/* Cache Control Page.	*/
	struct mode_page_header header;
	struct scsi_cache_control page8;
};

static char *hdr_str =			"Cache Control";
static char *page_str =			"Page 8";
static char *rcd_str =			"Read Cache Disable";
static char *ms_str =			"Multiple Selection Enable";
static char *wce_str =			"Write Cache Enable";
static char *write_reten_pri_str =	"Write Retention Priority";
static char *read_reten_pri_str =	"Demand Read Retention Priority";
static char *dis_prefetch_xfer_str  =	"Disable Prefetch Transfer Len";
static char *min_prefetch_str =		"Minimum Prefetch";
static char *max_prefetch_str =		"Maximum Prefetch";
static char *max_prefetch_ceiling_str =	"Maximum Prefetch Ceiling";

/************************************************************************
 *									*
 * ShowCachePage() - Show The Cache Control Page.			*
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
ShowCachePage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct cac_params ms_cac_params;
	register struct cac_params *ms = &ms_cac_params;
	register struct scsi_cache_control *ca = &ms_cac_params.page8;
	u_short tmp;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseCachePage (ms, PageControlField)) != SUCCESS) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    ca = (struct scsi_cache_control *) ph;
	}

	ShowPageHeader (&ca->page_header);

	PrintAscii (rcd_str, yesno_table[ca->ca_rcd], PNL);

	PrintAscii (ms_str, yesno_table[ca->ca_ms], PNL);

	PrintAscii (wce_str, yesno_table[ca->ca_wce], PNL);

	PrintNumeric (write_reten_pri_str, ca->ca_write_reten_pri, PNL);

	PrintNumeric (read_reten_pri_str, ca->ca_read_reten_pri, PNL);

	tmp = (u_short) ( (ca->ca_dis_prefetch_xfer_1 << 8) +
			  (ca->ca_dis_prefetch_xfer_0) );
	PrintNumeric (dis_prefetch_xfer_str, tmp, PNL);

	tmp = (u_short) ( (ca->ca_min_prefetch_1 << 8) +
			  (ca->ca_min_prefetch_0) );
	PrintNumeric (min_prefetch_str, tmp, PNL);

	tmp = (u_short) ( (ca->ca_max_prefetch_1 << 8) +
			  (ca->ca_max_prefetch_0) );
	PrintNumeric (max_prefetch_str, tmp, PNL);

	tmp = (u_short) ( (ca->ca_max_prefetch_ceiling_1 << 8) +
			  (ca->ca_max_prefetch_ceiling_0) );
	PrintNumeric (max_prefetch_ceiling_str, tmp, PNL);

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetCachePage() - Set The Cache Control Page.				*
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
SetCachePage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct cac_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct cac_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct cac_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyCachePage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Set the read-ahead control parameters.
	 */
	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	return (SelectCachePage (de, SaveModeParameters));
}

/************************************************************************
 *									*
 * ChangeCachePage() - Change The Cache Control Page.			*
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
ChangeCachePage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct cac_params ch_cac_params;
	struct cac_params ms_cac_params;
	register struct cac_params *ch = &ch_cac_params;
	register struct cac_params *de;
	register struct cac_params *ms = &ms_cac_params;
	register struct parser_control *pc = &ParserControl;
	register struct scu_device *scu = ScuDevice;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct cac_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct cac_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseCachePage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseCachePage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page8, &ms->page8)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	if (ch->page8.ca_rcd) {
	    deflt = ms->page8.ca_rcd;
	    range.pr_max = ch->page8.ca_rcd;
	    if ((status = PromptUser (pc, TYPE_VALUE, rcd_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_rcd = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (rcd_str);
	    }
	}

	if (ch->page8.ca_ms) {
	    deflt = ms->page8.ca_ms;
	    range.pr_max = ch->page8.ca_ms;
	    if ((status = PromptUser (pc, TYPE_VALUE, ms_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_ms = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (ms_str);
	    }
	}

	if (ch->page8.ca_wce) {
	    deflt = ms->page8.ca_wce;
	    range.pr_max = ch->page8.ca_wce;
	    if ((status = PromptUser (pc, TYPE_VALUE, wce_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_wce = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (wce_str);
	    }
	}

	if (ch->page8.ca_write_reten_pri) {
	    deflt = ms->page8.ca_write_reten_pri;
	    range.pr_max = ch->page8.ca_write_reten_pri;
	    if ((status = PromptUser (pc, TYPE_VALUE, write_reten_pri_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_write_reten_pri = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (write_reten_pri_str);
	    }
	}

	if (ch->page8.ca_read_reten_pri) {
	    deflt = ms->page8.ca_read_reten_pri;
	    range.pr_max = ch->page8.ca_read_reten_pri;
	    if ((status = PromptUser (pc, TYPE_VALUE, read_reten_pri_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_read_reten_pri = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (read_reten_pri_str);
	    }
	}

	chbits = (u_long) ( (ch->page8.ca_dis_prefetch_xfer_1 << 8) +
			    (ch->page8.ca_dis_prefetch_xfer_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page8.ca_dis_prefetch_xfer_1 << 8) +
				(ms->page8.ca_dis_prefetch_xfer_0) ) &
			         chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   dis_prefetch_xfer_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_dis_prefetch_xfer_1 = (u_char) (answer >> 8);
	    ms->page8.ca_dis_prefetch_xfer_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (dis_prefetch_xfer_str);
	    }
	}

	chbits = (u_long) ( (ch->page8.ca_min_prefetch_1 << 8) +
			    (ch->page8.ca_min_prefetch_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page8.ca_min_prefetch_1 << 8) +
				(ms->page8.ca_min_prefetch_0) ) &
			         chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   min_prefetch_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_min_prefetch_1 = (u_char) (answer >> 8);
	    ms->page8.ca_min_prefetch_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (min_prefetch_str);
	    }
	}

	chbits = (u_long) ( (ch->page8.ca_max_prefetch_1 << 8) +
			    (ch->page8.ca_max_prefetch_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page8.ca_max_prefetch_1 << 8) +
				(ms->page8.ca_max_prefetch_0) ) &
			         chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   max_prefetch_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_max_prefetch_1 = (u_char) (answer >> 8);
	    ms->page8.ca_max_prefetch_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (max_prefetch_str);
	    }
	}

	chbits = (u_long) ( (ch->page8.ca_max_prefetch_ceiling_1 << 8) +
			    (ch->page8.ca_max_prefetch_ceiling_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page8.ca_max_prefetch_ceiling_1 << 8) +
				(ms->page8.ca_max_prefetch_ceiling_0) ) &
			         chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			   max_prefetch_ceiling_str, ':',
			   &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page8.ca_max_prefetch_ceiling_1 = (u_char) (answer >> 8);
	    ms->page8.ca_max_prefetch_ceiling_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (max_prefetch_ceiling_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectCachePage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyCachePage() - Verify The Cache Control Page.		*
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
VerifyCachePage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_cache_control *se;
int verify_verbose;
{
	struct cac_params ch_cac_params;
	register struct cac_params *ch = &ch_cac_params;
	register struct cac_params *de;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct cac_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct cac_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseCachePage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page8.page_header.page_code != PageCode) {
	    if ((status = SenseCachePage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_cache_control *) 0) {
	    register struct scsi_cache_control *ca = &de->page8;

	    *ca = *se;
	}

	if ((status = SetupChangeable (&ch->page8, &de->page8)) != SUCCESS) {
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
#endif /* notdef */
	return (status);
}

/************************************************************************
 *									*
 * SenseCachePage() - Sense The Cache Control Page.			*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseCachePage (ms, pcf)
struct cac_params *ms;
u_char pcf;
{
	return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectCachePage() - Select The Cache Control Page.			*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectCachePage (ms, smp)
struct cac_params *ms;
u_char smp;
{
	struct cac_params se_cac_params;
	register struct cac_params *se = &se_cac_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
