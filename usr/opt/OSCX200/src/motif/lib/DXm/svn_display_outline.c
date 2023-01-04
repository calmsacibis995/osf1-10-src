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
#define LS_PRINTF    
#define SYNC_ON_SCROLL 
#define LS_EVENT
*/
/*
*=======================================================================
*
*                COPYRIGHT (c) 1988, 1989, 1991, 1992 BY
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
/*    MODIFICATION HISTORY:						      */
/*									      */
/*	059	JS			13-Oct-1993			      */
/*		Fix RtoL when in column mode, OutlineMapPosition was 	      */
/*		incorrectly offsetting the x-coord causing clicks on	      */
/*		the primary window to be ignored.			      */
/*	058	AN			31-Mar-1993			      */
/*		Made the routine LclOutlineHideWidgets, non-static, so that   */
/*		SVN_DISPLAY module could access it.			      */
/*	057	CS			10-Feb-1993			      */
/*		Fix arguments to DisplayEraseLocationCursor.		      */
/*	056	AN			03-Nov-1992			      */
/*		Change the pixel placement for drawing entries, highlighting  */
/*		and location cursor.					      */
/*	055	An			27-Oct-1992			      */
/*		Fixed bug in LclOutlineScrollButton routine, where if there   */
/*		was only one entry in the display, an ACCVIO would occur.     */
/*	054	CS			11-Aug-1992			      */
/*		Fix OutlineMapPosition and LclOutlineFindOffset to work	      */
/*		properly with the new widget hierarchy.  Fix a bug in 	      */
/*		LclOutlinePositionJump that was causing the display_count to  */
/*		be wrong.						      */
/*	053	CS			 2-Apr-1992			      */
/*		Fix bug in OutlineDeleteEntries for case where after_entry is */
/*		zero.  Have to set display_count = num_path = 0.	      */
/*	052	CS			20-Mar-1992			      */
/*		Fix bug in OutlineDeleteEntries calling StructGetEntryPtr     */
/*		with svn.entries[svnw->svn.display_count] when display_count  */
/*		equals zero.						      */
/*	051	CS			19-Mar-1992			      */
/*		Remove hack in LclOutlineScrollAdjust for num_entries < 5.    */
/*		Fix bug in LclOutlineScrollUp calling LclOutlineDragLS with   */
/*		a zero value argument.					      */
/*	050	CS			16-Mar-1992			      */
/*		Change OutlineResize so svn.display_count > svn.num_path.     */
/*		Fix bug in LclOutlineScrollAdjust setting slider size < 1.    */
/*	049	CS			12-Feb-1992			      */
/*		Change OutlineDeleteEntries so it sets the display_count      */
/*		and num_path fields properly and resizes the PTR window.      */
/*	048	AN			22-Jan-1992			      */
/*		Add check around call to routine LclOutlineEraseLocationCursor*/
/*		to make sure location cursor is not NULL.		      */
/*	047	CS			 9-Jan-1992			      */
/*		Fix bugs in LclOutlinePositionJump.  Made the entries array   */
/*		correct before changing the geometry of the ptr window.	      */
/*		Changed OutlineVScroll for XmCR_TO_TOP and XmCR_TO_BOTTOM so  */
/*		it positions entry 1 at top and svn.num_entries at bottom.    */
/*		Fix off-by-1 bug in LclOutlineIntersects.		      */
/*	046	AN			 8-Jan-1992			      */
/*		Fixed bug where sometimes location cursor would not be erased */
/*		when the SVN window would lose focus. Also fixed a bug in     */
/*		OutlineVScroll where the scroll->value was not always being   */
/*		used to calculate scroll-to-bottom, scroll-to-top.	      */
/*	045	CS			 3-Jan-1992			      */
/*		When managing or resizing the PTR window, unmanage the	      */
/*		subwidgets first.					      */
/*	044	CS			 2-Jan-1992			      */
/*		Changed LclOutlineScrollUp and LclOutlineScrollDown to call   */
/*		LclOutlineDragLS.  Made some changes in scrolling widgets     */
/*	043	CS			12-Dec-1991			      */
/*		Fix LclOutlineScrollAdjust for the case when svn.num_entries  */
/*		equals zero.						      */
/*	042	AN		         5-Dec-1991			      */
/*		Take out section of code in routine LclOutlineDragLS where    */
/*		a quick return was taken for a special case, this broke       */
/*		live scrolling all entries that are all level 0.	      */
/*	041	CS			 2-Dec-1991			      */
/*		Reset internal values in LclOutlineDragLS when taking early   */
/*		return.  Made a few changes to LclOutlinePositionJump around  */
/*		clearing the display.					      */
/*	040	CS			26-Nov-1991			      */
/*		Made changes for NorthWest gravity.			      */
/*	039	CS			22-Nov-1991			      */
/*		Fixed problems with LclOutlineScrollDown and		      */
/*		LclOutlineAdjustDisplayLS for scrolling up.		      */
/*	038	CS			21-Nov-1991			      */
/*		Modified LclOutlineResizePTR to calculate the height of the   */
/*		the PTR window.  Fixed several problems with LclOutlineScrollUp*/
/*		and LclOutlineScrollDown.  Changed LclOutlineEnable to only   */
/*		call LclOutlineScrollAdjust if grop_pending == FALSE	      */
/*	037	CS			14-Nov-1991			      */
/*		Made changes for live scrolling.  Changed LclOutlineManagePTR,*/
/*		LclOutlineUnmanagePTR, LclOutlineDragLS,		      */
/*		LclOutlineAdjustDisplayLS				      */
/*	036	AN			25-Oct-1991			      */
/*		Redesigned PTR window, so that it is now a separate window    */
/*		This included changes to ALL drawing code, live-scrolling     */
/*		code and traversal code.  Also reworked some drawing routines */
/*		for efficieny and performance.				      */
/*	035	CS			27-Sep-1991			      */
/*		Fixed OutlinePositionDisplay for the case of scrolling down   */
/*		by a page.  If the bottom entry was partially obstructed by   */
/*		the horizontal scroll bar the incorrect entry was placed at   */
/*		the top.  Fixed OutlineVScroll for a scroll reason of	      */
/*		XmCR_VALUE_CHANGED.  Now an entry selected using the index    */
/*		window is always being visible after the scrollbar drag       */
/*		operation completes.  Changed XtDestroyWidget to              */
/*		XtUnmapWidget for the index window.			      */
/*	034	AN			14-May-1991			      */
/*		Fixed bug in LclOutlineAdjustDisplayLS where if num_path      */
/*		== 0 and trying to scroll down, then text would get elongated.*/
/*		, it was a off by 1 pixel problem.			      */
/*	033	AN			03-Apr-1991			      */
/*		Fix bug in LclOutlineScrollAdjust that would give XmScrollBar */
/*		warnings sometimes...					      */
/*	032	AN			28-Mar-1991			      */
/*		Add support for live scrolling with widget components.        */
/*	031	AN			20-Mar-1991			      */
/*		Add changes for bug fix, so that when mode is R->L, and tree  */
/*		display mode, the nav. button does not overwrite the bottom   */
/*		scrollbar.						      */
/*	030	AN			19-Mar-1991			      */
/*		Added watch cursor to operations on outer scrollbar buttons.  */
/*		Also put check in OutlineScrollHelp routine so that if	      */
/*		scroll event parameter == NULL, it would use CurrentTime.     */
/*	029	AN			14-Mar-1991			      */
/*		Fixed a few bugs in live scrolling, with path-to-root.	      */
/*	028	AN		        27-Feb-1991			      */
/*		Bug fix in live scrolling code for Non-pathtoroot.	      */
/*	027	AN			26-Feb-1991			      */
/*		Add support in live scrolling for path-to-root line.	      */
/*      026	AN			25-Feb-1991			      */
/*		Add support for live scrolling in SVN			      */
/*	025	AN			28-Jan-1990			      */
/*		Make routines LclOutlineScrollButton to non-static so that    */
/*		SVN.C module could reference it.			      */
/*	024	AN			11-Dec-1990			      */
/*		Change look of location cursor so it is now a solid line with */
/*		the color of highlight_color... and the highlight feature of  */
/*		SVN is now a dashed line.				      */
/*	023	AN			20-Nov-1990			      */
/*		Made routine LclOutlineScrollDown non-static.		      */
/*	022	AN			05-Oct-1990			      */
/*		More support for keyboard traversal.			      */
/*	021	AN			27-Sep-1990			      */
/*		Add support for loc. cursor in tree mode.		      */
/*	020	AN			20-Sep-1990			      */
/*		Added support for location cursor for new Motif style	      */
/*		guide mouse semantics.					      */
/*      019     S. Lavigne              23-Aug-1990                           */
/*              Integrate the DEC Israel changes into this module - their     */
/*              code is turned on by the macro LayoutIsRtoL.                  */
/*	018	AN			20-Aug-1990			      */
/*		Add code for keyboard traversal and location cursor drawing.  */
/*	017	AN			11-Jul-1990			      */
/*		Change names of variables and anything external to be	      */
/*		internationalized for sides or pane windows.  "rhs"->secondary*/
/*		and "lhs" -> primary.					      */
/*	016	AN			28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	015	AN			25-Jun-1990			      */
/*		Changes made so that only compound strings are supported      */
/*		instead of text strings and compound strings.		      */
/*	014	AN			12-Jun-1990			      */
/*		Fix bug in routine LclOutlineVScrollDrag, is wasn't copying   */
/*		the cs string for the index window, and later doing a free.   */
/*	013	AN			07-Jun-1990			      */
/*		Fix bug in OutlineEnableDisplay where lhs_window entries      */
/*		where being cleared and never redrawn properly.		      */
/*	012	AN			22-May-1990			      */
/*		Fix problem where if lhs-window was resized to less than 2    */
/*		pixels and a drag was preformed on an entry that was	      */
/*		selected, X errors would occur. Fix was placed in	      */
/*		OutlineCreateGhost routine.				      */
/*	011	S. Lavigne 		18-May-1990			      */
/*		Fix several accvios in routines LclOutlineScrollButton and    */
/*		LclOutlineScrollAdjust when user has resized window and the   */
/*		display count is 0.                                           */
/*	010	AN			17-May-1990			      */
/*		Fix misc. refresh problems with column separators.	      */
/*		Also change LclOutlineChangeBaseX routine so that it unhigh-  */
/*		lights any entries before it does the XCopyArea to perform    */
/*		a horizontal scroll.					      */
/*	009	AN			09-Apr-1990			      */
/*		Fix routine LclOutlineVScrollDrag so that it will allow       */
/*		compound strings to be seen in index window also... not just  */
/*		text.							      */
/*	008	AN			27-Mar-1990			      */
/*		Change declaration of varible in LclOutlineScrollUp routine   */
/*		from Dimension to Position.. because routine expects it to    */
/*		hold signed value.					      */
/*	007	S. Lavigne 		01-Feb-1990			      */
/*		Add #pragma standard lines to compile cleanly with /STAN=PORT.*/
/*      006	Will Walker		26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	005	Will Walker		25-Jan-1990			      */
/*		Change DECwWsSvn*.h to DECwMSvn*.h.			      */
/*	004	S. Lavigne 		23-Jan-1990			      */
/*		Change all BCSESVN routine references to SVN.  Change all     */
/*		SvnWidget typedef references to svn_widget.  Change the	      */
/*		naming style of the constants from Svn* to SvnK*.  	      */
/*      003     S. Lavigne              19-Jan-1990                           */
/*              Modify routine LclOutlineVScrollDrag to set the resource      */
/*		XmNrecomputeSize to false on XmCreateLabel when creating the  */
/*		index window so the widget doesn't resize itself each time to */
/*		hold the label text string.  This default changed in Motif.   */
/*      002     S. Lavigne              12-Jan-1990                           */
/*              Make some additional post-converter adjustments to work	      */
/*		under Motif (get rid of the Developer Decisions and Motif     */
/*		Transition).  Fix up the include files, callback reasons,     */
/*		and compound string handling.  Free compound strings using    */
/*		XmStringFree instead of XtFree.				      */
/*	001	S. Lavigne 		05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/

#define  SVN_DISPLAY_OUTLINE

#ifdef VMS
/*#pragma nostandard*/
#include "XmP.h"
/*#pragma standard*/
#else
#include <Xm/XmP.h>
#endif

#include "DXmSvnP.h"
#include "svnprivate.h"


/*
**  Local routine declarations
*/
static void LclOutlineDrawEntry         ();  /*  Draw an entry             */
static void LclOutlineDrawEntryTop      ();
void LclOutlineClearEOS          ();  /*  Clear the end of screen   */
    void LclOutlineDrawColumnLines   ();  /*  Draw column lines again   */
static int  LclOutlineFindOffset        ();  /*  Finds offset given x, y   */
    int  LclOutlineMapNumberToOffset ();  /*  Maps number to offset     */
    int  LclOutlineMapOffsetToY      ();  /*  Maps offset into a Y      */
    int  LclOutlineEntryAppend       ();  /*  Appends entry at bottom   */
    int  LclOutlineEntryAppendLS     ();  /*  Appends entry at bottom   */
       void OutlineUnmanageSecPTR();	/* Unmanages Path-to-root window */
       void OutlineManageSecPTR();	/* Manages the Path-to-root window */
static void LclOutlineUnmanagePTR();	/* Unmanages Path-to-root window */
static void LclOutlineManagePTR();	/* Manages the Path-to-root window */
static void LclOutlineResizePTR();	/* Resizes the Path-to-root window */
static void LclOutlineForceExpose();	/* Forces dispatching of Expose events */
static void LclOutlineMoveWidgets();	/* Moves a set of widgets */
	void LclOutlineHideWidgets();	/* Hides a set of widgets */
#ifdef _NO_PROTO
extern void DisplayEraseLocationCursor ();
#else
extern void DisplayEraseLocationCursor (svn_widget svnw);
#endif

/*
**  Helper routines.
*/
static Boolean LclOutlineIntersects ();


/*
**  Local positioning routines
*/
    void     LclOutlinePositionJump        ();
    int      LclOutlineAtBottom            ();
static Position LclOutlineGetPositionGivenTop ();


/*
**  Forward routine declarations for scrollbar callbacks
*/
static int  LclOutlineScrollUpCandidate ();
    void LclOutlineScrollUp          ();
static void LclOutlineScrollDown        ();
static void LclOutlineChangeBaseX       ();
static void LclOutlineVScrollDrag       ();
static void LclOutlineDragLS();
       void LclOutlineAdjustDisplayLS();
       void LclOutlineScrollAdjust      ();
       void svn_sync();

/*
**  Scroll button action routines
*/
       void LclOutlineScrollButton      ();
static void LclOutlineScrollTimeout     ();


/*
** Here is the OutlineMapPosition public entry point using the C language 
** interface.  This routine must determine the entry number and component
** number that are being displayed at the given x,y coordinate.  We will 
** assume that this coordinate is relative to the SVN window.
*/

void OutlineMapPosition (svnw, findx, findy, entry_number, component_number, entry_tag)

  svn_widget svnw;
  int findx, findy;
  int *entry_number;
  int *component_number;
  XtPointer *entry_tag;

{
    /*
    **  Cast the widget passed into an SvnWidget data structure.
    */
    DXmSvnEntryPtr svnentry;
    int dummy_arg, offset, i, left_x, right_x, spacing, primary_window_width;
    XtPointer dummy_unsigned_arg;
    Position dst_x, dst_y;

    /*
    **  Set up the arguments so that everything looks like it's passed.
    */
    if (component_number == (int *) NULL)
	component_number = &dummy_arg;
    if (entry_tag == NULL)
	entry_tag = &dummy_unsigned_arg;

    *component_number = 0;
    *entry_number = 0;
    *entry_tag = NULL;

    /*
    **  Find out at what display offset maps into this x and y position.
    */
    offset = LclOutlineFindOffset (svnw, findx, findy);

    /*
    **  If it's not found, then get out of here now...
    */
    if (offset == 0)
	return;

    /*
    **  Give the application the entry number
    */
    *entry_number = svnw->svn.entries[offset];

    /*
    **  Get the svn entry structure...
    */
    svnentry = StructGetEntryPtr (svnw, *entry_number);

    /*
    **  Give the caller the tag...
    */
    *entry_tag = svnentry->entry_tag;

    /*
    **  Save the level number
    */
    svnw->svn.map_level = svnentry->level;

    /*
    **  Get the coordinates of the primary window relative to the SVN window
    */
    StructTranslateCoords(svnw->svn.primary_window_widget, &dst_x, &dst_y);

    /*
    ** If we have r->l layout, we have to add into the width calculation 
    ** the size of the vertical scrollbar, since it appears on the left.
    */
    primary_window_width = XtWidth(svnw->svn.primary_window_widget);
    if (LayoutIsRtoL(svnw))
        primary_window_width = primary_window_width + button_height - 3;

    /*
    **	Check if findx is in the primary window widget
    */
    if (findx >= dst_x && findx < dst_x + primary_window_width)
    {
	int x1, x2, w1;
	int max_primary_comp = svnentry->num_components;	    /* maximum component on the left hand side */

	/*
	**  See if there are less.
	*/
	if ((svnw->svn.display_mode == DXmSvnKdisplayColumns) || svnw->svn.secondary_components_unmapped)
	   if (svnw->svn.start_column_component != 0)
	      max_primary_comp = svnw->svn.start_column_component - 1;

        /*
        **  If max_primary_comp is greater than the actual number...
        */
	if (max_primary_comp > svnentry->num_components)
	    max_primary_comp = svnentry->num_components;

	/*
	**  Compute the base x value based on the entries level number and horizontal scroll position
	*/
	left_x = (int) svnw->svn.margin_width + ((int) svnentry->level * (int) svnw->svn.indent_margin)
		- (int) svnw->svn.window_basex;

	/*
	**  Normalize findx for this window
	*/
	findx -= dst_x;

	/*
	**  Loop through components 1 to max_primary_comp - 1.  This loop assumes that the component width has been set. 
	**  This is safe because we know that the entry is being displayed.
	*/
	for (i = 1; i < max_primary_comp; i++)
	{
	    x1 = (int) svnentry->entrycompPtr[i-1].x;	    /* X offset of current component */
	    w1 = (int) svnentry->entrycompPtr[i-1].width;   /* width of current component */
	    x2 = (int) svnentry->entrycompPtr[i].x;	    /* X offset of next component */
	    spacing = (x2 - (x1 + w1))/2;		    /* half of space between this component and next */
	    right_x = left_x + x1 + w1 + spacing;
	    if (findx < right_x)
		break;
	}
	*component_number = i;
    }
    else if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
    {
	/*
	**  Get the coordinates of the secondary window relative to the SVN window
	*/
	StructTranslateCoords(svnw->svn.secondary_window_widget, &dst_x, &dst_y);

	/*
	**  Check if the X coordinate is within the secondary window
	*/
	if (findx >= dst_x && findx < dst_x + XtWidth (svnw->svn.secondary_window_widget))
	{
	    /*
	    **  If this entry has right hand side components, then see which component it was...  
	    */
	    if (svnentry->num_components >= svnw->svn.start_column_component && svnw->svn.start_column_component != 0)
	    {
		/*
		**  Compute the base x value based on the scrolled window.
		*/
		left_x = (svnw->svn.default_spacing / 2) - svnw->svn.secondary_base_x;

		/*
		**  Normalize the findx value to this window
		*/
		findx -= dst_x;

		spacing = (int) svnw->svn.default_spacing;
		for (i = svnw->svn.start_column_component; i < svnentry->num_components; i++)
		{
		    right_x = left_x + (int) svnw->svn.column_widths[i-1] + (spacing / 2);
		    if (findx < right_x)
			break;

		    left_x = left_x + (int) svnw->svn.column_widths[i-1] + spacing;
		}
		*component_number = i;
	    }
	}
    }

    if (*component_number == 0)
    {
	/*
	**  Could not find the component so clear the entry number and tag
	*/
	*entry_number = 0;
	*entry_tag = 0;
    }
}

/*
**  This local routine returns the Y coordinate of "entry_number" if "top_entry_number" were
**  to be placed as the first entry under the path to the root.  The y coordinate returned
**  is relative to the main window.
*/
static Position LclOutlineGetPositionGivenTop (svnw, top_entry_number, entry_number)

  svn_widget svnw;
  int top_entry_number;
  int entry_number;

