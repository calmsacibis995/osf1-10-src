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
static char *rcsid = "@(#)$RCSfile: motifshell.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 18:20:31 $";
#endif
 /* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: motifshell.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 18:20:31 $"
#endif
#endif

/****************************************************************************
 ****************************************************************************
 **
 **   File:     motifShell.c
 **
 **   Project:     Motif - widget examination program
 **
 **   Description: Program which shows resources of widgets
 **
 ****************************************************************************
 ****************************************************************************/

/***************************************************
*                                                  *
*  Revision history:                               *
*                                                  *
*  05/26/89      strong        Initial Version     *
*  06/01/89      strong        1.0                 *
*  06/26/89      pjlevine      complete rewrite    *
*  03/12/92      deblois       yet another rewrite *
*                                                  *
****************************************************/

#include <sys/file.h>

/*  Standard C headers  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>

/*  X headers  */
#include <X11/IntrinsicP.h>


/*  Xm headers  */
#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MenuShell.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>

#ifdef VMS
#define APP_CLASS       "XMdemo"
#define DEFAULT_FONT    "fixed"
#define TEST_STRING     "The quick brown fox jumps over the lazy dog."
#define TEST_HELP       "Type here to test the font.\nApply sets this window.\nOK sets main window."
#define HELP_FILE       "help."
#define WELCOME_FILE    "welcome."
#define MEMBERSHIP_FILE "membership."
#define RESEARCH_FILE   "research."
#define PRINCIPLES_FILE "principles."
#define MOTIF_FILE      "motif."
#define READ            0
#define WRITE           1
#else
#define APP_CLASS       "XMdemo"
#define DEFAULT_FONT    "fixed\0"
#define TEST_STRING     "The quick brown fox jumps over the lazy dog."
#define TEST_HELP       "Type here to test the font.\nApply sets this window.\nOK sets main window."
#define HELP_FILE       "help"
#define WELCOME_FILE    "welcome"
#define MEMBERSHIP_FILE "membership"
#define RESEARCH_FILE   "research"
#define PRINCIPLES_FILE "principles"
#define MOTIF_FILE      "motif"
#define READ            0
#define WRITE           1
#endif
/*  Global Variables  */
Display *display;
char         filename [256];
Widget       TextWin;
Widget       LabelW;
XtAppContext AppContext;
int          perr[2], p;





/*-------------------------------------------------------------*
 |                    ExtractNormalString                      |
 | support routine to get normal string from XmString          |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
char *ExtractNormalString(XmString cs)
#else
char *ExtractNormalString(cs)
     XmString cs;
#endif
{
  XmStringContext    context;
  XmStringCharSet    charset;
  XmStringDirection  direction;
  Boolean            separator;
  static char       *primitiveString;


  XmStringInitContext (&context, cs);
  XmStringGetNextSegment (context, &primitiveString, &charset,
			  &direction, &separator);
  XmStringFreeContext (context);

  return ((char *) primitiveString);
}


/*-------------------------------------------------------------*
 |                     FontSelectApply                         |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void FontSelectApply (Widget w, XtPointer client_data, XtPointer call_data)
#else
void FontSelectApply (w, client_data, call_data)
     Widget    w;
     XtPointer client_data;
     XtPointer call_data;
#endif
{
  XmSelectionBoxCallbackStruct *cdata = (XmSelectionBoxCallbackStruct *)call_data;
  Widget       textWidget = (Widget)client_data;
  char        *textstr;
  XmFontList   fontList; 
  XFontStruct *mfinfo;


  /* no font selected... */
  if (cdata->value == NULL) return;

  textstr = ExtractNormalString (cdata->value);

  if ((mfinfo = XLoadQueryFont(display, textstr))==NULL)
      printf ("couldn't open %s font\n", textstr);
  fontList = XmFontListCreate (mfinfo, XmFONTLIST_DEFAULT_TAG);

  XtVaSetValues(textWidget, XmNfontList,  fontList, NULL);
}


