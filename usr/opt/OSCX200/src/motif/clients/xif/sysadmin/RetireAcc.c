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
static char *rcsid = "@(#)$RCSfile: RetireAcc.c,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/12/20 21:32:47 $";
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
static Widget acc_retire_tl=NULL;
static Widget retire_acc_mw=NULL;
static Widget retire_rem_files_tb=NULL;

static AccountDataType AccData;

/*
 * Forward Callback declarations
 */
void DeleteAccountFilesNCloseCB();
void DeleteAccountFilesCB();
void RetireAccCloseCB();
void RetireAccSaveCB();
void RetireAccSaveNCloseCB();
void RetireAccSelectUserCB();
void RetireAccGetWidgetCB();

void RetireAccountCB();
void CloseRetireAccCB();

/*
 * Forward declarations
 */
static Boolean AccountPasswordDataErrorFree();
void           CloseRetireUserAccount();

/*
 * Names and addresses of callback routines to register with Mrm
 */
static MrmRegisterArg reglist [] = {
{"RetireAccCloseCB",             (caddr_t)RetireAccCloseCB},
{"RetireAccSelectUserCB",        (caddr_t)RetireAccSelectUserCB},
{"RetireAccSaveCB",              (caddr_t)RetireAccSaveCB},
{"RetireAccSaveNCloseCB",        (caddr_t)RetireAccSaveNCloseCB},
{"RetireAccGetWidgetCB",         (caddr_t)RetireAccGetWidgetCB}};

/*
 ==============================================================================
 = P R I V A T E   R O U T I N E S   T O   S U P P O R T   A C C O U N T S
 ============================================================================== 
*/

/******************************************************************************
 * RetireAccLoadUsernameTXList()
 ******************************************************************************/
static void RetireAccLoadUsernameTXList()

{
LoadUsernameTXList(AccData.usernameText,USER_AND_ISSO_MODIFICATION);
}
/******************************************************************************
 * DeleteAccountFiles()
 ******************************************************************************/
static void DeleteAccountFiles(username)
char *username;

{
struct passwd     *pwd;
char              *delete_str;
char aud_msg_buf[200];

pwd = getpwnam(username);   

if (pwd != (struct passwd *) 0)
   {
   /* "rm -fr pwd->pw_dir\0 */
   delete_str = (char *)XtMalloc(sizeof(char)*(strlen(pwd->pw_dir) +10));
   if (delete_str != (char *)NULL)
      {
      sprintf(delete_str,"rm -fr %s",pwd->pw_dir);
                                
      if (system(delete_str) == -1)
         {
         PostErrorDialog("msg_acc_error_files_not_deleted",retire_acc_mw,
                         NULL,NULL,NULL,NULL,NULL,NULL);
         }
      XtFree(delete_str);
      }
   sprintf(aud_msg_buf,"Files Deleted Upon Account (%s) Retirement", username);
   WriteSingleAuditString(aud_msg_buf);
   }
else 
   {
   sprintf(aud_msg_buf,"FAILURE: Attempted Files Deletion failed Upon Account (%s) Retirement", username);
   WriteSingleAuditString(aud_msg_buf);
   PostErrorDialog("msg_acc_error_files_not_deleted",retire_acc_mw,NULL,
                   NULL,NULL,NULL,NULL,NULL);
   }
}
/******************************************************************************
 * RetireNCloseAccount()
 ******************************************************************************/
static Boolean RetireNCloseAccount()

