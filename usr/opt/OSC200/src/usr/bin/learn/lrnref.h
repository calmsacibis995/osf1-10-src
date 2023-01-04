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
 *	@(#)$RCSfile: lrnref.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/10 15:57:02 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*

 */

/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: none 
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * lrnref.h	1.2  com/cmd/man/learn,3.1,9021 9/14/89 06:38:39
 */

#include <NLchar.h>
#define STRCMP NLstrcmp

#define PRINTF printf
#define FPRINTF fprintf

#define	READY	0
#define	PRINT	1
#define	COPYIN	2
#define	COPYOUT	3
#define	UNCOPIN	4
#define	UNCOPOUT	5
#define	PIPE	6
#define	UNPIPE	7
#define	YES	8
#define	NO	9
#define	SUCCEED	10
#define	FAIL	11
#define	BYE	12
#define	LOG	13
#define	CHDIR	14
#define	LEARN	15
#define	MV	16
#define	USER	17
#define	NEXT	18
#define	SKIP	19
#define	WHERE	20
#define	MATCH	21
#define	NOP	22
#define	BAD	23
#define	CREATE	24
#define	CMP	25
#define	ONCE	26
#define	AGAIN	27
#define	HINT	28

#define MAX_LEN 200
#define LEN_MAX 100
#define LEN_L   30

extern	int	more;
extern	char	*level;
extern	int	speed;
extern	char	*sname;
extern	char	*dname;
extern	char	*todo;
extern	int	didok;
extern	int	sequence;
extern	int	comfile;
extern	int	status;
extern	int	wrong;
extern	char	*pwline;
extern	char	*dir;
extern	FILE	*incopy;
extern	FILE	*scrin;
extern	int	logging;
extern	int	ask;
extern 	int	again;
extern	int	skip;
extern	int	teed;
extern	int	total;
