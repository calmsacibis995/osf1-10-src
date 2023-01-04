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
static char	*sccsid = "@(#)$RCSfile: m765drv.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:42 $";
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


/******************************************************************************
 *
 * 		I N C L U D E  and  E X T E R N
 *
 ******************************************************************************/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <i386/pmap.h>
#include <i386/AT386/m765.h>

extern	struct	fddev	m765dev;
extern	struct	fdcmn m765fdata;
extern  struct  fdtree fd0tree[];
extern  struct  fdtree fd1tree[];
extern	struct	fdtree fd2tree[];
extern	struct	fddrtab m765f0[];
extern	struct	fddrtab m765f1[];
extern	unsigned char	kbd_FDsts;
extern	char	fddtype;
extern	char	fdtype[];
/*****************************************************************************
 *
 *	external fd check routine
 *
 *****************************************************************************/
extchk(unit)
register unsigned int unit;
{
	if(kbd_FDsts&INFDD){			/* 2 internal FDD ? */
		if(!(kbd_FDsts&FDEXT)){		/* external conect ? */
			if(kbd_FDsts&FDCHG){
				if(unit)
					return(ERROR);
			} else {
				if(!(unit))
					return(ERROR);
			}
		} 
	}
	return(OK);
}

/*****************************************************************************
 *
 *	drive check ( 0 : 2HD  1 : 2HC ) routine
 *
 *****************************************************************************/
dunitchk(unit)
register unsigned int	unit;
{
	if(kbd_FDsts & INFDD){
		if( kbd_FDsts & FDCHG ){
			if( unit ||((!(unit))&&(kbd_FDsts&HDDRV)))
				return(ERROR);
		} else {
			if(!(unit&&(!(kbd_FDsts&HDDRV))))
				return(ERROR);
		}
	} else {
		if(kbd_FDsts&HDDRV)
			return(ERROR);
	}
	return(OK);
}
/*****************************************************************************
 *
 *	2D_external drive check ( ERROR : 2D  OK : not 2D ) routine
 *
 *****************************************************************************/
ext2dchk(unit)
register unsigned int	unit;
{
	int	chgsts;

	chgsts = kbd_FDsts & FDCHG;
	if((chgsts&&(!unit))||((!chgsts)&&unit))
		goto not2d;
	if(fddtype == EXT2D)
		return(ERROR);
not2d:	return(OK);
}

/*****************************************************************************
 *
 *	drive checge check routine
 *
 *****************************************************************************/
struct fdtree *dchgchk(unit)
register unsigned int	unit;
{
	if(dunitchk(unit)){
		if(ext2dchk(unit))
			return(&fd2tree[0]);
		return(&fd1tree[0]);
	} else
		return(&fd0tree[0]);
}

/*******************************************************************************
 *
 * 	new drive checge check routine
 *
 ******************************************************************************/
struct fdtree *dchgchk_new(dev)
register unsigned int 	dev;
{
	register unsigned int mtype;

        mtype = MEDIATYPE(dev);
	if ((mtype == 1 && DEVTYPE(dev) == 4) || DEVTYPE(dev) == 4)
		return(&fd0tree[0]);  /* if the media is F2h18 */
	mtype = (mtype == 0) ? 2 : mtype - 1;
	return(&fd1tree[mtype]);
}

/*****************************************************************************
 *
 *	get fdc parameter table routine
 *
 *****************************************************************************/
struct fddrtab *getparm(type,unit)
register int	type;
register unsigned int	unit;
{
	register struct fddrtab *rtn;

	type--;
	if(ext2dchk(unit))
		type += 4;
	rtn = (struct fddrtab *)ERROR;
	if(type < FORMMAX){
		if(dunitchk(unit))
			rtn = &m765f1[type];
		else
			rtn = &m765f0[type];
		if(rtn->dr_ncyl == 0)
			rtn = (struct fddrtab *)ERROR;
	}
	return(rtn);
}
/*****************************************************************************
 *
 *	new get fdc parameter table routine
 *
 *****************************************************************************/
