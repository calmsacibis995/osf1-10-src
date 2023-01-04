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
static char *rcsid = "@(#)$RCSfile: AUDModDesel.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/20 21:31:44 $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <Mrm/MrmAppl.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <X11/X.h>

#include "TextList.h"

#include "XIsso.h"
#include "Utilities.h"
#include "AUD.h"

#define MAIN_WIDGET	"AUDModDeselFRM"

/* align on longword boundary */
#define ALIGN(to,from,type) \
    to = (type *)((long)from + (sizeof(long) - \
    ((long)from & (sizeof(long) -1))) % sizeof(long))

/* round val up to next longword */
#define RND(val) (val)&(sizeof(long)-1) ? ((val)&~(sizeof(long)-1))+sizeof(long) : (val)

#define RULE(i,x)       deselData->rules[i/NRULESETS]->x[i%NRULESETS]
#define ATOL(x)         strtoul(x,(char **)NULL,10)

/* state variables for screens */
Boolean formChanged         = False;
static Boolean addingNew    = False;

/* data for OKToChange */
static AUDLoseChangeData changeData;

/* current file under edit */
char *currentDeselFile = (char *) NULL;

/* external (eventually) variables */
Boolean	changedDeselList = False;

/* forward declarations */
void AUDModDeselDoItCallback();
void AUDFreeDeselectionData();
void AUDCvtDeselDataToWidget();

/*==================<UI-Level Support Routines/Callbacks>================*/

void
AUDRestoreCorrectDeselFileName(w, data, info)
Widget 			w;
AUDLoseChangeDataPtr	data;
XmAnyCallbackStruct 	*info;
{
    AUDModDeselPtr	pModDeselData;
    Widget		ancestor;

    ancestor = search_for_parent(data->w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if(pModDeselData)
	{
	    if( currentDeselFile != (char *) NULL )
	    {
		XmTextSetString(pModDeselData->ui.deselectionFileTL_W, 
				currentDeselFile);
	    }
	    else
		XmTextSetString(pModDeselData->ui.deselectionFileTL_W, "");
	}
    }
}



void
AUDModDeselOKToChangeCallback(w, data, info)
Widget 			w;
AUDLoseChangeDataPtr	data;
XmAnyCallbackStruct 	*info;
{
    formChanged = False;

    (*data->function)(data->w, data->flag, 0);
}



static void
AUDModDeselClearFields(pData, clearTL)
AUDModDeselUIPtr pData;
Boolean clearTL;
{
    if(pData)
    {
	/* File Text Field */
	if( clearTL && pData->deselectionFileTL_W )
	    XmTextSetString(pData->deselectionFileTL_W, "");

	/* editable text fields */
	if(pData->hostTF_W)
	    XmTextSetString(pData->hostTF_W, "");

	if(pData->auidTF_W)
	    XmTextSetString(pData->auidTF_W, "");

	if(pData->ruidTF_W)
	    XmTextSetString(pData->ruidTF_W, "");

	if(pData->eventTF_W)
	    XmTextSetString(pData->eventTF_W, "");

	if(pData->pathTF_W)
	    XmTextSetString(pData->pathTF_W, "");

	if(pData->flagTF_W)
	    XmTextSetString(pData->flagTF_W, "");

	/* Actual list of the file */
	if(pData->deselDispLST_W)
	    XmListDeleteAllItems(pData->deselDispLST_W);

	if(pData->updatePB_W)
	    XtSetSensitive(pData->updatePB_W, False);

	if(pData->removePB_W)
	    XtSetSensitive(pData->removePB_W, False);

    }
}



void
AUDModDeselCreateFormCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDModDeselPtr	pModDeselData;

    pModDeselData = (AUDModDeselPtr) XtMalloc(sizeof(AUDModDesel));

    if( pModDeselData )
    {
	bzero( (char *)pModDeselData, (int)sizeof(AUDModDesel) );
	pModDeselData->ui.form_W = w;
	SetUserData( w, pModDeselData );
    }
    else
    {
	MemoryError();
    }
}



