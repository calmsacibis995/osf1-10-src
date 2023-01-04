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
static char *rcsid = "@(#)$RCSfile: AccCommon.c,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/12/20 21:30:58 $";
#endif

/******************************************************************************* *
 *
 *
 ******************************************************************************/

#include <stdio.h>                      /* For printf and so on. */
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <X11/cursorfont.h>
#include <Xm/TextF.h>
#include <Mrm/MrmAppl.h>                /* Motif Toolkit and MRM */
#include <X11/X.h>
#include <Xm/Xm.h>
#include <X11/Protocols.h>

#ifdef SEC_MAC
#include "labels.h"
#endif /* SEC_MAC */

#include "XIsso.h"
#include "XMain.h"
#include "XAccounts.h"
#include "Utilities.h"
#include "AccountSelection.h"
#include "AccCommon.h"
#include "Messages.h"
#include "Vuit.h"

extern Widget accounts_mw;
extern Widget acctmplt_mw;

/*
 ==============================================================================
 = P R I V A T E   D E F I N E S     
 ============================================================================== 
*/
#define TEMPLATE_HOME_DIR  ""
#define TEMPLATE_COMMENT   "";

/* File containing inventory of availale shells */
#define SHELL_INVENTORY_FILE 	"/etc/shells"

/*
 * Forward Callback declarations
 */
void AccCommonGetWidgetCB();
void AccCommonCreateMWCB();

static MrmRegisterArg reglist [] = {
{"AccCommonGetWidgetCB",             (caddr_t)AccCommonGetWidgetCB},
{"AccCommonCreateMWCB",              (caddr_t)AccCommonCreateMWCB}};

/*
 ==============================================================================
 = S T A T I C   G L O B A L S       
 ============================================================================== 
*/
static AccountDataType *pWidgetRetrievalAccData;

#define NUM_MONTHS 12
char *months[NUM_MONTHS] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
                            "Sep","Oct","Nov","Dec"};
#define NUM_DAYS 7
char *days[NUM_DAYS]     = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

#define NUM_VALID_UUCP_STRINGS 10
char *valid_uucp_strings[NUM_VALID_UUCP_STRINGS]= 
      {"Su","Mo","Tu","We","Th","Fr","Sa","Wk","Any","Never"};
/*
 ==============================================================================
 = P R I V A T E   R O U T I N E S   
 ============================================================================== 
*/

/******************************************************************************
 * TimeStringToSegments()          
 ******************************************************************************/
static void TimeStringToSegments(time_str, time_seg)
char *time_str;
char time_seg[8][10];

{
int   i;
char *ptr;
char *time_str_ptr;
char  time_str_tmp[50];

i = 0;

/* copy the time string as we will be altering it */
strcpy(time_str_tmp,time_str);

time_str_ptr = &(time_str_tmp[0]);

/* Set ptr to the beginning of time_str_ptr */
ptr = time_str_ptr;

while ((time_str_ptr = strpbrk(time_str_ptr,": ")) != (char *)NULL)
      {
      time_str_ptr[0] = '\0';
      strcpy(time_seg[i],ptr); i++;
      
      /* Set ptr to the beginning of the next segment */
      ptr = &(time_str_ptr[1]);

      /* increment time_str_ptr to pass the NULL */
      time_str_ptr = ptr; 
      } 

/* Handle the last segment */ 
strcpy(time_seg[i],ptr); 
}
/******************************************************************************
 * IsValidYear()          
 ******************************************************************************/
static Boolean IsValidYear(str,year)
char *str;
int  *year;

{

/* Check for minute of hour */
*year = atoi(str);

if ((*year >= 1900) && (*year < 2100))  
   {
   return(True);
   }

return(False);
}
/******************************************************************************
 * IsMinuteofHour()          
 ******************************************************************************/
static Boolean IsMinuteofHour(str,minute)
char *str;
int  *minute;

{

/* Check for minute of hour */
*minute = atoi(str);

if ((*minute >= 0) && (*minute < 60))  
   return(True);

return(False);
}
/******************************************************************************
 * IsHourofDay()          
 ******************************************************************************/
static Boolean IsHourofDay(str,hour)
char *str;
int  *hour;

{

/* Check for day of the month */
*hour = atoi(str);
if (*hour == 24)
   *hour = 0;

if ((*hour >= 0) && (*hour < 24))  
   return(True);

return(False);
}
/******************************************************************************
 * IsDayofMonth()          
 ******************************************************************************/
static Boolean IsDayofMonth(str,dayofmonth)
char *str;
int  *dayofmonth;

{

/* Check for day of the month */
*dayofmonth = atoi(str);

if ((*dayofmonth > 0) && (*dayofmonth < 32))  
   {
   return(True);
   }

return(False);
}
/******************************************************************************
 * IsMonth()          
 ******************************************************************************/
static Boolean IsMonth(str,month)
char *str;
int  *month;

{
int i;

/* Check for month */
for (i=0; i<NUM_MONTHS; i++)
    if (strcasecmp(str,months[i]) == 0)
       {
       *month = i;
       return(True);
       }

return(False);
}
/******************************************************************************
 * IsDayofWeek()          
 ******************************************************************************/
static Boolean IsDayofWeek(str,weekday)
char *str;
int  *weekday;

{
int i;

/* Check for day of the week */
for (i=0; i<NUM_DAYS; i++)
    if (strcasecmp(str,days[i]) == 0)
       {
       *weekday = i;
       return(True);
       }

return(False);
}
/******************************************************************************
 * StringToTime()          
 ******************************************************************************/
static Boolean StringToTime(time_str,time,type)
char   *time_str;
time_t *time;
int     type;

{
char       time_seg[8][10];
int        flag;
struct tm  tm_data;

TimeStringToSegments(time_str, time_seg);

if (! IsMonth(time_seg[0],&tm_data.tm_mon))
   flag = False;

if (! IsDayofMonth(time_seg[1],&tm_data.tm_mday))
   flag = False;

if (! IsHourofDay(time_seg[2],&tm_data.tm_hour))
   flag = False;

if (! IsMinuteofHour(time_seg[3],&tm_data.tm_min))
   flag = False;

if (IsValidYear(time_seg[4],&tm_data.tm_year))
   tm_data.tm_year = tm_data.tm_year - 1900;
else flag = False;

tm_data.tm_sec = 0;     /* initialize to 0 or mktime will barf */

tm_data.tm_isdst = - 1; /* negative value means check for local DLST info */

if (flag == False)
   {
   *time = (time_t) 0;
   if (type == ACC_USERS)
      PostErrorDialog("msg_error_invalid_exp_date",accounts_mw,NULL,NULL,NULL,
                      NULL,NULL,NULL);
   else
      PostErrorDialog("msg_error_invalid_exp_date",acctmplt_mw,NULL,NULL,NULL,
                      NULL,NULL,NULL);
   return(False);
   }
else
   {
   *time = mktime(&tm_data);
   }
}
/******************************************************************************
 * IsValidUUCPTimeFormat()          
 ******************************************************************************/
Boolean IsValidUUCPTimeFormat(time_str,type)          
char *time_str;
int   type;

{
int i,j;
char str[100];
char *strptr;

/* Make a copy of the time string to play with */
strcpy(str,time_str);

/* Set all known strings to spaces */
for (i=0; i<NUM_VALID_UUCP_STRINGS; i++)
    {
    while ((strptr=strstr(str,valid_uucp_strings[i])) != NULL)
          {
          for (j=0; j<strlen(valid_uucp_strings[i]); j++)
              strptr[j] = ' ';
          } 
    }

/* Set all '-' to spaces */
while ((strptr=strrchr(str,'-')) != NULL)
      strptr[0] = ' ';

/* Set all ',' to spaces */
while ((strptr=strrchr(str,',')) != NULL)
      strptr[0] = ' ';

/* We should be left with only digits, if not...error */
for (i=0; i<strlen(str); i++)
    if ((str[i] != ' ') && (! isdigit(str[i])))
       {
       if (type == ACC_USERS)
          PostErrorDialog("msg_error_invalid_tod",accounts_mw,NULL,NULL,NULL,
                          NULL,NULL,NULL);
       else
          PostErrorDialog("msg_error_invalid_tod",acctmplt_mw,NULL,NULL,NULL,
                           NULL,NULL,NULL);
       return(False);
       }
      
return(True);
}
/******************************************************************************
 * CreateUserData()          
 ******************************************************************************/
void CreateUserData(pAccData)
AccountDataType *pAccData;

{

pAccData->userInfo = (pAccSelCommon)XtMalloc(sizeof(AccSelCommon));
if (! pAccData->userInfo)
   {
   /* Memory Error */
   printf("Can't allocate userInfo structure!!!\n");
   fflush(stdout);
   exit(0);
   }
pAccData->dataLoaded = False;

pAccData->userInfo->UserList = NULL;
pAccData->userInfo->username = NULL;
pAccData->userInfo->tmpltname = NULL;

pAccData->userInfo->command_auths = NULL;

pAccData->apply = NULL;
pAccData->userInfo->user_xmstrings = NULL;
pAccData->userInfo->user_names = NULL;
pAccData->userInfo->user_count = 0;
}
/******************************************************************************
 * TemplateNameExits()
 ******************************************************************************/
static Boolean TemplateNameExists(tmpltname)
char    *tmpltname;

{
struct passwd *pwdp;

/* Should we check protected password database also? */

if ((pwdp = getpwnam(tmpltname)) == (struct passwd *) 0)
   {
   return (False);
   }
else
   {
   return (True);
   }
}
/******************************************************************************
 * UserNameExits()
 ******************************************************************************/
static Boolean UserNameExists(username)
char    *username;

{
/* Should we check protected password database also? */

if (getpwnam(username) == (struct passwd *) 0)
   return (False);
else
   return (True);
}
/******************************************************************************
 * UserUIDExits()
 ******************************************************************************/
static Boolean UserUidExists(userid)
int     userid;

{
if (userid && (getpwuid ((uid_t) userid) != (struct passwd *) 0))
   return (True);
else
   return (False);
}
/******************************************************************************
 * NameInNameList()           
 ******************************************************************************/
static Boolean NameInNameList(target_name,name_list,count)
char  *target_name;
char **name_list;
int    count;

{
int i;

for (i = 0; i < count; i++)
    {
    if (strcmp(target_name,name_list[i]) == 0)
       return(True);
    }
return(False);
}
/******************************************************************************
 * AccLoadPrivsFromScreen()
 ******************************************************************************/
static Boolean AccLoadPrivsFromScreen(pAccData)
AccountDataType *pAccData;

{
char             **names;
int                i,count;
struct pr_passwd  *prpwd;

prpwd = &pAccData->userInfo->userData.prpw;

XmSelectListGetItems(pAccData->baseList,&names,&count);
   
for (i = 0; i < pAccData->nprivs; i++)
    {
    if (NameInNameList(sys_priv[i].name,names,count))
       {
       ADDBIT(prpwd->ufld.fd_bprivs, sys_priv[i].value);
       }
    else
       {
       RMBIT(prpwd->ufld.fd_bprivs, sys_priv[i].value);
       }
    }

if (names)
   XtFree((char *)names);

XmSelectListGetItems(pAccData->kernelList,&names,&count);
   
for (i = 0; i < pAccData->nprivs; i++)
    {
    if (NameInNameList(sys_priv[i].name,names,count))
       {
       ADDBIT(prpwd->ufld.fd_sprivs, sys_priv[i].value);
       }
    else
       {
       RMBIT(prpwd->ufld.fd_sprivs, sys_priv[i].value);
       }
    }

if (names)
   XtFree((char *)names);

return(True);
}
/******************************************************************************
 * AccLoadCommandAuthsFromScreen()           
 ******************************************************************************/
static Boolean AccLoadCommandAuthsFromScreen(pAccData)
AccountDataType *pAccData;

{
char             **names;
int                i,k,count;
struct pr_passwd  *prpwd;

prpwd = &pAccData->userInfo->userData.prpw;

if (pAccData->userInfo->command_auths)
   {
   free_cw_table(pAccData->userInfo->command_auths);
   pAccData->userInfo->command_auths = NULL;
   }

pAccData->userInfo->command_auths=alloc_table(cmd_priv,pAccData->nauths);

XmSelectListGetItems(pAccData->authList,&names,&count);

k=0;
for (i = 0; i < pAccData->nauths; i++)
    {
    if (NameInNameList(cmd_priv[i].name,names,count))
       {
       ADDBIT(prpwd->ufld.fd_cprivs, cmd_priv[i].value);
       strcpy(pAccData->userInfo->command_auths[k++],cmd_priv[i].name);
       }
    else
       {
       RMBIT(prpwd->ufld.fd_cprivs, cmd_priv[i].value);
       }
    }
pAccData->userInfo->ncommand_auths = k;

return(True);
}

#ifdef SEC_MAC

/******************************************************************************
* AccLoadClearanceFromScreen()
******************************************************************************/
static Boolean AccLoadClearanceFromScreen(pAccData, type)
AccountDataType *pAccData;
int              type;

{
struct pr_passwd  *prpwd;
mand_ir_t         *pclearance;

prpwd = &pAccData->userInfo->userData.prpw;

if (pAccData->clearance_tx)
   {
   pclearance=clearance_er_to_ir(
              (char *)XmTextGetString(pAccData->clearance_tx));

   if (pclearance != (mand_ir_t *)NULL)
      {
      mand_copy_ir(pclearance,&(prpwd->ufld.fd_clearance));

      mand_free_ir(pclearance); 

      return(True);
      }
   else
      {
      if (type == ACC_USERS)
         PostErrorDialog("msg_acc_error_clearance",accounts_mw,NULL,NULL,NULL,
                         NULL,NULL,NULL);
      else
         PostErrorDialog("msg_acc_error_clearance",acctmplt_mw,NULL,NULL,NULL,
                         NULL,NULL,NULL);
      return(False);
      }
   }
}

#endif /* SEC_MAC */

/******************************************************************************
 * AccLoadSecGroupDataFromScreen()           
 ******************************************************************************/
Boolean AccLoadSecGroupDataFromScreen(pAccData,names,count)
AccountDataType  *pAccData;
char           ***names;
int              *count;

{
int    i;
char **groupNames;

XmSelectListGetItems(pAccData->secGroups,&groupNames,count);

/* 
 * The list of group names will eventually be passed to XModifyUserGroup
 * which expects the array to be one size greater than "count" and NULL
 * terminated.
 * 
 * We must therefore postprocess the array...(AccModUser.c: modify_acc_mod_user
 */
*names = (char **)Malloc(sizeof(char *) * (*count + 1));
for (i=0; i<*count; i++)
    {
    (*names)[i] = (char *) strdup(groupNames[i]);
    }
(*names)[*count] = (char *) strdup("\0");

/* Free the groupNames array as it has been copied */
AccFreeGroupDataStructure(&groupNames,*count);

return(True);
}
/******************************************************************************
 * AccStoreSystemDefault()
 ******************************************************************************/
int AccStoreSystemDefault(pAccData,defname,w)
AccountDataType *pAccData;
char            *defname;
Widget           w;

