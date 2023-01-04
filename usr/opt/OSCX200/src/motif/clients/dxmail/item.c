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
#ifndef lint
static char rcs_id[] = "@(#)$RCSfile: item.c,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/08/12 18:28:45 $";
#endif

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */

/**++
**  FACILITY:
**
**	Motif dxmail
**
**  ABSTRACT:
**
**	DECwindows Mail Item widget - based on Jay Bolgatz's 
** 	toggle button widget.
**
**  ENVIRONMENT:
**
**	ULTRIX/Motif
**
**  MODIFICATION HISTORY:
**	3.0	22-Aug-1989		MRR
**		Created from V2 source.
**	3.1	19-Nov-1989		MRR
**		Portability changes.
**	3.2	5-Jan-1990		MRR
**		Motif.
**	3.3	3-Apr-1990		MRR
**		Make includes explicit.
**	3.4	23-May-1990		MRR
**		/STAN=PORT
**	3.5	6-Jun-1990		MRR
**		Support Motif keyboard traversal.
**	3.6	14-Jun-1990		MRR
**		Use XtLastTimestampProcessed instead of CurrentTime.
**	3.7	5-Jul-1990		HP
**		Commenting out the XmPartOffset macro - should be fixed
**		in BL4.
**	3.8	6-Nov-1990		HP
**		Fixing looking through font list struct to get height.
**	3.9	27-Feb-1991		HP
**		Take out XmPrimitiveHelp routine - now provided by
**		toolkit.
**--
**/


/*---------------------------------------------------*/
/* include files                                     */
/*---------------------------------------------------*/

#define ITEM

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <X11/CoreP.h>
#include <Xm/Label.h>
#include <Xm/LabelP.h>

#include "itemP.h"
#include "item.h"

#define ItemIndex	(XmLabelIndex + 1)

#undef XmField

#define XmField(widget, offsetrecord, part, variable, type) \
	(*(type *)(((char *) (widget)) + offsetrecord[part/**/Index] + \
		XtOffset(part/**/Part *, variable)))
#if 0
#undef XmPartOffset

#define XmPartOffset(part, variable) \
        ((part/**/Index) << XmHALFLONGBITS) + XtOffset(part/**/Part *, variable)
#endif

#ifdef CoreIndex
#undef CoreIndex
#endif
#define CoreIndex XmCoreIndex

#define ItemField(w,class,field,type)				\
	XmField((w),						\
		 ((ItemClass)(w)->core.widget_class)->		\
		 	item_class.itemoffsets,class,		\
		 field,						\
		 type)

#define Background(w)		ItemField(w,Core,background_pixel,Pixel)
#define Width(w)		ItemField(w,Core,width,Dimension)
#define Height(w)		ItemField(w,Core,height,Dimension)

#define Foreground(w)		ItemField(w,XmPrimitive,foreground,Pixel)
#define Hel_cb(w)		ItemField(w,XmPrimitive,help_callback,XtCallbackList)
#define Highlighted(w)		ItemField(w,XmPrimitive,highlighted,Boolean)

#define Font(w)			ItemField(w,XmLabel,font,XmFontList)
#define ComPixmap(w)		ItemField(w,XmLabel,pixmap,Pixmap)
#define ForegroundGC(w)		ItemField(w,XmLabel,normal_GC,GC)
#define DrawMode(w)		ItemField(w,XmLabel,drawing_mode,unsigned char)
#define MarginWidth(w)		ItemField(w,XmLabel,margin_width,short)
#define MarginHeight(w)		ItemField(w,XmLabel,margin_height,short)
#define MarginLeft(w)		ItemField(w,XmLabel,margin_left,short)
#define MarginRight(w)		ItemField(w,XmLabel,margin_right,short)
#define MarginTop(w)		ItemField(w,XmLabel,margin_top,short)
#define MarginBottom(w)		ItemField(w,XmLabel,margin_bottom,short)

