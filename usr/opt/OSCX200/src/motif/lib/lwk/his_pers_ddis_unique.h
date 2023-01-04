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
/********************************************************************************************************************************/
/* Created 26-JAN-1990 11:58:32 by VAX SDL V3.2-11     Source: 26-JAN-1990 11:46:50 SYS$MMXP:[RAYNER.MEMEX.SERVICES]PERS.SDL;2 */
/********************************************************************************************************************************/
 
/*** MODULE LwkOpPERS_ENTRIES IDENT AX02-001 ***/
#define LWK_K_P_PERSISTENT 0
#define LWK_K_E_ANY 1
#define LWK_K_P_PERS_TYPE 2
#define LWK_K_P_PERS_PROPERTIES 3
#define LWK_K_E_PERSISTENT 4
#define LWK_K_P_PERS_SURROGATE 5
#define LWK_K_P_PERS_LINK 6
#define LWK_K_P_PERS_STEP 7
#define LWK_K_P_PERS_LINKNET 8
#define LWK_K_P_PERS_PATH 9
#define LWK_K_P_PERS_COMP_LINKNET 10
#define LWK_K_P_PERS_COMP_PATH 11
#define LWK_K_E_PERS_TYPE 12
#define LWK_K_P_SURR_SUBTYPE 13
#define LWK_K_E_PERS_SURROGATE 14
#define LWK_K_P_LINK_TYPE 15
#define LWK_K_P_LINK_SRC_DESC 16
#define LWK_K_P_LINK_SRC_KEYWORDS 17
#define LWK_K_P_LINK_TAR_DESC 18
#define LWK_K_P_LINK_TAR_KEYWORDS 19
#define LWK_K_P_LINK_SOURCE 20
#define LWK_K_P_LINK_TARGET 21
#define LWK_K_E_PERS_LINK 22
#define LWK_K_P_LINK_SRC_KEY_COUNT 23
#define LWK_K_P_LINK_SRC_WORDS 24
#define LWK_K_E_LINK_SRC_KEYWORDS 25
#define LWK_K_E_LINK_SRC_WORDS 27
#define LWK_K_P_LINK_TAR_KEY_COUNT 28
#define LWK_K_P_LINK_TAR_WORDS 29
#define LWK_K_E_LINK_TAR_KEYWORDS 30
#define LWK_K_E_LINK_TAR_WORDS 27
#define LWK_K_P_STEP_FOLLOW_TYPE 31
#define LWK_K_P_STEP_INTERVAL 32
#define LWK_K_P_STEP_OPERATION 33
#define LWK_K_P_STEP_ORIGIN 34
#define LWK_K_P_STEP_DESTINATION 35
#define LWK_K_E_PERS_STEP 36
#define LWK_K_P_LNET_NAME 37
#define LWK_K_P_LNET_SURROGATES 38
#define LWK_K_P_LNET_LINKS 39
#define LWK_K_E_PERS_LINKNET 40
#define LWK_K_P_LNET_SURR_COUNT 41
#define LWK_K_P_LNET_SURR_ELEMENTS 42
#define LWK_K_E_LNET_SURROGATES 43
#define LWK_K_E_LNET_SURR_ELEMENTS 45
#define LWK_K_P_LNET_SURR_PROPS 46
#define LWK_K_P_LNET_SURR_ID 47
#define LWK_K_P_LNET_LINK_COUNT 49
#define LWK_K_P_LNET_LINK_ELEMENTS 50
#define LWK_K_E_LNET_LINKS 51
#define LWK_K_E_LNET_LINK_ELEMENTS 53
#define LWK_K_P_LNET_LINK_PROPS 54
#define LWK_K_P_LNET_LINK_SOURCE_ID 55
#define LWK_K_P_LNET_LINK_TARGET_ID 56
#define LWK_K_P_PATH_NAME 58
#define LWK_K_P_PATH_SURROGATES 59
#define LWK_K_P_PATH_STEPS 60
#define LWK_K_E_PERS_PATH 61
#define LWK_K_P_PATH_SURR_COUNT 62
#define LWK_K_P_PATH_SURR_ELEMENTS 63
#define LWK_K_E_PATH_SURROGATES 64
#define LWK_K_E_PATH_SURR_ELEMENTS 66
#define LWK_K_P_PATH_SURR_PROPS 67
#define LWK_K_P_PATH_SURR_ID 68
#define LWK_K_P_PATH_STEP_COUNT 70
#define LWK_K_P_PATH_STEP_ELEMENTS 71
#define LWK_K_E_PATH_STEPS 73
#define LWK_K_E_PATH_STEP_ELEMENTS 74
#define LWK_K_P_PATH_STEP_PROPS 75
#define LWK_K_P_PATH_STEP_ORIG_ID 76
#define LWK_K_P_PATH_STEP_DEST_ID 77
#define LWK_K_P_PATH_STEP_IS_CURR 78
#define LWK_K_P_CLNET_NAME 80
#define LWK_K_P_CLNET_OBJECT_DESCS 81
#define LWK_K_E_PERS_COMP_LINKNET 82
#define LWK_K_P_CLNET_DESC_COUNT 83
#define LWK_K_P_CLNET_DESCS 84
#define LWK_K_E_CLNET_OBJECT_DESCS 85
#define LWK_K_E_CLNET_DESCS 27
#define LWK_K_P_CPATH_NAME 86
#define LWK_K_P_CPATH_OBJECT_DESCS 87
#define LWK_K_E_PERS_COMP_PATH 88
#define LWK_K_P_CPATH_DESC_COUNT 89
#define LWK_K_P_CPATH_DESCS 90
#define LWK_K_E_CPATH_OBJECT_DESCS 91
#define LWK_K_E_CPATH_DESCS 27
#define LWK_K_P_PERS_DESCRIPTION 92
#define LWK_K_P_PERS_AUTHOR 93
#define LWK_K_P_PERS_CREATION_DATE 94
#define LWK_K_P_PERS_KEYWORDS 95
#define LWK_K_P_PERS_APPL_PROPS 96
#define LWK_K_E_PERS_PROPERTIES 97
#define LWK_K_P_PERS_KEY_COUNT 98
#define LWK_K_P_PERS_WORDS 99
#define LWK_K_E_PERS_KEYWORDS 100
#define LWK_K_E_PERS_WORDS 27
#define LWK_K_P_PERS_PROP_COUNT 101
#define LWK_K_P_PERS_PROP_ELEMENTS 102
#define LWK_K_E_PERS_APPL_PROPS 103
#define LWK_K_E_PERS_PROP_ELEMENTS 105
