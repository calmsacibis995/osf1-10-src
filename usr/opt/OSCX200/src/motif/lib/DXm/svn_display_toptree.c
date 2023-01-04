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
#define SYNC_ON_SCROLL
/*
*=======================================================================
*
*                  COPYRIGHT (c) 1988, 1989, 1992 BY
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
/*	031	CS			13-Aug-1992			      */
/*		Added function LclTopTreeCheckLowerEntry.  This is used to    */
/*		fix a bug in TopTreeAddEntries and TopTreeDeleteEntries	      */
/*		that was setting the svn.current_entry improperly if entries  */
/*		were added and deleted before the display was updated.	      */
/*	030	CS			31-Jul-1992			      */
/*		Fix for CLD CXO08483.  Added calls to XSetClipMask for the    */
/*		highlight_gc and inverse_highlight_gc in TopTreeDrawExposed   */
/*		to set the clip mask to None.				      */
/*	029	AN			 8-Jan-1992			      */
/*		Fixed bug where sometimes location cursor would not be erased */
/*		when the SVN window would lose focus.			      */
/*	028	CS			 2-Jan-1992			      */
/*		Changed calls to DisplayDrawEntry and HIDE_WIDGET.	      */
/*	027	AN			21-Nov-1991			      */
/*		Have routine TopTreeChangeMode handle new PTR window.	      */
/*	026	CS			30-Oct-1991			      */
/*		Modified LclTopTreeScrollAdjustSize to handle case of the     */
/*		shown area being greater than the max area.  This was causing */
/*		a negative slider value to be set on the scroll bars.	      */
/*      025     MP			20-Mar-1991                           */
/*		Add changes for bug fix, so that when mode is R->L, and tree  */
/*		display mode, the nav. button does not overwrite the bottom   */
/*		scrollbar.						      */
/*              Since I replaced the nav bitmaps by bitmaps 9x9 there is      */
/*              no need to adjust the size of button, it will have its        */
/*              natural size after SetValues also                             */
/*      024	AN			25-Feb-1991			      */
/*		Change XCopyArea gc to copyarea_gc to avoid NoExpose's	      */
/*	023	AN			12-Dec-1990			      */
/*		Fix bug in TopTreeCreateGhost where we were using the entries[]*/
/*		array for tree mode... wrong...				      */
/*	022	AN			11-Dec-1990			      */
/*		Change look of location cursor so it is now a solid line with */
/*		the color of highlight_color... and the highlight feature of  */
/*		SVN is now a dashed line.				      */
/*	021	AN			17-Oct-1990			      */
/*		Fix bug in LclTopTreeScrollAdjustSize so that if the height   */
/*		or width of the tree at is 0, XmScrollbar warnings will not   */
/*		occur.							      */
/*	020	AN			09-Oct-1990			      */
/*		Change XtAddActions to XtAppAddActions.			      */
/*	019	AN			27-Sep-1990			      */
/*		Add support for loc. cursor in tree mode.		      */
/*	018	AN		        20-Sep-1990			      */
/*		Changed code to define cursors.				      */
/*      017     SL			22-Aug-1990                           */
/*              Integrate the DEC Israel changes into this module - their     */
/*              code is turned on by the macro LayoutIsRtoL.                  */
/*	016	AN			11-Jul-1990			      */
/*		Change names of variables and anything external to be	      */
/*		internationalized for sides or pane windows.  "rhs"->secondary*/
/*		and "lhs" -> primary.					      */
/*	015	AN			28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	014	AN			24-May-1990			      */
/*		Lots more changes to LclTopTreeLineIntersects routine         */
/*		to fix problems with connections on very large trees.         */
/*	013	AN			21-May-1990			      */
/*		Fix bug in TopTreeHighlightEntry so that if the tree is	      */
/*		very large, when an entry is selected all entries after       */
/*		the selected one are not cleared in the nav. window.	      */
/*	012	SL	 		06-Apr-1990			      */
/*		Modify LclTopTreeIndexWindowExpose to clear the navigation    */
/*		window when all of the entries have been deleted so stale     */
/*		data is not left in it.					      */
/*	011	AN			26-Mar-1990			      */
/*		Fix for problem where if entire tree was expanded and a delete*/
/*		entries was performed on entire tree, the root entry would    */
/*		become detached from window edge.			      */
/*	010	AN			19-Mar-1990			      */
/*		Fixed bug in LclTopTreeSetCurrentEntry routine, that would    */
/*		incorrectly reposition the root entry (for all tree styles),  */
/*		if the root was not visible in the display window when the    */
/*		collapse of all entries was preformed.			      */
/*	009	AN			13-Mar-1990			      */
/*		Rewrote routine LclTopTreeLineIntersects so that it could     */
/*		handle connections with coordinates > 32767...		      */
/*	008	SL	 		01-Feb-1990			      */
/*		Add #pragma standard lines to compile cleanly with /STAN=PORT.*/
/*      007	WW			26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	006	WW			25-Jan-1990			      */
/*		Change DECwWsSvn*.h to DECwMSvn*.h.			      */
/*	005	WW			25-Jan-1990			      */
/*		Add LclAdjustNavButton routine.  This is kind of hacky, but   */
/*		it keeps the navigation window button pixmap in the correct   */
/*		location.						      */
/*	004	SL	 		23-Jan-1990			      */
/*		Change all BCSESVN routine references to SVN.  Change all     */
/*		SvnWidget typedef references to svn_widget.  Change the       */
/*		naming style of the constants from Svn* to SvnK*.	      */
/*	003	WW			22-Jan-1990			      */
/*		Work on tree scrolling.  Had to do several things to remove   */
/*		the warnings one would get from the scrollbars during tree    */
/*		display mode.						      */
/*      002     SL			12-Jan-1990                           */
/*              Make some additional post-converter adjustments to work	      */
/*		under Motif.  Fix up the include files, callback reasons,     */
/*		change XmNtitle to XmNdialogTitle for nav window and delete   */
/*		DwtNacceptFocus (since did not map to anything) on	      */
/*		XmCreateFormDialog.  Change the code for creating the cursor  */
/*		in the nav window; change VoidProc to XmVoidProc.  Use the    */
/*		closure field in XtCallbackRec instead of tag field.	      */
/*	001	SL	 		05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/

/*
**			SVN Tree mode internals
**
**  INTRODUCTION: The SvnKdisplayTree mode currently supports 4 different tree
**  styles (SvnKtopTree, SvnKoutlineTree, SvnKhorizontalTree, SvnKuserDefinedTree).
**  Almost all support code for each of these styles are the same except for
**  the layout code.  There is some run-time conditional code in this module as
**  well as in some support macros in SVNPRIVATE.H.  Although all the names of
**  the routines in this module begin with TopTree, they generally apply to all
**  styles (they have the TopTree prefix for historical reasons, and this will
**  be corrected when ported to the DECwindows toolkit).
**
**
**  COORDINATE SYSTEMS:  The positioning of entries is done using a world
**  coordinate system.  The coordinates go from (0,0) to tree_height and
**  tree_width (which are stored in the Svn widget record).  The mapping of the
**  world coordinates into the Svn window coordinates is simply a translation
**  using the mapx and mapy variables in the Svn widget record.  All coordinates
**  are in pixels units for easy mapping.  The translation to/from window
**  and world coordinates are done using the X_TO_WINDOW, Y_TO_WINDOW,
**  X_TO_WORLD, and Y_TO_WORLD macros.  
**
**  
**  LAYOUT:  Unlike in OutlineMode, half of the position information for each
**  entry is stored with the entry itself.  In addition to the position of the
**  entry, also stored in entry is the connection point to the parent.  Because
**  of the complications of the various tree styles, the naming of the position
**  and connection point data is sometimes misleading.  The naming of the
**  position information (x, px, py) is only accurate when the tree style is
**  SvnKtopTree.  In SvnKhorizontalTree, for example, the meanings of all of these
**  fields are almost exactly reversed (x is really y, etc).  The objective of
**  the tree layout is to position each entry centered above its children
**  entries and keep each level of the tree on the same plane.  This is
**  accomplished by storing the level information in the Svn widget record, and
**  the other portion of the position in the entry itself.  In the case of
**  SvnKtopTree the important layout fields are:
**
**	svnentry->x -- The x position of the entry
**	svnentry->px -- The x position of the connection point
**	svnentry->py -- The offset of the connection point from the parent's level.
**	svnw->max_level -- The current size of levelx and levely arrays (these
**			arrays are dynamically allocated as needed).  They must
**			be maintained larger that the maximum level in the tree.
**			This is done in the AddEntry code in SVN_STRUCTURE.  We
**			need never check the bounds and safely access up to
**			max_level, and since it is larger that the deepest level
**			of the tree we can always access up to level+1 for any
**			entry in the tree.
**	svnw->levely -- Array of positions of the various levels (indexed
**			by level number)
**	svnw->levelx -- Array of max x position of entrys on various levels
**			(indexed by level number).
**
**
**  Most of this complexity is hidden in the X_POS, Y_POS, PX_POS, and PY_POS
**  macros.  They return the desired information.  Only the layout routines
**  are concerned with modifying these fields so if those require an
**  understanding of how these fields are used.
**
**
**  ENTRY DRAWING:  The contents of an entry are completely handled by the
**  DisplayDrawEntry routine in SVN_DISPLAY.  This routine is common between
**  both display modes.  This routine is totally driven by the information
**  stored in the entry and component structures.  The drawing of the bounding
**  box and connecting lines are handled by the LclTopTreeDrawEntry routine.
**
**
**  ENTRY_INFO:  The shape of the various rectangles and arcs are handled by
**  several macros.  Currently they are only used here, but if DDIF output were
**  ever added the information could be used by the DDIF output routines.
**
**
**  REDRAW: The TopTree module attempts to refrain from drawing when disabled.
**  In order to do this it makes extensive use of clipping rectangles.  Whenever
**  we are disabled and something needs to be drawn we specify a clipping
**  rectangle that contains the area into the Svn clipping rectangle structures.
**  When enable, these clips are set on the various GCs and anything that
**  intersects the rectangles are redrawn.
**
**
**  SCROLLING:  Scrolling code is slightly confusing because it has not yet been
**  removed from the Outline module.  Currently, SVN_DISPLAY_OUTLINE gets all
**  scrolling callbacks and depending upon the mode calls the corresponding
**  Tree mode routines.  This should be cleaned-up such that all scrolling
**  callbacks are common and in the SVN_DISPLAY module.
*/


#define  SVN_DISPLAY_TOPTREE

#ifdef VMS
/*#pragma nostandard*/
#include "XmP.h"
#include "DrawingA.h"
#include "Form.h"
#include "Shell.h"	/* For XtNiconName */
#include "cursorfont.h"
/*#pragma standard*/
#else
#include <Xm/XmP.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>
#endif

#include "DXmSvnP.h"
#include "svnprivate.h"
#include "DXmPrivate.h"


/*
**  Local routine declarations
*/
static int LclTopTreeMapRectangle   ();
static int LclTopTreeOffsetToComp   ();
static void LclTopTreeDrawBox	    ();	    /*  Draw box around an entry */
static void LclTopTreeDrawEntry        ();	    /*  Draw an entry            */
static void LclTopTreeLayoutRoot	    ();	    /*  Layout the entire tree   */
static void LclTopTreeLayout	    ();	    /*  Layout a subtree  */
static void LclTopTreeMoveSubtree	    ();	    /*  move subtree as specified */
static void LclTopTreeSetChildConnections();   /*  set connection points of child entries */
static void LclSideTreeLayoutRoot	    ();	    /*  Layout the entire tree   */
static void LclSideTreeLayout	    ();	    /*  Layout a subtree  */
static void LclUserDefinedTreeLayoutRoot();    /*  Layout the entire tree   */
static void LclOutlineTreeLayoutRoot();	    /*  Layout the entire tree   */

static void LclTopTreeScrollAdjustSize ();	    /* adjust size and min/max of scroll bars */
static void LclTopTreeAdjustHeightCentered();  /* Adjust Height routine that centers components */
static void LclTopTreePerformScroll    ();	    /* scrolling support routine */
static void LclTopTreeScrollWidgets    ();	    /* scrolling support routine */
static Boolean LclTopTreeIntersects    ();	    /* returns TRUE if rect intersect */
static Boolean LclTopTreeEntryVisible  ();	    /* returns TRUE if entry is visible */
static void LclTopTreeSetCurrentEntry  ();	    /* mark entry as current */
static void LclTopTreeIndexWindowExpose();	    /* redraws Index/Nav window */
static void LclTopTreeScrollDrag	    ();	    /* Support routine when slider dragged */
static void LclTopTreeUpdateNavBox	    ();	    /* Updates box in Nav/Index window */
    void LclTopTreeNavWindowButtonDown ();  /* Action routine for Nav window */
    void LclTopTreeNavWindowHelp    ();	    /* Action routine for Nav window */
static void LclAdjustNavButton    ();	    /* Adjusts navigation window push button */
static Boolean LclTopTreeCheckLowerEntry ();	    /* Checks if an entry_number is lower than svn.current_entry */


/*
**  Define module literals
*/
#define inc_length		10
#define max_points		4
#define buffer_size		1000
#define max_coord		32767



/*
**  XtCallbackRec structures for callbacks
*/
    static XtCallbackRec index_window_CBstruct [2] = {{(XtCallbackProc)LclTopTreeIndexWindowExpose, (XtPointer)NULL},
						      {(XtCallbackProc)NULL, (XtPointer)NULL}};


/*
**  Translations/Actions for MB1 and help in Nav window.
*/
    static const char NavWindowTranslations[] =
      "~Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNNAVWINDOWBUTTONDOWN()\n\
        Help<BtnUp>:                            SVNNAVWINDOWHELP()";


    static XtActionsRec NavWindowActionsList[] = {
      {"SVNNAVWINDOWBUTTONDOWN",   (XtActionProc)LclTopTreeNavWindowButtonDown},
      {"SVNNAVWINDOWHELP",     (XtActionProc)LclTopTreeNavWindowHelp},
      {(char *) NULL, (XtActionProc)NULL} };

/*
**  Procedure to adjust the location of the pixmap in the navigation window button.
*/
/*
 *   there is nothing to adjust since I replaced 17x17 bitmap by 9x9 one
 *   9x9 fits exactly into the space we have for pixmap and now it does not
 *   attempt the change geometry, but all my changes were under RtoL
 *   condition only
 */

static void LclAdjustNavButton (svnw)

   svn_widget svnw;
{
   Widget w = svnw->svn.nav_button;

/*
**  If the widget is not realized, then complain
*/
    if (!XtIsRealized (w)) return;


/*
**  If the width or height is different, then set the new values and resize
**  the window.  Tell the widget about it afterwards.
*/
    if ((XtWidth(w) != button_height) || (XtHeight(w) != button_height))
       {

        /*
        **  Set the new width and height
        */
            XtWidth(w)  = button_height;
            XtHeight(w) = button_height;

        /*
        **  Call the widgets resize procedure
        */
            (* (XtCoreProc (w, resize))) (w);
       };

}

/*
**  This routine processes the top and bottom buttons.  It is called from the
**  routine OutlineScrollButton in SVN_DISPLAY_TOPTREE. This routine will move
**  the display to the next/previous entry that is not currently visible on the
**  screen and has the index_window attribute specified.  It performs its work
**  by calling the TopTreePositionDisplay routine to put the entry at the top
**  of the display.  The following values in the button_top variable determine
**  the action of this routine:
**
**       1 - MB1 on top button
**       2 - SHIFT/MB1 on top button
**       3 - MB1 on bottom button
**       4 - SHIFT/MB1 on bottom button
*/

void TopTreeScrollButton (svnw)

    svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int i;


/*
**  Must disable DXmSvn to change display position
*/
    DXmSvnDisableDisplay((Widget) svnw);


/*
**  If it's the top button the goto previous indexed entry that is non visible
*/
    if (svnw->svn.button_top <= 2) 
	{
	/*
	**  Search for the first visible indexed entry.
	*/
	for (i = 1; i <= svnw->svn.num_entries; i++) 
	    {
	    svnentry = StructGetEntryPtr (svnw, i);

	    /*
	    ** If this entry is marked for index window and 
	    ** visible, then exit the search loop.
	    */
	    if ((svnentry->index_window) && LclTopTreeEntryVisible(svnw, svnentry)) 
		break;
	    }


	/*
	**  Now search back from the first visible indexed item to the
	**  previous one.
	*/
	for (i--; i > 0; i--) 
	    {
	    /*
	    ** If this entry is marked for index window and is not currently
	    ** visible then exit the loop.
	    */
	    svnentry = StructGetEntryPtr (svnw, i);
	    if ((svnentry->index_window) && !LclTopTreeEntryVisible(svnw, svnentry)) 
		break;
	    }


	/*
	**  Position the new entry at the top of the display
	*/
	if (i <= 0) i = 1;
	TopTreePositionDisplay(svnw, i, DXmSvnKpositionTop);
	}


/*
**  If it's the bottom button then goto next indexed entry
*/
    if (svnw->svn.button_top > 2)
	{
	/*
	**  Search for the last visible indexed entry.
	*/
	for (i = svnw->svn.num_entries; i > 0; i--) 
	    {
	    svnentry = StructGetEntryPtr (svnw, i);

	    /*
	    ** If this entry is marked for index window and 
	    ** visible, then exit the search loop.
	    */
	    if ((svnentry->index_window) && LclTopTreeEntryVisible(svnw, svnentry)) 
		break;
	    }

	/*
	**  Now search forward until we find an indexed entry not currently
	**  visible
	*/
	for (i++; i < svnw->svn.num_entries; i++) 
	    {
	    /*
	    ** If this entry is marked for index window and is not currently
	    ** visible then exit the loop.
	    */
	    svnentry = StructGetEntryPtr (svnw, i);
	    if ((svnentry->index_window) && !LclTopTreeEntryVisible(svnw, svnentry)) 
		break;
	    }


	/*
	**  Position the new entry at the bottom of the display
	*/
	if (i >= svnw->svn.num_entries) i = svnw->svn.num_entries;
	    TopTreePositionDisplay(svnw, i, DXmSvnKpositionTop);
	}


/*
**  Enable and let the display update to the new position
*/
    DXmSvnEnableDisplay((Widget) svnw);
}




/*
** This routine finds the first visible entry and sets it as the current entry
** (in the current_entry and current_entry_number variables).  This gives us a
** new point of reference when changing display mode or tree style.  If the
** entry it finds is entry number one, then it does not set the current entry,
** so that the display will assume the default position with entry one centered
** as appropriate.
*/

void TopTreeSetCurrentEntry (svnw)

    svn_widget svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int i;

/*
**  If there already is a current entry, then don't need to do anything
*/
    if (svnw->svn.current_entry != NULL) return;


/*
**  Search for the first visible entry.
*/
    for (i = 1; i <= svnw->svn.num_entries; i++) 
	{
	svnentry = StructGetEntryPtr (svnw, i);

	/*
	** If this entry is marked for index window and 
	** visible, then exit the search loop.
	*/
	if (LclTopTreeEntryVisible(svnw, svnentry)) {
	    svnw->svn.current_entry_number = i;
	    LclTopTreeSetCurrentEntry(svnw, i);
	    break;
	    }
	}


/*
**  If current entry is the first, then clear current entry and let the
**  default position of entry one occur.
*/
    if (svnw->svn.current_entry_number == 1) {
	svnw->svn.current_entry_number = 0;
	svnw->svn.current_entry = NULL;
	}


} /* TopTreeSetCurrentEntry */




/*
**  Set the range hook (start position) for the range select.  The start
**  position in world coordinates is stored in the range_hook_y and
**  range_hook_x variables and the range_hook flag is set to TRUE.  Thus no
**  matter where the display is scrolled to, we can determine which entries
**  intersect the world coordinate rectangle. This allows the starting positon
**  of a range select box to go off the screen, but still select everything.
*/
void TopTreeSetRangeHook (svnw, x, y)
    
    svn_widget	svnw;
    int		x, y;	    /* position where range started */

{
/*
**  Store hook in range hook variables in world coordinates
*/
    if (LayoutIsRtoL(svnw))
	svnw->svn.range_hook_x = X_TO_WORLD(svnw, XtWidth (svnw) - x);
    else
        svnw->svn.range_hook_x = X_TO_WORLD(svnw, x);
    svnw->svn.range_hook_y = Y_TO_WORLD(svnw, y);
    svnw->svn.range_hook = TRUE;

} /* TopTreeRangeHook */

/*
**  Return the entries within the extended range box from the range_hook to the
**  (x,y) position specified in window coordinates.  This routine always
**  returns an array of entries (range_flag is always set to FALSE) allocated
**  with XtMalloc.  The caller is responsible for freeing the array using
**  XtFree.  The number of entries returned is returned in the entry_count
**  parameter.
*/
void TopTreeGetRangeEntries (svnw, x, y, range_flag, entry_count, entry_array)

  svn_widget svnw;
  int x;
  int y;
  int *range_flag;
  int *entry_count;
  LevelPtr *entry_array;

