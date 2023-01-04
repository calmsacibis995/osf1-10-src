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
# @(#)$RCSfile: cflow.sh,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/08/31 15:31:19 $ 
#
# COMPONENT_NAME: (CMDPROG) Programming Utilities
#
# FUNCTIONS: 
#
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

o="-DLANGUAGE_C -Dunix -D__LANGUAGE_C__ -D__unix__ -D__osf__ -D__alpha \
-D_LONGLONG -D_CFE -DSYSTYPE_BSD -D_SYSTYPE_BSD"
INVFLG=
DFLAG=
IFLAG=
MFLAG=

DIR=/usr/ccs/lib
FILES=
CFLAGS="-E"
CC=

TMP=/usr/tmp/cf.$$
TMPG=$TMP.g
trap "rm -f $TMP.?; kill $$" 1 2 3

base=`basename $0`
case $base in
        cflow | cflow.sh)       CC=/lib/cpp;;
        *cflow*)                CC=`echo $base | sed s/cflow/cpp/p`;;
        *)                      CC=/lib/cpp;;
esac

if [ $# -eq 0 ]
then
	goto usage_error
fi

echo "" >$TMP.g

# parse the command line into arguments
set -- `getopt rd:i:M:f:g:N:I:D:U: $*`

# parse the options
while [ "$1" != -- ]
do
        case "$1" in
        -r)
                INVFLG=1
                ;;
        -d)
                DFLAG="$1$2"
		shift
                ;;
        -i)
		if [ "$2" != "_" -a "$2" != "x" -a "$2" != "p" ]
		then
			goto usage_error
		fi
                IFLAG="$IFLAG $1$2"
		shift
                ;;
        -M)
		if [ "$2" != "A" ]
		then
			goto usage_error
		fi
                MFLAG="$MFLAG -MA"
		shift
                ;;
        -f)
                cat $2 </dev/null >>$TMPG
                shift
                ;;
        -g)
                TMPG=$2
                if [ "$TMPG" = "" ]
                then
                        TMPG=$TMP.g
                fi
                shift
                ;;
        -N)
                MFLAG="$MFLAG $1$2"
		shift
                ;;
        -[IDU])
                o="$o $1$2"
                shift
                ;;
        -*)
		goto usage_error
                ;;
        esac
        shift
done

# skip over the "--" getopt uses to seperate options from arguments
shift

# parse the file names
while [ "$1" != "" ]
do
        case "$1" in
        *.[ylciso])
                FILES="$FILES $1"           # Keep a list of files to process
                ;;
        *)
                dspmsg cflow.cat 1 "%s -- cflow can't process - file skipped\n" $1
                ;;
        esac
        shift
done

# Now process the specified files
for file in $FILES; do
        case $file in
                *.y)
                        yacc $file
                        sed -e "/^# line/d" y.tab.c > $file.c
                        $CC $CFLAGS $o $file.c > $TMP.i
                        $DIR/cflow1 $MFLAG -L$TMP.j $TMP.i 
                        if [ $? -eq 2 ]         # Unrecoverable failure in cflow1
                        then
                                 rm $TMP.? y.tab.c $file.c
                                 exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        rm y.tab.c $file.c
                        ;;
                *.l)
                        lex $file
                        sed -e "/^# line/d" lex.yy.c > $file.c
                        $CC $CFLAGS $o $file.c > $TMP.i
                        $DIR/cflow1 $MFLAG -L$TMP.j $TMP.i 
                        if [ $? -eq 2 ]         # Unrecoverable failure in cflow1
                        then
                                rm $TMP.? lex.yy.c $file.c
                                exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        rm lex.yy.c $file.c
                        ;;
                *.c)
                        $CC $CFLAGS $o $file > $TMP.i
                        $DIR/cflow1 $MFLAG -L$TMP.j $TMP.i 
                        if [ $? -eq 2 ]         # Unrecoverable failure in cflow1
                        then
                                rm $TMP.?
                                exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        ;;
                *.i)
                        $DIR/cflow1 $MFLAG -L$TMP.j $file 
                        if [ $? -eq 2 ]         # Unrecoverable failure in cflow1
                        then 
                                rm $TMP.? $file  
                                exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        ;;
                *.s)
                        a=`basename $file .s`
                        as -o $TMP.o $file
                        nm -he $TMP.o | sort -t'|' -n +1 -2 | $DIR/nmf $a ${a}.s >>$TMPG
                        ;;
                *.o)
                        a=`basename $file .o`
                        nm -he $file | sort -t'|' -n +1 -2 | $DIR/nmf $a ${a}.o >>$TMPG
                        ;;
        
        esac
done

if [ "$INVFLG" != "" ]
then
        grep "=" $TMPG >$TMP.q
        grep ":" $TMPG | $DIR/flip >>$TMP.q
        sort <$TMP.q >$TMPG
        rm $TMP.q
fi

$DIR/dag $DFLAG < $TMPG
rm -f $TMP.?

exit 0

usage_error:
dspmsg cflow.cat 2 "Usage: cflow [-dNumber] [-i_] [-ip] [-ix] [-MA] [-r]\n\
\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File [File ...]\n"
exit 0
