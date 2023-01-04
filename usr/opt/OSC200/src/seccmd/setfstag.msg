$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0
$quote "
$set MS_SETFSTAG

SETFSTAG_1	"%s: need sysadmin authorization\n"
SETFSTAG_2	"%s: index value out of range (%d:%d)\n"
SETFSTAG_3	"%s: -i must precede -t\n"
SETFSTAG_4	"%s: multiple values for tag #%d\n"
SETFSTAG_5	"usage:  %s [-v] [-a] {-i# -t#} ... device\n"
SETFSTAG_6	"Where -v is verbose\n"
SETFSTAG_7	"      -a updates all inodes (default updates 0-value tags only)\n"
SETFSTAG_8	"      -i notes tag index to use (from 0)\n"
SETFSTAG_9	"      -t is tag value to set\n"
SETFSTAG_10	"      device is the file system to update\n"
SETFSTAG_11	"Cannot open %s for reading and writing\n"
SETFSTAG_12	"Cannot read super block from %s\n"
SETFSTAG_13	" Last mounted on: %s\n"
SETFSTAG_14	" File System: %.6s Volume %.6s\n\n"
SETFSTAG_15	" ** Validate File System\n"
SETFSTAG_16	" ** Invalidate Superblock\n"
SETFSTAG_17	" ** Set Tag on Ilist\n"
SETFSTAG_18	" ** Restore Superblock State\n"
SETFSTAG_19	"Cannot rewrite super block\n"
SETFSTAG_20	"Cannot stat file system\n"
SETFSTAG_21	"Block or character special file required\n"
SETFSTAG_22	"File system is not clean -- use fsck first\n"
SETFSTAG_23	"File system not built to accommodate security tags\n"
SETFSTAG_24	"File system is mounted -- umount first\n"
SETFSTAG_25	"File system is not OKAY -- use fsck first\n"
SETFSTAG_26	"File does not appear to be a file system\n"
SETFSTAG_27	"Can't allocate space for disk block buffer\n"
SETFSTAG_28	"\tThere are %d inodes.\n"
SETFSTAG_29	"\tUpdating inode # "
SETFSTAG_30	"Read error on block %d, aborting\n"
SETFSTAG_31	"Write error on block %d, aborting\n"
SETFSTAG_32	"%s: mode contains non-digit '%c'\n"
SETFSTAG_33	"%s: insufficient privileges\n"
