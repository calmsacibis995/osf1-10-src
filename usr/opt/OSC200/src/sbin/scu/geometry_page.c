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
static char *rcsid = "@(#)$RCSfile: geometry_page.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/25 10:16:34 $";
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
 * File:	geometry_page.c
 * Author:	Robin T. Miller
 * Date:	December 19, 1990
 *
 * Description:
 *	This file contains the functions for the rigid disk geometry page.
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
int	SetGeometryPage(), ShowGeometryPage(), ChangeGeometryPage(),
	VerifyGeometryPage(), SenseGeometryPage(), SelectGeometryPage();

static u_char PageCode = DISK_GEOMETRY_PAGE;

struct mode_page_funcs geometry_page_funcs = {
	DISK_GEOMETRY_PAGE,		/* The mode page code.		*/
	SetGeometryPage,		/* Set mode page function.	*/
	ShowGeometryPage,		/* Show mode page function.	*/
	ChangeGeometryPage,		/* Change mode page function.	*/
	VerifyGeometryPage,		/* Verify mode page function.	*/
	SenseGeometryPage,		/* Sense mode page function.	*/
	SelectGeometryPage		/* Select mode page function.	*/
};

struct geometry_params {			/* Rigid Disk Geometry.	*/
	struct mode_page_header header;
	struct scsi_disk_geometry page4;
};

static char *hdr_str =			"Rigid Disk Geometry";
static char *page_str =			"Page 4";
static char *maximum_cylinders_str =	"Maximum Number of Cylinders";
static char *maximum_heads_str =	"Maximum Number of Heads";
static char *precomp_cyl_str =		"Write Precomp Starting Cylinder";
static char *current_cyl_str =		"Reduced Write Current Cylinder";
static char *step_rate_str =		"Drive Step Rate";
static char *landing_cyl_str =		"Landing Zone Cylinder";
static char *rpl_str =			"Rotational Position Locking";
static char *rotational_offset_str =	"Rotational Offset";
static char *medium_rotation_rate_str =	"Medium Rotation Rate";
#ifdef notdef
static char *too_large_str =		"value is too large!";
#endif notdef

