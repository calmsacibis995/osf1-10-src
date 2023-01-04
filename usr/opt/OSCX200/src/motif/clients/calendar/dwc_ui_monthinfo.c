/* dwc_ui_monthinfo.c */
#ifndef lint
static char rcsid[] = "$Id$";
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
**
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
**	This module contains the routines that handle information about months.
**
**--
*/

#include "dwc_compat.h"

#include <stdio.h>

#include "dwc_ui_calendar.h"
#include "dwc_ui_datestructures.h"
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_monthinfo.h"

/*
**  Routine prototypes for static routines in this module (global routine
**  prototypes are in MONTHINFO.H)
*/

MiMonthInfoList *allocate_month_list PROTOTYPE ((MiMonthInfoBlock *month));

static MiListOfMonthInfoList *allocate_month_list_list PROTOTYPE ((
	MiMonthInfoList	*month_info_list));

static MiMonthInfoBlock *create_month_info PROTOTYPE ((
	int		month,
	int		year,
	MiAllMonthInfo	*all_month_info));

static MiMonthInfoList *create_month_info_list PROTOTYPE ((
	int		month,
	int		year,
	MiAllMonthInfo	*all_month_info));

static MiMonthInfoList *get_month_info PROTOTYPE ((
	int		month,
	int		year,
	MiAllMonthInfo	*all_month_info));

static void join_month_info_list_lists PROTOTYPE ((
	MiListOfMonthInfoList	*first,
	MiListOfMonthInfoList	*second,
	MiAllMonthInfo		*all_month_info));


void MIInitialiseAllMonthInfo
#ifdef _DWC_PROTO_
	(
	MiAllMonthInfo  *all_month_info,
	unsigned int    cache_size,
	MonthInfoCallbackProc  month_info_callback,
	void		*month_info_tag)
#else	/* no prototypes */
	(all_month_info, cache_size, month_info_callback, month_info_tag)
	MiAllMonthInfo  *all_month_info;
	unsigned int    cache_size;
	MonthInfoCallbackProc  month_info_callback;
	void		*month_info_tag;
#endif	/* prototype */
{

    /*
    **  Set all list pointers to null and zero cache count.
    */

    all_month_info->listlist    = (MiListOfMonthInfoList *)NULL;
    all_month_info->cache       = (MiMonthInfoList *)NULL;
    all_month_info->cache_count = (unsigned int)0;
    all_month_info->cache_size  = (unsigned int)cache_size;
    all_month_info->month_info_callback = month_info_callback;
    all_month_info->month_info_tag      = month_info_tag;

}

MiMonthInfoList *allocate_month_list
#ifdef _DWC_PROTO_
	(
	MiMonthInfoBlock	*month)
#else	/* no prototypes */
	(month)
	MiMonthInfoBlock	*month;
#endif	/* prototype */
    {
    MiMonthInfoList   *month_info_list ;

    /*
    **	Allocate the month info list, null out next & last pointers and
    **	attach passed month info to it.
    */
    
    month_info_list = (MiMonthInfoList *) XtMalloc (sizeof (MiMonthInfoList));
    if (month_info_list == NULL)  return (NULL) ;

    month_info_list->next  = NULL ;
    month_info_list->last  = NULL ;
    month_info_list->month = month ;

    return (month_info_list) ;

}

MiMonthInfoList *MIAddNextMonthToList
#ifdef _DWC_PROTO_
	(
	MiMonthInfoList	*mil_supplied,
	MiAllMonthInfo	*all_month_info)
#else	/* no prototypes */
	(mil_supplied, all_month_info)
	MiMonthInfoList	*mil_supplied;
	MiAllMonthInfo	*all_month_info;
