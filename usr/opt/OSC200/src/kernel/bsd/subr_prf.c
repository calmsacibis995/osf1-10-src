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
static char *rcsid = "@(#)$RCSfile: subr_prf.c,v $ $Revision: 4.4.31.3 $ (DEC) $Date: 1993/05/27 18:59:47 $";
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <cpus.h>
#include <varargs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/reboot.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/syslog.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <kern/lock.h>
#include <kern/parallel.h>
#include <machine/cpu.h>	/* for cpu_number() */
#include <machine/xpr.h>	/* reg_desc for %r/%R */
#include <sys/dk.h>		/* for SAR counters */


int printstate = PROMPRINT;		/* console state flags */
int prfoverflow;			/* count of prf overflows */
char *prfbuftab[NCPUS][8];		/* xcpu printf request pointers */

extern int log_open;			/* whether /dev/klog is open */
extern int con_open;			/* whether /dev/kcon is open */
extern struct vnode *consvp;		/* vnode for redirected console */

char *panicstr;				/* pointer to panic string */
int paniccpu;				/* id of cpu that called panic */
int binlogpanic;			/* panic message flag for binlog */
decl_simple_lock_data(,panic_lock)

extern void log_puts(), xcpu_puts(), cons_puts(), prom_puts();


/*
 * Following is a summary of all of the current kernel printf interfaces:
 *
 *	printf	-- bootup, shutdown, and debug output to console
 *	aprintf	-- informational output to console (via syslogd)
 *	dprintf	-- emergency output to console (via console callback)
 *	sprintf	-- formatting of strings into caller-specified buffer
 *	sprintf2 - like sprintf but with 2nd arg for overflow check
 *	uprintf	-- error message to user's terminal (via VOP_WRITE)
 *	log	-- error message logging facility (via syslogd)
 *	putchar	-- single-character output utilizing printf
 *
 * All of the routines above can be called at any interrupt level without
 * the risk of blocking EXCEPT for uprintf(), which requires a process
 * context.  The others all do either polled i/o to the console (freezing
 * cpu execution and possibly losing clock ticks) or else put the formatted
 * message in the console buffer pointed to by "pconbuf" for user level i/o
 * to /dev/console by the syslog daemon (syslogd).
 *
 * The formatting of strings is now isolated from the output of the
 * characters.  The prf() and printn() routines now take pointers to
 * a buffer and its end and return the number of characters formatted.
 * All of the printf interface routines are responsible for providing
 * the buffer and taking appropriate action after prf() finishes its
 * formatting.  Except for sprintf() and sprintf2(), the buffers are
 * located on a kernel stack and are restricted to PRFMAX characters
 * in size.
 */

#define PRFMAX 128


/*
 * Format and display diagnostic information on the system console.  This
 * routine should only be used for bootup messages, emergency/panic messages,
 * or for debugging.  It should not be used for chit-chat (see aprintf below).
 * The characters are also put in the message buffer for syslogd and for crash
 * dumps.
 */
/*VARARGS1*/
void
printf(va_alist)	/* fmt, va_alist */
	va_dcl
{
	va_list valist;
	int n, s, ps, cpu;
	u_char buf[PRFMAX];

	va_start(valist);
	n = prf(&buf[0], &buf[PRFMAX], valist);
	va_end(valist);
	if (n <= 0)
		return;

	s = splextreme();
	ps = printstate;
	if (consvp && con_open && (ps & MEMPRINT) && !(ps & PANICPRINT)) {
		log_puts(&buf[0], 1);
		splx(s);
	} else {
		if (ps & MEMPRINT)
			log_puts(&buf[0], 0);
		if (s == SPLNONE)
			spl1();
		else
			splx(s);
		if ((cpu = cpu_number()) != master_cpu)
			xcpu_puts(&buf[0], &prfbuftab[cpu][s]);
		else if (ps & CONSPRINT)
			cons_puts(&buf[0]);
		else if (ps & PROMPRINT)
			prom_puts(&buf[0]);
		if (s == SPLNONE)
			spl0();
	}
	if ((ps & MEMPRINT) && !panicstr)
		logwakeup();
}

/*
 * Display a single character on the system console according to the
 * same rules as printf().  This is not a good idea anymore, since no
 * assurance can be made that multiple characters of a message will
 * stay together.  Using sprintf() to assemble a whole line is now
 * recommended.
 */
