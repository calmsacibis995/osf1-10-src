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
static char *rcsid = "@(#)$RCSfile: AUDModSel.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/12/20 21:31:47 $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Mrm/MrmAppl.h>
#include <Xm/Text.h>
#include <X11/X.h>

#include "TextList.h"
#include "SListTB.h"

#include "XIsso.h"
#include "Utilities.h"
#include "AUD.h"

#define MAIN_WIDGET	"AUDModSel1FRM"
#define MAIN2_WIDGET	"AUDModSel2FRM"

#define ALL	0
#define SECOND	1

/* Mask parsing macro */
#define lowbit(x) ((x) & (~(x) + 1))
  
/* forward declarations */
void AUDModSelDoItCallback();
void AUDFreeSelectionData();
void AUDCvtSelDataToWidget();

/* global variables */

/* widget for screen2 widgets to use since not child of main */
static Widget	mainWidget = (Widget) NULL;

/* data for OKToChange */
static AUDLoseChangeData changeData;

/* To hold previous "good" screen 2 data */
AUDModSelData	screen2Data;

/* current file under edit */
char *currentSelFile = (char *) NULL;

/* state variables for screens */
Boolean form1Changed = False;
Boolean form2Managed = False;
Boolean form2Changed = False;
static Boolean addingNew    = False;

/* external (eventually) variables */
Boolean	changedSelList = False;
Boolean changedBaseEvents;
Boolean changedSiteEvents;
Boolean changedAliasEvents;

/*==================<UI-Level Support Routines/Callbacks>================*/

Boolean
notBlanks(s)
char	*s;
{
    int len;
    int i;
    

    if(s != (char *) NULL)
	len = strlen(s);
    else
	len = 0;


    i = 0;
    while(i < len)
    {
	if(s[i] != ' ')
	    return True;
	else
	    i++;
    }
    return False;
}





void
AUDFreeString(str)
char **str;
{
    if(*str != (char *) NULL)
    {
	XtFree(*str);
	*str = (char *) NULL;
    }
}



void
AUDFreeEvent(ev)
AUDeventPtr	*ev;
{
    if(*ev != (AUDeventPtr) NULL)
    {
	AUDFreeString(&(*ev)->eventName);
	AUDFreeString(&(*ev)->subEvent);
    }

    XtFree((char *)*ev);
    *ev = (AUDeventPtr) NULL;
}



void
AUDFreeSelectionData( selData )
AUDModSelDataPtr	selData;
{
    int	i;

    if( selData )
    {
	for( i = 0; i < AUDIT_MAX_ENTRIES; i++ )
	{
	    AUDFreeString(&selData->hostName[i]);
	    AUDFreeString(&selData->auid[i]);
	    AUDFreeString(&selData->ruid[i]);
	    AUDFreeString(&selData->euid[i]);
	    AUDFreeString(&selData->userName[i]);
	    AUDFreeString(&selData->pid[i]);
	    AUDFreeString(&selData->ppid[i]);
	    AUDFreeString(&selData->devNo[i]);
	    AUDFreeString(&selData->vnodeID[i]);
	    AUDFreeString(&selData->vnodeNo[i]);
	    AUDFreeString(&selData->string[i]);
	    AUDFreeString(&selData->error[i]);
	    AUDFreeString(&selData->procName[i]);
	}

	/* events */
	i = 0;
	while( i < selData->numEvents )
	{
	    AUDFreeEvent(&selData->events[i]);
	    i++;
	}
	selData->numEvents = 0;
	selData->mask = 0;

	bzero(selData->startTime, sizeof(selData->startTime));
	bzero(selData->stopTime, sizeof(selData->stopTime));
    }

} /* end AUDFreeSelectionData() */





void
AUDLoadTokenStringFromWidget(w,
			     mask,
			     maskValue,
			     delimiters,
			     data)
Widget	w;
int	*mask;
int	maskValue;
char	*delimiters;
char	**data;
{
    char *buf;
    char *token;
    int	 count;

    buf = XmTextGetString(w);

    if(buf != (char *) NULL &&
       strlen(buf) != 0)
    {
	*mask |= maskValue;

	/* parse entries */
	token = strtok(buf, delimiters);

	if(token)
	{
	    data[0] = (char *) XtMalloc(strlen(token) + 1);
	    strcpy(data[0], token);


	    count = 1;
	    while ((count < AUDIT_MAX_ENTRIES) &&
		   (token = strtok((char *) 0, delimiters)) != (char *) 0)
	    {
		data[count] = (char *) XtMalloc(strlen(token) + 1);
		strcpy(data[count], token);
		count++;
	    }
	}
	XtFree(buf);
    }
}






void
AUDSaveScreen2Data(selUI)
AUDModSelUIPtr selUI;
{
    char	*delim 	     = ", ";
    char	*devNoDelim  = " ";
    char	*stringDelim = "\n";
    
    if( selUI )
    {
	/* make a sparse copy of the data, pertaining ONLY to 
	 * the information on screen 2.
	 */
	bzero( (char *)&screen2Data, (int)sizeof(AUDModSelData) );
	
	screen2Data.mask = 0;

	/* Device Numbers */
	AUDLoadTokenStringFromWidget(selUI->devNoTF_W,
				     &screen2Data.mask,
				     AUDMODSEL_DEVNO_MASK,
				     devNoDelim,
				     screen2Data.devNo);

	/* Vnode Device Numbers */
	AUDLoadTokenStringFromWidget(selUI->vnodeNoTF_W,
				     &screen2Data.mask,
				     AUDMODSEL_VNODENO_MASK,
				     devNoDelim,
				     screen2Data.vnodeNo);

	/* Vnode IDs */
	AUDLoadTokenStringFromWidget(selUI->vnodeIDTF_W,
				     &screen2Data.mask,
				     AUDMODSEL_VNODEID_MASK,
				     delim,
				     screen2Data.vnodeID);

	/* ProcName */
	AUDLoadTokenStringFromWidget(selUI->procNameTF_W,
				     &screen2Data.mask,
				     AUDMODSEL_PROCNAME_MASK,
				     delim,
				     screen2Data.procName);

	/* Strings */
	AUDLoadTokenStringFromWidget(selUI->stringTF_W,
				     &screen2Data.mask,
				     AUDMODSEL_STRING_MASK,
				     stringDelim,
				     screen2Data.string);

	/* Errors */
	AUDLoadTokenStringFromWidget(selUI->errorTF_W,
				     &screen2Data.mask,
				     AUDMODSEL_ERROR_MASK,
				     delim,
				     screen2Data.error);
    }
}




