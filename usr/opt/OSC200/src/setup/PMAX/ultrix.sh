#
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
# @(#)$RCSfile: ultrix.sh,v $ $Revision: 4.1.3.4 $ (DEC) $Date: 1992/06/25 10:44:32 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#SEC_LEVEL="C2"
#export SEC_LEVEL

#
# Setup variables describing the current environment
#
MACHINE="mips"
TARGET_MACHINE="PMAX"
target_machine="pmax"
CPUTYPE="MIPS"
cputype="mips"
target_os_type="ultrix"

export MACHINE TARGET_MACHINE target_machine CPUTYPE cputype target_os_type

#
# Environment variables for the host compilers
#

C_COMPILER=mips_cc
gccsuffix="mcc"
#C_COMPILER=gnu_cc
#gccsuffix="gcc"

# HACKS for MIPS C in ULTRIX build environment. For things like genhash
# which must run on the build machine. The idea is to use the MIPS C
# compiler in tools/pmax/mcc, but get the include files and libraries
# from the build machine (confused? you are not alone!).
ULT_INCDIRS="-I/usr/include"
#ULT_INCDIRS="-I/usr/include `genpath -I.`"
export ULT_INCDIRS
ULT_LIBDIRS="-L/usr/lib -L/lib"
export ULT_LIBDIRS
ULTRIX_BLD=1
export ULTRIX_BLD

#
# Use coff (gcc) or macho version of gcc
#
# Note: all these must be COFF if C_COMPILER=mips_cc
HOST_OBJECT_FORMAT=COFF
CMD_OBJECT_FORMAT=COFF
KERNEL_OBJECT_FORMAT=COFF

ARCHIVE_FORMAT=COFF

export ARCHIVE_FORMAT

#
# Define these if you do not have them
#
NO_VFPRINTF=NO_VFPRINTF
export NO_VFPRINTF
NO_SETENV=NO_SETENV
export NO_SETENV
NO_PW_STAYOPEN=NO_PW_STAYOPEN
export NO_PW_STAYOPEN
#NO_DIRENT=NO_DIRENT
#export NO_DIRENT
#NO_UUDECODE=NO_UUDECODE
#export NO_UUDECODE