{
/*
**  Local data declarations
*/
    int i;
    int num_path,ptr_height,y_position = 0, top_position;
    DXmSvnEntryPtr svnentry;
    int level;

/*
**  Get at the entry for the top_entry_number.  It needs to be valid so that
**  we can get at both the height and the level number.  
*/  
    svnentry = StructGetValidEntryPtr (svnw, top_entry_number);

/*
**  Initialize the y position to include the first entry under the path to the
**  root .  Don't add in the top entry it is the same as the target entry.
*/
    if (entry_number == top_entry_number)
	 top_position = y_position = 0;
    else top_position = y_position = svnentry->height;


/*
**  Figure out the path to the root.  The looping variable reflects which 
**  entry number we are looking at.  The level variable reflects the level
**  number that we are looking for.  
**
**  Based purely on the level number, we can now set the number of entries in 
**  the path to the root.
*/
    if (svnw->svn.show_path_to_root)
         num_path = svnentry->level;
    else num_path = 0;

    if (num_path != 0)
       {
         level = svnentry->level - 1;
         for (i = top_entry_number - 1;  i > 0;  i--)
            {
              svnentry = StructGetValidEntryPtr (svnw, i);
              if (svnentry->level == level)
                 {
	           y_position += svnentry->height;
                   if (level == 0) break;
                   level--;
                 };
            };
       };

    /* Figure out the height of the PTR, subtract off the top_entry_number if added in */

    ptr_height = y_position - top_position;

    /*
    **  Add all remaining entries to y_position excluding the top_entry specified
    **  since it has been added in before we figured out the path-to-root.
    */
    for (i = top_entry_number+1;  i < entry_number;  i++)
        {
          svnentry = StructGetValidEntryPtr (svnw, i);
          y_position += svnentry->height;
        };

    /******************************************************************************/
    /*  Return the y position.							  */
    /*                                                                            */
    /* If the top_entry_number is already in the position under the new PTR       */
    /* then we want to return the Y position of the next entry, so that we	  */
    /* scroll there instead.							  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.entries[num_path+1] == top_entry_number) ||
        (svnw->svn.num_path == 0))
	return (y_position);
    else
	return (y_position - ptr_height);
}


/*
**  This routine is the high level interface to DXmSvnPositionDisplay.  This
**  routine can assume that the widget is disabled.
*/

int OutlinePositionDisplay (svnw, entry_number, position)

  svn_widget svnw;
  int entry_number, position;

{
/*
**  Local data declarations
*/
    int new_top_entry;
    Position new_position;
    DXmSvnEntryPtr svnentry;


/*
**  Take care of our only possible failing condition
*/
    if (position == DXmSvnKpositionNextPage)
       if (LclOutlineAtBottom (svnw))
          return FALSE;


/*
**  Disable the SVN widget
*/
    DXmSvnDisableDisplay ((Widget)svnw);


    /*
    **  Remap DXmSvnKpositionPreviousPage and DXmSvnKpositionNextPage
    */
    switch (position)
    {
	case DXmSvnKpositionPreviousPage:		/* Turn previous page into a position bottom call */
	    position = DXmSvnKpositionBottom;
	    entry_number = svnw->svn.entries [svnw->svn.num_path + 1];
	    break;
	case DXmSvnKpositionNextPage:			/* Turn next page into a position top call */
	    position = DXmSvnKpositionTop;
	    entry_number = svnw->svn.entries [svnw->svn.display_count];
	    break;
    }


/*
**  Get the valid entry so that we can use the height field.
*/
    svnentry = StructGetValidEntryPtr (svnw, entry_number);


/*
**  Default the passed entry to be the top entry under the path to the root.
*/
    new_top_entry = entry_number;


    /*
    **  Handle the case where the last entry is obstructed by the horizontal scroll bar.
    */
    switch (position)
    {
	case DXmSvnKpositionTop:			    /* desired location is at the top */
	    {
	    int y;

	    /*
	    **  Get the position of this entry.
	    */
	    y = LclOutlineMapNumberToOffset (svnw, entry_number);   /* get offset */
	    y = LclOutlineMapOffsetToY (svnw, y);		    /* get y coordinate */

	    /*
	    **  If "y + height" of the entry is below the horizontal scroll bar then we have
	    **  a hanging entry at the bottom.  In this case use the previous entry as the new top entry.
	    */
	    if (y + svnentry->height > PRIMARY_HEIGHT(svnw))
		new_top_entry--;
	    }
	    break;
 
	case DXmSvnKpositionMiddle:			    /* desired location is in the middle */
	    while (new_top_entry != 1)
	    {
		/*
		**  Get this entries position based on the new_top_entry
		*/
		new_position = LclOutlineGetPositionGivenTop (svnw, new_top_entry, entry_number);

		/*
		**  If our entry is hanging off of the bottom, then backoff 1 if we had moved the top entry.
		**  Otherwise, we haven't much choice about the entry hanging over...
		*/
		if (new_position + svnentry->height > PRIMARY_HEIGHT(svnw))
		{
		    if (new_top_entry != entry_number) new_top_entry++;
			break;
		}

		/*
		**  If we are past the middle, then we are done...
		*/
		if (new_position + svnentry->height > (PRIMARY_HEIGHT(svnw) / 2))
		    break;

		/*
		**  Decrement the top entry and try again...
		*/
		new_top_entry--;
	    }
	    break;

	case DXmSvnKpositionBottom:			    /* desired location is at the bottom */
	    while (new_top_entry != 0)
	    {
		/*
		**  Get this entries position based on the new_top_entry
		*/
		new_position = LclOutlineGetPositionGivenTop (svnw, new_top_entry, entry_number);

		/*
		**  If our entry is hanging off of the bottom, then backoff 1 if we had moved the top entry.
		**  Otherwise, we haven't much choice about the entry hanging over...
		*/
		if (new_position + svnentry->height > PRIMARY_HEIGHT(svnw))
		{
		    if (new_top_entry != entry_number) new_top_entry++;
			break;
		}

		/*
		**  Decrement the top entry and try again...
		*/
		new_top_entry--;
	    }
	    break;

    }	/* switch */

/*
**  If the new top entry is zero, then make it a one...
*/
    if (new_top_entry == 0)
       new_top_entry = 1;

    svnw->svn.scroll_value = svnw->svn.current_value = svnw->svn.internal_value = new_top_entry; 

/*
**  If this was a position bottom call, then we must get out of this routine
**  before attempting to resolve any whitespace problems.  Commit to this new
**  display and leave.
*/
    if (position == DXmSvnKpositionBottom)
       {
         LclOutlinePositionJump (svnw, new_top_entry);
         DXmSvnEnableDisplay ((Widget)svnw);
         return TRUE;
       };


/*
**  Now we must determine if the entries that are going to be appended will
**  fill the screen.  The way we will do this is to validate entries after
**  the target entry until we go past the bottom.  If we run out of entries
**  before passing the bottom, we will again call ourselves to position the
**  last entry at the bottom.
**
**  Optimization:  The following expensive loop is trying to see if the screen
**                 needs pulled down.  This is only applicable if the new top
**                 entry is within the max number of display elements away from
**                 the end of the list.  This is a good optimization for products
**		   having more than max_display entries.
**
**		   This code approximates the number of entries able to be 
**		   displayed.
*/
    if (new_top_entry + (PRIMARY_HEIGHT(svnw) / svnentry->height) > svnw->svn.num_entries)
       {
	new_position = LclOutlineGetPositionGivenTop (svnw, new_top_entry, entry_number);
	new_position = new_position + svnentry->height;

	while (new_position < PRIMARY_HEIGHT(svnw))
	   {
	     if (entry_number == svnw->svn.num_entries)
		{
		  OutlinePositionDisplay (svnw, svnw->svn.num_entries, DXmSvnKpositionBottom);
		  DXmSvnEnableDisplay ((Widget)svnw);
		  return TRUE;
		};
	     entry_number++;
	     svnentry = StructGetValidEntryPtr (svnw, entry_number);
	     new_position = new_position + svnentry->height;
	   };
       };


/*
**  We fell out of the loop only if the screen filled up.  Commit the new 
**  screen layout.
*/
    LclOutlinePositionJump (svnw, new_top_entry);

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif

    DXmSvnEnableDisplay ((Widget)svnw);

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif

    return TRUE;

}

/*
**  This routine is called to map a particular rectangle into a list of 
**  selections that intersect that rectangle.  The calling program is 
**  responsible for allocating the array into which the intersecting entry
**  numbers are placed.  The return value of this procedure is the number of 
**  entries entered into the array.
**
**  The X and Y coordinates are relative to the window in svnw->svn.clips_window.
*/

int LclOutlineMapRectangle (svnw, entry_numbers, clip_window, low_x, low_y, high_x, high_y)

  svn_widget svnw;
  int * entry_numbers;
  Window clip_window;
  int low_x, low_y, high_x, high_y;

{
/*
**  Local data declarations
*/
    int i, count = 0;
    int start_entry = 0, end_entry = 0;

/*
**  For each entry that is on the screen;  compute the x, y, width, and height
**  of that entry.  If this entry intersects the box in any way, then add it
**  the list of selections.  We will check intersection by examining the four
**  corners of the entry.
*/

    if (clip_window == XtWindow(svnw->svn.primary_window_widget) ||
	clip_window == XtWindow(svnw->svn.secondary_window_widget))
    {
	/*
	** Main window.  Only check entries outside of path-to-root window.
	*/
	start_entry = svnw->svn.num_path + 1;
	end_entry = svnw->svn.display_count;
    }
    else if (clip_window == XtWindow(svnw->svn.primary_ptr_widget) ||
	     clip_window == XtWindow(svnw->svn.secondary_ptr_widget))
    {
	/*
	** Path-To-Root window.  Only check entries in the path-to-root window.
	*/
	start_entry = 1;
	end_entry = svnw->svn.num_path;
    }

    for (i = start_entry;  i <= end_entry;  i++)
        if (LclOutlineIntersects (svnw, i, low_x, low_y, high_x, high_y) == TRUE)
           {
             entry_numbers[count] = svnw->svn.entries[i];  
             count++;
           };


/*
**  Return the number of selections
*/
    return count;
}

/*
**  This routine is called when the user is testing to see if an autoscroll
**  should occur.  This routine is passed an X and Y coordinate and must
**  decide whether or not to autoscroll the screen or not.
*/

int OutlineAutoScrollCheck (svnw, y)

  svn_widget svnw;
  int y;

{
/*
**  If they are in the path to root section, then return true.
*/
    if (y < XtHeight(svnw->svn.primary_ptr_widget))
       if ((svnw->svn.num_path != 0) || (svnw->svn.entries[1] != 1))
          return 1;


/*
**  If they in the horizontal scroll bar and there are entries not being
**  shown, then scroll up
*/
    if ((y ) > FORM_HEIGHT(svnw))
       if (LclOutlineAtBottom (svnw) == FALSE)
	  if (LclOutlineScrollUpCandidate(svnw) > 0)
	     return 1;


/*
**  Nope
*/
    return 0;
}
 
/*
**  This routine is called when the user is dragging a set of entries or selecting
**  a range of entries.  This routine is passed an X and Y coordinate and must
**  decide whether or not to autoscroll the screen or not.  If it scrolls the
**  screen, it updates the box_base_y so that the box will be draw correctly 
**  now that the screen positions have changed.
*/

void OutlineAutoScrollDisplay (svnw, y)

  svn_widget svnw;
  int y;

{
/*
**  Local data declarations
*/
    int entry_number, offset, going, old_y, new_y;


/*
**  If they are above the path to the root line and there is either a path to
**  the root or there are entries not being shown.
*/
    if (y < XtHeight(svnw->svn.primary_ptr_widget))
       if ((svnw->svn.num_path != 0) || (svnw->svn.entries[1] != 1))
	  {
           /*
           **  If we do not have a range hook set, then just scroll without 
           **  worrying about a pixel count.
           */
               if (svnw->svn.range_hook == 0)
	          {
                    LclOutlineScrollDown (svnw);
                    LclOutlineScrollAdjust (svnw);
		    return;
                  };

           /*
           **  We do have a range_hook set 
           */
               entry_number = svnw->svn.range_hook;
               offset = LclOutlineMapNumberToOffset (svnw, entry_number);
               if ((offset == 0) || (offset > svnw->svn.num_path + 1))
                  {
                    entry_number = svnw->svn.entries[svnw->svn.num_path + 1];
                    offset = LclOutlineMapNumberToOffset (svnw, entry_number);
                  };
               old_y  = LclOutlineMapOffsetToY (svnw, offset);
               LclOutlineScrollDown (svnw);
               offset = LclOutlineMapNumberToOffset (svnw, entry_number);
               new_y  = LclOutlineMapOffsetToY (svnw, offset);
	       svnw->svn.box_base_y = svnw->svn.box_base_y + (new_y - old_y);
               LclOutlineScrollAdjust (svnw);
          };


/*
**  If they in the horizontal scroll bar, then scroll up.  
*/
    if (( y ) > FORM_HEIGHT(svnw))
       {
        /*
        **  If we do not have a range hook set, then just scroll without 
        **  worrying about a pixel count.
        */
            if (svnw->svn.range_hook == 0)
               {
                LclOutlineScrollUp (svnw);
                LclOutlineScrollAdjust (svnw);
                return;
              };

        /*
        **  We do have a range_hook set.  If the range_hook is not present or if
        **  the range hook is going away, then we will track the last entry on 
        **  the screen.  Otherwise, we will track the range hook itself.
        */
            entry_number = svnw->svn.range_hook;
            offset = LclOutlineMapNumberToOffset (svnw, entry_number);
	    going  = LclOutlineScrollUpCandidate (svnw);
            if ((offset == 0) || (offset == going))
               {
                 entry_number = svnw->svn.entries[svnw->svn.display_count];
                 offset = LclOutlineMapNumberToOffset (svnw, entry_number);
               };
            old_y  = LclOutlineMapOffsetToY (svnw, offset);
            LclOutlineScrollUp (svnw);
            offset = LclOutlineMapNumberToOffset (svnw, entry_number);
            new_y  = LclOutlineMapOffsetToY (svnw, offset);
	    svnw->svn.box_base_y = svnw->svn.box_base_y + (new_y - old_y);
            LclOutlineScrollAdjust (svnw);
       };
}

/*
**  This routine is called to create the ghost image to be used in a dragging
**  operation.  It is assumed that the ghost_x and ghost_y have been 
**  set to the initial position in the screen from which this ghost will
**  appear attached.  These values are passed in relative to the screen itself.
**  This routine will set the ghost_basex and ghost_basey values to be relative
**  to the origin of the pixmap.
**
**  If an entry number is passed into this routine, then we only want to
**  create a ghost that is the size of that entry and ignore all others.
*/
void OutlineCreateGhost (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    Screen  *screen  = XtScreen (svnw);
    Display *display = DisplayOfScreen (screen);
    int i, min_x = 99999, max_x = 0, min_y = 99999, max_y = 0;
    int low_y=0, y=0;



/******************************************************************************/
/*                                                                            */
/* Determine the rectangle size to be used in this ghost.		      */
/* If an entry number was passed into this routine, that signals that the     */
/* entry was NOT selected and we only want to create a ghost that size is     */
/* equal to the size of that entry number. Otherwise, continue as always      */
/* and create a ghost that will enclose ALL selected entries.		      */
/*                                                                            */
/******************************************************************************/
    if (entry_number != 0)
	{
	for (i = 1;  i <= svnw->svn.display_count;  i++)
	    {
	    DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
	    if (svnw->svn.entries[i] == entry_number)
		{
		int low_x  = svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin);
		int high_x = low_x + svnentry->width;
		int high_y = low_y + svnentry->height;

		if (min_x > low_x)  min_x = low_x;
		if (min_y > low_y)  min_y = low_y;
		if (max_x < high_x) max_x = high_x;
		if (max_y < high_y) max_y = high_y;
		break;
		}
	    low_y = low_y + svnentry->height;
	    }
	}
    else
	for (i = 1;  i <= svnw->svn.display_count;  i++)
	  {
          DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
          if (svnentry->selected)
             {
               int low_x  = svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin);
               int high_x = low_x + svnentry->width;
               int high_y = low_y + svnentry->height;

               if (min_x > low_x)  min_x = low_x;
               if (min_y > low_y)  min_y = low_y;
               if (max_x < high_x) max_x = high_x;
               if (max_y < high_y) max_y = high_y;
             };
          low_y = low_y + svnentry->height;
          };

/******************************************************************************/
/*                                                                            */
/* If these are fixed width entries, then override the high x value.  If the  */
/* width of the primary window is so small that the max_x will become negative... */
/* make sure we handle that case.					      */
/*                                                                            */
/******************************************************************************/
    if (svnw->svn.fixed_width)
	{
	if ((svnw->svn.display_mode == DXmSvnKdisplayColumns) &&
	    (XtWidth (svnw->svn.primary_window_widget) <= 2))
	      max_x = XtWidth (svnw->svn.secondary_window_widget) + 1;
        else
	      max_x = XtWidth (svnw->svn.primary_window_widget) - 1;
	}
	

/*
**  We can now set the relative cursor holding point in the pixmap
*/
    if (LayoutIsRtoL(svnw))
        svnw->svn.ghost_basex  = XtWidth(svnw) - svnw->svn.ghost_x - min_x;
    else
        svnw->svn.ghost_basex  = svnw->svn.ghost_x - min_x;
    svnw->svn.ghost_basey  = svnw->svn.ghost_y - min_y;
    svnw->svn.ghost_width  = max_x - min_x + 1;
    svnw->svn.ghost_height = max_y - min_y + 1;


/*
**  Create a pixmap of the appropriate size
*/
    svnw->svn.ghost = XCreatePixmap ( display, RootWindowOfScreen(screen), svnw->svn.ghost_width, 
                                      svnw->svn.ghost_height, (unsigned) DefaultDepthOfScreen (screen));


/*
**  Clear the pixmap
*/
    XFillRectangle (XtDisplay(svnw),svnw->svn.ghost,svnw->svn.foreground_gc,0,0,svnw->svn.ghost_width,svnw->svn.ghost_height);


/*
**  For each entry on the screen.  If it is currently selected then draw into
**  the pixmap using the normal graphic context.
*/
    if (entry_number != 0)
	{
	for (i = 1;  i <= svnw->svn.display_count ;  i++)
	    {
	    DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
	    if (svnw->svn.entries[i] == entry_number)
		{
		int x  = (svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin)) - min_x;
		int adjust_y  = y - min_y;
		int width  = svnentry->width;
        
		if (svnw->svn.fixed_width) width = svnw->svn.ghost_width - x - 2;
		if (LayoutIsRtoL(svnw))
                   x = 0;
		XDrawRectangle (XtDisplay(svnw), svnw->svn.ghost, svnw->svn.background_gc, 
			   (Position)x, (Position)adjust_y, 
                           (Dimension)width, (Dimension)svnentry->height);
		}
	    y = y + svnentry->height;
	    }
	}
    else	
	for (i = 1;  i <= svnw->svn.display_count;  i++)
	    {
	    DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
	    if (svnentry->selected)
		{
                int x  = (svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin)) - min_x;
                int adjust_y  = y - min_y;
                int width  = svnentry->width;
                if (svnw->svn.fixed_width) width = svnw->svn.ghost_width - x - 2;
                if (LayoutIsRtoL(svnw))
                   x = 0;
                XDrawRectangle (XtDisplay(svnw), svnw->svn.ghost, svnw->svn.background_gc, 
			       (Position)x, (Position)adjust_y, 
                               (Dimension)width, (Dimension)svnentry->height);
                };
            y = y + svnentry->height;
            };
}

/*
**  This routine is called to delete the ghost image used in a dragging operation.
*/

void OutlineDeleteGhost (svnw)

  svn_widget svnw;

{
/*
**  Delete the pixmap if it exists
*/
    if (svnw->svn.ghost != 0)
       XFreePixmap (XtDisplay(svnw), svnw->svn.ghost);


/*
**  Show that it is gone
*/
    svnw->svn.ghost = 0;
}

/*
**  This routine is called from SVN.C to do the display portion of the source 
**  module invalidating an entry.  
*/

void OutlineInvalidateEntry (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local routine data
*/
    DXmSvnEntryPtr svnentry = (DXmSvnEntryPtr) NULL;
    int i;


/*
**  Go through the list of displayed entries looking for this entry.
**  If the entry is found, then zero out the display_ptr which will
**  instruct the LclDraw routine to refresh this entry.
*/
    int offset = LclOutlineMapNumberToOffset (svnw, entry_number);

    if (offset != 0)
       {
         svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[offset]);
	 if (svnw->svn.display_invalid[offset] == 0)
	    {
	     svnw->svn.display_invalid[offset] = svnentry->height;
             if (svnw->svn.sub_widgets_used)
                for (i = 0;  i <= svnentry->num_components-1;  i++)
                   if (svnentry->entrycompPtr[i].type == DXmSvnKcompWidget)
		      HIDE_WIDGET (svnentry->entrycompPtr[i].var.is_widget.readwrite_text);
            };
         svnw->svn.display_changed = TRUE;
       };


/*
**  Get the entry structure for this entry number
*/
    if (svnentry == (DXmSvnEntryPtr) NULL)
       svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Show that it will need readjusted
*/
    svnentry->height_adjusted = FALSE;
}

/*
**  Specific screen routine to get the number being displayed.
*/
int OutlineGetNumDisplayed (svnw)

  svn_widget svnw;

{
/*
**  Return to the caller the current number of entries being displayed.
*/
    return svnw->svn.display_count;
}

/*
**  Display specific routine to get the list of displayed entries.
*/

void OutlineGetDisplayed (svnw, entry_nums, entry_tags, y_coords, num_array_entries)

  svn_widget svnw;
  int *entry_nums;
  XtPointer *entry_tags;
  int *y_coords;
  int num_array_entries;

{
/*
**  Local data
*/
    int i, number_to_copy;
    int y = 0;


/*
**  Figure out the number of entries to copy based on the length of the array
**  and the number of entries that are displayed.
*/
    if (svnw->svn.display_count > num_array_entries)
         number_to_copy = num_array_entries;
    else number_to_copy = svnw->svn.display_count;
 

/*
**  Loop through the entries being displayed.
*/
    for (i = 1;  i <= number_to_copy;  i++)
        {
          DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);

	  if (entry_nums != (int *) NULL) entry_nums [i-1] = svnw->svn.entries[i];
          if (entry_tags != (XtPointer *) NULL) entry_tags [i-1] = svnentry->entry_tag;
          if (y_coords   != (int *) NULL) y_coords   [i-1] = y;
   
          y = y + svnentry->height;
        };


/*  
**  Set any remaining array elements to null
*/
    for (i = number_to_copy + 1;  i < num_array_entries;  i++)
        {
	  if (entry_nums != (int *) NULL) entry_nums [i-1] = 0;
          if (entry_tags != (XtPointer *) NULL) entry_tags [i-1] = 0;
          if (y_coords   != (int *) NULL) y_coords   [i-1] = 0;
        };
}

/*
**  This routine is called whenever the application or source module has called 
**  the DXmSvnSetEntrySensitivity high level call which is located in SVN.C.
*/

void OutlineSetEntrySensitivity (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Go through the list of displayed entries looking for this entry.
*/
    int offset = LclOutlineMapNumberToOffset (svnw, entry_number);


/*
**  If the entry is found, then zero out the display_ptr which will
**  instruct the LclDraw routine to refresh this entry.
*/
    if (offset != 0)
       {
         DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[offset]);
	 if (svnw->svn.display_invalid[offset] == 0)
	     svnw->svn.display_invalid[offset] = svnentry->height;
         svnw->svn.display_changed = TRUE;
       };
}

/*
**  This routine is called indirectly from the DXmSvnAddEntries routine
**  located in SVN.C.  
*/

void OutlineAddEntries (svnw, after_entry, number_added)

  svn_widget svnw;
  int after_entry, number_added;

{
/*
**  Local data declarations
*/
    int i, changed_offset;


/*
**  If the entry being added after is at or beyond the end of the display, then leave.  
*/
    if (after_entry >= svnw->svn.entries[svnw->svn.display_count])
       {
         svnw->svn.display_changed = TRUE;
         return;
       };


/*
**  If there is a display portion and the entry is in that portion of the screen...
*/
    if (svnw->svn.display_count > svnw->svn.num_path)
       if (after_entry >= svnw->svn.entries[svnw->svn.num_path+1])
          {
            changed_offset = after_entry - svnw->svn.entries[svnw->svn.num_path+1];
            changed_offset = changed_offset + svnw->svn.num_path + 2;
            LclOutlineClearEOS (svnw, changed_offset);
            return;
          };


/*
**  If the entry being added after is in view (in the path), then that entry
**  that is being expanded will be made into the top entry after the path and
**  the list will be redisplayed from that entry down.
*/
    for (i = 1;  i <= svnw->svn.num_path;  i++)
      if (after_entry == svnw->svn.entries[i])
         {
           LclOutlineClearEOS (svnw, i + 1);
           return;
         };


/*
**  We are now in the position where the entry that had entries added to it is
**  somewhere hidden in the path to the root.  We will again not change the
**  display, but instead increment the entry numbers from that point down.
*/
    for (i = 1;  i <= svnw->svn.display_count;  i++)
       if (svnw->svn.entries[i] > after_entry)
          svnw->svn.entries[i] = svnw->svn.entries[i] + number_added;


/*
**  We must set display changed to true here in order to update the scrollbars.
*/
    svnw->svn.display_changed = TRUE;
}

/*
**  This routine is called indirectly from the DXmSvnDeleteEntries routine
**  located in SVN.C.  Its job is to mark the entries that will need 
**  refreshed, when the widget is re-enabled.
*/

void OutlineDeleteEntries (svnw, after_entry, number_of_entries)

  svn_widget svnw;
  int after_entry, number_of_entries;

{
/*
**  Local data declarations
*/
    int i, changed_offset;


/*
**  If the entry being deleted after is at or beyond the end of the display, 
**  then leave.  
*/
    if (after_entry >= svnw->svn.entries[svnw->svn.display_count])
       {
         svnw->svn.display_changed = TRUE;
         return;
       };


/*
**  If this delete is after entry zero, then clear the whole screen
*/
    if (after_entry == 0)
       {
         LclOutlineClearEOS (svnw, 1);
	 svnw->svn.display_count = svnw->svn.num_path = 0;
	 if (XtIsManaged(svnw->svn.primary_ptr_widget))
	    LclOutlineUnmanagePTR(svnw);
         return;
       };


/*
**  If there is a display portion and the entry is in that portion of the screen...
*/
    if (svnw->svn.display_count > svnw->svn.num_path)
       if (after_entry >= svnw->svn.entries[svnw->svn.num_path+1])
          {
            changed_offset = after_entry - svnw->svn.entries[svnw->svn.num_path+1];
            changed_offset = changed_offset + svnw->svn.num_path + 2;
            LclOutlineClearEOS (svnw, changed_offset);
            return;
          };


/*
**  If the entry being removed after is in view (in the path) AND the first 
**  entry after the path will be going away, then this entry should be made 
**  the top entry after the path and the list redisplayed from that entry down.
*/
    for (i = 1;  i <= svnw->svn.num_path;  i++)
	if (after_entry == svnw->svn.entries[i])
	    if ((after_entry + number_of_entries) >= svnw->svn.entries[svnw->svn.num_path+1])
            {
		DXmSvnEntryPtr svnentry;

		/*
		**  Clear from the entry after this entry.  Set the values for display_count
		**  and num_path and resize the PTR window.
		*/
		LclOutlineClearEOS (svnw, i + 1);
		svnw->svn.display_count = i;
		svnw->svn.num_path = svnw->svn.display_count - 1;

		if (XtIsManaged(svnw->svn.primary_ptr_widget))
		{
		    if (svnw->svn.num_path > 0)
			LclOutlineResizePTR(svnw);
		    else
			LclOutlineUnmanagePTR(svnw);
		}

		/*
		**  Set the display invalid bit so the entry after the PTR is redrawn.
		*/
		svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.display_count]);
		svnw->svn.display_invalid[svnw->svn.display_count] = svnentry->height;
		return;
	    }