void
AUDRestoreScreen2Data(selUI)
AUDModSelUIPtr		selUI;
{
    char 	all[1024];
    int	 	i;
    Boolean	savedState;

    /* save the state of form2 because this routine will cause the
     * the valueChanged callbacks for all widgets on screen 2 to be
     * invoked.  We will restore it at the end.
     */
    savedState = form2Changed;

    /* Device Numbers */
    if( screen2Data.mask & AUDMODSEL_DEVNO_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      screen2Data.devNo[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, screen2Data.devNo[i]) :
		       strcat(all, screen2Data.devNo[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->devNoTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->devNoTF_W,
			"");
    }

    /* Vnode IDS */
    if( screen2Data.mask & AUDMODSEL_VNODEID_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      screen2Data.vnodeID[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, screen2Data.vnodeID[i]) :
		       strcat(all, screen2Data.vnodeID[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->vnodeIDTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->vnodeIDTF_W,
			"");
    }

    /* Vnode Device Numbers */
    if( screen2Data.mask & AUDMODSEL_VNODENO_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      screen2Data.vnodeNo[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, screen2Data.vnodeNo[i]) :
		       strcat(all, screen2Data.vnodeNo[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->vnodeNoTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->vnodeNoTF_W,
			"");
    }

    /* Process Names */
    if( screen2Data.mask & AUDMODSEL_PROCNAME_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      screen2Data.procName[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, screen2Data.procName[i]) :
		       strcat(all, screen2Data.procName[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->procNameTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->procNameTF_W,
			"");
    }

    /* Strings */
    if( screen2Data.mask & AUDMODSEL_STRING_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      screen2Data.string[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, screen2Data.string[i]) :
		       strcat(all, screen2Data.string[i]);
	    strcat(all, "\n");
	    i++;
	}

	XmTextSetString(selUI->stringTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->stringTF_W,
			"");
    }

    /* Errors */
    if( screen2Data.mask & AUDMODSEL_ERROR_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      screen2Data.error[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, screen2Data.error[i]) :
		       strcat(all, screen2Data.error[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->errorTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->errorTF_W,
			"");
    }

    /* restore the true condition of form2 */
    form2Changed = savedState;
}

void
AUDFreeScreen2Data()
{
    int i;

    for( i = 0; i < AUDIT_MAX_ENTRIES; i++ )
    {
	AUDFreeString(&screen2Data.devNo[i]);
	AUDFreeString(&screen2Data.vnodeID[i]);
	AUDFreeString(&screen2Data.vnodeNo[i]);
	AUDFreeString(&screen2Data.string[i]);
	AUDFreeString(&screen2Data.error[i]);
	AUDFreeString(&screen2Data.procName[i]);
    }

    screen2Data.mask = 0;
}





void
AUDRestoreCorrectSelFileName(w, data, info)
Widget 			w;
AUDLoseChangeDataPtr	data;
XmAnyCallbackStruct 	*info;
{
    AUDModSelPtr	pModSelData;
    Widget		ancestor;

    ancestor = search_for_parent(data->w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if(pModSelData)
	{
	    if( currentSelFile != (char *) NULL )
	    {
		XmTextSetString(pModSelData->ui.selectionFileTL_W, 
				currentSelFile);
	    }
	    else
		XmTextSetString(pModSelData->ui.selectionFileTL_W, "");
	}
    }
}



void
AUDSetLabel(w, s)
Widget	w;
char	*s;
{
    XmString	xmstring;
    Arg		args[1];
    Cardinal	n;

    if( s != (char *) NULL )
    {
	xmstring = XmStringCreate(s, charset);
    }
    else
    {
	xmstring = XmStringCreate("                           ", charset);
    }
    
    n = 0;
    XtSetArg(args[n], XmNlabelString, xmstring); n++;
    XtSetValues(w, args, n);
    XmStringFree(xmstring);
}




static void
AUDModSelClearFields(pData, clearTL)
AUDModSelUIPtr	pData;
Boolean clearTL;
{
    if( pData )
    {
	/* Text Fields */
	if( pData->startTimeTF_W )
	    XmTextSetString(pData->startTimeTF_W, "");
	if( pData->stopTimeTF_W )
	    XmTextSetString(pData->stopTimeTF_W, "");
	if( clearTL && pData->selectionFileTL_W )
	    XmTextSetString(pData->selectionFileTL_W, "");
	if( pData->pidTF_W )
	    XmTextSetString(pData->pidTF_W, "");
	if( pData->ppidTF_W )
	    XmTextSetString(pData->ppidTF_W, "");
	if( pData->auidTF_W )
	    XmTextSetString(pData->auidTF_W, "");
	if( pData->ruidTF_W )
	    XmTextSetString(pData->ruidTF_W, "");
	if( pData->euidTF_W )
	    XmTextSetString(pData->euidTF_W, "");
	if( pData->userNameTF_W )
	    XmTextSetString(pData->userNameTF_W, "");
	if( pData->hostNameTF_W )
	    XmTextSetString(pData->hostNameTF_W, "");
	if( pData->devNoTF_W )
	    XmTextSetString(pData->devNoTF_W, "");
	if( pData->vnodeIDTF_W )
	    XmTextSetString(pData->vnodeIDTF_W, "");
	if( pData->vnodeNoTF_W )
	    XmTextSetString(pData->vnodeNoTF_W, "");
	if( pData->procNameTF_W )
	    XmTextSetString(pData->procNameTF_W, "");
	if( pData->stringTF_W )
	    XmTextSetString(pData->stringTF_W, "");
	if( pData->errorTF_W )
	    XmTextSetString(pData->errorTF_W, "");

	/* Label for 2nd screen */
	if( pData->selNameLBL_W )
	{
	    AUDSetLabel(pData->selNameLBL_W, (char *) NULL);
	}

	/* Event Scroll Lists */
	if(pData->baseEventSB_W) 
	{
	    XmSelectListTBSetAllItemsSelected(pData->baseEventSB_W);
	    XmSelListTBSetAllTBs(pData->baseEventSB_W,  (int)True, (int)True);
	}

	if(pData->aliasEventSB_W) 
	{
	    XmSelectListTBSetAllItemsSelected(pData->aliasEventSB_W);
	    XmSelListTBSetAllTBs(pData->aliasEventSB_W, (int)True, (int)True);
	}
    }
}



void
AUDModSel1CreateFormCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDModSelPtr	pModSelData;

    pModSelData = (AUDModSelPtr) XtMalloc(sizeof(AUDModSel));

    if( pModSelData )
    {
	bzero( (char *)pModSelData, (int)sizeof(AUDModSel) );
	pModSelData->ui.form1_W = w;
	mainWidget = w;
	SetUserData( w, pModSelData );
    }
    else
    {
	MemoryError();
    }
}





void
AUDModSel2CreateFormCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDModSelPtr	pModSelData;

    GetUserData(mainWidget, pModSelData);

    if( pModSelData )
    {
	pModSelData->ui.form2_W = w;
    }

    /* Set this data for this screen too. */
    SetUserData( w, pModSelData );
    
}




void
AUDModSelCreateWidgetsCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{

    Widget 		ancestor;
    AUDModSelPtr	pModSelData;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if( !ancestor )
	    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if( pModSelData )
	{
	    switch (*flag)
	    {
	      case AUDMODSEL_STARTTIME:
		pModSelData->ui.startTimeTF_W = w;
		break;
	      case AUDMODSEL_STOPTIME:
		pModSelData->ui.stopTimeTF_W = w;
		break;
	      case AUDMODSEL_SELFILELIST:
		pModSelData->ui.selectionFileTL_W = w;
		break;
	      case AUDMODSEL_PID:
		pModSelData->ui.pidTF_W = w;
		break;
	      case AUDMODSEL_PPID:
		pModSelData->ui.ppidTF_W = w;
		break;
	      case AUDMODSEL_AUID:
		pModSelData->ui.auidTF_W = w;
		break;
	      case AUDMODSEL_RUID:
		pModSelData->ui.ruidTF_W = w;
		break;
	      case AUDMODSEL_EUID:
		pModSelData->ui.euidTF_W = w;
		break;
	      case AUDMODSEL_USERNAME:
		pModSelData->ui.userNameTF_W = w;
		break;
	      case AUDMODSEL_HOSTNAME:
		pModSelData->ui.hostNameTF_W = w;
		break;
	      case AUDMODSEL_BASEEVENT:
		pModSelData->ui.baseEventSB_W = w;
		XmSelListTBAddTabGroup(w);
		break;
	      case AUDMODSEL_ALIASEVENT:
		pModSelData->ui.aliasEventSB_W = w;
		XmSelListTBAddTabGroup(w);
		break;
	      case AUDMODSEL_ALL_EVENT_PB:
		pModSelData->ui.allEventsPB_W = w;
		break;
	      case AUDMODSEL_NO_EVENT_PB:
		pModSelData->ui.noEventsPB_W = w;
		break;
	      case AUDMODSEL_MORE_PB:
		pModSelData->ui.moreSelPB_W = w;
		break;
	      case AUDMODSEL_DELETE_PB:
		pModSelData->ui.deleteSelPB_W = w;
		break;
	      case AUDMODSEL_OK1:
		pModSelData->ui.OK1 = w;
		break;
	      case AUDMODSEL_APPLY1:
		pModSelData->ui.Apply1 = w;
		break;
	      case AUDMODSEL_CANCEL1:
		pModSelData->ui.Cancel1 = w;
		break;
	      case AUDMODSEL_HELP1:
		pModSelData->ui.Help1 = w;
		break;
	      case AUDMODSEL_SELLABEL:
		pModSelData->ui.selNameLBL_W = w;
		break;
	      case AUDMODSEL_DEVNO:
		pModSelData->ui.devNoTF_W = w;
		break;
	      case AUDMODSEL_VNODEID:
		pModSelData->ui.vnodeIDTF_W = w;
		break;
	      case AUDMODSEL_VNODENO:
		pModSelData->ui.vnodeNoTF_W = w;
		break;
	      case AUDMODSEL_PROCNAME:
		pModSelData->ui.procNameTF_W = w;
		break;
	      case AUDMODSEL_STRING:
		pModSelData->ui.stringTF_W = w;
		break;
	      case AUDMODSEL_ERROR:
		pModSelData->ui.errorTF_W = w;
		break;
	      case AUDMODSEL_OK2:
		pModSelData->ui.OK2 = w;
		break;
	      case AUDMODSEL_CANCEL2:
		pModSelData->ui.Cancel2 = w;
		break;
	      case AUDMODSEL_HELP2:
		pModSelData->ui.Help2 = w;
		break;
	    }
	}
    }
}




void
AUDModSel1MapFormCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    XmString		xmstring;
    AUDModSelPtr	pModSelData;
    int			i, j;
    SListTBType 	*data_lst; 
    static int		firstTime = True;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    /*
	     * When we map the callback, the following must occur:
	     *
	     * 1) (re)load list of selection files available
	     * 2) desensitize the delete button
	     * 3) select "All Events for Selection"
	     * 4) clear all existing fields
	     */
	    if(AUDGetFileList(AUDIT_SELDIR,
			      &audglobal.selectionFiles,
			      &audglobal.numSelectionFiles,
			      !firstTime, 
			      changedSelList))
	    {		    
		PostErrorDialog("AUDmsg_err_reading_sel_file",
				ancestor,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL);
		return;
	    }

	    /* Update the Selection File Widget Values if necessary */
	    if( firstTime || changedSelList )
	    {
		/* Delete current list */
		XmTextListDeleteAllItems(pModSelData->ui.selectionFileTL_W);

		for (i = 0; i < audglobal.numSelectionFiles; i++)
		{
		    XmString	xmstring;

		    xmstring = XmStringCreate(audglobal.selectionFiles[i],
					      charset);

		    XmTextListAddItem(pModSelData->ui.selectionFileTL_W,
				      xmstring,
				      0);
		    XmStringFree(xmstring);
		}
	    }

	    if( firstTime || changedBaseEvents || changedSiteEvents )
	    {
		if(AUDGetBaseEventList(&audglobal.baseEventList, 
				       &audglobal.baseEventCount,
				       !firstTime, 
				       changedBaseEvents) ||
		   AUDGetSiteEventList(&audglobal.siteEventList, 
				       &audglobal.siteEventCount,
				       True,
				       !firstTime, 
				       changedSiteEvents))
		{
		    /* ERROR */
		    PostErrorDialog("AUDmsg_err_reading_events_file",
				    ancestor,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL);
		    return;
		}

		data_lst = (SListTBType *) XtMalloc(sizeof(SListTBType) * 
						    audglobal.baseEventCount);
		if (data_lst != (SListTBType *)NULL)
		{
		    /* load the widget */
		    for (i = 0; i < audglobal.baseEventCount; i++)
		    {
			data_lst[i].name = (char *) XtMalloc(strlen(audglobal.baseEventList[i]) + 1);

			if(data_lst[i].name != (char *)NULL)
			{
			    strcpy(data_lst[i].name, audglobal.baseEventList[i]);
			    data_lst[i].on_flag  = 1;
			    data_lst[i].off_flag = 1;
			}
			else
			    MemoryError();
		    }

		    XmSelectListTBAddItems(pModSelData->ui.baseEventSB_W,
					   data_lst,
					   audglobal.baseEventCount,
					   0);
		    XmSelectListTBFree(data_lst,
				       audglobal.baseEventCount);
		}
		else
		    MemoryError();

		data_lst = (SListTBType *) XtMalloc(sizeof(SListTBType) * 
						    audglobal.siteEventCount);
		if (data_lst != (SListTBType *)NULL)
		{
		    /* load the widget */
		    for (i = 0; i < audglobal.siteEventCount; i++)
		    {
			data_lst[i].name = (char *) XtMalloc(strlen(audglobal.siteEventList[i]) + 1);

			if(data_lst[i].name != (char *)NULL)
			{
			    strcpy(data_lst[i].name, audglobal.siteEventList[i]);
			    data_lst[i].on_flag  = 1;
			    data_lst[i].off_flag = 1;
			}
			else
			    MemoryError();
		    }

		    XmSelectListTBAddItems(pModSelData->ui.baseEventSB_W,
					   data_lst,
					   audglobal.siteEventCount,
					   0);
		    XmSelectListTBFree(data_lst,
				       audglobal.siteEventCount);
		}
		else
		    MemoryError();
	    }

	    if( firstTime || changedAliasEvents )
	    {
		if(AUDGetAliasEventList(&audglobal.aliasEventList, 
					&audglobal.aliasEventCount,
					!firstTime, 
					changedAliasEvents))
		{
		    /* ERROR */
		    PostErrorDialog("AUDmsg_err_reading_alias_file",
				    ancestor,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL);
		    return;
		}

		data_lst = (SListTBType *) XtMalloc(sizeof(SListTBType) * 
						    audglobal.aliasEventCount);
		if (data_lst != (SListTBType *)NULL)
		{
		    /* load the widget */
		    for (i = 0; i < audglobal.aliasEventCount; i++)
		    {
			data_lst[i].name = (char *) XtMalloc(strlen(audglobal.aliasEventList[i]) + 1);

			if(data_lst[i].name != (char *)NULL)
			{
			    strcpy(data_lst[i].name, audglobal.aliasEventList[i]);
			    data_lst[i].on_flag  = 1;
			    data_lst[i].off_flag = 1;
			}
			else
			    MemoryError();
		    }

		    XmSelectListTBAddItems(pModSelData->ui.aliasEventSB_W,
					   data_lst,
					   audglobal.aliasEventCount,
					   0);
		    XmSelectListTBFree(data_lst,
				       audglobal.aliasEventCount);
		}
		else
		    MemoryError();
	    }

	    /* desensitize the "Delete" button */
	    XtSetSensitive(pModSelData->ui.deleteSelPB_W, False);

	    /* re-initialize all fields to whitespace */
	    AUDModSelClearFields(&pModSelData->ui, True);
		
	    /* no current changes */
	    form1Changed = form2Changed = False;

	    XmSelListInitTBChangeFlag(pModSelData->ui.baseEventSB_W);
	    XmSelListInitTBChangeFlag(pModSelData->ui.aliasEventSB_W);

	    /* OK, we are no longer a virgin */
	    firstTime = False;

	    /* we will need to keep notion of rev nos for each
	     * item here in the future when more screens use them.
	     */
	    changedSelList     = False;
	    changedBaseEvents  = False;
	    changedSiteEvents  = False;
	    changedAliasEvents = False;
	}
    }
}




