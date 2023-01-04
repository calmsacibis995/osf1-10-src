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
/*
 * @(#)$RCSfile: AccCommon.h,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/12/20 21:31:09 $
 */

/******************************************************************************* *
 *
 *
 ******************************************************************************/

#define MAX_WEEKS       1103

#define ACC_CHG_TIME_INDEX          0
#define ACC_EXP_TIME_INDEX          (ACC_CHG_TIME_INDEX+1)
#define ACC_LIFETIME_INDEX          (ACC_EXP_TIME_INDEX+1)
#define ACC_MAX_LENGTH_INDEX        (ACC_LIFETIME_INDEX+1)
#define ACC_MIN_LENGTH_INDEX        (ACC_MAX_LENGTH_INDEX+1)
#define ACC_HIST_LIMIT_INDEX        (ACC_MIN_LENGTH_INDEX+1)
#define ACC_REQUIRED_INDEX          (ACC_HIST_LIMIT_INDEX+1)
#define ACC_USER_CHOOSE_INDEX       (ACC_REQUIRED_INDEX+1)
#define ACC_GENERATED_INDEX         (ACC_USER_CHOOSE_INDEX+1)
#define ACC_RAND_CHARS_INDEX        (ACC_GENERATED_INDEX+1)
#define ACC_RAND_LETTERS_INDEX      (ACC_RAND_CHARS_INDEX+1)
#define ACC_TRIV_CHECKS_INDEX       (ACC_RAND_LETTERS_INDEX+1)
#define ACC_SITE_TRIV_CHECKS_INDEX  (ACC_TRIV_CHECKS_INDEX+1)
#define ACC_LAST_PASSWD_FIELD       (ACC_SITE_TRIV_CHECKS_INDEX+1)

/*
 * List of Widget Id's to be passed back from uil files through widget
 * creation callbacks
 */
#define ACC_USERNAME           0
#define ACC_USERNAMELBL        (ACC_USERNAME+1)
#define ACC_TEMPLATE           (ACC_USERNAMELBL+1)
#define ACC_TEMPLATE_COUNT     (ACC_TEMPLATE+1)
#define ACC_PRI_GROUP          (ACC_TEMPLATE_COUNT+1)
#define ACC_SHELL              (ACC_PRI_GROUP+1)
#define ACC_HOME_DIR           (ACC_SHELL+1)
#define ACC_UID                (ACC_HOME_DIR+1)
#define ACC_GID                (ACC_UID+1)
#define ACC_COMMENTS           (ACC_GID+1)
#define ACC_COMMENTSLBL        (ACC_COMMENTS+1)
#define ACC_NICE               (ACC_COMMENTSLBL+1)
#define ACC_LOCKED             (ACC_NICE+1)
#define ACC_MAX_TRIES          (ACC_LOCKED+1)
#define ACC_EXP_DATE           (ACC_MAX_TRIES+1)
#define ACC_TOD_DATE           (ACC_EXP_DATE+1)
#define ACC_CHG_TIME           (ACC_TOD_DATE+1)
#define ACC_EXP_TIME           (ACC_CHG_TIME+1)
#define ACC_LIFETIME           (ACC_EXP_TIME+1)
#define ACC_MAX_LENGTH         (ACC_LIFETIME+1)
#define ACC_MIN_LENGTH         (ACC_MAX_LENGTH+1)
#define ACC_HIST_LIMIT         (ACC_MIN_LENGTH+1)
#define ACC_REQUIRED           (ACC_HIST_LIMIT+1)
#define ACC_USER_CHOOSE        (ACC_REQUIRED+1)
#define ACC_GENERATED          (ACC_USER_CHOOSE+1)
#define ACC_RAND_CHARS         (ACC_GENERATED+1)
#define ACC_RAND_LETTERS       (ACC_RAND_CHARS+1)
#define ACC_TRIV_CHECKS        (ACC_RAND_LETTERS+1)
#define ACC_SITE_TRIV_CHECKS   (ACC_TRIV_CHECKS+1)
#define ACC_CMD_AUTHS          (ACC_SITE_TRIV_CHECKS+1)
#define ACC_BASE_PRIVS         (ACC_CMD_AUTHS+1)
#define ACC_KERNEL_AUTHS       (ACC_BASE_PRIVS+1)
#define ACC_CLEARANCE_TX       (ACC_KERNEL_AUTHS+1)
#define ACC_CLEARANCE_PB       (ACC_CLEARANCE_TX+1)
#define ACC_SEC_GROUPS         (ACC_CLEARANCE_PB+1)
#define ACC_TEMPLATE_PDM       (ACC_SEC_GROUPS+1)
#define ACC_RETIRE_DISABLE     (ACC_TEMPLATE_PDM+1)
#define ACC_RETIRE_REM_FILES   (ACC_RETIRE_DISABLE+1)
#define ACC_APPLY_PB           (ACC_RETIRE_REM_FILES+1)
#define ACC_OK_PB              (ACC_APPLY_PB+1)
#define ACC_CANCEL_PB          (ACC_OK_PB+1)
#define ACC_HELP_PB            (ACC_CANCEL_PB+1)
#define ACC_AUD_NAME           (ACC_HELP_PB+1)
#define ACC_AUD_BASE_EVENTS    (ACC_AUD_NAME+1)
#define ACC_AUD_ALIAS_EVENTS   (ACC_AUD_BASE_EVENTS+1)
#define ACC_AUD_AND_MASK       (ACC_AUD_ALIAS_EVENTS+1)
#define ACC_AUD_OR_MASK        (ACC_AUD_AND_MASK+1)
#define ACC_AUD_USER_MASK      (ACC_AUD_OR_MASK+1)
#define ACC_AUD_OFF_MASK       (ACC_AUD_USER_MASK+1)
#define ACC_AUD_SELECT_NONE_PB (ACC_AUD_OFF_MASK+1)
#define ACC_AUD_SELECT_ALL_PB  (ACC_AUD_SELECT_NONE_PB+1)
#define ACC_AUD_OK_PB          (ACC_AUD_SELECT_ALL_PB+1)
#define ACC_AUD_APPLY_PB       (ACC_AUD_OK_PB+1)
#define ACC_AUD_CANCEL_PB      (ACC_AUD_APPLY_PB+1)
#define ACC_AUD_HELP_PB        (ACC_AUD_CANCEL_PB+1)

