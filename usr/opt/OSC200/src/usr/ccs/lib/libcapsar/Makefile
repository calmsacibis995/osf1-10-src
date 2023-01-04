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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/19 16:35:27 $

LIBRARIES 	= libcapsar.a
OFILES	= \
	capsar_Destroy.o \
	capsar_append_header.o \
	capsar_append_message.o \
	capsar_create.o \
	capsar_delete_header.o \
	capsar_delete_message.o \
	capsar_extract.o \
	capsar_get_body_type.o \
	capsar_get_buf.o \
	capsar_get_header.o \
	capsar_get_header_list.o \
	capsar_get_messages.o \
	capsar_get_name.o \
	capsar_get_separator.o \
	capsar_is_body_type.o \
	capsar_limitations.o \
	capsar_log.o \
	capsar_message_type.o \
	capsar_new.o \
	capsar_new_parse.o \
	capsar_parse_file.o \
	capsar_quick.o \
	capsar_report.o \
	capsar_set_name.o \
	capsar_set_separator.o \
	capsar_size.o \
	capsar_take_whole.o \
	capsar_unparse_file.o \
	concat.o \
	copy.o \
	error.o \
	getcpy.o \
	match.o \
	r1bindex.o

EXPLIB_TARGETS	= export_libcapsar.a
EXPDIR		= /usr/ccs/lib/

MSGHDRS		= libcapsar_msg.h
CATFILES	= libcapsar.cat
HFILES		= ${MSGHDRS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
