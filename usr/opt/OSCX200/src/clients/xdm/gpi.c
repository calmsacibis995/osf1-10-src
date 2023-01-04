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
static char *BuildSystemHeader= "@(#)$RCSfile: gpi.c,v $ $Revision: 1.1.6.8 $ (DEC) $Date: 1993/08/26 17:24:17 $";
#endif		/* BuildSystemHeader */
/*****************************************************************************
 * gpi.c - A  generic prompting interface                                    *
 *****************************************************************************/

/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

#define UID_FILE_NAME     "gpiforms.uid"
#define HOSTPLACEHOLDER   "%hostname%"
#define GPI_BUFSIZ            256

/*****************************************************************************
 * Macros                                                                    *
 *****************************************************************************/

#include <stdio.h>
#ifdef _DEBUG
#include <stdio.h>
#define DEBUG(x)     {printf(x);fflush(stdout);}
#define SETJMP(x)    (printf("setjmp on %lu\n",(unsigned long)x),setjmp(x))
#define LONGJMP(x,y) (printf("longjmp to %lu\n",(unsigned long)x),longjmp(x,y))
#else
#define DEBUG(x)
#define SETJMP(x)    setjmp(x)
#define LONGJMP(x,y) longjmp(x,y)
#endif

#define SetInfo(cbi, t, w, l) \
    (cbi.widgetTag = t, cbi.pLoginWidgets = w, cbi.pLoginData = l)

#define SetListInfo(cbi, t, w, l) \
    (cbi.widgetTag = t, cbi.pListWidgets = w, cbi.pListData = l)


#define SetFormInfo(cbi, t, w, l) \
    (cbi.widgetTag = t, cbi.pFormWidgets = w, cbi.pFormData = l)

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include <strings.h>
#include <setjmp.h>
#include <StringDefs.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Mrm/MrmAppl.h>
#include <Mrm/MrmPublic.h>
#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/TextP.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include "gpi.h"

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifndef MAX
#define MAX(x,y)	((x) > (y) ? (x) : (y))
#endif

/*****************************************************************************
 * Types                                                                     *
 *****************************************************************************/

typedef enum _GpiIReturnValue {
    GpiIAbort,                	/* Abort further processing */
    GpiIContinue,        	/* Continue processing */
    GpiINeither                	/* Neither of the above */
} GpiIReturnValue;

typedef struct {
    GpiIReturnValue returnValue;
    Widget	    answerWidget;
    char*	    answerString;
    int        	    maxAnswerLength;
    char**	    arrayOfStrings;
    int        	    numArrayStrings;
    Boolean	    mayDisplayTimeout;
    Boolean	    mustMatch;
    unsigned long   timeout;
    XtIntervalId    timerId;
    jmp_buf	    timerJmpBuf;
} GpiCallbackInfo, * PGpiCallbackInfo;

enum LoginWidgetKind { Top, Login, Name, Password, NamePrompt,
        	       PasswordPrompt, Greeting, Ok, Abort, LastOne };

typedef struct {
    Widget     widget;
    Position   posX, posY, defX, defY;
    Dimension  dW, dH, defW, defH;
} LoginWidgets, * PLoginWidgets;

typedef struct {
    GpiIReturnValue returnValue;
    char*	    pName;
    char*	    pPassword;
    int             pMaxLength;
    int             pCurLength;
} LoginData, * PLoginData;

typedef struct {
    enum LoginWidgetKind widgetTag;
    PLoginWidgets        pLoginWidgets;
    PLoginData	         pLoginData;
} LoginCallbackInfo, * PLoginCallbackInfo;

/*****************************************************************************/
enum ListWidgetKind {ListTop, ListForm, ListTitle, ListOk, ListAbort, ListList, ListLast};

typedef struct {
    Widget     widget;
    Position   posX, posY, defX, defY;
    Dimension  dW, dH, defW, defH;
} ListWidgets, * PListWidgets;

typedef struct {
    GpiIReturnValue returnValue;
	int         num_answers;
	char**        	selections;
    char**	    answer;
	int*         	answer_maxlen;
	Widget        	list_widget;
} ListData, * PListData;

typedef struct {
    enum ListWidgetKind widgetTag;
    PListWidgets        pListWidgets;
    PListData	        pListData;
    Boolean                	    mayDisplayTimeout;
    unsigned long         	timeout;
    XtIntervalId        	timerId;
    jmp_buf                        	timerJmpBuf;
} ListCallbackInfo, * PListCallbackInfo;

/*****************************************************************************/
enum FormWidgetKind {FormTop, FormForm, FormClientTitle, FormTitle, FormPrompt,
    FormOk, FormAbort, FormLast};

typedef struct {
    Widget     widget;
    Position   posX, posY, defX, defY;
    Dimension  dW, dH, defW, defH;
} FormWidgets, * PFormWidgets;

typedef struct {
    GpiIReturnValue returnValue;
    int num_answers;
    unsigned char** selections;
    unsigned char** answer;
    int* answer_maxlen;
    int* answer_curlen;
    Boolean* answer_visible;
    Widget* answer_widgets;
} FormData, *PFormData;

typedef struct {
    enum FormWidgetKind widgetTag;
    PFormWidgets        pFormWidgets;
    PFormData	        pFormData;
    Boolean                	    mayDisplayTimeout;
    unsigned long        	timeout;
    XtIntervalId        	timerId;
    jmp_buf                        	timerJmpBuf;
} FormCallbackInfo, * PFormCallbackInfo;

typedef struct {
    int which_widget;
    PFormCallbackInfo pToInfo;
} FormNCallbackInfo, *PFormNCallbackInfo;


/*****************************************************************************
 * Resources                                                        	     *
 *****************************************************************************/

#define	Offset(field)	XtOffsetOf(MessageOptionRec, field)

typedef struct {
    char* pTitle;
    char* pOkLabel;
} MessageOptionRec;

static XtResource MessageResources[] = {
    {
      XmNdialogTitle, XmCDialogTitle, XtRString, sizeof (char *),
      Offset(pTitle), XtRString, "GpiDisplayMessage"
    },
    {
      XmNokLabelString, XmCOkLabelString, XtRString, sizeof (char *),
      Offset(pOkLabel), XtRString, "Acknowledge"
    },
};

#undef Offset

#define	Offset(field)	XtOffsetOf(FormOptionRec, field)

typedef struct {
    char* pTitle;
    char* pOkLabel;
    char* pCancelLabel;
} FormOptionRec;

static XtResource FormResources[] = {
    {
      XmNdialogTitle, XmCDialogTitle, XtRString, sizeof (char *),
      Offset(pTitle), XtRString, "GpiForm"
    },
    {
      XmNokLabelString, XmCOkLabelString, XtRString, sizeof (char *),
      Offset(pOkLabel), XtRString, "Ok"
    },
    {
      XmNcancelLabelString, XmCCancelLabelString, XtRString, sizeof (char *),
      Offset(pCancelLabel), XtRString, "Cancel"
    },
};

#undef Offset

#define	Offset(field)	XtOffsetOf(ListOptionRec, field)

typedef struct {
    char* pTitle;
    char* pOkLabel;
    char* pCancelLabel;
} ListOptionRec;

static XtResource ListResources[] = {
    {
      XmNdialogTitle, XmCDialogTitle, XtRString, sizeof (char *),
      Offset(pTitle), XtRString, "GpiQueryUserWithList"
    },
    {
      XmNokLabelString, XmCOkLabelString, XtRString, sizeof (char *),
      Offset(pOkLabel), XtRString, "Continue"
    },
    {
      XmNcancelLabelString, XmCCancelLabelString, XtRString, sizeof (char *),
      Offset(pCancelLabel), XtRString, "Cancel"
    },
};

#undef Offset

#define	Offset(field)	XtOffsetOf(FailOptionRec, field)

typedef struct {
    char*        	pTitle;
    char*        	pOkLabel;
    Pixel        	cFailColor;
    XFontStruct*	pFailFont;
    char*        	pFail;
    int                	iFailTimeout;
} FailOptionRec;

static XtResource FailResources[] = {
    {
      XmNdialogTitle, XmCDialogTitle, XtRString, sizeof (char *),
      Offset(pTitle), XtRString, "LoginFail"
    },
    {
      XmNokLabelString, XmCOkLabelString, XtRString, sizeof (char *),
      Offset(pOkLabel), XtRString, "Acknowledge"
    },
    {
      XgpiNfailColor, XtCForeground, XtRPixel, sizeof (Pixel),
      Offset(cFailColor), XtRString, "Black"
    },
    {
      XgpiNfailFont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
      Offset(pFailFont), XtRString,
      "*-new century schoolbook-bold-r-normal-*-180-*"
    },
    {
      XgpiNfail, XgpiCFail, XtRString, sizeof (char *),
      Offset(pFail), XtRString, "Login failed, please try again."
    },
    {
      XgpiNfailTimeout, XgpiCFailTimeout, XtRInt, sizeof (int),
      Offset(iFailTimeout), XtRString, "10"
    },
};

#undef Offset

 /* need to add special handling for cntl u, cntl c, and delete. */