{
int i;
struct pr_default *prdf;

if (XGetSystemInfo(&pAccData->defaultData,w) != SUCCESS)
   return(0);

prdf = &pAccData->defaultData.df;

/* Template name */
if (prdf->prg.fg_template)
   strcpy(pAccData->dflt.tmpltname,prdf->prd.fd_template);
else 
   strcpy(pAccData->dflt.tmpltname,"");

/* UID */
if (prdf->prg.fg_uid)
   pAccData->dflt.uid = prdf->prd.fd_uid;
else 
   pAccData->dflt.uid = 0;

/* Expiration Date */
if (prdf->prg.fg_expdate)
   pAccData->dflt.expdate = prdf->prd.fd_expdate;
else 
   pAccData->dflt.expdate = 0;

/* Time of Day Restriction */
if (prdf->prg.fg_tod)
   strcpy(pAccData->dflt.tod,prdf->prd.fd_tod);
else 
   strcpy(pAccData->dflt.tod,"");

/* Maximum Failed Attempts */
if (prdf->prg.fg_max_tries)
   pAccData->dflt.maxTries = prdf->prd.fd_max_tries;
else 
   pAccData->dflt.maxTries = 0;

/* Nice Value */
if (prdf->prg.fg_nice)
   pAccData->dflt.nice = prdf->prd.fd_nice;
else 
   pAccData->dflt.nice = 0;

/* Account Locked */
if (prdf->prg.fg_lock)
   pAccData->dflt.locked = prdf->prd.fd_lock;
else 
   pAccData->dflt.locked = 0;

/* Password Parameters */

/* Minimum Password Change Time */
if (prdf->prg.fg_min)
   pAccData->dflt.fieldValue[ACC_CHG_TIME_INDEX] = prdf->prd.fd_min;
else 
   pAccData->dflt.fieldValue[ACC_CHG_TIME_INDEX] = 0;

/* Password Expiration Time */
if (prdf->prg.fg_expire)
   pAccData->dflt.fieldValue[ACC_EXP_TIME_INDEX] = prdf->prd.fd_expire;
else 
   pAccData->dflt.fieldValue[ACC_EXP_TIME_INDEX] = 0;

/* Password Lifetime */
if (prdf->prg.fg_lifetime)
   pAccData->dflt.fieldValue[ACC_LIFETIME_INDEX] = prdf->prd.fd_lifetime;
else 
   pAccData->dflt.fieldValue[ACC_LIFETIME_INDEX] = 0;

/* Maximum Password Length */
if (prdf->prg.fg_maxlen)
   pAccData->dflt.fieldValue[ACC_MAX_LENGTH_INDEX] = prdf->prd.fd_maxlen;
else 
   pAccData->dflt.fieldValue[ACC_MAX_LENGTH_INDEX] = 0;

/* Minimum Password Length */
if (prdf->prg.fg_minlen)
   pAccData->dflt.fieldValue[ACC_MIN_LENGTH_INDEX] = prdf->prd.fd_minlen;
else 
   pAccData->dflt.fieldValue[ACC_MIN_LENGTH_INDEX] = 0;

/* Password History Limit */
if (prdf->prg.fg_pwdepth)
   pAccData->dflt.fieldValue[ACC_HIST_LIMIT_INDEX] = prdf->prd.fd_pwdepth;
else 
   pAccData->dflt.fieldValue[ACC_HIST_LIMIT_INDEX] = 0;

/* Password Required */
if (prdf->prg.fg_nullpw)
   pAccData->dflt.fieldValue[ACC_REQUIRED_INDEX] = prdf->prd.fd_nullpw;
else 
   pAccData->dflt.fieldValue[ACC_REQUIRED_INDEX] = 0;

/* Users choose own */
if (prdf->prg.fg_pick_pwd)
   pAccData->dflt.fieldValue[ACC_USER_CHOOSE_INDEX]=prdf->prd.fd_pick_pwd;
else 
   pAccData->dflt.fieldValue[ACC_USER_CHOOSE_INDEX]=0;

/* System Generated Password */
if (prdf->prg.fg_gen_pwd)
   pAccData->dflt.fieldValue[ACC_GENERATED_INDEX] = prdf->prd.fd_gen_pwd;
else 
   pAccData->dflt.fieldValue[ACC_GENERATED_INDEX] = 0;

/* Random Character Password */
if (prdf->prg.fg_gen_chars)
   pAccData->dflt.fieldValue[ACC_RAND_CHARS_INDEX]=prdf->prd.fd_gen_chars;
else 
   pAccData->dflt.fieldValue[ACC_RAND_CHARS_INDEX]=0;

/* Random Letters Password */
if (prdf->prg.fg_gen_letters)
   pAccData->dflt.fieldValue[ACC_RAND_LETTERS_INDEX]=prdf->prd.fd_gen_letters;
else 
   pAccData->dflt.fieldValue[ACC_RAND_LETTERS_INDEX]=0;

/* Enforce Triviality Checks */
if (prdf->prg.fg_restrict)
   pAccData->dflt.fieldValue[ACC_TRIV_CHECKS_INDEX]=prdf->prd.fd_restrict;
else 
   pAccData->dflt.fieldValue[ACC_TRIV_CHECKS_INDEX]=0;

/* Enforce Site Triviality Checks */
if (prdf->prg.fg_policy)
   pAccData->dflt.fieldValue[ACC_SITE_TRIV_CHECKS_INDEX]=prdf->prd.fd_policy;
else 
   pAccData->dflt.fieldValue[ACC_SITE_TRIV_CHECKS_INDEX]=0;

#ifdef SEC_PRIV

/* Command Authoriations */
if (prdf->prg.fg_cprivs)
   for (i=0; i<AUTH_CPRIVVEC_SIZE; i++)
       pAccData->dflt.cprivs[i] = prdf->prd.fd_cprivs[i];
else 
   for (i=0; i<AUTH_CPRIVVEC_SIZE; i++)
       pAccData->dflt.cprivs[i] = 0L;

/* Base Privileges */
if (prdf->prg.fg_bprivs)
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.bprivs[i]=prdf->prd.fd_bprivs[i];
else 
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.bprivs[i]=0L;

/* System Privileges */
if (prdf->prg.fg_sprivs)
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.sprivs[i]=prdf->prd.fd_sprivs[i];
else 
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.sprivs[i]=0L;

#endif /* SEC_PRIV */

#ifdef SEC_MAC

/* Clearance */
if (prdf->prg.fg_clearance)
   mand_copy_ir(&(prdf->prd.fd_clearance),&(pAccData->dflt.clearance));
else
   {
   /* How to initialize the clearance */
   }

#endif /* SEC_MAC */

/* Audit Display */
if (prdf->prg.fg_auditdisp)
   strcpy(pAccData->dflt.auditdisp,prdf->prd.fd_auditdisp);
else 
   strcpy(pAccData->dflt.auditdisp,"");

/* Audit Control */
if (prdf->prg.fg_auditcntl)
   pAccData->dflt.auditcntl=prdf->prd.fd_auditcntl;
else 
   pAccData->dflt.auditcntl=0;
}
/******************************************************************************
 * AccStoreTemplateAsDefault()
 ******************************************************************************/
int AccStoreTemplateAsDefault(pAccData,tmpltname,w)
AccountDataType *pAccData;
char            *tmpltname;
Widget           w;

{
int                i;
struct pr_passwd  *prpwd;
struct prpw_if     defData;

if (XGetUserInfo(tmpltname,&defData,w) != SUCCESS)
   return(0);

prpwd = &defData.prpw;

/* Template name */
if (prpwd->uflg.fg_template)
   strcpy(pAccData->dflt.tmpltname,prpwd->ufld.fd_template);
else if (prpwd->sflg.fg_template)
   strcpy(pAccData->dflt.tmpltname,prpwd->sfld.fd_template);
else 
   strcpy(pAccData->dflt.tmpltname,"");

/* UID */
if (prpwd->uflg.fg_uid)
   pAccData->dflt.uid = prpwd->ufld.fd_uid;
else if (prpwd->sflg.fg_uid)
   pAccData->dflt.uid = prpwd->sfld.fd_uid;
else 
   pAccData->dflt.uid = 0;

/* Expiration Date */
if (prpwd->uflg.fg_expdate)
   pAccData->dflt.expdate = prpwd->ufld.fd_expdate;
else if (prpwd->sflg.fg_expdate)
   pAccData->dflt.expdate = prpwd->sfld.fd_expdate;
else 
   pAccData->dflt.expdate = 0;

/* Time of Day Restriction */
if (prpwd->uflg.fg_tod)
   strcpy(pAccData->dflt.tod,prpwd->ufld.fd_tod);
else if (prpwd->sflg.fg_tod)
   strcpy(pAccData->dflt.tod,prpwd->sfld.fd_tod);
else 
   strcpy(pAccData->dflt.tod,"");

/* Maximum Failed Attempts */
if (prpwd->uflg.fg_max_tries)
   pAccData->dflt.maxTries = prpwd->ufld.fd_max_tries;
else if (prpwd->sflg.fg_max_tries)
   pAccData->dflt.maxTries = prpwd->sfld.fd_max_tries;
else 
   pAccData->dflt.maxTries = 0;

/* Nice Value */
if (prpwd->uflg.fg_nice)
   pAccData->dflt.nice = prpwd->ufld.fd_nice;
else if (prpwd->sflg.fg_nice)
   pAccData->dflt.nice = prpwd->sfld.fd_nice;
else 
   pAccData->dflt.nice = 0;

/* Account Locked */
if (prpwd->uflg.fg_lock)
   pAccData->dflt.locked = prpwd->ufld.fd_lock;
else if (prpwd->sflg.fg_lock)
   pAccData->dflt.locked = prpwd->sfld.fd_lock;
else 
   pAccData->dflt.locked = 0;

/* Password Parameters */

/* Minimum Password Change Time */
if (prpwd->uflg.fg_min)
   pAccData->dflt.fieldValue[ACC_CHG_TIME_INDEX] = prpwd->ufld.fd_min;
else if (prpwd->sflg.fg_min)
   pAccData->dflt.fieldValue[ACC_CHG_TIME_INDEX] = prpwd->sfld.fd_min;
else 
   pAccData->dflt.fieldValue[ACC_CHG_TIME_INDEX] = 0;

/* Password Expiration Time */
if (prpwd->uflg.fg_expire)
   pAccData->dflt.fieldValue[ACC_EXP_TIME_INDEX] = prpwd->ufld.fd_expire;
else if (prpwd->sflg.fg_expire)
   pAccData->dflt.fieldValue[ACC_EXP_TIME_INDEX] = prpwd->sfld.fd_expire;
else 
   pAccData->dflt.fieldValue[ACC_EXP_TIME_INDEX] = 0;

/* Password Lifetime */
if (prpwd->uflg.fg_lifetime)
   pAccData->dflt.fieldValue[ACC_LIFETIME_INDEX] = prpwd->ufld.fd_lifetime;
else if (prpwd->sflg.fg_lifetime)
   pAccData->dflt.fieldValue[ACC_LIFETIME_INDEX] = prpwd->sfld.fd_lifetime;
else 
   pAccData->dflt.fieldValue[ACC_LIFETIME_INDEX] = 0;

/* Maximum Password Length */
if (prpwd->uflg.fg_maxlen)
   pAccData->dflt.fieldValue[ACC_MAX_LENGTH_INDEX] = prpwd->ufld.fd_maxlen;
else if (prpwd->sflg.fg_maxlen)
   pAccData->dflt.fieldValue[ACC_MAX_LENGTH_INDEX] = prpwd->sfld.fd_maxlen;
else 
   pAccData->dflt.fieldValue[ACC_MAX_LENGTH_INDEX] = 0;

/* Minimum Password Length */
if (prpwd->uflg.fg_minlen)
   pAccData->dflt.fieldValue[ACC_MIN_LENGTH_INDEX] = prpwd->ufld.fd_minlen;
else if (prpwd->sflg.fg_minlen)
   pAccData->dflt.fieldValue[ACC_MIN_LENGTH_INDEX] = prpwd->sfld.fd_minlen;
else 
   pAccData->dflt.fieldValue[ACC_MIN_LENGTH_INDEX] = 0;

/* Password History Limit */
if (prpwd->uflg.fg_pwdepth)
   pAccData->dflt.fieldValue[ACC_HIST_LIMIT_INDEX] = prpwd->ufld.fd_pwdepth;
else if (prpwd->sflg.fg_pwdepth)
   pAccData->dflt.fieldValue[ACC_HIST_LIMIT_INDEX] = prpwd->sfld.fd_pwdepth;
else 
   pAccData->dflt.fieldValue[ACC_HIST_LIMIT_INDEX] = 0;

/* Password Required */
if (prpwd->uflg.fg_nullpw)
   pAccData->dflt.fieldValue[ACC_REQUIRED_INDEX] = prpwd->ufld.fd_nullpw;
else if (prpwd->sflg.fg_nullpw)
   pAccData->dflt.fieldValue[ACC_REQUIRED_INDEX] = prpwd->sfld.fd_nullpw;
else 
   pAccData->dflt.fieldValue[ACC_REQUIRED_INDEX] = 0;

/* Users choose own */
if (prpwd->uflg.fg_pick_pwd)
   pAccData->dflt.fieldValue[ACC_USER_CHOOSE_INDEX]=prpwd->ufld.fd_pick_pwd;
else if (prpwd->sflg.fg_pick_pwd)
   pAccData->dflt.fieldValue[ACC_USER_CHOOSE_INDEX]=prpwd->sfld.fd_pick_pwd;
else 
   pAccData->dflt.fieldValue[ACC_USER_CHOOSE_INDEX]=0;

/* System Generated Password */
if (prpwd->uflg.fg_gen_pwd)
   pAccData->dflt.fieldValue[ACC_GENERATED_INDEX] = prpwd->ufld.fd_gen_pwd;
else if (prpwd->sflg.fg_gen_pwd)
   pAccData->dflt.fieldValue[ACC_GENERATED_INDEX] = prpwd->sfld.fd_gen_pwd;
else 
   pAccData->dflt.fieldValue[ACC_GENERATED_INDEX] = 0;

/* Random Character Password */
if (prpwd->uflg.fg_gen_chars)
   pAccData->dflt.fieldValue[ACC_RAND_CHARS_INDEX]=prpwd->ufld.fd_gen_chars;
else if (prpwd->sflg.fg_gen_chars)
   pAccData->dflt.fieldValue[ACC_RAND_CHARS_INDEX]=prpwd->sfld.fd_gen_chars;
else 
   pAccData->dflt.fieldValue[ACC_RAND_CHARS_INDEX]=0;

/* Random Letters Password */
if (prpwd->uflg.fg_gen_letters)
   pAccData->dflt.fieldValue[ACC_RAND_LETTERS_INDEX]=prpwd->ufld.fd_gen_letters;
else if (prpwd->sflg.fg_gen_letters)
   pAccData->dflt.fieldValue[ACC_RAND_LETTERS_INDEX]=prpwd->sfld.fd_gen_letters;
else 
   pAccData->dflt.fieldValue[ACC_RAND_LETTERS_INDEX]=0;

/* Enforce Triviality Checks */
if (prpwd->uflg.fg_restrict)
   pAccData->dflt.fieldValue[ACC_TRIV_CHECKS_INDEX]=prpwd->ufld.fd_restrict;
else if (prpwd->sflg.fg_restrict)
   pAccData->dflt.fieldValue[ACC_TRIV_CHECKS_INDEX]=prpwd->sfld.fd_restrict;
else 
   pAccData->dflt.fieldValue[ACC_TRIV_CHECKS_INDEX]=0;

/* Enforce Site Triviality Checks */
if (prpwd->uflg.fg_policy)
   pAccData->dflt.fieldValue[ACC_SITE_TRIV_CHECKS_INDEX]=prpwd->ufld.fd_policy;
else if (prpwd->sflg.fg_policy)
   pAccData->dflt.fieldValue[ACC_SITE_TRIV_CHECKS_INDEX]=prpwd->sfld.fd_policy;
else 
   pAccData->dflt.fieldValue[ACC_SITE_TRIV_CHECKS_INDEX]=0;

#ifdef SEC_PRIV

/* Command Auths */
if (prpwd->uflg.fg_cprivs)
   for (i=0; i<AUTH_CPRIVVEC_SIZE; i++)
       pAccData->dflt.cprivs[i] = prpwd->ufld.fd_cprivs[i];
else if (prpwd->sflg.fg_cprivs)
   for (i=0; i<AUTH_CPRIVVEC_SIZE; i++)
       pAccData->dflt.cprivs[i] = 0L;
   
/* Base Privileges */
if (prpwd->uflg.fg_bprivs)
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.bprivs[i]=prpwd->ufld.fd_bprivs[i];
else if (prpwd->sflg.fg_bprivs)
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.bprivs[i]=prpwd->sfld.fd_bprivs[i];
else
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.bprivs[i]=0L;

/* System Privileges */
if (prpwd->uflg.fg_sprivs)
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.sprivs[i]=prpwd->ufld.fd_sprivs[i];
else if (prpwd->sflg.fg_sprivs)
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.sprivs[i]=prpwd->sfld.fd_sprivs[i];
else
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       pAccData->dflt.sprivs[i]=0L;

#endif /* SEC_PRIV */

#ifdef SEC_MAC

/* Clearance */
if (prpwd->uflg.fg_clearance)
   mand_copy_ir(&(prpwd->ufld.fd_clearance), &(pAccData->dflt.clearance));
else if (prpwd->sflg.fg_clearance)
   mand_copy_ir(&(prpwd->sfld.fd_clearance), &(pAccData->dflt.clearance));
else
   {
   /* How to initialize clearance? */
   }

#endif /* SEC_MAC */

/* Audit Display */
if (prpwd->uflg.fg_auditdisp)
   strcpy(pAccData->dflt.auditdisp,prpwd->ufld.fd_auditdisp);
else if (prpwd->sflg.fg_auditdisp)
   strcpy(pAccData->dflt.auditdisp,prpwd->sfld.fd_auditdisp);
else 
   strcpy(pAccData->dflt.auditdisp,"");

/* Audit Control */
if (prpwd->uflg.fg_auditcntl)
   pAccData->dflt.auditcntl=prpwd->ufld.fd_auditcntl;
else if (prpwd->sflg.fg_gen_letters)
   pAccData->dflt.auditcntl=prpwd->sfld.fd_auditcntl;
else 
   pAccData->dflt.auditcntl=0;
}
/******************************************************************************
 * AccRetrieveAndStoreDefault()
 ******************************************************************************/
