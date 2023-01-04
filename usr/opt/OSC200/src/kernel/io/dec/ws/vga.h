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
 * @(#)$RCSfile: vga.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/07/30 18:35:48 $
 */

#ifndef _VGA_H_
#define _VGA_H_

/* FIXME - default attribute for VGA adapters which use character attributes */
/* NOTE: attribute byte is 0xBF, where B is bg index, and F is fg index */
#define VGA_ATTR_NORMAL		0x01	/* black bg, white fg */
#define VGA_ATTR_REVERSE	0x10	/* white bg, black fg */

#define VGA_FB_ADDR		0x0a0000
#define VGA_TX_ADDR		0x0b8000

extern vm_offset_t get_io_handle(); /* FIXME - get this in a header file */

/* FIXME - default attribute for VGA adapters which use character attributes */
#define DEFAULT_ATTRIBUTE 0x01	/* black bg, white fg */

#define GET_IO_STRIDE(w,t) \
	(get_io_handle((w),1,(t)) ^ get_io_handle(0,1,(t)))

/* FIXME FIXME - do we want to continue this??? */
/* FIXME FIXME - LOADGR referenced in vga.c and vga_data.c */

#ifndef LOADGR
#define LOADGR  0                       /* Load glyphs for GR.          */
#endif /* LOADGR */

extern int vga_probe();
extern int vga_attach();
extern void vga_interrupt();
extern void vga_bot();

#endif /* _VGA_H_ */