void
AUDModSel2MapFormCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDModSelPtr	pModSelData;
    char		*buf;
    XmString		xmstring;
    Arg			args[1];
    Cardinal		n;

    GetUserData(mainWidget, pModSelData);

    if (pModSelData)
    {
	/* Set up the Selection FileName label */
	buf = XmTextGetString(pModSelData->ui.selectionFileTL_W);

	if(buf != (char *) NULL  &&
	   strlen(buf) != 0)
	{
	    AUDSetLabel(pModSelData->ui.selNameLBL_W, buf);
	    XtFree(buf);
	}
	else
	{
	    AUDSetLabel(pModSelData->ui.selNameLBL_W, (char *) NULL);
	}

	form2Managed = True;

	/* save the data in case the user cancels out */
	AUDSaveScreen2Data(&pModSelData->ui);
    }
}




void
AUDModSelSelectAllCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData;
    

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    /* Event Scroll Lists */
	    if(pModSelData->ui.baseEventSB_W) 
		XmSelectListTBSetAllItemsSelected(pModSelData->ui.baseEventSB_W);

	    if(pModSelData->ui.aliasEventSB_W) 
		XmSelectListTBSetAllItemsSelected(pModSelData->ui.aliasEventSB_W);
	}
    }
}




void
AUDModSelSelectNoCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData;
    

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    /* Event Scroll Lists */
	    if(pModSelData->ui.baseEventSB_W) 
		XmSelectListTBSetAllItemsAvailable(pModSelData->ui.baseEventSB_W);

	    if(pModSelData->ui.aliasEventSB_W) 
		XmSelectListTBSetAllItemsAvailable(pModSelData->ui.aliasEventSB_W);
	}
    }
}



void
AUDModSelOKToChangeCallback(w, data, info)
Widget 			w;
AUDLoseChangeDataPtr	data;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData;
	
    form1Changed = form2Changed = False;

    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    XmSelListInitTBChangeFlag(pModSelData->ui.baseEventSB_W);
	    XmSelListInitTBChangeFlag(pModSelData->ui.aliasEventSB_W);
	}
    }
    
    (*data->function)(data->w, data->flag, 0);
}






