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
static char *rcsid = "@(#)$RCSfile: AccTmplts.c,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/12/20 21:31:23 $";
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
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Mrm/MrmAppl.h>                /* Motif Toolkit and MRM */
#include <X11/X.h>
#include "XIsso.h"
#include "XMain.h"
#include "XAccounts.h"
#include "Utilities.h"
#include "Resources.h"
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

#define INVALID_GROUP_ID -100
/******************************************************************************
 * Static Global data
 ******************************************************************************/

/*
 * Set the top level widget to NULL to distinguish between having to Open
 * the Account Window and create the shell vs Close, Unrealize, Open again 
 * and merely re-Realize the Shell
 */
static Widget acc_modify_tl=NULL;
Widget acctmplt_mw=NULL;

/*
 * Static Function Declarations
 */
static Boolean TemplatePasswordDataErrorFree();
static Boolean TemplateNameErrorFree();

void AccTmpltModifyClose();

static AccountDataType AccData;

/*
 * Callback declarations
 */
void AccTmpltCloseCB();
void AccTmpltSaveCB();
void AccTmpltSaveNCloseCB();
void AccTmpltSelectTmpltCB();
void CreateTemplateCB();
void CreateTemplateNCloseCB();
void ModifyTemplateCB();
void ModifySysdfltCB();
void ModifySysdfltNCloseCB();
void ModifyTemplateNCloseCB();
void DeleteTemplateCB();
void CloseTemplateCB();
void AccTmpltsOpenAuditScreenCB();
void AccModTmpltHelpCB();

/*
 * Names and addresses of callback routines to register with Mrm
 */
static MrmRegisterArg reglist [] = {
{"AccTmpltCloseCB",              (caddr_t)AccTmpltCloseCB},
{"AccTmpltSelectTmpltCB",        (caddr_t)AccTmpltSelectTmpltCB},
{"AccTmpltSaveCB",               (caddr_t)AccTmpltSaveCB},
{"DeleteTemplateCB",             (caddr_t)DeleteTemplateCB},
{"AccTmpltsOpenAuditScreenCB",   (caddr_t)AccTmpltsOpenAuditScreenCB},
{"AccTmpltSaveNCloseCB",         (caddr_t)AccTmpltSaveNCloseCB}};

/*
 ==============================================================================
 = P R I V A T E   R O U T I N E S   T O   S U P P O R T   T E M P L A T E S
 ============================================================================== 
*/

/******************************************************************************
 * AccTmpltsLoadTmpltnameTXList()
 ******************************************************************************/
static void AccTmpltsLoadTmpltnameTXList()

{
LoadTmpltnameTXList(AccData.tmpltnameText);
}
/******************************************************************************
 * AccTmpltsLoadPriGroupTXList()
 ******************************************************************************/
static void AccTmpltsLoadPriGroupTXList()

{
LoadPriGroupTXList(AccData.priGroups);
}
/******************************************************************************
 * TailorAccScreenToTemplates()
 ******************************************************************************/
static void TailorAccScreenToTemplates()

{
XmTextListSetSensitive(AccData.usernameText,FALSE);
XtSetSensitive(AccData.usernameLBL,FALSE);
XtSetSensitive(AccData.comments,FALSE);
XtSetSensitive(AccData.commentsLBL,FALSE);
}
/******************************************************************************
 * AddTemplateCBs()
 ******************************************************************************/
static void AddTemplateCBs()

{
XtAddCallback(AccData.tmpltnameText,XmNactivateCallback,
              AccTmpltSelectTmpltCB,(caddr_t)NULL);
XtAddCallback(AccData.apply_pb,XmNactivateCallback,
              AccTmpltSaveCB,(caddr_t)NULL);
XtAddCallback(AccData.cancel_pb,XmNactivateCallback,
              AccTmpltCloseCB,(caddr_t)NULL);
XtAddCallback(AccData.ok_pb,XmNactivateCallback,
              AccTmpltSaveNCloseCB,(caddr_t)NULL);
XtAddCallback(AccData.help_pb,XmNactivateCallback,
              AccModTmpltHelpCB,(caddr_t)NULL);
}
/******************************************************************************
 * AssignTabGroups()
 ******************************************************************************/
