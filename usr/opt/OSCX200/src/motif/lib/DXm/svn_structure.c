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
*                 COPYRIGHT (c) 1988, 1989, 1992 BY
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
/*	023	AD			16-Jul-1993			      */
/*		Fix bug in StructFindSvnWidget which causes a Toolkit abort   */
/*		if widget isn't an Svn widget.
/*	022	AN			21-Apr-1993			      */
/*		Add fix in StructTranslateCoords routine so that it takes     */
/*		into account vertical scrollbar on r->l layouts.	      */
/*	021	AN			09-Mar-1993			      */
/*		Add section of code to check if string is the same as stored  */
/*		in the routine DXmSvnSetComponentText.			      */
/*      020     CS                      11-Aug-1992                           */
/*              Add function StructTranslateCoords.                           */
/*	019	CS			25-Mar-1992			      */
/*		Use bcopy instead of memmove on the Sun because there is no   */
/*		memmove function in the Sun C run time library.		      */
/*	018	CS			 19-Mar-1992			      */
/*		Add code to reallocate memory for svn.path_entries.	      */
/*	017	CS			 2-Jan-1992			      */
/*		Made a few changes in how subwidgets are handled.	      */
/*	016	A. Napolitano		25-Jun-1991			      */
/*		Change memcpy to memmove in StructOpenArray routine, because  */
/*		of problems on ULTRIX where the copy was only moving the      */
/*		lower half of the memory if memory was overlapping.	      */
/*	015	A. Napolitano	       10-Apr-1991			      */
/*		Change references to svn_widget in prototype def's to Widget. */
/*	014	A. Napolitano		19-Mar-1991			      */
/*		Added routine DXmSvnGetComponentText.			      */
/*	013	A. Napolitano		20-Sep-1990			      */
/*		Added support for location cursor for new Motif style	      */
/*		guide mouse semantics.					      */
/*	012	A. Napolitano		11-Jul-1990			      */
/*		Change names of variables and anything external to be	      */
/*		internationalized for sides or pane windows.  "rhs"->secondary*/
/*		and "lhs" -> primary.					      */
/*	011	A. Napolitano		28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	010	A. Napolitano		25-Jun-1990			      */
/*		Changes made so that only compound strings are supported      */
/*		instead of text strings and compound strings.		      */
/*	009	S. Lavigne 		25-Apr-1990			      */
/*		Move conditional SVNDEBUG code down in routine		      */
/*		SvnGetComponentText to fix a compilation error.               */
/*	008	A. Napolitano		09-Apr-1990			      */
/*		Take out code that is conditionally compiled out with the     */
/*		symbol DECW_V2.	Now Motif version code will only support      */
/*		compound strings.					      */
/*	007	S. Lavigne 		19-Mar-1990			      */
/*		Add new routines SvnGetEntryLevel and SvnGetEntrySensitivity. */
/*		Fix accvios that can occur in external routines SvnSet* and   */
/*		SvnGet* when an invalid entry number is passed into them.     */
/*		Make index_window param a Boolean in SvnSetEntryIndexWindow.  */
/*	006	S. Lavigne 		01-Feb-1990			      */
/*		Add #pragma standard lines to compile cleanly with /STAN=PORT.*/
/*      005	Will Walker		26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	004	Will Walker		25-Jan-1990			      */
/*		Change DECwWsSvn*.h to DECwMSvn*.h.			      */
/*	003	S. Lavigne 		23-Jan-1990			      */
/*		Change all BCSESVN routine references to SVN.  Change all     */
/*		SvnWidget typedef references to svn_widget.  Change the	      */
/*		naming style of the constants from Svn* to SvnK*.	      */
/*      002     S. Lavigne              15-Jan-1990                           */
/*              Make some additional post-converter adjustments to work	      */
/*		under Motif.  Fix up the include files, callback reasons,     */
/*		compound string handling, free compound strings using	      */
/*		XmStringFree instead of XtFree, modify parameters on the      */
/*		XmStringGetNextSegment call.				      */
/*	001	S. Lavigne 		05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/

#define  SVN_STRUCTURE

#ifdef VMS
/*#pragma nostandard*/
#include "XmP.h"
/*#pragma standard*/
#else
#include <Xm/XmP.h>
#endif

#include "DXmPrivate.h"
#include "DXmSvnP.h"
#include "svnprivate.h"

/*
** Local Routines
*/

void        LclLinkAddEntry         ();
DXmSvnEntryPtr LclLinkDeleteEntry      ();
static void LclDeallocateComponent  ();
static void LclDetermineValidity    ();
static DXmSvnEntryPtr LclStructGetCachePtr    ();


/*
** Here is the DXmSvnSetEntry public entry point using the C language interface.
**
** This routine has a precondition that the entry being set is currently 
** INVALID.  The source module can force this via the DXmSvnInvalidateEntry
** high level routine.
*/

#ifdef _NO_PROTO
void DXmSvnSetEntry (w, entry_number, width, height, num_components, sensitivity, entry_tag, index_window)

  Widget w;
  int entry_number, width, height, num_components;
  Boolean sensitivity;
  XtPointer entry_tag;
  Boolean index_window;
#else
void DXmSvnSetEntry (Widget w, int entry_number, int width, int height, int num_components, Boolean sensitivity, XtPointer entry_tag, Boolean index_window)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry structure that was allocated during the AddEntries call.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Set the information in the entry that can only be set in this call.
*/
    svnentry->width       = width;
    svnentry->orig_height = height;


/*
**  Set the other information
*/
    svnentry->entry_tag    = entry_tag;
    svnentry->index_window = index_window;


/*
**  If index_window is true, then set index_window_needed
*/
    if (index_window) svnw->svn.index_window_needed = TRUE;


/*
**  Sensitivity is the opposite of grayed.
*/
    if (sensitivity)
         svnentry->grayed = FALSE;
    else svnentry->grayed = TRUE;


/*
**  Call the specialized procedure to set the number of components in the entry.
*/
    DXmSvnSetEntryNumComponents ((Widget) svnw, entry_number, num_components);


/* 
**  Call the local routine which will determine whether the correct number of
**  components for the entry has been defined.
*/
    LclDetermineValidity (svnentry);
}

/*
**  Here is the DXmSvnSetEntrySensitivity public entry point using the C 
**  language interface.  The sensitivity is the opposite Boolean value of
**  the grayed one...
*/

