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
static char *rcsid = "@(#)$RCSfile: scu_tables.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/12/15 20:57:09 $";
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
 * File:	scu_tables.c
 * Author:	Robin T. Miller
 * Date:	November 21, 1990
 *
 * Description:
 *	This file contains the tables used by the table parser.
 */

#include <sys/types.h>
#include <sys/mtio.h>
#include <io/common/iotypes.h>
#include <io/cam/rzdisk.h>
#include <io/cam/cdrom.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#include <io/cam/scsi_rodirect.h>
#include <io/cam/scsi_sequential.h>
#include <io/cam/scsi_special.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"
#include "scu_pages.h"
#include "scsipages.h"

/*
 * External References:
 */
#if defined(CAMDEBUG)

extern u_long camdbg_flag;

#endif /* defined(CAMDEBUG) */

extern u_long cdbg_DumpLimit;

extern int Help(), ProcessModePage();

/*
 * Play Audio Specific Functions:
 */
extern int PlayAudio(), PlayAudioMSF(), PlayRange();
extern int PlayTrack(), PlayVTrack();

/*
 * Magtape Specific Functions:
 */
extern int DoMtCmd(), MapTapeDensity(), SetTapeParameters();

/*
 * Declare the Show Functions:
 */
extern int ShowCapacity(), ShowExpression(), ShowInquiry(), ShowSense();
extern int ShowModeParamHeaders();

/*
 * Show Audio Specific Functions:
 */
extern int DoCdromCmd();
extern int ShowCatalog(), ShowChannel(), ShowLBAHeader();
extern int ShowTOC(), ShowTOCHeader();
extern int ShowTOCEntry(), ShowTOCFull(), ShowTOCSummary();
extern int ShowPlayPosition(), ShowPlayStatus();
extern int ShowISRC();

/*
 * Set Audio Specific Functions:
 */
extern int SetAddressFormat(), SetPosition(), SetVolume(), SkipTracks();
extern int SetSonyVolume();

/*
 * Miscellaneous Functions:
 */
extern int SourceInputFile(), SwitchDeviceEntry();
extern int SetCamFlags(), SetOutputFile(), SetOutputLog();
extern int SetRadixDec(), SetRadixHex(), SleepTime();

/*
 * Diagnostic Specific Functions:
 */
extern int DoAllTests(), DoSelfTests(), ControllerTests(), DriveTests();
extern int DoMemoryTests(), ShowMemorySize();
extern int DoSimpleCommand(), DoTestUnitReady();
extern int DownloadMicrocode();
extern int SetRecovery();

/*
 * Data I/O Specific Functions:
 */
extern int EraseMedia(), ReadMedia(), ScanMedia(), VerifyMedia(), WriteMedia();

/*
 * Direct-Access Specific Functions:
 */
extern int FormatUnit(), ReassignBlock(), ShowDefects();

/*
 * CAM Specific Functions:
 */
extern int ReleaseSIMQ(), ResetBus(), ResetDevice();
extern int SetDeviceNexus(), SetDeviceType();
extern int ScanEdtEntrys(), ShowEdtEntrys();
extern int ShowDeviceNexus(), ShowDeviceType(), ShowPathInquiry();

/*
 * XZA-specific routines
 */
extern int XZAReadCnt(), XZAShowParams();
extern int XZASoftErrs(), XZAHardErrs();
extern int XZASetId(), XZAShowNCRRegs(), XZAWriteImage(), XZAReadImage();

/*
 * Forward References:
 */
extern struct cmd_entry CmdTable[];
extern struct key_entry cam_keytable[];
extern struct key_entry change_keytable[];
extern struct key_entry download_keytable[];
extern struct key_entry magtape_keytable[];
extern struct key_entry read_keytable[];
extern struct key_entry release_keytable[];
extern struct key_entry reserve_keytable[];
extern struct key_entry reset_keytable[];
extern struct key_entry scan_keytable[];
extern struct key_entry set_keytable[];
extern struct key_entry show_keytable[];
extern struct key_entry test_keytable[];
extern struct key_entry verify_keytable[];
extern struct key_entry write_keytable[];
extern struct key_entry xza_keytable[];
extern struct key_entry xza_show_keytable[];
extern struct key_entry xza_set_keytable[];
extern struct key_entry xza_errors_keytable[];
#if defined(NOT_IMPLEMENTED)
extern struct key_entry xza_read_keytable[];
extern struct key_entry xza_write_keytable[];
#endif /* defined(NOT_IMPLEMENTED) */
extern struct key_entry set_tape_keytable[];
extern struct key_entry tape_blocking_keytable[];
extern struct key_entry tape_density_keytable[];

extern struct key_entry format_keytable[];
extern struct key_entry format_defects_keytable[];
extern struct key_entry defects_keytable[];
extern struct key_entry defect_format_keytable[];
extern struct key_entry density_keytable[];
extern struct key_entry device_type_keytable[];
extern struct key_entry direct_keytable[];
extern struct key_entry iodefaults_keytable[];
extern struct key_entry ioparams_keytable[];
extern struct key_entry memory_keytable[];
extern struct key_entry nexus_keytable[];
extern struct key_entry report_format_keytable[];
extern struct key_entry page_keytable[];
extern struct key_entry pcf_keytable[];
extern struct key_entry eunits_keytable[];
extern struct key_entry sunits_keytable[];

/*
 * Audio Specific Keytables:
 */
extern struct key_entry play_keytable[];
extern struct key_entry toc_keytable[];
extern struct key_entry skip_keytable[];
extern struct key_entry set_audio_keytable[];
extern struct key_entry show_audio_keytable[];

extern struct key_entry address_keytable[];
extern struct key_entry audio_sony_keytable[];
extern struct key_entry volume_keytable[];
extern struct key_entry chan0_keytable[];
extern struct key_entry chan1_keytable[];

/*
 * Define the Command Table:
 */
