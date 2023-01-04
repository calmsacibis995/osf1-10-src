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
static char *rcsid = "@(#)$RCSfile: xmpiano.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 21:07:59 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */
/****************************************************************************
 ****************************************************************************
 **
 **   File:         xmpiano.c
 **
 **   By:           Andrew deBlois
 **
 **   This application won't be able to play tunes on the pmax and sun
 **   since you can't change the tones generated bu XBell.
 **
 ****************************************************************************
 ****************************************************************************/

#include <math.h>
#include <stdio.h>
#include <Xm/XmP.h>
#include <Xm/Xm.h>
#include <X11/Shell.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingAP.h>
#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MenuShell.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrolledW.h>
#include <Xm/SelectioB.h>
#include <Xm/Separator.h>
#include "xmpiano.xbm"

#define APP_NAME  "xmpiano"
#define APP_CLASS "XMpiano"

#define REST         0
#define EIGHTH       1
#define EIGHTHSHARP  4
#define QUARTER      2
#define QUARTERSHARP 5
#define HALF         3
#define HALFSHARP    6
#define NOTE_COUNT   12   /* number of white keys */
#define LOCAL_HOST   "local"

#define BASE_F      (double)246.9413
#define DURATION    150

#define WKEY_WIDTH  48
#define BKEY_WIDTH  48
#define OFFSET      20
#define KEYS        21
#define WKEY_HEIGHT 140

typedef struct _NoteRec
{
  Display  *display;
  int       noteType;       /* type of note or rest. */
  int       noteDuration;
  int       noteNumber;     /* index of note to play */
  int       noteIndex;      /* which number note this is. used for positioning. */
  double    baseFrequency;  /* base frequency is the frequency of middle C, 0 = rest */
                            /* to shift an octive, this value is changed.  */
  int       ledgerLine;
  struct _NoteRec *next;
} NoteRec;

typedef struct _StaffRec
{
  Display *display;
  NoteRec *notes;
} StaffRec;


/* Function Declarations */

#ifndef _NO_PROTO
/* KEYBOARD */
Widget BuildKeys         (Widget parent, int noteCount, int wkeyWidth, int wkeyHeight);
Widget CreateKeyboard    (Widget parent);

/* SCORE */
Widget DrawNotes         (Widget staff);
void   DrawStaffCB       (Widget staff, XtPointer clientData, XtPointer callData);
void   SetIcon           (Widget w, char *cursorBits, char *maskBits);
void   DrawNote          (Widget staff, NoteRec *note);
void   SetActiveNote     (Widget w, int noteType);
void   AddNoteAtPosn     (Widget staff, int y, int noteType);
void   AddNoteToStaffCB  (Widget staff, XtPointer clientData, XtPointer callData);
Widget AddNewStaff       (Widget score, Display *newDisplay, char *dspName);
void   PostStaffMenu     (Widget w, XtPointer clientData, XEvent *event, Boolean *dispatch);
Widget CreateStaffMenu   (Widget parent, Widget staff, char *dspname);

/* OTHER */
void   AddVoiceCB        (Widget staff, XtPointer clientData, XtPointer callData);
void   RemoveVoiceCB     (Widget staff, XtPointer clientData, XtPointer callData);
void   ClearVoiceCB      (Widget staff, XtPointer clientData, XtPointer callData);
void   PlayVoiceCB       (Widget staff, XtPointer clientData, XtPointer callData);
void   PlayAllCB         (Widget staff, XtPointer clientData, XtPointer callData);
void   SetAppIcon        (Widget shell);
void   GetBell           (Display *dpy);
void   SetBell           (Display *dpy, int pitch, int duration);
int    Pitch             (int note);
void   PlayNote          (Display *dsp, int note, int duration);
void   SoundCB           (Widget w, XtPointer note, XtPointer callData);
void   SetNoteCB         (Widget w, XtPointer clientData, XtPointer callData);
Widget CreateScore       (Widget parent);
Widget CreateNotebook    (Widget parent);
#else
/* KEYBOARD */
Widget BuildKeys         ();
Widget CreateKeyboard    ();

/* SCORE */
Widget DrawNotes         ();
void   DrawStaffCB       ();
void   SetIcon           ();
void   DrawNote          ();
void   SetActiveNote     ();
void   AddNoteAtPosn     ();
void   AddNoteToStaffCB  ();
Widget AddNewStaff       ();
void   PostStaffMenu     ();
Widget CreateStaffMenu   ();

/* OTHER */
void   AddVoiceCB        ();
void   RemoveVoiceCB     ();
void   ClearVoiceCB      ();
void   PlayVoiceCB       ();
void   PlayAllCB         ();
void   SetAppIcon        ();
void   GetBell           ();
void   SetBell           ();
int    Pitch             ();
void   PlayNote          ();
void   SoundCB           ();
void   SetNoteCB         ();
Widget CreateScore       ();
Widget CreateNotebook    ();
#endif


/* Globals */
XtAppContext context;
int          orig_percent, orig_pitch, orig_duration;
Widget       activeStaff;
Pixmap       eighthpix, quarterpix, halfpix, restpix;
Pixmap       eighthsharppix, quartersharppix, halfsharppix;
Pixmap       eighthmask, quartermask, halfmask, restmask;
Pixmap       eighthsharpmask, quartersharpmask, halfsharpmask;
int          activeNote;
GC           noteGC;
Widget       score;
Widget       key[50];


