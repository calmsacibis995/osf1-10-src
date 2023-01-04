/* #module DT_control "X0.0" */
/*
 *  Title:	DT_control
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993  All Rights      |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  
 *  Module Abstract:
 *
 *	This is the "control" component of the DECterm widget.
 *	It implements all the basic widget control functions for DECterm.
 *
 *	This module also contains the widget constant data structures
 *	such as the class structure and resource list.
 *
 *  Procedures contained in this module:
 *
 *	DECwTerm -		create a DECterm widget (high level call)
 *	DECwTermCreate -	create a DECterm widget (low level call)
 *
 *  Author:	Tom Porcher
 *
 *  Modification history:
 *
 *  Alfred von Campe    07-Dec-1993     BL-E
 *	- Subclass DECterm widget off of XmManager instead of just Composite.
 *
 *  Alfred von Campe    25-Oct-1993     BL-E
 *	- Change turkish language designator to tr_TR.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *
 *  Alfred von Campe    07-Oct-1992     Ag/BL10
 *      - Added type casts to satisfy Alpha compiler.
 *
 *  Aston Chan		20-Aug-1992	Post V1.1
 *	- ToLung's fix to make terminal type switchable on DECterm widget
 *	  based on xnllanguage in Japanese TPU.
 *
 *  Alfred von Campe    02-Apr-1992     Ag/BL6.2.1
 *      - Change name of #include file from CursorFont.h to cursorfont.h
 *
 *  Alfred von Campe    20-Feb-1992     V3.1
 *      - Add color text support.
 *
 *  Aston Chan		18-Feb-1991	V3.1/BL5
 *	- Shai's and Tolung's fix.  Change TerminalType resource to be a
 *	  read/write one.
 *
 *  Aston Chan		19-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Aston Chan		14-Nov-1991	V3.1
 *	- DECwNhelpCallback is the same as XmNhelpcallback so no change for
 *	  HyperHelp on that.
 *	- Change CreateWaitCursor() to DECwTermWatchCursor().
 *	- Add DECwTermNormalCursor().
 *
 *  Aston Chan		08-Nov-1991	V3.1
 *	- Add CreateWaitCursor() routine to bring up the watch while waiting
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Changed private #include file names to all lower case.
 *
 *  Eric Osman		20-Mar-1991	V3.0
 *	- Update action_table so that user can still type text character
 *	in DECterm when mouse is moved onto scrollbar.
 *
 *  Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS fonts.
 *
 *  Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS fonts.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 *  Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- forward declarations of set value functions
 * 
 *  Bob Messenger	23-Jun-1990	X3.0-5
 *	- Add printer port support.
 *	- Add secure keyboard support.
 *
 *  Mark Woodbury   25-May-1990 X3.0-3M
 *  - Motif update
 *
 *  Bob Messenger	05-Oct-1989	V2.0
 *	- Prevent fontUsed resource from being modified from outside the
 *	  DECterm widget (and prevent memory corruption from trying to
 *	  do this in o_set_value_fontUsed, which can be called after
 *	  open_font).
 *
 *  Bob Messenger	16-Jul-1989	X2.0-16
 *	- Set XtVersionDontCheck in core class record, to avoid widget class
 *	  version mismatch errors.
 *
 *  Bob Messenger	31-May-1989	X2.0-13
 *	- Declare o_set_value_fontUsed.
 *
 *  Bob Messenger	14-May-1989	X2.0-10
 *	- Added errorCallback, widget_error() and widget_message().
 *
 *  Bob Messenger	17-Apr-1989	X2.0-6
 *	- Declare DECtermWidgetClass with globaldef instead of extern.
 *
 *  Bob Messenger	13-Apr-1989	X2.0-6
 *	- Added DwtDECtermClearComm backward compatibility routine, which I
 *	  forgot last time.
 *	- Declare DECtermWidgetClass as noshare instead of read only, so VUE
 *	  will link properly.
 *
 *  Bob Messenger	 9-Apr-1989	X2.0-6
 *	- Add help callback, and add shell values callback and help callback
 *	  to high level creation routine.
 *
 *  Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Make widget class noshare on VMS for shareable library.
 *
 *  Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Change to new widget bindings (DECwTerm instead of DwtDECterm).
 *	  Keep old routine names for backward compatibility.
 *	- Removed external declaration for s_set_value_tabStops
 *
 *  Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Update shellValuesCallback in c_set_values_callback.
 *
 *  Bob Messenger	 1-Apr-1989	X2.0-5
 *	- Changed external declarations for s_set_value_title and
 *	  s_set_value_iconName to s_set_value_defaultTitle and
 *	  s_set_value_defaultIconName.
 *
 *  Bob Messenger	15-Mar-1989	X2.0-3
 *	- Added external declarations for s_set_value_transcriptSize,
 *	  regis_set_value_shareColormap, regis_set_value_bitPlanes,
 *	  and regis_set_value_backingStore.
 *
 *  Tom Porcher		 9-Jun-1988	X0.4-32
 *	- Make c_set_values prevent read-only resources from being altered.
 *	  This fixes the bug where "New" (Use System Defaults) does not
 *	  resize the window.
 *
 *  Eric Osman		13-May-1988	X0.4-27
 *	- Only run blink timer if necessary
 *
 *  Bob Messenger	28-Apr-1988	X0.4-14
 *	- Add calls to o_disable_redisplay and o_adjust_display in c_set_values.
 *
 *  Bob Messenger	28-Apr-1988	X0.4-14
 *	- Add calls to o_disable_redisplay and o_adjust_display in c_set_values.
 *
 *  Tom Porcher		22-Apr-1988	X0.4-11
 *	- #undef'ed the RESOURCE and C_RESOURCE macros so they don't get
 *	  redefined warnings on Ultrix.
 *	- Corrected i_accept_focus() to return Boolean.
 *
 *  Tom Porcher		21-Apr-1988	X0.4-10
 *	- Change all x_create() routines to x_realize().
 *	- Add calls to s_initialize(), i_initialize().
 *
 *  Tom Porcher		 7-Apr-1988	X0.4-7
 *	- Changed c_set_values to not use the class resource list; it's
 *	  not public, and now it's different.
 *	- Updated class record for new toolkit.
 *
 *  Eric Osman		 1-Apr-88	X0.4-0
 *	- Move blink calls from here into dt_output.
 *
 *  Eric Osman (Uh oh, trouble!)		
 *			29-Mar-88	X0.4-0
 *	- Add cursor blink support.
 *
 *  Bob Messenger	18-Feb-1988	X0.4-0
 *	- Added routines to create and destroy ReGIS and sixel contexts.
 *
 *  Tom Porcher		17-Jan-1988	X0.3-2
 *	- Moved i_create() above s_create() so that button_handler is correctly
 *	  set up.
 *
 *  Tom Porcher		31-Dec-1987	X0.3
 *	- Removed call to o_return_optimal_size; now done by o_initialize.
 *
 *  Tom Porcher		 4-Dec-1987	X0.3
 *	Added call to o_return_optimal_size() in c_set_value to insure
 *	that displayHeight and displayWidth are set correctly.
 *
 *  Tom Porcher		17-Nov-1987	X0.3
 *	Added c_set_value_v and other things to correct resource handling.
 *	Fixed c_set_values() to handle 1's complement resource_offset.
 *
 *  Tom Porcher		12-Nov-1987	X0.3
 *	removed c_focus_in; added external i_accept_focus.
 *	Added name parameter to DwtDECterm and DwtDECtermCreate.
 *
 *  Tom Porcher		19-Oct-1987	X0.1
 *	First release.
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 *
 */


