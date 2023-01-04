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
static char *rcsid = "@(#)$RCSfile: kdebug_io.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/27 14:18:47 $";
#endif

#include <sys/kdebug.h>
#include <machine/varargs.h>

static struct device_buf ttybuf;
extern struct kdebug_cpusw *kdebug_cpup;
extern KdebugInfo kdebug_info;

void kprintf(unsigned long lvl, char *va_alist, ...);

long
init_saio()
{
    if ((*kdebug_cpup->tty_dev->dt_init)(kdebug_cpup->tty_base)) {
        return(-1);
    }

    CIRC_FLUSH(&ttybuf);

    return(0);
}

/*
 * _circ_putc -- insert character in circular buffer
 */
static void
circ_putc(
    char c,
    struct device_buf *db)
{
    char *cp;

    cp = db->db_in + 1 >= &db->db_buf[sizeof(db->db_buf)] ? db->db_buf :
	 db->db_in + 1;

    /*
     * if buffer is full, ignore the character
     */
    if (cp == db->db_out) {
	kprintf(DBG_WARNING, "\ndropped char\n");
	return;
    }

    *db->db_in = c;
    db->db_in = cp;
}

/*
 * circ_getc -- remove character from circular buffer
 */
static char
circ_getc(
    struct device_buf *db)
{
    char c;

    c = *db->db_out++;
    if (db->db_out >= &db->db_buf[sizeof(db->db_buf)])
	db->db_out = db->db_buf;
    return c;
}

/*
 * kdebug_getc -- read in as much data as available, blocking call
 */
char
kdebug_getc()
{
    do {
        while ((*kdebug_cpup->tty_dev->dt_rx_rdy)())
	    circ_putc((char) (*kdebug_cpup->tty_dev->dt_rx_read)(), &ttybuf);
    } while (CIRC_EMPTY(&ttybuf));
    return circ_getc(&ttybuf);
}

/*
 * kdebug_nblock_getc -- read in as much data as available, non-blocking call
 */
char
kdebug_nblock_getc(
    long *success)
{
    long i = 0;

    while (CIRC_EMPTY(&ttybuf) && i++ < 2000000) {
        while ((*kdebug_cpup->tty_dev->dt_rx_rdy)())
	    circ_putc((char) (*kdebug_cpup->tty_dev->dt_rx_read)(), &ttybuf);
    }
    if (CIRC_EMPTY(&ttybuf)) {
	*success = 0L;
	return;
    } else {
	*success = 1L;
        return circ_getc(&ttybuf);
    }
}

/*
 * kdebug_putc -- write out one character, blocking call
 */
void
kdebug_putc(
    char c)
{
    while (!(*kdebug_cpup->tty_dev->dt_tx_rdy)())
	;
    (*kdebug_cpup->tty_dev->dt_tx_write)(c);
}

/*
 * Printn prints a number n in hex.
 */
static void
printn(
    unsigned long n)
{
    char prbuf[32];
    char *cp;

    cp = prbuf;
    do {
	*cp++ = "0123456789abcdef"[n&0xf];
	n >>= 4;
    } while (n);

    do
	kdebug_putc(*--cp);
    while (cp > prbuf);
}

static void
puts(
    char *s)
{
    unsigned long c;

    if (s == 0)
	s = "<NULL>";

    while (c = *s++) {
        if (c == '\n')
	    kdebug_putc('\r');
	kdebug_putc(c);
    }
}

static void
prf(
    char *fmt,
    va_list adx)
{
    char b;
    char c;
    char *s;

    while ((c = *fmt++) != '\0') {

	if (c != '%') {
	    kdebug_putc(c);
	} else {
	    c = *fmt++;

	    switch (c) {
	    case 'x':
		printn(va_arg(adx, long));
		break;

	    case 'c':
		b = va_arg(adx, char);
		kdebug_putc(b & 0x7f);
		break;

	    case 's':
		s = va_arg(adx, char *);
		puts(s);
		break;

	    case '%':
		kdebug_putc('%');
		break;

	    case 'd':
	    case 'u':
	    case 'o':
	    default:
		/*
		 * supporting these means pulling in private kdebug routines
		 * for div and rem which will be needed in printn()
		 */
		kprintf(DBG_WARNING, "prf: format '%c' unsupported\n", c);
		quit();
		break;
	    }
	}
    }
}

void
kprintf(
    unsigned long lvl,
    char	*va_alist,
    ...)
{
    va_list ap;
    char *fmt;

    if (!(kdebug_info.state & KDEBUG_ENABLED) || !(lvl & kdebug_info.debug))
	return;

    va_start(ap);
    fmt = va_arg(ap, char *);
    prf(fmt, ap);
    va_end(ap);
}