{
/*
**  Local data declarations
*/
    Position low_x, low_y, high_x, high_y;
    DXmSvnEntryPtr svnentry;
    int i;


/*
** The tree mode always returns a list, so set range_flag to FALSE
*/
    *range_flag = FALSE;
    *entry_count = 0;


/*
**  If the range hook isn't set, then don't nothing is in the range.
**  Otherwise clear the range hook.
*/
    if (!svnw->svn.range_hook) return;
    svnw->svn.range_hook = FALSE;


/*
**  Convert the next corner's x,y to world coordinates
*/
    if (LayoutIsRtoL(svnw))
	x = XtWidth (svnw) - x;
    x = X_TO_WORLD(svnw, x);
    y = Y_TO_WORLD(svnw, y);


/*
**  Find the low and high x values of the rectangle.
*/
    if (x > svnw->svn.range_hook_x) {
	low_x = svnw->svn.range_hook_x;
	high_x = x;
	}
    else {
	high_x = svnw->svn.range_hook_x;
	low_x = x;
	};



/*
**  Find the low and high y values of the rectangle.  NOTE: We are in world
**  coordinates so large y values are lower on the screen.
*/
    if (y > svnw->svn.range_hook_y) {
	low_y = svnw->svn.range_hook_y;
	high_y = y;
	}
    else {
	high_y = svnw->svn.range_hook_y;
	low_y = y;
	};


/*
**  Loop through the entries to determine how large the entry_array must be
**  so we can XtMalloc storage for it.
*/
    for (i = svnw->svn.num_entries; i > 0; i--) {
	svnentry = StructGetValidEntryPtr (svnw, i);
	if (LclTopTreeIntersects ((int)X_POS(svnw, svnentry), 
			          (int)Y_POS(svnw, svnentry), 
				  (int)svnentry->width, 
				  (int)svnentry->height, 
				  low_x, low_y, high_x, high_y)) {
	    (*entry_count)++;
	    };
	};


/*
**  Map the rectangle into the entry_numbers inside it.
*/
    *entry_array = (LevelPtr) XtMalloc(*entry_count * sizeof(int));
    *entry_count = LclTopTreeMapRectangle(svnw, *entry_array, low_x, low_y, high_x, high_y);


} /* TopTreeGetRangeEntries */

/*
**  This routine is the interface for the public DXmSvnMapPosition
**  routine.  This routine must determine the entry number and component number
**  that are being displayed at the given x,y coordinate.  We will assume that
**  this coordinate is relative to the SVN window.
*/

void TopTreeMapPosition (svnw, findx, findy, entry_number, component_number, entry_tag)

  svn_widget svnw;
  int findx;
  int findy;
  int *entry_number;
  int *component_number;
  XtPointer *entry_tag;

{
/*
**  Cast the widget passed into an DXmSvnWidget data structure.
*/
    int entry;
    DXmSvnEntryPtr svnentry;
    int mapped_entry = 0;
    int mapped_component = 0;
    XtPointer mapped_tag = 0;

	    
/*
**  X-coord is an offset from the right border
*/
    if (LayoutIsRtoL(svnw))
	findx = XtWidth (svnw) - findx;

/*
**  Convert findx, findy to world coordinates
*/
    findx = X_TO_WORLD(svnw, findx);
    findy = Y_TO_WORLD(svnw, findy);


/*
**  Find out at what display offset maps into this x and y position.
*/
    for (entry = svnw->svn.num_entries; entry > 0; entry--) {
	svnentry = StructGetValidEntryPtr (svnw, entry);
	if ((findx >= X_POS(svnw, svnentry)) && 
	    (findx <= (X_POS(svnw, svnentry) + (int)svnentry->width)) &&
	    (findy >= Y_POS(svnw, svnentry)) && 
	    (findy <= (Y_POS(svnw, svnentry) + (int)svnentry->height)))
	    {
	    mapped_entry = entry;
	    mapped_component = LclTopTreeOffsetToComp(svnw, svnentry, 
		findx - X_POS(svnw, svnentry), findy - Y_POS(svnw, svnentry));
	    mapped_tag = svnentry->entry_tag;
	    svnw->svn.map_level = svnentry->level;
	    break;
	    };
	};


/*
**  Give them to the caller
*/
    *entry_number = mapped_entry;
    if (component_number != NULL) *component_number = mapped_component;
    if (entry_tag != NULL) *entry_tag = mapped_tag;


} /* TopTreeMapPosition */



/*
**  This routine determines which component within the specified entry
**  the offset (x,y) specifies and returns it.
*/

static int LclTopTreeOffsetToComp (svnw, svnentry, x, y)
    
    svn_widget svnw;
    DXmSvnEntryPtr svnentry;
    int x, y;

{
    DXmSvnCompPtr comp;		/* components of entry */
    int j;


/*
**  Subtract out the arc width and highlight width (if necessary) to get actual
**  corner of first component.
*/
    x -= svnw->svn.arc_width;
    y -= 2;
    if (svnw->svn.expect_highlighting) {
	x -= highlight_width;
	y -= highlight_width;
	}


/*
**  Loop throught the components looking for a match
*/    
    for (j = 1;  j <= svnentry->num_components;  j++)
	{ 
	comp = &svnentry->entrycompPtr[j-1];
	if ((comp->x <= x) && (comp->y <= y) && 
	   (x <= (comp->x + comp->width)) && (y <= (comp->y + comp->height)))
	    return j;
	};

/*
**  Not in any component, return component 0.
*/
    return  0; 


} /* LclTopTreeOffsetToComp */

/*
**  This routine is the high level interface to DXmSvnPositionDisplay.  This
**  routine simply determines where the display is to be set to and calls the
**  LclTopTreePerformScroll routine to get us there.
*/

int TopTreePositionDisplay (svnw, entry_number, position)

  svn_widget svnw;
  int entry_number;
  int position;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;
    int new_mapx, new_mapy;


/*
**  Set the current position
*/
    new_mapy = svnw->svn.mapy;
    new_mapx = svnw->svn.mapx;


/*
**  Determine the entry to position, if not a next/prev page operation
*/
    if ((position != DXmSvnKpositionNextPage) && (position != DXmSvnKpositionPreviousPage))
	{
	svnentry = StructGetValidEntryPtr (svnw, entry_number);


	/*
	**  Attempt to center the entry horizontally
	*/
	new_mapx = X_POS(svnw, svnentry) - (REALWIDTH(svnw)/2) + (svnentry->width/2);
	}


/*
**  Dispatch to local routines based on the position code
*/
    switch (position)
      {
        case DXmSvnKpositionTop:
	    new_mapy = Y_POS(svnw, svnentry);
	    break;
        case DXmSvnKpositionMiddle: 
	    new_mapy = Y_POS(svnw, svnentry) - (REALHEIGHT(svnw)/2);
	    break;
        case DXmSvnKpositionBottom: 
	    new_mapy = Y_POS(svnw, svnentry) - REALHEIGHT(svnw) + svnentry->height;
	    break;
        case DXmSvnKpositionPreviousPage:
	    new_mapy -= REALHEIGHT(svnw);
	    break;
        case DXmSvnKpositionNextPage:
	    new_mapy += REALHEIGHT(svnw);
	    break;
      };


    /*
    **  If the widget is realized then scroll to the new position.
    */
    if (XtIsRealized (svnw)) 
	LclTopTreePerformScroll(svnw, new_mapx - svnw->svn.mapx, new_mapy - svnw->svn.mapy);
    return TRUE;


}   /* TopTreePositionDisplay */

/*
**  This routine is called to map a particular world coordinate rectangle into
**  a list of selections that intersect that rectangle.  The calling program is
**  responsible for allocating the array into which the intersecting entry
**  numbers are placed.  The return value of this procedure is the number of
**  entries entered into the array.
*/

static int LclTopTreeMapRectangle (svnw, entry_numbers, low_x, low_y, high_x, high_y)

  svn_widget svnw;
  int (*entry_numbers)[];
  int low_x, low_y, high_x, high_y;

{
    DXmSvnEntryPtr svnentry;
    int i;
    int count = 0;


/*
**  Find out at what display offset maps into this x and y position.
*/
    for (i = svnw->svn.num_entries; i > 0; i--) {
	svnentry = StructGetValidEntryPtr (svnw, i);
	if (LclTopTreeIntersects ((int)X_POS(svnw, svnentry), 
				  (int)Y_POS(svnw, svnentry), 
				  (int)svnentry->width, 
				  (int)svnentry->height, 
				  low_x, low_y, high_x, high_y)) {
	    (*entry_numbers)[count] = i;
	    count++;
	    };
	};


/*
**  Return the count of entries
*/
    return count;


} /* LclTopTreeMapRectangle */



/*
**  This routine is called to create the ghost image to be used in a dragging
**  operation.
**  If an entry number is passed into this routine, then we only want to
**  create a ghost that is the size of that entry and ignore all others.
*/

void TopTreeCreateGhost (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Local data declarations
*/
    Screen  *screen  = XtScreen (svnw);
    Display *display = DisplayOfScreen (screen);
    int i, min_x = 99999, max_x = 0, min_y = 99999, max_y = 0;


/******************************************************************************/
/*                                                                            */
/* Determine the rectangle size to be used in this ghost.		      */
/* If an entry number was passed into this routine, that signals that the     */
/* entry was NOT selected and we only want to create a ghost that size is     */
/* equal to the size of that entry number. Otherwise, continue as always      */
/* and create a ghost that will enclose ALL selected entries.		      */
/*                                                                            */
/******************************************************************************/
/*
**  Determine the rectangle size to be used in this ghost.
*/
    if (entry_number != 0)
	{
	DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, entry_number);

	int low_x  = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));
	int low_y =  Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));
	int high_x = low_x + svnentry->width;
	int high_y = low_y + svnentry->height;

	if (min_x > low_x)  min_x = low_x;
	if (min_y > low_y)  min_y = low_y;
	if (max_x < high_x) max_x = high_x;
	if (max_y < high_y) max_y = high_y;
	}

	/* We want a ghost for each selected entry... in the tree */
	
	else
	    {
	    for (i = 1;  i <= svnw->svn.num_entries;  i++)
		{
		DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, i);
		if (svnentry->selected)
		    {
		    int low_x  = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));
		    int low_y =  Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));
		    int high_x = low_x + svnentry->width;
		    int high_y = low_y + svnentry->height;

		    if (min_x > low_x)  min_x = low_x;
		    if (min_y > low_y)  min_y = low_y;
		    if (max_x < high_x) max_x = high_x;
		    if (max_y < high_y) max_y = high_y;
		    };
		};
	    }; 
	


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
**  For each entry on the screen.  If it is currently selected then draw it's
**  outline into the pixmap using the normal graphic context.
*/
    if (entry_number != 0)
	{
	DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, entry_number);

	int x  = X_TO_WINDOW(svnw, X_POS(svnw, svnentry)) - min_x;
	int adjust_y  = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry)) - min_y;

	if (LayoutIsRtoL(svnw))
		LclTopTreeDrawBox(svnw, svnw->svn.ghost, svnentry,
			svnw->svn.background_gc,
			svnw->svn.ghost_width - x - svnentry->width - 1,
			adjust_y, FALSE);
	else
		LclTopTreeDrawBox(svnw, svnw->svn.ghost, svnentry,
			svnw->svn.background_gc, x, adjust_y, FALSE);
	}
    else
	{		    
	for (i = 1;  i <= svnw->svn.num_entries;  i++)
	    {
	    DXmSvnEntryPtr svnentry = StructGetEntryPtr (svnw, i);
	    if (svnentry->selected)
		{
		int x  = X_TO_WINDOW(svnw, X_POS(svnw, svnentry)) - min_x;
		int adjust_y  = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry)) - min_y;
		if (LayoutIsRtoL(svnw))
		   LclTopTreeDrawBox(svnw, svnw->svn.ghost, svnentry,
			svnw->svn.background_gc,
			svnw->svn.ghost_width - x - svnentry->width - 1,
			adjust_y, FALSE);
		else
	           LclTopTreeDrawBox(svnw, svnw->svn.ghost, svnentry,
			svnw->svn.background_gc, x, adjust_y, FALSE);
		};
	    };
	}


} /* TopTreeCreateGhost */ 



/*
**  This routine is called to delete the ghost image used in a dragging operation.
*/

void TopTreeDeleteGhost (svnw)

  svn_widget svnw;

{
    OutlineDeleteGhost(svnw);
} /* TopTreeDeleteGhost */

/*
**  This routine is called from the SVN module to do the display portion of the
**  source module invalidating an entry.  The purpose of this routine is to
**  merely record the fact that this entry was invalidated.  If the entry is
**  not currently in the display viewport, then this call is ignored.  If this
**  entry has sub_widgets on the screen they are removed.  When an entry is
**  invalidated the size may change so the layout of the tree must be
**  recalculated.
*/

void TopTreeInvalidateEntry (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
    int j;			/* index */
    DXmSvnEntryPtr svnentry;	/* entry invalidated */
        

/*
**  Get the entry pointer
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);


/*
**  If the entry is visible, then we must redraw the display
*/    
    if (LclTopTreeEntryVisible(svnw, svnentry)) {
	/*
	**  Set this entry as the current entry to keep it from moving when we
	**  redisplay.
	*/	
	LclTopTreeSetCurrentEntry(svnw, entry_number);
	svnw->svn.display_changed = TRUE;
	if (svnw->svn.sub_widgets_used)
	   for (j = 0;  j <= svnentry->num_components - 1;  j++)
	      if (svnentry->entrycompPtr[j].type == DXmSvnKcompWidget)
	         HIDE_WIDGET(svnentry->entrycompPtr[j].var.is_widget.readwrite_text);
	};


/*
**  Mark entry as needing to be recalculated.
*/
    svnentry->height_adjusted = FALSE;


} /* TopTreeInvalidateEntry */



/*
**  This routine is called whenever the application or source module has called
**  the DXmSvnSetEntrySensitivity high level call which is located in the SVN
**  module.
**
**  This routine is responsible for seeing if it's currently in the display 
**  viewport and if the widget is enabled, to change the sensitivity right 
**  away.  The caller of this routine has ensured that the sensitivity has 
**  actually changed.  If the entry in question is not in the viewport, then 
**  this call can be ignored.
**
**  Note that the sensitivity is the inverse of grayed and has already been set
**  by the corresponding Structure.C routine.
*/

void TopTreeSetEntrySensitivity (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
/*
**  Just call highlight entry to handle the redraw
*/
    TopTreeHighlightEntry(svnw, entry_number);

} /* TopTreeSetEntrySensitivity */


/*
**  Checks if entry_number is a lower entry number than svnw->svn.current_entry 
*/
static Boolean LclTopTreeCheckLowerEntry (svnw, entry_number)
    svn_widget svnw;
    int entry_number;
{
    DXmSvnEntryPtr svnentry;
    int i;
    Boolean entry_lower = TRUE;

    /*
    **  Determine if after_entry is a lower entry number than svnw->svn.current_entry 
    */
    for (i = 1; i < entry_number && entry_lower; i++)
    {
	svnentry = StructGetEntryPtr (svnw, i);
	if (svnentry == svnw->svn.current_entry)
	    entry_lower = FALSE;
    }

    return entry_lower;
}


/*
**  This routine is called indirectly from the DXmSvnAddEntries routine
**  located in SVN.C.  It is responsible for setting the current entry so
**  when the tree the new layout is performed the display will not move too much.
**  It is also responsible for making sure there is enough space in the levelx
**  and levely arrays in the widget record.  This is the only place that the
**  number of levels can increase.
*/

void TopTreeAddEntries (svnw, after_entry)

  svn_widget svnw;
  int after_entry;

{
/*
**  If entry zero was specified, there can be no current entry.
*/
    /* No current entry the first time */
    if (after_entry != 0)
    {
	if (svnw->svn.display_changed && svnw->svn.current_entry != NULL)
	    if (LclTopTreeCheckLowerEntry(svnw, after_entry))
	    {
		/*
		**  after_entry is a lower entry number than svn.current_entry.
		**  Clear display_changed and current_entry so after_entry will be
		**  set as the current_entry in the call to LclTopTreeSetCurrentEntry.
		*/
		svnw->svn.display_changed = FALSE;
		svnw->svn.current_entry = NULL;
	    }

	LclTopTreeSetCurrentEntry(svnw, after_entry);
    }


/*
**  After an AddEntries we must re-layout the tree.
*/
    svnw->svn.display_changed = TRUE;

} /* TopTreeAddEntries */



/*
**  This routine is called indirectly from the DXmSvnDeleteEntries routine
**  located in SVN.C.  Its job is to mark the entries that will need to be
**  refreshed, when the widget is re-enabled.  If sub-widgets are being
**  displayed they are removed from the screen.  It also stores the current
**  entry so that when layout occurs the display doesn't move.
*/

void TopTreeDeleteEntries (svnw, after_entry, number_of_entries)

  svn_widget svnw;
  int after_entry;
  int number_of_entries;

{
    int i, j;
    DXmSvnEntryPtr svnentry;

/*
** Do processing for all deleted entries  
*/
    for (i = after_entry + 1; i <= after_entry + number_of_entries; i++) {
	svnentry = StructGetEntryPtr(svnw,i);
	/*
	** If the current entry is being deleted, then cancel it.  The
	** current entry will then be set by the call below.
	*/
	if (svnw->svn.current_entry == svnentry) svnw->svn.current_entry = NULL;
 
	/*
	** Remove widget components from screen if its entry has been deleted
	*/
 	if (svnw->svn.sub_widgets_used) {
	    for (j = 0;  j <= svnentry->num_components - 1;  j++)
	       if (svnentry->entrycompPtr[j].type == DXmSvnKcompWidget)
		  HIDE_WIDGET(svnentry->entrycompPtr[j].var.is_widget.readwrite_text);
	    }
	};								    


/*
**  Set the current entry to keep display from moving
*/
    if (svnw->svn.display_changed && svnw->svn.current_entry != NULL)
	if (LclTopTreeCheckLowerEntry(svnw, after_entry))
	{
	    /*
	    **  after_entry is a lower entry number than svn.current_entry.
	    **  Clear display_changed and current_entry so after_entry will be
	    **  set as the current_entry in the call to LclTopTreeSetCurrentEntry.
	    */
	    svnw->svn.display_changed = FALSE;
	    svnw->svn.current_entry = NULL;
	}

    LclTopTreeSetCurrentEntry(svnw, after_entry);
    svnw->svn.display_changed = TRUE;


/*
**  If there are no entries left, make sure that there is no current entry.
*/
    if (svnw->svn.num_entries == 0) {
	svnw->svn.current_entry = NULL;
	}


} /* TopTreeDeleteEntries */



/*
**  This routine is called indirectly from the DXmSvnEnableDisplay routine
**  located in SVN.C.  Its job is to get the screen up to date based on calls
**  that were made during the disabled state.  Only when the display_changed
**  flag is set do we need to call the applicable layout routine.  Otherwise
**  we just call the TopTreeDraw routine to get things drawn.
*/

void TopTreeEnableDisplay (svnw)

  svn_widget svnw;

{
/*
**  If the display has been changed, skip this stuff and go to work
*/
    if (!svnw->svn.display_changed) 
	{
	/*
	**  If this is a forced Refresh, or we have some clips from
	**  expose events to handle call the draw routine.
	*/
	if ((svnw->svn.refresh_all) || (svnw->svn.clip_count != 0))
	    {
	    /*
	    **  In UserDefined Trees, they may grow anytime, so call
	    **  the layout routine to calculate the tree width and height.
	    */
	    if (svnw->svn.tree_style == DXmSvnKuserDefinedTree)
		LclUserDefinedTreeLayoutRoot(svnw);
	    TopTreeDraw(svnw);
	    }
	return;	    
	}


/*
**  Avoid recursion...
*/
    svnw->svn.display_changed = FALSE;


/*
**  Disable the widget in case new entries are added while we are here
*/
    DXmSvnDisableDisplay ((Widget) svnw);


/*
**  Indicate that there are no entries being displayed
*/
     svnw->svn.display_count = 0;


/*
** Layout the tree until everything is drawn properly
*/

    if (svnw->svn.tree_style == DXmSvnKoutlineTree)
	LclOutlineTreeLayoutRoot(svnw);
    else if (svnw->svn.tree_style == DXmSvnKhorizontalTree)
	LclSideTreeLayoutRoot(svnw);
    else if (svnw->svn.tree_style == DXmSvnKuserDefinedTree)
	LclUserDefinedTreeLayoutRoot(svnw);
    else 
	LclTopTreeLayoutRoot(svnw);


/*
**  Now that we have updated the layout, clear the current entry.
*/
    svnw->svn.current_entry = NULL;


/*
**  Redraw everything (get rid of any pending graphicsExposures events)
*/
    svnw->svn.refresh_all = TRUE;
    svnw->svn.update_nav_window = TRUE;
    TopTreeDraw(svnw);


/*
**  Disable the widget in case new entries are added while we are here
*/
    DXmSvnEnableDisplay ((Widget) svnw);


} /* TopTreeEnableDisplay */



/*
**  This routine is called when the selection status of an entry has 
**  transitioned from low to high.  It may not be on the screen at this
**  time (Select All).  If the Nav Window is up, then we need to also
**  change the highlighting there as well.
*/