void
AUDModDeselCreateWidgetsCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{

    Widget 		ancestor;
    AUDModDeselPtr	pModDeselData;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if( pModDeselData )
	{
	    switch (*flag)
	    {
	      case AUDDESEL_DESEL_LIST:
		pModDeselData->ui.deselectionFileTL_W = w;
		break;

	      case AUDDESEL_DESEL_DISP:
		pModDeselData->ui.deselDispLST_W = w;
		break;

	      case AUDDESEL_HOST_TEXT:
		pModDeselData->ui.hostTF_W = w;
		break;

	      case AUDDESEL_AUID_TEXT:
		pModDeselData->ui.auidTF_W = w;
		break;

	      case AUDDESEL_RUID_TEXT:
		pModDeselData->ui.ruidTF_W = w;
		break;

	      case AUDDESEL_EVENT_TEXT:
		pModDeselData->ui.eventTF_W = w;
		break;

	      case AUDDESEL_PATH_TEXT:
		pModDeselData->ui.pathTF_W = w;
		break;

	      case AUDDESEL_FLAG_TEXT:
		pModDeselData->ui.flagTF_W = w;
		break;

	      case AUDDESEL_OK_PB:
		pModDeselData->ui.OKPB_W = w;
		break;

	      case AUDDESEL_APPLY_PB:
		pModDeselData->ui.ApplyPB_W = w;
		break;

	      case AUDDESEL_CANCEL_PB:
		pModDeselData->ui.CancelPB_W = w;
		break;

	      case AUDDESEL_HELP_PB:
		pModDeselData->ui.HelpPB_W = w;
		break;

	      case AUDDESEL_DELETE_PB:
		pModDeselData->ui.deletePB_W = w;
		break;

	      case AUDDESEL_ADD_PB:
		pModDeselData->ui.addPB_W = w;
		break;

	      case AUDDESEL_UPDATE_PB:
		pModDeselData->ui.updatePB_W = w;
		break;

	      case AUDDESEL_REMOVE_PB:
		pModDeselData->ui.removePB_W = w;
		break;
	    }
	}
    }
}




void
AUDModDeselMapFormCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    XmString		xmstring;
    AUDModDeselPtr	pModDeselData;
    int			i;
    static int		firstTime = True;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if (pModDeselData)
	{
	    /*
	     * When we map the callback, the following must occur:
	     *
	     * 1) (re)load list of deselection files available
	     * 2) desensitize the delete button
	     * 3) clear all existing fields
	     */
	    if(AUDGetFileList(AUDIT_DESELDIR,
			      &audglobal.deselectionFiles,
			      &audglobal.numDeselectionFiles,
			      !firstTime, 
			      changedDeselList))
	    {		    
		PostErrorDialog("AUDmsg_err_reading_desel_file",
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
	    if( firstTime || changedDeselList )
	    {
		/* Delete current list */
		XmTextListDeleteAllItems(pModDeselData->ui.deselectionFileTL_W);

		for (i = 0; i < audglobal.numDeselectionFiles; i++)
		{
		    XmString	xmstring;

		    xmstring = XmStringCreate(audglobal.deselectionFiles[i],
					      charset);

		    XmTextListAddItem(pModDeselData->ui.deselectionFileTL_W,
				      xmstring,
				      0);
		    XmStringFree(xmstring);
		}
	    }

	    /* desensitize the "Delete" button */
	    XtSetSensitive(pModDeselData->ui.deletePB_W, False);

	    /* re-initialize all fields to whitespace */
	    AUDModDeselClearFields(&pModDeselData->ui, True);
		
	    /* no current changes */
	    formChanged = False;

	    /* OK, we are no longer a virgin */
	    firstTime = False;

	    /* we will need to keep notion of rev nos for each
	     * item here in the future when more screens use them.
	     */
	    changedDeselList = False;
	}
    }
}



