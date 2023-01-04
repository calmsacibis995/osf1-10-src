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
static char *rcsid = "@(#)$RCSfile: CreateAcc.c,v $ $Revision: 1.1.2.9 $ (DEC) $Date: 1993/12/20 21:32:38 $";
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
#include "hyperhelp.h"

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

#define INVALID_GROUP_ID  -100
#define PASSWD_PAG        "/etc/passwd.pag"
#define PASSWD_DIR        "/etc/passwd.dir"
#define PW_ID_MAP         "/etc/auth/system/pw_id_map"

/******************************************************************************
 * Static Global data
 ******************************************************************************/

/*
 * Set the top level widget to NULL to distinguish between having to Open
 * the Account Window and create the shell vs Close, Unrealize, Open again 
 * and merely re-Realize the Shell
*/
static Widget acc_create_tl=NULL;
Widget create_acc_mw=NULL;

static AccountDataType    AccData;

static CHANGES_MADE = False;
/*
* Forward Callback declarations
*/
void CreateAccountCloseCB();
void CreateAccSaveCB();
void CreateAccSaveNCloseCB();
void CreateAccSelectUserCB();
void CreateAccHelpCB();
void CreateAccGetWidgetCB();

void CreateAccountCB();
void CreateAccountFieldChangedCB();
void CloseCreateAccountCB();

/*
* Forward declarations
*/
static Boolean AccountPasswordDataErrorFree();
void           CloseCreateUserAccount();

/*
* Names and addresses of callback routines to register with Mrm
*/
static MrmRegisterArg reglist [] = {
{"CreateAccountCloseCB",       (caddr_t)CreateAccountCloseCB},
{"CreateAccSelectUserCB",      (caddr_t)CreateAccSelectUserCB},
{"CreateAccSaveCB",            (caddr_t)CreateAccSaveCB},
{"CreateAccSaveNCloseCB",      (caddr_t)CreateAccSaveNCloseCB},
{"CreateAccHelpCB", 	       (caddr_t)CreateAccHelpCB},
{"CreateAccountFieldChangedCB",(caddr_t)CreateAccountFieldChangedCB},
{"CreateAccGetWidgetCB",       (caddr_t)CreateAccGetWidgetCB}};

/*
==============================================================================
= P R I V A T E   R O U T I N E S   T O   S U P P O R T   A C C O U N T S
============================================================================== 
*/

/******************************************************************************
 * CreateAccLoadUsernameTXList()
 ******************************************************************************/
static void CreateAccLoadUsernameTXList()

{
LoadUsernameTXList(AccData.usernameText,USER_AND_ISSO_MODIFICATION);
}
/******************************************************************************
 * CreateAccLoadPriGroupTXList()
 ******************************************************************************/
static void CreateAccLoadPriGroupTXList()

{
LoadPriGroupTXList(AccData.priGroups);
}
/******************************************************************************
* UserNameExits()
******************************************************************************/
static Boolean UserNameExists(username)
char    *username;

{
if (getpwnam(username) == (struct passwd *) 0)
   return (False);
else
   return (True);
}
/******************************************************************************
* AccountIsNewUser()
******************************************************************************/
static Boolean AccountIsNewUser(pwd)
struct passwd pwd;

{
if (UserNameExists(pwd.pw_name))
   {
   PostErrorDialog("msg_error_create_acc_user_exists",create_acc_mw,NULL,
                   NULL,NULL,NULL,NULL,NULL);
   return(False);
   }

if (UserUidExists(pwd.pw_uid))
   {
   PostErrorDialog("msg_error_create_acc_uid_exists",create_acc_mw,NULL,
                   NULL,NULL,NULL,NULL,NULL);
   return(False);
   }

return(True);
}
/******************************************************************************
 * CreateAccount()
 ******************************************************************************/
static Boolean CreateAccount()

