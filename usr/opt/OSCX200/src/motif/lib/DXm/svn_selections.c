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

/*
*=======================================================================
*
*			  COPYRIGHT (c) 1988, 1989, 1990, 1991 BY
*	      DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*
* THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
* ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
* INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
* COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
* OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
* TRANSFERRED. 
*
* THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
* AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
* CORPORATION.
*
* DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
* SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*
*=======================================================================
*/

/******************************************************************************/
/*									      */
/*   FACILITY:								      */
/*									      */
/*        SVN -- Structured Visual Navigation Widget 			      */
/*									      */
/*   ABSTRACT:								      */
/*									      */
/*									      */
/*   AUTHORS:								      */
/*									      */
/*									      */
/*   CREATION DATE:							      */
/*									      */
/*   MODIFICATION HISTORY:						      */
/*									      */
/*	019	AN			20-Oct-1992			      */
/*		Add new field to SvnEventInfo structure to contain XEvent     */
/*		structure.						      */
/*	017	CS			20-Mar-1992			      */
/*		Fix DXmSvnSelectEntry so it handles an entry number out	      */
/*		of range.						      */
/*	016	AN			05-Dec-1991			      */
/*		Add bug fix in SelectSelectSet routine so that a check is     */
/*		made to see if there are any entries in SVN at all.	      */
/*	015	AN			10-Apr-1991			      */
/*		Change references to svn_widget in prototype def's to Widget. */
/*	014	AN			19-Mar-1991			      */
/*		Fix bug in DXmSvnSelectEntry so that if current entry selected*/
/*		number would be same as the previously selected entry...      */
/*		the location cursor would not be moved off previous entry.    */
/*	013	AN			23-Oct-1990			      */
/*		Fix bug in DXmSvnSelectComponent routine to redisplay	      */
/*		location cursor.					      */
/*	012	AN			17-Oct-1990			      */
/*		Fix bug in DXmSvnSelectComponent and DXmSvnSelectEntry where  */
/*		it was not resetting the location cursor to the newly	      */
/*		selected entry.						      */
/*	011	AN			20-Sep-1990			      */
/*		Added support for location cursor for new Motif style	      */
/*		guide mouse semantics into selection routines.		      */
/*	010	AN			28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	009	SL			20-Mar-1990			      */
/*		Fix accvios that can occur in various external routines       */
/*		when an invalid entry number is passed into them.             */
/*	008	AN			28-Feb-1990			      */
/*		Make sure that SvnGetHightlighted returns array of entry-tags */
/*	007	SL			01-Feb-1990			      */
/*		Add #pragma standard lines to compile cleanly with /STAN=PORT.*/
/*      006	WW			26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	005	WW			25-Jan-1990			      */
/*		Change DECwWsSvn*.h to DECwMSvn*.h.			      */
/*	004	SL			23-Jan-1990			      */
/*		Change the naming style of the constants from Svn* to SvnK*.  */
/*		This fixes the conflict that arose between the constant	      */
/*		SvnSelectEntry and the entry point name.		      */
/*	003	SL			23-Jan-1990			      */
/*		Change all BCSESVN references to SVN.  Change typedef	      */
/*		SvnWidget references to svn_widget.			      */
/*      002     SL			12-Jan-1990                           */
/*              Make some additional adjustments to work under Motif.         */
/*              Fix up the include files.                                     */
/*	001	SL			05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/

#define  SVN_SELECTIONS
#ifdef VMS
/*#pragma nostandard*/
#include "XmP.h"
/*#pragma standard*/
#else
#include <Xm/XmP.h>
#endif

#include "DXmSvnP.h"
#include "svnprivate.h"


#define not_first     0
#define first_of_one  1
#define first_of_many 3

/*
**  Local Routine declarations
*/
       void LclSelectEntry    ();
       void LclUnselectEntry  ();

static void LclRedrawSelected ();

static void LclHighlightEntry    ();
static void LclUnhighlightEntry  ();
static void LclRedrawHighlighted ();


/*
**  The following local procedure determines if the entry is selected based
**  on the multiple selection modes.  If we are not in column mode, then this
**  is exactly equivalent to the selected field in the entry.
*/
    Boolean LclEntrySelected ();

/*
** Here is the DXmSvnGetNumSelections public entry point.
*/

