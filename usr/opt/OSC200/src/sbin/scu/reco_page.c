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
static char *rcsid = "@(#)$RCSfile: reco_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:21:23 $";
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
 * File:	reco_page.c
 * Author:	Robin T. Miller
 * Date:	December 19, 1990
 *
 * Description:
 *	This file contains the functions for the disconnect/reconnect
 * control page.
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
int	SetRecoPage(), ShowRecoPage(), ChangeRecoPage(),
	VerifyRecoPage(), SenseRecoPage(), SelectRecoPage();

static u_char PageCode = DISCO_RECO_PAGE;

struct mode_page_funcs reco_page_funcs = {
	DISCO_RECO_PAGE,		/* The mode page code.		*/
	SetRecoPage,			/* Set mode page function.	*/
	ShowRecoPage,			/* Show mode page function.	*/
	ChangeRecoPage,			/* Change mode page function.	*/
	VerifyRecoPage,			/* Verify mode page function.	*/
	SenseRecoPage,			/* Sense mode page function.	*/
	SelectRecoPage			/* Select mode page function.	*/
};

struct reco_params {				/* Reconnect/Disconnect	*/
	struct mode_page_header header;
	struct scsi_disco_reco page2;
};

static char *hdr_str =			"Disconnect/Reconnect Control";
static char *page_str =			"Page 2";
static char *buffer_full_ratio_str =	"Buffer Full Ratio";
static char *buffer_empty_ratio_str =	"Buffer Empty Ratio";
static char *bus_inactivity_limit_str = "Bus Inactivity Limit";
static char *disconnect_time_limit_str = "Disconnect Time Limit";
static char *connect_time_limit_str = 	"Connect Time Limit";
static char *maximum_burst_size_str =	"Maximum Burst Size";
static char *dtdc_str =			"Data Transfer Disc Control";