/*-------------------------------------------------------------*
 |                       FontSelectOK                          |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void FontSelectOK (Widget w, XtPointer client_data, XtPointer call_data)
#else
void FontSelectOK (w, client_data, call_data)
     Widget    w;
     XtPointer client_data;
     XtPointer call_data;
#endif
{
  XmSelectionBoxCallbackStruct *callback_data =
    (XmSelectionBoxCallbackStruct *)call_data;
  char        *textstr;
  XmFontList   fontList;
  XFontStruct *mfinfo;

  if (callback_data->value == (XmString) NULL)
    return;

  textstr = ExtractNormalString (callback_data->value);

  if ((mfinfo = XLoadQueryFont(display, textstr))==NULL)
      printf ("couldn't open %s font\n", textstr);
  fontList = XmFontListCreate (mfinfo, XmFONTLIST_DEFAULT_TAG);

  XtVaSetValues(TextWin, XmNfontList, fontList, NULL);
}



/*-------------------------------------------------------------*
 |                        FontTest                             |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void FontTest (Widget w, XtPointer client_data, XtPointer call_data)
#else
void FontTest (w, client_data, call_data)
     Widget    w;
     XtPointer client_data;
     XtPointer call_data;
#endif
{
  XmSelectionBoxCallbackStruct *callback_data = 
    (XmSelectionBoxCallbackStruct *)call_data;
  Widget       txtWidget = (Widget)client_data;
  char        *textstr = DEFAULT_FONT;
  XmFontList   fontList; 
  XFontStruct *mfinfo;


  if (callback_data->value == (XmString) NULL)
    return;

  textstr = ExtractNormalString (callback_data->value);
  if (textstr == NULL) sprintf(textstr, "%s", DEFAULT_FONT);

  if ((mfinfo = XLoadQueryFont(display, textstr))==NULL)
      printf ("couldn't open %s font\n", textstr);
  fontList = XmFontListCreate (mfinfo, " ");

  XtVaSetValues(txtWidget,
		XmNfontList,  fontList,
		XmNvalue,     TEST_HELP,
		NULL);
}

/*-------------------------------------------------------------*
 |                        NextCap                              |
 *-------------------------------------------------------------*/
  
#ifndef _NO_PROTO
char *NextCap (char *path, char *cp, int len)
#else
char *NextCap (path, cp, len)
     char *path;
     char *cp;
     int len;
#endif
{
  static int  finish = 0;
  int         span;
  char       *ep, *np;
   
  
  if (!finish)
    return(NULL);
  
  if (ep = strchr(cp, ':'))
    span = ep - cp;
  else
    {
      finish++;
      ep = strchr(cp, '\0');
      span = ep - cp;
    };
  
  np = malloc(span + len + 2);
  strncpy(np, cp, span);

  np[span] = '/';
  np[span+1] = '\0';

  return(np);
}


