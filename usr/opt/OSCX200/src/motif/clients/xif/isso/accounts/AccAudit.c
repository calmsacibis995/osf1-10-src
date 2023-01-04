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
static char *rcsid = "@(#)$RCSfile: AccAudit.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/10/13 19:37:44 $";
#endif

/*******************************************************************************
 *
 *
 * Assumptions:
 *
 *
 ******************************************************************************/

#include <stdio.h>                      /* For printf and so on. */
#include <pwd.h>
#include <grp.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Mrm/MrmAppl.h>                /* Motif Toolkit and MRM */
#include <X11/X.h>
#include <sys/audit.h>
#include "../isso/XIsso.h"
#include "XMain.h"
#include "XAccounts.h"
#include "Utilities.h"
#include "Resources.h"
#include "../isso/accounts/AccountSelection.h"
#include "../isso/accounts/AccCommon.h"
#include "Messages.h"
#include "SListTB.h"

/* The vuit generated application include file.				    */
/* If the user does not specify a directory for the file		    */
/* then the vaxc$include logical needs to be defined to point to the	    */
/* directory of the include file.					    */
#define REF_TYPE
#include "Vuit.h"

/******************************************************************************
 * Local Defines      
 ******************************************************************************/
/* Also defined in AccUtils.c...perhaps we should have a common .h file */
#define NUM_TEMPLATE_ACCOUNTS  20 

/* Control Mask Toggle Button indexs */
#define AND_TB  0 
#define OR_TB   1 
#define USER_TB 2
#define OFF_TB  3

#define NUM_TBS 4

static char audit_control_masks[4] = {AUDIT_OR,AUDIT_AND,AUDIT_OFF,AUDIT_USR};

static int parent_type;
/******************************************************************************
 * Static Global data
 ******************************************************************************/

/*
 * Set the top level widget to NULL to distinguish between having to Open
 * the Account Window and create the shell vs Close, Unrealize, Open again 
 * and merely re-Realize the Shell
*/
static Widget acc_audit_events_tl=NULL;
static Widget acc_audit_fd=NULL;
static Widget name_tx;
/* static XmSelectListTBWidget base_sl,alias_sl; */
static Widget parentWidget;
static Widget processControlTB[4];
static Widget select_all_pb,select_none_pb;
static Widget ok_pb,apply_pb,cancel_pb,help_pb;

static AccountDataType *pWidgetRetrievalAccData;

/* Initial Audit Control State so we can tell if it has been set */
static int AuditControlState = -1;

/*
 * Forward Callback declarations
 */
void AccAuditCloseCB();
void AccAuditDoCloseCB();
void AccAuditSaveCB();
void AccAuditDoSaveCB();
void AccAuditDoSaveNCloseCB();
void AccAuditSaveNCloseCB();
void AccAuditGetWidgetCB();
void AccAuditSelectAllCB();
void AccAuditSelectNoCB();

/*
 * Forward declarations
 */
void AccAuditSave();
void AccCloseAuditEventsScreen();

/*
 * Names and addresses of callback routines to register with Mrm
 */
static MrmRegisterArg reglist [] = {
{"AccAuditGetWidgetCB",        (caddr_t)AccAuditGetWidgetCB},
{"AccAuditSelectAllCB",        (caddr_t)AccAuditSelectAllCB},
{"AccAuditSelectNoCB",         (caddr_t)AccAuditSelectNoCB},
{"AccAuditCloseCB",            (caddr_t)AccAuditCloseCB},
{"AccAuditSaveCB",             (caddr_t)AccAuditSaveCB},
{"AccAuditSaveNCloseCB",       (caddr_t)AccAuditSaveNCloseCB}};

Boolean changedBaseEvents;
Boolean changedSiteEvents;
Boolean changedAliasEvents;
char    **baseEventList;
char    **siteEventList;
char    **aliasEventList;
int     baseEventCount;
int     siteEventCount;
int     aliasEventCount;

/*
==============================================================================
= P U B L I C   R O U T I N E S   T O   S U P P O R T   U S E R   A U D I T  
============================================================================== 
*/

