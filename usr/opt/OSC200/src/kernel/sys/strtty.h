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
 * @(#)$RCSfile: strtty.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/07/15 18:52:32 $
 */
/* Generic STREAMS tty structure for use by STREAMS serial device drivers */

#ifndef _SYS_STRTTY_H_
#define _SYS_STRTTY_H_

struct t_buf
{	
	mblk_t		*bu_bp;		/* message block pointer */
	unsigned char	*bu_ptr;	/* data buffer pointer */
	ushort		bu_cnt;		/* data buffer char count */
};

struct strtty
{
	struct t_buf	t_in;		/* input buffer information */
	struct t_buf	t_out;		/* output buffer info */
	queue_t		*t_rdqp;	/* read queue pointer */
	mblk_t		*t_ioctlp;	/* ioctl block pointer */
	mblk_t		*t_lbuf;	/* pointer to a large data buffer */
	dev_t		t_dev;		/* device number */
	struct  termios t_termios;      /* termios state */
#define t_iflag         t_termios.c_iflag
#define t_oflag         t_termios.c_oflag
#define t_cflag         t_termios.c_cflag
#define t_lflag         t_termios.c_lflag
#define t_min           t_termios.c_cc[VMIN]
#define t_time          t_termios.c_cc[VTIME]
#define t_cc            t_termios.c_cc
#define t_ispeed        t_termios.c_ispeed
#define t_ospeed        t_termios.c_ospeed
	short		t_state;	/* internal state flags */
	char		t_line;		/* line discipline */
	char		t_datat;	/* more internal state flags */
};

#endif
