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
static char     *sccsid = "@(#)$RCSfile: ntp_unixclock.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/07/07 15:52:11 $";
#endif
/*
 */

/*
 * ntp_unixclock.c - routines for reading and adjusting a 4BSD-style
 *		     system clock
 */

#include <stdio.h>
#include <nlist.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp_unixtime.h>
#define	CLOCK_ADJ	4

extern int debug;
extern int gettokens();
extern int getnetnum();
extern int allow_set_backward;
extern int dont_change_time;

/*
 * These routines (init_systime, get_systime, step_systime, adj_systime)
 * implement an interface between the (more or less) system independent
 * bits of NTP and the peculiarities of dealing with the Unix system
 * clock.  These routines will run with good precision fairly independently
 * of your kernel's value of tickadj.  I couldn't tell the difference
 * between tickadj==40 and tickadj==5 on a microvax, though I prefer
 * to set tickadj == 500/hz when in doubt.  At your option you
 * may compile this so that your system's clock is always slewed to the
 * correct time even for large corrections.  Of course, all of this takes
 * a lot of code which wouldn't be needed with a reasonable tickadj and
 * a willingness to let the clock be stepped occasionally.  Oh well.
 */

/*
 * Clock variables.  We round calls to adjtime() to adj_precision
 * microseconds, and limit the adjustment to tvu_maxslew microseconds
 * (tsf_maxslew fractional sec) in one four second period.  As we are
 * thus limited in the speed and precision with which we can adjust the
 * clock, we compensate by keeping the known "error" in the system time
 * in sys_offset.  This is added to timestamps returned by get_systime().
 * We also remember the clock precision we computed from the kernel in
 * case someone asks us.
 */
int adj_precision;	/* adj precision in usec (tickadj) */
int tvu_maxslew;	/* maximum adjust doable in CLOCK_ADJ sec (usec) */

unsigned int tsf_maxslew;	/* same as above, as long format */

static l_fp sys_offset;		/* correction for current system time */

/*
 * Tables for converting between time stamps and struct timeval's
 * are in the library.  Reference them here.
 */
extern u_int ustotslo[];
extern u_int ustotsmid[];
extern u_int ustotshi[];

extern int tstoushi[];
extern int tstousmid[];
extern int tstouslo[];

int log_ok=FALSE;

/*
 * init_systime - initialize the system clock support code, return
 *		  clock precision.
 *
 * Note that this code obtains to kernel variables related to the local
 * clock, tickadj and tick.  The code knows how the Berkeley adjtime
 * call works, and assumes these two variables are obtainable and are
 * used in the same manner.  Tick is supposed to be the number of
 * microseconds which are added to the system clock at clock interrupt
 * time when the time isn't being slewed.  Tickadj is supposed to be
 * the number of microseconds which are added or subtracted from tick when
 * the time is being slewed.
 *
 * If either of these two variables is missing, or is there but is used
 * for a purpose different than that described, you are SOL and may have
 * to do some custom kludging.
 *
 * This really shouldn't be in here.
 */
void
init_systime()
{
	u_int tickadj;
	u_int tick;
	u_int hz;
	void clock_parms();

	/*
	 * Obtain the values
	 */
	clock_parms(&tickadj, &tick);

	/*
	 * If tickadj or hz wasn't found, we're doomed.  If hz is
	 * unreasonably small, forget it.
	 */
	if (tickadj == 0 || tick == 0) {
		syslog(LOG_ERR, "tickadj or tick unknown, exiting");
		exit(3);
	}
	if (tick > 65535) {
		syslog(LOG_ERR, "tick value of %u is unreasonably large",
		    tick);
		exit(3);
	}

	/*
	 * Estimate hz from tick
	 */
	hz = 1000000L / tick;

	/*
	 * Set adj_precision and the maximum slew based on this.  Note
	 * that maxslew is set slightly shorter than it needs to be as
	 * insurance that all slews requested will complete in CLOCK_ADJ
	 * seconds.
	 */
#ifdef ADJTIME_IS_ACCURATE
	adj_precision = 1;
#else
	adj_precision = tickadj;
#endif /* ADJTIME_IS_ACCURATE */
	tvu_maxslew = tickadj * (hz-1) * CLOCK_ADJ;
	if (tvu_maxslew > 999990) {
		/*
		 * Don't let the maximum slew exceed 1 second in 4.  This
		 * simplifies calculations a lot since we can then deal
		 * with less-than-one-second fractions.
		 */
		tvu_maxslew = (999990/adj_precision) * adj_precision;
	}
	TVUTOTSF(tvu_maxslew, tsf_maxslew);
#ifdef DEBUG
	if (debug)
		printf(
	"adj_precision = %d, tvu_maxslew = %d, tsf_maxslew = 0.%08x\n",
		    adj_precision, tvu_maxslew, tsf_maxslew);
#endif

	/*
	 * Set the current offset to 0
	 */
	sys_offset.l_ui = sys_offset.l_uf = 0;
}