struct fddrtab *getparm_new(type,unit)
register int	type;
register unsigned int	unit;
{
	register struct fddrtab *rtn;

	type--;
	rtn = (struct fddrtab *)ERROR;
	if(type < FORMMAX){
		if (type) 
			rtn = &m765f1[type];
		else
			rtn = &m765f0[type];
		if(rtn->dr_ncyl == 0)
			rtn = (struct fddrtab *)ERROR;
	}
	return(rtn);
}

/*****************************************************************************
 *
 *	fdc reset routine
 *
 *****************************************************************************/
rstout(mbufh,unit)
register struct	fdmbuf *mbufh;
register unsigned int unit;
{
	register int	outd;
	int	mtrnum;	

	mtrnum = (mbufh->b_cmd.c_rbmtr&MTRMASK);	
	outd = (mtrnum<<MTR_ON)|unit;
	outb( CTRLREG , outd );
	waitx(MSEC);
	outd |= FDC_RST;
	outb( CTRLREG , outd );
	waitx(MSEC);
	outd |= DMAREQ;
	outb( CTRLREG , outd );
	waitx(MSEC);
}

/*****************************************************************************
 *
 *	specify command routine
 *
 ******************************************************************************/
specify()
{
	if(fdc_sts(FD_OSTS))		/* status check */
		goto sperr;
	outb( DATAREG , SPCCMD );		/* Specify command */
	if(fdc_sts(FD_OSTS))		/* status check */
		goto sperr;
	outb( DATAREG , m765fdata.f_srthut ); 	/* Step rate,Head unload time */
	if(fdc_sts(FD_OSTS))		/* status check */
		goto sperr;
	outb( DATAREG , m765fdata.f_hltnd );  	/* Head load time,Non DMA Mode*/
sperr:	return;
}
/*****************************************************************************
 *
 *	transfer rate set routine
 *
 *****************************************************************************/
trfrate( type )		/* set transfer rate */
register unsigned char	type;
{
	outb( VFOREG , ((type & RATEMASK)>>6) );
}

/*****************************************************************************
 *
 *	recalibrate and seek transfer rate set routine
 *
 *****************************************************************************/
rbskrate( type )		/* set transfer rate */
register char	type;
{
	if(type & RAPID)
		trfrate(RPSEEK);		/* set transfer rate */
	else
		trfrate(NMSEEK);		/* set transfer rate */
}
/****************************************************************************
 *
 *	recalibrate command routine
 *
 ****************************************************************************/
rbrate(mbufh,mtype,unit)
register struct	fdmbuf *mbufh;
char	 mtype;
unsigned unit;
{
	register int	rtn = 1;
	register int	rty_flg = 2;
	unsigned int	x;

	rbskrate(mtype);			/* set transfer rate */
	while((rty_flg--)&&rtn){
		if(rtn = fdc_sts(FD_OSTS))	/* status check */
			break;
		outb( DATAREG , RBCMD );	/* recalibrate command*/
		if(rtn = fdc_sts(FD_OSTS))	/* status check */
			break;
		mbufh->b_cmd.c_intr |= WUPFLAG;

		x = SPL();
		outb( DATAREG , unit );
		rtn = ERROR;
		while(rtn) {
			sleep(mbufh,PZERO);
			if((rtn = sis()) == ST0OK)
				mbufh->b_cmd.c_intr |= WUPFLAG;
			else
				break;
		}
		splx(x);
		waitx(m765fdata.f_hst*MSEC);
	}
	return(rtn);
}
/*****************************************************************************
 *
 *	seek command routine
 *
 ****************************************************************************/
