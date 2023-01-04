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
 *	@(#)$RCSfile: tar.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:19:20 $
 */ 
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
 * COMPONENT_NAME: tar.h
 *                                                                    
 * ORIGIN: IBM
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   


#ifndef _TAR_H_
#define _TAR_H_
#include <standards.h>

/*
 * POSIX required that certain values be included in tar.h.  It also requires
 * that if _POSIX_SOURCE is defined then only those standard specific values
 * be present.  This header includes all the POSIX required entries.
 */

#ifdef _POSIX_SOURCE

#define TMAGIC		"ustar"		/* ustar and a null */
#define TMAGLEN		6		  
#define TVERSION	"00"		/* 00 and no null */
#define TVERSLEN	2

/* Values used in typeflag field */
#define REGTYPE		'0'		/* regular file */
#define AREGTYPE	'\0'		/* regular file */
#define LNKTYPE		'1'		/* line         */
#define SYMTYPE		'2'		/* reserved     */
#define CHRTYPE		'3'		/* character special */
#define BLKTYPE 	'4'		/* block special */
#define DIRTYPE		'5'		/* directory */
#define FIFOTYPE	'6'		/* FIFO special */
#define CONTTYPE	'7'		/* reserved */

/* Bits used in the mode field - values in octal */
#define TSUID		04000		/* set UID on execution */
#define TSGID		02000		/* set GID on execution */
#define TSVTX		01000		/* reserved 	   	*/
#define TUREAD		00400		/* read by owner	*/
#define TUWRITE		00200		/* write by owner	*/
#define TUEXEC		00100		/* execute/search by owner */
#define TGREAD		00040		/* read by group 	*/
#define TGWRITE		00020		/* write by group 	*/
#define TGEXEC		00010		/* execute/search by group */
#define TOREAD		00004		/* read by other 	*/
#define TOWRITE		00002		/* write by other 	*/
#define	TOEXEC		00001		/* execute/search by other */

#endif /* _POSIX_SOURCE */
#endif /* _TAR_H_ */
