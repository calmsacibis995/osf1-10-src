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
static char rcsid[] = "$RCSfile: XmIm.c,v $ $Revision: 1.1.4.4 $ $Date: 1993/10/19 18:52:15 $"
#endif
#endif
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */


#include "XmI.h"
#include <Xm/BaseClassP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/VendorSEP.h>
#include <Xm/VendorSP.h>
#include <Xm/DrawP.h>
#include <Xm/DisplayP.h>
#include "MessagesI.h"
#include <stdio.h>


#ifdef _NO_PROTO
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#else
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#endif


typedef struct _XmICStruct {
    struct _XmICStruct *next;
    Widget icw;
    XIC xic;
    Window focus_window;
    XIMStyle input_style;
    int status_width;
    int preedit_width;
    int sp_height;
    Boolean has_focus;
} XmICStruct;

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static XIM get_xim() ;
static int add_sp() ;
static int add_p() ;
static int add_fs() ;
static int add_bgpxmp() ;
static XIMStyle check_style() ;
static int ImGetGeo() ;
static void ImSetGeo() ;
static void ImGeoReq() ;
static XFontSet extract_fontset() ;
static void remove_icstruct() ;
static XmICStruct * get_icstruct() ;
static XmICStruct * get_iclist() ;
static void draw_separator() ;
static void null_proc() ;
static void ImCountVaList() ;
static ArgList ImCreateArgList() ;

#else

static XIM get_xim( 
                        Widget p) ;
static int add_sp( 
                        String name,
                        XtArgVal value,
                        ArgList *slp,
                        ArgList *plp,
                        ArgList *vlp) ;
static int add_p( 
                        String name,
                        XtArgVal value,
                        ArgList *slp,
                        ArgList *plp,
                        ArgList *vlp) ;
static int add_fs( 
                        String name,
                        XtArgVal value,
                        ArgList *slp,
                        ArgList *plp,
                        ArgList *vlp) ;
static int add_bgpxmp( 
                        String name,
                        XtArgVal value,
                        ArgList *slp,
                        ArgList *plp,
                        ArgList *vlp) ;
static XIMStyle check_style( 
			XIMStyles *styles,
                        XIMStyle preedit_style,
                        XIMStyle status_style) ;
static int ImGetGeo( 
                        Widget vw) ;
static void ImSetGeo( 
                        Widget vw) ;
static void ImGeoReq( 
                        Widget vw) ;
static XFontSet extract_fontset( 
                        XmFontList fl) ;
static void remove_icstruct( 
                        Widget w) ;
static XmICStruct * get_icstruct( 
                        Widget w) ;
static XmICStruct * get_iclist( 
                        Widget w) ;
static void draw_separator( 
                        Widget vw) ;
static void null_proc( 
                        Widget w,
                        XtPointer ptr,
                        XEvent *ev,
                        Boolean *bool) ;
static void ImCountVaList( 
                        va_list var,
                        int *total_count) ;
static ArgList ImCreateArgList( 
                        va_list var,
                        int total_count) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


#ifdef _NO_PROTO
typedef int (*XmImResLProc)() ;
#else
typedef int (*XmImResLProc)( String, XtArgVal, ArgList*, ArgList*, ArgList*) ;
#endif

typedef struct {
    String xmstring;
    String xstring;
    XrmName xrmname;
    XmImResLProc proc;
} XmImResListStruct;

static XmImResListStruct XmImResList[] = {
    {XmNbackground, XNBackground, NULLQUARK, add_sp},
    {XmNforeground, XNForeground, NULLQUARK, add_sp},
    {XmNbackgroundPixmap, XNBackgroundPixmap, NULLQUARK, add_bgpxmp},
    {XmNspotLocation, XNSpotLocation, NULLQUARK, add_p},
    {XmNfontList, XNFontSet, NULLQUARK, add_fs},
    {XmNlineSpace, XNLineSpace, NULLQUARK, add_sp}
};

typedef struct {
    XIM xim;
    XIMStyles *styles;
} _XIMStruct;

#define MAXARGS 10
static Arg xic_vlist[MAXARGS];
static Arg status_vlist[MAXARGS];
static Arg preedit_vlist[MAXARGS];

#define OVERTHESPOT "overthespot"
#define OFFTHESPOT "offthespot"
#define ROOT "root"
#define MAXSTYLES 3
#define SEPARATOR_HEIGHT 2

#define GEO_CHG 0x1
#define BG_CHG 0x2

typedef struct {
    Widget im_widget;
    XmICStruct *iclist;
    Widget current_widget;
} XmImInfo;

#define MSG1	_XmMsgXmIm_0000

void 
#ifdef _NO_PROTO
XmImRegister( w, reserved )
        Widget w ;
	unsigned int reserved ;
#else
XmImRegister(
        Widget w,
	unsigned int reserved )
