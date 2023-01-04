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
static char	*sccsid = "@(#)$RCSfile: cons_sw.c,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/04/14 16:03:03 $";
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
 * derived from cons_sw.c	4.2      (ULTRIX)  8/9/90";
 */


/*
 * Modification History
 *
 * 20-Dec-90 - Kuo-Hsiung Hsieh
 *	OSF started.
 *	------------------------------------------------------
 *
 * 29-Oct-89 - Randall Brown
 *      Added the call to cpu specific c_init() in cninit().
 *
 * 11-Jul-89 - Randall Brown
 *	Added the ttys entry to each switch entry.
 *
 * 20-May-89 - Randall Brown
 *
 *	created file.
 *
 */

#include <sys/conf.h>
#include <sys/tty.h>
#include <hal/cons_sw.h>


#define DEFAULTSPEED B4800
#define IFLAGS	(EVENP|ECHO|XTABS|CRMOD)
#define IFLAG (BRKINT|IGNPAR|ISTRIP|IXON|IXANY|ICRNL)
#define OFLAG (OPOST|TAB3|ONLCR)
#define LFLAG (ISIG|ICANON|ECHO)
#define CFLAG (PARENB|CREAD|CS7|CLOCAL)


struct cons_sw *cons_swp; /* pointer to the current cons_sw entry 	*/
int consDev = 0;        /* describes whether console is graphic device 	*/
int console_line;	/* non graphic device console line number 	*/
int console_baud;	/* non graphic device console baud rate 	*/

extern int ttselect();
extern int cpu;
extern struct cons_sw cons_sw[];

nocons()
{
	return(0);
}

/*
 *      This routine provides a dummy return for any console drivers which
 *      do not support mmap().  -1 is the normal return for an invalid
 *      mapping.
 */
noconsmmap(dev, off, prot)
    dev_t dev;
    off_t off;
    int prot;
{
        return(-1);
}

/*
 *	This routine is called to configure the console.
 *	
 *	It loops through the entire cons_sw table until it finds
 * 	an entry whose ID matches the cpu ID.
 *
 *	If it reaches the end of the table, then something is wrong.
 */
cninit()
{
    int i = 0; /* Start at beginning of table */

    cons_swp = 0;	/* initialize cons_sw pointer */
    while (cons_sw[i].system_type != 0) { /* 0 == end of table */
	if (cpu == cons_sw[i].system_type) {
	    cons_swp = &cons_sw[i];
	    break;
	}
	i++;
    }
    if (cons_swp == 0) {
	panic("System does not have a console configured.");
    }
    cdevsw[0].d_ttys = cons_swp->ttys;
    (cons_swp->c_init)();		/* call device specific init routine */
}

cnopen(dev, flag)
    dev_t dev;
    int flag;
{
    return((cons_swp->c_open)(dev, flag));
}

cnclose(dev)
    dev_t dev;
{
    return((cons_swp->c_close)(dev));
}

cnread(dev, uio)
    dev_t dev;
    struct uio *uio;
{
    return((cons_swp->c_read)(dev, uio));
}

cnwrite(dev, uio)
    dev_t dev;
    struct uio *uio;
{
    return((cons_swp->c_write)(dev, uio));
}

cnioctl(dev, cmd, addr, flag)
    dev_t dev;
    unsigned int cmd;
    caddr_t addr;
    int flag;
{
    return((cons_swp->c_ioctl)(dev, cmd, addr, flag));
}

cnstop(tp, flag)
    struct tty *tp;
    int flag;
{
    return((cons_swp->c_stop)(tp, flag));
}

cnstart(tp)
    struct tty *tp;
{
    return((cons_swp->c_start)(tp));
}

cnselect(dev, rw)
    dev_t dev;
    int rw;
{
    return((cons_swp->c_select)(dev, rw));
}

cnmmap(dev, off, prot)
    dev_t dev;
    off_t off;
    int prot;
{
    return((cons_swp->c_mmap)(dev, off, prot));
}

cnputc(c)
    unsigned int c;
{
    return((cons_swp->c_putc)(c));
}

cngetc()
{
    return((cons_swp->c_getc)());
}

cnprobe(c)
    unsigned int c;
{
    return((cons_swp->c_probe)(c));
}

cnrint(c)	/* Console receive interrupt  */
int c;
{
    return((cons_swp->c_rint)(c));
}

cnxint(c)	/* Console transmit interrupt */
int c;
{
    return((cons_swp->c_xint)(c));
}

cnparam(tp, t)
register struct tty *tp;
register struct termios *t;
{
    return((cons_swp->c_param)(tp, t));
}
