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
static char *BuildSystemHeader= "$Id: fileselect.c,v 1.1.4.3 1993/07/30 19:50:35 Lynda_Rice Exp $";
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
 *	fileselect.c - file selection stuff
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 23rd June 1988
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

static char sccsid[] = "@(#)fileselect.c	1.1	15:20:47 7/3/88";


#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  /* DEBUG */
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
 *	FileSelectorCancelCallBack
 *
 ********************************/

static void
FileSelectorCancelCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	register DxDiffDisplayPtr dxdiffdisplay = (DxDiffDisplayPtr)clientd;
	register FileSelectorPtr  fsptr;

	if (FileSelectorPtrFileSelector(DxDiffDisplayPtrLFileSelector(dxdiffdisplay)) == w) {
		fsptr = DxDiffDisplayPtrLFileSelector(dxdiffdisplay);
	} else {
		fsptr = DxDiffDisplayPtrRFileSelector(dxdiffdisplay);
	}

	XtUnmanageChild(w);	/* unmanage the sucker !! */
}

static XtCallbackRec	fileselectorcancelcallbacklist[] = {
	{ (VoidProc)FileSelectorCancelCallback, NULL },
	{ (VoidProc)NULL, NULL }
};

/********************************
 *
 *	FileSelectorDestroyCallback
 *
 ********************************/

static void
FileSelectorDestroyCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	register DxDiffDisplayPtr dxdiffdisplay = (DxDiffDisplayPtr)clientd;
	register FileSelectorPtr  fsptr = NULL;

	if (DxDiffDisplayPtrLFileSelector(dxdiffdisplay) &&
	    FileSelectorPtrFileSelector(DxDiffDisplayPtrLFileSelector(dxdiffdisplay)) == w) {
		fsptr = DxDiffDisplayPtrLFileSelector(dxdiffdisplay);
			DxDiffDisplayPtrLFileSelector(dxdiffdisplay) = (FileSelectorPtr)NULL;
	} else {
		fsptr = DxDiffDisplayPtrRFileSelector(dxdiffdisplay);
		DxDiffDisplayPtrRFileSelector(dxdiffdisplay) = (FileSelectorPtr)NULL;
	}

	if (fsptr == (FileSelectorPtr)NULL)
		return;

	if (FileSelectorPtrFile(fsptr) != (char *)NULL)
		XtFree(FileSelectorPtrFile(fsptr));
	if (FileSelectorPtrDir(fsptr) != (char *)NULL)
		XtFree(FileSelectorPtrDir(fsptr));

	XtFree((char *)fsptr);

}


static XtCallbackRec	fileselectordestroycallbacklist[] = {
	{ (VoidProc)FileSelectorDestroyCallback, NULL },
	{ (VoidProc)NULL, NULL }
};

/********************************
 *
 *	FileSelectorActivateCallback
 *
 ********************************/

static void
FileSelectorActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	register DxDiffDisplayPtr	dxdiffdisplay = (DxDiffDisplayPtr)clientd;
	XmFileSelectionBoxCallbackStruct	*cbsp = (XmFileSelectionBoxCallbackStruct *)calld;
	register FileSelectorPtr 	fsptr;

#if (((XmVERSION == 1) && (XmREVISION >=2)) || XmVERSION >= 2)
#define charset XmFONTLIST_DEFAULT_TAG
#else
	char				*charset = "ISO8859-1";
#endif

	if (FileSelectorPtrFileSelector(DxDiffDisplayPtrLFileSelector(dxdiffdisplay)) == w) {
		fsptr = DxDiffDisplayPtrLFileSelector(dxdiffdisplay);
	} else {
		fsptr = DxDiffDisplayPtrRFileSelector(dxdiffdisplay);
	}

	if (cbsp->reason == XmCR_OK) {
		int len;

		if (cbsp->value != (XmString)NULL) {
			FileSelectorPtrValue(fsptr) = (XmString)(cbsp->value);

			if (FileSelectorPtrFile(fsptr) != (char *)NULL)
				XtFree(FileSelectorPtrFile(fsptr));

			
			XmStringGetLtoR(cbsp->value, charset, 
					&FileSelectorPtrFile(fsptr));
		}

		if (cbsp->mask != (XmString)NULL) {
			FileSelectorPtrValue(fsptr) = (XmString)(cbsp->mask);

			if (FileSelectorPtrDir(fsptr) != (char *)NULL)
				XtFree(FileSelectorPtrDir(fsptr));

			
			XmStringGetLtoR(cbsp->mask, charset,
					&FileSelectorPtrDir(fsptr));
		}
	}
}

