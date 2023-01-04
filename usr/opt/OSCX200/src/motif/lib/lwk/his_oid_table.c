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
/* Created 14-JUL-1992 18:42:14.53  by DDIS Parse-Table Generator V1.0        */
/* Source:  1-APR-1992 16:19:51.00  MEMEX$BUILD:[HIS.BL24A.SOURCE]HIS_OBJECTID.DDIS;1  */
/******************************************************************************/

#ifdef vms
globaldef unsigned char lwk_lwkobjectid[] = {
#else
unsigned char lwk_lwkobjectid[] = {
#endif
/******************************************************************************/
/* Parse Table Header                                                         */
/******************************************************************************/
	255,   1,   0,   0,		/* Ident                            */
	 20,   0,   0,   0,		/* TABLE_BEGIN-lwk_lwkobjectid                  */
	152,   0,   0,   0,		/* ENTRY_BEGIN-lwk_lwkobjectid                  */
	160,   0,   0,   0,		/* VALUE_BEGIN-lwk_lwkobjectid                  */
	160,   0,   0,   0,		/* TABLE_END-lwk_lwkobjectid                  */
/******************************************************************************/
/* Parse Table Entries                                                        */
/******************************************************************************/
	 16,   0,   0,   0,	/* 0 LWK_K_T_OBJECTID [UNIVERSAL 16]  */
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

	128,   0,   0,   0,	/* 2 LWK_K_T_DESCRIPTION [CONTEXT 0]  */
	  0,			/*                          */
	  2,			/* SEQUENCE optional        */
	  6,   0,		/*   type offset to 8       */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	138,   0,   0,   0,	/* 3 LWK_K_T_LINKBASE_ID [CONTEXT 10] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	139,   0,   0,   0,	/* 4 LWK_K_T_OBJECT_ID [CONTEXT 11]   */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	140,   0,   0,   0,	/* 5 LWK_K_T_OBJECT_DOMAIN [CONTEXT 12] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	141,   0,   0,   0,	/* 6 LWK_K_T_CONTAINER_ID [CONTEXT 13] */
	  6,			/* INTEGER                  */
	  1,			/* SEQUENCE required        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 7 DDIS$K_T_EOC                     */
	  0,			/*                          */
	  0,			/* SEQUENCE end             */
	  0,   0,		/*                          */
	  0,   0,		/*                          */
	  0,   0,		/*                          */

	129,   0,   0,   0,	/* 8 LWK_K_T_LINKBASE_NAME [CONTEXT 1] */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	130,   0,   0,   0,	/* 9 LWK_K_T_OBJECT_NAME [CONTEXT 2]  */
	136,			/* OCTET STRING             */
	  2,			/* SEQUENCE optional        */
	  0,   0,		/*                          */
	  0,   0,		/* No default value         */
	  0,   0,		/*                          */

	  0,   0,   0,   0,	/* 10 DDIS$K_T_EOC                     */
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