int AccRetrieveAndStoreDefault(pAccData,defname,w)
AccountDataType *pAccData;
char            *defname;
Widget           w;

{

if (strcmp(defname,SYSTEM_DEFAULT) == 0)
   AccStoreSystemDefault(pAccData,"default",w);
else
   AccStoreTemplateAsDefault(pAccData,defname,w);
}
/******************************************************************************
 * ModifySysDefaultForDBSave()           
 ******************************************************************************/
Boolean ModifySysDefaultForDBSave(pAccData)
AccountDataType *pAccData;

{
int                i;
int                ret;
int                value;
struct pr_passwd  *prpwd;
struct pr_default *prdf;

prdf = &pAccData->defaultData.df;
prpwd = &pAccData->userInfo->userData.prpw;

/* Expiration Date */
if (prdf->prd.fd_expdate != prpwd->ufld.fd_expdate)
   {
   prdf->prg.fg_expdate = 1;
   prdf->prd.fd_expdate = prpwd->ufld.fd_expdate;
   }

/* Time of Day Restriction */
if (strcmp(prdf->prd.fd_tod,prpwd->ufld.fd_tod) != 0)
   {
   prdf->prg.fg_tod = 1;
   strcpy(prdf->prd.fd_tod,prpwd->ufld.fd_tod);
   }

/* Password required */
if (prdf->prd.fd_nullpw != prpwd->ufld.fd_nullpw)
   {
   prdf->prg.fg_nullpw = 1;
   prdf->prd.fd_nullpw = prpwd->ufld.fd_nullpw;
   }

/* User chooses password */
if (prdf->prd.fd_pick_pwd != prpwd->ufld.fd_pick_pwd)
   {
   prdf->prg.fg_pick_pwd = 1;
   prdf->prd.fd_pick_pwd = prpwd->ufld.fd_pick_pwd;
   }

/* System Generated Password */
if (prdf->prd.fd_gen_pwd != prpwd->ufld.fd_gen_pwd)
   {
   prdf->prg.fg_gen_pwd = 1;
   prdf->prd.fd_gen_pwd = prpwd->ufld.fd_gen_pwd;
   }

/* Random Characters */
if (prdf->prd.fd_gen_chars != prpwd->ufld.fd_gen_chars)
   {
   prdf->prg.fg_gen_chars = 1;
   prdf->prd.fd_gen_chars = prpwd->ufld.fd_gen_chars;
   }

/* Random Letters */
if (prdf->prd.fd_gen_letters != prpwd->ufld.fd_gen_letters)
   {
   prdf->prg.fg_gen_letters = 1;
   prdf->prd.fd_gen_letters = prpwd->ufld.fd_gen_letters;
   }

/* Triviality Checks */
if (prdf->prd.fd_restrict != prpwd->ufld.fd_restrict)
   {
   prdf->prg.fg_restrict = 1;
   prdf->prd.fd_restrict = prpwd->ufld.fd_restrict;
   }

/* Minimum Change Time */
if (prdf->prd.fd_min != prpwd->ufld.fd_min)
   {
   prdf->prg.fg_min = 1;
   prdf->prd.fd_min = prpwd->ufld.fd_min;
   }

/* Expiration Time */
if (prdf->prd.fd_expire != prpwd->ufld.fd_expire)
   {
   prdf->prg.fg_expire = 1;
   prdf->prd.fd_expire = prpwd->ufld.fd_expire;
   }

/* Lifetime */
if (prdf->prd.fd_lifetime != prpwd->ufld.fd_lifetime)
   {
   prdf->prg.fg_lifetime = 1;
   prdf->prd.fd_lifetime = prpwd->ufld.fd_lifetime;
   }

/* Maximum Length */
if (prdf->prd.fd_maxlen != prpwd->ufld.fd_maxlen)
   {
   prdf->prg.fg_maxlen = 1;
   prdf->prd.fd_maxlen = prpwd->ufld.fd_maxlen;
   }

/* Maximum allowed Login Attempts */
if (prdf->prd.fd_max_tries != prpwd->ufld.fd_max_tries)
   {
   prdf->prg.fg_max_tries = 1;
   prdf->prd.fd_max_tries = prpwd->ufld.fd_max_tries;
   }

/* Nice Value */
if (prdf->prd.fd_nice != prpwd->ufld.fd_nice)
   {
   prdf->prg.fg_nice = 1;
   prdf->prd.fd_nice = prpwd->ufld.fd_nice;
   }

/* Account Locked */
if (prdf->prd.fd_lock != prpwd->ufld.fd_lock)
   {
   prdf->prg.fg_lock = 1;
   prdf->prd.fd_lock = prpwd->ufld.fd_lock;
   }

/* Audit Display */
if (strcmp(prdf->prd.fd_auditdisp,prpwd->ufld.fd_auditdisp) != 0)
   {
   prdf->prg.fg_auditdisp = 1;
   strcpy(prdf->prd.fd_auditdisp,prpwd->ufld.fd_auditdisp);
   }

/* Audit Control */
if (prdf->prd.fd_auditcntl != prpwd->ufld.fd_auditcntl)
   {
   prdf->prg.fg_auditcntl = 1;
   prdf->prd.fd_auditcntl = prpwd->ufld.fd_auditcntl;
   }

#ifdef SEC_PRIV

if (prdf->prd.fd_cprivs != prpwd->ufld.fd_cprivs)
   {
   prdf->prg.fg_cprivs = 1;
   for (i=0; i<AUTH_CPRIVVEC_SIZE; i++)
       prdf->prd.fd_cprivs[i] = prpwd->ufld.fd_cprivs[i];
   }

if (prdf->prd.fd_bprivs != prpwd->ufld.fd_bprivs)
   {
   prdf->prg.fg_bprivs = 1;
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       prdf->prd.fd_bprivs[i] = prpwd->ufld.fd_bprivs[i];
   }

if (prdf->prd.fd_sprivs != prpwd->ufld.fd_sprivs)
   {
   prdf->prg.fg_sprivs = 1;
   for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
       prdf->prd.fd_sprivs[i] = prpwd->ufld.fd_sprivs[i];
   }

#endif /* SEC_PRIV */

#ifdef SEC_MAC

/* Clearance */
ret=mand_ir_relationship(&(prpwd->ufld.fd_clearance),&(prdf->prd.fd_clearance));
if (ret && !(ret & MAND_EQUAL))
   {
   prdf->prg.fg_clearance = 1;
   mand_copy_ir(&(prpwd->ufld.fd_clearance), &(prdf->prd.fd_clearance));
   }

#endif /* SEC_MAC */

return(True);
}
/******************************************************************************
 * AccAssignSmartFlagsToUnlockAccount()
 ******************************************************************************/
void AccAssignSmartFlagsToUnlockAccount(prpwd)
struct pr_passwd *prpwd;

{
time_t  life;
int     logins_allowed;
time_t  now;

if (prpwd->uflg.fg_schange) 
   {
   if (prpwd->uflg.fg_lifetime)
      life = prpwd->ufld.fd_lifetime;
   else if (prpwd->sflg.fg_lifetime)
      life = prpwd->sfld.fd_lifetime;
   else
      life = (time_t) 0;
   now = time ((long *) 0);
   if (life > 0 && now - prpwd->ufld.fd_schange > life) 
      {
      /* set successful change time back to start
       * of expired period to force a change */
      if (prpwd->uflg.fg_expire)
         now -= prpwd->ufld.fd_expire;
      else if (prpwd->sflg.fg_expire)
         now -= prpwd->sfld.fd_expire;
      prpwd->ufld.fd_schange = now;
      prpwd->uflg.fg_schange = 1;
      }
   }

/* check whether number of unsuccessful logins has been
 * exceeded.  Don't reset the validity of the password.
 */
if (prpwd->uflg.fg_max_tries)
   logins_allowed = prpwd->ufld.fd_max_tries;
else if (prpwd->sflg.fg_max_tries)
   logins_allowed = prpwd->sfld.fd_max_tries;
else
   logins_allowed = 0;
if (prpwd->uflg.fg_nlogins)
   if (logins_allowed > 0 && prpwd->ufld.fd_nlogins > logins_allowed)
      prpwd->uflg.fg_nlogins = 1;
      prpwd->ufld.fd_nlogins = 0;
}
/******************************************************************************
 * AccAssignSmartFlagsForDBSave()           
 ******************************************************************************/
Boolean AccAssignSmartFlagsForDBSave(pAccData,type)
AccountDataType *pAccData;
int              type;

{
int               i;
int               ret;
int               value;
struct pr_passwd *prpwd;

prpwd = &pAccData->userInfo->userData.prpw;

/* Set fg_name == 1 OR protected DB entry will be deleted */
prpwd->uflg.fg_name = 1;

if (type == ACC_USERS)
   {
   /* Save the template name if there is one... */
   if (strcmp(prpwd->ufld.fd_template,"") != 0)
      prpwd->uflg.fg_template = 1;
   }
else if (type == ACC_TEMPLATES)
   {
   /* This IS a template */
   prpwd->ufld.fd_istemplate = 1;
   prpwd->uflg.fg_istemplate = 1;
   }

/* Time of Day Restriction */
if (strcmp(prpwd->ufld.fd_tod,pAccData->dflt.tod) != 0)
   prpwd->uflg.fg_tod = 1;

/* Expiration Date */
if (prpwd->ufld.fd_expdate != pAccData->dflt.expdate)
   prpwd->uflg.fg_expdate = 1;

/* Password required */
if (prpwd->ufld.fd_nullpw != pAccData->dflt.fieldValue[ACC_REQUIRED_INDEX])
   prpwd->uflg.fg_nullpw = 1;

/* User chooses password */
if (prpwd->ufld.fd_pick_pwd != pAccData->dflt.fieldValue[ACC_USER_CHOOSE_INDEX])
   prpwd->uflg.fg_pick_pwd = 1;

/* System Generated Password */
if (prpwd->ufld.fd_gen_pwd != pAccData->dflt.fieldValue[ACC_GENERATED_INDEX])
   prpwd->uflg.fg_gen_pwd = 1;

/* Random Characters */
if (prpwd->ufld.fd_gen_chars != pAccData->dflt.fieldValue[ACC_RAND_CHARS_INDEX])
   prpwd->uflg.fg_gen_chars = 1;

/* Random Letters */
if (prpwd->ufld.fd_gen_letters != 
    pAccData->dflt.fieldValue[ACC_RAND_LETTERS_INDEX])
   prpwd->uflg.fg_gen_letters = 1;

/* Triviality Checks */
if (prpwd->ufld.fd_restrict != pAccData->dflt.fieldValue[ACC_TRIV_CHECKS_INDEX])
   prpwd->uflg.fg_restrict = 1;

/* Site Triviality Checks */
if (prpwd->ufld.fd_policy != 
    pAccData->dflt.fieldValue[ACC_SITE_TRIV_CHECKS_INDEX])
   prpwd->uflg.fg_policy = 1;

/* Minimum Change Time */
if (prpwd->ufld.fd_min != pAccData->dflt.fieldValue[ACC_CHG_TIME_INDEX])
   prpwd->uflg.fg_min = 1;

/* Expiration Time */
if (prpwd->ufld.fd_expire != pAccData->dflt.fieldValue[ACC_EXP_TIME_INDEX])
   prpwd->uflg.fg_expire = 1;

/* Lifetime */
if (prpwd->ufld.fd_lifetime != pAccData->dflt.fieldValue[ACC_LIFETIME_INDEX])
   prpwd->uflg.fg_lifetime = 1;

/* Maximum Length */
if (prpwd->ufld.fd_maxlen != pAccData->dflt.fieldValue[ACC_MAX_LENGTH_INDEX])
   prpwd->uflg.fg_maxlen = 1;

/* Minimum Length */
if (prpwd->ufld.fd_minlen != pAccData->dflt.fieldValue[ACC_MIN_LENGTH_INDEX])
   prpwd->uflg.fg_minlen = 1;

/* History Limit */
if (prpwd->ufld.fd_pwdepth != pAccData->dflt.fieldValue[ACC_HIST_LIMIT_INDEX])
   prpwd->uflg.fg_pwdepth = 1;

/* Maximum allowed Login Attempts */
if (prpwd->ufld.fd_max_tries != pAccData->dflt.maxTries)
   prpwd->uflg.fg_max_tries = 1;

/* Nice Value */
if (prpwd->ufld.fd_nice != pAccData->dflt.nice)
   prpwd->uflg.fg_nice = 1;

/* Account Locked */
if (prpwd->ufld.fd_lock != pAccData->dflt.locked)
   {
   prpwd->uflg.fg_lock = 1;

   /* If the USER account is being unlocked...*/
   if (type == ACC_USERS)
      AccAssignSmartFlagsToUnlockAccount(prpwd);

   }

/* Audit Control */
if (prpwd->ufld.fd_auditcntl != pAccData->dflt.auditcntl)
   prpwd->uflg.fg_auditcntl = 1;

/* Audit Display */
if (strcmp(prpwd->ufld.fd_auditdisp,pAccData->dflt.auditdisp) != 0)
   prpwd->uflg.fg_auditdisp = 1;

#ifdef SEC_PRIV

for (i=0; i<AUTH_CPRIVVEC_SIZE; i++)
    if (prpwd->ufld.fd_cprivs[i] != pAccData->dflt.cprivs[i])
       prpwd->uflg.fg_cprivs = 1;

for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
    if (prpwd->ufld.fd_bprivs[i] != pAccData->dflt.bprivs[i])
       prpwd->uflg.fg_bprivs = 1;

for (i=0;i<SEC_SPRIVVEC_SIZE;i++)
    if (prpwd->ufld.fd_sprivs[i] != pAccData->dflt.sprivs[i])
       prpwd->uflg.fg_sprivs = 1;

#endif /* SEC_PRIV */

#ifdef SEC_MAC

/* Clearance */
ret=mand_ir_relationship(&(prpwd->ufld.fd_clearance),
                         &(pAccData->dflt.clearance));
if (ret && !(ret & MAND_EQUAL))
   prpwd->uflg.fg_clearance = 1;

#endif /# SEC_MAC */

return(True);
}
/******************************************************************************
 * AccLoadPasswordDataFromScreen()           
 ******************************************************************************/