#ifdef _NO_PROTO
void DXmSvnSetEntrySensitivity (w, entry_number, sensitivity)

  Widget w;
  int entry_number;
  Boolean  sensitivity;

#else
void DXmSvnSetEntrySensitivity (
  Widget w,
  int entry_number,
  Boolean  sensitivity)

#endif
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Routine data declarations
*/
    DXmSvnEntryPtr svnentry;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry record from the struct module.  Use the get valid one to be
**  sure that the information is accurate with the source module.  
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If the new value is the same as the old, then change the bit and tell 
**  display about it.
*/
    if (svnentry->grayed == sensitivity)
       {
        if (svnentry->grayed) svnentry->grayed = FALSE; 
        else svnentry->grayed = TRUE;

        if ((svnentry->grayed) && (svnentry->selected))
           SelectClearSelections (svnw, entry_number, entry_number);

        DisplaySvnSetEntrySensitivity (svnw, entry_number);
       };
} 

/*
** Here is the DXmSvnSetEntryNumComponents public entry point using the C language interface.
**
** This routine has a precondition that the entry being set is currently 
** INVALID.  The source module can force this via the DXmSvnInvalidateEntry
** high level routine.
**
** Note that this procedure will not release component information if the number
** of components has changed.  This is different than previous incarnations of
** this procedure.  Callers may need to call invalidate entry explicitly.
*/

void DXmSvnSetEntryNumComponents (w, entry_number, num_components)

  Widget w;
  int entry_number, num_components;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int i;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry structure that was allocated during the AddEntries call.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If components have never been allocated for this entry, then allocate 
**  memory for the component array initialized to zeros.  Commit the number
**  of components to avoid invalidating each of the components later.
*/
    if (svnentry->num_allocated == 0)
       {
         svnentry->entrycompPtr  = (DXmSvnCompPtr) XtCalloc (num_components, (sizeof(DXmSvnCompStruct)));
         svnentry->num_allocated = num_components;
       };


/*
**  If there are less components allocated than are currently needed, then 
**  extend the component array to hold the new components.  Initialize all of
**  the new component types to DXmSvnKcompNotSet.  This is because the reallocate
**  will not zero out the memory for us.  
*/
    if (svnentry->num_allocated < num_components)
       {
	 svnentry->entrycompPtr = (DXmSvnCompStruct *) XtRealloc ((char *)svnentry->entrycompPtr, num_components * sizeof(DXmSvnCompStruct));

         for (i = svnentry->num_allocated;  i <= num_components - 1;  i++)
             svnentry->entrycompPtr[i].type = DXmSvnKcompNotSet;

         svnentry->num_allocated = num_components;
       };
		     

/*
**  At this point, we know that the number of components allocated is large
**  enough.
**
**  If we now have more components, then mark the entry as being invalid.
*/
    if (svnentry->num_components < num_components)
       {
         svnentry->num_components = num_components;
         svnentry->valid = FALSE;
       };


/*
**  If the number of components less than before, then we don't have to change
**  the component allocation, but we do need to invalidate the components that
**  are no longer going to be used.
*/
    if (svnentry->num_components > num_components)
       {
         for (i = num_components;  i <= svnentry->num_components - 1;  i++)
	     {
	       if (svnentry->entrycompPtr[i].type == DXmSvnKcompText)
		  XmStringFree(svnentry->entrycompPtr[i].var.is_text.text);

	       svnentry->entrycompPtr[i].type = DXmSvnKcompNotSet;
	     };

         svnentry->num_components = num_components;
       };		     
}

/*
** Here is the DXmSvnSetEntryIndexWindow entry point using the C language interface.
*/

#ifdef _NO_PROTO
void DXmSvnSetEntryIndexWindow (w, entry_number, index_window)

  Widget w;
  int entry_number;
  Boolean index_window;

#else
void DXmSvnSetEntryIndexWindow (
  Widget w,
  int entry_number,
  Boolean index_window)
#endif
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local Data
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
**  Get the entry structure that was allocated during the AddEntries call.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Set the information in the entry.   
*/
    svnentry->index_window = index_window;


/*
**  If index_window is true, then set index_window_needed
*/
    if (index_window) svnw->svn.index_window_needed = TRUE;

} 

/*
** Here is the DXmSvnSetEntryTag entry point using the C language interface.
*/

void DXmSvnSetEntryTag (w, entry_number, entry_tag)

  Widget w;
  int entry_number;
  XtPointer entry_tag;

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
**  Get the entry structure that was allocated during the AddEntries call.
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Set the information in the entry.   
*/
    svnentry->entry_tag = entry_tag;
} 

/*
** Here is the DXmSvnSetComponentHidden public entry point using the C language interface.
*/

void DXmSvnSetComponentHidden (w, entry_number, component_number, hidden_mode)

  Widget w;
  int entry_number, component_number, hidden_mode;

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
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry structure
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Set the hidden_mode field
*/
    svnentry->entrycompPtr[component_number-1].hidden_mode = hidden_mode;
}

/*
**  Here is the DXmSvnSetComponentText public entry point using the C language interface.
*/

void DXmSvnSetComponentText (w, entry_number, component_number, x, y, cs, fontlist)

  Widget w;
  int entry_number, component_number, x, y;
  XmString cs;
  XmFontList fontlist;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;


/*
**  Make it a bit more convenient
*/
    int comp_offset = component_number - 1;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry and component structures
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);

    /* If the CS string coming in is the exact same string as the one we already */
    /* have stored in our internal structure, then we really don't want to free  */
    /* and recopy that same string, so just jump around this section.	 An      */
    /* example of this is if the appl. does a getcomponenttext and then a	 */
    /* setcomponenttext.							 */

    if (svnentry->entrycompPtr[comp_offset].var.is_text.text != cs)
	{
	if (svnentry->entrycompPtr[comp_offset].type == DXmSvnKcompText)
		XmStringFree(svnentry->entrycompPtr[comp_offset].var.is_text.text);


	/*
	**  Set the component information.  Must make a local copy of the string in
	**  dynamic memory.  We can assume that the delete routine is responsible for
	**  returning any SVN managed memory. 
	*/
	svnentry->entrycompPtr[comp_offset].var.is_text.text = XmStringCopy(cs);
	}


/*
**  Set the component information.  Must make a local copy of the string in
**  dynamic memory.  We can assume that the delete routine is responsible for
**  returning any SVN managed memory. 
*/
    svnentry->entrycompPtr[comp_offset].var.is_text.text = XmStringCopy(cs);


