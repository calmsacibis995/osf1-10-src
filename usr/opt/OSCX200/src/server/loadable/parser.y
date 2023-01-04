%{
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
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1992 BY            			    *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/
#include "loadable.h"

int level = 0; 		/* current context  */
int lines = 1; 		/* current line number */

LibraryTypes CurSection; /* current section context */

int CurSectionIndex=0;	/* index into current section list */

int * LibListCounts;	/* list of number of libs in each section */
LS_LibraryReq ** Libs;	

char * LdLibraryPath = (char *)NULL;	/* for LD_LIBRARY_PATH */

char ** ConfigArgv;
int 	ConfigArgc;

/* scratch areas */
char **ArgsScratch;
int ArgsIndex=0;

extern Boolean ls_core_replace;

extern void AddLibReq();

#define FillLibReq(ptr) \
    switch ( ArgsIndex ) { 						    \
	case 4: CopyString(ptr->DeviceName, 	ArgsScratch[3]);	    \
	case 3: CopyString(ptr->ProcName, 	ArgsScratch[2]);	    \
	case 2: CopyString(ptr->LibFileName, 	ArgsScratch[1]);	    \
	case 1: CopyString(ptr->LibName,	ArgsScratch[0]);	    \
	default: break;							    \
    }									    \
    switch ( ArgsIndex ) {						    \
	case 1: /* make up the .so libname based on lib name */		    \
	    SprintfString( ptr->LibFileName, "lib%s.so", ArgsScratch[0]);   \
	case 2: /* init proc name is null if not specified */		    \
	    ptr->ProcName = NULL;					    \
	case 3: /* device name is null if not specified */		    \
	    ptr->DeviceName = NULL;					    \
	default: break;							    \
    }									    \
    ResetArgs()

%}

%union {
	int integer;
	char * string;
	int cmd;
}

%type <string> string
%token COMPONENT_SYSTEM
%token COMPONENT_CORE
%token CORE_REPLACE
%token COMPONENT_DEVICE
%token COMPONENT_EXTENSIONS
%token COMPONENT_FONT_RENDERERS
%token COMPONENT_AUTH
%token COMPONENT_TRANSPORTS
%token COMPONENT_INPUT
%token LIBRARY_PATH
%token <string> STRING
%token ARGS
%token END_SUBCONTEXT
%token START_SUBCONTEXT

%%

sections:	/* empty */
		| sections section
		;

section:	  system
		| core
		| device
		| extensions
		| font_renderers
		| auth_protocols
		| transports
		| input
		| library_path
		| args
		;

system:		COMPONENT_SYSTEM 		
		{ CurSection = C_SYSTEM;} 
		libraries
		;

core:		COMPONENT_CORE 		
		replacement
		{ CurSection = C_CORE;} 
		libraries
		;

replacement:	/* empty */
                  { ls_core_replace = False; }
		| CORE_REPLACE
		  { ls_core_replace = True; }
		;

device:		COMPONENT_DEVICE 		
		{ CurSection = C_DEVICE; } 
		libraries
		;

extensions:	COMPONENT_EXTENSIONS 	
		{ CurSection = C_EXTENSIONS; } 
		libraries
		;

font_renderers:	COMPONENT_FONT_RENDERERS 
		{ CurSection = C_FONT_RENDERERS; }
		libraries
		;

auth_protocols:	COMPONENT_AUTH 
		{ CurSection = C_AUTH; }
		libraries
		;

transports:	COMPONENT_TRANSPORTS 
		{ CurSection = C_TRANSPORTS; }
		libraries
		;

input:		COMPONENT_INPUT 
		{ CurSection = C_INPUT; }
		libraries
		;

libraries:	START_SUBCONTEXT 
		{ CurSectionIndex = LibListCounts[CurSection];}
		lib_list 
		END_SUBCONTEXT 
		{
		  LibListCounts[CurSection] = CurSectionIndex;
		}
		;

dep_list:	/* empty */
		| dep_list dep_entry
		;

dep_entry:	START_SUBCONTEXT dep_set
		{
		  LS_LibraryReq * ptr;
		  AddSubLibReq(Libs, CurSection, CurSectionIndex);
		  ptr = &(Libs[CurSection][CurSectionIndex].
		    SubLibs[Libs[CurSection][CurSectionIndex].NumSubLibs]);
		  FillLibReq(ptr);
		  Libs[CurSection][CurSectionIndex].NumSubLibs ++;
		}
		END_SUBCONTEXT
		;

dep_set:	STRING 
		| STRING STRING
		| STRING STRING STRING
		| STRING STRING STRING STRING
		;

lib_list:	/* empty */
		| lib_list lib_entry
		;

lib_entry:	START_SUBCONTEXT lib_set 
		{ 
		  LS_LibraryReq * ptr;
		  AddLibReq(Libs, CurSection, CurSectionIndex);
		  ptr = &(Libs[CurSection][CurSectionIndex]);
		  FillLibReq(ptr);
		  Libs[CurSection][CurSectionIndex].NumSubLibs = 0;
		}
		dep_list
		{
		  CurSectionIndex++;
		}
		END_SUBCONTEXT
		;

lib_set:	STRING 
		| STRING STRING
		| STRING STRING STRING 
		| STRING STRING STRING STRING
		;

