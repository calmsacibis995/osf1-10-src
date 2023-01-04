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
/* $XConsortium: ph_map.h,v 5.2 91/02/16 10:07:49 rws Exp $ */

/*
 */
/***********************************************************
Copyright 1989, 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

						All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 * Copyright (c) 1989, 1990, 1991 by M.I.T. and Sun Microsystems, Inc.
 */

/*--------------------------------------------------------------------*\
|
|  Copyright (C) 1989, 1990, 1991, National Computer Graphics Association
|
|  Permission is granted to any individual or institution to use, copy, or
|  redistribute this software so long as it is not sold for profit, provided
|  this copyright notice is retained.
|
|                         Developed for the
|                National Computer Graphics Association
|                         2722 Merrilee Drive
|                         Fairfax, VA  22031
|                           (703) 698-9600
|
|                                by
|                 SimGraphics Engineering Corporation
|                    1137 Huntington Drive  Unit A
|                      South Pasadena, CA  91030
|                           (213) 255-0900
|---------------------------------------------------------------------
|
| Author        :	SimGraphics Engineering Corportation
|
| File          :	ph_map.h
| Date          :	Fri Feb  9 10:46:55 PST 1990
| Project       :	PLB
| Description   :	'C' callable PHIGS+ names defined/extern'd
| Status        :	Version 1.0
|
| Revisions     :
|
|       2/90            MFC Tektronix, Inc.: PEX-SI API implementation.
|
|      12/90            MFC Tektronix, Inc.: PEX-SI PEX5R1 Release.
|
\*--------------------------------------------------------------------*/