static Boolean AccLoadPasswordDataFromScreen(pAccData,type)
AccountDataType *pAccData;
int              type;

{
int               value;
struct pr_passwd *prpwd;

prpwd = &pAccData->userInfo->userData.prpw;

/* Template name */
strcpy(prpwd->ufld.fd_template,XmTextFieldGetString(pAccData->tmpltnameText));
if (removeBlanks(prpwd->ufld.fd_template))
   XmTextSetString(pAccData->tmpltnameText,prpwd->ufld.fd_template);

/* 
 * Covert the system default template to blanks as the library interface
 * to prpwd doesn't understand it and this is purely a presentation issue.
 */
if (strcmp(SYSTEM_DEFAULT_TEMPLATE,prpwd->ufld.fd_template) == 0)
   {
   strcpy(prpwd->ufld.fd_template,"");
   }

if (type == ACC_USERS)
   {
   /* User name */
   strcpy(prpwd->ufld.fd_name,XmTextFieldGetString(pAccData->usernameText));
   if (removeBlanks(prpwd->ufld.fd_name))
      XmTextSetString(pAccData->usernameText,prpwd->ufld.fd_name);
   }
else
   {
   /* Copy the template name to the name field */
   strcpy(prpwd->ufld.fd_name,prpwd->ufld.fd_template);
   }

/* Password required */
/* Note that this is really a negative button (backwards) */
ASSIGN_TB_STATE(pAccData->fields[ACC_REQUIRED_INDEX], prpwd->ufld.fd_nullpw);

/* The field is displayed backwards and must be read in consistently */
if (prpwd->ufld.fd_nullpw)
   prpwd->ufld.fd_nullpw = 0;
else prpwd->ufld.fd_nullpw = 1;

/* User chooses password */
ASSIGN_TB_STATE(pAccData->fields[ACC_USER_CHOOSE_INDEX],
                prpwd->ufld.fd_pick_pwd);

/* System Generated Password */
ASSIGN_TB_STATE(pAccData->fields[ACC_GENERATED_INDEX],prpwd->ufld.fd_gen_pwd);

/* Random Characters */
ASSIGN_TB_STATE(pAccData->fields[ACC_RAND_CHARS_INDEX],
                prpwd->ufld.fd_gen_chars);

/* Random Letters */
ASSIGN_TB_STATE(pAccData->fields[ACC_RAND_LETTERS_INDEX],
                prpwd->ufld.fd_gen_letters);

/* Triviality Checks */
ASSIGN_TB_STATE(pAccData->fields[ACC_TRIV_CHECKS_INDEX],
                prpwd->ufld.fd_restrict);

ASSIGN_TB_STATE(pAccData->fields[ACC_SITE_TRIV_CHECKS_INDEX],
                prpwd->ufld.fd_policy);

/* Minimum Change Time */
XmScaleGetValue(pAccData->fields[ACC_CHG_TIME_INDEX],&value);
pAccData->fieldValue[ACC_CHG_TIME_INDEX] = (time_t)(value * SECINWEEK);
prpwd->ufld.fd_min = pAccData->fieldValue[ACC_CHG_TIME_INDEX];

/* Expiration Time */
XmScaleGetValue(pAccData->fields[ACC_EXP_TIME_INDEX],&value);
pAccData->fieldValue[ACC_EXP_TIME_INDEX] = (time_t)(value * SECINWEEK);
prpwd->ufld.fd_expire = pAccData->fieldValue[ACC_EXP_TIME_INDEX];

/* Lifetime */
XmScaleGetValue(pAccData->fields[ACC_LIFETIME_INDEX],&value);
pAccData->fieldValue[ACC_LIFETIME_INDEX] = (time_t)(value * SECINWEEK);
prpwd->ufld.fd_lifetime = pAccData->fieldValue[ACC_LIFETIME_INDEX];

/* Maximum Length */
XmScaleGetValue(pAccData->fields[ACC_MAX_LENGTH_INDEX],&value);
pAccData->fieldValue[ACC_MAX_LENGTH_INDEX] = value;
prpwd->ufld.fd_maxlen = pAccData->fieldValue[ACC_MAX_LENGTH_INDEX];

/* Minimum Length */
XmScaleGetValue(pAccData->fields[ACC_MIN_LENGTH_INDEX],&value);
pAccData->fieldValue[ACC_MIN_LENGTH_INDEX] = value;
prpwd->ufld.fd_minlen = pAccData->fieldValue[ACC_MIN_LENGTH_INDEX];

/* History Limit */
XmScaleGetValue(pAccData->fields[ACC_HIST_LIMIT_INDEX],&value);
pAccData->fieldValue[ACC_HIST_LIMIT_INDEX] = value;
prpwd->ufld.fd_pwdepth = pAccData->fieldValue[ACC_HIST_LIMIT_INDEX];

/* Must check for values set to 0 */
if (((pAccData->fieldValue[ACC_LIFETIME_INDEX] <
      pAccData->fieldValue[ACC_EXP_TIME_INDEX]) &&
      (pAccData->fieldValue[ACC_LIFETIME_INDEX] != (time_t) 0)) ||
     ((pAccData->fieldValue[ACC_EXP_TIME_INDEX] <
       pAccData->fieldValue[ACC_CHG_TIME_INDEX]) &&
       (pAccData->fieldValue[ACC_EXP_TIME_INDEX] != (time_t) 0)))
   {
   if (type == ACC_USERS)
      PostErrorDialog("msg_acc_error_usr_passwd",accounts_mw,NULL,NULL,NULL,
                      NULL,NULL,NULL);
   else 
      PostErrorDialog("msg_acc_error_usr_passwd",acctmplt_mw,NULL,NULL,NULL,
                      NULL,NULL,NULL);
   return(False);
   }

return(True);
}
/******************************************************************************
 * AccLoadLoginDataFromScreen()
 ******************************************************************************/
Boolean AccLoadLoginDataFromScreen(pAccData,type)
AccountDataType *pAccData;
int              type;

{
int               value;
struct pr_passwd *prpwd;
time_t            time;
char             *time_str;

prpwd = &pAccData->userInfo->userData.prpw;

/* Maximum allowed Login Attempts */
XmScaleGetValue(pAccData->maxTries,&value);
prpwd->ufld.fd_max_tries = (ushort) value;

/* Nice Value */
XmScaleGetValue(pAccData->nice,&value);
prpwd->ufld.fd_nice = value;

/* Account Locked */
ASSIGN_TB_STATE(pAccData->locked,prpwd->ufld.fd_lock);

/* Time of Day Restriction */
strcpy(prpwd->ufld.fd_tod,XmTextFieldGetString(pAccData->tod_tx));
if (removeBlanks(prpwd->ufld.fd_tod))
   XmTextSetString(pAccData->tod_tx,prpwd->ufld.fd_tod);
if (! IsValidUUCPTimeFormat(prpwd->ufld.fd_tod,type))          
   return(False);

/* Expiration Data */
time_str = XmTextFieldGetString(pAccData->expdate_tx);
if (removeBlanks(time_str))
   XmTextSetString(pAccData->expdate,time_str);

/* If the field is not set assume 0 */
if (strcmp(time_str,"") == 0)
   prpwd->ufld.fd_expdate = 0;
else
   {
   if (StringToTime(time_str,&time,type))
      prpwd->ufld.fd_expdate = time;
   else 
      {
      XtFree(time_str);
      return(False);
      }
   }

XtFree(time_str);

return(True);
}
/******************************************************************************
 * AccPrintPrpwdRecord()
 ******************************************************************************/
static void AccAccPrintPrpwdRecord(pAccData)
AccountDataType *pAccData;

{
struct pr_passwd *prpwd = &pAccData->userInfo->userData.prpw;

printf("fg_min uflg %d ufld %d sflg %d sfld %d\n",
       prpwd->uflg.fg_min,TimeToWeeks(prpwd->ufld.fd_min),
       prpwd->sflg.fg_min,TimeToWeeks(prpwd->sfld.fd_min));
printf("fg_expire uflg %d ufld %d sflg %d sfld %d\n",
       prpwd->uflg.fg_expire,TimeToWeeks(prpwd->ufld.fd_expire),
       prpwd->sflg.fg_expire,TimeToWeeks(prpwd->sfld.fd_expire));
printf("fg_lifetime uflg %d ufld %d sflg %d sfld %d\n",
       prpwd->uflg.fg_lifetime,TimeToWeeks(prpwd->ufld.fd_lifetime),
       prpwd->sflg.fg_lifetime,TimeToWeeks(prpwd->sfld.fd_lifetime));
printf("fg_maxlen uflg %d ufld %d sflg %d sfld %d\n",
       prpwd->uflg.fg_maxlen,prpwd->ufld.fd_maxlen,
       prpwd->sflg.fg_maxlen,prpwd->sfld.fd_maxlen);
printf("fg_minlen uflg %d ufld %d sflg %d sfld %d\n",
       prpwd->uflg.fg_minlen,prpwd->ufld.fd_minlen,
       prpwd->sflg.fg_minlen,prpwd->sfld.fd_minlen);
printf("fg_pwdepth uflg %d ufld %d sflg %d sfld %d\n",
       prpwd->uflg.fg_pwdepth,prpwd->ufld.fd_pwdepth,
       prpwd->sflg.fg_pwdepth,prpwd->sfld.fd_pwdepth);
printf("fg_nullpw uflg %d ufld %d sflg %d sfld %d\n",
       prpwd->uflg.fg_nullpw,prpwd->ufld.fd_nullpw,
       prpwd->sflg.fg_nullpw,prpwd->sfld.fd_nullpw);
}
/******************************************************************************
 * AccPrintUserData();
 ******************************************************************************/
void AccPrintUserData(pAccData,pwd,grp,secGroup_names,secGroup_count)
AccountDataType *pAccData;
struct   passwd pwd;
struct   group  grp;
char   **secGroup_names;
int      secGroup_count;

{
struct pr_passwd *prpwd;
int               i;

prpwd = &pAccData->userInfo->userData.prpw;

printf("\nAccount Information to be saved for *%s*\n",pwd.pw_name);
printf("UID           %d\n",pwd.pw_uid);
printf("Home Dir     *%s*\n",pwd.pw_dir);
printf("Shell        *%s*\n",pwd.pw_shell);
printf("Comments     *%s*\n",pwd.pw_gecos);
printf("Pri Group ID  %d\n",pwd.pw_gid);
printf("\nSecondary Groups:\n");
for (i = 0; i< secGroup_count; i++)
    printf("   *%s*\n",secGroup_names[i]);
printf("\nMax Logins    %d\n",prpwd->ufld.fd_max_tries);
printf("Nice Value    %d\n",prpwd->ufld.fd_nice);
printf("Account Lock  %d\n",prpwd->ufld.fd_lock);
printf("Min Change    %d\n",prpwd->ufld.fd_min);
printf("Expiration    %d\n",prpwd->ufld.fd_expire);
printf("Lifetime      %d\n",prpwd->ufld.fd_lifetime);
printf("Max Length    %d\n",prpwd->ufld.fd_maxlen);
printf("Password Req  %d\n",prpwd->ufld.fd_nullpw);
printf("User Chooses  %d\n",prpwd->ufld.fd_pick_pwd);
printf("System Gen    %d\n",prpwd->ufld.fd_gen_pwd);
printf("Random Chars  %d\n",prpwd->ufld.fd_gen_chars);
printf("Random Letter %d\n",prpwd->ufld.fd_gen_letters);
printf("Triv Checks   %d\n",prpwd->ufld.fd_restrict);

#ifdef SEC_PRIV
printf("\nCommand Auths:\n");
for (i = 0; i < pAccData->userInfo->ncommand_auths; i++)
    printf("   *%s*\n",pAccData->userInfo->command_auths[i]);
printf("\nKernel Auths:\n");
for (i = 0; i < pAccData->nprivs; i++)
    if (ISBITSET(prpwd->ufld.fd_sprivs, sys_priv[i].value))
       printf("   *%s*\n",sys_priv[i].name);
printf("\nBase Privs:\n");
for (i = 0; i < pAccData->nprivs; i++)
    if (ISBITSET(prpwd->ufld.fd_bprivs, sys_priv[i].value))
       printf("   *%s*\n",sys_priv[i].name);
#endif /* SEC_PRIV */
}
/******************************************************************************
 * AccSetAllUserFieldFlags()           
 ******************************************************************************/
void AccSetAllUserFieldFlags(pAccData,value)
AccountDataType *pAccData;
int      value;

{
int               i;
struct pr_passwd *prpwd;

prpwd = &pAccData->userInfo->userData.prpw;

/* Indicate that values are stored as user fields and not system defaults */

prpwd->uflg.fg_name        = value;
prpwd->uflg.fg_tod         = value;
prpwd->uflg.fg_expdate     = value;
prpwd->uflg.fg_uid         = value;
prpwd->uflg.fg_nice        = value;
#ifdef SEC_PRIV
prpwd->uflg.fg_cprivs      = value;
prpwd->uflg.fg_sprivs      = value;
prpwd->uflg.fg_bprivs      = value;
#endif /* SEC_PRIV */
#ifdef SEC_MAC
prpwd->uflg.fg_clearance   = value;
#endif /* SEC_MAC */
prpwd->uflg.fg_min         = value;
prpwd->uflg.fg_minlen      = value;
prpwd->uflg.fg_maxlen      = value;
prpwd->uflg.fg_expire      = value;
prpwd->uflg.fg_lifetime    = value;
prpwd->uflg.fg_pick_pwd    = value;
prpwd->uflg.fg_gen_pwd     = value;
prpwd->uflg.fg_restrict    = value; 
prpwd->uflg.fg_policy      = value; 
prpwd->uflg.fg_nullpw      = value;
prpwd->uflg.fg_pwdepth     = value;
prpwd->uflg.fg_pwdict      = value;
prpwd->uflg.fg_gen_chars   = value;
prpwd->uflg.fg_gen_letters = value;
prpwd->uflg.fg_max_tries   = value;
prpwd->uflg.fg_lock        = value;
prpwd->uflg.fg_istemplate  = value;
prpwd->uflg.fg_template    = value;
prpwd->uflg.fg_auditdisp   = value;
prpwd->uflg.fg_auditcntl   = value;
}
/******************************************************************************
 * AccDisplayCmdAuthData()
 ******************************************************************************/
static void AccDisplayCmdAuthData(pAccData)
AccountDataType *pAccData;

{
int i;
XmString xmstring;
struct pr_passwd *prpwd = &pAccData->userInfo->userData.prpw;

if (prpwd->uflg.fg_cprivs)
   {
   for (i = 0; i < pAccData->nauths; i++)
       {
       if (ISBITSET(prpwd->ufld.fd_cprivs, cmd_priv[i].value))
          {
          xmstring = XmStringCreate(cmd_priv[i].name, charset);
          XmSelectListAddItem(pAccData->authList,xmstring,0);
          XmAvailableListDeleteItem(pAccData->authList,xmstring,0);
          XmStringFree(xmstring);
          }
       }
   }

else if (prpwd->sflg.fg_cprivs)
   {
   for (i = 0; i < pAccData->nauths; i++)
       {
       if (ISBITSET(prpwd->sfld.fd_cprivs, cmd_priv[i].value))
          {
          xmstring = XmStringCreate(cmd_priv[i].name, charset);
          XmSelectListAddItem(pAccData->authList,xmstring,0);
          XmAvailableListDeleteItem(pAccData->authList,xmstring,0);
          XmStringFree(xmstring);
          }
       }
   }
}
/******************************************************************************
 * AccDisplayPrivData()
 ******************************************************************************/
