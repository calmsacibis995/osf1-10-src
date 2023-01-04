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
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/textdisplayadb.c,v 1.1.2.2 92/08/03 09:50:02 Dave_Hill Exp $";	/* BuildSystemHeader */
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
 *      dxdiff
 *
 *      textdisplayadb.c  - code
 *
 *      Author: Laurence P. G. Cable
 *
 *      Created : May 9th 1988
 *
 *
 *      Description
 *      -----------
 *
 *
 *      Modification History
 *      ------------ -------
 *      
 */

static char sccsid[] = "@(#)textdisplayadb.c    1.5     19:02:17 10/4/88";


#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/ScrollBarP.h>
#include <Xm/LabelP.h>
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
extern	void VScrollBarUnitIncCallBack(),
	     VScrollBarUnitDecCallBack(),
	     VScrollBarPageIncCallBack(),
	     VScrollBarPageDecCallBack(),
	     VScrollBarToTopCallBack(),
	     VScrollBarToBottomCallBack(),
	     VScrollBarDragCallBack(),
	     VScrollBarValueChangedCallBack();

extern	void HScrollBarUnitIncCallBack(),
	     HScrollBarUnitDecCallBack(),
	     HScrollBarPageIncCallBack(),
	     HScrollBarPageDecCallBack(),
	     HScrollBarToTopCallBack(),
	     HScrollBarToBottomCallBack(),
	     HScrollBarDragCallBack(),
	     HScrollBarValueChangedCallBack();

/********************************
 *
 *      TextDisplayADBDestroyCallBack
 *
 ********************************/

static void
TextDisplayADBDestroyCallBack(w, clientd, calld)
        Widget  w;
        caddr_t clientd,
                calld;
{
        XtFree((char *)(TextDisplayADBPtr)clientd);
}

static XtCallbackRec TextDisplayADBDestroyCallbackList[] = {
        { (VoidProc)TextDisplayADBDestroyCallBack, 0 },
        { (VoidProc)NULL, 0 }
};

/********************************
 *
 *      NewTextDisplayADB
 *
 ********************************/

static  TextDisplayADBPtr
NewTextDisplayADB(copy)
        TextDisplayADBPtr copy;
{
        TextDisplayADBPtr new;

        if ((new = (TextDisplayADBPtr)XtMalloc(sizeof (TextDisplayADB))) == (TextDisplayADBPtr)NULL) {
                return new;     /* error */
        }

        if (copy != (TextDisplayADBPtr)NULL) {
                bcopy((char *)copy, (char *)new, sizeof (TextDisplayADB));
        }

        return new;
}


/********************************
 *
 *      CreateTextDisplayADB
 *
 ********************************/

static  TextDisplayADB  textdisplayadb = {
        StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)TextDisplayADBDestroyCallbackList),
        StaticInitDialogBoxArgList(XmPIXELS, XmSTRING, NULL,
                                   XmDIALOG_WORK_AREA, True, False, 0, 0),
        StaticInitADBConstraintArgList(XmATTACH_FORM, XmATTACH_FORM,
                                       XmATTACH_SELF, XmATTACH_SELF,
                                       NULL, NULL,
                                       NULL, NULL,
                                       0, 0, 0, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
};

#define FileNameLabelHeight             30
#define FileNameLabelBorderWidth        1

#define VScrollBarWidth                 16
#define VScrollBarBorderWidth           1

#define MenuBarHeight                   FileNameLabelHeight
#define MenuBarBorderWidth              1
        
#define TextDisplayBorderWidth          0

extern  FileNamePtr     CreateFileName();
extern  AMenuBarPtr     CreateNewDisplayMenu();
extern  TextDisplayPtr  CreateTextDisplay();

