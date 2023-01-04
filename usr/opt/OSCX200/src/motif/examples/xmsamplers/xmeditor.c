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
static char *rcsid = "@(#)$RCSfile: xmeditor.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 21:34:16 $";
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
static char rcsid[] = "$RCSfile: xmeditor.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 21:34:16 $"
#endif
#endif
/*
*  (c) Copyright 1989 HEWLETT-PACKARD COMPANY. */

/**---------------------------------------------------------------------
***	
***	file:		xmeditor.c
***
***	project:	Motif Widgets example programs
***
***	description:	This program demonstrates the Motif text, main window,
***			and dialog widgets, as well as the cut and paste
***			functions.
***	
***	defaults:	xmeditor.c depends on these defaults:
***
#
*allowShellResize:		true
*borderWidth:			0
*highlightThickness:		2
*traversalOn:			true
*keyboardFocusPolicy:		explicit
#
xmeditor*menu_bar*background:	#58f
#
***-------------------------------------------------------------------*/

/*-------------------------------------------------------------
**	Include Files
*/

#include <stdio.h>
#ifdef VMS
#include <file.h>
#else
#include <fcntl.h>
#endif
#include <errno.h>
#ifndef VMS
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/FileSB.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/CutPaste.h>

/*-------------------------------------------------------------
**	Global Variables
*/

#define MENU_HELP		200
#define MENU_EXIT		201
#define MENU_OPEN		202
#define MENU_NEW		203
#define MENU_CLOSE		204
#define MENU_SAVE		205
#define MENU_SAVE_AS		206
#define MENU_PRINT		207
#define MENU_CUT		208
#define MENU_COPY		209
#define MENU_PASTE		210
#define MENU_CLEAR		211

#define DIALOG_FSELECT		300
#define DIALOG_CWARNING		301
#define DIALOG_XWARNING		302
#define DIALOG_NEW		303
#define DIALOG_SAVE		304
#define DIALOG_HELP		305
#define DIALOG_PRINT		306

/* defines a temporary file for file transfers */

Widget text;			/* multi-line text widget		    */
Widget cut_button;		/* clipboard cut button 		    */
Widget copy_button;		/* clipboard copy button 		    */
Widget paste_button;		/* clipboard paste button 		    */
Widget clear_button;		/* clipboard clear button 		    */
Widget open_dialog;		/* file selection dialog 		    */
Widget new_dialog;		/* file name prompt dialog 		    */
Widget close_warning;		/* special internal selection dialog	    */
Widget exit_warning;		/* special internal selection dialog	    */
Widget general_warning;		/* warning dialog	 		    */
Widget save_dialog;		/* save as prompt dialog	 	    */
Widget print_warning;		/* warning dialog		 	    */
Boolean file_saved = True;	/* indicates that the present file is saved */
char *filename = NULL;		/* string containing file name 		    */
int start_pos, end_pos;		/* start and end position of last action    */

XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
				/* used to set up XmStrings */

char Error[128];
XtAppContext app_context;/*  Application Context  	*/
Widget Shell1;	   /*  ApplicationShell 	*/

/* bits for exclamation point in dialog */
char warningBits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00,
   0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xe0, 0x07, 0x00,
   0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};


/************************************************************************
 *
 *  CreateDefaultImage - create a default images for warning symbol.
 *
 **********************************<->***********************************/

static XImage *CreateDefaultImage(bits, width, height)
char *bits;
int width, height;
{
    XImage *image;

    image = (XImage *) XtMalloc(sizeof (XImage));
    image->width = width;
    image->height = height;
    image->data = bits;
    image->depth = 1;
    image->xoffset = 0;
    image->format = XYBitmap;
    image->byte_order = LSBFirst;
    image->bitmap_unit = 8;
    image->bitmap_bit_order = LSBFirst;
    image->bitmap_pad = 8;
    image->bytes_per_line = (width+7)/8;
#ifdef DEC_MOTIF_BUG_FIX
    image->bits_per_pixel = 1;	/* for Alpha */
#endif
    return(image);
}

/*-------------------------------------------------------------
**	OpenFile - Open the present file.  Returns true if file 
**                 exists and open is sucessful.
*/

