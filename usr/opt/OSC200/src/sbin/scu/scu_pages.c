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
static char *rcsid = "@(#)$RCSfile: scu_pages.c,v $ $Revision: 1.1.12.2 $ (DEC) $Date: 1993/11/23 23:10:57 $";
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
 * File:	scu_pages.c
 * Author:	Robin T. Miller
 * Date:	September 10, 1991
 *
 * Description:
 *	This file contains functions to process SCSI mode pages.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <io/common/iotypes.h>
#include <io/cam/cdrom.h>
#include <io/cam/rzdisk.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>

#undef SUCCESS
#undef FATAL_ERROR

#include "scu.h"
#include "scu_device.h"
#include "scu_pages.h"
#include "scsipages.h"

/*
 * External References:
 */
extern char *malloc_palign(int size);
extern void free_palign(char *pa_addr);

extern void FreeDefaultPages (struct scu_device *scu);
extern int OpenPager (char *pager);
extern void Perror();

extern struct mode_page_funcs error_page_funcs[];
extern struct mode_page_funcs reco_page_funcs[];
extern struct mode_page_funcs direct_page_funcs[];
extern struct mode_page_funcs geometry_page_funcs[];
extern struct mode_page_funcs flexible_page_funcs[];
extern struct mode_page_funcs cache_page_funcs[];
extern struct mode_page_funcs audio_page_funcs[];
extern struct mode_page_funcs device_config_page_funcs[];
extern struct mode_page_funcs medium_part_page1_funcs[];
extern struct mode_page_funcs cdrom_page_funcs[];
extern struct mode_page_funcs dec_page_funcs[];
extern struct mode_page_funcs readahead_page_funcs[];
extern struct mode_page_funcs *mode_page_table[];

extern u_char PageCode;
extern struct mode_page_funcs unknown_page_funcs[];

/*
 * Strings shared by all SCSI modules:
 */
char *too_large_str =			"value is too large!";
char *setting_str =			"Setting '%s' to %d\n";
char *leaving_str =			"Leaving '%s' at %d\n";
char *changing_str =			"Changing ";
char *verifying_str =			"Verifying ";

/*
 * List of legal inputs for page control field question.
 *
 *	---> Order MUST not be changed for mode sense command <---
 */
char *pcf_table[] = {
	"current",
	"changeable",
	"default",
	"saved",
	NULL,
};

char *command_table[] = {
	"all pages",
	"error recovery",
	"reconnect/disconnect control",
	"direct-access device format",
	"rigid disk drive geometry",
	"flexible disk parameters",
	"read-ahead control",
	"device parameters",
	"audio control",
	"configuration parameters",
	NULL,
};

struct PageHeader {
	struct mode_parameter_header parameter_header;
	struct mode_block_descriptor block_descriptor;
	struct scsi_page_header page_header;
};

static char *mode_length_str =		"Mode Data Length";
static char *medium_type_str =		"Medium Type";
static char *device_specific_str =	"Device Specific Parameter";
static char *speed_str =		"Tape Speed";
static char *buffered_str = 		"Buffered Mode";
static char *write_protected_str =	"Write Protected";
static char *block_desc_length_str =	"Block Descriptor Length";
static char *density_code_str =		"Density Code";
static char *logical_blocks_str =	"Number of Logical Blocks";
static char *logical_block_length_str =	"Logical Block Length";

static char *page_code_str =		"Page Code";
static char *parameter_savable_str =	"Parameters Savable";
static char *page_length_str =		"Page Length";

/*
 * Define the Tape Density Table.
 *
 * NOTE:  This table *must* match actual tape density codes.
 *	  Also, tapes which share the same BPI value (e.g., 10000_BPI)
 *	  are differentiated by density name (i.e., QIC_120 or QIC_150).
 *	  This latter is necessary to parse to a particular density code.
 */
