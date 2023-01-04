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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: m765knl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:46 $";
#endif 
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

#include <sys/types.h>
#include <sys/table.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <ufs/dir.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <i386/ipl.h>
#include <i386/AT386/atbus.h>
#include <i386/AT386/m765.h>
#include <i386/AT386/disk.h>
#include <i386/handler.h>
#include <i386/dispatcher.h>
#include <sys/uio.h>

#define NOP_DELAY {asm("nop");}

#define CMOS_ADDR	0x70	/* I/O port address for CMOS ram addr */
#define CMOS_DATA	0x71	/* I/O port address for CMOS ram data */
#define FDTBL		0x10	/* byte offset of the disk type in CMOS ram */

extern	struct	fddev	m765dev;	/* device data table */
extern	struct	fdubuf unitbuf[];	/* unit buffer table */
extern	struct	fddrtab m765f0[];	/* format search table */
extern	struct	fddrtab m765f1[];	/* format search table */
extern  struct  fddrtab fd3_5, fd5_25; /* Device data templates */
extern	int	m765verify[];		/* write after read flag */
extern	struct	fdcmn m765fdata;	/* fdc parameter */
extern	unsigned char	kbd_FDsts;	/* fd_jumper data */
extern	char	fddtype;		/* external fd type */
extern	char	fdtype[];		/* CMOS fd type */
extern	char	*fderr;			/* error message */
extern	char	*fdmsg[];		/* error message */
extern	int	dmause;			/* flag for dma */
extern	struct fdtree  *dchgchk();	/* check unit_change */
extern  struct fdtree  *dchgchk_new();	/* new dchgchk() */	
extern	struct fddrtab *getparm();	/* get fdc parameter table */  
extern	struct fddrtab *getparm_new();	/* new get fdc parameter table */  
extern 	struct buf	fdrbuf[];
int	fdminphys();

int	fdintr(), fdprobe(), fdslave(), fdattach();

int	(*fdintrs[])() = { fdintr, 0 };

struct	isa_driver	fddriver = 
	{fdprobe, fdslave, fdattach, "fd", 0, 0, 0};

static ihandler_t fd_handler;
static ihandler_id_t *fd_handler_id;

fdprobe(ctlr)
struct isa_ctlr *ctlr;
{
	struct	fdmbuf *mbufh;
	register struct	fdubuf *ubufh,*wubufh;
	register int	cnt0;
        char dev_type;
        
	/* Driver supports only ONE controller */
	if (fd_handler_id)
		return 0;

	if ((inb(ctlr->ctlr_addr + STSREG - CTRLREG) & DATAOK) != DATAOK)
		return 0;

	fd_handler.ih_level = ctlr->ctlr_pic;
	fd_handler.ih_handler = ctlr->ctlr_intr[0];
	fd_handler.ih_resolver = i386_resolver;
	fd_handler.ih_rctlr = ctlr;
	fd_handler.ih_hparam[0].intparam = ctlr->ctlr_ctlr;
	fd_handler.ih_stats.intr_type = INTR_DEVICE;
	fd_handler.ih_stats.intr_cnt = 0;
	if ((fd_handler_id = handler_add(&fd_handler)) != NULL)
		handler_enable(fd_handler_id);
	else
		panic("Unable to add floppy disk interrupt handler");

	mbufh = &(m765dev.d_bufh);
	mbufh->b_unit = 0;			/* set unit_switch */
	mbufh->b_cmd.c_rbmtr = 0;	/* recalibrate/motor flag */
	mbufh->b_cmd.c_intr = CMDRST;		/* interrupt flag */
	wubufh = (struct fdubuf *)mbufh;
	for( cnt0 = 0 ; cnt0 < MAXUNIT ; cnt0++ ){
		ubufh = &unitbuf[cnt0];
		ubufh->b_unit = cnt0;
		ubufh->b_seekaddr = 0;
		ubufh->av_forw = ubufh->av_back = 0;
		mbufh->b_active[cnt0] = IO_IDLE;
		ubufh->b_unitb = wubufh;
		wubufh->b_unitf = ubufh;
		wubufh = ubufh;
	}
	ubufh->b_unitf = mbufh->b_unitf;	/* set unit_buffer_end */
	mbufh->b_rwerr = mbufh->b_seekerr = mbufh->b_rberr = 0;

	rstout(mbufh,0);	
	specify();

	return(1);
}

fdslave(dev)
struct isa_dev		*dev;
{
	int slave = dev->dev_slave;

	/* read cmos ram (address 0x10) */
        outb(CMOS_ADDR, FDTBL);
        NOP_DELAY
        fddtype = inb(CMOS_DATA);

        if (slave == 0)
                dev->dev_type = (fddtype & 0xf0) >> 4;
        else
                dev->dev_type = fddtype & 0x0f;

        fdtype[slave] = dev->dev_type;
        
 /*
        fddtype = 0x20;

	if((fddtype&0x0f)==0x04){
		fddtype &= 0xf0;
		fddtype |= fddtype >> 4;
	} else {
		fddtype &= 0x0f;
		fddtype |= fddtype << 4;
	}
*/
        
        if (dev->dev_type && slave < 2)
                return(1);

        return(0);
}

fdattach(dev)
struct isa_dev *dev;
{
	int unit = dev->dev_unit;
        uchar type = dev->dev_type;
        
	printf("fd%d: (", unit);

        switch(type) {
        case 0:
                break;
        case 1:
                printf("360Kb 5.25 inch");
                break;
        case 2:
                printf("1.2Mb 5.25 inch");
                break;
        case 4:
                printf("1.44Mb 3.5 inch");
                break;
        default:
                printf("Unknown %x", type);
                break;
        }
        printf(") irq = %d\n", dev->dev_mi->ctlr_pic);
        
        /* Setup correct drive type */
        if (type == 4)
                bcopy(&fd3_5, &m765dev.d_drtab[unit], sizeof(fd3_5));
        else
                bcopy(&fd5_25, &m765dev.d_drtab[unit], sizeof(fd5_25));
                
	BUF_LOCKINIT(&fdrbuf[unit]);
}

/*****************************************************************************
 *
 * TITLE:	fdopen
 *
 * ABSTRACT:	Open a unit. 
 *
 ****************************************************************************/
fdopen(dev, flag, otyp)
dev_t	dev;
int	flag;			/* not used */
int	otyp;			/* not used */
{
	register struct fddev *dd;
	register struct	fdmbuf *mbufh;
	struct	fdubuf *ubufh;
	register unsigned unit;
	register int error;

	dd = &m765dev;
	mbufh = &(dd->d_bufh);
	unit = UNIT(dev);
	error = ESUCCESS;
	if (unit < MAXUNIT && dd->d_drtab[unit].dr_part) {
		ubufh = mbufh->b_unitf;
		while(ubufh->b_unit != unit)
			ubufh = ubufh->b_unitf;	
		error = fdchk(dd,mbufh,ubufh,dev);
	} else
		error = ENXIO;		/* No such unit */
	return(error);
}
/************************************************************************
 *
 *	floppy disk media type check and dooropen check
 *
 ************************************************************************/