library_path:	LIBRARY_PATH START_SUBCONTEXT string END_SUBCONTEXT 
		{
		    LdLibraryPath = (char *)malloc(strlen(ArgsScratch[0])+1);
		    strcpy(LdLibraryPath, ArgsScratch[0]);
		    ResetArgs();
		}
		;

args:		ARGS START_SUBCONTEXT 
		strings 
		{
		    register int i;
		    ConfigArgc = ArgsIndex;
		    ConfigArgv = (char **)malloc(sizeof(char *) * ConfigArgc);
		    for ( i = 0; i < ConfigArgc; i++ )
			CopyString(ConfigArgv[i], ArgsScratch[i]);
		    ResetArgs();
		}
		END_SUBCONTEXT 
		;

strings:	/* empty */
		| strings string
		;

string:		STRING 
		;

%% 
extern void AddArgs();
#include "lex.yy.c"
yyerror(s)
char * s;
{
fprintf(stderr,"%s at line %d\n", s, lines);
}

#define ARGS_MALLOC_SIZE 4096
#define ARGS_SIZE 100

static char * baseSpace = (char *)NULL, * nextSpace = (char *)NULL;
static int sizeSpace = 0;
static char ** baseArgs = (char **)NULL, ** nextArgs = (char **)NULL;
static int sizeArgs = 0;

void AddArgs(string)
char * string;
{
    char * currentSpace;

    if ( sizeSpace == 0 ) {
	baseSpace = (char *)malloc(sizeof(char) * ARGS_MALLOC_SIZE);
	sizeSpace = ARGS_MALLOC_SIZE;
	nextSpace = baseSpace;
    }
    if ( sizeSpace < (nextSpace-baseSpace + strlen(string) + 1) ) {
	int curSize = nextSpace - baseSpace;
	sizeSpace += ARGS_MALLOC_SIZE;
	baseSpace = (char *)realloc(baseSpace, sizeof(char) * sizeSpace);
	nextSpace = baseSpace + curSize;
    }
    currentSpace = nextSpace;
    if ( string == (char *)NULL || string[0] == '\0' ) {
	*nextSpace = '\0';
	nextSpace++;
    }
    else {
	strcpy(nextSpace, string);
	nextSpace += strlen(string) + 1;
    }
    if ( sizeArgs == 0 ) {
	baseArgs = (char **)malloc(sizeof(char *) * ARGS_SIZE);
	sizeArgs = ARGS_SIZE;
	nextArgs = baseArgs;
    	ArgsScratch = baseArgs;
    }
    if ( sizeArgs < (nextArgs-baseArgs) ) {
	int curSize = nextArgs - baseArgs;
	sizeArgs += ARGS_SIZE;
	baseArgs = (char **)realloc(baseArgs, sizeof(char *) * sizeArgs);
	nextArgs = baseArgs + curSize;
    	ArgsScratch = baseArgs;
    }
    *nextArgs = currentSpace;
    nextArgs++;
    ArgsIndex++;
}
void ResetArgs()
{
    ArgsIndex = 0;
    nextSpace = baseSpace;
    nextArgs = baseArgs;
    ArgsScratch = baseArgs;
}
void FreeArgs()
{
    if ( baseSpace != (char *)NULL )
	free(baseSpace);
    if ( baseArgs != (char **)NULL )
	free(baseArgs);
    ArgsIndex = 0;
    ArgsScratch = (char **)NULL;
    nextSpace = baseSpace = (char *)NULL;
    nextArgs = baseArgs = (char **)NULL;
    sizeSpace = sizeArgs = 0;
}

static int SectionAlloced[NUM_SECTIONS];

#define LIB_MALLOC_SIZE	10
void AddLibReq(Libs, CurSection, CurSectionIndex)
    LS_LibraryReq 	** Libs;
    int			CurSection, CurSectionIndex;
{
    static inited = 0;
    int i;

    if ( inited == 0 ) {
	inited = 1;
	for (i = 0; i < NUM_SECTIONS; i++)
	    SectionAlloced[i] = 0;
    }
    if ( SectionAlloced[CurSection] <= CurSectionIndex ) {
	SectionAlloced[CurSection] += LIB_MALLOC_SIZE;
	if ( Libs[CurSection] == (LS_LibraryReq *)NULL ) 
	    Libs[CurSection] = (LS_LibraryReq *)malloc(
		sizeof(LS_LibraryReq) * SectionAlloced[CurSection]);
	else
	    Libs[CurSection] = (LS_LibraryReq *)realloc(Libs[CurSection],
		sizeof(LS_LibraryReq) * SectionAlloced[CurSection]);
    }
}
	    

void AddSubLibReq(Libs, CurSection, CurSectionIndex)
    LS_LibraryReq 	** Libs;
    int			CurSection, CurSectionIndex;
{

    LS_LibraryReq	* ptr = &Libs[CurSection][CurSectionIndex];
    int			NumSubLibs = ptr->NumSubLibs;

    if ( NumSubLibs == 0 ) 
	ptr->SubLibs = (struct LS_LibraryReq *)
		malloc( sizeof(LS_LibraryReq) * LIB_MALLOC_SIZE);
    else 
        if ( (NumSubLibs % LIB_MALLOC_SIZE ) == 0) 
	    ptr->SubLibs = (struct LS_LibraryReq *)realloc( ptr->SubLibs,
		sizeof(LS_LibraryReq) * (NumSubLibs + LIB_MALLOC_SIZE));
}
	    

