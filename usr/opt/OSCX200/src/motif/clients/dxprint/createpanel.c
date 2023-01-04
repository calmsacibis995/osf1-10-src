/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                 *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	This module handles createing the panel
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	 4-Feb-1992	Edward P Luwish
**		Performance and I18N modifications
**		Fixed bug in MrmFetchWidgetOverride call
**
**	11-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
**	26-Oct 1990	KMR		Kathy Robinson
**		Unmange color button on mono system
**
*/

/*
** Include files
*/
#include "iprdw.h"

#ifdef VMS
#include <ssdef.h>
#include <signal.h>
#endif /* VMS */

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <DXm/DECspecific.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "smdata.h"
#include "smshare.h"
#include "smresource.h"
#include "prdw_entry.h"

static unsigned int	initial = 0;
extern XtAppContext applicationContext;

int create_panel ()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create the session manager main window, menu bar, and control
**	panel.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
*/
{
    Widget	main_window;
    static Widget	widget_list[3];
    Arg	arglist[10];
    Arg	args[30];
    char	*dummy;
    unsigned long rows = 0;
    int 	screen_type;

    static XtTranslations label_translations_parsed; 

    static XtTranslations main_translations_parsed; /* Translation table */

    static char label_translation_table [] = 
	 "<MapNotify>:	   	map_event()";

    static char main_translation_table [] =
	"<ConfigureNotify>:         configure_event()";
    /*
    ** now establish an action binding table which associates the above
    ** procedure names (ascii) with actual addresses
    */

    static XtActionsRec our_action_table [] =
    {
	{"configure_event", (XtActionProc)configure_event},
	{"map_event",       (XtActionProc)map_event}
    };

    static Arg pulldown_args[] =
    {
	{XmNsubMenuId, (XtArgVal)NULL},
    };

    static MrmRegisterArg reglist[] =
    {
        {"widget_create_proc", (caddr_t) widget_create_proc},
	{"session_menu_cb", (caddr_t)session_menu_cb},
	{"end_session", (caddr_t)end_session},
	{"help_menu_cb", (caddr_t)help_menu_cb},
	{"customize_menu_cb", (caddr_t)setup_menu_cb},
        {"print_menu_cb", (caddr_t)print_menu_cb},
	{"sens_help_proc", (caddr_t)sens_help_proc},
	{"page_size_cancel", (caddr_t)page_size_cancel},
	{"sixel_device_cancel", (caddr_t)sixel_device_cancel},
	{"quitbutton", (caddr_t) &smdata.quit_button},
	{"screenbutton", (caddr_t) &smdata.screen_button},
        {"printerbutton", (caddr_t) &smdata.printer_button},
        {"printesbutton", (caddr_t) &smdata.print_es},
        {"printposbutton", (caddr_t) &smdata.print_pos},
        {"captureesbutton", (caddr_t) &smdata.capture_es},
        {"captureposbutton", (caddr_t) &smdata.capture_pos},
	{"postopbutton", (caddr_t) &smdata.postopbutton},
	{"sixopbutton", (caddr_t) &smdata.sixopbutton},
	{"use_last_button", (caddr_t) &smdata.use_last_button},
	{"use_system_button", (caddr_t) &smdata.use_system_button},
	{"save_current_button", (caddr_t) &smdata.save_current_button},
/*	{"helpoverviewbutton", &smdata.help_overview},	*/
/*	{"helpaboutbutton", &smdata.help_about},	*/
	{"helpwin", (caddr_t) &smdata.help_window},
	{"helpver", (caddr_t) &smdata.help_version},
	{"helpcon", (caddr_t) &smdata.help_context},
	{"helphelp", (caddr_t) &smdata.help_help},
        {"messagearea", (caddr_t) &smdata.message_area},
	{"PSDDIFCallback", (caddr_t) PSDDIFCallback},
        {"PSColorCallback", (caddr_t) PSColorCallback},
        {"PSPostCallback", (caddr_t) PSPostCallback},

        {"FormatBoxId", (caddr_t) &prtsetup.format_id},
        {"PostScriptId", (caddr_t) &prtsetup.postscript_id},
        {"SixelId", (caddr_t) &prtsetup.sixel_id},
        {"BitmapId", (caddr_t) &prtsetup.bitmap_id},

        {"RatioBoxId", (caddr_t) &prtsetup.ratio_id},
        {"OnetoOneId", (caddr_t) &prtsetup.r1to1_id},
        {"TwotoOneId", (caddr_t) &prtsetup.r2to1_id},

        {"SaverBoxId", (caddr_t) &prtsetup.saver_id},
        {"PositiveId", (caddr_t) &prtsetup.positive_id},
        {"NegativeId", (caddr_t) &prtsetup.negative_id},

        {"FilenameId", (caddr_t) &prtsetup.capture_file_id},
/*        {"FilePromptId", &prtsetup.file_prompt_id},  */
        {"RotatePromptId", (caddr_t) &prtsetup.rotate_prompt_id},

        {"ColorBoxId", (caddr_t) &prtsetup.prtcolor_id},
        {"PSColorId", (caddr_t) &prtsetup.color_id},
        {"PSGreyId", (caddr_t) &prtsetup.grey_id},
        {"PSBWId", (caddr_t) &prtsetup.bw_id},

	{"FitBoxId", (caddr_t) &prtsetup.fit_id},
	{"FitReduceId", (caddr_t) &prtsetup.reduce_id},
	{"FitGrowId", (caddr_t) &prtsetup.grow_id},
	{"FitShrinkId", (caddr_t) &prtsetup.shrink_id},
	{"FitScaleId", (caddr_t) &prtsetup.scale_id},
	{"FitCropId", (caddr_t) &prtsetup.crop_id},

	{"DestPrintId", (caddr_t) &prtsetup.print_id},
	{"DestFileId", (caddr_t) &prtsetup.file_id},
	{"DestBothId", (caddr_t) &prtsetup.both_id},

	{"CaptEntrId", (caddr_t) &prtsetup.entire_id},
	{"CaptPartId", (caddr_t) &prtsetup.partial_id},

	{"DelayId", (caddr_t) &prtsetup.delay_id},

	{"OrientBoxId", (caddr_t) &prtsetup.orient_box_id},
	{"BestId", (caddr_t) &prtsetup.best_id},
	{"PortraitId", (caddr_t) &prtsetup.portrait_id},
	{"LandscapeId", (caddr_t) &prtsetup.landscape_id},

	{"PageBoxId", (caddr_t) &prtsetup.page_size_box_id},
	{"LetterId", (caddr_t) &prtsetup.letter_id},
	{"BsizeId", (caddr_t) &prtsetup.Bsize_id},
	{"AsizeId", (caddr_t) &prtsetup.Asize_id},
	{"A5sizeId", (caddr_t) &prtsetup.A5size_id},
	{"A4sizeId", (caddr_t) &prtsetup.A4size_id},
	{"A3sizeId", (caddr_t) &prtsetup.A3size_id},
	{"B5sizeId", (caddr_t) &prtsetup.B5size_id},
	{"B4sizeId", (caddr_t) &prtsetup.B4size_id},
	{"LedgerId", (caddr_t) &prtsetup.ledger_id},
	{"LegalId", (caddr_t) &prtsetup.legal_id},
	{"ExecId", (caddr_t) &prtsetup.exec_id},
	{"SixelDevId", (caddr_t) &prtsetup.sixel_box_id},
	{"VTId", (caddr_t) &prtsetup.vt_id},
	{"LA50Id", (caddr_t) &prtsetup.la50_id},
	{"LA75Id", (caddr_t) &prtsetup.la75_id},
	{"LA100Id", (caddr_t) &prtsetup.la100_id},
	{"LA210Id", (caddr_t) &prtsetup.la210_id},
	{"LN03Id", (caddr_t) &prtsetup.ln03_id},
	{"LJ250Id", (caddr_t) &prtsetup.lj250_id},
	{"LJ250lrId", (caddr_t) &prtsetup.lj250lr_id},
	{"LCG01Id", (caddr_t) &prtsetup.lcg01_id},
	{"menubarId", (caddr_t) &prtsetup.menubar_id},
	{"blinkerId", (caddr_t) &prtsetup.blinker_id},
	{"sixelOKId", (caddr_t) &prtsetup.sixel_OK_id},
	{"sixelCancelId", (caddr_t) &prtsetup.sixel_Cancel_id},
	{"sixelHelpId", (caddr_t) &prtsetup.sixel_Help_id},
	{"pagesizeOKId", (caddr_t) &prtsetup.page_size_OK_id},
	{"pagesizeCancelId", (caddr_t) &prtsetup.page_size_Cancel_id},
	{"pagesizeHelpId", (caddr_t) &prtsetup.page_size_Help_id},
	{"pageyornId", (caddr_t) &prtsetup.pageyorn_id},
	{"sixelyornId", (caddr_t) &prtsetup.sixelyorn_id}
    };

    static int reglist_num = XtNumber(reglist);

    main_translations_parsed = XtParseTranslationTable(main_translation_table);
    label_translations_parsed = XtParseTranslationTable(label_translation_table);

    MrmRegisterNames (reglist, reglist_num);

    /* make our action table known to the toolkit */
    XtAppAddActions
	(applicationContext, our_action_table, XtNumber(our_action_table));

    MrmFetchWidget
    (
	s_DRMHierarchy,
	"MainWindow",
	smdata.toplevel,
	&main_window,
	&drm_dummy_class
    );

    MrmFetchWidget
    (
	s_DRMHierarchy,
	"page_size_dialog",
	smdata.toplevel,
	&smdata.pagesize_panel,
	&drm_dummy_class
    );

    MrmFetchWidget
    (
	s_DRMHierarchy,
	"sixel_device_dialog",
	smdata.toplevel,
	&smdata.sixel_panel,
	&drm_dummy_class
    );

    MrmFetchWidget
    (
	s_DRMHierarchy,
	"WorkAreaDialog",
	smdata.toplevel,
	&smdata.message_area,
	&drm_dummy_class
    );


    XtMoveWidget(smdata.toplevel, smsetup.x, smsetup.y);
    initialize_defaults();
    updatesetup();

    /*
    ** The following lines have been removed.  Eliminating color output options
    ** is now performed in the screen-number confirmation routine (printcb.c)
    */
    XtSetArg (args[0], XmNtranslations, main_translations_parsed);
    XtSetValues(smdata.message_area, args, 1);

    XtUnmanageChild(prtsetup.blinker_id);

    {
#ifndef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif
	Dimension   width = 0, temp_width;
	/*
	** Find the width of the buttons in the option menus.
	*/
	XtVaGetValues (prtsetup.postscript_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.sixel_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.bitmap_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);

	XtVaGetValues (prtsetup.positive_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.negative_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);

	XtVaGetValues (prtsetup.portrait_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.landscape_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.best_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);

	XtVaGetValues (prtsetup.color_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.grey_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.bw_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);

	XtVaGetValues (prtsetup.shrink_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.grow_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.scale_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.reduce_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);
	XtVaGetValues (prtsetup.crop_id, XmNwidth, &temp_width, NULL);
	width = MAX (width, temp_width);

	/*
	** Set them all to match the biggest.
	*/
	XtVaSetValues (prtsetup.postscript_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.sixel_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.bitmap_id, XmNwidth, width, NULL);

	XtVaSetValues (prtsetup.positive_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.negative_id, XmNwidth, width, NULL);

	XtVaSetValues (prtsetup.portrait_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.landscape_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.best_id, XmNwidth, width, NULL);

	XtVaSetValues (prtsetup.color_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.grey_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.bw_id, XmNwidth, width, NULL);

	XtVaSetValues (prtsetup.shrink_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.grow_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.scale_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.reduce_id, XmNwidth, width, NULL);
	XtVaSetValues (prtsetup.crop_id, XmNwidth, width, NULL);
    }

    XtManageChild(main_window);

    XtRealizeWidget(smdata.toplevel);

    {
	Widget shell= XtParent(prtsetup.format_id);
	Widget form = XtParent(shell);
	WidgetList children;
	int num_children;
	int i;
	children = DXmChildren ((CompositeWidget)form);
	num_children = DXmNumChildren ((CompositeWidget)form);
	for (i = 0; i < num_children; i++)
	{
	    XtVaSetValues
	    (
		children[i],
		XmNleftAttachment, XmATTACH_NONE,
		XmNrightAttachment, XmATTACH_FORM,
		NULL
	    );
	}
    }

    return(1);
}

