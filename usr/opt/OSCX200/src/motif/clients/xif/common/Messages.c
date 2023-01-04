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
static char *rcsid = "@(#)$RCSfile: Messages.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/08/30 16:42:20 $";
#endif

/*******************************************************************************
 *
 * FILE: Messages.c
 *
 ******************************************************************************/

#include <stdio.h>                      /* For printf and so on. */
#include <Xm/MessageB.h>
#include <Mrm/MrmAppl.h>                /* Motif Toolkit and MRM */
#include <X11/X.h>

typedef struct {
   Widget widget;
   Widget parent;
} MsgDlgStruct;

/******************************************************************************
 * Message Dialog Defines
 ******************************************************************************/
#define NOT_FOUND -1

#define ERROR_DIALOG          1
#define WORKING_DIALOG        2
#define QUESTION_DIALOG       3

#define ERROR_DIALOG_NAME     "Error"
#define WORKING_DIALOG_NAME   "Working"
#define QUESTION_DIALOG_NAME  "Question"

#define UNKNOWN_MSG           "Can't find message strings: check uid file."

/******************************************************************************
 * Static Global data
 ******************************************************************************/

/*
 * Create a hook to the structure holding Message Dialogs and their parents.
 */
static MsgDlgStruct *error_ed = NULL;
static int           num_error_eds=0;

static MsgDlgStruct *working_wd = NULL;
static int           num_working_wds=0;

static MsgDlgStruct *question_qd = NULL;
static int           num_question_qds=0;

static MrmHierarchy    s_MrmHierarchy;

/*
 ==============================================================================
 = C A L L B A C K   F U N C T I O  N S
 ==============================================================================
*/

/******************************************************************************
 * DlgCloseCB()
 ******************************************************************************/
void DlgCloseCB(w, dlg, reason)
Widget	       w;
Widget         dlg;
unsigned long *reason;
{
XtUnmanageChild(dlg);
}

/*
 ==============================================================================
 = S T A T I C   R O U T I N E S   T O   S U P P O R T   M E S S A G E S
 ============================================================================== 
*/


/******************************************************************************
 * DlgRemoveAllCallbacks()
 ******************************************************************************/
static void DlgRemoveAllCallbacks(w)
Widget w;

{
XtRemoveAllCallbacks(w,XmNokCallback);
XtRemoveAllCallbacks(w,XmNcancelCallback);
/* Eventually remove the Help Callback if necessary */
/* XtRemoveAllCallbacks(w,XmNhelpCallback); */
}
/******************************************************************************
 * DlgAddAllCallbacks()
 ******************************************************************************/
static void DlgAddAllCallbacks(w,okCB,okData,cancelCB,cancelData,helpCB,
                               helpData)
Widget    w;
void      (*okCB)();
XtPointer okData;
void      (*cancelCB)();
XtPointer cancelData;
void      (*helpCB)();
XtPointer helpData;

{
Widget helpPB;

/* Add the standard "Unmanage" CB and the client CB if necessary */

XtAddCallback(w,XmNokCallback,(XtCallbackProc)DlgCloseCB,(caddr_t)w);
if (okCB)
   XtAddCallback(w,XmNokCallback,okCB,okData);

XtAddCallback(w,XmNcancelCallback,(XtCallbackProc)DlgCloseCB,(caddr_t)w);
if (cancelCB)
   XtAddCallback(w,XmNcancelCallback,cancelCB,cancelData);

/*
 * Unmanage the help button if no help callback exists, manage it if 
 * it does. This is wasteful but I don't trust the XtIsManaged routine.
 */
if (helpCB)
   {
   XtAddCallback(w,XmNhelpCallback,helpCB,helpData);
   helpPB = XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON);
   XtManageChild(helpPB);
   }
else
   {
   helpPB = XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON);
   XtUnmanageChild(helpPB);
   }
}

/******************************************************************************
 * CreateDlg()
 ******************************************************************************/
static void CreateDlg(parent,dlg,num_dlgs,dlg_type)
Widget         parent;
MsgDlgStruct **dlg;
int            num_dlgs;
int            dlg_type;

