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
static char	*sccsid = "@(#)$RCSfile: stub.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:10 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#include "mach_kdb.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/conf.h>
#include <ufs/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/kernel.h>
#include <vm/vm_kern.h>
#define DEBUG
#include <i386/PS2/debugf.h>

#define KD_READY_0

#ifdef KD_READY_0
extern int	kddebug;		/* defined in kd.c */
int		rebootflag = 1;		/* used in kd.c */
#endif KD_READY_0

#ifdef nodef
kdopen(dev,flag)			/* real thing is in kd.c */
	int	dev;
	int	flag;
{
	DEBUGF(kddebug & DEBUG_OPEN,
		printf("kdopen(dev=%x,flag=%x)\n",dev,flag));
	return(0);
}

kdclose(dev,flag)			/* real thing is in kd.c */
	int	dev;
	int	flag;
{
	DEBUGF(kddebug & DEBUG_CLOSE,
		printf("kdclose(dev=%x,flag=%x)\n",dev,flag));
	return(0);
}

kdread(dev,uio,flag)			/* real thing is in kd.c */
	int		dev;
	struct uio	*uio;
	int		flag;
{
if (kddebug & BIT27)
	return(real_kdread(dev,uio,flag));
else {
	int	c;
	int	c_count = 0;

	DEBUGF(kddebug & DEBUG_READ,
		printf("kdread(dev=%x,uio=%x,flag=%x)\n",dev,uio,flag));
	do {
		c = getchar();
		c_count++;
	} while (ureadc(c,uio) && c != '\n' && uio->uio_iovcnt > 0);
	DEBUGF(kddebug & DEBUG_READ,
		printf("kdread leaving... c_count=0x%x\n", c_count));

	return(0);
}
}

kdwrite(dev,uio,flag)			/* real thing is in kd.c */
	int		dev;
	struct uio	*uio;
	int		flag;
{
	int c;

	DEBUGF(kddebug & DEBUG_WRITE,
		printf("kdwrite(dev=%x,uio=%x)\n",dev,uio));
	while (uio->uio_iovcnt > 0 && (c = uwritec(uio)) >= 0)
		cnputc(c);
	return(0);

}

kdioctl(dev,cmd,addr,mode)		/* real thing is in kd.c */
	int		dev;
	int		cmd;
	caddr_t		addr;
	int		mode;
{
	DEBUGF(kddebug & DEBUG_IOCTL,
		printf("kdioctl(dev=%x,cmd=%x,addr=%x,mode=%x)\n",
				dev,cmd,addr,mode));
	return(0);
}

kdintr(vec)
	int		vec;
{
	DEBUGF(kddebug & DEBUG_INTR,
		printf("kdintr(vec=%x)\n",vec));
	panic("kdintr");
}
#endif nodef

kdd_getchar()
{
	return(cngetc());
}

#ifdef notdef
in(port)
int port;
{
	return inb (port);
}						/* com.c */

out(port,value)
int port,value;
{ 	
	outb (port,value);
}						/* com.c */
#endif

#ifdef notdef
readtodc() {printf("readtodc");}		/* clock.c */
writetodc() {panic("writetodoc");}		/* clock.c */
#endif

fpintr() {panic("fpintr");}			/* pic.c */
asm() {panic("asm");}				/* ?.c */

kbdopen() {printf("--- kbdopen ---\n");}	/* used in conf.c */
kbdclose() {printf("--- kbdclose ---\n");}	/* used in conf.c */
kbdread() {printf("--- kbdread ---\n");}	/* used in conf.c */
kbdioctl() {printf("--- kbdioctl ---\n");}	/* used in conf.c */
kbdselect() {printf("--- kbdselect ---\n");}	/* used in conf.c */
kd_enqsc() {printf("--- kd_enqsc ---\n");}	/* used in kd.c */
kd_slmscu() {printf("--- kd_slmscu ---\n");}	/* used in kd.c */
kd_slmscd() {printf("--- kd_slmscd ---\n");}	/* used in kd.c */
kd_slmwd() {printf("--- kd_slmwd ---\n");}	/* used in kd.c */