struct cmd_entry CmdTable[] =
{
    {	"allow",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_ALLOW_REMOVAL,			/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	SCSI_ALLOW_REMOVAL,			/* c_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) | 		/* c_spec */
	 BITMASK(ALL_DTYPE_OPTICAL) |
	 BITMASK(ALL_DTYPE_RODIRECT) | 
	 BITMASK(ALL_DTYPE_SEQUENTIAL)), 
	(caddr_t) 0,				/* c_argp */
	"allow removal",			/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"change",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_CHANGE,				/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	ALL_DEVICE_TYPES,			/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"change",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	change_keytable,			/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"download",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_DOWNLOAD,				/* c_token */
	TYPE_STRING,				/* c_type */
	(PF_CONTROL | PF_DYNAMIC | PF_KEYS_OPT),/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DownloadMicrocode,			/* c_func */
	0,					/* c_cmd */
	ALL_DEVICE_TYPES,			/* c_spec */
	(caddr_t) &InputFileName,		/* c_argp */
	(caddr_t) 0,				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	download_keytable,			/* c_keyp */
	10,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"eject",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_EJECT_CADDY,				/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	CDROM_EJECT_CADDY,			/* c_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) | 		/* c_spec */
	 BITMASK(ALL_DTYPE_OPTICAL) |
	 BITMASK(ALL_DTYPE_RODIRECT)),
	(caddr_t) 0,				/* c_argp */
	"eject caddy/media",			/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"evaluate",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_EVALUATE,				/* c_token */
	TYPE_VALUE,				/* c_type */
	PF_SPECIAL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	ShowExpression,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) &ExpressionValue,		/* c_argp */
	"evalute expression",			/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"exit",					/* c_name */
	"quit",					/* c_alias */
	C_EXIT,					/* c_token */
	TYPE_SET_FLAG,				/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	(caddr_t) TRUE,				/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) &ExitFlag,			/* c_argp */
	(caddr_t) 0,				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"format",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_FORMAT_UNIT,				/* c_token */
	0,					/* c_type */
	(PF_KEYS_OPT | PF_EXACT | PF_CONTROL),	/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	FormatUnit,				/* c_func */
	SCSI_FORMAT_UNIT,			/* c_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* c_spec */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	(caddr_t) 0,				/* c_argp */
	"format unit",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	format_keytable,			/* c_keyp */
	2,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"help",					/* c_name */
	"?",					/* c_alias */
	C_HELP,					/* c_token */
	TYPE_STRING,				/* c_type */
	PF_ARGS_OPT,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	Help,					/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) &HelpTopic,			/* c_argp */
	(caddr_t) 0,				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"mt",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_MT,					/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	0,					/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"mt",					/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	magtape_keytable,			/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"pause",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_PAUSE_PLAY,				/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	CDROM_PAUSE_PLAY,			/* c_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"pause play",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"play",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_PLAY,					/* c_token */
	0,					/* c_type */
	(PF_KEYS_OPT | PF_DEFAULT | PF_CONTROL), /* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	CDROM_PLAY_AUDIO_TI,			/* c_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"play audio",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	play_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"prevent",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_PREVENT_REMOVAL,			/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	SCSI_PREVENT_REMOVAL,			/* c_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) | 		/* c_spec */
	 BITMASK(ALL_DTYPE_OPTICAL) |
	 BITMASK(ALL_DTYPE_RODIRECT) | 
	 BITMASK(ALL_DTYPE_SEQUENTIAL)), 
	(caddr_t) 0,				/* c_argp */
	"prevent removal",			/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"read",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_READ,					/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"read",					/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	read_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"reassign",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_REASSIGN_BLOCK,			/* c_token */
	0,					/* c_type */
	(PF_EXACT | PF_CONTROL),		/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	ReassignBlock,				/* c_func */
	SCSI_REASSIGN_BLOCK,			/* c_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* c_spec */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	(caddr_t) 0,				/* c_argp */
	"reassign block",			/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	direct_keytable,			/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"reset",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_RESET,				/* c_token */
	0,					/* c_type */
	PF_EXACT,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"reset",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	reset_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"release",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_RELEASE,				/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"release",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	release_keytable,			/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"reserve",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_RESERVE,				/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"reserve",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	reserve_keytable,			/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"resume",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_RESUME_PLAY,				/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	CDROM_RESUME_PLAY,			/* c_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"resume play",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"show",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_SHOW,					/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"show",					/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	show_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"scan",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_SCAN,					/* c_token */
	0,					/* k_type */
	0,					/* k_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"scan",					/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	scan_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"set",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_SET,					/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"set",					/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	set_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"skip",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_SKIP,					/* c_token */
	0,					/* c_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	0,					/* c_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"skip",					/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	skip_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"sleep",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_SLEEP,				/* c_token */
	TYPE_VALUE,				/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	SleepTime,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) &SleepValue,			/* c_argp */
	"sleep time",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"source",				/* c_name */
	"@",					/* c_alias */
	C_SOURCE,				/* c_token */
	TYPE_STRING,				/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	SourceInputFile,			/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) &SourceFileName,		/* c_argp */
	(caddr_t) 0,				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"start",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_START_UNIT,				/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	SCSI_START_UNIT,			/* c_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) | 		/* c_spec */
	 BITMASK(ALL_DTYPE_OPTICAL) |
	 BITMASK(ALL_DTYPE_RODIRECT)), 
	(caddr_t) 0,				/* c_argp */
	"start unit",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"stop",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_STOP_UNIT,				/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoCdromCmd,				/* c_func */
	SCSI_STOP_UNIT,				/* c_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) | 		/* c_spec */
	 BITMASK(ALL_DTYPE_OPTICAL) |
	 BITMASK(ALL_DTYPE_RODIRECT)), 
	(caddr_t) 0,				/* c_argp */
	"stop unit",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"switch",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_SWITCH,				/* c_token */
	TYPE_STRING,				/* c_type */
	(PF_ARGS_OPT | PF_DYNAMIC),		/* c_flags */
	0,					/* c_cflags */
	(caddr_t) 0,				/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	SwitchDeviceEntry,			/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) &UserDeviceName,		/* c_argp */
	(caddr_t) 0,				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"test",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_TEST,					/* c_token */
	0,					/* c_type */
	(PF_KEYS_OPT | PF_DEFAULT | PF_CONTROL), /* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoSelfTests,				/* c_func */
	0,					/* c_cmd */
	ALL_DEVICE_TYPES,			/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"test",					/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	test_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"tur",					/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_TUR,					/* c_token */
	0,					/* c_type */
	PF_CONTROL,				/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	DoTestUnitReady,			/* c_func */
	0,					/* c_cmd */
	ALL_DEVICE_TYPES,			/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"test unit ready",			/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	0,					/* c_keyp */
	0,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"verify",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_VERIFY,				/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"verify",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	verify_keytable,			/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"write",				/* c_name */
	(caddr_t) 0,				/* c_alias */
	C_WRITE,				/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"write",				/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	write_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    {	"xza",					/* c_name */
	"skz",					/* c_alias */
	C_XZA,					/* c_token */
	0,					/* c_type */
	0,					/* c_flags */
	0,					/* c_cflags */
	0,					/* c_default */
	0,					/* c_min */
	0,					/* c_max */
	(int (*)()) 0,				/* c_func */
	0,					/* c_cmd */
	0,					/* c_spec */
	(caddr_t) 0,				/* c_argp */
	"xza",					/* c_msgp */
	(caddr_t) 0,				/* c_strp */
	xza_keytable,				/* c_keyp */
	1,					/* c_keyc */
	{ 0 }					/* c_kopts */
    },
    { 0 }				/* End of Command Table */
};