struct tape_density_entry tape_density_table[] = {
    {	T_DEFAULT_DENSITY,	"Default Density"	},	    /* 0x00 */
    {	T_DENSITY_800_BPI,	"800 BPI (NRZI, R)"	},	    /* 0x01 */
    {	T_DENSITY_1600_BPI,	"1600 BPI (PE, R)"	},	    /* 0x02 */
    {	T_DENSITY_6250_BPI,	"6250 BPI (GCR, R)"	},	    /* 0x03 */
    {	T_DENSITY_8000_BPI,	"8000 BPI (GCR, C)"	},	    /* 0x04 */
    {	T_DENSITY_QIC_24,	"8000 BPI, QIC-24 (GCR, C)" },	    /* 0x05 */
    {	T_DENSITY_3200_BPI,	"3200 BPI (PE, R)"	},	    /* 0x06 */
    {	T_DENSITY_6400_BPI,	"6400 BPI (IMFM, C)"	},	    /* 0x07 */
    {	T_DENSITY_8000_BPI,	"8000 BPI (GCR, CS)"	},	    /* 0x08 */
    {	T_DENSITY_38000_BPI,	"37871 BPI (GCR, C)"	},	    /* 0x09 */
    {	T_DENSITY_6666_BPI,	"6667 BPI (MFM, C)"	},	    /* 0x0A */
    {	T_DENSITY_1600_BPI,	"1600 BPI (PE, C)"	},	    /* 0x0B */
    {	T_DENSITY_12690_BPI,	"12690 BPI (GCR, C)"	},	    /* 0x0C */
    {	T_DENSITY_QIC_120_ECC,	"10000 BPI, QIC-120 with ECC"	},  /* 0x0D */
    {	T_DENSITY_QIC_150_ECC,	"10000 BPI, QIC-150 with ECC"	},  /* 0x0E */
    {	T_DENSITY_QIC_120,	"10000 BPI, QIC-120 (GCR, C)"	},  /* 0x0F */
    {	T_DENSITY_QIC_150,	"10000 BPI, QIC-150 (GCR, C)"	},  /* 0x10 */
    {	T_DENSITY_QIC_320,	"16000 BPI, QIC-320 (GCR, C)"	},  /* 0x11 */
    {	T_DENSITY_QIC_1350,	"51667 BPI, QIC-1350 (RLL, C)"	},  /* 0x12 */
    {	T_DENSITY_61000_BPI,	"61000 BPI, 4mm Tape  (DDS, CS)"},   /* 0x13 */
    {	T_DENSITY_54000_BPI,	"54000 BPI, 8mm Tape  (???, CS)"},   /* 0x14 */
    {	T_DENSITY_45434_BPI,	"45434 BPI, 8mm Tape 8500 mode"},    /* 0x15 */
    {	T_DENSITY_10000_BPI,	"10000 BPI (MFM, C)"	},	    /* 0x16 */
    {	T_DENSITY_42500_BPI,	"42500 BPI (MFM, CS)"	},	    /* 0x17 */
    {	T_DENSITY_42500_BPI,	"42500 BPI (MFM, CS)"	},	    /* 0x18 */
    {	T_DENSITY_62500_BPI,	"62500 BPI (MFM, CS)"	},	    /* 0x19 */
    {	T_DENSITY_UNKNOWN,	"Unknown Density"	},	    /* 0x1A */
    {	T_DENSITY_UNKNOWN,	"Unknown Density"	},	    /* 0x1B */
    {	T_DENSITY_UNKNOWN,	"Unknown Density"	},	    /* 0x1C */
    {	T_DENSITY_UNKNOWN,	"Unknown Density"	},	    /* 0x1D */
    {	T_DENSITY_QIC_1G,	"36000 BPI, QIC-1G (GCR, C)"},	    /* 0x1E */
    {	T_DENSITY_UNKNOWN,	"Unknown Density"	},	    /* 0x1F */
    {	T_DENSITY_UNKNOWN,	"Unknown Density"	},	    /* 0x20 */
    {	T_DENSITY_UNKNOWN,	"Unknown Density"	},	    /* 0x21 */
    {	T_DENSITY_QIC_2G,	"40640 BPI, QIC-2G (GCR, C)"},	    /* 0x22 */
    {	T_DENSITY_UNKNOWN,	"Unknown Density"	}
};
int tape_density_entrys =
	sizeof(tape_density_table) / sizeof(struct tape_density_entry);

typedef struct optical_medium_entry {
	enum optical_medium op_density;	/* The tape density value.	*/
	char	*op_name;		/* The tape density name.	*/
} OPTICAL_MEDIUM_ENTRY;

/*
 * Define the Optical Memory Medium-Types Table.
 */
