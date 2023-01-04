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
 *	@(#)$RCSfile: access.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/05/03 22:51:55 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * ORIGINS: 27, 3
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _SYS_ACCESS_H_
#define _SYS_ACCESS_H_

#include <standards.h>

/* POSIX does not define access.h, however, certain values in access.h
 * are required to be included by unistd.h when _POSIX_SOURCE is defined.
 * Therefore, these values are confined within POSIX ifdefs.
 */

#ifdef _POSIX_SOURCE
/*
 *  BSD defines
 */
#define	F_OK	00		/* E_ACC does file exist */
#define	X_OK	01		/* X_ACC is it executable by caller */
#define	W_OK	02		/* W_ACC writable by caller */
#define	R_OK	04		/* R_ACC readable by caller */

#endif /* _POSIX_SOURCE */

#ifdef _OSF_SOURCE

#define R_ACC	04	/* read */
#define W_ACC	02	/* write */
#define X_ACC	01	/* execute (search) */
#define E_ACC	00	/* check existence of file */
#define NO_ACC	00	/* no access rights */


#define EFF_ONLY_OK     010	/* for fdfs and access() */

#endif /* _OSF_SOURCE */
#endif /* _SYS_ACCESS_H_ */