{
int      n;
Arg      args[10];
XmString xmstring;

/* Realloc Space for data structure */
*dlg=(MsgDlgStruct *)XtRealloc((char *)*dlg,sizeof(MsgDlgStruct)*(num_dlgs+1));

/*
 * Don't supply any system close routines as we want to unmanage and not
 * destroy the widget in order to use it again if necessary.
 */
n = 0;
XtSetArg(args[n], XmNautoUnmanage, False);  n++;

switch (dlg_type)
{
case ERROR_DIALOG:
        XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); n++;

        xmstring = XmStringCreate(ERROR_DIALOG_NAME,XmSTRING_DEFAULT_CHARSET);
        if (! xmstring) MemoryError();
        XtSetArg(args[n], XmNdialogTitle, xmstring);  n++;

        (*dlg)[num_dlgs].widget =
           XmCreateErrorDialog(parent,ERROR_DIALOG_NAME,args,n);
        (*dlg)[num_dlgs].parent = parent;

        XmStringFree(xmstring);
     break;
case WORKING_DIALOG:
        XtSetArg(args[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;

        xmstring = XmStringCreate(WORKING_DIALOG_NAME,XmSTRING_DEFAULT_CHARSET);
        if (! xmstring) MemoryError();
        XtSetArg(args[n], XmNdialogTitle, xmstring);  n++;

        (*dlg)[num_dlgs].widget =
           XmCreateWorkingDialog(parent,WORKING_DIALOG_NAME,args,n);
        (*dlg)[num_dlgs].parent = parent;

        XmStringFree(xmstring);
     break;
case QUESTION_DIALOG:
        XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); n++;

        xmstring=XmStringCreate(QUESTION_DIALOG_NAME,XmSTRING_DEFAULT_CHARSET);
        if (! xmstring) MemoryError();
        XtSetArg(args[n], XmNdialogTitle, xmstring);  n++;

        (*dlg)[num_dlgs].widget =
           XmCreateQuestionDialog(parent,QUESTION_DIALOG_NAME,args,n);
        (*dlg)[num_dlgs].parent = parent;

        XmStringFree(xmstring);
     break;
default: printf("rcvd unknown dialog type %d\n",dlg_type);
}

}
/******************************************************************************
 * DlgParentToIndex()
 ******************************************************************************/
static int DlgParentToIndex(parent,dlg,num_dlgs)
Widget        parent;
MsgDlgStruct *dlg;
int           num_dlgs;

{
int i;

/* Search existing dialogs for one that matches parent */

for (i=0; i<num_dlgs; i++)
    {
    if (dlg[i].parent == parent)
       {
       return(i);
       }
    }

return(NOT_FOUND);
}
/******************************************************************************
 * MsgDlgSetMessage()
 ******************************************************************************/
static void MsgDlgSetMessage(w,msgname)
Widget  w;
char   *msgname;

{
int      n;
Arg      args[10];
XmString xmstring;
MrmCode  return_type;


if(MrmFetchLiteral(s_MrmHierarchy,msgname,XtDisplay(w),(caddr_t *)&xmstring,
                   &return_type) != MrmSUCCESS)
  {
  xmstring = XmStringCreate(UNKNOWN_MSG,XmSTRING_DEFAULT_CHARSET);
  if (! xmstring) MemoryError();
  }
  
n = 0;
XtSetArg(args[n], XmNmessageString, xmstring);  n++;
XtSetValues(w, args, n);
}
/******************************************************************************
 * MsgBoxFlush()
 ******************************************************************************/
static void MsgBoxFlush(w)
Widget w;

{
XtAppContext app_context;

app_context = XtWidgetToApplicationContext(w);

XSync(XtDisplay(w),False);

while (1)
   {
   XEvent event;

   XtAppNextEvent(app_context,&event);
   XtDispatchEvent(&event);
   if ((event.type == Expose) && (event.xexpose.window == XtWindow(w)))
      break;
   }

XSync(XtDisplay(w),False);
}

/*
 ==============================================================================
 = P U B L I C   R O U T I N E S   T O   S U P P O R T   M E S S A G E S
 ============================================================================== 
*/

/******************************************************************************
 * MessagesSetMrmHierarchy ()
 ******************************************************************************/
void MessagesSetMrmHierarchy(hierarchy)
MrmHierarchy hierarchy;

{
s_MrmHierarchy = hierarchy;
}
/******************************************************************************
 * UnPostWorkingDialog ()
 ******************************************************************************/
void UnPostWorkingDialog(parent)
Widget   parent;

{
int i;

if ((i=DlgParentToIndex(parent,working_wd,num_working_wds)) == NOT_FOUND)
   {
   /* Error: Can't find Working Dialog to pop down */
   }
else
   {
   XtUnmanageChild(working_wd[i].widget);
   }
}
/******************************************************************************
 * PostWorkingDialog ()
 ******************************************************************************/
void PostWorkingDialog(msgname,parent,okCB,okData,cancelCB,cancelData,
                       helpCB, helpData)
char      *msgname;
Widget     parent;
void       (*okCB)();
XtPointer  okData;
void       (*cancelCB)();
XtPointer  cancelData;
void       (*helpCB)();
XtPointer  helpData;