static struct optical_medium_entry optical_medium_table[] = {
    {	MT_DEFAULT,		"Default Medium Type"		},
    {	MT_OP_READ_ONLY,	"Optical Read only medium"	},
    {	MT_OP_WRITE_ONCE,	"Optical Write once medium"	},
    {	MT_OP_ERASABLE,		"Reversible or Erasable medium"	},
    {	MT_OP_RONLY_WONCE,	"Read only & Write once medium"	},
    {	MT_OP_RONLY_ERASABLE,	"Read only & reversible/erasable" },
    {	MT_OP_WONCE_ERASABLE,	"Write once & reversible/erasable" },
    {	MT_OP_UNKNOWN,		"Reserved or vendor specific"	}
};
static int optical_medium_entrys = 
	sizeof(optical_medium_table) / sizeof(struct optical_medium_entry);

/************************************************************************
 *									*
 * InitPageTables() - Initialize the Page Function Table.		*
 *									*
 * Inputs:	pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
InitPageTables()
{
	mode_page_table[ERROR_RECOVERY_PAGE] = error_page_funcs;
	mode_page_table[DISCO_RECO_PAGE] = reco_page_funcs;
	mode_page_table[DIRECT_ACCESS_PAGE] = direct_page_funcs;
	mode_page_table[DISK_GEOMETRY_PAGE] = geometry_page_funcs;
	mode_page_table[FLEXIBLE_DISK_PAGE] = flexible_page_funcs;
	mode_page_table[CACHE_CONTROL_PAGE] = cache_page_funcs;
	mode_page_table[AUDIO_CONTROL_PAGE] = audio_page_funcs;
	mode_page_table[DEVICE_CONFIG_PAGE] = device_config_page_funcs;
	mode_page_table[MEDIUM_PART_PAGE1] = medium_part_page1_funcs;
	mode_page_table[CDROM_DEVICE_PAGE] = cdrom_page_funcs;
	mode_page_table[DEC_SPECIFIC_PAGE] = dec_page_funcs;
	mode_page_table[READAHEAD_CONTROL_PAGE] = readahead_page_funcs;
}

/************************************************************************
 *									*
 * GetAllPages() - Get All Device Mode Pages.				*
 *									*
 * Inputs:	pcf = The page control field.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
GetAllPages (pcf)
u_char pcf;
{
	register struct scu_device *scu = ScuDevice;
	register caddr_t page_buffer;
	register int pcf_index = (pcf >> PCF_SHIFT);
	u_char size = MODE_PAGES_SIZE;

	page_buffer = scu->scu_mode_pages[pcf_index];
	if (page_buffer == (caddr_t) 0) {
	    page_buffer = malloc_palign ((size_t) size);
	    if (page_buffer == (caddr_t) 0) {
		errno = ENOMEM;
		Perror ("Failed to allocate mode page buffer of %d bytes.", size);
		return (ENOMEM);
	    }
	    scu->scu_mode_pages[pcf_index] = page_buffer;
	}
	return (SensePage ((struct scsi_page_params *) page_buffer,
						pcf, ALL_MODE_PAGES, size));
}

/************************************************************************
 *									*
 * GetDefaultPage() - Get The Default Mode Page.			*
 *									*
 * Description:								*
 *	This function is used to return a pointer to the default mode	*
 * page parameters.  If the mode page has not been allocated yet, this	*
 * routine allocates a buffer, initializes the buffer, and saves the	*
 * buffer address in the per device structure.				*
 *									*
 * Inputs:	page = The mode page to get.				*
 *		size = The size of the mode page.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void *
GetDefaultPage (page, size)
u_char page;
u_int size;
{
	register struct scu_device *scu = ScuDevice;
	register caddr_t page_buffer;

	page_buffer = scu->scu_default_pages[page];
	if (page_buffer == (caddr_t) 0) {
	    page_buffer = (caddr_t) malloc ((size_t) size);
	    if (page_buffer == (char *) 0) {
		errno = ENOMEM;
		Perror ("Failed to allocate mode page buffer of %d bytes.", size);
		return (page_buffer);
	    }
	    (void) bzero (page_buffer, size);
	    scu->scu_default_pages[page] = page_buffer;
	}
	return (page_buffer);
}

/************************************************************************
 *									*
 * FreeDefaultPages() - Frre The Default Mode Pages.			*
 *									*
 * Description:								*
 *	This function returns all memory allocated for default mode	*
 * pages.  This is normally required when the device type or nexus	*
 * changes.								*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
FreeDefaultPages (scu)
register struct scu_device *scu;
{
	register caddr_t page_buffer;
	register int page;

	for (page = 0; page < MAX_MODE_PAGES; page++) {
	    page_buffer = scu->scu_default_pages[page];
	    if (page_buffer != (caddr_t) 0) {
		(void) free (page_buffer);
		scu->scu_default_pages[page] = (caddr_t) 0;
	    }
	}
	return;
}

/************************************************************************
 *									*
 * ProcessModePage() - Process Device Mode Pages.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ProcessModePage (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;
	register struct scsi_page_params *pages;
	register struct mode_parameter_header *mph;
	register struct scsi_page_header *ph = NULL;
	register u_char *bptr;
	struct mode_page_funcs *pf;
	int ret_status = SUCCESS, status;
	int saved_flag = DisplayModeParameters;
	int hdr_length, mode_length;
	u_char pcf_index;

	if ((status = SetupOperationType()) != SUCCESS) {
	    return (status);
	}
	pcf_index = (PageControlField >> PCF_SHIFT);

	/*
	 * If a mode page was specified, then process just that page.
	 */
	if (ModePageCode > 0) {
	    pf = mode_page_table[ModePageCode];
	    if ( ISSET_KOPT (ce, K_MODE_PAGE) ||
		 (pf == (struct mode_page_funcs *) 0) ) {
		pf = unknown_page_funcs;
		PageCode = ModePageCode;
	    }
	    ModePageCode = -1;
	    if (OperationType == SHOW_OPERATION)
		(void) OpenPager (NULL);
	    return (DispatchModePage (ce, ke, pf, ph));
	}

	ProcessAllPages = TRUE;
	DisplayModeParameters = FALSE;
	if ((status = GetAllPages (PageControlField)) != SUCCESS) {
	    return (status);
	}

	pages = (struct scsi_page_params *) scu->scu_mode_pages[pcf_index];
	mph = &pages->mode_header.parameter_header;
	mode_length = mph->mph_data_length + 1;

	if (OperationType == SHOW_OPERATION) {
	    (void) OpenPager (NULL);
	    ShowModeParameters ((struct mode_page_header *) pages);
	}
	hdr_length = sizeof(*mph) + mph->mph_block_desc_length;
	mode_length -= hdr_length;
	bptr = (u_char *)pages;
	bptr += hdr_length;

	/*
	 * Loop through all the mode pages returned.
	 */
	while (mode_length) {
	    ph = (struct scsi_page_header *) bptr;
	    if ( (ph->page_code == 0) && (ph->page_length == 0) ) {
		break;
	    }
	    pf = mode_page_table[ph->page_code];
	    if (pf != (struct mode_page_funcs *) 0) {
		if (ph->page_length) {
		    if ((status = DispatchModePage (ce, ke, pf, ph)) != SUCCESS) {
			ret_status = status;
		    }
		} else {
		    if (DebugFlag) {
			Printf ("Warning: Page 0x%x is zero length page.\n",
							ph->page_code);
		    }
		}
	    } else {
#ifdef notdef
		Printf ("\nWarning: No page functions for page code 0x%x\n",
							ph->page_code);
		cdbg_DumpBuffer ((char *)ph, sizeof(*ph) + ph->page_length);
#endif
		pf = unknown_page_funcs;
		PageCode = ph->page_code;
		if (ph->page_length) {
		    if ((status = DispatchModePage (ce, ke, pf, ph)) != SUCCESS) {
			ret_status = status;
		    }
		} else {
		    if (DebugFlag) {
			Printf ("Warning: Page 0x%x is zero length page.\n",
							ph->page_code);
		    }
		}
	    }
	    mode_length -= (sizeof(*ph) + ph->page_length);
	    bptr += (sizeof(*ph) + ph->page_length);
	}
	ProcessAllPages = FALSE;
	DisplayModeParameters = saved_flag;
	return (ret_status);
}

