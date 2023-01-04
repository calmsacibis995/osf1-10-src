/* dwc_ui_dateformat.c */
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
**      This module contains the definitions and code for building date/time
**	strings in different languages.
**
**--
*/

#include "dwc_compat.h"

#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <X11/DECwI18n.h>
#include <DXm/DECspecific.h>
#ifdef vaxc
#pragma standard
#endif

#include "dwc_ui_misc.h"		    /* for MISCFetchDRMValue */
#include "dwc_ui_dateformat.h"
#include "dwc_ui_catchall.h"	



/* This module contains all the definitions for date and time formats in    */
/* the different languages.						    */
/*									    */
/* This is done by use of a pseudo FAO (or SPRINTF) string.  The introducer */
/* for formatting types is %.  The following formatting types are	    */
/* supported:								    */
/*									    */
/*	%CN -- numeric century, e.g. 19					    */
/*	%CZ --    "       "     with leading zero, e.g. 09		    */
/*									    */
/*	%YN -- numeric year,    e.g. 87					    */
/*	%YZ --    "	  "     with leading zero, e.g. 01		    */
/*									    */
/*	%MN -- numeric month,   e.g. 9					    */
/*	%MZ --    "      "      with leading zero, e.g. 04		    */
/*	%MS -- textual month,   short form, e.g. Jan			    */
/*	%ML --    "      "      long form, e.g. January			    */
/*									    */
/*	%DN -- numeric day,     e.g. 29					    */
/*	%DZ --    "     "       with leading zero, e.g. 03		    */
/*									    */
/*	%HN -- numeric hour,    e.g. 14					    */
/*	%HZ --    "     "       with leading zero, e.g. 01		    */
/*	%TN -- numeric hour, twelve hour clock, e.g. 1 for 13h		    */
/*	%TZ --    "     "      "     "     "   with leading zero, e.g. 01   */
/*									    */
/*	%NN -- numeric minutes, e.g. 59					    */
/*	%NZ --    "       "     with leading zero, e.g. 05		    */
/*									    */
/*	%SN -- numeric seconds, e.g. 57					    */
/*	%SZ --    "       "     with leading zero, e.g. 06		    */
/*									    */
/*	%PN -- numeric parts of second (hundredths), e.g. 99		    */
/*	%PZ --    "      "   "    "         "       with leading zero	    */
/*									    */
/*	%WA -- weekday,		abbreviated form, E.g. M		    */
/*	%WS -- weekday,		short form, E.g. Mon			    */
/*	%WL -- weekday,         long form, E.g. Monday			    */
/*									    */
/*	%AP -- AM/PM							    */
/*									    */
/*	%KS -- week text,	short form, E.g. Wk			    */
/*	%KL -- week text,       long form, E.g. Week			    */
/*									    */
/*	%XC -- suffix for century, e.g. "st" for 21st			    */
/*	%XY --    "    "  year						    */
/*	%XM --    "    "  month 					    */
/*	%XD --    "    "  day						    */
/*	%XH --    "    "  hour						    */
/*	%XT --    "    "  hour, twelve hour clock			    */
/*	%XN --    "    "  minute					    */
/*	%XS --    "    "  second					    */
/*	%XP --    "    "  part of second				    */
/*									    */
/*	%%% -- %							    */
/*									    */
/*	any other text is literal					    */
/*									    */
/*	So....e.g.							    */
/*									    */
/*	"%DN-%MS-%CN-%YZ %HN:%NZ"	    1-Jan-2001 3:04		    */
/*	"%WL the %DN%XD of %ML, %CN%YZ"	    Monday the 10th of March, 1987  */

#define	result_size_max	255

static int initialized;

