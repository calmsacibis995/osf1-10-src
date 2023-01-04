#ifndef _dateformat_h_
#define _dateformat_h_ 1
/* $Header$								    */
/* #module DWC$UI_DATEFORMAT.H "V3-008"					    */
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
**	Marios Cleovoulou, November-1987
**
**  ABSTRACT:
**
**	This include file contains the structures and definitions needed for the
**	date formatting routines.
**
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3-008 PGLF	02-Feb-1990 Convert DwtLatin1String to XmStringCreate. Include 
**			    Xm.h for XmString stuff. Remove LIB$:, port to Motif
**	V2-007	Per Hamnqvist					18-Apr-1989
**		Get two macros into this module from associated .c file
**		to avoid confusing C_PROTOS conversion program.
**	V2-006	Per Hamnqvist					17-Apr-1989
**		Move comment to outside of function prototype.
**	V1-005	Ken Cowan					 4-Apr-1989
**		DATE_TIME_TEXT and DATE_TIME_CS now take the
**		name of a format string, not the string itself.
**	V1-004  Denis G. Lacroix				18-Jan-1989
**		Add #pragma stuff to keep picky compiler happy
**	V1-003	Marios Cleovoulou				11-Apr-1988
**		Add compound string support
**	V1-002	Per Hamnqvist					25-Jan-1988
**		Prepare for port to Ultrix, first pass.
**	V1-001  Marios Cleovoulou				27-Nov-1987
**		Initial version.
**--
**/

#ifndef _Xm_h
#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <DXm/DECspecific.h>
#ifdef vaxc
#pragma standard
#endif
#endif

#include "dwc_compat.h"
#include "dwc_ui_datestructures.h"
#include "dwc_ui_uil_values_const.h"

/*
extern char *number_texts    [100] ;
extern char *number_texts_lz [100] ;
extern XmString *number_week;
extern XmString *lz_number_week;
extern XmString *number_day;
extern XmString *lz_number_day;
*/

/* name of format in UIL, like dwc_k_slot_date_format (above) */

char *DATEFORMATTimeToText PROTOTYPE ((int format_index, dtb *date_time));

XmString DATEFORMATTimeToCS PROTOTYPE ((int format_index, dtb *date_time));

/*
** Special for week and day numbers.  This gets us the month widgets I18N.
*/
XmString DATEFORMATWeekToCS PROTOTYPE ((int week, Boolean lz));

XmString DATEFORMATDayToCS PROTOTYPE ((int day, Boolean	lz));

XmString DATEFORMATWeekToSharedCS PROTOTYPE ((int week, Boolean lz));

XmString DATEFORMATDayToSharedCS PROTOTYPE ((int day, Boolean lz));

#endif /* _dateformat_h_ */
