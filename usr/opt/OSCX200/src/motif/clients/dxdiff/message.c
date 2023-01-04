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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: message.c,v 1.1.2.3 92/11/25 08:10:33 Russ_Kuhn Exp $";
#endif		/* BuildSystemHeader */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
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
 *
 *	dxdiff
 *
 *	message.c - message box handling
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 4th July 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 */



#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include "dxdiff.h"
#include "arglists.h"
#include "y.tab.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"
#include "differencebox.h"
#include "menu.h"
#include "text.h"
#include "display.h"


/********************************
 *
 *	MessageBoxDestroyCallback
 *
 ********************************/

static void
MessageBoxDestroyCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	register MessageBoxPtr messagebox = (MessageBoxPtr)clientd;

	if (LabelLabel(MessageBoxPtrLabelArgList(messagebox)) != NULL) {
		XtFree((char *)LabelLabel(MessageBoxPtrLabelArgList(messagebox)));
	}

	XtFree((char *)messagebox);
}


static XtCallbackRec	messageboxdestroycallbacklist[] = {
	{ (VoidProc)MessageBoxDestroyCallback, NULL },
	{ (VoidProc)NULL, NULL }
};

/********************************
 *
 *	MessageBoxActivateCallback
 *
 ********************************/

static void
MessageBoxActivateCallback(w, clientd, calld)	/* the default one !!!! */
	Widget	w;
	caddr_t	clientd,
		calld;
{
	register MessageBoxPtr messagebox = (MessageBoxPtr)clientd;

	XtDestroyWidget(MessageBoxPtrWidget(messagebox));	/* throw it away */
}

static XtCallbackRec	messageboxactivatecallbacklist[] = {
	{ (VoidProc)MessageBoxActivateCallback, NULL },
	{ (VoidProc)NULL, NULL }
};

/********************************
 *
 *	SetMessageBoxMessage
 *
 ********************************/

void 
SetMessageBoxMessage(messagebox, message)
	register MessageBoxPtr messagebox;
	char		       *message;
{
	if (LabelLabel(MessageBoxPtrLabelArgList(messagebox)) != NULL) {
		XtFree((char *)LabelLabel(MessageBoxPtrLabelArgList(messagebox)));
	}

	if (message != (char *)NULL) {
		LabelLabel(MessageBoxPtrLabelArgList(messagebox)) = (XtArgVal)
		XmStringLtoRCreate(message , "ISO8859-1");
		XtSetValues(MessageBoxPtrWidget(messagebox),
			    PointerToArg(LabelLabel(MessageBoxPtrLabelArgList(messagebox))),
			    1
		);
	}
}

/********************************
 *
 *	NewMessageBox
 *
 ********************************/

static MessageBoxPtr
NewMessageBox(copy)
	register MessageBoxPtr copy;
{
	register MessageBoxPtr new;

	if ((new = (MessageBoxPtr)XtMalloc(sizeof (MessageBox))) == (MessageBoxPtr)NULL) {
		return (MessageBoxPtr)NULL;	/* error */
	}

	if (copy != (MessageBoxPtr)NULL)
		bcopy((char*)copy, (char *)new, sizeof (MessageBox));

	return new;
}

/********************************
 *
 *	CreateMessageBox
 *
 ********************************/

static MessageBox messagebox = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)messageboxdestroycallbacklist),
	StaticInitDialogBoxArgList(XmPIXELS, XmSTRING, NULL, 
	    XmDIALOG_APPLICATION_MODAL, XmRESIZE_ANY, True, 0, 0),
	StaticInitLabelArgList(XmSTRING, NULL, 10, 10, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	{ XmNdefaultPosition, True },
	{ XmNactivateCallback, (XtArgVal)messageboxactivatecallbacklist },
	NULL,
	NULL
};

MessageBoxPtr
CreateMessageBox(parent, name, core, dialog, label, message, activateproc, closure)
	Widget			parent;
	char			*name;
	CoreArgListPtr		core;
	DialogBoxArgListPtr	dialog;
	LabelArgListPtr		label;
	char			*message;
	void			(*activateproc)();
	caddr_t			closure;
{
	register MessageBoxPtr new;

	if ((new = NewMessageBox(&messagebox)) == (MessageBoxPtr)NULL) {
		return (MessageBoxPtr)NULL;	/* error */
	}


	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&MessageBoxPtrCoreArgList(new), sizeof (CoreArgList) - 
		      sizeof (Arg));
	}

	if (dialog != (DialogBoxArgListPtr)NULL) {
		bcopy((char *)dialog, (char *)&MessageBoxPtrDialogBoxArgList(new), sizeof (DialogBoxArgList));
	}

	if (label != (LabelArgListPtr)NULL) {
		bcopy((char *)label, (char *)&MessageBoxPtrLabelArgList(new), sizeof (LabelArgList));
	}

	DialogBoxStyle(MessageBoxPtrDialogBoxArgList(new)) = 
		XmDIALOG_APPLICATION_MODAL;

	DialogBoxTitle(MessageBoxPtrDialogBoxArgList(new)) = (XtArgVal)"dxdiff: message";

	(&MessageBoxPtrLabelArgList(new))->label.name = XmNmessageString;
	if (message != (char *)NULL) {
		LabelLabel(MessageBoxPtrLabelArgList(new)) = (XtArgVal)
		XmStringLtoRCreate(message , "ISO8859-1");
	}

	messageboxdestroycallbacklist[0].closure = messageboxactivatecallbacklist[0].closure = new;

	if (activateproc != (void (*)())NULL) {
		MessageBoxPtrActivateCallBack(new) = (int)activateproc;
	}

	MessageBoxPtrClosure(new) = (caddr_t)closure;	/* the display  ??? */


	MessageBoxPtrWidget(new) = (Widget)XmCreateErrorDialog(parent,
							   name,
							   &MessageBoxPtrCoreArgList(new),
						 	   NumberOfArgsBetween(&MessageBoxPtrCoreArgList(new),
							   &MessageBoxPtrActivateCallBack(new)));

	if (MessageBoxPtrWidget(new) == (Widget)NULL) {
             return (MessageBoxPtr)NULL;   /* error */
        }

	XtUnmanageChild((Widget)XmMessageBoxGetChild(MessageBoxPtrWidget(new),
			XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild((Widget)XmMessageBoxGetChild(MessageBoxPtrWidget(new),
			XmDIALOG_HELP_BUTTON));
	XtManageChild((Widget)MessageBoxPtrWidget(new));

	return new;
}