static char FormResultTranslationTable[] = {  
   ":Ctrl<Key>a:           self-insert()\n\
    :Ctrl<Key>b:           self-insert()\n\
    :Ctrl<Key>c:           self-insert()\n\
    :Ctrl<Key>d:           self-insert()\n\
    :Ctrl<Key>e:           self-insert()\n\
    :Ctrl<Key>f:           self-insert()\n\
    :Ctrl<Key>g:           self-insert()\n\
    :Ctrl<Key>h:           self-insert()\n\
    :Ctrl<Key>i:           self-insert()\n\
    :Ctrl<Key>j:           self-insert()\n\
    :Ctrl<Key>k:           self-insert()\n\
    :Ctrl<Key>l:           self-insert()\n\
    :Ctrl<Key>m:           self-insert()\n\
    :Ctrl<Key>n:           self-insert()\n\
    :Ctrl<Key>o:           self-insert()\n\
    :Ctrl<Key>p:           self-insert()\n\
    :Ctrl<Key>q:           self-insert()\n\
    :Ctrl<Key>r:           self-insert()\n\
    :Ctrl<Key>s:           self-insert()\n\
    :Ctrl<Key>t:           self-insert()\n\
    :Ctrl<Key>v:           self-insert()\n\
    :Ctrl<Key>w:           self-insert()\n\
    :Ctrl<Key>x:           self-insert()\n\
    :Ctrl<Key>y:           self-insert()\n\
    :Ctrl<Key>z:           self-insert()\n\
    :Ctrl<Key>u:	   beginning-of-line() kill-to-end-of-line()\n\
    :<Key>Return:          set-session-argument() activate() next-tab-group()\n\
    :<Key>F1:		   set-session-argument(failsafe) activate()\n\
    :<Key>F2:		   set-session-argument(failsafe) activate()\n"
};


/*****************************************************************************
 * Prototypes                                                        	     *
 *****************************************************************************/
XtActionProc GpiSetSessionArgument( Widget		widget,
					XEvent*		event,
					String*		params,
					Cardinal*	num_params );

static void DisplayMessage(Boolean	  mayDisplayTimeout,
                	    unsigned long timeout,
                	    char* 	  warningString);
static void GeneralAbort(Widget    widget,
                        	    XtPointer clientData,
                        	    XtPointer callData);
static void GeneralContinue(Widget	 widget,
                        	       XtPointer clientData,
                        	       XtPointer callData);
static void PromptContinue(Widget	widget,
                        	      XtPointer clientData,
                        	      XtPointer callData);
static void PromptNoMatch(Widget    widget,
                        	     XtPointer clientData,
                        	     XtPointer callData);
static void TimedOut(XtPointer clientData,XtIntervalId* id);
static void ListTimedOut(XtPointer clientData,XtIntervalId* id);
static void FormTimedOut(XtPointer clientData,XtIntervalId* id);
static XtActionProc GContinue(Widget	 widget,
                	       XEvent*	 event,
                	       String*	 params,
                	       Cardinal* num_params);
static XtActionProc GAbort(Widget    widget,
                	    XEvent*   event,
                	    String*   params,
                	    Cardinal* num_params);
static void LoginAbort(Widget	widget,
                        	  XtPointer	client_data,
                        	  XtPointer	call_data);

static void LoginContinue(Widget	widget,
                        	     XtPointer	client_data,
                        	     XtPointer	call_data);

static void LoginCCallback(Widget    widget,
                        	      XtPointer client_data,
                        	      XtPointer call_data);

static void LoginModifyVerify(Widget    widget,
                        	      XtPointer client_data,
                        	      XmTextVerifyPtr call_data);

static XtActionProc LAbort(Widget	widget,
                	    XEvent*	event,
                	    String*	params,
                	    Cardinal*	num_params);

static XtActionProc LDelete(Widget	widget,
                	    XEvent*	event,
                	    String*	params,
                	    Cardinal*	num_params);

static Boolean NullGetSelection(
	XmTextSource source,
	XmTextPosition *start,
	XmTextPosition *end);

static void NullSetSelection(
	XmTextSource source,
	XmTextPosition start,
	XmTextPosition end);

/*****************************************************************************
 * List Routines *
 *****************************************************************************/
static void ListCBAbort(Widget	widget,
                        	  XtPointer	client_data,
                        	  XtPointer	call_data);

static void ListCBContinue(Widget	widget,
                        	     XtPointer	client_data,
                        	     XtPointer	call_data);

static void ListCCallback(Widget    widget,
                        	      XtPointer client_data,
                        	      XtPointer call_data);

/*****************************************************************************
 * Form Routines *
 *****************************************************************************/
static void FormCBAbort(Widget	widget,
                        	  XtPointer	client_data,
                        	  XtPointer	call_data);

static void FormCBContinue(Widget	widget,
                        	     XtPointer	client_data,
                        	     XtPointer	call_data);

static void FormCCallback(Widget    widget,
                        	      XtPointer client_data,
                        	      XtPointer call_data);


/*****************************************************************************
 * Global variables                                                	     *
 *****************************************************************************/

static Boolean GpiInitedXt           = False;
Widget  GpiShellWidget = (Widget) NULL;
static char    GpiTranslationTable[] = {
    ":Ctrl<Key>c:           GpiAbort()\n\
    :<Key>Return: Return()  GpiContinue()"
};

 /* need to add special handling for cntl u and delete. */
static char LoginResultTranslationTable[] = {  
   ":Ctrl<Key>a:           self-insert()\n\
    :Ctrl<Key>b:           self-insert()\n\
    :Ctrl<Key>d:           self-insert()\n\
    :Ctrl<Key>e:           self-insert()\n\
    :Ctrl<Key>f:           self-insert()\n\
    :Ctrl<Key>g:           self-insert()\n\
    :Ctrl<Key>h:           self-insert()\n\
    :Ctrl<Key>i:           self-insert()\n\
    :Ctrl<Key>j:           self-insert()\n\
    :Ctrl<Key>k:           self-insert()\n\
    :Ctrl<Key>l:           self-insert()\n\
    :Ctrl<Key>m:           self-insert()\n\
    :Ctrl<Key>n:           self-insert()\n\
    :Ctrl<Key>o:           self-insert()\n\
    :Ctrl<Key>p:           self-insert()\n\
    :Ctrl<Key>q:           self-insert()\n\
    :Ctrl<Key>r:           self-insert()\n\
    :Ctrl<Key>s:           self-insert()\n\
    :Ctrl<Key>t:           self-insert()\n\
    :Ctrl<Key>v:           self-insert()\n\
    :Ctrl<Key>w:           self-insert()\n\
    :Ctrl<Key>x:           self-insert()\n\
    :Ctrl<Key>y:           self-insert()\n\
    :Ctrl<Key>z:           self-insert()\n\
    :Ctrl<Key>u:	   beginning-of-line() kill-to-end-of-line()"
};

static XErrorHandler orig_handler;
static Boolean wm_present;
static Boolean abort_status;

static int local_xerror(dpy, event)
    Display *dpy;
    XErrorEvent *event;
{

    if ((event->request_code == X_ChangeWindowAttributes)
	&& (event->error_code == BadAccess))
	{
	wm_present = True;
	return;
	}
    else
	{
	orig_handler(dpy, event);
	}
}
/*****************************************************************************
 * Public routines                                                	     *
 *****************************************************************************/

void GpiDisplayMessage(Widget	      shellWidget,
                	unsigned long timeout,
                	Boolean	      displayTimeout,
                	unsigned char* 	      warningString)
{
    if (GpiEstablishShell(shellWidget) == GpiAbort)
        return;
    DisplayMessage(displayTimeout, timeout, (char *)warningString);
}
  