/*
**  Fill in the common component information
*/
    svnentry->entrycompPtr[comp_offset].type   = DXmSvnKcompText;
    svnentry->entrycompPtr[comp_offset].orig_x = x;
    svnentry->entrycompPtr[comp_offset].orig_y = y;


/*
**  Fill in the information pertaining to a text entry
*/
    svnentry->entrycompPtr[comp_offset].var.is_text.font_list = fontlist;

/* 
**  Call the local routine which will determine whether the correct number of
**  components for the entry has been defined.
*/
    LclDetermineValidity (svnentry);

}

/*
** Here is the DXmSvnGetComponentText public entry point using the C language
*/

XmString DXmSvnGetComponentText (w, entry_number, comp_number)

  Widget w;
  int entry_number, comp_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;
    XmString cs;
/*
**  Make accessing easier
*/
    int comp_offset = comp_number - 1;

    
/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry and component structures
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If this component number is too big, return NULL
*/
    if (svnentry->num_components < comp_number)
       return ((XmString) NULL);


/*
**  If this component is not text, then return a NULL pointer
*/
    if (svnentry->entrycompPtr[comp_offset].type != DXmSvnKcompText)
       return ((XmString) NULL);


/*
**  Return this pointer
*/
    return (svnentry->entrycompPtr[comp_offset].var.is_text.text);
}

/*
** Here is the DXmSvnSetComponentPixmap public entry point using C 
*/

void DXmSvnSetComponentPixmap (w, entry_number, comp_number, x, y, pixmap, width, height)

  Widget w;
  int entry_number, comp_number, x, y;
  Pixmap pixmap;
  int width, height;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;


/*
**  Make it a bit more convenient
*/
    int comp_offset = comp_number - 1;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Get the entry structure
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Free the old text component based on the old component type...  This is 
**  needed if changing component types...
*/
    if (svnentry->entrycompPtr[comp_offset].type == DXmSvnKcompText)
       XmStringFree(svnentry->entrycompPtr[comp_offset].var.is_text.text);


/*
**  Set the common component information.  
*/
    svnentry->entrycompPtr[comp_offset].type        = DXmSvnKcompPixmap;
    svnentry->entrycompPtr[comp_offset].orig_x      = x;
    svnentry->entrycompPtr[comp_offset].orig_y      = y;
    svnentry->entrycompPtr[comp_offset].width       = width;
    svnentry->entrycompPtr[comp_offset].height      = height;
    svnentry->entrycompPtr[comp_offset].orig_width  = width;
    svnentry->entrycompPtr[comp_offset].orig_height = height;


/*
**  Set the pixmap specific component information
*/
    svnentry->entrycompPtr[comp_offset].var.is_pixmap.pixmap = pixmap;


/*
**  Call the local routine which will determine whether the correct number of
**  components for the entry has been defined.
*/
    LclDetermineValidity (svnentry);
}

/*
** Here is the DXmSvnSetComponentWidget public entry point using C 
*/

void DXmSvnSetComponentWidget (w, entry_number, comp_number, x, y, subw)

  Widget w;
  int entry_number, comp_number, x, y;
  Widget subw;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;

/*
**  Make it a bit more convenient
*/
    int comp_offset = comp_number - 1;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
	XtError(SVN_ILLENTNUM);
#endif


/*
**  Mark the fact that this call had been made
*/
    svnw->svn.sub_widgets_used = TRUE;


/*
**  Get the entry structure
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Free the old text component based on the old component type...  This is 
**  needed if changing component types...
*/
    if (svnentry->entrycompPtr[comp_offset].type == DXmSvnKcompText)
       XmStringFree(svnentry->entrycompPtr[comp_offset].var.is_text.text);


/*
**  Set the component information.  
*/
    svnentry->entrycompPtr[comp_offset].type   = DXmSvnKcompWidget;
    svnentry->entrycompPtr[comp_offset].orig_x = x;
    svnentry->entrycompPtr[comp_offset].orig_y = y;


/*
**  Set the widget specific component information
*/
    svnentry->entrycompPtr[comp_offset].var.is_widget.readwrite_text = subw;


/*
**  position the subwidget out of site...
*/
    INIT_WIDGET(svnw, subw);

/*
**  Make sure the widget_list is large enough to hold all form children
*/
    if (svnw->svn.widget_list_number < XtNumChildren(svnw->svn.primary_form))
    {
	svnw->svn.widget_list_number = XtNumChildren(svnw->svn.primary_form) + WIDGET_LIST_INCREMENT;
	svnw->svn.widget_list = (WidgetList) XtRealloc((char *)svnw->svn.widget_list,
						       svnw->svn.widget_list_number * sizeof(Widget));
    }

/*
**  Call the local routine which will determine whether the correct number of
**  components for the entry has been defined.
*/
    LclDetermineValidity(svnentry);
}

/*
** Here is the DXmSvnGetEntryTag public entry point using the C language
*/

XtPointer DXmSvnGetEntryTag (w, entry_number)

  Widget w;
  int entry_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;


/*
**  If the entry number is out of bounds, return 0
*/
    if ((entry_number <= 0) || (entry_number > svnw->svn.num_entries))
       return (0);


/*
**  Get the entry structure
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Return the tag field
*/
    return (svnentry->entry_tag);
}

/*
** Here is the DXmSvnGetEntryNumber public entry point using the C language
*/

unsigned int DXmSvnGetEntryNumber (w, tag)

  Widget w;
  XtPointer tag;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    DXmSvnEntryPtr svnentry;
    int entry_number;


/*
**  Get the first entry structure
*/
    svnentry = svnw->svn.entryPtr;
    entry_number = 1;


/*
**  Spin through the following loop until the entry is found or the end of
**  the list is hit.
*/
    while (svnentry != (DXmSvnEntryPtr) NULL)
       {
         if (svnentry->entry_tag == tag) return entry_number;
         entry_number++;
         svnentry = svnentry->next;
       };


/*
**  No hit...
*/
    return 0;
}

/*
** Here is the DXmSvnGetEntryLevel public entry point using the C language
*/

unsigned int DXmSvnGetEntryLevel (w, entry_number)

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
**  Get the entry structure
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Return the level field
*/
    return (svnentry->level);

}

