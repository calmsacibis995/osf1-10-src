
/***************************************************************************
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

/* define IdsVMS if we want the VMS function bindings to compile */
#ifdef VMS
#define IdsVMS
#endif

/* define MOTIF if we want the MOTIFfunction bindings to compile */
#define MOTIF

#ifdef VMS
#include <decw$include/StringDefs.h>
#include <MRMAPPL.H>
#include <XM.H>
#else
#include <X11/StringDefs.h>  
#include <Mrm/MrmAppl.h>
#include <Xm/XmP.h>
#include <X11/CoreP.h> /* put by Dhiren 10/29/93 */
#include <X11/CompositeP.h> /* put by Dhiren 10/29/93 */
#include <X11/ConstrainP.h> /* put by Dhiren 10/29/93 */
#include <Xm/ManagerP.h> /* put by Dhiren 10/29/93 */
#endif

#include <ids__widget.h>     /* IDS public/private defs.                      */


/*
**  Function pointer types
*/
typedef Boolean         (*IDS_BoolProc) ();
typedef unsigned long   (*IDS_LongProc) ();
typedef void            (*IDS_VoidProc) ();
   
#define RenderIndex (XmManagerIndex + 1)
#define ImageIndex  (RenderIndex + 1)
#define PannedIndex (ImageIndex + 1)

/*
** To have upward compatability. Note NEW_RESOURCE_STRUCT is removed.
*/
#if defined(__STDC__) && !(UNIXCPP)
#define IdsPartOffset_(index,part,variable) \
        (index##Index << XmHALFLONGBITS) + XtOffsetOf(part##Part,variable)
#else
#define IdsPartOffset_(index,part,variable) \
        (index/**/Index << XmHALFLONGBITS) + XtOffsetOf(part/**/Part,variable)
#endif

#define GetIdsPartOffset_(r,offset) \
        ((r)->resource_offset & 0xFFFF) + \
         (*(offset))[ (r)_>resource_offset >> XmHALFLONGBITS];
#define CoreIndex XmCoreIndex

/*
**  These macros are moved from ids$$macros.h which are MOTIF specific
*/
#define Callback_(callback)     (callback != NULL)

    /*
    **  Macros for generating pointer variable names for IDS widgets.
    */
#if defined (__STDC__) && !defined(UNIXCPP)
#define IdsBase_(w) (_idsBase_##w)	/* variable name for widget (w)	    */
#define IdsPart_(w) (_idsPart_##w)	/* variable name for IDS part of (w)*/
#else
#define IdsBase_(w) (_idsBase_/**/w)	/* variable name for widget (w)	    */
#define IdsPart_(w) (_idsPart_/**/w)	/* variable name for IDS part of (w)*/
#endif
    /*
    **  Macro for generating IDS widget part pointer variables.
    */
	/*  Type cast a widget to the specified 'class' and generate a pair of
	**  variables.  The first is a pointer to the complete widet, which is
	**  usable for all superclasses above the IDS part of the widget.  The
	**  second is a pointer to just the IDS part of the widget.
	**
	**  For example, to generate a variable pair for class 'PannedImage'
	**	based on argument 'pw', specify:
	**
	**	    DefineWidgetParts_(PannedImage,pw);	    {NOTE: No spaces}
	**
	**  Which generates the following variables:
	**
	**	PannedImageWidget  _idsBase_pw (PannedImageWidget)pw;
	**	IdsPannedImagePart _idsPart_pw (IdsPannedImagePart)(pw+OFFSET);
	**
	**	    OFFSET is the offset from the widget base to the IDS parts
	**	    which is determined during the ClassInitialize call to the 
	**	    highest IDS super-class, RenderImage.
	**
	**  All this ugliness is imposed on IDS by the DECwindows developers
	**  in the name of 'upward compatibility of widgets'.  It is required
	**  only of widgets NOT under their direct control, so that they can
	**  change the widget structure above IDS and still have applications
	**  link and run without IDS being re-compiled.
	*/
#if defined(__STDC__) && !defined(UNIXCPP)
#define DefineWidgetParts_(c,w) c##Widget IdsBase_(w)=(c##Widget)w;\
	 Ids##c##Part IdsPart_(w)=(Ids##c##Part)(((char *) (w))+\
         renderImageWidgetMotifClassRec.render_image_class.offsets[RenderIndex])
