#ifndef _dwc_ui_profile_h_
#define _dwc_ui_profile_h_ 1
/* $Header$ */
/* #module dwc_ui_profile.h */
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
**	Ken Cowan, March 1989
**
**  ABSTRACT:
**
**	Public entry points from dwc_ui_profile.c
**
**--
*/


#include    "dwc_compat.h"

void PROFILEOldDBProfileHack PROTOTYPE ((CalendarDisplay cd));

void PROFILEDO_SAVE_CURRENT_SETTINGS PROTOTYPE ((Widget w, caddr_t tag));

void PROFILEDO_RESTORE_SETTINGS PROTOTYPE ((Widget w, caddr_t tag));

void PROFILEDO_USE_DEFAULT_SETTINGS PROTOTYPE ((Widget w, caddr_t tag));

void PROFILESetDefaults PROTOTYPE ((ProfileStructure *profile));

void PROFILEByteSwap PROTOTYPE ((ProfileStructure *profile));

#endif	/* _dwc_ui_profile_h_ */
