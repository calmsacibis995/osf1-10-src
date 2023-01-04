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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/DXm/PwVMS.h,v 1.1.2.2 92/03/31 17:15:44 Russ_Kuhn Exp $ */
/************************************************************************/
/*									*/
/*	Copyright (c) Digital Equipment Corporation, 1990  		*/
/*	All Rights Reserved.  Unpublished rights reserved		*/
/*	under the copyright laws of the United States.			*/
/*									*/
/*	The software contained on this media is proprietary		*/
/*	to and embodies the confidential technology of 			*/
/*	Digital Equipment Corporation.  Possession, use,		*/
/*	duplication or dissemination of the software and		*/
/*	media is authorized only pursuant to a valid written		*/
/*	license from Digital Equipment Corporation.			*/
/*									*/
/*	RESTRICTED RIGHTS LEGEND   Use, duplication, or 		*/
/*	disclosure by the U.S. Government is subject to			*/
/*	restrictions as set forth in Subparagraph (c)(1)(ii)		*/
/*	of DFARS 252.227-7013, or in FAR 52.227-19, as			*/
/*	applicable.							*/
/*									*/
/************************************************************************/
/******************************************************************************/
/*									      */
/*   FACILITY:								      */
/*									      */
/*        Print Widget 							      */
/*									      */
/*   ABSTRACT:								      */
/*									      */
/*	This module contains the combination of 4 VMS-specific files the      */
/*	print widget needs for doing logical name and queue operations.	      */
/*									      */
/*	The four files were created from fscndef.h, lnmdef.h, quidef.h, and   */
/*	sjcdef.h.							      */
/*									      */
/*   AUTHORS:								      */
/*	Will Walker	Winter-1990                                           */
/*									      */
/*   CREATION DATE:     Winter-1990					      */
/*									      */
/*   MODIFICATION HISTORY:						      */
/*									      */
/*	002	Will Walker		18-Apr-1990			      */
/*		Add new copyright notice				      */
/*	001	Will Walker		19-Mar-1990			      */
/*		Create module.						      */
/*									      */
/******************************************************************************/
#ifndef _PWVMS_h
#define _PWVMS_h

/*  DEC/CMS REPLACEMENT HISTORY, Element FSCNDEF.H */
/*  *1    12-APR-1988 11:14:13 GEORGE "Add some .H files" */
/*  DEC/CMS REPLACEMENT HISTORY, Element FSCNDEF.H */
#define FSCN$M_NODE 1
#define FSCN$M_DEVICE 2
#define FSCN$M_ROOT 4
#define FSCN$M_DIRECTORY 8
#define FSCN$M_NAME 16
#define FSCN$M_TYPE 32
#define FSCN$M_VERSION 64
#define FSCN$S_FLDFLAGS 1
#define FSCN$V_NODE 0
#define FSCN$V_DEVICE 1
#define FSCN$V_ROOT 2
#define FSCN$V_DIRECTORY 3
#define FSCN$V_NAME 4
#define FSCN$V_TYPE 5
#define FSCN$V_VERSION 6
#define FSCN$_FILESPEC 1
#define FSCN$_NODE 2
#define FSCN$_DEVICE 3
#define FSCN$_ROOT 4
#define FSCN$_DIRECTORY 5
#define FSCN$_NAME 6
#define FSCN$_TYPE 7
#define FSCN$_VERSION 8
#define FSCN$S_ITEM_LEN 8
#define FSCN$S_FSCNDEF 8
#define FSCN$W_LENGTH 0
#define FSCN$W_ITEM_CODE 2
#define FSCN$L_ADDR 4

