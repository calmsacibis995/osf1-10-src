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
*                        COPYRIGHT (c) 1988, 1989 BY
*            DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
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
/*	022	A. Napolitano		10-Apr-1991			      */
/*		Change "svn_widget" to "Widget" in prototypes.		      */
/*      021	P.Rollman		3-apr-1991                            */
/*		Add function prototypes                                       */
/*	020	A. Napolitano		19-Mar-1991			      */
/*		Add new routine DXmSvnGetComponentText			      */
/*      019	A. Napolitano		25-Feb-1991			      */
/*		Add fields to support live scrolling in SVN.		      */
/*	018	A. Napolitano		11-Dec-1990			      */
/*		Changed def. for DXmSvnNhelpRrequestedCallback so that it is  */
/*		its own instead of defined to be XmNhelpCallback.	      */
/*	017	A. Napolitano		28-Nov-1990			      */
/*		Add event structure to generic callback struc.		      */
/*	016	A. Napolitano		17-Oct-1990			      */
/*		Add another field to callback struct for dragged entry #      */
/*	015	A. Napolitano		20-Sep-1990			      */
/*		Add resource for new callback EntryTransfer.		      */
/*	014	A. Napolitano		20-Aug-1990			      */
/*		Add resources and fields for keyboard traversal and location  */
/*		cursor.							      */
/*	013	A. Napolitano		11-Jul-1990			      */
/*		Change names of variables and anything external to be	      */
/*		internationalized for sides or pane windows.  "rhs"->secondary*/
/*		and "lhs" -> primary.					      */
/*	012	A. Napolitano		10-Jul-1990			      */
/*		Add callback reasons defs and constant defs that are	      */
/*		identical to originals for DXm Motif inclusion rules.	      */
/*	011	A. Napolitano		28-Jun-1990			      */
/*		Make sure all resource strings are prefixed by "DXm"	      */
/*	010	A. Napolitano		28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	009	A. Napolitano		25-Jun-1990			      */
/*		Changes made so that only compound strings are supported      */
/*		instead of text strings and compound strings.		      */
/*	008	S. Lavigne 		30-Mar-1990			      */
/*		Add new entries SvnGetEntryLevel and SvnGetEntrySensitivity.  */
/*	007	Will Walker		05-Feb-1990			      */
/*		Add SvnGet{Lhs|Rhs}WorkWidget.			              */
/*      006	Will Walker		26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	005	S. Lavigne 		24-Jan-1990			      */
/*		Minor changes.					              */
/*	004	Will Walker		23-Jan-1990			      */
/*		Change SvnSelectEntry to DXmSvnKselectEntry.		      */
/*		Change naming scheme for all the constants.		      */
/*	003	S. Lavigne 		23-Jan-1990			      */
/*		Change all BCSESVN references to SVN.  Change high-level      */
/*		routine BcseSvn to SvnWidget and change low-level routine     */
/*		BcseSvnCreate to SvnCreateWidget.  Also, change routine	      */
/*		BcseSvnInitializeForDRM to SvnInitializeForMRM.		      */
/*      002     S. Lavigne              12-Jan-1990                           */
/*              Make some additional adjustments to work under Motif.         */
/*		Change some of the callback reasons around because	      */
/*		SvnCRHelpRequested changed from DwtCRHelpRequested (26) to    */
/*		XmCR_HELP (1).						      */
/*	001	S. Lavigne 		05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/

#ifndef _DXmSvn_h
#define _DXmSvn_h

/*----------------------------------*/
/* Class Name                       */
/*----------------------------------*/

#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#define DXmSvnClassName "Svn"



/*----------------------------------*/
/* Resource names                   */
/*----------------------------------*/

