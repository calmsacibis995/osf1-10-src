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
/* Created 14-JUL-1992 18:42:20.32  by DDIS Parse-Table Generator V1.0        */
/* Source:  1-APR-1992 16:21:06.00  MEMEX$BUILD:[HIS.BL24A.SOURCE]HIS_PROPERTY.DDIS;1  */
/******************************************************************************/

#ifdef vms
globaldef unsigned char lwk_lwkproperty[] = {
#else
unsigned char lwk_lwkproperty[] = {
#endif
/******************************************************************************/
/* Parse Table Header                                                         */
/******************************************************************************/
	255,   1,   0,   0,		/* Ident                            */
	 20,   0,   0,   0,		/* TABLE_BEGIN-lwk_lwkproperty                  */
	 16,   1,   0,   0,		/* ENTRY_BEGIN-lwk_lwkproperty                  */
	 24,   1,   0,   0,		/* VALUE_BEGIN-lwk_lwkproperty                  */
	 24,   1,   0,   0,		/* TABLE_END-lwk_lwkproperty                  */
/******************************************************************************/
/* Parse Table Entries                                                        */
/******************************************************************************/
	 16,   0,   0,   0,	/* 0 LWK_K_T_PROPERTY [UNIVERSAL 16]  */
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

	128,   0,   0,   0,	/* 2 LWK_K_T_PROPERTY_NAME [CONTEXT 0] */
	136,			/* OCTET STRING             */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	129,   0,   0,   0,	/* 3 LWK_K_T_PROPERTY_DOMAIN [CONTEXT 1] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	130,   0,   0,   0,	/* 4 LWK_K_T_PROPERTY_VALUE [CONTEXT 2] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  3,   0,		/*   type offset to 7       */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	131,   0,   0,   0,	/* 5 LWK_K_T_PROPERTY_VALUES [CONTEXT 3] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  9,   0,		/*   type offset to 14      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 6 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	138,   0,   0,   0,	/* 7 LWK_K_T_VALUE_INTEGER [CONTEXT 10] */
	  6,			/* INTEGER                  */
	  4,			/* SEQUENCE CHOICE          */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	139,   0,   0,   0,	/* 8 LWK_K_T_VALUE_BOOLEAN [CONTEXT 11] */
	  6,			/* INTEGER                  */
	  4,			/* SEQUENCE CHOICE          */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	140,   0,   0,   0,	/* 9 LWK_K_T_VALUE_FLOAT [CONTEXT 12] */
	  5,			/* FLOATING-POINT           */
	  4,			/* SEQUENCE CHOICE          */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	141,   0,   0,   0,	/* 10 LWK_K_T_VALUE_DATE [CONTEXT 13]  */
	136,			/* OCTET STRING             */
	  4,			/* SEQUENCE CHOICE          */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	142,   0,   0,   0,	/* 11 LWK_K_T_VALUE_STRING [CONTEXT 14] */
	136,			/* OCTET STRING             */
	  4,			/* SEQUENCE CHOICE          */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	143,   0,   0,   0,	/* 12 LWK_K_T_VALUE_CSTRING [CONTEXT 15] */
	136,			/* OCTET STRING             */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 13 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	158,   0,   0,   0,	/* 14 LWK_K_T_VALUES_COUNT [CONTEXT 30] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  31,   0,   0,	/* 15 LWK_K_T_VALUES_DOMAIN [CONTEXT 31] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	159,  32,   0,   0,	/* 16 LWK_K_T_VALUES_ELEMENTS [CONTEXT 32] */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  2,   0,		/*   type offset to 18      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 17 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	130,   0,   0,   0,	/* 18 LWK_K_T_VALUES_VALUE [CONTEXT 2] */
	  0,			/*                          */
	  5,			/* SEQUENCE OF              */
	245, 255,		/*   type offset to 7       */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	131,   0,   0,   0,	/* 19 LWK_K_T_VALUES_VALUES [CONTEXT 3] */
	  0,			/*                          */
	  5,			/* SEQUENCE OF              */
	251, 255,		/*   type offset to 14      */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 20 DDIS$K_T_EOC                     */
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
