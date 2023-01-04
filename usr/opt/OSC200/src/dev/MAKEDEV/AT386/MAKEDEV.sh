#!/sbin/sh
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
#	@(#)MAKEDEV.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Device "make" file.  Valid arguments:
#	std	standard devices
#	local	configuration specific devices
# Tapes:
#	qt*	1/4" streamer
# Disks:
#	hd*	esdi disk
#	fd*	floppy disk
# Terminal multiplexors:
# Pseudo terminals:
#	pty*	set of 16 master and slave pseudo terminals
# Printers:
# Graphics/windows:
# Misc:
#	ingres	Ingres lock driver
#
umask 77
for i
do
case $i in

std)
        mknod console      c 1 0   ; chmod 622 console
        mknod tty          c 2 0   ; chmod 666 tty
        mknod mem          c 3 0   ; chmod 644 mem
        mknod kmem         c 3 1   ; chmod 644 kmem
        mknod null         c 3 2   ; chmod 666 null
        mknod mouse        c 12 0  ; chmod 666 mouse
        mknod kbd          c 11 0  ; chmod 666 kbd
        mknod klog         c 19 0  ; chmod 600 klog
        ;;


qt*)
        umask 0 ; unit=`expr $i : '..\(.*\)'`
        case $i in
        st*) chr=6; blk=2 ;;
        esac
        case $unit in
        0|1|2|3)
		rew=$unit
		norew=`expr $rew + 4`
		eight=`expr $rew + 8`
                twelve=`expr $rew + 12`
                mknod rst$unit    c $chr $rew
		mknod rst$eight   c $chr $eight
                mknod nrst$unit   c $chr $norew
                mknod nrst$eight  c $chr $twelve
		mknod qt$unit     b $blk 0
                umask 77
                ;;
        *)
                echo "bad unit number in: $i; use qt0 thru qt3"
                ;;
        esac
        ;;

hd*)
        unit=`expr $i : '..\(.*\)'`
        case $i in
	hd*) name=hd; blk=0; chr=4;;
        esac
	mknod ${name}${unit}a      b $blk `expr $unit '*' 8 + 0`
	mknod ${name}${unit}b      b $blk `expr $unit '*' 8 + 1`
	mknod ${name}${unit}c      b $blk `expr $unit '*' 8 + 2`
	mknod ${name}${unit}d      b $blk `expr $unit '*' 8 + 3`
	mknod ${name}${unit}e      b $blk `expr $unit '*' 8 + 4`
	mknod ${name}${unit}f      b $blk `expr $unit '*' 8 + 5`
	mknod ${name}${unit}g      b $blk `expr $unit '*' 8 + 6`
	mknod ${name}${unit}h      b $blk `expr $unit '*' 8 + 7`
	mknod r${name}${unit}a     c $chr `expr $unit '*' 8 + 0`
	mknod r${name}${unit}b     c $chr `expr $unit '*' 8 + 1`
	mknod r${name}${unit}c     c $chr `expr $unit '*' 8 + 2`
	mknod r${name}${unit}d     c $chr `expr $unit '*' 8 + 3`
	mknod r${name}${unit}e     c $chr `expr $unit '*' 8 + 4`
	mknod r${name}${unit}f     c $chr `expr $unit '*' 8 + 5`
	mknod r${name}${unit}g     c $chr `expr $unit '*' 8 + 6`
	mknod r${name}${unit}h     c $chr `expr $unit '*' 8 + 7`
        ;;

fd*|fh*)
        unit=`expr $i : '..\(.*\)'`
        case $i in
		fd*) name=fd; blk=1; chr=5; minor=3;;
		fh*) name=fd; blk=1; chr=5; minor=1;;
        esac
	mknod floppy b $blk $minor; mknod rfloppy c $chr $minor
	;;

ttys)
	mknod ttys0 c 13 0
	mknod ttys1 c 13 1
	;;


ingres|iilock|iidr)
	mknod iilock c 16 0	; chmod 600 iilock
	chown ingres iilock
	;;

pty*)
        class=`expr $i : 'pty\(.*\)'`
        case $class in
        0) offset=0 name=p;;
        1) offset=16 name=q;;
        2) offset=32 name=r;;
        *) echo "bad unit for pty in: $i; use pty0 thru pty2";;
        esac
        case $class in
        0|1|2)
                umask 0
                eval `echo $offset $name | awk ' { b=$1; n=$2 } END {
                        for (i = 0; i < 16; i++)
                                printf("mknod tty%s%x c 9 %d; \
                                        mknod pty%s%x c 10 %d; ", \
                                        n, i, b+i, n, i, b+i); }'`
                umask 77
                ;;
        esac
        ;;

local)
	sh MAKEDEV.local
	;;

*)
	echo "I don't know how to MAKEDEV $i."
	;;
esac
done
