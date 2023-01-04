#!/usr/bin/sh
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
#	@(#)install.sh	3.3	(ULTRIX/OSF)	10/10/91
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
# 
# 
# OSF/1 Release 1.0


# 
# COMPONENT_NAME: (CMDFILES) commands that manipulate files
# 
# FUNCTIONS: install
# 
# ORIGINS: 3, 27
# 
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# install.sh	1.4  com/cmd/files,3.1,9021 1/18/90 15:18:28

# NAME: usage
#
# FUNCTION: displays the usage statement
#
#
# RETURNS: 0 if successful; 2 if unsuccessful.
#
#
# 09/13/91 - Tom Peterson
#	- changed symbolic names to numbers according to OSF/1 1.0.2
#	- also changed install.msg accordingly
#
# 09/16/91 - zanne
#	- fix local arg flag settings to fix -m option with -c from
#	- qar 01103 also found -u and -g with potential problems.
#
#
usage()
{
	if msg=`dspmsg -s 1 install.cat 1 "Usage: install [-c dira] [-f dirb] [-i] [-M] [-m mode] [-u owner] [-g group] [-S] [-n dirc] [-o] [-s] file [dirx ...]"`
	then :
	else msg="Usage: install [-c dira] [-f dirb] [-i] [-M] [-m mode] [-u owner] [-g group] [-S] [-n dirc] [-o] [-s] file [dirx ...]"
  	fi 
	$ECHO $msg
	exit 2
}

#
# NAME: install (shell script)
#
# FUNCTION: installs a command
#
# EXECUTION ENVIRONMENT: Shell 
#
# NOTES:
# 	Possible flags:
#	-c dira		installs a new command file in dira only if that
#			file does not already exist there.
#	-f dirb		installs a command file in dirb whether or not file   
#			already exists.
#	-i		ignores the default directory list and searches only
#			thoes directories specified on the command line.
# 	-M 		moves a file instead of being copied.
#	-n dirc		installs file in dirx if it is not in any of the     
#			searched directories.
#	-m mode		specifies the mode of the destination file
#	-u mode		specifies the owner of the destination file
#	-g mode		specifies the group of the destination file
#	-o		saves the old copy of file.
#	-s		displays error messages only.
#	-S		strip's the binary after installation.
#
# RETURNS: 0 if it is successful; 2 if it is unsuccessful.
#
 
#
# initializes some shell variables
#
FLIST=/etc/syslist
DEFAULT="/bin /usr/bin /etc /lib /usr/lib" 
FOUND="" MOVOLD="" MVFILE=""
ECHO=echo FLAG=off MODE=755 OWNER=bin GROUP=bin
MFLAG="" UFLAG="" GFLAG="" STRIP=""
MVERR=1 CPERR=1
dspmsg="/usr/bin/dspmsg"
#
# load messages from the message catalog
#
if INSTALLC=`dspmsg -s 1 install.cat 3 "install: -c dir: illegal option with"`
then :
else INSTALLC="install: -c dir: illegal option with"
fi
if OPTION=`dspmsg -s 1 install.cat 4 "flag."`
then :
else OPTION="option!"
fi
if INSTALLF=`dspmsg -s 1 install.cat 6 "install: -f dir: illegal option with"`
then :
else INSTALLF="install: -f dir: illegal option with"
fi
if INSTALLI=`dspmsg -s 1 install.cat 8 "install: -i: illegal option with"`
then :
else INSTALLI="install: -i: illegal option with"
fi
if INSTALLM=`dspmsg -s 1 install.cat INVAL_M "install: -M: illegal option with"`
then :
else INSTALLM="install: -M: illegal option with"
fi
if INSTALLN=`dspmsg -s 1 install.cat 11 "install: -n dir: illegal option with"`
then :
else INSTALLN="install: -n dir: illegal option with"
fi
if INSTALLO=`dspmsg -s 1 install.cat 13 "install: -o: illegal option with"`
then :
else INSTALLO="install: -o: illegal option with"
fi
if INSTALL=`dspmsg -s 1 install.cat 14 "install:"`
then :
else INSTALL="install:"
fi
if EXIST=`dspmsg -s 1 install.cat 15 "already exists in"`
then :
else EXIST="already exists in"
fi
if MOVE=`dspmsg -s 1 install.cat 16 "moved to"`
then :
else MOVE="moved to"
fi
if OLD=`dspmsg -s 1 install.cat 17 "/OLD"`
then :
else OLD="/OLD"
fi
if INSTALLCP=`dspmsg -s 1 install.cat  18 "install: cp"`
then :
else INSTALLCP="install: cp"
fi
if SLASH=`dspmsg -s 1 install.cat 19 "/"`
then :
else SLASH="/"
fi
if FAIL=`dspmsg -s 1 install.cat 20 "failed"`
then :
else FAIL="failed"
fi
if INSTALLAS=`dspmsg -s 1 install.cat 21 "installed as"`
then :
else INSTALLAS="installed as"
fi
if CHMOD=`dspmsg -s 1 install.cat 22 "chmod"`
then :
else CHMOD="chmod"
fi
if CHGRP=`dspmsg -s 1 install.cat 23 "chgrp"`
then :
else CHGRP="chgrp"
fi
if CHOWN=`dspmsg -s 1 install.cat 24 "chown"`
then :
else CHOWN="chown"
fi
if STRIP=`dspmsg -s 1 install.cat 25 "strip"`
then :
else STRIP="strip"
fi
if LOLD=`dspmsg -s 1 install.cat 26 "old"`
then :
else LOLD="old"
fi
if NOTFOUND=`dspmsg -s 1 install.cat 27 "was not found anywhere!"`
then :
else NOTFOUND="was not found anywhere!"
fi
if BYDEFAULT=`dspmsg -s 1 install.cat 28 "by default!"`
then :
else BYDEFAULT="by default!"
fi