/*-------------------------------------------------------------*
 |                        file_exist                           |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
int file_exist (char *fullname)
#else
int file_exist (fullname)
     char *fullname;
#endif
{
  if (fopen(fullname,"r"))
    return(1);
  else
    return(0);
}  


/*-------------------------------------------------------------*
 |                   search_in_env                             |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
char *search_in_env (char *filename)
#else
char *search_in_env (filename)
     char *filename;
#endif
{
  int   i, len;
  char *envpath, *prefix, *cp;


  len = strlen(filename);
  if (envpath = getenv("PATH"))
    {
      cp  = envpath;
      cp += 2; 
      
      while (prefix = NextCap(envpath, cp, len))
	{
	  cp += strlen(prefix);
	  strncat(prefix, filename, len);

	  if (file_exist(prefix))
	    return(prefix);
	  
	  free(prefix);
        }  
   
    }

  return(NULL);
}

/*-------------------------------------------------------------*
 |                        GetSource                            |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
char *GetSource (char *fileptr)
#else
char *GetSource (fileptr)
     char *fileptr;
#endif
{
  static char *retbuff;
  int          fd, flen, catlen;
  char        *capfileptr, *defaultcap, *datahome;
  
  
  if ((fd = open (fileptr, O_RDONLY)) < 0)
    {
      if (defaultcap = getenv("MOTIFSHELLFILES"))
	{
	  catlen = strlen(defaultcap);
	  datahome = (char *) malloc(catlen + strlen(fileptr) + 2);
	  strncpy(datahome, defaultcap, catlen);
	  datahome[catlen] = '/';
	  datahome[catlen + 1] = '\0';
	  strcat(datahome, fileptr);

	  if ((fd = open(datahome, O_RDONLY)) < 0)
	    {
	      printf ("Cannot find the sourch file %s in %s\n", fileptr, datahome);
	      free(datahome);
	      return((char *) NULL);
	    }
	  free(datahome);
	}
      else
	if (capfileptr = search_in_env(fileptr))
	  {
	    fd = open (capfileptr, O_RDONLY);
	    free(capfileptr);
	  }
	else
	  {
	    printf ("Cannot find the sourch file %s\n", fileptr);
	    printf ("Please setup MOTIFSHELLFILES entry in environment and put data files there.\n");
	    return ((char *) NULL);
	  }
    }

  flen = GetFileLen(fd);
  retbuff = (char*) calloc (1, flen + 1);

  if (read (fd, retbuff, flen) <= 0)
    {
      printf ("Error reading file %s\n", fileptr);
      return ((char *) NULL);
    }
  close (fd);

  return (retbuff);
}


/*-------------------------------------------------------------*
 |                          DoTheFont                          |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
XmString *DoTheFont (int *count)
#else
XmString *DoTheFont (count)
     int *count;
#endif
{
  char **fontlist;
  int i;
  static XmString *addrstr;


  fontlist = XListFonts (display, "*", 1200, count);
  addrstr  = (XmString *) XtCalloc (*count, sizeof (XmString));
  for (i = 0; i < *count; i++) addrstr[i] = XmStringCreateLocalized(fontlist[i]);

  return (addrstr);
}


/*-------------------------------------------------------------*
 |                       PopupHelpShell                        |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void PopupHelpShell (Widget w, XtPointer client_data, XtPointer call_data)
#else
void PopupHelpShell (w, client_data, call_data)
     Widget    w;
     XtPointer client_data;
     XtPointer call_data;
#endif
{
  Widget helpText = (Widget)client_data;
  char *buffer;


  buffer = GetSource (HELP_FILE);
  XmTextSetString (helpText, buffer);
}


/*-------------------------------------------------------------*
 |                         SetLabelStr                         |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void SetLabelStr (XmString tcs)
#else
void SetLabelStr (tcs)
     XmString tcs;
#endif
{
  XtVaSetValues(LabelW, XmNlabelString, tcs, NULL);
}


/*-------------------------------------------------------------*
 |                          SetLabel                           |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void SetLabel (char *label)
#else
void SetLabel (label)
     char *label;
#endif
{
  XmString      tcs;

  tcs = XmStringCreateLocalized(label);
  SetLabelStr(tcs);
  XmStringFree(tcs);
}

/*-------------------------------------------------------------*
 |                    ShowFontDialogShell                      |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void ShowFontDialogShell (Widget parent, char *label)
#else
void ShowFontDialogShell (parent, label)
     Widget    parent;
     char     *label;
#endif
{
  Widget         workText, list;
  static Widget  dlog = NULL;
  int            count;
  XmString      *fonts;


  if (dlog == NULL)
    {
      fonts = DoTheFont(&count);

      dlog = XmCreateSelectionDialog(parent, label, NULL, 0);

      workText = XtVaCreateManagedWidget ("workText", xmTextWidgetClass, dlog,
					  XmNeditable,     True,
					  XmNeditMode,     XmMULTI_LINE_EDIT,
					  XmNcolumns,      30,
					  XmNrows,         3,
					  XmNresizeHeight, False,
					  XmNwordWrap,     True,
					  XmNvalue,        TEST_STRING,
					  NULL);
      XtVaSetValues(dlog,
		    XmNwidth,                450,
		    XmNheight,               450,
		    XmNallowShellResize,     True,
		    XmNlistItems,            fonts,
		    XmNlistItemCount,        count,
		    XmNlistVisibleItemCount, 10,
		    NULL);

      list = XmSelectionBoxGetChild(dlog, XmDIALOG_LIST);
      XtVaSetValues(list, XmNselectionPolicy, XmSINGLE_SELECT, NULL);

      XtUnmanageChild(XmSelectionBoxGetChild(dlog, XmDIALOG_APPLY_BUTTON));

      XtAddCallback (dlog, XmNokCallback,     FontSelectOK,     NULL);
      XtAddCallback (dlog, XmNhelpCallback,   FontTest,         (XtPointer)workText);
      XtAddCallback (list, XmNsingleSelectionCallback, FontSelectApply,
		     (XtPointer)workText);
    }

  XtManageChild(dlog);
}




/*-------------------------------------------------------------*
 |                   CreateHelpDialogShell                     |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void CreateHelpDialogShell (Widget parent, char *say)
#else
void CreateHelpDialogShell (parent, say)
     Widget    parent;
     char     *say;
#endif
{
  static Widget dlog = NULL;
  Widget helpText, button;

  if (dlog == NULL)
    {
      dlog     = XmCreateFormDialog(parent, "Help Window", NULL, 0);
      helpText = XtVaCreateManagedWidget("helpText", xmTextWidgetClass, dlog,
					 XmNeditable,         False,
					 XmNeditMode,         XmMULTI_LINE_EDIT,
					 XmNcolumns,          50,
					 XmNresizeHeight,     True,
					 XmNwordWrap,         True,
					 XmNleftAttachment,   XmATTACH_FORM,
					 XmNtopAttachment,    XmATTACH_FORM,
					 XmNrightAttachment,  XmATTACH_FORM,
					 XmNbottomAttachment, XmATTACH_FORM,
					 XmNbottomOffset,     30,
					 NULL);
      button   = XtVaCreateManagedWidget("dismiss", xmPushButtonWidgetClass, dlog,
					 XmNleftAttachment,   XmATTACH_FORM,
					 XmNtopAttachment,    XmATTACH_WIDGET,
					 XmNtopWidget,        helpText,
					 XmNrightAttachment,  XmATTACH_FORM,
					 XmNbottomAttachment, XmATTACH_FORM,
					 NULL);
      XtAddCallback (XtParent(dlog), XmNpopupCallback, PopupHelpShell,
		     (XtPointer)helpText);
    }

  XtManageChild(dlog);
}



/*-------------------------------------------------------------*
 |                         CreateTextWin                       |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget CreateTextWin (Widget parent)
#else
Widget CreateTextWin (parent)
     Widget parent;
#endif
{
  Widget    stext;
  char     *buffer;
  XmString  tcs;


  tcs    = XmStringCreateLocalized ("Welcome to Motif");
  LabelW = XtVaCreateManagedWidget("Label", xmLabelWidgetClass, parent,
				   XmNleftAttachment,  XmATTACH_FORM,
				   XmNtopAttachment,   XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNlabelString,     tcs,
				   XmNmarginHeight,    5,
				   XmNshadowThickness, 1,
				   NULL);
  XmStringFree (tcs);

  stext  = XmCreateScrolledText(parent, "s_text", NULL, 0);
  XtVaSetValues(stext,
		XmNeditMode,         XmMULTI_LINE_EDIT,
		XmNeditable,         False,
		XmNcolumns,          80,
		XmNrows,             30,
		NULL);
  XtVaSetValues(XtParent(stext),
		XmNleftAttachment,   XmATTACH_FORM,
		XmNtopAttachment,    XmATTACH_WIDGET,
		XmNrightAttachment,  XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNtopWidget,        LabelW,
		XmNleftOffset,       5,
		XmNrightOffset,      5,
		XmNbottomOffset,     5,
		XmNheight,           550,
		XmNwidth,            500,
		XmNresizeWidth,      True,
		XmNresizeHeight,     False,
		NULL);
  XtManageChild(stext);

  buffer = GetSource (WELCOME_FILE);
  XmTextSetString (stext, buffer);

  return(stext);
}


/*-------------------------------------------------------------*
 |                         GetFileLen                          |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
int GetFileLen (int fd)
#else
int GetFileLen (fd)
     int fd;
#endif
{
  static int retval;

#ifdef VMS
  lseek (fd, 0, 0);  
  retval = lseek (fd, 0, 2);
  lseek (fd, 0, 0);  
#else
  lseek (fd, 0, L_SET);  
  retval = lseek (fd, 0, L_XTND);
  lseek (fd, 0, L_SET);  
#endif

  return (retval);
}


/*-------------------------------------------------------------*
 |                         SysCall                             |
 *-------------------------------------------------------------*/