static void AccDisplayPrivData(pAccData)
AccountDataType *pAccData;

{
int   i;
XmString xmstring;
struct pr_passwd *prpwd = &pAccData->userInfo->userData.prpw;

if (prpwd->uflg.fg_bprivs)
   {
   for (i = 0; i < pAccData->nprivs; i++)
       {
       if (ISBITSET(prpwd->ufld.fd_bprivs, sys_priv[i].value))
          {
          xmstring = XmStringCreate(sys_priv[i].name, charset);
          XmAvailableListDeleteItem(pAccData->baseList, xmstring,0);
          XmSelectListAddItem(pAccData->baseList,xmstring,0);
          XmStringFree(xmstring);
          }
       }
   }

else if (prpwd->sflg.fg_bprivs)
   {
   for (i = 0; i < pAccData->nprivs; i++)
       {
       if (ISBITSET(prpwd->sfld.fd_bprivs, sys_priv[i].value))
          {
          xmstring = XmStringCreate(sys_priv[i].name, charset);
          XmAvailableListDeleteItem(pAccData->baseList,xmstring,0);
          XmSelectListAddItem(pAccData->baseList,xmstring,0);
          XmStringFree(xmstring);
          }
       }
   }

if (prpwd->uflg.fg_sprivs)
   {
   for (i = 0; i < pAccData->nprivs; i++)
       {
       if (ISBITSET(prpwd->ufld.fd_sprivs, sys_priv[i].value))
          {
          xmstring = XmStringCreate(sys_priv[i].name, charset);
          XmAvailableListDeleteItem(pAccData->kernelList,xmstring,0);
          XmSelectListAddItem(pAccData->kernelList,xmstring,0);
          XmStringFree(xmstring);
          }
       }
    }

else if (prpwd->sflg.fg_sprivs)
   {
   for (i = 0; i < pAccData->nprivs; i++)
       {
       if (ISBITSET(prpwd->sfld.fd_sprivs, sys_priv[i].value))
          {
          xmstring = XmStringCreate(sys_priv[i].name, charset);
          XmAvailableListDeleteItem(pAccData->kernelList,xmstring,0);
          XmSelectListAddItem(pAccData->kernelList,xmstring,0);
          XmStringFree(xmstring);
          }
       }
    }
}

#ifdef SEC_MAC

/******************************************************************************
* AccDisplayClearance()
******************************************************************************/
static void AccDisplayClearance(pAccData)
AccountDataType *pAccData;

{
struct pr_passwd *prpwd = &pAccData->userInfo->userData.prpw;

if (pAccData->clearance_tx)
   {
   if (prpwd->uflg.fg_clearance)
   XmTextSetString(pAccData->clearance_tx,
                   clearance_ir_to_er(&prpwd->ufld.fd_clearance));
   else
      if (prpwd->sflg.fg_clearance)
          XmTextSetString(pAccData->clearance_tx,
                          clearance_ir_to_er(&prpwd->sfld.fd_clearance));
   else
      XmTextSetString(pAccData->clearance_tx,"");
   }
}

#endif /* SEC_MAC */

/******************************************************************************
 * AccClearLoginGraphicObjects()
 ******************************************************************************/
static void AccClearLoginGraphicObjects(pAccData)
AccountDataType *pAccData;

{
Arg args[10];
int minimum;

if (pAccData->maxTries)
   {
   SET_SCALE_TO_MINIMUM(pAccData->maxTries,minimum,args);
   }
if (pAccData->nice)
   {
   SET_SCALE_TO_MINIMUM(pAccData->nice,minimum,args);
   }
if (pAccData->locked)
   {
   XmToggleButtonSetState(pAccData->locked, False);
   }
}
/******************************************************************************
 * AccDisplayLoginData()
 ******************************************************************************/
static void AccDisplayLoginData(pAccData)
AccountDataType *pAccData;

{
int value;
time_t tvalue;
char time_str[26];
int defaultFlag = False;
struct tm *tm;
struct pr_passwd *prpwd = &pAccData->userInfo->userData.prpw;

if (prpwd->uflg.fg_max_tries)
   {
   value = (int) prpwd->ufld.fd_max_tries;
   }
else 
   {
   defaultFlag |= ACC_MAX_TRIES_FLAG;
   if (prpwd->sflg.fg_max_tries)
      value = (int) prpwd->sfld.fd_max_tries;
   else
      value = 0;
   }
XmScaleSetValue(pAccData->maxTries,value);

if (prpwd->uflg.fg_nice)
   {
   value = prpwd->ufld.fd_nice;
   }
else
   {
   defaultFlag |= ACC_NICE_FLAG;
   if (prpwd->sflg.fg_nice)
      value = (int) prpwd->sfld.fd_nice;
   else
      value = 0;
   }
XmScaleSetValue(pAccData->nice,value);

if (pAccData->locked)
   {
   /*
    * Access this libsecurity function to see if login will be denied
    * because of lock flag. too many unsuccessful attempts etc.
    */
   if (locked_out(prpwd))
      {
      XmToggleButtonSetState(pAccData->locked, True);
      }
   }

/* Time of Day Restriction */
if (pAccData->tod_tx)
   {
   if (prpwd->uflg.fg_tod)
      XmTextSetString(pAccData->tod_tx,prpwd->ufld.fd_tod);
   else 
      {
      if (prpwd->sflg.fg_tod)
         XmTextSetString(pAccData->tod_tx,prpwd->sfld.fd_tod);
      else 
         XmTextSetString(pAccData->tod_tx,"");
      }
   }

/* Expiration Date */
if (prpwd->uflg.fg_expdate)
   tvalue = prpwd->ufld.fd_expdate;
else 
   {
   if (prpwd->sflg.fg_expdate)
      tvalue = prpwd->sfld.fd_expdate;
   else 
      tvalue = 0;
   }

/* Only Display the date if it is nonzero; else no expiration date  */
if (pAccData->expdate_tx)
   {
   if (tvalue != 0)
      {
      tm = localtime(&tvalue);
      sprintf(time_str,"%s %d %.2d:%.2d %4d",
              months[tm->tm_mon],tm->tm_mday,
              tm->tm_hour,tm->tm_min,tm->tm_year+1900);
      XmTextSetString(pAccData->expdate_tx,time_str);
      }
   else
      {
      XmTextSetString(pAccData->expdate_tx,"");
      }
   }

/* If account has expired, notify by changing exdate label color and font */
if (tvalue && tvalue <= time((time_t *)0)) 
   {
   /* In Progress */
   }

/* 
 * Flush all those nasty events before we set ourselves up to do updates 
 * again.  This will hopefully keep our defaults list up to date.
 */
XSync(XtDisplay(pAccData->nice), False);
pAccData->ignoreUpdates = False;
}
/******************************************************************************
 * AccClearPasswordGraphicObjects()
 ******************************************************************************/
static void AccClearPasswordGraphicObjects(pAccData)
AccountDataType *pAccData;

{
Arg args[10];
int minimum;

if (pAccData->fields[ACC_CHG_TIME_INDEX] != NULL)
   {
   SET_SCALE_TO_MINIMUM(pAccData->fields[ACC_CHG_TIME_INDEX],minimum,args);
   }
if (pAccData->fields[ACC_EXP_TIME_INDEX] != NULL)
   {
   SET_SCALE_TO_MINIMUM(pAccData->fields[ACC_EXP_TIME_INDEX],minimum,args);
   }
if (pAccData->fields[ACC_LIFETIME_INDEX] != NULL)
   {
   SET_SCALE_TO_MINIMUM(pAccData->fields[ACC_LIFETIME_INDEX],minimum,args);
   }
if (pAccData->fields[ACC_MAX_LENGTH_INDEX] != NULL)
   {
   SET_SCALE_TO_MINIMUM(pAccData->fields[ACC_MAX_LENGTH_INDEX],minimum,args);
   }
if (pAccData->fields[ACC_REQUIRED_INDEX] != NULL)
   {
   XmToggleButtonSetState(pAccData->fields[ACC_REQUIRED_INDEX],False,False);
   }
if (pAccData->fields[ACC_USER_CHOOSE_INDEX] != NULL)
   {
   XmToggleButtonSetState(pAccData->fields[ACC_USER_CHOOSE_INDEX],False,False);
   }
if (pAccData->fields[ACC_GENERATED_INDEX] != NULL)
   {
   XmToggleButtonSetState(pAccData->fields[ACC_GENERATED_INDEX],False,False);
   }
if (pAccData->fields[ACC_RAND_CHARS_INDEX] != NULL)
   {
   XmToggleButtonSetState(pAccData->fields[ACC_RAND_CHARS_INDEX],False,False);
   }
if (pAccData->fields[ACC_RAND_LETTERS_INDEX] != NULL)
   {
   XmToggleButtonSetState(pAccData->fields[ACC_RAND_LETTERS_INDEX],False,False);
   }
if (pAccData->fields[ACC_TRIV_CHECKS_INDEX] != NULL)
   {
   XmToggleButtonSetState(pAccData->fields[ACC_TRIV_CHECKS_INDEX],False,False);
   }
}
/******************************************************************************
 * AccDisplayPasswordData()
 ******************************************************************************/
static void AccDisplayPasswordData(pAccData)
AccountDataType *pAccData;

{
int value;
int state;
int defaultFlag;
struct pr_passwd *prpwd = &pAccData->userInfo->userData.prpw;

defaultFlag = 0;

/* Minimum password change time */
if (prpwd->uflg.fg_min)
   value = TimeToWeeks(prpwd->ufld.fd_min);
else 
   {
   ADDMASKBIT(defaultFlag, ACC_CHG_TIME_INDEX);
   if (prpwd->sflg.fg_min)
      value = TimeToWeeks(prpwd->sfld.fd_min);
   else 
      value = 0;
   }
XmScaleSetValue(pAccData->fields[ACC_CHG_TIME_INDEX],value);

/* Password expiration time */
if (prpwd->uflg.fg_expire)
   value = TimeToWeeks(prpwd->ufld.fd_expire);
else 
   {
   ADDMASKBIT(defaultFlag, ACC_EXP_TIME_INDEX);
   if (prpwd->sflg.fg_expire)
      value = TimeToWeeks(prpwd->sfld.fd_expire);
   else 
      value = 0;
   }
XmScaleSetValue(pAccData->fields[ACC_EXP_TIME_INDEX],value);

/* Password lifetime */
if (prpwd->uflg.fg_lifetime)
   value = TimeToWeeks(prpwd->ufld.fd_lifetime);
else
   {
   ADDMASKBIT(defaultFlag, ACC_LIFETIME_INDEX);
   if (prpwd->sflg.fg_lifetime)
      value = TimeToWeeks(prpwd->sfld.fd_lifetime);
   else 
      value = 0;
   }
XmScaleSetValue(pAccData->fields[ACC_LIFETIME_INDEX],value);

/* Maximum password length */
if (prpwd->uflg.fg_maxlen)
   value = prpwd->ufld.fd_maxlen;
else
   {
   ADDMASKBIT(defaultFlag, ACC_MAX_LENGTH_INDEX);
   if (prpwd->sflg.fg_maxlen)
      value = prpwd->sfld.fd_maxlen;
   else 
      value = 0;
   }
XmScaleSetValue(pAccData->fields[ACC_MAX_LENGTH_INDEX],value);

/* Minimum password length */
if (prpwd->uflg.fg_minlen)
   value = prpwd->ufld.fd_minlen;
else
   {
   ADDMASKBIT(defaultFlag, ACC_MIN_LENGTH_INDEX);
   if (prpwd->sflg.fg_minlen)
      value = prpwd->sfld.fd_minlen;
   else 
      value = 0;
   }
XmScaleSetValue(pAccData->fields[ACC_MIN_LENGTH_INDEX],value);

/* Password History Limit */
if (prpwd->uflg.fg_pwdepth)
   value = (int) prpwd->ufld.fd_pwdepth;
else
   {
   ADDMASKBIT(defaultFlag, ACC_HIST_LIMIT_INDEX);
   if (prpwd->sflg.fg_pwdepth)
      value = (int) prpwd->sfld.fd_pwdepth;
   else 
      value = 0;
   }
XmScaleSetValue(pAccData->fields[ACC_HIST_LIMIT_INDEX],value);

/* 
 * Note that these are backwards.  The term "password required" is the 
 * opposite of null password.
 */
state = True;
if (prpwd->uflg.fg_nullpw)
   {
   if (prpwd->ufld.fd_nullpw)
      state = False;
   }
else
   {
   ADDMASKBIT(defaultFlag, ACC_REQUIRED_INDEX);
   if (prpwd->sflg.fg_nullpw && prpwd->sfld.fd_nullpw)
      state = False;
   }
XmToggleButtonSetState(pAccData->fields[ACC_REQUIRED_INDEX], state);

state = False;
if (prpwd->uflg.fg_pick_pwd)
   {
   if (prpwd->ufld.fd_pick_pwd)
      state = True;
   }
else
   {
   ADDMASKBIT(defaultFlag, ACC_USER_CHOOSE_INDEX);
   if (prpwd->sflg.fg_pick_pwd && prpwd->sfld.fd_pick_pwd)
      state = True;
   }
XmToggleButtonSetState(pAccData->fields[ACC_USER_CHOOSE_INDEX], state);

state = False;
if (prpwd->uflg.fg_gen_pwd)
   {
   if (prpwd->ufld.fd_gen_pwd)
      state = True;
   }
else
   {
   ADDMASKBIT(defaultFlag, ACC_GENERATED_INDEX);
   if (prpwd->sflg.fg_gen_pwd && prpwd->sfld.fd_gen_pwd)
      state = True;
   }
XmToggleButtonSetState(pAccData->fields[ACC_GENERATED_INDEX], state);

state = False;
if (prpwd->uflg.fg_gen_chars)
   {
   if (prpwd->ufld.fd_gen_chars)
      state = True;
   }
else
   {
   ADDMASKBIT(defaultFlag, ACC_RAND_CHARS_INDEX);
   if (prpwd->sflg.fg_gen_chars && prpwd->sfld.fd_gen_chars)
      state = True;
   }
XmToggleButtonSetState(pAccData->fields[ACC_RAND_CHARS_INDEX], state);

state = False;
if (prpwd->uflg.fg_gen_letters)
   {
   if (prpwd->ufld.fd_gen_letters)
      state = True; /* This was False in AccModPass.c: LoadUsers, An error? */
   }
else
   {
   ADDMASKBIT(defaultFlag, ACC_RAND_LETTERS_INDEX);
   if (prpwd->sflg.fg_gen_letters && prpwd->sfld.fd_gen_letters)
      state = True;
   }
XmToggleButtonSetState(pAccData->fields[ACC_RAND_LETTERS_INDEX], state);

state = False;
if (prpwd->uflg.fg_restrict)
   {
   if (prpwd->ufld.fd_restrict)
      state = True;
   }
else
   {
   ADDMASKBIT(defaultFlag, ACC_TRIV_CHECKS_INDEX);
   if (prpwd->sflg.fg_restrict && prpwd->sfld.fd_restrict)
      state = True;
   }
XmToggleButtonSetState(pAccData->fields[ACC_TRIV_CHECKS_INDEX], state);

state = False;
if (prpwd->uflg.fg_policy)
   {
   if (prpwd->ufld.fd_policy)
      state = True;
   }
else
   {
   ADDMASKBIT(defaultFlag, ACC_SITE_TRIV_CHECKS_INDEX);
   if (prpwd->sflg.fg_policy && prpwd->sfld.fd_policy)
      state = True;
   }
XmToggleButtonSetState(pAccData->fields[ACC_SITE_TRIV_CHECKS_INDEX], state);

/* 
 * Flush all those nasty events before we set ourselves up 
 * to do updates again.  This will hopefully keep our defaults 
 * list up to date.
 */
XSync(XtDisplay(pAccData->fields[ACC_CHG_TIME_INDEX]), False);
pAccData->ignoreUpdates = False;
}
/******************************************************************************
 * AccDisplayReferenceCount()
 ******************************************************************************/