/*
**  We now know that the entry being deleted after is somewhere hidden above
**  the screen or somewhere in the path to root area.  What we do know is that
**  it is above the display portion of the screen.
**
**  We now must see if any of the entries in the display will be going away
**  on this delete and clear from that point down.  This is only the case 
**  where the entries after this entry are siblings and not children.
*/
    for (i = 1;  i <= svnw->svn.display_count;  i++)
	if (svnw->svn.entries[i] > after_entry)
	    if (svnw->svn.entries[i] <= after_entry + number_of_entries)
	    {
		DXmSvnEntryPtr svnentry;

		/*
		**  Clear from this entry.  Set the values for display_count and num_path
		**  and resize the PTR window.
		*/
		LclOutlineClearEOS (svnw, i);
		svnw->svn.display_count = i - 1;

		if (svnw->svn.display_count > 0)
		    svnw->svn.num_path = svnw->svn.display_count - 1;
		else
		    svnw->svn.num_path = 0;

		if (XtIsManaged(svnw->svn.primary_ptr_widget))
		{
		    if (svnw->svn.num_path > 0)
			LclOutlineResizePTR(svnw);
		    else
			LclOutlineUnmanagePTR(svnw);
		}

		/*
		**  Set the display invalid bit so the entry after the PTR is redrawn.
		*/
		if (svnw->svn.display_count > 0)
		{
		    svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.display_count]);
		    svnw->svn.display_invalid[svnw->svn.display_count] = svnentry->height;
		}
		return;
	    }


/*
**  We are now in the position where the entry that had entries removed to it is
**  somewhere hidden in the path to the root.  We will again not change the
**  display, but instead decrement the entry numbers from that point down.
*/
    for (i = 1;  i <= svnw->svn.display_count;  i++)
       if (svnw->svn.entries[i] > after_entry)
          svnw->svn.entries[i] = svnw->svn.entries[i] - number_of_entries;


/*
**  We must set display changed to true here in order to update the scrollbars.
*/
    svnw->svn.display_changed = TRUE;
}

/*
**  This routine is called indirectly from the DXmSvnEnableDisplay routine
**  located in SVN.C.  Its job is to get the screen up to date based on calls
**  that were made during the disabled state.
*/

void OutlineEnableDisplay (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnCallbackStruct temp;
    int num_entries;
    register int i, y;
    DXmSvnEntryPtr svnentry;
    Dimension old_height;
    Widget w1, w2;

/*
**  If we have nothing to do, then leave.
*/
    if (!svnw->svn.display_changed) return;
    svnw->svn.display_changed = FALSE;


/*
**  If we are not REALIZED, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  Initialize variables
*/
    num_entries = svnw->svn.num_entries;
    y = 0;

    if (XtIsManaged(svnw->svn.primary_ptr_widget))
    {
	w1 = svnw->svn.primary_ptr_widget;
	w2 = svnw->svn.secondary_ptr_widget;
    }
    else
    {
	w1 = svnw->svn.primary_window_widget;
	w2 = svnw->svn.secondary_window_widget;
    }


/*
**  Disable the widget in case new entries are added while we are here
*/
    DXmSvnDisableDisplay ((Widget)svnw);

/*
**  Ensure that all of the entries being displayed are valid.  If the 
**  entry is not valid, then that means that either a subwidget has
**  resized or the application has invalidated the entry.  The screen
**  has not yet been changed.
*/

    for (i = 1;  i <= svnw->svn.display_count;  i++)
    {
	/*
	**  Get a valid entry pointer.  This will cause the widget layout
	**  to be recalculated.
	*/
	svnentry = StructGetValidEntryPtr (svnw, svnw->svn.entries[i]);

	/*
	**  If the invalid bit is set...
	*/
	if (svnw->svn.display_invalid[i] != 0)
	{
	    /*
	    **  The old height had been saved...
	    */
	    old_height = svnw->svn.display_invalid[i];

	    if (old_height != svnentry->height)
	    {
		/*
		**  If the height has changed, then the entire rest of the screen is in disarray.
		**  In fact, the number of entries that can be displayed may change.
		*/
		if (i > svnw->svn.num_path + 1)
		    /*
		    **  the entry changing height is below the path to root, clear the entire end of the screen.
		    */
		    LclOutlineClearEOS (svnw, i);
		else
		{
		    /*
		    **  If the entry is in the path to the root then preserve the top entry.
		    */
		    int current_top = svnw->svn.entries[svnw->svn.num_path+1];
		    LclOutlineClearEOS (svnw, 1);
		    LclOutlinePositionJump (svnw, current_top);
		}

		/*
		**  Fall out of the validations because we cleared the screen.
		*/
		break;
	    }

	    svnw->svn.display_invalid[i] = 0;
		
	    /******************************************************************************/
	    /*                                                                            */
	    /* Clear and redraw the entry in the appropriate window.			  */
	    /*                                                                            */
	    /******************************************************************************/

	    XClearArea (XtDisplay(svnw), 
			XtWindow(w1), 
			0, 
			(Position)y,
			XtWidth (w1),
			svnentry->height,
			FALSE);

	    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
		XClearArea (XtDisplay(svnw), 
			    XtWindow(w2), 
			    0, 
			    (Position)y,
			    XtWidth (w2),
			    svnentry->height,
			    FALSE);
	   
	    LclOutlineDrawEntry (svnw, svnentry, y, w1);
	}

	/*
	**  If this is the last entry in the PTR window, reset variables for the main window.
	*/
	if ((i == svnw->svn.num_path) && (XtIsManaged(svnw->svn.primary_ptr_widget)))
	{
	    y = 0;
	    w1 = svnw->svn.primary_window_widget;
	    w2 = svnw->svn.secondary_window_widget;
	}
	else
	    y = y + svnentry->height;
    }

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif


    /******************************************************************************/
    /*                                                                            */
    /* If this is column mode.. then we want to clear out the display screen	  */
    /* after the last entry displayed.					          */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	    {
	    XClearArea (XtDisplay(svnw), 
			   XtWindow(svnw->svn.secondary_window_widget), 
			   0, 
			   (Position)y, 
		           XtWidth (svnw->svn.secondary_window_widget),
			   XtHeight(svnw->svn.secondary_window_widget), 
			   FALSE);
	    }

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif

/*
**  Now try to append as many entries as possible
*/
    while (svnw->svn.entries[svnw->svn.display_count] < num_entries)
       if (LclOutlineEntryAppend (svnw, svnw->svn.entries[svnw->svn.display_count] + 1) == FALSE)
          break;


#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif

    /* NOTE ---- this section of code MIGHT not be needed */
    if (svnw->svn.num_path == 0 && XtIsManaged(svnw->svn.primary_ptr_widget))
	LclOutlineUnmanagePTR(svnw);

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif

/*
** Draw the column lines if we are in column mode.
*/

    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
       LclOutlineDrawColumnLines(svnw);

/*
**  Readjust the scroll bars only if there have been no changes since we 
**  entered this routine.
*/
    if (!svnw->svn.display_changed && !svnw->svn.grop_pending)
       LclOutlineScrollAdjust (svnw);


/*
**  Re-Enable the widget
*/
    DXmSvnEnableDisplay ((Widget)svnw);

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif


/*
**  Issue the callback for the display having changed.
*/
    temp.reason = DXmSvnCRDisplayChanged;
    XtCallCallbacks ((Widget)svnw, DXmSvnNdisplayChangedCallback, &temp);
}

/*
**  This routine is called when the selection status of an entry has 
**  transitioned from low to high.  It may not be on the screen at this
**  time (Select All).
*/

void OutlineHighlightEntry (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data
*/
    int y;
    DXmSvnEntryPtr svnentry;


/*
**  Get the offset of this entry.  If it is not being displayed then leave.
*/
    int offset = LclOutlineMapNumberToOffset (svnw, entry_number);
    if (offset == 0) return;


/*
**  Get the entry structure
*/
    svnentry = StructGetValidEntryPtr (svnw, svnw->svn.entries[offset]);


/*
**  If the widget is disabled, then mark this entry as needing redisplayed.
**  This will repaint the entry when the widget is re-enabled...
*/
    if (svnw->svn.disabled_count != 0) 
       {
	 if (svnw->svn.display_invalid[offset] == 0)
	     svnw->svn.display_invalid[offset] = svnentry->height;
         svnw->svn.display_changed = TRUE;
         return;
       };


/*
**  If the widget is not disabled, then we may have a valid or an invalid
**  entry.  If its invalid, then let the display routine deal with the
**  problem.  This is because it knows how to deal with entries whose height
**  may change.
*/
    if (svnw->svn.display_invalid[offset] != 0)
       {
         OutlineEnableDisplay (svnw);
         return;
       };


/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  Both drawing sections below need the y position
*/
    y = LclOutlineMapOffsetToY (svnw, offset);



    /******************************************************************************/
    /*                                                                            */
    /* Redraw the entry becoming unselected that is not on the path to the root   */
    /* line.  If this entry is in the path-to-root window, then clear from that   */
    /* PTR window, else clear from the main entry window.			  */
    /*                                                                            */
    /******************************************************************************/
    if ((svnw->svn.show_path_to_root) && (offset <= svnw->svn.num_path))
	{
	XClearArea (XtDisplay(svnw),
		    XtWindow(svnw->svn.primary_ptr_widget),
		    0, 
		    (Position)y, 
		    XtWidth(svnw->svn.primary_ptr_widget),
		    svnentry->height,
		    FALSE);

	if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	    XClearArea (XtDisplay(svnw),
			XtWindow(svnw->svn.secondary_ptr_widget),
			0, 
			(Position)y, 
			XtWidth(svnw->svn.secondary_ptr_widget),
			svnentry->height,
			FALSE);

	LclOutlineDrawEntry (svnw, svnentry, y, svnw->svn.primary_ptr_widget);
	}
    else
	{	
	XClearArea (XtDisplay(svnw),
		    XtWindow(svnw->svn.primary_window_widget),
		    0, 
		    (Position)y, 
		    XtWidth(svnw->svn.primary_window_widget),
		    svnentry->height,
		    FALSE);

	if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	    XClearArea (XtDisplay(svnw),
			XtWindow(svnw->svn.secondary_window_widget),
			0, 
			(Position)y, 
			XtWidth(svnw->svn.secondary_window_widget),
			svnentry->height,
			FALSE);

        LclOutlineDrawEntry (svnw, svnentry, y, svnw->svn.primary_window_widget);
	}


}

/*
**  This routine is called when the application calls the high level equivalent.
**  Its job is to update the vertical scroll bar and to possibly append the
**  given entry number to the display.
*/
void OutlineFlushEntry (svnw, entry_number)

    svn_widget svnw;
    int entry_number;

{
/*
**  Append the entry if appropriate the entry number passed is the next logical
**  entry on the screen.
*/
    if ((svnw->svn.entries[svnw->svn.display_count] + 1) == entry_number)
         LclOutlineEntryAppend (svnw, entry_number);
    else LclOutlineScrollAdjust (svnw);

}

/*
**  This routine is called when the widget is exposed.  Note that the window
**  identifier is located in the clips_window field.
*/

void OutlineDraw (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    int i, y = 0;

/*
**  Ensure that all of the entries being displayed are valid.  Only do this if
**  this routine is being entered with a disabled widget.  This can only occur
**  if the application is dispatching events during a callback.  We must also
**  avoid calling back to get a valid entry.  If the heights have changed OR
**  if the invalidated entry has not been revalidated, then just clear the end
**  of the display.
**
**  Note that this compares it to one because Redisplay disables the display
*/

    if (svnw->svn.disabled_count > 1)
	for (i = 1;  i <= svnw->svn.display_count;  i++)
	{
	    DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
	    if (svnw->svn.display_invalid[i] != 0)
	    {
		Dimension old_height = svnw->svn.display_invalid[i];
		if (!svnentry->valid || (old_height != svnentry->height))
		{
		   if (i > svnw->svn.num_path + 1) LclOutlineClearEOS (svnw, i);
		   else {
			  int current_top = svnw->svn.entries[svnw->svn.num_path+1];
			  LclOutlineClearEOS (svnw, 1);
			  LclOutlinePositionJump (svnw, current_top);
			}

		   break;
		}
		svnw->svn.display_invalid[i] = 0;

		/******************************************************************************/
		/*                                                                            */
		/* See if this entry is from the PTR window or the main entry window.  Clear  */
		/* and redraw the entry in the appropriate window.			      */
		/*                                                                            */
		/******************************************************************************/
		if ((XtIsManaged(svnw->svn.primary_ptr_widget)) && (i <= svnw->svn.num_path))
		{
		    XClearArea (XtDisplay(svnw), 
			   XtWindow(svnw->svn.primary_ptr_widget), 
			   0, 
			   (Position)y, 
		           XtWidth (svnw->svn.primary_ptr_widget),
			   svnentry->height,
			   FALSE);

		    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
			XClearArea (XtDisplay(svnw), 
				    XtWindow(svnw->svn.secondary_ptr_widget), 
				    0, 
				    (Position)y, 
				    XtWidth (svnw->svn.secondary_ptr_widget),
				    svnentry->height,
				    FALSE);
		   
		    LclOutlineDrawEntry(svnw, svnentry, y, svnw->svn.primary_ptr_widget);
#ifdef LS_EVENT
printf("OutlineDraw drawing entry %d in ptr window, y = %d\n", i, y);
#endif
		}
		else
		{
		    XClearArea (XtDisplay(svnw),
			      XtWindow(svnw->svn.primary_ptr_widget),
			      0, 
			      (Position)y,
		              XtWidth(svnw->svn.primary_window_widget),
			      svnentry->height, 
			      FALSE);

		    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
			 XClearArea (XtDisplay(svnw),
			      XtWindow(svnw->svn.secondary_window_widget),
			      0, 
			      (Position)y,
		              XtWidth(svnw->svn.secondary_window_widget),
			      svnentry->height, 
			      FALSE);

		    LclOutlineDrawEntry (svnw, svnentry, y, svnw->svn.primary_window_widget);
#ifdef LS_EVENT
printf("OutlineDraw drawing entry %d in main window, y = %d\n", i, y);
#endif
		}

	    }

	    /*
	    **  If this is the last entry in the PTR window, reset y for the main window.
	    */
	    if ((i == svnw->svn.num_path) && (XtIsManaged(svnw->svn.primary_ptr_widget)))
		y = 0;
	    else
		y = y + svnentry->height;
	}

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif

/*
**  Disable the widget in case new entries are added while we are here
*/
    DXmSvnDisableDisplay ((Widget)svnw);


/*
**  Redraw only the exposed entries
*/
    OutlineDrawExposed (svnw);

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif

/*
**  Enable the display in case display has changed
*/
    DXmSvnEnableDisplay ((Widget)svnw);

}

/*
**  This routine handles any resize work after the window is done...  Keep in
**  mind that an expose on the entire window is waiting to happen...  While
**  processing the expose, the widget is disabled, displayed, and then reenabled.
*/

void OutlineResize (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    int freey, j;

/*
**  Truncate off any entries whose top is beyond the horizontal scroll bar for
**  the case of the window getting smaller.
*/
    freey = 0;
    for (j = 1;  j <= svnw->svn.display_count;  j++)
        {
          DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[j]);
          if (freey > FORM_HEIGHT(svnw))
             {
               svnw->svn.display_count = j - 1;
               break;
             };
          freey = freey + svnentry->height;
        };

/*
**  The display_count must be > num_path
*/
    if (svnw->svn.num_path > 0 && svnw->svn.display_count < svnw->svn.num_path + 1)
	svnw->svn.display_count = svnw->svn.num_path + 1;

/*
**  Mark the display as having been changed.  This will cause the entries to
**  be appended and the scroll bars to be adjusted.
*/
    svnw->svn.display_changed = TRUE;
}

/*
**  This routine is called indirectly when the application has called the
**  routine DXmSvnSetApplDragging.  It must turn on or off any subwidgets.
*/

void OutlineSetApplDragging (svnw, value)

  svn_widget svnw;
  int value;

{
/*
**  If it is being turned on, then we must desensitize our subwidgets so that
**  they do not get events.  Setting them insensitive is not sufficient.  We
**  must also create the ghost for them...
*/
    if (value)
       {
          OutlineCreateGhost (svnw,0);
       };


/*
**  If it is being turned off, then we must reenable our subwidgets and destroy
**  the ghost...
*/
    if (!value)
       {
          OutlineDeleteGhost (svnw);
       };

}

/*
**  This local routine determines if a rectangle and an entry intersect.  It returns
**  true or false based on the fact.  The X coordinates are relative to the
**  window in svnw->svn.clips_window
*/

static Boolean LclOutlineIntersects (svnw, offset, low_x, low_y, high_x, high_y)

   svn_widget svnw;
   int offset, low_x, low_y, high_x, high_y;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int x, y;


/*
**  Get the y position of this entry.
*/
    y = LclOutlineMapOffsetToY (svnw, offset);


/*
**  If the top line in the entry is beyond the lowest line in the rectangle
**  then it can't intersect...
*/
    if (y > high_y) return FALSE;


/*
**  Get the SVN entry since we need to look at the height.  If the 
**  bottom line in the entry is above the top line in the rectangle
**  then it can't intersect.  Check for <= because coordinates are
**  zero based.
*/
    svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[offset]);
    if ((y + (int)svnentry->height) <= low_y) return FALSE;


/*
**  If the high X side of the rectangle is less than the beginning 
**  X value of the entry, then we missed to the left.
*/
    x = svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin) - (int) svnw->svn.window_basex;
    if (x > high_x) return FALSE;


/*
**  If we do not have fixed width entries, then compute the real width
**  of the entry.  the right line in the entry is to the left of the left line in the
**  rectangle, then it doesn't intersect.  Check for <= because coordinates are
**  zero based.
*/
    if (!svnw->svn.fixed_width)
       if ((x + svnentry->width) <= low_x)
	  return FALSE;


/*
**  It intersects...
*/
    return TRUE;
}

/*
**  This local routine will take care of clearing the screen from a given
**  entry offset being displayed.  This routine takes care of unmapping 
**  subwidgets, really issuing the clear commands, and adjusting the variables
**  needed to redisplay things later.  
**
**  It redraws the column lines if necessary.
*/

void LclOutlineClearEOS (svnw, offset)

  svn_widget svnw;
  int offset;

{
/*
**  Local data declarations
*/
    int i, y, j;
    int ptr_height, new_display_count;

/*
**  If the offset exceeds the number being displayed, then leave.
*/
    if (offset > svnw->svn.display_count)  return;

       
/*
**  We know that the display is being changed
*/
    svnw->svn.display_changed = TRUE;


/*
**  Get the y position that will be the top.  y relative to the window the offset is in.
*/
    y = LclOutlineMapOffsetToY (svnw, offset);

    /******************************************************************************/
    /*                                                                            */
    /* If the offset of the entry specified is within the path-to-root, then	  */
    /* the logical here has to be to clear all entries after this offset, and the */
    /* remainder of the main entry window entries.				  */
    /*                                                                            */
    /******************************************************************************/
    if ( XtIsManaged(svnw->svn.primary_ptr_widget) && offset <= svnw->svn.num_path && XtIsRealized(svnw) )
	{
	/*
	**  There are a few cases where the PTR window may have been resized out of sync with the
	**  svn.num_path value.  Must make sure that y is in the PTR window.
	*/
	if (y > PTR_HEIGHT(svnw))
	    return;

	/*
	**  Clear all entries from y to the bottom of the ptr window.
	*/
	XClearArea(XtDisplay(svnw),
			   XtWindow(svnw->svn.primary_ptr_widget),
			   0,
			   y,
			   XtWidth(svnw->svn.primary_ptr_widget),
			   PTR_HEIGHT(svnw) - y,
			   FALSE);
		
	if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
		    XClearArea(XtDisplay(svnw),
			   XtWindow(svnw->svn.secondary_ptr_widget),
			   0,
			   y,
			   XtWidth(svnw->svn.secondary_ptr_widget),
			   PTR_HEIGHT(svnw) - y,
			   FALSE);

		
	/* Clear main entries window */
	
	XClearArea (XtDisplay(svnw),
		      XtWindow(svnw->svn.primary_window_widget), 
		      0, 
		      (Position)0, 
		      XtWidth(svnw->svn.primary_window_widget),
		      PRIMARY_HEIGHT(svnw),
		      FALSE);

       if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	     XClearArea (XtDisplay(svnw),
		      XtWindow(svnw->svn.secondary_window_widget), 
		      0, 
		      (Position)0, 
		      XtWidth(svnw->svn.secondary_window_widget),
		      PRIMARY_HEIGHT(svnw),
		      FALSE);

	/******************************************************************************/
	/*                                                                            */
	/* Update the number of entries being display.  Even thou we cleared entries  */
	/* from the PTR window, we want the display count to == num_path, since we    */
	/* know what entries are in the PTR window and don't need to be recalculated  */
	/*                                                                            */
	/******************************************************************************/
	new_display_count = svnw->svn.num_path;
	}
    
    /******************************************************************************/
    /*                                                                            */
    /* Want to clear entries ONLY from main entry window... therefore find out    */
    /* the y of this offset and clear everything in window after that point.      */
    /*                                                                            */
    /******************************************************************************/
    else
	{
	if ((PRIMARY_HEIGHT(svnw) > y) && (XtIsRealized(svnw)))
	    {
	    XClearArea (XtDisplay(svnw),
		      XtWindow(svnw->svn.primary_window_widget), 
		      0, 
		      (Position)y, 
		      XtWidth(svnw->svn.primary_window_widget),
		      PRIMARY_HEIGHT(svnw) - y,
		      FALSE);

          if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	     XClearArea (XtDisplay(svnw),
		      XtWindow(svnw->svn.secondary_window_widget), 
		      0, 
		      (Position)y, 
		      XtWidth(svnw->svn.secondary_window_widget),
		      PRIMARY_HEIGHT(svnw) - y,
		      FALSE);

	    };

	/*
	**  Update the number of entries being displayed
	*/
	new_display_count = offset - 1;
	}
	
    /*
    **  For each of the entries being removed, unmap their subwidgets and lose the
    **  pointer to the entry.
    */
    if (svnw->svn.sub_widgets_used)
	LclOutlineHideWidgets(svnw, offset, svnw->svn.display_count);

    /*
    **  Update the number of entries being displayed
    */
    svnw->svn.display_count = new_display_count;

    /*
    **  If this is columns mode, then redraw the lines on the right hand side.
    */
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
       if (svnw->svn.start_column_component != 0)
          if (svnw->svn.column_lines)
           {
	    /*
	    **  Local data declarations
	    */
                int comp_offset = svnw->svn.start_column_component - 1;
                int column_x = 0 - svnw->svn.secondary_base_x;


	    /*
	    **  Loop on all possible right hand side components.  We will stop
            **  this loop when we see a column width of zero or until we go off
            **  the right side.
	    */
		while (TRUE)
		  {
		    /*
		    **  If this component has a width of zero, then leave.
		    */
		        if (svnw->svn.column_widths [comp_offset] == 0) break;


		    /*
		    **  If the next component has a width of zero, then leave.
		    */
		        if (svnw->svn.column_widths [comp_offset + 1] == 0) break;


 		    /*
		    **  Add in the width of this component.  We are drawing lines
                    **  after the components because we do not have one in front
		    **  of the first secondary component
		    */
	                column_x = column_x + svnw->svn.column_widths [comp_offset] + svnw->svn.default_spacing;
                        

                    /*
		    **  If we are beyond the window, then leave.
		    */
		        if (column_x > XtWidth (svnw->svn.secondary_window_widget)) break;


		    /*
                    **  If we are in the window, then draw the line.
		    */
	                if (column_x > 0)
                           XDrawLine (XtDisplay(svnw),
	                              XtWindow(svnw->svn.secondary_window_widget),
	                              svnw->svn.foreground_gc, 
                                      ( (LayoutIsRtoL(svnw)) ? XtWidth(svnw->svn.secondary_window_widget) - column_x : column_x),
	                              0,
                                      ( (LayoutIsRtoL(svnw)) ? XtWidth(svnw->svn.secondary_window_widget) - column_x : column_x),
		                      XtHeight(svnw->svn.secondary_window_widget)- 0);

 
                    /*
                    **  Increment to the next component and the next column
                    */
                        comp_offset++;
                  };
           };

}  /* End of routine LclOutlineClearEOS */

