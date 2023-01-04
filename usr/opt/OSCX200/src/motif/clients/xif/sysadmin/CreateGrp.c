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
static char *rcsid = "@(#)$RCSfile: CreateGrp.c,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/12/20 21:32:43 $";
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
#include "../isso/XIsso.h"
#include "XMain.h"
#include "XAccounts.h"
#include "Utilities.h"
#include "Resources.h"
#include "../isso/accounts/AccountSelection.h"
#include "../isso/accounts/AccCommon.h"
#include "Messages.h"

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

/******************************************************************************
 * Static Global data
 ******************************************************************************/

/*
 * Set the top level widget to NULL to distinguish between having to Open
 * the Account Window and create the shell vs Close, Unrealize, Open again 
 * and merely re-Realize the Shell
 */
static Widget grp_create_tl=NULL;
static Widget create_grp_mw=NULL;
static Widget template_pdm;

static Widget template_pb[NUM_TEMPLATE_ACCOUNTS];
Widget acctmplt_mw; /* Temporarily For Compatibility with AccCommon.c */

static AccountDataType AccData;

/*
 * Forward Callback declarations
 */
void CreateGrpCloseCB();
void CreateGrpSaveCB();
void CreateGrpSaveNCloseCB();
void CreateGrpSelectGrpCB();
void CreateGrpGetWidgetCB();

void CreateGroupCB();
void CloseCreateGrpCB();

/*
 * Forward declarations
 */
void CloseCreateGroup();

/*
 * Names and addresses of callback routines to register with Mrm
 */
static MrmRegisterArg reglist [] = {
{"CreateGrpCloseCB",             (caddr_t)CreateGrpCloseCB},
{"CreateGrpSelectGrpCB",         (caddr_t)CreateGrpSelectGrpCB},
{"CreateGrpSaveCB",              (caddr_t)CreateGrpSaveCB},
{"CreateGrpSaveNCloseCB",        (caddr_t)CreateGrpSaveNCloseCB},
{"CreateGrpGetWidgetCB",         (caddr_t)CreateGrpGetWidgetCB}};

/*
 ==============================================================================
 = P R I V A T E   R O U T I N E S   T O   S U P P O R T   G R O U P S 
 ============================================================================== 
*/

/******************************************************************************
 * CreateGrpLoadPriGroupTXList()
 ******************************************************************************/
static void CreateGrpLoadPriGroupTXList()

{
LoadPriGroupTXList(AccData.priGroups);
}
/******************************************************************************
 * AccountAssignAndDisplayGID()
 ******************************************************************************/
static void AccountAssignAndDisplayGID(gid)
int gid;

{
char           temp[10];

SET_TEXT_FROM_NUMBER(temp, gid, AccData.gidText);
}

/******************************************************************************
 * WriteNewGroup()
 ******************************************************************************/
static int WriteNewGroup(name, id)
char *name;
int   id;

{
int    ret;
struct group newgroup;


/* Create a new group */
newgroup.gr_name = name;
newgroup.gr_passwd = "";
newgroup.gr_gid = id;
newgroup.gr_mem = (char **) 0;

ret = XCreateGroup(&newgroup);

return(ret == SUCCESS);
}
/******************************************************************************
 * ValidateGroupInfo()
 ******************************************************************************/
static int ValidateGroupInfo(name, id)
char *name;
int   id;
{

/* Must be able to access the password file */
if (CheckGroupAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkgrp_no_access",create_grp_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

/* Check name is valid, no ":" */
if (CheckValidGroup(name) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkgrp_invalid_group",create_grp_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

/* Check if name already exists */
if (GroupNameExists(name) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkgrp_group_exists",create_grp_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

/* Check the group number is in a valid range */
if (CheckValidGid(id) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkgrp_invalid_gid",create_grp_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

/* Check the id is unique */
if (GroupIdExists(id) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkgrp_gid_exists",create_grp_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

return(True);
}
/******************************************************************************
 * CreateGroup()
 ******************************************************************************/
static Boolean CreateGroup()

{
char group_name[100];
char group_id[10];
int  gid;
 
PostWorkingDialog("msg_acc_working_create_group",create_grp_mw,
                  NULL,NULL,NULL,NULL,NULL,NULL);

/* Retrieve Data from graphic widgets */
strcpy(group_name,XmTextFieldGetString(AccData.priGroups));
if (removeBlanks(group_name))
      XmTextSetString(AccData.priGroups,group_name);

strcpy(group_id,XmTextFieldGetString(AccData.gidText));
if (removeBlanks(group_id))
      XmTextSetString(AccData.gidText,group_id);

gid = atoi(group_id);

if (! ValidateGroupInfo(group_name, gid))
   {
   UnPostWorkingDialog(create_grp_mw);

   return(False);
   }

if (! WriteNewGroup(group_name, gid))
   {
   /* WriteNewGroup does not currently trap any errors */
   UnPostWorkingDialog(create_grp_mw);

   return(False);
   }

/* Reload the group list because a new one now exists */
LoadPriGroupTXList(AccData.priGroups);

UnPostWorkingDialog(create_grp_mw);

return(True);
}
/******************************************************************************
 * AccountModifyScreenIsOpen()
 ******************************************************************************/
static Boolean AccountModifyScreenIsOpen()

{
if (grp_create_tl)
   return(True);
return(False);
}

/*
 ==============================================================================
 = S T A T I C   R O U T I N E S   T O   S U P P O R T   G R O U P S     
 ============================================================================== 
*/

/*
 ==============================================================================
 = C A L L B A C K   F U N C T I O  N S
 ==============================================================================
*/

/******************************************************************************
 * CreateGroupCB()
 ******************************************************************************/
void CreateGroupCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
CreateGroup();
}        
/******************************************************************************
 * CreateGroupNCloseCB()
 ******************************************************************************/
void CreateGroupNCloseCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
if (! CreateGroup())
   return;

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_create_group",create_grp_mw,
                      CloseCreateGrpCB,NULL,NULL,NULL,NULL,NULL);
else
   CloseCreateGroup();
}        
/******************************************************************************
 * CloseCreateGrpCB()
 ******************************************************************************/
void CloseCreateGrpCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
CloseCreateGroup();
}
/******************************************************************************
 * CreateGrpCloseCB()
 ******************************************************************************/
void CreateGrpCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
int data_lost;

/* Check for data that has been changed and will be lost */
data_lost = 0;

if (data_lost)
   PostQuestionDialog("msg_acc_close_create_grp_lose",create_grp_mw,
                      CloseCreateGrpCB,NULL,NULL,NULL,NULL,NULL);
else
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_close_create_grp",create_grp_mw,
                         CloseCreateGrpCB,NULL,NULL,NULL,NULL,NULL);
   else
      CloseCreateGroup();
}
/******************************************************************************
 * CreateGrpSaveNCloseCB()
 ******************************************************************************/
void CreateGrpSaveNCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{
if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",create_grp_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else
   {
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_create_group",create_grp_mw,
                         CreateGroupCB,NULL,NULL,NULL,NULL,NULL);
   else
      {
      if (CreateGroup())
         CloseCreateGroup();
      }
   }
}
/******************************************************************************
 * CreateGrpSaveCB()
 ******************************************************************************/
void CreateGrpSaveCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{

if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",create_grp_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else
   {
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_create_group",create_grp_mw,
                         CreateGroupNCloseCB,NULL,NULL,NULL,NULL,NULL);
   else
      {
      CreateGroup();
      }
   }
}
/******************************************************************************
 * CreateGrpSelectGrpCB()
 ******************************************************************************/
void CreateGrpSelectGrpCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
char         *groupname;
struct group *grp;

groupname = XmTextFieldGetString(w);
if (removeBlanks(groupname))
   XmTextSetString(w,groupname);

if (! XmTextListItemExists(w, groupname))
   {
   /* Generate next available GID */
   AccountAssignAndDisplayGID(choosegroupid());
   }
else
   {
   if ((grp=getgrnam(groupname)) != (struct group *) 0)
      {
      AccountAssignAndDisplayGID((int) grp->gr_gid);
      }
   else
      {
      PostErrorDialog("msg_error_grp_not_found",create_grp_mw,NULL,NULL,NULL,
                      NULL,NULL,NULL);
      }
   }

XtFree(groupname);
}
/******************************************************************************
 * CreateGrpGetWidgetCB()       
 ******************************************************************************/
void CreateGrpGetWidgetCB(w,widget_id,reason)
Widget	       w;
int	      *widget_id;
unsigned long *reason;

{
int         i;
Arg         args[10];

   switch (*widget_id)
      {
      case ACC_PRI_GROUP:
             AccData.priGroups = w;
           break;
      case ACC_GID:
             AccData.gidText = w;
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
 * OpenCreateGroup()
 ******************************************************************************/
void OpenCreateGroup(className,display)
char *className;
Display *display;
{
int n; Arg args[10];
/*
 * If this is the first time we open this window...
 */
if (! AccountModifyScreenIsOpen())
   {
   MrmRegisterNames (reglist,(sizeof reglist/sizeof reglist[0]));
   MrmRegisterTextListClass();
   MrmRegisterSelectListClass();

   n = 0;
   XtSetArg(args[n], XmNallowShellResize, True);  n++;
   XtSetArg(args[n], XmNy, 115);  n++;
   grp_create_tl=XtAppCreateShell("Create Groups",className,
                                  topLevelShellWidgetClass, display,args,n);

   /* Replace the WM_DELETE_RESPONSE from f.kill with controlled close */
   AccCommonReplaceSystemMenuClose(grp_create_tl,CloseCreateGrpCB);

   InitializeWidgetIDsToNULL(&AccData);

   /* Load data structure attached to the main widget user data field */
   InitializeUserDataStructure(&AccData);

   /* Initialize Widget Retrieval Account Data Structure */
   AccCommonInitWidgetRetrieval(&AccData);
                                                                                
   NEW_VUIT_Manage("GroupCreateMW",&create_grp_mw,grp_create_tl);

   /* Initialize the graphic screen objects with necessary data */
   InitializeGraphicObjects(&AccData);

   /* Clear the Graphic Objects before re-stating them */
   AccClearGraphicObjects(&AccData);

   XmTextListSetLoadRoutine(AccData.priGroups,CreateGrpLoadPriGroupTXList);

   XtRealizeWidget(grp_create_tl);
   }
else
   {
   XtRealizeWidget(grp_create_tl);
   }
}
/******************************************************************************
 * CloseCreateGroup()
 ******************************************************************************/
void CloseCreateGroup()

{
/* Clear Object so next screen opening doesn't display old data */
AccClearGraphicObjects(&AccData);

XtUnrealizeWidget(grp_create_tl);
}
