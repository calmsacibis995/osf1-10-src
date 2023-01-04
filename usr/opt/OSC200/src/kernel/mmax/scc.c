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
static char	*sccsid = "@(#)$RCSfile: scc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:49 $";
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




/*1
 * scc.c
 *
 *	This file contains the routines used for sending messages
 *	to the SCC from the DPC.
/*/


#include <sys/param.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <mmaxio/crqdefs.h>
#include <mmaxio/emcdefs.h>
#include <mmaxio/msdefs.h>
#include <mmax/sccdefs.h>
#include <mmax/scc_attns.h>
#include <mmax/toy.h>
#include <mmax/isr_env.h>
#include <kern/queue.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/syslog.h>
#include <sys/table.h>


/*
 * Declaration so MULTIMAX compilation will work.
 */

extern	crq_t *Slot_crqs[NUM_SLOT];

int scc_isr();
static	crq_toy_msg_t toy_cmd;
static	mpqueue_head_t	toy_q;
lock_data_t	toy_lock;
struct	toy	toy;


/*2
.* init_scc - initilize the SCC for communication with the host.
 *
.* ARGUMENTS: None.
 *
.* USAGE:
 *	Initialize the SCC for communications with the host. This includes
 *	completing the SCC Requester CRQ initialization (adding the master
 *	vector) and creating the SCC Slot CRQ. Called once early in the
 *	initialization. Must be called BEFORE any other messages are
 *	sent anywhere.
 */

init_scc()
{
	static	crq_t slot_crq;
	static	crq_maxsize_msg_t	scc_msg[NUM_SCC_ATTNS];
	register crq_t *req_crq;
	register crq_msg_t *free;
	register i;

	/*
	 * Calculate the address of the SCC requestor CRQ andallocate
	 * an interrupt vector for it.
	 */
	req_crq = REQ_CRQ(SCC_SLOT, 0);
	crq_lock_init(&req_crq->crq_slock);
	if(alloc_vector(&req_crq->crq_master_vect, scc_isr, REQ_LUN, INTR_DEVICE))
		panic("init_scc: alloc vector failed\n");

	init_crq(&slot_crq, CRQMODE_INTR, 0,
		MAKEUNITID(0, SCC_SLOT, 0, SLOT_LUN), CRQ_STATS_NULL);

	if(alloc_vector(&slot_crq.crq_master_vect, scc_isr, SLOT_LUN, INTR_DEVICE))
		panic("init_scc: slot alloc vector failed\n");

	/*
	 * Populate the free message queue for attentions.
	 */
	for (i = 0; i < NUM_SCC_ATTNS; i++)
		put_free(&scc_msg[i], &slot_crq);

	/*
	 * Create a channel to the SCC slot with the newly created
	 * CRQ. If successful, the slot crq is copied to the
	 * system array of per slot CRQs.
	 */
	if(polled_create_chan(&slot_crq, scc_isr, SLOT_LUN) != 0)
		panic("init_scc: create channel failed\n");
	Slot_crqs[SCC_SLOT] = &slot_crq;

	mpqueue_init(&toy_q);
	lock_init(&toy_lock, TRUE);

}


/*2
.* set_timeofyear - set time of year clock in the SCC of a MULTIMAX
 *
.* ARGUMENTS:
 *
.* set_time - Time in internal UNIX form (i.e. seconds since 1/1/70).
 *
.* USAGE:
 *	This routine is used to set the time-of-year for a MULTIMAX
 *	system.
 */

set_timeofyear(set_time)
time_t set_time;
{
	int error = 0;

	lock_write(&toy_lock);	/* lock the routine */

	toy.toy_tenths_sec = 0;
	
	/*
	 * Convert the UNIX internal time to something more "human" and fill
	 * in the toy structure.
	 */
	{
	int year, month, day, hour, min, sec, day_of_week;
	cnvt_time_to_parts(set_time,&year,&month,&day, &hour,&min,&sec,
			&day_of_week);
	toy.toy_unit_sec = sec % 10;
	toy.toy_ten_sec = sec / 10;
	toy.toy_unit_min = min % 10;
	toy.toy_ten_min = min / 10;
	toy.toy_unit_hour = hour % 10;
	toy.toy_ten_hour = hour / 10;
	toy.toy_unit_day = day % 10;
	toy.toy_ten_day = day / 10;
	toy.toy_unit_month = month % 10;
	toy.toy_ten_month = month / 10;
	toy.toy_unit_year = year % 10;
	year /= 10;
	toy.toy_ten_year = year % 10;
	year /= 10;
	toy.toy_hun_year = year % 10;
	toy.toy_thou_year = year / 10;
	toy.toy_day_of_week = day_of_week;
	}

	/*
	 * Initialize the toy command message.
	 */
	toy_cmd.toy_hdr.crq_msg_code = CRQOP_SCC_SET_TOY;
	toy_cmd.toy_hdr.crq_msg_unitid = MAKEUNITID(0, SCC_SLOT, 0, SLOT_LUN);
	toy_cmd.toy_hdr.crq_msg_refnum = (long) &toy_q;
	toy_cmd.toy_addr = &toy;

	/*
	 * Send command to SCC to set the time of year
	 */
	send_slot_cmd(&toy_cmd, SCC_SLOT);

	/*
	 * Synchronize with completion.
	 */
	if ((crq_toy_msg_t *)
		get_isr_queue(toy_cmd.toy_hdr.crq_msg_refnum) == &toy_cmd) {
		
		/*
		 * Verify successful set time-of-year.
		 */
		if (toy_cmd.toy_hdr.crq_msg_status != STS_SUCCESS)
			/* u.u_error = EIO */;
	} else
		error = EIO;

	lock_done(&toy_lock);	/* lock the routine */
	return(error);
}