{
struct passwd      pwd;
struct group       grp;
char             **secGroup_names;
int                secGroup_count;
int                ret_code;
int                ret_flag;
struct pr_passwd  *prpwd;
int                mod_state[NUM_MOD_STATES] = {0,0,0,0};

char aud_msg_buf[200];

/* Return immediately with a True flag if no changes have been made */
if (! CHANGES_MADE)
   return(True);

PostWorkingDialog("msg_acc_working_create_account",create_acc_mw,
	  NULL,NULL,NULL,NULL,NULL,NULL);

/* Set the RELEVANT (screen manipulated) user flags to 0 */
AccSetAllUserFieldFlags(&AccData,0);

/* Retrieve Data from graphic widgets */
AccLoadAccountDataFromScreen(&AccData,&pwd,&grp, (int)ACC_USERS);
AccLoadSecGroupDataFromScreen(&AccData,&secGroup_names, &secGroup_count);

/* Get the Protected Password DB data pointer */
prpwd = &AccData.userInfo->userData.prpw;

/* Copy the name field to the protected password database structure */
strcpy(prpwd->ufld.fd_name,pwd.pw_name);

/* Assign uid as it is currently part of pwd structure only */
prpwd->ufld.fd_uid = pwd.pw_uid;
prpwd->uflg.fg_uid = 1;

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

/* Set fg_name == 1 OR protected DB entry will be deleted */
prpwd->uflg.fg_name = 1;

/* Fill in miscellaneous fields */
pwd.pw_passwd = "*";
#ifdef AUX
pwd.pw_age = "";
#endif
pwd.pw_comment = "";

ret_flag = True;

/* Check for pre-existance of user; else post an error box */

/* Check the Screen Data for errors */
if (! AccountIsNewUser(pwd)) 
   ret_flag = False;
else if ((! AccountPasswordDataErrorFree(pwd))) 
   ret_flag = False;
else
   {
   /* Save the New User */
   ret_code = XCreateUserAccount(&pwd,create_acc_mw,mod_state);
   if (ret_code != SUCCESS)  /* Update the group information (/etc/group) */
      ret_flag = False;
   else
      {
      ret_code = XModifyUserGroup(pwd.pw_name,(int)pwd.pw_gid,secGroup_names);
      if (ret_code != SUCCESS)
         ret_flag = False;
      else
         {
         /* Set a flag to indicate the /etc/group file has been modified */
         mod_state[MOD_ETC_GROUP] = 1;

         /*
          * No need to set a flag here as the pr_passwd file was created 
          * in XCreateUserAccount.
          */
         ret_code = XWriteUserInfo(&AccData.userInfo->userData);
         if (ret_code != SUCCESS)
            ret_flag = False;
         }
      }
   if (ret_flag == False)
      {
      AccCommonDeleteAccount(pwd,&AccData,create_acc_mw,mod_state);
      }
   }

/* If an error occured, remove all traces of the account */

/*
 * If called inappropriately, the following will wipe out
 * an existing home directory structure, very dangerous.
 */
if (ret_flag == False)
   {
   sprintf(aud_msg_buf,"FAILURE: Account Creation failed: %s %d %s %s %d",
           pwd.pw_name,pwd.pw_uid,pwd.pw_dir,pwd.pw_shell,pwd.pw_gid);
   WriteSingleAuditString(aud_msg_buf);

   AccCommonDeleteAccount(pwd,&AccData,create_acc_mw,mod_state);
   }
else
   {
   sprintf(aud_msg_buf,"Account Creation Succeeded: %s %d %s %s %d\n",
           pwd.pw_name,pwd.pw_uid,pwd.pw_dir,pwd.pw_shell,pwd.pw_gid);
   WriteSingleAuditString(aud_msg_buf);
   }

/* Free alloced data retrieved from screen widgets and not from getpwname() */
AccFreePasswordDataStructure(&pwd);
AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

UnPostWorkingDialog(create_acc_mw);

/* If Account is saved properly, reset "Changes" flag to false */
if (ret_flag == True)
   CHANGES_MADE = False;

return(ret_flag);
}
/******************************************************************************
* CreateAccountScreenIsOpen()
******************************************************************************/
static Boolean CreateAccountScreenIsOpen()