/*
 * get_systime - return the system time in timestamp format
 */
void
get_systime(ts)
	l_fp *ts;
{
	struct timeval tv;

#if !defined(SLEWALWAYS) && !defined(LARGETICKADJ)
	/*
	 * Quickly get the time of day and convert it
	 */
	(void) gettimeofday(&tv, (struct timezone *)NULL);
	TVTOTS(&tv, ts);
	ts->l_uf += TS_ROUNDBIT;	/* guaranteed not to overflow */
#else
	/*
	 * Get the time of day, convert to time stamp format
	 * and add in the current time offset.  Then round
	 * appropriately.
	 */
	(void) gettimeofday(&tv, (struct timezone *)NULL);
	TVTOTS(&tv, ts);
	L_ADD(ts, &sys_offset);
	if (ts->l_uf & TS_ROUNDBIT)
		L_ADDUF(ts, (unsigned int) TS_ROUNDBIT);
#endif	/* !defined(SLEWALWAYS) && !defined(LARGETICKADJ) */
	ts->l_ui += JAN_1970;
	ts->l_uf &= TS_MASK;
}


/*
 * step_systime - do a step adjustment in the system time (at least from
 *		  NTP's point of view.
 */
void
step_systime(ts)
	l_fp *ts;
{
#ifndef SLEWALWAYS
	struct timeval timetv, adjtv;
	int isneg = 0;
	extern char *lfptoa();
	extern char *tvtoa();
	extern char *utvtoa();

	
	/*
	 * We can afford to be sloppy here since if this is called
	 * the time is really screwed and everything is being reset.
	 */

#ifdef DEBUG
	if (debug > 4)
	  printf("STEP routine: sys_offset: %s ts: %s\n",
		 lfptoa(&sys_offset,9), lfptoa(&ts,9));
#endif
	L_ADD(&sys_offset, ts);
#ifdef DEBUG
	if (debug > 4)
	  printf("STEP routine: sys_offset: %s\n",
		 lfptoa(&sys_offset,9));
#endif
	if (L_ISNEG(&sys_offset)) {
		isneg = 1;
		if (allow_set_backward)
	      	    L_NEG(&sys_offset);
	}

		
	if (allow_set_backward || isneg==0) {
	  log_ok=TRUE;
	  TSTOTV(&sys_offset, &adjtv);

	  (void) gettimeofday(&timetv, (struct timezone *)NULL);
#ifdef DEBUG
	if (debug > 3) {
		printf("step: %s, sys_offset = %s, adjtv = %s, timetv = %s\n",
		    lfptoa(&ts, 9), lfptoa(&sys_offset, 9), tvtoa(&adjtv),
		    utvtoa(&timetv));
		syslog(LOG_INFO, "settimeofday:step: %s, sys_offset = %s, adjtv = %s, timetv = %s\n",
		    lfptoa(&ts, 9), lfptoa(&sys_offset, 9), tvtoa(&adjtv),
		    utvtoa(&timetv));
	      }
#endif
	if (isneg) {
		timetv.tv_sec -= adjtv.tv_sec;
		timetv.tv_usec -= adjtv.tv_usec;
		if (timetv.tv_usec < 0) {
			timetv.tv_sec--;
			timetv.tv_usec += 1000000;
		}
	} else {
		timetv.tv_sec += adjtv.tv_sec;
		timetv.tv_usec += adjtv.tv_usec;
		if (timetv.tv_usec >= 1000000) {
			timetv.tv_sec++;
			timetv.tv_usec -= 1000000;
		}
	}
	if (!dont_change_time){
	if (settimeofday(&timetv, (struct timezone *)NULL) != 0)
		syslog(LOG_ERR, "Can't set time of day: %m");
      }
#ifdef DEBUG
	if (debug > 3)
		printf("step: new timetv = %s\n", utvtoa(&timetv));
#endif
	sys_offset.l_ui = sys_offset.l_uf = 0;
	}
#else
	/*
	 * Just add adjustment into the current offset.  The update
	 * routine will take care of bringing the system clock into
	 * line.
	 */
	L_ADD(&sys_offset, ts);
#endif	/* SLEWALWAYS */
}


