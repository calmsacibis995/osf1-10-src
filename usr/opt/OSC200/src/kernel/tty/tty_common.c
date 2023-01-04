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
static char *rcsid = "@(#)$RCSfile: tty_common.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:08:30 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <sys/param.h>
#undef TTYDEFCHARS
#include <sys/ioctl.h>
#include <tty/tty_common.h>
#define TTYDEFCHARS
#include <sys/ttydefaults.h>

/* symbolic sleep message strings */
const char ttyin[] = "ttyin";
const char ttyout[] = "ttyout";
const char ttopen[] = "ttyopn";
const char ttclos[] = "ttycls";
const char ttybg[] = "ttybg";
const char ttybuf[] = "ttybuf";

/*
 * Table giving parity for characters and indicating
 * character classes to tty driver. The 8th bit
 * indicates parity, the 7th bit indicates the character
 * is an alphameric or underscore (for ALTWERASE), and the
 * low 6 bits indicate delay type.  If the low 6 bits are 0
 * then the character needs no special processing on output.
 */

const char partab[] = {
	0001,0201,0201,0001,0201,0001,0001,0201,	/* nul - bel */
	0202,0004,0003,0207,0005,0206,0201,0001,	/* bs - si */
	0201,0001,0001,0201,0001,0201,0201,0001,	/* dle - etb */
	0001,0201,0201,0001,0201,0001,0001,0201,	/* can - us */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* sp - ' */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* ( - / */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* 0 - 7 */
	0300,0100,0000,0200,0000,0200,0200,0000,	/* 8 - ? */
	0200,0100,0100,0300,0100,0300,0300,0100,	/* @ - G */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* H - O */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* P - W */
	0300,0100,0100,0200,0000,0200,0200,0300,	/* X - _ */
	0000,0300,0300,0100,0300,0100,0100,0300,	/* ` - g */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* h - o */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* p - w */
	0100,0300,0300,0000,0200,0000,0000,0201,	/* x - del */
	/*
	 * meta chars
	 */
	0001,0201,0201,0001,0201,0001,0001,0201,	/* nul - bel */
	0202,0004,0003,0201,0005,0206,0201,0001,	/* bs - si */
	0201,0001,0001,0201,0001,0201,0201,0001,	/* dle - etb */
	0001,0201,0201,0001,0201,0001,0001,0201,	/* can - us */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* sp - ' */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* ( - / */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* 0 - 7 */
	0300,0100,0000,0200,0000,0200,0200,0000,	/* 8 - ? */
	0200,0100,0100,0300,0100,0300,0300,0100,	/* @ - G */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* H - O */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* P - W */
	0300,0100,0100,0200,0000,0200,0200,0300,	/* X - _ */
	0000,0300,0300,0100,0300,0100,0100,0300,	/* ` - g */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* h - o */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* p - w */
	0100,0300,0300,0000,0200,0000,0000,0201 	/* x - del */
};

struct speedtab ttcompatspeeds[] = {
	38400,	15,
	19200,	14,
	9600,	13,
	4800,	12,
	2400,	11,
	1800,	10,
	1200,	9,
	600,	8,
	300,	7,
	150,	5,
	134,	4,
	110,	3,
	75,	2,
	50,	1,
	0,	0,
	-1,	-1,
};

int ttcompatspcodes[16] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600, 1200,
	1800, 2400, 4800, 9600, 19200, 38400
};

int
ttspeedtab(int speed, struct speedtab table[])
{
	register int i;

	for (i = 0; table[i].sp_speed != -1; i++)
		if (table[i].sp_speed == speed)
			return(table[i].sp_code);
	return(-1);
}

tcflag_t
ttcompatgetflags( register tcflag_t iflag, 
		  register tcflag_t lflag,
		  register tcflag_t oflag,
		  register tcflag_t cflag
		  )
{
	register flags = 0;

	if (iflag&IXOFF)
		flags |= TANDEM;
	if (iflag&ICRNL || oflag&ONLCR)
		flags |= CRMOD;
	if (cflag&PARENB) {
		if (iflag&INPCK) {
			if (cflag&PARODD)
				flags |= ODDP;
			else
				flags |= EVENP;
		} else
			flags |= EVENP | ODDP;
	} else {
		if (!(oflag&OPOST))
			flags |= LITOUT;
		if ((cflag&CSIZE) == CS8)
			flags |= PASS8;
	}
	
	if ((lflag&ICANON) == 0) {	
		/* fudge */
		if (iflag&IXON || lflag&ISIG || lflag&IEXTEN || cflag&PARENB)
			flags |= CBREAK;
		else
			flags |= RAW;
	}
	if (oflag&OXTABS)
		flags |= XTABS;
	if (lflag&ECHOE)
		flags |= CRTERA|CRTBS;
	if (lflag&ECHOKE)
		flags |= CRTKIL|CRTBS;
	if (lflag&ECHOPRT)
		flags |= PRTERA;
	if (lflag&ECHOCTL)
		flags |= CTLECH;
	if ((iflag&IXANY) == 0)
		flags |= DECCTQ;
	flags |= lflag&(ECHO|MDMBUF|TOSTOP|FLUSHO|NOHANG|PENDIN|NOFLSH);
	return (flags);
}

