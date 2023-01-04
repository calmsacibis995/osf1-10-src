
/****************************************************************************
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
**      This module implements the IDS image widget for diplaying 
**	ISL image frames in a DECwindows environment.
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
**      Subu Garikapati
**
*****************************************************************************/
    /*
    **	Standard C include files
    */

#include <math.h>	    /* math routines				    */

    /*
    **  ISL and IDS include files
    */
#define     MOTIFIMAGE         /* define so IDS$$WIDGET_MOTIF.H knows who we are     */

#include <ids__widget_motif.h> /* IDS public/private, Xmtoolkit, Xlib defs. */

#ifndef NODAS_PROTO
#include <idsprot.h>            /* IDS prototypes */
#endif
#ifdef ASK_MARK
#if defined(NULL)
#undef NULL
#endif
#define NULL 0
#endif


/*
**  Table of contents
*/
    /*
    **  PUBLIC -- high level entry points
    */
#ifdef NODAS_PROTO
#ifdef VMS
Widget	IDSXM$STATIC_IMAGE();		/* VMS create image widget	    */
#endif
Widget	IdsXmStaticImage();		/* C   create image widget	    */
    /*
    **  PUBLIC -- low level entry points
    */
#ifdef VMS
Widget  IDSXM$STATIC_IMAGE_CREATE(); /* VMS: create static image widget	    */
XPoint *IDSXM$GET_COORDINATES();   /* VMS: Compute pre-rendered coordinates */
void	IDSXM$REDISPLAY_IMAGE();   /* VMS: re-display at new coordinates    */
void	IDSXM$APPLY_GRAVITY();	   /* VMS: re-display using gravity	    */
#endif
Widget  IdsXmStaticImageCreate();  /* C:   create static image widget	    */
XPoint *IdsXmGetCoordinates();	   /* C:   Compute pre-rendered coordinates */
void	IdsXmRedisplayImage();	   /* C:   re-display at new coordinates    */
void	IdsXmApplyGravity();	   /* C:   re-display using gravity	    */
    /*
    **  Class entry points exported via the Image Class Record.
    */
static void		ClassInitialize();	/* Image class init	    */
static void		ClassPartInitialize();	/* Image class inheritance  */
static void		Initialize();		/* Image instance init	    */
static void		Realize();		/* realize image widget	    */
static void		Destroy();		/* destroy image widget	    */
static void		Resized();		/* resize image widget	    */
static void		Exposed();		/* re-draw image in window  */
static Boolean		SetValues();		/* change widget values	    */
static XtGeometryResult	GeometryManager();	/* always say NO	    */
static Boolean		RedrawWindow();		/* Redraw the entire window */
static Boolean		RerenderWindowMotif();	/* Render the entire window */
static void		RedrawRegion();		/* Redraw an exposed region */
static void		ImageCallback();	/* Application callback	    */
void			ZoomingCheckMotif();	/* Handles zoom setup       */
    /*
    **  Class actions exported to Intrinsics Translation Manager
    */
static void		VisibilityChange();	/* visibility notification  */
static void		ScrollComplete();	/* Extended scroll complete */
    /*
    **  internal routines
    */
static void		CreateBars();		/* create private scrollbars*/
static void		ConfigureBars();	/* size and place scrollbars*/
static void		ManageBar();		/* (un)manage scrollbar	    */
static void		ManageBox();		/* (un)manage scrollbox	    */
static void		HorizontalCallback();	/* H scrollbar callbacks    */
static void		VerticalCallback();	/* V scrollbar callbacks    */
static void		ImageToWindow();	/* transfer image to window */
static void		SetWrkArea();		/* set size of Wrk area	    */
static Boolean		LimitSrcWrk();		/* Src(x,y)(w,h) & Wrk(x,y) */
static void		ApplyGravity();		/* initial Src/Wrk (x,y)    */
static void		ApplyScheme();		/* do RenderImage w/ Scheme */
static void		SendExpose();		/* send XExpose to ourself  */

#else
PROTO(static void ClassInitialize, (void));
PROTO(static void ClassPartInitialize, (ImageMotifClass /*ic*/));
PROTO(static void Initialize, (Widget /*req*/, Widget /*new*/, ArgList /*args*/, Cardinal */*num_args*/));
PROTO(static void Realize, (Widget /*iw*/, Mask */*valuemask*/, XSetWindowAttributes */*attributes*/));
PROTO(static void Destroy, (Widget /*iw*/));
PROTO(static void Resized, (Widget /*iw*/));
PROTO(static void Exposed, (Widget /*iw*/, XEvent */*event*/, Region /*region*/));
PROTO(static Boolean SetValues, (Widget /*old*/, Widget /*req*/, Widget /*new*/));
PROTO(static XtGeometryResult GeometryManager, (Widget /*iw*/, XtWidgetGeometry */*desired*/, XtWidgetGeometry */*allowed*/));
PROTO(static Boolean RedrawWindow, (Widget /*iw*/));
PROTO(static void RedrawRegion, (Widget /*iw*/, XExposeEvent */*expose*/));
PROTO(static void ImageCallback, (Widget /*iw*/, int /*bars*/, int /*reason*/, XEvent */*event*/));
PROTO(static void VisibilityChange, (Widget /*iw*/, XVisibilityEvent */*event*/));
PROTO(static void ScrollComplete, (Widget /*w*/, XEvent */*event*/));
PROTO(static void CreateBars, (Widget /*iw*/));
PROTO(static void ConfigureBars, (Widget /*iw*/));
PROTO(static void ManageBar, (Widget /*bar*/, int /*src*/, int /*wrk*/, int /*img*/, int /*inc*/, XmVoidProc /*drag*/));
PROTO(static void ManageBox, (Widget /*iw*/));
PROTO(static void HorizontalCallback, (Widget /*w*/, Opaque /*param*/, XmScrollBarCallbackStruct */*cbs*/));
PROTO(static void VerticalCallback, (Widget /*w*/, Opaque /*param*/, XmScrollBarCallbackStruct */*cbs*/));
PROTO(static void ImageToWindow, (Widget /*iw*/, int /*ix*/, int /*iy*/, int /*wx*/, int /*wy*/, int /*w*/, int /*h*/));
PROTO(static void SetWrkArea, (Widget /*iw*/, int /*width*/, int /*height*/));
PROTO(static Boolean LimitSrcWrk, (Widget /*iw*/));
PROTO(static void ApplyGravity, (Widget /*iw*/, int /*src*/, int /*dst*/));
PROTO(static void ApplyScheme, (Widget /*iw*/, XExposeEvent */*expose*/));
PROTO(static void SendExpose, (Widget /*iw*/));
PROTO(static Boolean RerenderWindowMotif, (Widget /*iw*/, long /*flag*/));
#endif

/*
**  MACRO definitions -- (see also IdsImage.h and IDS$$MACROS.H)
*/
#define TotalWidth_(w)  (XtWidth (w)+XtBorderWidth(w))
#define TotalHeight_(w) (XtHeight(w)+XtBorderWidth(w))

/*
**  Equated Symbols
*/
#define DEFAULT_WIDTH	100
#define DEFAULT_HEIGHT	100
#define INVALID_COORD	0x80000001
/*
**  External References
*/
#ifdef NODAS_PROTO
extern Boolean IdsXmCompareRenderings(); /* Compare Proposed/Current renderings */
#endif
/*
**	Local Storage
*/
    /*
    **	default translations and actions records
    */
static char ImageTranslations[] =
	"@Help<Btn1Up>:	    IDS-image-help()\n\
	 <Visible>:	    IDS-visible()";
static XtActionsRec ImageActions[] =
	{{"IDS-visible",    (XtActionProc)VisibilityChange},};
    /*
    **	additional scrollbar translations and actions records
    */
static char ScrollPage [] =
    "~Shift ~Ctrl ~Meta ~Alt ~@Help<Btn1Down>:\
                                DWTSCGRABTHUMB()\n\
     None<Btn1Up>:              DWTSCTRIGGER()	   IDS-scroll-complete()\n\
     Lock<Btn1Up>:              DWTSCTRIGGER()     IDS-scroll-complete()\n\
     ~Shift ~Ctrl ~Meta ~Alt ~@Help ~Button1<Btn2Up>:\
                                DWTSCGOTOTOP(0)\n\
     ~Shift ~Ctrl ~Meta ~Alt ~@Help ~Button1<Btn3Up>:\
                                DWTSCGOTOBOTTOM(0)\n\
     ~@Help <Btn1Up>:           DWTSCCANCELTIMER() IDS-scroll-complete()\
                                DWTSCCANCELDRAG()  IDS-scroll-complete()\n\
     Button1<Btn2Down>:         DWTSCCANCELTIMER()\
                                DWTSCCANCELDRAG()\n\
     Button1<Btn3Down>:         DWTSCCANCELTIMER()\
                                DWTSCCANCELDRAG()\n\
     @Help<Btn1Up>:             DWTSCCANCELTIMER() IDS-scroll-complete()\
                                DWTSCCANCELDRAG()  IDS-scroll-complete()\
                                DWTSCHELP()        IDS-scroll-complete()";

static XtTranslations ScrollPageParsed;
static XtActionsRec ScrollActions [] =
	{{"IDS-scroll-complete",  (XtActionProc)ScrollComplete},};

static XtResource ImageResources[] = {
    {	IdsNsourceX,		IdsCSourceX,
	XtRInt,		        sizeof(int),
        IdsPartOffset_(Image,Image,request_coords.source_x),
	XtRImmediate,		(caddr_t) 0 },

    {	IdsNsourceY,		IdsCSourceY,
	XtRInt,			sizeof(int),
        IdsPartOffset_(Image,Image,request_coords.source_y),
	XtRImmediate,		(caddr_t) 0 },

    {	IdsNsourceWidth,	IdsCSourceWidth,
	XtRInt,		        sizeof(int),
        IdsPartOffset_(Image,Image,source_width),
	XtRImmediate,		(caddr_t) 0 },

    {	IdsNsourceHeight,	IdsCSourceHeight,
	XtRInt,			sizeof(int),
        IdsPartOffset_(Image,Image,source_height),
	XtRImmediate,		(caddr_t) 0 },

    {	IdsNsourceGravity,	IdsCSourceGravity,
	IdsRGravity,		sizeof(int),
        IdsPartOffset_(Image,Image,source_gravity),
	XtRString,		IdsSNorthWest },

    {	IdsNwindowGravity,	IdsCWindowGravity,
	IdsRGravity,		sizeof(int),
        IdsPartOffset_(Image,Image,window_gravity),
	XtRString,		IdsSNorthWest },

    {	IdsNwindowX,		IdsCWindowX,
	XtRInt,			sizeof(int),
        IdsPartOffset_(Image,Image,request_coords.work_x),
	XtRImmediate,		(caddr_t) 0 },

    {	IdsNwindowY,		IdsCWindowY,
	XtRInt,			sizeof(int),
        IdsPartOffset_(Image,Image,request_coords.work_y),
	XtRImmediate,		(caddr_t) 0 },

    {	IdsNwindowWidth,	IdsCWindowWidth,
	XtRInt,			sizeof(int),
        IdsPartOffset_(Image,Image,work_width),
	XtRImmediate,		(caddr_t) 0 },

    {	IdsNwindowHeight,	IdsCWindowHeight,
	XtRInt,			sizeof(int),
        IdsPartOffset_(Image,Image,work_height),
	XtRImmediate,		(caddr_t) 0 },

    {   IdsNviewCallback,	XtCCallback, 
	XtRCallback,		sizeof(XtCallbackList ),
        IdsPartOffset_(Image,Image,view_callback),
	XtRCallback,		NULL },


    {	XmNexposeCallback,	XtCCallback,
	XtRCallback,		sizeof(XtCallbackList ),
        IdsPartOffset_(Image,Image,expose_callback),
	XtRCallback,		NULL },

    {   IdsNdragCallback,	XtCCallback, 
	XtRCallback,		sizeof(XtCallbackList),
        IdsPartOffset_(Image,Image,drag_callback),
	XtRCallback,		NULL},

    {   IdsNzoomCallback,	XtCCallback, 
	XtRCallback,		sizeof(XtCallbackList),
        IdsPartOffset_(Image,Image,zoom_callback),
	XtRCallback,		NULL},

    {	XmNincrement,		XmCIncrement,
	XtRInt,			sizeof(int),
        IdsPartOffset_(Image,Image,increment_bar),
	XtRImmediate,		(caddr_t) 1 },

    {	IdsNscrollHorizontal,	IdsCScrollHorizontal,
	XtRBoolean,		sizeof(Boolean),
        IdsPartOffset_(Image,Image,enable_horizontal),
	XtRImmediate,		(caddr_t) False },

    {	IdsNscrollVertical,	IdsCScrollVertical,
	XtRBoolean,		sizeof(Boolean),
        IdsPartOffset_(Image,Image,enable_vertical),
	XtRImmediate,		(caddr_t) False },

    {	IdsNscrollDynamic,	IdsCScrollDynamic,
	XtRBoolean,		sizeof(Boolean),
        IdsPartOffset_(Image,Image,enable_dynamic),
	XtRImmediate,		(caddr_t) False },
    };

    /*
    **  Class record set at compile/link time, passed to the widgetcreate
    **  routine as the the class.
    */
