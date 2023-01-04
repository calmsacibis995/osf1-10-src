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
static char *rcsid = "@(#)$RCSfile: main.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 20:14:02 $";
#endif
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
tatic char rcsid[] = "$RCSfile: main.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 20:14:02 $"
#endif
#endif

#include "main.h"

/* ===============================================================
 *    Main: Initialize, create the application shell and loop
 */
#ifdef VMS
#ifdef _NO_PROTO
int main(argc, argv)
		int argc;
		char *argv[];
#else
int main(int argc, char *argv[])
#endif
#else
#ifdef _NO_PROTO
void main(argc, argv)
		int argc;
		char *argv[];
#else
void main(int argc, char *argv[])
#endif
#endif
{
   int save_argc;
   char ** save_argv;
   char **p1, **p2;
   int i;
   static char myClass[] = "Fileview";
   static char myName[] = "fileview" ;
   ViewPtr this;

/* 
 * init widget names
 */
   new_pane = "new_pane";
   kill_pane = "kill_pane";
   search = "search";

   /* save argc and argv */
   save_argc = argc;
   save_argv = (char **) XtCalloc(argc, sizeof(char *));
   for ( p1 = argv, p2 = save_argv, i = argc; i-- > 0 ; ) 
     *p2++ = *p1++ ;

   XtToolkitInitialize();
   theContext = XtCreateApplicationContext();
   XtSetLanguageProc(theContext, (XtLanguageProc) MyLanguageProc, theContext); 
   theDisplay = XtOpenDisplay(theContext, NULL, myName, myClass, 
			      NULL, 0, &argc, argv);

   theWidgetRoot = XtAppCreateShell(myName, myClass,
				    applicationShellWidgetClass,
				    theDisplay, NULL, 0);
 
   MrmInitialize();
   if (MrmOpenHierarchyPerDisplay(theDisplay, UIL_FILE_COUNT, uid_files,
				   NULL, &theUIDdatabase)
        != MrmSUCCESS)
       /* || (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)) */
     {
        fprintf (stderr,
                 "Cannot open hierarchy defined by %s\n", uid_files[0]);
        exit(0);
     }
   this = NewFileShell(theWidgetRoot, True, save_argc, save_argv);
   XtPopup(this->shell, XtGrabNone);
   /* XtRealizeWidget(this->text_source); */
   XtAppMainLoop(theContext);
}

/* ===============================================================
 * The language proc. Check that the language specified is 
 * suppored by libX and libC.
 * Note: Xt guarantees that xnl is not NULL. No need to test it.
 * Return a language string usable by XtResolvePathname.
 */
#ifdef _NO_PROTO
static String MyLanguageProc(dpy, xnl, theContext)
		Display * dpy;
		String xnl;
		XtAppContext theContext;
#else
static String MyLanguageProc(Display * dpy, String xnl, 
			     XtAppContext theContext)
#endif
{
#define C_locale_name "C"
   String new_locale;
   char msg[128];
   String lang = getenv("LANG");

   new_locale = setlocale(LC_ALL, xnl); 
   if (new_locale == NULL || ( ! XSupportsLocale()) ) {
      String locale_name;

      locale_name = (*xnl == '\0') ? ((lang == NULL) ?  "NULL":lang )
					: xnl;
      sprintf(msg, "Check locale: `%s' not supported by C and X library", 
	      locale_name);
      XtAppWarning(theContext, msg);
      new_locale = NULL; 
   }
   else if (! XSetLocaleModifiers(""))
     XtAppWarning(theContext,
		  "X locale modifiers not supported, using default");

   if (new_locale == NULL) {
      lang = C_locale_name;
	 XtAppWarning(theContext, 
		      "Trying to use the C locale\n");
      new_locale = setlocale(LC_ALL,  C_locale_name);
      if (new_locale == NULL || setlocale(LC_CTYPE, NULL) == NULL) {
	 XtAppWarning(theContext, 
		      "Your C library is brain damaged, brother\n");
      }
   }
   return setlocale(LC_CTYPE, NULL);
}

/* ===============================================================
 *   Create a new top level shell.
 * 	If primary is true create a quit entry,
 *	else a close entryn
 */
#ifdef _NO_PROTO
static ViewPtr NewFileShell(parent, primary, argc, argv)
		Widget parent;
		Bool primary;
		int argc;
		char *argv[];
#else
static ViewPtr NewFileShell(Widget parent, Bool primary,
			   int argc, char *argv[])