/*
** Here is the DXmSvnGetEntrySensitivity public entry point using the C language
** It returns:
**	0, if the entry is insensitive.
**	1, if the entry is sensitive.
*/

unsigned int DXmSvnGetEntrySensitivity (w, entry_number)

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
**  Get the entry structure
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  The svn entry contains a field called 'grayed', which is the opposite
**  of the sensitivity.  If an entry is grayed, then it is not sensitive.
*/

    if (svnentry->grayed) 
	return 0;
    else 
	return 1;

}

/*
** Here is the DXmSvnValidateAll public entry point using the C language
*/

void DXmSvnValidateAll (w)

    Widget w;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    int entry_number = 1;


/*
**  Leave if there are no entries
*/
    if (svnw->svn.num_entries == 0) return;


/*
**  Put out the watch cursor
*/
    DisplaySetWatch (svnw, TRUE);


/*
**  Spin in a loop getting a valid entry pointer for each SVN entry.  We will
**  leave the loop when the number of entries in the entire SVN is equal to
**  the entry that we just validated.
*/
    while (TRUE)

       {
         StructGetValidEntryPtr (svnw, entry_number);
         if (entry_number == svnw->svn.num_entries) break;
         entry_number++;
       };


/*
**  Cancel the watch cursor
*/
    DisplaySetWatch (svnw, FALSE);
}

/*
**  Here is the SVN private StructOpenArray routine.  Given the address of an
**  array, the size of an array entry, and the maximum length of the array,
**  it will open up a hole in the array at the provided offset.
**
**  If the array is actually full, then the last entry information will be 
**  lost when the hole is opened.
**
**  Example:  Array of 5 integers.  Want to open a hole at offset 2.
**
**       +-------+    +-------+
**       |   A   |    |   A   |    The array_ptr is the address of the array
**       +-------+    +-------+
**       |   B   |    |   B   |    The size_per is 4 for ints	   
**       +-------+    +-------+
**       |   C   |    |   0   |    The array_len is 5
**       +-------+    +-------+
**       |   D   |    |   C   |    The open_offset is 2
**       +-------+    +-------+
**       |   E   |    |   D   |    Data E is lost after the open
**       +-------+    +-------+
*/

void StructOpenArray (array_ptr, size_per, array_len, open_offset)

  char * array_ptr;
  int size_per, array_len, open_offset;

{
/*
**  Local data declarations
*/
    char * open_address;
    int    copy_length;


/*
**  The open_address will be used as the start location of the copy and
**  as the start address for the clear operation.
*/
    open_address = &array_ptr [size_per * open_offset];


/*
**  The copy length is the length of the array minus the offset minus one
**  times the size of an entry.  Note that if the array is 4 units long
**  and the open is taking place at the last entry (offset 3), then the
**  copy length is 4 - 3 - 1 which is zero.
*/
    copy_length = size_per * (array_len - open_offset - 1);


/*
**  Copy the bytes if there are any to copy
*/
#ifdef SUN
    /*
    **	The Sun C RTL does not support the memmove function so use bcopy instead.
    **	bcopy on the Sun is safe for overlapping memory.
    */
    if (copy_length > 0) bcopy (open_address, (char *) open_address + size_per, copy_length);
#else
    if (copy_length > 0) memmove (open_address + size_per, open_address, copy_length);
#endif


/*
**  Set the hole to all zeros.
*/
    memset (open_address, 0, size_per);
}

/*
**  Here is the SVN private StructCloseArray routine.  Given the address of an
**  array, the size of an array entry, and the used length of the array, it 
**  will close a hole in the array at the provided offset.
**
**  Example:  Array of 5 integers.  Want to close offset 2.
**
**       +-------+    +-------+
**       |   A   |    |   A   |    The array_ptr is the address of the array
**       +-------+    +-------+
**       |   B   |    |   B   |    The size_per is 4 for ints	   
**       +-------+    +-------+
**       |   C   |    |   D   |    The array_len is 5
**       +-------+    +-------+
**       |   D   |    |   E   |    The close_offset is 2
**       +-------+    +-------+
**       |   E   |    |   0   |    Data C is lost after the open
**       +-------+    +-------+
**                                 The end of the array is cleared
*/

void StructCloseArray (array_ptr, size_per, array_len, close_offset)

  char * array_ptr;
  int size_per, array_len, close_offset;

{
/*
**  Local data declarations
*/
    char * close_address;
    int copy_length;


/*
**  The close_address will be used as the destination of the copy.
*/
    close_address = &array_ptr [size_per * close_offset];


/*
**  The copy length is the length of the array minus the offset minus one
**  times the size of an entry.  Note that if the array is 4 units long
**  and the close is taking place at the last entry (offset 3), then the
**  copy length is 4 - 3 - 1 which is zero.
*/
    copy_length = size_per * (array_len - close_offset - 1);


/*
**  Copy the bytes.
*/
#ifdef SUN
    /*
    **	The Sun C RTL does not support the memmove function so use bcopy instead.
    **	bcopy on the Sun is safe for overlapping memory.
    */
    if (copy_length > 0) bcopy ( (char *) close_address + size_per, close_address, copy_length);
#else
    if (copy_length > 0) memmove (close_address, close_address + size_per, copy_length);
#endif


/*
**  Clear the last entry in the array
*/
    memset (&array_ptr [size_per * (array_len - 1)], 0, size_per);
}

/*
**  This routine inserts a new component into each entry which already has a
**  component structure defined.  Due to the number of entries that may be 
**  affected, this procedure must be as tight as possible.
**
**  Since each entry points to the next entry, we will not even concern 
**  ourselves with which entry we are on.  We must keep count in case we
**  have to call invalidate entry routine.
*/

#ifdef _NO_PROTO
void DXmSvnInsertComponent (w, comp_number, width, tag)

  Widget w;
  int comp_number;
  Dimension width;
  XtPointer tag;
#else
void DXmSvnInsertComponent (
  Widget w,
  int comp_number,
  Dimension width,
  XtPointer tag)
#endif
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int entry_number;


/*
**  Make it a bit more convenient
*/
    int comp_offset = comp_number - 1;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
#endif


/*
**  Begin by having the first entry be in the entry variable.
*/
    svnentry = svnw->svn.entryPtr;
    entry_number = 1;