{
int i;

if ((i=DlgParentToIndex(parent,working_wd,num_working_wds)) == NOT_FOUND)
   {
   i=num_working_wds;
   CreateDlg(parent,&working_wd,num_working_wds,WORKING_DIALOG);

   DlgAddAllCallbacks(working_wd[i].widget,okCB,okData,cancelCB,cancelData,
                      helpCB,helpData);

   /* Set the message string */
   MsgDlgSetMessage(working_wd[i].widget,msgname);

   XtManageChild(working_wd[i].widget);

   /* Increment the number of working dialogs counter */
   num_working_wds++;
   }
else
   {
   if (! XtIsManaged(working_wd[i].widget))
      {
      DlgRemoveAllCallbacks(working_wd[i].widget);

      DlgAddAllCallbacks(working_wd[i].widget,okCB,okData,cancelCB,cancelData,
                         helpCB,helpData);

      /* Set the message string */
      MsgDlgSetMessage(working_wd[i].widget,msgname);

      XtManageChild(working_wd[i].widget);
      }
   else
      {
      /* Look at code for VUIT_Manage to circulate the window up */
      }
   }

/* Must open a temporary Event Processing loop to instantiate Message Box */
MsgBoxFlush(working_wd[i].widget);
}
/******************************************************************************
 * UnPostErrorDialog ()
 ******************************************************************************/
void UnPostErrorDialog(parent)
Widget   parent;

{
int i;

if ((i=DlgParentToIndex(parent,error_ed,num_error_eds)) == NOT_FOUND)
   {
   /* Error: Can't find Error Dialog to pop down */
   }
else
   {
   XtUnmanageChild(error_ed[i].widget);
   }
}
/******************************************************************************
 * PostErrorDialog ()
 ******************************************************************************/
void PostErrorDialog(msgname,parent,okCB,okData,cancelCB,cancelData,
                     helpCB,helpData)
char      *msgname;
Widget     parent;
void       (*okCB)();
XtPointer  okData;
void       (*cancelCB)();
XtPointer  cancelData;
void       (*helpCB)();
XtPointer  helpData;

{
int i;

if ((i=DlgParentToIndex(parent,error_ed,num_error_eds)) == NOT_FOUND)
   {
   i=num_error_eds;
   CreateDlg(parent,&error_ed,num_error_eds,ERROR_DIALOG);
   
   DlgAddAllCallbacks(error_ed[i].widget,okCB,okData,cancelCB,cancelData,
                      helpCB,helpData);

   /* Set the message string */
   MsgDlgSetMessage(error_ed[i].widget,msgname);

   XtManageChild(error_ed[i].widget);

   /* Increment the number of error dialogs counter */
   num_error_eds++;
   }
else
   {
   if (! XtIsManaged(error_ed[i].widget))
      {
      DlgRemoveAllCallbacks(error_ed[i].widget);

      DlgAddAllCallbacks(error_ed[i].widget,okCB,okData,cancelCB,cancelData,
                         helpCB,helpData);

      /* Set the message string */
      MsgDlgSetMessage(error_ed[i].widget,msgname);

      XtManageChild(error_ed[i].widget);
      }
   else
      {
      /* Look at code for VUIT_Manage to circulate the window up */
      }
   }

/* Must open a temporary Event Processing loop to instantiate Message Box */
MsgBoxFlush(error_ed[i].widget);
}
/******************************************************************************
 * UnPostQuestionDialog ()
 ******************************************************************************/
void UnPostQuestionDialog(parent)
Widget   parent;

{
int i;

if ((i=DlgParentToIndex(parent,question_qd,num_question_qds)) == NOT_FOUND)
   {
   /* Error: Can't find Error Dialog to pop down */
   }
else
   {
   XtUnmanageChild(question_qd[i].widget);
   }
}
/******************************************************************************
 * PostQuestionDialog ()
 ******************************************************************************/
void PostQuestionDialog(msgname,parent,okCB,okData,cancelCB,cancelData,
                        helpCB,helpData)
char     *msgname;
Widget    parent;
void      (*okCB)();
XtPointer okData;
void      (*cancelCB)();
XtPointer cancelData;
void      (*helpCB)();
XtPointer helpData;

{
int i;

if ((i=DlgParentToIndex(parent,question_qd,num_question_qds)) == NOT_FOUND)
   {
   i=num_question_qds;
   CreateDlg(parent,&question_qd,num_question_qds,QUESTION_DIALOG);
   
   DlgAddAllCallbacks(question_qd[i].widget,okCB,okData,cancelCB,cancelData,
                      helpCB,helpData);

   /* Set the message string */
   MsgDlgSetMessage(question_qd[i].widget,msgname);

   XtManageChild(question_qd[i].widget);

   /* Increment the number of question dialogs counter */
   num_question_qds++;
   }
else
   {
   if (! XtIsManaged(question_qd[i].widget))
      {
      DlgRemoveAllCallbacks(question_qd[i].widget);

      DlgAddAllCallbacks(question_qd[i].widget,okCB,okData,cancelCB,cancelData,
                         helpCB,helpData);

      /* Set the message string */
      MsgDlgSetMessage(question_qd[i].widget,msgname);

      XtManageChild(question_qd[i].widget);
      }
   else
      {
      /* Look at code for VUIT_Manage to circulate the window up */
      }
   }

/* Must open a temporary Event Processing loop to instantiate Message Box */
MsgBoxFlush(question_qd[i].widget);
}
