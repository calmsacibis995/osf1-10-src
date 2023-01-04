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
 * @(#)$RCSfile: kdebug.h,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/27 14:18:54 $
 */

#ifndef _SYS_KDEBUG_H_
#define _SYS_KDEBUG_H_

#include <sys/types.h>
#include <kern/lock.h>

/*
 * kdebug states
 */
#define	KDEBUG_DISABLED	0x00
#define	KDEBUG_ENABLED	0x01
#define	KDEBUG_BRK	0x02

#define	KDEBUG_MAGIC	0xfeedface

/*
 * request codes for kdebug subsystem entry point
 */
#define	KDEBUG_REQ_INIT		1
#define	KDEBUG_REQ_BRKPT	2
#define	KDEBUG_REQ_MEM		3

typedef struct _KdebugInfo {
    volatile u_long	state;
    u_long		magic;
    u_long		debug;
    u_long		*exc_frame;
    u_int		lock;
    u_int		nofault;
} KdebugInfo;

/*
 * debugging flags
 */
#define DBG_GENERAL     0x0001          /* general */
#define DBG_WARNING     0x0002          /* warnings */
#define DBG_ERROR       0x0004          /* errors */
#define DBG_BRKPT       0x0008          /* breakpoint handling code */
#define DBG_MEM         0x0010          /* memory management */
#define DBG_IO          0x0020          /* character i/o */
#define DBG_RDEBUG      0x0040          /* remote debug requests */
#define DBG_PROTO       0x0080          /* remote debug protocol */

/*
 * kdebug's cpu data
 */
struct kdebug_cpusw {
    unsigned long	system_type;		/* value for cpu */
    char		*system_string;		/* description of machine */
    struct device_table *tty_dev;		/* 'tty' device table */
    unsigned long	tty_base;		/* base address of tty device */
    void		(*delay)();		/* microsecond delay */
};

/*
 * system identifiers - we use our own rather than the kernel's since kdebug
 * is often supported before the kernel port
 */
#define ST_ADU                    1       /* Alpha ADU systype */
#define ST_DEC_4000               2       /* Cobra systype */
#define ST_DEC_7000               3       /* Ruby systype */
#define ST_DEC_3000_500           4       /* Flamingo systype */
#define ST_JENSEN         	  6       /* Jensen systype */
#define ST_DEC_3000_300           7       /* Pelican systype */
#define ST_MORGAN         	  8       /* Morgan systype */

/*
 * device table is interface between debugger and device drivers
 */
struct device_table {
    long (*dt_init)(unsigned long);			/* init */
    unsigned char (*dt_rx_read)();			/* read character */
    long (*dt_tx_write)();				/* write character */
    long (*dt_rx_rdy)();				/* ready to read */
    long (*dt_tx_rdy)();				/* ready to write */
};

/*
 * character device buffer and functions
 */
#define	CBUFSIZE	1024

struct device_buf {
	char *db_in;		/* pts at next free char */
	char *db_out;		/* pts at next filled char */
	char db_buf[CBUFSIZE];	/* circular buffer for input */
};

#define	CIRC_EMPTY(x)	((x)->db_in == (x)->db_out)
#define	CIRC_FLUSH(x)	((x)->db_in = (x)->db_out = (x)->db_buf)

/*
 * these are the dbx register indices
 */
#define	R_R0		0
#define	R_R1		1
#define	R_R2		2
#define	R_R3		3
#define	R_R4		4
#define	R_R5		5
#define	R_R6		6
#define	R_R7		7
#define	R_R8		8
#define	R_R9		9
#define	R_R10		10
#define	R_R11		11
#define	R_R12		12
#define	R_R13		13
#define	R_R14		14
#define	R_R15		15
#define	R_R16		16
#define	R_R17		17
#define	R_R18		18
#define	R_R19		19
#define	R_R20		20
#define	R_R21		21
#define	R_R22		22
#define	R_R23		23
#define	R_R24		24
#define	R_R25		25
#define	R_R26		26
#define	R_R27		27
#define	R_R28		28
#define	R_R29		29
#define	R_R30		30
#define	R_R31		31
#define	R_F0		32
#define	R_F1		33
#define	R_F2		34
#define	R_F3		35
#define	R_F4		36
#define	R_F5		37
#define	R_F6		38
#define	R_F7		39
#define	R_F8		40
#define	R_F9		41
#define	R_F10		42
#define	R_F11		43
#define	R_F12		44
#define	R_F13		45
#define	R_F14		46
#define	R_F15		47
#define	R_F16		48
#define	R_F17		49
#define	R_F18		50
#define	R_F19		51
#define	R_F20		52
#define	R_F21		53
#define	R_F22		54
#define	R_F23		55
#define	R_F24		56
#define	R_F25		57
#define	R_F26		58
#define	R_F27		59
#define	R_F28		60
#define	R_F29		61
#define	R_F30		62
#define	R_F31		63
#define	R_EPC		64
#define	R_PS		65
#define	R_SR		70
#define	NREGS		71

/*
 * max packet size in longwords between dbx and kdebug
 */
#define	MAXPACKET	1024

/*
 * Ascii characters that are special to protocol
 */
#define	SYN		0x16
#define	DLE		0x10

#define	REXMIT_TIME	100000

/*
 * remote debugging packet identifiers
 */
#define P_REG_READ	'r'			/* read registers */
#define P_REG_WRITE	'R'			/* write registers */
#define P_DATA_READ	'd'			/* read data */
#define P_DATA_WRITE	'D'			/* write data */
#define P_THREAD_CNT	't'			/* get the number of threads */
#define P_THREAD_LIST	'T'			/* get the list of threads */
#define P_SET_PID	'p'			/* set the pid */
#define P_CONT		'c'			/* continue the child */
#define P_STEP		's'			/* single step the child */
#define P_EXIT		'x'			/* exit kdebug */
#define P_ACK		'*'			/* ack */
#define P_ERROR		'e'			/* error */
#define P_INIT		'b'			/* init */

#endif
