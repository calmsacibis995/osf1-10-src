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
static char *rcsid = "@(#)$RCSfile: generic.c,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/10/18 20:08:43 $";
#endif

/*
 * 16-Aug-1989 - Randall Brown
 *		Fixed usleep() so that delays over 1 seconds will work.
 *		When an input string wait fails without receiving any 
 *		chars, the debug statement no longer prints garbage.
 *
 * 28-Apr-1988 - Tim Burke
 *		Added a flag to acucap called rd (reset delay).  If the re
 *		flag is specified which specifies to toggle DTR, examine the
 *		rd flag for a one second delay.
 *
 * 16-Jul-1987 - John Williams
 *		Put speed routine from tip.c into generic.c 
 *
 * 20-Jun-1987 - Tim Burke
 *		Added the capability of operating the modem at two speeds.
 *		The speed specified by "is" will be set.  Then a string "co"
 *		will be sent to the modem to change default speed.  Next the
 *		line speed will be changed to "xs" before dialing.
 *
 * 22-Apr-1987 - Marc Teitelbaum
 *		Do not "close" the dialer FD - ever.  It was opened
 *		for us, and is mean't to be used again in trying
 *		new numbers.  If we close it, the following tries would
 *		fail. Instead just flush both the input and output queues
 *		upon failure.
 * 28-Jan-1987 - Marc Teitelbaum
 *		Fix to only TIOCFLUSH the input queue. Previously
 *		it was flushing the output queue before all the
 *		dial characters got out to the modem.  This is why
 *		short delays between characters didn't work - not
 *		because the modems couldn't handle the speed - but
 *		rather because the last few dial chars were sweeped 
 *		off the tty output queue and never got to the modem.
 *		Also, check the return code of writes in output(),
 *		and if debug is set, report unsuccessful writes.
 * 14-Jan-1987 - Marc Teitelbaum
 *		Add TIOCMODEM ioctl.  Uucp generic forgot to, and its
 *		better done here anyway.  Add ability to imbed one
 *		second delays in the strings (\d).  These are a must
 *		for getting the most out of (really) smart modems
 *		like the hayes.  Change the delay sent on the abort
 *		and disconnect strings to dialdel.  It was set to "1",
 *		which, if lsleep is set, is ridiculousely small.
 * 12-Sep-1986 - Marc Teitelbaum
 *		Fix dialer hang.  If the acucap entry specified a
 *		disconnect or abort string, and the dial failed, then
 *		the write would hang waiting for carrier (but WITHOUT
 *		a timeout).  The fix is to again set TIOCNCAR to ignore
 *		carrier while writing the abort or disconnect string.
 * 11-05-85	Bugfix. Reset timedout to 0 else uucp can get into
 *		a state where it hangs up on people (thinks an 
 *		error has happened even though it has not).
 * 10-14-85     Changed input debug messages. Added replacestr to account
 *		for uucp's use of '-' to mean delay. This string 
 *		allows a characters worth of substitution whenever the
 *		characters '-' OR '=' are seen in the phone number.	
 * 10-10-85	Changed how response strings come back. Now will
 *		only eat up to a match. Otherwise, will eat everything
 *		in typeahead looking for respstr. Also added new
 *		string 'os' as what is seen when we really get a 
 *		carrier. Finally added 'si' boolean for modem
 *		attached to stupid interfaces such as dmf32 which
 *		only passes chars back after carrier/CTS/DCD are
 *		present. lp@decvax 
 * Routines for calling up on any modem.
 *              by lp@decvax  3/12/85
 */

#include <sys/time.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/termio.h>

#define NOSYNC 1
#define BADDIAL 2
#define NOCAR 3
#define GENBUF 512
#define DEFDELAY 1

#define GEN_TRUE		1
#define GEN_FALSE		0
#define GEN_SUCCESS		0
#define GEN_FAILURE		-1
#define GEN_WARNING		1

#define P(fmtstr)		fprintf (stderr, fmtstr)
#define P1(fmtstr,arg)		fprintf (stderr, fmtstr, arg)
#define D(fmtstr)		if (debugn) fprintf (stderr, fmtstr)
#define D1(fmtstr,arg)		if (debugn) fprintf (stderr, fmtstr, arg)
#define DP(errstr)		perror (errstr)

/*
 * Forward Declarations:
 */
#if !defined(_NO_PROTO_)

extern char *interp(char *s);
extern int typeahead(int masker, int secns, int usecns);