void TopTreeHighlightEntry (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{
    DXmSvnEntryPtr	svnentry;	/* Entry to Highlight */

/*
**  If visible, clear and redraw the entry in the window.
*/
    svnentry = StructGetValidEntryPtr (svnw, entry_number);

    if (LclTopTreeEntryVisible(svnw, svnentry)) {
	/*
	**  Clear the bounding box for the entry
	*/
	if (LayoutIsRtoL(svnw))
	    {
	    Dimension   rtol_x = (Position) (XtWidth(svnw->svn.primary_window_widget)
					- svnentry->width - 1
					- X_TO_WINDOW(svnw, X_POS(svnw,svnentry)));
	    XClearArea (XtDisplay(svnw), XtWindow(svnw->svn.primary_window_widget),
		rtol_x,
		(Position)Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry)),
		(Dimension)svnentry->width+1,
		(Dimension)svnentry->height+1, FALSE);
	    }
	else
	    XClearArea (XtDisplay(svnw), XtWindow(svnw->svn.primary_window_widget), 
		(Position)X_TO_WINDOW(svnw, X_POS(svnw, svnentry)), 
		(Position)Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry)), 
		(Dimension)svnentry->width+1, 
		(Dimension)svnentry->height+1, FALSE);


	/*
	**  If we are enabled, draw the entry immediately, otherwise set the
	**  clips so that the entry will be drawn when we are enabled again.
	*/
	if ((svnw->svn.disabled_count == 0) &&
	    (svnw->svn.tree_style != DXmSvnKuserDefinedTree))
	    {
	    /*
	    **  Redraw the entry
	    */
	    LclTopTreeDrawEntry(svnw, XtWindow(svnw->svn.primary_window_widget), svnentry, 
		X_TO_WINDOW(svnw, X_POS(svnw, svnentry)),
		Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry)));
	    }
	else
	    {
	    /*
	    **  Set the clipping rectangles for the redraw.  If there
	    **  is no more room, then redraw entire display.
	    */
	    if (svnw->svn.clip_count < max_clips)
		{
		svnw->svn.clips[svnw->svn.clip_count].x = ( (LayoutIsRtoL(svnw)) ?
			XtWidth(svnw->svn.primary_window_widget)
			- X_TO_WINDOW(svnw, X_POS(svnw, svnentry)) - svnentry->width
		      : X_TO_WINDOW(svnw, X_POS(svnw, svnentry)));
		svnw->svn.clips[svnw->svn.clip_count].y = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));
		svnw->svn.clips[svnw->svn.clip_count].width = svnentry->width + 1;
		svnw->svn.clips[svnw->svn.clip_count].height = svnentry->height + 1;
		svnw->svn.clip_count++;
		}
	    else 
		{
		svnw->svn.clip_count = 1;
		svnw->svn.refresh_all = TRUE;
		}

	    /*
	    **  If we are not disabled, call the draw routine
	    */
	    if (svnw->svn.disabled_count == 0) TopTreeDraw(svnw);
	    }
	};


/*
**  If the navigation window is up, change the selection there too.
*/
    if (svnw->svn.nav_window != NULL && XtIsManaged(svnw->svn.nav_window)) {
	Widget w = svnw->svn.nav_window;
	int x = (X_POS(svnw, svnentry) * XtWidth(w)) / svnw->svn.tree_width;
	int y = (Y_POS(svnw, svnentry) * XtHeight(w)) / svnw->svn.tree_height;
	int width; 
	int height;
	int arc_width = (svnw->svn.arc_width * XtWidth(w)) / svnw->svn.tree_width;
	GC  gc = svnw->svn.foreground_gc;

        
        /******************************************************************************/
        /*                                                                            */
        /* Special considerations have to be taken for the height and width ..	If    */
	/* the tree is either very large or very width... the division calculation    */
	/* render a floating point number < 1.0 but greater than 0.0.  So to even     */
	/* things out , we know we can't draw anything less than 1 pixel...           */
        /*                                                                            */
        /******************************************************************************/
	width = (svnentry->width * XtWidth(w)) / svnw->svn.tree_width;
        height = (svnentry->height * XtHeight(w)) / svnw->svn.tree_height;
	if (height == 0) height = 1;
	if (width == 0) width = 1;
	
	/*
	**  Call a support routine to erase the box in the nav window, so that
	**  our drawing here won't mess up.
	*/
	LclTopTreeUpdateNavBox(svnw, svnw->svn.nav_window, 
	    svnw->svn.mapx, svnw->svn.mapy, TRUE);

	XClearArea (XtDisplay(svnw), XtWindow(w), 
		    (Position)( (LayoutIsRtoL(svnw)) ? XtWidth (w) - x - width : x), 
		    (Position)y, 
		    (Dimension)width, 
		    (Dimension)height, FALSE);


	if (arc_width == 0) {
	    if (svnentry->selected)	
		XFillRectangle(XtDisplay(w),XtWindow(w), gc, 
			       (Position)( (LayoutIsRtoL(svnw)) ? XtWidth (w) - x - width : x), (Position)y, 
			       (Dimension)width, (Dimension)height);
	    else
		XDrawRectangle(XtDisplay(w),XtWindow(w), gc, 
			       (Position)( (LayoutIsRtoL(svnw)) ? XtWidth (w) - x - width : x), (Position)y, 
			       (Dimension)width, (Dimension)height);
	    }
	else {
	    if (svnentry->selected) {
		/*
		**  Allocate buffers and set its attributes, fill with arc
		**  data and flush.
		*/
		BUFF_ARC_INIT(Arcs, 3);
		BUFF_ARC_SET_ATTRS(Arcs, svnw, XtWindow(w), gc, TRUE);
		BUFF_ARC(Arcs, ( (LayoutIsRtoL(svnw)) ? XtWidth (w) - x - arc_width * 2 : x),
			y, arc_width * 2, height, ( (LayoutIsRtoL(svnw)) ? 90*64 : -90*64), -180*64);
		BUFF_ARC(Arcs, ( (LayoutIsRtoL(svnw)) ? XtWidth (w) - x - width : x + width - (2 * arc_width)), 
			y, arc_width * 2, height, ( (LayoutIsRtoL(svnw)) ? -90*64 : 90*64), -180*64);
		BUFF_ARC_FLUSH(Arcs);

		XFillRectangle(XtDisplay(w),XtWindow(w), gc,
		    (Position)( (LayoutIsRtoL(svnw)) ? XtWidth (w) - x + arc_width - width : x + arc_width), 
		    (Position)y, (Dimension)width - (2 * arc_width), 
		    (Dimension)height);
		}
	    else {
		/*
		**  Allocate buffers
		*/
		BUFF_SEG_INIT(Segs, 3);
		BUFF_ARC_INIT(Arcs, 3);


		/*
		**  Set attributes for buffers
		*/
		BUFF_SEG_SET_ATTRS(Segs, svnw, XtWindow(w), gc);
		BUFF_ARC_SET_ATTRS(Arcs, svnw, XtWindow(w), gc, FALSE);


		/*
		**  Do draw calls into buffers
		*/
		BUFF_SEG(Segs,
			( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - arc_width : x + arc_width), y,
			( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width + arc_width : x + width - arc_width), y);
		BUFF_SEG(Segs,
			( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - arc_width : x + arc_width), y + height,
			( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width + arc_width : x + width - arc_width), y + height);
		BUFF_ARC(Arcs,
			( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - arc_width * 2 : x), y, arc_width * 2, height,
			( (LayoutIsRtoL(svnw)) ? 90*64 : -90*64), -180*64);
		BUFF_ARC(Arcs,
			( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width : x + width - (2 * arc_width)), y, 
			arc_width * 2, height, ( (LayoutIsRtoL(svnw)) ? -90*64 : 90*64), -180*64);

		/*
		**  Flush the buffers
		*/
		BUFF_ARC_FLUSH(Arcs);
		BUFF_SEG_FLUSH(Segs);
		}
	    }


	/*
	**  Call a support routine to redraw the box
	*/
	LclTopTreeUpdateNavBox(svnw, svnw->svn.nav_window, 
	    svnw->svn.mapx, svnw->svn.mapy, FALSE);
	}


} /* TopTreeHighlightEntry */



/*
**  This routine is called when the widget is exposed or when the widget is
**  enabled.  If the refresh_all flag is set, then we clear the screen and
**  draw from scratch.  Otherwise, we simply redraw the exposed areas.  We also
**  update the nav box as needed.  If the update_nav_window flag is TRUE, then
**  the entire nav window is redrawn.
*/

void TopTreeDraw (svnw)

  svn_widget svnw;

{
/*
**  Local Variables
*/
    XEvent new_event;


/*
**  Set the window of interest for clip rectangles
*/
    svnw->svn.clips_window = XtWindow (svnw->svn.primary_window_widget);


/*
**  If this is a forced refresh then eat all pending expose events and
**  delete the clipping rectangles so everything gets drawn.
*/
    if (svnw->svn.refresh_all)
	{
	CLEAR_SCREEN(svnw);
	while (XCheckTypedWindowEvent(XtDisplay(svnw), svnw->svn.clips_window, GraphicsExpose, &new_event)) {};
	while (XCheckTypedWindowEvent(XtDisplay(svnw), svnw->svn.clips_window,         Expose, &new_event)) {};
	svnw->svn.clip_count = 0;
	svnw->svn.refresh_all = FALSE;
	}
    else
	DisplayGraphicsExpose(svnw);


/*
**  Redraw exposed (clipping rectangles set by caller)
*/
    TopTreeDrawExposed(svnw, XtWindow(svnw->svn.primary_window_widget));


/*
**  Update the nav window if requested
*/
if (svnw->svn.update_nav_window)
    {
    if ((svnw->svn.nav_window != NULL) && XtIsManaged(svnw->svn.nav_window)) {
	svnw->svn.nav_window_box_width = 0;
	LclTopTreeIndexWindowExpose(svnw->svn.nav_window, svnw);
	}
    }

else 
    {
    /*
    **  Call a support routine to update the box in the nav window
    */
    LclTopTreeUpdateNavBox(svnw, svnw->svn.nav_window, 
	svnw->svn.mapx, svnw->svn.mapy, FALSE);
    }

} /* TopTreeDraw */



/*
** This routine will draw an entire entry in the window specified at
** the x, y window position.
*/

static void LclTopTreeDrawEntry (svnw, wid, svnentry, basex, basey)

  svn_widget svnw;
  Window wid;		/* drawable to recieve output */
  DXmSvnEntryPtr svnentry;
  int basey,basex;

{
/*
**  Local data declarations
*/
    GC            gc;          /*  Entry graphics context       */
    int		  width;       /*  Width of the entry */
    int		  height;
    int		  x, y;
    int		  x2, y2;
    int		  start_angle, end_angle;
    int		  ex, ey, ew, eh;
    int		  draw_loc_cursor = FALSE;

/*
**  Default the GC to either grayed or normal
*/
    if ((svnentry->grayed) || (XtIsSensitive(svnw) == FALSE))
         gc = svnw->svn.grey_gc;
    else gc = svnw->svn.foreground_gc;

/*
**  Get dimensions out of the entry for easier reference
*/
    ex = basex;
    ey = basey;
    ew = svnentry->width;
    eh = svnentry->height;


/******************************************************************************/
/*                                                                            */
/* Set a flag to see if we have to draw location cursor around this entry.    */
/*                                                                            */
/******************************************************************************/
    if ((svnentry == svnw->svn.location_cursor_entry) &&
	(svnw->svn.last_event.state != (int)ControlMask))
	draw_loc_cursor = TRUE;


/*
**  Invert the entire space if it is selected and we own the global selections.
*/
    if ((svnentry->selected) && (svnw->svn.show_selections))
	{
	Position box_x = ( LayoutIsRtoL(svnw) ? XtWidth(svnw->svn.primary_window_widget)
                                    - ex - svnentry->width : ex );
				    
	LclTopTreeDrawBox (svnw, wid, svnentry, gc, box_x, ey,
			(svnw->svn.tree_style == DXmSvnKuserDefinedTree));

	/*
	**  Draw the filled rectangle to denote selection
	*/
	FILL_RECT_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height);
	if (LayoutIsRtoL(svnw))
	    x = XtWidth (svnw->svn.primary_window_widget) - x - width;
	XFillRectangle (XtDisplay(svnw), wid, gc, (Position)x, (Position)y, (Dimension)width, (Dimension)height);

	/******************************************************************************/
	/*                                                                            */
	/* If this entry needs to have the location cursor... draw it..		      */
	/*                                                                            */
	/******************************************************************************/
	if (draw_loc_cursor)
	    {
	    gc = svnw->svn.location_cursor_gc;

	    XDrawRectangle( XtDisplay(svnw),
			    wid,
			    gc,
			    (Position)x,
			    (Position)y+1,
			    (Dimension)width-1,
			    (Dimension)height-2);
	    }

	/*
	**  If arcs on the ends, fill them in too
	*/
	if (svnw->svn.arc_width != 0) { 
	    /*
	    **  Allocate and initialize arc buffer
	    */
	    BUFF_ARC_INIT(FilledArcs, 3);
	    BUFF_ARC_SET_ATTRS(FilledArcs, svnw, wid, gc, TRUE);


	    FILL_LEFT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, 
		width, height, start_angle, end_angle);
	    if (LayoutIsRtoL(svnw))
		{
		if ((svnw->svn.show_highlighting) && (svnentry->highlighted))
		    x = XtWidth(svnw->svn.primary_window_widget) - x - ew + 2*highlight_width;
		else
	    	    x = XtWidth(svnw->svn.primary_window_widget) - x - ew;
		}
	    BUFF_ARC(FilledArcs, x, y, width, height, start_angle, end_angle);


	    FILL_RIGHT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, 
		width, height, start_angle, end_angle);
	    if (LayoutIsRtoL(svnw))
		{
		if ((svnw->svn.show_highlighting) && (svnentry->highlighted))
		    x = XtWidth(svnw->svn.primary_window_widget) - x + ew - 2 * width - 2*highlight_width;
		else
		    x = XtWidth(svnw->svn.primary_window_widget) - x + ew - 2 * width;
		}
	    BUFF_ARC(FilledArcs, x, y, width, height, start_angle, end_angle);
	    BUFF_ARC_FLUSH(FilledArcs);
	    };

       gc = svnw->svn.background_gc;
       }


/*
**  It is not selected, but if it is highlighted draw the highlight rectangle
*/
    else if ((svnw->svn.show_highlighting) && (svnentry->highlighted)) {
	/*
	**  Draw the box around it only if requested
	*/
	    gc = svnw->svn.highlight_gc;

	if (LayoutIsRtoL(svnw))
	    LclTopTreeDrawBox (svnw, wid, svnentry, gc,
		    XtWidth (svnw->svn.primary_window_widget) - ex - svnentry->width,
		    ey, (svnw->svn.tree_style == DXmSvnKuserDefinedTree));
	else
	    LclTopTreeDrawBox (svnw, wid, svnentry, gc, ex, ey,
		    (svnw->svn.tree_style == DXmSvnKuserDefinedTree));


	gc = svnw->svn.tree_highlight_gc;

	if (svnw->svn.arc_width == 0) {
	    /*
	    **  Draw the highlight rectangle to denote selection
	    */
	    HIGHLIGHT_RECT_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height);
	    XDrawRectangle (XtDisplay(svnw), wid, gc,
		(Position)( (LayoutIsRtoL(svnw)) ? XtWidth(svnw->svn.primary_window_widget) - x - width	: x),
		(Position)y, (Dimension)width, (Dimension)height);
	    }
	else {
	    /*
	    **  Allocate buffers
	    */
	    BUFF_SEG_INIT(Segs, 3);
	    BUFF_ARC_INIT(Arcs, 3);


	    /*
	    **  Set attributes for buffers
	    */
	    BUFF_SEG_SET_ATTRS(Segs, svnw, wid, gc);
	    BUFF_ARC_SET_ATTRS(Arcs, svnw, wid, gc, FALSE);


	    /*
	    **  Draw the highlight rectangle to denote selection
	    */
	    HIGHLIGHT_TOP_COORDS(svnw, svnentry, ex, ey, ew, eh, x, y, x2, y2);
	    BUFF_SEG(Segs,
		    ( (LayoutIsRtoL(svnw)) ? XtWidth (svnw->svn.primary_window_widget) - x : x), y,
		    ( (LayoutIsRtoL(svnw)) ? XtWidth (svnw->svn.primary_window_widget) - x2 : x2), y2);
	    HIGHLIGHT_BOT_COORDS(svnw, svnentry, ex, ey, ew, eh, x, y, x2, y2);
	    BUFF_SEG(Segs,
		    ( (LayoutIsRtoL(svnw)) ? XtWidth (svnw->svn.primary_window_widget) - x : x), y,
		    ( (LayoutIsRtoL(svnw)) ? XtWidth (svnw->svn.primary_window_widget) - x2 : x2), y2);


	    HIGHLIGHT_LEFT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y,
		width, height, start_angle, end_angle);
	    if (LayoutIsRtoL(svnw))
		x = XtWidth(svnw->svn.primary_window_widget) - x - ew + highlight_width;
	    BUFF_ARC(Arcs, x, y, width, height, start_angle, end_angle);


	    HIGHLIGHT_RIGHT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, 
		width, height, start_angle, end_angle);
	    if (LayoutIsRtoL(svnw))
		x = XtWidth(svnw->svn.primary_window_widget) - x + ew - 2 * width - highlight_width;
	    BUFF_ARC(Arcs, x, y, width, height, start_angle, end_angle);


	    BUFF_ARC_FLUSH(Arcs);
	    BUFF_SEG_FLUSH(Segs);
	    };
	}


/*
**  It is not selected and not highlighted so draw the outline if requested.
*/
    else
	{
	if (svnw->svn.tree_entry_outlines)
	    {
	    /*
	    **  Draw the box around the node (if requested)
	    */
	    if (LayoutIsRtoL(svnw))
		LclTopTreeDrawBox (svnw, wid, svnentry, gc,
		    XtWidth (svnw->svn.primary_window_widget) - ex - svnentry->width, ey,
		    (svnw->svn.tree_style == DXmSvnKuserDefinedTree));
	    else
		LclTopTreeDrawBox (svnw, wid, svnentry, gc, ex, ey,
		    (svnw->svn.tree_style == DXmSvnKuserDefinedTree));
	    }

	/******************************************************************************/
	/*                                                                            */
	/* If this entry needs to have the location cursor... draw it..		      */
	/*                                                                            */
	/******************************************************************************/
	if (draw_loc_cursor)
	    {
	    gc = svnw->svn.location_cursor_gc;

	    ew = svnentry->width;
	    eh = svnentry->height;
	    
	    FILL_RECT_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height);
	    
	    if (LayoutIsRtoL(svnw))
		x = XtWidth (svnw->svn.primary_window_widget) - x - width;
	    
	    XDrawRectangle( XtDisplay(svnw),
			    wid,
			    gc,
			    (Position)x,
			    (Position)y+1,
			    (Dimension)width -1,
			    (Dimension)height - 3);
	    }

	};


/*
**  Call the common draw routine to display the contents
*/
    basex += svnw->svn.arc_width;
    if (svnw->svn.expect_highlighting) {
	basex += highlight_width;
	basey += highlight_width;
	};

    DisplayDrawEntry(svnw, svnw->svn.primary_window_widget, svnentry, basex + 1, basey + 1);

} /* LclTopTreeDrawEntry */



/*
**  This routine handles resizing of the display.  We need to update the slider
**  size/position and the size/location of the nav box in the nav window.
*/

void TopTreeResize (svnw)

  svn_widget svnw;

{
/*
**  resize the scrollbar sliders
*/
    LclTopTreeScrollAdjustSize(svnw);


/*
**  Call a support routine to update the box in the nav window
*/
    LclTopTreeUpdateNavBox(svnw, svnw->svn.nav_window, 
	svnw->svn.mapx, svnw->svn.mapy, FALSE);


} /* TopTreeResize */

/*
**  This routine is called when the user is dragging a set of entries or
**  selecting a range of entries.  This routine is passed an X and Y window
**  coordinate and must decide whether or not to autoscroll the screen or not.
**  If it scrolls the screen, it updates the box_base_y and box_base_x so that
**  the box will be drawn correctly now that the screen positions have changed.
*/

void TopTreeAutoScrollDisplay (svnw, x, y)

  svn_widget svnw;
  int x;
  int y;

{
/*
**  Local variables
*/
    int delta_x = 0, delta_y = 0;	    /* amount to scroll */


/*
**  True if to the left of the window
*/
    if (x < 0)
	delta_x -= inc_length;


/*
**  True if above the window
*/
    if (y < 0)
	delta_y -= inc_length;


/*
**  True if to the right of the visible window
*/
    if (x > REALWIDTH(svnw)) 
	delta_x += inc_length;


/*
**  True if below the visible window
*/
    if (y > REALHEIGHT(svnw)) 
	delta_y += inc_length;


/*
**  Now perform the scroll 
*/
    if (LayoutIsRtoL(svnw))
	LclTopTreePerformScroll(svnw, - delta_x, delta_y);
    else
        LclTopTreePerformScroll(svnw, delta_x, delta_y);


/*
**  Update the base of the box that is drawn on the display
*/
    svnw->svn.box_base_x = X_TO_WINDOW(svnw, svnw->svn.range_hook_x);
    if (LayoutIsRtoL(svnw))
	svnw->svn.box_base_x = XtWidth(svnw) - svnw->svn.box_base_x;
    svnw->svn.box_base_y = Y_TO_WINDOW(svnw, svnw->svn.range_hook_y);


} /* TopTreeAutoScrollDisplay */



/*
**  This routine will test autoscroll if necessary based on the X and Y.
**  It returns TRUE if the autoscroll should occur, FALSE otherwise.
*/

int TopTreeAutoScrollCheck (svnw, x, y)

  svn_widget svnw;
  int x;
  int y;

