/* DWC$UI_DATEFUNCTIONS "V1-007" */
#ifndef lint
static char rcsid[] = "$Header$";
#endif /* lint */
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
**	This module contains various date and time manipulation functions.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	V1-001  Marios Cleovoulou				27-Nov-1987
**		Initial version.
**	V1-002  Marios Cleovoulou				 1-Jan-1988
**		Incorporate Ken Cowan's DATEFUNCDateForDayNumber implementation.
**	V1-003  Marios Cleovoulou				 6-Jan-1988
**		Remove ADD_DAYS_TO_DATE routine as not needed anymore.
**	V1-004	Per Hamnqvist					25-Jan-1988
**		Prepare for port to Ultrix, first pass.
**	V1-005  Marios Cleovoulou				 4-Mar-1988
**		Fix week numbering bug.
**	V1-006  Marios Cleovoulou				30-Mar-1988
**		Implement STARTING_DATE_FOR_WEEK_NUMBER routine.
**	V1-007  Marios Cleovoulou				 1-Apr-1988
**		Get return types right in WEEK_NUMBER_FOR_DATE.
**--
**/

#include "dwc_compat.h"
#include "dwc_ui_datefunctions.h"


static const int days_before_month[12] =
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

static const int days_before_month_ly[12] =
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

static const int month_lengths[12] =
    {31, 28, 31, 30, 31, 30, 31, 31, 30,  31, 30, 31};

/* GREGORIAN_FUDGE is the number of "days" from 1-Jan-0000 to the   */
/* beginning of Gregorian, as if there were no discontinuities in   */
/* the calendar							    */
/*								    */

#define GREGORIAN_FUDGE  577736

int
DATEFUNCDaysInMonth
#ifdef _DWC_PROTO_
	(
	int	month,			/* 1 -> 12 */
	int	year)			/* ccyy    */
#else	/* no prototypes */
	(month, year)
	int	month;			/* 1 -> 12 */
	int	year;			/* ccyy    */
#endif	/* prototype */
{

    if (month != 2)	    return (month_lengths [month - 1]);
    if ((year % 4) != 0)    return (28);
    if ((year % 400) == 0)  return (29);
    if ((year % 100) == 0)  return (28);

    return (29);

} /* DATEFUNCDaysInMonth */

int
DATEFUNCDaysSinceBeginOfYear
#ifdef _DWC_PROTO_
	(
	int	day,
	int	month,			/* 1 -> 12 */
	int	year)			/* ccyy    */
#else	/* no prototypes */
	(day, month, year)
	int	day;
	int	month;			/* 1 -> 12 */
	int	year;			/* ccyy    */
#endif	/* prototype */
{
    int	    dsboy;

    
    dsboy = day - 1 + days_before_month [month - 1];
    if (month < 3)	    return (dsboy);
    if ((year % 4) != 0)    return (dsboy);
    if ((year % 400) == 0)  return (dsboy + 1);
    if ((year % 100) == 0)  return (dsboy);
    return (dsboy + 1);	

} /* DATEFUNCDaysSinceBeginOfYear */

int
DATEFUNCDaysSinceBeginOfTime
#ifdef _DWC_PROTO_
	(
	int	day,			/* 1 -> 31 */
	int	month,			/* 1 -> 12 */
	int	year)			/* ccyy    */
#else	/* no prototypes */
	(day, month, year)
	int	day;			/* 1 -> 31 */
	int	month;			/* 1 -> 12 */
	int	year;			/* ccyy    */
#endif	/* prototype */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      int = DATEFUNCDaysSinceBeginOfTime (int day, int month, int year);
**
**	Returns the number of days since the beginning of time up to, and
**	including, the day described by the passed values.  Only works for
**	dates starting with 15-Oct-1582.  The result of this function modulo 7
**	plus 1 will yield the day of the week of the passed date, with Friday
**	being day 1.
**
**  FORMAL PARAMETERS:
**
**	int	day			day of month (1 -> 28/29/30/31)
**	int	month			month of year (1 -> 12)
**	int	year			year, including century, e.g. 1987
**
**  IMPLICIT INPUTS:
**
**      int	days_before_month [12]	array of days in a year before the
**					indexing month.  Note February in this
**					table always has 28 days.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      int	return value		days since the begining of time
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