#
# parses the arguments
#
for i in $*
do
	if [ $FLAG = on ]
	then
		case $i in
		    -*) 
			if msg=`dspmsg -s 1 install.cat 2 "install: The -c, -f, -n options each require a directory following!\n\t The -m, -u, -g options require mode, owner or group following!"`
			then :
			else msg="install: The -c, -f, -n options each require a directory following!\n\t The -m, -u, -g options require mode, owner or group following!"
			fi
			$ECHO $msg
			exit 2;;
		     *) FLAG=off
			continue;;
		esac
	fi
	case $i in
	    -c) if [ x$ARG = x-f -o x$arg = x-i -o x$arg = x-o -o x$arg = x-n -o x$arg = x-M ]
		then
		 	$ECHO "$INSTALLC ${arg-"-f"} $OPTION" 
			exit 2
		elif test $# -lt 3
		then
			if msg=`dspmsg -s 1 install.cat 5 "install: -c option requires at least 3 args!"`
			then :
			else msg="install: -c option requires at least 3 args!" 
			fi
			$ECHO $msg
			exit 2
		else
			direct=$2
			FLAG=on
			ARG=-c
			flag=-c
			shift; shift
		fi;;
	    -f) if [ x$ARG = x-c -o x$arg = x-i -o x$arg = x-n -o x$arg = x-M ]
		then
			$ECHO "$INSTALLF ${arg-"-c"} $OPTION"
			exit 2
		elif test $# -lt 3 
		then
			if msg=`dspmsg -s 1 install.cat 7 "install: -f option requires at least 3 args!"`
			then :
			else msg="install: -f option requires at least 3 args!"
			fi
			$ECHO $msg
			exit 2
		else
			direct=$2
			FLAG=on
			ARG=-f
			flag=-f
			shift; shift
		fi;;
	  -i) if [ x$ARG  = x-c -o x$ARG = x-f -o x$arg = x-M ]
		then
	 		$ECHO "$INSTALLI $flag $OPTION"
			exit 2
		elif test $# -lt 3
		then
			if msg=`dspmsg -s 1 install.cat 9 "install: -i option requires at least 3 args!"`
			then :
			else msg="install: -i option requires at least 3 args!"
			fi
			$ECHO $msg
			exit 2
		else
			DEFAULT=""
			arg=-i
			flag=-i
			shift
		fi;;
	  -M) if [ x$ARG  = x-c -o x$ARG = x-f -o x$arg = x-i -o x$arg = x-n ]
		then 
			$ECHO "$INSTALLM $flag $OPTION" 
			exit 2
	        elif test $# -lt 2 
		then 
		 	usage	
		else
			MVFILE=yes
			arg=-M
			flag=-M
			shift
		fi;;
	  -n) if [ x$ARG = x-c -o x$ARG = x-f -o x$arg = x-M ]
		then
			$ECHO "$INSTALLN $flag $OPTION" 
			exit 2
		elif test $# -lt 3
		then
			if msg=`dspmsg -s 1 install.cat  12 "install: -n option requires at least 3 args!"`
			then :
			else msg="install: -n option requires at least 3 args!"
			fi
			$ECHO $msg
			exit 2
		else
			LASTRES=$2
			FLAG=on
			FOUND=n
			arg=-n
			flag=-n
			shift; shift
		fi;;
	  -m) if test $# -lt 3
		then 
		 	usage	
		else
			case $2 in
			 	[0-7][0-7][0-7]) MODE=$2;; 
				[0-7][0-7][0-7][0-7]) MODE=$2;;
				*)
				if msg=`dspmsg -s 1 install.cat 29 "chmod: Use absolute mode to specify the desired permission settings"`
				then :
				else msg="chmod: Use absolute mode to specify the desired permission settings"
				fi
				$ECHO $msg
				exit 2
		 	esac	
			MFLAG=on
			FLAG=on
			arg=-m
			shift; shift
		fi;;
	  -u) if test $# -lt 3
		then 
		 	usage	
		else
			OWNER=$2
			UFLAG=on
			FLAG=on
			arg=-u
			shift; shift
		fi;;
	  -g) if test $# -lt 3
		then 
			usage	
		else
			GROUP=$2
			GFLAG=on
			FLAG=on
			arg=-g
			shift; shift
		fi;;
	  -o) if  [ x$ARG = x-c ]
		then
			$ECHO "$INSTALLO $ARG $OPTION" 
			exit 2
		elif test $# -lt 2
		then
			usage	
		else
			MOVOLD=yes
			arg=-o
			shift
		fi;;
	  -s) if test $# -lt 2
		then
			usage	
		else
			ECHO=:
			arg=-s
			shift
		fi;;
	  -S) if test $# -lt 2
		then 
			usage	
		else
			STRIP=yes
			arg=-S
			shift
		fi;;
	     *) break;;
	esac