/*2
.* get_timeofyear - get time of year using the clock in the SCC of a MULTIMAX
 *
.* ARGUMENTS:
 *
.* get_time - Pointer to time in UNIX internal form (seconds past 1/1/70),
 *	that is to be filled in by this routine.
 *
.* get_ticks - Pointer to number of ticks representing a fraction of a
 *	second past the get_time parameter that are to be filled in by
 *	this routine.
 *
.* USAGE:
 *	This routine is used to get the time-of-year for a MULTIMAX
 *	system.
 */

get_timeofyear(get_time)
time_t *get_time;
{
	int error = 0;

	lock_write(&toy_lock);	/* lock the routine */

	/*
	 * Initialize the toy command message.
	 */
	toy_cmd.toy_hdr.crq_msg_code = CRQOP_SCC_GET_TOY;
	toy_cmd.toy_hdr.crq_msg_unitid = MAKEUNITID(0, SCC_SLOT, 0, SLOT_LUN);
	toy_cmd.toy_hdr.crq_msg_refnum = (long) &toy_q;
	toy_cmd.toy_addr = &toy;

	/*
	 * Send command to SCC to get the time of year
	 */
	send_slot_cmd(&toy_cmd, SCC_SLOT);

	/*
	 * Synchronize with completion.
	 */
	if ((crq_toy_msg_t *)
		get_isr_queue(toy_cmd.toy_hdr.crq_msg_refnum) == &toy_cmd) {
		
		/*
		 * Verify successful get time-of-year.
		 */
		if (toy_cmd.toy_hdr.crq_msg_status != STS_SUCCESS)
			error = EIO;
	} else
		error = EIO;

	/*
	 * Convert the toy structure into a UNIX time_t
	 */
	cnvt_parts_to_time (get_time,
		(toy.toy_unit_year&0xf) + ((toy.toy_ten_year&0xf)*10) +
			1900,
			/* ((toy.toy_hun_year&0xf)*100) + 
			   ((toy.toy_thou_year&0xf)*1000), NOT IN BBRAM YET */
		(toy.toy_unit_month&0xf) + ((toy.toy_ten_month&0xf)*10),
		(toy.toy_unit_day&0xf) + ((toy.toy_ten_day&0xf)*10),
		(toy.toy_unit_hour&0xf) + ((toy.toy_ten_hour&0xf)*10),
		(toy.toy_unit_min&0xf) + ((toy.toy_ten_min&0xf)*10),
		(toy.toy_unit_sec&0xf) + ((toy.toy_ten_sec&0xf)*10));

	lock_done(&toy_lock);	/* lock the routine */
	return(error);
}


/*
 * The following sections deal with converting the UNIX internal time
 * to its constituent parts and vice versa.
 */

/* Seconds in a day */

#define SEC_PER_DAY (24*60*60)

/* Days in each month */

static short month_days[12] =
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* Days in a year */

#define days_in_year(Y) ((Y) % 4 ? 365 : 366)

/* Days in a month accounting for leap years */

#define days_in_month(Y,M) (((Y)%4==0 && (M)==1) ? 29 : month_days[M])

/* Starting year for UNIX internal time (1970) */

#define YRREF 1970


/*2
.* cnvt_time_to_parts - convert UNIX internal time into its parts.
 *
.* ARGUMENTS:
 *
.* the_time - time in UNIX internal form (seconds after 1/1/70).
 *
.* year - pointer to year portion of the time (output parameter).
 *
.* month - pointer to month portion of the time (output parameter).
 *
.* day - pointer to day of month portion of the time (output parameter).
 *
.* hour - pointer to hour of day portion of the time (output parameter).
 *
.* min - pointer to minute of hour portion of the time (output parameter).
 *
.* sec - pointer to seconds of minute portion of the time (output parameter).
 *
.* day_of_week - pointer to day number within the week portion of the 
 *	time (output parameter).
 *
.* USAGE:
 *	This routine is used when changing the time in the hardware clocks
 *	of various systems to convert the UNIX internal time to something
 *	more appropriate for clocks.
 */

