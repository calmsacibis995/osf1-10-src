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
 * @(#)$RCSfile: sec_es_types.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/07/15 18:52:26 $
 */
#ifndef _SEC_ES_TYPES_H_
#define _SEC_ES_TYPES_H_

#define	ES_KERNEL_STARTUP	1	/* boot record */
#define	ES_KERNEL_SHUTDOWN	2	/* shutdown record */
#define	ES_LOGIN_LOCAL		1	/* successful login */
#define	ES_LOGIN_REMOTE		2	/* successful login */
#define	ES_LOGIN_FAILED		3	/* failed login */
#define	ES_LOGOFF		4	/* logoff */
#define ES_AUTOLOG_TIMER	5	/* TPATH: auto-logoff by timer */
#define ES_SESSION		6	/* TPATH: new trusted path session */
#define ES_TPLOGIN		7	/* TPATH: successful tpath user login */
#define ES_TPLOGOFF		8	/* TPATH: trusted path logoff */
#define ES_AUTOLOG_MULTI	9	/* CHOTS: auto-logoff multi login */
#define ES_MULTI_FAILED		10	/* CHOTS: login failed multi login */
#define ES_2P_SUCCESS		11	/* CHOTS: two person check succeeded */
#define ES_2P_FAILED		12	/* CHOTS: two person check failed */
#define ES_LOGIN_MAX		12
#define	ES_PW_CHANGE		1	/* pw change */
#define	ES_PW_CHANGE_FAILED	2	/* pw change failed */
#define	ES_PW_CHANGE_NOPRIV	3	/* no priv for pw change */
#define ES_PW_MAX		3
#define	ES_USER_LOCK		1	/* login failure for locked user */
#define	ES_TERM_LOCK		2	/* login failure for locked terminal*/
#define ES_SET_USER_LOCK	3	/* lock user account */
#define ES_SET_USER_UNLOCK	4	/* unlock user account */
#define ES_SET_TERM_LOCK	5	/* lock terminal */
#define ES_SET_TERM_UNLOCK	6	/* unlock terminal */
#define	ES_TIME_LOCK		7	/* login failure for time lock */
#define ES_LOCK_MAX		7
#define ES_AUD_ENABLE		1	/* audit enabled */
#define	ES_AUD_DISABLE		2	/* disable auditing */
#define	ES_AUD_MODIFY		3	/* subsystem parameter modified */
#define	ES_AUD_REPORT		4	/* audit report generation */
#define	ES_AUD_ARCHIVE		5	/* compacted audit file archival */
#define	ES_AUD_DMN_INIT		6	/* compaction daemon start */
#define	ES_AUD_DMN_END		7	/* compaction daemon termination */
#define	ES_AUD_SHUTDOWN		8	/* audit shutdown notification */
#define	ES_AUD_ERROR		9	/* audit error condition */
#define ES_AUD_MAX		9
#define	ES_DB_INTEGRITY		1	/* Auth database integrity */
#define	ES_FS_INTEGRITY		2	/* File system discr integrity */
#define	ES_DB_RESOURCE		3	/* Auth database resource denial */
#define	ES_SEC_FAILURE		4	/* Security action failed in auth DB */
#define ES_DB_MAX		4
#define	ES_WS_DISPLAY_OPEN	1	/* display connection opened */
#define ES_WS_DISPLAY_CLOSED	2	/* display connection closed */
#define ES_WS_WINDOW_MANAGE	3	/* manage a client window */
#define ES_WS_WINDOW_UNMANAGE	4	/* unmanage a client window */
#define ES_WS_TP_MANAGE		5	/* manage trusted path client window */
#define ES_WS_IL_CHANGE		6	/* window information label change */
#define ES_WS_IL_FLOAT		7	/* window information label float */
#define ES_WS_IIL_CHANGE	8	/* window input information label chg */
#define ES_WS_SL_CHANGE		9	/* window input information label chg */
#define ES_WS_RUN_LEVEL_CHANGE	10	/* trusted path run level change */
#define ES_WS_CUT_PASTE		11	/* successful data paste */
#define ES_WS_CUT_PASTE_ERROR	12	/* setup error on cut paste */
#define ES_WS_FILE_IL_CHANGE	13	/* file information label change */
#define ES_WS_FILE_IL_FAILED	14	/* file information label error */
#define ES_WS_FILE_SL_CHANGE	15	/* file sensitivity label change */
#define ES_WS_FILE_SL_FAILED	16	/* file sensitivity label error */
#define ES_WS_FILE_BOTH_CHANGE	17	/* file il/sl label change */
#define ES_WS_FILE_BOTH_FAILED	18	/* file il/sl label error */
#define	ES_WS_ROLE_ASSUMED	19	/* enter an administrative role */
#define	ES_WS_ROLE_ENDED	20	/* leave an administrative role */
#define ES_WS_SL_DOWNGRADE	21	/* SL downgrade needed for paste */
#define ES_WS_IL_DOWNGRADE	22	/* window IL downgrade */
#define ES_WS_PW_CHANGE		23	/* sucessful password change */
#define ES_WS_PW_CHANGE_FAILED	24	/* unsuccessful password change */
#define ES_WS_PW_NO_AUTH	25	/* no authorization to change pw */
#define ES_WS_CHANGE_CLRNCE	26	/* downgrade session clearance */
#define	ES_WS_ACCESS_DENIED	27	/* server access denial */
#define	ES_WS_INSUFF_PRIV	28	/* server privilege denial */
#define	ES_WS_SERVER_RESET	29	/* server reset */
#define ES_WS_MAX		29
#define ES_MAX_CODE	ES_WS_MAX	/* based on WINDOW_SYSTEM group */

#endif