int gen_dialer(char *num, char *acu);
int flush_rw(int f);
int gendial(char *num);
int genoutput(char *str, int delay);
int geninput(char *str, char *rspstr);
int cindex(char *s, char *t);
int usleep(int secdelay, int usecdelay);

int DisableModemControl(int fd);
int EnableModemControl(int fd);
unsigned int GetModemSignals(int fd);
int HangupModem(int fd);
int ShowModemSignals(int fd);
int WaitForCarrier(int fd);

#endif /* !defined(_NO_PROTO_) */

static  int genALRM();
static  int timerALRM();
static int timedout;
static  jmp_buf timeoutbuf, alrmbuf;
extern 	int speed();
char *syncstr, *respstr, *dialstr, *respdial, *abostr, *onlstr,
		*disconstr, charup, *compstr, hangup, dtr, stupidi,
		*speedstr, *respsync, *dialterm, lsleep, debugn, *replacestr,
		dtr_delay;
int syncdel, dialdel, cdelay, compdel, dialack, dspeed, cspeed;
int FDD = -1;  
int generrno;
struct termio tty_termio;

gen_dialer(num, acu)
	register char *num;
	char *acu;
{
        char junk[GENBUF];
	char errd = 0;
	int fread = FREAD;
	int zero = 0;

	generrno = timedout = 0;

	if(debugn) {
	    P ("--------------------------------------------------\n");
	    P1("   Generic dialer modem type (at): %s\n", acu);
	    P1("                Phone number (pn): %s\n", num);
	    P1("      Modem initialize speed (is): %d\n", dspeed);
	    P1("    Modem conversation speed (xs): %d\n", cspeed);
	    P1(" Switch speed command string (co): %s\n", interp(speedstr));
	    P ("--------------------------------------------------\n");
	    P1("    Wait for carrier up flag (cr): %d\n", charup);
	    P1("                 Hangup flag (hu): %d\n", hangup);
	    P1("              Reset DTR flag (re): %d\n", dtr);
	    P ("--------------------------------------------------\n");
	    P1("      Synchronization string (ss): %s\n", interp(syncstr));
	    P1("             Response string (sr): %s\n", interp(respsync));
	    P1("                 Dial string (di): %s\n", interp(dialstr));
	    P1("      Dial terminator string (dt): %s\n", interp(dialterm));
	    P1("      Dial acknowledge delay (da): %d\n", dialack);
	    P1("        Dial response string (dr): %s\n", interp(respdial));
	    P1("  Disconnect sequence string (ds): %s\n", interp(disconstr));
	    P1("         Modem online string (os): %s\n", interp(onlstr));
	    P1("           Completion string (cs): %s\n", interp(compstr));
	    P1("          Replacement string (rs): %s\n", interp(replacestr));
	    P1("          Modem abort string (ab): %s\n", interp(abostr));
	    P ("--------------------------------------------------\n");
	    P1("            Local sleep flag (ls): %d\n", lsleep);
	    P1("Ignore response strings flag (si): %d\n", stupidi);
	    P1("        Synchonization delay (sd): %d\n", syncdel);
	    P1("            Dial delay value (dd): %d\n", dialdel);
	    P1("      Completion delay value (cd): %d\n", compdel);
	    P1("  Full (carrier) delay value (fd): %d\n", cdelay);
	    P1(" Delay after DTR toggle flag (rd): %d\n", dtr_delay);
	    P ("--------------------------------------------------\n");
	    (void) ShowModemSignals(FDD);
	}

	(void) DisableModemControl(FDD);

	/*
	 * Get in sync
	 */
	if (!gensync()) 
		errd = NOSYNC;

	/*
	 * If boolean hangup then we will hangup on any problems
	 */

	if (hangup) 
		if (ioctl(FDD, TIOCHPCL, 0) < 0) DP("TIOCHPCL");
	else {
		/* don't have any way to undo it yet... */
	}
		
	/*
	 * All generic dialers have to do some type of dialing!
	 */

	if (!errd) {
		if (!gendial(num)) {
			gen_disconnect();
			errd = BADDIAL;
		}
		/* Flush the input queue (not the output queue!) */
		if (ioctl(FDD, TIOCFLUSH, &fread) < 0) DP("TIOCFLUSH");
	}

	/*
	 * online waits some defined time for the carrier
	 * to come up. If it doesnt the remote system might be down
	 */

	if (!errd) {
	   if (!online()) {
		gen_disconnect();
		errd = NOCAR;
	   }
	}


	/*
	 * Finish up sequence (if any) 
	 */

	if (!errd) {
	   if ((*compstr) && (!geninput(&junk[0], compstr))) {
	       errd = NOSYNC;
	   }
	}
	   
	if (timedout || errd) {
		gen_disconnect();       /* insurance */
		generrno = errd;
		return (0);
	} else {
		if (debugn) (void) ShowModemSignals(FDD);
	}

	generrno = 0;
	return (1);
}