/*  DEC/CMS REPLACEMENT HISTORY, Element LNMDEF.H */
/*  *1    12-APR-1988 11:14:20 GEORGE "Add some .H files" */
/*  DEC/CMS REPLACEMENT HISTORY, Element LNMDEF.H */
#define LNM$M_NO_ALIAS 1
#define LNM$M_CONFINE 2
#define LNM$M_CRELOG 4
#define LNM$M_TABLE 8
#define LNM$M_CONCEALED 256
#define LNM$M_TERMINAL 512
#define LNM$M_EXISTS 1024
#define LNM$M_SHAREABLE 65536
#define LNM$M_CREATE_IF 16777216
#define LNM$M_CASE_BLIND 33554432
#define LNM$S_LNMDEF 4
#define LNM$V_NO_ALIAS 0
#define LNM$V_CONFINE 1
#define LNM$V_CRELOG 2
#define LNM$V_TABLE 3
#define LNM$V_CONCEALED 8
#define LNM$V_TERMINAL 9
#define LNM$V_EXISTS 10
#define LNM$V_SHAREABLE 16
#define LNM$V_CREATE_IF 24
#define LNM$V_CASE_BLIND 25
#define LNM$C_TABNAMLEN 31
#define LNM$C_NAMLENGTH 255
#define LNM$C_MAXDEPTH 10
#define LNM$_INDEX 1
#define LNM$_STRING 2
#define LNM$_ATTRIBUTES 3
#define LNM$_TABLE 4
#define LNM$_LENGTH 5
#define LNM$_ACMODE 6
#define LNM$_MAX_INDEX 7
#define LNM$_PARENT 8
#define LNM$_LNMB_ADDR 9
#define LNM$_CHAIN -1