void
putchar(c)
	u_char c;
{
	if (c)
		printf("%c", c);
}

/*
 * Format and display an informational message on the system console
 * asynchronously if possible.  This is recommended for non-emergency
 * messages deserving the administrator's/operator's attention, but
 * which are not indicative of a kernel problem.  It normally uses
 * "syslogd" to perform user-level console i/o.  The characters are
 * also put in the message buffer.
 */
/*VARARGS1*/
void
aprintf(va_alist)	/* fmt, va_alist */
	va_dcl
{
	va_list valist;
	int n, s, ps, cpu;
	u_char buf[PRFMAX];

	va_start(valist);
	n = prf(&buf[0], &buf[PRFMAX], valist);
	va_end(valist);
	if (n <= 0)
		return;

	s = splextreme();
	ps = printstate;
	if (con_open && (ps & MEMPRINT) && !(ps & PANICPRINT)) {
		log_puts(&buf[0], 1);
		splx(s);
	} else {
		if (ps & MEMPRINT)
			log_puts(&buf[0], 0);
		if (s == SPLNONE)
			spl1();
		else
			splx(s);
		if ((cpu = cpu_number()) != master_cpu)
			xcpu_puts(&buf[0], &prfbuftab[cpu][s]);
		else if (ps & CONSPRINT)
			cons_puts(&buf[0]);
		else if (ps & PROMPRINT)
			prom_puts(&buf[0]);
		if (s == SPLNONE)
			spl0();
	}
	if ((ps & MEMPRINT) && !panicstr)
		logwakeup();
}

/*
 * Format and display a message on the system console as reliably as
 * possible, with preference given to the console callback mechanism.
 */
/*VARARGS1*/
void
dprintf(va_alist)	/* fmt, va_alist */
	va_dcl
{
	va_list valist;
	int n, s, ps, cpu;
	u_char buf[PRFMAX];

	va_start(valist);
	n = prf(&buf[0], &buf[PRFMAX], valist);
	va_end(valist);
	if (n <= 0)
		return;

	if ((s = getspl()) == SPLNONE)
		spl1();
	ps = printstate;
	if ((cpu = cpu_number()) != master_cpu)
		xcpu_puts(&buf[0], &prfbuftab[cpu][s]);
	else if (ps & PROMPRINT)
		prom_puts(&buf[0]);
	else if (ps & CONSPRINT)
		cons_puts(&buf[0]);
	if (s == SPLNONE)
		spl0();
}

/*
 * Format a message into the buffer specified by the caller.  No buffer
 * overflow checking is performed.  The string is null-terminated.  The
 * number of characters put into the buffer not including the terminating
 * null character is provided as the return value.
 */
/*VARARGS2*/
int
sprintf(va_alist)	/* buf, fmt, va_alist */
	va_dcl
{
	va_list valist;
	u_char *buf;
	int n;

	va_start(valist);
	buf = va_arg(valist, u_char *);
	n = prf(buf, NULL, valist);
	va_end(valist);

	return(n);
}

/*
 * Format a message into the buffer specified by the caller the same
 * as sprintf(), but with overflow checking performed according to an
 * additional 2nd argument.
 */
/*VARARGS3*/
int
sprintf2(va_alist)	/* buf, end, fmt, va_alist */
	va_dcl
{
	va_list valist;
	u_char *buf, *end;
	int n;

	va_start(valist);
	buf = va_arg(valist, u_char *);
	end = va_arg(valist, u_char *);
	n = prf(buf, end, valist);
	va_end(valist);

	return(n);
}

/*
 * Format a message and write it to the vnode for the controlling terminal
 * of the currently running user process.  This routine requires a process
 * context, and it may block if the tty queue is above the high water mark.
 */
