/* DWC$UI_ERRORBOXES "V3-022" */
#ifndef lint
static char rcsid[] = "$Id$";
#endif /* lint */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Marios Cleovoulou, November-1987
**
**  ABSTRACT:
**
**	Generic error message box routines.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3.0-022 Paul Ferwerda					23-Oct-1990
**		If NeedNew dialog box had help popped up, Calendar would ACCVIO
**		if you exited the dialog box since help wasn't being killed
**		properly. Add HELPDestoryForModal to  ERRORDestroyErrorbox.
** V3.0-021 Paul Ferwerda					17-Oct-1990
**		Put cancel button back into message boxes.
** V3.0-020 Paul Ferwerda					05-Oct-1990
**		Added ERRORDisplayTextCall which will display text and do a
**		callback if we need it to.
** V3.0-019 Paul Ferwerda					28-Sep-1990
**		Toolkit now positions some message boxes, tweaked code in
**		create_error_box which did positioning. Added code to unmanage
**		cancel button on errors, added remove_cursor routine for error
**		displays without a callback so that the the inactive cursor gets
**		undone. Try to make sure that the msg boxes are stacked on top.
**		
**  V3-018  Paul Ferwerda					05-Feb-1990
**		Tweaked include files, port to Motif, got rid of LIB$. Changed
**		VoidProc to XmVoidProc, DwtLatin1String(text) to
**		XmStringCreateSimple(text). Switch
**		DwcSetArg for XtSetArg. Changed XtStringFree to XmStringFree.
**	V2-017	Marios Cleovoulou				27-May-1989
**		Use inactive cursor
**	V2-016	Per Hamnqvist					19-May-1989
**		Extract real text associated with errno in strerror rather
**		than always returning NULL.
**	V1-015	Ken Cowan					21-Mar-1989
**		Convert to UIL.
**	V1-014	Marios Cleovoulou				10-Oct-1988
**		Give text of VMS specific errors to user
**	V1-013	Marios Cleovoulou				16-Sep-1988
**		Change callback order so error box dismisses faster
**	V1-012	Marios Cleovoulou				11-Sep-1988
**		Add modeless message boxes too
**	V1-011	Per Hamnqvist					05-Jul-1988
**		Change from decw$include to X11 on Ultrix include
**	V1-010	Marios Cleovoulou				30-Jun-1988
**		Remove private help system
**	V1-009	Marios Cleovoulou				29-Jun-1988
**		Make error boxes that are children of toplevel shells appear
**		at the shells geometry positions.
**	V1-008	Per Hamnqvist					10-May-1988
**		Comment out code in our Ultrix stub routine strerror.
**	V1-007  Marios Cleovoulou				10-May-1988
**		Use DwcHelpForWidget
**	V1-006  Marios Cleovoulou				21-Apr-1988
**		Implement "strerror" for ULTRIX
**	V1-005	Per Hamnqvist					21-Apr-1988
**		a) When including stdio, use a notation that Ultrix
**		   understands too.
**		b) Include the "errno.h" module explicitly on Ultrix.
**	V1-004  Marios Cleovoulou				19-Apr-1988
**		Change some constant names
**	V1-003  Marios Cleovoulou				13-Apr-1988
**		Add new help stuff
**	V1-002  Marios Cleovoulou				12-Apr-1988
**		Add compound string support.
**	V1-001  Marios Cleovoulou				 2-Apr-1988
**		Initial version.
**--
**/

#include    "dwc_compat.h"

#ifdef	VMS
#include <descrip.h>
#include <starlet.h>
#endif

#include <stdio.h>
#include <errno.h>

#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/Xm.h>
#ifdef vaxc
#pragma standard
#endif

#include "dwc_ui_datestructures.h"
#include "dwc_ui_calendar.h"
#include "dwc_ui_catchall.h"	
#include "dwc_ui_errorboxes.h"
#include "dwc_ui_misc.h"		/* for MISCGetTimeFrom... */
#include "dwc_ui_icons.h"
#include "dwc_ui_help.h"

extern AllDisplaysRecord ads;		/* defined in dwc_ui_calendar.c */

static void
create_error_box PROTOTYPE ((
	Widget	    parent,
	char	    *name,
	char	    *text,
	Boolean	    prepend_our_text,
	XtCallbackProc  callback_proc,
	caddr_t	    tag));

static void
ERROR_BOX_FOCUS PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

static void
remove_cursor PROTOTYPE ((
	Widget			w,
	caddr_t			tag));

void ERRORReportError
#ifdef _DWC_PROTO_
	(
	Widget	parent,
	char	*name,
	XtCallbackProc callback_proc,
	caddr_t	tag)
#else	/* no prototypes */
	(parent, name, callback_proc, tag)
	Widget	parent;
	char	*name;
	XtCallbackProc	callback_proc;
	caddr_t	tag;
#endif	/* prototype */
{
    /* No text, don't add any text to the label*/
    create_error_box(parent, name, NULL, FALSE, callback_proc, tag);

}

