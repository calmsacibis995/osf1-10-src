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
 *	@(#)$RCSfile: XAudit.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:04:42 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#if SEC_BASE

#ifndef __XAudit__
#define __XAudit__
/*
    filename:
        XAudit.h
    
    copyright:
        Copyright (c) 1989-1990 SKM, L.P.
        Copyright (c) 1989-1990 SecureWare Inc.
        ALL RIGHTS RESERVED

    
    function:
        include file for screen interface to the audit subsystem
*/

/* Expand necessary #defines */
#ifdef hpux
#define _INCLUDE_XOPEN_SOURCE
#define _XPG2
#endif

/* Common C include files */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <sys/errno.h>
#include <ctype.h>
#include <signal.h>
#include <dirent.h>

/* Security include files */
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>       /* for ADDBIT, RMBIT, ISBITSET */
#include <pwd.h>
#include <grp.h>

/* Bring in .h files which are needed */
#ifndef NAME_MAX
#include <limits.h>
#endif

#ifndef O_RDONLY
#include <sys/file.h>
#endif

/* Bring in subsystem primary include file */
#include "XMain.h"

#define AUDIRWIDTH                  35
#define DATESTRINGSIZE              24
#define DEVICE_NAME_MAX             30
#define DIRCHUNK                    10  /* number of new directories to add */
#define ENTSIZ                      14  /* aud_pfiles.c - size parm file name */
#define LSFILLWIDTH                 73
/* Should really remove this - it is in XMain.h */
#ifndef NGROUPNAME
#define NGROUPNAME                   9
#endif
#define WCLIMIT_LOW               1024
#define WCLIMIT_HIGH              4096

#define YESCHAR                     'Y'
#define NOCHAR                      'N'

#define AUDIT_DAEMON_PROGRAM        "/tcb/bin/auditd"
#define AUDIT_COMMAND               "/tcb/bin/auditcmd"
#define AUDIT_PATH_LEN  (sizeof(AUDIT_LOGDIR) + sizeof(AUDIT_LOG_FILENAME))
#define AUDIT_LOCK_PATH             "/tcb/files/audit/audit_lock"
#define CAT_COMMAND                 "/bin/cat"
#define REDUCE_REPORT_PATH          "/tcb/files/audit/reports/"
#define AUDIT_LOG_DIRECTORY         "/tcb/files/audit/"

#ifndef MAIN
extern
#endif
Widget
    maintenance_disable_push_widget,
    maintenance_enable_push_widget,
    statistics_push_widget;

typedef struct {
    int         nusers;
    char        **users;
    int         ngroups;
    char        **groups;
    char        this;
    char        future;
    int         ndescs;
    struct  audit_init  au;
} AUDIT_USERS_GROUPS_STRUCT;

typedef struct {
    char        filename[ENTSIZ + 1];
    char        **events;
    char        **files;
    int         nfiles;
    char        **users;
    int         nusers;
    char        **groups;
    int         ngroups;
    char        s_hour[3];
    char        s_min[3];
    char        s_month[4];
    char        s_day[3];
    char        s_year[3];
    char        e_hour[3];
    char        e_min[3];
    char        e_month[4];
    char        e_day[3];
    char        e_year[3];
    struct audit_select as;
    int         ndescs;
#ifdef SEC_MAC
    mand_ir_t   *slevel_min;
    mand_ir_t   *slevel_max;
    mand_ir_t   *olevel_min;
    mand_ir_t   *olevel_max;
#endif /* SEC_MAC */
} AUDIT_SELECTION_STRUCT;

#ifdef SEC_MAC
typedef struct {                           
    char        this_session;
    char        future_sessions;
    mand_ir_t   *min_ir_ptr;
    mand_ir_t   *max_ir_ptr;
    struct audit_init au;
} AUDIT_SENSITIVITY_STRUCT;
#endif /* SEC_MAC */

typedef  struct   {    
    char        **mask;
    char        this;       /* this session -- do ioctl */
    char        future;     /* future sessions -- write file */
    int         ndescs;
    struct audit_init  au;
} audmask_fillin;

typedef  struct  {
    char        this;
    char        future;
    int         ndescs;
    char        compacted;
    char        audit_on_startup;
    char        shut_or_panic;
    char        initially_audit_on_startup;
    struct      audit_init  au;
} audparm_fillin;

struct  ls_fillin {
    char    **sessions;
    int     nsessions;
    int     ndescs;
};

#ifndef MAIN
extern
#endif
struct ls_fillin lsfil;

struct  lpfile_fillin {
    char    **files;
    int     nfiles;
};

#ifndef MAIN
extern
#endif
struct lpfile_fillin lpfile_fil;

struct audir_fillin {
    char    **dirs;
    int     ndirs;
    int     ndescs;
    char    *root_dir;
    struct audit_init
            au;
};

#ifndef MAIN
extern
#endif
struct audir_fillin audfil;

extern char
    *Realloc(),
    *Malloc(),
    *Calloc(),
    **alloc_cw_table(),
    **expand_cw_table(),
    *strdup(),
    *strrchr();
    
extern FILE
    *fopen(),
    *new_audit_parms_file(),    /* aud_dirlst.c */
    *old_audit_parms_file(),    /* aud_dirlst.c */
    *open_aud_parms();          /* accounts/XAuthUtils.c */
    
extern void
    CenterForm(),               /* Centers a form */
    CreateThreeButtons(),       /* Creates the OK, CANCEL, HELP buttons */
    HelpDisplayOpen(),          /* ../common/XHelpDisp.c */
    link_new_parms_file(),      /* aud_dirlst.c */
    unlink_new_parms_file(),    /* aud_dirlst.c */
    StartAuditFunctionalUnits(),/* Xaudit.c */
    StopAuditFunctionalUnits(); /* Xaudit.c */
    
extern int
    AuditPermission(),
    errno,
    strcmp(),
    FileDisplayOpen();          /* ../common/XFileDisp.c */
#endif /* __XAudit__ */
#endif /* SEC_BASE */