{
/*
**  True if to the left of the window
*/
    if (x < 0) return TRUE;


/*
**  True if above the window
*/
    if (y < 0) return TRUE;


/*
**  True if to the right of the visible window
*/
    if (x > REALWIDTH(svnw)) return TRUE;


/*
**  True if below the visible window
*/
    if (y > REALHEIGHT(svnw)) return TRUE;



/*
**  Otherwise no scrolling necessary
*/
    return FALSE;


} /* TopTreeAutoScrollCheck */



/*  
**  This routine is called by the STRUCTURE module whenever a valid entry is
**  desired.   It is only called if the entry attribute height_adjusted is
**  false.  Normally, this routine just calls the common
**  DisplayAdjustEntryHeight routine, unless the DXmSvnTreeCenteredComponents
**  resource is set to TRUE.  In that case it calls the
**  LclTopTreeAdjustHeightCentered routine.  After calling either of these
**  routines it adds in the necessary space for shadows and/or highlighting and
**  then sets the entry width and height.
*/

void TopTreeAdjustHeight (svnw, svnentry)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;

{	
/*
**  Force components to be centered if requested
*/
    if (svnw->svn.centered_components)
       LclTopTreeAdjustHeightCentered(svnw,svnentry);
    else
	DisplayAdjustEntryHeight(svnw,svnentry);


/*
**  Add in the space for the arcs if they are being drawn. 
*/
    if (svnw->svn.arc_width != 0) 
	svnentry->width += (2 * svnw->svn.arc_width);


/*
**  Add in the space for the highlighting, if requested. 
*/
    if (svnw->svn.expect_highlighting) {
	svnentry->width += (2 * highlight_width) + 1;
	svnentry->height += (2 * highlight_width) + 1;
	}
    else 
	svnentry->height += 2;


/*
**  Add in the space for the shadow, if requested. 
*/
    if (svnw->svn.entry_shadows && !svnw->svn.expect_highlighting) {
	svnentry->width += highlight_width;
	svnentry->height += highlight_width;
	};    


} /* TopTreeAdjustHeight */




/*  
**  This routine is called by the SVN.C module whenever a widget components
**  changes its size.  It marks the display as changed so that layout will
**  occur, the entry is specified as current, so that the display will not
**  move, and the entry is marked for resize.
*/

void TopTreeAdjustEntrySize (svnw, entry_number)

  svn_widget svnw;
  int entry_number;

{	
/*
**  Local Variables
*/
    DXmSvnEntryPtr svnentry = StructGetEntryPtr(svnw, entry_number);


/*
**  Save the position of this entry to keep the display from jumping
*/
    LclTopTreeSetCurrentEntry(svnw, entry_number);


/*
**  Mark the entry as needing to be adjusted
*/
    svnentry->height_adjusted = FALSE;

/*
**  Mark the display as being changed
*/
    svnw->svn.display_changed = TRUE;


} /* TopTreeAdjustEntrySize */



/*
**  This local routine saves the entry passed as the current entry so that upon
**  redraw of the display we can correctly position it so it doesn't move (or
**  is forced to be visible).  If there is already a current_entry this routine
**  does not update it to allow multiple add entries to redisplay in a better
**  location.
*/

static void LclTopTreeSetCurrentEntry (svnw, entry_number)

    svn_widget svnw;	
    int entry_number;	    /* entry number to save */

{
    DXmSvnEntryPtr svnentry;   /* entry to save */
    int x, y;		    /* position of entry */


/*
**  If there is already a current entry, or entry 0 was specified, or the
**  display has changed and we haven't done the new layout yet, then do nothing
*/
    if (svnw->svn.display_changed) return;
    if (svnw->svn.current_entry != NULL) return;
    if (entry_number == 0) return;
    if (svnw->svn.vscroll == NULL) return;


/*
**  Get the entry information
*/
    svnentry = StructGetEntryPtr (svnw, entry_number);
    x = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));
    y = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));


    
    /******************************************************************************/
    /*                                                                            */
    /* If this entry wasn't previously visible... position it according to its    */
    /* tree style.								  */
    /*                                                                            */
    /******************************************************************************/
    if  (((x + (int)svnentry->width) < 0) || (x > REALWIDTH(svnw)))
	{
	switch (svnw->svn.tree_style)
	    {
	    case DXmSvnKtopTree :	x = (REALWIDTH(svnw) / 2) - (svnentry->width / 2);
				break;
	    case DXmSvnKhorizontalTree:  x = svnw->svn.level_spacing;
				break;
	    case DXmSvnKoutlineTree:
				x = svnw->svn.level_spacing;
	    }
	}
	

    if (((y + (int)svnentry->height) < 0) || (y > REALHEIGHT(svnw))) 
	{
	switch (svnw->svn.tree_style)
	    {
	    case DXmSvnKtopTree :	y = REALHEIGHT(svnw) / 2;
				break;
	    case DXmSvnKhorizontalTree:  y = REALHEIGHT(svnw) / 2;
				break;
	    case DXmSvnKoutlineTree:
				y = svnw->svn.sibling_spacing;
	    }
	}


/*
**  Store the information on the current entry 
*/
    svnw->svn.current_entry = svnentry;
    svnw->svn.prevx = x;
    svnw->svn.prevy = y;


} /* LclTopTreeSetCurrentEntry */



/*
**  This local routine determines if two rectangles intersect in world
**  coordinates.  The first rectangle is specified by a position and width, the
**  second by the coordinates of its bounding box. It returns true or false.
*/

static Boolean LclTopTreeIntersects (x, y, width, height, low_x, low_y, high_x, high_y)

   int x, y, width, height, low_x, low_y, high_x, high_y;

{
/*
**  If the top line in the entry is beyond the lowest line in the rectangle
**  then it can't intersect...
*/
    if (y > high_y) return FALSE;


/*
**  If the bottom line in the entry is above the top line in the rectangle
**  then it can't intersect...
*/
    if ((y + height) < low_y) return FALSE;


/*
**  If the left line in the entry is to the right of the right line in the
**  rectangle, then it can't intersect
*/
    if (x > high_x) return FALSE;


/*
**  If the right line in the entry is to the left of the left line in the
**  rectangle, then it doesn't intersect
*/
    if ((x + width) < low_x) return FALSE;


/*
**  It intersects...
*/
    return TRUE;


} /* LclTopTreeIntersects */

#define K_TO_LEFT	1
#define K_TO_RIGHT	2
#define K_TO_BOTTOM	4
#define K_TO_TOP	8		    


/******************************************************************************/
/*                                                                            */
/*  encode_points							      */
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*									      */
/* This routine will use the Cohen-Sutherland coding scheme to assist region  */
/* codes to a point, which identifies the coordinate region of the point in   */
/* relation to the specified rectangle.					      */
/*									      */
/*									      */
/*									      */
/*	These codes have two significant properties:                          */
/*                                                                            */
/*	1.  If the CODE of two points are each zero, both points are          */
/*	    within the window and no clipping needs to be done.               */
/*                                                                            */
/*	2.  If the logical AND of the CODEs of two points is non-zero,        */
/*	    the points are both outside of the window and are so              */
/*	    positioned that the line joining them does not intersect          */
/*	    the window. (I.e., it is a line segment which need not            */
/*	    be even partially displayed).                                     */
/*                                                                            */
/*                                                                            */
/*			|			|                             */
/*		1001	|	  1000		|    1010                     */
/*			|			|                             */
/*    YT----------------+-----------------------+----------------             */
/*			|			|                             */
/*			|	 Window		|                             */
/*			|			|                             */
/*		0001	|	  0000		|    0010                     */
/*			|			|                             */
/*    YB----------------+-----------------------+----------------             */
/*			|			|                             */
/*		0101	|	  0100		|    0110                     */
/*			|			|                             */
/*		       XL			XR                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void encode_point(x,y,code, bx1, by1, bx2, by2)
    long int	x;
    long int	y;
    char       *code;
    int bx1, by1, bx2, by2;		    /* upper-right and lower-left corners of box */


    {
    
    
    *code = 0;	    /* Initialize code to be 0000 - in  RECTANGLE */
    
    if (x < bx1)
	*code = (*code | K_TO_LEFT);
    if (x > bx2)
	*code = (*code | K_TO_RIGHT);
    if (y < by1)
	*code = (*code | K_TO_BOTTOM);
    if (y > by2)
	*code = (*code | K_TO_TOP);

    }




/******************************************************************************/
/*                                                                            */
/* This routine will swap P1 for P2 only if it is not in the window...	      */
/*                                                                            */
/******************************************************************************/
Boolean swap_points_if_needed (x1, y1, x2, y2, code1, code2)
    long int	*x1, *y1, *x2, *y2;	
    char	*code1, *code2;		
    {
    long int	temp_int;
    char	temp_code;
    
    if (! *code1)				
	{
	temp_int = *x1;
	*x1 = *x2;
	*x2 = temp_int;

	temp_int = *y1;
	*y1 = *y2;
	*y2 = temp_int;

	temp_code = *code1;
	*code1 = *code2;
	*code2 = temp_code;
	return (TRUE);
	}
     else
        return(FALSE);

     }	    /* End of routine swap_points_if_needed*/	
	

/******************************************************************************/
/*                                                                            */
/*  LclTopTreeLineIntersects						      */
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*  This local routine determines if a line intersects the specified rectangle*/
/*  in device X coordinates.  The line is specified by its end points, and the*/
/*  rectangle by the corners of its bounding box.  It returns true or false.  */
/*									      */
/*  This routine has been rewritten so that we could handle cases where the   */
/*  line has to be clipped because the value of a device coordinate specified */
/*  for the SVN entry is larger than a legal SHORT value.		      */
/*									      */
/* This routine will use the Cohen-Sutherland coding scheme to assign region  */
/* codes to a point, which identifies the coordinate region of the point in   */
/* relation to the specified rectangle.					      */
/*									      */
/* The above routine "encode_points" will assign these code values ...	      */
/*									      */
/******************************************************************************/
static Boolean LclTopTreeLineIntersects (a_x1,a_y1,
					 a_x2,a_y2,
					 bx1, by1,
					 bx2, by2)

   long int *a_x1, *a_y1, *a_x2, *a_y2;	    /* address of two endpoint of the line */
   int bx1, by1, bx2, by2;		    /* upper-right and lower-left corners of box */

   {
   float slope;	    			    /* slope in the line equation */
   Boolean intersects = FALSE;		    /* flag indicating whether line intersects rect. */
   Boolean done = FALSE;		    /* loop flag */
   char   code1, code2;			    /* Region codes for Cohen-Sutherland algorithm */
   Boolean swapped = FALSE;		    /* flag stating if the points have switched places */
    
   long int x1 = *a_x1;
   long int y1 = *a_y1;
   long int x2 = *a_x2;
   long int y2 = *a_y2;
	    
#define accept_point(code1, code2) (!(code1 | code2))
#define reject_point(code1, code2) ((code1 & code2))                
    
    /******************************************************************************/
    /*                                                                            */
    /* Get the slope of the line once ... since this slope does not change ...    */
    /* NOTE... we don't want to get the slope of the line if this is a horizontal */
    /* line.								          */
    /*                                                                            */
    /* Can't just do a straight slope calculation here.. must first check to      */
    /* see if we would get a negative slope ...					  */
    /*										  */
    /******************************************************************************/
    if (x1 != x2)
	slope = (float)(y2 - y1) / (float)(x2 - x1);
	    


	while (!done)		/* calculate both points out */
	    {
            /******************************************************************************/
            /*                                                                            */
            /* First encode both points to region codes.				  */
            /*                                                                            */
            /******************************************************************************/
	    encode_point(x1, y1, &code1, bx1, by1, bx2, by2);
	    encode_point(x2, y2, &code2, bx1, by1, bx2, by2);

	    /* If both points are accepted... in region... just return success */

	    if (accept_point(code1, code2))
		{
		done = TRUE;
		intersects = TRUE;
		}
	    else
                {
                /******************************************************************************/
                /*                                                                            */
                /* Now check to see if they were both rejected.. if so just return not success.*/
                /*                                                                            */
                /******************************************************************************/
		if (reject_point(code1, code2))
		    {
		    done = TRUE;
		    intersects = FALSE;
		    }

		else
		    {
                    /******************************************************************************/
                    /*                                                                            */
                    /* We need to make sure that this point is outside the window.  If not, we    */
		    /* will swap the point with the other point of the line ... We are guaranteed */
		    /* that at least one point is outside the window, since if both were inside   */
		    /* the line would have already been accepted above...			  */ 
                    /*                                                                            */
		    /* Assign the local variable 'swapped' the XOR value of the whether the points */
		    /* have been swapped.  We need to save this state, so that at the end of      */
		    /* this routine, we will know whether or not to swap back the points.         */
		    /*										  */
                    /******************************************************************************/
		    swapped ^= swap_points_if_needed(&x1, &y1, &x2, &y2, &code1, &code2);

		    /* Handle horizontal case specially */

		    if (x1 == x2)
			{
			if (code1 & K_TO_TOP)
			    y1 = by2;
			else
			    y1 = by1;
			}
		    else
			/* all other lines ... */
			{

			/* Handle lines that might be from the left */

			if (code1 & K_TO_LEFT)
			    {
			    y1 = (long int) (y1 + (bx1 - x1) * slope);
			    x1 = bx1;
			    }

			/* Handle lines that might be from the right */

			else if (code1 & K_TO_RIGHT)
			    {
			    y1 = (long int) (y1  + (bx2 - x1) * slope);
			    x1  = bx2;
			    }

			/* Handle lines that might be from the 	bottom of the rectangle */
			    
		    	else if (code1 & K_TO_BOTTOM)
			    {
			    x1 = (long int) (x1 + (by1 - y1) / slope);
			    y1 = by1;
			    }

			/* Handle lines that might be from the top of the rectangle */

			else if (code1 & K_TO_TOP)
			    {
			    x1 = (long int) (x1 + (by2 - y1) / slope);
			    y1 = by2;
			    }

			}  /* End of section LINE not a horizontal line */

		    }	/* End of section .. lin not rejected */
		
		} /* End of section... line not accepted */


	    }  /* End of WHILE loop */


    
    /******************************************************************************/
    /*                                                                            */
    /* If this line intersects the display window... then we will want to copy    */
    /* the new coordinates that intersect the display rectangle into the original */
    /* coordinate variables passed to this routine.  If the points had been	  */
    /* swapped during the above algorithm process, make sure we unswap them before*/
    /* we copy them back to the calling routine.				  */
    /*                                                                            */
    /******************************************************************************/
     if (intersects)
	{
	if (!swapped)
	    {
	    *a_x1 = x1;
	    *a_y1 = y1;		
	    *a_x2 = x2;
	    *a_y2 = y2;		
	    }
	else
	    {
	    *a_x1 = x2;
	    *a_y1 = y2;		
	    *a_x2 = x1;
	    *a_y2 = y1;		
	    }
	}
	
	
    return (intersects);


} /* End of routine LclTopTreeLineIntersects */



/*
**  This local routine determines if the connection line intersects the
**  specified rectangles in window coordinates.  If so, it draws the segment(s)
**  that are visible.
*/

static void LclTopTreeDrawConnection (svnw, x1, y1, x2, y2, 
			       bx1, by1, bx2, by2, ConSegs)

   svn_widget svnw;
   int x1, y1, x2, y2;	    /* two endpoint of the line */
   int bx1, by1, bx2, by2;  /* upper-right and lower-left corners of box */
   BUFF_SEG_INIT(*ConSegs,500); 

{
/*
**  Local Variables
*/
    DXmSvnPoint points[max_points];						    
    int num_points;
    int i;


/*
**  Get the points in the connection line
*/
    CONNECTION_DATA (svnw, points, num_points, x1, y1, x2, y2, svnw->svn.level_spacing/2);


/*
**  If any of the lines are visible then draw them
*/
    if (LayoutIsRtoL(svnw))
	{
	for (i = 0; i < num_points; i++)
	    points[i].x = XtWidth (svnw->svn.primary_window_widget) - points[i].x;
	for (i = num_points - 1; i > 0; i--) 
	    {
	    /*
	    **   I make clip box 2 pixels wider because I don't understand
	    **   an algorithm of LclTopTreeLineIntersects. It makes last segment
	    **   inclined and leaves one or two pixels of it inside the clip
	    **   area. Extended box puts additional pixels out of clip area and
	    **   X clips them out.
	    */
	    if (LclTopTreeLineIntersects(&points[i].x,&points[i].y,
			&points[i-1].x,&points[i-1].y, bx1-1, by1, bx2+1, by2))
		BUFF_SEG((*ConSegs), points[i-1].x, points[i-1].y, points[i].x,points[i].y);		
	    }
	}
    else
	for (i = 1; i < num_points; i++) 
	    {
	    if (LclTopTreeLineIntersects(&points[i].x,&points[i].y,
			&points[i-1].x,&points[i-1].y, bx1, by1, bx2, by2))
		{
		BUFF_SEG((*ConSegs), points[i-1].x, points[i-1].y, points[i].x, points[i].y);
		}
	    }


} /* LclTopTreeDrawConnection */



/*
**  This local routine determines if the connection line intersects the
**  specified rectangles in window coordinates.  If so, it draws the segment(s)
**  that are visible.
*/

static void LclTopTreeDrawEntryConnection (svnw, svnentry, bx1, by1, bx2, by2, ConSegs)

   svn_widget svnw;
   int bx1, by1, bx2, by2;  /* upper-right and lower-left corners of box */
   DXmSvnEntryPtr svnentry;
   BUFF_SEG_INIT(*ConSegs, 500);

{
/*
**  Local variables
*/
    int num_points;
    DXmSvnPoint points[max_points];
    int px, py, x, y;
    int turn_length;
    int i;


/*
**  Specify where a perpendicular line will turn
*/
    px = X_TO_WINDOW(svnw, PX_POS(svnw, svnentry));
    py = Y_TO_WINDOW(svnw, PY_POS(svnw, svnentry));
    x = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));
    y =	Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));

    turn_length = svnw->svn.level_spacing/2;
    if (svnw->svn.tree_style == DXmSvnKtopTree) {
	x += svnentry->width/2; 
	}
    else {
	y += svnentry->height/2; 
	}

/*
**  Draw the lines
*/
    CONNECTION_DATA (svnw, points, num_points, px, py, x, y, turn_length);
    for (i = 1; i < num_points; i++)
	if (LclTopTreeLineIntersects(&points[i].x,&points[i].y,
	    &points[i-1].x,&points[i-1].y, bx1, by1, bx2, by2)) {
	    BUFF_SEG((*ConSegs), 
		    ( (LayoutIsRtoL(svnw)) ? XtWidth (svnw->svn.primary_window_widget) - points[i-1].x : points[i-1].x),
		    points[i-1].y,
		    ( (LayoutIsRtoL(svnw)) ? XtWidth (svnw->svn.primary_window_widget) - points[i].x : points[i].x),
		    points[i].y);
	    }

} /* LclTopTreeDrawEntryConnection */





/*
**  This local routine determines if an entry is visible.  It returns true or
**  false.
*/

static Boolean LclTopTreeEntryVisible(svnw, svnentry)

    svn_widget	svnw;
    DXmSvnEntryPtr svnentry;

{
/* 
**  Local variables
*/
    int x = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));
    int y = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));


    
/*
**  Return FALSE if it isn't visible
*/
    if (y > REALHEIGHT(svnw)) return FALSE;
    if (x > REALWIDTH(svnw)) return FALSE;
    if ((y + (int)svnentry->height) < 0) return FALSE;
    if ((x + (int)svnentry->width) < 0) return FALSE;

/*
**  It's visible so return TRUE
*/
    return TRUE;


} /* LclTopTreeEntryVisble */



/*
**  This routine is called when size of the vertical scroll bar needs to be
**  updated.  The current value of the scroll bar is queried and will change
**  only when the size actually changes to prevent flicker.
*/