void
ERRORDisplayError
#ifdef _DWC_PROTO_
	(
	Widget	parent,
	char	*name)
#else	/* no prototypes */
	(parent, name)
	Widget	parent;
	char	*name;
#endif	/* prototype */
    {

    /* No text, don't add any text to the label, no callbacks or tag*/
    create_error_box(parent, name, NULL, FALSE, NULL, NULL);

}

void
ERRORDisplayText
#ifdef _DWC_PROTO_
	(
	Widget	parent,
	char	*name,
	char	*text)
#else	/* no prototypes */
	(parent, name, text)
	Widget	parent;
	char	*name;
	char	*text;
#endif	/* prototype */
    {

    /* text, add the text to the label, callback and tag */
    create_error_box(parent, name, text, TRUE, NULL, NULL);

}

void
ERRORDisplayTextCall
#ifdef _DWC_PROTO_
	(
	Widget	parent,
	char	*name,
	char	*text,
	XtCallbackProc callback_proc,
	caddr_t	tag)
#else	/* no prototypes */
	(parent, name, text, callback_proc, tag)
	Widget	parent;
	char	*name;
	char	*text;
	XtCallbackProc	callback_proc;
	caddr_t	tag;
#endif	/* prototype */
    {

    /* text, add the text to the label, callback and tag */
    create_error_box(parent, name, text, TRUE, callback_proc, tag);

}

#ifndef VMS

extern int sys_nerr;
extern char *sys_errlist[];

char *
strerror
#ifdef _DWC_PROTO_
	(
	int	errornumber)
#else	/* no prototypes */
	(errornumber)
	int	errornumber;
#endif	/* prototype */
    {

    if (errornumber <= sys_nerr)
	{
	return (sys_errlist[errornumber]);
	}
    else
	{
	return (NULL);
	}

}	

#endif

#ifdef VMS

static struct dsc$descriptor_s dsc$error;
static char		       vmserror [257];

#endif


char *
get_error_text
#ifdef _DWC_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{
    char	*text;

#ifdef VMS
    int		status;
    short int	msglen;
    int		outadr;

    if (errno == EVMSERR) {
	dsc$error.dsc$a_pointer  = vmserror;
	dsc$error.dsc$w_length   = 257;
	dsc$error.dsc$b_class    = DSC$K_CLASS_S;
	dsc$error.dsc$b_dtype    = DSC$K_DTYPE_T;

	status = sys$getmsg (vaxc$errno, &msglen, &dsc$error, 1, &outadr);
	if ((status & 1) == 0) {
	    return ("?Unknown error");
	}
	vmserror [msglen] = '\0';
	return (vmserror);
    }
#endif

    text = strerror (errno);
    if (text == NULL) {
	text = "?Unknown error";
    }
    return (text);

}

void
ERRORReportErrno
#ifdef _DWC_PROTO_
	(
	Widget	parent,
	char	*name,
	XtCallbackProc callback_proc,
	caddr_t	tag)
#else	/* no prototypes */
	(parent, name, callback_proc, tag)
	Widget	parent;
	char	*name;
	XtCallbackProc	callback_proc;
	caddr_t	tag;
#endif	/* prototype */
{
    char	*text;

    text = get_error_text ();

    /* text, add the text to the label, callback and tag */
    create_error_box(parent, name, text, TRUE, callback_proc, tag);

}

void
ERRORDisplayErrno
#ifdef _DWC_PROTO_
	(
	Widget	parent,
	char	*name)
#else	/* no prototypes */
	(parent, name)
	Widget	parent;
	char	*name;
#endif	/* prototype */
    {
    char	*text;

    text = get_error_text();

    /* text, add our text to the label, callback and tag */
    create_error_box(parent, name, text, TRUE, NULL, NULL);

}

static void
create_error_box
#ifdef _DWC_PROTO_
	(
	Widget	    parent,
	char	    *name,
	char	    *text,
	Boolean	    prepend_our_text,
	XtCallbackProc  callback_proc,
	caddr_t	    closure)
#else	/* no prototypes */
	(parent, name, text, prepend_our_text, callback_proc, closure)
	Widget	    parent;
	char	    *name;
	char	    *text;
	Boolean	    prepend_our_text;
	XtCallbackProc  callback_proc;
	caddr_t	    closure;
