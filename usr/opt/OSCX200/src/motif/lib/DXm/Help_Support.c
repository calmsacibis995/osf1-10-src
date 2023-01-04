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
*/

/*
**++
**  Subsystem:
**	DXmHelp 
**
**  Version: V1.0
**
**  Abstract:
**	provides high level calls for using the Help Widget
**
**  Keywords:
**	none
**
**  Environment:
**	User mode, executable image
**
**  Author:
**	Andre Pavanello
**
**  Creation Date: 18-Mar-88
**
**  Modification History:
**
**		Rich					        23-Jul-90
**	    Remove DXm/ from private include file includes
**
**              Leo						02-Aug-89
**	    Q&D port to Motif
**
**  B-rel.1	Andre Pavanello					19-Jan-89
**	    Add new support routines for Help Widget attribute propagations
**	    
**  BL11.1	Andre Pavanello					11-Oct-88
**	    rewrite the argument passing
**	    use XtMalloc and XtFree only
**
**  X004-1-3	Andre Pavanello					21-Apr-88
**	    Change conditional compilation statements for vcc
**	    Change include file to be all lowercase
**
**  X004-1-2	Andre Pavanello					13-Apr-88
**	    fix pointer when inserting new widget in list
**
**  X004-1-1	Andre Pavanello					12-Apr-88
**	    Convert to compound string
**	    Add the context parameter
**--
*/

/*
**  Include Files
*/
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "DXmPrivate.h"
#include <DXm/DXmHelpBP.h>
#include "Help_Support.h"

/*
**  Macro Definitions
*/
#define ARGLIST_SIZE 60
    
/*
**  Table of Contents
*/
       void DXmHelpSetup();
       void DXmHelpDisplay();
       void DXmHelpCleanup();
       void HelpSetup();
       void HelpDisplay();
       void HelpPropagateArgs();
static void DXmHelpSetup_Lowlevel();
static void HelpSetup_Lowlevel();
static void DXmHelpDisplay_Lowlevel();
static void DXmHelpCleanup_Lowlevel();
static void LoadParentHelpArgs();
static void unmap();
static void insert_widget_id();
static void set_widget_unmapped();
static Widget get_widget_id();



void DXmHelpSetup(cntxt, par_widget_id, appl, lib_typ, lib, over, gloss)
HELP_CONTEXT_BLOCK  **cntxt;
Widget		    par_widget_id;
XmString	    appl;
int		    lib_typ;
XmString	    lib,
		    over,
		    gloss;
/*
**++
**  Functional Description:
**	sets all the parameter for the Help Widget
**
**  Keywords:
**	setup, widget, DECwindows
**
**  Arguments:
**	cntxt   : address of the help context pointer
**	par_widget_id : the parent widget id
**	appl	: address of a application name string
**	lib_typ : the Help Library type
**	lib	: address of a library spec string
**	over	: address of an overview frame string
**	gloss	: address of a glossary frame string
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    HELP_CONTEXT_BLOCK *context;

    context = (HELP_CONTEXT_BLOCK *) XtMalloc(sizeof(HELP_CONTEXT_BLOCK));
    *cntxt = context;

    context->help_list = NULL;
    context->application = NULL;
    context->library = NULL;
    context->overview = NULL;
    context->glossary = NULL;    

    if (appl != NULL)
	context->application = XmStringCopy(appl);

    context->library_type = lib_typ;

    if (lib != NULL)
	context->library = XmStringCopy(lib);
	
    if (over != NULL)
	context->overview = XmStringCopy(over);
	
    if (gloss != NULL)
	context->glossary = XmStringCopy(gloss);
    
    context->parent_id = par_widget_id;

    DXmHelpSetup_Lowlevel(context);
    
    return;
    }

void DXmHelpDisplay(cntxt, topic)
HELP_CONTEXT_BLOCK *cntxt;
XmString      topic;
/*
**++
**  Functional Description:
**	displays a Help Topic
**
**  Keywords:
**	display, widget, DECwindows
**
**  Arguments:
**	cntxt	    : address of the help context block
**	topic	    : address of a topic string
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    
    DXmHelpDisplay_Lowlevel(cntxt, topic);
    
    return;
    }

void DXmHelpCleanup(cntxt)
HELP_CONTEXT_BLOCK *cntxt;
/*
**++
**  Functional Description:
**	destroys all the reusable widgets and frees global storage
**
**  Keywords:
**	destroy, help, DECwindows
**
**  Arguments:
**	cntxt	: address of an help context block
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    
    DXmHelpCleanup_Lowlevel(cntxt);
    
    return;
    }




#ifdef DWTVMS	    /* VMS bindings */