gen_disconnect()
{
	D("\r\ngendisconnect()\r\n");
	if (FDD < 0) return;
	(void) DisableModemControl(FDD);
	if(*disconstr)  {
		sleep(DEFDELAY);
		genoutput(disconstr,dialdel);
		sleep(DEFDELAY);
	}
	if (debugn) (void) ShowModemSignals(FDD);
	flush_rw(FDD);
}

gen_abort()
{
	D("genabort()\r\n");
	if (FDD < 0) return;
	(void) DisableModemControl(FDD);
	if(*abostr) {
		genoutput(abostr,dialdel);
	}
	flush_rw(FDD);
}

flush_rw(f)
int f;
{
	int rw = FREAD | FWRITE;
	if (ioctl(f, TIOCFLUSH, &rw) < 0) DP("TIOCFLUSH");
}

static int
genALRM()
{
	timedout = 1;
	longjmp(timeoutbuf, 1);
}

gendial(num)
char *num;
{
	char obuf[GENBUF];
	char respbuf[GENBUF];
	int ret=0;

	D1("gendial(char *num = %s)\n", num);
	if (*dialstr) {
		if(*replacestr) {
			char *p1 = num;
			while(*p1) {
				if(*p1 == '=' || *p1 == '-')
					*p1 = *replacestr;
				p1++;
			}
		}
		obuf[0]='\0';
		strcat(obuf, dialstr);
		strcat(obuf, num); 
		if (*dialterm)
			strcat(obuf, dialterm);
		if(!genoutput(obuf, dialdel))
			return(ret);
		else
			ret=1;
	}

	if (dialack > 0) {
	    if (lsleep) {
		usleep (0, dialack);
	    } else {
		sleep (dialack);
	    }
	}

	if ((respdial) && (*respdial)) {
		if(!geninput(&respbuf[0],respdial)) {
			return(0);
		}
		return (1);
	}
	return(ret);
}

online() {

	if (setjmp(timeoutbuf)) {
		D("online failed\n");
		return(0);
	}

	signal(SIGALRM, genALRM);
	if (charup) {                   /* Conditionally wait for carrier */
		D("waiting for carrier...\n");
		alarm(cdelay);
		/*
		 * Suspend, waiting for carrier signal (CD).
		 */
		(void) WaitForCarrier(FDD);
		alarm(0);
		D("carrier detected\n");
		EnableModemControl(FDD);
	}
	if (debugn) (void) ShowModemSignals(FDD);

	/*
	 * Reset alarm to ensure we don't hang on the read.
	 */
	alarm(cdelay);
	if(*onlstr) {
		char junk[GENBUF];
		if(!geninput(&junk[0], onlstr)) {
			D("online failed\n");
			return(0);
		}
	}
	alarm(0);
	signal(SIGALRM, SIG_DFL);
	D("online succeeded\n");
	return(1);
}

