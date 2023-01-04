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
 * @(#)pa.h	5.1	(ULTRIX)	6/19/91
 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
#ifndef _PA_H_
#define _PA_H_

/* 
 * Definitions for 2DA.
 * 
 * Modification History:
 *
 * 13-Aug-1990	Sam Hsu
 *		Cleanup for 2da/3da/fb merged multiscreen.
 *
 * 28-May-1990	Win Treese
 *		Created, based on io/tc/ga.h.
 */

/*
 * Interrupt status bits: these bits are set in in px_info 'intr_status'
 * field.
 *
 * INTR_ACTIVE is set by the server when it submits a packet to the STIC.
 * As long as it is set it means that there is at least one packet which
 * has not yet completed (and thus that the interrupt service routine can
 * be expected to be entered at some point in the future.  This bit
 * is cleared by the isr when it becomes blocked or idle.
 *
 * INTR_BLOCKED is set by the isr when it encounters a microcode packet
 * which it cannot deal with (e.g. one which must be emulated by the
 * server).  It is cleared by the server immediately prior to queueing
 * a packet which the isr can deal with.
 * 
 * INTR_CLIP is set by the isr when it is sending off a packet which
 * has specified a cliplist.
 *
 * INTR_ERR is set by the isr when it detects an error.
 *
 *   Value of intr_status	Meaning
 *   --------------------	-------
 *	       0                Idle.  No packet completions pending.
 *             1                A packet will complete in the future.
 *             2                Idle because isr can't emulate ucode pkt.
 *             3                ILLEGAL.
 */

#define PA_INTR_ACTIVE		(1<<0)
#define PA_INTR_BLOCKED		(1<<1)
#define PA_INTR_CLIP		(1<<2)
#define PA_INTR_ERR		(1<<3)
#define PA_INTR_NEEDSIG		(1<<4)

#if defined(KERNEL) || defined(PA_DEBUG)
/*
 * Template for accessing various fields on the 2DA board.
 */
typedef struct _pa_map {
#ifdef __alpha
    int		stic_poll_reg;		/* 0x000000 */
    char	__pad1[0x300000-sizeof(int)];
#define PA_STIC_OFFSET	0x300000
#define PA_STAMP_OFFSET	0x180000
    stic_regs	stic_reg;		/* 0x300000 */
    char	__pad6[0x400000-0x300000-sizeof(stic_regs)];
#define PA_BT459_OFFSET	0x400000
    struct bt459 vdac_reg;		/* 0x400000 */
    char	__pad3[0x600000-0x400000-sizeof(struct bt459)];
#define PA_BT459_RESET	0x600000
    int		rom[1<<19];		/* 0x600000 */
#else /* __alpha */
    int		stic_poll_reg;		/* 0x000000 */
    char	__pad1[0x180000-sizeof(int)];
#define PA_STIC_OFFSET	0x180000
#define PA_STAMP_OFFSET	0x0C0000
    stic_regs	stic_reg;		/* 0x180000 */
    char	__pad6[0x200000-0x180000-sizeof(stic_regs)];
#define PA_BT459_OFFSET	0x200000
    struct bt459 vdac_reg;		/* 0x200000 */
    char	__pad3[0x300000-0x200000-sizeof(struct bt459)];
#define PA_BT459_RESET	0x300000
    int		rom[1<<18];		/* 0x300000 */
#endif /* __alpha */
} pa_map;


typedef struct _pa_info {
    int intr_status;			/* Interrupt status. */
    int bufselect;			/* Which buffer to use. */
    u_int pkt_intr_count;		/* Pkt-done interrupt count. */
    u_int vert_intr_count;		/* Vert. interrupt count. */
    u_int err_intr_count;		/* Error interrupt count. */
    u_int stray_intr_count;		/* Stray interrupt count. */
} pa_info;

#endif /* kernel || pa_debug */


#ifdef KERNEL
/*
 * (px_info *) to PX-board field ptr
 */
#define PA_MAP(C)		((pa_map *)PX_BASE(C))
#define PA_VDAC(G)		(& (PA_MAP(G)->vdac_reg) )
#define PA_POLL(G)		(& (PA_MAP(G)->stic_poll_reg) )
#define PA_STIC(G)		( (stic_regs *)(((px_info *)(G))->stic) )
#define PA_STAMP(G)		(((int *)(((px_info *)(G))->stamp)

#else
/*
 * For user-level debugging/playing of/with the board
 */
#ifdef PA_DEBUG
#define PAO_MAP(X)	 	(  ( pa_map *)(((pxInfo *)(X))->pxo))
#define PAO_POLL(X)	 	(& (PAO_MAP(X)->stic_poll_reg) )
#define PAO_VDAC(X)	 	(& (PAO_MAP(X)->vdac) )
#define PAO_STIC(X)	 	(& (PAO_MAP(X)->stic_reg) )

#endif
#endif /* kernel */

#define PA_RBUF_SIZE	_96K		/* Ringbuffer size (bytes). */

/****************************************************************************
 *
 * All definitions from here to EOF are related to servicing
 * packet-done interrupts.
 *
 ****************************************************************************/

/* sizes in bytes */
#define  WORD			(0x4)
#define  PAGE			(0x1000)
#define  HALF_PAGE		(PAGE >> 1)
#define  MEG			(0x100000)

#define  PA_CMDBUF0_OFF		(0 * PAGE)
#define  PA_CMDBUF0_SIZE	(1 * PAGE)
#define  PA_CMDBUF1_OFF		(1 * PAGE)
#define  PA_CMDBUF1_SIZE	(1 * PAGE)
#define  PA_IMAGE_BUFFER_OFF	(2 * PAGE)
#define  PA_IMAGE_BUFFER_SIZE	(3 * PAGE)
#define  PA_COMAREA_SRVCOM_OFF	(5 * PAGE + 4)
#define  PA_COMAREA_SRVCOM_SIZE	(HALF_PAGE)
#define  PA_2DCOM_OFF		(5 * PAGE + HALF_PAGE)
#define  PA_2DCOM_SIZE		(HALF_PAGE)
#define  PA_CLIPLIST_OFF	(6 * PAGE)
#define  PA_CLIPLIST_SIZE	(2 * PAGE)
#define  PA_INTR_BUFFER_OFF	(8 * PAGE)
#define  PA_INTR_BUFFER_SIZE	(PA_QUEUE_PACKETS * PAGE)

#ifdef KERNEL
#define N_MAX_CLIPRECTS		16
#define N_MAX_CLIPLISTS		8
#define N_FLUSH			1
#define N_PASSPACKET		5
#define N_NO_CLIPLIST		(N_MAX_CLIPLISTS)
#define NUM_REQUIRED_CONTEXT	4
#define LINE_WIDTH_PER_PKT	1
#define XYMASK_PER_PKT		1
#endif

/* 
 * Given the address of a PixelStamp packet, return the 
 * index of the first word of the 2-word cliprect field
 */

#define DECODE_CLIP_INDEX(p)  ( NUM_REQUIRED_CONTEXT + \
	((((*p) & (1<<8))) >> 5) + ((((*p) & (1<<10))) >> 10) )