done
#
# gets file name and searches the specified directory, ie, dira or dirb
#
FILEP=$i FILE=`echo $i | sed -e "s/.*\///"`

# Save the setuid bit of source file which is a setuid program.
# We used this flag to assert the setuid bit of destination file
# after "cp" or "strip" a file.
if [ -u $FILEP ]
then
        SETUID=1
else
        SETUID=0
fi

if [ x$ARG = x-c -o x$ARG = x-f ]
then
	case $2 in
		-*) usage ;;
		"") :	;;
	esac
	if test -f $direct/$FILE -o -f $direct/$FILE/$FILE
	then
		case $ARG in
			-c) $ECHO "$INSTALL $FILE $EXIST $direct"
			    exit 2;;
			-f) if [ -k $direct/$FILE ]
			    then
				chmod -t $direct/$FILE
				$direct/$FILE < /dev/null > /dev/null
				tbit=on
			    fi
			    if [ "$MOVOLD" = yes ]
			    then
				mv $direct/$FILE $direct/OLD$FILE
				cp $direct/OLD$FILE $direct/$FILE
				if [ $? = 0 ]
				then
				   $ECHO "$INSTALL $FILE $MOVE $direct$OLD$FILE"
				   chgrp $GROUP $direct/$FILE
                                   if [ -u $direct/OLD$FILE ]
                                   then
                                        chmod u+s $direct/$FILE
                                   fi
				else
			 	   $ECHO "$INSTALLCP $direct$OLD$FILE $direct$SLASH$FILE $FAIL"
				   exit 2
				fi
			    fi
			    LS=`ls -l $direct/$FILE`
			    OLDWMODE=`expr "$LS" : '..\(.\).*'`
			    chmod u+w $direct/$FILE
			    if cp $FILEP $direct/$FILE
			    then
				$ECHO "$FILEP $INSTALLAS $direct$SLASH$FILE"
                                # cp nullified setuid bit, set back.
                                if [ "$SETUID" = 1 ]
                                then
                                        if [ "$MFLAG" = on ]
                                        then
                                            chmod $MODE $direct/$FILE
                                        else
                                            chmod u+s $direct/$FILE
                                        fi
                                fi
			    fi
			    if [ "$MFLAG" = on ]
				then
			  	if chmod $MODE $direct/$FILE
				then
					$ECHO "$CHMOD $MODE $direct$SLASH$FILE"
				else
					exit 2
				fi
			    else
				if [ $OLDWMODE = - ]
			    	then
					chmod u-w $direct/$FILE
			    	fi
			        if [ "$tbit" = on ]
			        then
					chmod +t $direct/$FILE
			        fi
			    fi
			    if [ "$GFLAG" = on ]
			    then
				if chgrp $GROUP $direct/$FILE
				then
					$ECHO "$CHGRP $GROUP $direct$SLASH$FILE"
                                        # chgrp nullified setuid bit, set back.
                                        if [ "$SETUID" = 1 ]
                                        then
                                                if [ "$MFLAG" = on ]
                                                then
                                                    chmod $MODE $direct/$FILE
                                                else
                                                    chmod u+s $direct/$FILE
                                                fi
                                        fi
				else
					exit 2
				fi
			    fi
			    if [ "$UFLAG" = on ]
			    then
				if chown $OWNER $direct/$FILE
				then
					$ECHO "$CHOWN $OWNER $direct$SLASH$FILE"
                                        # chown nullified setuid bit, set back.
                                        if [ "$SETUID" = 1 ]
                                        then
                                                if [ "$MFLAG" = on ]
                                                then
                                                    chmod $MODE $direct/$FILE
                                                else
                                                    chmod u+s $direct/$FILE
                                                fi
                                        fi
				else 
					exit 2
				fi
			    fi
			    if [ "$STRIP" = yes ]
			    then
				if /bin/strip $direct/$FILE 
				then
					$ECHO "$STRIP $direct$SLASH$FILE"
                                        # strip nullified setuid bit, set back.
                                        if [ "$SETUID" = 1 ]
                                        then
                                                if [ "$MFLAG" = on ]
                                                then
                                                    chmod $MODE $direct/$FILE
                                                else
                                                    chmod u+s $direct/$FILE
                                                fi
                                        fi
				else
					exit 2
				fi
			    fi
			    exit;;
		esac
	else
