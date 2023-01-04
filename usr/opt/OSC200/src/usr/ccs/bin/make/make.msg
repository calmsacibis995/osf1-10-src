$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$
$quote "
$set MS_MAKE 
$ 
$
$	The following messages pertain to the main.c file
$
RDINT "Reading internal rules.\n"
SUFLIST "No suffix list.\n"
TIME "TIME(%s)=%s"
FLEINC "*** `%s' may be inconsistent\n"
FLERMV "*** `%s' removed\n"
MKENTRY "MAKE ENTRY (%s"
CTIME ") at %s"
MKEXIT "MAKE EXIT (%d) at %s"
OPENDIR "Open directories:\n"
MACROS "Macros:\n"
DONE "  done=%d"
MAINNM "  (MAIN NAME)"
DEPENDSON "\n depends on:"
CMDS "\n commands:"
WORKINGDIR "Error trying to get current working directory, %s\n");
DISDIR "Current working directory for make is %s\n"

$
$ These definitions are in calls to function fatal
$
NODESC  "No description argument after -f flag"
CANTOPEN  "Cannot open %s"
NOFILEDES  "No description file"
NOARGS  "No arguments or description file"
FILEERR  "%s: Description file error"
BADFLAG "Unknown flag argument %c"
$
$	The following messages pertain to the dirs.c file
$
HASHOVERFLOW "Directory hash table overflow"
MAXNOFILE "NOFILE < 20"
NOMEMORY "out of memory"
CANTHAPPEN "Cannot happen in srchdir"
$
$	The following messages pertain to the doname.c file
$
DONAME "doname(%s, %d)\n"
CMDLINES "Too many command lines for `%s'\n"
SUFFIX "Looking for Single suffix rule.\n"
SOURCE  "Looking for source version.\n"
OBJECT  "Looking for object version.\n"
CHECKOUT "CO(%s): %s\n"
RCSRULES "%s has both an RCS file and rules\n"
TARTIGN  "Warning: target `%s' found as `%s' ignored\n"
DONTMAKE "Don't know how to make %s\n"
NOREMAKE "`%s' not remade because of errors\n"
NOTTODATE "`%s' is up to date.\n"
NOFLAG "%stouch %s\n"
TIME1 "TIME1(%s)=%s"
TIME3 "TIME3(%s)=%s"
INFRULE "Building %s using inference rule %s, because it is out of date relatative to %s\n"
SPACES "  "
$
$ These definitions are in calls to function fatal
$
RCSFILRUL "%s has both an RCS file and rules"
$ don't put a return on the end of the line
CANTMKE " Don't know how to make %s"
$ don't put a return on the end of the line


$
$	The following messages pertain to the dosys.c file
$
SYSSIG  "*** Signal %d"
DUMPCORE  " - core dumped"
EXITED  "*** Exit %d"
IGNORE  " (ignored)\n"
$
$ These definitions are in calls to function fatal
$
CANTFORK "Cannot fork"
NOCMD "no command"
MAXARGS "%s: Too many arguments."
CANTLOAD "Cannot load %s"
NOLOADSH  "Couldn't load Shell"
BADWAIT  "bad wait code"
CANTPIPE "Cannot make pipe"
CANTGETFL "Cannot allocate file structure"

$
$	The following messages pertain to the dyndep.c file
$
UNIFY "unify(%s, %s):\n"
EXECSUB "execsub:"
$
$ These definitions are in calls to function fatal
$
BADRULE "%s rule can only use a single ':'"
EXECSUBST "Execution substitution not supported on locks"
UNKNPASS "Unknown pass in dyndep"
DYNLIB "%s: illegal dynamic library name"
BADVAR "Reference to uninstantiated variable"
WHNAMES "`...` dependencies must be whole names"

$
$	The following messages pertain to the files.c file
$
USER "User %s, %s\n"
OLDRCS "%s: old RCS working file\n"
SMKDIR "%smkdir %s\n"
USING "using %s\n"
SCD "%scd %s\n"
MODE  "using mode of %s\n"
$
$ These definitions are in calls to function fatal
$
OLDRCS1 "%s: old RCS working file"
$ this line does not get a newline
NOTSUFFIX "mkobjdir: not suffix"
BOTCH  "mkobjdir: dir botch: %s"
CANTMKDIR  "Couldn't make directory: %s"
NOTADIR  "Not a directory: %s"
MKMOVE  "makemove: %s"
LONGPATH  "plainmake: path too long"
BADENV  "bad environment (movedmake)"
MAXCOMP  "%s: only one component allowed"
MKCHDIR "makechdir: %s"
NODIR  "No such directory: %s"
BADDIR  "bad directory: %s"
$
$	The following messages pertain to the libs.c file
$
LOOKARCH "lookarch(%s)\n"
FOUNDSYM "foundsym(%s)\n"
OPENARCH "openarch(%s)\n"
SKIPPING "skipping(%.*s)\n"
$
$ These definitions are in calls to function fatal
$
NOTARCH  "%s is not an archive"
BADOBJ  "%s is a corrupt object module\n"
SEEKERR  "Seek error on archive file %s"
BADDATE  "Bad date field for %.14s in %s"
BADSZ  "Bad size field for %.14s in archive %s"
READERR  "Read error on archive %s"
FDSYMBOL  "Cannot find symbol %s in archive %s"
OFFTABEL  "Cannot alloc offset table for archive %s"
STRTABLE  "Cannot alloc string table for archive %s"
BADDATEARCH  "Bad date for %.14s, archive %s"
GLOBAL  "Cannot do global variable search in pdp11 or old object file."

$
$	The following messages pertain to the misc.c file
$
CHANGED "Warning: %s changed after being used\n"
MAKESTOP "Make: %s.  Stop.\n"
STOP "\nStop.\n"
LINE "%s, line %d: %s"
TOOLONG "$? list too long\n"
EXITDIR "Current working directory on exit from make is %s\n"
$
$ These definitions are in calls to function fatal
$
TABLEOVER  "Hash table overflow"
RECMACRO  "%s: infinitely recursive macro?"
XTRACHAR  "'%s': extra chars"
NOCANDO  "sorry, no can do '%s'"
BADMOD  "Unrecognized modifier '%s'"
