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
**	This module provides the routines to create the shell widget for the
**	Help widget (copied from the Hidden Shell widget L. Treggiari)
**
**  Version: X0.1
**
**  Keywords:
**	Help
**
**  Authors:
**	Patricia Avigdor, CASEE group, MEMEX project
**
**  Creation Date: 02-Dec-1987
**
**  Modification History:
**	X0.1	29-Dec-87   Pat	    BL6.2 merge
**	X0.1	21-Jan-88   Pat	    Bl6.4 merge
**	X0.1	09-Feb-88   Pat	    work on positionning
**	X0.1	18-Feb-88   Pat	    add unmap callback
**	X0.1	25-Feb-88   Pat	    fix typecasting bug
**		07-Mar-88   Pat	    FT1 intrinsics updates
**		12-Jan-89   Pat	    add map callback
**	B-BL2-0 17-Mar-89   Andr   remove NOT_VMS_V1 conditional, remove the
**				    else part
**		02-Aug-89   Leo     Q&D port to Motif
**		26-Nov-90   Rich    Add a GeometryManager and ConfigureWindow
**				    routines.
**				    Without them, you can't set XtNx and XtNy on
**				    Help widget and have it work correctly.
**
**				    In managed_set_changed, reset the help
**				    widgets x,y back to 0,0 after the helpshell
**				    has been moved.
**		03-Feb-93   Rich    Convert static error message strings to
**				    character arrays.
**--
*/

/*
**  Include Files
*/

#define HELPSHELL

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/ShellP.h>
#include <X11/VendorP.h>
#include <Xm/Xm.h>
#include "DXmPrivate.h"
#include <DXm/DXmHelpSP.h>
#include <DXm/DXmHelpBP.h>
#include "DXmMessI.h"

/*---------------------------------------------------*/
/* Message definitions for character arrays	     */
/*---------------------------------------------------*/
#define HELPSHLONEKID	 _DXmMsgHlpShl_0000
#define HELPSHLMSG0	 _DXmMsgHlpShlName_0000

#define CALLBACK(w,which,why,evnt)		\
{						\
    if (XtIsSubclass ((Widget)w, dxmHelpWidgetClass))	\
    {						\
        XmAnyCallbackStruct temp;		\
        temp.reason = why;			\
        temp.event  = evnt;			\
	XtCallCallbacks ((Widget)w, which, &temp);	\
    }						\
}


/*
**  External declarations
*/
extern void DXmPositionWidget();

/*
**  Forward declarations
*/

static void Initialize();
static void Realize();
static void add_child();
static void delete_child();
static void managed_set_changed();
static void ConfigureWindow();
static XtGeometryResult GeometryManager();


/*
** Widget resources
*/
static Boolean true = TRUE;
static Bool false = FALSE;

static XtResource helpshellresources[]=
{
    { XtNtransient, XtCTransient, XtRBoolean, sizeof(Boolean),
    XtOffset(DXmHelpShellWidget, wm.transient), XtRBoolean, (XtPointer) &true},
/*    { XtNnoResize, XtCNoResize,  XtRLongBoolean, sizeof(Bool),
    XtOffset(DXmHelpShellWidget, vendor.dechints.no_resize_button), 
    XtRLongBoolean, (char *) &false},        */
};
externaldef(dxmhelpshellclassrec) DXmHelpShellClassRec dxmHelpShellClassRec =
{
    {
	/* superclass         */    (WidgetClass) &topLevelShellClassRec,
	/* class_name         */    "DXmHelpShell",
	/* size               */    sizeof(DXmHelpShellWidgetRec),
	/* Class Initialize   */    NULL,
	/* Class_part init    */    NULL,
	/* Class init'ed ?    */    FALSE,
	/* initialize         */    Initialize,			
	/* initialize_hook    */    NULL,
	/* realize            */    Realize,		
	/* actions            */    NULL,
	/* num_actions        */    0,
	/* resources          */    helpshellresources,
	/* resource_count     */    XtNumber(helpshellresources),
	/* xrm_class          */    NULLQUARK,
	/* compress_motion    */    FALSE,
	/* compress_exposure  */    TRUE,
	/* compress_enterleave*/    FALSE,
	/* visible_interest   */    TRUE,
	/* destroy            */    NULL,
	/* resize             */    XtInheritResize,		/* from Shell */
	/* expose             */    NULL,		
	/* set_values         */    NULL,			
	/* set_values_hook    */    NULL,
	/* set_values_almost  */    XtInheritSetValuesAlmost,
	/* get_values_hook    */    NULL,
	/* accept_focus       */    NULL,
	/* version	      */    XtVersion,
	/* callback_private   */    NULL,
	/* tm_table	      */    XtInheritTranslations,
	/* query geometry proc*/    XtInheritQueryGeometry,
	/* disp accelerator   */    NULL,
	/* extension	      */    NULL,
    },
    {					    /* composite class record       */
	/* geometry_manager   */    (XtGeometryHandler)GeometryManager,
	/* change_managed     */    managed_set_changed,
	/* insert_child	      */    add_child,
	/* delete_child	      */    delete_child,
	/* extension	      */    NULL,	
    },
    {					    /* shell class record	    */
	/* extension	      */    NULL,
    },
    {					    /* wm shell class record	    */
	/* extension	      */    NULL,
    },
    {					    /* vendor shell class record    */
	/* extension	      */    NULL,
    },
    {					    /* toplevel shell class record  */
	/* extension	      */    NULL,
    },
    {					    /* help shell class record	    */
	/* extension	      */    NULL,
    }
};