void
AUDModDeselDoItCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData = (AUDModDeselPtr) NULL;
    int			i;
    
    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
	GetUserData(ancestor, pModDeselData);

    if( formChanged )
    {
	changeData.w        = w;
	changeData.flag     = 0;
	changeData.function = AUDModDeselDoItCallback;
 
	if( ancestor )
	    PostErrorDialog("AUDmsg_conf_cancel_data",
			    ancestor,
			    AUDModDeselOKToChangeCallback,
			    (XtPointer) &changeData,
			    AUDRestoreCorrectDeselFileName,
			    (XtPointer) &changeData,
			    NULL,
			    NULL);
	return;
    }
    else
    {
	if (pModDeselData)
	{
	    char 		*buf;

	    /* Cases:
	     *
	     * 1) User has selected an existing deselection file
	     * 2) User has typed a new selection file name
	     */

	    /* But first, we must update our notion of existing
	     * deselection files, since we are establishing a criterion
	     * that the deselection file must be on the list to be
	     * considered existing.  If, for example, a new file
	     * was just created and then selected, we wouldn't detect
	     * that it existed without first adding to our list.  If
	     * this is not the case, then the call will be a noop since
	     * nothing will have changed since the last call.
	     */

	    if(AUDGetFileList(AUDIT_DESELDIR, 
			      &audglobal.deselectionFiles,
			      &audglobal.numDeselectionFiles,
			      True,
			      changedDeselList))
	    {		    
		PostErrorDialog("AUDmsg_err_reading_desel_file",
				ancestor,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL);
		return;
	    }

	    buf = XmTextGetString(pModDeselData->ui.deselectionFileTL_W);

	    if(buf != (char *) NULL &&
	       strlen(buf) != 0)
	    {
		i = 0;
		while(i < audglobal.numDeselectionFiles &&
		      strcmp(audglobal.deselectionFiles[i],
			     buf))
		    i++;

		if( i < audglobal.numDeselectionFiles )
		{
		    /* modify existing */
		    AUDModDeselClearFields(&pModDeselData->ui, False);

		    /* Free up the data */
		    AUDFreeDeselectionData(&pModDeselData->data);

		    /* Load the data */
		    if( AUDReadDeselectionData(buf,
					       &pModDeselData->data) != AUDSuccess )
		    {
			/* ERROR */
			PostErrorDialog("AUDmsg_err_reading_desel_data",
					ancestor,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
			return;
		    }
			
		    AUDCvtDeselDataToWidget(&pModDeselData->data, 
					    &pModDeselData->ui);
			
		    addingNew = False;
			
		    /* sensitize the "Delete" button */
		    XtSetSensitive(pModDeselData->ui.deletePB_W, True);
		}
		else
		{
		    /* create new */
		    AUDModDeselClearFields(&pModDeselData->ui, False);
		    
		    addingNew = True;
		    
		    /* desensitize the "Delete" button */
		    XtSetSensitive(pModDeselData->ui.deletePB_W, False);
		}

		/* save this name as the "current" one */
		if( currentDeselFile )
		{
		    XtFree(currentDeselFile);
		    currentDeselFile = (char *) NULL;
		}

		currentDeselFile = (char *) XtMalloc(strlen(buf)+1);
		if(currentDeselFile)
		    strcpy(currentDeselFile, buf);
		else
		    MemoryError();

		XtFree(buf);
	    }
	    else
	    {
		/* Blank field */
		if( currentDeselFile )
		    XtFree(currentDeselFile);

		currentDeselFile = (char *) NULL;
		
		/* desensitize the "Delete" button */
		XtSetSensitive(pModDeselData->ui.deletePB_W, False);
	    }

	    /* initialize state */
	    formChanged = False;
	}
    }
}



int
AUDModDeselApplyChanges(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData;
    

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if (pModDeselData)
	{
	    int  i;
	    char *buf;

	    buf = XmTextGetString(pModDeselData->ui.deselectionFileTL_W);

	    if(buf != (char *) NULL &&
	       strlen(buf) != 0)
	    {
		/*
		 * At this point, we really might be adding a new deselection
		 * file even though the code has not yet detected it.
		 * So, we must compare the name against the list and
		 * make the final call.
		 */
		i = 0;
		while(i < audglobal.numDeselectionFiles &&
		      strcmp(audglobal.deselectionFiles[i],
			     buf))
		    i++;

		if( i == audglobal.numDeselectionFiles )
		{
		    addingNew   = True;
		    formChanged = True;
		}

		if( formChanged )
		{
		    if(AUDWriteDeselectionData(buf, &pModDeselData->ui))
		    {
			PostErrorDialog("AUDmsg_err_writing_desel_data",
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

		    XmTextListAddItem(pModDeselData->ui.deselectionFileTL_W,
				      xmstring,
				      0);
		    XmStringFree(xmstring);

		    changedDeselList = True;
		    addingNew        = False;
		}

		/* clean-up */
		AUDFreeDeselectionData(&(pModDeselData->data));
		XtFree(buf);
	    }
	}
    }
    return(AUDSuccess);
}





void
AUDModDeselCancel(w)
Widget	w;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if (pModDeselData)
	{
	    if(formChanged)
	    {
		changeData.w        = w;
		changeData.flag     = 0;
		changeData.function = AUDModDeselCancel;
 
		PostErrorDialog("AUDmsg_conf_cancel_data",
				ancestor,
				AUDModDeselOKToChangeCallback,
				(XtPointer) &changeData,
				NULL,
				NULL,
				NULL,
				NULL);
		return;
	    }

	    VUIT_Unmanage(MAIN_WIDGET);
	}
    }
}