void DXMHELP$SETUP(cntxt, par_widget_id, appl, lib_typ, lib,  over, gloss)
HELP_CONTEXT_BLOCK      **cntxt;
Widget			par_widget_id;
XmString		appl;
int			*lib_typ;
XmString		lib,
			over,
			gloss;
/*
**++
**  Functional Description:
**	sets all the parameter for the Help Widget
**
**  Keywords:
**	setup, widget, DECwindows
**
**  Arguments:
**	cntxt	    : address of an help context block pointer
**	par_widget_id : the parent widget id
**	appl	    : address of a application name compound string
**	lib_typ	    : address of the Help Library type
**	lib	    : address of a library spec compound string
**	over	    : address of an overview name compound string
**	gloss	    : address of a glossary frame compound string
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    HELP_CONTEXT_BLOCK *context;
    
    context = (HELP_CONTEXT_BLOCK *) XtMalloc(sizeof(HELP_CONTEXT_BLOCK));
    *cntxt = context;

    context->help_list = NULL;
    context->application = NULL;
    context->library = NULL;
    context->overview = NULL;
    context->glossary = NULL;    

    if (appl != NULL)
	context->application = XmStringCopy(appl);

    context->library_type = *lib_typ;

    if (lib != NULL)
	context->library = XmStringCopy(lib);
	
    if (over != NULL)
	context->overview = XmStringCopy(over);
	
    if (gloss != NULL)
	context->glossary = XmStringCopy(gloss);
    
    context->parent_id = par_widget_id;

    DXmHelpSetup_Lowlevel(context);
    
    return;
    }

void DXMHELP$DISPLAY(cntxt, topic)
HELP_CONTEXT_BLOCK *cntxt;
XmString	    topic;
/*
**++
**  Functional Description:
**	displays a Help Topic
**
**  Keywords:
**	display, widget, DECwindows
**
**  Arguments:
**	cntxt	: address of an help context block
**	topic   : address of a topic compound string
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    
    DXmHelpDisplay_Lowlevel(cntxt, topic);
    
    return;
    }

void DXMHELP$CLEANUP(cntxt)
HELP_CONTEXT_BLOCK *cntxt;
/*
**++
**  Functional Description:
**	destroys all the reusable widgets and frees global storage
**
**  Keywords:
**	destroy, help, DECwindows
**
**  Arguments:
**	cntxt	: address of an help context block
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    
    DXmHelpCleanup_Lowlevel(cntxt);

    return;
    }



#endif		/* VMS bindings */

static void DXmHelpSetup_Lowlevel(cntxt)
HELP_CONTEXT_BLOCK *cntxt;
/*
**++
**  Functional Description:
**	sets all the parameter for the Help Widget
**
**  Keywords:
**	setup, widget, DECwindows
**
**  Arguments:
**	cntxt : the address of the help context block
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    int		mapped = FALSE;
    Widget	help_widget_id;
    Arg		arglist[7];
    XtCallbackRec	unmap_callback[2];
/*
**  Setup callback structure for unmap
*/
    unmap_callback[0].callback = (XtCallbackProc) unmap;
    unmap_callback[0].closure = (XtPointer) cntxt;
    unmap_callback[1].callback = NULL;
/*    
**  Load the argument list for widget creation. The initial topic is set
**  to NULL.
*/
    XtSetArg (arglist[0], DXmNapplicationName, cntxt->application);
    XtSetArg (arglist[1], DXmNlibrarySpec, cntxt->library);
    XtSetArg (arglist[2], DXmNlibraryType, cntxt->library_type);
    XtSetArg (arglist[3], DXmNfirstTopic, NULL);
    XtSetArg (arglist[4], DXmNoverviewTopic, cntxt->overview);
    XtSetArg (arglist[5], DXmNglossaryTopic, cntxt->glossary);
    XtSetArg (arglist[6], XmNunmapCallback, unmap_callback);
