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
 * special socket routine for hp
 */

#include <sys/types.h>
#include <sys/socket.h>

int
set_socket_option (socket_id, option)
int socket_id;
char option;
{
	int optlen = 1;
	char optval = 0x0;

	getsockopt (socket_id, SOL_SOCKET, option, &optval, &optlen);

	optval |= option;

	setsockopt (socket_id, SOL_SOCKET, option, &optval, 1);
}


int
unset_socket_option (socket_id, option)
int socket_id;
char option;
{
	int optlen = 1;
	char optval = 0x0;

	getsockopt (socket_id, SOL_SOCKET, option, &optval, &optlen);

	optval &= ~option;

	setsockopt (socket_id, SOL_SOCKET, option, &optval, 1);
}
