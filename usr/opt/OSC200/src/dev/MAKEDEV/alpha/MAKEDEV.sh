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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
#	Modified MAKEDEV for OSF/1 selfhosting
#	
#
# Device MAKEDEV script.
# See makedev(8) for more information.
#
# Generic:
#	std	    standard devices below:
#	drum		kernel drum device
#	klog		error log device
#	kcon		kernel console device
#	kbinlog         binary error log device
#	kmem		kernel memory device	
#	mem		memory device
#	null		null device
#	trace		trace device
#	tty		tty device
#	cam		CAM user agent
#	audit		audit device
#	prf		profiling device
#	zero		source of zero
#	local	    customer specific devices
#
# Systems:
#       DEC_2000
#	DEC_3000
#	DEC_4000
#	DEC_7000
#
# Consoles:
#	console     system console
#
# Disks:
#	rz*	    scsi disks
#	ra*	    mscp disks
#
# Tapes:
#	tz*	    scsi tapes
#	tms*	    tmscp tapes
#
# Graphics/Modems/Terminals/Printers:
#	pm*	    mono/color bitmap graphics/mouse
#	lat*	    sets of 16 network local area terminals (lat)
#	pty*	    sets of 16 network pseudoterminals
#	PTY_*	    sets of 368 network pseudoterminals
#	ptmx*       2 pty master cloning devices (one for bsd master;
#		    one for System V master)
#       lp*	    DEC2000_300 parallel printer (VTI Combo chip)
#
# Terminals:
#	dc*	    DECstation 3100 serial lines
#	mdc*	    DECsystem 5100 serial lines
#	scc*	    DECsystem 5000 serial lines
#	ace*	    DEC2000_300 serial lines (ACE UARTs on VTI Combo chip)
#	cobtty	    DEC4000 alternate serial line
#
# Security:
#	sec	    standard security devieces.
#
umask 77

case $1 in
-s) silent="true" ; shift;;
*) silent="" ;;
esac

# standard device list
std=\
"klog kcon kbinlog kmem mem null console pm0 tty pty0 pty1 ptmx cam audit prf zero"

for arg
do
case $arg in
std)
	devlist=$std
	;;
PTY_1) 
	devlist="$devlist pty0 pty1 pty2 pty3 pty4 pty5 pty6 pty7 pty8 pty9 
		 pty10 pty11 pty12 pty13 pty14 pty15 pty16 pty17 pty18 pty19\ 
		 pty20 pty21 pty22"
	;;
PTY_2)
	devlist="$devlist pty23 pty24 pty25 pty26 pty27 pty28 pty29 pty30\
 	 	 pty31 pty32 pty33 pty34 pty35 pty36 pty37 pty38 pty39 pty40\
		 pty41 pty42 pty43 pty44 pty45"
	;;
PTY_3) 
	devlist="$devlist pty46 pty47 pty48 pty49 pty50 pty51 pty52 pty53\
		  pty54 pty55 pty56"
	;;
PTY_4)
	devlist="$devlist pty57 pty58 pty59 pty60 pty61 pty62 pty63 pty64"
	;;
PTY_5)
	devlist="$devlist pty65 pty66 pty67 pty68 pty69 pty70 pty71 pty72"
	;;
PTY_6)
	devlist="$devlist pty73 pty74 pty75 pty76 pty77 pty78 pty79 pty80"
	;;
PTY_7)
	devlist="$devlist pty81 pty82 pty83 pty84 pty85 pty86 pty87 pty88"

	;;
PTY_8)
	devlist="$devlist pty89 pty90 pty91 pty92 pty93 pty94 pty95 pty96"
	;;
PTY_9)
	devlist="$devlist pty97 pty98 pty99 pty100 pty101"
	;;
*)	devlist="$devlist $arg"
	;;
esac
done

for fullname in $devlist
do

tryname=`expr $fullname : ',*\(.[a-zA-Z]*\)'`
tryunit=`expr $fullname : '[^0-9]*\([0-9]*\)'`
trypart=`expr $fullname : '[^0-9]*[0-9]*\(.*\)'`
if test "X$silent" = "X"  ; then
	echo "MAKEDEV: special file(s) for "$fullname": "
fi