#include "dectermp.h"

#if defined(VMS_DECTERM) || defined (VXT_DECTERM)
#include <cursorfont.h>
#include <decw$cursor.h>
#else
#include <X11/cursorfont.h>
#include <X11/decwcursor.h>
#endif

/*
 * Forward routine declarations
 */

Widget DECwTerm();
Widget DECwTermCreate();

void DECwTermWatchCursor();
void DECwTermNormalCursor();

void c_class_initialize();
void c_initialize();
void c_realize();
Boolean c_set_values();
void c_destroy();

void c_set_value_callbacks();

void c_type_initialize();

XtGeometryResult c_geometry_manager();
static XmNavigability WidgetNavigable();

/*
 * External routine declarations.  These are needed by macro RESOURCE
 * that is expanded by one of the include files.
 */

extern Boolean i_accept_focus();

extern void o_reconfigure();
extern void o_expose();
extern void o_disable_redisplay();
extern void o_enable_redisplay();
extern void o_adjust_display();

extern void s_realize();
extern void o_realize();
extern void i_realize();
extern void regis_create();
extern void sixel_create();

extern void s_destroy();
extern void o_destroy();
extern void i_destroy();
extern void regis_destroy();
extern void sixel_destroy();

extern void s_set_value_columns_rows();
extern void s_set_value_screenMode();
extern void s_set_value_saveLinesOffTop();
extern void s_set_value_userPreferenceSet();
extern void s_set_value_responseDA();
extern void s_set_value_terminalMode();
extern void s_set_value_misc();
extern void s_set_value_cursor_blink();
extern void s_set_value_keyboardDialect();
extern void s_set_value_eightBitCharacters();
extern void s_set_value_keyclickEnable();
extern void s_set_value_autoRepeatEnable();
extern void s_set_value_batchScrollCount();
extern void s_set_value_statusDisplayEnable();
extern void s_set_value_localEcho();
extern void s_set_value_concealAnswerback();
extern void s_set_value_answerbackMessage();
extern void s_set_value_textCursorEnable();
extern void s_set_value_transcriptSize();
extern void s_set_value_defaultTitle();
extern void s_set_value_defaultIconName();
extern void s_set_value_printMode();
extern void s_set_value_printExtent();
extern void s_set_value_printerPortName();
extern void s_set_value_printerFileName();
extern void s_set_value_prt_to_host();
extern void s_set_value_crm();	/* 910903, TN400 */
extern void s_set_value_printDisplayMode();
extern void s_set_value_kanji_78_83();
extern void s_set_value_jisRomanAsciiMode();
extern void s_set_value_kanjiKatakanaMode();
extern void s_set_value_terminalType();
extern void s_set_value_ksRomanAsciiMode();
extern void s_set_value_rToL();
extern void s_set_value_copyDir();