/************************************************************************
 *									*
 * DispatchModePage() - Dispatch to Mode Page Functions.		*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		pf = The mode page function table.			*
 *		ph = The page header (if any).				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DispatchModePage (ce, ke, pf, ph)
struct cmd_entry *ce;
struct key_entry *ke;
struct mode_page_funcs *pf;
struct scsi_page_header *ph;
{
	int status = FAILURE;

	switch (OperationType) {

	    case SET_OPERATION: {
		status = (*pf->pf_setpage)(ce, ke, ph);
		break;
	    }

	    case SHOW_OPERATION:
		status = (*pf->pf_showpage)(ce, ke, ph);
		break;

	    case CHANGE_OPERATION:
		status = (*pf->pf_changepage)(ce, ke, ph);
		break;

	    case VERIFY_OPERATION:
		status = (*pf->pf_verifypage)(ce, ke, ph, VERBOSE_MODE);
		break;

	    default:
		return (FAILURE);
	}
	return (status);
}

/************************************************************************
 *									*
 * ShowModeParamHeaders() - Show The Mode Parameter Headers.		*
 *									*
 * Description:								*
 *	This function is used to sense and display the mode parameters	*
 * (parameter header & block descriptors).				*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowModeParamHeaders (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct mode_page_header mode_parameters;
	register struct mode_page_header *mh = &mode_parameters;
	u_char pcf = PageControlField, page_code = MODE_PARAMETERS_PAGE;
	int status;

	if ((status = SensePage (mh, pcf, page_code, sizeof(*mh))) == FAILURE) {
	    return (status);
	}
	ShowParamHeader ("", "Mode", "Page 0", PageControlField);
	ShowModeParameters (mh);
	return (status);
}

/************************************************************************
 *									*
 * SensePage() - Sense A Particular SCSI Mode Page.			*
 *									*
 * Inputs:	ms = Pointer to mode sense structure.			*
 *		pcf = The page control field.				*
 *		page = The mode page to sense.				*
 *		size = Size of mode sense structure.			*
 *									*
 * Returns:	Returns 0 / -1 / 1 = SUCCESS / FAILURE / WARNING	*
 *									*
 ************************************************************************/
