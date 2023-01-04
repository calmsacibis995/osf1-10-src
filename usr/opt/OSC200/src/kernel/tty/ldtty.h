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
 * @(#)$RCSfile: ldtty.h,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/07 22:54:53 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#ifndef	_SYS_LDTTY_H_
#define _SYS_LDTTY_H_

#include <sys/types.h>
#include <sys/limits.h>
#include <sys/ioctl.h>		/* for struct winsize */
#include <sys/eucioctl.h>	/* for multibyte processing */
#include <sys/termios.h>
#include <sys/termio.h>		/* for compat */
#include <sys/stream.h>
#include <sys/sysconfig.h>

/*
 * Per-tty structure for "ldtty" line discipline STREAMS module
 *
 * Contains only the info pertaining to things that interest the
 * line discipline.  No hardware-specific stuff here.
 */
struct ldtty {
	queue_t 	*t_queue;	/* pointer to our READ queue */
	mblk_t		*t_rawbuf;	/* input buffer */
	mblk_t		*t_rawtail;	/* tail of input buffer */
	int		t_rawcc;	/* count in input buffer */
	mblk_t		*t_unsent;	/* ldtty_sendraw() optimization */
	int		t_unsent_ndx;	/* ldtty_sendraw() optimization */
	int		t_shcc;		/* size of shead copy of rawbuf */
	mblk_t 		*t_outbuf;	/* buffer for output characters */
	mblk_t		*t_sparebuf;	/* spare for mem. tricks: no b_cont */
#if MACH_ASSERT
	int		t_sparehit;
	int		t_sparemiss;
	int		t_rawtrace;
#endif
	unsigned long 	t_state;	/* line discipline state */
	int		t_intimeout_posted; /* booolean flag for read srvp -- */
					    /* protected by t_intimeout_lock */
	int 		t_intimeout;	/* timeout() id for reads */
	int 		t_outbid;	/* bufcall() id for outmsg */
	int 		t_ackbid;	/* bufcall() id for ioctl acks */
	int 		t_hupbid;	/* bufcall() id for M_HANGUP */
	int		t_col;		/* current output column */
	int		t_rocount;	/* chars echoed since last output */
	int 		t_rocol;	/* first echo column */
	struct winsize 	t_winsize;	/* window size */
	struct termios 	t_termios;	/* termios state */
	tcflag_t	t_flags;	/* for compat */
	int		t_ioctl_cmd;	/* original ioctl command */
	mblk_t		*t_qioctl;	/* queued ioctl */
	long		t_shad_time;	/* Value of t_cc[VTIME] in ticks */
	eucioc_t	t_cswidth;	/* character widths */
	int		t_codeset;	/* current input EUC codeset */
	int		t_eucleft;	/* bytes left for input character */
	int		t_eucind;	/* index to eucbytes */
	uchar_t		t_eucbytes[8];	/* place to build input EUC character */
	int		t_out_codeset;	/* current output EUC codeset */
	int		t_out_eucleft;	/* bytes left for output character */
	int		t_out_eucind;	/* index to out_eucbytes */
	uchar_t		t_out_eucbytes[8]; /* place to build output EUC char */
	decl_simple_lock_data(,t_intimeout_lock)
};

#define	t_iflag		t_termios.c_iflag
#define	t_oflag		t_termios.c_oflag
#define	t_cflag		t_termios.c_cflag
#define	t_lflag		t_termios.c_lflag
#define	t_cc		t_termios.c_cc
#define t_ispeed	t_termios.c_ispeed
#define t_ospeed	t_termios.c_ospeed

/*
 * The following two values are chosen to be powers of 2 to
 * coincide with STREAMS message block sizes.  This is not absolutely
 * necessary, but may make STREAMS flow control work a little better
 */
#define LDTTYCHUNKSIZE 64	/* size of input mblks */
#define LDTTYMAX 768		/* STREAMS tty output buffer size */

/*
 * Default hi and lo water for streams flow control.
 */
#define LDTTY_STREAMS_HIWAT		2048
#define LDTTY_STREAMS_LOWAT		(LDTTY_STREAMS_HIWAT - LDTTYMAX)

/*
 * Following two values are used to time start/stop transmission 
 * if IXOFF is set.
 */
#define LDTTYHIWAT (MAX_INPUT/2)	/* hi watermark */
#define LDTTYLOWAT (MAX_INPUT/5)	/* low watermark */

/*
 * Flag bits returned by ldtty_input -- used after processing of a full
 * message to direct actions of ldtty_post_input().
 */
#define T_POST_WAKEUP	0x01       	/* ldtty_wakeup() required */
#define T_POST_START	0x02       	/* ldtty_start() required */
#define T_POST_TIMER	0x04       	/* need to start timer */
#define T_POST_BACKUP	0x08       	/* rawq full -- let data backup on q */
#define T_POST_FLUSH	0x10       	/* input flushed -- ignore prev. bits */

/* internal state bits */
#define TS_TIMEOUT	0x00000001	/* delay timeout in progress */
#define TS_WOPEN	0x00000002	/* waiting for open to complete */
#define TS_ISOPEN	0x00000004	/* device is open */
#define TS_RAWBACKUP	0x00000008	/* raw input clogged temporarily */
#define TS_NOCANON	0x00000010	/* no input processing */
#define TS_TTSTOP	0x00000020	/* output stopped by ctl-s */
#define TS_VTIME_FLAG	0x00000040	/* first time through read() */
#define TS_TBLOCK	0x00000080	/* tandem queue blocked */
#define TS_TYPEN	0x00000100	/* retyping suspended input (PENDIN) */
#define TS_WAITOUTBUF	0x00000200	/* waiting for bufcall on outbuf */
#define TS_WAITOUTPUT	0x00000400	/* waiting for write side canput */
#define TS_WAITEUC	0x00000800	/* full EUC char waiting for outbuf */
#define TS_ASLEEP	0x00001000	/* waiting for output to drain */

/* state for intra-line fancy editing work */
#define TS_BKSL		0x00002000	/* state for lowercase \ work */
#define TS_ERASE	0x00004000	/* within a \.../ for PRTRUB */
#define TS_LNCH		0x00008000	/* next character is literal */
#define TS_CNTTB	0x00010000	/* counting tabwidth, lv. FLUSHO alone*/
#define	TS_LOCAL	(TS_BKSL|TS_ERASE|TS_LNCH|TS_TYPEN|TS_CNTTB)

#define TS_INTIMEOUT	0x00020000	/* an input timeout is active. */
#define TS_MBENABLED	0x00040000	/* multibyte EUC enabled */
#define TS_MINSAT	0x00080000	/* VMIN satisfied; no read yet */

/* define partab character types */
#define ORDINARY	0
#define CONTROL		1
#define BACKSPACE	2
#define NEWLINE		3
#define TAB		4
#define VTAB		5
#define RETURN		6
#define FF		7

#define ldtty_breakc(c)							\
	(c == '\n' || CCEQ(cc[VEOF], c) ||				\
	 CCEQ(cc[VEOL], c) || CCEQ(cc[VEOL2], c))

#define ldtty_mbenabled(tp)	((tp)->t_state & TS_MBENABLED)

#define ldtty_msgdsize(mp) 	((mp) ? msgdsize(mp) : 0)

#define ldtty_newrawbuf(tp, mp) do {					\
	(tp)->t_rawbuf = (tp)->t_rawtail = (tp)->t_unsent = (mp);	\
	(tp)->t_unsent_ndx = ((tp)->t_rawtail->b_wptr - 		\
			(tp)->t_rawtail->b_datap->db_base);		\
} while (0)
	