static void AccDisplayReferenceCount(pAccData,type)
AccountDataType *pAccData;
int              type;

{
int count;
char temp[10];

struct pr_passwd *prpwd = &pAccData->userInfo->userData.prpw;

/*
 * Reference count is meaningless in the case of system default as all
 * accounts reference it.
 */
if (strcmp(SYSTEM_DEFAULT_TEMPLATE,prpwd->ufld.fd_template) == 0)
   {
   XmTextSetString(pAccData->tmpltcountText,"");
   return;
   }

/* 
 * WARNING: Calling ReferenceCount calls getprpwent which disturbs 
 * any outstanding call's data structure to query DBs such as getpwent.
 */
if (type == ACC_TEMPLATES)
    count = TemplateReferenceCount(prpwd->ufld.fd_name);
else 
    count = TemplateReferenceCount(prpwd->ufld.fd_template);

SET_TEXT_FROM_NUMBER(temp, count, pAccData->tmpltcountText);
}
/******************************************************************************
 * AccDisplayTemplateName()
 ******************************************************************************/
static void AccDisplayTemplateName(pAccData,type)
AccountDataType *pAccData;
int              type;

{
struct pr_passwd *prpwd = &pAccData->userInfo->userData.prpw;

if (type == ACC_USERS)
   XmTextSetString(pAccData->tmpltnameText, prpwd->ufld.fd_template);
else if (type == ACC_TEMPLATES)
   XmTextSetString(pAccData->tmpltnameText, prpwd->ufld.fd_name);
}
/******************************************************************************
 * AccountGetAnddDisplayDefaultData()
 ******************************************************************************/
static int AccountGetAndDisplayDefaultData(pAccData,w)
AccountDataType *pAccData;
Widget           w;

{
int i;
int value;
int state;
time_t tvalue;
struct tm *tm;
char time_str[26];
XmString xmstring;

/* Store the system default data in the dflt structure of AccData */
/* The default name parameter is ignored for now */
AccStoreSystemDefault(pAccData,"default",w);

/* Template name: hard code the display for now */
XmTextSetString(pAccData->tmpltnameText,SYSTEM_DEFAULT_TEMPLATE);

/* Time of Day Restriction */
XmTextSetString(pAccData->tod_tx,pAccData->dflt.tod);

/* Expiration Date */
tvalue = pAccData->dflt.expdate;
/* Only Display the date if it is nonzero; else no expiration date  */
if (tvalue != 0)
   {
   tm = localtime(&tvalue);
   sprintf(time_str,"%s %d %.2d:%.2d %4d",
           months[tm->tm_mon],tm->tm_mday,
           tm->tm_hour,tm->tm_min,tm->tm_year+1900);
   XmTextSetString(pAccData->expdate_tx,time_str);
   }

/* Maximum Failed Attempts */
value =  pAccData->dflt.maxTries;
XmScaleSetValue(pAccData->maxTries,value);

/* Nice Value */
value = (int) pAccData->dflt.nice;
XmScaleSetValue(pAccData->nice,value);

/* Account Locked */
if (pAccData->dflt.locked)
   XmToggleButtonSetState(pAccData->locked, True);
else
   XmToggleButtonSetState(pAccData->locked, False);

/* Password Parameters */

/* Minimum Password Change Time */
value = TimeToWeeks(pAccData->dflt.fieldValue[ACC_CHG_TIME_INDEX]);
XmScaleSetValue(pAccData->fields[ACC_CHG_TIME_INDEX],value);

/* Password Expiration Time */
value = TimeToWeeks(pAccData->dflt.fieldValue[ACC_EXP_TIME_INDEX]);
XmScaleSetValue(pAccData->fields[ACC_EXP_TIME_INDEX],value);

/* Password Lifetime */
value = TimeToWeeks(pAccData->dflt.fieldValue[ACC_LIFETIME_INDEX]);
XmScaleSetValue(pAccData->fields[ACC_LIFETIME_INDEX],value);

/* Maximum Password Length */
value = pAccData->dflt.fieldValue[ACC_MAX_LENGTH_INDEX];
XmScaleSetValue(pAccData->fields[ACC_MAX_LENGTH_INDEX],value);

/* Minimum Password Length */
value = pAccData->dflt.fieldValue[ACC_MIN_LENGTH_INDEX];
XmScaleSetValue(pAccData->fields[ACC_MIN_LENGTH_INDEX],value);

/* Password History Limit */
value = pAccData->dflt.fieldValue[ACC_HIST_LIMIT_INDEX];
XmScaleSetValue(pAccData->fields[ACC_HIST_LIMIT_INDEX],value);

/* Password Required */
state = True;
if (pAccData->dflt.fieldValue[ACC_REQUIRED_INDEX])
   state = False;
XmToggleButtonSetState(pAccData->fields[ACC_REQUIRED_INDEX], state);

/* Users choose own */
state = False;
if (pAccData->dflt.fieldValue[ACC_USER_CHOOSE_INDEX])
   state = True;
XmToggleButtonSetState(pAccData->fields[ACC_USER_CHOOSE_INDEX], state);

/* System Generated Password */
state = False;
if (pAccData->dflt.fieldValue[ACC_GENERATED_INDEX])
   state = True;
XmToggleButtonSetState(pAccData->fields[ACC_GENERATED_INDEX], state);

/* Random Character Password */
state = False;
if (pAccData->dflt.fieldValue[ACC_RAND_CHARS_INDEX])
   state = True;
XmToggleButtonSetState(pAccData->fields[ACC_RAND_CHARS_INDEX], state);

/* Random Letters Password */
state = False;
if (pAccData->dflt.fieldValue[ACC_RAND_LETTERS_INDEX])
   state = True;
XmToggleButtonSetState(pAccData->fields[ACC_RAND_LETTERS_INDEX], state);

/* Enforce Triviality Checks */
state = False;
if (pAccData->dflt.fieldValue[ACC_TRIV_CHECKS_INDEX])
   state = True;
XmToggleButtonSetState(pAccData->fields[ACC_TRIV_CHECKS_INDEX], state);

/* Enforce Site Triviality Checks */
state = False;
if (pAccData->dflt.fieldValue[ACC_SITE_TRIV_CHECKS_INDEX])
   state = True;
XmToggleButtonSetState(pAccData->fields[ACC_SITE_TRIV_CHECKS_INDEX], state);

#ifdef SEC_PRIV

/* Command Auths */
for (i = 0; i < pAccData->nauths; i++)
    {
    if (ISBITSET(pAccData->dflt.cprivs, cmd_priv[i].value))
       {
       xmstring = XmStringCreate(cmd_priv[i].name, charset);
       XmSelectListAddItem(pAccData->authList,xmstring,0);
       XmAvailableListDeleteItem(pAccData->authList,xmstring,0);
       XmStringFree(xmstring);
       }
    }

/* Base Privs */
for (i = 0; i < pAccData->nprivs; i++)
    {
    if (ISBITSET(pAccData->dflt.bprivs, sys_priv[i].value))
       {
       xmstring = XmStringCreate(sys_priv[i].name, charset);
       XmAvailableListDeleteItem(pAccData->baseList, xmstring,0);
       XmSelectListAddItem(pAccData->baseList,xmstring,0);
       XmStringFree(xmstring);
       }
    }

/* Kernel Auths */   
for (i = 0; i < pAccData->nprivs; i++)
    {
    if (ISBITSET(pAccData->dflt.sprivs, sys_priv[i].value))
       {
       xmstring = XmStringCreate(sys_priv[i].name, charset);
       XmAvailableListDeleteItem(pAccData->kernelList,xmstring,0);
       XmSelectListAddItem(pAccData->kernelList,xmstring,0);
       XmStringFree(xmstring);
       }
    }

#endif /* SEC_PRIV */

#ifdef SEC_MAC

if (pAccData->clearance_tx)
XmTextSetString(pAccData->clearance_tx,
           clearance_ir_to_er(&(pAccData->dflt.clearance)));

#endif /* SEC_MAC */

/*
 * Assign the Audit Display/Control Data to the user ufld structure as it
 * might be displayed later.
 */
pAccData->userInfo->userData.prpw.uflg.fg_auditdisp = 1;
strcpy(pAccData->userInfo->userData.prpw.ufld.fd_auditdisp,
       pAccData->dflt.auditdisp);

pAccData->userInfo->userData.prpw.uflg.fg_auditcntl = 1;
pAccData->userInfo->userData.prpw.ufld.fd_auditcntl = pAccData->dflt.auditcntl;
}
/******************************************************************************
 * AccountGetAndDisplayUserData()
 ******************************************************************************/
static int AccountGetAndDisplayUserData(pAccData,username,type)
AccountDataType *pAccData;
char            *username;
int              type;

{
struct passwd     *pwd;
struct group      *grp;
char               prigrpnam[25];
XmString           xmstring;
char             **cp;
char               temp[UID_SIZE];
int      count;

/* Get Account Information. No "free" is necessary. */
pwd = getpwnam(username);
if (pwd == (struct passwd *) 0)
   {
   /* Name has already been checked for validity, Display error message? */
   return(0);
   }

/* Only Display UID,Home Dir,Comments if User and not Template */
if (type == ACC_USERS)
   {
   /* Redisplay the username as it was cleared with all fields previously */
   XmTextSetString(pAccData->usernameText, username);

   /* Set the userInfo data structure name field */
   pAccData->userInfo->username = username;

   SET_TEXT_FROM_NUMBER(temp, pwd->pw_uid, pAccData->uidText);
   XmTextSetString(pAccData->directory, pwd->pw_dir);
   XmTextSetString(pAccData->comments, pwd->pw_gecos);
   }
else if (type == ACC_TEMPLATES)
   {
   /* Redisplay the username as it was cleared with all fields previously */
   XmTextSetString(pAccData->tmpltnameText, username);

   /* Set the userInfo data structure name field */
   pAccData->userInfo->tmpltname = username;
   }
 
if (strcmp(pwd->pw_shell,"")==0)
   XmTextSetString(pAccData->shell,"/bin/sh");
else XmTextSetString(pAccData->shell, pwd->pw_shell);

if ((grp = getgrgid(pwd->pw_gid)) != (struct group *) 0)
   {
   XmTextSetString(pAccData->priGroups,grp->gr_name);
   /* Remember primary group name so not to double count in secondary groups */
   strcpy(prigrpnam,grp->gr_name);
   }
else 
   {
   XmTextSetString(pAccData->priGroups,"");
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
                XmSelectListAddItem(pAccData->secGroups,xmstring,0);
                XmAvailableListDeleteItem(pAccData->secGroups,xmstring);
                XmStringFree(xmstring);
                }
             }
          }
      }
 
/* Get Password Information */
if ((type == ACC_USERS) || (type == ACC_TEMPLATE_ASSIGNMENT))
   {
   if (XGetUserInfo(username,&pAccData->userInfo->userData,
                    accounts_mw) != SUCCESS)
      {
      return(0);
      }
   }
else
   {
   if (XGetUserInfo(username,&pAccData->userInfo->userData,
                    acctmplt_mw) != SUCCESS)
      {
      return(0);
      }
   }

AccDisplayTemplateName(pAccData,type);
AccDisplayPasswordData(pAccData); /* Stolen from AccModPass.c */
AccDisplayLoginData(pAccData);    /* Stolen from AccModLogin.c */
AccDisplayReferenceCount(pAccData,type); 
#ifdef SEC_PRIV
AccDisplayPrivData(pAccData);     /* Stolen from AccModPriv.c */
AccDisplayCmdAuthData(pAccData);  /* Stolen from AccModAuth.c */
#endif /* SEC_PRIV */
#ifdef SEC_MAC
AccDisplayClearance(pAccData);
#endif /* SEC_MAC */
}
/******************************************************************************
 * LoadShellTXList()
 ******************************************************************************/
static void LoadShellTXList(w)
Widget	       w;

{
char        shell_str[256];
struct stat shell_stat;
FILE       *fp;

if ((fp= fopen(SHELL_INVENTORY_FILE, "r")) != (FILE *)NULL)
   {
   while (fscanf(fp,"%s",shell_str) != EOF)
         if (stat(shell_str,&shell_stat) == 0)
            {
            /* Check that the shell is executable by the world for now */
            if ((shell_stat.st_mode & (S_IEXEC >> 6)) != 0)
               XmTextListAddItem(w,XmStringCreate(shell_str,charset),0);
            }

   fclose(fp);
   }
}
/******************************************************************************
 * DisplayAllCommandAuthsAsAvailable()
 ******************************************************************************/
static void DisplayAllCommandAuthsAsAvailable(pAccData)
AccountDataType *pAccData;

{

/* 
 * Clear out old Selected Groups and Old Available Groups 
 * OR add Selected Groups to available OR ...
 */
XmSelectListDeleteAllItems(pAccData->authList);
XmAvailableListDeleteAllItems(pAccData->authList);
XmAvailableListAddItems(pAccData->authList,pAccData->auth_xmstrings,
                        pAccData->nauths,0);
}
/******************************************************************************
 * DisplayAllKernelAuthsAsAvailable()
 ******************************************************************************/
static void DisplayAllKernelAuthsAsAvailable(pAccData)
AccountDataType *pAccData;

{

XmSelectListDeleteAllItems(pAccData->kernelList);
XmAvailableListDeleteAllItems(pAccData->kernelList);
XmAvailableListAddItems(pAccData->kernelList,pAccData->priv_xmstrings,
                        pAccData->nprivs,0);
}
/******************************************************************************
 * DisplayAllBasePrivsAsAvailable()
 ******************************************************************************/
static void DisplayAllBasePrivsAsAvailable(pAccData)
AccountDataType *pAccData;

{

XmSelectListDeleteAllItems(pAccData->baseList);
XmAvailableListDeleteAllItems(pAccData->baseList);
XmAvailableListAddItems(pAccData->baseList,pAccData->priv_xmstrings,
                              pAccData->nprivs,0);
}
/******************************************************************************
 * DisplayAllGroupsAsAvailable()
 ******************************************************************************/
static void DisplayAllGroupsAsAvailable(pAccData)
AccountDataType *pAccData;

{

/* 
 * Clear out old Selected Groups and Old Available Groups 
 * OR add Selected Groups to available OR ...
 */
XmSelectListDeleteAllItems(pAccData->secGroups);
XmAvailableListDeleteAllItems(pAccData->secGroups);
XmAvailableListAddItems(pAccData->secGroups,pAccData->group_xmstrings,
                        pAccData->ngroups,0);
}
/******************************************************************************
 * LoadBaseAndKernelPrivs()
 ******************************************************************************/
static void LoadBaseAndKernelPrivs(pAccData)
AccountDataType *pAccData;

{
int         i;

for (i = 0; sys_priv[i].name != (char *) 0; i++);

pAccData->nprivs = i;

/* Malloc XmString arrays for the lists of all groups */
pAccData->priv_xmstrings =
   (XmString *)XtMalloc(sizeof(XmString) * pAccData->nprivs);

if (!pAccData->priv_xmstrings)
   MemoryError();
/* Dies */

for (i = 0; i < pAccData->nprivs; i++)
    {
    pAccData->priv_xmstrings[i] = XmStringCreate(sys_priv[i].name, charset);
    }
}
/******************************************************************************
 * LoadCommandAuths()          
 ******************************************************************************/
static void LoadCommandAuths(pAccData)
AccountDataType *pAccData;

