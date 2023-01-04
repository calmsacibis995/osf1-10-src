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
**	RKR		05-Mar-1991
**	Removed the On Context, On Terms and On Version from the Using Help
**	pulldown menu.  That entailed pulling out the following infrastructure.
**
**	Remove: DXmSOncontext		"On Context"
**	        DXmSOncontextmnem	"C"
**    	        XmString            oncontext_label 	    from widget
**              Widget	            oncontext_button        from widget
**              KeySym              oncontext_label_mnem    from widget
**              XmStringCharSet     oncontext_label_mnem_cs from widget
**
**              DXmSGlossary            "On Terms..."
**              DXmSGlossarymnem        "T"
**              XmString	    glossary_label          from widget
**              Widget		    helpglossary_button     from widget
**              KeySym              glossary_label_mnem     from widget
**              XmStringCharSet     glossary_label_mnem_cs  from widget
**
**              DXmSAbout		"On Version..."
**              DXmSAboutmnem		"V"
**              XmString	    about_label             from widget
**		Widget		    about_button            from widget
**              KeySym              about_label_mnem        from widget
**		XmStringCharSet     about_label_mnem_cs     from widget
**
**	RKR		08-Nov-1990
**	Change default value for label of acknowledge button on error msg
**	to be "OK" (instead of "Ok").
**	Change default value for label of dismissal button to be "Cancel"
**	(instead of "Dismiss").
**
**	RKR		28-Aug-1990
**	Add elipses (...) to On Window, On Version and On Terms strings.
**
**	RKR		20-Aug-1990
**	Set up #defines that incorporate the DECIsreal changes
**
**	RKR		19-Aug-1990
**	Add a dialog_style field to widget.
**
**	RKR		16-Jul-1990
**	Change default value for label of acknowledge button on error msg
**	to be "Ok".
**
**	RKR		25-Jun-1990
**	Add resources for On Context button and rearrange the string values
**	associated with the Using Help pulldown buttons.
**
**	RKR		21-Jun-1990
**	Change mnemonic for 'Save As' to be 'A' per Motif style guide.
**
**	RKR		15-Jun-1990
**	Add resources to support mnemonics.
**
**	RKR 		17-May-1990
**	Remove definition of DXmSNulltopic and DXmSHelpfont to avoid confusion.
**
**--
**/

#ifndef _DXmHelpBoxP_h
#define _DXmHelpBoxP_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#include <DECW$INCLUDE:XmP.h>
#include <DECW$INCLUDE:ManagerP.h>
#include <DECW$INCLUDE:DXmHelpB.h>
#else
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <DXm/DXmHelpB.h>
#endif


/******************************************************************************/
/* Set up #defines that incorporate the DECIsreal changes.                    */
/*                                                                            */
/******************************************************************************/
#define avi_changes
#ifdef avi_changes
#define RTOL
/*
**   Under this cover all the RtoL geometry code based on the XmManager resource
**   namely DXmNlayoutDirecion.
**
*/

#define GENERAL_TEXT
/*
**   This conditional enables most the code which already uses compound string
**   but was good for latin 1 only (it uses XmStringLtoRCreate) to use instead
**   a DXm extension which is locale dependent.
**
*/

#define GENERAL_WM_TITLE
/*
**   This conditional expand the Help widget treatment of text which is intended
**   to be displayed on the title bar by the window manager.
**
**   It supports both the old Ascii only text and the new R4 supported
**   "CompoundText" encoding.
**
**   (for Latin 1 only text, it uses the old "String" atom which means that 
**   even non ICCCM compliant Window Manager can display the title bar text.
**
*/

#define HELP_LOCALE_STRINGS
/*
**   This conditional enables all the default values for the string resources
**   to consult the I18N layer and translate it in a locale sensitve way.
**
*/

#define HELP_LOCALE_MNEMONICS
/*
**   Enables all the default Mnemonics to consult the I18N layer to translate
**   the English default to a proper Locale sensitive KeySym.
**
**   Note that the current I18n layer lacks this functionality so I add a sample
**   code within the Help widget itself - please consult Mike Collins to resolve
**   this issue. ( proper comments on that are within the module help_widget.c)
**
*/