#define ldtty_bufreset(tp) do {						\
	mblk_t	*mp;							\
	if (mp = unlinkb((tp)->t_rawbuf))				\
		freemsg(mp);						\
	(tp)->t_rawbuf->b_rptr = (tp)->t_rawbuf->b_wptr =		\
		(tp)->t_rawbuf->b_datap->db_base;			\
	(tp)->t_rawcc = 0;						\
	(tp)->t_unsent = (tp)->t_rawtail = (tp)->t_rawbuf;		\
	(tp)->t_unsent_ndx = 0;						\
} while (0)

#define ldtty_bufclr(tp) do {						\
	(tp)->t_rawbuf = (tp)->t_rawtail = NULL;			\
	(tp)->t_rawcc = 0;						\
} while (0)

#define ldtty_iocack_msg(iocp, count, mp) do {				\
	(mp)->b_datap->db_type = M_IOCACK;				\
	iocp->ioc_error = 0;						\
	if (((iocp)->ioc_count = (count)) == 0 && (mp)->b_cont) {	\
		freemsg((mp)->b_cont);					\
		(mp)->b_cont = 0;					\
	}								\
} while (0)

#define ldtty_iocnak_msg(iocp, error, mp) do {				\
	(mp)->b_datap->db_type = M_IOCNAK;				\
	(iocp)->ioc_error = (error);					\
	if ((mp)->b_cont) {						\
		freemsg((mp)->b_cont);					\
		(mp)->b_cont = 0;					\
	}								\
} while (0)

