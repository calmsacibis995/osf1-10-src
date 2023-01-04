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
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/errors.c,v 1.1.2.2 92/08/03 09:48:27 Dave_Hill Exp $";	/* BuildSystemHeader */
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
 *	02 Feb 1990	Colin Prosser
 *
 *	Fix typo in return type of ReportDiffErrors().  "void void" ==> void
 *	The compiler ought to have caught this but didn't until now.
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


typedef	struct _differrclosure {	/* private structure for this modeule */
	DxDiffDisplayPtr	dxdiffdisplay;	/* my display */
	DenPtr			denptr;		/* cuurent error being displayed in the diff list */
} DiffErrClosure, *DiffErrClosurePtr;

/********************************
 *
 *	ReportDiffErrorsMBActivateCallBack
 *
 ********************************/

static void
ReportDiffErrorsMBActivateCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	register MessageBoxPtr	     messagebox = (MessageBoxPtr)clientd;
	register DiffErrClosurePtr   differrclosure = (DiffErrClosurePtr)MessageBoxPtrClosure(messagebox);

	if (differrclosure->denptr != (DenPtr)NULL) {
		SetMessageBoxMessage(messagebox, differrclosure->denptr->error);
		differrclosure->denptr = differrclosure->denptr->next;
		XtSetMappedWhenManaged(MessageBoxPtrWidget(messagebox), False);
		XtSetMappedWhenManaged(MessageBoxPtrWidget(messagebox), True);
	} else { /* all done */
		XtFree((char *)differrclosure);
		XtDestroyWidget(MessageBoxPtrWidget(messagebox));
	}
}

/********************************
 *
 *	ReportDiffErrors
 *
 ********************************/

void
ReportDiffErrors(dxdiffdisplay)
	register DxDiffDisplayPtr dxdiffdisplay;
{
	char			   *error;
	register MessageBoxPtr	   messagebox;
	register DiffErrClosurePtr differrclosure;

	if ((differrclosure = (DiffErrClosurePtr)XtMalloc(sizeof (DiffErrClosure))) == (DiffErrClosurePtr)NULL) {
		return;	/* oops */
	}


	if (DxDiffDisplayPtrDiffList(dxdiffdisplay)->dencnt != 0) {
		error = DxDiffDisplayPtrDiffList(dxdiffdisplay)->denhead->error;
		differrclosure->denptr = DxDiffDisplayPtrDiffList(dxdiffdisplay)->denhead->next;
	} else {
		error = "diff(1) child returned non zero exit status!";
		differrclosure->denptr = (DenPtr)NULL;
	}

	messagebox = (MessageBoxPtr)CreateMessageBox(MainADBPtrWidget(DxDiffDisplayPtrMainADB(dxdiffdisplay)),
					"dxdiffdisplayerror",
					(CoreArgListPtr)NULL,
					(DialogBoxArgListPtr)NULL,
					(LabelArgListPtr)NULL,
					error,
					differrclosure
		     );
}
