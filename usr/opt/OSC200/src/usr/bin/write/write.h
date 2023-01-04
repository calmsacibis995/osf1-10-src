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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: write.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 21:17:04 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* write.h	1.3  com/cmd/comm/write,3.1,9021 2/9/90 16:34:35 */

#define MAX_USERID_LEN 32 /* same as in struct utmp declared in utmp.h */
#define MAX_HOST_LEN 64  /* as defined in ping.c */
#define MAX_SER_LEN 10
#define DATE_LEN 30
#define MAX_TTY_LEN 32 /* same as in struct utmp declared in utmp.h */

#define RELAY  '0'
#define RWRITE '1'
#define HWRITE '2'
#define QUERY  '3'

#define OK 0
#define CANCEL 1
#define MQUERY 2

#define SOK "ok"
#define SCANCEL "cancel"
#define SQUERY "query"

/* common control characters */
#define WRT_BELL	7  
#define WRT_NEWLINE	10	

/* error codes from writesrv */
#define NOTLOG 0
#define NOTTY -1
#define NOPERM -2
#define NOOPEN -3
#define MALLOC -4
#define BADHAND -5
#define SNDRPLY -6
#define GETRPLY -7 
#define NOSERVICE -8

struct ttys {
	char tty[PATH_MAX+1];
	struct ttys *next;
};