#endif	/* prototype */
    {
    MiListOfMonthInfoList   *mill_for_supplied_mil ;
    MiListOfMonthInfoList   *mill ;
    int			year ;
    int			month ;

    /*
    **  Give up is list is empty.  Should signal or something ???????????
    **  If the next month is already on the list then just bump reference
    **  count and return month.
    */
    
    if (mil_supplied == NULL)  return (NULL) ;
    if (mil_supplied->next != NULL) {
	mil_supplied->next->month->reference_count++ ;
	return (mil_supplied->next) ;
    }
    
    /*
    **  Find out which list list entry list is at the end of.
    */

    mill_for_supplied_mil = all_month_info->listlist ;
    while (mill_for_supplied_mil->final_mil != mil_supplied) {
	mill_for_supplied_mil = mill_for_supplied_mil->next ;
    }
    
    /*
    **  Get year and month numbers for next month
    */
    
    if (mil_supplied->month->month_number == 12) {
	year  = mil_supplied->month->year_number + 1 ;    
	month = 1 ;    
    } else {
	year  = mil_supplied->month->year_number ;    
	month = mil_supplied->month->month_number + 1 ;    
    }

    /*
    **  Try to find the month we want in the list of all months.  Look at
    **  the first entry in each list.
    */

    mill = all_month_info->listlist ;
    while (mill != NULL) {
	if ((mill->first_mil->month->year_number  == year)  &&
	    (mill->first_mil->month->month_number == month)) {

	    /*
	    **	Found at the beginning of this list.  Append this list to
	    **	list previous month is on, bump reference count for month
	    **  and return "new" month.
	    */

	    join_month_info_list_lists (mill_for_supplied_mil, mill,
				        all_month_info) ;
	    mil_supplied->next->month->reference_count++ ;
	    return (mil_supplied->next) ;

	}
	mill = mill->next ;
    }


    /*
    **  If we couldn't find it amongst the existing used months then go
    **  get it, either for new or from the cache.
    */

    mil_supplied->next = get_month_info (month, year, all_month_info) ;
    if (mil_supplied->next == NULL) {
	return (NULL) ;
    }

    /*
    **  Tie it into list, adjust end pointer and return it.
    */
    
    mil_supplied->next->last = mil_supplied ;
    mil_supplied->next->next = NULL ;

    mill_for_supplied_mil->final_mil = mil_supplied->next ;
    return (mil_supplied->next) ;

}

MiMonthInfoList *MIAddPreviousMonthToList
#ifdef _DWC_PROTO_
	(
	MiMonthInfoList	*mil_supplied,
	MiAllMonthInfo	*all_month_info)
#else	/* no prototypes */
	(mil_supplied, all_month_info)
	MiMonthInfoList	*mil_supplied;
	MiAllMonthInfo	*all_month_info;
#endif	/* prototype */
    {
    MiListOfMonthInfoList   *mill_for_supplied_mil ;
    MiListOfMonthInfoList   *mill ;
    int			year ;
    int			month ;


    /*
    **  Give up is list is empty.  Should signal or something ???????????
    **  If the previous month is already on the list then just bump reference
    **  count and return month.
    */
    
    if (mil_supplied == NULL)  return (NULL) ;
    if (mil_supplied->last != NULL) {
	mil_supplied->last->month->reference_count++ ;
	return (mil_supplied->last) ;
    }
    
    /*
    **  Find out which list list entry list is at the beginning of.
    */

    mill_for_supplied_mil = all_month_info->listlist ;
    while (mill_for_supplied_mil->first_mil != mil_supplied) {
	mill_for_supplied_mil = mill_for_supplied_mil->next ;
    }
    
    /*
    **  Get year and month numbers for previous month
    */
    
    if (mil_supplied->month->month_number == 1) {
	year  = mil_supplied->month->year_number - 1 ;    
	month = 12 ;    
    } else {
	year  = mil_supplied->month->year_number ;    
	month = mil_supplied->month->month_number - 1 ;    
    }

    /*
    **	Try to find the month we want by looking at the end of the list of
    **	all months.
    */

    mill = all_month_info->listlist ;
    while (mill != NULL) {
	if ((mill->final_mil->month->year_number  == year)  &&
	    (mill->final_mil->month->month_number == month)) {

	    /*
	    **	Found at the beginning of this list.  Append this list to
	    **	list previous month is on, bump reference count for month,
	    **  and return.
	    */

	    join_month_info_list_lists (mill, mill_for_supplied_mil,
				        all_month_info) ;
	    mil_supplied->last->month->reference_count++ ;
	    return (mil_supplied->last) ;

	}
	mill = mill->next ;
    }


    /*
    **  If we couldn't find it amongst the existing used months then go
    **  get it, either for new or from the cache.
    */

    mil_supplied->last = get_month_info (month, year, all_month_info) ;
    if (mil_supplied->last == NULL)  return (NULL) ;

    /*
    **  Tie it into list, adjust beginning pointer and return it.
    */
    
    mil_supplied->last->next = mil_supplied ;
    mil_supplied->last->last = NULL ;

    mill_for_supplied_mil->first_mil = mil_supplied->last ;
    return (mil_supplied->last) ;

}