static int
gensync()
{
	char respbuf[GENBUF];
	int cbaud, dbaud;
	struct sgttyb sgtty_struct;
	int ret=0;

	D("gensync()\n");

	/*
	 * Toggle DTR to reset the modem.
	 */
	if (dtr) {
		if (ioctl(FDD, TIOCCDTR, 0) < 0) DP("TIOCCDTR");
		sleep(2);
		if (ioctl(FDD, TIOCSDTR, 0) < 0) DP("TIOCSDTR");
		/*
		 * Give the modem a second to respond to the reset.
		 */
		if (dtr_delay)
			sleep(1);
	} 

	cbaud = speed(cspeed);
	dbaud = speed(dspeed);
	/*
	 * Initialize modem at this speed.
	 */
	if (dbaud != NULL) {	
	    if (ioctl(FDD, TIOCGETP, &sgtty_struct) < 0) {
		DP("TIOCGETP");
	    } else {
		sgtty_struct.sg_ispeed = sgtty_struct.sg_ospeed = dbaud;
		D1("initializing modem at %d baud\n", dspeed);
		if (ioctl(FDD, TIOCSETP, &sgtty_struct) < 0) DP("TIOCSETP");
	    }
	}
	if(*syncstr) {
		if(!genoutput(syncstr,syncdel))
			return(0);
		else
			ret = 1;
	}
	if(*respsync) {
		if(!geninput(&respbuf[0], respsync))
			return(0);
		else
			ret = 1;
	}
	/*
	 * Change modem speed after sending out a string which tells the
	 * modem what speed to use.
	 */
	if (cbaud != NULL) {
	    if (*speedstr) {
		D1("Modem speed string=%s", speedstr);
		genoutput(speedstr, syncdel);
		geninput(&respbuf[0], respsync);
	    }
	    sleep(2);
	    D1("dialing modem at %d baud\n", cspeed);
	    if (ioctl(FDD, TIOCGETP, &sgtty_struct) < 0) {
		DP("TIOCGETP");
	    } else {
		sgtty_struct.sg_ispeed = sgtty_struct.sg_ospeed = cbaud; 
		if (ioctl(FDD, TIOCSETP, &sgtty_struct) < 0) DP("TIOCSETP");
	    }
	    sleep(2);
	    genoutput(syncstr, syncdel);
	    geninput(&respbuf[0], respsync);
	    return(1);
	}
	return(ret);
}

genoutput(str, delay)
char *str;
int delay;
{
	char c;

	D1("genoutput: %s\n", interp(str));

	while (c = *str++) {
		/* a 'd' with the high bit set means sleep one sec (\d) */
		if ((c&0xff) == (('d'|0x80)&0xff)) { 
			D(" DELAY ");
			sleep(1);
			continue;
		}
		c = (c & 0x7f); 
		D1(" %x\n", c);
		if (write(FDD, &c, 1) != 1) {
			DP("write failed");
			return(0);
		}
		if(lsleep)	
			usleep(0, delay);
		else
			sleep(delay); 
	}
	D("\n");
	return(1);
}

/* Input eats things in typeahead upto & including the rspstr */
geninput(str, rspstr)
char *str, *rspstr;
{
	char c, *strb = str;
	register int strsiz = GENBUF;
	register int tp;

	D1("geninput: waiting for -> %s\n", interp(rspstr));

	*str = '\0'; /* null terminate string */

	/* Calling an interface stupid brings us back in a hurry */
	if(stupidi)
		return(1);

	/*
	 * Make initial timeout long, since the input buffer queue
	 * was flushed after sending the phone number to dial.
	 */
	tp = typeahead((1<<FDD), 60, 0);
	while (tp && (--strsiz > 0)) {  /* Eats everything in typeahead */
		if (read(FDD, &c, 1) != 1) {
			DP("read failed");
			return(0);
		}
		*str++ = c&0177;
		*str = '\0';
		if(cindex(strb, rspstr) >= 0) {
			D1("geninput: %s\n", interp(strb));
			return(1);
		}
		tp = typeahead((1<<FDD), 1, 0);
	}
	D ("geninput (failed)\n");
	D1("  expected response -> %s\n", interp(rspstr));
	D1("  response returned -> %s\n", interp(strb));
	return(0);
}

cindex(s, t)
char *s, *t;
{
	int i, j, k;

	for (i=0; s[i] != '\0'; i++) {
		for (j=i, k=0; t[k] != '\0' && s[j] == t[k]; j++, k++)
			;
		if(t[k] == '\0')
			return(i);
	}
	return(-1);
}

static int
timerALRM()
{
	longjmp(alrmbuf, 1);
}

usleep(secdelay, usecdelay)
int secdelay, usecdelay;
{
	struct itimerval val, oval;

	if(setjmp(alrmbuf)) {
		signal(SIGALRM, SIG_DFL);
		return(1);
	}
	oval.it_interval.tv_sec = 0;
	oval.it_interval.tv_usec = 0;
	oval.it_value.tv_sec = 0;
	oval.it_value.tv_usec = 0;
	val.it_interval.tv_sec = 0;
	val.it_interval.tv_usec = 0;
	/* make sure microseconds is less than 1 second */
	if (usecdelay >= 1000000) {
	    secdelay += usecdelay / 1000000;
	    usecdelay %= 1000000;
	}
	val.it_value.tv_sec = secdelay;
	val.it_value.tv_usec = usecdelay;
	signal(SIGALRM, timerALRM);
	setitimer(ITIMER_REAL, &val, &oval);
	pause();
	return(0);
}

#if 0
/*
 * Baud rate mapping table
 */