case $tryname in 
	klog|kcon|kbinlog|kmem|mem|null|trace|tty|cam|audit|prf|zero)
	case $tryname in
	drum) chr=4 ; base=0 ; prot=644;;
	klog) chr=3 ; base=0 ; prot=600;;
	kcon) chr=3 ; base=1 ; prot=600;;
	kbinlog) chr=31 ; base=0 ; prot=600;;
	kmem) chr=2 ; base=1 ; prot=440;;
	mem) chr=2 ; base=0 ; prot=440;;
	null) chr=2 ; base=2 ; prot=666;;
	trace) chr=11 ; base=0 ; prot=444;;
	tty) chr=1 ; base=0 ; prot=666;;
	cam) chr=30 ; base=0 ; prot=600;;
	audit) chr=17 ; base=0 ; prot=400;;
	prf) chr=37; base=0; prot=600;;
	zero) chr=38; base=0; prot=666;;
	esac
	if test ! -c ./$tryname ; then
		echo $tryname" \c" ; mknod $tryname c \
		$chr $base ; chmod $prot $tryname
	fi
;;
console)
	case $tryname in
	console) chr=0 ; base=0 ; prot=622;;
	esac
	case $tryname in
	console)
		if test ! -c ./$tryname ; then
			echo $tryname" \c" ; mknod $tryname c \
			$chr $base ; chmod $prot $tryname
		fi
	;;
	esac
;;
rz)
	partlet=$trypart
	maxunits=128
	case $tryname in
	rz) blk=8; chr=8;;
	esac
	if test $tryunit -lt $maxunits 2>/dev/null
	then
	    case $tryname in
	    rz)
		case $trypart in
		a) partnum=0;; b) partnum=1;; c) partnum=2;;
		d) partnum=3;; e) partnum=4;; f) partnum=5;;
		g) partnum=6;; h) partnum=7;;
		[i-zA-Z])
		    echo MAKEDEV: bad partition value in: $fullname
		    exit 1
		;;
		esac
		case $trypart in
		[a-h])
		    if test ! -b ./$tryname$tryunit$partlet ; then
		    echo $tryname$tryunit$partlet" \c" ; \
		    mknod $tryname$tryunit$partlet b \
			       $blk `expr $tryunit '/' 8 '*' 16384 + $tryunit '%' 8 \
			       '*' 1024  + $partnum`
		    fi
		    if test ! -c ./r$tryname$tryunit$partlet ; then
		    echo "r"$tryname$tryunit$partlet" \c" ; \
		    mknod r$tryname$tryunit$partlet c \
			       $chr `expr $tryunit '/' 8 '*' 16384 + $tryunit '%' 8 \
			       '*' 1024  + $partnum`
		    fi
		;;
		*)
		    for partlet in a b c d e f g h
		    do
		    case $partlet in
		    a) partnum=0;; b) partnum=1;; c) partnum=2;;
		    d) partnum=3;; e) partnum=4;; f) partnum=5;;
		    g) partnum=6;; h) partnum=7;;
		    esac
		    if test ! -b ./$tryname$tryunit$partlet ; then
		    echo $tryname$tryunit$partlet" \c" ; \
		    mknod $tryname$tryunit$partlet b \
			       $blk `expr $tryunit '/' 8 '*' 16384 + $tryunit '%' 8 \
			       '*' 1024  + $partnum`
		    fi
		    if test ! -c ./r$tryname$tryunit$partlet ; then
		    echo "r"$tryname$tryunit$partlet" \c" ; \
		    mknod r$tryname$tryunit$partlet c \
			       $chr `expr $tryunit '/' 8 '*' 16384 + $tryunit '%' 8 \
			       '*' 1024  + $partnum`
		    fi
		    done
		;;
		esac
	    ;;
	    esac
	else
	    echo MAKEDEV: bad disk unit in: $fullname
	    exit 1
	fi
