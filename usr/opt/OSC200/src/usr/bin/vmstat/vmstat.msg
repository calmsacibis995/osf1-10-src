$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0
$

$quote "

$set MS_VMSTAT

USAGE 		"usage: vmstat [-fisM] [interval [count]]\n"
EPERM		"missing privilege to run"
ENOMEM		"insufficient memory for process table"
NO_VMSTATS	"cannot get vm statistics info"
NO_SYSINFO	"cannot get system info"
NO_MALLOCBUCKTES "cannot get malloc buckets info"
NO_MALLOCTYPES   "cannot get malloc kmemtypes info"
NO_MALLOCNAMES  "cannot get memory type names"
NO_INTR		"cannot get interrupt info"
NO_NOTIFY	"could not disable notify port"
NO_NOTRECEIVE	"notify thread receive error"
RPT_VMSTATS	"Virtual Memory Statistics: (pagesize = %d)\n"
RPT_FORKS	"forks"
RPT_VFORKS	"vforks"
RPT_ACTIVE	"active pages"
RPT_INACTIVE	"inactive pages"
RPT_FREEPAGES	"free pages"
RPT_WIRED	"wired pages"
RPT_FAULTS	"virtual memory page faults"
RPT_COW		"copy-on-write page faults"
RPT_ZEROFILL	"zero fill page faults"
RPT_REACTIVE	"reattaches from reclaim list"
RPT_PAGEINS	"pages paged in"
RPT_PAGEOUTS	"pages paged out"
RPT_LOOKUPS	"number of lookups"
RPT_HITS	"number of hits"
RPT_CACHE	"hit ratio percentage"
RPT_CONTEXT	"task and thread context switches"
RPT_DEVINTR	"device interrupts"
RPT_SYSCALL	"system calls"
RPT_MEMUSE1	"\nMemory usage by bucket:\n\n"
RPT_MEMUSE2	"bucket#  element_size  elements_in_use  elements_free  bytes_in_use\n"
RPT_MEMUSE3	"\nTotal memory being used from buckets =%d bytes\n"
RPT_MEMUSE4	"Total free memory in buckets =%d bytes\n\n\n"
RPT_MEMUSE5	"Memory usage by type: Type and Number of bytes being used\n\n"