int
SensePage (ms, pcf, page, size)
register struct scsi_page_params *ms;
u_char pcf;
u_char page;
u_char size;
{
	struct mode_sel_sns_params mode_sense_params;
	struct mode_sel_sns_params *msp = &mode_sense_params;
	int status;

	bzero ((char *) ms, size);
	bzero ((char *) msp, sizeof(msp));
	msp->msp_addr = (caddr_t) ms;
	/*
	 * TODO:  Field msp_length is limited to sizeof(u_char).
	 *	  This MUST change to support 10-byte CDB's.
	 *	  Defined in io/cam/rzdisk.h
	 */
	msp->msp_length = size;
	msp->msp_pgcode = page;
	msp->msp_pgctrl = (pcf >> PCF_SHIFT);
	status = DoIoctl (SCSI_MODE_SENSE, (char *) msp, "mode sense");
	if (status == SUCCESS) {
	    struct mode_parameter_header *mph;
	    struct scsi_page_header *ph;
	    int hdr_length;
	    mph = &ms->mode_header.parameter_header;
	    hdr_length = sizeof(*mph) + mph->mph_block_desc_length;
	    ph = (struct scsi_page_header *) ((caddr_t) mph + hdr_length);
	    /*
	     * A page length of zero means that either the page does not
	     * exist, or that the page is not changeable.
	     */
	    if (ph->page_length == 0) {
		status = WARNING;
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * SelectPage() - Do Mode Select On A SCSI Page.			*
 *									*
 * Inputs:	ms = Pointer to mode select structure.			*
 *		size = Size of mode select structure.			*
 *		smp = Save mode parameters value.			*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SelectPage (ms, size, smp)
register struct scsi_page_params *ms;
u_char size;
u_char smp;
{
	struct mode_sel_sns_params mode_select_params;
	struct mode_sel_sns_params *msp = &mode_select_params;
	struct PageHeader *ph = (struct PageHeader *) ms;
	struct mode_parameter_header *mph = &ms->mode_header.parameter_header;
	int page = ph->page_header.page_code;
	int page_savable = ph->page_header.ps;
	int mode_length = (mph->mph_data_length + 1);
	int status;

	if (mode_length != size) {
	    if (DebugFlag) {
        	Printf (
	"Warning: Mode Sense data length (%d) != Mode Select size (%d)\n",
							mode_length, size);
		Printf (
	"Warning: Using mode sense data length for Mode Select command...\n");
	    }
	}

	if ((status = SetupHeaders (ms)) != SUCCESS) {
		return (status);
	}
	bzero ((char *) msp, sizeof(msp));
	msp->msp_addr = (caddr_t) ms;
	msp->msp_length = mode_length;
	msp->msp_pgcode = page;
	msp->msp_setps = (smp && page_savable) ? TRUE : FALSE;
	return (DoIoctl (SCSI_MODE_SELECT, (char *) msp, "mode select"));
}

/************************************************************************
 *									*
 * ShowParamHeader() - Display Page Parameter Header Message.		*
 *									*
 * Inputs:	pre = Prefix to display before header.			*
 *		hdr = The page header being displayed.			*
 *		page = The page code (i.e., "Page 1").			*
 *		pcf = The page control field.				*
 *									*
 * Outputs:	None.							*
 *									*
 ************************************************************************/
void
ShowParamHeader (pre, hdr, page, pcf)
char *pre, *hdr, *page;
u_char pcf;
{
	Printf ("\n%s%s Parameters (%s - %s values):\n",
			pre, hdr, page, pcf_table[pcf >> PCF_SHIFT]);

	if (OperationType != SHOW_OPERATION) {
		Printf ("\n");
	}
}

/************************************************************************
 *									*
 * ShowModeParameters() - Display Mode Page Parameter Information.	*
 *									*
 * Inputs:	mh = The mode sense parameters header.			*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
ShowModeParameters (mh)
struct mode_page_header *mh;
{
	register struct mode_parameter_header *mph;

	mph = &mh->parameter_header;
	ShowModeHeader (mh);
	if (mph->mph_block_desc_length) {
	    ShowBlockDesc (mh);
	}
}

/************************************************************************
 *									*
 * ShowModeHeader() - Display Mode Sense Parameter Header.		*
 *									*
 * Inputs:	header = The SCSI Mode Sense Header Structure.		*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
ShowModeHeader (header)
register struct mode_page_header *header;
{
	register struct scu_device *scu = ScuDevice;
	register struct mode_parameter_header *mph = &header->parameter_header;

	Printf ("\nMode Parameter Header:\n\n");

	PrintNumeric (mode_length_str, mph->mph_data_length, PNL);

	/*
	 * Decode the medium type field.
	 */
	PrintHex (medium_type_str, mph->mph_medium_type, DNL);
	if (PageControlField != PCF_CHANGEABLE) {
	    char *medium_str = NULL;

	    if (scu->scu_device_type == ALL_DTYPE_DIRECT) {

		switch (mph->mph_medium_type) {

		    case MT_DEFAULT:
			medium_str = "Default Medium Type";
			break;

		    case MT_DD_DISKETTE:
			medium_str = "Double Density Diskette";
			break;

		    case MT_HD_DISKETTE:
			medium_str = "High Density Diskette";
			break;

		    case MT_ED_DISKETTE:
			medium_str = "Extra Density Diskette";
			break;

		    default:
			break;
		}
	    } else if (scu->scu_device_type == ALL_DTYPE_OPTICAL) {
		if (mph->mph_medium_type < optical_medium_entrys) {
		    medium_str = optical_medium_table[mph->mph_medium_type].op_name;
		} else {
		    medium_str = optical_medium_table[MT_OP_UNKNOWN].op_name;
		}
	    } else if (mph->mph_medium_type == MT_DEFAULT) {
		medium_str = "Default Medium Type";
	    }
	    if (medium_str != NULL) Printf (" = %s", medium_str);
	}
	Printf("\n");

	/*
	 * Decode the device specific parameters.
	 */
	if (PageControlField != PCF_CHANGEABLE) {
	  if (scu->scu_device_type == ALL_DTYPE_SEQUENTIAL) {
	    PrintHex (speed_str, mph->mph_speed, DNL);
	      if (mph->mph_speed == SPEED_DEFAULT) {
		Printf (" = Use Default Speed\n");
	      } else if (mph->mph_speed == SPEED_LOWEST) {
		Printf (" = Use Lowest Speed\n");
	      } else {
		Printf (" = Use increasing speeds\n");
	      }
	    PrintHex (buffered_str, mph->mph_buffer_mode, PNL);
	    PrintAscii(write_protected_str,
			yesno_table[mph->mph_write_protect], PNL);
	  } else {
	    PrintHex (device_specific_str, mph->mph_device_specific, DNL);
	    if (mph->mph_device_specific & DS_WRITE_PROTECT) {
		Printf (" (%s)\n", write_protected_str);
	    } else {
		Printf ("\n");
	    }
	  }
	} else {
	  PrintHex (device_specific_str, mph->mph_device_specific, PNL);
	}

	PrintNumeric (block_desc_length_str, mph->mph_block_desc_length, PNL);
}

/************************************************************************
 *									*
 * ShowBlockDesc() - Display Mode Sense Block Descriptor.		*
 *									*
 * Inputs:	header = The SCSI Mode Sense Header Structure.		*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
ShowBlockDesc (header)
register struct mode_page_header *header;
{
	register struct scu_device *scu = ScuDevice;
	register struct mode_block_descriptor *mbd = &header->block_descriptor;
	u_long tmp;

	Printf ("\nMode Parameter Block Descriptor:\n\n");

	/*
	 * For sequential access devices, decode the density code.
	 */
	PrintHex (density_code_str, mbd->mbd_density_code, DNL);
	if ( (PageControlField != PCF_CHANGEABLE) &&
	     (scu->scu_device_type == ALL_DTYPE_SEQUENTIAL) ) {
	    if (mbd->mbd_density_code < tape_density_entrys) {
		Printf (" = %s\n",
		    tape_density_table[mbd->mbd_density_code].td_name);
	    } else {
		Printf (" = %s\n",
			tape_density_table[T_DENSITY_UNKNOWN].td_name);
	    }
	} else {
	    Printf("\n");
	}

	tmp = (u_long) ( ((u_long)mbd->mbd_num_blocks_2 << 16) +
			 (mbd->mbd_num_blocks_1 << 8) +
			 (mbd->mbd_num_blocks_0) );
	PrintNumeric (logical_blocks_str, tmp, PNL);

	tmp = (u_long) ( ((u_long)mbd->mbd_block_length_2 << 16) +
			 (mbd->mbd_block_length_1 << 8) +
			 (mbd->mbd_block_length_0) );
	PrintNumeric (logical_block_length_str, tmp, PNL);
}

/************************************************************************
 *									*
 * ShowPageHeader() - Display Mode Sense Page Header.			*
 *									*
 * Inputs:	page_header = The SCSI Page Header Structure.		*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
ShowPageHeader (page_header)
register struct scsi_page_header *page_header;
{
	if (DisplayBlockDesc) {
		Printf ("\nPage Header:\n");
	}

	PrintHex (page_code_str, page_header->page_code, 1);

	PrintAscii (parameter_savable_str, yesno_table[page_header->ps], 1);

	PrintNumeric (page_length_str, page_header->page_length, 1);
}

/************************************************************************
 *									*
 * SetupChangeable() - Setup Mode Page Changeable Parameters.		*
 *									*
 * Description:								*
 *	This function ensures all fields not marked as changeable, get	*
 * cleared prior to attempting to do mode select on them (CCS/SCSI-1).	*
 * For SCSI-2 compliant devices, fields which are NOT changeable MUST	*
 * not be changed (same value from mode sense sent on mode select).	*
 *									*
 * Inputs:	ch = The changeable page parameters.			*
 *		se = The mode page to setup.				*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetupChangeable (ch, se)
struct scsi_page_header *ch;
struct scsi_page_header *se;
{
	register struct scu_device *scu = ScuDevice;
	register u_char *chp = (u_char *) ch + sizeof(struct scsi_page_header);
	register u_char *sep = (u_char *) se + sizeof(struct scsi_page_header);
	register u_char page_length = se->page_length;
	int any_changeable = FALSE;

	if (ch->page_length != page_length) {
	    Printf ("Changeable page length != Mode page being setup.\n");
	    return (FAILURE);
	}

	/* 
	 * Check if we have any changeable fields.
	 */
	while (page_length--) {
	    if (*chp++) {
		any_changeable = TRUE;
		break;
	    }
	}
	if (any_changeable != TRUE) {
	    return (WARNING);
	}

	/*
	 * Clear non-changeable fields for non-SCSI-2 compliant devices.
	 */
	if (scu->scu_inquiry->rdf != ALL_RDF_SCSI2) {
	    chp = (u_char *) ch + sizeof(struct scsi_page_header);
	    page_length = se->page_length;
	    while (page_length--) {
		*sep++ &= *chp++;
	    }
	}
	return (SUCCESS);
}