void
AUDModDeselApplyCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{

    if(AUDModDeselApplyChanges(w, flag, info) == 0)
    {
	formChanged = False;
    }
}





void
AUDModDeselOKCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    if(AUDModDeselApplyChanges(w, flag, info) == 0)
    {
	formChanged = False;
    }
    else
	return;

    AUDModDeselCancel(w);
}




void
AUDModDeselCancelCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDModDeselCancel(w);
}



void
AUDModDeselDeleteFile(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData;
    char		filename[1024];

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if (pModDeselData)
	{
	    char 	*buf;
	    XmString	xmstring;

	    /* Get rid of the file!! */
	    buf = XmTextGetString(pModDeselData->ui.deselectionFileTL_W);

	    if(buf != (char *) NULL &&
	       strlen(buf) != 0)
	    {
		strcpy(filename, AUDIT_DESELDIR);
		strcat(filename, "/");
		strcat(filename, buf);

		if( unlink(filename) != 0 )
		{
		    PostErrorDialog("AUDmsg_err_removing_desel_file",
				    ancestor,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL);
		    return;
		}
		
		AUDModDeselClearFields(&pModDeselData->ui, True);

		xmstring = XmStringCreate(buf, charset);

		XmTextListDeleteItem(pModDeselData->ui.deselectionFileTL_W,
				     xmstring);

		changedDeselList = True;

		formChanged = False;

		/* desensitize the "Delete" button */
		XtSetSensitive(pModDeselData->ui.deletePB_W, False);

		XtFree(buf);
		XmStringFree(xmstring);
	    }
	}
    }
}



void
AUDModDeselDeleteFileCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData;
    

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	PostQuestionDialog("AUDmsg_conf_remove_desel_file",
			   ancestor,
			   AUDModDeselDeleteFile,
			   NULL,
			   NULL,
			   NULL,
			   NULL,
			   NULL);
    }
}

void
AUDModDeselSelectItemCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmListCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if (pModDeselData)
	{
	    char		*str;

            if( info->selected_item_count == 0 )
	    {
		XmTextSetString(pModDeselData->ui.hostTF_W , "");
		XmTextSetString(pModDeselData->ui.auidTF_W , "");
		XmTextSetString(pModDeselData->ui.ruidTF_W , "");
		XmTextSetString(pModDeselData->ui.eventTF_W, "");
		XmTextSetString(pModDeselData->ui.pathTF_W , "");
		XmTextSetString(pModDeselData->ui.flagTF_W , "");

		XtSetSensitive(pModDeselData->ui.updatePB_W, False);
		XtSetSensitive(pModDeselData->ui.removePB_W, False);
	    }
	    /* must be one since we were called and we only allow 1 selection */
	    else
	    {
		char *data[AUDDESEL_NUM_TUPLES];
		char *delimiters = { " \t\n" };
		char *token;
		int  i, count;

		XtSetSensitive(pModDeselData->ui.updatePB_W, True);
		XtSetSensitive(pModDeselData->ui.removePB_W, True);

		XmStringGetLtoR(info->item, XmSTRING_DEFAULT_CHARSET, &str);
		
		/* parse entries */
		token = strtok(str, delimiters);

		if(token)
		{
		    data[0] = (char *) XtMalloc(strlen(token) + 1);
		    strcpy(data[0], token);

		    count = 1;
		    while ((count < AUDDESEL_NUM_TUPLES) &&
			   (token = strtok((char *) 0, delimiters)) != (char *) 0)
		    {
			data[count] = (char *) XtMalloc(strlen(token) + 1);
			strcpy(data[count], token);
			count++;
		    }
		}
		XtFree(str);

		/* fill in the text fields */
		XmTextSetString(pModDeselData->ui.hostTF_W , data[0]);
		XmTextSetString(pModDeselData->ui.auidTF_W , data[1]);
		XmTextSetString(pModDeselData->ui.ruidTF_W , data[2]);
		XmTextSetString(pModDeselData->ui.eventTF_W, data[3]);
		XmTextSetString(pModDeselData->ui.pathTF_W , data[4]);
		XmTextSetString(pModDeselData->ui.flagTF_W , data[5]);

		for(i = 0; i < AUDDESEL_NUM_TUPLES; i++)
		    XtFree(data[i]);
	    }
	}
    }
}



