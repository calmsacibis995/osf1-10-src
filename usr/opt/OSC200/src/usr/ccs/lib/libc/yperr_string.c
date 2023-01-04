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
static char     *sccsid = "@(#)$RCSfile: yperr_string.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/08 00:28:55 $";
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
#pragma weak yperr_string = __yperr_string
#endif
#endif
#include <rpcsvc/ypclnt.h>

/*
 * This returns a pointer to an error message string appropriate to an input
 * NIS error code.  An input value of zero will return a success message.
 * In all cases, the message string will start with a lower case chararacter,
 * and will be terminated neither by a period (".") nor a newline.
 */
char *
yperr_string(code)
	int code;
{
	char *pmesg;
	
	switch (code) {

	case 0:  {
		pmesg = "NIS operation succeeded";
		break;
	}
		
	case YPERR_BADARGS:  {
		pmesg = "args to NIS function are bad";
		break;
	}
	
	case YPERR_RPC:  {
		pmesg = "RPC failure on NIS operation";
		break;
	}
	
	case YPERR_DOMAIN:  {
		pmesg = "can't bind to a server which serves domain";
		break;
	}
	
	case YPERR_MAP:  {
		pmesg = "no such map in server's domain";
		break;
	}
		
	case YPERR_KEY:  {
		pmesg = "no such key in map";
		break;
	}
	
	case YPERR_YPERR:  {
		pmesg = "internal NIS server or client error";
		break;
	}
	
	case YPERR_RESRC:  {
		pmesg = "local resource allocation failure";
		break;
	}
	
	case YPERR_NOMORE:  {
		pmesg = "no more records in map database";
		break;
	}
	
	case YPERR_PMAP:  {
		pmesg = "can't communicate with portmapper";
		break;
		}
		
	case YPERR_YPBIND:  {
		pmesg = "can't communicate with ypbind";
		break;
		}
		
	case YPERR_YPSERV:  {
		pmesg = "can't communicate with ypserv";
		break;
		}
		
	case YPERR_NODOM:  {
		pmesg = "local domain name not set";
		break;
	}

	case YPERR_BADDB:  {
		pmesg = "NIS map data base is bad";
		break;
	}

	case YPERR_VERS:  {
		pmesg = "NIS client/server version mismatch";
		break;
	}

	case YPERR_ACCESS: {
		pmesg = "permission denied";
		break;
	}

	case YPERR_BUSY: {
		pmesg = "database is busy";
		break;
	}
		
	default:  {
		pmesg = "unknown NIS client error code";
		break;
	}
	
	}

	return(pmesg);
}
