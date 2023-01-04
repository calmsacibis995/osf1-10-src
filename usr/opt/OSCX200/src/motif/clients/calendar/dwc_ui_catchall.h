/* $Header$ */
/* #module DWC$UI_CATCHALL_PUBLIC "V2-004" */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Denis G. Lacroix, February 1989
**
**  ABSTRACT:
**
**	Public include file for Calendar's User Interface error handling 
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	V2-001  Denis G. Lacroix				28-Feb-1989
**		Initial version.
**	V2-002  Denis G. Lacroix				28-Feb-89
**		Added DWC$UI_Establish and modified DWC$UI_Catchall
**	V2-003	Denis G. Lacroix				28-Feb-89
**		Changed the DWC$Context typedef to caddr_t
**	V2-004	Per Hamnqvist					28-Feb-1989
**		Change caddr_t for char *
**--
**/


#include    "dwc_compat.h"

/*	  
**  Exit codes
*/	  
#ifdef VMS
#define	DwcCleanExitCode    1
#define	DwcErrorExitCode    2
#else
#define	DwcCleanExitCode    0
#define	DwcErrorExitCode    2
#endif


/*
**  Type Definitions
*/

typedef enum
    {
    DWC$DRM,
    DWC$DRM_ALARM,
    DWC$DRM_CALENDAR,
    DWC$DRM_ERRORBOX,
    DWC$DRM_FETCHICON,
    DWC$DRM_FETCHLITERAL,
    DWC$DRM_FETCHMANAGE,
    DWC$DRM_HELP,
    DWC$DRM_HIERARCHY,
    DWC$DRM_ICONBOX,
    DWC$DRM_NODAYNOTEED,
    DWC$DRM_NOINCLUDE,
    DWC$DRM_NOPRINTDIALOG,
    DWC$DRM_NOPRINTWIDGET,
    DWC$DRM_NOREPEAT,
    DWC$DRM_NOSLOTED,
    DWC$DRM_PBCLOSE,
    DWC$PRINT,
    DWC$PRINT_CDA,
    DWC$UI,
    DWC$UI_APPENDSTRING,
    DWC$UI_FAILNEXTSEG,
    DWC$UI_NOFONT,
    DWC$UI_NOFONTINIT,
    DWC$UI_NOSTRINGINIT,
    DWC$UI_FAILCONVERT
    } DWC$FailureType;
    
typedef char *DWC$Context;
    
/*									    
**  Function Prototype Definitions
*/

void
DWC$UI_Catchall PROTOTYPE ((
	DWC$FailureType	failure_type,
	int		error_code_1,
	int		error_code_2));

void
DWC$UI_Establish PROTOTYPE ((
	DWC$Context	context));