#define DXmSvnNfontList			   "DXmfontListDefault"
#define DXmSvnNfontListLevel0              "DXmfontListLevel0"
#define DXmSvnNfontListLevel1              "DXmfontListLevel1"
#define DXmSvnNfontListLevel2              "DXmfontListLevel2"
#define DXmSvnNfontListLevel3              "DXmfontListLevel3"
#define DXmSvnNfontListLevel4              "DXmfontListLevel4"
#define DXmSvnNindentMargin                "DXmindentMargin"
#define DXmSvnNfixedWidthEntries           "DXmfixedWidthEntries"
#define DXmSvnNnumberOfEntries		   "DXmnumberOfEntries"
#define DXmSvnNdisplayMode                 "DXmdisplayMode"
#define DXmSvnNmultipleSelections          "DXmmultipleSelections"
#define DXmSvnNghostPixmap                 "DXmghostPixmap"
#define DXmSvnNghostX                      "DXmghostX"
#define DXmSvnNghostY                      "DXmghostY"
#define DXmSvnNghostWidth                  "DXmghostWidth"
#define DXmSvnNghostHeight                 "DXmghostHeight"
#define DXmSvnNdefaultSpacing              "DXmdefaultSpacing"
#define DXmSvnNtruncateText		   "DXmtruncateText"
#define DXmSvnNuseScrollButtons            "DXmuseScrollButtons"
#define DXmSvnNexpectHighlighting          "DXmexpectHighlighting"
#define DXmSvnNforceSeqGetEntry            "DXmforceSeqGetEntry"
#define DXmSvnNshowPathToRoot              "DXmshowPathToRoot"
#define DXmSvnNhelpRequestedCallback       "DXmhelpRequestedCallback" 
#define DXmSvnNtreeLevelSpacing		   "DXmtreeLevelSpacing"
#define DXmSvnNtreeSiblingSpacing	   "DXmtreeSiblingSpacing"
#define DXmSvnNtreeStyle		   "DXmtreeStyle"
#define DXmSvnNtreeArcWidth		   "DXmtreeArcWidth"
#define DXmSvnNtreeCenteredComponents	   "DXmtreeCenteredComponents"
#define DXmSvnNtreePerpendicularLines	   "DXmtreePerpendicularLines"
#define DXmSvnNtreeIndexAll		   "DXmtreeIndexAll"
#define DXmSvnNnavWindowTitle		   "DXmnavWindowTitle"
#define DXmSvnNtreeEntryShadows		   "DXmtreeEntryShadows"
#define DXmSvnNtreeEntryOutlines	   "DXmtreeEntryOutlines"
#define DXmSvnNcolumnLines                 "DXmcolumnLines"
#define DXmSvnNstartColumnComponent        "DXmstartColumnComponent"
#define DXmSvnNsecondaryComponentsUnmapped "DXmsecondaryComponentsUnmapped"
#define DXmSvnNsecondaryBaseX              "DXmsecondaryBaseX"
#define DXmSvnNselectionMode               "DXmselectionMode"
#define DXmSvnNpaneWidget                  "DXmpaneWidget"
#define DXmSvnNprimaryWindowWidget         "DXmprimaryWindowWidget"
#define DXmSvnNsecondaryWindowWidget       "DXmsecondaryWindowWidget"
#define DXmSvnNoutlineHScrollWidget        "DXmoutlineHScrollWidget"
#define DXmSvnNprimaryPercentage           "DXmprimaryPercentage"
#define DXmSvnNstartLocationCursor	   "DXmstartLocationCursor"
#define DXmSvnNliveScrolling		   "DXmliveScrolling"
#define DXmSvnNselectAndConfirmCallback    "DXmselectAndConfirmCallback"
#define DXmSvnNextendConfirmCallback       "DXmextendConfirmCallback"
#define DXmSvnNentrySelectedCallback       "DXmentrySelectedCallback"
#define DXmSvnNentryUnselectedCallback     "DXmentryUnselectedCallback"
#define DXmSvnNtransitionsDoneCallback     "DXmtransitionsDoneCallback"
#define DXmSvnNattachToSourceCallback      "DXmattachToSourceCallback"
#define DXmSvnNdetachFromSourceCallback    "DXmdetachFromSourceCallback"
#define DXmSvnNselectionsDraggedCallback   "DXmselectionsDraggedCallback"
#define DXmSvnNgetEntryCallback            "DXmgetEntryCallback"
#define DXmSvnNdraggingCallback            "DXmdraggingCallback"
#define DXmSvnNdraggingEndCallback         "DXmdraggingEndCallback"
#define DXmSvnNdisplayChangedCallback      "DXmdisplayChangedCallback"
#define	DXmSvnNpopupMenuCallback	   "DXmpopupMenuCallback"
#define DXmSvnNentryTransferCallback	   "DXmentryTransferCallback"


