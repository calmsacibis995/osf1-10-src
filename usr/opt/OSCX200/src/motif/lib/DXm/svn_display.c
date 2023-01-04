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

/*=======================================================================
*
*                   COPYRIGHT (c) 1988, 1989, 1992 BY
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
/*	041	AN			 31-Mar-1993			      */
/*		Add call to LclOutlineHideWidgets in Resize so that when      */
/*		resize happens any components that are widgets are unmanaged  */
/*		and remanaged in there new places during the redraw process.  */
/*	040	AN			 03-Nov-1992			      */
/*		Change the pixel placement of the location cursor for around  */
/*		components in column mode.				      */
/*	039	CS			 19-Mar-1992			      */
/*		Fix bug in LclDisplayResizeSubwidget.  A widget was being set */
/*		to a zero width/height.  This caused a problem when calling   */
/*		XConfigureWindow.  Remove the accelerators from the form      */
/*		widgets because they were grabbing the return key events.     */
/*	038	CS			 16-Mar-1992			      */
/*		Added an event handler for StructureNotify events on the      */
/*		primary scrolled window so the svn.primary_percentage	      */
/*		resource can be updated when the pane mullion is changed by   */
/*		the user.						      */
/*	037	CS			  2-Jan-1992			      */
/*		Changed arguments passed to DisplayDrawEntry.  Added function */
/*		DisplayInsertOrder.					      */
/*	036	AN			09-Dec-1991			      */
/*		Add fix in DisplayDrawColumns where location cursor was being */
/*		drawn 2 pixels off in some cases.			      */
/*	035	AN			21-Nov-1991			      */
/*		Add fixes in OutlineChangeMode to handle new PTR windows.     */
/*	034	AN			08-Nov-1991			      */
/*		Casting for RISC compile problems...			      */
/*	033	AN			25-Oct-1991			      */
/*		Redesigned PTR window, so that it is now a separate window    */
/*		This included changes to ALL drawing code, live-scrolling     */
/*		code and traversal code.  Also reworked some drawing routines */
/*		for efficieny and performance.				      */
/*	032	AN			13-May-1991			      */
/*		Fixed bug in DisplayEraseLocationCursor, so that it uses the  */
/*		correct base_x for erasing the location cursor in column mode.*/
/*	031	AN		       10-Apr-1991			      */
/*		Change references to svn_widget in prototype def's to Widget. */
/*	030	P. Rollman		1-Apr-1991			      */
/*		Add ANSI function definitions				      */
/*      029     M. Poyarkov             Mar-20-1991                           */
/*              To prevent resizing of nav button I change the size of        */
/*              bitmaps and remove margins. Now bitmap fits exactly into      */
/*              nav push button and we do not need LclAdjustNavButton.        */
/*      028	AN			25-Feb-1991			      */
/*		Change XCopyArea gc to copyarea_gc to avoid NoExpose's	      */
/*	027	AN			25-Jan-1991			      */
/*		Add routine DisplayOutlineEraseLocationCursor.		      */
/*	026	AN			11-Dec-1990			      */
/*		Change look of location cursor so it is now a solid line with */
/*		the color of highlight_color... and the highlight feature of  */
/*		SVN is now a dashed line.				      */
/*	025	AN			28-Nov-1990			      */
/*		Set traversalOn resource to FALSE for all scrollbars and      */
/*		pushbuttons in SVN... keyboard traversal will handle this     */
/*		functionality for them.					      */
/*	024	AN			23-Oct-1990			      */
/*		Put fix into DisplayDrawColumns routine so that if no	      */
/*		components in an entry on RHS but other entries have	      */
/*		components... then the columns lines would get redisplayed.   */
/*	023	AN			09-Oct-1990			      */
/*		Change XtAddActions to XtAppAddActions.			      */
/*	022	AN			27-Sep-1990			      */
/*		Add support for loc. cursor in tree mode.		      */
/*	021	AN			20-Sep-1990			      */
/*		Added support for location cursor for new Motif style	      */
/*		guide mouse semantics.					      */
/*      020     S. Lavigne              24-Aug-1990                           */
/*              Integrate the DEC Israel changes into this module - their     */
/*              code is turned on by the macro LayoutIsRtoL.                  */
/*	019	AN			23-Aug-1990			      */
/*		More changes to make manager class and keyboard traversal work.*/
/*	018	AN			11-Jul-1990			      */
/*		Change names of variables and anything external to be	      */
/*		internationalized for sides or pane windows.  "rhs"->secondary*/
/*		and "lhs" -> primary.					      */
/*	017	AN			28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	016	AN			25-Jun-1990			      */
/*		Changes made so that only compound strings are supported      */
/*		instead of text strings and compound strings.		      */
/*	015	AN			22-May-1990			      */
/*		Fixed bug in DisplayDrawColumnsBox where if the last comp.    */
/*		field of an entry was null.. and one selected the one before  */
/*		the last component field, the last component field box would  */
/*		still be highlighted.					      */
/*	014	AN			08-May-1990			      */
/*		Fix bug in DisplayResize that causes resizing problems with   */
/*		the pane widget if SVN contained more than 2 panes.	      */
/*	013	AN			24-Apr-1990			      */
/*		Take out code that setup the SVN translation table up in      */
/*		the window widgets themselves.. instead, make sure the evnets */
/*		get propagated correctly up to the top SVN widget as in XUI.  */
/*	012	AN			09-Apr-1990			      */
/*		Take out code that supports only text components.. , now      */
/*		Motif version will only have compound strings...	      */
/*	011	AN			26-Mar-1990			      */
/*		Fix bug where if resource SvnNlhsPercentage was set to 100%   */
/*		X errors would occur.					      */
/*	010	S. Lavigne 		01-Feb-1990			      */
/*		To compile cleanly with /STANDARD=PORT, add #pragma standard  */
/*		lines around #include files and externalrefs.		      */
/*      009	Will Walker		31-Jan-1990			      */
/*		Remove include of  PaneP.h				      */
/*      008	Will Walker		26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	007	Will Walker		25-Jan-1990			      */
/*		Change DECwWsSvn*.h to DECwMSvn*.h.			      */
/*	006	S. Lavigne 		24-Jan-1990			      */
/*		Change SvnComp* constants to SvnKcomp* since the naming	      */
/*		convention changed.					      */
/*	005	Will Walker		23-Jan-1990			      */
/*		Convert Svn* constants to SvnK*.			      */
/*	004	Will Walker		22-Jan-1990			      */
/*		Change highlight pixmap to arm pixmap for nav wind button.    */
/*		Change BcseSvnxxxx to Svnxxx.				      */
/*	003	Will Walker		19-Jan-1990			      */
/*		Temporarily remove DWTPBHELP from the translation tables for  */
/*		the totop and tobottom push buttons on the vertical	      */
/*		scrollbar.  Augment the translation table for the	      */
/*		lhs_window_widget and rhs_window_widget using the parsed      */
/*		translation table created in ClassInitialize.  Since Motif    */
/*		does not allow 0 increment for scroll bars, temporarily set   */
/*		these to 1.  Since the Scrolling Policy for a ScrolledWindow  */
/*		can only be set during creation time, move the		      */
/*		ScrollingPolicy from a SetValues call to the creation call.   */
/*									      */
/*		NOTE:  Something is different with the translation tables     */
/*		       (action lists?)!  The modifications described above    */
/*		       were a work around for this problem.		      */
/*									      */
/*	002	Will Walker		15-Jan-1990			      */
/*		Perform post-motif-port modifications.			      */
/*	001	S. Lavigne 		05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/

#define  SVN_DISPLAY

#ifdef VMS
/*#pragma nostandard*/
#include "XmP.h"
#include "MrmPublic.h"
#include "StringDefs.h"
#include "Text.h"
#include "cursorfont.h"
#include "descrip.h"
extern char *VMSDescToNull();  /* jmg */
/*#pragma standard*/
#else
#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#include <X11/StringDefs.h>
#include <Xm/Text.h>
#include <X11/cursorfont.h>
#endif

#include "Pane.h"
#include "PaneP.h"
#include "DXmSvnP.h"
#include "svnprivate.h"

/*
**  This module defines all display routines that can be called from 
**  applications and from other modules within the SVN widget.  This 
**  module then dispatches to either the outline mode or the tree mode.
**
**  The routines starting with Svn are callable directly from applications.
**  Routines that start with DisplaySvn are called from the SVN.C module which
**  is called by an application.  The Display routines are called only from 
**  other modules and not directly in response to application calls.
*/

/*
** Forward Declarations
*/
#ifdef _NO_PROTO
XmFontList DisplayFontList();
static Cardinal DisplayInsertOrder();
#else
XmFontList DisplayFontList(svn_widget svnw , DXmSvnEntryPtr svnentry ,
						DXmSvnCompPtr comp );
static Cardinal DisplayInsertOrder(Widget w);
#endif


/*
**  XtCallbackRec structures for callbacks
*/
    static XtCallbackRec OutlineScrollHelp_CBstruct [2] = {{(XtCallbackProc)OutlineScrollHelp, (XtPointer)NULL},
							   {(XtCallbackProc)NULL, (XtPointer)NULL}};

    static XtCallbackRec OutlineVScroll_CBstruct    [2] = {{(XtCallbackProc)OutlineVScroll, (XtPointer)NULL},
							   {(XtCallbackProc)NULL, (XtPointer)NULL}};

    static XtCallbackRec OutlineHScroll_CBstruct    [2] = {{(XtCallbackProc)OutlineHScroll, (XtPointer)NULL},
							   {(XtCallbackProc)NULL, (XtPointer)NULL}};

    static XtCallbackRec DisplayNavButton_CBstruct  [2] = {{(XtCallbackProc)DisplayNavButton, (XtPointer)NULL},
							   {(XtCallbackProc)NULL, (XtPointer)NULL}};
 

/*
**  Translations/Actions for totop and tobottom buttons.  Note that these pass
**  an argument to the called procedure.
*/
#if 0
    static char TopButtonTranslation[] =
      "~Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNSCROLLBUTTONDOWN(1)\n\
        Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNSCROLLBUTTONDOWN(2)\n\
       ~Shift ~Ctrl ~Meta ~Help<Btn1Up>:	SVNSCROLLBUTTONUP()\n\
        Shift ~Ctrl ~Meta ~Help<Btn1Up>:	SVNSCROLLBUTTONUP()\n\
        Help<BtnUp>:                            DWTPBHELP()";
#else
    static char TopButtonTranslation[] =
      "~Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNSCROLLBUTTONDOWN(1)\n\
        Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNSCROLLBUTTONDOWN(2)\n\
       ~Shift ~Ctrl ~Meta ~Help<Btn1Up>:	SVNSCROLLBUTTONUP()\n\
        Shift ~Ctrl ~Meta ~Help<Btn1Up>:	SVNSCROLLBUTTONUP()";
#endif

#if 0
    static char BotButtonTranslation[] =
      "~Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNSCROLLBUTTONDOWN(3)\n\
        Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNSCROLLBUTTONDOWN(4)\n\
       ~Shift ~Ctrl ~Meta ~Help<Btn1Up>:	SVNSCROLLBUTTONUP()\n\
        Shift ~Ctrl ~Meta ~Help<Btn1Up>:	SVNSCROLLBUTTONUP()\n\
        Help<BtnUp>:                            DWTPBHELP()";
#else
    static char BotButtonTranslation[] =
      "~Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNSCROLLBUTTONDOWN(3)\n\
        Shift ~Ctrl ~Meta ~Help<Btn1Down>:      SVNSCROLLBUTTONDOWN(4)\n\
       ~Shift ~Ctrl ~Meta ~Help<Btn1Up>:	SVNSCROLLBUTTONUP()\n\
        Shift ~Ctrl ~Meta ~Help<Btn1Up>:	SVNSCROLLBUTTONUP()";
#endif

    static XtActionsRec ButtonActionsList[] = {
      {"SVNSCROLLBUTTONDOWN",   (XtActionProc)OutlineScrollButtonDown},
      {"SVNSCROLLBUTTONUP",     (XtActionProc)OutlineScrollButtonUp},
      {(char *) NULL, (XtActionProc)NULL} };


/*
**  Definition of Nav button icons
*/
/*
 * the size available for pixmap on 17x17 button is 9x9
 * after shadow and without margins. I'll put the appropriate pixmap
 * and define marginWidth and marginHeight as 0 to avoid acrobatics
 * in LclAdjustNavButton in SVN_DISPLAY_TOPTREE.C
 */
#define nav_open_width 9
#define nav_open_height 9
static unsigned char nav_open_bits[] = {
   0x00, 0x00, 0xfe, 0x00, 0x82, 0x00, 0xba, 0x00, 0xaa, 0x00, 0xba, 0x00,
   0x82, 0x00, 0xfe, 0x00, 0x00, 0x00};

#define nav_close_width 9
#define nav_close_height 9
static unsigned char nav_close_bits[] = {
   0x00, 0x00, 0xfe, 0x00, 0x82, 0x00, 0x82, 0x00, 0x82, 0x00, 0x82, 0x00,
   0x82, 0x00, 0xfe, 0x00, 0x00, 0x00};

/* Actually, this one is not used, but let's be consistent             */
#define no_nav_width 9
#define no_nav_height 9
static unsigned char no_nav_bits[] = {
   0x00, 0x00, 0xfe, 0x00, 0x82, 0x00, 0xba, 0x00, 0xaa, 0x00, 0xba, 0x00,
   0x82, 0x00, 0xfe, 0x00, 0x00, 0x00};

#define up_icon_width 17
#define up_icon_height 17
static unsigned char up_icon_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x01, 0x00, 0x80, 0x02, 0x00, 0x40, 0x04, 0x00, 0x20, 0x08, 0x00,
   0x10, 0x10, 0x00, 0x08, 0x20, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00};

#define down_icon_width 17
#define down_icon_height 17
static unsigned char down_icon_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x08, 0x20, 0x00,
   0x10, 0x10, 0x00, 0x20, 0x08, 0x00, 0x40, 0x04, 0x00, 0x80, 0x02, 0x00,
   0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00};

/*
**  Routine:  DXmSvnMapPosition
**
**  This routine must determine the entry number and component number that are 
**  being displayed at the given x,y coordinate.  We will assume that this 
**  coordinate is relative to the SVN window.
*/
#ifdef _NO_PROTO
void DXmSvnMapPosition (w, findx, findy, entry_number, component, tag)
  Widget w;
  int findx;
  int findy;
  int *entry_number;
  int *component;
  XtPointer *tag;
