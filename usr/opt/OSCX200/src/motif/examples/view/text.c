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
static char *rcsid = "@(#)$RCSfile: text.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 20:31:19 $";
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
static char rcsid[] = "$RCSfile: text.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 20:31:19 $"
#endif
#endif

#include "text.h"

/* ===================================================================
 * The OK callback for FSB. Check the selected file can be opened.
 * If so, close the old file, read the new file.
 * Create an unmapped text source with the contents of the file.
 */
#ifdef _NO_PROTO
void FileOKCallback(fsb, this, call_data)
		Widget fsb;
		ViewPtr this;
		XmFileSelectionBoxCallbackStruct *call_data;
#else
void FileOKCallback(Widget fsb, ViewPtr this,
			XmFileSelectionBoxCallbackStruct *call_data)
#endif
{
   FILE * file;
   char * path;
   Arg args[8];
   int n = 0;
   char *buffer;
   static char no_file[] = "no_file" ;
   static XmString no_file_msg = NULL;
   XmPushButtonCallbackStruct dummy;
   int filesize;
   XmStringContext ctxt;
   XmStringCharSet charset;
   XmStringDirection dir;
   Boolean sep;
	 

   XmStringInitContext(&ctxt, call_data->value);
   XmStringGetNextSegment(ctxt, &path, &charset, &dir, &sep);
   XmStringFreeContext(ctxt);
   XtFree((char *)dir);
   XtFree((char *)sep);
   XtFree((char *)charset);
   if (((file = OpenFile(path)) == NULL)
       || (buffer = ReadFile(file, &filesize)) == NULL)
     {
	 if (no_file_msg == NULL)
	   no_file_msg = FetchString(this, no_file);
	 ViewError(this, no_file_msg, call_data->value);
      }
   else {
      PanePtr pane = this->panes, tmp;
	
      XtPopdown(XtParent(fsb));
      while ( pane != NULL) { /* destroy all panes */
	 XtDestroyWidget(pane->text);
	 tmp = pane;
	 pane = pane->next;
	 XtFree((char *)tmp);
      }
      this->panes = NULL;
      this->current_pane = NULL;
      /* Set the new source text */
      XmTextSetString(this->text_source, buffer); 
      XtFree(buffer);
      XtFree(path);
      CloseFile(file);
      NewPaneCallback(this->paned_window, this, &dummy);
      XtVaSetValues(this->path,
		    XmNlabelString, call_data->value,
		    NULL);
   }
}

/* ===================================================================
 * The new pane callback. Create a new pane in the pane window.
 * Alloc a pane structure, initialize it.
 * Set focus to the new pane.
 * Allow menu items on panes.
 */

#ifdef _NO_PROTO
void NewPaneCallback(widget, this, call_data)
		Widget widget;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
void NewPaneCallback(Widget widget, ViewPtr this,
			    XmPushButtonCallbackStruct *call_data)
#endif
{
   PanePtr new;
   Arg args[10];
   int n = 0;
   short split = 0;
   short rows, cols;
   Dimension width, height;
   short index ;
   XmFontList fl;
   XmFontContext fontctxt;
   XmFontType type;
   XFontSetExtents * bbox;
   XtPointer font;
   int em, line;
   Widget target = NULL;

   new = (PanePtr) XtCalloc(sizeof(Pane), 1);
   if (this->panes != NULL)
     this->panes->previous = new;
   new->next = this->panes;
   this->panes = new;

/*
 * If not first time, split current pane in 2 to create the new pane
 */
   if (this->n_panes == 0) { /* first time, just load the file */
      SetSensitive(this->view_cascade, new_pane, True);
      SetSensitive(this->view_cascade, search, True);
   }
   else {
      target = XtParent(this->current_pane->text);
   }
   ++this->n_panes;
   n = 0;
   XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
   XtSetArg(args[n], XmNallowResize, True); n++;
   new->text = XmCreateScrolledText(this->paned_window, "pane", args, n);
/*
   XtAddCallback(new->text,
		 XmNmodifyVerifyCallback, (XtCallbackProc) NoInsert, this);
*/
   XtAddCallback(new->text, 
		 XmNfocusCallback, (XtCallbackProc) ChangeCurrentPane, this);
   XmTextSetSource(new->text, 
		   XmTextGetSource(this->text_source), 0, 0);
   if (target != NULL) { /* this is not the first pane */
      n = 0;
      XtSetArg(args[n], XmNpositionIndex, &index); n++;
      XtSetArg(args[n], XmNheight, &height); n++;
      XtGetValues(target, args, n);
      ++index;
      n = 0;
      XtSetArg(args[n], XmNpositionIndex, index); n++;
      XtSetArg(args[n], XmNheight, (Dimension) height/2); n++;
      XtSetValues(XtParent(new->text), args, n);
      XmTextSetTopCharacter(new->text,
			    XmTextGetTopCharacter(this->current_pane->text));
      XtVaSetValues(target, XmNheight, (Dimension) height/2);
   }
   XtManageChild(new->text);

   if  (this->n_panes == 2)
     SetSensitive(this->view_cascade, kill_pane, True);

   XmProcessTraversal(new->text, XmTRAVERSE_CURRENT);
   this->current_pane = new;
}