static XmString long_week_text = NULL;
static XmString short_week_text = NULL;
static XmString *abbreviated_day_names = NULL;
static XmString *short_day_names = NULL;
static XmString *long_day_names = NULL;
static XmString *short_month_names = NULL;
static XmString *long_month_names = NULL;
static XmString *am_pm_texts = NULL;
static XmString *century_suffix_texts = NULL;
static XmString *year_suffix_texts = NULL;
static XmString *month_suffix_texts = NULL;
static XmString *day_suffix_texts = NULL;
static XmString *hour_suffix_texts = NULL;
static XmString *hour_12_suffix_texts = NULL;
static XmString *minute_suffix_texts = NULL;
static XmString *second_suffix_texts = NULL;
static XmString *sec_part_suffix_texts = NULL;

/*
** CS text tables for fetching numbers.  These may not be Latinized-Arabic!
*/
static XmString percent_cs;

static XmString *number_year;
static XmString *number_century;
static XmString *number_month;
XmString *number_day;
static XmString *number_hour;
static XmString *number_min;
static XmString *number_sec;
static XmString *number_hund;
XmString *number_week;
static XmString *lz_number_year;
static XmString *lz_number_century;
static XmString *lz_number_month;
XmString *lz_number_day;
static XmString *lz_number_hour;
static XmString *lz_number_min;
static XmString *lz_number_sec;
static XmString *lz_number_hund;
XmString *lz_number_week;


static void init_format_constants PROTOTYPE ((void));

static int find_format PROTOTYPE ((char *text, int *new_pos, int *index));

static XmString YIELD_CN PROTOTYPE ((dtb *date_time));

static XmString YIELD_CZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_YN PROTOTYPE ((dtb *date_time));

static XmString YIELD_YZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_MN PROTOTYPE ((dtb *date_time));

static XmString YIELD_MZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_MS PROTOTYPE ((dtb *date_time));

static XmString YIELD_ML PROTOTYPE ((dtb *date_time));

static XmString YIELD_DN PROTOTYPE ((dtb *date_time));

static XmString YIELD_DZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_HN PROTOTYPE ((dtb *date_time));

static XmString YIELD_HZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_TN PROTOTYPE ((dtb *date_time));

static XmString YIELD_TZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_NN PROTOTYPE ((dtb *date_time));

static XmString YIELD_NZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_SN PROTOTYPE ((dtb *date_time));

static XmString YIELD_SZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_PN PROTOTYPE ((dtb *date_time));

static XmString YIELD_PZ PROTOTYPE ((dtb *date_time));

static XmString YIELD_WA PROTOTYPE ((dtb *date_time));

static XmString YIELD_WS PROTOTYPE ((dtb *date_time));

static XmString YIELD_WL PROTOTYPE ((dtb *date_time));

static XmString YIELD_KS PROTOTYPE ((dtb *date_time));

static XmString YIELD_KL PROTOTYPE ((dtb *date_time));

static XmString YIELD_AP PROTOTYPE ((dtb *date_time));

static XmString YIELD_XC PROTOTYPE ((dtb *date_time));

static XmString YIELD_XY PROTOTYPE ((dtb *date_time));

static XmString YIELD_XM PROTOTYPE ((dtb *date_time));

static XmString YIELD_XD PROTOTYPE ((dtb *date_time));

static XmString YIELD_XH PROTOTYPE ((dtb *date_time));

static XmString YIELD_XT PROTOTYPE ((dtb *date_time));

static XmString YIELD_XN PROTOTYPE ((dtb *date_time));

static XmString YIELD_XS PROTOTYPE ((dtb *date_time));

static XmString YIELD_XP PROTOTYPE ((dtb *date_time));

static XmString YIELD_PCS PROTOTYPE ((dtb *date_time));


typedef struct
{
    char	*entry;
    XmString	(*routine)();
} dispatch_entry;

#define	dispatch_table_size 36

