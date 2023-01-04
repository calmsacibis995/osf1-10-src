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
static char     *sccsid = "@(#)$RCSfile: ypprot_err.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/08 00:29:24 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 *  1.8 87/08/12 
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak ypprot_err = __ypprot_err
#endif
#endif
#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

/*
 * Maps a NIS protocol error code (as defined in
 * yp_prot.h) to a NIS client interface error code (as defined in
 * ypclnt.h).
 */
int
ypprot_err(yp_protocol_error)
	unsigned int yp_protocol_error;
{
	int reason;

	switch (yp_protocol_error) {
	case YP_TRUE: 
		reason = 0;
		break;
 	case YP_NOMORE: 
		reason = YPERR_NOMORE;
		break;
 	case YP_NOMAP: 
		reason = YPERR_MAP;
		break;
 	case YP_NODOM: 
		reason = YPERR_DOMAIN;
		break;
 	case YP_NOKEY: 
		reason = YPERR_KEY;
		break;
 	case YP_BADARGS:
		reason = YPERR_BADARGS;
		break;
 	case YP_BADDB:
		reason = YPERR_BADDB;
		break;
 	case YP_VERS:
		reason = YPERR_VERS;
		break;
	default:
		reason = YPERR_YPERR;
		break;
	}
	
  	return(reason);
}