typedef struct _stic_cmd {
        unsigned Op                     :4;
        unsigned RGBMode                :2;
        unsigned ZMode                  :2;
        unsigned XYMask                 :2;
        unsigned LineWidthPer           :2;
        unsigned Reserved0              :7;
        unsigned ClipRect               :1;
        unsigned Reserved1              :1;
        unsigned Mesh                   :1;
        unsigned Reserved2              :1;
        unsigned AALine                 :1;
        unsigned Reserved3              :7;
        unsigned HSEquals               :1;
} sticCmd;

typedef struct {
    unsigned long minval;
    unsigned long maxval;
} pa_StampClipRect;

typedef struct {
        long		 numClipRects;
        long		 refCount;
        pa_StampClipRect clipRects[N_MAX_CLIPRECTS];
} pa_ClipList;


#define PA_QUEUE_PACKETS 16
#define PA_LAST_QPACKET (PA_QUEUE_PACKETS-1)

#define PA_QDEPTH(p) PA_QUEUE_PACKETS
#define PA_QLAST(p) PA_LAST_QPACKET

#define NEXT_BUF(p2d, i) ( ((i) == PA_QLAST(p2d)) ? 0 : (i)+1 )
#define PREV_BUF(p2d, i) ( ((i) == 0 ) ? PA_QLAST(p2d) : (i)-1 )
#define ONELESSTHAN(p2d, i1, i2) ( ((i2) == 0) ? (i1) == PA_QLAST(p2d) : \
				        (i1) == (i2) - 1 )