/* ===================================================================
 * The kill pane callback. Delete a pane in the pane window.
 * Free pane structure.
 */

#ifdef _NO_PROTO
void KillPaneCallback(button, this, call_data)
		Widget button;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
void KillPaneCallback(Widget button, ViewPtr this,
			     XmPushButtonCallbackStruct *call_data)
#endif
{
   PanePtr *pane, tmp;

/*
   printf("%d panes left\n", this->n_panes-1);
 */
   for (pane = &this->panes; *pane != this->current_pane; )
     pane = &((*pane)->next);
/* 
 * Destroy the old one, free memory.
 * Do not allow the last pane to be destroyed
 * Make destroy command unavailable if last pane.
 * Make next or previous pane become current and traverse to it.
 */

   tmp = *pane;
   *pane = (*pane)->next;
   XtDestroyWidget(tmp->text);
   XtFree((char *)tmp);
   this->current_pane = (*pane == NULL) ? this->panes : *pane;
   if ( --this->n_panes < 2 )
      SetSensitive(this->view_cascade, kill_pane, False);

   XmProcessTraversal(this->current_pane->text, XmTRAVERSE_CURRENT);
}

/* =====================================================================
 * Focus has moved. Change current pane.
 */

#ifdef _NO_PROTO
static void ChangeCurrentPane(text, this, verify)
		Widget text;
		ViewPtr this;
		XmAnyCallbackStruct verify;
#else
static void ChangeCurrentPane(Widget text, ViewPtr this, 
			      XmAnyCallbackStruct verify)
#endif
{
   PanePtr pane = this->panes;

   while (pane != NULL) {
      if (pane->text == text)
	break;
      pane = pane->next;
   }
   this->current_pane = pane;
}

/* ===============================================================
 *   The Find Callback. The parent widget is passed as client data.
 *
 */

#ifdef _NO_PROTO
void FindCallback(button, this, call_data)
		Widget button;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
void FindCallback(Widget button, ViewPtr this,
			  XmPushButtonCallbackStruct *call_data)
#endif
{
  Widget template;
  XtCallbackRec ok[2], cancel[2];
#define NUMBOXES 4
#define NUMBUTTONS 2
#define TITLEDFRAME 2
  Widget framed[TITLEDFRAME];
  Widget frame;
  Widget forward, backward;

  if ( this->search_box == NULL ) {
     Arg args[10];
     int n = 0;

     search_msg = FetchString(this, search_prompt);
     XtSetArg(args[n], XmNautoUnmanage, False); n++;
     XtSetArg(args[n], XmNmessageString, search_msg); n++;
     ok[0].callback = (XtCallbackProc) SearchSubstring;
     ok[0].closure = (XtPointer) this;
     ok[1].callback = cancel[1].callback = NULL;
     ok[1].closure = cancel[1].closure = NULL;
     cancel[0].callback = (XtCallbackProc) CancelSearch;
     cancel[0].closure = (XtPointer) this;
     XtSetArg(args[n], XmNokCallback, (XtCallbackList) ok);  n++;
     XtSetArg(args[n], XmNcancelCallback, (XtCallbackList) cancel);  n++;
     this->search_box = XmCreateTemplateDialog(this->shell, "search_box",
					       args, n);
     n = 0;
     template = XmCreateForm(this->search_box, "form", args, n);
     n = 0;
     XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++; 
     XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM) ; n++;
     XtSetArg(args[n], XmNleftAttachment,  XmATTACH_FORM) ; n++;
     this->search_entry = XmCreateTextField(template, "entry", args, n);
     n = 0;
     XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++; 
     XtSetArg(args[n], XmNbottomAttachment,XmATTACH_WIDGET); n++; 
     XtSetArg(args[n], XmNbottomWidget, this->search_entry); n++; 
     XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM) ; n++;
     XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM) ; n++;
     frame = XmCreateFrame(template, "dir_frame", args, n);
     n = 0;
     XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
     framed[0] = XmCreateLabel(frame, "title", args, n);
     n = 0;
     XtSetArg(args[n], XmNchildType, XmFRAME_WORKAREA_CHILD); n++;
     XtSetArg(args[n], XmNisAligned, True); n++;
     XtSetArg(args[n], XmNradioAlwaysOne, True); n++;
     XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
     XtSetArg(args[n], XmNentryClass, xmToggleButtonWidgetClass); n++;
     XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
     this->direction = 
       framed[1] = XmCreateRadioBox(frame, "direction", args, n);
     n = 0;
     XtSetArg(args[n], XmNuserData, XmTEXT_FORWARD); n++;
     forward = XmCreateToggleButton(framed[1], "forward", args, n);
     n = 0;
     XtSetArg(args[n], XmNuserData, XmTEXT_BACKWARD); n++;
     backward = XmCreateToggleButton(framed[1], "backward", args, n);
     XtManageChild(forward);
     XtManageChild(backward);
     XtManageChildren(framed, TITLEDFRAME);
     XmToggleButtonSetState(forward, True, True);
     XtManageChild(this->search_entry);
     XtManageChild(frame);
     XtManageChild(template);
  }
  if (XtIsManaged(this->search_box))
    XtPopup(XtParent(this->search_box), XtGrabNone);
  else
    XtManageChild(this->search_box);
  XmProcessTraversal(this->search_entry, XmTRAVERSE_CURRENT);