static dispatch_entry 
    dispatch_table [dispatch_table_size] =
       {
	{"CN", YIELD_CN},	/*	%CN -- numeric century, e.g. 19					    */
	{"CZ", YIELD_CZ},	/*	%CZ --    "       "     with leading zero, e.g. 09		    */
	{"YN", YIELD_YN},	/*	%YN -- numeric year,    e.g. 87					    */
	{"YZ", YIELD_YZ},	/*	%YZ --    "	  "     with leading zero, e.g. 01		    */
	{"MN", YIELD_MN},	/*	%MN -- numeric month,   e.g. 9					    */
	{"MZ", YIELD_MZ},	/*	%MZ --    "      "      with leading zero, e.g. 04		    */
	{"MS", YIELD_MS},	/*	%MS -- textual month,   short form, e.g. Jan			    */
	{"ML", YIELD_ML},	/*	%ML --    "      "      long form, e.g. January			    */
	{"DN", YIELD_DN},	/*	%DN -- numeric day,     e.g. 29					    */
	{"DZ", YIELD_DZ},	/*	%DZ --    "     "       with leading zero, e.g. 03		    */
	{"HN", YIELD_HN},	/*	%HN -- numeric hour,    e.g. 14					    */
	{"HZ", YIELD_HZ},	/*	%HZ --    "     "       with leading zero, e.g. 01		    */
	{"TN", YIELD_TN},	/*	%TN -- numeric hour, twelve hour clock, e.g. 1 for 13h		    */
	{"TZ", YIELD_TZ},	/*	%TZ --    "     "      "     "     "   with leading zero, e.g. 01   */
	{"NN", YIELD_NN},	/*	%NN -- numeric minutes, e.g. 59					    */
	{"NZ", YIELD_NZ},	/*	%NZ --    "       "     with leading zero, e.g. 05		    */
	{"SN", YIELD_SN},	/*	%SN -- numeric seconds, e.g. 57					    */
	{"SZ", YIELD_SZ},	/*	%SZ --    "       "     with leading zero, e.g. 06		    */
	{"PN", YIELD_PN},	/*	%PN -- numeric parts of second (hundredths), e.g. 99		    */
	{"PZ", YIELD_PZ},	/*	%PZ --    "      "   "    "         "       with leading zero	    */
	{"WA", YIELD_WA},	/*	%WA -- weekday,         abbreviated form, E.g. M		    */
	{"WS", YIELD_WS},	/*	%WS -- weekday,		short form, E.g. Mon			    */
	{"WL", YIELD_WL},	/*	%WL -- weekday,         long form, E.g. Monday			    */
	{"KS", YIELD_KS},	/*	%KS -- week text,	short form, E.g. Wk			    */
	{"KL", YIELD_KL},	/*	%KL -- week text,       long form, E.g. Week			    */
	{"AP", YIELD_AP},	/*	%AP -- AM/PM							    */
	{"XC", YIELD_XC},	/*	%XC -- suffix for century, e.g. "st" for 21st			    */
	{"XY", YIELD_XY},	/*	%XY --    "    "  year						    */
	{"XM", YIELD_XM},	/*	%XM --    "    "  month 					    */
	{"XD", YIELD_XD},	/*	%XD --    "    "  day						    */
	{"XH", YIELD_XH},	/*	%XH --    "    "  hour						    */
	{"XT", YIELD_XT},	/*	%XT --    "    "  hour, twelve hour clock			    */
	{"XN", YIELD_XN},	/*	%XN --    "    "  minute					    */
	{"XS", YIELD_XS},	/*	%XS --    "    "  second					    */
	{"XP", YIELD_XP},	/*	%XP --    "    "  part of second				    */
	{"%%", YIELD_PCS},	/*	%%% -- %							    */
       };

static int find_format
#ifdef _DWC_PROTO_
	(
	char	*format,
	int	*pos,
	int	*index)
#else	/* no prototypes */
	(format, pos, index)
	char	*format;
	int	*pos;
	int	*index;