cnvt_time_to_parts(the_time, year, month, day, hour, min, sec, day_of_week)
time_t the_time;
int *year, *month, *day, *hour, *min, *sec, *day_of_week;
{
	register int days, hour_min_sec, tmp_year, tmp_month;

	/*
	 * Divide the date into days and seconds.
	 */

	hour_min_sec = the_time % SEC_PER_DAY;
	days = the_time / SEC_PER_DAY;
	if (hour_min_sec < 0) {
		days--;
		hour_min_sec += SEC_PER_DAY;
	}

	/* 
	 * Generate the HH:MM:SS from the seconds calculated above.
	 */

	*sec = hour_min_sec % 60;
	hour_min_sec /= 60;
	*min = hour_min_sec % 60;
	*hour = hour_min_sec / 60;

	/*
	 * Generate the day within the week. Note 1/1/70 was a
	 * Thursday. (Note: 7340036 is 4 mod 7. We use such a large number
	 * in the calculation to assure that we always take the mod of
	 * a positive number.)
	 */

	*day_of_week = (days + 7340036) % 7;

	/*
	 * Calculate the year.
	 */

	if (days >= 0)
		for (tmp_year=YRREF; days>=days_in_year(tmp_year); tmp_year++)
			days -= days_in_year(tmp_year);
	else
		for (tmp_year=YRREF; days<0; tmp_year--)
			days += days_in_year(tmp_year-1);
	*year = tmp_year;

	/*
	 * Calculate the month in the year.
	 */

	for (tmp_month=0; days>=days_in_month(tmp_year,tmp_month); tmp_month++)
		days -= days_in_month(tmp_year,tmp_month);
	*month = tmp_month + 1;

	/*
	 * What we have left over is the day of the month.
	 */

	*day = days + 1;
}


/*2
.* cnvt_parts_to_time - convert the constituent parts of a time to
 *	a UNIX internal time.
 *
.* ARGUMENTS:
 *
.* the_time - pointer to a time_t structure that is to be filled in
 *	with the UNIX internal time (output parameter).
 *
.* year - year
 *
.* month - month of year
 *
.* day - day of month
 *
.* hour - hour of the day (24 hour clock)
 *
.* min - minute of the hour
 *
.* sec - seconds of the minute
 *
.* USAGE:
 *	This routine is used after reading the hardware real-time-clock
 *	to convert the time thus obtained into a form suitable for
 *	UNIX internal use.
 */

cnvt_parts_to_time(the_time,year,month,day,hour,min,sec)
time_t *the_time;
register int year, month;
int hour, min, sec;

/**/
{
	register time_t tmp_time;
	tmp_time = 0;

	/*
	 * Calculate the number of days accounted for by the years.
	 */

	if (year < YRREF) {
		register tmp_year;
		for (tmp_year=year; tmp_year<YRREF; tmp_year++)
			tmp_time -= days_in_year(tmp_year);
	} else {
		register tmp_year;
		for (tmp_year=YRREF; tmp_year<year; tmp_year++)
			tmp_time += days_in_year(tmp_year);
	}

	/*
	 * Calculate the number of days accounted for by the months.
	 */

	month--;
	while (--month >= 0)
		tmp_time += days_in_month(year,month);

	/*
	 * We have days accounted for by the years and the months of the
	 * date. Convert to seconds.
	 */

	*the_time = ((tmp_time+(day-1))*SEC_PER_DAY) + 
			(hour*60*60) + (min*60) + sec;
}



#ifdef	GET_SYS_STATE

/*2
.* get_sys_state - Get array that describes the configuration of the slots
 *
.* ARGUMENTS:
 *
.* state - Pointer to structure that can receive the configuration.
 *
.* RETURN VALUE:
 *	This procedure returns with u.u_error set to non-zero
 *	if getting the state failed.
 *
.* USAGE:
 *	This routine is used to get the configuration of the slots in a
 *	MULTIMAX system. It is called once very early in initialization.
 *
.* ASSUMPTIONS:
 *	The actual state structure pointed to by the
 *	parameter is mapped physical-to-virtual.
 */

