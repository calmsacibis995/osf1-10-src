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
 * @(#)$RCSfile: xpg4demo.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/03 15:55:59 $
 */

#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <nl_types.h>
#include <limits.h>
#include <time.h>
#include <search.h>

/*
 * Constants.
 */
#define FIRSTNAME_MAX	10	/* Maximum length of first name in chars */
#define SURNAME_MAX	30	/* Maximum length of surname in chars */
#define CC_LEN		3	/* Fixed length of cost center in chars */
#define BADGENUM_MIN	1	/* Minimum value of badge number */ 
#define BADGENUM_MAX	999999	/* Maximum value of badge number */

/*
 * Constants for the record format.  Assumptions are that:
 *
 *	   - Badge number, cost center, and date fields are stored in PCS.
 *	   - Other fields are stored in the locale's codeset. 
 *           (for example, ISO8859-1)
 *	   - Each field in the record is separated by the tab character. 
 *	   - Values in name fields can contain space characters. 
 *	   - Records can contain identical name values, but  
 *	     the badge numbers are unique.
 *	   - Records are separated by the \n character, not by 
 *           the characters <CR><LF>. 
 */
#define REC_BADGE_LEN	(sizeof(char)*6)
				/* Length of badge number field in bytes */
#define REC_FIRSTNM_LEN	(MB_LEN_MAX*FIRSTNAME_MAX)
				/* Length of firstname field in bytes */
#define REC_SURNM_LEN	(MB_LEN_MAX*SURNAME_MAX)
				/* Length of surname field in bytes */
#define REC_CC_LEN	(sizeof(char)*CC_LEN)
				/* Length of cost center field in bytes */
#define REC_JOD_LEN	(sizeof(char)*(2+1+2+1+2))
				/* Length of date field in bytes */
#define REC_LEN		(REC_BADGE_LEN + REC_FIRSTNM_LEN + REC_SURNM_LEN + \
			 REC_CC_LEN + REC_JOD_LEN + sizeof(char)*5)
				/* Total length of record in bytes */

/*
 * Data types.
 */

typedef int Boolean;
typedef enum { FileBad, FileNew, FileReadonly, FileWritable } FileMode;

#if INT_MAX >= BADGENUM_MAX
#define BADGE_FORMAT	"%d"
typedef int BadgeNumber;
#else
#define BADGE_FORMAT	"%ld"
typedef long BadgeNumber;
#endif

typedef struct _EmployeeRec Employee;
typedef struct _EmployeeNodeRec EmployeeNode;

/*
 * Data structure for storing employee information.
 */
struct _EmployeeRec {
	BadgeNumber	badge_num;
	wchar_t		first_name[FIRSTNAME_MAX+1];
	wchar_t		surname[SURNAME_MAX+1];
	wchar_t		cost_center[CC_LEN+1];
	struct tm	date_of_join;
};

/*
 * Data structure for a tree node. This must be consistent with the
 * structure used by the tsearch() family of functions.
 */
struct _EmployeeNodeRec {
	Employee	*employee;
};

/*
 * Generic function/procedure data types.
 */
typedef void		(*VoidProc)(void);
typedef void		(*ActionProc)(const void *node,
				      VISIT visit,
				      int level);

/*
 * Global variables shared by the modules.
 */
extern EmployeeNode	*EmployeeRoot;		/* Root of employee tree */
extern Employee		*CurrentEmployee;	/* Current employee (if any) */
extern nl_catd		MsgCat;			/* Message catalog descriptor */
extern char		*ProgName;		/* Program name */
extern char		*DatabaseName;		/* Database name */
extern Boolean		FullScreenMode;		/* True if in full screen mode */
extern Boolean		SurnameFirst;		/* True if surname comes first */
extern int		NErrors;		/* Error counter */
extern Boolean		ModifiedFlag;		/* True if the employee data is modified */
/*
 * general macros
 */
#ifndef TRUE
#define TRUE	(0==0)
#define FALSE	(!TRUE)
#endif
#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif
#define NUM_ELEMENTS(a)	(sizeof(a) / sizeof(a[0]))

/*
 * tsearch macros.
 */
#define TSearch(key, compar) \
		(EmployeeNode *) tsearch((void *)(key), (void **)&EmployeeRoot, compar)
#define TFind(key, compar) \
		(EmployeeNode *) tfind((void *)(key), (void **)&EmployeeRoot, compar)
#define TDelete(key, compar) \
		(EmployeeNode *) tdelete((void *)(key), (void **)&EmployeeRoot, compar)
#define TWalk(action) \
		twalk((void *)EmployeeRoot, (ActionProc)(action))

/*
 * Messaging macros.
 */
#define GetErrorMsg(id, defmsg)\
			catgets(MsgCat, MSGError, id, defmsg)
#define GetStringMsg(id, defmsg)\
			catgets(MsgCat, MSGString, id, defmsg)
#define GetMsg(id, defmsg)\
			catgets(MsgCat, MSGInfo, id, defmsg)
#define ErasePromptLine()	GotoPromptLine()
#define EraseMessageLine()	GotoMessageLine()

/*
 * Global functions.
 */
extern int ReadDatabase(const char *dbname);
extern Employee *FindRecByBadgeNo(BadgeNumber bn);

extern void ShowEmployee(Employee *emp);
extern void ShowMenu(void);
extern void CommandLoop(void);

extern void GotoPromptLine(void);
extern void GotoMessageLine(void);
extern void GotoCommandLine(void);
extern void InitMessages(void);

/*
 * Functions defined in util.c.
 */
extern int (*CompareNames)(const void *e1, const void *e2);
extern int CompFirstnames(const void *e1, const void *e2);
extern int CompSurnames(const void *e1, const void *e2);
extern int MbsWidth(const char *mbs);
extern char *GetMessage(int set_id, int msg_id, const char *s);

#ifndef MESSAGE_C
extern void Inform(const char *fmt, ...);
extern void Ask(const char *fmt, ...);
extern void Error(const char *fmt, ...);
extern void Warning(const char *fmt, ...);
extern void Fatal(const char *fmt, ...);
#endif
extern void NoMemory(const char *for_what);
extern int Confirm(const char *msg);