/* defined here mainly to avoid dragging in N10 include files */
typedef struct _pa_Packet {
     union {
	struct {
	    long opcode;
	} un;
	struct {
	    long opcode;
	    long word_count;
	    long cliplist_sync;
	    long data[1];
	} PassPacket;
	struct {
	    long opcode;
	    long word_count;
	    long sram_phys_addr;
	    long r3000_virt_addr;
	} ReadSram;
	struct {
	    long opcode;
	    long word_count;
	    long sram_phys_addr;
	    long r3000_virt_addr;
	} WriteSram;
	struct {
	    long opcode;
	    long sram_phys_addr;
	    long data;
	} PutData;
	struct {
	    long opcode;
	    long cliplist_number;
	    long cliprect_count;
	    pa_StampClipRect rect[1];
	} LoadClipList;
     } un;
} pa_Packet, *pa_PacketPtr;


typedef struct _Com2d {
    long pad1[3];			/* get to aligned boundary */
    long NoOp[10];			/* No-op packet area */
    long Stic_NoOp[10];			/* Store Stic NoOp */
    long Video_NoOp[4];			/* Store Video NoOp */
    volatile long intr_status;		/* interrupt status */
    volatile long lastRead;
    volatile long lastWritten;
    volatile long qDepth;
    pa_StampClipRect *pCliprect;
    long numCliprect;
    pa_StampClipRect *fixCliprect;
    volatile long *srv_qpoll[PA_QUEUE_PACKETS];
    volatile long *intr_qpoll[PA_QUEUE_PACKETS];
    volatile long *save_region[PA_QUEUE_PACKETS];
} pa_Com2d, *pa_Com2dPtr;


typedef struct _pa_ComArea {

	/* Request buffers 0 and 1. */
	long CmdBuf0[ PA_CMDBUF0_SIZE >> 2 ];
	long CmdBuf1[ PA_CMDBUF1_SIZE >> 2 ];

	/* Image buffer -- leave word at the end for count value. */
	long image_buf[ (PA_IMAGE_BUFFER_SIZE+4) >> 2 ];

	/* X Server common area -- remove word to make up for count value. */
	long SRVCom[ (PA_COMAREA_SRVCOM_SIZE-4) >> 2 ];

	/* 2D Server common area. */
	pa_Com2d SRV2DCom;
	char pad1[PA_2DCOM_SIZE - sizeof(pa_Com2d)];

	/* Cliplist */
	pa_ClipList ClipL[ N_MAX_CLIPLISTS ];
	char pad2[PA_CLIPLIST_SIZE-sizeof(pa_ClipList)*N_MAX_CLIPLISTS];

	/* Interrupt-driven request buffers. */
	long IntrBuf[PA_QUEUE_PACKETS][PA_CMDBUF0_SIZE >> 2];

} pa_ComArea, *pa_ComAreaPtr;

#ifdef KERNEL
int	pa_attach();
int	pa_bootmsg();
int	pa_setup();
int	pa_ioctl();
int	pa_map_screen();
int *	pa_getPacket();
void	pa_sendPacket();
void	pa_interrupt();
#endif
#endif /* _PA_H_ */