#define ldtty_need_setopts(tp1, tp2)					\
	(((tp1)->c_lflag & (ICANON|TOSTOP)) !=				\
	 ((tp2)->c_lflag & (ICANON|TOSTOP)))


/*
 * Macros for raw input timeouts.
 */

#define ldtty_set_intimeout(tp, tmo) do {				\
	ldtty_unset_intimeout(tp);					\
	(tp)->t_state |= TS_INTIMEOUT;					\
	(tp)->t_intimeout = timeout(ldtty_intimeout, (tp), tmo);	\
} while (0)

#define ldtty_unset_intimeout(tp) do {					\
	int	s;							\
									\
	(tp)->t_state &= ~TS_INTIMEOUT;					\
	if ((tp)->t_intimeout) {					\
		untimeout((tp)->t_intimeout);				\
		(tp)->t_intimeout = 0;					\
	}								\
	s = spltty();							\
	simple_lock(&(tp)->t_intimeout_lock);				\
	(tp)->t_intimeout_posted = 0;					\
	simple_unlock(&(tp)->t_intimeout_lock);				\
	splx(s);							\
} while (0)

#define ldtty_post_intimeout(tp) do {					\
	int s = spltty();						\
	simple_lock(&(tp)->t_intimeout_lock);				\
	(tp)->t_intimeout_posted = 1;					\
	simple_unlock(&(tp)->t_intimeout_lock);				\
	splx(s);							\
} while (0)

#define ldtty_check_intimeout(tp, retp) do {				\
	if (tp->t_state & TS_INTIMEOUT) {				\
		int s = spltty();					\
		simple_lock(&(tp)->t_intimeout_lock);			\
		if ((tp)->t_intimeout_posted == 1) {			\
			*(retp) = 1;					\
			(tp)->t_intimeout_posted = 0;			\
			(tp)->t_intimeout = 0;				\
			(tp)->t_state &= ~TS_INTIMEOUT;			\
		}							\
		simple_unlock(&(tp)->t_intimeout_lock);			\
		splx(s);						\
	}								\
} while (0)

extern int tk_nin, tk_rawcc, tk_cancc, tk_nout;

/****						       ****
 ****	Prototypes from here to the end of the file.   ****
 ****						       ****/

void
euctty_echo(struct ldtty *, int);

void
euctty_erase(struct ldtty *);

void
euctty_kill(struct ldtty *);

int
euctty_out(struct ldtty *);

int
euctty_rocount(struct ldtty *);

PRIVATE_STATIC void
euctty_rub(struct ldtty *, unsigned char *);

PRIVATE_STATIC int
euctty_scrwidth(struct ldtty *, unsigned char *);

PRIVATE_STATIC int
euctty_state(struct ldtty *);

void
euctty_werase(struct ldtty *);

int
euctty_write(struct ldtty *, mblk_t *);

PRIVATE_STATIC int
euctty_writechar(struct ldtty *, unsigned char, int *);

PRIVATE_STATIC int
ldtty_b_to_m(unsigned char *, int, struct ldtty *);