{
if (acc_create_tl)
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
* CreateAccEntNameEntered()
******************************************************************************/
static void CreateAccEntNameEntered(w,username)
Widget  w;
char   *username;

{
struct passwd     *pwd;
struct group      *grp;
char               prigrpnam[25];
char             **cp;
XmString           xmstring;
char               temp[10];

if ((XmTextListItemExists(w,username))&& (UserNameExists(username)))
   {
   /* Clear the Graphic Objects before re-stating them */
   AccClearGraphicObjects(&AccData);

   /* Get Account Information. No "free" is necessary. */
   pwd = getpwnam(username);
   if (pwd != (struct passwd *) 0)
      {
      /* Redisplay the username as it was cleared with all fields previously */
      XmTextSetString(AccData.usernameText, username);

      SET_TEXT_FROM_NUMBER(temp, pwd->pw_uid, AccData.uidText);
      XmTextSetString(AccData.directory, pwd->pw_dir);
      XmTextSetString(AccData.comments, pwd->pw_gecos);

      if (strcmp(pwd->pw_shell,"")==0)
         XmTextSetString(AccData.shell,"/bin/sh");
      else XmTextSetString(AccData.shell, pwd->pw_shell);

      if ((grp = getgrgid(pwd->pw_gid)) != (struct group *) 0)
         {
         XmTextSetString(AccData.priGroups,grp->gr_name);
         /* Remember prigroup name so not to double count in secondary groups */
         strcpy(prigrpnam,grp->gr_name);
         }
      else 
         {
         XmTextSetString(AccData.priGroups,"");
         strcpy(prigrpnam,"");
         }

   
      /* Select the secondary groups */
      setgrent();
      while (grp = getgrent())
            {
            for (cp = grp->gr_mem; *cp; cp++)
                {
                if (strcmp(username,*cp) == 0)
                   {
                   /* Don't count the primary group name in secondary groups */
                   if (strcmp(grp->gr_name,prigrpnam) != 0)
                      {
                      xmstring = XmStringCreate(grp->gr_name, charset);
                      if (! xmstring)
                         MemoryError();
                      XmSelectListAddItem(AccData.secGroups,xmstring,0);
                      XmAvailableListDeleteItem(AccData.secGroups,xmstring);
                      XmStringFree(xmstring);
                      }
                   }
                }
            } /* while */
      } /* if */
      else PostErrorDialog("msg_acc_error_user_not_found",create_acc_mw,NULL,
                           NULL,NULL,NULL,NULL,NULL);
   } /* if */
else
   PostErrorDialog("msg_acc_error_user_not_found",create_acc_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
}
/******************************************************************************
* AssignAndDisplayHomeDirStub()
******************************************************************************/
static void AssignAndDisplayHomeDirStub()

{
XmTextSetString(AccData.directory,ApplicationResourcesInfo.defaultHomeDirRoot);
}
/******************************************************************************
* AccountAssignAndDisplayUID()
******************************************************************************/
static void AccountAssignAndDisplayUID(uid)
int uid;

{
char           temp[UID_SIZE];

AccData.uid = uid;
SET_TEXT_FROM_NUMBER(temp, uid, AccData.uidText);
}
/******************************************************************************
* DisplayAllGroupsAsAvailable()
******************************************************************************/
static void DisplayAllGroupsAsAvailable()

{

/* 
* Clear out old Selected Groups and Old Available Groups 
* OR add Selected Groups to available OR ...
*/
XmSelectListDeleteAllItems(AccData.secGroups);
XmAvailableListDeleteAllItems(AccData.secGroups);
XmAvailableListAddItems(AccData.secGroups,AccData.group_xmstrings,
		AccData.ngroups,0);
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
   PostErrorDialog("msg_acc_error_mkusr_invalid_name",create_acc_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

/* Check that UID is valid */

if (CheckValidUid(pwd.pw_uid) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_uid",create_acc_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

/* Check that home directory is valid */

if (CheckValidHomeDir(&pwd) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_invalid_home_dir",create_acc_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

if (CheckValidShell(pwd.pw_shell) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_shell",create_acc_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

if (CheckValidComment(pwd.pw_gecos) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_gecos",create_acc_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return (False);
   }

return(True);
}

/*
==============================================================================
= C A L L B A C K   F U N C T I O  N S
==============================================================================
*/

/******************************************************************************
* CreateAccountFieldChangedCB()
******************************************************************************/
void CreateAccountFieldChangedCB(w,client_data,call_data)
Widget  w;
caddr_t client_data;
caddr_t call_data;

{
CHANGES_MADE = True;
}

/******************************************************************************
* CreateAccountHelpCB()
******************************************************************************/
void CreateAccHelpCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
HyperHelpDisplay(CREATE_ACC_BOX);
}        
/******************************************************************************
* CreateAccountCB()
******************************************************************************/
void CreateAccountCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
CreateAccount();
}        
/******************************************************************************
* CreateNCloseAccountCB()
******************************************************************************/
void CreateNCloseAccountCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
if (! CreateAccount())
   return;

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_account",create_acc_mw,
                      CloseCreateAccountCB,NULL,NULL,NULL,NULL,NULL);
else
   CloseCreateUserAccount();
}        
/******************************************************************************
* CloseCreateAccountCB()
******************************************************************************/
void CloseCreateAccountCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
CloseCreateUserAccount();
}
/******************************************************************************
* CreateAccountCloseCB()
******************************************************************************/
void CreateAccountCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
int data_lost;

/* Check for data that has been changed and will be lost */
data_lost = 0;

if (data_lost)
   PostQuestionDialog("msg_acc_close_account_lose",create_acc_mw,
                      CloseCreateAccountCB,NULL,NULL,NULL,NULL,NULL);
else
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_close_account",create_acc_mw,
                         CloseCreateAccountCB,NULL,NULL,NULL,NULL,NULL);
   else
      CloseCreateUserAccount();
}
/******************************************************************************
* CreateAccSaveCB()
******************************************************************************/
void CreateAccSaveCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{
if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",create_acc_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else
   {
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_create_account",create_acc_mw,
                         CreateAccountCB,NULL,NULL,NULL,NULL,NULL);
   else
      CreateAccount();
   }
}
/******************************************************************************
* CreateAccSaveNCloseCB()
******************************************************************************/
void CreateAccSaveNCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{

if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",create_acc_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else
   {
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_create_account",create_acc_mw,
                         CreateNCloseAccountCB,NULL,NULL,NULL,NULL,NULL);
   else
      {
      if (CreateAccount())
         CloseCreateUserAccount();
      }
   }
}
/******************************************************************************
* CreateAccSelectUserCB()
******************************************************************************/
void CreateAccSelectUserCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
char *username;