void
AUDModSelDoItCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData = (AUDModSelPtr) NULL;
    int			i;
    Boolean 		eventsChanged = False;
    
    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
	GetUserData(ancestor, pModSelData);

    if(pModSelData)
    {
	eventsChanged = 
		XmSelListTBMadeChanges(pModSelData->ui.baseEventSB_W) ||
		XmSelListTBMadeChanges(pModSelData->ui.aliasEventSB_W);
    }

    if( eventsChanged || form1Changed || form2Changed )
    {
	changeData.w        = w;
	changeData.flag     = 0;
	changeData.function = AUDModSelDoItCallback;
 
	if( ancestor )
	    PostErrorDialog("AUDmsg_conf_cancel_data",
			    ancestor,
			    AUDModSelOKToChangeCallback,
			    (XtPointer) &changeData,
			    AUDRestoreCorrectSelFileName,
			    (XtPointer) &changeData,
			    NULL,
			    NULL);
	return;
    }
    else
    {
	if (pModSelData)
	{
	    char 		*buf;

	    /* Cases:
	     *
	     * 1) User has selected an existing selection file
	     * 2) User has typed a new selection file name
	     */

	    /* But first, we must update our notion of existing
	     * selection files, since we are establishing a criterion
	     * that the selection file must be on the list to be
	     * considered existing.  If, for example, a new file
	     * was just created and then selected, we wouldn't detect
	     * that it existed without first adding to our list.  If
	     * this is not the case, then the call will be a noop since
	     * nothing will have changed since the last call.
	     */

	    if(AUDGetFileList(AUDIT_SELDIR, 
			      &audglobal.selectionFiles,
			      &audglobal.numSelectionFiles,
			      True,
			      changedSelList))
	    {		    
		PostErrorDialog("AUDmsg_err_reading_sel_file",
				ancestor,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL);
		return;
	    }

	    buf = XmTextGetString(pModSelData->ui.selectionFileTL_W);

	    if(buf != (char *) NULL &&
	       strlen(buf) != 0)
	    {
		i = 0;
		while(i < audglobal.numSelectionFiles &&
		      strcmp(audglobal.selectionFiles[i],
			     buf))
		    i++;

		if( i < audglobal.numSelectionFiles )
		{
		    /* modify existing */
		    AUDModSelClearFields(&pModSelData->ui, False);

		    /* Get rid of the data side */
		    AUDFreeSelectionData(&pModSelData->data);
		    
		    if( AUDReadSelectionData(buf,
					     &pModSelData->data) != 0 )
		    {
			/* ERROR */
			PostErrorDialog("AUDmsg_err_reading_sel_data",
					ancestor,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
			return;
		    }
			
		    AUDCvtSelDataToWidget(&pModSelData->data, 
					  &pModSelData->ui);
			
		    addingNew = False;
			
		    /* sensitize the "Delete" button */
		    XtSetSensitive(pModSelData->ui.deleteSelPB_W, True);
		}
		else
		{
		    /* create new */
		    AUDModSelClearFields(&pModSelData->ui, False);
		    
		    addingNew = True;
		    
		    /* desensitize the "Delete" button */
		    XtSetSensitive(pModSelData->ui.deleteSelPB_W, False);
		}

		/* save this name as the "current" one */
		if( currentSelFile )
		{
		    XtFree(currentSelFile);
		    currentSelFile = (char *) NULL;
		}

		currentSelFile = (char *) XtMalloc(strlen(buf)+1);
		if(currentSelFile)
		    strcpy(currentSelFile, buf);
		else
		    MemoryError();

		/* update the label on screen 2 if it's managed */
		if( form2Managed )
		{
		    AUDSetLabel(pModSelData->ui.selNameLBL_W, buf);
		    AUDSaveScreen2Data(&pModSelData->ui);
		}
		XtFree(buf);
	    }
	    else
	    {
		/* Blank field */
		if( currentSelFile )
		    XtFree(currentSelFile);

		currentSelFile = (char *) NULL;
		
		/* desensitize the "Delete" button */
		XtSetSensitive(pModSelData->ui.deleteSelPB_W, False);
	    }

	    /* initialize state */
	    form1Changed = form2Changed = False;
	    XmSelListInitTBChangeFlag(pModSelData->ui.baseEventSB_W);
	    XmSelListInitTBChangeFlag(pModSelData->ui.aliasEventSB_W);
	}
    }
}





int
AUDModSel1ValueChangedCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    form1Changed = True;
}




int
AUDModSel2ValueChangedCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    form2Changed = True;
}



int
AUDModSelApplyChanges(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData;
    

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    int  i;
	    char *buf;

	    buf = XmTextGetString(pModSelData->ui.selectionFileTL_W);

	    if(buf != (char *) NULL &&
	       strlen(buf) != 0)
	    {
		Boolean eventsChanged;

		/*
		 * At this point, we really might be adding a new selection
		 * file even though the code has not yet detected it.
		 * So, we must compare the name against the list and
		 * make the final call.
		 */
		i = 0;
		while(i < audglobal.numSelectionFiles &&
		      strcmp(audglobal.selectionFiles[i],
			     buf))
		    i++;

		if( i == audglobal.numSelectionFiles )
		{
		    addingNew    = True;
		    form1Changed = True;
		}

		eventsChanged = 
		  XmSelListTBMadeChanges(pModSelData->ui.baseEventSB_W) ||
		  XmSelListTBMadeChanges(pModSelData->ui.aliasEventSB_W);

		if( eventsChanged || form1Changed || form2Changed )
		{
		    AUDFreeSelectionData(&(pModSelData->data));

		    if(AUDCvtWidgetToSelData(&pModSelData->data,
					     &pModSelData->ui))
		    {
			PostErrorDialog("AUDmsg_err_need_at_least_one_event",
					ancestor,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
			return(-1);
		    }

		    if(AUDWriteSelectionData(buf, &pModSelData->data))
		    {
			PostErrorDialog("AUDmsg_err_writing_sel_data",
					ancestor,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
			return(-1);
		    }
		}

		/* Update the list of files if this is a new one */
		if( addingNew )
		{
		    XmString	xmstring;

		    xmstring = XmStringCreate(buf, charset);

		    XmTextListAddItem(pModSelData->ui.selectionFileTL_W,
				      xmstring,
				      0);
		    XmStringFree(xmstring);

		    changedSelList = True;
		    addingNew      = False;
		}

		/* clean-up */
		AUDFreeSelectionData(&pModSelData->data);
		XtFree(buf);
	    }
	}
    }
    return(AUDSuccess);
}





void
AUDModSelCancel(w, window)
Widget	w;
int	window;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData;

    ancestor = search_for_parent(w, MAIN_WIDGET);
    if( !ancestor )
	    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    Boolean eventsChanged;

	    eventsChanged = 
		    XmSelListTBMadeChanges(pModSelData->ui.baseEventSB_W) ||
		    XmSelListTBMadeChanges(pModSelData->ui.aliasEventSB_W);
	    if((window == ALL &&
		(eventsChanged || form1Changed || form2Changed)))
	    {
		changeData.w        = w;
		changeData.flag     = window;
		changeData.function = AUDModSelCancel;
 
		PostErrorDialog("AUDmsg_conf_cancel_data",
				ancestor,
				AUDModSelOKToChangeCallback,
				(XtPointer) &changeData,
				NULL,
				NULL,
				NULL,
				NULL);
		return;
	    }

	    if( form2Managed )
	    {
		VUIT_Unmanage(MAIN2_WIDGET);
		form2Managed = False;
		AUDFreeScreen2Data();
	    }

	    if(window == ALL)
		VUIT_Unmanage(MAIN_WIDGET);
	}
    }
}





void
AUDModSelApplyCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{

    if(AUDModSelApplyChanges(w, flag, info) == 0)
    {
	Widget		ancestor;
	AUDModSelPtr	pModSelData;
	
	form1Changed = form2Changed = False;

	ancestor = search_for_parent(w, MAIN_WIDGET);

	if (ancestor)
	{
	    GetUserData(ancestor, pModSelData);

	    if (pModSelData)
	    {
		XmSelListInitTBChangeFlag(pModSelData->ui.baseEventSB_W);
		XmSelListInitTBChangeFlag(pModSelData->ui.aliasEventSB_W);
	    }
	}
    }
}





void
AUDModSel1OKCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    if(AUDModSelApplyChanges(w, flag, info) == 0)
    {
	Widget		ancestor;
	AUDModSelPtr	pModSelData;
	
	form1Changed = form2Changed = False;

	ancestor = search_for_parent(w, MAIN_WIDGET);

	if (ancestor)
	{
	    GetUserData(ancestor, pModSelData);

	    if (pModSelData)
	    {
		XmSelListInitTBChangeFlag(pModSelData->ui.baseEventSB_W);
		XmSelListInitTBChangeFlag(pModSelData->ui.aliasEventSB_W);
	    }
	}
    }
    else
	return;

    AUDModSelCancel(w, ALL);
}




void
AUDModSel1CancelCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDModSelCancel(w, ALL);
}





void
AUDModSel2OKCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDModSelCancel(w, SECOND);
}




void
AUDModSel2CancelCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData;
    
    ancestor = search_for_parent(w, MAIN_WIDGET);
    if( !ancestor )
	    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    AUDRestoreScreen2Data(&pModSelData->ui);
	    AUDModSelCancel(w, SECOND);
	}
    }
}





void
AUDModSelDeleteFile(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData;
    char		filename[1024];

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    char 	*buf;
	    XmString	xmstring;

	    /* Get rid of the file!! */
	    buf = XmTextGetString(pModSelData->ui.selectionFileTL_W);

	    if(buf != (char *) NULL &&
	       strlen(buf) != 0)
	    {
		strcpy(filename, AUDIT_SELDIR);
		strcat(filename, "/");
		strcat(filename, buf);

		if( unlink(filename) != 0 )
		{
		    PostErrorDialog("AUDmsg_err_removing_sel_file",
				    ancestor,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL);
		    return;
		}
		
		AUDModSelClearFields(&pModSelData->ui, True);

		xmstring = XmStringCreate(buf, charset);

		XmTextListDeleteItem(pModSelData->ui.selectionFileTL_W,
				     xmstring);

		changedSelList = True;

		form1Changed = form2Changed = False;
		XmSelListInitTBChangeFlag(pModSelData->ui.baseEventSB_W);
		XmSelListInitTBChangeFlag(pModSelData->ui.aliasEventSB_W);

		/* desensitize the "Delete" button */
		XtSetSensitive(pModSelData->ui.deleteSelPB_W, False);

		XtFree(buf);
		XmStringFree(xmstring);
	    }
	}
    }
}