extern void o_set_value_reverseVideo();
extern void o_set_value_cursorStyle();
extern void o_set_value_fontSetSelection();
extern void o_set_value_bigFontSetName();
extern void o_set_value_gsFontSetName();
extern void o_set_value_littleFontSetName();
extern void o_set_value_bigFontOtherName();
extern void o_set_value_littleFontOtherName();
extern void o_set_value_gsFontOtherName();
extern void o_set_value_scrollHorizontal();
extern void o_set_value_scrollVertical();
extern void o_set_value_foreground();
extern void o_set_value_background();
extern void o_set_value_condensedFont();
extern void o_set_value_fontUsed();
extern void o_set_value_useBoldFont();
extern void o_set_value_fineFontSetName();

extern void i_set_value_inputCallback();
extern void i_set_value_whiteSpace();

#ifdef SECURE_KEYBOARD
extern void i_set_value_secureKeyboard();
#endif

extern void regis_set_value_shareColormap();
extern void regis_set_value_bitPlanes();
extern void regis_set_value_backingStore();
extern void regis_set_value_regisScreenMode();


/*
 * Class action table
 */

extern void InputFromWidgets();
static XtActionsRec action_table[] = {
    {"InputFromWidgets",  (XtActionProc) InputFromWidgets}
};

/*
 * Resource list for DECterm parameters
 */

static XtResource resource_list[] = {

#define RESOURCE( p1, p2, p3, p4, p5, p6, p7, p8 ) \
  { p1, p2, p3, sizeof(p4), XtOffset(DECtermWidget,common.p5), p6, p7 },
#define C_RESOURCE( p1, p2, p3, p4, p5, p6, p7, p8 ) \
  { p1, p2, p3, sizeof(p4), XtOffset(DECtermWidget,p5), p6, p7 },

#include "dt_resources.h"
#undef RESOURCE
#undef C_RESOURCE
    };


/*
 * DECterm's private resource list for DECterm parameters
 *
 */