GpiReturnValue GpiList(
  Widget shellWidget,
  unsigned long timeout,
  unsigned char* promptString,
  unsigned char* top_title,
  unsigned char** arrayOfStrings,
  int numArrayStrings,
  unsigned char** answerString,
  int maxAnswerLength[],
  Boolean multiple)
{
    Cardinal        status;
    Arg                	pArgs[15];
    int                	i,nArgs;
    XmString	s1, s2, s3;
    ListOptionRec	optionRec;
    GpiCallbackInfo	gpiCallbackInfo;
    XEvent        	event;
	ListCallbackInfo info[ListLast];
    PListCallbackInfo	 pInfo = info;
	enum ListWidgetKind iWidget;
    ListWidgets	 listWidgets[ListLast];
    char*        	 uidFile[1];
    MrmHierarchy hierarchyId;
    MrmType        	 widgetClass;
    ListData	 listData;
    
    DEBUG("Entering GpiList\n");
    if(GpiEstablishShell(shellWidget) == GpiAbort)
        return(GpiAbort);

	listData.num_answers = numArrayStrings;
	listData.selections = (char **)arrayOfStrings;
	listData.answer = (char **)answerString;
	listData.answer_maxlen = maxAnswerLength;

	listWidgets[ListTop].widget = XtCreatePopupShell("GpiList",
                	topLevelShellWidgetClass, GpiShellWidget,NULL,0);

	uidFile[0] = UID_FILE_NAME;
	MrmOpenHierarchy((MrmCount)(1),uidFile,0,&hierarchyId);
    SetListInfo(info[ListTop],                	ListTop,	   listWidgets, &listData);
    SetListInfo(info[ListForm],        	ListForm,	   listWidgets, &listData);
    SetListInfo(info[ListTitle],        	ListTitle,	   listWidgets, &listData);
    SetListInfo(info[ListOk],                	ListOk,        	   listWidgets, &listData);
    SetListInfo(info[ListAbort],        	ListAbort,	   listWidgets, &listData);
    SetListInfo(info[ListList],        	ListList,	   listWidgets, &listData);

    nArgs = 0;
    XtSetArg(pArgs[nArgs], "ListCCallback",	ListCCallback	); nArgs++; 
    XtSetArg(pArgs[nArgs], "ListCBAbort",        	ListCBAbort	); nArgs++; 
    XtSetArg(pArgs[nArgs], "ListCBContinue",	ListCBContinue	); nArgs++; 
    XtSetArg(pArgs[nArgs], "pInfo",                	pInfo                	); nArgs++; 
    XtSetArg(pArgs[nArgs], "title",                	&info[ListTitle]); nArgs++; 
    XtSetArg(pArgs[nArgs], "listList",                	&info[ListList]); nArgs++; 
    XtSetArg(pArgs[nArgs], "Ok",                        	&info[ListOk]	); nArgs++; 
    XtSetArg(pArgs[nArgs], "Abort",                	&info[ListAbort]); nArgs++; 
    status = MrmRegisterNames(pArgs, nArgs);
	if(status != MrmSUCCESS)
        	fprintf(stderr, "Register Names failed\n");

	status = MrmFetchWidget(hierarchyId,"GpiList",listWidgets[ListTop].widget,
        	&listWidgets[ListForm].widget,&widgetClass);
	if(status != MrmSUCCESS)
        	fprintf(stderr, "Fetch Widget failed\n");
	MrmCloseHierarchy(hierarchyId);
    XtManageChild(listWidgets[ListForm].widget);

	for(iWidget = ListTop;iWidget < ListLast;iWidget++)
	{
        	nArgs = 0;
        	XtSetArg(pArgs[nArgs],XmNx,&listWidgets[iWidget].defX);        	nArgs++;
        	XtSetArg(pArgs[nArgs],XmNy,&listWidgets[iWidget].defY);        	nArgs++;
        	XtSetArg(pArgs[nArgs],XmNwidth,&listWidgets[iWidget].defW);	nArgs++;
        	XtSetArg(pArgs[nArgs],XmNheight,&listWidgets[iWidget].defH);nArgs++;
        	XtGetValues(listWidgets[iWidget].widget,pArgs,nArgs);
	}

	listData.list_widget = listWidgets[ListList].widget;
	
    XtGetApplicationResources(listWidgets[ListForm].widget, 
                                	(XtPointer) &optionRec,
                                	ListResources, XtNumber(ListResources), (Arg*) NULL, 0);

    s2 = XmStringCreateLtoR(promptString, XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNlabelString,          s2); nArgs++;
	XtSetValues(listWidgets[ListTitle].widget,pArgs,nArgs);
	XmStringFree(s2);
	
	nArgs = 0;
	XtSetArg(pArgs[nArgs], XmNvisibleItemCount,	numArrayStrings); nArgs++;
	if(multiple)
	{
        	XtSetArg(pArgs[nArgs], XmNselectionPolicy,	XmEXTENDED_SELECT); nArgs++;
	}
	else
	{
        	XtSetArg(pArgs[nArgs], XmNselectionPolicy,	XmSINGLE_SELECT); nArgs++;
	}
	XtSetValues(listWidgets[ListList].widget,pArgs,nArgs);

	for(i=0;i<numArrayStrings;i++)
	{
        	s1 = XmStringCreateLtoR(arrayOfStrings[i], XmSTRING_DEFAULT_CHARSET);
        	XmListAddItem(listWidgets[ListList].widget,s1,0);
	}

    s1 = XmStringCreateLtoR("OK", XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNlabelString,          s1       ); nArgs++;
	XtSetValues(listWidgets[ListOk].widget,pArgs,nArgs);
	XmStringFree(s1);

    s1 = XmStringCreateLtoR("Abort", XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNlabelString,          s1       ); nArgs++;
	XtSetValues(listWidgets[ListAbort].widget,pArgs,nArgs);
	XmStringFree(s1);

	for(iWidget = ListTop;iWidget < ListLast;iWidget++)
	{
        	nArgs = 0;
        	XtSetArg(pArgs[nArgs],XmNx,&listWidgets[iWidget].posX);        	nArgs++;
        	XtSetArg(pArgs[nArgs],XmNy,&listWidgets[iWidget].posY);        	nArgs++;
        	XtSetArg(pArgs[nArgs],XmNwidth,&listWidgets[iWidget].dW);	nArgs++;
        	XtSetArg(pArgs[nArgs],XmNheight,&listWidgets[iWidget].dH);nArgs++;
        	XtGetValues(listWidgets[iWidget].widget,pArgs,nArgs);
	}
 
    pInfo->mayDisplayTimeout = True;
	if ((pInfo->timeout = timeout) != 0)
   	{
        	pInfo->timerId = 
                	XtAppAddTimeOut(XtWidgetToApplicationContext(GpiShellWidget),
                	     timeout, ListTimedOut, (XtPointer) pInfo);
    }
	XtPopup(listWidgets[ListTop].widget,XtGrabNone);
	listData.returnValue = GpiINeither;
    if(! SETJMP(pInfo->timerJmpBuf))
    {
        	while(listData.returnValue == GpiINeither)
        	{
                	XtAppNextEvent(XtWidgetToApplicationContext(GpiShellWidget),&event);
                	XtDispatchEvent(&event);
        	}
        	if(timeout && pInfo->timerId)
        	    XtRemoveTimeOut(pInfo->timerId);
	}
    XtDestroyWidget(listWidgets[ListTop].widget);
    return(listData.returnValue);
}

#define maxidlen	32

