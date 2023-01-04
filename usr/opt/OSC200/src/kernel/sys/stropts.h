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
 * @(#)$RCSfile: stropts.h,v $ $Revision: 4.2.9.5 $ (DEC) $Date: 1993/12/15 22:12:12 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989-1991  Mentat Inc.
 **/

#ifndef	_SYS_STROPTS_H
#define	_SYS_STROPTS_H

#include <sys/ioctl.h>

/*
 * STREAMS ioctl commands - group 'S'
 */

#define	I_NREAD		_IO('S', 1) /* return the number of bytes in 1st msg */
#define	I_PUSH		_IO('S', 2) /* push module just below stream head */
#define	I_POP		_IO('S', 3) /* pop module below stream head */
#define	I_LOOK		_IO('S', 4) /* retrieve name of first stream module */
#define	I_FLUSH		_IO('S', 5) /* flush all input and/or output queues */
#define	I_SRDOPT	_IO('S', 6) /* set the read mode */
#define	I_GRDOPT	_IO('S', 7) /* get the current read mode */
#define	I_STR		_IO('S', 8) /* create an internal ioctl message */
#define	I_SETSIG	_IO('S', 9) /* request SIGPOLL signal on events */
#define	I_GETSIG	_IO('S',10) /* query the registered events */
#define	I_FIND		_IO('S',11) /* check for module in stream */
#define	I_LINK		_IO('S',12) /* connect stream under mux fd */
#define	I_UNLINK	_IO('S',13) /* disconnect two streams */
#define	I_ISASTREAM	_IO('S',14) /* identifies as stream/pipe/fifo */
#define	I_PEEK		_IO('S',15) /* peek at data on read queue */
#define	I_FDINSERT	_IO('S',16) /* create a message and send downstream */
#define	I_SENDFD	_IO('S',17) /* send an fd to a connected pipe stream */
#define	I_RECVFD	_IO('S',18) /* retrieve a file descriptor */
#define	I_FLUSHBAND	_IO('S',19) /* flush a particular input and/or output band */
#define	I_SWROPT	_IO('S',20) /* set the write mode */
#define	I_GWROPT	_IO('S',21) /* get the current write mode */
#define	I_LIST		_IO('S',22) /* get a list of all modules on a stream */
#define	I_ATMARK	_IO('S',23) /* is the next message is "marked"? */
#define	I_CKBAND	_IO('S',24) /* check for a message of a particular band */
#define	I_GETBAND	_IO('S',25) /* get the band of the next message */
#define	I_CANPUT	_IO('S',26) /* check to see if a message may be passed on a stream */
#define	I_SETCLTIME	_IO('S',27) /* set the close timeout wait */
#define	I_GETCLTIME	_IO('S',28) /* get the current close timeout wait */
#define	I_PLINK		_IO('S',29) /* persistently connect a stream under a mux */
#define	I_PUNLINK	_IO('S',30) /* disconnect a persistent link */
/* 31-39 available */
#define	I_GETMSG	_IO('S',40) /* getmsg() system call */
#define	I_PUTMSG	_IO('S',41) /* putmsg() system call */
#define	I_GETPMSG	_IO('S',42) /* getpmsg() system call */
#define	I_PUTPMSG	_IO('S',43) /* putpmsg() system call */
#define	I_PIPE		_IO('S',44) /* connect two streams as a pipe */
#define	I_FIFO		_IO('S',45) /* convert a stream into a FIFO */
/* 45-59 available */
#define I_STR_ATTR	_IO('S',60) /* SEC_BASE attribute versions of above. */
#define I_PEEK_ATTR	_IO('S',61) /* ... */
#define I_FDINSERT_ATTR	_IO('S',62)
#define I_SENDFD_ATTR	_IO('S',63)
#define I_RECVFD_ATTR	_IO('S',64)
#define I_GETMSG_ATTR	_IO('S',65)
#define I_PUTMSG_ATTR	_IO('S',66)
#define I_GETPMSG_ATTR	_IO('S',67)
#define I_PUTPMSG_ATTR	_IO('S',68)

/* priority message request on putmsg() or strpeek */
#define	RS_HIPRI	0x1

/* flags for getpmsg and putpmsg */
#define	MSG_HIPRI	RS_HIPRI
#define	MSG_BAND	0x2	/* Retrieve a message from a particular band */
#define	MSG_ANY		0x4	/* Retrieve a message from any band */

/* return values from getmsg(), 0 indicates all ok */
#define	MORECTL		0x1	/* more control info available */
#define	MOREDATA	0x2	/* more data available */

/* flush requests */
#define	FLUSHR		0x1		/* Flush the read queue */
#define	FLUSHW		0x2		/* Flush the write queue */
#define	FLUSHRW		(FLUSHW|FLUSHR)	/* Flush both */
#define	FLUSHBAND	0x40		/* Flush a particular band */

/* I_FLUSHBAND */
struct bandinfo {
	unsigned char	bi_pri;		/* Band to flush */
	int		bi_flag;	/* One of the above flush requests */
};

/* signal event masks */
#define	S_INPUT		0x1	/* A non-M_PCPROTO message has arrived */
#define	S_HIPRI		0x2	/* A priority (M_PCPROTO) message is available */
#define	S_OUTPUT	0x4	/* The write queue is no longer full */
#define	S_MSG		0x8	/* A signal message has reached the front of read queue */
#define	S_RDNORM	0x10	/* A non-priority message is available */
#define	S_RDBAND	0x20	/* A banded messsage is available */
#define	S_WRNORM	0x40	/* Same as S_OUTPUT */
#define	S_WRBAND	0x80	/* A priority band exists and is writable */
#define	S_ERROR		0x100	/* Error message has arrived */
#define	S_HANGUP	0x200	/* Hangup message has arrived */
#define	S_BANDURG	0x400	/* Use SIGURG instead of SIGPOLL on S_RDBAND signals */

/* flags for I_ATMARK */
#define	ANYMARK		0x1	/* Check if message is marked */
#define	LASTMARK	0x2	/* Check if this is the only message marked */

/* read mode bits for I_S|GRDOPT; choose one of the following */
#define	RNORM		0x1	/* byte-stream mode, default */
#define	RMSGD		0x2	/* message-discard mode */
#define	RMSGN		0x4	/* message-nondiscard mode */
#define	RFILL		0x8	/* fill read buffer mode (PSE private) */

/* More read modes, these are bitwise or'ed with the modes above */
#define	RPROTNORM	0x10	/* Normal handling of M_PROTO/M_PCPROTO, default */
#define	RPROTDIS	0x20	/* Discard M_PROTO/M_PCPROTO message blocks */
#define	RPROTDAT	0x40	/* Convert M_PROTO/M_PCPROTO into M_DATA */
#define	RPROTCOMPRESS	0x80	/* Compress M_DATA if messages allow */

/* write modes for I_S|GWROPT */
#define	SNDZERO		0x1	/* Send a zero-length message downstream on a write of zero bytes */

/* Remove all lower streams on I_UNLINK/I_PUNLINK */
#define	MUXID_ALL	(-1)

struct	strbuf {
	int	maxlen;		/* max buffer length */
	int	len;		/* length of data */
	char *	buf;		/* pointer to buffer */
};

/* structure of ioctl data on I_FDINSERT */
struct	strfdinsert {
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	long		flags;	/* type of message, 0 or RS_HIPRI */
	int		fildes;	/* fd of other stream */
	int		offset;	/* where to put other stream read qp */
};

/* NOTE: this should really be located in some common area for stream.h
 * and stropts.h.
 */
#ifndef FMNAMESZ
#define FMNAMESZ	16
#endif

struct str_mlist {
	char	l_name[FMNAMESZ + 1];
};

/* I_LIST structures */
struct str_list {
	int	sl_nmods;	/* number of modules in sl_modlist array */
	struct str_mlist * sl_modlist;
};

/* I_PEEK structure */
struct	strpeek {
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	long		flags;	/* if RS_HIPRI, get priority messages only */
};

/* structure for getpmsg and putpmsg */
struct	strpmsg {
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	int		band;
	long		flags;
};

/* structure of ioctl data on I_RECVFD */
struct	strrecvfd {
	int		fd;	/* new file descriptor */
	uid_t		uid;	/* user id of sending stream */
	gid_t		gid;
	char		fill[8];
};

/* structure of ioctl data on I_STR */
struct	strioctl {
	int	ic_cmd;		/* downstream command */
	int	ic_timout;	/* ACK/NAK timeout */
	int	ic_len;		/* length of data arg */
	char *	ic_dp;		/* ptr to data arg */
};

/* structure of ioctl data for I_FDINSERT_ATTR */
struct strfdinsert_attr {
        struct strbuf   ctlbuf;
        struct strbuf   databuf;
        long            flags;  /* type of message, 0 or RS_HIPRI */
        int             fildes; /* fd of other stream */
        int             offset; /* where to put other stream read qp */
        struct strbuf   attrbuf;
};

/* structure of ioctl data for I_PEEK_ATTR */
struct strpeek_attr {
        struct strbuf   ctlbuf;
        struct strbuf   databuf;
        long            flags;  /* if RS_HIPRI, get priority messages only */
        struct strbuf   attrbuf;
};

/* structure for getpmsg_attr and putpmsg_attr */
struct	strpmsg_attr {
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	int		band;
	long		flags;
        struct strbuf   attrbuf;
};

/* structure of ioctl data for I_RECVFD_ATTR */
struct strrecvfd_attr {
        int             fd;     /* new file descriptor */
        uid_t		uid;    /* user id of sending stream */
        gid_t		gid;
        struct strbuf   attrbuf;
};

/* structure of ioctl data for I_SENDFD_ATTR */
struct strsendfd_attr {
        int             fd;     /* file descriptor to be passed */
        struct strbuf   attrbuf;
};

/* structure of ioctl data for I_STR_ATTR */
struct strioctl_attr {
        int     ic_cmd;         /* downstream command */
        int     ic_timout;      /* ACK/NAK timeout */
        int     ic_len;         /* length of data arg */
        char *  ic_dp;          /* ptr to data arg */
        struct strbuf   attrbuf;
};

#ifndef	_KERNEL
#ifdef	_NO_PROTO
extern int getmsg();
extern int putmsg();
extern int getpmsg();
extern int putpmsg();
extern int isastream();
extern int fattach();
extern int fdetach();
#else
_BEGIN_CPLUSPLUS
extern int getmsg(int, struct strbuf *, struct strbuf *, int *);
extern int putmsg(int, struct strbuf *, struct strbuf *, int);
extern int getpmsg(int, struct strbuf *, struct strbuf *, int *, int *);
extern int putpmsg(int, struct strbuf *, struct strbuf *, int, int);
extern int isastream(int);
extern int fattach(int, const char *);
extern int fdetach(const char *);
_END_CPLUSPLUS
#endif
#endif
#endif	/* _SYS_STROPTS_H */