/*
**  This routine jumps to a location at the top...  All callers of this routine
**  disable and reenable the widget.  This means that only the path and the
**  first entry under the path need dealt with.  This routine then sets the
**  display_changed to TRUE to take care of appending new entries and updating
**  the scroll bars.
*/
void LclOutlinePositionJump (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    int i, level, differ, count, num_path, entries[max_display];
    DXmSvnEntryPtr svnentry;
    int old_num_path;

#ifdef LS_PRINTF
printf("Called LclOutlinePositionJump, entry = %d\n", entry_number);
#endif

/*
**  Get at the entry for the entry_number.  It need not be valid because we
**  only need the level number.  
*/  
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  Based purely on the level number, we can now set the num_path and count
**  variables.
*/
    if (svnw->svn.show_path_to_root)
         num_path = svnentry->level;
    else num_path = 0;

    count = num_path + 1;


/*
**  See if maybe we're already at our destination...
*/
    if (   (num_path == svnw->svn.num_path)
        && (svnw->svn.display_count > count)
        && (svnw->svn.entries[count] == entry_number)
	&& (entry_number != 1) )
	return;


/*
**  Move the initial entry_number and pointer into the proper position.  Note
**  that indexing into both arrays is one-based.
*/
    entries [count] = entry_number;


/*
**  Figure out the path to the root.  The looping variable reflects which 
**  entry number we are looking at.  The level variable reflects the level
**  number that we are looking for.  
*/
    if (num_path != 0)
       {
         level = svnentry->level - 1;
         for (i = entry_number - 1;  i > 0;  i--)
            {
              svnentry = StructGetEntryPtr (svnw, i);
              if (svnentry->level == level)
                 {
                   entries [level+1] = i;
                   if (level == 0) break;
                   level--;
                 };
            };
       };


/*
**  Now we're ready to make this visible to the user.  We have setup a local
**  structure and we have the currently being displayed structure in the 
**  instance record.
**
**  Find the point at which the two structures are different.  Place that
**  point into the differ variable.  Default this value to one past the
**  end of our local display.
*/
    if (svnw->svn.display_count == 0) differ = 1;
    else { 
           differ = count + 1;
           for (i = 1;  i <= count;  i++)
               if ((entries[i] != svnw->svn.entries[i]) || (svnw->svn.display_invalid[i] != 0))
                  {
                   differ = i;
                   break;
                  };
         };


/*
**  If the differing point is beyond what is currently being displayed, then
**  just set display changed to true and leave.  Also be sure that it's bigger
**  than what we deem count to be...
*/
    if ((differ > count) && (differ > svnw->svn.display_count))
       {
         svnw->svn.display_changed = TRUE;
         return;
       };

/*
**  Commit the new path length
*/
    old_num_path = svnw->svn.num_path;
    svnw->svn.num_path = num_path;
    
    /******************************************************************************/
    /*                                                                            */
    /* If the PTR number of entries == 0, then just see if we need to unmanage    */
    /* the PTR window if not just return.  When num_path = 0 then count = 1.	  */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.num_path == 0)
    {
	if (XtIsManaged(svnw->svn.primary_ptr_widget))
	{
	    /*
	    **  Clear the entire screen and unmange the PTR window
	    */
	    LclOutlineClearEOS (svnw, 1);
	    LclOutlineUnmanagePTR(svnw);
	}
	else
	{
	    /*
	    **  Clear from differ to the end of the screen.
	    */
	    LclOutlineClearEOS (svnw, differ);
	}

	/*
	**  Append the entry under the PTR window, if it is not already there.
	*/
	if (entries[count] != svnw->svn.entries[count] || svnw->svn.display_count == 0)
	{
	    LclOutlineEntryAppend (svnw, entries[count]);

	    svnentry = StructGetValidEntryPtr(svnw, entries[count]);
	    svnw->svn.display_invalid[count] = svnentry->height;
	}
    }
    else
    {
	/******************************************************************************/
	/*                                                                            */
	/* After we commit to a new path... if they are different, then we have to    */
	/* adjust the size of the PTR window, if one exists.  If the new num_path     */
	/* is not == 0, and the PTR window is unmanaged, then we must manage it before */
	/* we can resize it.							      */
	/*                                                                            */
	/******************************************************************************/
	if ((svnw->svn.show_path_to_root) && (!XtIsManaged(svnw->svn.primary_ptr_widget)))
	{
	    if (svnw->svn.sub_widgets_used)
		LclOutlineHideWidgets(svnw, 1, svnw->svn.display_count);

	    /*
	    **	Clear the entire display.  This also sets svnw->svn.display_count = 0.
	    */
	    LclOutlineClearEOS (svnw, 1);

	    for (i = 1; i <= svnw->svn.num_path; i++)
	    {
		svnentry = StructGetValidEntryPtr(svnw, entries[i]);
		svnw->svn.entries[i] = entries[i];
		svnw->svn.display_invalid[i] = svnentry->height;
	    }

	    svnw->svn.display_count = svnw->svn.num_path;
	    LclOutlineManagePTR(svnw);
	}
	else
	{
	    /******************************************************************************/
	    /*                                                                            */
	    /* See if the new num_path is different than the old.  If so, then adjust the */
	    /* size of the PTR window.							  */
	    /*                                                                            */
	    /******************************************************************************/
	    /*
	    **  Clear from differ to the end of the screen.
	    */
	    LclOutlineClearEOS (svnw, differ);

	    /******************************************************************************/
	    /*                                                                            */
	    /* Go thru path-to-root entries... setting display_invalid bit on ones after  */
	    /* 'differ'.  Therefore, any entry that is different in entries[] and         */
	    /* svnw->svn.entries... need to be invalidated.				  */
	    /*                                                                            */
	    /******************************************************************************/
	    for (i = differ; i <= svnw->svn.num_path; i++)
	    {
		svnentry = StructGetValidEntryPtr(svnw, entries[i]);
		svnw->svn.entries[i] = entries[i];
		svnw->svn.display_invalid[i] = svnentry->height;
	    }

	    if (old_num_path != svnw->svn.num_path)
	    {
		if (svnw->svn.sub_widgets_used)
		    LclOutlineHideWidgets(svnw, svnw->svn.num_path + 1, svnw->svn.display_count);

		LclOutlineResizePTR(svnw);
	    }
	}

	/*
	**  Append the entry under the PTR window, if it is not already there.
	*/
	if (entries[count] != svnw->svn.entries[count])
	    LclOutlineEntryAppend (svnw, entries[count]);
	else
	    svnw->svn.display_count = count;

	svnentry = StructGetValidEntryPtr(svnw, entries[count]);
	svnw->svn.display_invalid[count] = svnentry->height;

    }   /* End of section to handle PTR changes */
}

/*  
**  This routine is called by the STRUCTURE module whenever a valid entry
**  is desired.   It is only called if 'height_adjusted' is false.
*/

void OutlineAdjustHeight (svnw, svnentry)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;

{	
/*
**  Local Variables
*/
    int real_width;


/*
**  Call the common routine to do most of the work
*/
    DisplayAdjustEntryHeight (svnw, svnentry);


/*
**  Bump the entry height and width.  Do it differently for expectHighlighting.
*/
    if (svnw->svn.expect_highlighting)
       {
     /*    svnentry->height += 5; */
         svnentry->width  += 4;
       }
    else
       {
     /*    svnentry->height += 2; */
         svnentry->width  += 1;
       };


/*
**  See if this entry is bigger than the biggest so far.  First adjust the width
**  for the indention amount.
*/
    real_width = svnentry->width + svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin);
    if (svnw->svn.max_width < real_width)  svnw->svn.max_width = real_width;


}

/*  
**  This routine is called by the SVN module whenever a widget component
**  wants to change size.
*/

void OutlineAdjustEntrySize (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{	
/*
**  Invalidate Entry does all the work we need
*/
    OutlineInvalidateEntry (svnw, entry_number);


}

/*
**  Routine that redraws those entries that intersect clipping rectangles.

*/

void OutlineDrawExposed (svnw)

  svn_widget svnw;

{
/*
**  Routine data declarations
*/
    int entry_numbers[max_display], i, j, count;
    Boolean redraw_main = FALSE, redraw_ptr = FALSE;
    int y = 0;
    Dimension x_adjust = 0;
    DXmSvnEntryPtr svnentry;

    /*
    **  Return if no displayed entries.
    */
    if (svnw->svn.display_count == 0)  return;


    /*
    **  If this is a forced refresh, then ignore the clipping rectangles and
    **  refresh all of the entries.
    */
    if (svnw->svn.refresh_all) 
    {
	svnw->svn.refresh_all = FALSE;
	redraw_ptr = TRUE;
	redraw_main = TRUE;
	for (i = 1;  i <= svnw->svn.display_count;  i++)
	    svnw->svn.display_invalid [i] = TRUE;
    }
    else
    {
	/*
	**  Determine which window needs to be redrawn.
	**
	**  Loop on each of the clipping rectangles marking the intersected entries
	**  as invalid.  We do it this way because there may be multiple clipping 
	**  rectangles and they may intersect common entries.  If you display the
	**  entry in the first loop, you may be displaying it many times.
	**
	**  If the clips are in the secondary, then we must setup a variable
	**  called the x_adjust.  This is because the routine that maps
	**  a rectangle to intersecting entries assumes that the x and y
	**  positions are relative to the SVN window and not the secondary window.
	*/
	if (svnw->svn.clip_count != 0)
	{
	    if (svnw->svn.clips_window == XtWindow (svnw->svn.secondary_window_widget))
		x_adjust = XtWidth (svnw->svn.primary_window_widget) + mullion_width;

	    for (i = 0;  i < svnw->svn.clip_count;  i++)
	    {
		count = LclOutlineMapRectangle (svnw, entry_numbers, svnw->svn.clips_window,
						(int)svnw->svn.clips[i].x + x_adjust, 
						(int)svnw->svn.clips[i].y,
						(int)svnw->svn.clips[i].x + x_adjust + svnw->svn.clips[i].width,
						(int)svnw->svn.clips[i].y + svnw->svn.clips[i].height);

		for (j = 0;  j < count;  j++)
		    svnw->svn.display_invalid[LclOutlineMapNumberToOffset(svnw,entry_numbers[j])] = TRUE;

		if (count > 0)
		    redraw_main = TRUE;
	    }
	}

	if (svnw->svn.ptr_clip_count != 0)
	{
	    if (svnw->svn.ptr_clips_window == XtWindow (svnw->svn.secondary_ptr_widget))
		x_adjust = XtWidth (svnw->svn.primary_ptr_widget) + mullion_width;

	    for (i = 0;  i < svnw->svn.ptr_clip_count;  i++)
	    {
		count = LclOutlineMapRectangle (svnw, entry_numbers, svnw->svn.ptr_clips_window,
						(int)svnw->svn.ptr_clips[i].x + x_adjust, 
						(int)svnw->svn.ptr_clips[i].y,
						(int)svnw->svn.ptr_clips[i].x + x_adjust + svnw->svn.ptr_clips[i].width,
						(int)svnw->svn.ptr_clips[i].y + svnw->svn.ptr_clips[i].height);

		for (j = 0;  j < count;  j++)
		    svnw->svn.display_invalid[LclOutlineMapNumberToOffset(svnw,entry_numbers[j])] = TRUE;

		if (count > 0)
		    redraw_ptr = TRUE;
	    }
	}
    }

    if (redraw_ptr)
	for (i = 1; i <= svnw->svn.num_path; i++)
	{
	    svnentry = StructGetValidEntryPtr(svnw, svnw->svn.entries[i]);

	    if (svnw->svn.display_invalid[i])
	    {
#ifdef LS_EVENT
printf("OutlineDrawExposed, drawing entry %d in ptr window\n", svnw->svn.entries[i]);
#endif
		XClearArea (XtDisplay(svnw), 
		       XtWindow(svnw->svn.primary_ptr_widget), 
		       0, 
		       (Position)y, 
		       XtWidth (svnw->svn.primary_ptr_widget),
		       svnentry->height,
		       FALSE);

		if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
		    XClearArea (XtDisplay(svnw), 
				XtWindow(svnw->svn.secondary_ptr_widget), 
				0, 
				(Position)y, 
				XtWidth (svnw->svn.secondary_ptr_widget),
				svnentry->height,
				FALSE);

		LclOutlineDrawEntry (svnw, svnentry, y, svnw->svn.primary_ptr_widget);
		svnw->svn.display_invalid[i] = 0;
	    }
	    y = y + svnentry->height;
	}

    y = 0;
    if (redraw_main)
	for (i = svnw->svn.num_path + 1;  i <= svnw->svn.display_count;  i++)
	{
	    svnentry = StructGetValidEntryPtr (svnw, svnw->svn.entries[i]);

	    if (svnw->svn.display_invalid[i])
	    {
#ifdef LS_EVENT
printf("OutlineDrawExposed, drawing entry %d in main window, y = %d\n", svnw->svn.entries[i], y   );
#endif
		LclOutlineDrawEntry (svnw, svnentry, y, svnw->svn.primary_window_widget);
		svnw->svn.display_invalid[i] = 0;
	    }
	    y = y + svnentry->height;
	}

    svnw->svn.clip_count = 0;
    svnw->svn.ptr_clip_count = 0;

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif

/******************************************************************************/
/*                                                                            */
/* Make sure we redraw the column lines if we are in column display mode.     */
/*                                                                            */
/******************************************************************************/
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	LclOutlineDrawColumnLines(svnw);


}

/*
**  This routine does processing when the display type is changed to Outline.
*/
void OutlineChangeMode (svnw)

    svn_widget	svnw;

{
/*
**  Local variables
*/
    int entry_number;


/*
**  Set-up code...
*/
    if (svnw->svn.display_mode != DXmSvnKdisplayTree)
	{
	Arg args[1];


	/*
	**  Set the display position
	*/
	if (svnw->svn.current_entry_number > 0)
	    OutlinePositionDisplay(svnw, svnw->svn.current_entry_number, DXmSvnKpositionTop);

	/*
	**  Insensitize the nav button, and make it look inactive
	*/
	XtSetSensitive(svnw->svn.nav_button,FALSE);

        if (!LayoutIsRtoL(svnw))
	    {
	    XtSetArg (args[0] ,XmNlabelPixmap, svnw->svn.outline_nav_pixmap);
	    XtSetValues (svnw->svn.nav_button, args, 1);
	    }
	}

/*
**  Clean-up code...
*/
    else {
	/******************************************************************************/
	/*                                                                            */
	/* If PTR window is managed, then we need to unmanage it for tree mode.	      */
	/*                                                                            */
	/******************************************************************************/
	if (XtIsManaged(svnw->svn.primary_ptr_widget))
	    LclOutlineUnmanagePTR(svnw);

	/*
	**  Enable the nav button.
	*/
	XtSetSensitive(svnw->svn.nav_button,TRUE);


	/*
	**  Set the current entry for the Tree code.
	*/
	svnw->svn.current_entry = NULL;
	entry_number = svnw->svn.entries[svnw->svn.num_path + 1];

	/*
	**  Return if nothing displayed
	*/
	if (entry_number == 0) return;

	/*
	**  If entry number is the root, let position default 
	*/
	if (entry_number == 1) return;


	/*
	**  Otherwise set current to one found.
	*/
	svnw->svn.current_entry = StructGetEntryPtr(svnw, entry_number);
	svnw->svn.prevx = 0;
	svnw->svn.prevy = 0;

	/*
	**  Destroy the index window
	*/
	if (svnw->svn.index_window != NULL)
	    {
	    XtDestroyWidget(svnw->svn.index_window);
	    svnw->svn.index_window = NULL;
	    }
	}

} /* OutlineChangeMode */

/*
**  This routine scrolls the screen up one line.
*/

void LclOutlineScrollUp (svnw)
svn_widget svnw;
{
/*
**  Local data declarations
*/
    DXmSvnCallbackStruct temp;
    int offset;
    int old_num_path;

    /*
    **  If we are displaying the last entry, then ignore it too.
    */
    if (LclOutlineAtBottom (svnw) == TRUE)
       return;

    /******************************************************************************/
    /*                                                                            */
    /* Save the old value for the path to root, then set the new value.  We need  */
    /* to save the old, so that later we know whether to redraw entries in the    */
    /* PTR window, along with resetting the height of that window.		  */
    /*                                                                            */
    /******************************************************************************/
    old_num_path = svnw->svn.num_path;

    /*
    **  Call the local routine that returns as its value the offset into the
    **  path that should be removed.  For some reason this routine sets
    **  svnw->svn.num_path field so we have to restore it.
    */
    offset = LclOutlineScrollUpCandidate (svnw);
    svnw->svn.num_path = old_num_path;

    /*
    **  If really was at bottom, or couldn't scroll because display too small, 
    **  then return.
    */
    if (offset >= 0)
    {
	if (offset > svnw->svn.num_path)
	    LclOutlineDragLS(svnw, svnw->svn.entries[offset] + 1);
	else
	    LclOutlineDragLS(svnw, svnw->svn.entries[offset + 1]);

	/*
	**  Issue the callback for the display having changed.
	*/
	temp.reason = DXmSvnCRDisplayChanged;
	XtCallCallbacks ((Widget)svnw, DXmSvnNdisplayChangedCallback, &temp);
    }
    return;
}

/*
**  This routine determines who gets axed when a scroll up operation
**  is being performed.  If can't find a candidate (such as when the display
**  is too small to hold the path-to-root) then return -1.
*/

static int LclOutlineScrollUpCandidate (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr entry1, entry2, entry3;
    int i, previous_level;


/*
**  If we are not showing the path to the root, then return offset 1.
*/
    if (!svnw->svn.show_path_to_root) return 1;


/*
**  If there is currently no path to root, then go through each entry and find 
**  the first two that are on the same level.  Set the path to root counter and
**  the display top variable to make it look as if the path to root was established.
*/
    if (svnw->svn.num_path == 0)
       for (i = 1;  i < svnw->svn.display_count;  i++)
           {
             entry1 = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
             entry2 = StructGetEntryPtr (svnw, svnw->svn.entries[i+1]);

             if (entry1->level > entry2->level)
                {
                 svnw->svn.num_path = i - 1;
                 break;
                };

             if (entry1->level == entry2->level)
                {
                 svnw->svn.num_path = i;
                 break;
                };
           };


/*
**  If we have don't have at least num_path plus one entries displayed...
*/
    if (svnw->svn.display_count <= svnw->svn.num_path)
       return -1;


/*
**  Now we know we have a path to the root section.
**
**  Since we're scrolling up, we must remove an entry from the path to root
**  array.  Our choices are to delete one of three entries depending on the
**  current top entry and the level number of the last in the path to root
**  and the next entries level number.
**
**  Our first choice is to see if the last two entries in the real path to root
**  are at the same level.  If they are then remove the latter of the two.
*/
    if (svnw->svn.num_path > 1)
       {
         entry1 = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.num_path]);
         entry2 = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.num_path-1]);
         if (entry1->level == entry2->level) return (svnw->svn.num_path);
       };


/*
**  Our next candidate is the last guy in the path to root if it is at the
**  same level as the guy at the top of the display portion.
*/
    if (svnw->svn.num_path > 0)
    {
         entry1 = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.num_path]);
         entry2 = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.num_path+1]);
         if (entry1->level == entry2->level)
	 {
	    if (entry2->level != 0)
		return (svnw->svn.num_path);
	    else
		return (svnw->svn.num_path + 1);	    /* the entries are both level 0 so remove them both */
	 }
    }


/*
**  If we at the bottom, then return offset 0;
*/
    if ((LclOutlineAtBottom(svnw) == TRUE) || (svnw->svn.num_path == 0))
       {
         return (-1);
       };


/*
**  Our next candidate will be the top entry if it is at the same level or is
**  at a greater level than the one below it.  Note that we may not have a 
**  second entry underneath the path to the root.
*/
    entry1 = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.num_path+1]); 
    entry2 = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.num_path+1]+1);
    if (entry2 == (DXmSvnEntryPtr) NULL) entry2 = entry1->next;
    if (entry2 == (DXmSvnEntryPtr) NULL) return (-1);
    if (entry1->level >= entry2->level) return (svnw->svn.num_path+1);
    

/*
**  Now we know that the top entry is at a lower level than the one below it.
**
**  If the top entry is at the same or at a lower level than the last one in the
**  path, then axe the last entry in the path to root.
*/
    entry3 = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.num_path]);
    if (entry1->level <= entry3->level) return (svnw->svn.num_path);


/*
**  We must go forward until we find two entries at the same level.  We will
**  axe the first of those two.
*/
    for (i = svnw->svn.num_path + 1;  i <= svnw->svn.display_count;  i++)
        {
          previous_level = entry2->level;
          entry2 = StructGetEntryPtr (svnw, svnw->svn.entries[i]+2);
          if (entry2->level <= previous_level) return i + 1;
        };


/*
**  We should not get here, but just in case we'll return top.
*/
    return (svnw->svn.num_path);
}

/*
**  This routine scrolls all of the entries down one position.  We will pull
**  down the one that had just disappeared.
*/

static void LclOutlineScrollDown (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    int i, offset, entry_number;
    int level = -1, offset_level = 0;
    DXmSvnCallbackStruct temp;
    DXmSvnEntryPtr svnentry;


    /*
    **  If we are already at the top, then ignore this call.
    */
    if ((svnw->svn.num_path == 0) && (svnw->svn.entries[1] == 1)) 
       return;


    /*
    **  If the entire path to root plus 1 was not being displayed, then don't scroll.
    */
    if ((svnw->svn.show_path_to_root) && (svnw->svn.display_count <= svnw->svn.num_path))
       return;

    /*
    **  If we do not allow a path, then we know that we are going to bring in the
    **  next entry from the top...
    */
    if (!svnw->svn.show_path_to_root)
    {
	offset = 1;
	entry_number = svnw->svn.entries[1] - 1;
    }
    else
    {
	offset = svnw->svn.num_path + 1;
	entry_number = 0;

	/*
	**  Save the level of the current entry
	*/
	svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[offset]);
	level = offset_level = svnentry->level;

	for (i = svnw->svn.entries[offset] - 1; i > 0; i--)
	{
	    svnentry= StructGetEntryPtr(svnw, i);
	    if (svnentry->level >= level)
	    {
		entry_number = i;
		break;
	    }
	    if (level > 0)
		level--;
	}
    }

    if (entry_number == 0)
	entry_number = svnw->svn.entries[1];

    LclOutlineDragLS(svnw, entry_number);

    /*
    **  Issue the callback for the display having changed.
    */
    temp.reason = DXmSvnCRDisplayChanged;
    XtCallCallbacks ((Widget)svnw, DXmSvnNdisplayChangedCallback, &temp);

    return;
}

/*
**  Routine that shifts the screen to a new window_basex value.
*/
static void LclOutlineChangeBaseX (svnw, newvalue)

  svn_widget svnw;
  int newvalue;

