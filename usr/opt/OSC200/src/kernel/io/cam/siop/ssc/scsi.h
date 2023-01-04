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
/*                    SCSI.h Revisions										  */
/*																						  */
/*  init   Date        Revision/Changes									  */
/*  ----   ----        -----------------									  */
/*  bsb   9/7/90       1.0 / Initial general customer release       */
/********************************************************************/


/*
    SCSI.H - Header file for SCSI script compiler
*/

#ifndef SCSI

#define SCSI

#include "portable.h"
#include "error.h"
#include <stdio.h>

typedef struct {
    BOOL defined;
    INT32 value;
    struct sym_s *sym;
}   expr_t;

#define MAX_LEX_IN_LEN 256
#define MAX_IDENT_LEN 32

extern void print_symbols();
extern int all_defined();
extern void error();
extern void pass2();
extern void pass();
extern void patch();
extern void sub_patch();
extern void init_yacc();
extern void init_line();
extern char *scsi_malloc();
extern void post_compile_printing();
extern char lex_buffer[];
extern int ErrorChar;
extern int line_no;
extern FILE *input_file;
extern int iteration_limit;
extern int iteration_num;
extern BOOL compile_flag;
extern int ErrorCount;
extern int WarningCount;
extern int InstructionCount;

extern BOOL lis_flag;		 /* List File Output */
extern BOOL err_flag;       /* Error file output */
extern BOOL out_flag;		 /* Generate output file */
extern BOOL undefine_flag;  /* Flag to undefine Instructions and Count */
extern BOOL deb_flag;       /* Standard (Ballard) pass one output */
extern BOOL verbose_flag;   /* Write helpful info to stdout during compile */
extern FILE *lisfile;
extern FILE *debfile;
extern FILE *outfile;
extern FILE *errfile;


typedef struct key_s {
    char            *name;      /* Text for matching keyword */
    int             token;      /* Token ID that gets passed on */
    INT32           val;        /* Additional value passed on through yylval variable */
}   keyword_t;

typedef struct sym_s {
    char            name[MAX_IDENT_LEN+1];   /* Symbol name */
    INT32           value;      /* Assigned value */
    int             type;       /* Identifier type (ABSOLUTE, EXTERNAL, etc) */
    int             entry;      /* Is label an entry point? */
    BOOL            defined;    /* Defined flag */
    int             pass;       /* Use to track multiple declarations */
    struct sym_s    *next;      /* Link to next entry in table */
    struct patch_s  *patch;     /* Link to patch entries */
    struct patch_s  *endpatch;
}   symbol_t, *symbol_pt;


typedef struct patch_s {
    UINT32          addr;       /* Location of patch */
    int             width;      /* Width of patch */
    struct patch_s  *next;      /* Link to next entry */
}   patch_t, *patch_pt;


extern symbol_t    *SymTableStart;
extern symbol_t    *SymTableEnd;  
extern symbol_t    *ScriptPatches;


#endif