Boolean OpenFile()
{
   struct stat statbuf;		/* Information on a file. */
   int   file_length;		/* Length of file. 	  */
   char *file_string;	        /* Contents of file. 	  */
   FILE *fp = NULL;		/* Pointer to open file   */


   if ((fp = fopen(filename, "r+")) == NULL)
	if ((fp = fopen(filename, "r")) != NULL) {
	    fprintf(stderr, "Warning: file opened read only.\n");
	} else {
	    return(False);
	}

   if (stat(filename, &statbuf) == 0)
	 file_length = statbuf.st_size;
   else
	 file_length = 1000000; /* arbitrary file length */

   /* read the file string */
   file_string = (char *) XtMalloc((unsigned)file_length);
   fread(file_string, sizeof(char), file_length, fp);

   /* close up the file */
   if (fclose(fp) != NULL) fprintf(stderr, "Warning: unable to close file.\n");

   /* added the file string to the text widget */
   XmTextSetString(text, file_string);

   file_saved = True; /* intialize to True */
	 
   /* make appropriate item sensitive */
   XtSetSensitive(text, True);
   XtSetSensitive(cut_button, True);
   XtSetSensitive(copy_button, True);
   XtSetSensitive(paste_button, True);
   XtSetSensitive(clear_button, True);
   return(True);
}

/*-------------------------------------------------------------
**	SaveFile - Save the present file.
*/

Boolean SaveFile()
{
    char *file_string = NULL;   	  /* Contents of file.		     */
    FILE *tfp;				  /* Pointer to open temporary file. */
    char namebuf[BUFSIZ]; 		  /* for "system" call below         */
    char *tempname = (char *)XtMalloc(25); /* Temporary file name.           */
    int status;


#ifdef VMS
    sprintf(tempname, "%s", (_XmConst char *)mktemp("xmeditXXXXXX.tmp"));
#else
    sprintf(tempname, "%s", (_XmConst char *)mktemp("/tmp/xmeditXXXXXX"));
#endif
    
    if ((tfp = fopen(tempname, "w")) == NULL) {
       fprintf(stderr, "Warning: unable to open temp file, text not saved.\n");
       return(False);;
     }

    /* get the text string */
    file_string = (char *)XmTextGetString(text);

    /* write to a temp file */
    fwrite(file_string, sizeof(char), strlen(file_string) + 1, tfp);

    /* flush and close the temp file */
    if (fflush(tfp) != NULL) 
      fprintf(stderr,"Warning: unable to flush file.\n");

    if (fclose(tfp) != NULL) 
      fprintf(stderr,"Warning: unable to close file.\n");

    if (file_string != NULL) {
        XtFree((XtPointer)file_string); /* free the text string */
      }

    /* 
     * Move the tempname to the saved file, but do it independent
     *  of filesystem boundaries
     */

#ifdef VMS 
    sprintf(namebuf, "copy %s %s", tempname, filename);
#else
    sprintf(namebuf, "cp %s %s", tempname, filename);
#endif
    status = system(namebuf);

#ifdef VMS
    sprintf (namebuf, "delete/nolog %s;*\0", tempname);
    system (namebuf);
#else
    unlink(tempname);
#endif

    if (status == 0) 
      {
	file_saved = True;
      } else 
	{
	  fprintf(stderr, "Warning: unable to save file.\n");
	  XtFree(tempname);
	  return(False);
	}
    XtFree(tempname);
    return(True);
}

/*-------------------------------------------------------------
**      CloseFile - Close the present file.
*/

void CloseFile()
{
  /* 
   * Zero out the text string in the text widget.
   * caution: is causes a value changed callack. 
   */

  XmTextSetString(text, "");

  file_saved = True; /* reinitialize file_saved flag */

  /* free the file name */
  if (filename != NULL) {
    XtFree(filename);
    filename = NULL;
  }

  /* set text to insensitive */
  XtSetSensitive(text, False);
}

/*-------------------------------------------------------------
**	FileChangedCB Process callback from Text.
*/
/* ARGSUSED */
void FileChangedCB(w, client_data, call_data) 
Widget    w;		/*  widget id		*/
XtPointer client_data;	/*  data from application   */
XtPointer call_data;	/*  data from widget class  */
{
  /* 
   * Set the file_saved flag to indicate that the
   *  file has been modified and the user should be
   *  notified before exiting. 
   */

    file_saved = False;
}

