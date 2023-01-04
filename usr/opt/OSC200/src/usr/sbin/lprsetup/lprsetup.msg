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
$ @(#)$RCSfile: lprsetup.msg,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/10/01 14:09:47 $
$ 
$quote "
$set MS_LPRSETUP
$	Messages used by lprsetup. Individual printers have their help messages
$	defined in the file /etc/lprsetup.dat as the special symbol 'H?'.  That
$	file must be edited too in order to complete any internationalization.
$	
$	Please be careful when editing to preserve the order of the commands
$	List of valid commands (Note: single letter abreviations need to be
$	unique! This means the first letters need be unique) The commands 
$	can only be a max of CMD_LEN (as defined in lprsetup.h) long.
$	The total number of commands and aliases is MAX_CMDS (as located above).
$	COMMANDS_10 NEEDS TO HAVE A SINGLE LETTER AFFIRMATIVE ALIAS FIRST
$	COMMANDS_11 NEEDS TO HAVE A SINGLE LETTER NEGATION ALIAS FIRST
$	The commands aliases need to have a trailing space and \n in order 
$	to parse.

COMMANDS_0	"\nCommand  < add modify delete exit view quit help >: "
COMMANDS_1	"? help HELP \n"
COMMANDS_2	"add ADD \n"
COMMANDS_3	"delete DELETE \n"
COMMANDS_4	"exit EXIT quit QUIT \n"
COMMANDS_5	"list LIST \n"
COMMANDS_6	"modify MODIFY \n"
COMMANDS_7	"print PRINT \n"
COMMANDS_8	"view VIEW \n"
COMMANDS_9	"printer? PRINTER? \n"
COMMANDS_10	"yes YES \n"
COMMANDS_11	"no NO none NONE \n"
COMMANDS_12	"unknown UNKNOWN \n"

$	General messages used by the main program
LPRSETUP_0	"\nusage: %s\n"
LPRSETUP_1	"\n%s: must be superuser!\n\n"
LPRSETUP_2	"\n\n    Digital OSF/1 Printer Setup Program\n\n"
LPRSETUP_3	"\nAdding printer entry, type '?' for help.\n"
LPRSETUP_4	"\nEnter printer name to add [%s] : "
LPRSETUP_5	"\nSorry, printer '%s' already exists.\n"
LPRSETUP_6	"\nInvalid choice.  Type '?' for help.\n"
LPRSETUP_7	"\nError in adding printcap entry, try again.\n"
LPRSETUP_8	"\nError in associating printer files/directories.\n"
LPRSETUP_9	"\nModifying a printer entry, type '?' for help.\n"
LPRSETUP_10	"\nEnter printer name to modify (or view to view printcap file): "
LPRSETUP_11	"\nSorry, printer number '%s' is not in %s.\n"
LPRSETUP_12	"\nDeleting a printer entry, type '?' for help.\n"
LPRSETUP_13	"\nEnter printer name to delete (or view to view printcap file): "
LPRSETUP_14	"\nCannot delete printer %s, entry not found.\n"
LPRSETUP_15	"\nDelete %s, are you sure? [n] "
LPRSETUP_16	"\n%s not deleted.\n"
LPRSETUP_17	"\nAdding comments to printcap file for new printer, type '?' for help.\n"
LPRSETUP_18	"Do you want to add comments to the printcap file [%s] ? : "
LPRSETUP_19	"Enter comments below - Press RETURN on empty line to exit\n\n"
LPRSETUP_20	"\nSet device pathname 'lp'"
LPRSETUP_21	"\nSet accounting file 'af'"
LPRSETUP_22	"\nSet spooler directory 'sd'"
LPRSETUP_23	"\nSet printer error log file 'lf'"
LPRSETUP_24	"\nInternal Error, no \"lp\" entry"
LPRSETUP_25	"\nSet printer connection type 'ct'"
LPRSETUP_26	"\nYou must enter a value for 'ts'"
LPRSETUP_27	"\nSet terminal server name 'ts'"
LPRSETUP_28	"\nYou must enter a value for either 'op' or 'os'\n"
LPRSETUP_29	"\nSet terminal server output port 'op'"
LPRSETUP_30	"\nSet terminal server output service 'os'"
$	 
LPRSETUP_31	"\nThe tty you are currently using is not a LAT configured tty.\n\
You must configure a tty for LAT use, see MAKEDEV(8) and latcp(8)\n\
for more details."
$	
LPRSETUP_32	"\nSet printer baud rate 'br'"
$	
LPRSETUP_33	"\nSet printserver output filter 'of'\n\retype default \
exactly with NODE replaced by printserver node name\n"
$	
LPRSETUP_34	"\nFor a printserver running TCP/IP, use 'iplpscomm' \
instead of 'lpscomm'\n"
$	
LPRSETUP_35	"\nSet remote system name 'rm'"
LPRSETUP_36	"\nSet remote system printer name 'rp'"
LPRSETUP_37	"\nEnter symbol name: "
LPRSETUP_38	"Do you wish to continue with this entry?  [y] "
$	
LPRSETUP_39	"\nFor more information on the specific printer types\n \
Enter `printer?'\n"
$	
LPRSETUP_40	"\nEnter the FULL name of one of the following printer types:\n"
LPRSETUP_41	"\nor press RETURN for [unknown] : "
LPRSETUP_42	"\nUsing 'unknown' for printer type, OK? [ n ] "
LPRSETUP_43	"\nDon't know about printer '%s'\n"
LPRSETUP_44	"\nEnter 'y' to try again, or 'n' to use 'unknown' [y]: "
LPRSETUP_45	"\nmalloc: not enough space for symbols!\n"
LPRSETUP_46	"\nEntering PRINTER TYPE help information MODE\n"
$	
LPRSETUP_47	"\nEnter the printer type for specific information\n \
        Enter '?' for help, 'quit' to exit : "
$	
LPRSETUP_48	"\n\nThe printer type %s is NOT SUPPORTED by DIGITAL - \
No Information\n\n"
$	
LPRSETUP_49	"\nLeaving PRINTER TYPE help information MODE\n"
$	
LPRSETUP_50	"\n\nReturning to Selection of Printer Type for ADDING a New \
Printer\n\n"
$	
LPRSETUP_51	"\nEnter printer synonym: "
LPRSETUP_52	"\nSynonym is already in use, try something else.\n"
LPRSETUP_53	"\nSynonym too long, truncating to %d characters.\n"
LPRSETUP_54	"internal error: cannot find symbol %s in table\n"
LPRSETUP_55	"\nmalloc: no space for %s\n"
LPRSETUP_56	"\nPress 'RETURN' to continue or 'quit RETURN' to quit: "
LPRSETUP_57	"\ngetpwnam: cannot get id for %s\n"
LPRSETUP_58	"\nChanging spool directory\n"
LPRSETUP_59	"Do you want to delete the old spool directory? [n] "
LPRSETUP_60	"\ngetpwnam: cannot find %s uid in passwd file.\n"
$	
LPRSETUP_61	"\nEnter the name of the symbol you wish to change.\nEnter 'p' \
to print the current values, 'l' to list\n all printcap values or 'q' to \
quit.\n"
$	
LPRSETUP_62	"\nEnter symbol name:  "
LPRSETUP_63	"\nCannot rename %s to %s (errno = %d).\n"
LPRSETUP_64	"\nCannot open intermediate file: %s.\n"
LPRSETUP_65	"\nBad type (%d) for %s\n"
LPRSETUP_66	"\nCannot find accounting file entry in table!\nNo accounting \
file created.\n"
LPRSETUP_67	"\nCannot make intermediate accounting directory: %s\n"
LPRSETUP_68	"\nCannot chmod %s to mode 0755\n"
LPRSETUP_69	"\nIntermediate account directory: %s already exists\n \
but it is not a directory!\n"
LPRSETUP_70	"\nCannot create accounting file %s\n"
LPRSETUP_71	"\nCannot chmod %s to mode 0644\n"
LPRSETUP_72	"\nCannot chown %s to (%o/%o)\n"
LPRSETUP_73	"\nFile with same name as accounting file %s already exists\n"
LPRSETUP_74	"\nCannot find error log file entry in table!\nNo error log \
file created.\n"
LPRSETUP_75	"\nCannot make intermediate error log directory: %s\n"
$	
LPRSETUP_76	"\nIntermediate error log directory: %s already exists\n \
but it is not a directory!\n"
$	
LPRSETUP_77	"\nCannot create error log file %s\n"
$	
LPRSETUP_78	"\nWarning: regular file with same name as error log file %s \
already exists.\n"
$	
LPRSETUP_79	"\nWarning: symbolic link with same name as error log file \
%s already exists.\nThis may result in printers sharing an error log file.\n"
$	
LPRSETUP_80	"\nInvalid name for error log file %s\n"
$	
LPRSETUP_81	"File with same name as error log file %s already exists,\nbut \
it is not a regular file or a symbolic link.\n"
$	
LPRSETUP_82	"Status information word S_IFMT = %d\n"
$	
LPRSETUP_83	"\nCannot find spooler directory entry in table!\nNo spooling \
directory created.\n"
$	
LPRSETUP_84	"Warning: queue will use default spool directory \
[/usr/spool/lpd]\n"
$	
LPRSETUP_85	"\nCannot make spooling directory path: %s\n"
$	
LPRSETUP_86	"\nSpooling directory path: %s already exists,\nbut it \
is not a directory!\n"
$	
LPRSETUP_87	"Couldn't unlink spooler file %s\n"
LPRSETUP_88	"Couldn't unlink old spooler directory (%s)\n"
LPRSETUP_89	"Removed spooling directory: %s\n"
LPRSETUP_90	"\nSpooling directory %s does not exist\n"
LPRSETUP_91	"\nCan not unlink spooling directory %s because of errno %d\n"
LPRSETUP_92	"Do you want to delete '%s' file '%s' [n] "
LPRSETUP_93	"Couldn't unlink :%s: file: %s\n"
LPRSETUP_94	"Deleted file: %s\n"
LPRSETUP_95	"\nCannot open %s\n"
LPRSETUP_96	"\nDo you want to capture print job accounting data ([y]|n)? "

$	Messages used by misc.c routines
MISC_0	"\nPlease answer yes or no  [%c] "
MISC_1	"\nEnter a value for '%s'? "
MISC_2	"Sorry, no specific help is available for symbol'%s'\n"
$	The following strings are underlined with '-' chars. Edit carefully.
MISC_3	"\n\tPrinter #%s \n\t----------"
$	MISC_4 is a generic single character ('-') for underlining
MISC_4	"-"
MISC_5A	"\nSymbol "
MISC_5B	"\n------ "
MISC_6A	"used "
MISC_6B	"---- "
MISC_7A	" type  value"
MISC_7B	" ----  -----\n"
$	MISC_8A and MISC_8B must be able to be centered under MISC_6A
MISC_8A	"YES  "
MISC_8B	" NO  "
$	The first group of question marks of MISC_8C is centered under MISC_6A
MISC_8C	" ???    ??????\n"
MISC_9	"\nAre these the final values for printer %s ? [y] "
MISC_10	"\nSymbol name '%s' is too long!\n"
MISC_11	"\nSymbol '%s' not found.  Use the 'list' command for a\nlist of all of the symbols and their defaults.\n"
MISC_12	"\nEnter a new value for symbol '%s'?  [%s] "
MISC_13	"\nNew '%s' is '%s', is this correct?  [y] "
MISC_14	"\nSorry, the value of '%s' must be one of: "
$	In MISC_15 the 'on' and 'off' are required in English as the are stored
$	internal to the printer database.  Should change this in the future to
$	read language varient and store 'on' or 'off' as appropriate.
MISC_15	"\nSorry, boolean symbol '%s' can only be 'on' or 'off'\n"
MISC_16	"\nSorry, integer symbol '%s' must contain only digits (0 - 9).\n"
MISC_17	"\nSorry, illegal baudrate: %s\n\nAvailable baud rates are:\n"
MISC_18	"\nSorry, the value of symbol '%s' must begin with '/'.\n"
MISC_19	"lprsetup: bad type %d for symbol %s.\n"
MISC_20A	"\nSet up activity is complete for this printer.\n"
MISC_20B	"Verify that the printer works properly by using\n"
MISC_20C	"the lpr(1) command to send files to the printer.\n"
MISC_21	"\nPress 'RETURN' to continue or 'quit RETURN' to quit: "
MISC_22	
MISC_23	
MISC_24	
MISC_25	
MISC_26	
MISC_27	
MISC_28	
MISC_29	
MISC_30	

$	Messges used by printlib.c routines
PRINTLIB_1	"Printcap entry to long\n"
PRINTLIB_2	"Bad printcap entry\n"
PRINTLIB_3	"Infinite tc= loop in /etc/printcap\n"
PRINTLIB_4	"Infinite tc= loop in /etc/lprsetup.dat\n"

$	General help messages (commands and operation)
LPRSETUP_HELP_0A	"\n\
This program helps you to set up /etc/printcap for your printers.\n\
\n\
Select a command from the Main Command Menu by typing one of the\n\
commands listed (or just the first letter of the command)\n\
and pressing RETURN.  You will then be prompted to enter\n\
the number of the printer (for example 0 or 1).  Then you will be\n\
prompted for other responses, depending on the command.\n\
\n\
For all prompts, a default response value is given in \[ \].  To use\n\
the default, just press RETURN.  You can always enter '?' for help.\n"
LPRSETUP_HELP_0B	"\n\
Some knowledge of the printer and its installation is required\n\
in order to correctly use this program.  Refer to the passage\n\
in the System Management Guide for general requirements.\n\
Chapter 4 - The Print System\n\
\n\
Most of the setup activities require you to enter values for symbols\n\
in the file /etc/printcap.  Refer to printcap(4) in the\n\
Digital OSF/1 Reference Pages for a summary of the possible symbols\n\
for /etc/printcap.  This program provides a detailed explanation\n\
of each printcap symbol in response to '?' after a symbol name\n\
has been specified and a value for the symbol is prompted.\n"
LPRSETUP_HELP_0C	"\n\
The Available commands are:\n\
\n\
  add    -> add a new printcap entry (local and remote)\n\
  delete -> delete a printcap entry (local and remote)\n\
  modify -> modify a printcap entry (local and remote)\n\
  view   -> view the contents of the printcap file\n\
  exit   -> exit or quit lprsetup\n\
  quit   -> quit or exit lprsetup\n\
  ?	 -> ask for help  (at any prompt)\n\
  help   -> ask for help  (at any prompt)\n\
\n\
Commands may be abbreviated to their first letter.\n\
Any command can be quickly exited by typing a Control-D\n\n"
LPRSETUP_HELP_1A	"\n\
Enter a printer name, ie., the name by which you want to identify\n\
the printer through the lpr command, example: lpr -Pprintername.\n\
Lprsetup uses an internal numbering scheme from 0 to 99. The\n\
next available number is the default name, you may chose the\n\
default by simply pressing RETURN or enter any other alphanumeric\n\
name that is appropriate. Lprsetup will always assign at least\n\
two printer synonyms: the default number and 'lp default number', plus\n\
any others you specify, both here and later when specifying synonyms.\n"
LPRSETUP_HELP_1B	"\
If the default number was 1, the two names would be '1' and 'lp1'.\n\
This printer could then be identified with 'lpr -P1' or 'lpr -Plp1'\n\
If you only have one printer or are entering the first of many\n\
printers, the first one will have a printer number of 0. This is\n\
recognized as your system's default printer and will have an\n\
additional name of just 'lp'. This means if you use 'lpr' without\n\
specifiying a specific printer this is the one it will use.\n"
LPRSETUP_HELP_2	"\n\
This section deletes an existing printer.\n\
Enter the name of the printer.  The name must be one of the printer's\n\
synonyms.  The name of the printer that you enter must match\n\
that of a printer in the /etc/printcap file in order to delete it.\n\
\n\
The entry is removed from the printcap file, the spooling directory\n\
is deleted, along with the accounting and error log files, if used.\n"
LPRSETUP_HELP_3	"\n\
This section modifies one or more existing printcap entries.\n\
Enter the name of the printer.  The name must be one of the printer's\n\
synonyms.  The name of the printer that you enter must match\n\
that of a printer in the /etc/printcap file in order to modify it.\n\
You will then be prompted to name the symbol in /etc/printcap which\n\
you wish to change.  You may instead enter 'p' to print all of the\n\
current printcap parameters; 'l' to list all of the possible printcap\n\
symbols and defaults;  or enter 'q' (for quit) to return to the\n\
main Command menu.\n"
LPRSETUP_HELP_4	"\n\
Enter the name of the printcap symbol you wish to modify.  Other\n\
valid entries are:\n\
  q ->  to quit (no more changes)\n\
  p ->  to print the symbols you have specified so far\n\
  l ->  to list all of the possible symbols and defaults\n\
The names of the printcap symbols are:\n\n"
LPRSETUP_HELP_5	"\n\
Enter alternate names for this printer.  Some examples include\n\
'draft',  'letter', and 'LA-75 Companion Printer'.  If the name\n\
contains blanks or tabs it is assumed to be last. You can enter as many\n\
alternate names	for a printer as you like, but the total length of\n\
the line containing the names must be less than 80 characters.\n\
After entering a synonym, you will be prompted again.  If you do not\n\
wish to enter any more synonyms, press RETURN to continue.\n\
\n\
Each synonym (including	the printer number) identifies the printer\n\
to the print system.  For example, if you enter the synonym 'draft'\n\
for this printer, then the command 'lpr -Pdraft files ...'\n\
will print files on this printer.\n"
LPRSETUP_HELP_6A	"\n\
Printers are listed by type and only those supported by DIGITAL.\n\
These printers have some default values	already included in the\n\
setup program.\n\
\n\
Other printers can be set up by using 'unknown' and then\n\
responding to the prompts, using values similar to those for\n\
supported printers.\n\
\n\
Responding 'remote' allows you to designate a remote system\n\
for printing.  In this case, only four printcap entries are required:\n\
  rm ->  name of the remote system\n\
  rp ->  name of the printer on the remote system\n\
  sd ->  pathname of the spooling directory on this system\n\
  lp ->  the local line printer device - which is always null\n\
The lp parameter must be present in order to print to a remote printer.\n"
LPRSETUP_HELP_6B	"\n\
Responding 'printer?' allows you to enter a mode where more informatio\n\
can be requested for each printer type. In this mode you will be\n\
prompted to enter the same printer types as above. Information about\n\
the printer and the default printcap entries for that printer will be\n\
displayed. Enter 'quit' to return to the prompt to select the printer\n\
type being added.\n\
\n\
When specifying printer type full command names and printer names\n\
must be used.\n\
\n\
The default printer type is 'unknown'.\n\n"
LPRSETUP_HELP_7	"\nEnter a new value, or press RETURN to use the default.\n"
LPRSETUP_HELP_8	"\n\
Add Comments allows you to add comments to the printcap file while\n\
still running lprsetup. The comments will be insert directly above\n\
the printcap entry in the printcap file. The default is not to enter\n\
any comments so simply press RETURN when prompted if you do not want\n\
to enter comments. Any other response will assume that you do. At the\n\
'# ' prompt enter your comment, press RETURN at the '# ' prompt to\n\
exit.\n\n"
LPRSETUP_HELP_9	"\n\
Printer Type allows you to request more information on a specific\n\
printer type. Specify one of the supported printers, 'remote', or\n\
'unknown' at the prompt. Type 'quit' to exit this mode. More\n\
information about that printer is displayed, including the default\n\
printcap settings.\n\
\n\
The Supported printers are:\n\n"
LPRSETUP_HELP_10	"feature is not used with\n\
the parallel line printer interface.  You would only\n\
specify this symbol for a printer which is connected via\n\
a serial terminal line.\n"

$	
$	Help strings for the printcap options
OPTION_HELP_af	"\n\
The 'af' parameter is the name of the accounting file used to\n\
keep track of the number of pages printed by each user for each\n\
printer.  The name of the accounting file should be unique for\n\
each printer on your system. The information stored in the\n\
accounting file is used by the pac(8) program. Note this file\n\
must be owned by `daemon'. It should be if it was created using\n\
this script. The `af' parameter is not applicable for remote\n\
printer entries.\n\
\n\
When the accounting file is created, intermediate directories\n\
will be created as neccessary.\n"
OPTION_HELP_br	"\n\
The 'br' parameter specifies the baud rate for the printer.\n\
The baud rate is dependent upon the printer hardware.\n\
Consult your printer hardware manual for the correct baud rate.\n\
The 'br' parameter is only applicable for tty devices, ie. serial.\n\
This parameter has no effect on printers connected to the console\n\
port or to printers connected on a parallel port.\n"
OPTION_HELP_cf	"\n\
The 'cf' parameter specifies the output filter for the\n\
cifplot data filter.\n\
\n\
Printer filter entries can either be an executable program\n\
or a single line which is interpreted by the shell.\n"
OPTION_HELP_ct	"\n\
The 'ct' parameter specifies the type of connection to the printer.\n\
Choices are dev, LAT, and remote.\n\
    dev    - for local devices \n\
    LAT    - for LAT devices (must be specified in UPPERCASE (LAT)\n\
    remote - for remote printers, when 'lp' is NULL"
OPTION_HELP_df	"\n\
The 'df' parameter specifies the output filter for the TeX data\n\
filter (DVI format).\n\
\n\
Printer filter entries can either be an executable program\n\
or a single line which is interpreted by the shell.\n"
OPTION_HELP_dn	"\n\
The 'dn' parameter specifies the name of the daemon program to\n\
invoke each time a print request is made to the printer.  The\n\
default daemon name is '/usr/lbin/lpd', and should not be\n\
changed.  The 'dn' parameter is available here so that the\n\
system may support multiple line printer daemons.\n"
OPTION_HELP_du	"\n\
The 'du' parameter specifies the daemon UID used by the printer\n\
spooler programs.  The default value, (0) should not be changed.\n\
The 'du' parameter is available here so that printer daemons,\n\
other than /usr/lib/lpd, may be used.\n"
OPTION_HELP_fc	"\n\
The 'fc' parameter specifies which terminal flag bits to clear\n\
when initializing the printer line.  Normally, all of the bits\n\
should be cleared (fc=0177777 octal) before calling 'fs'.  Refer\n\
to the discussion of 'sg_flags' in termios(4) of the Digital OSF/1 Reference\n\
Pages, or see help message for the 'fs' parameter.\n"
OPTION_HELP_ff	"\n\
The 'ff' parameter is the string to send as a form feed to the\n\
printer.  The default value for this parameter is '\\f'.\n"
OPTION_HELP_fo	"\n\
The boolean parameter 'fo' specifies whether a form feed will\n\
be printed when the device is first opened.  This is in addition\n\
to the normal form feed which is printed by the driver when the\n\
device is opened.  To suppress ALL printer induced form feeds,\n\
use the 'sf' flag, in addition to the 'fo' flag.\n\
(NOT USED)\n"
OPTION_HELP_fs_A	"\n\
The 'fs' parameter specifies which terminal flag bits to set\n\
when initializing the printer line.  Normally, all of the bits\n\
should be cleared (using fc=0177777 octal) and then 'fs' should be\n\
used to set the specified bits.  Refer to the discussion of\n\
'sg_flags' in termios(4) of the Digital OSF/1 Reference Pages.\n\
A short discussion on each bit is listed below:\n"
OPTION_HELP_fs_B	"\n\
  ALLDELAY  0177400  Delay algorithm selection\n\
  BSDELAY   0100000  Select Backspace delays (not implemented)\n\
  BS0	    0\n\
  BS1	    0100000\n\
  VTDELAY   0040000  Select form-feed and vertical tab delays\n\
  FF0	    0\n\
  FF1	    0100000\n\
  CRDELAY   0030000  Select carriage-return delay\n\
  CR0	    0\n\
  CR1	    0010000\n\
  CR2	    0020000\n\
  CR3	    0030000\n\
  TBDELAY   0060000  Select tab delays\n\
  TAB0	    0\n\
  TAB1	    0002000\n\
  TAB2	    0004000\n\
  XTABS	    0006000\n\
  NLDELAY   0001400  Select new-lines delay\n\
  NL0	    0\n\
  NL1	    0000400\n\
  NL2	    0001000\n\
  NL3	    0001400\n\
  EVENP	    0000200  Even parity allowed on input (most terminals)\n\
  ODDP	    0000100  Odd parity allowed on input\n\
  RAW	    0000040  Raw mode: wake up on all characters; 8bit interface\n\
  CRMOD	    0000020  Map CR into LF; echo LF or CR as CR-LF\n\
  ECHO	    0000010  Echo (full duplex)\n\
  LCASE	    0000004  Map upper case to lower on input\n\
  CBREAK    0000002  Return each character as soon as it is typed\n\
  TANDEM    0000001  Automatic flow control\n"
OPTION_HELP_gf	"\n\
The 'gf' parameter specifies the graph data filter.\n\
\n\
Printer filter entries can either be an executable program\n\
or a single line which is interpreted by the shell.\n"
OPTION_HELP_ic	"\n\
The 'ic' parameter is the driver that supports (nonstandard)\n\
ioctl to an independent printout.\n"
OPTION_HELP_if	"\n\
The 'if' parameter is the name of a filter which does\n\
accounting.  Filters which can be used for if include:\n\
\n\
  Filter name:	     Description:\n\
  ------------	     ------------\n\
  /usr/lbin/lpf	     line printer filter\n\
		       (LP25, LP26, LP27, LP29, LG01, LA210, LQP02, LQP03)\n\
  /usr/lbin/lqf	     letter quality filter (LQP02, LQP03)\n\
  /usr/lbin/ln03of   Non-PostScript Laser Printers (LN03,LN03S)\n\
  /usr/lbin/la75of   LA75 Printers\n\
  /usr/lbin/lg31of   LG31 Printers\n\
  /usr/lbin/lg02of   LG02 Printers\n\
  /usr/lbin/lj250of  LJ250 DEColorwriter filter\n\
  /usr/lbin/ln03rof  LN03R postscript Printer\n\
\n\
If an accounting file is specified with the 'af' parameter then this\n\
filter is used instead of the one specified by the 'of' parameter\n\
to process the print job.\n\
\n\
Printer filter entries can either be an executable program\n\
or a single line which is interpreted by the shell.\n"
OPTION_HELP_lf	"\n\
The 'lf' parameter is the logfile where errors are reported.\n\
The default logfile, if one is not specified, is '/dev/console'.\n\
If you have more than one printer on your system, you should give\n\
each logfile a unique name.\n\
\n\
When the error log file is created, intermediate directories\n\
will be created as neccessary.\n"
OPTION_HELP_lo	"\n\
The 'lo' parameter is the name of the lock file used by the\n\
printer daemon to control printing the jobs in each spooling\n\
directory.  The default value, 'lock', should not be changed.\n\
This parameter is available for use by 'other' printer daemons.\n"
OPTION_HELP_lp	"\n\
The 'lp' parameter is the name of the special file to open for\n\
output.  The default value is '/dev/lp', which is for a parallel\n\
printer. Additional parallel printers should have names like: lp1,\n\
lp2, lp3, ..., etc.,. For serial printers, a terminal line should\n\
have a name like /dev/tty01, /dev/tty02, ..., etc.\n"
OPTION_HELP_mc	"\n\
The 'mc' parameter is the maximum number of copies allowed.\n\
This parameter is used in conjunction with the lpr -#n option.\n\
See lpr(1) in the Digital OSF/1 Reference Pages.\n"
OPTION_HELP_mx	"\n\
The 'mx' parameter specifies the maximum allowable filesize\n\
(in BUFSIZ blocks) printable by each user.  Specifying mx=0\n\
removes the filesize restriction entirely. If this parameter\n\
is not specified, the default value of 1000 blocks is assumed.\n\
The 'mx' parameter effects the size of the job on the machine\n\
that the lpr command was executed.\n"
OPTION_HELP_nc	"\n\
The 'nc' parameter does not allow control characters in the output file.\n"
OPTION_HELP_nf	"\nThe 'nf' parameter specifies a ditroff filter.\n"
OPTION_HELP_of_A	"\n\
The 'of' parameter specifies the output filter to be used with\n\
the printer.  Output filters are used to filter text data to the\n\
printer device when accounting is not used or when all text data\n\
must be passed through a filter.  Filters which can be used for\n\
output include:	\n\
\n\
  Filter name:	     Description:\n\
  ------------	     ------------\n\
  /usr/lbin/lpf	     line printer filter\n\
		       (LP25, LP26, LP27, LP29, LG01, LA210, LQP02, LQP03)\n\
  /usr/lbin/lqf	     letter quality filter (LQP02, LQP03)\n\
  /usr/lbin/ln03of   Non-PostScript Laser Printers (LN03,LN03S)\n\
  /usr/lbin/la75of   LA75 Printers\n\
  /usr/lbin/lg31of   LG31 Printers\n\
  /usr/lbin/lg02of   LG02 Printers\n\
  /usr/lbin/lj250of  LJ250 DEColorwriter filter\n\
  /usr/lbin/ln03rof  LN03R postscript Printer\n"
OPTION_HELP_of_B	"\n\
This filter is used as the output filter unless an accounting file\n\
is specified with the 'af' parameter, in which case the filter\n\
specified with the 'if' parameter is used as the output filter\n\
as well as keeping track of accounting information.\n\
\n\
If you are setting up a PrintServer, the output filter requires\n\
non-standard arguments.  Replace the dummy nodename with the node\n\
name of your PrintServer followed by '%%U %%H %%J' (This causes your\n\
Nodename, Username Hostname and Job-id to be passed to the filter.)\n"
OPTION_HELP_op	"\n\
The 'op' parameter specifies the object port on a LAT terminal server.\n\
This parameter must be specified in uppercase.\n"
OPTION_HELP_os	"\n\
The 'os' parameter specifies the object service on a LAT server.\n"
OPTION_HELP_pl	"\n\
The 'pl' parameter specifies the page length in lines.  The\n\
default page length is 66 lines.\n"
OPTION_HELP_pp	"\n\
The 'pp' parameter specifies the print command filter replacement.\n"
OPTION_HELP_ps	"\n\
The 'ps' parameter specifies the mode in which the daemon runs.  It\n\
selects between non-postscript and specific postscript printers.  Choices\n\
are non_PS and LPS.\n\
(NOT USED UNLESS OPTIONAL PRINTSERVER SOFTWARE INSTALLED)\n"
OPTION_HELP_pw	"\n\
The 'pw' parameter specifies the page width in characters.  The\n\
default page width is 132 characters, although a page width of\n\
80 characters is more useful for letter quality printers, whose\n\
standard paper size is 8 1/2\" x 11\".\n"
OPTION_HELP_px	"\n\
The 'px' parameter specifies the page width in pixels.\n"
OPTION_HELP_py	"\n\
The 'py' parameter specifies the page length in pixels.\n"
OPTION_HELP_rf	"\n\
The 'rf' parameter specifies the filter for printing FORTRAN style\n\
text files.\n\
\n\
Printer filter entries can either be an executable program\n\
or a single line which is interpreted by the shell.\n"
OPTION_HELP_rm	"\n\
The 'rm' parameter specifies the machine name for a remote printer.\n\
This parameter should only appear in the printcap entries for remote\n\
printers.\n\
\n\
In order for a remote machine to accept a printer request the name of\n\
the machine making the request must appear in either the /etc/hosts.equiv\n\
file or the /etc/hosts.lpd file on the remote system, ie., the system\n\
that the printer resides on.\n\
\n\
A remote machine name in /etc/hosts.equiv allows root on the remote\n\
machine to have root priviledges on the host. This is a little over\n\
for just printing needs and is dangerous for security reasons.\n\
\n\
A remote machine name in the /etc/hosts.lpd file allows users on the\n\
remote machine to use the printer on the host, however, root does not\n\
have any priviledges.  A '*' on the first line of /etc/hosts.lpd allows\n\
all remote systems to use its printers.\n"
OPTION_HELP_rp	"\n\
The 'rp' parameter specifies the remote printer name argument.\n\
The name specified must be one of the recognized names for the\n\
printyer on the remote machine.\n\
\n\
In order for a remote machine to accept a printer request the name of\n\
the machine making the request must appear in either the /etc/hosts.equiv\n\
file or the /etc/hosts.lpd file on the remote system, ie., the system\n\
that the printer resides on.\n\
\n\
A remote machine name in /etc/hosts.equiv allows root on the remote\n\
machine to have root priviledges on the host. This is a little over\n\
for just printing needs and is dangerous for security reasons.\n\
\n\
A remote machine name in the /etc/hosts.lpd file allows users on the\n\
remote machine to use the printer on the host, however, root does not\n\
have any priviledges. A '*' on the first line of /etc/hosts.lpd allows\n\
all remote systems to use its printers.\n"
OPTION_HELP_rs	"\n\
The 'rs' parameter restricts the remote users to those with local accounts.\n"
OPTION_HELP_rw	"\n\
The boolean parameter 'rw' specifies that the printer is to be\n\
opened for both reading and writing.  Normally, the printer is\n\
opened for writing only.\n"
OPTION_HELP_sb	"\n\
The boolean 'sb' parameter specifies a short banner consisting of one\n\
line only.\n"
OPTION_HELP_sc	"\n\
The boolean 'sc' parameter suppresses multiple copies.  This is\n\
equivalent to setting the 'mc' parameter to 1.\n"
OPTION_HELP_sd	"\n\
The 'sd' parameter specifies the spooling directory where files\n\
are queued before they are printed.  Each spooling directory\n\
should be unique. All printcap entries must specify a spooling\n\
directory, both local and remote.\n\
\n\
When the spooling directory is created, intermediate directories\n\
will be created as neccessary.\n"
OPTION_HELP_sf	"\n\
The boolean parameter 'sf' suppresses all printer induced form\n\
feeds, except those which are actually in the file.  The 'sf'\n\
flag, in conjunction with 'sh', is useful when printing a letter\n\
on a single sheet of stationery.\n"
OPTION_HELP_sh	"\n\
The boolean parameter 'sh' suppresses printing of the normal\n\
burst page header.  This often saves paper, in addition to being\n\
useful when printing a letter on a single sheet of stationary.\n"
OPTION_HELP_st	"\n\
The 'st' parameter specifies the status file name. The default name\n\
is 'status'. The status file is located in the spooling directory.\n\
The current status of the printer is written in this file.\n"
OPTION_HELP_tf	"\n\
The 'tf' parameter specifies a troff data filter.\n"
OPTION_HELP_tr	"\n\
The 'tr' parameter specifies a trailing string to print when\n\
the spooling queue empties.  It is generally a series of form\n\
feeds, or sometimes an escape sequence, to reset the printer\n\
to a known state.\n"
OPTION_HELP_ts	"\n\
The 'ts' parameter specifies a LAT terminal server node name.\n\
This parameter must be specified in uppercase.\n"
OPTION_HELP_uv	"\n\
The 'uv' parameter specifies the Ultrix version and is used by the daemon\n\
to decide how to work out the connection type and whether to expand %\n\
escapes.  There are two version arguments: 3.0 and 4.0.  If 3.0 is used\n\
the daemon uses rm rm lp etc. to work out the connection in a backwardly\n\
compatable way; if 4.0 is used, the connection type is taken from ct and\n\
any % escapes are expanded. (IGNORED on OSF/1 for ULTRIX compatibility only)\n"
OPTION_HELP_vf	"\n\
The 'vf' parameter specifies a raster image filter.\n\
Filters currently available include:\n\
\n\
  Filter name:	 Description:\n\
  ------------	 ------------\n\
    (none, except as noted)\n\
\n\
Note that the following raster image filters are specified using\n\
the 'if' or 'of'parameter:\n\
  /usr/lbin/lj250of LJ250 Ink Jet Printer filter\n\
  /usr/lbin/ln03of  Non-postscript Laser Printers (LN03, LN03S)\n\
  /usr/lbin/la75of  LA75\n\
  /usr/lbin/lg02of  LG02\n\
  /usr/lbin/lg31of  LG31\n\
  /usr/lbin/ln03rof LN03R postscript Printer\n\
\n\
Printer filter entries can either be an executable program\n\
or a single line which is interpreted by the shell.\n"
OPTION_HELP_xc	"\n\
The 'xc' parameter specifies the local mode bits to clear\n\
when the terminal line is first opened.  Refer to the\n\
discussion of the local mode word in termios(4) of the Digital OSF/1\n\
Reference Pages.  xc#0177777 clears all bits. See help\n\
message for the 'xs' parameter.\n"
OPTION_HELP_xf	"\n\
The 'xf' parameter specifies the pass-thru filter name.\n\
This routine is used when output is already formatted for\n\
printing and does not require special filtering.\n\
\n\
Printer filter entries can either be an executable program\n\
or a single line which is interpreted by the shell.\n"
OPTION_HELP_xs_A	"\n\
The 'xs' parameter specifies the local mode bits to set when the\n\
terminal line is first opened.  Refer to the discussion of the local\n\
mode word in termios(4) of the Digital OSF/1 Reference Pages. Normally the\n\
bits are first cleared using the 'xc' parameter xc#0177777. A short\n\
discussion on each of the local mode bits is presented below:\n"
OPTION_HELP_xs_B	"\n\
  LCRTBS   000001  Backspace on erase rather than echoing erase\n\
  LPRTERA  000002  Printing terminal erase mode\n\
  LCRTERA  000004  Erase character echoes as backspace-space-backspace\n\
  LTILDE   000010  Convert ~ to ` on output (for Hazeltine terminals) \n\
  LMDMBUF  000020  Start/Stop output when carrier drops (unimplemented)\n\
  LLITOUT  000040  Suppress output translations - 8 bit\n\
  LTOSTOP  000100  Send SIGTOC fro background output\n\
  LFLUSHO  000200  Output is being flushed\n\
  LNOHANG  000400  Do not send hangup when carrier drops\n\
  LETXACK  001000  Diablo style buffer hacking (unimplemented)\n\
  LCRTKIL  002000  BS-space-BS erase entire line on line kill\n\
  LCTLECH  010000  Echo input control chars as ^X, delete as ^?\n\
  LPENDIN  020000  Retype pending input at next read or input char\n\
  LDECCTQ  040000  Only ^Q restores output after ^S, like DEC systems\n\
  LNOFLSH  100000  Flush output on receipt of 'suspend character'\n"
OPTION_HELP_Da	"\n\
The 'Da' parameter specifies the default DATA TYPE used by the daemon.\n\
Choices are: ansi, ascii, postscript, regis, and tek.  If a data type\n\
other than postscript is specified, a translator is invoked by the daemon\n\
to convert the files in the job to postscript.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Dl	"\n\
The 'Dl' parameter specifies the DEVICE CONTROL MODULE library file.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_It	"\n\
The 'It' parameter specifies the default INPUT TRAY.  Choices are bottom,\n\
lcit, middle and top.\n\
An error will occur if the specified INPUT TRAY is not available on the\n\
printer.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Lf	"\n\
The 'Lf' parameter specifies the LAYUP TO POSTSCRIPT translator program.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Lu	"\n\
The 'Lu' parameter specifies the default LAYUP DEFINITION FILE.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Ml	"\n\
The 'Ml' parameter specifies the default action to be taken with user\n\
errors produced by the PrintServer.  Choices are keep and ignore.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Nu	"\n\
The 'Nu' parameter specifies the default NUMBER UP value (ie number of\n\
pages per sheet).  The value must be in the range [1-100].\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Or	"\n\
The 'Or' parameter specifies the default ORIENTATION.  Choices are\n\
portrait and landscape.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Ot	"\n\
The 'Ot' parameter specifies the default OUTPUT TRAY. Choices are face-up,\n\
lcos, lower, side, top and upper.\n\
An error will occur if the specified OUTPUT TRAY is not available on the\n\
printer.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Ps	"\n\
The 'Ps' parameter specifies the default PAGE SIZE.  Choices are a,\n\
letter, a3, a4, a5, b, ledger, b4, b5, executive and legal.\n\
An error will occur if the specified PAGE SIZE is not available on the\n\
printer.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Sd	"\n\
The 'Sd' parameter specifies the default SHEET SIZE value.  It differs\n\
from the Ss parameter in that if the specified sheet size is not available\n\
then no error will occur and the job will be printed on whatever sheet\n\
size is available.  This parameter is overriden by the Ss parameter. If Sd\n\
is not specified, the translators assume a-sized paper so it\n\
should always be used in countries where the standard paper size is other\n\
than a.  Choices are a, letter,a3, a4, a5, b, ledger, b4, b5, executive\n\
and legal.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Si	"\n\
The 'Si' parameter specifies the default SIDES option.  Choices are 1,\n\
one_sided_duplex, 2, two_sided_duplex, tumble, two_sided_tumble,\n\
one_sided_duplex, one_sided_tumble, two_sided_simplex.\n\
An error will occur if the specified SIDES option is not available on the\n\
printer.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Ss	"\n\
The 'Ss' parameter specifies the default SHEET SIZE.  Choices are a,\n\
letter,a3, a4, a5, b, ledger, b4, b5, executive and legal.\n\
An error will occur if the specified SHEET SIZE is not available on the\n\
printer.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Ul	"\n\
The 'Ul' parameter specifies the default UPPER PAGE LIMIT value.  The\n\
must be in the range [1-10000]\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"
OPTION_HELP_Xf	"\n\
The 'Xf' parameter specifies the translator dispatch program.\n\
\n\
This parameter is only valid with the optional PrintServer software for\n\
postscript printers.\n"

$set MS_LPRSETUP_SEC
$	no strings yet as SECLIB et al. isn't used {yet}