static XtCallbackRec	fileselectoractivatecallbacklist[] = {
	{ (VoidProc)FileSelectorActivateCallback, NULL },
	{ (VoidProc)NULL, NULL }
};

extern void HelpActivateCallback();

#ifdef HYPERHELP
static XtCallbackRec	fileselectorhelpcallbacklist[] = {
	{ (VoidProc)HelpActivateCallback, "select_files"},
	{ (VoidProc)NULL, NULL }
#else
static XtCallbackRec	fileselectorhelpcallbacklist[] = {
	{ (VoidProc)HelpActivateCallback, "Overview file_menu open_files"},
	{ (VoidProc)NULL, NULL }
#endif
};
/********************************
 *
 *	NewFileSelector
 *
 ********************************/

static FileSelectorPtr
NewFileSelector(copy)
	register FileSelectorPtr copy;
{
	register FileSelectorPtr new;

	if ((new = (FileSelectorPtr)XtMalloc(sizeof (FileSelector))) == (FileSelectorPtr)NULL) {
		return (FileSelectorPtr)NULL;	/* error */
	}

	if (copy != (FileSelectorPtr)NULL)
		bcopy((char*)copy, (char *)new, sizeof (FileSelector));

	return new;
}

/********************************
 *
 *	CreateFileSelector
 *
 ********************************/

static FileSelector fileselector = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)fileselectordestroycallbacklist),
	StaticInitDialogBoxArgList(XmPIXELS, XmSTRING, NULL, XmDIALOG_MODELESS,
				   XmRESIZE_ANY, True, 10, 10),
	{ XmNautoUnmanage, True},
	{ XmNdefaultPosition, True },
	{ XmNdirMask, NULL },
	{ XmNokCallback, (XtArgVal)fileselectoractivatecallbacklist },
	{ XmNcancelCallback, (XtArgVal)fileselectorcancelcallbacklist },
	{ XmNhelpCallback, (XtArgVal)fileselectorhelpcallbacklist},
	NULL,
	NULL,			 	/* leave the context alone! */
	NULL,
	NULL,
	NULL,
	NULL
};

FileSelectorPtr
CreateFileSelector(parent, name, label, core, dialog, dxdiffdisplay, closure)
	Widget			parent;
	char			*name;
	char			*label;
	CoreArgListPtr		core;
	DialogBoxArgListPtr	dialog;
	DxDiffDisplayPtr	dxdiffdisplay;
	caddr_t			closure;
{
	register FileSelectorPtr new;

	if ((new = NewFileSelector(&fileselector)) == (FileSelectorPtr)NULL) {
		return (FileSelectorPtr)NULL;	/* error */
	}


	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&FileSelectorPtrCoreArgList(new), sizeof (CoreArgList) - 
		      sizeof (Arg));
	}

	if (dialog != (DialogBoxArgListPtr)NULL) {
		bcopy((char *)dialog, (char *)&FileSelectorPtrDialogBoxArgList(new), sizeof (DialogBoxArgList));
	}

	DialogBoxStyle(FileSelectorPtrDialogBoxArgList(new)) = XmDIALOG_MODELESS;
	DialogBoxTitle(FileSelectorPtrDialogBoxArgList(new)) = (XtArgVal)label;
	FileSelectorPtrDirMaskArg(new) = (XtArgVal)
		XmStringLtoRCreate("*" , "ISO8859-1");

	fileselectordestroycallbacklist[0].closure = fileselectoractivatecallbacklist[0].closure = 
	fileselectorcancelcallbacklist[0].closure = closure;


	FileSelectorPtrFileSelector(new) = (Widget)XmCreateFileSelectionDialog(parent,
								  name,
								  &FileSelectorPtrCoreArgList(new),
								  NumberOfArgsBetween(&FileSelectorPtrCoreArgList(new),
								  &FileSelectorPtrHelpCallBack(new)));

	if (FileSelectorPtrFileSelector(new) == (Widget)NULL) {
             return (FileSelectorPtr)NULL;   /* error */
        }

	{
		char	newname[80];
		Arg	arg;

		/* Only print :x if this is not the main window. */
		if ( DxDiffDisplayPtrDisplayIdx(dxdiffdisplay) == 0 )
		    sprintf(newname,"%s", label );
		else sprintf(newname,"%s: %1d", label, DxDiffDisplayPtrDisplayIdx(dxdiffdisplay));
		
		arg.name = XtNtitle;
		arg.value = (XtArgVal)newname;

		XtSetValues(XtParent(FileSelectorPtrFileSelector(new)), &arg, 1);
	}
	
	XtManageChild(FileSelectorPtrFileSelector(new));

	return new;
}