/*
**  While the entry is not null, then do that entry if the component structure
**  is already allocated.
*/
    while (svnentry != (DXmSvnEntryPtr) NULL)

      {
       /*
       **  If we have a component structure, then it may need expanded.
       */
           if (svnentry->num_allocated != 0) 

              {
               /*
               **  Make sure that this newly inserted entry does not force holes in
               **  the entries.
               */
		   if (svnentry->num_allocated < comp_offset)
                      XtError(SVN_INSERTHOLE);


	       /*
               **  If the entry is valid, then invalidate it with the high level call.
               */
                   if (svnentry->valid)
                      DXmSvnInvalidateEntry ((Widget) svnw, entry_number);

  
               /*
               **  Extend the memory holding the components if necessary.  We 
	       **  know that that there are at least num_components entries 
	       **  already.  If we need more space, we will reallocate the 
	       **  entrycompPtr space.
               */
                   if (svnentry->num_allocated == svnentry->num_components)
		      {
			DXmSvnCompPtr tempcompPtr = (DXmSvnCompPtr) XtCalloc (svnentry->num_allocated + 1, (sizeof(DXmSvnCompStruct)));
			memcpy (tempcompPtr,svnentry->entrycompPtr,svnentry->num_allocated * sizeof(DXmSvnCompStruct));
	                XtFree ((char *)svnentry->entrycompPtr);
			svnentry->num_allocated++;
	                svnentry->entrycompPtr = tempcompPtr;
		      };


	       /*
               **  Move all entries out to insert a hole.
               */
                   StructOpenArray (svnentry->entrycompPtr, sizeof(DXmSvnCompStruct), svnentry->num_allocated, comp_offset);


	       /*
               **  Commit the new number of components in this entry
               */
	           svnentry->num_components++;
               };


       /*
       **  Position to the next entry
       */
	   svnentry = svnentry->next;
           entry_number++;
      };


/*
**  Push out the entries in the column_widths and column_tags array.
*/
    StructOpenArray (svnw->svn.column_widths,    sizeof(Dimension),    max_comps, comp_offset);
    StructOpenArray (svnw->svn.column_tags,      sizeof(XtPointer), max_comps, comp_offset);
    StructOpenArray (svnw->svn.column_width_set, sizeof(Boolean),      max_comps, comp_offset);


/*
**  Put in the new values.
*/
    svnw->svn.column_widths [comp_offset] = width;
    svnw->svn.column_tags   [comp_offset] = tag;


/*
**  Set the boolean if the width was set by application
*/
    if (width == 0)
         svnw->svn.column_width_set [comp_offset] = FALSE;
    else svnw->svn.column_width_set [comp_offset] = TRUE;


/*
**  Add the new width plus a spacing into the maximum width (only if the width is
**  not zero.  If it is zero, then adjustments will fix the width.
*/
    if (width != 0)
       svnw->svn.secondary_max_width = svnw->svn.secondary_max_width + svnw->svn.default_spacing + width;
}

/*
**  This routine removes a component from each entry which already has a
**  component structure defined.  Due to the number of entries that may be 
**  affected, this procedure must be as tight as possible.
*/

void DXmSvnRemoveComponent (w, comp_number)

  Widget w;
  int comp_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int entry_number;
    int comp_offset = comp_number - 1;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
#endif


/*
**  Begin by having the first entry be in the entry variable.
*/
    svnentry = svnw->svn.entryPtr;
    entry_number = 1;


/*
**  While the entry is not null, then do that entry if the component structure
**  is already allocated.
*/
    while (svnentry != (DXmSvnEntryPtr) NULL)
      {
       /*
       **  If we have a component structure, then we can squeeze out this 
       **  undesired child...
       */
           if (svnentry->num_allocated != 0) 

              {
               /*
               **  Make sure that this entry has that component number.
               */
		   if (svnentry->num_allocated < comp_number)
                      XtError(SVN_REMOVEMISS);

	       /*
               **  If the entry is valid, then invalidate it with the high 
	       **  level call.  It could be that this component's removal 
	       **  would change the height of the entry.
               */
                   if (svnentry->valid)
		      DisplaySvnInvalidateEntry (svnw, entry_number);

	       /*
	       **  Get rid of the component data before we squish it.
	       */
                   if (svnentry->entrycompPtr[comp_offset].type == DXmSvnKcompText)
		      XmStringFree(svnentry->entrycompPtr[comp_offset].var.is_text.text);

	       /*
               **  Move all entries up unless this is the last component.
               */
                   StructCloseArray (svnentry->entrycompPtr, sizeof(DXmSvnCompStruct), svnentry->num_allocated, comp_offset);

	       /*
               **  Commit the new number of components in this entry
               */
	           svnentry->num_components--;
               };


       /*
       **  Position to the next entry
       */
	   svnentry = svnentry->next;
           entry_number++;
      };


/*
**  Subtract this column plus spacing from the width if it is not zero...
*/
    if (svnw->svn.column_widths [comp_offset] != 0)
       {
        svnw->svn.secondary_max_width = svnw->svn.secondary_max_width - svnw->svn.default_spacing;
        svnw->svn.secondary_max_width = svnw->svn.secondary_max_width - svnw->svn.column_widths [comp_offset];
       };

/*
**  Squeeze out the column width and column tag
*/
    StructCloseArray (svnw->svn.column_widths,    sizeof(Dimension),    max_comps, comp_offset);
    StructCloseArray (svnw->svn.column_tags,      sizeof(unsigned int), max_comps, comp_offset);
    StructCloseArray (svnw->svn.column_width_set, sizeof(Boolean),      max_comps, comp_offset);
}

/*
**  This routine sets a new component width.  It must force all entries to be
**  readjusted in height and redrawn on the screen.
*/

#ifdef _NO_PROTO
void DXmSvnSetComponentWidth (w, comp_number, width)

  Widget w;
  int comp_number;
  Dimension width;
#else
void DXmSvnSetComponentWidth (
  Widget w,
  int comp_number,
  Dimension width)
#endif
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Make it a bit more convenient
*/
    int comp_offset = comp_number - 1;


/*
**  Debug diagnostic
*/
#ifdef SVNDEBUG
    if (svnw->svn.disabled_count == 0)
	XtError(SVN_NOTDIS);
#endif


/*
**  Set the boolean if the width is being set by application
*/
    if (width == 0)
         svnw->svn.column_width_set [comp_offset] = FALSE;
    else svnw->svn.column_width_set [comp_offset] = TRUE;


