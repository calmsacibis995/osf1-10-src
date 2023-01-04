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
 * Definitions for the ADU SCSI port driver.
 *
 * Modification History: /sys/io/scsi/alpha/adureg.h
 *
 * 7-Nov-91 -- Farrell Woods
 *	Fixup for OSF merge:
 *	remove explicit "alpha" from include paths
 *	bracket SCSI_CMD_LEN macro to ensure that it's not
 *	redefined
 *
 * 30-Nov-90 -- Tim Burke
 *	Created this file for Alpha support.
 */

/*
 * Protect the file from multiple includes.
 */
#ifndef __ADU_SCSI_HDR
#define __ADU_SCSI_HDR

#include <sys/types.h>
#include <hal/adudefs.h>
#include <hal/ka_adu.h>
#include "scsi_debug.h"

/*
 * Ringing the doorbell involves writing arbitrary data to the doorbell reg.
 */
#define RING_SCSI_DOORBELL 				\
	*adu_scsi_regs.db = 1;				\
	mb();

/*
 * Macros are needed to set and clear interrupt bits.  This is necessary
 * bacause the register is write-only so a read-modify-write is not possible.
 */
#define	ENABLE_SCSI_INTERRUPT(val)			\
        adu_scsi_intr_reg |= (val); 			\
	*adu_scsi_regs.icr = adu_scsi_intr_reg;		\
	mb();

#define	DISABLE_SCSI_INTERRUPT(val)			\
        adu_scsi_intr_reg &= ~(val); 			\
	*adu_scsi_regs.icr = adu_scsi_intr_reg;		\
	mb();

/*
 * Queue manipulation macros
 *
 * These queues are used to store command requests when the command ring
 * is full.  There is a free queue of entries and a waitq which are actual
 * formatted commands waiting to be placed in the command ring.
 */
#define	Remove_entry( entry ) {						\
    ( void )remque( entry );						\
}
#define	Insert_entry( entry, queue ) {					\
    ( void )insque( entry, ( queue ).blink );				\
}

/*
 * The command ring contains 32 entries, each 128 bytes long.
 * Structure of the SCSI_SEND_COMMAND ring entry:
 */

struct send_command {
  volatile u_int   	flag;		/* Set to SCSI_SEND_COMMAND	      */
  volatile u_int 	pad0;
  volatile u_long 	pad1[3];
  volatile u_long 	cmdtag; 	/* Arbitrary command tag	      */
  volatile u_int 	target;		/* Target portion of phys SCSI addr   */
  volatile u_int 	bus;		/* Bus portion of physical SCSI addr  */
  volatile u_int 	nmsg;		/* Length of SCSI outbound message    */
  volatile u_int	ncmd;		/* Length in bytes of the SCSI command*/
  volatile u_int 	options;	/* Flags that modify command operation*/
  volatile u_int	timeout;	/* Longest driver waits for completion*/
  volatile u_char	*dataaddr;	/* Physical address of data buffer    */
  volatile u_int	ndata;		/* Length of buf for DATAI & DATAO    */
  volatile u_int 	pad2;
  volatile u_long 	pad3[2];
  volatile u_long	msg;		/* Actual message transmitted on bus  */
  volatile u_char	cmd[24];	/* Actual command transmitted on bus  */
};

/*
 * The message ring contains 32 entries, each 128 bytes long.
 * Structure of the SCSI_COMMAND_COMPLETE message ring entry:
 */

struct command_complete {
  volatile u_int 	flag;		/* Set to SCSI_COMMAND_COMPLETE	      */
  volatile u_int 	pad0;
  volatile u_long 	pad1[3];
  volatile u_long 	cmdtag;		/* Copy of cmdtag from send_command   */
  volatile u_int	cstatus;	/* Final completion status of the cmd */
  volatile u_int 	ndata;		/* Number of data bytes transfered    */
  volatile u_int	nsstatus;	/* Number of bytes stored in sstatus  */
  volatile u_int	pad2;
  volatile u_long	pad3;
  volatile u_char 	sstatus[64];	/* Actual SCSI status bytes.	      */
};

/*
 * Declarations of the actual ring structures.  They consist of a 32 entry 
 * command ring followed by a 32 entry message ring.
 */
#define	ADU_RING_ENTRIES	32

struct scsi_ring {
	struct send_command	command_ring[ADU_RING_ENTRIES];
	struct command_complete	message_ring[ADU_RING_ENTRIES];
};

/*
 * This is what a command in the waitq will look like.  It is just the same
 * structure as the actual command ring with forward and backward links used
 * to chain them together.
 */
struct adu_scsi_wait {
	struct adu_scsi_wait *flink;		/* Forward pointer	*/
	struct adu_scsi_wait *blink;		/* Back pointer		*/
	struct send_command  send_command;	/* Actual ring packet   */
};

/*
 * Defines the queue head.  A queue is merely a forward and backward link.
 * An empty queue has the flink=blink=&queue.flink.
 */
struct aduqueue {
	struct adu_scsi_wait *flink;		/* Forward queue link	*/
	struct adu_scsi_wait *blink;		/* Backward queue link	*/
};

/*
 * Allocate enough space for this many entries on the waitq.  This implies that
 * there can be at most ADU_SCSI_NUMWAITS + 32 SCSI commands outstanding at any
 * time on an individual bus.
 *
 * NOTE: Since there can only be 1 command outstanding to a target there
 * probably is no need for a waitq anyways!
 */
#define ADU_SCSI_NUMWAITS 20

/*
 * SCSI FLAG Codes	(Table 4-14)
 */
#define SCSI_EMPTY		0
#define SCSI_SEND_COMMAND	1
#define SCSI_COMMAND_COMPLETE	2

/*
 * SCSI CSTATUS Codes	(Table 4-15)
 */
#define SCSI_OK			0
#define SCSI_SELECT		1
#define SCSI_REJECT		2
#define SCSI_TIMEOUT		3
#define SCSI_BUS		4
#define SCSI_OVERRUN		5
#define SCSI_UNDERRUN		6
#define SCSI_FIRMWARE		7

/*
 * SCSI OPTIONS Flags	(Table 4-16)
 */
#define SCSI_NORETRY		1
#define SCSI_NOSENSE		2

/*
 * Layout of the interrupt control register (icr)
 */
#define ADU_SCSI_IE		0x1		/* Interrupt Enable	*/

/*
 * This version of the driver only supports group 0 and 1 commands.  These
 * commands are of length 6 and 10 bytes respectively.  The type is determined
 * by the command number.
 */
#ifndef GET_SCSI_CMD_LEN
#define GET_SCSI_CMD_LEN(cmd)   ((((cmd >> 5) & 0x07) == 1) ? 10 : 6)
#endif
/*
 * Hack to allow overlapped usage of fields in the sc structure.
 */
#define sc_no_disconnects    sc_oddbyte
/*
 * Limit the maximum data transfer request size to 64K.
 */
#define SCSI_ADU_MAXXFER    65536	
/*
 * Values for the align_buf_inuse field of the softc structure.
 */
#define SCSI_ALIGN_READ		0x1	/* Data input command		*/
#define SCSI_ALIGN_WRITE	0x2	/* Data output command		*/
/*
 * The alignment buffer must be big enough to support the largest possible
 * scsi read or write size.  This size is presently limited to 64K in the
 * routine minphys which is in vm/vm_swp.c.  If the value of MAXPHYS changes
 * in that file this algorithm will fall appart.
 */
#define SCSI_MAXPHYS (64 * 1024)
#endif /* __ADU_SCSI_HDR */      /* Multi include protection ends here */