fdseek( mbufh , mtype , unit , cylno )
register struct	fdmbuf *mbufh;
register char	mtype;
unsigned unit;
register int	cylno;
{
	unsigned int	x;
	int	rtn;

	rbskrate(mtype);
	if(rtn = fdc_sts(FD_OSTS))	/* status check */
		return(rtn);
	outb( DATAREG , SEEKCMD );	/* seek command */
	if(rtn = fdc_sts(FD_OSTS))	/* status check */
		return(rtn);
	outb( DATAREG , unit );		/* drive number */
	if(rtn = fdc_sts(FD_OSTS))	/* status check */
		return(rtn);
	x = SPL();
	mbufh->b_cmd.c_intr |= WUPFLAG;
	outb( DATAREG , cylno );	/* seek count */
	rtn = ERROR;
	while(rtn){	
		sleep(mbufh,PZERO);
		if((rtn = sis()) == ST0OK)
			mbufh->b_cmd.c_intr |= WUPFLAG;
		else
			break;
	}
	splx(x);
	waitx(m765fdata.f_hst*MSEC);
	return(rtn);
}
/****************************************************************************
 *
 *	dma parameter set routine
 *
 ****************************************************************************/
dmaset(mbufh)
register struct fdmbuf *mbufh;
{
	register long	address;
	register int	data;
	register int	dmalen;

	outb(DMACMD1,DMADATA0);	/* DMA #1 command register 	*/
	outb(DMAMSK1,DMADATA1);	/* DMA #1 all mask register	*/
	outb(DMACMD2,DMADATA2);
	outb(DMASMSK,DMADATA3);
	outb(DMAMOD2,DMADATA4);
	outb(DMAMSK2,DMADATA5);
	switch(mbufh->b_cmd.c_rwdata[0]){
		case RDM:
			data = DMARD;
			break;
		case WTM:
		case FMTM:
			data = DMAWT;
			break;
		case RDMV:
			data = DMAVRF;
			break;
	}
	outb(DMABPFF,data);
	outb(DMAMODE,data);

	/* get work buffer physical address */
	address = kvtophys(mbufh->b_xferaddr);
	dmalen = i386_trunc_page(address) + I386_PGBYTES - address;
	dmalen = dmalen <= mbufh->b_xferdma ? dmalen : mbufh->b_xferdma;
	mbufh->b_xferdma = dmalen;

	/* set buffer address */
	outb(DMAADDR,(int)address&BYTEMASK);		
	outb(DMAADDR,(((int)address>>8)&BYTEMASK));
	outb(DMAPAGE,(((int)address>>16)&BYTEMASK));

	/* set transfer count */
	dmalen--;
	outb(DMACNT,dmalen&BYTEMASK);	
	outb(DMACNT,((dmalen>>8)&BYTEMASK));
	outb(DMAMSK,CHANEL2);
}
/*****************************************************************************
 *
 *	seek commnd routine(use interrupt)
 *
 *****************************************************************************/
fdiseek( dd , unit , cylno )
struct	fddev *dd;
unsigned unit;
int	cylno;
{
	register struct	fdmbuf *mbufh;
	register struct fdubuf *ubufh;
	register int	rtn;
	int	seektype;

	mbufh = &(dd->d_bufh);
	ubufh = mbufh->b_unitf;
	if(unit)
		ubufh = ubufh->b_unitf;
	rbskrate( dd->d_drtab[unit].dr_type );	/* set transfer rate */
	if(rtn = fdc_sts(FD_OSTS))		/* status check */
		goto fdiend;
	outb( DATAREG , SEEKCMD );		/* seek command */
	if(rtn = fdc_sts(FD_OSTS))		/* status check */
		goto fdiend;
	outb( DATAREG , unit );			/* drive number */
	if(rtn = fdc_sts(FD_OSTS))		/* status check */
		goto fdiend;
	ubufh->b_seekaddr = cylno;
	if(dd->d_drtab[unit].dr_type&DOUBLE)
		cylno = cylno * 2;
	mbufh->b_cmd.c_intr |= SKFLAG;
	outb( DATAREG , cylno );		/* seek count */
fdiend:	
	if(rtn)
		rtn |= SEEKCMD<<8;
	return(rtn);
}
/*****************************************************************************
 *
 *	recalibrate command routine(use interrupt)
 *
 *****************************************************************************/
