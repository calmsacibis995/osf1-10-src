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
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/textdisplay.c,v 1.1.2.2 92/08/03 09:49:52 Dave_Hill Exp $";	/* BuildSystemHeader */
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
 *      textdisplay.c - text display handler code
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
 *      17th Aug 1988   Laurence P. G. Cable
 *
 *      Add autoShowInsertPoint field to TextArgList to stop
 *      cursor being displayed.
 */

static char sccsid[] = "@(#)textdisplay.c       1.9     19:02:16 10/4/88";


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
 *      TextDisplayDestroyCallBack
 *
 ********************************/

static void
TextDisplayDestroyCallBack(w, clientd, calld)
        Widget  w;
        caddr_t clientd,
                calld;
{
        register TextDisplayPtr textdisplay = (TextDisplayPtr)clientd;

        if (TextDisplayPtrFile(textdisplay) != (FileInfoPtr)NULL) {
                FreeFile(TextDisplayPtrFile(textdisplay));
        }

        XtFree((char *)textdisplay);
}


static  XtCallbackRec   TextDisplayDestroyCallbackList[] = {
        { (VoidProc)TextDisplayDestroyCallBack, 0 },
        { (VoidProc)NULL, 0 }
};

/********************************
 *
 *      NewTextDisplay
 *
 ********************************/

static  TextDisplayPtr
NewTextDisplay(copy)
        TextDisplayPtr copy;
{
        register TextDisplayPtr new;

        if ((new = (TextDisplayPtr)XtMalloc(sizeof (TextDisplay))) == (TextDisplayPtr)NULL) {
                return new;
        }

        if (copy != (TextDisplayPtr)NULL) {
                bcopy((char *)copy, (char *)new, sizeof (TextDisplay));
        }

        return new;
}

/********************************
 *
 *      CreateTextDisplay
 *
 ********************************/

static TextDisplay textdisplay = {
        StaticInitCoreArgList(0, 0, 0, 0, 0, NULL), /**** NOTE: Not statically inited ****/
        StaticInitADBConstraintArgList(XmATTACH_WIDGET, XmATTACH_WIDGET, 
                                       XmATTACH_SELF, XmATTACH_SELF,
                                       NULL, NULL,
                                       NULL, NULL,
                                       0, 0, 0, 0),
        StaticInitTextArgList(0, 0, XmPIXELS, False, False,
                              XmSTATIC, XmAPPLICATION_DEFINED, XmVARIABLE,
			      XmMULTI_LINE_EDIT,
                              True, True,
			      80,60,False,(XtArgVal)"",False,False,False,False,0),
        StaticInitFontArgList(0, 0, NULL),
	NULL,
        NULL,
        NULL,   /* scroll window */
        0,              /* visiblewidth */
        0,              /* textwidgetxoffset */
        0,              /* actual width of text widget */
};