static void LclTopTreeScrollAdjustSize (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations.  The normal names are unnormalized while the
**  names prefixed with n are the normalized (1 to 100) values.
*/
    Arg arglist[4];
    int argcount;

    int cvalue, cshown, cmax, cmin;
    int  value,  shown, max;


/*
**  Get the current vertical scroll bar values.
*/
    XtSetArg(arglist[0], XmNvalue,    &cvalue);
    XtSetArg(arglist[1], XmNsliderSize, &cshown);
    XtSetArg(arglist[2], XmNmaximum,    &cmax);
    XtSetArg(arglist[3], XmNminimum,    &cmin);
    XtGetValues (svnw->svn.vscroll, arglist, 4);


    /*
    **  Set the vertical slider size and position based upon the percentage visible.
    */
    if (svnw->svn.tree_height == 0)
    {
	max = 3;
	shown = 1;
	value = 1;
    }
    else
    {
	shown = REALHEIGHT(svnw);
	if (svnw->svn.mapy < 0)
	    value = 0;
	else
	    value = svnw->svn.mapy;
	max = svnw->svn.tree_height;

	/*
	**  If the tree is smaller than the screen then mapy may be negative.
	**  Make sure the scroll bar shows this.
	*/
	if (shown > svnw->svn.tree_height)
	{ 
	    if (svnw->svn.mapy >= 0)
	    {
		shown = svnw->svn.tree_height - svnw->svn.mapy;
		value = svnw->svn.mapy;
	    }
	    else
		shown = svnw->svn.tree_height;
	}
    }

    if (value > max - shown) value = max - shown;

/*
**  Set the vertical scrollbar values if they are different
*/
    argcount = 0;
    if (cvalue != value) {XtSetArg(arglist[argcount], XmNvalue,    value);  argcount++; };
    if (cshown != shown) {XtSetArg(arglist[argcount], XmNsliderSize,    shown);  argcount++; };
    if (cmin != 0) {XtSetArg(arglist[argcount], XmNminimum, 0);  argcount++; };
    if (cmax != max) {XtSetArg(arglist[argcount], XmNmaximum, max);  argcount++; };
    if (argcount != 0) XtSetValues (svnw->svn.vscroll, arglist, argcount);


/*
**  Get the current horizontal scroll bar values.
*/
    XtSetArg(arglist[0], XmNvalue,    &cvalue);
    XtSetArg(arglist[1], XmNsliderSize, &cshown);
    XtSetArg(arglist[2], XmNmaximum,    &cmax);
    XtSetArg(arglist[3], XmNminimum,    &cmin);
    XtGetValues (svnw->svn.hscroll, arglist, 4);

    /*
    **  Set the slider based upon the percentage visible.
    */
    if (svnw->svn.tree_width == 0)
    {
	max = 3;
	shown = 1;
	value = 1;
    }
    else
    {
	shown = REALWIDTH(svnw);
	if (svnw->svn.mapx < 0)
	    value = 0;
	else
	    value = svnw->svn.mapx;
	max = svnw->svn.tree_width;

	/*
	**  If the tree is smaller than the screen then mapx may be negative.
	**  Make sure the scroll bar shows this.
	*/
	if (shown > svnw->svn.tree_width)
	{ 
	    if (svnw->svn.mapx >= 0)
	    {
		shown = svnw->svn.tree_width - svnw->svn.mapx;
		value = svnw->svn.mapx; 
	    }
	    else
		shown = svnw->svn.tree_width;
	}
    }

    if (value > max - shown) value = max - shown;

/*
**  Set the horizontal scrollbar values if they are different
*/
    argcount = 0;
    if (cvalue != value) {XtSetArg(arglist[argcount], XmNvalue,    value);  argcount++; };
    if (cshown != shown) {XtSetArg(arglist[argcount], XmNsliderSize,    shown);  argcount++; };
    if (cmin != 0) {XtSetArg(arglist[argcount], XmNminimum, 0);  argcount++;};
    if (cmax != max) {XtSetArg(arglist[argcount], XmNmaximum, max);  argcount++; };
    if (argcount != 0) XtSetValues (svnw->svn.hscroll, arglist, argcount);

} /* LclTopTreeScrollAdjustSize */

/*
**  This callback routine gets all of the vertical scroll bar callbacks. 
*/

void TopTreeVScroll (w, scroll)

  Widget w;
  XmScrollBarCallbackStruct *scroll;

{
/*
**  The parent of the vertical scroll bar is the svn widget
*/
    svn_widget svnw = StructFindSvnWidget (w);


/*
**  XmCR_HELP reason code
*/
    if (scroll->reason == XmCR_HELP)
       {
         DXmSvnCallbackStruct temp;
         temp.reason = DXmSvnCRHelpRequested;
         temp.entry_number = -1;
         DXmSvnDisableDisplay ((Widget) svnw);
         XtCallCallbacks ((Widget)svnw, DXmSvnNhelpRequestedCallback, &temp);
         DXmSvnEnableDisplay ((Widget) svnw);
         return;
       };


/*
**  Case on the reason code for the rest
*/
    switch (scroll->reason)

      {
        case XmCR_VALUE_CHANGED: 
	    /*
	    **	If we had an index window remove it
	    */
	    if (svnw->svn.index_window != 0) 
		{
		XtUnmapWidget(svnw->svn.index_window);
		}

	    svnw->svn.index_window_shown = FALSE;

	    /*
	    **	Update the scroll bars to the new position unless we are
	    **	updating the screen (in that case the scroll bars are already
	    **	correct).
	    */
	    if (!svnw->svn.update_in_progress) {
		LclTopTreePerformScroll(svnw, 0, scroll->value - svnw->svn.mapy);
		};
            break;

        case XmCR_INCREMENT: 
	    LclTopTreePerformScroll(svnw, 0, inc_length);
            break;

        case XmCR_DECREMENT:
	    LclTopTreePerformScroll(svnw, 0, -inc_length);
            break;

        case XmCR_PAGE_INCREMENT: 
	    LclTopTreePerformScroll(svnw, 0, REALHEIGHT(svnw));
	    break;

        case XmCR_PAGE_DECREMENT: 
	    LclTopTreePerformScroll(svnw, 0, -REALHEIGHT(svnw));
	    break;

        case XmCR_DRAG: 
	    /*
	    **	Update the position of the drag in the index window
	    */
	    LclTopTreeScrollDrag(svnw, svnw->svn.mapx, scroll->value);
	    break;

        case XmCR_TO_TOP: 
	    LclTopTreePerformScroll(svnw, 0, scroll->pixel);
	    break;

        case XmCR_TO_BOTTOM: 
	    LclTopTreePerformScroll(svnw, 0, -(REALHEIGHT(svnw) - scroll->pixel));
	    break;

      };


} /* LclTopTreeVScroll */



/*
**  This routine gets all of the horizontal scroll callbacks.
*/

void TopTreeHScroll (w, scroll)

  Widget w;
  XmScrollBarCallbackStruct *scroll;

{
/*
**  The parent of the vertical scroll bar is the svn widget
*/
    svn_widget svnw = StructFindSvnWidget (w);
    DXmSvnCallbackStruct temp;


/*
**  Switch on the reason code
*/
    switch (scroll->reason)

      {
        case XmCR_VALUE_CHANGED: 
	    /*
	    **	If we had an index window remove it
	    */
	    if (svnw->svn.index_window != 0) {
		XtUnmapWidget(svnw->svn.index_window);
		};

	    svnw->svn.index_window_shown = FALSE;

	    /*
	    **	If we are updating the screen then we have already set the
	    **	slider size, don't do it again.
	    */
	    if (!svnw->svn.update_in_progress) {
		LclTopTreePerformScroll(svnw, scroll->value - svnw->svn.mapx, 0);
		};
	    break;

        case XmCR_INCREMENT: 
	    LclTopTreePerformScroll(svnw, inc_length, 0);
	    break;

        case XmCR_DECREMENT: 
	    LclTopTreePerformScroll(svnw, -inc_length, 0);
	    break;

        case XmCR_PAGE_INCREMENT: 
	    LclTopTreePerformScroll(svnw, REALWIDTH(svnw), 0);
	    break;

        case XmCR_PAGE_DECREMENT: 
	    LclTopTreePerformScroll(svnw, -REALWIDTH(svnw), 0);
	    break;

        case XmCR_TO_TOP: 
	    LclTopTreePerformScroll(svnw, scroll->pixel, 0);
	    break;

        case XmCR_TO_BOTTOM: 
	    LclTopTreePerformScroll(svnw, -(REALWIDTH(svnw) - scroll->pixel), 0);
	    break;

        case XmCR_DRAG: 
	    /*
	    **	Update the position of the drag in the index window
	    */
	    LclTopTreeScrollDrag(svnw, 
		scroll->value,
		svnw->svn.mapy);
	    break;

        case XmCR_HELP: 
	     temp.reason = DXmSvnCRHelpRequested;
	     temp.entry_number = -1;
	     DXmSvnDisableDisplay ((Widget) svnw);
	     XtCallCallbacks ((Widget)svnw, DXmSvnNhelpRequestedCallback, &temp);
	     DXmSvnEnableDisplay ((Widget) svnw);
	     break;
      };


} /* LclTopTreeHScroll */



/*
**  This routine scrolls the display the requested amount along the X and Y
**  axis.  It prevents the scrolling from overshooting the visible display and
**  causes the display to redraw to expose the new portions.  If one of the
**  delta's is zero then no scrolling is done in that direction.  If the
**  resulting change in the display is less than one page then a copy area is
**  done to move the data displayed to the new location otherwise the screen
**  is jumped to the new page of data.
*/
static void LclTopTreePerformScroll (svnw, delta_x, delta_y)

    svn_widget	svnw;
    int		delta_y;

{
/*
**  Local variables
*/
    int	copy_src_y;	/* src y for scroll */
    int	copy_src_x;	/* src y for scroll */
    int copy_dst_y;	/* dest x for scroll */
    int copy_dst_x;	/* dest x for scroll */
    int clear_y;	/* x coordinate of exposed region to be cleared */
    int clear_x;	/* x coordinate of exposed region to be cleared */
    int clear_width;	/* width of rectangle to be cleared */
    int clear_height;	/* height of rectangle to be cleared */
    int actual_delta_y;	/* actual change in display position */
    int actual_delta_x;	/* actual change in display position */
    int abs_delta_y;	/* absolute value of actual_delta_y */
    int abs_delta_x;	/* absolute value of actual_delta_x */
    int new_mapx;	/* mapx position after scroll */
    int new_mapy;	/* mapy position after scroll */
		    

/*
**  Update the Y mapping postion, and keep the tree visible
*/
    actual_delta_y = svnw->svn.mapy;
    new_mapy = svnw->svn.mapy + delta_y;

    if (delta_y != 0) {
	/*
	**  If tree is smaller than window force it to be centered
	*/
	if (svnw->svn.tree_height < REALHEIGHT(svnw)) {
	    new_mapy = svnw->svn.tree_height/2 - REALHEIGHT(svnw)/2;
	    if ((svnw->svn.tree_style == DXmSvnKtopTree)) 
		new_mapy = 0;
	    if ((svnw->svn.tree_style == DXmSvnKoutlineTree)) 
		new_mapy = 0;
	    }
	else {
	    if (new_mapy < 0) new_mapy = 0;
	    if (new_mapy > (svnw->svn.tree_height - REALHEIGHT(svnw))) 
		new_mapy = svnw->svn.tree_height - REALHEIGHT(svnw);
	    }
	}

    actual_delta_y -= new_mapy;


/*
**  Update the X mapping postion, and keep the tree visible
*/
    actual_delta_x = svnw->svn.mapx;
    new_mapx = svnw->svn.mapx + delta_x;

    if (delta_x != 0) {
	if (svnw->svn.tree_width < REALWIDTH(svnw)) {
	    new_mapx = svnw->svn.tree_width/2 - REALHEIGHT(svnw)/2;
	    if ((svnw->svn.tree_style == DXmSvnKhorizontalTree)) 
		new_mapx = 0;
	    if ((svnw->svn.tree_style == DXmSvnKoutlineTree)) 
		new_mapx = 0;
	    }
	else {
	    if (new_mapx > (svnw->svn.tree_width - REALWIDTH(svnw)))
		new_mapx = svnw->svn.tree_width - REALWIDTH(svnw);
	    if (new_mapx < 0) new_mapx = 0;
	    }
	}

    actual_delta_x -= new_mapx;


/*
**  Set the source and destination y based on sign of actual_delta_y
*/
    if (actual_delta_y < 0) {
	abs_delta_y = -actual_delta_y;
	copy_src_y = abs_delta_y;
	copy_dst_y = 0;
	clear_y = REALHEIGHT(svnw) - abs_delta_y;
	clear_height = abs_delta_y;
	}
    else if (actual_delta_y > 0) {
	abs_delta_y = actual_delta_y;
	copy_src_y = 0;
	copy_dst_y = abs_delta_y;
	clear_y = 0;
	clear_height = abs_delta_y;
	}
    else {
	clear_height = 0;
	abs_delta_y = 0;
	copy_src_y = 0;
	copy_dst_y = 0;
	};
	

/*
**  Set the source and destination x based on sign of actual_delta_x
*/
    if (actual_delta_x < 0) {
	copy_src_x = -actual_delta_x;
	copy_dst_x = 0;
	abs_delta_x = -actual_delta_x;
	clear_x = REALWIDTH(svnw) - abs_delta_x;
	clear_width = XtWidth(svnw) - clear_x;
	}
    else if (actual_delta_x > 0) {
	copy_src_x = 0;
	copy_dst_x = actual_delta_x;
	abs_delta_x = actual_delta_x;
	clear_x = 0;
	clear_width = abs_delta_x;
	}
    else {
	clear_width = 0;
	abs_delta_x = 0;
	copy_src_x = 0;
	copy_dst_x = 0;
	};

    if (LayoutIsRtoL(svnw))
	{
	register t = copy_src_x;

	copy_src_x = copy_dst_x;
	copy_dst_x = t;
	clear_width = abs_delta_x;

	if (actual_delta_x > 0)
	    clear_x = XtWidth(svnw->svn.primary_window_widget) - abs_delta_x;
	else 
	    clear_x = 0;
	}
/*
**  If there are any widgets present, scroll them too.
*/
    if (svnw->svn.sub_widgets_used)
	LclTopTreeScrollWidgets(svnw, actual_delta_x, actual_delta_y);


/*
**  If we are scrolling more than a page, then don't do copy, just refresh
**  the screen and return.
*/
    if ((abs_delta_y > REALHEIGHT(svnw)) || (abs_delta_x > REALWIDTH(svnw))) {
	svnw->svn.refresh_all = TRUE;
	svnw->svn.mapx -= actual_delta_x;
	svnw->svn.mapy -= actual_delta_y;
	if (svnw->svn.disabled_count == 0) TopTreeDraw(svnw);
	return;
	};	


/*
**  Set the window of interest for clip rectangles
*/
    svnw->svn.clips_window = XtWindow (svnw->svn.primary_window_widget);


/*
**  Update the display if it has changed
*/
    if ((actual_delta_y != 0) || (actual_delta_x != 0)) {
	/*
	**  Move the display by actual_delta_y and actual_delta_x pixels
	*/
	XCopyArea (XtDisplay(svnw), svnw->svn.clips_window, XtWindow(svnw->svn.primary_window_widget), 
		   svnw->svn.copyarea_gc, copy_src_x, copy_src_y, 
		   REALWIDTH(svnw) - abs_delta_x, 
		   REALHEIGHT(svnw) - abs_delta_y, 
		   copy_dst_x, copy_dst_y);
	}
    else {
	/*
	**  No actual change, so just return;
	*/
	return;
	}

/*
**  Redraw any graphics exposures that may have occurred because of obscuring
**  windows. NOTE: We have intentionally not yet updated the map position until
**  we moved the widgets
*/
    DisplayGraphicsExpose(svnw);
    svnw->svn.mapx -= actual_delta_x;
    svnw->svn.mapy -= actual_delta_y;


/*
**  Clear the area that needs to be redrawn due to the scroll in the Y
**  direction, and set the clipping rectangles for the exposed area.
*/
    if (actual_delta_y != 0) {
#ifndef OMIT_SCROLL_CLEAR
        XClearArea (XtDisplay(svnw), svnw->svn.clips_window, 0, (Position)clear_y, 
		    XtWidth(svnw), (Dimension)clear_height, FALSE);
#endif	

	if (svnw->svn.clip_count < max_clips) 
	    {
	    svnw->svn.clips[svnw->svn.clip_count].x = 0;
	    svnw->svn.clips[svnw->svn.clip_count].y = clear_y;
	    svnw->svn.clips[svnw->svn.clip_count].width = XtWidth(svnw);
	    svnw->svn.clips[svnw->svn.clip_count].height = clear_height;
	    svnw->svn.clip_count++;
	    }
	else
	    {
	    svnw->svn.clip_count = 0;
	    svnw->svn.refresh_all = TRUE;
	    }
	};
    

/*
**  Clear the area that needs to be redrawn due to the scroll in the X
**  direction.
*/
    if (actual_delta_x != 0) {
#ifndef OMIT_SCROLL_CLEAR
	XClearArea (XtDisplay(svnw), svnw->svn.clips_window, 
		    (Position)clear_x, 0, (Dimension)clear_width, XtHeight(svnw), FALSE);
#endif	

	if (svnw->svn.clip_count < max_clips) 
	    {
	    svnw->svn.clips[svnw->svn.clip_count].x = clear_x;
	    svnw->svn.clips[svnw->svn.clip_count].y = 0;
	    svnw->svn.clips[svnw->svn.clip_count].width = clear_width;
	    svnw->svn.clips[svnw->svn.clip_count].height = XtHeight(svnw);
	    svnw->svn.clip_count++;
	    }
	else
	    {
	    svnw->svn.clip_count = 0;
	    svnw->svn.refresh_all = TRUE;
	    }
	};

/*
**  Force a sync to get straggling events.  Call the DisplayExpose routine
**  to adjust any expose events that may have moved.
*/
#ifdef SYNC_ON_SCROLL
    XSync(XtDisplay(svnw),FALSE);
#endif
    DisplayExpose(svnw, actual_delta_x, actual_delta_y);

/*
**  If we are not disabled, then redraw newly exposed areas
*/
    if (svnw->svn.disabled_count == 0) {
	TopTreeDraw(svnw);
	}


} /* LclTopTreePerformScroll*/



/*
**  This routine moves all widgets that are visible on the screen the
**  specified number of pixels in the x and y directions.
*/
static void LclTopTreeScrollWidgets(svnw, delta_x, delta_y)

    svn_widget	svnw;
    int		delta_x, delta_y;

{
    register int i, j;	    /* loop index */
    DXmSvnEntryPtr	svnentry;   /* entry to display */
    int x, y;		    /* position of entry in window corrdinates */
    int comp_x, comp_y;	    /* position to move widget to */
		    

/*
**  loop through all of the entries and move their widget components if
**  they are visible.
*/
    for (i = 1; i <= svnw->svn.num_entries; i++) {
	svnentry = StructGetValidEntryPtr (svnw, i);
	y = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));
	x = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));

	/*
	** Only move the widget if it is visible
	*/
	if (LclTopTreeEntryVisible(svnw, svnentry)) {
	    /*
	    ** Find and move the widget components of this entry
	    */
	    for (j = 0;  j <= svnentry->num_components-1;  j++)
	        { 
		if (svnentry->entrycompPtr[j].type == DXmSvnKcompWidget)  {
		    comp_x = x + svnentry->entrycompPtr[j].x + delta_x;
		    comp_y = y + svnentry->entrycompPtr[j].y + delta_y;
		    if (svnw->svn.expect_highlighting) {
			comp_x = comp_x + 2;
			comp_y = comp_y + 3;
			};

		    HIDE_WIDGET(svnentry->entrycompPtr[j].var.is_widget.readwrite_text);
		    };
	        };
	    };
	};


} /* LclTopTreeScrollWidgets */



/*
**  This routine is called when the application has called the routine
**  DXmSvnSetApplDragging.  
*/

void TopTreeSetApplDragging (svnw, value)

  svn_widget svnw;
  int value;

{
/*
**  If it is being turned on.  We must also create the ghost for them.
*/
    if (value)
       {
          TopTreeCreateGhost (svnw);
       };


/*
**  If it is being turned off, then we must destroy the ghost.
*/
    if (!value)
       {
          OutlineDeleteGhost (svnw);
       };


} /* TopTreeSetApplDragging */



/* 
** This routine shifts all children of this entry the number of specified pixels
** along the y axis.   This routine is needed when two subtress would otherwise
** overlap.
*/

static void LclTopTreeMoveSubtree(svnw, start_entry_num, level, delta_x)

    svn_widget	svnw;
    int		start_entry_num;    /* first entry to change */
    int		level;		    /* children at level to be changed */ 
    int		delta_x;	    /* amount to move entry */

{
/*
**  Local Variables 
*/
    register int i;			/* loop index */
    register DXmSvnEntryPtr  next_entry;	/* svn entry pointer */


/*
**  Set the parent connection point for the subtree (everything at a lower
**  level than this node).
*/
    for (i = start_entry_num; i <= svnw->svn.num_entries; i++) {
	next_entry = StructGetEntryPtr (svnw, i);
	if (next_entry->level > level) {
	    next_entry->x += delta_x;
	    next_entry->px += delta_x;
	    }
	else
	    break;
	};


} /* LclTopTreeMoveSubtree*/



/* 
** This routine changes the connection point of all children of this entry to
** point to the specifed point.
*/

static void LclTopTreeSetChildConnections(svnw, start_entry_num, end_entry_num, level, px, dpy) 

    svn_widget	svnw;
    int		start_entry_num;    /* first entry to change */
    int		end_entry_num;	    /* last entry to change */
    int		level;		    /* children at level to be changed */ 
    int		px, dpy;		    /* new connection point */

{
/*
**  Local Variables
*/
    register int i;			/* loop index */
    register DXmSvnEntryPtr  next_entry;	/* svn entry pointer */


/*
**  Set the parent connection point for all of our children
*/
    for (i = start_entry_num; i <= end_entry_num; i++) {
	next_entry = StructGetEntryPtr (svnw, i);
	if (next_entry->level == level) {
	    next_entry->px = px;
	    next_entry->py = dpy;
	    };
	};


} /* LclTopTreeSetChildConnections */



/*
**  Layout the entries for the DXmSvnKtopTree style by filling in the X, PX, and PY
**  fields in each entry and the levelx and levely arrays in the widget record.
*/

static void LclTopTreeLayout(svnw, entry_number, cx, cy)

    svn_widget svnw;		
    int	    *entry_number;	/* entry number to layout */
    int	    *cx, *cy;		/* center position of the entry placed */

