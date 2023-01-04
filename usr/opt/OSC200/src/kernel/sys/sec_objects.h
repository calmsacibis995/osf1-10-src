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
 * @(#)$RCSfile: sec_objects.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:07:31 $
 */
#ifndef __SEC_OBJECTS__
#define __SEC_OBJECTS__ 1

/* Kernel objects */

#define	OT_REGULAR		1	/* regular file */
#define	OT_DIRECTORY		2	/* directory */
#define	OT_DEVICE		3	/* block, char device */
#define	OT_PIPE			4	/* named, unnamed pipe */
#define	OT_PROCESS		5	/* self explanatory */
#define	OT_MEMORY		6	/*	""	*/
#define	OT_STREAM		7	/* V.3 stream */
#define	OT_SOCKET		8	/* Berkeley socket */
#define	OT_SHARED_MEMORY	9	/* shared memory segment */
#define	OT_SEMAPHORE		10	/* semaphore */
#define	OT_MESSAGE_QUEUE	11	/* message queue */
#define OT_FILE_DESCR		12	/* file descriptor */
#define	OT_SYMLINK		13	/* symbolic link */

/* Authentication database objects */

#define	OT_PWD			14	/* /etc/passwd */
#define	OT_GRP			15	/* /etc/group */
#define	OT_PRPWD		16	/* Protected Password Database */
#define	OT_TERM_CNTL		17	/* Terminal Control Database */
#define	OT_FILE_CNTL		18	/* File Control Database */
#define	OT_DFLT_CNTL		19	/* System Default Database */
#define	OT_SUBSYS		20	/* Subsystem Database */
#define OT_DEV_ASG		21	/* Device Assignment Database */
#define OT_LP_CNTL		22	/* Printer Control Database */

/* Window System objects */

#define OT_WINDOW		23	/* top level window */
#define OT_OVERRIDE_WINDOW	24	/* override redirect window */
#define	OT_PIXMAP		25	/* pixmap */
#define	OT_COLORMAP		26	/* colormap */
#define	OT_PROPERTY		27	/* window property */
#define	OT_ATOM			28	/* window system atom */
#endif
