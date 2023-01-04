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
 *	@(#)$RCSfile: scc_data.c,v $ $Revision: 1.2.12.2 $ (DEC) $Date: 1993/06/24 22:33:57 $
 */ 
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
 * Modification History: scc_data.c
 *
 * 17-Apr-91	Kuo-Hsiung Hsieh
 *		Created file.
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
#include <vm/vm_kern.h>
#include <kern/xpr.h>
#include <kern/queue.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
/* #include <sys/exec.h> */
/* #include <sys/kmalloc.h> */
/* #include <sys/sys_tpath.h> */
#include <io/dec/uba/ubavar.h>	/* auto-config headers */

#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>

#include <machine/cpu.h>
#include <io/dec/tc/ioasic.h>
#include <io/dec/tc/scc_common.h>   /* mjm - sync support */
#include <io/dec/tc/sccreg.h>
#include <io/dec/ws/vsxxx.h>
#include <io/dec/tc/xcons.h>
#include <io/dec/tc/tc.h>
#include <hal/cons_sw.h>

#include "scc.h"

#ifdef BINARY

extern	struct	tty scc_tty[];   /* tty structure */
extern  int	nSCC;		/* number of SCC controllers */
extern	struct  scc_softc *sccsc;	/* software controller struct for scc */
extern	u_char	sccmodem[];	/* keeps track of modem state */
extern	int     scc_cbaud[];     /* baud rate integer flag */
extern	int     scc_brk[];             /* break condition */
extern	struct	timeval scctimestamp[];
extern	char	sccsoftCAR;
extern	char	sccdefaultCAR;
extern	int	scc_cnt;
extern	caddr_t	sccstd[];
extern	struct	controller *sccinfo[];
extern	struct 	scc_softc scc_softc[]; 

/* mjm - sync support */
#ifndef WDD_SSCC
extern	int	sscc_Probe();
extern	int	sscc_Attach();
#endif

#else

struct	tty scc_tty[NSCCLINE];   /* tty structure */
struct  scc_softc *sccsc;	/* software controller struct for scc */
u_char	sccmodem[NSCCLINE];	/* keeps track of modem state */
int     scc_cbaud[NSCCLINE];     /* baud rate integer flag */
int     scc_brk[NSCCLINE];             /* break condition */
struct	timeval scctimestamp[NSCCLINE];
char	sccsoftCAR;
char	sccdefaultCAR;
int	scc_cnt = NSCCLINE;
caddr_t	sccstd[] = { 0 };
struct	controller *sccinfo[1];
struct 	scc_softc scc_softc[1]; 

/* mjm - sync support */
#ifndef WDD_SSCC
int sscc_Probe(com)
int com;
{
    return( 0 );
}
int sscc_Attach(com)
int com;
{
    return( 0 );
}
#endif

#endif