#else
#define DefineWidgetParts_(c,w) c/**/Widget IdsBase_(w)=(c/**/Widget)w;\
	 Ids/**/c/**/Part IdsPart_(w)=(Ids/**/c/**/Part)(((char *) (w))+\
         renderImageWidgetMotifClassRec.render_image_class.offsets[RenderIndex])
#endif

#define ForePix_(w)     (XmManagerPart_(w)->foreground)
#define XmManagerPart_(w)   ((XmManagerPart *)(((char *)(w))\
                            +renderImageWidgetMotifClassRec.render_image_class\
                            .offsets[XmManagerIndex]))
  

    /*
    **  Call RenderImageClass functions from subclasses.
    */
#define UpdateCallback_(o,ol,n,nl,name) \
        ((* renderImageWidgetMotifClass->render_image_class.update_callback) \
                                            ((o),(ol),(n),(nl),(name)))
#define RenderImage_(w,e) \
     ((* renderImageWidgetMotifClass->render_image_class.render_image)((w),(e)))

    /*
    **  Call ImageClass functions from subclasses.
    */
#define RedrawWindow_(w) \
        ((* imageWidgetMotifClass->image_class.redraw_window)((w)))
#define RedrawRegion_(w,e) \
        ((* imageWidgetMotifClass->image_class.redraw_region)((w),(e)))
#define ImageCallback_(w,b,r,e) \
        ((* imageWidgetMotifClass->image_class.image_callback)((w),(b),(r),(e)))


/*---------------------*/
/* render image widget */
/*---------------------*/
typedef struct {		    /* Render Image Part of Class record    */
	XmOffsetPtr	offsets;	    /* offsets to instance parts    */
	IDS_VoidProc    update_callback;    /* update callback list	    */
	IDS_BoolProc	render_image;	    /* create an image rendering    */
	XtProc		pad0;		    /* reserved for future use	    */
	XtProc		pad1;		    /* reserved for future use	    */
	caddr_t         extension;	    /* Pointer to extension record  */
} RenderImageClassPart;

typedef struct _RenderImageMotifClassRec {  /* Render Image Class record    */
	CoreClassPart		core_class;
        CompositeClassPart      composite_class;
        ConstraintClassPart     constraint_class;
        XmManagerClassPart      manager_class;
	RenderImageClassPart	render_image_class;
} RenderImageMotifClassRec, *RenderImageMotifClass;

typedef struct {		/* Render Image Part of instance record	    */
	XImage			 ximage;
	Pixmap		         pixmap;
	GC		         zero_max_gc;
	GC		         zero_min_gc;
	GC		         image_gc;
        unsigned long            rendered_img;
	unsigned long		 rendered_fid;
	unsigned long		 rendered_pho;
	XtCallbackList		 xielist_callback;
	XtCallbackList		 render_callback;
	XtCallbackList		 work_notify_callback;
	XtCallbackList		 save_callback;
	XtCallbackList		 error_callback;
	RenderContextStruct	 context;
	RenderContextStructXie	 xiecontext;
	IdsRenderCallbackStruct	 current;
	IdsRenderCallbackStruct	 proposed;
	Boolean			 copy_fid;
	Boolean			 force_copy_fid;
	Boolean			 _pad_[2];
} RenderImagePart;

typedef struct {		/* IDS Part of Render Image instance record */
	RenderImagePart	  render;
} IdsRenderImagePartRec, *IdsRenderImagePart;

typedef struct {		/* Render Image Widget instance record	    */
	CorePart	  core;
        CompositePart     composite;
        ConstraintPart    constraint;
        XmManagerPart     manager;
	RenderImagePart	  render;
} RenderImageWidgetRec,  *RenderImageWidget;