;;
ra)
        partlet=$trypart
        maxunits=256
        case $tryname in
        ra) blk=23; chr=28;;
        esac
        if test $tryunit -lt $maxunits 2>/dev/null
        then
            case $tryname in
            ra)
                case $trypart in
                a) partnum=0;; b) partnum=1;; c) partnum=2;;
                d) partnum=3;; e) partnum=4;; f) partnum=5;;
                g) partnum=6;; h) partnum=7;;
                [i-zA-Z])
                    echo MAKEDEV: bad partition value in: $fullname
                    exit 1
                ;;
                esac
                case $trypart in
                [a-h])
                    if test ! -b ./$tryname$tryunit$partlet ; then
                    echo $tryname$tryunit$partlet" \c" ; \
                    mknod $tryname$tryunit$partlet b \
                               $blk `expr $tryunit '*' 64 + $partnum`
                    fi
                    if test ! -c ./r$tryname$tryunit$partlet ; then
                    echo "r"$tryname$tryunit$partlet" \c" ; \
                    mknod r$tryname$tryunit$partlet c \
                               $chr `expr $tryunit '*' 64 + $partnum`
                    fi
                ;;
                *)
                    for partlet in a b c d e f g h
                    do
                    case $partlet in
                    a) partnum=0;; b) partnum=1;; c) partnum=2;;
                    d) partnum=3;; e) partnum=4;; f) partnum=5;;
                    g) partnum=6;; h) partnum=7;;
                    esac
                    if test ! -b ./$tryname$tryunit$partlet ; then
                    echo $tryname$tryunit$partlet" \c" ; \
                    mknod $tryname$tryunit$partlet b \
                               $blk `expr $tryunit '*' 64 + $partnum`
                    fi
                    if test ! -c ./r$tryname$tryunit$partlet ; then
                    echo "r"$tryname$tryunit$partlet" \c" ; \
                    mknod r$tryname$tryunit$partlet c \
                               $chr `expr $tryunit '*' 64 + $partnum`
                    fi
                    done
                ;;
                esac
            ;;
            esac
        else
            echo MAKEDEV: bad disk unit in: $fullname
            exit 1
        fi
;;
fd)
        partlet=$trypart
        maxunits=2
        blk=14; chr=14
        if test $tryunit -lt $maxunits 2>/dev/null
        then
            case $tryname in
            fd)
                case $trypart in
                a) partnum=0;; b) partnum=1;; c) partnum=2;;
                d) partnum=3;; e) partnum=4;; f) partnum=5;;
                g) partnum=6;; h) partnum=7;;
                [i-zA-Z])
                    echo MAKEDEV: bad partition value in: $fullname
                    exit 1
                ;;
                esac
                case $trypart in
                [a-h])
                    if test ! -b ./$tryname$tryunit$partlet ; then
                    echo $tryname$tryunit$partlet" \c" ; \
                    mknod $tryname$tryunit$partlet b \
                               $blk `expr $tryunit '*' 8 + $partnum`
                    fi
                    if test ! -c ./r$tryname$tryunit$partlet ; then
                    echo "r"$tryname$tryunit$partlet" \c" ; \
                    mknod r$tryname$tryunit$partlet c \
                               $chr `expr $tryunit '*' 8 + $partnum`
                    fi
                ;;
                *)
		    # only make the a and c partition by default
                    for partlet in a c
                    do
                    case $partlet in
                    a) partnum=0;; b) partnum=1;; c) partnum=2;;
                    d) partnum=3;; e) partnum=4;; f) partnum=5;;
                    g) partnum=6;; h) partnum=7;;
                    esac
                    if test ! -b ./$tryname$tryunit$partlet ; then
                    echo $tryname$tryunit$partlet" \c" ; \
                    mknod $tryname$tryunit$partlet b \
                               $blk `expr $tryunit '*' 8 + $partnum`
                    fi
                    if test ! -c ./r$tryname$tryunit$partlet ; then
                    echo "r"$tryname$tryunit$partlet" \c" ; \
                    mknod r$tryname$tryunit$partlet c \
                               $chr `expr $tryunit '*' 8 + $partnum`
                    fi
                    done
                ;;
                esac
            ;;
            esac
        else
            echo MAKEDEV: bad disk unit in: $fullname
            exit 1
        fi
;;
tms)
	umask 0 ; unit=$tryunit ; count=0
	maxunits=32
	case $tryname in
	tms) chr=27;;
	esac
	if test $tryunit -lt $maxunits 2>/dev/null
	then
	    while :
	    do
	    if test -c ./rmt$count"h"
	    then
		count=`expr $count + 1`
	    else
		break
	    fi
	    done
# test for ls because of standalone system doesn't have ls.
# Remove in future, when standalone system updated.
	    if test -f /bin/ls
	    then
		ls -l *rmt* > tmp$$ 2> /dev/null
	    fi
	    case $tryname in
	    tms)
		for k in 0 1 2 3 
		do
		    minnum=`expr $tryunit '*' 64 + $k`
# test for awk because of standalone system doesn't have awk.
# Remove in future, when standalone system updated.
		    if test -f /bin/awk
		    then
			made=`awk ' BEGIN { s = "n" }
			    { if ($4 == '$chr'"," && $5 == '$minnum')
				s = "y"
			    }
			    END { print s }' tmp$$`
		    else
			made="n"
		    fi
		    if test $made = "y"
		    then
			continue
		    else
			case $k in
			0) echo "rmt"$count"l "
			   mknod ./rmt$count"l" c $chr $minnum
			;;
			1) echo "nrmt"$count"l "
			   mknod ./nrmt$count"l" c $chr $minnum
			;;
			2) echo "rmt"$count"h "
			   mknod ./rmt$count"h" c $chr $minnum
			;;
			3) echo "nrmt"$count"h "
			    mknod ./nrmt$count"h" c $chr $minnum
			;;
			esac
		    fi
		done
	    ;;
	    esac