get_sys_state(sys)
sys_state_t *sys;
{

	register crq_sys_state_msg_t *cmd;
	register isr_queue_t *isr_q;
	int error = 0;
#define get_state_alloc_size (sizeof(crq_sys_state_msg_t)+sizeof(isr_queue_t))

	/*
	 * Allocate a command packet and isr queue.
	 */
	if ((cmd = (crq_sys_state_msg_t *)
		MALLOC(get_state_alloc_size)) == 0) {
		error = EIO;
		return;
	}
	isr_q = (isr_queue_t *) ((caddr_t) cmd + sizeof(crq_sys_state_msg_t));
	initisrqueue(isr_q);

	/*
	 * Initialize the get state command message.
	 */
	cmd->sys_state_hdr.crq_msg_code = CRQOP_SCC_SYS_STATE;
	cmd->sys_state_hdr.crq_msg_unitid = MAKEUNITID(0,SCC_SLOT,0,SLOT_LUN);
	cmd->sys_state_hdr.crq_msg_refnum = (long) isr_q;
	cmd->sys_state_addr = sys;

	/*
	 * Send command to SCC to get the state
	 */
	send_slot_cmd(cmd, SCC_SLOT);

	/*
	 * Synchronize with completion.
	 */
	if ((crq_sys_state_msg_t *)
		get_isr_queue(cmd->sys_state_hdr.crq_msg_refnum) == cmd) {
		
		/*
		 * Verify successful get state
		 */
		if (cmd->sys_state_hdr.crq_msg_status != STS_SUCCESS)
			error = EIO;
	} else
		error = EIO;

	/*
	 * Deallocate command packet and isr queue.
	 */
	MFREE(get_state_alloc_size, cmd);
	return(error);
}

#endif	GET_SYS_STATE

/*2
.* get_bbram_avail - Get the number of bytes available from SCC battery backed
 *		up RAM.
 *
.* ARGUMENTS: None.
 *
.* RETURN VALUE:
 *	Returns the size of the RAM available. Returns zero on failure.
 *
.* USAGE:
 *	This routine is used to get the size of SCC battery backed up RAM
 *	that is available. The battery backed up RAM is used for 
 *	the device configuration table.
 */

get_bbram_avail(bp)
struct buf *bp;
{

	register crq_bbram_msg_t *cmd;
	static isr_queue_t isr_q;
	int size, sts;

	/*
	 * Allocate a command packet.
	 */
	cmd = (crq_bbram_msg_t *)ms_getcmd(bp, &sts, FALSE);
	if (sts != ESUCCESS) {
		bp->b_error = sts;
		bp->b_flags |= B_ERROR;
		return(0);
	}
	
	initisrqueue(&isr_q);

	/*
	 * Initialize the bbram command message.
	 */
	cmd->bbram_hdr.crq_msg_code = CRQOP_SCC_GET_BBRAM_AVAIL;
	cmd->bbram_hdr.crq_msg_unitid = MAKEUNITID(0,SCC_SLOT,0,SLOT_LUN);
	cmd->bbram_hdr.crq_msg_refnum = (long) &isr_q;

	/*
	 * Send command to SCC to get the available RAM
	 */
	send_slot_cmd(cmd, SCC_SLOT);

	/*
	 * Synchronize with completion.
	 */
	if ((crq_bbram_msg_t *)
		get_isr_queue(cmd->bbram_hdr.crq_msg_refnum) == cmd) {
		
		/*
		 * Verify successful get available RAM
		 */
		if (cmd->bbram_hdr.crq_msg_status != STS_SUCCESS) {
			ms_freecmd((crq_ms_xfer_msg_t *)cmd);
			return(0);
		}
	} else {
		ms_freecmd((crq_ms_xfer_msg_t *)cmd);
		return(0);
	}

	/*
	 * Get the size and deallocate command packet.
	 */
	size = cmd->bbram_size;
	ms_freecmd((crq_ms_xfer_msg_t *)cmd);
	return(size);

}


/*2
.* get_current_dct - Get the device configuration table currently in the
 *		SCC battery backed up RAM.
 *
.* ARGUMENTS:
 *
.* bp - Pointer to buffer structure
 *
.* dct_size - Number of bytes in the structure that can receive the table
 *
.* RETURN VALUE:
 *	Also returns the amount of bytes read, or zero if error.
 *
.* USAGE:
 *	This routine is used to get the device configuration table. It is
 *	used by sysboot to actually look at entries, and by the kernel
 *	sysadmin system call to obtain the table.
 */

