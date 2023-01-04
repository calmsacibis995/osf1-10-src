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
 *	@(#)$RCSfile: handler.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:50 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * Copyright 1990 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Module Function:
 *	Interrupt Handler service routine header file to support loadable
 *		device drivers.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _HANDLER_H
#define _HANDLER_H

extern ihandler_id_t	*handler_add(/* ihandler_t *ih; */);
extern int		handler_del(/* ihandler_id_t *id; */);
extern int		handler_enable(/* ihandler_id_t *id; */);
extern int		handler_disable(/* ihandler_id_t *id; */);
extern int		handler_state(/* ihandler_id_t *id; int state, set; */);

#endif /*_HANDLER_H*/