#endif	/* prototype */
{
    Cardinal	    ac;
    Arg		    arglist [3];
    
    MrmType	    class;
    Widget	    msgbox;
    XmString	    stext;
    XmString	    ftext;
    XmString	    cs;
    int		    status;
    Position	    x, y;
    XtWidgetGeometry	request ;
    XtGeometryResult	result ;

    static XtCallbackRec givefocus_cb [] = {
	{(XtCallbackProc) ERROR_BOX_FOCUS, NULL} ,
	{NULL, NULL},	/* will get filled in parameters */
	{NULL, NULL}
    };

    status = MrmFetchWidget(ads.hierarchy,
			    name,
			    parent,
			    &msgbox,
			    &class);

    if (status != MrmSUCCESS)
	DWC$UI_Catchall(DWC$DRM_ERRORBOX, status, 0);


    if ((XtCallbackProc)callback_proc != (XtCallbackProc)NULL)
	{
	givefocus_cb[1].callback = (XtCallbackProc)callback_proc;
	givefocus_cb[1].closure = (caddr_t)closure;
	}
    else
	{
	givefocus_cb[1].callback = (XtCallbackProc)remove_cursor;
	givefocus_cb[1].closure = (caddr_t)msgbox;
	}

    ac = 0;    
    XtSetArg(arglist[ac], XmNokCallback, givefocus_cb ); ac++;
    XtSetArg(arglist[ac], XmNcancelCallback, givefocus_cb ); ac++;
    XtSetValues(msgbox, arglist, ac);


    /* Replace the existing label with the passed text */
    if ((text != NULL) && (! prepend_our_text)) {
	cs = XmStringCreateSimple(text);
	XtSetArg (arglist [0], XmNmessageString,          cs);
	XtSetValues( msgbox, arglist, 1);
	XmStringFree(cs);
    }

    /* append the passed text to the existing label */
    if (prepend_our_text && (text != NULL)) {
	XtSetArg (arglist [0], XmNmessageString, &stext);
	XtGetValues (msgbox, arglist, 1);
	cs = XmStringCreateSimple(text);
	ftext = XmStringConcat (stext, cs);
	XtSetArg (arglist [0], XmNmessageString, ftext);
	XtSetValues (msgbox, arglist, 1);
	XmStringFree(cs);
	XmStringFree(ftext);
	/*
	**  Don't free the strings, as it seems the widget doesn't copy them.
	**  This means stext.  The GetValues seems to return
	**  a pointer that references the string owned by the widget,
	**  not a copy of the compound string.   Ftext seems to be
	**  copied, as the next GetValues returns a different address
	**  than ftext.
	*/
    }

    XtManageChild (msgbox);

    /*	  
    **  Is the message box in the right place?
    */	  
    if ( (XtX(XtParent(msgbox)) == 0) && (XtY(XtParent(msgbox)) == 0) )

	/* No, let's place it intelligently */
	if ( (XtX( parent ) != 0) && (XtY( parent ) != 0) )
	    {
	    XtSetArg( arglist[0], XmNx, XtX( parent ) + 25 );
	    XtSetArg( arglist[1], XmNy, XtY( parent ) + 25 );
	    XtSetValues( msgbox, arglist, 2 );
	    }
	else
	    {
	    x = .40 * XWidthOfScreen  (XtScreen (msgbox));
	    y = .40 * XHeightOfScreen (XtScreen (msgbox));
	    XtSetArg( arglist[0], XmNx, x + 25 );
	    XtSetArg( arglist[1], XmNy, y + 25 );
	    XtSetValues( msgbox, arglist, 2 );
	    };

    /*	  
    **  Let's try to see that it goes on top
    */	  
    request.request_mode = CWStackMode;
    request.stack_mode = Above;
    result = XtMakeGeometryRequest(msgbox, &request, NULL);

    XtRealizeWidget (msgbox);
    ICONSInactiveCursorDisplay (msgbox, ads.inactive_cursor);
    
}

void
ERRORDestroyErrorBox
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
    {

    /*	  
    **  Let's get rid of any help we may have.
    */	  
    HELPDestroyForModal(w, 0, NULL);    

    XtUnmanageChild (w);

    /* Why do we destroy the parent?  Well, the server was complaining	    */
    /* about not getting a valid window id when we tried to destroy 'w'.    */
    /* The box, in any case, is a MessageBox inside of a Dialog shell so    */
    /* this seems to work */
    XtDestroyWidget(XtParent(w));
    XFlush (XtDisplay (w));

    return;
}

static void
ERROR_BOX_FOCUS
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
    {
    Time	    time;
    Widget	    real_parent;
    Arg		    arglist[1];
    Cardinal	    ac;
    unsigned char   style;

    time = MISCGetTimeFromAnyCBS( cbs );
    real_parent = XtParent(XtParent(w));

/* try to give input focus to the widget that is our parent. Note
   that there is a hidden shell in the middle. */


    XmProcessTraversal (real_parent, XmTRAVERSE_CURRENT);    

    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */

    ICONSInactiveCursorRemove (w);

    ac = 0;
    XtSetArg(arglist[ac], XmNdialogStyle, &style ); ac++;
    XtGetValues( real_parent, arglist, ac );
    
    if (style == (int)XmDIALOG_APPLICATION_MODAL)
	ICONSInactiveCursorDisplay (real_parent, ads.inactive_cursor);

    return;
}

static void
remove_cursor
#ifdef _DWC_PROTO_
	(
	Widget w,
	caddr_t tag)
#else	/* no prototypes */
	(w, tag)
	Widget w;
	caddr_t tag;
#endif	/* prototype */
{
    ICONSInactiveCursorRemove(w);

    
}