# file does not exist
		cp $FILEP $direct/$FILE
		if [ $? = 0 ]
		then
			$ECHO "$FILEP $INSTALLAS $direct$SLASH$FILE"
                        # cp nullified setuid bit, set it back.
                        if [ "$SETUID" = 1 ]
                        then
                                if [ "$MFLAG" = on ]
                                then
                                    chmod $MODE $direct/$FILE
                                else
                                    chmod u+s $direct/$FILE
                                fi
                        fi
# if -m , -u or -g flag is not on, file will get the default values
			chmod $MODE $direct/$FILE       
			if [ $? != 0 ] 
			then exit 2
			fi
			chgrp $GROUP $direct/$FILE
			if [ $? != 0 ]
			then 
                                exit 2
                        else
                                # chgrp nullified setuid bit, set it back.
                                if [ "$SETUID" = 1 ]
                                then
                                        if [ "$MFLAG" = on ]
                                        then
                                                chmod $MODE $direct/$FILE
                                        else
                                                chmod u+s $direct/$FILE
                                        fi
                                fi
			fi
 			chown $OWNER $direct/$FILE
			if [ $? != 0 ]
			then 
                                exit 2
                        else
                                # chown nullified setuid bit, set it back.
                                if [ "$SETUID" = 1 ]
                                then
                                        if [ "$MFLAG" = on ]
                                        then
                                                chmod $MODE $direct/$FILE
                                        else
                                                chmod u+s $direct/$FILE
                                        fi
                                fi
			fi
			if [ "$MFLAG" = on ]
			then
				$ECHO "$CHMOD $MODE $direct$SLASH$FILE"
			fi
			if [ "$GFLAG" = on ]
			then
				$ECHO "$CHGRP $GROUP $direct$SLASH$FILE"
                                # chgrp nullified setuid bit, set it back.
                                if [ "$SETUID" = 1 ]
                                then
                                        if [ "$MFLAG" = on ]
                                        then
                                                chmod $MODE $direct/$FILE
                                        else
                                                chmod u+s $direct/$FILE
                                        fi
                                fi
			fi
			if [ "$UFLAG" = on ]
			then
				$ECHO "$CHOWN $OWNER $direct$SLASH$FILE"
                                # chown nullified setuid bit, set it back.
                                if [ "$SETUID" = 1 ]
                                then
                                        if [ "$MFLAG" = on ]
                                        then
                                                chmod $MODE $direct/$FILE
                                        else
                                                chmod u+s $direct/$FILE
                                        fi
                                fi
			fi
			if [ "$STRIP" = yes ]
			then 
				if /bin/strip $direct/$FILE
				then 
					$ECHO "$STRIP $direct$SLASH$FILE"
                                        # strip nullified setuid bit, set it
                                        if [ "$SETUID" = 1 ]
                                        then
                                                if [ "$MFLAG" = on ]
                                                then
                                                    chmod $MODE $direct/$FILE
                                                else
                                                    chmod u+s $direct/$FILE
                                                fi
                                        fi
				else
					exit 2
 				fi
			fi
		fi
	fi
	exit