#ifdef VMS
externaldef($$z_aa_imageWidgetMClassR) ImageMotifClassRec
	    imageWidgetMotifClassRec = {
#else
externaldef($$z_aa_imageWidgetMClassR) ImageMotifClassRec
	    imageWidgetMotifClassRec = {
#endif

  /*	Core Class Part		*/
  {
    /* superclass	    */	(WidgetClass) &renderImageWidgetMotifClassRec,
    /* class_name	    */	IdsSClassImage,
#ifdef __alpha
/*
** this resolves unalligned access errors but keeps the message box from
** going away automatically in the demo_motif program.  ASK_MARK
*/
    /* widget_size          */  sizeof(ImageWidgetRec),
#else
    /* widget_size          */  sizeof(ImagePart),
#endif
    /* class_initialize	    */  ClassInitialize,
    /* class_part_initialize*/	ClassPartInitialize,
    /* class_inited	    */	FALSE,
    /* initialize	    */	Initialize,
    /* initialize hook	    */	NULL,
    /* realize		    */	Realize,
    /* actions		    */	ImageActions,
    /* num_actions	    */	XtNumber(ImageActions),
    /* resources	    */	(XtResourceList) ImageResources,
    /* num_resources	    */	XtNumber(ImageResources),
    /* xrm_class	    */	NULLQUARK,
    /* compress_motion	    */	TRUE,
    /* compress_exposure    */	TRUE,
    /* compress_enterleave  */	TRUE,
    /* visible_interest	    */	FALSE,
    /* destroy		    */	Destroy,
    /* resize		    */	Resized,
    /* expose		    */	Exposed,
    /* set_values	    */	(XtSetValuesFunc)SetValues,
    /* set_values_hook	    */	NULL,
    /* set_values_almost    */	(XtAlmostProc)_XtInherit,
    /* get_values_hook	    */	NULL,
    /* accept_focus	    */	NULL,
    /* version		    */	XtVersionDontCheck,
    /* callback_offsets	    */	NULL,
    /* tm_table		    */	ImageTranslations,
    /* query_geometry	    */	NULL,
    /* display_accelerator  */	NULL,
    /* extension	    */	NULL,
    },

  /* composite class fields  */
    {
        /* geometry_manager     */      GeometryManager,
        /* change_managed       */      (XtAlmostProc)_XtInherit,
        /* insert_child         */      (XtAlmostProc)_XtInherit,
        /* delete_child         */      (XtAlmostProc)_XtInherit,
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
        /* translations  (XtTranslation) _XtInherit, */  XtInheritTranslations,
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
    /* redraw window	    */	RedrawWindow,
    /* redraw region	    */	RedrawRegion,
    /* image callback	    */	ImageCallback,
    /* pad0		    */	NULL,
    /* extension	    */	NULL,
    }
};
externaldef(imageWidgetMotifClass) ImageMotifClass
	    imageWidgetMotifClass = (ImageMotifClass) &imageWidgetMotifClassRec;

/*****************************************************************************
**  IDSXM$STATIC_IMAGE
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS high level public entry: create an image widget.
**
**  FORMAL PARAMETERS:
**
**	parent	    - parent of widget
**	name$dsc    - name descriptor of widget
**	x	    - x placement within parent
**	y	    - y placement within parent
**	w	    - width  of image widget window desired
**	h	    - height of image widget window desired
**	fid	    - frame id of image to display
**	rend	    - application callback called when image is to be rendered
**	view	    - application callback called when image view changes
**	help	    - application callback called when help is requested
**
**  IMPLICIT INPUTS:
**
**	Resource manager's data base and attributes of widget's parents.
**
**  FUNCTION VALUE:
**
**      Widget identifier of image widget created
**
*****************************************************************************/
#ifdef IdsVMS
Widget IDSXM$STATIC_IMAGE( parent, name$dsc, x, y, w, h, fid, rend, view, help )
 Widget		*parent;
 VMS_STRING	 name$dsc;
 int		*x, *y;
 int		*w, *h;
 unsigned long  *fid;
 XtCallbackList	 rend, view, help;
{
    ImageWidget iw;
    Arg	al[16];
    char *name = (char *) DwtDescToNull( name$dsc );
    int ac = 0;

#ifdef TRACE
printf( "Entering Routine IDSXM$STATIC_IMAGE in module IDS_STATIC_IMAGE_MOTIF \n");
#endif

    SETARG_( al, ac, XmNx,		*x    );
    SETARG_( al, ac, XmNy,		*y    );
    SETARG_( al, ac, XmNwidth,		*w    );
    SETARG_( al, ac, XmNheight,	*h    );
    SETARG_( al, ac, IdsNfid,		*fid  );
    SETARG_( al, ac, IdsNrenderCallback, rend );
    SETARG_( al, ac, IdsNviewCallback,   view );
    SETARG_( al, ac, XmNhelpCallback,   help );
    iw = (ImageWidget) XtCreateWidget(name, imageWidgetMotifClass, *parent, al, ac);
    XtFree( name );

#ifdef TRACE
printf( "Leaving Routine IDSXM$STATIC_IMAGE in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( (Widget) iw );
}
#endif

/*****************************************************************************
**  IdsXmStaticImage
**
**  FUNCTIONAL DESCRIPTION:
**
**      C high level public entry: create an image widget.
**
**  FORMAL PARAMETERS:
**
**	parent	    - parent of widget
**	name	    - name of widget
**	x	    - x placement within parent
**	y	    - y placement within parent
**	w	    - width  of image widget window desired
**	h	    - height of image widget window desired
**	fid	    - frame id of image to display
**	rend	    - application callback called when image is to be rendered
**	view	    - application callback called when image view changes
**	help	    - application callback called when help is requested
**
**  IMPLICIT INPUTS:
**
**	Resource manager's data base and attributes of widget's parents.
**
**  FUNCTION VALUE:
**
**      Widget identifier of image widget created
**
*****************************************************************************/
Widget IdsXmStaticImage( parent, name, x, y, w, h, fid, rend, view, help )
 Widget		 parent;
 char		*name;
 int		 x, y;
 int		 w, h;
 unsigned long	 fid;
 XtCallbackList	 rend;
 XtCallbackList	 view;
 XtCallbackList	 help;
{
    Arg	al[16];
    int ac = 0;
    Widget  returnWidget;

#ifdef TRACE
printf( "Entering Routine IdsXmStaticImage in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    SETARG_( al, ac, XmNx,              x    );
    SETARG_( al, ac, XmNy,              y    );
    SETARG_( al, ac, XmNwidth,          w    );
    SETARG_( al, ac, XmNheight,         h    );
    SETARG_( al, ac, IdsNfid,            fid  );
    SETARG_( al, ac, IdsNrenderCallback, rend );
    SETARG_( al, ac, IdsNviewCallback,   view );
    SETARG_( al, ac, XmNhelpCallback,   help );

    returnWidget = XtCreateWidget( name, (WidgetClass)imageWidgetMotifClass, 
		    parent, al, ac );

#ifdef TRACE
printf( "Leaving Routine IdsXmStaticImage in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( returnWidget );
}

/*****************************************************************************
**  IDSXM$STATIC_IMAGE_CREATE
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS low level public entry: create an image widget.
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
**	Resource manager's data base and attributes of widget's parents.
**
**  FUNCTION VALUE:
**
**      Widget identifier of image widget created
**
*****************************************************************************/
#ifdef IdsVMS
Widget IDSXM$STATIC_IMAGE_CREATE( parent, name$dsc, arglist, argCount )
 Widget		*parent;
 VMS_STRING	 name$dsc;
 Arg		*arglist;
 int		*argCount;
{
    ImageWidget iw;
    char *name = (char *) DwtDescToNull( name$dsc );

#ifdef TRACE
printf( "Entering Routine IDSXM$STATIC_IMAGE_CREATE in module IDS_STATIC_IMAGE_MOTIF \n");
#endif

    iw = (ImageWidget)
	XtCreateWidget( name, (WidgetClass)imageWidgetMotifClass, *parent, arglist, *argCount );
    XtFree( name );

#ifdef TRACE
printf( "Leaving Routine IDSXM$STATIC_IMAGE_CREATE in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( (Widget) iw );
}
#endif

/*****************************************************************************
**  IdsXmStaticImageCreate
**
**  FUNCTIONAL DESCRIPTION:
**
**      C low level public entry: create an image widget.
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
**	Resource manager's data base and attributes of widget's parents.
**
**  FUNCTION VALUE:
**
**      Widget identifier of image widget created
**
*****************************************************************************/
Widget IdsXmStaticImageCreate( parent, name, arglist, argCount )
 Widget  parent;
 char   *name;
 Arg    *arglist;
 int     argCount;
{
    Widget  returnWidget;
#ifdef TRACE
printf( "Entering Routine IdsXmStaticImageCreate in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    returnWidget = XtCreateWidget(name,(WidgetClass)imageWidgetMotifClass,parent,arglist,argCount);
#ifdef TRACE
printf( "Leaving Routine IdsXmStaticImageCreate in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return returnWidget;
}

/*****************************************************************************
**  IDSXM$GET_COORDINATES
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS public entry: get original coordinates from rendered coordinates.
**
**  FORMAL PARAMETERS:
**
**	iw	- image widget
**	from	- render coordinate list to translate from
**	to	- original coordinate list to create
**	num	- number of coordinate pairs to translate
**
**  FUNCTION VALUE:
**
**      to list pointer
**
*****************************************************************************/
#ifdef IdsVMS
XPoint *IDSXM$GET_COORDINATES( iw, from, to, num, type )
 Widget	*iw;
 XPoint *from, *to;
 int	*num;
 int	*type;
{
    XPoint  *returnXPoint;
#ifdef TRACE
printf( "Entering Routine IDSXM$GET_COORDINATES in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Call the C version with d'referenced arguments.
    */
    returnXPoint = IdsXmGetCoordinates( *iw, from, to, *num, *type );
#ifdef TRACE
printf( "Leaving Routine IDSXM$GET_COORDINATES     in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( returnXPoint );
}
#endif

/*****************************************************************************
**  IdsXmGetCoordinates
**
**  FUNCTIONAL DESCRIPTION:
**
**      C public entry: get original coordinates from rendered coordinates.
**
**  FORMAL PARAMETERS:
**
**	iw	- image widget
**	from	- render coordinate list to translate from
**	to	- original coordinate list to create
**	num	- number of coordinate pairs to translate
**	type	- coordinate type: image or window
**
**  FUNCTION VALUE:
**
**      to list pointer
**
*****************************************************************************/
XPoint *IdsXmGetCoordinates( iw, from, to, num, type )
 Widget	iw;
 XPoint *from, *to;
 int	num;
 int	type;

{
    DefineWidgetParts_(Image,iw);
    RenderContext ctx = &Context_(iw);
    IdsRenderCallback rcb   = PropRnd_(ctx);
    XPoint *src, *dst;
    int	    cnt;

#ifdef TRACE
printf( "Entering Routine IdsXmGetCoordinates in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( !XtIsSubclass( iw, (WidgetClass)imageWidgetMotifClass ))
	/*
	**  No renagade widgets allowed here.
	*/
	XtErrorMsg("notImgWgt","IdsGetCoordinates","IdsImageError",
		   "widget is not imageWidgetMotifClass or subclass",
		    NULL,NULL);
    /*
    **  Make sure there is a current rendering.
    */
   if( intCompute_(rcb) == Ids_IslClient && cFid_(iw) == 0 )
        XtErrorMsg("invRndImg","IdsGetCoordinates","IdsImageError",
                   "invalid rendered image ", NULL,NULL);
    else if(  intCompute_(rcb) == Ids_XieServer && cXieImg_(iw) == 0 )
        XtErrorMsg("invRndImg","IdsGetCoordinates","IdsImageError",
                   "invalid rendered image ", NULL,NULL);

    for( src = from, dst = to, cnt = num; cnt-- > 0; src++, dst++ )
	switch ( type )
	    {
	case Ids_RenderedCoordinates:
	    *dst = *src;
	    IdsXmTranslatePoint( iw, dst );
	    break;

	case Ids_WindowCoordinates:
	    dst->x = SrcW_(iw) < WrkW_(iw) ? src->x - cWrkX_(iw)
					   : src->x + cSrcX_(iw);
	    dst->y = SrcH_(iw) < WrkH_(iw) ? src->y - cWrkY_(iw)
					   : src->y + cSrcY_(iw);
	    IdsXmTranslatePoint( iw, dst );
	    break;

	default:
	XtErrorMsg("invCorTyp","IdsGetCoordinates","IdsImageError",
		   "invalid coordinate type", NULL,NULL);
	    }

#ifdef TRACE
printf( "Leaving Routine IdsXmGetCoordinates in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( to );
}

/*****************************************************************************
**  IDSXM$REDISPLAY_IMAGE
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS public entry: re-display image from (sx,sy) of the image
**	to (wx,wy) in the window.
**
**  FORMAL PARAMETERS:
**
**	iw	    - Image Widget id
**	sx	    - x coordinate of source image to display
**	sy	    - y coordinate of source image to display
**	wx	    - x coordinate of window to display image 
**	wy	    - y coordinate of window to display image 
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
#ifdef IdsVMS
void IDSXM$REDISPLAY_IMAGE( iw, sx, sy, wx, wy )
 Widget *iw;
 int	*sx, *sy, *wx, *wy;
{
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine IDSXM$REDISPLAY_IMAGE in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Call the C version with d'referenced arguments.
    */
    IdsXmRedisplayImage( *iw, ( sx == NULL ? 0 : *sx ),
			    ( sy == NULL ? 0 : *sy ),
			    ( wx == NULL ? 0 : *wx ),
			    ( wy == NULL ? 0 : *wy ));
#ifdef TRACE
printf( "Leaving Routine IDSXM$REDISPLAY_IMAGE in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}
#endif

/*****************************************************************************
**  IdsXmRedisplayImage
**
**  FUNCTIONAL DESCRIPTION:
**
**      C public entry: re-display image from (sx,sy) of the image
**	to (wx,wy) in the window.
**
**  FORMAL PARAMETERS:
**
**	iw	    - Image Widget id
**	sx	    - x coordinate of source image to display
**	sy	    - y coordinate of source image to display
**	wx	    - x coordinate of window to display image 
**	wy	    - y coordinate of window to display image 
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
void IdsXmRedisplayImage( iw, sx, sy, wx, wy )
 Widget iw;
 int    sx, sy, wx, wy;
{
    DefineWidgetParts_(Image,iw);
    RenderContext ctx = &Context_(iw);
    IdsRenderCallback rcb   = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine IdsXmRedisplayImage in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( !XtIsSubclass( iw, (WidgetClass)imageWidgetMotifClass ))
	/*
	**  No renagade widgets allow here.
	*/
	XtErrorMsg("notImgWgt","IdsRedisplayImage","IdsImageError",
		   "widget is not of ImageWidgetMotifClass or subclass",NULL,NULL);

    else if( ( intCompute_(rcb) == Ids_IslClient && cFid_(iw) != 0 ) ||
             ( intCompute_(rcb) == Ids_XieServer && cXieImg_(iw) != 0 ) )
	{
	/*
	**  Load new coordinates.
	*/
	rSrcX_(iw) = sx;
	rSrcY_(iw) = sy;
	rWrkX_(iw) = wx;
	rWrkY_(iw) = wy;
	Redraw_(iw) |= LimitSrcWrk( iw );

	/*
	**  If the new coordinates require a Redraw, send an XExposeEvent to 
	**  ourself so the redisplay will happen 'after' the application has
	**  returned control to Intrinsics (ie. XtNextEvent will process it).
	*/
	if( Redraw_(iw) && pRender_(iw) == Ids_Passive )
	    SendExpose( iw );
	}
#ifdef TRACE
printf( "Leaving Routine IdsXmRedisplayImage in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  IDSXM$APPLY_GRAVITY
**
**  FUNCTIONAL DESCRIPTION:
**
**      VMS public entry: re-display image using gravity.
**	The gravity settings replace those in the widget.
**
**  FORMAL PARAMETERS:
**
**	iw	    - Image Widget id
**	src_grav    - source gravity constant.
**	win_grav    - window gravity constant.
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
#ifdef IdsVMS
void IDSXM$APPLY_GRAVITY( iw, src_grav, win_grav )
 Widget		*iw;
 unsigned long	*src_grav, *win_grav;
{
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine IDSXM$APPLY_GRAVITY in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Call the C version with d'referenced arguments.
    */
    IdsXmApplyGravity( *iw, ( src_grav == NULL ? Ids_NoGravity : *src_grav ),
			  ( win_grav == NULL ? Ids_NoGravity : *win_grav ));

#ifdef TRACE
printf( "Leaving Routine IDSXM$APPLY_GRAVITY in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}
#endif

/*****************************************************************************
**  IdsXmApplyGravity
**
**  FUNCTIONAL DESCRIPTION:
**
**      C public entry: re-display image using gravity.
**	The gravity settings replace those in the widget.
**
**  FORMAL PARAMETERS:
**
**	iw	    - Image Widget id
**	src_grav    - source gravity constant.
**	win_grav    - window gravity constant.
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
void IdsXmApplyGravity( iw, src_grav, win_grav )
 Widget		iw;
 unsigned long	src_grav, win_grav;
{
    DefineWidgetParts_(Image,iw);
    RenderContext ctx = &Context_(iw);
    IdsRenderCallback rcb   = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine IdsXmApplyGravity in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( !XtIsSubclass( iw, (WidgetClass)imageWidgetMotifClass ))
	/*
	**  No renagade widgets allow here.
	*/
	XtErrorMsg("notImgWgt","IdsApplyGravity","IdsImageError",
		   "widget is not of ImageWidgetMotifClass or subclass",NULL,NULL);

    else if( ( intCompute_(rcb) == Ids_IslClient && cFid_(iw) != 0 ) ||
             ( intCompute_(rcb) == Ids_XieServer && cXieImg_(iw) != 0 ) )
	{
	/*
	**  Install the new gravity constants, then apply them.
	*/
	SGrav_(iw) = src_grav;
	WGrav_(iw) = win_grav;

	ApplyGravity( iw, TRUE, TRUE );
	Redraw_(iw) |= LimitSrcWrk( iw );
	/*
	**  If the new coordinates require a Redraw, send an XExposeEvent to 
	**  ourself so the redisplay will happen 'after' the application has
	**  returned control to Intrinsics (ie. XtNextEvent will process it).
	*/
	if( Redraw_(iw) && pRender_(iw) == Ids_Passive )
	    SendExpose( iw );
	}
#ifdef TRACE
printf( "Leaving Routine IdsXmApplyGravity in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ClassInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**	Called only the first time a widget of this class is initialized.
**
**  FORMAL PARAMETERS:
**
**	none
**
*****************************************************************************/
static void ClassInitialize()
{
#ifdef TRACE
printf( "Entering Routine ClassInitialize in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Create actual offsets to instance record parts and to resources.
    */
    XmResolvePartOffsets(imageWidgetMotifClass,
			 &imageWidgetMotifClassRec.image_class.offsets);
    /*
    **	Register GRAVITY type converter.
    */
    XtAddConverter( XtRString, IdsRGravity, IdsStringToGravity, 0, 0 );

    /*
    **	Add ImageWidget routine to handle scrollbar "None<Btn1Up>" events.
    */
    XtAddActions( ScrollActions, XtNumber( ScrollActions ) );

    /*
    **	Parse scrollbar override translations for "None<Btn1Up>" events.
    */
#ifdef TRACE
printf( "Leaving Routine ClassInitialize in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ClassPartInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**	Called only the first time a widget of this class is initialized.
**      Passes inheritance of ImageMotifClass functions to subclasses.
**
**  FORMAL PARAMETERS:
**
**	ic	- Image Class record
**
*****************************************************************************/
static void ClassPartInitialize( ic )
    ImageMotifClass	ic;
{
    ImageMotifClass	super = (ImageMotifClass) ic->core_class.superclass;

#ifdef TRACE
printf( "Entering Routine ClassPartInitialize in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Pass inheritance of ImageMotifClass functions to subclasses.
    */
    if( (IDS_BoolProc) ic->image_class.redraw_window
     == (IDS_BoolProc) _XtInherit)
	ic->image_class.redraw_window = super->image_class.redraw_window;

    if( (IDS_BoolProc) ic->image_class.redraw_region
     == (IDS_BoolProc) _XtInherit)
	ic->image_class.redraw_region = super->image_class.redraw_region;

    if( (IDS_BoolProc) ic->image_class.image_callback
     == (IDS_BoolProc) _XtInherit)
	ic->image_class.image_callback = super->image_class.image_callback;

#ifdef TRACE
printf( "Leaving Routine ClassPartInitialize in module IDS_STATIC_IMAGE_MOTIF \n");
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
**	req	- Image Widget, as built from arglist.
**	new	- Image Widget, as modified by superclasses.
**
*****************************************************************************/
static void Initialize( req, new, args, num_args )
    Widget	req, new;
    ArgList args;
    Cardinal *num_args;
{
    DefineWidgetParts_(Image,new);

#ifdef TRACE
printf( "Entering Routine Initialize in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **  Set default widget size if none was specified.
    */
    if( XtWidth(new) <= 0 )
	XtWidth(new)  = DEFAULT_WIDTH;

    if( XtHeight(new) <= 0 )
	XtHeight(new) = DEFAULT_HEIGHT;
 
    /*
    **  No scrollbars yet.
    */
    SPix_(new)	 = NULL;
    SBox_(new)	 = NULL;
    Hbar_(new)	 = NULL;
    Vbar_(new)	 = NULL;
    ActBar_(new) = FALSE;
    DoBars_(new) = FALSE;

    /*
    **  Init other private or read-only resources.
    */
    Visibility_(new) = VisibilityFullyObscured;
    cCoords_(new)    = rCoords_(new);
    SrcW_(new)       = 0;
    SrcH_(new)       = 0;
    Redraw_(new)     = FALSE;

#ifdef TRACE
printf( "Leaving Routine Initialize in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  Realize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine realizes the widget: creates the widget window and 
**	generates the initial rendering parameters.
**
**  FORMAL PARAMETERS:
**
**	iw	   - Image Widget 
**	valuemask  - intrinsics supplied mask for window creation
**	attributes - intrinsics supplied attributes for window creation
**
*****************************************************************************/
static void Realize( iw, valuemask, attributes )
    Widget		 iw;
    Mask                 *valuemask;
    XSetWindowAttributes *attributes;
{
    DefineWidgetParts_(Image,iw);
    RenderContext ctx = &Context_(iw);

    *valuemask |= CWColormap;
    attributes->colormap = Colormap_(iw);

#ifdef TRACE
printf( "Entering Routine Realize in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Have intrinsics create our widget window.
    */
    XtCreateWindow( iw, InputOutput, CopyFromParent, *valuemask, attributes );

    /*
    **  Initialize the stable parts of our widget context.
    */
    Dpy_(ctx)	  = XtDisplay(iw);
    Scr_(ctx)	  = XtScreen(iw);
    Win_(ctx)	  = XtWindow(iw);
    WinD_(ctx)	  = Depth_(iw);
    Cells_(ctx)	  = CellsOfScreen(XtScreen(iw));
    Vis_(ctx)     = DefaultVisualOfScreen( XtScreen(iw) );
    PropRnd_(ctx) = &Proposed_(iw);

    /*
    **  Generate initial rendering parameters.
    */
    SetWrkArea( iw, XtWidth(iw), XtHeight(iw) );
    if( IdsXmCompareRenderings( iw )  )
	{
	if( pReason_(iw) == IdsCRNormal )
	    pReason_(iw)  = IdsCRRealized;
	Redraw_(iw) = TRUE;
	}
    else
	ConfigureBars( iw );
#ifdef TRACE
printf( "Leaving Routine Realize in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  Destroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      Widget's been requested to destroy its resources.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**
*****************************************************************************/
static void Destroy( iw )
    Widget iw;
{
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine Destroy in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( SPix_(iw) != NULL )
	/*
	**  Free inter-scrollbar label pixmap.
	*/
	XFreePixmap( XtDisplay(iw), SPix_(iw) );
#ifdef TRACE
printf( "Leaving Routine Destroy in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  Resized
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called when the parent's geom mgr resizes the widget.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**
*****************************************************************************/
static void Resized( iw )
   Widget iw;
{
    DefineWidgetParts_(Image,iw);
    RenderContext ctx = &Context_(iw);
    IdsRenderCallback rcb   = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine Resized in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( XtIsRealized(iw) )
	{
	SetWrkArea( iw, XtWidth(iw), XtHeight(iw) );

	if( IdsXmCompareRenderings( iw ) )
	    {
	    /*
	    **  Resize will require a re-rendering.
	    */
	    if( pReason_(iw) == IdsCRNormal )
		pReason_(iw)  = IdsCRResized;
	    }
	else
	    /*
	    **	Just resize our scrollbars.
	    */
	    ConfigureBars(iw);

	/*
	**  Limit image display dimensions and placement.
	*/
	cSrcX_(iw)   = INVALID_COORD;	/* invalidate Wrk area image data   */
	cSrcY_(iw)   = INVALID_COORD;
	Redraw_(iw) |= LimitSrcWrk( iw );


	if  (iProto_(rcb) == Ids_Window ) /* rendering with VXT 2000 */
	    {
	    RerenderWindowMotif(iw, FALSE);
            ConfigureBars(iw);
            LimitSrcWrk( iw );
            Redraw_(iw) = FALSE;
            ImageCallback( iw, DoBars_(iw), IdsCRViewChanged, NULL );
	    }
	}

#ifdef TRACE
printf( "Leaving Routine Resized in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  Exposed
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will re-render/re-draw the image in the Wrk area of the
**	widget window and callback the application.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	expose	- XExposeEvent structure
**
*****************************************************************************/
static void Exposed( iw, event, region )
    Widget iw;
    XEvent *event;
    Region region;
    {
    XExposeEvent *expose = (XExposeEvent *)event;
    DefineWidgetParts_(Image,iw);
    XmDrawingAreaCallbackStruct ecbs;
    RenderContext ctx = &Context_(iw);
    IdsRenderCallback rcb   = PropRnd_(ctx);
    RenderContextXie xiectx = &XieContext_(iw);

#ifdef TRACE
printf( "Entering Routine Exposed in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if (Context_(iw).pipe_desc->pipe == NULL && Photo_(xiectx) == 0 )
                        /* ignore if pipe is busy or xiepipe is not freed  */
	{
	if  (iProto_(rcb) == Ids_Window ) /* rendering with VXT 2000 */
	    {
	    /* Always re-render if the mode is not passive */

 	    if ( pRender_(iw) != Ids_Passive ) 
		{
                LimitSrcWrk( iw );
                if (pRender_(iw) != Ids_Purge)
                    {
		    Redraw_(iw) = TRUE;
	            RerenderWindowMotif(iw, FALSE);

    		    /*
		    ** Set scrollbar parameters and (un)manage them.
		    */
	            ConfigureBars( iw );
	            /*
	            **  Update the scrollbars and notify the application.
	            */
                    ImageCallback( iw, DoBars_(iw), IdsCRViewChanged, (XEvent *)expose );
	  	    }
                else
		    RenderImage_(iw, expose);

		}
	    else if ( !Redraw_(iw) ) 
	        {
	        /*
	        **  The image view hasn't changed, so redraw only the 
		**  exposed area.
	        */
	        RedrawRegion(iw, expose);
	        ecbs.reason = XmCR_EXPOSE;
	        ecbs.event  = (XEvent *) expose;
	        ecbs.window	= XtWindow(iw);
	        XtCallCallbacks( iw, XmNexposeCallback, &ecbs ); 
	        }

	    }
	else
	    {
 	    if ( pRender_(iw) != Ids_Passive ) 
	        ApplyScheme( iw, expose );	/* (re-)render the image    */

	    if ( !Redraw_(iw) )
	        {
	        /*
	        **  The image view hasn't changed, so redraw only the 
		**  exposed area.
	        */
	        RedrawRegion(iw, expose);
	        ecbs.reason = XmCR_EXPOSE;
	        ecbs.event  = (XEvent *) expose;
	        ecbs.window	= XtWindow(iw);
	        XtCallCallbacks( iw, XmNexposeCallback, &ecbs ); 
	        }

	    else if ( expose->count == 0 ) 
	        /*
	        **  Received the final expose event, now redraw the entire Wrk area.
	        */
	        if( RedrawWindow( iw) )
	            /*
	            **  Update the scrollbars and notify the application.
	            */
	            ImageCallback( iw, DoBars_(iw), IdsCRViewChanged, expose );
	    }
	}
#ifdef TRACE
printf( "Leaving Routine Exposed in module IDS_STATIC_IMAGE_MOTIF \n");
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
**	old	- Image Widget, copy of original.
**	req	- Image Widget, as built from arglist.
**	new	- Image Widget, as modified by superclasses.
**
**  FUNCTION VALUE:
**
**	FALSE	(re-display is never requested via Toolkit)
**
*****************************************************************************/
static Boolean SetValues( old, req, new )
    Widget  old, req, new;
{
    DefineWidgetParts_(Image,old);
    DefineWidgetParts_(Image,new);
    RenderContext ctx = &Context_(new);
    IdsRenderCallback rcb = PropRnd_(ctx);

#ifdef TRACE
printf( "Entering Routine SetValues in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    UpdateCallback_(old, &ViewCB_(old), new, &ViewCB_(new), IdsNviewCallback);
    SrcW_(new) = SrcW_(old);	        /* preserve our read-only resources */
    SrcH_(new) = SrcH_(old);

    if( XtIsRealized(new) )
	{
	/*
	**  Set Redraw_() if rendering, display size, or coordinates change.
	*/
	SetWrkArea( new, XtWidth(old), XtHeight(old) );

	ApplyGravity( new, SGrav_(old) != SGrav_(new),
			   WGrav_(old) != WGrav_(new) );
	Redraw_(new) |= IdsXmCompareRenderings(new) || LimitSrcWrk(new)
		    || cSrcX_(old) != rSrcX_(new) || cSrcY_(old) != rSrcY_(new)
		    || cWrkX_(old) != rWrkX_(new) || cWrkY_(old) != rWrkY_(new);

	if (( pRender_(new) == Ids_Passive ) ||
             (iProto_(rcb) == Ids_Window ))
	    ConfigureBars(new);

	if( Redraw_(new)  &&  pRender_(old) == Ids_Passive )
	    /*
	    **	Our OLD renderMode state was Passive, so we need to trigger 
	    **	an expose event to get the window redrawn (if our OLD state
	    **	wasn't Passive, it means we're already working on the redraw).
	    */
            /* set the redraw to false so that the expose will redraw just
            ** what is in the window.
            */
            if (iProto_(rcb) == Ids_Window )
		{
                pRender_(new) = Ids_Normal;
                Redraw_(new) = FALSE;
		}
	    SendExpose( new );
	}

#ifdef TRACE
printf( "Leaving Routine SetValues in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( FALSE );	    /* never tell Intrinsics to clear our window    */
}

/*****************************************************************************
**  GeometryManager
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called to ...
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	desired	- desired geometry change
**	allowed	- allowed geometry change
**
**  FUNCTION VALUE:
**
**	XtGeometryNo
**
*****************************************************************************/
static XtGeometryResult GeometryManager( iw, desired, allowed )
   Widget iw;
   XtWidgetGeometry *desired;
   XtWidgetGeometry *allowed;
{
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine GeometryManager in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Always say no.
    */
#ifdef TRACE
printf( "Leaving Routine GeometryManager in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( XtGeometryNo );
}

/*****************************************************************************
**  RedrawWindow
**
**  FUNCTIONAL DESCRIPTION:
**
**      Redraw the entire image, as best we can, at the requested x and y.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**
**  FUNCTION VALUE:
**
**	Boolean	- TRUE if new display of image differs from previous.
**
*****************************************************************************/
static Boolean RedrawWindow( iw )
Widget  iw;
    {
    DefineWidgetParts_(Image,iw);
    RenderContext ctx = &Context_(iw);
    RenderContextXie xiectx = &XieContext_(iw);
    IdsRenderCallback rcb = PropRnd_(ctx);
    int asx, asy, awx, awy, dsx, dsy;

#ifdef TRACE
printf( "Entering Routine RedrawWindow in module IDS_STATIC_IMAGE_MOTIF \n");
#endif

    /*
    **	Redraw if something has changed and the Wrk area is not fully obscured.
    */
    if( Redraw_(iw)  &&  Visibility_(iw) != VisibilityFullyObscured )
	{
        if(( intCompute_(rcb) == Ids_XieServer ) && (iProto_(rcb) != Ids_XImage))
	    {
	    if (iProto_(rcb) == Ids_Window )
		RerenderWindowMotif (iw, FALSE);

	    else if( DPhoto_(xiectx) != NULL && iProto_(rcb) == Ids_Photomap )
		{
		/*
		**  Display the XiePhotomap image data in the window.
		*/
                dsx = rSrcX_(iw) - cSrcX_(iw);  /* delta source X           */
                dsy = rSrcY_(iw) - cSrcY_(iw);  /* delta source Y           */
                asx = abs( dsx );               /* absolute delta source X  */
                asy = abs( dsy );               /* absolute delta source Y  */

                if( Visibility_(iw) == VisibilityPartiallyObscured
                    || asx >= SrcW_(iw) || asy >= SrcH_(iw) || cSrcX_(iw) < 0 )
		    /*
		    **  Display the XiePhotomap image data in the window.
		    */
	    	    XieExport( DPhoto_(xiectx),      /* server Photo{flo|map}*/
			       XtWindow(iw),         /* window id            */
                               GC_(iw),              /* GC used              */
                               rSrcX_(iw), 
			       rSrcY_(iw),           /* from:  img X,Y       */
			       rWrkX_(iw), 
                               rWrkY_(iw),           /* to: drawable X,Y     */
                               SrcW_(iw),            /* width                */
                               SrcH_(iw),            /* height               */
                               LPhoto_(xiectx),      /* client LUT           */
                               Cmap_(ctx),           /* colormap             */
                               MchLim_(ctx),         /* match_limit          */
                               GraLim_(ctx) );       /* gray_limit           */


                else
                    {
                    /*
                    **  Some of the requested image is in the Wrk area, move it.
                    */
                    XCopyArea( XtDisplay(iw), XtWindow(iw), XtWindow(iw),
                            GC_(iw),
                            cWrkX_(iw) + (dsx <= 0 ? 0 : dsx),  /* Src X    */
                            cWrkY_(iw) + (dsy <= 0 ? 0 : dsy),  /* Src Y    */
                            SrcW_(iw) - asx,                    /* Width    */
                            SrcH_(iw) - asy,                    /* Height   */
                            rWrkX_(iw) + (dsx >= 0 ? 0 : asx),  /* Wrk X    */
                            rWrkY_(iw) + (dsy >= 0 ? 0 : asy)); /* Wrk Y    */
    

                    /*
                    **  Redraw top or bottom of Wrk area.
                    */
                    if( dsy != 0 )
			XieExport( DPhoto_(xiectx),  /* server Photo{flo|map}*/
			       XtWindow(iw),         /* window id            */
                               GC_(iw),              /* GC used              */
                               rSrcX_(iw),           /* Src X      */
                               rSrcY_(iw) + (dsy <= 0 ? 0      /* Src Y (top)*/
                                       : SrcH_(iw) - dsy),     /*    (bottom)*/
                               rWrkX_(iw),                     /* Wrk X      */
                               rWrkY_(iw) + (dsy <= 0 ? 0      /* Wrk Y (top)*/
                                       : SrcH_(iw) - dsy),     /*    (bottom)*/
                               SrcW_(iw),                      /* Width      */
                               asy,                            /* Height     */
                               LPhoto_(xiectx),      /*  client LUT          */
                               Cmap_(ctx),           /* colormap             */
                               MchLim_(ctx),         /* match_limit          */
                               GraLim_(ctx) );       /* gray_limit           */

                    /*
                    **  Redraw left or right side (just a corner if dsy != 0).
                    */
                    if( dsx != 0 )
			XieExport( DPhoto_(xiectx),  /* server Photo{flo|map}*/
			       XtWindow(iw),         /* window id            */
                               GC_(iw),              /* GC used              */
                               rSrcX_(iw) + (dsx < 0 ? 0      /* Src X(left)*/
                                          : SrcW_(iw) - dsx), /*     (right)*/
                               rSrcY_(iw) - (dsy > 0 ? 0      /* Src Y (all)*/
                                                  : dsy),     /*    (corner)*/
                               rWrkX_(iw) + (dsx < 0 ? 0      /* Wrk X(left)*/
                                          : SrcW_(iw) - dsx), /*     (right)*/
                               rWrkY_(iw) - (dsy > 0 ? 0      /* Wrk Y (all)*/
                                                  : dsy),     /*    (corner)*/
                               asx,                           /* Width      */
                               SrcH_(iw) - asy,               /* Height     */
                               LPhoto_(xiectx),      /* client LUT          */
                               Cmap_(ctx),           /* colormap             */
                               MchLim_(ctx),         /* match_limit          */
                               GraLim_(ctx) );       /* gray_limit           */
                    }
		}

	    else if( Pix_(iw) != NULL  && iProto_(rcb) == Ids_Pixmap )
	       PixmapToXieWindow_(iw, rSrcX_(iw), rSrcY_(iw), SrcW_(iw), SrcH_(iw),
				rWrkX_(iw), rWrkY_(iw) );
	    }	

	/*
	**  If image is in a pixmap, redraw entire Wrk area from there.
	*/
	else if (( intCompute_(rcb) == Ids_IslClient ) || 
		( iProto_(rcb) == Ids_XImage))
	    {
            if( Pix_(iw) != NULL )
	      PixmapToWindow_(iw, rSrcX_(iw), rSrcY_(iw), SrcW_(iw), SrcH_(iw),
				rWrkX_(iw), rWrkY_(iw) );

	    /*
	    ** otherwise we must supply at least part of the image to the server
            */
	    else if( Idata_(iw) != NULL )
		{
		/*
		**  Compute coordinate changes.
		*/
		dsx = rSrcX_(iw) - cSrcX_(iw);	/* delta source X	    */
		dsy = rSrcY_(iw) - cSrcY_(iw);	/* delta source Y	    */
		asx = abs( dsx );		/* absolute delta source X  */
		asy = abs( dsy );		/* absolute delta source Y  */

		if( Visibility_(iw) == VisibilityPartiallyObscured
		    || asx >= SrcW_(iw) || asy >= SrcH_(iw) || cSrcX_(iw) < 0 )
		    /*
		    ** No usable image data in Wrk area, redraw the whole thing.
		    */
		    ImageToWindow( iw, rSrcX_(iw), rSrcY_(iw),
					   rWrkX_(iw), rWrkY_(iw),
					   SrcW_(iw),  SrcH_(iw) );

		else
		    {
		    /*
		    **  Some of the requested image is in the Wrk area, move it.
		    */
		    XCopyArea( XtDisplay(iw), XtWindow(iw), XtWindow(iw), 
		            GC_(iw),
			    cWrkX_(iw) + (dsx <= 0 ? 0 : dsx),	/* Src X    */
			    cWrkY_(iw) + (dsy <= 0 ? 0 : dsy),	/* Src Y    */
			     SrcW_(iw) - asx,			/* Width    */
			     SrcH_(iw) - asy,			/* Height   */
			    rWrkX_(iw) + (dsx >= 0 ? 0 : asx),	/* Wrk X    */
			    rWrkY_(iw) + (dsy >= 0 ? 0 : asy));	/* Wrk Y    */

		    /*
		    **  Redraw top or bottom of Wrk area.
		    */
		    if( dsy != 0 )
			 ImageToWindow(iw,
			    rSrcX_(iw),			      /* Src X	    */
			    rSrcY_(iw) + (dsy <= 0 ? 0	      /* Src Y (top)*/
				       : SrcH_(iw) - dsy),    /*    (bottom)*/
			    rWrkX_(iw),			      /* Wrk X	    */
			    rWrkY_(iw) + (dsy <= 0 ? 0	      /* Wrk Y (top)*/
				       : SrcH_(iw) - dsy),    /*    (bottom)*/
			    SrcW_(iw),			      /* Width	    */
			    asy );			      /* Height	    */

		    /*
		    **  Redraw left or right side (just a corner if dsy != 0).
		    */
		    if( dsx != 0 )
			ImageToWindow(iw,
			    rSrcX_(iw) + (dsx < 0 ? 0	      /* Src X(left)*/
					  : SrcW_(iw) - dsx), /*     (right)*/
			    rSrcY_(iw) - (dsy > 0 ? 0	      /* Src Y (all)*/
						  : dsy),     /*    (corner)*/
			    rWrkX_(iw) + (dsx < 0 ? 0	      /* Wrk X(left)*/
					  : SrcW_(iw) - dsx), /*     (right)*/
			    rWrkY_(iw) - (dsy > 0 ? 0	      /* Wrk Y (all)*/
						  : dsy),     /*    (corner)*/
			    asx,			      /* Width	    */
			    SrcH_(iw) - asy );		      /* Height	    */
		    }
	  	}
	    }

	if (iProto_(rcb) != Ids_Window )
	    {
	    /*
	    **  Compute coordinate changes within window.
	    */
	    awx = abs( rWrkX_(iw) - cWrkX_(iw) );/* absolute delta work X    */
	    awy = abs( rWrkY_(iw) - cWrkY_(iw) );/* absolute delta work Y    */

	    if( awy != 0 )
	        /*
	        **  Moved vertically within work area, clear area above or below.
	        */
	        XClearArea( XtDisplay(iw), XtWindow(iw),
			    cWrkX_(iw),				/* Wrk X    */
			    cWrkY_(iw) < rWrkY_(iw) ||		/* Wrk Y    */
			     SrcH_(iw) < awy ? cWrkY_(iw)	/*  (above) */
				       : rWrkY_(iw) + SrcH_(iw),/*  (below) */
			    SrcW_(iw),				/* Width    */
			    MIN(SrcH_(iw), awy),		/* Height   */
			    FALSE );				/* no event */


	    if( awx != 0 && awy < SrcH_(iw) )
	        /*
	        **  Moved horizontally within work area and less than the height
	        **	of the image vertically, clear left or right side or corner.
	        */
	        XClearArea( XtDisplay(iw), XtWindow(iw),
			    cWrkX_(iw) < rWrkX_(iw) ||		/* Wrk X    */
			     SrcW_(iw) < awx ? cWrkX_(iw)	/*   (left) */
				       : rWrkX_(iw) + SrcW_(iw),/*   (right)*/
			    MAX(cWrkY_(iw), rWrkY_(iw)),	/* Wrk Y    */
			    MIN( SrcW_(iw), awx),		/* Width    */
			    SrcH_(iw) - awy,			/* Height   */
			    FALSE );				/* no event */

	    /*
	    **  Shove everything on its way to the server.
	    */
	    XFlush( XtDisplay(iw) ); 
	    }
	}
    /*
    **  Save the current coordinates.
    */
    cCoords_(iw) = rCoords_(iw);

#ifdef TRACE
printf( "Leaving Routine RedrawWindow in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( IfReplace_( Redraw_(iw), Redraw_(iw), FALSE ));
}

/*****************************************************************************
**  RerenderWindowMotif
**
**  FUNCTIONAL DESCRIPTION:
**
**      Rerender the entire image, at the requested x and y.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**
**  FUNCTION VALUE:
**
**	Boolean	- TRUE if new display of image differs from previous.
**
*****************************************************************************/
static Boolean RerenderWindowMotif( iw , flag)
Widget  iw;
long flag;
    {
    DefineWidgetParts_(PannedImage,iw);

    RenderContext 	ctx = &Context_(iw);
    RenderContextXie 	xiectx = &XieContext_(iw);
    IdsRenderCallback 	rcb = PropRnd_(ctx);
    DataForXie         	xiedat;
    XEvent 		*event;
    int asx, asy, awx, awy, dsx, dsy;

#ifdef TRACE
printf( "Entering Routine RerenderWindowMotif in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    xiedat = PriXie_(xiectx);  

    /*
    **	Redraw if something has changed and the Wrk area is not fully obscured.
    */

    /* Check if zoom was performed */
    ZoomingCheckMotif(iw, 
		&Iroix_(xiedat), 	
		&Iroiy_(xiedat), 
		&Iroih_(xiedat), 
		&Iroiw_(xiedat));

    IdsXmCompareRenderings( iw ); 

    if (( Redraw_(iw)  &&  Visibility_(iw) != VisibilityFullyObscured )
        && ( intCompute_(rcb) == Ids_XieServer )
	   && (iProto_(rcb) == Ids_Window ) )
	{
	/*
	**  Display the XiePhotomap image data in the window.
	*/
        dsx = rSrcX_(iw) - cSrcX_(iw);  /* delta source X           */
        dsy = rSrcY_(iw) - cSrcY_(iw);  /* delta source Y           */
        asx = abs( dsx );               /* absolute delta source X  */
        asy = abs( dsy );               /* absolute delta source Y  */

        if ((!Panning_(iw) && ( !flag || Zooming_(iw) )) && 
	    ( Visibility_(iw) == VisibilityPartiallyObscured
              || asx >= SrcW_(iw) || asy >= SrcH_(iw) 
	                          || cSrcX_(iw) < 0) )

	    /* 
	    ** Need to rebuild pipe and rerender because
            ** we need image data outside of the current window.
	    */
	    {
            pRender_(iw) = Ids_Normal;
	    if (!Zooming_(iw))
	        {
                IULx_(xiedat) = rSrcX_(iw);
                IULy_(xiedat) = rSrcY_(iw);
		}
	    RenderImage_( iw, event );
            if( Pix_(iw) != NULL )
                PixmapToXieWindow_(iw, 0, 0,
                 ((WrkW_(iw) > Iwide_(iw)) ? Iwide_(iw) : WrkW_(iw)),
                 ((WrkH_(iw) > Ihigh_(iw)) ? Ihigh_(iw) : WrkH_(iw)),
                  rWrkX_(iw), rWrkY_(iw) );
	    pRender_(iw) = Ids_Passive; 
	    }
        else
            {
            /*
            **  Some of the requested image is in the Wrk area, move it.
            */
            XCopyArea( XtDisplay(iw), XtWindow(iw), XtWindow(iw),
                       GC_(iw),
                       cWrkX_(iw) + (dsx <= 0 ? 0 : dsx),  /* Src X    */
                       cWrkY_(iw) + (dsy <= 0 ? 0 : dsy),  /* Src Y    */
		       SrcW_(iw) - asx,			/* Width    */
		       SrcH_(iw) - asy,			/* Height   */
                       rWrkX_(iw) + (dsx >= 0 ? 0 : asx),  /* Wrk X    */
                       rWrkY_(iw) + (dsy >= 0 ? 0 : asy)); /* Wrk Y    */
    

            if (Panning_(iw) || flag)
                {
                /*
                **  Clear top or bottom of Wrk area.
                */
                if( dsy != 0 )
	            XClearArea( XtDisplay(iw), XtWindow(iw),
			        cWrkX_(iw),		    /* Wrk X      */
                                cWrkY_(iw) + (dsy <= 0 ? 0  /* Wrk Y (top)*/
                                       : SrcH_(iw) - dsy),  /*    (bottom)*/
                                SrcW_(iw),                  /* Width      */
                                asy,                        /* Height     */
			        FALSE );		    /* no event   */

                /*
                **  Clear left or right side.
		**  (just a corner if dsy != 0).
                */

                if( dsx != 0 )
  	            XClearArea( XtDisplay(iw), XtWindow(iw),
                               rWrkX_(iw) + (dsx < 0 ? 0      /* Wrk X(left)*/
                                          : SrcW_(iw) - dsx), /*     (right)*/
                               rWrkY_(iw) - (dsy > 0 ? 0      /* Wrk Y (all)*/
                                                  : dsy),     /*    (corner)*/
                               asx,                           /* Width      */
                               SrcH_(iw) - asy,               /* Height     */
			       FALSE );				/* no event */

            	}
	    else
		{
		/* Need to render at the current coordinates */
	        pRender_(iw) = Ids_Normal;
	        if (!Zooming_(iw))
		    {
                    IULx_(xiedat) = rSrcX_(iw);
                    IULy_(xiedat) = rSrcY_(iw);
		    }
		RenderImage_( iw, event );
                if( Pix_(iw) != NULL )
                    PixmapToXieWindow_(iw, 0, 0,
                     ((WrkW_(iw) > Iwide_(iw)) ? Iwide_(iw) : WrkW_(iw)),
                     ((WrkH_(iw) > Ihigh_(iw)) ? Ihigh_(iw) : WrkH_(iw)),
                      rWrkX_(iw), rWrkY_(iw) );
	        pRender_(iw) = Ids_Passive; 
		}
	    }	
	}
    /*
    **  Compute coordinate changes within window.
    */
    awx = abs( rWrkX_(iw) - cWrkX_(iw) );	/* absolute delta work X    */
    awy = abs( rWrkY_(iw) - cWrkY_(iw) );	/* absolute delta work Y    */

    if( awy != 0 )
        /*
	**  Moved vertically within work area, clear area above or below.
	*/
	XClearArea( XtDisplay(iw), XtWindow(iw),
			    cWrkX_(iw),				/* Wrk X    */
			    cWrkY_(iw) < rWrkY_(iw) ||		/* Wrk Y    */
			     SrcH_(iw) < awy ? cWrkY_(iw)	/*  (above) */
				       : rWrkY_(iw) + SrcH_(iw),/*  (below) */
			    SrcW_(iw),				/* Width    */
			    MIN(SrcH_(iw), awy),		/* Height   */
			    FALSE );				/* no event */


    if( awx != 0 && awy < SrcH_(iw) )
        /*
        **  Moved horizontally within work area and less than the height
        **	of the image vertically, clear left or right side or corner.
        */
        XClearArea( XtDisplay(iw), XtWindow(iw),
			    cWrkX_(iw) < rWrkX_(iw) ||		/* Wrk X    */
			     SrcW_(iw) < awx ? cWrkX_(iw)	/*   (left) */
				       : rWrkX_(iw) + SrcW_(iw),/*   (right)*/
			    MAX(cWrkY_(iw), rWrkY_(iw)),	/* Wrk Y    */
			    MIN( SrcW_(iw), awx),		/* Width    */
			    SrcH_(iw) - awy,			/* Height   */
			    FALSE );				/* no event */

    /*
    **  Shove everything on its way to the server.
    */
    LimitSrcWrk( iw );
    XFlush( XtDisplay(iw) ); 

    /*
    **  Save the current coordinates.
    */
    cCoords_(iw) = rCoords_(iw);

#ifdef TRACE
printf( "Leaving Routine RerenderWindowMotif in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( IfReplace_( Redraw_(iw), Redraw_(iw), FALSE ));
    }

/*****************************************************************************
**  ZoomingCheckMotif
**
**  FUNCTIONAL DESCRIPTION:
**
**      Handles zoom setup if necessary.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	ix	- Zoom start x 
**	iy 	- Zoom start y
**	iheight - Zoom region height
**	iwidth  - Zoom region width
**
**  FUNCTION VALUE:
**
**	void
**
*****************************************************************************/
void ZoomingCheckMotif( iw, ix, iy, iheight, iwidth )
Widget  iw;
unsigned long *ix;
unsigned long *iy;
unsigned long *iheight;
unsigned long *iwidth;
    {
    DefineWidgetParts_(PannedImage,iw);
    RenderContext ctx = &Context_(iw);
    IdsRenderCallback rcb = PropRnd_(ctx);
    long roi_x;
    long roi_y;

#ifdef TRACE
printf( "Entering Routine ZoomingCheckMotif in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    LimitSrcWrk( iw ); 
    if (Zooming_(iw))
	{
	XPoint src_xpoint, dst_xpoint;
        RenderContextXie    xiectx = &XieContext_(iw);
        DataForXie          xiedat;

        xiedat = PriXie_(xiectx);	
	/* 
	** If we are performing a zoom, set up the internal ROI 
	** so that a crop will be performed.
	*/
	src_xpoint.x = ZstartX_(iw) + rSrcX_(iw) - rWrkX_(iw);
	src_xpoint.y = ZstartY_(iw) + rSrcY_(iw) - rWrkY_(iw);

	IdsXmGetCoordinates(
        		iw,
		        &src_xpoint,
		        &dst_xpoint,
		        1,
		        Ids_RenderedCoordinates);

	/* Set roi */
	roi_x = dst_xpoint.x;
	roi_y = dst_xpoint.y;

	/* Get lower right coordinates */
	src_xpoint.x = (ZstartX_(iw)  + rSrcX_(iw) - rWrkX_(iw)) 
				+ Zwidth_(iw);
	src_xpoint.y = (ZstartY_(iw) + rSrcY_(iw) - rWrkY_(iw)) 
				+ Zheight_(iw);
	IdsXmGetCoordinates(
        		iw,
		        &src_xpoint,
		        &dst_xpoint,
		        1,
		        Ids_RenderedCoordinates);

	/* if there is currently a roi set (meaning that image is zoomed)
	** adjust the current roi based on the scale factor of the 
	** currently zoomed roi.
	*/

	*ix = roi_x;
        *iy = roi_y;
	*iwidth  = abs (dst_xpoint.x - *ix) ;
	*iheight  = abs (dst_xpoint.y - *iy) ;
	iZXSc_(rcb) = iXSc_(rcb);
	iZYSc_(rcb) = iYSc_(rcb);
	            
	if (xiedat != 0)
	    {
            IULx_(xiedat) = 0;
	    IULy_(xiedat) = 0;
	    }

	XClearArea( XtDisplay(iw), XtWindow(iw), 0, 0,
			    WrkW_(iw), WrkH_(iw), FALSE );
	}
    else
	{
	if ((Zwidth_(iw) == 0) && (Zheight_(iw) == 0))
	    /* Resetting image to scale to fit -- Ok to zero out these */
	    {
	    *ix = 0;
	    *iy = 0;
	    *iwidth = 0;
	    *iheight = 0;
	    iZXSc_(rcb) = 0.0;
	    iZYSc_(rcb) = 0.0;
	    }
	else
            if (!Panning_(iw) )
	        XClearArea( XtDisplay(iw), XtWindow(iw), 0, 0,
			    WrkW_(iw), WrkH_(iw), FALSE );
	}
#ifdef TRACE
printf( "Leaving Routine ZoomingCheckMotif in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    }

/*****************************************************************************
**  RedrawRegion
**
**  FUNCTIONAL DESCRIPTION:
**
**      Redraw an exposed region of the image in the image Wrk area.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	expose	- XExposeEvent structure
**
*****************************************************************************/
static void RedrawRegion( iw, expose )
    Widget iw;
    XExposeEvent *expose;
    {
    DefineWidgetParts_(Image,iw);
    RenderContext      ctx    = &Context_(iw);
    RenderContextXie   xiectx = &XieContext_(iw);
    IdsRenderCallback  rcb    = PropRnd_(ctx);
    DataForXie         xiedat;
    int sx, sy, sw, sh, wx, wy;
    
#ifdef TRACE
printf( "Entering Routine RedrawRegion in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    wx = expose->x;					    /* Wrk X	    */
    wy = expose->y;					    /* Wrk Y	    */
    sx = expose->x + cSrcX_(iw) - cWrkX_(iw);		    /* Src X	    */
    sy = expose->y + cSrcY_(iw) - cWrkY_(iw);		    /* Src Y	    */
        
    sw = expose->width  + sx > Iwide_(iw) ? Iwide_(iw) - sx /* Src Width... */
           : expose->width  + wx >  WrkW_(iw) ?  WrkW_(iw) - wx 
							    /*..keep within */
					  : expose->width;  /*..image limits*/

    sh = expose->height + sy > Ihigh_(iw) ? Ihigh_(iw) - sy /* Src Height.. */
           : expose->height + wy >  WrkH_(iw) ?  WrkH_(iw) - wy 
							    /*..keep within */
    					  : expose->height; /*..image limits*/

    if( sx < 0 )
	{					    /* beyond left of image */
	sw += sx;				    /* adjust Src Width	    */
	wx -= sx;				    /* adjust Wrk X	    */
	sx  = 0;				    /* reset  Src X	    */
	}

    if( sy < 0 )
	{					    /* beyond top of image  */
	sh += sy;				    /* adjust Src Height    */
	wy -= sy;				    /* adjust Wrk Y	    */
	sy  = 0;				    /* reset  Src Y	    */
	}

    if( sw > 0 && sh > 0 )			    /* anything to redraw ? */
        {
        /*
        ** Transfer the image region to the widget window (from Photomap 
	** Pixmap or XImage).
        */

        if( intCompute_(rcb) == Ids_XieServer )
	    {
	    if( DPhoto_(xiectx) != NULL && iProto_(rcb) == Ids_Photomap )
	               XieExport( DPhoto_(xiectx),   /* server Photo{flo|map}*/
                       XtWindow(iw),                 /* window id            */
                       GC_(iw),                      /* GC used              */
                       rSrcX_(iw), rSrcY_(iw),       /* from:  img X,Y       */
                       rWrkX_(iw), rWrkY_(iw),       /* to: drawable X,Y     */
                       SrcW_(iw),                    /* width                */
                       SrcH_(iw),                    /* height               */
                       LPhoto_(xiectx),              /* client LUT           */
                       Cmap_(ctx),                   /* colormap             */
                       MchLim_(ctx),                 /* match_limit          */
                       GraLim_(ctx) );               /* gray_limit           */
	    else
                if (iProto_(rcb) != Ids_Window)
                    PixmapToXieWindow_(iw, sx, sy, sw, sh, wx, wy);
                else
                    PixmapToXieWindow_(iw, 0, 0,
                   ((WrkW_(iw) > Iwide_(iw)) ? Iwide_(iw) : WrkW_(iw)),
                     ((WrkH_(iw) > Ihigh_(iw)) ? Ihigh_(iw) : WrkH_(iw)),
                         cWrkX_(iw), cWrkY_(iw));
	    } /* end xie server */
        else  /* not computing on server */
	    if( !PixmapToWindow_(iw, sx, sy, sw, sh, wx, wy) )
	        ImageToWindow( iw, sx, sy, wx, wy, sw, sh);
        }/* End if anything to redisplay */
    else
	{
        if (iProto_(rcb) == Ids_Window)
            Redraw_(iw) = FALSE;
        else
            Redraw_(iw) |= LimitSrcWrk( iw );
	}
#ifdef TRACE
printf( "Leaving Routine RedrawRegion in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    }

/*****************************************************************************
**  ImageCallback
**
**  FUNCTIONAL DESCRIPTION:
**
**      Image widget application callback routine: the image has been
**	repositioned in the window: update scrollbars and notify application.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget (or subclass)
**	bars	- flag indicating whether or not scrollbars should be updated
**	reason	- callback reason
**	event	- XEvent structure
**
*****************************************************************************/
static void ImageCallback( iw, bars, reason, event )
    Widget   iw;
    Boolean  bars;
    int	     reason;
    XEvent  *event;
{
    DefineWidgetParts_(Image,iw);
    static IdsDragCallbackStruct dcb;
    static IdsViewCallbackStruct vcb;
    static IdsZoomCallbackStruct zcb;
    Arg al[2];
    int ac;

#ifdef TRACE
printf( "Entering Routine ImageCallback in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( bars && EnableH_(iw) )

        {
	long max_width;
	/*
	**  Update the horizontal scrollbar slider.
	*/
	max_width = (Iwide_(iw) > WrkW_(iw)) ? Iwide_(iw) : WrkW_(iw);
        ac = 0;
        SETARG_( al, ac, XmNmaximum, max_width);
        XtSetValues( Hbar_(iw), al, ac );
	XmScrollBarSetValues( Hbar_(iw), rSrcX_(iw), WrkW_(iw),
					   Incr_(iw), WrkW_(iw), FALSE );
	}

    if( bars && EnableV_(iw) )
	{
	long max_height;

	max_height = (Ihigh_(iw) > WrkH_(iw)) ? Ihigh_(iw) : WrkH_(iw);
        ac = 0;
        SETARG_( al, ac, XmNmaximum, max_height);
        XtSetValues( Vbar_(iw), al, ac );
	/*
	**  Update the vertical scrollbar slider.
	*/
	XmScrollBarSetValues( Vbar_(iw), rSrcY_(iw), WrkH_(iw),
					   Incr_(iw), WrkH_(iw), FALSE );
	}

    switch( reason )
	{
    case IdsCRDragImage :
	if( Callback_( DragCB_(iw) ))
	    {
	    dcb.reason	      = reason;		/* always IdsCRDrag	    */
	    dcb.event	      = event;		/* XEvent pointer	    */
	    dcb.source_x      = cSrcX_(iw);	/* x within image	    */
	    dcb.source_y      = cSrcY_(iw);	/* y within image	    */
	    dcb.window_x      = cWrkX_(iw);	/* x within Wrk area	    */
	    dcb.window_y      = cWrkY_(iw);	/* y within Wrk area	    */
	    XtCallCallbacks( iw, IdsNdragCallback, &dcb );
	    }
	break;

    case IdsCRZoomImage :
	if( Callback_( ZoomCB_(iw) ))
	    {
	    zcb.reason	      = reason;		/* always IdsCRDrag	    */
	    zcb.event	      = event;		/* XEvent pointer	    */
	    ZoomingCheckMotif(iw, 
		&zcb.start_x, 			/* Zoom starting x point    */
		&zcb.start_y, 			/* Zoom starting y point    */
		&zcb.width, 			/* Zoom area width          */
		&zcb.height);			/* Zoom area height         */
	    XtCallCallbacks( iw, IdsNzoomCallback, &zcb );
	    }
	break;

    case IdsCRViewChanged :
	if( Callback_( ViewCB_(iw) ))
	    {
	    /*
	    **  Load the callback structure.
	    */
	    vcb.reason	      = reason;		/* always IdsCRViewChanged  */
	    vcb.event	      = event;		/* event pointer (or NULL)  */
	    vcb.source_x      = cSrcX_(iw);	/* x  within image	    */
	    vcb.source_y      = cSrcY_(iw);	/* y  within image	    */
	    vcb.source_width  = SrcW_(iw);	/* width  displayed	    */
	    vcb.source_height = SrcH_(iw);	/* height displayed	    */
	    vcb.window_x      = cWrkX_(iw);	/* x  within Wrk area	    */
	    vcb.window_y      = cWrkY_(iw);	/* y  within Wrk area	    */
	    vcb.window_width  = WrkW_(iw);	/* width  of Wrk area	    */
	    vcb.window_height = WrkH_(iw);	/* height of Wrk area	    */
	    vcb.frame_depth   = Ideep_(iw);	/* depth  of frame	    */
	    vcb.frame_width   = Iwide_(iw);	/* width  of frame	    */
	    vcb.frame_height  = Ihigh_(iw);	/* height of frame	    */
	    XtCallCallbacks( iw, IdsNviewCallback, &vcb );
	    }
	break;
	}
#ifdef TRACE
printf( "Leaving Routine ImageCallback in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  VisibilityChange
**
**  FUNCTIONAL DESCRIPTION:
**
**      Visibility notification, save new visibility state.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	event	- XVisibilityEvent structure
**
*****************************************************************************/
static void VisibilityChange( iw, event )
    Widget	       iw;
    XVisibilityEvent  *event;
{
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine VisibilityChange in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Save the new visibility state: VisibilityUnObscured,
    **	    VisibilityPartiallyObscured or VisibilityFullyObscured.
    **
    **	The visible state determines if/how we draw to the Wrk area.
    */
    Visibility_(iw) = event->state;
#ifdef TRACE
printf( "Leaving Routine VisibilityChange in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ScrollComplete
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called by scrollbar "None<Btn1Up>" events.  These
**	events originate from the scrollbar itself or one of its children.
**
**  FORMAL PARAMETERS:
**
**	w	- Widget in which event occured
**	event	- structure describing event
**
*****************************************************************************/
static void ScrollComplete( w, event )
   Widget  w;
   XEvent *event;
{
    Widget iw;
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine ScrollComplete in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **  The Image Widget is either the parent or grandparent.
    */
    iw = XtIsSubclass( XtParent(w), (WidgetClass)imageWidgetMotifClass)
			    ? XtParent(w) : XtParent(XtParent(w));

    if( DoBars_(iw) && ActBar_(iw) )
	/*
	**  Scrollbar activity (drag or timer driven scrolling) is complete,
	**  notify application of final image position.
	*/
	ImageCallback( iw, FALSE, IdsCRViewChanged, event );

    ActBar_(iw) = FALSE;
#ifdef TRACE
printf( "Leaving Routine ScrollComplete in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  CreateBars
**
**  FUNCTIONAL DESCRIPTION:
**
**	Initialize scrollbars and inter-scrollbar label: create, realize, 
**	add translations, configure, and manage.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**
*****************************************************************************/
static void CreateBars( iw )
   Widget iw;
{
    DefineWidgetParts_(Image,iw);
    static XtCallbackRec callback[2] = {{NULL, 0}, {NULL, 0},};
    static Arg hbar[] = {
	{ XmNorientation,	    (XtArgVal)XmHORIZONTAL	}, };
    static Arg vbar[] = {
	{ XmNorientation,	    (XtArgVal)XmVERTICAL	}, };
    Arg al[8];
    int ac;

#ifdef TRACE
printf( "Entering Routine CreateBars in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Create scrollbar widgets.
    */
    Hbar_(iw) = (Widget) XmCreateScrollBar( iw, "hbar", hbar, XtNumber(hbar) );
    Vbar_(iw) = (Widget) XmCreateScrollBar( iw, "vbar", vbar, XtNumber(vbar) );
    /*
    **	Create pixmap for inter-scrollbar label widget and draw a style guide
    **	conforming inter-scrollbar button using the scrollbar pre-Realize
    **	foreground and background colors.
    */
    SPix_(iw) = XCreatePixmap( XtDisplay(iw), RootWindowOfScreen(XtScreen(iw)),
			       XtWidth(Vbar_(iw)), XtHeight(Hbar_(iw)),
			       Depth_(iw));
/*
    XFillRectangle( XtDisplay(iw), SPix_(iw), BackGC_(Hbar_(iw)),
		    0, 0, XtWidth(Vbar_(iw)), XtHeight(Hbar_(iw)) );
    XDrawRectangle( XtDisplay(iw), SPix_(iw), ForeGC_(Hbar_(iw)),
		    XtWidth(Vbar_(iw)) / 5,     XtHeight(Hbar_(iw)) / 5,
		    (int) ( ceil( XtWidth(Vbar_(iw)) * 0.6 )),
		    (int) ( ceil(XtHeight(Hbar_(iw)) * 0.6 )) );
*/
    /*
    **	Create the inter-scrollbar label widget.
    */
    ac = 0;
    SETARG_( al, ac, XmNlabelPixmap,      SPix_(iw)			);
    SETARG_( al, ac, XmNlabelType,   XmPIXMAP			);
    SETARG_( al, ac, XmNborderWidth, XtBorderWidth(Vbar_(iw))	);
    SBox_(iw) = (Widget) XmCreateLabel( iw, "sbox", al, ac );

    /*
    **	Realize the widgets.
    */
    XtRealizeWidget( Hbar_(iw) );
    XtRealizeWidget( Vbar_(iw) );
    XtRealizeWidget( SBox_(iw) );
    /*
    **  Add callbacks and "None<Btn1Up>" override translations to scrollbars.
    **  Note: the scrollbars MUST be realized before adding the translations.
    */
    ac = 0;
    SETARG_( al, ac, XmNvalueChangedCallback,  callback         );
    SETARG_( al, ac, XmNtoTopCallback,         callback         );
    SETARG_( al, ac, XmNtoBottomCallback,      callback         );
    /*
    These are not supported under Motif. To Find a Suitable one
    SETARG_( al, ac, XmNtranslations,          ScrollPageParsed );
    SETARG_( al, ac, DwtNtranslations1,         ScrollAro1Parsed );
    SETARG_( al, ac, DwtNtranslations2,         ScrollAro2Parsed );
    */
    callback[0].callback = (XmVoidProc) HorizontalCallback;
    XtSetValues( Hbar_(iw), al, ac );
    callback[0].callback = (XmVoidProc) VerticalCallback;
    XtSetValues( Vbar_(iw), al, ac );
    
    DoBars_(iw) = TRUE;			    /* enable use of scrollbars	    */
#ifdef TRACE
printf( "Leaving Routine CreateBars in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ConfigureBars
**
**  FUNCTIONAL DESCRIPTION:
**
**      Configure and manage (or unmanage) scrollbars.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**
*****************************************************************************/
static void ConfigureBars( iw )
   Widget iw;
{
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine ConfigureBars in module IDS_STATIC_IMAGE_MOTIF \n");
#endif

    if( DoBars_(iw) )
	{
	DoBars_(iw) = False;		/* ignore scrollbar callbacks	    */

	if( EnableH_(iw) )		/* (un)manage horizontal scrollbar  */
	    {
	    XtConfigureWidget( Hbar_(iw), -XtBorderWidth(Hbar_(iw)), WrkH_(iw),
					  WrkW_(iw), XtHeight(Hbar_(iw)),
					  XtBorderWidth(Hbar_(iw)));
	    ManageBar( Hbar_(iw), cSrcX_(iw), WrkW_(iw), Iwide_(iw), Incr_(iw),
		       DynBar_(iw) ? (XmVoidProc)HorizontalCallback : NULL);

	    }
	else if( XtIsManaged( Hbar_(iw) ))
	     XtUnmanageChild( Hbar_(iw) );

	if( EnableV_(iw) )		/* (un)manage vertical scrollbar    */
	    {
	    XtConfigureWidget( Vbar_(iw), WrkW_(iw), -XtBorderWidth(Vbar_(iw)),
					  XtWidth(Vbar_(iw)), WrkH_(iw),
					  XtBorderWidth(Vbar_(iw)));
	    ManageBar( Vbar_(iw), cSrcY_(iw), WrkH_(iw), Ihigh_(iw), Incr_(iw),
		       DynBar_(iw) ? (XmVoidProc)VerticalCallback : NULL );
	    }
	else if( XtIsManaged( Vbar_(iw) ))
	     XtUnmanageChild( Vbar_(iw) );

	ManageBox( iw );		/* (un)manage inter-scrollbar label */

	DoBars_(iw) = TRUE;		/* re-enable scrollbar callbacks    */
	}
#ifdef TRACE
printf( "Leaving Routine ConfigureBars in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ManageBar
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called to manage our scrollbars.
**
**  FORMAL PARAMETERS:
**
**	bar	- Scroll Widget 
**      src	- src position currently displayed
**      wrk     - Wrk dimension
**      img     - Img dimension
**      inc     - scroll button increment
**	drag	- scrollbar drag callback routine
**
*****************************************************************************/
static void ManageBar( bar, src, wrk, img, inc, drag )
   Widget   bar;
   int	    src, wrk, img, inc;
   XmVoidProc drag;
{
    static XtCallbackRec callback[2] = {{NULL, 0}, {NULL, 0},};
    static Arg al[] = {
	{ XmNvalue,	    (XtArgVal)0 },
	{ XmNsliderSize,    (XtArgVal)0 },
	{ XmNincrement,	    (XtArgVal)0 },
	{ XmNpageIncrement, (XtArgVal)0 },
	{ XmNmaximum,	    (XtArgVal)0 },
	{ XmNdragCallback, (XtArgVal)callback }, 
    };

#ifdef TRACE
printf( "Entering Routine ManageBar in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    al[0].value = (src<0) ? 0:src; /* position of the slider */   
    al[1].value = wrk; /* slider size         */
    al[2].value = inc; /* amount of button increment/decrement */
    al[3].value = wrk; /* amount of page increment/decrement */
    al[4].value = (img>wrk) ? img:wrk; /* slider maximum value */
    /*
    ** Quick fix for the warning messages when scroll is set. This has to be 
    ** looked in for more detail
    */
    callback[0].callback = drag; 
    XtSetValues( bar, al, XtNumber(al) );

    if( !XtIsManaged( bar ))
       XtManageChild( bar );
#ifdef TRACE
printf( "Leaving Routine ManageBar in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ManageBox
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called to manage and unmanage our inter scrollbar label.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**
*****************************************************************************/
static void ManageBox( iw )
   Widget   iw;
{
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine ManageBox in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( XtIsManaged( Hbar_(iw)) && XtIsManaged( Vbar_(iw)) )
	{
	/*
	**  Both scrollbars are managed, so show our inter scrollbar label box.
	*/
	XtConfigureWidget( SBox_(iw),
			   XtX(Vbar_(iw)),
			   XtY(Hbar_(iw)),
			   XtWidth(Vbar_(iw)),
			   XtHeight(Hbar_(iw)),
			   XtBorderWidth(Vbar_(iw)) );
	if( !XtIsManaged( SBox_(iw) ))
	   XtManageChild( SBox_(iw) );
	}
    else if( XtIsManaged( SBox_(iw) ))
         XtUnmanageChild( SBox_(iw) );
#ifdef TRACE
printf( "Leaving Routine ManageBox in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  HorizontalCallback
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called to handle horizontal scrollbar callbacks.
**
**  FORMAL PARAMETERS:
**
**	w	- Scroll Widget originating callback
**	param	-
**	cbs	- Scroll Widget callback structure
**
*****************************************************************************/
static void HorizontalCallback( w, param, cbs )
   Widget w;
   Opaque param;
   XmScrollBarCallbackStruct *cbs;
{
    Widget iw = XtParent(w);
    DefineWidgetParts_(Image,iw);
    RenderContext     ctx    = &Context_(iw);

#ifdef TRACE
printf( "Entering Routine HorizontalCallback in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( DoBars_(iw) )
	switch( cbs->reason )
	    {
	case XmCR_VALUE_CHANGED:
	    rSrcX_(iw)   = cbs->value;
	    Redraw_(iw) |= LimitSrcWrk( iw );
	    Redraw_(iw) = TRUE;

            if( Redraw_(iw) && (RedrawWindow( iw ) ||
                                VxtImagingOption(XtDisplay(iw), ctx)))
	        {
                if(!ActBar_(iw) )
                    ActBar_(iw) =  cbs->event == NULL
                                || cbs->event->type != ButtonRelease;

		ImageCallback(iw, FALSE,
			      IdsCRViewChanged,
			      cbs->event);
		}
	    break;


	case XmCR_DRAG:
	    rSrcX_(iw)   = cbs->value;
	    Redraw_(iw) |= LimitSrcWrk( iw );
            if  (VxtImagingOption(XtDisplay(iw), ctx))
		{
		/* Drag around what is currently visible in the window
		** rerender only when dropped.
		*/
	        if( Redraw_(iw) && RerenderWindowMotif( iw, TRUE ) )
		    {
		    if(!ActBar_(iw) )
		        ActBar_(iw) =  cbs->event == NULL
				|| cbs->event->type != ButtonRelease;
		    ImageCallback(iw, FALSE,
			      ActBar_(iw) ? IdsCRDragImage : IdsCRViewChanged,
			      cbs->event);
		    }
		}
	    else
		{
	        if( Redraw_(iw) && RedrawWindow( iw ) )
		    {
	             if(!ActBar_(iw) )
		            ActBar_(iw) =  cbs->event == NULL
				|| cbs->event->type != ButtonRelease;
		        ImageCallback(iw, FALSE,
			      ActBar_(iw) ? IdsCRDragImage : IdsCRViewChanged,
			      cbs->event);
		    }
		}
	    break;

	case XmCR_TO_BOTTOM:
	    rSrcX_(iw) -= WrkW_(iw);
	case XmCR_TO_TOP:
	    rSrcX_(iw)  += cbs->pixel;
	    Redraw_(iw) |= LimitSrcWrk( iw );
            if( Redraw_(iw) && (RedrawWindow( iw ) ||
                                VxtImagingOption(XtDisplay(iw), ctx)))
		ImageCallback( iw, DoBars_(iw), IdsCRViewChanged, cbs->event );
	    break;
	    }
#ifdef TRACE
printf( "Leaving Routine HorizontalCallback in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  VerticalCallback
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called to handle vertical scrollbar callbacks.
**
**  FORMAL PARAMETERS:
**
**	w	- Scroll Widget originating callback
**	param	-
**	cbs	- Scroll Widget callback structure
**
*****************************************************************************/
static void VerticalCallback( w, param, cbs )
   Widget w;
   Opaque param;
   XmScrollBarCallbackStruct *cbs;
{
    Widget iw = XtParent(w);
    DefineWidgetParts_(Image,iw);
    RenderContext     ctx    = &Context_(iw);

#ifdef TRACE
printf( "Entering Routine VerticalCallback in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( DoBars_(iw) )
	switch( cbs->reason )
	    {
	case XmCR_VALUE_CHANGED:
	    rSrcY_(iw)   = cbs->value;
	    Redraw_(iw) |= LimitSrcWrk( iw );
	    Redraw_(iw) = TRUE;
            
	    if( Redraw_(iw) && (RedrawWindow( iw ) || 
				VxtImagingOption(XtDisplay(iw), ctx)))
	        {
                if(!ActBar_(iw) )
                    ActBar_(iw) =  cbs->event == NULL
                                || cbs->event->type != ButtonRelease;
	     	ImageCallback(iw, FALSE,
			      IdsCRViewChanged,
			      cbs->event);
	  	}
	    break;

	case XmCR_DRAG:
	    rSrcY_(iw)   = cbs->value;
	    Redraw_(iw) |= LimitSrcWrk( iw );

            if  (VxtImagingOption(XtDisplay(iw), ctx))
		{
		/* 
		** Drag around what is currently visible in the window
		** rerender only when dropped.
		*/
	        if( Redraw_(iw) && RerenderWindowMotif( iw, TRUE ) )
	  	    {
	 	    if(!ActBar_(iw) )
	 	        ActBar_(iw) =  cbs->event == NULL
				|| cbs->event->type != ButtonRelease;
	 	    ImageCallback(iw, FALSE,
			      ActBar_(iw) ? IdsCRDragImage : IdsCRViewChanged,
			      cbs->event);
	 	    }
		}
	    else
	        {
	        if( Redraw_(iw) && RedrawWindow( iw) )
	  	    {
	 	    if(!ActBar_(iw) )
	 	        ActBar_(iw) =  cbs->event == NULL
				|| cbs->event->type != ButtonRelease;
	 	    ImageCallback(iw, FALSE,
			      ActBar_(iw) ? IdsCRDragImage : IdsCRViewChanged,
			      cbs->event);
	 	    }
		}
	    break;

	case XmCR_TO_BOTTOM:
	    rSrcY_(iw) -= WrkH_(iw);
	case XmCR_TO_TOP:
	    rSrcY_(iw)  += cbs->pixel;
	    Redraw_(iw) |= LimitSrcWrk( iw );
	    if( Redraw_(iw) && (RedrawWindow( iw ) || 
				VxtImagingOption(XtDisplay(iw), ctx)))
		ImageCallback( iw, DoBars_(iw), IdsCRViewChanged, cbs->event );
	    break;
	    }
#ifdef TRACE
printf( "Leaving Routine VerticalCallback in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ImageToWindow
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called to transfer XImage data directly to the window.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	ix,iy	- ximage xy coordinates
**	wx,wy	- window xy coordinates
**	w, h	- width and height
**
*****************************************************************************/
static void ImageToWindow( iw, ix, iy, wx, wy, w, h )
 Widget	   iw;
 int ix, iy, wx, wy, w, h;
{
    DefineWidgetParts_(Image,iw);
    int limit, offset = 0, width = w;

#ifdef TRACE
printf( "Entering Routine ImageToWindow in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
#ifdef GPX_HACKS
    /*
    **	The GPX implementation has a bug which causes unreliable results when
    **	using XPutImage to send image data which has a width greater than 
    **	the WidthOfScreen - BitmapUnit.  So we sub-divide the XPutImage calls
    **	if the width exceeds the BUG's limitation.
    */
    if( PlanesOfScreen(XtScreen(iw)) > 1 )
	for( limit = WidthOfScreen(XtScreen(iw)) - BitmapUnit(XtDisplay(iw));
			width > limit; offset += limit, width -= limit )
	    XPutImage( XtDisplay(iw), XtWindow(iw), GC_(iw), &Img_(iw), 
				 ix + offset, iy, wx + offset, wy, limit, h );
#endif
    /*
    **	Transfer all (or the remainder) of the image data requested.
    */
    XPutImage( XtDisplay(iw), XtWindow(iw), GC_(iw), &Img_(iw), 
				 ix + offset, iy, wx + offset, wy, width, h );
#ifdef TRACE
printf( "Leaving Routine ImageToWindow in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  SetWrkArea
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called to set the dimensions of the image Wrk area.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	width	- width  of widget window
**	height	- height of widget window
**
*****************************************************************************/
static void SetWrkArea( iw, width, height )
 Widget	   iw;
 Dimension width, height;
{
    DefineWidgetParts_(Image,iw);

#ifdef TRACE
printf( "Entering Routine SetWrkArea in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( ( EnableH_(iw) || EnableV_(iw) )  &&  Hbar_(iw) == NULL )
	/*
	**  Create scrollbars: they've been requested but not created yet.
	*/
	CreateBars( iw );
    /*
    **	Set work area equal to full widget window minus scrollbars.  Never let
    **	the Wrk area shrink out of existence, or scrollbars become unmanageable.
    */
    WrkW_(iw) = width  - (EnableV_(iw) ? TotalWidth_( Vbar_(iw)) : 0);
    WrkW_(iw) = MAX( (int)WrkW_(iw), 1 );

    WrkH_(iw) = height - (EnableH_(iw) ? TotalHeight_(Hbar_(iw)) : 0);
    WrkH_(iw) = MAX( (int)WrkH_(iw), 1 );

#ifdef TRACE
printf( "Leaving Routine SetWrkArea in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  LimitSrcWrk
**
**  FUNCTIONAL DESCRIPTION:
**
**      Determine source width & height, limit Src(x,y), and Wrk(x,y).
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**
**  FUNCTION VALUE:
**
**	TRUE	dimensions or coordinates differ from previous.
**	FALSE	no change from previous.
**
*****************************************************************************/
static Boolean LimitSrcWrk( iw )
    Widget iw;
{
    DefineWidgetParts_(Image,iw);
    Boolean change;
    int sw, sh;

#ifdef TRACE
printf( "Entering Routine LimitSrcWrk in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Set Src(w & h) to be the smaller of either the Wrk area or the image.
    */
    sw = MIN( WrkW_(iw), Iwide_(iw) );
    sh = MIN( WrkH_(iw), Ihigh_(iw) );
    /*
    **	Limit the requested source (x,y) to keep within the image bounds.
    */
    rSrcX_(iw) = sw == Iwide_(iw)  || rSrcX_(iw) < 0 ? 0 
		 : MIN(Iwide_(iw)-sw, rSrcX_(iw));

    rSrcY_(iw) = sh == Ihigh_(iw)  || rSrcY_(iw) < 0 ? 0
		 : MIN(Ihigh_(iw)-sh, rSrcY_(iw));
    /*
    **	Limit the requested Wrk(x,y) to keep within the Wrk area bounds.
    */
    rWrkX_(iw) = sw == WrkW_(iw)  || rWrkX_(iw) < 0 ? 0
		 : MIN(WrkW_(iw)-sw, rWrkX_(iw));
    rWrkY_(iw) = sh == WrkH_(iw)  || rWrkY_(iw) < 0 ? 0
		 : MIN(WrkH_(iw)-sh, rWrkY_(iw));
    /*
    **	Save the new dimensions and see if any of the above has changed.
    */
    change  = IfReplace_( SrcW_(iw) != sw, SrcW_(iw), sw );
    change |= IfReplace_( SrcH_(iw) != sh, SrcH_(iw), sh );
    change |= cSrcX_(iw) != rSrcX_(iw) || cSrcY_(iw) != rSrcY_(iw)
	   || cWrkX_(iw) != rWrkX_(iw) || cWrkY_(iw) != rWrkY_(iw);

#ifdef TRACE
printf( "Leaving Routine LimitSrcWrk in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    return( change );
}

/*****************************************************************************
**  ApplyGravity
**
**  FUNCTIONAL DESCRIPTION:
**
**	Reposition image according to window and/or source gravity.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	src	- flag: apply source gravity
**	dst	- flag: apply window gravity
**
*****************************************************************************/
static void ApplyGravity( iw, src, dst )
    Widget  iw;
    Boolean src, dst;
{
    DefineWidgetParts_(Image,iw);
    Boolean  x_wrk_gravity =   WrkW_(iw) >  Iwide_(iw);
    Boolean  y_wrk_gravity =   WrkH_(iw) >  Ihigh_(iw);
    int *x = x_wrk_gravity ? &rWrkX_(iw) : &rSrcX_(iw);
    int *y = y_wrk_gravity ? &rWrkY_(iw) : &rSrcY_(iw);

#ifdef TRACE
printf( "Entering Routine ApplyGravity in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    switch(Ids_CenterHorz & (x_wrk_gravity ?(dst ? WGrav_(iw) : Ids_NoGravity)
					   :(src ? SGrav_(iw) : Ids_NoGravity)))
	{
    case Ids_Left :
	*x = 0;
	break;
    case Ids_CenterHorz :
	*x = abs( WrkW_(iw) - Iwide_(iw) ) / 2;
	break;
    case Ids_Right :
	*x = ( x_wrk_gravity ? WrkW_(iw) : Iwide_(iw) ) - SrcW_(iw);
	}

    switch(Ids_CenterVert & (y_wrk_gravity ?(dst ? WGrav_(iw) : Ids_NoGravity)
					   :(src ? SGrav_(iw) : Ids_NoGravity)))
	{
    case Ids_Top :
	*y = 0;
	break;
    case Ids_CenterVert :
	*y = abs( WrkH_(iw) - Ihigh_(iw) ) / 2;
	break;
    case Ids_Bottom :
	*y = ( y_wrk_gravity ? WrkH_(iw) : Ihigh_(iw) ) - SrcH_(iw);
	}
#ifdef TRACE
printf( "Leaving Routine ApplyGravity in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}

/*****************************************************************************
**  ApplyScheme
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine calls RenderImage to carry out the action requested by
**	the render Mode and Scheme .  If a new (or purged) rendering results, 
**	we setup display coordinates in preparation for displaying the image.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget 
**	expose	- XExposeEvent structure
**
*****************************************************************************/
static void ApplyScheme( iw, expose )
    Widget iw;
    XExposeEvent *expose;
{
    DefineWidgetParts_(Image,iw);
    RenderContext ctx = &Context_(iw);
    IdsRenderCallback rcb   = PropRnd_(ctx);
    unsigned long fid = cFid_(iw);
    XieImage xieimage = cXieImg_(iw);

#ifdef TRACE
printf( "Entering Routine ApplyScheme in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    if( RenderImage_( iw, expose ) )
	{
	/*
	**  We have a new rendering, adjust display placement and dimensions.
	*/
	cSrcX_(iw)   = INVALID_COORD;	/* invalidate Wrk area image data   */
	cSrcY_(iw)   = INVALID_COORD;
	Redraw_(iw) |= LimitSrcWrk( iw );

	/*
	**  If it is also a new fid, apply source and/or window gravity.
	*/
       if(  intCompute_(rcb) == Ids_IslClient && cFid_(iw) != 0 &&
                                                            cFid_(iw) != fid )
	    ApplyGravity( iw, TRUE, TRUE );

        /*
        **  Or if it is a new xieimage apply source and/or window gravity.
        */
        if( intCompute_(rcb) == Ids_XieServer && cXieImg_(iw) != 0
                                                && cXieImg_(iw) != xieimage )
            ApplyGravity( iw, TRUE, TRUE );
  
	/*
	**  Clear the Wrk area if the image is smaller than the Wrk area.
	*/
	if( SrcW_(iw) < WrkW_(iw) || SrcH_(iw) < WrkH_(iw) )
	    XClearArea( XtDisplay(iw), XtWindow(iw), 0, 0,
			    WrkW_(iw), WrkH_(iw), FALSE );
	}
    /*
    **	Set scrollbar parameters and (un)manage them.
    */

    ConfigureBars( iw );
#ifdef TRACE
printf( "Leaving Routine ApplyScheme in module IDS_STATIC_IMAGE_MOTIF \n");
#endif

}

/*****************************************************************************
**  SendExpose
**
**  FUNCTIONAL DESCRIPTION:
**
**      Send an XExposeEvent to our own widget window.
**
**  FORMAL PARAMETERS:
**
**	iw	- Image Widget.
**
*****************************************************************************/
static void SendExpose( iw )
    Widget  iw;
{
    DefineWidgetParts_(Image,iw);
    static XExposeEvent event = { Expose };

#ifdef TRACE
printf( "Entering Routine SendExpose in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
    /*
    **	Send an XExposeEvent to our own window to:
    **    1  rerender the image, if anything affecting rendition has changed,
    **    2  redraw the image in the Wrk area (even if nothing has changed).
    */
    if( XtIsRealized(iw) && Visibility_(iw) != VisibilityFullyObscured )
	{
	event.display = XtDisplay(iw);
	event.window  = XtWindow(iw);
	event.x	      = 0;
	event.y	      = 0;
	event.width   = WrkW_(iw);
	event.height  = WrkH_(iw);
	XSendEvent(XtDisplay(iw), 
		   XtWindow(iw), 
		   FALSE, 
		   ExposureMask, 
		   (XEvent *) &event );
	}
#ifdef TRACE
printf( "Leaving Routine SendExpose in module IDS_STATIC_IMAGE_MOTIF \n");
#endif
}
