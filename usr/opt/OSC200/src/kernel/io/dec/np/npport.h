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
 * @(#)$RCSfile: npport.h,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/07/13 17:47:45 $
 */
/*
 * derived from npport.h	5.2	(ULTRIX)	10/16/91
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		constants, data structure definitions, port command
 *		definitions, register definitions, and macros.
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
 *   23-Jul-1991	Brian Nadeau
 *	New NPORT module.
 */

/* CI Port Driver Naming Conventions.
 *
 * There are currently two N_PORT CI hardware port types.  The two types are
 * sufficiently related to one another to allow their control by a single port
 * driver.  Differences between the types also exist.  Some of these
 * differences are trivial and may be coped with through appropriate special
 * casing.  Others are not so minor and special casing is not sufficient in and
 * of itself to handle them.
 *
 * Major differences between hardware port types are ultimately based upon
 * discrepancies in port register formats.  Fortunately, not all types possess
 * unique register formats.  This allows the two hardware port types to be
 * grouped into two families based upon shared register formats:
 *
 *
 * It is this grouping into families which provides the basis for the CI port
 * driver naming conventions.  These naming conventions affect routine, macro,
 * constant, and data structure names.  They are listed below in the form of a
 * set of rules:
 *
 * 1. Global names( routines ) pertaining to all hardware ports are prefixed
 *    with "np_" as in the routine np_probe().
 * 2. Local names( constants, data structures, macros ) pertaining to all
 *    hardware ports have no specific prefix as in the constant MAX_CABLES, the
 *    macro Comp_loopback(), and the data structure VCD.
 * 3. Names pertaining to a specific hardware port type are are prefaced with
 *    the specific port type itself as in the routine cimna_isr(), and the
 *    constant CIMNA_MAXFN.
 */

#ifndef _NPPORT_H_
#define _NPPORT_H_

/* CI Constants.
 */
					/* Cable Update Type Codes	     */
#define	CABLE_GB		 1	/* Check good->bad transitions	     */
#define	CABLE_BG		 2	/* Check bad->good transitions	     */
#define	CABLE_CROSSED		 3	/* Check crossing transitions	     */
#define	CABLE_LB_GB		 4	/* Chk loopback good->bad transitions*/
#define	CABLE_LB_BG		 5	/* Chk loopback bad->good transitions*/

					/* Cluster size definitions	     */
#define	CSZ_8		 	 8	/* Cluster size = 8 systems 	     */
#define	CSZ_16			16	/* Cluster size = 16 systems	     */
#define	CSZ_32			32	/* Cluster size = 32 systems	     */
#define	CSZ_64			64	/* Cluster size = 64 systems	     */
#define	CSZ_128		       128	/* Cluster size = 128 systems	     */

					/* Console Logging Format Codes	     */
				 	/* 0 - No optional information logged*/
				 	/* 1 - CI PPD special format code    */
				 	/* 2 - CI PPD special format code    */
				 	/* 3 - CI PPD special format code    */
				 	/* 4 - CI PPD special format code    */
				 	/* 5 - Rem port station addr logged  */
#define	CF_LPORT		 6	/* Local port station address logged */
#define	CF_PPR			 7	/* PPR port register logged	     */
#define	CF_REGS			 8	/* Port registers logged	     */
#define	CF_REGS2		 9	/* Port registers logged	     */
#define	CF_BIREGS		10	/* BIIC registers logged	     */
#define	CF_PKT			11	/* CI packet fields logged	     */
#define	CF_UCODE		12	/* Microcode revision levels logged  */
#define	CF_CPU			13	/* CPU revision levels logged	     */
#define	CF_INIT			14	/* Initial lport init info logged    */

					/* Device Attention Logging Modifiers*/
#define	LOG_NOREGS		 0	/* Do not log device registers       */
#define	LOG_REGS		 1	/* Log device registers		     */

					/* CI Informational Event Codes	     */
/* CI informational events are either local or remote port specific.  There are
 * currently no path specific CI informational events.  ci_console_log(),
 * ci_cli[], and ci_cltab[][] must be updated to reflect new additions.
 * ci_log_dev_attn() must be updated when a new local port specific CI
 * informational event is added.  ci_log_packet() must be updated when a new
 * remote port specific CI informational event is added.
 *
 * The following CI informational events are local port specific but may NOT
 * have the local port crash severity modifier( ESM_LPC ) applied:
 *
 *	LPORT_INIT, LPORT_REINIT, POWERUP
 *
 * The following CI informational events are remote port specific:
 *
 *	CABLE0_BG, CABLE1_BG, CABLES_CU, CABLE0_LBBG, CABLE1_LBBG
 *
 * NOTE: Local port specific CI informational events are never candidates for
 *	 application of the local port crash severity modifier( ESM_LPC ).
 *
 * NOTE: CABLE0_LBBG, CABLE1_LBBG are classified as remote port specific CI
 *	 informational events even though the identity of the remote port is
 *	 actually the local port.
 *
 * NOTE: Choose an appropriate console formatting code( CF ) which includes
 *	 displaying of the local port station address when updating ci_cli[]
 *	 with a new local port specific CI informational event.  Choose one
 *	 which includes displaying of the remote port station address when
 *	 updating ci_cli[] with a new remote port specific CI informational
 *	 event.
 */
#define	I_CABLE0_BG	( PDI | 0x01 )	/* Cable 0 bad->good transition	     */
#define	I_CABLE1_BG	( PDI | 0x02 )	/* Cable 1 bad->good transition	     */
#define	I_CABLES_CU	( PDI | 0x03 )	/* Cables crossed->uncrossed	     */
#define	I_CABLE0_LBBG	( PDI | 0x04 )	/* Loopback Cable 0 bad->good transit*/
#define	I_CABLE1_LBBG	( PDI | 0x05 )	/* Loopback Cable 1 bad->good transit*/
#define	I_LPORT_INIT	( PDI | 0x06 )	/* Initial local port initialization */
#define	I_LPORT_REINIT	( PDI | 0x07 )	/* Local CI port initialization	     */
#define	I_POWERUP	( PDI | 0x08 )	/* Power restored to local port	     */

					/* CI Warning Event Codes	     */
/* CI warning events are either local or remote port specific.  There are
 * currently no path specific CI warning events.  ci_console_log(),
 * ci_cltab[][], and ci_clw[] must be updated to reflect new additions.
 * ci_log_dev_attn() must be updated when a new local port specific CI warning
 * event is added.  ci_log_packet() must be updated when a new remote port
 * specific CI warning event is added.
 *
 * The following CI warning events are local port specific but may NOT have the
 * local port crash severity modifier( ESM_LPC ) applied:
 *
 *	UCODE_WARN, POWER, STRAY
 *
 * The following CI warning events are remote port specific:
 *
 *	CABLE0_GB, CABLE1_GB, CABLES_UC, CABLE0_LBGB, CABLE1_LBGB
 *
 * NOTE: Local port specific CI warning events are never candidates for
 *	 application of the local port crash severity modifier( ESM_LPC ).
 *
 * NOTE: CABLE0_LBGB, CABLE1_LBGB are classified as remote port specific CI
 *	 warning events even though the identity of the remote port is actually
 *	 the local port.
 *
 * NOTE: Choose an appropriate console formatting code( CF ) which includes
 *	 displaying of the local port station address when updating ci_clw[]
 *	 with a new local port specific CI warning event.  Choose one which
 *	 includes displaying of the remote port station address when updating
 *	 ci_clw[] with a new remote port specific CI warning event.
 */
#define	W_CABLE0_GB	( PDW  | 0x01 )	/* Cable 0 good->bad transition	     */
#define	W_CABLE1_GB	( PDW  | 0x02 )	/* Cable 1 good->bad transition	     */
#define	W_CABLES_UC	( PDW  | 0x03 )	/* Cables uncrossed->crossed	     */
#define	W_CABLE0_LBGB	( PDW  | 0x04 )	/* Loopback Cable 0 good->bad transit*/
#define	W_CABLE1_LBGB	( PDW  | 0x05 )	/* Loopback Cable 1 good->bad transit*/
#define	W_UCODE_WARN	( PDAW | 0x06 )	/* Out-of-revision microcode	     */
#define	W_POWER		( PDW  | 0x07 )	/* Power failure occurred	     */
#define	W_STRAY		( PDW  | 0x08 )	/* Stray interrupt received	     */

					/* CI Remote Error Event Codes	     */
/* CI remote error events are always remote port specific.  There are currently
 * no local port or path specific CI remote error events.  ci_console_log(),
 * ci_log_packet(), ci_clre[], and ci_cltab[][] must be updated to reflect new
 * additions.
 *
 * The following CI remote error events are remote port specific:
 *
 *	RPORT_STATE
 *
 * NOTE: Choose an appropriate console formatting code( CF ) which includes 
 *	 displaying of the remote port station address when updating ci_clre[]
 *	 with a new CI remote error event.
 */
#define	RE_RPORT_STATE	( PDRE | 0x01 )	/* Improper remote port state	     */

					/* CI Error Event Codes		     */
/* CI error events are either local port or path specific.  There are
 * currently no remote port specific CI error events.  ci_console_log(),
 * ci_cle[], and ci_cltab[][] must be updated to reflect new additions.
 * ci_log_dev_attn() must be updated when a new local port specific CI error
 * event is added.  ci_log_packet() must be updated when a new path specific CI
 * error event is added.
 *
 * The following CI error events are local port specific but may NOT have the
 * local port crash severity modifier( ESM_LPC ) applied:
 *
 *	NOMEM, UCODE_LOAD, UCODE_START, DISABLE_STATE, ENABLE_STATE,
 *	SHORT_DPAGE
 *
 * The following CI error events are path specific but may NOT have the path
 * crash severity modifier( ESM_PC ) applied:
 *
 *	NOCABLES, CLOSEDVCD, RPORTSTATE
 *
 * Path specific information is always displayed by default during console
 * logging of path specific CI error events.  The local port station address is
 * always displayed by default during console logging of local port specific CI
 * error events.
 *
 * NOTE: Path and local port specific CI error events are NEVER candidates for
 *	 application of the path( ESM_PC ) or local port( ESM_LPC ) crash
 *	 severity modifiers.
 *
 * NOTE: When updating ci_cle[] with a new CI error event bear in mind the
 *	 information displayed by default when choosing an appropriate console
 *	 formatting code( CF ).
 */
#define	E_NOMEM		( PDE | 0x01 )	/* Insufficient memory to init port  */
#define	E_UCODE_LOAD	( PDE | 0x02 )	/* Unable to verify microcode load   */
#define	E_UCODE_START	( PDE | 0x03 )	/* Unable to start microcode	     */
#define	E_NOCABLES	( PDE | 0x04 )	/* Closing vc due to cable failures  */
#define	E_CLOSEDVCD	( PDE | 0x05 )	/* Message on closed VC received     */
#define	E_RPORTSTATE	( PDE | 0x06 )	/* Remote port in invalid state	     */
#define	E_DISABLE_STATE ( PDE | 0x07 )	/* Unable to enter the disable state */
#define	E_ENABLE_STATE  ( PDE | 0x08 )	/* Unable to enter the enable state  */
#define	E_SHORT_DPAGE	( PDE | 0x09 )	/* Unable to support short data page */

					/* CI Severe Error Event Codes	     */
/* CI severe error events are either local port or path specific.  There are
 * currently no remote port specific CI severe error events.  ci_console_log(),
 * ci_clse[], and ci_cltab[][] must be updated to reflect new additions.
 * Either ci_log_dev_attn() or ci_log_packet() must be updated when a new local
 * port specific CI severe error event is added, depending upon the nature of
 * the event.  ci_log_packet() must be updated when a new path specific CI
 * severe error event is added.  ci_crash_lport() also requires updating when
 * the new addition is a candidate for application of the local port crash
 * severity modifier( ESM_LPC ).
 *
 * The following CI severe error events are local port specific and may have
 * the local port crash severity modifier( ESM_LPC ) applied:
 *
 *	INVBNAME, INVBSIZE, BACCVIO, INVLPKTSIZE, INVDPORT, UNKCMD, ABORTPKT,
 *	UNKSTATUS, UNKOPCODE, INVOPCODE, POWER, MSE, 
 *	DSE, PARITY, PORTERROR, SANITYTIMER, MFQE
 *
 * The following CI severe error events are local port specific but may NOT
 * have the local port crash severity modifier( ESM_LPC ) applied:
 *
 *	BADPORTNUM
 *
 * The following CI severe error events are path specific but may NOT have the
 * path crash severity modifier( ESM_PC ) applied:
 *
 *	INVRPKTSIZE, UNRECPKT, OSEQMSG
 *
 * Path specific information is always displayed by default during console
 * logging of path specific CI severe error events.  The local port station
 * address is always displayed by default during console logging of local port
 * specific CI severe error events.
 *
 * NOTE: Path specific CI severe error events are NEVER candidates for
 *	 application of the path crash severity modifier( ESM_PC ).  Not all
 *	 local port specific CI severe error events are candidates for
 *	 application of the local port crash severity modifier( ESM_LPC ).
 *
 * NOTE: When updating np_clse[] with a new CI severe error event bear in mind
 *	 the information displayed by default when choosing an appropriate
 *	 console formatting code( CF ).
 *
 * NOTE: These error codes or in common with the CI VAX code with the
 *	 exception of two new error codes SE_INVCGN and SE_DFQE.
 *	 Holes were left where error codes did not make sense for N_PORT.
 *	 Updated np_clse[] with NULL strings where needed.
 *
 */
#define	SE_INVRPKTSIZE ( PDSE  | 0x02 )	/* received invalid sized packet     */
#define	SE_UNRECPKT    ( PDSE  | 0x03 )	/* Unrecognized packet received	     */
#define	SE_OSEQMSG     ( PDSE  | 0x04 )	/* Out-of-sequenced message received */
#define	SE_BADPORTNUM  ( PDSE  | 0x05 )	/* Invalid port number encountered   */
#define	SE_INVBNAME    ( PDSE  | 0x06 )	/* Invalid local buffer name	     */
#define	SE_INVBSIZE    ( PDSE  | 0x07 )	/* Local buffer length violation     */
#define	SE_INVLPKTSIZE ( PDSE  | 0x09 )	/* local packet size violation	     */
#define	SE_INVDPORT    ( PDSE  | 0x0A )	/* Invalid destination port	     */
#define	SE_UNKCMD      ( PDSE  | 0x0B )	/* Unknown local port command	     */
#define	SE_UNKSTATUS   ( PDSE  | 0x0D )	/* Unknown status in packet	     */
#define	SE_UNKOPCODE   ( PDSE  | 0x0E )	/* Unknown opcode in packet	     */
#define	SE_INVOPCODE   ( PDSE  | 0x0F )	/* Invalid opcode in packet	     */
#define	SE_POWER       ( PDSE  | 0x10 )	/* Power failure occurred	     */
#define	SE_MSE         ( PDSE  | 0x1B )	/* Memory system error reported	     */
#define	SE_DSE	       ( PDSE  | 0x1D )	/* Data structure error reported     */
#define	SE_PARITY      ( PDSE  | 0x1E )	/* Parity error reported	     */
#define	SE_PORTERROR   ( PDSE  | 0x20 )	/* Miscellaneous port error reported */
#define	SE_SANITYTIMER ( PDSE  | 0x22 )	/* Sanity timer expired		     */
#define	SE_MFQE        ( PDSE  | 0x23 )	/* Message free queue exhausted	     */
#define	SE_INVPA       ( PDSE  | 0x24 )	/* Invalid port addr. in recv. pkt.  */
#define	SE_INVSN       ( PDSE  | 0x25 )	/* Invalid sequence number received  */
#define	SE_INVDDL      ( PDSE  | 0x26 )	/* Invalid datalink address received */
#define	SE_IRESVCD     ( PDSE  | 0x27 )	/* Insufficient VCD entries	     */
#define	SE_IRESEQ      ( PDSE  | 0x28 )	/* Insuff. RESEQ resources (RDP only)*/
#define	SE_DISCVCPKT   ( PDSE  | 0x29 )	/* Discarded VC packet ( RDP only )  */
#define	SE_INVCGN      ( PDSE  | 0x2A )	/* invalid VC generation number      */
#define	SE_DFQE        ( PDSE  | 0x2B )	/* Datagram free queue exhausted     */

					/* CI Fatal Error Event Codes	     */
/* CI fatal error events are always local port specific.  There are currently
 * no path or remote port specific CI fatal error events.  ci_console_log(),
 * ci_clfe[], and ci_cltab[][] must be updated to reflect new additions. 
 * ci_log_initerr() must be updated when the new local port specific CI fatal
 * error event may occur during initial probing of the local CI port.
 * ci_log_dev_attn() must be updated when the new local port specific CI fatal
 * error event may NOT occur during initial probing of the local CI port.
 * ci_crash_lport() also requires updating when the new addition is a candidate
 * for application of the local port crash severity modifier( ESM_LPC ).
 *
 * The following CI fatal error events are local port specific and may have the
 * local port crash severity modifier( ESM_LPC ) applied:
 *
 *	NOCI, BADMAXPORT, BADUCODE, PORTERROR
 *
 * The following CI fatal error events are local port specific but may NOT have
 * the local port crash severity modifier( ESM_LPC ) applied:
 *
 *	INIT_NOMEM, INIT_ZEROID, INIT_NOUCODE, INIT_UNKHPT, INIT_MISMTCH,
 *	UCODE_LOAD, UCODE_START, CPU
 *
 * The local port station address is displayed by default during console
 * logging of all CI fatal error events.
 *
 * NOTE: Not all local port specific CI fatal error events are candidates for
 *	 application of the local port crash severity modifier( ESM_LPC ).
 *
 * NOTE: When updating ci_clfe[] with a new CI fatal error event bear in mind
 *	 the information displayed by default when choosing an appropriate
 *	 console formatting code( CF ).
 */
#define	FE_INIT_NOMEM	( PDFE | 0x01 )	/* Init - insufficient memory        */
#define	FE_INIT_ZEROID	( PDFE | 0x02 )	/* Init - zero system id number      */
#define	FE_INIT_NOUCODE	( PDFE | 0x03 )	/* Init - can't find CI microcode    */
#define	FE_INIT_UNKHPT	( PDFE | 0x04 )	/* Init - unknown hardware port type */
#define	FE_INIT_MISMTCH	( PDFE | 0x05 )	/* Init - ucode-port type mismatch   */
#define	FE_UCODE_LOAD	( PDFE | 0x06 )	/* Unable to verify microcode load   */
#define	FE_UCODE_START	( PDFE | 0x07 )	/* Unable to start port		     */
#define	FE_CPU		( PDFE | 0x08 )	/* Init - CPU ucode not at rev level */
#define	FE_NOCI		( PDFE | 0x09 )	/* CI adapter permanently absent     */
#define	FE_BADMAXPORT	( PDFE | 0x0A )	/* Invalid maximum port number	     */
#define	FE_BADUCODE	( PDFE | 0x0B )	/* Invalid CI ucode revision levels  */
#define	FE_PORTERROR	( PDFE | 0x0C )	/* Misc fatal port error reported    */

					/* Loadable Microcode Types	     */
#define	UCODE_CI780		 1	/* CI780/CI780/CIBCI Functional Ucode*/
#define	UCODE_CIBCA		 2	/* CIBCA-AA Functional Microcode     */

					/* Local Port Mapping Constants      */
#define	MAP_REGS		 0	/* Map only the I/O adapter space    */
#define	MAP_FULL		 1	/* Map fully the local CI port	     */

					/* OPC field command/response codes  */
#define SNDPM			 0	/* Send physical media packet        */
#define PMSNT			 0 	/* Physical media packet sent        */
#define PMREC			 1	/* Physical media packet received    */
					/* Msg/Dg received		     */
#define RETQE			 2	/* Return Msg/Dg queue elements      */
#define QERET			 2	/* Msg/Dg queue elements returned    */
#define PURGQ			 3	/* Purge queues			     */
#define QPURG			 3	/* Queues purged                     */
#define INVTC			 4	/* Invalidate translation cache      */
#define TCINV			 4	/* Trnalation cache invalidated      */
#define RDCNT			 5	/* Read count			     */
#define CNTRD			 5	/* Count read			     */
#define SETCHNL			 6	/* Set channel			     */
#define CHNLSET			 6	/* Channel set			     */
#define SETCKT			 7	/* Set circuit			     */
#define CKTSET			 7	/* Circuit set			     */

					/* CI_OPC field command/respnse codes*/
#define	SNDDG	         	 1	/*  Send datagram		     */
#define	DGSNT		 	 1	/*  Datagram sent		     */
#define	DGREC		 	 1	/*  Datagram received		     */
#define	SNDMSG		 	 2	/*  Send message		     */
#define	MSGSNT		 	 2	/*  Message sent		     */
#define	MSGREC		 	 2	/*  Message received		     */
#define	RETCNF			 3	/* Returned confirm - UNUSED	     */
#define	CNFRET			 3	/* Confirm returned - UNUSED	     */
#define	CNFREC			 3	/* Confirm received		     */
#define	MCNFREC			 4	/* Maintenance confirm recvd - UNUSED*/
#define	REQID			 5	/* Request identification	     */
#define	IDREQ			 5	/* Identification requested	     */
#define	SNDRST			 6	/* Send reset 			     */
#define	RSTSNT			 6	/* Reset sent 			     */
#define	SNDSTRT			 7	/* Send start			     */
#define	STRTSNT			 7	/* Start send			     */
#define REQDAT0			 8	/* Request data @ priority 0 	     */
#define DATREQ0			 8	/* Data requested @ priority 0       */
#define REQDAT1			 9	/* Request data @ priority 1	     */
#define DATREQ1			 9	/* Data requested @ priority 1	     */
#define REQDAT2			10	/* Request data @ priority 2 	     */
#define DATREQ2			10	/* Data requested @ priority 2       */
#define	IDREC			11	/* Identification received	     */
#define	REQPS			12	/* */
#define	PSREQ			12	/* */
#define	SNDLB			13	/* Send loopback		     */
#define	LBSNT			13	/* Loopback sent		     */
#define	LBREC			13	/* Loopback received                 */
#define	REQMDAT			14	/* Request maintenance data - UNUSED */
#define	MDATREQ			14	/* Maintenance data requested -UNUSED*/
#define	RETPS			15	/* Returned PS			     */
#define	SNDDAT			16	/* Send data			     */
#define	DATSNT			16	/* Data sent			     */
#define	SNTREC			16	/* Sent data received		     */
#define	RETDAT			17	/* Return data - UNUSED		     */
#define	DATRET			17	/* Data returned - UNUSED	     */
#define	DATREC			17	/* Data received        	     */
#define	SNDMDAT			18	/* Send maintenance data - UNUSED    */
#define	MDATSNT			18	/* Maintenance data sent - UNUSED    */
#define	MDATREC			19	/* Maintenance data received - UNUSED*/

					/* Port Driver Panic Strings	     */
#define	PANIC_BADUNMAP	"ci\t- invalid unmapping of local port\n"
#define	PANIC_CABLE	"ci\t- unknown cable status check requested\n"
#define	PANIC_HPT	"ci\t- unknown/invalid hardware port type\n"
#define	PANIC_IC	"ci\t- unknown interconnect type\n"
#define	PANIC_MAP	"ci\t- attempting to map already mapped adapter\n"
#define	PANIC_NOSCPKT	"ci\t- no set circuit off command packet\n"
#define	PANIC_NOTCPKT  "ci\t- no invalidate translation cache command packet\n"
#define	PANIC_ONBOARD	"ci\t- attempting to load unnecessary microcode\n"
#define	PANIC_PCCBFB	"ci\t- invalid pccb fork block\n"
#define	PANIC_REQLPC	"ci\t- panic requested on all local port failures\n"
#define	PANIC_UNKCF	"ci\t- unknown console logging formatting code\n"
#define	PANIC_UNKCODE	"ci\t- unknown/invalid event code\n"
#define	PANIC_UNKLPC	"ci\t- unknown local port crash reason\n"
#define	PANIC_UNMAP	"ci\t- attempting to unmap already unmapped adapter\n"


					/* Status Error Type		     */
#define	T_OK			00	/* Status is OK			     */
#define	T_PSVIO		 	01	/* Packet size violation	     */
#define	T_INVBNAME		02	/* Invalid buffer name		     */
#define	T_INVBSIZE		03	/* Buffer length violation	     */
					/* Transmit packet errors only	     */
#define	T_UCMD			16	/* Unrecognized port command	     */
#define	T_DPORT			17	/* Invalid destination port	     */
#define	T_VCC			18	/* VC closed before command execution*/
#define	T_INVCGN		19	/* Invalid VC generation number      */
#define	T_NOPATH		20	/* No path			     */
#define	T_IRESVCD		21	/* Insufficient VCD resources        */
#define	T_IRESEQ		22	/* Insufficient reseq. resources RDP */
					/* Receive packet errors only	     */
#define	T_UPKT			32	/* Unrecognized packet		     */
#define	T_INVPA			33	/* Invalid port addr. in recv. pkt.  */
#define	T_INVDDL		34	/* Invalid destination datalink addr */
#define	T_INVSN		 	35	/* Invalid sequence number on VC     */
#define	T_DISCVCPKT		36	/* Discarded VC pkt RDP adap. only   */

					/* Miscellaneous Constants	     */
#define ADAP_QUE_STP		 7	/* Number of N_PORT adap que stoppers*/
#define CHNL_QUE_STP		 7	/* Number of N_PORT chnl que stoppers*/

/* CI Port Specific Constants.
 */
					/* CIMNA/CITCA Cache Sizes	     */
#define	CI_DG_CACHE		 4 	/* Size of datagram cache	     */
#define	CI_MSG_CACHE		 4	/* Size of message cache	     */

					/* Microcode Related Constants       */
#define	CITCA_MAXFN		 0	/* Current CITCA max fn ucode level  */
#define	CITCA_MAXST		 0	/* Current CITCA max st ucode level  */
#define	CIMNA_MAXFN		 0	/* Current CIMNA max fn ucode level  */
#define	CIMNA_MAXST		 0	/* Current CIMNA max st ucode level  */

					/* CIMNA Register Offsets	      */
#define	CIMNA_XCOMM	    0x0010L	/* XMI Comm. register                 */
#define	CIMNA_PSCR	    0x0014L	/* Adap scan cntrl register	      */
#define	CIMNA_PSDR	    0x0018L	/* Adap scan data  reg                */
#define	CIMNA_AMCSR	    0x001CL	/* Adap maintenance cntl & status reg */
#define	CIMNA_PDCSR	    0x0020L	/* Adap diag. control & status reg    */
#define	CIMNA_XFAER	    0x002CL	/* XMI failing address lw1 reg        */
#define	CIMNA_CASR	    0x0030L	/* Channel/adap status register       */
#define	CIMNA_CASRHI	    0x0034L	/* Channel/adap error status register */
#define	CIMNA_ASNR	    0x0038L	/* Adap serial number reg             */
#define	CIMNA_AIDR	    0x0040L	/* Adap interrupt destination reg     */
#define	CIMNA_AMIVR	    0x0048L	/* Adap misc interrupt vector reg     */
#define	CIMNA_ACIVR	    0x0050L	/* Adap completion intr vector reg    */
#define	CIMNA_CAFAR	    0x0058L	/* Channel/adap failing address regis */
#define	CIMNA_CIRR	    0x0060L	/* Completion intr release reg 	      */
#define	CIMNA_ASUBR	    0x0068L	/* Adapter substitute register 	      */
#define	CIMNA_ABBR	    0x0080L	/* Adapter block base register 	      */
#define	CIMNA_CCQ2IR	    0x0088L	/* Channel command queue 0 control reg*/
#define	CIMNA_CCQ1IR	    0x0090L	/* Channel command queue 1 control reg*/
#define	CIMNA_CCQ0IR	    0x0098L	/* Channel command queue 2 control reg*/
#define	CIMNA_ADFQIR	    0x00A0L	/* Adap dg free queue control reg     */
#define	CIMNA_AMFQIR	    0x00A8L	/* Adap msg free queue control reg    */
#define	CIMNA_CASRCR	    0x00B0L	/* Channel/adap status release control*/
#define	CIMNA_CICR	    0x00B8L	/* Channel initialization control reg */
#define	CIMNA_CECR	    0x00C0L	/* Channel enable control register    */
#define	CIMNA_AMTCR	    0x00C8L	/* Adap maintenance timer control reg */
#define	CIMNA_AMTECR	    0x00D0L	/* Adap maint timer expiration ctl reg*/
#define	CIMNA_AITCR	    0x00D8L	/* Adap maint timer expiration ctl reg*/

					/* CITCA Register Offsets 	     */
#define CITCA_TDEV	    0x0000	/* TC device type 		     */
#define CITCA_TBER	    0x0008	/* TC bus error register    	     */
#define CITCA_PSCR	    0x0010	/* Port scan control register        */
#define CITCA_PSDR	    0x0018	/* Port scan data register           */
#define CITCA_PDCSR	    0x0020	/* Port diag. control & status reg.  */
#define CITCA_CASR	    0x0028	/* Channel adapter status low reg.   */
#define CITCA_CASRHI	    0x002C	/* Channel adapter status hi reg.    */
#define CITCA_CAFAR	    0x0030	/* Channel adapter failing addr. reg.*/
#define CITCA_ASNR	    0x0038	/* Adapter serial number register    */
#define CITCA_ABBR	    0x0080	/* Adapter block base register       */
#define CITCA_CCQ2IR   	    0x0088	/* Channel command que 2 insert reg. */
#define CITCA_CCQ1IR   	    0x0090	/* Channel command que 1 insert reg. */
#define CITCA_CCQ0IR   	    0x0098	/* Channel command que 0 insert reg. */
#define CITCA_ADFQIR   	    0x00A0	/* Adapter DG free  que insert reg.  */
#define CITCA_AMFQIR   	    0x00A8	/* Adapter MSG free que insert reg.  */
#define CITCA_CASRCR	    0x00B0	/* Channel adapter status release reg.*/
#define CITCA_CICR	    0x00B8	/* Channel initialize control reg.   */
#define CITCA_CECR	    0x00C0	/* Channel enable control reg.       */
#define CITCA_AMTCR	    0x00C8	/* Adap. maint. sanity timer cntl reg.*/
#define CITCA_AMTECR	    0x00D0	/* Adap. maint. sanity timer exp. reg.*/
#define CITCA_AITCR	    0x00D8	/* Adapter intr. holdoff timer reg.*/
#define CITCA_AMCSR	    0x00E0	/* Adapter maint. control status reg.*/

					/* Miscellaneous Constants	     */
#define	MAX_CABLES		 2	/* Maximum cable number		     */

/* CI Register Definitions.
 */
					/* CIBCA/CIMNA BIIC/XMI Device Type  */
					/*  Register Port/Hardware Revision  */
					/*  Field Mask Bits		     */
#define	CIBX_DEV_UCODE	  0x800000	/* Port microcode is onboard	     */

					/* Port Control Register Mask Bits   */
#define	CICR_CIC	0x00000001	/* Port initialize control	     */
#define	CECR_CEC	0x00000001	/* Port enable control		     */
#define	AMTCR_MTC	0x00000001	/* Port Maintenance timer control    */
#define	CSRCR_CSRC	0x00000001	/* Port status reg release control   */

					/* CIMNA Port Maintenance Control */
#define	CIMNA_PMCS_MIN	0x00000001	/* Maintence initialize		     */
#define	CIMNA_PMCS_STD	0x00000002	/* Maintence sanity timer disable    */
#define	CIMNA_PMCS_IE	0x00000004	/* interrupt enable	     */
#define	CIMNA_PMCS_CPDE	0X00000010	/* CPU no responce error	     */
#define	CIMNA_PMCS_CPER	0X00000020	/* CPU error status	  	    */
#define	CIMNA_PMCS_PRER	0X00000040	/* Port read error responce	     */
#define	CIMNA_PMCS_PWER	0X00000080	/* Port write error responce	     */
#define	CIMNA_PMCS_STUF	0X00000100	/* ustack underflow error	     */
#define	CIMNA_PMCS_STOF	0x00000200	/* ustack overflow error	     */
#define	CIMNA_PMCS_YRPE	0X00000400	/* Y register parity error	     */
#define	CIMNA_PMCS_XRPE	0X00000800	/* X register parity error	     */
#define	CIMNA_PMCS_IBPE	0X00001000	/*  error	     */
#define	CIMNA_PMCS_CSPE	0X00002000	/*  error	     */
#define	CIMNA_PMCS_PRIBPE 0X00004000	/*  error		     */

#define	CIMNA_ASUB_DQE	0x00000001	/* DG free queue exhausted intr enable*/

					/* CITCA Port Maintenance Control */
#define	CITCA_PMCS_MIN	0x00000001	/* Maintence initialize		     */
#define	CITCA_PMCS_STD	0x00000002	/* Maintence sanity timer disable    */
#define	CITCA_PMCS_DQE	0x00000004	/* DG free queue exhausted intr enable*/
#define	CITCA_PMCS_IE	0x00000008	/* interrupt enable	     */
#define	CITCA_PMCS_CPER	0X00000020	/* CPU error status	  	     */
#define	CITCA_PMCS_STUF	0X00000100	/* ustack underflow error	     */
#define	CITCA_PMCS_STOF	0x00000200	/* ustack overflow error	     */
#define	CITCA_PMCS_YRPE	0X00000400	/* Y register parity error	     */
#define	CITCA_PMCS_XRPE	0X00000800	/* X register parity error	     */
#define	CITCA_PMCS_IBPE	0X00001000	/*  error	     */
#define	CITCA_PMCS_CSPE	0X00002000	/*  error	     */
#define	CITCA_PMCS_PRIBPE 0X00004000	/*  error		     */

					/* Common Maint Control and	     */
					/*  Status Register Mask Bits	     */
#define	AMCSR_MIN	0x00000001	/* Maintenance timer initialize	     */
#define	AMCSR_STD	0x00000002	/* Maintenance sanity timer disable  */
#define	AMCSR_DQE	0x00000004	/* DG free queue exhausted intr enable*/
#define	AMCSR_IE	0x00000008	/* interrupt enable	     */


					/* CITCA/CIMNA Chnl Status Register  */
#define CSR_AMFQE  	0x00000001	/* Adap. MSG free queue exhausted    */
#define CSR_ADFQE  	0x00000002	/* Adap. DG free queue exhausted     */
#define CSR_DSE    	0x00000004	/* Data structure error              */
#define CSR_MSE    	0x00000008	/* Memory system error               */
#define CSR_CAC    	0x00000010	/* Abnormal condition error          */
#define CSR_STE    	0x00000020	/* Sanity timer expiration error     */
#define CSR_CIC    	0x00000040	/* Adapter initialization complete   */
#define CSR_CEC    	0x00000080	/* Adapter enable complete           */
#define CSR_ASIC   	0x00000100	/* Adapter single completion intr.   */
#define CSR_UNIN   	0x00000200	/* Adapter single uninitialized      */
#define CSR_CME    	0x80000000	/* Maintenance error                 */
#define CSR_ERRS   	0xFFFFFE3F	/* CSR error bits                    */

					/* XMI Error Register Bit Masks	     */
#define	CIMNA_XBE_TTO	0x00000004	/* CIMNA XBE disable TTO 	     */
#define	CIMNA_XBE_ERRS	0x88F7B400	/* CIMNA XBE errors mask	     */
#define	CIMNA_XBE_HARDE	0x00F7B400	/* CIMNA XBE hard errors mask	     */
#define	CIMNA_XBE_SOFTE	0x08000000	/* CIMNA XBE soft errors mask	     */

					/* Turbochannel bus error register   */
#define CITCA_TC_PE     0x00002022	/* TC bus parity error bits	     */
#define CITCA_TC_ERR    0x00005550	/* TC bus error bits	     	     */
#define CITCA_TC_STF    0x00010000	/* TC bus self test failed bit       */
#define CITCA_TC_NSES   0x00020000	/* TC bus error node specific err bit*/
#define CITCA_TC_DTEC   0x04000000	/* TC bus disable TC error check bit */
#define CITCA_TC_DTPC   0x08000000	/* TC bus disable TC parity check bit*/
#define CITCA_TC_DTAC   0x10000000	/* TC bus disable TC ack check bit   */
#define CITCA_TC_NRST   0x40000000	/* TC bus node reset bit             */
#define CITCA_TC_ES     0x80000000	/* TC bus error summary bit          */

					/* Adapter PB Ibuf_len Mask Bits */
#define	FCN_IBUF_LEN	0x0FFF0000	/* IBUF_LEN of port	*/

					/* Adapter PB type field	     */
					/*  Mask Bits 		             */
#define	TYPE_CI		0x00000001	/* Media adapter type - 1 = CI 	     */
#define TYPE_RDPS	0x00000010	/* RDP adapter support  	     */
#define TYPE_ESS	0x00000020	/* Explicit state supported in VCD   */
#define	TYPE_EAF	0x00000040	/* Explicit address format	     */
#define TYPE_SDP	0x00000080	/* Short data page 4k supported */
#define TYPE_SIC	0x00000100	/* Single intr completion */


#define FCN_EXT2_MMV 	0x80000000	/* Member map valid 		     */
#define FCN_EXT2_EAS 	0x00008000	/* Explicit address support	     */
#define FCN_EXT2_FSN 	0x00004000	/* Full sequence numbers	     */
#define FCN_EXT2_RDP 	0x00002000	/* Resequence dual path  	     */
#define FCN_EXT2_NADP 	0x00001000	/* Non-adp adapter support	     */
#define FCN_EXT2_ONI 	0x00000800	/* Order not important support	     */
#define FCN_EXT2_NMEMS 	0x000000FF	/* Number of members in pgrp <= 64   */
#define FCN_EXT2_EFR 	0x0000E000	/* MASK EAS,FSN,RDP      	     */
#define FCN_EXT2_FR 	0x00006000	/* MASK FSN,RDP		      	     */

#define SHORT_DATA_PAGE		 1	/* Request for short data page 	     */


/* CI Data Structure Definitions.
 */
typedef struct		  {		/* Loopback CRC Table		     */
    u_char			:  8;	/* MBZ				     */
    u_char	lblen7;			/* Loopback data size plus 7	     */
    u_char	port_num;		/* Local port number		     */
    u_char	not_port_num;		/* One's complement( local port num )*/
    u_char	lport_addr;		/* Local port station address	     */
    u_char	opcode;			/* Value of SNDLB opcode	     */
    u_char			:  8;	/* MBZ				     */
    u_char	lbdata[ LBDSIZE ];	/* Loopback data		     */
} LBCRC;

					/* Microcode Revision Level Table    */
typedef struct _mrltab {		/*  Entry			     */
    u_char	 ram;			/* RAM revision level		     */
    u_char	 rom;			/* PROM/EEPROM revision level	     */
    u_short	 warn;			/* Warning message flag		     */
#define	UCODE_NOWARNING		0	/*  No warning message required	     */
#define	UCODE_WARNING		1	/*  Warning message required	     */
} MRLTAB;

typedef	struct _vcd	{		/* Virtual Circuit Descriptor	     */
    u_int	sseq_num	:  3;	/* Sending sequence number   	     */
    u_int	rseq_num	:  3;	/* Sending sequence number   	     */
    u_int	rdp		:  1;	/* Resequence dual path      	     */
    u_int	fsn		:  1;	/* Full sequence numbers     	     */
    u_int	eas		:  1;	/* Subnode Addressing 	     	     */
    u_int	pstatus		:  2;	/* Cable status		     	     */
#define	VCD_BAD		0		/*  Both cables are bad		     */
#define	VCD_CABLE0 	1		/*  Cable 0 is good		     */
#define	VCD_CABLE1	2		/*  Cable 1 is good		     */
#define	VCD_GOOD	3		/*  Both cables are good	     */
    u_int	nadp		:  1;	/* NADP     		     	     */
    u_int	dqi		:  1;	/* DFREEQ inhibit	     	     */
    u_int			:  2;	/* MBZ			     	     */
    u_int	vcstate		:  1;	/* Virtual circuit state     	     */
#define	VC_OFF		0		/*  Virtual circuit is off	     */
#define	VC_ON		1		/*  Virtual circuit is on	     */
    u_int			: 12;	/* MBZ			     	     */
    u_int	gn		:  3;	/* Generation number         	     */
    u_int			:  1;	/* MBZ			     	     */
    u_int	snd_st		:  3;	/* Send states      	     	     */
    u_int			:  5;	/* MBZ     		     	     */
    u_int	rcv_st		:  2;	/* Receive states     	     	     */
    u_int			: 22;	/* MBZ   		     	     */
} VCD;

typedef struct _chnl_state {		/* Channel state variables	     */
    u_int	dqii		:  1;	/* Datagram queue inhibit illegal    */
    u_int	dqid		:  1;	/* Datagram queue inhibit default    */
    u_int	dqia		:  1;	/* Datagram queue inhibit all        */
    u_int			: 29;	/* MBZ				     */
    u_int			: 32;	/* MBZ				     */
} CHNLSTATE;

/* CI Port Specific Data Structure Definitions.
 */

/* CI Port Command Definitions.
 */
typedef struct	{			/* Packet Header Opcode Flags	     */
	u_short	rsp		:  1;	/* Response requested bit	     */
	u_short	is		:  1;	/* Interrupt suppress bit	     */
	u_short	common		:  1;	/* Common bit			     */
#define Flush_cache	common		/* Flush buffer cache		     */
#define Dgfq_entry	common		/* Datagram free queue entry	     */
#define Cnt_all_events	common		/* Count events all addrs & datalink */
	u_short			:  1;	/* SBZ				     */
	u_short			:  4;	/* Reserved for software	     */
	u_short gn 		:  3;	/* Virtual circuit generation number */
	u_short pkt_multiple	:  3;	/* Packet multiple bits 	     */
	u_short path_select	:  2;	/* Path select bits 		     */
#define	CS_AUTO		0		/*  Automatic cable selection	     */
#define	CS_CABLE0	1		/*  Select path a - cable 0	     */
#define	CS_CABLE1	2		/*  Select path b - cable 1	     */
} OPC_FLAGS;

typedef struct	{			/* Packet Header CI Opcode Flags     */
    union {
	struct {
    	    u_char lp		:  1;	/* Last packet when set              */
    	    u_char ns		:  3;	/* Sequence number		     */
    	    u_char pkt_multiple	:  3;	/* Packet multiple 		     */
    	    u_char fr		:  1;	/* Force reset			     */
#define	Force_reset	ov.a.fr		/*  Force reset			     */
#define	Default_start	ov.a.fr		/*  Default start address	     */
        } a;
	struct {
    	    u_char 		:  4;	/* Place holder			     */
    	    u_char sp		:  2;	/* Send path			     */
#define Internal	0
#define Path_A		1
#define Path_B		2
    	    u_char w_sel	:  2;	/* Path status table window select   */
        } b;
    } ov; 
} CIOPC_FLAGS;

typedef	struct	{			/* CKTSET/SETCKT Packet Header Format*/
    struct _vcd	mask;			/* Mask controlling VCD modification */
#define Exp_mask	0x000089FF	/* Explicit Mask - CST,NADP,EAS,FSN, */
					/* 		   RDP, NR0:2, NS0:2 */
#define Exp_mask_eas	0x00008100	/* Explicit Mask Close - CST, EAS    */
    struct _vcd	mvalue;			/* VCD modification values	     */
    struct _vcd ivalue;			/* VCD prior to modification	     */
    u_short vcd_res;			/* VCD resources remaining lowerbound*/
    u_short reseq_res;			/* Reseq. resources remaining lowerbo*/
} CKTSETH, SETCKTH;

typedef	struct	{			/* CKTSET/SETCKT Packet Header Format*/
    struct _chnl_state mask;		/* Mask controlling chnl state variab*/
    struct _chnl_state mvalue;		/* Modification values 		     */
    struct _chnl_state ivalue;		/* Initial channel state variables   */
    u_short vcd_res;			/* VCD resources remaining lowerbound*/
} SETCHNLH, CHNLSETH;

typedef	struct	{			/* IDREC Packet Header Format	     */
    struct _tid	xctid;			/* Transaction identifier	     */
    u_int	port_type;		/* Port type			     */
    u_int	ucode_rev;		/* Port microcode revision level     */
    u_int	port_fcn;		/* Port functionality mask	     */
    u_int	reset_port	:  8;	/* Port which caused last reset	     */
    u_int	port_state	:  3;	/* Port state			     */
    u_int	sys_state	: 21;	/* Implementation specific state     */
    u_int	port_fcn_ext;		/* Port functionality extension	     */
    u_int	port_fcn_ext2;		/* Port functionality extension #2   */
    u_int			: 32;	/* Sub_map field		     */
    u_int			: 32;	/* Sub_map field		     */
    u_int			: 32;	/* RSV field			     */
    u_int			: 32;	/* RSV field			     */
} IDRECH;

					/* IDREQ/REQID/SNDRST/CNFREC/DATREC  */
typedef	struct	{			/*  Packet Header Format	     */
    struct _tid	xctid;			/* Transaction identifier	     */
} CNFRECH, REQIDH, IDREQH, SNDRSTH, DATRECH, REQPSH, PSREQH;

typedef struct	{			/* LBREC Packet Header Format	     */
    u_short	lblength;		/* Size of loopback data	     */
    u_char	lbdata[ LBDSIZE ];	/* Loopback data		     */
} LBRECH;

typedef	struct	{			/* REQDAT/SNDDAT Packet Header Format*/
    struct _tid	     xctid;		/* Transaction identifier	     */
    u_int	     length;		/* Transaction length		     */
    struct _npbname sbname;		/* Sending buffer name		     */
    u_int	     sboff;		/* Sending buffer byte offset	     */
    struct _npbname rbname;		/* Receiving buffer name	     */
    u_int	     rboff;		/* Receiving buffer byte offset	     */
} REQDATH, SNDDATH;

typedef struct	{			/* SNDLB Packet Header Format	     */
    u_short	lblength;		/* Size of loopback data	     */
    u_char	lbdata[ LBDSIZE ];	/* Loopback data		     */
    u_char	crc[ 4 ];		/* Loopback packet CRC		     */
} SNDLBH;

typedef	struct	{			/* SNDSTRT Packet Header Format	     */
    struct _tid	xctid;			/* Transaction identifier	     */
    u_int	start_addr;		/* Starting address		     */
} SNDSTRTH;

typedef	struct	{			/* RETQE Packet Header Format	     */
    u_short	qe_req;			/* Number of que entries to be return*/
    u_short	qe_ret;			/* Number of que entries returned    */
} RETQEH, QERETH;

typedef	struct	{			/* PURGQ Packet Header Format	     */
    u_short	cmdq_mask;		/* Internal cmdq mask 		     */
#define	Purgq_mask	0x000F		/* Internal cccq0-3		     */
    u_short	resv;			/* reserved			     */
} PURGQH, QPURGH;

typedef struct  {			/* INVTC Packet Header Format        */
    NPBNAME bname;			/* Buffer name 			     */
} INVTCH, TCINVH;

typedef struct  {
    u_short    opt_cnt_len;
    u_short    imp_cnt_len;
    u_int      dg_disc;
    struct {
        u_int      patha_ack;
        u_int      patha_nak;
        u_int      patha_no_rsp;
        u_int      pathb_ack;
        u_int      pathb_nak;
        u_int      pathb_no_rsp;
    } ddl;
    struct {
        u_int      reserv[4];
        u_int      sntdat_bodies_snt;
        u_int      sntdat_data_snt;
        u_int      cnf_bodies_rcv;
        u_int      reqdat_oper_cmp;
        u_int      retdat_bodies_rcv;
        u_int      retdat_data_rcv;
        u_int      sntdat_bodies_rcv;
        u_int      sntdat_data_rcv;
        u_int      cnf_bodies_snt;
        u_int      reqdat_bodies_rcv;
        u_int      retdat_bodies_snt;
        u_int      retdat_data_snt;
    } dlb;
    struct {
        u_int      dg_bodies_snt;
        u_int      dg_txt_snt;
        u_int      msg_bodies_snt;
        u_int      msg_txt_snt;
        u_int      misc_bodies_snt;
        u_int      dg_bodies_rcv;
        u_int      dg_txt_rcv;
        u_int      msg_bodies_rcv;
        u_int      msg_txt_rcv;
        u_int      misc_bodies_rcv;
    } opt;
} RDCNTH, CNTRDH;

typedef struct  {			/* PSREC Packet Header Format        */
    u_int	mem_map[128]		/* Bitmaps 			     */
} PSRECH;


/* CI Macros.
 */
					/* Svatophy Address Shift Macro 
					 * for CCQxIR,AxFQIR address shift
					 * right per N_Port arch. ECO	    
					 */
#ifdef __alpha
#define Svatophy_shiftR5( value,regaddr,bus ) {			\
    u_long physaddr;						\
    svatophys(( value ), &physaddr);				\
    WRTCSR(LONG_32,bus,regaddr,(physaddr >> 5));	\
}								
#else
#define Svatophy_shiftR5( value, addr ) {			\
    u_long pfn;							\
    svatophys(( value ), &pfn );				\
    ( addr ) = pfn >> 5;					\
}
#endif

					/* Cluster Size Macros
					 */