#endif
{
   Widget mw, children[4], menubar, entry;
   Arg args[20];
   String names[8];
   XtCallbackProc procs[8];
   XtPointer private[8];
   int n;
   Dimension width, height;
   ViewPtr this;
   WidgetClass class;

#define SetMenuEntry(k,l,p,d) names[k]=l;procs[k]=p;private[k]=d;

   /* alloc the object for that view */
   this = (ViewPtr) XtCalloc(sizeof(View), 1);
   n = 0;
   XtSetArg(args[n], XmNallowShellResize, True); n++;
   if (primary) {
      XtSetArg(args[n], XmNargv, argv); n++;
      XtSetArg(args[n], XmNargc, argc); n++;
      XtSetArg(args[n], XmNtitle, "File View Main"); n++;
      class = applicationShellWidgetClass;
   }
   else {
      XtSetArg(args[n], XmNtitle, "File" ); n++;
      class = topLevelShellWidgetClass;
   }
   this->shell = XtCreatePopupShell("primary", class, parent, args, n);
/*
 * Create the main window, the menubar and the pane 
 * Store the view in userData for further use.
 */
   n = 0;
   XtSetArg(args[n], XmNuserData, this); n++;
   mw = XmCreateMainWindow(this->shell, "main", args, n);
   children[0] = XmCreateMenuBar(mw, "menubar", NULL, 0);

   n = 0;
   SetMenuEntry(n, "open", (XtCallbackProc) OpenFileCallback,
		(XtPointer) this); n++;
   SetMenuEntry(n, "opennew", (XtCallbackProc) OpenNewShellCallback,
		(XtPointer) this); n++;
   SetMenuEntry(n, NULL, NULL, NULL); n++;
   if (primary) {
      SetMenuEntry(n, "exit",  (XtCallbackProc) ExitCallback,
		   (XtPointer) parent);
   }
   else {
      SetMenuEntry(n, "close",  (XtCallbackProc) CloseCallback,
		   (XtPointer) this);
   }
   n++;

   (void) CreateMenuBarEntry(children[0], "file", names, procs, private, n);
   
   n = 0;
   SetMenuEntry(n, new_pane, (XtCallbackProc) NewPaneCallback,
		(XtPointer) this); n++;
   SetMenuEntry(n, kill_pane,(XtCallbackProc) KillPaneCallback,
		(XtPointer) this); n++;
   SetMenuEntry(n, search, (XtCallbackProc) FindCallback,
		(XtPointer) this); n++;
   this->view_cascade = 
     CreateMenuBarEntry(children[0], "view", names, procs, private, n);
   SetSensitive(this->view_cascade, new_pane, False);
   SetSensitive(this->view_cascade, kill_pane, False);
   SetSensitive(this->view_cascade, search, False);
   
   SetMenuEntry(0, "help_view", (XtCallbackProc) HelpCallback,
		(XtPointer) this);
   entry = CreateMenuBarEntry(children[0], "help", names, procs, private, 1);

   n = 0;
   XtSetArg(args[n], XmNmenuHelpWidget, entry); n++;
   XtSetValues(children[0], args, n);
   XtManageChild(children[0]);

   n = 0;
   width = WidthOfScreen(XtScreenOfObject(mw)) / 2 ;
   height = HeightOfScreen(XtScreenOfObject(mw)) * 2 / 3 ;
   XtSetArg(args[n], XmNwidth, width); n++;
   XtSetArg(args[n], XmNheight, height); n++;
   XtSetArg(args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
   children[1] = XmCreateForm(mw, "work", args, n);

   n = 0;
   XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++; 
   XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++; 
   XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM) ; n++;
   XtSetArg(args[n], XmNleftAttachment,  XmATTACH_FORM) ; n++;
   this->paned_window = XmCreatePanedWindow(children[1], "panes", args, n);
   XtManageChild(this->paned_window);
   n = 0;
   this->text_source = XmCreateText(children[1],   /* the form */
				    "textsource", args, n);
   n = 0;
   XtSetArg(args[n], XmNlabelString, FetchString(this, "empty_file")); n++;
   this->path = children[2] = XmCreateLabel(mw, "filename", args, n);

   n = 0;
   XtSetArg(args[n], XmNmenuBar, children[0]); n++;
   XtSetArg(args[n], XmNworkWindow, children[1]); n++;
   XtSetArg(args[n], XmNmessageWindow, children[2]); n++;
   XtSetValues(mw, args, n);
   XtManageChildren(children, 3);
   XtManageChild(mw);
   return this;
}

/* ===============================================================
 *   Create a cascade and a pulldown entry into a menubar, 
 * 	from a list of push button specs.
 *   If the label is NULL, create a separator instead of a push button.
 */
