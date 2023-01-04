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
 *	@(#)$RCSfile: Utils.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:01:29 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifdef SEC_BASE

#ifndef _Utils_H_
#define _Utils_H_

/*
	filename:
		Utils.h
	
	copyright:
		Copyright (c) 1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		common include file for Utils.c
*/


int	IsPrIsso();
int	IsIsso();
int	IsPrRetired();
int	IsPrLocked();

char	*Realloc();
char	*Malloc();
char	*Calloc();
void	Free();

char	*MallocChar();
int	*MallocInt();


void	MemoryError();
int	stricmp();
int	strincmp();
char	*strstr();
char	*strdup();

char	**alloc_cw_table();
char	**expand_cw_table();
void	free_cw_table();
void	sort_cw_table();
char	**alloc_table();

int	TimeToWeeks();
int	TimeToDays();

void	GetAllGroups();
int	GetUserByName();
int	GetUserByUID();
void	GetAllUsers();
void	GetAllIssoUsers();
void	GetAllNonIssoUsers();
void	GetAllDevices();
void	GetAllTerminals();
void	GetAllPrinters();
void	GetAllRemovables();
void	GetAllHosts();

char	*internet_to_hostname();
char	*hostname_to_internet();

FILE	*popen_all_output();
void	pclose_all_output();

int	InvalidUser();

#endif /* _Utils_H_ */
#endif /* SEC_BASE */
