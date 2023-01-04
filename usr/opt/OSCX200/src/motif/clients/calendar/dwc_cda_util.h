/* #module DWC_CDA_UTIL.H "V3-005"					    */
/* $Header$ */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; print routines
**
**  AUTHOR:
**
**	David Kubelka, February-1989
**
**  ABSTRACT:
**
**	This is the public header file for the DWC CDA utilitiesd
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**  V3-005  Paul Ferwerda					01-Aug-1990
**	    Replaced CDA$Y_*_handle with unsigned longs since mips stuff doesn't
**	    like void *s.		
**	V2-004  Denis G. Lacroix				17-Aug-1989
**		Two things: the code to print DDIF was stripping off the
**		icons twice (and thus DDIF output was essentially garbage),
**		and the code wasn't using the V2 CDA Style Guide
**		Definitions.
**	V2-003  Denis G. Lacroix				02-Aug-1989
**		Fix the error handling logic for the case when a file
**		cannot be opened.
**	V2-002	Per Hamnqvist					29-Feb-1989
**		Remove redef of void (since this is already done in compat.h)
**	V2-001	Per Hamnqvist					28-Feb-1989
**		Change void * for char * on Ultrix.
**	V2-000  David Kubelka					14-FEB-1989
**
**
**--
**/
 
#include    "dwc_compat.h"

typedef unsigned long CDA_Y_Aggregate_Type;
typedef unsigned long CDA_Y_Scope_Code;
typedef unsigned long CDA_Y_Aggregate_Index;
typedef unsigned long CDA_Y_Aggregate_Item;
typedef unsigned long CDA_Y_Add_Info;
typedef unsigned long CDA_Y_Text_File_Handle;

typedef struct _CDA_TextFileCtxRec
{   CDA_Y_Text_File_Handle  hndl;
    char		    *buffer;
    int			    length;
} DWCCDA_TextFileCtxRec, *DWCCDA_TextFileCtx;

typedef struct _CDA_ContextRec
{   unsigned long	    stream_handle;
    unsigned long	    file_handle;
    unsigned long	    root;
    unsigned long	    aggregate_handle[10];
    int			    depth;
} DWCCDA_ContextRec, *DWCCDA_Context;

int DWCCDA_Open_Text_File PROTOTYPE ((
	DWCCDA_TextFileCtx	ctx,
	char		*filespec,
	char		*default_filespec));

int DWCCDA_Read_Text_File PROTOTYPE ((
	DWCCDA_TextFileCtx	ctx));

int DWCCDA_Close_Text_File PROTOTYPE ((
	DWCCDA_TextFileCtx	ctx));

void DWCCDA_Store_Item_char PROTOTYPE ((
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	char			*value));

void DWCCDA_Store_Item_char_array PROTOTYPE ((
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	CDA_Y_Aggregate_Index	aggregate_index,
	char			*value));

void DWCCDA_Store_Item_int PROTOTYPE ((
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	int			value));

void DWCCDA_Delete_Aggregate PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Put_Aggregate PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Enter_Segment PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Enter_Scope PROTOTYPE ((
	DWCCDA_Context		ctx,
	CDA_Y_Scope_Code	scope_code));

void DWCCDA_Leave_Segment PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Leave_Scope PROTOTYPE ((
	DWCCDA_Context		ctx,
	CDA_Y_Scope_Code	scope_code));

void DWCCDA_Create_Aggregate PROTOTYPE ((
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Type	aggregate_type));

void DWCCDA_Close_File PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Close_Stream PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Finish PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Init PROTOTYPE ((
	DWCCDA_Context	ctx,
	char		*file_spec,
	char		*product_identifier,
	char		*product_name,
	int		style_guide));

void DWCCDA_Hrd_New_Line PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Hrd_New_Page PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Hrd_Tab PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Sft_New_Line PROTOTYPE ((
	DWCCDA_Context	ctx));

void DWCCDA_Text PROTOTYPE ((
	DWCCDA_Context	ctx,
	char		*text));

void DWCCDA_char PROTOTYPE ((
	DWCCDA_Context	ctx,
	char		*text));

void DWCCDA_chars PROTOTYPE ((
	DWCCDA_Context	ctx,
	int		length,
	char		*text));

void DWCCDA_Segment_Type PROTOTYPE ((
	DWCCDA_Context	ctx,
	char		*segment_type));

