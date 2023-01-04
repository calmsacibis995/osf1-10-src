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
static char *rcsid = "@(#)$RCSfile: cdrom_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:03:33 $";
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
 * File:	cdrom_page.c
 * Author:	Robin T. Miller
 * Date:	December 17, 1990
 *
 * Description:
 *	This file contains the functions for the deivce parameter page.
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
int	SetCdromPage(), ShowCdromPage(), ChangeCdromPage(),
	VerifyCdromPage(), SenseCdromPage(), SelectCdromPage();

static u_char PageCode = CDROM_DEVICE_PAGE;

struct mode_page_funcs cdrom_page_funcs = {
	CDROM_DEVICE_PAGE,		/* The mode page code.		*/
	SetCdromPage,			/* Set mode page function.	*/
	ShowCdromPage,			/* Show mode page function.	*/
	ChangeCdromPage,		/* Change mode page function.	*/
	VerifyCdromPage,		/* Verify mode page function.	*/
	SenseCdromPage,			/* Sense mode page function.	*/
	SelectCdromPage			/* Select mode page function.	*/
};

struct cdrom_params {				/* Device Parameters.	*/
	struct mode_page_header header;
	struct scsi_cdrom_device page0d;
};

static char *hdr_str =			"CD-ROM";
static char *page_str =			"Page D";
static char *inactive_timer_str =	"Inactivity Timer Multiplier";
static char *num_su_per_mu_str =	"Number of S-units per M-units";
static char *num_fu_per_su_str =	"Number of F-units per S-units";

/************************************************************************
 *									*
 * ShowCdromPage() - Show The CD-ROM Device Parameters.			*
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
ShowCdromPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct cdrom_params ms_cdrom_params;
	register struct cdrom_params *ms = &ms_cdrom_params;
	register struct scsi_cdrom_device *cd = &ms_cdrom_params.page0d;
	int status = SUCCESS;
	u_long tmp;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseCdromPage (ms, PageControlField)) != 0) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    cd = (struct scsi_cdrom_device *) ph;
	}

	ShowPageHeader (&cd->page_header);

	PrintNumeric (inactive_timer_str, cd->cd_inactive_timer, PNL);

	tmp = (u_long) ( (cd->cd_num_su_per_mu_1 << 8) +
			 (cd->cd_num_su_per_mu_0) );
	PrintNumeric (num_su_per_mu_str, tmp, PNL);

	tmp = (u_long) ( (cd->cd_num_fu_per_su_1 << 8) +
			 (cd->cd_num_fu_per_su_0) );
	PrintNumeric (num_fu_per_su_str, tmp, PNL);

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetCdromPage() - Set The CD-ROM Device Parameters.			*
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
SetCdromPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct cdrom_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct cdrom_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct cdrom_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyCdromPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters Page (%s)\n", hdr_str, page_str);
	}
	if ((status = SelectCdromPage (de, SaveModeParameters)) != 0) {
	    return (status);
	}
	return (status);
}

/************************************************************************
 *									*
 * ChangeCdromPage() - Change The CD-ROM Device Parameters.		*
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
ChangeCdromPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct cdrom_params ch_cdrom_params;
	struct cdrom_params ms_cdrom_params;
	register struct cdrom_params *ch = &ch_cdrom_params;
	register struct cdrom_params *de;
	register struct cdrom_params *ms = &ms_cdrom_params;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct cdrom_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct cdrom_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseCdromPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseCdromPage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page0d, &ms->page0d)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	chbits = (u_long) ch->page0d.cd_inactive_timer;
	if (chbits) {
	    deflt = (u_long) (ms->page0d.cd_inactive_timer & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE, inactive_timer_str,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0d.cd_inactive_timer = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (inactive_timer_str);
	    }
	}

	chbits = (u_long) ( (ch->page0d.cd_num_su_per_mu_1 << 8) +
			    (ch->page0d.cd_num_su_per_mu_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page0d.cd_num_su_per_mu_1 << 8) +
				(ms->page0d.cd_num_su_per_mu_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			num_su_per_mu_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0d.cd_num_su_per_mu_1 = (u_char) (answer >> 8);
	    ms->page0d.cd_num_su_per_mu_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (num_su_per_mu_str);
	    }
	}

	chbits = (u_long) ( (ch->page0d.cd_num_fu_per_su_1 << 8) +
			    (ch->page0d.cd_num_fu_per_su_0) );
	if (chbits) {
	    deflt = (u_long) ( ((ms->page0d.cd_num_fu_per_su_1 << 8) +
				(ms->page0d.cd_num_fu_per_su_0)) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			num_fu_per_su_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page0d.cd_num_fu_per_su_1 = (u_char) (answer >> 8);
	    ms->page0d.cd_num_fu_per_su_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (num_fu_per_su_str);
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectCdromPage (ms, SaveModeParameters)) == SUCCESS) {
	    *de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyCdromPage() - Verify The CD-ROM Device Parameters.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		se = The mode page to set (if any).			*
 * 		verify_verbose = Display verification messages.		*
 *									*
 * Implicit Inputs:							*
 *		cur_dtype = The current device type information.	*
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
VerifyCdromPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_cdrom_device *se;
int verify_verbose;
{
	struct cdrom_params ch_cdrom_params;
	struct cdrom_params vd_cdrom_params;
	register struct cdrom_params *ch = &ch_cdrom_params;
	register struct cdrom_params *de;
	register struct cdrom_params *vd = &vd_cdrom_params;
	int status;
	u_char pcf;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct cdrom_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct cdrom_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseCdromPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page0d.page_header.page_code != PageCode) {
	    if ((status = SenseCdromPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_cdrom_device *) 0) {
	    register struct scsi_cdrom_device *cd = &de->page0d;

	    *cd = *se;
	}

	/*
	 * Get the vendor default values for sanity checks.
	 */
	if ((status = SenseCdromPage (vd, PCF_DEFAULT)) != SUCCESS) {
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
 * SenseCdromPage() - Sense The CD-ROM Device Parameters Page.		*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseCdromPage (ms, pcf)
struct cdrom_params *ms;
u_char pcf;
{
    return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectCdromPage() - Select The CD-ROM Device Parameters Page.	*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectCdromPage (ms, smp)
struct cdrom_params *ms;
u_char smp;
{
	struct cdrom_params se_cdrom_params;
	register struct cdrom_params *se = &se_cdrom_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
