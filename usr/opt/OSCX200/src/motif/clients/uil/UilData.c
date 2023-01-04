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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: UilData.c,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:33:03 $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      Global data definitions
**
**--
**/


/*
**
**  INCLUDE FILES
**
**/

#include <Mrm/MrmAppl.h>
#include <Mrm/Mrm.h>

#ifdef DEC_MOTIF_EXTENSION
#include <DXm/DXmHelpB.h>
#include <DXm/DXmColor.h>
#include <DXm/DXmPrint.h>
#include <DXm/DXmCSText.h>
#include <DXm/DXmSvn.h>
#endif

#include <setjmp.h>

#include "UilDefI.h"
#include "UilSymGen.h"

/*
**
**  DEFINE and MACRO DEFINITIONS
**
**/



/*
**
**  EXTERNAL VARIABLE DECLARATIONS
**
**/



/*
**
**  GLOBAL VARIABLE DECLARATIONS
**
**/

/*    Case sensitivity switch; TRUE if case sensitive.    */



externaldef(uil_comp_glbl) boolean	uil_v_case_sensitive = TRUE;

/*    Location to store error type.    */

externaldef(uil_comp_glbl) jmp_buf	uil_az_error_env_block;
externaldef(uil_comp_glbl) boolean	uil_az_error_env_valid = FALSE;

/*    Debugging switches.    */

#if debug_version
externaldef(uil_comp_glbl) boolean	uil_v_dump_tokens = FALSE;
externaldef(uil_comp_glbl) boolean	uil_v_dump_symbols = FALSE;
#endif

externaldef(uil_comp_glbl) status	uil_l_compile_status = uil_k_success_status;
externaldef(uil_comp_glbl) Uil_compile_desc_type   *Uil_compile_desc_ptr = NULL;
externaldef(uil_comp_glbl) int		Uil_percent_complete = 0;
externaldef(uil_comp_glbl) int		Uil_lines_processed = 0;
externaldef(uil_comp_glbl) char	*Uil_current_file = "";


/*	Define the user-defined object and default character set */
externaldef(uil_sym_glbl) unsigned short int	uil_sym_user_defined_object =
		sym_k_user_defined_object;
externaldef(uil_sym_glbl) unsigned short int	uil_sym_default_charset =
		sym_k_iso_latin1_charset;
externaldef(uil_sym_glbl) unsigned short int	uil_sym_isolatin1_charset =
		sym_k_iso_latin1_charset;

/*	argument types, allowed tables, constraints, related arguments */
#include "UilSymArTy.h"
#include "UilSymRArg.h"
#include "UilSymArTa.h"
#include "UilSymReas.h"
#include "UilSymCtl.h"
#include "UilConst.h"

/* Allowed children and child classes */
#include "UilSymChCl.h"
#include "UilSymChTa.h"

/*	sym_k... to name vectors. */
#include "UilSymNam.h"

/*	Define mapping of sym_k_..._value to data type names. */
externaldef(uil_comp_glbl) int uil_max_value = sym_k_max_value;
externaldef(uil_comp_glbl) char *uil_datatype_names [] = {
    "",	/* NOT USED */
    "any",
    "argument",
    "asciz_table",
    "boolean",
    "string",
    "class_rec_name",
    "color",
    "color_table",
    "compound_string",
    "float",
    "font",
    "font_table",
    "icon",
    "identifier",
    "integer",
    "integer_table",
    "keysym",
    "pixmap",
    "reason",
    "rgb",
    "single_float",
    "string_table",
    "translation_table",
    "widget_ref",
    "xbitmapfile",
    "localized_string",
    "wchar_string",
    "fontset",
    "child",
};

/*	Enumeration Set/Value tables */
#include "UilSymEnum.h"

/*	Character set attribute tables */
#include "UilSymCSet.h"

/*	Mrm encoding information */
#include "UilUrmClas.h"

/*	Names of tokens in grammar */
#include "UilTokName.h"

/*	The keyword tables */
#include "UilKeyTab.h"


/*
**
**  OWN VARIABLE DECLARATIONS
**
**/