#define	Cluster_size( p )	( p->Chnl_pb[p->C_idx].type.ci.arb_modulus )

					/* Event Logging Macros
					 */
#define	Clog_ftab( tab, event )						\
    ( tab[ Eseverity( event )][ Esubclass( event )].ftable + Ecode( event) - 1)
#define	Clog_maxcode( tab, event )					\
    ( tab[ Eseverity( event )][ Esubclass( event )].max_code )
#define	Clog_tabcode( tab, event )	( Clog_ftab( tab, event )->fcode )
#define	Clog_tabmsg( tab, event )	( Clog_ftab( tab, event )->msg )
#define	Elcibiregs( p )		(( struct bi_regs * )p )
#define	Elciciregs( p )		(( struct ci_regs * )p )
#define	Elcicommon( elp )	( &elp->el_body.elci.cicommon )
#define	Elcicpurevlev( p )	(( struct ci_cpurevlev * )p )
#define	Elcidattn( elp )	( &elp->el_body.elci.citypes.cidattn )
#define	Elcilcommon( elp )						\
    ( &elp->el_body.elci.citypes.cilpkt.cilcommon )
#define	Elcinppacket( elp )						\
    ( &elp->el_body.elci.citypes.cilpkt.cilpktopt.cinppacket )
#define	Elcirevlev( p )		(( struct ci_revlev * )p )
#define	Elciucode( p )		(( struct ci_ucode * )p )
#define	Elcixmiregs( p )	(( struct cixmi_regs * )p )
#define	Elcitcaregs( p )	(( struct citca_regs * )p )
#define	Elcinpapb( p )		(( struct ci_npapb * )p )
#define	Elcinpcpb( p )		(( struct ci_npcpb * )p )

					/* Loopback Macros
					 */
