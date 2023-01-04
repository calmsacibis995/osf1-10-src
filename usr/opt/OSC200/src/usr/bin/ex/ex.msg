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
$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ @(#)$RCSfile: ex.msg,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/08/03 11:20:32 $
$
$
$ HISTORY
$ 
$ OSF/1 1.1.1

$
$ COMPONENT_NAME: (CMDEDIT) ex.msg
$
$ FUNCTIONS: none
$
$ (C) COPYRIGHT International Business Machines Corp. 1984, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.6  com/cmd/edit/vi/ex.msg, cmdedit, bos320, 9130320 7/1/91 16:30:41
$ 
$len 
$set MS_EX
$quote "

$ Some messages have a @ or | character in them. This controls how much of 
$ that message will be shown. Ex, vi, view, vedit, edit, and tvi (the editors 
$ that use this message catalog) have a "terse" option to control how much 
$ information the user gets: 
$ The space should remain 
$       original    how it's shown      how it's shown 
$       message:     if terse:          if not terse: 
$       ________    _______________     ______________ 
$       xxx@yyy         xxx               xxx yyy  (the @ becomes a space) 
$       xxx|yyy         xxx               yyy 
$  M_499 through M_506 are used only with tvi. Other messages in this 
$       message catalog not listed below are also used by tvi which is also 
$       known as the trusted editor. 
$  M_258 through M_267 are the only messages used by expreserve, a program used 
$       to preserve files during an editing session or system failure. 
$  M_268 thorough M_276, M_278, M_287, and M_288 are the only messages used by 
$       exrecover, a program used to recover files saved by expreserve. 
$       Some messages begin with a space. Please preserve the space in the 
$       translation.
$       Do not translate %s, %c, %d, etc. These will be replaced by file names,
$       numbers, etc.
$       The term "regular expression" is a Unix technical term.
$
M_000 "%s is not a recognized flag.\n"
M_002 " Address_1 > address_2.| The first address cannot exceed the second address."
M_003 " Count is not correct.| The specified count must be greater than 0."
M_004 "An address is not allowed@with the specified command."
M_005 "The specified address is not valid."
M_006 " No match to end of file.| The address search reached the end of the file without matching the specified pattern."
M_007 " No match to top of file.| The address search reached the beginning of the file without matching the specified pattern."
M_008 " Not found| The pattern does not exist."
M_009 "A mark must be the ' character or a single character in the range a through z."
M_010 "The specified mark is not defined@and cannot be referenced."
M_011 "An address cannot be less than 0.@The first buffer line is 1."
M_012 "Not that many lines@exist in the file."
M_013 "The address offset after the command is too large."
M_015 "Cannot get the value of the HOME environment variable."
M_016 " No parameter.| The %s command requires  the following letter"
M_017 " Parameter not correct.| The parameter to the mark command must be a lower case letter."
M_018 "The preserve command failed."
$       Do not translate preserve.
M_019 " The file is preserved."
M_020 "No write@since the file was last modified.  Use :rew! to force the action."
$       Do not translate rew!
M_021 "Old tty driver|Not using new tty driver/shell."
$       Not part of present executable. Do not translate message. 
M_022 "Buffer names must be a single character, a through z or A through Z"
M_023 " Current line is EOF.| The current line is at the end of the file."
M_024 " Command not known.| %c is not a known command character."
M_025 " Extra characters.| There are extra characters at the end of the command."
M_026 " [Warning - %s is incomplete]"
M_027 " %d files@to edit."
M_028 " No more files@to edit."
M_029 " Extra characters.| There are extra characters at the end of the %s command."
M_032 "No write@since the file was last changed.  The %s! command will force the action."
$       Leave the ! character after the %s. 
M_033 " No such open or visual command| %S is not a recognized command while in open or visual mode."
$       Do not translate open or visual. 
M_034 " No such command| %S is not a recognized command."
M_035 " [Press return to continue] "
$       Leave the space at the end. 
M_036 "No more memory@for the append. Too many lines in the file."
M_037 "Line too long.| The result of a line join would be greater than %d characters."
M_038 " No address for %s.| The %s command requires a destination address."
M_039 "The specified move command would have no effect."
M_040 "The destination address of a move command cannot be within the source address."
M_041 "Cannot use the put command while inside a global command or macro."
M_042 "Line too long.| A shift cannot create a line longer than %d characters."
M_043 " Tag not correct| Only one tag is allowed per line."
$       do not translate tag. 
M_044 "There is no previous tag."
$       do not translate tag. 
M_045 "The %S entry in the tags file is not correct."
M_046 "No write has occurred@since last change."
M_047 "No tags file exists.  Run the ctags command on the file or files being edited."
$       do not translate tag. 
M_048 "%S is not a valid tag and is not in the tags file."
$       do not translate tag. 
M_049 "Cannot use the yank command inside a global command or macro."
M_050 "\nAt the end of the file."
M_051 "At the end of the file."
M_052 "Reached the end of the file."
M_053 "Reached the beginning of the file."
M_054 "Cannot use the undo command@in a global command."
$       "undo" is a Unix command.
M_055 "There is no command to undo."
M_056 "No change.| The last command to undo did not change anything."
M_057 "Specify a left side to the definition."
M_058 "Specify a right side to the definition."
M_059 "ex: The left hand side can only contain letters, digits and underscores."
M_060 "Definitions do not allow recursion."
M_061 "The left hand side of a map definition must be only 1 character and cannot include the : character."
M_062 "Definitions do not allow recursion."
M_064 "Not defined.| The specified macro is not defined."
M_065 "Cannot create more than 32 macro definitions."
M_066 "Cannot allocate more than 512 characters to macro text."
M_067 "An input line cannot be longer than %d characters."
M_068 "The Cntl-H character is discarded in input lines.\n"
M_069 "No file name.| No file name is currently defined."
M_070 " [Read only]"
M_071 " [Not edited]"
M_072 " [Modified]"
M_073 " No defined file name "
M_074 " The cursor is at line %1$d of %2$d --%3$ld%%-- ."
$       Please leave multiple % in the message.
M_075 "The pattern is too long."
M_076 "The maximum length of an argument is %d characters."
M_077 "No alternate file name@to substitute for the # character."
M_078 "No current file name@to substitute for the %% character."
M_079 "The pipe system call failed."
$       Do not translate pipe.
M_080 "Cannot create a process at this time."
M_081 "NCARGS parameter list is too long"
M_082 "NARGS parameter list is too long"
$       NCARGS and NARGS are system defined do not translate. 
M_083 "There is no file name that matches."
M_084 "Specify a file name"
M_085 "Ambiguous| There are too many file names"
M_086 "A file name cannot be longer then 1020 characters"
M_087 " [New file]"
M_088 " Block special file"
M_089 " Teletype"
M_090 " Character special file"
M_091 " Directory"
M_092 " Executable file"
M_093 " Archive file"
M_095 " [Read only]"
M_096 "Use w or w>> to write the file."
M_097 "No file name.| No file name is currently defined."
M_098 " File already exists.| The file already exists. Use w! %s to force the write."
$       w! is a subcommand. Do not translate. 
M_099 "The file has read permission only."
M_101 "Use w! to write the partial buffer."
$       w! is a subcommand. Do not translate. 
M_102 " [New file]"
M_103 " [Existing file]"
M_104 " [Last line is not complete]"
M_105 " A line cannot be longer than %d characters."
M_106 "The limit is 200 nested source files."
M_107 " %1$d/%2$ld"
$        Do not translate.  This is the terse message for M_292, M_295, and M_296. 
M_109 " (%ld null character)"
$       This will be used after M_107, M_292, M_295, or M_296. 
M_111 "A global within a global@command is not allowed."
M_112 "Global requires regular expression.| A global command requires a regular expression."
M_113 "The command in a global cannot be longer than 256 characters."
M_114 "An error in the requested substitution has created a loop."
M_115 "No match.| No match exists for the substitute pattern."
M_116 "Substitute needs regular expression.| Specify a regular expression for the substitute command."
M_117 "No previous regular expression| Specify a regular expression."
M_118 "No previous substitute regular expression| Specify a substitute regular expression for the & character to repeat."
M_119 "The replacement pattern is too long.@The limit is 256 characters."
M_120 "The substitution creates a line that is too long.@ The limit is %d characters."
M_121 "%d substitutions|%d substitutions were made"
M_122 " on %d lines"
$       M_122 will be displayed after M_121 if displayed anywhere.
M_123 "Regular expressions delimiters cannot be letters or digits."
M_124 "No previous scan regular expression.| no previous scanning regular expression is defined."
M_126 "A regular expression is not correct.| A regular expression \\ must be followed by a / or a ? character."
M_128 "Specify a closing delimiter@for the regular expression."
M_129 "Regular expression too long.| The regular expression cannot expand to more than 512 characters."
M_130 "No match for \\( delimiter.| There are more \\( start delimiters than end \\) delimiters."
M_131 "Too many \\( expressions.| Specify no more than 9 \\(...\\) constructs in a regular expression."
M_132 "No match for \\) delimiter.| There are more end \\) delimiters than start \\( delimiters."
M_133 "Cannot use &@- for a replacement pattern in a regular expression."
M_134 "Cannot use \\d@- for a replacement pattern in a regular expression."
M_135 "* character is not valid.| The * cannot be used after a \\(...\\) construct in a regular expression."
M_137 "Empty character class.| Cannot use an empty [] or [^] character class."
M_138 "Specify a closing ] delimiter."
M_139 "No new lines in re.| Regular expressions cannot contain new line characters."
M_140 "No closing delimiter.| The regular expression must have a closing delimiter."
M_141 "Internal error in regular expression."
M_142 "%S is not a recognized option.@Use set all to see all options."
M_143 "Option %s must be set to a value."
M_144 "Specify the = character@in the assignment for option %s."
M_145 "Specify a number@after the = operator."
M_146 "Option string is too long.@The limit is 128 characters."
M_147 "Cannot change terminal type while in open or visual mode."
M_148 "%d lines"
M_149 "An address cannot be zero@when using the specified command."
M_150 "No lines@exist in the specified buffer."
M_153 "Out of memory@saving lines for undo."
$       "undo" is an editor command.
M_190 "There is a system call error number %d."
M_191 " \r\nYour file has been preserved\r\n"
M_192 "An interrupt signal occurred."
M_193 "\nAn interrupt signal occurred."
M_194 "The temporary file is too large."
M_196 "Out of space in a named buffer or a numbered buffer."
M_197 "Editing buffer %c is empty."
M_198 "Cannot put a partial line inside macro"
M_199 "Editing buffer %c is empty."
M_200 "The register is greater than %d characters."
M_202 "%s is not a recognized terminal type."
M_203 "Specify a shell command@or use the :shell command."
M_204 "A command cannot be longer than 128 characters."
M_205 "No previous command@exists for the ! substitution."
$       Leave the ! character after the. 
M_206 "No alternate file name@exists for the # substitution."
M_207 "No file name@exists for the %% substitution."
$       needs to have multiple % characters so that when displayed only one appears. 
M_208 "[No write]|[There has been no write since the last change]"
M_209 "No previous command@is defined to repeat."
M_210 "Cannot create a pipe."
M_211 "Cannot create a process at this time."
M_212 "No %s!\n"
$       Leave the ! character after the %s. 
M_213 "Cannot create a pipe."
M_214 "Cannot create another process at this time."
M_215 "Cannot create a pipe for the recovery."
M_216 "Cannot create a process for the recovery."
M_217 "No recovery routine is available."
M_218 "Process number %1$d ended with a signal %2$d."
M_219 "Memory image file created."
$       M_219 will be displayed after M_218 if displayed anywhere. 
M_220 "Pattern does not exist.| The pattern does not exist on the specified line."
M_221 "You are already in open or visual mode."
M_222 "Open or visual mode is not available when the novice option is set."
M_223 "%s is a generic terminal type."
M_224 "[Using open mode]"
M_225 "Visual mode requires more cursor capability than the terminal provides."
M_226 "Cannot use visual mode on a terminal which overstrikes."
M_227 "Visual mode requires clear screen capability."
M_228 "Visual mode requires scrolling capability."
M_229 "The screen is too large for the internal buffer"
M_230 "Do not know enough about the terminal type to use %s"
M_231 "The terminal is too wide"
M_232 "The screen is too large"
$       M_229 through M_232 are debug messages. 
M_233 "There is an internal software error in the vscroll function."
$       Do not translate vscroll. 
M_234 "The current buffer is empty."
M_235 "There is an internal software error in the vredraw function."
$       Do not translate vredraw. 
M_236 "An error occurred while reading input."
M_238 "The macro is too long.@It may be recursive."
M_239 "A macro cannot be nested to more than 256 levels."
M_240 "Use Q to enter ex command mode or use :q to exit vi."
M_241 "REPLACE 1 CHARACTER"
$       message M_241 is the 'verbose' half of M_279 
M_242 "REPLACE MODE"
$       message M_242 is the 'verbose' half of M_280 
M_243 "INPUT MODE"
$       message M_243 is the 'verbose' half of M_281 
M_245 "A macro cannot be nested to more than 256 levels."
M_246 "An input line cannot be longer than %d characters."
M_248 "There is an internal software error in the vclreol function."
$       Do not translate vclreol. 
M_249 "There is an internal software error in the vgoto function."
$       Do not translate vgoto. 
M_250 "The line is too long for the open command"
M_251 "The line is too long"
M_252 "The character class is not recognized@in the [: ... :] construct."
M_253 "There is an internal software error in the cclass function."
$       Do not translate cclass. 
M_258 "expreserve: Only a user with proper authority can preserve files.\n"
M_259 "Buffer format error"
M_260 "Buffer read error"
M_261 "Cannot find a name"
M_262 "A copy of a file you were editing was saved when the system went down."
M_263 "A copy of a file you were editing was saved when the editor was killed."
$      "Kill" is a Unix technical term.
M_264 "No name was associated with this file so it has been named LOST.\n"
M_265 "A copy of your file %s was saved when the system went down."
M_266 "A copy of your file %s was saved when the editor was killed."
M_267 "This file can be retrieved using the recover command of the editor.  An easy way to do this is to give the command:\"ex -r %s\".  This works for the \"edit\" and \"vi\" editors also.\n"
M_268 " The number of parameters to exrecover is not correct."
$       Do not translate exrecover. 
M_269 " [Dated %1$s. This version is the newest of %2$d saved copies.]"
$       "Dated" is used in the sense of assigning a date.
M_270 " [Dated %s]"
M_271 "exrecover: There is not enough memory available now."
M_272 "exrecover: There are no preserved files.\n"
M_273 "%1$s saved %2$d lines of file %3$s\n"
M_274 "exrecover: The specified file does not exist."
M_275 "Lost line(s):"
M_277 " Temporary file is too large"
M_278 "exrecover: System call error %d has occurred."
M_279 "r"
$       This, and the following 2 messages should not be translated.
M_280 "R"
M_281 "I"
M_282 " %1$d more lines@in the file after the %2$s command."
M_283 "%1$d fewer lines@in the file after the %2$s command."
M_284 "%d more lines"
M_285 "%d fewer lines"
M_286 "%d lines"
M_287 "%a %sD %T"
$       This is a format string for strftime, not printf.
$	It may be reordered as needed for each country.
$	%a is the short day of the week.
$	%sD is the short date with format controled by LC_TIME.
$	%sT is the time with format controlled by LC_TIME.
M_288 "On %a %sD at %sT"
$       This is a format string for strftime, not printf.
$	It may be reordered as needed for each country.
$	%a is the short day of the week.
$	%sD is the short date with format controled by LC_TIME.
$	%sT is the time ommiting the seconds with format controlled by LC_TIME.
M_289 "The parameter to the partial character option must be a single character."
M_290 "Specify an f, g, r, or w@parameter after the = character."
M_291 "There is %d more file to edit."
M_292 " %1$d line, %2$ld character"
M_293 "There are fewer characters than specified "
M_294 "There are %d more files to edit."
M_295 " %1$d line, %2$ld characters"
M_296 " %1$d lines, %2$ld characters"
M_297 " (%ld null characters)"
$       This will be used after M_107, M_292, M_295, or M_296. 
M_298 " [Shared lock]"
M_299 " [Exclusive lock]"
M_300 " [FILE BEING MODIFIED BY ANOTHER PROCESS]"
M_301 " [File open by another process]"
M_302 " Cannot set novice interactively - invoke as vedit"
M_303 "vscroll: error, line cannot exceed maximum displayable characters."
M_304 "No command specified to -c.\n"
M_305 "Usage: vedit [-c subcommand] [-l] [-R] | [-r] [-t tag] [-wnumber] [-] [file ...]\n"
M_306 "Usage: vi [-c subcommand] [-ls] [-R] | [-r] [-t tag] [-wnumber] [+subcommand] [-] [file ...]\n"
M_307 "Usage: view [-c subcommand] [-l] [-t tag] [-wnumber] [+subcommand] [-] [file ...]\n"
M_308 "Usage: ex [-c subcommand] [-lRsv] [-wnumber] [+subcommand] [-] [file ...]\n   or: ex [-c subcommand] [-lRsv] [-t tag] [file ...]\n   or: ex [-c subcommand] -r[file] [-lRsv] [file]\n"
M_309 "Usage: edit [-c subcommand] [-lRv] [-wnumber] [+subcommand] [-] [file ...]\n   or: edit [-c subcommand] [-lRv] [-t tag] [file ...]\n   or: edit [-c subcommand] -r[file] [-lRv] [file]\n"
M_499 "New mappings cannot be defined."
M_500 "Tags are not allowed."
M_501 "Shell escapes are not allowed."
M_502 "The Q command is not allowed."
M_503 "The recover command is not allowed."
M_504 "The open command is not allowed."
M_505 "The preserve command is not allowed."
$       Do not translate preserve.
M_506 "Mode switching is not allowed."
M_513 "Cannot find the Terminfo database.\n"
$       Do not translate Terminfo. 
M_514 "The %s option must be greater than 0. Default value used."
M_650 "Invalid multibyte character string encountered, conversion failed."
M_651 "Invalid wide character string encountered, conversion failed."
M_652 "Incomplete or invalid multibyte character encountered, conversion failed."
M_670 "Property invalid for the current locale."
M_671 "Error in character collation value."

$
$ mesages for expre_sec.c
$
SECMSG_1	"%s: can't initialize for sensitivity labels\n"
SECMSG_2	"%s: can't set clearance\n"
SECMSG_3	"%s: insufficient privileges\n"
SECMSG_4	"%s: can't allocate SL buffer\n"
SECMSG_5	"%s: can't get SL of %s\n"
SECMSG_6	"%s: can't change SL to process %s\n"

$
$ these messages are in ex.c
$
M_700		"Not enough memory space available.\n"
M_701		"cannot handle %d lines\n"
M_702		"set \"linelimit\" lower\n"