typedef struct {	/* DT_Resource, *DT_ResourceList */
    short int resource_size;
    short int resource_offset;
    void (*resource_handler)();
} DT_Resource, *DT_ResourceList;

static DT_Resource dt_resource_list[] = {

#define   RESOURCE( p1, p2, p3, p4, p5, p6, p7, p8 ) \
  { sizeof(p4), XtOffset(DECtermWidget,common.p5), p8 },
#define C_RESOURCE( p1, p2, p3, p4, p5, p6, p7, p8 ) \
  { sizeof(p4), XtOffset(DECtermWidget,p5), p8 },

#include "dt_resources.h"
#undef RESOURCE
#undef C_RESOURCE

    };

/*
 * Widget Class record declarations
 */

typedef struct {
    int dummy;
    } DECtermClassPart;

typedef struct {		/* DECtermWidgetClassRec */
    CoreClassPart core_class;
    CompositeClassPart composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart manager_class;
    DECtermClassPart decterm_class;
    } DECtermWidgetClassRec;


static XmBaseClassExtRec baseClassExtRec = {
    NULL,
    NULLQUARK,
    XmBaseClassExtVersion,
    sizeof(XmBaseClassExtRec),
    NULL,				/* InitializePrehook	*/
    NULL,				/* SetValuesPrehook	*/
    NULL,				/* InitializePosthook	*/
    NULL,				/* SetValuesPosthook	*/
    NULL,				/* secondaryObjectClass	*/
    NULL,				/* secondaryCreate	*/
    NULL,               		/* getSecRes data	*/
    { 0 },      			/* fastSubclass flags	*/
    NULL,				/* getValuesPrehook	*/
    NULL,				/* getValuesPosthook	*/
    NULL,                               /* classPartInitPrehook */
    NULL,                               /* classPartInitPosthook*/
    NULL,                               /* ext_resources        */
    NULL,                               /* compiled_ext_resources*/
    0,                                  /* num_ext_resources    */
    FALSE,                              /* use_sub_resources    */
    WidgetNavigable,                    /* widgetNavigable      */
    NULL                                /* focusChange          */
};

/*
 * Widget Class record data constant
 */
        
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
externaldef($$DECtermWidgetClassData)
#endif
    DECtermWidgetClassRec DECtermWidgetClassData = {
	{
	/* superclass	*/	(WidgetClass) &xmManagerClassRec,
	/* class_name	*/	"DECterm",
	/* widget_size	*/	sizeof(DECtermData),
	/* class_initialize */	c_class_initialize,
	/* class_part_init */	NULL,
	/* class_inited	*/	FALSE,
	/* initialize	*/	c_initialize,
	/* initialize_hook */	NULL,
	/* realize	*/	c_realize,
	/* action_bindings */	action_table,
	/* action_count */	XtNumber(action_table),
	/* resources	*/	resource_list,
	/* resource_count */	XtNumber(resource_list),
	/* xrm_class	*/	NULLQUARK,
	/* compress_motion */	TRUE,
	/* compress_exposure */	FALSE,
	/* compress_enter_exit */ TRUE,
	/* viewable_interest */	TRUE,
	/* destroy */		c_destroy,
	/* resize */		o_reconfigure,
	/* expose */		o_expose,
	/* set_values */	c_set_values,
	/* set_values_hook */	NULL,
	/* set_values_almost */	XtInheritSetValuesAlmost,
	/* get_values_hook */	NULL,
	/* accept_focus */	i_accept_focus,
	/* version */		XtVersionDontCheck,
	/* callback_private */	NULL,
	/* tm_table */		XtInheritTranslations,
        /* query_geometry */    NULL,
	/* disp accelerator */  NULL,
	/* extension */		(XtPointer)&baseClassExtRec,
	},
	{
	/* geometry_manager */  c_geometry_manager,
	/* change_managed */	XtInheritChangeManaged,
	/* insert_child */	XtInheritInsertChild,
	/* insert_child */	XtInheritDeleteChild,
	/* extension */		NULL,
	},
	{
	/*constraint resource */ NULL,
	/* number of constraints */ 0,
	/* size of constraint*/	 0,
	/* initialization */	NULL,
	/* destroy proc */	NULL,
	/* set_values proc */	NULL,
	/* extension */		NULL,
	},
	{
	/* translations */	XtInheritTranslations,        
	/* syn_resources */	NULL,
	/* num_syn_resources */	0,
	/* syn_cont_resources */ NULL,
	/* num_syn_cont_resources */ 0,
	/* parent_process */	XmInheritParentProcess,
	/* extension */		NULL,
	},
	{
	/* dummy */		0
	}

    };

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
globaldef
#endif
WidgetClass DECtermWidgetClass = (WidgetClass) &DECtermWidgetClassData;