/*		
**  Create the initial Help Widget
*/
    help_widget_id = DXmCreateHelp(cntxt->parent_id, "Help", arglist, 7);
/*
**  Make the newly created widget id available for reuse i.e. not mapped
*/
    insert_widget_id(cntxt, help_widget_id, mapped);

    return;
    }

static void DXmHelpDisplay_Lowlevel(cntxt, topic)
HELP_CONTEXT_BLOCK *cntxt;
XmString topic;
/*
**++
**  Functional Description:
**	displays a Help Topic
**
**  Keywords:
**	display, widget, DECwindows
**
**  Arguments:
**	cntxt	    : address of the help context block
**	topic	    : address of a topic string
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    int         mapped_widget = TRUE;
    Widget	help_widget_id;
    Arg		arglist[7];
    XtCallbackRec unmap_callback[2];
/*
**  Setup callback structure for unmap
*/
    unmap_callback[0].callback = (XtCallbackProc) unmap;
    unmap_callback[0].closure = (XtPointer) cntxt;
    unmap_callback[1].callback = NULL;
/*
**  Get an unmapped widget id from the list
*/
    help_widget_id = get_widget_id(cntxt);
    
    if (help_widget_id == NULL)
	{
/*
**	The list of reusable widgets is empty, so create a new one
*/
	XtSetArg (arglist[0], DXmNapplicationName, cntxt->application);
	XtSetArg (arglist[1], DXmNlibrarySpec, cntxt->library);
	XtSetArg (arglist[2], DXmNlibraryType, cntxt->library_type);
	XtSetArg (arglist[3], DXmNfirstTopic, topic);
	XtSetArg (arglist[4], DXmNoverviewTopic, cntxt->overview);
	XtSetArg (arglist[5], DXmNglossaryTopic, cntxt->glossary);
	XtSetArg (arglist[6], XmNunmapCallback, unmap_callback);
		
	help_widget_id = DXmCreateHelp(cntxt->parent_id, "Help", arglist, 7);
/*	
**	insert the widget id in the list
*/
	insert_widget_id(cntxt, help_widget_id, mapped_widget);
	}
    else
	{
/*
**	reuse an existing widget
*/
	XtSetArg(arglist[0], DXmNfirstTopic, topic);
	XtSetValues(help_widget_id, arglist, 1);
	}

    XtManageChild(help_widget_id);

    return;
    }

static void DXmHelpCleanup_Lowlevel(cntxt)
HELP_CONTEXT_BLOCK *cntxt;
/*
**++
**  Functional Description:
**	destroys all the reusable widgets and frees memory
**
**  Keywords:
**	destroy, help, DECwindows
**
**  Arguments:
**	cntxt : address of a help context block
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    WIDGET_ID_LIST *next_item,
		   *list_ptr;

    if (cntxt != NULL)
	{
/*
**	run through the list of widget ids
*/
	list_ptr = cntxt->help_list;
	while (list_ptr != NULL)
	    {
	    if (list_ptr->widget_id != NULL)
		{
/*
**		Destroy the widget
*/
		XtDestroyWidget(list_ptr->widget_id);
		}
	    next_item = list_ptr->next;
/*
**	    Free the list element
*/
	    XtFree((char *)list_ptr);
	    list_ptr = next_item;
	    }
/*	    
**	Free the content of the help context block
*/
	if (cntxt->application != NULL) XtFree((char *)cntxt->application);
	if (cntxt->library != NULL)     XtFree((char *)cntxt->library);
	if (cntxt->overview != NULL)    XtFree((char *)cntxt->overview);
	if (cntxt->glossary != NULL)    XtFree((char *)cntxt->glossary);
/*	
**	Free the context block itself
*/
	XtFree((char *)cntxt);
	}

    return;
    }

void HelpSetup(cntxt, par_widget_id, appl, lib_typ, lib, over, gloss)
HELP_CONTEXT_BLOCK  **cntxt;
Widget		    par_widget_id;
XmString	    appl;
int		    lib_typ;
XmString	    lib,
		    over,
		    gloss;
