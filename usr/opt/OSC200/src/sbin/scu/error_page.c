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
static char *rcsid = "@(#)$RCSfile: error_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:12:01 $";
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
 * File:	error_page.c
 * Author:	Robin T. Miller
 * Date:	December 19, 1990
 *
 * Description:
 *	This file contains the functions to show the error control page.
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
int EnableRecovery (struct scu_device *scu);
int DisableRecovery (struct scu_device *scu);
int	SetErrorPage(), ShowErrorPage(), ChangeErrorPage(),
	VerifyErrorPage(), SenseErrorPage(), SelectErrorPage();

static u_char PageCode = ERROR_RECOVERY_PAGE;

struct mode_page_funcs error_page_funcs = {
	ERROR_RECOVERY_PAGE,		/* The mode page code.		*/
	SetErrorPage,			/* Set mode page function.	*/
	ShowErrorPage,			/* Show mode page function.	*/
	ChangeErrorPage,		/* Change mode page function.	*/
	VerifyErrorPage,		/* Verify mode page function.	*/
	SenseErrorPage,			/* Sense mode page function.	*/
	SelectErrorPage			/* Select mode page function.	*/
};

struct error_params {				/* Error Recovery.	*/
	struct mode_page_header header;
	struct scsi_error_recovery page1;
};

static char *hdr_str =			"Error Recovery";
static char *page_str =			"Page 1";
static char *dcr_str =			"Disable Correction (DCR)";
static char *dte_str =			"Disable Transfer on Error (DTE)";
static char *per_str =			"Post Recoverable Error (PER)";
static char *eec_str = 			"Enable Early Correction (EEC)";
static char *rc_str =			"Read Continuous (RC)";
static char *tb_str =			"Transfer Block (TB)";
static char *arre_str =			"Automatic Read Allocation (ARRE)";
static char *awre_str =			"Automatic Write Allocation (AWRE)";
static char *retry_count_str =		"Retry Count";
static char *read_retry_count_str =	"Read Retry Count";
static char *correction_span_str =	"Correction Span";
static char *head_offset_count_str =	"Head Offset Count";
static char *strobe_offset_count_str =	"Data Strobe Offset Count";
static char *write_retry_count_str =	"Write Retry Count";
static char *recovery_time_limit_str =	"Recovery Time Limit";