void MIRemoveMonthInfoListItem
#ifdef _DWC_PROTO_
	(
	MiMonthInfoList	*mil_supplied,
	MiAllMonthInfo	*all_month_info)
#else	/* no prototypes */
	(mil_supplied, all_month_info)
	MiMonthInfoList	*mil_supplied;
	MiAllMonthInfo	*all_month_info;
#endif	/* prototype */
    {
    MiMonthInfoList	*first_mil ;
    MiListOfMonthInfoList   *mill_for_supplied_mil ;
    MiListOfMonthInfoList   *new_mill ;

    /*
    **  Give up if list is empty.  Should signal or something here ???????
    **  Decrement reference count.  If it doesn't go down to zero, then just
    **  return.
    */
    
    if (mil_supplied == NULL)  return  ;
    mil_supplied->month->reference_count-- ;
    if (mil_supplied->month->reference_count != 0) {
	return ;
    }

    /*
    **	Backup to beginning of list and find out which list list entry list
    **	is on.
    */

    first_mil = mil_supplied ;
    while (first_mil->last != NULL) {
	first_mil = first_mil->last ;
    }

    mill_for_supplied_mil = all_month_info->listlist ;
    while (mill_for_supplied_mil->first_mil != first_mil) {
	mill_for_supplied_mil = mill_for_supplied_mil->next ;
    }

    /*
    **  Decouple month from list.
    */
    
    if (mil_supplied->next != NULL) {
	mil_supplied->next->last = mil_supplied->last ;
    }
    if (mil_supplied->last != NULL) {
	mil_supplied->last->next = mil_supplied->next ;
    }


    if (first_mil == mil_supplied) {

	/*
	**  If it was at the top of the list then move list list entry down
	**  one.  If there isn't one to move it down to, get rid of the
	**  list list entry.
	*/
    
	if (mil_supplied->next != NULL) {
	    mill_for_supplied_mil->first_mil = mil_supplied->next ;
	} else {
	    if (mill_for_supplied_mil->last != NULL) {
		mill_for_supplied_mil->last->next = mill_for_supplied_mil->next ;
	    }
	    if (mill_for_supplied_mil->next != NULL) {
		mill_for_supplied_mil->next->last = mill_for_supplied_mil->last ;
	    }

	    if (all_month_info->listlist == mill_for_supplied_mil) {
		all_month_info->listlist = mill_for_supplied_mil->next ;
	    }
	    XtFree ((char *) mill_for_supplied_mil);
	}

    } else {

	/*
	**  At end or in middle of the list.
	*/

	if (mil_supplied->next != NULL) {

	    /*
	    **	Ah.  In the middle of the list.  Awkward case.  Need to
	    **	generate a new list list entry and split the list up.
	    */
	    
	    mil_supplied->last->next = NULL ;
	    mil_supplied->next->last = NULL ;
	    new_mill = allocate_month_list_list (mil_supplied->next) ;
	    new_mill->last = mill_for_supplied_mil ;
	    new_mill->next = mill_for_supplied_mil->next ;
	    mill_for_supplied_mil->next = new_mill ;
	    if (new_mill->next != NULL) {
		new_mill->next->last = new_mill ;
	    }

	    new_mill->final_mil = mill_for_supplied_mil->final_mil ;
	}

	/*
	**  In either case adjust original list list's end pointer.
	*/

	mill_for_supplied_mil->final_mil = mil_supplied->last ;

    }


    /*
    **	Now add the item to the front of the cache.  If cache is full,
    **	throw away the oldest item.  If cache doesn't exist then create it!
    */
    
    if (all_month_info->cache_count == 0) {
	all_month_info->cache = mil_supplied ;
	mil_supplied->next    = mil_supplied ;
	mil_supplied->last    = mil_supplied ;
	all_month_info->cache_count = 1 ;
    } else {
	mil_supplied->last = all_month_info->cache ;
	mil_supplied->next = all_month_info->cache->next ;
	all_month_info->cache->next->last = mil_supplied ;
	all_month_info->cache->next       = mil_supplied ;
	all_month_info->cache             = mil_supplied ;
	all_month_info->cache_count++ ;
    }

    if (all_month_info->cache_count > all_month_info->cache_size) {
	mil_supplied = all_month_info->cache->next ;
	mil_supplied->last->next = mil_supplied->next ;
	mil_supplied->next->last = mil_supplied->last ;
	all_month_info->cache_count-- ;
	XtFree ((char *)mil_supplied->month);
	XtFree ((char *)mil_supplied);
    }

}

