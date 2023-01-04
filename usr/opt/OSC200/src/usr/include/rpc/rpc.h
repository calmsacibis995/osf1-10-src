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
 *	@(#)$RCSfile: rpc.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:10:45 $
 */
/*
 */


/* 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */


/*
 * rpc.h, Just includes the billions of rpc header files necessary to 
 * do remote procedure calling.
 */

#ifndef _rpc_rpc_h
#define	_rpc_rpc_h
#ifdef	KERNEL
#include "../rpc/types.h"
#include "../netinet/in.h"

/* external data representation interfaces */
#include "../rpc/xdr.h"		/* generic (de)serializer */

/* Client side only authentication */
#include "../rpc/auth.h"	/* generic authenticator (client side) */

/* Client side (mostly) remote procedure call */
#include "../rpc/clnt.h"	/* generic rpc stuff */

/* semi-private protocol headers */
#include "../rpc/rpc_msg.h"	/* protocol for rpc messages */
#include "../rpc/auth_unix.h"	/* protocol for unix style cred */
#include "../rpc/auth_des.h"	/* protocol for des style cred */

/* Server side only remote procedure callee */
#include "../rpc/svc.h"		/* service manager and multiplexer */
#include "../rpc/svc_auth.h"	/* service side authenticator */

#else

#include <rpc/types.h>		/* some typedefs */
#include <netinet/in.h>

/* external data representation interfaces */
#include <rpc/xdr.h>		/* generic (de)serializer */

/* Client side only authentication */
#include <rpc/auth.h>		/* generic authenticator (client side) */

/* Client side (mostly) remote procedure call */
#include <rpc/clnt.h>		/* generic rpc stuff */

/* semi-private protocol headers */
#include <rpc/rpc_msg.h>	/* protocol for rpc messages */
#include <rpc/auth_unix.h>	/* protocol for unix style cred */

/* Server side only remote procedure callee */
#include <rpc/svc.h>		/* service manager and multiplexer */
#include <rpc/svc_auth.h>	/* service side authenticator */
#endif /* KERNEL */

#endif /*!_rpc_rpc_h*/