/************************************************************************
 *									*
 * ShowGeometryPage() - Show The Rigid Disk Geometry Page.		*
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
ShowGeometryPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct geometry_params ms_geometry_params;
	register struct geometry_params *ms = &ms_geometry_params;
	register struct scsi_disk_geometry *dg = &ms_geometry_params.page4;
	register struct scu_device *scu = ScuDevice;
	u_long tmp;
	int status = SUCCESS;

	if (ph == (struct scsi_page_header *) 0) {
	    if ((status = SenseGeometryPage (ms, PageControlField)) != SUCCESS) {
		return (status);
	    }

	    ShowParamHeader ("", hdr_str, page_str, PageControlField);

	    if (DisplayModeParameters) {
		ShowModeParameters (&ms->header);
	    }
	} else {
	    ShowParamHeader ("", hdr_str, page_str, PageControlField);
	    dg = (struct scsi_disk_geometry *) ph;
	}

	ShowPageHeader (&dg->page_header);

	tmp = (u_long) ( ((u_long)dg->dg_cylinders_2 << 16) +
			 ((u_long)dg->dg_cylinders_1 << 8) +
			 ((u_long)dg->dg_cylinders_0) );
	PrintNumeric (maximum_cylinders_str, tmp, PNL);

	PrintNumeric (maximum_heads_str, dg->dg_heads, PNL);

	tmp = (u_long) ( ((u_long)dg->dg_precomp_cyl_2 << 16) +
			 ((u_long)dg->dg_precomp_cyl_1 << 8) +
			 ((u_long)dg->dg_precomp_cyl_0) );
	PrintNumeric (precomp_cyl_str, tmp, PNL);

	tmp = (u_long) ( ((u_long)dg->dg_current_cyl_2 << 16) +
			 ((u_long)dg->dg_current_cyl_1 << 8) +
			 ((u_long)dg->dg_current_cyl_0) );
	PrintNumeric (current_cyl_str, tmp, PNL);

	tmp = (u_long) ( ((u_long)dg->dg_step_rate_1 << 8) +
			 ((u_long)dg->dg_step_rate_0) );
	PrintNumeric (step_rate_str, tmp, PNL);

	tmp = (u_long) ( ((u_long)dg->dg_landing_cyl_2 << 16) +
			 ((u_long)dg->dg_landing_cyl_1 << 8) +
			 ((u_long)dg->dg_landing_cyl_0) );
	PrintNumeric (landing_cyl_str, tmp, PNL);

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {

	    PrintNumeric (rpl_str, dg->dg_rpl, PNL);

	    PrintNumeric (rotational_offset_str,
				dg->dg_rotational_offset, PNL);

	    tmp = (u_long) ( ((u_long)dg->dg_rpm_1 << 8) +
			     ((u_long)dg->dg_rpm_0) );
	    PrintNumeric (medium_rotation_rate_str, tmp, PNL);
	}

	if (!ProcessAllPages) Printf ("\n");
	return (status);
}

/************************************************************************
 *									*
 * SetGeometryPage() - Set The Rigid Disk Geometry Page.		*
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
SetGeometryPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	register struct geometry_params *de;
	int status;

	/*
	 * Get pointer to the default mode page parameters.
	 */
	de = (struct geometry_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct geometry_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Setup the defaults by verifying the parameters.
	 */
	if ((status = VerifyGeometryPage (ce, ke, ph, QUIET_MODE)) != SUCCESS) {
	    return (status);
	}

	if (DisplayVerbose) {
	    Printf ("Setting %s Parameters (%s)\n", hdr_str, page_str);
	}
	if ((status = SelectGeometryPage (de, SaveModeParameters)) != SUCCESS) {
	    return (status);
	}
	if (DisplayVerbose && (PageControlField == PCF_SAVED) && !Formatting) {
	    Printf ("Parameters are only saved after a FORMAT UNIT command.\n");
	}
	return (status);
}