XtActionProc map_event
#if _PRDW_PROTO_
(
    Widget	w,
    XEvent	*event,
    String	*params,
    Cardinal	*num_params
)
#else
(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine ensures that the message area label is centered
**	in the dialog box.   To know that it is centered, we need to
**	know the width of the dialog box which is not known until the
**	dialog box is mapped.  Get the width and then attach the label
**	to the left edge of the dialgo box.  A negative offset
**	will move it to the right.  width/4 will make it centered.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
*/
{
    Arg	arglist[20];
    Dimension   count;

    XtSetArg (arglist[0], XmNwidth, &count);
    XtGetValues (w, arglist, 1);
    XtSetArg (arglist[0], XmNleftOffset, (XtArgVal)(-(count/4)));
    XtSetValues(w, arglist, 1);
}

Widget MyCreateWidget
#if _PRDW_PROTO_
(
    String      name,
    WidgetClass widgetClass,
    Widget      parent,
    ArgList     args,
    Cardinal    num_args
)
#else
(name, widgetClass, parent, args, num_args)
    String      name;
    WidgetClass widgetClass;
    Widget      parent;
    ArgList     args;
    Cardinal    num_args;
#endif
{
    Widget	retwid = 0;

    if (num_args == 0)
	MrmFetchWidget
	    (s_DRMHierarchy, name, parent, &retwid, &drm_dummy_class);
    else
	MrmFetchWidgetOverride
	(
	    s_DRMHierarchy,
	    name,
	    parent,
	    name,
	    args,
	    num_args,
	    &retwid,
            &drm_dummy_class
	);
    return (retwid);
}

int move_event()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the control panel is moved.  We want to save the
**	current position of the control panel for the next time the user
**	logs in.  This routine will get the x and y of the 
**	moved resized dialog box and save them in the resource database.
**	This database is then written to the session manager resource
**	file if the user saves settings when ending the session.
**
**  FORMAL PARAMETERS:
**
**	standard callback parameters not used.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Arg arglist[5];
unsigned	int	ivalue,status;
char	astring[10];	
Position    x = 0;
Position    y = 0;


/* Get the current width and height of the dialog box */
XtSetArg (arglist[0], XmNx, &x);
XtSetArg (arglist[1], XmNy, &y);
XtGetValues (smdata.toplevel, arglist, 2);

if (x != smsetup.x)
    {
    /* If the width has changed, then save the new resource */
    smsetup.x = x;
    status = int_to_str(smsetup.x, astring, sizeof(astring));
    if (status == SS$_NORMAL)
	{
	sm_put_resource(smx, astring);
	}
   /* Setting this variable means that the user will be asked if they
      want to save their settings when they end the session */
    smdata.resource_changed = 1;
    }

if (y != smsetup.y)
    {
    /* If the height has changed, then save the new resource */
    smsetup.y = y;
    status = int_to_str(smsetup.y, astring, sizeof(astring));
    if (status == SS$_NORMAL)
	{
	sm_put_resource(smy, astring);
	}
   /* Setting this variable means that the user will be asked if they
      want to save their settings when they end the session */
    smdata.resource_changed = 1;
    }

return;
}