/* Get Username */
username = XmTextFieldGetString(w);
removeBlanks(username);

/* Clear the Graphic Objects before re-stating them */
/* AccClearGraphicObjects(&AccData); */

XmTextFieldSetString(w,username);

if (! XmTextListItemExists(w, username))
   {
   /* Generate next available UID */
   AccountAssignAndDisplayUID(chooseuserid());
   /* Fetch and Assign Home Directory stub */
   AssignAndDisplayHomeDirStub();
   }
else
   {
   CreateAccEntNameEntered(w,username);
   CHANGES_MADE = False;
   }
}
/******************************************************************************
* CreateAccGetWidgetCB()       
******************************************************************************/
void CreateAccGetWidgetCB(w,widget_id,reason)
Widget	       w;
int	      *widget_id;
unsigned long *reason;

{
int         i;
Arg         args[10];

switch (*widget_id)
{
case ACC_USERNAME:
   AccData.usernameText = w;
break;
case ACC_TEMPLATE:
   AccData.tmpltnameText = w;
   XmTextListSetSensitive(AccData.tmpltnameText,FALSE);
break;
case ACC_TEMPLATE_COUNT:
   AccData.tmpltcountText = w;
break;
case ACC_PRI_GROUP:
   AccData.priGroups = w;
break;
case ACC_SHELL:
   AccData.shell = w;
break;
case ACC_HOME_DIR:
   AccData.directory = w;
break;
case ACC_UID:
   AccData.uidText = w;
break;
case ACC_COMMENTS:
   AccData.comments = w;
break;
case ACC_SEC_GROUPS: 
   AccData.secGroups = w;
   ForceSelListPolicy(w);
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
* OpenCreateUserAccount()
******************************************************************************/
void OpenCreateUserAccount(className,display)
char *className;
Display *display;
{
int n; Arg args[10];
/*
* If this is the first time we open this window...
*/
if (! CreateAccountScreenIsOpen())
   {
   MrmRegisterNames (reglist,(sizeof reglist/sizeof reglist[0]));
   MrmRegisterTextListClass();
   MrmRegisterSelectListClass();

   n = 0;
   XtSetArg(args[n], XmNallowShellResize, True);  n++;
   XtSetArg(args[n], XmNy, 115);  n++;
   acc_create_tl=XtAppCreateShell("Create User Accounts",className,
   			          topLevelShellWidgetClass,display,args,n);

   /* Replace the WM_DELETE_RESPONSE from f.kill with controlled close */
   AccCommonReplaceSystemMenuClose(acc_create_tl,CloseCreateAccountCB);

   InitializeWidgetIDsToNULL(&AccData);

   /* Load data structure attached to the main widget user data field */
   InitializeUserDataStructure(&AccData);

   /* Initialize Widget Retrieval Account Data Structure */
   AccCommonInitWidgetRetrieval(&AccData);

   NEW_VUIT_Manage("AccountCreateMW",&create_acc_mw,acc_create_tl);

   /* Initialize the graphic screen objects with necessary data */
   InitializeGraphicObjects(&AccData);

   /* Clear the Graphic Objects before re-stating them */
   AccClearGraphicObjects(&AccData);

   /* Set "Changes" Flag to False */
   CHANGES_MADE = False;

   XmTextListSetLoadRoutine(AccData.priGroups,CreateAccLoadPriGroupTXList);

   XmTextListSetLoadRoutine(AccData.usernameText,CreateAccLoadUsernameTXList);

   XtRealizeWidget(acc_create_tl);
   }
else
   {
   XtRealizeWidget(acc_create_tl);
   }
}
/******************************************************************************
* CloseCreateUserAccount()
******************************************************************************/
void CloseCreateUserAccount()
{
/* Clear object so next time screen os opened old data will be gone */
AccClearGraphicObjects(&AccData);

/* Set "Changes" Flag to False */
CHANGES_MADE = False;

XtUnrealizeWidget(acc_create_tl);
}
