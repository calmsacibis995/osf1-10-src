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
static char *sccsid = "@(#)$RCSfile: xcons.c,v $ $Revision: 1.2.3.4 $ (DEC) $Date: 1992/06/25 18:41:08 $";
#endif	lint

/************************************************************************
 *									*
 *			Copyright (c) 1988,89 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************
 *
 * xcons.c
 *
 * xcons alternate console driver
 *
 * Modification history
 *
 *   6-Nov-91   Mike Larson
 *		Enable DEVIOCGET recognition.
 *
 *   1-May-91	khh
 *		Ported file to OSF.
 *
 *   4-Jul-90	Randall Brown
 *		Created file.
 */
/*
#include "../h/types.h"
#include "../h/errno.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/conf.h"
#include "../h/file.h"
#include "../h/devio.h"
#include "../h/exec.h"
*/
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <io/common/devio.h>
#include <sys/exec.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <io/dec/tc/xcons.h>

#define GRAPHIC_DEV	0x2	/* rpbfix: needs to be moved to a header file */

struct tty	xcons_tty[1];

/* rpbfix: determine how xcons_kern_loop should be set */
int	xcons_kern_loop = 1;


int 	xconsstart();

extern int consDev, printstate;

xconsopen(dev, flag)
	dev_t 	dev;
	int	flag;
{
        register struct tty *tp;
	register int unit = minor(dev);

	/* can not open xcons if graphic head is not console */
	if ((consDev != GRAPHIC_DEV) || (unit != XCONSDEV))
	    return (ENXIO);

	tp = &xcons_tty[unit];
#if	SEC_BASE
	if (tp->t_state&TS_XCLUDE && !privileged(SEC_ALLOWDACACCESS, 0))
#else
	if (tp->t_state&TS_XCLUDE && u.u_uid!=0)
#endif
	    return (EBUSY);
	tp->t_addr = (caddr_t)tp;
	tp->t_oproc = xconsstart;

	/*
	 * Look at the compatibility mode to specify correct 
	 * default parameters and to insure only standard specified 
	 * functionality.
	 */
	 /*
	if ((u.u_procp->p_progenv == A_SYSV) || 
	    (u.u_procp->p_progenv == A_POSIX)) {
	    flag |= O_TERMIO;
	    tp->t_line = TERMIODISC;
	}
	*/
	tp->t_line = TTYDISC;		/* for osf */
#ifdef notdef
#ifdef O_NOCTTY
	/*
	 * Set state bit to tell tty.c not to assign this line as the 
	 * controlling terminal for the process which opens this line.
	 */
	if ((flag & O_NOCTTY) && (u.u_procp->p_progenv == A_POSIX))
	    tp->t_state |= TS_ONOCTTY;
#endif O_NOCTTY
#endif
	if ((tp->t_state&TS_ISOPEN) == 0) {
	    ttychars(tp);
	    tp->t_state = TS_ISOPEN|TS_CARR_ON;
	    /*
	    tp->t_cflag = tp->t_cflag_ext = B9600;
	    tp->t_iflag_ext = 0;
	    tp->t_oflag_ext = 0;
	    tp->t_lflag_ext = 0;
	    */

	    tp->t_flags = RAW;
	    tp->t_iflag = 0;
	    tp->t_oflag = 0;
	    tp->t_cflag |= CS8|CREAD|HUPCL; 
	    tp->t_lflag = 0;
	    
	    tp->t_iflag |= IXOFF;	/* flow control for qconsole */
	}

        /*
 	 * Process line discipline specific open
 	 */
        return ((*linesw[tp->t_line].l_open)(dev, tp, flag));
}

xconsclose(dev, flag)
	dev_t 	dev;
	int	flag;
{
        register struct tty *tp;
	register int unit = minor(dev);

	tp = &xcons_tty[unit];

	(*linesw[tp->t_line].l_close)(tp);
	ttyclose(tp);

	tty_def_close(tp);
}

xconsread(dev, uio, flag)
	dev_t	dev;
	struct uio *uio;
	int flag;
{
	register int unit = minor(dev);
        register struct tty *tp = &xcons_tty[unit];

	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

xconswrite(dev, uio, flag)
	dev_t	dev;	
	struct uio *uio;
	int flag;
{
	register int unit = minor(dev);
        register struct tty *tp = &xcons_tty[unit];

	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

xconsioctl(dev, cmd, data, flag)
	dev_t	dev;
	int	cmd;
	caddr_t	data;
	int	flag;
{
	register int error, unit = minor(dev);
        register struct tty *tp = &xcons_tty[unit];

	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
	    return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0)
	    return (error);

	switch (cmd) {
/*
 * Hook for future DEVIOCGET functionality.
 */
	case DEVIOCGET:
		return (ENOTTY);

	default:
		return (ENOTTY);
	}
}

xcons_chkq()
{
        register struct tty *tp = &xcons_tty[XCONSDEV];

        if (((tp->t_state & TS_ISOPEN) == 0) || (printstate & PANICPRINT) ||
	    (xcons_kern_loop == 0)) {
	    return (XCONS_CLOSED);
	}
	
	if (tp->t_state & TS_TBLOCK) {
	    return (XCONS_BLOCKED);
	}

	return (XCONS_OK);
}
	

xconsrint(c)
	register char c;
{
        register struct tty *tp = &xcons_tty[XCONSDEV];

	(*linesw[tp->t_line].l_rint)(c, tp);
}

xconsstart(tp)
	register struct tty *tp;
{
        register struct tty *tp0;
	register int s, c;

	s = spltty();

	tp0 = &cdevsw[0].d_ttys[0];

	while (tp->t_outq.c_cc) {
	c = getc(&tp->t_outq);

	/* if START char call console's start routine */
	if (c == tp->t_cc[VSTART]) {
	    /* pass START char onto console to restart output */
	    (*linesw[tp0->t_line].l_rint)(tp0->t_cc[VSTART], tp0);
	}
	/* if STOP char call console's stop routine */
	else if (c == tp->t_cc[VSTOP]) {
	    if ((tp0->t_state&TS_TTSTOP) == 0) {
		tp0->t_state |= TS_TTSTOP;
		(*cdevsw[0].d_stop)(tp0, 0);
	    }
	}
    }
	splx(s);
}

xconsstop(tp)
	register struct tty *tp;
{
}
