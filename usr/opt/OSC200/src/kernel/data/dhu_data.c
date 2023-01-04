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
 * dhu_data.c
 *
 * Modification history
 *
 * 17-Feb-92 - Fernando Fraticelli
 *	Initial port from Ultrix to OSF.
 *
 * OSF work started above this.
 *-----------------------------------------------------------------------------
 *
 * DH(QUV)11/CX(ABY)(8,16) data file
 *
 * 16-Jan-86 - Larry Cohen
 *
 *	Add full DEC standard 52 support.
 *
 * 14-Apr-86 - jaw
 *
 *	Remove MAXNUBA referances.....use NUBA only!
 *
 * 26-Apr-86 - ricky palmer
 *
 *	Added "devio.h" to include list. V2.0
 *
 * 29-Jan-87 - Tim Burke
 *
 *	Added definition of dhudsr, a variable used to define the type of 
 *	modem control that is being followed.
 *
 * 10-Mar-87 - rsp (Ricky Palmer)
 *
 *	Updated comments to reflect new CX series controllers.
 *
 * 11-Aug-87 - Tim Burke
 *
 *	Added exec.h to list of include files for compatibility mode check 
 *	stored in the upper 4 bits of the magic number.
 *
 * 24-Mar-88 - Tim Burke
 *
 *	Added dhu_lines which is a variable (one per board) to define how many
 *	lines this board has.
 *
 * 12-Jun-89 - dws
 *
 *	Added trusted path support.
 *
 * 11-Sep-91 - ff
 *
 *	Changed paths for OSF support
 */

#include	"dhu.h"
#include	"uba.h"

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/termio.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/vm.h>
#include <sys/kernel.h>
#include <io/common/devio.h>

#include <arch/mips/hal/cpuconf.h>
#include <io/common/devdriver.h>

#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>
#include <io/dec/uba/dhureg.h>

#include <sys/clist.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/exec.h>

#ifdef BINARY

extern	struct	controller *dhuinfo[];
extern	struct	dhu_softc dhu_softc[];
extern	short	dhusoftCAR[];
extern	short	dhudefaultCAR[];

extern	struct	tty dhu11[];
extern	u_char	 dhumodem[];
extern	struct timeval	dhutimestamp[];
extern	int	ndhu11;

extern	int	nNDHU;
extern	int	cbase[];
extern	int	dhudsr;
extern	int	dhu_lines[];

#else BINARY

int	cbase[NUBA];		/* base address in unibus map */
struct	device *dhuinfo[NDHU];
struct	dhu_softc dhu_softc[NDHU];
short	dhusoftCAR[NDHU];
short	dhudefaultCAR[NDHU];

struct	tty dhu11[NDHU*16];   /* one tty structure per line */
u_char	 dhumodem[NDHU*16];   /* to keep track of modem state */
struct	timeval dhutimestamp[NDHU*16]; /* to keep track of CD transient drops */
int	ndhu11	= NDHU*16;   /* total number of dhu lines   */
int	nNDHU	= NDHU;     /* total number of dhu modules */

int     dhu_lines[NDHU];

#ifdef NODSR
int dhudsr = 0;			/* A "0" here means ignore DSR */
#else NODSR
int dhudsr = 1;			/* A "1" here means drop line if DSR drops */
#endif NODSR

#endif BINARY