/*
 * now make a public symbol that points to this class record
 */

externaldef(dxmhelpshellwidgetclass) DXmHelpShellWidgetClass dxmHelpShellWidgetClass = &dxmHelpShellClassRec;



/*
** Initializes an instance of the help shell widget
*/
static void
Initialize (request, new)
    Widget request;				/* as built by arglist */
    Widget new;					/* as left by superclass */
{
    /*
     * must have some size
     */

    if (XtWidth  (new) <= 0)  XtWidth  (new) = 5;
    if (XtHeight (new) <= 0)  XtHeight (new) = 5;

    /*
     * NOT GOOD:  The intrinsics do not allow
     * manage_children calls to be propogated to the parent's 
     * managed_set_changed proc prior to realization of the parent.  This
     * leads to a chicken-egg problem when trying to use managed/unmanage
     * to control mapping since it isn't normally realized before being
     * managed.... so make sure that all help shell widgets are created
     * as realized widgets.
     */

    XtRealizeWidget (new);
}


/*
** add a child to the help shell widget
*/

static void add_child (w)
    Widget w;
{
    CompositeWidget p = (CompositeWidget) XtParent (w);
    
    if (XtNumChildren (p) > 0) 
    {
      	DXMERROR (HELPSHLMSG0,
		HELPSHLONEKID);
/*
 * can't yet, have to add it

	return;
 */
    }

    if (XtIsRectObj(w))
        (* ((CompositeWidgetClass)compositeWidgetClass)->composite_class.insert_child) (w);
}


/*
** Remove a child from the help shell widget
*/

static void delete_child (w)
    Widget w;
{
    if (XtIsRectObj(w))
        (* ((CompositeWidgetClass)compositeWidgetClass)->composite_class.delete_child) (w);
}


/*
** Manage the help shell children
*/

static void 
managed_set_changed (help_shell)
    DXmHelpShellWidget help_shell;
{
    DXmHelpWidget	help_widget;

    help_widget = (DXmHelpWidget)help_shell->composite.children[0];

    /*
    ** If the help widget is being destroyed return.
    */
    if (help_widget->core.being_destroyed)
	return;
	
    /*
    ** if manage was set to TRUE for the help widget then position it
    ** on the screen
    */
    if (help_widget->core.managed) 
    {
	Arg args[5];
	int argc = 0;
	
	/*
	 * If the shell's child is not yet realized, do it now
	 */
	if (!XtIsRealized(help_widget))
	    XtRealizeWidget((Widget)help_widget);

	/* 
	** Make sure that the shell has the same common parameters as its
	** child.
	*/
	if (XtWidth(help_widget)  != XtWidth(help_shell))
	{
	    XtSetArg(args[argc], XtNwidth, XtWidth(help_widget));
	    argc++;
	}
	if (XtHeight(help_widget) != XtHeight(help_shell))
	{
	    XtSetArg(args[argc], XtNheight, XtHeight(help_widget));
	    argc++;
	}
	if (XtBorderWidth(help_widget) != XtBorderWidth(help_shell))
	{
	    XtSetArg(args[argc], XtNborderWidth, XtBorderWidth(help_widget));
	    argc++;
	}
	

	/*
	** If default position is TRUE, determine the position of the
	** help widget window trying to avoid the parent window and
	** within the screen boundaries.
	*/
			
	if (help_widget->dxmhelp.default_pos)
	{
	    DXmPositionWidget (help_widget, help_widget->dxmhelp.avoid_widget,
			    help_widget->dxmhelp.avoid_count);
	}
	else
	{
	    /* set the help shell to the help widget's position */
	    
	    XtSetArg (args[argc], XtNx, XtX(help_widget));
	    argc++;
	    XtSetArg (args[argc], XtNy, XtY(help_widget));
	    argc++;
	}	    

	if (argc != 0)
	XtSetValues ((Widget)help_shell, args, argc);

	/*  
	 *  Move the window to (-borderwidth, -borderwidth)
	 *  but don't tell the widget.  It thinks it's where
	 *  the shell is...
	 */
	if (help_widget->core.x != (int)(-help_widget->core.border_width) ||
	   help_widget->core.y != (int)(-help_widget->core.border_width))
	    XMoveWindow(XtDisplay(help_widget), XtWindow(help_widget),
			(int)(-help_widget->core.border_width),
			(int)(-help_widget->core.border_width));


	/*
	 * Since the shell has been moved to a new location, reset outselves
	 * to relative 0,0 within the shell.
	 */

	help_widget->core.x = 0;
	help_widget->core.y = 0;

	/*
	** Activate the map callback.
	*/
	CALLBACK (help_widget, XmNmapCallback, XmCR_MAP, NULL);	
	
	/*
	** Now popup the help shell widget
	*/

	XtPopup((Widget)help_shell, help_shell->helpshell.grabkind);
    }
    /*
    ** if manage was set to FALSE popdown the help shell widget
    */
    else
    {
	XtPopdown((Widget)help_shell);
	
	/*
	** Activate the unmap callback.
	*/
	CALLBACK (help_widget, XmNunmapCallback, XmCR_UNMAP, NULL);
	    
    }
}                       