int DXmSvnGetNumSelections (w)

  Widget w;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Return to the caller the current number of selections.
*/
    return svnw->svn.num_selections;
}

/*
** Here is the DXmSvnGetNumHighlighted public entry point.
*/

int DXmSvnGetNumHighlighted (w)

    Widget w;
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Return to the caller the current number of highlighted entries.
*/
    return svnw->svn.num_highlighted;
}

/*
**  Here is the DXmSvnGetSelections public entry point.
**
**  Go thru the entire list, if an entry is selected add the entry number to
**  the selections array.  If we get to the end of the list before all the
**  selections were found, issue a warning and return.  If there are any
**  remaining elements in the array after the selections were found, set them
**  to NULL.
**
**  If the array passed by the application is shorter than the number of 
**  selections, then give the application only the first portion.
*/

void DXmSvnGetSelections (w, selections, comps, entry_tags, num_array_entries)

  Widget w;
  int *selections;
  int *comps;
  XtPointer *entry_tags;
  int num_array_entries;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;
    int i, entry_number, number_to_copy;


/*
**  Figure out the number of entries to copy based on the length of the array
**  and the number of entries that are selected.
*/
    if (svnw->svn.num_selections > num_array_entries)
         number_to_copy = num_array_entries;
    else number_to_copy = svnw->svn.num_selections;
 

/*
**  Get the first entry, so the next entry interface can be used.
*/
    entry_number = 1;
    svnentry = svnw->svn.entryPtr;


/*
**  Loop thru the entries, setting the arrays if the entry is selected
*/
    for (i = 0;  i < number_to_copy;  i++)
        {
	  while (!svnentry->selected)
             {
               svnentry = svnentry->next;
               entry_number++;
             };

	  selections[i] = entry_number;

	  if (comps != (int *) NULL) comps[i] = svnentry->selected_comp;
	  if (entry_tags != (XtPointer *) NULL) entry_tags[i] = svnentry->entry_tag;

	  svnentry = svnentry->next;
	  entry_number++;
	};


/*  
**  Set any remaining array elements to null
*/
    for (i = number_to_copy + 1;  i < num_array_entries;  i++)
        {
          selections[i] = 0;
          if (comps != (int *) NULL) comps[i] = 0;
          if (entry_tags != (XtPointer *) NULL) entry_tags[i] = 0;
        };
}

/*
**  Here is the DXmSvnGetHighlighted public entry point.
**
**  Go thru the entire list, if an entry is highlighted add the entry number to
**  the highlighted array.  If we get to the end of the list before all the
**  highlighted entries were found, issue a warning and return.  If there are
**  any remaining elements in the array after the highlighted entries were
**  found, set them to NULL.
**
**  If the array passed by the application is shorter than the number of 
**  highlighted entries, then give the application only the first portion.
*/

void DXmSvnGetHighlighted (w, highlighted, entry_tags, num_array_entries)

  Widget w;
  int *highlighted;
  XtPointer *entry_tags;
  int num_array_entries;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;
    int i, entry_number, number_to_copy;


/*
**  Figure out the number of entries to copy based on the length of the array
**  and the number of entries that are highlighted.
*/
    if (svnw->svn.num_highlighted > num_array_entries)
         number_to_copy = num_array_entries;
    else number_to_copy = svnw->svn.num_highlighted;
 

/*
**  Get the first entry, so the next entry interface can be used.
*/
    entry_number = 1;
    svnentry = svnw->svn.entryPtr;


/*
**  Loop thru the entries, setting the arrays if the entry is highlighted
*/
    for (i = 0;  i < number_to_copy;  i++)
        {
	  while (!svnentry->highlighted)
             {
               svnentry = svnentry->next;
               entry_number++;
             };

	  highlighted[i] = entry_number;
	  if (entry_tags != (XtPointer *) NULL) entry_tags[i] = svnentry->entry_tag;

	  svnentry = svnentry->next;
	  entry_number++;
	};


/*  
**  Set any remaining array elements to null
*/
    for (i = number_to_copy + 1;  i < num_array_entries;  i++)
        {
          highlighted[i] = 0;
          if (entry_tags != (XtPointer *) NULL) entry_tags[i] = 0;
        };
}