int bauds[] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600,
	1200, 1800, 2400, 4800, 9600, 19200, 38400, -1
};
speed(n)
	int n;
{
	register int *p;

	for (p = bauds; *p != -1;  p++)
		if (*p == n)
			return (p - bauds);
	return (NULL);
}
#endif

static int
DisableModemControl(fd)
int fd;
{
	int status = GEN_SUCCESS;

	/*
	 * Set CLOCAL so we can talk to the modem.
	 */
	if (ioctl(fd, TCGETA, &tty_termio) < 0) {
	    DP("TCGETA");
	    status = GEN_FAILURE;
	} else if ((tty_termio.c_cflag & CLOCAL) == 0) {
	    tty_termio.c_cflag |= CLOCAL;
	    if (ioctl(fd, TCSETA, &tty_termio) < 0) {
		DP("TCSETA");
		status = GEN_FAILURE;
	    }
	}
	return (status);
}

static int
EnableModemControl(fd)
int fd;
{
	int status = GEN_SUCCESS;

	/*
	 * Reset CLOCAL so terminal driver senses modem signals.
	 */
	if (ioctl(fd, TCGETA, &tty_termio) < 0) {
	    DP("TCGETA");
	    status = GEN_FAILURE;
	} else {
	    tty_termio.c_cflag &= ~CLOCAL;
	    if (ioctl(fd, TCSETA, &tty_termio) < 0) {
		DP("TCSETA");
		status = GEN_FAILURE;
	    }
	}
	return (status);
}

unsigned int
GetModemSignals(fd)
int fd;
{
	unsigned int msigs;

	if (ioctl(fd, TIOCMGET, &msigs) < 0) {
		DP("TIOCGET");
		return(GEN_FAILURE);
	}
	return (msigs);
}

int
HangupModem(fd)
int fd;
{
	int status;
	unsigned int delay = 3;

	if ((status = ioctl(fd, TIOCCDTR, 0)) < 0) DP("TIOCCDTR");
	sleep(delay);
	if ((status = ioctl(fd, TIOCSDTR, 0)) < 0) DP("TIOCSDTR");
	sleep(1);
	return (status);
}

int
ShowModemSignals(fd)
int fd;
{
	unsigned int msigs;

	if ( (msigs = GetModemSignals(fd)) == GEN_FAILURE) {
		return(GEN_FAILURE);
	}
	P ("--------------------------------------------------\r\n");
	P1("Modem Signals Set: 0x%x\r\n", msigs);
	if (msigs & TIOCM_LE) {
	  P1("   0x%x = TIOCM_LE = Line Enable.\r\n", TIOCM_LE);
	}
	if (msigs & TIOCM_DTR) {
	  P1("   0x%x = TIOCM_DTR = Data Terminal Ready.\r\n", TIOCM_DTR);
	}
	if (msigs & TIOCM_RTS) {
	  P1("   0x%x = TIOCM_RTS = Request To Send.\r\n", TIOCM_RTS);
	}
	if (msigs & TIOCM_ST) {
	  P1("   0x%x = TIOCM_ST = Secondary Transmit.\r\n", TIOCM_ST);
	}
	if (msigs & TIOCM_SR) {
	  P1("   0x%x = TIOCM_SR = Secondary Receive.\r\n", TIOCM_SR);
	}
	if (msigs & TIOCM_CTS) {
	  P1("   0x%x = TIOCM_CTS = Clear To Send.\r\n", TIOCM_CTS);
	}
	if (msigs & TIOCM_CAR) {
	  P1("   0x%x = TIOCM_CAR = Carrier Detect.\r\n", TIOCM_CAR);
	}
	if (msigs & TIOCM_RNG) {
	  P1("   0x%x = TIOCM_RNG = Ring Indicator.\r\n", TIOCM_RNG);
	}
	if (msigs & TIOCM_DSR) {
	  P1("   0x%x = TIOCM_DSR = Data Set Ready.\r\n", TIOCM_DSR);
	}
	P ("--------------------------------------------------\r\n");
	return(GEN_SUCCESS);
}

int
WaitForCarrier(fd)
int fd;
{
	unsigned int msigs;
	unsigned int delay = 1;

	do {
	    if ( (msigs = GetModemSignals(fd)) == GEN_FAILURE) {
		return(GEN_FALSE);
	    }
	    sleep (delay);
	} while ( (msigs & (TIOCM_CAR|TIOCM_DSR)) == 0);
	return (GEN_TRUE);
}