static void AssignTabGroups()

{

XmTextListAddTabGroup(AccData.tmpltnameText);
XmTextListAddTabGroup(AccData.shell);
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
 * ModifySysdflt()
 ******************************************************************************/
static Boolean ModifySysdflt()

{
struct passwd      pwd;
struct group       grp;
char             **secGroup_names;
int                secGroup_count;
int                ret_code;
struct pr_passwd  *prpwd;
 
PostWorkingDialog("msg_acc_working_modify_template",acctmplt_mw,
                  NULL,NULL,NULL,NULL,NULL,NULL);

/* Get the default values (system default) */
AccRetrieveAndStoreDefault(&AccData,SYSTEM_DEFAULT,acctmplt_mw);

/* Get the Protected Password DB data pointer */
prpwd = &AccData.userInfo->userData.prpw;

/* Retrieve Data from graphic widgets */
if (! AccLoadDataFromScreen(&AccData,&pwd,&grp,&secGroup_names,&secGroup_count,
                            (int)ACC_TEMPLATES))
   {
   UnPostWorkingDialog(acctmplt_mw);
   return(False);
   }

ModifySysDefaultForDBSave(&AccData);

if (XWriteSystemInfo(&AccData.defaultData,acctmplt_mw) != SUCCESS)
   {
   UnPostWorkingDialog(acctmplt_mw);
   return(False);
   }
 
UnPostWorkingDialog(acctmplt_mw);

return(True);
}
/******************************************************************************
 * DeleteTemplate()
 ******************************************************************************/
static void DeleteTemplate()

{
char             **secGroup_names;
struct pr_passwd  *prpwd;
struct passwd      pwd;
int                ret_code;
char aud_msg_buf[200];

if (TemplateReferenceCount(XmTextFieldGetString(AccData.tmpltnameText)) != 0)
   {
   PostErrorDialog("msg_error_template_refcount",acctmplt_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   return;
   }

PostWorkingDialog("msg_acc_working_delete_template",acctmplt_mw,
                  NULL,NULL,NULL,NULL,NULL,NULL);

/* Get the Protected Password DB data pointer */
prpwd = &AccData.userInfo->userData.prpw;

/* Load the Template name */
strcpy(prpwd->ufld.fd_name,XmTextFieldGetString(AccData.tmpltnameText));

/* Do Not Delete System Default */
if (strcmp(SYSTEM_DEFAULT_TEMPLATE,prpwd->ufld.fd_name) == 0)
   {
   UnPostWorkingDialog(acctmplt_mw);
   PostErrorDialog("msg_error_cannot_delete_dflt",acctmplt_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   return;
   }

if (removeBlanks(prpwd->ufld.fd_name))
   XmTextSetString(AccData.tmpltnameText,prpwd->ufld.fd_name);

/* Assign the template name to the password structure */
pwd.pw_name = XmTextFieldGetString(AccData.tmpltnameText);

/* Set fg_name == 0 so protected DB entry will be deleted */
prpwd->uflg.fg_name = 0;

/* Set the is_template flag to 0 so library thinks we are deleting user */
prpwd->uflg.fg_istemplate = 1;
prpwd->ufld.fd_istemplate = 0;

/* Delete the Protected Password Database Entry */
ret_code = XWriteUserInfo(&AccData.userInfo->userData,acctmplt_mw);

if (ret_code == SUCCESS)
   {
   /* Delete the /etc/passwd entry */
   ret_code = XDeleteUserpwd(&pwd,acctmplt_mw);
   if (ret_code == SUCCESS)
      {
      /* Delete the group information (/etc/group) */
      /* Pass an invalid primary group and no secondary groups */
      secGroup_names = (char **)Malloc(sizeof(char *) * (1));
      (secGroup_names)[0] = (char *) strdup("\0");
      ret_code = XModifyUserGroup(pwd.pw_name,INVALID_GROUP_ID,
                                  secGroup_names, acctmplt_mw);
      XtFree((char *)secGroup_names);
      if (ret_code == SUCCESS)
         {
         AccClearGraphicObjects(&AccData);
         sprintf(aud_msg_buf,"Template Account (%s) Deleted",pwd.pw_name);
         WriteSingleAuditString(aud_msg_buf);
         }
      else
         {
         PostErrorDialog("msg_error_delete_etcgroup",acctmplt_mw,NULL,NULL,
                         NULL,NULL,NULL,NULL);
         }
      }
   else
      {
      /* Do we get more specific error codes ? */
      PostErrorDialog("msg_error_delete_etcpasswd",acctmplt_mw,NULL,NULL,NULL,
                      NULL,NULL,NULL);
      }
   }
else
   {
   /* Do we get more specific error codes ? */
   PostErrorDialog("msg_error_delete_template",acctmplt_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }

UnPostWorkingDialog(acctmplt_mw);
}
/******************************************************************************
 * ModifyTemplate()
 ******************************************************************************/
static Boolean ModifyTemplate()

{
struct passwd      pwd;
struct group       grp;
char             **secGroup_names;
int                secGroup_count;
int                ret_code;
struct pr_passwd  *prpwd;
char aud_msg_buf[200];
 
PostWorkingDialog("msg_acc_working_modify_template",acctmplt_mw,
                  NULL,NULL,NULL,NULL,NULL,NULL);

/* Set the RELEVANT (screen manipulated) user flags to 0 */
AccSetAllUserFieldFlags(&AccData,0);

/* Get the default values (system default) */
AccRetrieveAndStoreDefault(&AccData,SYSTEM_DEFAULT,acctmplt_mw);

/* Get the Protected Password DB data pointer */
prpwd = &AccData.userInfo->userData.prpw;

/* Retrieve Data from graphic widgets */
if (! AccLoadDataFromScreen(&AccData,&pwd,&grp,&secGroup_names,&secGroup_count,
                            (int)ACC_TEMPLATES))
   {
   /* Free alloced data retrieved from screen widgets and not getpwname() */
   AccFreePasswordDataStructure(&pwd);
   AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

   UnPostWorkingDialog(acctmplt_mw);
   return(False);
   }

AccAssignSmartFlagsForDBSave(&AccData,ACC_TEMPLATES);

/*
 * pwd structure already holds the template name
 */
if ((! AccData.userInfo->username) ||
    strcmp(AccData.userInfo->username,pwd.pw_name) != 0)
   {
   XtFree(AccData.userInfo->username);

   AccData.userInfo->username =
      (char *)XtMalloc(sizeof(char *) * (strlen(pwd.pw_name) + 1));
   if (AccData.userInfo->username != NULL)
      strcpy(AccData.userInfo->username,pwd.pw_name);
   }

/* Set field = 1 so WriteUserInfo will know this is a template */
prpwd->ufld.fd_istemplate = 1;

/* Fill in miscellaneous fields */
pwd.pw_passwd = "*";
#ifdef AUX
pwd.pw_age = "";
#endif
pwd.pw_comment = "";

if (! TemplatePasswordDataErrorFree(pwd))
   {
   /* Free alloced data retrieved from screen widgets and not getpwname() */
   AccFreePasswordDataStructure(&pwd);
   AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

   UnPostWorkingDialog(acctmplt_mw);
   return(False);
   }

/* Create /etc/passwd & /tcb/files/auth */
ret_code = XModifyUserpwd(&pwd,acctmplt_mw); 

if (ret_code == SUCCESS)    /* Update the group information (/etc/group) */
   ret_code = XModifyUserGroup(pwd.pw_name,(int)pwd.pw_gid,secGroup_names,
                              acctmplt_mw);

if (ret_code == SUCCESS)
   ret_code = XWriteUserInfo(&AccData.userInfo->userData,acctmplt_mw);

if (ret_code == SUCCESS)
   {
   /* Self Audit if a template account was locked */
   if ((prpwd->uflg.fg_lock == 1) && (prpwd->ufld.fd_lock == 1))
      {
      sprintf(aud_msg_buf,"Template Account (%s) Locked",pwd.pw_name);
      WriteSingleAuditString(aud_msg_buf);
      }
   /* Self Audit if a template account was unlocked */
   else if ((prpwd->uflg.fg_lock == 1) && (prpwd->ufld.fd_lock == 0))
      {
      sprintf(aud_msg_buf,"Template Account (%s) Unlocked",pwd.pw_name);
      WriteSingleAuditString(aud_msg_buf);
      }
   }

#ifdef SEC_PRIV
if (ret_code == SUCCESS)
   ret_code = write_authorizations(prpwd->ufld.fd_name,
                                   AccData.userInfo->command_auths,
                                   AccData.userInfo->ncommand_auths);
/*
 * ignore error as it is not yet understood and operation seems to work
 *
if (ret_code != SUCCESS)
   PostErrorDialog("msg_error_write_auths",acctmplt_mw,
                   NULL,NULL,NULL,NULL,NULL,NULL);
 *
 */
#endif /* SEC_PRIV */
 
/* Free alloced data retrieved from screen widgets and not getpwname() */
AccFreePasswordDataStructure(&pwd);
AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

UnPostWorkingDialog(acctmplt_mw);

if (ret_code == SUCCESS)
   return(True);
else return(False);
}
/******************************************************************************
 * CreateTemplate()
 ******************************************************************************/
static Boolean CreateTemplate()

{
struct passwd      pwd;
struct group       grp;
char             **secGroup_names;
int                secGroup_count;
int                ret_code;
struct pr_passwd  *prpwd;
int                mod_state[NUM_MOD_STATES] = {0,0,0,0};
char aud_msg_buf[200];
 
PostWorkingDialog("msg_acc_working_create_template",acctmplt_mw,
                  NULL,NULL,NULL,NULL,NULL,NULL);

/* Set the RELEVANT (screen manipulated) user flags to 0 */
AccSetAllUserFieldFlags(&AccData,0);

/* Get the default values (system default) */
AccRetrieveAndStoreDefault(&AccData,SYSTEM_DEFAULT,acctmplt_mw);

/* Get the Protected Password DB data pointer */
prpwd = &AccData.userInfo->userData.prpw;

/* 
 * Design Problem: 
 *
 * How do we know whether the Audit Mask and Control Values have been
 * assigned or inherited as there is no good starting point to flag as
 * the beginning of template creation data. The easiest thing to do for
 * now seems to be to force the template to be created with system default
 * audit values and allow there manipulation through template modification.
 */

/* Assign the default audit mask and control values (see above) */
strcpy(prpwd->ufld.fd_auditdisp,AccData.dflt.auditdisp);
prpwd->ufld.fd_auditcntl = AccData.dflt.auditcntl;

/* Retrieve Data from graphic widgets */
if (! AccLoadDataFromScreen(&AccData,&pwd,&grp,&secGroup_names,&secGroup_count,
                            (int)ACC_TEMPLATES))
   {
   /* Free alloced data retrieved from screen widgets and not getpwname() */
   AccFreePasswordDataStructure(&pwd);
   AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

   UnPostWorkingDialog(acctmplt_mw);
   return(False);
   }

AccAssignSmartFlagsForDBSave(&AccData,(int)ACC_TEMPLATES);

/* Assign Template Name to userInfo structure if not already assigned or NULL */
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

if ((! TemplatePasswordDataErrorFree(pwd)) ||
    (! TemplateNameErrorFree(prpwd->ufld.fd_template)))
   {
   /* Free alloced data retrieved from screen widgets and not getpwname() */
   AccFreePasswordDataStructure(&pwd);
   AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

   UnPostWorkingDialog(acctmplt_mw);
   return(False);
   }

/* Create the Template */

/* Create /etc/passwd and /tcb/files/auth */
ret_code = XCreateUserTemplate(&pwd,prpwd->ufld.fd_template,acctmplt_mw,
                               mod_state);

if (ret_code == SUCCESS)    /* Update the group information (/etc/group) */
   ret_code = XModifyUserGroup(pwd.pw_name,(int)pwd.pw_gid,secGroup_names,
                               acctmplt_mw);

if (ret_code == SUCCESS)
   {
   /* Set a flag to indicate the /etc/group file has been modified */
   mod_state[MOD_ETC_GROUP] = 1;

   /*
    * No need to set a flag here as the pr_passwd file was created
    * in XCreateUserAccount.
    */
   ret_code = XWriteUserInfo(&AccData.userInfo->userData,acctmplt_mw);
   }

#ifdef SEC_PRIV
if (ret_code == SUCCESS)
   /* ret_code = */write_authorizations(prpwd->ufld.fd_name,
                                   AccData.userInfo->command_auths,
                                   AccData.userInfo->ncommand_auths);
/*
 * ignore error as it is not yet understood and operation seems to work
 *
if (ret_code != SUCCESS)
   PostErrorDialog("msg_error_write_auths",acctmplt_mw,
                   NULL,NULL,NULL,NULL,NULL,NULL);
 *
 */
#endif /* SEC_PRIV */

/* If an error occured, remove all traces of the account */
if (ret_code != SUCCESS)
   AccCommonDeleteAccount(pwd,&AccData,acctmplt_mw,mod_state);
/* Reload as it needs to be in the list to open the audit events screen */
else LoadTmpltnameTXList(AccData.tmpltnameText);
 
/* Free alloced data retrieved from screen widgets and not getpwname() */
AccFreePasswordDataStructure(&pwd);
AccFreeGroupDataStructure(&secGroup_names,secGroup_count);

UnPostWorkingDialog(acctmplt_mw);

if (ret_code == SUCCESS)
   {
   sprintf(aud_msg_buf,"Template Account (%s) Created",pwd.pw_name);
   WriteSingleAuditString(aud_msg_buf);
   return(True);
   }
else 
   {
   sprintf(aud_msg_buf,"FAILURE: Template Account (%s) Creation error",
           pwd.pw_name);
   WriteSingleAuditString(aud_msg_buf);
   }
   return(False);
}
/******************************************************************************
 * TemplateNameErrorFree()
 ******************************************************************************/
static Boolean TemplateNameErrorFree(name)
char *name;

{

/* Check that template name is valid. No ":" */
if (CheckValidName(name) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_name",acctmplt_mw,NULL,NULL,
                   NULL,NULL,NULL,NULL);
   return(False);
   }

return(True);
}
/******************************************************************************
 * TemplatePasswordDataErrorFree()
 ******************************************************************************/
static Boolean TemplatePasswordDataErrorFree(pwd)
struct passwd pwd;

{
if (CheckValidShell(pwd.pw_shell) != SUCCESS)
   {
   PostErrorDialog("msg_acc_error_mkusr_invalid_shell",acctmplt_mw,NULL,NULL,
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
 * AccountModTmpltHelpCB()
 ******************************************************************************/
void AccModTmpltHelpCB (w, client_data, call_data)
Widget  w;
caddr_t client_data;
caddr_t call_data;
{
HyperHelpDisplay(MOD_TEMP_BOX);
}

/******************************************************************************
 * TemplateDeleteCB()
 ******************************************************************************/
void TemplateDeleteCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
DeleteTemplate();
}
/******************************************************************************
 * DeleteTemplateCB()
 ******************************************************************************/
void DeleteTemplateCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
PostQuestionDialog("msg_acc_delete_template",acctmplt_mw,TemplateDeleteCB,
                   NULL,NULL,NULL,NULL,NULL);
}
/******************************************************************************
 * ModifyTemplateNCloseCB()
 ******************************************************************************/
void ModifyTemplateNCloseCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
if (! ModifyTemplate())
   return;

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_template",acctmplt_mw,CloseTemplateCB,
                      NULL,NULL,NULL,NULL,NULL);
else
   AccTmpltModifyClose();
}
/******************************************************************************
 * ModifySysdfltNCloseCB()
 ******************************************************************************/
void ModifySysdfltNCloseCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
if (! ModifySysdflt())
   return;

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_template",acctmplt_mw,CloseTemplateCB,
                      NULL,NULL,NULL,NULL,NULL);
else
   AccTmpltModifyClose();
}
/******************************************************************************
 * ModifySysdfltCB()
 ******************************************************************************/
void ModifySysdfltCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
ModifySysdflt();
}
/******************************************************************************
 * ModifyTemplateCB()
 ******************************************************************************/
void ModifyTemplateCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
ModifyTemplate();
}
/******************************************************************************
 * CreateTemplateNCloseCB()
 ******************************************************************************/
void CreateTemplateNCloseCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
if (! CreateTemplate())
   return;

if (ApplicationResourcesInfo.questionAllActions)
   PostQuestionDialog("msg_acc_close_template",acctmplt_mw,CloseTemplateCB,
                      NULL,NULL,NULL,NULL,NULL);
else
   AccTmpltModifyClose();
}
/******************************************************************************
 * CreateTemplateCB()
 ******************************************************************************/
void CreateTemplateCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
CreateTemplate();
}
/******************************************************************************
 * CloseTemplateCB()
 ******************************************************************************/
void CloseTemplateCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{

/* Clear all fields so next time screen is managed it won't display old data */
AccClearGraphicObjects(&AccData);

XtUnrealizeWidget(acc_modify_tl);
}
/******************************************************************************
 * AccTmpltCloseCB()
 ******************************************************************************/
void AccTmpltCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
int data_lost;

/* Check for data that has been changed and will be lost */
data_lost = 0;

if (data_lost)
   PostQuestionDialog("msg_acc_close_template_lose",acctmplt_mw,
                      CloseTemplateCB,NULL,NULL,NULL,NULL,NULL);