/************************************************************************
 *									*
 * ShowErrorPage() - Show The Error Recovery Parameters.		*
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
ShowErrorPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct error_params ms_error_params;
	register struct error_params *ms = &ms_error_params;
	register struct scsi_error_recovery *er = &ms_error_params.page1;
	register struct scu_device *scu = ScuDevice;
	u_long tmp;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseErrorPage (ms, PageControlField)) != 0) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    er = (struct scsi_error_recovery *) ph;
	}

	ShowPageHeader (&er->page_header);

	if (PageControlField == PCF_CHANGEABLE) {
	    PrintNumeric (dcr_str, er->er_dcr, PNL);
	} else {
	    PrintAscii (dcr_str, yesno_table[er->er_dcr], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {
	    PrintNumeric (dte_str, er->er_dte, PNL);
	} else {
	    PrintAscii (dte_str, yesno_table[er->er_dte], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {
	    PrintNumeric (per_str, er->er_per, PNL);
	} else {
	    PrintAscii (per_str, yesno_table[er->er_per], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {
	    PrintNumeric (eec_str, er->er_eec, PNL);
	} else {
	    PrintAscii (eec_str, yesno_table[er->er_eec], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {
	    PrintNumeric (rc_str, er->er_rc, PNL);
	} else {
	    PrintAscii (rc_str, yesno_table[er->er_rc], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {
	    PrintNumeric (tb_str, er->er_tb, PNL);
	} else {
	    PrintAscii (tb_str, yesno_table[er->er_tb], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {
	    PrintNumeric (arre_str, er->er_arre, PNL);
	} else {
	    PrintAscii (arre_str, yesno_table[er->er_arre], PNL);
	}

	if (PageControlField == PCF_CHANGEABLE) {
	    PrintNumeric (awre_str, er->er_awre, PNL);
	} else {
	    PrintAscii (awre_str, yesno_table[er->er_awre], PNL);
	}

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    PrintNumeric (read_retry_count_str, er->er_read_retry_count, PNL);
	} else {
	    PrintNumeric (retry_count_str, er->er_retry_count, PNL);
	}

	PrintNumeric (correction_span_str, er->er_correction_span, PNL);

	PrintNumeric (head_offset_count_str, er->er_head_offset_count, PNL);

	PrintNumeric (strobe_offset_count_str, er->er_strobe_offset_count, PNL);

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    PrintNumeric (write_retry_count_str,
				er->er_write_retry_count, PNL);

	    tmp = (u_long) ( (er->er_recovery_time_limit_1 << 8) +
			     (er->er_recovery_time_limit_0) );
	    PrintNumeric (recovery_time_limit_str, tmp, PNL);
	} else {
	    PrintNumeric (recovery_time_limit_str,
				er->er_recovery_time_limit, PNL);
	}

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetErrorPage() - Set The Error Recovery Parameters.			*
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
SetErrorPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct error_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct error_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct error_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyErrorPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	return (SelectErrorPage (de, SaveModeParameters));
}

/************************************************************************
 *									*
 * ChangeErrorPage() - Change The Error Recovery Parameters.		*
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
ChangeErrorPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct error_params ch_error_params;
	struct error_params ms_error_params;
	register struct error_params *ch = &ch_error_params;
	register struct error_params *de;
	register struct error_params *ms = &ms_error_params;
	register struct parser_control *pc = &ParserControl;
	register struct scu_device *scu = ScuDevice;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct error_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct error_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * First, find out what fields are changeable.
	 */
	if ((status = SenseErrorPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseErrorPage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page1, &ms->page1)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	if (ch->page1.er_dcr) {
	    deflt = ms->page1.er_dcr;
	    range.pr_max = ch->page1.er_dcr;
	    if ((status = PromptUser (pc, TYPE_VALUE, dcr_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_dcr = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (dcr_str);
	    }
	}

	if (ch->page1.er_dte) {
	    deflt = ms->page1.er_dte;
	    range.pr_max = ch->page1.er_dte;
	    if ( (status = PromptUser (pc, TYPE_VALUE, dte_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_dte = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (dte_str);
	    }
	}

	if (ch->page1.er_per) {
	    deflt = ms->page1.er_per;
	    range.pr_max = ch->page1.er_per;
	    if ( (status = PromptUser (pc, TYPE_VALUE, per_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_per = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (per_str);
	    }
	}

	if (ch->page1.er_eec) {
	    deflt = ms->page1.er_eec;
	    range.pr_max = ch->page1.er_eec;
	    if ( (status = PromptUser (pc, TYPE_VALUE, eec_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_eec = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (eec_str);
	    }
	}

	if (ch->page1.er_rc) {
	    deflt = ms->page1.er_rc;
	    range.pr_max = ch->page1.er_rc;
	    if ( (status = PromptUser (pc, TYPE_VALUE, rc_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_rc = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (rc_str);
	    }
	}

	if (ch->page1.er_tb) {
	    deflt = ms->page1.er_tb;
	    range.pr_max = ch->page1.er_tb;
	    if ( (status = PromptUser (pc, TYPE_VALUE, tb_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_tb = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (tb_str);
	    }
	}

	if (ch->page1.er_arre) {
	    deflt = ms->page1.er_arre;
	    range.pr_max = ch->page1.er_arre;
	    if ( (status = PromptUser (pc, TYPE_VALUE, arre_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_arre = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (arre_str);
	    }
	}

	if (ch->page1.er_awre) {
	    deflt = ms->page1.er_awre;
	    range.pr_max = ch->page1.er_awre;
	    if ( (status = PromptUser (pc, TYPE_VALUE, awre_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_awre = answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (awre_str);
	    }
	}

	/*
	 * Even though these are the same field offset, I want to present
	 * the retry count appropriately to the user.
	 */
	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    chbits = (u_long) ch->page1.er_read_retry_count;
	    if (chbits) {
		deflt = (u_long) (ms->page1.er_read_retry_count & chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			read_retry_count_str, ':', &deflt,
			&answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page1.er_read_retry_count = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (read_retry_count_str);
		}
	    }
	} else {
	    chbits = (u_long) ch->page1.er_retry_count;
	    if (chbits) {
		deflt = (u_long) (ms->page1.er_retry_count & chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			retry_count_str, ':', &deflt,
			&answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page1.er_retry_count = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (retry_count_str);
		}
	    }
	}

	chbits = (u_long) ch->page1.er_correction_span;
	if (chbits) {
	    deflt = (u_long) (ms->page1.er_correction_span & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, correction_span_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_correction_span = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (correction_span_str);
	    }
	}

	chbits = (u_long) ch->page1.er_head_offset_count;
	if (chbits) {
	    deflt = (u_long) (ms->page1.er_head_offset_count & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, head_offset_count_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_head_offset_count = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (head_offset_count_str);
	    }
	}

	chbits = (u_long) ch->page1.er_strobe_offset_count;
	if (chbits) {
	    deflt = (u_long) (ms->page1.er_strobe_offset_count & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			strobe_offset_count_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page1.er_strobe_offset_count = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (strobe_offset_count_str);
	    }
	}

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    chbits = (u_long) ch->page1.er_write_retry_count;
	    if (chbits) {
		deflt = (u_long) (ms->page1.er_write_retry_count & chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			write_retry_count_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page1.er_write_retry_count = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (write_retry_count_str);
		}
	    }

	    chbits = (u_long) ( (ch->page1.er_recovery_time_limit_1 << 8) +
			        (ch->page1.er_recovery_time_limit_0) );
	    if (chbits) {
		deflt = (u_long) ( ((ms->page1.er_recovery_time_limit_1 << 8) +
				    (ch->page1.er_recovery_time_limit_0) ) &
					chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			recovery_time_limit_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page1.er_recovery_time_limit_1 = (u_char) (answer >> 8);
		ms->page1.er_recovery_time_limit_0 = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (recovery_time_limit_str);
		}
	    }
	} else {
	    chbits = (u_long) ch->page1.er_recovery_time_limit;
	    if (chbits) {
		deflt = (u_long) (ms->page1.er_recovery_time_limit & chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			recovery_time_limit_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page1.er_recovery_time_limit = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (recovery_time_limit_str);
		}
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectErrorPage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyErrorPage() - Verify The Error Recovery Parameters.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		se = The mode page to set (if any).			*
 * 		verify_verbose = Display verification messages.		*
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
VerifyErrorPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_error_recovery *se;
int verify_verbose;
{
	struct error_params ch_error_params;
	register struct error_params *ch = &ch_error_params;
	register struct error_params *de;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct error_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct error_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseErrorPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page1.page_header.page_code != PageCode) {
	    if ((status = SenseErrorPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_error_recovery *) 0) {
	    register struct scsi_error_recovery *er = &de->page1;

	    *er = *se;
	}

	if ((status = SetupChangeable (&ch->page1, &de->page1)) != SUCCESS) {
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
	 * Verify Error Recovery parameters.
	 */
	if (dt->dtype_options & SUP_ERROR_RECOVERY) {
	    if (ch->page1.er_dcr) {
		de->page1.er_dcr = (dt->dtype_error_recovery & ER_DCR) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, dcr_str, de->page1.er_dcr);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (dcr_str);
		}
	    }
	    if (ch->page1.er_dte) {
		de->page1.er_dte = (dt->dtype_error_recovery & ER_DTE) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, dte_str, de->page1.er_dte);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (dte_str);
		}
	    }
	    if (ch->page1.er_per) {
		de->page1.er_per = (dt->dtype_error_recovery & ER_PER) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, per_str, de->page1.er_per);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (per_str);
		}
	    }
	    if (ch->page1.er_eec) {
		de->page1.er_eec = (dt->dtype_error_recovery & ER_EEC) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, eec_str, de->page1.er_eec);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (eec_str);
		}
	    }
	    if (ch->page1.er_rc) {
		de->page1.er_rc = (dt->dtype_error_recovery & ER_RC) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, rc_str, de->page1.er_rc);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (rc_str);
		}
	    }
	    if (ch->page1.er_tb) {
		de->page1.er_tb = (dt->dtype_error_recovery & ER_TB) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, tb_str, de->page1.er_tb);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (tb_str);
		}
	    }
	    if (ch->page1.er_arre) {
		de->page1.er_arre = (dt->dtype_error_recovery & ER_ARRE) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, arre_str, de->page1.er_arre);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (arre_str);
		}
	    }
	    if (ch->page1.er_awre) {
		de->page1.er_awre = (dt->dtype_error_recovery & ER_AWRE) ? 1 : 0;
		if (verify_verbose) {
		    Printf (setting_str, awre_str, de->page1.er_awre);
		}
	    } else {
		if (verify_verbose) {
		    FieldNotChangeable (awre_str);
		}
	    }
	} else {
	    /*
	     * Set the default error recovery parameters.
	     */
	    if (verify_verbose) Printf (leaving_str, dcr_str, de->page1.er_dcr);
	    if (verify_verbose) Printf (leaving_str, dte_str, de->page1.er_dte);
	    bit_changed = 0;
	    if (ch->page1.er_per) {
		bit_changed = 1;
		de->page1.er_per = 1;	/* Report recoverable errors. */
	    }
	    if (verify_verbose) {
		Printf ( (bit_changed) ? setting_str : leaving_str,
						per_str, de->page1.er_per);
	    }
	    if (verify_verbose) Printf (leaving_str, eec_str, de->page1.er_eec);
	    if (verify_verbose) Printf (leaving_str, rc_str,  de->page1.er_rc);
	    bit_changed = 0;
	    if (ch->page1.er_tb) {
		bit_changed = 1;
		de->page1.er_tb = 1;	/* Transfer block with data error. */
	    }
	    if (verify_verbose) {
		Printf ( (bit_changed) ? setting_str : leaving_str,
						tb_str,  de->page1.er_tb);
	    }
	    if (verify_verbose) Printf (leaving_str, arre_str, de->page1.er_arre);
	    if (verify_verbose) Printf (leaving_str, awre_str, de->page1.er_awre);
	}

	/*
	 * Verify "Retry Count" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page1.er_retry_count;
	if (ch_field) {
	    if (dt->dtype_options & SUP_RETRY_COUNT) {
		if (dt->dtype_retry_count <= ch_field) {
		    field_changed = 1;
		    de->page1.er_retry_count = dt->dtype_retry_count;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", retry_count_str, too_large_str);
		    }
		}
	    }
	    if (!field_changed) {
		field_changed = 1;
		de->page1.er_retry_count = DEF_RETRY_COUNT;
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					retry_count_str,
					de->page1.er_retry_count);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (retry_count_str);
	    }
	}

	/*
	 * Verify "Correction Span" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page1.er_correction_span;
	if (ch_field) {
	    if (dt->dtype_options & SUP_CORRECTION_SPAN) {
		if (dt->dtype_correction_span <= ch_field) {
		    field_changed = 1;
		    de->page1.er_correction_span = dt->dtype_correction_span;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", correction_span_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					correction_span_str,
					de->page1.er_correction_span);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (correction_span_str);
	    }
	}

	/*
	 * Verify "Head Offset Count" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page1.er_head_offset_count;
	if (ch_field) {
	    if (dt->dtype_options & SUP_HEAD_OFFSET) {
		if (dt->dtype_head_offset <= ch_field) {
		    field_changed = 1;
		    de->page1.er_head_offset_count = dt->dtype_head_offset;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", head_offset_count_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					head_offset_count_str,
					de->page1.er_head_offset_count);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (head_offset_count_str);
	    }
	}

	/*
	 * Verify "Data Strobe Offset Count" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page1.er_strobe_offset_count;
	if (ch_field) {
	    if (dt->dtype_options & SUP_STROBE_OFFSET) {
		if (dt->dtype_strobe_offset <= ch_field) {
		    field_changed = 1;
		    de->page1.er_strobe_offset_count = dt->dtype_strobe_offset;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", strobe_offset_count_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					strobe_offset_count_str,
					de->page1.er_strobe_offset_count);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (strobe_offset_count_str);
	    }
	}

	/*
	 * Verify "Recovery Time Limit" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page1.er_recovery_time_limit;
	if (ch_field) {
	    if (dt->dtype_moptions & SUP_RECOVERY_TIME) {
		if (dt->dtype_recovery_time <= ch_field) {
		    field_changed = 1;
		    de->page1.er_recovery_time_limit = dt->dtype_recovery_time;
		} else {
		    if (verify_verbose) {
			Printf ("%s %s\n", recovery_time_limit_str, too_large_str);
		    }
		}
	    }
	    if (verify_verbose) {
		Printf ( (field_changed) ? setting_str : leaving_str,
					recovery_time_limit_str,
					de->page1.er_recovery_time_limit);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (recovery_time_limit_str);
	    }
	}
#endif
	return (status);
}

/************************************************************************
 *									*
 * SetRecovery() - Set The Error Recovery Parameters.			*
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
SetRecovery (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;

	if (RecoveryFlag == TRUE) {
		return ( EnableRecovery (scu) );
	} else {
		return ( DisableRecovery (scu) );
	}
}

/************************************************************************
 *									*
 * EnableRecovery() - Enable Error Recovery				*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
EnableRecovery (scu)
register struct scu_device *scu;
{
	struct error_params de_error_params;
	register struct error_params *de = &de_error_params;
	register struct error_params *ms;
	u_char pcf = PCF_CURRENT;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	ms = (struct error_params *) GetDefaultPage (PageCode, sizeof(*ms));
	if (ms == (struct error_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Get the default page parameters (unless already done).
	 */
	if (ms->page1.page_header.page_code != PageCode) {
	    if ((status = SenseErrorPage (ms, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * If error control was not saved, then get the saved page.
	 */
	if ( (scu->scu_error_control == 0) || (scu->scu_read_retrys == 0) ) {
	    if ((status = SenseErrorPage (de, PCF_DEFAULT)) != SUCCESS) {
		return (status);
	    }
	    scu->scu_error_control = de->page1.er_control;
	    if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
		scu->scu_read_retrys = de->page1.er_read_retry_count;
		scu->scu_write_retrys = de->page1.er_write_retry_count;
	    } else {
		scu->scu_read_retrys = de->page1.er_retry_count;
	    }
	}
	ms->page1.er_control = scu->scu_error_control;
	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    ms->page1.er_read_retry_count = scu->scu_read_retrys;
	    ms->page1.er_write_retry_count = scu->scu_write_retrys;
	} else {
	    ms->page1.er_retry_count = scu->scu_read_retrys;
	}

	return ( SetErrorRecovery (ms) );
}

/************************************************************************
 *									*
 * DisableRecovery() - Disable Error Recovery				*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DisableRecovery (scu)
register struct scu_device *scu;
{
	register struct error_params *ms;
	u_char pcf = PCF_CURRENT;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	ms = (struct error_params *) GetDefaultPage (PageCode, sizeof(*ms));
	if (ms == (struct error_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Get the default page parameters (unless already done).
	 */
	if (ms->page1.page_header.page_code != PageCode) {
	    if ((status = SenseErrorPage (ms, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	scu->scu_error_control = ms->page1.er_control;
	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    scu->scu_read_retrys = ms->page1.er_read_retry_count;
	    scu->scu_write_retrys = ms->page1.er_write_retry_count;
	} else {
	    scu->scu_read_retrys = ms->page1.er_retry_count;
	}
	ms->page1.er_control = (ER_DCR | ER_DTE | ER_PER | ER_TB);
	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    ms->page1.er_read_retry_count = 0;
	    ms->page1.er_write_retry_count = 0;
	} else {
	    ms->page1.er_retry_count = 0;
	}
	return ( SetErrorRecovery (ms) );
}

/************************************************************************
 *									*
 * SetErrorRecovery() - Set The Error Recovery Parameters.		*
 *									*
 * Description:								*
 *	This function verifies the error control bits being enabled	*
 * are supported by the disk controller.  If the bit isn't changeable,	*
 * we silently reset that bit before setting the page parameters.	*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SetErrorRecovery (ms)
struct error_params *ms;
{
	struct error_params ch_error_params;
	register struct error_params *ch = &ch_error_params;
	int status;

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseErrorPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page1, &ms->page1)) != SUCCESS) {
	    return (status);
	}
	return (SelectErrorPage (ms, MS_DONT_SAVE_PARMS));
}

/************************************************************************
 *									*
 * SenseErrorPage() - Sense The Error Recovery Parameters.		*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseErrorPage (ms, pcf)
struct error_params *ms;
u_char pcf;
{
	register struct scu_device *scu = ScuDevice;
	int mode_length;

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    mode_length = sizeof(*ms);
	} else {
	    mode_length = sizeof(ms->header) + ccs_error_page_length;
	}
	return (SensePage (ms, pcf, PageCode, mode_length));
}


/************************************************************************
 *									*
 * SelectErrorPage() - Select The Error Recovery Parameters.		*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectErrorPage (ms, smp)
register struct error_params *ms;
u_char smp;
{
	struct error_params se_error_params;
	register struct error_params *se = &se_error_params;
	register struct scu_device *scu = ScuDevice;
	int mode_length;

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    mode_length = sizeof(*se);
	} else {
	    mode_length = sizeof(se->header) + ccs_error_page_length;
	}
	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, mode_length, smp));
}