/*
==============================================================================
= S T A T I C   R O U T I N E S   T O   S U P P O R T   U S E R   A U D I T
============================================================================== 
*/

/******************************************************************************
 * AssignTabGroups()
 ******************************************************************************/
static void AssignTabGroups()

{
int i;

XmSelListTBAddTabGroup(pWidgetRetrievalAccData->base_sl);
XmSelListTBAddTabGroup(pWidgetRetrievalAccData->alias_sl);
XmAddTabGroup(select_all_pb);
XmAddTabGroup(select_none_pb);

for (i=0; i<NUM_TBS; i++)
    XmAddTabGroup(processControlTB[i]);

XmAddTabGroup(ok_pb);
XmAddTabGroup(apply_pb);
XmAddTabGroup(cancel_pb);
XmAddTabGroup(help_pb);
}
/******************************************************************************
 * AuditEventsScreenIsOpen()
 ******************************************************************************/
static Boolean AuditEventsScreenIsOpen()

{
if (acc_audit_events_tl)
  return(True);
return(False);
}
/******************************************************************************
 * AuditEventsNAliasesToSegments()
 ******************************************************************************/
static void AuditEventsNAliasesToSegments(audit_str,data_lst,seg_count)
char          *audit_str;
SListTBType  **data_lst; 
int           *seg_count;

{
char *ptr;
char *audit_str_ptr;
char *audit_str_tmp;
char *colon_ptr;

*seg_count = 0;

if ((audit_str_tmp = (char *) XtMalloc(strlen(audit_str) + 1)) == NULL)
   {
   /* Memory Error */
   exit(0);
   }

/* copy the audit string as we will be altering it */
strcpy(audit_str_tmp,audit_str);

audit_str_ptr = &(audit_str_tmp[0]); 

/* Set ptr to the beginning of audit_str_ptr */
ptr = audit_str_ptr;

/* field is NULL terminated??? */
/* look for , as delimeters, some event names have spaces so that won't do */
while ((audit_str_ptr = strpbrk(audit_str_ptr,",")) != (char *)NULL)
      {
      (*seg_count)++;

      audit_str_ptr[0] = '\0';
      *data_lst = (SListTBType *)
          XtRealloc((char *)*data_lst, sizeof(SListTBType) * *seg_count);

      if (*data_lst)
         {
         (*data_lst)[(*seg_count)-1].name = (char *) XtMalloc(strlen(ptr) + 1);
         strcpy((*data_lst)[(*seg_count)-1].name,ptr);

         colon_ptr = strpbrk( (*data_lst)[(*seg_count)-1].name, ":");
         colon_ptr[0] = '\0';

         /* assign  the on off flags :1:1 from the name */
         if (colon_ptr[1] == '1')
            (*data_lst)[(*seg_count)-1].on_flag = 1;
         else (*data_lst)[(*seg_count)-1].on_flag = 0;

         if (colon_ptr[3] == '1')
            (*data_lst)[(*seg_count)-1].off_flag = 1;
         else (*data_lst)[(*seg_count)-1].off_flag = 0;
         }
      else
         {
         /* MemoryError() */
         exit(0);
         }

      /* Set ptr to the beginning of the next segment */
      ptr = &(audit_str_ptr[1]);

      /* increment audit_str_ptr to pass the NULL */
      audit_str_ptr = ptr;
      }

/* Handle the last segment if there is one */
if ((ptr != NULL) && (strcmp(ptr,"") != 0))
   {
   (*seg_count)++;

   *data_lst = (SListTBType *) XtRealloc((char *)*data_lst,
                                          sizeof(SListTBType) * *seg_count);
   if (*data_lst)
      {
      (*data_lst)[(*seg_count)-1].name = (char *) XtMalloc(strlen(ptr) + 1);
      strcpy((*data_lst)[(*seg_count)-1].name,ptr);

      colon_ptr = strpbrk( (*data_lst)[(*seg_count)-1].name, ":");
      colon_ptr[0] = '\0';

      /* assign the on off flags :1:1 from the name */
      if (colon_ptr[1] == '1')
         (*data_lst)[(*seg_count)-1].on_flag = 1;
      else (*data_lst)[(*seg_count)-1].on_flag = 0;

      if (colon_ptr[3] == '1')
         (*data_lst)[(*seg_count)-1].off_flag = 1;
      else (*data_lst)[(*seg_count)-1].off_flag = 0;
      }
   else
      {
      /* MemoryError() */
      exit(0);
      }
   }
XtFree(audit_str_tmp);
}
/******************************************************************************
 * DisplayAccAuditEventsAndAliases()
 ******************************************************************************/
