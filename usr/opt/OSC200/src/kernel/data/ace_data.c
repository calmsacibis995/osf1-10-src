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
 * Modification History: ace_data.c
 *
 * 15-Jul-92	Win Treese
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
#include <sys/devio.h>
#include <vm/vm_kern.h>
#include <kern/xpr.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <io/dec/uba/ubavar.h>	/* auto-config headers */
#include <machine/cpu.h>
#include <io/dec/eisa/aceregs.h>


#include "ace.h"

#ifdef BINARY

extern	struct	tty ace_tty[];   /* tty structure */
extern  int	nACE;		/* number of ACE controllers */
extern	struct  ace_softc *acesc;	/* software controller struct for ace*/
extern	u_char	acemodem[];	/* keeps track of modem state */
extern	int     ace_cbaud[];     /* baud rate integer flag */
extern	int     ace_brk[];             /* break condition */
extern	struct	timeval acetimestamp[];
extern	char	acesoftCAR;
extern	char	acedefaultCAR;
extern	int	ace_cnt;
extern	caddr_t	acestd[];
extern	struct	controller *aceinfo[];
extern	struct 	ace_softc ace_softc[]; 

#else

struct	tty ace_tty[NACELINE];   /* tty structure */
struct  ace_softc *acesc;	/* software controller struct for ace */
u_char	acemodem[NACELINE];	/* keeps track of modem state */
int     ace_cbaud[NACELINE];     /* baud rate integer flag */
int     ace_brk[NACELINE];             /* break condition */
struct	timeval acetimestamp[NACELINE];
char	acesoftCAR;
char	acedefaultCAR;
int	ace_cnt = NACELINE;
caddr_t	acestd[] = { 0 };
struct	controller *aceinfo[1];
struct 	ace_softc ace_softc[1]; 

#endif