/*-------------------------------------------------------------
**	MenuCB - Process callback from PushButtons in PulldownMenus.
*/
/* ARGSUSED */
void MenuCB(w, client_data, call_data) 
Widget    w;		/*  widget id		*/
XtPointer client_data;	/*  data from application   */
XtPointer call_data;	/*  data from widget class  */
{
  register int n;		/* arg count		    */
  Arg args[10];			/* arg list		    */
  char *command;		/* command used in printing */
  XmString tcs;

  switch ((int)client_data)
    {
    case MENU_OPEN:
      /* display the file selection dialog */
      XtManageChild(open_dialog);
      break;

    case MENU_NEW:
      /* display the prompt dialog */
      XtManageChild(new_dialog);
      break;

    case MENU_CLOSE:
      /* the present file has not been saved since
	 the last modification */
      if (!file_saved) /* display the 'save' message dialog */
	XtManageChild(close_warning);
      else
	CloseFile();
      break;

    case MENU_SAVE:
      /* open a temp file for writing */
      SaveFile();
      break;

    case MENU_SAVE_AS:
      /* Display the 'save as' dialog with the
	 present filename displayed in it. */
      n = 0;
      tcs  = XmStringCreateLtoR(filename, charset);
      XtSetArg(args[n], XmNtextString, tcs); n++;
      XtSetValues(save_dialog, args, n);
      XtManageChild(save_dialog);
      break;

    case MENU_PRINT:
      if(!file_saved)
	XtManageChild(print_warning);
      else if (filename != NULL) {
	/* malloc space for the command name. 
	   Note: command = size of the filename +
	   "lp " + null terminator */
	command = XtMalloc(strlen(filename) + 4);
#ifdef VMS
	sprintf(command, "print %s", filename);
#else
	sprintf(command, "lp %s", filename);
#endif
	if (system(command) != NULL)
	  fprintf(stderr, "print failed");
	XtFree(command);
      }
      break;

    case MENU_EXIT:
      /* exit if there is no files open */
      if (!file_saved) /* display the 'save' message dialog */
	XtManageChild(exit_warning);
      else {
	/* close up file pointers and descriptors */
	CloseFile();
	
	/* exit this program */
	exit(0);
      }
      break;

    case MENU_CUT:
      {
	/* needed to get the event time */
	XmAnyCallbackStruct * cb = (XmAnyCallbackStruct *) call_data;

	/* call routine to copy selection to clipboard */
	XmTextCut(text, cb->event->xbutton.time);
      }
      break;

    case MENU_COPY:
      {
	/* needed to get the event time */
	XmAnyCallbackStruct * cb = (XmAnyCallbackStruct *) call_data;

	XmTextCopy(text, cb->event->xbutton.time);
      }
      break;

    case MENU_PASTE:
      {
	/* needed to get the event time */
	XmAnyCallbackStruct * cb = (XmAnyCallbackStruct *) call_data;

	XmTextPaste(text);
      }
      break;

    case MENU_CLEAR:
      {
	XmTextPosition left, right;

	/* needed to get the event time */
	XmAnyCallbackStruct * cb = (XmAnyCallbackStruct *) call_data;

	XmTextGetSelectionPosition(text, &left, &right);
	XmTextReplace(text, left, right, "");
	XmTextSetInsertionPosition(text, left);
      }
      break;

    case MENU_HELP:
      /* no help at this time */
      break;

    default:
      /* unknown client_data was received and there is no setup 
         to handle this */
      fprintf(stderr, "Warning: in menu callback\n");
      break;
    }
}

/*-------------------------------------------------------------
**	DialogApplyCB - Process callback from Dialog apply actions.
*/
/* ARGSUSED */
static void DialogApplyCB(w, client_data, call_data) 
Widget	w;		/*  widget id		*/
XtPointer client_data;	/*  data from application   */
XtPointer call_data;	/*  data from widget class  */
{
  char *command;			/* command used in printing */

  switch ((int)client_data)
    {
    case DIALOG_PRINT:
      if (filename != NULL) {
	/* malloc space for the command name. 
	   Note: command = size of the filename + "lp " + null terminator */
	command = XtMalloc(strlen(filename) + 4);
#ifdef VMS
	sprintf(command, "print %s", filename);
#else
	sprintf(command, "lp %s", filename);
#endif
	if (system(command) != NULL)
	  fprintf(stderr, "print failed");
	XtFree(command);
			}
    case DIALOG_CWARNING:
      CloseFile();
      file_saved = True; /* reset the default */
      break;

    case DIALOG_XWARNING:
      CloseFile();
      exit();
      break;		

    default:
      /* unknown client_data was recieved and
	 there is no setup to handle this */
      fprintf(stderr, "Warning: in apply callback\n");
      break;

    }
}

