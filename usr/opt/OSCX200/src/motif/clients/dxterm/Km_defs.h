/*
 *  Title:	KM_DEFS.H - for Toggle_keyboard.c
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  
 *  Module Abstract:
 *
 *	Definitions required by TOGGLE_KEYBOARD.C
 *
 *  Modification History:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Aston Chan		07-Apr-1993	V1.2/BL2
 *	- Take out definition of NULL.  Complained by Alpha V1.2 build.
 *
 */

#include <stdio.h>
#define		MAXGROUPSUPPORT		10

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
#define		PROC_PRI		5
#else
#define		PROC_PRI		-1
#endif

#define 	FileNameLength 		256
#define 	MaxLineLength 		160
#define		UnknownKeyboard		-1
#define		ALLOC_INCREMENT		1200

#define		CarriageReturn		0xa

/* application class and name use for resource db*/
#if defined(VMS_DECTERM)
#define         KM_CLASS                "DECW$KM"
#define  	KM_RESOURCE_FILE  	"DECW$USER_DEFAULTS:DECW$KM.DAT"
#else
#define         KM_CLASS                "DXkm"
#define         KM_RESOURCE_FILE        ".DXkm"
#endif

/* Atom names */
#define DEC_KM_WINDOW_ID                "DEC_KM_WINDOW_ID"
#define DEC_KM_GROUP			"DEC_KM_GROUP"
#define DEC_KM_GROUP_DISPLAY		"DEC_KM_GROUP_DISPLAY"
#define DEC_KM_GROUP_V3			"DEC_KM_GROUP_V3"
#define DEC_KM_PRIMARY_GROUP_FILE       "DEC_KM_PRIMARY_GROUP_FILE"
#define DEC_KM_SECONDARY_GROUP_FILE     "DEC_KM_SECONDARY_GROUP_FILE"
#define DEC_KM_PAUSE			"DEC_KM_PAUSE"
#define V3_SME_EXT			"DEC_V3_EXT"
#define DEC_SM_PAUSE_WINDOW             "_DEC_SM_PAUSE_WINDOW"
#define DEC_KEYMAP_MODE			"_DEC_KEYMAP_MODE"

/* environment variable which allow to skip the new V3 extension use */
/* use the logical name to suppress the use of the extension */
#define KM_USE_OLD_KB			"DEC_KM_USE_OLD_KB"

/* default resources */

#define DEFAULT_HEB_SWITCH 		"DEFAULT"
#define INIT_PKBD  			"north american lk201la"
#define INIT_SKBD   			"hebrew_lk201lt"
#define DEFAULT_GROUP			0
#define DEFAULT_MAX_GROUP 		2
#define DEFAULT_SCAN_INTERVAL		60
#define DEFAULT_STICKY_STATE		FALSE

#define DEFAULT_SWITCH_KEYCODE		0xB1
#define DEFAULT_SWITCH_MODMASK		ControlMask

#define DEFAULT_KM_MODE 		1
#define KM_MODE_V1			0
#define KM_MODE_V3		        1
#define KM_MODE_V3_SOFT_SW		2