#define IndWidth(w)		ItemField(w,Item,indicator_width,short)
#define IndHeight(w)		ItemField(w,Item,indicator_height,short)
#define IndX(w)			ItemField(w,Item,indicator_x,int)
#define IndY(w)			ItemField(w,Item,indicator_y,int)
#define OrigForeground(w)	ItemField(w,Item,original_foreground,Pixel)
#define OrigBackground(w)	ItemField(w,Item,original_background,Pixel)
#define SingleCallback(w)	ItemField(w,Item,single_callback,XtCallbackList)
#define SingleConfirmCallback(w) ItemField(w,Item,singleconfirm_callback,XtCallbackList)
#define ExtendCallback(w)	ItemField(w,Item,extend_callback,XtCallbackList)

#define Value(w)		ItemField(w,Item,value,Boolean)

/* JV - Define a constant for the space between the pixmap and the text
**	to make it easier to adjust.
**	Define the Max macro here so we don't have to include DECWMAIL_COMMONDEFS
**	Define jimsbitmapkluge to turn kluges on or off
*/
#define jimsbitmapkluge
#define PIXELSBETWEEN	6
#ifndef Max
#define Max(a,b) ((a) > (b) ? (a) : (b))
#endif
#define TextLineHeight(font) (font->max_bounds.ascent + font->max_bounds.descent)
/*---------------------------------------------------*/
/* forward declarations                              */
/*                                                   */
/* this is a list of all of the procedures in this   */
/* module in the order they appear                    */
/*---------------------------------------------------*/

static void 		  SetIndicatorSize();
static void 		  ClassInitialize();
static void 		  Initialize();
static void		  Destroy();
static void		  DrawIndicator();
static void		  HandleExpose();
static void		  Redisplay();
static int 		  ItemCallback();
static Boolean		  SetValues();
static void		  LclUpdateCallback();
static void		  DisplayReverse();
static void		  DisplayNormal();
static void		  Arm();
static void 		  Extend();
static void		  Single();
static void		  SingleConfirm();
static void 		  Help();
Widget 			  DXmItem();
unsigned int		  DXmItemGetState();
void 			  DXmItemSetState();
Widget 			  DXmItemCreate();

/*
 * default event bindings
 */
static char DefaultTranslation[] =

    "<Btn1Down>:				Arm()\n\
     Ctrl<Btn1Up>:				Extend()\n\
     <Btn1Down>,<Btn1Up>:			Single()\n\
     <Btn1Down>(2+):				Arm()\n\
     <Btn1Up>(2+):				SingleConfirm()\n\
    ~Ctrl ~Shift ~Meta ~Alt<Key>osfSelect:	Extend()\n\
    ~Ctrl ~Shift ~Meta ~Alt<Key>space:		Single()\n\
     Ctrl ~Shift ~Meta ~Alt<Key>space:		Extend()\n\
    ~Ctrl ~Shift ~Meta ~Alt<Key>Return:		SingleConfirm()\n\
     Ctrl ~Shift ~Meta ~Alt<Key>Return:		SingleConfirm()\n\
     Ctrl ~Shift ~Meta ~Alt<Key>osfActivate:	SingleConfirm()\n\
    ~Ctrl ~Shift ~Meta ~Alt<Key>osfHelp:	Help()";

/*
 * transfer vector from translation manager action names to
 * address of routines 
 */

static XtActionsRec ActionsTable[] = 
    {
	{"Arm",			(XtActionProc)Arm},
        {"Extend",		(XtActionProc)Extend},
	{"Single",		(XtActionProc)Single},
	{"SingleConfirm",	(XtActionProc)SingleConfirm},
    	{"Help",                (XtActionProc)Help},
        {NULL, NULL}
    };

/*---------------------------------------------------*/
/* widget resources                                  */
/*                                                   */
/* these are the resources (attributes) that the     */
/* widget supports                                   */
/*---------------------------------------------------*/