/*
**  If the column width is not changing, then leave early.
*/
    if (svnw->svn.column_widths [comp_offset] == width) return;


/*
**  Ensure that all entries are redrawn when the widget is reenabled
*/
    XClearArea (XtDisplay(svnw),
	    XtWindow(svnw->svn.secondary_window_widget), 
	    0, 0, 
	    XtWidth(svnw->svn.secondary_window_widget),
	    XtHeight(svnw->svn.secondary_window_widget), 
	    TRUE);


/*
**  Subtract this column plus spacing from the width if it is not zero...
*/
    if (svnw->svn.column_widths [comp_offset] != 0)
       {
        svnw->svn.secondary_max_width = svnw->svn.secondary_max_width - svnw->svn.default_spacing;
        svnw->svn.secondary_max_width = svnw->svn.secondary_max_width - svnw->svn.column_widths [comp_offset];
       };


/*
**  Put in the new value.
*/
    svnw->svn.column_widths [comp_offset] = width;


/*
**  Add the new width plus a spacing into the maximum width (only if the width is
**  not zero.  If it is zero, then adjustments will fix the width.
*/
    if (width != 0)
       svnw->svn.secondary_max_width = svnw->svn.secondary_max_width + svnw->svn.default_spacing + width;
}

/*
**  This routine gets a component width.
*/

Dimension DXmSvnGetComponentWidth (w, comp_number)

  Widget w;
  int comp_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Return the value.
*/
    return svnw->svn.column_widths [comp_number - 1];
}

/*
**  This routine sets a new component tag.
*/

void DXmSvnSetComponentTag (w, comp_number, tag)

  Widget w;
  int comp_number;
  XtPointer tag;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Put in the new value.
*/
    svnw->svn.column_tags [comp_number - 1] = tag;
}

/*
**  This routine gets a component tag.
*/

XtPointer DXmSvnGetComponentTag (w, comp_number)

  Widget w;
  int comp_number;

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Return the value.
*/
    return svnw->svn.column_tags [comp_number - 1];
}

/*
**  Here is the DXmSvnGetComponentNumber which finds a component number given
**  a component tag value.  Zero means no hit.
*/

#ifdef _NO_PROTO
int DXmSvnGetComponentNumber (w, comp_tag)

  Widget w;
  XtPointer comp_tag;
#else
int DXmSvnGetComponentNumber (
  Widget w,
  XtPointer comp_tag)
#endif
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data
*/
    int comp_number;


/*
**  Go through the list of comp tags.  If a hit is found, return the number.
*/
    for (comp_number = 1;  comp_number <= max_comps;  comp_number++)
	if (svnw->svn.column_tags [comp_number-1] == comp_tag)
	   return comp_number;


/*
**  No hit...
*/
    return 0;
}

/*
** Here is the SVN private StructSvnAddEntries routine
*/

void StructSvnAddEntries (svnw, after_entry, number_of_entries, level, entry_tags, index_window)

  svn_widget svnw;
  int after_entry, number_of_entries, level;
  XtPointer * entry_tags;
  int index_window;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr NewEntry, AfterEntry;
    int i;


/*
**  The level arrays must be larger than the deepest level in the tree.  When
**  allocating the actual space, we add one because C arrays are zero-based.
**  The arrays are indexed [0..max_level].
*/
    if (level >= svnw->svn.max_level) {
	svnw->svn.max_level = level + 1;
	svnw->svn.levelx = (LevelPtr) XtRealloc((char *)svnw->svn.levelx, (svnw->svn.max_level + 1) * sizeof(int));
 	svnw->svn.levely = (LevelPtr) XtRealloc((char *)svnw->svn.levely, (svnw->svn.max_level + 1) * sizeof(int));

	svnw->svn.path_entries = (short *) XtRealloc((char *)svnw->svn.path_entries, (svnw->svn.max_level + 1) * sizeof(short));
	};


/*
**  If entries are being added to an empty structure, then initialize the 
**  cache to be NULL and zeros.  This alleviates the need to initialize this
**  value from outside of this module.
*/
    if (svnw->svn.num_entries == 0)
       {
         svnw->svn.cache_number  = 0;
         svnw->svn.cache_pointer = (DXmSvnEntryPtr) NULL;
       };


/*
**  Get the entry after which these will be placed.
*/
    if (after_entry == 0) 
         AfterEntry = (DXmSvnEntryPtr) NULL;
    else AfterEntry = StructGetEntryPtr (svnw, after_entry);


/*
**  If entries are being added before the cache entry, then we will back the
**  cache back to that entry.
*/
    if (svnw->svn.cache_number != 0)
       if (svnw->svn.cache_number > after_entry)
          {
            svnw->svn.cache_number  = after_entry;
            svnw->svn.cache_pointer = AfterEntry;
          };


/*
**  Allocate memory, create the required number of entries  
*/
    for (i = 1;  i <= number_of_entries;  i++)
	{
          /*
          **  Get the new entry
          */
              NewEntry = (DXmSvnEntryPtr) XtCalloc (1, (sizeof(DXmSvnEntryStruct)));


          /*
          **  Set the known values
          */
              NewEntry->index_window = index_window;
              NewEntry->level = level;


          /*
          **  Set the optional values
          */
              if (entry_tags != (XtPointer *) NULL) 
		NewEntry->entry_tag = entry_tags [i-1];


          /*
          **  Link it into the list and bump the pointer
          */
              LclLinkAddEntry(svnw, AfterEntry, NewEntry);
              AfterEntry = NewEntry;
	};


/*
**  If index_window is true, then set index_window_needed
*/
    if (index_window) svnw->svn.index_window_needed = TRUE;
}

/*
** Here is the SVN private StructSvnDeleteEntries routine
*/

void StructSvnDeleteEntries (svnw, after_entry, number_of_entries)

  svn_widget svnw;
  int after_entry, number_of_entries;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr CurrentEntry;
    DXmSvnEntryPtr NextEntry;
    int i;


/*
**  Get the first entry that is going away.
*/	
    CurrentEntry = StructGetEntryPtr (svnw, after_entry+1);


/*
**  If the after entry is less than the cache pointer, then back the entry
**  pointer down to that entry...
*/
    if (after_entry < svnw->svn.cache_number)
         {
           svnw->svn.cache_number  = after_entry;
           svnw->svn.cache_pointer = CurrentEntry->prev;
         };