void
ttcompatsetsgflags(
		  register tcflag_t flags,
		  tcflag_t *iflagp,
		  tcflag_t *oflagp,
		  tcflag_t *lflagp,
		  tcflag_t *cflagp,
		  cc_t *minp,
		  cc_t *timep
		)
{
	register tcflag_t	iflag = *iflagp;
	register tcflag_t	oflag = *oflagp;
	register tcflag_t	lflag = *lflagp;
	register tcflag_t	cflag = *cflagp;

	if (flags & RAW) {
		iflag &= IXOFF;
		oflag &= ~OPOST;
		lflag &= ~(ISIG|ICANON|IEXTEN);
		*minp = 1;
		*timep = 0;
	} else {
		iflag |= BRKINT|IXON|IMAXBEL;
		oflag |= OPOST;
		lflag |= ISIG|IEXTEN;
		if (flags & XTABS)
			oflag |= OXTABS;
		else
			oflag &= ~OXTABS;
		if (flags & CBREAK)
			lflag &= ~ICANON;
		else
			lflag |= ICANON;
		if (flags&CRMOD) {
			iflag |= ICRNL;
			oflag |= ONLCR;
		} else {
			iflag &= ~ICRNL;
			oflag &= ~ONLCR;
		}
	}
	if (flags&ECHO)
		lflag |= ECHO;
	else
		lflag &= ~ECHO;
		
	if ((flags&(EVENP|ODDP)) == EVENP) {
		iflag |= INPCK;
		cflag &= ~PARODD;
	} else if ((flags&(EVENP|ODDP)) == ODDP) {
		iflag |= INPCK;
		cflag |= PARODD;
	} else 
		iflag &= ~INPCK;
	if (flags&TANDEM)
		iflag |= IXOFF;
	else
		iflag &= ~IXOFF;
	*iflagp = iflag;
	*oflagp = oflag;
	*lflagp = lflag;
	*cflagp = cflag;
}

void
ttcompatsetlflags(
		  register tcflag_t flags,
		  tcflag_t *iflagp,
		  tcflag_t *oflagp,
		  tcflag_t *lflagp,
		  tcflag_t *cflagp
		)
{
	register tcflag_t iflag = *iflagp;
	register tcflag_t oflag = *oflagp;
	register tcflag_t lflag = *lflagp;
	register tcflag_t cflag = *cflagp;

	if (flags&CRTERA)
		lflag |= ECHOE;
	else
		lflag &= ~ECHOE;
	if (flags&CRTKIL)
		lflag |= ECHOKE;
	else
		lflag &= ~ECHOKE;
	if (flags&PRTERA)
		lflag |= ECHOPRT;
	else
		lflag &= ~ECHOPRT;
	if (flags&CTLECH)
		lflag |= ECHOCTL;
	else
		lflag &= ~ECHOCTL;
	if ((flags&DECCTQ) == 0)
		lflag |= IXANY;
	else
		lflag &= ~IXANY;
	lflag &= ~(MDMBUF|TOSTOP|FLUSHO|NOHANG|PENDIN|NOFLSH);
	lflag |= flags&(MDMBUF|TOSTOP|FLUSHO|NOHANG|PENDIN|NOFLSH);
	if (flags&(LITOUT|PASS8)) {
		iflag &= ~ISTRIP;
		cflag &= ~(CSIZE|PARENB);
		cflag |= CS8;
		if (flags&LITOUT)
			oflag &= ~OPOST;
		if ((flags&(PASS8|RAW)) == 0)
			iflag |= ISTRIP;
	} else if ((flags&RAW) == 0) {
		cflag &= ~CSIZE;
		cflag |= CS7|PARENB;
		oflag |= OPOST;
	}
	*iflagp = iflag;
	*oflagp = oflag;
	*lflagp = lflag;
	*cflagp = cflag;
}