GpiReturnValue GpiForm(
  Widget shellWidget,
  unsigned long timeout,
  unsigned char* client_title,
  XFontStruct* client_title_font,
  Pixel client_title_color,
  unsigned char* title,
  XFontStruct* title_font,
  Pixel title_color,
  int nprompts,
  unsigned char* promptStrings[],
  unsigned char* answerStrings[],
  XFontStruct* prompt_font,
  Pixel prompt_color,
  XFontStruct* answer_font,
  Pixel answer_color,
  int maxNameLength[],
  Boolean visible[])
{
    Cardinal status;
    int nArgs, i;
    int maxAnswerLength; /* Not used.  Maybe in here for future alignment....*/
    Arg pArgs[15];
    XmString s1, s2;
    XmFontList f1;
    Dimension width, height, top_offset, left_offset;
    XEvent event;
    Widget w;
    FormWidgets* promptWidgets;
    Widget* resultWidgets;
    char str[maxidlen];
    FormNCallbackInfo* pNInfo;
    FormOptionRec optionRec;
    FormCallbackInfo info[FormLast];
    PFormCallbackInfo pInfo = info;
    enum FormWidgetKind iWidget;
    FormWidgets formWidgets[FormLast];
    char* uidFile[1];
    MrmHierarchy hierarchyId;
    MrmType widgetClass;
    FormData formData;
    XmTextSource source;
    Pixel tbackground;
    Screen* scrn;
    Display* dpy;
    int* curNameLength;
    int maxHeight, maxWidth;
    XtTranslations result_translations;
    XSetWindowAttributes sAttributes;
    char *dpy_name;
    Atom *prop_list;
    char *prop_name;
    int num_prop;

    abort_status = False;

    if (GpiEstablishShell(shellWidget) == GpiAbort) return(GpiAbort);

    promptWidgets = (FormWidgets *) XtMalloc(nprompts * sizeof(FormWidgets));
    resultWidgets = (Widget *) XtMalloc(nprompts * sizeof(Widget));
    /* needs to be freed;  2 above also? */
    curNameLength = (int *)XtMalloc(nprompts*sizeof(int));
    /* needs to be freed */
    pNInfo = (FormNCallbackInfo*)XtMalloc(nprompts*sizeof(FormNCallbackInfo));
    for (i = 0; i < nprompts; i++) {
        pNInfo[i].which_widget = i;
        pNInfo[i].pToInfo = pInfo;
    };

    formData.answer_widgets = resultWidgets;
    formData.num_answers = nprompts;
    formData.selections = promptStrings;
    formData.answer = answerStrings;
    formData.answer_maxlen = maxNameLength;
    formData.answer_curlen = curNameLength;
    for (i = 0; i < nprompts; i++) {
        formData.answer_curlen[i] = 0;
    };
    formData.answer_visible = visible;

    dpy = XtDisplay(GpiShellWidget);
    scrn = DefaultScreenOfDisplay(dpy); 

    /*
     * Determine whether there is a window manager
     */
    wm_present = False;

    dpy_name = DisplayString(dpy);
    if ((strchr(dpy_name, ':') != dpy_name)
	&& (0 != strncmp("local", dpy_name, 5)))
	{
	/*
	 * Display is not local, may be WM running on remote X terminal
	 */
	prop_list = XListProperties(dpy, RootWindowOfScreen(scrn), &num_prop);
	if (prop_list)
	    {
	    for (i=0; i<num_prop; i++)
		{
		prop_name = XGetAtomName(dpy, prop_list[i]);
		if ((0 == strcmp(prop_name, "_MOTIF_WM_INFO"))
		    || (0 == strcmp(prop_name, "WM_ICON_SIZE"))
		    || (0 == strcmp(prop_name, "WM_STATE")))
		    wm_present = True;
		XFree(prop_name);
		if (wm_present)
		    break;
		}
	    XFree(prop_list);
	    }

	/*
	 * If we still haven't detected the presence of a window manager, we
	 * now resort to more crude methods to make sure there isn't one.
	 */
	if (!wm_present)
	    {
	    orig_handler = XSetErrorHandler(local_xerror);
	    sAttributes.event_mask = SubstructureRedirectMask;
	    XChangeWindowAttributes(dpy, RootWindowOfScreen(scrn),
				    CWEventMask, &sAttributes);
	    XSync (dpy, False );
	    XSetErrorHandler(orig_handler);
	    /* clear the event mask so a window manager can start later */
	    sAttributes.event_mask = NoEventMask;
	    XChangeWindowAttributes(dpy, RootWindowOfScreen(scrn),
				    CWEventMask, &sAttributes);
	    }

	}
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNmwmDecorations, 0); nArgs++; 
    XtSetArg(pArgs[nArgs], XmNmwmFunctions, 0); nArgs++; 
    XtSetArg(pArgs[nArgs], XmNinput, True); nArgs++; 
    XtSetArg(pArgs[nArgs], XmNx, XWidthOfScreen(scrn)/2); nArgs++; 
    XtSetArg(pArgs[nArgs], XmNy, XHeightOfScreen(scrn)/2); nArgs++; 
    formWidgets[FormTop].widget = XtCreatePopupShell("GpiForm",
        topLevelShellWidgetClass, GpiShellWidget,pArgs,nArgs);

    uidFile[0] = UID_FILE_NAME;

    status = MrmOpenHierarchy((MrmCount)(1), uidFile, 0, &hierarchyId);
    if(status != MrmSUCCESS) fprintf(stderr, "Open Hierarchy failed\n");

    SetFormInfo(info[FormTop],    FormTop,    formWidgets, &formData);
    SetFormInfo(info[FormForm],   FormForm,   formWidgets, &formData);
    SetFormInfo(info[FormClientTitle],  FormClientTitle,  formWidgets, &formData);
    SetFormInfo(info[FormTitle],  FormTitle,  formWidgets, &formData);
    SetFormInfo(info[FormOk],     FormOk,     formWidgets, &formData);
    SetFormInfo(info[FormAbort],  FormAbort,  formWidgets, &formData);
    SetFormInfo(info[FormPrompt], FormPrompt, formWidgets, &formData);

    /* Set up the uil interface names */
    nArgs = 0;
    XtSetArg(pArgs[nArgs], "FormCCallback",      FormCCallback    ); nArgs++;
    XtSetArg(pArgs[nArgs], "FormCBAbort",        FormCBAbort      ); nArgs++;
    XtSetArg(pArgs[nArgs], "FormCBContinue",     FormCBContinue   ); nArgs++;
    XtSetArg(pArgs[nArgs], "pNInfo",             pNInfo           ); nArgs++;
    XtSetArg(pArgs[nArgs], "pInfo",              pInfo            ); nArgs++;
    XtSetArg(pArgs[nArgs], "clientTitle",        &info[FormClientTitle] ); nArgs++;
    XtSetArg(pArgs[nArgs], "title",              &info[FormTitle] ); nArgs++;
    XtSetArg(pArgs[nArgs], "prompts",            &info[FormPrompt]); nArgs++;
    XtSetArg(pArgs[nArgs], "Ok",                 &info[FormOk]    ); nArgs++;
    XtSetArg(pArgs[nArgs], "Abort",              &info[FormAbort] ); nArgs++;
    status = MrmRegisterNames(pArgs, nArgs);
    if(status != MrmSUCCESS) fprintf(stderr, "Register Names failed\n");

    /* Load the widgets. */
    status = MrmFetchWidget(hierarchyId, "GpiForm", formWidgets[FormTop].widget,
        &formWidgets[FormForm].widget, &widgetClass);
    if(status != MrmSUCCESS) fprintf(stderr, "Fetch Widget failed\n");
    MrmCloseHierarchy(hierarchyId);
    XtManageChild(formWidgets[FormForm].widget);

    /* Get default x, y, width, and height of all widgets. */
    for(iWidget = FormTop; iWidget < FormLast; iWidget++) {
        nArgs = 0;
        XtSetArg(pArgs[nArgs], XmNx, &formWidgets[iWidget].defX);      nArgs++;
        XtSetArg(pArgs[nArgs], XmNy, &formWidgets[iWidget].defY);      nArgs++;
        XtSetArg(pArgs[nArgs], XmNwidth, &formWidgets[iWidget].defW);  nArgs++;
        XtSetArg(pArgs[nArgs], XmNheight, &formWidgets[iWidget].defH); nArgs++;
        XtGetValues(formWidgets[iWidget].widget, pArgs, nArgs);
    }

    XtGetApplicationResources(formWidgets[FormForm].widget,         
        (XtPointer) &optionRec, FormResources, XtNumber(FormResources), (Arg*) NULL, 0);

    nArgs = 0;
    if (!client_title) {
        XtSetArg(pArgs[nArgs], XmNheight, 0);               nArgs++;
        s1 = XmStringCreateLtoR("", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(pArgs[nArgs], XmNlabelString, s1); nArgs++;
        XtSetValues(formWidgets[FormClientTitle].widget,  pArgs, nArgs);
        XmStringFree(s1);
    }
    else {
        s1 = XmStringCreateLtoR(client_title, XmSTRING_DEFAULT_CHARSET);
        XtSetArg(pArgs[nArgs], XmNlabelString, s1); nArgs++;

	if (client_title_font)
	    {
	    f1 = XmFontListCreate(client_title_font, XmSTRING_DEFAULT_CHARSET);
	    XtSetArg(pArgs[nArgs], XmNfontList, f1);            nArgs++;
	    }
        XtSetArg(pArgs[nArgs], XmNforeground, client_title_color); nArgs++;

        XtSetValues(formWidgets[FormClientTitle].widget,  pArgs, nArgs);
        XmStringFree(s1);

	if (client_title_font)
	    XmFontListFree(f1);

    };


    nArgs = 0;
    if (!title) {
        XtSetArg(pArgs[nArgs], XmNheight, 0);                nArgs++;
        s1 = XmStringCreateLtoR("", XmSTRING_DEFAULT_CHARSET);
        XtSetArg(pArgs[nArgs], XmNlabelString, s1); nArgs++;
        XtSetValues(formWidgets[FormTitle].widget,  pArgs, nArgs);
        XmStringFree(s1);
    }
    else {
        s1 = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
        XtSetArg(pArgs[nArgs], XmNlabelString, s1); nArgs++;
        f1 = XmFontListCreate(title_font, XmSTRING_DEFAULT_CHARSET);
        XtSetArg(pArgs[nArgs], XmNfontList, f1);            nArgs++;
        XtSetArg(pArgs[nArgs], XmNforeground, title_color); nArgs++;
        XtSetValues(formWidgets[FormTitle].widget,  pArgs, nArgs);
        XmStringFree(s1);
        XmFontListFree(f1);
    };

    /* load the question and answer widgets dynamically into the prompt form */
    w = NULL;
    maxAnswerLength = 0;
    top_offset = 5;
    left_offset = 5;
    result_translations =
        XtParseTranslationTable(FormResultTranslationTable);
    for (i = 0; i < nprompts; i++) {
        sprintf(str,"Prompt %d",i);
        s2 = XmStringCreateLtoR(promptStrings[i], XmSTRING_DEFAULT_CHARSET);
        nArgs = 0;
        XtSetArg(pArgs[nArgs], XmNlabelString, s2);                nArgs++;
        XtSetArg(pArgs[nArgs], XmNtopAttachment, XmATTACH_WIDGET); nArgs++;
        XtSetArg(pArgs[nArgs], XmNtopWidget, w);                   nArgs++;
        XtSetArg(pArgs[nArgs], XmNtopOffset, top_offset);          nArgs++;
        XtSetArg(pArgs[nArgs], XmNleftAttachment, XmATTACH_FORM);  nArgs++;
        XtSetArg(pArgs[nArgs], XmNleftOffset, left_offset);        nArgs++;
        XtSetArg(pArgs[nArgs], XmNalignment, XmALIGNMENT_END);     nArgs++;
        if (prompt_font != 0) {
            f1 = XmFontListCreate(prompt_font, XmSTRING_DEFAULT_CHARSET);
            if (f1 != 0) {XtSetArg(pArgs[nArgs], XmNfontList, f1); nArgs++;};
        };
        XtSetArg(pArgs[nArgs], XmNforeground, prompt_color); nArgs++;
        promptWidgets[i].widget = XmCreateLabel(formWidgets[FormPrompt].widget,
                str, pArgs, nArgs);
        if (prompt_font != 0) XmFontListFree(f1);

        nArgs = 0;
        XtSetArg(pArgs[nArgs], XmNeditable,   True);                nArgs++;
        XtSetArg(pArgs[nArgs], XmNmaxLength,  maxNameLength[i]);    nArgs++;
        XtSetArg(pArgs[nArgs], XmNtopWidget, w);                    nArgs++;
        XtSetArg(pArgs[nArgs], XmNtopOffset, top_offset);           nArgs++;
        XtSetArg(pArgs[nArgs], XmNtopAttachment, XmATTACH_WIDGET);  nArgs++;
        XtSetArg(pArgs[nArgs], XmNleftWidget, promptWidgets[i].widget); nArgs++;
        XtSetArg(pArgs[nArgs], XmNleftAttachment, XmATTACH_WIDGET); nArgs++;
        XtSetArg(pArgs[nArgs], XmNleftOffset, left_offset);         nArgs++;

        if (!visible[i]) {
            XtSetArg(pArgs[nArgs], XmNverifyBell, False);           nArgs++;
        };
        if (answer_font != 0) {
            f1 = XmFontListCreate(answer_font, XmSTRING_DEFAULT_CHARSET);
            if (f1 != 0) {XtSetArg(pArgs[nArgs], XmNfontList, f1); nArgs++;};
        };
        XtSetArg(pArgs[nArgs], XmNforeground, answer_color); nArgs++;
        resultWidgets[i] = XmCreateText(formWidgets[FormPrompt].widget,
            str, pArgs, nArgs);
        XtOverrideTranslations(resultWidgets[i], result_translations);

        if (answer_font != 0) XmFontListFree(f1);
        w = resultWidgets[i];

 /* make it invisible */

        if(!visible[i]) {            
            source = GetSrc(w);
            source->SetSelection = NullSetSelection;
            source->GetSelection = NullGetSelection;

            nArgs = 0;
            XtSetArg(pArgs[nArgs], XmNautoShowCursorPosition, False); nArgs++;
            XtSetArg(pArgs[nArgs], XmNcursorPositionVisible, False); nArgs++;
            XtSetValues(w, pArgs, nArgs);
            _XmTextDisableRedisplay(w,TRUE);
        }

        if(maxAnswerLength < maxNameLength[i]) maxAnswerLength = maxNameLength[i];
    };

    XtManageChildren(resultWidgets, nprompts);
    for (i = 0; i < nprompts; i++) {
        XtManageChild(promptWidgets[i].widget);
    };

    /* Set ok button resource. */
    s1 = XmStringCreateLtoR("OK", XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNlabelString, s1); nArgs++;
    XtSetValues(formWidgets[FormOk].widget, pArgs, nArgs);
    XmStringFree(s1);

    /* Set abort button resource. */
    s1 = XmStringCreateLtoR("Cancel", XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNlabelString, s1); nArgs++;
    XtSetValues(formWidgets[FormAbort].widget, pArgs, nArgs);
    XmStringFree(s1);


    /* Get new x, y, width, and height of some widgets. */
    for (i = 0; i < nprompts; i++){
	Dimension	ypos;
	int		new_offset;

        nArgs = 0;
        XtSetArg(pArgs[nArgs], XmNx, &promptWidgets[i].posX);    nArgs++;
        XtSetArg(pArgs[nArgs], XmNy, &promptWidgets[i].posY);    nArgs++;
        XtSetArg(pArgs[nArgs], XmNwidth, &promptWidgets[i].dW);  nArgs++;
        XtSetArg(pArgs[nArgs], XmNheight, &promptWidgets[i].dH); nArgs++;
        XtGetValues(promptWidgets[i].widget, pArgs, nArgs);

	/*
	 * Try to line up the prompt labels and result Text widgets.
	 */
	XtSetArg(pArgs[0], XmNy, &ypos);
	XtSetArg(pArgs[1], XmNheight, &height);
	XtGetValues(resultWidgets[i], pArgs, 2);
	/* this is fudged to work best with the default font */
	new_offset = (ypos + height) -
			(promptWidgets[i].posY + promptWidgets[i].dH) - 2;
	
	if (new_offset > 0)
	    {
	    XtSetArg(pArgs[0], XmNtopOffset, (Dimension)new_offset); 
	    XtSetValues(promptWidgets[i].widget, pArgs, 1);
	    }
    };
    
    for(iWidget = FormTop; iWidget < FormLast; iWidget++) {
        nArgs = 0;
        XtSetArg(pArgs[nArgs], XmNx, &formWidgets[iWidget].posX);      nArgs++;
        XtSetArg(pArgs[nArgs], XmNy, &formWidgets[iWidget].posY);      nArgs++;
        XtSetArg(pArgs[nArgs], XmNwidth, &formWidgets[iWidget].dW);  nArgs++;
        XtSetArg(pArgs[nArgs], XmNheight, &formWidgets[iWidget].dH); nArgs++;
        XtGetValues(formWidgets[iWidget].widget, pArgs, nArgs);
    }

    /*  Fix up widgets so they look nice. */
    maxWidth = 0;
    maxHeight = 0;
    for (i = 0; i < nprompts; i++){
        maxWidth = MAX(maxWidth, promptWidgets[i].dW);
        maxHeight = MAX(maxHeight, promptWidgets[i].dH);
    };
    maxWidth *= 1.25;  /* hack fudge factor */
    for (i = 0; i < nprompts; i++){
        nArgs = 0;
        XtSetArg(pArgs[nArgs], XmNwidth, maxWidth); nArgs++;
        XtSetArg(pArgs[nArgs], XmNheight, maxHeight); nArgs++;
        XtSetValues(promptWidgets[i].widget, pArgs, nArgs);
    };

    /* ok button */
    nArgs = 0;
    if (formWidgets[FormOk].dW < formWidgets[FormOk].defW) {
        XtSetArg(pArgs[nArgs], XmNwidth, formWidgets[FormOk].defW); nArgs++;
    };
    if (formWidgets[FormOk].dH < formWidgets[FormOk].defH) {
        XtSetArg(pArgs[nArgs], XmNheight, formWidgets[FormOk].defH); nArgs++;
    };
    if (nArgs)
        XtSetValues(formWidgets[FormOk].widget, pArgs, nArgs);

    /* abort button */
    nArgs = 0;
    if (formWidgets[FormAbort].dW < formWidgets[FormAbort].defW) {
        XtSetArg(pArgs[nArgs], XmNwidth, formWidgets[FormAbort].defW); nArgs++;
    };
    if (formWidgets[FormAbort].dH < formWidgets[FormAbort].defH) {
        XtSetArg(pArgs[nArgs], XmNheight, formWidgets[FormAbort].defH); nArgs++;
    };
    if (nArgs)
        XtSetValues(formWidgets[FormAbort].widget, pArgs, nArgs);

    /* Center the top widget on the screen. */
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNwidth, &width);   nArgs++;
    XtSetArg(pArgs[nArgs], XmNheight, &height); nArgs++;
    XtGetValues(formWidgets[FormForm].widget, pArgs, nArgs);
        
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNx, ((XWidthOfScreen(scrn) - width)/2)); nArgs++; 
    XtSetArg(pArgs[nArgs], XmNy, ((XHeightOfScreen(scrn) - height)/2)); nArgs++; 
    XtSetValues(formWidgets[FormTop].widget, pArgs, nArgs);

    /* Do login prompting. */
    pInfo->mayDisplayTimeout = True;
    if ((pInfo->timeout = timeout) != 0) {
        pInfo->timerId = 
            XtAppAddTimeOut(XtWidgetToApplicationContext(GpiShellWidget),
                timeout, FormTimedOut, (XtPointer) pInfo);
    }
    XtPopup(formWidgets[FormTop].widget, XtGrabNone);
    XtSetKeyboardFocus(formWidgets[FormTop].widget, NULL);

    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNwidth, &width);   nArgs++;
    XtSetArg(pArgs[nArgs], XmNheight, &height); nArgs++;
    XtGetValues(formWidgets[FormForm].widget, pArgs, nArgs);
        
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNx, ((XWidthOfScreen(scrn) - width)/2));  nArgs++; 
    XtSetArg(pArgs[nArgs], XmNy, ((XHeightOfScreen(scrn) - height)/2)); nArgs++; 
    XtSetValues(formWidgets[FormTop].widget, pArgs, nArgs);

    /*
     * Set the first text field so that it is "current", 
     * that is the active member of the tab group.
     */
    XmProcessTraversal(resultWidgets[0],XmTRAVERSE_CURRENT);

    /*
     * change it so that ENTER in the last text field is the
     * same action as an "OK" button.
     * This probably violates the Motif Style Guide but causes
     * the login box to mimic the behavior of Ultrix dxlogin
     * and MIT clean xdm (and thus meets user expectations)
     */
    XtAddCallback(resultWidgets[nprompts-1],XmNactivateCallback, 
	FormCBContinue, (XtPointer)pInfo);

    /*
     * make sure we have input focus, regardless of mouse position
     */
    if (!wm_present)
	XSetInputFocus(XtDisplay (formWidgets[FormTop].widget),
		    XtWindow (formWidgets[FormTop].widget),
		    RevertToPointerRoot,
		    CurrentTime);


    formData.returnValue = GpiINeither;
    if(! SETJMP(pInfo->timerJmpBuf)) {
        while(formData.returnValue == GpiINeither) {
            XtAppNextEvent(XtWidgetToApplicationContext(GpiShellWidget), &event);
            XtDispatchEvent(&event);
        }
        if(timeout && pInfo->timerId) XtRemoveTimeOut(pInfo->timerId);
    }
    XtUnmanageChild(formWidgets[FormTop].widget);
    XtDestroyWidget(formWidgets[FormTop].widget);
    return((GpiReturnValue) pInfo->pFormData->returnValue);
}



