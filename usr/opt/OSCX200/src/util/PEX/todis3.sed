# 
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
# HISTORY
# 
# $XConsortium: todis3.sed,v 1.1 91/02/16 09:51:46 rws Exp $
#  Copyright (c) 1990 by Sun Microsystems, Inc.
#  A Sed script for converting to the PHIGS DIS C binding 
#
s/size_raster\.x_val/size_raster\.size_x/g
s/size_raster\.y_val/size_raster\.size_y/g
s/size_raster\.z_val/size_raster\.size_z/g
s/size_dc\.x_val/size_dc\.size_x/g
s/size_dc\.y_val/size_dc\.size_y/g
s/size_dc\.z_val/size_dc\.size_z/g
s/\.z_val/\.delta_z/g
s/\.y_val/\.delta_y/g
s/\.x_val/\.delta_x/g
s/\.vertexdata/\.vertex_data/g
s/\.unsupported/\.unsupp/g
s/\.style_asf/\.style_ind_asf/g
s/\.space/\.char_space/g
s/\.shading/\.shad_meth/g
s/\.shades/\.shads/g
s/\.shad_meths/\.shad_meths/g
s/\.refplanes/\.ref_planes/g
s/\.reference/\.ref/g
s/\.properties/\.props/g
s/\.area_props/\.refl_props/g
s/\.priority/\.disp_pri/g
s/\.pfcf/\.line_fill_ctrl_flag/g
s/\.path/\.path_list/g
s/\.pat_ref_point_vec/\.pat_ref_point_vecs/g
s/\.pat_array/\.colr_array/g
s/\.nurb_surface/\.nurb_surf/g
s/\.num_pred_ind/\.num_pred_inds/g
s/\.num_facilities/\.num_id_facs/g
s/\.num_expans/\.num_char_expans/g
s/\.num_ex_refs/\.num_elem_refs/g
s/\.num_ex_ref_lists/\.num_elem_ref_lists/g
s/\.num_elems/\.num_elem_types/g
s/\.num_choices/\.num_pick_ids/g
s/\.num_alt/\.num_prompts/g
s/\.min_expan/\.min_char_expan/g
s/\.method/\.meth/g
s/\.max_view/\.view_reps/g
s/\.max_text/\.text_bundles/g
s/\.max_pat/\.pat_reps/g
s/\.max_mark/\.mark_bundles/g
s/\.max_line/\.line_bundles/g
s/\.max_light_src/\.light_src_rep/g
s/\.max_int/\.int_bundles/g
s/\.max_expan/\.max_char_expan/g
s/\.max_edge/\.edge_bundles/g
s/\.max_dcue/\.dcue_rep/g
s/\.max_colr_map/\.colr_map_rep/g
s/\.max_colr/\.colr_reps/g
s/\.int_asf/\.style_asf/g
s/\.initial_position/\.init_pos/g
s/\.initial_pos/\.init_pos/g
s/\.indicator/\.ind/g
s/\.id_facilities/\.id_facs/g
s/\.hatche_styles/\.hatch_styles/g
s/\.fas/\.fill_set/g
s/\.expan/\.char_expan/g
s/\.ex_refs/\.elem_refs/g
s/\.ex_ref_lists/\.elem_ref_lists/g
s/\.esc_out_/\.escape_out_/g
s/\.esc_in_/\.escape_in_/g
s/\.elems/\.elem_types/g
s/\.elem_num/\.elem_pos/g
s/\.distgmode/\.disting_mode/g
s/\.depth_cue/\.dcue/g
s/\.delete/\.del/g
s/\.cullmode/\.cull_mode/g
s/\.colr_bundle/\.colr_rep/g
s/\.colr_asf/\.colr_ind_asf/g
s/\.cieluv_z/\.cieluv_y_lum/g
s/\.atts/\.attrs/g
s/\.num_atts/\.num_attrs/g
s/\.approx_value/\.approx_val/g
s/\.anno_text/\.anno_text_rel/g
s/\.alt_prompts/\.prompts/g
s/\.shadmeths/\.shad_meths/g
s/\.refeqs/\.refl_eqns/g
s/\.pat_bundle/\.pat_rep/g
s/\.uvalue/\.u_val/g
s/\.vvalue/\.v_val/g
s/\.ratsel/\.rationality/g
s/\.ext_cell_array/\.cell_array_plus/g
s/\.normal/\.norm/g
s/\.x_dim/\.size_x/g
s/\.y_dim/\.size_y/g
s/\.z_dim/\.size_z/g
s/pet_r3\.mk/pet_r3\.marker_attrs/g
s/pet_r3\.ln/pet_r3\.line_attrs/g
s/pet_r4\.ln/pet_r4\.line_attrs/g
s/attr\.ln/attr\.line_attrs/g
s/attr\.in/attr\.int_attrs/g
s/attr\.fill_set\.edge/attr\.fill_set\.edge_attrs/g
s/attr\.fill_set\.in/attr\.fill_set\.int_attrs/g
s/size_raster->x_val/size_raster->size_x/g
s/size_raster->y_val/size_raster->size_y/g
s/size_raster->z_val/size_raster->size_z/g
s/size_dc->x_val/size_dc->size_x/g
s/size_dc->y_val/size_dc->size_y/g
s/size_dc->z_val/size_dc->size_z/g
s/\->z_val/->delta_z/g
s/\->y_val/->delta_y/g
s/\->x_val/->delta_x/g
s/\->vertexdata/->vertex_data/g
s/\->unsupported/->unsupp/g
s/\->style_asf/->style_ind_asf/g
s/\->space/->char_space/g
s/\->shading/->shad_meth/g
s/\->shades/->shads/g
s/\->shad_meths/->shad_meths/g
s/\->refplanes/->ref_planes/g
s/\->reference/->ref/g
s/\->properties/->props/g
s/\->area_props/->refl_props/g
s/\->priority/->disp_pri/g
s/\->pfcf/->line_fill_ctrl_flag/g
s/\->path/->path_list/g
s/\->pat_ref_point_vec/->pat_ref_point_vecs/g
s/\->pat_array/->colr_array/g
s/\->nurb_surface/->nurb_surf/g
s/\->num_pred_ind/->num_pred_inds/g
s/\->num_facilities/->num_id_facs/g
s/\->num_expans/->num_char_expans/g
s/\->num_ex_refs/->num_elem_refs/g
s/\->num_ex_ref_lists/->num_elem_ref_lists/g
s/\->num_elems/->num_elem_types/g
s/\->num_choices/->num_pick_ids/g
s/\->num_alt/->num_prompts/g
s/\->min_expan/->min_char_expan/g
s/\->method/->meth/g
s/\->max_view/->view_reps/g
s/\->max_text/->text_bundles/g
s/\->max_pat/->pat_reps/g
s/\->max_mark/->mark_bundles/g
s/\->max_line/->line_bundles/g
s/\->max_light_src/->light_src_rep/g
s/\->max_int/->int_bundles/g
s/\->max_expan/->max_char_expan/g
s/\->max_edge/->edge_bundles/g
s/\->max_dcue/->dcue_rep/g
s/\->max_colr_map/->colr_map_rep/g
s/\->max_colr/->colr_reps/g
s/\->int_asf/->style_asf/g
s/\->initial_position/->init_pos/g
s/\->initial_pos/->init_pos/g
s/\->indicator/->ind/g
s/\->id_facilities/->id_facs/g
s/\->hatche_styles/->hatch_styles/g
s/\->fas/->fill_set/g
s/\->expan/->char_expan/g
s/\->ex_refs/->elem_refs/g
s/\->ex_ref_lists/->elem_ref_lists/g
s/\->esc_out_/->escape_out_/g
s/\->esc_in_/->escape_in_/g
s/\->elems/->elem_types/g
s/\->elem_num/->elem_pos/g
s/\->distgmode/->disting_mode/g
s/\->depth_cue/->dcue/g
s/\->delete/->del/g
s/\->cullmode/->cull_mode/g
s/\->colr_bundle/->colr_rep/g
s/\->colr_asf/->colr_ind_asf/g
s/\->cieluv_z/->cieluv_y_lum/g
s/\->atts/->attrs/g
s/\->num_atts/->num_attrs/g
s/\->approx_value/->approx_val/g
s/\->anno_text/->anno_text_rel/g
s/\->alt_prompts/->prompts/g
s/\->shadmeths/->shad_meths/g
s/\->refeqs/->refl_eqns/g
s/\->pat_bundle/->pat_rep/g
s/\->uvalue/->u_val/g
s/\->vvalue/->v_val/g
s/\->ratsel/->rationality/g
s/\->ext_cell_array/->cell_array_plus/g
s/\->normal/->norm/g
s/->x_dim/->size_x/g
s/->y_dim/->size_y/g
s/\.z_dim/->size_z/g
s/pet_r3->mk/pet_r3->marker_attrs/g
s/pet_r3->ln/pet_r3->line_attrs/g
s/pet_r4->ln/pet_r4->line_attrs/g
s/pet_r5->ln/pet_r5->line_attrs/g
s/pet_r5->in/pet_r5->int_attrs/g
s/pet_r5->edge/pet_r5\.edge_attrs/g
s/attr->ln/attr->line_attrs/g
s/attr->in/attr->int_attrs/g
