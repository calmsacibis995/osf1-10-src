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
 * @(#)$RCSfile: hyperhelp.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/12/20 21:30:48 $
 */

#define ISSO_OVERVIEW		0
#define KEYBOARD		ISSO_OVERVIEW+1
#define HELP			KEYBOARD+1
#define MOD_USER_BOX		HELP+1
#define MOD_TEMP_BOX		MOD_USER_BOX+1
#define AD_OVERVIEW		MOD_TEMP_BOX+1
#define CREATE_ACC_BOX		AD_OVERVIEW+1
#define CREATE_GROUP_BOX	CREATE_ACC_BOX+1
#define RETIRE_DIALOG_BOX 	CREATE_GROUP_BOX+1
#define SELECT_DEV_BOX		RETIRE_DIALOG_BOX+1
#define MOD_PRINT_BOX		SELECT_DEV_BOX+1
#define ADD_TERM_BOX		MOD_PRINT_BOX+1
#define ADD_REMOVE_BOX		ADD_TERM_BOX+1
#define MOD_REMOVE_BOX		ADD_REMOVE_BOX+1
#define MOD_TERM_BOX		MOD_REMOVE_BOX+1
#define DEV_DEFAULTS_BOX	MOD_TERM_BOX+1
#define AUDMODSEL1_DIALOG_BOX	DEV_DEFAULTS_BOX+1
#define AUDMODSEL2_DIALOG_BOX	AUDMODSEL1_DIALOG_BOX+1
#define AUDIT_FIELD_BOX		AUDMODSEL2_DIALOG_BOX+1
#define AUDGENREP_DIALOG_BOX	AUDIT_FIELD_BOX+1
#define AUDGENREP_REPORT_BOX	AUDGENREP_DIALOG_BOX+1
#define AUDGENREP_CHANGELOG_BOX	AUDGENREP_REPORT_BOX+1
#define AUDGENREP_STATUS_BOX	AUDGENREP_CHANGELOG_BOX+1
#define AUDGENREP_NEWDIR_BOX	AUDGENREP_STATUS_BOX+1
#define AUDDESEL_HELP		AUDGENREP_NEWDIR_BOX+1
