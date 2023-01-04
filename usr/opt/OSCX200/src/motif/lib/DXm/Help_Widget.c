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
***********************************************************
**                                                        *
**  Copyright (c) Digital Equipment Corporation, 1990  	  *
**  All Rights Reserved.  Unpublished rights reserved	  *
**  under the copyright laws of the United States.	  *
**                                                        *
**  The software contained on this media is proprietary	  *
**  to and embodies the confidential technology of 	  *
**  Digital Equipment Corporation.  Possession, use,	  *
**  duplication or dissemination of the software and	  *
**  media is authorized only pursuant to a valid written  *
**  license from Digital Equipment Corporation.	    	  *
**  							  *
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	  *
**  disclosure by the U.S. Government is subject to	  *
**  restrictions as set forth in Subparagraph (c)(1)(ii)  *
**  of DFARS 252.227-7013, or in FAR 52.227-19, as	  *
**  applicable.	    					  *
**  		                                          *
***********************************************************
**++
**  FACILITY:
**
**	< to be supplied >
**
**  ABSTRACT:
**
**	< to be supplied >
**
**  ENVIRONMENT:
**
**	< to be supplied >
**
**  MODIFICATION HISTORY:
**
**	< to be supplied >
**
**--
**/


/*
**++
**  Subsystem:
**	Help system
**
**  Description:
**	This module provides the routines to create the help widget
**
**  Version: X0.1
**
**  Keywords:
**	Help
**
**  Authors:
**	Patricia Avigdor, CASEE group, MEMEX project
**
**  Creation Date: 25-Nov-1987
**
**  Modification History:
**	15-Dec-87   Pat	    implement new UI design
**	29-Dec-87   Pat	    BL6.2 merge
**	11-Jan-88   Pat	    BL6.3 merge
**	13-Jan-88   Pat	    implement titles + keywords
**	14-Jan-88   Pat	    BL6.4 merge
**	25-Jan-88   Pat	    IFT11 merge
**	27-Jan-88   Pat	    menubar sensitive even if error
**	03-Feb-88   Pat	    port to ULTRIX
**	04-Feb-88   Pat	    implement copy to clipboard
**	05-Feb-88   Pat	    work on SetValues
**	18-Feb-88   Pat	    add unmap callback, context sensitive help
**	24-Feb-88   Pat	    change subwidgets names for xdefaults.dat
**	25-Feb-88   Pat	    fix include files
**	07-Mar-88   Pat	    intrinsics updates
**	09-Mar-88   Pat	    UI changes
**	10-Mar-88   Pat	    work on compound strings use
**	22-Mar-88   Pat	    add search dialog boxes and functions
**	05-Avr-88   Pat	    fixing bugs
**	27-Avr-88   Pat	    fixing bugs
**	05-May-88   Pat	    adding new resources to fix greying of menu items
**	13-May-88   Andre   Add CSH for the search dialog boxes
**	16-May-88   Andre   Use file selection widget for 'Save as...'
**	16-Sep-88   Andre   Fix library context initialization bug
**	26-Sep-88   Andre   Make help text appear at the beginning
**	28-Sep-88   Pat	    remove spacing on menubar following Leo's guidelines
**	15-Dec-88   Pat	    SetValues on new library and new topic.
**	19-Dec-88   Pat	    clean up resources and add new ones.
**	06-Jan-89   Pat	    implement SetValues on all labels.
**	12-Jan-89   Pat     implement context sensitive help for goback end exit
**			    buttons.
**	12-Jan-89   Pat	    change default behavior when first topic is null.
**	12-Jan-89   Pat	    implement map callback.
**	17-Jan-89   Pat	    fix repositioning of ADB when managed.
**	19-Jan-89   Pat	    implement 1 versus many help widget for help on help.
**	20-Jan-89   Pat	    fix history box for visited widgets
**	23-Jan-89   Pat	    fix geometry management
**	24-Jan-89   Pat	    fix font propagation
**	25-Jan-89   Pat	    fix help title composition
**       1-Aug-89   Leo     Q&D port to Motif
**	09-May-90   Rich    Change all calls from DXmMakeGeometryRequest to
**			    _XmMakeGeometryRequest.
**			    Change all calls from DXmChangeWindowGeometry to
**			    _XmConfigureObject.
**
**	17-May-90   Rich    Remove typecasting of XtTranslations on
**			    XtInheritTranslations specification in
**			    manager_class fields.
**
**	25-May-90   Rich    In Realize, resize ourselves smaller if we don't fit
**			    on low-res device.
**
**	06-Jun-90   Rich    In open_library, transmit help widget id to
**			    DXmHelp__init_help.
**
**	15-Jun-90   Rich    Add mnemonics.
**
**	22-Jun-90   Rich    In build_search_title and build_search_keyword,
**			    hang the listbox off of the push-button which is
**		 	    now the tallest of the 3 objects above the listbox.
**
**	27-Jun-90   Rich    Change the way _DXmCheckFit is used -- now that it
**			    returns correct values.
**	09-Jul-90   Rich    Add entry point DXmCreateHelpDialog.
**			    Same code as DXmCreateHelp, but abides by naming
**			    convention.
**	12-Jul-90   Rich    For Non-VMS systems, use an abbreviated pathname.
**			    Let XtResolvePathName figure out the rest of the
**			    path.
**	17-Jul-90   Rich    Change names of default help on help.
**			    On VMS it goes to file:
**				SYS$HELP:DECW$DXMHELP_HELP.HLB.
**			    On non-VMS systems it goes to a directory whose last
**			    named path segment is DXmHelp_Help.
**
**	31-Jul-90   Rich    Before relying on widget id returned by
**			    XmTrackingLocate, see if it is non-zero.
**			    Perform Ungrab if we see a zero widget id, because
**			    XmTrackingLocate doesn't in this case.
**
**	13-Aug-90   Rich    Add an Expose handler to catch situation where
**			    an initial error message has not been popped to
**			    the front.
**
**	19-Aug-90   Rich    Add dialog_style field and XmNdialogStyle resource
**			    to help_widget.
**
**	20-Aug-90   Rich    Reconcile DECIsreal changes with latest.
**
**	21-Aug-90   Rich    Pick up declaration for DXMCreateHelpDialog from
**			    extern declaration in DXMHelpB.h rather than from
**			    from local forward declaration.
**			    Start using the DXm standard wait cursor, delivered
**			    by _DXmCreateWaitCursor.
**
**	27-Aug-90   Rich    Add type casting to make I18N changes compile
**			    cleanly on non-VMS systems.
**
**	29-Aug-90   Rich    Missed some typecasting on last edit.
**	04-Sep-90   Rich    Fix accvio at Destroy time.  Bug introduced with
**			    Dialog Style code.
**
**	07-Sep-90   Rich    Re-implement get_string_from_cs to do away with its
**			    usage of XmStringGetNextSegment.
**
**	10-Sep-90   Rich   Comment on the state of Dialog Style as of IFT:
**			   The code that was added to the Help widget to support
**			   dialog style was roughly lifted from the Bulletin
**			   Board widget.
**			   A slot was made in the Help widget instance structure
**			   to hold the dialog style of the Help widget.
**			   Some supporting code is in place, e.g., the dialog
**			   style of the current help widget is propagated to
**			   secondary help widgets created.
**			   However, the mechanisms for actually setting and
**			   manipulating this field were not completely debugged.
**			   In particular, some critical code lies inside of a
**			   clause that asks:
**			   if (XtIsDialoghell (XtParent(help_widget,...
**			   This will never be true for the help widget, whose
**			   parent is a HelpShell widget (classed off of
**		           ToplevelShell).
**			   As result, the majority of the current code dealing
**			   with Help widget Dialog style is never executed
**			   unless the caller does an explicit SetValues on
**			   resource XmNdialogStyle.  This should be discouraged
**			   until this code is working.
**
**			   There are some more serious issues that need to be
**			   resolved first.  In order to make dialog style
**			   work correctly, it may be necessary to change the
**			   the kind of shell the Help widget uses for the
**			   DXmCreateHelpDialog entry point.
**			   If such a change is made to support the
**			   DXmCreateHelpDialog entry point, care must be used to
**			   make the DXmCreateHelp continue to function as it now
**			   does, else apps may break.  Right now there is no
**			   difference between the two entry points.
**
**	9-Oct-90   Vick	   Move I18n stuff to DXmMisc.c and I18n.c
**
**	10-Oct-90  Rich    Comment out the remainder of the dialog style
**			   support.
**	16-Oct-90  Rich    Routine build_avoid_widgets assumes that the parent
**			   of the help widget is always the help shell widget.
**			   As a result it does
**			   XtParent(help_widget->core.parent) computations to
**			   get at the parent of its shell.
**			   Some applications, like VUIT, create the help widget
**			   via a XtCreateWidget call, giving it a parent which
**			   is a toplevel shell.  In such a case,
**			   XtParent(help_widget->core.parent) yields a 0.
**			   This broke the assumption within build_avoid_widgets
**			   that you would encounter a non-help widget widget
**			   before running out of parents, by searching the
**			   parent chain.
**
**	08-Nov-90  Rich    Fix miscellaneous styleguide items.
**			   . In build_search_title, the fontlist of the visit
**			     button was being erroneous overwritten with the
**			     text_font_list.
**
**			   . In DXmCreateHelp and DXmCreateHelpDialog , keep
**			     help shell from resizing too small to be useful.
**
**			   . Prevent search boxes and error message boxes from
**			     being resizable via window decorations.  It makes
**			     no sense to resize these objects.
**
**      27-Nov-90  Rich    In SetValues routine, set the Help widget's shell's
**                         (x,y) to where they will be when the setvalues is
**                         over, and then restore them before leaving the
**                         setvalues routine.  This allows a popup error message
**                         produced if get_help encounters an error,
**                         to get popped up over the right spot -- namely
**                         centered over where the help widget will be.
**
**	27-Nov-90  Rich    Blend in latest changes from DECIsreal.
**			   Modified the Help widget to take advantage of the
**			   fact that RtoL geometry is supported by the XmForm
**			   widget (Which was not the case when we delivered
**			   the Help widget back in August).
**
**			   So, instead of changing attachment inside the Help
**			   widget to support both kinds of geometry, we let
**			   XmForm do the work for the Help widget.
**			   Without this change the RtoL geometry cannot be
**			   realized since the current Help widget changes the
**			   attachment of it's subordinates but the XmForm is
**			   doing it also... so it is actually reversed again...
**
**	28-Nov-90  Rich    Revise SetValues changes to only diddle the
**			   the helpshell's x,y during SetValues if the x,y
**			   are, in fact, changing.
**
**	19-Dec-90  Rich	   At widget Realize time, register a WMProtocol
**			   registration to catch a Close operation by the
**			   Window Manager.  In response to this callback,
**			   we perform the code equivalent to being activated
**			   with our Exit button.
**			   If we don't do this, the Window Manager seems to
**			   delete off our shell and we don't know about it.
**			   On a subsequent help call, an accvio results in
**			   setvalues 'cause it can't find the parent of
**			   help_widget.
**	07-Jan-91  Rich	   Switch over to using the  "?" cursor for context-
**			   sensitive help processing.
**	15-Jan-91  Rich	   In Initialize, temporarily do NOT free the
**			   CurrentCharset variable. Its value is the result of a
**			   DXMGetLocaleCharset call, but the resulting string
**			   may not be heap storage at all.
**			   This may have to be undone if the 
**			   DXMGetLocaleCharset/I18NGetLocaleCharset interface
**			   changes.
**			   
**	28-Jan-91  Jim V.  #ifdef use of decw$c_questionmark_cursor so this
**			   will compile on ULTRIX where questionmark cursor
**			   isn't defined (yet).  Also check decw$cursor font
**			   to make sure it contains the questionmark cursor.
**			   
**	31-Jan-91  Rich	   Switch to using the MIT question mark cursur
**			   (XC_question_arrow) as a fallback if we can't get
**			   decw$c_questionmark_cursor. [e.g. on non-VMS systms]
**			   
**	31-Jan-91  Rich    Remove dependence on R4_INTRINSICS symbol.
**			   
**	05-Feb-91  Rich	   Straighten out type casting problems introduced with
**			   MOTIF 1.1.1 update.
**
**	18-Feb-91  Aston   Include the I18n.h header file.  Add I18nContext
**			   param to the DXmGetLocaleString() and
**			   DXmGetLocaleMnemonic() routines
**	19-Feb-91  Jim V   change I18n.h to DECwI18n.h, add include of
**			   DECspecific.h
**
**	04-Mar-91  Rich	   Remove everything from the Using Help pulldown
**			   menu except for On Windows.
**			   In particular, this does away with the entries for:
**				On Context, On Version and On Terms.
**
**			   Fixed bug.  In build_work_area, within a 
**			   ifdef RTOL, we were setarg'ing into arglist rather
**			   than listal.
**	03-Apr-91  Rich	   In Initialize, free the  CurrentCharset variable.
**			   Its value is the result of a DXMGetLocaleCharset
**			   call, and the resulting string is now heap storage.
**			   This is basically, undoing fix of 15-Jan-1991.
**			   DXMGetLocaleCharset/I18NGetLocaleCharset interface
**			   changed.
**			   
**			   In Destroy proc, stop destroying children.
**			   They are automatically destroyed by Intrinsics when
**			   the help widget itself is destroyed.
**	04-Apr-91  Will	   Add function prototypes.
**
**	27-Sept-91 Angela  Fixed bug in which formDialog boxes could not be
**			   canceled via the F11 (osfCancel) button.  Added
**			   setvalues on search menus' search_history, search_title
**			   and search_keyword dialog boxes for XmNcancelButton.
**
**	07-Apr-91  Cliff   Fixed bug in build_work_area. If the first time the
**			   help widget was called there were no additional topics,
**			   then the size of the list box was not set properly.
**
**	07-Apr-91  Rich	   Fixed attachments bug in the three search dialogs.
**			   Lack of right and bottom attachments on some widgets
**			   was causing parts of the form dialog shadow to be
**			   obscured.
**
**			   In Destroy proc, stop destroying children.
**			   They are automatically destroyed by Intrinsics when
**			   the help widget itself is destroyed.
**
**      6-Nov-92   Ken     Fixed listbox behavior to use Browse style and
**                         force selection to track with insertion point.
**                         Also changed list boxes to *always* have a 
**                         default selection.  This is because assumptions
**                         were made in the defaultAction code about selections
**                         and they were either picking up the last active
**                         selection and/or core dumping cause it was an 
**                         invalid item index.
**
**	30-Dec-92  Rich	   Undo the fix of 03-Apr-91 for Motif 1.2.  
**			   DXMGetLocaleCharset once again returns a pointer
**			   to private data, so freeing it is taboo.
**			   
**	08-Feb-93  Andy	   Convert static string to character array.
**--
*/

/*
**  Include Files
*/

#define HELPB

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/ShellP.h>
#include <X11/VendorP.h>
#include <Xm/BulletinBP.h>
#include <Xm/LabelP.h>
#include <Xm/PushBP.h>
#include <Xm/RowColumnP.h>
#include <Xm/CascadeBP.h>
#include <Xm/SeparatoGP.h>
#include <Xm/LabelGP.h>
#include <Xm/PushBGP.h>
#include <Xm/ScrolledWP.h>
#include <Xm/FormP.h>
#include <Xm/MessageBP.h>
#include <Xm/ListP.h>
#include <Xm/TextP.h>
#include <Xm/SelectioBP.h>
#include <Xm/FileSBP.h>
#include <Xm/CutPaste.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>		/* for XmAddWMProtocolCallback, etc. */
#include "DXmPrivate.h"
#include <DXm/DXmHelpBP.h>
#include <DXm/DXmHelpSP.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Vendor.h>
#include <X11/DECwI18n.h>
#include <DXm/DECspecific.h>
#include "DXmMessI.h"
#ifdef VMS				
#include <descrip.h>			/* descriptor definitions	    */
#include <DECW$Cursor.h>
#else
#include "Descripu.h"			/* descriptor definitions	    */
#include <X11/decwcursor.h>
#endif

/*
**  Macro Definitions
*/
#ifndef MAX
#define MAX(x,y)  (((x) > (y)) ? (x) : (y))
#endif

#ifdef RTOL
#define LayoutDirection(w)  ((w)->manager.dxm_layout_direction)
#define LayoutIsRtoL(w)     ((w)->manager.dxm_layout_direction == DXmLAYOUT_LEFT_DOWN )

#endif /* RTOL */

#ifdef HELP_MNEM_CHARSET
extern	KeySym    	DXmGetLocaleMnemonic();
extern	XmStringCharSet	DXmGetLocaleCharset();
extern	Boolean 	DXmContainsStringCharset();
#endif /* HELP_MNEM_CHARSET */


/*****************************
#ifndef void
typedef void (*VoidProc) ();
#endif
*****************************/

#define MARGIN_WIDTH 10			    /* text widget margin width	    */
#define MARGIN_HEIGHT 5 		    /* text widget margin height    */
#define VISIBLE_TOPIC 5			    /* number of visible topics	    */

#define MIN_HELP_WIDGET_HEIGHT		260 /* Minimum resize height        */
#define MIN_HELP_WIDGET_WIDTH		390 /* Minimum resize width         */

#ifdef VMS
#define LIBRARY	"sys$help:decw$dxmhelp_help" /* VMS help on help library    */
#else
#define LIBRARY	"DXmHelp_Help"              /* ULTRIX help on help library  */
#endif

#define DXMHELPFONT "-*-Terminal-Medium-R-Narrow--*-140-*-*-C-*-ISO8859-1"

#define STS$K_SUCCESS    1
#define STS$V_SUCCESS    0x00
#define STS$M_SUCCESS    0x00000001
#define VMS_STATUS_SUCCESS(code)  ( ( (code) & STS$M_SUCCESS ) 	>> STS$V_SUCCESS )

#ifdef DIALOG_STYLE_SUPPORT
#define WARN_DIALOG_STYLE       _DXmMsgHlpWid_0000
#endif  /*DIALOG_STYLE_SUPPORT*/

static void 		HwDialogStyleDefault();


/*
**  External declarations
*/
extern int	    DXmHelp__init_help();
extern int	    DXmHelp__get_frame ();
extern int	    DXmHelp__search_title ();
extern int	    DXmHelp__search_keyword ();
extern int	    DXmHelp__get_keyword ();
extern int	    DXmHelp__free_array();
extern int	    DXmHelp__free_frame();
extern int	    DXmHelp__close_help();
extern int	    DXmHelp__set_cache_mode();
extern void	    HelpSetup();
extern void	    HelpDisplay();
extern void	    HelpPropagateArgs();
extern void	    DXmHelpCleanup();
extern void	    DXmPositionWidget();
    

/*
**  Type Definitions
*/

typedef struct __MsgVecInstance {
	    unsigned long  num_words;
	    XmString message;
	    unsigned int  fao_count;
	    XmString fao_parameter;
	} _MsgVecInstance, *_MsgVec;
	
typedef XmAnyCallbackStruct *Reason;

/*
**  Global Data Definitions
*/
static	XColor  foreground = {0, 65535,     0,     0} ; /* Red */
static	XColor  background = {0, 65535, 65535, 65535} ; /* White */
	
/*
** Widget resources
*/
static const int	def_library_type 	= DXmTextLibrary ;
static const Boolean	def_pos 		= TRUE;
static const int	def_colons 		= 55;
static const int	def_rows 		= 20;
static const Boolean	default_caching		= FALSE;
static const int	MaxClipboardTries	= 1000;
       
static XtResource DXmHelpResources[] = {


	{	XmNstringDirection,
		XmCStringDirection, XmRStringDirection, sizeof (XmStringDirection),
		XtOffset (DXmHelpWidget, dxmhelp.string_direction),
		XmRStringDirection, (XtPointer) XmSTRING_DIRECTION_L_TO_R
	},

	{	XmNbuttonFontList,
		XmCButtonFontList, XmRFontList, sizeof (XmFontList),
		XtOffset (DXmHelpWidget, dxmhelp.button_font_list),
	        XmRString,
		(XtPointer) NULL		/* I18N */
	},

	{	XmNlabelFontList,
		XmCLabelFontList, XmRFontList, sizeof (XmFontList),
		XtOffset (DXmHelpWidget, dxmhelp.label_font_list),
	        XmRString,
		(XtPointer) NULL		/* I18N */
	},

	{	XmNtextFontList,
		XmCTextFontList, XmRFontList, sizeof (XmFontList),
		XtOffset (DXmHelpWidget, dxmhelp.text_font_list),
	        XmRString,
		(XtPointer) NULL		/* I18N */
	},


	{	XmNmapCallback,
		XmCCallback, XmRCallback, sizeof (XtCallbackList),
		XtOffset (DXmHelpWidget, dxmhelp.map_callback),
		XmRImmediate, (XtPointer) NULL
	},

	{	XmNunmapCallback,
		XmCCallback, XmRCallback, sizeof (XtCallbackList),
		XtOffset (DXmHelpWidget, dxmhelp.unmap_callback),
		XmRImmediate, (XtPointer) NULL
	},

   {DXmNcols,			DXmCCols,			XtRInt,
    sizeof (int),		XtOffset (DXmHelpWidget, dxmhelp.colons),
    XtRInt,			(XtPointer) &def_colons},

   {DXmNrows,			DXmCRows,			XtRInt,
    sizeof (int),		XtOffset (DXmHelpWidget, dxmhelp.rows),
    XtRInt,			(XtPointer) &def_rows},

   {DXmNdefaultPosition,	DXmCDefaultPosition,		XtRBoolean,
    sizeof (Boolean),		XtOffset (DXmHelpWidget, dxmhelp.default_pos),
    XtRBoolean,			(XtPointer) &def_pos},

   {DXmNlibraryType,		DXmCLibraryType,		XtRInt,
    sizeof (int),		XtOffset (DXmHelpWidget, dxmhelp.library_type),
    XtRInt,			(XtPointer) &def_library_type},

   {DXmNapplicationName,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.application),
    XtRString,			(XtPointer) NULL},

   {DXmNlibrarySpec,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.library),
    XtRString,			(XtPointer) NULL},
    
   {DXmNfirstTopic,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.first_topic),
    XtRString,			(XtPointer) NULL},
    
   {DXmNoverviewTopic,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.overview),
    XtRString,			(XtPointer) NULL},

   {DXmNglossaryTopic,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.glossary),
    XtRString,			(XtPointer) NULL},
    
   {DXmNviewLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.view_menu_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSView},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNgotoLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.goto_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGoto},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNgobackLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.goback_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGoback},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNgooverLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.goover_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGooverview},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNvisitLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.visit_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSVisit},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNvisitglosLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.visitglos_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSVisitglossary},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNfileLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.file_menu_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSFile},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNsaveasLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.saveas_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSaveas},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNexitLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.exit_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSExit},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNeditLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.edit_menu_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSEdit},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNcopyLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.copy_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSCopy},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNselectallLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.selectall_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSelectall},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNsearchLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.search_menu_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSearch},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNhistoryLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.history_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHistory},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNtitleLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.title_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSTitle},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNkeywordLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.keyword_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSKeyword},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNhelpLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.help_menu_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelpOnHelp},
#endif /* HELP_LOCALE_STRINGS */

#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNglossaryLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.glossary_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGlossary},
#endif /* HELP_LOCALE_STRINGS */
#endif 	/* Remove everything except On Windows from Using Help Menu */


#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNaboutLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.about_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSAbout},
#endif /* HELP_LOCALE_STRINGS */
#endif 	/* Remove everything except On Windows from Using Help Menu */


#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNoncontextLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.oncontext_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSOncontext},
#endif /* HELP_LOCALE_STRINGS */
#endif 	/* Remove everything except On Windows from Using Help Menu */

   {DXmNaddtopicLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.addtopic_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSAddtopic},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNhistoryboxLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.history_box_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelptopichistory},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNtopictitlesLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.topic_titles_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSTopictitles},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNdismissLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.dismiss_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSDismiss},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNsearchtitleboxLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.searchtitle_box_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSearchtopictitles},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNtitlesLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.titles_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSTitles},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNsearchkeywordboxLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.searchkeyword_box_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSearchtopickeywords},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNkeywordsLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.keywords_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSKeywords},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNsearchapplyLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.apply_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSearchapply},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNgototopicLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.goto_topic_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGotoTopic},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNgobacktopicLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.goback_topic_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGobackTopic},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNvisittopicLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.visit_topic_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSVisitTopic},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNcloseLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.quit_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSClose},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNhelphelpLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.helphelp_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelp},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNhelpontitleLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.helpontitle_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelpontitle},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNhelptitleLabel,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.helptitle_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelptitle},
#endif /* HELP_LOCALE_STRINGS */
    
   {DXmNbadlibMessage,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.badlib_message),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSBadlibrary},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNbadframeMessage,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.badframe_message),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSBadframe},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNnulllibMessage,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.nulllib_message),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSNulllibrary},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNnotitleMessage,		XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.notitle_message),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSNotitle},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNnokeywordMessage,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.nokeyword_message),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSNokeyword},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNerroropenMessage,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.erroropen_message),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSErroropen},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNhelpAcknowledgeLabel,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.acknowledge_label),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelpAcknowledgeLabel},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNhelpOnHelpTitle,	XmCXmString,			XmRXmString,
    sizeof (char *),		XtOffset (DXmHelpWidget, dxmhelp.helponhelp_title),
#ifdef HELP_LOCALE_STRINGS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelpOnHelpTitle},
#endif /* HELP_LOCALE_STRINGS */

   {DXmNcacheHelpLibrary,	DXmCCacheHelpLibrary,		XtRBoolean,
    sizeof (Boolean),		XtOffset (DXmHelpWidget, dxmhelp.cache_library),
    XtRBoolean,			(XtPointer) &default_caching},


#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNaboutLabelMnem,		XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.about_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSAboutmnem},
#endif /* HELP_LOCALE_MNEMONICS */
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNoncontextLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.oncontext_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSOncontextmnem},
#endif /* HELP_LOCALE_MNEMONICS */
#endif 	/* Remove everything except On Windows from Using Help Menu */

   {DXmNcopyLabelMnem,		XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.copy_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSCopymnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNeditLabelMnem,		XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.edit_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSEditmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNexitLabelMnem,		XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.exit_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSExitmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNfileLabelMnem,		XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.file_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSFilemnem},
#endif /* HELP_LOCALE_MNEMONICS */

#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNglossaryLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.glossary_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGlossarymnem},
#endif /* HELP_LOCALE_MNEMONICS */
#endif 	/* Remove everything except On Windows from Using Help Menu */

   {DXmNgooverLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.goover_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGoovermnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNhelpLabelMnem,		XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.help_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelpmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNhistoryLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.history_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHistorymnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNkeywordLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.keyword_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSKeywordmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNsaveasLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.saveas_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSaveasmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNsearchLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.search_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSearchmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNselectallLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.selectall_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSSelectallmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNtitleLabelMnem,		XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.title_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSTitlemnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNviewLabelMnem,		XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.view_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSViewmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNvisitglosLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.visitglos_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSVisitglossarymnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNgototopicLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.goto_topic_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGotoTopicmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNgobackLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.goback_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSGobackmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNvisittopicLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.visit_topic_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSVisitTopicmnem},
#endif /* HELP_LOCALE_MNEMONICS */

   {DXmNhelphelpLabelMnem,	XmCMnemonic,			XmRKeySym,
    sizeof (KeySym),		XtOffset (DXmHelpWidget, dxmhelp.helphelp_label_mnem),
#ifdef HELP_LOCALE_MNEMONICS
    XtRString,                  (XtPointer) NULL},
#else
    XtRString,			DXmSHelpHelpmnem},
#endif /* HELP_LOCALE_MNEMONICS */

#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNaboutLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.about_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},
#endif 	/* Remove everything except On Windows from Using Help Menu */


#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNoncontextLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.oncontext_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},
#endif 	/* Remove everything except On Windows from Using Help Menu */

   {DXmNcopyLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.copy_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNeditLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.edit_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNexitLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.exit_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNfileLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.file_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

#if 0	/* Remove everything except On Windows from Using Help Menu */
   {DXmNglossaryLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.glossary_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},
#endif 	/* Remove everything except On Windows from Using Help Menu */

   {DXmNgooverLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.goover_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNhelpLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.help_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNhistoryLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.history_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNkeywordLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.keyword_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNsaveasLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.saveas_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNsearchLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.search_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNselectallLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.selectall_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNtitleLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.title_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNviewLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.view_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNvisitglosLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.visitglos_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNgototopicLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.goto_topic_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNgobackLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.goback_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNvisittopicLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),    XtOffset(DXmHelpWidget,dxmhelp.visit_topic_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {DXmNhelphelpLabelMnemCS,	XmCMnemonicCharSet,             XmRString,
    sizeof(XmStringCharSet),   	XtOffset(DXmHelpWidget,dxmhelp.helphelp_label_mnem_cs),
    XmRImmediate,               (XtPointer) NULL},

   {XmNdialogStyle,		XmCDialogStyle, 		XmRDialogStyle,
    sizeof (unsigned char),	XtOffset (DXmHelpWidget, dxmhelp.dialog_style),
    XmRCallProc, 		(XtPointer) HwDialogStyleDefault},
 

} ;
	
/*
**  Forward declarations
*/
static Cursor		HwCreateWaitCursor();
static void		HwDisplayWaitCursor();
static void		HwDisplayAllWaitCursor();
static void		HwRemoveWaitCursor();
static void		HwRemoveAllWaitCursor();
static void		HwRaiseWindow();
static void		add_kid ();
static void		remove_child ();
static void		free_array ();
static void		insert_trail ();
static void		remove_trail ();
static void		free_trail ();
static void		check_message ();
static void		error_ok_cb();
static void		error_help_cb();
static void		set_message();
static void		pulling_view();
static Boolean		NullTopicInTrail();
static int		get_string_from_cs ();
static void		get_help ();
static void		go_to ();
static void		get_help_arguments();
static void		visit_help ();
static void		visit ();
static void		goback ();
static void		goto_overview ();
static void		visit_glossary ();
static void		help ();
static void		help_on_help();
#if 0	/* Remove everything except On Windows from Using Help Menu */
static void		help_glossary ();
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
static void 		help_oncontext ();
static void		help_about ();
#endif 	/* Remove everything except On Windows from Using Help Menu */

static void		help_view ();
static void		help_view_goto ();
static void		help_view_goback ();
static void		help_view_overview ();
static void		help_view_visit ();
static void		help_view_glossary ();
static void		help_file ();
static void		help_edit ();
static void		help_edit_copy ();
static void		help_edit_selectall ();
static void		help_search ();
static void		help_search_history ();
static void		help_search_title ();
static void		help_search_keyword ();
static void		help_help ();
static void		help_help_help ();
#if 0	/* Remove everything except On Windows from Using Help Menu */
static void		help_help_glossary ();
#endif 	/* Remove everything except On Windows from Using Help Menu */
#if 0	/* Remove everything except On Windows from Using Help Menu */
static void		help_help_about ();
#endif 	/* Remove everything except On Windows from Using Help Menu */
static void		help_frame ();
static void		help_listbox ();
static void		help_goback ();
static void		help_exit ();
static void             help_history ();
static void             help_title ();
static void		help_saveas();
#if 0	/* Remove everything except On Windows from Using Help Menu */
static void		on_context();
#endif 	/* Remove everything except On Windows from Using Help Menu */
static void		csh_menubar();
static void             help_keyword ();
static void		pulling_file();
static void		ok_saveas ();
static void             cancel_saveas ();
static void		build_saveas ();
static void		quit ();
static void		pulling_edit ();
static void		copy_clipboard ();
static Boolean		LockClipboard();
static int		CopyTextToClipboard();
static void		selectall ();
static void		pulling_search ();
static void		select_history ();
static void		goto_select_history ();
static void		set_history ();
static void		add_history ();
static void		free_history ();
static void		goto_history ();
static void		visit_history ();
static void		dismiss_history ();
static void		build_search_history ();
static void		search_title ();
static void		select_title ();
static void		goto_select_title ();
static void		goto_title ();
static void		visit_title ();
static void		dismiss_title ();
static void		build_search_title ();
static void		search_keyword ();
static void		select_keyword ();
static void		search_select_keyword ();
static void		select_keyword_title ();
static void		goto_select_keyword_title ();
static void		goto_keyword_title ();
static void		visit_keyword_title ();
static void		dismiss_keyword ();
static int		get_keywords ();
static void		build_search_keyword ();
static Boolean		check_library ();
static Widget		build_view_menu ();
static Widget		build_file_menu ();
static Widget		build_edit_menu ();
static Widget		build_search_menu ();
static Widget		build_help_menu ();
static void		build_menubar ();
static void		select_topic ();
static void		goto_select_topic ();
static void		build_work_area ();
static void		open_library ();
static void		set_helpshell_title ();
static void		init_help ();
static void		Initialize ();
static void		change_widget ();
static void		load_geo ();
static void		find_menubar_height ();
static void		change_help_size ();
static void		Layout ();
static void		Realize ();
static void		free_help ();
static void		Destroy ();
static void		Resize ();
static void		clear_help ();
static void		update_label ();
static void		update_title ();
static Boolean		SetValues ();
static XtGeometryResult geometry_manager ();
static void		set_changed ();
static void 		HwExposeCheck();
static XtCallbackProc   close_call_from_WM();


/*
** Static initialization of the menu widget class record, must do each field
*/
    
externaldef(dxmhelpwidgetclassrec) DXmHelpClassRec dxmHelpClassRec = {

    /*
    ** Core Class record
    */
    
   {
    (WidgetClass) &xmManagerClassRec,	/* superclass         		    */
    "DXmHelp",				/* class_name  			    */
    sizeof (DXmHelpWidgetRec),		/* size of Help widget instance	    */
    NULL,				/* class init proc		    */
    NULL,				/* class part init proc		    */
    FALSE,				/* Class is not initialised	    */
    Initialize,				/* Widget init proc		    */
    NULL,				/* initialize_hook		    */
    Realize,				/* Widget realise proc		    */
    NULL,				/* Class Action Table		    */
    NULL,				/* action count			    */
    DXmHelpResources,			/* this class's resource list	    */
    XtNumber (DXmHelpResources),	/*  "	  " resource_count	    */
    NULLQUARK,				/* xrm_class			    */
    TRUE,				/* do compressed motion		    */
    TRUE,				/* do compressed exposure	    */
    TRUE,				/* do compressed enterleave	    */
    TRUE,				/* do VisibilityNotify		    */
    Destroy,				/* class destroy proc		    */
    Resize,				/* class resize proc		    */
    HwExposeCheck,			/* class expose proc		    */
    SetValues,				/* class set_value proc		    */
    NULL,				/* set_values_hook		    */
    XtInheritSetValuesAlmost,		/* set_values_almost		    */
    NULL,				/* get_values_hook		    */
    NULL,				/* class accept focus proc	    */
    XtVersion,				/* version			    */
    NULL,				/* callback_private		    */
    NULL,				/* tm_table			    */
    NULL,				/* query geometry widget proc	    */
    NULL,				/* display accelerator		    */
    NULL,				/* extension			    */
    },
    /*
    ** Composite Class record
    */

   {geometry_manager,			/* childrens geo mgr proc	    */
    set_changed,			/* set changed proc		    */
    (XtWidgetProc) add_kid,		/* add a child			    */
    remove_child,			/* remove a child		    */
    NULL,				/* extension			    */
    },

   {		/* constraint_class fields */
      NULL,					/* resource list        */   
      0,					/* num resources        */   
      0,					/* constraint size      */   
      NULL,					/* init proc            */   
      NULL,					/* destroy proc         */   
      NULL,					/* set values proc      */   
      NULL,                                     /* extension            */
   },

   {		/* manager_class fields */
      XtInheritTranslations,			/* default translations   */
      NULL,					/* get resources      	  */
      0, 					/* num get_resources 	  */
      NULL,					/* get_cont_resources     */
      0,					/* num_get_cont_resources */
      XmInheritParentProcess,			/* parent_process	  */
      NULL,					/* extension		  */
   },

    
    /*
    ** DXmHelp Class record
    */

   {
	NULL
   }					/* extension			    */

} ;

externaldef(dxmhelpwidgetclass) WidgetClass dxmHelpWidgetClass = (WidgetClass) &dxmHelpClassRec ;


static Cursor HwCreateWaitCursor(w)
    Widget w;
{
    return ( (Cursor)DXmCreateCursor(w, DXm_WAIT_CURSOR) );
}
    

static void HwDisplayWaitCursor(w, cursor)
    Widget  w;
    Cursor  cursor;
    
    {

    XDefineCursor(XtDisplay (w), XtWindow (w), cursor);
    XFlush(XtDisplay (w));

    }


static void HwDisplayAllWaitCursor(w)
    Widget  w;
    
    {
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    
    HwDisplayWaitCursor(help_widget, help_widget->dxmhelp.cursor);

    if (help_widget->dxmhelp.history_box != NULL)
	if (XtIsManaged(help_widget->dxmhelp.history_box->box))
	    HwDisplayWaitCursor(help_widget->dxmhelp.history_box->box,
		help_widget->dxmhelp.cursor);

    if (help_widget->dxmhelp.title_box != NULL)
	if (XtIsManaged(help_widget->dxmhelp.title_box->box))
	    HwDisplayWaitCursor(help_widget->dxmhelp.title_box->box,
		help_widget->dxmhelp.cursor);

    if (help_widget->dxmhelp.keyword_box != NULL)
	if (XtIsManaged(help_widget->dxmhelp.keyword_box->box))
	    HwDisplayWaitCursor(help_widget->dxmhelp.keyword_box->box,
		help_widget->dxmhelp.cursor);

    }


static void HwRemoveWaitCursor(w)
   Widget  w;
   
    {

    XUndefineCursor(XtDisplay (w), XtWindow (w));

    }


static void HwRemoveAllWaitCursor(w)
    Widget  w;
    
    {
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    
    HwRemoveWaitCursor(help_widget);

    if (help_widget->dxmhelp.history_box != NULL)
	if (XtIsManaged(help_widget->dxmhelp.history_box->box))
	    HwRemoveWaitCursor(help_widget->dxmhelp.history_box->box);

    if (help_widget->dxmhelp.title_box != NULL)
	if (XtIsManaged(help_widget->dxmhelp.title_box->box))
	    HwRemoveWaitCursor(help_widget->dxmhelp.title_box->box);

    if (help_widget->dxmhelp.keyword_box != NULL)
	if (XtIsManaged(help_widget->dxmhelp.keyword_box->box))
	    HwRemoveWaitCursor(help_widget->dxmhelp.keyword_box->box);

    }
    

static void HwRaiseWindow(dialog_box)
    Widget dialog_box;
    {
    Widget shell;

    shell = XtParent(dialog_box);
    XRaiseWindow(XtDisplay(shell), XtWindow(shell));

    return;
    }
        

static void add_kid (child)
    Widget child;
/*
**++
**  Functional Description:
**	Add a child to the Help widget. 
**
**  Keywords:
**	Composite widget
**
**  Arguments:
**	child : widget to insert in the help widget's children array
**   
**  Side effects:
**	None
**   
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    /*
    ** Call the superclass's insert_child operation.
    */
    (* ((CompositeWidgetClass)
    compositeWidgetClass)->composite_class.insert_child)(child);

}

static void remove_child (child)
    Widget    child;
/*
**++
**  Functional Description:
**	Delete a child from the help widget
**
**  Keywords:
**	Composite widget
**
**  Arguments:
**	child: widget to remove from the help widget's children array
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    /*
    ** Call the superclass's delete_child operation.
    */
    (* ((CompositeWidgetClass)
    compositeWidgetClass)->composite_class.delete_child) (child);
}

static void free_array (array, count)
    char **array;
    int count;
/*
**++
**  Functional Description:
**	Deallocate the string array.
**
**  Keywords:
**	Memory management
**
**  Arguments:
**	array: pointer to the string array to deallocate.
**	count: size of the array.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    int i;

    if (array != NULL) {
	for (i = 0; i < count;i++)
	    XtFree(array[i]);
	XtFree((char *)array);
    }
}

static void insert_trail (help_widget, topic)
    DXmHelpWidget help_widget;
    char *topic;
/*
**++
**  Functional Description:
**	Insert the given topic on top of the trail list.
**
**  Keywords:
**	Trail management
**
**  Arguments:
**	help_widget: pointer to the help widget's record.
**	topic : topic to push on top of the trail list.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpTrail *p = NULL;

    /*
    ** Allocate the trail record.
    */
    p = (DXmHelpTrail *) XtMalloc (sizeof(DXmHelpTrail));

    /*
    ** Initialize the trail record.
    */
    if (topic != NULL)
	p->topic = (char *)XmStringCopy((XmString) topic);
    else
	p->topic = NULL;
    p->title = NULL;

    /*
    ** Insert the record at the beginning of the trail list.
    */
    if (help_widget->dxmhelp.trail == NULL)
	p->next = NULL;
    else
	p->next = help_widget->dxmhelp.trail;
    help_widget->dxmhelp.trail = p;
}

static void remove_trail(help_widget)
    DXmHelpWidget help_widget;
/*
**++
**  Functional Description:
**	Remove the top topic from the trail list.
**
**  Keywords:
**	Trail management.
**
**  Arguments:
**	help_widget : pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpTrail *p = NULL;

    /*
    ** If the trail list isn't empty, remove the first record from the list
    ** and deallocate the memory space for it.
    */
    p = help_widget->dxmhelp.trail;
    if (p != NULL)
    {
	if (p->next != NULL)
	    help_widget->dxmhelp.trail = p->next;
	else
	    help_widget->dxmhelp.trail = NULL;
	XtFree(p->topic);
	XtFree(p->title);
	XtFree((char *)p);
    }
}

static void free_trail(help_widget)
    DXmHelpWidget help_widget;
/*
**++
**  Functional Description:
**	Free the trail list.
**
**  Keywords:
**	Trail management.
**
**  Arguments:
**	help_widget: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpTrail *p, *q;

    /*
    ** Loop on the trail list to the end and free each trail record.
    */
    p = help_widget->dxmhelp.trail;
    q = help_widget->dxmhelp.trail;
    while (p != NULL)
    {
	XtFree(q->topic);
        XtFree(q->title);
	p = q->next;
	XtFree((char *)q);
	q = p;
    }
    help_widget->dxmhelp.trail = NULL;
}


static void check_message (w, parent)
    Widget w;
    Widget parent;
/*
**++
**  Functional Description:
**	If there is an error message to display, update the message box widget
**	and map it.
**
**  Keywords:
**	Error handling.
**
**  Arguments:
**	w: pointer to the widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    XtCallbackRec ok_callback[2];
    XtCallbackRec help_callback[2];
    _MsgVecInstance message_vec;
    Arg arglist[20];
    int ac;
    Widget message;

    if (help_widget->dxmhelp.message)
	{

	ok_callback[0].callback = (XtCallbackProc) error_ok_cb;
	ok_callback[0].closure = (XtPointer) help_widget;
	ok_callback[1].callback = NULL;

	help_callback[0].callback = (XtCallbackProc) error_help_cb;
	help_callback[0].closure = (XtPointer) help_widget;
	help_callback[1].callback = NULL;

	message_vec.num_words = 3;
	message_vec.message = help_widget->dxmhelp.message_text;
	message_vec.fao_count = 1;
	message_vec.fao_parameter = help_widget->dxmhelp.message_parameter;
    
	message = help_widget->dxmhelp.message_box;
	
	/*
	**  Create or reuse the CS message widget
	*/

#define USE_DISPLAYCSMESSAGE
#ifdef USE_DISPLAYCSMESSAGE
	DXmDisplayCSMessage(parent, "Message", TRUE, (Position) 0,
	    (Position) 0, XmDIALOG_APPLICATION_MODAL, (long *) &message_vec, &message, NULL,
	    (XtCallbackRec *) ok_callback, (XtCallbackRec *) help_callback);
	    
	/*
	**  Set the appropriate label and font for the OK button
	*/

	ac = 0;
	XtSetArg (arglist[ac], XmNokLabelString,
	    help_widget->dxmhelp.acknowledge_label); ac++;
	XtSetArg (arglist[ac], XmNbuttonFontList,
	    help_widget->dxmhelp.button_font_list); ac++;
	XtSetArg (arglist[ac], XmNlabelFontList,
	    help_widget->dxmhelp.label_font_list); ac++;
	XtSetArg (arglist[ac], XmNtextFontList,
	    help_widget->dxmhelp.text_font_list); ac++;
#ifdef RTOL
	XtSetArg (arglist[ac], DXmNlayoutDirection,
	    LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNnoResize, TRUE);
        ac++;
	XtSetValues (message, arglist, ac);

#else

	if (!message)
	{
	    /* 	
	     *  Create the message box widget
	     */
	    ac = 0;
	    XtSetArg (arglist[ac], XmNdefaultPosition, TRUE);
	    ac++;
	    XtSetArg (arglist[ac], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
	    ac++;
	    XtSetArg (arglist[ac], XmNmessageString, help_widget->dxmhelp.message_text);
	    ac++;
	    XtSetArg (arglist[ac], XmNokCallback, ok_callback);
	    ac++;
	    XtSetArg (arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	    XtSetArg (arglist[ac], XmNokLabelString, help_widget->dxmhelp.acknowledge_label);
	    ac++;
	    XtSetArg (arglist[ac], XmNbuttonFontList,
	        help_widget->dxmhelp.button_font_list); ac++;
	    XtSetArg (arglist[ac], XmNlabelFontList,
	        help_widget->dxmhelp.label_font_list); ac++;
	    XtSetArg (arglist[ac], XmNtextFontList,
	        help_widget->dxmhelp.text_font_list); ac++;
#ifdef RTOL
	    XtSetArg (arglist[ac], DXmNlayoutDirection,
	        LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
	    XtSetArg (arglist[ac], XmNnoResize, TRUE);
            ac++;
	    message = XmCreateMessageDialog(parent, "Message", arglist, ac);
	}
	else
	{
	    ac = 0;
	    XtSetArg (arglist[ac], XmNmessageString, help_widget->dxmhelp.message_text);
	    ac++;
	    XtSetArg (arglist[ac], XmNokLabelString, help_widget->dxmhelp.acknowledge_label);
	    ac++;
	    XtSetArg (arglist[ac], XmNbuttonFontList,
	        help_widget->dxmhelp.button_font_list); ac++;
	    XtSetArg (arglist[ac], XmNlabelFontList,
	        help_widget->dxmhelp.label_font_list); ac++;
	    XtSetArg (arglist[ac], XmNtextFontList,
	        help_widget->dxmhelp.text_font_list); ac++;
#ifdef RTOL
	    XtSetArg (arglist[ac], DXmNlayoutDirection,
	        LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
	    XtSetValues (message, arglist, ac);
	}
#endif

	help_widget->dxmhelp.message_box = message;

	XtManageChild(message);

 	if (XtIsManaged(message))
	    HwRaiseWindow(message);
	}
}

static void error_ok_cb(button, w)
    Widget button;
    Widget w;

    {
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    XtUnmanageChild(help_widget->dxmhelp.message_box);
    XtDestroyWidget(help_widget->dxmhelp.message_box);
    help_widget->dxmhelp.message_box = NULL;
    
    }


static void error_help_cb(button, w)
    Widget button;
    Widget w;
    
    {
    XmString topic;

    topic = XmStringLtoRCreate("csh error", "ISO8859-1");

    help_on_help(w, topic);

    XtFree((char *)topic);
    
    }


static void set_message(hw, msg, msg_param)
    DXmHelpWidget hw;
    XmString msg;
    XmString msg_param;
/*
**++
**  Functional Description:
**	loads a message and a message params in the message vector.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	hw: pointer to the help widget's record
**	msg: the message to be displayed
**	msg_param: the message parameter to be substituted in the message
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
    {
    /*
    **  Free the previous message if necessary
    */
    
    if (hw->dxmhelp.message_text != NULL)
	XtFree((char *)hw->dxmhelp.message_text);

    if (hw->dxmhelp.message_parameter != NULL)
	XtFree((char *)hw->dxmhelp.message_parameter);

    /*
    **  Load the message text into the message field
    */
    
    hw->dxmhelp.message_text = XmStringCopy(msg);

    if (msg_param != NULL)
	hw->dxmhelp.message_parameter = XmStringCopy(msg_param);
    else
	hw->dxmhelp.message_parameter = NULL;

    /*
    **  Set the flag indicating that a message has to be displayed
    */
    
    hw->dxmhelp.message = TRUE;

    return;
    }
    

static void pulling_view (menu, w)
    Widget menu;
    Widget w;
/*
**++
**  Functional Description:
**	Pulling callback routine for the view menu: check if the library context
**	isn't null, else set to insensitive the menu items.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	menu: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** Set the button to insensitive if the overview topic wasn't specified or
    ** the library context is null.
    */
    if ((help_widget->dxmhelp.library_context != NULL) &&
	(help_widget->dxmhelp.overview != NULL))
        XtSetSensitive (help_widget->dxmhelp.overview_button, TRUE);
    else
        XtSetSensitive (help_widget->dxmhelp.overview_button, FALSE);
	
    /*
    ** If the library context and the glossary topic are not null, set the
    ** glossary button to sensitive if it exists.
    */
    if (help_widget->dxmhelp.glossary_button != NULL)
    {
	if ((help_widget->dxmhelp.library_context != NULL) &&
	    (help_widget->dxmhelp.glossary != NULL))
	    XtSetSensitive (help_widget->dxmhelp.glossary_button, TRUE);
	else
	    XtSetSensitive (help_widget->dxmhelp.glossary_button, FALSE);
    }
    
    /*
    ** If the trail list contains more than one topic and the library context
    ** isn't null, set the goback button to sensitive.
    */
    if ((help_widget->dxmhelp.trail != NULL) &&
	(help_widget->dxmhelp.trail->next != NULL) &&
	(help_widget->dxmhelp.library_context != NULL))
	XtSetSensitive (help_widget->dxmhelp.goback_button, TRUE);
    else
	XtSetSensitive (help_widget->dxmhelp.goback_button, FALSE);
}

static int get_string_from_cs(cs, text)
    XmString cs;
    char **text;
    
/*
**++
**  Functional Description:
**	Extract the text string from the given compound string.
**
**  Keywords:
**	Help create, update
**
**  Arguments:
**	cs: compound string.
**	text: text string to return.
**
**  Result:
**	Integer: TRUE or FALSE.
**
**  Exceptions:
**	None
**--
*/
{
    int  l_rout_status;
    long l_size,l_status;
    char *textp;

    textp = NULL;

    textp = (char *) DXmCvtCStoOS(cs,&l_size,&l_status);
    if (textp != NULL)
        {
	*text = textp;
	l_rout_status = TRUE;
        }
    else
        {
	*text =NULL;
	l_rout_status = FALSE;
        };

    return (l_rout_status);
}

static void get_help (help_widget)
    DXmHelpWidget help_widget;
/*
**++
**  Functional Description:
**	Get the selected topic frame, update the text widget and the list box.
**
**  Keywords:
**	Help update
**
**  Arguments:
**	help_widget: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    char *title_ptr = NULL;
    char *text_ptr = NULL;
    char **topic_save = NULL;
    char **title_save = NULL;
    int count_save = 0;
    char *text_save = NULL;
    Arg arglist[10];
    Arg textarg[3];
    int ac = 0;
    int status = 0;
    char *topic;
    int i;
#ifdef GENERAL_TEXT
    char *help_text_buf = NULL;
    long count,stat;
#endif
    /*
    ** Save the previous title array, topic array, topic count and topic text.
    */
    title_save = help_widget->dxmhelp.title_array;
    topic_save = help_widget->dxmhelp.topic_array;
    count_save = help_widget->dxmhelp.topic_count;
    text_save = (char *)help_widget->dxmhelp.text;

    /*
    ** Get the information associated with the topic stored on top of
    ** the trail. If the trail is empty, get the list of all toplevel topics.
    */
    if (help_widget->dxmhelp.library_context != NULL)
    {
	if ((help_widget->dxmhelp.trail != NULL) &&
	    (help_widget->dxmhelp.trail->topic != NULL))
	{
	    /*
	    ** Extract the topic string from the compound string.
	    */
	    if (get_string_from_cs (help_widget->dxmhelp.trail->topic, &topic)
				!= TRUE)
		topic = NULL;
	    else
	    
		/*
		**  Test for an empty C string
		*/
		
		if (strlen(topic) <= 0) {
		    XtFree(topic);
		    topic= NULL;
		}
	}
	else
	    topic = NULL;
	
	/*
	** Get the frame and the list of subtopics from the help
	** library.
	*/
	status =
	    DXmHelp__get_frame(help_widget->dxmhelp.library_context,
			topic, &title_ptr, &text_ptr,
			&help_widget->dxmhelp.topic_array,
			&help_widget->dxmhelp.title_array,
			&help_widget->dxmhelp.topic_count); 
    
	if (VMS_STATUS_SUCCESS(status)== STS$K_SUCCESS)
	{
	    /*
	    ** If the text and title returned are NULL initialize them to a
	    ** blanc string.
	    */
	    if (text_ptr == NULL)
		text_ptr = " ";
	    if (title_ptr == NULL)
		title_ptr = " ";

	    /*
	    ** Allocate the help text memory space: the help text will
	    ** contain the topic title, two blanc lines and the topic
	    ** text.
	    */
#ifdef CSTEXT
            help_text_buf = XtMalloc  (strlen (title_ptr)
                                        + strlen ("\n\n")
                                        + strlen(text_ptr)
                                        + 1);
            strcpy (help_text_buf, title_ptr);
            strcat (help_text_buf, "\n\n");
            strcat (help_text_buf, text_ptr);
            help_widget->dxmhelp.text =
		 (XmString) DXmCvtFCtoCS((Opaque) help_text_buf, &count, &stat);
	    XtFree( help_text_buf) ;

#else

	    help_widget->dxmhelp.text = XtMalloc  (strlen (title_ptr)
						+ strlen ("\n\n")
						+ strlen(text_ptr)
						+ 1);
	    strcpy (help_widget->dxmhelp.text, title_ptr);
	    strcat (help_widget->dxmhelp.text, "\n\n");
	    strcat (help_widget->dxmhelp.text, text_ptr);
#endif  /* CSTEXT */

	    /*
	    ** Check if the trail list isn't empty.
	    */
	    if (help_widget->dxmhelp.trail != NULL)
	    {
		/*
		** If the trail record on top of the trail list has an
		** empty title, initialize it.
		*/
		if (help_widget->dxmhelp.trail->title == NULL)
		    help_widget->dxmhelp.trail->title = 
#ifdef GENERAL_TEXT
			(char *) DXmCvtFCtoCS( (Opaque) title_ptr,&count,&stat);
#else
			(char *) XmStringLtoRCreate(title_ptr, "ISO8859-1");
#endif /* GENERAL_TEXT */

		/*
		** If the topic isn't null add it to the history list else
		** set the history.
		*/
		if (help_widget->dxmhelp.trail->topic != NULL)
		    add_history (help_widget);
		else
		    set_history (help_widget, FALSE);
	    }
	    /*
	    ** Set the error flag to false.
	    */
	    help_widget->dxmhelp.message = FALSE;		    
	}
	else
	/*
	** Error in getting the frame and subtopics associated to
	** the topic. 
	*/
	{
	    set_message(help_widget, help_widget->dxmhelp.badframe_message,
		help_widget->dxmhelp.trail->topic);

	    /*
	    ** Remove the topic from the top of the trail list.
	    */
	    remove_trail (help_widget);

	}
	/*
	** Free the string returned for the topic.
	*/
	if (topic != NULL)
	    XtFree(topic);
    }
    else
    /*
    ** Error: library context set to null means there is a problem with the
    ** library.
    */
    {
	/*
	** If a library was specified, means it couldn't be open.
	*/
	if (help_widget->dxmhelp.library != NULL)
	{

	    set_message(help_widget, help_widget->dxmhelp.badlib_message,
		help_widget->dxmhelp.library);

	}   
	else
	/*
	** There wasn't any library specified.
	*/
	{

	    set_message(help_widget, help_widget->dxmhelp.nulllib_message,
		NULL);

	}
    }

    /*
    ** If the error flag is false, update the text and the listbox widgets.
    */
    if (!help_widget->dxmhelp.message)
    {
	/*
	** Unmanage the additional topics label and the listbox.
	*/
	XtUnmanageChild(help_widget->dxmhelp.add_topic);
	XtUnmanageChild(XtParent(help_widget->dxmhelp.help_topic));

	/*
	** Clear the selection in the text widget and update its contents.
	*/
#ifdef CSTEXT
	DXmCSTextDisableRedisplay (help_widget->dxmhelp.help_text, FALSE);
	DXmCSTextClearSelection (help_widget->dxmhelp.help_text, CurrentTime);
	DXmCSTextSetString (help_widget->dxmhelp.help_text,
			    help_widget->dxmhelp.text);
	DXmCSTextSetTopPosition (help_widget->dxmhelp.help_text, 0);
	DXmCSTextEnableRedisplay (help_widget->dxmhelp.help_text);
#else
	_XmTextDisableRedisplay (help_widget->dxmhelp.help_text, FALSE);
	XmTextClearSelection (help_widget->dxmhelp.help_text, CurrentTime);
	XmTextSetString (help_widget->dxmhelp.help_text,
			    help_widget->dxmhelp.text);
	XmTextSetTopCharacter (help_widget->dxmhelp.help_text, 0);
	_XmTextEnableRedisplay (help_widget->dxmhelp.help_text);
#endif /* CSTEXT */

	/*
	** Update the listbox content's.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNitemCount, help_widget->dxmhelp.topic_count);
	ac++;
	XtSetArg (arglist[ac], XmNitems, help_widget->dxmhelp.title_array);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
        ac++;
#endif /* RTOL */
	XtSetValues (help_widget->dxmhelp.help_topic, arglist, ac);
        /* Need to pre-def a selection of some sort for DefaultAction =kbm */
        XmListSelectPos (help_widget->dxmhelp.help_topic,
            (((XmListWidget )help_widget->dxmhelp.help_topic)->list.itemCount 
            > 1 ? 1 : 0), False);
        XmListSetPos (help_widget->dxmhelp.help_topic, 1); /* set scroll */
	/*
	** If there's a preselected topic and it exists in the listbox, tell the
	** listbox to select it (highlight it). Set the goto and visit buttons
	** to be sensitive in this case.
	*/
	if ((help_widget->dxmhelp.selected_topic != NULL)&&
	    (XmListItemExists(help_widget->dxmhelp.help_topic,
				  help_widget->dxmhelp.selected_title)))
	{
	    
	    ac = 0;
	    XtSetArg (arglist[ac], XmNselectedItems,
			    &help_widget->dxmhelp.selected_title);
	    ac++;
	    XtSetArg (arglist[ac], XmNselectedItemCount, 1);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.help_topic, arglist, ac);
	    XtSetSensitive (help_widget->dxmhelp.goto_button, TRUE);
	    XtSetSensitive (help_widget->dxmhelp.visit_button, TRUE);
	    
	}
	else
	/*
	** There isn't any topic preselected so set the goto and visit buttons
	** to insensitive.
	*/
	{
         /* Need to pre-def a selection of some sort for DefaultAction =kbm */
            XmListSelectPos (help_widget->dxmhelp.help_topic,
             (((XmListWidget )help_widget->dxmhelp.help_topic)->list.itemCount
                > 1 ? 1 : 0), 
                False);
            XmListSetPos (help_widget->dxmhelp.help_topic, 1); /* set scroll */
	    XtSetSensitive (help_widget->dxmhelp.goto_button, FALSE);
	    XtSetSensitive (help_widget->dxmhelp.visit_button, FALSE);
	}
	/*
	** Free the previously saved arrays and the saved text.
	*/
	free_array (title_save,count_save);
	free_array (topic_save,count_save);
        XtFree(text_save);
	    
	/*
	** If there are subtopics, manage the additional topic label and the
	** listbox.
	*/    
        if (help_widget->dxmhelp.topic_count != 0)
	{
	    XtManageChild (help_widget->dxmhelp.add_topic);
	    XtManageChild (help_widget->dxmhelp.help_topic);
	    XtManageChild (XtParent(help_widget->dxmhelp.help_topic));
	}
    }
    else
    /*
    ** Pop up the error message box.
    */
	if (XtIsRealized(help_widget))
	    check_message (help_widget, help_widget);

    /*
    ** If the trail list contains more than one topic and the library_context
    ** isn't null, set the goback button to sensitive else to insensitive.
    */
    if ((help_widget->dxmhelp.trail != NULL) &&
	(help_widget->dxmhelp.trail->next != NULL) &&
	(help_widget->dxmhelp.library_context != NULL))
	XtSetSensitive (help_widget->dxmhelp.goback, TRUE);
    else
	XtSetSensitive (help_widget->dxmhelp.goback, FALSE);
}


static void go_to (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the goto button and menu item:
**	update the help widget with the selected topic. 
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    HwDisplayAllWaitCursor(help_widget);
    
    if (help_widget->dxmhelp.selected_topic!= NULL)
    {
	/*
	** Insert the selected topic in the trail list.
	*/
	insert_trail(help_widget, help_widget->dxmhelp.selected_topic);
	/*
	** Free the selected topic and title and set them to null, so that after
	** the update of the listbox there isn't any preselected topic in the
	** listbox.
	*/
	XtFree ((char *)help_widget->dxmhelp.selected_topic);
	help_widget->dxmhelp.selected_topic = NULL;
	XtFree ((char *)help_widget->dxmhelp.selected_title);
	help_widget->dxmhelp.selected_title = NULL;

	/*
	** Get the help frame associated with the topic on top of the trail list
	** and update the help text and lisbox.
	*/
	get_help(help_widget);
    }
    
    if (XtIsManaged(help_widget))
	HwRaiseWindow(help_widget);

    HwRemoveAllWaitCursor(help_widget);
}
    

static void get_help_arguments (w, arglist, argcount)
    Widget w;
    Arg *arglist;
    int *argcount;
/*
**++
**  Functional Description:
**	Loads all help widget arguments which should be propagated.
**
**  Keywords:
**	Help create.
**
**  Arguments:
**	w: pointer to the help widget's record.
**	arglist: arguments array.
**	argcount: number of arguments.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    int ac = 0;
    
    XtSetArg (arglist[ac], DXmNcols,
	    help_widget->dxmhelp.colons); ac++;
    XtSetArg (arglist[ac], DXmNrows,
	    help_widget->dxmhelp.rows); ac++;
    XtSetArg (arglist[ac], XmNbuttonFontList,
	    help_widget->dxmhelp.button_font_list); ac++;
    XtSetArg (arglist[ac], XmNlabelFontList,
	    help_widget->dxmhelp.label_font_list); ac++;
    XtSetArg (arglist[ac], XmNtextFontList,
	    help_widget->dxmhelp.text_font_list); ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection,
	          LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNstringDirection,
	    help_widget->dxmhelp.string_direction); ac++;
    XtSetArg (arglist[ac], DXmNviewLabel,
	    help_widget->dxmhelp.view_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNgototopicLabel,
	    help_widget->dxmhelp.goto_topic_label); ac++;
    XtSetArg (arglist[ac], DXmNgobacktopicLabel,
	    help_widget->dxmhelp.goback_topic_label); ac++;
    XtSetArg (arglist[ac], DXmNgooverLabel,
	    help_widget->dxmhelp.goover_label); ac++;
    XtSetArg (arglist[ac], DXmNvisittopicLabel,
	    help_widget->dxmhelp.visit_topic_label); ac++;
    XtSetArg (arglist[ac], DXmNvisitglosLabel,
	    help_widget->dxmhelp.visitglos_label); ac++;
    XtSetArg (arglist[ac], DXmNfileLabel,
	    help_widget->dxmhelp.file_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNsaveasLabel,
	    help_widget->dxmhelp.saveas_label); ac++;
    XtSetArg (arglist[ac], DXmNexitLabel,
	    help_widget->dxmhelp.exit_label); ac++;
    XtSetArg (arglist[ac], DXmNeditLabel,
	    help_widget->dxmhelp.edit_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNcopyLabel,
	    help_widget->dxmhelp.copy_label); ac++;
    XtSetArg (arglist[ac], DXmNselectallLabel,
	    help_widget->dxmhelp.selectall_label); ac++;
    XtSetArg (arglist[ac], DXmNsearchLabel,
	    help_widget->dxmhelp.search_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNhistoryLabel,
	    help_widget->dxmhelp.history_label); ac++;
    XtSetArg (arglist[ac], DXmNtitleLabel,
	    help_widget->dxmhelp.title_label); ac++;
    XtSetArg (arglist[ac], DXmNkeywordLabel,
	    help_widget->dxmhelp.keyword_label); ac++;
    XtSetArg (arglist[ac], DXmNhelpLabel,
	    help_widget->dxmhelp.help_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNhelphelpLabel,
	    help_widget->dxmhelp.helphelp_label); ac++;

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNglossaryLabel,
	    help_widget->dxmhelp.glossary_label); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNaboutLabel,
	    help_widget->dxmhelp.about_label); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNoncontextLabel,
	    help_widget->dxmhelp.oncontext_label); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtSetArg (arglist[ac], DXmNaddtopicLabel,
	    help_widget->dxmhelp.addtopic_label); ac++;
    XtSetArg (arglist[ac], DXmNgobackLabel,
	    help_widget->dxmhelp.goback_label); ac++;
    XtSetArg (arglist[ac], DXmNcloseLabel,
	    help_widget->dxmhelp.quit_label); ac++;
    XtSetArg (arglist[ac], DXmNgotoLabel,
	    help_widget->dxmhelp.goto_label); ac++;
    XtSetArg (arglist[ac], DXmNvisitLabel,
	    help_widget->dxmhelp.visit_label); ac++;
    XtSetArg (arglist[ac], DXmNsearchapplyLabel,
	    help_widget->dxmhelp.apply_label); ac++;
    XtSetArg (arglist[ac], DXmNdismissLabel,
	    help_widget->dxmhelp.dismiss_label); ac++;
    XtSetArg (arglist[ac], DXmNtopictitlesLabel,
	    help_widget->dxmhelp.topic_titles_label); ac++;
    XtSetArg (arglist[ac], DXmNhistoryboxLabel,
	    help_widget->dxmhelp.history_box_label); ac++;
    XtSetArg (arglist[ac], DXmNsearchtitleboxLabel,
	    help_widget->dxmhelp.searchtitle_box_label); ac++;
    XtSetArg (arglist[ac], DXmNtitlesLabel,
	    help_widget->dxmhelp.titles_label); ac++;
    XtSetArg (arglist[ac], DXmNsearchkeywordboxLabel,
	    help_widget->dxmhelp.searchkeyword_box_label); ac++;
    XtSetArg (arglist[ac], DXmNkeywordsLabel,
	    help_widget->dxmhelp.keywords_label); ac++;
    XtSetArg (arglist[ac], DXmNbadlibMessage,
	    help_widget->dxmhelp.badlib_message); ac++;
    XtSetArg (arglist[ac], DXmNbadframeMessage,
	    help_widget->dxmhelp.badframe_message); ac++;
    XtSetArg (arglist[ac], DXmNnulllibMessage,
	    help_widget->dxmhelp.nulllib_message); ac++;
    XtSetArg (arglist[ac], DXmNnotitleMessage,
	    help_widget->dxmhelp.notitle_message); ac++;
    XtSetArg (arglist[ac], DXmNnokeywordMessage,
	    help_widget->dxmhelp.nokeyword_message); ac++;
    XtSetArg (arglist[ac], DXmNerroropenMessage,
	    help_widget->dxmhelp.erroropen_message); ac++;
    XtSetArg (arglist[ac], DXmNhelpontitleLabel,
	    help_widget->dxmhelp.helpontitle_label); ac++;
    XtSetArg (arglist[ac], DXmNhelptitleLabel,
	    help_widget->dxmhelp.helptitle_label); ac++;
    XtSetArg (arglist[ac], XmNstringDirection,
	    help_widget->dxmhelp.string_direction); ac++;
    XtSetArg (arglist[ac], DXmNhelpAcknowledgeLabel,
	    help_widget->dxmhelp.acknowledge_label); ac++;
    XtSetArg (arglist[ac], DXmNhelpOnHelpTitle,
	    help_widget->dxmhelp.helponhelp_title); ac++;
    XtSetArg (arglist[ac], DXmNcacheHelpLibrary,
	    help_widget->dxmhelp.cache_library); ac++;


#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNaboutLabelMnem,
	    help_widget->dxmhelp.about_label_mnem); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNoncontextLabelMnem,
	    help_widget->dxmhelp.oncontext_label_mnem); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtSetArg (arglist[ac], DXmNcopyLabelMnem,
	    help_widget->dxmhelp.copy_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNeditLabelMnem,
	    help_widget->dxmhelp.edit_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNexitLabelMnem,
	    help_widget->dxmhelp.exit_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNfileLabelMnem,
	    help_widget->dxmhelp.file_label_mnem); ac++;

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNglossaryLabelMnem,
	    help_widget->dxmhelp.glossary_label_mnem); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtSetArg (arglist[ac], DXmNgooverLabelMnem,
	    help_widget->dxmhelp.goover_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNhelpLabelMnem,
	    help_widget->dxmhelp.help_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNhistoryLabelMnem,
	    help_widget->dxmhelp.history_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNkeywordLabelMnem,
	    help_widget->dxmhelp.keyword_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNsaveasLabelMnem,
	    help_widget->dxmhelp.saveas_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNsearchLabelMnem,
	    help_widget->dxmhelp.search_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNselectallLabelMnem,
	    help_widget->dxmhelp.selectall_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNtitleLabelMnem,
	    help_widget->dxmhelp.title_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNviewLabelMnem,
	    help_widget->dxmhelp.view_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNvisitglosLabelMnem,
	    help_widget->dxmhelp.visitglos_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNgototopicLabelMnem,
	    help_widget->dxmhelp.goto_topic_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNgobackLabelMnem,
	    help_widget->dxmhelp.goback_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNvisittopicLabelMnem,
	    help_widget->dxmhelp.visit_topic_label_mnem); ac++;
    XtSetArg (arglist[ac], DXmNhelphelpLabelMnem,
	    help_widget->dxmhelp.helphelp_label_mnem); ac++;


#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNaboutLabelMnemCS,
	    help_widget->dxmhelp.about_label_mnem_cs); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNoncontextLabelMnemCS,
	    help_widget->dxmhelp.oncontext_label_mnem_cs); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtSetArg (arglist[ac], DXmNcopyLabelMnemCS,
	    help_widget->dxmhelp.copy_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNeditLabelMnemCS,
	    help_widget->dxmhelp.edit_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNexitLabelMnemCS,
	    help_widget->dxmhelp.exit_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNfileLabelMnemCS,
	    help_widget->dxmhelp.file_label_mnem_cs); ac++;

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNglossaryLabelMnemCS,
	    help_widget->dxmhelp.glossary_label_mnem_cs); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtSetArg (arglist[ac], DXmNgooverLabelMnemCS,
	    help_widget->dxmhelp.goover_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNhelpLabelMnemCS,
	    help_widget->dxmhelp.help_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNhistoryLabelMnemCS,
	    help_widget->dxmhelp.history_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNkeywordLabelMnemCS,
	    help_widget->dxmhelp.keyword_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNsaveasLabelMnemCS,
	    help_widget->dxmhelp.saveas_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNsearchLabelMnemCS,
	    help_widget->dxmhelp.search_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNselectallLabelMnemCS,
	    help_widget->dxmhelp.selectall_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNtitleLabelMnemCS,
	    help_widget->dxmhelp.title_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNviewLabelMnemCS,
	    help_widget->dxmhelp.view_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNvisitglosLabelMnemCS,
	    help_widget->dxmhelp.visitglos_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNgototopicLabelMnemCS,
	    help_widget->dxmhelp.goto_topic_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNgobackLabelMnemCS,
	    help_widget->dxmhelp.goback_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNvisittopicLabelMnemCS,
	    help_widget->dxmhelp.visit_topic_label_mnem_cs); ac++;
    XtSetArg (arglist[ac], DXmNhelphelpLabelMnemCS,
	    help_widget->dxmhelp.helphelp_label_mnem_cs); ac++;
    XtSetArg (arglist[ac],DXmNdialogStyle,
	    help_widget->dxmhelp.dialog_style); ac++;

    *argcount = ac + *argcount;
}

static void visit_help (w, topic, library)
    Widget w;
    char *topic;
    char *library;
/*
**++
**  Functional Description:
**	Create a new help widget or reuse and update an existing one.
**
**  Keywords:
**	Help update.
**
**  Arguments:
**	w: pointer to the help widget's record.
**	topic: topic string to use as first topic.
**	library: library specification.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    Arg arglist[115];
    int ac = 0;
    Widget shell;

    /*
    ** If the given library is different from the Help on Help library,
    ** create a help widget or recycle a help widget from the visit context.
    ** Map the help widget.
    */
    if (check_library(library))
    {
	if (help_widget->dxmhelp.visit_context == NULL)
	    HelpSetup (&help_widget->dxmhelp.visit_context,
			  help_widget,
			  help_widget->dxmhelp.application,
			  help_widget->dxmhelp.library_type,
			  library,
			  help_widget->dxmhelp.overview,
			  help_widget->dxmhelp.glossary);
	
	HelpDisplay (help_widget->dxmhelp.visit_context, topic);
    }
    else
    /*
    ** The library is the Help on Help library, so create or recycle the help
    ** on help widget. Map the help on help widget.
    */
    {
	if (help_widget->dxmhelp.help_on_help == NULL)
	    {
	    /*
	    ** Get the parent help widget's resources into the arglist.
	    */
	    get_help_arguments (help_widget, arglist, &ac);

	    /*
	    ** Set the help on help ressources.
	    */
	    XtSetArg (arglist[ac], DXmNlibraryType, DXmTextLibrary); ac++;
	    XtSetArg (arglist[ac], DXmNlibrarySpec, library); ac++;
	    XtSetArg (arglist[ac], DXmNapplicationName,
			help_widget->dxmhelp.helptitle_label); ac++;
	    XtSetArg (arglist[ac], DXmNfirstTopic, topic); ac++;
	    XtSetArg (arglist[ac], DXmNoverviewTopic,
			XmStringLtoRCreate("overview", "ISO8859-1")); ac++;
	    XtSetArg (arglist[ac], DXmNglossaryTopic,
			XmStringLtoRCreate("glossary", "ISO8859-1")); ac++;

	    /*
	    ** Create the help on help widget.
	    */
	    help_widget->dxmhelp.help_on_help =
		DXmCreateHelpDialog ((Widget) help_widget, "help", (ArgList) arglist, ac);
		
	    XtManageChild (help_widget->dxmhelp.help_on_help);
	    
	    }
	 else
	    {
	    /*
	    ** Set the new topic on the help on help widget.
	    */
	    XtSetArg(arglist[0], DXmNfirstTopic, topic);
	    XtSetValues(help_widget->dxmhelp.help_on_help, arglist, 1);

	    if (XtIsManaged(help_widget->dxmhelp.help_on_help))
		HwRaiseWindow(help_widget->dxmhelp.help_on_help);
	    else
		XtManageChild (help_widget->dxmhelp.help_on_help);
	    }
    }

}


static void visit (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the visit button and menu item:
**	visit the selected topic.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    HwDisplayAllWaitCursor(help_widget);

    /*
    ** Create a new help widget or reuse and update an existing one.
    */
    if (help_widget->dxmhelp.selected_topic != NULL)
	visit_help (help_widget,
		    help_widget->dxmhelp.selected_topic,
		    help_widget->dxmhelp.library);

    HwRemoveAllWaitCursor(help_widget);
    
}

static void goback (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**  	Activate callback routine for the goback button and menu item:
**	go back to the previous topic.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;


    HwDisplayAllWaitCursor(help_widget);
    
    /*
    ** If the trail list contains more than one topic, set the help widget's
    ** selected topic to be the one on top of the trail list so that it will be
    ** preselected when the help widget gets updated. Remove the top topic
    ** from the trail list. Then update the help text and listbox.
    */ 
    if (help_widget->dxmhelp.trail->next != NULL)
    {
	help_widget->dxmhelp.selected_topic =
	    XmStringCopy((XmString) help_widget->dxmhelp.trail->topic);
	help_widget->dxmhelp.selected_title =
	    XmStringCopy((XmString) help_widget->dxmhelp.trail->title);
	remove_trail(help_widget);
	get_help(help_widget);
    }

    HwRemoveAllWaitCursor(help_widget);
    
}


static void goto_overview (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the goto overview menu item: go to
**	the overview topic.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    int comp = 1;

    HwDisplayAllWaitCursor(help_widget);
    
    /*
    ** Check if the help widget isn't already set to the overview topic.
    */
    if ((help_widget->dxmhelp.trail != NULL) &&
	(help_widget->dxmhelp.trail->topic != NULL))
       comp = !XmStringCompare((XmString) help_widget->dxmhelp.trail->topic,
	        		help_widget->dxmhelp.overview);

    /*
    ** The help widget has to be updtated.
    */
    if (comp != 0)
    {
	/*
	** Insert the overview topic on top of the trail list.
	*/
        insert_trail(help_widget, help_widget->dxmhelp.overview);
	/*
	** Deallocate and set to null the selected topic, so that there isn't
	** any preselected topic when the listbox gets updated.
	*/
	XtFree ((char *)help_widget->dxmhelp.selected_topic);
	help_widget->dxmhelp.selected_topic = NULL;
	XtFree ((char *)help_widget->dxmhelp.selected_title);
	help_widget->dxmhelp.selected_title = NULL;
	/*
	** Get the help frame and update the help text and the listbox.
	*/
	get_help(help_widget);
    }

    HwRemoveAllWaitCursor(help_widget);
    
}
    

static void visit_glossary (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the visit glossary menu item:
**	visit the glossary topic.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    HwDisplayAllWaitCursor(help_widget);

    /*
    ** Create a new help widget or reuse and update an existing one.
    */
    visit_help (help_widget,
		help_widget->dxmhelp.glossary
		,help_widget->dxmhelp.library);

    HwRemoveAllWaitCursor(help_widget);

}

static void help_on_help (w, topic)
    Widget w;
    char *topic;
/*
**++
**  Functional Description:
**	Create a new help widget or reuse and update an existing one to get help
**	on the help widget.
**
**  Keywords:
**	Help update.
**
**  Arguments:
**	w: pointer to the help widget's record.
**	topic: topic string to use as first topic.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString help_library;
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    
    HwDisplayAllWaitCursor(help_widget);

    /*
    ** Set the help library.
    */
    help_library = XmStringLtoRCreate(LIBRARY, "ISO8859-1");
    /*
    ** Create a new help widget or reuse and update an existing one.
    */
    visit_help (w, topic, help_library);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)help_library);
    
    HwRemoveAllWaitCursor(help_widget);
    
}

static void help (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the help menu item: get help on help.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic to the overview topic.
    */
    topic = XmStringLtoRCreate("overview", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

#if 0	/* Remove everything except On Windows from Using Help Menu */
static void help_glossary(button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the glossary menu item: get
**	help's glossary frame.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic to the glossary topic.
    */
    topic = XmStringLtoRCreate("glossary", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree (topic);
}
#endif 	/* Remove everything except On Windows from Using Help Menu */


#if 0	/* Remove everything except On Windows from Using Help Menu */
static void help_about (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the about menu item: get help's
**	about frame.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic to the about topic.
    */
    topic = XmStringLtoRCreate("aboutframe", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree (topic);
}
#endif 	/* Remove everything except On Windows from Using Help Menu */


static void help_view (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on view menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_go", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_view_goto (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the goto item in the view
**	menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_go menus_go_goto", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_view_goback (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the goback item in the view
**	menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_go menus_go_back", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_view_overview (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the overview item in the view
**	menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_go menus_go_gotoover", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_view_visit (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the visit item in the view
**	menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_go menus_go_visit", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_view_glossary (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the glossary item in the view
**	menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_go menus_go_visitgloss", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_file (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the file menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_file", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_edit (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the edit menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_edit", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_edit_copy (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the copy item in the edit
**	menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_edit menus_edit_copy", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_edit_selectall (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the selectall item in
**	the edit menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_edit menus_edit_select", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_search (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the search menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_search", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_search_history (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the history item in the
**	search menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_search menus_search_history", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_search_title (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the title item in the search
**	menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_search menus_search_title", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_search_keyword (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the keyword item in the
**	search menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_search menus_search_keyword", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_help (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the help menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_help", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_help_help (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the help on help item in
**	the help menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_help menus_help_help", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

#if 0	/* Remove everything except On Windows from Using Help Menu */
static void help_help_glossary (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the glossary item in the
**	help menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_help menus_help_glossary", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree (topic);
}
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
static void help_oncontext (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the On Context item in the
**	help menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_help menus_help_oncontext", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree (topic);
}
#endif 	/* Remove everything except On Windows from Using Help Menu */


#if 0	/* Remove everything except On Windows from Using Help Menu */
static void help_help_about (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the about item in the 
**	help menu.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_help menus_help_about", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree (topic);
}
#endif 	/* Remove everything except On Windows from Using Help Menu */

static void help_frame (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the help frame.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview over_guide_functions Over_guide_window over_guide_help_frame", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_listbox (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the help listbox and
**	additional topics label.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview over_guide_functions Over_guide_window over_guide_list_box", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_goback (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the goback button
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview Over_guide_functions buttons_goback", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_exit (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the exit button
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview Over_guide_functions buttons_exit", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_history (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the history dialog box.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_search menus_search_history", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_title (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the title dialog box.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_search menus_search_title", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}


static void help_keyword (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the keyword dialog box.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview menus_search menus_search_keyword", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void help_saveas (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the save as dialog box.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    Arg helparg[105];
    int ac = 0;

    /*
    ** Create a new help widget child of the save as dialog box 
    ** (because it's a modal dialog box) if not already created.
    */
    if (help_widget->dxmhelp.help_on_saveas == NULL)
    {
	/*
	** Get the help widget's arguments.
	*/
	get_help_arguments (help_widget, helparg, &ac);
	/*
	** Set the help on saveas specific arguments.
	*/
	XtSetArg (helparg [ac], DXmNlibrarySpec, XmStringLtoRCreate(LIBRARY, "ISO8859-1")); ac++;
	XtSetArg (helparg [ac], DXmNfirstTopic,
		   XmStringLtoRCreate("overview menus_file", "ISO8859-1")); ac++;
	XtSetArg (helparg [ac], DXmNapplicationName,
		   help_widget->dxmhelp.application); ac++;
	XtSetArg (helparg [ac], DXmNoverviewTopic,
			help_widget->dxmhelp.overview); ac++;
	XtSetArg (helparg [ac], DXmNglossaryTopic,
			help_widget->dxmhelp.glossary); ac++;
	XtSetArg (helparg [ac], DXmNlibraryType,
			help_widget->dxmhelp.library_type); ac++;
	XtSetArg (helparg [ac], XmNdefaultPosition, TRUE); ac++;

	help_widget->dxmhelp.help_on_saveas =
	    DXmCreateHelpDialog (help_widget->dxmhelp.saveas_box->box,
				  "help", (ArgList) helparg, ac);
    }
    
    XtManageChild (help_widget->dxmhelp.help_on_saveas);

}


#if 0	/* Remove everything except On Windows from Using Help Menu */
static void on_context (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the on_context menu item
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpOnContext(w, False);		/* get help, don't confine cursor */
}
#endif 	/* Remove everything except On Windows from Using Help Menu */


static void csh_menubar (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Context sensitive help callback: get help on the menubar.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    XmString topic;

    /*
    ** Set the first topic.
    */
    topic = XmStringLtoRCreate("overview", "ISO8859-1");
    /*
    ** Get help on help.
    */
    help_on_help (w, topic);
    /*
    ** Free the string storage.
    */
    XtFree ((char *)topic);
}

static void pulling_file (menu, w)
    Widget menu;
    Widget w;
/*
**++
**  Functional Description:
**	Pulling callback routine for the file menu: check if the library context
**	isn't null, else set to insensitive the save as menu item.
**
****  Keywords:
**	Callback routine
**
**  Arguments:
**	menu: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** If the library_context and the trail topic aren't null, set the
    ** save as item to sensitive else to insensitive.
    */
    if ((help_widget->dxmhelp.library_context != NULL) &&
	(!(NullTopicInTrail(help_widget->dxmhelp.trail))))
        XtSetSensitive (help_widget->dxmhelp.saveas_button, TRUE);
    else
        XtSetSensitive (help_widget->dxmhelp.saveas_button, FALSE);
}

static Boolean NullTopicInTrail(Trail)
    DXmHelpTrail *Trail;

{
    if (Trail == NULL)
	return(TRUE);
    else
	if (Trail->topic == NULL)
	    return(TRUE);

    return(FALSE);
}

static void ok_saveas (button, w, filesel_struct)
    Widget button;
    Widget w;
    XmFileSelectionBoxCallbackStruct *filesel_struct;
/*
**++
**  Functional Description:
**	Activate callback routine for the ok button of the save as box:
**	save the current frame in the given file.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**	filesel_struct: pointer to the file selection callback structure.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    char *filename = NULL;
    FILE *file_ptr;
    char *filenm;
#ifdef CSTEXT
    long  count,stat;
#endif /* CSTEXT */
    /*
    ** Extract the file name from the compound string returned in the file
    ** selection callback structure.
    */
    if (get_string_from_cs (filesel_struct->value,&filename ) == TRUE)
    {
#ifdef VMS
	/*
	** Create a VMS file name and open the file.
	*/
	filenm = malloc(strlen(filename) + 1);
	sscanf( filename, "%[^;]", filenm);
	file_ptr = fopen(filenm, "w", "rfm=var", "rat=cr");
	free(filenm);
#else
	/*
	** Open the file.
	*/
	file_ptr = fopen(filename, "w");
#endif
	if (file_ptr != NULL)
	{
	    /*
	    ** Write the help widget's text into the file.
	    */
#ifdef CSTEXT
	    char * help_text_buf;
            help_text_buf = (char *) DXmCvtCStoFC(help_widget->dxmhelp.text,
						  &count,&stat);
	    fprintf(file_ptr, "%s",help_text_buf);
            XtFree(help_text_buf) ;
#else
	    fprintf(file_ptr, "%s", help_widget->dxmhelp.text);
#endif /* CSTEXT */
	    fclose(file_ptr);
	    /*
	    ** Unmanage the save as dialog box.
	    */
	    XtUnmanageChild (help_widget->dxmhelp.saveas_box->box);
	}
	else
	/*
	** Error: the file couldn't be open.
	*/
	{

	    set_message(help_widget, help_widget->dxmhelp.erroropen_message,
		filesel_struct->value);

	    /*
	    ** Popup the message box.
	    */
	    check_message(help_widget, help_widget);
	}
    }
    else
    /*
    ** Error: filename extraction failed.
    */
    {
    }
}

static void cancel_saveas (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the cancel button of the save as box:
**	cancel the file selection dialog box.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** Unmanage the save as dialog box.
    */
    XtUnmanageChild (help_widget->dxmhelp.saveas_box->box);
}

static void build_saveas (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the save as menu item: build save as
**	dialog box if doesn't exist already and display it.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    Arg arglist[20];
    int ac = 0;
    DXmHelpBox *box;
    int hoffset = 10;
    int voffset = 10;
    int max_width = 0;
    XtCallbackRec ok_callback[2];
    XtCallbackRec cancel_callback[2];
    XtCallbackRec help_callback[2];

    HwDisplayAllWaitCursor(help_widget);
    
    /*
    ** Check if the box doesn't already exist.
    */
    
    if (help_widget->dxmhelp.saveas_box == NULL)
    
    {
	/*
	** Initialization.
	*/
	ok_callback[0].callback = (XtCallbackProc ) ok_saveas;
	ok_callback[0].closure  = (XtPointer) help_widget;
	ok_callback[1].callback = NULL;

	cancel_callback[0].callback = (XtCallbackProc ) cancel_saveas;
	cancel_callback[0].closure  = (XtPointer) help_widget;
	cancel_callback[1].callback = NULL;

	help_callback[0].callback = (XtCallbackProc ) help_saveas;
	help_callback[0].closure  = (XtPointer) help_widget;
	help_callback[1].callback = NULL;

	box = (DXmHelpBox *) XtMalloc(sizeof(DXmHelpBox));
	box->button0 = NULL;
	box->button3 = NULL;
	box->label2 = NULL;
	box->listbox1 = NULL;
	box->listbox2 = NULL;
	/*
	** Set up the file selection widget
	*/

	XtSetArg(arglist[ac], XmNmarginHeight, MARGIN_HEIGHT); ac++;
	XtSetArg(arglist[ac], XmNmarginWidth, MARGIN_WIDTH); ac++;
#ifdef VMS
	XtSetArg(arglist[ac], XmNdirMask, XmStringLtoRCreate("*.TXT", "ISO8859-1")); ac++;
	XtSetArg(arglist[ac], XmNtextString, XmStringLtoRCreate("HELP_TOPIC.TXT", "ISO8859-1"));ac++;
#else
	XtSetArg(arglist[ac], XmNdirMask, XmStringLtoRCreate("*", "ISO8859-1")); ac++;
	XtSetArg(arglist[ac], XmNtextString, XmStringLtoRCreate("Help_Topic", "ISO8859-1")); ac++;
#endif
	XtSetArg(arglist[ac], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); ac++;
	XtSetArg (arglist[ac], XmNnoResize, FALSE); ac++;
	XtSetArg(arglist[ac], XmNdefaultPosition, TRUE); ac++;
/*        XtSetArg(arglist[ac], DXmNtakeFocus, TRUE ); ac++;      */
	XtSetArg (arglist[ac], XmNbuttonFontList,
	    help_widget->dxmhelp.button_font_list); ac++;
	XtSetArg (arglist[ac], XmNlabelFontList,
	    help_widget->dxmhelp.label_font_list); ac++;
	XtSetArg (arglist[ac], XmNtextFontList,
	    help_widget->dxmhelp.text_font_list); ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,
                               LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
     	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
        XtSetArg(arglist[ac], XmNokCallback, ok_callback); ac++;
	XtSetArg(arglist[ac], XmNcancelCallback, cancel_callback); ac++;
	/*
	** Create the dialog box
	*/
/* I18N alert:
   Creating the Fileselb that way precludes I18n (Strings are English only ?)
*/
	box->box = (Widget)XmCreateFileSelectionDialog ( (Widget) help_widget, "file select",
	    arglist, ac);
	    
	/*
	** Do a search on "*".
	*/
#ifdef VMS
    	XmFileSelectionDoSearch(box->box, XmStringLtoRCreate("*.txt", "ISO8859-1"));
#else
    	XmFileSelectionDoSearch(box->box, XmStringLtoRCreate("*", "ISO8859-1"));
#endif
    
	help_widget->dxmhelp.saveas_box = box;

    }

    HwRemoveAllWaitCursor(help_widget);
    
    /*
    ** Popup the saveas box.
    */
    XtManageChild (help_widget->dxmhelp.saveas_box->box);
}

static void quit (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the exit button: exit from the
**	help widget.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** Close the help library.
    */
    if (help_widget->dxmhelp.library_context != NULL)
        DXmHelp__close_help (help_widget->dxmhelp.library_context);
    /*
    ** Unmanage history box if exists.
    */
    if (help_widget->dxmhelp.history_box != NULL)
        XtUnmanageChild (help_widget->dxmhelp.history_box->box);
    /*
    ** Unmanage title box if exists.
    */
    if (help_widget->dxmhelp.title_box != NULL)
        XtUnmanageChild (help_widget->dxmhelp.title_box->box);
    /*
    ** Unmanage keyword box if exists.
    */
    if (help_widget->dxmhelp.keyword_box != NULL)
        XtUnmanageChild (help_widget->dxmhelp.keyword_box->box);

    /*
    ** Unmanage help widget.
    */
    XtUnmanageChild ((Widget)help_widget);
}

static void pulling_edit (menu, w)
    Widget menu;
    Widget w;
/*
**++
**  Functional Description:
**	Pulling callback routine for the edit menu: check if something
**	selected in the text widget, else set to unsensitive the copy item.
**	Check also if the library context isn't null else set to insensitive the
**	menu items.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	menu: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    char *selected_string = NULL;
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** Get the current selection from the help text widget.
    */
#ifdef CSTEXT
    selected_string = (char *) DXmCSTextGetSelection (help_widget->dxmhelp.help_text);
#else
    selected_string = XmTextGetSelection (help_widget->dxmhelp.help_text);
#endif /* CSTEXT */

    /*
    ** If the string isn't null and the library context isn' null, set
    ** the copy item to sensitive else to insensitive.
    */
    if ((selected_string != NULL) &&
	(help_widget->dxmhelp.library_context != NULL))
        XtSetSensitive (help_widget->dxmhelp.copy_button, TRUE);
    else
        XtSetSensitive (help_widget->dxmhelp.copy_button, FALSE);

    /*
    ** If the library_context and the trail topic aren't null, set
    ** the select all item to sensitive else to insensitive.
    */
    if ((help_widget->dxmhelp.library_context != NULL) &&
	(!(NullTopicInTrail(help_widget->dxmhelp.trail))))
        XtSetSensitive (help_widget->dxmhelp.selectall_button, TRUE);
    else
        XtSetSensitive (help_widget->dxmhelp.selectall_button, FALSE);
}

static void copy_clipboard (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the copy menu item: copy the
**	selected string to the clipboard.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
    {
    char *selected_string = NULL;
    int status = 0;
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    HwDisplayAllWaitCursor(help_widget);

    /*
    ** Get the current selection from the help text widget.
    */
    
#ifdef CSTEXT
    selected_string = (char *) DXmCSTextGetSelection (help_widget->dxmhelp.help_text);
#else
    selected_string = XmTextGetSelection (help_widget->dxmhelp.help_text);
#endif /* CSTEXT */

    /*
    ** Check if the string isn't null.
    */
    
    if (selected_string != NULL) {
    
	if (LockClipboard(help_widget)) {

	    status = CopyTextToClipboard(help_widget, selected_string);
#ifdef CSTEXT
/* BUG alert: Memory is freed for 'CSTEXT' cond only. It should always do */
	    XtFree(selected_string);
#endif /* CSTEXT */

	    if (status != ClipboardSuccess) {
		/* setup error message for clipboard failure */;
	    }

	    status = XmClipboardUnlock(XtDisplay(help_widget),
		XtWindow(help_widget), FALSE);
		
	    if (status != ClipboardSuccess) {
		/* setup error message for clipboard failure */;
	    }

	}
	else {
	    /* setup error message for clipboard failure */;
	}
    }

    HwRemoveAllWaitCursor(help_widget);

    }

static Boolean LockClipboard(hw)
    Widget hw;

    {
    int status = ClipboardLocked;
    int tries = 0;
    DXmHelpWidget help_widget = (DXmHelpWidget) hw;

    while ((status != ClipboardSuccess) && (tries < MaxClipboardTries)) {
	status = XmClipboardLock(XtDisplay(help_widget),
	    XtWindow(help_widget));
	tries++;
    }

    if ((tries >= MaxClipboardTries) && (status != ClipboardSuccess))
	return(FALSE);
    else
	return(TRUE);
    }


static int CopyTextToClipboard(hw, text)
    Widget hw;
    char *text;

    {
    DXmHelpWidget help_widget = (DXmHelpWidget) hw;
    XmString clip_label;
    long item_id = 0;
    char *xa_string;
    int data_id = 0;
#ifdef CSTEXT
    Opaque *ddif = NULL;
    char *plain_text;
    Opaque *ct = NULL;
    long count,stat;
#endif /* CSTEXT */
#ifdef DEC_MOTIF_BUG_FIX
    int status;
#else
    enum clip_return status;
#endif
    Time eventtime;

    clip_label = XmStringLtoRCreate("Help Widget", "ISO8859-1");

    /*
    **  Start transaction with the clipboard
    */

    eventtime = XtLastTimestampProcessed(XtDisplay(help_widget));

    status = XmClipboardStartCopy
		(XtDisplay(help_widget),	/* display */
		 XtWindow(help_widget),		/* window */
		 clip_label,			/* clip_label */
		 eventtime,			/* timestamp */
		 (Widget)NULL,			/* widget */
		 (XmCutPasteProc)NULL,		/* callback */
		 &item_id);			/* item_id_ret */

    XtFree((char *)clip_label);

    if (status != ClipboardSuccess)
	return(status);
	
    /*
    ** Copy the string to the clipboard.
    */
    
    xa_string = XGetAtomName (XtDisplay (help_widget), XA_STRING);
    
#ifdef CSTEXT
    plain_text = (char *) DXmCvtCStoFC((XmString) text,&count,&stat);
    status = XmClipboardCopy (XtDisplay(help_widget), XtWindow(help_widget),
	item_id, xa_string, plain_text, (unsigned long) count, 0,
	(long *) &data_id);
    XtFree(plain_text);
#else
    status = XmClipboardCopy (XtDisplay(help_widget), XtWindow(help_widget),
	item_id, xa_string, text, (unsigned long) (strlen(text) + 1), 0,
	(unsigned long) &data_id);
#endif /* CSTEXT */

    XFree (xa_string);

    if (status != ClipboardSuccess)
	return(status);

#ifdef CSTEXT    
    /*
    ** Copy Compound Text to the clipboard.
    */
    
    ct = (Opaque *) XmCvtXmStringToCT((XmString) text);
    if (ct != NULL) {

    	status = XmClipboardCopy (XtDisplay(help_widget), XtWindow(help_widget),
		item_id, "COMPOUND_TEXT", ct, (unsigned long) (strlen((char *) ct) + 1), 0,
		(long *) &data_id);
    
	XtFree((char *)ct);

    	if (status != ClipboardSuccess)
		return(status);
    }

    /*
    ** Copy DDIF Text to the clipboard.
    */
    
    ddif = (Opaque *) DXmCvtCStoDDIF((XmString) text,&count,&stat);
    if (stat != DXmCvtStatusFail) {

    	status = XmClipboardCopy (XtDisplay(help_widget), XtWindow(help_widget),
		item_id, "DDIF", ddif, (unsigned long) count, 0,
		(long *) &data_id);
    
	XtFree((char *)ddif);	/* needs to check if this 'free' is O.K ? */

    	if (status != ClipboardSuccess)
		return(status);
    }
#endif /* CSTEXT */

    /*
    ** Finish transaction with clipboard.
    */
    
    status = XmClipboardEndCopy (XtDisplay(help_widget),
	XtWindow(help_widget), item_id);

    return(status);
    }
    

static void selectall (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the select all menu item: select all
**	the text in the help text widget.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** Set the selection on the text widget to the whole text.
    */
#ifdef CSTEXT
    DXmCSTextSetSelection (help_widget->dxmhelp.help_text, 0,
			  XmStringLength (help_widget->dxmhelp.text) -1,
			  CurrentTime );
#else
    XmTextSetSelection (help_widget->dxmhelp.help_text, 0,
			  strlen (help_widget->dxmhelp.text) -1,
			  CurrentTime );
#endif
}

static void pulling_search (menu, w)
    Widget menu;
    Widget w;
/*
**++
**  Functional Description:
**	Pulling callback routine for the search menu: check if the history
**	list isn't empty, else set the history item to insensitive. Check if the
**	library context isn't null else set items to insensitive.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	menu: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    if ((help_widget->dxmhelp.library_context != NULL) &&
	(help_widget->dxmhelp.history != NULL))
        XtSetSensitive (help_widget->dxmhelp.history_button, TRUE);
    else
        XtSetSensitive (help_widget->dxmhelp.history_button, FALSE);
    if (help_widget->dxmhelp.library_context != NULL)
    {
        XtSetSensitive (help_widget->dxmhelp.title_button, TRUE);
        XtSetSensitive (help_widget->dxmhelp.keyword_button, TRUE);
    }
    else
    {
        XtSetSensitive (help_widget->dxmhelp.title_button, FALSE);
        XtSetSensitive (help_widget->dxmhelp.keyword_button, FALSE);
    }
}


static void select_history (listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Single select callback routine for the history listbox: get the
**	selected topic in the listbox.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;  
    char **topic_array = NULL;
    DXmHelpTrail *p;
    int i;

    if (list->item_position != 0)
    {
	/*
	** Allocate a topic array.
	*/
	topic_array = (char **) XtMalloc(sizeof(char *) *
					help_widget->dxmhelp.history_count);

	/*
	** Fill in the topic array by looping on the history list.
	*/
	p = help_widget->dxmhelp.history;
	for (i = 0; i < help_widget->dxmhelp.history_count; i++)
	{
	    topic_array[i] = (char *)XmStringCopy((XmString) p->topic);
	    p = p->next;
	}

	/*
	** Free the previous selected history topic.
	*/
	XtFree ((char *)help_widget->dxmhelp.selected_history_topic);

	/*
	** Update the selected history topic to be the one returned in the
	** callback structure.
	*/
        help_widget->dxmhelp.selected_history_topic =
	    XmStringCopy((XmString) topic_array[list->item_position - 1]);

	/*
	** Free the topic array.
	*/
	free_array(topic_array, help_widget->dxmhelp.history_count);
    }
}

static void goto_select_history (listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Select and Confirm callback routine for the history listbox: get
**	the selected topic and go to it.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    char **topic_array = NULL;
    DXmHelpTrail *p;
    int i;
    
    if (list->item_position != 0)
    {
	/*
	** Allocate a topic array.
	*/
	topic_array = (char **) XtMalloc(sizeof(char *) *
					help_widget->dxmhelp.history_count);
    
	/*
	** Fill in the topic array by looping on the history list.
	*/
	p = help_widget->dxmhelp.history;
	for (i = 0; i < help_widget->dxmhelp.history_count; i++)
	{
	    topic_array[i] = (char *)XmStringCopy ((XmString) p->topic);
	    p = p->next;
	}

	/*
	** Free the previous selected history topic.
	*/
	XtFree ((char *)help_widget->dxmhelp.selected_history_topic);

	/*
	** Update the selected history topic to be the one returned in the
	** callback structure.
	*/
        help_widget->dxmhelp.selected_history_topic =
	    XmStringCopy((XmString) topic_array[list->item_position - 1]);

	/*
	** Free the topic array.
	*/
	free_array(topic_array, help_widget->dxmhelp.history_count);

	/*
	** Get the topic selected in the history box and update the help widget.
	*/
        goto_history (NULL, help_widget);
    }
}
    

static void set_history (help_widget, duplicate)
    DXmHelpWidget help_widget;
    Boolean duplicate;
/*
**++
**  Functional Description:
**	Set the history dialog box if it is managed.
**
**  Keywords:
**	History management.
**
**  Arguments:
**	help_widget: pointer to the help widget's record
**	duplicate: boolean indicating if the topic is already in the history
**	list.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    char **title_array = NULL;
    DXmHelpTrail *p;
    Arg arglist[10];
    int ac = 0;
    int i;
    

    
    if ((help_widget->dxmhelp.history_box != NULL) &&
	XtIsRealized(help_widget->dxmhelp.history_box->box) &&
	XtIsManaged(help_widget->dxmhelp.history_box->box))
    {
	/*
	** If the topic on top of the trail list is already in the history
	** list, select it in the listbox.
	*/
	if (duplicate)
	{
	    XtSetArg (arglist[ac], XmNselectedItems,
    			      &help_widget->dxmhelp.trail->title);
	    ac++;
	    XtSetArg (arglist[ac], XmNselectedItemCount, 1);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.history_box->listbox1,
			 arglist, ac);
	}
	else
	/*
	** The topic doesn't exists, so reset the listbox with the new list.
	*/
	{
	    /*
	    ** Allocate a title array.
	    */
	    title_array = (char **) XtMalloc(sizeof(char *) *
					    help_widget->dxmhelp.history_count);

	    /*
	    ** Initialize the title array looping on the history list.
	    */
	    p = help_widget->dxmhelp.history;
	    for (i = 0; i < help_widget->dxmhelp.history_count; i++)
	    {
		title_array[i] = (char *)XmStringCopy((XmString) p->title);
		p = p->next;
	    }
	    /*
	    ** Update the history listbox and select the topic on top of the
	    ** trail list if not null.
	    */
	    XtSetArg (arglist[ac], XmNitems, title_array);
	    ac++;
	    XtSetArg (arglist[ac], XmNitemCount,
				  help_widget->dxmhelp.history_count);
	    ac++;
	    if (help_widget->dxmhelp.trail->topic != NULL)
	    {
		XtSetArg (arglist[ac], XmNselectedItems,
				  &help_widget->dxmhelp.trail->title);
		ac++;
		XtSetArg (arglist[ac], XmNselectedItemCount, 1);
		ac++;
                XtSetValues (help_widget->dxmhelp.history_box->listbox1,
	            arglist, ac);
	    }
	    else
	    {
                /* Need to pre-def a selection of some sort for 
                 * DefaultAction == kbm ==
                 */
                XmListSelectPos (help_widget->dxmhelp.history_box->listbox1,
  (((XmListWidget )help_widget->dxmhelp.history_box->listbox1)->list.itemCount 
                    > 1 ? 1 : 0), False);
	    }
                XmListSetPos (help_widget->dxmhelp.history_box->listbox1, 1); 

	    /*
	    ** Update the selected history topic.
	    */
	    if (help_widget->dxmhelp.trail->topic != NULL)
		help_widget->dxmhelp.selected_history_topic =
		    XmStringCopy((XmString) help_widget->dxmhelp.trail->topic);
	    else
		help_widget->dxmhelp.selected_history_topic = NULL;
	    /*
	    ** Free the title array.
	    */		
	    free_array(title_array, help_widget->dxmhelp.history_count);
	}
    }
}

static void add_history (help_widget)
    DXmHelpWidget help_widget;
/*
**++
**  Functional Description:
**	Add a history record to the history list if the topic isn't already in
**	the list.
**
**  Keywords:
**	History management
**
**  Arguments:
**	help_widget: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpTrail *p = NULL;
    DXmHelpTrail *q = NULL;
    Boolean duplicate = FALSE;
        
    /* 
    ** If the history list is empty, create a history record and initialize it
    ** with the topic on top of the trail list.
    */
    if (help_widget->dxmhelp.history == (DXmHelpTrail *) NULL)
    {
	p = (DXmHelpTrail *) XtMalloc (sizeof(DXmHelpTrail));
	p->topic =
	    (char *)XmStringCopy ((XmString) help_widget->dxmhelp.trail->topic);
	p->title =
	    (char *)XmStringCopy ((XmString) help_widget->dxmhelp.trail->title);
	p->next = NULL;

	help_widget->dxmhelp.history = p;
	help_widget->dxmhelp.history_count ++;
    }
    /*
    ** The history list isn't empty : check if the topic isn't already there
    ** before adding a new record at the end of the list.
    */
    else
    {
	/*
	** Compare the topic on top of the trail list with all topics in the
	** history list.
	*/
	q = help_widget->dxmhelp.history;
	while ((q != NULL) && (!duplicate))
	{
	    if (XmStringCompare ((XmString) q->title,(XmString) help_widget->dxmhelp.trail->title))
		duplicate = TRUE;
	    q = q->next;
	}
	/*
	** If it's a new topic, create a history record, initialize it and add
	** it at the end of the list.
	*/
	if (!duplicate)
	{
	    q = help_widget->dxmhelp.history;
	    while (q->next != NULL) q = q->next;
	    p = (DXmHelpTrail *) XtMalloc (sizeof(DXmHelpTrail));
	    p->topic =
		(char *)XmStringCopy ((XmString) help_widget->dxmhelp.trail->topic);
	    p->title =
		(char *)XmStringCopy ((XmString) help_widget->dxmhelp.trail->title);
	    p->next = NULL;

	    q->next = p;
	    help_widget->dxmhelp.history_count ++;
	}
    }
    /*
    ** Reset the history dialog box if it's managed.
    */
    set_history (help_widget, duplicate);
}

/*
**  Free the history list
*/
static void free_history(help_widget)
    DXmHelpWidget help_widget;
/*
**++
**  Functional Description:
**	Deallocate the history list.
**
**  Keywords:
**	History management
**
**  Arguments:
**	help_widget: pointer to the help widget's record.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpTrail *p, *q;

    /*
    ** Free the history list.
    */
    p = help_widget->dxmhelp.history;
    q = help_widget->dxmhelp.history;
    while (p != NULL)
    {
	XtFree(q->topic);
	XtFree(q->title);
	p = q->next;
	XtFree((char *)q);
	q = p;
    }
    /*
    ** Reset the history list pointer and counter to NULL.
    */
    help_widget->dxmhelp.history = NULL;
    help_widget->dxmhelp.history_count = 0;
    /*
    ** Free the selected history topic and reset the pointer.
    */
    XtFree ((char *)help_widget->dxmhelp.selected_history_topic);
    help_widget->dxmhelp.selected_history_topic = NULL;
}

static void goto_history (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the goto button of the history box: go to
**	the selected history topic.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    **  Turn on the wait cursors
    */
    
    HwDisplayAllWaitCursor(help_widget);

    if (help_widget->dxmhelp.selected_history_topic!= NULL)
    {
	/*
	** Check if the selected topic is different from the one on the top of
	** the trail list.
	*/
        if (!XmStringCompare ((XmString) help_widget->dxmhelp.trail->topic,
		        (XmString) help_widget->dxmhelp.selected_history_topic))
	{    
	    /*
	    ** Push the selected topic on top of the trail list.
	    */
	    insert_trail(help_widget, help_widget->dxmhelp.selected_history_topic);

	    /*
	    ** Free the selected topic and title.
	    */
	    XtFree ((char *)help_widget->dxmhelp.selected_topic);
	    help_widget->dxmhelp.selected_topic = NULL;
	    XtFree ((char *)help_widget->dxmhelp.selected_title);
	    help_widget->dxmhelp.selected_title = NULL;

	    /*
	    ** Get the help frame associated with the topic on top of the trail
	    ** list and update the help text and lisbox.
	    */
	    get_help(help_widget);
	}
    }

    if (XtIsManaged(help_widget))
	HwRaiseWindow(help_widget);

    HwRemoveAllWaitCursor(help_widget);

}
    

static void visit_history (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the visit button of the history box: visit
**	the selected history topic.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    **  Turn the wait cursor on
    */

    HwDisplayAllWaitCursor(help_widget);

    /*
    ** Create a new help widget or reuse and update an existing one.
    */
    if (help_widget->dxmhelp.selected_history_topic != NULL)
	visit_help (help_widget,
		    help_widget->dxmhelp.selected_history_topic,
    		    help_widget->dxmhelp.library);

    HwRemoveAllWaitCursor(help_widget);
}
    

static void dismiss_history (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the dismiss button in the history box:
**	unmanage the history box.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** Unmanage the history dialog box.
    */
    XtUnmanageChild (help_widget->dxmhelp.history_box->box);
}

static void build_search_history (button, w, reason)
    Widget button;
    Widget w;
    Reason reason;
/*
**++
**  Functional Description:
**	Activate callback routine for the history item in the search menu:
**	create the history dialog box and manage it. Set the box contents.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    Arg arglist[20];
    int ac = 0;
    XtCallbackRec select_callback[2];
    XtCallbackRec select_go_callback[2];
    XtCallbackRec goto_callback[2];
    XtCallbackRec visit_callback[2];
    XtCallbackRec dismiss_callback[2];
    XtCallbackRec help_callback[2];
    DXmHelpTrail *p;
    DXmHelpBox *box;
    Widget shell;
    Time time;
    int listbox1_vis = 10;
    int hoffset = 10;
    int voffset = 10;
    int max_width = 400;
    int button_width = 0;
    int i;

    HwDisplayAllWaitCursor(help_widget);

    /*
    ** Check if the box doesn't already exist.
    */
    if (help_widget->dxmhelp.history_box == NULL)
    {
    
    	/*
	** Initialization.
	*/
	select_callback[0].callback = (XtCallbackProc) select_history;
	select_callback[0].closure = (XtPointer) help_widget;
	select_callback[1].callback = NULL;
	select_go_callback[0].callback = (XtCallbackProc) goto_select_history;
	select_go_callback[0].closure = (XtPointer) help_widget;
	select_go_callback[1].callback = NULL;
	goto_callback[0].callback = (XtCallbackProc ) goto_history;
	goto_callback[0].closure  = (XtPointer) help_widget;
	goto_callback[1].callback = NULL;
	visit_callback[0].callback = (XtCallbackProc ) visit_history;
	visit_callback[0].closure  = (XtPointer) help_widget;
	visit_callback[1].callback = NULL;
	dismiss_callback[0].callback = (XtCallbackProc ) dismiss_history;
	dismiss_callback[0].closure  = (XtPointer) help_widget;
	dismiss_callback[1].callback = NULL;
	help_callback[0].callback = (XtCallbackProc ) help_history;
	help_callback[0].closure  = (XtPointer) help_widget;
	help_callback[1].callback = NULL;

	/*
	** Initialize the box record.
	*/
	box = (DXmHelpBox *) XtMalloc(sizeof(DXmHelpBox));
	box->label2 = NULL;
	box->button0 = NULL;
	box->listbox2 = NULL;
	box->text = NULL;
	
	/*
	** Create the attached dialog box.
	*/
	XtSetArg (arglist[ac], XmNdialogStyle, XmDIALOG_MODELESS);
	ac++;
/*	XtSetArg (arglist[ac], DXmNtakeFocus, TRUE);
	ac++;   */
	XtSetArg (arglist[ac], XmNautoUnmanage, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNdefaultPosition, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNdialogTitle,help_widget->dxmhelp.history_box_label);
	ac++;
	XtSetArg (arglist[ac], XmNbuttonFontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNlabelFontList, help_widget->dxmhelp.label_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNtextFontList, help_widget->dxmhelp.text_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++; 
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNrubberPositioning, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNhorizontalSpacing, hoffset);
	ac++;
	XtSetArg (arglist[ac], XmNverticalSpacing, voffset);
	ac++;
	XtSetArg (arglist[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg (arglist[ac], XmNnoResize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	box->box = (Widget) 
	    XmCreateFormDialog ((Widget) help_widget, "history", arglist, ac);

	/*
	** Create the title label.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNborderWidth, 0);
	ac++;
	XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
	ac++;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.topic_titles_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.label_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,
                               LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	box->label1 = XmCreateLabel (box->box, "historylabel", arglist, ac);

	/*
	** Create the topic listbox.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNbrowseSelectionCallback, select_callback);
	ac++;
	XtSetArg (arglist[ac], XmNdefaultActionCallback, select_go_callback);
	ac++;
	XtSetArg (arglist[ac], XmNvisibleItemCount, listbox1_vis);
	ac++;
	XtSetArg (arglist[ac], XmNitems, NULL);
	ac++;
	XtSetArg (arglist[ac], XmNitemCount, 0);
	ac++;
	XtSetArg (arglist[ac], XmNlistSizePolicy, XmVARIABLE);
	ac++;
	XtSetArg (arglist[ac], XmNscrollBarDisplayPolicy, XmSTATIC);
	ac++;
/*  ??  These next two should probably be set in a resolution independent unit */
	XtSetArg (arglist[ac], XmNmarginHeight, 3);
	ac++;
	XtSetArg (arglist[ac], XmNlistSpacing, 3);
	ac++;
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, box->label1);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.text_font_list);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,
                               LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
		
	box->listbox1 =
		XmCreateScrolledList (box->box, "historylistbox", arglist, ac);
	
	/*
	** Create the goto button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.goto_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, goto_callback);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox1));
	ac++;
	XtSetArg (arglist[ac], XmNtopOffset,20);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,
                               LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button1 =
	    XmCreatePushButton (box->box, "historybutton", arglist, ac);

	/*
	** Create the visit button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.visit_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, visit_callback);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox1));
	ac++;
	XtSetArg (arglist[ac], XmNtopOffset, 20);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,
                               LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNleftWidget, box->button1);
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button2 =
	    XmCreatePushButton (box->box, "historybutton", arglist, ac);

	/*
	** Create the dismiss button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.dismiss_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, dismiss_callback);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox1));
	ac++;
	XtSetArg (arglist[ac], XmNtopOffset, 20);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,
                               LayoutDirection(help_widget)); ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button3 =
	    XmCreatePushButton (box->box, "historybutton", arglist, ac);

	/*
	** Set the default button. Shouldn't have one 'cause has no text widget.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNdefaultButton, box->button1);
	ac++;
	XtSetArg (arglist[ac], XmNcancelButton, box->button3);
	ac++;
	XtSetValues (box->box, arglist, ac);

	/*
	** Manage all the children widgets.
	*/
	XtManageChild(box->listbox1);
        XtManageChildren (XtChildren((XmBulletinBoardWidget)box->box),
			  XtNumChildren((XmBulletinBoardWidget)box->box));

	/*
	** Compute the width of the buttons and set it.
	*/
	button_width = MAX(button_width, XtWidth(box->button1));
	button_width = MAX(button_width, XtWidth(box->button2));
	button_width = MAX(button_width, XtWidth(box->button3));
	ac = 0;
	XtSetArg (arglist[ac], XmNrecomputeSize, FALSE);
	ac++;
	XtSetArg (arglist[ac], XtNwidth, button_width);
	ac++;
	XtSetValues (box->button1, arglist, ac);
	XtSetValues (box->button2, arglist, ac);
	XtSetValues (box->button3, arglist, ac);
	
	/*
	** Set the title label width to the max width.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XtNwidth, max_width);
	ac++;
	XtSetValues (box->label1, arglist, ac);

	/*
	** Store the box record into the help widget.
	*/
	help_widget->dxmhelp.history_box = box;
	/*
	** If the box isn't realized, realize it.
	*/
	if (!XtIsRealized(help_widget->dxmhelp.history_box->box))
	    XtRealizeWidget (help_widget->dxmhelp.history_box->box);

	/*
	** Position the box with the default algorythm if not already managed.
	*/
	if (!XtIsManaged(help_widget->dxmhelp.history_box->box))
	    DXmPositionWidget (help_widget->dxmhelp.history_box->box,
			(Widget *)&help_widget, 1);

	/*
	** Manage the box.
	*/
	XtManageChild (help_widget->dxmhelp.history_box->box);

    }

    else {
    
	/*
	**  Make the box pop on top if it is already on the screen
	*/
	
	if (XtIsManaged(help_widget->dxmhelp.history_box->box)) {
	    HwRaiseWindow(help_widget->dxmhelp.history_box->box);
	    time = ((XButtonEvent *) reason->event)->time;
	    XtCallAcceptFocus(help_widget->dxmhelp.history_box->box, &time); 
	}
	else
	    XtManageChild (help_widget->dxmhelp.history_box->box);
    }

    /*
    ** Set the listbox contents.
    */
    set_history (help_widget, FALSE);

    HwRemoveAllWaitCursor(help_widget);
    
}

static void search_title (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the apply button of the search title box:
**	search for titles which match the text entered by the user.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    XmString cs_title;
    char *title;
    Arg arglist[20];
    int ac = 0;
    char **topic_array = NULL;
    char **title_array = NULL;
    int topic_count;
    int count_save;
    int i;
    XmStringCharSet  char_set;
    Boolean direction;
#ifdef GENERAL_TEXT
    long count,stat;
#endif /* GENERAL_TEXT */

    /*
    **  Display wait cursors
    */

    HwDisplayAllWaitCursor(help_widget);

    /*
    ** Get the string entered by the user.
    */
#ifdef CSTEXT
    cs_title = (XmString) DXmCSTextGetString(help_widget->dxmhelp.title_box->text);
    title = (char *)DXmCvtCStoFC(cs_title,&count,&stat);
    XtFree((char *)cs_title);
#else
    title = XmTextGetString (help_widget->dxmhelp.title_box->text);
#endif /* CSTEXT */
    if (title != NULL)
    {
	/*
	** Search for the titles matching the string.
	*/
	DXmHelp__search_title (help_widget->dxmhelp.library_context,
			       title,
			       &topic_array,
			       &title_array,
			       &topic_count);
	
	/*
	** Free the previous search topic and title arrays.
	*/
	
	if (help_widget->dxmhelp.searchtitle_topic_array != NULL)
	{
	    XtSetArg (arglist[0], XmNitemCount, &count_save);
	    XtGetValues (help_widget->dxmhelp.title_box->listbox1, arglist, 1);
	    free_array (help_widget->dxmhelp.searchtitle_topic_array,
			count_save);
	    help_widget->dxmhelp.searchtitle_topic_array = NULL;
	}

	if (topic_count != 0)
	{

	    /*
	    **  The search operation was successful
	    */
	    		    
	    help_widget->dxmhelp.searchtitle_topic_array = topic_array;
	    
	    /*
	    ** Update the search title listbox.
	    */
	    
	    XtSetArg (arglist[ac], XmNitems, title_array);
	    ac++;
	    XtSetArg (arglist[ac], XmNitemCount, topic_count);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.title_box->listbox1, arglist, ac);
          /* Need to pre-def a selection of some sort for DefaultAction =kbm */
            XmListSelectPos (help_widget->dxmhelp.title_box->listbox1,
    (((XmListWidget )help_widget->dxmhelp.title_box->listbox1)->list.itemCount 
                > 1 ? 1 : 0), False);
            XmListSetPos (help_widget->dxmhelp.title_box->listbox1, 1); 

	    /*
	    ** Set the goto and visit buttons to insensitive.
	    */
	    
	    XtSetSensitive (help_widget->dxmhelp.title_box->button1, FALSE);
	    XtSetSensitive (help_widget->dxmhelp.title_box->button2, FALSE);

	    /*
	    ** Free the user string and the title array.
	    */
	    
	    XtFree (title);
	    free_array (title_array, topic_count);
	}	
	else
	
	/*
	** Error: there is no title matching the given string.
	*/
	
	{
	    /*
	    ** Clear the search title listbox.
	    */
	    
	    XtSetArg (arglist[ac], XmNitemCount, 0);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.title_box->listbox1, arglist, ac);

	    /*
	    **  Set up the error message
	    */
	    
#ifdef GENERAL_TEXT
	    cs_title = (XmString) DXmCvtFCtoCS(title, &count,&stat);
#else
	    cs_title = XmStringLtoRCreate (title, "ISO8859-1");
#endif /* GENERAL_TEXT */
	    set_message(help_widget, help_widget->dxmhelp.notitle_message,
		cs_title);
	    XtFree((char *)cs_title);
	    
	    /*
	    ** Display the message box.
	    */
	    
	    check_message (help_widget, help_widget->dxmhelp.title_box->box);
	    
	}
    }

    HwRemoveAllWaitCursor(help_widget);
    
}

static void select_title(listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Single select callback routine for the search title listbox: get the
**	selected topic in the listbox.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
	
    if (list->item_position != 0)
    {
	/*
	** Get the topic associated with the returned title number and store it
	** into the help widget's record.
	*/
        help_widget->dxmhelp.selected_searchtitle_topic =
	    XmStringCopy((XmString) help_widget->dxmhelp.searchtitle_topic_array[list->item_position - 1]);

	/*
	** Set the goto and visit buttons to sensitive.
	*/
	XtSetSensitive (help_widget->dxmhelp.title_box->button1, TRUE);
	XtSetSensitive (help_widget->dxmhelp.title_box->button2, TRUE);
    }

}

/*
** Get the selected topic and go to it directly (double click callback)
*/
static void goto_select_title(listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Select and Confirm callback routine for the title listbox: get the
**	selected topic and go to it.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    
    if (list->item_position != 0)
    {
	/*
	** Get the topic associated with the returned title number and store it
	** into the help widget's record.
	*/
	help_widget->dxmhelp.selected_searchtitle_topic =
	    XmStringCopy((XmString) help_widget->dxmhelp.searchtitle_topic_array[list->item_position - 1]);
		    
	/*
	** Set the goto and visit buttons to sensitive.
	*/
	XtSetSensitive (help_widget->dxmhelp.title_box->button1, TRUE);
	XtSetSensitive (help_widget->dxmhelp.title_box->button2, TRUE);

	/*
	** Go to the selected topic.
	*/
        goto_title (NULL, help_widget);
    }
    
}

static void goto_title (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the goto button of the search title box:
**	go to the selected search title.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    int comp = 1;

    /*
    **  Turn on the wait cursors
    */
    
    HwDisplayAllWaitCursor(help_widget);
	
    /*
    **  Check if the search title box already exists
    */
            
    if (help_widget->dxmhelp.selected_searchtitle_topic!= NULL)
    {
	/*
	** Check if the selected searchtitle topic is different from the one on
	** top of the trail list.
	*/
	if ((help_widget->dxmhelp.trail != NULL) &&
	    (help_widget->dxmhelp.trail->topic != NULL))
	    comp = !XmStringCompare ((XmString) help_widget->dxmhelp.trail->topic,
		        (XmString) help_widget->dxmhelp.selected_searchtitle_topic);
	if (comp != 0)
	{
	    /*
	    ** Push the selected searchtitle topic on top of the trail list.
	    */   
	    insert_trail(help_widget,
			 help_widget->dxmhelp.selected_searchtitle_topic);
	    /*
	    ** Free the help widget's selected topic and title.
	    */
	    XtFree ((char *)help_widget->dxmhelp.selected_topic);
	    help_widget->dxmhelp.selected_topic = NULL;
	    XtFree ((char *)help_widget->dxmhelp.selected_title);
	    help_widget->dxmhelp.selected_title = NULL;
	    
	    /*
	    ** Get the help frame associated with the topic on top of the trail
	    ** list and update the help text and listbox.
	    */
	    get_help(help_widget);
	}
    }

    if (XtIsManaged(help_widget))
	HwRaiseWindow(help_widget);

    HwRemoveAllWaitCursor(help_widget);
    
}
    

static void visit_title(button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the visit button of the search title box:
**	visit the selected search title.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    **  Turn on the wait cursor
    */
    
    HwDisplayAllWaitCursor(help_widget);
    
    /*
    ** Create a new help widget or reuse and update an existing one.
    */
    if (help_widget->dxmhelp.selected_searchtitle_topic!= NULL)
	visit_help (help_widget,
		    help_widget->dxmhelp.selected_searchtitle_topic,
		    help_widget->dxmhelp.library);

    HwRemoveAllWaitCursor(help_widget);
}
    

static void dismiss_title (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the dismiss button of the search title
**	box: unmanage the search title box.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** Unmanage the dialog box.
    */
    XtUnmanageChild (help_widget->dxmhelp.title_box->box);
}
	

static void build_search_title (button, w, reason)
    Widget button;
    Widget w;
    Reason reason;
/*
**++
**  Functional Description:
**	Activate callback routine for the title menu item in the search menu:
**	create the title dialog box and manage it.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    Arg arglist[20];
    int ac = 0;
#ifdef CSTEXT
    char *help_text_buf = NULL ;
    long count,stat;
#endif /* CSTEXT */
    XtCallbackRec search_title_callback[2];
    XtCallbackRec select_callback[2];
    XtCallbackRec select_go_callback[2];
    XtCallbackRec goto_callback[2];
    XtCallbackRec visit_callback[2];
    XtCallbackRec dismiss_callback[2];
    XtCallbackRec help_callback[2];
    DXmHelpBox *box;
    Time time;
    int listbox1_vis = 10;
    int max_width = 0;
    int button_width = 0;
    int hoffset = 10;
    int voffset = 10;
    int i;
                
    HwDisplayAllWaitCursor(help_widget);
	
    /*
    ** Check if the box isn't already created.
    */
    if (help_widget->dxmhelp.title_box == NULL)
    {

	/*
	** Initialization.
	*/
	search_title_callback[0].callback = (XtCallbackProc) search_title;
	search_title_callback[0].closure = (XtPointer) help_widget;
	search_title_callback[1].callback = NULL;
	select_callback[0].callback = (XtCallbackProc) select_title;
	select_callback[0].closure = (XtPointer) help_widget;
	select_callback[1].callback = NULL;
	select_go_callback[0].callback = (XtCallbackProc) goto_select_title;
	select_go_callback[0].closure = (XtPointer) help_widget;
	select_go_callback[1].callback = NULL;
	goto_callback[0].callback = (XtCallbackProc ) goto_title;
	goto_callback[0].closure  = (XtPointer) help_widget;
	goto_callback[1].callback = NULL;
	visit_callback[0].callback = (XtCallbackProc ) visit_title;
	visit_callback[0].closure  = (XtPointer) help_widget;
	visit_callback[1].callback = NULL;
	dismiss_callback[0].callback = (XtCallbackProc ) dismiss_title;
	dismiss_callback[0].closure  = (XtPointer) help_widget;
	dismiss_callback[1].callback = NULL;
	help_callback[0].callback = (XtCallbackProc ) help_title;
	help_callback[0].closure  = (XtPointer) help_widget;
	help_callback[1].callback = NULL;

	/*
	** Initialize the box record.
	*/
	box = (DXmHelpBox *) XtMalloc(sizeof(DXmHelpBox));
	box->listbox2 = NULL;
	
	/*
	** Create the attached dialog box.
	*/
	XtSetArg (arglist[ac], XmNdialogStyle, XmDIALOG_MODELESS);
	ac++;
	XtSetArg (arglist[ac], XmNdefaultPosition, FALSE);
	ac++;
/*	XtSetArg (arglist[ac], DXmNtakeFocus, TRUE);
	ac++; */
	XtSetArg (arglist[ac], XmNautoUnmanage, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNdialogTitle,
		  help_widget->dxmhelp.searchtitle_box_label);
	ac++;
	XtSetArg (arglist[ac], XmNbuttonFontList,
	    help_widget->dxmhelp.button_font_list); ac++;
	XtSetArg (arglist[ac], XmNlabelFontList,
	    help_widget->dxmhelp.label_font_list); ac++;
	XtSetArg (arglist[ac], XmNtextFontList,
	    help_widget->dxmhelp.text_font_list); ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNrubberPositioning, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNhorizontalSpacing, hoffset);
	ac++;
	XtSetArg (arglist[ac], XmNverticalSpacing, voffset);
	ac++;
	XtSetArg (arglist[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg (arglist[ac], XmNnoResize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	box->box = (Widget) 
	    XmCreateFormDialog ((Widget)help_widget,"titles", arglist, ac);

	/*
	** Create the title label.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNborderWidth, 0);
	ac++;
	XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
	ac++;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.titles_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.label_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	box->label1 = XmCreateLabel (box->box, "", arglist, ac);

	/*
	** Create the apply button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.apply_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, search_title_callback);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	box->button0 = XmCreatePushButton (box->box, "", arglist, ac);


	/*
	** Create the title text widget.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNresizeWidth, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNresizeHeight, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNwordWrap, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNeditable, TRUE);
	ac++;
	XtSetArg (arglist[ac], XmNborderWidth, 1);
	ac++;
	XtSetArg (arglist[ac], XmNcolumns, 34);
	ac++;
	XtSetArg (arglist[ac], XmNrows, 1);
	ac++;
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNleftWidget, box->label1);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.text_font_list);
	ac++;
#ifdef CSTEXT
	help_text_buf = (char *)DXmCvtFCtoCS("",&count,&stat); 
	XtSetArg (arglist[ac], XmNvalue, help_text_buf);
        ac++;
#endif /* CSTEXT */
#ifdef RTOL
        XtSetArg (arglist[ac],DXmNlayoutDirection,LayoutDirection(help_widget));
        ac++;
#endif /* RTOL */
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	    
#ifdef CSTEXT
        box->text = (Widget) DXmCreateCSText(box->box, "", arglist, ac);
        XtFree(help_text_buf) ;
#else
	box->text = XmCreateText(box->box, "", arglist, ac);
#endif /* CSTEXT */
    
	/*
	** Create the topic titles label.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNborderWidth, 0);
	ac++;
	XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
	ac++;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.topic_titles_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.label_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, box->button0);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	box->label2 = XmCreateLabel (box->box, "", arglist, ac);

	/*
	** Create the titles listbox.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNbrowseSelectionCallback, select_callback);
	ac++;
	XtSetArg (arglist[ac], XmNdefaultActionCallback, select_go_callback);
	ac++;
	XtSetArg (arglist[ac], XmNlistSizePolicy, XmVARIABLE);
	ac++;
	XtSetArg (arglist[ac], XmNscrollBarDisplayPolicy, XmSTATIC);
	ac++;
/*  ??  These next two should probably be set in a resolution independent unit */
	XtSetArg (arglist[ac], XmNmarginHeight, 3);
	ac++;
	XtSetArg (arglist[ac], XmNlistSpacing, 3);
	ac++;
	XtSetArg (arglist[ac], XmNvisibleItemCount, listbox1_vis);
	ac++;
	XtSetArg (arglist[ac], XmNitems, NULL);
	ac++;
	XtSetArg (arglist[ac], XmNitemCount, 0);
	ac++;
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, box->label2);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.text_font_list);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	box->listbox1 = XmCreateScrolledList (box->box, "", arglist, ac);

	/*
	** Create the goto button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.goto_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, goto_callback);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
 	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox1));
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
	XtSetArg (arglist[ac], XtNsensitive, FALSE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button1 = XmCreatePushButton (box->box, "", arglist, ac);

	/*
	** Create the visit button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.visit_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, visit_callback);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNleftWidget, box->button1);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox1));
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
	XtSetArg (arglist[ac], XtNsensitive, FALSE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button2 = XmCreatePushButton (box->box, "", arglist, ac);

	/*
	** Create the dismiss button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.dismiss_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, dismiss_callback);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox1));
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button3 = XmCreatePushButton (box->box, "", arglist, ac);


	/*
	** Manage all children.
	*/
	XtManageChild(box->listbox1);
        XtManageChildren (XtChildren((XmBulletinBoardWidget)box->box),
			  XtNumChildren((XmBulletinBoardWidget)box->box));

	/*
	** Compute the buttons width and set it.
	*/
	button_width = MAX(button_width, XtWidth(box->button0));
	button_width = MAX(button_width, XtWidth(box->button1));
	button_width = MAX(button_width, XtWidth(box->button2));
	button_width = MAX(button_width, XtWidth(box->button3));
	ac = 0;
	XtSetArg (arglist[ac], XmNrecomputeSize, FALSE);
	ac++;
	XtSetArg (arglist[ac], XtNwidth, button_width);
	ac++;
	XtSetValues (box->button0, arglist, ac);
	XtSetValues (box->button1, arglist, ac);
	XtSetValues (box->button2, arglist, ac);
	XtSetValues (box->button3, arglist, ac);

	/*
	** Compute the topic titles label's width and set it.
	*/
	max_width = XtWidth(box->label1) + XtWidth(box->text)
		    + XtWidth(box->button0) + (hoffset * 4);
	ac = 0;
	XtSetArg (arglist[ac], XtNwidth, max_width);
	ac++;
	XtSetValues (box->label2, arglist, ac);

	/*
	** Set the default button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNdefaultButton, box->button0);
	ac++;
	XtSetArg (arglist[ac], XmNcancelButton, box->button3);
	ac++;
	XtSetValues (box->box, arglist, ac);

	/*
	** Store the box record into the help widget.
	*/
	help_widget->dxmhelp.title_box = box;
	
	/*
	** Realize the title box if not already realized.
	*/
	if (!XtIsRealized(help_widget->dxmhelp.title_box->box))
	    XtRealizeWidget (help_widget->dxmhelp.title_box->box);
	/*
	** Position the box using the default algorythm.
	*/
	if (!XtIsManaged(help_widget->dxmhelp.title_box->box))
	    DXmPositionWidget (help_widget->dxmhelp.title_box->box,
			(Widget *)&help_widget, 1);

	/*
	** Popup the title box.
	*/
	XtManageChild (help_widget->dxmhelp.title_box->box);

    }
    
    else {

	/*
	**  Make the box flash if it is already on the screen
	*/
	
	if (XtIsManaged(help_widget->dxmhelp.title_box->box)) {
	    HwRaiseWindow(help_widget->dxmhelp.title_box->box);
	    time = ((XButtonEvent *) reason->event)->time;
	    XtCallAcceptFocus(help_widget->dxmhelp.title_box->box, &time);
	}
	else
	    XtManageChild (help_widget->dxmhelp.title_box->box);
    }

    HwRemoveAllWaitCursor(help_widget);
    
}


static void search_keyword (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the apply button of the keyword box:
**	search for topics which match the keyword entered by the user.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    XmString cs_keyword;

    char *keyword;
    Arg arglist[20];
    int ac = 0;
    char **topic_array = NULL;
    char **title_array = NULL;
    int topic_count;
    int count_save;
    int i;
    XmStringCharSet  char_set;
    Boolean direction;
#ifdef GENERAL_TEXT
    long count,stat;
#endif /* GENERAL_TEXT */

    /*
    **  Turn on the wait cursors
    */
    
    HwDisplayAllWaitCursor(help_widget);
	
    /*
    ** Get the string entered by the user.
    */
    
#ifdef CSTEXT
    cs_keyword = (XmString) DXmCSTextGetString(help_widget->dxmhelp.keyword_box->text);
    keyword = (char *) DXmCvtCStoFC(cs_keyword,&count,&stat);
    XtFree((char *)cs_keyword);
#else
    keyword = XmTextGetString (help_widget->dxmhelp.keyword_box->text);
#endif /* CSTEXT */
    if (keyword != NULL)
    {
	/*
	** Search for topics matching the keyword string.
	*/
	
	DXmHelp__search_keyword (help_widget->dxmhelp.library_context,
				keyword,
			        &topic_array,
				&title_array,
			        &topic_count);
	/*
	** Free the previous searchkeyword topic array.
	*/
	if (help_widget->dxmhelp.searchkeyword_topic_array != NULL)
	{
	    XtSetArg (arglist[0], XmNitemCount, &count_save);
	    XtGetValues (help_widget->dxmhelp.keyword_box->listbox2, arglist, 1);
	    free_array (help_widget->dxmhelp.searchkeyword_topic_array,
			count_save);
	    help_widget->dxmhelp.searchkeyword_topic_array = NULL;
	}

	if (topic_count != 0)
	{

	    /*
	    **  The search operation was successful
	    */
		    	    
	    help_widget->dxmhelp.searchkeyword_topic_array = topic_array;
	    
	    /*
	    ** Update the topic titles listbox.
	    */
	    
	    XtSetArg (arglist[ac], XmNitems, title_array);
	    ac++;
	    XtSetArg (arglist[ac], XmNitemCount, topic_count);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.keyword_box->listbox2, arglist, ac);
          /* Need to pre-def a selection of some sort for DefaultAction =kbm */
            XmListSelectPos (help_widget->dxmhelp.keyword_box->listbox2,
  (((XmListWidget )help_widget->dxmhelp.keyword_box->listbox2)->list.itemCount 
                > 1 ? 1 : 0), False);
            XmListSetPos (help_widget->dxmhelp.keyword_box->listbox2, 1); 

	    /*
	    ** Set the goto and visist buttons to insensitive.
	    */
	    
	    XtSetSensitive (help_widget->dxmhelp.keyword_box->button1, FALSE);
	    XtSetSensitive (help_widget->dxmhelp.keyword_box->button2, FALSE);

	    /*
	    ** Free the keyword string and the arrays.
	    */
	    
	    XtFree (keyword);
	    free_array (title_array, topic_count);
	}
	else
	/*
	** Error: the given keyword doesn't exist.
	*/
	{

	    /*
	    **  Clear the titles list box
	    */
		    
	    XtSetArg (arglist[ac], XmNitemCount, 0); ac++;
	    XtSetValues (help_widget->dxmhelp.keyword_box->listbox2,
		arglist, ac);

	    /*
	    **  Set up the error message widget
	    */
		    
#ifdef GENERAL_TEXT
	    cs_keyword = (XmString) DXmCvtFCtoCS(keyword, &count, &stat);
#else
	    cs_keyword = XmStringLtoRCreate(keyword, "ISO8859-1");
#endif /* GENERAL_TEXT */
	    set_message(help_widget, help_widget->dxmhelp.nokeyword_message,
		cs_keyword);
	    XtFree((char *)cs_keyword);
	    
	    /*
	    ** Popup the message box.
	    */
	    
	    check_message (help_widget, help_widget->dxmhelp.keyword_box->box);
	}
    }

    HwRemoveAllWaitCursor(help_widget);

}

static void select_keyword(listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Single select callback routine for the keyword listbox: get the selected
**	keyword in the listbox.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;  
    char *keyword;
	
    if (list->item_position != 0)
    {
	/*
	** Get the selected keyword and store it into the help widget's record.
	*/
	help_widget->dxmhelp.selected_keyword = XmStringCopy(list->item);
    
#ifdef CSTEXT
	/*
	** Set the value of the text widget to be the selected keyword.
	*/
        DXmCSTextSetString (help_widget->dxmhelp.keyword_box->text,
                          help_widget->dxmhelp.selected_keyword);
#else
	/*
	** Extract the keyword string from the compound string.
	*/
	get_string_from_cs (help_widget->dxmhelp.selected_keyword,&keyword)
	/*
	** Set the value of the text widget to be the selected keyword.
	*/
	XmTextSetString (help_widget->dxmhelp.keyword_box->text,
			  keyword);
	/*
	** Free the string.
	*/
	if (keyword)
	     XtFree (keyword);

#endif /* CSTEXT */
    }
    
}

static void search_select_keyword(listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Select and Confirm callback routine for the keyword listbox: get the
**	selected keyword and search titles for it.
**
**  Keywords:
**	CAllback routine
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;  
    char *keyword;
	
    if (list->item_position != 0)
    {
	/*
	** Get the selected keyword and store it into the help widget's record.
	*/
        help_widget->dxmhelp.selected_keyword = XmStringCopy(list->item);
#ifdef CSTEXT
        /*
        ** Set the value of the text widget to be the selected keyword.
        */
        DXmCSTextSetString (help_widget->dxmhelp.keyword_box->text,
                          help_widget->dxmhelp.selected_keyword);
#else

	/*
	** Extract the keyword string from the compound string.
	*/
	get_string_from_cs (help_widget->dxmhelp.selected_keyword,&keyword);
	/*
	** Set the value of the text widget to be the selected keyword.
	*/
	XmTextSetString (help_widget->dxmhelp.keyword_box->text,
			  keyword);
	/*
	** Free the string.
	*/
	if (keyword)
	    XtFree (keyword);
#endif /* CSTEXT */
	/*
	** Search for titles associated to the keyword.
	*/
	search_keyword ((Widget) NULL, help_widget);
    }
}



static void select_keyword_title(listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Single select callback routine for the title listbox: get the selected
**	title in the listbox.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;  

    if (list->item_position != 0)
    {
	/*
	** Get the topic associated with the selected title.
	*/
        help_widget->dxmhelp.selected_searchkeyword_topic =
	    XmStringCopy((XmString) help_widget->dxmhelp.searchkeyword_topic_array[list->item_position - 1]);

	/*
	** Set the goto and visit buttons to sensitive.
	*/
	XtSetSensitive (help_widget->dxmhelp.keyword_box->button1, TRUE);
	XtSetSensitive (help_widget->dxmhelp.keyword_box->button2, TRUE);
    }

}

static void goto_select_keyword_title(listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Select and Confirm callback routine for the title listbox: get the
**	selected title and go to it.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    if (list->item_position != 0)
    {
	/*
	** Get the topic associated with the selected title.
	*/
        help_widget->dxmhelp.selected_searchkeyword_topic =
	    XmStringCopy((XmString) help_widget->dxmhelp.searchkeyword_topic_array[list->item_position - 1]);
		    
	/*
	** Set the goto and visit buttons to sensitive.
	*/
	XtSetSensitive (help_widget->dxmhelp.keyword_box->button1, TRUE);
	XtSetSensitive (help_widget->dxmhelp.keyword_box->button2, TRUE);

	/*
	** Go to the frame associated with the selected searchkeyword topic.
	*/
        goto_keyword_title (NULL, help_widget);
    }
    
}

static void goto_keyword_title (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the goto button of the search keyword
**	dialog box: go to the selected topic.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    int comp = 1;

    /*
    **  Turn the wait cursors on
    */
    
    HwDisplayAllWaitCursor(help_widget);
    
    if (help_widget->dxmhelp.selected_searchkeyword_topic!= NULL)
    {
	/*
	** Check if the selected searchkeyword topic is different from the one on top of the
	** trail list.
	*/
	if ((help_widget->dxmhelp.trail != NULL) &&
	    (help_widget->dxmhelp.trail->topic != NULL))
	    comp = !XmStringCompare ((XmString) help_widget->dxmhelp.trail->topic,
		        (XmString) help_widget->dxmhelp.selected_searchkeyword_topic);
	if (comp != 0)
	{
	    /*
	    ** Push the selected searchkeyword topic on top of the trail list.
	    */   
	    insert_trail(help_widget,
			 help_widget->dxmhelp.selected_searchkeyword_topic);
	    /*
	    ** Free the previous selected topic and title.
	    */
	    XtFree ((char *)help_widget->dxmhelp.selected_topic);
	    help_widget->dxmhelp.selected_topic = NULL;
	    XtFree ((char *)help_widget->dxmhelp.selected_title);
	    help_widget->dxmhelp.selected_title = NULL;
	    /*
	    ** Get the help frame and update the text widget and listbox.
	    */
	    get_help(help_widget);
	}
    }

    if (XtIsManaged(help_widget))
	HwRaiseWindow(help_widget);

    HwRemoveAllWaitCursor(help_widget);
    
}

static void visit_keyword_title(button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the visit button of the keyword box: visit
**	the selected topic.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    **  Turn on the wait cursor
    */

    HwDisplayAllWaitCursor(help_widget);
	
    /*
    ** Create a new help widget or reuse and update an existing one.
    */
    if (help_widget->dxmhelp.selected_searchkeyword_topic!= NULL)
	visit_help (help_widget,
		    help_widget->dxmhelp.selected_searchkeyword_topic,
		    help_widget->dxmhelp.library);

    HwRemoveAllWaitCursor(help_widget);
}
	

static void dismiss_keyword (button, w)
    Widget button;
    Widget w;
/*
**++
**  Functional Description:
**	Activate callback routine for the dismiss button of the keyword box:
**	unmanage the keyword box.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    /*
    ** Unmanage the keyword box.
    */
    XtUnmanageChild (help_widget->dxmhelp.keyword_box->box);
}

static int get_keywords(help_widget)
    DXmHelpWidget help_widget;
/*
**++
**  Functional Description:
**	Create the keyword array for the current library.
**
**  Keywords:
**	Help creation, help update.
**
**  Arguments:
**	help_widget: widget id of the help_widget.
**
**  Result:
**	Integer: keyword count.
**
**  Exceptions:
**	None
**--
*/
{
    int keyword_count = 0;
    int i;

    /*
    ** Get the complete list of keywords from the help library.
    */
    
    DXmHelp__get_keyword (help_widget->dxmhelp.library_context,
			  &help_widget->dxmhelp.keyword_array,
			  &keyword_count);
			  
    /*
    ** Return number of keywords.
    */
    
    return keyword_count;
}


static void build_search_keyword (button, w, reason)
    Widget button;
    Widget w;
    Reason reason;

/*
**++
**  Functional Description:
**	Activate callback routine for the keywords menu item in the search menu:
**	create the keyword dialog box and manage it.
**
**  Keywords:
**	Callback routine
**
**  Arguments:
**	button: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    Arg arglist[20];
    Time time;
    DXmHelpBox *box;
    int ac = 0;
    int listbox1_vis = 5;
    int listbox2_vis = 5;
    int hoffset = 10;
    int voffset = 10;
    int max_width = 0;
    int button_width = 0;
    int keyword_count = 0;
#ifdef CSTEXT
    char *help_text_buf = NULL ;
    long count,stat;
#endif /* CSTEXT */
    XtCallbackRec search_keyword_callback[2];
    XtCallbackRec select_keyword_callback[2];
    XtCallbackRec search_select_keyword_callback[2];
    XtCallbackRec select_callback[2];
    XtCallbackRec select_go_callback[2];
    XtCallbackRec goto_callback[2];
    XtCallbackRec visit_callback[2];
    XtCallbackRec dismiss_callback[2];
    XtCallbackRec help_callback[2];

    HwDisplayAllWaitCursor(help_widget);
	
    if (help_widget->dxmhelp.keyword_box == NULL)
    {

	/*
	** Initialization.
	*/
    
	search_keyword_callback[0].callback = (XtCallbackProc) search_keyword;
	search_keyword_callback[0].closure = (XtPointer) help_widget;
	search_keyword_callback[1].callback = NULL;
	select_keyword_callback[0].callback = (XtCallbackProc) select_keyword;
	select_keyword_callback[0].closure = (XtPointer) help_widget;
	select_keyword_callback[1].callback = NULL;
	search_select_keyword_callback[0].callback = (XtCallbackProc) search_select_keyword;
	search_select_keyword_callback[0].closure = (XtPointer) help_widget;
	search_select_keyword_callback[1].callback = NULL;
	select_callback[0].callback = (XtCallbackProc) select_keyword_title;
	select_callback[0].closure = (XtPointer) help_widget;
	select_callback[1].callback = NULL;
	select_go_callback[0].callback = (XtCallbackProc) goto_select_keyword_title;
	select_go_callback[0].closure = (XtPointer) help_widget;
	select_go_callback[1].callback = NULL;
	goto_callback[0].callback = (XtCallbackProc ) goto_keyword_title;
	goto_callback[0].closure  = (XtPointer) help_widget;
	goto_callback[1].callback = NULL;
	visit_callback[0].callback = (XtCallbackProc ) visit_keyword_title;
	visit_callback[0].closure  = (XtPointer) help_widget;
	visit_callback[1].callback = NULL;
	dismiss_callback[0].callback = (XtCallbackProc ) dismiss_keyword;
	dismiss_callback[0].closure  = (XtPointer) help_widget;
	dismiss_callback[1].callback = NULL;
	help_callback[0].callback = (XtCallbackProc ) help_keyword;
	help_callback[0].closure  = (XtPointer) help_widget;
	help_callback[1].callback = NULL;

	/*
	** Initialize the box record.
	*/
	box = (DXmHelpBox *) XtMalloc(sizeof(DXmHelpBox));

	/*
	** Create the attached dialog box.
	*/
	XtSetArg (arglist[ac], XmNdialogStyle, XmDIALOG_MODELESS);
	ac++;
	XtSetArg (arglist[ac], XmNdefaultPosition, FALSE);
	ac++;
/*	XtSetArg (arglist[ac], DXmNtakeFocus, TRUE);
	ac++; */
	XtSetArg (arglist[ac], XmNautoUnmanage, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNdialogTitle,
		  help_widget->dxmhelp.searchkeyword_box_label);
	ac++;
	XtSetArg (arglist[ac], XmNbuttonFontList,
	    help_widget->dxmhelp.button_font_list); ac++;
	XtSetArg (arglist[ac], XmNlabelFontList,
	    help_widget->dxmhelp.label_font_list); ac++;
	XtSetArg (arglist[ac], XmNtextFontList,
	    help_widget->dxmhelp.text_font_list); ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNrubberPositioning, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNhorizontalSpacing, hoffset);
	ac++;
	XtSetArg (arglist[ac], XmNverticalSpacing, voffset);
	ac++;
	XtSetArg (arglist[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg (arglist[ac], XmNnoResize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	box->box = (Widget) 
	    XmCreateFormDialog ((Widget) help_widget,"keywords", arglist, ac);
	
	/*
	** Create the keyword label.
	*/
	ac = 0;    
	XtSetArg (arglist[ac], XmNborderWidth, 0);
	ac++;
	XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
	ac++;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.keywords_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.label_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
#ifdef RTOL
	XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
 	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	box->label1 = XmCreateLabel (box->box, "", arglist, ac);

	/*
	** Create the apply button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.apply_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, search_keyword_callback);
	ac++;
#ifdef RTOL
	XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	box->button0 = XmCreatePushButton (box->box, "", arglist, ac);


	/*
	** Create the keyword text widget.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNresizeWidth, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNresizeHeight, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNwordWrap, FALSE);
	ac++;
	XtSetArg (arglist[ac], XmNeditable, TRUE);
	ac++;
	XtSetArg (arglist[ac], XmNborderWidth, 1);
	ac++;
	XtSetArg (arglist[ac], XmNcolumns, 30);
	ac++;
	XtSetArg (arglist[ac], XmNrows, 1);
	ac++;
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNleftWidget, box->label1);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.text_font_list);
	ac++;
#ifdef CSTEXT
	help_text_buf = (char *)DXmCvtFCtoCS("",&count,&stat); 
        XtSetArg (arglist[ac], XmNvalue, help_text_buf);
        ac++;
#endif /* CSTEXT */
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
        ac++;
#endif /* RTOL */
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	    
#ifdef CSTEXT
	box->text = (Widget) DXmCreateCSText(box->box, "", arglist, ac);
	XtFree(help_text_buf);
#else
	box->text = XmCreateText(box->box, "", arglist, ac);
#endif /* CSTEXT */
	/*
	** Get the complete list of keywords from the help library.
	*/
	keyword_count = get_keywords(help_widget);
	
	/*
	** Create the keyword listbox.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNbrowseSelectionCallback, select_keyword_callback);
	ac++;
	XtSetArg (arglist[ac], XmNdefaultActionCallback,
		  search_select_keyword_callback);
	ac++;
	XtSetArg (arglist[ac], XmNlistSizePolicy, XmVARIABLE);
	ac++;
	XtSetArg (arglist[ac], XmNscrollBarDisplayPolicy, XmSTATIC);
	ac++;
/*  ??  These next two should probably be set in a resolution independent unit */
	XtSetArg (arglist[ac], XmNmarginHeight, 3);
	ac++;
	XtSetArg (arglist[ac], XmNlistSpacing, 3);
	ac++;
	XtSetArg (arglist[ac], XmNvisibleItemCount, listbox1_vis);
	ac++;
	XtSetArg (arglist[ac], XmNitems, help_widget->dxmhelp.keyword_array);
	ac++;
	XtSetArg (arglist[ac], XmNitemCount, keyword_count);
	ac++;
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, box->button0);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.text_font_list);
	ac++;
#ifdef RTOL
	XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	
	box->listbox1 = XmCreateScrolledList (box->box, "", arglist, ac);

	/*
	** Create the topic titles label.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNborderWidth, 0);
	ac++;
	XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
	ac++;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.topic_titles_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.label_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
#ifdef RTOL
	XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
	ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox1));
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	box->label2 = XmCreateLabel (box->box, "", arglist, ac);
	/*
	** Create the title listbox.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNbrowseSelectionCallback, select_callback);
	ac++;
	XtSetArg (arglist[ac], XmNdefaultActionCallback, select_go_callback);
	ac++;
	XtSetArg (arglist[ac], XmNlistSizePolicy, XmVARIABLE);
	ac++;
	XtSetArg (arglist[ac], XmNscrollBarDisplayPolicy, XmSTATIC);
	ac++;
/*  ??  These next two should probably be set in a resolution independent unit */
	XtSetArg (arglist[ac], XmNmarginHeight, 3);
	ac++;
	XtSetArg (arglist[ac], XmNlistSpacing, 3);
	ac++;
	XtSetArg (arglist[ac], XmNvisibleItemCount, listbox2_vis);
	ac++;
	XtSetArg (arglist[ac], XmNitems, NULL);
	ac++;
	XtSetArg (arglist[ac], XmNitemCount, 0);
	ac++;
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, box->label2);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.text_font_list);
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
        ac++;
#endif /* RTOL */
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	
	box->listbox2 = XmCreateScrolledList (box->box, "", arglist, ac);

	/*
	** Create the goto button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.goto_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, goto_callback);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox2));
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
        ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
	XtSetArg (arglist[ac], XtNsensitive, FALSE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button1 = XmCreatePushButton (box->box, "", arglist, ac);

	/*
	** Create the visit button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.visit_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, visit_callback);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox2));
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
        ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNleftWidget, box->button1);
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
	XtSetArg (arglist[ac], XtNsensitive, FALSE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button2 = XmCreatePushButton (box->box, "", arglist, ac);

	/*
	** Create the dismiss button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.dismiss_label);
	ac++;
	XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
	ac++;
	XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
	ac++;
	XtSetArg (arglist[ac], XmNactivateCallback, dismiss_callback);
	ac++;
	XtSetArg (arglist[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg (arglist[ac], XmNtopWidget, XtParent(box->listbox2));
	ac++;
#ifdef RTOL
        XtSetArg (arglist[ac], DXmNlayoutDirection,LayoutDirection(help_widget));
        ac++;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	    ac++;
	}
	XtSetArg (arglist[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	box->button3 = XmCreatePushButton (box->box, "", arglist, ac);

	/*
	** Manage all children.
	*/
	XtManageChild(box->listbox1);
	XtManageChild(box->listbox2);
        XtManageChildren (XtChildren((XmBulletinBoardWidget)box->box),
			  XtNumChildren((XmBulletinBoardWidget)box->box));

	/*
	** Compute the buttons' width and set it.
	*/
	button_width = MAX(button_width, XtWidth(box->button0));
	button_width = MAX(button_width, XtWidth(box->button1));
	button_width = MAX(button_width, XtWidth(box->button2));
	button_width = MAX(button_width, XtWidth(box->button3));
	ac = 0;
	XtSetArg (arglist[ac], XmNrecomputeSize, FALSE);
	ac++;
	XtSetArg (arglist[ac], XtNwidth, button_width);
	ac++;
	XtSetValues (box->button0, arglist, ac);
	XtSetValues (box->button1, arglist, ac);
	XtSetValues (box->button2, arglist, ac);
	XtSetValues (box->button3, arglist, ac);

	/*
	** Compute the topic titles label width and set it.
	*/
	max_width = XtWidth(box->label1) + XtWidth(box->text)
		    + XtWidth(box->button0) + (hoffset * 4);
	ac = 0;
	XtSetArg (arglist[ac], XtNwidth, max_width);
	ac++;
	XtSetValues (box->label2, arglist, ac);

	/*
	** Set the default button.
	*/
	ac = 0;
	XtSetArg (arglist[ac], XmNdefaultButton, box->button0);
	ac++;
	XtSetArg (arglist[ac], XmNcancelButton, box->button3);
	ac++;
	XtSetValues (box->box, arglist, ac);
    

	/*
	** Store the box record into the help widget.
	*/
	help_widget->dxmhelp.keyword_box = box;
	
	/*
	** Realize the keyword box if not already realized.
	*/
	if (!XtIsRealized(help_widget->dxmhelp.keyword_box->box))
	    XtRealizeWidget (help_widget->dxmhelp.keyword_box->box);
	/*
	** Position the keyword box using the default algorythm.
	*/
	if (!XtIsManaged(help_widget->dxmhelp.keyword_box->box))
	    DXmPositionWidget (help_widget->dxmhelp.keyword_box->box,
			(Widget *) &help_widget, 1);
	/*
	** Popup the keyword box.
	*/
	XtManageChild (help_widget->dxmhelp.keyword_box->box);

    }
    else {

	/*
	**  Pop the box if already on the screen
	*/
	
	if (XtIsManaged(help_widget->dxmhelp.keyword_box->box)) {
	    HwRaiseWindow(help_widget->dxmhelp.keyword_box->box);
	    time = ((XButtonEvent *) reason->event)->time;
	    XtCallAcceptFocus(help_widget->dxmhelp.keyword_box->box, &time); 
	}	    
	else
	    XtManageChild (help_widget->dxmhelp.keyword_box->box);
    }

    HwRemoveAllWaitCursor(help_widget);
}
    

static Boolean check_library(library)
    XmString library;
/*
**++
**  Functional Description:
**	Compare the given library to the help on help library.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	library: library to compare.
**
**  Result:
**	Boolean: true if libraries are different.
**
**  Exceptions:
**	None
**--
*/
{
    XmString help_library;
    Boolean status;

    /*
    ** Set the help library.
    */
    help_library = XmStringLtoRCreate(LIBRARY, "ISO8859-1");

    /*
    ** Compare the libraries.
    */
    if (library == NULL)
	status = TRUE;
    else
	if (!XmStringCompare(library, help_library))
	    status = TRUE;
	else
	    status = FALSE;
	
    /*
    ** Free string storage.
    */
    XtFree ((char *)help_library);

    return (status);
}


static Widget build_view_menu (menubar)
    Widget menubar;
/*
**++
**  Functional Description:
**	Create the help view menu.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	menubar: widget id of the menubar.
**
**  Result:
**	Widget id of the view pulldown menu.
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget;
    XmRowColumnWidget menu;
    Arg arglist[20];
    Arg arglist_2[1];
    int ac = 0;
        
    XtCallbackRec goto_callback[2] ;
    XtCallbackRec goback_callback[2] ;
    XtCallbackRec visit_callback[2];
    XtCallbackRec overview_callback[2] ;
    XtCallbackRec glossary_callback[2];
    XtCallbackRec help_callback[2];
    XtCallbackRec help_goto_callback[2];
    XtCallbackRec help_goback_callback[2];
    XtCallbackRec help_visit_callback[2];
    XtCallbackRec help_overview_callback[2];
    XtCallbackRec help_glossary_callback[2];
    XtCallbackRec pulling_callback[2];
    
    /*
    ** Initialization.
    */
    help_widget = (DXmHelpWidget) XtParent (menubar);

    goto_callback[0].callback = (XtCallbackProc ) go_to;
    goto_callback[0].closure  = (XtPointer) help_widget;
    goto_callback[1].callback = NULL;
    goback_callback[0].callback = (XtCallbackProc ) goback;
    goback_callback[0].closure  = (XtPointer) help_widget;
    goback_callback[1].callback = NULL;
    visit_callback[0].callback = (XtCallbackProc ) visit;
    visit_callback[0].closure  = (XtPointer) help_widget;
    visit_callback[1].callback = NULL;
    overview_callback[0].callback = (XtCallbackProc ) goto_overview;
    overview_callback[0].closure  = (XtPointer) help_widget;
    overview_callback[1].callback = NULL;
    glossary_callback[0].callback = (XtCallbackProc ) visit_glossary;
    glossary_callback[0].closure  = (XtPointer) help_widget;
    glossary_callback[1].callback = NULL;
    help_callback[0].callback = (XtCallbackProc ) help_view;
    help_callback[0].closure  = (XtPointer) help_widget;
    help_callback[1].callback = NULL;
    help_goto_callback[0].callback = (XtCallbackProc ) help_view_goto;
    help_goto_callback[0].closure  = (XtPointer) help_widget;
    help_goto_callback[1].callback = NULL;
    help_goback_callback[0].callback = (XtCallbackProc ) help_view_goback;
    help_goback_callback[0].closure  = (XtPointer) help_widget;
    help_goback_callback[1].callback = NULL;
    help_visit_callback[0].callback = (XtCallbackProc ) help_view_visit;
    help_visit_callback[0].closure  = (XtPointer) help_widget;
    help_visit_callback[1].callback = NULL;
    help_overview_callback[0].callback = (XtCallbackProc ) help_view_overview;
    help_overview_callback[0].closure  = (XtPointer) help_widget;
    help_overview_callback[1].callback = NULL;
    help_glossary_callback[0].callback = (XtCallbackProc ) help_view_glossary;
    help_glossary_callback[0].closure  = (XtPointer) help_widget;
    help_glossary_callback[1].callback = NULL;
    pulling_callback[0].callback = (XtCallbackProc ) pulling_view;
    pulling_callback[0].closure  = (XtPointer) help_widget;
    pulling_callback[1].callback = NULL;

    /*
    ** Create the view pulldown menu.
    */  
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
    XtSetArg (arglist[ac], XmNorientation, XmVERTICAL);
    ac++;
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;    
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;    
    XtSetArg (arglist[ac], XmNtearOffModel, XmTEAR_OFF_ENABLED);
    ac++;    
    menu = (XmRowColumnWidget) XmCreatePulldownMenu(menubar, "viewmenu", arglist, ac);

    /*
    ** Create the view menu entry and manage it.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.view_menu_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.view_label_mnem);
    ac++;
    if (help_widget->dxmhelp.view_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.view_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNsubMenuId, menu);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	ac++;
    }
    XtSetArg (arglist[ac], XmNcascadingCallback, pulling_callback);
    ac++;

    help_widget->dxmhelp.view_menu =
	XmCreateCascadeButton(menubar, "viewentry", arglist, ac);
    XtManageChild (help_widget->dxmhelp.view_menu);

    /*
    ** Create the goto menu item.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.goto_topic_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.goto_topic_label_mnem);
    ac++;
    if (help_widget->dxmhelp.goto_topic_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.goto_topic_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, goto_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_goto_callback);
	ac++;
    }
    XtSetArg (arglist[ac], XtNsensitive, FALSE);
    ac++;
    help_widget->dxmhelp.goto_button =
	    XmCreatePushButtonGadget ((Widget) menu, "gotobutton", arglist, ac);
	    
    /*
    ** Create the goback menu item.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.goback_topic_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.goback_label_mnem);
    ac++;
    if (help_widget->dxmhelp.goback_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.goback_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, goback_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_goback_callback);
	ac++;
    }
    help_widget->dxmhelp.goback_button =
	XmCreatePushButtonGadget ((Widget) menu, "gobackbutton", arglist, ac);
	
    /*
    ** Create the goto overview menu item.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.goover_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.goover_label_mnem);
    ac++;
    if (help_widget->dxmhelp.goover_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.goover_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, overview_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_overview_callback);
	ac++;
    }
    help_widget->dxmhelp.overview_button =
	XmCreatePushButtonGadget ((Widget) menu, "gooverbutton", arglist, ac);

    /*
    ** Create a separator.
    */
    XtSetArg (arglist_2[0], XmNorientation, XmHORIZONTAL);
    XmCreateSeparatorGadget ((Widget) menu, "", arglist_2, 1);
    
    /*
    ** Create the visit menu item.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.visit_topic_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.visit_topic_label_mnem);
    ac++;
    if (help_widget->dxmhelp.visit_topic_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.visit_topic_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, visit_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_visit_callback);
	ac++;
    }
    XtSetArg (arglist[ac], XtNsensitive, FALSE);
    ac++;
    help_widget->dxmhelp.visit_button =
	XmCreatePushButtonGadget ((Widget) menu, "visitbutton", arglist, ac);

    /*
    ** Create the visit glossary menu item if a glossary topic was specified .
    */
    if 	(help_widget->dxmhelp.glossary != NULL)
    {
#ifdef RTOL
    	ac = 4;
#else
    	ac = 3;
#endif /* RTOL */
	XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.visitglos_label);
	ac++;
	XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.visitglos_label_mnem);
	ac++;
        if (help_widget->dxmhelp.visitglos_label_mnem_cs)
            {
	    XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		      help_widget->dxmhelp.visitglos_label_mnem_cs);
	    ac++;
	    }
	XtSetArg (arglist[ac], XmNactivateCallback, glossary_callback);
	ac++;
    	if (check_library(help_widget->dxmhelp.library))
	{
	    XtSetArg(arglist[ac], XmNhelpCallback, help_glossary_callback);
	    ac++;
	}
	help_widget->dxmhelp.glossary_button =
	    XmCreatePushButtonGadget ((Widget) menu, "visitglosbutton", arglist, ac);
    }
    else
	help_widget->dxmhelp.glossary_button = NULL;

    	
    /*
    ** Manage all children of the pulldown menu.
    */
    XtManageChildren (XtChildren(menu), XtNumChildren(menu));   

    return ((Widget)menu);        
}


static Widget build_file_menu (menubar)
    Widget menubar;
/*
**++
**  Functional Description:
**	Build the file menu.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	menubar: widget id of the menubar.
**
**  Result:
**	Widget id of the file pulldown menu.
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget ;
    XmRowColumnWidget menu;
    Arg arglist[20];
    int ac = 0;
    
    XtCallbackRec saveas_callback[2];
    XtCallbackRec quit_callback[2];
    XtCallbackRec pulling_callback[2];
    XtCallbackRec help_callback[2];

    /*
    ** Initialization.
    */
    help_widget = (DXmHelpWidget) XtParent(menubar);

    saveas_callback[0].callback = (XtCallbackProc) build_saveas;
    saveas_callback[0].closure = (XtPointer) help_widget;
    saveas_callback[1].callback = NULL;
    quit_callback[0].callback = (XtCallbackProc) quit;
    quit_callback[0].closure = (XtPointer) help_widget;
    quit_callback[1].callback = NULL;
    help_callback[0].callback = (XtCallbackProc ) help_file;
    help_callback[0].closure  = (XtPointer) help_widget;
    help_callback[1].callback = NULL;
    pulling_callback[0].callback = (XtCallbackProc ) pulling_file;
    pulling_callback[0].closure  = (XtPointer) help_widget;
    pulling_callback[1].callback = NULL;

    /*
    ** Create the file pulldown menu.
    */
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
    XtSetArg (arglist[ac], XmNorientation, XmVERTICAL);
    ac++;
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;    
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;    
    XtSetArg (arglist[ac], XmNtearOffModel, XmTEAR_OFF_ENABLED);
    ac++;    
    menu = (XmRowColumnWidget) XmCreatePulldownMenu(menubar, "filemenu", arglist, ac);

    /*
    ** Create the file menu entry and manage it.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.file_menu_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.file_label_mnem);
    ac++;
    if (help_widget->dxmhelp.file_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.file_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNsubMenuId, menu);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	ac++;
    }
    XtSetArg (arglist[ac], XmNcascadingCallback, pulling_callback);
    ac++;
    help_widget->dxmhelp.file_menu = 
	XmCreateCascadeButton(menubar, "fileentry", arglist, ac);
    XtManageChild (help_widget->dxmhelp.file_menu);

    /*
    ** Create the save as menu item.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.saveas_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.saveas_label_mnem);
    ac++;
    if (help_widget->dxmhelp.saveas_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.saveas_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, saveas_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	ac++;
    }
    help_widget->dxmhelp.saveas_button =
	XmCreatePushButtonGadget ((Widget) menu, "saveasbutton", arglist, ac);

    /*
    ** Create the exit menu item.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.exit_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.exit_label_mnem);
    ac++;
    if (help_widget->dxmhelp.exit_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.exit_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, quit_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	ac++;
    }
    help_widget->dxmhelp.exit_button = 
	XmCreatePushButtonGadget ((Widget) menu, "exitbutton", arglist, ac);

    /*
    ** Manage all the children of the pulldown menu.
    */
    XtManageChildren (XtChildren(menu), XtNumChildren(menu));   

    return ((Widget)menu);        
}

static Widget build_edit_menu (menubar)
    Widget menubar;
/*
**++
**  Functional Description:
**	Build the edit menu.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	menubar: widget id of the menubar.
**
**  Result:
**	Widget id of the edit pulldown menu.
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget;
    XmRowColumnWidget menu;
    Arg arglist[20];
    int ac = 0;
    
    XtCallbackRec selectall_callback[2];
    XtCallbackRec copy_callback[2];
    XtCallbackRec pulling_callback[2];
    XtCallbackRec help_callback[2];
    XtCallbackRec help_copy_callback[2];
    XtCallbackRec help_selectall_callback[2];

    /*
    ** Initialization.
    */
    help_widget = (DXmHelpWidget) XtParent(menubar);
    
    selectall_callback[0].callback = (XtCallbackProc) selectall;
    selectall_callback[0].closure = (XtPointer) help_widget;
    selectall_callback[1].callback = NULL;
    copy_callback[0].callback = (XtCallbackProc) copy_clipboard;
    copy_callback[0].closure = (XtPointer) help_widget;
    copy_callback[1].callback = NULL;
    help_callback[0].callback = (XtCallbackProc ) help_edit;
    help_callback[0].closure  = (XtPointer) help_widget;
    help_callback[1].callback = NULL;
    help_copy_callback[0].callback = (XtCallbackProc ) help_edit_copy;
    help_copy_callback[0].closure  = (XtPointer) help_widget;
    help_copy_callback[1].callback = NULL;
    help_selectall_callback[0].callback = (XtCallbackProc ) help_edit_selectall;
    help_selectall_callback[0].closure  = (XtPointer) help_widget;
    help_selectall_callback[1].callback = NULL;
    pulling_callback[0].callback = (XtCallbackProc ) pulling_edit;
    pulling_callback[0].closure  = (XtPointer) help_widget;
    pulling_callback[1].callback = NULL;

    /*
    ** Create the edit pulldown menu.
    */
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
    XtSetArg (arglist[ac], XmNorientation, XmVERTICAL);
    ac++;
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;    
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;    
    XtSetArg (arglist[ac], XmNtearOffModel, XmTEAR_OFF_ENABLED);
    ac++;    
    menu = (XmRowColumnWidget) XmCreatePulldownMenu(menubar, "editmenu", arglist, ac);

    /*
    ** Create the edit menu entry and manage it.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.edit_menu_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.edit_label_mnem);
    ac++;
    if (help_widget->dxmhelp.edit_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.edit_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNsubMenuId, menu);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	ac++;
    }
    XtSetArg (arglist[ac], XmNcascadingCallback, pulling_callback);
    ac++;
    help_widget->dxmhelp.edit_menu =
	XmCreateCascadeButton(menubar, "editentry", arglist, ac);
    XtManageChild (help_widget->dxmhelp.edit_menu);

    /*
    ** Create the copy menu item.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.copy_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.copy_label_mnem);
    ac++;
    if (help_widget->dxmhelp.copy_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.copy_label_mnem_cs);
	ac++;
	}

    XtSetArg (arglist[ac], XmNactivateCallback, copy_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_copy_callback);
	ac++;
    }
    help_widget->dxmhelp.copy_button =
	XmCreatePushButtonGadget ((Widget) menu, "copybutton", arglist, ac);
	
    /*
    ** Create the select all menu item.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.selectall_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.selectall_label_mnem);
    ac++;
    if (help_widget->dxmhelp.selectall_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.selectall_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, selectall_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_selectall_callback);
	ac++;
    }
    help_widget->dxmhelp.selectall_button =
	XmCreatePushButtonGadget ((Widget) menu, "selectallbutton", arglist, ac);

    /*
    ** Manage all children.
    */
    XtManageChildren (XtChildren(menu), XtNumChildren(menu));

    return ((Widget)menu);
}

static Widget build_search_menu (menubar)
    Widget menubar;
/*
**++
**  Functional Description:
**	Build the search menu.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	menubar: widget id of the menubar.
**
**  Result:
**	Widget id of the search pulldown menu.
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget;
    XmRowColumnWidget menu;
    Arg arglist[20];
    int ac = 0;
	    
    XtCallbackRec	history_callback[2];
    XtCallbackRec	title_callback[2];
    XtCallbackRec	keyword_callback[2];
    XtCallbackRec help_callback[2];
    XtCallbackRec help_history_callback[2];
    XtCallbackRec help_title_callback[2];
    XtCallbackRec help_keyword_callback[2];
    XtCallbackRec pulling_callback[2];
    
    /*
    ** Initialization.
    */
    help_widget = (DXmHelpWidget) XtParent(menubar);

    history_callback[0].callback = (XtCallbackProc) build_search_history;
    history_callback[0].closure = (XtPointer) help_widget;
    history_callback[1].callback = NULL;
    title_callback[0].callback = (XtCallbackProc) build_search_title;
    title_callback[0].closure = (XtPointer) help_widget;
    title_callback[1].callback = NULL;
    keyword_callback[0].callback = (XtCallbackProc) build_search_keyword;
    keyword_callback[0].closure = (XtPointer) help_widget;
    keyword_callback[1].callback = NULL;
    help_callback[0].callback = (XtCallbackProc ) help_search;
    help_callback[0].closure  = (XtPointer) help_widget;
    help_callback[1].callback = NULL;
    help_history_callback[0].callback = (XtCallbackProc ) help_search_history;
    help_history_callback[0].closure  = (XtPointer) help_widget;
    help_history_callback[1].callback = NULL;
    help_title_callback[0].callback = (XtCallbackProc ) help_search_title;
    help_title_callback[0].closure  = (XtPointer) help_widget;
    help_title_callback[1].callback = NULL;
    help_keyword_callback[0].callback = (XtCallbackProc ) help_search_keyword;
    help_keyword_callback[0].closure  = (XtPointer) help_widget;
    help_keyword_callback[1].callback = NULL;
    pulling_callback[0].callback = (XtCallbackProc ) pulling_search;
    pulling_callback[0].closure  = (XtPointer) help_widget;
    pulling_callback[1].callback = NULL;

    /*
    ** Create the search pulldown menu.
    */
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
    XtSetArg (arglist[ac], XmNorientation, XmVERTICAL);
    ac++;
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;    
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;    
    XtSetArg (arglist[ac], XmNtearOffModel, XmTEAR_OFF_ENABLED);
    ac++;    
    menu = (XmRowColumnWidget) XmCreatePulldownMenu(menubar, "searchmenu", arglist, ac);

    /*
    ** Create the search menu entry and manage it.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.search_menu_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.search_label_mnem);
    ac++;
    if (help_widget->dxmhelp.search_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.search_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNsubMenuId, menu);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_callback);
	ac++;
    }
    XtSetArg (arglist[ac], XmNcascadingCallback, pulling_callback);
    ac++;
    help_widget->dxmhelp.search_menu = 
	XmCreateCascadeButton(menubar, "searchentry", arglist, ac);
    XtManageChild (help_widget->dxmhelp.search_menu);

    /*
    ** Create the history menu item.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.history_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.history_label_mnem);
    ac++;
    if (help_widget->dxmhelp.history_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.history_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, history_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_history_callback);
	ac++;
    }
    help_widget->dxmhelp.history_button =
	XmCreatePushButtonGadget ((Widget) menu, "historybutton", arglist, ac);
	
    /*
    ** Create the title menu entry.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.title_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.title_label_mnem);
    ac++;
    if (help_widget->dxmhelp.title_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.title_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, title_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_title_callback);
	ac++;
    }
    help_widget->dxmhelp.title_button =
	XmCreatePushButtonGadget ((Widget) menu, "titlebutton", arglist, ac);
	
    /*
    ** Create the keyword menu entry.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.keyword_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.keyword_label_mnem);
    ac++;
    if (help_widget->dxmhelp.keyword_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.keyword_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, keyword_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_keyword_callback);
	ac++;
    }
    help_widget->dxmhelp.keyword_button =
	XmCreatePushButtonGadget ((Widget) menu, "keywordbutton", arglist, ac);
    
    /*
    ** Manage all children.
    */
    XtManageChildren (XtChildren(menu), XtNumChildren(menu));   

    return ((Widget)menu);
}

static Widget build_help_menu (menubar)
    Widget menubar;
/*
**++
**  Functional Description:
**	Build the help menu.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	menubar: widget id of the menubar.
**
**  Result:
**	Widget id of the help menu entry.
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget;
    XmRowColumnWidget menu;
    Arg arglist[40];
    int ac = 0;
    XtCallbackRec help_callback[2];

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtCallbackRec glossary_callback[2];
    XtCallbackRec help_help_glossary_callback[2];
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtCallbackRec about_callback[2];
    XtCallbackRec help_help_about_callback[2];
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtCallbackRec help_help_callback[2];
    XtCallbackRec help_help_help_callback[2];

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtCallbackRec oncontext_callback[2];
    XtCallbackRec help_oncontext_callback[2];
#endif 	/* Remove everything except On Windows from Using Help Menu */
    
    /*
    ** Initialization.
    */
    help_widget = (DXmHelpWidget) XtParent(menubar);

    help_callback[0].callback = (XtCallbackProc) help;
    help_callback[0].closure = (XtPointer) help_widget;
    help_callback[1].callback = NULL;

#if 0	/* Remove everything except On Windows from Using Help Menu */
    glossary_callback[0].callback = (XtCallbackProc) help_glossary;
    glossary_callback[0].closure = (XtPointer) help_widget;
    glossary_callback[1].callback = NULL;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    about_callback[0].callback = (XtCallbackProc) help_about;
    about_callback[0].closure = (XtPointer) help_widget;
    about_callback[1].callback = NULL;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    help_help_callback[0].callback = (XtCallbackProc) help_help;
    help_help_callback[0].closure = (XtPointer) help_widget;
    help_help_callback[1].callback = NULL;

    help_help_help_callback[0].callback = (XtCallbackProc) help_help_help;
    help_help_help_callback[0].closure = (XtPointer) help_widget;
    help_help_help_callback[1].callback = NULL;

#if 0	/* Remove everything except On Windows from Using Help Menu */
    help_help_glossary_callback[0].callback = (XtCallbackProc) help_help_glossary;
    help_help_glossary_callback[0].closure = (XtPointer) help_widget;
    help_help_glossary_callback[1].callback = NULL;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    /* fix bug with context-sensitive help on 'About' frame */
    help_help_about_callback[0].callback = (XtCallbackProc) help_help_about;
    help_help_about_callback[0].closure = (XtPointer) help_widget;
    help_help_about_callback[1].callback = NULL;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    oncontext_callback[0].callback = (XtCallbackProc) on_context;
    oncontext_callback[0].closure = (XtPointer) help_widget;
    oncontext_callback[1].callback = NULL;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    help_oncontext_callback[0].callback = (XtCallbackProc) help_oncontext;
    help_oncontext_callback[0].closure = (XtPointer) help_widget;
    help_oncontext_callback[1].callback = NULL;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    /*
    ** Create the help pulldown menu.
    */
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
    XtSetArg (arglist[ac], XmNorientation, XmVERTICAL);
    ac++;
    XtSetArg (arglist[ac], XtNx, 0);
    ac++;    
    XtSetArg (arglist[ac], XtNy, 0);
    ac++;    
    XtSetArg (arglist[ac], XmNtearOffModel, XmTEAR_OFF_ENABLED);
    ac++;    
    menu = (XmRowColumnWidget) XmCreatePulldownMenu(menubar, "helpmenu", arglist, ac);

    /*
    ** Create the help [Using Help] menu entry and
    ** manage it.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.help_menu_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.help_label_mnem);
    ac++;
    if (help_widget->dxmhelp.help_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.help_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNsubMenuId, menu);
    ac++;
    XtSetArg(arglist[ac], XmNhelpCallback, help_help_callback);
    ac++;
    help_widget->dxmhelp.help_menu =
	XmCreateCascadeButton(menubar, "helpentry", arglist, ac);
    XtManageChild (help_widget->dxmhelp.help_menu);


#if 0	/* Remove everything except On Windows from Using Help Menu */
    /*
    ** Create the help[On Context] menu item.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.oncontext_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.oncontext_label_mnem);
    ac++;
    if (help_widget->dxmhelp.oncontext_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.oncontext_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, oncontext_callback);
    ac++;
    XtSetArg(arglist[ac], XmNhelpCallback, help_oncontext_callback);
    ac++;
    help_widget->dxmhelp.oncontext_button =
	XmCreatePushButtonGadget (menu, "oncontextbutton", arglist, ac);

#endif 	/* Remove everything except On Windows from Using Help Menu */


    /*
    ** Create the help [used to be Overview, now On Window] menu item.
    */
#ifdef RTOL
    ac = 3;
#else
    ac = 2;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.helphelp_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.helphelp_label_mnem);
    ac++;
    if (help_widget->dxmhelp.helphelp_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.helphelp_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, help_callback);
    ac++;
    XtSetArg(arglist[ac], XmNhelpCallback, help_help_help_callback);
    ac++;
    help_widget->dxmhelp.help_button =
	XmCreatePushButtonGadget ((Widget) menu, "helpbutton", arglist, ac);

#if 0	/* Remove everything except On Windows from Using Help Menu */
    /*
    ** Create the [used to be About] On Version menu item.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.about_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.about_label_mnem);
    ac++;
    if (help_widget->dxmhelp.about_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.about_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, about_callback);
    ac++;
    XtSetArg(arglist[ac], XmNhelpCallback, help_help_about_callback);
    ac++;
    help_widget->dxmhelp.about_button =
	XmCreatePushButtonGadget (menu, "aboutbutton", arglist, ac);
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    /*
    ** Create the glossary menu item.
    */
#ifdef RTOL
    ac = 4;
#else
    ac = 3;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.glossary_label);
    ac++;
    XtSetArg (arglist[ac], XmNmnemonic, help_widget->dxmhelp.glossary_label_mnem);
    ac++;
    if (help_widget->dxmhelp.glossary_label_mnem_cs)
        {
	XtSetArg (arglist[ac], XmNmnemonicCharSet, 
		  help_widget->dxmhelp.glossary_label_mnem_cs);
	ac++;
	}
    XtSetArg (arglist[ac], XmNactivateCallback, glossary_callback);
    ac++;
    XtSetArg(arglist[ac], XmNhelpCallback, help_help_glossary_callback);
    ac++;
    help_widget->dxmhelp.helpglossary_button =
	XmCreatePushButtonGadget (menu, "glossarybutton", arglist, ac);
#endif 	/* Remove everything except On Windows from Using Help Menu */


    /*
    ** Manage all children.
    */
    XtManageChildren (XtChildren(menu), XtNumChildren(menu));   

    return ((Widget)help_widget->dxmhelp.help_menu);
}

static void build_menubar (parent)
    Widget parent;
/*
**++
**  Functional Description:
**	Build the menubar.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	parent: widget id of the parent widget.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) parent;
    Arg arglist[10];
    int ac = 0;
    Widget help_pulldown;

    XtCallbackRec menubar_csh_cb[2];        
    XtCallbackRec help_callback[2];

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtCallbackRec glossary_callback[2];
    XtCallbackRec about_callback[2];
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtCallbackRec help_help_callback[2];
    
    /*
    ** Initialization.
    */
    menubar_csh_cb[0].callback = (XtCallbackProc) csh_menubar;
    menubar_csh_cb[0].closure = (XtPointer) help_widget;
    menubar_csh_cb[1].callback = NULL;
    help_callback[0].callback = (XtCallbackProc) help;
    help_callback[0].closure = (XtPointer) help_widget;
    help_callback[1].callback = NULL;

#if 0	/* Remove everything except On Windows from Using Help Menu */
    glossary_callback[0].callback = (XtCallbackProc) help_glossary;
    glossary_callback[0].closure = (XtPointer) help_widget;
    glossary_callback[1].callback = NULL;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    about_callback[0].callback = (XtCallbackProc) help_about;
    about_callback[0].closure = (XtPointer) help_widget;
    about_callback[1].callback = NULL;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    help_help_callback[0].callback = (XtCallbackProc) help_help;
    help_help_callback[0].closure = (XtPointer) help_widget;
    help_help_callback[1].callback = NULL;

    
    /*
    ** Create the menubar and manage it.
    */
    XtSetArg (arglist[ac], XmNorientation, XmHORIZONTAL);
    ac++;
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
    XtSetArg (arglist[ac], XmNhelpCallback, menubar_csh_cb);
    ac++;
    help_widget->dxmhelp.menubar =
	XmCreateMenuBar ((Widget) help_widget, "helpmenubar", arglist, ac);
    
    XtManageChild (help_widget->dxmhelp.menubar);

    /*
    ** Build the menus.
    */
    build_file_menu (help_widget->dxmhelp.menubar);
    build_edit_menu (help_widget->dxmhelp.menubar);
    build_view_menu (help_widget->dxmhelp.menubar);						    
    build_search_menu (help_widget->dxmhelp.menubar);

    /*
    ** Check if the library is different from the help on help library and 
    ** build the help menu.
    */
    if (check_library(help_widget->dxmhelp.library))
    {
	help_pulldown = build_help_menu (help_widget->dxmhelp.menubar);
	XtManageChild (help_pulldown);
	XtSetArg (arglist[0], XmNmenuHelpWidget, help_pulldown);
	XtSetValues (help_widget->dxmhelp.menubar, arglist, 1);
    }
}

static void select_topic (listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Single select callback routine for the additional topics listbox: get
**	the selected topic.
**
**  Keywords:
**	Calbback routine.
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;  

    if (list->item_position != 0)
    {
	/*
	** Get the topic associated with the selected title.
	*/
        help_widget->dxmhelp.selected_topic =
	    XmStringCopy((XmString) help_widget->dxmhelp.topic_array[list->item_position - 1]);
	/*
	** Get the selected title.
	*/
        help_widget->dxmhelp.selected_title = XmStringCopy(list->item);

	/*
	** Set the goto and visit buttons to sensitive.
	*/
        XtSetSensitive (help_widget->dxmhelp.goto_button, TRUE);
        XtSetSensitive (help_widget->dxmhelp.visit_button, TRUE);
	
    }
}

static void goto_select_topic (listbox_item, w, list)
    Widget listbox_item;
    Widget w;
    XmListCallbackStruct *list;
/*
**++
**  Functional Description:
**	Select and Confirm callback routine for the additional topics listbox:
**	get the selected topic and go to it.
**
**  Keywords:
**	Callback routine.
**
**  Arguments:
**	listbox_item: widget id of the widget doing the callback.
**	w: pointer to the help widget's record
**	list: pointer to the listbox callback structure
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    if (list->item_position != 0)
    {
	/*
	** Get the topic associated with the selected title.
	*/
        help_widget->dxmhelp.selected_topic =
	    XmStringCopy((XmString) help_widget->dxmhelp.topic_array[list->item_position - 1]);
	/*
	** Get the selected title.
	*/
        help_widget->dxmhelp.selected_title = XmStringCopy(list->item);
		    
	/*
	** Go to the selected topic.
	*/
        go_to (NULL, help_widget);
    }
}


static void build_work_area (w)
    Widget w;
/*
**++
**  Functional Description:
**	Build the work area of the help widget.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	w: pointer to the help widget's record
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    Arg arglist[20];
    Arg listal[20];
    Arg textal[20];
    int ac = 0;
    int button_width = 0;
                
    XtCallbackRec select_callback[2];
    XtCallbackRec select_go_callback[2];
    XtCallbackRec goback_callback[2];
    XtCallbackRec quit_callback[2];
    XtCallbackRec help_frame_callback[2];
    XtCallbackRec help_listbox_callback[2];
    XtCallbackRec help_goback_callback[2];
    XtCallbackRec help_exit_callback[2];

    /*
    ** Initialization.
    */
    select_callback[0].callback = (XtCallbackProc) select_topic;
    select_callback[0].closure = (XtPointer) help_widget;
    select_callback[1].callback = NULL;
    select_go_callback[0].callback = (XtCallbackProc) goto_select_topic;
    select_go_callback[0].closure = (XtPointer) help_widget;
    select_go_callback[1].callback = NULL;
    goback_callback[0].callback = (XtCallbackProc) goback;
    goback_callback[0].closure = (XtPointer) help_widget;
    goback_callback[1].callback = NULL;
    quit_callback[0].callback = (XtCallbackProc) quit;
    quit_callback[0].closure = (XtPointer) help_widget;
    quit_callback[1].callback = NULL;
    help_frame_callback[0].callback = (XtCallbackProc) help_frame;
    help_frame_callback[0].closure = (XtPointer) help_widget;
    help_frame_callback[1].callback = NULL;
    help_listbox_callback[0].callback = (XtCallbackProc) help_listbox;
    help_listbox_callback[0].closure = (XtPointer) help_widget;
    help_listbox_callback[1].callback = NULL;
    help_goback_callback[0].callback = (XtCallbackProc) help_goback;
    help_goback_callback[0].closure = (XtPointer) help_widget;
    help_goback_callback[1].callback = NULL;
    help_exit_callback[0].callback = (XtCallbackProc) help_exit;
    help_exit_callback[0].closure = (XtPointer) help_widget;
    help_exit_callback[1].callback = NULL;

    /*
    ** Create the help text widget.
    */
    XtSetArg (textal[ac], XmNresizeWidth, FALSE);
    ac++;
    XtSetArg (textal[ac], XmNresizeHeight, FALSE);
    ac++;
    XtSetArg (textal[ac], XmNwordWrap, TRUE);
    ac++;
    XtSetArg (textal[ac], XmNeditable, FALSE);
    ac++;
    XtSetArg (textal[ac], XmNborderWidth, 1);
    ac++;
    XtSetArg (textal[ac], XmNvalue, help_widget->dxmhelp.text);
    ac++;
    XtSetArg (textal[ac], XmNcolumns, (help_widget->dxmhelp.colons + 4));
    ac++;
    XtSetArg (textal[ac], XmNrows, help_widget->dxmhelp.rows + 1);
    ac++;
    XtSetArg (textal[ac], XmNscrollVertical, TRUE);
    ac++;
#ifdef RTOL
    if ( LayoutIsRtoL (help_widget) )
    {
        XtSetArg (textal[ac], XmNscrollLeftSide, TRUE);
        ac++;
    }
#endif /* RTOL */
    XtSetArg (textal[ac], XmNfontList, help_widget->dxmhelp.text_font_list);
    ac++;
    XtSetArg (textal[ac], XmNcursorPositionVisible, FALSE);
    ac++;
    XtSetArg (textal[ac], XmNautoShowCursorPosition, FALSE);
    ac++;
    XtSetArg (textal[ac], XmNtopPosition, 0);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(textal[ac], XmNhelpCallback, help_frame_callback);
	ac++;
    }
    XtSetArg (textal[ac], XmNmarginWidth, MARGIN_WIDTH);
    ac++;
    XtSetArg (textal[ac], XmNmarginHeight, MARGIN_HEIGHT);
    ac++;
    XtSetArg (textal[ac], XmNeditMode, XmMULTI_LINE_EDIT);
    ac++;
        
    help_widget->dxmhelp.help_text =
#ifdef CSTEXT
	(Widget) DXmCreateScrolledCSText(help_widget, "helptext", textal, ac);
#else
	XmCreateScrolledText(help_widget, "helptext", textal, ac);
#endif

    /*
    ** Create the additional topics label.
    */
    ac = 0;
    XtSetArg (arglist[ac], XmNborderWidth, 0);
    ac++;
    XtSetArg (arglist[ac], XmNalignment, XmALIGNMENT_BEGINNING);
    ac++;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.addtopic_label);
    ac++;
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.label_font_list);
    ac++;
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_listbox_callback);
	ac++;
    }
    help_widget->dxmhelp.add_topic =
	XmCreateLabel ((Widget) help_widget, "helplabel", arglist, ac);

    /*
    ** Create the additional topics listbox.
    */
    ac = 0;
    XtSetArg (listal[ac], XmNbrowseSelectionCallback, select_callback);
    ac++;
    XtSetArg (listal[ac], XmNdefaultActionCallback, select_go_callback);
    ac++;
    XtSetArg (listal[ac], XmNlistSizePolicy, XmCONSTANT);
    ac++;
    XtSetArg (listal[ac], XmNscrollBarDisplayPolicy, XmSTATIC);
    ac++;
/*  ??  These next three should probably be set in a resolution independent unit */
    XtSetArg (listal[ac], XmNmarginHeight, 6);
    ac++;
    XtSetArg (listal[ac], XmNmarginWidth, 6);
    ac++;
    XtSetArg (listal[ac], XmNlistSpacing, 6);
    ac++;
    XtSetArg (listal[ac], XmNvisibleItemCount, help_widget->dxmhelp.visible_count);
    ac++;
    XtSetArg (listal[ac], XmNitems, help_widget->dxmhelp.title_array);
    ac++;
    XtSetArg (listal[ac], XmNitemCount, help_widget->dxmhelp.topic_count);
    ac++;
    XtSetArg (listal[ac], XmNfontList, help_widget->dxmhelp.text_font_list);
    ac++;
#ifdef RTOL
    XtSetArg (listal[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(listal[ac], XmNhelpCallback, help_listbox_callback);
	ac++;
    }
    
    help_widget->dxmhelp.help_topic =
	XmCreateScrolledList ((Widget) help_widget, "helplistbox", listal, ac);

    /*
    ** Create the goback button.
    */
    ac = 0;
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.goback_label);
    ac++;
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
    ac++;
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNactivateCallback, goback_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_goback_callback);
	ac++;
    }
    XtSetArg (arglist[ac], XtNsensitive, FALSE);
    ac++;
    XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
    ac++;
    help_widget->dxmhelp.goback =
	XmCreatePushButton ((Widget) help_widget, "helpbutton", arglist, ac);
    
    /*
    ** Create the quit button.
    */
    ac = 0;    
    XtSetArg (arglist[ac], XmNlabelString, help_widget->dxmhelp.quit_label);
    ac++;
    XtSetArg (arglist[ac], XmNfontList, help_widget->dxmhelp.button_font_list);
    ac++;
    XtSetArg (arglist[ac], XmNstringDirection, help_widget->dxmhelp.string_direction);
    ac++;
#ifdef RTOL
    XtSetArg (arglist[ac], DXmNlayoutDirection, LayoutDirection(help_widget));
    ac++;
#endif /* RTOL */
    XtSetArg (arglist[ac], XmNactivateCallback, quit_callback);
    ac++;
    if (check_library(help_widget->dxmhelp.library))
    {
	XtSetArg(arglist[ac], XmNhelpCallback, help_exit_callback);
	ac++;
    }
    XtSetArg (arglist[ac], XmNrecomputeSize, TRUE);
    ac++;
    help_widget->dxmhelp.quit =
	XmCreatePushButton ((Widget) help_widget, "helpbutton", arglist, ac);

    /*
    ** Get the frame associated with the topic on top of the trail list and set
    ** the text widget and the listbox.
    */
    get_help (help_widget);

    /*
    ** get_help only manages the additional topics list when there are additional topics.  However,
    ** in order for Layout to set the size of the list correctly we need to manage the list.  This
    ** fixes a bug that occurred when there were no additonal topics the first time the help widget
    ** was called.  Subsequent calls to the help widget with additonal topics had the list sized
    ** incorrectly.
    */
    XtManageChild (help_widget->dxmhelp.help_topic);

    /*
    ** Manage the text widget and the buttons.
    */
    XtManageChild (help_widget->dxmhelp.help_text);
    XtManageChild (XtParent(help_widget->dxmhelp.help_text));
    XtManageChild (help_widget->dxmhelp.goback);
    XtManageChild (help_widget->dxmhelp.quit);

    /*
    ** Compute the buttons' width and set it.
    */
    button_width = MAX(button_width, XtWidth(help_widget->dxmhelp.goback));
    button_width = MAX(button_width, XtWidth(help_widget->dxmhelp.quit));
    ac = 0;
    XtSetArg (arglist[ac], XmNrecomputeSize, FALSE);
    ac++;
    XtSetArg (arglist[ac], XtNwidth, button_width);
    ac++;
    XtSetValues (help_widget->dxmhelp.goback, arglist, ac);
    XtSetValues (help_widget->dxmhelp.quit, arglist, ac);
}

static void build_avoid_widgets (w)
    Widget w;
/*
**++
**  Functional Description:
**	Build the array of widgets to avoid on the screen.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	w: pointer to the help widget's structure.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    int i = 0;
    int j = 0;
    Widget parent = NULL;
    Widget widget_array[50];

    /*
    ** Get the help widget's parent.
    */
    parent = XtParent (help_widget->core.parent);

    /*
    ** Loop until the parent isn't of class help widget [or we run out of
    ** parents] and store the widgets id in the widget_array.
    */
    while ( parent && (XtIsSubclass (parent, dxmHelpWidgetClass)) )
    {
	widget_array[i] = parent;
	i++;
	if (parent->core.parent)
	    {
	    /* If there is a parent [assumed to be some shell], get its parent*/
	    parent = XtParent (parent->core.parent);
	    }
	else
	    {
	    /* If there is no parent, then our shell is probably toplevel and
             * there is no parent -- force variable 'parent' to 0
             */
	    parent = 0;
	    }
    }
    /*
    ** Add the parent to the widget_array, if it exists
    */
    if (parent)
	{
        widget_array[i] = parent;
        i++;
	}
    
    /*
    ** Fill in the array of widgets to avoid and the count.
    */
    while (i > 0 && j < 8)
    {
	i--;
	help_widget->dxmhelp.avoid_widget[j] = widget_array[i];
	j++;
    }
    help_widget->dxmhelp.avoid_count = j;
}

static void open_library (help_widget)
    DXmHelpWidget	    help_widget;
/*
**++
**  Functional Description:
**	Get the help library specification and open the library.
**
**  Keywords:
**	Help creation, Help update.
**
**  Arguments:
**	help_widget: widget id of the help_widget.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    int vms_status = 0;
    char *library;
    char *library_context = NULL;
    
    if (help_widget->dxmhelp.library != NULL)
    {
	/*
	** Extract the library specification from the compound string.
	*/
	if (get_string_from_cs (help_widget->dxmhelp.library,&library) == TRUE)
	{
	    /*
	    ** Open the library.
	    */
#ifdef HANDLE_THIS_LATER
	    if (char_set == CDA$K_ISO_LATIN1)
#endif
	    {
		struct dsc$descriptor_s library_desc;
		
		library_desc.dsc$w_length = strlen(library);
		library_desc.dsc$b_dtype  = DSC$K_DTYPE_T;
		library_desc.dsc$b_class = DSC$K_CLASS_S;
		library_desc.dsc$a_pointer = library;
		vms_status = DXmHelp__init_help (&library_context,
		    &library_desc, help_widget->dxmhelp.cache_library,
		    help_widget);
		help_widget->dxmhelp.library_context = library_context;
	    }
	    XtFree(library);
	    /*
	    ** If the opening of the library was unsuccessfull, set the library
	    ** context to null.
	    */
	    if (VMS_STATUS_SUCCESS(vms_status)!= STS$K_SUCCESS)
		help_widget->dxmhelp.library_context = NULL;
	}
    }
    else
    /*
    **  No library specified, set library context to NULL
    */
	help_widget->dxmhelp.library_context = NULL;

}

#ifdef GENERAL_WM_TITLE 
static void
UpdateWMShellTitle( Wid)
DXmHelpWidget Wid ;
/****************
 * If the parent is a WMShell, this routine sets the title and title 
 * encoding resources of the WMShell according to the title
 * resource of the help widget.
 * CT encoding will be requested only if our title char_set is != Latin_1
 ****************/
{   
            Widget          parent ;
            char *          text ;
            XmStringCharSet charset ;
            Arg             al[10] ;
            Cardinal        ac ;
	    long	    status,count;
	    Boolean	    do_ct = False;
/****************/

    /* Set WMShell title (if present).
    */
    parent = XtParent( Wid) ;

    if(    XtIsWMShell( parent)    )
    {   
        /* Parent is a Window Manager Shell, so set WMShell title
        *   from the Help Title.
        */
        text = NULL ;
        ac = 0 ;
        if(    DXmCSContainsStringCharSet( Wid->dxmhelp.title))
            {   
                /*  If title consists of STRING_CHARSET charset, use atom 
                *   of "STRING".  Otherwise, convert to compound text and
                *   use atom of "COMPOUND_TEXT".
                */
                charset = "STRING" ;
            } 
        else
            {
                charset = "COMPOUND_TEXT" ;
                do_ct = True;
            } 

        if(    do_ct    )
            {   
		text = (char *) XmCvtXmStringToCT(Wid->dxmhelp.title);
            }
        else     
	    {
	    	text = (char *) DXmCvtCStoFC(Wid->dxmhelp.title,
							  &count,&status);
	    	if (status == DXmCvtStatusFail || count == 0) text = NULL;
	    }

	if(    text    )
	    {   
            	XtSetArg( al[ac], XtNtitle, text) ;  ++ac ;
            	XtSetArg( al[ac], XtNtitleEncoding, XmInternAtom( 
                                   XtDisplay( Wid), charset, FALSE)) ; ++ac ;
            	XtSetValues( parent, al, ac) ;
            	XtFree( (char *) text) ;
	    } 

    }
    return;
}
#endif /*GENERAL_WM_TITLE*/

static void set_helpshell_title (request_widget, new_widget)
    DXmHelpWidget	    request_widget;
    DXmHelpWidget	    new_widget;
/*
**++
**  Functional Description:
**	Compose and set the helpshell title.
**
**  Keywords:
**	Help creation, Help update.
**
**  Arguments:
**	request_widget: widget id of the requested widget.
**	new_widget: widget id of the new widget.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    char *title;
    Arg arglist[10];
#ifdef GENERAL_TEXT
    long count,stat;
#endif /* GENERAL_TEXT */
    /*
    ** Compose the help widget's title and the application name and store them.
    */
    if (request_widget->dxmhelp.application != NULL)
    {
        new_widget->dxmhelp.application =
	    XmStringCopy (request_widget->dxmhelp.application);

        if (check_library(request_widget->dxmhelp.library))

#ifdef RTOL
	    if (!LayoutIsRtoL(request_widget))
#else
	    if (!request_widget->dxmhelp.string_direction)
#endif /* RTOL */
	    {
		new_widget->dxmhelp.title =
		    XmStringConcat (new_widget->dxmhelp.helpontitle_label,
				request_widget->dxmhelp.application);
	    }
	    else
	    {
		new_widget->dxmhelp.title =
		    XmStringConcat (new_widget->dxmhelp.application,
				request_widget->dxmhelp.helpontitle_label);
	    }
	else
	{
	    new_widget->dxmhelp.title =
		XmStringCopy(new_widget->dxmhelp.helponhelp_title);
	}
    }
    else
        new_widget->dxmhelp.title =
	    XmStringCopy (new_widget->dxmhelp.helptitle_label);

#ifdef GENERAL_WM_TITLE 
    /*
    ** Update the title using Compound Text if applicable
    */
    UpdateWMShellTitle(new_widget);
#else
    /*
    ** Extract the help title from the compound string.
    */ 
#ifdef GENERAL_TEXT
    if ((title =(XmString)DXmCvtCStoFC(new_widget->dxmhelp.title,&count,&stat)) != NULL)
#else
    if (get_string_from_cs (new_widget->dxmhelp.title,&title) == TRUE)
#endif /* GENERAL_TEXT */
    {
	XtSetArg (arglist[0], XtNtitle, title);
	XtSetValues (new_widget->dxmhelp.shell, arglist, 1);
	XtFree (title);
    }
#endif /*GENERAL_WM_TITLE*/
}

static void init_help (help_widget, full)
    DXmHelpWidget help_widget;
    Boolean full;
/*
**++
**  Functional Description:
**	Initialize all or part of the help widget's private record.
**
**  Keywords:
**	Help creation, Help update
**
**  Arguments:
**	help_widget: widget id of the help widget.
**	full: full initialization or not.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
#ifdef CSTEXT
    long count,stat;

    /* Which one to use ? */
    /* help_widget->dxmhelp.text = DXmCvtFCtoCS("\n",&count,&stat); */
    help_widget->dxmhelp.text = XmStringSeparatorCreate(); 
#else
    help_widget->dxmhelp.text = (char *) strcpy(XtMalloc (strlen("\n")+1), "\n");
#endif /* CSTEXT */
    help_widget->dxmhelp.topic_count = 0;
    help_widget->dxmhelp.visible_count = VISIBLE_TOPIC;
    help_widget->dxmhelp.topic_array = NULL;
    help_widget->dxmhelp.title_array = NULL;
    help_widget->dxmhelp.selected_topic = NULL;
    help_widget->dxmhelp.selected_title = NULL;
    help_widget->dxmhelp.trail = NULL;    
    if (full)
    {
        help_widget->dxmhelp.history = NULL;
	help_widget->dxmhelp.history_count = 0;
	help_widget->dxmhelp.selected_history_topic = NULL;
	help_widget->dxmhelp.searchtitle_topic_array = NULL;
	help_widget->dxmhelp.selected_searchtitle_topic = NULL;
	help_widget->dxmhelp.searchkeyword_topic_array = NULL;
	help_widget->dxmhelp.selected_searchkeyword_topic = NULL;
	help_widget->dxmhelp.keyword_array = NULL;
	help_widget->dxmhelp.selected_keyword = NULL;
	help_widget->dxmhelp.message_text = NULL;
	help_widget->dxmhelp.message_parameter = NULL;
	help_widget->dxmhelp.message = FALSE;
	help_widget->dxmhelp.visit_context = NULL;
    }
}

static void Initialize (request, new)
    Widget	    request;
    Widget	    new;
/*
**++
**  Functional Description:
**	Initialize an instance of the help widget.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	request: widget with resource values as requested by the argument list,
**		the resource database and the widget defaults.
**	new: widget with the new values, both resource and non_resource that are
**		actually allowed.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    Boolean direction;
    char *textstr;
    DXmHelpWidget request_widget = (DXmHelpWidget) request;
    DXmHelpWidget new_widget = (DXmHelpWidget) new;
    char *s;    /* Temp string pointer for mnemonic char set copying */
    int             mwmStyle ;
    Arg             al[5] ;    

#ifdef HELP_MNEM_CHARSET
    XmStringCharSet  CurrentCharset = DXmGetLocaleCharset( );
    int	cset_len = strlen(CurrentCharset) + 1;
#define ALLOC_CHARSET  \
	(strcpy(XtMalloc(cset_len),CurrentCharset))
#endif /* HELP_MNEM_CHARSET */    

#ifdef RTOL
    if (LayoutIsRtoL(request_widget))
	new_widget->dxmhelp.string_direction = XmSTRING_DIRECTION_R_TO_L;
#endif /* RTOL */

    /*
    ** Store the library if specified.
    */
    if (request_widget->dxmhelp.library != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.library))
	    new_widget->dxmhelp.library =
		XmStringCopy (request_widget->dxmhelp.library);
	else
	    new_widget->dxmhelp.library = NULL;
	}
    /*
    ** Store the first_topic if specified.
    */
    if (request_widget->dxmhelp.first_topic != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.first_topic))
	    new_widget->dxmhelp.first_topic =
		XmStringCopy (request_widget->dxmhelp.first_topic);
	else
	    new_widget->dxmhelp.first_topic = NULL;
	}
    /*
    ** Store the overview topic if specified.
    */
    if (request_widget->dxmhelp.overview != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.overview))
		new_widget->dxmhelp.overview =
		    XmStringCopy (request_widget->dxmhelp.overview);
	else
	    new_widget->dxmhelp.overview = NULL;
	}
    /*
    ** Store the glossary topic if specified.
    */
    if (request_widget->dxmhelp.glossary != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.glossary))
		new_widget->dxmhelp.glossary =
		    XmStringCopy (request_widget->dxmhelp.glossary);
	else
	    new_widget->dxmhelp.glossary = NULL;
	}
    /*
    ** Store all widget labels if specified.
    */
    /*
    ** View menu labels.
    */
    if (request_widget->dxmhelp.view_menu_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.view_menu_label))
	    new_widget->dxmhelp.view_menu_label =
		XmStringCopy (request_widget->dxmhelp.view_menu_label);
	else
	    new_widget->dxmhelp.view_menu_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.view_menu_label =
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSView,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.goto_topic_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.goto_topic_label))
	    new_widget->dxmhelp.goto_topic_label =
		XmStringCopy (request_widget->dxmhelp.goto_topic_label);
	else
	    new_widget->dxmhelp.goto_topic_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.goto_topic_label = 
		(XmString)DXmGetLocaleString((I18nContext)NULL,DXmSGotoTopic,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.goback_topic_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.goback_topic_label))
	    new_widget->dxmhelp.goback_topic_label =
		XmStringCopy (request_widget->dxmhelp.goback_topic_label);
	else
	    new_widget->dxmhelp.goback_topic_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.goback_topic_label = 
	       (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSGobackTopic,I18NVERB | I18NMENU );
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.goover_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.goover_label))
	    new_widget->dxmhelp.goover_label =
		XmStringCopy (request_widget->dxmhelp.goover_label);
	else
	    new_widget->dxmhelp.goover_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.goover_label = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSGooverview,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.visit_topic_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.visit_topic_label))
	    new_widget->dxmhelp.visit_topic_label =
		XmStringCopy (request_widget->dxmhelp.visit_topic_label);
	else
	    new_widget->dxmhelp.visit_topic_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.visit_topic_label = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSVisitTopic,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.visitglos_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.visitglos_label))
	    new_widget->dxmhelp.visitglos_label =
		XmStringCopy (request_widget->dxmhelp.visitglos_label);
	else
	    new_widget->dxmhelp.visitglos_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.visitglos_label = 
	     (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSVisitglossary,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** File menu labels.
    */
    if (request_widget->dxmhelp.file_menu_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.file_menu_label))
	    new_widget->dxmhelp.file_menu_label =
		XmStringCopy (request_widget->dxmhelp.file_menu_label);
	else
	    new_widget->dxmhelp.file_menu_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.file_menu_label = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSFile,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.saveas_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.saveas_label))
	    new_widget->dxmhelp.saveas_label =
		XmStringCopy (request_widget->dxmhelp.saveas_label);
	else
	    new_widget->dxmhelp.saveas_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
        new_widget->dxmhelp.saveas_label = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSSaveas,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.exit_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.exit_label))
	    new_widget->dxmhelp.exit_label =
		XmStringCopy (request_widget->dxmhelp.exit_label);
	else
	    new_widget->dxmhelp.exit_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.exit_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSExit,I18NVERB | I18NMENU );
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Edit menu labels.
    */
    if (request_widget->dxmhelp.edit_menu_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.edit_menu_label))
	    new_widget->dxmhelp.edit_menu_label =
		XmStringCopy (request_widget->dxmhelp.edit_menu_label);
	else
	    new_widget->dxmhelp.edit_menu_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.edit_menu_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSEdit,I18NVERB);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.copy_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.copy_label))
	    new_widget->dxmhelp.copy_label =
		XmStringCopy (request_widget->dxmhelp.copy_label);
	else
	    new_widget->dxmhelp.copy_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.copy_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSCopy,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.selectall_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.selectall_label))
	    new_widget->dxmhelp.selectall_label =
		XmStringCopy (request_widget->dxmhelp.selectall_label);
	else
	    new_widget->dxmhelp.selectall_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.selectall_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSSelectall,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Search menu labels.
    */
    if (request_widget->dxmhelp.search_menu_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.search_menu_label))
	    new_widget->dxmhelp.search_menu_label =
		XmStringCopy (request_widget->dxmhelp.search_menu_label);
	else
	    new_widget->dxmhelp.search_menu_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.search_menu_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSSearch,I18NVERB | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.history_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.history_label))
	    new_widget->dxmhelp.history_label =
		XmStringCopy (request_widget->dxmhelp.history_label);
	else
	    new_widget->dxmhelp.history_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.history_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSHistory,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.title_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.title_label))
	    new_widget->dxmhelp.title_label =
		XmStringCopy (request_widget->dxmhelp.title_label);
	else
	    new_widget->dxmhelp.title_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.title_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSTitle,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.keyword_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.keyword_label))
	    new_widget->dxmhelp.keyword_label =
		XmStringCopy (request_widget->dxmhelp.keyword_label);
	else
	    new_widget->dxmhelp.keyword_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.keyword_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSKeyword,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Help on help menu labels.
    */
    if (request_widget->dxmhelp.help_menu_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.help_menu_label))
	    new_widget->dxmhelp.help_menu_label =
		XmStringCopy (request_widget->dxmhelp.help_menu_label);
	else
	    new_widget->dxmhelp.help_menu_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.help_menu_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSHelpOnHelp,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.helphelp_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.helphelp_label))
	    new_widget->dxmhelp.helphelp_label =
		XmStringCopy (request_widget->dxmhelp.helphelp_label);
	else                           
	    new_widget->dxmhelp.helphelp_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.helphelp_label   = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSHelp,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (request_widget->dxmhelp.glossary_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.glossary_label))
	    new_widget->dxmhelp.glossary_label =
		XmStringCopy (request_widget->dxmhelp.glossary_label);
	else
	    new_widget->dxmhelp.glossary_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.glossary_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSGlossary,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (request_widget->dxmhelp.about_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.about_label))
	    new_widget->dxmhelp.about_label =
		XmStringCopy (request_widget->dxmhelp.about_label);
	else                           
	    new_widget->dxmhelp.about_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.about_label  =     
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSAbout,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
#endif 	/* Remove everything except On Windows from Using Help Menu */


#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (request_widget->dxmhelp.oncontext_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.oncontext_label))
	    new_widget->dxmhelp.oncontext_label =
		XmStringCopy (request_widget->dxmhelp.oncontext_label);
	else
	    new_widget->dxmhelp.oncontext_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.oncontext_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSOncontext,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */
#endif 	/* Remove everything except On Windows from Using Help Menu */


    /*
    ** Additional topics label.
    */
    if (request_widget->dxmhelp.addtopic_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.addtopic_label))
	    new_widget->dxmhelp.addtopic_label =
		XmStringCopy (request_widget->dxmhelp.addtopic_label);
	else
	    new_widget->dxmhelp.addtopic_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.addtopic_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSAddtopic,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Goback and exit buttons labels inside the help widget.
    */
    if (request_widget->dxmhelp.goback_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.goback_label))
	    new_widget->dxmhelp.goback_label =
		XmStringCopy (request_widget->dxmhelp.goback_label);
	else
	    new_widget->dxmhelp.goback_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.goback_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSGoback,I18NVERB | I18NBUTTON );
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.quit_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.quit_label))
	    new_widget->dxmhelp.quit_label =
		XmStringCopy (request_widget->dxmhelp.quit_label);
	else
	    new_widget->dxmhelp.quit_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.quit_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSClose,I18NVERB | I18NBUTTON );
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Goto, visit, apply and dismiss buttons labels for the search boxes.
    */
    if (request_widget->dxmhelp.goto_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.goto_label))
	    new_widget->dxmhelp.goto_label =
		XmStringCopy (request_widget->dxmhelp.goto_label);
	else
	    new_widget->dxmhelp.goto_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.goto_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSGoto,I18NVERB | I18NBUTTON);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.visit_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.visit_label))
	    new_widget->dxmhelp.visit_label =
		XmStringCopy (request_widget->dxmhelp.visit_label);
	else
	    new_widget->dxmhelp.visit_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.visit_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSVisit,I18NVERB | I18NBUTTON);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.apply_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.apply_label))
	    new_widget->dxmhelp.apply_label =
		XmStringCopy (request_widget->dxmhelp.apply_label);
	else
	    new_widget->dxmhelp.apply_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.apply_label  = 
	       (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSSearchapply,I18NVERB | I18NBUTTON);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.dismiss_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.dismiss_label))
	    new_widget->dxmhelp.dismiss_label =
		XmStringCopy (request_widget->dxmhelp.dismiss_label);
	else
	    new_widget->dxmhelp.dismiss_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.dismiss_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSDismiss,I18NVERB | I18NBUTTON);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Search boxes labels.
    */
    if (request_widget->dxmhelp.topic_titles_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.topic_titles_label))
	    new_widget->dxmhelp.topic_titles_label =
		XmStringCopy (request_widget->dxmhelp.topic_titles_label);
	else
	    new_widget->dxmhelp.topic_titles_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.topic_titles_label  = 
	       (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSTopictitles,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** History box labels.
    */
    if (request_widget->dxmhelp.history_box_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.history_box_label))
	    new_widget->dxmhelp.history_box_label =
		XmStringCopy (request_widget->dxmhelp.history_box_label);
	else
	    new_widget->dxmhelp.history_box_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.history_box_label  = 
	  (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSHelptopichistory,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Search title box labels.
    */
    if (request_widget->dxmhelp.searchtitle_box_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.searchtitle_box_label))
	    new_widget->dxmhelp.searchtitle_box_label =
		XmStringCopy (request_widget->dxmhelp.searchtitle_box_label);
	else
	    new_widget->dxmhelp.searchtitle_box_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.searchtitle_box_label  = 
	 (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSSearchtopictitles,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.titles_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.titles_label))
	    new_widget->dxmhelp.titles_label =
		XmStringCopy (request_widget->dxmhelp.titles_label);
	else
	    new_widget->dxmhelp.titles_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.titles_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSTitles,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Search keyword box labels.
    */
    if (request_widget->dxmhelp.searchkeyword_box_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.searchkeyword_box_label))
	    new_widget->dxmhelp.searchkeyword_box_label =
		XmStringCopy (request_widget->dxmhelp.searchkeyword_box_label);
	else
	    new_widget->dxmhelp.searchkeyword_box_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.searchkeyword_box_label  = 
	 	  (XmString)
		  DXmGetLocaleString((I18nContext)NULL,DXmSSearchtopickeywords,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.keywords_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.keywords_label))
	    new_widget->dxmhelp.keywords_label =
		XmStringCopy (request_widget->dxmhelp.keywords_label);
	else
	    new_widget->dxmhelp.keywords_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.keywords_label  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSKeywords,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    /*
    ** Error message labels.
    */
    if (request_widget->dxmhelp.badlib_message != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.badlib_message))
	    new_widget->dxmhelp.badlib_message =
		XmStringCopy (request_widget->dxmhelp.badlib_message);
	else
	    new_widget->dxmhelp.badlib_message = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.badlib_message  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSBadlibrary,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.badframe_message != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.badframe_message))
	    new_widget->dxmhelp.badframe_message =
		XmStringCopy (request_widget->dxmhelp.badframe_message);
	else
	    new_widget->dxmhelp.badframe_message = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.badframe_message  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSBadframe,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.nulllib_message != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.nulllib_message))
	    new_widget->dxmhelp.nulllib_message =
		XmStringCopy (request_widget->dxmhelp.nulllib_message);
	else
	    new_widget->dxmhelp.nulllib_message = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.nulllib_message  = 
	       (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSNulllibrary,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.notitle_message != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.notitle_message))
	    new_widget->dxmhelp.notitle_message =
		XmStringCopy (request_widget->dxmhelp.notitle_message);
	else
	    new_widget->dxmhelp.notitle_message = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.notitle_message  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSNotitle,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.nokeyword_message != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.nokeyword_message))
	    new_widget->dxmhelp.nokeyword_message =
		XmStringCopy (request_widget->dxmhelp.nokeyword_message);
	else
	    new_widget->dxmhelp.nokeyword_message = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.nokeyword_message  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSNokeyword,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.erroropen_message != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.erroropen_message))
	    new_widget->dxmhelp.erroropen_message =
		XmStringCopy (request_widget->dxmhelp.erroropen_message);
	else
	    new_widget->dxmhelp.erroropen_message = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.erroropen_message  = 
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSErroropen,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.acknowledge_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.acknowledge_label))
	    new_widget->dxmhelp.acknowledge_label =
		XmStringCopy(request_widget->dxmhelp.acknowledge_label);
	else
	    new_widget->dxmhelp.acknowledge_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
	new_widget->dxmhelp.acknowledge_label  = 
		(XmString)
		DXmGetLocaleString((I18nContext)NULL,DXmSHelpAcknowledgeLabel,I18NVERB | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    /*
    ** The mnemonic character set strings
    */
    if (request_widget->dxmhelp.about_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.about_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.about_label_mnem_cs);
	new_widget->dxmhelp.about_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.about_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.about_label_mnem_cs = (XmStringCharSet) NULL;
#endif /* HELP_MNEM_CHARSET */
#endif 	/* Remove everything except On Windows from Using Help Menu */


#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (request_widget->dxmhelp.oncontext_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.oncontext_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.oncontext_label_mnem_cs);
	new_widget->dxmhelp.oncontext_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.oncontext_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.oncontext_label_mnem_cs = (XmStringCharSet) NULL;
#endif /* HELP_MNEM_CHARSET */
#endif 	/* Remove everything except On Windows from Using Help Menu */

    if (request_widget->dxmhelp.copy_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.copy_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.copy_label_mnem_cs);
	new_widget->dxmhelp.copy_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.copy_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.copy_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.edit_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.edit_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.edit_label_mnem_cs);
	new_widget->dxmhelp.edit_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.edit_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.edit_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.exit_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.exit_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.exit_label_mnem_cs);
	new_widget->dxmhelp.exit_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.exit_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.exit_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.file_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.file_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.file_label_mnem_cs);
	new_widget->dxmhelp.file_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.file_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.file_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (request_widget->dxmhelp.glossary_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.glossary_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.glossary_label_mnem_cs);
	new_widget->dxmhelp.glossary_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.glossary_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.glossary_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */
#endif 	/* Remove everything except On Windows from Using Help Menu */

    if (request_widget->dxmhelp.goover_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.goover_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.goover_label_mnem_cs);
	new_widget->dxmhelp.goover_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.goover_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.goover_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.help_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.help_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.help_label_mnem_cs);
	new_widget->dxmhelp.help_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.help_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.help_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.history_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.history_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.history_label_mnem_cs);
	new_widget->dxmhelp.history_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.history_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.history_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.keyword_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.keyword_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.keyword_label_mnem_cs);
	new_widget->dxmhelp.keyword_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.keyword_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.keyword_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.saveas_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.saveas_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.saveas_label_mnem_cs);
	new_widget->dxmhelp.saveas_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.saveas_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.saveas_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.search_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.search_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.search_label_mnem_cs);
	new_widget->dxmhelp.search_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.search_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.search_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.selectall_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.selectall_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.selectall_label_mnem_cs);
	new_widget->dxmhelp.selectall_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.selectall_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.selectall_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.title_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.title_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.title_label_mnem_cs);
	new_widget->dxmhelp.title_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.title_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.title_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.view_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.view_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.view_label_mnem_cs);
	new_widget->dxmhelp.view_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.view_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.view_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.visitglos_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.visitglos_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.visitglos_label_mnem_cs);
	new_widget->dxmhelp.visitglos_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.visitglos_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.visitglos_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.goto_topic_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.goto_topic_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.goto_topic_label_mnem_cs);
	new_widget->dxmhelp.goto_topic_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.goto_topic_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.goto_topic_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.goback_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.goback_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.goback_label_mnem_cs);
	new_widget->dxmhelp.goback_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.goback_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.goback_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.visit_topic_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.visit_topic_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.visit_topic_label_mnem_cs);
	new_widget->dxmhelp.visit_topic_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.visit_topic_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.visit_topic_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */

    if (request_widget->dxmhelp.helphelp_label_mnem_cs != NULL)
	{
	s = XtMalloc (strlen(request_widget->dxmhelp.helphelp_label_mnem_cs) + 1);
	strcpy (s, request_widget->dxmhelp.helphelp_label_mnem_cs);
	new_widget->dxmhelp.helphelp_label_mnem_cs = (XmStringCharSet) s;
	}
    else
#ifdef HELP_MNEM_CHARSET
	new_widget->dxmhelp.helphelp_label_mnem_cs = ALLOC_CHARSET;
#else
	new_widget->dxmhelp.helphelp_label_mnem_cs = (XmStringCharSet) NULL;
#endif  /* HELP_MNEM_CHARSET */


#ifdef HELP_LOCALE_MNEMONICS
#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (request_widget->dxmhelp.about_label_mnem == NULL)
	new_widget->dxmhelp.about_label_mnem = 
		DXmGetLocaleMnemonic((I18nContext)NULL,request_widget,
			DXmSAboutmnem,new_widget->dxmhelp.about_label_mnem_cs);
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (request_widget->dxmhelp.oncontext_label_mnem == NULL)
        new_widget->dxmhelp.oncontext_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,request_widget,
			DXmSOncontextmnem,
			new_widget->dxmhelp.oncontext_label_mnem_cs);
#endif 	/* Remove everything except On Windows from Using Help Menu */

    if (request_widget->dxmhelp.copy_label_mnem == NULL)
        new_widget->dxmhelp.copy_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSCopymnem,new_widget->dxmhelp.copy_label_mnem_cs);

    if (request_widget->dxmhelp.edit_label_mnem == NULL)
        new_widget->dxmhelp.edit_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSEditmnem,new_widget->dxmhelp.edit_label_mnem_cs);

    if (request_widget->dxmhelp.exit_label_mnem == NULL)
        new_widget->dxmhelp.exit_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSExitmnem,new_widget->dxmhelp.exit_label_mnem_cs);

    if (request_widget->dxmhelp.file_label_mnem == NULL)
        new_widget->dxmhelp.file_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSFilemnem,new_widget->dxmhelp.file_label_mnem_cs);

#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (request_widget->dxmhelp.glossary_label_mnem == NULL)
        new_widget->dxmhelp.glossary_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSGlossarymnem,
                        new_widget->dxmhelp.glossary_label_mnem_cs);
#endif 	/* Remove everything except On Windows from Using Help Menu */

    if (request_widget->dxmhelp.goover_label_mnem == NULL)
        new_widget->dxmhelp.goover_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSGoovermnem,
			new_widget->dxmhelp.goover_label_mnem_cs);

    if (request_widget->dxmhelp.help_label_mnem == NULL)
        new_widget->dxmhelp.help_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSHelpmnem,new_widget->dxmhelp.help_label_mnem_cs);

    if (request_widget->dxmhelp.history_label_mnem == NULL)
        new_widget->dxmhelp.history_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSHistorymnem,
			new_widget->dxmhelp.history_label_mnem_cs);

    if (request_widget->dxmhelp.keyword_label_mnem == NULL)
        new_widget->dxmhelp.keyword_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSKeywordmnem,
                        new_widget->dxmhelp.keyword_label_mnem_cs);

    if (request_widget->dxmhelp.saveas_label_mnem == NULL)
        new_widget->dxmhelp.saveas_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSSaveasmnem,
                        new_widget->dxmhelp.saveas_label_mnem_cs);

    if (request_widget->dxmhelp.search_label_mnem == NULL)
        new_widget->dxmhelp.search_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSSearchmnem,
			new_widget->dxmhelp.search_label_mnem_cs);

    if (request_widget->dxmhelp.selectall_label_mnem == NULL)
        new_widget->dxmhelp.selectall_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSSelectallmnem,
			new_widget->dxmhelp.selectall_label_mnem_cs);

    if (request_widget->dxmhelp.title_label_mnem == NULL)
        new_widget->dxmhelp.title_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSTitlemnem,new_widget->dxmhelp.title_label_mnem_cs);

    if (request_widget->dxmhelp.view_label_mnem == NULL)
        new_widget->dxmhelp.view_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSViewmnem,new_widget->dxmhelp.view_label_mnem_cs);

    if (request_widget->dxmhelp.visitglos_label_mnem == NULL)
        new_widget->dxmhelp.visitglos_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSVisitglossarymnem,
			new_widget->dxmhelp.visitglos_label_mnem_cs);

    if (request_widget->dxmhelp.goto_topic_label_mnem == NULL)
        new_widget->dxmhelp.goto_topic_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSGotoTopicmnem,
                        new_widget->dxmhelp.goto_topic_label_mnem_cs);

    if (request_widget->dxmhelp.goback_label_mnem == NULL)
        new_widget->dxmhelp.goback_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSGobackmnem,
			new_widget->dxmhelp.goback_label_mnem_cs);

    if (request_widget->dxmhelp.visit_topic_label_mnem == NULL)
        new_widget->dxmhelp.visit_topic_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSVisitTopicmnem,
                        new_widget->dxmhelp.visit_topic_label_mnem_cs);

    if (request_widget->dxmhelp.helphelp_label_mnem == NULL)
        new_widget->dxmhelp.helphelp_label_mnem =
                DXmGetLocaleMnemonic((I18nContext)NULL,(Widget) request_widget,
			DXmSHelpHelpmnem,
                        new_widget->dxmhelp.helphelp_label_mnem_cs);

#endif /* HELP_LOCALE_MNEMONICS */

    /*
    ** Help widget title labels.
    */
    if (request_widget->dxmhelp.helpontitle_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.helpontitle_label))
	    new_widget->dxmhelp.helpontitle_label =
		XmStringCopy (request_widget->dxmhelp.helpontitle_label);
	else
	    new_widget->dxmhelp.helpontitle_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
        new_widget->dxmhelp.helpontitle_label =
	       (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSHelpontitle,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */
    if (request_widget->dxmhelp.helptitle_label != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.helptitle_label))
	    new_widget->dxmhelp.helptitle_label =
		XmStringCopy (request_widget->dxmhelp.helptitle_label);
	else
	    new_widget->dxmhelp.helptitle_label = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
        new_widget->dxmhelp.helptitle_label =
		(XmString) DXmGetLocaleString((I18nContext)NULL,DXmSHelptitle,I18NNOUN | I18NLABEL);
#endif /* HELP_LOCALE_STRINGS */

    /*
    **  Help on Help title
    */
    if (request_widget->dxmhelp.helponhelp_title != NULL)
	{
	if (!XmStringEmpty(request_widget->dxmhelp.helponhelp_title))
	    new_widget->dxmhelp.helponhelp_title =
		XmStringCopy(request_widget->dxmhelp.helponhelp_title);
	else
	    new_widget->dxmhelp.helponhelp_title = NULL;
	}
#ifdef HELP_LOCALE_STRINGS
     else
        new_widget->dxmhelp.helponhelp_title =
	   (XmString) DXmGetLocaleString((I18nContext)NULL,DXmSHelpOnHelpTitle,I18NNOUN | I18NMENU);
#endif /* HELP_LOCALE_STRINGS */

/* I18N START*/
    if ( new_widget->dxmhelp.button_font_list == (XmFontList) NULL ){
	new_widget->dxmhelp.button_font_list =
	  (XmFontList) DXmFontListCreateDefault ((Widget) new_widget, XmNbuttonFontList);
#ifdef DEBUG
	printf("I18n default font is set for button in help widget\n");
    } else {
	printf("Use default font in resource for button in help\n");
#endif
    }
    if ( new_widget->dxmhelp.label_font_list == (XmFontList) NULL ){
	new_widget->dxmhelp.label_font_list =
	  (XmFontList) DXmFontListCreateDefault ((Widget) new_widget, XmNlabelFontList);
#ifdef DEBUG
	printf("I18n default font is set for label in help widget\n");
    } else {
	printf("Use default font in resource for label in help\n");
#endif
    }
    if ( new_widget->dxmhelp.text_font_list == (XmFontList) NULL ){
	new_widget->dxmhelp.text_font_list =
	  (XmFontList) DXmFontListCreateDefault ((Widget) new_widget, XmNtextFontList);
#ifdef DEBUG
	printf("I18n default font is set for text in help widget\n");
    } else {
	printf("Use default font in resource for text in help\n");
#endif
    }
/* I18N END */

    /*
    **  Caching mode
    */

    new_widget->dxmhelp.cache_library = request_widget->dxmhelp.cache_library;

    /*
    ** Store the help widget's shell.
    */
    new_widget->dxmhelp.shell = new_widget->core.parent;

    /*
    ** Set the shell title.
    */ 
    set_helpshell_title (request_widget, new_widget);

#ifdef DIALOG_STYLE_SUPPORT
    /*
    ** If parent is DialogShell, set dialog attributes and realize.
    */
    if ( XmIsDialogShell (XtParent (request))    )
        {
        new_widget->dxmhelp.shell = XtParent( request_widget) ;

        switch(    request_widget->dxmhelp.dialog_style    )
            {
            case XmDIALOG_PRIMARY_APPLICATION_MODAL:
                {   mwmStyle = MWM_INPUT_PRIMARY_APPLICATION_MODAL ;
                    break ;
                }
            case XmDIALOG_FULL_APPLICATION_MODAL:
                {   mwmStyle = MWM_INPUT_FULL_APPLICATION_MODAL ;
                    break;
                }
            case XmDIALOG_SYSTEM_MODAL:
                {   mwmStyle = MWM_INPUT_SYSTEM_MODAL ;
                    break ;
                }
            case XmDIALOG_MODELESS:
            default:
                {   mwmStyle = MWM_INPUT_MODELESS ;
                    break ;
                }
            }
        XtSetArg( al[0], XmNmwmInputMode, mwmStyle) ;
        XtSetValues( new_widget->dxmhelp.shell, al, (Cardinal) 1) ;
        XtRealizeWidget( new_widget->dxmhelp.shell) ;
        };


    /*  Validate dialog style.
    */
    if (!new_widget->dxmhelp.shell
        && (request_widget->dxmhelp.dialog_style != XmDIALOG_WORK_AREA)    )
        {
        _XmWarning( (Widget) new_widget, WARN_DIALOG_STYLE) ;
        new_widget->dxmhelp.dialog_style = XmDIALOG_WORK_AREA ;
        }
    else
	{   if ( new_widget->dxmhelp.shell
            && ( request_widget->dxmhelp.dialog_style != XmDIALOG_MODELESS)
            && ( request_widget->dxmhelp.dialog_style != XmDIALOG_SYSTEM_MODAL)
            && ( request_widget->dxmhelp.dialog_style
                                         != XmDIALOG_PRIMARY_APPLICATION_MODAL)
            && (request_widget->dxmhelp.dialog_style
                                       != XmDIALOG_FULL_APPLICATION_MODAL)    )
            {   _XmWarning( (Widget) new_widget, WARN_DIALOG_STYLE) ;
                new_widget->dxmhelp.dialog_style = XmDIALOG_MODELESS ;
            }
        }
#endif  /*DIALOG_STYLE_SUPPORT*/

    /*
    ** Initialize the help widget's private record.
    */
    init_help (new_widget, TRUE);
    new_widget->dxmhelp.history_box = NULL;
    new_widget->dxmhelp.title_box = NULL;
    new_widget->dxmhelp.keyword_box = NULL;
    new_widget->dxmhelp.saveas_box = NULL;
    new_widget->dxmhelp.message_box = NULL;
    new_widget->dxmhelp.help_on_help = NULL;
    new_widget->dxmhelp.help_on_saveas = NULL;
        
    /*
    ** build the array of widgets to avoid
    */
    build_avoid_widgets (new_widget);

    /*
    ** If a first_topic was specified, push it on top of the trail list.
    */
#ifdef NULL_FIRST_TOPIC_BUG
    /* The Toolkit Ref. Man. states that if a NULL string is provided
     * for a first_topic, the overview_topic will be displayed.
     */
    if (new_widget->dxmhelp.first_topic == NULL )
       insert_trail (new_widget, new_widget->dxmhelp.overview) ;
    else
#endif /* NULL_FIRST_TOPIC_BUG */
    insert_trail (new_widget, new_widget->dxmhelp.first_topic);
	            
    /*
    ** Open the library.
    */
    open_library(new_widget);

    /*
    **  Create the wait cursor
    */

    new_widget->dxmhelp.cursor = HwCreateWaitCursor(new_widget);
    
    /*
    ** Build the menubar and the work area.
    */  
    build_menubar (new_widget);
    build_work_area (new_widget);

    /*
    ** Compute the widget's size
    */
    XtWidth(new_widget) = 5;
    XtHeight(new_widget) = 5;

#ifdef HELP_MNEM_CHARSET
/*
 * Comment out these lines - as of 1.2 DXmGetLocaleCharset returns a pointer 
 * directly to data stored in a locale cache rather than a copy of this data.
 * Freeing a charset returned by DXmGetLocaleCharset or I18nGetLocaleCharset 
 * will cause random memory corruption to occur.
 *
 *   if (CurrentCharset)
 *       XtFree(CurrentCharset);
 */
#endif /* HELP_MNEM_CHARSET */
}

static void change_widget(w, g, instigator)
    Widget w;
    XtWidgetGeometry *g;
    Widget instigator;
/*
**++
**  Functional Description:
**	Change the children widgets' geometry.
**
**  Keywords:
**	Geometry management
**
**  Arguments:
**	w: widget id of the child.
**	g: geometry of the child widget.
**	instigator: instigator of the geometry change.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    if (w == NULL) return;
    
    /*
    ** Initialize the geometry mask.
    */
    g->request_mode = 0;

    /*
    ** Set the geometry mask and the geometry structure.
    */
    if (XtX(w)	    != g->x)
	g->request_mode |= CWX;
    if (XtY(w)	    != g->y)
	g->request_mode |= CWY;
    if (XtWidth(w)  != g->width)
	g->request_mode |= CWWidth;
    if (XtHeight(w) != g->height)
	g->request_mode |= CWHeight;

    /*
    ** Change the child window geometry and notify the child widget that it
    ** should recalculate the layout of internal data as needed.
    */
    if (g->request_mode)
    {
	_XmConfigureObject ( (Widget) w,
			     g->x, g->y,
			     g->width, g->height,
			     g->border_width);

	/*
	** If child widget isn't the instigator, notify the child widget.
	*/
	if (instigator != w)
	    (* (XtCoreProc (w, resize))) (w);	

    }
}


static void load_geo (w, g)
    Widget w;
    XtWidgetGeometry *g;
/*
**++
**  Functional Description:
**	Load the given widget geometry.
**
**  Keywords:
**	Geometry management
**
**  Arguments:
**	w: widget id
**	g: geometry structure.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    if (w != NULL)
    {
	g->width = XtWidth (w);
	g->height = XtHeight (w);
	g->x = XtX (w);
	g->y = XtY (w);
	g->border_width = XtBorderWidth (w);
    }
    else
    {
	g->width = 0;
	g->height = 0;
	g->x = 0;
	g->y = 0;
	g->border_width = 0;
    }
}

static void find_menubar_height (m, g)
    Widget m;
    XtWidgetGeometry *g;
/*
**++
**  Functional Description:
**	Find the current menubar height.
**
**  Keywords:
**	Geometry management
**
**  Arguments:
**	m: menubar widget id
**	g: geometry structure.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    if (m == NULL) return;

    /*
    ** If it's not a menu load its height.
    */
    if ( ! XmIsBulletinBoard(m) )
    {
	g->height = XtHeight (m);		
    }
    else
    {
	XtWidgetGeometry intended, reply;

	intended.request_mode = CWWidth;
	intended.width        = g->width;

	/*
	** Ask about the width; if it affects the size it will tell us.
	*/
	switch (XtQueryGeometry (m, &intended, &reply))
	{
	    /*
	    ** Wants to compromise, so take suggested height or just use
	    ** existing one.
	    */
	    case XtGeometryAlmost:		

		if (reply.request_mode & CWHeight)
		{
		    g->height = reply.height;	
		    return;
		}				
	    /*
	    ** Agrees, so take current height.
	    */
	    case XtGeometryYes:			
	    /*
	    ** Disagrees, so take current height.
	    */
	    case XtGeometryNo:			
		g->height = XtHeight (m);
		break;
	}
    }
}

static void
change_help_size (w, width, height)
    Widget w;
    int width, height;
/*
**++
**  Functional Description:
**	Change the size of the help widget.
**
**  Keywords:
**	Geometry management
**
**  Arguments:
**	w: help widget id
**	width, height: width and height to set. 
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;

    XtWidgetGeometry g;

    g.request_mode = CWWidth | CWHeight; 
    g.width  = width;
    g.height = height;

    _XmMakeGeometryRequest ((Widget) help_widget, &g);
}

static void Layout (w, instigator, bottom_up_change)
    Widget w;
    Widget instigator;
    int bottom_up_change;
/*
**++
**  Functional Description:
**	Compute the layout of all children and change their geometry.
**	If bottom_up_change is true, help must adapt to the text widget else
**	the text widget must fit help. After all the user via the window manager
**	has final authority.
**
**  Keywords:
**	Geometry management
**
**  Arguments:
**	w: pointer to the help widget's structure.
**	instigator: widget id of the instigator.
**	bottom_up_change: boolean flag.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    int tw, th;
    XtWidgetGeometry menu_geo, text_geo, add_geo, topic_geo, goback_geo,
		    quit_geo, help_geo;
    
    /*
    ** Check if the help widget is realized else return.
    */
    if (! XtIsRealized(help_widget)) return;

    /*
    ** Load initial geometry informations.
    */
    load_geo (help_widget->dxmhelp.menubar, &menu_geo); 
    load_geo (XtParent(help_widget->dxmhelp.help_text), &text_geo); 
    load_geo (help_widget->dxmhelp.add_topic, &add_geo); 
    load_geo (XtParent(help_widget->dxmhelp.help_topic), &topic_geo); 
    load_geo (help_widget->dxmhelp.goback, &goback_geo); 
    load_geo (help_widget->dxmhelp.quit, &quit_geo);
    load_geo (help_widget, &help_geo);

    /*
    ** Compute total width of everything except text widget.
    */
    tw = text_geo.border_width * 2;
    
    /*
    ** Compute the width of the help widget and of the text widget.
    */
    if (bottom_up_change)
	help_geo.width = text_geo.width + tw;
    else
	/*
	** Check for negative overflow.
	*/
	if (tw >= help_geo.width)
	    text_geo.width = 1;
	else
	    text_geo.width = help_geo.width - tw;
		

    /*
    ** Compute menubar width and height.
    */
    if (help_widget->dxmhelp.menubar != NULL)
    {
	XmRowColumnWidget m = (XmRowColumnWidget) help_widget->dxmhelp.menubar;
	menu_geo.width = help_geo.width - (XtBorderWidth (m) * 2);
	find_menubar_height (m, &menu_geo);
    }
        
    /*
    ** Compute total height of everything except the text widget.
    */
    th = menu_geo.height
       + add_geo.height
       + topic_geo.height
       + 45;

    /*
    ** Compute the height of the help widget and text widget.
    */
    if (bottom_up_change)
	help_geo.height = text_geo.height + th;
    else
	/*
	** Check for negative overflow.
	*/
	if (th >= help_geo.height)
	    text_geo.height = 1;
	else
	    text_geo.height = help_geo.height - th;
	
    /*
    ** Fill in additional topic label and topic listbox width.
    */
    add_geo.width = help_geo.width
		  - (add_geo.border_width * 2);
    topic_geo.width = help_geo.width
		  - (topic_geo.border_width * 2);

    /*
    ** Fill in the positioning information.
    */
    menu_geo.x = 0;
    menu_geo.y = 0;
    text_geo.x = 0;
    text_geo.y = menu_geo.y
		+ menu_geo.height
		+ (menu_geo.border_width * 2);
    add_geo.x = help_geo.border_width;
    add_geo.y = text_geo.y
	      + text_geo.height
	      + (text_geo.border_width * 2)
	      + 5;
    topic_geo.x = 0;
    topic_geo.y = add_geo.y
		+ add_geo.height
		+ (add_geo.border_width * 2);

#ifdef RTOL
    if (LayoutIsRtoL(help_widget)) {
#else
    if (help_widget->dxmhelp.string_direction) {
#endif /* RTOL */
	goback_geo.x = help_geo.width - goback_geo.width - 10;
	quit_geo.x = 10;
    }
    else {
	goback_geo.x = 10;
	quit_geo.x = help_geo.width - quit_geo.width - 10;
    }

    goback_geo.y = help_geo.height - 30;
    quit_geo.y = help_geo.height - 30;

    /*
    ** If help has to fit to the text widget resize it.
    */
    if (bottom_up_change)
	change_help_size (help_widget, help_geo.width, help_geo.height);
    /*
    ** Resize the widgets.
    */
    change_widget (help_widget->dxmhelp.menubar, &menu_geo, instigator);
    change_widget (XtParent(help_widget->dxmhelp.help_text), &text_geo, instigator);
    change_widget (help_widget->dxmhelp.add_topic, &add_geo, instigator);
    change_widget (XtParent(help_widget->dxmhelp.help_topic), &topic_geo, instigator);
    change_widget (help_widget->dxmhelp.goback, &goback_geo, instigator);
    change_widget (help_widget->dxmhelp.quit, &quit_geo, instigator);
}

    
static void Realize (w, window_mask, window_attributes)
    Widget		    w;
    Mask		    *window_mask;
    XSetWindowAttributes    *window_attributes;
/*
**++
**  Functional Description:
**	Create a window for an instance of the help widget.
**
**  Keywords:
**	Help creation.
**
**  Arguments:
**	w: pointer to the help widget's structure.
**	window_mask: specifies which fields in the attributes structure to use.
**	window_attributes: specifies the window attributes to use.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget) w;
    int savex, savey;

    /*
    ** If the widget is already realized, return.
    */
    if (XtIsRealized (w)) return;

    /*
    ** Save the x and y positions.
    */
    savex = XtX (w);
    savey = XtY (w);

    /*
    ** Set the window at (-border_width, -border_width) to avoid flicker
    */

    XtX(w) = XtY(w) = -XtBorderWidth(w);

    /*
    ** Create the window.
    */
    XtCreateWindow (w, InputOutput, CopyFromParent, *window_mask,
		    window_attributes) ;

    /*
    ** Set the saved values back
    */

    XtX(w) = savex;
    XtY(w) = savey;

    /*
    ** Compute the layout of the help widget's children.
    */
    Layout (w, NULL, TRUE);

    /*
    **	If error message to display do it
    */
    
    check_message(w, help_widget);

    {
    /*
    **	On some low-resolution displays, it is possible that the help widget,
    **	as realized, is too tall to allow the control buttons at the bottom to
    **  to be visible -- rendering the display inoperable.
    **  We check to see if the widget, as realized, is entirely visible.  If it
    **	isn't, we shorten it by the amount reported too big and cause a recalc
    **  of our own size
    */
    Dimension height_clipped, width_clipped;
    Position returned_x, returned_y;
    int dest_x, dest_y;

    if ( ! _DXmCheckFit (help_widget, &height_clipped, &width_clipped)) {
	XtHeight(help_widget) = XtHeight(help_widget) - height_clipped;
 	(* (XtCoreProc (help_widget, resize))) ((Widget)help_widget);	
    }

    {
    /*
    ** This block of code is here to protect ourselves from the action of the
    ** window manager's Close action -- also affectionately known as f.kill.
    **
    ** We do two things:
    **
    **    . Establish a callback so that a piece of our code --
    **      'close_call_from_WM' -- gets control when the action happens.
    **      From within that callback we mimic a call to
    **      our 'quit' function -- the callback that normally would get control
    **	    if the 'Exit' button is pressed.  This allows us to shut down
    **	    other children windows -- like the Search boxes -- in an orderly
    **	    fashion.
    **
    **    . Jam a state of XmDO_NOTHING into our parent's XmNdeleteResponse
    **	    resource.
    **      [Note: most of the time our parent is our own DXmHelpShell widget.]
    **	    Doing this keeps the superclass vendor shell from destroying itself
    **	    when asked to in response to this Close action.
    **      If we don't do this, the Close operation deletes our shell [parent].
    **	    If the application now tries to re-use us-- in particular, tries to
    **	    use SetValues to hand us a new topic string, it will accvio because
    **	    we no longer have a parent.
    **
    ** To do the former,
    ** register for WM protocols we're willing to participate in.
    **
    **  WM_DELETE_WINDOW - clients are notified when the MWM f.kill function
    **      is invoked by the user.  MWM does not terminate the client or
    **      destroy the window when a WM_DELETE_WINDOW notification is done.
    **
    */
    Atom delete_window_atom;
    Arg arglist[1];

    delete_window_atom = XmInternAtom(XtDisplay(XtParent(help_widget)),/* shell*/
                         "WM_DELETE_WINDOW", False);

    XmAddWMProtocolCallback(
                XtParent(help_widget),                  /* help widget shell  */
                delete_window_atom,
                (XtCallbackProc) close_call_from_WM,    /* callback routine   */
                (XtPointer) help_widget);                 /* Pass help widget as*/
                                                        /* private data       */


    XmActivateWMProtocol(XtParent(help_widget), delete_window_atom);

    /*
    ** Make our shell do nothing if asked to destroy itself in response to
    ** a WM_DELETE_WINDOW message as a result of Close                 
    */

    XtSetArg (arglist[0], XmNdeleteResponse, XmDO_NOTHING);
    XtSetValues (XtParent(help_widget), arglist, 1);

    }

    }
}

static void free_help (help_widget, full)
    DXmHelpWidget help_widget;
    Boolean full;
/*
**++
**  Functional Description:
**	Free help widget's memory allocation.
**
**  Keywords:
**	Help update
**
**  Arguments:
**	help_widget: widget id of the help_widget.
**	full: full deallocation or not.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    /*
    ** Free the topic and title arrays.
    */
    free_array (help_widget->dxmhelp.topic_array,
		help_widget->dxmhelp.topic_count);
    free_array (help_widget->dxmhelp.title_array,
		help_widget->dxmhelp.topic_count);

    /*
    ** Free the selected topic and selected title.
    */
    XtFree ((char *)help_widget->dxmhelp.selected_topic);
    XtFree ((char *)help_widget->dxmhelp.selected_title);

    /*
    ** Free the help text.
    */
    XtFree ((char *)help_widget->dxmhelp.text);

    /*
    ** Free the trail list
    */
    free_trail (help_widget);

    if (full)
    {
	int count;
	Arg arglist[2];

	
	/*
	** Free the history list
	*/
	if (help_widget->dxmhelp.history_box != NULL)
	    free_history (help_widget);

	if (help_widget->dxmhelp.title_box != NULL)
	{
	    /*
	    ** Free the selected searchtitle topic.
	    */
	    XtFree ((char *)help_widget->dxmhelp.selected_searchtitle_topic);
	    /*
	    ** Free the searchtitle topic array.
	    */
	    if (help_widget->dxmhelp.searchtitle_topic_array != NULL)
	    {
		XtSetArg (arglist[0], XmNitemCount, &count);
		XtGetValues (help_widget->dxmhelp.title_box->listbox1, arglist, 1);
		free_array (help_widget->dxmhelp.searchtitle_topic_array, count);
	    }
	}

	if (help_widget->dxmhelp.keyword_box != NULL)
	{
	    /*
	    ** Free the selected keyword.
	    */
	    XtFree ((char *)help_widget->dxmhelp.selected_keyword);
	    /*
	    ** Free the keyword array.
	    */
	    if (help_widget->dxmhelp.keyword_array != NULL)
	    {
		XtSetArg (arglist[0], XmNitemCount, &count);
		XtGetValues (help_widget->dxmhelp.keyword_box->listbox1, arglist, 1);
		free_array (help_widget->dxmhelp.keyword_array, count);
	    }
	    /*
	    ** Free the selected searchkeyword topic.
	    */
	    XtFree ((char *)help_widget->dxmhelp.selected_searchkeyword_topic);
	    /*
	    ** Free the searchkeyword topic array.
	    */
	    if (help_widget->dxmhelp.searchkeyword_topic_array != NULL)
	    {
		XtSetArg (arglist[0], XmNitemCount, &count);
		XtGetValues (help_widget->dxmhelp.keyword_box->listbox2, arglist, 1);
		free_array (help_widget->dxmhelp.searchkeyword_topic_array, count);
	    }
	}
	     
	/*
	** Free the message text
	*/
	if (help_widget->dxmhelp.message_text != NULL)
	    XtFree ((char *)help_widget->dxmhelp.message_text);
	if (help_widget->dxmhelp.message_parameter != NULL)
	    XtFree ((char *)help_widget->dxmhelp.message_parameter);
    }
}

static void Destroy (w)
    Widget w;
/*
**++
**  Functional Description:
**	Destroy the help widget's specific data.
**
**  Keywords:
**	Help destroy.
**
**  Arguments:
**	w: pointer to the help widget's structure.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget help_widget = (DXmHelpWidget)w;
        
    /*
    ** Free the string storage.
    */
    XtFree ((char *)help_widget->dxmhelp.title);
    XtFree ((char *)help_widget->dxmhelp.application);
    XtFree ((char *)help_widget->dxmhelp.library);
    XtFree ((char *)help_widget->dxmhelp.first_topic);
    XtFree ((char *)help_widget->dxmhelp.overview);
    XtFree ((char *)help_widget->dxmhelp.glossary);
    XtFree ((char *)help_widget->dxmhelp.view_menu_label);
    XtFree ((char *)help_widget->dxmhelp.goto_topic_label);
    XtFree ((char *)help_widget->dxmhelp.goback_topic_label);
    XtFree ((char *)help_widget->dxmhelp.goover_label);
    XtFree ((char *)help_widget->dxmhelp.visit_topic_label);
    XtFree ((char *)help_widget->dxmhelp.visitglos_label);
    XtFree ((char *)help_widget->dxmhelp.file_menu_label);
    XtFree ((char *)help_widget->dxmhelp.saveas_label);
    XtFree ((char *)help_widget->dxmhelp.exit_label);
    XtFree ((char *)help_widget->dxmhelp.edit_menu_label);
    XtFree ((char *)help_widget->dxmhelp.copy_label);
    XtFree ((char *)help_widget->dxmhelp.selectall_label);
    XtFree ((char *)help_widget->dxmhelp.search_menu_label);
    XtFree ((char *)help_widget->dxmhelp.history_label);
    XtFree ((char *)help_widget->dxmhelp.title_label);
    XtFree ((char *)help_widget->dxmhelp.keyword_label);
    XtFree ((char *)help_widget->dxmhelp.help_menu_label);
    XtFree ((char *)help_widget->dxmhelp.helphelp_label);

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtFree (help_widget->dxmhelp.glossary_label);
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtFree (help_widget->dxmhelp.about_label);
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtFree (help_widget->dxmhelp.oncontext_label);
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtFree ((char *)help_widget->dxmhelp.addtopic_label);
    XtFree ((char *)help_widget->dxmhelp.goback_label);
    XtFree ((char *)help_widget->dxmhelp.quit_label);
    XtFree ((char *)help_widget->dxmhelp.goto_label);
    XtFree ((char *)help_widget->dxmhelp.visit_label);
    XtFree ((char *)help_widget->dxmhelp.apply_label);
    XtFree ((char *)help_widget->dxmhelp.dismiss_label);
    XtFree ((char *)help_widget->dxmhelp.topic_titles_label);
    XtFree ((char *)help_widget->dxmhelp.history_box_label);
    XtFree ((char *)help_widget->dxmhelp.searchtitle_box_label);
    XtFree ((char *)help_widget->dxmhelp.titles_label);
    XtFree ((char *)help_widget->dxmhelp.searchkeyword_box_label);
    XtFree ((char *)help_widget->dxmhelp.keywords_label);

    XtFree ((char *)help_widget->dxmhelp.badlib_message);
    XtFree ((char *)help_widget->dxmhelp.badframe_message);
    XtFree ((char *)help_widget->dxmhelp.nulllib_message);
    XtFree ((char *)help_widget->dxmhelp.notitle_message);
    XtFree ((char *)help_widget->dxmhelp.nokeyword_message);
    XtFree ((char *)help_widget->dxmhelp.erroropen_message);
    XtFree ((char *)help_widget->dxmhelp.helponhelp_title);
    XtFree ((char *)help_widget->dxmhelp.acknowledge_label);
            

    /*
    ** Free up the mnemonic character set strings that exist
    */
#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (help_widget->dxmhelp.about_label_mnem_cs)
	XtFree (help_widget->dxmhelp.about_label_mnem_cs);
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (help_widget->dxmhelp.oncontext_label_mnem_cs)
	XtFree (help_widget->dxmhelp.oncontext_label_mnem_cs);
#endif 	/* Remove everything except On Windows from Using Help Menu */

    if (help_widget->dxmhelp.copy_label_mnem_cs)
	XtFree (help_widget->dxmhelp.copy_label_mnem_cs);
    if (help_widget->dxmhelp.edit_label_mnem_cs)
	XtFree (help_widget->dxmhelp.edit_label_mnem_cs);
    if (help_widget->dxmhelp.exit_label_mnem_cs)
	XtFree (help_widget->dxmhelp.exit_label_mnem_cs);
    if (help_widget->dxmhelp.file_label_mnem_cs)
	XtFree (help_widget->dxmhelp.file_label_mnem_cs);

#if 0	/* Remove everything except On Windows from Using Help Menu */
    if (help_widget->dxmhelp.glossary_label_mnem_cs)
	XtFree (help_widget->dxmhelp.glossary_label_mnem_cs);
#endif 	/* Remove everything except On Windows from Using Help Menu */

    if (help_widget->dxmhelp.goover_label_mnem_cs)
	XtFree (help_widget->dxmhelp.goover_label_mnem_cs);
    if (help_widget->dxmhelp.help_label_mnem_cs)
	XtFree (help_widget->dxmhelp.help_label_mnem_cs);
    if (help_widget->dxmhelp.history_label_mnem_cs)
	XtFree (help_widget->dxmhelp.history_label_mnem_cs);
    if (help_widget->dxmhelp.keyword_label_mnem_cs)
	XtFree (help_widget->dxmhelp.keyword_label_mnem_cs);
    if (help_widget->dxmhelp.saveas_label_mnem_cs)
	XtFree (help_widget->dxmhelp.saveas_label_mnem_cs);
    if (help_widget->dxmhelp.search_label_mnem_cs)
	XtFree (help_widget->dxmhelp.search_label_mnem_cs);
    if (help_widget->dxmhelp.selectall_label_mnem_cs)
	XtFree (help_widget->dxmhelp.selectall_label_mnem_cs);
    if (help_widget->dxmhelp.title_label_mnem_cs)
	XtFree (help_widget->dxmhelp.title_label_mnem_cs);
    if (help_widget->dxmhelp.view_label_mnem_cs)
	XtFree (help_widget->dxmhelp.view_label_mnem_cs);
    if (help_widget->dxmhelp.visitglos_label_mnem_cs)
	XtFree (help_widget->dxmhelp.visitglos_label_mnem_cs);
    if (help_widget->dxmhelp.goto_topic_label_mnem_cs)
	XtFree (help_widget->dxmhelp.goto_topic_label_mnem_cs);
    if (help_widget->dxmhelp.goback_label_mnem_cs)
	XtFree (help_widget->dxmhelp.goback_label_mnem_cs);
    if (help_widget->dxmhelp.visit_topic_label_mnem_cs)
	XtFree (help_widget->dxmhelp.visit_topic_label_mnem_cs);
    if (help_widget->dxmhelp.helphelp_label_mnem_cs)
	XtFree (help_widget->dxmhelp.helphelp_label_mnem_cs);

    free_help (help_widget, TRUE);
    
    /*	
    ** Cleanup the visited widgets
    */
    DXmHelpCleanup (help_widget->dxmhelp.visit_context);
    
    /*
    ** Destroy the helpshell
    */    
    XtDestroyWidget(help_widget->dxmhelp.shell);


#if 0		/* shouldn't be necessary to destroy children */
    /*
    ** Destroy the dialog boxes.
    */
    if (help_widget->dxmhelp.history_box->box != NULL)
	XtDestroyWidget(help_widget->dxmhelp.history_box->box);
    if (help_widget->dxmhelp.title_box->box != NULL)
	XtDestroyWidget(help_widget->dxmhelp.title_box->box);
    if (help_widget->dxmhelp.keyword_box->box != NULL)
	XtDestroyWidget(help_widget->dxmhelp.keyword_box->box);
    if (help_widget->dxmhelp.saveas_box->box != NULL)
	XtDestroyWidget(help_widget->dxmhelp.saveas_box->box);
    if (help_widget->dxmhelp.message_box != NULL)
	XtDestroyWidget(help_widget->dxmhelp.message_box);
    if (help_widget->dxmhelp.help_on_help != NULL)
	XtDestroyWidget(help_widget->dxmhelp.help_on_help);
    if (help_widget->dxmhelp.help_on_saveas != NULL)
	XtDestroyWidget(help_widget->dxmhelp.help_on_saveas);

#endif	/* shouldn't be necessary to destroy children */

} 

static void Resize (w)
    Widget w;
/*
**++
**  Functional Description:
**	Resize the help widget (just call layout but can't be removed because of
**	upward compatibity).
**
**  Keywords:
**	Geometry management.
**
**  Arguments:
**	w: widget id of the widget to resize
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    Layout (w, NULL, FALSE);
}

static void clear_help (help_widget, full)
    DXmHelpWidget help_widget;
    Boolean full;
/*
**++
**  Functional Description:
**	Clear and reset the help widget's children and dialog boxes.
**
**  Keywords:
**	Help update
**
**  Arguments:
**	help_widget: widget id of the help_widget.
**	full: clear all dialog boxes or not.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    Arg arglist[10];
    int ac = 0;
    
    /*
    ** Unmanage the additional topics label and listbox.
    */
    XtUnmanageChild(help_widget->dxmhelp.add_topic);
    XtUnmanageChild(XtParent(help_widget->dxmhelp.help_topic));

    /*
    ** Clear the text widget.
    */
#ifdef CSTEXT
    DXmCSTextDisableRedisplay (help_widget->dxmhelp.help_text, FALSE);
    DXmCSTextSetTopPosition (help_widget->dxmhelp.help_text, 0);
    DXmCSTextClearSelection (help_widget->dxmhelp.help_text, CurrentTime);
    DXmCSTextSetString (help_widget->dxmhelp.help_text,
			help_widget->dxmhelp.text);
    DXmCSTextEnableRedisplay (help_widget->dxmhelp.help_text);
#else
    _XmTextDisableRedisplay (help_widget->dxmhelp.help_text, FALSE);
    XmTextSetTopCharacter (help_widget->dxmhelp.help_text, 0);
    XmTextClearSelection (help_widget->dxmhelp.help_text, CurrentTime);
    XmTextSetString (help_widget->dxmhelp.help_text,
			help_widget->dxmhelp.text);
    _XmTextEnableRedisplay (help_widget->dxmhelp.help_text);
#endif /* CSTEXT */

    /*
    ** Clear the listbox.
    */
    ac = 0;
    XtSetArg (arglist[ac], XmNitemCount, help_widget->dxmhelp.topic_count);
    ac++;
    XtSetArg (arglist[ac], XmNitems, help_widget->dxmhelp.title_array);
    ac++;
    XtSetArg (arglist[ac], XmNselectedItems, &help_widget->dxmhelp.selected_title);
    ac++;
    XtSetArg (arglist[ac], XmNselectedItemCount, 1);
    ac++;
    XtSetValues (help_widget->dxmhelp.help_topic, arglist, ac);

    /*
    ** Set the goto and visit menu items to insensitive.
    */
    XtSetSensitive (help_widget->dxmhelp.goto_button, FALSE);
    XtSetSensitive (help_widget->dxmhelp.visit_button, FALSE);

    /*
    ** Clear the dialog boxes if they exist.
    */
    if (full)
    {
	int topic_count = 0;
	int keyword_count = 0;

	if (help_widget->dxmhelp.history_box != NULL)
	{
	    /*
	    ** Clear the history listbox.
	    */
	    ac = 0;
	    XtSetArg (arglist[ac], XmNitems, NULL);
	    ac++;
	    XtSetArg (arglist[ac], XmNitemCount, help_widget->dxmhelp.history_count);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.history_box->listbox1, arglist, ac);
          /* Need to pre-def a selection of some sort for DefaultAction =kbm */
            XmListSelectPos (help_widget->dxmhelp.history_box->listbox1,
  (((XmListWidget )help_widget->dxmhelp.history_box->listbox1)->list.itemCount 
                > 1 ? 1 : 0), False);
            XmListSetPos (help_widget->dxmhelp.history_box->listbox1, 1); 
	}

	if (help_widget->dxmhelp.title_box != NULL)
	{
	    /*
	    ** Clear the searchtitle text widget.
	    */
#ifdef CSTEXT
    	    DXmCSTextSetTopPosition (help_widget->dxmhelp.title_box->text, 0);
    	    DXmCSTextClearSelection (help_widget->dxmhelp.title_box->text, CurrentTime);
    	    DXmCSTextSetString (help_widget->dxmhelp.title_box->text, NULL);
#else
	    XmTextSetTopCharacter (help_widget->dxmhelp.title_box->text, 0);
	    XmTextClearSelection (help_widget->dxmhelp.title_box->text, CurrentTime);
	    XmTextSetString (help_widget->dxmhelp.title_box->text, NULL);
#endif /* CSTEXT */

	    /*
	    ** Clear the search title listbox.
	    */
	    ac = 0;
	    XtSetArg (arglist[ac], XmNitems, NULL);
	    ac++;
	    XtSetArg (arglist[ac], XmNitemCount, topic_count);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.title_box->listbox1, arglist, ac);
          /* Need to pre-def a selection of some sort for DefaultAction =kbm */
            XmListSelectPos (help_widget->dxmhelp.title_box->listbox1,
    (((XmListWidget )help_widget->dxmhelp.title_box->listbox1)->list.itemCount 
                > 1 ? 1 : 0), False);
            XmListSetPos (help_widget->dxmhelp.title_box->listbox1, 1); 
	
	    /*
	    ** Set the goto and visit buttons to insensitive.
	    */
	    XtSetSensitive (help_widget->dxmhelp.title_box->button1, FALSE);
	    XtSetSensitive (help_widget->dxmhelp.title_box->button2, FALSE);
	}

	if (help_widget->dxmhelp.keyword_box != NULL)
	{
	    /*
	    ** Clear the searchkeyword text widget.
	    */
#ifdef CSTEXT
    	    DXmCSTextSetTopPosition (help_widget->dxmhelp.keyword_box->text, 0);
    	    DXmCSTextClearSelection (help_widget->dxmhelp.keyword_box->text, CurrentTime);
    	    DXmCSTextSetString (help_widget->dxmhelp.keyword_box->text, NULL);
#else
	    XmTextSetTopCharacter (help_widget->dxmhelp.keyword_box->text, 0);
	    XmTextClearSelection (help_widget->dxmhelp.keyword_box->text, CurrentTime);
	    XmTextSetString (help_widget->dxmhelp.keyword_box->text, NULL);
#endif

	    /*
	    ** Get the new keyword list.
	    */
	    keyword_count = get_keywords(help_widget);
	    
	    /*
	    ** Reset the searchkeyword keyword listbox.
	    */
	    ac = 0;
	    XtSetArg (arglist[ac], XmNitems, help_widget->dxmhelp.keyword_array);
	    ac++;
	    XtSetArg (arglist[ac], XmNitemCount, keyword_count);
	    ac++;
	    XtSetArg (arglist[ac], XmNselectedItems,
			&help_widget->dxmhelp.selected_keyword);
	    ac++;
	    XtSetArg (arglist[ac], XmNselectedItemCount, 1);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.keyword_box->listbox1, arglist, ac);

	    /*
	    ** Clear the searchkeyword title listbox.
	    */
	    ac = 0 ;
	    XtSetArg (arglist[ac], XmNitems, NULL);
	    ac++;
	    XtSetArg (arglist[ac], XmNitemCount, topic_count);
	    ac++;
	    XtSetValues (help_widget->dxmhelp.keyword_box->listbox2, arglist, ac);
          /* Need to pre-def a selection of some sort for DefaultAction =kbm */
            XmListSelectPos (help_widget->dxmhelp.keyword_box->listbox2,
  (((XmListWidget )help_widget->dxmhelp.keyword_box->listbox2)->list.itemCount 
                > 1 ? 1 : 0), False);
            XmListSetPos (help_widget->dxmhelp.keyword_box->listbox2, 1); 

	    /*
	    ** Set the goto and visit buttons to insensitive.
	    */
	    XtSetSensitive (help_widget->dxmhelp.keyword_box->button1, FALSE);
	    XtSetSensitive (help_widget->dxmhelp.keyword_box->button2, FALSE);
	}
    }

}

static void update_label (w, label, dir)
    Widget	    w;
    XmString   label;
    int		    dir;
/*
**++
**  Functional Description:
**	Update the widget's label text string.
**
**  Keywords:
**	Help update
**
**  Arguments:
**	w: widget id.
**	label: label compound string.
**	type: label type.
**	dir: direction.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    Arg arglist[5];
    int argCount = 0;
 
   XtSetArg(arglist[argCount], XmNlabelString, label);
   argCount++;
   XtSetArg(arglist[argCount], XmNstringDirection, dir);  
   argCount++;
#ifdef RTOL
   XtSetArg(arglist[argCount], DXmNlayoutDirection, dir);  
   argCount++;
#endif /* RTOL */
 
   XtSetValues(w, arglist, argCount);
}

static void update_title (w, label, dir)
    Widget	    w;
    XmString   label;
    int		    dir;
/*
**++
**  Functional Description:
**	Update the widget's title text string.
**
**  Keywords:
**	Help update
**
**  Arguments:
**	w: widget id.
**	label: label compound string.
**	dir: direction.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
    Arg arglist[5];
    int argCount = 0;
 
   XtSetArg(arglist[argCount], XmNdialogTitle, label);
   argCount++;
   XtSetArg(arglist[argCount], XmNstringDirection, dir);  
   argCount++;
#ifdef RTOL
   XtSetArg(arglist[argCount], DXmNlayoutDirection, dir);  
   argCount++;
#endif /* RTOL */
 
   XtSetValues(w, arglist, argCount);
}

static Boolean SetValues (current, request, new)
    Widget	    current;
    Widget	    request;
    Widget	    new;
/*
**++
**  Functional Description:
**	Set the help widget's resources.
**
**  Keywords:
**	Help update.
**
**  Arguments:
**	current: specifies a copy of the widget as it was before the XtSetValues
**		    call.
**	request: specifies a copy of the widget with all values changed.
**	new: specifies the widget with the new values that are actually allowed.
**
**  Result:
**	Boolean indicating whether the widget needs to be redisplayed.
**
**  Exceptions:
**	None
**--
*/
{
    DXmHelpWidget old_widget = (DXmHelpWidget) current;
    DXmHelpWidget request_widget = (DXmHelpWidget) request;
    DXmHelpWidget new_widget = (DXmHelpWidget) new;
    Boolean need_layout = FALSE;
    Arg arglist[10];
    int ac = 0;
    Boolean comp = False;
    Boolean change_button = FALSE;
    Boolean change_buttons = FALSE;
    Boolean propagate = FALSE;			/* propagate to all widgets */
    Arg proplist[70];
    int propac = 0;                    
    Boolean propappl = FALSE;			/* propagate to application */
						/* visited widgets only	    */
    Arg applist[10];
    int appac = 0;
    int i, j;
    Widget w;
    int mwmStyle ;
    Boolean  temp_change_helpshell_coord = FALSE;/* Whether to diddle coord */
    Position saved_x, saved_y;                  /* Temporary storage for    */
                                                /* parent's x,y coord.      */

            
    /*
    ** Not allowed to change the library type
    */
    
    if (old_widget->dxmhelp.library_type != request_widget->dxmhelp.library_type)
	new_widget->dxmhelp.library_type = old_widget->dxmhelp.library_type;

    /*
    ** If the coordinates of the help widget are in the process of being changed
    ** temporarily set shell's x,y to where it will be when dust settles.
    ** This makes popup error msg (if any) come up in right place.
    ** These values must get restored before you exit from this routine.
    */
    if ( (old_widget->core.x != new_widget->core.x) || 
         (old_widget->core.y != new_widget->core.y)  )
	{
        saved_x = XtParent(old_widget)->core.x;
        saved_y = XtParent(old_widget)->core.y;
        XtParent(old_widget)->core.x = new_widget->core.x;
        XtParent(old_widget)->core.y = new_widget->core.y;
        new_widget->core.x = 0;
        new_widget->core.y = 0;
	temp_change_helpshell_coord = TRUE;
	}

    /*
    ** If new library, close the old one and open the new one and reset the help
    ** widget.
    */
    
    if (new_widget->dxmhelp.library == NULL)
	new_widget->dxmhelp.library_context = NULL;
	
    if ((new_widget->dxmhelp.library != NULL) &&
	(DXmStringCheck(&old_widget->dxmhelp.library,
		      &new_widget->dxmhelp.library)))
    {
	/*
	** Close the previous library.
	*/
	if (old_widget->dxmhelp.library_context != NULL)
	    DXmHelp__close_help (old_widget->dxmhelp.library_context);
	    
	/*
	** Open the new library.
	*/
	open_library(new_widget);
	    
	/*
	** Free the help widget's string storage.
	*/
	free_help (new_widget, TRUE);
	/*
	** Initialize the help widget.
	*/
	init_help (new_widget, TRUE);
	/*
	** Clear and reset the help widget and the dialog boxes.
	*/
	clear_help (new_widget, TRUE);
	/*
	** Cleanup the visited widgets.
	*/
        DXmHelpCleanup (new_widget->dxmhelp.visit_context);
	
	/*
	** Get the first topic help frame for the new library.
	*/
	if (request_widget->dxmhelp.first_topic != NULL)
	    if (!XmStringEmpty(request_widget->dxmhelp.first_topic))
		new_widget->dxmhelp.first_topic =
		    XmStringCopy (request_widget->dxmhelp.first_topic);
	insert_trail (new_widget, new_widget->dxmhelp.first_topic);
	get_help(new_widget);
    }
    else
    {

	/*
	**  Caching mode has changed
	*/

	if (old_widget->dxmhelp.cache_library !=
	    new_widget->dxmhelp.cache_library) {
	    
	    XtSetArg (proplist[propac], DXmNcacheHelpLibrary,
		new_widget->dxmhelp.cache_library); propac++;
	    propagate = TRUE;
	    DXmHelp__set_cache_mode(new_widget->dxmhelp.library_context,
		new_widget->dxmhelp.cache_library);
	}
    
    /*
    ** If new first topic, free and reinitialize part of the help record (but
    ** keep the history list, the title and keyword box and the visited widgets
    ** list as they are). Get the new help frame.
    */
	if ((request_widget->dxmhelp.first_topic != NULL) &&
	    (old_widget->dxmhelp.trail != NULL) &&
	    (old_widget->dxmhelp.trail->topic != NULL))
        {
            /* 
             *  Do a string comparison of topic.  If same then we
             *  don't incur the overhead.  Otherwise, we do. == kbm ==  
             */
            comp = XmStringCompare(old_widget->dxmhelp.trail->topic,
                            request_widget->dxmhelp.first_topic);
        }
        if (comp == False)   /* False is the default */
	{
	    /*
	    ** Free part of the help widget's record.
	    */
	    free_help (new_widget, FALSE);
	    /*
	    ** Initialize the same part.
	    */
	    init_help (new_widget, FALSE);
	    /*
	    ** Clear the help widget only.
	    */
	    clear_help (new_widget, FALSE);
	    /*
	    ** Get the new frame.
	    */
	    if (request_widget->dxmhelp.first_topic != NULL)
		{
		new_widget->dxmhelp.first_topic =
			XmStringCopy (request_widget->dxmhelp.first_topic);
		}
	    insert_trail (new_widget, new_widget->dxmhelp.first_topic);
	    get_help(new_widget);
	}
    }    
    /*
    ** If new overview or glossary frame, check the menu items sensitivity
    ** and propagate to the application visited widgets.
    */
    
    if (old_widget->dxmhelp.overview !=	request_widget->dxmhelp.overview)
    {
	if (old_widget->dxmhelp.overview == NULL)
	    XtSetSensitive(new_widget->dxmhelp.overview_button , TRUE);
	if (request_widget->dxmhelp.overview == NULL)
	    XtSetSensitive(new_widget->dxmhelp.overview_button , FALSE);
	else
	    new_widget->dxmhelp.overview =
		    XmStringCopy (request_widget->dxmhelp.overview);
		    
	XtSetArg(applist[appac], DXmNoverviewTopic,
		    new_widget->dxmhelp.overview);
	appac++;	
	propappl = TRUE;
    }
    if (old_widget->dxmhelp.glossary != request_widget->dxmhelp.glossary)
    {
	/* Check to make sure that the Glossary_button is actually valid */
	
	if (new_widget->dxmhelp.glossary_button != NULL)
	    {
	    if (old_widget->dxmhelp.glossary == NULL)
	        XtSetSensitive(new_widget->dxmhelp.glossary_button , TRUE);
	    if (request_widget->dxmhelp.glossary == NULL)
	        XtSetSensitive(new_widget->dxmhelp.glossary_button , FALSE);
	    else
	        new_widget->dxmhelp.glossary =
        	    XmStringCopy (request_widget->dxmhelp.glossary);
	    }
	
	XtSetArg(applist[appac], DXmNglossaryTopic,
		    new_widget->dxmhelp.glossary);
	appac++;	
	propappl = TRUE;
    }

    /*
    ** Check application name and type and set the helpshell title.
    */
    if ((old_widget->dxmhelp.application != new_widget->dxmhelp.application) ||
        (old_widget->dxmhelp.string_direction != new_widget->dxmhelp.string_direction) ||
#ifdef RTOL
        (LayoutDirection(old_widget) != LayoutDirection(new_widget) ) ||
#endif /* RTOL */
	(old_widget->dxmhelp.helpontitle_label != new_widget->dxmhelp.helpontitle_label) ||
	(old_widget->dxmhelp.helptitle_label != new_widget->dxmhelp.helptitle_label))	
    {
	/*
	** Set the helpshell title.
	*/
	set_helpshell_title (request_widget, new_widget);
	/*
	** Set the argument lists for propagation.
	*/
	XtSetArg(applist[appac], DXmNapplicationName,
		new_widget->dxmhelp.application);
	appac++;
	propappl = TRUE;
	XtSetArg(proplist[propac], DXmNhelpontitleLabel,
		new_widget->dxmhelp.helpontitle_label);
	propac++;	
	XtSetArg(proplist[propac], DXmNhelptitleLabel,
		new_widget->dxmhelp.helptitle_label);
	propac++;	
	XtSetArg(proplist[propac], XmNstringDirection,
		new_widget->dxmhelp.string_direction);
	propac++;	
#ifdef RTOL
	XtSetArg(proplist[propac], DXmNlayoutDirection,
		LayoutDirection(new_widget));
	propac++;	
#endif /* RTOL */
	propagate = TRUE;
    }    

    /*
    ** Not allowed to change the size in pixels
    */
    if (XtWidth(old_widget) != XtWidth(new_widget))
	new_widget->core.width = XtWidth (old_widget);
    if (XtHeight(old_widget) != XtHeight(new_widget))
	new_widget->core.height = XtHeight (old_widget);

    /*
    ** If new number of colons or rows recompute the size of the widget
    */
    if ((old_widget->dxmhelp.colons != request_widget->dxmhelp.colons) ||
        (old_widget->dxmhelp.rows != request_widget->dxmhelp.rows))
    {
	XtSetArg (arglist[0], DXmNcols, request_widget->dxmhelp.colons + 4);
	XtSetArg (arglist[1], DXmNrows, request_widget->dxmhelp.rows + 1);
	XtSetValues (new_widget->dxmhelp.help_text, arglist, 2);
	XtSetArg(proplist[propac], DXmNcols, new_widget->dxmhelp.colons);
	propac++;	
	XtSetArg(proplist[propac], DXmNrows, new_widget->dxmhelp.rows);
	propac++;	
	propagate = TRUE;
    }

/*  ?? Handle buttonFont, labelFont and textFont ?? */
/*     Look at bulletin board SetValues routine as an example  */
#ifdef HANDLE_THIS_LATER

    /*
    ** Update the help font
    */
    if (old_widget->dxmhelp.button_font_list != request_widget->dxmhelp.button_font_list)
    {
	/*
	** Call the toolkit propagation routine.
	*/
	
	XtSetArg (arglist[0], XmNfontList,
	          request_widget->dxmhelp.button_font_list);
	
	for (i=0; i < XtNumChildren(new_widget); i++)
	    XtSetValues(new_widget->composite.children[i], arglist, 1);
	for (i=0; i < new_widget->core.num_popups ; i++)
	    for (j = 0; j < XtNumChildren(new_widget->core.popup_list[i]); j++)
	    {
		w = (Widget) new_widget->core.popup_list[i];
/*		XtSetValues
		    (w->composite.children[j], arglist, 1);*/
	    }
		

	/*
	** Set the visited help widget's propagation argument list.
	*/
	XtSetArg(proplist[propac], XmNfontList,
		    new_widget->dxmhelp.button_font_list); propac++;	
	propagate = TRUE;
    }
    if (old_widget->dxmhelp.help_fontlist != request_widget->dxmhelp.help_fontlist)
    {
	XtSetArg (arglist[0], XmNfontList,
	          request_widget->dxmhelp.help_fontlist);
	XtSetValues (new_widget->dxmhelp.help_text, arglist, 1);
	XtSetValues (new_widget->dxmhelp.help_topic, arglist, 1);
	XtSetArg(proplist[propac], DXmNhelpFont,
		    new_widget->dxmhelp.help_fontlist); propac++;	
	propagate = TRUE;
    }
#endif

    /*
    ** Check the direction argument.
    */
#ifdef RTOL
    if (LayoutDirection(old_widget) != LayoutDirection(request_widget))
#else
    if (old_widget->dxmhelp.string_direction != request_widget->dxmhelp.string_direction)
#endif
    {
	/*
	** Call the toolkit propagation routine.
	*/
	
#ifdef RTOL
	XtSetArg (arglist[0], DXmNlayoutDirection,
	          LayoutDirection(request_widget));
#else
	XtSetArg (arglist[0], XmNstringDirection,
	          request_widget->dxmhelp.string_direction);
#endif
	
	for (i=0; i < XtNumChildren(new_widget); i++)
	    XtSetValues(new_widget->composite.children[i], arglist, 1);
	for (i=0; i < new_widget->core.num_popups ; i++)
	    for (j = 0; j < XtNumChildren(new_widget->core.popup_list[i]); j++)
	    {
		w = (Widget) new_widget->core.popup_list[i];
/*		XtSetValues
		    (w->composite.children[j], arglist, 1);*/
	    }
		

	/*
	** Set the visited help widget's propagation argument list.
	*/
#ifdef RTOL
	XtSetArg (arglist[propac], DXmNlayoutDirection,
	            LayoutDirection(new_widget)); propac++;
#else
	XtSetArg(proplist[propac], XmNstringDirection,
		    new_widget->dxmhelp.string_direction); propac++;	
#endif
	propagate = TRUE;
    }

    /*
    ** Check labels and reset them if necessary.
    */
    /*
    ** View menu labels.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.view_menu_label,
			 &new_widget->dxmhelp.view_menu_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.view_menu,
			new_widget->dxmhelp.view_menu_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.view_menu,
			new_widget->dxmhelp.view_menu_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg(proplist[propac], DXmNviewLabel,
		new_widget->dxmhelp.view_menu_label); propac++;	
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.goto_topic_label,
			 &new_widget->dxmhelp.goto_topic_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.goto_button,
			new_widget->dxmhelp.goto_topic_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.goto_button,
			new_widget->dxmhelp.goto_topic_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNgototopicLabel,
		new_widget->dxmhelp.goto_topic_label); propac++;
	propagate = TRUE;
    }				    
    if (DXmStringCheck (&old_widget->dxmhelp.goback_topic_label,
			 &new_widget->dxmhelp.goback_topic_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.goback_button,
			new_widget->dxmhelp.goback_topic_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.goback_button,
			new_widget->dxmhelp.goback_topic_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNgobacktopicLabel,
		new_widget->dxmhelp.goback_topic_label); propac++;
	propagate = TRUE;
    }			    
	
    if (DXmStringCheck (&old_widget->dxmhelp.goover_label,
			 &new_widget->dxmhelp.goover_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.overview_button,
			new_widget->dxmhelp.goover_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.overview_button,
			new_widget->dxmhelp.goover_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNgooverLabel,
		new_widget->dxmhelp.goover_label); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.visit_topic_label,
			 &new_widget->dxmhelp.visit_topic_label))
    {
#ifdef RTOL
 	update_label (new_widget->dxmhelp.visit_button,
			new_widget->dxmhelp.visit_topic_label,
			LayoutDirection(new_widget));
#else
 	update_label (new_widget->dxmhelp.visit_button,
			new_widget->dxmhelp.visit_topic_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNvisittopicLabel,
		new_widget->dxmhelp.visit_topic_label); propac++;
	propagate = TRUE;
    }			    
			    
    if (DXmStringCheck (&old_widget->dxmhelp.visitglos_label,
			 &new_widget->dxmhelp.visitglos_label))
    {
	if (new_widget->dxmhelp.glossary_button != NULL)
#ifdef RTOL
	    update_label (new_widget->dxmhelp.glossary_button,
			    new_widget->dxmhelp.visitglos_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.glossary_button,
			    new_widget->dxmhelp.visitglos_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNvisitglosLabel,
		new_widget->dxmhelp.visitglos_label); propac++;
	propagate = TRUE;
    }			    
    /*
    ** File menu labels.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.file_menu_label,
			 &new_widget->dxmhelp.file_menu_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.file_menu,
			new_widget->dxmhelp.file_menu_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.file_menu,
			new_widget->dxmhelp.file_menu_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNfileLabel,
		new_widget->dxmhelp.file_menu_label); propac++;
	propagate = TRUE;
    }			    

    if (DXmStringCheck (&old_widget->dxmhelp.saveas_label,
			 &new_widget->dxmhelp.saveas_label))
    {
#ifdef RTOL
 	update_label (new_widget->dxmhelp.saveas_button,
			new_widget->dxmhelp.saveas_label,
			LayoutDirection(new_widget));
#else
 	update_label (new_widget->dxmhelp.saveas_button,
			new_widget->dxmhelp.saveas_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNsaveasLabel,
		new_widget->dxmhelp.saveas_label); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.exit_label,
			 &new_widget->dxmhelp.exit_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.exit_button,
			new_widget->dxmhelp.exit_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.exit_button,
			new_widget->dxmhelp.exit_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNexitLabel,
		new_widget->dxmhelp.exit_label); propac++;
	propagate = TRUE;
    }			    
    /*
    ** Edit menu labels.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.edit_menu_label,
			 &new_widget->dxmhelp.edit_menu_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.edit_menu,
			new_widget->dxmhelp.edit_menu_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.edit_menu,
			new_widget->dxmhelp.edit_menu_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNeditLabel,
		new_widget->dxmhelp.edit_menu_label); propac++;
	propagate = TRUE;
    }			    

    if (DXmStringCheck (&old_widget->dxmhelp.copy_label,
			 &new_widget->dxmhelp.copy_label))
    {
#ifdef RTOL
  	update_label (new_widget->dxmhelp.copy_button,
			new_widget->dxmhelp.copy_label,
			LayoutDirection(new_widget));
#else
  	update_label (new_widget->dxmhelp.copy_button,
			new_widget->dxmhelp.copy_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNcopyLabel,
		new_widget->dxmhelp.copy_label); propac++;
	propagate = TRUE;
    }			    

    if (DXmStringCheck (&old_widget->dxmhelp.selectall_label,
			 &new_widget->dxmhelp.selectall_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.selectall_button,
			new_widget->dxmhelp.selectall_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.selectall_button,
			new_widget->dxmhelp.selectall_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNselectallLabel,
		new_widget->dxmhelp.selectall_label); propac++;
	propagate = TRUE;
    }			    
			    

    /*
    ** Search menu labels.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.search_menu_label,
			 &new_widget->dxmhelp.search_menu_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.search_menu,
			new_widget->dxmhelp.search_menu_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.search_menu,
			new_widget->dxmhelp.search_menu_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNsearchLabel,
		new_widget->dxmhelp.search_menu_label); propac++;
	propagate = TRUE;
    }			    
    
    if (DXmStringCheck (&old_widget->dxmhelp.history_label,
			 &new_widget->dxmhelp.history_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.history_button,
			new_widget->dxmhelp.history_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.history_button,
			new_widget->dxmhelp.history_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNhistoryLabel,
		new_widget->dxmhelp.history_label); propac++;
	propagate = TRUE;
    }			    
			    
    if (DXmStringCheck (&old_widget->dxmhelp.title_label,
			 &new_widget->dxmhelp.title_label))
    {
#ifdef RTOL
  	update_label (new_widget->dxmhelp.title_button,
			new_widget->dxmhelp.title_label,
			LayoutDirection(new_widget));
#else
  	update_label (new_widget->dxmhelp.title_button,
			new_widget->dxmhelp.title_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
 	XtSetArg (proplist[propac], DXmNtitleLabel,
		new_widget->dxmhelp.title_label); propac++;
	propagate = TRUE;
    }			    

    if (DXmStringCheck (&old_widget->dxmhelp.keyword_label,
			 &new_widget->dxmhelp.keyword_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.keyword_button,
			new_widget->dxmhelp.keyword_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.keyword_button,
			new_widget->dxmhelp.keyword_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNkeywordLabel,
		new_widget->dxmhelp.keyword_label); propac++;
	propagate = TRUE;
    }			    

    /*
    ** Help on help menu labels.
    */
    if (new_widget->dxmhelp.help_menu != NULL)
    {
	if (DXmStringCheck (&old_widget->dxmhelp.help_menu_label,
			     &new_widget->dxmhelp.help_menu_label))
	{
#ifdef RTOL
	    update_label (new_widget->dxmhelp.help_menu,
			    new_widget->dxmhelp.help_menu_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.help_menu,
			    new_widget->dxmhelp.help_menu_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	    XtSetArg (proplist[propac], DXmNhelpLabel,
		    new_widget->dxmhelp.help_menu_label); propac++;
	    propagate = TRUE;
	}			    
				
	if (DXmStringCheck (&old_widget->dxmhelp.helphelp_label,
			     &new_widget->dxmhelp.helphelp_label))
	{
#ifdef RTOL
	    update_label (new_widget->dxmhelp.help_button,
			    new_widget->dxmhelp.helphelp_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.help_button,
			    new_widget->dxmhelp.helphelp_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	    XtSetArg (proplist[propac], DXmNhelphelpLabel,
		    new_widget->dxmhelp.helphelp_label); propac++;
	    propagate = TRUE;
	}			    
				
#if 0	/* Remove everything except On Windows from Using Help Menu */
	if (DXmStringCheck (&old_widget->dxmhelp.glossary_label,
			     &new_widget->dxmhelp.glossary_label))
	{
#ifdef RTOL
	    update_label (new_widget->dxmhelp.helpglossary_button,
			    new_widget->dxmhelp.glossary_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.helpglossary_button,
			    new_widget->dxmhelp.glossary_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	    XtSetArg (proplist[propac], DXmNglossaryLabel,
		    new_widget->dxmhelp.glossary_label); propac++;
	    propagate = TRUE;
	}			    
#endif 	/* Remove everything except On Windows from Using Help Menu */

				
#if 0	/* Remove everything except On Windows from Using Help Menu */
	if (DXmStringCheck (&old_widget->dxmhelp.about_label,
			     &new_widget->dxmhelp.about_label))
        {
#ifdef RTOL
	    update_label (new_widget->dxmhelp.about_button,
			    new_widget->dxmhelp.about_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.about_button,
			    new_widget->dxmhelp.about_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	    XtSetArg (proplist[propac], DXmNaboutLabel,
		    new_widget->dxmhelp.about_label); propac++;
	    propagate = TRUE;
	}			    
#endif 	/* Remove everything except On Windows from Using Help Menu */


#if 0	/* Remove everything except On Windows from Using Help Menu */
	if (DXmStringCheck (&old_widget->dxmhelp.oncontext_label,
			     &new_widget->dxmhelp.oncontext_label))
        {
#ifdef RTOL
	    update_label (new_widget->dxmhelp.oncontext_button,
			    new_widget->dxmhelp.oncontext_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.oncontext_button,
			    new_widget->dxmhelp.oncontext_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	    XtSetArg (proplist[propac], DXmNoncontextLabel,
		    new_widget->dxmhelp.oncontext_label); propac++;
	    propagate = TRUE;
	}			    
#endif 	/* Remove everything except On Windows from Using Help Menu */

    }
    
    /*
    ** Additional topics label inside the help widget.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.addtopic_label,
		       &new_widget->dxmhelp.addtopic_label))
    {
#ifdef RTOL
	update_label (new_widget->dxmhelp.add_topic,
			new_widget->dxmhelp.addtopic_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.add_topic,
			new_widget->dxmhelp.addtopic_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNaddtopicLabel,
		new_widget->dxmhelp.addtopic_label); propac++;
	propagate = TRUE;
    }			    

    /*
    ** Goback and quit buttons labels inside the help widget.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.goback_label,
			 &new_widget->dxmhelp.goback_label))
    {
	XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	XtSetValues (new_widget->dxmhelp.goback, arglist, 1);
#ifdef RTOL
	update_label (new_widget->dxmhelp.goback,
			new_widget->dxmhelp.goback_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.goback,
			new_widget->dxmhelp.goback_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	change_button = TRUE;
	XtSetArg (proplist[propac], DXmNgobackLabel,
		new_widget->dxmhelp.goback_label); propac++;
	propagate = TRUE;
    }    
    if (DXmStringCheck (&old_widget->dxmhelp.quit_label,
			 &new_widget->dxmhelp.quit_label))
    {
	XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	XtSetValues (new_widget->dxmhelp.quit, arglist, 1);
#ifdef RTOL
	update_label (new_widget->dxmhelp.quit,
			new_widget->dxmhelp.quit_label,
			LayoutDirection(new_widget));
#else
	update_label (new_widget->dxmhelp.quit,
			new_widget->dxmhelp.quit_label,
			new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	change_button = TRUE;
	XtSetArg (proplist[propac], DXmNcloseLabel,
		new_widget->dxmhelp.quit_label); propac++;
	propagate = TRUE;
    }			    

    /*
    ** Goto, visit, apply and dismiss buttons for the search boxes.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.goto_label,
			 &new_widget->dxmhelp.goto_label))
    {
	if (new_widget->dxmhelp.history_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.history_box->button1, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.history_box->button1,
			    new_widget->dxmhelp.goto_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.history_box->button1,
			    new_widget->dxmhelp.goto_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	if (new_widget->dxmhelp.title_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.title_box->button1, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.title_box->button1,
			    new_widget->dxmhelp.goto_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.title_box->button1,
			    new_widget->dxmhelp.goto_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	if (new_widget->dxmhelp.keyword_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.keyword_box->button1, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.keyword_box->button1,
			    new_widget->dxmhelp.goto_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.keyword_box->button1,
			    new_widget->dxmhelp.goto_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	change_buttons = TRUE;
	XtSetArg (proplist[propac], DXmNgotoLabel,
		new_widget->dxmhelp.goto_label); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.visit_label,
			 &new_widget->dxmhelp.visit_label))
    {
	if (new_widget->dxmhelp.history_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.history_box->button2, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.history_box->button2,
			    new_widget->dxmhelp.visit_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.history_box->button2,
			    new_widget->dxmhelp.visit_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	if (new_widget->dxmhelp.title_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.title_box->button2, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.title_box->button2,
			    new_widget->dxmhelp.visit_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.title_box->button2,
			    new_widget->dxmhelp.visit_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	if (new_widget->dxmhelp.keyword_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.keyword_box->button2, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.keyword_box->button2,
			    new_widget->dxmhelp.visit_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.keyword_box->button2,
			    new_widget->dxmhelp.visit_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	change_buttons = TRUE;
	XtSetArg (proplist[propac], DXmNvisitLabel,
		new_widget->dxmhelp.visit_label); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.apply_label,
			 &new_widget->dxmhelp.apply_label))
    {
	if (new_widget->dxmhelp.title_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.title_box->button0, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.title_box->button0,
			    new_widget->dxmhelp.apply_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.title_box->button0,
			    new_widget->dxmhelp.apply_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	if (new_widget->dxmhelp.keyword_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.keyword_box->button0, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.keyword_box->button0,
			    new_widget->dxmhelp.apply_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.keyword_box->button0,
			    new_widget->dxmhelp.apply_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	change_buttons = TRUE;
	XtSetArg (proplist[propac], DXmNsearchapplyLabel,
		new_widget->dxmhelp.apply_label); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.dismiss_label,
			 &new_widget->dxmhelp.dismiss_label))
    {
	if (new_widget->dxmhelp.history_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.history_box->button3, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.history_box->button3,
			    new_widget->dxmhelp.dismiss_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.history_box->button3,
			    new_widget->dxmhelp.dismiss_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	if (new_widget->dxmhelp.title_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.title_box->button3, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.title_box->button3,
			    new_widget->dxmhelp.dismiss_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.title_box->button3,
			    new_widget->dxmhelp.dismiss_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	if (new_widget->dxmhelp.keyword_box != NULL)
	{
	    XtSetArg (arglist[0], XmNrecomputeSize, TRUE);
	    XtSetValues (new_widget->dxmhelp.keyword_box->button3, arglist, 1);
#ifdef RTOL
	    update_label (new_widget->dxmhelp.keyword_box->button3,
			    new_widget->dxmhelp.dismiss_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.keyword_box->button3,
			    new_widget->dxmhelp.dismiss_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	}
	change_buttons = TRUE;
	XtSetArg (proplist[propac], DXmNdismissLabel,
		new_widget->dxmhelp.dismiss_label); propac++;
	propagate = TRUE;
    }			    

    /*
    ** Search boxes label.
    */	
    if (DXmStringCheck (&old_widget->dxmhelp.topic_titles_label,
			 &new_widget->dxmhelp.topic_titles_label))
    {
	if (new_widget->dxmhelp.history_box != NULL)
#ifdef RTOL
	    update_label (new_widget->dxmhelp.history_box->label1,
			    new_widget->dxmhelp.topic_titles_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.history_box->label1,
			    new_widget->dxmhelp.topic_titles_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	if (new_widget->dxmhelp.title_box != NULL)
#ifdef RTOL
	    update_label (new_widget->dxmhelp.title_box->label2,
			    new_widget->dxmhelp.topic_titles_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.title_box->label2,
			    new_widget->dxmhelp.topic_titles_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	if (new_widget->dxmhelp.keyword_box != NULL)
#ifdef RTOL
	    update_label (new_widget->dxmhelp.keyword_box->label2,
			    new_widget->dxmhelp.topic_titles_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.keyword_box->label2,
			    new_widget->dxmhelp.topic_titles_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNtopictitlesLabel,
		new_widget->dxmhelp.topic_titles_label); propac++;
	propagate = TRUE;
    }			    

    /*
    ** History box label.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.history_box_label,
			 &new_widget->dxmhelp.history_box_label))
    {
	if (new_widget->dxmhelp.history_box != NULL)
	    update_title (new_widget->dxmhelp.history_box->box,
			    new_widget->dxmhelp.history_box_label,
			    new_widget->dxmhelp.string_direction);

	XtSetArg (proplist[propac], DXmNhistoryboxLabel,
		new_widget->dxmhelp.history_box_label); propac++;
	propagate = TRUE;
    }			    
    /*
    ** Search title box labels.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.searchtitle_box_label,
			 &new_widget->dxmhelp.searchtitle_box_label))
    {
	if (new_widget->dxmhelp.title_box != NULL)
	    update_title (new_widget->dxmhelp.title_box->box,
			    new_widget->dxmhelp.searchtitle_box_label,
			    new_widget->dxmhelp.string_direction);
	XtSetArg (proplist[propac], DXmNsearchtitleboxLabel,
		new_widget->dxmhelp.searchtitle_box_label); propac++;
	propagate = TRUE;
    }			    

    if (DXmStringCheck (&old_widget->dxmhelp.titles_label,
			 &new_widget->dxmhelp.titles_label))
    {
	if (new_widget->dxmhelp.title_box != NULL)
#ifdef RTOL
	    update_label (new_widget->dxmhelp.title_box->label1,
			    new_widget->dxmhelp.titles_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.title_box->label1,
			    new_widget->dxmhelp.titles_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNtitlesLabel,
		new_widget->dxmhelp.titles_label); propac++;
	propagate = TRUE;
    }			    

    /*
    ** Search keyword box labels.
    */
    if (DXmStringCheck (&old_widget->dxmhelp.searchkeyword_box_label,
			 &new_widget->dxmhelp.searchkeyword_box_label))
    {
	if (new_widget->dxmhelp.keyword_box != NULL)
	    update_title (new_widget->dxmhelp.keyword_box->box,
			    new_widget->dxmhelp.searchkeyword_box_label,
			    new_widget->dxmhelp.string_direction);
	XtSetArg (proplist[propac], DXmNsearchkeywordboxLabel,
		new_widget->dxmhelp.searchkeyword_box_label); propac++;
	propagate = TRUE;
    }			    
			    
    if (DXmStringCheck (&old_widget->dxmhelp.keywords_label,
			 &new_widget->dxmhelp.keywords_label))
    {
	if (new_widget->dxmhelp.keyword_box != NULL)
#ifdef RTOL
	    update_label (new_widget->dxmhelp.keyword_box->label1,
			    new_widget->dxmhelp.keywords_label,
			    LayoutDirection(new_widget));
#else
	    update_label (new_widget->dxmhelp.keyword_box->label1,
			    new_widget->dxmhelp.keywords_label,
			    new_widget->dxmhelp.string_direction);
#endif /* RTOL */
	XtSetArg (proplist[propac], DXmNkeywordsLabel,
		new_widget->dxmhelp.keywords_label); propac++;
	propagate = TRUE;
    }			    

    /*
    ** Message box labels.
    */

    if (DXmStringCheck (&old_widget->dxmhelp.badlib_message,
			 &new_widget->dxmhelp.badlib_message))
    {
	XtSetArg (proplist[propac], DXmNbadlibMessage,
		new_widget->dxmhelp.badlib_message); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.badframe_message,
			 &new_widget->dxmhelp.badframe_message))
    {
	XtSetArg (proplist[propac], DXmNbadframeMessage,
		new_widget->dxmhelp.badframe_message); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.nulllib_message,
			 &new_widget->dxmhelp.nulllib_message))
    {
	XtSetArg (proplist[propac], DXmNnulllibMessage,
		new_widget->dxmhelp.nulllib_message); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.notitle_message,
			 &new_widget->dxmhelp.notitle_message))
    {
	XtSetArg (proplist[propac], DXmNnotitleMessage,
		new_widget->dxmhelp.notitle_message); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.nokeyword_message,
			 &new_widget->dxmhelp.nokeyword_message))
    {
	XtSetArg (proplist[propac], DXmNnokeywordMessage,
		new_widget->dxmhelp.nokeyword_message); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.erroropen_message,
			 &new_widget->dxmhelp.erroropen_message))
    {
	XtSetArg (proplist[propac], DXmNerroropenMessage,
		new_widget->dxmhelp.erroropen_message); propac++;
	propagate = TRUE;
    }			    
    if (DXmStringCheck (&old_widget->dxmhelp.acknowledge_label,
			 &new_widget->dxmhelp.acknowledge_label))
    {
	XtSetArg (proplist[propac], DXmNhelpAcknowledgeLabel,
		new_widget->dxmhelp.acknowledge_label); propac++;
	propagate = TRUE;
    }			    

    /*
    **  Help on help title
    */
    if (DXmStringCheck (&old_widget->dxmhelp.helponhelp_title,
			 &new_widget->dxmhelp.helponhelp_title))
    {
	XtSetArg (proplist[propac], DXmNhelpOnHelpTitle,
		new_widget->dxmhelp.helponhelp_title); propac++;
	propagate = TRUE;
    }			    

    /*
    ** Propagate to the visited widgets only (not help on help).
    */
    if (propappl)
	HelpPropagateArgs(new_widget->dxmhelp.visit_context, applist, appac);
    /*
    ** Propagate the changes to the visited widgets, the help on help widget and
    ** the help on saveas widget.
    */
    if (propagate)
    {
	HelpPropagateArgs(new_widget->dxmhelp.visit_context, proplist, propac);
	if (new_widget->dxmhelp.help_on_help != NULL)
	    XtSetValues (new_widget->dxmhelp.help_on_help, proplist, propac);
	if (new_widget->dxmhelp.help_on_saveas != NULL)
	    XtSetValues (new_widget->dxmhelp.help_on_saveas, proplist, propac);
    }
    /*
    ** Reset the buttons position and width if any change was made.
    */
    if (change_button)
    {
	int button_width = 0;
	/*
	** Compute the buttons' width and set it.
	*/
	button_width = MAX(button_width, XtWidth(new_widget->dxmhelp.goback));
	button_width = MAX(button_width, XtWidth(new_widget->dxmhelp.quit));
	ac = 0;
	XtSetArg (arglist[ac], XmNrecomputeSize, FALSE);
	ac++;
	XtSetArg (arglist[ac], XtNwidth, button_width);
	ac++;
	XtSetValues (new_widget->dxmhelp.goback, arglist, ac);
	XtSetValues (new_widget->dxmhelp.quit, arglist, ac);
    }
    if (change_buttons)
    {
	/*
	** Compute the buttons' width and set it.
	*/
	if (new_widget->dxmhelp.history_box != NULL)
	{
	    int button_width = 0;
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.history_box->button1));
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.history_box->button2));
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.history_box->button3));
	    ac = 0;
	    XtSetArg (arglist[ac], XmNrecomputeSize, FALSE);
	    ac++;
	    XtSetArg (arglist[ac], XtNwidth, button_width);
	    ac++;
	    XtSetValues (new_widget->dxmhelp.history_box->button1, arglist, ac);
	    XtSetValues (new_widget->dxmhelp.history_box->button2, arglist, ac);
	    XtSetValues (new_widget->dxmhelp.history_box->button3, arglist, ac);
	}
	if (new_widget->dxmhelp.title_box != NULL)
	{
	    int button_width = 0;
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.title_box->button0));
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.title_box->button1));
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.title_box->button2));
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.title_box->button3));
	    ac = 0;
	    XtSetArg (arglist[ac], XmNrecomputeSize, FALSE);
	    ac++;
	    XtSetArg (arglist[ac], XtNwidth, button_width);
	    ac++;
	    XtSetValues (new_widget->dxmhelp.title_box->button0, arglist, ac);
	    XtSetValues (new_widget->dxmhelp.title_box->button1, arglist, ac);
	    XtSetValues (new_widget->dxmhelp.title_box->button2, arglist, ac);
	    XtSetValues (new_widget->dxmhelp.title_box->button3, arglist, ac);
	}
	if (new_widget->dxmhelp.keyword_box != NULL)
	{
	    int button_width = 0;
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.keyword_box->button0));
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.keyword_box->button1));
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.keyword_box->button2));
	    button_width = MAX(button_width,
			    XtWidth(new_widget->dxmhelp.keyword_box->button3));
	    ac = 0;
	    XtSetArg (arglist[ac], XmNrecomputeSize, FALSE);
	    ac++;
	    XtSetArg (arglist[ac], XtNwidth, button_width);
	    ac++;
	    XtSetValues (new_widget->dxmhelp.keyword_box->button0, arglist, ac);
	    XtSetValues (new_widget->dxmhelp.keyword_box->button1, arglist, ac);
	    XtSetValues (new_widget->dxmhelp.keyword_box->button2, arglist, ac);
	    XtSetValues (new_widget->dxmhelp.keyword_box->button3, arglist, ac);
	}
    }

#ifdef DIALOG_STYLE_SUPPORT
    /*  Validate dialog style.
    */
    if ( !new_widget->dxmhelp.shell &&
         request_widget->dxmhelp.dialog_style != XmDIALOG_WORK_AREA    )
	{
        _XmWarning( (Widget) new, WARN_DIALOG_STYLE) ;
        new_widget->dxmhelp.dialog_style = old_widget->dxmhelp.dialog_style ;
        }
    else
        {   if (new_widget->dxmhelp.shell
                && (request_widget->dxmhelp.dialog_style != XmDIALOG_MODELESS)
                && (request_widget->dxmhelp.dialog_style != XmDIALOG_SYSTEM_MODAL)
                && (request_widget->dxmhelp.dialog_style
                                         != XmDIALOG_PRIMARY_APPLICATION_MODAL)
                && (request_widget->dxmhelp.dialog_style
                                       != XmDIALOG_FULL_APPLICATION_MODAL)    )
            {   _XmWarning( (Widget) new_widget, WARN_DIALOG_STYLE) ;
                new_widget->dxmhelp.dialog_style
                                       = old_widget->dxmhelp.dialog_style ;
            }
        }

    if ( new_widget->dxmhelp.dialog_style !=
         old_widget->dxmhelp.dialog_style    )
        {   switch(    request_widget->dxmhelp.dialog_style    )
            {
            case XmDIALOG_PRIMARY_APPLICATION_MODAL:
                {   mwmStyle = MWM_INPUT_PRIMARY_APPLICATION_MODAL ;
                    break ;
                }
            case XmDIALOG_FULL_APPLICATION_MODAL:
                {   mwmStyle = MWM_INPUT_FULL_APPLICATION_MODAL ;
                    break ;
                }
            case XmDIALOG_SYSTEM_MODAL:
                {   mwmStyle = MWM_INPUT_SYSTEM_MODAL ;
                break ;
                }
            case XmDIALOG_MODELESS:
            default:
                {   mwmStyle = MWM_INPUT_MODELESS ;
                    break ;
                }
            }
        XtSetArg( arglist[0], XmNmwmInputMode, mwmStyle) ;
        XtSetValues( XtParent (new_widget), arglist, (Cardinal) 1 ) ;
        }
#endif  /*DIALOG_STYLE_SUPPORT*/

     /*
     ** Restore saved coord values that we diddled at the beginning of the
     ** routine.
     */
     if(temp_change_helpshell_coord)
	{
        new_widget->core.x = XtParent(old_widget)->core.x;
        new_widget->core.y = XtParent(old_widget)->core.y;
        XtParent(old_widget)->core.x = saved_x;
        XtParent(old_widget)->core.y = saved_y;
	}
    return (need_layout);
}


static XtGeometryResult geometry_manager (w, request, reply)
    Widget		w;
    XtWidgetGeometry	*request;
    XtWidgetGeometry	*reply;
/*
**++
**  Functional Description:
**	Manage the children's geometry.
**
**  Keywords:
**	Composite widget, Geometry management.
**
**  Arguments:
**	w: child widget id.
**	request: pointer to the record containing requested geometry values.
**	reply: pointer to the record containing the allowed geometry values.
**
**  Result:
**	XtGeometryResult: allow, disallow or suggest a compromise.
**
**  Exceptions:
**	None
**--
*/
{
    /*
    ** Get the help widget id.
    */
    DXmHelpWidget help_widget = (DXmHelpWidget) XtParent(w);

    /*
    ** If the child is the menubar.
    */
    if (XmIsBulletinBoard(w))
    {
	/*
	** If wants to change width, compute the new menubar width.
	*/
	if ((request->request_mode & CWWidth) != 0)
	{
	    int a = XtWidth(help_widget) - (XtBorderWidth(w)<<1);

	    /*
	    ** If the requested width is different from the new one, set the
	    ** return record with the new one and return geometryalmost.
	    */ 
	    if (request->width != a)
	    {
		*reply = *request;
		reply->width = a;
		return(XtGeometryAlmost);
	    }
	}

	/*
	** Change the child's window geometry.
	*/
	_XmConfigureObject  ( (Widget) w,
			      request->x, request->y,
			      request->width, request->height,
			      request->border_width);

	/*
	** Relayout the help_widget.
	*/
	Layout(help_widget, w, TRUE);
	/*
	** Return geometryyes.
	*/
	return(XtGeometryDone);
    }
    else
    {
	/*
	** Change the child's window geometry.
	*/
	_XmConfigureObject  ( (Widget) w,
			      request->x, request->y,
			      request->width, request->height,
			      request->border_width);
	/*
	** Relayout the help_widget.
	*/
	Layout(help_widget, w, TRUE);
	/*
	** Return geometryyes.
	*/
	return (XtGeometryDone) ;
    }

} 


static void set_changed (w)
    Widget w;
/*
**++
**  Functional Description:
**	Manage the children's managed state (does nothing).
**
**  Keywords:
**	Composite widget.
**
**  Arguments:
**	w: widget id of the child widget.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
{
} 


static void HwExposeCheck(help_widget, tag, callback)
    DXmHelpWidget     help_widget;
    XtPointer               tag;
    XmAnyCallbackStruct *callback;
/*
**++
**  Description:
**
**	This routine raises the error message window if there is one.
**
**	This routine, operated in response to an expose on the help widget, is
**	necessary to handle the situation where an error occurs in the process
**	of forming the help widget itself -- for example, a library-not-found
**	error.
**
**	In that situation it does no good to raise the error message widget
**	at the time the error is detected, because the Help widget does not yet
**	exist.
**	When help widget is finally posted it overlays the error message widget.
**
**	This routine catches that case and raises the error message widget
**	window after the help widget itself has been exposed.
**	
**  Keywords:
**	Help
**
**  Parameters:
**	help_wigdet : help widget id
**	tag	    : not used
**	callback    : not used
**
**  Exceptions:
**	None
**
**  Result:
**	None
**--
*/    
{
if (help_widget->dxmhelp.message)	           /* an error exists */
    {
    if (help_widget->dxmhelp.message_box)      /* error msg widget exists */
	{
	if (XtIsManaged(help_widget->dxmhelp.message_box))
	    HwRaiseWindow(help_widget->dxmhelp.message_box);
	}
    }
}

/****************************************************************/
static void HwDialogStyleDefault( widget, offset, value)
            Widget          widget ;
            int             offset ;
            XrmValue *      value ;
/****************
 * Set the default style to modeless if the parent is a
 *   DialogShell, otherwise set to workarea.
 ****************/
{
    static unsigned char    style ;
/****************/

    style = XmDIALOG_WORK_AREA ;

    if(    XmIsDialogShell (XtParent (widget))    )
    {   style = XmDIALOG_MODELESS ;
        }
    value->addr = (XtPointer) (&style) ;

    return ;
    }


static XtCallbackProc close_call_from_WM(widget, client_data, call_data)
    Widget 	widget;
    XtPointer 	client_data;
    XtPointer 	call_data;
{
/*
**++
**  Description:
**	This is called when the window manager is asked to 'close' the window.
**	We, in turn, execute the code that would be executed if the 'Exit'
**	button were activated.
**	
**  Parameters:
**	widget	    : the help widget
**	client_data : private-date -- also the help widget id
**	call_data   : ???
**
**  Exceptions:
**	None
**
**  Result:
**	None
**--
*/    

/* Mimic a quit callback. */

quit ( (XtPointer)0,	/* On a normal call to quit, this parameter would be  */
		        /* the widget id of the quit button.                  */
			/* However, this parameter is never used.             */
       client_data);    /* Widget id of the help widget.                      */

}


#ifdef _NO_PROTO
extern Widget DXmCreateHelp (parent, name, arglist, argcount)
    Widget	parent;
    char	*name;
    ArgList	arglist;
    int		argcount;
#else
extern Widget DXmCreateHelp (Widget 	parent, 
			     char 	*name, 
			     ArgList 	arglist, 
			     int 	argcount)
#endif /* _NO_PROTO */

/*
**++
**  Description:
**	This routine provides the low-level callable interface to create a
**	help widget
**	
**  Keywords:
**	Help
**
**  Parameters:
**	parent	    : parent widget,
**	name	    : widget name,
**	arglist	    : Decwtoolkit argument list,
**	argcount    : number of arguments
**
**  Exceptions:
**	None
**
**  Result:
**	returns the Help Widget id or NULL
**--
*/    
{
    DXmHelpWidget help_widget;
    DXmHelpShellWidget shell;
    Arg shellarg[10];
    int ac = 0;
    Atom delete_window_atom;        

    XtSetArg (shellarg[ac], XtNallowShellResize, TRUE);
    ac++;
    /**************************************************************************/
    /* Keep the widget from getting resized so small that its key parts start */
    /* to disppear.                                                           */
    /**************************************************************************/
    XtSetArg (shellarg[ac], XmNminWidth, MIN_HELP_WIDGET_WIDTH);
    ac++;
    XtSetArg (shellarg[ac], XmNminHeight, MIN_HELP_WIDGET_HEIGHT);
    ac++;

    shell = (DXmHelpShellWidget) DXmCreateHelpShell (parent, "Help",
							shellarg, ac);

    help_widget = (DXmHelpWidget) XtCreateWidget (name, dxmHelpWidgetClass,
		(Widget)shell, arglist, argcount) ;

    shell->helpshell.grabkind = XtGrabNone;
    return ((Widget) help_widget);

}



#ifdef _NO_PROTO
extern Widget DXmCreateHelpDialog (parent, name, arglist, argcount)
    Widget	parent;
    char	*name;
    ArgList	arglist;
    int		argcount;
#else
extern Widget DXmCreateHelpDialog (Widget 	parent, 
				   char 	*name, 
				   ArgList 	arglist, 
				   int 		argcount)
#endif /* _NO_PROTO */

/*
**++
**  Description:
**	This routine provides the low-level callable interface to create a
**	help widget
**	
**  Keywords:
**	Help
**
**  Parameters:
**	parent	    : parent widget,
**	name	    : widget name,
**	arglist	    : Decwtoolkit argument list,
**	argcount    : number of arguments
**
**  Exceptions:
**	None
**
**  Result:
**	returns the Help Widget id or NULL
**--
*/    
{
    DXmHelpWidget help_widget;
    DXmHelpShellWidget shell;
    Arg shellarg[10];
    int ac = 0;
        
    XtSetArg (shellarg[ac], XtNallowShellResize, TRUE);
    ac++;
    /**************************************************************************/
    /* Keep the widget from getting resized so small that its key parts start */
    /* to disppear.                                                           */
    /**************************************************************************/
    XtSetArg (shellarg[ac], XmNminWidth, MIN_HELP_WIDGET_WIDTH);
    ac++;
    XtSetArg (shellarg[ac], XmNminHeight, MIN_HELP_WIDGET_HEIGHT);
    ac++;
    shell = (DXmHelpShellWidget) DXmCreateHelpShell (parent, "Help",
							shellarg, ac);

    help_widget = (DXmHelpWidget) XtCreateWidget (name, dxmHelpWidgetClass,
		(Widget)shell, arglist, argcount) ;

    shell->helpshell.grabkind = XtGrabNone;
    return ((Widget) help_widget);

}