void
AUDModDeselAddCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if (pModDeselData)
	{
	    Cardinal		n;
	    Arg			args[2];
	    int			itemCount;
	    int			*positionList[1];
	    int			positionCount;
	    int			position;
	    XmStringTable	items;
	    XmString		changedItem;
	    char		rule[MAX_RULE_SIZ];
	    char		*host, *auid, *ruid, *event,
		                *path, *flag;

	    n = 0;
	    XtSetArg(args[n], XmNselectedItemCount, &itemCount); n++;
	    XtSetArg(args[n], XmNselectedItems, &items); n++;
	    XtGetValues(pModDeselData->ui.deselDispLST_W, args, n); 
	    
	    if(itemCount == 1)
	    {
		XmListGetSelectedPos(pModDeselData->ui.deselDispLST_W,
				     positionList,
				     &positionCount);
	    }

	    /* obtain each field from the text fields */
	    host  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.hostTF_W));
	    auid  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.auidTF_W));
	    ruid  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.ruidTF_W));
	    event = StripWhiteSpace(XmTextGetString(pModDeselData->ui.eventTF_W));
	    path  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.pathTF_W));
	    flag  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.flagTF_W));
		
	    /* illegal string will default to a '*' */
	    if(strlen(flag) != 0 &&
	       strcmp(flag, "*") &&
	       strcmp(flag, "r") &&
	       strcmp(flag, "w") &&
	       strcmp(flag, "rw"))
	    {
		if(strcmp(flag, "read") == 0)
		    strcpy(flag, "r");	
		else if(strcmp(flag, "write") == 0)
		    strcpy(flag, "w");
		else if(strcmp(flag, "read,write") == 0)
		    strcpy(flag, "rw");
		else if (strcmp(flag, "read write") == 0)
		    strcpy(flag, "rw");
		else
		    strcpy(flag, "*");
	    }

	    /* build a string for the rule */
	    sprintf(rule, "%s\t%s\t%s\t%s\t%s\t%s",
		    (strlen(host)  != 0) ? host  : "*",
		    (strlen(auid)  != 0) ? auid  : "*",
		    (strlen(ruid)  != 0) ? ruid  : "*",
		    (strlen(event) != 0) ? event : "*",
		    (strlen(path)  != 0) ? path  : "*",
		    (strlen(flag)  != 0) ? flag  : "*");

	    changedItem = XmStringCreate(rule, charset);

	    position = (itemCount) ? *positionList[0] : 0;

	    XmListAddItem(pModDeselData->ui.deselDispLST_W,
			  changedItem,
			  position);

	    if(itemCount)
	    {
		XmListSelectPos(pModDeselData->ui.deselDispLST_W,
				position,
				True);
	    }
	    else
	    {
		XmTextSetString(pModDeselData->ui.hostTF_W, "");
		XmTextSetString(pModDeselData->ui.auidTF_W, "");
		XmTextSetString(pModDeselData->ui.ruidTF_W, "");
		XmTextSetString(pModDeselData->ui.eventTF_W, "");
		XmTextSetString(pModDeselData->ui.pathTF_W, "");
		XmTextSetString(pModDeselData->ui.flagTF_W, "");
	    }

	    if(host) 
		XtFree(host);
	    if(auid)
		XtFree(auid);
	    if(ruid) 
		XtFree(ruid);
	    if(event) 
		XtFree(event);
	    if(path) 
		XtFree(path);
	    if(flag) 
		XtFree(flag);

	    XmStringFree(changedItem);

	    formChanged = True;
	}
    }
}