TextDisplayADBPtr
CreateTextDisplayADB(parent, name, core, constraints, whichfile, closure)
	char * name;
        Widget                  parent;
        CoreArgListPtr          core;
        ADBConstraintArgListPtr constraints;
        WhichFile               whichfile;
        caddr_t                 closure;
{
        register TextDisplayADBPtr      new;

        CoreArgList                     coreargs;
        CoreArgListPtr                  adbcore, fncore, mbcore, vsbcore, hsbcore, lblcore;
        ADBConstraintArgList            constraintargs;
        register int                    tx,ty,tw,th,tbw;
	Arg	args[2];

        if ((new = NewTextDisplayADB(&textdisplayadb)) == (TextDisplayADBPtr)NULL) {
                return new;     /* error */
        }

        if (core != (CoreArgListPtr)NULL) {
                bcopy((char *)core, (char *)&TextDisplayADBPtrCoreArgList(new),
                      sizeof (CoreArgList) - sizeof (Arg));
        }

        CoreBorderWidth(TextDisplayADBPtrCoreArgList(new)) = 0;

        DialogBoxUnits(TextDisplayADBPtrDialogBoxArgList(new)) = XmPIXELS;
        DialogBoxStyle(TextDisplayADBPtrDialogBoxArgList(new)) = XmDIALOG_WORK_AREA;
        DialogBoxResize(TextDisplayADBPtrDialogBoxArgList(new)) = XmRESIZE_ANY;
        DialogBoxChildOverlap(TextDisplayADBPtrDialogBoxArgList(new)) = False;


        if (constraints != (ADBConstraintArgListPtr)NULL) {
                bcopy((char *)constraints, (char *)&TextDisplayADBPtrADBConstraintArgList(new),
                      sizeof (ADBConstraintArgList));
        }

        ADBConstraintTopAttachment(TextDisplayADBPtrADBConstraintArgList(new)) = 
        ADBConstraintBottomAttachment(TextDisplayADBPtrADBConstraintArgList(new)) = XmATTACH_FORM;
        

        if ((TextDisplayADBPtrWhatFile(new) = whichfile) == LeftFile) {
                ADBConstraintRightAttachment(TextDisplayADBPtrADBConstraintArgList(new)) = XmATTACH_SELF;
                ADBConstraintLeftAttachment(TextDisplayADBPtrADBConstraintArgList(new)) = XmATTACH_FORM;
        } else {
                ADBConstraintLeftAttachment(TextDisplayADBPtrADBConstraintArgList(new)) = XmATTACH_SELF;
                ADBConstraintRightAttachment(TextDisplayADBPtrADBConstraintArgList(new)) = XmATTACH_FORM;
        }


        TextDisplayADBPtrWidget(new) =  (Widget)XmCreateForm(parent, name,
                                                (ArgList)&TextDisplayADBPtrCoreArgList(new),
                                                NumberOfArgsInArgListStruct(CoreArgList) +
                                                NumberOfArgsInArgListStruct(ADBConstraintArgList) +
                                                NumberOfArgsInArgListStruct(DialogBoxArgList)
                                        );

        if (TextDisplayADBPtrWidget(new) == (Widget)NULL) {
                XtFree((char *)new);
                return (TextDisplayADBPtr)NULL;
        }

        XtManageChild(TextDisplayADBPtrWidget(new));

        InitCoreArgList(coreargs);
        InitADBConstraintArgList(constraintargs);

        adbcore = &TextDisplayADBPtrCoreArgList(new);
        GetCoreArgs(TextDisplayADBPtrWidget(new), adbcore);


        CoreX(coreargs) = CoreY(coreargs) = FileNameLabelBorderWidth;
        CoreWidth(coreargs) = CorePtrWidth(adbcore) - 2 * FileNameLabelBorderWidth;
        CoreHeight(coreargs) = FileNameLabelHeight;
        CoreBorderWidth(coreargs) = FileNameLabelBorderWidth;

        ADBConstraintTopAttachment(constraintargs) = 
        ADBConstraintLeftAttachment(constraintargs) = 
        ADBConstraintRightAttachment(constraintargs) = XmATTACH_FORM;
        ADBConstraintBottomAttachment(constraintargs) = XmATTACH_NONE;
        

        ADBConstraintTopWidget(constraintargs) = 
        ADBConstraintRightWidget(constraintargs) = 
        ADBConstraintLeftWidget(constraintargs) = 
        ADBConstraintBottomWidget(constraintargs) = NULL;

        ADBConstraintTopOffset(constraintargs) = 
        ADBConstraintRightOffset(constraintargs) = 
        ADBConstraintLeftOffset(constraintargs) = 
        ADBConstraintBottomOffset(constraintargs) = 0;


        
        /* create the filename label */

        TextDisplayADBPtrFilename(new) = CreateFileName(TextDisplayADBPtrWidget(new),
                                                        "filenamebar",
                                                        &coreargs,
                                                        &constraintargs,
                                                        (LabelArgListPtr)NULL);

        if (TextDisplayADBPtrFilename(new) == (FileNamePtr)NULL) {
                XtDestroyWidget(TextDisplayADBPtrWidget(new));
                return (TextDisplayADBPtr)NULL;
        }
 

        /* now create the menu bar */
        

        GetCoreArgs(TextDisplayADBPtrWidget(new), adbcore);

        CoreX(coreargs) = MenuBarBorderWidth;
        CoreWidth(coreargs) = CorePtrWidth(adbcore) - 2 * MenuBarBorderWidth;
        CoreY(coreargs) = CorePtrHeight(adbcore) - (MenuBarHeight + MenuBarBorderWidth);
        CoreHeight(coreargs) = 1;       /* cheat ?? */
        CoreBorderWidth(coreargs) = MenuBarBorderWidth;

        ADBConstraintBottomAttachment(constraintargs) = XmATTACH_FORM;
        ADBConstraintTopAttachment(constraintargs) = XmATTACH_NONE;

        TextDisplayADBPtrMenuBar(new) = (AMenuBarPtr)CreateDisplayMenu(TextDisplayADBPtrWidget(new),
                                                          &coreargs,
                                                          &constraintargs,
                                                          (MenuBarArgListPtr)NULL,
                                                          closure
                                        );

        if (TextDisplayADBPtrMenuBar(new) == (AMenuBarPtr)NULL) {
                XtDestroyWidget(TextDisplayADBPtrWidget(new));
                return (TextDisplayADBPtr)NULL;
        }


        mbcore = &AMenuBarPtrCoreArgList(TextDisplayADBPtrMenuBar(new));
#if     0
        GetCoreArgs(AMenuBarPtrWidget(TextDisplayADBPtrMenuBar(new)), mbcore);
#endif

	ADBConstraintLeftAttachment(constraintargs) = XmATTACH_FORM;
	ADBConstraintRightAttachment(constraintargs) = XmATTACH_FORM;
	ADBConstraintTopAttachment(constraintargs) = XmATTACH_WIDGET;
        ADBConstraintTopWidget(constraintargs) = (XtArgVal)
		FileNamePtrWidget(TextDisplayADBPtrFilename(new));
	ADBConstraintBottomAttachment(constraintargs) = XmATTACH_WIDGET;
        ADBConstraintBottomWidget(constraintargs) = (XtArgVal)
		AMenuBarPtrWidget(TextDisplayADBPtrMenuBar(new));

        TextDisplayADBPtrTextDisplay(new) = CreateTextDisplay(TextDisplayADBPtrWidget(new),
                                                              "textdisplay",
                                                              &coreargs,
                                                              &constraintargs,
                                                              (TextArgListPtr)NULL,
                                                              (caddr_t)new,
							      whichfile
                                            );

        if (TextDisplayADBPtrTextDisplay(new) == (TextDisplayPtr)NULL) {
                XtDestroyWidget(TextDisplayADBPtrWidget(new));
                return (TextDisplayADBPtr)NULL;
        }

	XtSetArg(args[0], XmNverticalScrollBar, &TextDisplayADBPtrVScroll(new));
	XtSetArg(args[1], XmNhorizontalScrollBar, &TextDisplayADBPtrHScroll(new));
	XtGetValues(TextDisplayPtrScrollWidget(TextDisplayADBPtrTextDisplay(new)), 
		    args, 2);

	XtAddCallback((Widget)TextDisplayADBPtrVScroll(new), XmNincrementCallback,
			VScrollBarUnitIncCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrVScroll(new), XmNdecrementCallback,
			VScrollBarUnitDecCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrVScroll(new), XmNpageIncrementCallback,
			VScrollBarPageIncCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrVScroll(new), XmNpageDecrementCallback,
			VScrollBarPageDecCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrVScroll(new), XmNtoTopCallback,
			VScrollBarToTopCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrVScroll(new), XmNtoBottomCallback,
			VScrollBarToBottomCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrVScroll(new), XmNdragCallback,
			VScrollBarDragCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrVScroll(new), XmNvalueChangedCallback,
			VScrollBarValueChangedCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrHScroll(new), XmNincrementCallback,
			HScrollBarUnitIncCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrHScroll(new), XmNdecrementCallback,
			HScrollBarUnitDecCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrHScroll(new), XmNpageIncrementCallback,
			HScrollBarPageIncCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrHScroll(new), XmNpageDecrementCallback,
			HScrollBarPageDecCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrHScroll(new), XmNtoTopCallback,
			HScrollBarToTopCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrHScroll(new), XmNtoBottomCallback,
			HScrollBarToBottomCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrHScroll(new), XmNdragCallback,
			HScrollBarDragCallBack, (caddr_t)closure);
	XtAddCallback((Widget)TextDisplayADBPtrHScroll(new), XmNvalueChangedCallback,
			HScrollBarValueChangedCallBack, (caddr_t)closure);

        return new;
}