/************************************************************************
 *									*
 * SetupHeaders() - Setup Headers For Mode Select Command.		*
 *									*
 * Description:								*
 *	Function to setup the mode select headers.  Since we're using	*
 * the mode sense header, so certain fields must be zeroed before the	*
 * mode select is issued.						*
 *									*
 * Inputs:	ms = The Mode Select Header.				*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetupHeaders (ms)
struct PageHeader *ms;
{
	register struct mode_parameter_header *mph = &ms->parameter_header;
	register struct mode_block_descriptor *mbd = &ms->block_descriptor;
	struct scu_device *scu = ScuDevice;
	int status = SUCCESS;
	U32 block_length;

	/*
	 * If the page savable bit isn't set, then warn the user that
	 * this page won't be saved.
	 */
	if (CheckPageSavable) {
	    if ( (ms->page_header.ps == 0) && (DisplayVerbose) ) {
		Printf ("Warning: Page code %d is not savable by this drive.\n",
					ms->page_header.page_code);
		/* return (WARNING); */
	    }
	}

#ifdef notdef
	/*
	 * Do some sanity checking (reserved for some devices).
	 */
	if (mph->mph_medium_type != 0) {
	    if (DebugFlag) {
		Printf ("Warning: Unexpected medium type, was %d, setting to %d\n",
					mph->mph_medium_type, 0);
	    }
	    mph->mph_medium_type = 0;
	}
