% @(#) cube.geo,v 1.1 90/06/12 16:46:23 howards Exp %
%
Copyright 1989, 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

						All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
%

begin_structure 1;

interior_color_index 2;
polygon3
	 -5  -5  -5
	  5  -5  -5
	  5   5  -5
	 -5   5  -5
;

interior_color_index 6;
polygon3
	 -5  -5   5
	  5  -5   5
	  5   5   5
	 -5   5   5
;

interior_color_index 3;
polygon3
	  5  -5  -5
	  5  -5   5
	  5   5   5
	  5   5  -5
;

interior_color_index 4;
polygon3
	 -5  -5  -5
	 -5  -5   5
	 -5   5   5
	 -5   5  -5
;

interior_color_index 5;
polygon3
	 -5   5  -5
	  5   5  -5
	  5   5   5
	 -5   5   5
;

interior_color_index 7;
polygon3
	 -5  -5  -5
	  5  -5  -5
	  5  -5   5
	 -5  -5   5
;

end_structure;