#ifndef VMS
#ifndef _NO_PROTO
char *SysCall (Widget widget, char *systemCommand)
#else
char *SysCall (widget, systemCommand)
     Widget  widget;
     char   *systemCommand;
#endif
{
  char  str[256];
  char *findCmd;
  FILE *file;
  pid_t p;


  if ((p = fork()) == 0)
    {
      /* note - execlp uses PATH */
      execlp(systemCommand, systemCommand, NULL);

      /* if we fail to find the systemCommand, use 'find' to look for it. */
      fprintf(stderr, "can't find %s\n", systemCommand);

      findCmd = (char *) XtMalloc(strlen(systemCommand) + 24);
      sprintf(findCmd, "find .. -name '%s' -print", systemCommand);
      file = popen(findCmd, "r");
      XtFree(findCmd);

      while(fgets(str, 80, file) != NULL)
	{
	  str[strlen(str)-1] = '\0';
	  printf("Trying: %s\n", str);
	  execl(str, systemCommand, NULL);
	  printf("Still can't find %s...\n", systemCommand);
	}
      printf("I give up!\n");
      exit(0);
    }
}
#endif

/*-------------------------------------------------------------*
 |                           Quit                              |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void Quit(int i)
#else
void Quit(i)
     int i;
#endif
{
  printf("Bye!\n");
  exit(0);
}


/*-------------------------------------------------------------*
 |                          Menu1CB                            |
 | File  pulldown menu items.                                  |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void Menu1CB (Widget w, XtPointer clientData, XtPointer callData)
#else
void Menu1CB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int   itemNo = (int)clientData;

  switch (itemNo)
    {
    case 0: Quit(0);  break;  /* Quit */
    }
}


