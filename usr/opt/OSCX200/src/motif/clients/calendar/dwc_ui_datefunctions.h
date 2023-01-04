#ifndef _datefunctions_h_
#define _datefunctions_h_ 1
/* $Header$								    */
/* #module DWC$UI_DATEFUNCTIONS.H "V3-001"				    */
/*
**  Copyright (c) Digital Equipment Corporation, 1991
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
**	Dick Schoeller,  3-APR-1991
**
**  ABSTRACT:
**
**	Function prototypes for dwc_ui_datefunctions.c.
**
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**--
**/

int DATEFUNCDaysInMonth PROTOTYPE ((
	int	month,			/* 1 -> 12 */
	int	year));			/* ccyy    */

int DATEFUNCDaysSinceBeginOfYear PROTOTYPE ((
	int	day,
	int	month,			/* 1 -> 12 */
	int	year));			/* ccyy    */

int DATEFUNCDaysSinceBeginOfTime PROTOTYPE ((
	int	day,			/* 1 -> 31 */
	int	month,			/* 1 -> 12 */
	int	year));			/* ccyy    */

void DATEFUNCDateForDayNumber PROTOTYPE ((
	int	dsbot,
	int	*day,			/* 1 -> 31 */
	int	*month,			/* 1 -> 12 */
	int	*year));		/* ccyy    */

int DATEFUNCDayOfWeek PROTOTYPE ((
	int	day,			/* 1 -> 31 */
	int	month,			/* 1 -> 12 */
	int	year));			/* ccyy    */

void DATEFUNCWeekNumberForDate PROTOTYPE ((
	int			day,
	int			month,
	int			year,
	int			weekstart,
	int			first_day,
	int			first_month,
	unsigned char		*weekno,
	unsigned short int	*fiscal_year));

void DATEFUNCStartDateForWeekNo PROTOTYPE ((
	int	weeknum,
	int	fiscal_year,
	int	weekstart,
	int	first_day,
	int	first_month,
	int	*day,
	int	*month,
	int	*year));

int number_of_weeks_in_month PROTOTYPE ((
	int	month,
	int	year,
	int	first_day_of_month,
	int	weekstart));

#endif /* _datefunctions_h_ */