#else
void DXmSvnMapPosition (Widget w, int findx, int findy, 
			int *entry_number, int *component, XtPointer *tag)
#endif
{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Dispatch to the appropriate routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineMapPosition (svnw, findx, findy, entry_number, component, tag);  break;
        case DXmSvnKdisplayOutline: OutlineMapPosition (svnw, findx, findy, entry_number, component, tag);  break;
        case DXmSvnKdisplayTree:    TopTreeMapPosition (svnw, findx, findy, entry_number, component, tag);  break;
      };
}

/*
**  Routine:  DXmSvnPositionDisplay
**
**  This routine tries to satisfy an applications request to make a particular
**  entry displayed at a particular position.
*/

#ifdef _NO_PROTO
int DXmSvnPositionDisplay (w, entry_number, position)

  Widget w;
  int entry_number;
  int position;
#else
int DXmSvnPositionDisplay (Widget w, int entry_number, int position)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data declarations
*/
    int status;


/*
**  Dispatch to the appropriate routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: status = OutlinePositionDisplay (svnw, entry_number, position);  break;
        case DXmSvnKdisplayOutline: status = OutlinePositionDisplay (svnw, entry_number, position);  break;
        case DXmSvnKdisplayTree:    status = TopTreePositionDisplay (svnw, entry_number, position);  break;
      };


/*
**  Return the value
*/
    return (status);
}

/*
**  Routine:  DXmSvnAutoScrollCheck
**
**  This routine is called when the screen may need autoscrolled.  The module
**  SVN.C uses this routine in SVN dragging and in range selection.
*/

#ifdef _NO_PROTO
int DXmSvnAutoScrollCheck (w, x, y)

  Widget w;
  int x;
  int y;
#else
int DXmSvnAutoScrollCheck (Widget w, int x, int y)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data declarations
*/
    int status;


/*
**  Dispatch to the appropriate routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: status = OutlineAutoScrollCheck (svnw, y);  break;
        case DXmSvnKdisplayOutline: status = OutlineAutoScrollCheck (svnw, y);  break;
        case DXmSvnKdisplayTree:    status = TopTreeAutoScrollCheck (svnw, x, y);  break;
      };


/*
**  Return the status
*/
    return (status);
}

/*
**  Routine:  DXmSvnAutoScrollDisplay
**
**  This routine is called when the screen may need autoscrolled.  The module
**  SVN.C uses this routine in SVN dragging and in range selection.
*/

#ifdef _NO_PROTO
void DXmSvnAutoScrollDisplay (w, x, y)
  Widget w;
  int x;
  int y;
#else
void DXmSvnAutoScrollDisplay (Widget w, int x, int y)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);


/*
**  Dispatch to the appropriate routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineAutoScrollDisplay (svnw, y);  break;
        case DXmSvnKdisplayOutline: OutlineAutoScrollDisplay (svnw, y);  break;
        case DXmSvnKdisplayTree:    TopTreeAutoScrollDisplay (svnw, x, y);  break;
      };
}

/*
** Routine:  DXmSvnGetNumDisplayed
*/
#ifdef _NO_PROTO
int DXmSvnGetNumDisplayed (w)

    Widget w;
#else
int DXmSvnGetNumDisplayed (Widget w)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local data declarations
*/
    int count;


/*
**  Return to the caller the current number of entries being displayed.
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: count = OutlineGetNumDisplayed (svnw);  break;
        case DXmSvnKdisplayOutline: count = OutlineGetNumDisplayed (svnw);  break;
        case DXmSvnKdisplayTree:    count = 0;  break;
      };


/*
**  Return the count
*/
    return count;
}

/*
** Routine:  DXmSvnGetDisplayed
**
**  Call the display specific routine to get the list.
*/

#ifdef _NO_PROTO
void DXmSvnGetDisplayed (w, entry_nums, entry_tags, y_coords, num_array_entries)

  Widget w;
  int *entry_nums;
  XtPointer *entry_tags;
  int *y_coords;
  int num_array_entries;
#else
void DXmSvnGetDisplayed (Widget w, int *entry_nums, 
			 XtPointer *entry_tags, int *y_coords, 
			 int num_array_entries)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  If it's not tree mode, then call the right procedure.
*/
    if (svnw->svn.display_mode != DXmSvnKdisplayTree)
         OutlineGetDisplayed (svnw, entry_nums, entry_tags, y_coords, num_array_entries);
    else
         {
          /*
          **  Tree mode does not support this procedure...
          */
              int i;
              for (i = 1;  i < num_array_entries;  i++)
                  {
                   if (entry_nums != (int *) NULL) entry_nums [i-1] = 0;
                   if (entry_tags != (XtPointer *) NULL) entry_tags [i-1] = 0;
                   if (y_coords   != (int *) NULL) y_coords   [i-1] = 0;
                  };
         };
}

/*
**  Routine:  DisplayCreateGhost
**
**  This routine is called to create the ghost image to be used in a dragging
**  operation.  The identifier of the ghosting pixmap is saved in the instance
**  record.
*/

#ifdef _NO_PROTO
void DisplayCreateGhost (svnw, entry_number)

  svn_widget svnw;
  int entry_number;
#else
void DisplayCreateGhost (svn_widget svnw, int entry_number)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineCreateGhost (svnw, entry_number);  break;
        case DXmSvnKdisplayOutline: OutlineCreateGhost (svnw, entry_number);  break;
        case DXmSvnKdisplayTree:    TopTreeCreateGhost (svnw, entry_number);  break;
      };
}

/*
**  Routine:  DisplayDeleteGhost
**
**  This routine is called to delete the ghost image used in a dragging operation.
*/

#ifdef _NO_PROTO
void DisplayDeleteGhost (svnw)

  svn_widget svnw;
#else
void DisplayDeleteGhost (svn_widget svnw)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineDeleteGhost (svnw);  break;
        case DXmSvnKdisplayOutline: OutlineDeleteGhost (svnw);  break;
        case DXmSvnKdisplayTree:    TopTreeDeleteGhost (svnw);  break;
      };
}

/*
**  Routine:  DisplaySvnInvalidateEntry
**
**  This routine is called from SVN.C to do the display portion of the source 
**  module invalidating an entry.  The purpose of this routine is to merely 
**  record the fact that this entry was invalidated.  If the entry is not 
**  currently in the display viewport, then this call is ignored.
*/

#ifdef _NO_PROTO
void DisplaySvnInvalidateEntry (svnw, entry_number)

  svn_widget svnw;
  int entry_number;
#else
void DisplaySvnInvalidateEntry (svn_widget svnw, int entry_number)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineInvalidateEntry (svnw, entry_number);  break;
        case DXmSvnKdisplayOutline: OutlineInvalidateEntry (svnw, entry_number);  break;
        case DXmSvnKdisplayTree:    TopTreeInvalidateEntry (svnw, entry_number);  break;
      };
}

/*
**  Routine:  DisplaySvnSetEntrySensitivity
**
**  This routine is called whenever the application or source module has called 
**  the SvnSetEntrySensitivity high level call which is located in SVN.C.
**
**  This routine is responsible for seeing if its currently in the display 
**  viewport and if the widget is enabled, to changed the sensitivity right 
**  away.  The caller of this routine has ensured that the sensitivity has 
**  actually changed.  If the entry in question is not in the viewport, then 
**  this call can be ignored.
**
**  Note that the sensitivity is the inverse of grayed and has already been set
**  by the corresponding Structure.C routine.
*/

#ifdef _NO_PROTO
void DisplaySvnSetEntrySensitivity (svnw, entry_number)

  svn_widget svnw;
  int entry_number;
#else
void DisplaySvnSetEntrySensitivity (svn_widget svnw, int entry_number)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineSetEntrySensitivity (svnw, entry_number);  break;
        case DXmSvnKdisplayOutline: OutlineSetEntrySensitivity (svnw, entry_number);  break;
        case DXmSvnKdisplayTree:    TopTreeSetEntrySensitivity (svnw, entry_number);  break;
      };
}

/*
**  Routine:  DisplaySvnAddEntries
**
**  This routine is called indirectly from the SvnAddEntries routine
**  located in SVN.C.  
*/

#ifdef _NO_PROTO
void DisplaySvnAddEntries (svnw, after, number)

  svn_widget svnw;
  int after;
  int number;
#else
void DisplaySvnAddEntries (svn_widget svnw, int after, int number)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineAddEntries (svnw, after, number);  break;
        case DXmSvnKdisplayOutline: OutlineAddEntries (svnw, after, number);  break;
        case DXmSvnKdisplayTree:    TopTreeAddEntries (svnw, after);  break;
      };
}

/*
**  Routine:  DisplaySvnDeleteEntries
**
**  This routine is called indirectly from the SvnDeleteEntries routine
**  located in SVN.C.  It's job is to mark the entries that will need 
**  refreshed, when the widget is re-enabled.
*/

#ifdef _NO_PROTO
void DisplaySvnDeleteEntries (svnw, after, number)

  svn_widget svnw;
  int after;
  int number;
#else
void DisplaySvnDeleteEntries (svn_widget svnw, int after, int number)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineDeleteEntries (svnw, after, number);  break;
        case DXmSvnKdisplayOutline: OutlineDeleteEntries (svnw, after, number);  break;
        case DXmSvnKdisplayTree:    TopTreeDeleteEntries (svnw, after, number);  break;
      };
}

/*
**  Routine:  DisplaySvnEnableDisplay
**
**  This routine is called indirectly from the SvnEnableDisplay routine
**  located in SVN.C.  It's job is to get the screen up to date based on calls
**  that were made during the disabled state.
*/

#ifdef _NO_PROTO
void DisplaySvnEnableDisplay (svnw)

  svn_widget svnw;
#else
void DisplaySvnEnableDisplay (svn_widget svnw)
#endif

{
/*
**  If this widget has not yet been realized, then ignore this call.  It will
**  be displayed when the expose event comes through.
*/
    if (!XtIsRealized(svnw)) return;


/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineEnableDisplay (svnw);  break;
        case DXmSvnKdisplayOutline: OutlineEnableDisplay (svnw);  break;
        case DXmSvnKdisplayTree:    TopTreeEnableDisplay (svnw);  break;
      };
}

/*
**  Routine:  DisplayHighlightEntry
**
**  This routine is called when the selection status of an entry has 
**  transitioned from low to high.  It may not be on the screen at this
**  time (Select All).
*/

#ifdef _NO_PROTO
void DisplayHighlightEntry (svnw, entry_number)

  svn_widget svnw;
  int entry_number;
#else
void DisplayHighlightEntry (svn_widget svnw, int entry_number)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineHighlightEntry (svnw, entry_number);  break;
        case DXmSvnKdisplayOutline: OutlineHighlightEntry (svnw, entry_number);  break;
        case DXmSvnKdisplayTree:    TopTreeHighlightEntry (svnw, entry_number);  break;
      };
}

/*
**  Routine:  DisplayDraw
**
**  This routine is called when the widget is exposed or when the widget is
**  enabled and we don't know how many will fit.
*/

#ifdef _NO_PROTO
void DisplayDraw (svnw)

  svn_widget svnw;
#else
void DisplayDraw (svn_widget svnw)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineDraw (svnw);  break;
        case DXmSvnKdisplayOutline: OutlineDraw (svnw);  break;
        case DXmSvnKdisplayTree:    TopTreeDraw (svnw);  break;
      };
}

/*  
**  Routine:  DisplayAdjustHeight
**
**  This routine is called by the STRUCTURE module whenever a valid entry
**  is desired.  It is only called if 'height_adjusted' is false.
*/

#ifdef _NO_PROTO
void DisplayAdjustHeight (svnw, svnentry)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
#else
void DisplayAdjustHeight (svn_widget svnw, DXmSvnEntryPtr svnentry)
#endif

{	
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineAdjustHeight (svnw, svnentry);  break;
        case DXmSvnKdisplayOutline: OutlineAdjustHeight (svnw, svnentry);  break;
        case DXmSvnKdisplayTree:    TopTreeAdjustHeight (svnw, svnentry);  break;
      };
}

/*
**  Procedure to change the geometry of a sub-widget.
**/

#ifdef _NO_PROTO
static void LclDisplayResizeSubwidget (svnw, w, x, y, width, height)

   svn_widget svnw;
   Widget w;
   Position x, y;
   Dimension width, height;
#else
static void LclDisplayResizeSubwidget (svn_widget svnw, Widget w, Position x, 
				Position y, Dimension width, Dimension height)
#endif

{
/*
**  If the widget is not realized, then complain
*/
    if (!XtIsRealized (w)) return;


/*
**  Don't allow resizes to place widgets at negative positions or at positions
**  outside of the window...
*/
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (x > XtWidth (svnw))  x = XtWidth (svnw);
    if (y > XtHeight(svnw))  y = XtHeight(svnw);


/*
**  Don't allow resizes to make children bigger than SVN itself
*/
    if (width  > (XtWidth (svnw) - x))  width  = XtWidth (svnw) - x;
    if (height > (XtHeight(svnw) - y))  height = XtHeight(svnw) - y;

/*
**  Width and Height must be nonzero
*/
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;

/*
**  If the X or Y position is different, then move the sub widget
*/
    if ((XtX(w) != x) || (XtY(w) != y))
       {
         XtMoveWidget (w, (Position) x, (Position) y);
       };


/*
**  If the width or height is different, then set the new values and resize
**  the window.  Tell the widget about it afterwards.
*/
    if ((XtWidth(w) != width) || (XtHeight(w) != height))
	XtResizeWidget(w, width, height, w->core.border_width);
}

/*
**  Routine:  DisplayResize
**
**  This routine is called after the user has resized the window.  Note
**  that this procedure assumes that all border widths are 1.
*/

#ifdef _NO_PROTO
void DisplayResize (svnw)

  svn_widget svnw;
#else
void DisplayResize (svn_widget svnw)
#endif

