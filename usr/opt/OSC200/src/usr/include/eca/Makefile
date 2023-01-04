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
#
# HISTORY
#
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 23:20:10 $
#
# /usr/include/eca/Makefile
# Common Agent:  export common header and MOMGen template files

HEADER_FILES = \
	agent.h	agent_access_config.h ca_config.h dce_error.h evd_defs.h \
	extern_nil.h iso_defs.h man.h man_data.h mcc_interface_def.h \
	mir.h mo.h mold.h mold_uuid.h moss.h moss_asn1.h moss_cmp.h\
	moss_dna.h moss_i_table.h moss_inet.h moss_private.h nbase.h ncl_interface_def.h \
	pe.h rpc.h rpcexc.h snmp_mib.h snmppe_pei_event.h socket.h 

MOMGEN_TEMPLATES = eca_snmp.ms \
	build_access.c build_action.c build_action_action.c build_action_create.c \
	build_action_delete.c build_action_null.c build_cancel_null.c build_common.h \
	build_create.c build_create_null.c build_delete.c build_delete_null.c \
	build_directive.c build_export_oids.h build_extern_common.h build_find_instance.c \
	build_get.c build_get_candidate_instance.c build_get_null.c build_getnext.c \
	build_import_oids.h build_init.c build_init_perform_instance.c build_instance_avl.c \
	build_instance_avl_null.c build_mom.make build_perform_action.c build_perform_create.c \
	build_perform_create_instance.c build_perform_delete.c build_perform_get.c build_perform_get_instance.c \
	build_perform_init.c build_perform_nontable_getnext.c build_perform_set.c build_perform_snmp_get.c \
	build_perform_table_getnext.c build_prototypes.h build_set.c build_set_null.c \
	build_switch_get_candidate.c build_trap.c build_utility_routines.c 

DATAFILES 	= ${HEADER_FILES} ${MOMGEN_TEMPLATES}

INCLUDES	= ${DATAFILES}
EXPDIR		= /usr/include/eca/
IMODE		= 644

include ${MAKEFILEPATH}/rules.mk

