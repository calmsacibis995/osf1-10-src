.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.\"****************
.\" Font manipulation macros. Used to customize -mm for writing specs
.\" Macros defined:
.\" .Cw arg ...
.\"     constant-width (typewriter) font. Sets alternate arguments
.\"     in constant-width and current font. With no arguments, changes
.\"     to typewriter font. Similar to ".R", ".B" and ".I" macros in -mm.
.\" .Cb arg ...
.\"     constant-width bold font. same as .Cw, but uses constant-bold.
.fp 7 CW
.fp 8 CI
.fp 9 CB
.de Cw
.ie \\n(.$ \{.nr ;G \\n(.f
.    \" Arguments: begin alternating between this and previous font.
.    ds }I \\&\\f7\\$1\\fP\\$2
.    ie \\n(.$-2 \{\
.	\" 3 or more arguments:
.	\" if alternating with italic, provide extra 1/12 space
.	\" before 3rd and 5th arguments.
.	ds }i \\f7
.	if \\n(.f2 .ds }i \\f7\\^
.	ie !\\n(.$-3 \\*(}I\\*(}i\\$3
.	el \{.ie !\\n(.$-5 \\*(}I\\*(}i\\$3\\fP\\$4\\*(}i\\$5
.		el \\*(}I\\*(}i\\$3\\fP\\$4\\*(}i\\$5\\fP\\$6\}
'    br\}
.    \" 1 or 2 arguments:
.    el \\*(}I
.    \" any arguments: always restore previous font.
.    ft \\n(;G \}
.\" No arguments: change to new font.
.el .ft 7
.rm }I }i
.rr ;G
..
.de Cb
.ie \\n(.$ \{.nr ;G \\n(.f
.    \" Arguments: begin alternating between this and previous font.
.    ds }I \\&\\f9\\$1\\fP\\$2
.    ie \\n(.$-2 \{\
.	\" 3 or more arguments:
.	\" if alternating with italic, provide extra 1/12 space
.	\" before 3rd and 5th arguments.
.	ds }i \\f9
.	if \\n(.f2 .ds }i \\f9\\^
.	ie !\\n(.$-3 \\*(}I\\*(}i\\$3
.	el \{.ie !\\n(.$-5 \\*(}I\\*(}i\\$3\\fP\\$4\\*(}i\\$5
.		el \\*(}I\\*(}i\\$3\\fP\\$4\\*(}i\\$5\\fP\\$6\}
'    br\}
.    \" 1 or 2 arguments:
.    el \\*(}I
.    \" any arguments: always restore previous font.
.    ft \\n(;G \}
.\" No arguments: change to new font.
.el .ft 9
.rm }I }i
.rr ;G
..