else 
   {
   if (ApplicationResourcesInfo.questionAllActions)
      PostQuestionDialog("msg_acc_close_template",acctmplt_mw,
                         CloseTemplateCB,NULL,NULL,NULL,NULL,NULL);
   else
      AccTmpltModifyClose();
   }
}
/******************************************************************************
 * AccTmpltSaveCB()
 ******************************************************************************/
void AccTmpltSaveCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{
char *tmpltname;

tmpltname = XmTextFieldGetString(AccData.tmpltnameText);

if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",acctmplt_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else 
   {
   if (strcmp(tmpltname,SYSTEM_DEFAULT_TEMPLATE) == 0)
      {
      if (ApplicationResourcesInfo.questionAllActions)
         PostQuestionDialog("msg_acc_modify_sysdflt",acctmplt_mw,
                            ModifySysdfltCB,NULL,NULL,NULL,NULL,NULL);
      else
         ModifySysdflt();
      }
   else if (TemplateIsNew(tmpltname))
      {
      if (ApplicationResourcesInfo.questionAllActions)
         PostQuestionDialog("msg_acc_create_template",acctmplt_mw,
                            CreateTemplateCB,NULL,NULL,NULL,NULL,NULL);
      else 
         CreateTemplate();
      }
   else
      {
      if (ApplicationResourcesInfo.questionAllActions)
         PostQuestionDialog("msg_acc_modify_template",acctmplt_mw,
                            ModifyTemplateCB,NULL,NULL,NULL,NULL,NULL);
      else
         ModifyTemplate();
      }
   }
}
/******************************************************************************
 * AccTmpltSaveNCloseCB()
 ******************************************************************************/
void AccTmpltSaveNCloseCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;

{
char *tmpltname;

tmpltname = XmTextFieldGetString(AccData.tmpltnameText);

if (CheckPasswordAccess() != SUCCESS)
   {
   PostErrorDialog("msg_acc_passwd_no_access",acctmplt_mw,NULL,NULL,NULL,
                   NULL,NULL,NULL);
   }
else 
   {
   if (strcmp(tmpltname,SYSTEM_DEFAULT_TEMPLATE) == 0)
      {
      if (ApplicationResourcesInfo.questionAllActions)
         PostQuestionDialog("msg_acc_modify_sysdflt",acctmplt_mw,
                            ModifySysdfltNCloseCB,NULL,NULL,NULL,NULL,NULL);
      else
         {
         if (ModifySysdflt())
            AccTmpltModifyClose();
         }
      }
   else if (TemplateIsNew(tmpltname))
      {
      if (ApplicationResourcesInfo.questionAllActions)
         PostQuestionDialog("msg_acc_create_template",acctmplt_mw,
                            CreateTemplateNCloseCB,NULL,NULL,NULL,NULL,NULL);
      else
         {
         if (CreateTemplate())
            AccTmpltModifyClose();
         }
      }
   else
      {
      if (ApplicationResourcesInfo.questionAllActions)
         PostQuestionDialog("msg_acc_modify_template",acctmplt_mw,
                            ModifyTemplateNCloseCB,NULL,NULL,NULL,NULL,NULL);
      else
         {
         if (ModifyTemplate())
            AccTmpltModifyClose();
         }
      }
   }
}
/******************************************************************************
 * AccTmpltSelectTmpltCB()
 ******************************************************************************/
