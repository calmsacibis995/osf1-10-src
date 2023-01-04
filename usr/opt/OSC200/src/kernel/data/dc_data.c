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
 *	@(#)$RCSfile: dc_data.c,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/06/25 08:10:21 $
 */ 
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
 * derived from dc_data.c	4.1	(ULTRIX)	8/9/90
 */

/*
 * Modification History: dc_data.c
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 * 06-Mar-91	Mark Parenti
 *	Modify to use new I/O data structures.
 *
 * 04-Jul-90	Randall Brown
 *	Created file.
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/map.h>

#include <sys/buf.h>

#include <sys/vm.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/kernel.h>
/* osf added */
#include <vm/vm_kern.h>
#include <kern/xpr.h>

#include <io/common/devio.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>

#include <machine/cpu.h>
#include <io/dec/tc/dc7085reg.h>
#include <io/dec/ws/vsxxx.h>
#include <io/dec/tc/slu.h>

#include "dc.h"

#ifdef BINARY

extern struct	tty	dc_tty[];	/* tty structure		*/
extern struct	slu	slu;		/* serial line communcation function pointers */
extern int	nDC;			/* number of DC controllers */
extern	u_char	dcmodem[];
extern 	u_char	dcmodem_active[];
extern	int	dc_modem_line[];
extern	struct	timeval	dctimestamp[];

extern 	u_short	dc_brk[];
extern 	int	dc_modem_ctl;

extern	char	dcsoftCAR[];
extern	char	dcdefaultCAR[];
extern	int	nNDCLINE;
extern	int	dc_cnt;
extern	int	dc_attach_called;	

extern	struct 	controller *dcinfo[];

extern	struct 	dc_softc dc_softc[];

/*
 * definitions of the modem status register and transmit control register
 *
 * The bit placement of these registers are different from PMAX to 3MAX
 * these variables get set to where the appropriate signal is in the
 * register.
 */
extern	short	dc_rdtr[], dc_rrts[], dc_rcd[], dc_rss[];
extern	short	dc_rdsr[], dc_rcts[], dc_xmit[];

#else 

struct	tty dc_tty[NDC * NDCLINE];	/* tty structure		*/
struct	slu	slu;			/* Serial Line communication function pointers */
int	nDC = NDC;
u_char	dcmodem[NDC * NDCLINE];		/* keeps track of modem state */
u_char 	dcmodem_active[NDC] = {0};
int	dc_modem_line[NDC * NDCLINE];
struct	timeval dctimestamp[NDC * NDCLINE];
u_short	dc_brk[NDC];
int	dc_modem_ctl;	/* holds whether we use full or limited modem control */

char	dcsoftCAR[NDC];
char	dcdefaultCAR[NDC];
int	nNDCLINE = NDC * NDCLINE;
int	dc_cnt = 0;
int 	dc_attach_called = 0;

struct device *dcinfo[NDC];

struct dc_softc dc_softc[NDC];

/*
 * definitions of the modem status register and transmit control register
 *
 * The bit placement of these registers are different from PMAX to 3MAX
 * these variables get set to where the appropriate signal is in the
 * register.
 */
short	dc_rdtr[NDC * NDCLINE], dc_rrts[NDC *NDCLINE], dc_rcd[NDC * NDCLINE]; 
short	dc_rss[NDC * NDCLINE];
short	dc_rdsr[NDC * NDCLINE], dc_rcts[NDC * NDCLINE], dc_xmit[NDC * NDCLINE];

#endif