/*---------------------------------------------------*/
/* Realize(w,valuemask,attributes)                   */
/*                                                   */
/* this routine realizes this instance of the        */
/* helpshell					     */
/*---------------------------------------------------*/

static void
Realize(w, valuemask, attributes)
    Widget               w;
    Mask                 *valuemask;
    XSetWindowAttributes *attributes;
{
    attributes->event_mask |= StructureNotifyMask; 

    (* ((VendorShellWidgetClass)vendorShellWidgetClass)->core_class.realize)
					    (w,valuemask,attributes);
}


/************************************************************************
 *
 *  ConfigureWindow
 * 	
 * 		Don't move the position of dialog but update the fields
 *
 ************************************************************************/
static void ConfigureWindow(w, geom)
    Widget 		w;
    XtWidgetGeometry	*geom;
{
    XWindowChanges changes, old;
    Cardinal mask = 0;

    if (geom->request_mode & XtCWQueryOnly)
      return;
#ifdef notdef
    if (geom->request_mode & CWX)
	w->core.x = geom->x;

    if (geom->request_mode & CWY)
	w->core.y = geom->y;

    if (geom->request_mode & CWBorderWidth)
      {
	  w->core.border_width = geom->border_width;
      }
#endif
    if ((geom->request_mode & CWWidth) &&
	(w->core.width != geom->width))
      {
	  changes.width = w->core.width = geom->width;
	  mask |= CWWidth;
      }

    if ((geom->request_mode & CWHeight) &&
	(w->core.height != geom->height))
      {
	  changes.height = w->core.height = geom->height;
	  mask |= CWHeight;
      }

    if (mask != 0) 
      {
	  if (XtIsRealized(w)) 
	    {
		if (XtIsWidget(w))
		  XConfigureWindow(XtDisplay(w), XtWindow(w), mask, &changes);
#ifdef DEBUG	    
		else
		  XtError("gadgets aren't allowed in shell");
#endif /* DEBUG */	    
	    }
      }
}



/************************************************************************
 *
 *  GeometryManager
 *
 *  [ Lifted from DialogShell ]
 *
 ************************************************************************/
static XtGeometryResult GeometryManager( wid, request, reply )
    Widget wid;
    XtWidgetGeometry *request;
    XtWidgetGeometry *reply;
{
#define MAGIC_VAL ((Position)~0L)

    ShellWidget 	shell = (ShellWidget)(wid->core.parent);
    XtWidgetGeometry 	my_request;

    if(!(shell->shell.allow_shell_resize) && XtIsRealized(wid) &&
       (request->request_mode & (CWWidth | CWHeight | CWBorderWidth)))
      return(XtGeometryNo);
    /*
     * because of our klutzy API we mimic position requests on the
     * dialog to ourselves
     */
    my_request.request_mode = 0;
    /* %%% worry about XtCWQueryOnly */
    if (request->request_mode & XtCWQueryOnly)
      my_request.request_mode |= XtCWQueryOnly;
    if (request->request_mode & CWX) {
	if (request->x == MAGIC_VAL)
	  my_request.x = 0;
	else
	  my_request.x = request->x;
	my_request.request_mode |= CWX;
    }
    if (request->request_mode & CWY) {
	if (request->y == MAGIC_VAL)
	  my_request.y = 0;
	else
	  my_request.y = request->y;
	my_request.request_mode |= CWY;
    }
    if (request->request_mode & CWWidth) {
	my_request.width = request->width;
	my_request.request_mode |= CWWidth;
    }
    if (request->request_mode & CWHeight) {
	my_request.height = request->height;
	my_request.request_mode |= CWHeight;
    }
    if (request->request_mode & CWBorderWidth) {
	my_request.border_width = request->border_width;
	my_request.request_mode |= CWBorderWidth;
    }
    if (XtMakeGeometryRequest((Widget)shell, &my_request, NULL)
	== XtGeometryYes)
      {
          ConfigureWindow(wid, &my_request);

	  if (XmIsBulletinBoard(wid))
	      return XtGeometryDone;
	  else
	      return XtGeometryYes;
      } 
    else 
      return XtGeometryNo;
}



/*
 *************************************************************************
 *
 * Public creation entry point
 *
 *************************************************************************
 */

Widget
DXmCreateHelpShell (p, name, al, ac)
    Widget  p;				/* parent widget */
    char    *name;			/*  widget name */
    ArgList al;
    int	    ac;    
{
    return (XtCreatePopupShell(name, (WidgetClass)dxmHelpShellWidgetClass, p, al, ac));
}

