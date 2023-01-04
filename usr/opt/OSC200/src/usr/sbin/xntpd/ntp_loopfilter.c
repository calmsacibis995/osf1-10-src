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
static char     *sccsid = "@(#)$RCSfile: ntp_loopfilter.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 16:57:27 $";
#endif
/*
 */
/*
 * ntp_loopfilter.c - implements the NTP loop filter algorithm
 */

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>

/*
 * The loop filter is implemented in slavish adherence to the
 * specification (Section 5), exept that for consistency and
 * generality we carry out the calculations to full time stamp
 * precision.
 *
 * Note that the long values below are the fractional portion of
 * a long fixed-point value.  This limits these values to +-0.5
 * seconds.  Since adjustments are capped at +-0.128 s, both the
 * clock_adjust and the compliance registers should be fine.  The
 * drift_comp register could conceivably exceed this range so it
 * is carried, at some pain, as a full l_fp.
 */
l_fp last_offset;	/* last adjustment done */

int clock_adjust;	/* clock adjust register (fraction only) */
l_fp drift_comp;	/* drift compensation register */
int compliance;	/* compliance register (fraction only) */

static int drift_adjust;	/* drift adjustment = drift_comp>>CLOCK_FREQ */

u_int watchdog_timer;	/* watchdog timer, in seconds */
int first_adjustment;	/* set to 1 if we're waiting for our first adjustment */

/*
 * Debug flag importation
 */
extern int debug;


/*
 * init_loopfilter - initialize loop filter data
 */
void
init_loopfilter()
{
	clock_adjust = 0;
	drift_comp.l_ui = 0;
	drift_comp.l_uf = 0;
	compliance = 0;
	drift_adjust = 0;
	watchdog_timer = 0;
	last_offset.l_i = 0;
	last_offset.l_f = 0;
	first_adjustment = 1;
}


/*
 * local_clock - the NTP logical clock loop filter.  Returns 1 if the
 *		 clock was stepped, 0 if it was slewed and -1 if it is
 *		 hopeless.
 */
int
local_clock(fp_offset, from)
	l_fp *fp_offset;		/* best offset estimate */
	struct sockaddr_in *from;	/* who offset is from, for messages */
{
	register int offset;
	register u_int tmp_ui;
	register u_int tmp_uf;
	register int tmp;
	int isneg;
	extern void step_systime();
	extern char *ntoa();
	extern char *lfptoa();
	extern char *mfptoa();
	extern int correct_any;
	extern int no_log;
	extern int log_ok;

#ifdef DEBUG
	if (debug > 1)
		printf("local_clock(%s, %s)\n", lfptoa(fp_offset, 9),
		    ntoa(from));
#endif

	/*
	 * Take the absolute value of the offset
	 */
	tmp_ui = fp_offset->l_ui;
	tmp_uf = fp_offset->l_uf;
	if (M_ISNEG(tmp_ui, tmp_uf)) {
		M_NEG(tmp_ui, tmp_uf);
		isneg = 1;
	} else
		isneg = 0;

	/*
	 * If the clock is way off, don't tempt fate by correcting it.
	 * Unless the -g "correct any" flag is set.
	 */
	if (!correct_any){
	  if (tmp_ui >= CLOCK_WAYTOOBIG) {
	    syslog(LOG_ERR,
		   "Clock appears to be %u seconds %s, something may be wrong",
		   tmp_ui, isneg>0?"slow":"fast");
	    return (-1);
	  }
	}

	/*
	 * Save this offset for later perusal
	 */
	last_offset = *fp_offset;

	/*
	 * If the magnitude of the offset is greater than CLOCK.MAX, step
	 * the time and reset the registers.
	 */
	if (tmp_ui > CLOCK_MAX_I || (tmp_ui == CLOCK_MAX_I
	    && (u_int)tmp_uf >= (u_int)CLOCK_MAX_F)) {
	  

		step_systime(fp_offset);

	        if (!no_log && log_ok){
		  syslog(LOG_INFO, "adjust: STEP %s offset %s\n",
			 ntoa(from), lfptoa(fp_offset));
		}

		
		clock_adjust = 0;
		watchdog_timer = 0;
		first_adjustment = 1;
		if (!log_ok)
		  return(0);
		else
		  {
		    log_ok=FALSE;
		    return (1);
		  }
	}


	/*
	 * Here we've got an offset small enough to slew.  Note that
	 * since the offset is small we don't have to carry the damned
	 * high order intword in our calculations.
	 *
	 * I don't have Dave Mills' religious zeal for avoiding 
	 * multiplications and divisions, and I'm not nearly comfortable
	 * enough with NTP's operations to make the approximations he
	 * does.  Oh, well.
	 */
	offset = fp_offset->l_f;
	clock_adjust = offset;