void
AUDModDeselUpdateCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if (pModDeselData)
	{
	    Cardinal		n;
	    Arg			args[2];
	    int			itemCount;
	    XmStringTable	items;

	    n = 0;
	    XtSetArg(args[n], XmNselectedItemCount, &itemCount); n++;
	    XtSetArg(args[n], XmNselectedItems, &items); n++;
	    XtGetValues(pModDeselData->ui.deselDispLST_W, args, n); 
	    
	    if(itemCount == 1)
	    {
		XmString	changedItem;
		char		rule[MAX_RULE_SIZ];
		char		*host, *auid, *ruid, *event,
		                *path, *flag;

		/* obtain each field from the text fields */
		host  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.hostTF_W));
		auid  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.auidTF_W));
		ruid  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.ruidTF_W));
		event = StripWhiteSpace(XmTextGetString(pModDeselData->ui.eventTF_W));
		path  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.pathTF_W));
		flag  = StripWhiteSpace(XmTextGetString(pModDeselData->ui.flagTF_W));
		
		/* illegal string will default to a '*' */
		if(strlen(flag) != 0 &&
		   strcmp(flag, "*") &&
		   strcmp(flag, "r") &&
		   strcmp(flag, "w") &&
		   strcmp(flag, "rw"))
		{
		    if(strcmp(flag, "read") == 0)
			strcpy(flag, "r");	
		    else if(strcmp(flag, "write") == 0)
			strcpy(flag, "w");
		    else if(strcmp(flag, "read,write") == 0)
			strcpy(flag, "rw");
		    else if (strcmp(flag, "read write") == 0)
			strcpy(flag, "rw");
		    else
			strcpy(flag, "*");
		}

		/* build a string for the rule */
		sprintf(rule, "%s\t%s\t%s\t%s\t%s\t%s",
			(strlen(host)  != 0) ? host  : "*",
			(strlen(auid)  != 0) ? auid  : "*",
			(strlen(ruid)  != 0) ? ruid  : "*",
			(strlen(event) != 0) ? event : "*",
			(strlen(path)  != 0) ? path  : "*",
			(strlen(flag)  != 0) ? flag  : "*");

		changedItem = XmStringCreate(rule, charset);

		XmListReplaceItems(pModDeselData->ui.deselDispLST_W,
				   items,
				   itemCount,
				   &changedItem);

		XmListSelectItem(pModDeselData->ui.deselDispLST_W,
				 changedItem,
				 True);

		if(host) 
		    XtFree(host);
		if(auid)
		    XtFree(auid);
		if(ruid) 
		    XtFree(ruid);
		if(event) 
		    XtFree(event);
		if(path) 
		    XtFree(path);
		if(flag) 
		    XtFree(flag);

		XmStringFree(changedItem);

		formChanged = True;
	    }
	}
    }
}



void
AUDModDeselRemoveCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDModDeselPtr	pModDeselData;

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pModDeselData);

	if (pModDeselData)
	{
	    Cardinal		n;
	    Arg			args[2];
	    int			itemCount;
	    int			*positionList[1];
	    int			positionCount;
	    XmStringTable	items;

	    n = 0;
	    XtSetArg(args[n], XmNselectedItemCount, &itemCount); n++;
	    XtSetArg(args[n], XmNselectedItems, &items); n++;
	    XtGetValues(pModDeselData->ui.deselDispLST_W, args, n); 
	    
	    if(itemCount == 1)
	    {
		XmListGetSelectedPos(pModDeselData->ui.deselDispLST_W,
				     positionList,
				     &positionCount);

		XmListDeleteItem(pModDeselData->ui.deselDispLST_W,
				 items[0]);

		XmListSelectPos(pModDeselData->ui.deselDispLST_W,
				*positionList[0],
				True);

		formChanged = True;
	    }
	}
    }
}



/*=====================<Low-Level Support & I/O Routines>==================*/

int
AUDWriteDeselectionData( deselFile, deselUI )
char			*deselFile;
AUDModDeselUIPtr	deselUI;
{
    FILE		*fp;
    int			i;
    char		fileName[1024];
    Cardinal		n;
    Arg			args[2];
    int			itemCount;
    XmStringTable	items;
    time_t		tm;

    /* Open the file */
    strcpy(fileName, AUDIT_DESELDIR);
    strcat(fileName, "/");
    strcat(fileName, deselFile);

    if( (fp = fopen(fileName, "w")) == (FILE *) 0 )
	    return(-1);

    rewind( fp );

    /* Write out the file */
    time(&tm);
    fprintf(fp, "# This file generated by XIsso on %s\n",
	    ctime(&tm));

    fprintf(fp, "# HOST AUID RUID EVENT PATHNAME FLAG\n\n");

    n = 0;
    XtSetArg(args[n], XmNitemCount, &itemCount); n++;
    XtSetArg(args[n], XmNitems, &items); n++;
    XtGetValues(deselUI->deselDispLST_W, args, n); 

    for(i = 0; i < itemCount; i++)
    {
	char *data[AUDDESEL_NUM_TUPLES];
	char *delimiters = { " \t\n" };
	char *str, *token;
	int  j, count;

	XmStringGetLtoR(items[i], XmSTRING_DEFAULT_CHARSET, &str);
		
	/* parse entries */
	token = strtok(str, delimiters);

	if(token)
	{
	    data[0] = (char *) XtMalloc(strlen(token) + 1);
	    strcpy(data[0], token);

	    count = 1;
	    while ((count < AUDDESEL_NUM_TUPLES) &&
		   (token = strtok((char *) 0, delimiters)) != (char *) 0)
	    {
		data[count] = (char *) XtMalloc(strlen(token) + 1);
		strcpy(data[count], token);
		count++;
	    }
	}
	XtFree(str);

	for(j = 0; j < AUDDESEL_NUM_TUPLES; j++)
	{
	    /* XXX filter read, write */
	    fprintf(fp, "%s ", data[j]);
	    XtFree(data[j]);
	}
	fprintf(fp, "\n");
    }

    fflush(fp);
    fclose(fp);

    return(AUDSuccess);
}



