#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

###############################################################################
#
#  This file defines all the possible patch variations which can
#  be applied to a kernel at installation time.
#
#  Each line consists of two fields:  the patch tag and the patch
#  definition.  The patch tag is separated from its corresponding
#  definition by any number of space or tab characters.  If a patch
#  will not fit on a single line, additional lines with the same tag
#  may be used.
#
#  The resulting patch file generated for a host by the installation
#  script consists of a host name comment header line, followed by the
#  definition fields (in order) of all lines from this master file
#  whose tag fields match one of the patch names recorded in the
#  "kernel" attribute of the current host.
#
###############################################################################
#
#  Don't disable auto-nice 
#
auton	umodes?W ((@umodes)&(~0x4))
#
large	nproc?W 0t420
large	nfile?W 0t809
large	ninode?W 0t1036
large	ncallout?W 0t436
large	nclist?W 0t660
#
medium	nproc?W 0t276
medium	nfile?W 0t550
medium	ninode?W 0t712
medium	ncallout?W 0t292
medium	nclist?W 0t444
#
mup40	maxuprc?W 0t40
mup128	maxuprc?W 0t128
#
page8k	page_size?W 0x2000
#
vr100	qvmodel?W 0t100
#
vmlim	vm_initial_limit_data+4?W 0t128*0t1024*0t1024
#
vmlim192	vm_initial_limit_data+4?W 0t192*0t1024*0t1024
#
nfile-550	nfile?W 0t550
#
#	Network service machines need a bigger arp-table
#
arptab	arptab_nb?W 7f