/*VARARGS1*/
int
uprintf(va_alist)	/* fmt, va_alist */
	va_dcl
{
	va_list valist;
	int n, error;
	struct proc *pp;
	struct pgrp *gp;
	struct session *sp;
	struct vnode *vp;
	struct uio uio;
	struct iovec iov;
	u_char buf[PRFMAX];

	va_start(valist);
	n = prf(&buf[0], &buf[PRFMAX], valist);
	va_end(valist);
	if (n <= 0)
		return(0);

	unix_master();
	if ((pp = u.u_procp) &&
	    (pp->p_flag & SCTTY) &&
	    (gp = pp->p_pgrp) &&
	    (sp = gp->pg_session) &&
	    (vp = sp->s_ttyvp) &&
	    vp->v_type == VCHR) {
		VREF(vp);
		iov.iov_base = (caddr_t)&buf[0];
		iov.iov_len = n;
		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = 0;
		uio.uio_resid = n;
		uio.uio_segflg = UIO_SYSSPACE;
		uio.uio_rw = UIO_WRITE;
		VOP_WRITE(vp, &uio, IO_APPEND, NOCRED, error);
		VUNREF(vp);
		unix_release();
		return(error);
	}
	unix_release();
	return(KERN_FAILURE);
}

/*
 * Format a message and send it to the log.  If syslogd is not running
 * (or if the priority level is LOG_PRINTF), then the message is sent
 * to the console as well (or instead).
 */
/*VARARGS2*/
void
log(va_alist)		/* level, fmt, va_alist */
	va_dcl
{
	va_list valist;
	int level, n, m, s, ps, cpu;
	u_char buf[PRFMAX];

	n = 0;
	va_start(valist);
	level = va_arg(valist, int);
	if (level != LOG_PRINTF) {
		buf[n++] = '<';
		n += printn(&buf[1], &buf[PRFMAX], (u_long)level, 10L, 0, 0);
		buf[n++] = '>';
	}
	m = n;
	n += prf(&buf[n], &buf[PRFMAX], valist);
	va_end(valist);
	if (n <= 0)
		return;

	s = splextreme();
	ps = printstate;
	if (m == 0 && con_open && (ps & MEMPRINT) && !(ps & PANICPRINT)) {
		log_puts(&buf[0], 1);
		splx(s);
	} else {
		if (ps & MEMPRINT)
			log_puts(&buf[0], 0);
		if (m == 0 || !log_open || !(ps & MEMPRINT)) {
			if (s == SPLNONE)
				spl1();
			else
				splx(s);
			if ((cpu = cpu_number()) != master_cpu)
				xcpu_puts(&buf[m], &prfbuftab[cpu][s]);
			else if (ps & CONSPRINT)
				cons_puts(&buf[m]);
			else if (ps & PROMPRINT)
				prom_puts(&buf[m]);
			if (s == SPLNONE)
				spl0();
		} else
			splx(s);
	}
	if ((ps & MEMPRINT) && !panicstr)
		logwakeup();
}

/*
 * Warn that a system table is full.
 */
void
tablefull(tab)
	char *tab;
{
	log(LOG_ERR, "%s table is full\n", tab);

        /*
         * If logging error for proc, file, or vnode bump up overflow counter.
	 */
        if (!strncmp(tab,"proc", 4))
                tbl_proc_ov++;
	else if (!strncmp(tab,"file", 4))
                tbl_file_ov++;
	else if (!strncmp(tab,"vnode", 5))
                tbl_inod_ov++;
}

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then reboots.
 * If we are called twice, then we avoid trying to
 * sync the disks as this often leads to recursive panics.
 */
int
panic(s)
	char *s;
{
	int bootopt;

	simple_lock(&panic_lock);
	if (!panicstr) {
		panicstr = s;
		paniccpu = cpu_number();
		printstate |= PANICPRINT;
		bootopt = RB_AUTOBOOT;
	} else if (cpu_number() == paniccpu) {
		bootopt = RB_AUTOBOOT | RB_NOSYNC;
	} else {
		simple_unlock(&panic_lock);
		halt_cpu();
		/* NOTREACHED */
	}

        /*
	 * To capture the panic string in the binary event logger, we need
	 * to know other than via "panicstr" that the next printf should
	 * result in a binlog entry.  Setting the following global flag
	 * causes log_puts() to invoke binlog_logmsg() for a panic message.
	 */
	binlogpanic = 1;

	simple_unlock(&panic_lock);
#if	NCPUS > 1
	printf("panic (cpu %d): %s\n", paniccpu, s);
#else	
	printf("panic: %s\n", s);
#endif	

	/*
	 * After saving the current context and attempting to generate a
	 * crash dump, boot() will terminate execution of the kernel, and
	 * the system will halt or reboot appropriately.
	 */
	boot(RB_PANIC, bootopt);
	/* NOTREACHED */

	/*
	 * Since smart RISC compilers avoid saving the value for "s" in
	 * the typical case, dbx tracebacks of crash dumps display bogus
	 * panic strings.  The following line of code is never executed,
	 * since boot() doesn't return, but it's existence forces "s" to
	 * be saved above, thus making tracebacks more intelligible.
	 */
	return(s != (char *)0);
}

