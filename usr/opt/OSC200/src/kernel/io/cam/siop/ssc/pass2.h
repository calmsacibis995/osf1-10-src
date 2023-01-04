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
/********************************************************************/
/*                    Compiler Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes                             */
/*	 ----   ----        ----------------									  */
/*  bsb   9/7/90       1.0	/ Initial general customer release       */
/********************************************************************/
/*                    pass2.h Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes									  */
/*  ----   ----        -----------------									  */
/*  bsb   9/7/90       1.0 / Initial general customer release       */
/********************************************************************/

#ifndef _SSC_PASS2_H_
#define _SSC_PASS2_H_


#include        "portable.h"
#include        <stdio.h>
#include        <string.h>

char            *Scanner;
char            *IntermCode;
FILE            *outfile;
FILE            *IntermFile;


#define DEBUG         0 
#define VOID            void
#define FALSE           0
#define TRUE            1
#define EXIT            1
#define ENTRI           0
#define EOLP            0
#define SS_EOF          1
#define IDENTIFY        2
#define VARIABLE        3
#define NUMBER          4
#define PASSIFY         5
#define END             6
#define SEEK_END        2
#define E_EXP_NUM       0
#define E_EXP_IDENT     1
#define FATAL           5
#define OK              0

char    pass_chars[] = "ABCDEFabcdef0123456789";
char    token_chars[] = "!@#$%^&*()_-+=~`\":;{}[,]<>?./";
char    TokenName[32];
int     TokenType;
int  HandlePass();
char Get1Char();
void PushBack();
int  GetTokenType();
BOOL InstStart;
BOOL InstEnd;
BOOL GetToken();
BOOL GenDefine();
BOOL GenArray();
BOOL InstArray();
BOOL HandleVar();
BOOL HandleExternal();
BOOL HandleCounts();
void FuncTracer();
BOOL ishexdigit();
BOOL isdigit();
void HandleIdent();
BOOL BeginConversion();
BOOL ReadIntermediate();
BOOL ParseCmdLine ();
void SysError();
void OpenFile();
int  GetFileSize();
void DoInstructions();
void DoPatches();
void DisplayRetVal();
void PushBackToken();
BOOL CompareString();
char Upper();
BOOL Line;
int count;

#endif 