#endif	/* prototype */
{
    int	    format_pos ;

    format_pos = 0 ;
    while (format [format_pos] != '\0')
    {
	if (format [format_pos] == '%')
	{
	    char    *s ;
	    int	    i = 0 ;

	    while (i < dispatch_table_size)
	    {
		if ((dispatch_table[i].entry[0] == format [format_pos + 1]) &&
		    (dispatch_table[i].entry[1] == format [format_pos + 2]))
		{
		    *index = i;
		    *pos = format_pos;
		    return ( TRUE );
		}
		i = i + 1 ;
	    }
	    return ( FALSE );
	} 
	else
	{
	    format_pos = format_pos + 1 ;
	}
    }

    return (FALSE);

}

XmString DATEFORMATWeekToCS
#ifdef _DWC_PROTO_
	(
	int	week,
	Boolean	lz
	)
#else	/* no prototypes */
	(week, lz)
	int	week;
	Boolean	lz;
#endif	/* prototype */
{
    if (lz)
    {
	if (lz_number_week == NULL)
	    lz_number_week = (XmString *)MISCFetchDRMValue
		(dwc_k_lz_number_week);

	return (XmStringCopy(lz_number_week[week]));
    }
    else
    {
	if (number_week == NULL)
	    number_week = (XmString *)MISCFetchDRMValue
		(dwc_k_number_week);

	return (XmStringCopy(number_week[week]));
    }
}

XmString DATEFORMATWeekToSharedCS
#ifdef _DWC_PROTO_
	(
	int	week,
	Boolean	lz
	)
#else	/* no prototypes */
	(week, lz)
	int	week;
	Boolean	lz;
#endif	/* prototype */
{
    if (lz)
    {
	if (lz_number_week == NULL)
	    lz_number_week = (XmString *)MISCFetchDRMValue
		(dwc_k_lz_number_week);

	return (lz_number_week[week]);
    }
    else
    {
	if (number_week == NULL)
	    number_week = (XmString *)MISCFetchDRMValue
		(dwc_k_number_week);

	return (number_week[week]);
    }
}

XmString DATEFORMATDayToCS
#ifdef _DWC_PROTO_
	(
	int	day,
	Boolean	lz
	)
#else	/* no prototypes */
	(day, lz)
	int	day;
	Boolean	lz;
#endif	/* prototype */
{
    if (lz)
    {
	if (lz_number_day == NULL)
	    lz_number_day = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_day);

	return (XmStringCopy(lz_number_day[day]));
    }
    else
    {
	if (number_day == NULL)
	    number_day = (XmString *)MISCFetchDRMValue (dwc_k_number_day);

	return (XmStringCopy(number_day[day]));
    }
}

XmString DATEFORMATDayToSharedCS
#ifdef _DWC_PROTO_
	(
	int	day,
	Boolean	lz
	)
#else	/* no prototypes */
	(day, lz)
	int	day;
	Boolean	lz;
#endif	/* prototype */
{
    if (lz)
    {
	if (lz_number_day == NULL)
	    lz_number_day = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_day);

	return (lz_number_day[day]);
    }
    else
    {
	if (number_day == NULL)
	    number_day = (XmString *)MISCFetchDRMValue (dwc_k_number_day);

	return (number_day[day]);
    }
}

char *DATEFORMATTimeToText
#ifdef _DWC_PROTO_
	(
	int	format_index,
	dtb	*date_time)
#else	/* no prototypes */
	(format_index, date_time)
	int	format_index;
	dtb	*date_time;
#endif	/* prototype */
{
    XmString   cs;
    char	    *text;
    long	byte_count,status;

    cs = DATEFORMATTimeToCS (format_index, date_time);
    text = (Opaque)DXmCvtCStoFC(cs, &byte_count, &status);
    if (status == DXmCvtStatusFail)
    {
	DWC$UI_Catchall(DWC$UI_FAILCONVERT,status,0);
    }
    XmStringFree (cs);
    return (text);
}

void debug_cs
#ifdef _DWC_PROTO_
(XmString foo)
#else
(foo)
XmString foo;
#endif
{
    char    *text;
    long	byte_count,status;
    text = (Opaque)DXmCvtCStoFC(foo, &byte_count, &status);
    if (status == DXmCvtStatusFail)
    {
	printf ("couldn't convert\n");
    }
    else
    {
	printf ("%s\n", text);
	XtFree (text);
    }
}

