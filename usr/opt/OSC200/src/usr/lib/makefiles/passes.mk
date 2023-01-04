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
#
#	@(#)$RCSfile: passes.mk,v $ $Revision: 4.2.15.3 $ (DEC) $Date: 1993/06/16 17:59:06 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
#  These list the "tags" associated with each pass
#
_PASS_FIRST_TAGS_=EXPINC
_PASS_SECOND_TAGS_=EXPLIBC EXPCRT0 EXPLIBLD
_PASS_THIRD_TAGS_=EXPLOADER
_PASS_FOURTH_TAGS_=${NO_SHARED_LIBRARIES?:EXPSHLIB}
_PASS_FIFTH_TAGS_=EXPLIB
_PASS_BASIC_TAGS_=${SECURITY_ONLY?SECURITY:STANDARD}

#
#  These list the variables used to define subdirectories to recurse into
#
_EXPINC_SUBDIRS_=${EXPINC_SUBDIRS}
_EXPLIBC_SUBDIRS_=${EXPLIBC_SUBDIRS}
_EXPCRT0_SUBDIRS_=${EXPCRT0_SUBDIRS}
_EXPLIBLD_SUBDIRS_=${EXPLIBLD_SUBDIRS}
_EXPLOADER_SUBDIRS_=${EXPLOADER_SUBDIRS}
_EXPSHLIB_SUBDIRS_=${EXPSHLIB_SUBDIRS}
_EXPLIB_SUBDIRS_=${EXPLIB_SUBDIRS}
_SECURITY_SUBDIRS_=${SEC_SUBDIRS}
_STANDARD_SUBDIRS_=${SUBDIRS}

#
#  For each ACTION define the action for recursion, the passes for the
#  action, the targets for the complete action, and the targets for each
# pass of the action
#
_BUILD_ACTION_=dopass
_BUILD_PASSES_=FIRST SECOND THIRD FOURTH FIFTH BASIC
_BUILD_PASSES_=\
      ${EXPINC_ONLY?FIRST:${EXPLIBC_ONLY?SECOND:${EXPLOADER_ONLY?THIRD: \
      ${EXPSHLIB_ONLY?FOURTH:${EXPLIB_ONLY?FIFTH: \
      ${BASIC_ONLY?BASIC:FIRST SECOND THIRD FOURTH FIFTH BASIC}}}}}}
_BUILD_TARGETS_=${_BUILD_PASSES_/*/__DO_BUILD__PASS_&__}
_BUILD_EXPINC_TARGETS_=\
	${EXPINC_TARGETS?${EXPINC_TARGETS}:${INCLUDES/^/export_}}
_BUILD_EXPLIBC_TARGETS_=\
	${EXPLIBC_TARGETS}
_BUILD_EXPCRT0_TARGETS_=\
	${EXPCRT0_TARGETS}
_BUILD_EXPLIBLD_TARGETS_=\
	${EXPLIBLD_TARGETS}
_BUILD_EXPLOADER_TARGETS_=\
	${EXPLOADER_TARGETS}
_BUILD_EXPSHLIB_TARGETS_=\
	${EXPSHLIB_TARGETS}
_BUILD_EXPLIB_TARGETS_=\
	${EXPLIB_TARGETS}
_BUILD_SECURITY_TARGETS_=\
	${SEC_PROGRAMS} \
	${SEC_LIBRARIES} \
	${SEC_OBJECTS} \
	${SEC_SCRIPTS} \
	${SEC_DATAFILES} \
	${SEC_OTHERS} \
	${SEC_MANPAGES:=.0} \
	${SEC_DOCUMENTS/*/&.ps &.out}
_BUILD_STANDARD_TARGETS_=\
	${PROGRAMS} \
	${LIBRARIES} \
	${OBJECTS} \
	${SCRIPTS} \
	${_DATAFILES_} \
	${OTHERS} \
	${MANPAGES:=.0} \
	${DOCUMENTS/*/&.ps &.out}
_DOPASS_TARGETS_=__DO_BUILD__PASS_${MAKEFILE_PASS}__

_COMP_ACTION_=comp
_COMP_PASSES_=BASIC
_COMP_TARGETS_=${_COMP_PASSES_/*/__DO_COMP__PASS_&__}
_COMP_SECURITY_TARGETS_=\
	${_BUILD_SECURITY_TARGETS_}
_COMP_STANDARD_TARGETS_=\
	${_BUILD_STANDARD_TARGETS_}

