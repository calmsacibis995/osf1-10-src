.\"
.\" *****************************************************************
.\" *                                                               *
.\" *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
.\" *                                                               *
.\" *   All Rights Reserved.  Unpublished rights  reserved  under   *
.\" *   the copyright laws of the United States.                    *
.\" *                                                               *
.\" *   The software contained on this media  is  proprietary  to   *
.\" *   and  embodies  the  confidential  technology  of  Digital   *
.\" *   Equipment Corporation.  Possession, use,  duplication  or   *
.\" *   dissemination of the software and media is authorized only  *
.\" *   pursuant to a valid written license from Digital Equipment  *
.\" *   Corporation.                                                *
.\" *                                                               *
.\" *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
.\" *   by the U.S. Government is subject to restrictions  as  set  *
.\" *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
.\" *   or  in  FAR 52.227-19, as applicable.                       *
.\" *                                                               *
.\" *****************************************************************
.\"
.\"
.\" HISTORY
.\"
.\" $XConsortium: r5.tbl.ms,v 1.3 91/04/23 16:34:30 keith Exp $
.EF 'Release 5 major changes'- % -'December 4, 1989'
.OF 'Release 5 major changes'- % -'December 4, 1989'
.EH ''''
.OH ''''
.TL
Major changes in Release 5 of
the X v11 Sample Server
.AU
Keith Packard
.AI
MIT X Consortium
.LP
This is an attempted summary of major changes to the sample server from
release 4 to release 5.  Unlike the last release, release 5 does not contain
a large number of seemingly arbitrary changes.  Porting an R4 based DDX to
R5 should take a matter of minutes to hours, depending on how much font
structure specific code your DDX contains.  Naturally, this document is
intended only as an aid to rather than a replacement for the server source
code.
.NH
Font Structure Changes
.XS
Font Structure Changes
.XE
.LP
This is the only area in the server which has seen major upheaval.  The
entire font mechanism has been redone from stem to stern.  If you are
fortunate to be using the mi text routines, and have simply implemented your
own glyph blt routines (as both mfb and cfb have always done), the change
is very minor.  In prior releases, the glyphs were passed to the rendering
routine in the old CharInfoRec, which contained the metrics of the glyph, an
offset from the base of the bitmaps and a boolean value telling whether this
glyph should be rendered at all.  The offset and boolean value were packed
together into a single 32 bit word, 24 bits for the offset and 8 bits for
the boolean.  Yuck.  R5 has redone this by simply placing a pointer to the
bitmap data in the structure; a NULL pointer has the obvious meaning of a
non-existant glyph.  So, the extra argument passed to the glyph blt routines
is no longer necessary; however I envision a future change to the interface
which will require an argument, so we'll leave it in for this release.
.LP
If you are not fortunate, and have implemented your own PolyText/ImageText
routines, you have your work cut out for you.  The font structures are all
new.  There are macros in include/dixfontstr.h which should help you figure
out what to change.  Good luck.
.NH
ClipNotify
.XS
ClipNotify
.XE
.LP
There used to be a hack in mivaltree.c to register interest in the changes
to a windows clip list.  This hack has been legitimized and is now a Screen
procedure.  When specified, this procedure is called whenever the clip list
to a window has changed.  It is called after the new clip list has been
installed on the window.
.NH
MI changes
.XS
MI changes
.XE
MI has remained relatively constant. One change is new algorithms for
rendering non-zero width arcs which are much easier to understand, and are
somewhat faster.  Other than that, the major changes involved integrating
the font structure changes into the text helper routines.
.NH
CFB changes
.XS
CFB changes
.XE
.LP
CFB has been largely rewritten to take advantage of some new algorithms
which make the source pool smaller while making rendering much faster in
general.  The only horrible change is that a small amount of assembly code
was written to speed up polytext rendering.  Assembly for sparc, mips and
68k processors exists; you should definately write routines for your
hardware as the old horrible C code has been ripped out, and the current
code is less than speedy.  By the time R5 ships, I hope to have made CFB
work on other than 8-bit displays; there is a small chance changes will be
made to allow it to be compiled multiple times for different depths,
allowing a poly-depth system to use cfb for all depths.
.LP
Reduced Rop is a system which allows a small number of functions to perform
all combinations of rasterop, foreground pixel and planemask.  By
precomputing two values ("xor" and "and"), and by recompiling source files
with different definitions, each raster op can run at full speed.  The basic
equation is simple:
.nf

	(dst & and) ^ xor

.fi
By proper computation of the two constants, this one equation performs all
rasterops (for example, Copy mode is simple: and = 0, xor = fg).  After
computing the two values, a "reduced rop" is computed.  The reduced rop can
be used to select different algorithms for rendering; in particular the most
common reduced rop is GXcopy - in this case, the rendering selected will
avoid fetching the destination pixel.  These reduced rops move the
complications of discovering "equivalent" rops (like GXinverted and GXxor in
many cases) into a single function.
.LP
Merged Rop is a system which performs bitblt rasterops in a single (rather
large) equation, avoiding branches at the expense of a bit of boolean
arithmetic.  Even with the additional computation, this should run faster
than the old code on any processor I can imagine.  The basic equation is:
.nf

	(dst & (src & and1 ^ xor1) ^ (src & and2 ^ xor2))

.fi
The values and? and xor? are computed at the time the function is started; a
precomputed array based on the raster op is merged with the planemask.  As
with the reduced rasterop code, special cases can be extracted to avoid some
or all of this computation.
.NH
MFB changes
.XS
MFB changes
.XE
.LP
Little was done in MFB other than integrating the merge-rop code into the
tiling and bitblt functions.
.LP
An interface change was made, however.  In r4 the function
mfbRegisterCopyPlaneProc was added to allow something other than miCopyPlane
to be called for mismatched depths.  Cfb used this function to speed up
CopyPlane from PSZ to 1 bit pixmaps.  In r4, calling this function was
optional, a screen/depth pair which didn't call was simply relegated to
using miCopyPlane.  In r5, this call has been made manditory.  This removes
the final reference to miCopyPlane from the server, eliminating the entire
mibitblt.c file from the image.  If you fail to call this routine when
needed, your server will core dump the first time you copy an n-bit object
to a bitmap.
