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
static char rcsid[] = "@(#)$RCSfile: talk_ctl.h,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/11/02 19:21:23 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: TCPIP talk_ctl.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	talk_ctl.h	5.4 (Berkeley) 6/29/88
 */
/* talk_ctl.h	1.3  com/sockcmd/talk,3.1,9021 10/8/89 17:31:52 */

#include <sys/types.h>
#include <protocols/talkd.h>
#include <netinet/in.h>
#include "talk.h"
#include <errno.h>

extern	int errno;

extern	struct sockaddr_in daemon_addr;
extern	struct sockaddr_in ctl_addr;
extern	struct sockaddr_in my_addr;
extern	struct in_addr my_machine_addr;
extern	struct in_addr his_machine_addr;
extern	u_short daemon_port;
extern	int ctl_sockt;
extern	CTL_MSG msg;