#define	Comp_loopback( pccb, bp )					\
    ( bp->lblength == LBDSIZE &&					\
      bcmp( bp->lbdata, pccb->Lbdata, LBDSIZE ) == 0 )
#define	Compute_lbcrc( pccb ) {						\
    LBCRC	  buf;							\
    ( void )bzero(( u_char * )&buf, sizeof( LBCRC ));			\
    buf.lblen7 = ( u_char )( LBDSIZE + 7 );				\
    buf.port_num = ( u_char )Scaaddr_low( pccb->lpinfo.addr );		\
    buf.not_port_num = ( u_char )~buf.port_num;				\
    buf.lport_addr = ( u_char )buf.port_num;				\
    buf.opcode = SNDLB;							\
    ( void )bcopy( pccb->Lbdata, buf.lbdata, LBDSIZE );			\
    pccb->Lbcrc = ( u_int )~( crc(( u_char * )np_crctable,		\
			           ( u_int )-1,				\
				   ( u_int )buf.lblen7,			\
				   ( u_char * )&buf ));			\
}

					/* Packet Header Macros
				         */
#define	Cable0_status( bp )	( bp->status.path_a )
#define	Cable1_status( bp )	( bp->status.path_b )
#define	Cnfrec( bp )		(( CNFRECH * )bp )
#define	Cselect( bp )		(( OPC_FLAGS * )&bp->flags )->path_select
#define	Caevents( bp )		(( OPC_FLAGS * )&bp->flags )->Cnt_all_events
#define	Dstart( bp )		(( CIOPC_FLAGS * )&bp->ci_flags )->Default_start
#define	Dgfqe( bp )		(( OPC_FLAGS * )&bp->flags )->Dgfq_entry
#define	Freset( bp )		(( CIOPC_FLAGS * )&bp->ci_flags )->Force_reset
#define	Fcache( bp )		(( OPC_FLAGS * )&bp->flags )->Flush_cache
#define Idrec( bp )    		(( IDRECH * )bp )
#define	Lbrec( bp )		(( LBRECH * )bp )
#define	Receive_cable( bp )	( bp->status.Rec_path )
#define	Send_cable( bp )	(( CIOPC_FLAGS * )&bp->ci_flags )->ov.b.sp
#define	Sndlb( bp )		(( SNDLBH * )bp )

					/* Port Crashing Macros
					 */
