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
static char *rcsid = "@(#)$RCSfile: scs_data.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/04/13 14:58:58 $";
#endif
/*
 * derived from scs_data.c	4.1	(ULTRIX)	7/2/90";
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		Systems Communication Services
 *
 *   Abstract:	This module contains Systems Communication Services( SCS )
 *		configurable variables and data structures.
 *
 *   Creator:	Todd M. Katz	Creation Date:	May 6, 1985
 *
 *   Modification History:
 *
 *   18-Sep-1989	Pete Keilty
 *	Added SCS errlogging varaible scs_errlog.
 *
 *   10-Feb-1989	Todd M. Katz		TMK0002
 *	Added SCS configuration variable scs_severity.
 *
 *   08-Jan-1988	Todd M. Katz		TMK0001
 *	Formated module, and revised comments.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<io/dec/scs/sca.h>
#include		<io/dec/scs/scaparam.h>
#include		"scs_data.h"
#include		"bvpssp.h"
#include		"dssc.h"
#include		"hsc.h"
#include		"scsnet.h"
#include		"uq.h"

/* SCS Configuration Variables.
 */
u_long		scs_cushion		/* SCS send credit cushion	     */
		    = 1;		/*  MAX:    16;	DEF:      1; MIN:  0 */
u_long		scs_dg_size		/* Maximum application datagram size */
		    = 256;		/*  MAX:   984; DEF:    567; MIN: 28 */
u_long		scs_max_conns		/* Maximum number of connections     */
		    = SCS_MAX_CONNS;	/*  MAX: 32767; DEF:     68; MIN:  2 */
u_long		scs_msg_size		/* Maximum appl sequenced msg size   */
		    = 256;		/*  MAX:   984; DEF:    112; MIN: 52 */
u_char		scs_node_name[ 8 ]	/* SCS node name( blank filled )     */
		    = SCS_NODE_NAME;
u_char		scs_sanity		/* SCS sanity timer interval( secs ) */
		    = 2;		/*  MAX:    60; DEF:      1; MIN:  1 */
u_short		scs_severity		/* SCS console logging severity	     */
		    = SCA_SEVERITY;	/*  MAX:     5; DEF:	  0; MIN:  0 */
scaaddr		scs_system_id		/* SCS system identification number  */
		    = SCS_SYSID;
u_short		scs_errlog		/* SCA system errlog severity level  */
		    = SCA_ERRLOG2;	/* MAX:	     3; DEF:      2; MIN:  0 */

int		dsaisr_thread_off	/* DSA isr thread disable	     */
		    = 0;		/* MAX:	     1; DEF:	  0; MIN:  0 */


/* 
 * DSA Memory Allocation Parameters
 *
 * On system startup SCS will use the paramters below
 * to allocate static memory for all DSA operations.
 *
 * Primary use for each zone:
 * 	64 byte		- nport carriers 
 *	256 byte	- nport mapping resource, request blocks
 *	512 byte	- scs connection blocks
 *	1024 byte	- sysap connection blocks
 *	2048 byte	- disk/tape unit blocks, pccb
 *
 * Using dbx you can look at the zone statistics:
 * # dbx -k /vmunix /dev/mem
 * (dbx)p sca_zone_history
 *
 */

/* max number of entries in each DSA zone */
#define SCA_ZONE_64   	512
#define SCA_ZONE_256 	1024
#define SCA_ZONE_512  	256
#define SCA_ZONE_1024  	32
#define SCA_ZONE_2048	128

/* actual parameter structure used in scs_subr */
unsigned int sca_zone_max[SCA_ZONE_MAX] = {
      SCA_ZONE_64,              /*     64 Byte  */
      SCA_ZONE_256,             /*    256 Byte  */
      SCA_ZONE_512,             /*    512 Byte  */
      SCA_ZONE_1024,            /*   1024 Byte  */
      SCA_ZONE_2048,            /*   2048 Byte  */
};