/*
 * Define the Change Keyword Table:
 */
struct key_entry change_keytable[] =
{
    {	"pages",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PAGES,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ProcessModePage,			/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	page_keytable,				/* k_keyp */
	2					/* k_keyc */
    },
    { 0 }				/* End of Change KeyTable */
};

/*
 * Define the Magtape Keyword Table:
 */
struct key_entry magtape_keytable[] =
{
    {	"bsf",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BSF,					/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_CONTROL | PF_ARGS_OPT),		/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTBSF,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) &MtopCount,			/* k_argp */
	"backward space file",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"bsr",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BSR,					/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_CONTROL | PF_ARGS_OPT),		/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTBSR,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) &MtopCount,			/* k_argp */
	"backward space record",		/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"erase",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ERASE,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTERASE,				/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"erase tape",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"fsf",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_FSF,					/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_CONTROL | PF_ARGS_OPT),		/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTFSF,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) &MtopCount,			/* k_argp */
	"forward space file",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"fsr",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_FSR,					/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_CONTROL | PF_ARGS_OPT),		/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTFSR,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) &MtopCount,			/* k_argp */
	"forward space record",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"load",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_LOAD,					/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTLOAD,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"load tape",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"rewind",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_REWIND,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTREW,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"rewind tape",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"offline",				/* k_name */
	"rewoffl",				/* k_alias */
	K_UNLOAD,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTOFFL,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"unload tape",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"online",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ONLINE,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTONLINE,				/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"load tape",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"retension",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RETENSION,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTRETEN,				/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"retension tape",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"seod",					/* k_name */
	"seorm",				/* k_alias */
	K_SEOD,					/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTSEOD,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"space end of data",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"unload",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_UNLOAD,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTUNLOAD,				/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"unload tape",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"weof",					/* k_name */
	"eof",					/* k_alias */
	K_WEOF,					/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_CONTROL | PF_ARGS_OPT),		/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMtCmd,				/* k_func */
	MTWEOF,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) &MtopCount,			/* k_argp */
	"write file mark",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Magtape KeyTable */
};

/*
 * Define the Read Keyword Table:
 */
struct key_entry read_keytable[] =
{
    {	"media",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MEDIA,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ReadMedia,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read media",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	ioparams_keytable,			/* k_keyp */
	25					/* k_keyc */
    },
    { 0 }				/* End of Read KeyTable */
};

/*
 * Define the Scan Keyword Table:
 */
struct key_entry scan_keytable[] =
{
    {	"edt",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_EDT,					/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ScanEdtEntrys,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"scan EDT",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"media",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MEDIA,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ScanMedia,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"scan media",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	ioparams_keytable,			/* k_keyp */
	25					/* k_keyc */
    },
    { 0 }				/* End of Scan KeyTable */
};

/*
 * Define the Verify Keyword Table:
 */
struct key_entry verify_keytable[] =
{
    {	"media",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MEDIA,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	VerifyMedia,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"verify media",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	ioparams_keytable,			/* k_keyp */
	25					/* k_keyc */
    },
    { 0 }				/* End of Verify KeyTable */
};

/*
 * Define the Write Keyword Table:
 */
struct key_entry write_keytable[] =
{
    {	"media",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MEDIA,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	WriteMedia,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"write media",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	ioparams_keytable,			/* k_keyp */
	25					/* k_keyc */
    },
    { 0 }				/* End of Write KeyTable */
};

/*
 * Define the Play Keyword Table:
 */
struct key_entry play_keytable[] =
{
    {	"starting_track",			/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_STARTING_TRACK,			/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_ARGS_OPT | PF_CONTROL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_TRACK,			/* k_min */
	CDROM_MAX_TRACK,			/* k_max */
	PlayRange,				/* k_func */
	0,					/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &StartingTrack,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	play_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"ending_track",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ENDING_TRACK,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_ARGS_OPT | PF_CONTROL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_TRACK,			/* k_min */
	CDROM_MAX_TRACK,			/* k_max */
	PlayRange,				/* k_func */
	0,					/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &EndingTrack,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"track",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TRACK,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_TRACK,			/* k_min */
	CDROM_MAX_TRACK,			/* k_max */
	PlayTrack,				/* k_func */
	CDROM_PLAY_AUDIO_TI,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &TrackNumber,			/* k_argp */
	"play audio track/index",		/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"audio",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_AUDIO_LBA,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	PlayAudio,				/* k_func */
	CDROM_PLAY_AUDIO,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &LogicalBlock,		/* k_argp */
	"play audio",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	direct_keytable,			/* k_keyp */
	2					/* k_keyc */
    },
    {	"msf",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_AUDIO_MSF,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	PlayAudioMSF,				/* k_func */
	CDROM_PLAY_AUDIO_MSF,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"play audio msf",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	sunits_keytable,			/* k_keyp */
	8					/* k_keyc */
    },
    {	"sony",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VENDOR_UNIQUE,			/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	audio_sony_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    { 0 }				/* End of Play KeyTable */
};

/*
 * Define the Set Keyword Table:
 */
struct key_entry set_keytable[] =
{
    {	"bus",					/* k_name */
	"skz",					/* k_alias */
	K_BUS,					/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserBus,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"audio",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_AUDIO,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	set_audio_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"bypass",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DONT_CARE,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BypassFlag,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"cam",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CAM,					/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	cam_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"debug",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DEBUG,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DebugFlag,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"defaults",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DEFAULTS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	iodefaults_keytable,			/* k_keyp */
	25					/* k_keyc */
    },
    {	"dump",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DUMP,					/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DumpFlag,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"device",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DEVICE_TYPE,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetDeviceType,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"set device type",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	device_type_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"dump-limit",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DUMP_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &cdbg_DumpLimit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"log",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_OUTPUT_LOG,				/* k_token */
	TYPE_STRING,				/* k_type */
	PF_DYNAMIC,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetOutputLog,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &OutputLogName,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"nexus",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_NEXUS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetDeviceNexus,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"set device nexus",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"pages",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PAGES,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ProcessModePage,			/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	page_keytable,				/* k_keyp */
	2					/* k_keyc */
    },
    {	"pager",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_OUTPUT_PAGER,				/* k_token */
	TYPE_STRING,				/* k_type */
	PF_DYNAMIC,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &OutputPager,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0    					/* k_keyc */
    },
    {	"paging",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PAGING_FLAG,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &PagingFlag,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"recovery",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RECOVERY_FLAG,			/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetRecovery,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) &RecoveryFlag,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"position",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SEEK_POSITION,			/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetPosition,				/* k_func */
	SCSI_SEEK_POSITION,			/* k_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) | 		/* k_spec */
	 BITMASK(ALL_DTYPE_RODIRECT)), 
	(caddr_t) &LogicalBlock,		/* k_argp */
	"seek position",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	direct_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"tape",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_AUDIO,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	BITMASK(ALL_DTYPE_SEQUENTIAL),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	set_tape_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"verbose",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VERBOSE,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DisplayVerbose,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"watch",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_WATCH_PROGRESS,			/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &WatchProgress,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Set KeyTable */
};