#ifdef _NO_PROTO
static Widget CreateMenuBarEntry(menubar, entry, names, procs, private, count)
		Widget menubar;
		String entry;
		String *names;
		XtCallbackProc *procs;
		XtPointer *private;
		int count;
#else
static Widget CreateMenuBarEntry(Widget menubar, String entry, String names[],
				 XtCallbackProc procs[],
				 XtPointer private[], int count)
#endif
{
   Widget menu;
   Widget child;
   Arg args[8];
   int n;
   int i;
   char menu_name[64];

   n = 0;
   XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
   menu = XmCreatePulldownMenu(menubar, 
			       strcat(strcpy(menu_name, entry), "Menu"),
			       args, n);
   for(i = 0; i < count; i++) {
      n = 0;
      if (names[i] == NULL)
	child = XmCreateSeparator(menu, "sep", args, n);
      else {
	 child = XmCreatePushButton(menu, names[i], args, n);
	 if (procs[i] != NULL)
	   XtAddCallback(child, XmNactivateCallback, procs[i], private[i]);
      }
      XtManageChild(child);
   }
   n = 0;
   XtSetArg(args[n], XmNsubMenuId, menu); n++;
   child = XmCreateCascadeButton(menubar, entry, args, n);
   XtManageChild(child);
   return child;
}

/* ===============================================================
 *   Routine to change menu bar items sensitivity.
 *   Takes a cascade button, extract the pulldown, find the item.
 */

#ifdef _NO_PROTO
void SetSensitive(cascade, item, sensitive)
		Widget cascade;
		String item;
		Boolean sensitive;
#else
void SetSensitive(Widget cascade, String item, Boolean sensitive)
#endif
{
   Widget menu, button;

   XtVaGetValues(cascade, XmNsubMenuId, &menu, NULL);
   button = XtNameToWidget(menu, item);
   XtSetSensitive(button, sensitive);
}

/* ===============================================================
 *   The Exit Callback. The root widget is passed as client data.
 *	Close properly. Exit.
 */

#ifdef _NO_PROTO
static void ExitCallback(button, root, call_data)
		Widget button;
		Widget root;
		XmPushButtonCallbackStruct *call_data;
#else
static void ExitCallback(Widget button, Widget root,
			 XmPushButtonCallbackStruct *call_data)
#endif
{
   Display * dpy = XtDisplay(root);

   XtDestroyWidget(root);
   XtCloseDisplay(dpy);
   exit(0);
}

/* ===============================================================
 *   The Exit Callback. The root widget is passed as client data.
 *	Close properly. Exit.
 */

#ifdef _NO_PROTO
static void CloseCallback(button, this, call_data)
		Widget button;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
static void CloseCallback(Widget button, ViewPtr this,
			  XmPushButtonCallbackStruct *call_data)
#endif
{

   printf("close callback\n");
   XtDestroyWidget(this->shell);
   XtFree((char *) this);
}

/* ===================================================================
 * The Help callback: Show an information message box.
 */
#ifdef _NO_PROTO
static void HelpCallback(widget, this, call_data)
		Widget	widget;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
static void HelpCallback(Widget	widget, ViewPtr this,
			 XmPushButtonCallbackStruct *call_data)
#endif
{
  static Widget HelpDialog = NULL;

  if ( HelpDialog == NULL ) {
     Arg args[8];
     int n = 0;
     XmString help1, help2, help3, help_msg;
     
     help2 = FetchString(this, "help_file");
     help1 = XmStringConcat(help2, XmStringSeparatorCreate());
     XmStringFree(help2);
     help2 = FetchString(this, "help_view");
     help_msg = XmStringConcat(help1, 
			       XmStringConcat(help2,
					      XmStringSeparatorCreate()));
     XmStringFree(help1);
     XmStringFree(help2);
     help2 = FetchString(this, "help_search");
     help1 = help_msg;
     help_msg = XmStringConcat(help1, help2);
     XmStringFree(help1);
     XmStringFree(help2);
     XtSetArg(args[n], XmNmessageString, help_msg); 
     n++;
     HelpDialog = XmCreateInformationDialog(this->shell,
					    "helpdialog", args, n);
     XmStringFree(help_msg);
  }
  XtManageChild(HelpDialog);
}

/* ===============================================================
 *   The Open New Shell Callback. 
 * The primary View object is passed in the client_data
 *	Open a new shell and a new file selected by a file selection box.
 */

#ifdef _NO_PROTO
static void OpenNewShellCallback(widget, this, call_data)
		Widget widget;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
static void OpenNewShellCallback(Widget widget, ViewPtr this,
			 XmPushButtonCallbackStruct *call_data)
