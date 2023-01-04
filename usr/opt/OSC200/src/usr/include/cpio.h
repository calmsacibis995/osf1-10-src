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
 *	@(#)$RCSfile: cpio.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:14:14 $
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
 * COMPONENT_NAME: cpio.h
 *                                                                    
 * ORIGIN: IBM
 *
 * Copyright International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * cpio.h	1.2  com/inc,3.1,9021 3/29/90 15:01:16
 */                                                                   
#ifndef _CPIO_H_
#define _CPIO_H_

#define	C_IRUSR		0000400
#define	C_IWUSR		0000200
#define	C_IXUSR		0000100
#define	C_IRGRP		0000040
#define	C_IWGRP		0000020
#define	C_IXGRP		0000010
#define	C_IROTH		0000004
#define	C_IWOTH		0000002
#define	C_IXOTH		0000001
#define	C_ISUID		0004000
#define	C_ISGID		0002000
#define	C_ISVTX		0001000
#define	C_ISDIR		0040000
#define	C_ISFIFO	0010000
#define	C_ISREG		0100000
#define	C_ISBLK		0060000
#define	C_ISCHR		0020000
#define	C_ISCTG		0110000
#define	C_ISLNK		0120000
#define	C_ISSOCK	0140000

#define	MAGIC		"070707"

#endif  /* _CPIO_H_ */