void
termios_to_sgttyb(struct termios *termp, struct sgttyb *sg)
{
	register u_char *cc = termp->c_cc;
	register int speed;

	speed = ttspeedtab(termp->c_ospeed, ttcompatspeeds);
	sg->sg_ospeed = (speed == -1) ? 15 : speed;
	if (termp->c_ispeed == 0)
		sg->sg_ispeed = sg->sg_ospeed;
	else {
		speed = ttspeedtab(termp->c_ispeed, ttcompatspeeds);
		sg->sg_ispeed = (speed == -1) ? 15 : speed;
	}
	sg->sg_erase = cc[VERASE];
	sg->sg_kill = cc[VKILL];
	sg->sg_flags = ttcompatgetflags(termp->c_iflag, termp->c_lflag,
				termp->c_oflag, termp->c_cflag);
}

void
sgttyb_to_termios(struct sgttyb *sg, struct termios *termp, tcflag_t *flagp)
{
	int speed;

	if (((speed = sg->sg_ispeed) > 15) || (speed < 0))
		termp->c_ispeed = speed;
	else
		termp->c_ispeed = ttcompatspcodes[speed];
	if (((speed = sg->sg_ospeed) > 15) || (speed < 0))
		termp->c_ospeed = speed;
	else
		termp->c_ospeed = ttcompatspcodes[speed];
	termp->c_cc[VERASE] = sg->sg_erase;
	termp->c_cc[VKILL] = sg->sg_kill;
	*flagp = (*flagp & 0xffff0000) | sg->sg_flags;
	ttcompatsetsgflags(*flagp, &termp->c_iflag, &termp->c_oflag, 
		&termp->c_lflag, &termp->c_cflag, 
		&termp->c_cc[VMIN], &termp->c_cc[VTIME]);
}

void
termios_to_tchars(struct termios *termp, struct tchars *tc)
{
	tc->t_intrc = termp->c_cc[VINTR];
	tc->t_quitc = termp->c_cc[VQUIT];
	tc->t_startc = termp->c_cc[VSTART];
	tc->t_stopc = termp->c_cc[VSTOP];
	tc->t_eofc = termp->c_cc[VEOF];
	tc->t_brkc = termp->c_cc[VEOL];
}

void
tchars_to_termios(struct tchars *tc, struct termios *termp)
{
	termp->c_cc[VINTR] = tc->t_intrc;
	termp->c_cc[VQUIT] = tc->t_quitc;
	termp->c_cc[VSTART] = tc->t_startc;
	termp->c_cc[VSTOP] = tc->t_stopc;
	termp->c_cc[VEOF] = tc->t_eofc;
	termp->c_cc[VEOL] = tc->t_brkc;
	if (tc->t_brkc == -1)
		termp->c_cc[VEOL2] = ((cc_t)_POSIX_VDISABLE);
}

void
termios_to_ltchars(struct termios *termp, struct ltchars *ltc)
{
	ltc->t_suspc = termp->c_cc[VSUSP];
	ltc->t_dsuspc = termp->c_cc[VDSUSP];
	ltc->t_rprntc = termp->c_cc[VREPRINT];
	ltc->t_flushc = termp->c_cc[VDISCARD];
	ltc->t_werasc = termp->c_cc[VWERASE];
	ltc->t_lnextc = termp->c_cc[VLNEXT];
}

void
ltchars_to_termios(struct ltchars *ltc, struct termios *termp)
{
	termp->c_cc[VSUSP] = ltc->t_suspc;
	termp->c_cc[VDSUSP] = ltc->t_dsuspc;
	termp->c_cc[VREPRINT] = ltc->t_rprntc;
	termp->c_cc[VDISCARD] = ltc->t_flushc;
	termp->c_cc[VWERASE] = ltc->t_werasc;
	termp->c_cc[VLNEXT] = ltc->t_lnextc;
}

void
flags_to_termios(unsigned int cmd, tcflag_t newflags, struct termios *termp, tcflag_t *flagp)
{
	if (cmd == TIOCLSET)
		*flagp = (*flagp & 0xffff) | newflags << 16;
	else {
		int ret = ttcompatgetflags(termp->c_iflag, termp->c_lflag,
					   termp->c_oflag, termp->c_cflag);

		*flagp = (ret & 0xffff0000) | (*flagp & 0xffff);
		if (cmd == TIOCLBIS)
			*flagp |= newflags << 16;
		else
			*flagp &= ~(newflags << 16);
	}
	ttcompatsetlflags(*flagp, &termp->c_iflag, &termp->c_oflag,
			  &termp->c_lflag, &termp->c_cflag);
}