String fallback[] = {
  "XMpiano*highlightThickness:     		0",
  "XMpiano*borderWidth:      			0",
  "XMpiano*iconImage:     			xmpiano.bmp",
  "XMpiano*borderWidth:     			0",
  "XMpiano*margin:     				0",
  "XMpiano*wKey.background:     		white",
  "XMpiano*bKey.background:     		black",
  "XMpiano*wKey.foreground:     		black",
  "XMpiano*bKey.foreground:     		white",
  "XMpiano*wKey.width:     			48",
  "XMpiano*wKey.height:     			140",
  "XMpiano*wKey.shadowThickness:     		2",
  "XMpiano*bKey.shadowThickness:     		4",
  "XMpiano*wKey.armColor:     			grey85",
  "XMpiano*bKey.armColor:     			grey0",
  "XMpiano*wKey.topShadowColor:     		white",
  "XMpiano*bKey.bottomShadowColor:     		black",
  "XMpiano*wKey.topShadowColor:     		grey60",
  "XMpiano*bKey.bottomShadowColor:     		grey20",
  "XMpiano*bKey.labelString:     		",
  "XMpiano*wKey.labelString:     		",

  "XMpiano*keyboard.marginWidth:		10",
  "XMpiano*keyboard.marginHeight:		10",

  "XMpiano*scoreWin.height:			150",
  "XMpiano*staff.width:				900",
  "XMpiano*staff.height:			100",

  "XMpiano*popupBtn1.labelString:		Add Voice",
  "XMpiano*popupBtn2.labelString:		Remove Voice",
  "XMpiano*popupBtn3.labelString:		Clear Voice",
  "XMpiano*popupBtn4.labelString:		Play Voice",
  "XMpiano*popupBtn5.labelString:		Play All",
  "XMpiano*popupBtn6.labelString:		Save Voice",
  "XMpiano*popupBtn7.labelString:		Load Voice",
  "XMpiano*popupBtn8.labelString:		Quit",

  "XMpiano*notebook.orientation:		horizontal",
  "XMpiano*notebook.adjustLast:			false",
  "XMpiano*notebook*paneMaximum:		40",

  "XMpiano*dspPromptDlog.labelString:		Enter name of display to connect to:",
  "XMpiano*warnDlog.messageString:		Error in connecting to display",
  
  NULL
  };




/********************************** Callbacks *************************************/


/*--------------------------------------------------------------------*
 |                               QuitCB                               |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void QuitCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void QuitCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  exit(0);
}


/*--------------------------------------------------------------------*
 |                            DoAddVoiceCB                            |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void DoAddVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void DoAddVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  XmSelectionBoxCallbackStruct *cb = (XmSelectionBoxCallbackStruct *)callData;
  Widget        score = (Widget)clientData;
  String        dspName;
  Display      *newDisplay;
  String        appName, appClass;
  Widget        newStaff;
  static Widget dlog = NULL;
  int           n, argc = 0;
  Arg           args[5];


  XtGetApplicationNameAndClass(XtDisplay(w), &appName, &appClass);
  XmStringGetLtoR(cb->value, XmSTRING_DEFAULT_CHARSET, &dspName);

  newDisplay = XtOpenDisplay(context, dspName, appName, appClass, NULL, 0, &argc, NULL);

  if (newDisplay != NULL)
    newStaff = AddNewStaff(score, newDisplay, dspName);
  else
    {
      if (dlog == NULL)
	{
	  n = 0;
	  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
	  XtSetArg(args[n], XmNdialogType,  XmDIALOG_ERROR);                  n++;
	  dlog = XmCreateWarningDialog(score, "warnDlog", args, n);
	}
      XtManageChild(dlog);
    }
  if (dspName) XtFree(dspName);
}


/*--------------------------------------------------------------------*
 |                            AddVoiceCB                              |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void AddVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void AddVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget        score = (Widget)clientData;
  Cardinal      n;
  Arg           args[5];
  static Widget dlog = NULL;


  if (dlog == NULL)
    {
      n = 0;
      XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
      XtSetArg(args[n], XmNdialogType,  XmDIALOG_PROMPT);                 n++;
      dlog = XmCreatePromptDialog(score, "dspPromptDlog", args, n);

      XtAddCallback(dlog, XmNokCallback, DoAddVoiceCB, (XtPointer)score);
    }

  XtManageChild(dlog);
}


/*--------------------------------------------------------------------*
 |                           RemoveVoiceCB                            |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void RemoveVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void RemoveVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget staff = (Widget)clientData;

  XtDestroyWidget(staff);
}


/*--------------------------------------------------------------------*
 |                            ClearVoiceCB                            |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void ClearVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void ClearVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget   staff = (Widget)clientData;
  StaffRec *staffData;
  NoteRec  *notes, *np;


  XtVaGetValues(staff, XmNuserData, &staffData, NULL);

  if (staffData != NULL)
    {
      while (staffData->notes != NULL)
	{
	  np = staffData->notes;
	  staffData->notes = staffData->notes->next;
	  XtFree((char *)np);
	}

      XClearArea(XtDisplay(staff), XtWindow(staff), 0, 0, 0, 0, TRUE);
    }
}


/*--------------------------------------------------------------------*
 |                            DisarmKey                               |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void DisarmKey (XtPointer clientData, XtIntervalId *id)
#else
void DisarmKey (clientData, id)
     XtPointer clientData;
     XtIntervalId *id;
#endif
{
  Widget key = (Widget) clientData;
  XEvent event;

  XtCallActionProc(key, "Disarm", &event, NULL, 0);
}


/*--------------------------------------------------------------------*
 |                             PlayNotes                              |
 | Note that bell duration and interval timing are defined in X as    |
 | based on milliseconds.  Lets hope so...                            |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void PlayNotes (XtPointer clientData, XtIntervalId *id)
#else
void PlayNotes (clientData, id)
     XtPointer clientData;
     XtIntervalId *id;
#endif
{
  NoteRec *note = (NoteRec *)clientData;
  XEvent event;

  if (note != NULL)
    {
      XSync(note->display, FALSE);
      /* if not a rest, set timer to unpress the key. */
      if (note->noteType != REST)
	{
	  PlayNote(note->display, note->noteNumber, note->noteDuration);
#ifdef CHORDS
	  if (TRUE)
#else
	  if (XtDisplay(key[1]) == note->display)
#endif
	    {
	      XtCallActionProc(key[note->noteNumber], "Arm", &event, NULL, 0);
	      XtAppAddTimeOut(context, note->noteDuration, DisarmKey, key[note->noteNumber]);
	      XFlush(note->display);
	    }
	}

      /* set timer to play next note. */
      XtAppAddTimeOut(context, note->noteDuration, PlayNotes, note->next);
    }
}