{
/*
**  Local data declarations
*/
    Position src_x, dst_x, clr_x;
    Dimension clr_wid;
    int i, j, num_components;
    int	hid_highlight = FALSE;

/*
**  Max the value such that the widest known entry fits perfectly
*/
    if (newvalue + XtWidth (svnw->svn.primary_window_widget) > svnw->svn.max_width)
       newvalue = svnw->svn.max_width - XtWidth (svnw->svn.primary_window_widget);


/*
**  If the value is less than zero, then make it zero
*/
    if (newvalue < 0) newvalue = 0;


/*
**  If the base is already the same, then leave
*/
    if (svnw->svn.window_basex == newvalue) return;


/*
**  Compute the copy source, destination, etcetera if moving right...
*/
    if (svnw->svn.window_basex > newvalue)
       {
	if (LayoutIsRtoL(svnw))
	    {
	    src_x   = svnw->svn.window_basex - newvalue;
	    dst_x   = 0;
	    clr_wid = src_x;
	    clr_x   = XtWidth (svnw->svn.primary_window_widget) - clr_wid;
	    }
        else
	    {
	    src_x   = 0;
	    clr_x   = 0;
	    dst_x   = svnw->svn.window_basex - newvalue;
	    clr_wid = dst_x;
	    }
       };


/*
**  Compute the copy source, destination, etcetera if moving left...
*/
    if (svnw->svn.window_basex < newvalue)
       {
	if (LayoutIsRtoL(svnw))
	    {
	    src_x   = 0;
	    dst_x   = newvalue - svnw->svn.window_basex;
	    clr_x   = 0;
	    clr_wid = dst_x;
	    }
	else
	    {
	    dst_x   = 0;
	    src_x   = newvalue - svnw->svn.window_basex;
	    clr_wid = src_x;
	    clr_x   = XtWidth (svnw->svn.primary_window_widget) - clr_wid;
	    }
       };


/*
**  The clearing area may exceed the actual window size if the distance that
**  we are moving exceeds the window size.  Adjust for this situation.  Note
**  that the addition of clr_x to the width is really a subtraction.
*/
    if (clr_x < 0)
       {
	 clr_wid = clr_wid + clr_x;
         clr_x   = 0;
       };



/******************************************************************************/
/*                                                                            */
/* Before we go ahead and do the XCopyArea.. we want to get rid of any	      */
/* highlight boxes that might be around entries on the display window.        */
/*                                                                            */
/******************************************************************************/
        
    if (svnw->svn.show_highlighting)
	if (svnw->svn.num_highlighted != 0)
	    {
	    DXmSvnHideHighlighting((Widget)svnw);
	    hid_highlight = TRUE;
	    }


/******************************************************************************/
/*                                                                            */
/* Clear out the location cursor, since we don't want that copied when the    */
/* XCopyArea.								      */
/*                                                                            */
/******************************************************************************/

    if (svnw->svn.location_cursor_entry != NULL)
	DisplayEraseLocationCursor(svnw);
    
/*
**  Issue the copy and the clear.  Note that the heights and Y positions are always
**  the same regardless of copy direction.  Also note that the copy width is always
**  the width of the widget minus the width of the clear.
*/
    if (clr_wid < XtWidth (svnw->svn.primary_window_widget))
       {
       XCopyArea (XtDisplay(svnw),
		  XtWindow(svnw->svn.primary_window_widget),
		  XtWindow(svnw->svn.primary_window_widget), 
		  svnw->svn.copyarea_gc, 
                  (Position)src_x, 
   	          (Position)0, 
   	          XtWidth (svnw->svn.primary_window_widget) - clr_wid, 
	          (Dimension)FORM_HEIGHT(svnw), 
	          (Position)dst_x, 
	          (Position)0);

       if (XtIsManaged(svnw->svn.primary_ptr_widget))
           {
	    XCopyArea (XtDisplay(svnw),
		  XtWindow(svnw->svn.primary_ptr_widget),
		  XtWindow(svnw->svn.primary_ptr_widget), 
		  svnw->svn.copyarea_gc, 
                  (Position)src_x, 
   	          (Position)0, 
   	          XtWidth (svnw->svn.primary_window_widget) - clr_wid, 
	          (Dimension)PTR_HEIGHT(svnw), 
	          (Position)dst_x, 
	          (Position)0);
	    }
	}

/*
**  Clear the area and setup a clip rectangle to handle the redisplay
*/
    XClearArea (XtDisplay(svnw),
		XtWindow(svnw->svn.primary_window_widget),
		(Position)clr_x, 0, 
	        (Dimension)clr_wid, (Dimension)FORM_HEIGHT(svnw), FALSE);

    if (svnw->svn.clip_count < max_clips) 
       {
        svnw->svn.clips[svnw->svn.clip_count].x = clr_x;
        svnw->svn.clips[svnw->svn.clip_count].y = 0;
        svnw->svn.clips[svnw->svn.clip_count].width = clr_wid;
        svnw->svn.clips[svnw->svn.clip_count].height = FORM_HEIGHT(svnw);
        svnw->svn.clip_count++;
       }
    else
       {
        svnw->svn.clip_count = 0;
        svnw->svn.refresh_all = TRUE;
       };


    
    /******************************************************************************/
    /*                                                                            */
    /* If the PTR window is managed, then also clear and setup clip rectangles    */
    /* for it.									  */
    /*                                                                            */
    /******************************************************************************/
    if (XtIsManaged(svnw->svn.primary_ptr_widget))
       {
	XClearArea (XtDisplay(svnw),
		    XtWindow(svnw->svn.primary_ptr_widget),
		    (Position)clr_x, 0, 
		    (Dimension)clr_wid,
		    (Dimension)PTR_HEIGHT(svnw),
		    FALSE);

	if (svnw->svn.ptr_clip_count < max_clips) 
	    {
	    svnw->svn.ptr_clips[svnw->svn.clip_count].x = clr_x;
	    svnw->svn.ptr_clips[svnw->svn.clip_count].y = 0;
	    svnw->svn.ptr_clips[svnw->svn.clip_count].width = clr_wid;
	    svnw->svn.ptr_clips[svnw->svn.clip_count].height = PTR_HEIGHT(svnw);
	    svnw->svn.ptr_clip_count++;
	    }
	else
	    {
	    svnw->svn.ptr_clip_count = 0;
	    svnw->svn.refresh_all = TRUE;
	    };
	    
       }    /* PTR window exists */
       
    

/*
**  Commit the new basex value.
*/
    svnw->svn.window_basex = newvalue;


/*
**  Move all of the widgets to the correct x position
*/
    if (svnw->svn.sub_widgets_used)
       for (i = 1;  i <= svnw->svn.display_count;  i++)
          {
            Position basex = svnw->svn.margin_width;
            DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
            if (svnw->svn.expect_highlighting)
                 basex = basex + (svnentry->level * svnw->svn.indent_margin) - svnw->svn.window_basex + 3;
            else basex = basex + (svnentry->level * svnw->svn.indent_margin) - svnw->svn.window_basex + 1;
            if ((svnw->svn.display_mode == DXmSvnKdisplayColumns) || svnw->svn.secondary_components_unmapped)
                 num_components = svnw->svn.start_column_component - 1;
            else num_components = svnentry->num_components;
            if ((svnw->svn.start_column_component == 0) || (num_components > svnentry->num_components))
                 num_components = svnentry->num_components;
            for (j = 0;  j <= num_components - 1;  j++)
                if (svnentry->entrycompPtr[j].type == DXmSvnKcompWidget)
                   MOVE_WIDGET_HORIZONTAL(svnw, svnentry->entrycompPtr[j].var.is_widget.readwrite_text, 
					  (Position)(basex+svnentry->entrycompPtr[j].x));
          };


/*
**  Synchronized swimming...
*/
    svn_sync(svnw);

/*
**  Catch any graphics expose event due to occluded windows.
*/
    svnw->svn.clips_window = XtWindow (svnw->svn.primary_window_widget);
    DisplayGraphicsExpose (svnw);
    OutlineDrawExposed(svnw);


/******************************************************************************/
/*                                                                            */
/* Since before we did the copyarea we hid the highlighting on highlighted    */
/* entries so that multiple vertical lines did not show up the entry's right  */
/* hand side... now we have to highlight those entries before we finish up    */
/* with this routine.						              */
/*                                                                            */
/******************************************************************************/
    if (hid_highlight)
	DXmSvnShowHighlighting((Widget)svnw);


/******************************************************************************/
/*                                                                            */
/* Lastly, make sure we redraw the location cursor.			      */
/*                                                                            */
/******************************************************************************/
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);

}

/*
**  Routine that shifts the screen to a new secondary_base_x value.
*/
static void LclOutlineChangeSecondaryBaseX (svnw, newvalue)

  svn_widget svnw;
  int newvalue;

{
/*
**  Local data declarations
*/
    Position src_x, dst_x, clr_x;
    Dimension clr_wid;
    int i, j;


/*
**  Max the value such that the widest known entry fits perfectly
*/
    if (newvalue + XtWidth (svnw->svn.secondary_window_widget) > svnw->svn.secondary_max_width)
       newvalue = svnw->svn.secondary_max_width - XtWidth (svnw->svn.secondary_window_widget);


/*
**  If the value is less than zero, then make it zero
*/
    if (newvalue < 0) newvalue = 0;


/*
**  If the base is already the same, then leave
*/
    if (svnw->svn.secondary_base_x == newvalue) return;


/*
**  Compute the copy source, destination, etcetera if moving right...
*/
    if (svnw->svn.secondary_base_x > newvalue)
       {
	if (LayoutIsRtoL(svnw))
	    {
	    register delta_x = svnw->svn.secondary_base_x - newvalue;

	    src_x   = delta_x;
	    dst_x   = 0;
	    clr_x   = XtWidth (svnw->svn.secondary_window_widget) - delta_x;
	    clr_wid = delta_x;
	    }
	else
	    {
	    src_x   = 0;
	    clr_x   = 0;
	    dst_x   = svnw->svn.secondary_base_x - newvalue;
	    clr_wid = dst_x;
	    }
       };


/*
**  Compute the copy source, destination, etcetera if moving left...
*/
    if (svnw->svn.secondary_base_x < newvalue)
       {
	if (LayoutIsRtoL(svnw))
	    {
	    register delta_x = newvalue - svnw->svn.secondary_base_x;

	    src_x   = 0;
	    dst_x   = delta_x;
	    clr_x   = 0;
	    clr_wid = delta_x;
	    }
        else 
	    {
	    dst_x   = 0;
	    src_x   = newvalue - svnw->svn.secondary_base_x;
	    clr_wid = src_x;
	    clr_x   = XtWidth (svnw->svn.secondary_window_widget) - clr_wid;
	    }
       };


/*
**  The clearing area may exceed the actual window size if the distance that
**  we are moving exceeds the window size.  Adjust for this situation.  Note
**  that the addition of clr_x to the width is really a subtraction.
*/
    if (clr_x < 0)
       {
	 clr_wid = clr_wid + clr_x;
         clr_x   = 0;
       };


/******************************************************************************/
/*                                                                            */
/* Before we issue the copy area, make sure we clear out the location cursor. */
/*                                                                            */
/******************************************************************************/
    if (svnw->svn.location_cursor_entry != NULL)
	DisplayEraseLocationCursor(svnw);


/*
**  Issue the copy and the clear.  Note that the heights and Y positions are always
**  the same regardless of copy direction.  Also note that the copy width is always
**  the width of the widget minus the width of the clear.
*/
    if (clr_wid < XtWidth (svnw->svn.secondary_window_widget))
       {
       XCopyArea (XtDisplay(svnw),
		  XtWindow(svnw->svn.secondary_window_widget),
		  XtWindow(svnw->svn.secondary_window_widget), 
		  svnw->svn.copyarea_gc, 
                  (Position)src_x, 
   	          (Position)0, 
   	          XtWidth (svnw->svn.secondary_window_widget) - clr_wid, 
	          (Dimension)FORM_HEIGHT(svnw), 
	          (Position)dst_x, 
	          (Position)0);

       if (XtIsManaged(svnw->svn.secondary_ptr_widget))
           {
	    XCopyArea (XtDisplay(svnw),
		  XtWindow(svnw->svn.secondary_ptr_widget),
		  XtWindow(svnw->svn.secondary_ptr_widget), 
		  svnw->svn.copyarea_gc, 
                  (Position)src_x, 
   	          (Position)0, 
   	          XtWidth (svnw->svn.secondary_window_widget) - clr_wid, 
	          (Dimension)PTR_HEIGHT(svnw), 
	          (Position)dst_x, 
	          (Position)0);
	    }
	}

/*
**  Clear the area and setup a clip rectangle to handle the redisplay
*/
    XClearArea (XtDisplay(svnw),
		XtWindow(svnw->svn.secondary_window_widget),
		(Position)clr_x, 0, 
	        (Dimension)clr_wid, (Dimension)FORM_HEIGHT(svnw), FALSE);

    if (svnw->svn.clip_count < max_clips) 
       {
        svnw->svn.clips[svnw->svn.clip_count].x = clr_x;
        svnw->svn.clips[svnw->svn.clip_count].y = 0;
        svnw->svn.clips[svnw->svn.clip_count].width = clr_wid;
        svnw->svn.clips[svnw->svn.clip_count].height = FORM_HEIGHT(svnw);
        svnw->svn.clip_count++;
       }
    else
       {
        svnw->svn.clip_count = 0;
        svnw->svn.refresh_all = TRUE;
       };

    /******************************************************************************/
    /*                                                                            */
    /* If the PTR window is managed, then also clear and setup clip rectangles    */
    /* for it.									  */
    /*                                                                            */
    /******************************************************************************/
    if (XtIsManaged(svnw->svn.secondary_ptr_widget))
       {
	XClearArea (XtDisplay(svnw),
		XtWindow(svnw->svn.secondary_ptr_widget),
		(Position)clr_x, 0, 
	        (Dimension)clr_wid,
		(Dimension)PTR_HEIGHT(svnw), FALSE);

	if (svnw->svn.ptr_clip_count < max_clips) 
	    {
	    svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].x = clr_x;
	    svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].y = 0;
	    svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].width = clr_wid;
	    svnw->svn.ptr_clips[svnw->svn.ptr_clip_count].height = PTR_HEIGHT(svnw);
	    svnw->svn.ptr_clip_count++;
	    }
	else
	    {
	    svnw->svn.ptr_clip_count = 0;
	    svnw->svn.refresh_all = TRUE;
	    };

	}   /* handling PTR window */
	
    

/*
**  Commit the new basex value.
*/
    svnw->svn.secondary_base_x = newvalue;


/*
**  Move all of the widgets to the correct x position
*/
    if (svnw->svn.sub_widgets_used)
       for (i = 1;  i <= svnw->svn.display_count;  i++)
          {
            DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
	    int column_x = svnw->svn.default_spacing / 2;
            if (svnentry->num_components >= svnw->svn.start_column_component)
	       if (svnw->svn.start_column_component != 0)
	          for (j = svnw->svn.start_column_component-1;  j <= svnentry->num_components - 1;  j++)
		      {
                       if (svnentry->entrycompPtr[j].type == DXmSvnKcompWidget)
			  MOVE_WIDGET_HORIZONTAL(svnw, svnentry->entrycompPtr[j].var.is_widget.readwrite_text, 
						 (Position)column_x - svnw->svn.secondary_base_x);

		       column_x = column_x + svnw->svn.column_widths [j] + svnw->svn.default_spacing;
		     };
          };


/*
**  Synchronized swimming...
*/
    svn_sync(svnw);

/*
**  Catch any graphics expose event due to occluded windows.
*/
    svnw->svn.clips_window = XtWindow (svnw->svn.secondary_window_widget);
    DisplayGraphicsExpose (svnw);
    OutlineDrawExposed (svnw);


/******************************************************************************/
/*                                                                            */
/* Lastly, make sure we redraw the location cursor.			      */
/*                                                                            */
/******************************************************************************/
    DisplayHighlightEntry(svnw, svnw->svn.location_cursor);

}

/*
**  This routine will redisplay an entry in the SVN widget window.  If we are
**  in Columns mode then we will draw both sides.
*/

static void LclOutlineDrawEntry (svnw, svnentry, basey, wind)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  int basey;
  Widget wind;

{
/*
**  Local data declarations
*/
    GC gc;
    int basex, width;


/*
**  The following declarations are setup to tell us how to invert
**  selected entries.  If invert_right is true, then either all, none,
**  or a particular component is inverted.  The values are 99, 0, or
**  the component number.
*/
    Boolean invert_left  = FALSE;
    int     invert_right = 0;


/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  Setup the inversion values.  If there is no right hand side, then
**  just invert the left...
*/
    if (svnentry->selected && svnw->svn.show_selections)
	if (svnw->svn.display_mode != DXmSvnKdisplayColumns)
	    invert_left = TRUE;
	else
	{
            if (svnentry->selected_comp < svnw->svn.start_column_component)
		/*
		**  Selection is on the left or range selected.
		*/
		switch ((int)svnw->svn.selection_mode)
		{
		    case DXmSvnKselectEntry:
			invert_left  = TRUE;
			invert_right = 99;
			break;
		    case DXmSvnKselectComp:
			invert_left  = TRUE;
			invert_right = 0;
			break;
		    case DXmSvnKselectCompAndPrimary:
			invert_left = TRUE;
			invert_right = 0;
			break;
		    case DXmSvnKselectEntryOrComp:
			invert_left  = TRUE;
			invert_right = 99;
			break;
		}
	    else if (svnw->svn.start_column_component != 0)
		/*
		**  Selection is on the right.
		*/
		switch ((int)svnw->svn.selection_mode)
		{
		    case DXmSvnKselectEntry:
			invert_left  = TRUE;
			invert_right = 99;
			break;
		    case DXmSvnKselectComp:
			invert_left  = FALSE;
			invert_right = svnentry->selected_comp;
			break;
		    case DXmSvnKselectCompAndPrimary:
			invert_left  = TRUE;
			invert_right = svnentry->selected_comp;
			break;
		    case DXmSvnKselectEntryOrComp:
			invert_left  = FALSE;
			invert_right = svnentry->selected_comp;
			break;
		}
	}

/*
**  Compute the base x value based on the entries level number and on whether
**  the navigation bar is on the left side.
*/
    basex = svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin) - svnw->svn.window_basex;


/*
**  Default the GC to either grayed or normal
*/
    if ((svnentry->grayed) || (XtIsSensitive(svnw) == FALSE))
         gc = svnw->svn.grey_gc;
    else gc = svnw->svn.foreground_gc;
 

/*
**  Invert the entire left hand side if it is selected and we own the global selections.
*/
    if (invert_left)
       {
        /*
        **  Compute the width of the rectangle...
        */
            width = svnentry->width;
            if (svnw->svn.fixed_width) width = XtWidth (svnw->svn.primary_window_widget) - basex;

        /*
        **  Draw things differently if we are showing the highlighting...
        */
            XFillRectangle (XtDisplay(svnw),
			    XtWindow(wind),
			    gc,
                            (Position)( (LayoutIsRtoL(svnw)) ? XtWidth(wind) - basex - width : basex),
			    (Position)basey,  
			    (Dimension)width,  
			    (Dimension)svnentry->height);

        /*
        **  Change the GC
        */
            gc = svnw->svn.background_gc;
       };


/*
**  Draw each component of the entry.  The x and y coordinates are computed
**  by this routine.  If we expect highlighting draw the contents of the 
**  entry slightly shifted to make room fo the highlight box when it appears.
**  Also add one to the base instead of adding one to each component position
**  inside the Draw routine.
*/
    if ((svnentry->selected) && (!invert_left))
	{
	 svnentry->selected = FALSE;
         if (svnw->svn.expect_highlighting)
              DisplayDrawEntry (svnw, wind, svnentry, basex, basey);
         else DisplayDrawEntry (svnw, wind, svnentry, basex, basey);
	 svnentry->selected = TRUE;
	}
    else 
	{
         if (svnw->svn.expect_highlighting)
              DisplayDrawEntry (svnw, wind, svnentry, basex, basey);
         else DisplayDrawEntry (svnw, wind, svnentry, basex, basey);
        };


    /*
    **  If this is column mode, then draw the other side
    */
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	{
	/* First see if entry is supposed to be drawn in PTR window of secondary widget */

	if (wind == svnw->svn.primary_ptr_widget)
	    {
	    DisplayDrawColumnsBox (svnw, svnentry, 0 - svnw->svn.secondary_base_x, basey, invert_right,
		    svnw->svn.secondary_ptr_widget);
	    if (svnw->svn.expect_highlighting)
		DisplayDrawColumns (svnw, svnentry, 0 - svnw->svn.secondary_base_x, basey, invert_right,
		    svnw->svn.secondary_ptr_widget);
	    else		
		DisplayDrawColumns (svnw, svnentry, 0 -	svnw->svn.secondary_base_x, basey, invert_right,
		    svnw->svn.secondary_ptr_widget);
	    }

	/* Draw entry in the main secondary entry window of display */
	else
	    {
	    DisplayDrawColumnsBox (svnw, svnentry, 0 - svnw->svn.secondary_base_x, basey, invert_right,
		    svnw->svn.secondary_window_widget);
	    if (svnw->svn.expect_highlighting)
		DisplayDrawColumns (svnw, svnentry, 0 - svnw->svn.secondary_base_x, basey, invert_right,
		    svnw->svn.secondary_window_widget);
	    else  DisplayDrawColumns (svnw, svnentry, 0 - svnw->svn.secondary_base_x, basey, invert_right,
		    svnw->svn.secondary_window_widget);
	    }
	};       /* column mode - draw components */

/*
**  Box the entire entry if it is highlighted.
*/
    if ((svnentry->highlighted) && (svnw->svn.show_highlighting))
       {
         width = svnentry->width;
         if (svnw->svn.fixed_width) width = XtWidth (wind) - basex;
	 if (svnentry->selected)
	    gc = svnw->svn.inverse_highlight_gc;
	 else
	    gc = svnw->svn.highlight_gc;


         XDrawRectangle (XtDisplay(svnw),XtWindow(wind), gc, 
                         (Position)( (LayoutIsRtoL(svnw)) ? XtWidth(wind)
                                    - basex - width - 1 : basex), (Position)basey,
			 (Dimension)width, (Dimension)svnentry->height-1);
       };


    /*
    ** Finally draw the location cursor... right now it's a plain rectangle...
    ** foreground GC...  if and only if this entry == location cursor entry.
    */ 
    if ((svnentry == svnw->svn.location_cursor_entry))
/*	(svnw->svn.last_event.state != (int)ControlMask)) */
	{
	GC gc;

	gc = svnw->svn.location_cursor_gc;
	width = svnentry->width;
	if (svnw->svn.fixed_width)
	    width = XtWidth(wind) - basex;
	
	XDrawRectangle (XtDisplay(svnw),
			XtWindow(wind),
			gc,
			(Position)( (LayoutIsRtoL(svnw)) ? XtWidth(wind)
                                    - basex - width - 1 : basex),
			(Position)basey,
			(Dimension)width,
			(Dimension)svnentry->height-1);
	}

}

/*
** This routine will redisplay the top portion of an entry
*/

static void LclOutlineDrawEntryTop (svnw, svnentry, basey, offset)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  int basey;
  int offset;

{
/*
**  Local data declarations
*/
    GC gc;
    int basex, width;


/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  Compute the base x value based on the entries level number and on whether
**  the navigation bar is on the left side.
*/
    basex = svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin) - svnw->svn.window_basex;


/*
**  Compute the width of the rectangle...
*/
    width = svnentry->width;
    if (svnw->svn.fixed_width) width = XtWidth (svnw->svn.primary_window_widget) - basex;


/*
**  Clear the whole line, once again, see if the clear needs to be from the PTR window.
*/
    if ((svnw->svn.show_path_to_root) && (offset <= svnw->svn.num_path))
	XClearArea (XtDisplay(svnw), XtWindow(svnw->svn.primary_ptr_widget), 0, (Position)basey, 
	        XtWidth (svnw->svn.primary_ptr_widget), (Dimension)1, FALSE);
    else 
	XClearArea (XtDisplay(svnw), XtWindow(svnw->svn.primary_window_widget), 0, (Position)basey, 
	        XtWidth (svnw->svn.primary_window_widget), (Dimension)1, FALSE);


/*
**  Decide whether to draw the meat of it as a forground or a background GC.
**  Default to showing the background and then switch if appropriate.
*/
    gc = svnw->svn.background_gc;

    if ((svnw->svn.show_selections) && (!svnw->svn.show_highlighting) && (svnentry->selected))
       gc = svnw->svn.foreground_gc;
 

/*
**  Draw the line in the computed GC
*/
    XDrawLine (XtDisplay(svnw), XtWindow(svnw->svn.primary_window_widget), gc, basex, basey, basex + width - 1, basey);
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
       XDrawLine (XtDisplay(svnw), XtWindow(svnw->svn.secondary_window_widget), gc, basex, basey, basex + width - 1, basey);
}

/*
**  This routine finds out what offset corresponds to a given X and Y location.
**  It returns the offset number or zero if no entry is displayed at that 
**  X and Y location.  The find_x and find_y values are relative to the SVN widget.
*/

static int LclOutlineFindOffset (svnw, find_x, find_y)

  svn_widget svnw;
  int find_x, find_y;

{
    /*
    **  Routine data declarations
    */
    int i, offset, left_x, top_y, bottom_y, found;
    Position dst_x, dst_y;
    DXmSvnEntryPtr svnentry;

    /*
    **  If there are no entries being displayed...
    */
    if (svnw->svn.display_count == 0) return 0;

    top_y = 0;
    found = FALSE;
    offset = 0;

    /*
    **  Determine which window find_y is in.  Get the coordinates of the
    **	primary_window_widget relative to the SVN widget.
    */
    StructTranslateCoords(svnw->svn.primary_window_widget, &dst_x, &dst_y);
    if (find_y >= dst_y && find_y < dst_y + PRIMARY_HEIGHT(svnw))
    {
	offset = svnw->svn.num_path + 1;
	top_y = dst_y;
    }
    else if (XtIsManaged(svnw->svn.primary_ptr_widget))
    {
	/*
	**  Get the coordinates of the primary_ptr_widget relative to the SVN widget
	*/
	StructTranslateCoords(svnw->svn.primary_ptr_widget, &dst_x, &dst_y);
	if (find_y >= dst_y && find_y < dst_y + PTR_HEIGHT(svnw))
	{
	    offset = 1;
	    top_y = dst_y;
	}
    }

    if (find_y < top_y || offset == 0) return 0;

    /*
    **  Spin through the entries to find the offset.
    */
    for (i = offset;  i <= svnw->svn.display_count && !found;  i++)
    {
	svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[offset]);
	bottom_y = top_y + svnentry->height; 
	if (find_y < bottom_y)
	    found = TRUE;
	else
	{
	    top_y = bottom_y;
	    offset++;
	}
    }

    if (found == FALSE) return 0;

    /*
    **  If the X position is not passed (zero value), then return the found
    **  offset without checking the X position.
    */
    if (find_x == 0) return offset;

    /*
    **  If the X position is less than the starting display point, then return zero
    */
    left_x = dst_x + svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin) - svnw->svn.window_basex;
    if (find_x < left_x) return 0;

    /*
    **  If we have fixed width entries, then it was a hit.
    */
    if (svnw->svn.fixed_width) return offset;

    /*
    **  Now we must see if the X position is past the end of the entry.  
    */
    if (find_x <= left_x + svnentry->width) return offset;

    /*
    **  If it is in the right hand window, then return the offset.
    */
    if (find_x > XtWidth (svnw->svn.primary_window_widget)) return offset;

    /*
    **  It was beyond the right side of the entry
    */
    return 0;
}

