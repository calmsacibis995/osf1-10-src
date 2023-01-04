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
 *	Modification History
 *
 *	20-Jul-1989	Mark A. Parenti
 *	Remove uq_burst table. This information is now the the uq_cinfo
 *	table in uqport.h
 *
 *	07-Mar-1989	Todd M. Katz		TMK0002
 *		1. Include header file ../vaxmsi/msisysap.h.
 *		2. Use the ../machine link to refer to machine specific header
 *		   files.
 *
 *	18-July-1988 - map
 *		Dynamically allocate data structures.
 *
 *      02-Jun-1988     Ricky S. Palmer
 *              Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *	14-Mar-1988	Larry Cohen
 *		Remove all references to ra_info.
 *
 *	09-Jan-1988	Todd M. Katz		TMK0001
 *		Included new header files ../vaxscs/scaparam.h,
 *		../vaxmsi/msisysap.h, and  ../vaxmsi/msiscs.h.
 */

#include "uq.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
/* NOT in OSF
#include <sys/dir.h>
*/
#include <sys/user.h>
#include <sys/map.h>
#include <sys/vm.h>
#include <sys/vmmac.h>
#include <sys/dk.h>
#include <sys/cmap.h>
#include <sys/uio.h>
#include <sys/ioctl.h>

#include <machine/cpu.h>
#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>
#include <io/dec/sysap/mscp_msg.h>
#include <io/common/devdriver.h>


#include <io/dec/bi/bireg.h>
#include <io/dec/bi/buareg.h>
#include <io/dec/bi/bdareg.h>

#include	<sys/types.h>
/*
#include	<sys/time.h>
*/
#include	<dec/binlog/errlog.h>
#include	<io/dec/scs/sca.h>
#include	<io/dec/scs/scaparam.h>
#include	<io/dec/ci/cippdsysap.h>
#include	<io/dec/ci/cisysap.h>
#include	<io/dec/np/npsysap.h>
#include	<io/dec/msi/msisysap.h>
#include	<io/dec/bi/bvpsysap.h>
#include	<io/dec/gvp/gvpsysap.h>
#include	<io/dec/uba/uqsysap.h>
#include	<io/dec/sysap/sysap.h>
#include	<io/dec/ci/cippdscs.h>
#include	<io/dec/ci/ciscs.h>
#include	<io/dec/np/npscs.h>
#include	<io/dec/msi/msiscs.h>
#include	<io/dec/bi/bvpscs.h>
#include	<io/dec/gvp/gvpscs.h>
#include	<io/dec/uba/uqscs.h>
#include	<io/dec/scs/scs.h>
#include	<io/dec/uba/uqppd.h>




/* 	Port Info Block
 */

struct port_info *port_info_ptr[NUQ];

struct	controller *uqminfo[NUQ];

int	nNUQ = NUQ;

/* UQ hardware port type conf init stub routines which
 * are not used by the autoconfiguration code. 
 */
udainit() { /* stub */ }
klesiuinit() { /* stub */ }