#endif notdef
	mph->mph_reserved_0 = 0;
	mph->mph_reserved_1 = 0;
	if (scu->scu_device_type != ALL_DTYPE_SEQUENTIAL) {
	    mph->mph_reserved_2 = 0;	/* Speed & Buffer Mode for tapes. */
	}
	mbd->mbd_reserved_0 = 0;

	if (mph->mph_block_desc_length != sizeof(struct mode_block_descriptor)) {
	    if (DisplayVerbose) {
		Printf ("Warning: Wrong block descriptor length, was %d, setting to %d\n",
					mph->mph_block_desc_length,
					sizeof(struct mode_block_descriptor));
	    }
	    mph->mph_block_desc_length = sizeof(struct mode_block_descriptor);
	}

	block_length = (U32) (
			((U32)mbd->mbd_block_length_2 << 16) +
			((U32)mbd->mbd_block_length_1 << 8) +
			((U32)mbd->mbd_block_length_0) );

	/*
	 * Ensure direct-access devices are set to the correct block size.
	 * This is done since CD-ROM's normally default to 1k block size,
	 * and certain SCSI controllers (i.e., SCSI to ESDI) do not have
	 * a valid block size in this field until formatting is complete.
	 */
	if ( (scu->scu_device_type == ALL_DTYPE_DIRECT) ||
	     (scu->scu_device_type == ALL_DTYPE_RODIRECT) ) {
	    if (block_length != DEC_BLOCK_SIZE) {
	      if (DisplayVerbose) {
		Printf ("Warning: Wrong block length, was %d, setting to %d\n",
			block_length, DEC_BLOCK_SIZE);
	      }
	    }
	    mbd->mbd_block_length_2 = LTOB (DEC_BLOCK_SIZE, 2);
	    mbd->mbd_block_length_1 = LTOB (DEC_BLOCK_SIZE, 1);
	    mbd->mbd_block_length_0 = LTOB (DEC_BLOCK_SIZE, 0);
	}

	ms->page_header.ps = 0;		/* Reserved field for mode select. */
	ms->page_header.reserved = 0;
	return (status);
}