/*----------------------------------*/
/* Position Display constants       */
/*----------------------------------*/

#define DXmSvnKpositionTop           1
#define DXmSvnKpositionMiddle        2
#define DXmSvnKpositionBottom        3
#define DXmSvnKpositionPreviousPage  4
#define DXmSvnKpositionNextPage      5

#define DXmSVN_POSITION_TOP		DXmSvnKpositionTop
#define DXmSVN_POSITION_MIDDLE		DXmSvnKpositionMiddle
#define DXmSVN_POSITION_BOTTOM		DXmSvnKpositionBottom
#define DXmSVN_POSITION_PREVIOUS_PAGE	DXmSvnKpositionPreviousPage
#define DXmSVN_POSITION_NEXT_PAGE	DXmSvnKpositionNextPage




/*----------------------------------*/
/* Display Mode constants           */
/*----------------------------------*/

#define DXmSvnKdisplayNone	     0
#define DXmSvnKdisplayOutline        1
#define DXmSvnKdisplayTree	     2
#define DXmSvnKdisplayAllModes       3
#define DXmSvnKdisplayColumns        4

#define DXmSVN_DISPLAY_NONE		DXmSvnKdisplayNone
#define DXmSVN_DISPLAY_OUTLINE		DXmSvnKdisplayOutline
#define DXmSVN_DISPLAY_TREE		DXmSvnKdisplayTree
#define DXmSVN_DISPLAY_ALL_MODES	DXmSvnKdisplayAllModes
#define DXmSVN_DISPLAY_COLUMNS		DXmSvnKdisplayColumns

/*----------------------------------*/
/* Selection Mode constants         */
/*----------------------------------*/

#define DXmSvnKselectEntry           0
#define DXmSvnKselectComp            1
#define DXmSvnKselectCompAndPrimary  2
#define DXmSvnKselectEntryOrComp     3

#define DXmSVN_SELECT_ENTRY		DXmSvnKselectEntry
#define DXmSVN_SELECT_COMP		DXmSvnKselectComp
#define DXmSVN_SELECT_COMP_AND_PRIMARY	DXmSvnKselectCompAndPrimary
#define DXmSVN_SELECT_ENTRY_OR_COMP	DXmSvnKselectEntryOrComp

/*----------------------------------*/
/* Tree Style constants             */
/*----------------------------------*/

#define DXmSvnKtopTree		 1
#define DXmSvnKhorizontalTree	 2
#define DXmSvnKoutlineTree	 3
#define DXmSvnKuserDefinedTree	 4

#define DXmSVN_TOP_TREE			DXmSvnKtopTree
#define DXmSVN_HORIZONTAL_TREE		DXmSvnKhorizontalTree
#define DXmSVN_OUTLINE_TREE		DXmSvnKoutlineTree
#define DXmSVN_USER_DEFINED_TREE	DXmSvnKuserDefinedTree

/*----------------------------------*/
/* Callback reasons                 */
/*----------------------------------*/

#define DXmSvnCRHelpRequested       XmCR_HELP  /* 1 */
#define DXmSvnCRSelectAndConfirm    2
#define DXmSvnCREntrySelected       3
#define DXmSvnCREntryUnselected     4
#define DXmSvnCRAttachToSource      5
#define DXmSvnCRDetachFromSource    6
#define DXmSvnCRSelectionsDragged   8
#define DXmSvnCRGetEntry            9
#define DXmSvnCRDragging            10
#define DXmSvnCRDraggingEnd         11
#define DXmSvnCRExtendConfirm       13
#define DXmSvnCRTransitionsDone     14
#define DXmSvnCRDisplayChanged      15
#define DXmSvnCRPopupMenu	    16
#define DXmSvnCREntryTransfer	    17