{
/*
** Local Variables
*/
    int		our_entry_num;	/* our entry number */
    int		delta_y;	/* amount to change y placement of levels */
    int		i;		/* loop counter */
    int		x, y;		/* position of this entry */
    int		level;		/* level of current entry */
    DXmSvnEntryPtr	svnentry;	/* entry to layout */
    DXmSvnEntryPtr	next_entry;	/* next entry to layout */
    int		lx, ly;		/* our leftmost child */
    int		rx, ry;		/* our rightmost child */
    int		width, height;


    /* TBS -- there may be extraneous checks here */
    if (*entry_number == 0) return;
    if (*entry_number > svnw->svn.num_entries) return;
    svnentry = StructGetValidEntryPtr (svnw, *entry_number);
    width = svnentry->width;
    height = svnentry->height;
    level = svnentry->level;
    our_entry_num = *entry_number;
    *entry_number += 1;


/*
**  If we have children lay them out first
*/
    if (*entry_number <= svnw->svn.num_entries) {
	next_entry = StructGetValidEntryPtr (svnw, *entry_number);
	if (next_entry->level > level) {
	    LclTopTreeLayout(svnw, entry_number, &lx, &ly);	    

	    /*
	    ** Now do the rest of the children
	    */
	    rx = lx;
	    ry = ly;
	    while (*entry_number <= svnw->svn.num_entries) {
    		next_entry = StructGetValidEntryPtr (svnw, *entry_number);
		if (next_entry->level == (level + 1)) {
		    LclTopTreeLayout(svnw, entry_number, &rx, &ry);	    
		    }
		else
		    break;
		}


	    /*
	    **  Place ourself centered above our children;
	    */
	    x = ((rx + lx) / 2) - (width/2);


	    /* 
	    **	Don't center over children if we overlap a previous node 
	    */
	    if (x <= (*svnw->svn.levelx)[level]) {
		int dx;	    /* amount that children need to be moved by */
		dx = (*svnw->svn.levelx)[level] + svnw->svn.sibling_spacing - x;
		x = (*svnw->svn.levelx)[level] + svnw->svn.sibling_spacing;
		(*svnw->svn.levelx)[level] = x + width;
		for (i = level+1; i <= svnw->svn.max_level; i++)
		    (*svnw->svn.levelx)[i] += dx;
		LclTopTreeMoveSubtree(svnw, our_entry_num+1, level, dx);
		}
	    else {
		(*svnw->svn.levelx)[level] = x + width;
		};
	    
	    svnentry->x = x;


	    /*
	    ** Our Y position is stored in the levely array so just make sure
	    ** that when we add our height into this level it doesn't overlap
	    ** with the one below us.
	    */
	    y = (*svnw->svn.levely)[level] + height + svnw->svn.level_spacing;
	    if (y > (*svnw->svn.levely)[level+1]) {
		/*
		** We are larger than any other entry on this level, then move
		** all lower levels down.
		*/
		delta_y = y - (*svnw->svn.levely)[level+1];
		for (i = level + 1; i <= svnw->svn.max_level; i++) 
		    (*svnw->svn.levely)[i] += delta_y;
		};


	    /*
	    ** Make all the children connect to this node
	    */
	    LclTopTreeSetChildConnections(svnw, our_entry_num, *entry_number-1,
		level + 1, x + (width/2), height);


	    /*
	    **  Return the center position
	    */
	    *cx = x + width/2;
	    *cy = (*svnw->svn.levely)[level];
	    return;
	    }
	};


/*
** Otherwise, just place ourself.  Our X position is sibling_spacing units to
** the right of the last entry at this level.  Then update the levelx array such
** that it contains our right-most edge.
*/
    x = (*svnw->svn.levelx)[level] + svnw->svn.sibling_spacing;
    (*svnw->svn.levelx)[level] = x + width;
    svnentry->x = x;
    y = (*svnw->svn.levely)[level] + height + svnw->svn.level_spacing;

/*
** Our Y position is stored in the levely array so just make sure that when we
** add our height into this level it doesn't overlap with the one below us.
*/
    if (y > (*svnw->svn.levely)[level+1]) {
	/*
	** We are larger than any other entry on this level, then move
	** all lower levels down to maintain the level_spacing between them.
	*/
	delta_y = y - (*svnw->svn.levely)[level+1];
	for (i = level + 1; i <= svnw->svn.max_level; i++) 
	    (*svnw->svn.levely)[i] += delta_y;
	};


/*
** Make all the children connect to this node
*/
    LclTopTreeSetChildConnections(svnw, our_entry_num, *entry_number-1,
	level + 1, x + (width/2), height);


/*
**  Return the center position
*/
    *cx = x + (width/2);
    *cy = (*svnw->svn.levely)[level];
    return;	    


} /* LclTopTreeLayout */




/*
**  This routine assumes that the tree layout has already been computed and
**  simply draws those portions of the tree that are visible into the svn
**  window.  If the display has been changed, then this routine just returns
**  because the layout must be recalculated.  If there are no clips, then
**  all visible entries are drawn, otherwise, only those intersecting the
**  clips are drawn.  This routine also takes care of making sure the
**  scroll bars are properly updated to reflect the new position of the display.
**  While it is redrawing, it sets the update_in_progress flag to true to
**  eliminate recursive calls to the scroll bar updating routines.
*/

void TopTreeDrawExposed(svnw, wid)

    svn_widget	svnw;
    Window	wid;  /* drawable to receive output */

{
/*
** Local Variables
*/
    register int i, j;	    /* loop index */
    DXmSvnEntryPtr	svnentry;   /* entry to display */
    int x, y;		    /* position of entry in window corrdinates */
    int px, py;		    /* connection point in window corrdinates */
    GC	gc;		    /* graphics context to draw connecting lines */
    int low_x, low_y;	    /* low corner of clip rectangle */
    int high_x, high_y;	    /* high corner of clip rectangle */
    BUFF_SEG_INIT(ConSegs, 500); /* Connecting lines buffer */


/*
**  Return if nothing to display, or the display_needs to be recalculated
*/
    if (svnw->svn.num_entries == 0) return;
    if (svnw->svn.display_changed) return;    

/*
**  Adjust the slider position if necessary, and set the state to updating
**  screen.
*/
    svnw->svn.update_in_progress = TRUE;
    LclTopTreeScrollAdjustSize (svnw);


/*
**  Default the GC for normal drawing
*/
    gc = svnw->svn.foreground_gc;


/*
**  Set-up attributes for connection lines buffer
*/
    BUFF_SEG_SET_ATTRS(ConSegs, svnw, wid, gc);


/*
**  If there are any clips preprocess them
*/
    if (svnw->svn.clip_count != 0)
	DisplayMergeClips(svnw->svn.clips, &(svnw->svn.clip_count));
 

/*
**  If there are no clipping rectangles, then redisplay all of the entries.
*/
    if (svnw->svn.clip_count == 0)
	{
        svnw->svn.refresh_all = FALSE;
	for (i = 1; i <= svnw->svn.num_entries; i++) {
	    svnentry = StructGetValidEntryPtr (svnw, i);
	    y = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));
	    x = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));
	    px = X_TO_WINDOW(svnw, PX_POS(svnw, svnentry));
	    py = Y_TO_WINDOW(svnw, PY_POS(svnw, svnentry));


	    /*
	    ** Do our own clipping to the window for improved performance
	    */
	    if (LclTopTreeEntryVisible(svnw, svnentry)) {
		/*
		** Draw the entry 
		*/
		LclTopTreeDrawEntry(svnw, wid, svnentry, x, y);


		/*
		** Draw the connection line to the parent node
		*/
		LclTopTreeDrawEntryConnection(svnw, svnentry,
			0,0,(int)REALWIDTH(svnw), (int)REALHEIGHT(svnw),&ConSegs);
		}

	    /*
	    ** Otherwise draw only connection line (if it is visible)
	    */
	    else {
		/*
		** Draw the connection line to the parent node if it is visible
		*/
		if (svnw->svn.tree_style == DXmSvnKtopTree)
		    x += (svnentry->width / 2);
		else
		    y += (svnentry->height / 2);

		LclTopTreeDrawConnection(svnw, px, py, x, y, 0, 0, 
			(int)REALWIDTH(svnw), (int)REALHEIGHT(svnw), &ConSegs);
		};
	    };

	/*
	**  Flush connections
	*/
	BUFF_SEG_FLUSH(ConSegs);
	}


/*
**  Loop on each of the clipping rectangles redrawing the intersected entries
*/
    else {
	/*
	**  There are some clipping rectangles, so set them in the GCs
	*/
	XSetClipRectangles (XtDisplay(svnw),svnw->svn.grey_gc,      0,0,svnw->svn.clips,svnw->svn.clip_count,Unsorted);
	XSetClipRectangles (XtDisplay(svnw),svnw->svn.foreground_gc,0,0,svnw->svn.clips,svnw->svn.clip_count,Unsorted);
	XSetClipRectangles (XtDisplay(svnw),svnw->svn.background_gc,0,0,svnw->svn.clips,svnw->svn.clip_count,Unsorted);
	XSetClipRectangles (XtDisplay(svnw),svnw->svn.highlight_gc,0,0,svnw->svn.clips,svnw->svn.clip_count,Unsorted);
	XSetClipRectangles (XtDisplay(svnw),svnw->svn.inverse_highlight_gc,0,0,svnw->svn.clips,svnw->svn.clip_count,Unsorted);
	if (svnw->svn.expect_highlighting)
	    XSetClipRectangles (XtDisplay(svnw),svnw->svn.tree_highlight_gc,0,0,svnw->svn.clips,svnw->svn.clip_count,Unsorted);


       /*
       **  Draw all things intersecting the clipping rectangles
       */
       for (i = 0;  i < svnw->svn.clip_count;  i++)
            {
	    /* 
	    ** Convert clip rectangle into world coordinates
	    */
	    if (LayoutIsRtoL(svnw))
		{
	        low_x  = X_TO_WORLD(svnw, XtWidth(svnw->svn.primary_window_widget)
			    - svnw->svn.clips[i].x) - svnw->svn.clips[i].width;
	        high_x = X_TO_WORLD(svnw, XtWidth(svnw->svn.primary_window_widget)
			    - svnw->svn.clips[i].x);
		}
	    else
		{
	        low_x  = X_TO_WORLD(svnw, svnw->svn.clips[i].x);
	        high_x = X_TO_WORLD(svnw, svnw->svn.clips[i].x + svnw->svn.clips[i].width);
		}
	    low_y  = Y_TO_WORLD(svnw, svnw->svn.clips[i].y);
	    high_y = Y_TO_WORLD(svnw, svnw->svn.clips[i].y + svnw->svn.clips[i].height);

	    /* 
	    ** Redraw all entries that intersect this clip rectangle
	    */
	    for (j = 1; j <= svnw->svn.num_entries; j++) {
		svnentry = StructGetValidEntryPtr (svnw, j);
		x = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));
		y = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));
		px = X_TO_WINDOW(svnw, PX_POS(svnw, svnentry));
		py = Y_TO_WINDOW(svnw, PY_POS(svnw, svnentry));


		/*
		**  Redraw the entry and connection if it intersest the clip rectangle
		*/
		if (LclTopTreeIntersects ((int)X_POS(svnw, svnentry), 
				          (int)Y_POS(svnw, svnentry), 
				          (int)svnentry->width, 
					  (int)svnentry->height, 
					  low_x, low_y, high_x, high_y)) {
		    LclTopTreeDrawEntry (svnw, wid, svnentry, x, y);
		    LclTopTreeDrawEntryConnection(svnw, svnentry,
				    0,0,(int)REALWIDTH(svnw), (int)REALHEIGHT(svnw),&ConSegs);
		    }


		/*
		**  Redraw only the connection if it intersest the clip rectangle
		*/
		else {
		    if (svnw->svn.tree_style == DXmSvnKtopTree)
			x += (svnentry->width / 2);
		    else
			y += (svnentry->height/ 2);

		    LclTopTreeDrawConnection(svnw, px, py, x, y, 
			svnw->svn.clips[i].x, svnw->svn.clips[i].y,
			svnw->svn.clips[i].x + svnw->svn.clips[i].width,
			svnw->svn.clips[i].y + svnw->svn.clips[i].height,
			&ConSegs);
		    };
		};
	    };


	/*
	**  Flush drawing before removing Clips 
	*/
	BUFF_SEG_FLUSH(ConSegs);


	/*
	**  Reset the clip rectangles to none
	*/
	svnw->svn.clip_count = 0;
	XSetClipMask (XtDisplay(svnw), svnw->svn.grey_gc,       None);
	XSetClipMask (XtDisplay(svnw), svnw->svn.foreground_gc, None);
	XSetClipMask (XtDisplay(svnw), svnw->svn.background_gc, None);
	XSetClipMask (XtDisplay(svnw), svnw->svn.highlight_gc, None);
	XSetClipMask (XtDisplay(svnw), svnw->svn.inverse_highlight_gc, None);
	if (svnw->svn.expect_highlighting)
	    XSetClipMask (XtDisplay(svnw), svnw->svn.tree_highlight_gc, None);
	};


/*
**  Nolonger updating display, get rid of clipping rectangles
*/
    svnw->svn.update_in_progress = FALSE;


} /* TopTreeDrawExposed */



/*
**  This routine does processing when the display mode is changed.  It is called
**  on both changes to/from Tree mode so both initial and clean-up processing
**  can be done.
*/
void TopTreeChangeMode (svnw)

    svn_widget	svnw;

{
/*
**  Set-up code...
*/
    if (svnw->svn.display_mode == DXmSvnKdisplayTree) {
	Arg args[1];
	XtSetArg (args[0] ,XmNlabelPixmap, svnw->svn.tree_nav_open_pixmap);
	XtSetValues (svnw->svn.nav_button, args, 1);

        if (!LayoutIsRtoL(svnw))
            LclAdjustNavButton (svnw);
	}

/*
**  Clean-up code...
*/
    else {
	/*
	**  Set the current entry for the Outline code.
	*/
	svnw->svn.current_entry = NULL;
	TopTreeSetCurrentEntry(svnw);
	if (svnw->svn.nav_window_popup != NULL)
	    XtUnmanageChild(svnw->svn.nav_window_popup);
	if (svnw->svn.index_window != NULL) {
	    XtDestroyWidget(svnw->svn.index_window);
	    svnw->svn.index_window = 0;
	    svnw->svn.index_window_shown = FALSE;
	    };
	}


} /* TopTreeChangeMode */



/*
**  This routine does a complete top tree layout from the root.  If there
**  is a current entry it keeps it from moving otherwise it repositions the
**  display to put the root in the center top.
*/

static void LclTopTreeLayoutRoot(svnw)

    svn_widget	svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;   
    int entry_number;	    /* entry number to lay out */
    int i;		    /* loop index */
    int lx, rx;		    /* X pos of leftmost and rightmost children */
    int x, y;		    


/*
** Clear layout variables
*/
    svnw->svn.mapx = 0;
    svnw->svn.mapy = 0;

    for (i = 0; i <= svnw->svn.max_level; i++) {
	(*svnw->svn.levelx)[i] = 0;
	(*svnw->svn.levely)[i] = svnw->svn.level_spacing;
	};

/*
**  Now layout the tree.  
*/
    if (svnw->svn.num_entries > 0) {
	entry_number = 1;    
	LclTopTreeLayout(svnw, &entry_number, &lx, &y);
	rx = lx;	/* in case there is only one node in the tree */
	while (entry_number <= svnw->svn.num_entries)
	    LclTopTreeLayout(svnw, &entry_number, &rx, &y);
	};

/*
**  Determine the max width of the tree
*/
    x = (*svnw->svn.levelx)[0];
    for (i = 1; i <= svnw->svn.max_level; i++) {
	if (x < (*svnw->svn.levelx)[i])
	    x = (*svnw->svn.levelx)[i];
	};
    svnw->svn.tree_width = x + svnw->svn.sibling_spacing;


/*
**  Determine the max height of the tree
*/
    svnw->svn.tree_height = (*svnw->svn.levely)[svnw->svn.max_level];


/*
** Make all first level children connect to the invisible root
*/
    if (svnw->svn.num_entries > 0) {
	svnentry = StructGetEntryPtr (svnw, 1);
	LclTopTreeSetChildConnections(svnw, 1, svnw->svn.num_entries,
	    svnentry->level, (lx + rx)/2, 0);
	};


/*
**  If there is a "current_entry" then maintain it's position, otherwise set
**  the mapping point such that the root is at the center top.
*/
    if (svnw->svn.current_entry != NULL) {
	svnw->svn.mapx = X_POS(svnw, svnw->svn.current_entry) - svnw->svn.prevx;
	svnw->svn.mapy = Y_POS(svnw, svnw->svn.current_entry) - svnw->svn.prevy;
	}
    else {
	svnw->svn.mapx = (lx + rx)/2 - (REALWIDTH(svnw)/2);
	svnw->svn.mapy = 0;
	};


} /* LclTopTreeLayoutRoot */



/*
**  Draw the box around the entry specified.  If the clear-flag is set, then
**  the background is first cleared using the background color.
*/

static void LclTopTreeDrawBox (svnw, window_id, svnentry, gc, basex, basey, clear_flag)

    svn_widget svnw;
    Window window_id;
    DXmSvnEntryPtr svnentry;
    GC gc;
    int basex, basey;
    int clear_flag;

{
/*
**  Local Variables
*/
    int ew = svnentry->width;
    int eh = svnentry->height;


/*
**  Allocate buffers for drawing
*/
    BUFF_SEG_INIT(Segs, 8);
    BUFF_ARC_INIT(Arcs, 10);
    BUFF_ARC_INIT(EraseArcs, 4);


/*
**  Set buffer attributes
*/
    BUFF_SEG_SET_ATTRS(Segs, svnw, window_id, gc);
    BUFF_ARC_SET_ATTRS(Arcs, svnw, window_id, gc, FALSE);
    BUFF_ARC_SET_ATTRS(EraseArcs, svnw, window_id, svnw->svn.background_gc, TRUE);


/*
**  Draw the box around the node
*/
    if (svnw->svn.arc_width == 0) {
	/*
	**  If requested then clear the area first
	*/
	if (clear_flag)
	    XFillRectangle(XtDisplay(svnw), window_id, svnw->svn.background_gc,
		 (Position)basex, (Position)basey, (Dimension)ew+1, (Dimension)eh+1);


	/*
	** Draw or don't draw Shadow as requested
	*/
	if (!svnw->svn.entry_shadows) {
	    /*
	    **  Draw the rectangular bounding box
	    */
	    XDrawRectangle(XtDisplay(svnw), window_id, gc, (Position)basex, (Position)basey, (Dimension)ew, (Dimension)eh);
	    }
	else {
	    int sx, sy, sx2, sy2;	/* shadow line info */


	    /*
	    **	Draw the rectangular bounding box smaller to leave room for the
	    **	shadow.
	    */
	    XDrawRectangle (XtDisplay(svnw), window_id, gc, 
		           (Position)basex, (Position)basey, 
			   (Dimension)ew - 4, (Dimension)eh - 4);


	    /*
	    **  Draw the bottom shadow
	    */
	    sx = basex + 1;
	    sy = basey + eh - 3;
	    sx2 = basex + ew - 1;
	    BUFF_SEG(Segs, sx, sy, sx2, sy);
	    sx++; sy++;
	    BUFF_SEG(Segs, sx, sy, sx2, sy);
	    sx++; sy++;
	    BUFF_SEG(Segs, sx, sy, sx2, sy);


	    /*
	    **  Draw the right-side shadow
	    */
	    sx = basex + ew - 3;
	    sy = basey + 1;
	    sy2 = basey + eh - 1;
	    BUFF_SEG(Segs, sx, sy, sx, sy2);
	    sx++; sy++;
	    BUFF_SEG(Segs, sx, sy, sx, sy2);
	    sx++; sy++;
	    BUFF_SEG(Segs, sx, sy, sx, sy2);
	    }

	}

/*
**  Otherwise, draw box with arcs on the ends
*/
    else {
	int x, y, x2, y2, width, height, start_angle, end_angle;


	/*
	**  Get the information about the top line
	*/
	TOP_LINE_COORDS(svnw, svnentry, basex, basey, eh, ew, x, y, x2, y2);


	/*
	**  If requested clear the center rectangular the area first
	*/
	if (clear_flag)
	    XFillRectangle(XtDisplay(svnw), window_id, svnw->svn.background_gc, 
			   (Position)x, (Position)y, (Dimension)x2 - x, (Dimension)eh);


	/*
	**  Draw the top line
	*/
	BUFF_SEG(Segs, x, y, x2, y2);



	/*
	**  The rest differs if the entry has a shadow
	*/
	if (!svnw->svn.entry_shadows) {
	    /*
	    **  Get the information about and draw the bottom line
	    */
	    BOT_LINE_COORDS(svnw, svnentry, basex, basey, eh, ew, x, y, x2, y2);
	    BUFF_SEG(Segs, x, y, x2, y2);


	    /*
	    **  Get the data describing the left end arc
	    */
	    LEFT_ARC_DATA(svnw, svnentry, basex, basey, ew, eh, x, y, 
		width, height, start_angle, end_angle);


	    /*
	    **  If UserDefined Tree then clear the area first
	    */
	    if (clear_flag) 
		BUFF_ARC(EraseArcs, x, y, width, height, start_angle, end_angle);


	    /*
	    **  Draw the left end arc
	    */
	    BUFF_ARC(Arcs, x, y, width, height, start_angle, end_angle);


	    /*
	    **  Get the data describing the right end arc
	    */
	    RIGHT_ARC_DATA(svnw, svnentry, basex, basey, ew, eh, x, y, 
		width, height, start_angle, end_angle);


	    /*
	    **  If UserDefined Tree then clear the area first
	    */
	    if (clear_flag) 
		BUFF_ARC(EraseArcs, x, y, width, height, start_angle, end_angle);


	    /*
	    **  Draw the right end arc
	    */
	    BUFF_ARC(Arcs, x, y, width, height, start_angle, end_angle);
	    }

	/*
	**  Draw the box with the entry shadow
	*/
	else {
	    /*
	    **  Get the information about and draw the bottom line
	    */
	    BOT_LINE_COORDS(svnw, svnentry, basex, basey, eh, ew, x, y, x2, y2);
	    BUFF_SEG(Segs, x, y-3, x2, y2-3);
	    BUFF_SEG(Segs, x, y-2, x2, y2-2);
	    BUFF_SEG(Segs, x, y-1, x2, y2-1);
	    BUFF_SEG(Segs, x, y,   x2, y2);


	    /*
	    **  Get the data describing the left end arc
	    */
	    LEFT_ARC_DATA(svnw, svnentry, basex, basey, ew, eh, x, y, 
		width, height, start_angle, end_angle);


	    /*
	    **  If UserDefined Tree then clear the area first
	    */
	    if (clear_flag) 
		BUFF_ARC(EraseArcs, x, y, width, height, start_angle, end_angle);


	    /*
	    **  Draw the left end arc (for the shadow only draw the bottom
	    **	quarter of the arc)
	    */
	    BUFF_ARC(Arcs, x, y, width, height-3, start_angle, end_angle);
	    start_angle = -90 * 64;
	    end_angle = -90 * 64;
	    BUFF_ARC(Arcs, x, y, width, height-2, start_angle, end_angle);
	    BUFF_ARC(Arcs, x, y, width, height-1, start_angle, end_angle);
	    BUFF_ARC(Arcs, x, y, width, height, start_angle, end_angle);


	    /*
	    **  Get the data describing the right end arc
	    */
	    RIGHT_ARC_DATA(svnw, svnentry, basex, basey, ew, eh, x, y, 
		width, height, start_angle, end_angle);


	    /*
	    **  If UserDefined Tree then clear the area first
	    */
	    if (clear_flag) 
		BUFF_ARC(EraseArcs, x, y, width, height, start_angle, end_angle);


	    /*
	    **  Draw the right end arc (for the shadow only draw the bottom
	    **  quarter of the arc)
	    */
	    BUFF_ARC(Arcs, x, y, width, height-3, start_angle, end_angle);
	    start_angle = 0;
	    end_angle = -90 * 64;
	    BUFF_ARC(Arcs, x, y + 1, width, height-3, start_angle, end_angle);
	    BUFF_ARC(Arcs, x, y + 2, width, height-3, start_angle, end_angle);
	    BUFF_ARC(Arcs, x, y + 3, width, height-3, start_angle, end_angle);
	    };
	};

/*
**  Flush drawing buffers
*/
    BUFF_ARC_FLUSH(EraseArcs);
    BUFF_SEG_FLUSH(Segs);
    BUFF_ARC_FLUSH(Arcs);


} /* LclTopTreeDrawBox */



