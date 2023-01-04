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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: newgrp_sec.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 17:18:33 $";
#endif
/*
 * HISTORY
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 */
/*
 * OSF/1 1.2
 */

#include <sys/secdefines.h>

#if SEC_BASE

/* #ident "@(#)newgrp_sec.c	5.1 17:50:48 8/15/90 SecureWare" */

/*
 * Based on:	newgrp_sec.c	2.1 11:11:03 1/25/89
 */

#include <sys/security.h>
#include <prot.h>

/*
 * Prompt for and encrypt a password and return 1 if the password matches
 * the supplied cipher text, and return 0 if not.  Erase the cleartext
 * buffer before returning.
 */
int
newgrp_good_password(cipher, message)
	char *cipher;
	char *message;
{
	register int is_good;

	is_good = (strcmp(bigcrypt(getpasswd(message, AUTH_MAX_PASSWD_LENGTH),
				   cipher),
			  cipher) == 0);

	/*
	 * Clear the cleartext left in the static buffer in getpasswd().
	 */
	(void) getpasswd((char *) 0, AUTH_MAX_PASSWD_LENGTH);

	return is_good;
}
#endif
