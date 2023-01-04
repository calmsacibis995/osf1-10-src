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
static char *rcsid = "@(#)$RCSfile: mc146818clock.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/12/09 20:25:04 $";
#endif
#ifdef DEBUGGING
static char *gravaDBlabels[] =
{
        "RTC_SECS",     /* 0 */
        "UNDEFINED",    /* 1 */
        "RTC_MINS",     /* 2 */
        "UNDEFINED",    /* 3 */
        "RTC_HRS",      /* 4 */
        "UNDEFINED",    /* 5 */
        "UNDEFINED",    /* 6 */
        "RTC_DAYS",     /* 7 */
        "RTC_MONTHS",   /* 8 */
        "RTC_YEARS",    /* 9 */
        "RTC_REGA"      /* a-10 */
        "RTC_REGB"      /* b-11 */
        "RTC_REGC"      /* c-12 */
        "RTC_REGD"      /* d-13 */
};
int     gravaTimeFlag = 0; /* test with kdbx to assign gravaTimeFlag=1
*/
#endif


/***************************************************************************
 *
 * 	mc146818clock.c
 *
 *	motorola 146818 compatible clock code
 *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <machine/clock.h>
#include <io/common/devdriver.h>
#include <hal/mc146818clock.h>


/*
 * define global RAP and RDP addresses to be filled in by
 * cpu initialization routines of systems which
 * use this clock code.  Define two seperate values to
 * protect us against sparse space causing rdp to not
 * be simply at rap address + 1.
 *
 * No longer set up rap and rdp here, moved to knxx file.
 *
 */
io_handle_t	mc146818clock_rap;
io_handle_t	mc146818clock_rdp;

/*
 * read and write routines for generic RTC register
 * access through the standard read/write port routines.
 * To access RTC registers,  write the target register offset
 * to the RAP (register address port) and read/write the the
 * RDP (register data port).
 */

u_int rtc_read(offset)
	u_int offset;	/* The RTC register offset */
{
#ifdef DEBUGGING
if (offset == 9) printf ("READ YEARS\n");
printf ("rtc_read(%d)\n", offset);
#endif
    WRITE_BUS_D8( mc146818clock_rap, offset); 
    mb();
    return(READ_BUS_D8( mc146818clock_rdp));
}

u_int rtc_write(offset,data) 
	u_int offset;	/* The RTC register offset */
	u_int data;	/* New RTC value */
{
#ifdef DEBUGGING
if (offset == 9) printf ("WRITE YEARS\n");
printf ("rtc_write(offset %d, data %d)\n", offset, data);
#endif
    WRITE_BUS_D8( mc146818clock_rap, offset);
    mb();
    WRITE_BUS_D8( mc146818clock_rdp, data);
    mb();
}


/*************************************************************
 *
 *	mc146818_readtodclk()
 *
 *	read the time-of-day clock chip and
 *	return the number of 100ns units 
 *
 **************************************************************/

u_long mc146818_readtodclk()
{
    struct tm tm;
    long rc = 0;
    int s;
    uint rega, regb, regc, regd;

    /*
     * If UIP (update in progress) is set, the 146818 should not be read.
     * Furthermore, the 146818 can not be read more than 1/4 of the
     * available bus cycles and not more than 50 contiguous references
     * can be to the chip.  Therefore the DELAY(10).
     *
     * If UIP is not set, then we have 244us to read the state of the
     * 146818, hence the spls.
     */
    s = splextreme();

    rega = rtc_read( RTC_REGA );
    while (rega & RTA_UIP) {
	splx(s);
	DELAY(10);
	s = splextreme();
	rega = rtc_read( RTC_REGA );
    }

    regb = rtc_read(RTC_REGB);
    if ((regb & RTB_DMBINARY) == 0) {
	printf("WARNING: Resetting TOY from BCD mode, must set time manually\n");
	regb |= RTB_DMBINARY;
	rtc_write(RTC_REGB, regb);
	return (0);
    }
    
    regb = rtc_read(RTC_REGB);
    if ((regb & RTB_24HR) == 0) {
	printf ("WARNING: Resetting TOY from 12-hour mode - must set time manually\n");
	regb |= RTB_24HR;
	rtc_write(RTC_REGB, regb);
	return (0);
    }

    tm.tm_sec = rtc_read( RTC_SECS );
    tm.tm_min = rtc_read( RTC_MINS );
    tm.tm_hour = rtc_read( RTC_HRS );
    tm.tm_mday = rtc_read( RTC_DAYS );   /* Day of the month */
    tm.tm_mon = rtc_read( RTC_MONTHS ) - 1;  /* 0..11, not 1..12 */
#ifdef DEBUGGING
if (gravaTimeFlag)
{
    /*
    **  do it the old way
    */
    tm.tm_year = rtc_read( RTC_YEARS );
    printf ("setting tm.tm_year from hardware\n");
}
else
{
#endif
    /*
    **  force the year to be 1970 (that was the epoch that *WAS*!)
    */
    tm.tm_year = 70;
#ifdef DEBUGGING
    printf ("mc146818_readtodclk: forcing year to 70\n");
}
#endif
    rc = cvt_tm_to_sec(&tm);

    rc *= UNITSPERSEC;	/* Scale to 100ns ticks */
    rc += TODRZERO;		/* Indicate a valid time */

    splx(s);
    return (rc);
}


/*************************************************************
 *
 *	mc146818_writetodclk()
 *
 *	write the time-of-day to clock chip
 *
 **************************************************************/

mc146818_writetodclk(u_long yrtime )
{
    struct tm tm; 

    cvt_sec_to_tm(yrtime, &tm);

    rtc_write( RTC_SECS,  tm.tm_sec );
    rtc_write( RTC_MINS,  tm.tm_min );
    rtc_write( RTC_HRS,  tm.tm_hour );
    rtc_write( RTC_DAYS,  tm.tm_mday );
    rtc_write( RTC_MONTHS,  tm.tm_mon + 1 );   /* 1..12, not 0..11 */
#ifdef DEBUGGING
if (gravaTimeFlag)
{
    /*
    **  do it the old way
    */
    rtc_write( RTC_YEARS,  tm.tm_year );
    printf ("mc146818_writetodclk: setting year register\n");
}
else
{
    /*
    ** DO NOT WRITE TO YEAR REGISTER
    */
    printf ("mc146818_writetodclk: leaving tm.tm_year untouched\n");
}
#endif
}