/*
**  Loop through the entries...
*/
    for (i = 1;  i <= number_of_entries;  i++)
        {
          NextEntry = LclLinkDeleteEntry (svnw, CurrentEntry);
          LclDeallocateComponent (CurrentEntry);
          XtFree ((char *)CurrentEntry);
          CurrentEntry = NextEntry; 
        };
}

/*
** Here is the SVN private StructSvnInvalidateEntry routine
*/

void StructSvnInvalidateEntry (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Set the invalid bits in the record.   
*/
    svnentry->valid = FALSE;
}

/*
** Here is the SVN private StructGetEntryPtr routine
*/

DXmSvnEntryPtr StructGetEntryPtr (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr current_entry = (DXmSvnEntryPtr) NULL;
    int current_number;


/*
**  Return entry 1 very fast...
*/
    if (entry_number == 1) return svnw->svn.entryPtr;


/*
**  If the desired entry is beyond the cache, then start at the cache...
*/
    if (entry_number > svnw->svn.cache_number)
       {
         current_number = svnw->svn.cache_number;
         current_entry  = svnw->svn.cache_pointer;
       };


/*
**  If the desired entry is closer to the cache than to the front of the list,
**  then also start at the cache...
*/
    if (entry_number < svnw->svn.cache_number)
       if (entry_number >= (svnw->svn.cache_number - entry_number))
          {
            current_number = svnw->svn.cache_number;
            current_entry  = svnw->svn.cache_pointer;
          };


/*
**  If the current entry pointer is still NULL, then start at the beginning...
*/
    if (current_entry == (DXmSvnEntryPtr) NULL)
       {
         current_number = 1;
         current_entry  = svnw->svn.entryPtr;
       };


/*
**  See if we should go forward to the desired entry.
*/
    if (entry_number > current_number)
       while (entry_number != current_number)
          {
            current_number++;
            current_entry = current_entry->next;
          };


/*
**  Well, see if we should go backward...
*/
    if (entry_number < current_number)
       while (entry_number != current_number)
          {
            current_number--;
            current_entry = current_entry->prev;
          };
        

/*
**  Return the current entry pointer...
*/
    return current_entry;
}

/*
**  Here is the local routine that looks for an entry number using the cache...
**
**  This routine deals with the cache being advanced only if the entry that it
**  is currently in the cache is valid...
*/

static DXmSvnEntryPtr LclStructGetCachePtr (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  If the entry number being asked for is less than the cache value, then go
**  to the normal non-cache get entry routine...  This is because we know that
**  if we are not in forced mode, they are all valid...
*/
    if (entry_number < svnw->svn.cache_number)
       return StructGetEntryPtr (svnw, entry_number);


/*
**  If the cache value is zero, then set it to the first entry.
*/
    if (svnw->svn.cache_number == 0)
       {
         svnw->svn.cache_number  = 1;
         svnw->svn.cache_pointer = svnw->svn.entryPtr;
       };


/*
**  See if we should go forward to the desired entry...  We must ensure that
**  the entry in the cache is valid prior to advancing the pointer...  Note
**  only when we are forced into sequential entry mode...
*/
    if (entry_number > svnw->svn.cache_number)
       while (entry_number != svnw->svn.cache_number)
          {
           /*
           **  If we are forcing sequential get entry callbacks, then make sure
           **  that the entry is valid.
           */
               if ((svnw->svn.cache_pointer->valid == FALSE) && (svnw->svn.force_seq_get_entry))
                  {
                   /*
                   **  Local data declarations
                   */
                       DXmSvnCallbackStruct temp;

                   /*
                   **  Fill in the callback information
                   */
		       temp.reason       = DXmSvnCRGetEntry;
		       temp.entry_number = svnw->svn.cache_number;
		       temp.entry_tag    = svnw->svn.cache_pointer->entry_tag;
		       temp.entry_level  = svnw->svn.cache_pointer->level;

                   /*
                   **  Issue the callback
                   */
	               XtCallCallbacks ((Widget)svnw, DXmSvnNgetEntryCallback, &temp);
                  };


           /*
           **  Bump the cache pointer to the next entry...
           */
               svnw->svn.cache_number++;
               svnw->svn.cache_pointer = svnw->svn.cache_pointer->next;
          };


/*
**  Return the entry pointer that is in the cache.
*/
    return svnw->svn.cache_pointer;
}

/*
** Here is the SVN private StructGetValidEntryPtr routine
*/

DXmSvnEntryPtr StructGetValidEntryPtr (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int num_entries;
    DXmSvnCallbackStruct temp;


/*
**  Spin through looking for the entry
*/
    svnentry = LclStructGetCachePtr (svnw, entry_number);


/*
**  If the entry is not valid, then we must stay here until we get it.
*/
    if (!svnentry->valid)

       {
        /*
        **  Disable the widget
        */
            DXmSvnDisableDisplay ((Widget) svnw);
       

        /*
        **  Spin in a loop until the correct conditions are met.
        */
            while (TRUE)
   
               {
                /*
                **  Record the number of entries currently known
                */
                    num_entries = svnw->svn.num_entries;


                /*
                **  Fill in the callback information
                */
                    temp.reason       = DXmSvnCRGetEntry;
                    temp.entry_number = entry_number;
                    temp.entry_tag    = svnentry->entry_tag;
                    temp.entry_level  = svnentry->level;

         
                /*
                **  Issue the callback
                */
                    XtCallCallbacks ((Widget)svnw, DXmSvnNgetEntryCallback, &temp);


                /*
                **  If the number of entries has changed, then find this entry
                **  number again.
                */
                    if (svnw->svn.num_entries != num_entries)
                       svnentry = StructGetEntryPtr (svnw, entry_number);
                       

                /*
                **  If the entry is valid, then break out of the loop
                */
                    if (svnentry->valid) break;

            };


        /*
        **  Enable the widget
        */
            DXmSvnEnableDisplay ((Widget) svnw);
       };


/*
**  If the height has not yet been adjusted, then do that before returning.
*/
    if (!svnentry->height_adjusted)
       DisplayAdjustHeight (svnw, svnentry);


/*
**  Return the entry to the caller
*/
    return svnentry;
}

/*
** Here is the local routine that will add the entry to the proper position
** within the linked list.
*/
void LclLinkAddEntry (svnw, AfterEntry, NewEntry)

  svn_widget svnw;
  DXmSvnEntryPtr AfterEntry, NewEntry;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr NextEntry;