#define ACC_NUM_PASSWD_FIELDS   ACC_LAST_PASSWD_FIELD

#define UID_SIZE       32                           /* characters, not bits */

#define SET_TEXT_FROM_NUMBER(b, v, w) \
        sprintf((b), "%d", (v)); \
        XmTextFieldSetString((w), (b));

#define SET_SCALE_TO_MINIMUM(w, m, a) \
        XtSetArg(a[0], XmNminimum, &m); \
        XtGetValues(w,a,1); \
        XmScaleSetValue(w,m);

#define ASSIGN_TB_STATE(w, v) \
        if (XmToggleButtonGetState(w)) \
           v = 1; \
        else \
           v = 0;

#define ISSO_MODIFICATION          1
#define USER_MODIFICATION          2
#define TEMPLATE_MODIFICATION      3
#define USER_AND_ISSO_MODIFICATION 4

#define GETMASKBIT(d,i)         (((d) & (1<<(i))) == (1<<(i)))
#define ADDMASKBIT(d,i)         ((d) |= (1<<(i)))

#define NICE_MIN        -19
#define NICE_MAX        19
#define MAX_TRIES_MIN   0
#define MAX_TRIES_MAX   999

#define ACC_MAX_TRIES_FLAG 1
#define ACC_NICE_FLAG      2
#define ACC_LOCKED_FLAG    4
#define ACC_ALL_DATA_FLAGS (ACC_MAX_TRIES_FLAG|ACC_NICE_FLAG|ACC_LOCKED_FLAG)
 
/* account_type Flag Distinguish between User and Template function calls */
                                   /* User Selected */
#define ACC_USERS               1
                                   /* Template Selected */
#define ACC_TEMPLATES           2
                                   /* Assign Template to Account Data */
#define ACC_TEMPLATE_ASSIGNMENT 3

#define SYSTEM_DEFAULT "system_default"

/* UIL Widget hierarchy parents for Accounts and Templates */
#define ACCOUNT_WIDGET "AccountModifyMW"
#define ACCTMPLT_WIDGET "AccTmpltModifyMW"

#define MAX_UNAME_LEN AUTH_MAX_UNAME_SIZE
#define MAX_SHELL_LEN (100+1)

/* defines to trap the progress of account template creation */
#define MOD_ETC_PASSWD 0
#define MOD_ETC_GROUP  1
#define MOD_PR_PASSWD  2
#define MOD_HOME_DIR   3