/*
 * DECwTerm() -- create a DECterm widget (high-level)
 * DECwTermCreate() -- create a DECterm widget (low-level)
 */

Widget DECwTerm( parent, name,
		 inputCallback,
		 stopOutputCallback,
		 startOutputCallback,
		 resizeCallback,
		 shellValuesCallback,
		 helpCallback )
    Widget		parent;
    String		name;
    XtCallbackList	inputCallback;
    XtCallbackList	stopOutputCallback;
    XtCallbackList	startOutputCallback;
    XtCallbackList	resizeCallback;
    XtCallbackList	shellValuesCallback;
    XtCallbackList	helpCallback;
{
    Arg		arglist[6];	/* argument list for passing callbacks */

    XtSetArg( arglist[0], DECwNinputCallback, inputCallback );
    XtSetArg( arglist[1], DECwNstopOutputCallback, stopOutputCallback );
    XtSetArg( arglist[2], DECwNstartOutputCallback, startOutputCallback );
    XtSetArg( arglist[3], DECwNresizeCallback, resizeCallback );
    XtSetArg( arglist[4], DECwNshellValuesCallback, shellValuesCallback );
    XtSetArg( arglist[5], DECwNhelpCallback, helpCallback );

    return DECwTermCreate( parent, name, arglist, 6 );

}	/* end of DECwTerm() */

Widget DECwTermCreate( parent, name, arglist, argcount )
    Widget	parent;
    String	name;
    ArgList	arglist;
    int		argcount;
{

    return XtCreateWidget( name, DECtermWidgetClass, parent, arglist, argcount ) ;

}	/* end of DECwTerm() */

/* well, that was easy!!  Now comes the real stuff... */

/*
 * DECwTermClearComm() -- reset all flow control
 *
 */
void DECwTermClearComm( w )
    DECtermWidget	w;
{
int call_data;

/*
 * Call clear_comm routines on all other components.
 */
    o_clear_comm( w ) ;
    s_clear_comm( w ) ;
    i_clear_comm( w ) ;

    call_data = DECwCRStopPrintScreen;
    XtCallCallbacks( (Widget)w, DECwNstopPrintingCallback, &call_data );

} /* end of DECwTermClearComm */