{
char              *username;
struct pr_passwd  *prpwd;
char aud_msg_buf[200];
 
PostWorkingDialog("msg_acc_working_retire_account",retire_acc_mw,
                  NULL,NULL,NULL,NULL,NULL,NULL);

username = XmTextFieldGetString(AccData.usernameText);
if (removeBlanks(username))
   XmTextSetString(AccData.usernameText,username);

if (! XmTextListItemExists(AccData.usernameText,username))
   {
   UnPostWorkingDialog(retire_acc_mw);

   PostErrorDialog("msg_acc_error_user_not_found",retire_acc_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   return(False);
   }

if (XGetUserInfo(username,&AccData.userInfo->userData,retire_acc_mw) != SUCCESS)
   {
   UnPostWorkingDialog(retire_acc_mw);

   /* XGetUserInfo will display an error message */
   return(False);
   }

/* Get the Protected Password DB data pointer */
prpwd = &AccData.userInfo->userData.prpw;

/* Set the Retire Field and Flag */
prpwd->ufld.fd_retired = 1;
prpwd->uflg.fg_retired = 1;

/* Assign Name to userInfo structure if not already assigned */
AccData.userInfo->username = username;

if (XWriteUserInfo(&AccData.userInfo->userData) != SUCCESS)
   {
   UnPostWorkingDialog(retire_acc_mw);

   /* XWriteUserInfo will display an error message */

   sprintf(aud_msg_buf,"FAILURE: Retire Account (%s) failed", username);
   WriteSingleAuditString(aud_msg_buf);
   return(False);
   }

sprintf(aud_msg_buf,"Account (%s) Retired", username);
WriteSingleAuditString(aud_msg_buf);

if (XmToggleButtonGetState(retire_rem_files_tb))
   {
   UnPostWorkingDialog(retire_acc_mw);

   PostQuestionDialog("msg_acc_delete_account_files",retire_acc_mw,
                      DeleteAccountFilesNCloseCB,(XtPointer)username,NULL,NULL,
                      NULL,NULL);

   XmTextFieldSetString(AccData.usernameText,"");

   /* Reload the list of users as there should be one less */
   LoadUsernameTXList(AccData.usernameText,USER_AND_ISSO_MODIFICATION);
   
   return(True);
   }

XmTextFieldSetString(AccData.usernameText,"");

/* Reload the list of users as there should be one less */
LoadUsernameTXList(AccData.usernameText,USER_AND_ISSO_MODIFICATION);
   
UnPostWorkingDialog(retire_acc_mw);

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_retire_acc",retire_acc_mw,
                      CloseRetireAccCB,NULL,NULL,NULL,NULL,NULL);
else
   CloseRetireUserAccount();

return(True);
}
/******************************************************************************
 * RetireAccount()
 ******************************************************************************/
static Boolean RetireAccount()

{
char              *username;
struct pr_passwd  *prpwd;
char aud_msg_buf[200];
 
PostWorkingDialog("msg_acc_working_retire_account",retire_acc_mw,
                  NULL,NULL,NULL,NULL,NULL,NULL);

username = XmTextFieldGetString(AccData.usernameText);
if (removeBlanks(username))
   XmTextSetString(AccData.usernameText,username);

if (! XmTextListItemExists(AccData.usernameText,username))
   {
   UnPostWorkingDialog(retire_acc_mw);

   PostErrorDialog("msg_acc_error_user_not_found",retire_acc_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   return(False);
   }

if (XGetUserInfo(username,&AccData.userInfo->userData,retire_acc_mw) != SUCCESS)
   {
   UnPostWorkingDialog(retire_acc_mw);

   /* XGetUserInfo will display an error message */
   return(False);
   }

/* Get the Protected Password DB data pointer */
prpwd = &AccData.userInfo->userData.prpw;

/* Set the Retire Field and Flag */
prpwd->ufld.fd_retired = 1;
prpwd->uflg.fg_retired = 1;

/* Assign Name to userInfo structure if not already assigned */
AccData.userInfo->username = username;

if (XWriteUserInfo(&AccData.userInfo->userData) != SUCCESS)
   {
   UnPostWorkingDialog(retire_acc_mw);

   /* XWriteUserInfo will display an error message */

   sprintf(aud_msg_buf,"FAILURE: Retire Account (%s) failed", username);
   WriteSingleAuditString(aud_msg_buf);
   return(False);
   }

sprintf(aud_msg_buf,"Account (%s) Retired", username);
WriteSingleAuditString(aud_msg_buf);

if (XmToggleButtonGetState(retire_rem_files_tb))
   {
   PostQuestionDialog("msg_acc_delete_account_files",retire_acc_mw,
                      DeleteAccountFilesCB,(XtPointer)username,NULL,NULL,
                      NULL,NULL);
   }

XmTextFieldSetString(AccData.usernameText,"");

/* Reload the list of users as there should be one less */
LoadUsernameTXList(AccData.usernameText,USER_AND_ISSO_MODIFICATION);
   
UnPostWorkingDialog(retire_acc_mw);

return(True);
}
/******************************************************************************
 * AccountModifyScreenIsOpen()
 ******************************************************************************/
static Boolean AccountModifyScreenIsOpen()

{
if (acc_retire_tl)
   return(True);
return(False);
}

/*
 ==============================================================================
 = S T A T I C   R O U T I N E S   T O   S U P P O R T   A C C O U N T S
 ============================================================================== 
*/

/******************************************************************************
 * AccountPasswordDataErrorFree()
 ******************************************************************************/
static Boolean AccountPasswordDataErrorFree(pwd)
struct passwd pwd;

{
/* Check that user name is valid. No ":" */
if (CheckValidName(pwd.pw_name) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_name",retire_acc_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

if (CheckValidShell(pwd.pw_shell) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_shell",retire_acc_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

return(True);
}

/*
 ==============================================================================
 = C A L L B A C K   F U N C T I O  N S
 ==============================================================================
*/

/******************************************************************************
 * DeleteAccountFilesCB()
 ******************************************************************************/
void DeleteAccountFilesCB(w,username,reason)
Widget         w;
char          *username;
unsigned long *reason;

{
DeleteAccountFiles(username);
}        
/******************************************************************************
 * DeleteAccountFilesNCloseCB()
 ******************************************************************************/
void DeleteAccountFilesNCloseCB(w,username,reason)
Widget         w;
char          *username;
unsigned long *reason;

{
DeleteAccountFiles(username);

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_retire_acc",retire_acc_mw,
                      CloseRetireAccCB,NULL,NULL,NULL,NULL,NULL);
else
   CloseRetireUserAccount();
}        
/******************************************************************************
 * RetireAccountCB()
 ******************************************************************************/
void RetireAccountCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
RetireAccount();
}        
/******************************************************************************
 * RetireNCloseAccountCB()
 ******************************************************************************/
void RetireNCloseAccountCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
if (! RetireNCloseAccount())
   return;
}        
/******************************************************************************
 * CloseRetireAccCB()
 ******************************************************************************/
void CloseRetireAccCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
CloseRetireUserAccount();
}
/******************************************************************************
 * RetireAccCloseCB()
 ******************************************************************************/
void RetireAccCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_retire_acc",retire_acc_mw,
                      CloseRetireAccCB,NULL,NULL,NULL,NULL,NULL);
else
   CloseRetireUserAccount();
}
/******************************************************************************
 * RetireAccSaveCB()
 ******************************************************************************/
void RetireAccSaveCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{
if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",retire_acc_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else
   {
   PostQuestionDialog("msg_acc_retire_account",retire_acc_mw,
                      RetireAccountCB,NULL,NULL,NULL,NULL,NULL);
   }
}
/******************************************************************************
 * RetireAccSaveNCloseCB()
 ******************************************************************************/
void RetireAccSaveNCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{

if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",retire_acc_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else
   {
   PostQuestionDialog("msg_acc_retire_account",retire_acc_mw,
                      RetireNCloseAccountCB,NULL,NULL,NULL,NULL,NULL);
   }
}
/******************************************************************************
 * RetireAccSelectUserCB()
 ******************************************************************************/
void RetireAccSelectUserCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
char          *username;

/* Get Username */
username = XmTextFieldGetString(w);
if (removeBlanks(username))
   XmTextSetString(w,username);
}
/******************************************************************************
 * RetireAccGetWidgetCB()       
 ******************************************************************************/
void RetireAccGetWidgetCB(w,widget_id,reason)
Widget	       w;
int	      *widget_id;
unsigned long *reason;

{
int         i;

   switch (*widget_id)
      {
      case ACC_USERNAME:
             AccData.usernameText = w;
           break;
      case ACC_RETIRE_REM_FILES:
             retire_rem_files_tb = w;
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
 * OpenRetireUserAccount()
 ******************************************************************************/
void OpenRetireUserAccount(className,display)
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
   acc_retire_tl=XtAppCreateShell("Retire User Accounts",className,
                                  topLevelShellWidgetClass,display,args,n);

   /* Replace the WM_DELETE_RESPONSE from f.kill with controlled close */
   AccCommonReplaceSystemMenuClose(acc_retire_tl,CloseRetireAccCB);

   InitializeWidgetIDsToNULL(&AccData);

   /* Load data structure attached to the main widget user data field */
   InitializeUserDataStructure(&AccData);

   /* Initialize Widget Retrieval Account Data Structure */
   AccCommonInitWidgetRetrieval(&AccData);

   NEW_VUIT_Manage("AccountRetireMW",&retire_acc_mw,acc_retire_tl);

   /* Initialize the graphic screen objects with necessary data */
   InitializeGraphicObjects(&AccData);

   /* Clear the Graphic Objects before re-stating them */
   AccClearGraphicObjects(&AccData);

   XmTextListSetLoadRoutine(AccData.usernameText,RetireAccLoadUsernameTXList);

   XtRealizeWidget(acc_retire_tl);
   }
else
   {
   XtRealizeWidget(acc_retire_tl);
   }
}
/******************************************************************************
 * CloseRetireUserAccount()
 ******************************************************************************/
void CloseRetireUserAccount()

{
/* Clear objects before closing so screen will not be reopened with old data */
AccClearGraphicObjects(&AccData);

XtUnrealizeWidget(acc_retire_tl);
}