{
    int	    dsbot;	/* Days since the beginning of time.  We build up   */
			/* the return value in here			    */


    /* It's easier to start assuming the the Gregorian calendar goes all    */
    /* the way back to 1-Jan-0000.  We'll correct for this later.	    */
    /*									    */
    /* Start with day of month, and then add in the number of days previous */
    /* to the month in the year.  Add in number of days in years previous   */
    /* (ignore leap years for now).					    */

    dsbot = day   + days_before_month [month - 1];
    dsbot = dsbot + ((year-1) * 365);

    /* Correct for leap years.  One every four years except on the century, */
    /* but a special one every 400 years.				    */

    dsbot = dsbot + ((year - 1) / 4);
    dsbot = dsbot - ((year - 1) / 100);
    dsbot = dsbot + ((year - 1) / 400);

    /* Subtract the number of days from our start date to the beginning of  */
    /* the real Gregorian calendar.  If you don't believe GREGORIAN_FUDGE work it    */
    /* out for yourself!						    */

    dsbot = dsbot - GREGORIAN_FUDGE;
    
    /* Now catch dates before the beginning of the real Gregorian calendar, */
    /* Friday the 15th of October, 1582 and return -1 if we got one.	    */

    if (dsbot < 0)  return (-1);
    
    /* If it is before March then return result.  Otherwise check if this   */
    /* is a leap year and correct assumption that all Februarys have 28	    */
    /* days.								    */

    if (month < 3)	    return (dsbot);
    if ((year % 4) != 0)    return (dsbot);
    if ((year % 400) == 0)  return (dsbot + 1);
    if ((year % 100) == 0)  return (dsbot);
    return (dsbot + 1);	

    /* Do a ((dsbot % 7) + 1) with the returned result to get the day of    */
    /* week.  Day 1 is a Friday (15-Oct-1582 was a Friday), day 7 is a	    */
    /* Thursday.							    */

} /* DATEFUNCDaysSinceBeginOfTime */

void
DATEFUNCDateForDayNumber
#ifdef _DWC_PROTO_
	(
	int	dsbot,
	int	*day,			/* 1 -> 31 */
	int	*month,			/* 1 -> 12 */
	int	*year)			/* ccyy    */
#else	/* no prototypes */
	(dsbot, day, month, year)
	int	dsbot;
	int	*day;			/* 1 -> 31 */
	int	*month;			/* 1 -> 12 */
	int	*year;			/* ccyy    */