static void DisplayAccAuditEventsAndAliases(audit_str)
char *audit_str;

{
int           i,j,k;
int           seg_count,alias_count,base_count;
SListTBType  *data_lst=NULL;
SListTBType  *alias_data_lst=NULL;
SListTBType  *base_data_lst=NULL;

/* Extract the string of events into a list */
AuditEventsNAliasesToSegments(audit_str,&data_lst,&seg_count);

alias_data_lst = (SListTBType * ) XtMalloc(sizeof(SListTBType) * seg_count);

base_data_lst = (SListTBType * ) XtMalloc(sizeof(SListTBType) * seg_count);

/* Add the Events/Aliases to the appropriate selection boxes */
j=0;
k=0;
for (i=0; i<seg_count; i++)
    {
    if (XmAvailableListTBItemExists(pWidgetRetrievalAccData->base_sl,data_lst[i].name))
       {
       base_data_lst[j].name = data_lst[i].name;
       base_data_lst[j].on_flag = data_lst[i].on_flag;
       base_data_lst[j].off_flag = data_lst[i].off_flag;
       j++;
       }
    else if (XmAvailableListTBItemExists(pWidgetRetrievalAccData->alias_sl,data_lst[i].name))
       {
       alias_data_lst[k].name = data_lst[i].name;
       alias_data_lst[k].on_flag = data_lst[i].on_flag;
       alias_data_lst[k].off_flag = data_lst[i].off_flag;
       k++;
       }
    else
       {
       printf("Unknown event *%s*\n",data_lst[i].name);
       }
    }

XmSelectListTBSetItemsSelected(pWidgetRetrievalAccData->base_sl,base_data_lst,j);

XmSelectListTBSetItemsSelected(pWidgetRetrievalAccData->alias_sl,alias_data_lst,k);

/* Free Up the Audit String Segment List and names */
XmSelectListTBFree(data_lst,seg_count);

/* Free only the lst and not the name items as they were freed w/ above */
XtFree((char *)base_data_lst);
XtFree((char *)alias_data_lst);
}
/******************************************************************************
* AccAuditSave()
******************************************************************************/
static void AccAuditSave()

{
int           i;
int           event_count,alias_count;
char         *ptr;
SListTBType  *data_lst; 
struct pr_passwd *prpwd = &(pWidgetRetrievalAccData->userInfo->userData.prpw);

/* Extract the list of Events/Aliases and write to prpwd field */

XmSelectListTBGetItems(pWidgetRetrievalAccData->base_sl,&data_lst,&event_count);
prpwd->uflg.fg_auditdisp = 1;

ptr=&(prpwd->ufld.fd_auditdisp[0]);

for (i=0; i<event_count; i++)
    {
    sprintf(ptr,"%s",data_lst[i].name);
    ptr+=strlen(data_lst[i].name);
    sprintf(ptr,":%d:%d,",data_lst[i].on_flag,data_lst[i].off_flag);
    ptr+=5;
    }

XmSelectListTBFree(data_lst,event_count);

XmSelectListTBGetItems(pWidgetRetrievalAccData->alias_sl,&data_lst,&alias_count);

for (i=0; i<alias_count; i++)
    {
    sprintf(ptr,"%s",data_lst[i].name);
    ptr+=strlen(data_lst[i].name);
    sprintf(ptr,":%d:%d,",data_lst[i].on_flag,data_lst[i].off_flag);
    ptr+=5;
    }
if ((alias_count+event_count) > 0)
   ptr--;
ptr[0] = '\0';

XmSelectListTBFree(data_lst,alias_count);

/* Get the Audit Control State */
for (i=0; i<NUM_TBS; i++)
    if (XmToggleButtonGetState(processControlTB[i]))
       if (AuditControlState != audit_control_masks[i])
          {
          /* To optimize, might want to check if this is a system setting */
          prpwd->uflg.fg_auditcntl = 1;
          prpwd->ufld.fd_auditcntl = audit_control_masks[i];
          }
}
/******************************************************************************
* InitializeAccountAuditScreen()
******************************************************************************/
static void InitializeAccountAuditScreen(parent,pAccData,name,type)
Widget           parent;
AccountDataType *pAccData;
char            *name;
int              type;