void c_type_initialize( w )
DECtermWidget w;
{
    int lang_index;
    w->source.wvt$l_ext_flags = 0;
    switch ( w->common.terminalType ) {
    case DECwKanji:
	w->source.wvt$l_ext_flags |= vte1_m_tomcat;
	break;
    case DECwHanzi:
	w->source.wvt$l_ext_flags |= vte1_m_bobcat;
	break;
    case DECwHangul:
	w->source.wvt$l_ext_flags |= vte1_m_dickcat;
	break;
    case DECwHanyu:
	w->source.wvt$l_ext_flags |= vte1_m_fishcat;
	break;
    case DECwHebrew:
	w->source.wvt$l_ext_flags |= vte1_m_hebrew;
	break;
    case DECwStandard2:
	w->common.terminalType = DECwStandard;
	w->source.wvt$l_ext_flags = 0;
	break;
    case DECwMulti:
	w->source.wvt$l_ext_specific_flags |= vte2_m_multi_mode;
    default:
	lang_index = decw$term_get_lang_index( NULL, XtDisplay( w ));
	switch ( lang_index ) {
	case  7: w->common.terminalType = DECwHanzi;	/* zh_CN */
		 w->source.wvt$l_ext_flags |= vte1_m_bobcat;
		 break;
	case  8: w->common.terminalType = DECwHanyu;	/* zh_TW */
		 w->source.wvt$l_ext_flags |= vte1_m_fishcat;
		 break;
	case 15: w->common.terminalType = DECwHebrew;	/* iw_IL */
		 w->source.wvt$l_ext_flags |= vte1_m_hebrew;
		 break;
	case 17: w->common.terminalType = DECwKanji;	/* ja_JP */
		 w->source.wvt$l_ext_flags |= vte1_m_tomcat;
		 break;
	case 18: w->common.terminalType = DECwHangul;	/* ko_KR */
		 w->source.wvt$l_ext_flags |= vte1_m_dickcat;
		 break;

	case 28: w->common.terminalType = DECwGreek;    /* gr_GR */
		 w->source.wvt$l_ext_flags |= vte1_m_greek;
		 break;

	case 29: w->common.terminalType = DECwTurkish;  /* tr_TR */
		 w->source.wvt$l_ext_flags |= vte1_m_turkish;
		 break;

	case  0:					/* en_US */
	default: w->common.terminalType = DECwStandard;
		 w->source.wvt$l_ext_flags = 0;
		 break;
	}
	break;
    }

    if ( w->source.wvt$l_ext_flags & vte1_m_turkish )
    {
	w->common.keyboardDialect = DECwTurkishDialect;
	w->common.userPreferenceSet = DECwISO_Latin5_Supplemental;
    } else if ( w->source.wvt$l_ext_flags & vte1_m_greek )
    {
	w->common.keyboardDialect = DECwGreekDialect;
	w->common.userPreferenceSet = DECwISO_Latin7_Supplemental;
    } else

    if ( w->source.wvt$l_ext_flags & vte1_m_hebrew ) {
	w->common.keyboardDialect = DECwHebrewDialect;
	w->common.userPreferenceSet = DECwISO_Latin8_Supplemental;
    }
}

/*
 * c_initialize() -- initialize the DECterm widget
 *
 * This is called by XtCreateWidget.
 * When called, all the resource fields have been initialized by the
 * arglist specified by the application or by the resource
 * manager from the resource database or from the resource list.
 *
 * This routine initializes enough of the data structures so that
 * all widget routines will work.  Most of the real work comes
 * when the widgets are actually realized.
 */

void c_initialize( request, w )
    DECtermWidget		w;
{

    c_type_initialize( w );
    regis_initialize( w );
    o_initialize( w );		/* depends on regis_initialize */
    sixel_initialize( w );	/* depends on o_initialize, regis_initialize */
    i_initialize( w );
    s_initialize( w );		/* depends on all of the above */

    w->core.width = w->common.displayWidth;
    w->core.height = w->common.displayHeight;

} /* end of c_initialize */

/*
 * c_realize() -- realize the DECterm widget
 *
 * This is called by XtRealizeWidget.
 */

void c_realize( w, value_mask, attributes )
    DECtermWidget	w;
    Mask		*value_mask;
    XSetWindowAttributes  *attributes;
{
/*
 */
    attributes->event_mask |= 
			   ButtonMotionMask|Button1MotionMask|
			   Button2MotionMask|Button3MotionMask|Button4MotionMask|
			   Button5MotionMask;
/*
 * Create the main window for the DECterm widget.
 */
    XtCreateWindow((Widget) w, (unsigned int) InputOutput,
		   (Visual *) CopyFromParent,
                   *value_mask, attributes);

/*
 * Call creation routines on all other components.
 */
    o_realize( w ) ;
    regis_realize( w );	/* initialize before ReGIS is called by Source */
    sixel_realize( w );
    i_realize( w ) ;
    s_realize( w ) ;

} /* end of c_realize */

/*
 * c_set_values -- set new values for a DECterm widget
 */