int GpiLoginFail(
	Widget	 shellWidget,
	unsigned long timeout,
	char*	 failString)
{
    Arg                	pArgs[10];
    int                	nArgs;
    FailOptionRec	optionRec;
    XmString        	s1, s2, s3;
    XmFontList        	f1;
    GpiCallbackInfo	gpiCallbackInfo; 
    Widget        	failWidget;
    Widget        	buttonWidget;
    XEvent        	event;
    Screen 		*scrn;
    Display		*dpy;
    Dimension		width, height;

    if (abort_status)
	return True;
    
    if (GpiEstablishShell(shellWidget) == GpiAbort)
        return abort_status;

    dpy = XtDisplay(GpiShellWidget);
    scrn = DefaultScreenOfDisplay(dpy); 


    s1 = XmStringCreateLtoR(failString, XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNmwmDecorations, 0); nArgs++; 
    XtSetArg(pArgs[nArgs], XmNmwmFunctions, 0); nArgs++; 
    XtSetArg(pArgs[nArgs], XmNmessageString,   s1  ); nArgs++;
    XtSetArg(pArgs[nArgs], XmNnoResize,        True); nArgs++;
    XtSetArg(pArgs[nArgs], XmNhelpLabelString, NULL); nArgs++;
    failWidget = XmCreateWarningDialog(GpiShellWidget, "GpiLoginFail",
                                	pArgs, nArgs);
    XmStringFree(s1);

    XtGetApplicationResources(failWidget,
                	       (XtPointer) &optionRec,
                	       FailResources,
                	       XtNumber(FailResources),
                	       (Arg*) NULL, 0);
    s1 = XmStringCreateLtoR(optionRec.pFail,     XmSTRING_DEFAULT_CHARSET);
    s2 = XmStringCreateLtoR(optionRec.pOkLabel,  XmSTRING_DEFAULT_CHARSET);
    s3 = XmStringCreateLtoR(optionRec.pTitle,    XmSTRING_DEFAULT_CHARSET);
    f1 = XmFontListCreate(  optionRec.pFailFont, XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    if (failString == NULL)
	{
        	XtSetArg(pArgs[nArgs], XmNmessageString, s1); nArgs++;
	}
    XtSetArg(pArgs[nArgs], XmNokLabelString, s2                  ); nArgs++;
    XtSetArg(pArgs[nArgs], XmNdialogTitle,   s3                  ); nArgs++;
    XtSetArg(pArgs[nArgs], XmNlabelFontList, f1                  ); nArgs++;
    XtSetArg(pArgs[nArgs], XmNforeground,    optionRec.cFailColor); nArgs++;
    XtSetValues(failWidget, pArgs, nArgs);
    XmStringFree(s1);
    XmStringFree(s2);
    XmStringFree(s3);
    XmFontListFree(f1);
    
    buttonWidget = XmMessageBoxGetChild(failWidget, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(buttonWidget);
    buttonWidget =
        XmMessageBoxGetChild(failWidget, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild(buttonWidget);
#if 0
    /* unused */
    XtAddCallback(failWidget, XmNcancelCallback,
        	   GeneralAbort, (XtPointer) &gpiCallbackInfo);
#endif
    XtAddCallback(failWidget, XmNokCallback,
        	   GeneralContinue, (XtPointer) &gpiCallbackInfo);
    XtManageChild(failWidget);


    /*
     * Center the dialog box on the screen.
     */
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNwidth, &width); nArgs++;
    XtSetArg(pArgs[nArgs], XmNheight, &height); nArgs++;
    XtGetValues(failWidget, pArgs, nArgs);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNx, ((XWidthOfScreen(scrn) - width)/2)); nArgs++;
    XtSetArg(pArgs[nArgs], XmNy, ((XHeightOfScreen(scrn) - height)/2)); nArgs++;
    XtSetValues(failWidget, pArgs, nArgs);

    /* 
     * make sure we have input focus, regardless of mouse position
     */
    if (!wm_present)
	XSetInputFocus(XtDisplay (failWidget), 
		       XtWindow (failWidget),
		       RevertToPointerRoot, 
		       CurrentTime);

    if (timeout == 0)
    {
        	timeout = optionRec.iFailTimeout * 1000;
    }
    if ((gpiCallbackInfo.timeout = timeout) != 0)
    {
        	gpiCallbackInfo.timerId = 
	    	XtAppAddTimeOut(XtWidgetToApplicationContext(GpiShellWidget),
                	     timeout, TimedOut, (XtPointer) &gpiCallbackInfo);
    }
    gpiCallbackInfo.mayDisplayTimeout = False;
    gpiCallbackInfo.returnValue = GpiINeither;
    if (! SETJMP(gpiCallbackInfo.timerJmpBuf))
    {
        	while (gpiCallbackInfo.returnValue == GpiINeither)
        	{
                	XtAppNextEvent(XtWidgetToApplicationContext(GpiShellWidget),&event);
                	XtDispatchEvent(&event);
        	}
        	if (timeout && gpiCallbackInfo.timerId)
                	XtRemoveTimeOut(gpiCallbackInfo.timerId);
    }
    XtDestroyWidget(XtParent(failWidget));
    return abort_status;
}

GpiReturnValue GpiEstablishShell(
	Widget shellWidget)
{
    if (GpiShellWidget == (Widget) NULL)
    {
        	Arg pArgs[10];
        	int nArgs;
	
        	if (shellWidget)
        	{
                	GpiShellWidget = shellWidget;
                	nArgs = 0;
                	XtSetArg(pArgs[nArgs], "GpiContinue", GContinue); nArgs++; 
                	XtSetArg(pArgs[nArgs], "GpiAbort",    GAbort   ); nArgs++; 
                	XtSetArg(pArgs[nArgs], "abort-login", LAbort   ); nArgs++; 
                	XtSetArg(pArgs[nArgs], "delete-login", LDelete ); nArgs++;
    			XtSetArg(pArgs[nArgs], "set-session-argument", GpiSetSessionArgument ); nArgs++;
 
                	XtAppAddActions(XtWidgetToApplicationContext(GpiShellWidget),
                                	 (XtActionList)pArgs, nArgs);
                	return(GpiContinue);
        	}
	    
        	{ /* a new block */
                	XtAppContext context;
        		extern char* getenv();
                	char*	       name = getenv("DISPLAY");
                	Display*     display;
                	int          argc = 0;
                	String*      argv = NULL;
                	Screen*      screen;
                	Position     x, y;
                	  
                	if (! GpiInitedXt)
                	{
                        	XtToolkitInitialize();
                        	GpiInitedXt = True;
                	}
                	context = XtCreateApplicationContext();
                	if (! name)
                        	name = ":0.0";
                	display = XtOpenDisplay(context, name, "gpi name",
                                        	   "gpi class", NULL, 0, &argc, argv);
                	if (! display)
                	{
                        	String values[2];
                        	Cardinal num;
                        	values[0] = name;
                        	values[1] = NULL;
                        	num = 1;
                        	XtAppWarningMsg(context, "invalidDisplay", "gpi",
                                        	   "gpi class",
                                        	   "gpi: Could not open display %s",
                                        	   values, &num);
                        	return(GpiAbort);
                	}
                	  
                	screen = DefaultScreenOfDisplay(display);
                	x = XWidthOfScreen(screen)/2;
                	y = XHeightOfScreen(screen)/2;
                	  
                	nArgs = 0;
                	XtSetArg(pArgs[nArgs], XmNborderWidth, 0     ); nArgs++;
                	XtSetArg(pArgs[nArgs], XmNx,	  	        x     ); nArgs++;
                	XtSetArg(pArgs[nArgs], XmNy,	    	        y     ); nArgs++;
                	XtSetArg(pArgs[nArgs], XmNwidth,	        1     ); nArgs++;
                	XtSetArg(pArgs[nArgs], XmNheight,            1     ); nArgs++;
                	XtSetArg(pArgs[nArgs], XmNscreen,            screen); nArgs++;
                	XtSetArg(pArgs[nArgs], XmNmappedWhenManaged, False ); nArgs++;
                	GpiShellWidget = XtAppCreateShell("gpi name", "gpi class",
                                                	  applicationShellWidgetClass,
                                                	  display, pArgs, nArgs);
                	if (! GpiShellWidget)
                	{
                        	XtAppWarningMsg(context, "createAppShellFailed", "gpi",
                                        	   "gpi class",
                                        	   "gpi: Could not create application shell.",
                                        	   (String*) NULL, (Cardinal*) 0);
                        	return(GpiAbort);
                	}
                	XtRealizeWidget(GpiShellWidget);
                	nArgs = 0;
                	XtSetArg(pArgs[nArgs], "GpiContinue", GContinue); nArgs++; 
                	XtSetArg(pArgs[nArgs], "GpiAbort",    GAbort   ); nArgs++; 
                	XtSetArg(pArgs[nArgs], "abort-login", LAbort   ); nArgs++; 
                        XtSetArg(pArgs[nArgs], "delete-login", LDelete ); nArgs++;
                	XtAppAddActions(XtWidgetToApplicationContext(GpiShellWidget),
                                	   (XtActionList)pArgs, nArgs);
        	}
	}
	return(GpiContinue);
}    

void GpiRemoveShell()
  {
    GpiShellWidget = (Widget) NULL;
  }

/*****************************************************************************
 * Private routines                                                	     *
 *****************************************************************************/

static void DisplayMessage(
	Boolean	  mayDisplayTimeout,
	unsigned long timeout,
	char* 	  warningString)
{
    Arg                	pArgs[10];
    int                	nArgs;
    XmString        	s1, s2;
    MessageOptionRec	optionRec;
    GpiCallbackInfo	gpiCallbackInfo; 
    Widget        	warningWidget;
    Widget        	buttonWidget;
    XEvent        	event;
    Dimension        	width, height, dist;
	Screen *scrn;
	Display*     dpy;
    
    DEBUG("Entering DisplayMessage\n");
    s1 = XmStringCreateLtoR(warningString, XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNmessageString,   s1  ); nArgs++;
    XtSetArg(pArgs[nArgs], XmNnoResize,        True); nArgs++;
    XtSetArg(pArgs[nArgs], XmNhelpLabelString, NULL); nArgs++;
    XtSetArg(pArgs[nArgs], XmNmwmDecorations, 0);        	nArgs++; 
    XtSetArg(pArgs[nArgs], XmNmwmFunctions, 0);        	nArgs++; 
    XtSetArg(pArgs[nArgs], XmNinput, True);                	nArgs++; 
    XtSetArg(pArgs[nArgs], XmNborderWidth, 1  );        	nArgs++;
    XtSetArg(pArgs[nArgs], XmNselectionLabelString, s1  ); nArgs++;
    XtSetArg(pArgs[nArgs], XmNhelpLabelString,      NULL); nArgs++;
    warningWidget = XmCreateWarningDialog(GpiShellWidget, "GpiDisplayMessage",
                                	   pArgs, nArgs);
    XmStringFree(s1);
    
    XtGetApplicationResources(warningWidget,
                	       (XtPointer) &optionRec,
                	       MessageResources,
                	       XtNumber(MessageResources),
                	       (Arg*) NULL, 0);
    s1 = XmStringCreateLtoR(optionRec.pOkLabel, XmSTRING_DEFAULT_CHARSET);
    s2 = XmStringCreateLtoR(optionRec.pTitle,   XmSTRING_DEFAULT_CHARSET);
    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNokLabelString, s1); nArgs++;
    XtSetArg(pArgs[nArgs], XmNdialogTitle,   s2); nArgs++;
    XtSetValues(warningWidget, pArgs, nArgs);
    XmStringFree(s1);
    XmStringFree(s2);
    
    buttonWidget = XmMessageBoxGetChild(warningWidget, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(buttonWidget);
    buttonWidget =
        XmMessageBoxGetChild(warningWidget, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild(buttonWidget);
#if 0
    /* unused */
    XtAddCallback(warningWidget, XmNcancelCallback,
        	   GeneralAbort, (XtPointer) &gpiCallbackInfo);
#endif
    XtAddCallback(warningWidget, XmNokCallback,
        	   GeneralContinue, (XtPointer) &gpiCallbackInfo);
    XtManageChild(warningWidget);

	dpy = XtDisplay(GpiShellWidget);
	scrn = DefaultScreenOfDisplay(dpy); 
	nArgs = 0;
	XtSetArg(pArgs[nArgs],XmNwidth,&width);	nArgs++;
	XtSetArg(pArgs[nArgs],XmNheight,&height);nArgs++;
	XtGetValues(warningWidget,pArgs,nArgs);
	
	nArgs = 0;
	XtSetArg(pArgs[nArgs], XmNx, 
        	((XWidthOfScreen(scrn) - width)/2)); nArgs++; 
    XtSetArg(pArgs[nArgs], XmNy, 
        	((XHeightOfScreen(scrn) - height)/2)); nArgs++; 
	XtSetValues(warningWidget,pArgs,nArgs);

    if ((gpiCallbackInfo.timeout = timeout) != 0)
	{
        	gpiCallbackInfo.timerId = 
        	    XtAppAddTimeOut(XtWidgetToApplicationContext(GpiShellWidget),
                	     timeout, TimedOut, (XtPointer) &gpiCallbackInfo);
    }
    gpiCallbackInfo.mayDisplayTimeout = mayDisplayTimeout;
    gpiCallbackInfo.returnValue = GpiINeither;
    if (! SETJMP(gpiCallbackInfo.timerJmpBuf))
    {
        	while (gpiCallbackInfo.returnValue == GpiINeither)
        	{
                	XtAppNextEvent(XtWidgetToApplicationContext(GpiShellWidget),&event);
                	XtDispatchEvent(&event);
        	}
        	if (timeout && gpiCallbackInfo.timerId)
                	XtRemoveTimeOut(gpiCallbackInfo.timerId);
	}
    XtDestroyWidget(XtParent(warningWidget));
    DEBUG("Leaving DisplayMessage\n");
}

/*****************************************************************************
 * Callbacks                                                        	     *
 *****************************************************************************/

static void GeneralAbort(
	Widget	  widget,	
	XtPointer clientData,
	XtPointer callData)
{
    PGpiCallbackInfo pInfo = (PGpiCallbackInfo) clientData;

    pInfo->returnValue = GpiIAbort;
}

static void GeneralContinue(
	Widget	 widget,
	XtPointer clientData,
	XtPointer callData)
{
    PGpiCallbackInfo pInfo = (PGpiCallbackInfo) clientData;

    pInfo->returnValue = GpiIContinue;
}

static void PromptContinue(
	Widget    widget,
	XtPointer clientData,
	XtPointer callData)
{
    PGpiCallbackInfo pInfo = (PGpiCallbackInfo) clientData;
    XmSelectionBoxCallbackStruct* pData =
        (XmSelectionBoxCallbackStruct*) callData;
    char* plainText;

    if(! XmStringGetLtoR(pData->value, XmSTRING_DEFAULT_CHARSET, &plainText))
        plainText = NULL;
    
    if(plainText)
        	strncpy(pInfo->answerString, plainText, pInfo->maxAnswerLength);
    else
        *(pInfo->answerString) = '\0';
    pInfo->returnValue = GpiIContinue;
}

static void PromptNoMatch(
	Widget    widget,
	XtPointer clientData,
	XtPointer callData)
{
    PGpiCallbackInfo pInfo = (PGpiCallbackInfo) clientData;
    XmSelectionBoxCallbackStruct* pData =
        (XmSelectionBoxCallbackStruct*) callData;
    char* plainText;

    DEBUG("Entering PromptNoMatch\n");
    if (pInfo->timeout && pInfo->timerId)
    {
        	XtRemoveTimeOut(pInfo->timerId);
        	pInfo->timerId = 0;
    }

    if (! XmStringGetLtoR(pData->value, XmSTRING_DEFAULT_CHARSET,&plainText))
        plainText = NULL;
    if (plainText)
    {
        	int i;
        	for (i = 0; i < pInfo->numArrayStrings; i++)
        	{
                	if (strcmp(plainText, pInfo->arrayOfStrings[i]) == 0)
                	{
                        	strncpy(pInfo->answerString,plainText,pInfo->maxAnswerLength);
                        	pInfo->returnValue = GpiIContinue;
                        	break;
                	}
        	}
    }
    if (pInfo->returnValue == GpiINeither)
    {
        	DisplayMessage(False, pInfo->timeout,
                	"You must choose from the given options!");
    }
    if (pInfo->timeout)
    {
        	pInfo->timerId = 
                	XtAppAddTimeOut(XtWidgetToApplicationContext(GpiShellWidget),
                	     pInfo->timeout, TimedOut, (XtPointer) pInfo);
    }
    DEBUG("Leaving PromptNoMatch\n");
}

static void LoginAbort(
	Widget	widget,
	XtPointer	client_data,
	XtPointer	call_data)
{
    PLoginCallbackInfo pInfo = (PLoginCallbackInfo) client_data;
    
    pInfo->pLoginData->returnValue = GpiIAbort;
    pInfo->pLoginData->pName = NULL;
    pInfo->pLoginData->pPassword = NULL; /* Shouldn't this do a free? */
    pInfo->pLoginData->pCurLength = 0;
}

static void LoginContinue(
	Widget	widget,
    XtPointer	client_data,
    XtPointer	call_data)
{
    PLoginCallbackInfo pInfo = (PLoginCallbackInfo) client_data;
    
    pInfo->pLoginData->returnValue = GpiIContinue;
    pInfo->pLoginData->pName =
        XmTextFieldGetString(pInfo->pLoginWidgets[Name].widget);
    pInfo->pLoginData->pPassword[pInfo->pLoginData->pCurLength] = '\0';
/*    pInfo->pLoginData->pPassword =
        	XmTextGetString(pInfo->pLoginWidgets[Password].widget);
*/
}

static void LoginModifyVerify(
    Widget	widget,
    XtPointer	client_data,
    XmTextVerifyPtr call_data)
{
    int length, indx;
    PLoginCallbackInfo pInfo = (PLoginCallbackInfo) client_data;

    
    pInfo->pLoginData->returnValue = GpiINeither; /* Do I need to do this? */
    if ((*call_data).reason != XmCR_MODIFYING_TEXT_VALUE) return;
    length = (*((*call_data).text)).length;

    if (length == 0) { /* terrible hack for delete. */
        if (pInfo->pLoginData->pCurLength > 0)
            pInfo->pLoginData->pCurLength = pInfo->pLoginData->pCurLength -1 ;
    }
    else {
        if (pInfo->pLoginData->pCurLength + length > pInfo->pLoginData->pMaxLength)
            length = pInfo->pLoginData->pMaxLength - pInfo->pLoginData->pCurLength;
        for (indx = 0; indx < length; indx++) {
            pInfo->pLoginData->pPassword[pInfo->pLoginData->pCurLength + indx] =
                (*((*call_data).text)).ptr[indx];
        };
        pInfo->pLoginData->pCurLength = pInfo->pLoginData->pCurLength + length;
    };
    (*call_data).doit = FALSE;
}


static XtActionProc LDelete(
    Widget      widget,
    XEvent*     event,
    String*     params,
    Cardinal*    num_params)
{
    int nArgs;
    Arg pArgs[5];
    PLoginCallbackInfo pInfo;

    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNuserData, &pInfo); nArgs++;
    XtGetValues(widget, pArgs, nArgs);


    if (pInfo) {
        pInfo->pLoginData->returnValue = GpiINeither; /* Do I need to do this? */
        if (pInfo->pLoginData->pCurLength > 0)
            pInfo->pLoginData->pCurLength = pInfo->pLoginData->pCurLength - 1;
    }
}


static void LoginCCallback(
	Widget	widget,
	XtPointer client_data,
	XtPointer call_data)
{
    PLoginCallbackInfo pInfo = (PLoginCallbackInfo) client_data;
    
    pInfo->pLoginWidgets[ pInfo->widgetTag ].widget = widget;
}

static Boolean NullGetSelection(source,start,end)
	XmTextSource source;
	XmTextPosition *start,*end;
{
	return FALSE;
}

static void NullSetSelection(source,start,end)
	XmTextSource source;
	XmTextPosition start,end;
{
}


/*****************************************************************************
 * Timers                                                        	     *
 *****************************************************************************/

static void TimedOut(
	XtPointer	   clientData,
	XtIntervalId* id)
{
    PGpiCallbackInfo pInfo = (PGpiCallbackInfo) clientData;

    DEBUG("Entering TimedOut\n");
    if (pInfo->timerId != *id)
        return;
    if (pInfo->returnValue == GpiINeither && pInfo->mayDisplayTimeout)
	{
        	DisplayMessage(False, pInfo->timeout,
                	"You have not answered\nin the alloted time.");
	}
    pInfo->timerId = 0;
    pInfo->returnValue = GpiIAbort;
    DEBUG("Leaving TimedOut -- ");
    LONGJMP(pInfo->timerJmpBuf, True);
}

/*****************************************************************************
 * Actions                                                        	     *
 *****************************************************************************/

static XtActionProc GContinue(
	Widget	 widget,
	XEvent*	 event,
	String*	 params,
	Cardinal* num_params)
{
    int nArgs;
    Arg pArgs[2];
    PGpiCallbackInfo pInfo;


    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNuserData, &pInfo); nArgs++;
    XtGetValues(widget, pArgs, nArgs);

    if (pInfo)
	{
        	char* plainText = XmTextGetString(pInfo->answerWidget);

        if (plainText)
        	{
                	if (pInfo->mustMatch)
	    	{
                        	int i;
        	
                        	if (pInfo->timeout && pInfo->timerId)
                        	{
                                	XtRemoveTimeOut(pInfo->timerId);
                                	pInfo->timerId = 0;
                        	}
                        	for (i = 0; i < pInfo->numArrayStrings; i++)
                        	{
                                	if (strcmp(plainText, pInfo->arrayOfStrings[i]) == 0)
                                	{
                                        	strncpy(pInfo->answerString, plainText,
                                                	pInfo->maxAnswerLength);
                                        	pInfo->returnValue = GpiIContinue;
                                        	break;
                                	}
                        	}
                        	if (pInfo->returnValue == GpiINeither)
                        	{
                                	DisplayMessage(False, pInfo->timeout,
                                                	"You must choose from the given options!");
                        	}
                        	if (pInfo->timeout)
                        	{
                                	pInfo->timerId = 
                                	XtAppAddTimeOut(XtWidgetToApplicationContext(
                                                                	   GpiShellWidget),
                                                	 pInfo->timeout, TimedOut,
                                                	 (XtPointer) pInfo);
                        	}
                	}
                	else
                	{
                        	strncpy(pInfo->answerString,plainText,pInfo->maxAnswerLength);
                        	pInfo->returnValue = GpiIContinue;
                	}
                	XtFree(plainText);
        	}
        	else
        	{
                	*(pInfo->answerString) = '\0';
                	pInfo->returnValue = GpiIContinue;
        	}
	}
}