void
panic_init()
{
	simple_lock_init(&panic_lock);
}

/*
 * Send a null-terminated string to the console driver.
 */
void
cons_puts(s)
	register u_char *s;
{
	u_int c;

	while (c = *s++)
		cnputc(c);
}

/*
 * Send a null-terminated string to the master cpu.
 */
void
xcpu_puts(s, prfbufp)
	u_char *s;
	volatile u_char **prfbufp;
{
#if NCPUS > 1
	mb();
	*prfbufp = s;
	prfrequest = 1;
	mb();
	interrupt_master_cpu();
	while (*prfbufp)
		spin_wait();
#endif
}

/*
 * Handle a remote printf request from a slave cpu.
 */
void
xcpu_puts_ipir()
{
}


/*
 * The following flags in the 6th printf() argument affect the formatting.
 */
#define LEFT	1	/* left-justify within field */
#define ZERO	2	/* left-fill with leading zeros */
#define SIGN	4	/* use minus sign for negative values */
#define FORM	8	/* use alternate form (0x/0 prefixes for hex/octal) */

/*
 * The following macro is only for use within prf() and printn().
 * Note that it references the local variables "bufptr" and "bufend",
 * and that its argument might not be evaluated (and thus should not
 * invoke side effects).
 */
#define PUTC(c) if (!bufend || bufptr < bufend) *bufptr++ = (c); else

/*
 * Format strings of characters for printf() and its counterparts
 * {c,d,s,u}printf() and log().  Strings are always null-terminated.
 * The return value is the number of characters generated not including
 * the terminating null character.
 *
 * Formats supported:
 *
 *	%x/%X	unsigned hex (optional field width and 'l' for long)
 *	%d/%D	signed decimal (optional field width and 'l' for long)
 *	%u/%U	unsigned decimal (optional field width and 'l' for long)
 *	%o/%O	unsigned octal (optional field width and 'l' for long)
 *	%p/%P	pointers in hexadecimal (%P generates 0x prefix)
 *	%s	null-terminated strings (optional field width)
 *	%c/%C	characters masked by 0x7f/0xff
 *	%b	bitfield decoding (see info below)
 *	%n/%N	symbolic names without/with value
 *	%r/%R	register descriptor (see machine/xpr.h)
 *	%%	prints the percent character itself
 *
 * Usage of %b is:
 *
 *	printf("reg=%b\n", regval, "<base><arg>*");
 *
 * where <base> is the output base expressed as a control character,
 * e.g. \10 gives octal; \20 gives hex.  Each arg is a sequence of
 * characters, the first of which gives the bit number to be inspected
 * (origin 1), and the next characters (up to a control character, i.e.
 * a character <= 32), give the name of the bit.  Thus
 *
 *	printf("reg=%b\n", 3, "\10\2BITTWO\1BITONE\n");
 *
 * would produce output:
 *
 *	reg=3<BITTWO,BITONE>
 *
 * To accomodate 64 bits we use the ASCII range 129 - 192 (0201-0300),
 * inclusive.  The range 1-32 will also work up to 32 bits.  Bit fields
 * can be identified by two successive bit number codes before the name.
 */