Boolean c_set_values( oldw, requestw, neww )
    DECtermWidget	oldw, requestw, neww;
{
    DT_ResourceList	list = dt_resource_list;
    int                 i;

    /* Copy read-only resources in new widget from old */

    neww->common.displayWidth = oldw->common.displayWidth;
    neww->common.displayHeight = oldw->common.displayHeight;
    neww->common.displayWidthInc = oldw->common.displayWidthInc;
    neww->common.displayHeightInc = oldw->common.displayHeightInc;
    neww->common.fontUsed = oldw->common.fontUsed;

    o_disable_redisplay( neww );

    for ( i=0; i < XtNumber(dt_resource_list); i++,list++ )
    {
	int	j;
	int	offset;
	char	*old,*new;

	offset = list->resource_offset;

	old = (char*) oldw+offset;
	new = (char*) neww+offset;

	for ( j=0; j < list->resource_size; j++)
	{
	    if ( *old++ != *new++ ) break ;
        }

	if ( j != list->resource_size )
        {
	    if ((caddr_t) list->resource_handler != NULL )
	    {
		(*(list->resource_handler))( oldw, neww );
	    }
	}
    } /* end for ( i... ) */

    o_enable_redisplay( neww );
    o_adjust_display( neww );

    return FALSE;	/* any redisplay has been done */

} /* end of c_set_values */

/*
 * c_set_value_callbacks -- a callback procedure has changed
 *
 * We update them all if any have changed.
 */

void c_set_value_callbacks( oldw, neww )
    DECtermWidget oldw, neww;
{

    
DecTermUpdateCallback
( oldw, &(oldw->common.stopOutputCallback),
                       neww, &(neww->common.stopOutputCallback),
		       DECwNstopOutputCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.startOutputCallback),
                       neww, &(neww->common.startOutputCallback),
		       DECwNstartOutputCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.resizeCallback),
                       neww, &(neww->common.resizeCallback),
		       DECwNresizeCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.shellValuesCallback),
                       neww, &(neww->common.shellValuesCallback),
		       DECwNshellValuesCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.helpCallback),
                       neww, &(neww->common.helpCallback),
		       DECwNhelpCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.errorCallback),
                       neww, &(neww->common.errorCallback),
		       DECwNerrorCallback );

DecTermUpdateCallback
( oldw, &(oldw->common.startPrintingCallback),
                       neww, &(neww->common.startPrintingCallback),
		       DECwNstartPrintingCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.stopPrintingCallback),
                       neww, &(neww->common.stopPrintingCallback),
		       DECwNstopPrintingCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.startPrinterToHostCallback),
                       neww, &(neww->common.startPrinterToHostCallback),
		       DECwNstartPrinterToHostCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.stopPrinterToHostCallback),
                       neww, &(neww->common.stopPrinterToHostCallback),
		       DECwNstopPrinterToHostCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.printLineCallback),
                       neww, &(neww->common.printLineCallback),
		       DECwNprintLineCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.printerStatusCallback),
                       neww, &(neww->common.printerStatusCallback),
		       DECwNprinterStatusCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.sendBreakCallback),
                       neww, &(neww->common.sendBreakCallback),
		       DECwNsendBreakCallback );

    
DecTermUpdateCallback
( oldw, &(oldw->common.exitCallback),
                       neww, &(neww->common.exitCallback),
		       DECwNexitCallback );

    
}


/*
 * c_destroy -- destroy a DECterm widget
 *
 * This routine will return all dynamic storage (besides the Widget Data Record)
 * allocated by DECterm.
 */

void c_destroy( w )
    DECtermWidget	w;
{

/*
 * Call destroy routines on all other components.
 */
    o_destroy( w ) ;
    s_destroy( w ) ;
    i_destroy( w ) ;
    regis_destroy( w );
    sixel_destroy( w );

/*
 * Remove all callbacks
 */

    XtRemoveAllCallbacks( (Widget) w, DECwNinputCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNstopOutputCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNstartOutputCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNresizeCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNshellValuesCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNhelpCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNerrorCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNstartPrintingCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNstopPrintingCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNstartPrinterToHostCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNstopPrinterToHostCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNprintLineCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNprinterStatusCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNsendBreakCallback );
    XtRemoveAllCallbacks( (Widget) w, DECwNexitCallback );

} /* end of c_destroy */


/*
 * c_class_initialize -- initialize DECterm class widget structure
 */

void c_class_initialize()
{
    baseClassExtRec.record_type = XmQmotif;
} /* end of c_class_initialize */


XtGeometryResult c_geometry_manager()
{
 return(XtGeometryYes);
}

/* backward compatibility entry points */