# test for ls because of standalone system doesn't have ls.
# Remove in future, when standalone system updated.
	    if test -f /bin/ls
	    then
		rm tmp$$
	    fi
	else
		echo MAKEDEV: bad tape unit in: \
		     $fullname : $maxunits tape devices maximum
		exit 1
	fi
;;
tz)
        umask 0 ; unit=$tryunit ; count=0
        maxunits=128
        case $tryname in
        tz) chr=9;;
        esac
        if test $tryunit -lt $maxunits 2>/dev/null
        then
            while :
            do
            if test -c ./rmt$count"h"
            then
                count=`expr $count + 1`
            else
                break
            fi
            done
# test for ls because of standalone system doesn't have ls.
# Remove in future, when standalone system updated.
            if test -f /bin/ls
            then
                ls -l *rmt* > tmp$$ 2> /dev/null
            fi
            case $tryname in
            tz)
                for k in 0 2 4 6 1 3 5 7
                do
                    minnum=`expr $tryunit '/' 8 '*' 16384 + $tryunit '%' 8 \
                               '*' 1024  + $k`
# test for awk because of standalone system doesn't have awk.
# Remove in future, when standalone system updated.
                    if test -f /bin/awk
                    then
                        made=`awk ' BEGIN { s = "n" }
                            { if ($4 == '$chr'"," && $5 == '$minnum')
                                s = "y"
                            }
                            END { print s }' tmp$$`
                    else
                        made="n"
                    fi
                    if test $made = "y"
                    then
                        continue
                    else
                        case $k in
                        0) echo "rmt"$count"l "
                           mknod ./rmt$count"l" c $chr $minnum
                        ;;
                        1) echo "nrmt"$count"l "
                           mknod ./nrmt$count"l" c $chr $minnum
                        ;;
                        2) echo "rmt"$count"h "
                           mknod ./rmt$count"h" c $chr $minnum
                        ;;
                        3) echo "nrmt"$count"h "
                            mknod ./nrmt$count"h" c $chr $minnum
                        ;;
                        4) echo "rmt"$count"m "
                            mknod ./rmt$count"m" c $chr $minnum
                        ;;
                        5) echo "nrmt"$count"m "
                            mknod ./nrmt$count"m" c $chr $minnum
                        ;;
                        6) echo "rmt"$count"a "
                            mknod ./rmt$count"a" c $chr $minnum
                        ;;
                        7) echo "nrmt"$count"a "
                            mknod ./nrmt$count"a" c $chr $minnum
                        ;;
                        esac
                    fi
                done
            ;;
            esac
# test for ls because of standalone system doesn't have ls.
# Remove in future, when standalone system updated.
            if test -f /bin/ls
            then
                rm tmp$$
            fi
        else
                echo MAKEDEV: bad tape unit in: \
                     $fullname : $maxunits tape devices maximum
                exit 1
        fi

;;
ptmx)
	umask 0
	case $tryname in
	ptmx) chr=32 ; base=7 ;
		if test ! -c ./$tryname"_bsd" ; then
			echo $tryname"_bsd \c" ; mknod ./$tryname"_bsd" c \
			$chr $base 
		fi
		if test -c ./"ptm" ; then
			rm ./"ptm" 
		fi
		echo "ptm \c" ; ln  ./"ptmx_bsd" ./"ptm" 
	;;
	esac
	umask 77