static XmPartResource resources[] = 
{       
    /* item specific resources */

    {XmNvalue, XmCValue, XtRBoolean, sizeof(Boolean),
	 XmPartOffset(Item,value), 
	 	XtRImmediate, (caddr_t)False},

    {	XmNsingleSelectionCallback,
	XtCCallback, 
	XtRCallback, 
	sizeof (XtCallbackList),
	XmPartOffset (Item,single_callback), 
	XtRCallback, 
	(caddr_t) NULL},

    {	XmNextendedSelectionCallback,
	XtCCallback, 
	XtRCallback, 
	sizeof (XtCallbackList),
	XmPartOffset (Item, extend_callback), 
	XtRCallback, 
	(caddr_t) NULL},

    {	XmNdefaultActionCallback,
	XtCCallback, 
	XtRCallback, 
	sizeof (XtCallbackList),
	XmPartOffset(Item,singleconfirm_callback), 
	XtRCallback, 
	(caddr_t) NULL},

    {	XmNtraversalOn,
	XmCTraversalOn,
	XmRBoolean,
	sizeof(Boolean),
	XmPartOffset(XmPrimitive,traversal_on),
	XmRImmediate,
	(caddr_t) True},

    {	XmNhighlightThickness,
	XmCHighlightThickness,
	XmRHorizontalDimension,
	sizeof(Dimension),
	XmPartOffset(XmPrimitive,highlight_thickness),
	XmRImmediate,
	(caddr_t) 2},

    };

externaldef(itemclassrec) 
    ItemClassRec itemclassrec = 
{
    {
    	/* superclass	      */	(WidgetClass) &xmLabelClassRec,
    	/* class_name	      */	DXmSClassItem,
    	/* widget_size	   */	sizeof(ItemRec),
    	/* class_initialize   */    	ClassInitialize,
	/* chained class init */	NULL,
    	/* class_inited       */	FALSE,
    	/* initialize	      */	Initialize,
        /* initialize hook    */        NULL,
    	/* realize	      */	XtInheritRealize,
    	/* actions	      */	ActionsTable,
    	/* num_actions	      */	XtNumber(ActionsTable),
    	/* resources	      */	(XtResource *) resources,
    	/* num_resources      */	XtNumber(resources),
    	/* xrm_class	      */	NULLQUARK,
    	/* compress_motion    */	TRUE,
    	/* compress_exposure  */	XtExposeCompressMaximal,
        /* compress enter/exit*/        TRUE,
    	/* visible_interest   */	FALSE,
    	/* destroy	      */	Destroy,
    	/* resize	      */	XmInheritResize,
    	/* expose	      */	HandleExpose,
    	/* set_values	      */	(XtSetValuesFunc)SetValues,
        /* set values hook    */        NULL,
        /* set values almost  */        XtInheritSetValuesAlmost,
        /* get values hook    */        NULL,
    	/* accept_focus	      */	NULL,
        /* version            */        XtVersionDontCheck,
        /* callback offset    */        NULL,
        /* default trans      */        DefaultTranslation,
	/* query geo proc     */	XtInheritQueryGeometry,
    	/* disp accelerator   */	NULL,
    	/* extension          */    	NULL
    },

    {					/* XmPrimitive */
	/* XtWidgetProc	border_highlight */ (XtWidgetProc) _XtInherit,
	/* XtWidgetProc border_unhighlight */ (XtWidgetProc) _XtInherit,
	/* XtTranslations   translations */ XtInheritTranslations,
	/* XtActionProc	arm_and_activate */ SingleConfirm,
	/*XmGetValue Resource *get_resources */	NULL,
	/* int	num_get_resources */	    0,
	/* extension          */    NULL
    },
    {					/* Label */
				    (XtWidgetProc) _XtInherit,
				    (XtWidgetProc) _XtInherit,
				    XtInheritTranslations,
	/* extension          */    NULL
    },
    {					/* Item */
	/* offsets	      */    NULL,
	/* extension          */    NULL,
    }
};
externaldef(itemwidgetclass) 
    ItemClass itemwidgetclass 
		= (ItemClass) &itemclassrec;

/*---------------------------------------------------*/
/* this routine will initialize the class instance   */
/*---------------------------------------------------*/

static void ClassInitialize()
{
    XmResolvePartOffsets(itemwidgetclass, 
	&itemclassrec.item_class.itemoffsets);
}


                           
/*---------------------------------------------------*/
/* this routine initializes this instance of the     */
/* item					     */
/*---------------------------------------------------*/

/* ARGSUSED */
static void Initialize(request,w)
Widget request,w;
{
    ItemWidget tb = (ItemWidget) w;

    if (ComPixmap(tb))
    	SetIndicatorSize(tb);
    else
    {
	IndHeight(tb) = 0;
	IndWidth(tb)  = 0;
    }

/* Remember the original foreground and background */
    OrigForeground(tb) = Foreground(tb);
    OrigBackground(tb) = Background(tb);

/* Reverse the foreground and background if it's selected */
    if (Value(tb))
	{
	DisplayReverse(tb);
	};
}