/*-------------------------------------------------------------*
 |                          Menu2CB                            |
 | OSF Happenings  pulldown menu items.                        |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void Menu2CB (Widget w, XtPointer clientData, XtPointer callData)
#else
void Menu2CB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int        itemNo = (int)clientData;
  char      *buffer;
  XmString   labelStr;


  XtVaGetValues(w, XmNlabelString, &labelStr, NULL);
  SetLabelStr (labelStr);

  switch (itemNo)
    {
    case 0: buffer = GetSource (MEMBERSHIP_FILE);  break;
    case 1: buffer = GetSource (RESEARCH_FILE);    break;
    case 2: buffer = GetSource (PRINCIPLES_FILE);  break;
    case 3: buffer = GetSource (MOTIF_FILE);       break;
    }
  XmTextSetString (TextWin, buffer); 
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


/*-------------------------------------------------------------*
 |                          Menu3CB                            |
 | Demos  pulldown menu items.                                 |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void Menu3CB (Widget w, XtPointer clientData, XtPointer callData)
#else
void Menu3CB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int      itemNo = (int)clientData;
  XmString labelStr;


  XtVaGetValues(w, XmNlabelString, &labelStr, NULL);
  SetLabelStr (labelStr);

  switch (itemNo)
    {
#ifdef VMS
    case 0: vms_spawn("@motifgif",True); break;
    case 1: vms_spawn("@periodic",True); break;
    case 2: vms_spawn("@motifbur",True); break;
#else
    case 0: SysCall (w, "motifgif");  break;
    case 1: SysCall (w, "periodic");  break;
    case 2: SysCall (w, "motifbur");  break;
#endif 
   }
}


/*-------------------------------------------------------------*
 |                          Menu4CB                            |
 | Unix Commands  pulldown menu items.                         |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void Menu4CB (Widget w, XtPointer clientData, XtPointer callData)
#else
void Menu4CB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int       itemNo = (int)clientData;
  char     *buffer;
  XmString  labelStr;

  switch (itemNo)
    {
#ifdef VMS
    case 0: vms_spawn("create/terminal/detached",True); break;
#else
    case 0: SysCall (w, "xterm");  break;  /* Terminal */
