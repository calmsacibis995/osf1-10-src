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
 *	@(#)$RCSfile: spray.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:21:01 $
 */ 
/*
 */
/*
 * OSF/1 Release 1.0
 */

/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 */

#ifndef _rpcsvc_spray_h
#define _rpcsvc_spray_h

#define SPRAYPROG 100012
#define SPRAYPROC_SPRAY 1
#define SPRAYPROC_GET 2
#define SPRAYPROC_CLEAR 3
#define SPRAYVERS_ORIG 1
#define SPRAYVERS 1

#define SPRAYOVERHEAD 86	/* size of rpc packet when size=0 */
#define SPRAYMAX 8845		/* related to max udp packet of 9000 */

int xdr_sprayarr();
int xdr_spraycumul();

struct spraycumul {
	unsigned counter;
	struct timeval clock;
};

struct sprayarr {
	int *data;
	int lnth;
};

#endif /*!_rpcsvc_spray_h*/