/*
**  This routine is called when the user is dragging the slider in either
**  scroll bar.  The valuex and valuey parameters dictate the position
**  in world coordinates of the virtual display under the slider.
*/

static void LclTopTreeScrollDrag (svnw, valuex, valuey)

  svn_widget svnw;
  int valuex, valuey;

{
/*
**  Local data declarations
*/
    Arg arglist[6];


/*
**  If the Navigation window is available, then use it otherwise, bring up
**  a temporary index window.
*/
    if ((svnw->svn.nav_window != NULL) && XtIsManaged(svnw->svn.nav_window_popup)) {
	LclTopTreeUpdateNavBox(svnw, svnw->svn.nav_window, 
	    valuex, valuey, FALSE);
	return;
	}


/*
**  See if we have our index window
*/
    if (svnw->svn.index_window == NULL)
	{
	index_window_CBstruct[0].closure = (XtPointer) svnw;
	XtSetArg(arglist[0], XmNx, REALWIDTH(svnw) - IDXW_WIDTH(svnw) - 11);
	XtSetArg(arglist[1], XmNy, REALHEIGHT(svnw) - IDXW_HEIGHT(svnw) - 11);
	XtSetArg(arglist[2], XmNwidth, IDXW_WIDTH(svnw) + 5);
	XtSetArg(arglist[3], XmNheight, IDXW_HEIGHT(svnw) + 5);
	XtSetArg(arglist[4], XmNborderWidth, 3);
	XtSetArg(arglist[5], XmNexposeCallback, index_window_CBstruct);
	svnw->svn.index_window = XmCreateDrawingArea ((Widget)svnw, "Idx", arglist, 6);
	XtManageChild (svnw->svn.index_window);
	svnw->svn.index_window_shown = TRUE;
	}

     else if (svnw->svn.index_window_shown) {
	/*
	**  Call a support routine to draw the box
	*/
	    LclTopTreeUpdateNavBox(svnw, svnw->svn.index_window, 
		valuex, valuey, FALSE);
	}

     else {
	/*
	**  The index window has been created, but is not visible, so
	**  manage it and remember that it is up.
	*/
        XtSetArg(arglist[0], XmNx, REALWIDTH(svnw) - IDXW_WIDTH(svnw) - 11);
        XtSetArg(arglist[1], XmNy, REALHEIGHT(svnw) - IDXW_HEIGHT(svnw) - 11);
        XtSetArg(arglist[2], XmNwidth, IDXW_WIDTH(svnw) + 5);
        XtSetArg(arglist[3], XmNheight, IDXW_HEIGHT(svnw) + 5);
	XtSetValues(svnw->svn.index_window, arglist, 4);
	XtMapWidget(svnw->svn.index_window);
        svnw->svn.index_window_shown = TRUE;
	}

} /* LclTopTreeScrollDrag */



/*
**  This routine draws a depiction of all entries in the tree that are
**  to be displayed in the index window.
*/
static void LclTopTreeIndexWindowExpose (w, svnw)

    Widget w;		/* The window widget that needs to be drawn */
    svn_widget svnw;	/* The tag is the SVN widget */

{
/*
** Local Variables
*/
    DXmSvnEntryPtr	svnentry;
    int i, k;
    int x, y, px, py, width, height, arc_width;	
    GC gc;
    int num_points;
    DXmSvnPoint points[max_points];
    int turn_length;
    BUFF_RECT_INIT(Rects, 500);
    BUFF_RECT_INIT(FilledRects, 500);
    BUFF_ARC_INIT(Arcs, 500);
    BUFF_ARC_INIT(FilledArcs, 500);
    BUFF_SEG_INIT(Segs, 500);


/*
**  Clear the update nav window flag
*/
    svnw->svn.update_nav_window = FALSE;


/*
**  Clear the window before the redraw.  Do it here and not after checking 
**  the number of entries because we want to refresh (clear) the window for 
**  the case where all of the entries have been deleted so that stale data
**  is not left in it (this is the only thing we want to do).
*/
    XClearArea (XtDisplay(w), XtWindow(w), 0, 0, XtWidth(w), XtHeight(w), FALSE);


/*
**  If there is nothing to display then just return
*/
    if (svnw->svn.num_entries <= 0) return;


/*
**  Default the GC for normal drawing
*/
    gc = svnw->svn.foreground_gc;


/*
**  Initialize the drawing buffers
*/
    BUFF_RECT_SET_ATTRS(Rects, svnw, XtWindow(w), gc, FALSE);
    BUFF_RECT_SET_ATTRS(FilledRects, svnw, XtWindow(w), gc, TRUE);
    BUFF_ARC_SET_ATTRS(Arcs, svnw, XtWindow(w), gc, FALSE);
    BUFF_ARC_SET_ATTRS(FilledArcs, svnw, XtWindow(w), gc, TRUE);
    BUFF_SEG_SET_ATTRS(Segs, svnw, XtWindow(w), gc);


/*
**  Calculate arc_width, if arcs are really small, then change everything
**  to rectangles (arc_width of zero) for improved drawing performance.
*/
    {
    int scale = (svnw->svn.tree_height + svnw->svn.tree_width) / 2;

    if (((svnw->svn.arc_width * XtWidth(w)) / scale) < 3)
	arc_width = 0;
    else 
	arc_width = (svnw->svn.arc_width * XtWidth(w)) / svnw->svn.tree_width;
    }	 


/*
**  Loop through all entries
*/
    for (i = 1; i <= svnw->svn.num_entries; i++) {
	svnentry = StructGetEntryPtr(svnw, i);
	/*
	**  If marked to go into the index window, the draw it.
	*/
	if (svnentry->index_window || svnw->svn.index_all) {
	    x = (X_POS(svnw, svnentry) * XtWidth(w)) / svnw->svn.tree_width;
	    y = (Y_POS(svnw, svnentry) * XtHeight(w)) / svnw->svn.tree_height;
	    width = (svnentry->width * XtWidth(w)) / svnw->svn.tree_width;
	    height = (svnentry->height * XtHeight(w)) / svnw->svn.tree_height;
	    px = (PX_POS(svnw, svnentry) * XtWidth(w)) / (svnw->svn.tree_width);
	    py = (PY_POS(svnw, svnentry) * XtHeight(w)) / svnw->svn.tree_height;


	    /*
	    **  Draw the entry node
	    */
		if (arc_width == 0) {
		    if (svnentry->selected) {
			BUFF_RECT(FilledRects, 
				( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width : x), y,
				width, height);
			}
		    else
			BUFF_RECT(Rects,
				( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width : x), y,
				width, height);
		    }
		else {
		    if (svnentry->selected) {
			BUFF_RECT(FilledRects,
				( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width + arc_width : x + arc_width), y, 
				width - (2 * arc_width), height);
			BUFF_ARC(FilledArcs,
				( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - arc_width * 2 : x), y,
				arc_width * 2, height, ( (LayoutIsRtoL(svnw)) ? 90*64 : -90*64), -180*64);
			BUFF_ARC(FilledArcs,
				( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width : x + width - (2 * arc_width)), y, 
				arc_width * 2, height, ( (LayoutIsRtoL(svnw)) ? -90*64 : 90*64), -180*64);
			}
		    else {
			BUFF_SEG(Segs, ( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - arc_width : x + arc_width), y, 
				( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x + arc_width - width : x + width - arc_width), y);

			BUFF_SEG(Segs, ( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - arc_width : x + arc_width), y + height,
				( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width + arc_width : x + width - arc_width), y + height);

			BUFF_ARC(Arcs, ( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - arc_width * 2 : x), y, 
				arc_width * 2, height, ( (LayoutIsRtoL(svnw)) ? 90*64 : -90*64), -180*64);

			BUFF_ARC(Arcs, ( (LayoutIsRtoL(svnw)) ? XtWidth(w) - x - width : x + width - (2 * arc_width)), 
				y, arc_width * 2, height, ( (LayoutIsRtoL(svnw)) ? -90*64 : 90*64), -180*64);
			}
		    };


	    /*
	    **  Specify where a perpendicular line will turn
	    */
	    if (svnw->svn.tree_style == DXmSvnKtopTree) {
		x += width / 2;
		turn_length = (svnw->svn.level_spacing * XtHeight(w)) /
			(svnw->svn.tree_height * 2);
		}
	    else {
		y += height / 2;
		turn_length = (svnw->svn.level_spacing * XtWidth(w)) /
			(svnw->svn.tree_width * 2);
		}


	    /*
	    **	Draw connecting line to parent
	    */
	    CONNECTION_DATA (svnw, points, num_points, px, py, x, y, turn_length);
	    for (k = 1; k < num_points; k++)
		BUFF_SEG(Segs,
			( (LayoutIsRtoL(svnw)) ? XtWidth(w) - points[k-1].x : points[k-1].x), points[k-1].y, 
			( (LayoutIsRtoL(svnw)) ? XtWidth(w) - points[k].x : points[k].x), points[k].y);
	    };
	};

/*
**  Flush the drawing buffers
*/
    BUFF_RECT_FLUSH(Rects);
    BUFF_RECT_FLUSH(FilledRects);
    BUFF_ARC_FLUSH(Arcs);
    BUFF_ARC_FLUSH(FilledArcs);
    BUFF_SEG_FLUSH(Segs);


/*
**  Call a support routine to update the box in the nav window
*/
    svnw->svn.nav_window_box_width = 0;
    LclTopTreeUpdateNavBox(svnw, w, 
	svnw->svn.mapx, svnw->svn.mapy, FALSE);

} /* LclTopTreeIndexWindowExpose */



/*  
**  This routine is called to place all the components of an entry centered to
**  minimize the entry width.  It then fills in the height and width, and marks
**  the entry as adjusted.
*/

static void LclTopTreeAdjustHeightCentered (svnw, svnentry)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;

{	
/*
**  Local data declarations
*/
    DXmSvnCompPtr comp;
    Dimension below, total_width = 0, total_height = 1;
    int i;


/*
**  Loop through the components and find the largest width.  While doing that
**  also set the y position of each component.
*/
    for (i = 0;  i <= svnentry->num_components - 1;  i++)
	{
        comp = &svnentry->entrycompPtr[i];


	/*
	**  Restore working fields from original values
	*/
	comp->x      = comp->orig_x;
	comp->y      = comp->orig_y;
	comp->width  = comp->orig_width;
	comp->height = comp->orig_height;


	/*
	**  Fetch size information about this component
	*/
	DisplayCompData (svnw, svnentry, comp, &below);
	comp->y = total_height;
	total_height += comp->height + below + 1;
        if (comp->width >= total_width) total_width = comp->width;
	};

/*
**  Loop through the components setting the x position of each component
*/
    for (i = 0;  i <= svnentry->num_components - 1;  i++)
	{  	
        comp = &svnentry->entrycompPtr[i];
	comp->x = (total_width/2) - (comp->width)/2;
	};

/*
**  Set the height and width of the entry
*/
    svnentry->height = total_height;
    svnentry->width  = total_width + 1;
    svnentry->height_adjusted = TRUE;
}

/*
**  This routine performs the action associated with the Nav button.  If the
**  NavWindow is up then it is removed otherwise it is created and managed.
*/

void TopTreeNavButton (svnw)

  svn_widget svnw;

{
/*
**  Local data declarations
*/
    Arg args[15];
    Position x, y;	    /* position of DXmSvn in root coordinates */

/*
**  Local data for the watch cursor
*/
    XColor cursor_fore, cursor_back;
    Font   cursor_font;
    int    cursor_nav;


/*
**  If the NavWindow has been created then just manage/unmanage it.
*/
    if (svnw->svn.nav_window_popup != NULL) {
	if (XtIsManaged(svnw->svn.nav_window_popup)) {
	    XtUnmanageChild(svnw->svn.nav_window_popup);
	    XtSetArg (args[0] ,XmNlabelPixmap, svnw->svn.tree_nav_open_pixmap);
	    XtSetValues (svnw->svn.nav_button, args, 1);
	    }
	else {
	    XtManageChild(svnw->svn.nav_window_popup);
	    XtSetArg (args[0] ,XmNlabelPixmap, svnw->svn.tree_nav_close_pixmap);
	    XtSetValues (svnw->svn.nav_button, args, 1);


	    /*
	    **	Set the width of the box to zero to denote that it has not yet
	    **	been drawn.
	    */
	    svnw->svn.nav_window_box_width = 0;
	    }
	}


/*
**  Otherwise, we must create it
*/
    else {
	/*
	**  Locals for screen width
	*/
	int screen_number = XDefaultScreen(XtDisplay(svnw));
	Position max_x = XDisplayWidth (XtDisplay(svnw),screen_number) - (XtWidth(svnw) / 2);
	Position max_y = XDisplayHeight(XtDisplay(svnw),screen_number) - (XtHeight(svnw) / 2);


        /*
        **  Set up or translations/actions
        */
	XmString cs = svnw->svn.nav_window_title;
	XtTranslations nav_parsed = XtParseTranslationTable (NavWindowTranslations);
        XtAppAddActions ( XtWidgetToApplicationContext ((Widget)svnw),
			  NavWindowActionsList,
			  XtNumber(NavWindowActionsList));


	/*
	**  Create the dialog box to contain nav window 
	*/
	XtTranslateCoords((Widget)svnw, XtWidth(svnw), XtHeight(svnw), &x, &y);
	x += 5; y += 25;
	if (x > max_x) x = max_x;
	if (y > max_y) y = max_y;
	XtSetArg(args[0], XmNx,	    x);
	XtSetArg(args[1], XmNy,	    y);
	XtSetArg(args[2], XmNdialogTitle,  cs);
	XtSetArg(args[3], XmNmarginWidth,  0);
	XtSetArg(args[4], XmNmarginHeight, 0);
	XtSetArg(args[5], XmNwidth,	   XtWidth(svnw)/2);
	XtSetArg(args[6], XmNheight,	   XtHeight(svnw)/2);
	XtSetArg(args[7], XmNnoResize,	   FALSE);
	XtSetArg(args[8], XmNdialogStyle,  XmDIALOG_MODELESS);
#if 0
	XtSetArg(args[9], DwtNacceptFocus, FALSE);  /* Mapping unknown */
#endif
	svnw->svn.nav_window_popup = XmCreateFormDialog((Widget) svnw,"Nav", args, 9);

	
	/*
	**  Create the window widget
	*/
	index_window_CBstruct[0].closure = (XtPointer) svnw;
	XtSetArg(args[0], XmNx, 0);
	XtSetArg(args[1], XmNy, 0);
	XtSetArg(args[2], XmNwidth,    XtWidth(svnw)/2);
	XtSetArg(args[3], XmNheight,	XtHeight(svnw)/2);
	XtSetArg(args[4], XmNexposeCallback, index_window_CBstruct);
	XtSetArg(args[5], XmNtopOffset,	0);
	XtSetArg(args[6], XmNbottomOffset,	0);
	XtSetArg(args[7], XmNleftOffset,	0);
	XtSetArg(args[8], XmNrightOffset,	0);
	XtSetArg(args[9], XmNtopAttachment, XmATTACH_FORM);
	XtSetArg(args[10],XmNbottomAttachment, XmATTACH_FORM);
	XtSetArg(args[11],XmNleftAttachment, XmATTACH_FORM);
	XtSetArg(args[12],XmNrightAttachment, XmATTACH_FORM);
	XtSetArg(args[13],XmNtranslations ,nav_parsed);
	svnw->svn.nav_window = XmCreateDrawingArea ((Widget)svnw->svn.nav_window_popup, 
	    "Nav", args, 14);


	/*
	**  Set the Icon name in case the toplevel shell isn't realized (as
	**  in VAX Notes).  In this case we have an icon in the icon box.
	*/
	XtSetArg(args[0], XtNiconName, "Navigation");
	XtSetValues (XtParent(svnw->svn.nav_window_popup), args, 1);


	/*
	**  Define a cursor to make the window look active, if it hasn't been
	**  done before
	*/
	if (svnw->svn.nav_box_cursor == 0) 
	    {
	    cursor_fore.pixel = 0;
	    cursor_fore.red   = 0;
	    cursor_fore.green = 0;
	    cursor_fore.blue  = 0;

	    cursor_back.pixel = 0;
	    cursor_back.red   = 65535;
	    cursor_back.green = 65535;
	    cursor_back.blue  = 65535;

	    svnw->svn.nav_box_cursor = XCreateFontCursor(XtDisplay(svnw), XC_crosshair);
	    }


	/*
	** Manage the Nav window, and change the nav button icon
	*/
	XtManageChild (svnw->svn.nav_window);
	XtManageChild (svnw->svn.nav_window_popup);
	XtSetArg (args[0] ,XmNlabelPixmap, svnw->svn.tree_nav_close_pixmap);
	XtSetValues (svnw->svn.nav_button, args, 1);


	/*
	**  Now that the window is visible, change the cursor.
	*/
        XDefineCursor (XtDisplay(svnw), XtWindow(svnw->svn.nav_window),
		       svnw->svn.nav_box_cursor);


	/*
	**  Set the coordinates of the current location box
	**  to (-1) to denote that it has not yet been drawn.
	*/
	svnw->svn.nav_window_box_width = 0;
	}

    if (!LayoutIsRtoL(svnw))
        LclAdjustNavButton (svnw);

} /* TopTreeNavButton */



/*
**  This routine updates the box showing the current display position in 
**  Nav Window.
*/

static void LclTopTreeUpdateNavBox (svnw, w, x, y, clear)

  svn_widget svnw;
  Widget w;	    /* window widget to update */
  int x, y;	    /* new position in world coordinates of box */
  int clear;	    /* Clear old box, but don't draw new */