fdchk(dd,mbufh,ubufh,dev)
register struct	fddev *dd;
register struct	fdmbuf *mbufh;
register struct	fdubuf *ubufh;
dev_t	dev;
{
	unsigned int	unit,x;
	int	rtn;
	unsigned char	kbd_sts;

	x = SPL();
	openchk(mbufh);
	mbufh->b_cmd.c_devflag |= FDMCHK;
	chkbusy(mbufh);
	splx(x);
	/*
	kbc_ctrl(GETFDSTS,0,&kbd_sts);	** read keyboard_io (0xb4 command) **
	*/
	kbd_sts = 0x1e;
	if(kbd_sts != kbd_FDsts){
		for( x = 0 ; x < MAXUNIT ; x++ )
			dd->d_drtab[x].dr_type &= ~OKTYPE; 
		kbd_FDsts = kbd_sts;
	}


	unit = UNIT(dev);
	mbufh->b_cmd.c_stsflag |= MTRFLAG;
	mtr_on(mbufh,unit);			/* motor on	*/

	rtn = fdopenchk(dd,mbufh,unit);        

	mbufh->b_cmd.c_stsflag &= ~MTRFLAG;
	mtr_on(mbufh,unit);			/* motor on	*/

	x = SPL();
	openfre(mbufh);
	splx(x);

	if((!(rtn))        /* if (rtn == OK) */
		&&((ext2dchk(unit))||(!(dd->d_drtab[unit].dr_type & OKTYPE)))){
		mediatype(dd,dev,mbufh,ubufh);
	}	
	return(rtn);
}

/**************************************************************************
 *
 *	Check door open routine
 *
 **************************************************************************/
fdopenchk(dd,mbufh,unit)
register struct	fddev	 *dd;
register struct	fdmbuf *mbufh;
register unsigned int	 unit;
{
	struct	fddrtab *dr;
	int	opendata;

	dr = &dd->d_drtab[unit];
	if(!(extchk(unit))){              
		opendata = inb(VFOREG);
		mbufh->b_unit = unit;
		if(extfdtype(mbufh,unit)){   
			if((!(ext2dchk(unit))) 
 	  		  &&((opendata & OPENBIT)||(!(dr->dr_type & OKTYPE)))){
				dr->dr_type &= ~OKTYPE;
				if(!(rbrate( mbufh , 0x08 , unit )))
					fdseek(mbufh , 0x08 , unit , 2);
					if(!(inb(VFOREG) & OPENBIT))
						return(0);
			} else
				return(0);
		} 
	}
	return(ENXIO);
}

/**************************************************************************
 *
 *	Check external floppy disk drive type routine
 *
 **************************************************************************/
extfdtype(mbufh,unit)
register struct	fdmbuf *mbufh;
register unsigned int	 unit;
{
	if(((kbd_FDsts&FDCHG)&&unit)||((!(kbd_FDsts&FDCHG))&&(!unit))){
		if(!fddtype){
			if(rbrate( mbufh , NMSEEK , unit ))
				return(0);
			fdseek( mbufh , NMSEEK , unit , 51 );
			fdseek( mbufh , NMSEEK , unit , 11 );
			fdseek( mbufh , NMSEEK , unit , 10 );
			if(sds(unit) & TRACK0)
				fddtype = EXT2D;
			else
				fddtype = EXT2HD;
		}
	}
	return(1);
}
/**************************************************************************
 *
 *	Media type check routine
 *
 *	set floppy parameter to m765dev.d_drtab[unit].
 *
 **************************************************************************/
mediatype(dd,dev,mbufh,ubufh)
struct	fddev *dd;
dev_t	dev;
register struct	fdmbuf *mbufh;
struct	fdubuf *ubufh;
{
	register struct	fdtree *tree;
	struct	fddrtab *dr;
	register struct	buf	*wbp;

	dr = &dd->d_drtab[UNIT(dev)];
	tree = dchgchk_new(dev);        /* MODIFIED BY KOICHI YAMADA */
	wbp = geteblk(BLKSIZE);		/* get struct buf area */
	while(tree->fd_yes){
		m765sweep(dd, UNIT(dev), tree->fd_parm);
		mbufh->b_cmd.c_rbmtr &= ~(1<<(RBSHIFT+UNIT(dev)));
		++mbufh->b_rwerr;
		clearbp(wbp,dev);
		wbp->b_flags = (B_READ|B_VERIFY);
		wbp->b_bcount  = dr->dr_secsiz;
		wbp->b_pfcent = (long)((tree->cylno*dr->dr_spc)
				+(tree->headno*dr->dr_nsec)+tree->secno-1);
		setqueue(dd,mbufh,ubufh,wbp,UNIT(dev));
		biowait(wbp);
		if(wbp->b_error)
			tree = tree->fd_no;
		else
			tree = tree->fd_yes;
	}
	brelse(wbp);
	if(tree->fd_parm){
		m765sweep(dd, UNIT(dev), tree->fd_parm);
		dr->dr_type |= OKTYPE;
	}
	else
		fddtype = 0;
	return(0);
}


/*********************************************************************
 *
 *	set cylno,headno,secno to command table
 *
 *********************************************************************/
clearbp(bp,dev)
register struct buf *bp;
register dev_t	dev;
{
	event_clear(&bp->b_iocomplete);
	bp->b_dev = dev;
	bp->b_proc = 0; bp->b_error = 0; bp->b_resid = 0;
}
/*****************************************************************************
 *
 * TITLE:	fdclose
 *
 * ABSTRACT:	Close a unit.
 *
 *	Called on last close. mark the unit closed and not-ready.
 *
 *****************************************************************************/
fdclose(dev, flag, otyp, offset)
dev_t	dev;		/* major, minor numbers */
int	flag;		/* not used */
int	otyp;		/* not used */
off_t	offset;		/* not used */
{
	register struct fddev	*dd;
	register struct fdmbuf *mbufh;
	register struct fdubuf *ubufh;
	unsigned int	x;
	unsigned int	unit = UNIT(dev);

	dd = &m765dev;
	mbufh = &(dd->d_bufh);

	/* Clear the bit.
	 * If last close of drive insure drtab queue is empty before returning.
	 */
	ubufh = mbufh->b_unitf;
	while(ubufh->b_unit != unit)
		ubufh = ubufh->b_unitf;	
	x = SPL();
	while(ubufh->av_forw != NULL) {
		mbufh->b_active[unit] |= IO_WAIT;
		sleep(&mbufh->b_active[unit], PRIBIO);
	}
	splx(x);
	return(ESUCCESS);
}
/*****************************************************************************
 *
 * TITLE:	fdstrategy
 *
 * ABSTRACT:	Queue an I/O Request, and start it if not busy already.
 *
 *	Reject request if unit is not-ready.
 *
 *	Note:	check for not-ready done here ==> could get requests
 *		queued prior to unit going not-ready.
 *		not-ready status to those requests that are attempted
 *		before a new volume is inserted.  Once a new volume is
 *		inserted, would get good I/O's to wrong volume.
 * 
 *		OSF requirement, john dugas august 2, 1990.
 *		The interlude is necessary because physio assumes the
 *		driver is doing dma directly into the user's address
 *		space. This driver does not. The user's address may 
 *		or may not be valid at interrrupt time. do_pohysio sets 
 *		up the tranfer into a kernel buffer, calls the real
 *		strategy routine and does the right thing with the
 *		data. 
 *
 * CALLS:	m765strchk(),iodone(),setqueue()
 *
 * CALLING ROUTINES:	fdread (indirectly, thru physio)
 *			fdwrite (indirectly, thru physio)
 *
 ****************************************************************************/