#define NUM_MOD_STATES 4

/* Database locations for account management */
#define INVALID_GROUP_ID  -100
#define PASSWD_PAG        "/etc/passwd.pag"
#define PASSWD_DIR        "/etc/passwd.dir"
#define PW_ID_MAP         "/etc/auth/system/pw_id_map"

typedef struct _AccountValues_ {
    char           username[MAX_UNAME_LEN];
    char           tmpltname[MAX_UNAME_LEN];
    int            uid;
    time_t         expdate;
    char           tod[AUTH_TOD_SIZE];
    ushort         maxTries;
    int            nice;
    char           locked;
    privvec_t      bprivs;
    privvec_t      sprivs;
    mask_t         cprivs[AUTH_CPRIVVEC_SIZE];
#ifdef SEC_MAC
    mand_ir_t      clearance; /* Filler follows for compatibility with prot.h */
    char           fd_clearance_filler[200]; /* MUST follow fd_clearance */
#endif /* SEC_MAC */
    int            fieldValue[ACC_NUM_PASSWD_FIELDS];
    char           auditdisp[AUTH_MAX_AUDIT_LENGTH];
    uchar_t        auditcntl;   /* audit mask use control */
} AccountValuesType;

typedef struct _AccountData_ {
    int        dataLoaded;
    int        dataIsDefault;
    Widget     apply;
    Widget     apply_pb;
    Widget     ok_pb;
    Widget     cancel_pb;
    Widget     help_pb;
    Widget     usernameText;
    Widget     usernameLBL;
    Widget     tmpltnameText;
    Widget     tmpltcountText;
    Widget     uidText;
    Widget     directory;
    Widget     shell;
    Widget     comments;
    Widget     commentsLBL;
    Widget     priGroups;
    Widget     gidText;
    Widget     secGroups;
    Widget     base_sl;
    Widget     alias_sl;
    int        uid;
    int        ngroups;
    char     **groups;
    XmString  *group_xmstrings;
    Widget     baseList;
    Widget     kernelList;
    int        nprivs;
    char     **privs;
    XmString  *priv_xmstrings;
    Widget     authList;
    int        nauths;
    char     **auths;
    XmString  *auth_xmstrings;
    Widget     clearance_tx;
    Widget     clearance_pb;
    int        ignoreUpdates; /* necessary for LoadUsers compatibility */
    int        fieldValue[ACC_NUM_PASSWD_FIELDS];
    Widget     fields[ACC_NUM_PASSWD_FIELDS];
    Widget     maxTries;
    Widget     nice;
    Widget     locked;
    Widget     expdate_tx;
    time_t     expdate;
    Widget     tod_tx;
    char       tod[AUTH_TOD_SIZE];
    struct sdef_if defaultData;
    pAccSelCommon  userInfo;
    AccountValuesType dflt;
} AccountDataType;

extern void NEW_VUIT_Manage(char *widget_name,Widget *widget,Widget parent);
extern void ForceSelListPolicy(Widget w);
extern void AccClearGraphicObjects(AccountDataType *pAccData);
extern void InitializeGraphicObjects(AccountDataType *pAccData);
extern void InitializeUserDataStructure(AccountDataType *pAccData);
extern void InitializeWidgetIDsToNULL(AccountDataType *pAccData);
extern void LoadUsernameTXList(Widget w, int type);
extern void LoadPriGroupTXList(Widget w);
extern void AccFreeGroupDataStructure(char ***secGroup_names,
                                      int secGroup_count);
extern void AccFreePasswordDataStructure(struct passwd *pwdp);

extern Boolean AccLoadDataFromScreen(AccountDataType *pAccData,
                                     struct passwd *pwdp,struct group *grpp,
                                     char ***names,int *count,int type);

extern void AccEntNameEntered(AccountDataType *pAccData,
                              Widget w,char *name, int type);

extern void AccAssignTemplate(AccountDataType *pAccData,char *template_name);

extern Boolean AccountIsNew(char *username);

extern Boolean TemplateIsNew(char *tmpltname);

extern Boolean AccAssignSmartFlagsForDBSave(AccountDataType *pAccData,int type);

extern void AccCommonDeleteAccount(struct passwd pwd,AccountDataType *pAccData,
                                   Widget tl,int *mod_state);

extern void AccCommonReplaceSystemMenuClose(Widget tl, void (*CB)());