#define HELP_MNEM_CHARSET
/*
**   Since the current help widget didn't specify any value for mnemonic char
**   set resources (NULL) and it actually
**   relies on having a proper default within the Label widget itself, I let
**   my self add the neccessary code within the initialize proc to consult
**   the I18N layer to get the current default and plug it into each
**   mnemonic charset resource.
**
**   However Since there is a routine in Motif for querying for the current 
**   charset I used that one but under a disguise of a _DXm type of call.
**
**   Please consult Mike Colllins on this problem to move that too into the
**   DXMmisc and I18n Layer respectively.
**
**   Note that this is also commented in the code within help_widget.c
**
*/

#define NULL_TOPIC_BUG
/*
**   You can drop that one but it seems that is should be there to comply
**   with the documentation. - From my point of view you can drop it.
**   Read the comments on that in Help_widget.c
**
*/

#define CSTEXT
/*
**   This conditional (very important for a *true* I18n of the Help widget) is
**   actually three fold:
**
**   1. All instances of XmText widget are replaced with DXmCStext widget.
**
**   2. Various book keeping fields within the Help instant record are now
**      contains compound String instead of plain Ascii.
**
**   3. ClipBoard copy routine now store 3 formats ("String" - as before,
**     "CompoundText - for ICCCM compliancy, "DDIF" for DEC inter clients 
**      intechange).
**
**      (note for a original bug alert comment in the routine copy_clipboard)
**
*/
#endif  /* avi_changes */


/* 
 * Help conversion strings 
 */

#define DXmSHelptitle			"Help"
#define DXmSHelpontitle			"Help on "

#define DXmSView			"View"
#define	DXmSViewmnem			"V"

#define DXmSGoto			"Go to"
#define DXmSGoback			"Go Back"

#define DXmSGooverview			"Go to Overview"
#define	DXmSGoovermnem			"O"

#define DXmSVisit			"Visit"

#define DXmSVisitglossary		"Visit Glossary"
#define	DXmSVisitglossarymnem		"G"

#define DXmSFile			"File"
#define	DXmSFilemnem			"F"

#define DXmSSaveas			"Save As..."
#define	DXmSSaveasmnem			"A"

#define DXmSExit			"Exit"
#define	DXmSExitmnem			"x"

#define DXmSEdit			"Edit"
#define	DXmSEditmnem			"E"

#define DXmSCopy			"Copy"
#define	DXmSCopymnem			"C"

#define DXmSSelectall			"Select All"
#define	DXmSSelectallmnem		"S"

#define DXmSSearch			"Search"
#define	DXmSSearchmnem			"S"

#define DXmSHistory			"History..."
#define	DXmSHistorymnem			"H"

#define DXmSTitle			"Title..."
#define	DXmSTitlemnem			"T"

#define DXmSKeyword			"Keyword..."
#define	DXmSKeywordmnem			"K"

#define DXmSHelp			"On Help"
#define	DXmSHelpHelpmnem		"H"     

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmSGlossary			"Glossary..."
#define	DXmSGlossarymnem		"G"     
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmSAbout			"Product Information..."
#define	DXmSAboutmnem			"P"               
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
#define DXmSOncontext			"Context-Sensitive Help"
#define	DXmSOncontextmnem		"C"
#endif 	/* Remove everything except On Windows from Using Help Menu */

#define DXmSAddtopic			" Additional topics"
#define DXmSHelptopichistory		"Search Topic History"
#define DXmSTopictitles			"Topic Titles "
#define DXmSDismiss			"Cancel"
#define DXmSSearchtopictitles		"Search Topic Titles"
#define DXmSTitles			"Title "
#define DXmSSearchtopickeywords		"Search Topic Keywords"
#define DXmSKeywords			"Keyword "
#define DXmSSearchapply			"Apply"
#define DXmSBadlibrary		        "Couldn't open library !CS"
#define DXmSBadframe			"Couldn't find frame !CS"
#define DXmSNulllibrary		        "No library specified"
#define DXmSNotitle			"No title to match string !CS"
#define DXmSNokeyword			"Couldn't find keyword !CS"
#define DXmSErroropen			"Error opening file !CS"

#define DXmSGotoTopic			"Go to Topic"
#define	DXmSGotoTopicmnem		"t" 

#define DXmSGobackTopic			"Go Back"
#define	DXmSGobackmnem			"B"