#endif
    case 1:                                /* File Listing */
      {
	XtVaGetValues(w, XmNlabelString, &labelStr, NULL);
	SetLabelStr (labelStr);
#ifdef VMS
	system ("dir/prot/owner/date/size/output=foo.txt");
	system ("convert/fdl=stream foo.txt foo.txt");
	buffer = GetSource ("foo.txt");
	XmTextSetString (TextWin, buffer);
	system ("delete/nolog foo.txt;*");
#else
	system ("ls -al > foo");
	buffer = GetSource ("foo");
	XmTextSetString (TextWin, buffer);
	system ("rm -r foo");
#endif
	break;
      }
    case 2:                                /* Process Status */
      {
	XtVaGetValues(w, XmNlabelString, &labelStr, NULL);
	SetLabelStr (labelStr);
#ifdef VMS
	system ("show system /output=foo.txt");
	system ("convert/fdl=stream foo.txt foo.txt");
	buffer = GetSource ("foo.txt");
	XmTextSetString (TextWin, buffer);
	system ("delete/nolog foo.txt;*");
#else
	system ("ps -a > foo");
	buffer = GetSource ("foo");
	XmTextSetString (TextWin, buffer);
	system ("rm -r foo");
#endif
	break;
      }
    case 3:                                /* Show Source */
      {
	XtVaGetValues(w, XmNlabelString, &labelStr, NULL);
	SetLabelStr (labelStr);
	buffer = GetSource (filename);
	XtVaSetValues(TextWin, XmNvalue, buffer, NULL);
	break;
      }
    }
}


/*-------------------------------------------------------------*
 |                          Menu5CB                            |
 | X Programs  pulldown menu items.                            |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void Menu5CB (Widget w, XtPointer clientData, XtPointer callData)
#else
void Menu5CB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int itemNo = (int)clientData;

  switch (itemNo)
    {
#ifdef VMS
    case 0: vms_spawn("run sys$system:decw$clock",True);    break;
    case 1: vms_spawn("run sys$system:decw$banner",True);   break;
    case 2: vms_spawn ("run decw$examples:ico.exe",True); break;
#else
    case 0: SysCall (w, "xclock");  break;
    case 1: SysCall (w, "xload");   break;
    case 2: SysCall (w, "kaleid");  break;
#endif
    }
}


/*-------------------------------------------------------------*
 |                          Menu6CB                            |
 | Font  pulldown menu items.                                  |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void Menu6CB (Widget w, XtPointer clientData, XtPointer callData)
#else
void Menu6CB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int itemNo = (int)clientData;

  switch (itemNo)
    {
    case 0: /* Load... */
      {
	ShowFontDialogShell (XtParent(w), "List O' Fonts");
	break;
      }
    }
}