MiMonthInfoList *MIGetMonthInfoListItem
#ifdef _DWC_PROTO_
	(
	int		month,
	int		year,
	MiAllMonthInfo	*all_month_info)
#else	/* no prototypes */
	(month, year, all_month_info)
	int		month;
	int		year;
	MiAllMonthInfo	*all_month_info;
#endif	/* prototype */
    {
    MiMonthInfoList	*month_info_list ;
    MiListOfMonthInfoList   *mill ;
    MiListOfMonthInfoList   *new_mill ;
    int			months ;
    
    /*
    **  Try to find the month we want in the list of all months
    */

    months = (year * 12) + month ;
    mill = all_month_info->listlist ;

    while (mill != NULL) {
	if ((((mill->first_mil->month->year_number * 12) +
	       mill->first_mil->month->month_number) <= months) &&
	    (((mill->final_mil->month->year_number * 12) +
	       mill->final_mil->month->month_number) >= months)) {

	    /*
	    **  Found somewhere in this list.  Skip down to it and bump
	    **  reference count for month and return it.
	    */

	    month_info_list = mill->first_mil ;
	    months = months - (mill->first_mil->month->year_number * 12) -
			       mill->first_mil->month->month_number ;
	    while (months > 0) {
		month_info_list = month_info_list->next ;
		months-- ;
	    }

	    month_info_list->month->reference_count++ ;
	    return (month_info_list) ;
	}
	mill = mill->next ;
    }

    /*
    **	If we couldn't find it amongst the existing used months then go get it,
    **	either for new or from the cache and allocate a new list list entry for
    **	it.
    */

    month_info_list = get_month_info (month, year, all_month_info) ;
    if (month_info_list == NULL)  return (NULL) ;

    new_mill = allocate_month_list_list (month_info_list) ;
    if (all_month_info->listlist != NULL) {
	new_mill->next = all_month_info->listlist ;
	all_month_info->listlist->last = new_mill ;
    }
    all_month_info->listlist = new_mill ;

    return (month_info_list) ;

}

/*
**  This really shouldn't be here.  Move to the module that will read month
**  information from the file......
*/
void MIGetMonthMarkup
#ifdef _DWC_PROTO_
	(
	MiMonthInfoBlock	*month_info,
	MiAllMonthInfo		*all_month_info)
