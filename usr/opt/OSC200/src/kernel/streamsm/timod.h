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
 *	@(#)$RCSfile: timod.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/05/12 15:15:46 $
 */ 

/** Copyright (c) 1990  Mentat Inc.
 ** timod.h 2.2, last change 3/1/91
 **/

#ifndef _TIMOD_H
#define	_TIMOD_H

/* Ioctl values for timod requests */
#define	TI_BIND			(('T'<<8)|1)
#define	TI_GETINFO		(('T'<<8)|2)
#define	TI_OPTMGMT		(('T'<<8)|3)
#define	TI_UNBIND		(('T'<<8)|4)
#define	TI_GETMYNAME		(('T'<<8)|5)
#define	TI_GETPEERNAME		(('T'<<8)|6)

/* Extra values for XTI */
#define	TI_XTI_HELLO		(('T'<<8)|7)	/* This is an XTI stream, not a TLI stream */
#define	TI_XTI_GET_STATE	(('T'<<8)|8)	/* Retrieve state saved for XTI */
#define	TI_XTI_CLEAR_EVENT	(('T'<<8)|9)	/* Clear disconnect or orderly release indication */

#define TI_XTI_MODE		(('T'<<8)|10)
#define TI_TLI_MODE		(('T'<<8)|11)

/* Return structure for TI_XTI_GET_STATE request */
typedef struct xti_state_s {
	unsigned int	xtis_qlen;	/* Saved qlen parameter from t_bind */
} XTIS, * XTISP;

#endif	/* _TIMOD_H */