/*-------------------------------------------------------------*
 |                          Menu7CB                            |
 | Help  pulldown menu items.                                  |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
void Menu7CB (Widget w, XtPointer clientData, XtPointer callData)
#else
void Menu7CB (w, clientData, callData)
     Widget w;
     XtPointer clientData;
     XtPointer callData;
#endif
{
  int itemNo = (int)clientData;
  

  switch (itemNo)
    {
    case 0: /* Are you sure? */
      {
	CreateHelpDialogShell (XtParent(w), "Help Window");
	break;
      }
    }
}


#ifdef DEC_MOTIF_BUG_FIX
static  char     *menuString[] = {
    "File", "OSF Happenings", "Demos", "O/S Commands", "X Programs", "Font", "Help"
    };
#else
static  char     *menuString[] = {
    "File", "OSF Happenings", "Demos", "Unix Commands", "X Programs", "Font", "Help"
    };
#endif
static   char     *subString[][XtNumber(menuString)] = {
    { "Quit" },
    { "OSF Membership", "OSF's Research Institute",  "OSF's Principles", "OSF/Motif" },
    { "Pictures", "Periodic Table", "Motif Burger" },
    { "Terminal", "File Listing", "Process Status", "Show Source" },
    { "Xclock", "Xload", "Kaleidescope"},
    { "Load..." },
    { "Are you sure ?" }
  };


/*-------------------------------------------------------------*
 |                         CreateMenuBar                       |
 *-------------------------------------------------------------*/
#ifndef _NO_PROTO
Widget CreateMenuBar (Widget parent)
#else
Widget CreateMenuBar (parent)
     Widget parent;