int
prf(bufptr, bufend, va)
	u_char *bufptr;
	u_char *bufend;
	va_list va;
{
	register u_char *fmt, *s;
	register u_long c, n, t;
	int longdata, width, flags;
	u_char *orig_bufptr;
	struct reg_desc *rd;
	struct reg_values *rv;

	fmt = va_arg(va, u_char *);
	if (!fmt)
		return(0);

	orig_bufptr = bufptr;
loop:
	while ((c = *fmt++) != '%') {
		if (c) {
			PUTC(c);
			continue;
		}
		if (!bufend || bufptr < bufend) {
			*bufptr = '\0';
			return(bufptr - orig_bufptr);
		}
		prfoverflow++;
		*--bufend = '\0';
		return(bufend - orig_bufptr);
	}

	longdata = 0;
	width = 0;
	flags = 0;
	n = 1;
	while (n && (c = *fmt++)) {
		switch (c) {
		case 'l':
		case 'L':
			longdata = 1;
			break;
		case '-':
			flags |= LEFT;
			break;
		case '#':
			flags |= FORM;
			break;
		case '0':
			if (width)
				width *= 10;
			else
				flags |= ZERO;
			break;
		default:
			if (c >= '1' && c <= '9')
				width = 10 * width + c - '0';
			else
				n = 0; /* terminates option loop */
			break;
		}
	}

	switch (c) {
	case 'X':
	case 'x':
		if (longdata)
			n = (u_long)va_arg(va, u_long);
		else
			n = (u_long)va_arg(va, u_int);
		bufptr += printn(bufptr, bufend, n, 16L, width, flags);
		break;
	case 'D':
	case 'd':
		if (longdata)
			n = (u_long)va_arg(va, long);
		else
			n = (u_long)va_arg(va, int);
		bufptr += printn(bufptr, bufend, n, 10L, width, flags | SIGN);
		break;
	case 'U':
	case 'u':
		if (longdata)
			n = (u_long)va_arg(va, u_long);
		else
			n = (u_long)va_arg(va, u_int);
		bufptr += printn(bufptr, bufend, n, 10L, width, flags);
		break;
	case 'O':
	case 'o':
		if (longdata)
			n = (u_long)va_arg(va, u_long);
		else
			n = (u_long)va_arg(va, u_int);
		bufptr += printn(bufptr, bufend, n, 8L, width, flags);
		break;
	case 'P':
		flags |= FORM;
		/* fall through */
	case 'p':
		n = (u_long)va_arg(va, char *);
		bufptr += printn(bufptr, bufend, n, 16L, width, flags);
		break;
	case 's':
		s = va_arg(va, u_char *);
		if (!s)
			s = (u_char *)"<NULL>";
		if (width)
			width -= strlen(s);
		if (width > 0 && !(flags & LEFT)) {
			do {
				PUTC(' ');
			} while (--width > 0);
		}
		while (c = *s++)
			PUTC(c);
		if (width > 0) {
			do {
				PUTC(' ');
			} while (--width > 0);
		}
		break;
	case 'C':
	case 'c':
		t = (c == 'C') ? 0xffL : 0x7fL;
		if (longdata) {
			n = (u_long)va_arg(va, u_long);
			width = sizeof(u_long);
		} else {
			n = (u_long)va_arg(va, u_int);
			width = sizeof(u_int);
		}
#if BYTE_ORDER == LITTLE_ENDIAN
		while (width-- > 0) {
			if (c = (n & t))
				PUTC(c);
			n >>= NBBY;
		}
#endif
#if BYTE_ORDER == BIG_ENDIAN
		if (c = (n & t))
			PUTC(c);
#endif
		break;
	case 'b':
		n = va_arg(va, u_long);
		s = va_arg(va, u_char *);
		t = *s++;
		if (t == 10L)
			flags |= SIGN;
		bufptr += printn(bufptr, bufend, n, t, width, flags);
		if (!n)
			break;
		width = t; /* save radix value in case of bit fields */
		PUTC('<');
		longdata = 0;
		while ((t = *s++) && (c = *s)) {
			/*
			 * To accomodate 64 bits we use the ASCII
			 * range 129 - 192 (0201-0300), inclusive.
			 */
			if (--t >= 128)
				t -= 128;
			if (c <= 32 || (c >= 129 && c <= 192)) {
				if (--c > 128)
					c -= 128;
				if (t = ((n >> c) & ((2L << (t - c)) - 1L))) {
					if (longdata++)
						PUTC(',');
					while ((c = *++s) > 32 && c < 129)
						PUTC(c);
					PUTC('=');
					bufptr += printn(bufptr, bufend, t,
						(u_long)width, 0, flags);
				} else {
					while ((c = *++s) > 32 && c < 129)
						;
				}
			} else {
				if (n & (1L << t)) {
					if (longdata++)
						PUTC(',');
					PUTC(c);
					while ((c = *++s) > 32 && c < 129)
						PUTC(c);
				} else {
					while ((c = *++s) > 32 && c < 129)
						;
				}
			}
		}
		PUTC('>');
		break;
	case 'n':
	case 'N':
		n = va_arg(va, u_long);
		rv = va_arg(va, struct reg_values *);
		for (s = (u_char *)"???"; rv->rv_name; rv++) {
			if (n == rv->rv_value) {
				s = (u_char *)rv->rv_name;
				break;
			}
		}
		while (c = *s++)
			PUTC(c);
		if (c == 'N' || !rv->rv_name) {
			PUTC(':');
			bufptr += printn(bufptr, bufend, n, 10L, width,
				flags | SIGN);
		}
		break;
	case 'r':
	case 'R':
		n = va_arg(va, u_long);
		rd = va_arg(va, struct reg_desc *);
		if (c == 'R') {
			bufptr += printn(bufptr, bufend, n, 16L, width,
				flags | FORM);
			if (!n)
				break;
		}
		PUTC('<');
		for (longdata = 0; rd->rd_mask; rd++) {
			t = (rd->rd_shift < 0)
				? (n & rd->rd_mask) >> -rd->rd_shift
				: (n & rd->rd_mask) << rd->rd_shift;
			if (!rd->rd_format && !rd->rd_values &&
			    (!rd->rd_name || !t))
				continue;
			if (longdata++)
				PUTC(',');
			if (rd->rd_name) {
				if (rd->rd_format || rd->rd_values || t) {
					s = (u_char *)rd->rd_name;
					while (c = *s++)
						PUTC(c);
				}
				if (rd->rd_format || rd->rd_values)
					PUTC('=');
			}
			if (rd->rd_format) {
				bufptr += sprintf2(bufptr, bufend,
					rd->rd_format, t);
				if (rd->rd_values)
					PUTC(':');
			}
			if (rd->rd_values) {
				s = (u_char *)"???";
				for (rv = rd->rd_values; rv->rv_name; rv++) {
					if (t == rv->rv_value) {
						s = (u_char *)rv->rv_name;
						break;
					}
				}
				while (c = *s++)
					PUTC(c);
			}
		}
		PUTC('>');
		break;
	case '%':
		PUTC('%');
		break;
	case '\0':
		fmt--; /* this provides graceful recovery from format errors */
		break;
	}
	goto loop;
}