void AccTmpltSelectTmpltCB(w, tag, reason)
Widget	       w;
int	      *tag;
unsigned long *reason;
{
char          *username;

/* Get Username */
username = XmTextFieldGetString(w);

AccEntNameEntered(&AccData,w,username,(int)ACC_TEMPLATES);
}
/******************************************************************************
 * AccTmpltsOpenAuditScreenCB()
 ******************************************************************************/
void AccTmpltsOpenAuditScreenCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
AccOpenAuditEventsScreen(&AccData,XmTextFieldGetString(AccData.tmpltnameText),
                         ACC_TEMPLATES,acctmplt_mw);
}

/*
 ==============================================================================
 = E N T R Y   P O I N T   T O   M O D I F Y   U S E R   A C C O U N T S
 ============================================================================== 
*/

/******************************************************************************
 * AccTmpltModifyOpen()
 ******************************************************************************/
void AccTmpltModifyOpen(className,display)
char    *className;
Display *display;
{
int n; Arg args[10];
/*
 * If this is the first time we open this window...
 */
if (! acc_modify_tl)
   {
   MrmRegisterNames (reglist,(sizeof reglist/sizeof reglist[0]));

   n = 0;
   XtSetArg(args[n], XmNallowShellResize, True);  n++;
   XtSetArg(args[n], XmNy, 115);  n++;
   acc_modify_tl=XtAppCreateShell("Modify Account Templates",className,
                                  topLevelShellWidgetClass,
                                  display,args,n);

   /* Replace the WM_DELETE_RESPONSE from f.kill with controlled close */
   AccCommonReplaceSystemMenuClose(acc_modify_tl,CloseTemplateCB);

   /* Load data structure attached to the main widget user data field */
   InitializeUserDataStructure(&AccData);

   /* Initialize Widget Retrieval Account Data Structure */
   AccCommonInitWidgetRetrieval(&AccData);

   NEW_VUIT_Manage(ACCTMPLT_WIDGET,&acctmplt_mw,acc_modify_tl);

   /* Initialize the graphic screen objects with necessary data */
   InitializeGraphicObjects(&AccData);

   /* Tailor the stock account screen to Template nuances */
   TailorAccScreenToTemplates();

   AssignTabGroups();

   /* Add Callbacks */
   AddTemplateCBs();

   /* Clear the Graphic Objects before re-stating them */
   AccClearGraphicObjects(&AccData);

   XmTextListSetLoadRoutine(AccData.tmpltnameText,AccTmpltsLoadTmpltnameTXList);

   XmTextListSetLoadRoutine(AccData.priGroups,AccTmpltsLoadPriGroupTXList);

   XtRealizeWidget(acc_modify_tl);
   }
else
   {
   XtRealizeWidget(acc_modify_tl);
   }
}
/******************************************************************************
 * AccTmpltModifyClose()
 ******************************************************************************/
void AccTmpltModifyClose()
{
/* Clear all fields so next time screen is managed it won't display old data */
AccClearGraphicObjects(&AccData);

XtUnrealizeWidget(acc_modify_tl);
}