void
AUDModSelDeleteFileCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModSelPtr	pModSelData;
    

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	PostQuestionDialog("AUDmsg_conf_remove_sel_file",
			   ancestor,
			   AUDModSelDeleteFile,
			   NULL,
			   NULL,
			   NULL,
			   NULL,
			   NULL);
    }
}



void
AUDModSelTextFocusCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor, textWidget;
    AUDModSelPtr	pModSelData;
    Arg		        args[1];
    Cardinal            n;


    
    ancestor = search_for_parent(w, MAIN_WIDGET);
    if( !ancestor )
	    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    n = 0;
	    XtSetArg(args[n], XmNcursorPositionVisible, True); n++;

	    textWidget = (Widget) NULL;

	    switch (*flag)
	    {
	      case AUDMODSEL_STARTTIME:
		textWidget = pModSelData->ui.startTimeTF_W;
		break;
	      case AUDMODSEL_STOPTIME:
		textWidget = pModSelData->ui.stopTimeTF_W;
		break;
	      case AUDMODSEL_SELFILELIST:
		textWidget = pModSelData->ui.selectionFileTL_W;
		break;
	      case AUDMODSEL_PID:
		textWidget = pModSelData->ui.pidTF_W;
		break;
	      case AUDMODSEL_PPID:
		textWidget = pModSelData->ui.ppidTF_W;
		break;
	      case AUDMODSEL_AUID:
		textWidget = pModSelData->ui.auidTF_W;
		break;
	      case AUDMODSEL_RUID:
		textWidget = pModSelData->ui.ruidTF_W;
		break;
	      case AUDMODSEL_EUID:
		textWidget = pModSelData->ui.euidTF_W;
		break;
	      case AUDMODSEL_USERNAME:
		textWidget = pModSelData->ui.userNameTF_W;
		break;
	      case AUDMODSEL_HOSTNAME:
		textWidget = pModSelData->ui.hostNameTF_W;
		break;
	      case AUDMODSEL_DEVNO:
		textWidget = pModSelData->ui.devNoTF_W;
		break;
	      case AUDMODSEL_VNODEID:
		textWidget = pModSelData->ui.vnodeIDTF_W;
		break;
	      case AUDMODSEL_VNODENO:
		textWidget = pModSelData->ui.vnodeNoTF_W;
		break;
	      case AUDMODSEL_PROCNAME:
		textWidget = pModSelData->ui.procNameTF_W;
		break;
	      case AUDMODSEL_ERROR:
		textWidget = pModSelData->ui.errorTF_W;
		break;
	      case AUDMODSEL_STRING:
		textWidget = pModSelData->ui.stringTF_W;
		break;
	    }
	    if( textWidget != (Widget) NULL )
		XtSetValues(textWidget, args, n);
	}
    }
}




void
AUDModSelTextLosingFocusCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor, textWidget;
    AUDModSelPtr	pModSelData;
    Arg		        args[1];
    Cardinal            n;
    Boolean		useForm2 = False;
    char		*delim 	     = ", ";
    char		*devNoDelim  = " ";
    char		*stringDelim = "\n";
    char		*relevantDelimiters;

    ancestor = search_for_parent(w, MAIN_WIDGET);
    if( !ancestor )
    {
	useForm2 = True;
	ancestor = mainWidget;
    }

    if (ancestor)
    {
	GetUserData(ancestor, pModSelData);

	if (pModSelData)
	{
	    n = 0;
	    XtSetArg(args[n], XmNcursorPositionVisible, False); n++;

	    textWidget = (Widget) NULL;

	    switch (*flag)
	    {
	      case AUDMODSEL_STARTTIME:
		textWidget = pModSelData->ui.startTimeTF_W;
		break;
	      case AUDMODSEL_STOPTIME:
		textWidget = pModSelData->ui.stopTimeTF_W;
		break;
	      case AUDMODSEL_SELFILELIST:
		textWidget = pModSelData->ui.selectionFileTL_W;
		break;
	      case AUDMODSEL_PID:
		textWidget = pModSelData->ui.pidTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_PPID:
		textWidget = pModSelData->ui.ppidTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_AUID:
		textWidget = pModSelData->ui.auidTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_RUID:
		textWidget = pModSelData->ui.ruidTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_EUID:
		textWidget = pModSelData->ui.euidTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_USERNAME:
		textWidget = pModSelData->ui.userNameTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_HOSTNAME:
		textWidget = pModSelData->ui.hostNameTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_DEVNO:
		textWidget = pModSelData->ui.devNoTF_W;
		relevantDelimiters = devNoDelim;
		break;
	      case AUDMODSEL_VNODEID:
		textWidget = pModSelData->ui.vnodeIDTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_VNODENO:
		textWidget = pModSelData->ui.vnodeNoTF_W;
		relevantDelimiters = devNoDelim;
		break;
	      case AUDMODSEL_PROCNAME:
		textWidget = pModSelData->ui.procNameTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_ERROR:
		textWidget = pModSelData->ui.errorTF_W;
		relevantDelimiters = delim;
		break;
	      case AUDMODSEL_STRING:
		textWidget = pModSelData->ui.stringTF_W;
		relevantDelimiters = stringDelim;
		break;
	    }
	    if( textWidget != (Widget) NULL )
	    {
		XtSetValues(textWidget, args, n);

		/* detect too many entries */
		if(*flag != AUDMODSEL_STARTTIME &&
		   *flag != AUDMODSEL_STOPTIME &&
		   *flag != AUDMODSEL_SELFILELIST)
		{
		    char *buf;
		    char *token;
		    int  count;

		    buf = XmTextGetString(textWidget);

		    if(buf != (char *) NULL &&
		       strlen(buf) != 0)
		    {
			/* parse entries */
			token = strtok(buf, relevantDelimiters);

			count = 0;
			if(token)
			{
			    count++;
			    while ((token = strtok((char *) 0, 
						   relevantDelimiters)) 
				   != (char *) 0)
			    {
				count++;
			    }
			    
			    if(count > AUDIT_MAX_ENTRIES)
			    {
				PostErrorDialog("AUDmsg_err_entry_count",
						(useForm2 ? 
						 pModSelData->ui.form2_W :
						 ancestor),
						NULL,
						NULL,
						NULL,
						NULL,
						NULL,
						NULL);
			    }
			}
		    }
		    
		    XtFree(buf);
		}
	    }
	}
    }
}




/*=====================<Low-Level Support & I/O Routines>==================*/