/* Flushq() removes all packets on the target queue.  In performing this task
 * it makes the following assumptions:
 *
 * 1. The local CI port is disabled and can not access the target queue.
 * 2. The only thread active on the local port is the one flushing the queue.
 *
 * These assumptions allow Flushq() to immediately zero a locked target queue
 * without worrying about possible size effects on other threads or on the
 * local port itself.  Flushq() also zeroes a queue on failure to obtain a
 * queue interlock.  Zeroing a queues results in permanent loss of all packets
 * currently residing on it.
 */
#define	Flushq( pccb, q ) {						\
}

#define Alloc_np_carrier( carhp, carp, pccb ) { 			\
    SCA_KM_ALLOC( carhp, CARRIERH *, sizeof( CARRIERH ), KM_SCABUF, KM_CLEAR )\
    if( carhp ) {							\
	carhp->Chdr_size = sizeof( CARRIERH ); 				\
        carp = Carhd_to_car( carhp );					\
        carp->Ctoken = ( caddr_t )carp;					\
    }									\
}
#define Init_chnl_ques( pccb, idx ) { 					\
    CARRIERH *carhp;							\
    CARRIER *carp;							\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Dccq2->H_ptr );				\
    pccb->Dccq2->T_ptr = ( caddr_t )carp;				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Dccq1->H_ptr );				\
    pccb->Dccq1->T_ptr = ( caddr_t )carp;				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Dccq0->H_ptr );				\
    pccb->Dccq0->T_ptr = ( caddr_t )carp;				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Chnl_cmdq[idx].cccq3.H_ptr );		\
    svatophys( carp, &pccb->Chnl_cmdq[idx].cccq3.T_ptr );		\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Chnl_cmdq[idx].cccq2.H_ptr );		\
    svatophys( carp, &pccb->Chnl_cmdq[idx].cccq2.T_ptr );		\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Chnl_cmdq[idx].cccq1.H_ptr );		\
    svatophys( carp, &pccb->Chnl_cmdq[idx].cccq1.T_ptr );		\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Chnl_cmdq[idx].cccq0.H_ptr );		\
    svatophys( carp, &pccb->Chnl_cmdq[idx].cccq0.T_ptr );		\
}
#define Init_adap_ques( pccb ) { 					\
    CARRIERH *carhp;							\
    CARRIER *carp;							\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
									\
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_STOPPER;			\
    pccb->Adrq.H_ptr = ( caddr_t )carp;					\
    svatophys( carp, &pccb->Adrq.T_ptr );				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
									\
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Dadfq.H_ptr );				\
    pccb->Dadfq.T_ptr = ( caddr_t )carp;				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
									\
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_STOPPER;			\
    pccb->Addfq.H_ptr = ( caddr_t )carp;				\
    svatophys( carp, &pccb->Addfq.T_ptr );				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
									\
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Damfq.H_ptr );				\
    pccb->Damfq.T_ptr = ( caddr_t )carp;				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
									\
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_STOPPER;			\
    pccb->Admfq.H_ptr = ( caddr_t )carp;				\
    svatophys( carp, &pccb->Admfq.T_ptr );				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
									\
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Aadfq.H_ptr );				\
    svatophys( carp, &pccb->Aadfq.T_ptr );				\
    carhp = ( CARRIERH * )pccb->Cfreeq.flink;                           \
									\
    Remove_entry((( npbq * )carhp )->flink )                            \
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq );		\
    carp = Carhd_to_car( carhp );                                 	\
    carp->next_car.a.ptr = ( caddr_t )Q_ZSTOPPER;                       \
    svatophys( carp, &pccb->Aamfq.H_ptr );				\
    svatophys( carp, &pccb->Aamfq.T_ptr );				\
}

					/* Port Register Access Macros
					 */