/*
 * Define Set Audio Keyword Table:
 */
struct key_entry set_audio_keytable[] =
{
    {	"address",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ADDRESS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"volume",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VOLUME,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetVolume,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	volume_keytable,			/* k_keyp */
	6					/* k_keyc */
    },
    {	"sony",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VENDOR_UNIQUE,			/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	audio_sony_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    { 0 }				/* End of Set Audio KeyTable */
};

/*
 * Define the Show Keyword Table:
 */
struct key_entry show_keytable[] =
{
    {	"audio",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_AUDIO,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	show_audio_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"capacity",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CAPACITY,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowCapacity,				/* k_func */
	SCSI_READ_CAPACITY,			/* k_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) | 		/* k_spec */
	 BITMASK(ALL_DTYPE_OPTICAL) |
	 BITMASK(ALL_DTYPE_RODIRECT)), 
	(caddr_t) 0,				/* k_argp */
	"read capacity",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"defects",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DEFECT_LIST,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowDefects,				/* k_func */
	SCSI_READ_DEFECT_DATA,			/* k_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* k_spec */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	(caddr_t) 0,				/* k_argp */
	"read defect list",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	defects_keytable,			/* k_keyp */
	2					/* k_keyc */
    },
    {	"device",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DEVICE_TYPE,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowDeviceType,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"get device type",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"edt",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_EDT,					/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowEdtEntrys,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"show EDT",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	4					/* k_keyc */
    },
    {	"inquiry",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_INQUIRY,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowInquiry,				/* k_func */
	SCSI_GET_INQUIRY_DATA,			/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"get inquiry data",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"memory",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MEMORY_SIZE,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowMemorySize,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	memory_keytable,			/* k_keyp */
	10					/* k_keyc */
    },
    {	"mode-parameters",			/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MODE_PARAMETERS,			/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowModeParamHeaders,			/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"nexus",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_NEXUS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowDeviceNexus,			/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"show device nexus",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"pages",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PAGES,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ProcessModePage,			/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	page_keytable,				/* k_keyp */
	2					/* k_keyc */
    },
    {	"path-inquiry",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PATH_INQUIRY,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowPathInquiry,			/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"path inquiry",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"sense",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SENSE,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowSense,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"show sense",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Show KeyTable */
};

/*
 * Define Show Audio Keyword Table:
 */
struct key_entry show_audio_keytable[] =
{
    {	"channel",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CHANNEL,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowChannel,				/* k_func */
	CDROM_READ_SUBCHANNEL,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read sub-channel",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1    					/* k_keyc */
    },
    {	"catalog",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CATALOG,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowCatalog,				/* k_func */
	CDROM_READ_SUBCHANNEL,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read sub-channel",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1    					/* k_keyc */
    },
    {	"isrc",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ISRC,					/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowISRC,				/* k_func */
	CDROM_READ_SUBCHANNEL,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read sub-channel",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1    					/* k_keyc */
    },
    {	"header",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_LBA_HEADER,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowLBAHeader,				/* k_func */
	CDROM_READ_HEADER,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &LogicalBlock,		/* k_argp */
	"read header",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	direct_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"position",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PLAY_POSITION,			/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowPlayPosition,			/* k_func */
	CDROM_READ_SUBCHANNEL,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read sub-channel",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1    					/* k_keyc */
    },
    {	"status",				/* k_name */
	"volume",				/* k_alias */
	K_PLAY_STATUS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowPlayStatus,				/* k_func */
	CDROM_PLAYBACK_STATUS,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"playback status",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1    					/* k_keyc */
    },
    {	"toc",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TOC,					/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowTOC,				/* k_func */
	CDROM_TOC_ENTRYS,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read TOC entries",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	toc_keytable,				/* k_keyp */
	1    					/* k_keyc */
    },
    { 0 }				/* End of Show Audio KeyTable */
};

/*
 * Define the Skip Keyword Table:
 */
struct key_entry skip_keytable[] =
{
    {	"backward",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BACKWARD_TRACK,			/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_ARGS_OPT | PF_CONTROL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_TRACK,			/* k_min */
	CDROM_MAX_TRACK,			/* k_max */
	SkipTracks,				/* k_func */
	0,					/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &BackwardTracks,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"forward",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_FORWARD_TRACK,			/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_ARGS_OPT | PF_CONTROL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_TRACK,			/* k_min */
	CDROM_MAX_TRACK,			/* k_max */
	SkipTracks,				/* k_func */
	0,					/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &ForwardTracks,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Skip KeyTable */
};

/*
 * Define the Address Keyword Table:
 */
struct key_entry address_keytable[] =
{
    {	"format",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ADDRESS_FORMAT,			/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetAddressFormat,			/* k_func */
	CDROM_SET_ADDRESS_FORMAT,		/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &AddressFormat,		/* k_argp */
	"set address format",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"lba",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_LBA,					/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) CDROM_LBA_FORMAT,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &AddressFormat,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"msf",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MSF,					/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) CDROM_MSF_FORMAT,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &AddressFormat,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { (caddr_t) PC$_LAMBDA },		/* Optional */
    { 0 }				/* End of Address KeyTable */
};

/*
 * Define the Direct Keyword Table:
 */
struct key_entry direct_keytable[] =
{
    {	"lba",					/* k_name */
	"lbn",					/* k_alias */
	K_LBA,					/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &LogicalBlock,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"length",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_LENGTH,				/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &TransferLength,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Audio KeyTable */
};

/*
 * Define the Mode Page Keyword Table:
 */
struct key_entry page_keytable[] =
{
    {	"error-recovery",			/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ERROR_RECOVERY,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ERROR_RECOVERY_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"disconnect",				/* k_name */
	"reconnect",				/* k_alias */
	K_DISCO_RECO,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) DISCO_RECO_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"direct-access",			/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DIRECT_ACCESS,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) DIRECT_ACCESS_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"geometry",				/* k_name */
	"rigid",				/* k_alias */
	K_DISK_GEOMETRY,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) DISK_GEOMETRY_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"flexible",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_FLEXIBLE_DISK,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) FLEXIBLE_DISK_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"audio-control",			/* k_name */
	"volume",				/* k_alias */
	K_AUDIO_CONTROL,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) AUDIO_CONTROL_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"cdrom",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CDROM_PARAMS,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) CDROM_DEVICE_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"cache-control",			/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CACHE_CONTROL,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) CACHE_CONTROL_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"dec-specific",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DEC_SPECIFIC,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) DEC_SPECIFIC_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"readahead-control",			/* k_name */
	"rac",					/* k_alias */
	K_READAHEAD_CONTROL,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) READAHEAD_CONTROL_PAGE, 	/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"device-configuration",			/* k_name */
	"configuration",			/* k_alias */
	K_DEVICE_CONFIG,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) DEVICE_CONFIG_PAGE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"medium-partition1",			/* k_name */
	"part1",				/* k_alias */
	K_MEDIUM_PARTITION_1,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) MEDIUM_PART_PAGE1,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"code",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MODE_PAGE,				/* k_token */
	TYPE_HEX,				/* k_type */
	(PF_RANGE | PF_KEYS_OPT),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0xff,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ModePageCode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    {	"pcf",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PCF,					/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	pcf_keytable,				/* k_keyp */
	1					/* k_keyc */
    },
    { 0 }				/* End of Mode Page KeyTable */
};