/************************************************************************
 *									*
 * ChangeGeometryPage() - Change The Rigid Disk Geometry Page.		*
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
ChangeGeometryPage (ce, ke, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_page_header *ph;
{
	struct scu_device *scu = ScuDevice;
	struct geometry_params ch_geometry_params;
	struct geometry_params ms_geometry_params;
	register struct geometry_params *ch = &ch_geometry_params;
	register struct geometry_params *de;
	register struct geometry_params *ms = &ms_geometry_params;
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
	de = (struct geometry_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct geometry_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseGeometryPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if ((status = SenseGeometryPage (ms, pcf)) != SUCCESS) {
	    return (status);
	}

	if ((status = SetupChangeable (&ch->page4, &ms->page4)) != SUCCESS) {
	    return (status);
	}

	ShowParamHeader (changing_str, hdr_str, page_str, pcf);

	range.pr_min = 0;
	/*
	 * Only request those fields which are changeable.
	 */
	chbits = (u_long) ( ((u_long)ch->page4.dg_cylinders_2 << 16) +
			    ((u_long)ch->page4.dg_cylinders_1 << 8) +
			    ((u_long)ch->page4.dg_cylinders_0) );
	if (chbits) {
	    deflt = (u_long) ( (((u_long)ms->page4.dg_cylinders_2 << 16) +
			        ((u_long)ms->page4.dg_cylinders_1 << 8) +
			        ((u_long)ms->page4.dg_cylinders_0) ) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			maximum_cylinders_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page4.dg_cylinders_2 = (u_char) (answer >> 16);
	    ms->page4.dg_cylinders_1 = (u_char) (answer >> 8);
	    ms->page4.dg_cylinders_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (maximum_cylinders_str);
	    }
	}

	chbits = (u_long) ch->page4.dg_heads;
	if (chbits) {
	    deflt = (u_long) (ms->page4.dg_heads & chbits);
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			maximum_heads_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page4.dg_heads = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (maximum_heads_str);
	    }
	}

	chbits = (u_long) ( ((u_long)ch->page4.dg_precomp_cyl_2 << 16) +
			    ((u_long)ch->page4.dg_precomp_cyl_1 << 8) +
			    ((u_long)ch->page4.dg_precomp_cyl_0) );
	if (chbits) {
	    deflt = (u_long) ( (((u_long)ms->page4.dg_precomp_cyl_2 << 16) +
			        ((u_long)ms->page4.dg_precomp_cyl_1 << 8) +
			        ((u_long)ms->page4.dg_precomp_cyl_0) ) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			precomp_cyl_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page4.dg_precomp_cyl_2 = (u_char) (answer >> 16);
	    ms->page4.dg_precomp_cyl_1 = (u_char) (answer >> 8);
	    ms->page4.dg_precomp_cyl_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (precomp_cyl_str);
	    }
	}

	chbits = (u_long) ( ((u_long)ch->page4.dg_current_cyl_2 << 16) +
			    ((u_long)ch->page4.dg_current_cyl_1 << 8) +
			    ((u_long)ch->page4.dg_current_cyl_0) );
	if (chbits) {
	    deflt = (u_long) ( (((u_long)ms->page4.dg_current_cyl_2 << 16) +
			        ((u_long)ms->page4.dg_current_cyl_1 << 8) +
			        ((u_long)ms->page4.dg_current_cyl_0) ) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			current_cyl_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page4.dg_current_cyl_2 = (u_char) (answer >> 16);
	    ms->page4.dg_current_cyl_1 = (u_char) (answer >> 8);
	    ms->page4.dg_current_cyl_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (current_cyl_str);
	    }
	}

	chbits = (u_long) ( ((u_long)ch->page4.dg_step_rate_1 << 8) +
			    ((u_long)ch->page4.dg_step_rate_0) );
	if (chbits) {
	    deflt = (u_long) ( (((u_long)ms->page4.dg_step_rate_1 << 8) +
			        ((u_long)ms->page4.dg_step_rate_0) ) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			step_rate_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page4.dg_step_rate_1 = (u_char) (answer >> 8);
	    ms->page4.dg_step_rate_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (step_rate_str);
	    }
	}

	chbits = (u_long) ( ((u_long)ch->page4.dg_landing_cyl_2 << 16) +
			    ((u_long)ch->page4.dg_landing_cyl_1 << 8) +
			    ((u_long)ch->page4.dg_landing_cyl_0) );
	if (chbits) {
	    deflt = (u_long) ( (((u_long)ms->page4.dg_landing_cyl_2 << 16) +
			        ((u_long)ms->page4.dg_landing_cyl_1 << 8) +
			        ((u_long)ms->page4.dg_landing_cyl_0) ) &
				chbits );
	    range.pr_max = chbits;
	    if ( (status = PromptUser (pc, TYPE_VALUE,
			landing_cyl_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		return (status);
	    }
	    ms->page4.dg_landing_cyl_2 = (u_char) (answer >> 16);
	    ms->page4.dg_landing_cyl_1 = (u_char) (answer >> 8);
	    ms->page4.dg_landing_cyl_0 = (u_char) answer;
	} else {
	    if (DebugFlag) {
		FieldNotChangeable (landing_cyl_str);
	    }
	}

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    chbits = (u_long) ch->page4.dg_rpl;
	    if (chbits) {
		deflt = (u_long) (ms->page4.dg_rpl & chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			rpl_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page4.dg_rpl = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (rpl_str);
		}
	    }

	    chbits = (u_long) ch->page4.dg_rotational_offset;
	    if (chbits) {
		deflt = (u_long) (ms->page4.dg_rotational_offset & chbits);
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			rotational_offset_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page4.dg_rotational_offset = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (rotational_offset_str);
		}
	    }

	    chbits = (u_long) ( ((u_long)ch->page4.dg_rpm_1 << 8) +
			    ((u_long)ch->page4.dg_rpm_0) );
	    if (chbits) {
		deflt = (u_long) ( (((u_long)ms->page4.dg_rpm_1 << 8) +
			        ((u_long)ms->page4.dg_rpm_0) ) &
				chbits );
		range.pr_max = chbits;
		if ( (status = PromptUser (pc, TYPE_VALUE,
			medium_rotation_rate_str, ':',
			&deflt, &answer, PF_RANGE, &range)) != SUCCESS) {
		    return (status);
		}
		ms->page4.dg_rpm_1 = (u_char) (answer >> 8);
		ms->page4.dg_rpm_0 = (u_char) answer;
	    } else {
		if (DebugFlag) {
		    FieldNotChangeable (medium_rotation_rate_str);
		}
	    }
	}

	/*
	 * Finally, set the new page parameters.
	 */
	if ((status = SelectGeometryPage (ms, SaveModeParameters)) == SUCCESS) {
		*de = *ms;		/* Set new default parameters.	*/
	}
	return (status);
}

