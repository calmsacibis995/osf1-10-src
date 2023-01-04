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
static char *rcsid = "@(#)$RCSfile: dec2000_cons.c,v $ $Revision: 1.1.5.7 $ (DEC) $Date: 1993/11/02 15:25:52 $";
#endif
/*
 * Revision 1.1.2.2  1992/10/12  07:43:56  Gary_Dupuis
 * 	Initial insertion into live pool.
 * 	[92/10/09  09:53:01  Gary_Dupuis]
 */
/*
 * Alpha Jensen console dispatcher
 * This module is called during machine initialization to figure out
 * if there is a screen and keyboard.  If so, they are used.
 * Otherwise, the on-board serial port is used.
 * 
 * Regardless of which is used, the other may be opened later by the
 * user through the normal open routine.
 *
 * Modification history
 *
 * W. Treese  06/02/92 ported to Alpha/OSF
 * L. Stewart 10/18/91 created from carcass of old apc driver.
 */

#include <sys/conf.h>
#include <sys/types.h>
#include <sys/tty.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>

/* Entry points for the graphics driver and the serial line driver. */

extern int  gpc_cons_init(), ace_cons_init();
extern int  gpcopen(), aceopen();
extern int  gpcclose(), aceclose();
extern int  gpcread(), aceread();
extern int  gpcwrite(), acewrite();
extern int  gpcselect(), aceselect();
extern int  gpcioctl(), aceioctl();
extern int  gpcputc(), aceputc();
extern int  gpcgetc(), acegetc();
extern int  gpcrint(), acerint();
extern int  gpcxint(), acexint();
extern int  gpcstart(), acestart();
extern struct tty gpc_tty[], ace_tty[];

extern struct tty cons[];

int abcprobe(int reg1, int reg2)
{
	/* Initialize select/wakeup queue. */
	queue_init(&cons[0].t_selq);

	/* probe graphics console and (possibly) alternate console */
	gpcprobe(reg1, reg2);
/* FIXME - aceprobe(reg1, reg2); *//* FIXME - needed and OK to do? */

	/* Initialization is done in abc_cons_init, so there is
	 * nothing to do here. */

	return(1);
}

int abcattach(int reg)
{
	/* Initialization is done in abc_cons_init, so there is
	 * nothing to do here. */

	return(1);
}

int abc_cons_init()
{
    /*
     * First, always complete ACE initialization.
     * Then we must decide whether the console will be on a graphics
     *  device or on a serial line.
     * gpc_cons_init returns 1 if it finds a graphics console and zero
     *  otherwise.
     */
    (void) ace_cons_init();

    consDev = (gpc_cons_init()) ? GRAPHIC_DEV : 0;

    if (consDev == GRAPHIC_DEV)
	cdevsw[0].d_ttys = &gpc_tty[0];
    else 
	cdevsw[0].d_ttys = &ace_tty[0];
}

int abc_cnopen(dev, flag)
     dev_t dev;
     unsigned int flag;
{
    if ( consDev == GRAPHIC_DEV || minor(dev) == 1 )
	return( gpcopen(dev, flag) );
    else
        return( aceopen(dev, flag) );
}

int abc_cnclose(dev, flag)
     dev_t dev;
     unsigned int flag;
{
    if ( consDev == GRAPHIC_DEV || minor(dev) == 1 )
	return( gpcclose(dev, flag) );
    else
        return( aceclose(dev, flag) );
}

int abc_cnread(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     unsigned int flag;
{
    if ( consDev == GRAPHIC_DEV || minor(dev) == 1 )
	return( gpcread(dev, uio, flag) );
    else
        return( aceread(dev, uio, flag) );
}

int abc_cnwrite(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     unsigned int flag;
{
    if ( consDev == GRAPHIC_DEV || minor(dev) == 1 )
	return( gpcwrite(dev, uio, flag) );
    else
        return( acewrite(dev, uio, flag) );
}

int abc_cnselect(dev, events, revents, scanning)
     dev_t dev;
     short *events, *revents;
     int scanning;
{
    if ( consDev == GRAPHIC_DEV || minor(dev) == 1 )
	return( gpcselect(dev, events, revents, scanning) );
    else
        return( aceselect(dev, events, revents, scanning) );
}

/*ARGSUSED*/
int abc_cnioctl(dev, cmd, data, flag)
     dev_t dev;
     int cmd;
     caddr_t data;
     unsigned int flag;
{
    if ( consDev == GRAPHIC_DEV || minor(dev) == 1 )
	return( gpcioctl(dev, cmd, data, flag) );
    else
        return( aceioctl(dev, cmd, data, flag) );
}      

int abc_cnputc(c)
     register int c;
{
    if ( consDev == GRAPHIC_DEV )
        gpcputc(c);
    else
        aceputc(c);
    if ( (c & 0xff) == '\n')
	abc_cnputc('\r');
}

int abc_cngetc()
{
    register int c;
    if ( consDev == GRAPHIC_DEV )
	c = gpcgetc();
    else
        c = acegetc();
    if ( c == -1 ) return( c );
    c &= 0xff;
    if ( c == '\r' ) c = '\n';
    return (c);
}

int abc_cnrint(int unit)
{
    if ( consDev == GRAPHIC_DEV )
        return(gpcrint(unit));
    else
        return(acerint(unit));
}

int abc_cnxint(int unit)
{
    if ( consDev == GRAPHIC_DEV )
        return(gpcxint(unit));
    else
        return(acexint(unit));
}

int abc_cnstart(struct tty *tp)
{
    if ( consDev == GRAPHIC_DEV )
        return(gpcstart(gpc_tty));
    else
        return(acestart(ace_tty));
}
