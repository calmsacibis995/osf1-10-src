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
 * @(#)$RCSfile: pcxas.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/06/24 22:44:02 $
 */

#ifndef _PCXAS_H_
#define _PCXAS_H_

#define PCXAS_LEFT_BUTTON	0x01
#define PCXAS_RIGHT_BUTTON	0x02
#define PCXAS_MIDDLE_BUTTON	0x04

#define PCXAS_X_SIGN		0x10
#define PCXAS_Y_SIGN		0x20

#define PCXAS_X_OVER		0x40
#define PCXAS_Y_OVER		0x80

#ifdef KERNEL
caddr_t pcxas_init_closure();
int pcxas_init_pointer();
void pcxas_reset_pointer();
void pcxas_enable_pointer();
void pcxas_disable_pointer();
int pcxas_tablet_event();
int pcxas_mouse_event();
void pcxas_set_tablet_overhang();
#endif /* KERNEL */

#endif /*_PCXAS_H_*/