Widget DwtDECterm( parent, name,
		   inputCallback,
		   stopOutputCallback,
		   startOutputCallback,
		   resizeCallback )
    Widget		parent;
    String		name;
    XtCallbackList	inputCallback;
    XtCallbackList	stopOutputCallback;
    XtCallbackList	startOutputCallback;
    XtCallbackList	resizeCallback;
{
    return DECwTerm( parent, name, inputCallback, stopOutputCallback,
			startOutputCallback, resizeCallback, NULL, NULL );
}

Widget DwtDECtermCreate( parent, name, arglist, argcount )
    Widget	parent;
    String	name;
    ArgList	arglist;
    int		argcount;
{
    return DECwTermCreate( parent, name, arglist, argcount );
}

void DwtDECtermClearComm( w )
    DECtermWidget w;
{
    DECwTermClearComm( w );
}

/*
 * widget_error - signal an error to the application
 *
 * This routine calls the errorCallback callback routine in the application
 * to signal that the DECTerm Widget has encountered an error condition.
 */

widget_error( w, code, status, text )
    DECtermWidget w;	/* DECTerm Widget ID */
    int code;		/* DECterm specific error code (see decterm.h) */
    int status;		/* operating system specific error code */
    char *text;		/* null terminated message text */
{
    DECwTermErrorCallbackStruct data;

    data.reason = DECwCRError;
    data.code = code;
    data.status = status;
    data.text = text;
    XtCallCallbacks( (Widget)w, DECwNerrorCallback, &data );
}

/*
 * widget_message - print a debugging message from the widget
 *
 * This routine calls widget_error to ask the application to display
 * a non-translatable informational message that is useful for debugging.
 * The arguments are similar to printf, with up to four arguments in
 * adition to the widget ID and format string.  The data is allocated on
 * the stack, so the callback routine has to print the message text
 * immediately or else copy it to allocated memory before returning.
 */

widget_message( w, format, arg1, arg2, arg3, arg4 )
    DECtermWidget w;
    char *format;
    void *arg1, *arg2, *arg3, *arg4;
{
    char buffer[512];

    sprintf( buffer, format, arg1, arg2, arg3, arg4 );
    widget_error( w, DECW$K_MSG_INFORMATIONAL, 0, buffer );
}

void DECwTermWatchCursor(w)
    DECtermWidget w;
{
    XColor		cursor_colors[2];
    Font		cursor_font;
    int			cursor_wait;
    Display		*display;
    Screen		*screen;
    static 		wait_cursor = NULL;

    if ( wait_cursor == NULL )
    {		
	/********************************************************************/
	/*								    */
	/* Try to get Digital's wait cursor.  If we can't find the wait	    */
	/* cursor font, just load the X watch cursor.			    */
	/*								    */
	/********************************************************************/

	display = XtDisplay(w);

	if (cursor_font = XLoadFont(display,"decw$cursor"))
	{     
	    screen = DefaultScreenOfDisplay(display);

	    cursor_colors[0].pixel = WhitePixelOfScreen(screen);
	    cursor_colors[1].pixel = BlackPixelOfScreen(screen);

	    XQueryColors( display,DefaultColormapOfScreen(screen),
			  cursor_colors,2 );

	    cursor_wait = decw$c_wait_cursor;

	    wait_cursor = XCreateGlyphCursor( display,
					      cursor_font,
					      cursor_font,
					      cursor_wait,
					      cursor_wait+1,
					      &cursor_colors[0],
					      &cursor_colors[1] );
    
	    XUnloadFont(display,cursor_font);
	}
	else
	    wait_cursor = XCreateFontCursor(display,XC_watch);

    }

    XDefineCursor( XtDisplay(w), XtWindow(w), wait_cursor );

} /* DECwTermWatchCursor */

void DECwTermNormalCursor ( w )
    DECtermWidget w;
{
	/* Just a jacket routine for restoring normal cursor.  It is here
	 * for completeness.
	 */
	XUndefineCursor ( XtDisplay(w), XtWindow(w) );
}

static XmNavigability
WidgetNavigable( wid)
    Widget wid ;
{   
  return XmTAB_NAVIGABLE;
}