/*
 * adj_systime - called once every CLOCK_ADJ seconds to make system time
 *		 adjustments.
 */
void
adj_systime(adj)
	int adj;
{
	register unsigned int offset_i, offset_f;
	register int temp;
	register unsigned int residual;
	register int isneg = 0;
	struct timeval adjtv, oadjtv;
	extern char *mfptoa();
	extern char *umfptoa();
	extern char *utvtoa();
	extern char *tvtoa();

#ifdef DEBUG
	if (debug > 4)
	  printf("ADJUST routine: sys_offset: %s adj: %d\n",
		 lfptoa(&sys_offset,9), adj);
#endif
	/*
	 * Move the current offset into the registers
	 */
	offset_i = sys_offset.l_ui;
	offset_f = sys_offset.l_uf;

	/*
	 * Add the new adjustment into the system offset.  Adjust the
	 * system clock to minimize this.
	 */
	M_ADDF(offset_i, offset_f, adj);
#ifdef DEBUG
	if (debug > 4)
	  printf("ADJUST routine: sys_offset: %s \n",
		 lfptoa(&sys_offset,9));
#endif
	if (M_ISNEG(offset_i, offset_f)) {
		isneg = 1;
		M_NEG(offset_i, offset_f);
	}
#ifdef DEBUG
	if (debug > 4)
		printf("adj_systime(%s): offset = %s%s\n",
		    mfptoa((adj<0?-1:0), adj, 9), isneg?"-":"",
		    umfptoa(offset_i, offset_f, 9));
#endif

	adjtv.tv_sec = 0;
	if (offset_i > 0 || offset_f >= tsf_maxslew) {
		/*
		 * Slew is bigger than we can complete in
		 * the adjustment interval.  Make a maximum
		 * sized slew and reduce sys_offset by this
		 * much.
		 */
		M_SUBUF(offset_i, offset_f, tsf_maxslew);
		if (!isneg) {
			adjtv.tv_usec = tvu_maxslew;
		} else {
			adjtv.tv_usec = -tvu_maxslew;
			M_NEG(offset_i, offset_f);
		}

#ifdef DEBUG
		if (debug > 4)
			printf(
			    "maximum slew: %s%s, remainder = %s\n",
			    isneg?"-":"", umfptoa(0, tsf_maxslew, 9),
			    mfptoa(offset_i, offset_f, 9));
#endif
	} else {
		/*
		 * We can do this slew in the time period.  Do our
		 * best approximation (rounded), save residual for
		 * next adjustment.
		 *
		 * Note that offset_i is guaranteed to be 0 here.
		 */
		TSFTOTVU(offset_f, temp);
#ifndef ADJTIME_IS_ACCURATE
		/*
		 * Round value to be an even multiple of adj_precision
		 */
		residual = temp % adj_precision;
		temp -= residual;
		if (residual << 1 >= adj_precision)
		temp += adj_precision;
#endif /* ADJTIME_IS_ACCURATE */
		TVUTOTSF(temp, residual);
		M_SUBUF(offset_i, offset_f, residual);
		if (isneg) {
			adjtv.tv_usec = -temp;
			M_NEG(offset_i, offset_f);
		} else {
			adjtv.tv_usec = temp;
		}
#ifdef DEBUG
		if (debug > 4)
			printf(
		"slew adjtv = %s, adjts = %s, sys_offset = %s\n",
			    tvtoa(&adjtv), umfptoa(0, residual, 9),
			    mfptoa(offset_i, offset_f, 9));
#endif
	      }

	sys_offset.l_ui = offset_i;
	sys_offset.l_uf = offset_f;

	if (adjtv.tv_usec == 0)
		return;
	if (!dont_change_time){
	  if (adjtime(&adjtv, &oadjtv) != 0)
	    syslog(LOG_ERR, "Can't do time adjustment: %m");
#ifdef DEBUG
	  if (debug > 4)
	    printf(
		   "adjtime: adjtv= %s  oadjtv= %s\n",
		   tvtoa(&adjtv), tvtoa(&oadjtv));
#endif
	}
	if (oadjtv.tv_sec != 0 || oadjtv.tv_usec != 0) {
		syslog(LOG_DEBUG, "Previous time adjustment didn't complete");
#ifdef DEBUG
		if (debug > 4)
			printf(
			    "Previous adjtime() incomplete, residual = %s\n",
			    tvtoa(&oadjtv));
#endif
	}
}