fdstrategy2(bp)
struct buf *bp;	/* buffer header */
{
	register struct fddev	 *dd;
	register struct	fdubuf *ubufh;
	register struct	fdmbuf *mbufh;
	unsigned int	unit;

	/* initializations */
	dd = &m765dev;
	unit = UNIT(bp->b_dev);
	mbufh = &(dd->d_bufh);
	ubufh = mbufh->b_unitf;
	while(ubufh->b_unit != unit)
		ubufh = ubufh->b_unitf;	
	bp->b_error = 0;
	if(m765strchk(dd,bp,bp->b_dev)){	/* check parameter error */
		biodone(bp);
		return(0);
	}
	setqueue(dd,mbufh,ubufh,bp,unit);	/* set queue to buffer */
	return(0);
}

fdstrategy(bp)
struct buf *bp;
{
	
	if(bp->b_flags & B_PHYS) {
		do_physio(bp,fdstrategy2);
		return;
	}
	fdstrategy2(bp);
}

/**************************************************************************
 *
 *	strategy error check routine
 *
 **************************************************************************/
m765strchk(dd,bp,dev)
register struct fddev	 *dd;
register struct buf *bp;
register dev_t	dev;
{
	struct	fddrtab *dr,*sdr;
	struct	fdpart *p;
	unsigned        bytes_left;
	daddr_t		secno;
	int	type;
	daddr_t		psecno;
	unsigned int	unit;
	unit = UNIT(dev);

	dr = &dd->d_drtab[unit];
	/* set b_resid to b_bcount because we haven't done anything yet */
	bp->b_resid = bp->b_bcount;

	if (!(dr->dr_type & OKTYPE)){
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
	/*	cmn_err(CE_CONT,"%s %d : %s\n",fderr,unit,fdmsg[ILLFMT]); */
		printf("%s %d : %s\n",fderr,unit,fdmsg[ILLFMT]);
		return(ERROR);
	}

	if(type = MEDIATYPE(bp->b_dev)){
		if(!(sdr = getparm_new(type,unit))){
			bp->b_flags |= B_ERROR;
			bp->b_error = EIO;
		/*	cmn_err(CE_CONT,"%s %d : %s\n",fderr,unit,fdmsg[ILLFMT]); */
			printf("%s %d : %s\n",fderr,unit,fdmsg[ILLFMT]);
			return(ERROR);
		}
		if(cmpparm(dr,sdr)){
			bp->b_flags |= B_ERROR;
			bp->b_error = EIO;
		/*	cmn_err(CE_CONT,"%s %d : %s\n",fderr,unit,fdmsg[ILLFMT]); */
			printf("%s %d : %s\n",fderr,unit,fdmsg[ILLFMT]);
			return(ERROR);
		}
	}
	p = &dr->dr_part[PARTITION(bp->b_dev)];
	/*
	 * Figure "secno" from b_blkno. Adjust sector # for partition.
	 *
	 * If reading just past the end of the device, it's
	 * End of File.  If not reading, or if read starts further in
	 * than the first sector after the partition, it's an error.
	 *
	 * secno is logical blockno / # of logical blocks per sector */
	secno = bp->b_blkno;
	secno = secno * NBPSCTR / dr->dr_secsiz;
	psecno = p->p_nsec;
	if(CYLFLAG(dev)&&(dr->dr_ncyl==80))
		psecno -= (dr->dr_spc * 3);
	if (secno >= psecno) {
		if (!((bp->b_flags & B_READ) && (secno == psecno))){
			/* off the deep end */
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		return(ERROR);
	}
/* At this point, it is no longer possible to directly return from strategy.
   We now set b_resid to the number of bytes we cannot transfer because
   they lie beyond the end of the request's partition.  This value is 0
   if the entire request is within the partition. */
	bytes_left = (p->p_nsec - secno) * dr->dr_secsiz;
	bp->b_resid = ((bp->b_bcount <= bytes_left)
			? 0 : (bp->b_bcount - bytes_left));

	secno += p->p_fsec;
	bp->b_pfcent = (long)secno;
	return(OK);
}
/***************************************************************************
 *
 *	set queue to buffer
 *
 ***************************************************************************/
setqueue(dd,mbufh,ubufh,bp,unit)
register struct	fddev	*dd;
register struct	fdmbuf *mbufh;
register struct	fdubuf *ubufh;
struct buf *bp;
unsigned int	unit;
{
	unsigned int	x;
	int	count;

	x = SPL();
	openchk(mbufh);					/* openning check */
	mbufh->b_cmd.c_devflag |= STRCHK;
	fd_disksort(ubufh,bp,dd->d_drtab[unit].dr_nsec); /* queue the request */
	/*
	 * If no requests are in progress, start this one up.  Else
	 * leave it on the queue, and fdintr will call m765io later.
	 */
	for ( count = MAXUNIT ; count > 0 ; count--){
		if(mbufh->b_active[count-1] != IO_IDLE)
			break;
	}
	mbufh->b_active[unit] |= IO_BUSY;
	if(!(count)){
		/*
		 * Set b_active flag to indicate controller is busy.  No one
		 * should make requests to the controller while this is set.
	 	 * It gets cleared right above here, usually when fdintr
		 * calls m765io to process next request and the queue is empty.
		 */
		dmaget();
		m765io(dd,ubufh,unit);
	}
	splx(x);
}

/***************************************************************************
 *
 *	check io_busy routine
 *
 ***************************************************************************/
chkbusy(mbufh)
register struct	fdmbuf *mbufh;
{
	while(mbufh->b_cmd.c_devflag & STRCHK ){
		mbufh->b_cmd.c_devflag |= STRWAIT;
		sleep(&(mbufh->b_cmd.c_devflag),PZERO);
	} 
}
/***************************************************************************
 *
 *	free open routine
 *
 ***************************************************************************/
openstr(mbufh)
register struct	fdmbuf *mbufh;
{
	mbufh->b_cmd.c_devflag &= ~STRCHK;
	if(mbufh->b_cmd.c_devflag & STRWAIT){
		mbufh->b_cmd.c_devflag &= ~STRWAIT;
		wakeup(&(mbufh->b_cmd.c_devflag));
	}
}

/***************************************************************************
 *
 *	check fdopen() routine
 *
 ***************************************************************************/
openchk(mbufh)
register struct	fdmbuf *mbufh;
{
	while(mbufh->b_cmd.c_devflag & FDMCHK ){
		mbufh->b_cmd.c_devflag |= FDWAIT;
		sleep(&(mbufh->b_cmd.c_devflag),PZERO);
	} 
}

/***************************************************************************
 *
 *	free fdopen() routine
 *
 ***************************************************************************/
openfre(mbufh)
register struct	fdmbuf *mbufh;
{
	mbufh->b_cmd.c_devflag &= ~FDMCHK;
	if(mbufh->b_cmd.c_devflag & FDWAIT){
		mbufh->b_cmd.c_devflag &= ~FDWAIT;
		wakeup(&(mbufh->b_cmd.c_devflag));
	}
}
/*****************************************************************************
 *
 * TITLE:	m765io
 *
 * ABSTRACT:	Start handling an I/O request.
 *
 ****************************************************************************/
m765io(dd,ubufh,unit)
struct fddev *dd;
struct	fdubuf *ubufh;
register unsigned int	unit;
{
	extern  int(m765iosub)();
	register struct fdmbuf *mbufh;
	register struct buf *bp;

	mbufh = &(dd->d_bufh);
	bp = ubufh->av_forw;		/* move bp to mbufh->b_buf */
	mbufh->b_buf = bp;
	mbufh->b_unit = unit;

	mbufh->b_xferaddr  = bp->b_un.b_addr;
	mbufh->b_xfercount = bp->b_bcount - bp->b_resid;
	mbufh->b_sector    = (daddr_t)bp->b_pfcent;

	mbufh->b_cmd.c_stsflag |= MTRFLAG;
	if(!(mtr_start(mbufh,unit)))
		timeout(m765iosub,dd,HZ);
	else
		m765iosub(dd);
}
/****************************************************************************
 *
 *	m765io subroutine
 *
 ****************************************************************************/
m765iosub(dd)
struct fddev  *dd;
{
	register struct fdmbuf *mbufh;
	register struct fdubuf *ubufh;
	register unsigned int	unit;
	struct fddrtab *dr;
	int	startsec,lastsec;

	mbufh = &(dd->d_bufh);
	unit = mbufh->b_unit;

	ubufh = mbufh->b_unitf;
	while(ubufh->b_unit != unit)
		ubufh = ubufh->b_unitf;	
	dr = &dd->d_drtab[unit];

	rwcmdset(dr,mbufh,unit,mbufh->b_sector);
	if(mbufh->b_buf->b_flags&B_FORMAT)
		goto skipchk;
	startsec = (mbufh->b_cmd.c_rwdata[3] * dr->dr_nsec) 
			+ mbufh->b_cmd.c_rwdata[4];
	lastsec = startsec+(mbufh->b_xfercount/dr->dr_secsiz)-1;
	if(lastsec > dr->dr_spc){
		mbufh->b_xferdma = 
			(dr->dr_spc-startsec+1) * dr->dr_secsiz;
	} else
skipchk:	mbufh->b_xferdma = mbufh->b_xfercount;
	if(!(mbufh->b_cmd.c_rbmtr & (1<<(RBSHIFT+unit))))
       		mbufh->b_status=rbirate(mbufh,unit,dr->dr_type);
	else {
		if(ubufh->b_seekaddr != mbufh->b_cmd.c_saddr)
			mbufh->b_status =
				fdiseek(dd,unit,mbufh->b_cmd.c_saddr);
		else {
			mbufh->b_status = outicmd(dd,mbufh,unit);
		}
	}
	if(mbufh->b_status)
		intrerr0(dd,mbufh,unit);
	return;
}
/***************************************************************************
 *
 *	read / write / format / verify command set to command table
 *
 ***************************************************************************/
rwcmdset(dr,mbufh,unit,sector)
register struct fddrtab *dr;
register struct fdmbuf *mbufh;
unsigned unit;
daddr_t	sector;
{
	short	resid;
	register struct fdcmd *cmd;

	cmd = &mbufh->b_cmd;
	switch(mbufh->b_buf->b_flags&(B_FORMAT|B_VERIFY|B_READ|B_WRITE)){
	case B_VERIFY|B_WRITE:	/* VERIFY after WRITE */
		cmd->c_rwdata[0] = RDMV;
		break;

	case B_FORMAT:
		cmd->c_dcount = FMTCNT; 
		cmd->c_rwdata[0] = FMTM;
		cmd->c_saddr = sector / dr->dr_spc;
		resid = sector % dr->dr_spc;
		cmd->c_rwdata[1] = unit|((resid/dr->dr_nsec)<<2);
		cmd->c_rwdata[2] = 
			((struct fmttbl *)mbufh->b_buf->b_un.b_addr)->s_type;
		cmd->c_rwdata[3] = dr->dr_nsec;
		cmd->c_rwdata[4] = dr->dr_fgpl;
		cmd->c_rwdata[5] = FMTDATA;
		break;

	case B_WRITE:
	case B_READ:
	case B_READ|B_VERIFY:
		cmd->c_dcount = RWCNT;
		if(mbufh->b_buf->b_flags&B_READ)
			if(mbufh->b_buf->b_flags&B_VERIFY)
				cmd->c_rwdata[0] = RDMV;
			else
				cmd->c_rwdata[0] = RDM;
		else {
			cmd->c_rwdata[0] = WTM;	/* format or write */
		}
		resid = sector % dr->dr_spc;
		cmd->c_rwdata[3] = resid / dr->dr_nsec;
		cmd->c_rwdata[1] = unit|(cmd->c_rwdata[3]<<2);
		cmd->c_saddr = 
		cmd->c_rwdata[2] = sector / dr->dr_spc;
		cmd->c_rwdata[4] = (resid % dr->dr_nsec) + 1;
		if((cmd->c_rwdata[5] = dr->dr_secsiz>>8) == 4)
			cmd->c_rwdata[5] = 3;
		cmd->c_rwdata[6] = dr->dr_nsec;
		cmd->c_rwdata[7] = dr->dr_rwgpl;
		cmd->c_rwdata[8] = m765fdata.f_dtl;
		break;
	}
}

/*****************************************************************************
 *
 * TITLE:	fdread
 *
 * ABSTRACT:	"Raw" read.  Use physio().
 *
 * CALLS:	fdstrategy (indirectly, thru physio)
 *
 ****************************************************************************/
fdread(dev, uio)
register dev_t	dev;
struct uio *uio;
{ 
	struct buf *bp;
	int error;

	bp = &fdrbuf[UNIT(dev)];
	BUF_LOCK(bp);

	error = physio(fdstrategy, bp, dev, B_READ, fdminphys, uio);

	BUF_UNLOCK(bp);
	return(error);
}

/*****************************************************************************
 *
 * TITLE:	fdwrite
 *
 * ABSTRACT:	"Raw" write.  Use physio().
 *
 * CALLS:	fdstrategy (indirectly, thru physio)
 *
 ****************************************************************************/
fdwrite(dev, uio)
register dev_t	dev;
struct uio *uio;
{
	struct buf *bp;
	int error;

	bp = &fdrbuf[UNIT(dev)];
	BUF_LOCK(bp);

	error = physio(fdstrategy, bp, dev, B_WRITE, fdminphys, uio);

	BUF_UNLOCK(bp);
	return(error);
}
/*****************************************************************************
 *
 * TITLE:	fdminphys
 *
 * ABSTRACT:	Trim buffer length if buffer-size is bigger than page size
 *
 * CALLS:	physio
 *
 ****************************************************************************/
fdminphys(bp)
struct buf	*bp;
{
	if (bp->b_bcount > PAGESIZ)
		bp->b_bcount = PAGESIZ;
}
/*****************************************************************************
 *
 * TITLE:	fdioctl
 *
 * ABSTRACT:	m765 driver special functions.
 *
 * CALLING ROUTINES:	kernel
 *
 ****************************************************************************/
int 
fdioctl(dev, cmd, cmdarg, flag)
dev_t	dev;		/* major, minor numbers */
int	cmd;		/* command code */
int	*cmdarg;	/* user structure with parameters */
int	flag;		/* not used */
{
	register struct fddev *dd;
	register struct fdmbuf *mbufh;
	struct fdubuf *ubufh;
	register unsigned unit;

	dd = &m765dev;
	unit = UNIT(dev);
	mbufh = &(dd->d_bufh);
	ubufh = mbufh->b_unitf;
	while(ubufh->b_unit != unit)
		ubufh = ubufh->b_unitf;	
	switch (cmd) {
		case V_SETPARMS:    /* Caller wants reset_parameters */
			return(fd_setparms(dd,mbufh,unit,*cmdarg));
		case V_GETPARMS:    /* Caller wants device parameters */
			return(fd_getparms(dd,dev,cmdarg));
		case V_FORMAT:
			return(fd_format(dd,dev,cmdarg));
		case V_VERIFY:	/* cmdarg : 0 == not verify mode */
				/*	    0 != verify mode     */
			m765verify[unit] = *cmdarg;
			return(0);
		default:
			return(EINVAL);
	}
}
/****************************************************************************
 *
 *	set fd parameters 
 *
 ****************************************************************************/
int
fd_setparms(dd,mbufh,unit,cmdarg)
register struct fddev *dd;
struct fdmbuf *mbufh;
register unsigned int unit;
long cmdarg;
{
	register struct fddrtab *dr;
	int	type;
	struct fddrtab *fdparm;
	unsigned int x;

	dr = &dd->d_drtab[unit];
	mbufh->b_cmd.c_rbmtr &= ~(1<<(RBSHIFT+unit));
	extfdtype(&dd->d_bufh,unit);
	if(type = MEDIATYPE(cmdarg)){
		if((fdparm = getparm_new(type,unit))
				== (struct fddrtab *)ERROR){
			return(EINVAL);
		}
	} else {
	   	if(dunitchk(unit)){
			if(ext2dchk(unit))
				fdparm = &m765f1[10];
			else
				fdparm = &m765f1[2];
		} else
			fdparm = &m765f0[0];
	}
	x = SPL();
	openchk(mbufh);
	mbufh->b_cmd.c_devflag |= FDMCHK;
	chkbusy(mbufh);
	m765sweep(dd, unit, fdparm);
	dr->dr_type |= OKTYPE;
	openfre(mbufh);
	splx(x);
	return(0);
}
/****************************************************************************
 *
 *	get fd parameters 
 *
 ****************************************************************************/
int
fd_getparms(dd,dev,cmdarg)
struct fddev *dd;
dev_t	dev;		/* major, minor numbers */
int	*cmdarg;
{
	register struct fddrtab *dr;
	struct disk_parms *diskp;
	register struct fdpart *p;

	diskp = (struct disk_parms *)cmdarg;

	dr = &dd->d_drtab[UNIT(dev)];
	p = &dr->dr_part[PARTITION(dev)];
	if(dr->dr_type & OKTYPE){
		diskp->dp_type = DPT_FLOPPY;
		diskp->dp_heads = dr->dr_nrhead;
		diskp->dp_sectors = dr->dr_nsec;
		diskp->dp_secsiz = dr->dr_secsiz;
		diskp->dp_pstartsec = p->p_fsec;
		if(CYLFLAG(dev)&&(dr->dr_ncyl==80)){
			diskp->dp_cyls = CYL77;
			diskp->dp_pnumsec = 
				p->p_nsec - (dr->dr_spc * 3);
		} else {
			diskp->dp_cyls = dr->dr_ncyl;
			diskp->dp_pnumsec = p->p_nsec;
		}
		diskp->dp_dosheads = dr->dr_nrhead;
		diskp->dp_doscyls = dr->dr_ncyl;
		diskp->dp_dossectors  = dr->dr_nsec;
		/* Put parameters into user segment */
		/*
		if(copyout((caddr_t)&diskp, cmdarg, (unsigned)sizeof(struct disk_parms))){
			return(EFAULT);
		}
		*/
	} else {
		return(ENXIO);
	}
	return(0);
}
/****************************************************************************
 *
 *	format command
 *
 ****************************************************************************/
fd_format(dd,dev,cmdarg)
struct fddev *dd;
dev_t	dev;		/* major, minor numbers */
int	*cmdarg;
{
	register struct fddrtab *dr;
	register struct buf *bp;
	register daddr_t track;
	union  io_arg  *varg;
	ushort	num_trks;

	dr = &dd->d_drtab[UNIT(dev)];
	if(!(dr->dr_type & OKTYPE)) {
		return(EINVAL);	
	}
	/*
	if(copyin(cmdarg,(caddr_t)&varg,(unsigned)sizeof(union io_arg))){
		return(EFAULT);
	}
	*/
	varg = (union io_arg *)cmdarg;

	num_trks = varg->ia_fmt.num_trks;
	track = (daddr_t)(varg->ia_fmt.start_trk*dr->dr_nsec);
	varg->ia_fmt.start_trk += dr->dr_part[PARTITION(dev)].p_fsec/dr->dr_nsec;
	if((track + (num_trks*dr->dr_nsec))>dr->dr_part[PARTITION(dev)].p_nsec){
		return(EINVAL);
	}
	bp = geteblk(BLKSIZE);		/* get struct buf area */
	for( ; num_trks > 0 ; num_trks-- , track += dr->dr_nsec ){
		clearbp(bp,dev);
		bp->b_flags = B_FORMAT;	
		bp->b_bcount  = dr->dr_nsec * FMTID;
		bp->b_blkno = (daddr_t)(track * dr->dr_secsiz / NBPSCTR);
		if(makeidtbl(bp->b_un.b_addr,dr,
				varg->ia_fmt.start_trk++,varg->ia_fmt.intlv)) {
			return(EINVAL);
		}
		fdstrategy(bp);
		biowait(bp);
		if(bp->b_error){
			if(( bp->b_error == (char)EBBHARD )
				||( bp->b_error == (char)EBBSOFT ))
				return(EIO);
			else
				return(bp->b_error);
			break;
		}
	}
fmterr:	brelse(bp);
	return(0);	
}
/****************************************************************************
 *
 *	make id table for format
 *
 ****************************************************************************/
makeidtbl(tblpt,dr,track,intlv)
struct fmttbl *tblpt;
struct fddrtab *dr;
unsigned short track;
unsigned short intlv;
{
	register int	i,j,secno;
	char	cyl,head,s_type;

/*	if(!((0 < intlv)&&(intlv < dr->dr_nsec)))	*/
	if(!(intlv < dr->dr_nsec))
		return(1);
	for( i = 0 ; i < dr->dr_nsec ; i++ )
		tblpt[i].sector = 0;
	cyl = track / dr->dr_nrhead;
	head = track % dr->dr_nrhead;
	if((s_type = dr->dr_secsiz>>8) == 4)
		s_type = 3;
	for(i = 0 , j = 0 , secno = 1 ; i < dr->dr_nsec ; i++){
		tblpt[j].cyl = cyl;
		tblpt[j].head = head;
		tblpt[j].sector = secno++;
		tblpt[j].s_type = s_type;
		j += intlv;
		if(j < dr->dr_nsec ){
			continue;
		}
		j -= dr->dr_nsec;
		for( ; j < dr->dr_nsec ; j++ ){
			if(tblpt[j].sector == 0)
				break;
		}
	}
	return(0);
}
/*****************************************************************************
 *
 * TITLE:	fdintr
 *
 * ABSTRACT:	Handle interrupt.
 *
 *	Interrupt procedure for m765 driver.  Gets status of last
 *	operation and performs service function according to the
 *	type of interrupt.  If it was an operation complete interrupt,
 *	switches on the current driver state and either declares the
 *	operation done, or starts the next operation
 *
 ****************************************************************************/
fdintr(level)
int	level;			/* interrupt level (Not used)*/
{
	extern  int(m765intrsub)();
	register struct	fddev		*dd;
	register struct	fdmbuf	*mbufh;
	register struct fdubuf	*ubufh;
	struct fddrtab *dr;
	unsigned int	unit;

#if	0
	if (first_fdopen_ever) {
		printf("fdintr: fdinit() hasn't been called yet.\n");
		fdinit();
		first_fdopen_ever = 0;
	}
#endif
	dd = &m765dev;
	mbufh = &(dd->d_bufh);
	unit = mbufh->b_unit;
	dr = &dd->d_drtab[unit];
	if((level != RWLEVEL)&&(mbufh->b_cmd.c_stsflag & INTROUT)){
		untimeout(fdintr, RWLEVEL);
#if	0
		untimeout(mbufh->b_cmd.c_timeid);
#endif
	}
	mbufh->b_cmd.c_stsflag &= ~INTROUT;	
	ubufh = mbufh->b_unitf;
	while(ubufh->b_unit != unit)
		ubufh = ubufh->b_unitf;	
	switch(mbufh->b_cmd.c_intr){
		case RWFLAG:
			rwintr(dd,mbufh,ubufh,unit,dr);
			break;	

		case SKFLAG:
		case SKEFLAG|SKFLAG:
		case RBFLAG:
			timeout(m765intrsub,dd,SEEKWAIT);
			break;

		case WUPFLAG:
			mbufh->b_cmd.c_intr &= ~WUPFLAG;
			wakeup(mbufh);
			break;
		default:
		/*	cmn_err(CE_WARN,"%s %d : %s",fderr,unit,fdmsg[ILLINT]); */
		/*	printf("%s %d : %s",fderr,unit,fdmsg[ILLINT]);
			printf("interrupt type is %d\n",mbufh->b_cmd.c_intr); */
			break;
	}
	return(1);
}

/*****************************************************************************
 *
 *	interrup subroutine (seek recalibrate)
 *
 *****************************************************************************/
m765intrsub(dd)
register struct	fddev		*dd;
{
register struct	fdmbuf	*mbufh;
register struct fdubuf	*ubufh;
unsigned int	unit;

	mbufh = &(dd->d_bufh);
	unit = mbufh->b_unit;
	ubufh = mbufh->b_unitf;
	while(ubufh->b_unit != unit)
		ubufh = ubufh->b_unitf;	
	mbufh->b_status = sis();
	if(mbufh->b_status !=  ST0OK){
		switch(mbufh->b_cmd.c_intr){
			case SKFLAG:
				seekintr(dd,mbufh,ubufh,unit);
				break;

			case SKEFLAG|SKFLAG:
				seekintre(dd,mbufh,ubufh,unit);
				break;

			case RBFLAG:
				rbintr(dd,mbufh,ubufh,unit);
				break;
		}
	}
}
/*****************************************************************************
 *
 *	read / write / format / verify interrupt routine
 *
 *****************************************************************************/
rwintr(dd,mbufh,ubufh,unit,dr)
register struct	fddev	*dd;
register struct	fdmbuf	*mbufh;
register struct fdubuf	*ubufh;
unsigned	unit;
struct fddrtab *dr;
{
	mbufh->b_cmd.c_intr &= ~RWFLAG;
	if(openrtry(dd,mbufh,unit))
		return;
	if(mbufh->b_status = rwstschk())
		rwierr(dd,mbufh,unit,dr);
	else {
		/* write command */
		if((mbufh->b_buf->b_flags&(B_FORMAT|B_READ|B_WRITE))==B_WRITE){
			if(m765verify[unit])
				if(wverify(dd,dr,mbufh,unit))
					return;
		}
		/* clear retry count */
		mbufh->b_rwerr = mbufh->b_seekerr = mbufh->b_rberr = 0;
		mbufh->b_xfercount -= mbufh->b_xferdma;
		mbufh->b_xferaddr += mbufh->b_xferdma;
		mbufh->b_sector =
			(daddr_t)mbufh->b_sector+(mbufh->b_xferdma/dr->dr_secsiz);
		/* next address ( cyl,head,sec ) */
		if((int)mbufh->b_xfercount>0){
			m765iosub(dd);
		} else {
			quechk(dd,ubufh,unit);
		}
	}
}
/****************************************************************************
 *
 *	door open timeout
 *
 ****************************************************************************/
openrtry(dd,mbufh,unit)
register struct	fddev	*dd;
register struct	fdmbuf	*mbufh;
int	unit;
{
	register char	seekpoint;

	if((mbufh->b_buf->b_flags&(B_READ|B_VERIFY))!=(B_READ|B_VERIFY)){
		if(inb(VFOREG)&OPENBIT){
			if(mbufh->b_buf->b_flags&B_FORMAT){
				mbufh->b_status = TIMEOUT;
				intrerr0(dd,mbufh,unit);
			} else {
				if((inb(STSREG)&ST0OK)!=ST0OK)
				/*	cmn_err(CE_CONT,"%s %d : %s\n",fderr,mbufh->b_unit,fdmsg[DOORERR]); */
					printf("%s %d : %s\n",fderr,mbufh->b_unit,fdmsg[DOORERR]);
				rstout(mbufh,unit);	
				specify();
				mbufh->b_cmd.c_rbmtr &= RBRST;
				mbufh->b_cmd.c_intr |= SKEFLAG;
				if(mbufh->b_cmd.c_saddr > 2)
					seekpoint = mbufh->b_cmd.c_saddr-2;
				else
					seekpoint = mbufh->b_cmd.c_saddr+2;
				fdiseek(dd,unit,seekpoint);
			}
			return(1);
		}
	}
	return(0);
}

/****************************************************************************
 *
 *	write verify routine
 *
 ****************************************************************************/
wverify(dd,dr,mbufh,unit)
register struct fddev *dd;
struct	fddrtab *dr;
register struct fdmbuf *mbufh;
register unsigned int	 unit;
{
	if(!(mbufh->b_buf->b_flags & B_VERIFY)){
		mbufh->b_buf->b_flags |= B_VERIFY;
		rwcmdset(dr,mbufh,unit,mbufh->b_sector);
		if(mbufh->b_status = outicmd(dd,mbufh,unit))
			intrerr0(dd,mbufh,unit);
		return(ERROR);
	} else 
		mbufh->b_buf->b_flags &= ~B_VERIFY;
	return(OK);
}
/*****************************************************************************
 *
 *	read / write / format / verify error routine
 *
 *****************************************************************************/
rwierr(dd,mbufh,unit,dr)
register struct	fddev		*dd;
register struct	fdmbuf	*mbufh;
unsigned int	unit;
register struct fddrtab *dr;
{
	char	seekpoint;
	short	status;

	if((mbufh->b_buf->b_flags&(B_READ|B_VERIFY))==(B_READ|B_VERIFY)){
		if((mbufh->b_rwerr&SRMASK)<MEDIARD)
			goto rwrtry;
		if((mbufh->b_rwerr&MRMASK)<MEDIASEEK)
			goto rwseek;
		else
			goto rwexit;
	} else {
		if(mbufh->b_buf->b_flags&B_VERIFY){
			mbufh->b_buf->b_flags &= ~B_VERIFY;
			rwcmdset(dr,mbufh,unit,mbufh->b_sector);
		}
	}
rwrtry:	status = mbufh->b_status;
	if((++mbufh->b_rwerr&SRMASK)<SRETRY){
#ifdef	DEBUG
		if((mbufh->b_buf->b_flags&(B_READ|B_VERIFY))
			!= (B_READ|B_VERIFY)){
		/*	cmn_err(CE_CONT,"Warning : "); */
			printf("Warning : ");
			checkerr(dr,mbufh,unit);
		}
#endif
		mbufh->b_status = outicmd(dd,mbufh,unit);
	} else {
rwseek:		mbufh->b_rwerr = (mbufh->b_rwerr&RMRMASK)+MINC;
		if((mbufh->b_rwerr&MRMASK)<MRETRY){
			mbufh->b_cmd.c_intr |= SKEFLAG;
			if(mbufh->b_cmd.c_saddr > 2)
				seekpoint = mbufh->b_cmd.c_saddr-2;
			else
				seekpoint = mbufh->b_cmd.c_saddr+2;
			mbufh->b_status=fdiseek(dd,unit,seekpoint);
		} else {
			mbufh->b_rwerr = (mbufh->b_rwerr&LRMASK)+LINC;
			if((mbufh->b_rwerr&LRMASK)<LRETRY)
       				mbufh->b_status=rbirate(mbufh,unit,dr->dr_type);
		}
	}
	if(mbufh->b_status){
		mbufh->b_status = status;
rwexit:		intrerr0(dd,mbufh,unit);
	}
}
/*****************************************************************************
 *
 *	recalibrate interrupt routine
 *
 *****************************************************************************/
rbintr(dd,mbufh,ubufh,unit)
struct	fddev  *dd;
register struct	fdmbuf *mbufh;
register struct	fdubuf *ubufh;
register unsigned int	 unit;
{
	mbufh->b_cmd.c_intr &= ~RBFLAG;
	if(mbufh->b_status){
		if(++mbufh->b_rberr<SRETRY)
			mbufh->b_status =
				rbirate(mbufh,unit,dd->d_drtab[unit].dr_type);
	} else {
		mbufh->b_cmd.c_rbmtr |= 1<<(RBSHIFT+unit);
		ubufh->b_seekaddr = 0;
		mbufh->b_rberr = 0;
		mbufh->b_status=fdiseek(dd,unit,mbufh->b_cmd.c_saddr);
	}
	if(mbufh->b_status)
		intrerr0(dd,mbufh,unit);
}

/******************************************************************************
 *
 *	seek interrupt routine
 *
 ******************************************************************************/
seekintr(dd,mbufh,ubufh,unit)
register struct	fddev *dd;
register struct	fdmbuf *mbufh;
register struct	fdubuf *ubufh;
unsigned int	 unit;
{
	mbufh->b_cmd.c_intr &= ~SKFLAG;
	if(mbufh->b_status){
			seekierr(dd,mbufh,unit,mbufh->b_cmd.c_saddr);
	} else {
		ubufh->b_seekaddr = mbufh->b_cmd.c_saddr;
		mbufh->b_status = outicmd(dd,mbufh,unit);
	}
	if(mbufh->b_status)
		intrerr0(dd,mbufh,unit);
	else
		mbufh->b_seekerr = 0;
}
/*****************************************************************************
 *
 *	seek error retry interrupt routine
 *
 ******************************************************************************/
seekintre(dd,mbufh,ubufh,unit)
struct	fddev *dd;
register struct	fdmbuf *mbufh;
register struct	fdubuf *ubufh;
unsigned int	 unit;
{
	register char		seekpoint;

	mbufh->b_cmd.c_intr &= ~(SKEFLAG|SKFLAG);
	if(mbufh->b_cmd.c_saddr > 2)
		seekpoint = mbufh->b_cmd.c_saddr-2;
	else
		seekpoint = mbufh->b_cmd.c_saddr+2;
	if(mbufh->b_status)
		seekierr(dd,mbufh,unit,seekpoint);
	else {
		ubufh->b_seekaddr = seekpoint;
		mbufh->b_status=fdiseek(dd,unit,mbufh->b_cmd.c_saddr);
	}
	if(mbufh->b_status)
		intrerr0(dd,mbufh,unit);
	else
		mbufh->b_seekerr = 0;
}

/*****************************************************************************
 *
 *	seek error routine
 *
 ******************************************************************************/
seekierr(dd,mbufh,unit,seekpoint)
struct fddev  *dd;
register struct fdmbuf *mbufh;
register unsigned int	unit;
register char		seekpoint;
{
	if((++mbufh->b_seekerr&SRMASK)<SRETRY){
		mbufh->b_status=fdiseek(dd,unit,seekpoint);
	} else {
		mbufh->b_seekerr = (mbufh->b_seekerr&MRMASK) + MINC;
		if((mbufh->b_seekerr&MRMASK)<MRETRY)
			mbufh->b_status=rbirate(mbufh,unit,dd->d_drtab[unit].dr_type);
	}
	if(mbufh->b_status){
		intrerr0(dd,mbufh,unit);
	}
}
/*****************************************************************************
 *
 * TITLE:	m765sweep
 *
 * ABSTRACT:	Perform an initialization sweep.  
 *
 **************************************************************************/
m765sweep(dd, unit, cdr)
struct fddev *dd;			/* device parameters */
register unsigned int	unit;
register struct fddrtab  *cdr;	/* device initialization data */
{
	register struct fddrtab *dr;

	dr = &dd->d_drtab[unit];
	dr->dr_ncyl	      = cdr->dr_ncyl;
	dr->dr_nrhead	      = cdr->dr_nrhead;
	dr->dr_nsec	      = cdr->dr_nsec;
	dr->dr_secsiz	      = cdr->dr_secsiz;	
	dr->dr_spc	      = cdr->dr_spc;
	dr->dr_part[0].p_fsec = cdr->dr_part[0].p_fsec;
	dr->dr_part[0].p_nsec = cdr->dr_part[0].p_nsec;
	dr->dr_part[1].p_fsec = cdr->dr_part[1].p_fsec;
	dr->dr_part[1].p_nsec = cdr->dr_part[1].p_nsec;
	dr->dr_type	      = cdr->dr_type;
	dr->dr_rwgpl	      = cdr->dr_rwgpl;
	dr->dr_fgpl	      = cdr->dr_fgpl;
}

/******************************************************************************
 *
 *	fdc parameter cmp routine
 *
 ******************************************************************************/
cmpparm(sdr,ddr)
register struct fddrtab  *sdr;	/* device initialization data */
register struct fddrtab  *ddr;	/* device initialization data */
{
	if(sdr->dr_ncyl   != ddr->dr_ncyl)
		return(ERROR);
	if(sdr->dr_nrhead != ddr->dr_nrhead)
		return(ERROR);
	if(sdr->dr_nsec   != ddr->dr_nsec)
		return(ERROR);
	if(sdr->dr_secsiz != ddr->dr_secsiz)	
		return(ERROR);
	if((sdr->dr_type&~OKTYPE) != ddr->dr_type)
		return(ERROR);
	if(sdr->dr_rwgpl  != ddr->dr_rwgpl)
		return(ERROR);
	if(sdr->dr_fgpl   != ddr->dr_fgpl)
		return(ERROR);
	return(OK);
}
/*****************************************************************************
 *
 *  TITLE:  m765disksort
 *
 *****************************************************************************/
fd_disksort(dp, bp, nsec)
struct fdubuf *dp;		/*  Pointer to head of active queue	*/
register struct buf *bp;	/*  Pointer to buffer to be inserted	*/
char	nsec;			/*  Sectors/track */
{
	register struct buf *bp2; /*  Pointer to next buffer in queue	*/
	register struct buf *bp1; /*  Pointer where to insert buffer	*/

	bp1 = (struct buf *)dp;
	bp2 = bp1->av_forw;
	if(bp2){
		bp1 = bp2;
		bp2 = bp1->av_forw;
	}
	while(bp2){
		if(relative(bp1->b_pfcent,bp->b_pfcent,nsec)
				<relative(bp1->b_pfcent,bp2->b_pfcent,nsec))
			break;
		bp1 = bp2;
		bp2 = bp1->av_forw;
	}
	/* bp->b_start = lbolt; */
	bp1->av_forw = bp;
	bp->av_forw = bp2;
}

relative(sector1,sector2,nsec)
register daddr_t sector1;
register daddr_t sector2;
register char	 nsec;
{
	if(sector1>sector2)
		return((int)((sector1/nsec) - (sector2/nsec)));
	else
		return((int)((sector2/nsec) - (sector1/nsec)));
}
/*****************************************************************************
 *
 *	Set Interrupt error and FDC reset
 *
 *****************************************************************************/
intrerr0(dd,mbufh,unit)
struct	 fddev *dd;
register struct	fdmbuf *mbufh;
register unsigned int	 unit;
{
	register struct	fddrtab *dr;
	struct buf *bp;		/*  Pointer to next buffer in queue	*/
	struct	fdubuf *ubufh;

	dr = &dd->d_drtab[unit];
	ubufh = mbufh->b_unitf;
	while(ubufh->b_unit != unit)
		ubufh = ubufh->b_unitf;	
	checkerr(dr,mbufh,unit);
	mbufh->b_rwerr = mbufh->b_seekerr = mbufh->b_rberr = 0;
	mbufh->b_cmd.c_intr = CMDRST;
	if((mbufh->b_buf->b_flags&(B_READ|B_VERIFY))!=(B_READ|B_VERIFY)){
		if(((kbd_FDsts&FDCHG)&&unit)||((!(kbd_FDsts&FDCHG))&&(!unit))){
			fddtype = 0;
			dr->dr_type &= ~OKTYPE; 
		}
	}
	bp = mbufh->b_buf;
	bp->b_flags |= B_ERROR;
	switch(mbufh->b_status&BYTEMASK){
		case ADDRERR:
			bp->b_error = EIO;
			break;
		case OVERRUN:
			bp->b_error = EIO;
			break;
		case FDCERR:
			bp->b_error = EIO;
			break;
		case TIMEOUT:
			bp->b_error = EIO;
			break;				
		case WTPRT:
			bp->b_error = ENODEV;
			break;
		case NOREC:
			bp->b_error = EBBHARD;
			break;				
		case CRCERR:
			bp->b_error = EBBSOFT;
			break;				
	}
	rstout(mbufh,unit);
	specify();
	mbufh->b_cmd.c_rbmtr &= RBRST;
	quechk(dd,ubufh,unit);
}
/*****************************************************************************
 *
 *	Next queue check routine
 *
 *****************************************************************************/
quechk(dd,ubufh,unit)
struct	fddev *dd;
struct	fdubuf *ubufh;
unsigned int	unit;
{
	register struct	fdmbuf *mbufh;
	register struct	buf *bp;
	register int	count;
	int	wunit;
	unsigned int	x;
	
	mbufh = &(dd->d_bufh);

	x = SPL();
	/* clear retry count */
	mbufh->b_rwerr = mbufh->b_seekerr = mbufh->b_rberr = 0;
	bp = ubufh->av_forw;
	bp->b_resid = bp->b_resid + mbufh->b_xfercount;
	ubufh->av_forw = bp->av_forw; 

	biodone(bp);
	for ( wunit = unit , count = MAXUNIT ; count > 0 ; count--){
		ubufh = ubufh->b_unitf;	
		wunit = (wunit+1)%MAXUNIT;
		if(ubufh->av_forw != NULL)
			break;
		else {	
			if(mbufh->b_active[wunit] & IO_WAIT)
				wakeup((caddr_t)&mbufh->b_active[wunit]);
			mbufh->b_active[wunit] = IO_IDLE;
		}
	}
	if(count){
		m765io(dd,ubufh,wunit);
	} else {
		dmadone();
		mbufh->b_cmd.c_stsflag &= ~MTRFLAG;
		mtr_on(mbufh,unit);
		openstr(mbufh);
	}
	splx(x);
}
/*****************************************************************************
 *
 *	interrupt error output routine
 *
 *****************************************************************************/
checkerr(dr,mbufh,unit)
register struct	fddrtab *dr;
register struct fdmbuf *mbufh;
register unsigned int	unit;
{
	int	resid;

	if((mbufh->b_buf->b_flags&(B_READ|B_VERIFY))!=(B_READ|B_VERIFY)){
		resid = mbufh->b_xfercount = mbufh->b_xferdma - 1 - residcnt();
		resid = (mbufh->b_sector + ( resid / dr->dr_secsiz ))
			% dr->dr_spc;
	/*	cmn_err(CE_CONT,"%s %d : %s\n",
			fderr,unit,fdmsg[mbufh->b_status&BYTEMASK]); */
		printf("%s %d : %s\n",
			fderr,unit,fdmsg[mbufh->b_status&BYTEMASK]);
	/*	cmn_err(CE_CONT,"cylinder = %d  ",mbufh->b_cmd.c_saddr); */
		printf("cylinder = %d  ",mbufh->b_cmd.c_saddr);
	/*	cmn_err(CE_CONT,"head = %d  sector = %d  byte/sec = %d\n",
		resid / dr->dr_nsec , (resid % dr->dr_nsec)+1 , dr->dr_secsiz); */
		printf("head = %d  sector = %d  byte/sec = %d\n",
		resid / dr->dr_nsec , (resid % dr->dr_nsec)+1 , dr->dr_secsiz);
	}
}
/*****************************************************************************
 *
 *	get dma cntroler
 *
 *****************************************************************************/
dmaget()
{
	register unsigned int	x;

	x = splhi();
	while(dmause==DMABUSY)
		sleep(&dmause,PZERO);
	dmause = DMABUSY;
	splx(x);
}

/*****************************************************************************
 *
 *	release dma cntroler
 *
 *****************************************************************************/
dmadone()
{
	register unsigned int	x;

	x = splhi();
	dmause = DMADONE;
	wakeup(&dmause);
	splx(x);
}

fdprint(dev,str)
dev_t	dev;
char	*str;
{
	printf("floppy disk driver: %s on bad dev %d, partition %d\n",
			str, UNIT(dev), PARTITION(dev)	);
}

fdsize()
{
	printf("fdsize()	-- not implemented\n");
}

fddump()
{
	printf("fddump()	-- not implemented\n");
}

