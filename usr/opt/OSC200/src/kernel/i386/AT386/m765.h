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
 *	@(#)$RCSfile: m765.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:38 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
 
/*
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 *    Copyright 1988  Intel Corporation.
 */

/*	Copyright (c) 1987, 1988 TOSHIBA Corp.		*/
/*		All Rights Reserved			*/



/*******************************************************************
 *
 *	 Toshiba Floppy Driver for UNIX System V R3
 *
 *					June 21, 1988 
 *
 *	Intended Drive Units:
 *		Worldwide - Model No. ND-356  3.5" unformatted 2MB/1MB
 *		UNIX Media Type Name: 2HD512/2DD512/2D512/1D512.
 *
 *		In Japan Only - Model No. ND-355  3.5" unformatted 1.6MB/1MB
 *		UNIX Media Type Name: 2HC1024/2HC512/2HC256/2DD512/2D512/1D512.
 *
 *		Worldwide - Model No. ND-04DT-A  5.25" unformatted 500 KB
 *		UNIX Media Type Name: 2D512/1D512.
 *
 *		In Japan Only - Model No. ND-08DE  5.25" unformatted 1.6MB/1MB
 *		UNIX Media Type Name: 2HC1024/2HC512/2HC256/2DD512/2D512/1D512.
 *
 *		Use with other devices may require modification.
 *
 *	Notes:
 *		For further detail regarding drive units contact 
 *		Toshiba America,Inc. Disk Products Division,
 *		Irvine, CA (714) 583-3000.
 *
 *******************************************************************/



/*
 *		KEYBOARD status (FD status)
 *
 *		 7  6  5  4  3  2  1  0
 *      	|--+--+--+--+--+--+--+--|
 *      	|  |  |  |  |  |  |  |  |
 *      	|--+--+--+--+--+--+--+--|
 *			  ^  ^  ^  ^
 *		          |  |  |  |------ external fdd connecte flag
 *			  |  |  |                  0:not conected
 *			  |  |  |                  1:conected
 *		          |  |  |--------- drive change flag
 *			  |  |                     0:changed
 *			  |  |                     1:not changed
 *			  |  |------------ internal fdd count 
 *			  |			   0:2 internal
 *			  |			   1:1 internal
 *		          |--------------- inner fd drive type flag
 *		                           0:2.0Mbyte type
 *		                           1:1.6Mbyte type
 */
#define FDEXT		0x02		/* external fd connect flag  */
#define FDCHG		0x04		/* fd change flag	*/
#define INFDD		0x08		/* internal fdd count */
#define HDDRV		0x10		/* check_data for FDD0 2hd or 2hc */
/*
 *	fdcmd.c_rbmtr
 *
 *      	|--+--+--+--+--+--+--+--|
 *      	|  |  |  |  |  |  |  |  |
 *      	|--+--+--+--+--+--+--+--|
 *		       ^  ^        ^  ^
 *		       |  |        |  |--- unit0 motor on flag
 *		       |  |        |------ unit1 motor on flag
 *		       |  |--------------- unit0 recalibrate flag
 *		       |------------------ unit1 recalibrate flag
 */
#define MTRMASK		0x003		/* mask motor_flag for get status */
#define MTRRST		0x0fc		/* reset motor_flag data */
#define RBSHIFT		0x004		/* shift count for recalibrate data */
#define RBRST		0x0cf		/* reset recalibrate data */

/*
 *	fdcmd.c_intr
 *
 *        	    		   |--+--+--+--+--+--+--+--|
 *        	    		   |  |  |  |  |  |  |  |  |
 *        	    		   |--+--+--+--+--+--+--+--|
 *		 	      	     ^  ^  ^  ^  ^  ^  ^  ^
 *                       reserved  --+  |  |  |  |  |  |  +--- read/write flag
 *                       reserved  -----+  |  |  |  |  +------ seek flag
 * 	     		 reserved  --------+  |  |  +------ seek flag(for retry)
 * recalibrate/seek flag(for open) -----------+  +--------- recalibrate flag
 */
#define RWFLAG		0x001
#define SKFLAG		0x002
#define SKEFLAG		0x004
#define RBFLAG		0x008
#define WUPFLAG		0x010
#define CMDRST		0x000	

/* 
 *	fddrtab.dr_type 
 *
 *        +---+---+---+---+---+---+---+---+
 *        |   |   |   |   |   |   |   |   |
 *        +---+---+---+---+---+---+---+---+
 *          ^   ^   ^   ^   ^
 *          |   |   |   |   |-------------- rapid seek flag
 *          |---|   |   |                     0: normal seek
 *            |     |   |                     1: rapid seek
 *            |     |   |------------------ detect format
 *            |     |                         0: no detect
 *            |     |                         1: format type OK
 *            |     |---------------------- 40 or 80 cylinder(for 2hc/2dd drive)
 *            |                               0: 80 cylinder
 *            |                               1: 40 cylinder
 *            |---------------------------- transfer rate(for read/write/format)
 *                                            00: 500kbps  10: 250kbps
 *                                            01: 300kbps  11: reserved
 */
#define OKTYPE		0x10		/* media change flag */
#define DOUBLE		0x20		/* double/single step change */
#define RAPID		0x08		/* rapid seek flag */
#define RPSEEK		0x00		/* rapid seek */
#define NMSEEK		0x80		/* normal seek */
#define RATEMASK	0xc0		/* transfer parameter mask data */

/*
 *	device number
 *
 *	 15             10  9  8  7                    0
 *	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *	| 0  0  0  0  0  0  0  1| 0|  |  |  |         	|
 *	+-----------+-----+-----+--+--+--+--+-----------+
 *	  ^                    ^  ^  ^  ^  ^  ^        ^ 
 *	  |____________________|  |__|  |  |  |________|
 *		    |		    |   |  |      |
 *		    |		    |   |  |      |-  media type
 *	       major number	    |   |  |     0:auto type 5:2dd512(9sector)
 *				    |   |  |     1:2hd512    6:2dd512(8sector)
 *				    |   |  |     2:2hc256    7:2d512 (9sector)
 *				    |   |  |     3:2hc512    8:2d512 (8sector)
 *				    |   |  |     4:2hc1024 [ 9:1d512 (9sector)]
 *				    |   |  |     	   [10:1d512 (8sector)]
 *	     			    |   |  |-- use cly 0 (0:use/1:not use)
 *				    |   |----- cylinder count (0:80/1:77)
 *                                  |--------- unit number
 */
#define UNIT(dev)	((dev & 0xc0)>>6)	/* get unit number */
#define CYLFLAG(dev)	((dev & 0x20)>>5)	/* get cylinder count flag */
#define PARTITION(dev)	((dev & 0x10)>>4)	/* get partition status */
#define MEDIATYPE(dev)	(dev & 0x0f)		/* get media type */
#define DEVTYPE(dev)	(fdtype[UNIT(dev)])	/* device type (from CMOS) */
#define CYL77		77			/* 77 cylinders */
/*****************************************************************************
 
		wait time / timeout count

 *****************************************************************************/
/*
#ifdef	OWS50
#define MSEC		0x5ae
#define STSCHKCNT	0x2000
#endif
*/
#define MSEC		0x1000		/* 1 msec loop count */
#define STSCHKCNT	0x2800		/* For check status */
#define ITOUT		HZ*5		/* interrupt timeout count */
#define TOUT		HZ/4		/* media type check timeout count */
#define MTRSTOP		HZ*2		/* motor off time */
#define SEEKWAIT	HZ/100*3	/* head_lock time */

/******************************************************************************
 
		define for FDC

 ******************************************************************************/
/* FDC register */
#define CTRLREG		0x03f2		/* controle register */
#define STSREG		0x03f4		/* status register */
#define DATAREG		0x03f5		/* data register */
#define VFOREG		0x03f7		/* vfo register */

/* CTRLREG flags */
#define HEADLOAD	0x01
#define FDC_RST		0x04
#define MTR_ON		0x04
#define DMAREQ		0x08

/* status for command_out */
#define FD_OSTS		0x00		/* For output check */
#define FD_ISTS		0x40		/* For input check */
#define DTOCPU		0x40
#define DATAOK		0x80

/* Command for FDC */
#define SPCCMD		0x03		/* Specify command */
#define SDSCMD		0x04		/* sense device status */
#define RBCMD		0x07		/* Recalibrate command */
#define SISCMD		0x08		/* Sense interrupt status command */
#define SEEKCMD		0x0f		/* seek command */
#define RDM		0xe6		/* FDC READ command */
#define RDMV		0x42e6		/* VERIFY READ command */
#define WTM		0xc5		/* FDC WRITE command */
#define FMTM		0x4d		/* FDC FORMAT command */
#define FMTDATA		0x5e		/* format data */

/* check value */
#define TRACK0		0x10
#define RWLEVEL		0x80		/* interrupt level */
#define OPENBIT		0x80		/* VFO check define */
#define GETFDSTS	0xb4		/* kb ioctl */
#define BYTEMASK	0xff
#define WORDMASK	0xffff
#define SEG1		0x10000

/* FDC error code define */
#define OK		0x00
#define ERROR		0xff
#define EBBHARD		128
#define EBBSOFT		129
#define ST0AT		0x40
#define ST0IC		0x80
#define ST0OK		0xc0
#define ADDRERR		0x01
#define WTPRT		0x02
#define NOREC		0x03
#define OVERRUN		0x04
#define CRCERR		0x05
#define FDCERR		0x06
#define ILLFMT		0x07
#define TIMEOUT		0x08
#define DOORERR		0x09
#define ILLINT		0x0a

/******************************************************************************
 
		define for DMA

 ******************************************************************************/
/* DMA register */
#define DMACMD1		0x08		/* DMA #1 command register 	*/
#define DMAMSK1		0x0f		/* DMA #1 all mask register	*/
#define DMACMD2		0xd0		/* DMA #2 command register 	*/
#define DMASMSK		0xd4		/* DMA #2 single mask register	*/
#define DMAMOD2		0xd6		/* DMA #2 mode register    	*/
#define DMAMSK2		0xde		/* DMA #2 all mask register	*/
#define DMADATA		0x00		/* DMA #1 command data	 	*/
#define DMABPFF		0x0c
#define DMAMODE		0x0b
#define DMAADDR		0x04
#define DMAPAGE		0x81
#define DMACNT		0x05
#define DMAMSK		0x0a

#define DMADONE		0
#define DMABUSY		1

/* dma set data */
#define DMARD		0x46		/* DMA read mode		*/
#define DMAWT		0x4a		/* DMA	write mode		*/
#define DMAVRF		0x42		/* DMA verify mode		*/

#define DMADATA0	0x00		/* DMA #2 all mask data 	*/
#define DMADATA1	0x0b		/* DMA #1 all mask data 	*/
#define DMADATA2	0x00		/* DMA #2 command data	 	*/
#define DMADATA3	0x00		/* DMA #2 single mask data	*/
#define DMADATA4	0xc0		/* DMA #2 mode data	    	*/
#define DMADATA5	0x0e		/* DMA #2 all mask data 	*/
#define CHANEL2		0x02

/******************************************************************************
 
		etc. define

 ******************************************************************************/
#define SPL		spl4			/* for driver mutex */
#define MAXUNIT		2			/* Max unit number */
#define BLKSIZE		512			/* block size	   */

/* fddtype */
#define EXT2D	0x11
#define EXT2HD	0x22

/* fdcmd.c_stsflag	*/
#define MTRFLAG	0x01
#define MTROFF	0x02
#define INTROUT	0x04

/* fdcmd.c_devflag (media check flag . etc.) */
#define FDMCHK		0x01
#define FDWAIT		0x02
#define STRCHK		0x04
#define STRWAIT		0x08

/* fdcmd.c_dcount */
#define FDCCNT	9	/* Command table for read/write/format (FDC) */
#define RWCNT	9	/* Read/Write command count */
#define FMTCNT	6	/* format command count */

struct	fdcmd {
	int	c_rbmtr;		/* moter & rcalibrate flag */
	int	c_intr;			/* intr flag */
	int	c_stsflag;		/* moter flag */
	int	c_mtrid;		/* motor off queue id */
	int	c_timeid;		/* interrupt timeout id */
	int	c_devflag;		/* device status   */
	int	c_dcount;		/* Read/Write/Format data count */
	int	c_rwdata[FDCCNT];	/* Read/Write/Format cmd (FDC) */
	int	c_saddr;		/* cmd seek address */
};

struct	fdubuf {
	int	mb_flags;		/* not used		      */
	struct	fdubuf *b_unitf;	/* first buffer for this dev  */
	struct	fdubuf *b_unitb;	/* last buffer for this dev   */
	struct	buf 	*av_forw;	/* head of I/O queue (b_forw) */
	struct 	buf	*av_back;	/* tail of I/O queue (b_back) */
	char	b_unit;			/* unit number		      */
	int	b_seekaddr;		/* cylinder address           */
};
/* fdmbuf.b_rberr/fdmbuf.b_seekerr/fdmbuf.b_rwerr */
#define MEDIARD		0x01
#define MEDIASEEK	0x01
#define SRETRY		0x03
#define MRETRY		0x30
#define LRETRY		0x300
#define SRMASK		0x0f
#define MRMASK		0xf0
#define RMRMASK		0xff0
#define LRMASK		0xf00
#define MINC		0x10
#define LINC		0x100

/* fdmbuf.b_active[] Values of buffer-header b_active,
 * used for mutual-exclusion of opens and other I/O requests. */
#define IO_IDLE		0		/* idle -- anything goes */
#define IO_BUSY		1		/* something going on */
#define IO_WAIT		2		/* waiting for controller to be idle */

struct	fdmbuf {
	int	mb_flags;		/* see buf.h			   */
	struct	fdubuf *b_unitf;	/* first buffer for this dev 	   */
	struct	fdubuf *b_unitb;	/* last buffer for this dev 	   */
	char	b_active[MAXUNIT];	/* busy flag 			   */
	char	b_unit;			/* for change unit		   */
	short	b_rberr;		/* rb error count (for recovery)   */
	short	b_seekerr;		/* seek error count (for recovery) */
	short	b_rwerr;		/* r/w error count (for recovery)  */
	short	b_status;		/* error status			   */
	struct	buf	*b_buf;		/* set bp address 		   */
	caddr_t b_xferaddr;		/* trasfer address		   */
	unsigned int b_xfercount;	/* total transfer count		   */
	unsigned int b_xferdma;		/* dma transfer count		   */
	daddr_t	b_sector;		/* read/write sector 		   */
	struct	fdcmd	b_cmd;		/* set command table address       */
};

#define FMTID	4
struct fmttbl {
	unsigned char cyl;
	unsigned char head;
	unsigned char sector;
	unsigned char s_type;
};

struct	fdpart {
	daddr_t p_fsec;		/* start sector number		*/
	daddr_t p_nsec;		/* disk length (sector count)	*/
};

#define FORMMAX	0x0e	/* support mediatype count */
struct	fddrtab {
	ushort	dr_ncyl;	/* cylinder count		 */
	char	dr_nrhead;	/* removable heads		 */
	char	dr_nsec;	/* sector per track		 */
	ushort	dr_secsiz;	/* sector_size (byte)		 */
	ushort	dr_spc;		/* actual sectors/cylinder	 */
	struct	fdpart *dr_part; /* start sector , disk length */
	char	dr_type;	/* media type			 */
	char	dr_rwgpl;	/* Read / Write Gap length	 */
	char	dr_fgpl;	/* Format Gap length		 */
};

struct	fdcmn {
	char	f_srthut;	/* Step rate , Head unload time  */
	char	f_hltnd;	/* Head load time , Non DMA mode */
	char	f_hst;		/* Head Setting time		 */
	char	f_dtl;		/* Data length			 */
};

struct	fdtree {
	struct	fddrtab *fd_parm;  /* parameter address   */
	int	cylno;		   /* read cylnder number */
	int	headno;		   /*      head number    */
	int	secno;		   /*      sector number  */
	struct	fdtree *fd_yes;    /* next tree	       */
	struct	fdtree *fd_no;	   /* next tree	       */
};

struct	fddev {
	struct	fddrtab d_drtab[MAXUNIT];	/* floppy disk parameter */
	struct	fdmbuf  d_bufh;		/* pointer to regular buffer queue */
};



#define HZ		100		/* 100 ticks/second of the clock */
#define NBPSCTR		512		/* Bytes per LOGICAL disk sector */
					/* These should be added to 
                                                          "sys/param.h". */

#define V_SETPARMS	_IO('v',14)	/* Set drivers parameters */

#define PAGESIZ		4096

/*
 * The following is a temporary kludge. B_VERIFY and B_FORMAT are
 * not defined in osf's buf.h. We'll employ unused bits for this release
 * but must fix this for future releases for obvious reasons.
 */
#define B_VERIFY   0x80000000
#define B_FORMAT   0x40000000


/*
 * Define driver specific buf fields.
 */

#define 	b_pfcent  b_driver_un_2.longvalue