#define DXmSVisitTopic			"Visit Topic"
#define	DXmSVisitTopicmnem		"V"

#define DXmSClose			"Exit"

#define DXmSHelpOnHelp			"Help"

#define DXmSHelpAcknowledgeLabel	"OK"

#define DXmSHelpOnHelpTitle		"Help"
#define	DXmSHelpmnem			"H"       
                                                              
/* New fields for the Help widget class record */

typedef struct {
    XtPointer		extension;	/* Pointer to extension record */
} DXmHelpClassPart ;

typedef struct {
    CoreClassPart	core_class ;
    CompositeClassPart	composite_class ;
    ConstraintClassPart	constraint_class;
    XmManagerClassPart	manager_class;
    DXmHelpClassPart	dxmhelp_class ;
} DXmHelpClassRec, *DXmHelpWidgetClass ;


/* Class record constants */

externalref DXmHelpClassRec	dxmHelpClassRec ;

/* New fields for the Help widget instance record */

typedef struct _DXmHelpTrail {
    char		*topic;
    char		*title;
    struct _DXmHelpTrail *next;
} DXmHelpTrail;
    
typedef struct _DXmHelpBox {
    Widget		box;
    Widget		label1;
    Widget		label2;
    Widget		button0;
    Widget		button1;
    Widget		button2;
    Widget		button3;
    Widget		listbox1;
    Widget		listbox2;
    Widget		text;
}DXmHelpBox;
    