/*
** Here is the DXmSvnClearSelection public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnClearSelection (w, entry_number)

  Widget w;
  int entry_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry pointer.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If it's selected, then unselect it...
*/
    if (svnentry->selected) LclUnselectEntry(svnw, svnentry, entry_number);
}

/*
** Here is the DXmSvnClearHighlight public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnClearHighlight (w, entry_number)

  Widget w;
  int entry_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry pointer.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If it's highlighted, then unhighlight it...
*/
    if (svnentry->highlighted) LclUnhighlightEntry(svnw, svnentry,
	entry_number);
}

/*
** Here is the DXmSvnClearSelections public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnClearSelections (w)

    Widget w;

{
    svn_widget svnw = StructFindSvnWidget (w);

    svnw->svn.last_event.event = NULL;
/*
**  Call the local routine to clear all of the selections
*/
    SelectClearSelections(svnw, 1, svnw->svn.num_entries);
}

/*
** Here is the DXmSvnClearHighlighting public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnClearHighlighting (w)

    Widget w;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Call the local routine to clear all of the highlighting
*/
    SelectClearHighlighting(svnw, 1, svnw->svn.num_entries);
}

/*
** Here is the DXmSvnSelectAll public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnSelectAll (w)

    Widget w;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;
    int entry_number;
    Boolean first = first_of_many;
	
    svnw->svn.last_event.event = NULL;

/*
**  Loop through all entries until the end of the list is encountered.  For 
**  each entry that is currently not selected, call the local routine that 
**  transitions an entry from the unselected to selected state.  We do this
**  because there is alot of activity generated during this transition like
**  informing the display manager and optionally informing the application.
*/
    entry_number = 1;
    svnentry = svnw->svn.entryPtr;
 
    while (svnentry != (DXmSvnEntryPtr) NULL)
	{
          if (!LclEntrySelected (svnw, svnentry, entry_number, 0))
             {
               LclSelectEntry (svnw, svnentry, entry_number, 0, CurrentTime, first);
               first = not_first;
             };
 	  svnentry = svnentry->next;
	  entry_number++;
	};
}

/*
** Here is the DXmSvnHighlightAll public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnHighlightAll (w)

    Widget w;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;
    int entry_number;


/*
**  Loop through all entries until the end of the list is encountered.  For
**  each entry that is currently not highlighted, call the local routine that
**  transitions an entry from the unhighlighted to highlighted state.  We do
**  this because there is alot of activity generated during this transition
**  like informing the display manager and optionally informing the
**  application.
*/
    entry_number = 1;
    svnentry = svnw->svn.entryPtr;
 
    while (svnentry != (DXmSvnEntryPtr) NULL)
	{
          if (!svnentry->highlighted) LclHighlightEntry (svnw, svnentry, entry_number);
 	  svnentry = svnentry->next;
	  entry_number++;
	};
}

/*
** Here is the DXmSvnSelectComponent public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnSelectComponent (w, entry_number, comp_number)

  Widget w;
  int entry_number, comp_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif

    svnw->svn.last_event.event = NULL;
/*
**  Get the entry pointer.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);
 

/*
**  Select it if necessary
*/
    if (!LclEntrySelected (svnw, svnentry, entry_number, comp_number))
       LclSelectEntry(svnw, svnentry, entry_number, comp_number, CurrentTime, first_of_one);


    /******************************************************************************/
    /*                                                                            */
    /* Check to see if the location cursor is at the new selection entry.  If not */
    /* then set it there.							  */
    /*                                                                            */
    /******************************************************************************/
	{
	int prev_entry = svnw->svn.location_cursor;
	
	svnw->svn.location_cursor = entry_number;
	svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw, svnw->svn.location_cursor);
	DisplayHighlightEntry(svnw, prev_entry);
	DisplayHighlightEntry(svnw, svnw->svn.location_cursor);
	}
}