get_current_dct(bp,dct_size)
struct buf *bp;
int dct_size;
{

	int sts;
	register crq_dct_msg_t *cmd;
	static isr_queue_t isr_q;
	static dct_t loc_dct;
	int copysize;

	copysize = 0;
	/*
	 * Allocate a command packet.
	 */
	cmd = (crq_dct_msg_t *)ms_getcmd(bp, &sts, FALSE);
	if (sts != ESUCCESS) {
		goto noget;
	}

	initisrqueue(&isr_q);

	/*
	 * Initialize the get device configuration table command message.
	 */
	cmd->dct_hdr.crq_msg_code = CRQOP_SCC_GET_DCT;
	cmd->dct_hdr.crq_msg_unitid = MAKEUNITID(0,SCC_SLOT,0,SLOT_LUN);
	cmd->dct_hdr.crq_msg_refnum = (long) &isr_q;
	cmd->dct_addr = &loc_dct;
	cmd->dct_buff_size = dct_size;

	/*
	 * Send command to SCC to get the dct
	 */
	send_slot_cmd(cmd, SCC_SLOT);

	/*
	 * Synchronize with completion.
	 */
	if ((crq_dct_msg_t *)
		get_isr_queue(cmd->dct_hdr.crq_msg_refnum) == cmd) {
		
		/*
		 * Verify successful get dct
		 */
		if (cmd->dct_hdr.crq_msg_status != STS_SUCCESS)
			goto noget;
	} else
		goto noget;

	/*
	 * If the command succeeded, then copy the result to the caller's
	 * buffer (we do it this way because the caller's buffer may
	 * not be mapped physical==virtual).
	 */
	sts = ESUCCESS;
	if (loc_dct.dct_hdr.dct_total_bytes > 0) {
		copysize = min(dct_size,
				loc_dct.dct_hdr.dct_total_bytes);
		if (bp->b_flags & B_PHYS)
			sts = copyout((caddr_t) &loc_dct, bp->b_un.b_addr, copysize);
		else
			bcopy ((caddr_t) &loc_dct, bp->b_un.b_addr, copysize);
		if (sts != ESUCCESS)
			goto noget;
	}
	else
		goto noget;

	/*
	 * Deallocate command packet.
	 */
	ms_freecmd((crq_ms_xfer_msg_t *)cmd);
	return (copysize);

noget:
	bp->b_error = sts;
	bp->b_flags |= B_ERROR;
	return(copysize);
}

/*2
.* set_current_dct - Set the device configuration table currently in the
 *		SCC battery backed up RAM.
 *
.* ARGUMENTS:
 *
.* bp - pointer to buffer structure
 *
.* dct_size - Number of bytes in the structure.
 *
.* RETURN VALUE:
 *	Also returns the amount of bytes written, or zero if error.
 *
.* USAGE:
 *	This routine is used to set the device configuration table. It is
 *	used by the kernel sysadmin system call to set the table.
 */

set_current_dct(bp, dct_size)
struct buf *bp;
int dct_size;
{

	int sts;
	int copysize;
	register crq_dct_msg_t *cmd;
	static isr_queue_t isr_q;
	static dct_t loc_dct;

	copysize = 0;
	/*
	 * Allocate a command packet.
	 */
	cmd = (crq_dct_msg_t *)ms_getcmd(bp, &sts, FALSE);
	if (sts != ESUCCESS) {
		goto noset;
	}

	initisrqueue(&isr_q);

	/*
	 * Copy the caller's device configuration table to the newly 
	 * allocated one (NOTE: the caller's table may not be mapped 
	 * physical==virtual).
	 */
	sts = ESUCCESS;
	if (dct_size <= 0) {
		goto noset;
	} else {
		if (bp->b_flags & B_PHYS)
			sts = copyin(bp->b_un.b_addr, (caddr_t) &loc_dct, dct_size);
		else
			bcopy (bp->b_un.b_addr, (caddr_t) &loc_dct, dct_size);
		if (sts != ESUCCESS)
			goto noset;
	}

	/*
	 * Initialize the get state command message.
	 */
	cmd->dct_hdr.crq_msg_code = CRQOP_SCC_SET_DCT;
	cmd->dct_hdr.crq_msg_unitid = MAKEUNITID(0,SCC_SLOT,0,SLOT_LUN);
	cmd->dct_hdr.crq_msg_refnum = (long) &isr_q;
	cmd->dct_addr = &loc_dct;
	cmd->dct_buff_size = dct_size;

	/*
	 * Send command to SCC to set the dct
	 */
	if (dct_size) {
		send_slot_cmd(cmd, SCC_SLOT);

	/*
	 * Synchronize with completion.
	 */
		if ((crq_dct_msg_t *)
			get_isr_queue(cmd->dct_hdr.crq_msg_refnum) == cmd) {
		
		/*
		 * Verify successful set dct
		 */
			if (cmd->dct_hdr.crq_msg_status == STS_SUCCESS) {
				copysize = dct_size;
			} else {
				goto noset;
			}
		} else
			goto noset;

	}
	/*
	 * Deallocate command packet.
	 */
	ms_freecmd((crq_ms_xfer_msg_t *)cmd);
	return(copysize);

noset:
	bp->b_error = sts;
	bp->b_flags |= B_ERROR;
	return(copysize);
}


#ifdef	SCC_PROFILE
/*2
.* start_profiling - Start kernel profiling
 *
.* ARGUMENTS:
 *
.* rate - Rate at which profiling interrupts are to occur.
 *
.* RETURN VALUE:
 *	This procedure returns with u.u_error set to non-zero
 *	if starting profiling failed.
 *
.* USAGE:
 *	This routine is used to start kernel profiling interrupts
 *	occuring on a MULTIMAX.
 */

