$ 
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$
$ HISTORY
$ 
$ @(#)$RCSfile: make.msg,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/09/02 17:23:11 $
$ 
$ /*
$  * X/OPEN message catalogue
$  */

$quote "

$set MS_MAKE

NOCLSPAREN	"No closing parenthesis in archive specification\n"
REMOVED		"*** %s removed\n"
EMPTYSTR	"%s expands to empty string"
FORKFAIL	"Could not fork"
NOTFOUND	": not found\n"
ERRCODE		"*** Error code %d"
SIGNAL		"*** Signal %d"
CONTINUE	" (continuing)\n"
IGNORE		" (ignored)\n"
WAITERR		"error in wait: %d"
STOP		"\n\nStop.\n"
ERRGRAPH	"Graph cycles through %s\n"
UPTODATE	"`%s' is up to date.\n"
NOTREMAKE	"`%s' not remade because of errors.\n"
BADCALL		"bad call to Hash_DeleteEntry\n"
TOUCH		"touch %s\n"
TOUCHERR	"*** couldn't touch %s: %s"
MCONT		"Can't figure out how to make %s (continuing)\n"
MSTOP		"Can't figure out how to make %s. Stop"
ILLARG		"illegal (null) argument."
NODEFAULT	"make: no default rules (%s)."
MAKEOPENERR	"make: cannot open %s."
MAKE1		"make: "
MAKE2		"make: %s.\n"
USAGE		"usage: make [-einpqrst] [-f makefile ] [-k | -S] [macros=name]...  [target_name]... \n"
LINENUM		"\"%s\", line %d: "
WARNING		"warning: "
IOPERATOR	"Inconsistent operator for %s"
ARERR		"Error in archive specification: \"%s\""
NEEDOPERATOR	"Need an operator"
NOTPOSIX	".POSIX directive is not first non-comment line"
IGNORETARGET	"Extra target ignored"
MIXTARGET	"Special and mundane targets don't mix. Mundane ones ignored"
MISSDEPOP	"Missing dependency operator"
SRCARERR	"Error in source archive spec \"%s\""
OPENERR		"Cannot open %s"
SHCOMMAND	"Unassociated shell command \"%.20s\""
NEEDTAB		"Shell command needs a leading tab"
MISSOPERATOR	"Missing operator"
FATALERR	"Fatal errors encountered -- cannot continue\n"
NOTARGET	"No target to make.\n"
VARRECURSIVE	"Variable %s is recursive."
NOEQUALSIGN	"Equal sign not found in macro substitution\n"
VARSPECERR	"Unclosed variable specification for %s"
UNKNOWNTARG     "\"%s\" -- target unknown."

$
$ Debug messages
$
$set MS_DEBUG

MAKE001	"\n# Finding implied suffix dependencies and building the graph for \"%s\".....\n"
MAKE002	"\n# All dependencies for %s are now determined , now run creation commands....\n"
MAKE003	"\n# Running creation commands on all children of \"%s\".....\n"
MAKE004	"\n# Examining implied Parents.....\n"
MAKE005	"\n Implied Parents for \"%s\" were found.\n"
MAKE006	"\n# Examining modification time of \"%s\".\n"
MAKE007	"\n# With respect of its newest child, modification time of %s\n was considered up-to-date.\n"
MAKE008	"\n# \"%s\" must be remade!\n"
MAKE009	"update time: %s\n"
MAKE010	"\t\"%s\" was modified %s .\n"
MAKE011	"\t\"%s\" is considered \"non-existent\".\n"
MAKE012	"\t\"%s\" was modified before sources.\n",
MAKE013	"\t\"%s\" is non-existent and has no sources.\n"
MAKE014	"\t \"::\" operator and no sources.\n"

DIR001	"Searching for file: %s/%s .... "
DIR002	"Found file: %s/%s\n"
DIR003	"couldn't open search path, file not found\n"
DIR004	"Found file in hash table: %s/%s\n"
DIR005	"component mismatch -- continuing..."
DIR006	"returning %s\n"
DIR007	"must be here but isn't -- returing NULL\n"
DIR008	"failed.\n"
DIR009	"failed. Trying subdirectories..."
DIR010	"checking %s..."
DIR011	"got it.\n"
DIR012	"Caching %s for %s\n"
DIR013	"Search failed. "
DIR014	"Checked . already, returning NULL\n"
DIR015	"Looking for \"%s\"..."
DIR016	"got it (in mtime cache)\n"
DIR017	"Caching %s for %s\n"
DIR018	"failed. Returning NULL\n"
DIR019	"Using cached time %s for %s\n"
DIR020	"CASHING %s..."
DIR021	"done\n"

GRAPH01	"\n#**************************************************************\n"
GRAPH02	"#***** The information below contains macro and target ********\n"
GRAPH03 "#***** definitions after the default rules were read, *********\n"
GRAPH04	"#***** and before make processing has occured. ****************\n"
GRAPH05	"#**************************************************************\n\n"
GRAPH06	"#***** definitions after all makefiles were read, and *********\n"
GRAPH07	"#***** after all make processing has occured. *****************\n"

SUFF001	"inserting %s(%d)..."
SUFF002	"at end of list\n"
SUFF003	"before %s(%d)\n"
SUFF004	"already there\n"
SUFF005	"defining transformation from `%s' to `%s'\n"
SUFF006	"deleting transformation from %s to %s\n"
SUFF007	"transformation %s complete\n"
SUFF008	"Found the SCCS file.\n"
SUFF009	"Found the file.\n"

SUFF010	"not there.\n"
SUFF011	"using null suffix\n"
SUFF012	"Local variables for which this node is a member of archive: \n"
SUFF013	"Local variables for which this node is a archive of archive: \n"
SUFF014	"\Local variables for which this node is a target:\n"
SUFF015	"\tNo transformation from %s -> %s\n"
SUFF016	"# No known suffix on %s. Using .NULL suffix\n"
SUFF017	"\Local variable for which this node is a target:\n"
SUFF018	"\Local variables for which this node is a prefix:\n"
SUFF019	"\tNo valid suffix on %s\n"
SUFF020	"#\tTo: "
SUFF021	"#\tFrom: "
SUFF022	"\n#* No known Suffixes.\n\n"
SUFF023	"#*** Printing all known Suffix Transformation Rules:\n"
SUFF024	"\n#* No known Suffix Transformation Rules.\n\n"

TARG001	"#\t\tTarget Name = \"%s\".\n"
TARG002	"#*** Printing Main Target \"%s\" Information:\n"
TARG003	"#*** Printing Target \"%s\" Information:\n"
TARG004	"#\tTarget \"%s\" had \"unmade\" Prerequisites.\n"
TARG005	"#\tTarget \"%s\" had no \"unmade\" Prerequisites.\n"
TARG006	"#\tTarget \"%s\" was last modified on: %s.\n"
TARG007	"#\tTarget \"%s\" status was considered \"%s\".\n"
TARG008	"#\tTarget \"%s\" had no modification time.\n"
TARG009	"#\tTarget \"%s\" could be non-existent: Its status was considered \"%s\".\n"
TARG010	"#\tTarget \"%s\" was not modified and not \"made\".\n"
TARG011	"#\tTarget \"%s\" was an \"Implied\" prerequisite of the following target(s):\n "
TARG012	"#\tTarget \"%s\" was not an \"Implied\" prerequisite of any other target.\n"
TARG013	"#\tTarget \"%s\" was a prerequisite of the following target(s):\n"
TARG014	"#\tTarget \"%s\" was a not prerequisite of any other target.\n"
TARG015	"#\tTarget \"%s\" had the following Operator.   "
TARG016	"#\tTarget \"%s\" had the following Prerequisite(s): \n"
TARG017	"#\tTarget \"%s\" had no known Prerequisites.\n"
TARG018	"#\tTarget \"%s\" had the following Command(s):\n"
TARG019	"#\tTarget \"%s\ had no known Commands.\n"
TARG020	"#\tTarget \"%s\" had the following Cohort(s):\n"
TARG021	"#\t\t\"%s\" is only a Prerequisite.\n"
TARG022	"#*** Printing all known Target(s) Information:\n"	
TARG023	"\n#* No known Target(s).\n\n"
TARG024	"#*** Printing  all Prerequisites that are not targets:\n"
TARG025	"#*** Printing all known Global Variables:\n"
TARG026	"#*** Printing all known Command-line Variables:\n"
TARG027	"#*** Printing all known Suffix Information:\n"

VAR001	"Result is \"%s\""