/*-------------------------------------------------------------
**	DialogCancelCB
**		Process callback from Dialog cancel actions.
*/
static void DialogCancelCB(w, client_data, call_data) 
Widget	w;		/*  widget id		*/
XtPointer client_data;	/*  data from application   */
XtPointer call_data;	/*  data from widget class  */
{
  switch ((int)client_data)
    {
    case DIALOG_FSELECT:
      /* popdown the file selection box */
      XtUnmanageChild(open_dialog);
      break;

    case DIALOG_CWARNING:
    case DIALOG_XWARNING:
    case DIALOG_NEW:
    case DIALOG_PRINT:
    case DIALOG_SAVE:
    case DIALOG_HELP:
      /* no action is necessary at this time */
      break;

    default:
      /* a unknown client_data was recieved and
	 there is no setup to handle this */
      fprintf(stderr, "Warning: in cancel callback\n");
      break;
    }
}

/*-------------------------------------------------------------
**	DialogAcceptCB
**		Process callback from Dialog actions.
*/
/* ARGSUSED */
static void DialogAcceptCB(w, client_data, call_data) 
Widget w;		/*  widget id		*/
XtPointer client_data;	/*  data from application   */
XtPointer call_data;	/*  data from widget class  */
{
  char *command;		/* command used in printing */
  
  switch ((int)client_data)
    {
    case DIALOG_FSELECT:
      /* open the file and read it into the text widget */
      if (filename != NULL) {
	XtFree(filename);
	filename = NULL;
      }
      {
	XmFileSelectionBoxCallbackStruct *fcb =
	  (XmFileSelectionBoxCallbackStruct *) call_data;

	/* get the filename from the file selection box */
	XmStringGetLtoR(fcb->value, charset, &filename);

	/* Open file, print error if it does not exist. */
	if (!OpenFile())
	  fprintf(stderr, "Warning: unable to open file\n");

	/* popdown the file selection box */
	XtUnmanageChild(open_dialog);
      }
      break;

    case DIALOG_NEW:
      /* open the file and read it into the text widget */
      if (filename != NULL) {
	XtFree(filename);
	filename = NULL;
      }
      {
	XmSelectionBoxCallbackStruct *scb =
	  (XmSelectionBoxCallbackStruct *) call_data;
	
	/* get the filename string from the file
	   name prompt box */
	XmStringGetLtoR(scb->value, charset, &filename);

	/* open file if it exists,
	   if not set items sensitive */
	if (!OpenFile()) {
	  /* make appropriate item sensitive */
	  XtSetSensitive(text, True);
	  XtSetSensitive(cut_button, True);
	  XtSetSensitive(paste_button, True);
	  XtSetSensitive(copy_button, True);
	  XtSetSensitive(clear_button, True);
	}
	
	/* popdown the file selection box */
	XtUnmanageChild(new_dialog);
      }
      break;

    case DIALOG_CWARNING:
      /* save the file */
      if (SaveFile()) {
	CloseFile(); /* close the file */
      } else
	fprintf(stderr, 
		"Warning: unable to save file, file not closed");
      break;

    case DIALOG_XWARNING:
      /* save the file */
      if (SaveFile()) {
	CloseFile(); /* close the file */
	exit(0);
      } else
	fprintf(stderr,
		"Warning: unable to save file, exit aborted");
      break;

    case DIALOG_SAVE:
      {
	XmSelectionBoxCallbackStruct *scb =
	  (XmSelectionBoxCallbackStruct *) call_data;

	/* get the filename string from the file
	   selection box */
	XmStringGetLtoR(scb->value, charset, &filename);
	
	SaveFile();

	XtUnmanageChild(save_dialog);
      }
      break;

    case DIALOG_PRINT:
      /* save the file */
      if (SaveFile()) {
	if (filename != NULL) {
	  /* malloc space for the command name. 
	     Note: command = size of the filename + "lp " + null terminator */
	  command = XtMalloc(strlen(filename) + 4);
#ifdef VMS
	sprintf(command, "print %s", filename);
#else
	sprintf(command, "lp %s", filename);
#endif
	  if (system(command) != NULL)
	    fprintf(stderr, "print failed");
	  XtFree(command);
	}
      } else
	fprintf(stderr, 
		"Warning: unable to save file, file not printed");
      break;

    case DIALOG_HELP:
      /* no help at this time */
      break;

    default:
      /* unknown callback type */
      fprintf (stderr, "Warning: in accept callback\n");
      break;
    }
}

