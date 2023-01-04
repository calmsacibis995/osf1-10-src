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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/dxdiff.c,v 1.1.4.2 1993/07/30 19:50:19 Lynda_Rice Exp $";	/* BuildSystemHeader */
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
 *	dxdiff.c - the main body!
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 22nd April 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	25th July 1988	Laurence P. G. Cable
 *
 *	Added John Hoffman's 'merged utility' changes
 *
 *	16 Jan 1990	Colin Prosser
 *
 *	Fixed typo in lineNumberForeground resource name, so it
 *	and -lnfg option now work.  Clears UWS QAR 01560.
 *
 */

static char sccsid[] = "@(#)dxdiff.c	1.10	19:00:26 1/16/90";
 
#include <fcntl.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <X11/StringDefs.h>

#include <sys/types.h>
#include <sys/stat.h>
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
#include "displaymenu.h"

extern DxDiffDisplayPtr CreateDxDiffDisplay();


/* application options */

struct _application_options app_options;



int	defaultwidth,defaultheight;



XtResource resources[] = {
	{
		"slaveVerticalScrolling",
		"",
		XtRBoolean,
		sizeof (Boolean),
		XtOffset(application_options_ptr,slaveverticalscrolling),
		XtRString,
		"on"
	},
	{
		"slaveHorizontalScrolling",
		"",
		XtRBoolean,
		sizeof (Boolean),
		XtOffset(application_options_ptr,slavehorizontalscrolling),
		XtRString,
		"on"
	},
	{
		"displayLineNumbers",
		"",
		XtRBoolean,
		sizeof (Boolean),
		XtOffset(application_options_ptr,displaylinenumbers),
		XtRString,
		"off"
	},
	{
		"drawDiffsAsLines",
		"",
		XtRBoolean,
		sizeof (Boolean),
		XtOffset(application_options_ptr,drawdiffsaslines),
		XtRString,
		"on"
	},
	{
		"debug",
		"",
		XtRBoolean,
		sizeof (Boolean),
		XtOffset(application_options_ptr,Debug),
		XtRString,
		"off"
	},
	{
		"lineNumberForeground",
		"",
		XtRPixel,
		sizeof (app_options.linenumberforeground.pixel),
		XtOffset(application_options_ptr,linenumberforeground.pixel),
		XtRString,
		"black"
	}
};
int	   noofresources = sizeof (resources) / sizeof (resources[0]);

static XrmOptionDescRec options[] = {
	{
		"-sv",
		"slaveVerticalScrolling",
		XrmoptionNoArg,
		"off"
	},
	{
		"+sv",
		"slaveVerticalScrolling",
		XrmoptionNoArg,
		"on"
	},
	{
		"-sh",
		"slaveHorizontalScrolling",
		XrmoptionNoArg,
		"off"
	},
	{
		"+sh",
		"slaveHorizontalScrolling",
		XrmoptionNoArg,
		"on"
	},
	{	"-ln",
		"displayLineNumbers",
		XrmoptionNoArg,
		"off"
	},
	{
		"+ln",
		"displayLineNumbers",
		XrmoptionNoArg,
		"on"
	},
	{	"-dl",
		"drawDiffsAsLines",
		XrmoptionNoArg,
		"off"
	},
	{
		"+dl",
		"drawDiffsAsLines",
		XrmoptionNoArg,
		"on"
	},
	{
		"-debug",
		"debug",
		XrmoptionNoArg,
		"off"
	},
	{
		"+debug",
		"debug",
		XrmoptionNoArg,
		"on"
	},
	{
		"-lnfg",
		"lineNumberForeground",
		XrmoptionSepArg,
		""
	}
};


char			**cmdlinefilelist;
int			cmdlinenoffiles;
Widget			toplevel;	/* now global */
XtAppContext		app_context ;
CoreArgList		initcore;
DxDiffDisplayPtr	maindxdiffdisplay;

#ifdef HYPERHELP

Opaque	help_context;

/* Error processing function for HyperHelp system */

void help_error(problem_string, status)
    char    *problem_string;
    int     status;

{
    fprintf(stderr, "%s, %x\n", problem_string, status);
    exit(0);
}

#endif

#ifdef	COMBINE
diff_main(c, v, e)
#else
main(c, v, e)
#endif
	int	c;
	char	**v,
		**e;
{
	short	w,h;
	Arg	wandh[2];
	
#ifdef HYPERHELP
	MrmInitialize();
	DXmInitialize();
#endif

	toplevel = XtInitialize(DXDIFFNAME, DXDIFFCLASS, options, 
				sizeof (options) / sizeof (options[0]),
				&c, v);

#ifdef R5_XLIB
	/*
	 * Set a locale for the toolkit
	 */
	app_context = XtWidgetToApplicationContext(toplevel) ;
	XtSetLanguageProc(app_context, NULL, app_context) ;
#endif


	XtGetApplicationResources(toplevel, (XtPointer)&app_options, 
		resources, noofresources, (ArgList)NULL, 0);

	dxdiffIconCreate(toplevel);

	fcntl(ConnectionNumber(XtDisplay(toplevel)), F_SETFD, 0x1);

	/* create the display */

	InitCoreArgList(initcore);

	defaultwidth = WidthOfScreen(DefaultScreenOfDisplay(XtDisplay(toplevel))) / 2;
	defaultheight = HeightOfScreen(DefaultScreenOfDisplay(XtDisplay(toplevel))) / 2;

	XtSetArg(wandh[0], XmNwidth, &w);
	XtSetArg(wandh[1], XmNheight, &h);
	XtGetValues(toplevel, wandh, 2);

	CoreWidth(initcore) = (w == 0) ? defaultwidth : w;
	CoreHeight(initcore) = (h == 0) ? defaultheight : h;
	CoreDestroyCallBack(initcore) = NULL;

	maindxdiffdisplay = (DxDiffDisplayPtr)CreateNewDxDiffDisplay(
		toplevel, "dxdiffdisplay", &initcore);

	XtRealizeWidget(toplevel);

	if (c > 2) {	/* got files on cmd line! */
		cmdlinefilelist = v;
		cmdlinenoffiles = c - 1;

		if (DoDiffs(maindxdiffdisplay, v[c - 2], v[c - 1])) {
			LoadDiffs(maindxdiffdisplay, v[c - 2], v[c - 1], 
				  DxDiffDisplayPtrDiffList(maindxdiffdisplay)->edchead,
				  DxDiffDisplayPtrDiffList(maindxdiffdisplay)->edctail);
		} else {	/* oops - error */
			ReportDiffErrors(maindxdiffdisplay);
		}
	}

#ifdef HYPERHELP
	DXmHelpSystemOpen(&help_context, toplevel, dxdiff_help, help_error,
			  "Help System Error");
#endif

	XtMainLoop();
}