{
XmString     xmstring;
SListTBType *data_lst; 
int          i,j,n;
int          state;
static int   firstTime = True;
/* struct pr_passwd *prpwd = &(pAccData->userInfo->userData.prpw); */
struct pr_passwd *prpwd;

prpwd = &(pAccData->userInfo->userData.prpw);

/* Save the parent widget for future Dialogs and the Data Structure Pointer */
parentWidget = parent;
pWidgetRetrievalAccData = pAccData;

/*
 * The screen is modal and must be closed before moving on so always open it
 */
parent_type = type;
if (parent_type == ACC_USERS)
   NEW_VUIT_Manage("AccountAuditEventsFD",&acc_audit_fd,parent);
else
   NEW_VUIT_Manage("AccTmpltAuditEventsFD",&acc_audit_fd,parent);
XmTextFieldSetString(name_tx,name);

/*
 * Load the scrolled lists
 */
if (AUDGetBaseEventList(&baseEventList,&baseEventCount,!firstTime,
                        changedBaseEvents) ||
    AUDGetSiteEventList(&siteEventList,&siteEventCount,0,!firstTime,
                        changedSiteEvents))
   {
   /* ERROR */
   PostErrorDialog("AUDmsg_err_reading_events_file",
                   parent,NULL,NULL,NULL,NULL,NULL,NULL);
   return;
   }

data_lst = (SListTBType *) XtMalloc(sizeof(SListTBType) * baseEventCount);
if (data_lst == NULL)
   {
   /* Memory Error */
   exit(0);
   }

/* load the widget */
for (i = 0; i < baseEventCount; i++)
    {
    data_lst[i].name = (char *) XtMalloc(strlen(baseEventList[i]) + 1);
    strcpy(data_lst[i].name, baseEventList[i]);
    data_lst[i].on_flag = 1;
    data_lst[i].off_flag = 1;
    }
XmAvailableListTBAddItems(pWidgetRetrievalAccData->base_sl,data_lst,baseEventCount,0);

XmSelectListTBFree(data_lst,baseEventCount);

data_lst = (SListTBType *) XtMalloc(sizeof(SListTBType) * siteEventCount);
if (data_lst == NULL)
   {
   /* Memory Error */
   exit(0);
   }

for (i=baseEventCount,j=0; i<(baseEventCount + siteEventCount); i++,j++)
    {
    data_lst[j].name = (char *) XtMalloc(strlen(siteEventList[j]) + 1);
    strcpy(data_lst[j].name, siteEventList[j]);
    data_lst[j].on_flag = 1;
    data_lst[j].off_flag = 1;
    }
XmAvailableListTBAddItems(pWidgetRetrievalAccData->base_sl,data_lst,siteEventCount,0);

XmSelectListTBFree(data_lst,siteEventCount);

if (AUDGetAliasEventList(&aliasEventList,&aliasEventCount,!firstTime,
                         changedAliasEvents))
   {
   /* ERROR */
   PostErrorDialog("AUDmsg_err_reading_alias_file",
                   parent,NULL,NULL,NULL,NULL,NULL,NULL);
   return;
   }

data_lst = (SListTBType *) XtMalloc(sizeof(SListTBType) * aliasEventCount);
if (data_lst == NULL)
   {
   /* Memory Error */
   exit(0);
   }

for (i = 0; i < aliasEventCount; i++ )
    {
    data_lst[i].name = (char *) XtMalloc(strlen(aliasEventList[i]) + 1);
    strcpy(data_lst[i].name, aliasEventList[i]);
    data_lst[i].on_flag = 1;
    data_lst[i].off_flag = 1;
    }

XmAvailableListTBAddItems(pWidgetRetrievalAccData->alias_sl,data_lst,aliasEventCount,0);

XmSelectListTBFree(data_lst,aliasEventCount);

/* Initialize the Audit Control State every time through */
AuditControlState = -1;

/* Set The Audit Control Mask */
for (i=0; i<NUM_TBS; i++)
    {
    /* Set the Audit Process Control Flags appropriately */
    state = False;

    if (prpwd->uflg.fg_auditcntl)
       {
       if (prpwd->ufld.fd_auditcntl == audit_control_masks[i])
          {
          AuditControlState = audit_control_masks[i];
          state = True;
          }
       }
    else
       {
       if (prpwd->sflg.fg_auditcntl) 
          {
          if (prpwd->sfld.fd_auditcntl == audit_control_masks[i])
             {
             AuditControlState = audit_control_masks[i];
             state = True;
             }
          }
       }
    XmToggleButtonSetState(processControlTB[i], state);
    }

/* Set the events and aliases */
if (strcmp(SYSTEM_DEFAULT_TEMPLATE,name) == 0)
   DisplayAccAuditEventsAndAliases(pAccData->dflt.auditdisp);
else 
   if (prpwd->uflg.fg_auditdisp)
      DisplayAccAuditEventsAndAliases(prpwd->ufld.fd_auditdisp);
else
    if (prpwd->sflg.fg_auditdisp) 
       DisplayAccAuditEventsAndAliases(prpwd->sfld.fd_auditdisp);

/* Initialize the Tab Groups */
AssignTabGroups();
}

