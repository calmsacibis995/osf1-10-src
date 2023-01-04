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
 *	@(#)$RCSfile: af.h,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/06/03 18:00:38 $
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

/* af.h	1.3  com/sockcmd/routed,3.1,8943 10/8/89 16:59:43 */
/* 
 * COMPONENT_NAME: TCPIP af.h
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
 *	af.h	5.7 (Berkeley) 6/1/90
 */

/*
 * Routing table management daemon.
 */

/*
 * Per address family routines.
 */
struct afswitch {
	int	(*af_hash)();		/* returns keys based on address */
	int	(*af_netmatch)();	/* verifies net # matching */
	int	(*af_output)();		/* interprets address for sending */
	int	(*af_portmatch)();	/* packet from some other router? */
	int	(*af_portcheck)();	/* packet from privileged peer? */
	int	(*af_checkhost)();	/* tells if address is valid */
	int	(*af_rtflags)();	/* get flags for route (host or net) */
	int	(*af_sendroute)();	/* check bounds of subnet broadcast */
	int	(*af_canon)();		/* canonicalize address for compares */
	char	*(*af_format)();	/* convert address to string */
};

/*
 * Structure returned by af_hash routines.
 */
struct afhash {
	u_int	afh_hosthash;		/* host based hash */
	u_int	afh_nethash;		/* network based hash */
};

extern struct afswitch afswitch[];		/* table proper */
int	af_max;				/* number of entries in table */
