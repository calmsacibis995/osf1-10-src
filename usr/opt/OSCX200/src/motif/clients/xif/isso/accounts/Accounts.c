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
static char *rcsid = "@(#)$RCSfile: Accounts.c,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/12/20 21:31:28 $";
#endif

/*******************************************************************************
 *
 *
 * Assumptions:
 *
 * 1 Updates to "Databases" depend on a pr_passwd data structure fld value
 *   and corresponding flg value indicating whether the value is set or
 *   a default should be taken. The old role programs allowed for
 *   defaults. This version will always explicitly set the value of the
 *   field as it is more of a what-you-see-is-what-you-get approach.
 *   The structure definition initializes all flags to 1 so no flags
 *   are to be programatically altered here.
 *
 * 2 All default flags and fields in the structure pr_passwd will be
 *   maintained for backward compatibility until such time as it is
 *   determined that they are no longer necessary.
 *
 ******************************************************************************/

#include <stdio.h>                      /* For printf and so on. */
#include <pwd.h>
#include <grp.h>
#include <Xm/Xm.h>
#include <X11/Protocols.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Mrm/MrmAppl.h>                /* Motif Toolkit and MRM */
#include <X11/X.h>
#include "XIsso.h"
#include "XMain.h"
#include "XAccounts.h"
#include "Resources.h"
#include "Utilities.h"
#include "AccountSelection.h"
#include "AccCommon.h"
#include "Messages.h"
#include "hyperhelp.h"

/* The vuit generated application include file.				    */
/* If the user does not specify a directory for the file		    */
/* then the vaxc$include logical needs to be defined to point to the	    */
/* directory of the include file.					    */
#define REF_TYPE
#include "Vuit.h"

/******************************************************************************
 * Static Global data
 ******************************************************************************/

/*
 * Set the top level widget to NULL to distinguish between having to Open
 * the Account Window and create the shell vs Close, Unrealize, Open again 
 * and merely re-Realize the Shell
 */
static Widget acc_modify_tl=NULL;
Widget accounts_mw=NULL;

/*
 * Forward declarations
 */
static Boolean AccountPasswordDataErrorFree();
static void    AccountModifyClose();

static AccountDataType AccData;

static int modification_type;
/*
 * Forward Callback declarations
 */
void AccountSelectUserCB();
void AccountSelectTemplateCB();
void AccountCloseCB();
void AccountSaveCB();
void AccountSaveNCloseCB();
void AccountAssignTemplateCB();

void ModifyAccountCB();
void CloseAccountCB();
void AccModUserHelpCB();
void AccountsOpenAuditScreenCB();

/*
 * Names and addresses of callback routines to register with Mrm
 */
static MrmRegisterArg reglist [] = {
{"AccountsOpenAuditScreenCB",   (caddr_t)AccountsOpenAuditScreenCB},
{"AccountCloseCB",              (caddr_t)AccountCloseCB}};

/*
 ==============================================================================
 = P R I V A T E   R O U T I N E S   T O   S U P P O R T   A C C O U N T S
 ============================================================================== 
*/
/******************************************************************************
 * AccountsLoadUsernameTXList()
 ******************************************************************************/
static void AccountsLoadUsernameTXList()

{
LoadUsernameTXList(AccData.usernameText,modification_type);
}
/******************************************************************************
 * AccountsLoadTmpltnameTXList()
 ******************************************************************************/
static void AccountsLoadTmpltnameTXList()

{
LoadTmpltnameTXList(AccData.tmpltnameText);
}
/******************************************************************************
 * AccountsLoadPriGroupTXList()
 ******************************************************************************/
static void AccountsLoadPriGroupTXList()

{
LoadPriGroupTXList(AccData.priGroups);
}
/******************************************************************************
 * ModifyAccount()
 ******************************************************************************/
static Boolean ModifyAccount()