/*
** Here is the DXmSvnSelectEntry public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnSelectEntry (w, entry_number)

  Widget w;
  int entry_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;


    svnw->svn.last_event.event = NULL;

/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif

    if ((entry_number < 1) || (entry_number > svnw->svn.num_entries))
	return;

/*
**  Get the entry pointer
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);
 

/*
**  Select it if necessary
*/
    if (!LclEntrySelected (svnw, svnentry, entry_number, 0))
       LclSelectEntry(svnw, svnentry, entry_number, 0, CurrentTime, first_of_one);


    
    /******************************************************************************/
    /*                                                                            */
    /* Check to see if the location cursor is at the new selection entry.  If not */
    /* then set it there.							  */
    /*									          */
    /******************************************************************************/
    {
    int prev_entry = svnw->svn.location_cursor;
	
    svnw->svn.location_cursor = entry_number;
    svnw->svn.location_cursor_entry = StructGetEntryPtr(svnw, svnw->svn.location_cursor);
    DisplayHighlightEntry(svnw, prev_entry);
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);
    }
    
}

/*
** Here is the DXmSvnHighlightEntry public entry point.
**
** This routine can assume that the widget is currently disabled to avoid any
** conflicts with user actions.
*/

void DXmSvnHighlightEntry (w, entry_number)

  Widget w;
  int entry_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry pointer
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);
 

/*
**  Highlight it if necessary
*/
    if (!svnentry->highlighted)
       LclHighlightEntry(svnw, svnentry, entry_number);
}

/*
** Here is the DXmSvnHideSelections public entry point.
*/

void DXmSvnHideSelections (w)

    Widget w;
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  If we already lost them before or we don't own them, then ignore this call
*/
    if (!svnw->svn.show_selections) return;


/*
**  Show that we own the global selection
*/
    svnw->svn.show_selections = FALSE;


/*
**  Actions needing taken TBS
*/
    LclRedrawSelected (svnw);
}

/*
** Here is the DXmSvnHideHighlighting public entry point.
*/

void DXmSvnHideHighlighting (w)

    Widget w;
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  If we already lost them before or we don't own them, then ignore this call
*/
    if (!svnw->svn.show_highlighting) return;


/*
**  Show that highlighting is off
*/
    svnw->svn.show_highlighting = FALSE;


/*
**  Actions needing taken TBS
*/
    LclRedrawHighlighted (svnw);
}

/*
** Here is the DXmSvnShowSelections public entry point.
*/

void DXmSvnShowSelections (w)

    Widget w;
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  If we already own them before, then ignore this request.
*/
    if (svnw->svn.show_selections) return;


/*
**  Show that we own the global selection
*/
    svnw->svn.show_selections = TRUE;


/*
**  Actions needing taken TBS
*/
    LclRedrawSelected (svnw);
}

/*
** Here is the DXmSvnShowHighlighting public entry point.  Refuse to do this
** if expect_selections is not set.
*/

void DXmSvnShowHighlighting (w)

    Widget w;
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  If we already own them before, then ignore this request.
*/
    if (svnw->svn.show_highlighting) return;


/*
**  If they did not set the DXmSvnNexpectHighlighting resource to true, then ignore
**  this request...
*/
    if (svnw->svn.expect_highlighting == FALSE) return;


/*
**  Show that highlighting is on
*/
    svnw->svn.show_highlighting = TRUE;


/*
**  Actions needing taken TBS
*/
    LclRedrawHighlighted (svnw);
}

/*
**  Local routine that redraws all of the selected entries that is called from
**  hide and show.
*/

static void LclRedrawSelected (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int i, entry_number;


/*
**  If there are no selections, then we are done
*/
    if (svnw->svn.num_selections == 0) return;


/*
**  Get the first entry, so the next entry interface can be used.
*/
    entry_number = 1;
    svnentry = svnw->svn.entryPtr;


/*
**  Loop thru the entries, calling the hightlight procedure if selected
*/
    for (i = 1;  i <= svnw->svn.num_selections;  i++)
        {
	  while (!svnentry->selected)
             {
               svnentry = svnentry->next;
               entry_number++;
             };

          DisplayHighlightEntry (svnw, entry_number);

	  svnentry = svnentry->next;
	  entry_number++;
	};
}

/*
**  Local routine that redraws all of the highlighted entries that is called
**  from hide and show.
*/