/*  DEC/CMS REPLACEMENT HISTORY, Element QUIDEF.H */
/*  *1    12-APR-1988 11:14:20 GEORGE "Add some .H files" */
/*  DEC/CMS REPLACEMENT HISTORY, Element QUIDEF.H */
#define QUI$_CANCEL_OPERATION 1
#define QUI$_DISPLAY_CHARACTERISTIC 2
#define QUI$_DISPLAY_FILE 3
#define QUI$_DISPLAY_FORM 4
#define QUI$_DISPLAY_JOB 5
#define QUI$_DISPLAY_QUEUE 6
#define QUI$_TRANSLATE_QUEUE 7
#define QUI$_RESERVED_FUNC_1 8
#define QUI$_RESERVED_FUNC_2 9
#define QUI$_ACCOUNT_NAME 1
#define QUI$_AFTER_TIME 2
#define QUI$_ASSIGNED_QUEUE_NAME 3
#define QUI$_BASE_PRIORITY 4
#define QUI$_CHARACTERISTIC_NAME 5
#define QUI$_CHARACTERISTIC_NUMBER 6
#define QUI$_CHARACTERISTICS 7
#define QUI$_CHECKPOINT_DATA 8
#define QUI$_CLI 9
#define QUI$_COMPLETED_BLOCKS 10
#define QUI$_CONDITION_VECTOR 11
#define QUI$_CPU_DEFAULT 12
#define QUI$_CPU_LIMIT 13
#define QUI$_DEVICE_NAME 14
#define QUI$_ENTRY_NUMBER 15
#define QUI$_FILE_COPIES 16
#define QUI$_FILE_COPIES_CHKPT 17
#define QUI$_FILE_COPIES_DONE 18
#define QUI$_FILE_FLAGS 19
#define QUI$_FILE_SETUP_MODULES 20
#define QUI$_FILE_SPECIFICATION 21
#define QUI$_FILE_STATUS 22
#define QUI$_FIRST_PAGE 23
#define QUI$_FORM_DESCRIPTION 24
#define QUI$_FORM_FLAGS 25
#define QUI$_FORM_LENGTH 26
#define QUI$_FORM_MARGIN_BOTTOM 27
#define QUI$_FORM_MARGIN_LEFT 28
#define QUI$_FORM_MARGIN_RIGHT 29
#define QUI$_FORM_MARGIN_TOP 30
#define QUI$_FORM_NAME 31
#define QUI$_FORM_NUMBER 32
#define QUI$_FORM_SETUP_MODULES 33
#define QUI$_FORM_STOCK 34
#define QUI$_FORM_WIDTH 35
#define QUI$_GENERIC_TARGET 36
#define QUI$_INTERVENING_BLOCKS 37
#define QUI$_INTERVENING_JOBS 38
#define QUI$_JOB_COPIES 39
#define QUI$_JOB_COPIES_CHKPT 40
#define QUI$_JOB_COPIES_DONE 41
#define QUI$_JOB_FLAGS 42
#define QUI$_JOB_LIMIT 43
#define QUI$_JOB_NAME 44
#define QUI$_JOB_RESET_MODULES 45
#define QUI$_JOB_SIZE 46
#define QUI$_JOB_SIZE_MAXIMUM 47
#define QUI$_JOB_SIZE_MINIMUM 48
#define QUI$_JOB_STATUS 49
#define QUI$_LAST_PAGE 50
#define QUI$_LIBRARY_SPECIFICATION 51
#define QUI$_LOG_QUEUE 52
#define QUI$_LOG_SPECIFICATION 53
#define QUI$_NOTE 54
#define QUI$_OPERATOR_REQUEST 55
#define QUI$_OWNER_UIC 56
#define QUI$_PAGE_SETUP_MODULES 57
#define QUI$_PARAMETER_1 58
#define QUI$_PARAMETER_2 59
#define QUI$_PARAMETER_3 60
#define QUI$_PARAMETER_4 61
#define QUI$_PARAMETER_5 62
#define QUI$_PARAMETER_6 63
#define QUI$_PARAMETER_7 64
#define QUI$_PARAMETER_8 65
#define QUI$_PRIORITY 66
#define QUI$_PROCESSOR 67
#define QUI$_PROTECTION 68
#define QUI$_QUEUE_FLAGS 69
#define QUI$_QUEUE_NAME 70
#define QUI$_QUEUE_STATUS 71
#define QUI$_REFUSAL_REASON 72
#define QUI$_REQUEUE_PRIORITY 73
#define QUI$_REQUEUE_QUEUE_NAME 74
#define QUI$_SCSNODE_NAME 75
#define QUI$_SEARCH_FLAGS 76
#define QUI$_SEARCH_NAME 77
#define QUI$_SEARCH_NUMBER 78
#define QUI$_SUBMISSION_TIME 79
#define QUI$_UIC 80
#define QUI$_USERNAME 81
#define QUI$_WSDEFAULT 82
#define QUI$_WSEXTENT 83
#define QUI$_WSQUOTA 84
#define QUI$_RESERVED_BOOLEAN_1 85
#define QUI$_RESERVED_BOOLEAN_2 86
#define QUI$_RESERVED_INPUT_1 87
#define QUI$_RESERVED_INPUT_2 88
#define QUI$_DEFAULT_FORM_NAME 89
#define QUI$_DEFAULT_FORM_NUMBER 90
#define QUI$_DEFAULT_FORM_STOCK 91
#define QUI$_JOB_PID 92
#define QUI$_RESERVED_OUTPUT_5 93
#define QUI$_RESERVED_OUTPUT_6 94
#define QUI$M_FILE_BURST 1
#define QUI$M_FILE_BURST_EXP 2
#define QUI$M_FILE_DELETE 4
#define QUI$M_FILE_DOUBLE_SPACE 8
#define QUI$M_FILE_FLAG 16
#define QUI$M_FILE_FLAG_EXP 32
#define QUI$M_FILE_TRAILER 64
#define QUI$M_FILE_TRAILER_EXP 128
#define QUI$M_FILE_PAGE_HEADER 256
#define QUI$M_FILE_PAGINATE 512
#define QUI$M_FILE_PASSALL 1024
#define QUI$M_FILE_PAGINATE_EXP 2048
#define QUI$V_FILE_BURST 0
#define QUI$V_FILE_BURST_EXP 1
#define QUI$V_FILE_DELETE 2
#define QUI$V_FILE_DOUBLE_SPACE 3
#define QUI$V_FILE_FLAG 4
#define QUI$V_FILE_FLAG_EXP 5
#define QUI$V_FILE_TRAILER 6
#define QUI$V_FILE_TRAILER_EXP 7
#define QUI$V_FILE_PAGE_HEADER 8
#define QUI$V_FILE_PAGINATE 9
#define QUI$V_FILE_PASSALL 10
#define QUI$V_FILE_PAGINATE_EXP 11
#define QUI$M_FILE_CHECKPOINTED 1
#define QUI$M_FILE_EXECUTING 2
#define QUI$V_FILE_CHECKPOINTED 0
#define QUI$V_FILE_EXECUTING 1
#define QUI$M_FORM_SHEET_FEED 1
#define QUI$M_FORM_TRUNCATE 2
#define QUI$M_FORM_WRAP 4
#define QUI$V_FORM_SHEET_FEED 0
#define QUI$V_FORM_TRUNCATE 1
#define QUI$V_FORM_WRAP 2
#define QUI$M_JOB_CPU_LIMIT 1
#define QUI$M_JOB_FILE_BURST 2
#define QUI$M_JOB_FILE_BURST_ONE 4
#define QUI$M_JOB_FILE_BURST_EXP 8
#define QUI$M_JOB_FILE_FLAG 16
#define QUI$M_JOB_FILE_FLAG_ONE 32
#define QUI$M_JOB_FILE_FLAG_EXP 64
#define QUI$M_JOB_FILE_TRAILER 128
#define QUI$M_JOB_FILE_TRAILER_ONE 256
#define QUI$M_JOB_FILE_TRAILER_EXP 512
#define QUI$M_JOB_LOG_DELETE 1024
#define QUI$M_JOB_LOG_NULL 2048
#define QUI$M_JOB_LOG_SPOOL 4096
#define QUI$M_JOB_LOWERCASE 8192
#define QUI$M_JOB_NOTIFY 16384
#define QUI$M_JOB_RESTART 32768
#define QUI$M_JOB_WSDEFAULT 65536
#define QUI$M_JOB_WSEXTENT 131072
#define QUI$M_JOB_WSQUOTA 262144
#define QUI$M_JOB_FILE_PAGINATE 524288
#define QUI$M_JOB_FILE_PAGINATE_EXP 1048576
#define QUI$V_JOB_CPU_LIMIT 0
#define QUI$V_JOB_FILE_BURST 1
#define QUI$V_JOB_FILE_BURST_ONE 2
#define QUI$V_JOB_FILE_BURST_EXP 3
#define QUI$V_JOB_FILE_FLAG 4
#define QUI$V_JOB_FILE_FLAG_ONE 5
#define QUI$V_JOB_FILE_FLAG_EXP 6
#define QUI$V_JOB_FILE_TRAILER 7
#define QUI$V_JOB_FILE_TRAILER_ONE 8
#define QUI$V_JOB_FILE_TRAILER_EXP 9
#define QUI$V_JOB_LOG_DELETE 10
#define QUI$V_JOB_LOG_NULL 11
#define QUI$V_JOB_LOG_SPOOL 12
#define QUI$V_JOB_LOWERCASE 13
#define QUI$V_JOB_NOTIFY 14
#define QUI$V_JOB_RESTART 15
#define QUI$V_JOB_WSDEFAULT 16
#define QUI$V_JOB_WSEXTENT 17
#define QUI$V_JOB_WSQUOTA 18
#define QUI$V_JOB_FILE_PAGINATE 19
#define QUI$V_JOB_FILE_PAGINATE_EXP 20
#define QUI$M_JOB_ABORTING 1
#define QUI$M_JOB_EXECUTING 2
#define QUI$M_JOB_HOLDING 4
#define QUI$M_JOB_INACCESSIBLE 8
#define QUI$M_JOB_REFUSED 16
#define QUI$M_JOB_REQUEUE 32
#define QUI$M_JOB_RESTARTING 64
#define QUI$M_JOB_RETAINED 128
#define QUI$M_JOB_STARTING 256
#define QUI$M_JOB_TIMED 512
#define QUI$V_JOB_ABORTING 0
#define QUI$V_JOB_EXECUTING 1
#define QUI$V_JOB_HOLDING 2
#define QUI$V_JOB_INACCESSIBLE 3
#define QUI$V_JOB_REFUSED 4
#define QUI$V_JOB_REQUEUE 5
#define QUI$V_JOB_RESTARTING 6
#define QUI$V_JOB_RETAINED 7
#define QUI$V_JOB_STARTING 8
#define QUI$V_JOB_TIMED 9
#define QUI$M_QUEUE_BATCH 1
#define QUI$M_QUEUE_CPU_DEFAULT 2
#define QUI$M_QUEUE_CPU_LIMIT 4
#define QUI$M_QUEUE_FILE_BURST 8
#define QUI$M_QUEUE_FILE_BURST_ONE 16
#define QUI$M_QUEUE_FILE_FLAG 32
#define QUI$M_QUEUE_FILE_FLAG_ONE 64
#define QUI$M_QUEUE_FILE_TRAILER 128
#define QUI$M_QUEUE_FILE_TRAILER_ONE 256
#define QUI$M_QUEUE_GENERIC 512
#define QUI$M_QUEUE_GENERIC_SELECTION 1024
#define QUI$M_QUEUE_JOB_BURST 2048
#define QUI$M_QUEUE_JOB_FLAG 4096
#define QUI$M_QUEUE_JOB_SIZE_SCHED 8192
#define QUI$M_QUEUE_JOB_TRAILER 16384
#define QUI$M_QUEUE_RETAIN_ALL 32768
#define QUI$M_QUEUE_RETAIN_ERROR 65536
#define QUI$M_QUEUE_SWAP 131072
#define QUI$M_QUEUE_TERMINAL 262144
#define QUI$M_QUEUE_WSDEFAULT 524288
#define QUI$M_QUEUE_WSEXTENT 1048576
#define QUI$M_QUEUE_WSQUOTA 2097152
#define QUI$M_QUEUE_FILE_PAGINATE 4194304
#define QUI$M_QUEUE_RECORD_BLOCKING 8388608
#define QUI$V_QUEUE_BATCH 0
#define QUI$V_QUEUE_CPU_DEFAULT 1
#define QUI$V_QUEUE_CPU_LIMIT 2
#define QUI$V_QUEUE_FILE_BURST 3
#define QUI$V_QUEUE_FILE_BURST_ONE 4
#define QUI$V_QUEUE_FILE_FLAG 5
#define QUI$V_QUEUE_FILE_FLAG_ONE 6
#define QUI$V_QUEUE_FILE_TRAILER 7
#define QUI$V_QUEUE_FILE_TRAILER_ONE 8
#define QUI$V_QUEUE_GENERIC 9
#define QUI$V_QUEUE_GENERIC_SELECTION 10
#define QUI$V_QUEUE_JOB_BURST 11
#define QUI$V_QUEUE_JOB_FLAG 12
#define QUI$V_QUEUE_JOB_SIZE_SCHED 13
#define QUI$V_QUEUE_JOB_TRAILER 14
#define QUI$V_QUEUE_RETAIN_ALL 15
#define QUI$V_QUEUE_RETAIN_ERROR 16
#define QUI$V_QUEUE_SWAP 17
#define QUI$V_QUEUE_TERMINAL 18
#define QUI$V_QUEUE_WSDEFAULT 19
#define QUI$V_QUEUE_WSEXTENT 20
#define QUI$V_QUEUE_WSQUOTA 21
#define QUI$V_QUEUE_FILE_PAGINATE 22
#define QUI$V_QUEUE_RECORD_BLOCKING 23
#define QUI$M_QUEUE_ALIGNING 1
#define QUI$M_QUEUE_IDLE 2
#define QUI$M_QUEUE_LOWERCASE 4
#define QUI$M_QUEUE_OPERATOR_REQUEST 8
#define QUI$M_QUEUE_PAUSED 16
#define QUI$M_QUEUE_PAUSING 32
#define QUI$M_QUEUE_REMOTE 64
#define QUI$M_QUEUE_RESETTING 128
#define QUI$M_QUEUE_RESUMING 256
#define QUI$M_QUEUE_SERVER 512
#define QUI$M_QUEUE_STALLED 1024
#define QUI$M_QUEUE_STARTING 2048
#define QUI$M_QUEUE_STOPPED 4096
#define QUI$M_QUEUE_STOPPING 8192
#define QUI$M_QUEUE_UNAVAILABLE 16384
#define QUI$V_QUEUE_ALIGNING 0
#define QUI$V_QUEUE_IDLE 1
#define QUI$V_QUEUE_LOWERCASE 2
#define QUI$V_QUEUE_OPERATOR_REQUEST 3
#define QUI$V_QUEUE_PAUSED 4
#define QUI$V_QUEUE_PAUSING 5
#define QUI$V_QUEUE_REMOTE 6
#define QUI$V_QUEUE_RESETTING 7
#define QUI$V_QUEUE_RESUMING 8
#define QUI$V_QUEUE_SERVER 9
#define QUI$V_QUEUE_STALLED 10
#define QUI$V_QUEUE_STARTING 11
#define QUI$V_QUEUE_STOPPED 12
#define QUI$V_QUEUE_STOPPING 13
#define QUI$V_QUEUE_UNAVAILABLE 14
#define QUI$M_SEARCH_ALL_JOBS 1
#define QUI$M_SEARCH_WILDCARD 2
#define QUI$M_SEARCH_BATCH 4
#define QUI$M_SEARCH_SYMBIONT 8
#define QUI$M_SEARCH_THIS_JOB 16
#define QUI$V_SEARCH_ALL_JOBS 0
#define QUI$V_SEARCH_WILDCARD 1
#define QUI$V_SEARCH_BATCH 2
#define QUI$V_SEARCH_SYMBIONT 3
#define QUI$V_SEARCH_THIS_JOB 4