/*
 * Define the Mode Sense Page Control Field (pcf) Keyword Table:
 */
struct key_entry pcf_keytable[] =
{
    {	"current",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PCF_CURRENT,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) PCF_CURRENT,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetRadixDec,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &PageControlField,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"changeable",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PCF_CHANGEABLE,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) PCF_CHANGEABLE,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetRadixHex,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &PageControlField,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"default",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PCF_DEFAULT,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) PCF_DEFAULT,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetRadixDec,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &PageControlField,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"saved",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_PCF_SAVED,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) PCF_SAVED,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetRadixDec,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &PageControlField,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { (caddr_t) PC$_LAMBDA },		/* Optional */
    { 0 }				/* End of PCF KeyTable */
};

/*
 * Define the CAM Keyword Table:
 */
struct key_entry cam_keytable[] =
{
#if defined(CAMDEBUG)
    {	"debug",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CAM_DEBUG,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &camdbg_flag,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
#endif /* defined(CAMDEBUG) */
    {	"flags",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CCB_FLAGS,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetCamFlags,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &CamFlags,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"retrys",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CAM_RETRYS,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &CamRetrysFlag,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"timeout",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DONT_CARE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &CamTimeout,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of CAM KeyTable */
};

/*
 * Define the Read Defects Keyword Table:
 */
struct key_entry defects_keytable[] =
{
    {	"all",					/* k_name */
	"known",				/* k_alias */
	K_ALL_DEFECTS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	defect_format_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"primary",				/* k_name */
	"vendor",				/* k_alias */
	K_VENDOR_DEFECTS,			/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	defect_format_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"grown",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_GROWN_DEFECTS,			/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	defect_format_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    { 0 }				/* End of Defects KeyTable */
};

/*
 * Define the Defect Format Keyword Table:
 */
struct key_entry defect_format_keytable[] =
{
    {	"block",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BLOCK_FORMAT,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) BLK_FORMAT,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefectFormat,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"bfi",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BFI_FORMAT,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) BFI_FORMAT,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefectFormat,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"sector",				/* k_name */
	"physical",				/* k_alias */
	K_PHYSICAL_FORMAT,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) PHY_FORMAT,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefectFormat,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of defect format KeyTable */
};

/*
 * Define the Format Keyword Table:
 */
struct key_entry format_keytable[] =
{
    {	"density",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DENSITY,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	density_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"defects",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DEFECT_LIST,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	(BITMASK(ALL_DTYPE_DIRECT) |		/* k_spec */
	 BITMASK(ALL_DTYPE_OPTICAL)),
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	format_defects_keytable,		/* k_keyp */
	1					/* k_keyc */
    },
    { 0 }				/* End of Format KeyTable */
};

/*
 * Define the Diskette Density Keyword Table:
 */
struct key_entry density_keytable[] =
{
    {	"RX50",					/* k_name */
	"400KB",				/* k_alias */
	K_FORMAT_RX50,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) V_FORMAT_RX50,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityType,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"LD",					/* k_name */
	"low",					/* k_alias */
	K_FORMAT_LD,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) V_FORMAT_LD,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityType,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"DD",					/* k_name */
	"double",				/* k_alias */
	K_FORMAT_DD,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) V_FORMAT_DD,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityType,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"HD",					/* k_name */
	"high",					/* k_alias */
	K_FORMAT_HD,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) V_FORMAT_HD,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityType,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"HD5",					/* k_name */
	"high5",				/* k_alias */
	K_FORMAT_HD5,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) V_FORMAT_HD5,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityType,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"ED",					/* k_name */
	"extra",				/* k_alias */
	K_FORMAT_ED,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) V_FORMAT_ED,			/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityType,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"other",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_FORMAT_OTHER,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) V_FORMAT_OTHER,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityType,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Density KeyTable */
};

/*
 * Define the Format Defects Keyword Table:
 */