/************************************************************************
 *									*
 * ShowRecoPage() - Show The Disconnect/Reconnect Control Page.		*
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
ShowRecoPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct reco_params ms_reco_params;
	register struct reco_params *ms = &ms_reco_params;
	register struct scsi_disco_reco *dr = &ms_reco_params.page2;
	register struct scu_device *scu = ScuDevice;
	u_short tmp;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseRecoPage (ms, PageControlField)) != 0) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    dr = (struct scsi_disco_reco *) ph;
	}

	ShowPageHeader (&dr->page_header);

	PrintNumeric (buffer_full_ratio_str, dr->dr_buffer_full_ratio, PNL);

	PrintNumeric (buffer_empty_ratio_str, dr->dr_buffer_empty_ratio, PNL);

	tmp = (u_short) ( (dr->dr_bus_inactivity_limit_1 << 8) +
			  (dr->dr_bus_inactivity_limit_0) );
	PrintNumeric (bus_inactivity_limit_str, tmp, PNL);

	tmp = (u_short) ( (dr->dr_disconnect_time_limit_1 << 8) +
			  (dr->dr_disconnect_time_limit_0) );
	PrintNumeric (disconnect_time_limit_str, tmp, PNL);

	tmp = (u_short) ( (dr->dr_connect_time_limit_1 << 8) +
			  (dr->dr_connect_time_limit_0) );
	PrintNumeric (connect_time_limit_str, tmp, PNL);

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    tmp = (u_short) ( (dr->dr_maximum_burst_size_1 << 8) +
			      (dr->dr_maximum_burst_size_0) );
	    PrintNumeric (maximum_burst_size_str, tmp, PNL);

	    PrintNumeric (dtdc_str, dr->dr_dtdc, PNL);
	}

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetRecoPage() - Set The Disconnect/Reconnect Control Page.		*
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
SetRecoPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct reco_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct reco_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct reco_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyRecoPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	return (SelectRecoPage (de, SaveModeParameters));
}

/************************************************************************
 *									*
 * ChangeRecoPage() - Change The Disconnect/Reconnect Control Page.	*
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
ChangeRecoPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct reco_params ch_reco_params;
	struct reco_params ms_reco_params;
	register struct reco_params *ch = &ch_reco_params;
	register struct reco_params *de;
	register struct reco_params *ms = &ms_reco_params;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	register struct scu_device *scu = ScuDevice;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct reco_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct reco_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * First, find out what fields are changeable.
	 */
	if ((status = SenseRecoPage (ch, PCF_CHANGEABLE)) != 0) {
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
	if ((status = SenseRecoPage (ms, pcf)) != 0) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page2, &ms->page2)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	chbits = (u_long) ch->page2.dr_buffer_full_ratio;
	if (chbits) {
	    deflt = (u_long) (ms->page2.dr_buffer_full_ratio & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, buffer_full_ratio_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page2.dr_buffer_full_ratio = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (buffer_full_ratio_str);
	    }
	}

	chbits = (u_long) ch->page2.dr_buffer_empty_ratio;
	if (chbits) {
	    deflt = (ms->page2.dr_buffer_empty_ratio & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, buffer_empty_ratio_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page2.dr_buffer_empty_ratio = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (buffer_empty_ratio_str);
	    }
	}

	chbits = (u_long) ( (ch->page2.dr_bus_inactivity_limit_1 << 8) +
			    (ch->page2.dr_bus_inactivity_limit_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page2.dr_bus_inactivity_limit_1 << 8) +
				(ms->page2.dr_bus_inactivity_limit_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			bus_inactivity_limit_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page2.dr_bus_inactivity_limit_1 = (u_char) (answer >> 8);
	    ms->page2.dr_bus_inactivity_limit_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (bus_inactivity_limit_str);
	    }
	}

	chbits = (u_long) ( (ch->page2.dr_disconnect_time_limit_1 << 8) +
			    (ch->page2.dr_disconnect_time_limit_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page2.dr_disconnect_time_limit_1 << 8) +
				(ms->page2.dr_disconnect_time_limit_0)) &
			       chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			disconnect_time_limit_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page2.dr_disconnect_time_limit_1 = (u_char) (answer >> 8);
	    ms->page2.dr_disconnect_time_limit_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (disconnect_time_limit_str);
	    }
	}

	chbits = (u_long) ( (ch->page2.dr_connect_time_limit_1 << 8) +
			    (ch->page2.dr_connect_time_limit_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page2.dr_connect_time_limit_1 << 8) +
				(ms->page2.dr_connect_time_limit_0)) &
			       chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			connect_time_limit_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page2.dr_connect_time_limit_1 = (u_char) (answer >> 8);
	    ms->page2.dr_connect_time_limit_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (connect_time_limit_str);
	    }
	}

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    chbits = (u_long) ( (ch->page2.dr_maximum_burst_size_1 << 8) +
			        (ch->page2.dr_maximum_burst_size_0) );
	    if (chbits) {
		deflt = (u_long) ( ((ms->page2.dr_maximum_burst_size_1 << 8) +
				    (ms->page2.dr_maximum_burst_size_0)) &
				    chbits );
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			maximum_burst_size_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page2.dr_maximum_burst_size_1 = (u_char) (answer >> 8);
		ms->page2.dr_maximum_burst_size_0 = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (maximum_burst_size_str);
		}
	    }

	    chbits = (u_long) ch->page2.dr_dtdc;
	    if (chbits) {
		deflt = (u_long) (ms->page2.dr_dtdc & chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE, dtdc_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page2.dr_dtdc = (u_char) answer;
	    } else {
		if (DebugFlag) {
		   FieldNotChangeable (dtdc_str);
		}
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectRecoPage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyRecoPage() - Verify The Disconnect/Reconnect Control Page.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		ph = The page header (if any).				*
 * 		verify_verbose = Display verification messages.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
static int
VerifyRecoPage (ce, ke, ph, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
int verify_verbose;
{
	struct reco_params ch_reco_params;
	register struct reco_params *ch = &ch_reco_params;
	register struct reco_params *de;
	int status;
	u_char pcf = PCF_DEFAULT;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct reco_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct reco_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseRecoPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page2.page_header.page_code != PageCode) {
	    if ((status = SenseRecoPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
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
	 * Verify "Buffer Full Ratio" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page2.dr_buffer_full_ratio;
	if (ch_field) {
	    if (dt->dtype_moptions & SUP_BUF_FULL_RATIO) {
		if (dt->dtype_buffer_full_ratio <= ch_field) {
		    field_changed = 1;
		    de->page2.buffer_full_ratio = dt->dtype_buffer_full_ratio;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", buffer_full_ratio_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					buffer_full_ratio_str,
					de->page2.dr_buffer_full_ratio);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (buffer_full_ratio_str);
	    }
	}

	/*
	 * Verify "Buffer Empty Ratio" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page2.dr_buffer_empty_ratio;
	if (ch_field) {
	    if (dt->dtype_moptions & SUP_BUF_EMPTY_RATIO) {
		if (dt->dtype_buffer_empty_ratio <= ch_field) {
		    field_changed = 1;
		    de->page2.dr_buffer_empty_ratio = dt->dtype_buffer_empty_ratio;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", buffer_empty_ratio_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					buffer_empty_ratio_str,
					de->page2.dr_buffer_empty_ratio);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (buffer_empty_ratio_str);
	    }
	}

	/*
	 * Verify "Bus Inactivity Limit" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page2.dr_inactivity_limit);
	if (ch_field) {
	    if (dt->dtype_moptions & SUP_BUS_INACT_LIMIT) {
		if (dt->dtype_bus_inactivity <= ch_field) {
		    field_changed = 1;
		    de->page2.dr_inactivity_limit =
					htoccs2(dt->dtype_bus_inactivity);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", bus_inactivity_limit_str, too_large_str);
		    }
		}
	    }
	    if (!field_changed) {
		field_changed = 1;
		de->page2.dr_inactivity_limit =
					htoccs2(DEF_BUS_INACTIVITY_LIMIT);
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					bus_inactivity_limit_str,
				ccstoh2(de->page2.dr_inactivity_limit));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (bus_inactivity_limit_str);
	    }
	}

	/*
	 * Verify "Disconnect Time Limit" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page2.dr_disconnect_time_limit);
	if (ch_field) {
	    if (dt->dtype_moptions & SUP_DISC_TIME_LIMIT) {
		if (dt->dtype_disconnect_time <= ch_field) {
		    field_changed = 1;
		    de->page2.dr_disconnect_time_limit =
					htoccs2(dt->dtype_disconnect_time);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", disconnect_time_limit_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					disconnect_time_limit_str,
				ccstoh2(de->page2.dr_disconnect_time_limit));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (disconnect_time_limit_str);
	    }
	}

	/*
	 * Verify "Connect Time Limit" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page2.dr_connect_time_limit);
	if (ch_field) {
	    if (dt->dtype_moptions & SUP_CONN_TIME_LIMIT) {
		if (dt->dtype_connect_time <= ch_field) {
		    field_changed = 1;
		    de->page2.dr_connect_time_limit =
					htoccs2(dt->dtype_connect_time);
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", connect_time_limit_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					connect_time_limit_str,
				ccstoh2(de->page2.dr_connect_time_limit));
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (connect_time_limit_str);
	    }
	}
#endif notdef
	return (status);
}

/************************************************************************
 *									*
 * SenseRecoPage() - Sense The Disconnect/Reconnect Control Page.	*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseRecoPage (ms, pcf)
struct reco_params *ms;
u_char pcf;
{
	return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectRecoPage() - Select The Disconnect/Reconnect Control Page.	*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectRecoPage (ms, smp)
struct reco_params *ms;
u_char smp;
{
	struct reco_params se_reco_params;
	register struct reco_params *se = &se_reco_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
