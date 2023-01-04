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
 *	@(#)$RCSfile: syslog.h,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1993/12/15 22:12:17 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)syslog.h	7.16 (Berkeley) 6/28/90
 */

#ifndef	_SYS_SYSLOG_H_
#define _SYS_SYSLOG_H_

#include <standards.h>
#include <sys/syslog_pri.h>

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
#include <sys/types.h>
#include <sys/socket.h>

struct syslog_data {
	int	log_file;
	int	log_stat;
	char	*log_tag;
	int	log_mask;
	int	log_facility;
	struct sockaddr syslog_addr;
};
#define	SYSLOG_DATA_INIT {-1, 0, "syslog", 0xff, LOG_USER}
#endif	/* _REENTRANT || _THREAD_SAFE */

_BEGIN_CPLUSPLUS
extern int	openlog __((const char *, int, int));
extern int	syslog __((int, const char *, ...));
extern void	closelog __((void));
extern int	setlogmask __((int));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int	openlog_r __((const char *, int, int, struct syslog_data *));
extern int	syslog_r __((int, struct syslog_data *, const char *fmt, ...));
extern void	closelog_r __((struct syslog_data *));
extern int	setlogmask_r __((int, struct syslog_data *));
#endif	/* _REENTRANT || _THREAD_SAFE */
_END_CPLUSPLUS

#endif	/* _SYS_SYSLOG_H_ */