int
AUDWriteSelectionData( selFile, selData )
char			*selFile;
AUDModSelDataPtr	selData;
{
    FILE		*fp;
    unsigned int 	tmask;
    int			i;
    char		fileName[1024];

    /* Open the file */
    strcpy(fileName, AUDIT_SELDIR);
    strcat(fileName, "/");
    strcat(fileName, selFile);

    if( (fp = fopen(fileName, "w")) == (FILE *) 0 )
	    return(-1);

    rewind( fp );

    tmask = selData->mask;

    while( tmask )
    {
	unsigned int	index = (unsigned int) lowbit(tmask);

	tmask &=~ index;

	switch( index )
	{
	  case AUDMODSEL_STARTTIME_MASK:
	    if(notBlanks(selData->startTime))
		fprintf(fp, "%s %s ", 
			AUDREP_STARTTIME_OPT, 
			selData->startTime);
	    break;

	  case AUDMODSEL_STOPTIME_MASK:
	    if(notBlanks(selData->stopTime))
		fprintf(fp, "%s %s ", 
			AUDREP_STOPTIME_OPT, 
			selData->stopTime);
	    break;

	  case AUDMODSEL_HOSTNAME_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->hostName[i] != (char *) NULL) &&
		  notBlanks(selData->hostName[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_HOSTNAME_OPT, 
			selData->hostName[i++]);
	    }
	    break;

	  case AUDMODSEL_RUID_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->ruid[i] != (char *) NULL) &&
		  notBlanks(selData->ruid[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_RUID_OPT, 
			selData->ruid[i++]);
	    }
	    break;

	  case AUDMODSEL_EUID_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->euid[i] != (char *) NULL) &&
		  notBlanks(selData->euid[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_EUID_OPT, 
			selData->euid[i++]);
	    }
	    break;

	  case AUDMODSEL_AUID_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->auid[i] != (char *) NULL) &&
		  notBlanks(selData->auid[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_AUID_OPT, 
			selData->auid[i++]);
	    }
	    break;

	  case AUDMODSEL_USERNAME_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->userName[i] != (char *) NULL) &&
		  notBlanks(selData->userName[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_USERNAME_OPT, 
			selData->userName[i++]);
	    }
	    break;

	  case AUDMODSEL_PID_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->pid[i] != (char *) NULL) &&
		  notBlanks(selData->pid[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_PID_OPT, 
			selData->pid[i++]);
	    }
	    break;

	  case AUDMODSEL_PPID_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->ppid[i] != (char *) NULL) &&
		  notBlanks(selData->ppid[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_PPID_OPT, 
			selData->ppid[i++]);
	    }
	    break;

	  case AUDMODSEL_DEVNO_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->devNo[i] != (char *) NULL) &&
		  notBlanks(selData->devNo[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_DEVNO_OPT, 
			selData->devNo[i++]);
	    }
	    break;

	  case AUDMODSEL_VNODEID_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->vnodeID[i] != (char *) NULL) &&
		  notBlanks(selData->vnodeID[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_VNODEID_OPT, 
			selData->vnodeID[i++]);
	    }
	    break;

	  case AUDMODSEL_VNODENO_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->vnodeNo[i] != (char *) NULL) &&
		  notBlanks(selData->vnodeNo[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_VNODENO_OPT, 
			selData->vnodeNo[i++]);
	    }
	    break;

	  case AUDMODSEL_STRING_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->string[i] != (char *) NULL) &&
		  notBlanks(selData->string[i]))
	    {
		fprintf(fp, "%s \"%s\" ", 
			AUDREP_STRING_OPT, 
			selData->string[i++]);
	    }
	    break;

	  case AUDMODSEL_ERROR_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->error[i] != (char *) NULL) &&
		  notBlanks(selData->error[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_ERROR_OPT, 
			selData->error[i++]);
	    }
	    break;

	  case AUDMODSEL_PROCNAME_MASK:
	    i = 0;
	    while((i < AUDIT_MAX_ENTRIES) &&
		  (selData->procName[i] != (char *) NULL) &&
		  notBlanks(selData->procName[i]))
	    {
		fprintf(fp, "%s %s ", 
			AUDREP_PROCNAME_OPT, 
			selData->procName[i++]);
	    }
	    break;

	  case AUDMODSEL_EVENT_MASK:
	    if( !selData->allEvents )
	    {
		for( i = 0; i < selData->numEvents; i++ )
		{
		    if( selData->events[i]->subEvent )
			fprintf(fp, "%s %s.%s:%d:%d ",
				AUDREP_EVENT_OPT, 
				selData->events[i]->eventName,
				selData->events[i]->subEvent,
				selData->events[i]->success,
				selData->events[i]->failure);
		    else
			fprintf(fp, "%s %s:%d:%d ",
				AUDREP_EVENT_OPT, 
				selData->events[i]->eventName,
				selData->events[i]->success,
				selData->events[i]->failure);
		}
	    }
	    break;

	  default:
	    /* Ignore */
	    break;
	} /* end switch */
    } /* end while */

    fclose(fp);
    return( AUDSuccess );

} /* end AUDWriteSelectionData() */




int
AUDReadParameter(fp, index, data, mask, maskValue)
FILE	*fp;
int	*index;
char	**data;
int	*mask, maskValue;
{
    char	item[256];

    if( fscanf(fp, "%s", item) != 1 )
	return(-1);
    else
    {
	if( *index < AUDIT_MAX_ENTRIES )
	{
	    data[*index] = (char *) XtMalloc(strlen(item)+1);
			    
	    if(data[*index])
		strcpy(data[*index], item);
	    else
		return(-1);
	    (*index)++;
	}	
    }
    *mask |= maskValue;
    
    return(AUDSuccess);
}




int
AUDReadSelectionData( selFile, selData )
char			*selFile;
AUDModSelDataPtr	selData;
{
    FILE	*fp;
    char	ch, fileName[1024], option[256], item[256], nextitem[256];
    int		i, hostNameIndex, auidIndex, ruidIndex, euidIndex,
                userNameIndex, pidIndex, ppidIndex, devNoIndex,
                vnodeIDIndex, vnodeNoIndex, stringIndex, errorIndex,
                procNameIndex, eventIndex;

    /* Open the file */
    strcpy(fileName, AUDIT_SELDIR);
    strcat(fileName, "/");
    strcat(fileName, selFile);

    if( (fp = fopen(fileName, "r")) == (FILE *) 0 )
	    return(-1);

    rewind( fp );

    selData->mask      = 0;
    selData->numEvents = 0;

    hostNameIndex = 0, auidIndex = 0, ruidIndex = 0, euidIndex = 0,
    userNameIndex = 0, pidIndex = 0, ppidIndex = 0, devNoIndex = 0,
    vnodeIDIndex = 0, vnodeNoIndex = 0, stringIndex = 0, errorIndex = 0,
    procNameIndex = 0, eventIndex = 0;

    while( fscanf(fp, "%s", option) != EOF )
    {
	if( option[0] != '-' )
	    continue;

	switch( option[1] )
	{
	  case AUDREP_STARTTIME_CONST:
	    if( fscanf(fp, "%s", selData->startTime) != 1 )
		return(-1);
	    selData->mask |= AUDMODSEL_STARTTIME_MASK;
	    break;

	  case AUDREP_STOPTIME_CONST:
	    if( fscanf(fp, "%s", selData->stopTime) != 1 )
		return(-1);
	    selData->mask |= AUDMODSEL_STOPTIME_MASK;
	    break;

	  case AUDREP_HOSTNAME_CONST: 
	    if(AUDReadParameter(fp, 
				&hostNameIndex, 
				selData->hostName, 
				&selData->mask, 
				AUDMODSEL_HOSTNAME_MASK))
		return(-1);
	    break;

	  case AUDREP_RUID_CONST: 
	    if(AUDReadParameter(fp, 
				&ruidIndex, 
				selData->ruid, 
				&selData->mask, 
				AUDMODSEL_RUID_MASK))
		return(-1);
	    break;

	  case AUDREP_EUID_CONST: 
	    if(AUDReadParameter(fp, 
				&euidIndex, 
				selData->euid, 
				&selData->mask, 
				AUDMODSEL_EUID_MASK))
		return(-1);
	    break;

	  case AUDREP_AUID_CONST:
	    if(AUDReadParameter(fp, 
				&auidIndex, 
				selData->auid, 
				&selData->mask, 
				AUDMODSEL_AUID_MASK))
		return(-1);
	    break;

	  case AUDREP_USERNAME_CONST:
	    if(AUDReadParameter(fp, 
				&userNameIndex, 
				selData->userName, 
				&selData->mask, 
				AUDMODSEL_USERNAME_MASK))
		return(-1);
	    break;

	  case AUDREP_PID_CONST:
	    if(AUDReadParameter(fp, 
				&pidIndex, 
				selData->pid, 
				&selData->mask, 
				AUDMODSEL_PID_MASK))
		return(-1);
	    break;

	  case AUDREP_PPID_CONST:
	    if(AUDReadParameter(fp, 
				&ppidIndex, 
				selData->ppid, 
				&selData->mask, 
				AUDMODSEL_PPID_MASK))
		return(-1);
	    break;

	  case AUDREP_DEVNO_CONST:
	    if(AUDReadParameter(fp, 
				&devNoIndex, 
				selData->devNo, 
				&selData->mask, 
				AUDMODSEL_DEVNO_MASK))
		return(-1);
	    break;

	  case AUDREP_VNODEID_CONST:
	    if(AUDReadParameter(fp, 
				&vnodeIDIndex, 
				selData->vnodeID, 
				&selData->mask, 
				AUDMODSEL_VNODEID_MASK))
		return(-1);
	    break;

	  case AUDREP_VNODENO_CONST:
	    if(AUDReadParameter(fp, 
				&vnodeNoIndex, 
				selData->vnodeNo, 
				&selData->mask, 
				AUDMODSEL_VNODENO_MASK))
		return(-1);
	    break;

	  case AUDREP_STRING_CONST:
	    /* leading " */
	    fscanf(fp, "%c", &ch);
	    while( ch != '"' )
		fscanf(fp, "%c", &ch);

	    /* get the string */
	    fscanf(fp, "%c", &ch);
	    i = 0;
	    while( ch != '"' )
	    {
		item[i++] = ch;
		fscanf(fp, "%c", &ch);
	    }

	    item[i] = '\0';

	    if( stringIndex < AUDIT_MAX_ENTRIES )
	    {
		selData->string[stringIndex] = (char *) 
			XtMalloc(strlen(item)+1);
			    
		if(selData->string[stringIndex])
		    strcpy(selData->string[stringIndex], item);
		else
		    return(-1);
		stringIndex++;
	    }	

	    selData->mask |= AUDMODSEL_STRING_MASK;
	    break;

	  case AUDREP_ERROR_CONST:
	    if(AUDReadParameter(fp, 
				&errorIndex, 
				selData->error, 
				&selData->mask, 
				AUDMODSEL_ERROR_MASK))
		return(-1);
	    break;

	  case AUDREP_PROCNAME_CONST:
	    if(AUDReadParameter(fp, 
				&procNameIndex, 
				selData->procName, 
				&selData->mask, 
				AUDMODSEL_PROCNAME_MASK))
		return(-1);
	    break;

	  case AUDREP_EVENT_CONST:
	    /* events are funny.  They are in the format:
	     *
	     * <event name>[.subevent]:{0,1}:{0,1}
	     *
	     */
	    if( fscanf(fp, "%s", item) != 1 )
		return(-1);
	    else
	    {
		char		*ptr;

		/* item may contain the entire event or just part of the
		 * event name.
		 */
		while((ptr = strchr(item, ':')) == (char *) 0)
		{
		    /* need to get the rest before proceeding */
		    if( fscanf(fp, "%s", nextitem) != 1 )
			return(-1);
		    strcat(item, " ");
		    strcat(item, nextitem);
		}

		/* got the whole thing */
		selData->events = (AUDevent **) 
			XtRealloc((char *)selData->events,
				  sizeof(AUDevent *) * (eventIndex+1));

		if( selData->events )
		{
		    int cnt;

		    selData->events[eventIndex] = (AUDevent *)
			    XtMalloc(sizeof(AUDevent));
		    if(selData->events[eventIndex])
		    {
			/* determine if this is an event with a 
			 * subevent.
			 */
			if( (ptr = strchr(item, '.')) != (char *)  NULL)
			{
			    cnt = 0;
			    while( *(ptr+1+cnt) != ':' )
				cnt++;
			    selData->events[eventIndex]->subEvent = (char *)
				XtMalloc(cnt+1);
			
			    strncpy(selData->events[eventIndex]->subEvent, 
				    ptr+1,
				    cnt);
			    selData->events[eventIndex]->subEvent[cnt] = '\0';
			}
			else
				selData->events[eventIndex]->subEvent = 
					(char *) NULL;

			cnt = 0;
			while(item[cnt] != ':' && item[cnt] != '.')
			    cnt++;

			selData->events[eventIndex]->eventName = (char *)
				XtMalloc(cnt+1);

			/* event name */
			if(selData->events[eventIndex]->eventName)
			{
			    strncpy(selData->events[eventIndex]->eventName,
				    item,
				    cnt);
			    selData->events[eventIndex]->eventName[cnt] = '\0';
			}
			else
			    return(-1);

			/* success, failure flags */
			while(item[cnt] != ':')
			    cnt++;
			selData->events[eventIndex]->success = 
				atoi(&item[cnt+1]);
			selData->events[eventIndex]->failure = 
				atoi(&item[cnt+3]);

			eventIndex++;
		    }
		    else
		    {
			MemoryError();
		    }
		}
		else
		{
		    MemoryError();
		}
	    }
	    break;

	  default:
	    /* ignore for now */
	    break;
	} /* end switch */
    } /* end while */

    selData->numEvents = eventIndex;
    selData->allEvents = (eventIndex == 0);

    if( eventIndex > 0 )
	selData->mask |= AUDMODSEL_EVENT_MASK;

    fclose( fp );
    return( AUDSuccess );

} /* end AUDReadSelectionData() */
	