/* ARGSUSED */
static void Arm(w, event)
Widget w;
XEvent *event;
{
    ItemWidget tb = (ItemWidget)w;

    (void)XmProcessTraversal(tb, XmTRAVERSE_CURRENT);
}

/*
 * set indicator dimensions based on the following layout scheme,
 * also update the size of the widget to reflect the new left margin
 * 
 *    margin-margin-margin-margin-margin
 *    borderhighlight-borderhighligh-borderhighlight
 *    blank-blank-blank-blank-blank-blank
 *    indicator-indicator-indicator
 *    .....
 *    indicator-indicator-indicator
 *    blank-blank-blank-blank-blank-blank
 *    borderhighlight-borderhighlight-borderhighlight
 *    margin-margin-margin-margin-margin
 **
 ** JV - we're always creating our bitmaps to be 16x16.  Force the
 ** height of the item widget to be the Max of either the text height
 ** or the bitmap height (16).
 */

static void SetIndicatorSize(w)
Widget w;
{
    ItemWidget tb = (ItemWidget) w;
    int j, theight;
    Arg	    arglist[2];
    XmFontContext context;
    XmStringCharSet charset;
    static XFontStruct	    *font;
    static XFontStruct	    *tmpfont;
    int	maxheight=0;
    static Boolean  did_it = FALSE;

    /* 
     * indicator height to be small enough to fit in the widget with
     * border hightlighting but no bigger than a character height if this
     * is a text label 
     */
    if (!did_it)
	{
	did_it = TRUE;
	maxheight=0;
	XmFontListInitFontContext(&context, Font(tb));
	while (XmFontListGetNextFont(context, &charset, &tmpfont))
	    {
	    if ((tmpfont->max_bounds.ascent + tmpfont->max_bounds.descent)
		     > maxheight)
		{
		font = tmpfont;
		}
	    XtFree (charset);
	    }
	XmFontListFreeFontContext(context);
	};
#ifndef jimsbitmapkluge
    XtHeight(tb) = TextLineHeight(font) + MarginTop(tb)
		+ MarginBottom(tb) + 2*MarginHeight(tb);
    IndHeight (tb) = XtHeight (tb);
    if (IndHeight (tb) > TextLineHeight ((Font (tb))->font))
	    IndHeight (tb) = TextLineHeight ((Font (tb))->font);
#else    
    IndHeight (tb) = 16;
    theight = TextLineHeight(font) + MarginTop(tb) +
		MarginBottom(tb) + 2*MarginHeight(tb);
    XtHeight(tb) = Max(theight, IndHeight(tb)) + 2;
#endif


/* %%% Just for now */
    IndWidth (tb) = 16;

    /* 
     * set label's left margin to make room for indicator and border 
     * highlighting
     */

    j = IndWidth (tb) + PIXELSBETWEEN;		/* JV - use constant for spacing */

    XtSetArg(arglist[0], XmNwidth, XtWidth(tb) + j -MarginLeft(tb));
    XtSetArg(arglist[1], XmNmarginLeft, j);
    XtSetValues((Widget)tb, arglist, 2);
}



/*---------------------------------------------------*/
/* removes indicator when pixmap added to item       */
/*						     */
/* called by SetValues				     */	
/*---------------------------------------------------*/

static void RemoveIndicator(tb)
ItemWidget tb;
{
    int j;

    /* 
     * reset label's left margin and width to take up space vacated
     * by indicator
     */

    j = IndWidth (tb) + PIXELSBETWEEN;		/* JV - use constant for spacing */

    XtWidth (tb) = XtWidth (tb) - j;
    
    MarginLeft (tb) = MarginLeft (tb) - j;

    IndWidth (tb)  = 0;
    IndHeight (tb) = 0;
}




/*---------------------------------------------------*/
/* set x and y coords of indicator		     */
/*---------------------------------------------------*/

static void SetIndicatorPosition(w)
Widget w;
{
    ItemWidget tb = (ItemWidget) w;
             
    IndX (tb) = MarginWidth (tb) + 2;

#ifndef jimsbitmapkluge
    IndY (tb) = MarginHeight (tb) + MarginTop (tb);
#else
    IndY (tb) = 1;
#endif
}



                           
/*---------------------------------------------------*/
/* this routine draws the item widget indicator    */
/*---------------------------------------------------*/
                           
