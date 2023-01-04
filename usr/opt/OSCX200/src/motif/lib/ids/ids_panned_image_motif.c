
/***********************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
***************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      Image Display Services (IDS)
**
**  ABSTRACT:
**
**      This module implements the IDS panned image widget for diplaying 
**	and panning ISL image frames in a DECwindows environment.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**
**  AUTHOR(S):
**
**      Robert NC Shelley
**
**  CREATION DATE:  January 8, 1988
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#undef DECWINDOWS
    /*
    **  ISL and IDS include files
    */
#define	    MOTIFPANNEDIMAGE   /* defined so IDS$$WIDGET_MOTIF.H knows who we are    */

#include <ids__widget_motif.h> /* IDS public/private, Xmtoolkit, Xlib defs  */
#include <img/ImgDef.h>
#ifndef NODAS_PROTO
#include <idsprot.h>            /* IDS prototypes */
#endif


/*
**  Table of contents
*/

    /*
    **  PUBLIC -- high level entry points
    */
#ifdef NODAS_PROTO
#ifdef VMS
Widget	IDSXM$PANNED_IMAGE();		/* VMS create panned image	    */
#endif
Widget	IdsXmPannedImage();		/* C   create panned image	    */

    /*
    **  PUBLIC -- low level entry points
    */
#ifdef VMS
Widget	IDSXM$PANNED_IMAGE_CREATE();	/* VMS create panned image widget   */
#endif
Widget	IdsXmPannedImageCreate();		/* C   create panned image widget   */

    /*
    **  Class record entry points exported to Intrinsics
    */
static void		ClassInitialize();	/* initialize panned class  */
static void             Initialize();           /* initialize panned widget */
static Boolean          SetValues();            /* change widget values     */
    /*
    **  internal routines   -- none --
    */
static void             GrabImage();            /* start pan: button down   */
static void             MoveImage();            /* pan image: pointer moved */
static void             DropImage();            /* end pan: button up       */
   

static void ZoomedFit();		/* Perform fit within            */
static void ZoomedStart();		/* Start Zoom -- button down     */
static void ZoomedMove();		/* Drawing bounding box for zoom */
static void ZoomedEnd();		/* End Zoom, button up           */
static void DrawRect();			/* Draw Rectangle for zoom region*/
static void CreateRoi();
#else
PROTO(static void ClassInitialize, (void));
PROTO(static void Initialize, (Widget /*req*/, Widget /*new*/, ArgList /*args*/, Cardinal */*num_args*/));
PROTO(static Boolean SetValues, (Widget /*old*/, Widget /*req*/, Widget /*new*/));
PROTO(static void GrabImage, (Widget /*pw*/, XButtonEvent */*event*/));
PROTO(static void MoveImage, (Widget /*pw*/, XButtonEvent */*event*/));
PROTO(static void DropImage, (Widget /*pw*/, XButtonEvent */*event*/));

PROTO(static void ZoomedFit, (Widget /*pw*/, XButtonEvent */*event*/, char */*tag*/));
PROTO(static void ZoomedStart, (Widget /*pw*/, XButtonEvent */*event*/, char */*tag*/));
PROTO(static void ZoomedMove, (Widget /*pw*/, XButtonEvent */*event*/, char */*tag*/));
PROTO(static void ZoomedEnd, (Widget /*pw*/, XButtonEvent */*event*/, char */*tag*/));
PROTO(static void DrawRect, (Widget /*pw*/));
PROTO(static void CreateRoi, (Widget /*pw*/));
#endif



/*
**  MACRO definitions -- (see also IDS$IMAGE.H and IDS$$MACROS.H)
*/

/*
**  Equated Symbols 
*/
#define PI 3.141592654


/*
**  External References
*/
static Boolean          RenderImage(); 
/*
**	Local Storage
*/
/* Panned Translations */

static char PannedTranslations[] =            /* View window has rubber band box  */
         "None<Btn3Down>:            IDS-zoomed-start()\n\
         Button3<Motion>:           IDS-zoomed-move()\n\
         None<Btn3Up>:              IDS-zoomed-end()\n\
         None<Btn2Up>:              IDS-zoomed-fit()\n\
	~Shift ~Ctrl ~Meta ~Alt ~@Help <Btn1Down>:     IDS-panned-grab()\n\
         ! Lock  Button1<Motion>:       IDS-panned-move()\n\
         ! Button1<Motion>:     IDS-panned-move()\n\
         ~Shift ~Ctrl ~Meta ~Alt ~@Help <Btn1Up>:       IDS-panned-drop()\n\
         @Help<Btn1Up>:             IDS-image-help()\n\
         <Visible>:                 IDS-visible()";
 