{
struct passwd      pwd;
struct group       grp;
char             **secGroup_names;
int                secGroup_count;
int                ret_code;
char               tmpltname[MAX_UNAME_LEN];
struct pr_passwd  *prpwd;
char aud_msg_buf[200];
 
PostWorkingDialog("msg_acc_working_modify_account",accounts_mw,
                  NULL,NULL,NULL,NULL,NULL,NULL);

/* Set the RELEVANT (screen manipulated) user flags to 0 */
AccSetAllUserFieldFlags(&AccData,0);

/* Get the Protected Password DB data pointer */
prpwd = &AccData.userInfo->userData.prpw;

/* Retrieve Data from graphic widgets */
if (! AccLoadDataFromScreen(&AccData,&pwd,&grp,&secGroup_names,&secGroup_count,
                            (int)ACC_USERS))
   {
   /* Free alloced data retrieved from screen widgets */
   AccFreePasswordDataStructure(&pwd);
   AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

   UnPostWorkingDialog(accounts_mw);
   return(False);
   }

/* 
 * Make sure the account already exists and is accessible 
 * (its in the scrolled list of choices)
 */
if (! XmTextListItemExists(AccData.usernameText,pwd.pw_name))
   {
   /* Free alloced data retrieved from screen widgets */
   AccFreePasswordDataStructure(&pwd);
   AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

   UnPostWorkingDialog(accounts_mw);

   PostErrorDialog("msg_acc_error_user_doesnt_exist",accounts_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

/* Get the default values (either template or system default) */
if (strcmp(prpwd->ufld.fd_template,"") == 0)
   AccRetrieveAndStoreDefault(&AccData,SYSTEM_DEFAULT,accounts_mw);
else
   AccRetrieveAndStoreDefault(&AccData,prpwd->ufld.fd_template,accounts_mw);

/* Assign uid as it is currently part of pwd structure only */
prpwd->ufld.fd_uid = pwd.pw_uid;
prpwd->uflg.fg_uid = 1;

AccAssignSmartFlagsForDBSave(&AccData,(int)ACC_USERS);

/* Assign Name to userInfo structure if not already assigned or NULL */
if ((! AccData.userInfo->username) ||
    strcmp(AccData.userInfo->username,pwd.pw_name) != 0)
   {
   XtFree(AccData.userInfo->username);
   
   AccData.userInfo->username = 
      (char *)XtMalloc(sizeof(char *) * (strlen(pwd.pw_name) + 1));
   if (AccData.userInfo->username != NULL)
      strcpy(AccData.userInfo->username,pwd.pw_name);
   }

/* Set field = 0 so WriteUserInfo will know this is a template */
prpwd->ufld.fd_istemplate = 0;

/* Fill in miscellaneous fields */
pwd.pw_passwd = "*";
#ifdef AUX
pwd.pw_age = "";
#endif
pwd.pw_comment = "";

if (! AccountPasswordDataErrorFree(pwd))
   {
   /* Free alloced data retrieved from screen widgets and not from getpwname */
   AccFreePasswordDataStructure(&pwd);
   AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

   UnPostWorkingDialog(accounts_mw);
   return(False);
   }

ret_code = XModifyUserpwd(&pwd);

/* Update the group information (/etc/group) */
if (ret_code == SUCCESS) 
   ret_code = XModifyUserGroup(pwd.pw_name,(int)pwd.pw_gid,secGroup_names,
                               accounts_mw);
if (ret_code == SUCCESS)
   ret_code = XWriteUserInfo(&AccData.userInfo->userData,accounts_mw);

if (ret_code == SUCCESS)
   {
   /* Self Audit if an account was locked */
   if ((prpwd->uflg.fg_lock == 1) && (prpwd->ufld.fd_lock == 1))
      {
      sprintf(aud_msg_buf,"Account (%s) Locked",prpwd->ufld.fd_name);
      WriteSingleAuditString(aud_msg_buf);
      }
   /* Self Audit if an account was unlocked */
   else if ((prpwd->uflg.fg_lock == 1) && (prpwd->ufld.fd_lock == 0))
      {
      /* Create The Audit Message String */
      sprintf(aud_msg_buf,"Account (%s) Unlocked",prpwd->ufld.fd_name);

      /* send our report to audit */
      WriteSingleAuditString(aud_msg_buf);
      }
   }

#ifdef SEC_PRIV
if (ret_code == SUCCESS)
   {
/****
   ret_code = write_authorizations(prpwd->ufld.fd_name,
                                   AccData.userInfo->command_auths,
                                   AccData.userInfo->ncommand_auths);
   if (ret_code == SUCCESS)
      {
      sprintf(aud_msg_buf,
              "Successful update of command auth database entry for user (%s)",
              prpwd->ufld.fd_name);
      WriteSingleAuditString(aud_msg_buf);
      }
   else
      {
      sprintf(aud_msg_buf,
              "Unable to update command auth database entry for user (%s)",
              prpwd->ufld.fd_name);
      WriteSingleAuditString(aud_msg_buf);
      }
 ****/

   /*
    * ignore error as it is not yet understood and operation seems to work
    *
   if (ret_code != SUCCESS)
      PostErrorDialog("msg_error_write_auths",accounts_mw,
                      NULL,NULL,NULL,NULL,NULL,NULL);
    *
    */
   }
#endif /* SEC_PRIV */


/* Free alloced data retrieved from screen widgets and not from getpwname() */
AccFreePasswordDataStructure(&pwd);
AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

UnPostWorkingDialog(accounts_mw);

if (ret_code == SUCCESS)
   return(True);
else return(False);
}
/******************************************************************************
 * AccountModifyScreenIsOpen()
 ******************************************************************************/
static Boolean AccountModifyScreenIsOpen()

{
if (acc_modify_tl)
   return(True);
return(False);
}

/*
 ==============================================================================
 = P U B L I C   R O U T I N E S   T O   S U P P O R T   A C C O U N T S
 ============================================================================== 
*/

/*
 ==============================================================================
 = S T A T I C   R O U T I N E S   T O   S U P P O R T   A C C O U N T S
 ============================================================================== 
*/

/******************************************************************************
 * TailorAccScreenToAccounts()
 ******************************************************************************/
static void TailorAccScreenToAccounts()

{
}
/******************************************************************************
 * AddAccountCBs()
 ******************************************************************************/
static void AddAccountCBs()

{
XtAddCallback(AccData.usernameText,XmNactivateCallback,
              AccountSelectUserCB,(caddr_t)NULL);
XtAddCallback(AccData.tmpltnameText,XmNactivateCallback,
              AccountSelectTemplateCB,(caddr_t)NULL);
XtAddCallback(AccData.apply_pb,XmNactivateCallback,
              AccountSaveCB,(caddr_t)NULL);
XtAddCallback(AccData.cancel_pb,XmNactivateCallback,
              AccountCloseCB,(caddr_t)NULL);
XtAddCallback(AccData.ok_pb,XmNactivateCallback,
              AccountSaveNCloseCB,(caddr_t)NULL);
XtAddCallback(AccData.help_pb,XmNactivateCallback,
              AccModUserHelpCB,(caddr_t)NULL);
}
/******************************************************************************
 * AssignTabGroups()
 ******************************************************************************/
static void AssignTabGroups()

{

XmTextListAddTabGroup(AccData.usernameText);
XmTextListAddTabGroup(AccData.tmpltnameText);
XmTextListAddTabGroup(AccData.shell);
XmAddTabGroup(AccData.comments);
XmTextListAddTabGroup(AccData.priGroups);
XmSelListAddTabGroup(AccData.secGroups);
XmAddTabGroup(AccData.expdate_tx);
XmAddTabGroup(AccData.tod_tx);
XmAddTabGroup(AccData.maxTries); 
XmAddTabGroup(AccData.nice);
XmAddTabGroup(AccData.locked);
XmAddTabGroup(AccData.fields[ACC_CHG_TIME_INDEX]);
XmAddTabGroup(AccData.fields[ACC_EXP_TIME_INDEX]);
XmAddTabGroup(AccData.fields[ACC_LIFETIME_INDEX]); 
XmAddTabGroup(AccData.fields[ACC_MAX_LENGTH_INDEX]); 
XmAddTabGroup(AccData.fields[ACC_MIN_LENGTH_INDEX]); 
XmAddTabGroup(AccData.fields[ACC_HIST_LIMIT_INDEX]); 
XmAddTabGroup(AccData.fields[ACC_REQUIRED_INDEX]);
XmAddTabGroup(AccData.fields[ACC_USER_CHOOSE_INDEX]);
XmAddTabGroup(AccData.fields[ACC_GENERATED_INDEX]); 
XmAddTabGroup(AccData.fields[ACC_RAND_CHARS_INDEX]);
XmAddTabGroup(AccData.fields[ACC_RAND_LETTERS_INDEX]); 
XmAddTabGroup(AccData.fields[ACC_TRIV_CHECKS_INDEX]);
XmAddTabGroup(AccData.fields[ACC_SITE_TRIV_CHECKS_INDEX]);

#ifdef SEC_PRIV
XmSelListAddTabGroup(AccData.authList);
XmSelListAddTabGroup(AccData.kernelList);
XmSelListAddTabGroup(AccData.baseList);
#endif /* SEC_PRIV */

XmAddTabGroup(AccData.ok_pb);
XmAddTabGroup(AccData.apply_pb);
XmAddTabGroup(AccData.cancel_pb);
}
/******************************************************************************
 * AccountPasswordDataErrorFree()
 ******************************************************************************/
static Boolean AccountPasswordDataErrorFree(pwd)
struct passwd pwd;

{
/* Check that user name is valid. No ":" */
if (CheckValidName(pwd.pw_name) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_name",accounts_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

if (CheckValidShell(pwd.pw_shell) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_shell",accounts_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

if (CheckValidGid(pwd.pw_gid) != SUCCESS)
   {
   /* The user is forced to pick from a list so he must not have picked */
   PostErrorDialog("msg_error_need_primary_group",accounts_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
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
 * AccountModUserHelpCB()
 ******************************************************************************/
void AccModUserHelpCB (w, client_data, call_data)
Widget	w;
caddr_t client_data;
caddr_t call_data;
{
HyperHelpDisplay(MOD_USER_BOX);
}

/******************************************************************************
 * AccountAssignTemplateCB()
 ******************************************************************************/
void AccountAssignTemplateCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
int       n;
Arg       args[10];
XmString  xmstring;
char     *str;

n = 0;
XtSetArg(args[n],XmNlabelString,&xmstring); n++;
XtGetValues(w, args, n);

XmStringGetLtoR(xmstring,charset,&str);
AccAssignTemplate(&AccData,str);

XmStringFree(str);
}
/******************************************************************************
 * ModifyAccountCB()
 ******************************************************************************/
void ModifyAccountCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
ModifyAccount();
}        
/******************************************************************************
 * ModifyNCloseAccountCB()
 ******************************************************************************/
void ModifyNCloseAccountCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
if (! ModifyAccount())
   return;

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_account",accounts_mw,
                      CloseAccountCB,NULL,NULL,NULL,NULL,NULL);
else
   AccountModifyClose();
}        
/******************************************************************************
 * CloseAccountCB()
 ******************************************************************************/
void CloseAccountCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
AccountModifyClose();
}
/******************************************************************************
 * AccountCloseCB()
 ******************************************************************************/
void AccountCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
int data_lost;

/* Check for data that has been changed and will be lost */
data_lost = 0;

if (data_lost)
   PostQuestionDialog("msg_acc_close_account_lose",accounts_mw,
                      CloseAccountCB,NULL,NULL,NULL,NULL,NULL);
else
   {
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_close_account",accounts_mw,
                         CloseAccountCB,NULL,NULL,NULL,NULL,NULL);
   else
      AccountModifyClose();
   }
}
/******************************************************************************
 * AccountSaveCB()
 ******************************************************************************/
void AccountSaveCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{
if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",accounts_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else
   {
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_modify_account",accounts_mw,
                         ModifyAccountCB,NULL,NULL,NULL,NULL,NULL);
   else
      ModifyAccount();
   }
}
/******************************************************************************
 * AccountSaveNCloseCB()
 ******************************************************************************/
void AccountSaveNCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{

if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",accounts_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else
   {
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_modify_account",accounts_mw,
                         ModifyNCloseAccountCB,NULL,NULL,NULL,NULL,NULL);
   else
      {
      if (ModifyAccount())
         AccountModifyClose();
      }
   }
}
/******************************************************************************
 * AccountSelectTemplateCB()
 ******************************************************************************/
void AccountSelectTemplateCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
char          *tmpltname;

/* Get Template name */
tmpltname = XmTextFieldGetString(w);

AccAssignTemplate(&AccData,tmpltname);
}
/******************************************************************************
 * AccountSelectUserCB()
 ******************************************************************************/
void AccountSelectUserCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
char          *username;

/* Get Username */
username = XmTextFieldGetString(w);

AccEntNameEntered(&AccData,w,username,ACC_USERS);
}
/******************************************************************************
 * AccountsOpenAuditScreenCB()
 ******************************************************************************/
void AccountsOpenAuditScreenCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
AccOpenAuditEventsScreen(&AccData,XmTextFieldGetString(AccData.usernameText),
                         ACC_USERS,accounts_mw);
}