/*
**++
**  Functional Description:
**	sets all the parameter for the Help Widget
**
**  Keywords:
**	setup, widget, DECwindows
**
**  Arguments:
**	cntxt   : address of the help context pointer
**	par_widget_id : the parent widget id
**	appl	: address of a application name string
**	lib_typ : the Help Library type
**	lib	: address of a library spec string
**	over	: address of an overview frame string
**	gloss	: address of a glossary frame string
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    HELP_CONTEXT_BLOCK *context;

    context = (HELP_CONTEXT_BLOCK *) XtMalloc(sizeof(HELP_CONTEXT_BLOCK));
    *cntxt = context;

    context->help_list = NULL;
    context->application = NULL;
    context->library = NULL;
    context->overview = NULL;
    context->glossary = NULL;    

    if (appl != NULL)
	context->application = XmStringCopy(appl);

    context->library_type = lib_typ;

    if (lib != NULL)
	context->library = XmStringCopy(lib);
	
    if (over != NULL)
	context->overview = XmStringCopy(over);
	
    if (gloss != NULL)
	context->glossary = XmStringCopy(gloss);
    
    context->parent_id = par_widget_id;

    HelpSetup_Lowlevel(context);
    
    return;
    }

static void HelpSetup_Lowlevel(cntxt)
HELP_CONTEXT_BLOCK *cntxt;
/*
**++
**  Functional Description:
**	sets all the parameter for the Help Widget
**
**  Keywords:
**	setup, widget, DECwindows
**
**  Arguments:
**	cntxt : the address of the help context block
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    int		mapped = FALSE;
    int		ac = 0;
    Widget	help_widget_id;
    DXmHelpWidget parent_help;
    Arg		arglist[ARGLIST_SIZE];
    XtCallbackRec	unmap_callback[2];
    
    /*
    **  Setup callback structure for unmap
    */
    
    unmap_callback[0].callback = (XtCallbackProc) unmap;
    unmap_callback[0].closure = (XtPointer) cntxt;
    unmap_callback[1].callback = NULL;
    
    /*
    ** Get the parent help widget's resources into the arglist.
    ** 
    **  **** make sure the arglist is BIG enough *****
    */

    LoadParentHelpArgs(cntxt->parent_id, arglist, &ac);

    /*    
    **  Load the argument list for widget creation. The initial topic is set
    **  to NULL.
    */
    
    XtSetArg (arglist[ac], DXmNapplicationName, cntxt->application); ac++;
    XtSetArg (arglist[ac], DXmNlibrarySpec, cntxt->library); ac++;
    XtSetArg (arglist[ac], DXmNlibraryType, cntxt->library_type); ac++;
    XtSetArg (arglist[ac], DXmNfirstTopic, NULL); ac++;
    XtSetArg (arglist[ac], DXmNoverviewTopic, cntxt->overview); ac++;
    XtSetArg (arglist[ac], DXmNglossaryTopic, cntxt->glossary); ac++;
    XtSetArg (arglist[ac], XmNunmapCallback, unmap_callback); ac++;
    
    /*		
    **  Create the initial Help Widget
    */
    
    help_widget_id = DXmCreateHelp(cntxt->parent_id, "Help", arglist, ac);
    
    /*
    **  Make the newly created widget id available for reuse
    */
    
    insert_widget_id(cntxt, help_widget_id, mapped);

    return;
    }

