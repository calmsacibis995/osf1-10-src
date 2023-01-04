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
 * @(#)$RCSfile: fmtmsg.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/05/11 20:36:45 $
 */

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 *  Major Classifications
 */
#define MM_HARD		0x00000001L
#define MM_SOFT		0x00000002L
#define MM_FIRM		0x00000004L
/*
 *  Message Source Sub-classifications
 */
#define MM_APPL		0x00000100L
#define MM_UTIL 	0x00000200L
#define MM_OPSYS	0x00000400L
/*
 *  Display Sub-classifications
 */
#define MM_PRINT	0x00010000L
#define MM_CONSOLE	0x00020000L
/*
 *  Status sub-classifications
 */
#define MM_RECOVER	0x01000000L
#define MM_NRECOV	0x02000000L
/*
 *  Severity 
 */
#define MM_NOSEV	0
#define MM_HALT		1
#define MM_ERROR	2
#define MM_WARNING	3
#define MM_INFO		4

/*
 *  Exit Codes
 */
#define MM_OK		1
#define MM_NOTOK	2
#define MM_NOMSG	3
#define MM_NOCON	4
/*
 *  Null Values
 */
#define MM_NULLLBL	(char *)NULL	/* label */
#define MM_NULLSEV	(int)0 		/* severity */ 
#define MM_NULLMC	(long)0		/* classification */
#define MM_NULLTXT	(char *)NULL	/* text */
#define MM_NULLACT	(char *)NULL	/* action */
#define MM_NULLTAG	(char *)NULL	/* tag */


extern int fmtmsg( long, const char *, int, const char *, const char *, const char *);
