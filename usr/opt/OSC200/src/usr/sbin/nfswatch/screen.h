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
 * @(#)$RCSfile: screen.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 19:23:41 $
 */
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/screen.h,v 4.0 1993/03/01 19:59:00 davy Exp $
 *
 * screen.h - definitions for the display screen.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: screen.h,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.4  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.3  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.2  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.1  1993/01/15  15:44:00  davy
 * Changed field size.
 *
 * Revision 3.0  1991/01/23  08:23:26  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.2  90/08/17  15:47:12  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:32  davy
 * NFSWATCH Release 1.0
 * 
 */

#define NONNFSLINES	16		/* non-NFS counter lines	*/
#define NFSLINES	(2 * (LINES-NONNFSLINES)) /* NFS counter lines	*/

#define SCR_MIDDLE	40		/* middle of screen, y coord	*/
#define SCR_PKTLEN	20		/* size of packet name field	*/
#define SCR_NFSLEN	20		/* size of file sys name field	*/

/*
 * X0 is the X location of the field name, X is the coordinate of the
 * field value.  Y is the vertical coordinate of the field name and
 * value.
 */
#define SCR_IF_Y	3
#define SCR_HOST_X	0		/* destination host name	*/
#define SCR_HOST_Y	0
#define SCR_DATE_X	28		/* current date			*/
#define SCR_DATE_Y	0
#define SCR_ELAPS_X0	57		/* elapsed time			*/
#define SCR_ELAPS_X	71
#define SCR_ELAPS_Y	0
#define SCR_PKTINT_X0	0		/* packets this interval	*/
#define SCR_PKTINT_X	19
#define SCR_PKTINT_Y	1
#define SCR_PKTTOT_X0	0		/* total packets received	*/
#define SCR_PKTTOT_X	19
#define SCR_PKTTOT_Y	2
#define SCR_PROMPT_X0	0		/* prompt			*/
#define SCR_PROMPT_X	10
#define SCR_PROMPT_Y	(LINES - 1)

#define SCR_PKT_Y	5		/* start of packet counters	*/
#define SCR_PKTHDR_X	21		/* header coords		*/
#define SCR_PKTHDR_Y	4
#define SCR_PKT_INT_X	19		/* interval counter		*/
#define SCR_PKT_PCT_X	26		/* percentage			*/
#define SCR_PKT_TOT_X	31		/* total counter		*/
#define SCR_PKT_NAME_X	0

#define SCR_NFS_Y	15		/* start of nfs counters	*/
#define SCR_NFSHDR_X	5		/* header coords		*/
#define SCR_NFSHDR_Y	14
#define SCR_NFS_INT_X	19		/* interval counter		*/
#define SCR_NFS_PCT_X	26		/* percentage			*/
#define SCR_NFS_TOT_X	31		/* total counter		*/
#define SCR_NFS_COMP_X	41		/* completed replies		*/
#define SCR_NFS_RESP_X	51		/* time of replies		*/
#define SCR_NFS_RSQR_X	61		/* squared time of replies	*/
#define SCR_NFS_RMAX_X	71		/* max response time		*/
#define SCR_NFS_NAME_X	0

/*
 * Screen text items to be displayed.
 */
struct scrtxt {
	short	s_x;			/* x coordinate			*/
	short	s_y;			/* y coordinate			*/
	char	*s_text;		/* text to be displayed		*/
};