/*
==============================================================================
= C A L L B A C K   F U N C T I O  N S
==============================================================================
*/

/******************************************************************************
* AccAuditDoSaveNCloseCB()
******************************************************************************/
void AccAuditDoSaveNCloseCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
AccAuditSave();

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_audit",acc_audit_fd,
                      AccAuditDoCloseCB,NULL,NULL,NULL,NULL,NULL);
else
   AccCloseAuditEventsScreen();
}        
/******************************************************************************
* AccAuditDoSaveCB()
******************************************************************************/
void AccAuditDoSaveCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
AccAuditSave();
}        
/******************************************************************************
* AccAuditDoCloseCB()
******************************************************************************/
void AccAuditDoCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{

AccCloseAuditEventsScreen();
}
/******************************************************************************
* AccAuditCloseCB()
******************************************************************************/
void AccAuditCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason; 
{ 
if (ApplicationResourcesInfo.questionAllActions) 
   PostQuestionDialog("msg_acc_close_audit",acc_audit_fd, 
                      AccAuditDoCloseCB,NULL,NULL,NULL,NULL,NULL); 
else AccCloseAuditEventsScreen(); 
} 
/******************************************************************************
* AccAuditSaveCB()
******************************************************************************/
void AccAuditSaveCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{
if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_store_audit_changes",acc_audit_fd,
                      AccAuditDoSaveCB,NULL,NULL,NULL,NULL,NULL);
else
   AccAuditSave();
}
/******************************************************************************
 * AccAuditSaveNCloseCB()
 ******************************************************************************/
void AccAuditSaveNCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_store_audit_changes",acc_audit_fd,
         	      AccAuditDoSaveNCloseCB,NULL,NULL,NULL,NULL,NULL);
else
   {
   AccAuditSave();
   AccCloseAuditEventsScreen();
   }
}
/******************************************************************************
 * AccAuditSelectAllCB()       
 ******************************************************************************/