{
/*
**  Local data declarations
*/
    Position x, y;
    Dimension width, height, primary_width, secondary_width;
    int spacing;
    Widget primary_scrolled_window;

    primary_scrolled_window = XtParent(svnw->svn.primary_form);
    XtVaGetValues(primary_scrolled_window, XmNspacing, &spacing, NULL);


/*
**  The pane widget needs resized.
*/
    x      = 0;
    y      = 0;
    width  = XtWidth (svnw) - button_height - 2 - spacing;
    height = XtHeight (svnw);

    if (LayoutIsRtoL(svnw))
	/* Leave space for the vertical scroll bar. */
        LclDisplayResizeSubwidget (svnw, svnw->svn.pane_widget, button_height + 3, y, width, height);
    else
        LclDisplayResizeSubwidget (svnw, svnw->svn.pane_widget, x, y, width, height);


/*
**  Default the primary_width to be the whole work area in case there is no secondary.
*/
    primary_width = width;


/*
**  Since the window has resized, we must apply the primary_percentage.
**  Make sure that if we have greater than 2 panes in SVN... we do not
**  touch the sizing of the primary or secondary ... 
*/
    if ((svnw->svn.display_mode == DXmSvnKdisplayColumns) &&
	(PaneNumChildren(svnw->svn.pane_widget) < 3))
       {
	/*
	**  The right hand side scroll window is the parent of
	**  the right hand side window widget.
	*/
	    Widget secondary_scroll_window = XtParent(svnw->svn.secondary_form);

	/*
	**  Ensure legal values...
	*/
	    if (svnw->svn.primary_percentage <   0) svnw->svn.primary_percentage = 0;
	    if (svnw->svn.primary_percentage > 100) svnw->svn.primary_percentage = 100;

        /*
        **  Compute the right and left hand side widths.
	*/
            primary_width = primary_width - mullion_width;
            secondary_width = ((100 - svnw->svn.primary_percentage) * primary_width) / 100;

	    /******************************************************************************/
	    /*                                                                            */
	    /* If the with of the secondary comes out to be 0...(meaning that 100 percent was   */
	    /* selected for primary_percentage... then make sure that the window is at least  */
	    /* created with the width of 1.						*/
	    /*                                                                            */
	    /******************************************************************************/
	    if (secondary_width == 0)
		secondary_width = 1;

	    primary_width = primary_width - secondary_width;


	/*
	**  Make the Pane widget readjust
	*/
	    PaneSetMinMax (secondary_scroll_window, secondary_width, secondary_width);
	    PaneSetMinMax (secondary_scroll_window, 0, 0);
       };


/*
**  Set the size of the vertical scrollbar.  The Y position and the Width remain
**  the same, while the X and Height may change.
*/
    x      = XtWidth (svnw) - button_height - 2;
    width  = button_height;
    y      = 0;
    height = XtHeight(svnw) - button_height - 3;

    if (svnw->svn.use_scroll_buttons)
       {
	 y      = button_height + 1;
	 height = height - (2 * button_height) - 2;
       };


    if (LayoutIsRtoL(svnw))
	/* Vertical scroll bar will be on the left border. */
        LclDisplayResizeSubwidget (svnw, svnw->svn.vscroll, 0, y, width, height);
    else
        LclDisplayResizeSubwidget (svnw, svnw->svn.vscroll, x, y, width, height);


/*
**  Set the size and position of the Nav button. 
*/
    y      = XtHeight(svnw) - button_height - 2;
    x      = XtWidth (svnw) - button_height - 2;
    height = button_height;
    width  = button_height;
    if (LayoutIsRtoL(svnw))
        LclDisplayResizeSubwidget (svnw, svnw->svn.nav_button, 0, y, width, height);
    else
        LclDisplayResizeSubwidget (svnw, svnw->svn.nav_button, x, y, width, height);


/*
**  Move the buttons if we are using them...
*/
    if (svnw->svn.use_scroll_buttons)
       {
         x      = XtWidth (svnw) - button_height - 2;
         y      = 0;
         width  = button_height;
         height = button_height;
         if (LayoutIsRtoL(svnw))
             LclDisplayResizeSubwidget (svnw, svnw->svn.top_button, 0, y, width, height);
         else
             LclDisplayResizeSubwidget (svnw, svnw->svn.top_button, x, y, width, height);

         y      = XtHeight(svnw) - (2 * button_height) - 3;
         if (LayoutIsRtoL(svnw))
             LclDisplayResizeSubwidget (svnw, svnw->svn.bot_button, 0, y, width, height);
         else
             LclDisplayResizeSubwidget (svnw, svnw->svn.bot_button, x, y, width, height);
       };

    /*
    **  For each of the entries in the display, unmap their subwidgets and lose the
    **  pointer to the entry, therefore when the resize completes they will be remanaged
    **  in their correct location.
    */
    if (svnw->svn.sub_widgets_used)
	LclOutlineHideWidgets(svnw, 1, svnw->svn.display_count);


/*
**  Dispatch to the proper routine to do any remaining work.
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineResize (svnw);  break;
        case DXmSvnKdisplayOutline: OutlineResize (svnw);  break;
        case DXmSvnKdisplayTree:    TopTreeResize (svnw);  break;
      };
}

/*
**  Routine:  DisplayWorkAreaFixup
**
**  This routine is called when one of the work areas is not totally occupying
**  the scroll window areas.  This situation is triggered by EXPOSE events 
**  being generated on the svn window itself.
*/

#ifdef _NO_PROTO
void DisplayWorkAreaFixup (svnw)

  svn_widget svnw;
#else
void DisplayWorkAreaFixup (svn_widget svnw)
#endif

{
/*
**  Local data declarations
*/
    Dimension primary_width, secondary_width, height;


/*
**  Now we must resize our work areas that are in the scroll windows.
*/
    primary_width  = XtWidth (svnw->svn.hscroll);
    height = XtY (svnw->svn.hscroll);
    LclDisplayResizeSubwidget (svnw, svnw->svn.primary_window_widget, 0, 0, primary_width, height);


/*
**  If we have a secondary...
*/
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
       {
        secondary_width  = XtWidth (svnw->svn.secondary_hscroll);
        height = XtY (svnw->svn.secondary_hscroll);
        LclDisplayResizeSubwidget (svnw, svnw->svn.secondary_window_widget, 0, 0, secondary_width, height);
        svnw->svn.display_changed = TRUE;
	svnw->svn.primary_percentage = (primary_width * 100) / (primary_width + secondary_width); 
       };
}

/*
**  This function controls the insertion order of children of the form widgets.  This is
**  necessary to get subwidget components of entries to be above the Svn and separator
**  widgets in the stacking order so they are visible.
*/
static Cardinal DisplayInsertOrder (w)
Widget w;				/* widget being inserted */
{
    if (XtIsSubclass((Widget)w, (WidgetClass)dxmSvnWindowWidgetClass) || XmIsSeparatorGadget(w))
    {
	Cardinal num_children;

	XtVaGetValues(XtParent(w), XtNnumChildren, &num_children, NULL);
	return (num_children);		/* add child to end of children list */
    }
    else
	return (0);			/* add child to beginning of children list */
}


XtEventHandler LclDisplayPrimaryResize (w, client_data, event, continue_to_dispatch)
Widget w;
XtPointer client_data;
XEvent *event;
Boolean *continue_to_dispatch;
{
/*
**  Local data declarations
*/
    svn_widget svnw;
    Widget scrolled_window;


    /*
    **  Go up through the parent chain looking for the svn class widget.  
    */
    svnw = StructFindSvnWidget (w);

    /*
    **	Get the primary scrolled window.
    */
    scrolled_window = XtParent(svnw->svn.primary_form);

    if (w == scrolled_window && event->xany.window == XtWindow(w) && event->type == ConfigureNotify)
    {
	/*  Received a ConfigureNotify event for the primary scrolled window
	**  which means the scrolled window was resized.  Need to update the
	**  primary_percentage resource and adjust the scrollbars if it changed.
        **
	*/

#ifdef DEC_MOTIF_BUG_FIX
	/*
	**  If the layout is RtoL, we need to clear the window which in turn
	**  will cause an expose event. 
	*/
        if (LayoutIsRtoL(svnw))
	   {
           XClearArea (XtDisplay(svnw),
                       XtWindow(svnw->svn.primary_window_widget), 0, 0,
                       XtWidth(svnw->svn.primary_window_widget),
                       XtHeight(svnw->svn.primary_window_widget), TRUE);
	   }
#endif /* DEC_MOTIF_BUG_FIX */


	if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	{
	    int	percentage;

#ifdef DEC_MOTIF_BUG_FIX
	    /*
	    **  If the layout is RtoL, we need to clear the window which in turn
	    **  will cause an expose event. 
    	    */
            if (LayoutIsRtoL(svnw))
	        {
                XClearArea (XtDisplay(svnw),
                       XtWindow(svnw->svn.secondary_window_widget), 0, 0,
                       XtWidth(svnw->svn.secondary_window_widget),
                       XtHeight(svnw->svn.secondary_window_widget), TRUE);
		}
#endif /* DEC_MOTIF_BUG_FIX */

	    percentage = (XtWidth(svnw->svn.hscroll) * 100) / (XtWidth(svnw->svn.hscroll) + XtWidth(svnw->svn.secondary_hscroll));

	    if (percentage != svnw->svn.primary_percentage)
	    {
		svnw->svn.primary_percentage = percentage;
		LclOutlineScrollAdjust(svnw);
	    }
	}
    }
}

/*
**  Routine:  DisplayCreate
**
**  This routine is called during the Initialize phase of the widget.
**
**  This routine creates the vertical and horizontal scroll bars.  It also 
**  creates the index window but does not manage it.
**
**  This routine only creates the widgets.  During the realize stage of the
**  SVN initialization all of the widgets are positioned correctly anyway, so
**  just try to get as much of them right as possible.
**  
**    Svn widget
**     |
**     +---- Top arrow push button
**     |
**     +---- Vertical scroll bar
**     |
**     +---- Bottom arrow push button
**     |
**     +---- Corner navigation button
**     |
**     +---- Pane widget (Readonly resource)
**            |
**            +---- Scroll Window
**            |      |
**            |      +---- Horizontal scroll bar
**	      |	     |
**	      |	     +---- Form
**            |	     	    |
**	      |	     	    +---- Simple window
**            |		    |
**	      |		    +---- Separator
**            |		    |
**	      |		    +---- Simple window
**	      |
**            +---- Scroll Window
**		     |
**		     +---- Horizontal scroll bar
**	      	     |
**	      	     +---- Form
**            	     	    |
**	      	     	    +---- Simple window
**            		    |
**	      		    +---- Separator
**            		    |
**	      		    +---- Simple window
**
*/

#ifdef _NO_PROTO
void DisplayCreate (svnw)

  svn_widget svnw;
#else
void DisplayCreate (svn_widget svnw)
#endif

{
/*
**  Local data declarations
*/
    Arg arglist[30];
    int	argcnt = 0;

    Dimension pane_width, primary_width, secondary_width, height;
    XtTranslations compiled_translations = NULL;
    Widget primary_scroll_window, secondary_scroll_window;



static char scroll_window_translations [] =
   "<ConfigureNotify>:  SVNWORKAREAFIXUP()";

/*
**  Compute the default pane width based on the SVN width.  Make
**  the minimum width 100 pixels.
*/
    pane_width = XtWidth(svnw) - button_height;
    if (pane_width < 100) pane_width = 100;


/*
**  Compute the right hand side and left hand side widths
**  to be 1/2 of the pane.
*/
    primary_width = pane_width;
    secondary_width = pane_width;


/*
**  Try to do better if this is in columns mode.
*/
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	{
          primary_width = primary_width - mullion_width;
          secondary_width = ((100 - svnw->svn.primary_percentage) * primary_width) / 100;

	  /******************************************************************************/
          /*                                                                            */
          /* If the with of the secondary comes out to be 0...(meaning that 100 percent was   */
	  /* selected for primary_percentage... then make sure that the window is at least  */
	  /* created with the width of 1.						*/
          /*                                                                            */
          /******************************************************************************/
	  if (secondary_width == 0)
	    secondary_width = 1;
	    

	  primary_width = primary_width - secondary_width;
        };


/*
**  Compute the height (minimum of 100)
*/
    height = XtHeight (svnw);
    if (height < 100) height = 100;


/*
**  Create the Pane widget.  We remove the translations from the Pane widget
**  because it causes button events to be absorbed just due to it asking for
**  help modified buttons.  
*/
    argcnt = 0;
    XtSetArg (arglist[argcnt], XmNresizePolicy, XmRESIZE_NONE); argcnt++;
    XtSetArg (arglist[argcnt], XmNorientation, XmHORIZONTAL); argcnt++;
    XtSetArg (arglist[argcnt], XmNborderWidth, 0); argcnt++;
    XtSetArg (arglist[argcnt], XmNwidth, pane_width); argcnt++;
    XtSetArg (arglist[argcnt], XmNheight, height); argcnt++;
    XtSetArg (arglist[argcnt], XmNtraversalOn, TRUE); argcnt++;

    svnw->svn.pane_widget = (Widget) PaneCreateWidget ((Widget) svnw, "pane", arglist, argcnt);
    XtUninstallTranslations (svnw->svn.pane_widget); 
    XtManageChild (svnw->svn.pane_widget);



/*
**  Create the two scroll windows and put them in the Pane widget.
*/
    argcnt = 0;
    XtSetArg(arglist[argcnt], XmNx, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNy, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNwidth, primary_width); argcnt++;
    XtSetArg(arglist[argcnt], XmNheight, height); argcnt++;
    XtSetArg(arglist[argcnt], XmNscrollingPolicy, XmAPPLICATION_DEFINED); argcnt++;
    XtSetArg(arglist[argcnt], XmNborderWidth, 0); argcnt++;
    XtSetArg (arglist[argcnt], XmNtraversalOn, TRUE); argcnt++;
    /*
    ** The 'primary' window can be on the right or left half of pane.
    */
    if (LayoutIsRtoL(svnw))
        secondary_scroll_window = (Widget) XmCreateScrolledWindow(svnw->svn.pane_widget,
                        "primary",
                        arglist, argcnt);
    else
        primary_scroll_window = (Widget) XmCreateScrolledWindow(svnw->svn.pane_widget,
                        "primary",
                        arglist, argcnt);

    XtSetArg(arglist[argcnt], XmNx, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNy, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNwidth, secondary_width); argcnt++;
    XtSetArg(arglist[argcnt], XmNheight, height); argcnt++;
    XtSetArg(arglist[argcnt], XmNscrollingPolicy, XmAPPLICATION_DEFINED); argcnt++;
    XtSetArg(arglist[argcnt], XmNborderWidth, 0); argcnt++;
    XtSetArg (arglist[argcnt], XmNtraversalOn, TRUE); argcnt++;

    /*
    **  The 'secondary' window will be on the left half of the pane.
    */
    if (LayoutIsRtoL(svnw))
        primary_scroll_window = (Widget) XmCreateScrolledWindow(svnw->svn.pane_widget,
                        "secondary",
                        arglist, argcnt);
    else
        secondary_scroll_window = (Widget) XmCreateScrolledWindow(svnw->svn.pane_widget,
                        "secondary",
                        arglist, argcnt);


    /*
    **  Add an event handler to the primary scroll window so when the pane sash
    **  is moved, we can update the primary_percentage resource.
    */
    XtAddEventHandler((Widget)primary_scroll_window, StructureNotifyMask, False, (XtEventHandler)LclDisplayPrimaryResize, NULL);
    
    /******************************************************************************/
    /*                                                                            */
    /* Create the form widget for the primary scrolled window.			  */
    /*                                                                            */
    /******************************************************************************/
    argcnt = 0;
    XtSetArg (arglist[argcnt], XmNresizePolicy, XmRESIZE_ANY); argcnt++;
    XtSetArg (arglist[argcnt], XtNinsertPosition, (XtOrderProc) DisplayInsertOrder); argcnt++;
    XtSetArg(arglist[argcnt], XmNmarginHeight, svnw->svn.margin_height);argcnt++;

    if (LayoutIsRtoL(svnw))
	svnw->svn.secondary_form = (Widget)XmCreateForm	(secondary_scroll_window, "primary_form", arglist, argcnt);
    else
	svnw->svn.primary_form = (Widget)XmCreateForm (primary_scroll_window, "primary_form", arglist, argcnt);

    /******************************************************************************/
    /*                                                                            */
    /* Create the form widget for the secondary scrolled window.		  */
    /*                                                                            */
    /******************************************************************************/
    argcnt = 0;
    XtSetArg (arglist[argcnt], XmNresizePolicy, XmRESIZE_ANY); argcnt++;
    XtSetArg (arglist[argcnt], XtNinsertPosition, (XtOrderProc) DisplayInsertOrder); argcnt++;
    XtSetArg(arglist[argcnt], XmNmarginHeight, svnw->svn.margin_height);argcnt++;
    if (LayoutIsRtoL(svnw))
	svnw->svn.primary_form = (Widget)XmCreateForm (primary_scroll_window, "secondary_form", arglist, argcnt);
    else
	svnw->svn.secondary_form = (Widget)XmCreateForm (secondary_scroll_window, "secondary_form", arglist, argcnt);

    /*
    **	Remove the accelerators from the form widgets.  Must do this to keep the forms
    **	from grabbing the return key.
    */
    argcnt = 0;
    XtSetArg(arglist[argcnt], XmNaccelerators, NULL); argcnt++;
    XtSetValues(svnw->svn.primary_form, arglist, argcnt);
    XtSetValues(svnw->svn.secondary_form, arglist, argcnt);

/*
**  Manage the right hand side only if we are in columns mode
*/
    XtManageChild (svnw->svn.primary_form);
    XtManageChild (primary_scroll_window);
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
       {
       XtManageChild (svnw->svn.secondary_form);
       XtManageChild (secondary_scroll_window);
       }
       
    /******************************************************************************/
    /*                                                                            */
    /* Create the simple windows that are going to be attached to the FORM widgets */
    /* for the primary and secondary scrolled windows and the separators. 	  */
    /*										  */
    /* After creation of the window widgets for both the path-to-root and	  */
    /* the main entry area, just manage the main area, because we don't need      */
    /* the path-to-root windows until the first scroll occurs.			  */
    /*										  */
    /******************************************************************************/
    svnw->svn.primary_ptr_widget = DXmSvnWindow (svnw, svnw->svn.primary_form,"primary_ptr", primary_width, height, 2);
    svnw->svn.primary_window_widget = DXmSvnWindow (svnw, svnw->svn.primary_form, "primary_window", primary_width, height, 1);
    svnw->svn.secondary_ptr_widget = DXmSvnWindow(svnw, svnw->svn.secondary_form, "secondary_ptr", primary_width, height, 4);
    svnw->svn.secondary_window_widget = DXmSvnWindow (svnw, svnw->svn.secondary_form, "secondary_window", primary_width, height, 3);

    XtSetArg(arglist[0], XmNtopAttachment, XmATTACH_WIDGET);
    XtSetArg(arglist[1], XmNtopWidget, svnw->svn.primary_ptr_widget);
    XtSetArg(arglist[2], XmNleftAttachment, XmATTACH_FORM);
    XtSetArg(arglist[3], XmNrightAttachment, XmATTACH_FORM);
    svnw->svn.primary_separator_widget = (Widget)XmCreateSeparatorGadget(svnw->svn.primary_form, "primary_separator", arglist, 4);
    XtSetArg(arglist[1], XmNtopWidget, svnw->svn.secondary_ptr_widget);
    svnw->svn.secondary_separator_widget = (Widget)XmCreateSeparatorGadget(svnw->svn.secondary_form, "secondary_separator", arglist, 4);

    XtManageChild (svnw->svn.primary_window_widget);
    XtManageChild (svnw->svn.secondary_window_widget);


/*
**  Compile the translations and add them to the widgets
*/
    compiled_translations = XtParseTranslationTable (scroll_window_translations);
    XtAugmentTranslations (primary_scroll_window, compiled_translations); 
    XtAugmentTranslations (secondary_scroll_window, compiled_translations);


/*
**  Create the work area and the horizontal scroll for the left hand side
*/
    argcnt = 0;
    XtSetArg(arglist[argcnt], XmNx, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNy, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNwidth, primary_width); argcnt++;
    XtSetArg(arglist[argcnt], XmNheight, button_height); argcnt++;
    XtSetArg(arglist[argcnt], XmNincrement, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageIncrement, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNsliderSize, 4); argcnt++;
    XtSetArg(arglist[argcnt], XmNvalue, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNminimum, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNmaximum, 5); argcnt++;
    XtSetArg(arglist[argcnt], XmNorientation, XmHORIZONTAL); argcnt++;
    XtSetArg(arglist[argcnt], XmNvalueChangedCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNhelpCallback, 		OutlineScrollHelp_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNincrementCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNdecrementCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageIncrementCallback, OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageDecrementCallback, OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNtoTopCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNtoBottomCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNdragCallback, 		OutlineHScroll_CBstruct); argcnt++;
    XtSetArg (arglist[argcnt], XmNtraversalOn, FALSE); argcnt++;
    svnw->svn.hscroll = (Widget) XmCreateScrollBar(primary_scroll_window,
			"primary_scrollbar",
			arglist, argcnt);

    if (LayoutIsRtoL(svnw))
	{
	/*  Force it to be RtoL.  */
        XtSetArg (arglist[0], DXmNlayoutDirection, DXmLAYOUT_LEFT_DOWN);
        XtSetArg (arglist[1], XmNprocessingDirection, XmMAX_ON_LEFT);
        XtSetValues (svnw->svn.hscroll, arglist, 2);
	}

/*
**  Manage the two scroll bars
*/
    XtManageChild (svnw->svn.hscroll);


/*
**  Create the horizontal scroll for the left hand side
*/
    argcnt = 0;
    XtSetArg(arglist[argcnt], XmNx, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNy, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNwidth, secondary_width); argcnt++;
    XtSetArg(arglist[argcnt], XmNheight, button_height); argcnt++;
    XtSetArg(arglist[argcnt], XmNincrement, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageIncrement, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNsliderSize, 4); argcnt++;
    XtSetArg(arglist[argcnt], XmNvalue, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNminimum, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNmaximum, 5); argcnt++;
    XtSetArg(arglist[argcnt], XmNorientation, XmHORIZONTAL); argcnt++;
    XtSetArg(arglist[argcnt], XmNvalueChangedCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNhelpCallback, 		OutlineScrollHelp_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNincrementCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNdecrementCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageIncrementCallback, OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageDecrementCallback, OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNtoTopCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNtoBottomCallback, 	OutlineHScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNdragCallback, 		OutlineHScroll_CBstruct); argcnt++;
    XtSetArg (arglist[argcnt], XmNtraversalOn, FALSE); argcnt++;
    svnw->svn.secondary_hscroll = (Widget) XmCreateScrollBar(secondary_scroll_window,
			    "secondary_scrollbar",
			    arglist, argcnt);

    if (LayoutIsRtoL(svnw))
	{
	/*  Force it to be RtoL.  */
        XtSetArg (arglist[0], DXmNlayoutDirection, DXmLAYOUT_LEFT_DOWN);
        XtSetArg (arglist[1], XmNprocessingDirection, XmMAX_ON_LEFT);
        XtSetValues (svnw->svn.secondary_hscroll, arglist, 2);
	}

/*
**  Manage the two scroll bars
*/
    XtManageChild (svnw->svn.secondary_hscroll);


/*
**  Set the work area and scrollbars to the ScrollWindows
*/
    XmScrolledWindowSetAreas (primary_scroll_window, svnw->svn.hscroll, NULL, svnw->svn.primary_form);
    XmScrolledWindowSetAreas (secondary_scroll_window, svnw->svn.secondary_hscroll, NULL, svnw->svn.secondary_form);


/*
**  Create the vertical scrollbar widget
*/
    argcnt = 0;
    XtSetArg(arglist[argcnt], XmNx, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNy, 0); argcnt++;
    XtSetArg(arglist[argcnt], XmNwidth, button_height); argcnt++;
    XtSetArg(arglist[argcnt], XmNheight, height); argcnt++;
    XtSetArg(arglist[argcnt], XmNincrement, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageIncrement, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNsliderSize, 4); argcnt++;
    XtSetArg(arglist[argcnt], XmNvalue, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNminimum, 1); argcnt++;
    XtSetArg(arglist[argcnt], XmNmaximum, 5); argcnt++;
    XtSetArg(arglist[argcnt], XmNorientation, XmVERTICAL); argcnt++;
    XtSetArg(arglist[argcnt], XmNvalueChangedCallback, 	OutlineVScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNhelpCallback, 		OutlineScrollHelp_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNincrementCallback, 	OutlineVScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNdecrementCallback, 	OutlineVScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageIncrementCallback, OutlineVScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNpageDecrementCallback, OutlineVScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNtoTopCallback, 	OutlineVScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNtoBottomCallback, 	OutlineVScroll_CBstruct); argcnt++;
    XtSetArg(arglist[argcnt], XmNdragCallback, 		OutlineVScroll_CBstruct); argcnt++;
    XtSetArg (arglist[argcnt], XmNtraversalOn, FALSE); argcnt++;
    svnw->svn.vscroll = (Widget) XmCreateScrollBar(svnw,
			"SVSc",
			arglist, argcnt);

    XtManageChild (svnw->svn.vscroll); 


/*
**  Set up the common arglist information for all of our pushbuttons
*/
    argcnt = 0;
    XtSetArg (arglist[argcnt], XmNwidth, button_height); argcnt++;
    XtSetArg (arglist[argcnt], XmNheight, button_height); argcnt++;
    XtSetArg (arglist[argcnt], XmNlabelType, XmPIXMAP); argcnt++;
    XtSetArg (arglist[argcnt], XmNhelpCallback, OutlineScrollHelp_CBstruct); argcnt++;

/*
**  The pushbutton x position is the same as the vertical scrollbar
*/
    if (svnw->svn.use_scroll_buttons)
       {
        /*
        **  Local data declarations
        */
            XtTranslations top_parsed = XtParseTranslationTable (TopButtonTranslation);
            XtTranslations bot_parsed = XtParseTranslationTable (BotButtonTranslation);


        /*
        **  Add our actions table in...
        */
            XtAppAddActions( XtWidgetToApplicationContext((Widget)svnw),
			     ButtonActionsList,
			     XtNumber(ButtonActionsList) );


        /*
        **  Create the pixmaps...
        */
	    svnw->svn.top_pixmap = XCreatePixmapFromBitmapData (
		XtDisplay(svnw),			/* (IN) display */
		XDefaultRootWindow(XtDisplay(svnw)),	/* (IN) drawable */
		(char *) up_icon_bits,			/* (IN) bitmap data */
		up_icon_width,				/* (IN) pixmap width */
		up_icon_height,				/* (IN) pixmap height */
		svnw->svn.foreground_pixel,		/* (IN) foreground pixel */
		svnw->core.background_pixel,		/* (IN) background pixel */
		DefaultDepthOfScreen(XtScreen(svnw)));	/* (IN) pixmap depth */

	    svnw->svn.bot_pixmap = XCreatePixmapFromBitmapData (
		XtDisplay(svnw),			/* (IN) display */
		XDefaultRootWindow(XtDisplay(svnw)),	/* (IN) drawable */
		(char *) down_icon_bits,		/* (IN) bitmap data */
		down_icon_width,			/* (IN) pixmap width */
		down_icon_height,			/* (IN) pixmap height */
		svnw->svn.foreground_pixel,		/* (IN) foreground pixel */
		svnw->core.background_pixel,		/* (IN) background pixel */
		DefaultDepthOfScreen(XtScreen(svnw)));	/* (IN) pixmap depth */

        /*
        **  Create the top push button
        */
            XtSetArg (arglist[argcnt], XmNlabelPixmap, svnw->svn.top_pixmap);
            XtSetArg (arglist[argcnt+1], XmNtranslations, top_parsed);
	    XtSetArg (arglist[argcnt+2], XmNtraversalOn, FALSE); 

            svnw->svn.top_button = (Widget) XmCreatePushButton (svnw, "T", arglist, argcnt+3);
            XtManageChild (svnw->svn.top_button); 
        

        /*
        **  Create the bottom button
        */
            XtSetArg (arglist[argcnt], XmNlabelPixmap, svnw->svn.bot_pixmap);
            XtSetArg (arglist[argcnt+1], XmNtranslations, bot_parsed);
	    XtSetArg (arglist[argcnt+2], XmNtraversalOn, FALSE); 

            svnw->svn.bot_button = (Widget) XmCreatePushButton (svnw, "B", arglist, argcnt+3);
            XtManageChild (svnw->svn.bot_button); 
      }


/*
**  Set the fields to zero if we were not using the buttons.
*/
    if (!svnw->svn.use_scroll_buttons)
       {
        svnw->svn.top_pixmap = (Pixmap) NULL;
        svnw->svn.bot_pixmap = (Pixmap) NULL;
        svnw->svn.top_button = (Widget) NULL;
        svnw->svn.bot_button = (Widget) NULL;
      }


/*
**  Create the nav button pixmap
**  There are two different pixmaps, we use original one for LtoR
**  and smaller one (9x9) for RtoL. Angela take care to check that 9x9
**  pixmap are o.k for LtoR and if so, remove the use of 17x17 pixmaps here.
*/
    svnw->svn.tree_nav_open_pixmap = XCreatePixmapFromBitmapData (
	XtDisplay(svnw),			/* (IN) display */
	XDefaultRootWindow(XtDisplay(svnw)),	/* (IN) drawable */
	(char *) nav_open_bits,			/* (IN) bitmap data */
	nav_open_width,				/* (IN) pixmap width */
	nav_open_height,			/* (IN) pixmap height */
	svnw->svn.foreground_pixel,		/* (IN) foreground pixel */
	svnw->core.background_pixel,		/* (IN) background pixel */
	DefaultDepthOfScreen(XtScreen(svnw)));	/* (IN) pixmap depth */

    svnw->svn.tree_highlight_pixmap = XCreatePixmapFromBitmapData (
	XtDisplay(svnw),			/* (IN) display */
	XDefaultRootWindow(XtDisplay(svnw)),	/* (IN) drawable */
	(char *) nav_open_bits,			/* (IN) bitmap data */
	nav_open_width,				/* (IN) pixmap width */
	nav_open_height,			/* (IN) pixmap height */
	svnw->core.background_pixel,		/* (IN) background pixel */
	svnw->svn.foreground_pixel,		/* (IN) foreground pixel */
	DefaultDepthOfScreen(XtScreen(svnw)));	/* (IN) pixmap depth */

    svnw->svn.tree_nav_close_pixmap = XCreatePixmapFromBitmapData (
	XtDisplay(svnw),			/* (IN) display */
	XDefaultRootWindow(XtDisplay(svnw)),	/* (IN) drawable */
	(char *) nav_close_bits,		/* (IN) bitmap data */
	nav_close_width,			/* (IN) pixmap width */
	nav_close_height,			/* (IN) pixmap height */
	svnw->svn.foreground_pixel,		/* (IN) foreground pixel */
	svnw->core.background_pixel,		/* (IN) background pixel */
	DefaultDepthOfScreen(XtScreen(svnw)));	/* (IN) pixmap depth */

    svnw->svn.outline_nav_pixmap = XCreatePixmapFromBitmapData (
	XtDisplay(svnw),			/* (IN) display */
	XDefaultRootWindow(XtDisplay(svnw)),	/* (IN) drawable */
	(char *) no_nav_bits,			/* (IN) bitmap data */
	no_nav_width,				/* (IN) pixmap width */
	no_nav_height,				/* (IN) pixmap height */
	svnw->svn.foreground_pixel,		/* (IN) foreground pixel */
	svnw->core.background_pixel,		/* (IN) background pixel */
	DefaultDepthOfScreen(XtScreen(svnw)));	/* (IN) pixmap depth */


/*
**  Create corner button to control the nav window
*/
    XtSetArg (arglist[argcnt], XmNactivateCallback, DisplayNavButton_CBstruct);
    XtSetArg (arglist[argcnt+1], XmNarmPixmap , svnw->svn.tree_highlight_pixmap);

    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	 XtSetArg (arglist[argcnt+2], XmNlabelPixmap, svnw->svn.tree_nav_open_pixmap);
    else XtSetArg (arglist[argcnt+2], XmNlabelPixmap, svnw->svn.outline_nav_pixmap);


/*
** to put 9x9 bitmap into 17x17 button we must remove margins
*/
    if (LayoutIsRtoL(svnw)) {
        XtSetArg (arglist[argcnt+3], XmNmarginWidth, 0);
        XtSetArg (arglist[argcnt+4], XmNmarginHeight, 0);
    }
    svnw->svn.nav_button = (Widget) XmCreatePushButton (svnw, "NavPb", arglist,
								argcnt+5);

    if (svnw->svn.display_mode != DXmSvnKdisplayTree)
	XtSetSensitive(svnw->svn.nav_button,FALSE);

    XtManageChild (svnw->svn.nav_button); 

}

/*
**  Routine:  DisplaySetApplDragging
**
**  This routine is called in response to the application calling the routine
**  SvnSetApplDragging.  The display module must turn off or on any 
**  subwidgets.
*/

#ifdef _NO_PROTO
void DisplaySetApplDragging (svnw, value)

  svn_widget svnw;
  int value;
#else
void DisplaySetApplDragging (svn_widget svnw, int value)
#endif

{
/*
**  Dispatch to the proper routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineSetApplDragging (svnw, value);  break;
        case DXmSvnKdisplayOutline: OutlineSetApplDragging (svnw, value);  break;
        case DXmSvnKdisplayTree:    TopTreeSetApplDragging (svnw, value);  break;
      };
}

/*
**  Routine:  DisplayChangeMode
**
**  This routine is called when the display_mode resource is changed.  It allows
**  the new and old display routines to do any set-up or clean-up necessary when
**  the display mode is changed.
**
**  The return value is whether or not to redisplay.
*/

#ifdef _NO_PROTO
int DisplayChangeMode (oldsvnw, svnw)

  svn_widget oldsvnw, svnw;
#else
int DisplayChangeMode (svn_widget oldsvnw, svn_widget svnw)
#endif

{
/*
**  Local data
*/
   int i;
   
   DXmSvnEntryPtr svnentry;
   Widget secondary_scroll_window;
   Dimension secondary_width;


/*
**  Always cause each entry to be adjusted so that it will look correct.
*/
    for (i = 1; i <= svnw->svn.num_entries; i++) {
	svnentry = StructGetEntryPtr (svnw, i);
      	svnentry->height_adjusted = FALSE;
	};


/*
**  Set the max width field to zero so that the horizontal scroll bars will 
**  be accurate.
*/
    svnw->svn.max_width = 0;


/*
**  If one of the two modes is tree, then set other things...
*/
    if ((oldsvnw->svn.display_mode == DXmSvnKdisplayTree) || (svnw->svn.display_mode == DXmSvnKdisplayTree))
       {
         svnw->svn.display_changed = TRUE;
         svnw->svn.display_count = 0;
         CLEAR_SCREEN(svnw);
       };


/*
**  The right hand side scroll window is the parent of
**  the right hand side window widget.
*/
    secondary_scroll_window = XtParent (svnw->svn.secondary_form);


/*
**  Call different procedures based on the old mode.
*/
    switch (oldsvnw->svn.display_mode)

      {
        case DXmSvnKdisplayTree	    : TopTreeChangeMode (svnw);
				      break;

        case DXmSvnKdisplayOutline : OutlineChangeMode (svnw);  
				     break;

        case DXmSvnKdisplayColumns : OutlineChangeMode (svnw);
	                             XtUnmanageChild (secondary_scroll_window);
				     if (XtIsManaged(svnw->svn.secondary_ptr_widget))
					OutlineUnmanageSecPTR(svnw);
				     break;
      };


/*
**  Now for the entering side...
*/
    switch (svnw->svn.display_mode)

      {
        case DXmSvnKdisplayTree    : TopTreeChangeMode (svnw);
				     break;

        case DXmSvnKdisplayOutline : OutlineChangeMode (svnw);  
				     break;

        case DXmSvnKdisplayColumns
				    : OutlineChangeMode (svnw);
		                     XtManageChild (svnw->svn.secondary_form);
				     XtManageChild (secondary_scroll_window);
				     if (XtIsManaged(svnw->svn.primary_ptr_widget))
					OutlineManageSecPTR(svnw);
				     secondary_width = XtWidth(svnw) - button_height - 3;
	  	                     secondary_width = secondary_width - mullion_width;
		                     secondary_width = ((100 - svnw->svn.primary_percentage) * secondary_width) / 100;

				/******************************************************************************/
				/*                                                                            */
				/* If the with of the secondary comes out to be 0...(meaning that 100 percent was   */
				/* selected for primary_percentage... then make sure that the window is at least  */
				/* created with the width of 1.						      */
				/*                                                                            */
				/******************************************************************************/
				if (secondary_width == 0)
				    secondary_width = 1;

	                         PaneSetMinMax (secondary_scroll_window, secondary_width, secondary_width);
                                 PaneSetMinMax (secondary_scroll_window, 0, 0);
				 break;
      };


/*
**  Return false meaning not redisplay.  
*/
    return FALSE;
}

/*
**  This routine will redisplay an entry in the SVN widget window.  The 
**  components drawn into the window will depend upon whether the secondary window
**  is mapped or not.
*/

#ifdef _NO_PROTO
void DisplayDrawEntry (svnw, widget, svnentry, basex, basey)

  svn_widget svnw;		/* Svn Widget */
  Widget widget;		/* widget containing the drawable to recieve output */
  DXmSvnEntryPtr svnentry;	/* Entry to draw */
  int basex, basey;		/* x,y location to draw entry */
#else
void DisplayDrawEntry (svn_widget svnw, Widget widget, DXmSvnEntryPtr svnentry, 
                       int basex, int basey)
#endif

{
/*
**  Local data declarations
*/
    DXmSvnCompPtr comp;
    XFontStruct *compfont;
    XTextItem item[5];
    GC gc;
    int compx, compy, comp_number, max_comp_number, itemcount;


/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  Default the GC to either grayed or normal
*/
    if ((svnentry->grayed) || (XtIsSensitive(svnw) == FALSE))
         gc = svnw->svn.grey_gc;
    else gc = svnw->svn.foreground_gc;


/*
**  Change the GC if it is selected and we own the global selections.
*/
    if ((svnentry->selected) && (svnw->svn.show_selections))
       gc = svnw->svn.background_gc;


/*
**  Draw each component of the entry.  The x and y coordinates are computed
**  by this routine.  The max comp number is set depending on whether or not
**  there is a left hand side and what happens to right hand side components
**  if there is no right.
*/
    comp_number = 1;
    max_comp_number = svnentry->num_components;

    if ((svnw->svn.display_mode == DXmSvnKdisplayColumns) || svnw->svn.secondary_components_unmapped)
       if (svnw->svn.start_column_component != 0)
           max_comp_number = svnw->svn.start_column_component - 1;


/*
**  If max_comp_number is less than the real number, then reduce the count
*/
    if (max_comp_number > svnentry->num_components)
        max_comp_number = svnentry->num_components;


/*
**  Loop through the components
*/
    while (comp_number <= max_comp_number)

        {
          /*
          **  Get the component
          */
	  comp = &svnentry->entrycompPtr[comp_number-1];


	  /*
	  **  If this component is hidden, don't draw it
	  */
	  if ((comp->hidden_mode != svnw->svn.display_mode) && 
              (comp->hidden_mode != DXmSvnKdisplayAllModes))
	      {
	      /*
	      **  Determine the component real x and real y.
	      **  If RtoL, reverse X-coordinate.
	      */

                  if (LayoutIsRtoL(svnw))
                      compx = XtWidth (svnw->svn.primary_window_widget) - basex - comp->x - comp->width;
                  else
                      compx = basex + comp->x;
		  compy = basey + comp->y;


	      /*
	      **  Clear the bounding box of the component if tree is user
	      **  defined.
	      */
		  if ((svnw->svn.display_mode == DXmSvnKdisplayTree) &&
		      (svnw->svn.tree_style == DXmSvnKuserDefinedTree) &&
		      (!svnentry->selected))
		    {
		      XFillRectangle (XtDisplay(svnw), XtWindow(widget), 
			    svnw->svn.background_gc, 
			    (Position)compx - 1,
			    (Position)compy - 1, 
			    (Dimension)comp->width + 2, 
			    (Dimension)comp->height + 2);
		    }

		  switch ((int) comp->type)
		  {
		     case DXmSvnKcompPixmap:
			/*
			**  we are drawing a pixmap...
			*/
			XCopyArea (XtDisplay(svnw),comp->var.is_pixmap.pixmap,XtWindow(widget), 
				   svnw->svn.foreground_gc,0,0,comp->width,comp->height,compx,compy);
			break;

		     case DXmSvnKcompWidget:
			/*
			**  we are drawing a subwidget, map it into place
			*/
			SHOW_WIDGET (svnw, comp->var.is_widget.readwrite_text, widget, compx, compy);
			break;
		     case DXmSvnKcompText:
			/*
			** we are drawing a compound String...
			*/
			XmStringDraw (XtDisplay(svnw), XtWindow(widget), 
				      DisplayFontList (svnw, svnentry, comp), 
				      comp->var.is_text.text, gc, compx, compy,
				      0, 0, 0, NULL);
			break;
		  } 

	      }

          /*
          **  Increment to the next component
          */
          comp_number++;
        };
}

/*
**  This routine will redisplay the secondary window.  The components drawn into 
**  the window will depend upon whether the secondary window is mapped or not.
**
**  The X coordinates of the components are computed dynamically which will
**  prevent readjusting every entry when the maximum column width changes.
**
**  This routine now will clip components if they do not fit...
*/

#ifdef _NO_PROTO
void DisplayDrawColumns (svnw, svnentry, basex, basey, invert_option, wind)

  svn_widget svnw;	
  DXmSvnEntryPtr svnentry;	
  int basex, basey, invert_option;
  Widget wind;		    /* Which window to draw components to */
#else
void DisplayDrawColumns (svn_widget svnw, DXmSvnEntryPtr svnentry, int basex, 
                        int basey, int invert_option, Widget wind)
#endif

{
/*
**  Local data declarations
*/
    DXmSvnCompPtr comp;
    XFontStruct *compfont;
    XTextItem item[5];
    GC gc;
    int compx, compy, comp_number, max_comp_number, itemcount, column_x, line_x;
    int draw_loc_cursor = FALSE, draw_lines = FALSE;

/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  If there are really no secondary components, then return
*/
    if (svnw->svn.start_column_component == 0) return;


/*
**  Draw each component of the entry.  The x and y coordinates are computed
**  by this routine.  The max comp number is set depending on whether or not
**  there is a left hand side and what happens to right hand side components
**  if there is no right.
*/
    comp_number = svnw->svn.start_column_component;
    max_comp_number = svnentry->num_components;


/*
**  The column_x coordinate starts at 1/2 of the default spacing
*/
    column_x = svnw->svn.default_spacing / 2;

/*
** Set a flag to state whether this entry should have the location cursor.
*/
    if (svnw->svn.location_cursor_entry == svnentry)
	{
	draw_loc_cursor = TRUE;
	if (comp_number > max_comp_number)
	    draw_lines = TRUE;
	}	
    
/*
**  Loop through the components
*/
    while (comp_number <= max_comp_number)

        {
          /*
          **  Get the component
          */
	  comp = &svnentry->entrycompPtr[comp_number-1];


	  /*
	  **  If this component is hidden, don't draw it
	  */
	  if ((comp->hidden_mode != svnw->svn.display_mode) && 
              (comp->hidden_mode != DXmSvnKdisplayAllModes))
	      {
	      /*
	      **  Local boolean indicating that column clip used.
	      */
	          Boolean column_clipped = FALSE;
                  XRectangle clip;


	      /*
	      **  Determine the component real x and real y.
	      **  If RtoL, reverse X-coordinate.
	      */
		  compx = basex + column_x;
                  if (LayoutIsRtoL(svnw))
                      compx = XtWidth(wind) - compx
                                - svnw->svn.column_widths[comp_number-1];
		  compy = basey + comp->y;

		  line_x = compx + ( (LayoutIsRtoL(svnw)) ?
                                svnw->svn.column_widths[comp_number-1] + (svnw->svn.default_spacing / 2)
                                : - (svnw->svn.default_spacing / 2));


	      /*
	      **  Default the GC to either grayed or normal
	      */
		  if ((svnentry->grayed) || (XtIsSensitive(svnw) == FALSE))
		       gc = svnw->svn.grey_gc;
		  else gc = svnw->svn.foreground_gc;

			
	      /*
	      **  Change the GC if it is inverted
	      */
		  if ((invert_option == comp_number) || (invert_option == 99))
  	             gc = svnw->svn.background_gc;


	      /*
	      **  Draw lines between the components if called for.  We do nothing
	      **  on the first component, but for the rest we will draw the line 
	      **  at the X coordinate minus 1/2 of the default spacing value.
	      */
	          if (svnw->svn.column_lines)
	             if (comp_number > svnw->svn.start_column_component)
			{
                        XDrawLine (XtDisplay(svnw),
	                           XtWindow(wind),
	                           svnw->svn.foreground_gc, 
				   line_x,
	                           0,
				   line_x,
		                   XtHeight(wind));
			}
 
	      /*
	      **  If the component width is greater than the column width
	      **  the set the clip and record the fact to remove it.
	      */
	          if (comp->width > svnw->svn.column_widths[comp_number-1])
		     {
		       column_clipped = TRUE;

		       clip.x      = compx;
		       clip.y      = compy;
		       clip.width  = svnw->svn.column_widths[comp_number-1];
		       clip.height = comp->height;  

		       XSetClipRectangles (XtDisplay(svnw), gc, 0, 0, &clip, 1, Unsorted);
		     };


	      /*
	      **  If we are drawing a pixmap...
	      */
		  if (comp->type == DXmSvnKcompPixmap)
		     XCopyArea (XtDisplay(svnw),comp->var.is_pixmap.pixmap,
				XtWindow(wind),svnw->svn.foreground_gc,
				0,0,comp->width,comp->height,compx,compy);


	      /*
	      **  If we are drawing a subwidget, then map it into place
	      */
		  if (comp->type == DXmSvnKcompWidget)
		     SHOW_WIDGET (svnw, comp->var.is_widget.readwrite_text, wind, compx, compy);

	      /*
	      ** If we are drawing a compound String...
	      */
  		  if (comp->type == DXmSvnKcompText) {
		     XmStringDraw (XtDisplay(svnw), XtWindow (wind),
			DisplayFontList (svnw, svnentry, comp), 
			comp->var.is_text.text, gc, compx, compy,
			0, 0, 0, NULL);
		     } 

	     
	      /*
	      **  Reset the clip if necessary.
	      */
	          if (column_clipped)
			XSetClipMask (XtDisplay(svnw), gc, None);
	      };


          /*
          **  Increment to the next component and the next column
          */
	  column_x = column_x + svnw->svn.column_widths[comp_number-1] + svnw->svn.default_spacing;
          comp_number++;
        };

     /*
     **  Finally draw the location cursor around the entire secondary window. 
     */
	if (draw_loc_cursor)
	   {
	    int width;

	    if (svnw->svn.fixed_width)
		width = XtWidth(wind) - basex;
	    else 
		width = svnw->svn.location_cursor_entry->width -
			XtWidth(svnw->svn.primary_window_widget);
		
		XDrawRectangle (XtDisplay(svnw),
				XtWindow(wind),
				svnw->svn.location_cursor_gc,
				(Position)( (LayoutIsRtoL(svnw)) ? XtWidth(wind)
                                    - basex - width - 1 : basex),
				(Position)basey,
				(Dimension)width,
				(Dimension)svnentry->height-1);
	      }
    

    /******************************************************************************/
    /*                                                                            */
    /* If this loop has never been entered, that means there are NO right	*/
    /* hand side componenets, so we have to make sure that the column lines    */
    /* are redrawn anyway.						        */
    /*                                                                            */
    /******************************************************************************/
    if (draw_lines)
	LclOutlineDrawColumnLines(svnw);
	
}

/*
**  This routine will draw the inverted boxes for the secondary window.
*/

#ifdef _NO_PROTO
void DisplayDrawColumnsBox (svnw, svnentry, basex, basey, invert_option, wind)

  svn_widget svnw;	
  DXmSvnEntryPtr svnentry;	
  int basex, basey, invert_option;
  Widget wind;
#else
void DisplayDrawColumnsBox (svn_widget svnw, DXmSvnEntryPtr svnentry, 
			int basex, int basey, int invert_option, Widget wind)
#endif

{
/*
**  Local data declarations
*/
    DXmSvnCompPtr comp;
    GC gc;
    int compx, comp_number, max_comp_number, column_x = 0;
    Dimension box_width;


/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  If there are really no secondary components, then return
*/
    if (svnw->svn.start_column_component == 0) return;


/*
**  If nothing needs boxes, then leave now.
*/
    if (invert_option == 0) return;


/*
**  Default the GC to either grayed or normal
*/
    if ((svnentry->grayed) || (XtIsSensitive(svnw) == FALSE))
         gc = svnw->svn.grey_gc;
    else gc = svnw->svn.foreground_gc;


/*
**  Draw each component of the entry.  The x and y coordinates are computed
**  by this routine.  The max comp number is set depending on whether or not
**  there is a left hand side and what happens to right hand side components
**  if there is no right.
*/
    comp_number = svnw->svn.start_column_component;
    max_comp_number = svnentry->num_components;


/*
**  Loop through the components
*/
    while (comp_number <= max_comp_number)

        {
          /*
          **  Get the component
          */
	  comp = &svnentry->entrycompPtr[comp_number-1];


	  /*
	  **  If this component is hidden, don't draw it
	  */
	  if ((comp->hidden_mode != svnw->svn.display_mode) && 
              (comp->hidden_mode != DXmSvnKdisplayAllModes))
	      {
	      /*
	      **  Determine the component real x value
	      */
		  compx = basex + column_x;


            /******************************************************************************/
            /*                                                                            */
            /* Determine the width of the box to highlight... If the selection mode is    */
	    /* then entire entry.. then just set the width == side of secondary window.. else   */
	    /* just highlight around the component itself.				  */
            /*                                                                            */
            /******************************************************************************/
	    if ((svnw->svn.selection_mode == DXmSvnKselectEntry) ||
		((svnw->svn.selection_mode == DXmSvnKselectEntryOrComp) && 
			svnentry->selected_comp < svnw->svn.start_column_component))
		{
		box_width = XtWidth (wind) - compx; 
		comp_number = max_comp_number;
		}
	    else
		box_width = svnw->svn.column_widths[comp_number-1] + svnw->svn.default_spacing;
		
	      /*
	      **  Invert the component if necessary
	      */
		  if ((invert_option == comp_number) || (invert_option == 99))
		     {
                          if (LayoutIsRtoL(svnw))
                              compx = XtWidth(wind) - compx - box_width;
		      /*
		      **  Draw the rectangle
		      */
			  XFillRectangle (XtDisplay(svnw), XtWindow(wind),
					  gc, (Position)compx, (Position)basey, 
					  box_width, (Dimension)svnentry->height);
		     };
	      };


          /*
          **  Increment to the next component and next column
          */
	  column_x = column_x + svnw->svn.column_widths[comp_number-1] + svnw->svn.default_spacing;
          comp_number++;
        };
}

/*
**  Routine that resolves graphic exposure events on scroll operations.  This
**  routine assumes that all entries are valid and they just need repaired to
**  solve graphic exposure problems.  This routine returns when all graphic
**  exposures are resolved.
**
**  It is assumed that the caller has deposited a window identifier into the 
**  clips_window field of the widget.
*/

#ifdef _NO_PROTO
void DisplayGraphicsExpose (svnw)

  svn_widget svnw;
#else
void DisplayGraphicsExpose (svn_widget svnw)
#endif

{
/*
**  Routine data declarations
*/
    XEvent new_event;


/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  Continue looping until we do not get a new exposure event.  We will be
**  stacking up the rectangles and marking the invalid bits in the entries
**  that need to be redisplayed.
*/
    while (XCheckTypedWindowEvent(XtDisplay(svnw), svnw->svn.clips_window, GraphicsExpose, &new_event))

       {
         /*
         **  Add the clipping rectangle to our array of clipping rectangles
         */
	 if (svnw->svn.clip_count < max_clips) 
	    {
             svnw->svn.clips[svnw->svn.clip_count].x      = new_event.xgraphicsexpose.x;
             svnw->svn.clips[svnw->svn.clip_count].y      = new_event.xgraphicsexpose.y;
             svnw->svn.clips[svnw->svn.clip_count].width  = new_event.xgraphicsexpose.width;
             svnw->svn.clips[svnw->svn.clip_count].height = new_event.xgraphicsexpose.height;
             svnw->svn.clip_count++;
	    }
	  else
	    {
	     svnw->svn.clip_count = 0;
	     svnw->svn.refresh_all = TRUE;
	    }
       };
}

/*
**  Routine that resolves graphic exposure events on scroll operations.  This
**  routine assumes that all entries are valid and they just need repaired to
**  solve graphic exposure problems.  This routine returns when all graphic
**  exposures are resolved.
*/

#ifdef _NO_PROTO
void DisplayExpose (svnw, delta_x, delta_y)

  svn_widget svnw;
  int delta_x, delta_y;
#else
void DisplayExpose (svn_widget svnw, int delta_x, int delta_y)
#endif

{
/*
**  Routine data declarations
*/
    XEvent new_event;


/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  Continue looping until we do not get a new exposure event.  We will be
**  stacking up the rectangles and marking the invalid bits in the entries
**  that need to be redisplayed.
*/
    while (XCheckTypedWindowEvent(XtDisplay(svnw), svnw->svn.clips_window, Expose, &new_event))

       {
         /*
         **  Add the clipping rectangle to our array of clipping rectangles
         */
	 if (svnw->svn.clip_count < max_clips) 
	    {
             svnw->svn.clips[svnw->svn.clip_count].x      = new_event.xexpose.x
								+ ( (LayoutIsRtoL(svnw)) ? - delta_x : delta_x);
             svnw->svn.clips[svnw->svn.clip_count].y      = new_event.xexpose.y + delta_y;
             svnw->svn.clips[svnw->svn.clip_count].width  = new_event.xexpose.width;
             svnw->svn.clips[svnw->svn.clip_count].height = new_event.xexpose.height;
             svnw->svn.clip_count++;
	    }
	  else
	    {
	     svnw->svn.clip_count = 0;
	     svnw->svn.refresh_all = TRUE;
	    }
       };
}

/*
**  This routine is called by DisplayAdjustHeight whenever a line of components
**  is desired. 
**
**  This routine calculates the proper width and height of an entry based on the
**  current components.  If the application supplies a Y value for a component
**  that value is used.
**
**  This routine ensures that components within a given line do not overlap 
**  other components within that line.  If they are found to overlap, then the
**  component is pushed out enough to leave a 'default_spacing' gap.  The effect
**  will be that it is a ripple effect.  The second component may move while
**  the third may not because moving the second did not force it into the third.
*/


#ifdef _NO_PROTO
static void LclDisplayLineData (svnw, svnentry, comps, first_comp, last_comp, y_plus)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  DXmSvnCompPtr comps [max_comps];
  int first_comp, last_comp;
  Dimension *y_plus;
#else
static void LclDisplayLineData (svn_widget svnw, DXmSvnEntryPtr svnentry,
                                DXmSvnCompPtr comps [max_comps], int first_comp, 
                                int last_comp, Dimension *y_plus)
#endif

{
/*
**  Local data declarations
*/
    Dimension below, max_below = 0, max_above = 0, max_height = 0;
    Boolean do_columns;
    int i;
    

/*
**  Set the boolean do_columns based on the display mode and whether this is
**  the first line in an entry.
*/
    do_columns = (   (first_comp == 1)
                  && (svnw->svn.display_mode == DXmSvnKdisplayColumns)
                  && (svnw->svn.start_column_component != 0));


/*
**  Compute the baseline y position and maximum descent values for each of the
**  components.
*/
    for (i = first_comp;  i <= last_comp;  i++)
      {
	DisplayCompData(svnw, svnentry, comps[i], &below);
	if (max_height < comps[i]->height) max_height = comps[i]->height;
      };


/*
**  If this is the first line of an entry and we are in columns mode, 
**  then use the columns in the max_above and max_below calculations.
*/
    if (do_columns)
       for (i = svnw->svn.start_column_component;  i <= svnentry->num_components;  i++)
           {
	     DisplayCompData(svnw, svnentry, comps[i], &below);
	     if (max_height < comps[i]->height) max_height = comps[i]->height;
           };


/*
**  Now we have the maximum height of things that are to be centered
**  along with the maximum ascent/descent of text strings that are to
**  be lined up.  If the ascent/descents are more than the max_height
**  then make that the max_height.
*/
    if (max_height < max_above + max_below)
        max_height = max_above + max_below;


/*
**  If max_height is greater than max_above plus the max_below, then
**  adjust the max_above value to center text within the pixmap.
*/
    if (max_height > max_above + max_below)
       max_above = max_above + ((max_height - (max_above + max_below)) / 2);


/*
**  If application did not supply a Y value, align each y value
**  according to their ascent values.  Do this for columns also
**  if it is appropriate.
*/
    for (i = first_comp;  i <= last_comp;  i++)
        if (comps[i]->y == 0)
	    comps[i]->y = ((max_height - comps[i]->height) / 2) + *y_plus;

    if (do_columns)
       for (i = svnw->svn.start_column_component;  i <= svnentry->num_components;  i++)
           if (comps[i]->y == 0)
	      comps[i]->y = ((max_height - comps[i]->height) / 2) + *y_plus;


/*
**  Do the overlap detection pass comparing component X values and widths.  Note
**  that this is only necessary when there is more than one component.  Note also
**  that is two components are up against each other, but not overlapping that
**  this will leave it alone.
*/
    for (i = first_comp + 1;  i <= last_comp;  i++)
        if (comps[i]->x < comps[i-1]->x + comps[i-1]->width)
            comps[i]->x = comps[i-1]->x + comps[i-1]->width + svnw->svn.default_spacing;


/*
**  Pass through the components setting the column width field on the widget
**  if it is zero.
*/
    if (do_columns)
       {
         Boolean redisplay = FALSE;
         for (i = svnw->svn.start_column_component;  i <= svnentry->num_components;  i++)
	     {
  	       /*
	       **  If there is none at all, then set it to this width
	       */
                   if (svnw->svn.column_widths [i-1] == 0)
	              { 
                        svnw->svn.column_widths [i-1] = comps[i]->width;
                        svnw->svn.secondary_max_width = svnw->svn.secondary_max_width + svnw->svn.default_spacing + comps[i]->width;
	   	      };


               /*
	       **  If the old one is too small and it was calculated by SVN,
	       **  then do the computations and set a flag to redisplay all entries.
	       */
		   if (svnw->svn.column_width_set [i-1] == FALSE)
	              if (svnw->svn.column_widths [i-1] < comps[i]->width)
	  	         {
		           svnw->svn.secondary_max_width = svnw->svn.secondary_max_width - svnw->svn.column_widths [i-1];
                           svnw->svn.column_widths [i-1] = comps[i]->width;
		           svnw->svn.secondary_max_width = svnw->svn.secondary_max_width + svnw->svn.column_widths [i-1];
		           redisplay = TRUE;
		         };
             };

         if (redisplay)
	    {
	    /* Clear out PTR window first */
	    
	    if (XtIsManaged(svnw->svn.secondary_ptr_widget))
		XClearArea (XtDisplay(svnw), XtWindow(svnw->svn.secondary_ptr_widget), 0, 0, 
	            XtWidth(svnw->svn.secondary_ptr_widget),XtHeight(svnw->svn.secondary_ptr_widget), TRUE);

	    /* Now clear out the main entries secondary window */
	    
	    XClearArea (XtDisplay(svnw), XtWindow(svnw->svn.secondary_window_widget), 0, 0, 
	            XtWidth(svnw->svn.secondary_window_widget),XtHeight(svnw->svn.secondary_window_widget), TRUE);
	    }
       };


/*
**  Use the last component for width calculations
*/
    if (svnentry->width < comps[last_comp]->width + comps[last_comp]->x + 1) 
        svnentry->width = comps[last_comp]->width + comps[last_comp]->x + 1;


/*
**  Set the y_plus to include current line height.
*/
    *y_plus = *y_plus + max_height;
}

/*  
**  This routine is called by the STRUCTURE module whenever a valid entry
**  is desired.   It is only called if 'height_adjusted' is false.
*/

#ifdef _NO_PROTO
void DisplayAdjustEntryHeight (svnw, svnentry)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
#else
void DisplayAdjustEntryHeight (svn_widget svnw, DXmSvnEntryPtr svnentry)
#endif

{	
/*
**  Local data declarations
*/
    int i, first_comp = 1, last_comp_x = -1, num_components;
    Dimension y_plus = 0;
    DXmSvnCompPtr comps [max_comps];
    

/*
**  Clear the entry width and height
*/
    svnentry->width  = 0;
    svnentry->height = 0;


/*
**  Initialize all components of the entry to their original dimensions.
*/
    for (i = 1;  i <= svnentry->num_components;  i++)
	{
	 /*
	 **  Get the address of the widget components locally
	 */
             comps[i] = &svnentry->entrycompPtr[i-1];


	 /*
	 **  Fill in working fields from original values
	 */
	     comps[i]->x      = comps[i]->orig_x;
	     comps[i]->y      = comps[i]->orig_y;
	     comps[i]->width  = comps[i]->orig_width;
	     comps[i]->height = comps[i]->orig_height;
	};


/*
**  Set the number of components locally.  This is for the current number of
**  components in the left hand side.  The lower level line level procedure
**  will handle the left hand side components when adjusting the first line.
*/
    if (
	((svnw->svn.display_mode == DXmSvnKdisplayColumns)	|| svnw->svn.secondary_components_unmapped)  &&
	(svnw->svn.start_column_component != 0) 
	)
        num_components = svnw->svn.start_column_component - 1;
    else num_components = svnentry->num_components;


/*
**  If there are really no secondary components, then restore the old value
*/
    if (num_components > svnentry->num_components)
       num_components = svnentry->num_components;


/*
**  Process the first lines of an entry.  The last line (and in most cases the
**  only line) will not be processed in this loop.
*/
    for (i = 1;  i <= num_components;  i++)
	{  	
	  /*
	  **  If hidden in this display_mode, then set it's x,y position
	  **  to zero to keep it from contributing to the entry width.
	  **  Copy the X position from the previous entry so that overlapping
	  **  will still be eliminated.
	  */
	  if ((comps[i]->hidden_mode == svnw->svn.display_mode) || (comps[i]->hidden_mode == DXmSvnKdisplayAllModes))
	    {
	      if (i == 1) comps[i]->x = 0;
	      else comps[i]->x = comps[i-1]->x;

	      comps[i]->y = 0;
	    }


	  /*
	  **  Process all components seen thus far as a group if the current
	  **  component is not hidden in this mode.
	  */
	  if ((comps[i]->x <= last_comp_x) &&
	      (comps[i]->hidden_mode != svnw->svn.display_mode) &&
	      (comps[i]->hidden_mode != DXmSvnKdisplayAllModes))

	     {
	       LclDisplayLineData (svnw, svnentry, comps, first_comp, i-1, &y_plus);
	       first_comp = i;
	     };

	  last_comp_x = comps[i]->x;
	};


/*
**  Figure out the last line.  Add the height of that line into the height 
**  of the entry.  Also see if the last line is the longest line.
*/
    LclDisplayLineData (svnw, svnentry, comps, first_comp, num_components, &y_plus);


/*
**  The y_plus value contains the entry height.
*/
    svnentry->height = y_plus;


/*
**  If the user said that it was taller than that, then trust them
*/
    if (svnentry->height < svnentry->orig_height)
        svnentry->height = svnentry->orig_height;

       
/*
**  Reflect the fact that the adjustment has been made
*/
    svnentry->height_adjusted = TRUE;
}

/*
**  This routine determines the width and height (both above and below the
**  baseline) of a component.  The above value is put into the component
**  height.  The width is put into the component width.
*/

#ifdef _NO_PROTO
void DisplayCompData (svnw, svnentry, comp, below)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  DXmSvnCompPtr comp;
  Dimension *below;
#else
void DisplayCompData (svn_widget svnw, DXmSvnEntryPtr svnentry, DXmSvnCompPtr comp, Dimension *below)
#endif

{
/*
**  Local data declarations
*/
    XmFontList  complist;
    XFontStruct *compfont;

/*
**  If display_mode is tree then make invisible components width and height 0
*/
    if ((comp->hidden_mode == svnw->svn.display_mode) ||
        (comp->hidden_mode == DXmSvnKdisplayAllModes)) {
	comp->height = 0;
	comp->width  = 0;
	*below = 0;
	return;
	}


/*
**  Compute the baseline y position and maximum descent values for each of the
**  components.
*/
    switch (comp->type)
	{
	case DXmSvnKcompPixmap: *below = 0;
				break;

	case DXmSvnKcompWidget: comp->height = XtHeight (comp->var.is_widget.readwrite_text);
			        comp->width = XtWidth(comp->var.is_widget.readwrite_text);
			        *below = 0;
			        break;

	case DXmSvnKcompText : *below = 0;
			       complist = DisplayFontList (svnw, svnentry, comp);
			       XmStringExtent(complist, comp->var.is_text.text,
			       &comp->width, &comp->height);
	};
}

/*
**  Routine:  DisplayNavButton
**
**  This routine is called in response to a push of the Nav button
*/

#ifdef _NO_PROTO
void DisplayNavButton (w)

  Widget w;
#else
void DisplayNavButton (Widget w)
#endif

{
/*
**  Determine the svn widget assocatiated with this button.
*/
    svn_widget svnw = StructFindSvnWidget (w);


/*
**  Dispatch to the proper routine.  Currently no support in Outline...
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayTree: TopTreeNavButton (svnw);  break;
      };

} /* DisplayNavButton */

/*
**  Routine:  DisplayAdjustEntrySize
**
**  This routine is called during geometry requests so that the display
**  routines can accomodate the new entry size.
*/

#ifdef _NO_PROTO
void DisplayAdjustEntrySize (svnw, entry_number)

  svn_widget svnw;
  int entry_number;
#else
void DisplayAdjustEntrySize (svn_widget svnw, int entry_number)
#endif

{
/*
**  Dispatch to the appropriate routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineAdjustEntrySize (svnw, entry_number);  break;
        case DXmSvnKdisplayOutline: OutlineAdjustEntrySize (svnw, entry_number);  break;
        case DXmSvnKdisplayTree:    TopTreeAdjustEntrySize (svnw, entry_number);  break;
      };
}

/*
**  Routine:  DXmSvnSetEntryPosition
**
**  This routine sets the position of an entry in user define tree mode.  If
**  the tree style is not userDefined then this routine does nothing.
*/

#ifdef _NO_PROTO
void DXmSvnSetEntryPosition (w, entry_number, window_mode, x, y)

    Widget w;
    int entry_number;
    Boolean window_mode;
    int x, y;
#else
void DXmSvnSetEntryPosition (Widget w, int entry_number, Boolean window_mode, int x, int y)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local Variables
*/
    DXmSvnEntryPtr svnentry;
    int new_x, new_y;
    int old_x, old_y;
    int width, height;


/*
**  If this is not a user defined tree just return
*/
    if (svnw->svn.display_mode != DXmSvnKdisplayTree) return;
    if (svnw->svn.tree_style != DXmSvnKuserDefinedTree) return;


/*
**  If position is in window mode, then change it to world coordinates
*/
    if (window_mode == TRUE) {
	new_x = x;
	new_y = y;
	x = X_TO_WORLD(svnw, x);
	y = Y_TO_WORLD(svnw, y);
	}
    else {
	new_x = X_TO_WINDOW(svnw, x);
	new_y = Y_TO_WINDOW(svnw, y);
	};


/*
**  Get the entry and fill in the specified position
*/
    svnentry = StructGetValidEntryPtr(svnw, entry_number);
    old_x = X_TO_WINDOW(svnw, X_POS(svnw, svnentry));
    old_y = Y_TO_WINDOW(svnw, Y_POS(svnw, svnentry));
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    svnentry->px = x;
    svnentry->py = y;    
    width = svnentry->width + 1;
    height = svnentry->height + 1;


/*
**  Set the window of interest for clip rectangles
*/
    svnw->svn.clips_window = XtWindow (svnw->svn.primary_window_widget);


/*
**  TBS -- We need a concept of keeping these clips around until we get enabled again!
**  for now clear the old area.
*/
    XClearArea (XtDisplay(svnw),
	        svnw->svn.clips_window,
                (Position)( (LayoutIsRtoL(svnw)) ?
                        XtWidth(svnw->svn.primary_window_widget) - old_x - width + 1 : old_x),
		(Position)old_y,
		(Dimension)width,
		(Dimension)height,
		FALSE);


/*
**  Set the clipping rectangles on both the source and destination
**  of this entry.
*/
    if (svnw->svn.clip_count < max_clips - 1) {
        svnw->svn.clips[svnw->svn.clip_count].x = ( (LayoutIsRtoL(svnw)) ?
                        			   XtWidth(svnw->svn.primary_window_widget) - old_x - width + 1 : old_x);
	svnw->svn.clips[svnw->svn.clip_count].y = old_y;
	svnw->svn.clips[svnw->svn.clip_count].width = width;
	svnw->svn.clips[svnw->svn.clip_count].height = height;
	svnw->svn.clip_count++;
        svnw->svn.clips[svnw->svn.clip_count].x = ( (LayoutIsRtoL(svnw)) ?
                        			   XtWidth(svnw->svn.primary_window_widget) - new_x - width + 1 : new_x);
	svnw->svn.clips[svnw->svn.clip_count].y = new_y;
	svnw->svn.clips[svnw->svn.clip_count].width = width;
	svnw->svn.clips[svnw->svn.clip_count].height = height;
	svnw->svn.clip_count++;
	}
    else {
	svnw->svn.refresh_all = TRUE;
	svnw->svn.clip_count = 1;
	};


/*
**  Force Nav window update
*/
    svnw->svn.update_nav_window = TRUE;


} /* DXmSvnSetEntryPosition */

/*
**  Routine:  DXmSvnFlushEntry
**
**  This routine does an append of this entry if appropriate.  It only applies
**  to outline mode.
*/
#ifdef _NO_PROTO
void DXmSvnFlushEntry (w, entry_number)

    Widget w;
    int entry_number;
#else
void DXmSvnFlushEntry (Widget w, int entry_number)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Dispatch to the appropriate routine
*/
    switch (svnw->svn.display_mode)
      {
        case DXmSvnKdisplayColumns: OutlineFlushEntry (svnw, entry_number);  break;
        case DXmSvnKdisplayOutline: OutlineFlushEntry (svnw, entry_number);  break;
      };
}

/*
**  Routine:  DXmSvnGetEntryPosition
**
**  This routine sets the position of an entry in user define tree mode.  If
**  the tree style is not userDefined then this routine does nothing.
*/
#ifdef _NO_PROTO
void DXmSvnGetEntryPosition (w, entry_number, window_mode, x, y)

    Widget w;
    int entry_number;
    Boolean window_mode;
    int *x, *y;
#else
void DXmSvnGetEntryPosition (Widget w, int entry_number, Boolean window_mode, int *x, int *y)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Local Variables
*/
    DXmSvnEntryPtr svnentry;


/*
**  Get the entry and extract the position info
*/
    svnentry = StructGetValidEntryPtr(svnw, entry_number);


/*
**  If Outline mode call local support routine to find the position
*/
    if (svnw->svn.display_mode != DXmSvnKdisplayTree) 
	{
	/*
	**  Determine the offset of the entry
	*/
	int offset = LclOutlineMapNumberToOffset(svnw, entry_number);


	/*
	**  Return -1 if not visible, or request was for internal coordinates
	*/
	if ((offset == 0) || (window_mode == FALSE))
	    {
	    *x = -1;
	    *y = -1;
	    return;
	    }


	/*
	**  Map that offset into a Y window coordinate, and compute the base x
	**  value based on the entries level number and on whether the
	**  navigation bar is on the left side.
	*/
	*y = LclOutlineMapOffsetToY(svnw, offset);
	*x = svnw->svn.margin_width + (svnentry->level * svnw->svn.indent_margin) - svnw->svn.window_basex;
	return;
	}



/*
**  Otherwise, Just use the macros to get position
*/
    *x = X_POS(svnw, svnentry);
    *y = Y_POS(svnw, svnentry);


/*
**  If position is desired in window mode, then change it to window coordinates
*/
    if (window_mode == TRUE) {
	*y = Y_TO_WINDOW(svnw, *x);
	*x = X_TO_WINDOW(svnw, *y);
	}


} /* DXmSvnGetEntryPosition */

/*
**  Routine:  DXmSvnGetTreePosition
**
**  This routine gets the position of the tree in tree mode.  This routine
**  is only valid when the display mode is tree.
*/
#ifdef _NO_PROTO
void DXmSvnGetTreePosition (w, x, y)

    Widget w;
    int *x, *y;
#else
void DXmSvnGetTreePosition (Widget w, int *x, int *y)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Return the map position
*/
    *x = svnw->svn.mapx;
    *y = svnw->svn.mapy;


} /* DXmSvnGetTreePosition */