#undef NUMBUTTONS
#undef NUMBOXES
#undef TITLEDFRAME
}


/* ===============================================================
 *   The Find Callback. The View object is passed as client data.
 *
 */

#ifdef _NO_PROTO
static void CancelSearch(button, this, call_data)
		Widget button;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
static void CancelSearch(Widget button, ViewPtr this,
			    XmPushButtonCallbackStruct *call_data)
#endif
{  
   XtPopdown(XtParent(this->search_box));
}

/* ===============================================================
 *   The Find Callback. The parent widget is passed as client data.
 *
 */

#ifdef _NO_PROTO
static void SearchSubstring(button, this, call_data)
		Widget button;
		ViewPtr this;
		XmPushButtonCallbackStruct *call_data;
#else
static void SearchSubstring(Widget button, ViewPtr this,
			    XmPushButtonCallbackStruct *call_data)
#endif
{
#define STRING_MAX_CHARS 1024
   char *substring;
   int status;
   XmTextPosition pos;
/*   int last = XmTextFieldGetLastPosition(this->search_entry); */
   XmString search = NULL;
   XmTextDirection direction;
   Widget toggle;

/*

   status = XmTextFieldGetSubstring(this->search_entry, 0, last,
				    STRING_MAX_CHARS, substring);
   if (status == XmCOPY_FAILED || status == XmCOPY_TRUNCATED) {
      if (no_search_msg == NULL)
	no_search_msg = FetchString(this, no_search);
      search =  XmStringCreateLocalized(substring);
      ViewWarning(this, no_search_msg, search);
      return;
   }
*/
   substring = XmTextFieldGetString(this->search_entry);
   if (substring == NULL) {
      if (no_pattern_msg == NULL)
	no_pattern_msg = FetchString(this, no_pattern);
      ViewWarning(this, no_pattern_msg, (XmString) NULL);
      return;
   }
   XtVaGetValues(this->direction, XmNmenuHistory, &toggle, NULL);
   XtVaGetValues(toggle, XmNuserData, &direction, NULL);
   if (XmTextFindString(this->current_pane->text,
			XmTextGetInsertionPosition(this->current_pane->text),
			substring, direction, &pos))
     {
	XmTextSetTopCharacter(this->current_pane->text, pos);
	XmTextSetInsertionPosition(this->current_pane->text, pos);
	XtPopdown(XtParent(this->search_box));
     }
   else {
      if (not_found_msg == NULL)
	not_found_msg = FetchString(this, not_found);
      search =  XmStringCreateLocalized(substring);
      ViewWarning(this, not_found_msg, search);
      XmStringFree(search);
   }
   XtFree(substring);
}

/* =====================================================================
 * Reject text insertion
 */

#ifdef _NO_PROTO
static void NoInsert(text, this, verify)
		Widget text;
		ViewPtr this;
		XmTextVerifyPtr verify;
#else
static void NoInsert(Widget text, ViewPtr this, XmTextVerifyPtr verify)
#endif
{
/* 
 if (verify->startPos != verify->endPos)
     printf("deleting text %d %d\n", verify->startPos, verify->endPos);
   if (verify->text != NULL && verify->text->length > 0)
     printf("inserting %d characters: '%s'\n",
	    verify->text->length,
	    verify->text->ptr);
*/
}

