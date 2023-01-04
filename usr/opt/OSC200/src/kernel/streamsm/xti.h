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
 * @(#)$RCSfile: xti.h,v $ $Revision: 4.2.11.4 $ (DEC) $Date: 1993/07/30 11:37:50 $
 */ 

/** Copyright (c) 1989  Mentat Inc.
 ** xti.h 1.1, last change 4/14/90
 **/

#ifndef _XTI_H
#define _XTI_H

/*
 * The following are the error codes needed by both the kernel
 * level transport providers and the user level library.
 */

#define	TBADADDR	1	/* incorrect addr format		*/
#define	TBADOPT		2	/* incorrect option format		*/
#define	TACCES		3	/* incorrect permissions		*/
#define	TBADF		4	/* illegal transport fd			*/
#define	TNOADDR		5	/* couldn't allocate addr		*/
#define	TOUTSTATE	6	/* out of state				*/
#define	TBADSEQ		7	/* bad call sequence number		*/
#define TSYSERR		8	/* system error				*/
#define	TLOOK		9	/* event requires attention		*/
#define	TBADDATA	10	/* illegal amount of data		*/
#define	TBUFOVFLW	11	/* buffer not large enough		*/
#define	TFLOW		12	/* flow control				*/
#define	TNODATA		13	/* no data				*/
#define	TNODIS		14	/* discon_ind not found of queue	*/
#define	TNOUDERR	15	/* unitdata error not found		*/
#define	TBADFLAG	16	/* bad flags				*/
#define	TNOREL		17	/* no ord rel found on queue		*/
#define	TNOTSUPPORT	18	/* primitive not supported		*/
#define	TSTATECHNG	19	/* state is in process of changing	*/
#define	TNOSTRUCTYPE	20	/* unsupported struct-type requested	*/
#define TBADNAME	21	/* invalid transport provider name	*/
#define	TBADQLEN	22	/* qlen is zero				*/
#define	TADDRBUSY	23	/* address in use			*/

/*
 * The following are the events returned from t_look()
 */

#define	T_LISTEN	0x0001	/* connection indication received	*/
#define	T_CONNECT	0x0002	/* connect conformation received	*/
#define	T_DATA		0x0004	/* normal data received			*/
#define	T_EXDATA	0x0008	/* expedited data received		*/
#define	T_DISCONNECT	0x0010	/* disconnect received			*/
#define	T_UDERR		0x0040	/* datagram error indication		*/
#define	T_ORDREL	0x0080	/* orderly release indication		*/
#define	T_GODATA	0x0100	/* sending normal data is again possible*/
#define	T_GOEXDATA	0x0200	/* sending expedited data in again possible */
#define	T_EVENTS	0x0400

/*
 * The following are the flag definitions needed by the
 * user level library routines.
 */

#define	T_MORE		0x01	/* more data				*/
#define	T_EXPEDITED	0x02	/* expedited data			*/
#define	T_NEGOTIATE	0x04	/* set opts				*/
#define	T_CHECK		0x08	/* check opts				*/
#define	T_DEFAULT	0x10	/* get default opts			*/
#define T_SUCCESS	0x20	/* successful				*/
#define	T_FAILURE	0x40	/* failure				*/

struct t_info {
	long	addr;		/* size of protocol address		*/
	long	options;	/* size of protocal options		*/
	long	tsdu;		/* size of max transport service data unit */
	long	etsdu;		/* size of max expedited data		*/
	long	connect;	/* max data for connection primitives	*/
	long	discon;		/* max data for disconnect primitives	*/
	long	servtype;	/* provider service type		*/
};

/*
 * Service types defines
 */

#define T_COTS		1	/* Connection-oriented transport service */
#define	T_COTS_ORD	2	/* Connection-oriented with orderly release */
#define	T_CLTS		3	/* Connectionless transport service	*/

/*
 * netbuf structure
 */

struct netbuf {
	unsigned int	maxlen;
	unsigned int	len;
	char *		buf;
};

/*
 * t_bind - format of the address and options arguments of bind
 */

struct t_bind {
	struct netbuf	addr;
	unsigned	qlen;
};

/*
 * options management structure
 */

struct t_optmgmt {
	struct netbuf	opt;
	long		flags;
};

/*
 * disconnect structure
 */

struct t_discon {
	struct netbuf	udata;
	int		reason;
	int		sequence;
};

/*
 * call structure
 */

struct t_call {
	struct netbuf	addr;
	struct netbuf	opt;
	struct netbuf	udata;
	int		sequence;
};

/*
 * datagram structure
 */

struct t_unitdata {
	struct netbuf	addr;
	struct netbuf	opt;
	struct netbuf	udata;
};

/*
 * unitdata error structure
 */

struct t_uderr {
	struct netbuf	addr;
	struct netbuf	opt;
	long		error;
};

/*
 * The following are structure types used when dynamically
 * allocating the above structure via alloc().
 */

#define	T_BIND_STR	1	/* struct t_bind			*/
#define T_OPTMGMT_STR	2	/* struct t_optmgmt			*/
#define	T_CALL_STR	3	/* struct t_call			*/
#define	T_DIS_STR	4	/* struct t_discon			*/
#define	T_UNITDATA_STR	5	/* struct t_unitdata			*/
#define	T_UDERROR_STR	6	/* struct t_uderr			*/
#define	T_INFO_STR	7	/* struct t_info			*/

/*
 * The following bits specify which fields of the above
 * structures should be allocated by t_alloc().
 */

#define	T_ADDR		0x01	/* address				*/
#define	T_OPT		0x02	/* options				*/
#define	T_UDATA		0x04	/* user data				*/
#define	T_ALL		0x07	/* all the above			*/

/*
 * The following are the states for the user.
 */

#define T_UNINIT	0	/* uninitialized state			*/
#define	T_UNBND		1	/* unbound				*/
#define	T_IDLE		2	/* idle					*/
#define	T_OUTCON	3	/* outgoing connection pending		*/
#define	T_INCON		4	/* incoming connection pending		*/
#define	T_DATAXFER	5	/* data transfer			*/
#define	T_OUTREL	6	/* outgoing orderly release		*/
#define	T_INREL		7	/* incoming orderly release		*/

/* general purpose defines */

#define	T_YES		1
#define	T_NO		0
#define	T_UNUSED	(-1)
#define	T_NULL		0
#define	T_ABSREQ	0x8000

	/* SPECIFIC ISO OPTIONS AND MANAGEMENT PARAMETERS */

/* definition of the ISO transport classes */

#define	T_CLASS0	0
#define	T_CLASS1	1
#define	T_CLASS2	2
#define	T_CLASS3	3
#define	T_CLASS4	4

/* definition of the priorities */

#define	T_PRITOP	0
#define	T_PRIHIGH	1
#define	T_PRIMID	2
#define	T_PRILOW	3
#define	T_PRIDFLT	4

/* definition of the protection levels */

#define	T_NOPROTECT		1
#define	T_PASSIVEPROTECT	2
#define	T_ACTIVEPROTECT		4

/* default value for the length of TPDU's */

#define	T_LTPDUDFLT	128

/* 
 * rate structure
 */

struct rate {
	long	targetvalue;	/* target value				*/
	long	minacceptvalue;	/* minimum acceptable value		*/
};

/*
 * reqvalue structure
 */

struct reqvalue {
	struct rate	called;		/* called rate			*/
	struct rate	calling;	/* calling rate			*/
};

/*
 * throughput structure
 */

struct thrpt {
	struct reqvalue	maxthrpt;	/* maximum throughput		*/
	struct reqvalue	avgthrpt;	/* average throughput		*/
};

/*
 * management structure
 */

struct management {
	short	dflt;		/* T_YES: the following parameters	*/
				/* are ignored, default vales are used;	*/
				/* T_NO: the following parameters	*/
				/* are used				*/
	int	ltpdu;		/* maximum length of TPDU (in octets)	*/
	short	reastime;	/* reassignment time (in seconds)	*/
	char	class;		/* preferred class;			*/
				/* value: T_CLASS - TCLASS4		*/
	char	altclass;	/* alternative class			*/
	char	extform;	/* extended format: T_YES or T_NO	*/
	char	flowctrl;	/* flow control: T_YES or T_NO		*/
	char	checksum;	/* checksum (cl. 4): T_YES or T_NO	*/
	char	netexp;		/* network expedited data: T_YES or T_NO*/
	char	netrecptcf;	/* receipt confirmation: T_YES or T_NO	*/ 
};

/*
 * ISO connection-oriented options
 */

struct isoco_options {
	struct thrpt	throughput;	/* throughput			*/
	struct reqvalue	transdel;	/* transit delay		*/
	struct rate	reserrorrate;	/* residual error rate		*/
	struct rate	transffailprob;	/* transfer failure problem	*/
	struct rate	estfailprob;	/* connection establishment failure */
	struct rate	relfailprob;	/* connection release failure	*/
	struct rate	estdelay;	/* connection establishment delay */
	struct rate	reldelay;	/* connection release delay	*/
	struct netbuf	connresil;	/* connection resilience	*/
	unsigned short	protection;	/* protection			*/
	short		priority;	/* priority			*/
	struct management mngmt;	/* management parameters	*/
	char		expd;		/* expedited data: T_YES or T_NO*/
};

/*
 * ISO connectionless options
 */

struct isocl_options {
	struct rate	transdel;	/* transit delay		*/
	struct rate	reserrorrate;	/* residual error rate		*/
	unsigned short	protection;	/* protection			*/
	short		priority;	/* priority			*/
};
	
		/* TCP SPECIFIC ENVIRONMENT */

/*
 * TCP Precedence Levels
 */

#define	T_ROUTINE	0
#define	T_PRIORITY	1
#define	T_IMMEDIATE	2
#define	T_FLASH		3
#define	T_OVERRIDEFLASH	4
#define	T_CRITIC_ECP	5
#define	T_INETCONTROL	6
#define	T_NETCONTROL	7

/*
 * TCP security options structure
 */

struct secoptions {
	short	security;	/* security fiedl			*/
	short	compartment;	/* compartment				*/
	short	handling;	/* handling restrictions		*/
	long	tcc;		/* transmission control code		*/
};

/*
 * TCP options
 */

struct tcp_options {
	short		precedence;	/* precedence			*/
	long		timeout;	/* abort timeout		*/
	long		max_seg_size;	/* maximum segment size		*/
	struct secoptions secopt;	/* TCP security options		*/
};

#ifndef	_KERNEL

/* 
 * XTI error return
 */

#ifdef _REENTRANT
/*
 * Per thread t_errno is provided by the threads provider. Both the extern int
 * and the per thread value must be maintained buy the threads libarary.

 */

#define t_errno   (*_terrno())

#endif  /* _REENTRANT */

extern	int	t_errno;
extern	char *	t_errlist[];
extern	int	t_nerr;

#include <standards.h>

extern	int	t_accept __((int, int, struct t_call *));
extern	char *	t_alloc __((int, int, int));
extern	int	t_bind __((int, struct t_bind *, struct t_bind *));
extern	int	t_blocking __((int));
extern	int	t_close __((int));
extern	int	t_connect __((int, struct t_call *,struct t_call *));
extern	void	t_error __((char *));
extern	int	t_free __((char *, int));
extern	int	t_getinfo __((int, struct t_info *));
extern	int	t_getstate __((int));
extern	int	t_listen __((int, struct t_call *));
extern	int	t_look __((int));
extern	int	t_nonblocking __((int));
extern	int	t_open __((char *, int, struct t_info *));
extern	int	t_optmgmt __((int, struct t_optmgmt *, struct t_optmgmt *));
extern	int	t_rcv __((int, char *, unsigned, int *));
extern	int	t_rcvconnect __((int, struct t_call *));
extern	int	t_rcvdis __((int, struct t_discon *));
extern	int	t_rcvrel __((int));
extern	int	t_rcvudata __((int, struct t_unitdata *, int *));
extern	int	t_rcvuderr __((int, struct t_uderr *));
extern	int	t_snd __((int, char *, unsigned, int));
extern	int	t_snddis __((int, struct t_call *));
extern	int	t_sndrel __((int));
extern	int	t_sndudata __((int, struct t_unitdata *));
extern	int	t_sync __((int));
extern	int	t_unbind __((int));
#endif  /* !_KERNEL */
#endif