/*
**  Routine:  DXmSvnSetTreePosition
**
**  This routine sets the position of the tree in tree mode.  This routine
**  is only valid when the display mode is tree.
*/
#ifdef _NO_PROTO
void DXmSvnSetTreePosition (w, x, y)

    Widget w;
    int x, y;
#else
void DXmSvnSetTreePosition (Widget w, int x, int y)
#endif

{
    svn_widget svnw = StructFindSvnWidget (w);

/*
**  Change the tree position
*/
    svnw->svn.mapx = x;
    svnw->svn.mapy = y;
    svnw->svn.refresh_all = TRUE;

} /* DXmSvnSetTreePosition */

/*
** This routine puts the clipping rectangles in the proper form.  It removes
** overlapping clips and may merge some clips together.  In the future it could
** be enhanced to sort the clips to improve performance.  It may return 0 or
** more clips.  If a clip that is the size of the drawing area is found, then
** all of the clips are deleted so the entire display is redrawn.
**
*/
#ifdef _NO_PROTO
void DisplayMergeClips(clip, clip_count)
    XRectangle	*clip;
    short	*clip_count;
#else
void DisplayMergeClips(XRectangle *clip, short *clip_count)
#endif

{
    int i, j;	/* loop index */
    int min_x, min_y, max_y, max_x;


/*
**  Loop through merging overlapping clips
*/
    for (i = 0; i < *clip_count; i++) 
	{
	/*
	**  Skip deleted clips
	*/
	if (clip[i].width == 0) continue;


	/*
	**  Compare this clip against all others looking for overlap
	*/
	for (j = 0; j < (*clip_count - 1); j++)
	    {
	    /*
	    **  If i and j are the same clip, skip it
	    */
	    if (i == j) continue;


	    /*
	    **  Skip deleted clips
	    */
	    if (clip[j].width == 0) continue;

	    
	    /*
	    **  ignore this one if they don't overlap
	    */
	    if ((clip[i].x + clip[i].width ) < (clip[j].x)) continue;
	    if ((clip[i].y + clip[i].height) < (clip[j].y)) continue;
	    if ((clip[j].x + clip[j].width ) < (clip[i].x)) continue;
	    if ((clip[j].y + clip[j].height) < (clip[i].y)) continue;


	    /*
	    **  They do overlap, so figure out the bounding box 
	    */
	    if (clip[i].y < clip[j].y)
		 min_y = clip[i].y;
	    else min_y = clip[j].y;


	    if (clip[i].x < clip[j].x) 
		 min_x = clip[i].x;
	    else min_x = clip[j].x;


	    if ((clip[i].y + clip[i].height) > (clip[j].y + clip[j].height))
		 max_y = clip[i].y + clip[i].height;
	    else max_y = clip[j].y + clip[j].height;


	    if ((clip[i].x + clip[i].width) > (clip[j].x + clip[j].width))
		 max_x = clip[i].x + clip[i].width;
	    else max_x = clip[j].x + clip[j].width;


	    /*
	    **  Store the bounding box of the two clips as the new clip
	    **  and mark the other as deleted
	    */
	    clip[j].x = min_x;
	    clip[j].y = min_y;
	    clip[j].width  = max_x - min_x;
	    clip[j].height = max_y - min_y;
	    clip[i].width  = 0;
	    }
	}


/*
**  Second pass:  Loop through the clips deleting the holes made by the first
**  pass.
*/
    j = 0;
    for (i = 0; i < *clip_count; i++) 
	{
	/*
	**  Skip deleted clips
	*/
	if (clip[i].width == 0) continue;


	/*
	**  If we need to move the clip, do so
	*/
	if (i != j) 
	    clip[j] = clip[i];


	/*
	**  Increment the number of clips
	*/
	j++;
	}


/*
**  Reset the number of clips
*/
    *clip_count = j;


} /* DisplayMergeClips */