XmString DATEFORMATTimeToCS
#ifdef _DWC_PROTO_
	(
	int	format_index,
	dtb	*date_time)
#else	/* no prototypes */
	(format_index, date_time)
	int	format_index;
	dtb	*date_time;
#endif	/* prototype */
{
    XmStringContext ctx;
    XmString	    format_cs;
    int		    status;
    char	    *segment_text;
    XmStringCharSet charset;
    XmStringDirection dir_r_to_l;
    XmStringComponentType   next_component;
    Boolean	    separator;

    int		    last_pos, new_pos, index;
    XmString	    new_cs, old_cs, temp_cs;


    if (!initialized)
	init_format_constants();

    format_cs = MISCFetchDRMValue (format_index); /* do not free format_cs 
						    since we get
						    a ptr to static storage */

    if( !XmStringInitContext(&ctx, format_cs ))
    {
	DWC$UI_Catchall( DWC$UI_NOSTRINGINIT, status, 0);  
    }

    old_cs = NULL;   

    /*	 We want to know if we're at the end so peek before we try to get
    **	 the next segment so we can do the right thing
    */
    while ( (next_component = XmStringPeekNextComponent(ctx)) !=
	    XmSTRING_COMPONENT_END)
    {
        /*	  
	**  Did we get a valid segment?
	*/	  
        if (XmStringGetNextSegment(ctx, &segment_text, &charset,
				    &dir_r_to_l, &separator ))
	{
	    last_pos = 0;
	    new_pos = 0;
	    index = -1;
	    while( find_format( &segment_text[last_pos], &new_pos, &index ) )
	    {
		/*
		**  First, copy the text immediately prior to the format
		**  variable.
		*/
		segment_text[new_pos + last_pos] = '\0';  /* temporaryily terminate string */
		if ( segment_text[last_pos] != '\0')
		{
		    temp_cs = XmStringSegmentCreate( &segment_text[last_pos],
						charset, 
						dir_r_to_l,
						separator );
		    if (old_cs == NULL)
		    {
			new_cs = XmStringCopy(temp_cs);
		    }
		    else
		    {
			new_cs = XmStringConcat(old_cs, temp_cs);
		    }
		    XmStringFree( old_cs );
		    XmStringFree( temp_cs );
		    old_cs = new_cs;
		}

		/*
		**  Update our index to point immediately after the format
		**  variable.  3 is the size of a format variable.
		*/
		last_pos = last_pos + new_pos + 3;

		/*
		**  Fetch the substitution of the format variable.  Append
		**  it to the string we are building up.
		*/
		temp_cs = dispatch_table[index].routine( date_time );
		if (!XmStringEmpty( temp_cs ))
		{
		    if (old_cs == NULL)
		    {
			new_cs = XmStringCopy(temp_cs);
		    }
		    else
		    {
			new_cs = XmStringConcat(old_cs, temp_cs);
		    }
		    XmStringFree( old_cs );
		    old_cs = new_cs;
		    /* do not free the CS returned from the format routine
		    ** since it returns a pointer to our static storage.
		    */
		}
	    } /* end find_format */
	
	    /*
	    ** Now, copy the text to the right of the last format variable
	    ** we found.  If none were found, last_pos will be zero so we'll
	    ** copy the whole thing.
	    */
	    if ( segment_text[last_pos] != '\0')
	    {
		temp_cs = XmStringSegmentCreate( &segment_text[last_pos],
						charset, 
						dir_r_to_l,
						separator );
		if (old_cs == NULL)
		{
		    new_cs = XmStringCopy(temp_cs);
		}
		else
		{
		    new_cs = XmStringConcat(old_cs, temp_cs);
		}
		XmStringFree( old_cs );
		XmStringFree( temp_cs );
		old_cs = new_cs;
	    }

	    XtFree(segment_text);
	    XtFree(charset);

	} /* end GetNextSegment */

    } /* end of PeekNextComponent */

    /*
    **	XmStringPeekNextComponent returned an XmSTRING_COMPONENT_END, or
    **	XmString GetNextSegment returned a FALSE for no more, meaning
    **	we're at the end of the string
    */
    XmStringFreeContext(ctx);
    return (old_cs);
}