#define DXmCR_SVN_SELECT_AND_CONFIRM	DXmSvnCRSelectAndConfirm
#define DXmCR_SVN_ENTRY_SELECTED	DXmSvnCREntrySelected
#define DXmCR_SVN_ENTRY_UNSELECTED	DXmSvnCREntryUnselected
#define DXmCR_SVN_ATTACH_TO_SOURCE	DXmSvnCRAttachToSource
#define DXmCR_SVN_SELECTIONS_DRAGGED	DXmSvnCRSelectionsDragged
#define DXmCR_SVN_GET_ENTRY		DXmSvnCRGetEntry
#define DXmCR_SVN_DRAGGING		DXmSvnCRDragging
#define DXmCR_SVN_DRAGGING_END		DXmSvnCRDraggingEnd
#define DXmCR_SVN_EXTEND_CONFIRM	DXmSvnCRExtendConfirm
#define DXmCR_SVN_TRANSITIONS_DONE	DXmSvnCRTransitionsDone
#define DXmCR_SVN_DISPLAY_CHANGED	DXmSvnCRDisplayChanged
#define DXmCR_SVN_HELP_REQUESTED	DXmSvnCRHelpRequested
#define DXmCR_SVN_POPUP_MENU		DXmSvnCRPopupMenu
#define DXmCR_SVN_ENTRY_TRANSFER	DXmSvnCREntryTransfer

/*----------------------------------*/
/* Callback structure definition    */
/*----------------------------------*/

typedef struct
{
    int           reason;		/*  Used by all    */
    int           entry_number;         /*  Used by 80%    */
    int           component_number;     /*  Used by 40%    */
    int           first_selection;
    int           x;
    int           y;
    XtPointer	  entry_tag;
    Time          time;                 /*  Used by Select */
    int		  entry_level;
    int		  loc_cursor_entry_number;
    int		  transfer_mode;	/* Used for DXmSvnCREntryTransfer */
					/*   whether a move or copy operation */
    int		  dragged_entry_number;
    XEvent	  *event;		/* Event structure */

} DXmSvnCallbackStruct;


/*----------------------------------*/
/* Symbols for specific help items  */
/* returned in entry_number field   */
/* of help callback		    */
/*----------------------------------*/

#define DXmSvnKhelpScroll	-1
#define DXmSvnKhelpNavButton	-2
#define DXmSvnKhelpNavWindow	-3

#define DXmSVN_HELP_SCROLL		DXmSvnKhelpScroll
#define DXmSVN_HELP_NAV_BUTTON		DXmSvnKhelpNavButton
#define DXmSVN_HELP_NAV_WINDOW		DXmSvnKhelpNavWindow

/*----------------------------------*/
/* Symbols for values returned in   */
/* first_selection field of a callback */
/*----------------------------------*/

#define DXmSvnKnotFirst		0
#define DXmSvnKfirstOfOne	1
#define DXmSvnKfirstOfMany	3

#define DXmSVN_NOT_FIRST		DXmSvnKnotFirst
#define DXmSVN_FIRST_OF_ONE		DXmSvnKfirstOfOne
#define DXmSVN_FIRST_OF_MANY		DXmSvnKfirstOfMany

/*----------------------------------*/
/* Symbols for values returned in   */
/* transfer_flag field of a callback */
/*----------------------------------*/
#define DXmSvnKtransferUnknown	0
#define DXmSvnKtransferMove	1
#define DXmSvnKtransferCopy	2


/*----------------------------------*/
/* DXmSvn widget external routines     */
/*----------------------------------*/