/*-------------------------------------------------------------
**	CreateSpecialWarningDialog
**		Create special 4 button message box out of a
**	Selection box.
*/
static Widget CreateSpecialWarningDialog(parent, name, image_string, message,
					  arglist, argcount)
Widget		parent;
String		name;
String		image_string;
String		message;
ArgList		arglist;
int		argcount;
{
  Widget warning_dialog;/*  special warning selection box */
  Widget work_area;	/*  rowcolumn for pixmap and text */
  Widget pixmap_label;	/*  pixmap label 		  */
  Widget text_label;	/*  text label 			  */
  Widget apply_button;	/*  apply button		  */
  Widget ok_button;	/*  ok button			  */
  Widget kid[5];        /*  buttons		          */
  Pixel	 foreground;	/*  dialog foreground		  */
  Pixel	 background;	/*  dialog background		  */
  Pixmap pixmap;	/*  dialog pixmap		  */
  register int i;       /*  kid index			  */
  Arg  args[10];   	/*  arg list		          */
  register int n;       /*  arg count		          */
  XmString tcs;

  warning_dialog = XmCreatePromptDialog(parent, name, arglist, argcount);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  work_area = XmCreateRowColumn(warning_dialog, "workarea", args, n);
  XtManageChild(work_area);

  n = 0;
  XtSetArg(args[n], XmNforeground, &foreground); n++;
  XtSetArg(args[n], XmNbackground, &background); n++;
  XtGetValues(warning_dialog, args, n);

  n = 0;
  XtSetArg(args[n], XmNlabelType, XmPIXMAP); n++;
  pixmap = XmGetPixmap(XtScreen(warning_dialog), image_string,
		       foreground, background);
  XtSetArg(args[n], XmNlabelPixmap, pixmap); n++;
  XtSetArg(args[n], XmNtraversalOn, False); n++;
  pixmap_label = XmCreateLabel(work_area, "pixmap_label", args, n);
  XtManageChild(pixmap_label);

  n = 0;
  tcs = XmStringCreateLtoR(message, charset); 
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNtraversalOn, False); n++;
  text_label = XmCreateLabel(work_area, "text_label", args, n);
  XtManageChild(text_label);
  XmStringFree(tcs);

  apply_button = XmSelectionBoxGetChild(warning_dialog,
					 XmDIALOG_APPLY_BUTTON);
	
  n = 0;
  tcs = XmStringCreateLtoR("Discard", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetValues(apply_button, args, n);
  XtManageChild(apply_button);
  XmStringFree(tcs);

  ok_button = XmSelectionBoxGetChild(warning_dialog, XmDIALOG_OK_BUTTON);
  n = 0;
  tcs = XmStringCreateLtoR("Save", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetValues(ok_button, args, n);
  XmStringFree(tcs);
	
  /* 
   *  Unmanage unneeded children.
   */
  i = 0;
  kid[i++] = XmSelectionBoxGetChild(warning_dialog, XmDIALOG_TEXT);
  kid[i++] = XmSelectionBoxGetChild(warning_dialog,
				    XmDIALOG_SELECTION_LABEL);
  XtUnmanageChildren(kid, i);
  return(warning_dialog);
}

/*-------------------------------------------------------------
**	CreateMenuBar
**		Create MenuBar in MainWindow
*/

static Widget CreateMenuBar(parent)
Widget parent;
{
  Widget menu_bar;	/*  RowColumn	 		*/
  Widget cascade1;	/*  CascadeButton		*/
  Widget cascade2;	/*  CascadeButton               */
  Widget cascade3;   
  Widget menu_pane1;	/*  RowColumn	 		*/
  Widget menu_pane2;    /*  RowColumn			*/
  Widget button1, button2, button3, button4, button5, button6, button7;
  XImage *image;	/*  image for warning pixmap	*/
  Arg args[10];		/*  arg list			*/
  register int	n;	/*  arg count			*/
  XmString tcs, tcs1;

  /*
   *	Create MenuArea.
   */

  n = 0;
  menu_bar = XmCreateMenuBar(parent, "menu_bar", args, n);

  /*
   *	Create "Options" PulldownMenu.
   */

  n = 0;
  menu_pane1 = XmCreatePulldownMenu(menu_bar, "menu_pane1", args, n);
  image = CreateDefaultImage(warningBits, 32, 32);
  XmInstallImage(image, "warning_image");
					
  n = 0;
  tcs = XmStringCreateLtoR("Open", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'O'); n++;
  button1 = XmCreatePushButton(menu_pane1, "Open", args, n);
  XtAddCallback(button1, XmNactivateCallback, MenuCB, (XtPointer)MENU_OPEN);
  XtManageChild(button1);
  XmStringFree(tcs);

  open_dialog = XmCreateFileSelectionDialog(parent,
					    "file selection dialog", NULL, 0);

  XtAddCallback(open_dialog, XmNokCallback,
		 DialogAcceptCB, (XtPointer)DIALOG_FSELECT);
  XtAddCallback(open_dialog, XmNcancelCallback,
		 DialogCancelCB, (XtPointer)DIALOG_FSELECT);

  n = 0;
  tcs = XmStringCreateLtoR("New", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'N'); n++;
  button2 = XmCreatePushButton(menu_pane1, "New", args, n);
  XtAddCallback(button2, XmNactivateCallback, MenuCB, (XtPointer)MENU_NEW);
  XtManageChild(button2);
  XmStringFree(tcs);

  n = 0;
  tcs = XmStringCreateLtoR("Enter name of new file.", charset);
  XtSetArg(args[n], XmNselectionLabelString, tcs); n++; 
  new_dialog = XmCreatePromptDialog(parent,
				    "new file dialog", args, n);
  XtAddCallback(new_dialog, XmNokCallback,
		 DialogAcceptCB, (XtPointer)DIALOG_NEW);
  XtAddCallback(new_dialog, XmNcancelCallback,
		 DialogCancelCB, (XtPointer)DIALOG_NEW);
  XmStringFree(tcs);

  n = 0;
  tcs = XmStringCreateLtoR("Close", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'C'); n++;
  button3 = XmCreatePushButton(menu_pane1, "Close", args, n);
  XtAddCallback(button3, XmNactivateCallback, MenuCB, (XtPointer)MENU_CLOSE);
  XtManageChild(button3);
  XmStringFree(tcs);

  n=0;
  close_warning = CreateSpecialWarningDialog(parent, "save_warning",
					     "warning_image", 
					     "Save Changes?", args, n);

  XtAddCallback(close_warning, XmNapplyCallback,
		 DialogApplyCB, (XtPointer)DIALOG_CWARNING);
  XtAddCallback(close_warning, XmNokCallback,
		 DialogAcceptCB, (XtPointer)DIALOG_CWARNING);

  n = 0;
  tcs = XmStringCreateLtoR("Save", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'S'); n++;
  button4 = XmCreatePushButton(menu_pane1, "Save", args, n);
  XtAddCallback(button4, XmNactivateCallback, MenuCB, (XtPointer)MENU_SAVE);
  XtManageChild(button4);
  XmStringFree(tcs);

  n = 0;	
  tcs = XmStringCreateLtoR("Save As...", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'A'); n++;
  button5 = XmCreatePushButton(menu_pane1, "Save As...", args, n);
  XtAddCallback(button5, XmNactivateCallback, MenuCB, (XtPointer)MENU_SAVE_AS);
  XtManageChild(button5);
  XmStringFree(tcs);

  n = 0;
  tcs = XmStringCreateLtoR("Save As...", charset);
  XtSetArg(args[n], XmNselectionLabelString, tcs); n++;
  save_dialog = XmCreatePromptDialog(parent, "save dialog", args, n);
  XtAddCallback(save_dialog, XmNokCallback,
		DialogAcceptCB, (XtPointer)DIALOG_SAVE);
  XmStringFree(tcs);

  n = 0;
  tcs = XmStringCreateLtoR("Print", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'P'); n++;
  button6 = XmCreatePushButton(menu_pane1, "Print", args, n);
  XtAddCallback(button6, XmNactivateCallback, MenuCB, (XtPointer)MENU_PRINT);
  XtManageChild(button6);
  XmStringFree(tcs);

  n = 0;
  tcs = XmStringCreateLtoR("Save file before printing?", charset);
  XtSetArg(args[n], XmNselectionLabelString, tcs); n++;
  print_warning = CreateSpecialWarningDialog(parent, "print_warning",
					     "warning_image", 
					     "Save file before printing?", 
					     args, n);
  XtAddCallback(print_warning, XmNokCallback,
		 DialogAcceptCB, (XtPointer)DIALOG_PRINT);
  XmStringFree(tcs);

  n = 0;
  tcs = XmStringCreateLtoR("Exit", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'E'); n++;
  tcs1 = XmStringCreateLtoR("F3", charset);
  XtSetArg(args[n], XmNacceleratorText, tcs1);
  XtSetArg(args[n], XmNaccelerator, "<Key>F3:"); n++;
  button7 = XmCreatePushButton(menu_pane1, "Exit", args, n);
  XtAddCallback(button7, XmNactivateCallback, MenuCB, (XtPointer)MENU_EXIT);
  XtManageChild(button7);
  XmStringFree(tcs);
  XmStringFree(tcs1);

  n=0;
  exit_warning = CreateSpecialWarningDialog(parent, "exit warning",
					    "warning_image", 
					    "Save Changes?", args, n);
  XtAddCallback(exit_warning, XmNapplyCallback,
		 DialogApplyCB, (XtPointer)DIALOG_XWARNING);
  XtAddCallback(exit_warning, XmNokCallback,
		 DialogAcceptCB, (XtPointer)DIALOG_XWARNING);

  n = 0;
  XtSetArg(args[n], XmNsubMenuId, menu_pane1);  n++;
  tcs = XmStringCreateLtoR("File", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'F'); n++;
  cascade1 = XmCreateCascadeButton(menu_bar, "File", args, n);
  XtManageChild(cascade1);
  XmStringFree(tcs);

  /*	
   * Create "Options" PulldownMenu.
   */

  n = 0;
  menu_pane2 = XmCreatePulldownMenu(menu_bar, "menu_pane2", args, n);

  n = 0;
  tcs = XmStringCreateLtoR("Cut", charset);
  tcs1 = XmStringCreateLtoR("Shift+Del", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 't'); n++;
  XtSetArg(args[n], XmNacceleratorText, tcs1); n++;
#ifdef hpux
  XtSetArg(args[n], XmNaccelerator, "Shift<Key>DeleteChar:"); n++;
#else /* hpux */
  XtSetArg(args[n], XmNaccelerator, "Shift<Key>Delete:"); n++;
#endif /* hpux */
  cut_button = XmCreatePushButton(menu_pane2, "Cut", args, n);
  XtAddCallback(cut_button, XmNactivateCallback, MenuCB, (XtPointer)MENU_CUT);
  XtManageChild(cut_button);
  XtSetSensitive(cut_button, False);
  XmStringFree(tcs);
  XmStringFree(tcs1);

  n = 0;
  tcs = XmStringCreateLtoR("Copy", charset);
  tcs1 = XmStringCreateLtoR("Ctrl+Ins", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'C'); n++;
  XtSetArg(args[n], XmNacceleratorText, tcs1); n++;
#ifdef hpux
  XtSetArg(args[n], XmNaccelerator, "Ctrl<Key>InsertChar:"); n++;
#else /* hpux */
  XtSetArg(args[n], XmNaccelerator, "Ctrl<Key>Insert:"); n++;
#endif /* hpux */
  copy_button = XmCreatePushButton(menu_pane2, "Copy", args, n);
  XtAddCallback(copy_button, XmNactivateCallback, MenuCB, (XtPointer)MENU_COPY);
  XtManageChild(copy_button);
  XtSetSensitive(copy_button, False);
  XmStringFree(tcs);
  XmStringFree(tcs1);

  n = 0;
  tcs = XmStringCreateLtoR("Paste", charset);
  tcs1 = XmStringCreateLtoR("Shift+Ins", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'P'); n++;
  XtSetArg(args[n], XmNacceleratorText, tcs1); n++;
#ifdef hpux
  XtSetArg(args[n], XmNaccelerator, "Shift<Key>InsertChar:"); n++;
#else /* hpux */
  XtSetArg(args[n], XmNaccelerator, "Shift<Key>Insert:"); n++;
#endif /* hpux */
  paste_button = XmCreatePushButton(menu_pane2, "Paste", args, n); n++;
  XtAddCallback(paste_button, XmNactivateCallback, MenuCB, (XtPointer)MENU_PASTE);
  XtManageChild(paste_button);
  XmStringFree(tcs);
  XmStringFree(tcs1);

  n = 0;
  tcs = XmStringCreateLtoR("Clear", charset);
  tcs1 = XmStringCreateLtoR("Del", charset);
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'e'); n++;

  /* 
   * pseudo accelerator - Text already handles this action appropriately 
   */

  XtSetArg(args[n], XmNacceleratorText, tcs1); n++;
  clear_button = XmCreatePushButton(menu_pane2, "Clear", args, n);
  XtAddCallback(clear_button, XmNactivateCallback, MenuCB, (XtPointer)MENU_CLEAR);
  XtManageChild(clear_button);
  XtSetSensitive(clear_button, False);
  XmStringFree(tcs);
  XmStringFree(tcs1);

  n = 0;
  tcs = XmStringCreateLtoR("Edit", charset);
  XtSetArg(args[n], XmNsubMenuId, menu_pane2);  n++;
  XtSetArg(args[n], XmNlabelString, tcs); n++;
  XtSetArg(args[n], XmNmnemonic, 'E'); n++;
  cascade2 = XmCreateCascadeButton(menu_bar, "Edit", args, n);
  XtManageChild(cascade2);
  XmStringFree(tcs);

  /*	Create "Help" button.
   */

  n = 0;
  cascade3 = XmCreateCascadeButton(menu_bar, "Help", args, n);
  XtAddCallback(cascade3, XmNactivateCallback, MenuCB, (XtPointer)MENU_HELP);
  XtManageChild(cascade3);

  n = 0;
  XtSetArg(args[n], XmNmenuHelpWidget, cascade3);  n++;
  XtSetValues(menu_bar, args, n);
  return(menu_bar);
}