/*--------------------------------------------------------------------*
 |                            PlayVoiceCB                             |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void PlayVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void PlayVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget    staff = (Widget)clientData;
  StaffRec *staffData;
  NoteRec  *notes;


  XtVaGetValues(staff, XmNuserData, &staffData, NULL);
  if (staffData != NULL)
    XtAppAddTimeOut(context, 1, PlayNotes, staffData->notes);
}


/*--------------------------------------------------------------------*
 |                             PlayAllCB                              |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void PlayAllCB (Widget staff, XtPointer clientData, XtPointer callData)
#else
void PlayAllCB (staff, clientData, callData)
     Widget staff;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget    score = (Widget)clientData;
  StaffRec *staffData;
  NoteRec  *notes;
  Widget   *voice;
  int       numVoices, n;

  /* play the notes of all voices. */
  XtVaGetValues(score, XmNchildren, &voice, XmNnumChildren, &numVoices, NULL);

  for (n = 0;  n < numVoices;  n++)
    {
      XtVaGetValues(voice[n], XmNuserData, &staffData, NULL);
      if (staffData != NULL)
	XtAppAddTimeOut(context, 1, PlayNotes, staffData->notes);
    }
}


/*--------------------------------------------------------------------*
 |                            DoSaveVoiceCB                           |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void DoSaveVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void DoSaveVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget    staff = (Widget)clientData;
  XmFileSelectionBoxCallbackStruct *fdata = (XmFileSelectionBoxCallbackStruct *)callData;
  StaffRec *staffData;
  NoteRec  *note;
  FILE     *fp;
  char     *fileName;
  static Widget errDlog = NULL;


  if (fdata->length > 0)
    {
      XmStringGetLtoR(fdata->value, XmSTRING_DEFAULT_CHARSET, &fileName);

      XtVaGetValues(staff, XmNuserData, &staffData, NULL);
      if (staffData != NULL)
	{
	  fp = fopen(fileName, "w");

	  for (note = staffData->notes;  note != NULL;  note = note->next)
	    fprintf(fp, "%d %d %d %d %f %d\n",
		    note->noteType,
		    note->noteDuration,
		    note->noteNumber,
		    note->noteIndex,
		    note->baseFrequency,
		    note->ledgerLine);
	  fclose(fp);
	}

      if (fileName) XtFree(fileName);
    }
}



/*--------------------------------------------------------------------*
 |                            SaveVoiceCB                             |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void SaveVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void SaveVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget        staff = (Widget)clientData;
  static Widget fsdlog = NULL;
  static Widget oldStaff;


  if (fsdlog == NULL)
    {
      fsdlog = XmCreateFileSelectionDialog(staff, "saveDlog", NULL, 0);
      XtVaSetValues(fsdlog, XmNautoUnmanage, True, XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL, NULL);
    }
  else
    {
      XtRemoveCallback(fsdlog, XmNokCallback, DoSaveVoiceCB, oldStaff);
    }

  XtAddCallback(fsdlog, XmNokCallback, DoSaveVoiceCB, staff);
  oldStaff = staff;

  XtManageChild(fsdlog);
}



/*--------------------------------------------------------------------*
 |                           DoLoadVoiceCB                            |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void DoLoadVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void DoLoadVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget    staff = (Widget)clientData;
  XmFileSelectionBoxCallbackStruct *fdata = (XmFileSelectionBoxCallbackStruct *)callData;
  StaffRec *staffData;
  NoteRec  *note, *tail;
  FILE     *fp;
  Boolean   done = FALSE;
  int       noteOffset;
  char     *fileName;


  if (fdata->length > 0)
    {
      XmStringGetLtoR(fdata->value, XmSTRING_DEFAULT_CHARSET, &fileName);

      fp = fopen(fileName, "r");
      if (fileName) XtFree(fileName);

      if (fp != NULL)
	{
	  XtVaGetValues(staff, XmNuserData, &staffData, NULL);
	  if (staffData->notes == NULL)
	    {
	      tail = NULL;
	      noteOffset = 0;
	    }
	  else
	    for (tail=staffData->notes, noteOffset = 1;
		 tail->next != NULL;
		 tail = tail->next, noteOffset++);
	  
	  while (!done)
	    {
	      note = (NoteRec *) XtMalloc(sizeof(NoteRec));
	      if (fscanf(fp, "%d %d %d %d %f %d\n",
			 &note->noteType,
			 &note->noteDuration,
			 &note->noteNumber,
			 &note->noteIndex,
			 &note->baseFrequency,
			 &note->ledgerLine)
		  > 0)
		{
		  note->noteIndex += noteOffset;
		  note->display = staffData->display;
		  note->next = NULL;
		  
		  if (tail == NULL)
		    {
		      staffData->notes = note;
		      tail = note;
		    }
		  else
		    {
		      tail->next = note;
		      tail = tail->next;
		    }
		}
	      else
		{
		  XtFree((XtPointer)note);
		  done = TRUE;
		}
	    }
	  fclose(fp);
	}
      
      XClearArea(XtDisplay(w), XtWindow(staff), 0, 0, 0, 0, TRUE);
    }
}




/*--------------------------------------------------------------------*
 |                            LoadVoiceCB                             |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void LoadVoiceCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void LoadVoiceCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  Widget        staff = (Widget)clientData;
  static Widget fsdlog = NULL;
  static Widget oldStaff;


  if (fsdlog == NULL)
    {
      fsdlog = XmCreateFileSelectionDialog(staff, "loadDlog", NULL, 0);
      XtVaSetValues(fsdlog, XmNautoUnmanage, True, XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL, NULL);
    }
  else
    {
      XtRemoveCallback(fsdlog, XmNokCallback, DoLoadVoiceCB, oldStaff);
    }

  XtAddCallback(fsdlog, XmNokCallback, DoLoadVoiceCB, staff);
  oldStaff = staff;

  XtManageChild(fsdlog);
}


/*--------------------------------------------------------------------*
 |                              DrawNotes                             |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget DrawNotes (Widget staff)
#else
Widget DrawNotes (staff)
     Widget staff;
#endif
{
  StaffRec *staffData;
  NoteRec  *notes, *np;


  XtVaGetValues(staff, XmNuserData, &staffData, NULL);
  if (staffData != NULL)
    for (np = staffData->notes;  np != NULL;  np = np->next)
      DrawNote(staff, np);
}



/*--------------------------------------------------------------------*
 |                             DrawStaffCB                            |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void DrawStaffCB (Widget staff, XtPointer clientData, XtPointer callData)
#else
void DrawStaffCB (staff, clientData, callData)
     Widget staff;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int           i, y;
  Dimension     width, height;  /* Not 'int'!! */
  XExposeEvent *exposeEvent = (XExposeEvent *)((XmDrawingAreaCallbackStruct *)callData)->event;

  /*
   * should take into account the expose event. Use:
   * exposeEvent->x
   * exposeEvent->y
   * exposeEvent->width
   * exposeEvent->height
   * exposeEvent->count
   */

  XtVaGetValues(staff, XmNwidth, &width, XmNheight, &height, NULL);

  for (i=4; i<=12; i+=2)
    {
      y = i*(int)height / 16;
      XDrawLine(XtDisplay(staff), XtWindow(staff), DefaultGCOfScreen(XtScreen(staff)),
		0, y, width, y);
    }

  DrawNotes(staff);
}