/*
**  This routine is given a display offset and must calculate the Y position of
**  that offset.  This routine can be passed an offset between 1 and the number
**  of displayed entries plus 1.  The y returned is relative the the window that
**  the entry is in (either the PTR or main).
*/

int LclOutlineMapOffsetToY (svnw, offset)

  svn_widget svnw;
  int offset;

{
    /*
    **  Local data declarations
    */
    register int i, top_y, start_entry;

    /*
    ** Check if offset is in the PTR window.  If so then start at entry 1.  If not then
    ** start at the entry after the last entry in the PTR window.
    */
    if ( (offset <= svnw->svn.num_path) && XtIsManaged(svnw->svn.primary_ptr_widget) )
	start_entry = 1;
    else
	start_entry = svnw->svn.num_path + 1;

    /*
    **  Loop from 1 to just one shy of the desired offset.  In the loop add in the
    **  height of the entry at that offset.  In order to stop one before the offset
    **  the loop uses a less than operator.
    */
    top_y = 0; 
    for (i = start_entry;  i < offset;  i++)
        {
          DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
          top_y += svnentry->height;
        };
    
    return top_y;
	
}

/*
**  This routine searches the entries being displayed and returns the offset
**  at which a particular entry was found or zero if not found.
*/

int LclOutlineMapNumberToOffset (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    int offset;


    /*
    **  Loop through the path to the root entries plus the first entry.  If it's 
    **  less than the current entry, but not equal to, then it was not found.
    */
    for (offset = 1;  offset <= svnw->svn.num_path + 1;  offset++)
	if (entry_number < svnw->svn.entries[offset]) return 0;
	    else
		if (entry_number == svnw->svn.entries[offset]) return offset;

    /*
    **  The entry is in the main display portion.  We can compute the offset based
    **  on the first entry after the path to the root.
    */
    offset = entry_number - svnw->svn.entries [svnw->svn.num_path+1];
    offset = offset + svnw->svn.num_path + 1;


    /*
    **  If this offset is not being displayed (off the bottom), then return 0.
    */
    if (offset > svnw->svn.display_count) offset = 0;
    

    /*
    **  Return the offset
    */
    return offset;
}

/*
**  This local routine adds an entry after the last entry if it will fit.  It
**  updates the number being displayed and returns TRUE/FALSE as to whether it
**  fit.
*/

int LclOutlineEntryAppend (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr newentry;
    int lasty;


/*
**  Get the Y position for the appendage
*/
    lasty = LclOutlineMapOffsetToY (svnw, svnw->svn.display_count + 1);

/*
**  If the Y position goes beyond the horizontal scroll bar, then forget it.
*/
    if (lasty > PRIMARY_HEIGHT(svnw)) 
	return FALSE;


/*
**  Get the entry pointer for the new entry.  Note that we sort of disable and
**  reenable around this code so that we don't go recursive...
*/
    svnw->svn.disabled_count++;
    newentry = StructGetValidEntryPtr (svnw, entry_number);
    svnw->svn.disabled_count--;


/*
**  Display the new entry at the bottom
*/
    LclOutlineDrawEntry (svnw, newentry, lasty, svnw->svn.primary_window_widget);


/*
**  Change the display structure and counter
*/
    svnw->svn.display_count++;
    svnw->svn.entries[svnw->svn.display_count] = entry_number;
    svnw->svn.display_invalid[svnw->svn.display_count] = 0;


/*
**  Return a success status
*/
    return TRUE;
}

/*
**  This local routine adds an entry after the last entry if it will fit.  It
**  updates the number being displayed and returns TRUE/FALSE as to whether it
**  fit.
*/

int LclOutlineEntryAppendLS (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr newentry;
    int lasty;


/*
**  Get the Y position for the appendage
*/
    lasty = LclOutlineMapOffsetToY (svnw, svnw->svn.display_count + 1);

/******************************************************************************/
/*                                                                            */
/**  If the Y position goes beyond the horizontal scroll bar, then forget it. */
/*									      */
/******************************************************************************/
    if (lasty > PRIMARY_HEIGHT(svnw)) 
	return FALSE;

/*
**  Get the entry pointer for the new entry.  Note that we sort of disable and
**  reenable around this code so that we don't go recursive...
*/
    svnw->svn.disabled_count++;
    newentry = StructGetValidEntryPtr (svnw, entry_number);
    svnw->svn.disabled_count--;


/*
**  Change the display structure and counter
*/
    svnw->svn.display_count++;
    svnw->svn.entries[svnw->svn.display_count] = entry_number;


/*
**  Return a success status
*/
    return TRUE;
}

/*
**  This routine returns true or false if we are at the bottom of the display.
**  The reason that it is now a routine is because of displaying partial
**  entries.
*/

int LclOutlineAtBottom (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int y;


/*
**  If there are no entries at all, then we must be at the bottom. 
*/
    if (svnw->svn.num_entries == 0)
       return TRUE;


/*
**  If we are not displaying the last entry, then we cannot be at the bottom.
**  We check for less than and not equal to because of partial entry displays.
*/
    if (svnw->svn.entries[svnw->svn.display_count] < svnw->svn.num_entries)
       return FALSE;


/*
**  If we are displaying the last entry, then see if we are displaying the
**  whole entry.
*/
    svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.display_count]);


/*
**  Get the y position of this entry.
*/
    y = LclOutlineMapOffsetToY (svnw, svnw->svn.display_count);

/*
**  If the y plus the height is above the horizontal scroll bar then we are 
**  displaying the whole thing... and are at the bottom...
*/
    if ((y + svnentry->height) <= PRIMARY_HEIGHT(svnw))
       return TRUE;


/*
**  Return that we are not at the bottom yet...
*/
    return FALSE;
}

/*
**  This routine is called when the size of the vertical scroll bar needs changing.
**  We will not change the slider when the path to the root line changes, but 
**  only in response to an add entries, delete entries, or resize.
**
**  We will change the scroll bar only when the size actually changes...
*/
void LclOutlineScrollAdjust (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations.  The normal names are unnormalized while the
**  names prefixed with n are the normalized (1 to 100) values;
*/
    Arg arglist[4];
    int argcount, cvalue, cshown, cminim, cmaxim, value, shown, minim, maxim;


/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  Get the current vertical scroll bar values.
*/
    XtSetArg(arglist[0], XmNvalue,   &cvalue);
    XtSetArg(arglist[1], XmNsliderSize, &cshown);
    XtSetArg(arglist[2], XmNminimum, &cminim);
    XtSetArg(arglist[3], XmNmaximum, &cmaxim);

    XtGetValues (svnw->svn.vscroll, arglist, 4);


    /*********************************************************************/
    /* If the user has resized the window so that only a portion of ONE  */
    /* entry is in the window (the display_count is 0 in this case) then */
    /* adjust the vertical scrollbar so that everything is being shown.  */
    /* This fixes an accvio that previously occurred below when trying   */
    /* to calculate svnentry.                                            */
    /*********************************************************************/

    if ((svnw->svn.num_entries == 0) || (svnw->svn.display_count == 0))
    {
	value = 1;
	shown = 4;
	minim = 1;
	maxim = 5;
    }
    else
    {
	DXmSvnEntryPtr svnentry;
	Position y;

	/*
	**  If everything is not being shown and the window has been resized but is
	**  still large enough so that we can display at least one entry, then we
	**  will set the real values.
	**
	**  If everything is shown and the window is large enough so that we can
	**  display at least one entry, make sure that we do not include half an entry
	**  in the slider size.
	*/
	value = svnw->svn.entries[svnw->svn.num_path+1];
	minim = 1;
	maxim = svnw->svn.num_entries + 1;

	svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[svnw->svn.display_count]);
	y = LclOutlineMapOffsetToY (svnw, svnw->svn.display_count);

	if ((y + svnentry->height) > PRIMARY_HEIGHT(svnw))
	    shown = svnw->svn.display_count - svnw->svn.num_path - 1;
	else
	    shown = svnw->svn.display_count - svnw->svn.num_path;

        if (shown < 1) shown = 1;
    }


/*
**  Set the vertical scrollbar values if they are different
*/
    argcount = 0;

    if (cvalue != value) {XtSetArg(arglist[argcount], XmNvalue,   value);  argcount++; };
    if (cshown != shown) {XtSetArg(arglist[argcount], XmNsliderSize, shown);  argcount++; };
    if (cminim != minim) {XtSetArg(arglist[argcount], XmNminimum, minim);  argcount++; };
    if (cmaxim != maxim) {XtSetArg(arglist[argcount], XmNmaximum, maxim);  argcount++; };

    if (argcount != 0) XtSetValues (svnw->svn.vscroll, arglist, argcount);


/*
**  Get the current horizontal scroll bar values.
*/
    XtSetArg(arglist[0], XmNvalue,   &cvalue);
    XtSetArg(arglist[1], XmNsliderSize, &cshown);
    XtSetArg(arglist[2], XmNminimum, &cminim);
    XtSetArg(arglist[3], XmNmaximum, &cmaxim);

    XtGetValues (svnw->svn.hscroll, arglist, 4);


/*
**  Compute the values needed for the horizontal scroll bar.
*/
    minim =  0;
    if (svnw->svn.num_entries == 0)
    {
	maxim = XtWidth(svnw->svn.primary_window_widget);
	shown = maxim;
	value = minim;
    }
    else
    {
	value =  svnw->svn.window_basex;
	shown =  XtWidth (svnw->svn.primary_window_widget);
	/*
	**  Compute the maximum value for the horizontal scrollbar.  This has to take
	**  into account the case where the widest entry right hand side is in view or
	**  not in view.
	*/
	if ((svnw->svn.max_width - svnw->svn.window_basex) <= XtWidth (svnw->svn.primary_window_widget))
	     maxim =  XtWidth (svnw->svn.primary_window_widget) + svnw->svn.window_basex;
	else
	    maxim =  svnw->svn.max_width;
    }

    if (shown < 1) shown = 1;

/*
**  Set the horizontal scrollbar values if they are different
*/
    argcount = 0;

    if (cvalue != value) {XtSetArg(arglist[argcount], XmNvalue,   value);  argcount++; };
    if (cshown != shown) {XtSetArg(arglist[argcount], XmNsliderSize, shown);  argcount++; };
    if (cminim != minim) {XtSetArg(arglist[argcount], XmNminimum, minim);  argcount++; };
    if (cmaxim != maxim) {XtSetArg(arglist[argcount], XmNmaximum, maxim);  argcount++; };

    if (argcount != 0) XtSetValues (svnw->svn.hscroll, arglist, argcount);


/*
**  Do the secondary if necessary
*/
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
       {
	/*
	**  Get the current horizontal scroll bar values.
	*/
	    XtSetArg(arglist[0], XmNvalue,   &cvalue);
	    XtSetArg(arglist[1], XmNsliderSize, &cshown);
	    XtSetArg(arglist[2], XmNminimum, &cminim);
	    XtSetArg(arglist[3], XmNmaximum, &cmaxim);

	    XtGetValues (svnw->svn.secondary_hscroll, arglist, 4);

	    minim =  0;
	    if (svnw->svn.num_entries == 0)
	    {
		maxim = XtWidth(svnw->svn.secondary_window_widget);
		shown = maxim;
		value = minim;
	    }
	    else
	    {
		value =  svnw->svn.secondary_base_x;
		shown =  XtWidth (svnw->svn.secondary_window_widget);
		/*
		**  Compute the maximum value for the horizontal scrollbar.  This has to take
		**  into account the case where the widest entry right hand side is in view or
		**  not in view.
		*/
		if ((svnw->svn.secondary_max_width - svnw->svn.secondary_base_x) <= XtWidth (svnw->svn.secondary_window_widget))
		    maxim = XtWidth (svnw->svn.secondary_window_widget) + svnw->svn.secondary_base_x;
		else maxim = 
		    svnw->svn.secondary_max_width;
	    }

        if (shown < 1) shown = 1;

	/*
	**  Set the horizontal scrollbar values if they are different
	*/
	    argcount = 0;

	    if (cvalue != value) {XtSetArg(arglist[argcount], XmNvalue,   value);  argcount++; };
	    if (cshown != shown) {XtSetArg(arglist[argcount], XmNsliderSize, shown);  argcount++; };
	    if (cminim != minim) {XtSetArg(arglist[argcount], XmNminimum, minim);  argcount++; };
	    if (cmaxim != maxim) {XtSetArg(arglist[argcount], XmNmaximum, maxim);  argcount++; };

	    if (argcount != 0) XtSetValues (svnw->svn.secondary_hscroll, arglist, argcount);
	};
}

/*
**  This callback routine is invoked from all of the buttons and the scroll
**  bars when the user requests help.
*/

void OutlineScrollHelp (w, unused_tag, scroll)

  Widget w;
  int unused_tag;
  XmScrollBarCallbackStruct *scroll;

{
/*
**  The parent of all of them is the svn widget
*/
    svn_widget svnw = StructFindSvnWidget (w);
    DXmSvnCallbackStruct temp;


/*
**  Set up the structure...  Set the component number field to the widget id of
**  the scroll bar making the call.  This is for VMSmail.
*/
    temp.reason = DXmSvnCRHelpRequested;
    if (w == svnw->svn.nav_button)
	temp.entry_number = DXmSvnKhelpNavButton;
    else
	temp.entry_number = DXmSvnKhelpScroll;
    if (scroll == NULL)
	temp.time = CurrentTime;
    else	
	temp.time = scroll->event->xbutton.time;
    temp.component_number = (int) w;


/*
**  Make the callback
*/
    XtCallCallbacks ((Widget)svnw, DXmSvnNhelpRequestedCallback, &temp);
}

/*
**  This callback routine gets all of the vertical scroll bar callbacks.  We
**  will either handle the work here or dispatch to a local routine.
*/

void OutlineVScroll (w, unused_tag, scroll)

  Widget w;
  int unused_tag;
  XmScrollBarCallbackStruct *scroll;

{
/*
**  The parent of the vertical scroll bar is the svn widget
*/
    svn_widget svnw = StructFindSvnWidget (w);


/*
**  As a workaround to the multiple callback bug in the scrollbar.  If scroll
**  is already in progress, then ignore this call and return.  Otherwise set
**  the VScrollInProgress flag in case we get called before this routine
**  completes (If we have to do a GetEntry callback the application callback
**  routine may dispatch some events).
*/
    if (svnw->svn.vscroll_in_progress) return;
    svnw->svn.vscroll_in_progress = TRUE;


/*
**  If the widget is empty, then ignore all calls from the scrollbar
*/
    if (svnw->svn.num_entries == 0)
       {
	  svnw->svn.vscroll_in_progress = FALSE;
          return;
       };

 
/*
**  Call the TopTree routine if needed
*/
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
       {
          TopTreeVScroll (w, scroll);
	  svnw->svn.vscroll_in_progress = FALSE;
          return;
       };


/*
**  Adjust the value if it is greater than the number of entries.  This is 
**  a necessary step...
*/
    if (scroll->value > svnw->svn.num_entries) scroll->value = svnw->svn.num_entries;


    /*
    **  Case on the reason code
    */
    switch (scroll->reason)
    {
	case XmCR_INCREMENT:
	    LclOutlineScrollUp (svnw);
	    break;

        case XmCR_DECREMENT:
	    LclOutlineScrollDown (svnw);
	    break;

        case XmCR_PAGE_INCREMENT:
	    DisplaySetWatch (svnw, TRUE);
	    OutlinePositionDisplay (svnw, 0, DXmSvnKpositionNextPage);  
	    DisplaySetWatch (svnw, FALSE);
	    break;

        case XmCR_PAGE_DECREMENT:
	    DisplaySetWatch (svnw, TRUE);
	    OutlinePositionDisplay (svnw, 0, DXmSvnKpositionPreviousPage);  
	    DisplaySetWatch (svnw, FALSE);
	    break;

        case XmCR_DRAG:
	    if (svnw->svn.live_scrolling)
		LclOutlineDragLS (svnw, scroll->value);  
	    else
		LclOutlineVScrollDrag(svnw, scroll->value);
	    break;

        case XmCR_TO_TOP:
	    if (svnw->svn.num_entries > 4)		/* position entry 1 at top */
		OutlinePositionDisplay (svnw, 1, DXmSvnKpositionTop);
	    break;

        case XmCR_TO_BOTTOM:
	    if (svnw->svn.num_entries > 4)		/* position last entry at bottom */
		OutlinePositionDisplay (svnw, svnw->svn.num_entries, DXmSvnKpositionBottom);
	    break;

      case XmCR_VALUE_CHANGED:
	    /*
	    **  We'll only use this when the index window is being used.  This will always be indicated by
	    **  a shown value of non-zero which is guaranteed true after a call to the dragging routine.
	    */
	    if (svnw->svn.index_window_shown != 0)
	    {
		if (scroll->value != svnw->svn.entries[svnw->svn.num_path+1]) 
		{
		    int entry_number;
		    DXmSvnEntryPtr window_entry;

		    /*
		    **  Because of the way LclOutliveVScrollDrag works, the value of the scrollbar may not be an
		    **  indexable entry.  So we have to go backwards until we find an indexable entry.
		    */
		    entry_number = scroll->value;
		    window_entry = StructGetEntryPtr (svnw, entry_number);

		    while (!window_entry->index_window && entry_number > 1)
		    {
			entry_number--;
			window_entry = StructGetEntryPtr (svnw, entry_number);
		    }

		    OutlinePositionDisplay (svnw, entry_number, DXmSvnKpositionTop);
		}
		/*
		**  Unmap the index window and clear the index_window_shown field.
		*/
		if (svnw->svn.index_window != NULL)
		    XtUnmapWidget (svnw->svn.index_window);
		svnw->svn.index_window_shown = 0;
	    }
	    break;

      }	    /* switch */

    svnw->svn.vscroll_in_progress = FALSE;
}

/*
**  This routine gets all of the horizontal scroll callbacks for the
**  right hand side window.
*/

void LclOutlinesecondaryHScroll (svnw, scroll)

  svn_widget svnw;
  XmScrollBarCallbackStruct *scroll;

{
/*
**  Switch on the reason code
*/
    switch (scroll->reason)

      {
        case XmCR_VALUE_CHANGED  : LclOutlineChangeSecondaryBaseX (svnw, scroll->value);
                                   LclOutlineScrollAdjust (svnw);
                                   break;

        case XmCR_INCREMENT      : LclOutlineChangeSecondaryBaseX (svnw, svnw->svn.secondary_base_x+10);
                                   LclOutlineScrollAdjust (svnw);
                                   break;

        case XmCR_DECREMENT      : LclOutlineChangeSecondaryBaseX (svnw, svnw->svn.secondary_base_x-10);
                                   LclOutlineScrollAdjust (svnw);
                                   break;

        case XmCR_PAGE_INCREMENT : LclOutlineChangeSecondaryBaseX (svnw, 
					svnw->svn.secondary_base_x+(XtWidth(svnw->svn.secondary_window_widget)/2));
				   LclOutlineScrollAdjust (svnw);
				   break;

        case XmCR_PAGE_DECREMENT : LclOutlineChangeSecondaryBaseX (svnw, 
					svnw->svn.secondary_base_x-(XtWidth(svnw->svn.secondary_window_widget)/2));
                                   LclOutlineScrollAdjust (svnw);
                                   break;
      };
}

/*
**  This routine gets all of the horizontal scroll callbacks.
*/

void OutlineHScroll (w, unused_tag, scroll)

  Widget w;
  int unused_tag;
  XmScrollBarCallbackStruct *scroll;

{
/*
**  The parent of the vertical scroll bar is the svn widget
*/
    svn_widget svnw = StructFindSvnWidget (w);


/*
**  As a workaround to the multiple callback bug in the scrollbar.  If scroll
**  is already in progress, then ignore this call and return.  Otherwise set
**  the HScrollInProgress flag in case we get called before this routine
**  completes (If we have to do a GetEntry callback the application callback
**  routine may dispatch some events).
*/
    if (svnw->svn.hscroll_in_progress) return;
    svnw->svn.hscroll_in_progress = TRUE;


/*
**  Call the TopTree routine if needed
*/
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
       {
          TopTreeHScroll (w, scroll);
	  svnw->svn.hscroll_in_progress = FALSE;
          return;
       };


/*
**  See if this is the secondary scroll bar...
*/
    if (w == svnw->svn.secondary_hscroll)
       {
          LclOutlinesecondaryHScroll (svnw, scroll);
	  svnw->svn.hscroll_in_progress = FALSE;
          return;
       };


/*
**  Switch on the reason code
*/
    switch (scroll->reason)

      {
        case XmCR_VALUE_CHANGED  : LclOutlineChangeBaseX (svnw, scroll->value);
                                   LclOutlineScrollAdjust (svnw);
                                   break;

        case XmCR_INCREMENT      : LclOutlineChangeBaseX (svnw, svnw->svn.window_basex+10);
                                   LclOutlineScrollAdjust (svnw);
                                   break;

        case XmCR_DECREMENT      : LclOutlineChangeBaseX (svnw, svnw->svn.window_basex-10);
                                   LclOutlineScrollAdjust (svnw);
                                   break;

        case XmCR_PAGE_INCREMENT : LclOutlineChangeBaseX (svnw, 
					svnw->svn.window_basex+(XtWidth(svnw->svn.primary_window_widget)/2));
                                   LclOutlineScrollAdjust (svnw);
                                   break;

        case XmCR_PAGE_DECREMENT : LclOutlineChangeBaseX (svnw, 
					svnw->svn.window_basex-(XtWidth(svnw->svn.primary_window_widget)/2));
                                   LclOutlineScrollAdjust (svnw);
                                   break;
      };
	

    svnw->svn.hscroll_in_progress = FALSE;
}

/*
**  This routine is called when the user is dragging the slider in the vertical
**  scroll bar.
*/
static void LclOutlineVScrollDrag (svnw, value)

  svn_widget svnw;
  int value;

{
    /*
    **  Local data declarations
    */
    DXmSvnEntryPtr window_entry;
    XmString cs = NULL;
    Arg arglist[5];
    int i, window_number;
    
    
    /*
    **  If we don't need an index window, then set the index_window_shown 
    **  value to non zero to show that we have done some drag operation.
    */
    if (svnw->svn.index_window_needed == FALSE)
    {
	svnw->svn.index_window_shown = 1;
	return;
    }

    if (svnw->svn.index_window == NULL)
    {
	/*
	**  The index window does not exist so create the label widget for the index.  Set XmNrecomputeSize to false so that
	**  the widget doesn't resize itself each time to hold the label string (it stays a fixed length).
	*/
	XtSetArg(arglist[0], XmNalignment, XmALIGNMENT_BEGINNING);
	XtSetArg(arglist[1], XmNborderWidth, 3);
	XtSetArg(arglist[2], XmNwidth, (XtWidth(svnw)/2));
	XtSetArg(arglist[3], XmNx, (XtWidth (svnw)/2) - 3);
	XtSetArg(arglist[4], XmNrecomputeSize, FALSE);
	svnw->svn.index_window = (Widget) XmCreateLabel (svnw, "IndexNotFound", arglist, 5);
	XtManageChild (svnw->svn.index_window);
	svnw->svn.index_window_shown = 0;
    }
    else if (svnw->svn.index_window_shown == 0)
    {
	/*
	**  The index window exists but is not displayed so map it
	*/
	XtSetArg(arglist[0], XmNwidth, (XtWidth(svnw)/2));		/* need to do this because the */
	XtSetArg(arglist[1], XmNx, (XtWidth (svnw)/2) - 3);		/* window size may have changed */
	XtSetValues(svnw->svn.index_window, arglist, 2);
	XtMapWidget (svnw->svn.index_window);
	svnw->svn.index_window_shown = 0;
    }

    /*
    **  Get to the desired entry 
    */
    window_number = value;
    window_entry = StructGetEntryPtr (svnw, window_number);

    /*
    **  Now go backwards until we find an indexable entry...
    */
    while (!window_entry->index_window && window_number > 1)
    {
	window_number--;
	window_entry = StructGetEntryPtr (svnw, window_number);
    }

    /*
    **  If this is the one currently being shown, then we are done.
    */
    if (svnw->svn.index_window_shown == window_number)
	return;
    svnw->svn.index_window_shown = window_number;

    /*
    **  Get the entry valid if necessary.  We CANNOT go into the fix screen up code.
    */
    svnw->svn.disabled_count++;
    window_entry = StructGetValidEntryPtr (svnw, window_number);
    svnw->svn.disabled_count--;

    /* Get a hold of the first indexable component in the entry */
    
    for (i = 0;  i <= window_entry->num_components-1;  i++)
	if (window_entry->entrycompPtr[i].type == DXmSvnKcompText)
	{
	    /*
	    **  Set the current value
	    */
	    cs = XmStringCopy(window_entry->entrycompPtr[i].var.is_text.text);
	    XtSetArg (arglist[0], XmNlabelString, cs);
	    XtSetValues (svnw->svn.index_window, arglist, 1);
	    XmStringFree (cs);
	    break;
	}
}

