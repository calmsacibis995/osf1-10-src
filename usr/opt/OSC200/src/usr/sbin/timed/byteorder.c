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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: byteorder.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/13 17:52:21 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/* 
 * COMPONENT_NAME: TCPIP byteorder.c
 * 
 * FUNCTIONS: bytehostorder, bytenetorder 
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
 */
/* byteorder.c	1.2  com/sockcmd/timed,3.1,9021 10/8/89 17:42:57 */
/*
#ifndef lint
static char sccsid[] = "byteorder.c	2.6 (Berkeley) 6/18/88";
#endif  not lint */

#include "globals.h"
#include <protocols/timed.h>

/*
 * Two routines to do the necessary byte swapping for timed protocol
 * messages. Protocol is defined in /usr/include/protocols/timed.h
 */

bytenetorder(ptr)
struct tsp *ptr;
{
	ptr->tsp_seq = htons((u_short)ptr->tsp_seq);
	switch (ptr->tsp_type) {

	case TSP_SETTIME:
	case TSP_ADJTIME:
	case TSP_SETDATE:
	case TSP_SETDATEREQ:
		ptr->tsp_time.tv_sec = htonl(ptr->tsp_time.tv_sec);
		ptr->tsp_time.tv_usec = htonl(ptr->tsp_time.tv_usec);
		break;
	
	default:
		break;		/* nothing more needed */
	}
}

bytehostorder(ptr)
struct tsp *ptr;
{
	ptr->tsp_seq = ntohs((u_short)ptr->tsp_seq);
	switch (ptr->tsp_type) {

	case TSP_SETTIME:
	case TSP_ADJTIME:
	case TSP_SETDATE:
	case TSP_SETDATEREQ:
		ptr->tsp_time.tv_sec = ntohl(ptr->tsp_time.tv_sec);
		ptr->tsp_time.tv_usec = ntohl(ptr->tsp_time.tv_usec);
		break;
	
	default:
		break;		/* nothing more needed */
	}
}