TextDisplayPtr
CreateTextDisplay(parent, name, core, constraints, text, closure, whichfile)
        Widget                  parent;
        char                    *name;
        CoreArgListPtr          core;
        ADBConstraintArgListPtr constraints;
        TextArgListPtr          text;
        caddr_t                 closure;
        WhichFile               whichfile;
{
        register TextDisplayPtr new;

        if ((new = NewTextDisplay(&textdisplay)) == (TextDisplayPtr)NULL) {
                return new;     /* error */ }

        if (core != (CoreArgListPtr)NULL) {
                bcopy((char *)core, (char *)&TextDisplayPtrCoreArgList(new),
                      sizeof(CoreArgList) - sizeof(Arg));
        }

        if (constraints != (ADBConstraintArgListPtr)NULL) {
                bcopy((char *)constraints, (char *)&TextDisplayPtrConstraintArgList(new),
                      sizeof(ADBConstraintArgList));
        }

        if (text != (TextArgListPtr)NULL) {
                bcopy((char *)text, (char *)&TextDisplayPtrTextArgList(new),
                      sizeof(TextArgList));
        }
        
	TextScrollLeftSide(TextDisplayPtrTextArgList(new)) = (whichfile == LeftFile);

        TextDisplayPtrScrollWidget(new) = (Widget)XmCreateScrolledWindow(parent, "textwindow", 
                                        (ArgList)&TextDisplayPtrCoreArgList(new),
                                        NumberOfArgsInArgListStruct(CoreArgList) +
                                        NumberOfArgsInArgListStruct(ADBConstraintArgList) +
                                        NumberOfArgsInArgListStruct(TextArgList) - 1
                                    );

        if (TextDisplayPtrScrollWidget(new) == (Widget)NULL) {
                return (TextDisplayPtr)NULL;    /* error */
        }

        TextDisplayPtrScrollWidth(new) = CorePtrWidth(core);
        TextDisplayPtrScrollHeight(new) = CorePtrHeight(core);

        XtManageChild(TextDisplayPtrScrollWidget(new));

        TextResizeHeight(TextDisplayPtrTextArgList(new)) =
        TextResizeWidth(TextDisplayPtrTextArgList(new)) = 
        TextWordWrap(TextDisplayPtrTextArgList(new)) =
        TextEditable(TextDisplayPtrTextArgList(new)) = False;   /* force */

        CoreX(TextDisplayPtrCoreArgList(new)) =  CoreY(TextDisplayPtrCoreArgList(new)) = 0;
	CoreHeight(TextDisplayPtrCoreArgList(new)) = 200;
        /* set up the Destroy callback now .... */

        CoreDestroyCallBack(TextDisplayPtrCoreArgList(new)) = (XtArgVal)TextDisplayDestroyCallbackList;
        TextDisplayDestroyCallbackList[0].closure = (caddr_t)new;

        TextDisplayPtrWidget(new) = (Widget)XmCreateText(TextDisplayPtrScrollWidget(new), name,
                                        (ArgList)&TextDisplayPtrCoreArgList(new),
                                        NumberOfArgsInArgListStruct(CoreArgList) +
                                        NumberOfArgsInArgListStruct(ADBConstraintArgList) +
                                        NumberOfArgsInArgListStruct(TextArgList)
                                    );

	if (TextDisplayPtrWidget(new) != (Widget)NULL) {
	    Arg arg;

	    XtManageChild(TextDisplayPtrWidget(new));
	    XtUninstallTranslations(TextDisplayPtrWidget(new));
	    arg.name = XmNfontList;
	    arg.value = (XtArgVal)&TextDisplayPtrFontList(new);
	    XtGetValues(TextDisplayPtrWidget(new), &arg, 1);
	}
        return new;
}

/********************************
 *
 *      LoadTextDisplay
 *
 ********************************/

Boolean
LoadTextDisplay(textdisplay, filename)
        TextDisplayPtr  textdisplay;
        char            *filename;
{
        extern  FileInfoPtr LoadNewFile();
	XmTextPosition lastchar;

	lastchar = XmTextGetLastPosition(TextDisplayPtrWidget(textdisplay));
	if (lastchar) lastchar--;
	XmTextSetHighlight(TextDisplayPtrWidget(textdisplay), 0, lastchar,
			    XmHIGHLIGHT_NORMAL);

        if (TextDisplayPtrFile(textdisplay) != (FileInfoPtr)NULL) {
                FreeFile(TextDisplayPtrFile(textdisplay));
        }

        if ((TextDisplayPtrFile(textdisplay) = LoadNewFile(filename)) == (FileInfoPtr)NULL) {
                return False;
        } else {
                TextCols(TextDisplayPtrTextArgList(textdisplay)) =
                        TextDisplayPtrFile(textdisplay)->widestline;
                TextValue(TextDisplayPtrTextArgList(textdisplay)) = (XtArgVal)
                        TextDisplayPtrFile(textdisplay)->data;

                XmTextSetMaxLength(TextDisplayPtrWidget(textdisplay), 
                                     TextDisplayPtrFile(textdisplay)->filesize + 1);

                XmTextSetString(TextDisplayPtrWidget(textdisplay),
                                  TextDisplayPtrFile(textdisplay)->data);
                XmTextShowPosition(TextDisplayPtrWidget(textdisplay), 0L);      /* force it */
        }

        return True;
}