#endif
{
   XmPushButtonCallbackStruct dummy_data;
   ViewPtr view;

   view = NewFileShell(theWidgetRoot, False, 0, NULL);
   XtPopup(view->shell, XtGrabNone); 
}

/* ===============================================================
 *   The Open File Callback. The View object is passed in the client_data
 *	Open a new file selected by a file selection box.
 */

#ifdef _NO_PROTO
static void OpenFileCallback(widget, this, call_data)
		Widget widget;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
static void OpenFileCallback(Widget widget, ViewPtr this,
			 XmPushButtonCallbackStruct *call_data)
#endif
{
   Arg args[8];
   int n = 0;

   if (this->fsb == NULL) {
      this->fsb = CreateFileSelectionBox(this);
      XtManageChild(this->fsb);
   }
   else if (XtIsManaged(this->fsb))
     XtPopup(XtParent(this->fsb), XtGrabNone);
   else XtManageChild(this->fsb);
}

/* ===============================================================
 *   Create File Selection Box
 */
#ifdef _NO_PROTO
static Widget CreateFileSelectionBox(this)
		ViewPtr this;
#else
static Widget CreateFileSelectionBox(ViewPtr this)
#endif
{
   Arg args[8];
   int n = 0;
   Widget fsb;
   
   XtSetArg(args[n], XmNallowShellResize, True); n++;
   XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW);  n++;
/*
     XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);
     n++;
 */
   fsb = XmCreateFileSelectionDialog(this->shell, "fsb", args, n);
   XtAddCallback(fsb, XmNokCallback, (XtCallbackProc) FileOKCallback, this);
   XtAddCallback(fsb, XmNcancelCallback, (XtCallbackProc) FileCancelCallback,
		 this);
   return fsb;
}

/* ===================================================================
 * Pop down the fsb.
 */
#ifdef _NO_PROTO
static void FileCancelCallback(fsb, this, call_data)
		Widget fsb;
		ViewPtr this;
		XmFileSelectionBoxCallbackStruct *call_data;
#else
static void FileCancelCallback(Widget fsb, ViewPtr this,
			XmFileSelectionBoxCallbackStruct *call_data)
#endif
{
   XtPopdown(XtParent(this->fsb));
}

/* =====================================================================
 * Fetch an string from Mrm database
 */

#ifdef _NO_PROTO
XmString FetchString(this, name)
		ViewPtr this;
		String name;
#else
XmString FetchString(ViewPtr this, String name)
#endif
{
   MrmCode code;
   XmString fetched;

   if (MrmFetchLiteral(theUIDdatabase, name, theDisplay,
		       (XtPointer) &fetched, (MrmCode *) &code) == MrmSUCCESS) 
     return fetched;

   ViewError(this, XmStringCreateLocalized("Can't fetch from database: "),
		  XmStringCreateLocalized(name));
   return XmStringCreateLocalized("String Not Found");
}

/* =====================================================================
 * Show application modal error dialogue box
 */

#ifdef _NO_PROTO
void ViewError(this, s1, s2)
		ViewPtr this;
		XmString s1;
		XmString s2;
#else
void ViewError(ViewPtr this, XmString s1, XmString s2)
#endif
{
   XmString msg;

   if (s1 == NULL)
     msg = s2;
   else if (s2 == NULL)
     msg = s1;
   else 
     msg = XmStringConcat(s1, s2);

   if (this->error_box == NULL) {
      Arg args[8];
      int n = 0;

      XtSetArg(args[n], XmNdialogStyle, 
	       XmDIALOG_FULL_APPLICATION_MODAL);
      n++;
      this->error_box = XmCreateErrorDialog(this->shell, "error", args, n);
   }
   XtVaSetValues(this->error_box, XmNmessageString, msg, NULL);
   XtManageChild(this->error_box);
}

/* =====================================================================
 * Show application modal error dialogue box
 */

#ifdef _NO_PROTO
void ViewWarning(this, s1, s2)
		ViewPtr this;
		XmString s1;
		XmString s2;
#else
void ViewWarning(ViewPtr this, XmString s1, XmString s2)
#endif
{
   XmString msg;

   if (s1 == NULL)
     msg = s2;
   else if (s2 == NULL)
     msg = s1;
   else 
     msg = XmStringConcat(s1, s2);

   if (this->warning_box == NULL) {
      Arg args[8];
      int n = 0;

      XtSetArg(args[n], XmNdialogStyle, 
	       XmDIALOG_FULL_APPLICATION_MODAL);
      n++;
      this->warning_box = XmCreateWarningDialog(this->shell, 
						"warning", args, n);
   }
   XtVaSetValues(this->warning_box, XmNmessageString, msg, NULL);
   XtManageChild(this->warning_box);
}