#define	Bad_reg( reg )		( BADADDR( reg, 4, pccb->Bus ))

#ifdef __alpha
#define	Get_reg( reg )		(( !Bad_reg( reg )) ? 	\
				(RDCSR(LONG_32,pccb->Bus,reg)) : 0 )
#else
#define	Get_reg( reg )		(( !Bad_reg( reg )) ? *reg : 0 )
#endif


					/* Miscellaneous Macros
					 */
#define Aibuf_len( pccb )	( pccb->Npadap->npab.adap_pb.ibuf_len )
#define Avcdt_len( pccb )	( pccb->Npadap->npab.adap_pb.vcdt_len )
#define Aramp( pccb )		( pccb->Npadap->npab.adap_pb.ramp )
#define Atyp_media( pccb )	( pccb->Npadap->npab.adap_pb.media )
#define Atyp_rdp( pccb )	( pccb->Npadap->npab.adap_pb.rdp )
#define Atyp_es( pccb )		( pccb->Npadap->npab.adap_pb.es )
#define Atyp_ea( pccb )		( pccb->Npadap->npab.adap_pb.ea )
#define Atyp_sdp( pccb )	( pccb->Npadap->npab.adap_pb.sdp )
#define Atyp_sic( pccb )	( pccb->Npadap->npab.adap_pb.sic )
#define Imp_lport_addr( pccb )	( pccb->Chnl_pb[pccb->C_idx].xp_addr >> 8 )
#define Exp_lport_addr( pccb )	( pccb->Chnl_pb[pccb->C_idx].xp_addr )
#define EFR_MASK( value )	((( value ) & FCN_EXT2_EFR ) >> 7 )
#define FR_MASK( value )	((( value ) & FCN_EXT2_FR ) >> 7 )
#define NADP_MASK( value )	((( value ) & FCN_EXT2_NADP ) >> 1 )
#define Mmv( value )		(( value ) & FCN_EXT2_MMV )

					/* Shorthand Notations
					 */
