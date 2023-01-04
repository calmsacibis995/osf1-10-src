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
static char *rcsid = "@(#)$RCSfile: npvar.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 17:48:37 $";
#endif
/*
 * derived from npvar.c	5.2	(ULTRIX)	10/16/91";
 */
/************************************************************************
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		internal data structures and variables.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Modification History:
 *
 *   31-Oct-1991	Peter Keilty
 *	Ported to OFS/1
 *
 *   16-Oct-1991	Brian Nadeau
 *	Updates/bug fixes.
 *
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<dec/binlog/errlog.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>
#include		<io/dec/ci/cippd.h>
#include		<io/dec/np/npport.h>

/* External Variables and Routines.
 */
extern	PB		*cippd_get_pb();
extern	SCSH		*np_alloc_dg(), *np_alloc_msg(), *np_remove_dg(),
			*np_remove_msg();
extern	u_long		np_alloc_buf(), np_get_port(), np_init_pb(),
			np_req_data(), np_send_data(), np_remote_reset(),
			np_remote_start(), np_send_reqid(), np_set_circuit(),
			cippd_open_pb(), np_map_buf();
extern	void		np_crash_lport(), np_dealloc_buf(), np_init_port(),
			np_inv_cache(), np_log_badport(), np_notify_port(),
			np_shutdown(), np_test_lpconn(), np_update_ptype(),
			cippd_crash_pb(), cippd_remove_pb(), np_add_dg(),
			np_add_msg(), np_dealloc_dg(), np_dealloc_msg(),
			np_send_dg(), np_send_msg(), np_unmap_buf();

/* Global Variables and Data Structures.
 */
u_char		*np_black_hole		/* Black Hole Mapping Page	     */
		      = NULL;
u_long		np_bhole_pfn		/* Black Hole Page Frame Number      */
		     = 0;
NPBDDB		npbddb 			/* N_PORT Buffer Discriptor Data Base*/
		     = { 0 };		/* Head Structure		     */
NPBDDB		*np_bddb		/* N_PORT Buffer Discriptor Data Base*/
		     = &npbddb;		/* Head Structure Pointer	     */

char		*np_align_array[BDL_NUM][BDL_SIZE];