/*
**  If AfterEntry is NULL, then insert this entry into the beginning of the list.
*/
    if (AfterEntry == (DXmSvnEntryPtr) NULL) 
       {
         if (svnw->svn.entryPtr == (DXmSvnEntryPtr) NULL)
              {
                svnw->svn.entryPtr = NewEntry;
                return;
              }
         else {
                NewEntry->next = svnw->svn.entryPtr;
                svnw->svn.entryPtr->prev = NewEntry;
                svnw->svn.entryPtr = NewEntry;
                return;
              };
       };
 
 
/*
**  If AfterEntry is at the end of the list, then insert this at the end.
*/
    if (AfterEntry->next == (DXmSvnEntryPtr) NULL)
       {
         AfterEntry->next = NewEntry;
         NewEntry->prev = AfterEntry;
         return;
       };


/*
**  Insert this node in between two nodes
*/
    NextEntry = AfterEntry->next;
    NewEntry->prev = AfterEntry;
    NewEntry->next = AfterEntry->next;
    AfterEntry->next = NewEntry;
    NextEntry->prev = NewEntry;
}

/*
** Here is the local routine that will delete the entry from the linked list.
*/

DXmSvnEntryPtr LclLinkDeleteEntry (svnw, EntryPtr)

  svn_widget svnw;
  DXmSvnEntryPtr EntryPtr;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr NextEntry;
    DXmSvnEntryPtr PrevEntry;


/*
**  If this entry is the first in the linked list, then update the head pointer
**  to point to this nodes next pointer (which may be null).  If the next guy
**  is not NULL, then have his previous pointer be NULL so that he knows that
**  he is at the beginning of the list.
*/
    if (EntryPtr->prev == (DXmSvnEntryPtr) NULL)
       {
         svnw->svn.entryPtr = EntryPtr->next;
         NextEntry = EntryPtr->next;
         if (EntryPtr->next != (DXmSvnEntryPtr) NULL) NextEntry->prev = (DXmSvnEntryPtr) NULL;
         return NextEntry;
       };


/*
**  Now we know it's not at the head of the list.  See if it is the last entry
**  in the list.
*/
    if (EntryPtr->next == (DXmSvnEntryPtr) NULL)
       {
         PrevEntry = EntryPtr->prev;
         PrevEntry->next = (DXmSvnEntryPtr) NULL;
         return ((DXmSvnEntryPtr) NULL);
       };


/*
**  Now we know it must be in the middle of the list.
*/
    PrevEntry = EntryPtr->prev;
    NextEntry = EntryPtr->next;
    PrevEntry->next = NextEntry;
    NextEntry->prev = PrevEntry;
    return NextEntry;
}

/*
** Here is the local routine that will deallocate memory for component array.
*/

static void LclDeallocateComponent (svnentry)

  DXmSvnEntryPtr svnentry;

{
/*
**  Local data declarations
*/
    int i;


/*
**  If there are no components allocated, then return
*/
    if (svnentry->num_allocated == 0) return;


/*
**  Go through each component deallocating any memory allocated within it.
*/
    for (i = 0;  i <= svnentry->num_components - 1;  i++) {
       if (svnentry->entrycompPtr[i].type == DXmSvnKcompText)
	  XmStringFree(svnentry->entrycompPtr[i].var.is_text.text);
	}

/*
**  Deallocate the component itself.
*/
    XtFree ((char *)svnentry->entrycompPtr);


/*
**  Show that it is gone
*/
    svnentry->num_components = 0;
    svnentry->num_allocated  = 0;
}

/*
**  Here is the local routine that will check the component array to see
**  if all the components for the entry have been set.  If the number of
**  components in the array equals the total number of components for the
**  entry, entry->valid will be set to TRUE.
*/

static void LclDetermineValidity (svnentry)

  DXmSvnEntryPtr svnentry;

{
/*
**  Local data declarations
*/
    int i;


/*
**  It may be already valid?
*/
    if (svnentry->valid) return;


/*
**  If there are no components, return.
*/
    if (svnentry->num_components == 0) return;


/*
**  For each component,  if it is not set, then return.
*/
    for (i = 0;  i <= svnentry->num_components - 1;  i++)
       if (svnentry->entrycompPtr[i].type == DXmSvnKcompNotSet)
          return;


/*
**  All of the components have been set.  Set the entry valid.
*/
    svnentry->valid = TRUE;
} 

/*
**  This procedure walks up from the current widget looking for the SVN class
**  type of widget.
*/

svn_widget StructFindSvnWidget (w)
 
    Widget w;

{
/*
**  Local data declarations
*/
    WidgetClass w_class;


/*
** First make sure the widget isn't null, which means we walked up the
** parent tree and found no SVN widget. Just return NULL, otherwise toolkit
** will choke.
*/
    if (w == NULL)
	return (svn_widget) NULL;
/*
**  If this is an SVN widget, then return it.  
*/
    if (XtIsSubclass((Widget)w, (WidgetClass)dxmSvnWidgetClass))
	return (svn_widget) w;

/*
**  Otherwise call ourself looking at the parent of this widget.
*/
    return (StructFindSvnWidget (XtParent(w)));
}

/*
**  Calculate the offset of widget w from the SVN widget.
**  Widget w must be in the SVN widget tree.
*/
void StructTranslateCoords(w, x, y)
    Widget      w;                                  /* widget to calculate offsets for */
    Position    *x;                                 /* X offset */
    Position    *y;                                 /* Y offset */
{
    Position tmpx, tmpy;
    svn_widget svnw = StructFindSvnWidget (w);

    if (w == NULL) return;

    /*
    **  Check for NULL x or y
    */
    if (x == NULL) x = &tmpx;
    if (y == NULL) y = &tmpy;

    *x = 0;
    *y = 0;

    while (!XtIsSubclass((Widget)w, (WidgetClass)dxmSvnWidgetClass))
    {
        /*
        **  Add x offset and border width for this widget
        */
        *x += w->core.x + w->core.border_width;
        *y += w->core.y + w->core.border_width;

        w = XtParent(w);
    }

    /* 
     * If we have r->l layout, we have to take into the x calculation the size of 
     * the vertical scrollbar. 
     */
    if (LayoutIsRtoL(svnw))
	*x = *x - button_height + 3;

    /*
    **  Add the border width of the SVN widget
    */
    *x += w->core.border_width;
    *y += w->core.border_width;
}