/*
**  This routine either turns on or turns off the watch cursor.
*/

#ifdef _NO_PROTO
void DisplaySetWatch (svnw, flag)

  svn_widget svnw;
  Boolean flag;
#else
void DisplaySetWatch (svn_widget svnw, Boolean flag)
#endif

{
/*
**  If we are not yet realized, then leave.
*/
    if (!XtIsRealized (svnw)) return;


/*
**  If the flag is TRUE, then turn on the watch and flush it out.
*/
    if (flag)
         XDefineCursor   (XtDisplay(svnw), XtWindow(svnw), svnw->svn.watch_cursor);
    else XUndefineCursor (XtDisplay(svnw), XtWindow(svnw));


/*
**  Flush out the request
*/
    XFlush (XtDisplay(svnw));
}



/*
**  This common display routine determines the font to use for a particular component
*/

#ifdef _NO_PROTO
XmFontList DisplayFontList (svnw, svnentry, comp)

  svn_widget svnw;
  DXmSvnEntryPtr svnentry;
  DXmSvnCompPtr comp;
#else
XmFontList DisplayFontList (svn_widget svnw, DXmSvnEntryPtr svnentry, DXmSvnCompPtr comp)
#endif

{
/*
**  Local data declarations
*/
    XmFontList candidate = (XmFontList)NULL;


/*
**  If a font is specified in the component, then that is the one to use
*/
    if (comp->var.is_text.font_list != (XmFontList) NULL) return (comp->var.is_text.font_list);


/*
**  Determine a potential default font according to the level number
*/
    switch (svnentry->level)
       {
         case 0: candidate = svnw->svn.level0_fontlist;  break;
         case 1: candidate = svnw->svn.level1_fontlist;  break;
         case 2: candidate = svnw->svn.level2_fontlist;  break;
         case 3: candidate = svnw->svn.level3_fontlist;  break;
         case 4: candidate = svnw->svn.level4_fontlist;  break;
       };


/*
**  If the candidate font is still null, then set it to be the default
*/
    if (candidate == (XmFontList) NULL) candidate = svnw->svn.default_fontlist;


/*
**  Return the candidate
*/
    return (candidate);
}

