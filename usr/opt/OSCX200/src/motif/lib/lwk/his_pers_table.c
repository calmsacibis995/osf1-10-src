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
/******************************************************************************/
/* Created 14-JUL-1992 18:42:08.40  by DDIS Parse-Table Generator V1.0        */
/* Source:  1-APR-1992 16:20:40.00  MEMEX$BUILD:[HIS.BL24A.SOURCE]HIS_PERSISTENT.DDIS;1  */
/******************************************************************************/

#ifdef vms
globaldef unsigned char lwk_lwkpersistent[] = {
#else
unsigned char lwk_lwkpersistent[] = {
#endif
/******************************************************************************/
/* Parse Table Header                                                         */
/******************************************************************************/
	255,   1,   0,   0,		/* Ident                            */
	 20,   0,   0,   0,		/* TABLE_BEGIN-lwk_lwkpersistent                */
	 12,   5,   0,   0,		/* ENTRY_BEGIN-lwk_lwkpersistent                */
	 20,   5,   0,   0,		/* VALUE_BEGIN-lwk_lwkpersistent                */
	 20,   5,   0,   0,		/* TABLE_END-lwk_lwkpersistent                */
/******************************************************************************/
/* Parse Table Entries                                                        */
/******************************************************************************/
	 16,   0,   0,   0,	/* 0 LWK_K_T_PERSISTENT [UNIVERSAL 16] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	  2,   0,		/*   type offset to 2       */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 1 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	223, 255, 118,   0,	/* 2 LWK_K_T_PERS_TYPE [PRIVATE 16374] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	  3,   0,		/*   type offset to 5       */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	148,   0,   0,   0,	/* 3 LWK_K_T_PERS_PROPERTIES [CONTEXT 20] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	 89,   0,		/*   type offset to 92      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 4 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	129,   0,   0,   0,	/* 5 LWK_K_T_PERS_SURROGATE [CONTEXT 1] */
	  0,			/*                          */
	  4,			/* SEQUENCE CHOICE          */
	  8,   0,		/*   type offset to 13      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	130,   0,   0,   0,	/* 6 LWK_K_T_PERS_LINK [CONTEXT 2]    */
	  0,			/*                          */
	  4,			/* SEQUENCE CHOICE          */
	  9,   0,		/*   type offset to 15      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	131,   0,   0,   0,	/* 7 LWK_K_T_PERS_STEP [CONTEXT 3]    */
	  0,			/*                          */
	  4,			/* SEQUENCE CHOICE          */
	 24,   0,		/*   type offset to 31      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	132,   0,   0,   0,	/* 8 LWK_K_T_PERS_LINKNET [CONTEXT 4] */
	  0,			/*                          */
	  4,			/* SEQUENCE CHOICE          */
	 29,   0,		/*   type offset to 37      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	133,   0,   0,   0,	/* 9 LWK_K_T_PERS_PATH [CONTEXT 5]    */
	  0,			/*                          */
	  4,			/* SEQUENCE CHOICE          */
	 49,   0,		/*   type offset to 58      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	134,   0,   0,   0,	/* 10 LWK_K_T_PERS_COMP_LINKNET [CONTEXT 6] */
	  0,			/*                          */
	  4,			/* SEQUENCE CHOICE          */
	 70,   0,		/*   type offset to 80      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	135,   0,   0,   0,	/* 11 LWK_K_T_PERS_COMP_PATH [CONTEXT 7] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	 75,   0,		/*   type offset to 86      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 12 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159,  40,   0,   0,	/* 13 LWK_K_T_SURR_SUBTYPE [CONTEXT 40] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 14 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159,  60,   0,   0,	/* 15 LWK_K_T_LINK_TYPE [CONTEXT 60]   */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  61,   0,   0,	/* 16 LWK_K_T_LINK_SRC_DESC [CONTEXT 61] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  62,   0,   0,	/* 17 LWK_K_T_LINK_SRC_KEYWORDS [CONTEXT 62] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  6,   0,		/*   type offset to 23      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  65,   0,   0,	/* 18 LWK_K_T_LINK_TAR_DESC [CONTEXT 65] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  66,   0,   0,	/* 19 LWK_K_T_LINK_TAR_KEYWORDS [CONTEXT 66] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  9,   0,		/*   type offset to 28      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  80,   0,   0,	/* 20 LWK_K_T_LINK_SOURCE [CONTEXT 80] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  81,   0,   0,	/* 21 LWK_K_T_LINK_TARGET [CONTEXT 81] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 22 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159,  63,   0,   0,	/* 23 LWK_K_T_LINK_SRC_KEY_COUNT [CONTEXT 63] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  64,   0,   0,	/* 24 LWK_K_T_LINK_SRC_WORDS [CONTEXT 64] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	  2,   0,		/*   type offset to 26      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 25 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	  4,   0,   0,   0,	/* 26 DDIS$K_T_OCTET_STRING [UNIVERSAL 4] */
	136,			/* OCTET STRING             */
	  5,			/* SEQUENCE OF              */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 27 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159,  67,   0,   0,	/* 28 LWK_K_T_LINK_TAR_KEY_COUNT [CONTEXT 67] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  68,   0,   0,	/* 29 LWK_K_T_LINK_TAR_WORDS [CONTEXT 68] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	253, 255,		/*   type offset to 26      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 30 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159,  90,   0,   0,	/* 31 LWK_K_T_STEP_FOLLOW_TYPE [CONTEXT 90] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  91,   0,   0,	/* 32 LWK_K_T_STEP_INTERVAL [CONTEXT 91] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  92,   0,   0,	/* 33 LWK_K_T_STEP_OPERATION [CONTEXT 92] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 100,   0,   0,	/* 34 LWK_K_T_STEP_ORIGIN [CONTEXT 100] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 101,   0,   0,	/* 35 LWK_K_T_STEP_DESTINATION [CONTEXT 101] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 36 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 110,   0,   0,	/* 37 LWK_K_T_LNET_NAME [CONTEXT 110]  */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 120,   0,   0,	/* 38 LWK_K_T_LNET_SURROGATES [CONTEXT 120] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  3,   0,		/*   type offset to 41      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,   2,   0,	/* 39 LWK_K_T_LNET_LINKS [CONTEXT 130] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	 10,   0,		/*   type offset to 49      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 40 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 121,   0,   0,	/* 41 LWK_K_T_LNET_SURR_COUNT [CONTEXT 121] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 122,   0,   0,	/* 42 LWK_K_T_LNET_SURR_ELEMENTS [CONTEXT 122] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	  2,   0,		/*   type offset to 44      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 43 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	 16,   0,   0,   0,	/* 44 LWK_K_T_ [UNIVERSAL 16]          */
	  0,			/*                          */
	  5,			/* SEQUENCE OF              */
	  2,   0,		/*   type offset to 46      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 45 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 123,   0,   0,	/* 46 LWK_K_T_LNET_SURR_PROPS [CONTEXT 123] */
	136,			/* OCTET STRING             */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 124,   0,   0,	/* 47 LWK_K_T_LNET_SURR_ID [CONTEXT 124] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 48 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,   3,   0,	/* 49 LWK_K_T_LNET_LINK_COUNT [CONTEXT 131] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,   4,   0,	/* 50 LWK_K_T_LNET_LINK_ELEMENTS [CONTEXT 132] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	  2,   0,		/*   type offset to 52      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 51 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	 16,   0,   0,   0,	/* 52 LWK_K_T_ [UNIVERSAL 16]          */
	  0,			/*                          */
	  5,			/* SEQUENCE OF              */
	  2,   0,		/*   type offset to 54      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 53 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,   5,   0,	/* 54 LWK_K_T_LNET_LINK_PROPS [CONTEXT 133] */
	136,			/* OCTET STRING             */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,   6,   0,	/* 55 LWK_K_T_LNET_LINK_SOURCE_ID [CONTEXT 134] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,   7,   0,	/* 56 LWK_K_T_LNET_LINK_TARGET_ID [CONTEXT 135] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 57 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  12,   0,	/* 58 LWK_K_T_PATH_NAME [CONTEXT 140]  */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  22,   0,	/* 59 LWK_K_T_PATH_SURROGATES [CONTEXT 150] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  3,   0,		/*   type offset to 62      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  32,   0,	/* 60 LWK_K_T_PATH_STEPS [CONTEXT 160] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	 10,   0,		/*   type offset to 70      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 61 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  23,   0,	/* 62 LWK_K_T_PATH_SURR_COUNT [CONTEXT 151] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  24,   0,	/* 63 LWK_K_T_PATH_SURR_ELEMENTS [CONTEXT 152] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	  2,   0,		/*   type offset to 65      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 64 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	 16,   0,   0,   0,	/* 65 LWK_K_T_ [UNIVERSAL 16]          */
	  0,			/*                          */
	  5,			/* SEQUENCE OF              */
	  2,   0,		/*   type offset to 67      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 66 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  25,   0,	/* 67 LWK_K_T_PATH_SURR_PROPS [CONTEXT 153] */
	136,			/* OCTET STRING             */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  26,   0,	/* 68 LWK_K_T_PATH_SURR_ID [CONTEXT 154] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 69 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  33,   0,	/* 70 LWK_K_T_PATH_STEP_COUNT [CONTEXT 161] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  34,   0,	/* 71 LWK_K_T_PATH_STEP_ELEMENTS [CONTEXT 162] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	  2,   0,		/*   type offset to 73      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 72 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	 16,   0,   0,   0,	/* 73 LWK_K_T_ [UNIVERSAL 16]          */
	  0,			/*                          */
	  5,			/* SEQUENCE OF              */
	  2,   0,		/*   type offset to 75      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 74 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  35,   0,	/* 75 LWK_K_T_PATH_STEP_PROPS [CONTEXT 163] */
	136,			/* OCTET STRING             */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  36,   0,	/* 76 LWK_K_T_PATH_STEP_ORIG_ID [CONTEXT 164] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  37,   0,	/* 77 LWK_K_T_PATH_STEP_DEST_ID [CONTEXT 165] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  38,   0,	/* 78 LWK_K_T_PATH_STEP_IS_CURR [CONTEXT 166] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 79 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  42,   0,	/* 80 LWK_K_T_CLNET_NAME [CONTEXT 170] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  43,   0,	/* 81 LWK_K_T_CLNET_OBJECT_DESCS [CONTEXT 171] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  2,   0,		/*   type offset to 83      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 82 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  44,   0,	/* 83 LWK_K_T_CLNET_DESC_COUNT [CONTEXT 172] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  45,   0,	/* 84 LWK_K_T_CLNET_DESCS [CONTEXT 173] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	198, 255,		/*   type offset to 26      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 85 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  62,   0,	/* 86 LWK_K_T_CPATH_NAME [CONTEXT 190] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  63,   0,	/* 87 LWK_K_T_CPATH_OBJECT_DESCS [CONTEXT 191] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  2,   0,		/*   type offset to 89      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 88 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159, 129,  64,   0,	/* 89 LWK_K_T_CPATH_DESC_COUNT [CONTEXT 192] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159, 129,  65,   0,	/* 90 LWK_K_T_CPATH_DESCS [CONTEXT 193] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	192, 255,		/*   type offset to 26      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 91 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	149,   0,   0,   0,	/* 92 LWK_K_T_PERS_DESCRIPTION [CONTEXT 21] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	150,   0,   0,   0,	/* 93 LWK_K_T_PERS_AUTHOR [CONTEXT 22] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	151,   0,   0,   0,	/* 94 LWK_K_T_PERS_CREATION_DATE [CONTEXT 23] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	152,   0,   0,   0,	/* 95 LWK_K_T_PERS_KEYWORDS [CONTEXT 24] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  3,   0,		/*   type offset to 98      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	158,   0,   0,   0,	/* 96 LWK_K_T_PERS_APPL_PROPS [CONTEXT 30] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  5,   0,		/*   type offset to 101     */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 97 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	153,   0,   0,   0,	/* 98 LWK_K_T_PERS_KEY_COUNT [CONTEXT 25] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	154,   0,   0,   0,	/* 99 LWK_K_T_PERS_WORDS [CONTEXT 26]  */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	183, 255,		/*   type offset to 26      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 100 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	159,  31,   0,   0,	/* 101 LWK_K_T_PERS_PROP_COUNT [CONTEXT 31] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  32,   0,   0,	/* 102 LWK_K_T_PERS_PROP_ELEMENTS [CONTEXT 32] */
	  0,			/*                          */
	  1,			/* SEQUENCE required        */
	  2,   0,		/*   type offset to 104     */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 103 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	  4,   0,   0,   0,	/* 104 DDIS$K_T_OCTET_STRING [UNIVERSAL 4] */
	136,			/* OCTET STRING             */
	  5,			/* SEQUENCE OF              */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 105 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

/******************************************************************************/
/* Entry Vectors                                                              */
/******************************************************************************/

	  1,   0,   0,   0,		/* Entry vector count               */
	  0,   0,   0,   0,		/* Entry point - index 0 */

/******************************************************************************/
/* Value Vectors                                                              */
/******************************************************************************/
/******************************************************************************/
/* Value Table Entries                                                        */
/******************************************************************************/
/******************************************************************************/
/* Parse Table End                                                            */
/******************************************************************************/
   };
