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
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak sys_errlist = __sys_errlist
#pragma weak sys_nerr = __sys_nerr
#endif
#endif
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: errlst.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/11/17 15:47:25 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: sys_errlist, sys_nerr
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * errlst.c	1.10  com/lib/c/gen,3.1,8943 10/30/89 00:34:04
 *
 *	Revision History:
 *
 * 14-May-91	Peter H. Smith
 *	Don't piggyback on EINPROGRESS.
 *
 * 03-May-91	Peter H. Smith
 *	Add POSIX 1003.4 Draft 10 errno definitions.
 */

/* errno lives here */
int errno;

char	*sys_errlist[] = {
	"Error 0",                             /* Corresponding errno */
	"Not owner",                           /*  EPERM          1 */
	"No such file or directory",           /*  ENOENT         2 */
	"No such process",                     /*  ESRCH          3 */
	"Interrupted system call",             /*  EINTR          4 */
	"I/O error",                           /*  EIO            5 */
	"No such device or address",           /*  ENXIO          6 */
	"Arg list too long",                   /*  E2BIG          7 */
	"Exec format error",                   /*  ENOEXEC        8 */
	"Bad file number",                     /*  EBADF          9 */
	"No child processes",                  /*  ECHILD         10 */
	"Resource deadlock avoided",           /*  EDEADLK        11 */
	"Not enough space",                    /*  ENOMEM         12 */
	"Permission denied",                   /*  EACCES         13 */
	"Bad address",                         /*  EFAULT         14 */
	"Block device required",               /*  ENOTBLK        15 */
	"Device busy",                         /*  EBUSY          16 */
	"File exists",                         /*  EEXIST         17 */
	"Cross-device link",                   /*  EXDEV          18 */
	"No such device",                      /*  ENODEV         19 */
	"Not a directory",                     /*  ENOTDIR        20 */
	"Is a directory",                      /*  EISDIR         21 */
	"Invalid argument",                    /*  EINVAL         22 */
	"File table overflow",                 /*  ENFILE         23 */
	"Too many open files",                 /*  EMFILE         24 */
	"Not a typewriter",                    /*  ENOTTY         25 */
	"Text file busy",                      /*  ETXTBSY        26 */
	"File too large",                      /*  EFBIG          27 */
	"No space left on device",             /*  ENOSPC         28 */
	"Illegal seek",                        /*  ESPIPE         29 */
	"Read-only file system",               /*  EROFS          30 */
	"Too many links",                      /*  EMLINK         31 */
	"Broken pipe",                         /*  EPIPE          32 */
	"Argument out of domain",              /*  EDOM           33 */
	"Result too large",                    /*  ERANGE         34 */

/* non-blocking and interrupt i/o */
	"Operation would block",		/* EWOULDBLOCK    35 */
     /* "Resource temporarily unavailable",     * EAGAIN         35 */
	"Operation now in progress",		/* EINPROGRESS    36 */
	"Operation already in progress",	/* EALREADY       37 */

/* ipc/network software */

	/* argument errors */
	"Socket operation on non-socket",	/* ENOTSOCK        38 */
	"Destination address required",		/* EDESTADDRREQ    39 */
	"Message too long",			/* EMSGSIZE        40 */
	"Protocol wrong type for socket",	/* EPROTOTYPE      41 */
	"Option not supported by protocol",	/* ENOPROTOOPT     42 */
	"Protocol not supported",		/* EPROTONOSUPPORT 43 */
	"Socket type not supported",		/* ESOCKTNOSUPPORT 44 */
	"Operation not supported on socket",	/* EOPNOTSUPP      45 */
	"Protocol family not supported",	/* EPFNOSUPPORT    46 */
	"Address family not supported by protocol family",
						/* EAFNOSUPPORT    47 */
	"Address already in use",		/* EADDRINUSE      48 */
	"Can't assign requested address",	/* EADDRNOTAVAIL   49 */

	/* operational errors */
	"Network is down",			/* ENETDOWN       50 */
	"Network is unreachable",		/* ENETUNREACH    51 */
	"Network dropped connection on reset",	/* ENETRESET      52 */
	"Software caused connection abort",	/* ECONNABORTED   53 */
	"Connection reset by peer",		/* ECONNRESET     54 */
	"No buffer space available",		/* ENOBUFS        55 */
	"Socket is already connected",		/* EISCONN        56 */
	"Socket is not connected",		/* ENOTCONN       57 */
	"Can't send after socket shutdown",	/* ESHUTDOWN      58 */
	"Too many references: can't splice",	/* ETOOMANYREFS   59 */
	"Connection timed out",			/* ETIMEDOUT      60 */
	"Connection refused",			/* EREFUSED       61 */
	"Too many levels of symbolic links",	/* ELOOP          62 */
	"File name too long",			/* ENAMETOOLONG   63 */
	"Host is down",				/* EHOSTDOWN      64 */
	"Host is unreachable",			/* EHOSTUNREACH   65 */
	"Directory not empty",			/* ENOTEMPTY      66 */
	"Too many processes",			/* EPROCLIM       67 */
	"Too many users",			/* EUSERS         68 */
	"Disc quota exceeded",			/* EDQUOT         69 */
	"Missing file or filesystem",           /* ESTALE         70 */
	"Item is not local to host",            /* EREMOTE        71 */
	"RPC structure is bad",			/* EBADRPC        72 */
	"RPC version is wrong",			/* ERPCMISMATCH   73 */
	"RPC program not available",		/* EPROGUNAVAIL   74 */
	"Program version wrong",		/* EPROGMISMATCH  75 */
	"Bad procedure for program",		/* EPROCUNAVAIL   76 */
	"No locks available",                   /* ENOLCK         77 */
	"Function not implemented",             /* ENOSYS         78 */
	"Error number 79 occurred",		/* ??????         79 */
	"No message of desired type",           /* ENOMSG         80 */
	"Identifier removed",                   /* EIDRM          81 */
	"Out of STREAMS resources",		/* ENOSR	  82 */
	"System call timed out",		/* ETIME	  83 */
	"Next message has wrong type",		/* EBADMSG	  84 */
	"Error in protocol",			/* EPROTO	  85 */
	"No message on stream head read q",	/* ENODATA	  86 */
	"fd not associated with a stream",	/* ENOSTR	  87 */
	"Tells open to clone the device",	/* ECLONEME	  88 */
	"Mounting a dirty fs w/o force",	/* EDIRTY	  89 */
	"Duplicate package name",		/* EDUPPKG	  90 */
	"Version mismatch",			/* EVERSION 	  91 */
	"Unresolved package name",		/* ENOPKG	  92 */
	"Unresolved symbol name",		/* ENOSYM	  93 */
	"Operation canceled",			/* ECANCELED	  94 */
	"Cannot start operation",		/* EFAIL	  95 */
	"Inappropriate operation for file type",/* EFTYPE	  96 */
	"Operation in progress",		/* EINPROG	  97 */
	"Too many timers",			/* EMTIMERS	  98 */
	"Function not implemented",		/* ENOTSUP	  99 */
	"Internal AIO operation complete",      /* EAIO          100 */
	"Error 101 occurred.",                  /* UNUSED        101 */
	"Error 102 occurred.",                  /* UNUSED        102 */
	"Error 103 occurred.",                  /* UNUSED        103 */
	"Error 104 occurred.",                  /* UNUSED        104 */
	"Error 105 occurred.",                  /* UNUSED        105 */
	"Error 106 occurred.",                  /* UNUSED        106 */
	"Error 107 occurred.",                  /* UNUSED        107 */
	"Error 108 occurred.",                  /* UNUSED        108 */
	"Error 109 occurred.",                  /* UNUSED        109 */
	"Error 110 occurred.",                  /* UNUSED        110 */
	"Error 111 occurred.",                  /* UNUSED        111 */
	"Error 112 occurred.",                  /* UNUSED        112 */
	"Error 113 occurred.",                  /* UNUSED        113 */
	"Error 114 occurred.",                  /* UNUSED        114 */
	"Error 115 occurred.",                  /* UNUSED        115 */
	"Illegal byte sequence"                 /* EILSEQ        116 */

};

int	sys_nerr = { sizeof(sys_errlist)/sizeof(sys_errlist[0]) };
