#ifndef _dwc_ui_monthinfo_h_
#define _dwc_ui_monthinfo_h_
/* $Id$ */
/* #module dwc_ui_monthinfo.h */
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
**	This include file contains structures for the month information handling
**	module of the calendar.
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
**	V1-002  Marios Cleovoulou				 1-Mar-1988
**		Add support for callback for markup and style.
**--
**/

#ifdef vaxc
#pragma nostandard
#endif
#include <X11/Intrinsic.h>	    /* for XtCallbackProc */
#ifdef vaxc
#pragma standard
#endif

#include    "dwc_compat.h"

/*
**  Basic month information block.  There is only one of these kept for each
**  month in memory for each calendar.  The reference count indicates how many
**  display widgets have an interest in the instance of the structure.
*/

typedef struct {
    unsigned short int	year_number ;	    /* ccyy 			    */
    unsigned char 	month_number ;	    /* 1 -> 12			    */
    unsigned char	month_length ;	    /* Number of days in month	    */
    unsigned char	first_weekday ;	    /* Weekday of 1st of the month  */
    unsigned int	reference_count ;   /* Interest level		    */
    unsigned char	markup [31] ;	    /* Markup for each day	    */
    unsigned char	style [31] ;	    /* Style for each day	    */
    unsigned int	selected ;	    /* Selected mask		    */
} MiMonthInfoBlock ;
    
/*
**  "MiMonthInfoList" provides for having linked lists of "MiMonthInfoBlock"s
*/

typedef struct MiMonthInfoList {
    struct MiMonthInfoList    *next ;	/* Next MiMonthInfoList structure	    */
    struct MiMonthInfoList    *last ;	/* Last MiMonthInfoList structure	    */
    MiMonthInfoBlock		    *month ;	/* The month information block	    */
} MiMonthInfoList ;


/*
**  "MiListOfMonthInfoList" provides for having lists of lists of month information
**  blocks.  "first_mil" points to the first of the linked list, "final_mil"
**  points to the last of the linked list.  These lists are NULL terminated.
*/

typedef struct MiListOfMonthInfoList {
    struct MiListOfMonthInfoList    *next ;		     /* Next structure	    */
    struct MiListOfMonthInfoList    *last ;		     /* Last structure	    */
    MiMonthInfoList		*first_mil ;
    MiMonthInfoList		*final_mil ;
} MiListOfMonthInfoList ;


/*
**  "MiAllMonthInfo" holds the pointer to the list of lists of month information
**  blocks for each calendar and the pointer and count of the cache.  Month
**  information blocks that are removed from the list of lists are moved to the
**  cache on a first-in-last-out basis (however, if the cache "fills up" the
**  oldest entries are removed and their memory deallocated).  Note that the
**  cache is a circular list of "MiMonthInfoList" items pointing to month
**  information blocks.
*/

struct _CalendarDisplayRecord;

typedef void (*MonthInfoCallbackProc) PROTOTYPE ((
	struct _CalendarDisplayRecord *cd,
	Cardinal	month,
	Cardinal	year,
	Cardinal	days,
	unsigned char	markup[],
	unsigned char	style[]));

typedef struct
{
    MiListOfMonthInfoList	*listlist ;
    MiMonthInfoList		*cache ;
    unsigned int		cache_count ;
    unsigned int		cache_size ;
    MonthInfoCallbackProc	month_info_callback;
    void			*month_info_tag ;
} MiAllMonthInfo ;

/*
**  Function prototypes for exported routines from DWC_UI_MONTHINFO.C
*/

MiMonthInfoList *MIAddNextMonthToList PROTOTYPE ((
	MiMonthInfoList	*mil_supplied,
	MiAllMonthInfo	*all_month_info));

void MIInitialiseAllMonthInfo PROTOTYPE ((
        MiAllMonthInfo		*all_month_info,
	unsigned int		cache_size,
	MonthInfoCallbackProc	month_info_callback,
	void			*month_info_tag));

MiMonthInfoList *MIAddPreviousMonthToList PROTOTYPE ((
	MiMonthInfoList	*mil_supplied,
	MiAllMonthInfo	*all_month_info));

void MIRemoveMonthInfoListItem PROTOTYPE ((
	MiMonthInfoList	*mil_supplied,
	MiAllMonthInfo	*all_month_info));

MiMonthInfoList *MIGetMonthInfoListItem PROTOTYPE ((
	int		month,
	int		year,
	MiAllMonthInfo	*all_month_info));

void MIGetMonthMarkup PROTOTYPE ((
	MiMonthInfoBlock	*month_info,
	MiAllMonthInfo		*all_month_info));

void MIPurgeMonthCache PROTOTYPE ((
	MiAllMonthInfo	*all_month_info));
#endif /* _monthinfo_h_ */
