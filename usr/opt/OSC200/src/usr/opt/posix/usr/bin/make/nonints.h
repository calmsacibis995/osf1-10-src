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
 * @(#)$RCSfile: nonints.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:05:29 $
 */

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>

extern ReturnStatus	Arch_ParseArchive (char **, Lst, GNode *);
extern void	Arch_Touch (GNode *);
extern int	Arch_MTime (GNode *);
extern int	Arch_MemMTime (GNode *);
extern void	Arch_Init (void);
extern void	Compat_Run(Lst);
extern void	Dir_Init (void);
extern char *	Dir_FindFile (char *, Lst);
extern int	Dir_MTime (GNode *);
extern void	Dir_PrintDirectories(void);
extern void     DirAddDir (Lst, char *);
extern void	Job_Touch (GNode *, Boolean);
extern Boolean	Job_CheckCommands (GNode *, void (*)(const char *, ...));
extern void	Error (const char *, ...);
extern void	Fatal (const char *, ...);
extern void	Punt (const char *, ...);
extern void	*emalloc(size_t);
extern void	enomem(void);
extern int	Make_TimeStamp (GNode *, GNode *);
extern Boolean	Make_OODate (GNode *);
extern int	Make_HandleTransform (GNode *, GNode *);
extern void	Make_DoAllVar (GNode *);
extern void	Parse_Error (int, const char *, ...);
extern Boolean	Parse_IsVar (char *);
extern void	Parse_DoVar (char *, GNode *);
extern void	Parse_File(char *, FILE *);
extern void	Parse_Init(void);
extern Lst	Parse_MainName(void);
extern char *	Str_Concat (char *, char *, int);
extern char ** Str_Break(char *, char *, int *);
extern char *	Str_FindSubstring(char *, char *);
extern void     Suff_ClearSuffixes (void);
extern Boolean	Suff_IsTransform (char *);
extern GNode *	Suff_AddTransform (char *);
extern int	Suff_EndTransform (GNode *, ...);
extern void     Suff_AddSuffix (char *);
extern void     Suff_FindDeps (GNode *);
extern void     Suff_Init (void);
extern void     Suff_PrintAll(void);
extern void	Targ_Init (void);
extern GNode *	Targ_NewGN (char *);
extern GNode *	Targ_FindNode (char *, int);
extern Lst	Targ_FindList (Lst, int);
extern Boolean	Targ_Ignore (GNode *);
extern Boolean	Targ_Silent (GNode *);
extern Boolean	Targ_Precious (GNode *);
extern void	Targ_SetMain (GNode *);
extern int	Targ_PrintCmd (char *, ...);
extern char *	Targ_FmtTime (time_t);
extern void	Targ_PrintType (int, ...);
extern void	Targ_PrintGraph(int);
extern void	Var_Set (char *, char *, GNode *);
extern void	Var_Append (char *, char *, GNode *);
extern Boolean	Var_Exists(char *, GNode *);
extern char *	Var_Value (char *, GNode *);
extern char *	Var_Parse (char *, GNode *, Boolean, int *, Boolean *);
extern char *	Var_Subst (char *, GNode *, Boolean);
extern void	Var_Init (void);
extern void	Var_Dump(GNode *);