static XtActionProc GAbort(
	Widget    widget,
	XEvent*   event,
	String*   params,
	Cardinal* num_params)
{
    int nArgs;
    Arg pArgs[2];
    PGpiCallbackInfo pInfo;

    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNuserData, &pInfo); nArgs++;
    XtGetValues(widget, pArgs, nArgs);

    if (pInfo)
	{
        	pInfo->returnValue = GpiIAbort;
	}
}

static XtActionProc LAbort(
	Widget	widget,
	XEvent*	event,
	String*	params,
	Cardinal*	num_params)
{
    int nArgs;
    Arg pArgs[5];
    PLoginCallbackInfo pInfo;

    nArgs = 0;
    XtSetArg(pArgs[nArgs], XmNuserData, &pInfo); nArgs++;
    XtGetValues(widget, pArgs, nArgs);

    if (pInfo)
    {
        	pInfo->pLoginData->returnValue = GpiIAbort;
        	pInfo->pLoginData->pName = NULL;
                pInfo->pLoginData->pPassword = NULL; /* Shouldn't this do a free? */
                pInfo->pLoginData->pCurLength = 0;
    }
}


/*****************************************************************************
 * List Callbacks *
 *****************************************************************************/
static void ListCCallback(
	Widget	widget,
	XtPointer client_data,
	XtPointer call_data)
{
    PListCallbackInfo pInfo = (PListCallbackInfo) client_data;
    
    pInfo->pListWidgets[ pInfo->widgetTag ].widget = widget;
}

