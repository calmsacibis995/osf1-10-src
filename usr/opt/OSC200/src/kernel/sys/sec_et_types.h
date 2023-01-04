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

/* These symbol definitions support compatibility with programs
 * that use the old Secureware audit function calls.  This file
 * provides backwards compartibility for existing applications
 * only and all new applications should use currently supported
 * functions.
 *
 * In systems that use Secureware audit, the symbol definitions
 * below are in audit.h
 */

#ifndef _SEC_ET_TYPES_H_
#define _SEC_ET_TYPES_H_

/* Event Types */

#define	ET_NOEVENT		0	/* not an audited event */
#define	ET_BOOT_DOWN		1	/* startup/shutdown */
#define	ET_LOGIN		2	/* login events */
#define	ET_PROCESS		3	/* process create/delete */
#define	ET_OBJECT_AVAIL		4	/* make available */
#define	ET_OBJECT_MAP		5	/* map to subject */
#define	ET_OBJECT_MOD		6	/* modify object */
#define	ET_OBJECT_UNAV		7	/* object unavailable */
#define	ET_OBJECT_CREAT		8	/* create object */
#define	ET_OBJECT_DEL		9	/* delete object */
#define	ET_DAC_CHANGE		10	/* change modes */
#define	ET_ACCESS_DENIAL	11	/* access denied */
#define	ET_SYS_ADMIN		12	/* system admin actions */
#define	ET_INSUFF_PRIV		13	/* insufficient privilege */
#define	ET_RES_DENIAL		14	/* resource denial */
#define	ET_IPC			15	/* inter-process comm */
#define	ET_PROCESS_MOD		16	/* change process control fields */
#define	ET_AUDIT		17	/* audit subsystem events */
#define	ET_SUBSYSTEM		18	/* special subsystem events */
#define ET_PRIVILEGE		19	/* use of privilege event */
#define ET_AUTHORIZATION	20	/* use of authorization event */
#define ET_SEC_LEVEL		21	/* set security level events */
#define ET_WINDOW_SYSTEM	22	/* server startup, shutdown, reset */
#define ET_WINDOW_SUBJECT	23	/* client startup, shutdown, attrs */
#define ET_WINDOW_OBJECT	24	/* object creation, deletion, changes */
#define ET_WINDOW_DATAMOVE	25	/* interwindow data moves */

#endif