_CLEAN_ACTION_=clean
_CLEAN_PASSES_=BASIC
_CLEAN_TARGETS_=${_CLEAN_PASSES_/*/__DO_CLEAN__PASS_&__}
_CLEAN_SECURITY_TARGETS_=\
	${SEC_PROGRAMS/^/_clean_prefix_} \
	${SEC_LIBRARIES/^/_clean_prefix_} \
	${SEC_OBJECTS/^/_clean_prefix_} \
	${SEC_SCRIPTS/^/_clean_prefix_} \
	${SEC_DATAFILES/^/_clean_prefix_} \
	${SEC_OTHERS/^/_clean_prefix_} \
	${SEC_MANPAGES/*/_clean_prefix_&.0} \
	${SEC_DOCUMENTS/*/_clean_prefix_&.ps _clean_prefix_&.out}
_CLEAN_STANDARD_TARGETS_=\
	${PROGRAMS/^/_clean_prefix_} \
	${LIBRARIES/^/_clean_prefix_} \
	${OBJECTS/^/_clean_prefix_} \
	${SCRIPTS/^/_clean_prefix_} \
	${_DATAFILES_/^/_clean_prefix_} \
	${OTHERS/^/_clean_prefix_} \
	${MANPAGES/*/_clean_prefix_&.0} \
	${DOCUMENTS/*/_clean_prefix_&.ps _clean_prefix_&.out}

_RMTARGET_ACTION_=rmtarget
_RMTARGET_PASSES_=BASIC
_RMTARGET_TARGETS_=${_RMTARGET_PASSES_/*/__DO_RMTARGET__PASS_&__}
_RMTARGET_SECURITY_TARGETS_=\
	${SEC_PROGRAMS/^/_rmtarget_prefix_} \
	${SEC_LIBRARIES/^/_rmtarget_prefix_} \
	${SEC_OBJECTS/^/_rmtarget_prefix_} \
	${SEC_SCRIPTS/^/_rmtarget_prefix_} \
	${SEC_DATAFILES/^/_rmtarget_prefix_} \
	${SEC_OTHERS/^/_rmtarget_prefix_} \
	${SEC_MANPAGES/*/_rmtarget_prefix_&.0} \
	${SEC_DOCUMENTS/*/_rmtarget_prefix_&.ps _rmtarget_prefix_&.out}
_RMTARGET_STANDARD_TARGETS_=\
	${PROGRAMS/^/_rmtarget_prefix_} \
	${LIBRARIES/^/_rmtarget_prefix_} \
	${OBJECTS/^/_rmtarget_prefix_} \
	${SCRIPTS/^/_rmtarget_prefix_} \
	${_DATAFILES_/^/_rmtarget_prefix_} \
	${OTHERS/^/_rmtarget_prefix_} \
	${MANPAGES/*/_rmtarget_prefix_&.0} \
	${DOCUMENTS/*/_rmtarget_prefix_&.ps _rmtarget_prefix_&.out}

_CLOBBER_ACTION_=clobber
_CLOBBER_PASSES_=BASIC
_CLOBBER_TARGETS_=${_CLOBBER_PASSES_/*/__DO_CLOBBER__PASS_&__}
_CLOBBER_SECURITY_TARGETS_=\
	${SEC_PROGRAMS/^/_clobber_prefix_} \
	${SEC_LIBRARIES/^/_clobber_prefix_} \
	${SEC_OBJECTS/^/_clobber_prefix_} \
	${SEC_SCRIPTS/^/_clobber_prefix_} \
	${SEC_DATAFILES/^/_clobber_prefix_} \
	${SEC_OTHERS/^/_clobber_prefix_} \
	${SEC_MANPAGES/*/_clobber_prefix_&.0} \
	${SEC_DOCUMENTS/*/_clobber_prefix_&.ps _clobber_prefix_&.out}
_CLOBBER_STANDARD_TARGETS_=\
	${PROGRAMS/^/_clobber_prefix_} \
	${LIBRARIES/^/_clobber_prefix_} \
	${OBJECTS/^/_clobber_prefix_} \
	${SCRIPTS/^/_clobber_prefix_} \
	${_DATAFILES_/^/_clobber_prefix_} \
	${OTHERS/^/_clobber_prefix_} \
	${MANPAGES/*/_clobber_prefix_&.0} \
	${DOCUMENTS/*/_clobber_prefix_&.ps _clobber_prefix_&.out}

_LINT_ACTION_=lint
_LINT_PASSES_=BASIC
_LINT_TARGETS_=${_LINT_PASSES_/*/__DO_LINT__PASS_&__}
_LINT_SECURITY_TARGETS_=\
	${SEC_PROGRAMS/^/_lint_prefix_} \
	${SEC_LIBRARIES/^/_lint_prefix_} \
	${SEC_OBJECTS/^/_lint_prefix_}
_LINT_STANDARD_TARGETS_=\
	${PROGRAMS/^/_lint_prefix_} \
	${LIBRARIES/^/_lint_prefix_} \
	${OBJECTS/^/_lint_prefix_}

