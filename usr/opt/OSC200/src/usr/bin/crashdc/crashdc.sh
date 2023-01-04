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
# @(#)$RCSfile: crashdc.sh,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/07/28 13:45:38 $

PROG=`basename $0`
trap "" 1
trap 'echo;echo "ERROR: $PROG terminated prematurely";exit 1' 2 3 15
trap 'rm -f $TMP 2> /dev/null' 0

AWK=/bin/awk
BC=/bin/bc
CAT=/bin/cat
CUT=/bin/cut
DATE=/bin/date
DBX=/bin/dbx
KDBX=/bin/kdbx
EXPR=/bin/expr
GREP=/bin/grep
HEAD=/bin/head
OD=/bin/od
PWD=/bin/pwd
RM=/bin/rm
SED=/bin/sed
STRINGS=/bin/strings
TAIL=/bin/tail

EXIT=0
for UTIL in $STRINGS $OD $HEAD $AWK $DBX $SED $GREP $DATE $RM $CAT $PWD $EXPR 
do
  [ ! -f $UTIL -a ! -x $UTIL ] && {
    echo "Error: $PROG - Utility $UTIL nonexistent or not executable"
    EXIT=1
  }
done
[ $EXIT -eq 1 ] && exit 1
case $# in
	0)	UNIX=/vmunix;CORE=/dev/mem;;
	1)	UNIX=$1;CORE=/dev/mem;;
	2)	UNIX=$1;CORE=$2;;
	*)	echo "Error: Usage: $PROG [object-file] [core-file]"
		exit 1;;
esac
([ ! -r $UNIX ] || [ ! -r $CORE ]) && {
        [ ! -r $UNIX ] &&
                echo "Error: $PROG - Cannot read $UNIX"
        [ ! -r $CORE ] &&
                echo "Error: $PROG - Cannot read $CORE"
        exit 1
}

MAGIC=`$OD -o $UNIX | $HEAD -1 | $AWK '{print $2}'`
case $MAGIC in
        000410) DUMPTYPE=vax;;
	000603) DUMPTYPE=alpha;;
        000542) DUMPTYPE=mips;;
        *)      DUMPTYPE=unknown;;
esac
[ "$DUMPTYPE" = "unknown" -o "$DUMPTYPE" = "vax" ] && {
  echo "Error: Cannot Analyze $DUMPTYPE architecture crash"
  exit 1
}

SELFV=`$STRINGS /vmunix | $GREP '(Rev'`
CRASHV=`$STRINGS $UNIX | $GREP '(Rev'`

SELFOP=`$STRINGS /vmunix | $GREP '(Rev' | $CUT -f1,2 -d' '`
CRASHOP=`$STRINGS $UNIX | $GREP '(Rev' | $CUT -f1,2 -d' '`

TMP=/tmp/cantmp.$$
echo "#
# CANASTA Data Collection (Version 1.3)
#"
echo "_crash_data_collection_time: `$DATE`"
echo "_current_directory: `$PWD`"
echo "_crash_kernel: $UNIX"
echo "_crash_core: $CORE"
echo "_crash_arch: $DUMPTYPE"
echo "_crash_os: $CRASHOP"
echo "_host_version: $SELFV"
echo "_crash_version: $CRASHV"

echo 'set $page=0
     print "_crashtime: ", time
     print "_boottime: ", boottime
     print "_config: ", utsname
     print "_cpu: ", cpu
     print "_system_string: ", cpup.system_string
     print "_num_cpus: ", numcpus
     print "_partial_dump: ", partial_dump
     print "_physmem(MBytes): ", (page_size*physmem)//(1024*1024)
     print "_panic_string: ", panicstr
     print "_stack_trace_begin:";where;print "_stack_trace_end:"
     print "_preserved_message_buffer_begin:";px *pmsgbuf ; print "_preserved_message_buffer_end:"
     print "_kernel_process_status_begin:"; kps; print "_kernel_process_status_end:"
     print "_current_pid: ", $pid
     set $hexints=1
     print "_current_tid: ", $tid
     set $hexints=0
     print "_proc_thread_list_begin:";tlist;print "_proc_thread_list_end:"
     print "_dump_begin:"; dump .; print "_dump_end:"
     set $pid=0
     print "_kernel_thread_list_begin:";tlist;print "_kernel_thread_list_end:"
     print "_savedefp: ", savedefp
     print "_kernel_memory_fault_data_begin: ";px kernel_memory_fault_data
     print "_kernel_memory_fault_data_end: "
     quit' | $DBX -k $UNIX $CORE 2>&1 | $AWK '
	/_savedefp:/	{if ($2 == "(nil)")
			    printf "_savedefp= nil",$1 > "'$TMP'"
                          else
			    printf "_savedefp= %s",$2 > "'$TMP'"
                        }
	/_crashtime:/  	{print;getline; print "CTIME= " $3 > "'$TMP'"}
	/_boottime:/   	{print;getline; print "BTIME= " $3 > "'$TMP'"}
		        {print}' 
eval `$AWK '{print $1 $2}' $TMP`
UPTIME=`echo "scale=2; $CTIME-$BTIME; ./60 ./60" | $BC | $TAIL -1`
echo "_uptime: $UPTIME hours"
echo
#
[ $_savedefp != "nil" ] && {
    $DBX -k $UNIX $CORE << ! 2>&1 
     	set \$page=0
     	set \$hexints=1
     	print "_savedefp_exception_frame_(savedefp/33X):"; savedefp/33X
     	print "_savedefp_exception_frame_ptr: ",savedefp
     	print "_savedefp_stack_pointer: ", savedefp[26]
     	print "_savedefp_processor_status: ", savedefp[27]
     	print "_savedefp_return_address: ", savedefp[23]
     	print "_savedefp_pc: ", savedefp[28]
     	print "_savedefp_pc/i: ";savedefp[28]/i
     	print "_savedefp_return_address/i: ";savedefp[23]/i 
	print "_kernel_memory_fault_data.fault_pc/i: ";kernel_memory_fault_data.fault_pc/i
	print "_kernel_memory_fault_data.fault_ra/i: ";kernel_memory_fault_data.fault_ra/i
     	quit 
!
}
echo "#
# Kdbx Output (swap,sum)
#"
$KDBX -dbx $DBX -k $UNIX $CORE << ! 2>&1
  print "_kdbx_sum: ";sum
  print "_kdbx_swap: ";swap
  print "_kdbx_proc: ";proc
  quit
!
echo "#
_crash_data_collection_finished:"
exit 