/*-------------------------------------------------------------*
 |                          SetAppIcon()                       |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void SetAppIcon(Widget shell)
#else
void SetAppIcon(shell)
     Widget shell;
#endif
{
  Pixmap  iconPixmap;

  iconPixmap = XCreateBitmapFromData(XtDisplay(shell), XtScreen(shell)->root,
				     xmpiano_bits, xmpiano_width, xmpiano_height);

  XtVaSetValues(shell, XmNiconPixmap, iconPixmap, NULL);
}



/*--------------------------------------------------------------------*
 |                            GetBell                                 |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void GetBell(Display *dpy)
#else
void GetBell(dpy)
     Display *dpy;
#endif
{
  XKeyboardState stateValues;

  XGetKeyboardControl(dpy, &stateValues);

  orig_percent  = stateValues.bell_percent;
  orig_pitch    = stateValues.bell_pitch;
  orig_duration = stateValues.bell_duration;
}



/*--------------------------------------------------------------------*
 |                             SetBell                                |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void SetBell(Display *dpy, int pitch, int duration)
#else
void SetBell(dpy, pitch, duration)
     Display *dpy;
     int pitch;
     int duration;
#endif
{
  XKeyboardControl controlValues;
  unsigned long    valueMask = KBBellPercent | KBBellPitch | KBBellDuration;

  controlValues.bell_percent  = orig_percent;
  controlValues.bell_pitch    = pitch;
  controlValues.bell_duration = duration;
  
  XChangeKeyboardControl(dpy, valueMask, &controlValues);
}



/*--------------------------------------------------------------------*
 |                            Pitch                                   |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
int Pitch (int note)
#else
int Pitch (note)
     int note;
#endif
{
  double x, m, n, f;

  /* notes are calculated from the base frequency of BASE_F. */
  /* This is the first note on the keyboard.                 */
  /* The frequency of a note = 2^(index / 12).               */

  x = (double)2.0;
  m = (double)note;
  n = (double)12.0;

  f = BASE_F * pow(x, (m/n));

  return((int)f);
}


/*--------------------------------------------------------------------*
 |                             PlayNote                               |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void PlayNote (Display *dsp, int note, int duration)
#else
void PlayNote (dsp, note, duration)
     Display *dsp;
     int note;
     int duration;
#endif
{
  SetBell(dsp, Pitch(note), duration);

  XBell(dsp, 100);

  SetBell(dsp, orig_pitch, orig_duration);
}


/*--------------------------------------------------------------------*
 |                             SoundCB                                |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void SoundCB (Widget w, XtPointer note, XtPointer callData)
#else
void SoundCB (w, note, callData)
     Widget w;
     XtPointer note;
     XtPointer callData;
#endif
{
  XmPushButtonCallbackStruct *cb = (XmPushButtonCallbackStruct *)callData;


  /* only play the note if this is a true arm event. */

  if (cb->event != NULL)
    PlayNote(XtDisplay(w), (int)note, DURATION);
}



