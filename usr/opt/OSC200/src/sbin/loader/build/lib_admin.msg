$ 
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$ 
$ 
$ OSF/1 Release 1.0

$ Message file for lib_admin command
$
$quote "         define quote character.
$len
$set MS_LIB_ADMIN        Define initial set#

$ messages for command lib_admin
$   GLOBAL COMMENT FOR TRANSLATORS
$       Do not translate lib_admin -- this is the command name.
$       Do not translate %o %s %1$s %2$s etc.
$	%s is generally a file name
ERR_BOOT	"ldr_bootstrap failed"
ERR_USAGE       "usage: %s [-i] [-p] [-o <output file>] [-v] <database file>\n"
$
$ Do not translate -i, -p, etc.
$
ERR_AFOPEN	"opening database file"
ERR_MALLOC	"malloc error"
ERR_BAD_VALUE   "bad value %s for attribute %s, lib %s\n"
ERR_ATTR_UNKNOWN "unknown attribute %s in library %s\n"
ERR_DB_SYNTAX   "syntax error in database file\n"

$
$ The first %s here is an attribute name
$
ERR_REMFILE	"unable to unmap old loader global file"
ERR_CONTEXT	"unable to create temporary context"
ERR_CTXBOOT	"unable to bootstrap temporary context"
ERR_INSTALL	"error installing library"
ERR_INSTALLLIB	"installing library %s\n"
ERR_PRELOAD	"error preloading library"
ERR_PRELOADLIB	"preloading library %s\n"
ERR_OUTFILE	"error creating output file"
ERR_FILEINIT	"error initializing global file"
ERR_RENAME	"unable to rename temp to global file"
$
$ Informative messages
$
MSG_INSTALL	"installed library %s\n"
MSG_PRELOAD	"loaded library %s\n"
MSG_DUMPED	"created global data file %s\n"
