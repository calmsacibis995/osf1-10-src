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
 * @(#)$RCSfile: lib_data.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 01:02:03 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/* @(#)$RCSfile: lib_data.h,v $ $Revision: 1.1.6.2 $ (OSF) $Date: 1993/06/08 01:02:03 $ */
/*
 * Library macros/ types for "thread" data access
 */

typedef struct lib_data_functions {
	int	(*data_hdl)(void **);
	void	*(*data_ref)(void *);
} lib_data_functions_t;

#define	LIB_DATA_FUNCTION(datastruct, operation, arg) \
	((datastruct).operation ? (*(datastruct).operation)(arg) : 0)

#define	lib_data_hdl(datastruct, hdl) \
	LIB_DATA_FUNCTION(datastruct, data_hdl, hdl)

#define	lib_data_ref(datastruct, hdl) \
	LIB_DATA_FUNCTION(datastruct, data_ref, hdl)

