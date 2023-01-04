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
/*
 * @(#)$RCSfile: tty_common.h,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/07 23:07:43 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#ifndef _TTY_COMMON_H_
#define _TTY_COMMON_H_

#include <mach_assert.h>
#include <sys/termios.h>

#define TTIPRI	28
#define TTOPRI	29

/* symbolic sleep message strings */
extern const char ttyin[], ttyout[], ttopen[], ttclos[], ttybg[], ttybuf[];
/* parity table */
extern const char partab[256];
/* tty default control characters */
extern cc_t ttydefchars[NCCS];

struct speedtab {
        int sp_speed;
        int sp_code;
};

extern struct speedtab ttcompatspeeds[];

extern int ttcompatspcodes[];

#ifndef _POSIX_VDISABLE		/* unistd.h, not included if _KERNEL */
#define _POSIX_VDISABLE	(0377)
#endif	/* _POSIX_VDISABLE */

#define CCEQ(val, c)	((c) == (val) && (val) != (cc_t)_POSIX_VDISABLE)

#if defined(PRIVATE_STATIC)
#undef PRIVATE_STATIC
#endif

#if 	(MACH_ASSERT || PROFILING)
#define PRIVATE_STATIC
#else
#define PRIVATE_STATIC		static
#endif

void
ltchars_to_termios(struct ltchars *, struct termios *);

void
flags_to_termios(unsigned int, tcflag_t, struct termios *, tcflag_t *);

void
sgttyb_to_termios(struct sgttyb *, struct termios *, tcflag_t *);

void
tchars_to_termios(struct tchars *, struct termios *);

void
termios_to_ltchars(struct termios *, struct ltchars *);

void
termios_to_sgttyb(struct termios *, struct sgttyb *);

void
termios_to_tchars(struct termios *, struct tchars *);

tcflag_t
ttcompatgetflags(tcflag_t, tcflag_t, tcflag_t, tcflag_t);

void
ttcompatsetsgflags(tcflag_t, tcflag_t *, tcflag_t *, tcflag_t *, tcflag_t *, cc_t *, cc_t *);

void
ttcompatsetlflags(tcflag_t, tcflag_t *, tcflag_t *, tcflag_t *, tcflag_t *);

int
ttspeedtab(int, struct speedtab []);

#endif 	/* _TTY_COMMON_H_ */
