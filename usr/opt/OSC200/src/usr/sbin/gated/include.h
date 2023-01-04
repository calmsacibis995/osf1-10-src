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
 *	@(#)$RCSfile: include.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/09 22:12:04 $
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
 * COMPONENT_NAME: TCPIP include.h
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 10 26 27 39 36
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   CENTER FOR THEORY AND SIMULATION IN SCIENCE AND ENGINEERING
 *			CORNELL UNIVERSITY
 *
 *      Portions of this software may fall under the following
 *      copyrights: 
 *
 *	Copyright (c) 1983 Regents of the University of California.
 *	All rights reserved.  The Berkeley software License Agreement
 *	specifies the terms and conditions for redistribution.
 *
 *  GATED - based on Kirton's EGP, UC Berkeley's routing daemon (routed),
 *	    and DCN's HELLO routing Protocol.
 *
 *
 */

/* include.h
 *
 * System and EGP header files to be included.
 */


#ifdef  vax11c
#include "config.h"
#endif /*  vax11c */
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/time.h>
#ifdef  vax11c
#include <sys/ttychars.h>
#include <sys/ttydev.h>
#endif /*  vax11c */
#include <sys/ioctl.h>
#ifndef vax11c
#include <sys/uio.h>
#endif /*  vax11c */

#include <sys/socket.h>
#ifndef vax11c
#include <sys/file.h>
#endif /*  vax11c */
#include <sys/audit.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <strings.h>

#ifdef vax11c
#define DONT_INCLUDE_IF_ARP
#endif /* vax11c */
#include <net/if.h>
#include <net/route.h>
#include <syslog.h>

#include "routed.h"
#include "defs.h"
#include "egp.h"
#include "egp_param.h"
#include "if.h"
#include "rt_table.h" 
#include "rt_control.h"
#include "af.h"
#include "trace.h"
#ifndef NSS
#include "rip.h"
#include "hello.h"
#endif
