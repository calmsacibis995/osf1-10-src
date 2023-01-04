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
static char *rcsid = "@(#)$RCSfile: motifgif.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 17:56:04 $";
#endif
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: motifgif.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 17:56:04 $"
#endif
#endif

/****************************************************************************
 ****************************************************************************
 **
 **   File:     motifgif.c
 **
 **   Project:     Motif - gif file display program
 **                Written by Peter Levine, OSF.
 **
 **   NOTE:        This program is boiler-plate which calls pict
 **                to actually display the picture...
 **
 **
 ****************************************************************************
 ****************************************************************************/

/***************************************************
*                                                  *
*  Revision history:                               *
*                                                  *
*  07/4/89      pjlevine
*                                                  *
****************************************************/


#include "motifgif.h"
#define STRING_OFFSET 14
int dialog_up = 1;
static XtResourceList resource;
char filename [256];

/***********************************************************/
void  Quit(i)
     int i;
{
     printf("exiting...\n\r");
     fflush(stdout);
     exit(0);
}

/***********************************************************/
/* Quit                             */
/**********************************************************/
void MenuQuit (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;
{


    exit (0);


}

/***********************************************************/
/* displayed on file selection                             */
/**********************************************************/
void ShowFiles (w, client_data, call_data)
Widget   w;
XtPointer client_data;
XtPointer call_data;
{


    XtManageChild (FileDialog);
    XtManageChild (FileSelection);


}

#ifdef VMS
#include <descrip.h>
#define CLI$M_NOWAIT 1
void vms_spawn(command, nowait)
String command;
Boolean nowait;
{
    int status;
    int flags;
    char error_text[80];
    struct dsc$descriptor cmddsc;

    if (nowait) {
        flags = CLI$M_NOWAIT;
    } else {
        flags = 0;
    }

    cmddsc.dsc$b_class = DSC$K_CLASS_S;
    cmddsc.dsc$b_dtype = DSC$K_DTYPE_T;
    cmddsc.dsc$w_length = strlen(command);
    cmddsc.dsc$a_pointer = command;

    status = lib$spawn(&cmddsc,0,0,&flags);
    if (!(status & 1)) {
        printf(error_text,"Exec failed. Status = %d",status);
    }
    return;
}
#endif /* VMS */

/***********************************************************/
/* displayed on OK file selection                          */
/***********************************************************/
void FileSelectOK (w, client_data, callback_data)
Widget    w;
XtPointer client_data;
XtPointer callback_data;

{
  char *textstr, *extract_normal_string ();
  int i;
  char *buffer, *GetSource ();
  char *filebuff;

  filebuff = (char *) calloc (1, 256);

  textstr = extract_normal_string (((XmSelectionBoxCallbackStruct *)callback_data)->value);

#ifdef VMS
  strcpy (filebuff, "@pict.com \"-nsc\" ");
  strcat (filebuff, textstr);
#else
  strcpy (filebuff, "pict -nsc ");
  strcat (filebuff, textstr);
  strcat (filebuff, " &");
#endif

#ifdef VMS
  vms_spawn(filebuff,True);
#else
  system (filebuff); 
#endif
}

/***********************************************************/
Widget CreateCascade (label, mnemonic, submenu, parent)
char       *label,
           mnemonic;
Widget     submenu,
           parent;

{

  Widget   widget;
  int      n;
  Arg      args[MAX_ARGS];
  XmString tcs;

  n = 0;
  tcs = XmStringLtoRCreate (label, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, mnemonic); n++;
  XtSetArg(args[n], XmNsubMenuId, submenu); n++;
  widget = XmCreateCascadeButton (parent, "cascade", args, n);
  XtManageChild(widget);
  XmStringFree(tcs);

  return (widget);
}

/***********************************************************/
Widget CreateDialogShell (parent, say, x, y)
Widget parent;
char *say;
Cardinal x, y;
{

  static   Widget   widget;
  int      n;
  Arg      args[MAX_ARGS];

  n = 0;
  XtSetArg(args[n], XmNwidth, 400);  n++;
  XtSetArg(args[n], XmNheight, 400);  n++;
  XtSetArg(args[n], XmNx, x);  n++;
  XtSetArg(args[n], XmNy, y);  n++;
  widget = XmCreateDialogShell (parent, say, args, n);


  return (widget);
}

/***********************************************************/
Widget CreateFileSelection (parent, name)
Widget parent;
char *name;
{

  static   Widget   widget;
  int      n;
  Arg      args[MAX_ARGS];
  XmString tcs;

  n = 0;

  tcs = XmStringLtoRCreate ("*.gif", XmSTRING_DEFAULT_CHARSET);
  XtSetArg(args[n], XmNdirMask, tcs); n++;
  XtSetArg(args[n], XmNdefaultPosition, False); n++;
  XtSetArg(args[n], XmNwidth, 450); n++;
  XtSetArg(args[n], XmNheight, 450); n++;
  widget = XmCreateFileSelectionBox (parent, name, args, n);
  XmStringFree(tcs);

  XtAddCallback (widget, XmNokCallback, FileSelectOK, NULL);

  return (widget);
}

/***********************************************************/
Widget CreatePushButton (label, mnemonic, parent)
char      *label;
KeySym    mnemonic;
Widget    parent;

{

  static Widget       widget;
  int          n;
  Arg          args[MAX_ARGS];
  XmString     tcs,
               acc_text;

  n = 0;
  tcs = XmStringLtoRCreate(label, XmSTRING_DEFAULT_CHARSET);
  acc_text = XmStringLtoRCreate("^A", XmSTRING_DEFAULT_CHARSET);
  XtSetArg(args[n], XmNmnemonic, mnemonic); n++;
  /* if default, extra border */
  if (bit_flag & DEFAULT)
  {
      XtSetArg(args[n], XmNshowAsDefault, 1); n++;
  }
  /* if there is an accelerator associated with the button */
  if (bit_flag & ACCEL)
  {
      XtSetArg(args[n], XmNaccelerator, "Ctrl<Key>A"); n++;
      XtSetArg(args[n], XmNacceleratorText, acc_text);n++;
  }
  /* if the button is inactive; greyed out */
  if (bit_flag & INACTIVE)
  {
      XtSetArg(args[n], XmNsensitive, False); n++;
  }

  widget = XmCreatePushButton(parent, label, args, n);
  XtManageChild(widget);
  XmStringFree(tcs);
  if (bit_flag & ACCEL)
    XmStringFree(acc_text);
  bit_flag = BITSOFF;  

  return(widget);
}


/***************************************************************/
#ifdef VMS
int  main(argc, argv)
#else
void  main(argc, argv)
#endif
int     argc;
char    **argv;
{
    register int           n;
    Arg           args[MAX_ARGS];
    int           num_items;

    Widget Dummy;
    Widget XmCreateScrolledText ();
    /* this will be the contents of the dialog selection box */
    XmString *addrstr, *GetWidgetHierarchy ();

	XtAppContext app;


    MenuBar = (Widget) NULL;

    signal(SIGHUP,  Quit);
    signal(SIGINT,  Quit);
    signal(SIGQUIT, Quit);


    XtToolkitInitialize();
	app = XtCreateApplicationContext();
    if ((display = XtOpenDisplay (app, NULL, argv[1], "XMclient",
				  NULL, 0, &argc, argv)) == NULL)
    {
	fprintf (stderr,"\n%s:  Can't open display\n", argv[0]);
        exit(1);
    }


    strcpy (filename, argv[0]);
#ifndef VMS
    strcat (filename, ".c");
#endif
    printf ("filename = %s\n", filename);
    n = 0;
    XtSetArg(args[n], XmNwidth,  WIDTH);  n++;
    XtSetArg(args[n], XmNheight, HEIGHT);  n++;
    XtSetArg(args[n], XmNallowShellResize, True);  n++;
    Shell = XtAppCreateShell(argv[0], NULL, applicationShellWidgetClass,
                              display, args, n);

    n = 0;
    MainWindow = XmCreateMainWindow(Shell, "MainWindow", args, n);
    XtManageChild(MainWindow);

    WorkRegion = XmCreateBulletinBoard (MainWindow, "s_text", args, n);
    XtManageChild (WorkRegion);

    CreateMenuBar ();

    FileDialog = CreateDialogShell (Shell, "Picture Selection Window",300,300);
    FileSelection = CreateFileSelection (FileDialog, "Select File"); 

    XtRealizeWidget(Shell);

    XtAppMainLoop(app);

}

/***********************************************************/
CreateMenuBar ()
{

    int           n;
    Arg           args[MAX_ARGS];


    /*
    **  the menubar
    */

    n = 0;
    MenuBar = (Widget) XmCreateMenuBar ((Widget) MainWindow, "MenuBar", 
					args, n); 
    if (MenuBar == NULL) {
      printf ("null menu bar\n\r");
      exit (0);
    }
    XtManageChild(MenuBar);

    /*  create the first level pulldowns */

    n = 0;
    PullDown1 = XmCreatePulldownMenu (MenuBar, "File", args, n);
    PullDown2 = XmCreatePulldownMenu (MenuBar, "Show", args, n);

    /*  create cascade buttons for menubar  */

    MenuBtn1 = CreateCascade ("File", 'F', PullDown1, MenuBar);
    MenuBtn2 = CreateCascade ("Show", 'S', PullDown2, MenuBar);


    /*
    **  Menu1
    */

    Label1A = CreatePushButton ("Quit",'1',PullDown1);
    XtAddCallback (Label1A, XmNactivateCallback, MenuQuit, NULL);
    Label2A = CreatePushButton ("Show Files",'1',PullDown2);
    XtAddCallback (Label2A, XmNactivateCallback, ShowFiles, NULL);
}


/* support routine to get normal string from XmString */
char *extract_normal_string(cs)
XmString cs;
{

  XmStringContext context;
  XmStringCharSet charset;
  XmStringDirection direction;
  Boolean separator;
  static char *primitive_string;

  XmStringInitContext (&context,cs);
  XmStringGetNextSegment (context,&primitive_string,
			  &charset,&direction,&separator);
  XmStringFreeContext (context);
  return ((char *) primitive_string);
}