#endif
{
  int       i;
  XmString  s[10];
  Widget    menuBar, helpCascade;

  menuBar = XmVaCreateSimpleMenuBar(parent, "MenuBar",
	XmVaCASCADEBUTTON, s[0] = XmStringCreateSimple(menuString[0]), menuString[0][0],
	XmVaCASCADEBUTTON, s[1] = XmStringCreateSimple(menuString[1]), menuString[1][0],
	XmVaCASCADEBUTTON, s[2] = XmStringCreateSimple(menuString[2]), menuString[2][0],
	XmVaCASCADEBUTTON, s[3] = XmStringCreateSimple(menuString[3]), menuString[3][0],
	XmVaCASCADEBUTTON, s[4] = XmStringCreateSimple(menuString[4]), menuString[4][0],
	XmVaCASCADEBUTTON, s[5] = XmStringCreateSimple(menuString[5]), menuString[5][0],
	XmVaCASCADEBUTTON, s[6] = XmStringCreateSimple(menuString[6]), menuString[6][0],
	NULL);
  for (i=0; i<=6; i++) XmStringFree(s[i]);

  XmVaCreateSimplePulldownMenu(menuBar, menuString[0], 0, Menu1CB,   /* File */
	XmVaPUSHBUTTON, s[0] = XmStringCreateSimple(subString[0][0]), NULL, NULL, NULL,
	NULL);
  for (i=0; i<1; i++) XmStringFree(s[i]);

  XmVaCreateSimplePulldownMenu(menuBar, menuString[1], 1, Menu2CB,   /* OSF Happenings */
	XmVaPUSHBUTTON, s[0] = XmStringCreateSimple(subString[1][0]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[1] = XmStringCreateSimple(subString[1][1]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[2] = XmStringCreateSimple(subString[1][2]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[3] = XmStringCreateSimple(subString[1][3]), NULL, NULL, NULL,
	NULL);
  for (i=0; i<=3; i++) XmStringFree(s[i]);

  XmVaCreateSimplePulldownMenu(menuBar, menuString[2], 2, Menu3CB,     /* Demos */
	XmVaPUSHBUTTON, s[0] = XmStringCreateSimple(subString[2][0]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[1] = XmStringCreateSimple(subString[2][1]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[2] = XmStringCreateSimple(subString[2][2]), NULL, NULL, NULL,
	NULL);
  for (i=0; i<=2; i++) XmStringFree(s[i]);

  XmVaCreateSimplePulldownMenu(menuBar, menuString[3], 3, Menu4CB,   /* Unix Commands */
	XmVaPUSHBUTTON, s[0] = XmStringCreateSimple(subString[3][0]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[1] = XmStringCreateSimple(subString[3][1]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[2] = XmStringCreateSimple(subString[3][2]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[3] = XmStringCreateSimple(subString[3][3]), NULL, NULL, NULL,
	NULL);
  for (i=0; i<=3; i++) XmStringFree(s[i]);

  XmVaCreateSimplePulldownMenu(menuBar, menuString[4], 4, Menu5CB,   /* X Programs */
	XmVaPUSHBUTTON, s[0] = XmStringCreateSimple(subString[4][0]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[1] = XmStringCreateSimple(subString[4][1]), NULL, NULL, NULL,
	XmVaPUSHBUTTON, s[2] = XmStringCreateSimple(subString[4][2]), NULL, NULL, NULL,
	NULL);
  for (i=0; i<=2; i++) XmStringFree(s[i]);

  XmVaCreateSimplePulldownMenu(menuBar, menuString[5], 5, Menu6CB,   /* Font */
	XmVaPUSHBUTTON, s[0] = XmStringCreateSimple(subString[5][0]), NULL, NULL, NULL,
	NULL);
  for (i=0; i<=0; i++) XmStringFree(s[i]);

  XmVaCreateSimplePulldownMenu(menuBar, menuString[6], 6, Menu7CB,     /* Help */
	XmVaPUSHBUTTON, s[0] = XmStringCreateSimple(subString[6][0]), NULL, NULL, NULL,
	NULL);
  for (i=0; i<=0; i++) XmStringFree(s[i]);
  XtVaSetValues(menuBar, XmNmenuHelpWidget, XtNameToWidget(menuBar, "button_6"), NULL);


  XtManageChild(menuBar);

  return (menuBar);
}



/*-------------------------------------------------------------*
 |                            main                             |
 *-------------------------------------------------------------*/

#ifndef _NO_PROTO
int main (int argc, char **argv)
#else
int main (argc, argv)
     int    argc;
     char **argv;
#endif
{
  Widget shell, mainWindow, workRegion, menuBar;


  signal(SIGHUP,  Quit);
  signal(SIGINT,  Quit);
  signal(SIGQUIT, Quit);

#ifdef VMS
    delete("foo.txt;*");
#else
 system ("touch foo");
 system ("rm -r foo");
#endif

  XtToolkitInitialize();
  AppContext = XtCreateApplicationContext();
  if ((display = XtOpenDisplay (AppContext, NULL, argv[0], APP_CLASS,
				NULL, 0, &argc, argv)) == NULL)
    {
      fprintf (stderr,"\n%s:  Can't open display\n", argv[0]);
      exit(1);
    }

  /* capture the source-code file name for use later. */
  sprintf(filename, "%s.c", argv[0]);

  XmRepTypeInstallTearOffModelConverter();

  shell = XtVaAppCreateShell(argv[0], APP_CLASS, applicationShellWidgetClass,
			     display, XmNallowShellResize, True, NULL);
			     

  mainWindow = XtVaCreateManagedWidget("mainWindow", xmMainWindowWidgetClass, shell,
				       XmNmarginWidth,  2,
				       XmNmarginHeight, 2, NULL);
  workRegion = XtVaCreateManagedWidget("s_text", xmFormWidgetClass, mainWindow, NULL);

  menuBar = CreateMenuBar (mainWindow);
  TextWin = CreateTextWin (workRegion);
  XmMainWindowSetAreas(mainWindow, menuBar, NULL, NULL, NULL, workRegion);


  XtRealizeWidget(shell);

  XtAppMainLoop(AppContext);
}