/*
**  This routine is called when the user is dragging the slider in the vertical
**  scroll bar and will perform live scrolling instead of using the index window.
*/

static void LclOutlineDragLS (svnw, value)

  svn_widget svnw;
  int value;

{
/*
**  Local data declarations
*/
    int		    new_y, basey = 0;
    DXmSvnEntryPtr  svnentry, prev_entryptr;
    int		    prev_entnum;
    int		    offset, j, i, level;
    int		    ls_num_path = 0, count;
    int		    old_ls_num_path;


    /*
    ** Save new entry number returned on scrollbar callback
    */
    svnw->svn.internal_value = value;
    old_ls_num_path = svnw->svn.ls_num_path;

#ifdef LS_EVENT
printf("Called LclOutlineDragLS\n");
#endif

    /******************************************************************************/
    /*                                                                            */
    /* Put in logic to handle path-to-root.  If we are showing the path-to-root */
    /* build a dummy array that just holds the new path to root entries based on  */
    /* this new entry number (value) being under the PTR window.		  */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.show_path_to_root)
    {
	svnentry = StructGetEntryPtr(svnw, value);
	ls_num_path = svnentry->level;
	count = ls_num_path+1;

	/******************************************************************************/
	/*                                                                            */
	/* This is a special case, if the entry (value) is at the location it wants to*/
	/* be after this scroll operation, then we are already there and no copy or   */
	/* rearrangment of entries is needed in the display array.		      */
	/*                                                                            */
	/******************************************************************************/
	if (svnw->svn.entries[count] == value)
	    if ((ls_num_path != 0) ||
		(ls_num_path == svnw->svn.num_path && svnw->svn.display_count > count) ||
		(ls_num_path == 0 && svnw->svn.num_path == 1 && value != 1))			/* level 0 entry in PTR */
	    {
#ifdef LS_PRINTF
		printf("Took return 1 in DragLS, values up to date,  value=%d\n",value);	    
		printf("ls_num_path = %d, [ls_num_path+1]=%d\n",ls_num_path,svnw->svn.entries[ls_num_path+1],offset);
#endif
		svnw->svn.internal_value = svnw->svn.current_value;
		svnw->svn.internal_x = svnw->svn.scroll_x;
		svnw->svn.internal_y = svnw->svn.scroll_y;
		return;
	    }


	/* Handle multiple level 0 entries here */

	if ((ls_num_path == 0) && (value != 1))
	{
	    prev_entryptr = StructGetEntryPtr(svnw, svnw->svn.entries[svnw->svn.num_path+1]);
	    if (prev_entryptr->level > svnentry->level)
	    {
		ls_num_path = 1;
		svnw->svn.path_entries[1] = svnw->svn.entries[1];
		svnw->svn.path_entries[2] = value;
	    }
	}
	else
	{
	    svnw->svn.path_entries[count] = value;

	    if (ls_num_path != 0)
	    {
		level = svnentry->level - 1;
		for (i = value - 1; i > 0; i--)
		{
		    svnentry = StructGetEntryPtr(svnw, i);
	    	    if (svnentry->level == level)
		    {
			svnw->svn.path_entries[level+1] = i;
			if (level == 0) break;
			level--;
		    }
		}
	    }
	}

	svnw->svn.ls_num_path = ls_num_path;
    }   /* section to handle path-to-root */


    /******************************************************************************/
    /*                                                                            */
    /* The first part of the IF statement will handle scrolling down in the display*/
    /* ...the ELSE will handle scrolling up in the display.			  */
    /*										  */
    /******************************************************************************/
    if (svnw->svn.internal_value >= svnw->svn.scroll_value)
    {
	/******************************************************************************/
	/*                                                                            */
	/* If the slider was dragged to the bottom of the scrollbar, then we need to  */
	/* just go immediately to process this as a LclOutinePositionJump in the      */
	/* graphics expose routine.						      */
	/*                                                                            */
	/******************************************************************************/
	if (DRAG_TO_BOTTOM(value))
	{
	    svnw->svn.basey = basey = PRIMARY_HEIGHT(svnw) + 1;
	    svnw->svn.internal_y += basey;

#ifdef LS_PRINTF
	    printf("SPECIAL case for bottom\n");
	    printf("num_entries-value=%d, display_count=%d\n",
		    svnw->svn.num_entries-value, svnw->svn.display_count);
#endif
	}
	else
	{	
	    /******************************************************************************/
	    /*                                                                            */
	    /* Search for this entry (represented by the callback value).. in the display */
	    /* array.									  */
	    /* The value returned in the scrollbar callback represents the approx.        */
	    /*    entry number that we want to scroll to..				  */
	    /*                                                                            */
	    /******************************************************************************/
	    for (offset = 1; ((svnw->svn.path_entries[offset] != value) &&
			    (offset <= ls_num_path)); offset++);
				    
	    if (offset > ls_num_path)
		for (offset = ls_num_path+1; ((svnw->svn.entries[offset] != value) &&
			(offset <= svnw->svn.display_count)); offset++);
	    
	    /******************************************************************************/
	    /*                                                                            */
	    /* If the specified entry that we want to scroll too is not found		  */
	    /* in the path-to-root entries OR in the display array			  */
	    /* we want the LclOutlineAdjustDisplayLS routine to clear the entire window   */
	    /* and redisplay the specified entry at the +1 position under the PTR window. */
	    /*										  */
	    /* Otherwise we have to mess with the					  */
	    /* display by scrolling the window up to that entry.. and then figure out     */
	    /* how many off-screen entries are needed to fill in the remainder of the     */
	    /* display.	Here we will figure out the amount of the screen to copy	  */
	    /* up to right under the PTR window, that value will be stored in basey.      */
	    /*                                                                            */
	    /******************************************************************************/
	    if ((offset <= ls_num_path) || (offset > svnw->svn.display_count))
		basey = PRIMARY_HEIGHT(svnw) + 1;		    /* add +1 to cause graphic expose */
	    else 
		basey = LclOutlineMapOffsetToY(svnw, offset);

	    svnw->svn.internal_y += basey;
	    svnw->svn.basey = basey;
	}
#ifdef LS_PRINTF
printf("offset = %d, ls_num_path = %d, display_count = %d\n", offset, ls_num_path, svnw->svn.display_count);
#endif
    } /* end of scrolling down */

    else  /* scrolling up */
    {
	/******************************************************************************/
	/*                                                                            */
	/* If the slider was dragged to the top of the scrollbar quickly then just go */
	/* immediately to process this as a LclOutinePositionJump in the graphics     */
	/* expose routine.							      */
	/*                                                                            */
	/******************************************************************************/
	if (DRAG_TO_TOP(value))
	{
	    basey = PRIMARY_HEIGHT(svnw) + 1;
	    svnw->svn.internal_y  -= basey;
	    svnw->svn.basey = -basey;

#ifdef LS_PRINTF
printf("SPECIAL case for top\n");
printf("num_entries-value=%d, display_count=%d\n",
	svnw->svn.num_entries-value, svnw->svn.display_count);
#endif
	}
	else
	{
	    /******************************************************************************/
	    /*                                                                            */
	    /* Create a display array for all entries that would be above the current     */
	    /* display... fill in the array with valid entries. Then in the LclOutline-   */
	    /* AdjustDisplayLS routine, we can fix up the real entries display array      */
	    /* with the correct entry numbers.					      */
	    /*                                                                            */
	    /******************************************************************************/
	    basey = 0;
	    offset = 0;

	    /* Set first entry to be at top entry - 1 */
	    prev_entnum = svnw->svn.entries[svnw->svn.num_path+1] - 1;

	    for (i = svnw->svn.display_count ; i >= svnw->svn.num_path+1 && prev_entnum > 0  ; i--)
	    {
		svnentry = StructGetEntryPtr(svnw, prev_entnum);

		/******************************************************************************/
		/*                                                                            */
		/* If this is the very first entry, or this entry number is invalid and was   */
		/* not previously fetched.. then we want to quit out of this loop.		  */
		/*                                                                            */
		/******************************************************************************/
		if (svnentry == NULL)
		    break;

		svnw->svn.up_entries[i] = prev_entnum;
		if (prev_entnum == value)
		    offset = i;

		prev_entnum--;	
	    }

	    if (offset > 0)
	    {
		int num_differ = (svnw->svn.display_count - offset) + 1;

		/*
		**  Found value in up_entries array
		**  Calculate the basey address for the XCopyArea...
		*/
		
		for (i = svnw->svn.display_count; i > (svnw->svn.display_count - num_differ); i--)
		{
		    svnw->svn.disabled_count++;
		    svnentry = StructGetValidEntryPtr(svnw, svnw->svn.up_entries[i]);
		    svnw->svn.disabled_count--;

		    basey = basey + svnentry->height;
		}
	    }
	    else
	    {
		/******************************************************************************/
		/*                                                                            */
		/* If we never find this VALUE entry number in the up_entries array then we   */
		/* know that we cannot optomize by performing a partial copy.. but must do    */
		/* a LclOutlinePositionJump in the svn_graphics_expose routine.. The way to   */
		/* that routine will know this is that the display array will not have been  */
		/* update to contain this wanted entry.. and it will automatically perform    */
		/* the PositionJump after it searches the display array.			  */
		/*                                                                            */
		/******************************************************************************/
		basey = PRIMARY_HEIGHT(svnw) + 1;		    /* add +1 to cause a graphic expose */
#ifdef LS_PRINTF
printf("Did not find entry in up_entries array\n");
#endif
	    }

	    svnw->svn.basey = -basey;
	    svnw->svn.internal_y  -= svnw->svn.basey;		    /* Save new y coordinate of current scroll position */
	}
    }	    /* end of scrolling up */

#ifdef LS_PRINTF
if (basey == 0)
    {
    printf("basey 0, ls_num_path = %d, offset = %d\n",ls_num_path,offset);
    printf("[ls_num_path]+1= %d, entries[offset]=%d\n",
	    svnw->svn.entries[ls_num_path]+1,svnw->svn.entries[offset]);
    }
printf("#1 - basey = %d, internal_value = %d\n",svnw->svn.basey,
	svnw->svn.internal_value);
#endif

    if (!svnw->svn.grop_pending)
	LclOutlineAdjustDisplayLS(svnw);

}    /* End of LclOutlineDragLS */


/******************************************************************************/
/*                                                                            */
/* This routine will do the actual partial copy of the screen for live-	      */
/* scrolling...and adjust the entries in the display array according.	      */
/* This routine is called from two places, the LclOutlineDragLS scrollbar     */
/* action routine, and the svn_graphics_expose routine.			      */
/*                                                                            */
/******************************************************************************/
void LclOutlineAdjustDisplayLS(svnw)
svn_widget svnw;
{
    int		    *x,*y, *internal_x, *internal_y, *scroll_value, *internal_value, *basex, *basey;
    DXmSvnEntryPtr  lastentry, svnentry;
    int		    offset, j, i, old_display_count;

    x = (int *) &svnw->svn.scroll_x;
    y = (int *) &svnw->svn.scroll_y;
    internal_x = (int *) &svnw->svn.internal_x;
    internal_y = (int *) &svnw->svn.internal_y;
    basex = (int *) &svnw->svn.basex;
    basey = (int *) &svnw->svn.basey;

#ifdef LS_EVENT
printf("Called LclOutlineAdjustDisplayLS\n");
#endif    
    /******************************************************************************/
    /*                                                                            */
    /* Here we do the actual CopyArea of a partial section of the display.	  */
    /*                                                                            */
    /******************************************************************************/
    XCopyArea (XtDisplay(svnw),
	       XtWindow(svnw->svn.primary_window_widget), 
	       XtWindow(svnw->svn.primary_window_widget), 
	       svnw->svn.copyarea_gc, 
	       *basex,
	       *basey, 
	       XtWidth(svnw->svn.primary_window_widget), 
	       XtHeight(svnw->svn.primary_window_widget), 
	       0,
	       0
	       );

    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	XCopyArea (XtDisplay(svnw),
		   XtWindow(svnw->svn.secondary_window_widget), 
		   XtWindow(svnw->svn.secondary_window_widget), 
		   svnw->svn.copyarea_gc, 
		   *basex,
		   *basey, 
		   XtWidth(svnw->svn.secondary_window_widget),
		   XtHeight(svnw->svn.secondary_window_widget), 
		   0,
		   0
		   );

#ifdef LS_PRINTF
printf("#2 - XCopyArea for this basey %d \n",*basey);
XFlush(XtDisplay(svnw));    /* For testing purposes */ 
#endif

    *x = *internal_x;
    *y = *internal_y;
    svnw->svn.current_value = svnw->svn.internal_value;		
		
#ifdef LS_EVENT
printf("Generating NoExpose\n");
#endif	
    /* Generate a NoExpose event to prevent overrun of exposures */
    XCopyArea(
	    XtDisplay(svnw),
	    XtWindow(svnw->svn.primary_window_widget),
	    XtWindow(svnw->svn.primary_window_widget),
	    svnw->svn.copyarea_gc, 
	    0,0,0,0,0,0);

    svnw->svn.grop_pending = TRUE;

    old_display_count = svnw->svn.display_count;

    /******************************************************************************/
    /*                                                                            */
    /* Process this as a LclOutinePositionJump in the graphics expose routine for */
    /* the following conditions:						  */
    /*   1. The slider was dragged to the top of the scrollbar quickly		  */
    /*   2. The slider was dragged to the bottom of the scrollbar		  */
    /*   3. The height to copy is higher than the primary window		  */
    /*                                                                            */
    /******************************************************************************/
    if (
	((svnw->svn.current_value < svnw->svn.scroll_value) && DRAG_TO_TOP(svnw->svn.current_value)) ||
	((svnw->svn.current_value >= svnw->svn.scroll_value) &&	DRAG_TO_BOTTOM(svnw->svn.current_value)) ||
	(abs(*basey) > PRIMARY_HEIGHT(svnw))
	)
	return;

    /******************************************************************************/
    /*                                                                            */
    /* Update the display array ..						  */
    /* Search for this entry (represented by the callback value).. in the display */
    /* array.								          */
    /* The value returned in the scrollbar callback represents the approx.        */
    /*    entry number that we want to scroll to..				  */
    /*                                                                            */
    /* First handle the path-to-root management... setting new entries in         */
    /* the PTR window into the display array if they are different.		  */
    /*										  */
    /* If the PTR window got smaller than we have to close up the display array   */
    /* by that many entries, so that it corresponds with the tru size of the      */
    /* the number of entries visible.						  */
    /*                                                                            */
    /* If the PTR window got larger than during the previous scroll operation     */
    /* we have to open up the display array and add the new entries in the	  */
    /* path to the display array.						  */
    /*										  */
    /* Finally  ...								  */
    /* See if any of the PTR entries need to be refreshed, if so, then just       */
    /* set invalid and let the drawing expose routines refresh those entries.     */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.show_path_to_root)
    {
	if (svnw->svn.ls_num_path == 0)
	{
	    svnw->svn.num_path = svnw->svn.ls_num_path;
	    /******************************************************************************/
	    /*                                                                            */
	    /* If the PTR window was previously managed, and now we do not want any       */
	    /* entries in the PTR, then we must unmanage the PTR window.		      */
	    /*                                                                            */
	    /******************************************************************************/
	    if (XtIsManaged(svnw->svn.primary_ptr_widget))
	    {
		if (svnw->svn.sub_widgets_used)
		    LclOutlineHideWidgets(svnw, 1, svnw->svn.display_count);

		LclOutlineUnmanagePTR(svnw);
	    }
	}
	else
	{	
	    if (svnw->svn.ls_num_path < svnw->svn.num_path)
	    {
		for (i = svnw->svn.num_path; i > svnw->svn.ls_num_path; i--)
		{
#ifdef LS_PRINTF
	printf("PTR window getting smaller, closing up entries\n");
#endif
		    /* Unmap any sub-widgets if they are used */
		    if (svnw->svn.sub_widgets_used)
		    {
			int compn;

			svnentry = StructGetEntryPtr(svnw, svnw->svn.entries[i]);
			for (compn = 0;  compn <= svnentry->num_components-1; compn++)
			    if (svnentry->entrycompPtr[compn].type == DXmSvnKcompWidget)
				HIDE_WIDGET (svnentry->entrycompPtr[compn].var.is_widget.readwrite_text);
		    }

		    StructCloseArray(svnw->svn.entries, sizeof(short), svnw->svn.display_count+1, i);
		    StructCloseArray (svnw->svn.display_invalid, sizeof(Dimension),	svnw->svn.display_count+1, i);
		    svnw->svn.display_count--;
#ifdef SYNC_ON_SCROLL
	svn_sync(svnw);
#endif
		}
	    }
	    else if (svnw->svn.ls_num_path > svnw->svn.num_path)
	    {
		for (i = svnw->svn.num_path+1; i <= svnw->svn.ls_num_path; i++)
		{
		    StructOpenArray(svnw->svn.entries, sizeof(short),svnw->svn.display_count+2, i);
		    StructOpenArray (svnw->svn.display_invalid, sizeof(Dimension),svnw->svn.display_count+2, i);
		    svnw->svn.display_count++;
		    svnentry = StructGetEntryPtr (svnw, svnw->svn.path_entries[i]);
		    svnw->svn.display_invalid[i] = svnentry->height;	    /* Make invalid so that it gets redrawn */
		    svnw->svn.entries[i] = svnw->svn.path_entries[i];
#ifdef LS_PRINTF
	printf("PTR window getting larger, opening up entries\n");
#endif
		}
	    }

	    for (i = 1; i <= svnw->svn.ls_num_path; i++)
		if ((svnw->svn.entries[i] != svnw->svn.path_entries[i]))
		{
		    svnentry = StructGetEntryPtr (svnw, svnw->svn.path_entries[i]);
		    svnw->svn.display_invalid[i] = svnentry->height;	    /* Make invalid so that it gets redrawn */
		    svnw->svn.entries[i] = svnw->svn.path_entries[i];	
		}


	    /******************************************************************************/
	    /*                                                                            */
	    /* If the # of entries in the new path-to-root != # entries in old-path-to    */
	    /* root, then reset the size of the PTR windows.			      */
	    /*                                                                            */
	    /******************************************************************************/
	    if (svnw->svn.ls_num_path != svnw->svn.num_path)
	    {
		svnw->svn.num_path = svnw->svn.ls_num_path;

		if (svnw->svn.sub_widgets_used)
		    LclOutlineHideWidgets(svnw, svnw->svn.num_path + 1, svnw->svn.display_count);

		LclOutlineResizePTR(svnw);

#ifdef LS_EVENT
printf("LclOutlineAdjustDisplayLS calling LclOutlineResizePTR\n");
#endif
	    }

	    /******************************************************************************/
	    /*                                                                            */
	    /* If there will be entries in the path-to-root and the path-to-root window   */
	    /* is NOT managed at this time, manage it.				      */
	    /*                                                                            */
	    /******************************************************************************/
	    if ((svnw->svn.show_path_to_root) && (!XtIsManaged(svnw->svn.primary_ptr_widget)))
	    {
		if (svnw->svn.sub_widgets_used)
		    LclOutlineHideWidgets(svnw, 1, svnw->svn.display_count);

		LclOutlineManagePTR(svnw);
	    }
	}
    }

    if (svnw->svn.current_value >= svnw->svn.scroll_value)	    /* Handle DOWN scrolling */
    {
	for (offset = 1; ((svnw->svn.entries[offset] != svnw->svn.current_value) &&
			(offset <= svnw->svn.num_path)); offset++);
				
	if (offset > svnw->svn.num_path)
	    for (offset = svnw->svn.num_path + 1; ((svnw->svn.entries[offset] != svnw->svn.current_value) &&
		(offset <= svnw->svn.display_count)); offset++);
	
	for (j = offset-1; j>= svnw->svn.num_path+1; j--)  /* Don't close up the entry that the callback came in on */
	{
	    /******************************************************************************/
	    /*                                                                            */
	    /* Unmap any subwidget that might be part of the entries being closed down    */
	    /*                                                                            */
	    /******************************************************************************/
	    if (svnw->svn.sub_widgets_used)
	    {
		int compn;
		svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[j]);

		for (compn = 0;  compn <= svnentry->num_components-1;  compn++)
		    if (svnentry->entrycompPtr[compn].type == DXmSvnKcompWidget)
			 HIDE_WIDGET (svnentry->entrycompPtr[compn].var.is_widget.readwrite_text);
	    }
		
	    StructCloseArray(svnw->svn.entries, sizeof(short), svnw->svn.display_count+1, j);
	    StructCloseArray (svnw->svn.display_invalid, sizeof(Dimension),	svnw->svn.display_count+1, j);
	    svnw->svn.display_count--;
	}
	
	/******************************************************************************/
	/*                                                                            */
	/* Now if any of the entries that were copied had sub-widgets.. we must MOVE  */
	/* the widgets to their new locations, before we complete this scroll.	      */
	/*                                                                            */
	/******************************************************************************/
	if (svnw->svn.sub_widgets_used)
	    LclOutlineMoveWidgets(svnw, svnw->svn.num_path + 1, svnw->svn.display_count, - svnw->svn.basey);

	/* Now fill in as many entries as will fit */
    
	while (svnw->svn.entries[svnw->svn.display_count] < svnw->svn.num_entries)
	    if (LclOutlineEntryAppendLS (svnw, svnw->svn.entries[svnw->svn.display_count] + 1) == FALSE)
		break;

#ifdef SYNC_ON_SCROLL
    svn_sync(svnw);
#endif
    }

    else					        /* Now handle UP scrolling */
    {
	/******************************************************************************/
	/*                                                                            */
	/* First thing we want to do, is to close up the entries that will fall off   */
	/* of the display.. because of the up scroll.  Then we have to open up the    */
	/* display array to handle the new entries that we want to display.	      */
	/*                                                                            */
	/******************************************************************************/
	for (offset = old_display_count ; offset >= svnw->svn.num_path + 1 ; offset--)
	{
	    if (svnw->svn.up_entries[offset] == svnw->svn.current_value)
	    {
		int num_differ = (old_display_count - offset) + 1;
		int num_entries_added = 0;
		
		/*
		**  Add new entries to array
		*/
		for (i = svnw->svn.num_path + 1; i <= svnw->svn.num_path + num_differ; i++)
		{
		    if (svnw->svn.entries[i] != svnw->svn.up_entries[offset + (i - svnw->svn.num_path) - 1])
		    {
			StructOpenArray(svnw->svn.entries, sizeof(short),svnw->svn.display_count+2, i);
			StructOpenArray (svnw->svn.display_invalid, sizeof(Dimension),svnw->svn.display_count+2, i);

			svnw->svn.entries[i] = svnw->svn.up_entries[offset + (i - svnw->svn.num_path) - 1];
			svnw->svn.display_invalid[i] = 1;

			svnw->svn.display_count++;
			num_entries_added++;
		    }
		}

		/*
		**  Remove entries that are below the bottom of the main window
		*/
		for (i = svnw->svn.display_count; i > 0 && LclOutlineMapOffsetToY(svnw, i) > PRIMARY_HEIGHT(svnw); i--)
		{
		    /* Unmap any sub-widgets if they are used */
		    if (svnw->svn.sub_widgets_used)
		    {
			int compn;

			svnentry = StructGetEntryPtr(svnw, svnw->svn.entries[i]);
			for (compn = 0;  compn <= svnentry->num_components-1; compn++)
			    if (svnentry->entrycompPtr[compn].type == DXmSvnKcompWidget)
				HIDE_WIDGET (svnentry->entrycompPtr[compn].var.is_widget.readwrite_text);
		    }
			
		    StructCloseArray(svnw->svn.entries, sizeof(short), svnw->svn.display_count+1, i);
		    StructCloseArray(svnw->svn.display_invalid, sizeof(Dimension), svnw->svn.display_count+1, i);
		    svnw->svn.display_count--;
		}

		/******************************************************************************/
		/*                                                                            */
		/* Now if any of the entries that were copied had sub-widgets.. we must MOVE  */
		/* the widgets to their new locations, before we complete this scroll.	      */
		/*                                                                            */
		/******************************************************************************/
		if (svnw->svn.sub_widgets_used)
		    LclOutlineMoveWidgets(svnw, svnw->svn.num_path + 1 + num_entries_added, svnw->svn.display_count, - *basey);

		break;
	    }	/* found value in up_entries array */
	}   /* end for */

    }   /* End of section for scrolling up */	

} /* End of LclOutlineAdjustDisplayLS */   	


