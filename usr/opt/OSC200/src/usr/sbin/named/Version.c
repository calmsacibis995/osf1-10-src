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
static char	*sccsid = "@(#)$RCSfile: Version.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:14:11 $";
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

*/
/* 
 * COMPONENT_NAME: TCPIP Version.c
 * 
 * FUNCTIONS: Child, Copyright 
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
 */
/*
 * Copyright (c) 1986, 1987 Regents of the University of California.
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

 */

#ifndef lint

#endif /* not lint */

char Version[] = "named 4.8.3.1 %VERSION%\n\t%WHOANDWHERE%\n";

#ifdef COMMENT

SCCS/s.Version.c:

D 4.8.3   90/06/27 17:05:07	bloom  37      35      00031/00028/00079
Version distributed with 4.4BSD Alpha tape (June 1990)

D 4.8.2   89/09/18 13:57:11	bloom	35	34	00020/00014/00087
Interim fixes release

D 4.8.1   89/02/08 17:12:15	karels	34	33	00026/00017/00075
branch for 4.8.1

D 4.8	88/07/09 14:27:00	karels	33	28	00043/00031/00049
4.8 is here!

D 4.7	87/11/21 13:17:56	karels	28	25	00047/00029/00033
4.7.3 beta

D 4.6	87/07/21 12:15:52	karels	25	24	00000/00000/00062
4.6 declared stillborn

D 4.5	87/02/10 12:33:25	kjd	24	18	00000/00000/00062
February 1987, Network Release. Child (bind) grows up, parent (kevin) leaves home.

D 4.4	86/10/01 10:06:26	kjd	18	12	00020/00017/00042
October 1, 1986 Network Distribution

D 4.3	86/06/04 12:12:18	kjd	12	7	00015/00028/00044
Version distributed with 4.3BSD

D 4.2	86/04/30 20:57:16	kjd	7	1	00056/00000/00016
Network distribution Freeze and one more version until 4.3BSD

D 1.1	86/04/30 19:30:00	kjd	1	0	00016/00000/00000
date and time created 86/04/30 19:30:00 by kjd

code versions:

Makefile
	Makefile	5.2 (Berkeley) 6/19/90
db.h
	db.h	4.16 (Berkeley) 6/1/90
ns.h
	ns.h	4.31 (Berkeley) 6/1/90
pathnames.h
	pathnames.h	5.4 (Berkeley) 6/1/90
db_dump.c
	db_dump.c	4.30 (Berkeley) 6/1/90
db_glue.c
	db_glue.c	4.4 (Berkeley) 6/1/90
db_load.c
	db_load.c	4.37 (Berkeley) 6/1/90
db_lookup.c
	db_lookup.c	4.17 (Berkeley) 6/1/90
db_reload.c
	db_reload.c	4.21 (Berkeley) 6/1/90
db_save.c
	db_save.c	4.15 (Berkeley) 6/1/90
db_update.c
	db_update.c	4.26 (Berkeley) 6/1/90
ns_forw.c
	ns_forw.c	4.30 (Berkeley) 6/27/90
ns_init.c
	ns_init.c	4.35 (Berkeley) 6/27/90
ns_main.c
	 Copyright (c) 1986, 1989, 1990 Regents of the University of California.\n\
	ns_main.c	4.49 (Berkeley) 6/27/90
ns_maint.c
	ns_maint.c	4.37 (Berkeley) 6/1/90
ns_req.c
	ns_req.c	4.44 (Berkeley) 6/27/90
ns_resp.c
	ns_resp.c	4.63 (Berkeley) 6/1/90
ns_sort.c
	ns_sort.c	4.8 (Berkeley) 6/1/90
ns_stats.c
	ns_stats.c	4.10 (Berkeley) 6/27/90
named-xfer.c
	 Copyright (c) 1988, 1990 Regents of the University of California.\n\
	named-xfer.c	4.14 (Berkeley) 6/27/90
newvers.sh
	newvers.sh	4.6 (Berkeley) 5/11/89

#endif COMMENT