#ifdef USING_PHIGS
extern void  panno_text_rel3();
extern void  pcopy_all_elems_struct();
extern void  pclose_phigs();
extern void  pclose_struct();
extern void  pclose_ws();
extern void  pdel_elem();
extern void  pdel_elemrange();
extern void  pempty_struct();
extern void  peval_view_map_matrix3();
extern void  peval_view_ori_matrix3();
extern void  pexec_struct();
extern void  pfill_area();
extern void  pfill_area3_data();
extern void  pfill_area_set();
extern void  pfill_area_set3_data();
extern void  pgdp3();
extern void  plabel();
extern void  popen_phigs();
extern void  popen_struct();
extern void  popen_ws();
extern void  poffset_elem_ptr();
extern void  ppolyline();
extern void  ppolyline3();
extern void  ppolymarker();
extern void  ppolymarker3();
extern void  ppost_struct();
extern void  pinq_elem_ptr();
extern void  pset_anno_style();
extern void  pset_anno_align();
extern void  pset_anno_char_ht();
extern void  pset_anno_char_up_vec();
extern void  pset_anno_path();
extern void  pset_back_int_colr();
extern void  pset_back_refl_eqn();
extern void  pset_back_int_shad_meth();
extern void  pset_back_int_style();
extern void  pset_back_int_style_ind();
extern void  pset_char_ht();
extern void  pset_char_space();
extern void  pset_char_up_vec();
extern void  pset_char_expan();
extern void  pset_colr_model();
extern void  pset_colr_rep();
extern void  pset_dcue_ind();
extern void  pset_dcue_rep();
extern void  pset_disp_upd_st();
extern void  pset_edge_colr();
extern void  pset_edge_flag();
extern void  pset_edgetype();
extern void  pset_elem_ptr();
extern void  pset_elem_ptr_label();
extern void  pset_edgewidth();
extern void  pset_global_tran3();
extern void  pset_hlhsr_id();
extern void  pset_hlhsr_mode();
extern void  pset_int_colr();
extern void  pset_refl_eqn();
extern void  pset_int_style();
extern void  pset_int_style_ind();
extern void  pset_int_shad_meth();
extern void  pset_linetype();
extern void  pset_local_tran3();
extern void  pset_light_src_rep();
extern void  pset_light_src_state();
extern void  pset_linewidth();
extern void  pset_marker_type();
extern void  pset_marker_size();
extern void  pset_line_colr();
extern void  pset_line_shad_meth();
extern void  pset_marker_colr();
extern void  pset_of_fill_area_set3_data();
extern void  pset_text_align();
extern void  pset_text_colr();
extern void  pset_text_font();
extern void  pset_text_path();
extern void  pset_text_prec();
extern void  pset_view_rep3();
extern void  pset_view_ind();
extern void  ptext();
extern void  ptext3();
extern void  punpost_struct();
extern void  pupd_ws();
#else /* USING_PHIGS */
#define  panno_text_rel3 noop_function
#define  pcopy_all_elems_struct noop_function
#define  pclose_phigs noop_function
#define  pclose_struct noop_function
#define  pclose_ws noop_function
#define  pdel_elem noop_function
#define  pdel_elemrange noop_function
#define  pempty_struct noop_function
#define  peval_view_map_matrix3 noop_function
#define  peval_view_ori_matrix3 noop_function
#define  pexec_struct noop_function
#define  pfill_area noop_function
#define  pfill_area3_data noop_function
#define  pfill_area_set noop_function
#define  pfill_area_set3_data noop_function
#define  pgdp3 noop_function
#define  plabel noop_function
#define  popen_phigs noop_function
#define  popen_struct noop_function
#define  popen_ws noop_function
#define  poffset_elem_ptr noop_function
#define  ppolyline noop_function
#define  ppolymarker noop_function
#define  ppolymarker3 noop_function
#define  ppost_struct noop_function
#define  pinq_elem_ptr noop_function
#define  pset_anno_style noop_function
#define  pset_anno_align noop_function
#define  pset_anno_char_ht noop_function
#define  pset_anno_char_up_vec noop_function
#define  pset_anno_path noop_function
#define  pset_back_int_colr noop_function
#define  pset_back_refl_eqn noop_function
#define  pset_back_int_shad_meth noop_function
#define  pset_back_int_style noop_function
#define  pset_back_int_style_ind noop_function
#define  pset_char_ht noop_function
#define  pset_char_space noop_function
#define  pset_char_up_vec noop_function
#define  pset_char_expan noop_function
#define  pset_colr_model noop_function
#define  pset_colr_rep noop_function
#define  pset_dcue_ind noop_function
#define  pset_dcue_rep noop_function
#define  pset_disp_upd_st noop_function
#define  pset_edge_colr noop_function
#define  pset_edge_flag noop_function
#define  pset_edgetype noop_function
#define  pset_elem_ptr noop_function
#define  pset_elem_ptr_label noop_function
#define  pset_edgewidth noop_function
#define  pset_global_tran3 noop_function
#define  pset_hlhsr_id noop_function
#define  pset_hlhsr_mode noop_function
#define  pset_int_colr noop_function
#define  pset_refl_eqn noop_function
#define  pset_int_style noop_function
#define  pset_int_style_ind noop_function
#define  pset_int_shad_meth noop_function
#define  pset_linetype noop_function
#define  pset_local_tran3 noop_function
#define  pset_light_src_rep noop_function
#define  pset_light_src_state noop_function
#define  pset_linewidth noop_function
#define  pset_marker_type noop_function
#define  pset_marker_size noop_function
#define  pset_line_colr noop_function
#define  pset_line_shad_meth noop_function
#define  pset_marker_colr noop_function
#define  pset_of_fill_area_set3_data noop_function
#define  pset_text_align noop_function
#define  pset_text_colr noop_function
#define  pset_text_font noop_function
#define  pset_text_path noop_function
#define  pset_text_prec noop_function
#define  pset_view_rep3 noop_function
#define  pset_view_ind noop_function
#define  ptext noop_function
#define  ptext3 noop_function
#define  punpost_struct noop_function
#define  pupd_ws noop_function
#endif /* USING_PHIGS */
