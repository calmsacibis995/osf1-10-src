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
 * @(#)$RCSfile: ipports.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 19:20:44 $
 */
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/ipports.h,v 4.0 1993/03/01 19:59:00 davy Exp $
 *
 * ipport.h - port definitions used by nfswatch, not provided by Ultrix
 *
 * Jeffrey Mogul
 * DECWRL
 *
 * log: ipports.h,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.0  1991/01/23  08:24:17  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.1  90/08/17  15:46:47  davy
 * Initial revision
 * 
 * Revision 1.1  90/04/20  13:59:23  mogul
 * Initial revision
 * 
 */

/*
 * In Ultrix and BSD, programs always use "getservbyname" to
 * do these translations, but I guess someone at Sun had to
 * build a case table at some point.
 */
#define IPPORT_ROUTESERVER	520	/* routing control protocol	*/
#define IPPORT_ECHO		  7	/* packet echo server		*/
#define IPPORT_DISCARD		  9	/* packet discard server	*/
#define IPPORT_SYSTAT		 11	/* system stats			*/
#define IPPORT_DAYTIME		 13	/* time of day server		*/
#define IPPORT_NETSTAT		 15	/* network stats		*/
#define IPPORT_FTP		 21	/* file transfer		*/
#define IPPORT_TELNET		 23	/* remote terminal service	*/
#define IPPORT_SMTP		 25	/* simple mail transfer protocol*/
#define IPPORT_TIMESERVER	 37	/* network time synchronization	*/
#define IPPORT_NAMESERVER	 53	/* domain name lookup		*/
#define IPPORT_WHOIS		 43	/* white pages			*/
#define IPPORT_MTP		 57	/* ???				*/
#define IPPORT_TFTP		 69	/* trivial file transfer	*/
#define IPPORT_RJE		 77	/* remote job entry		*/
#define IPPORT_FINGER		 79	/* finger			*/
#define IPPORT_TTYLINK		 87	/* ???				*/
#define IPPORT_SUPDUP		 95	/* SUPDUP			*/
#define IPPORT_EXECSERVER	512	/* rsh				*/
#define IPPORT_LOGINSERVER	513	/* rlogin			*/
#define IPPORT_CMDSERVER	514	/* rcmd				*/
#define IPPORT_BIFFUDP		512	/* biff mail notification	*/
#define IPPORT_WHOSERVER	513	/* rwho				*/