/*--------------------------------------------------------------------*
 |                             BuildKeys                              |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget BuildKeys (Widget parent, int noteCount, int wkeyWidth, int wkeyHeight)
#else
Widget BuildKeys (parent, noteCount, wkeyWidth, wkeyHeight)
     Widget parent;
     int noteCount;
     int wkeyWidth;
     int wkeyHeight;
#endif
{
  Pixmap   iconPixmaps[9];
  int      i, j = 0;
  Boolean  pixmapsSet = FALSE;


  XtVaSetValues(parent, XmNfractionBase, noteCount * 10, NULL);

  j = 0;
  for (i=0; i<noteCount; i++)
    if ( !((i+1)%7) || !((i-2)%7) ) j++;
    else
      {
	j = j+2;
	key[j] = XtVaCreateManagedWidget("bKey", xmPushButtonWidgetClass, parent,
					 XmNleftAttachment,   XmATTACH_POSITION,
					 XmNleftPosition,     i*10 + 7,
					 XmNrightAttachment,  XmATTACH_POSITION,
					 XmNrightPosition,    i*10 + 13,
					 XmNtopAttachment,    XmATTACH_FORM,
					 XmNbottomAttachment, XmATTACH_POSITION,
					 XmNbottomPosition,   78,
					 NULL);
	XtAddCallback(key[j], XmNarmCallback, SoundCB, (XtPointer)j);
      }

  j = 0;
  for (i=0; i<noteCount; i++)
    {
      if ( !(i%7) || !((i-3)%7) ) j--;
      j = j+2;
      key[j] = XtVaCreateManagedWidget("wKey", xmPushButtonWidgetClass, parent,
				       XmNleftAttachment,   XmATTACH_POSITION,
				       XmNleftPosition,     i*10,
				       XmNrightAttachment,  XmATTACH_POSITION,
				       XmNrightPosition,    (i+1)*10,
				       XmNtopAttachment,    XmATTACH_FORM,
				       XmNbottomAttachment, XmATTACH_FORM,
				       NULL);
      XtAddCallback(key[j], XmNarmCallback, SoundCB, (XtPointer)j);

      if (i==0)
	{
	  int          x, y, junk;
	  unsigned int bjunk;
	  Window       wjunk;

	  XQueryPointer(XtDisplay(key[j]), RootWindowOfScreen(XtScreen(key[j])),
			&wjunk, &wjunk, &x, &y, &junk, &junk, &bjunk);
	  if (x+y == 0)
	    {
	      Display       *dsp = XtDisplay(parent);
	      Window         win = RootWindowOfScreen(XtScreen(parent));
	      unsigned int   d   = DefaultDepthOfScreen(XtScreen(parent));
	      unsigned long  fg, bg, blankbg;
  
	      XtVaGetValues(key[j], XmNforeground, &fg, XmNbackground, &blankbg, XmNarmColor, &bg, NULL);


	      iconPixmaps[0] = XCreatePixmapFromBitmapData(dsp, win, blank_bits, blank_width, blank_height, fg, blankbg, d);
	      iconPixmaps[1] = XCreatePixmapFromBitmapData(dsp, win, vj_bits, vj_width, vj_height, fg, bg, d);
	      iconPixmaps[2] = XCreatePixmapFromBitmapData(dsp, win, dd_bits, dd_width, dd_height, fg, bg, d);
	      iconPixmaps[3] = XCreatePixmapFromBitmapData(dsp, win, ad_bits, ad_width, ad_height, fg, bg, d);
	      iconPixmaps[4] = XCreatePixmapFromBitmapData(dsp, win, al_bits, al_width, al_height, fg, bg, d);
	      iconPixmaps[5] = XCreatePixmapFromBitmapData(dsp, win, sm_bits, sm_width, sm_height, fg, bg, d);
	      iconPixmaps[6] = XCreatePixmapFromBitmapData(dsp, win, ec_bits, ec_width, ec_height, fg, bg, d);
	      iconPixmaps[7] = XCreatePixmapFromBitmapData(dsp, win, mc_bits, mc_width, mc_height, fg, bg, d);
	      iconPixmaps[8] = XCreatePixmapFromBitmapData(dsp, win, gl_bits, gl_width, gl_height, fg, bg, d);

	      pixmapsSet = TRUE;
	    }
	}
      if (pixmapsSet)
	{
	  XtVaSetValues(key[j],
			XmNlabelType,   XmPIXMAP,
			XmNlabelPixmap, iconPixmaps[0],
			XmNarmPixmap,   iconPixmaps[(i%8)+1],
			XmNmarginLeft,  0,
			XmNmarginRight, 0,
			XmNmarginTop,   100,
			NULL);
	}
    }

}


/*--------------------------------------------------------------------*
 |                           CreateKeyboard                           |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget CreateKeyboard(Widget parent)
#else
Widget CreateKeyboard(parent)
     Widget parent;
#endif
{
  int    i, j = 0;
  Widget keyBoard, wKey, bKey;


  keyBoard = XtVaCreateWidget("keyBoard", xmFormWidgetClass, parent, NULL);
  BuildKeys(keyBoard, NOTE_COUNT, WKEY_WIDTH, WKEY_HEIGHT);

  XtManageChild(keyBoard);
  return(keyBoard);
}


/*--------------------------------------------------------------------*
 |                            SetIcon                                 |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void SetIcon (Widget w, char *cursorBits, char *maskBits)
#else
void SetIcon (w, cursorBits, maskBits)
     Widget w;
     char *cursorBits;
     char *maskBits;
#endif
{
  Pixel     fgPix, bgPix;
  Cursor    cursor;
  Pixmap    cursorPixmap, maskPixmap;
  XColor    xcolors[2];
  unsigned  int width  = 16;
  unsigned  int height = 16;
  Display  *dsp = XtDisplay(w);
  


  cursorPixmap = XCreateBitmapFromData (dsp, DefaultRootWindow(dsp), cursorBits, width, height);
  maskPixmap   = XCreateBitmapFromData (dsp, DefaultRootWindow(dsp), maskBits,   width, height);
  
  xcolors[0].pixel = BlackPixelOfScreen(DefaultScreenOfDisplay(dsp));
  xcolors[1].pixel = WhitePixelOfScreen(DefaultScreenOfDisplay(dsp));

  XQueryColors(dsp, DefaultColormapOfScreen(DefaultScreenOfDisplay(dsp)), xcolors, 2);

  cursor = XCreatePixmapCursor(dsp, cursorPixmap, maskPixmap,
			       &(xcolors[0]), &(xcolors[1]), note_x_hot, note_y_hot);

  XFreePixmap (dsp, cursorPixmap);
  XFreePixmap (dsp, maskPixmap);
  
  XDefineCursor(dsp, XtWindow(w), cursor);
}


/*--------------------------------------------------------------------*
 |                             DrawNote                               |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void DrawNote (Widget staff, NoteRec *note)
#else
void DrawNote (staff, note)
     Widget staff;
     NoteRec *note;
#endif
{
  Dimension  width, height;
  Pixmap     notePix, notePixMask;
  int        x, y;


  switch (note->noteType)
    {
    case EIGHTH:	notePix = eighthpix;		notePixMask = eighthmask;	break;
    case EIGHTHSHARP:	notePix = eighthsharppix;	notePixMask = eighthsharpmask;	break;
    case QUARTER:	notePix = quarterpix;		notePixMask = quartermask;	break;
    case QUARTERSHARP:	notePix = quartersharppix;	notePixMask = quartersharpmask;	break;
    case HALF:		notePix = halfpix;		notePixMask = halfmask;		break;
    case HALFSHARP:	notePix = halfsharppix;		notePixMask = halfsharpmask;	break;
    case REST:		notePix = restpix;		notePixMask = restmask;		break;
    }

  XtVaGetValues(staff, XmNwidth, &width, XmNheight, &height, NULL);


  x = note->noteIndex * 15;
  y = (15 - note->ledgerLine) * (int)height / 16 - (note_height/2) - 4;

  /* if the position is off the right side of the staff, resize it. */
  if ((x + note_width) > (int)width) XtVaSetValues(staff, XmNwidth, x + 2*note_width, NULL);

  XSetClipMask  (XtDisplay(staff), noteGC, notePixMask);
  XSetClipOrigin(XtDisplay(staff), noteGC, x, y);
  XCopyArea(XtDisplay(staff), notePix, XtWindow(staff), noteGC,
	    0, 0, note_width, note_height,
	    x, y);
}