static XmString YIELD_CN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_century == NULL)
	number_century = (XmString *)MISCFetchDRMValue
	    (dwc_k_number_century);

    return(number_century[date_time->year/100]);
}

static XmString YIELD_CZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_century == NULL)
	lz_number_century = (XmString *)MISCFetchDRMValue
	    (dwc_k_lz_number_century);

    return (lz_number_century[date_time->year/100]);
}

static XmString YIELD_YN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_year == NULL)
	number_year = (XmString *)MISCFetchDRMValue (dwc_k_number_year);

    return (number_year[date_time->year % 100]);
}

static XmString YIELD_YZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_year == NULL)
	lz_number_year = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_year);

    return (lz_number_year[date_time->year % 100]);
}

static XmString YIELD_MN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_month == NULL)
	number_month = (XmString *)MISCFetchDRMValue (dwc_k_number_month);
    return (number_month[date_time->month-1]);
}

static XmString YIELD_MZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_month == NULL)
	lz_number_month = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_month);
    return (lz_number_month[date_time->month-1]);
}

static XmString YIELD_MS
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{

    if (short_month_names == NULL)
	short_month_names = (XmString *)MISCFetchDRMValue
	    (dwc_k_short_month_names);

    return (short_month_names [date_time->month - 1]) ;

}

static XmString YIELD_ML
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{

    if (long_month_names == NULL)
	long_month_names = (XmString *)MISCFetchDRMValue
	    (dwc_k_long_month_names);

    return (long_month_names [date_time->month - 1]) ;

}

static XmString YIELD_DN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_day == NULL)
	number_day = (XmString *)MISCFetchDRMValue (dwc_k_number_day);

    return (number_day[date_time->day]);
}

static XmString YIELD_DZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_day == NULL)
	lz_number_day = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_day);

    return (lz_number_day[date_time->day]);
}

static XmString YIELD_HN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_hour == NULL)
	number_hour = (XmString *)MISCFetchDRMValue (dwc_k_number_hour);

    return (number_hour[date_time->hour]);
}

static XmString YIELD_HZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_hour == NULL)
	lz_number_hour = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_hour);

    return (lz_number_hour[date_time->hour]);
}

static XmString YIELD_TN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_hour == NULL)
	number_hour = (XmString *)MISCFetchDRMValue (dwc_k_number_hour);

    if (date_time->hour > 12)
    {
	return (number_hour[date_time->hour - 12]);
    }
    else if (date_time->hour == 0)
    {
	return (number_hour[12]);
    }
    else
    {
	return (number_hour[date_time->hour]);
    }
}

static XmString YIELD_TZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_hour == NULL)
	lz_number_hour = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_hour);

    if (date_time->hour > 12)
    {
	return (lz_number_hour[date_time->hour - 12]);
    }
    else if (date_time->hour == 0)
    {
	return (lz_number_hour[12]);
    }
    else
    {
	return (lz_number_hour[date_time->hour]);
    }
}

static XmString YIELD_NN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_min == NULL)
	number_min = (XmString *)MISCFetchDRMValue (dwc_k_number_min);

    return (number_min[date_time->minute]);
}

static XmString YIELD_NZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_min == NULL)
	lz_number_min = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_min);

    return (lz_number_min[date_time->minute]);
}

static XmString YIELD_SN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_sec == NULL)
	number_sec = (XmString *)MISCFetchDRMValue (dwc_k_number_sec);

    return (number_sec[date_time->second]);
}

static XmString YIELD_SZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_sec == NULL)
	lz_number_sec = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_sec);

    return (lz_number_sec[date_time->second]);
}