static void DrawIndicator(w)
Widget w;
{
    GC  gc;        
    ItemWidget tb = (ItemWidget) w;

    SetIndicatorPosition(tb);

    gc = ForegroundGC(tb);

    /* first clear area containing indicator */

    XClearArea( XtDisplay(tb),
		XtWindow(tb),
		IndX (tb), 
		IndY (tb),
	      	IndWidth (tb), 
		IndHeight (tb),
		FALSE );

    /* Now draw the indicator pixmap */
    XCopyArea(	XtDisplay(tb),
		ComPixmap(tb),
		XtWindow(tb),
		gc,
		0, 0,
		IndWidth(tb),
		IndHeight(tb),
		IndX(tb),
		IndY(tb));
}       
                                                    
/*---------------------------------------------------*/
/* this routine handles an expose event by calling   */
/* the item redisplay routine indicating     */
/* that label's redisplay routine must also be called*/
/*---------------------------------------------------*/

static void HandleExpose(w, event, region)
    Widget w;
    XEvent *event;
    Region region;
{
    Redisplay(w,TRUE, event, region);
}
                                                    
/*---------------------------------------------------*/
/* Redisplay(w,call_label_redisplay)                 */
/*---------------------------------------------------*/

static void Redisplay(w,call_label_redisplay, event, region)
    Widget 	    w;
    Boolean     call_label_redisplay;
    XEvent	*event;
    Region	region;
{
    ItemWidget tb = (ItemWidget) w;

    if ((XtWindow(tb) != 0) && tb->core.visible)
    {
    	if (call_label_redisplay) 
	    (* xmLabelWidgetClass->core_class.expose) ((Widget)tb, event, region);

    	if (ComPixmap(tb))
	    DrawIndicator( tb );
    };
}

/*---------------------------------------------------*/
/* this routine will call the application with the   */
/* reasons specified by the application              */
/*---------------------------------------------------*/

static int ItemCallback(data, reason, value, event)
    ItemWidget 	data;
    unsigned int     	reason;
    unsigned int	value;
    XEvent		*event;
{

    DXmItemCallbackStruct temp;

    temp.reason = reason;
    temp.value = value;
    temp.event = event;

    switch (reason) 
      {
	case XmCR_SINGLE_SELECT:
	    XtCallCallbacks ((Widget)data, XmNsingleSelectionCallback, &temp);
	    break;
	case XmCR_DEFAULT_ACTION:
	    XtCallCallbacks ((Widget)data, XmNdefaultActionCallback, &temp);
	    break;
	case XmCR_MULTIPLE_SELECT:
	    XtCallCallbacks ((Widget)data, XmNextendedSelectionCallback, &temp);
	    break;
        case XmCR_HELP         : 
	    XtCallCallbacks ((Widget)data, XmNhelpCallback, &temp);
            break;
       }

}

/* These routines set up the item to be displayed either
 * with its normal foreground and background or reversed */

static void DisplayReverse(tb)
ItemWidget tb;
{
    Arg	arglist[2];

    XtSetArg(arglist[0], XmNforeground, OrigBackground(tb));
    XtSetArg(arglist[1], XmNbackground, OrigForeground(tb));
    XtSetValues((Widget)tb, arglist, 2);
}

static void DisplayNormal(tb)
ItemWidget tb;
{
    Arg	arglist[2];

    XtSetArg(arglist[0], XmNforeground, OrigBackground(tb));
    XtSetArg(arglist[1], XmNbackground, OrigForeground(tb));
    XtSetValues((Widget)tb, arglist, 2);
}


/*---------------------------------------------------*/
/* this routine toggles the button state and informs */
/* the caller that the widget has changed state	     */
/*---------------------------------------------------*/

static void Extend(w, event)
Widget w;
XEvent *event;
{
    ItemWidget tb = (ItemWidget) w;

/*    if ((event->x <= Width(tb) && (event->y <= Height(tb))*/
	{
	Value(tb) = !Value(tb);

	if (Value(tb))
	    {
	    DisplayReverse(tb);
	    }
	else
	    {
	    DisplayNormal(tb);
	    };

	ItemCallback(tb, XmCR_MULTIPLE_SELECT, Value(tb), event);
	};
}

