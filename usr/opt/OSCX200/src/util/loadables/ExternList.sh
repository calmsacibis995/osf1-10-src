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
# args:
#	libname
#	nm command
# 	objects

LIBNAME=$1
NM=$2
FILENAME=${LIBNAME}_externals.c
echo "/* THIS FILE IS GENERATED AT BUILD TIME AUTOMAGICALLY." > $FILENAME
echo " * DO NOT MODIFY BY HAND!" >> $FILENAME
echo " * " >> $FILENAME
echo " * `date` " >> $FILENAME
echo " * " >> $FILENAME
echo " */" >> $FILENAME
shift
shift
$NM $* | sed -e '/ N /d;/ U /d;/ V /d;/ [a-z] /d;/^$/d' | awk '{print $3}' | sort -u | sed -e '/^$/d' | awk '{printf("extern void * %s();\n",$1)}' >> $FILENAME
echo "short __${LIBNAME}_VersionMaj__ = 0;" >> $FILENAME
echo "short __${LIBNAME}_VersionMin__ = 1;" >> $FILENAME
echo "typedef struct {" >> $FILENAME
echo "	char  	* e_symbol_name;" >> $FILENAME
echo "	void	* e_symbol_address;" >> $FILENAME
echo "	short	e_major_version," >> $FILENAME
echo "		e_minor_version;" >> $FILENAME
echo "  char    e_replaced;" >> $FILENAME
echo "} __ls_symbols_list__;" >> $FILENAME
echo "__ls_symbols_list__ __${LIBNAME}_externals__ [] = {" >> $FILENAME
$NM $* | sed -e '/ N /d;/ U /d;/ V /d;/ [a-z] /d;/^$/d' | awk '{print $3}' | sort -u | sed -e '/^$/d' | awk '{printf("\t{\"%s\",\t(void *)%s,\t0,\t1,0},\n",$1,$1)}' >> $FILENAME
echo "};" >> $FILENAME