;;
dc|mdc|scc|ace|cobtty|lp|xcons|lat|pm|pty|pfilt)
	umask 44
	unit=$tryunit
	maxunits=10
	id1=0
	id2=0
	minnum=0
	scan=1
	scancnt=0
	savid1=0; savid2=0
	case $tryname in
	dc) umask 111; chr=21; base=0; linecnt=4; skipcnt=0; maxunits=4;;
	scc) umask 111; chr=24; base=0; linecnt=4; skipcnt=0; maxunits=4;;
	ace) umask 111; chr=35; base=0; linecnt=4; skipcnt=0; maxunits=4;;
	cobtty) umask 111; chr=0; base=1;; 
	mdc) umask 111; chr=0; base=1; linecnt=11; skipcnt=0; maxunits=3;;
	lat) chr=5; linecnt=16; skipcnt=0; base=0 ; maxunits=39;;
        pfilt) umask 066; chr=13; base=0; linecnt=64; skipcnt=0; maxunits=4;
		if [ X$tryunit = X ]
		then
			tryunit=0
		fi ;;
	pty) umask 0; ttychr=6; ptychr=7; id2=0;;
	pm) umask 117; chr=0; base=0;;
	xcons) umask 111; chr=23; base=0;;
	lp) umask 444; chr=34; base=0;;
	esac
	case $tryname in
	lat)
	if test $tryunit -ge $maxunits 2>/dev/null
	then
		echo MAKEDEV: bad terminal unit in: \
		     $fullname : $maxunits terminal devices maximum
		umask 77
		exit 1
	fi
	case $tryunit in
	[0-9]*) ;;
	*)
		echo MAKEDEV: bad terminal unit in: \
		     $fullname : $maxunits terminal devices maximum
		umask 77
		exit 1;;
	esac
#       Loop thru ttyname space to find a complete set of 16(or 12 if
#       lat38) unused ttynames and save the first ttyid number of the
#       set.
#       Intentionally hard-coded 38, this is a specific case not dynamic.
        if test $tryunit -eq 38
        then
                linecnt=12
        fi
	ls -o lat* > tmp$$ 2> /dev/null
	clonenum=5000
	made=`awk ' BEGIN { s = "n" }
		{ if ($4 == "'$chr','$clonenum'")
			s = "y"
		if ($4 == '$chr'"," && $5 == '$clonenum')
			s = "y"
		}
		END { print s }' tmp$$`
	if test $made = "n"
	then
                if test -c ./lat
                then
                        echo "\ndevice lat exists with different maj,min #"
                        echo "device lat not made"
                else
			umask 0
			echo "lat \c"
			mknod lat c $chr $clonenum
		fi
	fi
	rm tmp$$
	while :
	do
	if test -c ./tty$id1$id2
	then
		scancnt=0
	else
		scancnt=`expr $scancnt + 1`
		if test $scancnt -eq 1
		then
#			Normal exit here
			savid1=$id1
			savid2=$id2
		fi
		if test $scancnt -ge `expr $linecnt - $base`
		then
			id1=$savid1
			id2=$savid2
			break
		fi
	fi
	case $id2 in
	a) id2=b;;  b) id2=c;;
	c) id2=d;;  d) id2=e;;  e) id2=f;;
	f) id2=g;;  g) id2=h;;  h) id2=i;;
	i) id2=j;;  j) id2=k;;  k) id2=l;;
	l) id2=m;;  m) id2=n;;  n) id2=o;;
	o) id2=p;;  p) id2=q;;  q) id2=r;;
	r) id2=s;;  s) id2=t;;  t) id2=u;;
	u) id2=v;;  v) id2=w;;  w) id2=x;;
	x) id2=y;;  y) id2=z;;
	z) id2=A;;
	A) id2=B;;  B) id2=C;;
        C) id2=D;;  D) id2=E;;  E) id2=F;;
        F) id2=G;;  G) id2=H;;  H) id2=I;;
        I) id2=J;;  J) id2=K;;  K) id2=L;;
        L) id2=M;;  M) id2=N;;  N) id2=O;;
        O) id2=P;;  P) id2=Q;;  Q) id2=R;;
        R) id2=S;;  S) id2=T;;  T) id2=U;;
        U) id2=V;;  V) id2=W;;  W) id2=X;;
        X) id2=Y;;  Y) id2=Z;;
        Z) id2=0
	   id1=`expr $id1 + 1`
	   if test $id1 -ge 10
 	   then
#		We have looped thru all 620 names and have not found a
#		set of unused names.  Abnormal exit.  Notify the user.
		echo "\nMAKEDEV: Not enough contiguous ttynames available to \
create a set of LAT devices"
		umask 77
		exit 1
	   fi
	;;
	*) id2=`expr $id2 + $scan`
	   if test $id2 -ge 10
	   then
		id2=a
	   fi
	;;
	esac
	done
	ls -o tty* > tmp$$ 2> /dev/null
	moffset=16
	while :
	do
	if test $base -ne $linecnt
	then
		minnum=`expr $moffset '*' $tryunit + \
		       $skipcnt '*' $tryunit + $base`
		base=`expr $base + 1`
		if test $minnum -ge 620
		then