static void LclRedrawHighlighted (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int i, entry_number;


/*
**  If there are no highlighted entries, then we are done
*/
    if (svnw->svn.num_highlighted == 0) return;


/*
**  Get the first entry, so the next entry interface can be used.
*/
    entry_number = 1;
    svnentry = svnw->svn.entryPtr;


/*
**  Loop thru the entries, setting the arrays if the entry is highlighted
*/
    for (i = 1;  i <= svnw->svn.num_highlighted;  i++)
        {
	  while (!svnentry->highlighted)
             {
               svnentry = svnentry->next;
               entry_number++;
             };

          DisplayHighlightEntry (svnw, entry_number);

	  svnentry = svnentry->next;
	  entry_number++;
	};
}

/*
** Here is the low level widget private routine to toggle selections.  This 
** routine cannot assume that the widget is disabled.  Note that since this
** routine is not invoked from outside of the SVN widget, the widget parameter
** passed need not be cast into an svn structure.
*/

void SelectToggleSelections (svnw, first, last, compnm, time)

  svn_widget svnw;
  int first, last, compnm;
  Time time;

{
/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;
    int i, select_them_all = FALSE;
    Boolean first_select = first_of_many;


/*
**  Loop from first to last seeing if a single entry is selected.  If they are
**  all selected, then we will unselect all of them.  If there is one or more
**  not selected, then we will select all of them.
*/
    for (i = first;  i <= last;  i++)
        {
          svnentry = StructGetEntryPtr (svnw, i);
          if (!LclEntrySelected (svnw, svnentry, i, compnm))
             {
               select_them_all = TRUE;
               break;
             };
        };          


/*
**  Either select them all or clear them all.
*/
    for (i = first;  i <= last;  i++)
        {
          svnentry = StructGetEntryPtr (svnw, i);

          if ((select_them_all) && (!LclEntrySelected (svnw, svnentry, i, compnm)))
             {
               LclSelectEntry (svnw, svnentry, i, compnm, time, first_select);
               first_select = not_first;
             };      
          if ((!select_them_all) && (svnentry->selected))
             LclUnselectEntry (svnw, svnentry, i);
        };
}

/*
**  Here is the low level widget private routine to select a range of entries and
**  unselect all entries outside of the range.
*/

void SelectSelectSet (svnw, low_entry_number, high_entry_number, compnm, time)

  svn_widget svnw;
  int low_entry_number, high_entry_number, compnm;
  Time time;

{
/*
**  Local data
*/
    DXmSvnEntryPtr lowentry, highentry;
    int low_number, high_number, desired_number, num_grays_found = 0;
    Boolean first = first_of_many;


/* 
**  If this is one, then first_of_one 
*/
    if (low_entry_number == high_entry_number)
       first = first_of_one;
/*
**  Get the first entry pointer through the cache
*/
    lowentry  = StructGetEntryPtr (svnw, low_entry_number);
    highentry = lowentry;

	    
/*
**  Loop from the first entry to the last entry.  We will exit this loop with the 
**  highentry pointer pointing to the high_entry_number + 1 entry.
*/
    for ( high_number = low_entry_number;  high_number <= high_entry_number;  high_number++)

        {
          if (highentry->grayed) num_grays_found++;

          if ((!LclEntrySelected (svnw, highentry, high_number, compnm)) && (!highentry->grayed))
             {
               LclSelectEntry (svnw, highentry, high_number, compnm, time,first);
               first = not_first;
             };
          highentry = highentry->next;
        };


/*
**  Compute the desired number of selections
*/
    desired_number = high_entry_number - low_entry_number + 1;


/*
**  Adjust for the number of gray entries found
*/
    desired_number = desired_number - num_grays_found;


/*
**  If the proper number of entries are now selected, then leave.
**  If num_selections == 0 , then we don't need to select anything, just return.
*/
    if (svnw->svn.num_selections == desired_number)
       return;

    if (svnw->svn.num_selections == 0)
	return;


/*
**  Set up everything for the unselection loop.
*/
    low_number = low_entry_number;


/*
**  Do the following loop until the desired number of selections is hit...
*/
    while (TRUE)

      {
        /*
        **  Test previous entry
        */
            if (low_number != 1)
               {
                 /*
                 **  Get to the entry...
                 */
                     lowentry = lowentry->prev;
                     low_number--;
  

                 /*
                 **  Unselect it if it is selected
                 */
                     if (lowentry->selected)
                        {
                          LclUnselectEntry (svnw, lowentry, low_number);
                          if (svnw->svn.num_selections == desired_number) return;
                        };
               };


        /*
        **  Test next entry
        */
            if (high_number <= svnw->svn.num_entries)
               {
                 /*
                 **  Unselect it if it is selected
                 */
                     if (highentry->selected)
                        {
                          LclUnselectEntry (svnw, highentry, high_number);
                          if (svnw->svn.num_selections == desired_number) return;
                        };


                 /*
                 **  Get to the entry...
                 */
                     highentry = highentry->next;
                     high_number++;

               };
      };
}