PDT		np_pdt	= {		/* N_PORT Port Dispatch Table	     */
					/* Mandatory PD Functions	     */
		    np_alloc_dg,	/* Allocate Datagram Buffer	     */
		    np_dealloc_dg,	/* Deallocate Datagram Buffer	     */
		    np_add_dg,		/* Add Datagram Buffer to Free Pool  */
		    np_remove_dg,	/* Remove Dg Buffer from Free Pool   */
		    np_alloc_msg,	/* Allocate Message Buffer	     */
		    np_dealloc_msg,	/* Deallocate Message Buffer	     */
		    np_add_msg,		/* Add Message Buffer to Free Pool   */
		    np_remove_msg,	/* Remove Msg Buffer from Free Pool  */
		    np_send_msg,	/* Send Sequenced Message	     */
		    np_map_buf,		/* Map Buffer			     */
		    np_unmap_buf,	/* Unmap Buffer			     */
		    cippd_open_pb,	/* Transition Formative Path to Open */
		    cippd_crash_pb,	/* Crash Path			     */
		    cippd_get_pb,	/* Retrieve Path Block               */
		    cippd_remove_pb,	/* Remove Path Block from Databases  */
		    np_remote_reset,	/* Reset Remote Port and System	     */
		    np_remote_start,	/* Start Remote Port and System	     */

					/* Optional PD Functions	     */
		    np_send_dg,		/* Send Datagram		     */
		    np_send_data,	/* Send Block Data		     */
		    np_req_data,	/* Request Block Data		     */
		    np_crash_lport,	/* Crash Local Port		     */
		    np_shutdown,	/* Inform Systems of Local Shutdown  */

					/* Mandatory PD Functions for CI PPD */
		    np_get_port,	/* Retrieve Port Number from Buffer  */
     ( u_long (*)())np_init_port,	/* Initialize/Re-initialize a Port   */
     ( u_long (*)())np_log_badport,	/* Log CI Originating Bad Port Number*/
		    np_send_reqid,	/* Request Remote Port Identification*/
		    np_set_circuit,	/* Set Virtual Circuit State - On/Off*/

					/* Optional PD Functions for CI PPD  */
		    np_alloc_buf,	/* Allocate Emergency Command Packets*/
     ( u_long (*)())np_dealloc_buf,	/* Deallocate Emergency Command Pkts */
     ( u_long (*)())np_init_pb,		/* Initialize a Path Block	     */
     ( u_long (*)())np_inv_cache,	/* Invalidate Port Translation Cache */
     ( u_long (*)())np_notify_port,	/* Notify Port of CI PPD Activity    */
     ( u_long (*)())np_test_lpconn,	/* Test Local Port Connectivity	     */
     ( u_long (*)())np_update_ptype	/* Update Hardware Port Type	     */
};
u_long	*citca_regoff[] = {		/* CITCA Register Offsets	      */
	    ( u_long * )CITCA_AMCSR,	/* Adapter maint cntl & status reg    */
	    ( u_long * )CITCA_CASR,	/* Channel/Adapter status register    */
	    ( u_long * )CITCA_CASRHI,	/* Channel/Adapter error status regist*/
	    ( u_long * )CITCA_CAFAR,	/* Channel/Adapter failing address reg*/
	    ( u_long * )CITCA_ABBR,	/* Adapter block base register 	      */
	    ( u_long * )CITCA_CCQ0IR,	/* Channel command queue 0 cntrl reg  */
	    ( u_long * )CITCA_CCQ1IR,	/* Channel command queue 1 cntrl reg  */
	    ( u_long * )CITCA_CCQ2IR,	/* Channel command queue 2 cntrl reg  */
	    ( u_long * )CITCA_ADFQIR,	/* Adapter dg free queue control reg  */
	    ( u_long * )CITCA_AMFQIR,	/* Adapter msg free queue control reg */
	    ( u_long * )CITCA_CASRCR,	/* Channel/Adapter status rel cntrl re*/
	    ( u_long * )CITCA_CECR,	/* Channel enable control register    */
	    ( u_long * )CITCA_CICR,	/* Channel initialization control reg */
	    ( u_long * )CITCA_AMTCR,	/* Adapter maint/sanity timer cntrl re*/
	    ( u_long * )CITCA_AMTECR,	/* Adapter maint/sanity expiration reg*/
	    ( u_long * )CITCA_AITCR,	/* Adapter intr holdoff timer cntrl re*/
};

u_long	*cimna_regoff[] = {		/* CIMNA Register Offsets	      */
	    ( u_long * )CIMNA_AMCSR,	/* Adapter maint cntl & status reg    */
	    ( u_long * )CIMNA_CASR,	/* Channel/Adapter status register    */
	    ( u_long * )CIMNA_CASRHI,	/* Channel/Adapter error status regist*/
	    ( u_long * )CIMNA_CAFAR,	/* Channel/Adapter failing address reg*/
	    ( u_long * )CIMNA_ABBR,	/* Adapter block base register 	      */
	    ( u_long * )CIMNA_CCQ0IR,	/* Channel command queue 0 cntrl reg  */
	    ( u_long * )CIMNA_CCQ1IR,	/* Channel command queue 1 cntrl reg  */
	    ( u_long * )CIMNA_CCQ2IR,	/* Channel command queue 2 cntrl reg  */
	    ( u_long * )CIMNA_ADFQIR,	/* Adapter dg free queue control reg  */
	    ( u_long * )CIMNA_AMFQIR,	/* Adapter msg free queue control reg */
	    ( u_long * )CIMNA_CASRCR,	/* Channel/Adapter status rel cntrl re*/
	    ( u_long * )CIMNA_CECR,	/* Channel enable control register    */
	    ( u_long * )CIMNA_CICR,	/* Channel initialization control reg */
	    ( u_long * )CIMNA_AMTCR,	/* Adapter maint/sanity timer cntrl re*/
	    ( u_long * )CIMNA_AMTECR,	/* Adapter maint/sanity expiration reg*/
	    ( u_long * )CIMNA_AITCR,	/* Adapter intr holdoff timer cntrl re*/
};