int
AUDReadDeselectionData( deselFile, deselData )
char			*deselFile;
AUDModDeselDataPtr	deselData;
{
    char	fileName[1024];

    int lineno;
    char line[1024];
    char *ptr[6];
    FILE *fp;
    char *cp;
    int i, j, status;

    /* Open the file */
    strcpy(fileName, AUDIT_DESELDIR);
    strcat(fileName, "/");
    strcat(fileName, deselFile);

    if( (fp = fopen(fileName, "r")) == (FILE *)NULL )
	    return(1);

    /* *** The following logic was lifted from the audit_tool so as not to 
       re-invent the wheel.  It has been changed to store the data in 
       the data structure known to XIsso.
     */

    deselData->nrules = 0;

    status = AUDSuccess;

    /* read in rules */
    for ( lineno = 0;; lineno++ ) {

        /* read line (NOTE: fscanf doesn't allow variable width specification) */
        bzero ( line, 1024 );
        i = fscanf ( fp, "%1024[^\n]%*[\n]", line );
        if ( i == EOF ) break;

        /* ignore empty lines and comment lines */
        if ( i == 0 ) {
            fscanf ( fp, "%*[\n]", line );
            continue;
        }
        if ( line[0] == '#' ) continue;


        /* parse line into 6 strings
           allow for "old event" style names
        */
        for ( i = j = 0; i < 1024 && line[i] && j < 6; ) {
            if ( *(ptr[j++] = &line[i]) == '"' ) {
                ptr[j-1]++;
                for ( i++; i < 1024 && line[i] && line[i] != '"'; i++ );
                if ( i == 1024 ) break;
                line[i++] = '\0';
            }
            for ( ; i < 1024 && line[i] && line[i] != ' ' && line[i] != '\t'; i++ );
            if ( i < 1024 && line[i] ) line[i++] = '\0';
            for ( ; i < 1024 && line[i] && (line[i] == ' ' || line[i] == '\t'); i++ );
        }
        if ( j < 6 ) {
            continue;
        }


        /* allocate rules struct */
        if ( deselData->nrules%RULES_IN_SET == 0 ) {
            if ( (cp = (char *)malloc (RND(sizeof(AUDDeselectionData)) )) == NULL ) {
                MemoryError();
            }
            bzero ( cp, RND(sizeof(AUDDeselectionData)) );
            ALIGN ( deselData->rules[deselData->nrules/RULES_IN_SET], cp, AUDDeselectionData );
        }


        /* load hostname rule */
        for ( i = 0; i < MAXHOSTNAMELEN && ptr[0][i]; i++ );
        if ( (RULE(deselData->nrules,host) = (caddr_t)malloc(RND(i+1))) == NULL ) {
	    MemoryError();
        }
        bcopy ( ptr[0], RULE(deselData->nrules,host), i+1 );
        

        /* load audit_id rule */
        if ( ptr[1][0] == '*' ) RULE(deselData->nrules,auid) = -1;
        else RULE(deselData->nrules,auid) = ATOL(ptr[1]);


        /* load ruid rule */
        if ( ptr[2][0] == '*' ) RULE(deselData->nrules,ruid) = -1;
        else RULE(deselData->nrules,ruid) = ATOL(ptr[2]);


        /* load event rule */
        for ( i = 0; i < AUD_MAXEVENT_LEN && ptr[3][i]; i++ );
        if ( (RULE(deselData->nrules,event) = (caddr_t)malloc(RND(i+1))) == NULL ) {
	    MemoryError();
        }
        bcopy ( ptr[3], RULE(deselData->nrules,event), i+1 );
        for ( i = 0; i < sizeof(ptr[3]) && ptr[3][i] && ptr[3][i] != '.'; i++ );
        if ( ptr[3][i] == '.' ) {
            RULE(deselData->nrules,event)[i] = '\0';
            RULE(deselData->nrules,subevent) = &RULE(deselData->nrules,event)[i+1];
        }
        else RULE(deselData->nrules,subevent) = (char *)0;


        /* load param rule */
        for ( i = 0; i < MAX_RULE_SIZ && ptr[4][i]; i++ );
        if ( (RULE(deselData->nrules,param) = (caddr_t)malloc(RND(i+1))) == NULL ) {
	    MemoryError();
        }
        bcopy ( ptr[4], RULE(deselData->nrules,param), i+1 );


        /* load read/write ptr[5] rule */
	RULE(deselData->nrules,oprtn) = ( *ptr[5] == '*' ? -1 : 0 );
        for ( i = 0; i < sizeof(ptr[5]) && ptr[5][i]; i++ ) {
            if ( ptr[5][i] == 'r' ) RULE(deselData->nrules,oprtn) += 1;
            else if ( ptr[5][i] == 'w' ) RULE(deselData->nrules,oprtn) += 2;
        }
	if ( RULE(deselData->nrules,oprtn) != -1 ) RULE(deselData->nrules,oprtn)--;

        /* max # rules hit */
        if ( ++deselData->nrules == RULES_IN_SET * NRULESETS ) {
	    status = 2;
            break;
        }
    }

    fclose(fp);
    return(status);
}



