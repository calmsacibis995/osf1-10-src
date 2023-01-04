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
static char *rcsid = "@(#)$RCSfile: dec_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:07:21 $";
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
 * File:	dec_page.c
 * Author:	Robin T. Miller
 * Date:	December 20, 1990
 *
 * Description:
 *	This file contains the functions for the DEC specific page.
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
int	SetDecPage(), ShowDecPage(), ChangeDecPage(),
	VerifyDecPage(), SenseDecPage(), SelectDecPage();

static u_char PageCode = DEC_SPECIFIC_PAGE;

struct mode_page_funcs dec_page_funcs = {
	DEC_SPECIFIC_PAGE,		/* The mode page code.		*/
	SetDecPage,			/* Set mode page function.	*/
	ShowDecPage,			/* Show mode page function.	*/
	ChangeDecPage,			/* Change mode page function.	*/
	VerifyDecPage,			/* Verify mode page function.	*/
	SenseDecPage,			/* Sense mode page function.	*/
	SelectDecPage			/* Select mode page function.	*/
};

struct dec_params {				/* DEC Specific Page.	*/
	struct mode_page_header header;
	struct scsi_dec_specific page25;
};

static char *hdr_str =			"DEC Specific";
static char *page_str =			"Page 25";
static char *spd_str =			"Spin-up Disable";

/************************************************************************
 *									*
 * ShowDecPage() - Show The DEC Specific Parameters.			*
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
ShowDecPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct dec_params ms_dec_params;
	register struct dec_params *ms = &ms_dec_params;
	register struct scsi_dec_specific *ds = &ms_dec_params.page25;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseDecPage (ms, PageControlField)) != 0) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    ds = (struct scsi_dec_specific *) ph;
	}

	ShowPageHeader (&ds->page_header);

	PrintNumeric (spd_str, ds->ds_spd, PNL);

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetDecPage() - Set The CD-ROM Dec Control Parameters.		*
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
SetDecPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct dec_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct dec_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct dec_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyDecPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	if ((status = SelectDecPage (de, SaveModeParameters)) != 0) {
	    return (status);
	}
	return (status);
}

/************************************************************************
 *									*
 * ChangeDecPage() - Change The DEC Specific Parameters.		*
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
ChangeDecPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct dec_params ch_dec_params;
	struct dec_params ms_dec_params;
	register struct dec_params *ch = &ch_dec_params;
	register struct dec_params *de;
	register struct dec_params *ms = &ms_dec_params;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct dec_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct dec_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseDecPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseDecPage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page25, &ms->page25)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	chbits = (u_long) ch->page25.ds_spd;
	if (chbits) {
	    deflt = (u_long) (ms->page25.ds_spd & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, spd_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page25.ds_spd = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (spd_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectDecPage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyDecPage() - Verify The DEC Specific Parameters.		*
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
VerifyDecPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_dec_specific *se;
int verify_verbose;
{
	struct dec_params ch_dec_params;
	struct dec_params vd_dec_params;
	register struct dec_params *ch = &ch_dec_params;
	register struct dec_params *de;
	register struct dec_params *vd = &vd_dec_params;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct dec_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct dec_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseDecPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page25.page_header.page_code != PageCode) {
	    if ((status = SenseDecPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_dec_specific *) 0) {
	    register struct scsi_dec_specific *ds = &de->page25;

	    *ds = *se;
	}

	if ((status = SetupChangeable (&ch->page25, &de->page25)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Get the vendor default values for sanity checks.
	 */
	if ((status = SenseDecPage (vd, PCF_DEFAULT)) != SUCCESS) {
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
 * SenseDecPage() - Sense The DEC Specific Page.			*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseDecPage (ms, pcf)
struct dec_params *ms;
u_char pcf;
{
    return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectDecPage() - Select The DEC Specific Page.			*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectDecPage (ms, smp)
struct dec_params *ms;
u_char smp;
{
	struct dec_params se_dec_params;
	register struct dec_params *se = &se_dec_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
