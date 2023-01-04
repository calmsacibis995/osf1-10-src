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
 *	@(#)$RCSfile: snmp.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/06/04 08:32:51 $
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
 * COMPONENT_NAME: TCPIP snmp.h
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

#if     defined(AGENT_SNMP) || defined(AGENT_SGMP)

#define CORE_VALUE	0
#define SNMPSTRLEN	128		/* max length of a string */
#define SNMPMAXPKT	484		/* max length of a packet */

#define AGENT_REG	0x01		/* var registration from an agent */
#define AGENT_REQ	0x02		/* to request a var from an agent */
#define AGENT_ERR	0x03		/* error was encountered */
#define AGENT_RSP	0x04		/* response from an agent */
#define AGENT_CNF	0x05		/* confirmation to an agent */

/* primitive types */
#define INT		0		/* integer */
#define STR		1		/* octet string */
#define OBJ		2		/* object identifier */
#define EMPTY		3		/* empty */
#define IPADD		4		/* net address */
#define CNTR		5		/* counter */
#define GAUGE		6		/* gauge */
#define TIME		7		/* time */
#define OPAQUE		8		/* opaque */

struct mibtbl {
	char length;
	char object[11];
	int (*function)();
};

#endif  /* defined(AGENT_SNMP) || defined(AGENT_SGMP) */