/************************************************************************
 *									*
 * SetupOperationType() - Set the Mode Operation Type.			*
 *									*
 * Inputs:	None.							*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetupOperationType ()
{
	register struct parser_control *pc = &ParserControl;
	int status = SUCCESS;

	if ( ISSET_COPT (pc, C_SET) ) {
	    OperationType = SET_OPERATION;
	} else if ( ISSET_COPT (pc, C_SHOW) ) {
	    OperationType = SHOW_OPERATION;
	} else if ( ISSET_COPT (pc, C_CHANGE) ) {
	    OperationType = CHANGE_OPERATION;
	} else if ( ISSET_COPT (pc, C_VERIFY) ) {
	    OperationType = VERIFY_OPERATION;
	} else {
	    Printf ("No operation type to setup...\n");
	    status = FAILURE;
	}

	/*
	 * For all operations except Show, the PCF must be checked.
	 */
	if (OperationType != SHOW_OPERATION) {
	    /*
	     * If changeable, set PCF to obtain default parameters.
	     */
	    if (PageControlField == PCF_CHANGEABLE) {
		PageControlField = PCF_DEFAULT;
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * FieldNotChangeable() - Display Field Not Changeable Message.		*
 *									*
 * Inputs:	str = The field name which isn't changeable.		*
 *									*
 * Returns:	Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
void
FieldNotChangeable (str)
char *str;
{
	if (DisplayVerbose || DebugFlag) {
	    Printf ("The '%s' field is NOT changeable.\n", str);
	}
}
