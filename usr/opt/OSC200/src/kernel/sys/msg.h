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
 *	@(#)$RCSfile: msg.h,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/12/07 21:59:40 $
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 


#ifndef _SYS_MSG_H_
#define _SYS_MSG_H_

#include <standards.h>
#include <sys/ipc.h>

#ifdef _XOPEN_SOURCE

/*
 *	Message Operation Flags.
 */

#define	MSG_NOERROR	010000	/* no error if big message */
#define	MSG_TAG		020000	/* tag messages with pid as type */

/*
 *	Structure Definitions
 */

/*
 *	There is one msg structure for each message that may be in the system.
 */

#ifdef _KERNEL

struct msg {
	struct msg     *msg_next;	/* ptr to next message on q */
	long            msg_type;	/* message type */
	long		msg_ts; 	/* message text size */
	caddr_t         msg_addr;	/* message text address */
};

struct msg_wait {
	struct msg_wait *msg_wait_next; /* ptr to next msg type on wait q */
	long		msg_wait_type;  /* message type waiting on */
	long		msg_wait_cnt;   /* num of msgs waiting on for type */
};

#endif /* _KERNEL */


/*
 *	There is one msg queue id data structure for each q in the system.
 */

struct msqid_ds {
	struct ipc_perm	msg_perm;	/* operation permission struct */
	struct msg	*msg_first;	/* ptr to first message on q */
	struct msg	*msg_last;	/* ptr to last message on q */
	ushort_t	msg_cbytes;	/* current # bytes on q */
	msgqnum_t	msg_qnum;	/* # of messages on q */
	msglen_t	msg_qbytes;	/* max # of bytes on q */
	pid_t		msg_lspid;	/* pid of last msgsnd */
	pid_t		msg_lrpid;	/* pid of last msgrcv */
	time_t		msg_stime;	/* last msgsnd time */
	time_t		msg_rtime;	/* last msgrcv time */
	time_t		msg_ctime;	/* last change time */
	struct msg_wait	*msg_wait_list; /* list of message types */
                                                 /* waiting on */
};

#ifndef _KERNEL
#ifdef _NO_PROTO
extern int msgctl();
extern int msgget();
extern int msgrcv();
extern int msgsnd();
#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
extern int msgget(key_t, int);
extern int msgrcv(int, void *, size_t, long, int);
extern int msgsnd(int, const void *, size_t, int);
extern int msgctl(int, int, struct msqid_ds *);
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */
#endif /* _KERNEL */

#endif /* _XOPEN_SOURCE */

#ifdef	_OSF_SOURCE
/*
 * 	Implementation Constants. 
 */

#define	PMSG	(PZERO + 2)	/* message facility sleep priority */

/*
 * 	Permission Definitions. 
 */
#define	MSG_R		IPC_R	/* read permission */
#define	MSG_W		IPC_W	/* write permission */

/*
 *	ipc_perm Mode Definitions.
 */
#define	MSG_RWAIT	01000	/* a reader is waiting for a message */
#define	MSG_WWAIT	02000	/* a writer is waiting to send */


/*
 *	User message buffer template for msgsnd and msgrcv system calls.
 */

struct msgbuf {
	mtyp_t	mtype;		/* message type */
	char	mtext[1];	/* message text */
};


/*
 *	Message information structure.
 */

struct msginfo {
	int	msgmax,	/* max message size */
		msgmnb,	/* max # bytes on queue */
		msgmni,	/* # of message queue identifiers */
	   	msgtql;	/* # of system message headers */
       	long	msg;	/* # of send and receive messages */
};


#endif	/* _OSF_SOURCE */

#ifdef _KERNEL
extern struct msqid_ds msgque[];
extern struct msginfo msginfo;
#endif  /* _KERNEL */

#endif  /* _SYS_MSG_H_ */

