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
 * derived from uqscs.h	2.5	(ULTRIX)	10/12/89
 */
/*
 *	Modification History
 *
 *	07-May-1990  - Matthew S.  Add the declarations of init_leader
 *		and reset_leader, and UQPath_Is_Up.  These are used for
 *		smp-safeness.  UQPath_Is_Up is a constant defined to be
 *		32.  32 is used because a Cpu processor number cannot 
 *		ever be 32.  (0 <= Cpu_Id <= 31).  When the port is up
 *		and connected, both init_leader and reset_leader equal
 *		UQPath_Is_Up.  When a port is down, or the decision is
 *		otherwise made to re-init the port, the CPUs contend in
 *		an smp-safe manner, for the position of init_leader.
 *		The cpu that wins the smp contention sets init_leader
 *		to the cpu_num value of itself.  This is the cpu
 *		that will actually start the init.  When the four
 *		step init process is complete, init_leader is set back
 *		to UQPath_Is_Up.  The reset routine, uq_reset, is comprised 
 *		of initiating the four-step init, rebuilding the simulated
 *		SCS layer, and remapping the buffers.  In uq_reset, the cpus 
 *		contend for the position of reset leader, the winner sets the
 *		reset_leader to itself, the current cpu_num, sets flags that
 *		indicate to rebuild the SCS layer and re-map the buffers.
 *		uq_reset then goes to pick an init_leader in the described
 *		way.  After this is done, reset_leader is set back to
 *		UQPath_Is_Up.
 *
 *	20-July-1989 - map (Mark A. Parenti)
 *	        Add regptrs structure for use in accessing device registers.
 *		Make pccb changes to regptrs structure.
 *
 *	18-July-1988 - map
 *		Dynamically allocate data structures.
 *
 *	15-Feb-1988 -- map
 *		Removed pointers to SCS buffers.
 *		Add scswaitq for use in connection handshakes.
 */

#ifndef _UQSCS_H_
#define _UQSCS_H_

#define	NRSPL2	4		/* log2 number of response packets	*/
#define	NCMDL2	4		/* log2 number of command packets	*/
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)
#define NBUF	NRSP + NCMD + 8 + 1
#define UQPath_Is_Up  32	/* This must be a value that cannot */
				/* be a cpu number */


/*
 * UQSSP device registers and structures
 */

typedef struct _uqregptrs {

	volatile unsigned short	*uqip;	/* Initialization and Polling Register*/
	volatile unsigned short	*uqsa;	/* Status, Address and Purge Register*/
	volatile unsigned short *uqsaw; /* Write SA - VAXBI Only	     */
	volatile unsigned int	*uqpd;  /* Port Data Register - XMI Only     */
} UQREGPTRS;


typedef struct _uqpccb {		/* UQ Port Command and Control Block */
	struct	_uq	*uqptr;		/* Mapped address of uq structure */
	struct	_uq	*uq;		/* Unmapped address uq structure */
	uqbq		*uq_freel;	/* Free buffer list		*/
	uqbq		waitq;		/* command wait queue		*/
	uqbq		scswaitq;	/* scs wait queue		*/
	struct  kschedblk isrforkb;	/* Isr fork block queue 	*/
	struct  _pb  	*pb;		/* PB associated with this port	*/
	struct  _uqh 	*rspbtab[NRSP]; /* Physical address of buffer in ring */
					/* entry			*/
	struct	_uqh  	*cmdbtab[NCMD];	/* ditto			*/
	struct	_connid	contab[NCON];	/* SCS connection id mapping	*/
	struct	_uqh 	*lfptr[NCON]; 	/* Ptr to last fail packet 	*/
	struct	_uqregptrs uqregptrs; 	/* Ptr to IO regs		*/
	struct	_uqscp 	*uqscp; 	/* Pointer to ssp scratchpad area*/
	int		uqregsize;	/* Size of register space	*/
	int		ncon;		/* Current connection count	*/
	int		uq_ctlr;	/* Controller number		*/
	short		uq_con;		/* Connection mask		*/
	short		uq_mapped;	/* UNIBUS map allocated uq structure? */
	short		map_requests;	/* Num of outstanding mapping requests*/
	short		reinit_cnt;	/* Number of reinit tries remaining*/
	struct  {
	    u_int  rip		:1;	/* Recovery in progress flag	*/
	    u_int  isrfork	:1;	/* Isr fork block scheduled	*/
	    u_int		:30;	/* 	*/
	} lpstatus;
#define UQ_RIP	 	0x00000001
#define UQ_ISRFORK 	0x00000002
	int		uq_ubainfo;	/* Unibus mapping info 		*/
	int		uq_ivec;	/* interrupt vector address 	*/
	short		uq_lastcmd;	/* pointer into command ring 	*/
	short		uq_lastrsp;	/* pointer into response ring 	*/
	short		rsprindx;	/* index into response ring	*/
	short		rsp_cnt;	/* count of used response ring entries*/
	short		cmdrindx;	/* index into command ring	*/
	short		cmd_cnt;	/* count of used command ring entries*/
	int		poll_rate;	/* Time between polls of SA register*/
	short		step1r;		/* step 1 read data 		*/
	short		step1w;		/* step 1 write data 		*/
	short		step2r;		/* step 2 read data 		*/
	short		step2w;		/* step 2 write data 		*/
	short		step3r;		/* step 3 read data 		*/
	short		step3w;		/* step 3 write data 		*/
	short		step4r;		/* step 4 read data 		*/
	short		step4w;		/* step 4 write data 		*/
	u_int		xmipd;		/* Interrupt info xmi uqpd register*/
	u_int		bda_init_vec;	/* vector written by uq_port_reset after
					   BI Start Self Test */
	u_int		bda_init_dest;	/* BI dest value written by
					   uq_port_reset after BI SST */
	u_int		bda_init_errvec;/* BI errveca written by uq_port_reset
					   after BI SST */
	short		init_leader;	/* 32, or the cpu number of the
					   processor who is currently 
					   initing the port */
	short		reset_leader;	/* 32, or the cpu number of the
					   processor who is currently
					   resetting the response ring,
					   a reset, in this context, is
					   an init plus buffer mapping */
	struct 	bus 	*uq_bus;	/* for bus pointer on ALPHA */
} UQPCCB;

#endif