void AccAuditSelectAllCB (w, tag, reason)
Widget          w;
int             *tag;
unsigned long   *reason;
{
if (pWidgetRetrievalAccData->base_sl)
   XmSelectListTBSetAllItemsSelected(pWidgetRetrievalAccData->base_sl);

if (pWidgetRetrievalAccData->alias_sl)
   XmSelectListTBSetAllItemsSelected(pWidgetRetrievalAccData->alias_sl);
}
/******************************************************************************
 * AccAuditSelectNoCB()       
 ******************************************************************************/
void AccAuditSelectNoCB (w, tag, reason)
Widget          w;
int             *tag;
unsigned long   *reason;
{
if (pWidgetRetrievalAccData->base_sl)
   XmSelectListTBSetAllItemsAvailable(pWidgetRetrievalAccData->base_sl);

if (pWidgetRetrievalAccData->alias_sl)
   XmSelectListTBSetAllItemsAvailable(pWidgetRetrievalAccData->alias_sl);
}
/******************************************************************************
 * AccAuditGetWidgetCB()       
 ******************************************************************************/
void AccAuditGetWidgetCB(w,widget_id,reason)
Widget	       w;
int	      *widget_id;
unsigned long *reason;

{
int         i;
Arg         args[10];

switch (*widget_id)
{
case ACC_AUD_NAME:
        name_tx = w;
     break;
case ACC_AUD_AND_MASK:
        processControlTB[AND_TB] = w;
     break;
case ACC_AUD_OR_MASK:
        processControlTB[OR_TB] = w;
     break;
case ACC_AUD_USER_MASK:
        processControlTB[USER_TB] = w;
     break;
case ACC_AUD_OFF_MASK:
        processControlTB[OFF_TB] = w;
     break;
case ACC_AUD_BASE_EVENTS:
        pWidgetRetrievalAccData->base_sl = w;
     break;
case ACC_AUD_ALIAS_EVENTS:
        pWidgetRetrievalAccData->alias_sl = w;
     break;
case ACC_AUD_OK_PB:
        ok_pb = w;
     break;
case ACC_AUD_APPLY_PB:
        apply_pb = w;
     break;
case ACC_AUD_CANCEL_PB:
        cancel_pb = w;
     break;
case ACC_AUD_HELP_PB:
        help_pb = w;
     break;
case ACC_AUD_SELECT_NONE_PB:
        select_none_pb = w;
     break;
case ACC_AUD_SELECT_ALL_PB:
        select_all_pb = w;
     break;

default : printf("Got Unknown Widget ID %d\n",*widget_id);
}
}

/*
==============================================================================
= E N T R Y   P O I N T   T O   M O D I F Y   U S E R   A C C O U N T S
============================================================================== 
*/

/******************************************************************************
 * AccOpenAuditEventsScreen()
 ******************************************************************************/
void AccOpenAuditEventsScreen(pAccData,name,type,parent)
AccountDataType *pAccData;
char            *name;
int              type;
Widget           parent;

{
MrmRegisterNames (reglist,(sizeof reglist/sizeof reglist[0]));

if (type == ACC_USERS)
   {
   if (! XmTextListItemExists(pAccData->usernameText,name))
      {
      PostErrorDialog("msg_error_user_aud_bad_account",parent,NULL,
                      NULL,NULL,NULL,NULL,NULL);
      return;
      }
   }
else if (type == ACC_TEMPLATES)
   {
   if (! XmTextListItemExists(pAccData->tmpltnameText,name))
      {
      PostErrorDialog("msg_error_tmplt_aud_bad_account",parent,NULL,
                      NULL,NULL,NULL,NULL,NULL);
      return;
      }
   }

InitializeAccountAuditScreen(parent,pAccData,name,type);
}
/******************************************************************************
 * AccCloseAuditEventsScreen()
 ******************************************************************************/
void AccCloseAuditEventsScreen()
{

if (parent_type == ACC_USERS)
   VUIT_Unmanage("AccountAuditEventsFD");
else 
   VUIT_Unmanage("AccTmpltAuditEventsFD");

/* Remove all items form Base/Site Event List */
XmSelectListTBClean(pWidgetRetrievalAccData->base_sl);

/* Remove all items form Alias Event List */
XmSelectListTBClean(pWidgetRetrievalAccData->alias_sl);

}
