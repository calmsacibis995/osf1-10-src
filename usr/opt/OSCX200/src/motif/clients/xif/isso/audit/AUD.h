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
 * @(#)$RCSfile: AUD.h,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/12/20 21:31:35 $
 */

#ifndef _AUD_H_
#define _AUD_H_

/*
 *
 * AUD.h
 *
 * This header file contains all constants and structure definitions
 * used by the Audit User Interface code.
 *
 * Anthony C. Salvo
 *
 */

#include <sys/audit.h>

#define min(a,b) (((a) <= (b)) ? (a) : (b))
#define max(a,b) (((a) >= (b)) ? (a) : (b))

#define AUDSuccess (int) 0

/* Routine Declarations */

/* AUDUtils.c */
extern int 	AUDGetFileList();
extern int  	AUDGetBaseEventList();
extern int  	AUDGetSiteEventList();
extern int  	AUDGetAliasEventList();
extern Boolean	AUDIsAlias();
extern char	*StripWhiteSpace();
extern Boolean  AUDIsAuditLog();

/* Global Data Structures */
typedef void (*FuncPtr)();

typedef struct
{
    char *eventName;
    int  success;
    int	 failure;
    int	 isAlias;
    char *subEvent;
} AUDevent, *AUDeventPtr;    

typedef struct
{
    char	**baseEventList;
    int		baseEventCount;
    int		baseEventSerialNo;
    char	**siteEventList;
    int		siteEventCount;
    int		siteEventSerialNo;
    char	**aliasEventList;
    int		aliasEventCount;
    int		aliasEventSerialNo;
    char	**selectionFiles;
    int		numSelectionFiles;
    int		selectionFileSerialNo;
    char	**deselectionFiles;
    int		numDeselectionFiles;
    int		deselectionFileSerialNo;
    char	**auditLogDirList;
    int		auditLogDirListCount;
    int		auditLogDirListSerialNo;
    char	**remoteHostList;
    int		remoteHostListCount;
    int		remoteHostListSerialNo;
} AUDGlobalData, *AUDGlobalDataPtr;

typedef struct
{
    int	baseEvents;
    int	siteEvents;
    int	aliasEvents;
    int	selectionFiles;
    int	deselectionFiles;
    int	auditLogDirs;
    int remoteHostList;
} AUDSerialData, *AUDSerialDataPtr;

/* typedef for structure when user wants to cancel from modified screen */
typedef struct
{
    Widget	w;
    int		flag;
    void	(*function)();
} AUDLoseChangeData, *AUDLoseChangeDataPtr;


/* Common Variables */
extern AUDGlobalData	audglobal;
extern AUDSerialData	audmasterSerials;

/* Program names */
#define AUDIT_DAEMON	  "/usr/sbin/auditd"
#define AUDIT_MASK	  "/usr/sbin/auditmask"
#define AUDIT_TOOL	  "/usr/sbin/audit_tool"

/* Audit Subsystem Data Files */
#define AUDIT_EVENTS	  "/etc/sec/audit_events"
#define AUDIT_DIRLIST     "/etc/sec/auditd_loc"
#define AUDIT_ALIASES	  "/etc/sec/event_aliases"
#define AUDIT_SITE_EVENTS "/etc/sec/site_events"
#define AUDIT_HOSTLIST	  "/etc/sec/auditd_clients"
#define AUDIT_REPORT_NAME "report"

/* User Interface Data Files */
#define AUDIT_EVENT_LIST  "/var/tcb/audit/base_events"
#define AUDIT_SELDIR	  "/var/tcb/audit/selection"
#define AUDIT_DESELDIR    "/var/tcb/audit/deselection"
#define AUDIT_LOGDIR	  "/var/audit"
#define AUDIT_STARTPROF   "/var/tcb/audit/start_profile"
#define AUDIT_SYSMASK	  "/var/tcb/audit/sysmask"

/* Common Constants */
#define AUDIT_SUCCESS	  "succeed"
#define AUDIT_FAILURE	  "fail"
#define AUDIT_PREFIX	  "old"


/* Report Generation Specific Data */

#define AUDREP_STARTTIME_OPT	"-t"
#define AUDREP_STOPTIME_OPT	"-T"
#define AUDREP_HOSTNAME_OPT	"-h"
#define AUDREP_AUID_OPT		"-a"
#define AUDREP_RUID_OPT		"-r"
#define AUDREP_EUID_OPT		"-u"
#define AUDREP_USERNAME_OPT	"-U"
#define AUDREP_EVENT_OPT	"-e"
#define AUDREP_PID_OPT		"-p"
#define AUDREP_PPID_OPT		"-P"
#define AUDREP_DEVNO_OPT	"-x"
#define AUDREP_VNODEID_OPT	"-v"
#define AUDREP_VNODENO_OPT	"-V"
#define AUDREP_STRING_OPT	"-s"
#define AUDREP_ERROR_OPT	"-E"
#define AUDREP_PROCNAME_OPT	"-y"

#ifdef SECURITY_B1
#define AUDREP_SL_RANGE_OPT	"-L"
#define AUDREP_PRIV_OPT		"-k"
#ifdef SECURITY_CMW
#define AUDREP_IL_RANGE_OPT	"-I"
#endif /* SECURITY_CMW */
#endif /* SECURITY_B1 */

#define AUDREP_STARTTIME_CONST	't'
#define AUDREP_STOPTIME_CONST	'T'
#define AUDREP_HOSTNAME_CONST	'h'
#define AUDREP_AUID_CONST	'a'
#define AUDREP_RUID_CONST	'r'
#define AUDREP_EUID_CONST	'u'
#define AUDREP_USERNAME_CONST	'U'
#define AUDREP_EVENT_CONST	'e'
#define AUDREP_PID_CONST	'p'
#define AUDREP_PPID_CONST	'P'
#define AUDREP_DEVNO_CONST	'x'
#define AUDREP_VNODEID_CONST	'v'
#define AUDREP_VNODENO_CONST	'V'
#define AUDREP_STRING_CONST	's'
#define AUDREP_ERROR_CONST	'E'
#define AUDREP_PROCNAME_CONST	'y'

#ifdef SECURITY_B1
#define AUDREP_SL_RANGE_CONST	'L'
#define AUDREP_PRIV_CONST	'k'
#ifdef SECURITY_CMW
#define AUDREP_IL_RANGE_CONST  	'I'
#endif /* SECURITY_CMW */
#endif /* SECURITY_B1 */

/* Report Control Options */
#define AUDREP_BINARY_COPT	"-b"
#define AUDREP_BRIEF_COPT	"-B"
#define AUDREP_BOTH_COPT	"-"
#define AUDREP_DESEL_COPT	"-d"
#define AUDREP_PRDESEL_COPT	"-D"
#define AUDREP_FOLLOW_COPT	"-f"
#define AUDREP_FAST_COPT	"-F"
#define AUDREP_INTERACTIVE_COPT "-i"
#define AUDREP_OVERRIDE_COPT	"-o"
#define AUDREP_AUIDREP_COPT	"-R"
#define AUDREP_SORT_COPT	"-S"
#define AUDREP_MAPIDS_COPT	"-w"
#define AUDREP_STATS_COPT	"-Z"

/*-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-*/
/*									   */
/*  	M  O  D  I  F  Y     S  E  L  E  C  T  I  O  N    F  I  L  E  S    */
/*									   */
/*-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-*/


/* Modify Selection UI */

#define AUDIT_MAX_ENTRIES	8

typedef struct _AUDModSelUI_
{
    Widget	form1_W;
    Widget	selectionFileTL_W;
    Widget	startTimeTF_W;
    Widget	stopTimeTF_W;
    Widget	pidTF_W;
    Widget	ppidTF_W;
    Widget	auidTF_W;
    Widget	ruidTF_W;
    Widget	euidTF_W;
    Widget	userNameTF_W;
    Widget	hostNameTF_W;
    Widget	baseEventSB_W;	
    Widget	aliasEventSB_W;
    Widget	allEventsPB_W;
    Widget	noEventsPB_W;
    Widget	moreSelPB_W;
    Widget	deleteSelPB_W;
    Widget      OK1;
    Widget      Apply1;
    Widget      Cancel1;
    Widget      Help1;
    Widget	form2_W;
    Widget	selNameLBL_W;
    Widget	devNoTF_W;
    Widget	vnodeIDTF_W;
    Widget	vnodeNoTF_W;
    Widget	procNameTF_W;
    Widget	stringTF_W;
    Widget	errorTF_W;
    Widget      OK2;
    Widget      Cancel2;
    Widget      Help2;
}  AUDModSelUI, *AUDModSelUIPtr;


typedef struct
{
    unsigned int	mask;
    char		startTime[13];
    char		stopTime[13];
    char		*hostName[AUDIT_MAX_ENTRIES];
    char		*auid[AUDIT_MAX_ENTRIES];
    char		*ruid[AUDIT_MAX_ENTRIES];
    char		*euid[AUDIT_MAX_ENTRIES];
    char		*userName[AUDIT_MAX_ENTRIES];
    char		*pid[AUDIT_MAX_ENTRIES];
    char		*ppid[AUDIT_MAX_ENTRIES];
    char		*devNo[AUDIT_MAX_ENTRIES];
    char		*vnodeID[AUDIT_MAX_ENTRIES];
    char		*vnodeNo[AUDIT_MAX_ENTRIES];
    char		*string[AUDIT_MAX_ENTRIES];
    char		*error[AUDIT_MAX_ENTRIES];
    char		*procName[AUDIT_MAX_ENTRIES];
#ifdef SECURITY_B1
    char		*sl_range[AUDIT_MAX_ENTRIES];
    char		*priv[AUDIT_MAX_ENTRIES];
#ifdef SECURITY_CMW
    char		*il_range[AUDIT_MAX_ENTRIES];
#endif /* SECURITY_CMW */
#endif /* SECURITY_B1 */
    int			allEvents;
    int			numEvents;
    AUDeventPtr		*events;
} AUDModSelData, *AUDModSelDataPtr;


typedef struct _AUDModSel_ 
{
    AUDModSelUI		ui;
    AUDModSelData	data;
}  AUDModSel, *AUDModSelPtr;

/* Widget ID's */
#define AUDMODSEL_STARTTIME	0
#define AUDMODSEL_STOPTIME	1
#define AUDMODSEL_SELFILELIST	3
#define AUDMODSEL_PID		4
#define AUDMODSEL_PPID		5
#define AUDMODSEL_AUID		6
#define AUDMODSEL_RUID		7
#define AUDMODSEL_EUID		8
#define AUDMODSEL_USERNAME	9
#define AUDMODSEL_HOSTNAME	10
#define AUDMODSEL_BASEEVENT	11
#define AUDMODSEL_ALIASEVENT	12
#define AUDMODSEL_ALL_EVENT_PB	13
#define AUDMODSEL_NO_EVENT_PB	14
#define AUDMODSEL_MORE_PB	15
#define AUDMODSEL_DELETE_PB	16
#define AUDMODSEL_OK1		17
#define AUDMODSEL_APPLY1	18
#define AUDMODSEL_CANCEL1	19
#define AUDMODSEL_HELP1		20
#define AUDMODSEL_SELLABEL	21
#define AUDMODSEL_DEVNO		22
#define AUDMODSEL_VNODEID	23
#define AUDMODSEL_VNODENO	24
#define AUDMODSEL_PROCNAME	25
#define AUDMODSEL_STRING	26
#define AUDMODSEL_ERROR		27
#define AUDMODSEL_OK2		28
#define AUDMODSEL_CANCEL2	29
#define AUDMODSEL_HELP2		30
#define AUDMODSEL_EVENT		31


/* Masks - only for Widgets with values written to file */
#define AUDMODSEL_STARTTIME_MASK	(1 << AUDMODSEL_STARTTIME)
#define AUDMODSEL_STOPTIME_MASK		(1 << AUDMODSEL_STOPTIME)
#define AUDMODSEL_PID_MASK		(1 << AUDMODSEL_PID)
#define AUDMODSEL_PPID_MASK		(1 << AUDMODSEL_PPID)
#define AUDMODSEL_AUID_MASK		(1 << AUDMODSEL_AUID)
#define AUDMODSEL_RUID_MASK		(1 << AUDMODSEL_RUID)
#define AUDMODSEL_EUID_MASK		(1 << AUDMODSEL_EUID)
#define AUDMODSEL_USERNAME_MASK		(1 << AUDMODSEL_USERNAME)
#define AUDMODSEL_HOSTNAME_MASK		(1 << AUDMODSEL_HOSTNAME)
#define AUDMODSEL_DEVNO_MASK		(1 << AUDMODSEL_DEVNO)
#define AUDMODSEL_VNODEID_MASK		(1 << AUDMODSEL_VNODEID)
#define AUDMODSEL_VNODENO_MASK		(1 << AUDMODSEL_VNODENO)
#define AUDMODSEL_PROCNAME_MASK		(1 << AUDMODSEL_PROCNAME)
#define AUDMODSEL_STRING_MASK		(1 << AUDMODSEL_STRING)
#define AUDMODSEL_ERROR_MASK		(1 << AUDMODSEL_ERROR)
#define AUDMODSEL_EVENT_MASK		(1 << AUDMODSEL_EVENT)

/*-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-*/
/*									   */
/*  	      G  E  N  E  R  A  T  E    R  E  P  O  R  T  S                */
/*									   */
/*-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-*/

/* data buffer management declarations */

/* how many records can fit in a page? 
 * Note: This number is an estimate base on a 256K page.  The worst case
 * would be that each brief record would be 1 byte followed by the \0
 * then the full record would be 1 byte followed by the \0.  This would
 * never happen, but essentially 1 record would occupy 4 bytes.  This means
 * that 256K / 4 = 64K records could be present.  This is ludicrous, so
 * 8K is used as an approximation.  
 */
#define MAXRECS  	      8192
#define AUD_TIME_SLICE	      1000
#define AUDIT_BRIEF_POOL         0 /* split page 96K for brief, the rest for */
#define AUDIT_FULL_POOL      98304 /* full records */
#define AUDIT_PAGE_SIZE (systemPageSize * 32)

typedef struct
{
    int		numRecs;		 /* how many records? */
    int		briefPos;
    int		fullPos;		
    int		briefCursorPos[MAXRECS]; /* cursor locations of brief records*/
    char *	fullMemoryPos[MAXRECS];  /* memory locations of full records */
} AUDRecordMap, *AUDRecordMapPtr;


typedef struct _AUDPage
{
    int			number;		/* page number */
    char		*data;		/* actual data */
    Boolean		full;	 	/* amount "free" in buffer */
    Boolean		isOnScreen;	/* is this page on display? */
    struct _AUDPage	*next;		/* forward pointer */
    struct _AUDPage	*prev;		/* backward pointer */
    AUDRecordMapPtr	map;		/* map of full/brief */
} AUDPage, *AUDPagePtr;

typedef struct
{
    XtAppContext        app_context;
    Widget		w;
    XtIntervalId	timerID;
    int			fd1, fd2, fd3;
    pid_t		childpid;
    fd_set 		readmask;
    struct timeval 	timeout;
    int			listen;
    char                *buf;
    int                 nbytes;
    int			eof;
} WorkData, *WorkDataPtr;

/* user interface structures */
typedef struct _AUDGenRepUI_ 
{
    /* GENERATE REPORTS Screen */
    Widget	form_W;
    Widget	auditLogLST_W;
    Widget	selectionFileLST_W;
    Widget	deselectionFileLST_W;
    Widget      displayNowTB_W;
    Widget      displayCurrentTB_W;
    Widget	saveToFileTB_W;
    Widget	reportFilesTB_W;
    Widget	fileNameTF_W;
    Widget	fileNameLBL_W;
    Widget      followChangeTB_W;
    Widget      translateIDsTB_W;
    Widget      displayStatsTB_W;
    Widget	printRulesetTB_W;
    Widget      fullRecTB_W;
    Widget      briefRecTB_W;
    Widget	changeDirPB_W;
    Widget      OKPB_W;
    Widget      ApplyPB_W;
    Widget      CancelPB_W;
    Widget      HelpPB_W;
    Widget      auditLogDateLBL_W;
    Widget      auditLogDateTF_W;
    Widget      auditLogSizeLBL_W;
    Widget      auditLogSizeTF_W;
    Widget	auditLogLBL_W;

    /* CHANGE LOG DIR Screen */
    Widget	changeLogDirFRMD_W;
    Widget	changeLogDirTF_W;
    Widget	changeLogDirOKPB_W;
    Widget	changeLogDirCancelPB_W;
    Widget	changeLogDirHelpPB_W;

    /* ACTUAL REPORT Screen */
    Widget	actualReportFRM_W;
    Widget	actualReportTXT_W;
    Widget	actualReportTitleTXT_W;
    Widget	actualReportSCALE_W;
    Widget	actualReportCancelPB_W;
    Widget	actualReportHelpPB_W;

    /* STATUS WINDOW Screen */
    Widget	statusWindowFRM_W;
    Widget	statusWindowTXT_W;
    Widget	statusWindowClearPB_W;
    Widget	statusWindowDismissPB_W;
    Widget	statusWindowHelpPB_W;
    
    /* ASK FOR LOGDIR Screen */
    Widget	askNewDirFRMD_W;
    Widget	askNewDirTF_W;
    Widget	askNewDirOKPB_W;
    Widget	askNewDirCancelPB_W;
    Widget	askNewDirHelpPB_W;

    /* Full Audit Record Screen */
    Widget	fullRecFRMD_W;
    Widget	fullRecTXT_W;
    Widget	fullRecCancelPB_W;
}  AUDGenRepUI, *AUDGenRepUIPtr;



typedef struct
{
    /* logs, selection files, deselection files are in the global
     * data structure.
     */
    char		*fileName;
    Boolean             brief;
} AUDGenRepData, *AUDGenRepDataPtr;


typedef struct _AUDGenRep_ 
{
    AUDGenRepUI		ui;
    AUDGenRepData	data;
}  AUDGenRep, *AUDGenRepPtr;

/* Widget ID's */
#define AUDGENREP_LOGS		   0
#define AUDGENREP_SELFILES	   1
#define AUDGENREP_DESELFILES	   2
#define AUDGENREP_DISPNOW	   3
#define AUDGENREP_DISPCUR	   4
#define AUDGENREP_SAVETOFILE	   5
#define AUDGENREP_REPORTS	   6
#define AUDGENREP_FILENAME	   7
#define AUDGENREP_FILENAME_LBL	   8
#define AUDGENREP_FOLLOWCHANGE	   9
#define AUDGENREP_TRANSLATEIDS	  10
#define AUDGENREP_DISPSTATS	  11
#define AUDGENREP_PRINTRULES	  12
#define AUDGENREP_FULLREC	  13
#define AUDGENREP_BRIEFREC	  14
#define AUDGENREP_CHANGEDIR	  15
#define AUDGENREP_OK		  16
#define AUDGENREP_APPLY		  17
#define AUDGENREP_CANCEL	  18
#define AUDGENREP_HELP		  19

#define AUDGENREP_CLD_FORM	  20
#define AUDGENREP_CLD_TEXT        21
#define AUDGENREP_CLD_OK 	  22
#define AUDGENREP_CLD_CANCEL      23
#define AUDGENREP_CLD_HELP        24

#define AUDGENREP_AR_FORM	  25
#define AUDGENREP_AR_TEXT	  26
#define AUDGENREP_AR_SCALE	  27
#define AUDGENREP_AR_CANCEL	  28
#define AUDGENREP_AR_HELP	  29

#define AUDGENREP_SW_FORM	  30
#define AUDGENREP_SW_TEXT	  31
#define AUDGENREP_SW_CLEAR	  32
#define AUDGENREP_SW_DISMISS	  33
#define AUDGENREP_SW_HELP	  34
    
#define AUDGENREP_AND_FORM	  35
#define AUDGENREP_AND_TEXT	  36
#define AUDGENREP_AND_OK	  37
#define AUDGENREP_AND_CANCEL	  38
#define AUDGENREP_AND_HELP	  39

#define AUDGENREP_FAR_FORM	  40
#define AUDGENREP_FAR_TEXT	  41
#define AUDGENREP_FAR_CANCEL   	  42

#define AUDGENREP_AUDLOG_DATE_TEXT 43
#define AUDGENREP_AUDLOG_DATE_LBL  44
#define AUDGENREP_AUDLOG_SIZE_TEXT 45
#define AUDGENREP_AUDLOG_SIZE_LBL  46
#define AUDGENREP_LOGS_LBL	   47

#define AUDGENREP_AR_TITLE	   48


/*-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-*/
/*									   */
/*  M  O  D  I  F  Y     D  E  S  E  L  E  C  T  I  O  N    F  I  L  E  S  */
/*									   */
/*-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-*/

#define AUDDESEL_ALL		"*"
#define AUDDESEL_NUM_TUPLES     6

/* The following is pulled from the audit_tool, so it must be kept in sync */
#define NRULESETS       32
#define RULES_IN_SET    32
#define MAX_RULE_SIZ    4096


/* deselection ruleset, and ruleset ptrs */
typedef struct 
{
    char *host[RULES_IN_SET];
    uid_t auid[RULES_IN_SET];
    uid_t ruid[RULES_IN_SET];
    char *event[RULES_IN_SET];
    char *subevent[RULES_IN_SET];
    char *param[RULES_IN_SET];
    int  oprtn[RULES_IN_SET];
} AUDDeselectionData, *AUDDeselectionDataPtr;

typedef struct _AUDModDeselUI_ 
{
    Widget	form_W;
    Widget	deselectionFileTL_W;
    Widget	deselDispLST_W;
    Widget	hostTF_W;
    Widget	auidTF_W;
    Widget	ruidTF_W;
    Widget	eventTF_W;
    Widget	pathTF_W;
    Widget	flagTF_W;
    Widget	OKPB_W;
    Widget	ApplyPB_W;
    Widget	CancelPB_W;
    Widget	HelpPB_W;
    Widget	deletePB_W;
    Widget	addPB_W;
    Widget	updatePB_W;
    Widget	removePB_W;
}  AUDModDeselUI, *AUDModDeselUIPtr;


typedef struct _AUDDeselData_ 
{
    int			  nrules;
    AUDDeselectionDataPtr rules[NRULESETS];
}  AUDModDeselData, *AUDModDeselDataPtr;

typedef struct _AUDModDesel_ 
{
    AUDModDeselUI	ui;
    AUDModDeselData	data;
}  AUDModDesel, *AUDModDeselPtr;

/* Widget ID's */
#define AUDDESEL_DESEL_LIST	0
#define AUDDESEL_DESEL_DISP	1
#define AUDDESEL_HOST_TEXT	2
#define AUDDESEL_AUID_TEXT	3
#define AUDDESEL_RUID_TEXT	4
#define AUDDESEL_EVENT_TEXT	5
#define AUDDESEL_PATH_TEXT	6
#define AUDDESEL_FLAG_TEXT	7
#define AUDDESEL_OK_PB		8
#define AUDDESEL_APPLY_PB	9
#define AUDDESEL_CANCEL_PB     10
#define AUDDESEL_HELP_PB       11
#define AUDDESEL_DELETE_PB     12
#define AUDDESEL_ADD_PB        13
#define AUDDESEL_UPDATE_PB     14
#define AUDDESEL_REMOVE_PB     15

#endif /* _AUD_H_ */