#endif	/* prototype */
{
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine takes the number of days since the beginning of
**	Gregorian time and returns the day, month and year representing
**	it.
**
**  FORMAL PARAMETERS:
**
**      dsbot.rl.v	Number of days since the beginning of time.
**	day.wl.r	Day number within the month
**	month.wl.r	Month number within the year
**	year.wl.r	Year in a form like 1987
**
**  IMPLICIT INPUTS:
**
**      days_before_month	Array of 12 integers containing the
**				number of days in the year that pass by
**				before the first of the month.   For
**				example,  days_before_month[11] contains
**				the sum of the number of days in Jan +
**				Feb + Mar, etc until Nov.
**	days_before_month_ly	Same as days_before_month, except for
**				leap years.  Hence, the entries for Jan
**				and Feb are identical, and all the
**				others are one larger.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**
**      None
**
**--
**/

/*								    */
/* These constants are reflect the periodic nature of leap years.   */
/* See the Time Rep standard for a complete description of the	    */
/* Gregorian calendar.   Simply, every 4th year is a leap year,	    */
/* except every 100th which is common except every 400th with is    */
/* also a leap year.						    */
/*								    */
/* These constants are the number of days in a 4 year, 100 year and */
/* 400 year period.						    */
/*								    */

    int d4   = (365  *  4) + 1;	    
    int	d100 = (d4   * 25) - 1;
    int	d400 = (d100 *  4) + 1;

/*								    */
/* Local variables						    */
/*								    */

    int	    d, m, y;		/* temporary day, month, and year variables */
    const int	*table;		/* pointer to either the */
				/* days_before_month table, or the */
				/* days_before_month_ly table, */
				/* depending whether the year is a leap */
				/* year or not. */
    

/*								    */
/* Normalize dsbot start from 1-Jan-0000, then subtract teh number  */
/* of leap days that have occured between 1-Jan-0000 and the day    */
/* represented by dsbot.					    */
/*								    */
/* We use 1-Jan-0000 as day 0 because it makes the calculations	    */
/* easier.							    */
/*								    */

    d = dsbot + GREGORIAN_FUDGE;
    d = d -  (d) / d400;
    d = d +  (d) / d100;
    d = d -  (d) / d4;

/*								    */
/* Now calculate the year.  We have already taken into account the  */
/* leap days, so our year has exactly 365 days in it.		    */
/* The current year is the ceiling of d/365.  We'll fake the	    */
/* ceiling function by testing if d is evenly disvisible by 365.    */
/*								    */

    y = d / 365;
    if (( d % 365 ) != 0)
	y = y + 1;

/*								    */
/* Now that we have the year, we have to calculate the day number   */
/* within the year.						    */
/*								    */
/* Using the old d, d mod 365 is the number of days that have past  */
/* in the current 365 day period.  I can't figure out how to turn   */
/* that into a the day number within the year (which might be 366   */
/* becuase of leap year).  In fact, I think it isn't possible	    */
/* because the calculation of d/d400, d/d100 and d/4 are slightly   */
/* off for the last day of a leap year.				    */
/*								    */
/* So, the alternative is to start from the top, and exactly	    */
/* calculate the number of days that have occured between	    */
/* 1-Jan-0000 and 1-Jan of the year we calculated.		    */
/*								    */

    d = dsbot + GREGORIAN_FUDGE;
    d = d - 365*(y-1);
    d = d - (y-1)/4;
    d = d + (y-1)/100;
    d = d - (y-1)/400 ;

/*								    */
/* d is now the day number within the year. Find out what month	    */
/* that falls into, and then which day of the month it is.  Use a   */
/* different table if this is a leap year.			    */
/*								    */

    table = days_before_month;
    if ((y % 4) == 0) {
	if ((y % 100) != 0) {
	    table = days_before_month_ly;
	} else {
	    if ((y % 400) == 0) {
		table = days_before_month_ly;
	    }
	}
    }
    m = 11;
    while ((d <= table [m]) && (m != 0)){
	m--;
    }

    d = d - table [m];

/*								    */
/* Return the calculated values and exit.			    */
/*								    */

    *day   = d;
    *month = m + 1;
    *year  = y;

    return;    
}

int
DATEFUNCDayOfWeek
#ifdef _DWC_PROTO_
	(
	int	day,			/* 1 -> 31 */
	int	month,			/* 1 -> 12 */
	int	year)			/* ccyy    */
#else	/* no prototypes */
	(day, month, year)
	int	day;			/* 1 -> 31 */
	int	month;			/* 1 -> 12 */
	int	year;			/* ccyy    */
#endif	/* prototype */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      int = DATEFUNCDayOfWeek (int day, int month, int year);
**
**	Returns the day of week for the day described by the passed values.
**	Only works for dates starting with 15-Oct-1582.  Day 1 is Sunday and
**	hence, 7 is Saturday.  (This is easier for [most] people to deal with
**	than day 1 being a Friday!).  0 signifies an invalid date.
**
**  FORMAL PARAMETERS:
**
**	int	day			day of month (1 -> 28/29/30/31)
**	int	month			month of year (1 -> 12)
**	int	year			year, including century, e.g. 1987
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      int	return value		day of week
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