#			We should not hit this case, so leave w/o error.
			break
		fi
		made=`awk ' BEGIN { s = "n" }
			{ if ($4 == '$chr'"," && $5 == '$minnum')
				s = "y"
			  if ($4 == "'$chr','$minnum'")
				s = "y"
			}
			END { print s }' tmp$$`
		if test $made = "y"
		then
			continue
		else
			echo "tty"$id1$id2" \c"
			mknod tty$id1$id2 c $chr $minnum
		fi
		case $id2 in
		a) id2=b;;  b) id2=c;;
		c) id2=d;;  d) id2=e;;  e) id2=f;;
		f) id2=g;;  g) id2=h;;  h) id2=i;;
		i) id2=j;;  j) id2=k;;  k) id2=l;;
		l) id2=m;;  m) id2=n;;  n) id2=o;;
		o) id2=p;;  p) id2=q;;  q) id2=r;;
		r) id2=s;;  s) id2=t;;  t) id2=u;;
		u) id2=v;;  v) id2=w;;  w) id2=x;;
		x) id2=y;;  y) id2=z;;
		z) id2=A;;
		A) id2=B;;  B) id2=C;;
                C) id2=D;;  D) id2=E;;  E) id2=F;;
                F) id2=G;;  G) id2=H;;  H) id2=I;;
                I) id2=J;;  J) id2=K;;  K) id2=L;;
                L) id2=M;;  M) id2=N;;  N) id2=O;;
                O) id2=P;;  P) id2=Q;;  Q) id2=R;;
                R) id2=S;;  S) id2=T;;  T) id2=U;;
                U) id2=V;;  V) id2=W;;  W) id2=X;;
                X) id2=Y;;  Y) id2=Z;;
                Z) id2=0
	   	   id1=`expr $id1 + 1`
	   	   if test $id1 -ge 10
 	   	   then
			if test $base -eq $linecnt
			then
				break
			else
				echo MAKEDEV: Complete set of ttynames not available.
				umask 77
				exit 1
			fi
	   	   fi
		;;
		*) id2=`expr $id2 + $scan`
	   	   if test $id2 -ge 10
	   	   then
			id2=a
	   	   fi
		;;
		esac
	else
		break
	fi
	done
	rm tmp$$
	umask 77
	;;
	pfilt)
	    case $tryunit in
		0) base=0;;
		1) base=64;;
		2) base=128;;
		3) base=192;;
		*)
			echo "MAKEDEV: bad packetfilter unit in:\
				$fullname : 4 packetfilter devices maximum.\n"
			umask 77
			exit 1
		;;
	    esac
	    umask 22
	    if test ! -d pf
	    then
		mkdir pf
	    fi

	    umask 133
	    while :
	    do
		if test $id2 -le 63; then
			min=`expr $base + $id2`
			if test ! -c ./pf/pfilt$min ; then
			echo "pfilt$min \c" ; mknod pf/pfilt$min c\
							$chr $min
			fi
			id2=`expr $id2 + 1`
		else
			break;
		fi
	    done
	umask 77
	;;
	pty)
		case $tryunit in
		0) base=0 ; id1=p;;
		1) base=16 ; id1=q;;
		2) base=32 ; id1=r;;
		3) base=48 ; id1=s;;
		4) base=64 ; id1=t;;
		5) base=80 ; id1=u;;
		6) base=96 ; id1=v;;
		7) base=112 ; id1=w;;
		8) base=128 ; id1=x;;
		9) base=144 ; id1=y;;
		10) base=160 ; id1=z;;
		11) base=176 ; id1=a;;
		12) base=192 ; id1=b;;
		13) base=208 ; id1=c;;
		14) base=224 ; id1=e;;
		15) base=240 ; id1=f;;
 		16) base=256 ; id1=g;;
		17) base=272 ; id1=h;;
		18) base=288 ; id1=i;;
		19) base=304 ; id1=j;;
		20) base=320 ; id1=k;;
		21) base=336 ; id1=l;;
		22) base=352 ; id1=m;;
		23) base=368 ; id1=n;;
		24) base=384 ; id1=o;;
		25) base=400 ; id1=A;;
		26) base=416 ; id1=B;;
		27) base=432 ; id1=C;;
		28) base=448 ; id1=D;;
		29) base=464 ; id1=E;;
		30) base=480 ; id1=F;;
		31) base=496 ; id1=G;;
		32) base=512 ; id1=H;;
		33) base=528 ; id1=I;;
		34) base=544 ; id1=J;;
		35) base=560 ; id1=K;;
		36) base=576 ; id1=L;;
		37) base=592 ; id1=M;;
		38) base=608 ; id1=N;;
		39) base=624 ; id1=O;;
		40) base=640 ; id1=P;;
		41) base=656 ; id1=Q;;
		42) base=672 ; id1=R;;
		43) base=688 ; id1=S;;
		44) base=704 ; id1=T;;
		45) base=720 ; id1=U;;
		46) base=736 ; id1=V;;
		47) base=752 ; id1=W;;
		48) base=768 ; id1=X;;
		49) base=784 ; id1=Y;;
 		50) base=800 ; id1=Z;;
		51) base=816 ; id1=p;;
		52) base=862 ; id1=q;;
		53) base=908 ; id1=r;;
		54) base=954 ; id1=s;;
		55) base=1000 ; id1=t;;
		56) base=1046 ; id1=u;;
		57) base=1092 ; id1=v;;
		58) base=1138 ; id1=w;;
		59) base=1184 ; id1=x;;
		60) base=1230 ; id1=y;;
		61) base=1276 ; id1=z;;
		62) base=1322 ; id1=a;;
		63) base=1368 ; id1=b;;
		64) base=1414 ; id1=c;;
		65) base=1460 ; id1=e;;
 		66) base=1506 ; id1=f;;
		67) base=1552 ; id1=g;;
		68) base=1598 ; id1=h;;
		69) base=1644 ; id1=i;;
		70) base=1690 ; id1=j;;
		71) base=1736 ; id1=k;;
		72) base=1782 ; id1=l;;
		73) base=1828 ; id1=m;;
		74) base=1874 ; id1=n;;
		75) base=1920 ; id1=o;;
		76) base=1966 ; id1=A;;
		77) base=2012 ; id1=B;;
		78) base=2058 ; id1=C;;
		79) base=2104 ; id1=D;;
		80) base=2150 ; id1=E;;
		81) base=2196 ; id1=F;;
		82) base=2242 ; id1=G;;
		83) base=2288 ; id1=H;;
		84) base=2334 ; id1=I;;
		85) base=2380 ; id1=J;;
		86) base=2426 ; id1=K;;
		87) base=2472 ; id1=L;;
		88) base=2518 ; id1=M;;
		89) base=2564 ; id1=N;;
		90) base=2610 ; id1=O;;
		91) base=2656 ; id1=P;;
		92) base=2702 ; id1=Q;;
		93) base=2748 ; id1=R;;
		94) base=2794 ; id1=S;;
		95) base=2840 ; id1=T;;
		96) base=2886 ; id1=U;;
		97) base=2932 ; id1=V;;
		98) base=2978 ; id1=W;;
		99) base=3024 ; id1=X;;
		100) base=3070 ; id1=Y;;
		101) base=3116 ; id1=Z;;
		*)
			echo MAKEDEV: bad pseudoterminal unit in: \
			     $fullname : 101 pseudoterminal devices maximum
			umask 77
			exit 1
		;;
		esac
		while test $base -le 800
		do
		if test $id2 -ge 10
		then
		    case $id2 in
		    10) id2l=a;; 
		    11) id2l=b;; 
		    12) id2l=c;; 
		    13) id2l=d;;
		    14) id2l=e;; 
		    15) id2l=f;;
		     *) break;;
		    esac
		else
		    id2l=$id2
		fi
		if test ! -c ./pty$id1$id2l ; then
		echo "pty"$id1$id2l" \c"; mknod pty$id1$id2l c \
					  $ptychr `expr $base + $id2`
		fi
		if test ! -c ./tty$id1$id2l ; then
		echo "tty"$id1$id2l" \c"; mknod tty$id1$id2l c \
					  $ttychr `expr $base + $id2`
		fi
		id2=`expr $id2 + 1`
		done
		while test $base -gt 800
		do
		    case $id2 in
		    0)  id2l=g;;
		    1)  id2l=h;;
		    2)  id2l=i;;
		    3)  id2l=j;;
		    4)  id2l=k;;
		    5)  id2l=l;;
		    6)  id2l=m;;
		    7)  id2l=n;;
		    8)  id2l=o;;
		    9)  id2l=p;;
		    10)  id2l=q;;
		    11)  id2l=r;;
		    12)  id2l=s;;
		    13)  id2l=t;;
		    14)  id2l=u;;
		    15)  id2l=v;;
		    16)  id2l=w;;
		    17)  id2l=x;;
		    18)  id2l=y;;
		    19)  id2l=z;;
		    20) id2l=A;; 
		    21) id2l=B;; 
		    22) id2l=C;; 
		    23) id2l=D;;
		    24) id2l=E;; 
		    25) id2l=F;; 
		    26) id2l=G;; 
		    27) id2l=H;; 
		    28) id2l=I;; 
		    29) id2l=J;; 
		    30) id2l=K;; 
		    31) id2l=L;; 
		    32) id2l=M;; 
		    33) id2l=N;; 
		    34) id2l=O;; 
		    35) id2l=P;; 
		    36) id2l=Q;; 
		    37) id2l=R;; 
		    38) id2l=S;; 
		    39) id2l=T;; 
		    40) id2l=U;; 
		    41) id2l=V;; 
		    42) id2l=W;; 
		    43) id2l=X;; 
		    44) id2l=Y;; 
		    45) id2l=Z;; 
	             *) break;;
		    esac
		    if test ! -c ./tty$id1$id2l ; then
		    echo "tty"$id1$id2l" \c"; mknod tty$id1$id2l c \
					  $ttychr `expr $base + $id2`
		    fi
		    id2=`expr $id2 + 1`
		done
		umask 77
	;;
	pm)
		case $tryunit in
		0)
			test -c ./mouse ||
			{
				echo "mouse \c"
				mknod mouse c $chr `expr $base + 1`
#				echo "ttyd0 \c"
#				mknod ttyd0 c $chr `expr $base + 2`
#				echo "lp0 \c"
#				mknod lp0 c $chr `expr $base + 3`
			}
		;;
		*)
			echo MAKEDEV: bad pm unit in: $fullname
			umask 77
			exit 1
		;;
		esac
		umask 77
	;;
	xcons)
		test -c ./xcons ||
		{
			echo -n "xcons "
			mknod xcons c $chr 0
		}
	;;
	lp)
		test -c ./lp0 ||
		{
			echo "lp0 "
			mknod lp0 c $chr 0
		}
	;;
	ace)  
		case $tryunit in
		0)
			test -c ./tty00 || 
			{	echo "tty00 \c"
				mknod tty00 c $chr $base
			}
			test -c ./tty01 ||
			{
				echo "tty01 \c"
				mknod tty01 c $chr `expr $base + 1`
			}				
		;;
		*)
			echo MAKEDEV: bad tty unit in: $fullname
			umask 77
			exit 1
		;;
		esac
		umask 77

	;;
	dc|scc)  
		case $tryunit in
		0)
			test -c ./tty00 || 
			{	echo "tty00 \c"
				mknod tty00 c $chr `expr $base + 2`
			}
			test -c ./tty01 ||
			{
				echo "tty01 \c"
				mknod tty01 c $chr `expr $base + 3`
			}				
		;;
		*)
			echo MAKEDEV: bad tty unit in: $fullname
			umask 77
			exit 1
		;;
		esac
		umask 77
	;;
	cobtty)  
		test -c ./tty00 ||

		{	echo "tty00 \c"
			mknod tty00 c $chr $base 
		}
	;;
	mdc)
		case $tryunit in 
		0)
			umask 77
			mdclinenum=0;
			mdcones=0
			while test $mdclinenum -lt $linecnt
			do
				if test $mdcones -lt 10
				then 
					mdctens=0
				else
					mdctens=""
				fi
				test -c ./tty$mdctens$mdcones || {
				echo "tty"$mdctens$mdcones" \c"
				mknod tty$mdctens$mdcones c $chr $base
				}
				base=`expr $base + 1`
				mdcones=`expr $mdcones + 1`
				mdclinenum=`expr $mdclinenum + 1`
			done
		;;
		*)
			echo MAKEDEV: bad dc unit in: $fullname
			umask 77
			exit 1
		;;
		esac
		umask 77
	esac
;;
pr)
	case $tryunit in
	0)
		echo "pr0 \c "
		umask 33
		mknod pr0 c 22 0
	;;
	*)
		echo MAKEDEV: Support is only provided for pr unit 0: $fullname
		umask 77
		exit 1
	;;
	esac
;;
sec)
        for i in 0 1 2 3
        do
                test -c ./spd$i ||
                {
                        echo "spd"$i" \c"
                        mknod spd$i c 18 $i
                }
        done
        test -c ./spdcontrol ||
        {
                echo "spdcontrol \c"
                mknod spdcontrol c 18 255
        }
#       chmod 666 spd*
#       chown tcb spd*
#       chgrp tcb spd*
;;

local)
	echo MAKEDEV: attempting to execute /dev/MAKEDEV.local:
	/sbin/sh /dev/MAKEDEV.local $fullname
;;
*)
	echo MAKEDEV: unknown device in: $fullname
	exit 1
;;
esac
if test "X$silent" = "X" ; then
echo ""
fi
done
exit