	/*
	 * Calculate the frequency gain.  For our purposes this is
	 * an integer between 1 and 16 inclusive.  There is some fancy
	 * footwork going on here.  The fraction is actually kept in
	 * units of 2**-FRACTION_PREC seconds (2**-32).  The compliance
	 * factor is 2**CLOCK_FACTOR/second (2**18).  This means a right
	 * shift of (FRACTION_PREC-CLOCK_FACTOR) will give us their product.
	 * If this is greater than or equal to the compliance maximum
	 * (2**CLOCK_COMP, or 2**4) we use a gain of 1, meaning we don't
	 * have to bother with any of this if the compliance is greater
	 * than or equal to 2**(FRACTION_PREC-CLOCK_FACTOR+CLOCK_COMP).
	 * Otherwise we subtract the value from 2**4 to obtain the gain.
	 * The gain is multiplied by the watchdog timer time.
	 */
	if (first_adjustment) {
		first_adjustment = 0;
	} else if (watchdog_timer > (1<<(NTP_MINPOLL-4))) {
		/*
		 * Avoid spurious jumps by only updating the
		 * frequency error if the elapsed time since the
		 * last update exceeds 16 seconds.
		 */
		if (compliance < 0)	/* take absolute value of compliance */
			tmp_uf = (u_int) -compliance;
		else
			tmp_uf = (u_int) compliance;
		tmp_uf >>= FRACTION_PREC-CLOCK_FACTOR;
		if (tmp_uf >= (1<<CLOCK_COMP)) {
			tmp_uf = watchdog_timer;		/* gain is 1 */
		} else if (tmp_uf == 0) {
			tmp_uf = watchdog_timer << CLOCK_COMP;
		} else {
			tmp_uf = (1<<CLOCK_COMP) - tmp_uf; /* tmp is now gain */
			tmp_uf *= watchdog_timer;	/* evil multiply */
		}
		tmp = offset/(int)tmp_uf;  /* evil divide */
#ifdef DEBUG
		if (debug > 2)
			printf("watchdog %u, gain %u, change %s\n",
			    watchdog_timer, tmp_uf,
			    mfptoa((tmp<0)?(-1):0, tmp, 9));
#endif
		L_ADDF(&drift_comp, tmp);

		/*
		 * Assume that drift_comp >> CLOCK_FREQ is small enough
		 * to fit in a fraction.  Do the shifts all at once.
		 */
		drift_adjust = 
		    (int)((drift_comp.l_ui << (FRACTION_PREC-CLOCK_FREQ))
		    | (drift_comp.l_uf >> CLOCK_FREQ));
		
		/*
		 * Round the adjustment value.
		 */
		if (drift_comp.l_uf & (1<<(CLOCK_FREQ-1)))
			drift_adjust++;
	}
	watchdog_timer = 0;

	/*
	 * Calculate compliance for next go around
	 */
	tmp = offset - compliance;
	if (tmp < 0)
		compliance -= (-tmp) >> CLOCK_TRACK;
	else
		compliance += tmp >> CLOCK_TRACK;

#ifdef DEBUG
	if (debug > 1)
		printf("adj %s, drft %s, drft_adj %s, cmpl %s\n",
		    mfptoa((clock_adjust<0?-1:0), clock_adjust, 9),
		    lfptoa(&drift_comp, 9),
		    mfptoa((drift_adjust<0?-1:0), drift_adjust, 9),
		    mfptoa((compliance<0?-1:0), compliance, 9));
#endif
	if (!no_log){
	  syslog(LOG_DEBUG, "adjust: SLEW %s off %s drft %s cmpl %s\n",
		 ntoa(from), mfptoa((clock_adjust<0?-1:0), clock_adjust, 9),
		 lfptoa(&drift_comp, 9), 
		 mfptoa((compliance<0?-1:0), compliance, 9));
	}
	/*
	 * Whew.  I've had enough.
	 */
	return (0);
}


/*
 * adj_host_clock - Called every 2**CLOCK_ADJ seconds to update host clock
 */
void
adj_host_clock()
{
	register int adjustment;
	extern void adj_systime();

	adjustment = clock_adjust;
	if (adjustment < 0)
		adjustment = -((-adjustment) >> CLOCK_PHASE);
	else
		adjustment >>= CLOCK_PHASE;

	clock_adjust -= adjustment;
	adjustment += drift_adjust;
	adj_systime(adjustment);

	watchdog_timer += (1<<CLOCK_ADJ);
	if (watchdog_timer >= NTP_MAXAGE) {
		first_adjustment = 1;	/* don't use next offset for freq */
	}
}


/*
 * loop_config - configure the loop filter
 */
void
loop_config(item, value)
	int item;
	l_fp *value;	/* only one type of value so far */
{
	switch (item) {
	case LOOP_DRIFTCOMP:
		drift_comp = *value;
		/*
		 * Assume that drift_comp >> CLOCK_FREQ is small enough
		 * to fit in a fraction.  Do the shifts all at once.
		 */
		drift_adjust = 
		    (int)((drift_comp.l_ui << (FRACTION_PREC-CLOCK_FREQ))
		    | (drift_comp.l_uf >> CLOCK_FREQ));
		
		/*
		 * Round the adjustment value.
		 */
		if (drift_comp.l_uf & (1<<(CLOCK_FREQ-1)))
			drift_adjust++;
		break;
	default:
		/* sigh */
		break;
	}
}
