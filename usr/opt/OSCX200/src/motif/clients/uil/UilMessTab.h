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
 * Motif Release 1.2
*/ 
/*   $RCSfile: UilMessTab.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:36:07 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      DECwindows Toolkit User Interface Language Compiler (UIL)
**--
**/

#ifdef VMS
globalvalue uil$_add_source;
globalvalue uil$_arg_count;
globalvalue uil$_arg_type;
globalvalue uil$_backslash_ignored;
globalvalue uil$_bad_argument;
globalvalue uil$_bad_database;
globalvalue uil$_bad_lang_value;
globalvalue uil$_bug_check;
globalvalue uil$_cannot_convert;
globalvalue uil$_circular_def;
globalvalue uil$_circular_ref;
globalvalue uil$_control_char;
globalvalue uil$_create_proc;
globalvalue uil$_create_proc_inv;
globalvalue uil$_create_proc_req;
globalvalue uil$_ctx_req;
globalvalue uil$_dup_letter;
globalvalue uil$_dup_list;
globalvalue uil$_dupl_opt;
globalvalue uil$_future_version;
globalvalue uil$_gadget_not_sup;
globalvalue uil$_icon_letter;
globalvalue uil$_icon_width;
globalvalue uil$_illegal_forward_ref;
globalvalue uil$_info;
globalvalue uil$_inv_module;
globalvalue uil$_invalid_enumval;
globalvalue uil$_list_item;
globalvalue uil$_listing_open;
globalvalue uil$_listing_write;
globalvalue uil$_miss_opt_arg;
globalvalue uil$_name_too_long;
globalvalue uil$_names;
globalvalue uil$_never_def;
globalvalue uil$_no_enumset;
globalvalue uil$_no_source;
globalvalue uil$_no_uid;
globalvalue uil$_nonpvt;
globalvalue uil$_not_impl;
globalvalue uil$_null;
globalvalue uil$_obj_type;
globalvalue uil$_operand_type;
globalvalue uil$_out_of_memory;
globalvalue uil$_out_range;
globalvalue uil$_override_builtin;
globalvalue uil$_prev_error;
globalvalue uil$_previous_def;
globalvalue uil$_single_control;
globalvalue uil$_single_letter;
globalvalue uil$_single_occur;
globalvalue uil$_src_close;
globalvalue uil$_src_limit;
globalvalue uil$_src_null_char;
globalvalue uil$_src_open;
globalvalue uil$_src_read;
globalvalue uil$_src_truncate;
globalvalue uil$_submit_spr;
globalvalue uil$_subnotall;
globalvalue uil$_summary;
globalvalue uil$_supersede;
globalvalue uil$_syntax;
globalvalue uil$_too_many;
globalvalue uil$_too_many_dirs;
globalvalue uil$_uid_open;
globalvalue uil$_uid_write;
globalvalue uil$_undefined;
globalvalue uil$_unknown_charset;
globalvalue uil$_unknown_classrecnam;
globalvalue uil$_unknown_opt;
globalvalue uil$_unknown_seq;
globalvalue uil$_unsupp_charset;
globalvalue uil$_unsupp_const;
globalvalue uil$_unsupported;
globalvalue uil$_unterm_seq;
globalvalue uil$_value_too_large;
globalvalue uil$_widget_cycle;
globalvalue uil$_wmd_open;
globalvalue uil$_wrong_type;

int  const diag_rl_external_code[78] =
    {
        uil$_dupl_opt,
        uil$_unknown_opt,
        uil$_add_source,
        uil$_src_open,
        uil$_src_read,
        uil$_bug_check,
        uil$_src_truncate,
        uil$_out_range,
        uil$_unterm_seq,
        uil$_control_char,
        uil$_unknown_seq,
        uil$_backslash_ignored,
        uil$_name_too_long,
        uil$_out_of_memory,
        uil$_syntax,
        uil$_undefined,
        uil$_ctx_req,
        uil$_not_impl,
        uil$_wrong_type,
        uil$_unsupported,
        uil$_supersede,
        uil$_previous_def,
        uil$_nonpvt,
        uil$_arg_count,
        uil$_arg_type,
        uil$_obj_type,
        uil$_never_def,
        uil$_dup_list,
        uil$_list_item,
        uil$_prev_error,
        uil$_submit_spr,
        uil$_info,
        uil$_miss_opt_arg,
        uil$_listing_open,
        uil$_listing_write,
        uil$_inv_module,
        uil$_src_limit,
        uil$_src_null_char,
        uil$_summary,
        uil$_uid_open,
        uil$_no_uid,
        uil$_create_proc,
        uil$_create_proc_inv,
        uil$_create_proc_req,
        uil$_null,
        uil$_circular_def,
        uil$_no_source,
        uil$_single_occur,
        uil$_single_control,
        uil$_unknown_charset,
        uil$_names,
        uil$_single_letter,
        uil$_dup_letter,
        uil$_icon_width,
        uil$_icon_letter,
        uil$_too_many,
        uil$_subnotall,
        uil$_gadget_not_sup,
        uil$_operand_type,
        uil$_unsupp_charset,
        uil$_unsupp_const,
        uil$_too_many_dirs,
        uil$_src_close,
        uil$_circular_ref,
        uil$_override_builtin,
        uil$_no_enumset,
        uil$_invalid_enumval,
        uil$_bad_lang_value,
        uil$_widget_cycle,
        uil$_value_too_large,
        uil$_illegal_forward_ref,
        uil$_cannot_convert,
        uil$_bad_argument,
        uil$_bad_database,
        uil$_future_version,
        uil$_wmd_open,
        uil$_uid_write,
        uil$_unknown_classrecnam,
    };