#endif /* _NO_PROTO */
{
    Widget p;
    register XmICStruct *icp;
    register XmICStruct *bcp = NULL;
    XmICStruct *curic;
    XIMStyle input_style = 0;
    char tmp[BUFSIZ];
    char *cp, *tp, *cpend;
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    XmImInfo *im_info;

    XmDisplay	xmDisplay;
    _XIMStruct *xim_struct;
    register XIMStyles *styles;

    p = XtParent(w);

    while (!XtIsShell(p))
	p = XtParent(p);

    extData = _XmGetWidgetExtData((Widget)p, XmSHELL_EXTENSION);

    /* check extension data since app could be attempting to create
     * a text widget as child of menu shell. This is illegal, and will
     * be detected later, but check here so we don't core dump.
     */
    if (extData == NULL)
	return;

    ve = (XmVendorShellExtObject) extData->widget;

    if (get_xim(p) == NULL)
	return;

    if (ve->vendor.im_info == NULL)
    {
	if ((im_info = (XmImInfo *)XtMalloc(sizeof(XmImInfo))) == NULL)
		return;

	im_info->im_widget = NULL;
	im_info->iclist = NULL;
	im_info->current_widget = NULL;
	ve->vendor.im_info = (XtPointer)im_info;
    }
    else
	im_info = (XmImInfo *)ve->vendor.im_info;

    if (im_info->iclist == NULL)
    {
	im_info->iclist = (XmICStruct *)XtMalloc(sizeof(XmICStruct));
	curic = im_info->iclist;
    }
    else
    {
	icp = im_info->iclist;
	while (icp != NULL)
	{
	    if (icp->icw == w)		/* im widget already registered */
		return;
	    bcp = icp;
	    icp = icp->next;
	}
	bcp->next = (XmICStruct *)XtMalloc(sizeof(XmICStruct));
	curic = bcp->next;
    }

    if (curic == NULL)			/* malloc failed */
	return;

    curic->icw = w;
    curic->xic = 0;
    curic->focus_window = 0;
    curic->status_width = 0;
    curic->preedit_width = 0;
    curic->sp_height = 0;
    curic->has_focus = False;
    curic->next = NULL;

    /* Now determine the input style to be used for this XIC */

    xmDisplay = (XmDisplay) XmGetXmDisplay(XtDisplay(p));
    xim_struct = (_XIMStruct *)xmDisplay->display.xmim_info;
    styles = xim_struct->styles;

    input_style = 0;
    XtVaGetValues(p,XmNpreeditType,&cp,NULL);
    if (cp != NULL)
    {
	/* parse for the successive commas */

	cp = strcpy(tmp,cp);
	cpend = &tmp[strlen(tmp)];
	while(cp < cpend)
	{
	    tp = strchr(cp,',');
	    if (tp)
		*tp = 0;
	    else
		tp = cpend;

	    if (_XmStringsAreEqual(cp,OVERTHESPOT))
	    {
		if ((input_style = check_style(styles, XIMPreeditPosition,
			XIMStatusArea|XIMStatusNothing|XIMStatusNone)) != 0)
		    break;
	    }
	    else if (_XmStringsAreEqual(cp,OFFTHESPOT))
	    {
		if ((input_style = check_style(styles, XIMPreeditArea,
			XIMStatusArea|XIMStatusNothing|XIMStatusNone)) != 0)
		    break;
	    }
	    else if (_XmStringsAreEqual(cp,ROOT))
	    {
		if ((input_style = check_style(styles, XIMPreeditNothing,
			XIMStatusNothing|XIMStatusNone)) != 0)
		    break;
	    }
	    cp = tp+1;
	}
    }
    if (input_style == 0)
    {
	if ((input_style = check_style(styles, XIMPreeditNone, 
		XIMStatusNone)) == 0)
	{
	    /* no input style supported - will use XLookupString.
	     * remove from list
	     */ 
	    if (curic == im_info->iclist)
		im_info->iclist = NULL;
	    else
		bcp->next = NULL;

	    XtFree((char *)curic);
	    return;
	}
    }
    curic->input_style = input_style;

    /* We need to create this widget whenever there is a non-simple
     * input method in order to stop the intrinsics from calling
     * XMapSubwindows, thereby improperly mapping input method
     * windows which have been made children of the client or
     * focus windows.
     */

    if (input_style & (XIMStatusArea | XIMPreeditArea | XIMPreeditPosition))
	im_info->im_widget = XtVaCreateWidget("xmim_wrapper", coreWidgetClass,
				 p, XmNwidth, 10, XmNheight, 10, NULL);
}


void 
#ifdef _NO_PROTO
XmImUnregister( w )
        Widget w ;
#else
XmImUnregister(
        Widget w )
#endif /* _NO_PROTO */
{
    register XmICStruct *icp;

    if ((icp = get_icstruct(w)) == NULL)
	return;

    if (icp->xic)
	XDestroyIC(icp->xic);
    remove_icstruct(w);
}


#ifdef HP_MOTIF
void 
#ifdef _NO_PROTO
_XHP_XmImCloseIM( w )
        Widget w ;
#else
_XHP_XmImCloseIM(
        Widget w )
