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
static char *rcsid = "@(#)$RCSfile: sia_wind_xdm.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/08/26 17:24:24 $";
#endif

/*
   xdm customizations to sia routines.
*/

#include "siad.h"
#include "gpi.h"
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>


typedef struct _LoginOptionRec
    {
    String	greeting;
    XFontStruct	*greetingFont;
    Pixel	greetingColor;
    XFontStruct	*promptFont;
    Pixel	promptColor;
    XFontStruct	*answerFont;
    Pixel	answerColor;
    } LoginOptionsRec;

#define XmNgreeting		"greeting"
#define XmCGreeting		"Greeting"
#define XmNgreetingFont		"greetingFont"
#define XmCGreetingFont		"GreetingFont"
#define XmNgreetingColor	"greetingColor"
#define XmCGreetingColor	"GreetingColor"
#define XmNpromptFont		"promptFont"
#define XmCPromptFont		"PromptFont"
#define XmNpromptColor		"promptColor"
#define XmCPromptColor		"PromptColor"
#define XmNanswerFont		"answerFont"
#define XmCAnswerFont		"AnswerFont"
#define XmNanswerColor		"answerColor"
#define XmCAnswerColor		"AnswerColor"

#define Offset( field ) XtOffsetOf( LoginOptionsRec, field )

static XtResource loginResources[] =
 {
    { XmNgreeting, XmCGreeting, XtRString, sizeof(String),
	Offset(greeting), XtRString, NULL
    },
    { XmNgreetingFont, XmCGreetingFont, XtRFontStruct, sizeof(XFontStruct *),
	Offset(greetingFont), XtRString, 
	"*-new century schoolbook-bold-i-normal-*-240-*"
    },
    { XmNgreetingColor, XmCGreetingColor, XtRPixel, sizeof(Pixel),
	Offset(greetingColor), XtRString, "Black"
    },
    { XmNpromptFont, XmCPromptFont, XtRFontStruct, sizeof(XFontStruct *),
	Offset(promptFont), XtRString, 
        "*-new century schoolbook-medium-r-normal-*-180-*"
    },
    { XmNpromptColor, XmCPromptColor, XtRPixel, sizeof(Pixel),
	Offset(promptColor), XtRString, "Black"
    },
    { XmNanswerFont, XmCAnswerFont, XtRFontStruct, sizeof(XFontStruct *),
	Offset(answerFont), XtRString, 
        "*-new century schoolbook-medium-r-normal-*-180-*"
    },
    { XmNanswerColor, XmCAnswerColor, XtRPixel, sizeof(Pixel),
	Offset(answerColor), XtRString, "Black"
    },

 };



/* collect the SIA parameters from a window system. */

int sia_wind_collector_greet(
  unsigned long timeout,
  int rendition,
  unsigned char *title,
  int num_prompts,
  prompt_t *prompt[])
{
    Display* dpy;
    XFontStruct* title_font;
    Pixel title_color;
    char client_title[MAXHOSTNAMELEN + 50]; 
    char *hostname[MAXHOSTNAMELEN + 1];
    XFontStruct* client_title_font;
    Pixel client_title_color;
    XFontStruct* prompt_font;
    Pixel prompt_color;
    XFontStruct* answer_font;
    Pixel answer_color;
    int result;
    LoginOptionsRec	loginOptions;


    XtGetApplicationResources(GpiShellWidget, (XtPointer)&loginOptions,
				loginResources, XtNumber(loginResources),
				NULL, 0);
    

    if (!loginOptions.greeting)
	{
	strcpy(client_title, "DEC OSF/1: \0");
	if (gethostname(hostname, MAXHOSTNAMELEN + 1) == 0)
	    strcat(client_title, hostname);
	else 
	    strcat(client_title, "unable to read hostname\0");
	loginOptions.greeting = client_title;
	}

    dpy = XtDisplay(GpiShellWidget);

    if (!loginOptions.greetingFont)
	{
	loginOptions.greetingFont = XLoadQueryFont(dpy, "fixed"); 
	if(!loginOptions.greetingFont)
	    {
	    LogError("could not get any font\n");
	    exit(1);
	    }
	}
    if (!loginOptions.promptFont)
	{
	loginOptions.promptFont = XLoadQueryFont(dpy, "fixed"); 
	if(!loginOptions.promptFont)
	    {
	    LogError("could not get any font\n");
	    exit(1);
	    }
	}
    if (!loginOptions.answerFont)
	{
	loginOptions.answerFont = XLoadQueryFont(dpy, "fixed"); 
	if(!loginOptions.answerFont)
	    {
	    LogError("could not get any font\n");
	    exit(1);
	    }
	}

    result =  sia_wind_collector(timeout, rendition, loginOptions.greeting, 
				loginOptions.greetingFont,
        			loginOptions.greetingColor, 
				title, loginOptions.greetingFont, 
				loginOptions.greetingColor, num_prompts,
        			prompt, loginOptions.promptFont, 
				loginOptions.promptColor, 
				loginOptions.answerFont, 
				loginOptions.answerColor);

    return(result);
}

