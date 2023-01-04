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
 * @(#)$RCSfile: collation.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 22:42:09 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#include <netinet/in.h>

extern int	_getcolval( _LC_collate_t*, wchar_t *, wchar_t, const char *, int);
extern int	_wc_getcolval(_LC_collate_t*, wchar_t *, wchar_t, const wchar_t *, int);

extern wchar_t	_mbucoll(_LC_collate_t *, char *, char **);

extern char	*__do_replacement(_LC_collate_t *, const char *, int);

extern int	__forward_collate_std(_LC_collate_t *, char *, char *, int);
extern int	__forward_collate_sb(_LC_collate_t *, char *, char *, int);

extern int	__backward_collate_std(_LC_collate_t *, char *, char *, int );
extern int	__backward_collate_sb(_LC_collate_t *, char *, char *, int );

extern int	__back_pos_collate_sb(_LC_collate_t *, char *, char *, int );
extern int	__back_pos_collate_std(_LC_collate_t *, char *, char *, int );

extern int	__forw_pos_collate_sb(_LC_collate_t *, char *, char *, int );
extern int	__forw_pos_collate_std(_LC_collate_t *, char *, char *, int );

extern int 	__char_collate_std(_LC_collate_t *, char *, char *);
extern int	__char_collate_sb( _LC_collate_t *, char *, char *);

/*
 * collation_to_byte_string - converts a collation weight from a integral value to
 *	a byte string that can be used for string compares.
 */

#if __WCHAR_T_LEN == 4
#define	collation_to_byte_string htonl
#define byte_string_to_collation ntohl
#elif __WCHAR_T_LEN == 2
#define collation_to_byte_string htons
#define byte_string_to_collation ntohs
#endif /* __WCHAR_T_LEN */


