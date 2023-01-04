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
static char *rcsid = "@(#)$RCSfile: unknown_page.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/15 20:57:19 $";
#endif

/*
 * File:	unknown_page.c
 * Author:	Robin T. Miller
 * Date:	November 1, 1993
 *
 * Description:
 *	This file contains the functions for unknown mode pages.
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
int	SetUnknownPage(), ShowUnknownPage(), ChangeUnknownPage(),
	VerifyUnknownPage(), SenseUnknownPage(), SelectUnknownPage();

u_char PageCode;

struct mode_page_funcs unknown_page_funcs = {
	0,				/* The mode page code (unused).	*/
	SetUnknownPage,			/* Set mode page function.	*/
	ShowUnknownPage,		/* Show mode page function.	*/
	ChangeUnknownPage,		/* Change mode page function.	*/
	VerifyUnknownPage,		/* Verify mode page function.	*/
	SenseUnknownPage,		/* Sense mode page function.	*/
	SelectUnknownPage		/* Select mode page function.	*/
};

/*
 * NOTE: Restriction until support for larger mode pages is implemented.
 */
#define PAGE_DATA_SIZE	(MODE_PAGES_SIZE - \
	sizeof(struct mode_page_header) - sizeof(struct scsi_page_header))

struct scsi_unknown_page {
	struct	scsi_page_header page_header;
	u_char	page_data[PAGE_DATA_SIZE];
};

struct unknown_params {				/* DEC Specific Page.	*/
	struct mode_page_header header;
	struct scsi_unknown_page page;
};

static char *hdr_str =			"Unknown Page";
static char *page_str =			"Page %X";
static char page_name[sizeof(page_str) + 3];

/************************************************************************
 *									*
 * ShowUnknownPage() - Show Unknown Mode Page Parameters.		*
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
ShowUnknownPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct unknown_params ms_unknown_params;
	register struct unknown_params *ms = &ms_unknown_params;
	register struct scsi_unknown_page *up = &ms_unknown_params.page;
	int status = SUCCESS;
	int i, length;
	u_char *bp;
	char buf[12];

	(void) sprintf (page_name, page_str, PageCode);
	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseUnknownPage (ms, PageControlField)) != 0) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_name, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_name, PageControlField);
	    up = (struct scsi_unknown_page *) ph;
	}

	ShowPageHeader (&up->page_header);

	/*
	 * Start byte offset past the page header (code & length).
	 */
	for (i = 2, length = up->page_header.page_length, bp = (u_char *)up->page_data;
				(length > 0) ; --length) {
	    (void) sprintf (buf, "Byte %d", i++);
	    PrintHex (buf, *bp++, PNL);
	}

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetUnknownPage() - Set The Unknown Mode Page Parameters.		*
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
SetUnknownPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct unknown_params *de;
	int status;

	(void) sprintf (page_name, page_str, PageCode);
	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct unknown_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct unknown_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyUnknownPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_name);
	}
	if ((status = SelectUnknownPage (de, SaveModeParameters)) != 0) {
	    return (status);
	}
	return (status);
}

/************************************************************************
 *									*
 * ChangeUnknownPage() - Change The Unknown Mode Page Parameters.	*
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
ChangeUnknownPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct unknown_params ch_unknown_params;
	struct unknown_params ms_unknown_params;
	register struct unknown_params *ch = &ch_unknown_params;
	register struct unknown_params *de;
	register struct unknown_params *ms = &ms_unknown_params;
	register struct scsi_unknown_page *up = &ms_unknown_params.page;
	register struct parser_control *pc = &ParserControl;
	struct parser_range range;
	u_long answer, chbits, deflt;
	int status;
	u_char pcf;
	int i, length;
	u_char *chp, *msp;
	char buf[12];

	(void) sprintf (page_name, page_str, PageCode);
	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct unknown_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct unknown_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseUnknownPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseUnknownPage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page, &ms->page)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_name, pcf);

	range.pr_min = 0;
	/*
	 * Start byte offset past the page header (code & length).
	 */
	for (i = 2, length = up->page_header.page_length,
			chp = (u_char *)ch->page.page_data,
			msp = (u_char *)ms->page.page_data;
				(length > 0) ; --length, chp++, msp++) {
	    (void) sprintf (buf, "Byte %d", i++);
	    chbits = *chp;
	    if (chbits) {
		deflt = (*msp & chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_HEX, buf,
			':', &deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		*msp = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (buf);
		}
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectUnknownPage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyUnknownPage() - Verify The Unknown Mode Page Parameters.	*
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
VerifyUnknownPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_unknown_page *se;
int verify_verbose;
{
	struct unknown_params ch_unknown_params;
	struct unknown_params vd_unknown_params;
	register struct unknown_params *ch = &ch_unknown_params;
	register struct unknown_params *de;
	register struct unknown_params *vd = &vd_unknown_params;
	int status;
	u_char pcf;

	(void) sprintf (page_name, page_str, PageCode);
	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct unknown_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct unknown_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseUnknownPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ( (de->page.page_header.page_code != PageCode) ||
	     (de->page.page_header.page_length == 0) ) {
	    if ((status = SenseUnknownPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_unknown_page *) 0) {
	    register struct scsi_unknown_page *up = &de->page;

	    *up = *se;
	}

	if ((status = SetupChangeable (&ch->page, &de->page)) != SUCCESS) {
	    return (status);
	}

	/*
	 * Get the vendor default values for sanity checks.
	 */
	if ((status = SenseUnknownPage (vd, PCF_DEFAULT)) != SUCCESS) {
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
	    ShowParamHeader (verifying_str, hdr_str, page_name, pcf);
	}
#endif /* notdef */
	return (status);
}

/************************************************************************
 *									*
 * SenseUnknownPage() - Sense The Unknown Mode Page.			*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseUnknownPage (ms, pcf)
struct unknown_params *ms;
u_char pcf;
{
    return (SensePage (ms, pcf, PageCode, sizeof *ms));
}


/************************************************************************
 *									*
 * SelectUnknownPage() - Select The Unknown Mode Page.			*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectUnknownPage (ms, smp)
struct unknown_params *ms;
u_char smp;
{
	struct unknown_params se_unknown_params;
	register struct unknown_params *se = &se_unknown_params;

	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, sizeof *se, smp));
}