void HelpDisplay(cntxt, topic)
HELP_CONTEXT_BLOCK *cntxt;
XmString      topic;
/*
**++
**  Functional Description:
**	displays a Help Topic
**
**  Keywords:
**	display, widget, DECwindows
**
**  Arguments:
**	cntxt	    : address of the help context block
**	topic	    : address of a topic string
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    int         mapped_widget = TRUE;
    int		ac = 0;
    Widget	help_widget_id;
    Arg		arglist[ARGLIST_SIZE];
    XtCallbackRec unmap_callback[2];
    
    /*
    **  Setup callback structure for unmap
    */
    
    unmap_callback[0].callback = (XtCallbackProc) unmap;
    unmap_callback[0].closure = (XtPointer) cntxt;
    unmap_callback[1].callback = NULL;
    
    /*
    **  Get an unmapped widget id from the list
    */
    
    help_widget_id = get_widget_id(cntxt);
    
    if (help_widget_id == NULL)
	{
	
	/*
	**  The list of reusable widgets is empty, so create a new one.
	**  
        **  Get the parent help widget's resources into the arglist.
        ** 
        **  **** make sure the arglist is BIG enough *****
        */

        LoadParentHelpArgs(cntxt->parent_id, arglist, &ac);
	
	XtSetArg (arglist[ac], DXmNapplicationName, cntxt->application); ac++;
	XtSetArg (arglist[ac], DXmNlibrarySpec, cntxt->library); ac++;
	XtSetArg (arglist[ac], DXmNlibraryType, cntxt->library_type); ac++;
	XtSetArg (arglist[ac], DXmNfirstTopic, topic); ac++;
	XtSetArg (arglist[ac], DXmNoverviewTopic, cntxt->overview); ac++;
	XtSetArg (arglist[ac], DXmNglossaryTopic, cntxt->glossary); ac++;
	XtSetArg (arglist[ac], XmNunmapCallback, unmap_callback); ac++;
		
	help_widget_id = DXmCreateHelp(cntxt->parent_id, "Help", arglist, ac);
	
        /*	
        **  insert the widget id in the list
        */
	
	insert_widget_id(cntxt, help_widget_id, mapped_widget);
	}
    else
	{
	
	/*
	**  reuse an existing widget
	*/
	
	XtSetArg(arglist[0], DXmNfirstTopic, topic);
	XtSetArg(arglist[1], DXmNlibrarySpec, cntxt->library);
	XtSetValues(help_widget_id, arglist, 2);
	}

    XtManageChild(help_widget_id);

    return;
    }


void HelpPropagateArgs(context, arglist, argcount)
HELP_CONTEXT_BLOCK *context;
Arg arglist[];
int argcount;
/*
**++
**  Functional Description:
**	propagates arguments to the list of widgets
**
**  Keywords:
**	propagate, widget, DECwindows
**
**  Arguments:
**	cntxt	    : address of the help context block
**	arglist	    : the argument list to be propagated
**	argcount    : the number of arguments in the the arglist
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    WIDGET_ID_LIST *list;
	
    if (context != NULL)
	{
	list = context->help_list;
	
	/*
	**  Traverse the list of widgets
	*/
	
	while (list != NULL)
	    {
	    XtSetValues(list->widget_id, arglist, argcount);
	    list = list->next;
	    }
	}    
    
    return;
    }


static void LoadParentHelpArgs(parent_help, arglist, argcount)
DXmHelpWidget parent_help;
Arg *arglist;
int *argcount;
/*
**++
**  Functional Description:
**	loads the arglist of the help widget's parent
**
**  Keywords:
**	help, widget, DECwindows
**
**  Arguments:
**	parent_help : a help widget id
**	arglist : the argument list to load
**	argcount : the argument count
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    int ac = 0;
    
    /*
    **	50 arguments. *** Check if ARGLIST_SIZE is big enough ***
    */
    
    XtSetArg (arglist[ac], DXmNcols,
	    parent_help->dxmhelp.colons); ac++;
    XtSetArg (arglist[ac], DXmNrows,
	    parent_help->dxmhelp.rows); ac++;
    XtSetArg (arglist[ac], XmNbuttonFontList, 
	    parent_help->dxmhelp.button_font_list); ac++;
    XtSetArg (arglist[ac], XmNlabelFontList, 
	    parent_help->dxmhelp.label_font_list); ac++;
    XtSetArg (arglist[ac], XmNtextFontList, 
	    parent_help->dxmhelp.text_font_list); ac++;
    XtSetArg (arglist[ac], XmNstringDirection, 
	    parent_help->dxmhelp.string_direction); ac++;
    XtSetArg (arglist[ac], DXmNviewLabel,
	    parent_help->dxmhelp.view_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNgototopicLabel,
	    parent_help->dxmhelp.goto_topic_label); ac++;
    XtSetArg (arglist[ac], DXmNgobacktopicLabel,
	    parent_help->dxmhelp.goback_topic_label); ac++;
    XtSetArg (arglist[ac], DXmNgooverLabel,
	    parent_help->dxmhelp.goover_label); ac++;
    XtSetArg (arglist[ac], DXmNvisittopicLabel,
	    parent_help->dxmhelp.visit_topic_label); ac++;
    XtSetArg (arglist[ac], DXmNvisitglosLabel,
	    parent_help->dxmhelp.visitglos_label); ac++;
    XtSetArg (arglist[ac], DXmNfileLabel,
	    parent_help->dxmhelp.file_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNsaveasLabel,
	    parent_help->dxmhelp.saveas_label); ac++;
    XtSetArg (arglist[ac], DXmNexitLabel,
	    parent_help->dxmhelp.exit_label); ac++;
    XtSetArg (arglist[ac], DXmNeditLabel,
	    parent_help->dxmhelp.edit_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNcopyLabel,
	    parent_help->dxmhelp.copy_label); ac++;
    XtSetArg (arglist[ac], DXmNselectallLabel,
	    parent_help->dxmhelp.selectall_label); ac++;
    XtSetArg (arglist[ac], DXmNsearchLabel,
	    parent_help->dxmhelp.search_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNhistoryLabel,
	    parent_help->dxmhelp.history_label); ac++;
    XtSetArg (arglist[ac], DXmNtitleLabel,
	    parent_help->dxmhelp.title_label); ac++;
    XtSetArg (arglist[ac], DXmNkeywordLabel,
	    parent_help->dxmhelp.keyword_label); ac++;
    XtSetArg (arglist[ac], DXmNhelpLabel,
	    parent_help->dxmhelp.help_menu_label); ac++;
    XtSetArg (arglist[ac], DXmNhelphelpLabel,
	    parent_help->dxmhelp.helphelp_label); ac++;

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNglossaryLabel,
	    parent_help->dxmhelp.glossary_label); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