/*
**  Here is the low level widget private routine to clear selections.  This 
**  routine cannot assume that the widget is disabled.  Note that since this
**  routine is not invoked from outside of the SVN widget, the widget parameter
**  passed need not be cast into an svn structure.
*/

void SelectClearSelections (svnw, first, last)

  svn_widget svnw;
  int first, last;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int i;


/*
**  If there are no selections, get out fast.
*/
    if (svnw->svn.num_selections == 0) return;


/*
**  Get the first entry pointer
*/
    svnentry = StructGetEntryPtr (svnw, first);


/*
**  Loop through all entries from first to last on the list.  For 
**  each entry that is currently selected, call the local routine that 
**  transitions an entry from the selected to ed state.  We do this
**  because there is alot of activity generated during this transition like
**  informing the display manager and optionally informing the application.
*/
    for (i = first;  i <= last;  i++)
	{
	  if (svnentry->selected) 
             {
               LclUnselectEntry(svnw, svnentry, i);
               if (svnw->svn.num_selections == 0) return;
             };
 	  svnentry = svnentry->next;
	};
}

/*
** Here is the local routine that actually selects the entry.  A precondition
** is that the entry is currently unselected. The routine will notify both the
** application and the display module.  The selected bit will be set and the
** number of selected entries will be incremented.
*/

void LclSelectEntry (svnw, svnentry, entnm, compnm, time, first)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  int entnm, compnm;
  Time time;
  int first;

{
/*
**  Local data declarations
*/
    DXmSvnCallbackStruct temp;


/*
**  Ignore call if entry is insensitive
*/
    if (svnentry->grayed) return;

    

    svnentry->selected = TRUE;
    svnentry->selected_comp = compnm;
    svnw->svn.num_selections++;

/*
**  Notify the DXmSvn display module
*/
    DisplayHighlightEntry (svnw, entnm);




/*
**  Notify the application
*/
    temp.reason = DXmSvnCREntrySelected;
    temp.entry_number = entnm;
    temp.component_number = compnm;
    temp.entry_tag = svnentry->entry_tag;
    temp.time = time;
    temp.first_selection = first;
    temp.entry_level = svnentry->level;
    temp.event = svnw->svn.last_event.event;

/*
**  Issue the callback
*/
    XtCallCallbacks ((Widget)svnw, DXmSvnNentrySelectedCallback, &temp);


/*
**  Show that they may need reported
*/
    svnw->svn.transitions_made = TRUE;
}

/*
** Here is the local routine that actually unselects the entry.  A precondition
** is that the entry is currently selected. The routine will notify both the
** application and the display module.  The selected bit will be off and the
** number of selected entries will be decremented.
*/

void LclUnselectEntry (svnw, svnentry, entnm)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  int entnm;

{
/*
**  Local data declarations
*/
    DXmSvnCallbackStruct temp;


/*
**  Unselect this entry
*/

    svnentry->selected = FALSE;
    svnentry->selected_comp = 0;
    svnw->svn.num_selections--;
 

/*
**  Notify the DXmSvn display module 
*/
    DisplayHighlightEntry (svnw, entnm);

/*
**  Notify the application
*/
    temp.reason = DXmSvnCREntryUnselected;
    temp.entry_number = entnm;
    temp.entry_tag = svnentry->entry_tag;
    temp.entry_level = svnentry->level;
    temp.event = svnw->svn.last_event.event;


/*
**  Issue the callback
*/
    XtCallCallbacks ((Widget)svnw, DXmSvnNentryUnselectedCallback, &temp);


/*
**  Show that they may need reported
*/
    svnw->svn.transitions_made = TRUE;
}

/*
**  Here is the low level widget private routine to clear highlighting.  This 
**  routine cannot assume that the widget is disabled.  Note that since this
**  routine is not invoked from outside of the SVN widget, the widget parameter
**  passed need not be cast into an svn structure.
*/