/*-------------------------------------------------------------
**	CreateText
**		Create Text.
*/
static void CreateText(parent)
Widget parent;
{
  Arg args[10];		/*  arg list		*/
  register int n;	/*  arg count		*/

  /* create text widget */
  n = 0;
  XtSetArg(args[n], XmNrows, 24);  n++;
  XtSetArg(args[n], XmNcolumns, 80);  n++;
  XtSetArg(args[n], XmNresizeWidth, False);  n++;
  XtSetArg(args[n], XmNresizeHeight, False);  n++;
  XtSetArg(args[n], XmNscrollVertical, True);  n++;
  XtSetArg(args[n], XmNscrollHorizontal, True);  n++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;

  text = XmCreateScrolledText(parent, "text", args, n);

  /* add value changed callback */
  XtAddCallback(text, XmNmodifyVerifyCallback, FileChangedCB, (XtPointer)NULL);
  return;
}

/*-------------------------------------------------------------
**	main
**		Initialize toolkit.
**		Create MainWindow and subwidgets.
**		Realize toplevel widgets.
**		Process events.
*/
#ifdef DEC_MOTIF_BUG_FIX
int main (argc,argv)
#else
void main (argc,argv)
#endif
int  argc;
char **argv;
{
  Display *display;	   /*  Display			*/
  Widget main;		   /*  MainWindow	 	*/
  Widget menu_bar;	   /*  RowColumn	 	*/

  Arg args[10];		/*  arg list		*/
  register int	n;	/*  arg count		*/
  char	*progname;  	/* program name without the full pathname */

  if (progname=strrchr(argv[0], '/')){
    progname++;
  }
  else	{
    progname = argv[0];
  }

  /*	Initialize toolkit and open display.
   */

  XtToolkitInitialize();
  app_context = XtCreateApplicationContext();
  display = XtOpenDisplay(app_context, NULL, progname, "XMdemos",
			  NULL, 0, &argc, argv);
  if (!display)
    {
      XtWarning("xmeditor: can't open display, exiting...");
      exit(0);
    }
  
  /*	Create ApplicationShell.
   */
	
 Shell1 = XtAppCreateShell(progname, "XMdemos",
			       applicationShellWidgetClass, 
			       display, NULL, 0);

  /*	Create MainWindow.
   */
  n = 0;
  XtSetArg(args[n], XmNshadowThickness, 0);  n++;
  main = XmCreateMainWindow(Shell1, "main", args, n);
  XtManageChild(main);


  /*	Create MenuBar in MainWindow.
   */
  menu_bar = CreateMenuBar(main);
  XtManageChild(menu_bar);


  /*	Create Text.
   */
  CreateText(main);
  XtManageChild(text);

  XmAddTabGroup(text);

  XtSetSensitive(text, False);

  /*	Realize toplevel widgets.
   */
  XtRealizeWidget(Shell1);

  /*	Process events.
   */
  XtAppMainLoop(app_context);
}