{
int i;

build_cmd_priv();

pAccData->nauths = total_auths();

/* Malloc XmString arrays for the lists of command authorizations */
pAccData->auth_xmstrings = (XmString *)XtMalloc(sizeof(XmString) *
                                                        pAccData->nauths);

if (!pAccData->auth_xmstrings)
   MemoryError();
/* Dies */

for (i = 0; i < pAccData->nauths; i++)
    pAccData->auth_xmstrings[i] = XmStringCreate(cmd_priv[i].name,charset);
}
/******************************************************************************
 * LoadGroupNames()          
 ******************************************************************************/
static void LoadGroupNames(pAccData)
AccountDataType *pAccData;

{
int i;

GetAllGroups(&pAccData->ngroups, &pAccData->groups);

/* Malloc XmString arrays for the lists of all groups */
pAccData->group_xmstrings =
   (XmString *)XtMalloc(sizeof(XmString) * pAccData->ngroups);

if (!pAccData->group_xmstrings)
   MemoryError();
/* Dies */

for (i = 0; i < pAccData->ngroups; i++)
    pAccData->group_xmstrings[i] =
       XmStringCreate(pAccData->groups[i], charset);
}
/******************************************************************************
 * AccLoadAccountDataFromScreen()           
 ******************************************************************************/
Boolean AccLoadAccountDataFromScreen(pAccData,pwdp,grp,type)
AccountDataType *pAccData;
struct passwd    *pwdp;
struct group     *grp;
int               type;

{
char        *temp;

/* Username */
if (type == ACC_USERS)
   {
   /* User name */
   pwdp->pw_name = XmTextFieldGetString(pAccData->usernameText);
   if (removeBlanks(pwdp->pw_name))
      XmTextSetString(pAccData->usernameText,pwdp->pw_name);

   /* UID */
   temp = XmTextFieldGetString(pAccData->uidText);
   if (removeBlanks(temp))
      XmTextSetString(pAccData->uidText, temp);
   pwdp->pw_uid = atoi(temp);
   XtFree(temp);

   /* Home Directory */
   pwdp->pw_dir = XmTextFieldGetString(pAccData->directory);
   if (removeBlanks(pwdp->pw_dir))
      XmTextSetString(pAccData->directory, pwdp->pw_dir);

   /* Comments */
   pwdp->pw_gecos = XmTextFieldGetString(pAccData->comments);
   }
else /* type == ACC_TEMPLATES */
   {
   /* User name */
   pwdp->pw_name = XmTextFieldGetString(pAccData->tmpltnameText);
   if (removeBlanks(pwdp->pw_name))
      XmTextSetString(pAccData->tmpltnameText,pwdp->pw_name);

   /* Assign Standard Template Data */
   pwdp->pw_uid = choosetemplateuid();
   pwdp->pw_dir = TEMPLATE_HOME_DIR;
   pwdp->pw_gecos = TEMPLATE_COMMENT;
   }

/* Shell */
pwdp->pw_shell = XmTextFieldGetString(pAccData->shell);
if (removeBlanks(pwdp->pw_shell))
   XmTextSetString(pAccData->shell, pwdp->pw_shell);

/* Primary Group */
temp = XmTextFieldGetString(pAccData->priGroups);
if ((grp = getgrnam(temp)) != NULL)
   {
   pwdp->pw_gid = (int) grp->gr_gid;
   }
else
   {
   /* Field was never entered...Post an error message */
   pwdp->pw_gid = (int) -1;
   }
if (temp)
   XtFree(temp);

return(True);
}

/*
 ==============================================================================
 = P U B L I C   R O U T I N E S   T O   S U P P O R T   A C C O U N T S
 ============================================================================== 
*/

/******************************************************************************
 * NEW_VUIT_Manage()
 ******************************************************************************/
void NEW_VUIT_Manage(widget_name,w,parent)
char   *widget_name;
Widget *w;
Widget  parent;

{
Widget         id;
MrmType        class_id;
Window         pop_window;
XWindowChanges values;

if (HashLookup(widget_name, &id))
   if (XtIsManaged(id))
      {
      pop_window = XtWindow(XtParent(id));
      values.x = values.y = values.width = values.height =
                 values.border_width = values.sibling = NULL;
      values.stack_mode = Above;
      XConfigureWindow(display, pop_window, CWStackMode, &values);
      }
   else
      XtManageChild(id);
else
   {
   MrmFetchWidget(s_MrmHierarchy, widget_name, parent, w, &class_id);
   XtManageChild(*w);
   HashRegister(widget_name, *w);
   }
}
/******************************************************************************
 * AccCommonReplaceSystemMenuClose()
 ******************************************************************************/
void AccCommonReplaceSystemMenuClose(tl,CB)
Widget tl;
void (*CB)();

{
int  n;
Arg  args[10];
Atom delete_window_atom;

n = 0;
XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING);  n++;
XtSetValues(tl,args,n);

/* This only needs to be retrieved the first time through...oh well */
delete_window_atom = XmInternAtom(display,"WM_DELETE_WINDOW",1);


/* Specify a callback to be invoked when "close" is chosen from mwm menu */
XmAddWMProtocolCallback(tl,delete_window_atom,CB,NULL);
}
/******************************************************************************
 * AccountIsNew()
 ******************************************************************************/
Boolean AccountIsNew(name)
char *name;

{
if (UserNameExists(name))
   {
   return(False);
   }

return(True);
}
/******************************************************************************
 * TemplateIsNew()
 ******************************************************************************/
Boolean TemplateIsNew(name)
char *name;

{
if (TemplateNameExists(name))
   {
   return(False);
   }

return(True);
}
/******************************************************************************
 * ForceSelListPolicy()
 ******************************************************************************/
void ForceSelListPolicy(w)
Widget	       w;

{
int         n;
Arg         args[10];

n = 0;
XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC);  n++;
XtSetValues(w, args, n);
}
/******************************************************************************
 * LoadPriGroupTXList()
 ******************************************************************************/
void LoadPriGroupTXList(w)
Widget	       w;

{
int         i,name_count;
char      **pnames;

GetAllGroups(&name_count,&pnames);

/* Delete All Items in list in order to reload */
XmTextListDeleteAllItems(w);

for (i = 0; i < name_count; i++)
    {
    XmTextListAddItem(w,XmStringCreate(pnames[i],charset),0);
    }
}
/******************************************************************************
 * LoadTmpltnameTXList()
 ******************************************************************************/
void LoadTmpltnameTXList(w)
Widget w;

{
int         i,name_count;
char      **pnames;

/* Load the Username Text List with Usernames or Template Names */

GetAllTemplates(&name_count,&pnames);

/* Delete All Items in list in order to reload */
XmTextListDeleteAllItems(w);

for (i = 0; i < name_count; i++)
    {
    XmTextListAddItem(w,XmStringCreate(pnames[i],charset),0);
    }

/* Free the names after processing (see common/Utils.c) */
free_cw_table(pnames);
}
/******************************************************************************
 * LoadUsernameTXList()
 ******************************************************************************/
void LoadUsernameTXList(w,type)
Widget w;
int    type;

{
int         i,name_count;
char      **pnames;

/* Load the Username Text List with Usernames or Template Names */

switch (type)
   {
   case USER_MODIFICATION:
/*******
 * DEBUG PURPOSES ONLY!!!!!        T O   B E   R E M O V E D
 ******/
           endpwent();
           GetAllNonIssoUsers(&name_count,&pnames);
        break;
   case ISSO_MODIFICATION:
/*******
 * DEBUG PURPOSES ONLY!!!!!        T O   B E   R E M O V E D
 ******/
           endpwent();
           GetAllIssoUsers(&name_count,&pnames);
        break;
   case USER_AND_ISSO_MODIFICATION:
/*******
 * DEBUG PURPOSES ONLY!!!!!        T O   B E   R E M O V E D
 ******/
           endpwent();
           GetAllUsers(&name_count,&pnames);
        break;
   default: printf("ERROR: rcvd unknown user type %d\n",type);
   }

/* Delete All Items in list in order to reload */
XmTextListDeleteAllItems(w);

for (i = 0; i < name_count; i++)
    {
    XmTextListAddItem(w,XmStringCreate(pnames[i],charset),0);
    }

/* Free the names after processing (see common/Utils.c) */
free_cw_table(pnames);
}

/******************************************************************************
 * AccFreeGroupDataStructure()
 ******************************************************************************/
void AccFreeGroupDataStructure(secGroup_names,secGroup_count)
char ***secGroup_names;
int     secGroup_count;

{
int i;

for (i=0; i<secGroup_count; i++)
    {
    XtFree((char *)(*secGroup_names)[i]);
    }
XtFree((char *)(*secGroup_names));
}
/******************************************************************************
 * AccFreePasswordDataStructure();
 ******************************************************************************/
void AccFreePasswordDataStructure(pwdp)
struct passwd *pwdp;

{
/* XtFree returns if passed a NULL pointer */
XtFree(pwdp->pw_name);
XtFree(pwdp->pw_dir);
XtFree(pwdp->pw_shell);
XtFree(pwdp->pw_gecos);

/* Set to NULL in order to check before further freeing...*/
pwdp->pw_name = NULL;
pwdp->pw_dir = NULL;
pwdp->pw_shell = NULL;
pwdp->pw_gecos = NULL;
}
/******************************************************************************
 * AccAssignTemplate()
 ******************************************************************************/
void AccAssignTemplate(pAccData,tmpltname)
AccountDataType *pAccData;
char   *tmpltname;

{
char              *str;
struct passwd     *pwd;
char               temp[UID_SIZE];
struct pr_passwd  *prpwd;

/* Get the Protected Password DB data pointer */
prpwd = &pAccData->userInfo->userData.prpw;
 
/*
 * extract the current username string for safekeeping, cannot free this
 * string in this routine as it will be assigned to user data structure.
 */
str = XmTextFieldGetString(pAccData->usernameText);

/* Init all graphic object settings and free pAccData->userInfo->username */
AccClearGraphicObjects(pAccData);

/* Populate the screen with Template Data */
if (strcmp(SYSTEM_DEFAULT_TEMPLATE,tmpltname) == 0)
   AccountGetAndDisplayDefaultData(pAccData,accounts_mw);
else AccountGetAndDisplayUserData(pAccData,tmpltname,ACC_TEMPLATE_ASSIGNMENT);

/* Replace template name with user name on screen and in user data field */
pAccData->userInfo->username = str;
XmTextSetString(pAccData->usernameText,str);

/* Display template name on screen */
XmTextSetString(pAccData->tmpltnameText,tmpltname);

/*
 * Get Account Information. Still valid as no save has been attempted.
 * No "free" is necessary. The following will return NULL if the default
 * template is beeing referenced.
 */
pwd = getpwnam(str);

/* 
 * Replace the UID, Home Dir, and Comments if available. If this user is
 * not valid, don't flag the error here. The philosophy is to trap at
 * time of save but user might change username till then.
 */
if (pwd != (struct passwd *) 0)
   {
   /* Valid User */
   SET_TEXT_FROM_NUMBER(temp, pwd->pw_uid, pAccData->uidText);
   XmTextSetString(pAccData->directory, pwd->pw_dir);
   XmTextSetString(pAccData->comments, pwd->pw_gecos);
   }
}
/******************************************************************************
 * AccEntNameEntered()
 ******************************************************************************/
void AccEntNameEntered(pAccData, w, username, type)
AccountDataType *pAccData;
Widget           w;
char            *username;
int              type;

{

if (type == ACC_USERS)
   {
   if (! XmTextListIsSearchString(w))
      {
      if ((XmTextListItemExists(w,username))&& (UserNameExists(username)))
         {
         /* Clear the Graphic Objects before re-stating them */
         AccClearGraphicObjects(pAccData);
         AccountGetAndDisplayUserData(pAccData,username,type);
         }
      else
         PostErrorDialog("msg_acc_error_user_not_found",accounts_mw,NULL,NULL,
                         NULL,NULL,NULL,NULL);
      }
   }

else /* type == ACC_TEMPLATES */
   {
   if (! XmTextListIsSearchString(w))
      {
      if ((XmTextListItemExists(w,username))&& (UserNameExists(username)))
         {
         AccClearGraphicObjects(pAccData);
         AccountGetAndDisplayUserData(pAccData,username,type);
         }
      else if (strcmp(SYSTEM_DEFAULT_TEMPLATE,username) == 0)
         {
         AccClearGraphicObjects(pAccData);
         AccountGetAndDisplayDefaultData(pAccData,acctmplt_mw);
         }
      }
   /* TODO: What to do if the template name_to_be is in use as an account? */
   }
}
/******************************************************************************
 * AccClearGraphicObjects()
 ******************************************************************************/
void AccClearGraphicObjects(pAccData)
AccountDataType *pAccData;

{

/* Clear Availabe/Selected Sec Group List and Re-Initialize */
if (pAccData->secGroups)
   DisplayAllGroupsAsAvailable(pAccData);

#ifdef SEC_PRIV
/* Clear Availabe/Selected Command Auth List and Re-Initialize */
if (pAccData->authList)
   DisplayAllCommandAuthsAsAvailable(pAccData);

/* Clear Availabe/Selected Base Privileges List and Re-Initialize */
if (pAccData->baseList)
   DisplayAllBasePrivsAsAvailable(pAccData);

/* Clear Availabe/Selected Kernel Auths List and Re-Initialize */
if (pAccData->kernelList)
   DisplayAllKernelAuthsAsAvailable(pAccData);
#endif /* SEC_PRIV */

#ifdef SEC_MAC
if (pAccData->clearance_tx)
   XmTextSetString(pAccData->clearance_tx,"");
#endif /* SEC_MAC */

/* Free Username Text Field if not NULL and clear username Text Widget */
/* XtFree(pAccData->userInfo->username); */
pAccData->userInfo->username = NULL;
if (pAccData->usernameText)
   XmTextSetString(pAccData->usernameText,"");
   
/* Clear tmpltname Text Widget */
/* XtFree(pAccData->userInfo->tmpltname); */
pAccData->userInfo->tmpltname = NULL;
if (pAccData->tmpltnameText)
   XmTextSetString(pAccData->tmpltnameText,"");

/* Clear UID Text Widget */
if (pAccData->uidText)
   XmTextSetString(pAccData->uidText,"");
   
/* Clear Group ID */
if (pAccData->gidText)
   XmTextSetString(pAccData->gidText,"");

/* Clear Directory Text widget */
if (pAccData->directory)
   XmTextSetString(pAccData->directory,"");

/* Clear Shell Widget */
if (pAccData->shell)
   XmTextSetString(pAccData->shell,"");

/* Clear Comments Widget */
if (pAccData->comments)
   XmTextSetString(pAccData->comments,"");

/* Clear Primary Group Selection */
if (pAccData->priGroups)
   XmTextSetString(pAccData->priGroups,"");

/* Clear Password Scale Widgets and Toggle Buttons */
AccClearPasswordGraphicObjects(pAccData);

/* Clear Login Scale Widgets and Toggle Buttons */
AccClearLoginGraphicObjects(pAccData);
}
/******************************************************************************
 * InitializeGraphicObjects()
 ******************************************************************************/
void InitializeGraphicObjects(pAccData)
AccountDataType *pAccData;

{

/* Initialize the Group Selection List with all Available Group Names */
if (pAccData->secGroups)
   DisplayAllGroupsAsAvailable(pAccData);

#ifdef SEC_PRIV
/* Initialize the Command Auth List with all Available Command Auths */
if (pAccData->authList)
   DisplayAllCommandAuthsAsAvailable(pAccData);

/* Initialize the Base Privs Selection List with all Available Privs */
if (pAccData->baseList)
   DisplayAllBasePrivsAsAvailable(pAccData);

/* Initialize the Kernel Auths Selection List with all Available Privs */
if (pAccData->kernelList)
   DisplayAllKernelAuthsAsAvailable(pAccData);
#endif /* SEC_PRIV */

/* Fill Login Shell Text List */
if (pAccData->shell)
   LoadShellTXList(pAccData->shell);
}
/******************************************************************************
 * InitializeWidgetIDsToNULL()          
 ******************************************************************************/