typedef struct {
/* ?? re-arrange this in SOME logical order... ?? */
    XmString	application;		    /* application name	    */
    XmString	library;		    /* help library spec.   */
    XmString	first_topic;		    /* first topic spec.    */
    XmString	overview;		    /* overview topic spec. */
    XmString	glossary;		    /* glossary topic spec. */

	XmFontList	button_font_list;	/*  font lists		*/
	XmFontList	label_font_list;
	XmFontList	text_font_list;

	XmStringDirection	string_direction;

    unsigned int	library_type;		    /* help library type    */
    int			colons;			    /* number of colons for */
						    /* the text widget	    */
    int			rows;			    /* number of rows for   */
						    /* the text widget	    */
    XtCallbackList	unmap_callback;		    /* unmap callback	    */
    Boolean		default_pos;		    /* default position	    */

    Widget		shell;			    /* help shell widget id */
    Widget		menubar;		    /* menubar id	    */
    Widget		help_text;		    /* text widget id	    */
    Widget		help_topic;		    /* topic listbox id	    */
    Widget		add_topic;		    /* add_topic label id   */
    Widget		goback;			    /* goback button id	    */
    Widget		quit;			    /* exit button id	    */
    Widget		overview_button;	    /* overview menu item id*/
    Widget		glossary_button;	    /* glossary menu item id*/
	
    XmString	title;			    /* help widget title    */

    char		*library_context;	    /* help library context */
    XmString	text;			    /* current help text    */
    DXmHelpTrail	*trail;			    /* current help trail   */
    XmString	selected_topic;	    	    /* current selected topic*/
    XmString	selected_title;	    	    /* current selected title*/
    char		**topic_array;		    /* current topic array  */
    char		**title_array;		    /* current title array  */
    int			topic_count;		    /* current number of    */
						    /* topics		    */
    int			visible_count;		    /* number of visible    */
						    /* topics in the listbox*/

    DXmHelpBox		*saveas_box;		    /* save as dialog box   */
	
    DXmHelpBox		*title_box;		    /* search title dialog  */
						    /* box		    */
    char		**searchtitle_topic_array;  /* search title current */
						    /* topic array	    */
    XmString	selected_searchtitle_topic;/* search title current */
						    /* selected topic	    */
    
    DXmHelpBox		*keyword_box;		    /* search keyword	    */
						    /* dialog box	    */
    char		**keyword_array;	    /* search keyword	    */
						    /* current keyword array*/
    XmString	selected_keyword;	    /* search keyword	    */
						    /* current selected keyword*/
    char		**searchkeyword_topic_array;/* search keyword	    */
						    /* current topic array  */
    XmString	selected_searchkeyword_topic;/* search keyword	    */
						      /* current selected topic*/
    
    DXmHelpBox		*history_box;		    /* search history	    */
						    /* dialog box	    */
    DXmHelpTrail	*history;		    /* search history	    */
						    /* current trail	    */
    XmString	selected_history_topic;    /* search history	    */
						    /* current selected topic*/
    int			history_count;		    /* search history current*/
						    /* number of topics	    */

    XmString	view_menu_label;	    /* view menu label	    */
    XmString	goto_label;		    /* go to button label   */
    XmString	goback_label;		    /* goback button label  */
    XmString	goover_label;		    /* go to overview label */
    XmString	visit_label;		    /* visit button label   */
    XmString	visitglos_label;	    /* visit glossary item  */
    XmString	file_menu_label;	    /* file menu label	    */
    XmString	saveas_label;		    /* saveas menu item	    */
    XmString	exit_label;		    /* exit menu item	    */
    XmString	edit_menu_label;	    /* edit menu label	    */
    XmString	copy_label;		    /* copy menu item	    */
    XmString	selectall_label;	    /* select all menu item */
    XmString	search_menu_label;	    /* search menu label    */
    XmString	history_label;		    /* history menu item    */
    XmString	title_label;		    /* title menu item	    */
    XmString	keyword_label;		    /* keyword menu item    */
    XmString	help_menu_label;	    /* help on help menu label	    */
    XmString	helphelp_label;		    /* help menu item	    */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XmString	glossary_label;		    /* glossary menu item   */
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XmString	about_label;		    /* about menu item	    */
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XmString    oncontext_label;	    /* on context menu item */
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XmString	addtopic_label;		    /* additional topics label	*/
    XmString	history_box_label;	    /* history box title    */
    XmString	topic_titles_label;	    /* topic titles label   */
    XmString	dismiss_label;		    /* dismiss button label */
    XmString	searchtitle_box_label;	    /* title box title	    */
    XmString	titles_label;		    /* titles label	    */
    XmString	searchkeyword_box_label;    /* keyword box title    */
    XmString	keywords_label;		    /* keywords label	    */
    XmString	apply_label;		    /* apply button label   */
    Widget		goto_button;		    /* goto menu item id    */
    Widget		goback_button;		    /* goback menu item id  */
    Widget		visit_button;		    /* visit menu item id   */
    Widget		copy_button;		    /* copy menu item id    */
    Widget		history_button;		    /* history menu item id */
    Widget		title_button;		    /* title menu item id   */
    Widget		keyword_button;		    /* keyword menu item id */
    Widget		message_box;		    /* message_box widget   */
    Boolean		message;		    /* message to display   */
    XmString	message_text;		    /* message label	    */
    XmString	badlib_message;		    /* bad lib message	    */
    XmString	badframe_message;	    /* bad frame message    */
    XmString	nulllib_message;	    /* null lib message	    */
    XmString	notitle_message;	    /* no title message	    */
    XmString	nokeyword_message;	    /* no keyword message   */
    XmString	erroropen_message;	    /* error open file message*/
    Widget		avoid_widget[8];	    /* array of widgets to  */
    						    /* avoid		    */
    int			avoid_count;		    /* number of widgets to */
    						    /* avoid		    */
    char		*visit_context;		    /* pointer to visit context*/
    Widget		help_on_help;		    /* help on help widget  */
						    /* id		    */
    Widget		help_on_saveas;		    /* help on saveas widget*/
						    /* id		    */
    Widget		view_menu;		    /* view menu entry id   */
    Widget		file_menu;		    /* file menu entry id   */
    Widget		edit_menu;		    /* edit menu entry id   */
    Widget		search_menu;		    /* search menu entry id */
    Widget		help_menu;		    /* help menu entry id   */
    Widget		saveas_button;		    /* file menu item id    */
    Widget		exit_button;		    /* file menu item id    */
    Widget		selectall_button;	    /* edit menu item id    */
    Widget		help_button;		    /* help menu item id    */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    Widget		helpglossary_button;	    /* help menu item id    */
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    Widget		about_button;		    /* help menu item id    */
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    Widget		oncontext_button;	    /* help menu item id    */
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XmString	helpontitle_label;	    /* string to concat to  */
						    /* appl. name to build  */
						    /* the title	    */
    XmString	helptitle_label;	    /* string to use as	    */
						    /* title if appl. name  */
						    /* is null		    */
    XmString	goto_topic_label;	    /* goto topic menu item */
    XmString	goback_topic_label;	    /* goback menu item	    */
    XmString	visit_topic_label;	    /* visit topic menu item*/
    XmString	quit_label;		    /* exit button label    */
    XtCallbackList	map_callback;		    /* map callback	    */
    XmString	message_parameter;	    /* fao par. in err msg  */
    XmString	acknowledge_label;	    /* for the error box    */
    XmString	helponhelp_title;	    /* title for the	    */
						    /* help-on-help widget  */
    Boolean		cache_library;		    /* caching mode	    */
    Cursor		cursor;			    /* wait cursor	    */

    /* Mnemonics */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    KeySym      about_label_mnem;		    /*Mnemonic for About label*/
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    KeySym      oncontext_label_mnem;		    /*Mnemonic for On Context label*/
#endif 	/* Remove everything except On Windows from Using Help Menu */

    KeySym      copy_label_mnem;		    /*Mnemonic for Copy label*/
    KeySym      edit_label_mnem;		    /*Mnemonic for Edit label*/
    KeySym      exit_label_mnem;		    /*Mnemonic for Exit label*/
    KeySym      file_label_mnem;		    /*Mnemonic for File label*/

#if 0	/* Remove everything except On Windows from Using Help Menu */
    KeySym      glossary_label_mnem;		    /*Mnemonic for Glossary label*/
#endif 	/* Remove everything except On Windows from Using Help Menu */

    KeySym      goover_label_mnem;		    /*Mnemonic for Go To Overview label*/
    KeySym      help_label_mnem;		    /*Mnemonic for Using Help label*/
    KeySym      history_label_mnem;		    /*Mnemonic for History... label*/
    KeySym      keyword_label_mnem;		    /*Mnemonic for Keyword label*/
    KeySym      saveas_label_mnem;		    /*Mnemonic for Save As... label*/
    KeySym      search_label_mnem;		    /*Mnemonic for Search label*/
    KeySym      selectall_label_mnem;		    /*Mnemonic for Select All label*/
    KeySym      title_label_mnem;		    /*Mnemonic for Title... label*/
    KeySym      view_label_mnem;		    /*Mnemonic for View label*/
    KeySym      visitglos_label_mnem;		    /*Mnemonic for Visit Glossary label*/
    KeySym      goto_topic_label_mnem;		    /*Mnemonic for Go To Topic*/
    KeySym      goback_label_mnem;		    /*Mnemonic for Go Back label*/
    KeySym      visit_topic_label_mnem;		    /*Mnemonic for Visit Topic label*/
    KeySym      helphelp_label_mnem;		    /*Mnemonic for Overview label*/

    /* Mnemonic character sets */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XmStringCharSet     about_label_mnem_cs;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XmStringCharSet     oncontext_label_mnem_cs;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XmStringCharSet     copy_label_mnem_cs;
    XmStringCharSet     edit_label_mnem_cs;
    XmStringCharSet     exit_label_mnem_cs;
    XmStringCharSet     file_label_mnem_cs;

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XmStringCharSet     glossary_label_mnem_cs;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XmStringCharSet     goover_label_mnem_cs;
    XmStringCharSet     help_label_mnem_cs;
    XmStringCharSet     history_label_mnem_cs;
    XmStringCharSet     keyword_label_mnem_cs;
    XmStringCharSet     saveas_label_mnem_cs;
    XmStringCharSet     search_label_mnem_cs;
    XmStringCharSet     selectall_label_mnem_cs;
    XmStringCharSet     title_label_mnem_cs;
    XmStringCharSet     view_label_mnem_cs;
    XmStringCharSet     visitglos_label_mnem_cs;
    XmStringCharSet     goto_topic_label_mnem_cs;
    XmStringCharSet     goback_label_mnem_cs;
    XmStringCharSet     visit_topic_label_mnem_cs;
    XmStringCharSet     helphelp_label_mnem_cs;

    unsigned char       dialog_style;
} DXmHelpPart ;

typedef struct {
    CorePart		core ;
    CompositePart	composite ;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    DXmHelpPart		dxmhelp ;
} DXmHelpWidgetRec, *DXmHelpWidget ;

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif 