#define	Abbr		Npregptrs.abbr
#define	Adfqir		Npregptrs.adfqir
#define Adap_pb		Npadap->npab.adap_pb
#define	Aadfq		Npadap->npab.aadfq
#define	Addfq		Npadap->npab.addfq
#define	Aamfq		Npadap->npab.aamfq
#define	Admfq		Npadap->npab.admfq
#define	Adrq		Npadap->npab.adrq
#define	Aitcr		Npregptrs.aitcr
#define	Amcsr		Npregptrs.amcsr
#define	Amfqir		Npregptrs.amfqir
#define	Amtecr		Npregptrs.amtecr
#define	Amtcr		Npregptrs.amtcr
#define	Ampb_base	Npadap->npab.ampb_base.a.ptr
#define	Ampb_len	Npadap->npab.ampb_len
#define	Bdlt_base	Npadap->npab.bdlt_base.a.ptr
#define	Bdlt_len	Npadap->npab.bdlt_len
#define Bdl_idx   	buf_name.bdl_idx
#define Bdlt_idx  	buf_name.bdlt_idx
#define	Binuseq		pd.np.binuseq
#define	Bname		pd.np.bname
#define	Boff		pd.np.boff
#define Bus		pd.np.bus
#define	Ciisr		pd.np.ciisr
#define	Ccq0ir		Npregptrs.ccq0ir
#define	Ccq1ir		Npregptrs.ccq1ir
#define	Ccq2ir		Npregptrs.ccq2ir
#define	Cecr		Npregptrs.cecr
#define	Cesr		Npregptrs.cesr
#define	Cfar		Npregptrs.cfar
#define	Cicr		Npregptrs.cicr
#define	Csrcr		Npregptrs.csrcr
#define	Csr		Npregptrs.csr
#define	Chdr_size	type.hdr.size
#define	C_idx		pd.np.c_idx
#define	Cnextptr	next_car.a.ptr
#define Chnl_cmdq	Npadap->npab.chnl_cmdq
#define Chnl_pb		Npadap->npab.chnl_pb
#define	Cbufptr		buf_ptr.a.ptr
#define	Cbuftoken	buf_vptr.a.ptr
#define	Ctoken		car_addr.a.ptr
#define	Cfreeq		Npadap->cfreeq
#define	Cinuseq		Npadap->cinuseq
#define	Dbptr		db_ptr.a.ptr
#define	Dccq0		pd.np.dccq0
#define	Dccq1		pd.np.dccq1
#define	Dccq2		pd.np.dccq2
#define	Dg_size		pd.np.dg_size
#define	Dg_cache	pd.np.dg_cache
#define	Dfreeq		Npadap->dfreeq
#define	Devattn		pd.np.devattn
#define	Dadfq		Npadap->npab.dadfq
#define	Damfq		Npadap->npab.damfq
#define	Dqe_len		Npadap->npab.dqe_len
#define	Drv_param	Npadap->npab.drv_param
#define	Fn_level	pd.np.fn_level
#define	Func_mask	func_mask
#define H_ptr		head_ptr.a.ptr
#define	Invtcpkt	pd.np.invtcpkt
#define	Interconnect	pd.np.interconnect
#define	Intr_holdoff	Npadap->npab.intr_holdoff[0]
#define	Isrfb		pd.np.isrforkb
#define	Keepalive	Npadap->npab.keep_alive
#define Key   	  	buf_name.key
#define	Lbcrc		pd.np.lbcrc
#define	Lbdata		pd.np.lbdata
#define	Lbstatus	pd.np.lbstatus
#define	Load_ucode	pd.np.load_ucode
#define	Lpstatus	pd.np.lpstatus
#define	Max_fn_level	pd.np.max_fn_level
#define	Max_rom_level	pd.np.max_rom_level
#define	Mfreeq		Npadap->mfreeq
#define	Mqe_len		Npadap->npab.mqe_len
#define	Mrltab		pd.np.mrltab
#define	Msg_cache	pd.np.msg_cache
#define	Msg_size	pd.np.msg_size
#define Next_free_bd    root_ptr2.a.ptr
#define	Npadap		pd.np.npadap
#define	Npregptrs	pd.np.regptrs
#define	Ovhd_pd		pd.np.ovhd_pd
#define	Ovhd		pd.np.ovhd
#define	Pgsize		pd.np.pgsize
#define	Pkt_size	pd.np.pkt_size
#define	Pkt_mult	pd.np.pkt_mult
#define	Port_fcn	pd.np.port_fcn
#define	Port_fcn_ext	pd.np.port_fcn_ext
#define	Port_fcn_ext2	pd.np.port_fcn_ext2
#define	Pstatus		pd.np.pstatus
#define	Purgpkt		pd.np.purgpkt
#define	Reinit_tries	pd.np.reinit_tries
#define	Reset_port	pd.np.reset_port
#define	Rom_level	pd.np.rom_level
#define Rptr1		root_ptr1.a.ptr
#define Rptr2		root_ptr2.a.ptr
#define	Rport_state	pd.np.rport_state
#define	Rpslogmap	pd.np.rpslogmap
#define	Scpkt		pd.np.scpkt
#define	Start_port	pd.np.start_port
#define	St_level	Rom_level
#define T_ptr		tail_ptr.a.ptr
#define Typ0_max	pd.np.typ0_max
#define Typ1_max	pd.np.typ1_max
#define	Disable_port	pd.np.disable_port
#define	Ucode_rev	pd.np.ucode_rev

