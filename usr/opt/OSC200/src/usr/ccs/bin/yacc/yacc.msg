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
$ HISTORY

$ @(#)$RCSfile: yacc.msg,v $ $Revision: 4.1.4.2 $ (DEC) $Date: 1993/08/02 19:44:05 $

$ 
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$ 
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDLANG) Language Utilities
$
$ FUNCTIONS:
$
$
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set MS_YACC
$quote "
$len
$ Message numbers have been assigned a range of 700-799
$ Do not translate yacc, %d, %s, %1$d, %1$s, etc ...
$ In this message file only do not translate terminal, nonterminal, token, type.
$ These are very specific yacc technical terms.
NOPARSER        "Cannot find parser %s"
NOYACDEBUG      "Cannot open yacc.debug"
$ Do not translate yacc.debug
ETEMPF          "Cannot open action temporary file."
EITEM           "A terminal or nonterminal is too big."
SUMMARY1        "\n%d/%d terminals, %d/%d nonterminals\n"
$ Do not translate.
SUMMARY2        "%d/%d grammar rules, %d/%d states\n"
$ Do not translate.
SUMMARY3        "%d shift/reduce, %d reduce/reduce conflicts reported\n"
$ Do not translate.
SUMMARY4        "%d/%d working sets used\n"
$ Do not translate.
SUMMARY5        "memory: states,etc. %d/%d, parser %d/%d\n"
$ Do not translate.
SUMMARY6        "%d/%d distinct lookahead sets\n"
$ Do not translate.
SUMMARY7        "%d extra closures\n"
$ Do not translate.
SUMMARY8        "%d shift entries, %d exceptions\n"
$ Do not translate.
SUMMARY9        "%d goto entries\n"
$ Do not translate.
SUMMARY10       "%d entries saved by goto default\n"
$ Do not translate.
CONFLSR         "\nconflicts: %d shift/reduce\n"
$ Do not translate.
CONFLRR         "\nconflicts: %d reduce/reduce\n"
$ Do not translate.
CONFLSRR        "\nconflicts:%d shift/reduce, %d reduce/reduce\n"
$ Do not translate.
FATALERR         "\n yacc: %1$s Line %2$d\n"
$ Do not move %1$s
NONTERM         "The nonterminal %s is not defined."
YACINTER        "Internal error; pyield is %d."
$ Do not translate pyield.
NOLOOK          "Cannot look ahead as state requires."
ESTATE          "Cannot have more than 5000 states."
PITEM           "putitem(%S), state %d\n"
$ This is a debug message. Do not translate.
ESTATE2         "Use the -N<number> flag to increase state size."