#ifndef NOKMEM
/*
 * clock_parms - return the local clock tickadj and tick parameters
 *
 * Note that this version grovels about in /dev/kmem to determine
 * these values.  This probably should be elsewhere.
 */
void
clock_parms(tickadj, tick)
	u_int *tickadj;
	u_int *tick;
{
	register int i;
	int kmem;
	static struct nlist nl[] =
	{	{"_tickadj"},
		{"_tick"},
		{""},
	};
	static char *kernelnames[] = {
		"/vmunix",
		"/unix",
		NULL
	};
	struct stat stbuf;
	int vars[2];

#define	K_TICKADJ	0
#define	K_TICK		1
	/*
	 * Read clock parameters from kernel
	 */
	kmem = open("/dev/kmem", O_RDONLY);
	if (kmem < 0) {
		syslog(LOG_ERR, "Can't open /dev/kmem for reading: %m");
#ifdef	DEBUG
		if (debug)
			perror("/dev/kmem");
#endif
		exit(3);
	}

	for (i = 0; kernelnames[i] != NULL; i++) {
		if (stat(kernelnames[i], &stbuf) == -1)
			continue;
		if (nlist(kernelnames[i], nl) >= 0)
			break;
	}
	if (kernelnames[i] == NULL) {
		syslog(LOG_ERR,
		  "Clock init couldn't find kernel as either /vmunix or /unix");
		exit(3);
	}

	for (i = 0; i < (sizeof(vars)/sizeof(vars[0])); i++) {
		off_t where;

		vars[i] = 0;
		if ((where = nl[i].n_value) == 0) {
			syslog(LOG_ERR, "Unknown kernal var %s",
			       nl[i].n_name);
			continue;
		}
		if (lseek(kmem, where, L_SET) == -1) {
			syslog(LOG_ERR, "lseek for %s fails: %m",
			       nl[i].n_name);
			continue;
		}
		if (read(kmem, &vars[i], sizeof(int)) != sizeof(int)) {
			syslog(LOG_ERR, "read for %s fails: %m",
			       nl[i].n_name);
		}
	}
#ifdef	DEBUG
	if (debug) {
		printf("kernel vars: tickadj = %d, tick = %d\n",
		       vars[K_TICKADJ], vars[K_TICK]);
	}
#endif
	close(kmem);

	*tickadj = (u_int)vars[K_TICKADJ];
	*tick = (u_int)vars[K_TICK];

#undef	K_TICKADJ
#undef	K_TICK
}
#endif /* !NOKMEM */

/*
 * Translation table - keywords to function index
 */
#define CONFIG_UNKNOWN		0

#define	CONFIG_PEER		1
#define	CONFIG_SERVER		2


/* init_time() is run when the "-i" boottime flag is used with xntpd.
 * It sets the system time using NTP.
 */
void
  init_time()
{
  extern char *save_peeraddr[];
  char ntpdate_str[256];
  int i, tok, ntokens;
  FILE *fp;
  char config_file[25];
  extern int count_peers;
  char line[1024];
  char *tokens[20];
  struct sockaddr_in peeraddr;
	
  /* Read the config file for peers only */
  strcpy(config_file,"/etc/ntp.conf");
	
  if ((fp = fopen(config_file, "r")) == NULL) {
    /*
     * Broadcast clients can sometimes run without
     * a configuration file.
     */
    return;
  }

  while ((tok = gettokens(fp, line, tokens, &ntokens))
	 != CONFIG_UNKNOWN) {
    switch(tok) 
      {
      case CONFIG_PEER:
      case CONFIG_SERVER:
	if (getnetnum(tokens[1], &peeraddr, 0)) {
	  /* save_peeraddr is used by init_time */
	  count_peers++;
	  save_peeraddr[count_peers] = (char *) malloc(64);
	  sprintf (save_peeraddr[count_peers], "%s", ntoa(&peeraddr));
	}
	break;
     
      default:
	break;
      }
  }
			
  (void) rewind (fp);

  strcat (ntpdate_str, "/usr/sbin/ntpdate -bo ");
  for (i = 1; i < 4; i++) {
    strcat (ntpdate_str, save_peeraddr[i]);
    strcat (ntpdate_str, " ");
  }
  
  system ( ntpdate_str );
}