/*
 ==============================================================================
 = E N T R Y   P O I N T   T O   M O D I F Y   U S E R   A C C O U N T S
 ============================================================================== 
*/

/******************************************************************************
 * AccountModifyOpen()
 ******************************************************************************/
void AccountModifyOpen(className,display,type)
char    *className;
Display *display;
int      type;

{
int n; 
Arg args[10];

/*
 * If this is the first time we open this window...
 */
if (! AccountModifyScreenIsOpen())
   {
   /* Save the modification type (ISSO-from SysAdmin or General User) */
   modification_type = type;
 
   MrmRegisterNames (reglist,(sizeof reglist/sizeof reglist[0]));

   n = 0;
   XtSetArg(args[n], XmNy, 115);  n++;
   XtSetArg(args[n], XmNallowShellResize, True);  n++;
   if (type == USER_MODIFICATION)
      acc_modify_tl=XtAppCreateShell("Modify User Accounts",className,
                                     topLevelShellWidgetClass,display,args,n);
   else if (type == ISSO_MODIFICATION)
      acc_modify_tl=XtAppCreateShell("Modify ISSO Accounts",className,
                                     topLevelShellWidgetClass,display,args,n);

   /* Replace the WM_DELETE_RESPONSE from f.kill with controlled close */
   AccCommonReplaceSystemMenuClose(acc_modify_tl,CloseAccountCB);

   /* Load data structure attached to the main widget user data field */
   InitializeUserDataStructure(&AccData);

   /* Initialize Widget Retrieval Account Data Structure */
   AccCommonInitWidgetRetrieval(&AccData);

   NEW_VUIT_Manage(ACCOUNT_WIDGET,&accounts_mw,acc_modify_tl);

   /* Initialize the graphic screen objects with necessary data */
   InitializeGraphicObjects(&AccData);

   /* Tailor the stock account screen to Account nuances */
   TailorAccScreenToAccounts();
                                  
   AssignTabGroups();

   /* Add Callbacks */
   AddAccountCBs();

   /* Clear the Graphic Objects before re-stating them */
   AccClearGraphicObjects(&AccData);

   XtRealizeWidget(acc_modify_tl);

   XmTextListSetLoadRoutine(AccData.usernameText,AccountsLoadUsernameTXList);

   XmTextListSetLoadRoutine(AccData.tmpltnameText,AccountsLoadTmpltnameTXList);

   XmTextListSetLoadRoutine(AccData.priGroups,AccountsLoadPriGroupTXList);
   }
else
   {
   XtRealizeWidget(acc_modify_tl);
   }
}
/******************************************************************************
 * AccountModifyClose()
 ******************************************************************************/
static void AccountModifyClose()

{
/* Clear objects so next time screen is managed it won't display old data */
AccClearGraphicObjects(&AccData);

XtUnrealizeWidget(acc_modify_tl);
}