_TAGS_ACTION_=tags
_TAGS_PASSES_=BASIC
_TAGS_TARGETS_=${_TAGS_PASSES_/*/__DO_TAGS__PASS_&__}
_TAGS_SECURITY_TARGETS_=\
	${SEC_PROGRAMS/^/_tags_prefix_} \
	${SEC_LIBRARIES/^/_tags_prefix_} \
	${SEC_OBJECTS/^/_tags_prefix_}
_TAGS_STANDARD_TARGETS_=\
	${PROGRAMS/^/_tags_prefix_} \
	${LIBRARIES/^/_tags_prefix_} \
	${OBJECTS/^/_tags_prefix_}

_EXPORT_ACTION_=export
_EXPORT_PASSES_=FIRST SECOND THIRD FOURTH FIFTH
_EXPORT_PASSES_=\
      ${EXPINC_ONLY?FIRST:${EXPLIBC_ONLY?SECOND:${EXPLOADER_ONLY?THIRD: \
      ${EXPSHLIB_ONLY?FOURTH:${EXPLIB_ONLY?FIFTH: \
      ${BASIC_ONLY? :FIRST SECOND THIRD FOURTH FIFTH}}}}}}
_EXPORT_TARGETS_=${_EXPORT_PASSES_/*/__DO_EXPORT__PASS_&__}
_EXPORT_EXPINC_TARGETS_=\
	${EXPINC_TARGETS?${EXPINC_TARGETS}:${INCLUDES/^/export_}}
_EXPORT_EXPLIBC_TARGETS_=\
	${EXPLIBC_TARGETS}
_EXPORT_EXPCRT0_TARGETS_=\
	${EXPCRT0_TARGETS}
_EXPORT_EXPLIBLD_TARGETS_=\
	${EXPLIBLD_TARGETS}
_EXPORT_EXPLOADER_TARGETS_=\
	${EXPLOADER_TARGETS}
_EXPORT_EXPSHLIB_TARGETS_=\
	${EXPSHLIB_TARGETS}
_EXPORT_EXPLIB_TARGETS_=\
	${EXPLIB_TARGETS}

_INSTALL_ACTION_=install
_INSTALL_PASSES_=BASIC
_INSTALL_TARGETS_=${_INSTALL_PASSES_/*/__DO_INSTALL__PASS_&__}
_INSTALL_SECURITY_TARGETS_=\
	${SEC_INSTDIRS/^/_instdir_prefix_} \
	${SEC_SYMLINKS/^/_symlink_prefix_} \
	${SEC_ILIST/^/_install_prefix_} 
_INSTALL_STANDARD_TARGETS_=\
	${INSTDIRS/^/_instdir_prefix_} \
	${SYMLINKS/^/_symlink_prefix_} \
	${ILIST/^/_install_prefix_} \
	${NEW_FILES?${NEW_FILES/*/_install_prefix_.new..&}:} \
	${UPD_FILES?${UPD_FILES/*/_install_prefix_.upd..&}:} 

_INSTDIR_ACTION_=instdir
_INSTDIR_PASSES_=BASIC
_INSTDIR_TARGETS_=${_INSTDIR_PASSES_/*/__DO_INSTDIR__PASS_&__}
_INSTDIR_SECURITY_TARGETS_=\
	${SEC_INSTDIRS/^/_instdir_prefix_}
_INSTDIR_STANDARD_TARGETS_=\
	${INSTDIRS/^/_instdir_prefix_}


#
#  These implement the mechanism described above.  The easiest way to
#  think of this is as a procedure call mechanism implemented using
#  make dependents and wildcard matching.
#

#
#  __DO_<action>__PASS_<pass>__
#
#  depends upon:
#
#	foreach tag (_PASS_<pass>_TAGS_)
#		__DO_<action>__PASS_<pass>__TAG_<tag>__
#	end
#	foreach tag (_PASS_<pass>_TAGS_)
#		__DO_<action>__TAG_<tag>__
#	end
#
__DO_%__PASS_%2__: $${_PASS_%2_TAGS_/*/__DO_%__PASS_%2__TAG_&__} \
		   $${_PASS_%2_TAGS_/*/__DO_%__TAG_&__};@

#
#  __DO_<action>__TAG_<tag>__
#
#  depends upon:
#
#	foreach target (_<action>_<tag>_TARGETS_)
#		<target>
#	end
#
__DO_%__TAG_%2__: $${_%_%2_TARGETS_};@

#
#  __DO_<action>__PASS_<pass>__TAG_<tag>__
#
#  depends upon:
#
#	foreach subdir (_<tag>_SUBDIRS_)
#		__DO_<action>__PASS_<pass>__SUBDIR_<subdir>__
#	end
#
__DO_%__PASS_%2__TAG_%3__: $${_%3_SUBDIRS_/*/__DO_%__PASS_%2__SUBDIR_&__};@