start_profiling(rate)
int rate;

/**/
{

	register crq_profile_msg_t *cmd;
	register isr_queue_t *isr_q;
	int error = 0;
#define start_prof_alloc_size (sizeof(crq_profile_msg_t)+sizeof(isr_queue_t))

	/*
	 * Allocate a command packet and isr queue.
	 */
	if ((cmd = (crq_profile_msg_t *)
		MALLOC(start_prof_alloc_size)) == 0) {
		return(EIO);
	}
	isr_q = (isr_queue_t *) ((caddr_t) cmd + sizeof(crq_profile_msg_t));
	initisrqueue(isr_q);

	/*
	 * Initialize the start profiling command message.
	 */
	cmd->profile_hdr.crq_msg_code = CRQOP_SCC_START_PROFILE;
	cmd->profile_hdr.crq_msg_unitid = MAKEUNITID(0,SCC_SLOT,0,SLOT_LUN);
	cmd->profile_hdr.crq_msg_refnum = (long) isr_q;
	cmd->profile_rate = rate;

	/*
	 * Send command to SCC to start profiling
	 */
	send_slot_cmd(cmd, SCC_SLOT);

	/*
	 * Synchronize with completion.
	 */
	if ((crq_profile_msg_t *)
		get_isr_queue(cmd->profile_hdr.crq_msg_refnum, 0) == cmd) {
		
		/*
		 * Verify successful start profiling
		 */
		if (cmd->profile_hdr.crq_msg_status != STS_SUCCESS)
			error = EIO;
	} else
		error = EIO;

	/*
	 * Deallocate command packet and isr queue.
	 */
	MFREE(start_prof_alloc_size, cmd);
	return(error);
}

/*2
.* stop_profiling - Stop kernel profiling
 *
.* ARGUMENTS: None
 *
.* RETURN VALUE:
 *	This procedure returns with u.u_error set to non-zero
 *	if stopping profiling failed.
 *
.* USAGE:
 *	This routine is used to stop kernel profiling interrupts from
 *	occuring on a MULTIMAX.
 */

stop_profiling()

/**/
{

	register crq_profile_msg_t *cmd;
	register isr_queue_t *isr_q;
	int error = 0;
#define stop_prof_alloc_size (sizeof(crq_profile_msg_t)+sizeof(isr_queue_t))

	/*
	 * Allocate a command packet and isr queue.
	 */
	if ((cmd = (crq_profile_msg_t *)
		MALLOC(stop_prof_alloc_size)) == 0) {
		return(EIO);
	}
	isr_q = (isr_queue_t *) ((caddr_t) cmd + sizeof(crq_profile_msg_t));
	initisrqueue(isr_q);

	/*
	 * Initialize the stop profiling command message.
	 */
	cmd->profile_hdr.crq_msg_code = CRQOP_SCC_STOP_PROFILE;
	cmd->profile_hdr.crq_msg_unitid = MAKEUNITID(0,SCC_SLOT,0,SLOT_LUN);
	cmd->profile_hdr.crq_msg_refnum = (long) isr_q;

	/*
	 * Send command to SCC to stop profiling
	 */
	send_slot_cmd(cmd, SCC_SLOT);

	/*
	 * Synchronize with completion.
	 */
	if ((crq_profile_msg_t *)
		get_isr_queue(cmd->profile_hdr.crq_msg_refnum, 0) == cmd) {
		
		/*
		 * Verify successful stop profiling
		 */
		if (cmd->profile_hdr.crq_msg_status != STS_SUCCESS)
			error = EIO;
	} else
		error = EIO;

	/*
	 * Deallocate command packet and isr queue.
	 */
	MFREE(stop_prof_alloc_size, cmd);
	return(error);
}
#endif	SCC_PROFILE


/*2
.* scc_reboot - cause the SCC of the MULTIMAX to reboot the system
 *
.* ARGUMENTS: None
 *
.* USAGE:
 *	This routine is the last thing called by the panic subsystem
 *	on a MULTIMAX. It sends a message to the SCC that causes
 *	the SCC to reboot the system.
 */

scc_reboot()
{
	static crq_msg_t reboot_cmd; /* static cause MALLOC() may not work */
	static isr_queue_t reboot_isrqueue; /* static for same reason */
	register i;

	/*
	 * Clear the slot CRQ pointers so that the send_slot_cmd() routine
	 * will do the right thing.
	 */
	for (i=0; i<NUM_SLOT; i++)
		Slot_crqs[i] = CRQ_NULL;

	/*
	 * Initialize the ISR queue for the command.
	 */
	initisrqueue(&reboot_isrqueue);

	/*
	 * Initialize the reboot command.
	 */
	reboot_cmd.crq_msg_code = CRQOP_SCC_REBOOT;
	reboot_cmd.crq_msg_unitid = MAKEUNITID(0,SCC_SLOT,0,SLOT_LUN);
	reboot_cmd.crq_msg_refnum = (long) &reboot_isrqueue;

	/*
	 * Send the command to the SCC to reboot the system.
	 */
	send_slot_cmd(&reboot_cmd,SCC_SLOT);
}