{
/*
** Local Variables
*/
    GC gc;	    /* gc to draw box with */


/*
**  If the Output Window doesn't exist, isn't visible, or it's parent isn't
**  visible, then don't do anything.
*/
    if (w == NULL) return;
    if (!XtIsManaged(w)) return;


/*
**  If the SVN widget has been unmapped, then pretend that the user hit the
**  button to hide this box and leave.
*/
    if (!XtIsManaged(svnw))
       {
         TopTreeNavButton (svnw);
         return;
       };


/*
**  If the box hasn't moved, but has already been drawn (width != 0) then just
**  return
*/
    if ((x == svnw->svn.nav_window_box_x) &&
 	(y == svnw->svn.nav_window_box_y) &&
 	(svnw->svn.nav_window_box_width != 0)) return;


/*
**  If there are no entries in the tree, just return
*/
    if (svnw->svn.num_entries <= 0) return;


/*
**  Use the inverse gc (created during initialization)
*/
    gc = svnw->svn.nav_box_gc;

    
/*
**  Erase the old box if there was one
*/
    if (svnw->svn.nav_window_box_width != 0)
	if (LayoutIsRtoL(svnw))
	    XDrawRectangle(XtDisplay(w), XtWindow(w), gc,
			(Position)(XtWidth(w) - svnw->svn.nav_window_box_x - svnw->svn.nav_window_box_width),
			(Position)svnw->svn.nav_window_box_y,
			(Dimension)svnw->svn.nav_window_box_width,
			(Dimension)svnw->svn.nav_window_box_height);
	else
	    XDrawRectangle(XtDisplay(w), XtWindow(w), gc, 
			(Position)svnw->svn.nav_window_box_x,
			(Position)svnw->svn.nav_window_box_y,
			(Dimension)svnw->svn.nav_window_box_width,
			(Dimension)svnw->svn.nav_window_box_height);


/*
**  If we are only suppose to clear the box, then set the oldw to 0 (to
**  remember that no box is displayed and return without drawing the new box).
*/
    if (clear) {
	svnw->svn.nav_window_box_width = 0;
	return;
	}


/*
**  Draw the box around our current location in the tree, and
**  save those coordinates.
*/
    svnw->svn.nav_window_box_width =  (REALWIDTH(svnw) * (int)XtWidth(w)) / 
					svnw->svn.tree_width;
    if (svnw->svn.nav_window_box_width > (int)XtWidth(w)) 
	svnw->svn.nav_window_box_width = (int)XtWidth(w) - 2;

    svnw->svn.nav_window_box_height = ((REALHEIGHT(svnw) * (int)XtHeight(w)) / svnw->svn.tree_height);
    if (svnw->svn.nav_window_box_height > (int)XtHeight(w)) 
	svnw->svn.nav_window_box_height = (int)XtHeight(w) - 2;

    svnw->svn.nav_window_box_x = (x * (int)XtWidth(w)) / svnw->svn.tree_width;
    svnw->svn.nav_window_box_y = (y * (int)XtHeight(w)) / svnw->svn.tree_height;


/*
**  Handle the special case of the tree being smaller than window, but
**  the tree is not entirely visible.
*/
    if (x < 0) 
	if ((svnw->svn.tree_width - x) > REALWIDTH(svnw))
	    svnw->svn.nav_window_box_x = -(((svnw->svn.tree_width - 
		(REALWIDTH(svnw)+x)) * (int)XtWidth(w)) / svnw->svn.tree_width);
	else
	    svnw->svn.nav_window_box_x = 0;
	

    if (y < 0) 	
	if ((svnw->svn.tree_height - y) > REALHEIGHT(svnw))
	    svnw->svn.nav_window_box_y = -(((svnw->svn.tree_height -
		(REALHEIGHT(svnw)+y)) * (int)XtHeight(w)) / svnw->svn.tree_height);
	else
	    svnw->svn.nav_window_box_y = 0;
	    

/*
**  Draw the new box
*/
    if (LayoutIsRtoL(svnw))
	XDrawRectangle(XtDisplay(w), XtWindow(w), gc,
		    (Position)(XtWidth(w) - svnw->svn.nav_window_box_x - svnw->svn.nav_window_box_width),
		    (Position)svnw->svn.nav_window_box_y,
		    (Dimension)svnw->svn.nav_window_box_width,
		    (Dimension)svnw->svn.nav_window_box_height);
    else
        XDrawRectangle(XtDisplay(w), XtWindow(w), gc, 
		    (Position)svnw->svn.nav_window_box_x,
		    (Position)svnw->svn.nav_window_box_y,
		    (Dimension)svnw->svn.nav_window_box_width,
		    (Dimension)svnw->svn.nav_window_box_height);


} /* TopTreeUpdateNavBox */



/*
**  This routine processes a button down in the Nav window.  It manages the
**  dragging of the nav box around within the window.  Control remains here
**  until the button is released.  When the button is release the display
**  is scrolled to the new position except drag is cancelled.
*/

void LclTopTreeNavWindowButtonDown (w, event)

    Widget       w;
    XButtonEvent *event;

{
/*
**  Local data declarations
*/
    svn_widget svnw = StructFindSvnWidget (w);
    Window root_return, child_return;
    unsigned int state_mask_return;
    int root_x_return, root_y_return, win_x_return, win_y_return;
    int start_x, start_y;   /* original mouse position */
    int x, y;		    /* map position of the drag box */
    int last_x, last_y;	    /* last map position of the drag box */
    int status;		    
    int drag_box_width;
    int drag_box_height;	   


/*
**  Lint complained about used before set on these variables...
*/
    last_x = -1;
    last_y = -1;


/*
**  Calculate drag_box_width in world coordinates
*/
    drag_box_width = (svnw->svn.nav_window_box_width * svnw->svn.tree_width) /
		    (int) XtWidth(w);
    drag_box_height = (svnw->svn.nav_window_box_height * svnw->svn.tree_height)/
		    (int) XtHeight(w);


/*
**  Record the start location of the pointer to be the drag holding point
*/
    start_x = event->x;
    start_y = event->y;


/*
**  Do a while forever loop until the user is completed or cancelled.
*/
    while (TRUE)

      {
        /*
        **  Find out the current state of the pointer and mouse buttons.
        */
            status = XQueryPointer (XtDisplay(svnw->svn.nav_window), 
		XtWindow(svnw->svn.nav_window), &root_return, &child_return,
		&root_x_return, &root_y_return, &win_x_return, &win_y_return,
		&state_mask_return);


        /*
	**  If the return status is bad or one of the other buttons is pressed,
	**  then cancel (set end position to start position).
        */
            if ((status == 0) || (state_mask_return & Button2Mask) || (state_mask_return & Button3Mask))
               {
		/*
		**  Call a support routine to update the box in the nav window
		**  box back to its original position and return
		*/
		LclTopTreeUpdateNavBox(svnw, svnw->svn.nav_window, 
		    svnw->svn.mapx, svnw->svn.mapy, FALSE);
	        return;
               };


        /*
	**  Determine the new box position and force it to stay completely
	**  visible on the screen.
        */
	    if (LayoutIsRtoL(svnw))
		x = svnw->svn.mapx + (((start_x - win_x_return) * svnw->svn.tree_width)
		    / (int)XtWidth(w));
	    else
	        x = svnw->svn.mapx + (((win_x_return - start_x) * svnw->svn.tree_width) 
		    / (int)XtWidth(w));
	    y =  svnw->svn.mapy + (((win_y_return - start_y) * 
			    svnw->svn.tree_height) / (int)XtHeight(w));
	    if (x < 0) x = 0;
	    if (y < 0) y = 0; 
	    if ((y + drag_box_height) > svnw->svn.tree_height) {
		y = svnw->svn.tree_height - drag_box_height;
		};
	    if ((x + drag_box_width) > svnw->svn.tree_width) {
		x = svnw->svn.tree_width - drag_box_width;
		};

        /*
        **  Update the drag box position if it has changed
        */
	    if ((x != last_x) || (y != last_y))
		LclTopTreeUpdateNavBox(svnw, svnw->svn.nav_window, x, y, FALSE);


        /*
        **  Store the current position so we can determine motion
        */
	    last_x = x;	
	    last_y = y;


        /*
        **  If the button has been released, then exit the loop
        */
            if ((state_mask_return & Button1Mask) == FALSE)
               { 
                break;
               };
      };  /* while */


/*
**  If the location has changed, update the display window.
*/
    if ((x != svnw->svn.mapx) || (y != svnw->svn.mapy)) {
	LclTopTreePerformScroll(svnw, x - svnw->svn.mapx, y - svnw->svn.mapy);
	};


} /* LclTopTreeNavWindowButtonDown */



/*
**  This routine processes the help button down in the Nav window.
*/

void LclTopTreeNavWindowHelp (w, event)

    Widget       w;
    XButtonEvent *event;

{
/*
**  Local data declarations
*/
    svn_widget svnw = StructFindSvnWidget (w);
    DXmSvnCallbackStruct temp;


/*
**  Set up the structure...  Set the component number field to the widget id of
**  the scroll bar making the call.  This is for VMSmail.
*/
    temp.reason = DXmSvnCRHelpRequested;
    temp.entry_number = DXmSvnKhelpNavWindow;
    temp.time = event->time;
    temp.component_number = 0;


/*
**  Make the callback
*/
    XtCallCallbacks ((Widget)svnw, DXmSvnNhelpRequestedCallback, &temp);


} /* LclTopTreeNavWindowHelp */




/*
**  This routine does a complete Left tree layout from the root.  If there
**  is a current entry it keeps it from moving otherwise it repositions the
**  display to put the root in the center top.
*/

static void LclSideTreeLayoutRoot(svnw)

    svn_widget	svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;   
    int entry_number;	    /* entry number to lay out */
    int i;		    /* loop index */
    int lx, rx;		    /* X pos of leftmost and rightmost children */
    int x, y;		    


/*
** Clear layout variables
*/
    svnw->svn.mapx = 0;
    svnw->svn.mapy = 0;

    for (i = 0; i <= svnw->svn.max_level; i++) {
	(*svnw->svn.levelx)[i] = 0;
	(*svnw->svn.levely)[i] = svnw->svn.level_spacing;
	};

/*
**  Now layout the tree.  
*/
    if (svnw->svn.num_entries > 0) {
	entry_number = 1;    
	LclSideTreeLayout(svnw, &entry_number, &lx, &y);
	rx = lx;	/* in case there is only one node in the tree */
	while (entry_number <= svnw->svn.num_entries)
	    LclSideTreeLayout(svnw, &entry_number, &rx, &y);
	};

/*
**  Determine the max width of the tree
*/
    x = (*svnw->svn.levelx)[0];
    for (i = 1; i <= svnw->svn.max_level; i++) {
	if (x < (*svnw->svn.levelx)[i])
	    x = (*svnw->svn.levelx)[i];
	};
    svnw->svn.tree_height = x + svnw->svn.sibling_spacing;


/*
**  Determine the max height of the tree
*/
    svnw->svn.tree_width = (*svnw->svn.levely)[svnw->svn.max_level];


/*
** Make all first level children connect to the invisible root
*/
    if (svnw->svn.num_entries > 0) {
	svnentry = StructGetEntryPtr (svnw, 1);
	LclTopTreeSetChildConnections(svnw, 1, svnw->svn.num_entries,
	    svnentry->level, (lx + rx)/2, 0);
	};


/*
**  If there is a "current_entry" then maintain it's position, otherwise set
**  the mapping point such that the root is at the center top.
*/
    if (svnw->svn.current_entry != NULL) {
	svnw->svn.mapx = X_POS(svnw, svnw->svn.current_entry) - svnw->svn.prevx;
	svnw->svn.mapy = Y_POS(svnw, svnw->svn.current_entry) - svnw->svn.prevy;
	}
    else {
	svnw->svn.mapy = (lx + rx)/2 - (REALHEIGHT(svnw)/2);
	svnw->svn.mapx = 0;
	};


} /* LclSideTreeLayoutRoot */


/*
**  Layout the entries by filling in the X, PX, and PY fields in each entry
**  and the levelx and levely arrays in the widget record.
*/

static void LclSideTreeLayout(svnw, entry_number, cx, cy)

    svn_widget svnw;		
    int	    *entry_number;	/* entry number to layout */
    int	    *cx, *cy;		/* center position of the entry placed */

{
/*
** Local Variables
*/
    int		our_entry_num;	/* our entry number */
    int		delta_y;	/* amount to change y placement of levels */
    int		i;		/* loop counter */
    int		x, y;		/* position of this entry */
    int		level;		/* level of current entry */
    DXmSvnEntryPtr	svnentry;	/* entry to layout */
    DXmSvnEntryPtr	next_entry;	/* next entry to layout */
    int		lx, ly;		/* our leftmost child */
    int		rx, ry;		/* our rightmost child */
    int		width, height;


    /* TBS -- there may be extraneous checks here */
    if (*entry_number == 0) return;
    if (*entry_number > svnw->svn.num_entries) return;
    svnentry = StructGetValidEntryPtr (svnw, *entry_number);
    width = svnentry->width;
    height = svnentry->height;
    level = svnentry->level;
    our_entry_num = *entry_number;
    *entry_number += 1;


/*
**  If we have children lay them out first
*/
    if (*entry_number <= svnw->svn.num_entries) {
	next_entry = StructGetValidEntryPtr (svnw, *entry_number);
	if (next_entry->level > level) {
	    LclSideTreeLayout(svnw, entry_number, &lx, &ly);	    

	    /*
	    ** Now do the rest of the children
	    */
	    rx = lx;
	    ry = ly;
	    while (*entry_number <= svnw->svn.num_entries) {
    		next_entry = StructGetValidEntryPtr (svnw, *entry_number);
		if (next_entry->level == (level + 1)) {
		    LclSideTreeLayout(svnw, entry_number, &rx, &ry);	    
		    }
		else
		    break;
		}


	    /*
	    **  Place ourself centered above our children;
	    */
	    x = ((rx + lx) / 2) - (height/2);


	    /* 
	    **	Don't center over children if we overlap a previous node 
	    */
	    if (x <= (*svnw->svn.levelx)[level]) {
		int dx;	    /* amount that children need to be moved by */
		dx = (*svnw->svn.levelx)[level] + svnw->svn.sibling_spacing - x;
		x = (*svnw->svn.levelx)[level] + svnw->svn.sibling_spacing;
		(*svnw->svn.levelx)[level] = x + height;
		for (i = level+1; i <= svnw->svn.max_level; i++)
		    (*svnw->svn.levelx)[i] += dx;
		LclTopTreeMoveSubtree(svnw, our_entry_num+1, level, dx);
		}
	    else {
		(*svnw->svn.levelx)[level] = x + height;
		};


	    svnentry->x = x;
	    y = (*svnw->svn.levely)[level] + width + svnw->svn.level_spacing;
	    if (y > (*svnw->svn.levely)[level+1]) {
		/*
		** We are larger than any other entry on this level, then move
		** all lower levels down.
		*/
		delta_y = y - (*svnw->svn.levely)[level+1];
		for (i = level + 1; i <= svnw->svn.max_level; i++) 
		    (*svnw->svn.levely)[i] += delta_y;
		};


	    /*
	    ** Make all the children connect to this node
	    */
	    LclTopTreeSetChildConnections(svnw, our_entry_num, *entry_number-1,
		level + 1, (x + (height/2)), width);


	    /*
	    **  Return the center position
	    */
	    *cx = x + (height)/2;
	    *cy = (*svnw->svn.levely)[level];
	    return;
	    }
	};


/*
** Otherwise, just place ourself
*/
    x = (*svnw->svn.levelx)[level] + svnw->svn.sibling_spacing;
    (*svnw->svn.levelx)[level] = x + height;
    svnentry->x = x;
    y = (*svnw->svn.levely)[level] + width + svnw->svn.level_spacing;
    if (y > (*svnw->svn.levely)[level+1]) {
	/*
	** We are larger than any other entry on this level, then move
	** all lower levels down.
	*/
	delta_y = y - (*svnw->svn.levely)[level+1];
	for (i = level + 1; i <= svnw->svn.max_level; i++) 
	    (*svnw->svn.levely)[i] += delta_y;
	};


/*
**  Return the center position
*/
    *cx = x + (height)/2;
    *cy = (*svnw->svn.levely)[level];
    return;	    


} /* LclSideTreeLayout */




/*
**  This routine does a complete UserDefined tree layout from the root.  If
**  there is a current entry it keeps it from moving otherwise it leaves the
**  the display is left at the previous position.  The only real work in this
**  mode is determining the tree height and width such that the scroll bars
**  are correct.  An additional width and height are always added so that the
**  display is always somewhat larger then the nodes this always makes whitespace
**  available to drag nodes.
*/

static void LclUserDefinedTreeLayoutRoot(svnw)

    svn_widget	svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;   
    int i;		    /* loop index */
    int max_y = 0, max_x = 0;
    int x, y;


/*
** Determine the height and width of the tree
*/
    for (i = 1; i <= svnw->svn.num_entries; i++) {
	svnentry = StructGetValidEntryPtr(svnw, i);
	x = X_POS(svnw, svnentry) + svnentry->width;
	y = Y_POS(svnw, svnentry) + svnentry->height;
	if (x > max_x) max_x = x;
	if (y > max_y) max_y = y;
	}


/*
**  Set the tree width and height;
*/
    svnw->svn.tree_width = max_x + (XtWidth(svnw) / 2);
    svnw->svn.tree_height = max_y + (XtHeight(svnw) / 2);


/*
**  If there is a "current_entry" then maintain it's position, otherwise 
**  leave the tree where ever it is.
*/
    if (svnw->svn.current_entry != NULL) {
	svnw->svn.mapx = X_POS(svnw, svnw->svn.current_entry) - svnw->svn.prevx;
	svnw->svn.mapy = Y_POS(svnw, svnw->svn.current_entry) - svnw->svn.prevy;
	}

} /* LclUserDefinedTreeLayoutRoot */



/*
**  This routine does a complete Outline tree layout from the root.  If there
**  is a current entry it keeps it from moving otherwise it repositions the
**  display to put the root in the center top.
*/

static void LclOutlineTreeLayoutRoot(svnw)

    svn_widget	svnw;

{
/*
**  Local data declarations
*/
    DXmSvnEntryPtr svnentry;   
    int i, cur_y, max_x;


/*
** Determine the height of the tree
*/
    cur_y = svnw->svn.sibling_spacing;
    for (i = 1; i <= svnw->svn.num_entries; i++) {
	svnentry = StructGetValidEntryPtr(svnw, i);
	cur_y += svnentry->height + svnw->svn.sibling_spacing;
	}


/*
**  Set the tree height;
*/
    svnw->svn.tree_height = cur_y;


/*
**  Set the position of each level
*/
    (*svnw->svn.levely)[0] = svnw->svn.level_spacing;
    for (i = 1; i <= svnw->svn.max_level; i++) {
	(*svnw->svn.levely)[i] = (*svnw->svn.levely)[i-1] + svnw->svn.level_spacing;
	(*svnw->svn.levelx)[i] = 0;
	}


/*
**  Set the position of each entry
*/
    cur_y = svnw->svn.sibling_spacing;
    max_x = 0;
    for (i = 1; i <= svnw->svn.num_entries; i++) {
	svnentry = StructGetValidEntryPtr(svnw, i);
	svnentry->x = cur_y;

	/*
	**  Set the connection point to the parent
	*/
	if (svnentry->level !=0)
	    svnentry->px = (*svnw->svn.levelx)[svnentry->level - 1];
	else
	    svnentry->px = 0;
	svnentry->py = svnw->svn.arc_width + (svnw->svn.level_spacing/2);

	/*
	**  Set the connection point for this level to this entry.  It will
	**  be superseded, by the next entry on this same level.
	*/
	(*svnw->svn.levelx)[svnentry->level] = cur_y + svnentry->height;
	cur_y += svnentry->height + svnw->svn.sibling_spacing;

	
	/*
	**  Update the max X value found if larger that previous
	*/
	if (max_x < ((*svnw->svn.levely)[svnentry->level] + svnentry->width))
	    max_x = (*svnw->svn.levely)[svnentry->level] + svnentry->width;
	}
    

/*
**  Set the tree height;
*/
    svnw->svn.tree_width = max_x;



/*
**  If there is a "current_entry" then maintain it's position, otherwise set
**  the mapping point such that the root is at the left & top.
*/
    if (svnw->svn.current_entry != NULL) {
	svnw->svn.mapx = X_POS(svnw, svnw->svn.current_entry) - svnw->svn.prevx;
	svnw->svn.mapy = Y_POS(svnw, svnw->svn.current_entry) - svnw->svn.prevy;
	}
    else {
	svnw->svn.mapy = 0;
	svnw->svn.mapx = 0;
	};


} /* LclOutlineTreeLayoutRoot */


#ifdef SVN_DEBUG
static SVNSYNC(svnw, on)

    svn_widget svnw;
    int on;

{
/*
**  Make calls synchronized to aid debugging
*/
    XSynchronize(XtDisplay(svnw), on);
}
#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element SVN_DISPLAY_TOPTREE.C */
/*  *8    21-NOV-1989 13:13:11 RYAN "Portability" */
/*  *7     2-NOV-1989 15:47:30 RYAN "BL11.3+" */
/*  *6     4-OCT-1989 10:12:13 RYAN "BL11.2" */
/*  *5    28-SEP-1989 09:10:21 RYAN "Force rebuild" */
/*  *4    27-SEP-1989 09:34:43 RYAN "Back to BL11" */
/*  *3    26-SEP-1989 12:55:45 RYAN "BL11.1" */
/*  *2    23-AUG-1989 11:23:50 RYAN "BL11" */
/*  *1    22-AUG-1989 16:55:59 RYAN "Initial elements for V3" */
/*  DEC/CMS REPLACEMENT HISTORY, Element SVN_DISPLAY_TOPTREE.C */
