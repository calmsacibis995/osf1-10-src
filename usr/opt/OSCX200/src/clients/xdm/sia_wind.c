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
static char *BuildSystemHeader= "@(#)$RCSfile: sia_wind.c,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/08/20 15:17:52 $";
#endif		/* BuildSystemHeader */


/*
        SIA interface to xdm.
*/


#include "siad.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xm/Xm.h>
#include <X11/Xm/Display.h>
#include "dm.h"
#include "gpi.h"

static Display *dpy;
static XtAppContext context;
static Widget toplevel;


void sia_wind_init(argc, argv, d)
  int argc;
  char *argv[];
  struct display *d;
{
    int i;
    Arg arglist[30];
    Screen *scrn;
    char *name = d->name;
    Widget motif_dpy;

    XtToolkitInitialize ();
    context = XtCreateApplicationContext();
    dpy = XtOpenDisplay (context, name, "dxlogin", "DXlogin", 0, 0, &argc, argv);

    /* 
     * SecureDisplay is moved here from InitGreet() to avoid xdm killing
     * itself.
     */
    SecureDisplay (d, dpy);

    /*
     * Run the setup script - note this usually will not work when
     * the server is grabbed, so we don't even bother trying.
     * Moved here from InitGreet() to be sure the server font path
     * is set up so the Motif login widget can find its fonts.
     */
    if (!d->grabServer)
	SetupDisplay (d);

    /*
     * Disable Motif Drag and Drop
     */
    motif_dpy = XmGetXmDisplay(dpy);
    i = 0;
    XtSetArg(arglist[i], XmNdragInitiatorProtocolStyle, XmDRAG_NONE); i++;
    XtSetArg(arglist[i], XmNdragReceiverProtocolStyle, XmDRAG_NONE); i++;
    XtSetValues(motif_dpy, arglist, i);

    MrmInitialize();

    i = 0;
    scrn = DefaultScreenOfDisplay(dpy);
    XtSetArg(arglist[i], XmNmwmDecorations, 0); i++;
    XtSetArg(arglist[i], XmNmwmFunctions, 0); i++;
    XtSetArg(arglist[i], XmNoverrideRedirect, True); i++;
    XtSetArg(arglist[i], XmNallowResize, True); i++;
    XtSetArg(arglist[i], XmNborderWidth, 0); i++;
    XtSetArg(arglist[i], XmNmarginWidth, 0); i++;
    XtSetArg(arglist[i], XmNmarginHeight, 0); i++;
    XtSetArg(arglist[i], XtNscreen, scrn); i++;
    XtSetArg(arglist[i], XtNargc, argc); i++;
    XtSetArg(arglist[i], XtNargv, argv); i++;
    XtSetArg(arglist[i], XmNx, XWidthOfScreen( scrn) / 2); i++;
    XtSetArg(arglist[i], XmNy, XHeightOfScreen( scrn) / 2); i++;
    XtSetArg(arglist[i], XmNwidth, 1); i++;
    XtSetArg(arglist[i], XmNheight, 1); i++;
    XtSetArg(arglist[i], XmNmappedWhenManaged, False); i++;
    toplevel = XtAppCreateShell ((String) NULL, "DXlogin", 
        applicationShellWidgetClass, dpy, arglist, i);
    if ( GpiAbort == GpiEstablishShell(toplevel))
        fprintf(stderr,  "Failed to establish a GPI top level shell.\n");
}


void sia_wind_getdpy(
  Display **pdpy, 
  XtAppContext *pcontext, 
  Widget       *ptoplevel)
{
    *pdpy = dpy;
    *pcontext = context;
    *ptoplevel = toplevel;
}


void sia_wind_term()
{
    Display	*display;

    display = XtDisplay (toplevel);
    XtDestroyWidget (toplevel);
    XtCloseDisplay (display);
}


/* collect the SIA parameters from a window system. */

int sia_wind_collector(
  unsigned long timeout, 
  int rendition, 
  unsigned char *client_title, 
  XFontStruct* client_title_font,
  Pixel client_title_color,
  unsigned char *title, 
  XFontStruct* title_font,
  Pixel title_color,
  int num_prompts, 
  prompt_t pmpts[],
  XFontStruct* prompt_font,
  Pixel prompt_color,
  XFontStruct* answer_font,
  Pixel answer_color)
{
    int i, multiple;
    Boolean visible[MAX_PROMPTS];
    Screen *scrn;
    prompt_t *p;
    unsigned char *prompt_string[MAX_PROMPTS];
    unsigned char *result_string[MAX_PROMPTS];
    int max_result_length[MAX_PROMPTS];
    int min_result_length[MAX_PROMPTS];
    int control_flags[MAX_PROMPTS];

    scrn = DefaultScreenOfDisplay(dpy);
    XWarpPointer(dpy, None, RootWindowOfScreen (scrn), 0, 0, 0, 0, 
        WidthOfScreen(scrn) / 2, HeightOfScreen(scrn) / 2);

    for(i=0;i<MAX_PROMPTS;i++) {
        p = (prompt_t *)&pmpts[i];
        visible[i] = (p->control_flags & SIARESINVIS) ? False : True;
        prompt_string[i] = p->prompt;
        result_string[i] = p->result;
        max_result_length[i] = p->max_result_length;
        min_result_length[i] = p->min_result_length;
        control_flags[i] = p->control_flags;
    };

    switch(rendition) {
        case SIAMENUONE:
            GpiList(toplevel, timeout, title, client_title, prompt_string,
                num_prompts, result_string, max_result_length, 0);
            break;
        case SIAMENUANY:
            GpiList(toplevel, timeout, title, client_title, prompt_string,
                num_prompts, result_string, max_result_length, 1);
            break;
        case SIAFORM:
            GpiForm(toplevel, timeout, client_title, client_title_font,
                client_title_color, title, title_font, title_color, num_prompts,
                prompt_string, result_string, prompt_font, prompt_color,
                answer_font, answer_color, max_result_length, visible);
            break;
        case SIAONELINER:
            GpiForm(toplevel, timeout, client_title, client_title_font,
                client_title_color, title, title_font, title_color, num_prompts,
                prompt_string, result_string, prompt_font, prompt_color,
                answer_font, answer_color, max_result_length, visible);
            break;
        case SIAINFO:
        case SIAWARNING:
            GpiDisplayMessage(toplevel, timeout, 1, prompt_string[0]);
            break;
        default:
            return(SIACOLABORT);
            break;
    }
    return(SIACOLSUCCESS);
}


sia_wind_error(errmsg, timeout)
  char *errmsg;
  unsigned long timeout;
{
    Screen      *scrn;

    scrn = DefaultScreenOfDisplay(dpy);
    XWarpPointer(dpy, None, RootWindowOfScreen (scrn), 0, 0, 0, 0, 
        WidthOfScreen(scrn) / 2, HeightOfScreen(scrn) / 2);
    GpiDisplayMessage(toplevel, timeout, 1, (unsigned char *)errmsg);
}
