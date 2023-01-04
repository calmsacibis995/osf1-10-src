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
/****************************************************************************
**                                                                          *
**            Copyright (c) Digital Equipment Corporation, 1990.            *
**            All Rights Reserved.  Unpublished rights reserved             *
**            under the copyright laws of the United States.                *
**                                                                          *
**            The software contained on this media is proprietary           *
**            to and embodies the confidential technology of                *
**            Digital Equipment Corporation.  Possession, use,              *
**            duplication or dissemination of the software and              *
**            media is authorized only pursuant to a valid written          *
**            license from Digital Equipment Corporation.                   *
**                                                                          *
**            RESTRICTED RIGHTS LEGEND   Use, duplication, or               *
**            disclosure by the U.S. Government is subject to               *
**            restrictions as set forth in Subparagraph (c)(1)(ii)          *
**            of DFARS 252.227-7013, or in FAR 52.227-19, as                *
**            applicable.                                                   *
**                                                                          *
****************************************************************************/

/*
 * RCS:
 *    $Header: /usr/sde/osf1/rcs/x11/src/extensions/lib/smt/XSmtint.h,v 1.1.4.2 1993/09/03 20:28:31 Dave_Hill Exp $
 */


#ifndef XSMTINT_H
#define XSMTINT_H

#define SmtGetReq(name, req) \
	if ((smtDisplayPtr)dpy->pSmt) \
	{ \
	    register smtDisplayPtr pSmt = (smtDisplayPtr)dpy->pSmt; \
	    int status =	\
	    ipAllocateData(&pSmt->chan, dpy->max_request_size, &dpy->buffer); \
	    if (status != IP_SUCCESS) _SmtIpError(dpy, pSmt, status); \
	    req = (xSmt/**/name/**/Req *) dpy->buffer; \
	    req->reqType = smtExtCodes[ConnectionNumber(dpy)]->major_opcode; \
	    req->smtReqType = X_Smt/**/name; \
	    req->length = (SIZEOF(xSmt/**/name/**/Req))>>2; \
	    dpy->bufptr = dpy->buffer + SIZEOF(xSmt/**/name/**/Req); \
	    dpy->bufmax = dpy->buffer + (dpy->max_request_size << 2); \
	    dpy->request++; \
	} \
	else \
	{ \
	    WORD64ALIGN\
	    if ((dpy->bufptr + SIZEOF(xSmt/**/name/**/Req)) > dpy->bufmax)\
		    _XFlush(dpy);\
	    req = (xSmt/**/name/**/Req *)(dpy->last_req = dpy->bufptr);\
	    req->reqType = smtExtCodes[ConnectionNumber(dpy)]->major_opcode;\
	    req->smtReqType = X_Smt/**/name; \
	    req->length = (SIZEOF(xSmt/**/name/**/Req))>>2;\
	    dpy->bufptr += SIZEOF(xSmt/**/name/**/Req);\
	    dpy->request++;\
	}

#endif	/* XSMTINT_H */