/* 
 *Isr fork macros
 */
#define	Lpstatus_npisrfork( pccb )  (*( u_int * )&pccb->Lpstatus & NP_ISRFORK )

#define	Clear_npisrfork( pccb ) {					\
    *( u_int * )&pccb->Lpstatus &= ~NP_ISRFORK;				\
}
#define	Set_npisrfork( pccb ) {						\
    *( u_int * )&pccb->Lpstatus |= NP_ISRFORK;				\
}
#define Npstart_isrfp( pccb ) {						\
    u_long	unlock;							\
    if( !Test_pccb_lock( pccb )) {					\
        Lock_pccb( pccb )						\
        unlock = 1;							\
    } else {								\
        unlock = 0;							\
    }									\
    if( Lpstatus_npisrfork( pccb ) == 0 ) {				\
        Set_npisrfork( pccb ) 						\
        if( unlock ) {							\
            unlock = 0;							\
            Unlock_pccb( pccb )						\
        }								\
        Isr_threadfork( &pccb->Isrfb, np_rsp_handler, pccb )		\
    }									\
    if( unlock ) {							\
        Unlock_pccb( pccb )						\
    }									\
}



/* CI Port Specific Macros.
 */
#define Get_pgrp( pccb, cibp ) 					  	  \
    (( cibp->opc & 0x01 ) ? ( u_int )( cibp->srcport >> 8 ) : 		  \
     ( u_int )( cibp->dstport >> 8 ))

					/* CIMNA Ucode Revision Level Macros
				 	 */
#ifdef __alpha
#define	Cimna_fn_level( pccb )	((RDCSR(LONG_32,pccb->Bus, \
						pccb->Mna_dev) ) >> 24 )
#else
#define	Cimna_fn_level( pccb )	(*(( u_int * )pccb->Mna_dev ) >> 24 )
#endif 
					/* CITCA Ucode Revision Level Macros
				 	 */
#define	Citca_fn_level( pccb )	(*(( u_int * )pccb->Tca_dev ) >> 24 )

					/* Maintenance Interrupt Macros
					 */
/* These macros are used to manually poll for maintenance interrupts when port
 * interrupts have been explicitly disabled.  Interrupts are manually checked
 * for every 10 msec up to a maximum of 200 msec.
 */

#ifdef __alpha
#define	Adap_wait_unin() {						\
    u_int	dsecs;							\
    u_int	csr;							\
    DELAY( 10000 );							\
    for( dsecs = 20; dsecs > 0; --dsecs ) {				\
	csr = RDCSR(LONG_32,pccb->Bus,pccb->Csr);		\
	if( csr & CSR_UNIN ) {						\
	    break;							\
	} else {							\
	    DELAY( 10000 );						\
	}								\
    }									\
}
#define	Chnl_wait_pic_clear() {						\
    u_int	dsecs;							\
    u_int	csr;							\
    DELAY( 10000 );							\
    for( dsecs = 20; dsecs > 0; --dsecs ) {				\
	csr = RDCSR(LONG_32,pccb->Bus,pccb->Csr);		\
	if( !(csr & CSR_CIC) ) {					\
	    break;							\
	} else {							\
	    DELAY( 10000 );						\
	}								\
    }									\
}

#define	Chnl_wait_pic() {						\
    u_int	dsecs;							\
    u_int	csr;							\
    DELAY( 10000 );							\
    for( dsecs = 20; dsecs > 0; --dsecs ) {				\
	csr = RDCSR(LONG_32,pccb->Bus,pccb->Csr);		\
	if( csr & CSR_CIC ) {						\
	    break;							\
	} else {							\
	    DELAY( 10000 );						\
	}								\
    }									\
}
#else
#define	Adap_wait_unin() {						\
    u_int	dsecs;							\
    DELAY( 10000 );							\
    for( dsecs = 20; dsecs > 0; --dsecs ) {				\
	if( *pccb->Csr & CSR_UNIN ) {					\
	    break;							\
	} else {							\
	    DELAY( 10000 );						\
	}								\
    }									\
}
#define	Chnl_wait_pic_clear() {						\
    u_int	dsecs;							\
    DELAY( 10000 );							\
    for( dsecs = 20; dsecs > 0; --dsecs ) {				\
	if( !(*pccb->Csr & CSR_CIC) ) {					\
	    break;							\
	} else {							\
	    DELAY( 10000 );						\
	}								\
    }									\
}

#define	Chnl_wait_pic() {						\
    u_int	dsecs;							\
    DELAY( 10000 );							\
    for( dsecs = 20; dsecs > 0; --dsecs ) {				\
	if( *pccb->Csr & CSR_CIC ) {					\
	    break;							\
	} else {							\
	    DELAY( 10000 );						\
	}								\
    }									\
}
#endif
					/* Microcode Related Macros
					 */


/*  N_PORT Constants.
 */

					/* NP Panic Strings		     */
#define	NPPANIC_BNAME		"np\t- illegal buffer name\n"
#define	NPPANIC_XFERSIZE	"np\t- illegal buffer transfer size\n"
#define NPPANIC_ALIGN_ARRAY	"np\t- alignment array entry zero\n"

/* N_PORT Data Structure Definitions.
 */

typedef u_int u_int32;

typedef struct _ptr {
    union {
        caddr_t ptr;
        u_int32	size[2];
    } a;
} PTR;

typedef struct _bdlp {			/* Buffer Descriptor Leaf Pointer    */
    PTR	    bdl_ptr;			/* Buffer descriptor leaf pointer    */
#define BDLP_VALID    0x01		/* Valid bit in pointer		     */
    PTR     vbdl_ptr;			/* VA of BDL			     */
} BDLP;

typedef struct _dbptr {			/* Data Buffer Pointer Structure     */
    PTR	    db_ptr;			/* Physical address		     */
#define	BP_TYP0		0		/* Physical address of data buffer   */
#define	BP_TYP1		1		/* Physical addr packed array of typ0*/
#define	BP_TYP2		2		/* Physical addr packed array of typ1*/
} DBP;

#define NPORT_PGSIZE	 8192
#define ALIGN_4BYTES	 0x03
#define SIZE16K		16384
#define SIZE32K		32768
#define SIZE64K		65536
#define SIZE128K       131072	
#define SIZE256K       262144
#define TYP0_4K_MAX	 8192		/* Type0 ptr max for 4k page R3000   */
#define TYP0_8K_MAX 	16384		/* Type0 ptr max for 8k page ALPHA - */
#define TYP1_4K_MAX   2097152		/* Type1 ptr max for 4k page short   */
#define TYP1_8K_MAX   8388608		/* Type1 ptr max for 8k page ALPHA - */
					/* R4000?   			     */
typedef	struct	_bd	{		/*  N_PORT Buffer Descriptor 	     */
    u_short	  poff	:13;		/* Offset into page of buffer        */
    u_short		: 3;		/* SBZ	 			     */
    u_short	  valid	: 1;		/* Buffer Descriptor valid flag      */
#define	NPBD_VALID	0x10000
#define	NPBD_RDONLY	0x20000
    u_short	  ro	: 1;		/* Read only acess bit 		     */
    u_short		:14;		/* SBZ				     */
    NPBNAME	  buf_name;		/* Key ( sequence number )	     */
    u_int	  buf_len;		/* Size of buffer in bytes	     */
    u_int	  sbz;			/* SBZ 				     */
    PTR	  	  root_ptr1;		/* Physical address of buffer        */
    PTR   	  root_ptr2;		/* Physical address of buffer        */
} BD;

#define PTR_ARRAY_SIZE  	1024
typedef struct _ptr_array {
    DBP	    array[PTR_ARRAY_SIZE];
} PTR_ARRAY;

#define BDL_NUM  	  2
#define BDL_SIZE  	256
typedef struct _bdl {
    BD      array[BDL_SIZE];
} BDL;

#define BDLT_SIZE	512
typedef struct _bdlt {
    BDLP    array[BDLT_SIZE];
} BDLT;
					/*  N_PORT Buffer Descriptor Leaf    */
typedef	struct	_npbddb {		/*  Database Header		     */
    BD		*free_bd;		/* Free NP BD list head	     	     */
    BDLP	*bdlt;			/* Buffer Descriptor leaf table ptr  */
    u_short	size;			/* Size of data structure	     */
    u_short	type;			/* Structure type		     */
    u_short	valid;
} NPBDDB;

/*struct lock_t lk_npbddb;		/* NP BD database lock structure    */
/* OSF/1 */
struct slock lk_npbddb;			/* NP BD database lock structure    */