void SelectClearHighlighting (svnw, first, last)

  svn_widget svnw;
  int first, last;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int i;


/*
**  If there are no highlighted entries, get out fast.
*/
    if (svnw->svn.num_highlighted == 0) return;


/*
**  Get the first entry pointer
*/
    svnentry = StructGetEntryPtr (svnw, first);


/*
**  Loop through all entries from first to last on the list.  For 
**  each entry that is currently highlighted, call the local routine that 
**  transitions an entry from the highlighted to ed state.  We do this
**  because there is alot of activity generated during this transition like
**  informing the display manager and optionally informing the application.
*/
    for (i = first;  i <= last;  i++)
	{
	  if (svnentry->highlighted) 
             {
               LclUnhighlightEntry(svnw, svnentry, i);
               if (svnw->svn.num_highlighted == 0) return;
             };
 	  svnentry = svnentry->next;
	};
}

/*
**  This routine is called to conditionally report the end of a set of 
**  select/unselect transitions...
*/

void SelectReportTransitions (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnCallbackStruct temp;


/*
**  If the flag is false, then simply return...
*/  
    if (!svnw->svn.transitions_made) return;


/*
**  Show that they have been reported
*/
    svnw->svn.transitions_made = FALSE;


/*
**  Issue the callback
*/
    temp.reason = DXmSvnCRTransitionsDone;
    XtCallCallbacks ((Widget)svnw, DXmSvnNtransitionsDoneCallback, &temp);
}

/*
** Here is the local routine that actually highlights the entry.  A
** precondition is that the entry is currently unhighlighted. The routine will
** notify both the application and the display module.  The highlighted bit
** will be set and the number of highlighted entries will be incremented.
*/

static void LclHighlightEntry (svnw, svnentry, entnm)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  int entnm;

{
/*
**  Set the values 
*/
    svnentry->highlighted = TRUE;
    svnw->svn.num_highlighted++;
 

/*
**  Notify the DXmSvn display module
*/
    DisplayHighlightEntry (svnw, entnm);
}

/*
** Here is the local routine that actually unhighlights the entry.  A
** precondition is that the entry is currently highlighted. The routine will
** notify both the application and the display module.  The highlighted bit
** will be off and the number of highlighted entries will be decremented.
*/

static void LclUnhighlightEntry (svnw, svnentry, entnm)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  int entnm;

{
/*
**  Unhighlight this entry
*/
    svnentry->highlighted = FALSE;
    svnw->svn.num_highlighted--;
 

/*
**  Notify the DXmSvn display module 
*/

    DisplayHighlightEntry (svnw, entnm);
}

/*
**  This procedure determines if an entry is selected according to the 
**  selection modes.  This only matters in columns display mode since the user
**  can select a particular component on the right hand side.  All of this work
**  is so a user can select component 'n' and then select component 'n+1'.  The
**  callers of this procedure want to know if a particular entry is selected or
**  not.
**
**  This routine will automatically unselect entries that are necessary.
*/

Boolean LclEntrySelected (svnw, svnentry, entry_number, comp_number)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  int entry_number, comp_number;

{
/*
**  If the entry is not selected, then return FALSE right away.
*/
    if (!svnentry->selected) return FALSE;


/*
**  If we are not in column mode, then return TRUE right away.
*/
    if (svnw->svn.display_mode != DXmSvnKdisplayColumns) return TRUE;


/*
**  If they are on the same component, then return TRUE.
*/
    if (comp_number == svnentry->selected_comp) return TRUE;


/*
**  If we are in DXmSvnKselectEntry mode, then reverse video does not
**  change.  Return TRUE right away.
*/
    if (svnw->svn.selection_mode == DXmSvnKselectEntry) return TRUE;


/*
**  If the new component number is on the left and the selected component 
**  is also on the left, then return TRUE (that the entry is selected).
*/
    if ((comp_number < svnw->svn.start_column_component) && (svnentry->selected_comp < svnw->svn.start_column_component))
       return TRUE;


/*
**  We now know that at least one of the two components is on the right
**  and that the components are different.
*/
    LclUnselectEntry (svnw, svnentry, entry_number);
    return FALSE;
}