NONTERM1        "The nonterminal %s does not generate a token string."
WSET            "The working set is full."
$ working set is a internal array for work in progress.
STATELOOK       "\nState %d, nolook = %d\n"
$ This is a debug message. Do not translate.
FSET            "flag set!\n"
$ This is a debug message. Do not translate.
BADOPT          ""
$ Obsolete
NODEBUG         "Cannot open yacc.debug"
$ Do not translate yacc.debug
NOTOPN          "Cannot open %s"
NOTEMP          "Cannot open temporary file."
NOINPUT         "There is no specified input file."
BADSTART        "The %%start statement is not correct."
$ Do not translate %%start
BADSYNTAX       "The %%type syntax is not correct."
$ Do not translate %%type
EREDECLR        "The type of token %s is already declared."
EREDEC2         "The type of nonterminal %s is already declared."
EPRECID         "The precedence of %s is already declared."
ERETYPE         "The type of %s is already declared."
LATETYPE        "Define type number of %s earlier."
SYNTAX          "There is a syntax error."
PEOF            "Reached end of file before %%%%"
$ Do not translate %%%%
SNTAX1          "Syntax on first rule is not correct."
BADLHS          "Cannot have a token on LHS of grammar rule."
$ LHS is left hand side
BADDRULE        "There appears to be a missing ; or | in the rule."
$ Do not translate ; or |
BADPREC         "The %%prec syntax is not correct."
$ Do not translate %%prec
NONTERM2        "Cannot have nonterminal %s after %%prec"
$ Do not translate %%prec
MANYRULES       "Cannot have more than %d rules."
NOVALUE         "LHS has a type; rule must return a value."
$ LHS is left hand side
TCLASH          "A type conflict may occur because of default action."
MANYNTERMS      "Cannot have more than %d nonterminals."
MANYTERMS       "Cannot have more than %d terminals."
ESCINVAL        "The character escape is not valid."
BADCONSTRT      "The \\nnn construction is not valid."
$ \\nnn construction is referring to a coded entity in one of the input lines.
$ Do not translate \\nnn
OZERO           "\\000 is not valid."
$ Do not translate \\000
ELIT            "The identifiers and literals have too many characters."
UNTERM          "A < ... > clause has no end."
$ Do not translate < ... >
MISSQUOTE       "There is an extra or a missing ' or \" character."
ILLWORD         "The escape is not valid or %s is not a reserved word."
NOTYPE          "Must specify a type for %s."
NONAME          "Define %s earlier."
FEOF            "Reached end of file before %%}"
$ Do not translate %%)
BADCOMNT        "The comment is not correct."
BADIDENT        "Syntax on the $<identifier> clause is not correct."
$ Must have $<> around the word indentifier, identifier can be translated.
ILLOFF          "Use of $%d is not valid."
$ Do not translate $%d
NOTYPE2         "Must specify type of $%d"
$ Do not translate $%d
COMTEOF         "Reached end of file inside comment."
NLINSTR         "Newline found in string or character constant."
EOFINSTR        "Reached end of file in string or character constant."
NONTERMACT      "Action does not end."
REDRED1         "\n%d: reduce/reduce conflict (red'ns %d and %d ) on %s"
$ Do not translate.
OFF1            "off = %d, k = %d\n"
$ This is a debug message. Do not translate.
ACTOVR          "The action table is full."
ACTOVR2         "The action table is full."
GOTOON          "%s: gotos on "
$ This is a debug message. Do not translate.
SFTRED1         "\n%d: shift/reduce conflict (shift %d, red'n %d) on %s"
$ Do not translate.
ACCEPT          "accept"
$ Do not translate.
MERROR          "error"
$ Do not translate.
SHIFT           "shift %d"
$ Do not translate.
EREDUCE         "reduce %d"
$ Do not translate.
CRREDUCE        "\n\t.  reduce %d\n\n"
$ Do not translate.
CRERROR         "\n\t.  error\n\n"
$ Do not translate.
NOTRED          "Rule not reduced:   %s\n"
$ Do not translate.
NEVRRED         "yacc: %d rules are never reduced.\n"
BADTEMP         "The temporary file is corrupted."
ARYOVR          "The internal a array is full."
$ Do not translate a
NONTERM3        "Nonterminal %d, entry at %d\n"
$ Do not translate.
CANTGOTO        "Cannot put goto %d in array.\n"
$ Do not translate goto
STENTEQ         "State %d: entry at %d equals state %d\n"
$ Do not translate.
OPTSPC          "Out of space in optimizer a array."
$ Do not translate a
ARYCLBR         "The a array is corrupted at position %1$d, %2$d."
$ Do not translate a
STATENT         "State %d: entry at %d\n"
$ Do not translate.
STATERR         "Cannot put state %d in array.\n"
OPTUSED         "Optimizer space used: input %d/%d, output %d/%d\n"
$ Do not translate.
OPTTAB          "%d table entries, %d zero\n"
$ Do not translate.
OPTSPRED        "maximum spread: %d, maximum offset: %d\n"
$ Do not translate.
OUTOSPC         "Use -N<number> flag to increase space size."

NOOPT           "The optimizer cannot open the temporary file."
PREMIS          "Specify a prefix with the -%c flag."
OUTMEM          "There is not enough memory available now."
NMNUM           "Use an integer greater than 40000 with the -N flag."
NOPT            "The -N flag format is not correct."
EOFENC          "Reached end of file while processing %%union"
$ Do not translate %%union
NEEDARG		"Missing option argument for -%c flag."
USAGE		"usage: yacc -svdlt [-b prefix] [-p sym_prefix] [-P path] [-N num] file\n"
$ Do not translate usage message
BADHEXCONST	"The \\xnn construction is not valid."
$ Do not translate \\xnn