static void Single(w, event)
Widget w;
XEvent *event;
{
    ItemWidget tb = (ItemWidget) w;

/*    if ((event->x <= Width(tb) && (event->y <= Height(tb))*/
	{
	if (!Value(tb))
	    {
	    Value(tb) = TRUE;
	    DisplayReverse(tb);
	    };

	ItemCallback(tb, XmCR_SINGLE_SELECT, Value(tb), event);
	};
}

static void SingleConfirm(w, event)
Widget w;
XEvent *event;
{
    ItemWidget tb = (ItemWidget) w;

/*%%%    if ((event->x <= Width(tb) && (event->y <= Height(tb))*/
	{
	if (!Value(tb))
	    {
	    Value(tb) = TRUE;
	    DisplayReverse(tb);
	    };

	ItemCallback(tb, XmCR_DEFAULT_ACTION, Value(tb), event);
	};
}

/*---------------------------------------------------*/
/* this routine will be called from the widget's     */
/* main event handler via the translation manager.   */
/*---------------------------------------------------*/

static void Help(w, event)
Widget w;
XEvent *event;
{
    ItemWidget tb = (ItemWidget) w;

/* %%% Call _XmPrimitiveHelp? */
/*    ItemCallback(tb, XmCR_HELP, Value(tb), event);*/
    _XmPrimitiveHelp(tb, event);
}

#ifdef NOTINTOOLKIT
#ifdef _NO_PROTO
void _XmPrimitiveHelp (w, event)
XmPrimitiveWidget w;
XKeyPressedEvent *event;
#else /*  _NO_PROTO */
void _XmPrimitiveHelp (XmPrimitiveWidget w, XEvent *event)
#endif /* _NO_PROTO */
{
    Widget   parent;
    XmAnyCallbackStruct cb;    

    if (w == NULL) return;

    cb.reason = XmCR_HELP;
    cb.event = (XEvent *) event;

    do
    {
        if ((XtHasCallbacks(w, XmNhelpCallback) == XtCallbackHasSome))
        {
            XtCallCallbacks (w, XmNhelpCallback, &cb);
            return;
        }
        else
            w = (XmPrimitiveWidget)XtParent(w);
    }    
    while (w != NULL);
}
#endif


/* this routine detects differences in two versions  */
/* of a widget, when a difference is found the       */
/* appropriate action is taken.			     */

/* ARGSUSED */
static Boolean SetValues(old, request, new)
    Widget   old,request,new;
{
    ItemWidget oldtb = (ItemWidget) old;
    ItemWidget newtb = (ItemWidget) new;
    Boolean redisplay = FALSE;

    if (ComPixmap(oldtb) != ComPixmap(newtb))
    {
	if (ComPixmap(newtb))
	    SetIndicatorSize(newtb);     
	else 
	    RemoveIndicator(newtb);  

     	redisplay = TRUE;
    }

    if (Font (newtb) != Font (oldtb))
    {
	SetIndicatorSize(newtb);
	redisplay = TRUE;
    }

    if (Foreground(newtb) != Foreground(oldtb))
	{
	OrigForeground(newtb) = Foreground(newtb);
	redisplay = TRUE;
	};

    if (Background(newtb) != Background(oldtb))
	{
	OrigBackground(newtb) = Background(newtb);
	redisplay = TRUE;
	};

    if (Value(newtb) != Value(oldtb))
	{
	if (Value(newtb))
	    {
	    DisplayReverse(newtb);
	    }
	else
	    {
	    DisplayNormal(newtb);
	    };
	redisplay = TRUE;
	};

    LclUpdateCallback(
	oldtb, &(SingleCallback(oldtb)), newtb, &(SingleCallback(newtb)), 
	XmNsingleSelectionCallback);

    LclUpdateCallback(
	oldtb, &(SingleConfirmCallback(oldtb)), newtb, &(SingleConfirmCallback(newtb)), 
	XmNdefaultActionCallback);

    LclUpdateCallback(
	oldtb, &(ExtendCallback(oldtb)), newtb, &(ExtendCallback(newtb)), 
	XmNextendedSelectionCallback);

    return (redisplay);
}

