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
/************************************************************************
 *									*
 *			Copyright (c) 1989 by				*
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

/*
 *  support for Maxine Baseboard Graphics
 *  Joel Gringorten
 */

#ifndef _PMAGDV_H_
#define _PMAGDV_H_

#define PMAGDV_FB_OFFSET		0x1C1E0000

#define PMAGDV_SSR			0x1C040100
#define PMAGDV_VIDEO_BUFFER		0x1C140000
#define PMAGDV_FRAME_BUFFER		0xA000000
#define PMAGDV_VDAC_WRITE		0x1C1E0000
#define PMAGDV_VDAC_READ		0x1C1C0000

#define PMAGDV_SSR_BASE		    (*(u_int *)PHYS_TO_K1(PMAGDV_SSR))
#define PMAGDV_SSR_ADDR		    &PMAGDV_SSR_BASE
#define PMAGDV_VIDEO_BUFFER_BASE    (*(u_int *)PHYS_TO_K1(PMAGDV_VIDEO_BUFFER))
#define PMAGDV_VIDEO_BUFFER_ADDR    &PMAGDV_VIDEO_BUFFER_BASE

#define PMAGDV_VDAC_WRITE_BASE	     (*(u_int *) PHYS_TO_K1(PMAGDV_VDAC_WRITE))
#define PMAGDV_VDAC_WRITE_ADDR      &PMAGDV_VDAC_WRITE_BASE

#define PMAGDV_VDAC_READ_BASE	     (*(u_int *) PHYS_TO_K1(PMAGDV_VDAC_READ))
#define PMAGDV_VDAC_READ_ADDR       &PMAGDV_VDAC_READ_BASE

int pmagdv_attach();
void pmagdv_interrupt();
void pmagdv_enable_interrupt();
caddr_t pmagdv_init_closure();
caddr_t pmagdv_screen_init_closure();

#endif
