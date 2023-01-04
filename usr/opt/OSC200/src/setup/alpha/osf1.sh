#!/bin/sh
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
# OSF/1 Release 1.0

#SEC_LEVEL="C2"
#export SEC_LEVEL
#
# Setup variables describing the current environment
#
MACHINE="alpha"
TARGET_MACHINE="ALPHA"
target_machine="alpha"
CPUTYPE="ALPHA"
cputype="alpha"
target_os_type="osf"
machine=`/bin/machine`
host_machine=`/bin/machine`
host_os_type=`/bin/uname`

export MACHINE machine TARGET_MACHINE target_machine CPUTYPE cputype \
target_os_type host_machine host_os_type

#
# Environment variables for the host compilers
#

C_COMPILER=alpha_cc

#
# Use coff for alpha
#
# Note: all these must be COFF if C_COMPILER=alpha_cc
HOST_OBJECT_FORMAT=COFF
CMD_OBJECT_FORMAT=COFF
KERNEL_OBJECT_FORMAT=COFF

ARCHIVE_FORMAT=COFF

export ARCHIVE_FORMAT

#
# Define these if you do not have them
#
#NO_VFPRINTF=NO_VFPRINTF
#export NO_VFPRINTF
#NO_SETENV=NO_SETENV
#export NO_SETENV
#NO_PW_STAYOPEN=NO_PW_STAYOPEN
#export NO_PW_STAYOPEN
#NO_DIRENT=NO_DIRENT
#export NO_DIRENT
#NO_UUDECODE=NO_UUDECODE
#export NO_UUDECODE