/*
 * SetValue on the callback is interpreted as replacing
 * all callbacks
 */

/* ARGSUSED */
static void
LclUpdateCallback (r, rstruct, s, sstruct, argname)
    Widget r;					/* the real widget*/
    char **rstruct;		/* the real callback list */
    Widget s;					/* the scratch widget*/
    char **sstruct;		/* the scratch callback list */
    char           *argname;
{
    XtCallbackList list;

    /*
     * if a new callback has been specified in the scratch widget,
     * remove and deallocate old callback and init new 
     */
    if (*rstruct != *sstruct)
    {
	list = (XtCallbackList)*sstruct;
	/*
	 *  Copy the old callback list into the new widget, since
	 *  XtRemoveCallbacks needs the "real" widget
    	 */
        *sstruct = *rstruct;
	XtRemoveAllCallbacks(s, argname);
	*sstruct = NULL;
	XtAddCallbacks(s, argname, list);
    }
}

static void Destroy(w)
Widget	w;
{
    ItemWidget	tb = (ItemWidget) w;

    XtRemoveAllCallbacks((Widget)tb, XmNsingleSelectionCallback);
    XtRemoveAllCallbacks((Widget)tb, XmNextendedSelectionCallback);
    XtRemoveAllCallbacks((Widget)tb, XmNdefaultActionCallback);
}
/*---------------------------------------------------*/
/* public entry points for UNIX			     */
/*---------------------------------------------------*/

Widget DXmItem(parent, name, x, y, label, value, callback, helpcallback)
    Widget        	parent;
    char	       *name;
    int           	x;
    int           	y;
    XmString       label;
    unsigned int	value;
    XtCallbackList 	callback;
    XtCallbackList 	helpcallback;
{
    Arg 	arglist[10];
    int 	argCount = 0;


    XtSetArg(arglist[argCount], XmNx, x);
    argCount++;
    XtSetArg(arglist[argCount], XmNy, y);
    argCount++;
    XtSetArg(arglist[argCount], XmNlabelString, label);
    argCount++;
    XtSetArg(arglist[argCount], XmNvalue, value);
    argCount++;
           

    if (callback != NULL)
    {
    	XtSetArg(arglist[argCount], XmNsingleSelectionCallback, callback);
    	argCount++;
	XtSetArg(arglist[argCount], XmNdefaultActionCallback, callback);
	argCount++;
	XtSetArg(arglist[argCount], XmNextendedSelectionCallback, callback);
	argCount++;
    }

    if (helpcallback != NULL)
    {
    	XtSetArg(arglist[argCount], XmNhelpCallback, helpcallback);
    	argCount++;
    }

    return ( XtCreateWidget( name, 
			     (WidgetClass)itemwidgetclass, 
			     parent, 
			     arglist, 
			     argCount ) );
}


/*---------------------------------------------------*/
/* XtItemGetState(w)    		     */
/*                                                   */
/* this routine returns the button state	     */
/*---------------------------------------------------*/

unsigned int DXmItemGetState(w)
Widget w;
{
    ItemWidget tb = (ItemWidget) w;

    return Value(tb);

}

/*---------------------------------------------------*/
/* XtItemSetState(w, newstate )		     */
/*                                                   */
/* this routine changes the button state - note the  */
/* application callback is activated ONLY if the     */
/* caller sets the notify bit			     */
/*---------------------------------------------------*/

void DXmItemSetState(w, newstate, notify)
    Widget        w;
    unsigned int  newstate; 
    Boolean	  notify;
{
    ItemWidget tb = (ItemWidget) w;

    if (Value(tb) != newstate)
    {
    	Value(tb) = newstate;
	if (Value(tb))
	    {
	    DisplayReverse(tb);
	    }
	else
	    {
	    DisplayNormal(tb);
	    };

    }
    if (notify)
	{
	ItemCallback(tb, XmCR_MULTIPLE_SELECT, Value(tb), 
	    XtLastTimestampProcessed(XtDisplay(tb)));
	};


}

Widget DXmItemCreate(parent, name, args, argCount)
    Widget   parent;
    char    *name;
    ArgList  args;
    int      argCount;
{

    return ( XtCreateWidget( name, 
			     (WidgetClass)itemwidgetclass, 
			     parent, 
			     args, 
			     argCount ) );

}
