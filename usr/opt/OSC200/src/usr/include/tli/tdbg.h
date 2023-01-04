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
 *	@(#)$RCSfile: tdbg.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:22:56 $
 */ 

#include <standards.h>

extern void tr_accept __( (int fd, int resfd, struct t_call *call, int code) );
extern void tr_allocate __( (int fd, int struct_type, int fields, char *code) );
extern void tr_bind __( (int fd, struct t_bind *req, struct t_bind *ret, int code) );
extern void tr_close __( (int fd, int code) );
extern void tr_connect __( (int fd, struct t_call *sndcall, struct t_call *rcvcall,
			int code) );
extern void tr_free __( (int struct_type, int code) );
extern void tr_getinfo __( (int fd, struct t_info *info, int code) );
extern void tr_listen __( (int fd, struct t_call *call, int code) );
extern void tr_look __( (int fd, int code) );
extern void tr_open __( (char* name, int oflag, struct t_info *tinfo, int code) );
extern void tr_optmgmt __( (int fd, struct t_optmgmt *req, struct t_optmgmt *ret, 
			int code) );
extern void tr_rcv __( (int fd, char *buf, unsigned nbytes, int *flags, int code) );
extern void tr_rcvconnect __( (int fd, struct t_call *call, int code) );
extern void tr_rcvdis __( (int fd, struct t_discon *discon, int code) );
extern void tr_rcvrel __( (int fd, int code) );
extern void tr_rcvudata __( (int fd, struct t_unitdata *ud, int *flags, int code) );
extern void tr_rcvuderr __( (int fd, struct t_uderr *uderr, int code) );
extern void tr_snd __( (int fd, char *buf, unsigned nbytes, int flags, int code) );
extern void tr_snddis __( (int fd,struct t_call *call, int code) );
extern void tr_sndrel __( (int fd, int code) );
extern void tr_sndudata __( (int fd, struct t_unitdata *ud, int code) );
extern void tr_sync __( (int fd, int code) );
extern void tr_unbind __( (int fd, int code) );