static CLFTAB				/* Console Logging Formating Tables  */
/* NULL entries represent events which should never be logged by the CI port
 * driver.
 */
		np_cli[] = {		/* CI Informational Event Table	     */
    { CF_RPORT,  "cable a: transitioned from bad to good" },
    { CF_RPORT,  "cable b: transitioned from bad to good" },
    { CF_RPORT,  "cables: transitioned from crossed to uncrossed" },
    { CF_LPORT,  "cable a: connectivity transitioned from bad to good" },
    { CF_LPORT,  "cable b: connectivity transitioned from bad to good" },
    { CF_INIT,   "port initialized" },
    { CF_LPORT,  "port initialized" },
    { CF_LPORT,  "port recovered power" }
},
		np_clw[] = {		/* CI Warning Event Table	     */
    { CF_RPORT,  "cable a: transitioned from good to bad" },
    { CF_RPORT,  "cable b: transitioned from good to bad" },
    { CF_RPORT,  "cables: transitioned from uncrossed to crossed" },
    { CF_LPORT,  "cable a: connectivity transitioned from good to bad" },
    { CF_LPORT,  "cable b: connectivity transitioned from good to bad" },
    { CF_UCODE,  "port microcode: not at supported revision level" },
    { CF_LPORT,  "port lost power" },
    { CF_LPORT,	 "stray interrupt" }
},
		np_clre[] = {		/* CI Remote Error Event Table	     */
    { CF_RPORT,  "remote port: state precludes communication" }
},
		np_cle[] = {		/* CI Error Event Table		     */
    { CF_NONE,   "port initialization: insufficient memory" },
    { CF_NONE,   "port initialization: microcode verification error" },
    { CF_REGS,   "port initialization: unable to start port" },
    { CF_NONE,   "cables: all failed" },
    { CF_PKT,    "received message: virtual circuit closed" },
    { CF_NONE,   "remote port: invalid state" },
    { CF_REGS,   "port initialization: unable to enter disable state" },
    { CF_REGS,   "port initialization: unable to enter enable state" },
    { CF_NONE,   "port initialization: unable to support short data page" },
},
		np_clse[] = {		/* CI Severe Error Event Table	     */
    { CF_NONE,   NULL },
    { CF_PKT,    "remote packet: invalid size" },
    { CF_PKT,    "remote packet: unrecognized port operation code" },
    { CF_PKT,    "received message: out-of-sequence" },
    { CF_PKT,    "local/remote packet: invalid destination/source" },
    { CF_PKT,    "local packet: invalid local buffer name" },
    { CF_PKT,    "local packet: invalid local buffer length" },
    { CF_NONE,    NULL },
    { CF_PKT,    "local packet: invalid size" },
    { CF_PKT,    "local packet: invalid destination" },
    { CF_PKT,    "local packet: unknown port command" },
    { CF_NONE,    NULL },
    { CF_PKT,    "local packet: unknown status" },
    { CF_PKT,    "local packet: unknown port operation code" },
    { CF_PKT,    "local packet: invalid port operation code" },
    { CF_REGS,   "port lost power" },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_NONE,   NULL },
    { CF_REGS,   "memory system error occurred" },
    { CF_NONE,   NULL },
    { CF_REGS2,  "port data structure error occurred" },
    { CF_REGS,   "port parity error occurred" },
    { CF_NONE,   NULL },
    { CF_REGS2,  "port error bit( s ) set" },
    { CF_NONE,   NULL },
    { CF_REGS,   "port sanity timer expired" },
    { CF_REGS,   "port message free queue: exhausted" },
    { CF_PKT,    "received packet: invalid port address" },
    { CF_PKT,    "received packet: invalid sequence number" },
    { CF_PKT,    "received packet: invalid port datalink address" },
    { CF_PKT,    "set circuit: insufficient VCD entries" },
    { CF_PKT,    "set circuit: insufficient RESEQ resources" },
    { CF_PKT,    "received packet: discarded VC packet" },
    { CF_PKT,    "set circuit: invalid VC generation number" },
    { CF_REGS,   "port datagram free queue: exhausted" },
},
		np_clfe[] = {		/* CI Fatal Error Event Table	     */
    { CF_NONE,   "port initialization: insufficient memory" },
    { CF_NONE,   "scs_system_id has not been set to a non-zero value" },
    { CF_NONE,   "functional microcode: unable to locate" },
    { CF_NONE,   "hardware port type: unknown" },
    { CF_NONE,   "hardware port type: functional microcode does not match" },
    { CF_NONE,   "port initialization: microcode verification error" },
    { CF_REGS,   "port initialization: unable to start port" },
    { CF_CPU,    "cpu microcode: not at required revision levels" },
    { CF_REGS,   "port is permanently unavailable" },
    { CF_PPR,    "port recognizes invalid maximum port number" },
    { CF_UCODE,  "port microcode: invalid revision level" },
    { CF_REGS2,  "port error bit( s ) set" },
},
		np_clppdse[] = {	/* CI PPD Severe Error Event Table   */
    { CF_NONE,	 NULL },
    { CF_NONE,	 NULL },
    { CF_NONE,   "attempt made to crash nonexistent path" },
    { CF_NONE,	 "port failed ci ppd sanity check" },
    { CF_NONE,	 NULL }
};
CLSTAB					/* CI Console Logging Table	     */
	np_cltab[ ES_FE + 1 ][ ESC_PPD + 1 ] = {
{					/* Severity == ES_I		     */
    {  8,	np_cli     },		/*     Subclass == ESC_PD( CI )      */
    {  0,	NULL	   },		/*     Subclass == ESC_PPD( CI PPD ) */
},{					/* Severity == ES_W		     */
    {  8,	np_clw     },		/*     Subclass == ESC_PD( CI )      */
    {  0,	NULL	   },		/*     Subclass == ESC_PPD( CI PPD ) */
},{					/* Severity == ES_RE		     */
    {  1,	np_clre    },		/*     Subclass == ESC_PD( CI )      */
    {  0,	NULL	   },		/*     Subclass == ESC_PPD( CI PPD ) */
},{					/* Severity == ES_E		     */
    {  9,	np_cle     },		/*     Subclass == ESC_PD( CI )      */
    {  0,	NULL	   },		/*     Subclass == ESC_PPD( CI PPD ) */
},{					/* Severity == ES_SE		     */
    { 43,	np_clse    },		/*     Subclass == ESC_PD( CI )      */
    {  5,	np_clppdse },		/*     Subclass == ESC_PPD( CI PPD ) */
},{					/* Severity == ES_FE		     */
    { 12,	np_clfe    },		/*     Subclass == ESC_PD( CI )      */
    {  0,	NULL       },		/*     Subclass == ESC_PPD( CI PPD ) */
},
};
u_long		np_crctable[] = {	/* CI Loopback CRC Table	     */
    0, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4, 0x4DB26158,
    0x5005713C, 0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C, 0x9B64C2B0,
    0x86D3D2D4, 0xA00AE278, 0xBDBDF21C
};

MRLTAB		cimna_mrltable[] = {	/* CIXCD Microcode Revision Table    */
/* Additions to this table may require appropriate updating of CIXCD_MAXST
 * and CIXCD_MAXFN.  When updating this table with a new functional
 * microcode revision level, add entries for each and every known self-test
 * microcode revision level.  In other words, when adding functional microcode
 * revision level n, add entries for( functional/self-test ): n/2, n/3, etc....
 * If this is not done, a fatal error is detected(  invalid microcode revision
 * level ) during local port initialization and the port is taken permanently
 * offline.  This is a rather severe penalty to pay for failing to update the
 * on-board EEPROM with new self-test microcode, especially as the port is
 * fully usable.
 *
 * Format of functional ucode field is 000.00000 
 */
	{  0x20,  0, UCODE_NOWARNING },	/* Fn:  1.0  St:    0  - NO WARNING   */
	{  0,  0, 0		  }
},
		citca_mrltable[] = {	/* CITCA Microcode Revision Table    */
	{  0x20,  0, UCODE_NOWARNING },	/* Fn:  1.0  St:    0  - NO WARNING   */
	{  0,  0, 0		  }
};