/*--------------------------------------------------------------------*
 |                            SetActiveNote                           |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void SetActiveNote (Widget w, int noteType)
#else
void SetActiveNote (w, noteType)
     Widget w;
     int noteType;
#endif
{
  Pixmap mask = activeNote;


  switch(noteType)
    {
    case EIGHTH:
      mask = eighthmask;
      activeNote = EIGHTH;
      SetIcon(score, eighth_bits,  eighth_bits);
      break;

    case EIGHTHSHARP:
      mask = eighthsharpmask;
      activeNote = EIGHTHSHARP;
      SetIcon(score, eighth_sharp_bits,  eighth_sharp_bits);
      break;

    case QUARTER:
      mask = quartermask;
      activeNote = QUARTER;
      SetIcon(score, quarter_bits, quarter_bits);
      break;

    case QUARTERSHARP:
      mask = quartersharpmask;
      activeNote = QUARTERSHARP;
      SetIcon(score, quarter_sharp_bits, quarter_sharp_bits);
      break;

    case HALF:
      mask = halfmask;
      activeNote = HALF;
      SetIcon(score, half_bits,    half_bits);
      break;

    case HALFSHARP:
      mask = halfsharpmask;
      activeNote = HALFSHARP;
      SetIcon(score, half_sharp_bits,    half_sharp_bits);
      break;

    case REST:
      mask = restmask;
      activeNote = REST;
      SetIcon(score, rest_bits,    rest_bits);
      break;
    }

  XSetClipMask(XtDisplay(w), noteGC, mask);
}



/*--------------------------------------------------------------------*
 |                              SetNoteCB                             |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void SetNoteCB (Widget w, XtPointer clientData, XtPointer callData)
#else
void SetNoteCB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int noteType = (int)clientData;

  SetActiveNote (w, noteType);
}



/*--------------------------------------------------------------------*
 |                             NoteNumber                             |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
int NoteNumber (int ledgerLine, Boolean isASharp)
#else
int NoteNumber (ledgerLine, isASharp)
     int ledgerLine;
     Boolean isASharp;
#endif
{
  int n = 0;

  switch (ledgerLine)
    {
    case 1:  n = 1;  break;
    case 2:  n = 3;  break;
    case 3:  n = 5;  break;
    case 4:  n = 6;  break;
    case 5:  n = 8;  break;
    case 6:  n = 10; break;
    case 7:  n = 12; break;
    case 8:  n = 13; break;
    case 9:  n = 15; break;
    case 10: n = 17; break;
    case 11: n = 18; break;
    case 12: n = 20; break;
    }

  if (isASharp)
    return (n+1);
  else
    return (n);
}



/*--------------------------------------------------------------------*
 |                           AddNoteAtPosn                            |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void AddNoteAtPosn (Widget staff, int y, int noteType)
#else
void AddNoteAtPosn (staff, y, noteType)
     Widget staff;
     int y;
     int noteType;
#endif
{
  int        ledgerLine, noteCount, noteDuration;
  Dimension  height;
  StaffRec  *staffData;
  NoteRec   *noteList, *currentNoteList, *np;
  Boolean    isASharp = FALSE;


  /* find the corresponding ledger line in the staff. */
  XtVaGetValues(staff, XmNheight, &height, NULL);
  ledgerLine = 15 - (16 * y / (int)height);

  /* round up to G and down to middle C. */
  if (ledgerLine <  1) ledgerLine = 1;
     else
  if (ledgerLine > 12) ledgerLine = 12;

  switch (noteType)
    {
    case EIGHTH:	noteDuration = DURATION;				break;
    case EIGHTHSHARP:	noteDuration = DURATION;	isASharp = TRUE;	break;
    case QUARTER:	noteDuration = DURATION*2;				break;
    case QUARTERSHARP:	noteDuration = DURATION*2;	isASharp = TRUE;	break;
    case HALF:		noteDuration = DURATION*4;				break;
    case HALFSHARP:	noteDuration = DURATION*4;	isASharp = TRUE;	break;
    case REST:		noteDuration = DURATION;				break;
    default:            noteDuration = 0;
    }

  /* get the staff info - this tells the display to use. */
  XtVaGetValues(staff, XmNuserData, &staffData, NULL);
  if (staffData == NULL) printf("OH NO! - no data set for staff!\n");


  noteList = (NoteRec *)XtMalloc(sizeof(NoteRec));
  noteList->display       = staffData->display;
  noteList->noteType      = noteType;
  noteList->noteDuration  = noteDuration;
  noteList->noteNumber    = NoteNumber(ledgerLine, isASharp);
  noteList->ledgerLine    = ledgerLine;
  noteList->baseFrequency = BASE_F;
  noteList->next          = NULL;

  /* get the current list of notes. */
  currentNoteList = staffData->notes;

  /* find out how many there are. */
  for (noteCount=1, np=currentNoteList;  ((np != NULL) && (np->next != NULL));  np=np->next, noteCount++);

  if (np == NULL)
    {
      staffData->notes = noteList;
      noteList->noteIndex = noteCount;
    }
  else
    {
      np->next = noteList;
      noteList->noteIndex = noteCount+1;
    }

  DrawNote(staff, noteList);
}