#endif /* _NO_PROTO */
{
    XmDisplay	xmDisplay;
    _XIMStruct *xim_struct;

    xmDisplay = (XmDisplay) XmGetXmDisplay(XtDisplay(w));
    xim_struct = (_XIMStruct *)xmDisplay->display.xmim_info;
    XFree(xim_struct->styles);
    XCloseIM(xim_struct->xim);
    XtFree((XtPointer)xim_struct);
    xmDisplay->display.xmim_info = NULL;
}
#endif /* HP_MOTIF */


void 
#ifdef _NO_PROTO
XmImSetFocusValues( w, args, num_args )
        Widget w ;
        ArgList args ;
        Cardinal num_args ;
#else
XmImSetFocusValues(
        Widget w,
        ArgList args,
        Cardinal num_args )
#endif /* _NO_PROTO */
{
    register XmICStruct *icp;
    Widget p;
    Pixel bg;
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    XmImInfo *im_info;

    XmImSetValues(w, args, num_args);

    p = w;
    while (!XtIsShell(p))
	p = XtParent(p);

    if ((icp = get_icstruct(w)) == NULL)
	return;

    if (icp->focus_window == (Window) NULL)
    {
	XSetICValues(icp->xic, XNFocusWindow, XtWindow(w), NULL);
	icp->focus_window = XtWindow(w);
    }
    XSetICFocus(icp->xic);
    icp->has_focus = True;

    extData = _XmGetWidgetExtData((Widget)p, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;

    if (ve->vendor.im_height)
    {
	im_info = (XmImInfo *)ve->vendor.im_info;
	im_info->current_widget = icp->icw;
	XtVaGetValues(w, XmNbackground, &bg, NULL);
	XtVaSetValues(p, XmNbackground, bg, NULL);
	draw_separator(p);
    }
}


void 
#ifdef _NO_PROTO
XmImSetValues( w, args, num_args )
        Widget w ;
        ArgList args ;
        Cardinal num_args ;
#else
XmImSetValues(
        Widget w,
        ArgList args,
        Cardinal num_args )
#endif /* _NO_PROTO */
{
    register XmICStruct *icp;
    XmImResListStruct *rlp;
    register int i, j;
    register ArgList argp = args;
    ArgList tslp = &status_vlist[0];
    ArgList tplp = &preedit_vlist[0];
    ArgList tvlp = &xic_vlist[0];
    XrmName name;
    Widget p;
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    XmImInfo *im_info;
    int flags = 0;
    Pixel bg;
    char *ret;
    unsigned long mask;
    Boolean unrecognized = False;

    p = w;
    while (!XtIsShell(p))
	p = XtParent(p);

    extData = _XmGetWidgetExtData((Widget)p, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;

    if ((icp = get_icstruct(w)) == NULL)
	return;

    im_info = (XmImInfo *)ve->vendor.im_info;
    if (!XtIsRealized(p))
    {
	/* if vendor widget not realized, then the current info
	 * is that for the last widget to set values.
	 */

	im_info->current_widget = icp->icw;
    }

    for (i = num_args; i > 0; i--, argp++)
    {
	name = XrmStringToName(argp->name);

	for (rlp = XmImResList, j = sizeof(XmImResList)/
			sizeof(XmImResListStruct); j != 0; j--, rlp++)
	{
	    if (rlp->xrmname == name)
	    {
		flags |= (*rlp->proc) (rlp->xstring, argp->value, &tslp, 
								&tplp, &tvlp);
		break;
	    }
	}
	if (j == 0)		/* simply pass unrecognized values along */
	{
	    tvlp->name = argp->name;
	    tvlp->value = argp->value;
	    tvlp++;
	    unrecognized = True;
	}
    }

    tslp->name = NULL;
    tslp->value = (XtArgVal) NULL;
    tplp->name = NULL;
    tplp->value = (XtArgVal) NULL;

    tvlp->name = XNStatusAttributes;
    tvlp->value = (XtArgVal)&status_vlist[0];
    tvlp++;
    tvlp->name = XNPreeditAttributes;
    tvlp->value = (XtArgVal)&preedit_vlist[0];
    tvlp++;
    tvlp->name = NULL;
    tvlp->value = (XtArgVal) NULL;

    /* we do not create the IC until the initial data is ready to be passed */

    if ((get_xim(p) != NULL) && (icp->xic == NULL))
    {
	if (XtIsRealized(p))
	{
	    XSync(XtDisplay(p), False);
	    tvlp->name = XNClientWindow;
	    tvlp->value = (XtArgVal)XtWindow(p);
	    tvlp++;
	}
	if (icp->focus_window)
	{
	    tvlp->name = XNFocusWindow;
	    tvlp->value = (XtArgVal)icp->focus_window;
	    tvlp++;
	}
	tvlp->name = XNInputStyle;
	tvlp->value = (XtArgVal)icp->input_style;
	tvlp++;
	tvlp->name = NULL;
	tvlp->value = (XtArgVal) NULL;
	icp->xic = XCreateIC(get_xim(p), XNVaNestedList, &xic_vlist[0], NULL);
	if (icp->xic == NULL)
	{
	    remove_icstruct(w);
	    return;
	}
	XGetICValues(icp->xic, XNFilterEvents, &mask, NULL);
	if (mask)
	{
	    XtAddEventHandler(p, (EventMask)mask, False, null_proc, NULL);
	}
	if (XtIsRealized(p))
	{
	    ImGeoReq(p);
	    im_info->current_widget = icp->icw;
	}
    }
    else
    {
	ret = XSetICValues(icp->xic, XNVaNestedList, &xic_vlist[0], NULL);
	if (ret != NULL && !unrecognized)
	{
	    XVaNestedList slist, plist;
	    unsigned long status_bg, status_fg;
	    unsigned long preedit_bg, preedit_fg;

	    /* We do this in case an input method does not support
	     * change of some value, but does allow it to be set on
	     * create.  If however the value is not one of the 
	     * standard values, this im may not support it so we
	     * should ignore it.
	     */

	    XGetICValues(icp->xic, 
			XNStatusAttributes, slist = XVaCreateNestedList(0, 
					XNBackground, &status_bg,
					XNForeground, &status_fg, NULL),
			XNPreeditAttributes, plist = XVaCreateNestedList(0, 
					XNBackground, &preedit_bg,
					XNForeground, &preedit_fg, NULL),
			NULL);
	    XFree(slist);
	    XFree(plist);
	    XDestroyIC(icp->xic);

	    tslp->name = XNBackground;
	    tslp->value = (XtArgVal)status_bg;
	    tslp++;
	    tslp->name = XNForeground;
	    tslp->value = (XtArgVal)status_fg;
	    tslp++;
	    tslp->name = NULL;

	    tplp->name = XNBackground;
	    tplp->value = (XtArgVal)preedit_bg;
	    tplp++;
	    tplp->name = XNForeground;
	    tplp->value = (XtArgVal)preedit_fg;
	    tplp++;
	    tplp->name = NULL;

	    if (XtIsRealized(p))
	    {
		XSync(XtDisplay(p), False);
		tvlp->name = XNClientWindow;
		tvlp->value = (XtArgVal)XtWindow(p);
		tvlp++;
	    }
	    if (icp->focus_window)
	    {
		tvlp->name = XNFocusWindow;
		tvlp->value = (XtArgVal)icp->focus_window;
		tvlp++;
	    }
	    tvlp->name = XNInputStyle;
	    tvlp->value = (XtArgVal)icp->input_style;
	    tvlp++;
	    tvlp->name = NULL;
	    tvlp->value = (XtArgVal) NULL;
	    icp->xic = XCreateIC(get_xim(p),XNVaNestedList,&xic_vlist[0],NULL);
	    if (icp->xic == NULL)
	    {
		remove_icstruct(w);
		return;
	    }
	    ImGeoReq(p);
    	    if (icp->has_focus == True)
		XSetICFocus(icp->xic);
	    return;
	}
	if (flags & GEO_CHG)
	{
	    ImGeoReq(p);
    	    if (icp->has_focus == True)
		XSetICFocus(icp->xic);
	}
    }

    /* Since we do not know whether a set values may have been done
     * on top shadow or bottom shadow (used for the separator), we
     * will redraw the separator in order to keep the visuals in sync
     * with the current text widget. Also repaint background if needed.
     */

    if (im_info->current_widget == icp->icw &&
	flags & BG_CHG)
    {
	XtVaGetValues(w, XmNbackground, &bg, NULL);
	XtVaSetValues(p, XmNbackground, bg, NULL);
    }
}


void 
#ifdef _NO_PROTO
XmImUnsetFocus( w )
        Widget w ;
#else
XmImUnsetFocus(
        Widget w )
#endif /* _NO_PROTO */
{
    register XmICStruct *icp;

    if ((icp = get_icstruct(w)) == NULL)
	return;

    if (icp->xic)
	XUnsetICFocus(icp->xic);
    icp->has_focus = False;
}


XIM 
#ifdef _NO_PROTO
XmImGetXIM(w)
	Widget w;
#else
XmImGetXIM( 
	Widget w)
#endif /* _NO_PROTO */
{
    Widget p;

    p = w;
    while (!XtIsShell(p))
	p = XtParent(p);

    return get_xim(p);
}


int 
#ifdef _NO_PROTO
XmImMbLookupString( w, event, buf, nbytes, keysym, status )
        Widget w ;
        XKeyPressedEvent *event ;
        char *buf ;
        int nbytes ;
        KeySym *keysym ;
        int *status ;
#else
XmImMbLookupString(
        Widget w,
        XKeyPressedEvent *event,
        char *buf,
        int nbytes,
        KeySym *keysym,
        int *status )
#endif /* _NO_PROTO */
{
    register XmICStruct *icp;
#ifdef DEC_MOTIF_BUG_FIX
    static XComposeStatus compose_status = {NULL, 0};
#endif

    if ((icp = get_icstruct(w)) == NULL  ||  icp->xic == NULL)
    {
	if (status)
	    *status = XLookupBoth;
#ifdef DEC_MOTIF_BUG_FIX
	/*
	** Handle compose key processing
	*/
	return XLookupString(event, buf, nbytes, keysym, &compose_status);
#else
	return XLookupString(event, buf, nbytes, keysym, 0);
#endif
    }

    return XmbLookupString( icp->xic, event, buf, nbytes, keysym, status );
}

/* Private Functions */

void 
#ifdef _NO_PROTO
_XmImChangeManaged( vw )
        Widget vw ;
#else
_XmImChangeManaged(
        Widget vw )
#endif /* _NO_PROTO */
{
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    register int height;

    extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;

    height = ImGetGeo(vw);
    if (!ve->vendor.im_vs_height_set)
	vw->core.height += height; 
}

void
#ifdef _NO_PROTO
_XmImRealize( vw )
        Widget vw ;
#else
_XmImRealize(
        Widget vw )
#endif /* _NO_PROTO */
{
    XmICStruct *icp;
    Pixel bg;
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    XmImInfo *im_info;

    extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;
    im_info = (XmImInfo *)ve->vendor.im_info;

    if ( (icp = get_iclist(vw)) == NULL )
	return;

    /* We need to synchronize here to make sure the server has created
     * the client window before the input server attempts to reparent
     * any windows to it
     */

    XSync(XtDisplay(vw), False);
    for (; icp != NULL; icp = icp->next)
    {
	XSetICValues(icp->xic, XNClientWindow, 
			    XtWindow(vw), NULL);
    }

    ImSetGeo(vw);

    /* For some reason we need to wait till now before we set the 
     * initial background pixmap.
     */

    if (ve->vendor.im_height)
    {
	XtVaGetValues(im_info->current_widget, XmNbackground, &bg, NULL);
	XtVaSetValues(vw, XmNbackground, bg, NULL);
    }
}

void
#ifdef _NO_PROTO
_XmImResize( vw )
        Widget vw ;
#else
_XmImResize(
        Widget vw )
#endif /* _NO_PROTO */
{
    ImGetGeo(vw);
    ImSetGeo(vw);
}

void
#ifdef _NO_PROTO
_XmImRedisplay( vw )
        Widget vw ;
#else
_XmImRedisplay(
        Widget vw )
#endif /* _NO_PROTO */
{
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;

    if ((extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION)) == NULL)
	return;

    ve = (XmVendorShellExtObject) extData->widget;

    if (ve->vendor.im_height == 0)
	return;

    draw_separator(vw);

}

/* Begin static functions */


static XIM 
#ifdef _NO_PROTO
get_xim(p)
	Widget p;
#else
get_xim( 
	Widget p)
#endif /* _NO_PROTO */
{
    XmDisplay	xmDisplay;
    char tmp[BUFSIZ];
    char *cp;
    XmImResListStruct *rlp;
    register int i;
    _XIMStruct *xim_struct;
    String name, w_class;

    xmDisplay = (XmDisplay) XmGetXmDisplay(XtDisplay(p));
    xim_struct = (_XIMStruct *)xmDisplay->display.xmim_info;

    if (xim_struct == NULL)
    {
	xim_struct = (_XIMStruct *)XtMalloc(sizeof (_XIMStruct));

	if (xim_struct == NULL)
	    return NULL;

	xmDisplay->display.xmim_info = (XtPointer)xim_struct;

	XtVaGetValues(p,XmNinputMethod,&cp,NULL);
	if (cp != NULL)
	{
	    strcpy(tmp,"@im=");
	    strcat(tmp,cp);
	    XSetLocaleModifiers(tmp);
	}

	XtGetApplicationNameAndClass(XtDisplay(p), &name, &w_class);

	xim_struct->xim = XOpenIM(XtDisplay(p), XtDatabase(XtDisplay(p)), 
								name, w_class);
	xim_struct->styles = NULL;
	if (xim_struct->xim == NULL)
	{
#ifdef XOPENIM_WARNING
	    _XmWarning ((Widget)p, MSG1);
#endif
	    return NULL;
	}

	if (XGetIMValues(xim_struct->xim, 
			XNQueryInputStyle, &xim_struct->styles, NULL) != NULL)
	{
	    XCloseIM(xim_struct->xim);
	    xim_struct->xim = NULL;
	    _XmWarning ((Widget)p, MSG1);
	    return NULL;
	}

	/* initialize the list of xrm names */

	for (rlp = XmImResList, i = sizeof(XmImResList)/
			sizeof(XmImResListStruct); i != 0; i--, rlp++)
	{
	    rlp->xrmname = XrmStringToName(rlp->xmstring);
	}

    }
    return xim_struct->xim;
}

static int 
#ifdef _NO_PROTO
add_sp( name, value, slp, plp, vlp )
        String name ;
        XtArgVal value ;
        ArgList *slp ;
        ArgList *plp ;
        ArgList *vlp ;
#else
add_sp(
        String name,
        XtArgVal value,
        ArgList *slp,
        ArgList *plp,
        ArgList *vlp )
#endif /* _NO_PROTO */
{
    register ArgList tp;

    tp = *slp;
    tp->value = value;
    tp->name = name;
    tp++;
    *slp = tp;

    tp = *plp;
    tp->value = value;
    tp->name = name;
    tp++;
    *plp = tp;

    return BG_CHG;
}

static int 
#ifdef _NO_PROTO
add_p( name, value, slp, plp, vlp )
        String name ;
        XtArgVal value ;
        ArgList *slp ;
        ArgList *plp ;
        ArgList *vlp ;
#else
add_p(
        String name,
        XtArgVal value,
        ArgList *slp,
        ArgList *plp,
        ArgList *vlp )
#endif /* _NO_PROTO */
{
    register ArgList tp;

    tp = *plp;
    tp->value = value;
    tp->name = name;
    tp++;
    *plp = tp;

    return 0;
}

static int 
#ifdef _NO_PROTO
add_fs( name, value, slp, plp, vlp )
        String name ;
        XtArgVal value ;
        ArgList *slp ;
        ArgList *plp ;
        ArgList *vlp ;
#else
add_fs(
        String name,
        XtArgVal value,
        ArgList *slp,
        ArgList *plp,
        ArgList *vlp )
#endif /* _NO_PROTO */
{
    register ArgList tp;
    XFontSet fs;

    if ( (fs = extract_fontset((XmFontList)value)) == NULL)
	return 0;

    tp = *slp;
    tp->value = (XtArgVal)fs;
    tp->name = name;
    tp++;
    *slp = tp;

    tp = *plp;
    tp->value = (XtArgVal)fs;
    tp->name = name;
    tp++;
    *plp = tp;

    return GEO_CHG;
}

static int 
#ifdef _NO_PROTO
add_bgpxmp( name, value, slp, plp, vlp )
        String name ;
        XtArgVal value ;
        ArgList *slp ;
        ArgList *plp ;
        ArgList *vlp ;
#else
add_bgpxmp(
        String name,
        XtArgVal value,
        ArgList *slp,
        ArgList *plp,
        ArgList *vlp )
#endif /* _NO_PROTO */
{
    if ( (Pixmap)value == XtUnspecifiedPixmap )
	return 0;

    return add_sp( name, value, slp, plp, vlp );
}

static XIMStyle 
#ifdef _NO_PROTO
check_style( styles, preedit_style, status_style )
	XIMStyles *styles;
        XIMStyle preedit_style ;
        XIMStyle status_style ;
#else
check_style(
	XIMStyles *styles,
        XIMStyle preedit_style,
        XIMStyle status_style )
#endif /* _NO_PROTO */
{
    register int i;

    for (i=0; i < styles->count_styles; i++)
    {
	if ((styles->supported_styles[i] & preedit_style) &&
	    (styles->supported_styles[i] & status_style))
	    return styles->supported_styles[i];
    }
    return 0;
}


static int 
#ifdef _NO_PROTO
ImGetGeo( vw )
        Widget vw ;
#else
ImGetGeo(
        Widget vw )
#endif /* _NO_PROTO */
{
    XmICStruct *icp;
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    int height = 0;
    XRectangle rect;
    XRectangle *rp;

    if ( (icp = get_iclist(vw)) == NULL )
	return 0;

    extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;

    xic_vlist[0].name = XNAreaNeeded;
    xic_vlist[1].name = NULL;

    for (; icp != NULL; icp = icp->next)
    {
	if (icp->input_style & XIMStatusArea)
	{
	    xic_vlist[0].value = (XtArgVal)&rect;
	    rect.width = vw->core.width;
	    rect.height = 0;

	    XSetICValues(icp->xic, 
		XNStatusAttributes, &xic_vlist[0], 
		NULL);

	    xic_vlist[0].value = (XtArgVal)&rp;
	    XGetICValues(icp->xic, 
		XNStatusAttributes, &xic_vlist[0], 
		NULL);

	    if (rp->height > height)
		height = rp->height;
	    
	    icp->status_width = MIN(rp->width, vw->core.width);
	    icp->sp_height = rp->height;
	    XFree(rp);
	}
	if (icp->input_style & XIMPreeditArea)
	{
	    xic_vlist[0].value = (XtArgVal)&rect;
	    rect.width = vw->core.width;
	    rect.height = 0;

	    XSetICValues(icp->xic, 
		XNPreeditAttributes, &xic_vlist[0], 
		NULL);

	    xic_vlist[0].value = (XtArgVal)&rp;
	    XGetICValues(icp->xic, 
		XNPreeditAttributes, &xic_vlist[0], 
		NULL);

	    if (rp->height > height)
		height = rp->height;

	    icp->preedit_width = MIN(rp->width,
			    vw->core.width - icp->status_width);
	    if (icp->sp_height < rp->height)
		icp->sp_height = rp->height;
	    XFree(rp);
	}
    }

    if (height)
	height += SEPARATOR_HEIGHT;

    ve->vendor.im_height = height;
    return height;
}

static void 
#ifdef _NO_PROTO
ImSetGeo( vw )
        Widget vw ;
#else
ImSetGeo(
        Widget vw )
#endif /* _NO_PROTO */
{
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    ArgList tslp = &status_vlist[0];
    ArgList tplp = &preedit_vlist[0];
    register XmICStruct *icp;
    XRectangle rect_status;
    XRectangle rect_preedit;

    if ( (icp = get_iclist(vw)) == NULL)
	return;

    extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;

    if (ve->vendor.im_height == 0)
	return;

    for (; icp != NULL; icp = icp->next)
    {
	if (icp->input_style & XIMStatusArea)
	{
	    rect_status.x = 0;
	    rect_status.y = vw->core.height - icp->sp_height;
	    rect_status.width = icp->status_width;
	    rect_status.height = icp->sp_height;

	    tslp->name = XNArea;
	    tslp->value = (XtArgVal)&rect_status;
	    tslp++;
	}
	tslp->name = NULL;

	if (icp->input_style & XIMPreeditArea)
	{
	    rect_preedit.x = icp->status_width;
	    rect_preedit.y = vw->core.height - icp->sp_height;
	    rect_preedit.width = icp->preedit_width;
	    rect_preedit.height = icp->sp_height;

	    tplp->name = XNArea;
	    tplp->value = (XtArgVal)&rect_preedit;
	    tplp++;
	}
	tplp->name = NULL;

	XSetICValues(icp->xic, XNStatusAttributes, &status_vlist[0], 
			       XNPreeditAttributes, &preedit_vlist[0], 
			       NULL);
    }
}


static void 
#ifdef _NO_PROTO
ImGeoReq( vw )
        Widget vw ;
#else
ImGeoReq(
        Widget vw )
#endif /* _NO_PROTO */
{
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    XtWidgetGeometry 	my_request;
    int old_height;
    int delta_height;
    ShellWidget 	shell = (ShellWidget)(vw);

    if (!(shell->shell.allow_shell_resize) && XtIsRealized(vw))
	return;

    extData = _XmGetWidgetExtData(vw, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;

    old_height = ve->vendor.im_height;
    ImGetGeo(vw);
    if ((delta_height = ve->vendor.im_height - old_height) != 0)
    {
	my_request.height = vw->core.height + delta_height;
	my_request.request_mode = CWHeight;
	XtMakeGeometryRequest(vw, &my_request, NULL);
    }
    ImSetGeo(vw);
}


static XFontSet 
#ifdef _NO_PROTO
extract_fontset( fl )
        XmFontList fl ;
#else
extract_fontset(
        XmFontList fl )
#endif /* _NO_PROTO */
{
    XmFontContext context;
    XmFontListEntry next_entry;
    XmFontType type_return;
    XtPointer tmp_font;
    XFontSet first_fs = NULL;
    char *font_tag;

    if (!XmFontListInitFontContext(&context, fl))
       return NULL;

    do {
	next_entry = XmFontListNextEntry(context);
	if (next_entry)
	{
	    tmp_font = XmFontListEntryGetFont(next_entry, &type_return);
	    if (type_return == XmFONT_IS_FONTSET)
	    {
		font_tag = XmFontListEntryGetTag(next_entry);
		if (!strcmp(font_tag, XmFONTLIST_DEFAULT_TAG))
		{
		    XmFontListFreeFontContext(context);
		    return (XFontSet)tmp_font;
		}
		if (first_fs == NULL)
		    first_fs = (XFontSet)tmp_font;
	    }
#ifdef HP_MOTIF
	    else
	    {
		if (first_fs == NULL)
		    first_fs = (XFontSet)_XGetFontAssociateFontSet(
					    ((XFontStruct *)tmp_font)->fid);
	    }
#endif
	}
    } while (next_entry);

    XmFontListFreeFontContext(context);
    return first_fs;
}

static void
#ifdef _NO_PROTO
remove_icstruct( w )
	Widget w;
#else
remove_icstruct(
	Widget w )
#endif /* _NO_PROTO */
{
    register XmICStruct *icp, *bicp;
    Widget p;
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    XmImInfo *im_info;

    p = w;
    while (!XtIsShell(p))
	p = XtParent(p);

    extData = _XmGetWidgetExtData((Widget)p, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;
    if ((im_info = (XmImInfo *)ve->vendor.im_info) == NULL)
	return;
    icp = im_info->iclist;
    if (icp == NULL)
	return;

    bicp = NULL;
    while (icp != NULL && icp->icw != w) 
    {
	bicp = icp;
	icp = icp->next;
    }

    if (bicp == NULL)			/* removing list head */
    {
	im_info->iclist = icp->next;
    }
    else
    {
	bicp->next = icp->next;
    }

    XtFree((char *)icp);
}

static XmICStruct * 
#ifdef _NO_PROTO
get_icstruct( w )
        Widget w ;
#else
get_icstruct(
        Widget w )
#endif /* _NO_PROTO */
{
    register XmICStruct *icp;
    Widget p;
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    XmImInfo *im_info;

    p = w;
    while (!XtIsShell(p))
	p = XtParent(p);

    extData = _XmGetWidgetExtData((Widget)p, XmSHELL_EXTENSION);
    if (extData == NULL)
	return NULL;

    ve = (XmVendorShellExtObject) extData->widget;
    if ((im_info = (XmImInfo *)ve->vendor.im_info) == NULL)
	return NULL;

    icp = im_info->iclist;
    while (icp != NULL && icp->icw != w) 
	icp = icp->next;

    return icp;
}

static XmICStruct * 
#ifdef _NO_PROTO
get_iclist( w )
        Widget w ;
#else
get_iclist(
        Widget w )
#endif /* _NO_PROTO */
{
    Widget p;
    XmVendorShellExtObject	ve;
    XmWidgetExtData	extData;
    XmImInfo *im_info;

    p = w;
    while (!XtIsShell(p))
	p = XtParent(p);

    extData = _XmGetWidgetExtData((Widget)p, XmSHELL_EXTENSION);
    if (extData == NULL)
	return NULL;

    ve = (XmVendorShellExtObject) extData->widget;
    if ((im_info = (XmImInfo *)ve->vendor.im_info) == NULL)
	return NULL;
    else
	return im_info->iclist;
}


static void 
#ifdef _NO_PROTO
draw_separator( vw )
        Widget vw ;
#else
draw_separator(
        Widget vw )
#endif /* _NO_PROTO */
{
    XmPrimitiveWidget pw;
    XmVendorShellExtObject ve;
    XmWidgetExtData	extData;
    XmImInfo *im_info;

    extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION);
    ve = (XmVendorShellExtObject) extData->widget;
    if ((im_info = (XmImInfo *)ve->vendor.im_info) == NULL)
	return; 
    pw = (XmPrimitiveWidget)im_info->current_widget;
    if (!XmIsPrimitive(pw))
	return;

    _XmDrawSeparator(XtDisplay(vw), XtWindow(vw),
		  pw->primitive.top_shadow_GC,
		  pw->primitive.bottom_shadow_GC,
		  0,
		  0,
		  vw->core.height - ve->vendor.im_height,
		  vw->core.width,
		  SEPARATOR_HEIGHT,
		  SEPARATOR_HEIGHT,
                  0, 			/* separator.margin */
                  XmHORIZONTAL,		/* separator.orientation */
                  XmSHADOW_ETCHED_IN); /* separator.separator_type */
}

static void 
#ifdef _NO_PROTO
null_proc( w, ptr, ev, bool )
        Widget w ;
        XtPointer ptr ;
        XEvent *ev ;
        Boolean *bool ;
#else
null_proc(
        Widget w,
        XtPointer ptr,
        XEvent *ev,
        Boolean *bool )
#endif /* _NO_PROTO */
{
    /* This function does nothing.  It is only there to allow the
     * event mask required by the input method to be added to
     * the client window.
     */
}

/* The following section contains the varargs functions */


void 
#ifdef _NO_PROTO
XmImVaSetFocusValues( w, va_alist )
        Widget w ;
        va_dcl
#else
XmImVaSetFocusValues(
        Widget w,
        ... )
#endif /* _NO_PROTO */
{
    va_list	var;
    int	    	total_count;
    ArgList     args;

    Va_start(var,w);
    ImCountVaList(var, &total_count);
    va_end(var);

    Va_start(var,w);
    args  = ImCreateArgList(var, total_count);
    va_end(var);

    XmImSetFocusValues(w, args, total_count);
    XtFree((char *)args);
}

void
#ifdef _NO_PROTO
XmImVaSetValues( w, va_alist )
        Widget w ;
        va_dcl
#else
XmImVaSetValues(
        Widget w,
        ... )
#endif /* _NO_PROTO */
{
    va_list	var;
    int	    	total_count;
    ArgList     args;

    Va_start(var,w);
    ImCountVaList(var, &total_count);
    va_end(var);

    Va_start(var,w);
    args  = ImCreateArgList(var, total_count);
    va_end(var);

    XmImSetValues(w, args, total_count);
    XtFree((char *)args);
}


static void 
#ifdef _NO_PROTO
ImCountVaList( var, total_count )
        va_list var ;
        int *total_count ;
#else
ImCountVaList(
        va_list var,
        int *total_count )
#endif /* _NO_PROTO */
{
    String          attr;
    
    *total_count = 0;
 
    for(attr = va_arg(var, String) ; attr != NULL; attr = va_arg(var, String)) 
    {
	(void) va_arg(var, XtArgVal);
	++(*total_count);
    }
}

static ArgList
#ifdef _NO_PROTO
ImCreateArgList(var, total_count)
        va_list var ;
        int total_count ;
#else
ImCreateArgList(
        va_list var,
        int total_count )
#endif /* _NO_PROTO */
{
    ArgList args = (ArgList)XtMalloc(total_count * sizeof(Arg));
    register int i;

    for (i = 0; i < total_count; i++)
    {
	args[i].name = va_arg(var,String);
	args[i].value = va_arg(var,XtArgVal);
    }

    return args;
}
