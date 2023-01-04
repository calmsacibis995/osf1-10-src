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
 * @(#)$RCSfile: strtty_gen.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:08:23 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#ifndef _STRTTY_GEN_H_
#define _STRTTY_GEN_H_

#include <sys/stream.h>
#include <sys/termios.h>

typedef struct stt_s {
    dev_t		t_dev;  	    /* maj+min */
    int			t_state;	    /* state bits */
    queue_t		*t_queue; 	    /* pointer to read queue */
    mblk_t		*t_inmsg; 	    /* current input message */
    mblk_t		*t_outmsg; 	    /* current output message */
    mblk_t		*t_qioctl;	    /* queued (not yet proc.) ioctl */
    mblk_t		*t_curioctl; 	    /* currently stalled ioctl */
    int			t_lostinput; 	    /* discarded input characters */
    int			t_close_timer;	    /* timeout id for close */
    int			t_timer;	    /* timer for delay and tcsbrk */
    char		*t_private;	    /* port-specific data */
    int			t_private_i;	    /* port-specific data */
    int			(*t_start_proc)();  /* port-specific start output */
    void		(*t_stop_proc)();   /* port-specific stop output */
    void		(*t_starti_proc)(); /* port-specific start input */
    void		(*t_stopi_proc)();  /* port-specific stop input */
    int			(*t_ioctl_proc)();  /* port-specific ioctl */
    struct termios	t_termios;	    /* termios settings */
    struct tt_timetab	*t_tmtab;	    /* timeout control structure */
    struct tt_timejob	*t_timeout_ready;   /* ready timeout job, if any */
    int			t_timeout_state;    /* timeout state */
    decl_simple_lock_data(,t_timeout_lock)  /* timeout lock */
} STT, * STTP;

#define	t_iflag		t_termios.c_iflag
#define	t_oflag		t_termios.c_oflag
#define	t_cflag		t_termios.c_cflag
#define	t_lflag		t_termios.c_lflag
#define	t_cc		t_termios.c_cc
#define t_ispeed	t_termios.c_ispeed
#define t_ospeed	t_termios.c_ospeed

extern STTP           Null_tp;

int
strtty_rsrv(                    /* Read service routine */
            queue_t *q
           );

int
strtty_wput(                    /* Write put routine */
            register queue_t *q,
            register mblk_t *mp
           );

int
strtty_wsrv(                    /* Write service */
            queue_t *q
           );

void 
strtty_flush(
        register STTP tp,
        int rw,
        int generate
	);

void 
strtty_sendinputif(
              register STTP tp
              );

void 
strtty_sendinput(
            register STTP tp
            );

mblk_t * 
strtty_allocin(
	  register STTP tp
	  );

int 
strtty_startoutput( 
		register STTP tp, 
		register mblk_t *mp
	       );

int 
strtty_rstrt(
	register STTP tp
	);

int 
strtty_breakin(
	     register STTP tp,
	     register int c
	     );

void 
strtty_wtimeout( 
	register STTP tp
	);

int 
strtty_modem( 
	queue_t *q, 
	int carr
	);



/* Flags for t_state field of STT */
#define S_TIMEOUT	0x00000001	/* timeout outstanding */
#define S_WOPEN		0x00000002	/* waiting for open to complete */
#define S_ISOPEN	0x00000004	/* device is open */
#define S_FLUSH		0x00000008	/* outq has been flushed */
#define S_CARR_ON	0x00000010	/* software copy of carrier-present */
#define S_BUSY		0x00000020	/* output in progress */
#define S_ASLEEP	0x00000040	/* wakeup when output done */
#define S_XCLUDE	0x00000080	/* exclusive-use flag against open */
#define S_TTSTOP	0x00000100	/* output stopped by ctl-s */
#define S_TBLOCK	0x00000200	/* tandem queue blocked */
#define S_NBIO		0x00000400	/* tty in non-blocking mode */
#define S_ASYNC		0x00000800	/* tty in async i/o mode */
#define S_ISTOP		0x00001000	/* input stopped (via M_STOPI) */
#define S_DOBREAK	0x00002000	/* send a break */
#define S_TCSBRKON	0x00004000	/* first part of TCSBRK */
#define S_TCSBRKOFF	0x00008000	/* second part of TCSBRK */
#define S_WAITINMSG	0x00010000	/* waiting for alloc of t_inmsg */
#define S_CLOSING	0x00020000	/* device close started */
#define S_INUSE		0x00040000	/* device in use */
#define S_HUPCLS	0x00080000	/* hang up on last close */

/* Minimum size of buffer allocated by strtty_allocin() (actual buffer
 * will probably be much larger than this).
 */
#define INTERRUPT_BSIZE	10
#endif	/* _STRTTY_GEN_H_ */