void
AUDCvtSelDataToWidget( selData, selUI )
AUDModSelDataPtr	selData;
AUDModSelUIPtr		selUI;
{
    char	all[1024];
    SListTBType *base_data_lst;
    SListTBType *alias_data_lst;
    int		i, j, k;

    /* step through each field and set appropriately */

    /* Start Time */
    if( selData->mask & AUDMODSEL_STARTTIME_MASK )
    {
	XmTextSetString(selUI->startTimeTF_W,
			selData->startTime);
    }
    else
    {
	XmTextSetString(selUI->startTimeTF_W,
			"");
    }

    /* Stop Time */
    if( selData->mask & AUDMODSEL_STOPTIME_MASK )
    {
	XmTextSetString(selUI->stopTimeTF_W,
			selData->stopTime);
    }
    else
    {
	XmTextSetString(selUI->stopTimeTF_W,
			"");
    }

    /* Hostnames */
    if( selData->mask & AUDMODSEL_HOSTNAME_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->hostName[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, selData->hostName[i]) :
		       strcat(all, selData->hostName[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->hostNameTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->hostNameTF_W,
			"");
    }

    /* Audit IDS */
    if( selData->mask & AUDMODSEL_AUID_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->auid[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, selData->auid[i]) :
		       strcat(all, selData->auid[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->auidTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->auidTF_W,
			"");
    }

    /* Real UIDS */
    if( selData->mask & AUDMODSEL_RUID_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->ruid[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, selData->ruid[i]) :
		       strcat(all, selData->ruid[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->ruidTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->ruidTF_W,
			"");
    }

    /* Effective UIDS */
    if( selData->mask & AUDMODSEL_EUID_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->euid[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, selData->euid[i]) :
		       strcat(all, selData->euid[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->euidTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->euidTF_W,
			"");
    }

    /* PIDS */
    if( selData->mask & AUDMODSEL_PID_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->pid[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, selData->pid[i]) :
		       strcat(all, selData->pid[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->pidTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->pidTF_W,
			"");
    }

    /* PPIDS */
    if( selData->mask & AUDMODSEL_PPID_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->ppid[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, selData->ppid[i]) :
		       strcat(all, selData->ppid[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->ppidTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->ppidTF_W,
			"");
    }

    /* Usernames */
    if( selData->mask & AUDMODSEL_USERNAME_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->userName[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, selData->userName[i]) :
		       strcat(all, selData->userName[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->userNameTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->userNameTF_W,
			"");
    }

    /* Events */
    if( selData->mask & AUDMODSEL_EVENT_MASK )
    {
	XmSelectListTBSetAllItemsAvailable(selUI->baseEventSB_W);
	XmSelectListTBSetAllItemsAvailable(selUI->aliasEventSB_W);

	alias_data_lst = (SListTBType *) XtMalloc(sizeof(SListTBType) * 
						  selData->numEvents);
	base_data_lst  = (SListTBType *) XtMalloc(sizeof(SListTBType) * 
						  selData->numEvents);

	if(alias_data_lst != (SListTBType *) NULL && 
	   base_data_lst  != (SListTBType *) NULL)
	{ 
	    char fullEvent[AUD_MAXEVENT_LEN+1];

	    for( i = 0, j = 0, k = 0; i < selData->numEvents; i++)
	    {
		if(selData->events[i]->subEvent)
		{
		    sprintf(fullEvent, "%s.%s", 
			    selData->events[i]->eventName,
			    selData->events[i]->subEvent);
		}
		else
		{
		    sprintf(fullEvent, "%s", 
			    selData->events[i]->eventName);
		}

		/* Add the Events/Aliases to the appropriate selection boxes */
		if(XmAvailableListTBItemExists(selUI->baseEventSB_W, 
					       fullEvent))
		{
		    base_data_lst[j].name     = (char *) XtMalloc(strlen(fullEvent)+1);
		    if(base_data_lst[j].name != (char *)NULL)
			strcpy(base_data_lst[j].name, fullEvent);
		    else
			MemoryError();

		    base_data_lst[j].on_flag  = selData->events[i]->success;
		    base_data_lst[j].off_flag = selData->events[i]->failure;
		    j++;
		}
		else if (XmAvailableListTBItemExists(selUI->aliasEventSB_W,
						     fullEvent))
		{
		    alias_data_lst[k].name     = (char *) XtMalloc(strlen(fullEvent)+1);
		    if(alias_data_lst[k].name != (char *)NULL)
			strcpy(alias_data_lst[k].name, fullEvent);
		    else
			MemoryError();

		    alias_data_lst[k].on_flag  = selData->events[i]->success;
		    alias_data_lst[k].off_flag = selData->events[i]->failure;
		    k++;
		}
		else
		{
		    printf("Unknown event *%s*\n",fullEvent);
		}
	    }
	}
	else
	    MemoryError();

	XmSelectListTBSetItemsSelected(selUI->baseEventSB_W,
				       base_data_lst,
				       j);
	XmSelectListTBSetItemsSelected(selUI->aliasEventSB_W,
				       alias_data_lst,
				       k);

	/* Free */
	XmSelectListTBFree(base_data_lst ,j);
	XmSelectListTBFree(alias_data_lst,k);
    }

    /*
     * if no "-e"'s show up then we have effectively selected ALL
     * events.
     */
    else
    {
	XmSelectListTBSetAllItemsSelected(selUI->baseEventSB_W);
	XmSelectListTBSetAllItemsSelected(selUI->aliasEventSB_W);

	/* ZZZ reset the toggle buttons to the default case */
	XmSelListTBSetAllTBs(selUI->baseEventSB_W,  (int)True, (int)True);
	XmSelListTBSetAllTBs(selUI->aliasEventSB_W, (int)True, (int)True);
    }

    /* Device Numbers */
    if( selData->mask & AUDMODSEL_DEVNO_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->devNo[i] != (char *) NULL )
	{
	    (i == 0) ? strcpy(all, selData->devNo[i]) :
		       strcat(all, selData->devNo[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->devNoTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->devNoTF_W,
			"");
    }

    /* Vnode IDS */
    if( selData->mask & AUDMODSEL_VNODEID_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->vnodeID[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, selData->vnodeID[i]) :
		       strcat(all, selData->vnodeID[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->vnodeIDTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->vnodeIDTF_W,
			"");
    }

    /* Vnode Device Numbers */
    if( selData->mask & AUDMODSEL_VNODENO_MASK )
    {
	i = 0;

	while(i < AUDIT_MAX_ENTRIES &&
	      selData->vnodeNo[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, selData->vnodeNo[i]) :
		       strcat(all, selData->vnodeNo[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->vnodeNoTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->vnodeNoTF_W,
			"");
    }

    /* Process Names */
    if( selData->mask & AUDMODSEL_PROCNAME_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->procName[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, selData->procName[i]) :
		       strcat(all, selData->procName[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->procNameTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->procNameTF_W,
			"");
    }

    /* Strings */
    if( selData->mask & AUDMODSEL_STRING_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&
	      selData->string[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, selData->string[i]) :
		       strcat(all, selData->string[i]);
	    strcat(all, "\n");
	    i++;
	}

	XmTextSetString(selUI->stringTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->stringTF_W,
			"");
    }

    /* Errors */
    if( selData->mask & AUDMODSEL_ERROR_MASK )
    {
	i = 0;
	while(i < AUDIT_MAX_ENTRIES &&	
	      selData->error[i] != (char *) NULL )
  	{
	    (i == 0) ? strcpy(all, selData->error[i]) :
		       strcat(all, selData->error[i]);
	    strcat(all, " ");
	    i++;
	}

	XmTextSetString(selUI->errorTF_W,
			all);
    }
    else
    {
	XmTextSetString(selUI->errorTF_W,
			"");
    }

} /* end AUDCvtSelDataToWidget() */




int
AUDCvtWidgetToSelData( selData, selUI )
AUDModSelDataPtr	selData;
AUDModSelUIPtr		selUI;
{
    char	*buf;
    char	*delim 	     = ", ";
    char	*devNoDelim  = " ";
    char	*stringDelim = "\n";
    int		i, j, end, baseCount, aliasCount;
    SListTBType *base_data_lst, *alias_data_lst; 

    /* even though the data structure is "clean", we'll set the mask
     * anyway
     */
    selData->mask = 0;

    /* Start Time */
    buf = XmTextGetString(selUI->startTimeTF_W);

    if(buf != (char *) NULL &&
       strlen(buf) != 0)
    {
	selData->mask |= AUDMODSEL_STARTTIME_MASK;

	strncpy(selData->startTime, buf, end = min(strlen(buf), 12));
	selData->startTime[end] = '\0';
	XtFree(buf);
    }

    /* Stop Time */
    buf = XmTextGetString(selUI->stopTimeTF_W);

    if(buf != (char *) NULL &&
       strlen(buf) != 0)
    {
	selData->mask |= AUDMODSEL_STOPTIME_MASK;

	strncpy(selData->stopTime, buf, end = min(strlen(buf), 12));
	selData->stopTime[end] = '\0';
	XtFree(buf);
    }

    /* PID */
    AUDLoadTokenStringFromWidget(selUI->pidTF_W,
				 &selData->mask,
				 AUDMODSEL_PID_MASK,
				 delim,
				 selData->pid);

    /* PPID */
    AUDLoadTokenStringFromWidget(selUI->ppidTF_W,
				 &selData->mask,
				 AUDMODSEL_PPID_MASK,
				 delim,
				 selData->ppid);

    /* AUID */
    AUDLoadTokenStringFromWidget(selUI->auidTF_W,
				 &selData->mask,
				 AUDMODSEL_AUID_MASK,
				 delim,
				 selData->auid);

    /* RUID */
    AUDLoadTokenStringFromWidget(selUI->ruidTF_W,
				 &selData->mask,
				 AUDMODSEL_RUID_MASK,
				 delim,
				 selData->ruid);

    /* EUID */
    AUDLoadTokenStringFromWidget(selUI->euidTF_W,
				 &selData->mask,
				 AUDMODSEL_EUID_MASK,
				 delim,
				 selData->euid);

    /* UserName */
    AUDLoadTokenStringFromWidget(selUI->userNameTF_W,
				 &selData->mask,
				 AUDMODSEL_USERNAME_MASK,
				 delim,
				 selData->userName);

    /* HostName */
    AUDLoadTokenStringFromWidget(selUI->hostNameTF_W,
				 &selData->mask,
				 AUDMODSEL_HOSTNAME_MASK,
				 delim,
				 selData->hostName);

    /* ProcName */
    AUDLoadTokenStringFromWidget(selUI->procNameTF_W,
				 &selData->mask,
				 AUDMODSEL_PROCNAME_MASK,
				 delim,
				 selData->procName);

    /* Device Numbers */
    AUDLoadTokenStringFromWidget(selUI->devNoTF_W,
				 &selData->mask,
				 AUDMODSEL_DEVNO_MASK,
				 devNoDelim,
				 selData->devNo);

    /* Vnode Device Numbers */
    AUDLoadTokenStringFromWidget(selUI->vnodeNoTF_W,
				 &selData->mask,
				 AUDMODSEL_VNODENO_MASK,
				 devNoDelim,
				 selData->vnodeNo);

    /* Vnode IDs */
    AUDLoadTokenStringFromWidget(selUI->vnodeIDTF_W,
				 &selData->mask,
				 AUDMODSEL_VNODEID_MASK,
				 delim,
				 selData->vnodeID);

    /* Strings */
    AUDLoadTokenStringFromWidget(selUI->stringTF_W,
				 &selData->mask,
				 AUDMODSEL_STRING_MASK,
				 stringDelim,
				 selData->string);

    /* Errors */
    AUDLoadTokenStringFromWidget(selUI->errorTF_W,
				 &selData->mask,
				 AUDMODSEL_ERROR_MASK,
				 delim,
				 selData->error);
    /* Base Events */
    XmSelectListTBGetItems(selUI->baseEventSB_W,
			   &base_data_lst,
			   &baseCount);

    XmSelectListTBGetItems(selUI->aliasEventSB_W,
			   &alias_data_lst,
			   &aliasCount);

    /* detect "allEvents" */
    if((selData->numEvents = baseCount + aliasCount) == 0)
	return(-1);

    /* ZZZ expand case to check for toggle button settings */
    if((selData->numEvents == 
	(audglobal.baseEventCount + 
	 audglobal.siteEventCount + 
	 audglobal.aliasEventCount)) &&
       (XmSelListTBAllTBsInState(selUI->baseEventSB_W,
				 LEFT_TOGGLE_BUTTON,
				 (Boolean)True) &&
	XmSelListTBAllTBsInState(selUI->baseEventSB_W,
				 RIGHT_TOGGLE_BUTTON,
				 (Boolean)True) &&
	XmSelListTBAllTBsInState(selUI->aliasEventSB_W,
				 LEFT_TOGGLE_BUTTON,
				 (Boolean)True) &&
	XmSelListTBAllTBsInState(selUI->aliasEventSB_W,
				 RIGHT_TOGGLE_BUTTON,
				 (Boolean)True)))
    {
	selData->allEvents = True;
	selData->numEvents = 0;
    }
    else
    {
	selData->allEvents = False;
	selData->mask |= AUDMODSEL_EVENT_MASK;

	selData->events = (AUDevent **) 
		XtMalloc(sizeof(AUDevent *) * selData->numEvents);

	for( i = 0; i < selData->numEvents; i++ )
	{
	    selData->events[i] = (AUDevent *) XtMalloc(sizeof(AUDevent));
	    if(selData->events[i])
	    {
		char *ptr, *eventPtr;

		/* get the right address for processing */
		eventPtr = ((i < baseCount) ? base_data_lst [i].name :
			                      alias_data_lst[i-baseCount].name);

		/* must parse out the subevents */
		if( (ptr = strchr(eventPtr, '.')) != (char *) 0)
		{
		    /* We have a subevent */
		    *ptr = '\0';
		}
	    
		selData->events[i]->eventName = (char *)
			XtMalloc(strlen(eventPtr)+1);

		/* event name */
		if(selData->events[i]->eventName)
			strcpy(selData->events[i]->eventName,
			       eventPtr);
		else
		    MemoryError();

		/* success, failure flags */
		selData->events[i]->success = (i < baseCount) ? base_data_lst[i].on_flag :
			                                        alias_data_lst[i-baseCount].on_flag;
		selData->events[i]->failure = (i < baseCount) ? base_data_lst[i].off_flag :
			                                        alias_data_lst[i-baseCount].off_flag;

		/* is this an alias? */
		selData->events[i]->isAlias = (i < baseCount);

		/* sub event */
		if( ptr )
		{
		    selData->events[i]->subEvent = (char *)
			    XtMalloc(strlen(ptr+1)+1);
			
		    strcpy(selData->events[i]->subEvent, ptr+1);
		}
		else
		    selData->events[i]->subEvent = (char *) 0;

	    }
	    else
		MemoryError();
	} /* end for */
    } /* end else */

    XmSelectListTBFree(base_data_lst, baseCount);
    XmSelectListTBFree(alias_data_lst, aliasCount);

    return( AUDSuccess );

} /* end AUDCvtWidgetToSelData() */	
