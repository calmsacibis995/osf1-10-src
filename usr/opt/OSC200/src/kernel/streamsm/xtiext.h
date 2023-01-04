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
 * @(#)$RCSfile: xtiext.h,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/10/06 13:51:56 $
 */

#ifndef _XTIEXT_H
#define _XTIEXT_H

#ifdef OSI
#include <netosi/osi.h>
#endif


/*
 *
 * Structure used for manipulating socket options though XTI ioctl
 * (actually a pass-through to sosetopt and sogetopt)
 *
 * Current level(s) supported (from socket.h):
 *	SOL_SOCKET
 *
 * Current options supported (from socket.h):
 *	SO_DEBUG        flag           /* turn on debugging info recording *
 *	SO_ACCEPTCONN   flag           /* socket has had listen() *
 *	SO_REUSEADDR    flag           /* allow local address reuse *
 *	SO_KEEPALIVE    flag           /* keep connections alive *
 *	SO_DONTROUTE    flag           /* just use interface addresses *
 *	SO_BROADCAST    flag           /* permit sending of broadcast msgs *
 *	SO_USELOOPBACK  flag           /* bypass hardware when possible *
 *	SO_OOBINLINE    flag           /* leave received OOB data in line *
 *	SO_LINGER       struct linger  /* linger on close if data present *
 *	SO_SNDBUF       int            /* send buffer size *
 *	SO_RCVBUF       int            /* receive buffer size *
 *	SO_SNDLOWAT     int            /* send low-water mark *
 *	SO_RCVLOWAT     int            /* receive low-water mark *
 *	SO_SNDTIMEO     int            /* send timeout *
 *	SO_RCVTIMEO     int            /* receive timeout *
 *	SO_ERROR        int            /* get error status and clear *
 *	SO_TYPE         int            /* get socket type *
 *
 */

struct xtisoioctl {
	int		level;	  /* level where options are manipulated */
	int		optname;  /* Actual option to be manipulated */
	struct netbuf	opt;	  /* buffer for options value */
};

/*
 * ioctl commands for XTI are encoded as follows:
 *
 *  -------------------------------------------------
 * |    outlen     |     inlen     |group|  command  |
 *  -------------------------------------------------
 *        12              12          4        8
 *
 * Group is ['A'-'Z'] (although only 4 LSBits are used).
 * command is any number fron 0 to 256 (unique within the group);
 *	if command is a pass through to the generic socket ioctl command,
 *	then the command would be the number for the generic ioctl command.
 * outlen is the length of the struct returned as a result.
 * inlen is the length of the struct passed as input to the ioctl command.
 *
 */
/* XTI ioctl command breakup */
#define XTIOCPARM_MASK		0xff          /* 8 bits */
#define XTIOCLEN_MASK		0x3ff         /* 10 bits */
#define XTIOCGROUP_MASK		0x0f          /* 4 bits */

/* XTI ioctl command construction */
#define _XTIOC(cmd,group,inlen,outlen)		\
	((cmd) | 				\
	 ((XTIOCGROUP_MASK & (group)) << 8) | 	\
	 ((inlen) << 12) | 		\
	 ((outlen) << 22)			\
	)

#ifdef _KERNEL
#define XTIOCBASECMD(x)		( (x)       & XTIOCPARM_MASK   )
#define XTIOCGROUP(x)		( ((x)>>8)  & XTIOCGROUP_MASK  )
#define XTIOCPARM_INLEN(x)	( ((x)>>12) & XTIOCLEN_MASK    )
#define XTIOCPARM_OUTLEN(x)	( ((x)>>22) & XTIOCLEN_MASK    )

#endif /* _KERNEL */

/*
 * Commands for ioctl, X block (Generic XTI/socket level ioctl commands)
 *
 * NOTE: These commands use xtisoioctl for the parameter passing.
 *       It takes care of the lengths (strbuf style) for input output
 *       Hence the inlen and outlen is ignored
 *
 */
#define XTI_SOSETOPT	_XTIOC(1, 'X', 0, 0)
#define XTI_SOGETOPT	_XTIOC(2, 'X', 0, 0)

/* Commands for ioctl, I block (Generic IOCTL supported on the socket) */

/* Commands for ioctl, T block (TCP/IP Protocol specific) */

#ifdef OSI
/* Commands for ioctl, O block (OSI Protocol specific) */
#define XTI_OSISETXTITEMPLATE	  _XTIOC(TOPT_SXTITEMPLATE,\
					 'O',\
					 sizeof(struct topt_xtitemplate),\
					 0)
#define XTI_OSIGETNETSVCTEMPLATE  _XTIOC(TOPT_GTEMPLATE_NETSVC,\
					 'O',\
					 sizeof(struct topt_template_netsvc),\
					 sizeof(struct topt_template_netsvc))
#define XTI_OSIGETXTINETSVC	  _XTIOC(TOPT_GXTINETSVC,\
					 'O',\
					 sizeof(enum topt_netsvc),\
					 sizeof(enum topt_netsvc))

#endif	/* ifdef OSI */

#endif /* _XTIEXT_H */