rbirate(mbufh,unit,rbtype)
register struct	fdmbuf *mbufh;
unsigned unit;
register int	rbtype;
{
	register int	rtn;

	rbskrate(rbtype);		/* set transfer rate */
	if(rtn = fdc_sts(FD_OSTS))	/* status check */
		goto rbiend;
	outb( DATAREG , RBCMD );	/* recalibrate command */
	if(rtn = fdc_sts(FD_OSTS))	/* status check */
		goto rbiend;
	mbufh->b_cmd.c_intr |= RBFLAG;
	outb( DATAREG , unit );
rbiend:	
	if(rtn)
		rtn |= RBCMD<<8;
	return(rtn);
}
/*****************************************************************************
 *
 *	read / write / format / verify command out routine(use interrupt)
 *
 *****************************************************************************/
outicmd(dd,mbufh,unit)
struct	fddev *dd;
struct	fdmbuf *mbufh;
unsigned int unit;
{
	extern  int(fdintr)();
	int	rtn;
	unsigned int	x;
	register struct	fdcmd *cmd;
	register int	*data;
	register int	cnt0;

	dmaset(mbufh);
	trfrate(dd->d_drtab[unit].dr_type);	/* set transfer rate */
	cmd = &mbufh->b_cmd;
	data = &cmd->c_rwdata[0];
	x = SPL();
	for( cnt0 = 0 ; cnt0 < cmd->c_dcount ; cnt0++ , data++){
		if(rtn = fdc_sts(FD_OSTS))	/*statu check*/
			break;
		outb( DATAREG , *data );
	}
	if(!(rtn)){
		cmd->c_intr |= RWFLAG;
		cmd->c_stsflag |= INTROUT;
		if((mbufh->b_buf->b_flags&(B_READ|B_VERIFY))==(B_READ|B_VERIFY))
			cnt0 = TOUT;
		else
			cnt0 = ITOUT;
		cmd->c_timeid = timeout(fdintr,RWLEVEL,cnt0);
	}
	splx(x);
	return(rtn);
}

/*****************************************************************************
 *
 *	sense device status routine
 *
 *****************************************************************************/
sds(unit)
unsigned int unit;
{
	register int	rtn;

	if(rtn = fdc_sts(FD_OSTS))	/* status check */
		return(rtn);
	outb( DATAREG , SDSCMD );
	if(rtn = fdc_sts(FD_OSTS))	/* status check */
		return(rtn);
	outb( DATAREG , unit );
	if(rtn = fdc_sts(FD_ISTS))	/* status check */
		return(rtn);
	return((int)inb( DATAREG ));	/* get st3 */
}

/*****************************************************************************
 *
 *	sense interrupt status routine
 *
 *****************************************************************************/
sis()
{
	register int	rtn;
	register int	st0;

	if(rtn = fdc_sts(FD_OSTS))	/* status check */
		return(rtn);
	outb( DATAREG , SISCMD );
	if(rtn = fdc_sts(FD_ISTS))	/* status check */
		return(rtn);
	st0 = inb( DATAREG ) & ST0OK;	/* get st0 */
	if(rtn = fdc_sts(FD_ISTS))	/* status check */
		return(rtn);
	inb( DATAREG );			/* get pcn */
	switch(st0){
		case ST0AT:
		case ST0IC:
			st0 = FDCERR;
	}
	return(st0);
}

/*****************************************************************************
 *
 *	fdc status get routine
 *
 *****************************************************************************/
fdc_sts(mode)
register int	mode;
{
	register int	ind,cnt0;

	cnt0 = STSCHKCNT;
	while(cnt0--){
		ind = inb(STSREG);
		if( ind & DATAOK )
			if((ind & DTOCPU) == mode)
				return(0);
	}
	return(TIMEOUT);
}

/*****************************************************************************
 *
 *	read / write / format / verify status get routine
 *
 *****************************************************************************/