/*
 * Format a number n in base b, returning the number of characters
 * generated.  The resulting string is not null-terminated.
 */
int
printn(bufptr, bufend, n, b, width, flags)
	u_char *bufptr;
	u_char *bufend;
	u_long n;
	u_long b;
	int width;
	int flags;
{
	register u_char *cp;
	int negative, leadinghex, leadingzero;
	u_char *orig_bufptr, numbuf[NBBY * sizeof(u_long)];

	orig_bufptr = bufptr;
	if (b < 2L || b > 16L)
		b = 10L;
	if (negative = ((flags & SIGN) && (long)n < 0))
		n = (u_long)(-(long)n);
	leadinghex = ((flags & FORM) && b == 16L);
	leadingzero = (leadinghex || ((flags & FORM) && b == 8L && n != 0));
	cp = &numbuf[0];
	do {
		*cp++ = "0123456789abcdef"[n % b];
		n /= b;
	} while (n);
	width -= (cp-- - &numbuf[0]) + negative + leadinghex + leadingzero;
	if (width > 0 && !(flags & LEFT) && (flags & ZERO)) {
		if (negative)
			PUTC('-');
		if (leadingzero)
			PUTC('0');
		if (leadinghex)
			PUTC('x');
		do {
			PUTC('0');
		} while (--width > 0);
	} else {
		if (width > 0 && !(flags & LEFT)) {
			do {
				PUTC(' ');
			} while (--width > 0);
		}
		if (negative)
			PUTC('-');
		if (leadingzero)
			PUTC('0');
		if (leadinghex)
			PUTC('x');
	}
	do {
		PUTC(*cp);
	} while (--cp >= &numbuf[0]);
	if (width > 0) {
		do {
			PUTC(' ');
		} while (--width > 0);
	}
	return(bufptr - orig_bufptr);
}