/************************************************************************
 *									*
 * VerifyGeometryPage() - Verify The Rigid Disk Geometry Page.		*
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
VerifyGeometryPage (ce, ke, se, verify_verbose)
struct cmd_entry *ce;
struct key_entry *ke;
struct scsi_disk_geometry *se;
int verify_verbose;
{
	struct scu_device *scu = ScuDevice;
	struct geometry_params ch_geometry_params;
	struct geometry_params vd_geometry_params;
	register struct geometry_params *ch = &ch_geometry_params;
	register struct geometry_params *de;
	register struct geometry_params *vd = &vd_geometry_params;
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
	de = (struct geometry_params *) GetDefaultPage (PageCode, sizeof(*de));
	if (de == (struct geometry_params *) 0) {
	    return (FAILURE);
	}

	/*
	 * Find out what fields are changeable.
	 */
	if ((status = SenseGeometryPage (ch, PCF_CHANGEABLE)) != SUCCESS) {
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
	if (de->page4.page_header.page_code != PageCode) {
	    if ((status = SenseGeometryPage (de, pcf)) != SUCCESS) {
		return (status);
	    }
	}

	/*
	 * Copy the page parameters to set (if specified), since we need
	 * the mode parameter header and block descriptor to mode select.
	 */
	if (se != (struct scsi_disk_geometry *) 0) {
	    register struct scsi_disk_geometry *dg = &de->page4;

	    *dg = *se;
	}

	/*
	 * Get the vendor default values for sanity checks.
	 */
	if ((status = SenseGeometryPage (vd, PCF_DEFAULT)) != SUCCESS) {
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
	 * Verify "Maximum Number of Cylinders" parameter.
	 */
	field_changed = 0;
	ch_field = (ch->page4.dg_cylinders_2 << 16) |
				(ch->page4.dg_cylinders_1 << 8) | ch->page4.dg_cylinders_0;
	if (ch_field) {
	    de_field = (de->page4.dg_cylinders_2 << 16) |
				(de->page4.dg_cylinders_1 << 8) | de->page4.dg_cylinders_0;
	    if (dt->dtype_options & SUP_PCYL) {
		if ( (dt->dtype_pcyl <= ch_field) &&
				(dt->dtype_pcyl <= de_field) ) {
		    field_changed = 1;
		    de->page4.dg_cylinders_2 = 0;
		    de->page4.dg_cylinders_1 = (cur_dtype->dtype_pcyl >> 8) & 0xff;
		    de->page4.dg_cylinders_0 = cur_dtype->dtype_pcyl & 0xff;
		} else {
		    status = FAILURE;		/* Critical parameter. */
		    Printf ("%s %s\n", maximum_cylinders_str, too_large_str);
		}
	    }
	    if (verify_verbose) {
		de_field = (de->page4.dg_cylinders_2 << 16) |
				(de->page4.dg_cylinders_1 << 8) | de->page4.dg_cylinders_0;
		Printf ( (field_changed) ? setting_str : leaving_str,
				maximum_cylinders_str, de_field);
	    }
	} else {
	    vd_field = (vd->page4.dg_cylinders_2 << 16) |
				(vd->page4.dg_cylinders_1 << 8) | vd->page4.dg_cylinders_0;
	    if (dt->dtype_pcyl > vd_field) {
		status = FAILURE;		/* Critical parameter. */
		Printf ("%s %s\n", maximum_cylinders_str, too_large_str);
	    }
	    de->page4.dg_cylinders_2 = 0;
	    de->page4.dg_cylinders_1 = 0;
	    de->page4.dg_cylinders_0 = 0;
	    if (verify_verbose) {
		FieldNotChangeable (maximum_cylinders_str);
	    }
	}

	/*
	 * Verify "Maximum Number of Heads" parameter.
	 */
	field_changed = 0;
	ch_field = ch->page4.dg_heads;
	if (ch_field) {
	    de_field = de->page4.dg_heads;
	    if (dt->dtype_options & SUP_NHEAD) {
		if ( (dt->dtype_nhead <= ch_field) &&
				(dt->dtype_nhead <= de_field) ) {
		    field_changed = 1;
		    de->page4.dg_heads = cur_dtype->dtype_nhead;
		} else {
		    status = FAILURE;		/* Critical parameter. */
		    Printf ("%s %s\n", maximum_heads_str, too_large_str);
		}
	    }
	    if (verify_verbose) {
		de_field = de->page4.dg_heads;
		Printf ( (field_changed) ? setting_str : leaving_str,
				maximum_heads_str, de_field);
	    }
	} else {
	    vd_field = vd->page4.dg_heads;
	    if (dt->dtype_nhead > vd_field) {
		status = FAILURE;		/* Critical parameter. */
		Printf ("%s %s\n", maximum_heads_str, too_large_str);
	    }
	    de->page4.dg_heads = 0;
	    if (verify_verbose) {
		FieldNotChangeable (maximum_heads_str);
	    }
	}

	/*
	 * Verify "Write Precomp Starting Cylinder" parameter.
	 */
	field_changed = 0;
	ch_field = (ch->page4.dg_precomp_cyl_2 << 16) |
		(ch->page4.dg_precomp_cyl_1 << 8) | ch->page4.dg_precomp_cyl_0;
	if (ch_field) {
	    de_field = (de->page4.dg_precomp_cyl_2 << 16) |
		       (de->page4.dg_precomp_cyl_1 << 8) |
			de->page4.dg_precomp_cyl_0;
	    if (dt->dtype_options & SUP_PRECOMP_CYL) {
		if (dt->dtype_precomp_cyl <= ch_field) {
		    field_changed = 1;
		    de->page4.dg_precomp_cyl_2 = 0;
		    de->page4.dg_precomp_cyl_1 = (cur_dtype->dtype_precomp_cyl >> 8) & 0xff;
		    de->page4.dg_precomp_cyl_0 = cur_dtype->dtype_precomp_cyl & 0xff;
		} else {
		    Printf ("%s %s\n", precomp_cyl_str, too_large_str);
		}
	    }
	    if (verify_verbose) {
		de_field = (de->page4.dg_precomp_cyl_2 << 16) |
			   (de->page4.dg_precomp_cyl_1 << 8) |
			    de->page4.dg_precomp_cyl_0;
		Printf ( (field_changed) ? setting_str : leaving_str,
				precomp_cyl_str, de_field);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (precomp_cyl_str);
	    }
	}

	/*
	 * Verify "Reduced Write Current Cylinder" parameter.
	 */
	field_changed = 0;
	ch_field = (ch->page4.dg_current_cyl_2 << 16) |
		   (ch->page4.dg_current_cyl_1 << 8) |
		    ch->page4.dg_current_cyl_0;
	if (ch_field) {
	    de_field = (de->page4.dg_current_cyl_2 << 16) |
		       (de->page4.dg_current_cyl_1 << 8) |
			de->page4.dg_current_cyl_0;
	    if (dt->dtype_options & SUP_PRECOMP_CYL) {
		if (dt->dtype_current_cyl <= ch_field) {
		    field_changed = 1;
		    de->page4.dg_current_cyl_2 = 0;
		    de->page4.dg_current_cyl_1 = (cur_dtype->dtype_current_cyl >> 8) & 0xff;
		    de->page4.dg_current_cyl_0 = cur_dtype->dtype_current_cyl & 0xff;
		} else {
		    Printf ("%s %s\n", current_cyl_str, too_large_str);
		}
	    }
	    if (verify_verbose) {
		de_field = (de->page4.dg_current_cyl_2 << 16) |
			   (de->page4.dg_current_cyl_1 << 8) |
			    de->page4.dg_current_cyl_0;
		Printf ( (field_changed) ? setting_str : leaving_str,
				current_cyl_str, de_field);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (current_cyl_str);
	    }
	}

	/*
	 * Verify "Drive Step Rate" parameter.
	 */
	field_changed = 0;
	ch_field = ccstoh2(ch->page4.dg_step_rate);
	if (ch_field) {
	    de_field = ccstoh2(de->page4.dg_step_rate);
	    if (dt->dtype_options & SUP_CURRENT_CYL) {
		if (dt->dtype_step_rate <= ch_field) {
		    field_changed = 1;
		    de->page4.dg_step_rate = htoccs2(cur_dtype->dtype_step_rate);
		} else {
		    Printf ("%s %s\n", step_rate_str, too_large_str);
		}
	    }
	    if (verify_verbose) {
		de_field = ccstoh2(de->page4.dg_step_rate);
		Printf ( (field_changed) ? setting_str : leaving_str,
				step_rate_str, de_field);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (step_rate_str);
	    }
	}

	/*
	 * Verify "Landing Zone Cylinder" parameter.
	 */
	field_changed = 0;
	ch_field = (ch->page4.dg_landing_cyl_2 << 16) |
		   (ch->page4.dg_landing_cyl_1 << 8) |
		    ch->page4.dg_landing_cyl_0;
	if (ch_field) {
	    de_field = (de->page4.dg_landing_cyl_2 << 16) |
		       (de->page4.dg_landing_cyl_1 << 8) |
			de->page4.dg_landing_cyl_0;
	    if (dt->dtype_options & SUP_LANDING_CYL) {
		if (dt->dtype_landing_cyl <= ch_field) {
		    field_changed = 1;
		    de->page4.dg_landing_cyl_2 = 0;
		    de->page4.dg_landing_cyl_1 = (cur_dtype->dtype_landing_cyl >> 8) & 0xff;
		    de->page4.dg_landing_cyl_0 = cur_dtype->dtype_landing_cyl & 0xff;
		} else {
		    Printf ("%s %s\n", landing_cyl_str, too_large_str);
		}
	    }
	    if (verify_verbose) {
		de_field = (de->page4.dg_landing_cyl_2 << 16) |
			   (de->page4.dg_landing_cyl_1 << 8) |
			    de->page4.dg_landing_cyl_0;
		Printf ( (field_changed) ? setting_str : leaving_str,
				landing_cyl_str, de_field);
	    }
	} else {
	    if (verify_verbose) {
		FieldNotChangeable (landing_cyl_str);
	    }
	}
#endif notdef
	return (status);
}

/************************************************************************
 *									*
 * SenseGeometryPage() - Sense The Rigid Disk Geometry Page.		*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SenseGeometryPage (ms, pcf)
struct geometry_params *ms;
u_char pcf;
{
	register struct scu_device *scu = ScuDevice;
	int mode_length;

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    mode_length = sizeof(*ms);
	} else {
	    mode_length = sizeof(ms->header) + ccs_geometry_page_length;
	}
	return (SensePage (ms, pcf, PageCode, mode_length));
}


/************************************************************************
 *									*
 * SelectGeometryPage() - Select The Rigid Disk Geometry Page.		*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
SelectGeometryPage (ms, smp)
struct geometry_params *ms;
u_char smp;
{
	struct geometry_params se_geometry_params;
	register struct geometry_params *se = &se_geometry_params;
	register struct scu_device *scu = ScuDevice;
	int mode_length;

	if (scu->scu_inquiry->rdf == ALL_RDF_SCSI2) {
	    mode_length = sizeof(*se);
	} else {
	    mode_length = sizeof(se->header) + ccs_geometry_page_length;
	}
	*se = *ms;			/* Copy parameters to set. */
	return (SelectPage (se, mode_length, smp));
}