typedef struct _npbh	 {		/*  N_PORT Queue Buffer Header	     */
    struct _ptr   token;		/* q_buffer pointer		     */
    u_char	  opc;			/* Operation codes field 	     */
    u_char	  c_idx;		/* 0 <= Channel index <= 15 	     */
    OPC_FLAGS flags;			/* Operation Flags - see OPC_FLAGS   */
    struct	{			/* Response status		     */
	u_short	 failure	:  1;	/* Failure bit			     */
	u_short	 type		:  7;	/* Error type			     */
	u_short	 path_a		:  2;	/* Path A			     */
#define Rec_path	path_a		/* Received path status	     	     */
	u_short	 path_b		:  2;	/* Path B			     */
#define	CABLE_ACK	0		/*  ACK on cable or cable not use    */
#define	CABLE_NAK	1		/*  NAK on cable		     */
#define	CABLE_NORSP	2		/*  No response on cable	     */
#define	CABLE_TIMEOUT	3		/* CI arbitration timeout	     */
	u_short	 ca		:  1;	/* Counted events were for all -RDCNT*/
	u_short			:  3;	/* Place holder		     	     */
    } status;
    u_char	ci_opc;			/* CI_OPC command operation codes    */
    CIOPC_FLAGS ci_flags;		/* CI Operation Flags - CIOPC_FLAGS  */
    u_short	dstport;		/* New Explicit Addr. Format Adapters*/
    u_short	srcport;		/* New Explicit Addr. Format Adapters*/
    u_int	reserv;			/* Reserved for test		     */
} NPBH;

#define Nph_size_exp	sizeof( NPBH )	

					/*  N_PORT Port-to-Port      	     */
typedef	struct _npppdh {		/*  Driver Buffer Header	     */
    u_short	length;			/* Length of message/datagram	     */
    u_short	mtype;			/* PPD message type		     */
} NPPPDH;

typedef	struct _carrier { 		/* Nport carrier structure	     */
    struct _ptr next_car;
    struct _ptr buf_ptr;
    struct _ptr buf_vptr;
    struct _ptr car_addr;
} CARRIER;

typedef struct _hdr {
    struct _npbq     *flink;
    struct _npbq     *blink;
    u_int32	      size;		/* Size of header & packet	     */
    u_int32	      type;		/* Structure type		     */
    struct _carrierh *carh_ptr;		/* Carrier pointer		     */
} HDR;

typedef	struct _carrierh { 		/* Carrier & header structure	     */
    union {
	u_int32 size[8];		/* For N_PORT alignment	requirement  */
	HDR hdr;
    } type;
    CARRIER carrier;
} CARRIERH;

#define	Q_STOPPER	0x00000001      /* N_PORT Queue stopper     	     */
#define	Q_ZSTOPPER	0x00000000      /* N_PORT Queue zero stopper         */
#define Car_hdr_size 	sizeof( CARRIERH )
#define Car_size 	sizeof( CARRIER )
#define Nphdr_size 	64		/* N_PORT alignment requirement	     */
#define Car_to_carhd( carp )	 					\
    (( CARRIERH * )(( u_char * )carp - Car_size ))
#define Carhd_to_car( carhp )						\
    (( CARRIER * )(( u_char * )carhp + Car_size ))
#define Nphd_to_npbp( npbhp ) 						\
    (( NPBH * )(( u_char * )npbhp + Nphdr_size ))
#define Npbp_to_nphd( npbp ) 						\
    (( HDR * )(( u_char * )npbp - Nphdr_size ))


/*  N_PORT Macros.
 */
					/* NP Header Positioning and
					 *  Manipulation Macros
				         */
#define	Appl_size( npppdbp )		((( NPPPDH * )npppdbp )->length - 2 )
#define	Format_nph( pccb, npbp, npopc, ciopc, port_addr, dispose ) {	\
   U_int((( NPBH * )npbp )->opc ) = npopc|(pccb->C_idx << 8)|(dispose >> 8);\
   U_int((( NPBH * )npbp )->status ) = ciopc << 16;			\
   U_int((( NPBH * )npbp )->dstport ) = port_addr << 8;			\
   (( NPBH * )npbp )->reserv = 0;					\
}
#define	Format_npppdh( npppdbp, type, size ) {				\
    U_int((( NPPPDH * )npppdbp)->length ) = (( type << 16 ) | ( size + 2));\
}
#define	Pd_to_ppd( npbp, pccb )						\
    (( NPPPDH * )(( u_char * )npbp + pccb->lpinfo.Ovhd_pd ))
#define	Pd_to_scs( npbp, pccb )						\
    (( SCSH * )(( u_char * )npbp + pccb->lpinfo.Ovhd ))
#define	Ppd_to_pd( npppdbp, pccb )					\
    (( NPBH * )(( u_char * )npppdbp - pccb->lpinfo.Ovhd_pd ))
#define	Ppd_to_scs( npppdbp )	(( SCSH * )(( NPPPDH * )npppdbp + 1 ))
#define	Scs_to_ppd( scsbp )	(( NPPPDH * )scsbp - 1 )
#define	Scs_to_pd( scsbp, pccb ) 					\
    (( NPBH * )(( u_char * )scsbp - pccb->lpinfo.Ovhd ))

					/* Queuing Macros
					 */
/* SMP: PCCB must be either locked or lockable for all queuing macros.
 */
#ifdef __alpha
#define	Insqt_dccq0( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Binuseq )\
    Insqt( pccb, npbp, pccb->Dccq0, carp )				\
    Svatophy_shiftR5( carp, pccb->Ccq0ir, pccb->Bus )			\
}
#define	Insqt_dccq1( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Binuseq )\
    Insqt( pccb, npbp, pccb->Dccq1, carp )				\
    Svatophy_shiftR5( carp, pccb->Ccq1ir, pccb->Bus )			\
}
#define	Insqt_dccq2( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Binuseq )\
    Insqt( pccb, npbp, pccb->Dccq2, carp )				\
    Svatophy_shiftR5( carp, pccb->Ccq2ir, pccb->Bus )			\
}
#define	Insqt_dfreeq( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Dfreeq )\
    Insqt( pccb, npbp, &pccb->Dadfq, carp )				\
    Svatophy_shiftR5( carp, pccb->Adfqir, pccb->Bus )			\
}
#define	Insqt_mfreeq( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Mfreeq )\
    Insqt( pccb, npbp, &pccb->Damfq, carp )				\
    Svatophy_shiftR5( carp, pccb->Amfqir, pccb->Bus )			\
}
#else
#define	Insqt_dccq0( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Binuseq )\
    Insqt( pccb, npbp, pccb->Dccq0, carp )				\
    Svatophy_shiftR5( carp, *pccb->Ccq0ir )				\
}
#define	Insqt_dccq1( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Binuseq )\
    Insqt( pccb, npbp, pccb->Dccq1, carp )				\
    Svatophy_shiftR5( carp, *pccb->Ccq1ir )				\
}
#define	Insqt_dccq2( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Binuseq )\
    Insqt( pccb, npbp, pccb->Dccq2, carp )				\
    Svatophy_shiftR5( carp, *pccb->Ccq2ir )				\
}
#define	Insqt_dfreeq( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Dfreeq )\
    Insqt( pccb, npbp, &pccb->Dadfq, carp )				\
    Svatophy_shiftR5( carp, *pccb->Adfqir )				\
}
#define	Insqt_mfreeq( npbp, pccb ) {					\
    CARRIER *carp;							\
    Insert_entry((( npbq * )Npbp_to_nphd( npbp ))->flink, pccb->Mfreeq )\
    Insqt( pccb, npbp, &pccb->Damfq, carp )				\
    Svatophy_shiftR5( carp, *pccb->Amfqir )				\
}
#endif

/* N_PORT queueing mechanics */
#define Insqt( pccb, npbp, pq, carp ) {					\
    CARRIERH *carhp;							\
    CARRIER *carpt;							\
    npbp->token.a.size[1] = 0;						\
    npbp->token.a.ptr = ( caddr_t )npbp;				\
    carhp = ( Npbp_to_nphd( npbp )->carh_ptr );				\
    Insert_entry((( npbq * )carhp )->flink, pccb->Cinuseq ) 		\
    carp = Carhd_to_car( carhp );					\
    carpt = ( CARRIER * )(( pq )->T_ptr ); 				\
    svatophys( npbp, &carpt->Cbufptr );					\
    carpt->Cbuftoken = ( caddr_t )npbp;					\
    svatophys( carp, &carpt->Cnextptr );				\
    carpt->Cnextptr += 1;						\
    ( pq )->T_ptr = ( caddr_t )carp;					\
}
#define	Remqh_dfreeq( pccb, npbp ) {					\
    Remqh( pccb, npbp, &pccb->Addfq ) 					\
}
#define	Remqh_mfreeq( pccb, npbp ) {					\
    Remqh( pccb, npbp, &pccb->Admfq )	 				\
}
#define	Remqh_rspq( pccb, npbp ) {					\
    Remqh( pccb, npbp, &pccb->Adrq ) 					\
}
#define Remqh(  pccb, npbp, pq ) {					\
    CARRIER *carp;							\
    CARRIERH *carhp;							\
    carp = ( CARRIER * )(( pq )->H_ptr );				\
    Clean_dcache_addr( carp, 128, 0x7f )				\
    if( U_int( carp->Cnextptr ) & Q_STOPPER ) {				\
	npbp = NULL;							\
    } else { 								\
	npbp = ( NPBH * )( carp->Cbufptr );				\
	( pq )->H_ptr = ( caddr_t )( carp->Cnextptr );			\
        Clean_dcache_addr( npbp, 512, 0x1ff )                           \
	carhp = Car_to_carhd( carp ); 					\
	Npbp_to_nphd( npbp )->carh_ptr = carhp; 			\
        Remove_entry((( npbq * )carhp )->flink )			\
        Remove_entry((( npbq * )Npbp_to_nphd( npbp ))->flink )		\
    }									\
}

#ifdef __mips__
#define Clean_dcache_addr( addr, size, mask ) {                         \
    u_long      pfn;                              			\
    svatophys( addr, &pfn );                  				\
    clean_dcache(((u_long)PHYS_TO_K0( pfn ) & ~mask), size );           \
}
#else
#define Clean_dcache_addr( addr, size, mask )       ;
#endif  /* __mips__ */

/* SMP Locking Macros
 */
#ifndef REMOVE_DSA_SMP
#define	Init_npbd_lock() {						\
    lockinit( &lk_npbddb, &lock_npbddb_d );				\
}
#define	Lock_npbd() {							\
    smp_lock( &lk_npbddb, LK_RETRY );					\
}
#define	Unlock_npbd() {							\
    smp_unlock( &lk_npbddb );						\
}
#else
#define Init_npbd_lock()       ;
#define Lock_npbd()            ;
#define Unlock_npbd()          ;
#endif

#endif