{
    int	    dsbot;
    
    dsbot = DATEFUNCDaysSinceBeginOfTime (day, month, year);
    if (dsbot < 0)  return (0);

    return (((dsbot + 5) % 7) + 1);

} /* DATEFUNCDayOfWeek */

void
DATEFUNCWeekNumberForDate
#ifdef _DWC_PROTO_
	(
	int			day,
	int			month,
	int			year,
	int			weekstart,
	int			first_day,
	int			first_month,
	unsigned char		*weekno,
	unsigned short int	*fiscal_year)
#else	/* no prototypes */
	(day, month, year, weekstart, first_day, first_month, weekno, fiscal_year)
	int			day;
	int			month;
	int			year;
	int			weekstart;
	int			first_day;
	int			first_month;
	unsigned char		*weekno;
	unsigned short int	*fiscal_year;
#endif	/* prototype */
    {
    int	    first_year;
    int	    first;
    int	    first_weekday;
    int	    dsboy;
    
    /*
    **  Need checks for (a) week numbers in the year of the
    **  beginning of time and, (b) the joker that says the year
    **  starts on the 29th of February!  **********************
    */
    
    /*
    **  Get the number of days since the beginning of time to
    **  the first of the year for week number purposes.
    */
    
    if (((month * 31) + day) >= ((first_month * 31) + first_day)) {
	first_year = year;
    } else {
	first_year = year - 1;
    }

    first = DATEFUNCDaysSinceBeginOfTime (first_day, first_month, first_year);

    /*
    **	Now take into account what day of the week the user thinks the week
    **	starts.
    */
    
    first_weekday = ((first + 5) % 7) + 1;
    first = first + weekstart - first_weekday;
    if (first_weekday < weekstart) {
	first = first - 7;
    }

    /*
    **	Now get the number of days since then and easily (ha!) calculate the
    **	week number from the difference.
    */
    
    dsboy = DATEFUNCDaysSinceBeginOfTime (day, month, year) - first;

    *weekno = ((dsboy / 7) + 1);
    *fiscal_year = first_year;

}

void
DATEFUNCStartDateForWeekNo
#ifdef _DWC_PROTO_
	(
	int	weeknum,
	int	fiscal_year,
	int	weekstart,
	int	first_day,
	int	first_month,
	int	*day,
	int	*month,
	int	*year)
#else	/* no prototypes */
	(weeknum, fiscal_year, weekstart, first_day, first_month, day, month, year)
	int	weeknum;
	int	fiscal_year;
	int	weekstart;
	int	first_day;
	int	first_month;
	int	*day;
	int	*month;
	int	*year;
#endif	/* prototype */
    {
    int	    first;
    int	    first_weekday;
    int	    dsboy;
        
    /*
    **  Get the number of days since the beginning of time to
    **  the first of the year for week number purposes.
    */
    
    first = DATEFUNCDaysSinceBeginOfTime (first_day, first_month, fiscal_year);

    /*
    **	Now take into account what day of the week the user thinks the week
    **	starts.
    */
    
    first_weekday = ((first + 5) % 7) + 1;
    first = first + weekstart - first_weekday;
    if (first_weekday < weekstart) {
	first = first - 7;
    }

    dsboy = first + (7 * (weeknum - 1));

    DATEFUNCDateForDayNumber (dsboy, day, month, year);

}

int
number_of_weeks_in_month
#ifdef _DWC_PROTO_
	(
	int	month,
	int	year,
	int	first_day_of_month,
	int	weekstart)
#else	/* no prototypes */
	(month, year, first_day_of_month, weekstart)
	int	month;
	int	year;
	int	first_day_of_month;
	int	weekstart;
#endif	/* prototype */
    {
    int	    fdom;
    int	    blank_days;

    fdom = first_day_of_month;

    if (fdom == 0) {
	fdom = DATEFUNCDayOfWeek (1, month, year);
    }

    blank_days = fdom - weekstart;
    if (blank_days < 0) {
	blank_days = 7 + blank_days;
    }

    return (((DATEFUNCDaysInMonth(month, year) + blank_days - 1) / 7) + 1);

}