static void ListCBAbort(
	Widget	widget,
	XtPointer	client_data,
	XtPointer	call_data)
{
	int i;

    PListCallbackInfo pInfo = (PListCallbackInfo) client_data;
    
    pInfo->pListData->returnValue = GpiIAbort;
	for(i=0;i<pInfo->pListData->num_answers;i++)
        	pInfo->pListData->answer[i] = '\0';
}

static void ListCBContinue(
	Widget	widget,
    XtPointer	client_data,
	XtPointer	call_data)
{
	int i,poscnt,len;
	int *poslist;
	Widget w;

    PListCallbackInfo pInfo = (PListCallbackInfo) client_data;
    
	w = pInfo->pListData->list_widget;
    pInfo->pListData->returnValue = GpiIContinue;
	for(i=0;i<pInfo->pListData->num_answers;i++)
        	pInfo->pListData->answer[i][0] = '\0';

	if(!XmListGetSelectedPos(w,&poslist,&poscnt))
        	return;

	if(poscnt > pInfo->pListData->num_answers)
        	poscnt = pInfo->pListData->num_answers;
	for(i=0;i < poscnt;i++)
	{
        	strncpy(pInfo->pListData->answer[i], 
                	pInfo->pListData->selections[poslist[i]],
                	pInfo->pListData->answer_maxlen[i]);
        	len = strlen(pInfo->pListData->selections[poslist[i]-1]);
        	if(len > pInfo->pListData->answer_maxlen[i])
                	len = pInfo->pListData->answer_maxlen[i];
        	pInfo->pListData->answer[i][len] = '\0';
	}
}