/******************************************************************************/
/*                                                                            */
/* Display EraseLocationCursor... - this routine will erase the location      */
/*	cursor from the display...					      */
/*									      */
/******************************************************************************/
#ifdef _NO_PROTO
void DisplayEraseLocationCursor(svnw)

svn_widget svnw;
#else
void DisplayEraseLocationCursor(svn_widget svnw)
#endif

    {
    GC gc,loc_gc;
    int width,i;
    Position basex, basey;
    DXmSvnEntryPtr svnentry, temp_loc_entry;
    DXmSvnCompPtr  comp;
    int temp_loc, ptr_height;
        
    /******************************************************************************/
    /*                                                                            */
    /* To erase the location cursor in tree mode... just fake that the location   */
    /* cursor entry # and redraw the location cursor entry... then finally reset  */
    /* the location cursor back to the current entry before we return.	          */
    /*                                                                            */
    /******************************************************************************/
    if (svnw->svn.display_mode == DXmSvnKdisplayTree)
	{
	temp_loc = svnw->svn.location_cursor;
	temp_loc_entry = svnw->svn.location_cursor_entry;

	svnw->svn.location_cursor = svnw->svn.location_cursor + 1;
        svnw->svn.location_cursor_entry = (DXmSvnEntryPtr) -1;

	TopTreeHighlightEntry(svnw, temp_loc);

	svnw->svn.location_cursor = temp_loc;
	svnw->svn.location_cursor_entry = temp_loc_entry;
	
	return;
	}
	

    /*  Calculate out the starting x base position for the location cursor */
    /*    in the primary window widget */
     
    basex = svnw->svn.margin_width +
		(svnw->svn.location_cursor_entry->level * svnw->svn.indent_margin) - svnw->svn.window_basex;

    /* Calculate the Y position value */

    basey = 0;
    svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[1]);
    
    for (i = 1; ((svnw->svn.entries[i] != svnw->svn.location_cursor) &&
		 (i <= svnw->svn.display_count)); i++)
	{
	svnentry = StructGetEntryPtr (svnw, svnw->svn.entries[i]);

	basey = basey + svnentry->height;
	}


    /******************************************************************************/
    /*                                                                            */
    /* If we are in Outline or Column mode, clear out the location cursor from    */
    /* window that has the cursor, either the PTR or the main entry window.	  */
    /*                                                                            */
    /******************************************************************************/
    /* Get the correct GC */
    
    if (svnw->svn.location_cursor_entry->selected)
	  {
	  if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	      {
	      if  ((svnw->svn.selection_mode == DXmSvnKselectEntryOrComp) ||
		   (svnw->svn.selection_mode == DXmSvnKselectComp))
		    {
		    if (svnw->svn.location_cursor_entry->selected_comp < svnw->svn.start_column_component)
			gc = svnw->svn.foreground_gc;
		    else
			gc = svnw->svn.background_gc;
		    }
	      else   /* all other selection modes */
		  gc =  svnw->svn.foreground_gc;
	      }	

	  else     /* the display_mode == Outline */
	    gc = svnw->svn.foreground_gc;
	  }
    else    /* Entry is not selected */
	  gc = svnw->svn.background_gc;


    /* Get the width of the entry */
    
    if (svnw->svn.fixed_width)
	    width = XtWidth(svnw->svn.primary_window_widget) - basex;
    else
	    width = svnw->svn.location_cursor_entry->width;


    /* Is this entry in the PTR or main window */

    ptr_height = XtIsManaged(svnw->svn.primary_ptr_widget) ? XtHeight(svnw->svn.primary_ptr_widget) : 0;
	    
    if ( (XtIsManaged(svnw->svn.primary_ptr_widget)) && (i <= svnw->svn.num_path))
	{
	XDrawRectangle (XtDisplay(svnw),
		    XtWindow(svnw->svn.primary_ptr_widget),
		    gc,
		    (Position)basex,
		    (Position)basey,	    
		    (Dimension)width,
		    (Dimension)svnw->svn.location_cursor_entry->height-1);
	
	}
    else
	{
	XDrawRectangle (XtDisplay(svnw),
		    XtWindow(svnw->svn.primary_window_widget),
		    gc,
		    (Position)basex,
		    (Position)basey-ptr_height,   /* subtract off height of PTR window */	    
		    (Dimension)width,
		    (Dimension)svnw->svn.location_cursor_entry->height-1);
	}
	
    if (svnw->svn.display_mode == DXmSvnKdisplayColumns)
	{
	/******************************************************************************/
	/*                                                                            */
	/* If this entry is selected we assign the foreground GC, if and if this	  */
	/* component needs to be selected.						  */
	/*                                                                            */
	/******************************************************************************/
	loc_gc = svnw->svn.background_gc;
	    
	if (svnw->svn.location_cursor_entry->selected)
	    {
	    if (svnw->svn.selection_mode == DXmSvnKselectEntry) 
		    loc_gc = svnw->svn.foreground_gc;

	    if ((svnw->svn.selection_mode == DXmSvnKselectEntryOrComp) &&
		    (svnw->svn.location_cursor_entry->selected_comp  < svnw->svn.start_column_component))
		    loc_gc = svnw->svn.foreground_gc;
	     }

	if (svnw->svn.fixed_width)
	    width = XtWidth(svnw->svn.secondary_window_widget);
	else
	    width = svnw->svn.location_cursor_entry->width -
			XtWidth(svnw->svn.primary_window_widget);
					  
	if ((XtIsManaged(svnw->svn.primary_ptr_widget)) && (i <= svnw->svn.num_path))
	    {
	    XDrawRectangle (XtDisplay(svnw),
			    XtWindow(svnw->svn.secondary_ptr_widget),
			    loc_gc,
			    (Position)0,
			    (Position)basey,
			    (Dimension)width,
			    (Dimension)svnw->svn.location_cursor_entry->height-1);

	    }
	else
	    {
	    XDrawRectangle (XtDisplay(svnw),
			    XtWindow(svnw->svn.secondary_window_widget),
			    loc_gc,
			    (Position)0,
			    (Position)basey-ptr_height,
			    (Dimension)width,
			    (Dimension)svnw->svn.location_cursor_entry->height-1);
	    }
	    
	} /* end of erase cursor in Column Mode */ 


}	/* End of DisplayEraseLocationCursor routine */