#else	/* no prototypes */
	(month_info, all_month_info)
	MiMonthInfoBlock	*month_info;
	MiAllMonthInfo		*all_month_info;
#endif	/* prototype */
    {
    int	    i ;
    
    /*
    **  For now, set all markup values to be the default.
    */
    
    for (i = 0 ;  i < month_info->month_length ;  i++) {
	month_info->markup [i] = 0 ;
	month_info->style  [i] = 0 ;
    }

    month_info->selected = 0 ;

    if (all_month_info->month_info_callback != NULL) {
	(*all_month_info->month_info_callback)
	  (all_month_info->month_info_tag,
	   month_info->month_number, month_info->year_number, 
	   month_info->month_length, month_info->markup, month_info->style) ;
    }
    
}

static MiMonthInfoBlock *create_month_info
#ifdef _DWC_PROTO_
	(
	int		month,
	int		year,
	MiAllMonthInfo	*all_month_info)
#else	/* no prototypes */
	(month, year, all_month_info)
	int		month;
	int		year;
	MiAllMonthInfo	*all_month_info;
#endif	/* prototype */
    {
    MiMonthInfoBlock	*month_info ;

    /*
    **	Allocate the month info structure.  Fill in the year, month, length
    **	of month and first weekday (day of week of the 1st of the month)
    **	fields.  Set the reference count to zero and go get month days
    **	markup.
    */
    
    month_info = (MiMonthInfoBlock *) XtMalloc (sizeof (MiMonthInfoBlock));
    if (month_info == NULL)  return (NULL) ;

    month_info->year_number     = year ;    
    month_info->month_number    = month ;    
    month_info->month_length    = DATEFUNCDaysInMonth (month, year) ;    
    month_info->first_weekday   = DATEFUNCDayOfWeek (1, month, year) ;    
    month_info->reference_count = 0 ;

    MIGetMonthMarkup (month_info, all_month_info) ;
    
    return (month_info) ;

}

static MiListOfMonthInfoList *allocate_month_list_list
#ifdef _DWC_PROTO_
	(
	MiMonthInfoList	*month_info_list)
#else	/* no prototypes */
	(month_info_list)
	MiMonthInfoList	*month_info_list;
#endif	/* prototype */
    {
    MiListOfMonthInfoList   *mill ;

    /*
    **	Allocate the month info list list structure and attach passed month
    **	info list to it.  Set next & last pointers to null.
    */
    
    mill = (MiListOfMonthInfoList *) XtMalloc (sizeof (MiListOfMonthInfoList));
    if (mill == NULL)  return (NULL) ;

    mill->next       = NULL ;
    mill->last       = NULL ;
    mill->first_mil  = month_info_list ;
    mill->final_mil  = month_info_list ;

    return (mill) ;

}

static MiMonthInfoList *create_month_info_list
#ifdef _DWC_PROTO_
	(
	int		month,
	int		year,
	MiAllMonthInfo	*all_month_info)
#else	/* no prototypes */
	(month, year, all_month_info)
	int		month;
	int		year;
	MiAllMonthInfo	*all_month_info;
#endif	/* prototype */
    {
    MiMonthInfoList   *month_info_list ;
    MiMonthInfoBlock	    *month_info ;

    /*
    **  Get the month info structure for the passed month & year then allocate
    **  a month info list structure and attach the month info to it.
    */
    
    month_info = create_month_info (month, year, all_month_info) ;
    if (month_info == NULL)  return (NULL) ;

    month_info_list = allocate_month_list(month_info) ;
    if (month_info_list == NULL)
    {
	XtFree ((char *)month_info);
	return (NULL);
    }

    return (month_info_list);

}

static MiMonthInfoList *get_month_info
#ifdef _DWC_PROTO_
	(
	int		month,
	int		year,
	MiAllMonthInfo	*all_month_info)