static void ListTimedOut(XtPointer clientData, XtIntervalId* id)
{
    PListCallbackInfo pInfo = (PListCallbackInfo) clientData;

	if (pInfo->timerId != *id)
        	return;
	if(pInfo->pListData->returnValue == GpiINeither && 
        	pInfo->mayDisplayTimeout)
	{
        	DisplayMessage(False, pInfo->timeout,
                	"You have not answered\nin the alloted time.");
	}
    pInfo->timerId = 0;
    pInfo->pListData->returnValue = GpiIAbort;
    LONGJMP(pInfo->timerJmpBuf, True);
}

/*****************************************************************************
 * Form Callbacks *
 *****************************************************************************/

static void FormCCallback(
	Widget	widget,
	XtPointer client_data,
	XtPointer call_data)
{
    PFormCallbackInfo pInfo = (PFormCallbackInfo) client_data;
    
    pInfo->pFormWidgets[ pInfo->widgetTag ].widget = widget;
}

static void FormCBAbort(
	Widget	widget,
	XtPointer	client_data,
	XtPointer	call_data)
{
    int i;
    PFormCallbackInfo pInfo = (PFormCallbackInfo) client_data;
    
/*    exit(OBEYSESS_DISPLAY); */

    abort_status = True;

    pInfo->pFormData->returnValue = GpiIAbort;
    for (i = 0; i < pInfo->pFormData->num_answers; i++) {
        pInfo->pFormData->answer[i][0] = '\0';
        if (!pInfo->pFormData->answer_visible[i])
            pInfo->pFormData->answer_curlen[i] = 0;
    };
}

static void FormCBContinue(
    Widget widget,
    XtPointer client_data,
    XtPointer call_data)
{
    int i, n, len;
    char *str;
    Arg pArgs[15];
    PFormCallbackInfo pInfo = (PFormCallbackInfo) client_data;
    

    pInfo->pFormData->returnValue = GpiIContinue;

    for (i = 0; i < pInfo->pFormData->num_answers; i++) {
	n = 0;
	XtSetArg(pArgs[n], XmNvalue, &str); n++;
	XtGetValues(pInfo->pFormData->answer_widgets[i], pArgs, n);
	len = strlen(str);
	if(len > pInfo->pFormData->answer_maxlen[i])
	    len =  pInfo->pFormData->answer_maxlen[i];
	strncpy(pInfo->pFormData->answer[i], str, len);
	pInfo->pFormData->answer[i][len] = '\0';
    }
}


static void FormTimedOut(XtPointer clientData,XtIntervalId* id)
{
    PFormCallbackInfo pInfo = (PFormCallbackInfo) clientData;

    if (pInfo->timerId != *id)
        return;
    if(pInfo->pFormData->returnValue == GpiINeither && 
        	pInfo->mayDisplayTimeout)
    {
        	DisplayMessage(False, pInfo->timeout,
                	"You have not answered\nin the alloted time.");
    }
    pInfo->timerId = 0;
    pInfo->pFormData->returnValue = GpiIAbort;
    LONGJMP(pInfo->timerJmpBuf, True);
}