#ifdef _NO_PROTO
extern Widget       DXmSvnWidget();
extern Widget       DXmCreateSvn();
extern void         DXmSvnDisableDisplay();
extern void         DXmSvnEnableDisplay();
extern void         DXmSvnMapPosition();
extern int          DXmSvnGetNumSelections();
extern void         DXmSvnGetSelections();
extern void         DXmSvnClearSelection();
extern void         DXmSvnClearSelections();
extern void         DXmSvnSelectAll();
extern void         DXmSvnSelectComponent();
extern void         DXmSvnSelectEntry();
extern void         DXmSvnAddEntries();
extern void         DXmSvnDeleteEntries();
extern void         DXmSvnInvalidateEntry();
extern void         DXmSvnSetEntrySensitivity();
extern void         DXmSvnSetEntry();
extern void         DXmSvnSetComponentPixmap();
extern void	    DXmSvnSetComponentText();
extern XmString	    DXmSvnGetComponentText();
extern void         DXmSvnSetComponentWidget();
extern int          DXmSvnPositionDisplay();
extern XtPointer    DXmSvnGetEntryTag();
extern unsigned int DXmSvnGetEntryNumber();
extern void         DXmSvnHideSelections();
extern void         DXmSvnShowSelections();
extern int          DXmSvnAutoScrollCheck();
extern void         DXmSvnAutoScrollDisplay();
extern Widget	    DXmSvnGetPrimaryWorkWidget();
extern Widget	    DXmSvnGetSecondaryWorkWidget();
extern void         DXmSvnSetApplDragging();
extern int          DXmSvnGetNumHighlighted();
extern void         DXmSvnGetHighlighted();
extern int          DXmSvnGetNumDisplayed();
extern void         DXmSvnGetDisplayed();
extern void         DXmSvnClearHighlight();
extern void         DXmSvnClearHighlighting();
extern void         DXmSvnHighlightAll();
extern void         DXmSvnHighlightEntry();
extern void         DXmSvnShowHighlighting();
extern void         DXmSvnHideHighlighting();
extern unsigned int DXmSvnInitializeForMRM();
extern void         DXmSvnValidateAll();
extern void	    DXmSvnSetComponentHidden();
extern void	    DXmSvnSetEntryTag();
extern void	    DXmSvnSetEntryIndexWindow();
extern void	    DXmSvnSetEntryNumComponents();
extern void	    DXmSvnGetTreePosition();
extern void	    DXmSvnSetTreePosition();
extern void         DXmSvnSetEntryPosition();
extern void         DXmSvnGetEntryPosition();
extern void         DXmSvnFlushEntry();
extern void         DXmSvnInsertComponent();
extern void         DXmSvnRemoveComponent();
extern void         DXmSvnSetComponentWidth();
extern Dimension    DXmSvnGetComponentWidth();
extern void         DXmSvnSetComponentTag();
extern XtPointer    DXmSvnGetComponentTag();
extern int          DXmSvnGetComponentNumber();
extern unsigned int DXmSvnGetEntryLevel();
extern unsigned int DXmSvnGetEntrySensitivity();
#else

/* svn.c */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern unsigned int DXmSvnInitializeForMRM ( void );
extern Widget DXmCreateSvn ( Widget parent , char *name , Arg *arglist , int argCount );
extern void DXmSvnDisableDisplay ( Widget w );
extern void DXmSvnEnableDisplay ( Widget w );
extern void DXmSvnAddEntries ( Widget w , int after_entry , int number_of_entries , int level , XtPointer *entry_tags , int index_window );
extern void DXmSvnDeleteEntries ( Widget w , int after_entry , int number_of_entries );
extern void DXmSvnInvalidateEntry ( Widget w , int entry_number );
extern void DXmSvnSetApplDragging ( Widget w , int value );
extern Widget DXmSvnGetPrimaryWorkWidget ( Widget w );
extern Widget DXmSvnGetSecondaryWorkWidget ( Widget w );

/* svn_display.c */
extern void DXmSvnMapPosition ( Widget w , int findx , int findy , int *entry_number , int *component , XtPointer *tag );
extern int DXmSvnPositionDisplay ( Widget w , int entry_number , int position );
extern int DXmSvnAutoScrollCheck ( Widget w , int x , int y );
extern void DXmSvnAutoScrollDisplay ( Widget w , int x , int y );
extern int DXmSvnGetNumDisplayed ( Widget w );
extern void DXmSvnGetDisplayed ( Widget w , int *entry_nums , XtPointer *entry_tags , int *y_coords , int num_array_entries );
extern void DXmSvnSetEntryPosition ( Widget w , int entry_number , Boolean window_mode , int x , int y );
extern void DXmSvnFlushEntry ( Widget w , int entry_number );
extern void DXmSvnGetEntryPosition ( Widget w , int entry_number , Boolean window_mode , int *x , int *y );
extern void DXmSvnGetTreePosition ( Widget w , int *x , int *y );
extern void DXmSvnSetTreePosition ( Widget w , int x , int y );