/*2
.* scc_isr - SCC slot level interrupt routine.
 *
.* ARGUMENTS:
 *
.* lun - Logical unit (REQ_LUN or SLOT_LUN)
 *
.* USAGE;
 *	This routine is the slot level interrupt routine. All attentions
 *	and responses are queued to the proper isr queue.
 */

scc_isr(ihp)
ihandler_t *ihp;
{
	register int lun = ihp->ih_hparam[0].intparam;
	register crq_msg_t *attn;
	register crq_msg_t *rsp;
	register crq_t *crq;

	/*
	 * Set the CRQ address based on logical unit.
	 */
	switch(lun) {

	case REQ_LUN:
		crq = REQ_CRQ(SCC_SLOT, 0);
		break;
	case SLOT_LUN:
		crq = Slot_crqs[SCC_SLOT];
		break;
	default:
		printf("Message from unknown SCC lun %d\n", lun);
		return;

	}

	/*
 	 * Log any attention messages
 	 */
	(void)log_scc_attn(crq);


	/*
	 * Dequeue and pass responses to post-processing queue.
	 */
	while ((rsp = (crq_msg_t *)rec_rsp(crq)) != CRQ_MSG_NULL) {
		put_isr_queue(rsp, rsp->crq_msg_refnum);
	}
}


/****************************************************************************
 *
 * NAME:
 *	log_scc_attn
 *
 * DESCRIPTION:
 *	Checks the specified CRQ for attention messages and outputs
 *	a message for each attention found.
 *
 * ARGUMENTS:
 *	crq		- pointer to a CRQ
 *
 * RETURN VALUE:
 *	Number of attention messages found (zero if none).
 *
 * SIDE EFFECTS:
 *	Logs various errors.
 *	May initiate a shutdown sequence (someday).
 *
 * EXCEPTIONS:
 *	none
 *
 * ASSUMPTIONS:
 *	none
 */
typedef struct gen_attn_msg {
    crq_msg_t msg_hdr;
    char    msg[MAX_ATTN_SIZE - sizeof(crq_msg_t)];
} gen_attn_msg_t;

int
log_scc_attn(crq)
	crq_t *crq;
{
	crq_msg_t	*attn, *rec_attn();
	int		n_found = 0,
		fatal  = 0,
		opcode;
	gen_attn_msg_t
		attn_msg,
		*msg = &attn_msg;
	char *s;
	extern char	*panicstr;

    while ((attn = rec_attn(crq)) != CRQ_MSG_NULL) {

	/*
	 * Make a local copy of the message and return the original
	 * message to the CRQ's free list, then deal with copy only.
	 */
	bcopy((char *)attn, (char *)msg, sizeof(gen_attn_msg_t));
	put_free(attn, crq);
	if (!panicstr)
		printf("SCC Attention received.  See error log for details.\n");

	n_found++;
	opcode = (int)(((crq_msg_t *)msg)->crq_msg_code);
	s = (char *)NULL;

	switch (opcode) {

	case CRQATTN_SL_READ:
	    s = "Serial Line Read";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	case CRQATTN_SL_WRITE:
	    s = "Serial Line Write";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	case CRQATTN_ENV:
	    s = "Environmental";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    fatal += environ((crq_env_t *)msg);
	    break;

	case CRQATTN_MEM_CSR:
	    s = "Memory CSR";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    log(LOG_NOTICE, "\tMemory Slot %d: %s%s%s Error(s).  CSR: 0x%x.",
		(int)((crq_mem_csr_t *)msg)->mem_csr_slot,
		(int)(((crq_mem_csr_t *)msg)->mem_csr_csr.double_err ?
						" \"Double Bit\"" : ""),
		((crq_mem_csr_t *)msg)->mem_csr_csr.single_err ?
						" \"Single Bit\"" : "",
		((crq_mem_csr_t *)msg)->mem_csr_csr.write_err ?
						" \"Write Data Parity\"" : "",
		((crq_mem_csr_t *)msg)->mem_csr_csr);
	    if ( (int)((crq_mem_csr_t *)msg)->mem_csr_csr.double_err)
		fatal++;
	    break;

	case CRQATTN_PS_FAIL:
	    s = "Power Supply Failure";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n\t%s%d%s",
		s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid,
		"Power Supply #",
		(int)((crq_ps_fail_t *)msg)->ps_fail_unit,
		" indicating failure.");
	    fatal++;
	    break;

	case CRQATTN_SOFTERR:
	    s = "Hardware Protocol";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n\t%s",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid,
		"Non-fatal SCC hardware protocol failure");
	    break;

	case CRQATTN_ALIVE:
	    /*
	     * Probably ought to do something constructive here.
	     */
	    s = "(Ignored) Are-You-Alive";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	case CRQATN_LOCK_TMO:
	    s = "Lock Timeout";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	case CRQATN_VECT_NOT_XMIT:
	    s = "Vector Not Transmitted";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	case CRQATN_VECT_NOT_RCV:
	    s = "Vector Not Received";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	case CRQATN_NO_FREE_MSG:
	    s = "No Free Message";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	case CRQATN_INVLD_CRQ:
	    s = "Invalid CRQ";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	case CRQATN_EMC_ATN:
	    s = "Unimplemented EMC";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n\tStatus/Xtnd = 0x%x/%x",
			s,
			(int) msg->msg_hdr.crq_msg_code,
			(int) msg->msg_hdr.crq_msg_unitid,
			(int)((emc_atn_msg_t *)msg)->emc_atn_status,
			(int)((emc_atn_msg_t *)msg)->emc_atn_xtnd_stat);
	    break;

	case CRQATN_MSC_ATN:
	    s = "Unimplemented MSC";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    break;

	default:
	    s = "Unimplemented";
	    log(LOG_NOTICE,"%s attention (via SCC).\n\tCode 0x%x Unit 0x%x\n",s,
		(int) msg->msg_hdr.crq_msg_code,
		(int) msg->msg_hdr.crq_msg_unitid);
	    /* break;	Don't Bother!	*/

	}

    }

    if (fatal)			/* Fatal error?			*/
	    panic("Fatal SCC attention");	/* Begin Shutdown sequence	*/

    return (n_found);
}