rwstschk()
{
	int	rsult[7];
	register int	count;
	register int	rtn;

	for( count = 0 ; count < 7 ; count++ ){
		if(rtn = fdc_sts(FD_ISTS))	/* status check */
			goto rwend;
		rsult[count] = inb( DATAREG );
	}
	rtn = 0;
	if(rsult[0]&0xc0){
		rtn = m765dev.d_bufh.b_cmd.c_rwdata[0]<<8;
		if(rsult[0]&0x80){ rtn |= FDCERR;   goto rwend; }
		if(rsult[1]&0x80){ rtn |= NOREC;    goto rwend; }
		if(rsult[1]&0x20){ rtn |= CRCERR;   goto rwend; }
		if(rsult[1]&0x10){ rtn |= OVERRUN;  goto rwend; }
		if(rsult[1]&0x04){ rtn |= NOREC;    goto rwend; }
		if(rsult[1]&0x02){ rtn |= WTPRT;    goto rwend; }
		if(rsult[1]&0x01){ rtn |= ADDRERR;  goto rwend; }
		rtn |= FDCERR;
rwend:		outb( 0x0a , 0x06 );
	}
	return(rtn);
}
/*****************************************************************************
 *
 *	resid count get routine
 *
 ******************************************************************************/
unsigned int residcnt()
{
	register unsigned int	count;

	count = inb(DMACNT);
	count += (inb(DMACNT)<<8);
	return(count);
}

/*****************************************************************************
 *
 *	motor on routine
 *
 *****************************************************************************/
mtr_on(mbufh,unit)
register struct fdmbuf *mbufh;
register unsigned unit;
{
	register int	status;
	
	status = mtr_start(mbufh,unit);
	mtr_wait(mbufh,unit,status);
}
/*****************************************************************************
 *
 *	motor start routine
 *
 *****************************************************************************/
mtr_start(mbufh,unit)
register struct fdmbuf *mbufh;
register unsigned unit;
{
	int	status;
	register int	outd;
	unsigned int	x;
	int	(mtr_off)();

	x = splhi();
	if(mbufh->b_cmd.c_stsflag & MTROFF){
		untimeout(mtr_off, unit);
#if	0
	/*	untimeout(mbufh->b_cmd.c_mtrid); */
#endif
		mbufh->b_cmd.c_stsflag &= ~MTROFF;
	}
	status = mbufh->b_cmd.c_rbmtr&(1<<unit);
	mbufh->b_cmd.c_rbmtr |= (1<<unit);
	outd = (mbufh->b_cmd.c_rbmtr&MTRMASK)<<MTR_ON;
	outd |= FDC_RST|unit|DMAREQ;
	outb( CTRLREG , outd );
	splx(x);
	return(status);
}

/*****************************************************************************
 *
 *	motor on wait routine
 *
 *****************************************************************************/
mtr_wait(mbufh,unit,status)
register struct fdmbuf *mbufh;
register unsigned unit;
int	status;
{
	extern  int(mtr_off)();
	extern	int(mtr_wake)();
	register unsigned int	x;

	x = splhi();
	if(!(status)){
		timeout(mtr_wake,&mbufh->b_cmd.c_stsflag,HZ);
		sleep(&mbufh->b_cmd.c_stsflag,PZERO);
	}
	mbufh->b_cmd.c_stsflag |= MTROFF;
	mbufh->b_cmd.c_mtrid = timeout(mtr_off,unit,MTRSTOP);
	splx(x);
}

/*****************************************************************************
 *
 *	motor wakeup routine
 *
 *****************************************************************************/
mtr_wake(c_stsflag)
register int	*c_stsflag;
{
	wakeup(c_stsflag);
}

/*****************************************************************************
 *
 *	motor off routine
 *
 *****************************************************************************/
mtr_off(unit)
register unsigned unit;
{
	register struct	fdmbuf *mbufh;
	register int	outd;
	unsigned int	x;

	x = splhi();
	mbufh = &(m765dev.d_bufh);
	mbufh->b_cmd.c_stsflag &= ~MTROFF;
	if(!(mbufh->b_cmd.c_stsflag&MTRFLAG)){
		mbufh->b_cmd.c_rbmtr &= MTRRST;
		outd = FDC_RST | DMAREQ;
		outb( CTRLREG , outd );
	} 
	splx(x);
}

/*****************************************************************************
 *
 *	wait loop 
 *
 *****************************************************************************/
waitx(count0)
register unsigned int	count0;
{
	if(count0){
		while(count0--){
			;
		}
	}
}