/*
**  This routine processes the top and bottom buttons.
**
**       1 - MB1 on top button
**       2 - SHIFT/MB1 on top button
**       3 - MB1 on bottom button
**       4 - SHIFT/MB1 on bottom button
*/

void LclOutlineScrollButton (svnw)

    svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int entry_number, entry_level;


/*
**  Just call the corresponding tree routine if the display_mode is Tree.
*/
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	{
	TopTreeScrollButton(svnw);
	return;
	}

    /*********************************************************************/
    /* If the user has resized the window so small such that the         */
    /* display_count is 0 then there is nothing to adjust in this case,  */
    /* so return.  This fixes an accvio that previously occurred below   */
    /* when trying to calculate svnentry.                                */
    /*********************************************************************/
    if (svnw->svn.display_count == 0)
        return;


/*
**  If it's the top button and we're already at the top, then leave
*/
    if (svnw->svn.button_top <= 2)
       if (svnw->svn.entries[svnw->svn.num_path+1] == 1)
          return;


/*
**  If it's the bottom button and we're already at the bottom, then leave
*/
    if (svnw->svn.button_top > 2)
       if (LclOutlineAtBottom (svnw))
          return;


/*
**  Start the entry pointers at the top or the bottom entry.
*/
    if (svnw->svn.button_top <= 2) entry_number = svnw->svn.entries[svnw->svn.num_path+1];
    else entry_number = svnw->svn.entries[svnw->svn.display_count];


/*
**  Get the entry
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If we are looking at the last entry and it is partially displayed, then
**  back off by one entry.
*/
    if ((entry_number == svnw->svn.entries[svnw->svn.display_count]) &&
	 (entry_number != 1))		/* and this isn't the only entry in the display */
       {
         Position y = LclOutlineMapOffsetToY (svnw, svnw->svn.display_count);
         if (y + svnentry->height > PRIMARY_HEIGHT(svnw))
            {
              entry_number = entry_number - 1;
              svnentry = StructGetEntryPtr (svnw, entry_number);
            };
       };


/*
**  Get the entry's level
*/
    entry_level = svnentry->level;


    DisplaySetWatch(svnw, TRUE);
    
/*
**  Compute the entry number for the top button without the shift key
*/
    if (svnw->svn.button_top == 1)
       {
        /*
        **  Go backwards looking for the first entry whose level is different
        */
            if (svnw->svn.index_window_needed == TRUE)
               while (TRUE)
                  {
                    if (entry_number == 1) break;
                    svnentry = svnentry->prev;  entry_number--;
                    if (svnentry->index_window == TRUE) break;
                  };

        /*
        **  Go backwards looking for the first entry whose level is different
        */
            if (svnw->svn.index_window_needed == FALSE)
               while (TRUE)
                  {
                    if (entry_number == 1) break;
                    svnentry = svnentry->prev;  entry_number--;
                    if (svnentry->level != entry_level) break;
                  };
       };


/*
**  Compute the entry number for the top button with the shift key
*/
    if (svnw->svn.button_top == 2)
       while (TRUE)
          {
            if (entry_number == 1) break;
            svnentry = svnentry->prev;  entry_number--;
            if (svnentry->level == 0) break;
          };


/*
**  Bottom button, no shift key...
*/
    if (svnw->svn.button_top == 3)
       {
         /*
         **  Go forward looking for the next expandable entry
         */
             if (svnw->svn.index_window_needed == TRUE)
                while (TRUE)
                  {
                    if (entry_number == svnw->svn.num_entries) break;
                    svnentry = svnentry->next;  entry_number++;
                    if (svnentry->index_window == TRUE) break;
                  };
 

         /*
         **  Go forwards looking for the first entry whose level is different
         */
             if (svnw->svn.index_window_needed == FALSE)
                while (TRUE)
                  {
                    if (entry_number == svnw->svn.num_entries) break;
                    svnentry = svnentry->next;  entry_number++;
                    if (svnentry->level != entry_level) break;
                  };
       };


/*
**  Shift Bottom button
*/
    if (svnw->svn.button_top == 4)
       while (TRUE)
          {
            if (entry_number == svnw->svn.num_entries) break;
            svnentry = svnentry->next;  entry_number++;
            if (svnentry->level == 0) break;
          };


/*
**  Finish up with a position top or position bottom call.
*/
    if (svnw->svn.button_top <= 2) 
         OutlinePositionDisplay (svnw, entry_number, DXmSvnKpositionTop);
    else OutlinePositionDisplay (svnw, entry_number, DXmSvnKpositionBottom);

    DisplaySetWatch(svnw, FALSE);
}

/*
**  This routine is called when the user presses the button on the outer parts
**  of the scrollbar.  
*/

void OutlineScrollButtonDown (w, unused_event, argv)

    Widget       w;
    XButtonEvent *unused_event;
    char **argv;

{
/*
**  Local data declarations
*/
    svn_widget svnw = StructFindSvnWidget (w);


/*
**  Save this event information
*/
    svnw->svn.button_down = TRUE;
    svnw->svn.button_top  = atoi(*argv);


/*
**  Process the button
*/
    LclOutlineScrollButton (svnw);
    

/*
**  Set the scrolling timer...
*/
    svnw->svn.button_timerid = XtAppAddTimeOut (XtWidgetToApplicationContext((Widget)svnw), svnw->svn.button_waitms, 
                                                (XtTimerCallbackProc) LclOutlineScrollTimeout, svnw);
}

/*
**  This routine is called when the user releases the button on the scrollbar.
*/

void OutlineScrollButtonUp  (w)

    Widget w;

{
/*
**  Local data declarations
*/
    svn_widget svnw = StructFindSvnWidget (w);


/*
**  Save this event information
*/
    svnw->svn.button_down = FALSE;


/*
**  Cancel the timer...
*/
    if (svnw->svn.button_timerid != 0)
       {
         XtRemoveTimeOut(svnw->svn.button_timerid);
         svnw->svn.button_timerid = 0;
       };
}

/*
**  This routine will get called when the timeout on scroll buttons happen
*/

static void LclOutlineScrollTimeout (svnw)

    svn_widget svnw;

{
/*
**  If the button is not down, then leave
*/
    if (svnw->svn.button_down == FALSE)  return;


/*
**  Process the button
*/
    LclOutlineScrollButton (svnw);


/*
**  Reset the scrolling timer...
*/
    svnw->svn.button_timerid = XtAppAddTimeOut (XtWidgetToApplicationContext((Widget)svnw), svnw->svn.button_repeatms, 
                                                (XtTimerCallbackProc)LclOutlineScrollTimeout, svnw);
}        

void LclOutlineDrawColumnLines(svnw)

svn_widget svnw;

{
    int comp_offset = svnw->svn.start_column_component - 1;
    int column_x = 0 - svnw->svn.secondary_base_x;

/*
**  If this is columns mode, then redraw the lines on the right hand side.
*/
       if (svnw->svn.start_column_component != 0)
          if (svnw->svn.column_lines)
           {

	    /*
	    **  Loop on all possible right hand side components.  We will stop

            **  this loop when we see a column width of zero or until we go off
            **  the right side.
	    */
		while (TRUE)
		  {
		    /*
		    **  If this component has a width of zero, then leave.
		    */
		        if (svnw->svn.column_widths [comp_offset] == 0) break;


		    /*
		    **  If the next component has a width of zero, then leave.
		    */
		        if (svnw->svn.column_widths [comp_offset + 1] == 0) break;


 		    /*
		    **  Add in the width of this component.  We are drawing lines
                    **  after the components because we do not have one in front
		    **  of the first secondary component
		    */
	                column_x = column_x + svnw->svn.column_widths [comp_offset] + svnw->svn.default_spacing;
                        

                    /*
		    **  If we are beyond the window, then leave.
		    */
		        if (column_x > XtWidth (svnw->svn.secondary_window_widget)) break;


		    /*
                    **  If we are in the window, then draw the line.
		    */
	                if (column_x > 0)
                           XDrawLine (XtDisplay(svnw),
	                              XtWindow(svnw->svn.secondary_window_widget),
	                              svnw->svn.foreground_gc, 
                                      ( (LayoutIsRtoL(svnw)) ? XtWidth(svnw->svn.secondary_window_widget) - column_x : column_x),
                                      0,
                                      ( (LayoutIsRtoL(svnw)) ? XtWidth(svnw->svn.secondary_window_widget) - column_x : column_x),
                                      XtHeight(svnw->svn.secondary_window_widget));

 
                    /*
                    **  Increment to the next component and the next column
                    */
                        comp_offset++;
                  };
           };

}

static void LclOutlineEraseLocationCursor(svnw, window_widget)

svn_widget svnw;
Widget	   window_widget;

    {
    GC gc;
    int width,i;
    Position basex, basey, ptr_height;
    DXmSvnEntryPtr svnentry;
    DXmSvnCompPtr  comp;
    

    /*  Calculate out the starting x base position for the location cursor */
     
    basex = svnw->svn.margin_width +
		(svnw->svn.location_cursor_entry->level * svnw->svn.indent_margin) - svnw->svn.window_basex;

    /* Calculate the Y position value */

    ptr_height = XtIsManaged(svnw->svn.primary_ptr_widget) ? XtHeight(svnw->svn.primary_ptr_widget) : 0;
    basey = 0;

    for (i = 1; ((svnw->svn.entries[i] != svnw->svn.location_cursor) &&
		 (i <= svnw->svn.display_count)); i++)
	{
	svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
	basey = basey + svnentry->height;
	}

    if ((XtIsManaged(svnw->svn.primary_ptr_widget) && (i > svnw->svn.num_path)))
	basey = basey - ptr_height;

    /******************************************************************************/
    /*                                                                            */
    /* Clear out the location cursor from the primary window widget	          */
    /*                                                                            */
    /******************************************************************************/
    if (window_widget == svnw->svn.primary_window_widget)
    {
   

    /* Get the correct GC */
    
    if (svnw->svn.location_cursor_entry->selected)
	gc = svnw->svn.foreground_gc;
    else
	gc = svnw->svn.background_gc;


    /* Get the width of the entry */
    
    if (svnw->svn.fixed_width)
	width = XtWidth(svnw->svn.primary_window_widget) - basex;
    else
	width = svnw->svn.location_cursor_entry->width;

    	    
    XDrawRectangle (XtDisplay(svnw),
		    XtWindow(svnw->svn.primary_window_widget),
		    gc,
		    (Position)basex,
		    (Position)basey,	    
		    (Dimension)width,
		    (Dimension)svnw->svn.location_cursor_entry->height-1);
    
    }
    else
	{
	/******************************************************************************/
	/*                                                                            */
	/* If we are in column mode, when we have to handle the fact that the         */
	/* location cursor is drawn around each component of the entry.  Loop thru    */
	/* all components in the entry in clear the location cursor from all the      */
	/* components.								      */
	/*                                                                            */
	/******************************************************************************/
	int comp_number, max_comp_number, column_x;
	GC loc_gc;
	Position x, compx;

	
	if (svnw->svn.start_column_component == 0) return;

	svnentry = svnw->svn.location_cursor_entry;
	
	comp_number = svnw->svn.start_column_component;
	max_comp_number = svnentry->num_components;

	/*
	**  The column_x coordinate starts at 1/2 of the default spacing
	*/
	column_x = svnw->svn.default_spacing / 2;
    

	/*
	**  Loop through the components
	*/
	while (comp_number <= max_comp_number)
	    {
	  
	    comp = &svnentry->entrycompPtr[comp_number-1];

	    /*
	    **  Determine the component real x
	    **  If RtoL, reverse X-coordinate.
	    */
	    compx = svnw->svn.secondary_base_x + column_x;
            if (LayoutIsRtoL(svnw))
		compx = XtWidth(svnw->svn.secondary_window_widget) - compx
                                - svnw->svn.column_widths[comp_number-1];

	    
	    /******************************************************************************/
	    /*                                                                            */
	    /* If this entry is selected we assign the foreground GC, if and if this	  */
	    /* component needs to be selected.						  */
	    /*                                                                            */
	    /******************************************************************************/
	    loc_gc = svnw->svn.background_gc;
	    
	    if (svnentry->selected)
		{
		if (svnw->svn.selection_mode == DXmSvnKselectEntry )
		    loc_gc = svnw->svn.foreground_gc;
		else
		    if (comp_number == svnentry->selected_comp)
			loc_gc = svnw->svn.foreground_gc;
		}

	    /* Calculate out x starting point of location cursor */
		
	    x = compx + ( (LayoutIsRtoL(svnw)) ?
		        svnw->svn.column_widths[comp_number-1] + (svnw->svn.default_spacing / 2)
                        : - (svnw->svn.default_spacing / 2));

	    /* Calculate out width of location cursor */
		
	    if (svnw->svn.selection_mode == DXmSvnKselectEntry)
		width = XtWidth (svnw->svn.secondary_window_widget) - x; 
	    else 
		width = svnw->svn.column_widths[comp_number-1] + svnw->svn.default_spacing;
		
					  
	    XDrawRectangle (XtDisplay(svnw),
			    XtWindow(svnw->svn.secondary_window_widget),
			    loc_gc,
			    (Position)x,
			    (Position)basey,
			    (Dimension)width,
			    (Dimension)svnentry->height-1);


	    comp_number++;
	    
	    }	/* end of while loop */

	} /* end of erase cursor for secondary window widget */ 

    }	/* Endof LclOutlineEraseLocationCursor routine */


void OutlineUnmanageSecPTR(svnw)
svn_widget svnw;
{
#ifdef LS_EVENT
printf("Called OutlineUnmanageSecPTR\n");
#endif

    svnw->svn.widget_list[0] = (Widget) svnw->svn.secondary_ptr_widget;
    svnw->svn.widget_list[1] = (Widget) svnw->svn.secondary_separator_widget;
    XtUnmanageChildren(svnw->svn.widget_list, 2);

    XtVaSetValues(svnw->svn.secondary_window_widget,
		  XmNtopAttachment, XmATTACH_FORM,
		  NULL);

    LclOutlineForceExpose(svnw);

}   /* End of OutlineUnmanageSecPTR */



void OutlineManageSecPTR(svnw)
svn_widget svnw;
{
    DXmSvnEntryPtr svnentry;
    int i,y = 0;
    
#ifdef LS_EVENT
printf("Called OutlineManageSecPTR\n");
#endif

    /******************************************************************************/
    /*                                                                            */
    /* Attach the secondary to the secondary separator if the display mode is     */
    /* columns.									  */
    /*                                                                            */
    /******************************************************************************/
    XtVaSetValues(svnw->svn.secondary_window_widget,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, svnw->svn.secondary_separator_widget,
		  NULL);
    
    /******************************************************************************/
    /*                                                                            */
    /* Figure out what the size of the PTR window should be, based on the num-path*/
    /* value.									  */
    /*                                                                            */
    /******************************************************************************/
    for (i = 1; i <= svnw->svn.num_path; i++)
	{
	svnentry = StructGetValidEntryPtr(svnw, svnw->svn.entries[i]);
	y += svnentry->height;
	}

    /*
    ** Set the height of the path-to-root windows and manage
    ** the path-to-root and separator widgets.
    */
    XtVaSetValues(svnw->svn.secondary_ptr_widget,
		  XmNheight, y,
		  NULL);

    svnw->svn.widget_list[0] = (Widget) svnw->svn.secondary_ptr_widget;
    svnw->svn.widget_list[1] = (Widget) svnw->svn.secondary_separator_widget;
    XtManageChildren(svnw->svn.widget_list, 2);

    LclOutlineForceExpose(svnw);

}   /* End of OutlineManageSecPTR */

static void LclOutlineUnmanagePTR(svnw)
svn_widget svnw;
{
#ifdef LS_EVENT
printf("Called LclOutlineUnmanagePTR\n");
#endif

    svnw->svn.widget_list[0] = (Widget) svnw->svn.primary_ptr_widget;
    svnw->svn.widget_list[1] = (Widget) svnw->svn.primary_separator_widget;
    XtUnmanageChildren(svnw->svn.widget_list, 2);

    XtVaSetValues(svnw->svn.primary_window_widget,
		  XmNtopAttachment, XmATTACH_FORM,
		  NULL);

    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	OutlineUnmanageSecPTR(svnw);

    LclOutlineForceExpose(svnw);

}   /* End of LclOutlineUnmanagePTR */



static void LclOutlineManagePTR(svnw)
svn_widget svnw;
{
    DXmSvnEntryPtr svnentry;
    int i,y = 0;
    
#ifdef LS_EVENT
printf("Called LclOutlineManagePTR\n");
#endif

    /******************************************************************************/
    /*                                                                            */
    /* Now attach the primary window to the separator.  Attach the secondary	  */
    /* to the secondary separator if the display mode is columns.		  */
    /*                                                                            */
    /******************************************************************************/
    XtVaSetValues(svnw->svn.primary_window_widget,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, svnw->svn.primary_separator_widget,
		  NULL);
    
    /******************************************************************************/
    /*                                                                            */
    /* Figure out what the size of the PTR window should be, based on the num-path*/
    /* value.									  */
    /*                                                                            */
    /******************************************************************************/
    for (i = 1; i <= svnw->svn.num_path; i++)
	{
	svnentry = StructGetValidEntryPtr(svnw, svnw->svn.entries[i]);
	y += svnentry->height;
	}

    /*
    ** Set the height of the path-to-root windows and manage
    ** the path-to-root and separator widgets.
    */
    XtVaSetValues(svnw->svn.primary_ptr_widget,
		XmNheight, y,
		NULL);

    svnw->svn.widget_list[0] = (Widget) svnw->svn.primary_ptr_widget;
    svnw->svn.widget_list[1] = (Widget) svnw->svn.primary_separator_widget;
    XtManageChildren(svnw->svn.widget_list, 2);

    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	OutlineManageSecPTR(svnw);

    LclOutlineForceExpose(svnw);

}   /* End of LclOutlineManagePTR */

/*
**  Resize the Path-To-Root window.  This function assumes that svnw->svn.num_path and
**  svnw->svn.entries fields are valid.
*/
static void LclOutlineResizePTR (svnw)
svn_widget svnw;
{
    DXmSvnEntryPtr svnentry;
    int i, ptr_height = 0;
	
    /*
    **  Calculate the height of the path-to-root window
    */
    for (i = 1; i <= svnw->svn.num_path; i++)
    {
	svnentry = StructGetEntryPtr(svnw, svnw->svn.entries[i]);
	ptr_height += svnentry->height;
    }

#ifdef LS_EVENT
printf("Called LclOutlineResizePTR, grop = %s,  num_path = %d, BEFORE: width = %d, height = %d",
       (svnw->svn.grop_pending ? "TRUE" : "FALSE"), svnw->svn.num_path,
       svnw->svn.primary_ptr_widget->core.width, svnw->svn.primary_ptr_widget->core.height);
#endif

    XtVaSetValues(svnw->svn.primary_ptr_widget, XmNheight, ptr_height, NULL);
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	XtVaSetValues(svnw->svn.secondary_ptr_widget, XmNheight, ptr_height, NULL);

#ifdef LS_EVENT
printf("  AFTER: width = %d, height = %d\n",
       svnw->svn.primary_ptr_widget->core.width, svnw->svn.primary_ptr_widget->core.height);
#endif

    LclOutlineForceExpose(svnw);

}


static void LclOutlineForceExpose(svnw)
svn_widget svnw;
{
    XEvent event;
    Display * display = XtDisplay(svnw);

    XSync (display, FALSE);

    while (XCheckTypedEvent(display, Expose, &event))
	XtDispatchEvent(&event);
}

/******************************************************************************/
/*                                                                            */
/* Just for testing purposes... Will Sync up the svn display		      */
/*                                                                            */
/******************************************************************************/
void svn_sync(svnw)

svn_widget svnw;
{
    XSync(XtDisplay(svnw),FALSE);
}

/*
**  Function to move a group of subwidgets.  It turns out that because of the
**  way the form widget handles geometry, it is more efficient to unmanage
**  the widgets before moving them.
*/
static void LclOutlineMoveWidgets (svnw, start_entry, end_entry, move_amount)
svn_widget svnw;
int start_entry;
int end_entry;
int move_amount;
{
    Cardinal num_children;
    int num_found, i, j, new_y;
    DXmSvnEntryPtr svnentry;

    num_found = 0;
    for (i = start_entry;  i <= end_entry;  i++)
    {
	svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
	for (j = 0;  j <= svnentry->num_components-1;  j++)
	    if (svnentry->entrycompPtr[j].type == DXmSvnKcompWidget)
	    {
		svnw->svn.widget_list[num_found] = (Widget) svnentry->entrycompPtr[j].var.is_widget.readwrite_text;
		num_found++;
	    }
    }
    if (num_found > 20)
	XtUnmanageChildren(svnw->svn.widget_list, num_found);

    for (i = 0; i < num_found; i++)
    {
	/*
	**  MOVE_WIDGET_VERTICAL expects the y value to be the amount of offset from the top
	**  of the widget that the subwidget is attached too.  In this case this is the primary
	**  window.  Calculate the offset from the top of the primary window widget.
	*/
	new_y = XtY(svnw->svn.widget_list[i]) - XtY(svnw->svn.primary_window_widget) + move_amount;
	MOVE_WIDGET_VERTICAL(svnw->svn.widget_list[i], new_y);
    }

    if (num_found > 20)
	XtManageChildren(svnw->svn.widget_list, num_found);

}

void LclOutlineHideWidgets (svnw, start_entry, end_entry)
svn_widget svnw;
int start_entry;
int end_entry;
{
    Cardinal num_children;
    int num_found, i, j;
    DXmSvnEntryPtr svnentry;

    num_found = 0;
    for (i = start_entry;  i <= end_entry;  i++)
    {
	svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);
	for (j = 0;  j <= svnentry->num_components-1;  j++)
	    if (svnentry->entrycompPtr[j].type == DXmSvnKcompWidget)
	    {
		svnw->svn.widget_list[num_found] = (Widget) svnentry->entrycompPtr[j].var.is_widget.readwrite_text;
		num_found++;
	    }
    }

    if (num_found > 0)
	XtUnmanageChildren(svnw->svn.widget_list, num_found);

}

/*
**  Debug function to dump list of managed children of a widget
*/
static void LclOutlineDumpManaged(w)
Widget w;
{
    Widget widget;
    WidgetList children;
    int i;
    Cardinal num_children;
    int num_managed_sub = 0;
    int num_managed_other = 0;

    XtVaGetValues(w, XmNnumChildren, &num_children, XmNchildren, &children, NULL);
    for (i = 0; i < num_children; i++)
    {
	widget = children[i];
	    if (XtIsManaged(widget))
	    {
		if (XtIsSubclass((Widget)widget, (WidgetClass)dxmSvnWindowWidgetClass) || XmIsSeparatorGadget(widget))
		    num_managed_other++;
		else
		{
		    printf("widget = %d, name = %s\n", widget->core.self, widget->core.name);
		    num_managed_sub++;
		}
	    }
    }
    printf("number children = %d, number managed subwidgets = %d, number managed other = %d\n",
	   num_children, num_managed_sub, num_managed_other);
}
