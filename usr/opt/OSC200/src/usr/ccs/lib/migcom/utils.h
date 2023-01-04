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
 *	@(#)$RCSfile: utils.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:23:09 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	_UTILS_H_
#define	_UTILS_H_

/* stuff used by more than one of header.c, user.c, server.c */

extern void WriteImport(/* FILE *file, string_t filename */);
extern void WriteRCSDecl(/* FILE *file, identifier_t name, string_t rcs */);
extern void WriteBogusDefines(/* FILE *file */);

extern void WriteList(/* FILE *file, argument_t *args,
			 void (*func)(FILE *file, argument_t *arg),
			 u_int mask, char *between, char *after */);

extern void WriteReverseList(/* FILE *file, argument_t *args,
				void (*func)(FILE *file, argument_t *arg),
				u_int mask, char *between, char *after */);

/* good as arguments to WriteList */
extern void WriteNameDecl(/* FILE *file, argument_t *arg */);
extern void WriteVarDecl(/* FILE *file, argument_t *arg */);
extern void WriteTypeDecl(/* FILE *file, argument_t *arg */);
extern void WriteCheckDecl(/* FILE *file, argument_t *arg */);

extern char *ReturnTypeStr(/* routine_t *rt */);

extern char *FetchUserType(/* ipc_type_t *it */);
extern char *FetchServerType(/* ipc_type_t *it */);
extern void WriteFieldDeclPrim(/* FILE *file, argument_t *arg,
				  char *(*tfunc)(ipc_type_t *it) */);

extern void WriteStructDecl(/* FILE *file, argument_t *args,
			       void (*func)(FILE *file, argument_t *arg),
			       u_int mask, char *name */);

extern void WriteStaticDecl(/* FILE *file, ipc_type_t *it,
			       boolean_t dealloc, boolean_t longform,
			       identifier_t name */);

extern void WriteCopyType(/* FILE *file, ipc_type_t *it,
			     char *left, char *right, ... */);

extern void WritePackMsgType(/* FILE *file, ipc_type_t *it,
				boolean_t dealloc, boolean_t longform,
				char *left, char *right, ... */);

#endif	/* _UTILS_H_ */
