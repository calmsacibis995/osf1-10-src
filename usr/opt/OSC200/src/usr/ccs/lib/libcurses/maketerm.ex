a
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
#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile$ $Revision$ (DEC) $Date$
#
#
# HISTORY
#
# OSF/1 Release 1.0

#
# @(#)maketerm.ex        1.5  com/lib/curses,3.1,8943 10/16/89 23:27:16
#
# COMPONENT_NAME: (LIBCURSE) Curses Library
#
# FUNCTIONS:    maketerm.ex
#
# ORIGINS: 3, 10, 27
#
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)
# OBJECT CODE ONLY SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1988
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

The following ed script will edit the caps file and produce the capnames.c
file and term.h file which corresponds to the caps defined.  The edit
commands assume that the caps file is in the following general form.

name,    "code" "cd" comment

the comma after name and the quotes around the codes are expected as
shown here.  The 'white space' between fields may be spaces or tabs.
The width of name, code and comment are variable and comment may include
spaces. cd must be exactly 2 characters wide (within the quotes).

Many of the edit commands below use the following general specification
to separate the line into the segments identified above.

/.*,.*".*".*".."[  ]*.*/
		     AA
		A   A||__  The comment specifier
	    A  A|___|_____ White space after cd [space tab]*
	  AA|__|__________ cd specification
      A  A||______________ white space between code and cd
    AA|__|________________ code specification
 A A||____________________ white space after name
 |_|______________________ name with comma delimiter

These segments will frequently be grouped using \( and \) to pick those
parts needed for a given operation.
.
1,$d
E caps.e
g/^#/d
/--- begin bool/+,/--- end bool/-w bool
/--- begin num/+,/--- end num/-w num
/--- begin str/+,/--- end str/-w str
a
For each segment of the caps file (bool, num str) produces a names file in
the following form:
namesfile  (boolnames, numnames, strnames)
	char  *xxxnames[] {
	       "list of names taken from 'code' field of caps",
	       "each name within quotes and followed by a comma",
	       0
	       } ;

	char  *xxxcodes[] {
	       "list of codes taken from 'cd' field of caps"
	       "each name within quotes and followed by a comma",
	       0
	       } ;

.
1,$d
E bool
1,$s;.*,.*\(".*"\).*"..".*;\1,;
1,$-10g/^/.,+9j
+,$j
1i
char *boolnames[] = {
.
$a
0
};
.
w boolnames
E bool
1,$s;.*,.*".*".*\(".."\).*;\1,;
1,$-10g/^/.,+9j
+,$j
1i
char *boolcodes[] = {
.
$a
0
};
.
0r boolnames
w boolnames
E num
1,$s;.*,.*\(".*"\).*"..".*;\1,;
1,$j
1i
char *numnames[] = {
.
$a
0
};
.
w numnames
E num
1,$s;.*,.*".*".*\(".."\).*;\1,;
1,$j
1i
char *numcodes[] = {
.
$a
0
};
.
0r numnames
w numnames
e str
1,$s;.*,.*\(".*"\).*"..".*;\1,;
1,$-10g/^/.,+9j
+,$j
1i
char *strnames[] = {
.
$a
0
};
.
w strnames
E str
1,$s;.*,.*".*".*\(".."\).*;\1,;
1,$-10g/^/.,+9j
+,$j
1i
char *strcodes[] = {
.
$a
0
};
.
0r strnames
w strnames
a
Combine the boolnames, numnames and strnames files together to form
the capnames.c file.
.
1,$d
f capnames.c
r boolnames
r numnames
r strnames
w
1,$d
a
Invoke 'sed' to produce files from bool, num and str in the following
form :

Name, name, /* comment */

In this form the commas and the comment delimiters are significant. In
addition the name field is the same as name in the bool, num or str file,
while the Name is the same except for the leading character which is
translated to upper case.  (sed is used for this because I could not find
any way to perform the translation using ed.)

The following lines create the sed script file then invoke sed against
each of the three files to produce boolcap, numcap and strcap respectively.

.
1,$d
f sedin
a
s;\(.*,\).*".*".*".."[ 	]*\(.*\);\1 \1 /* \2*/;
h
s/\(.\).*/\1/
y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/
G
s/\(.\)..\(.\)/\1\2/
.
w
!sed -f sedin bool >boolcap
!sed -f sedin num  >numcap
!sed -f sedin str  >strcap
!rm sedin
1,$d
a
Next use the xxxcap files just produced to create termdef. this is the
part of term.h which provides all the #defines for references to the
structure. Each input line is used to produce a #define in the following
format:

#define name    CUR Name

or for string values in the form

#define name    CUR strs.Name
#define name    CUR strs2.Name

The second form is used after the first 100 string types.

.
1,$d
E strcap
1,$s;\(.*\), \(.*\), /\*.*\*/;#define \2	CUR strs.\1;
101,$s;strs\.;strs2.;
f termdef
w
1,$d
E boolcap
$r numcap
1,$s;\(.*\), \(.*\), /\*.*\*/;#define \2	CUR \1;
$r termdef
f termdef
w
1,$d
a
Next use the xxxcap files to create xxxstr files with will form the
basis for the structure declarations in term.h.  The general form of
these lines is simply

	Name,   /* comment*/

the strstr file is not created, rather after the conversion to the
form shown above, all the other parts of term.h are merged with that
data and term.h is written.

.
1,$d
E boolcap
1,$s;\(.*,\) .*, \(/\*.*\*/\);	\1	\2;
$,$s/,/;/
w boolstr
E numcap
1,$s;\(.*,\) .*, \(/\*.*\*/\);	\1	\2;
$,$s/,/;/
w numstr
E strcap
1,$s;\(.*,\) .*, \(/\*.*\*/\);	\1	\2;
100,100s/,/;/
100a
};
struct strs2 {
    charptr
.
$,$s/,/;/
0r termdef
a

typedef char *charptr ;

struct strs {
    charptr
.
$a
} ;

struct term {
    char
.
r boolstr
a
    short
.
r numstr
a

	struct strs strs;
	struct strs2 strs2;
	short Filedes;		/* file descriptor being written to */
#ifndef NONSTANDARD
	SGTTY Ottyb,		/* original state of the terminal */
	      Nttyb;		/* current state of the terminal */
#endif
};
#ifndef NONSTANDARD
extern struct term *cur_term;
#endif
.
1i
/*
 * term.h - this file is automatically made from caps and maketerm.ex.
 *
 * Guard against multiple includes.
 */

#ifndef auto_left_margin

.
$a

#endif /* auto_left_margin */

#ifdef SINGLE
extern struct term _first_term;
# define CUR	_first_term.
#else
# define CUR	cur_term->
#endif
/*
 * SVR4 terminfo support
 */
#define no_esc_ctlc     beehive_glitch
#define dest_tabs_magic_smso    teleray_glitch
#define micro_char_size micro_col_size
.
f term.h
w
!rm boolstr numstr boolcap numcap strcap num str bool termdef
!rm strnames numnames boolnames
q