static XtActionsRec PannedActions[] = {
       {"IDS-panned-grab",      (XtActionProc)GrabImage},
       {"IDS-panned-move",      (XtActionProc)MoveImage},
       {"IDS-panned-drop",      (XtActionProc)DropImage},
       {"IDS-zoomed-start",      (XtActionProc)ZoomedStart},
       {"IDS-zoomed-move",      (XtActionProc)ZoomedMove},
       {"IDS-zoomed-end",      (XtActionProc)ZoomedEnd},
       {"IDS-zoomed-fit",      (XtActionProc)ZoomedFit}
       };

static XtResource PannedResources[] = {
    {	IdsNprotocol,	    IdsCProtocol,
	IdsRProtocol,	    sizeof(int),
        IdsPartOffset_(Render,RenderImage,context.protocol),
	XtRString,	    IdsSPixmap},
};




    /*
    **  Class record set at compile/link time, passed to the widgetcreate 
    **	routine as the the class.
    */
#ifdef VMS
externaldef($$z_ab_pannedImageWidgetMClassR) PannedImageMotifClassRec
	    pannedImageWidgetMotifClassRec = {
#else
externaldef(__z_ab_pannedImageWidgetMClassR) PannedImageMotifClassRec
	    pannedImageWidgetMotifClassRec = {
#endif

  /*	Core Class Part		*/
  {
    /* superclass	    */	(WidgetClass) &imageWidgetMotifClassRec,
    /* class_name	    */	IdsSClassPannedImage,
#ifdef __alpha
/*
** this resolves unalligned access errors but keeps the message box from
** going away automatically in the demo_motif program.  ASK_MARK
*/
    /* widget_size          */  sizeof(PannedImageRec),
#else
    /* widget_size          */  sizeof(PannedImagePart),
#endif
    /* class_initialize	    */  ClassInitialize,
    /* class_part_initialize*/	NULL,
    /* class_inited	    */	FALSE,
    /* initialize	    */	Initialize,
    /* initialize hook	    */	NULL,
    /* realize		    */  (XtRealizeProc) _XtInherit,
    /* actions		    */	PannedActions,
    /* num_actions	    */	XtNumber(PannedActions),
    /* resources	    */  (XtResourceList) PannedResources,
    /* num_resources	    */	XtNumber(PannedResources),
    /* xrm_class	    */	NULLQUARK,
    /* compress_motion	    */	TRUE,
    /* compress_exposure    */	TRUE,
    /* compress_enterleave  */	TRUE,
    /* visible_interest	    */	FALSE,
    /* destroy		    */	NULL,

    /* resize               */  (XtWidgetProc) _XtInherit,
    /* expose               */  (XtExposeProc) _XtInherit,
    /* set_values           */  (XtSetValuesFunc)SetValues,
    /* set_values_hook      */  NULL,
    /* set_values_almost    */  (XtAlmostProc) _XtInherit,
    /* get_values_hook      */  NULL,
    /* accept_focus	    */	NULL,
    /* version		    */	XtVersionDontCheck,
    /* callback_offsets	    */	NULL,
    /* tm_table		    */	PannedTranslations,
    /* query_geometry	    */	NULL,
    /* display_accelerator  */	NULL,
    /* extension	    */	NULL,
    },

  /* composite class fields  */
    {
        /* geometry_manager     */      (XtGeometryHandler) _XtInherit,
        /* change_managed       */      (XtWidgetProc) _XtInherit,
        /* insert_child         */      (XtWidgetProc) _XtInherit,
        /* delete_child         */      (XtWidgetProc) _XtInherit,
        /* extension            */      0
    },

  /*    Constraint class fields  */
    {
        /* constrain_resources  */      NULL,
        /* num_resources        */      0,
        /* size_record          */      0,
        /* constrain_initialize */      NULL,
        /* constrain_destroy    */      NULL,
        /* constrain_setvalues  */      NULL,
        /* extension            */      NULL,
    },
 
  /*    XmManager class fields  */
    {
        /* translations (XtTranslations) _XtInherit,*/ XtInheritTranslations, 
        /* get_resources                */    NULL,
        /* num_get_resources            */    0,
        /* get_constraint_resources     */    NULL,
        /* num_get_constraint_resources */    0,
        /* extension                    */    NULL,
    },
    
  /*	RenderImage Class Part	*/
    {
    /* offsets		    */	NULL,
    /* update_callback	    */	_XtInherit,
    /* render_image	    */	(IDS_BoolProc) _XtInherit,
    /* pad0		    */	_XtInherit,
    /* pad1		    */	_XtInherit,
    /* extension	    */	NULL,
    },
  /*	Image Class Part	*/
    {
    /* offsets		    */	NULL,
    /* redraw window	    */	(IDS_BoolProc) _XtInherit,
    /* redraw region	    */	_XtInherit,
    /* image callback	    */	_XtInherit,
    /* pad0		    */	_XtInherit,
    },
  /*	Panned Image Class Part	*/
    {
    /* offsets		    */	NULL,
    /* pad0		    */	NULL,
    /* pad1		    */	NULL,
    /* extension	    */	NULL
    },

};
externaldef(pannedImageWidgetMotifClass) PannedImageMotifClass
   pannedImageWidgetMotifClass = (PannedImageMotifClass) &pannedImageWidgetMotifClassRec;


/*****************************************************************************
**  IDSXM$PANNED_IMAGE
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS high level public entry: create a panned image widget.
**
**  FORMAL PARAMETERS:
**
**	parent	    - parent of widget
**	name$dsc    - name descriptor of widget
**	x	    - x placement within parent
**	y	    - y placement within parent
**	w	    - width  of panned image widget window desired
**	h	    - height of panned image widget window desired
**	fid	    - frame id of image to display
**	rend	    - application callback called when image is to be rendered
**	view	    - application callback called when image view changes
**	drag	    - application callback called while image is dragged
**	help	    - application callback called when help is requested
**
**  IMPLICIT INPUTS:
**
**      attributes of widget's parents and resource manager's data base
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Widget identifier of panned image widget created
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
#ifdef IdsVMS
Widget IDSXM$PANNED_IMAGE(parent, name$dsc, x,y, w,h, fid, rend, view, drag, help )
 Widget		*parent;
 VMS_STRING	 name$dsc;
 int		*x, *y;
 int		*w, *h;
 unsigned long	*fid;
 XtCallbackList	 rend;
 XtCallbackList	 view;
 XtCallbackList	 drag;
 XtCallbackList	 help;

{
    PannedImageWidget pw;
    Arg	al[16];
    char *name = (char *) DwtDescToNull( name$dsc );
    int ac = 0;

#ifdef TRACE
printf( "Entering Routine IDSXM$PANNED_IMAGE in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    SETARG_( al, ac, XmNx,		*x    );
    SETARG_( al, ac, XmNy,		*y    );
    SETARG_( al, ac, XmNwidth,		*w    );
    SETARG_( al, ac, XmNheight,	*h    );
    SETARG_( al, ac, IdsNfid,		*fid  );
    SETARG_( al, ac, IdsNrenderCallback, rend );
    SETARG_( al, ac, IdsNviewCallback,   view );
    SETARG_( al, ac, IdsNdragCallback,   drag );
    SETARG_( al, ac, XmNhelpCallback,   help );

    pw = (PannedImageWidget)
	 XtCreateWidget( name, pannedImageWidgetMotifClass, *parent, al, ac );
    XtFree( name );

#ifdef TRACE
printf( "Leaving Routine IDSXM$PANNED_IMAGE in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    return (Widget) pw;
}
#endif

/*****************************************************************************
**  IdsXmPannedImage
**
**  FUNCTIONAL DESCRIPTION:
**
**      C high level public entry: create a panned image widget.
**
**  FORMAL PARAMETERS:
**
**	parent	    - parent of widget
**	name	    - name of widget
**	x	    - x placement within parent
**	y	    - y placement within parent
**	w	    - width  of panned image widget window desired
**	h	    - height of panned image widget window desired
**	fid	    - frame id of image to display
**	rend	    - application callback called when image is to be rendered
**	view	    - application callback called when image view changes
**	drag	    - application callback called while image is dragged
**	help	    - application callback called when help is requested
**
**  IMPLICIT INPUTS:
**
**      attributes of widget's parents and resource manager's data base
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Widget identifier of panned image widget created
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
Widget IdsXmPannedImage( parent, name, x, y, w, h, fid, rend, view, drag, help )
Widget          parent;
 char           *name;
 int             x, y;
 int             w, h;
 unsigned long   fid;
 XtCallbackList  rend;
 XtCallbackList  view;
 XtCallbackList  drag;
 XtCallbackList  help;
{
    Arg al[16];
    int ac = 0;
    Widget  returnWidget;
  
#ifdef TRACE
printf( "Entering Routine IdsXmPannedImage in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    SETARG_( al, ac, XmNx,               x    );
    SETARG_( al, ac, XmNy,               y    );
    SETARG_( al, ac, XmNwidth,           w    );
    SETARG_( al, ac, XmNheight,  h    );
    SETARG_( al, ac, IdsNfid,            fid  );
    SETARG_( al, ac, IdsNrenderCallback, rend );
    SETARG_( al, ac, IdsNviewCallback,   view );
    SETARG_( al, ac, IdsNdragCallback,   drag );
    SETARG_( al, ac, XmNhelpCallback,   help );

    returnWidget = XtCreateWidget( name, 
		    (WidgetClass) pannedImageWidgetMotifClass, parent, al, ac );

#ifdef TRACE
printf( "Leaving Routine IdsXmPannedImage in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    return( returnWidget );
}

/*****************************************************************************
**  IDSXM$PANNED_IMAGE_CREATE
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS low level public entry: create a panned image widget.
**
**  FORMAL PARAMETERS:
**
**	parent	    - parent of widget
**	name$dsc    - name descriptor of widget
**	arglist	    - override argument list for widget
**	argCount    - number of arguments in arglist
**
**  IMPLICIT INPUTS:
**
**      attributes of widget's parents and resource manager's data base
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Widget identifier of panned image widget created
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
#ifdef IdsVMS
Widget IDSXM$PANNED_IMAGE_CREATE( parent, name$dsc, arglist, argCount )
 Widget		*parent;
 VMS_STRING	 name$dsc;
 Arg		*arglist;
 int		*argCount;
{
    PannedImageWidget pw;
    char *name = (char *) DwtDescToNull( name$dsc );

#ifdef TRACE
printf( "Entering Routine IDSXM$PANNED_IMAGE_CREATE in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    pw = (PannedImageWidget)
     XtCreateWidget(name, (WidgetClass) pannedImageWidgetMotifClass,*parent, arglist,*argCount);
    XtFree( name );

#ifdef TRACE
printf( "Leaving Routine IDSXM$PANNED_IMAGE_CREATE in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    return (Widget) pw;
}
#endif

/*****************************************************************************
**  IdsXmPannedImageCreate
**
**  FUNCTIONAL DESCRIPTION:
**
**      C low level public entry: create a panned image widget.
**
**  FORMAL PARAMETERS:
**
**	parent	    - parent of widget
**	name	    - name of widget
**	arglist	    - override argument list for widget
**	argCount    - number of arguments in arglist
**
**  IMPLICIT INPUTS:
**
**      attributes of widget's parents and resource manager's data base
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Widget identifier of panned image widget created
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
Widget IdsXmPannedImageCreate( parent, name, arglist, argCount )
 Widget  parent;
 char   *name;
 Arg    *arglist;
 int     argCount;
{
Widget	returnWidget;

#ifdef TRACE
printf( "Entering Routine IdsXmPannedImageCreate in module IDS_PANNED_IMAGE_MOTIF \n");
#endif

returnWidget = XtCreateWidget(name,pannedImageWidgetMotifClass,parent,arglist,argCount);

#ifdef TRACE
printf( "Leaving Routine IdsXmPannedImageCreate in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
 return( returnWidget );
}

/*****************************************************************************
**  ClassInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      Class initialization routine.
**	Called only the first time a widget of this class is initialized.
**      This function has a MOTIF call
**
**  FORMAL PARAMETERS:
**
**	none
**
*****************************************************************************/
static void ClassInitialize()
{
#ifdef TRACE
printf( "Entering Routine ClassInitialize in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    /*
    **	Create actual offsets to instance record parts and to resources.
    */
    XmResolvePartOffsets(pannedImageWidgetMotifClass,
                    &pannedImageWidgetMotifClassRec.panned_image_class.offsets);

#ifdef TRACE
printf( "Leaving Routine ClassInitialize in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
}
/*****************************************************************************
**  Initialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      Instance initialize routine, called once for each widget created.
**
**  FORMAL PARAMETERS:
**
**	req	- Panned Image Widget, as built from arglist.
**	new	- Panned Image Widget, as modified by superclasses.
**
*****************************************************************************/
static void Initialize( req, new, args, num_args )
    Widget  req, new;
    ArgList args;
    Cardinal *num_args;
{

    DefineWidgetParts_(PannedImage,new);

#ifdef TRACE
printf( "Entering Routine Initialize in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    Panning_(new) = FALSE;		        /* set to initial state	    */
    Zooming_(new) = FALSE; 
    Zwidth_(new) = 0;
    Zheight_(new) = 0;
#ifdef TRACE
printf( "Leaving Routine Initialize in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
}


/*****************************************************************************
**  SetValues
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will take care of any changes that the application makes.
**
**  FORMAL PARAMETERS:
**
**	old	- Panned Image Widget, copy of original.
**	req	- Panned Image Widget, as built from arglist.
**	new	- Panned Image Widget, as modified by superclasses.
**
**  FUNCTION VALUE:
**
**	FALSE	nothing can change that requires a re-display
**
*****************************************************************************/
static Boolean SetValues( old, req, new )
    Widget  old, req, new;
{
    DefineWidgetParts_(PannedImage,old);
    DefineWidgetParts_(PannedImage,new);

#ifdef TRACE
printf( "Entering Routine SetValues in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    /*
    **	Update the drag callback (our only value added resource).
    */
    UpdateCallback_(old, &DragCB_(old), new, &DragCB_(new), IdsNdragCallback);

#ifdef TRACE
printf( "Leaving Routine SetValues in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    return( FALSE );	    /* never tell Intrinsics to clear our window    */
}

/*****************************************************************************
**  GrabImage
**
**  FUNCTIONAL DESCRIPTION:
**
**      Grab image: start of panning.  Called from Intrinsics at button down.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
**	event	- XButtonEvent structure
**
*****************************************************************************/
static void GrabImage( pw, event )
    Widget	  pw;
    XButtonEvent *event;
{
    DefineWidgetParts_(PannedImage,pw);

#ifdef TRACE
printf( "Entering Routine GrabImage in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    /*
    **	If we're realized and there is a rendered image to pan...
    */
    if( XtIsRealized(pw) && ( cFid_(pw) || cXieImg_(pw) ))
	{
	/*
	**	Save initial image display coordinates.
	*/
	BegX_(pw) = SrcW_(pw) < WrkW_(pw) ? cWrkX_(pw) : cSrcX_(pw);
	BegY_(pw) = SrcH_(pw) < WrkH_(pw) ? cWrkY_(pw) : cSrcY_(pw);

	/*
	**	Save initial mouse coordinates.
	*/
	PanX_(pw) = event->x;
	PanY_(pw) = event->y;

	/*
	**  Enable panning.
	*/
	Panning_(pw) = TRUE;
	}
#ifdef TRACE
printf( "Leaving Routine GrabImage in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  MoveImage
**
**  FUNCTIONAL DESCRIPTION:
**
**      Pan image: Called from Intrinsics because of compressed mouse movement.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
**	event	- XButtonEvent structure
**
*****************************************************************************/
static void MoveImage( pw, event )
Widget	  pw;
XButtonEvent *event;
    {
    DefineWidgetParts_(PannedImage,pw);
    RenderContext ctx = &Context_(pw);
    int sx, sy, wx, wy;

#ifdef TRACE
printf( "Entering Routine MoveImage in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    if( Panning_(pw))
	{
	Drag_(pw) = TRUE;
	/*
	**  Compute new coordinates.
	*/
	sx = cSrcX_(pw) + PanX_(pw) - event->x;
	sy = cSrcY_(pw) + PanY_(pw) - event->y;
	wx = cWrkX_(pw) - PanX_(pw) + event->x;
	wy = cWrkY_(pw) - PanY_(pw) + event->y;
	/*
	**  Keep new coordinates within src/wrk limits.
	*/
	rSrcX_(pw)   = sx <= 0 ? 0 : MIN(sx, Iwide_(pw) - SrcW_(pw));
	rSrcY_(pw)   = sy <= 0 ? 0 : MIN(sy, Ihigh_(pw) - SrcH_(pw));
	rWrkX_(pw)   = wx <= 0 ? 0 : MIN(wx,  WrkW_(pw) - SrcW_(pw));
	rWrkY_(pw)   = wy <= 0 ? 0 : MIN(wy,  WrkH_(pw) - SrcH_(pw));

	Redraw_(pw) |= rSrcX_(pw) != cSrcX_(pw) || rSrcY_(pw) != cSrcY_(pw)
		    || rWrkX_(pw) != cWrkX_(pw) || rWrkY_(pw) != cWrkY_(pw);
	/*
	**  Re-draw at new coordinates, update scrollbars (if dynamic),
	**  and notify application (if drag callback).
	*/
        if( Redraw_(pw) && (RedrawWindow_( pw ) ||
                                VxtImagingOption(XtDisplay(pw), ctx)))
	    ImageCallback_(pw, DoBars_(pw) && DynBar_(pw),
				IdsCRDragImage,event);
	/*
	**  Save current mouse position.
	*/
        PreviousX_(pw) = PanX_(pw);
        PreviousY_(pw) = PanY_(pw);

	PanX_(pw) = event->x;
	PanY_(pw) = event->y;
	}
#ifdef TRACE
printf( "Leaving Routine MoveImage in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  DropImage
**
**  FUNCTIONAL DESCRIPTION:
**
**      End panning.  Callback application with new display coordinates.
**		      Called from Intrinsics at button up.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
**	event	- XButtonEvent structure
**
*****************************************************************************/
static void DropImage( pw, event )
    Widget	  pw;
    XButtonEvent *event;
{
    DefineWidgetParts_(PannedImage,pw);
    RenderContext ctx = &Context_(pw);
    int sx, sy, wx, wy;

#ifdef TRACE
printf( "Entering Routine DropImage in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    if ( Panning_(pw)) 
	{
	if (!(VxtImagingOption(XtDisplay(pw), ctx)))
	    {
	    MoveImage( pw, event );	/* Re-draw at final position	    */
	    Panning_(pw) = FALSE;	/* turn off panning mode	    */
	    }
        else
	    {
	    if (Drag_(pw))
		{
		Drag_(pw) = FALSE;
	        /*
	        **  Compute new coordinates.
    	        */
                sx = cSrcX_(pw) + PreviousX_(pw) - event->x;
                sy = cSrcY_(pw) + PreviousY_(pw) - event->y;
                wx = cWrkX_(pw) - PreviousX_(pw) + event->x;
                wy = cWrkY_(pw) - PreviousY_(pw) + event->y;

     	        /*
	        **  Keep new coordinates within src/wrk limits.
	        */

	        rSrcX_(pw)   = sx <= 0 ? 0 : MIN(sx, Iwide_(pw) - SrcW_(pw));
	        rSrcY_(pw)   = sy <= 0 ? 0 : MIN(sy, Ihigh_(pw) - SrcH_(pw));
	        rWrkX_(pw)   = wx <= 0 ? 0 : MIN(wx,  WrkW_(pw) - SrcW_(pw));
	        rWrkY_(pw)   = wy <= 0 ? 0 : MIN(wy,  WrkH_(pw) - SrcH_(pw));

	        Redraw_(pw) = TRUE; /* Force redraw on drop if atom exists */

	        /*
	        **  Re-draw at new coordinates, update scrollbars (if dynamic),
	        **  and notify application (if drag callback).
	        */
	        Panning_(pw) = FALSE;	/* turn off panning mode	    */

                if( Redraw_(pw) && (RedrawWindow_( pw ) ||
                                VxtImagingOption(XtDisplay(pw), ctx)))
 	            ImageCallback_(pw, DoBars_(pw) && DynBar_(pw),
				IdsCRDragImage,event);

	        /*
	        **  Save current mouse position.
	        */
	        PanX_(pw) = event->x;
	        PanY_(pw) = event->y;
		}
	    }

	/*
	**  Callback application if beginning and final coordinates differ.
	*/
	BegX_(pw) -= SrcW_(pw) < WrkW_(pw) ? cWrkX_(pw) : cSrcX_(pw);
	BegY_(pw) -= SrcH_(pw) < WrkH_(pw) ? cWrkY_(pw) : cSrcY_(pw);

        if (VxtImagingOption(XtDisplay(pw), ctx))
            {
            rSrcX_(pw) = 0;
            rSrcY_(pw) = 0;
            }

	ImageCallback_(pw, DoBars_(pw), IdsCRViewChanged, event );
 	}
#ifdef TRACE
printf( "Leaving Routine DropImage in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ZoomedStart
**
**  FUNCTIONAL DESCRIPTION:
**
**      Start of zooming.  Draw intial rectangle, initialize zooming
** 	fields.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
**	event	- XButtonEvent structure
**	tag	- Widget id
*****************************************************************************/
static void ZoomedStart(pw, event, tag)
Widget	  pw;
XButtonEvent *event;
char *tag;
    {
    DefineWidgetParts_(PannedImage,pw);
    static XGCValues        my_values;
    RenderContext ctx = &Context_(pw);

#ifdef TRACE
printf( "Entering Routine ZoomedStart in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    /* If there's no fid, just return */
    if ( (!cFid_(pw)) && (!cXieImg_(pw)) )
        return;

    /* load up the zoom structure with the initial settings for zoom */
    ZstartX_(pw) = event->x;
    ZstartY_(pw) = event->y;
    Zwidth_(pw) = 1;
    Zheight_(pw) = 1;
    Zooming_(pw) = TRUE;
    /* create the GC for the DrawRectangle function */
    my_values.line_width = 1;
    my_values.function = GXinvert;
    ZGC_(pw) = XCreateGC (Dpy_(ctx), Win_(ctx), GCFunction | GCLineWidth,
			        &my_values);

    /* draw the initial rectangle */
    DrawRect(pw);
#ifdef TRACE
printf( "Leaving Routine ZoomedStart in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ZoomedEnd
**
**  FUNCTIONAL DESCRIPTION:
**
**      End of zooming.  Button has been released, display image with 
** 	new coordinates.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
**	event	- XButtonEvent structure
**	tag	- Widget id
**
*****************************************************************************/
static void ZoomedEnd(pw, event, tag)
Widget	  pw;
XButtonEvent *event;
char      *tag;
    {
    DefineWidgetParts_(PannedImage,pw);
    RenderContext ctx = &Context_(pw);
    IdsRenderCallback rcb = PropRnd_(ctx);
    Arg  al[2];
    long ac = 0;
    
#ifdef TRACE
printf( "Entering Routine ZoomedEnd in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    /* if there is no fid, just return */
    if ( (!cFid_(pw)) && (!cXieImg_(pw)) )
	{
#ifdef TRACE
printf( "Leaving Routine ZoomedEnd in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
        return;
	}

    /* if we're not currently zooming, return */
    if (!Zooming_(pw))
	{
#ifdef TRACE
printf( "Leaving Routine ZoomedEnd in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
        return;
	}

    /* Draw the final rectangle */
    DrawRect(pw);

    /* set the width and height of the area to zoom */

    Zwidth_(pw) = event->x - ZstartX_(pw);
    Zheight_(pw) = event->y - ZstartY_(pw);

    /* Do nothing if the height and width is zero */

    if ((Zwidth_(pw) == 0) || (Zheight_(pw) == 0)) 
	{
        Zooming_(pw) = FALSE;
#ifdef TRACE
printf( "Leaving Routine ZoomedEnd in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
        return;
        }


    /* If the bounding box was drawn to the top and left, set
    ** the height and width appropriately.
    */
    if (Zwidth_(pw) < 0) 
	{
        ZstartX_(pw) += Zwidth_(pw);
        Zwidth_(pw) = Zwidth_(pw) * -1;
    	}

    if (Zheight_(pw) < 0) 
	{
        ZstartY_(pw) += Zheight_(pw);
        Zheight_(pw) = Zheight_(pw) * -1;
        }

    /* do nothing if the height or width are less than or equal to 2 */
    if ((Zwidth_(pw) <= 2) || (Zheight_(pw) <= 2)) 
	{
        Zooming_(pw) = FALSE;
#ifdef TRACE
printf( "Leaving Routine ZoomedEnd in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
        return;
        }


    /* Make sure the bounding box fits within the image */

    if ((SrcW_(pw) != 0) && ( Zwidth_(pw) + ZstartX_(pw) ) >= SrcW_(pw)) 
	{
        Zwidth_(pw) = SrcW_(pw) - ZstartX_(pw);
        }

    if ((SrcH_(pw) != 0) && (Zheight_(pw) + ZstartY_(pw) ) >= SrcH_(pw)) 
	{
        Zheight_(pw) = SrcH_(pw) - ZstartY_(pw);
        }

    pReason_(pw)= IdsCRNormal;
    pRender_(pw) = Ids_Passive;
    Redraw_(pw) = TRUE;
	
    if (pSMode_(pw) == Ids_NoScale)
        pSMode_(pw) = Ids_FitWithin;

    if (VxtImagingOption(XtDisplay(pw), ctx))
        RedrawWindow_(pw);	
    else
	CreateRoi (pw);

    /* turn off zooming */
    Zooming_(pw) = FALSE;
    XFreeGC (Dpy_(ctx), ZGC_(pw) );
    ImageCallback_(pw, DoBars_(pw), IdsCRZoomImage,event);
    ImageCallback_(pw, DoBars_(pw), IdsCRViewChanged, event );
#ifdef TRACE
printf( "Leaving Routine ZoomedEnd in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    }

/*****************************************************************************
**  CreateRoi
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a Roi for Zoom on client side.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
*****************************************************************************/
static void CreateRoi(pw)
Widget	  pw;
    {
    DefineWidgetParts_(PannedImage,pw);
    long roix = 0;
    long roiy = 0;
    long roih = 0;
    long roiw = 0;
    struct ROI_RECT roi_rect;
    unsigned long roi_id;
    Arg  al[2];
    long ac = 0;
	
#ifdef TRACE
printf( "Entering Routine CreateRoi in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    ZoomingCheckMotif(pw,
            &roix,
            &roiy,
            &roih,
            &roiw);

    roi_rect.RoiL_RectUlx       = roix;
    roi_rect.RoiL_RectUly       = roiy;
    roi_rect.RoiL_RectPxls      = (unsigned long) roiw;
    roi_rect.RoiL_RectScnlns    = (unsigned long) roih;

    roi_id = ImgCreateRoi(
                        ImgK_RoitypeRect,
                        &roi_rect,
                        sizeof(struct ROI_RECT)
                        );

    SETARG_( al, ac, IdsNroi, roi_id); 
    SETARG_( al, ac, IdsNscaleMode, Ids_FitWithin);
    XtSetValues( pw, al, ac );
#ifdef TRACE
printf( "Leaving Routine CreateRoi in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    }

/*****************************************************************************
**  DrawRect
**
**  FUNCTIONAL DESCRIPTION:
**
**	Draw a rectangle.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
*****************************************************************************/
static void DrawRect(pw)
Widget	  pw;
    {
    DefineWidgetParts_(PannedImage,pw);
    Position x, y;
    Dimension width, height;
    RenderContext ctx = &Context_(pw);

#ifdef TRACE
printf( "Entering Routine DrawRect in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    /* get the starting x & y and the width and height. */
    x = ZstartX_(pw);
    y = ZstartY_(pw);
    width = Zwidth_(pw);
    height = Zheight_(pw);

    /* adjust the rectangle if width is negative */
    if (Zwidth_(pw) < 0) 
	{
        x = ZstartX_(pw) + Zwidth_(pw);
        width = Zwidth_(pw) * -1;
    	}

    /* adjust the rectangle if height is negative */
    if (Zheight_(pw) < 0) 
	{
        y = ZstartY_(pw) + Zheight_(pw);
        height = Zheight_(pw) * -1;
        }

    XDrawRectangle(Dpy_(ctx), Win_(ctx), ZGC_(pw), x, y, width, height);
#ifdef TRACE
printf( "Leaving Routine DrawRect in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ZoomedFit
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs an automatic scale to fit of the image.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
**	event	- XButtonEvent structure
**	tag	- Widget id
*****************************************************************************/
static void ZoomedFit(pw, event, tag)
Widget	  pw;
XButtonEvent *event;
char *tag;
    {
    DefineWidgetParts_(PannedImage,pw);
    RenderContext ctx = &Context_(pw);
    Arg  al[2];
    long ac = 0;

#ifdef TRACE
printf( "Entering Routine ZoomedFit in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    /* If there is no fid, just return */
    if ( (!cFid_(pw)) && (!cXieImg_(pw)) )
        return;

    ZstartX_(pw) = 0;
    ZstartY_(pw) = 0;
    Zwidth_(pw)  = 0;
    Zheight_(pw) = 0;
    Zooming_(pw) = FALSE;
    rSrcX_(pw)   = 0;
    rSrcY_(pw)   = 0;
    rWrkX_(pw)   = 0;
    rWrkY_(pw)   = 0;
    pReason_(pw) = IdsCRNormal;
    pRender_(pw) = Ids_Passive;

    if (VxtImagingOption(XtDisplay(pw), ctx))
	{
        Redraw_(pw) = TRUE;
        if (pSMode_(pw) == Ids_NoScale)
            pSMode_(pw) = Ids_FitWithin;

        RedrawWindow_( pw);
	}
    else
	{
        SETARG_( al, ac, IdsNroi, 0); 
        SETARG_( al, ac, IdsNscaleMode, Ids_FitWithin);
        XtSetValues( pw, al, ac );
	}

    ImageCallback_(pw, DoBars_(pw), IdsCRViewChanged, event );
#ifdef TRACE
printf( "Leaving Routine ZoomedFit in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    }

/*****************************************************************************
**  ZoomedMove
**
**  FUNCTIONAL DESCRIPTION:
** 
**	Moved the Zooming bounding box.
**	This routine handles Redrawing the new rectangle for zooming.
**
**  FORMAL PARAMETERS:
**
**	pw	- panned image widget 
**	event	- XButtonEvent structure
**	tag	- Widget id
*****************************************************************************/
static void ZoomedMove(pw, event, tag)
Widget	  pw;
XButtonEvent *event;
char *tag;
    {
    DefineWidgetParts_(PannedImage,pw);

#ifdef TRACE
printf( "Entering Routine xxx in module IDS_PANNED_IMAGE_MOTIF \n");
#endif

    if (!Zooming_(pw))
	{
#ifdef TRACE
printf( "Leaving Routine xxx in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
        return;
	}

    DrawRect(pw);
    Zwidth_(pw) = event->x - ZstartX_(pw);
    Zheight_(pw) = event->y - ZstartY_(pw);
    DrawRect(pw);
#ifdef TRACE
printf( "Leaving Routine xxx in module IDS_PANNED_IMAGE_MOTIF \n");
#endif
    }