/*  DEC/CMS REPLACEMENT HISTORY, Element SJCDEF.H */
/*  *1    12-APR-1988 11:14:28 GEORGE "Add some .H files" */
/*  DEC/CMS REPLACEMENT HISTORY, Element SJCDEF.H */
#define SJC$_ABORT_JOB 1
#define SJC$_ADD_FILE 2
#define SJC$_ALTER_JOB 3
#define SJC$_ALTER_QUEUE 4
#define SJC$_ASSIGN_QUEUE 5
#define SJC$_BATCH_CHECKPOINT 6
#define SJC$_BATCH_SERVICE 7
#define SJC$_CLOSE_DELETE 8
#define SJC$_CLOSE_JOB 9
#define SJC$_CREATE_JOB 10
#define SJC$_CREATE_QUEUE 11
#define SJC$_DEASSIGN_QUEUE 12
#define SJC$_DEFINE_CHARACTERISTIC 13
#define SJC$_DEFINE_FORM 14
#define SJC$_DELETE_CHARACTERISTIC 15
#define SJC$_DELETE_FORM 16
#define SJC$_DELETE_JOB 17
#define SJC$_DELETE_QUEUE 18
#define SJC$_ENTER_FILE 19
#define SJC$_MERGE_QUEUE 20
#define SJC$_PAUSE_QUEUE 21
#define SJC$_RESET_QUEUE 22
#define SJC$_START_ACCOUNTING 23
#define SJC$_START_QUEUE 24
#define SJC$_START_QUEUE_MANAGER 25
#define SJC$_STOP_ACCOUNTING 26
#define SJC$_STOP_QUEUE 27
#define SJC$_STOP_QUEUE_MANAGER 28
#define SJC$_SYNCHRONIZE_JOB 29
#define SJC$_WRITE_ACCOUNTING 30
#define SJC$_RESERVED_FUNC_1 31
#define SJC$_RESERVED_FUNC_2 32
#define SJC$_ACCOUNTING_MESSAGE 1
#define SJC$_ACCOUNTING_TYPES 2
#define SJC$_AFTER_TIME 3
#define SJC$_NO_AFTER_TIME 4
#define SJC$_ALIGNMENT_MASK 5
#define SJC$_ALIGNMENT_PAGES 6
#define SJC$_BASE_PRIORITY 7
#define SJC$_BATCH 8
#define SJC$_NO_BATCH 9
#define SJC$_BATCH_INPUT 10
#define SJC$_BATCH_OUTPUT 11
#define SJC$_CHARACTERISTIC_NAME 12
#define SJC$_CHARACTERISTIC_NUMBER 13
#define SJC$_NO_CHARACTERISTICS 14
#define SJC$_CHECKPOINT_DATA 15
#define SJC$_NO_CHECKPOINT_DATA 16
#define SJC$_CLI 17
#define SJC$_NO_CLI 18
#define SJC$_CPU_DEFAULT 19
#define SJC$_NO_CPU_DEFAULT 20
#define SJC$_CPU_LIMIT 21
#define SJC$_NO_CPU_LIMIT 22
#define SJC$_CREATE_START 23
#define SJC$_DELETE_FILE 24
#define SJC$_NO_DELETE_FILE 25
#define SJC$_DESTINATION_QUEUE 26
#define SJC$_DEVICE_NAME 27
#define SJC$_DOUBLE_SPACE 28
#define SJC$_NO_DOUBLE_SPACE 29
#define SJC$_ENTRY_NUMBER 30
#define SJC$_ENTRY_NUMBER_OUTPUT 31
#define SJC$_FILE_BURST 32
#define SJC$_FILE_BURST_ONE 33
#define SJC$_NO_FILE_BURST 34
#define SJC$_FILE_COPIES 35
#define SJC$_FILE_FLAG 36
#define SJC$_FILE_FLAG_ONE 37
#define SJC$_NO_FILE_FLAG 38
#define SJC$_FILE_IDENTIFICATION 39
#define SJC$_FILE_SETUP_MODULES 40
#define SJC$_NO_FILE_SETUP_MODULES 41
#define SJC$_FILE_SPECIFICATION 42
#define SJC$_FILE_TRAILER 43
#define SJC$_FILE_TRAILER_ONE 44
#define SJC$_NO_FILE_TRAILER 45
#define SJC$_FIRST_PAGE 46
#define SJC$_NO_FIRST_PAGE 47
#define SJC$_FORM_DESCRIPTION 48
#define SJC$_FORM_LENGTH 49
#define SJC$_FORM_MARGIN_BOTTOM 50
#define SJC$_FORM_MARGIN_LEFT 51
#define SJC$_FORM_MARGIN_RIGHT 52
#define SJC$_FORM_MARGIN_TOP 53
#define SJC$_FORM_NAME 54
#define SJC$_FORM_NUMBER 55
#define SJC$_FORM_SETUP_MODULES 56
#define SJC$_NO_FORM_SETUP_MODULES 57
#define SJC$_FORM_SHEET_FEED 58
#define SJC$_NO_FORM_SHEET_FEED 59
#define SJC$_FORM_STOCK 60
#define SJC$_FORM_TRUNCATE 61
#define SJC$_NO_FORM_TRUNCATE 62
#define SJC$_FORM_WIDTH 63
#define SJC$_FORM_WRAP 64
#define SJC$_NO_FORM_WRAP 65
#define SJC$_GENERIC_QUEUE 66
#define SJC$_NO_GENERIC_QUEUE 67
#define SJC$_GENERIC_SELECTION 68
#define SJC$_NO_GENERIC_SELECTION 69
#define SJC$_GENERIC_TARGET 70
#define SJC$_HOLD 71
#define SJC$_NO_HOLD 72
#define SJC$_JOB_BURST 73
#define SJC$_NO_JOB_BURST 74
#define SJC$_JOB_COPIES 75
#define SJC$_JOB_FLAG 76
#define SJC$_NO_JOB_FLAG 77
#define SJC$_JOB_LIMIT 78
#define SJC$_JOB_NAME 79
#define SJC$_JOB_RESET_MODULES 80
#define SJC$_NO_JOB_RESET_MODULES 81
#define SJC$_JOB_SIZE_MAXIMUM 82
#define SJC$_NO_JOB_SIZE_MAXIMUM 83
#define SJC$_JOB_SIZE_MINIMUM 84
#define SJC$_NO_JOB_SIZE_MINIMUM 85
#define SJC$_JOB_SIZE_SCHEDULING 86
#define SJC$_NO_JOB_SIZE_SCHEDULING 87
#define SJC$_JOB_STATUS_OUTPUT 88
#define SJC$_JOB_TRAILER 89
#define SJC$_NO_JOB_TRAILER 90
#define SJC$_LAST_PAGE 91
#define SJC$_NO_LAST_PAGE 92
#define SJC$_LIBRARY_SPECIFICATION 93
#define SJC$_NO_LIBRARY_SPECIFICATION 94
#define SJC$_LOG_DELETE 95
#define SJC$_NO_LOG_DELETE 96
#define SJC$_LOG_QUEUE 97
#define SJC$_LOG_SPECIFICATION 98
#define SJC$_NO_LOG_SPECIFICATION 99
#define SJC$_LOG_SPOOL 100
#define SJC$_NO_LOG_SPOOL 101
#define SJC$_LOWERCASE 102
#define SJC$_NO_LOWERCASE 103
#define SJC$_NEW_VERSION 104
#define SJC$_NEXT_JOB 105
#define SJC$_NOTE 106
#define SJC$_NO_NOTE 107
#define SJC$_NOTIFY 108
#define SJC$_NO_NOTIFY 109
#define SJC$_OPERATOR_REQUEST 110
#define SJC$_NO_OPERATOR_REQUEST 111
#define SJC$_OWNER_UIC 112
#define SJC$_PAGE_HEADER 113
#define SJC$_NO_PAGE_HEADER 114
#define SJC$_PAGE_SETUP_MODULES 115
#define SJC$_NO_PAGE_SETUP_MODULES 116
#define SJC$_PAGINATE 117
#define SJC$_NO_PAGINATE 118
#define SJC$_PARAMETER_1 119
#define SJC$_PARAMETER_2 120
#define SJC$_PARAMETER_3 121
#define SJC$_PARAMETER_4 122
#define SJC$_PARAMETER_5 123
#define SJC$_PARAMETER_6 124
#define SJC$_PARAMETER_7 125
#define SJC$_PARAMETER_8 126
#define SJC$_NO_PARAMETERS 127
#define SJC$_PASSALL 128
#define SJC$_NO_PASSALL 129
#define SJC$_PRIORITY 130
#define SJC$_PROCESSOR 131
#define SJC$_NO_PROCESSOR 132
#define SJC$_PROTECTION 133
#define SJC$_QUEUE 134
#define SJC$_QUEUE_FILE_SPECIFICATION 135
#define SJC$_RELATIVE_PAGE 136
#define SJC$_REQUEUE 137
#define SJC$_RESTART 138
#define SJC$_NO_RESTART 139
#define SJC$_RETAIN_ALL_JOBS 140
#define SJC$_RETAIN_ERROR_JOBS 141
#define SJC$_NO_RETAIN_JOBS 142
#define SJC$_SCSNODE_NAME 143
#define SJC$_SEARCH_STRING 144
#define SJC$_SWAP 145
#define SJC$_NO_SWAP 146
#define SJC$_TERMINAL 147
#define SJC$_NO_TERMINAL 148
#define SJC$_TOP_OF_FILE 149
#define SJC$_USER_IDENTIFICATION 150
#define SJC$_WSDEFAULT 151
#define SJC$_NO_WSDEFAULT 152
#define SJC$_WSEXTENT 153
#define SJC$_NO_WSEXTENT 154
#define SJC$_WSQUOTA 155
#define SJC$_NO_WSQUOTA 156
#define SJC$_ACCOUNT_NAME 157
#define SJC$_UIC 158
#define SJC$_USERNAME 159
#define SJC$_BUFFER_COUNT 160
#define SJC$_EXTEND_QUANTITY 161
#define SJC$_RECORD_BLOCKING 162
#define SJC$_NO_RECORD_BLOCKING 163
#define SJC$_QUEMAN_RESTART 164
#define SJC$_NO_QUEMAN_RESTART 165
#define SJC$_DEFAULT_FORM_NAME 166
#define SJC$_DEFAULT_FORM_NUMBER 167
#define SJC$_RESERVED_INPUT_3 168
#define SJC$_RESERVED_INPUT_4 169
#define SJC$_RESERVED_OUTPUT_1 170
#define SJC$_RESERVED_OUTPUT_2 171
#define SJC$M_ACCT_PROCESS 1
#define SJC$M_ACCT_IMAGE 2
#define SJC$M_ACCT_INTERACTIVE 4
#define SJC$M_ACCT_LOGIN_FAILURE 8
#define SJC$M_ACCT_SUBPROCESS 16
#define SJC$M_ACCT_DETACHED 32
#define SJC$M_ACCT_BATCH 64
#define SJC$M_ACCT_NETWORK 128
#define SJC$M_ACCT_PRINT 256
#define SJC$M_ACCT_MESSAGE 512
#define SJC$V_ACCT_PROCESS 0
#define SJC$V_ACCT_IMAGE 1
#define SJC$V_ACCT_INTERACTIVE 2
#define SJC$V_ACCT_LOGIN_FAILURE 3
#define SJC$V_ACCT_SUBPROCESS 4
#define SJC$V_ACCT_DETACHED 5
#define SJC$V_ACCT_BATCH 6
#define SJC$V_ACCT_NETWORK 7
#define SJC$V_ACCT_PRINT 8
#define SJC$V_ACCT_MESSAGE 9
#define SJC$S_ACCT_UNUSED 22
#define SJC$V_ACCT_UNUSED 10

#endif /* _PWVMS_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