#endif



char XmConst msg0[36] = "duplicate option \"%s\" was ignored";
char XmConst msg1[34] = "unknown option \"%s\" was ignored";
char XmConst msg2[43] = "additional UIL source file: %s was ignored";
char XmConst msg3[30] = "error opening source file: %s";
char XmConst msg4[43] = "error reading next line of source file: %s";
char XmConst msg5[19] = "Internal error: %s";
char XmConst msg6[32] = "line truncated at %d characters";
char XmConst msg7[31] = "value of %s is out of range %s";
char XmConst msg8[21] = "%s not terminated %s";
char XmConst msg9[37] = "unprintable character \\%d\\ ignored";
char XmConst msg10[32] = "unknown sequence \"%s\" ignored";
char XmConst msg11[46] = "unknown escape sequence \"\\%c\" - \\ ignored";
char XmConst msg12[46] = "name exceeds 31 characters - truncated to: %s";
char XmConst msg13[35] = "compiler ran out of virtual memory";
char XmConst msg14[56] = "unexpected %s token seen - parsing resumes after \"%c\"";
char XmConst msg15[44] = "%s %s must be defined before this reference";
char XmConst msg16[41] = "context requires a %s - %s was specified";
char XmConst msg17[26] = "%s is not implemented yet";
char XmConst msg18[39] = "found %s value when expecting %s value";
char XmConst msg19[45] = "the %s %s is not supported for the %s object";
char XmConst msg20[58] = "this %s %s supersedes a previous definition in this %s %s";
char XmConst msg21[33] = "name %s previously defined as %s";
char XmConst msg22[43] = "value used in this context must be private";
char XmConst msg23[55] = "procedure %s was previously declared with %d arguments";
char XmConst msg24[56] = "found %s value - procedure %s argument must be %s value";
char XmConst msg25[33] = "found %s %s when expecting %s %s";
char XmConst msg26[24] = "%s %s was never defined";
char XmConst msg27[39] = "%s %s already specified for this %s %s";
char XmConst msg28[29] = "%s item not allowed in %s %s";
char XmConst msg29[45] = "compilation terminated - fix previous errors";
char XmConst msg30[38] = "internal error - submit defect report";
char XmConst msg31[2] = " ";
char XmConst msg32[35] = "%s missing following \"%s\" option";
char XmConst msg33[31] = "error opening listing file: %s";
char XmConst msg34[34] = "error writing to listing file: %s";
char XmConst msg35[51] = "invalid module structure - check UIL module syntax";
char XmConst msg36[31] = "too many source files open: %s";
char XmConst msg37[38] = "source line contains a null character";
char XmConst msg38[45] = "errors: %d  warnings: %d  informationals: %d";
char XmConst msg39[27] = "error opening UID file: %s";
char XmConst msg40[25] = "no UID file was produced";
char XmConst msg41[53] = "creation procedure is not supported by the %s widget";
char XmConst msg42[59] = "creation procedure is not allowed in a %s widget reference";
char XmConst msg43[58] = "creation procedure is required in a %s widget declaration";
char XmConst msg44[46] = "a NULL character in a string is not supported";
char XmConst msg45[43] = "widget %s is part of a circular definition";
char XmConst msg46[25] = "no source file specified";
char XmConst msg47[35] = "%s %s supports only a single %s %s";
char XmConst msg48[41] = "%s widget supports only a single control";
char XmConst msg49[22] = "unknown character set";
char XmConst msg50[47] = "place names clause before other module clauses";
char XmConst msg51[47] = "color letter string must be a single character";
char XmConst msg52[48] = "color letter used for prior color in this table";
char XmConst msg53[37] = "row %d must have same width as row 1";
char XmConst msg54[52] = "row %d, column %d: letter \"%c\" not in color table";
char XmConst msg55[32] = "too many %ss in %s, limit is %d";
char XmConst msg56[48] = "Subqualifier not allowed with negated qualifier";
char XmConst msg57[60] = "%s gadget is not supported - %s widget will be used instead";
char XmConst msg58[28] = "%s type is not valid for %s";
char XmConst msg59[66] = "support for this character set may be removed in a future release";
char XmConst msg60[49] = "the %s constraint is not supported for the %s %s";
char XmConst msg61[37] = "too many \"%s\" options, limit is %d";
char XmConst msg62[30] = "error closing source file: %s";
char XmConst msg63[35] = "the %s value is circularly defined";
char XmConst msg64[28] = "overriding built-in name %s";
char XmConst msg65[51] = "the %s argument does not support enumerated values";
char XmConst msg66[57] = "the %s argument does not support the %s enumerated value";
char XmConst msg67[40] = "$LANG contains an unknown character set";
char XmConst msg68[66] = "the %s object's controls hierarchy contains a reference to itself";
char XmConst msg69[41] = "value %s is too large for context buffer";
char XmConst msg70[42] = "forward referencing is not allowed for %s";
char XmConst msg71[34] = "cannot convert %s type to %s type";
char XmConst msg72[14] = "%s is invalid";
char XmConst msg73[30] = "error reading binary database";
char XmConst msg74[47] = "binary database compiled with a future version";
char XmConst msg75[32] = "error opening database file: %s";
char XmConst msg76[27] = "error writing UID file: %s";
char XmConst msg77[45] = "'%s' is an unknown Toolkit class record name";

