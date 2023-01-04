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
 *	@(#)$RCSfile: scc_sl.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
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
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

/*

 *
 */

/*
 * Include file description:
 *
 * This file contains the structure definitions for all nonstandard
 * commands and attentions which are possible on the SCC serial line
 * ( SL ) read and write channels.
 *
 * Unit numbers:
 *
 * SCC serial line read channels:	1 x x x 0 0
 * SCC serial line write channels:	1 x x x 0 1
 *
 * Note: x x x is the line number of the serial line
 *
 * The following CRQ message structures are defined in this file
 *
 *		o crq_cl_config_msg_t
 *		o crq_sl_write_msg_t
 *		o crq_sl_pause_msg_t
 *		o crq_sl_resume_msg_t
 *		o crq_sl_read_msg_t
 *		o crq_sl_attn_msg_t
 *
 */

/* SL configuration and attentions */
/* NOTE the following:
 *
 *	1. All attentions are on the receive channel.
 *	2. Some bits in the sl_mode_attn field of the sl_mode structure are
 *	   significant ( The rest should be set to zero ).  By default, the
 *	   attention mode is zero. Each bit corresponds to a bit in the
 *	   status word and can be used to signal an attention.
 *	3. Whenver there is a change in one of the status bits which
 *	   was also set in the sl_mode_attn, an attention is issued with
 *	   the current status on the read channel.  There is not a 
 *	   correspondence between the number of transitions and the number
 *	   of attentions;  the objective is to inform the host of any
 *	   change in the status.  The only exception to this is the
 *	   error condition, ring buffer overflow; in this case, sending
 *	   the attention causes the status bit to be cleared.
 *	4. The uart itself is initialized with the sl_mode_uart field
 *	   of the sl_mode structure.
 */

/* status/attention mode bit definitions */

#define SLCHAN_READ_DATA_PRESENT	BIT(0)	

#define SLCHAN_READ_BUFFER_OVERFLOW	BIT(1)	
#define SLCHAN_SHORT_BREAK_DETECT	BIT(2)	
#define SLCHAN_LONG_BREAK_DETECT	BIT(4)	

#define SLCHAN_RING_DETECT		BIT(5)	
#define SLCHAN_DCD			BIT(6)	
#define SLCHAN_EXT_STAT_CHG		BIT(7)	

/* terminal is ready to receive and transmit */
#define SLCHAN_READY			BIT(8)	

/* UART mode bits */

#define SL_UART_MODE(write_baud,read_baud,parity_en,even,stop,bits) \
	( write_baud + read_baud << 8 + parity_en << 16 + stop << 18 + \
	  bits << 20 )

/*	.[0,7]		write baud rate ( 9600 default )
 *	.[8,15]		read baud rate ( NOP, set by write )
 *	.[16]		parity enable ( disable, default )
 *	.[17]		parity even
 *	.[18,19]	number of stop bits
 *			0 1 = 1 stop bit
 *			1 0 = 1 1/2 stop bits	( default )
 *			1 1 = 2 stop bits
 *	.[20,21]	bits per char
 *			0 0 = 5 bits
 *			0 1 = 6 bits
 *			1 0 = 7 bits
 *			1 1 = 8 bits  (default)
 */

#define SL_1STOP	BIT(18)
#define SL_8BIT		(BIT(20) | BIT(21))

struct sl_mode {
	long sl_mode_attn;
	long sl_mode_uart;
	} ;

/* opcodes: CRQOP_SL_SET_MODE, CRQOP_SL_GET_MODE */
typedef struct crq_sl_mode_msg {
	crq_msg_t	sl_config_hdr;
	struct sl_mode sl_config_mode;
	long sl_config_status; /* returned in either get or set */
	} crq_sl_mode_msg_t;
#define SL_MODE_MSG_REV_LEVEL	1

/* opcodes: CRQATTN_SL */
typedef struct crq_sl_attn_msg {
	crq_msg_t sl_attn_msg;
	long sl_attn_status;
	} crq_sl_attn_msg_t;
#define SL_ATTN_REV_LEVEL 1


/* configure serial line */
/* This message can be given on either serial line, however, both the
 * read and write channels are affected.
 */
typedef struct crq_sl_config_msg { /* OBSOLETE !!! */
	crq_msg_t	sl_config_hdr;
	/* If high bit is set, appropriate change to UART is made.
	 * If any are set, a new byte string is send out to UART.
	 */

	short	sl_config_attn;
	/* High bit set means reset attention mode.
	 */

	long	sl_config_attn_mode;
	/* .[0] ATTN on first character ( read channel )
	 * .[1] ATTN on ring buffer overflow ( read channel )
	 * .[2] ATTN on break detect ( read channel )
 	 * .[3,27]
	 * .[28] ATTN on carrier detect (read/write)
	 * .[29] ATTN on ring indicator (read/write)
	 * .[30] ATTN on modem state change (read/write)
	 * .[31] ATTN on data present (read)
	 * default = 0
	 */
	short	sl_config_modem;
	/* .[0]	modem active
	 * .[2,14]
	 * .[15] reset modem mode
	 * default = 0
	 */
	short	sl_config_bits;
	/* .[0] parity enable
	 * .[1] parity even
	 * .[2,3] number of stop bits
	 *	0 1 = 1 stop bit
	 *	1 0 = 1 1/2 stop bits
	 *	1 1 = 2 stop bits
	 * .[4,5] bits per character
	 *	0 0 = 5 bits
	 *	0 1 = 6 bits
	 *	1 0 = 7 bits
	 *	1 1 = 8 bits
	 * default = 0x34
	 */
	short	sl_config_baud;
	/* baud rate '#define' value with high bit set
	 *
	 * default = BAUD_9600
	 */
	short	sl_config_status;
	} crq_cl_config_msg_t;


/* write to SL write channel */
typedef struct crq_sl_write_msg {
	crq_msg_t	sl_write_hdr;
	short	sl_write_char_count;
	short	sl_write_pad0;
	char	*sl_write_buff;
	short	sl_write_actual;
	short	sl_write_pad1;
	} crq_sl_write_msg_t;
#define SL_WRITE_MSG_REV_LEVEL 1

/* resume SL write channel */
/* must be issued on immediate queue */
typedef struct crq_sl_pause_msg {
	crq_msg_t sl_pause_hdr;
	short sl_pause_status;
	} crq_sl_pause_msg_t;

/* pause SL write channel */
/* must be issued on immediate queue */
typedef struct crq_sl_resume_msg {
	crq_msg_t sl_resume_hdr;
	short sl_resume_status;
	} crq_sl_resume_msg_t;

/* SL generic attention message */ /* OBSOLETE !!! */
/* typedef struct crq_sl_attn_msg {
	crq_msg_t sl_attn_msg;
	/* the attn state is a mask of which attentions have occured
	long	sl_attn_state;
	long	sl_attn_status;
	} crq_sl_attn_msg_t;
*/
/* read from SL read channel */
typedef struct crq_sl_read_msg {
	crq_msg_t	sl_read_hdr;
	short	sl_read_data_limit;	/* maximum number of characters */
	short	sl_read_error_limit;	/* maximum number of errors */
	long	sl_read_delay; 		/* defined by free run counter */
	char	*sl_read_data;		/* address of data buffer */
	long	*sl_read_error;		/* address of error buffer */
	/*	sl_read_error[n] bits mean the following:
	 *	.[0,15]	index of error into receive buffer
	 *	.[16,18]
  	 *	.[19] = parity error
  	 *	.[20] = rcv uart overflow
  	 *	.[21] = framming error
  	 *	.[22] = break detect
	 *	.[23,31]
	 */
	short	sl_read_data_actual;		/* Actual characters read */
	short	sl_read_data_errors;		/* Actual number of errors */
	} crq_sl_read_msg_t;
#define SCC_ERR_INDEX	0xffff
#define SCC_ERR_FE	BIT(21)
#define SCC_ERR_PE	BIT(19)

/*
 *	Constants used by sccsl.c
 */
#define NSCCSL	2		/* # serial lines on scc */
#define SL_ATTN_RBUF	0x04	/* Attention on ring buffer overflow	*/
#define SL_ATTN_REXSTS	0x02	/* Attn on rcv chan on extern status change */
#define SL_ATTN_RESET	0x8000	/* Reset attention mode */
#define SL_ATTN_MODE	(SL_ATTN_RBUF | SL_ATTN_REXSTS | SL_ATTN_RESET)
#define SL_USART_8BIT	0x30	/* Set 8-bit characters	*/
#define SL_USART_1STOP	0x04	/* Set one stop bit	*/
#define SL_USART_PENABLE 0x01	/* Set parity checking	*/
#define SL_USART_PEVEN	0x02	/* Set even parity	*/
#define SL_USART_MODE	(SL_USART_8BIT | SL_USART_1STOP)

#define SL_NUM_ATTN	2	/* # attn packets pre-allocated per line */
#define SL_NUM_IMMED	2	/* # immed cmds pre-allocated per line */
#define SL_NUMRDBUF	2	/* # read buffers per line */
#define SL_RDBUFSZ	256	/* Size of read buffer */
#define SLCRQ_DISCONNECT	0	/* CRQ-to-SCC channel does not exist */
#define SLCRQ_CONNECT		1	/* Channel does exist */
#define SL_DELAY	10	/* Delay in ms before SCC interrupts */

#define IS_ODD(n) ((n)&1)
#define SL_GETUNIT(dev)	(dev)

/*
 * This structure defines most of the data neeed to talk to the serial
 * lines on the SCC.
 */
typedef	struct	sccsl	{
	short	crq_status;		/* connected or not */
	crq_t	xmt_crq;		/* transmit channel crq */
	crq_t	rcv_crq;		/* receive channel crq */
	isr_queue_t	ioque;		/* internal synchonization que */
	crq_abort_msg_t	immed_msg[SL_NUM_IMMED];
					/* send immediate command buffers */
	crq_sl_attn_msg_t	attn_msg[SL_NUM_ATTN];
					/* ... */
	crq_sl_write_msg_t	write_msg;
					/* One and only write message */
	crq_sl_read_msg_t	read_msg[SL_NUMRDBUF];
					/* Why multiple buffers? */
	u_int	read_error[SL_NUMRDBUF];/* one read error ber buffer */
	char	read_buffer[SL_NUMRDBUF][SL_RDBUFSZ];
					/* one data buffer per read message */
} sccsl_t;

/*
 *	Reference numbers 
 */
#define SCCSL_REF_WRITE		1

#define SCC_RDPKT_REV_LEVEL	1


/* Map minor dev to struct tty */
#define SL_DEVTOTTY(dev)	(&slc_tty[dev])

/* Map scc control block to tty */
#define SL_SPTOTTY(sp)	(&slc_tty[sp - sccsl])

/* Map tty to scc control block */
#define SL_TTYTOSP(tty)	(&sccsl[tty - slc_tty])