PRIVATE_STATIC void
ldtty_tblock(struct ldtty *);

PRIVATE_STATIC void
ldtty_break(struct ldtty *, mblk_t *);

void
ldtty_bsd43_ioctl(struct ldtty *, queue_t *, mblk_t *);

PRIVATE_STATIC int
ldtty_close(queue_t *, int, cred_t *);

int
ldtty_compat_to_termios(struct ldtty *, mblk_t *, int, struct termios *);

int
ldtty_configure(sysconfig_op_t, str_config_t *, size_t, str_config_t *, size_t);

PRIVATE_STATIC void
ldtty_copymsg(mblk_t *, int, int, mblk_t *);

int
ldtty_getoutbuf(struct ldtty *);

PRIVATE_STATIC void
ldtty_mnotify(struct ldtty *, mblk_t *);

void
ldtty_echo(struct ldtty *, int);

int
ldtty_flush(struct ldtty *, int, int);

PRIVATE_STATIC void
ldtty_flush_mp(struct ldtty *, int, mblk_t *, mblk_t *);

PRIVATE_STATIC int
ldtty_flush_shead(struct ldtty *, mblk_t *);

PRIVATE_STATIC void
ldtty_info(struct ldtty *);

int
ldtty_input(struct ldtty *, int);

PRIVATE_STATIC void
ldtty_intimeout(struct ldtty *);

PRIVATE_STATIC void
ldtty_ioctl(struct ldtty *, queue_t *, mblk_t *);

PRIVATE_STATIC int
ldtty_ioctl_ack(struct ldtty *, queue_t *, mblk_t *);

PRIVATE_STATIC void
ldtty_ioctl_reack(struct ldtty *);

PRIVATE_STATIC int
ldtty_mctl(struct ldtty *, queue_t *, mblk_t *);

PRIVATE_STATIC int
ldtty_mhangup(struct ldtty *, queue_t *, mblk_t *);

PRIVATE_STATIC int
ldtty_need_flush(int, struct ldtty *, struct termios *);

PRIVATE_STATIC int
ldtty_open(queue_t *, dev_t *, int, int, cred_t *);

int
ldtty_output(struct ldtty *, int);

PRIVATE_STATIC void
ldtty_pend(struct ldtty *);

PRIVATE_STATIC void
ldtty_post_input(struct ldtty *, int);

PRIVATE_STATIC int
ldtty_putc(int, struct ldtty *);

PRIVATE_STATIC void
ldtty_putint(struct ldtty *, unsigned int, int);

PRIVATE_STATIC void
ldtty_putstr(struct ldtty *, char *);

PRIVATE_STATIC mblk_t *
ldtty_readdata(struct ldtty *, mblk_t *);

void
ldtty_retype(struct ldtty *);

PRIVATE_STATIC int
ldtty_rput(queue_t *, mblk_t *);

PRIVATE_STATIC int
ldtty_rsrv(queue_t *);

void
ldtty_rub(struct ldtty *, int);

void
ldtty_rubo(struct ldtty *, int);

PRIVATE_STATIC void
ldtty_sendcanon(struct ldtty *);

PRIVATE_STATIC void
ldtty_sendraw(struct ldtty *);

int
ldtty_start(struct ldtty *);

PRIVATE_STATIC void
ldtty_sti(struct ldtty *, mblk_t *);

int
ldtty_stuffc(int, struct ldtty *);

void
ldtty_svid_ioctl(struct ldtty *, queue_t *, mblk_t *);

PRIVATE_STATIC int
ldtty_swinsz(struct ldtty *, mblk_t *);

int
ldtty_unstuffc(struct ldtty *);

PRIVATE_STATIC void
ldtty_wakeup(struct ldtty *);

PRIVATE_STATIC int
ldtty_wput(queue_t *, mblk_t *);

PRIVATE_STATIC int
ldtty_write(struct ldtty *, mblk_t *);

PRIVATE_STATIC int
ldtty_wsrv(queue_t *);

#endif	/* _SYS_LDTTY_H_ */