# done with the command if the file is installed by -c or -f 
fi

shift

#
# starts searching the dirx directories or the default directories
#
PUTHERE=""
for i in $*
do
	case $i in
		-*) usage ;;
	esac
	PUTHOLD=`find $i -name $FILE -type f -print`
	PUTHERE=`expr "\`echo $PUTHOLD\`" : '\([^ ]*\)'`
	if [ "$PUTHERE" != "" ]
	then break
	fi
done
if [ -r $FLIST -a "$PUTHERE" = "" ]
then
	PUTHERE=`grep "/${FILE}$" $FLIST | sed  -n -e '1p'`
fi
if [ "$PUTHERE" = "" ]
then
	for i in $DEFAULT
	do
		PUTHOLD=`find $i -name $FILE -type f -print`
		PUTHERE=`expr "\`echo $PUTHOLD\`" : '\([^ ]*\)'`
		if [ "$PUTHERE" != "" ]
		then break
		fi
	done
fi
if [ "$PUTHERE" != "" ]
then
		    if [ -k $PUTHERE ]
		    then
			chmod -t $PUTHERE
			$PUTHERE < /dev/null > /dev/null
			tbit=on
		    fi
		    if [ "$MOVOLD" = yes ]
		    then
			old=`echo $PUTHERE | sed -e "s/\/[^\/]*$//"`
			mv $PUTHERE $old/OLD$FILE
			cp $old/OLD$FILE $PUTHERE
			if [ $? = 0 ]
			then
			  chgrp $GROUP $PUTHERE
			  $ECHO "$LOLD $FILE $MOVE $old$OLD$FILE"
			else
			  $ECHO "$INSTALLCP $direct$OLD$FILE $direct$SLASH$FILE $FAIL"
			  exit 2
			fi
		    fi
		    FOUND=y
		    LS=`ls -l $PUTHERE`
		    OLDWMODE=`expr "$LS" : '..\(.\).*'`
		    chmod u+w $PUTHERE
		    if [ "$MVFILE" = yes ]
		    then 
		    	mv $FILEP $PUTHERE
			MVERR=$?
		    else
			cp $FILEP $PUTHERE
		        CPERR=$?
		    fi
                    # cp setuid bit,set it back
                    if [ "$SETUID" = 1 ]
                    then
                        if [ "$MFLAG" = on ]
                        then
                                chmod $MODE $PUTHERE
                        else
                                chmod u+s $PUTHERE
                        fi
                    fi
		    if [ $CPERR = 0 -o $MVERR = 0 ]
		    then
			$ECHO "$FILEP $INSTALLAS $PUTHERE"
		    else
			exit 2
		    fi
		    if [ "$MFLAG" = on ]
		    then
		  	if chmod $MODE $PUTHERE
			then
				$ECHO "$CHMOD $MODE $PUTHERE"
			else
				exit 2
			fi
		    else 
			if [ $OLDWMODE = - ]
		        then
				chmod u-w $PUTHERE
		    	fi
		        if [ "$tbit" = on ]
		    	then
			     	chmod +t $PUTHERE
		     	fi
                        # cp nullified setuid bit, set it back.
                        # chgrp nullified setuid bit, set it back.
                        if [ "$SETUID" = 1 ]
                        then
                                if [ "$MFLAG" = on ]
                                then
                                        chmod $MODE $PUTHERE
                                else
                                        chmod u+s $PUTHERE
                                fi
                        fi
		    fi
		    if [ "$GFLAG" = on ]
		    then
			if chgrp $GROUP $PUTHERE
			then
				$ECHO "$CHGRP $GROUP $PUTHERE"
                                # chgrp nullified setuid bit, set it back.
                                if [ "$SETUID" = 1 ]
                                then
                                        if [ "$MFLAG" = on ]
                                        then
                                                chmod $MODE $PUTHERE
                                        else
                                                chmod u+s $PUTHERE
                                        fi
                                fi
			else 
  				exit 2
			fi
		    fi
		    if [ "$UFLAG" = on ]
		    then
			if chown $OWNER $PUTHERE
			then 
				$ECHO "$CHOWN $OWNER $PUTHERE"
                                # chown nullified setuid bit, set it back.
                                if [ "$SETUID" = 1 ]
                                then
                                        if [ "$MFLAG" = on ]
                                        then
                                                chmod $MODE $PUTHERE
                                        else
                                                chmod u+s $PUTHERE
                                        fi
                                fi
			else
				exit 2
			fi
		    fi
		    break