void InitializeWidgetIDsToNULL(pAccData)
AccountDataType *pAccData;

{
int i;

pAccData->apply_pb         = NULL;
pAccData->ok_pb            = NULL;
pAccData->cancel_pb        = NULL;
pAccData->help_pb          = NULL;
pAccData->usernameText     = NULL;
pAccData->usernameLBL      = NULL;
pAccData->tmpltnameText    = NULL;
pAccData->tmpltcountText   = NULL;
pAccData->uidText          = NULL;
pAccData->gidText          = NULL;
pAccData->directory        = NULL;
pAccData->shell            = NULL;
pAccData->comments         = NULL;
pAccData->priGroups        = NULL;
pAccData->secGroups        = NULL;
pAccData->baseList         = NULL;
pAccData->kernelList       = NULL;
pAccData->authList         = NULL;
pAccData->maxTries         = NULL;
pAccData->nice             = NULL;
pAccData->locked           = NULL;
pAccData->tod_tx           = NULL;
pAccData->expdate_tx       = NULL;

for (i=0; i<ACC_NUM_PASSWD_FIELDS; i++)
    pAccData->fields[i] = NULL;
}
/******************************************************************************
 * InitializeUserDataStructure()          
 ******************************************************************************/
void InitializeUserDataStructure(pAccData)
AccountDataType *pAccData;

{

/* Load in all groups on the system */
LoadGroupNames(pAccData);

#ifdef SEC_PRIV
/* Load in Command Authorizations */
LoadCommandAuths(pAccData);

/* Load in Base Privs and Kernel Authorizations (one list)*/
LoadBaseAndKernelPrivs(pAccData);
#endif /* SEC_PRIV */
}
/******************************************************************************
 * AccLoadDataFromScreen()
 ******************************************************************************/
Boolean AccLoadDataFromScreen(pAccData,pwdp,grpp,names,count,type)
AccountDataType *pAccData;
struct passwd    *pwdp;
struct group     *grpp;
char           ***names;
int              *count;
int               type;

{
/* Retrieve Data from graphic widgets */
if (! AccLoadAccountDataFromScreen(pAccData,pwdp,grpp,type) ||
    ! AccLoadSecGroupDataFromScreen(pAccData,names,count) ||
    ! AccLoadPasswordDataFromScreen(pAccData,type) ||
#ifdef SEC_PRIV
    ! AccLoadCommandAuthsFromScreen(pAccData) ||
    ! AccLoadPrivsFromScreen(pAccData) ||
#endif /* SEC_PRIV */
#ifdef SEC_MAC
    ! AccLoadClearanceFromScreen(pAccData,type) ||
#endif /* SEC_MAC */
    ! AccLoadLoginDataFromScreen(pAccData,type))
   return(False);
return(True);
}
/******************************************************************************
 * AccCommonDeleteAccount()
 ******************************************************************************/
void AccCommonDeleteAccount(pwd,pAccData,tl,mod_state)
struct passwd    pwd;
AccountDataType *pAccData;
Widget           tl;
int             *mod_state;

{
int     ret;
char  **secGroup_names;
struct  pr_passwd *prpwd;
struct  passwd *new_pwdp;
char   *delete_str;
char   *pr_path_str;
char   *dir_path_str;
struct  stat sb;

/* Get the Protected Password DB data pointer */
prpwd = &pAccData->userInfo->userData.prpw;

if (mod_state[MOD_ETC_PASSWD])
   {
   /* Delete the /etc/passwd entry */
   if ((new_pwdp = getpwnam(pwd.pw_name)) != (struct passwd *) 0)
      {
      ret = XDeleteUserpwd(&pwd,tl);

      /* Remove the Password cache files */
      unlink(PASSWD_PAG);
      unlink(PASSWD_DIR);

      /* Remove the UID cache */
      unlink(PW_ID_MAP);
      }
   }

if (mod_state[MOD_ETC_GROUP])
   {
   /* Delete the user/group enties */
   secGroup_names = (char **)Malloc(sizeof(char *) * (1));
   if (secGroup_names == NULL)
      {
      /* Memory Error */
      exit(1);
      }

   (secGroup_names)[0] = (char *) strdup("\0");
   ret = XModifyUserGroup(pwd.pw_name,INVALID_GROUP_ID,secGroup_names,tl);
   XtFree((char *)secGroup_names);
   }

if (mod_state[MOD_PR_PASSWD])
   {
   /* "/tcb/files/auth/?/name\0" */
   pr_path_str = (char *)XtMalloc(sizeof(char)*(strlen(pwd.pw_name)+20));
   if (pr_path_str != (char *)NULL)
      {
      sprintf(pr_path_str,"/tcb/files/auth/%c/%s",pwd.pw_name[0],pwd.pw_name);
      if (stat(pr_path_str,&sb) == 0) /* 0 indicates success/existance */
         {
         /* Load the Template name */
         strcpy(prpwd->ufld.fd_name,pwd.pw_name);

         /* Set fg_name == 0 so protected DB entry will be deleted */
         prpwd->uflg.fg_name = 0;

         /* Set the is_template flag to 0 so lib thinks we are deleting user */
         prpwd->uflg.fg_istemplate = 1;
         prpwd->ufld.fd_istemplate = 0;

         /* Delete the Protected Password Database Entry */
         ret = XWriteUserInfo(&pAccData->userInfo->userData,tl);
         }
      XtFree(pr_path_str);
      }
   }


if (mod_state[MOD_HOME_DIR])
   {
   /* Delete the Home Dir if Created */
   if (stat(pwd.pw_dir,&sb) == 0) /* 0 indicates success/existance */
      {
      /* "rm -fr pwd->pw_dir\0" */
      dir_path_str = (char *)XtMalloc(sizeof(char)*(strlen(pwd.pw_dir) +10));
      if (dir_path_str != (char *)NULL)
         {
         sprintf(dir_path_str,"rm -fr %s",pwd.pw_dir);
         system(dir_path_str);
         XtFree(dir_path_str);
         }
      }
   }
}
/******************************************************************************/
/*                                                                            */
/* C A L L B A C K S   F O R   B O T H   A C C O U N T  S C R E E N S         */
/*                                                                            */
/******************************************************************************/

#ifdef SEC_MAC

/******************************************************************************
 * AccCommonClearanceCancelCB()  * Private Callback format of dxlabel *
 ******************************************************************************/
void AccCommonClearanceCancelCB(w,pAccData)
Widget            w;
AccountDataType  *pAccData;

{
}
/******************************************************************************
 * AccCommonClearanceOKCB()      * Private Callback format of dxlabel *
 ******************************************************************************/
void AccCommonClearanceOKCB(w,pAccData,pInf,pSen,labelText)
Widget            w;
AccountDataType  *pAccData;
ilb_ir_t         *pInf;
mand_ir_t        *pSen;
char             *labelText;

{
if (pAccData->clearance_tx)
   XmTextSetString(pAccData->clearance_tx,clearance_ir_to_er(pSen));
}
/******************************************************************************
 * AccCommonClearanceCB()
 ******************************************************************************/
void AccCommonClearanceCB(w,pAccData,reason)
Widget            w;
AccountDataType  *pAccData;
unsigned long    *reason;

{
Widget     label;
Widget     parent;
mand_ir_t *pclearance;

/* Retrieve the Current Clearance from the screen */
if (pAccData->clearance_tx)
   pclearance = clearance_er_to_ir(
                (char *)XmTextGetString(pAccData->clearance_tx));

parent = XtParent(XtParent(XtParent(w)));

label = (Widget) dxlabel(         parent,
                           L_SEN|L_CLEAR, /* flags */
                          (ilb_ir_t *) 0, /* Information Label */
                              pclearance, /* Sensitivity Label */
                             mand_clrnce, /* Clearance */
            "Accounts: Change Clearance", /* Message Text */
                  AccCommonClearanceOKCB, /* OK Callback */
              AccCommonClearanceCancelCB, /* Cancel Callback */
                       (char *)pAccData); /* Data Pointer */

XtManageChild(label);

if (pclearance != (mand_ir_t *)NULL)
   mand_free_ir(pclearance);
}

#endif /* SEC_MAC */


/******************************************************************************
 * AccCommonInitWidgetRetrieval()
 ******************************************************************************/
void AccCommonInitWidgetRetrieval(pAccData)
AccountDataType *pAccData;

{
pWidgetRetrievalAccData = pAccData;
}
/******************************************************************************
 * AccCommonAddBasePrivToKernelCB()
 ******************************************************************************/
void AccCommonAddBasePrivToKernelCB(w,pAccData,call_data)
Widget                w;
AccountDataType      *pAccData;
XmListCallbackStruct *call_data;

{
char *str;

XmStringGetLtoR(call_data->item,XmSTRING_DEFAULT_CHARSET,&str);

if (! XmSelectListItemExists(pAccData->kernelList,str))
   {
   XmSelectListAddItem(pAccData->kernelList,call_data->item,0);
   XmAvailableListDeleteItem(pAccData->kernelList,call_data->item);
   }

XtFree(str);
}
/******************************************************************************
 * AccCommonRemoveBasePrivCB()
 ******************************************************************************/
void AccCommonRemoveBasePrivCB(w,pAccData,call_data)
Widget                w;
AccountDataType      *pAccData;
XmListCallbackStruct *call_data;

{
char *str;

XmStringGetLtoR(call_data->item,XmSTRING_DEFAULT_CHARSET,&str);

if (XmSelectListItemExists(pAccData->baseList,str))
   {
   XmSelectListDeleteItem(pAccData->baseList,call_data->item);
   XmAvailableListAddItem(pAccData->baseList,call_data->item,0);
   }

XtFree(str);
}
/******************************************************************************
 * AccCommonCreateMWCB()
 ******************************************************************************/
void AccCommonCreateMWCB(w,tag,reason)
Widget         w;
int           *tag;
unsigned long *reason;

{
AccountDataType *pAccData=pWidgetRetrievalAccData;

/*
 * This callback creates the main window user data structure that drives
 * the screen. This was attached to the widget user data field because
 * each subscreen in the old role programs maintained its own. Now,
 * the data can be static global?
 */
CreateUserData(pAccData);
}
/******************************************************************************
 * AccCommonGetWidgetCB()       
 ******************************************************************************/
void AccCommonGetWidgetCB(w,widget_id,reason)
Widget	       w;
int	      *widget_id;
unsigned long *reason;

{
int         i;
Arg         args[10];
AccountDataType *pAccData=pWidgetRetrievalAccData;

switch (*widget_id)
   {
      case ACC_USERNAME:
             pAccData->usernameText = w;
           break;
      case ACC_USERNAMELBL:
             pAccData->usernameLBL = w;
           break;
      case ACC_TEMPLATE:
             pAccData->tmpltnameText = w;
           break;
      case ACC_TEMPLATE_COUNT:
             pAccData->tmpltcountText = w;
           break;
      case ACC_PRI_GROUP:
             pAccData->priGroups = w;
           break;
      case ACC_SHELL:
             pAccData->shell = w;
           break;
      case ACC_HOME_DIR:
             pAccData->directory = w;
           break;
      case ACC_UID:
             pAccData->uidText = w;
           break;
      case ACC_COMMENTS:
             pAccData->comments = w;
           break;
      case ACC_COMMENTSLBL:
             pAccData->commentsLBL = w;
           break;
      case ACC_MAX_TRIES:
             pAccData->maxTries = w;
           break;
      case ACC_NICE:
             pAccData->nice = w;
           break;
      case ACC_EXP_DATE:
             pAccData->expdate_tx = w;
           break;
      case ACC_TOD_DATE:
             pAccData->tod_tx = w;
           break;
      case ACC_LOCKED:
             pAccData->locked = w;
           break;
      case ACC_CHG_TIME: 
             pAccData->fields[ACC_CHG_TIME_INDEX] = w;
           break;
      case ACC_EXP_TIME: 
             pAccData->fields[ACC_EXP_TIME_INDEX] = w;
           break;
      case ACC_LIFETIME: 
             pAccData->fields[ACC_LIFETIME_INDEX] = w;
           break;
      case ACC_MAX_LENGTH: 
             pAccData->fields[ACC_MAX_LENGTH_INDEX] = w;
           break;
      case ACC_MIN_LENGTH: 
             pAccData->fields[ACC_MIN_LENGTH_INDEX] = w;
           break;
      case ACC_HIST_LIMIT:
             pAccData->fields[ACC_HIST_LIMIT_INDEX] = w;
           break;
      case ACC_REQUIRED: 
             pAccData->fields[ACC_REQUIRED_INDEX] = w;
           break;
      case ACC_USER_CHOOSE: 
             pAccData->fields[ACC_USER_CHOOSE_INDEX] = w;
           break;
      case ACC_GENERATED: 
             pAccData->fields[ACC_GENERATED_INDEX] = w;
           break;
      case ACC_RAND_CHARS: 
             pAccData->fields[ACC_RAND_CHARS_INDEX] = w;
           break;
      case ACC_RAND_LETTERS: 
             pAccData->fields[ACC_RAND_LETTERS_INDEX] = w;
           break;
      case ACC_TRIV_CHECKS: 
             pAccData->fields[ACC_TRIV_CHECKS_INDEX] = w;
           break;
      case ACC_SITE_TRIV_CHECKS: 
             pAccData->fields[ACC_SITE_TRIV_CHECKS_INDEX] = w;
           break;
      case ACC_CMD_AUTHS: 
             pAccData->authList = w;
             ForceSelListPolicy(w);
           break;
      case ACC_BASE_PRIVS: 
             pAccData->baseList = w;
             XmSelListAddAvailableCallback(pAccData->baseList,
                                           AccCommonAddBasePrivToKernelCB,
                                           (XtPointer) pAccData);             
             ForceSelListPolicy(w);
           break;
      case ACC_KERNEL_AUTHS: 
             pAccData->kernelList = w;
             XmSelListAddSelectedCallback(pAccData->kernelList,
                                          AccCommonRemoveBasePrivCB,
                                          (XtPointer) pAccData);             
             ForceSelListPolicy(w);
           break;
      case ACC_CLEARANCE_TX:
             pAccData->clearance_tx = w;
           break;

#ifdef SEC_MAC
      case ACC_CLEARANCE_PB:
             pAccData->clearance_pb = w;
             XtAddCallback(pAccData->clearance_pb,XmNactivateCallback,
                           (XtCallbackProc)AccCommonClearanceCB,
                           (XtPointer) pAccData);
           break;
#endif /* SEC_MAC */

      case ACC_SEC_GROUPS: 
             pAccData->secGroups = w;
             ForceSelListPolicy(w);
           break;
      case ACC_APPLY_PB: 
             pAccData->apply_pb= w;
           break;
      case ACC_OK_PB: 
             pAccData->ok_pb= w;
           break;
      case ACC_CANCEL_PB: 
             pAccData->cancel_pb= w;
           break;
      case ACC_HELP_PB: 
             pAccData->help_pb= w;
           break;
      default : printf("Got Unknown Widget ID %d\n",*widget_id);
   }
}
/******************************************************************************/
/*                                                                            */
/* C O M M O N   I N I T I A L I Z A T I O N                                  */
/*                                                                            */
/******************************************************************************/
/******************************************************************************
 * AccCommonInitialize()       
 ******************************************************************************/
void AccCommonInitialize()

{
MrmRegisterNames (reglist,(sizeof reglist/sizeof reglist[0]));
MrmRegisterTextListClass();
MrmRegisterSelectListClass();
MrmRegisterSelectListTBClass();
}