/****************************************************************************
 *
 * NAME:
 *	environ
 *
 * DESCRIPTION:
 *	After receiving an environmental attention, this routine will
 *	request the environmental data from the SCC and decode it.
 *
 * ARGUMENTS:
 *	environ_msg	- pointer to attention message.
 *
 * RETURN VALUE:
 *	0		- normally.
 *	1		- if a shutdown has been initiated.
 *
 * SIDE EFFECTS:
 *	none
 *
 * EXCEPTIONS:
 *	none
 *
 * ASSUMPTIONS:
 *	This routine must be called within 30 seconds of the attention message
 *	(before the SCC checks the environment again).  Otherwise, the SCC
 *	will overwrite its data for which the attention was sent.
 */
environ( environ_msg)
    crq_env_t *environ_msg;
{
    static char *stat_msg[] = {
	    "\n\tEnvironmental Sensor Warning!\n",
	    "\n\tSCC has confirmed the Power SHUTDOWN!\n",
	    "\n\tEnvironment is now normal...Shutdown sequence is cancelled!\n",
	    "\n\tSCC Environmental Sensor logic timed out!\n",
	    "\n\tSCC experienced a spurious Environmental Sensor interrupt!\n",
	    "\n\tSCC has initiated a Power SHUTDOWN sequence!\n",
	    "\n\tEnvironmental Sensor attention for unknown reason!\n"
	},
	*chan_msg[] = {
	    "SCC +5 voltage",
	    "SCC +12 voltage",
	    "SCC -12 voltage",
	    "Battery backup +5 voltage",
	    "Top of SCC temperature",
	    "Bottom of SCC temperature",
	    "Ambient temperature",
	    "Peripheral controller +5 voltage",
	    "Delta temperature:  Top of SCC - Ambient",
	    "Delta temperature:  Bottom of SCC - Top of SCC"
	};
    u_int	i, j, msg = environ_msg->env_attn_status - 1;

    if (msg >= (sizeof(stat_msg) / sizeof(stat_msg[0])))
	msg = (sizeof(stat_msg) / sizeof(stat_msg[0])) - 1;

    for (i = 1, j = 0; j < NUM_AD; i <<= 1, j++) {

	if (environ_msg->env_lo_wrning_mask & i) {
	    log(LOG_NOTICE, "\n\tLow warning on %s %s", (int)chan_msg[j], stat_msg[msg]);
	}
	if (environ_msg->env_hi_wrning_mask & i) {
	    log(LOG_NOTICE, "\n\tHigh warning on %s %s", (int)chan_msg[j], stat_msg[msg]);
	}
	if (environ_msg->env_lo_shtdwn_mask & i) {
	    log(LOG_NOTICE, "\n\tLow shutdown on %s %s", (int)chan_msg[j], stat_msg[msg]);
	}
	if (environ_msg->env_hi_shtdwn_mask & i) {
	    log(LOG_NOTICE, "\n\tHigh shutdown on %s %s", (int)chan_msg[j], stat_msg[msg]);
	}
    }

    return( environ_msg->env_attn_status == ENV_STATUS_SHTDWN ? 1 : 0 );
}