void
AUDCvtDeselDataToWidget( deselData, deselUI )
AUDModDeselDataPtr	deselData;
AUDModDeselUIPtr	deselUI;
{
    int		i;
    XmString	item;
    char	rule[MAX_RULE_SIZ];
    char        subevent[AUD_MAXEVENT_LEN];
    char	auid[7];
    char	ruid[7];
    char        oprtn[12];
    char	*options[4] = { "*", "r", "w", "rw" };

    /* build a list of items from the data structure */
    for(i = 0; i < deselData->nrules; i++)
    {
	if(RULE(i,subevent) != (char *) NULL)
	{
	    strcpy(subevent, ".");
	    strcat(subevent, RULE(i,subevent));
	}
	else
	{
	    strcpy(subevent, "");
	}

	if(RULE(i, auid) == -1)
	    strcpy(auid, AUDDESEL_ALL);
	else
	    sprintf(auid, "%d", RULE(i, auid));

	if(RULE(i, ruid) == -1)
	    strcpy(ruid, AUDDESEL_ALL);
	else
	    sprintf(ruid, "%d", RULE(i, ruid));

	if((RULE(i, oprtn) >= -1) && (RULE(i, oprtn) < 3))
	    strcpy(oprtn, options[RULE(i, oprtn)+1]);
	else
	    strcpy(oprtn, AUDDESEL_ALL);

	/* build a string for the rule */
	sprintf(rule, "%s\t%s\t%s\t%s%s\t%s\t%s",
		RULE(i, host),
		auid,
		ruid,
		RULE(i, event),
		subevent,
		RULE(i, param),
		oprtn);
	
	item = XmStringCreate(rule, charset);

	XmListAddItem(deselUI->deselDispLST_W,
		      item,
		      0);

	XmStringFree(item);
    }
}



void
AUDFreeDeselectionData(deselData)
AUDModDeselDataPtr	deselData;
{
    int i;

    for(i = 0; i < deselData->nrules; i++)
    {
	if(RULE(i,host))
	{
	    free(RULE(i,host));
	    RULE(i,host) = (char *) NULL;
	}

	if(RULE(i,event))
	{
	    free(RULE(i,event));
	    RULE(i,event) = (char *) NULL;
	}

	if(RULE(i,param))
	{
	    free(RULE(i,param));
	    RULE(i,param) = (char *) NULL;
	}
    }

    for(i = 0; i <= deselData->nrules/RULES_IN_SET; i++)
    {
	if(deselData->rules[i])
	{
	    free(deselData->rules[i]);
	    deselData->rules[i] = (AUDDeselectionDataPtr) NULL;
	}
    }

    deselData->nrules = 0;
}