/*---------------*/
/*  image widget */
/*---------------*/
typedef struct {			    /* Image Part of Class record   */
	XmOffsetPtr	offsets;	    /* offsets to instance parts    */
	IDS_BoolProc    redraw_window;	    /* Redraw the entire window	    */
	IDS_VoidProc    redraw_region;	    /* Redraw an exposed region	    */
	IDS_VoidProc    image_callback;	    /* IdsCRViewChanged & IdsCRDrag */
	XtProc		pad0;		    /* reserved for future use	    */
	caddr_t         extension;	    /* Pointer to extension record  */
} ImageClassPart;
typedef struct _imageMotifClassRec {		    /* Image Class record	    */
	CoreClassPart		core_class;
        CompositeClassPart      composite_class;
        ConstraintClassPart     constraint_class;
        XmManagerClassPart      manager_class;
	RenderImageClassPart	render_image_class;
	ImageClassPart		image_class;
} ImageMotifClassRec, *ImageMotifClass;
typedef struct {		/* Image Part of instance record	    */
	unsigned long		 visibility;
	unsigned long		 source_gravity;
	unsigned long		 window_gravity;
	unsigned long	         source_width;
	unsigned long		 source_height;
	unsigned long		 work_width;
	unsigned long		 work_height;
	DisplayCoordsStruct	 current_coords;
	DisplayCoordsStruct	 request_coords;
	XtCallbackList	 view_callback;
	XtCallbackList	 expose_callback;
	XtCallbackList	 drag_callback;
	XtCallbackList	 zoom_callback;
	Pixmap			 scroll_box_pix;
	Widget			 scroll_box;
	Widget			 horizontal_bar;
	Widget			 vertical_bar;
	int			 increment_bar;
	Boolean		         enable_horizontal;
	Boolean		         enable_vertical;
	Boolean		         enable_dynamic;
	Boolean		         scroll_active;
	Boolean		         do_bar_callbacks;
	Boolean		         redraw;
	Boolean		         _pad7_;
	Boolean		         _pad8_;
} ImagePart;
typedef struct {		/* IDS Part of Image instance record	    */
	RenderImagePart	  render;
	ImagePart	  image;
} IdsImagePartRec,       *IdsImagePart;
typedef struct {		/* Image Widget instance record		    */
	CorePart	  core;
        CompositePart     composite;
        ConstraintPart    constraint;
        XmManagerPart     manager;
	RenderImagePart	  render;
	ImagePart	  image;
} ImageWidgetRec,	 *ImageWidget;

/*---------------------*/
/* panned image widget */
/*---------------------*/
typedef struct {		    /* Panned Image Part of Class record    */
	XmOffsetPtr	offsets;	    /* offsets to instance parts    */
	XtProc		pad0;		    /* reserved for future use	    */
	XtProc		pad1;		    /* reserved for future use	    */
	caddr_t         extension;	    /* Pointer to extension record  */
} PannedImageClassPart;

typedef struct _PannedImageMotifClassRec {	    /* Panned Image Class record    */
	CoreClassPart		core_class;
        CompositeClassPart      composite_class;
        ConstraintClassPart     constraint_class;
        XmManagerClassPart      manager_class;
	RenderImageClassPart	render_image_class;
	ImageClassPart		image_class;
	PannedImageClassPart	panned_image_class;
} PannedImageMotifClassRec, *PannedImageMotifClass;

typedef struct {		/* Panned Image Part of instance record	    */
	int		    begin_x;
	int		    begin_y;
	int		    pan_x;
	int		    pan_y;
	int		    previous_x;
	int		    previous_y;
	Boolean		    panning;
	Boolean		    drag;
        Boolean 	    zooming;
        XtCallbackList      zoom_callback;
        int                 start_x;
        int                 start_y;
        int                 width;
        int                 height;
        GC 		    gc;
} PannedImagePart;

typedef struct {		/* IDS Part of Panned Image instance record */
	RenderImagePart	  render;
	ImagePart	  image;
	PannedImagePart	  panned;
} IdsPannedImagePartRec, *IdsPannedImagePart;

typedef struct {		/* Panned Image Widget instance record	    */
	CorePart	  core;
        CompositePart     composite;
        ConstraintPart    constraint;
        XmManagerPart     manager;
	RenderImagePart	  render;
	ImagePart	  image;
	PannedImagePart	  panned;
} PannedImageRec,        *PannedImageWidget;

/*
**  externals to provide class records and offsets to other widgets & debugger.
*/
#ifndef external
/*
** VAX VMS path (incl ALPHA)
*/
#if (defined(VAXC) || (defined(VMS) && defined(ALPHA)) )
#define external globalref
/*
** Ultrix path
*/
#else
#define external extern
#endif
#endif

#ifndef RENDERIMAGE
external RenderImageMotifClassRec       renderImageWidgetMotifClassRec;
external RenderImageMotifClass	        renderImageWidgetMotifClass;
#endif

#ifndef IMAGE
external ImageMotifClassRec		imageWidgetMotifClassRec;
external ImageMotifClass		imageWidgetMotifClass;
#endif

#ifndef PANNEDIMAGE
external PannedImageMotifClassRec	pannedImageWidgetMotifClassRec;
external PannedImageMotifClass	        pannedImageWidgetMotifClass;
#endif




















