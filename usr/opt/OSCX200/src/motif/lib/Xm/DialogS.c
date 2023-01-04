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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: DialogS.c,v $ $Revision: 1.1.6.6 $ $Date: 1994/01/14 16:44:22 $"
#endif
#endif
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
#ifdef WIN32
#include <Xlib_NT.h>
#endif
#include "XmI.h"
#include <Xm/DialogSP.h>
#include <Xm/DialogSEP.h>
#include <Xm/BaseClassP.h>
#include <Xm/BulletinBP.h>
#include "MessagesI.h"

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
#include <X11/StringDefs.h>
#include <Xm/ScrollBarP.h>
#include <Xm/DrawingAP.h>
#endif


#define MSG1	_XmMsgDialogS_0000

#define MAGIC_VAL ((Position)~0L)

#define HALFDIFF(a, b) ((((Position)a) - ((Position)b))/2)

#define TotalWidth(w)   (XtWidth  (w) + (2 * (XtBorderWidth (w))))
#define TotalHeight(w)  (XtHeight (w) + (2 *(XtBorderWidth (w))))

#define CALLBACK(w,which,why,evnt)		\
{						\
 if (XmIsBulletinBoard(w))	\
   {						\
 XmAnyCallbackStruct temp;		\
   temp.reason = why;			\
     temp.event  = evnt;			\
       XtCallCallbacks (w, which, &temp);	\
     }						\
   }

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#define FTS_WM_RESIZE_BORDER_SIZE       10
#define FTS_WM_TITLE_BAR_HEIGHT         20
#define FTS_WM_WIDTH_ADJUSTMENT         (2 * FTS_WM_RESIZE_BORDER_SIZE)
#define FTS_WM_HEIGHT_ADJUSTMENT        (FTS_WM_WIDTH_ADJUSTMENT + FTS_WM_TITLE_BAR_HEIGHT)

#define FTS_MAX_WIDTH(screen)           (WidthOfScreen(screen) - FTS_WM_WIDTH_ADJUSTMENT)
#define FTS_MAX_HEIGHT(screen)          (HeightOfScreen(screen) - FTS_WM_HEIGHT_ADJUSTMENT)
#define FTS_MANAGE                      0
#define FTS_UNMANAGE                    1

static void             Resize();
static void             Destroy();
static void             VertSliderMove();
static void             HorizSliderMove();

static XtCallbackRec VSCallBack[] =
{
   {(XtCallbackProc )VertSliderMove, (XtPointer) NULL},
   {NULL,           (XtPointer) NULL},
};

static XtCallbackRec HSCallBack[] =
{
   {(XtCallbackProc )HorizSliderMove, (XtPointer) NULL},
   {NULL,           (XtPointer) NULL},
};
#endif



/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void ClassInitialize() ;
static void ClassPartInit() ;
static Widget GetRectObjKid() ;
static void Initialize() ;
static Boolean SetValues() ;
static void InsertChild() ;
static void GetDefaultPosition() ;
static void ChangeManaged() ;
static XtGeometryResult GeometryManager() ;

#else

static void ClassInitialize( void ) ;
static void ClassPartInit( 
                        WidgetClass wc) ;
static Widget GetRectObjKid( 
                        CompositeWidget p) ;
static void Initialize( 
                        Widget request,
                        Widget new_w,
                        ArgList args,
                        Cardinal *num_args) ;
static Boolean SetValues( 
                        Widget current,
                        Widget request,
                        Widget new_w,
                        ArgList args,
                        Cardinal *num_args) ;
static void InsertChild( 
                        Widget w) ;
static void GetDefaultPosition( 
                        XmBulletinBoardWidget child,
                        Widget parent,
                        Position *xRtn,
                        Position *yRtn) ;
static void ChangeManaged( 
                        Widget wid) ;
static XtGeometryResult GeometryManager( 
                        Widget wid,
                        XtWidgetGeometry *request,
                        XtWidgetGeometry *reply) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


#ifdef FULL_EXT
static XmBaseClassExtRec	myBaseClassExtRec = {
    NULL,				/* Next extension	*/
    NULLQUARK,				/* record type XmQmotif	*/
    XmBaseClassExtVersion,		/* version		*/
    sizeof(XmBaseClassExtRec),		/* size			*/
    XmInheritInitializeSetup,		/* initialize setup	*/
    NULL,				/* initialize prehook	*/
    NULL,				/* initialize posthook	*/
    XmInheritInitializeCleanup,		/* initialize cleanup	*/
    XmInheritSetValuesSetup,		/* setValues setup	*/
    NULL,				/* setValues prehook	*/
    NULL,				/* setValues posthook	*/
    XmInheritSetValuesCleanup,		/* setValues cleanup	*/
    XmInheritGetValuesSetup,		/* getValues setup	*/
    NULL,				/* getValues prehook	*/
    NULL,				/* getValues posthook	*/
    XmInheritGetValuesCleanup,		/* getValues cleanup	*/
    (WidgetClass)&xmDialogShellExtClassRec,/* secondary class	*/
    XmInheritSecObjectCreate,		/* secondary create	*/
    {0},				/* fast subclass	*/
};
#else
static XmBaseClassExtRec	myBaseClassExtRec = {
    NULL,				/* Next extension	*/
    NULLQUARK,				/* record type XmQmotif	*/
    XmBaseClassExtVersion,		/* version		*/
    sizeof(XmBaseClassExtRec),		/* size			*/
    XmInheritInitializePrehook,		/* initialize prehook	*/
    XmInheritSetValuesPrehook,		/* set_values prehook	*/
    XmInheritInitializePosthook,	/* initialize posthook	*/
    XmInheritSetValuesPosthook,		/* set_values posthook	*/
    (WidgetClass)&xmDialogShellExtClassRec,/* secondary class	*/
    XmInheritSecObjectCreate,		/* secondary create	*/
    NULL,				/* getSecRes data	*/
    {0}				/* fast subclass	*/
};
#endif

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
static XtResource resources[] =
{
    {
        DXmNfitToScreenPolicy, DXmCFitToScreenPolicy,
        DXmRFitToScreenPolicy, sizeof(unsigned char),
        XtOffset(XmDialogShellWidget,dialog.fit_to_screen_policy),
        XmRImmediate, (XtPointer) DXmNONE
    },
#ifdef DEC_MOTIF_RTOL
    {
        DXmNlayoutDirection, DXmCLayoutDirection,
        DXmRLayoutDirection, sizeof(unsigned char),
        XtOffset(XmDialogShellWidget,dialog.dxm_layout_direction),
        XmRImmediate, (XtPointer) DXmLAYOUT_RIGHT_DOWN
    },
#endif
};
#endif


