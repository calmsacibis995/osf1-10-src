.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.\"***********************************
.\" Macro definitions for PIC inclusion.
.\" Taken from ~jeffc/Notes/pic.macros
.de PS  \" start picture (bwk)
.       \" $1 is height, $2 is width in units
.if t .sp .3
.nr 0j \\n(.u
.nr 1j \\$2
.ne \\$1u
.nf
.if n \{\
.	sp \\$1u/2u-1v
.	tl ''[Picture Omitted.]''
.	sp \\$1u/2u
.	di 3j\}
.in (\\n(.lu-\\n(1ju)/2u
..
.de PE  \" end of picture
.in
.if n \{\
.	di
.	rm 3j\}
.)R
.if \\n(0j .fi
.rr 0j 1j
.if t .sp .6
..
.\" End of PIC macros
.\" Beginning of Figure macros. Place lines above and below
.\" figure, and always keep on one page.
.\" .fS: Figure Start
.de fS
.br
.di fg
.br
\\l'\\n(.lu-\\n(.iu'
..
.\" .fC: Figure Cutline.
.de fC
.br
\\l'\\n(.lu-\\n(.iu'
.br
.)R
..
.\" .fE: Figure End.
.de fE
.br
.di
.nr 0j \\n(.u
.ne \\n(dnu
.nf
.fg
.if \\n(0j .fi
.rr 0j
.rm fg
..
.\" End of Figure Macros
.\"***********************************