/*--------------------------------------------------------------------*
 |                          AddNoteToStaffCB                          |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void AddNoteToStaffCB (Widget staff, XtPointer clientData, XtPointer callData)
#else
void AddNoteToStaffCB (staff, clientData, callData)
     Widget staff;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  XmDrawingAreaCallbackStruct *cb = (XmDrawingAreaCallbackStruct *)callData;
  XButtonEvent *btnEvent = (XButtonEvent *)cb->event;
  int           vposn, hposn;

  
  if ((btnEvent->button == Button1) && (btnEvent->type == ButtonPress))
    {
      AddNoteAtPosn(staff, btnEvent->y, activeNote);
    }
}


/*--------------------------------------------------------------------*
 |                             AddNewStaff                            |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget AddNewStaff (Widget score, Display *newDisplay, char *dspName)
#else
Widget AddNewStaff (score, newDisplay, dspName)
     Widget score;
     Display *newDisplay;
     char *dspName;
#endif
{
  Widget    staff;
  StaffRec *staffData;


  staffData = (StaffRec *) XtMalloc(sizeof(StaffRec));
  staffData->display = newDisplay;
  staffData->notes = NULL;

  staff = XtVaCreateManagedWidget("staff", xmDrawingAreaWidgetClass, score,
				  XmNuserData,     staffData,
				  XmNresizePolicy, XmRESIZE_NONE,
				  NULL);
  XtAddCallback(staff, XmNexposeCallback, DrawStaffCB,      NULL);
  XtAddCallback(staff, XmNinputCallback,  AddNoteToStaffCB, NULL);

  CreateStaffMenu(score, staff, dspName);

  return(staff);
}



/*--------------------------------------------------------------------*
 |                            PostStaffMenu  		              |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
void PostStaffMenu (Widget w, XtPointer clientData, XEvent *event, Boolean *dispatch)
#else
void PostStaffMenu (w, clientData, event, dispatch)
     Widget w;
     XtPointer clientData;
     XEvent *event;
     Boolean *dispatch;
#endif
{
  Widget        menu     = (Widget)clientData;
  XButtonEvent *btnEvent = (XButtonEvent *)event;
  int           button;


  XtVaGetValues(menu, XmNwhichButton, &button, NULL);
  if (btnEvent->button == button)
    {
      XmMenuPosition(menu, btnEvent);
      XtManageChild(menu);
    }
}


/*--------------------------------------------------------------------*
 |                           CreateStaffMenu                          |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget CreateStaffMenu (Widget score, Widget staff, char *dspName)
#else
Widget CreateStaffMenu (score, staff, dspName)
     Widget score;
     Widget staff;
     char *dspName;
#endif
{
  Widget menuShell, popupMenu, popupBtn[9];
 

  popupMenu = XmCreatePopupMenu(staff, "popupMenu", NULL, 0);
  XtAddEventHandler(staff, ButtonPressMask, False, PostStaffMenu, popupMenu);

  XtVaCreateManagedWidget(dspName, xmLabelWidgetClass,     popupMenu, NULL);
  XtVaCreateManagedWidget("line",  xmSeparatorWidgetClass, popupMenu, NULL);
  popupBtn[1] = XtVaCreateManagedWidget("popupBtn1", xmPushButtonWidgetClass, popupMenu, NULL);
  popupBtn[2] = XtVaCreateManagedWidget("popupBtn2", xmPushButtonWidgetClass, popupMenu, NULL);
  popupBtn[3] = XtVaCreateManagedWidget("popupBtn3", xmPushButtonWidgetClass, popupMenu, NULL);
  popupBtn[4] = XtVaCreateManagedWidget("popupBtn4", xmPushButtonWidgetClass, popupMenu, NULL);
  popupBtn[5] = XtVaCreateManagedWidget("popupBtn5", xmPushButtonWidgetClass, popupMenu, NULL);
  popupBtn[6] = XtVaCreateManagedWidget("popupBtn6", xmPushButtonWidgetClass, popupMenu, NULL);
  popupBtn[7] = XtVaCreateManagedWidget("popupBtn7", xmPushButtonWidgetClass, popupMenu, NULL);
  XtVaCreateManagedWidget("line",  xmSeparatorWidgetClass, popupMenu, NULL);
  popupBtn[8] = XtVaCreateManagedWidget("popupBtn8", xmPushButtonWidgetClass, popupMenu, NULL);

  XtAddCallback(popupBtn[1], XmNactivateCallback, AddVoiceCB,    score);
  XtAddCallback(popupBtn[2], XmNactivateCallback, RemoveVoiceCB, staff);
  XtAddCallback(popupBtn[3], XmNactivateCallback, ClearVoiceCB,  staff);
  XtAddCallback(popupBtn[4], XmNactivateCallback, PlayVoiceCB,   staff);
  XtAddCallback(popupBtn[5], XmNactivateCallback, PlayAllCB,     score);
  XtAddCallback(popupBtn[6], XmNactivateCallback, SaveVoiceCB,   staff);
  XtAddCallback(popupBtn[7], XmNactivateCallback, LoadVoiceCB,   staff);
  XtAddCallback(popupBtn[8], XmNactivateCallback, QuitCB,        NULL);

  return (menuShell);
}



/*--------------------------------------------------------------------*
 |                             CreateScore                            |
 | Creates the rowcolumn to holds the staffs.  Also creates an option |
 | menu for manipulating the staffs.                                  |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget CreateScore(Widget parent)
#else
Widget CreateScore(parent)
     Widget parent;
#endif
{
  Widget scoreWin, score, staff1;

  scoreWin  = XtVaCreateManagedWidget("scoreWin", xmScrolledWindowWidgetClass, parent,
				      XmNscrollingPolicy, XmAUTOMATIC, NULL);
  score     = XtVaCreateManagedWidget("score", xmRowColumnWidgetClass, scoreWin,
				      XmNadjustLast,   FALSE,
				      XmNnumColumns,   1,
				      XmNorientation,  XmVERTICAL,
				      XmNpacking,      XmPACK_COLUMN,
				      NULL);
  staff1 = AddNewStaff(score, XtDisplay(parent), LOCAL_HOST);

  return (score);
}


/*--------------------------------------------------------------------*
 |                           CreateNotebook                           |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget CreateNotebook(Widget parent)
#else
Widget CreateNotebook(parent)
     Widget parent;
#endif
{
  Widget   notebook, note[10];
  Pixel    fg, bg;
  Display *dsp = XtDisplay(parent);
  Window   win = RootWindowOfScreen(XtScreen(parent));
  int      d = DefaultDepthOfScreen(XtScreen(parent));
  

  notebook  = XtVaCreateManagedWidget("notebook", xmRowColumnWidgetClass, parent, NULL);

  XtVaGetValues(parent, XmNforeground, &fg, XmNbackground, &bg, NULL);

  eighthpix        = XCreatePixmapFromBitmapData(dsp, win, eighth_bits,        note_width, note_height, fg, bg, d);
  eighthsharppix   = XCreatePixmapFromBitmapData(dsp, win, eighth_sharp_bits,  note_width, note_height, fg, bg, d);

  quarterpix       = XCreatePixmapFromBitmapData(dsp, win, quarter_bits,       note_width, note_height, fg, bg, d);
  quartersharppix  = XCreatePixmapFromBitmapData(dsp, win, quarter_sharp_bits, note_width, note_height, fg, bg, d);

  halfpix          = XCreatePixmapFromBitmapData(dsp, win, half_bits,          note_width, note_height, fg, bg, d);
  halfsharppix     = XCreatePixmapFromBitmapData(dsp, win, half_sharp_bits,    note_width, note_height, fg, bg, d);

  restpix          = XCreatePixmapFromBitmapData(dsp, win, rest_bits,          note_width, note_height, fg, bg, d);

  eighthmask       = XCreatePixmapFromBitmapData(dsp, win, eighth_bits,        note_width, note_height, 1, 0, 1);
  eighthsharpmask  = XCreatePixmapFromBitmapData(dsp, win, eighth_sharp_bits,  note_width, note_height, 1, 0, 1);

  quartermask      = XCreatePixmapFromBitmapData(dsp, win, quarter_bits,       note_width, note_height, 1, 0, 1);
  quartersharpmask = XCreatePixmapFromBitmapData(dsp, win, quarter_sharp_bits, note_width, note_height, 1, 0, 1);

  halfmask         = XCreatePixmapFromBitmapData(dsp, win, half_bits,          note_width, note_height, 1, 0, 1);
  halfsharpmask    = XCreatePixmapFromBitmapData(dsp, win, half_sharp_bits,    note_width, note_height, 1, 0, 1);

  restmask         = XCreatePixmapFromBitmapData(dsp, win, rest_bits,          note_width, note_height, 1, 0, 1);

  note[1] = XtVaCreateManagedWidget("eighth",		xmPushButtonWidgetClass, notebook, NULL);
  note[2] = XtVaCreateManagedWidget("eighthsharp",	xmPushButtonWidgetClass, notebook, NULL);
  note[3] = XtVaCreateManagedWidget("quarter",		xmPushButtonWidgetClass, notebook, NULL);
  note[4] = XtVaCreateManagedWidget("quartersharp",	xmPushButtonWidgetClass, notebook, NULL);
  note[5] = XtVaCreateManagedWidget("half",		xmPushButtonWidgetClass, notebook, NULL);
  note[6] = XtVaCreateManagedWidget("halfsharp",	xmPushButtonWidgetClass, notebook, NULL);
  note[7] = XtVaCreateManagedWidget("rest",		xmPushButtonWidgetClass, notebook, NULL);

  XtAddCallback(note[1], XmNactivateCallback, SetNoteCB, (XtPointer)EIGHTH);
  XtAddCallback(note[2], XmNactivateCallback, SetNoteCB, (XtPointer)EIGHTHSHARP);
  XtAddCallback(note[3], XmNactivateCallback, SetNoteCB, (XtPointer)QUARTER);
  XtAddCallback(note[4], XmNactivateCallback, SetNoteCB, (XtPointer)QUARTERSHARP);
  XtAddCallback(note[5], XmNactivateCallback, SetNoteCB, (XtPointer)HALF);
  XtAddCallback(note[6], XmNactivateCallback, SetNoteCB, (XtPointer)HALFSHARP);
  XtAddCallback(note[7], XmNactivateCallback, SetNoteCB, (XtPointer)REST);

  XtVaSetValues(note[1], XmNlabelType, XmPIXMAP, XmNlabelPixmap, eighthpix,       NULL);
  XtVaSetValues(note[2], XmNlabelType, XmPIXMAP, XmNlabelPixmap, eighthsharppix,  NULL);
  XtVaSetValues(note[3], XmNlabelType, XmPIXMAP, XmNlabelPixmap, quarterpix,      NULL);
  XtVaSetValues(note[4], XmNlabelType, XmPIXMAP, XmNlabelPixmap, quartersharppix, NULL);
  XtVaSetValues(note[5], XmNlabelType, XmPIXMAP, XmNlabelPixmap, halfpix,         NULL);
  XtVaSetValues(note[6], XmNlabelType, XmPIXMAP, XmNlabelPixmap, halfsharppix,    NULL);
  XtVaSetValues(note[7], XmNlabelType, XmPIXMAP, XmNlabelPixmap, restpix,         NULL);
}


/*--------------------------------------------------------------------*
 |                              Main                                  |
 *--------------------------------------------------------------------*/
#ifndef _NO_PROTO
main(int argc, char **argv)
#else
main(argc, argv)
     int argc;
     char **argv;
#endif
{
  Widget        shell, panedWin;
  Widget        keyboard, notebook;
  int           fn;
  Pixel         fg, bg;
  XGCValues     values;

  
  shell     = XtVaAppInitialize(&context, APP_CLASS, NULL, 0, &argc, argv, fallback, NULL);

  panedWin  = XtVaCreateManagedWidget("panedWin",  xmPanedWindowWidgetClass, shell, NULL);
  keyboard  = CreateKeyboard(panedWin);
  score     = CreateScore   (panedWin);
  notebook  = CreateNotebook(panedWin);

  SetAppIcon(shell);
  XtRealizeWidget(shell);

  /* get the note GC */
  XtVaGetValues(score, XmNforeground, &fg, XmNbackground, &bg, NULL);
  values.foreground = fg;
  values.background = bg;
  noteGC = XtGetGC(score, GCForeground | GCBackground, &values);

  SetActiveNote(score, QUARTER);

  /* save the old bell values so that they can be restored. */
  GetBell(XtDisplay(shell));

  XtAppMainLoop(context);
}