/* svn_selections.c */
extern int DXmSvnGetNumSelections ( Widget w );
extern int DXmSvnGetNumHighlighted ( Widget w );
extern void DXmSvnGetSelections ( Widget w , int *selections , int *comps , XtPointer *entry_tags , int num_array_entries );
extern void DXmSvnGetHighlighted ( Widget w , int *highlighted , XtPointer *entry_tags , int num_array_entries );
extern void DXmSvnClearSelection ( Widget w , int entry_number );
extern void DXmSvnClearHighlight ( Widget w , int entry_number );
extern void DXmSvnClearSelections ( Widget w );
extern void DXmSvnClearHighlighting ( Widget w );
extern void DXmSvnSelectAll ( Widget w );
extern void DXmSvnHighlightAll ( Widget w );
extern void DXmSvnSelectComponent ( Widget w , int entry_number , int comp_number );
extern void DXmSvnSelectEntry ( Widget w , int entry_number );
extern void DXmSvnHighlightEntry ( Widget w , int entry_number );
extern void DXmSvnHideSelections ( Widget w );
extern void DXmSvnHideHighlighting ( Widget w );
extern void DXmSvnShowSelections ( Widget w );
extern void DXmSvnShowHighlighting ( Widget w );


/* svn_structure.c */
extern void DXmSvnSetEntry ( Widget w , int entry_number , int width , int height , int num_components , Boolean sensitivity , XtPointer entry_tag , Boolean index_window );
extern void DXmSvnSetEntrySensitivity ( Widget w , int entry_number , Boolean sensitivity );
extern void DXmSvnSetEntryNumComponents ( Widget w , int entry_number , int num_components );
extern void DXmSvnSetEntryIndexWindow ( Widget w , int entry_number , Boolean index_window );
extern void DXmSvnSetEntryTag ( Widget w , int entry_number , XtPointer entry_tag );
extern void DXmSvnSetComponentHidden ( Widget w , int entry_number , int component_number , int hidden_mode );
extern void DXmSvnSetComponentText ( Widget w , int entry_number , int component_number , int x , int y , XmString cs , XmFontList fontlist );
extern XmString DXmSvnGetComponentText ( Widget w , int entry_number , int comp_number );
extern void DXmSvnSetComponentPixmap ( Widget w , int entry_number , int comp_number , int x , int y , Pixmap pixmap , int width , int height );
extern void DXmSvnSetComponentWidget ( Widget w , int entry_number , int comp_number , int x , int y , Widget subw );
extern XtPointer DXmSvnGetEntryTag ( Widget w , int entry_number );
extern unsigned int DXmSvnGetEntryNumber ( Widget w , XtPointer tag );
extern unsigned int DXmSvnGetEntryLevel ( Widget w , int entry_number );
extern unsigned int DXmSvnGetEntrySensitivity ( Widget w , int entry_number );
extern void DXmSvnValidateAll ( Widget w );
extern void DXmSvnInsertComponent ( Widget w , int comp_number , Dimension width , XtPointer tag );
extern void DXmSvnRemoveComponent ( Widget w , int comp_number );
extern void DXmSvnSetComponentWidth ( Widget w , int comp_number , Dimension width );
extern Dimension DXmSvnGetComponentWidth ( Widget w , int comp_number );
extern void DXmSvnSetComponentTag ( Widget w , int comp_number , XtPointer tag );
extern XtPointer DXmSvnGetComponentTag ( Widget w , int comp_number );
extern int DXmSvnGetComponentNumber ( Widget w , XtPointer comp_tag );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _DXmSvn_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