#else	/* no prototypes */
	(month, year, all_month_info)
	int		month;
	int		year;
	MiAllMonthInfo	*all_month_info;
#endif	/* prototype */
    {
    MiMonthInfoList   *month_info_list ;

    /*
    **  If the cache has anything in it then look through the cache for the
    **  month & year wanted.
    */
    
    if (all_month_info->cache_count != 0) {
	month_info_list = all_month_info->cache ;

	while (TRUE) {
	    if ((month_info_list->month->year_number  == year) &&
		(month_info_list->month->month_number == month)) {

		/*
		**  Found it.  Decouple from the circular cache.  If it was
		**  at the front of the cache then move the cache pointer back
		**  to the previous entry (if there is one).  Don't forget to
		**  decrement the cache count.
		*/
		
		month_info_list->next->last = month_info_list->last ;
		month_info_list->last->next = month_info_list->next ;

		if (month_info_list == all_month_info->cache) {
		    if (month_info_list->last == month_info_list) {
			all_month_info->cache = NULL ;
		    } else {
			all_month_info->cache = month_info_list->last ;
		    }
		}
		all_month_info->cache_count = all_month_info->cache_count - 1 ;

		/*
		**  Null out pointers, set reference count to one and return
		**  the structure.
		*/

		month_info_list->next = NULL ;
		month_info_list->last = NULL ;
		month_info_list->month->reference_count = 1 ;

		return (month_info_list) ;
	    }

	    /*
	    **  Move backwards through cache (LIFO).  If we're back where we
	    **  started then exit the loop.
	    */

	    month_info_list = month_info_list->last ;
	    if (month_info_list == all_month_info->cache) {
		break ;
	    }
	}
    }

    /*
    **  Didn't find it in the cache.  Oh well, go and create the structure and
    **  get the info from the file.  Set the reference count to one and return
    **  the structure.
    */

    month_info_list = create_month_info_list (month, year, all_month_info) ;
    if (month_info_list == NULL) {
	return (NULL) ;
    }
    
    month_info_list->month->reference_count = 1 ;
    return (month_info_list) ;

}

static void join_month_info_list_lists
#ifdef _DWC_PROTO_
	(
	MiListOfMonthInfoList	*first,
	MiListOfMonthInfoList	*second,
	MiAllMonthInfo		*all_month_info)
#else	/* no prototypes */
	(first, second, all_month_info)
	MiListOfMonthInfoList	*first;
	MiListOfMonthInfoList	*second;
	MiAllMonthInfo		*all_month_info;
#endif	/* prototype */
    {

    /*
    **	Join the two lists by pointing the last item on the first list and
    **	the first item on the second list to each other
    */

    first->final_mil->next = second->first_mil ;
    second->first_mil->last = first->final_mil ;

    /*
    **	Adjust the end pointer for the first list and decouple the second list.
    **  If the second list was the one the all months structure pointed to
    **  then move the pointer on.  Anyway, throw away the second list header.
    */

    first->final_mil = second->final_mil ;
    
    if (second->last != NULL)  second->last->next = second->next ;
    if (second->next != NULL)  second->next->last = second->last ;

    if (all_month_info->listlist == second) {
	all_month_info->listlist = second->next ;
    }

    XtFree ((char *)second);

}

void MIPurgeMonthCache
#ifdef _DWC_PROTO_
	(
	MiAllMonthInfo	*all_month_info)
#else	/* no prototypes */
	(all_month_info)
	MiAllMonthInfo	*all_month_info;
#endif	/* prototype */
    {
    MiMonthInfoList   *mil ;

    while (all_month_info->cache_count > 0) {
	mil = all_month_info->cache->next ;
	mil->last->next = mil->next ;
	mil->next->last = mil->last ;
	XtFree ((char *)mil->month);
	XtFree ((char *)mil);
	all_month_info->cache_count-- ;
    }

}