#if 0	/* Remove everything except On Windows from Using Help Menu */
    XtSetArg (arglist[ac], DXmNaboutLabel,
	    parent_help->dxmhelp.about_label); ac++;
#endif 	/* Remove everything except On Windows from Using Help Menu */

    XtSetArg (arglist[ac], DXmNaddtopicLabel,
	    parent_help->dxmhelp.addtopic_label); ac++;
    XtSetArg (arglist[ac], DXmNgobackLabel,
	    parent_help->dxmhelp.goback_label); ac++;
    XtSetArg (arglist[ac], DXmNcloseLabel,
	    parent_help->dxmhelp.quit_label); ac++;
    XtSetArg (arglist[ac], DXmNgotoLabel,
	    parent_help->dxmhelp.goto_label); ac++;
    XtSetArg (arglist[ac], DXmNvisitLabel,
	    parent_help->dxmhelp.visit_label); ac++;
    XtSetArg (arglist[ac], DXmNsearchapplyLabel,
	    parent_help->dxmhelp.apply_label); ac++;
    XtSetArg (arglist[ac], DXmNdismissLabel,
	    parent_help->dxmhelp.dismiss_label); ac++;
    XtSetArg (arglist[ac], DXmNtopictitlesLabel,
	    parent_help->dxmhelp.topic_titles_label); ac++;
    XtSetArg (arglist[ac], DXmNhistoryboxLabel,
	    parent_help->dxmhelp.history_box_label); ac++;
    XtSetArg (arglist[ac], DXmNsearchtitleboxLabel,
	    parent_help->dxmhelp.searchtitle_box_label); ac++;
    XtSetArg (arglist[ac], DXmNtitlesLabel,
	    parent_help->dxmhelp.titles_label); ac++;
    XtSetArg (arglist[ac], DXmNsearchkeywordboxLabel,
	    parent_help->dxmhelp.searchkeyword_box_label); ac++;
    XtSetArg (arglist[ac], DXmNkeywordsLabel,
	    parent_help->dxmhelp.keywords_label); ac++;
    XtSetArg (arglist[ac], DXmNbadlibMessage,
	    parent_help->dxmhelp.badlib_message); ac++;
    XtSetArg (arglist[ac], DXmNbadframeMessage,
	    parent_help->dxmhelp.badframe_message); ac++;
    XtSetArg (arglist[ac], DXmNnulllibMessage,
	    parent_help->dxmhelp.nulllib_message); ac++;
    XtSetArg (arglist[ac], DXmNnotitleMessage,
	    parent_help->dxmhelp.notitle_message); ac++;
    XtSetArg (arglist[ac], DXmNnokeywordMessage,
	    parent_help->dxmhelp.nokeyword_message); ac++;
    XtSetArg (arglist[ac], DXmNerroropenMessage,
	    parent_help->dxmhelp.erroropen_message); ac++;
    XtSetArg (arglist[ac], DXmNhelpontitleLabel,
	    parent_help->dxmhelp.helpontitle_label); ac++;
    XtSetArg (arglist[ac], DXmNhelptitleLabel,
	    parent_help->dxmhelp.helptitle_label); ac++;
    XtSetArg (arglist[ac], DXmNhelpAcknowledgeLabel,
	    parent_help->dxmhelp.acknowledge_label); ac++;
    XtSetArg (arglist[ac], DXmNhelpOnHelpTitle,
	    parent_help->dxmhelp.helponhelp_title); ac++;
    XtSetArg (arglist[ac], DXmNcacheHelpLibrary,
	    parent_help->dxmhelp.cache_library); ac++;
    *argcount = ac;

    return;
    }