fi

case $FOUND in
	"") $ECHO "$INSTALL $FILE $NOTFOUND" 
	    exit 2;;
	 y) :  ;; 
	 n) cp $FILEP $LASTRES/$FILE
	    if [ $? = 0 ]
	    then
                # cp nullified setuit bit, set it back
                if [ "$SETUID" = 1 ]
                then
                        if [ "$MFLAG" = on ]
                        then
                                chmod $MODE $LASTRES/$FILE
                        else
                                chmod u+s $LASTRES/$FILE
                        fi
                fi
		$ECHO "$FILEP $INSTALLAS $LASTRES/$FILE $BYDEFAULT" 
# if -m , -u or -g flag is not on, file will get the default values
		cd $LASTRES
 		chmod $MODE $FILE
		if [ $? != 0 ]
		then exit 2
		fi
		chgrp $GROUP $FILE
		if [ $? != 0 ]
		then 
                        exit 2
                else
                        # chgrp nullified setuit bit, set it back
                        if [ "$SETUID" = 1 ]
                        then
                                if [ "$MFLAG" = on ]
                                then
                                       chmod $MODE $LASTRES/$FILE
                                else
                                       chmod u+s $LASTRES/$FILE
                                fi
                        fi
		fi
		chown $OWNER $FILE
		if [ $? != 0 ]
		then 
                        exit 2
                else
                        # chown nullified setuit bit, set it back
                        if [ "$SETUID" = 1 ]
                        then
                                if [ "$MFLAG" = on ]
                                then
                                       chmod $MODE $LASTRES/$FILE
                                else
                                       chmod u+s $LASTRES/$FILE
                                fi
                        fi
		fi
		if [ "$MFLAG" = on ]
		then
			$ECHO "$CHMOD $MODE $LASTRES/$FILE"
		fi
		if [ "$GFLAG" = on ]
		then
			$ECHO "$CHGRP $GROUP $LASTRES/$FILE"
                        # chgrp nullified setuid bit, set it back.
                        if [ "$SETUID" = 1 ]
                        then
                                if [ "$MFLAG" = on ]
                                then
                                       chmod $MODE $LASTRES/$FILE
                                else
                                       chmod u+s $LASTRES/$FILE
                                fi
                        fi
		fi
		if [ "$UFLAG" = on ]
		then
			$ECHO "$CHOWN $OWNER $LASTRES/$FILE"
                        # chown nullified setuid bit, set it back.
                        if [ "$SETUID" = 1 ]
                        then
                                if [ "$MFLAG" = on ]
                                then
                                       chmod $MODE $LASTRES/$FILE
                                else
                                       chmod u+s $LASTRES/$FILE
                                fi
                        fi
		fi
	    fi;;
esac
