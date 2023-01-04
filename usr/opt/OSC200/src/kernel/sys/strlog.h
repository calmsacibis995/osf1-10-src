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
 *	@(#)$RCSfile: strlog.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/12 19:03:45 $
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
/** Copyright (c) 1989  Mentat Inc.
 ** strlog.h 1.2, last change 21/20/89
 **/

#ifndef	_STRLOG_H
#define	_STRLOG_H

/*
 * STREAMS logging.
 */

struct log_ctl {
	short	mid;
	short	sid;
	char	level;
	short	flags;
	long	ltime;
	long	ttime;
	int	seq_no;
};

#define	SL_FATAL	0x1	/* Fatal error */
#define	SL_NOTIFY	0x2	/* Notify the system administrator */
#define	SL_ERROR	0x4	/* Pass message to error logger */
#define	SL_TRACE	0x8	/* Pass message to tracer */
#define SL_CONSOLE      0x10    /* Print the message on the console */
#define SL_WARN         0x20    /* Warning */
#define SL_NOTE         0x40    /* Notice this message */

struct trace_ids {
	short	ti_mid;
	short	ti_sid;
	char	ti_level;
	short	ti_flags;
};

#define	I_TRCLOG	1
#define	I_ERRLOG	2

#define	LOGMSGSZ	128
#endif