struct key_entry format_defects_keytable[] =
{
    {	"all",					/* k_name */
	"known",				/* k_alias */
	K_ALL_DEFECTS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"primary",				/* k_name */
	"vendor",				/* k_alias */
	K_VENDOR_DEFECTS,			/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"grown",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_GROWN_DEFECTS,			/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"none",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_NO_DEFECTS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Format Defects KeyTable */
};

/*
 * Define the End Units Keyword Table:
 */
struct key_entry eunits_keytable[] =
{
    {	"ending",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ENDING_UNITS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	eunits_keytable,			/* k_keyp */
	3					/* k_keyc */
    },
    {	"minute-units",				/* k_name */
	"mu",					/* k_alias */
	K_ENDING_M_UNITS,			/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Ending_M_Unit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"second-units",				/* k_name */
	"su",					/* k_alias */
	K_ENDING_S_UNITS,			/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Ending_S_Unit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"frame-units",				/* k_name */
	"fu",					/* k_alias */
	K_ENDING_F_UNITS,			/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Ending_F_Unit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Ending Units KeyTable */
};

/*
 * Define the Starting Units Keyword Table:
 */
struct key_entry sunits_keytable[] =
{
    {	"starting",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_STARTING_UNITS,			/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	sunits_keytable,			/* k_keyp */
	3					/* k_keyc */
    },
    {	"ending",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ENDING_UNITS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	eunits_keytable,			/* k_keyp */
	3					/* k_keyc */
    },
    {	"minute-units",				/* k_name */
	"mu",					/* k_alias */
	K_STARTING_M_UNITS,			/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Starting_M_Unit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"second-units",				/* k_name */
	"su",					/* k_alias */
	K_STARTING_S_UNITS,			/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Starting_S_Unit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"frame-units",				/* k_name */
	"fu",					/* k_alias */
	K_STARTING_F_UNITS,			/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Starting_F_Unit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Starting Units KeyTable */
};

/*
 * Define the Table of Contents Keyword Table:
 */
struct key_entry toc_keytable[] =
{
    {	"entry",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TOC_ENTRY,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_KEYS_OPT | PF_CONTROL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_TRACK,			/* k_min */
	0xff,					/* k_max */
	ShowTOCEntry,				/* k_func */
	CDROM_TOC_ENTRYS,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &TrackNumber,			/* k_argp */
	"read TOC entry",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1    					/* k_keyc */
    },
    {	"header",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TOC_HEADER,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowTOCHeader,				/* k_func */
	CDROM_TOC_HEADER,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read TOC header",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"full",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TOC_FULL,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowTOCFull,				/* k_func */
	CDROM_TOC_ENTRYS,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read TOC entries",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1    					/* k_keyc */
    },
    {	"summary",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TOC_SUMMARY,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ShowTOCSummary,				/* k_func */
	CDROM_TOC_ENTRYS,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"read TOC entries",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	address_keytable,			/* k_keyp */
	1    					/* k_keyc */
    },
    { 0 }				/* End of TOC KeyTable */
};

/*
 * Define the Sony Audio Vendor Unique Keyword Table:
 */
struct key_entry audio_sony_keytable[] =
{
    {	"indexes",				/* k_name */
	"ni",					/* k_alias */
	K_PLAY_INDEXES,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_KEYS_OPT | PF_CONTROL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_TRACK,			/* k_min */
	CDROM_MAX_TRACK,			/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &NumberIndexes,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"track",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TRACK,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_KEYS_OPT | PF_CONTROL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_TRACK,			/* k_min */
	CDROM_MAX_TRACK,			/* k_max */
	PlayVTrack,				/* k_func */
	CDROM_PLAY_TRACK,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &TrackNumber,			/* k_argp */
	"play track",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	audio_sony_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"audio",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_AUDIO_LBA,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	PlayAudio,				/* k_func */
	CDROM_PLAY_VAUDIO,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) &LogicalBlock,		/* k_argp */
	"play audio lba",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	direct_keytable,			/* k_keyp */
	2					/* k_keyc */
    },
    {	"msf",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_AUDIO_MSF,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	PlayAudioMSF,				/* k_func */
	CDROM_PLAY_MSF,				/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"play audio msf",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	sunits_keytable,			/* k_keyp */
	8					/* k_keyc */
    },
    {	"volume",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VOLUME,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetSonyVolume,				/* k_func */
	CDROM_PLAYBACK_CONTROL,			/* k_cmd */
	BITMASK(ALL_DTYPE_RODIRECT),		/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"playback control",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	volume_keytable,			/* k_keyp */
	6					/* k_keyc */
    },
    { 0 }				/* End of Vendor KeyTable */
};

/*
 * Define the Volume Control Keyword Table:
 */
struct key_entry volume_keytable[] =
{
    {	"channel-0",				/* k_name */
	"ch0",					/* k_alias */
	K_CHANNEL_0,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	chan0_keytable,				/* k_keyp */
	2					/* k_keyc */
    },
    {	"channel-1",				/* k_name */
	"ch1",					/* k_alias */
	K_CHANNEL_1,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	chan1_keytable,				/* k_keyp */
	2					/* k_keyc */
    },
    {	"level",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VOLUME_LEVEL,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_VOLUME,			/* k_min */
	CDROM_MAX_VOLUME,			/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &VolumeLevel,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Volume KeyTable */
};

/*
 * Define the Channel 0 Keyword Table:
 */
struct key_entry chan0_keytable[] =
{
    {	"port-select",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CHAN0_SELECT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_PORT_MUTED,			/* k_min */
	CDROM_CHANNEL_0_1,			/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Channel_0_Select,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"level",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CHAN0_VOLUME,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_VOLUME,			/* k_min */
	CDROM_MAX_VOLUME,			/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Channel_0_Volume,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { (caddr_t) PC$_LAMBDA },		/* Optional */
    { 0 }				/* End of Volume KeyTable */
};

/*
 * Define the Channel 1 Keyword Table:
 */
struct key_entry chan1_keytable[] =
{
    {	"port-select",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CHAN1_SELECT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_PORT_MUTED,			/* k_min */
	CDROM_CHANNEL_0_1,			/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Channel_1_Select,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"level",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CHAN1_VOLUME,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	CDROM_MIN_VOLUME,			/* k_min */
	CDROM_MAX_VOLUME,			/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &Channel_1_Volume,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { (caddr_t) PC$_LAMBDA },		/* Optional */
    { 0 }				/* End of Volume KeyTable */
};

/*
 * Define the Test Keyword Table:
 */
struct key_entry test_keytable[] =
{
    {	"selftest",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SELF_TESTS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoSelfTests,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"controller",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_CONTROLLER_TESTS,			/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ControllerTests,			/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"drive",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DRIVE_TESTS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DriveTests,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"memory",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_MEMORY_TESTS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoMemoryTests,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	memory_keytable,			/* k_keyp */
	10					/* k_keyc */
    },
    { 0 }				/* End of Test KeyTable */
};

/*
 * Define the Download Keyword Table:
 */
struct key_entry download_keytable[] =
{
    {	"save",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SAVE_DATA,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	(caddr_t) PC$_SHARED_TABLE,		/* k_name */
	(caddr_t) 0,				/* k_alias */
	PC_SHARED_TABLE,			/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) "download",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	memory_keytable,			/* k_keyp */
	10					/* k_keyc */
    },
    { 0 }				/* End of Download KeyTable */
};

/*
 * Define the Release Keyword Table:
 */
struct key_entry release_keytable[] =
{
    {	"simqueue",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SIMQ,					/* k_token */
	0,					/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ReleaseSIMQ,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"release SIMQ",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"device",				/* k_name */
	"unit",					/* k_alias */
	K_DEVICE,				/* k_token */
	0,					/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoSimpleCommand,			/* k_func */
	SCMD_RELEASE,				/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"release device",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Release KeyTable */
};

/*
 * Define the Reserve Keyword Table:
 */
struct key_entry reserve_keytable[] =
{
    {	"device",				/* k_name */
	"unit",					/* k_alias */
	K_DEVICE,				/* k_token */
	0,					/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	0,					/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	DoSimpleCommand,			/* k_func */
	SCMD_RESERVE,				/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"reserve device",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Reserve KeyTable */
};

/*
 * Define the Reset Keyword Table:
 */
struct key_entry reset_keytable[] =
{
    {	"bus",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RESET_BUS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ResetBus,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserBus,			/* k_argp */
	"reset bus",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"device",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RESET_DEVICE,				/* k_token */
	TYPE_ACTION,				/* k_type */
	(PF_KEYS_OPT | PF_CONTROL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	ResetDevice,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"reset device",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    { 0 }				/* End of Reset KeyTable */
};

/*
 * Define the Nexus Keyword Table:
 */
struct key_entry nexus_keytable[] =
{
    {	"bus",					/* k_name */
	"pid",					/* k_alias */
	K_BUS,					/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	MAX_SCSI_BUS,				/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserBus,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"target",				/* k_name */
	"tid",					/* k_alias */
	K_TARGET,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	MAX_SCSI_TARGET,			/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserTarget,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"lun",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_LUN,					/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	MAX_SCSI_LUN,				/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserLun,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	(caddr_t) PC$_LAMBDA,			/* k_name */
	(caddr_t) 0,				/* k_alias */
	PC_LAMBDA,				/* k_token */
	TYPE_NONE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	report_format_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    { 0 }				/* End of Nexus KeyTable */
};

/*
 * Define the Device Type Keyword Table:
 */
struct key_entry device_type_keytable[] =
{
    {	"direct-access",			/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_DIRECT,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_DIRECT,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"sequential-access",			/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_SEQUENTIAL,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_SEQUENTIAL,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"printer",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_PRINTER,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_PRINTER,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"processor",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_PROCESSOR,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_PROCESSOR,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"worm",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_WORM,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_WORM,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"rodirect",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_RODIRECT,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_RODIRECT,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"scanner",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_SCANNER,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_SCANNER,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"optical",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_OPTICAL,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_OPTICAL,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"changer",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_CHANGER,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_CHANGER,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    {	"communication",			/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DTYPE_COMM,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_KEYS_OPT,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) ALL_DTYPE_COMM,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &UserDeviceType,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	nexus_keytable,				/* k_keyp */
	3					/* k_keyc */
    },
    { 0 }				/* End of Nexus KeyTable */
};

/*
 * Define the Report Format Keyword Table:
 */
struct key_entry report_format_keytable[] =
{
    {	"full",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_REPORT_FULL,				/* k_token */
	TYPE_NONE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0    					/* k_keyc */
    },
    {	"summary",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_REPORT_SUMMARY,			/* k_token */
	TYPE_NONE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0    					/* k_keyc */
    },
    { (caddr_t) PC$_LAMBDA },		/* Optional */
    { 0 }				/* End of Report Format KeyTable */
};

/*
 * Define the I/O Parameters Keyword Table:
 */
struct key_entry ioparams_keytable[] =
{
    {	"align",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ALIGN_OFFSET,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &AlignOffset,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"bs",					/* k_name */
	"size",					/* k_alias */
	K_BLOCK_SIZE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BlockSize,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"compare",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_COMPARE_FLAG,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &CompareFlag,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"delay",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DELAY_VALUE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DelayValue,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"ending",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ENDING_LBA,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &EndingLogicalBlock,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"errors",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ERROR_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ErrorLimit,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"increment",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_INCREMENT_COUNT,			/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &IncrementCount,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"key",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RANDOMIZING_KEY,			/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &RandomizingKey,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"lba",					/* k_name */
	"lbn",					/* k_alias */
	K_LBA,					/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &LogicalBlock,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"length",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_LENGTH,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &TransferLength,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"limit",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DATA_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DataLimit,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"passes",				/* k_name */
	"iterations",				/* k_alias */
	K_PASS_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &PassLimit,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"pattern",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DATA_PATTERN,				/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DataPattern,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"random",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RANDOM_FLAG,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &RandomFlag,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"records",				/* k_name */
	"count",				/* k_alias */
	K_RECORD_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &RecordLimit,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"recovery",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RECOVERY_FLAG,			/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &TestRecoveryFlag,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"retrys",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RETRY_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &RetryLimit,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"seek",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SEEK_COUNT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &SeekCount,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"skip",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SKIP_COUNT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &SkipCount,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"starting",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_STARTING_LBA,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &StartingLogicalBlock,	/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"step",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_STEP_VALUE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &StepValue,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"verify",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VERIFY_LENGTH,			/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &VerifyLength,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of I/O Parameters KeyTable */
};

/*
 * Define the I/O Defaults Keyword Table:
 */
struct key_entry iodefaults_keytable[] =
{
    {	"align",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ALIGN_OFFSET,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultAlignOffset,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"bs",					/* k_name */
	"size",					/* k_alias */
	K_BLOCK_SIZE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultBlockSize,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"compare",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_COMPARE_FLAG,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultCompareFlag,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"delay",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DELAY_VALUE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultDelayValue,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"errors",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ERROR_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultErrorLimit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"increment",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_INCREMENT_COUNT,			/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultIncrementCount,	/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"key",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RANDOMIZING_KEY,			/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultRandomizingKey,	/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"limit",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DATA_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultDataLimit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"mode",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BUFFER_MODE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_SPECIAL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	7,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultBufferMode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"passes",				/* k_name */
	"iterations",				/* k_alias */
	K_PASS_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultPassLimit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"pattern",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DATA_PATTERN,				/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultDataPattern,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"random",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RANDOM_FLAG,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultRandomFlag,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"recovery",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RECOVERY_FLAG,			/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	PF_CONTROL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetRecovery,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) &RecoveryFlag,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"retrys",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RETRY_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultRetryLimit,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"seek",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SEEK_COUNT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultSeekCount,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"segment",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SEGMENT_SIZE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_ARGS_OPT | PF_SPECIAL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0xffffff,				/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultSegmentSize,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"skip",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SKIP_COUNT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultSkipCount,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"step",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_STEP_VALUE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultStepValue,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"savable",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SAVE_MODE_PARAMETERS,			/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &SaveModeParameters,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"verify",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VERIFY_LENGTH,			/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DefaultVerifyLength,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of I/O Defaults KeyTable */
};

/*
 * Define the Memory (Read/Write Buffer) Keyword Table:
 */
struct key_entry memory_keytable[] =
{
    {	"id",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BUFFER_ID,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_SPECIAL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0xff,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BufferId,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"mode",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BUFFER_MODE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_SPECIAL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	7,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BufferMode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"offset",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BUFFER_OFFSET,			/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_SPECIAL),		/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0xffffff,				/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BufferOffset,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
    },
    {	"segment",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_SEGMENT_SIZE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_RANGE | PF_ARGS_OPT | PF_SPECIAL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0xffffff,				/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &SegmentSize,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    /*
     * The following keywords are common I/O parameters, but due to
     * limitations of the parser, they must exist in this table to
     * avoid positional keywords.  If we don't do this, then I/O
     * parameters must be last on the command line (undesirable).
     */
    {	"bs",					/* k_name */
	"size",					/* k_alias */
	K_BLOCK_SIZE,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BlockSize,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"compare",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_COMPARE_FLAG,				/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &CompareFlag,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"errors",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_ERROR_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &ErrorLimit,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"limit",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DATA_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DataLimit,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"passes",				/* k_name */
	"iterations",				/* k_alias */
	K_PASS_LIMIT,				/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_SPECIAL,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &PassLimit,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"pattern",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DATA_PATTERN,				/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DataPattern,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"recovery",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_RECOVERY_FLAG,			/* k_token */
	TYPE_BOOLEAN,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &TestRecoveryFlag,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of Memory KeyTable */
};

/*
 * Define the XZA Keyword Table:
 */
struct key_entry xza_keytable[] =
{
    {	"show",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_SHOW,				/* k_token */
	TYPE_NONE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_show",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	xza_show_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"set",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_SET,				/* k_token */
	TYPE_NONE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_set",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	xza_set_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
#if defined(NOT_IMPLEMENTED)
    {	"read",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_READ,				/* k_token */
	TYPE_NONE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_read",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	xza_read_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"write",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_WRITE,				/* k_token */
	TYPE_NONE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_write",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	xza_write_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
#endif /* defined(NOT_IMPLEMENTED) */
    { 0 }				/* End of XZA KeyTable */
};

struct key_entry xza_show_keytable[] =
{
    {	"errors",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_ERRORS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_errors",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	xza_errors_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"counters",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_COUNTERS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	XZAReadCnt,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_read_count",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"params",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_PARAMS,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	XZAShowParams,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_show_params",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"ncr_regs",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_NCR,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	XZAShowNCRRegs,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_show_ncr_regs",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of XZA Show KeyTable */
};


struct key_entry xza_errors_keytable[] =
{
    {	"soft",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_SERR,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	XZASoftErrs,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_soft_errors",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"hard",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_HERR,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	XZAHardErrs,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) 0,				/* k_argp */
	"xza_hard_errors",			/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }				/* End of XZA Errors KeyTable */
};

struct key_entry xza_set_keytable[] =
{
    {	"scsi_id",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_SCSIID,				/* k_token */
	TYPE_VALUE,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	XZASetId,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) &UserSCSIID,			/* k_argp */
	"xza_set",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }			/* End of XZA set keytable */
};

#if defined(NOT_IMPLEMENTED)

struct key_entry xza_read_keytable[] =
{
    {	"image",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_IMAGE,				/* k_token */
	TYPE_STRING,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	XZAReadImage,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) &InputFileName,		/* k_argp */
	"xza_image",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }			/* End of XZA read keytable */
};

struct key_entry xza_write_keytable[] =
{
    {	"image",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_XZA_IMAGE,				/* k_token */
	TYPE_STRING,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	XZAWriteImage,				/* k_func */
	0,					/* k_cmd */
	ALL_DEVICE_TYPES,			/* k_spec */
	(caddr_t) &InputFileName,		/* k_argp */
	"xza_image",				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }			/* End of XZA write keytable */
};

#endif /* defined(NOT_IMPLEMENTED) */

/*
 * Define the Tape Keyword Table:
 */
struct key_entry set_tape_keytable[] =
{
    {	"blocking",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BLOCKING_MODE,			/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetTapeParameters,			/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	tape_blocking_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    {	"buffered",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_BUFFERED_MODE,			/* k_token */
	TYPE_VALUE,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	7,					/* k_max */
	SetTapeParameters,			/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BufferedMode,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"density",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_DENSITY,				/* k_token */
	TYPE_ACTION,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	SetTapeParameters,			/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) 0,				/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	tape_density_keytable,			/* k_keyp */
	1					/* k_keyc */
    },
    { 0 }			/* End of Tape keytable */
};

/*
 * Define the Tape Blocking Modes Keyword Table:
 */
struct key_entry tape_blocking_keytable[] =
{
    {	"fixed",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_FIXED_LENGTH,				/* k_token */
	TYPE_VALUE,				/* k_type */
	(PF_ARGS_OPT | PF_RANGE | PF_SPECIAL),	/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0xffffff,				/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BlockingSize,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"variable",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_VARIABLE_LENGTH,			/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &BlockingSize,		/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }			/* End of Tape Blocking keytable */
};

/*
 * Define the Tape Density Keyword Table:
 */
struct key_entry tape_density_keytable[] =
{
    {	"code",					/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_HEX,				/* k_type */
	PF_RANGE,				/* k_flags */
	0,					/* k_cflags */
	(caddr_t) 0,				/* k_default */
	0,					/* k_min */
	0xff,					/* k_max */
	(int (*)()) 0,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"default",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DEFAULT_DENSITY,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"800-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_800_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"1600-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_1600_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"6250-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_6250_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"8000-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_8000_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"3200-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_3200_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"6400-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_6400_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"38000-BPI",				/* k_name */
	"37871-BPI",				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_38000_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"6666-BPI",				/* k_name */
	"6667-BPI",				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_6666_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"12690-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_12690_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"16000-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_320,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"42500-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_42500_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"61000-BPI",				/* k_name */
	"4mm",					/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_61000_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"54000-BPI",				/* k_name */
	"8mm",					/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_54000_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"QIC-24",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_24,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"QIC-120",				/* k_name */
	"10000-BPI",				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_120,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"QIC-150",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_150,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"QIC-120-ECC",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_120_ECC,	/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"QIC-150-ECC",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_150_ECC,	/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"QIC-320",				/* k_name */
	"16000-BPI",				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_320,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"QIC-1350",				/* k_name */
	"51667-BPI",				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_1350,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"45434-BPI",				/* k_name */
	"8mm-8500-mode",			/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_45434_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"62500-BPI",				/* k_name */
	(caddr_t) 0,				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_62500_BPI,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"36000-BPI",				/* k_name */
	"QIC-1G",				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_1G,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    {	"40640-BPI",				/* k_name */
	"QIC-2G",				/* k_alias */
	K_TAPE_DENSITY,				/* k_token */
	TYPE_SET_FLAG,				/* k_type */
	0,					/* k_flags */
	0,					/* k_cflags */
	(caddr_t) T_DENSITY_QIC_2G,		/* k_default */
	0,					/* k_min */
	0,					/* k_max */
	MapTapeDensity,				/* k_func */
	0,					/* k_cmd */
	0,					/* k_spec */
	(caddr_t) &DensityCode,			/* k_argp */
	(caddr_t) 0,				/* k_msgp */
	(caddr_t) 0,				/* k_strp */
	0,					/* k_keyp */
	0					/* k_keyc */
    },
    { 0 }			/* End of Tape Density keytable */
};
