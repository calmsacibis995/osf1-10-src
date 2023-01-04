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
static char *rcsid = "@(#)$RCSfile: pict.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 18:02:25 $";
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
static char rcsid[] = "$RCSfile: pict.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 18:02:25 $"
#endif
#endif
/*
 * pict.c - displays GIF pictures on an X11 display using MOTIF
 *
 *  Author:    John Bradley, University of Pennsylvania
 *                (bradley@cis.upenn.edu)
 *             Peter Levine, Open Sofware Foundation
 *                (ported to MOTIF 7/6/89)
 *                - a midnight hack, 
 */

#define MAIN
#include "pict.h"
#define LABEL1 "Quit"
#define LABEL2 "Copy"
#define HEIGHT 500
#define WIDTH 500
char  current_file [256];
Pixmap pixmap;
int child_proc = 0;

/***********************************************************/
/* displayed on file selection                             */
/**********************************************************/
void MenuQuit (w, client_data, call_data)
Widget   w;
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

ForkANewOne (thefile)
char *thefile;
{
    char  cmd [256];

#ifdef VMS
    switch (vfork ()) 
#else
    switch (fork ()) 
#endif
      {
      case -1:
	printf ("error in fork\n");
	break;
      case 0:
	strcpy (cmd, "pict -nsc ");
	strcat (cmd, thefile);

	printf ("in exec, file = %s\n", thefile);
	printf ("in exec, cmd = %s\n", cmd);
	
	execlp ("pict", "pict", "-nsc", thefile, (char *) 0);
	break;
      default:
	printf ("in parent\n");
	break;
      }
}
/***********************************************************/
/* displayed on copy Image - Not used here                 */
/**********************************************************/
void CopyImage (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;
{

    register int           n;
    Arg           args[MAX_ARGS];


    ForkANewOne (current_file);
}

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

  textstr = extract_normal_string(((XmSelectionBoxCallbackStruct *)callback_data)->value);

  strcpy (current_file, textstr);

  ForkANewOne (current_file);
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
  widget = XmCreateFileSelectionBox (parent, name, args, n);
  XmStringFree(tcs);

  XtAddCallback (widget, XmNokCallback, FileSelectOK, NULL);

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

/*******************************************/
main(argc, argv)
    int   argc;
    char *argv[];
/*******************************************/
{
    int        i;
    char      *display, *geom, *fname;
    XEvent     event;
    register int           n;
    Arg           args[MAX_ARGS];

	XtAppContext app;

    cmd = argv[0];
    display = geom = fname = NULL;
    expImage = NULL;

    expand = 1;  strip = 0;  nostrip = 0;

    /*********************Options*********************/

    for (i = 1; i < argc; i++) {
        char *strind;

        if (!strncmp(argv[i],"-g",2)) {		/* geometry */
            i++;
            geom = argv[i];
            continue;
            }

        if (argv[i][0] == '=') {		/* old-style geometry */
            geom = argv[i];
            continue;
            }

        if (!strncmp(argv[i],"-d",2)) {		/* display */
            i++;
            display = argv[i];
            continue;
            }

#ifndef VMS
        strind = index(argv[i], ':');		/* old-style display */
        if(strind != NULL) {
            display = argv[i];
            continue;
            }
#endif

        if (!strcmp(argv[i],"-e")) {		/* expand */
            i++;
            expand=atoi(argv[i]);
            continue;
            }

        if (!strcmp(argv[i],"-s")) {		/* strip */
            i++;
            strip=atoi(argv[i]);
            continue;
            }

        if (!strcmp(argv[i],"-ns")) {		/* nostrip */
            nostrip++;
            continue;
            }

        if (!strcmp(argv[i],"-nsc")) {		/* nostrip child*/
            nostrip++;
	    child_proc = 0;
            continue;
            }

        if (argv[i][0] != '-') {		/* the file name */
            fname = argv[i];
            continue;
            }

        Syntax(cmd);
    }

    if (fname==NULL) fname="-";
    if (expand<1 || expand>MAXEXPAND) Syntax(cmd);
    if (strip<0 || strip>7) Syntax(cmd);

    /*****************************************************/

    /* Open up the display. */

    XtToolkitInitialize();
	app = XtCreateApplicationContext();
    if ((theDisp = XtOpenDisplay (app, NULL, argv[1], "XMclient",
				  NULL, 0, &argc, argv)) == NULL)
    {
	fprintf (stderr,"\n%s:  Can't open display\n", argv[0]);
        exit(1);
    }
/*
    if ( (theDisp=XOpenDisplay(display)) == NULL) {
        fprintf(stderr, "%s: Can't open display\n",argv[0]);
        exit(1);
        }
*/
    theScreen = DefaultScreen(theDisp);
    theCmap   = DefaultColormap(theDisp, theScreen);
    rootW     = RootWindow(theDisp,theScreen);
    theGC     = DefaultGC(theDisp,theScreen);
    fcol      = WhitePixel(theDisp,theScreen);
    bcol      = BlackPixel(theDisp,theScreen);
    theVisual = DefaultVisual(theDisp,theScreen);

    dispcells = DisplayCells(theDisp, theScreen);
    if (dispcells<=2) 
        FatalError("This program requires a color display, pref. 8 bits.");


    /****************** Open/Read the File  *****************/
    strcpy (current_file, fname);
    LoadGIF(fname);
    iWIDE = theImage->width;  iHIGH = theImage->height;

    eWIDE = iWIDE * expand;  eHIGH = iHIGH * expand;
    if (eWIDE > DisplayWidth(theDisp,theScreen)) 
        eWIDE = DisplayWidth(theDisp,theScreen);
    if (eHIGH > DisplayHeight(theDisp,theScreen)) 
        eHIGH = DisplayHeight(theDisp,theScreen);

    /**************** Create/Open X Resources ***************/
    if ((mfinfo = XLoadQueryFont(theDisp,"variable"))==NULL)
       FatalError("couldn't open 'variable' font\n");
    mfont=mfinfo->fid;
    XSetFont(theDisp,theGC,mfont);
    XSetForeground(theDisp,theGC,fcol);
    XSetBackground(theDisp,theGC,bcol);


    n = 0;
    XtSetArg(args[n], XmNwidth,  WIDTH);  n++;
    XtSetArg(args[n], XmNheight, HEIGHT);  n++;
    XtSetArg(args[n], XmNallowShellResize, True);  n++;
    Shell = XtAppCreateShell(argv[0], NULL, applicationShellWidgetClass,
                              theDisp, args, n);


    n = 0;
    MainWindow = XmCreateMainWindow(Shell, "MainWindow", args, n);
    XtManageChild(MainWindow);

    WorkRegion = XmCreateBulletinBoard (MainWindow, "s_text", args, n);
    XtManageChild (WorkRegion);
    
    if (!child_proc)
      CreateMenuBar ();

    n = 0;
    XtSetArg(args[n], XmNx,  10);  n++;
    XtSetArg(args[n], XmNy,  10);  n++;
    XtSetArg(args[n], XmNheight,  eHIGH + 50);  n++;
    XtSetArg(args[n], XmNwidth,  eWIDE + 50);  n++;
    XtSetArg(args[n], XmNshadowThickness,  2);  n++;
    MainBoard = XmCreateBulletinBoard (WorkRegion, "s_text", args, n);
    XtManageChild (MainBoard);

    n = 0;
    XtSetArg(args[n], XmNheight,  eHIGH + 50);  n++;
    XtSetArg(args[n], XmNwidth,  eWIDE + 50);  n++;

    XtRealizeWidget(Shell);

    DrawingArea = XmCreateDrawingArea (MainBoard, "d_area", args, n);
    XtCreateWindow (DrawingArea, InputOutput, CopyFromParent, 0L, 0);
    pixmap = XCreatePixmap (theDisp, XtWindow (DrawingArea), eWIDE, 
			    eHIGH, 
			    (unsigned int)DefaultDepth(theDisp, theScreen));
    Resize(eWIDE,eHIGH);

    DrawWindow(0, 0, eWIDE, eHIGH);

    n = 0;
    XtSetArg(args[n], XmNx,  20);  n++;
    XtSetArg(args[n], XmNy,  20);  n++;
    XtSetArg(args[n], XmNheight,  eHIGH);  n++;
    XtSetArg(args[n], XmNwidth,  eWIDE);  n++;
    XtSetArg(args[n], XmNlabelType,  XmPIXMAP);  n++;
    XtSetArg(args[n], XmNlabelPixmap,  pixmap);  n++;
    XtSetArg(args[n], XmNlabelInsensitivePixmap,  pixmap);  n++;

    Label = XmCreateLabel (MainBoard, "label", args, n);

    XtManageChild (Label);

/*    FileDialog = CreateDialogShell (Shell, "File View Window", 300, 300);
    FileSelection = CreateFileSelection (FileDialog, "Select File"); 
*/

    XtAppMainLoop(app);

}

/***********************************************************/
GetFile (fname)
char *fname;

{
    LoadGIF(fname);
    iWIDE = theImage->width;  iHIGH = theImage->height;

    eWIDE = iWIDE * expand;  eHIGH = iHIGH * expand;
    if (eWIDE > DisplayWidth(theDisp,theScreen)) 
        eWIDE = DisplayWidth(theDisp,theScreen);
    if (eHIGH > DisplayHeight(theDisp,theScreen)) 
        eHIGH = DisplayHeight(theDisp,theScreen);
}

/***********************************************************/
DisplayImage ()
{
    Resize(eWIDE,eHIGH);

    DrawWindow(0, 0, eWIDE, eHIGH);
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
Widget CreatePushButton (label, mnemonic, parent)
char      *label,
          mnemonic;
Widget    parent;

{

  static Widget       widget;
  int          n;
  Arg          args[MAX_ARGS];
  XmString     tcs,
               acc_text;

  n = 0;
  tcs = XmStringLtoRCreate(label, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, mnemonic); n++;
  widget = XmCreatePushButton(parent, label, args, n);
  XtManageChild(widget);
  XmStringFree(tcs);

  return(widget);
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

    /*  create cascade buttons for menubar  */

    MenuBtn1 = CreateCascade ("File", 'F', PullDown1, MenuBar);

    /*
    **  Menu1
    */

    Label1A = CreatePushButton (LABEL1,'1',PullDown1);
    XtAddCallback (Label1A, XmNactivateCallback, MenuQuit, NULL);
/*
    Label1B = CreatePushButton (LABEL2, '2', PullDown1);
    XtAddCallback (Label1B, XmNactivateCallback, CopyImage, NULL);
*/
}


/****************/
HandleEvent(event)
    XEvent *event;
/****************/
{
    switch (event->type) {
        case Expose: {
            XExposeEvent *exp_event = (XExposeEvent *) event;

            if (exp_event->window==mainW) 
                DrawWindow(exp_event->x,exp_event->y,
                           exp_event->width, exp_event->height);
            }
            break;

        case KeyPress: {
            XKeyEvent *key_event = (XKeyEvent *) event;
            char buf[128];
            KeySym ks;
            XComposeStatus status;

            XLookupString(key_event,buf,128,&ks,&status);
            if (buf[0]=='q' || buf[0]=='Q') Quit();
            }
            break;

        case ConfigureNotify: {
            XConfigureEvent *conf_event = (XConfigureEvent *) event;

            if (conf_event->window == mainW && 
                 (conf_event->width != eWIDE || conf_event->height != eHIGH))
                Resize(conf_event->width, conf_event->height);
            }
            break;


        case CirculateNotify:
        case MapNotify:
        case DestroyNotify:
        case GravityNotify:
        case ReparentNotify:
        case UnmapNotify:       break;

        default:		/* ignore unexpected events */
	  break;
        }  /* end of switch */
}


/***********************************/
Syntax()
{
    printf("Usage: %s filename [[-geometry] geom] [[-display] display]\n",cmd);
    printf("       [-e 1..%d] [-s 0-7] [-ns]\n",MAXEXPAND);
    exit(1);
}


/***********************************/
FatalError (identifier)
       char *identifier;
{
    fprintf(stderr, "%s: %s\n",cmd, identifier);
    exit(-1);
}


/***********************************/
Quit()
{
    exit(0);
}


/***********************************/
DrawWindow(x,y,w,h)
{
    int i;

    i = 0;

    XPutImage(theDisp, pixmap, theGC, expImage,x,y,x,y,w,h);
}


/***********************************/
CreateMainWindow(name,geom,argc,argv)
    char *name,*geom,**argv;
    int   argc;
{
    XSetWindowAttributes xswa;
    unsigned int xswamask;
    XSizeHints hints;
    int i,x,y,w,h;

    x=y=w=h=1;
    i=XParseGeometry(geom,&x,&y,(unsigned int*)&w,(unsigned int*)&h);
    if (i&WidthValue)  eWIDE = w;
    if (i&HeightValue) eHIGH = h;

    if (i&XValue || i&YValue) hints.flags = USPosition;  
                         else hints.flags = PPosition;

    hints.flags |= USSize;

    if (i&XValue && i&XNegative) 
        x = XDisplayWidth(theDisp,theScreen)-eWIDE-abs(x);
    if (i&YValue && i&YNegative) 
        y = XDisplayHeight(theDisp,theScreen)-eHIGH-abs(y);

    hints.x=x;             hints.y=y;
    hints.width  = eWIDE;  hints.height = eHIGH;
    hints.max_width = DisplayWidth(theDisp,theScreen);
    hints.max_height = DisplayHeight(theDisp,theScreen);
    hints.flags |= PMaxSize;

    xswa.background_pixel = bcol;
    xswa.border_pixel     = fcol;
    xswamask = CWBackPixel | CWBorderPixel;

    mainW = XCreateWindow(theDisp,rootW,x,y,eWIDE,eHIGH,2,0,CopyFromParent,
                          CopyFromParent, xswamask, &xswa);

    XSetStandardProperties(theDisp,mainW,"PictureView","PictureView",None,
                            argv,argc,&hints);

    if (!mainW) FatalError("Can't open main window");

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