XtActionProc configure_event
#if _PRDW_PROTO_
(
    Widget	widget,
    XEvent	*event,
    char	*params,
    Cardinal	*num_params
)
#else
(widget, event, params, num_params)
    Widget	widget;
    XEvent	*event;
    char	*params;
    Cardinal	*num_params;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the control panel is resized.  We want to save the
**	current size of the control panel for the next time the user
**	logs in.  This routine will get the width and height of the 
**	resized dialog box and save them in the resource database.
**	This database is then written to the session manager resource
**	file if the user saves settings when ending the session.
**
**  FORMAL PARAMETERS:
**
**	standard callback parameters not used.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Arg arglist[5];
unsigned long cols = 0;
unsigned long rows = 0;
unsigned	int	ivalue,status;
char	astring[10];	


/* Get the current width and height of the dialog box */
XtSetArg (arglist[0], XmNheight, &rows);
XtSetArg (arglist[1], XmNwidth, &cols);
XtGetValues (XtParent(smdata.message_area), arglist, 2);

if (rows != smsetup.rows)
    {
    /* If the width has changed, then save the new resource */
    smsetup.rows = rows;
    status = int_to_str(smsetup.rows, astring, sizeof(astring));
    if (status == SS$_NORMAL)
	{
	sm_put_resource(smrows, astring);
	}
   /* Setting this variable means that the user will be asked if they
      want to save their settings when they end the session */
    smdata.resource_changed = 1;
    }

if (cols != smsetup.cols)
    {
    /* If the height has changed, then save the new resource */
    smsetup.cols = cols;
    status = int_to_str(smsetup.cols, astring, sizeof(astring));
    if (status == SS$_NORMAL)
	{
	sm_put_resource(smcols, astring);
	}
   /* Setting this variable means that the user will be asked if they
      want to save their settings when they end the session */
    smdata.resource_changed = 1;
    }

return;
}