static void unmap(help_widget, cntxt_ptr)
Widget         help_widget;
HELP_CONTEXT_BLOCK *cntxt_ptr;
/*
**++
**  Functional Description:
**	unmaps a widget (i.e. makes it reusable)
**
**  Keywords:
**	unmap, help, widget, DECwindows
**
**  Arguments:
**	help_widget   : a widget id
**	cntxt_ptr     : address of a help context block
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {

    set_widget_unmapped(cntxt_ptr, help_widget);

    return;
    }

static void insert_widget_id(cntxt, widget_id, mapped)
HELP_CONTEXT_BLOCK  *cntxt;
Widget		    widget_id;
int		    mapped;
/*
**++
**  Functional Description:
**	inserts a widget id in a list
**
**  Keywords:
**	insert, widget, DECwindows
**
**  Arguments:
**	cntxt	    : address of a pointer to a help context block
**	widget_id   : a widget id
**	mapped	    : indicates if the widget is mapped
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    WIDGET_ID_LIST *list_ptr;
/*
**  Allocate a new element
*/
    list_ptr = (WIDGET_ID_LIST *) XtMalloc(sizeof(WIDGET_ID_LIST));
/*
**  store the widget id
*/
    list_ptr->widget_id = widget_id;
    list_ptr->mapped = mapped;
/*
**  Update the pointer to the list
*/
    list_ptr->next = cntxt->help_list;
    cntxt->help_list = list_ptr;

    return;
    }

static Widget get_widget_id(cntxt)
HELP_CONTEXT_BLOCK *cntxt;
/*
**++
**  Functional Description:
**	retrieves a unmapped widget id from the list or returns a NULL widget id
**	if none were found
**
**  Keywords:
**	retrieve, widget, DECwindows
**
**  Arguments:
**	cntxt : address of an help context block
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    WIDGET_ID_LIST *list_ptr;
    Widget widget_id = NULL;

    list_ptr = cntxt->help_list;

    while ((list_ptr != NULL) && (widget_id == NULL))
	{
	if (!list_ptr->mapped)
	    {
	    widget_id = list_ptr->widget_id;
	    list_ptr->mapped = TRUE;
	    }
	else
	    list_ptr = list_ptr->next;
	}
	
    return (widget_id);
    }


static void set_widget_unmapped(cntxt, widget_id)
HELP_CONTEXT_BLOCK *cntxt;
Widget		 widget_id;
/*
**++
**  Functional Description:
**	sets a widget id as unmapped in the list
**
**  Keywords:
**	set, widget, DECwindows
**
**  Arguments:
**	list_ptr : address to a list of widgets
**	widget_id : a widget id
**
**  Result:
**	none
**
**  Exceptions:
**	none
**--
*/
    {
    int found = FALSE;
    WIDGET_ID_LIST *list_ptr;

    list_ptr = cntxt->help_list;
/*    
**  Run down the list of stored widget id's
*/
    while ((list_ptr != NULL)  && (!found))
	if (list_ptr->widget_id == widget_id)
	    {
	    found = TRUE;
	    list_ptr->mapped = FALSE;
	    }
	else
	    list_ptr = list_ptr->next;

    return;
    }
