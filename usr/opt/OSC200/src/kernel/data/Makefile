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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.2.22.12 $ (DEC) $Date: 1993/11/03 22:25:19 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#


DATAFILES		= adu_ln_data.c audit_data.c \
			  autoconf_data.c cam_data.c cam_special_data.c \
			  ci_data.c cippd_data.c cons_sw_data.c dc_data.c \
			  fb_data.c ga_data.c gw_screen_data.c gx_data.c \
			  gvp_data.c if_fza_data.c if_fta_data.c if_ln_data.c \
			  kn230_option_data.c lk201_data.c mscp_data.c \
			  msi_data.c np_data.c pmap_data.c presto_data.c \
			  scc_data.c scs_data.c scsi_data.c sec_data.c \
			  sysap_data.c tc_option_data.c tty_pty_data.c \
			  uq_data.c  if_ne_data.c dhu_data.c \
			  dcedfs_data.c ws_data.c binlog_data.c \
			  cm_data.c if_qe_data.c \
			  xmi_data.c if_xna_data.c if_te_data.c cbus_data.c \
			  if_mfa_data.c pa_data.c pq_data.c px_data.c \
			  pv_data.c lsb_data.c nvtc_data.c steal_mem.c \
			  if_faa_data.c if_fxx_data.c \
			  fbus_option_data.c xti_data.c if_tra_data.c pty_data.c \
			  ace_data.c isa_data.c eisa_option_data.c \
			  vga_data.c pcxal_data.c pcxas_data.c \
			  bt_data.c vsxxx_data.c cdfs_data.c envram_data.c \
			  trn_sr_data.c

ILIST	= ${DATAFILES}
IDIR	= /usr/sys/data/
IMODE	= 444
IOWNER	= bin
IGROUP	= bin


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