#ifdef DEC_MOTIF_BUG_FIX
externaldef(xmdialogshellclassrec) XmDialogShellClassRec xmDialogShellClassRec = {
#else
XmDialogShellClassRec xmDialogShellClassRec = {
#endif
    {					    /* core class record */
	
	(WidgetClass) & transientShellClassRec,	/* superclass */
	"XmDialogShell", 		/* class_name */
	sizeof(XmDialogShellWidgetRec), /* widget_size */
	ClassInitialize,		/* class_initialize proc */
	ClassPartInit,			/* class_part_initialize proc */
	FALSE, 				/* class_inited flag */
	Initialize, 			/* instance initialize proc */
	NULL, 				/* init_hook proc */
	XtInheritRealize,		/* realize widget proc */
	NULL, 				/* action table for class */
	0, 				/* num_actions */
#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
        resources,                      /* resource list of class */
        XtNumber(resources),            /* num_resources in list */
#else
	NULL,	 			/* resource list of class */
	0,		 		/* num_resources in list */
#endif
	NULLQUARK, 			/* xrm_class ? */
	FALSE, 				/* don't compress_motion */
	TRUE, 				/* do compress_exposure */
	FALSE, 				/* do compress enter-leave */
	FALSE, 				/* do have visible_interest */
#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
        Destroy,                        /* destroy widget proc */
        Resize,                         /* resize widget proc */
#else
	NULL, 				/* destroy widget proc */
	XtInheritResize, 		/* resize widget proc */
#endif
	NULL, 				/* expose proc */
	SetValues, 			/* set_values proc */
	NULL, 				/* set_values_hook proc */
	XtInheritSetValuesAlmost, 	/* set_values_almost proc */
	NULL, 				/* get_values_hook */
	NULL, 				/* accept_focus proc */
	XtVersion, 			/* current version */
	NULL, 				/* callback offset    */
	XtInheritTranslations, 		/* default translation table */
	XtInheritQueryGeometry, 	/* query geometry widget proc */
	NULL, 				/* display accelerator    */
	(XtPointer)&myBaseClassExtRec,	/* extension record      */
    },
    { 					/* composite class record */
	GeometryManager,                /* geometry_manager */
	ChangeManaged, 			/* change_managed		*/
	InsertChild,			/* insert_child			*/
	XtInheritDeleteChild, 		/* from the shell */
	NULL, 				/* extension record      */
    },
    { 					/* shell class record */
	NULL, 				/* extension record      */
    },
    { 					/* wm shell class record */
	NULL, 				/* extension record      */
    },
    { 					/* vendor shell class record */
	NULL,				/* extension record      */
    },
    { 					/* transient class record */
	NULL, 				/* extension record      */
    },
    { 					/* our class record */
	NULL, 				/* extension record      */
    },
};


/*
 * now make a public symbol that points to this class record
 */

externaldef(xmdialogshellwidgetclass)
    WidgetClass xmDialogShellWidgetClass = (WidgetClass)&xmDialogShellClassRec;


#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */

static XContext fitToScreenContext;

typedef struct {
    XmScrollBarWidget hscroll;
    XmScrollBarWidget vscroll;
    XmDrawingAreaWidget interloper;
    } FitToScreenChildren;

#endif

    

static void 
#ifdef _NO_PROTO
ClassInitialize()
#else
ClassInitialize( void )
#endif /* _NO_PROTO */
{
  Cardinal                    wc_num_res, sc_num_res, wc_unique_res;
  XtResource                  *merged_list;
  int                         i, j, k;
  XtResourceList              uncompiled, res_list;
  Cardinal                    num;

/**************************************************************************
   VendorExt and  DialogExt resource lists are being merged into one
   and assigned to xmDialogShellExtClassRec. This is for performance
   reasons, since, instead of two calls to XtGetSubResources() XtGetSubvaluse()
   and XtSetSubvalues() for both the superclass and the widget class, now
   we have just one call with a merged resource list.

****************************************************************************/

  wc_num_res = xmDialogShellExtClassRec.object_class.num_resources ;

  wc_unique_res = wc_num_res - 1; /* XmNdeleteResponse has been defined */
                                  /* in VendorSE  */

  sc_num_res = xmVendorShellExtClassRec.object_class.num_resources;

  merged_list = (XtResource *)XtMalloc((sizeof(XtResource) * (wc_unique_res +
                                                                 sc_num_res)));

  _XmTransformSubResources(xmVendorShellExtClassRec.object_class.resources,
                           sc_num_res, &uncompiled, &num);

  for (i = 0; i < num; i++)
  {

  merged_list[i] = uncompiled[i];

  }

  XtFree((char *)uncompiled);

  res_list = xmDialogShellExtClassRec.object_class.resources;

  for (i = 0, j = num; i < wc_num_res; i++)
  {

   for (k = 0; 
        ((k < sc_num_res) &&  (strcmp(merged_list[k].resource_name,
                              res_list[i].resource_name) != 0)); k++)
   {
    ;
   }
   if ( (k < sc_num_res) && (strcmp(merged_list[k].resource_name, res_list[i].resource_name) == 0))
     merged_list[k] = res_list[i];
   else
   {
     merged_list[j] =
        xmDialogShellExtClassRec.object_class.resources[i];
     j++;
   }
  }

  xmDialogShellExtClassRec.object_class.resources = merged_list;
  xmDialogShellExtClassRec.object_class.num_resources =
                wc_unique_res + sc_num_res ;

  xmDialogShellExtObjectClass->core_class.class_initialize();

  myBaseClassExtRec.record_type = XmQmotif;

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
    fitToScreenContext = XUniqueContext();
#endif

}

/************************************************************************
 *
 *  ClassPartInit
 *    Set up the fast subclassing for the widget.
 *
 ************************************************************************/
static void 
#ifdef _NO_PROTO
ClassPartInit( wc )
        WidgetClass wc ;
#else
ClassPartInit(
        WidgetClass wc )
#endif /* _NO_PROTO */
{
   _XmFastSubclassInit(wc, XmDIALOG_SHELL_BIT);
}

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */

/************************************************************************
 *
 *  ConfigureDialog
 *
 *              Don't move the position of dialog but update the fields
 *
 ************************************************************************/
static void ConfigureDialog(w, geom)
    Widget              w;
    XtWidgetGeometry    *geom;
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
 *  VertSliderMove
 *    Callback for the sliderMoved resource of the vertical scrollbar
 *
 ************************************************************************/
/* ARGSUSED */
static  void  VertSliderMove(w,closure,call_data)
    Widget w;
    XtPointer  closure;
    XmScrollBarCallbackStruct *call_data;
{
    XmDialogShellWidget shell;
    Widget              apps_child;

    shell = (XmDialogShellWidget) XtParent(w);
    apps_child = GetRectObjKid((CompositeWidget) shell);

    _XmMoveObject(apps_child,
                 (Position ) apps_child->core.x,
                 (Position ) -((int ) call_data->value));

} /* VertSliderMove */

/************************************************************************
 *
 *  HorizSliderMove
 *    Callback for the sliderMoved resource of the horizontal scrollbar
 *
 ************************************************************************/
/* ARGSUSED */
static void HorizSliderMove(w,closure,call_data)
    Widget w;
    XtPointer  closure;
    XmScrollBarCallbackStruct *call_data;
{
    XmDialogShellWidget shell;
    Widget              apps_child;

    shell = (XmDialogShellWidget) XtParent(w);
    apps_child = GetRectObjKid((CompositeWidget) shell);

    _XmMoveObject(apps_child,
                 (Position ) -((int) call_data->value),
                  (Position ) apps_child->core.y);

} /* HorizSliderMove */

/************************************************************************
 *
 *  IsAddedChild
 *    Returns true if kid is an added child for I14Y.
 *
 ************************************************************************/
static Boolean IsAddedChild(p, kid)
    CompositeWidget     p;
    Widget              kid;
{
    FitToScreenChildren *ftscontext;

    /********************************************************************/
    /*                                                                  */
    /* Return right away if this kid isn't a scrollbar or a drawingarea */
    /*                                                                  */
    /********************************************************************/
    if (!XmIsScrollBar(kid) && !XmIsDrawingArea(kid))
        return FALSE;

    /********************************************************************/
    /*                                                                  */
    /* Get the FitToScreenChildren context block for this widget        */
    /*                                                                  */
    /********************************************************************/
    if ( XFindContext (XtDisplay(p), (Window)p, fitToScreenContext,
                                (caddr_t *)&ftscontext) == 0)
    {
        if ( kid == (Widget) ftscontext->hscroll ||
             kid == (Widget) ftscontext->vscroll ||
             kid == (Widget) ftscontext->interloper )
            return TRUE;
    }

    return FALSE;

} /* IsAddedChild */

/************************************************************************
 *
 *  IsAddedThatChanged
 *    Returns true if kid's user data indicates that it knew it was
 *    going to cause a change manage call.  This is done so the
 *    ChangeManaged routine will not get confused.
 *
 *    The value of -1 means that this is a child who caused the
 *    change.  (The incremental nature of calling ChangeManaged helps
 *    here in that only one child changes at a time.)  It is important
 *    that if the dialog shell knows it is going to change a "fit-to-screen"
 *    child that it sets its user data to -1.
 *
 *    Also, set the user data to 0 so we don't come across this again.
 *
 ************************************************************************/
static Boolean IsAddedThatChanged(p)
    CompositeWidget     p;
{
    FitToScreenChildren *ftscontext;

    /********************************************************************/
    /*                                                                  */
    /* Get the FitToScreenChildren context block for this widget        */
    /*                                                                  */
    /********************************************************************/
    if ( XFindContext (XtDisplay(p), (Window)p, fitToScreenContext,
                                (caddr_t *)&ftscontext) == 0)
    {
        if ( (ftscontext->hscroll &&
                ftscontext->hscroll->primitive.user_data == (XtPointer) -1) ||
             (ftscontext->vscroll &&
                ftscontext->vscroll->primitive.user_data == (XtPointer) -1) ||
             (ftscontext->interloper &&
                ftscontext->interloper->manager.user_data == (XtPointer) -1) )
        {
            if (ftscontext->hscroll)
                ftscontext->hscroll->primitive.user_data = 0;
            if (ftscontext->vscroll)
                ftscontext->vscroll->primitive.user_data = 0;
            if (ftscontext->interloper)
                ftscontext->interloper->manager.user_data = 0;
            return TRUE;
        }
    }

    return FALSE;

} /* IsAddedThatChanged */

/************************************************************************
 *
 *  MakeShellFitScreen
 *    Checks to see if the child will fit in the shell and on the
 *    screen.  Create scrollbars for shell if necessary.
 *
 *    Returns actions for managing/unmanaging the scrollbars, and a
 *    geometry request for resizing the shell.
 *
 ************************************************************************/
static void MakeShellFitScreen(shell,apps_child,hscroll_action,
                               vscroll_action,request)
    XmDialogShellWidget shell;
    Widget              apps_child;
    unsigned char       *hscroll_action;
    unsigned char       *vscroll_action;
    XtWidgetGeometry    *request;
{
    FitToScreenChildren *ftscontext;
    XmScrollBarWidget   hscroll = NULL;
    XmScrollBarWidget   vscroll = NULL;

    int                 effective_width;
     int                 effective_height;
    Boolean             check_again;

    Arg                 al[20];
    Cardinal            ac;

    Screen              *screen = (Screen *) XtScreen(shell);

    if (!apps_child) return;

    /********************************************************************/
    /*                                                                  */
    /* Get the FitToScreenChildren context block for this widget        */
    /*                                                                  */
    /********************************************************************/
    if ( XFindContext (XtDisplay(shell), (Window)shell, fitToScreenContext,
                                (caddr_t *)&ftscontext) == 0)
    {
        hscroll = ftscontext->hscroll;
        vscroll = ftscontext->vscroll;
    }

    /********************************************************************/
    /*                                                                  */
    /* Initialize the actions for the scrollbars.                       */
    /*                                                                  */
    /********************************************************************/
    if (hscroll && XtIsManaged(hscroll))
        *hscroll_action = FTS_MANAGE;
    else
        *hscroll_action = FTS_UNMANAGE;

    if (vscroll && XtIsManaged(vscroll))
        *vscroll_action = FTS_MANAGE;
    else
        *vscroll_action = FTS_UNMANAGE;

    /********************************************************************/
    /*                                                                  */
    /* If the request's height and width have not been set, set them    */
    /* from the shell.  Note that the mode for these will only be set   */
    /* if they change.                                                  */
    /*                                                                  */
    /********************************************************************/
    if (!(request->request_mode & CWWidth))
        request->width = XtWidth(shell);

    if (!(request->request_mode & CWHeight))
        request->height = XtHeight(shell);

    /********************************************************************/
    /*                                                                  */
    /* Make sure the shell fits on the screen.                          */
    /*                                                                  */
    /********************************************************************/
    if (request->width > FTS_MAX_WIDTH(screen))
    {
        request->request_mode |= CWWidth;
        request->width = FTS_MAX_WIDTH(screen);
    }

    if (request->height > FTS_MAX_HEIGHT(screen))
    {
        request->request_mode |= CWHeight;
        request->height = FTS_MAX_HEIGHT(screen);
    }

    /********************************************************************/
    /*                                                                  */
    /* Calculate the effective width and height of the shell based      */
    /* based upon the state of the scrollbars.                          */
    /*                                                                  */
    /********************************************************************/
    effective_width = request->width;
    if (vscroll && (*vscroll_action == FTS_MANAGE))
        effective_width -= XtWidth(vscroll);

    effective_height = request->height;
    if (hscroll && (*hscroll_action == FTS_MANAGE))
        effective_height -= XtHeight(hscroll);

    /********************************************************************/
    /*                                                                  */
    /* Create the scrollbars, if necessary.  Keep track of the          */
    /* scrollbar sizes because it may be necessary to resize the        */
    /* shell or child to fit when the scrollbars are added or removed.  */
    /*                                                                  */
    /********************************************************************/
    check_again = TRUE;
    while (check_again)
    {
        check_again = FALSE;

        /****************************************************************/
        /*                                                              */
        /* Horizontal Scrollbar                                         */
        /*                                                              */
        /****************************************************************/

        if (XtWidth(apps_child) > effective_width)
        {
            if (!hscroll)
            {
                ac = 0;
                XtSetArg(al[ac],XmNuserData,-1); ac++;
                XtSetArg(al[ac],XmNorientation,XmHORIZONTAL); ac++;
                XtSetArg(al[ac],XmNminimum,0); ac++;
                XtSetArg(al[ac],XmNvalue,0); ac++;
                XtSetArg(al[ac],XmNdragCallback,(XtArgVal)HSCallBack); ac++;
   
                ftscontext->hscroll = hscroll =
           (XmScrollBarWidget) XmCreateScrollBar((Widget) shell,"dialog_hscroll",al,ac);
            }

            if (*hscroll_action == FTS_UNMANAGE)
            {
                *hscroll_action = FTS_MANAGE;
                request->request_mode |= CWHeight;
                request->height += XtHeight(hscroll);
                if (request->height > FTS_MAX_HEIGHT(screen))
                    request->height = FTS_MAX_HEIGHT(screen);
                effective_height = request->height - XtHeight(hscroll);
            }
        }
        else if (hscroll && (*hscroll_action == FTS_MANAGE))
        {
            *hscroll_action = FTS_UNMANAGE;
            request->request_mode |= CWHeight;
            request->height -= XtHeight(hscroll);
            effective_height = request->height;
        }

        /****************************************************************/
        /*                                                              */
        /* Vertical Scrollbar                                           */
        /*                                                              */
        /****************************************************************/
        if (XtHeight(apps_child) > effective_height)
        {
            if (!vscroll)
            {
                ac = 0;
                XtSetArg(al[ac],XmNuserData,(XtPointer) -1); ac++;
                XtSetArg(al[ac],XmNorientation,XmVERTICAL); ac++;
                XtSetArg(al[ac],XmNminimum,0); ac++;
                XtSetArg(al[ac],XmNvalue,0); ac++;
                XtSetArg(al[ac],XmNdragCallback,(XtArgVal)VSCallBack); ac++;
                XtSetArg(al[ac],XmNvalueChangedCallback,
                                 (XtArgVal)VSCallBack); ac++;

                ftscontext->vscroll = vscroll =
           (XmScrollBarWidget) XmCreateScrollBar((Widget) shell,"dialog_vscroll",al,ac);
            }

            if (*vscroll_action == FTS_UNMANAGE)
            {
                *vscroll_action = FTS_MANAGE;
                request->request_mode |= CWWidth;
                request->width += XtWidth(vscroll);
                if (request->width > FTS_MAX_WIDTH(screen))
                    request->width = FTS_MAX_WIDTH(screen);
                effective_width = request->width - XtWidth(vscroll);
                check_again = TRUE;
            }
        }
        else if (vscroll && (vscroll_action == FTS_MANAGE))
        {
            *vscroll_action = FTS_UNMANAGE;
            request->request_mode |= CWWidth;
            request->width -= XtWidth(vscroll);
            effective_width = request->width;
            check_again = TRUE;
        }

    } /* while check_again */
 
    /********************************************************************/
    /*                                                                  */
    /* Now adjust the X,Y position of shell to fit as much as possible  */
    /* on the screen.                                                   */
    /*                                                                  */
    /********************************************************************/
    if (!(request->request_mode & CWX))
        request->x = XtX(shell);

    if (request->width + request->x + FTS_WM_RESIZE_BORDER_SIZE >
                 WidthOfScreen(screen))
    {
        request->request_mode |= CWX;
        request->x = WidthOfScreen(screen) -
                  request->width - FTS_WM_RESIZE_BORDER_SIZE;
    }


    if (!(request->request_mode & CWY))
        request->y = XtY(shell);

    if (request->height + request->y + FTS_WM_RESIZE_BORDER_SIZE >
                             HeightOfScreen(screen))
    {
        request->request_mode |= CWY;
        request->y = HeightOfScreen(screen) - request->height -
                FTS_WM_RESIZE_BORDER_SIZE;
    }

} /* MakeShellFitScreen */


/************************************************************************
 *
 *  MakeChildFitShell
 *    Checks to see if the child will fit in the shell.  If the shell
 *    is larger than the child, make the child the size of the shell.
 *    If the shell is smaller than the child, ask the child for its
 *    preferred size, and the set the size to be the MAX(preferred,shell)
 *    sizes.
 *
 *    Returns actions for managing/unmanaging the scrollbars, and a
 *    geometry request for resizing the child.
 *
 ************************************************************************/
static void MakeChildFitShell(shell,apps_child,hscroll_action,vscroll_action)
    XmDialogShellWidget shell;
    Widget              apps_child;
    unsigned char       *hscroll_action;
    unsigned char       *vscroll_action;
{
    FitToScreenChildren *ftscontext;
    XmScrollBarWidget   hscroll = NULL;
    XmScrollBarWidget   vscroll = NULL;

    XtWidgetGeometry    request;
    XtWidgetGeometry    desired,preferred;
    XtGeometryResult    georesult;

    int                 effective_width;
    int                 effective_height;
    Boolean             check_again;

    Arg                 al[20];
    Cardinal            ac;

    if (!apps_child) return;

    /********************************************************************/
    /*                                                                  */
    /* Get the FitToScreenChildren context block for this widget        */
    /*                                                                  */
    /********************************************************************/
    if ( XFindContext (XtDisplay(shell), (Window)shell, fitToScreenContext,
                                (caddr_t *)&ftscontext) == 0)
    {
        hscroll = ftscontext->hscroll;
        vscroll = ftscontext->vscroll;
    }

    /********************************************************************/
    /*                                                                  */
    /* Initialize the actions for the scrollbars.                       */
    /*                                                                  */
    /********************************************************************/
    if (hscroll && XtIsManaged(hscroll))
        *hscroll_action = FTS_MANAGE;
    else
        *hscroll_action = FTS_UNMANAGE;

    if (vscroll && XtIsManaged(vscroll))
        *vscroll_action = FTS_MANAGE;
    else
        *vscroll_action = FTS_UNMANAGE;

    /********************************************************************/
    /*                                                                  */
    /* Calculate the effective width and height of the shell based      */
     /* based upon the state of the scrollbars.                          */
    /*                                                                  */
    /********************************************************************/
    effective_width = XtWidth(shell);
    if (vscroll && (*vscroll_action == FTS_MANAGE))
        effective_width -= XtWidth(vscroll);

    effective_height = XtHeight(shell);
    if (hscroll && (*hscroll_action == FTS_MANAGE))
        effective_height -= XtHeight(hscroll);

    /********************************************************************/
    /*                                                                  */
    /* Ask the child to fit the shell (with scrollbars, if              */
    /* applicable) and see what happens.  This may fix everything.      */
    /* If it doesn't, we go on to see if scrollbars are needed.         */
    /*                                                                  */
    /********************************************************************/
    request.request_mode = 0;
    request.width = XtWidth(apps_child);
    request.height = XtHeight(apps_child);

    if ((request.width > effective_width) ||
        (request.height > effective_height))
    {
        desired.request_mode = CWHeight | CWWidth;
        desired.height = effective_height;
        desired.width = effective_width;

        georesult = XtQueryGeometry(apps_child, &desired, &preferred);

        if (georesult == XtGeometryYes)
        {
            request.request_mode = CWHeight | CWWidth;
            request.height = effective_height;
            request.width = effective_width;
        }
        else if (georesult == XtGeometryAlmost)
        {
            if (preferred.request_mode & CWWidth)
            {
                request.request_mode |= CWWidth;
                request.width = MAX(preferred.width,effective_width);
             }

            if (preferred.request_mode & CWHeight)
            {
                request.request_mode |= CWHeight;
                request.height = MAX(preferred.height,effective_height);
            }
        }
    }

    /********************************************************************/
    /*                                                                  */
    /* By the time we've gotten here, the child is as small as it's     */
    /* going to get.                                                    */
    /*                                                                  */
    /* Create/remove the scrollbars, if necessary.  Keep track of the   */
    /* scrollbar sizes because it may be necessary to make the child    */
    /* grow to fill in the space caused by unmanaging a scrollbar.      */
    /*                                                                  */
    /********************************************************************/
    check_again = TRUE;
    while (check_again)
    {
        check_again = FALSE;

        /****************************************************************/
        /*                                                              */
        /* Horizontal Scrollbar                                         */
        /*                                                              */
        /****************************************************************/
        if (request.width > effective_width)
        {
            if (!hscroll)
            {
                ac = 0;
                XtSetArg(al[ac],XmNuserData,-1); ac++;
                XtSetArg(al[ac],XmNorientation,XmHORIZONTAL); ac++;
                XtSetArg(al[ac],XmNminimum,0); ac++;
                XtSetArg(al[ac],XmNvalue,0); ac++;
                XtSetArg(al[ac],XmNdragCallback,(XtArgVal)HSCallBack); ac++;
                XtSetArg(al[ac],XmNvalueChangedCallback,
                                (XtArgVal)HSCallBack); ac++;

                ftscontext->hscroll = hscroll =
                    (XmScrollBarWidget) XmCreateScrollBar((Widget) shell,
                                               "dialog_hscroll",al,ac);
            }

            if (*hscroll_action == FTS_UNMANAGE)
            {
                *hscroll_action = FTS_MANAGE;
                if (XtHeight(hscroll) > effective_height)
                        effective_height = 0;
                else
                        effective_height = effective_height - XtHeight(hscroll);
                check_again = TRUE;
            }
        }
        else
        {
            if (hscroll && (*hscroll_action == FTS_MANAGE))
            {
                *hscroll_action = FTS_UNMANAGE;
                effective_height = XtHeight(shell);
                check_again = TRUE;
            }
            if (effective_height > request.height)
            {
                request.request_mode |= CWHeight;
                request.height = effective_height;
                check_again = TRUE;
            }
            if (effective_width > request.width)
            {
                request.request_mode |= CWWidth;

                 request.width = effective_width;
            }
        }

        /****************************************************************/
        /*                                                              */
        /* Vertical Scrollbar                                           */
        /*                                                              */
        /****************************************************************/
        if (request.height > effective_height)
        {
            if (!vscroll)
            {
                ac = 0;
                XtSetArg(al[ac],XmNuserData,(XtPointer) -1); ac++;
                XtSetArg(al[ac],XmNorientation,XmVERTICAL); ac++;
                XtSetArg(al[ac],XmNminimum,0); ac++;
                XtSetArg(al[ac],XmNvalue,0); ac++;
                XtSetArg(al[ac],XmNdragCallback,(XtArgVal)VSCallBack); ac++;
	        XtSetArg(al[ac],XmNvalueChangedCallback,
                                   (XtArgVal)VSCallBack); ac++;

                ftscontext->vscroll = vscroll =
                         (XmScrollBarWidget) XmCreateScrollBar((Widget) shell,
                               "dialog_vscroll",al,ac);
            }

            if (*vscroll_action == FTS_UNMANAGE)
            {
                *vscroll_action = FTS_MANAGE;
                if (XtWidth(vscroll) > effective_width)
                        effective_width = 0;
                else
                        effective_width = effective_width - XtWidth(vscroll);

                check_again = TRUE;
            }
        }
        else
        {
            if (vscroll && (*vscroll_action == FTS_MANAGE))
            {
                *vscroll_action = FTS_UNMANAGE;
                effective_width = XtWidth(shell);
                check_again = TRUE;
            }
            if (effective_width > request.width)
            {
                request.request_mode |= CWWidth;
                request.width = effective_width;
                check_again = TRUE;
            }
            if (effective_height > request.height)
            {
                request.request_mode |= CWHeight;
                request.height = effective_height;
            }
        }

    } /* while check_again */

    /********************************************************************/
    /*                                                                  */
    /* Now configure the widget.                                        */
    /*                                                                  */
    /********************************************************************/
    if (request.request_mode)
        XtResizeWidget(apps_child,
                       request.width,
                       request.height,
                       apps_child->core.border_width);

} /* MakeChildFitShell */


/************************************************************************
 *
 *  HandleScrollBars
 *    Handles Creation, Management, and Unmanagement of Added scrollbars.
 *
 *    The allow_create parameter lets us restrict when the scrollbars
 *    can be created.
 *
 ************************************************************************/
static void HandleScrollBars(shell,apps_child,hscroll_action,vscroll_action)
    XmDialogShellWidget	shell;
    Widget		apps_child;
    unsigned char	hscroll_action;
    unsigned char	vscroll_action;
{
    FitToScreenChildren	*ftscontext;
    XmScrollBarWidget	hscroll = NULL;
    XmScrollBarWidget	vscroll = NULL;
    XmDrawingAreaWidget	interloper = NULL;

    Arg			al[20];
    Cardinal		ac;
    int 		value_return, slider_size_return, inc_return, page_inc_return;
    int 		bar_size,vscroll_width = 0,value;
    
    if (!apps_child) return;
    
    /********************************************************************/
    /*									*/
    /* Get the FitToScreenChildren context block for this widget	*/
    /*									*/
    /********************************************************************/
    if ( XFindContext (XtDisplay(shell), (Window)shell, fitToScreenContext,
				(caddr_t *)&ftscontext) == 0)
    {
	hscroll = ftscontext->hscroll;
	vscroll = ftscontext->vscroll;
	interloper = ftscontext->interloper;
    }

    /********************************************************************/
    /*									*/
    /* Perform the desired manage/unmanage actions on the scrollbars.	*/
    /*									*/
    /********************************************************************/
    if (hscroll)
    {
	if (!XtIsManaged(hscroll) && (hscroll_action == FTS_MANAGE))
	{
	    hscroll->primitive.user_data = (XtPointer) -1;
	    XtManageChild((Widget) hscroll);
	}
	else if (XtIsManaged(hscroll) && (hscroll_action == FTS_UNMANAGE))
	{
	    _XmMoveObject(apps_child, 
			  (Position) 0, (Position) apps_child->core.y);
	    hscroll->primitive.user_data = (XtPointer) -1;
	    XtUnmanageChild((Widget) hscroll);
	    ac = 0;
	    XtSetArg(al[ac],XmNvalue,0); ac++;
	    XtSetValues((Widget) hscroll, al, ac);
	}
    }

    if (vscroll)
    {
	if (!XtIsManaged(vscroll) && (vscroll_action == FTS_MANAGE))
	{
	    vscroll->primitive.user_data = (XtPointer) -1;
	    XtManageChild((Widget) vscroll);
	}
	else if (XtIsManaged(vscroll) && (vscroll_action == FTS_UNMANAGE))
	{
	    _XmMoveObject(apps_child, 
			  (Position) apps_child->core.x, (Position) 0);
	    vscroll->primitive.user_data = (XtPointer) -1;
	    XtUnmanageChild((Widget) vscroll);
	    ac = 0;
	    XtSetArg(al[ac],XmNvalue,0); ac++;
	    XtSetValues((Widget) vscroll, al, ac);
	}
    }
    
    /********************************************************************/
    /*									*/
    /* Area between Scrollbars (create, [un]manage)			*/
    /*									*/
    /********************************************************************/
    if (hscroll && XtIsManaged(hscroll) && 
	vscroll && XtIsManaged(vscroll))
    {
	if (!interloper)
        {
	    ac = 0;
	    /* Disable traversal. There's no reason anyone would want
	    ** to traverse here anyway, and if we don't disable it
	    ** an infinite loop (bug 1312) can occur on TAB
	    */
	    XtSetArg(al[ac],XmNtraversalOn,FALSE); ac++;
	    XtSetArg(al[ac],XmNuserData,(XtPointer) -1); ac++;
	    ftscontext->interloper = interloper = 
		(XmDrawingAreaWidget) XmCreateDrawingArea((Widget) shell,"dialog_inbetween",al,ac);
	}

	if (!XtIsManaged(interloper))
	{
	    interloper->manager.user_data = (XtPointer) -1;
	    XtManageChild((Widget) interloper);
	}
    }
    else if (interloper && XtIsManaged(interloper))
    {
	interloper->manager.user_data = (XtPointer) -1;
	XtUnmanageChild((Widget) interloper);
    }

    /********************************************************************/
    /*									*/
    /* Horizontal Scrollbar (size, position)				*/
    /*									*/
    /********************************************************************/
    if (hscroll && XtIsManaged(hscroll))
    {
	XRaiseWindow(XtDisplay(hscroll), XtWindow(hscroll));
	bar_size = XtWidth(shell);
        if (vscroll && XtIsManaged(vscroll))
	{
	    vscroll_width = XtWidth(vscroll);
	    bar_size -= XtWidth(vscroll);
	}
	
 	XmScrollBarGetValues((Widget) hscroll,
			     &value_return,
			     &slider_size_return,
			     &inc_return,
			     &page_inc_return);

	value = value_return;
	if (XtWidth(apps_child) < (value + bar_size))
	    value = XtWidth(apps_child) - bar_size;
	    
	if (XtX(apps_child) != -value)
	    _XmMoveObject(apps_child,
			  (Position ) -((int ) value),
			  (Position ) apps_child->core.y);

 	ac = 0;

#ifdef DEC_MOTIF_RTOL
	if (LayoutDS(shell) != DXmLAYOUT_RIGHT_DOWN)
	{
	    XtSetArg(al[ac],XmNx,vscroll_width); ac++;
	    XtSetArg(al[ac],XmNminimum,-vscroll_width); ac++;
	    XtSetArg(al[ac],XmNmaximum,XtWidth(apps_child) - vscroll_width); ac++;
	}
	else
	{
#endif
	    XtSetArg(al[ac],XmNx,0); ac++;
	    XtSetArg(al[ac],XmNmaximum,XtWidth(apps_child)); ac++;
#ifdef DEC_MOTIF_RTOL
	}
#endif
	
	XtSetArg(al[ac],XmNy,XtHeight(shell) - XtHeight(hscroll)); ac++;
	XtSetArg(al[ac],XmNwidth,bar_size); ac++;
        XtSetArg(al[ac],XmNsliderSize,bar_size); ac++;
        XtSetArg(al[ac],XmNpageIncrement,MAX(1,bar_size - 5)); ac++;
        XtSetArg(al[ac],XmNincrement,MAX(1,bar_size/10)); ac++;
	if (value != value_return)
	{
	    XtSetArg(al[ac],XmNvalue,value); ac++;
        }
	
	XtSetValues((Widget) hscroll,al,ac);
    }


    /********************************************************************/
    /*									*/
    /* Vertical Scrollbar (size, position)				*/
    /*									*/
    /********************************************************************/
    if (vscroll && XtIsManaged(vscroll))
    {
	XRaiseWindow(XtDisplay(vscroll), XtWindow(vscroll));
	bar_size = XtHeight(shell);
        if (hscroll && XtIsManaged(hscroll))
	    bar_size -= XtHeight(hscroll);

	XmScrollBarGetValues((Widget) vscroll,
			     &value_return,
			     &slider_size_return,
			     &inc_return,
			     &page_inc_return);

	value = value_return;
	if (XtHeight(apps_child) < (value + bar_size))
	    value = XtHeight(apps_child) - bar_size;
	    
	if (XtY(apps_child) != -value)
	    _XmMoveObject(apps_child,
			  (Position ) apps_child->core.x,
			  (Position ) -((int ) value));

	ac = 0;

#ifdef DEC_MOTIF_RTOL
	if (LayoutDS(shell) != DXmLAYOUT_RIGHT_DOWN)
	    XtSetArg(al[ac],XmNx,0);
	else
#endif
	    XtSetArg(al[ac],XmNx,XtWidth(shell) - XtWidth(vscroll));
	ac++;

	XtSetArg(al[ac],XmNy,0); ac++;
	XtSetArg(al[ac],XmNheight,bar_size); ac++;
	XtSetArg(al[ac],XmNmaximum,XtHeight(apps_child)); ac++;
	XtSetArg(al[ac],XmNsliderSize,bar_size); ac++;
        XtSetArg(al[ac],XmNpageIncrement,MAX(1,bar_size - 5)); ac++;
        XtSetArg(al[ac],XmNincrement,MAX(1,bar_size/10)); ac++;
	if (value != value_return)
	{
	    XtSetArg(al[ac],XmNvalue,value); ac++;
        }
	XtSetValues((Widget) vscroll,al,ac);
    }

    /********************************************************************/
    /*									*/
    /* Area between Scrollbars (size, position)				*/
    /*									*/
    /********************************************************************/
    if (interloper && XtIsManaged(interloper))
    {
	XRaiseWindow(XtDisplay(interloper), XtWindow(interloper));

	ac = 0;

#ifdef DEC_MOTIF_RTOL
	if (LayoutDS(shell) != DXmLAYOUT_RIGHT_DOWN)
	    XtSetArg(al[ac],XmNx,0);
	else
#endif
	    XtSetArg(al[ac],XmNx,XtWidth(shell) - XtWidth(vscroll));
	ac++;

	XtSetArg(al[ac],XmNy,XtHeight(shell) - XtHeight(hscroll)); ac++;
	XtSetArg(al[ac],XmNwidth,XtWidth(vscroll)); ac++;
	XtSetArg(al[ac],XmNheight,XtHeight(hscroll)); ac++;
	XtSetValues((Widget) interloper,al,ac);
    }

} /* HandleScrollBars */

#endif

static Widget 
#ifdef _NO_PROTO
GetRectObjKid( p )
        CompositeWidget p ;
#else
GetRectObjKid(
        CompositeWidget p )
#endif /* _NO_PROTO */
{
    Cardinal	i;
    Widget	*currKid;
    
#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
    /********************************************************************/
    /*                                                                  */
    /* Get the FitToScreenChildren context block for this widget        */
    /*                                                                  */
    /********************************************************************/
    FitToScreenChildren *ftscontext = 0;
    XmDialogShellWidget dsw = (XmDialogShellWidget)p;

    if ( FitToScreenPolicy(dsw) != DXmNONE )
        XFindContext (XtDisplay(dsw), (Window)dsw, fitToScreenContext,
                                (caddr_t *)&ftscontext);
#endif


    for (i = 0, currKid = p->composite.children;
	 i < p->composite.num_children;
	 i++, currKid++)
      {
	  if(    XtIsRectObj( *currKid)
              /* The Input Method child is a CoreClass object; ignore it. */
              && ((*currKid)->core.widget_class != coreWidgetClass)    )
          {   
#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
              if (ftscontext == 0 ||
                  (*currKid != (Widget) ftscontext->hscroll &&
                   *currKid != (Widget) ftscontext->vscroll &&
                   *currKid != (Widget) ftscontext->interloper))
                  return (*currKid);
#else
              return (*currKid);
#endif
          } 
      }
    return NULL;
}


/************************************************************************
 *
 *  Initialize
 *
 ************************************************************************/
static void 
#ifdef _NO_PROTO
Initialize( request, new_w, args, num_args )
        Widget request ;
        Widget new_w ;
        ArgList args ;
        Cardinal *num_args ;
#else
Initialize(
        Widget request,
        Widget new_w,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
    XmDialogShellWidget d_shell = (XmDialogShellWidget)new_w;
#endif

    if (XtWidth  (new_w) <= 0)  XtWidth  (new_w) = 5;
    if (XtHeight (new_w) <= 0)  XtHeight (new_w) = 5;

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */

    /********************************************************************/
    /*                                                                  */
    /* If fit_to_screen_policy was requested, create a context block    */
    /* to hold the ids of the child widgets that we may need to create  */
    /*                                                                  */
    /********************************************************************/
    if (FitToScreenPolicy(d_shell) != DXmNONE)
    {
        FitToScreenChildren *ftscontext =
            (FitToScreenChildren *) XtMalloc(sizeof(FitToScreenChildren));
        (void) XSaveContext( XtDisplay(new_w),
                             (Window) new_w,
                             fitToScreenContext,
                             (caddr_t)ftscontext);
        ftscontext->hscroll = NULL;
        ftscontext->vscroll = NULL;
        ftscontext->interloper = NULL;
    }

#endif

}

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
/************************************************************************
 *
 *  Destroy
 *    Free up the child-id context block
 *
 ************************************************************************/
static void Destroy(dsw)
    XmDialogShellWidget dsw;
{
    FitToScreenChildren *ftscontext;

    if ( XFindContext (XtDisplay(dsw), (Window)dsw, fitToScreenContext,
                                (caddr_t *)&ftscontext) == 0)
    {
        XtFree((char *)ftscontext);
        XDeleteContext (XtDisplay(dsw), (Window)dsw, fitToScreenContext);
    }

} /* Destroy */
/************************************************************************
 *
 *  Resize
 *    If "fit_to_screen" was not requested, just invoke Dialog Shell's
 *    superclass' resize routine.
 *
 *    Otherwise, if the shell is now larger than the application child,
 *    resize the application child.  If it is smaller, ask the application
 *    child for its preferred size, and set it to
 *    MAX(preferred-size, shell-size).
 *
 ************************************************************************/
static void Resize(w)
    XmDialogShellWidget w;
{
    Widget              apps_child;
    unsigned char       hscroll_action;
    unsigned char       vscroll_action;
    if (FitToScreenPolicy(w) != DXmNONE)
    {
        /****************************************************************/
        /*                                                              */
        /* Get the application's child of the Dialog Shell.             */
        /*                                                              */
        /****************************************************************/
        apps_child = GetRectObjKid((CompositeWidget) w);
        if (!apps_child) return;

        /****************************************************************/
        /*                                                              */
        /* Now fit the child to shell and handle the added scrollbars.  */
        /*                                                              */
        /****************************************************************/
        MakeChildFitShell(w,apps_child,&hscroll_action,&vscroll_action);
        HandleScrollBars(w,apps_child,hscroll_action,vscroll_action);
    }
    else
    {
        (*(transientShellWidgetClass)->core_class.resize) ((Widget) w);
    }


} /* Resize */
#endif




/************************************************************************
 *
 *  SetValues
 *
 ************************************************************************/
static Boolean 
#ifdef _NO_PROTO
SetValues( current, request, new_w, args, num_args )
        Widget current ;
        Widget request ;
        Widget new_w ;
        ArgList args ;
        Cardinal *num_args ;
#else
SetValues(
        Widget current,
        Widget request,
        Widget new_w,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
        Widget child ;
#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
    /********************************************************************/
    /*                                                                  */
    /* If fit_to_screen_policy was requested, create a context block    */
    /* to hold the ids of the child widgets that we may need to create  */
    /*                                                                  */
    /* Don't allow any changes after the shell has been popped up.      */
    /*                                                                  */
    /********************************************************************/
    XmDialogShellWidget cd_shell = (XmDialogShellWidget)current;
    XmDialogShellWidget d_shell = (XmDialogShellWidget)new_w;
    FitToScreenChildren *ftscontext;

    if (FitToScreenPolicy(d_shell) != FitToScreenPolicy(cd_shell))
    {
        if (d_shell->shell.popped_up)
        {
            FitToScreenPolicy(d_shell) = FitToScreenPolicy(cd_shell);
        }
        else if (FitToScreenPolicy(d_shell) != DXmNONE)
        {
            if ( XFindContext (XtDisplay(new_w), (Window)new_w, fitToScreenContext,
                               (caddr_t *)&ftscontext) != 0)
            {
                ftscontext =
                    (FitToScreenChildren *) XtMalloc(sizeof(FitToScreenChildren));
                (void) XSaveContext( XtDisplay(new_w),
                                    (Window) new_w,
                                    fitToScreenContext,
                                    (caddr_t)ftscontext);
                ftscontext->hscroll = NULL;
                ftscontext->vscroll = NULL;
                ftscontext->interloper = NULL;
            }
        }
    }
#endif


    if(    !current->core.mapped_when_managed
        && new_w->core.mapped_when_managed    )
    {   
        if(    (child = GetRectObjKid( (CompositeWidget) new_w))
            && !child->core.being_destroyed    )
        {   
            CALLBACK( (Widget) child, XmNmapCallback, XmCR_MAP, NULL) ;
            XtPopup( new_w, XtGrabNone) ;
            } 
        } 
    return (FALSE);
    }

static void 
#ifdef _NO_PROTO
InsertChild( w )
        Widget w ;
#else
InsertChild(
        Widget w )
#endif /* _NO_PROTO */
{
    CompositeWidget p = (CompositeWidget) XtParent (w);
   
#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */

    /********************************************************************/
    /*                                                                  */
    /* If the new child is a scrollbar or a drawing area and the user   */
    /* data for the child is -1, don't do anything special, as these    */
    /* objects are being added by the dialog shell itself.              */
    /*                                                                  */
    /* Note that this isn't foolproof because the application may       */
    /* actually decide to do something like make a drawing area or a    */
    /* scrollbar a direct child of the dialog shell.  Since this        */
    /* scenario really doesn't make much sense, we should be safe (I    */
    /* hope I hope).                                                    */
    /*                                                                  */
    /********************************************************************/
    XmScrollBarWidget sb = (XmScrollBarWidget)w;
    XmDrawingAreaWidget da = (XmDrawingAreaWidget)w;
    if ((XmIsScrollBar(sb) && sb->primitive.user_data == (XtPointer) -1)  ||
        (XmIsDrawingArea(da) && da->manager.user_data == (XtPointer) -1))
    {
        sb->primitive.user_data = 0;
        (* ((CompositeWidgetClass)compositeWidgetClass)
                ->composite_class.insert_child) (w);
        return;
    }
#endif


    /*
     * Make sure we only have a rectObj, a VendorObject, and
     *   maybe an Input Method (CoreClass) object as children.
     */
    if (!XtIsRectObj(w))
      return;
    else
	{
	    if(    (w->core.widget_class != coreWidgetClass)
                /* The Input Method child is a CoreClass object. */
                && GetRectObjKid( p)    )
	      {
		  XtError(MSG1);
	      }
	    else
	      {   /*
		   * make sure we're realized so people won't core dump when 
		   *   doing incorrect managing prior to realize
		   */
		  XtRealizeWidget((Widget) p);
	      }
	}
    (*((CompositeWidgetClass) compositeWidgetClass)
                                          ->composite_class.insert_child)( w) ;
    return ;
}

static void 
#ifdef _NO_PROTO
GetDefaultPosition( child, parent, xRtn, yRtn )
        XmBulletinBoardWidget child ;
        Widget parent ;
        Position *xRtn ;
        Position *yRtn ;
#else
GetDefaultPosition(
        XmBulletinBoardWidget child,
        Widget parent,
        Position *xRtn,
        Position *yRtn )
#endif /* _NO_PROTO */
{
    Display 	*disp;
    int 	max_w, max_h;
    Position 	x, y;

    x = HALFDIFF(XtWidth(parent), XtWidth(child));
    y = HALFDIFF(XtHeight(parent), XtHeight(child));
    
    /* 
     * find root co-ords of the parent's center
     */
    if (XtIsRealized (parent))
      XtTranslateCoords(parent, x, y, &x, &y);
    
    /*
     * try to keep the popup from dribbling off the display
     */
    disp = XtDisplay (child);
    max_w = DisplayWidth  (disp, DefaultScreen (disp));
    max_h = DisplayHeight (disp, DefaultScreen (disp));
    
    if ((x + (int)TotalWidth  (child)) > max_w) 
      x = max_w - TotalWidth  (child);
    if ((y + (int)TotalHeight (child)) > max_h) 
      y = max_h - TotalHeight (child);
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    *xRtn = x;
    *yRtn = y;
}
#undef HALFDIFF







/*
 * border width and size and location are ty...
 *
 * 1. We allow the border width of a XmDialogShell child to change
 *    size arbitrarily.
 *
 * 2. The border width of the shell widget tracks the child's
 *    at all times, exactly.
 *
 * 3. The width of the shell is kept exactly the same as the
 *    width of the child at all times.
 *
 * 4. The child is always positioned at the location
 *    (- child_border, - child_border).
 *
 * the net result is the child has a border width which is always
 * what the user asked for;  but none of it is ever seen, it's all
 * clipped by the shell (parent).  The user sees the border
 * of the shell which is the size he set the child's border to.
 *
 * In the DEC window manager world the window manager does
 * exactly the same thing with the window it puts around the shell.
 * Hence the shell and child have a border width just as the user
 * set but the window manager overrides that and only a single
 * pixel border is displayed.  In a non-wm environment the child 
 * appears to have a border width, in reality this is the shell
 * widget border.  You wanted to know...
 */
static void 
#ifdef _NO_PROTO
ChangeManaged( wid )
        Widget wid ;
#else
ChangeManaged(
        Widget wid )
#endif /* _NO_PROTO */
{
        XmDialogShellWidget shell = (XmDialogShellWidget) wid ;
    /*
     *  If the child went to unmanaged, call XtPopdown.
     *  If the child went to managed, call XtPopup.
     */
    
    XmBulletinBoardWidget	 child;
    XmWidgetExtData		extData = _XmGetWidgetExtData((Widget) shell, XmSHELL_EXTENSION);
    XmVendorShellExtObject	ve = (XmVendorShellExtObject)extData->widget;
    Boolean			childIsBB;

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
    unsigned char       hscroll_action;
    unsigned char       vscroll_action;

    /********************************************************************/
    /*                                                                  */
    /* Don't do anything with added scrollbars or drawing areas.  The   */
    /* dialog shell should already know where they are and what to do   */
    /* with them.                                                       */
    /*                                                                  */
    /********************************************************************/
    if ((FitToScreenPolicy(shell) != DXmNONE) &&
        IsAddedThatChanged(shell))
        return;
#endif


    if (((child = (XmBulletinBoardWidget) GetRectObjKid((CompositeWidget) shell)) == NULL) ||
	(child->core.being_destroyed))
      return;
    
    childIsBB = XmIsBulletinBoard(child);
    
    if (child->core.managed) 
      {
	  XtWidgetGeometry	request;
	  Position		kidX, kidY;
	  Dimension		kidBW;
	  Boolean		defaultPosition = True;

	  /*
	   * temporary workaround for setkeyboard focus |||
	   */
	  if (((Widget)child != ve->vendor.old_managed)
#ifdef notdef
	      &&(_XmGetFocusPolicy(child) == XmEXPLICIT)
#endif /* notdef */
	      )
	    {
		XtSetKeyboardFocus((Widget)shell, (Widget)child);
		ve->vendor.old_managed = (Widget)child;
	    }

	  /* 
	   * if the child isn't realized, then we need to realize it
	   * so we have a valid size. It will get created as a result
	   * so we  zero out it's position info so it'll
	   * be okay and then restore it.
	   */
	  if (!XtIsRealized(child))
	    {
		kidX = XtX(child);
		kidY = XtY(child);
		kidBW = XtBorderWidth(child);
		
		XtX(child) = 0;
		XtY(child) = 0;
		XtBorderWidth(child) = 0;
#ifdef DEC_MOTIF_BUG_FIX
		/*
		 *  This is a hack, but it solves the following problem.
		 *  There is a timimg problem here if a dialog shell
		 *  is managed while the window manager is starting up.
		 *  The problem occurs if the call to XtRealizeWidget
	   	 *  results in a geometry request, the window manager
		 *  has started and so the server sends the ConfigureRequest
		 *  to the window manager, but the window manager has
		 *  not completed startup and so doesn't respond to the
		 *  request in the 5 second timeout period.  When the
		 *  shell times out, it will return XtGeometryNo to
		 *  the child, which may leave it very small.  Then
		 *  later in this routine, the code ensures that the
		 *  shell takes on the size of the child, and so they
		 *  both end up very small.  The "hack-around" for
		 *  this is to "pretend" that the child is not managed
		 *  during this call.  Then all geometry requests will
		 *  succeed without involving the window manager (yet).
		 *  The later code that sets the shell to the size of the
		 *  child *will* involve the window manager.  But, even
		 *  if that request times out, the shell will end up the
		 *  correct size when the window manager eventually
		 *  configures the window.
		 */
		child->core.managed = FALSE;
		XtRealizeWidget((Widget) child);
		child->core.managed = TRUE;
#else
		XtRealizeWidget((Widget) child);
#endif
		
		XtX(child) = kidX;
		XtY(child) = kidY;
		XtBorderWidth(child) = kidBW;
	    }
	  
	  else if (childIsBB)
	    {
		/*  
		 *  Move the window to 0,0
		 *  but don't tell the widget.  It thinks it's where
		 *  the shell is...
		 */
		if ((XtX(child) != 0) || (XtY(child) != 0))
		  XMoveWindow (XtDisplay(child), 
			       XtWindow(child), 
			       0, 0);
	    }
	  /*
	   * TRY TO FIX 1.0 BUG ALERT!
	   *
	   * map callback should occur BEFORE bulletinBoard class default positioning
	   * otherwise, widgets such as fileselection using map callback for
	   * correct sizing have default positioning done before the widget 
	   * grows to its correct dimensions
	   */
          if(    shell->core.mapped_when_managed    )
	  {   CALLBACK ((Widget) child, XmNmapCallback, XmCR_MAP, NULL);	
              } 
	  /* 
	   * Make sure that the shell has the same common parameters as 
	   * its child.  Then move the child so that the shell will 
	   * correctly surround it.
	   */
	  request.request_mode = 0;
	  
	  if (childIsBB)
	    {
		defaultPosition =
		  child->bulletin_board.default_position;
		if (defaultPosition && (ve->vendor.externalReposition))
		  defaultPosition = 
		    child->bulletin_board.default_position = 
		      False;
	    }
	  if (XtX(child) && childIsBB)
	    {
		kidX = XtX(child);
		XtX(child) = 0;
	    }
	  else
	    kidX = XtX(shell);
	  
	  if (XtY(child) && childIsBB)
	    {
		kidY = XtY(child);
		XtY(child) = 0;
	    }
	  else
	    kidY = XtY(shell);
	  if (XtBorderWidth(child) && childIsBB)
	    {
		kidBW = XtBorderWidth(child);
		XtBorderWidth(child) = 0;
	    }
	  else
	    kidBW = XtBorderWidth(shell);
	  
	  if (XtWidth (child) != XtWidth (shell))
	    {
		request.request_mode |= CWWidth;
		request.width = XtWidth(child);
	    }
   	  if (XtHeight (child) != XtHeight (shell))
    	    {
		request.request_mode |= CWHeight;
		request.height = XtHeight(child) + ve->vendor.im_height;
	    }
	  
	  if (childIsBB)
	    {
		if (defaultPosition)
		  {
		      GetDefaultPosition(child,
					 XtParent(shell),
					 &request.x,
					 &request.y);
		      if (request.x != kidX)
			request.request_mode |= CWX;
		      if (request.y != kidY)
			request.request_mode |= CWY;
		  }
		else
		  {
		      if (kidX != XtX(shell))
			{
			    request.request_mode |= CWX;
			    if (kidX == MAGIC_VAL)
			      request.x = 0;
			    else
			      request.x = kidX;
			}
		      if (kidY != XtY(shell))
			{
			    request.request_mode |= CWY;
			    if (kidY == MAGIC_VAL)
			      request.y = 0;
			    else
			      request.y = kidY;
			}
		  }
	    }
	  else
	    {
		if (kidX != XtX(shell))
		  {
		      request.request_mode |= CWX;
		      request.x = kidX;
		  }
		if (kidY != XtY(shell))
		  {
		      request.request_mode |= CWY;
		      request.y = kidY;
		  }
		if (kidBW != XtBorderWidth(shell))
		  {
		      request.request_mode |= CWBorderWidth;
		      request.border_width = kidBW;
		  }
	    }

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
          /**************************************************************/
          /*                                                            */
          /* Override the width and height if this guy is too large     */
          /* to fit on the screen.                                      */
          /*                                                            */
          /**************************************************************/
          if (FitToScreenPolicy(shell) != DXmNONE)
              MakeShellFitScreen(shell,child,&hscroll_action,
                                             &vscroll_action,&request);
#endif


	  if (request.request_mode)
	  {
	    XtMakeGeometryRequest((Widget) shell, &request, &request);
	    _XmImResize((Widget)shell);
	  }

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
          /**************************************************************/
          /*                                                            */
          /* Now that the shell has been resized, put up some scrollbars*/
          /* if necessary.                                              */
          /*                                                            */
          /**************************************************************/
          if (FitToScreenPolicy(shell) != DXmNONE)
              HandleScrollBars(shell,child,hscroll_action,vscroll_action);
#endif


#ifdef notdef
	  /*
	   * Set the mapStyle to manage so that if we are externally
	   * unmapped by the wm we will be able to recover on reciept
	   * of the unmap notify and unmanage ourselves
	   */
	  ve->vendor.mapStyle = _XmMANAGE_MAP;
#endif /* notdef */

	  /*
	   * the grab_kind is handled in the popup_callback
	   */
          if(    shell->core.mapped_when_managed    )
	  {   XtPopup  ((Widget) shell, XtGrabNone);
              } 
      }
    /*
     * CHILD BEING UNMANAGED
     */
    else
      {
              Position	x, y;

           XtTranslateCoords(( Widget)shell,
                         -((Position) shell->core.border_width),
                              -((Position) shell->core.border_width), &x, &y) ;
#ifdef notdef
	  if (XmIsBulletinBoard(child))
	    {
		XtX(child) = x;
		XtY(child) = y;
	    }
	  /*
	   * update normal_hints even though we shouldn't need to
	   */
	  SetWMOffset(shell);
#endif

          /*
           * Fix for CR5043 -
           * For nested Dialog Shells, it is necessary to unmanage
           * dialog shell popups of the child of this dialog shell.
           */
#ifdef DEC_MOTIF_BUG_FIX
           if (child->core.num_popups)
#else
           if (child->core.popup_list)
#endif
             {
               if (XmIsDialogShell(child->core.popup_list[0]))
                 {
                   XmDialogShellWidget next_shell =
                     (XmDialogShellWidget)(child->core.popup_list[0]);
                   Widget next_bb = next_shell->composite.children[0];
                   if (next_bb)
                     XtUnmanageChild(next_bb);
                 }
             }
          /* End Fix CR5043 */
 
	  /*
	   * take it down and then tell user
	   */
	  
	  XtPopdown((Widget) shell);
	  
	  CALLBACK ((Widget) child, XmNunmapCallback, XmCR_UNMAP, NULL);	

#ifdef DEC_MOTIF_EXTENSION
          if (childIsBB &&
              child->bulletin_board.dxm_auto_unrealize)
          {
              XtUnrealizeWidget((Widget) child);
          }
#endif

      }
    _XmNavigChangeManaged((Widget) shell);
}                       


/************************************************************************
 *
 *  GeometryManager
 *
 ************************************************************************/
static XtGeometryResult 
#ifdef _NO_PROTO
GeometryManager( wid, request, reply )
        Widget wid ;
        XtWidgetGeometry *request ;
        XtWidgetGeometry *reply ;
#else
GeometryManager(
        Widget wid,
        XtWidgetGeometry *request,
        XtWidgetGeometry *reply )
#endif /* _NO_PROTO */
{
    ShellWidget 	shell = (ShellWidget)(wid->core.parent);
    XtWidgetGeometry 	my_request;
    XmVendorShellExtObject ve;
    XmWidgetExtData   extData;

#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
    XtGeometryResult    georesult;
    unsigned char       hscroll_action;
    unsigned char       vscroll_action;
    XmDialogShellWidget dsw = (XmDialogShellWidget)shell;

    /************************************************************/
    /*                                                          */
    /* If it is one of our added kids, just say "YES"           */
    /*                                                          */
    /************************************************************/
    if (IsAddedChild(shell, wid))
    {
        if (request->request_mode & CWBorderWidth)
            wid->core.border_width = request->border_width;
        if (request->request_mode & CWX)
            wid->core.x = request->x;
        if (request->request_mode & CWY)
            wid->core.y = request->y;
        if (request->request_mode & CWWidth)
            wid->core.width = request->width;
        if (request->request_mode & CWHeight)
            wid->core.height = request->height;
        return (XtGeometryYes);
    }
#endif


    extData = _XmGetWidgetExtData((Widget)shell, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;

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
	my_request.height = request->height + ve->vendor.im_height;
	my_request.request_mode |= CWHeight;
    }
    if (request->request_mode & CWBorderWidth) {
	my_request.border_width = request->border_width;
	my_request.request_mode |= CWBorderWidth;
    }


#ifdef DEC_MOTIF_EXTENSION
/* Automatic I14Y Code */
    /************************************************************/
    /*                                                          */
    /* Make sure that the dialog box does not extend beyond the */
    /* bounds of the screen.  This can happen if there is no    */
    /*  geometry manager to say "no" to this                    */
    /*                                                          */
    /************************************************************/
    if (FitToScreenPolicy(dsw) != DXmNONE)
        MakeShellFitScreen(dsw,wid,&hscroll_action,&vscroll_action,&my_request);

    georesult = XtMakeGeometryRequest((Widget)shell, &my_request, &my_request);

    if ((georesult == XtGeometryYes) &&
       (FitToScreenPolicy(dsw) == DXmNONE))
    {
        ConfigureDialog(wid, &my_request);

        if (XmIsBulletinBoard(wid))
            return XtGeometryDone;
        else
            return XtGeometryYes;
    }
    else if (FitToScreenPolicy(dsw) != DXmNONE)
    {
        /********************************************************/
        /*                                                      */
        /*  Let the child grow if it wants to and add scroll    */
        /*  bars if necessary.                                  */
        /*                                                      */
        /********************************************************/
        my_request.request_mode = 0;

        if ((request->request_mode & CWHeight) &&
            (request->height > XtHeight(wid)))
        {
            my_request.height = request->height;
            my_request.request_mode |= CWHeight;
        }
        if ((request->request_mode & CWWidth) &&
            (request->width > XtWidth(wid)))
        {
            my_request.width = request->width;
            my_request.request_mode |= CWWidth;
        }

        if (my_request.request_mode)
        {
            ConfigureDialog(wid, &my_request);
            MakeChildFitShell(dsw,wid,&hscroll_action,&vscroll_action);
            MakeShellFitScreen(dsw,wid,&hscroll_action,&vscroll_action,&my_request);
            XtMakeGeometryRequest((Widget)shell, &my_request, &my_request);
            HandleScrollBars(dsw,wid,hscroll_action,vscroll_action);

            if (XmIsBulletinBoard(wid))
                return XtGeometryDone;
            else
                return XtGeometryYes;
        }
    }
#else
 
    if (XtMakeGeometryRequest((Widget)shell, &my_request, NULL)
	== XtGeometryYes) {
          if (!(request->request_mode & XtCWQueryOnly)) {
	      /* just report the size changes to the kid, not
		 the dialog position itself */
	      if (my_request.request_mode & CWWidth)
		  wid->core.width = my_request.width ;
	      if (my_request.request_mode & CWHeight)
		  wid->core.height = my_request.height - ve->vendor.im_height;
	      _XmImResize((Widget)shell);
	  }
	  return XtGeometryYes;
      }
#endif 
      else 
	  return XtGeometryNo;
}


/*
 *************************************************************************
 *
 * Public creation entry points
 *
 *************************************************************************
 */
/*
 * low level create entry points
 */
Widget 
#ifdef _NO_PROTO
XmCreateDialogShell( p, name, al, ac )
        Widget p ;
        char *name ;
        ArgList al ;
        Cardinal ac ;
#else
XmCreateDialogShell(
        Widget p,
        char *name,
        ArgList al,
        Cardinal ac )
#endif /* _NO_PROTO */
{
    return (XtCreatePopupShell(name, xmDialogShellWidgetClass, p, al, ac));
}

