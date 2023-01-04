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
static char	*sccsid = "@(#)$RCSfile: rtc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:10:05 $";
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
 * Copyright (c) 1988 Intel Corporation.
 * All rights reserved.
 *
 * INTEL CORPORATION PROPRIETARY INFORMATION
 *
 * This software is supplied under the terms of a license agreement 
 * or nondisclosure agreement with Intel Corporation and may not be 
 * copied or disclosed except in accordance with the terms of that 
 * agreement.
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <i386/AT386/rtc.h>

static unsigned char rtc[RTC_NREG];
static int first_rtcopen_ever = 1;

rtcinit()
{
	outb(RTC_ADDR, RTC_A);
	outb(RTC_DATA, RTC_DIV2 | RTC_RATE6);
	outb(RTC_ADDR, RTC_B);
	outb(RTC_DATA, RTC_HM);
}


int
rtcget(regs)
unsigned char	*regs;
{
	if (first_rtcopen_ever) {
		rtcinit();
		first_rtcopen_ever = 0;
	}
	outb(RTC_ADDR, RTC_D); 
	if (inb(RTC_DATA) & RTC_VRT == 0) return(-1);
	outb(RTC_ADDR, RTC_A);	
	while (inb(RTC_DATA) & RTC_UIP)		/* busy wait */
		outb(RTC_ADDR, RTC_A);	
	load_rtc(regs);
	return(0);
}	

rtcput(regs)
unsigned char 	*regs;
{
	register unsigned char	x;

	if (first_rtcopen_ever) {
		rtcinit();
		first_rtcopen_ever = 0;
	}
	outb(RTC_ADDR, RTC_B);
	x = inb(RTC_DATA);
	outb(RTC_ADDR, RTC_B);
	outb(RTC_DATA, x | RTC_SET); 	
	save_rtc(regs);
	outb(RTC_ADDR, RTC_B);
	outb(RTC_DATA, x & ~RTC_SET); 
}


extern struct timeval time;
extern struct timezone tz;

static int month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

yeartoday(year)
int year;
{
	return((year%4) ? 365 : 366);
}

hexdectodec(n)
char n;
{
	return(((n>>4)&0x0F)*10 + (n&0x0F));
}

char
dectohexdec(n)
int n;
{
	return((char)(((n/10)<<4)&0xF0) | ((n%10)&0x0F));
}


readtodc(tp)
	time_t	*tp;
{
	struct rtc_st rtclk;
	time_t n;
	int sec, min, hr, dom, mon, yr;
	int i, ospl, days = 0;

	ospl = spl5();
	if (rtcget(&rtclk)) {
		splx(ospl);
		return(-1);
	}
	splx (ospl);

	sec = hexdectodec(rtclk.rtc_sec);
	min = hexdectodec(rtclk.rtc_min);
	hr = hexdectodec(rtclk.rtc_hr);
	dom = hexdectodec(rtclk.rtc_dom);
	mon = hexdectodec(rtclk.rtc_mon);
	yr = hexdectodec(rtclk.rtc_yr);
	yr = (yr < 70) ? yr+100 : yr;

	n = sec + 60 * min + 3600 * hr;
	n += (dom - 1) * 3600 * 24;

	if (yeartoday(yr) == 366)
		month[1] = 29;
	for (i = mon - 2; i >= 0; i--)
		days += month[i];
	month[1] = 28;
	for (i = 70; i < yr; i++)
		days += yeartoday(i);
	n += days * 3600 * 24;

	n += tz.tz_minuteswest * 60;
	if (tz.tz_dsttime)
		n -= 3600;

	*tp = n;

	return(0);
}

writetodc()
{
	struct rtc_st rtclk;
	time_t n;
	int ospl, diff, i, j;

	ospl = spl5();
	if (rtcget(&rtclk)) {
		splx(ospl);
		return(-1);
	}
	splx(ospl);

	diff = tz.tz_minuteswest * 60;
	if (tz.tz_dsttime)
		diff -= 3600;
	n = (time.tv_sec - diff) % (3600 * 24);   /* hrs+mins+secs */
	rtclk.rtc_sec = dectohexdec(n%60);
	n /= 60;
	rtclk.rtc_min = dectohexdec(n%60);
	rtclk.rtc_hr = dectohexdec(n/60);

	n = (time.tv_sec - diff) / (3600 * 24);	/* days */
	rtclk.rtc_dow = (n + 4) % 7;  /* 1/1/70 is Thursday */

	for (j = 1970, i = yeartoday(j); n >= i; j++) {
		n -= i;
		i = yeartoday(j);
	}
	rtclk.rtc_yr = dectohexdec(j - 1900);

	if (i == 366)
		month[1] = 29;
	for (i = 0; n >= month[i]; i++)
		n -= month[i];
	month[1] = 28;
	rtclk.rtc_mon = dectohexdec(++i);

	rtclk.rtc_dom = dectohexdec(++n);

	ospl = spl5();
	rtcput(&rtclk);
	splx(ospl);

	return(0);
}

