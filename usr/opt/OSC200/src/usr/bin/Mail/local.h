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
static char rcsid[] = "@(#)$RCSfile: local.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/08/02 18:20:23 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/***
***/
/* 
 * COMPONENT_NAME: CMDMAILX local.h
 * 
 * FUNCTIONS: dup2, gethostname, gtty, stty, vfork 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/utsname.h>
#include "v7.local.h"

#define vfork() fork()
#define dup2(a,b) close(b);dup(a)

#define sgttyb termio
#define gtty(a,b) ioctl(a, TCGETA, b)
#define stty(a,b) ioctl(a, TCSETA, b)
#define sg_ospeed c_cflag & CBAUD
#define sg_erase c_cc[2]
#define sg_kill c_cc[3]
