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
static char rcsid[] = "@(#)$RCSfile: catio.h,v $ $Revision: 4.3.9.4 $ (DEC) $Date: 1993/06/10 16:48:32 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * COMPONENT_NAME: CMDMSG
 *
 * FUNCTIONS: catio.h
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 * 
 * catio.h	1.5  com/cmd/msg/catio.h, cmdmsg, bos320, 9125320 5/29/91
 */

/*
 * Note: There is a duplicate (sort of) of this file in libc.
 * 
 * If you change any values here, make sure the other also gets
 * changed.
 */

#ifndef _BLD	/* bootstrap indicator */
#include <mesg.h>
#endif /* _BLD */
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#define ERR 		(-1)
#define TRUE 		1
#define FALSE 		0

#define QTSTR		"$quote"
#define SETSTR		"$set"

#define PATH_FORMAT	"/usr/lib/nls/msg/%L/%N"
#define DEFAULT_LANG	"C"

#define SETMIN		1
#define SETMAX  	65535

#define die(s)			fputs((s), stderr), fputc('\n', stderr), exit(1)

#define FILE_UNUSED     (-1)
#define FILE_DUMMY      (-2)