static XmString YIELD_PN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (number_hund == NULL)
	number_hund = (XmString *)MISCFetchDRMValue (dwc_k_number_hund);

    return (number_hund[date_time->hundredth]);
}

static XmString YIELD_PZ
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (lz_number_hund == NULL)
	lz_number_hund = (XmString *)MISCFetchDRMValue (dwc_k_lz_number_hund);

    return (lz_number_hund[date_time->hundredth]);
}

static XmString YIELD_WA
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (abbreviated_day_names == NULL)
	abbreviated_day_names = (XmString *)MISCFetchDRMValue
	    (dwc_k_abr_day_names);

    return (abbreviated_day_names [date_time->weekday - 1]);
}

static XmString YIELD_WS
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (short_day_names == NULL)
	short_day_names = (XmString *)MISCFetchDRMValue (dwc_k_short_day_names);

    return (short_day_names [date_time->weekday - 1]);
}

static XmString YIELD_WL
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (long_day_names == NULL)
	long_day_names = (XmString *)MISCFetchDRMValue (dwc_k_long_day_names);

    return (long_day_names [date_time->weekday - 1]);
}

static XmString YIELD_KS
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    return (short_week_text);
}

static XmString YIELD_KL
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    return (long_week_text);
}

static XmString YIELD_AP
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (am_pm_texts == NULL)
	am_pm_texts = (XmString *)MISCFetchDRMValue (dwc_k_am_pm_texts);

    if (date_time->hour < 12)
    {
	return (am_pm_texts [0]);
    }
    else
    {
	return (am_pm_texts [1]);
    }
}

static XmString YIELD_XC
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (century_suffix_texts == NULL)
	century_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_century_suffix_texts);

    return (century_suffix_texts [date_time->year / 100]);
}

static XmString YIELD_XY
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (year_suffix_texts == NULL)
	year_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_year_suffix_texts);

    return (year_suffix_texts [date_time->year % 100]);
}

static XmString YIELD_XM
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (month_suffix_texts == NULL)
	month_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_month_suffix_texts);

    return (month_suffix_texts [date_time->month]);
}

static XmString YIELD_XD
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (day_suffix_texts == NULL)
	day_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_day_suffix_texts);

    return (day_suffix_texts [date_time->day]);
}

static XmString YIELD_XH
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (hour_suffix_texts == NULL)
	hour_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_hour_suffix_texts);

    return (hour_suffix_texts [date_time->hour]);
}

static XmString YIELD_XT
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (hour_12_suffix_texts == NULL)
	hour_12_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_hour_12_suffix_texts);

    if (date_time->hour < 13)
    {
        return (hour_12_suffix_texts [date_time->hour]);
    }
    else
    {
        return (hour_12_suffix_texts [date_time->hour - 12]);
    }
}

static XmString YIELD_XN
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (minute_suffix_texts == NULL)
	minute_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_minute_suffix_texts);

    return (minute_suffix_texts [date_time->minute]);
}

static XmString YIELD_XS
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (second_suffix_texts == NULL)
	second_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_second_suffix_texts);

    return (second_suffix_texts [date_time->second]);
}

static XmString YIELD_XP
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{

    if (sec_part_suffix_texts == NULL)
	sec_part_suffix_texts = (XmString *)MISCFetchDRMValue
	    (dwc_k_sec_part_suffix_texts);

    return (sec_part_suffix_texts [date_time->hundredth]);

}

static XmString YIELD_PCS
#ifdef _DWC_PROTO_
	(
	dtb	*date_time)
#else	/* no prototypes */
	(date_time)
	dtb	*date_time;
#endif	/* prototype */
{
    if (percent_cs == NULL) percent_cs = XmStringCreateSimple(" ");

    return (percent_cs);
}

static void init_format_constants
#ifdef _DWC_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{
    initialized = 1;

    long_week_text = MISCFetchDRMValue (dwc_k_long_week_text);
    short_week_text = MISCFetchDRMValue (dwc_k_short_week_text);
}
