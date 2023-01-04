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
 * @(#)$RCSfile: eucioctl.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 19:02:03 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

#ifndef _SYS_EUCIOCTL_H_
#define _SYS_EUCIOCTL_H_

/*
 * EUC ioctl's
 */
#define EUC_WSET	_IO('J', 0x81)	/* set code widths */
#define EUC_WGET	_IO('J', 0x82)	/* get code widths */
#define EUC_IXLON	_IO('J', 0x83)	/* input conversion on */
#define EUC_IXLOFF	_IO('J', 0x84)	/* input conversion off */
#define EUC_OXLON	_IO('J', 0x85)	/* output conversion on */
#define EUC_OXLOFF	_IO('J', 0x86)	/* output conversion off */
#define EUC_MSAVE	_IO('J', 0x87)	/* save mode, go to ASCII mode */
#define EUC_MREST	_IO('J', 0x88)	/* restore mode */

struct eucioc {
	unsigned char eucw[4];	/* memory width of code sets */
	unsigned char scrw[4];	/* screen width of code sets */
};
typedef struct eucioc	eucioc_t;

#endif /* _SYS_EUCIOCTL_H_ */