typedef struct
{
  XmConst int  l_severity;
  char XmConst *ac_text;
} diag_rz_msg_entry;

XmConst diag_rz_msg_entry diag_rz_msg_table[78] =
        {
	   { 2, msg0 },
	   { 2, msg1 },
	   { 3, msg2 },
	   { 4, msg3 },
	   { 4, msg4 },
	   { 4, msg5 },
	   { 3, msg6 },
	   { 3, msg7 },
	   { 3, msg8 },
	   { 3, msg9 },
	   { 3, msg10 },
	   { 3, msg11 },
	   { 3, msg12 },
	   { 4, msg13 },
	   { 3, msg14 },
	   { 3, msg15 },
	   { 3, msg16 },
	   { 3, msg17 },
	   { 3, msg18 },
	   { 2, msg19 },
	   { 1, msg20 },
	   { 3, msg21 },
	   { 3, msg22 },
	   { 3, msg23 },
	   { 3, msg24 },
	   { 3, msg25 },
	   { 3, msg26 },
	   { 3, msg27 },
	   { 3, msg28 },
	   { 4, msg29 },
	   { 4, msg30 },
	   { 1, msg31 },
	   { 3, msg32 },
	   { 4, msg33 },
	   { 4, msg34 },
	   { 3, msg35 },
	   { 4, msg36 },
	   { 3, msg37 },
	   { 1, msg38 },
	   { 4, msg39 },
	   { 1, msg40 },
	   { 3, msg41 },
	   { 3, msg42 },
	   { 3, msg43 },
	   { 2, msg44 },
	   { 3, msg45 },
	   { 4, msg46 },
	   { 2, msg47 },
	   { 2, msg48 },
	   { 3, msg49 },
	   { 3, msg50 },
	   { 3, msg51 },
	   { 3, msg52 },
	   { 3, msg53 },
	   { 3, msg54 },
	   { 3, msg55 },
	   { 4, msg56 },
	   { 2, msg57 },
	   { 3, msg58 },
	   { 2, msg59 },
	   { 2, msg60 },
	   { 2, msg61 },
	   { 2, msg62 },
	   { 3, msg63 },
	   { 2, msg64 },
	   { 2, msg65 },
	   { 2, msg66 },
	   { 3, msg67 },
	   { 3, msg68 },
	   { 4, msg69 },
	   { 4, msg70 },
	   { 3, msg71 },
	   { 3, msg72 },
	   { 4, msg73 },
	   { 4, msg74 },
	   { 4, msg75 },
	   { 4, msg76 },
	   { 2, msg77 },
        };
