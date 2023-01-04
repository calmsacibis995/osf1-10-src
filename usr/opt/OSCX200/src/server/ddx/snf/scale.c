/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/snf/scale.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

/*
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "Xos.h"
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "dixfont.h"
#include "dixfontstr.h"
#include "font.h"
#include "fontstruct.h"
#include "snfstruct.h"
#include "os.h"
#include "osstruct.h"
#include "misc.h"
#include "fontdir.h"

#define MAXFNAMELEN 1024

extern FontFile FindFontFile();
extern Atom MakeAtom();

typedef struct _scalables {
    int pixel, point, x, y, width;
} Scalables;

enum scaleType {
    atom, pixel_size, point_size, resolution_x, resolution_y, average_width,
    scaledX, scaledY, unscaled, scaledXoverY, uncomputed
    };
	
typedef struct _fontProp {
    char 	       *name;
    Atom		atom;
    enum scaleType	type;
} fontProp;


static unsigned long fontGeneration=0;		/* initialization flag */

static FontTable scaledFonts = (FontTable)NULL;

static fontProp fontNamePropTable[] = {
    "FOUNDRY",			0,	atom,
    "FAMILY_NAME",		0,	atom,
    "WEIGHT_NAME",		0,	atom,
    "SLANT",			0,	atom,
    "SETWIDTH_NAME",		0,	atom,
    "ADD_STYLE_NAME",		0,	atom,
    "PIXEL_SIZE",		0,	pixel_size,
    "POINT_SIZE",		0, 	point_size,
    "RESOLUTION_X",		0,	resolution_x,
    "RESOLUTION_Y",		0,	resolution_y,
    "SPACING",			0,	atom,
    "AVERAGE_WIDTH",		0,	average_width,
    "CHARSET_REGISTRY",		0,	atom,
    "CHARSET_ENCODING",		0,	atom,
    "FONT",			0,	atom
};
#define NPROPS ((sizeof(fontNamePropTable) / sizeof(fontProp)) - 1)

static fontProp fontPropTable[] = {
    "MIN_SPACE",		0,	scaledX,
    "NORM_SPACE",		0,	scaledX,
    "MAX_SPACE",		0,	scaledX,
    "END_SPACE",		0,	scaledX,
    "AVG_CAPITAL_WIDTH",	0,	scaledX,
    "AVG_LOWERCASE_WIDTH",	0,	scaledX,
    "QUAD_WIDTH",		0,	scaledX,
    "FIGURE_WIDTH",		0,	scaledX,
    "SUPERSCRIPT_X",		0,	scaledX,
    "SUPERSCRIPT_Y",		0,	scaledY,
    "SUBSCRIPT_X",		0,	scaledX,
    "SUBSCRIPT_Y",		0,	scaledY,
    "SUPERSCRIPT_SIZE",		0,	scaledY,
    "SUBSCRIPT_SIZE",		0,	scaledY,
    "SMALL_CAP_SIZE",		0,	scaledY,
    "UNDERLINE_POSITION",	0,	scaledY,
    "UNDERLINE_THICKNESS",	0,	scaledY,
    "STRIKEOUT_ASCENT",		0,	scaledY,
    "STRIKEOUT_DESCENT",	0,	scaledY,
    "ITALIC_ANGLE",		0,	unscaled,
    "CAP_HEIGHT",		0,	scaledY,
    "X_HEIGHT",			0,	scaledY,
    "RELATIVE_SETWIDTH",	0,	unscaled,
    "RELATIVE_WEIGHT",		0,	unscaled,
    "WEIGHT",			0,	scaledXoverY,
    "DESTINATION",		0,	unscaled
};
    
static void initFontPropTable()
{
    int		i;
    fontProp *	t;

    i = sizeof(fontNamePropTable) / sizeof(fontProp);
    for (t=fontNamePropTable; i; i--, t++)
	t->atom = MakeAtom(t->name, (unsigned)strlen(t->name), TRUE);

    i  = sizeof(fontPropTable) / sizeof(fontProp);
    for (t=fontPropTable; i; i--, t++)
	t->atom = MakeAtom(t->name, (unsigned)strlen(t->name), TRUE);
}

static char *
GetInt(ptr, val)
    char *ptr;
    int *val;
{
    ptr++;
    if (*ptr == '*')
    {
	*val = -1;
	ptr++;
    }
    else
	for (*val = 0; *ptr >= '0' && *ptr <= '9'; )
	    *val = *val * 10 + *ptr++ - '0';
    if (*ptr == '-')
	return ptr;
    return (char *)NULL;
}

static Bool
ParseXLFDName(fname, vals, subst)
    char *fname;
    register Scalables *vals;
    char subst;
{
    register char *ptr;
    register char *ptr1, *ptr2, *ptr3, *ptr4;

    if (!(*fname == '-') ||				/* foundry */
	!(ptr = index(fname+1, '-')) ||			/* family_name */
	!(ptr = index(ptr+1, '-')) ||			/* weight_name */
	!(ptr = index(ptr+1, '-')) ||			/* slant */
	!(ptr = index(ptr+1, '-')) ||			/* setwidth_name */
	!(ptr = index(ptr+1, '-')) ||			/* add_style_name */
	!(ptr1 = ptr = index(ptr+1, '-')) ||		/* pixel_size */
	!(ptr = GetInt(ptr, &vals->pixel)) ||
	!(ptr = GetInt(ptr, &vals->point)) ||		/* point_size */
	!(ptr = GetInt(ptr, &vals->x)) ||		/* resolution_x */
	!(ptr2 = ptr = GetInt(ptr, &vals->y)) ||	/* resolution_y */
	!(ptr3 = ptr = index(ptr+1, '-')) ||		/* spacing */
	!(ptr4 = ptr = GetInt(ptr, &vals->width)) ||	/* average_width */
	!(ptr = index(ptr+1, '-')) ||			/* charset_registry */
	index(ptr+1, '-'))				/* charset_encoding */
	return FALSE;
    if (subst)
    {
	ptr = ptr1 + 1;
	*ptr++ = subst;
	*ptr++ = '-';
	*ptr++ = subst;
	*ptr++ = '-';
	*ptr++ = subst;
	*ptr++ = '-';
	*ptr++ = subst;
	bcopy(ptr2, ptr, ptr3 - ptr2 + 1);
	ptr += ptr3 - ptr2 + 1;
	*ptr++ = subst;
	strcpy(ptr, ptr4);
    }
    return TRUE;
}

static void
ComputeScaleFactors(from, to, dx, dy)
    Scalables *from, *to;
    double *dx, *dy;
{
    /* compute scale factors */
    *dy = ((double) to->point * to->y) / (from->point * from->y);
    *dx = (((double)(to->x * from->y)) / (to->y * from->x)) * *dy;
    if (to->width > 0)
	*dx = to->width / (from->width * *dx);
}

#define SCORE(m,s) \
if (m >= 1.0) { \
    if ((m == 1.0) || (m == 2.0)) \
	score += (4 * s); \
    else if (m < minfrac) \
	score += (1 * s); \
    else if (m < 2.0) \
	score += (3 * s); \
    else \
	score += (2 * s); \
} else { \
    m = 1/m; \
    if (m < minfrac) \
	score += (1 * s); \
    else \
	score += (2 * s); \
}

static Bool
FindBestToScale(fname, vals, best)
    char *fname;
    register Scalables *vals, *best;
{
    register FontPathPtr fpr;
    Scalables temp;
    int best_path;
    register int i;
    int best_score, score;
    double dx, dy, best_dx, best_dy, minfrac;

    /* find all matching fonts */
    fpr = ExpandFontNamePattern((unsigned)strlen(fname), fname, 65535);
    if (!fpr)
	return False;
    /* find the best match */
    best_score = 0;
    for (i = 0; i < fpr->npaths; i++)
    {
	if (!ParseXLFDName(fpr->paths[i], &temp, '\0'))
	    continue;
	if ((temp.pixel <= 0) || (temp.point <= 0) ||
	    (temp.x <= 0) || (temp.y <= 0))
	    continue;
	ComputeScaleFactors(&temp, vals, &dx, &dy);
	minfrac = (double)(3 * temp.pixel);
	minfrac = (minfrac + 4.0) / minfrac;
	score = 0;
	SCORE(dy, 10);
	SCORE(dx, 1);
	if ((score > best_score) ||
	    ((score == best_score) &&
	     ((dy < best_dy) || ((dy == best_dy) && (dx < best_dx))))) {
	    best_path = i;
	    best_score = score;
	    best_dx = dx;
	    best_dy = dy;
	    *best = temp;
	}	    
    }
    if (best_score)
	strcpy(fname, fpr->paths[best_path]);
    FreeFontRecord(fpr);
    return (best_score != 0);
}

static int computeProps(pf, npf, nprops, xfactor, yfactor)
    DIXFontProp *	pf;
    DIXFontProp *	npf;
    unsigned int	nprops;
    double		xfactor, yfactor;
{
    int		n;
    int 	count;
    fontProp*	t;

    for (count=0; nprops > 0; nprops--, pf++) {
	n = sizeof(fontPropTable) / sizeof(fontProp);
	for (t=fontPropTable; n && (t->atom != pf->name); n--, t++) ;
	if (!n) continue;

	switch (t->type) {
	  case scaledX:
	    npf->value = xfactor * pf->value;
	    break;
	  case scaledY:
	    npf->value = yfactor * pf->value;
	    break;
	  case unscaled:
	    npf->value = pf->value;
	    break;
	  case scaledXoverY:
	    npf->value = pf->value * (xfactor / yfactor);
	}
	npf->name = pf->name;
	npf++;
	count++;
    }
    return count;
}

static int ComputeScaledProperties(font, canonname, vals, dx, dy, 
				   scalename, tmpProps)
    FontPtr	font;		/* the font to be scaled */
    char *	canonname;	/* canonical name of the scaled font */
    Scalables	*vals;
    double	dx, dy;		/* scale factors in x and y directions */
    char *	scalename;	/* returns XLFD name of the scaled font */
    DIXFontProp *tmpProps;	/* returns properties; preallocated */
{
    int		n;
    unsigned	len;
    char 	*ptr1, *ptr2, *ptr3, *ptr4;
    DIXFontProp *fp;
    fontProp	*fpt;

    if (fontGeneration != serverGeneration) {
	initFontPropTable();
	fontGeneration = serverGeneration;
    }
    ptr2 = canonname;
    ptr4 = scalename;
    for (fp=tmpProps, fpt=fontNamePropTable, n=NPROPS; n; fp++, fpt++, n--) {

	*ptr4 = '-';
	ptr1 = ptr2+1;
	if (*ptr1 == '-')
	    ptr2 = ptr1;
	else {
	    if (n > 1)
		ptr2 = index(ptr1+1, '-');
	    else
		ptr2 = index(ptr1+1, '\0');
	}
	ptr3 = ptr4+1;

	switch (fpt->type) {
	  case atom:
	    len = ptr2 - ptr1;
	    strncpy(ptr3, ptr1, len);
	    fp->value = MakeAtom(ptr3, len, TRUE);
	    break;
	  case pixel_size:
	    sprintf(ptr3, "%d", vals->pixel);
	    len = strlen(ptr3);
	    fp->value = vals->pixel;
	    break;
	  case point_size:
	    sprintf(ptr3, "%d", vals->point);
	    len = strlen(ptr3);
	    fp->value = vals->point;
	    break;
	  case resolution_x:
	    sprintf(ptr3, "%d", vals->x);
	    len = strlen(ptr3);
	    fp->value = vals->x;
	    break;
	  case resolution_y:
	    sprintf(ptr3, "%d", vals->y);
	    len = strlen(ptr3);
	    fp->value = vals->y;
	    break;
	  case average_width:
	    sprintf(ptr3, "%d", vals->width);
	    len = strlen(ptr3);
	    fp->value = vals->width;
	    break;
	}
	ptr4 = ptr3 + len;
	fp->name = fpt->atom;
    }
    *ptr4 = '\0';
    fp->name = fpt->atom;
    fp->value = MakeAtom(scalename, (unsigned)(ptr4 - scalename), TRUE);

    n = NPROPS + 1;
    n += computeProps(font->pFP, tmpProps + n, font->pFI->nProps, dx, dy);
    return n;
}

/* 
 *  ScaleFont 
 *  returns a pointer to the new scaled font, or NULL (due to BadAlloc).
 */
static FontPtr
ScaleFont(opf, widthMult, heightMult, props, propCount)
    FontPtr	opf;			/* originating font */
    double	widthMult;		/* glyphs width scale factor */
    double	heightMult;		/* glyphs height scale factor */
    DIXFontProp* props;			/* some of the new properties */
    int		propCount;		/* count of new properties */
{
    FontPtr	pf;
    FontInfoPtr	pfi, opfi;
    CharInfoPtr pci, opci;
    int 	nchars;			/* how many characters in the font */
    int		newWidth, newHeight;
    unsigned    glyphBytesOffset;
    unsigned	bytestoalloc, bytestoink, bytestoprops, bytestoglyphs;
    char *	fontspace;
    int *	scratch;
    static void	ScaleBitmap();

    /* Allocate a new Font struct in one chunk of memory.
     *
     *  FontRec			pf
     *  FontInfoRec		pf->pFI
     *  CharInfoRec's		pf->pCI[i]
     *  Glyphs			pf->pGlyphs[i] 	not necessarily the same i
     *  DIX Properties		pf->pFP
     *  Ink CharInfoRec's	pf->pInkCI, pf->pInkMin, pf->pInkMax
     */
    opfi = opf->pFI;
    bytestoalloc = sizeof (FontRec);
    bytestoalloc += BYTESOFFONTINFO(opfi);
    bytestoalloc += BYTESOFCHARINFO(opfi);
    bytestoglyphs = bytestoalloc;
    nchars = n2dChars(opfi);
    for (opci=opf->pCI; nchars; opci++, nchars--)
	if (opci->exists) {
	    newWidth = GLYPHWIDTHPIXELS(opci) * widthMult;
	    newHeight = GLYPHHEIGHTPIXELS(opci) * heightMult;
	    bytestoalloc += PADGLYPHWIDTHBYTES(newWidth) * newHeight;
	}
    nchars = n2dChars(opfi);
    bytestoprops = bytestoalloc;
    bytestoalloc += propCount * sizeof (DIXFontProp);
    bytestoink = bytestoalloc;
    if (opfi->inkMetrics) bytestoalloc += BYTESOFINKINFO(opfi);

    /* allocate the new font structure */

    if (! (fontspace = (char *) xalloc(bytestoalloc)))
	return (FontPtr) NULL;

    /* set up pointers into the new font space */

    pf = (FontPtr) fontspace;
    pfi = pf->pFI = (FontInfoPtr) (fontspace + sizeof(FontRec));
    *pfi = *opfi;
    pfi->nProps = propCount;

    pf->pCI = ADDRCharInfoRec(pfi);
    pf->pGlyphs = (char *) (fontspace + bytestoglyphs);
    bzero(pf->pGlyphs, (bytestoprops - bytestoglyphs));

    /* Copy over the scaled XLFD properties */

    pf->pFP = (DIXFontProp *) (fontspace + bytestoprops);
    bcopy((char *)props, (char *)pf->pFP, propCount * sizeof(DIXFontProp));

    if (pfi->inkMetrics) {
	pf->pInkMin = (CharInfoPtr)(fontspace + bytestoink);
	pf->pInkMax = pf->pInkMin + 1;
	pf->pInkCI = pf->pInkMax + 1;
    } else {
	pf->pInkMin = &pfi->minbounds;
	pf->pInkMax = &pfi->maxbounds;
	pf->pInkCI = pf->pCI;
    }

    /* FontRec */

    pf->osPrivate = opf->osPrivate;
    pf->fileType = opf->fileType;
    pf->refcnt = 0;

    /* FontInfo
     *	fontDescent
     *  fontAscent
     *  minbounds
     *  maxbounds
     *  --  everything else was copied from the original font, above.
     */

    pfi->fontDescent *= heightMult;
    pfi->fontAscent  *= heightMult;
    pfi->minbounds.metrics.leftSideBearing = MAXSHORT;
    pfi->minbounds.metrics.rightSideBearing = MAXSHORT;
    pfi->minbounds.metrics.ascent = MAXSHORT;
    pfi->minbounds.metrics.descent = MAXSHORT;
    pfi->minbounds.metrics.characterWidth = opf->pCI->metrics.characterWidth;

    pfi->maxbounds.metrics.leftSideBearing = MINSHORT;
    pfi->maxbounds.metrics.rightSideBearing = MINSHORT;
    pfi->maxbounds.metrics.ascent = MINSHORT;
    pfi->maxbounds.metrics.descent = MINSHORT;
    pfi->maxbounds.metrics.characterWidth = opf->pCI->metrics.characterWidth;

    /* For each character, set the per-character metrics, scale the glyph,
     * and check per-font minbounds and maxbounds character information.
     */

    /* Allocate the scratch space for the glyph scaling routine. */
    if (! (scratch = (int *)
	ALLOCATE_LOCAL((int) ((opfi->maxbounds.metrics.rightSideBearing -
		       opfi->minbounds.metrics.leftSideBearing)
		      * widthMult * sizeof(int))))) { 
	xfree(fontspace);
	return (FontPtr) NULL;
    }

    glyphBytesOffset = 0;
    pci = pf->pCI;
    for (opci = opf->pCI;  nchars;  pci++, opci++, --nchars) {
	int		width, height;

	if (! (pci->exists = opci->exists))
	    continue;

	/* Determine current and new width and height */

	width = GLYPHWIDTHPIXELS(opci);
	height = GLYPHHEIGHTPIXELS(opci);
	newWidth = width * widthMult;
	newHeight = height * heightMult;

	/* Scale the glyph */

	ScaleBitmap(&(opf->pGlyphs[(int) opci->byteOffset]), width,
		    height, &(pf->pGlyphs[(int) glyphBytesOffset]), newWidth,
		    newHeight, scratch);

	pci->byteOffset = glyphBytesOffset;
	glyphBytesOffset += (PADGLYPHWIDTHBYTES(newWidth) * newHeight);

	/* Scale the character metrics */

	pci->metrics.leftSideBearing = (opci->metrics.leftSideBearing *
					widthMult);
	pci->metrics.rightSideBearing = newWidth +
	    pci->metrics.leftSideBearing;
	pci->metrics.ascent = opci->metrics.ascent * heightMult;
	pci->metrics.descent = newHeight - pci->metrics.ascent;
	pci->metrics.characterWidth = opci->metrics.characterWidth * widthMult;
	pci->metrics.attributes = opci->metrics.attributes;

#define MINMAX(field) \
	if (pfi->minbounds.metrics.field > pci->metrics.field) \
	    pfi->minbounds.metrics.field = pci->metrics.field; \
	if (pfi->maxbounds.metrics.field < pci->metrics.field) \
	    pfi->maxbounds.metrics.field = pci->metrics.field

        MINMAX(leftSideBearing);
	MINMAX(rightSideBearing);
	MINMAX(ascent);
	MINMAX(descent);
	MINMAX(characterWidth);
#undef MINMAX

    }
    pci = pf->pCI;
    DEALLOCATE_LOCAL((char *) scratch);

    /* Check and set ink metrics */
    if (pfi->inkMetrics) {
	CharInfoPtr pci;
	int n;
	n = (bytestoalloc - bytestoink) / sizeof(CharInfoRec);
	bcopy(opf->pInkMin, pf->pInkMin, n);
	for (pci=pf->pInkMin; n; n--, pci++) {
	    if (pci->exists) {
		pci->metrics.leftSideBearing *= widthMult;
		pci->metrics.rightSideBearing *= widthMult;
		pci->metrics.ascent *= heightMult;
		pci->metrics.descent *= heightMult;
		pci->metrics.characterWidth *= widthMult;
	    }
	}
    }
    return pf;
}

static int lcm(a, b)		/* least common multiple */
    int a, b;
{
    register int m;
    register int larger, smaller;

    m = larger = max(a, b);
    smaller =    min(a, b);
    
    while (m % smaller)
	m += larger;
    return m;
}

static void ScaleBitmap(bitmap, width, height,
		 newBitmap, newWidth, newHeight, scratch)
    char *	bitmap;		/* padded bitmap to be scaled */
    int		width;		/* unpadded width of bitmap in bits */
    int		height;		/* unpadded height of bitmap in bits */
    char *	newBitmap;	/* allocated, zero'ed, padded memory */
    int		newWidth;	/* unpadded width of new bitmap in bits */
    int		newHeight;	/* unpadded height of new bitmap in bits */
    int *	scratch;	/* scratch memory: newWidth * sizeof int */
{
    int		kcounter;	/* 0 <= kcounter <= k */
    int		lcounter;	/* 0 <= lcounter <= l */
    int		kkcounter;	/* 0 <= kkcounter <= kk */
    int		llcounter;	/* 0 <= llcounter <= ll */
    int		newBit;		/* newBitmap column index, by bits */
    int	*	acc;		/* pseudonym for scratch */
    int		bitValue;	/* tmp variable */
    int		dataByte;
    char *	dataBytePointer;
    char *	saveDataBytePointer;

    /* The following variables have constant values, once assigned. */

    int		virtualWidth;	/* unpadded bit width of the virtual bitmap */
    int		virtualHeight;	/* unpadded bit height of the virtual bitmap */
    int		k;		/* horizontal bit copies in virtualBitmap */
    int		l;		/* vertical bit copies in virtualBitmap */
    int		kk;     	/* horiz. virtual bits in a newBitmap bit */
    int		ll;     	/* vertical virtual bits in a newBitmap bit */
    int		threshold;
    int		padWidth;	/* actual width in bytes of pad in bitmap */
    int		newPadWidth;

    if (!newWidth || !newHeight)
	return;

    padWidth = PADGLYPHWIDTHBYTES(width) - ((width + 7) >> 3);
    newPadWidth = PADGLYPHWIDTHBYTES(newWidth) - ((newWidth + 7) >> 3);
    virtualWidth = lcm(width, newWidth);
    virtualHeight = lcm(height, newHeight);
    k = virtualWidth / width;
    l = virtualHeight / height;
    kk = virtualWidth / newWidth;
    ll = virtualHeight / newHeight;
    threshold = kk * ll;

    saveDataBytePointer = bitmap;
    newBitmap--;
    lcounter = l;
    for (; newHeight; newHeight--, newBitmap += newPadWidth) {

	newBit = newWidth;
	acc = scratch;
	do 
	    *acc++ = threshold;
	while (--newBit);

	llcounter = ll;
	while (llcounter) {
	    int		bit;	/* index into glyph row, indicating a bit */
	    int		row_dup;	/* row duplication count from source */
	    int		kdup;
	    int		kkdup;
	    
	    if (! lcounter) {
		lcounter = l;
		dataBytePointer += padWidth;
		saveDataBytePointer = dataBytePointer;
	    } else 
		dataBytePointer = saveDataBytePointer;

	    if ((row_dup = llcounter) > lcounter)
		row_dup = lcounter;
	    lcounter -= row_dup;
	    llcounter -= row_dup;
	    row_dup <<= 1;

	    bit = 1;
	    kdup = k * row_dup;
	    kkdup = kk * row_dup;
	    kcounter = 0;
	    newBit = newWidth;
	    acc = scratch;
	    do {
		int tmp = 0;
		kkcounter = kkdup;
		if (! kcounter) {
		    /* advance to the next column of the source bitmap */
		    kcounter = kdup;
		    if (! (--bit)) {
			bit = 8;
			dataByte = *dataBytePointer++;
		    }
		    /* extract the appropriate bit from source bitmap */
#if (BITMAP_BIT_ORDER == LSBFirst)
		    bitValue = dataByte & 1;
		    dataByte >>= 1;
#else
		    bitValue = dataByte & 128;
		    dataByte <<= 1;
#endif
		}
		while ((kkcounter -= kcounter) > 0) {
		    if (bitValue)
			tmp += kcounter;
		    /* advance to the next column of the source bitmap */
		    kcounter = kdup;
		    if (! (--bit)) {
			bit = 8;
			dataByte = *dataBytePointer++;
		    }
		    /* extract the appropriate bit from source bitmap */
#if (BITMAP_BIT_ORDER == LSBFirst)
		    bitValue = dataByte & 1;
		    dataByte >>= 1;
#else
		    bitValue = dataByte & 128;
		    dataByte <<= 1;
#endif
		}
		if (bitValue)
		    tmp += kcounter + kkcounter;
		kcounter = -kkcounter;
		*acc++ -= tmp;
	    }
	    while (--newBit);
	}
	/* Set the appropriate bits in newBitmap, based on the count of bits
	 * set in the virtual bitmap which map to a single bit in the new
	 * bitmap, and based on the knowing the constant number of bits in
	 * the virtual bitmap which map to a bit in the new bitmap.
	 */
	acc=scratch;

#if (BITMAP_BIT_ORDER == LSBFirst)
	bitValue=128;
#else
	bitValue=1;
#endif
	newBit = newWidth;
	do {
#if (BITMAP_BIT_ORDER == LSBFirst)
	    if ((bitValue <<= 1) == 256) {
		bitValue = 0x1;
		newBitmap++;
	    }
#else
	    if (! (bitValue >>= 1)) {
		bitValue = 128;
		newBitmap++;
	    }
#endif
	    if (*acc++ < 0)
		*newBitmap |= bitValue;
	} while (--newBit);
    }
}

/*
 *	exported interfaces
 */

FontFile
FindScaledFont(path, fontname, table)
    FontPathPtr path;
    char *fontname;
    FontTable *table;
{
#if (BITMAP_BIT_ORDER != IMAGE_BYTE_ORDER)
    return NullFontFile; /* XXX we don't deal with this case yet */
#else
    char fname[MAXFNAMELEN];
    FontFile ff;
    Scalables vals;
    register int i;
    Bool found;
    Scalables best;
    FontPtr font = NullFont;
    FontPtr scaled_font;
    double dx, dy;
    FontTable tmptab;
    DIXFontProp *tmpProps;
    int	propCount;
    char canonname[1024];
    char scalename[1024];

    *table = scaledFonts;
    /* first see if we already have it */
    i = FindNameInFontTable(scaledFonts, fontname, &found);
    if (found) {
	ff = &scaledFonts->file.ff[scaledFonts->name.fn[i].u.index];
	if (ff->private)
	    return ff;
    }
    strcpy(fname, fontname);
    /* make sure it is a valid XLFD name, and get canonical form */
    if (!ParseXLFDName(fname, &vals, '0') ||
	!vals.pixel || !vals.point || !vals.x || !vals.y || !vals.width ||
	((vals.pixel < 0) && (vals.point < 0)))
	return NullFontFile;
    if ((vals.point > 0) && (vals.y > 0))
    {
	best.pixel = (vals.point * vals.y * 10) / 7227;
	if (vals.pixel < 0)
	    vals.pixel = best.pixel;
	else if ((best.pixel < vals.pixel - 1) ||
		 (best.pixel > vals.pixel + 1))
	    return NullFontFile;
    }
    else if (vals.pixel > 0)
    {
	if (vals.point > 0)
	    vals.y = (vals.pixel * 7227) / (vals.point * 10);
	else if (vals.y > 0)
	    vals.point = (vals.pixel * 7227) / (vals.y * 10);
    }
    if (vals.x < 0)
	vals.x = vals.y;
    if (found) {
	/* get values of font to scale */
	if (!ParseXLFDName(ff->name, &best, '\0'))
	    return NullFontFile;
	strcpy(canonname, scaledFonts->name.fn[i].name);
    } else {
	/* make sure the canonical form exists */
	FontPathPtr fpr;

	fpr = ExpandFontNamePattern((unsigned)strlen(fname), fname, 1);
	if (!fpr)
	    return NullFontFile;
	if (!fpr->npaths)
	{
	    FreeFontRecord(fpr);
	    return NullFontFile;
	}
	strcpy(canonname, fpr->paths[0]);
	FreeFontRecord(fpr);
	ff = FindFontFile (path, canonname, 0, FALSE, &tmptab);
	if (!ff || !ff->alias)
	    return NullFontFile;
	strcpy(fname, ff->name);
	/* get the wildcard form */
	if (!ParseXLFDName(fname, &best, '*'))
	    return NullFontFile;
    }
    if (vals.y < 0)
    {
	if (best.y <= 0)
	    return NullFontFile;
	vals.y = best.y;
	if (vals.point < 0)
	    vals.point = (vals.pixel * 7227) / (vals.y * 10);
	else
	    vals.pixel = (vals.point * vals.y * 10) / 7227;
	if (vals.x < 0)
	    vals.x = vals.y;
    }
    if (found)
	font = FontFileLoad(ff->name, (unsigned)strlen(ff->name));
    else if (FindBestToScale(fname, &vals, &best))
	font = FontFileLoad(fname, (unsigned)strlen(fname));
    if (!font)
	return NullFontFile;
    ComputeScaleFactors(&best, &vals, &dx, &dy);
    if (vals.width < 0)
	vals.width = best.width * dx;
    /* prepare font properties for the new font */
    tmpProps = (DIXFontProp *) 
	ALLOCATE_LOCAL(sizeof(DIXFontProp) *
		       (sizeof(fontNamePropTable) / sizeof(fontProp) +
			sizeof(fontPropTable) / sizeof(fontProp)));
    if (!tmpProps)
	return (FontFile) NULL;
    propCount = ComputeScaledProperties(font, canonname, &vals, dx, dy,
					scalename, tmpProps);
    /* compute the scaled font */
    scaled_font = ScaleFont(font, dx, dy, tmpProps, propCount);
    DEALLOCATE_LOCAL(tmpProps);
    /* do not need the base font any more */
    if (!font->refcnt)
	FontUnload(font);
    if (!scaled_font)
	return NullFontFile;
    /* enter into the table if new */
    if (!found) {
	i = AddFileEntry(scaledFonts, fname, True);
	if (i >= 0) {
	    ff = &scaledFonts->file.ff[i];
	    i = AddVerifiedNameEntry(scaledFonts, scalename, i);
	}
	if (i <= 0) {
	    FontUnload(scaled_font);
	    return NullFontFile;
	}
    }
    ff->private = (Opaque) scaled_font;
    scaled_font->osPrivate = (pointer) ff;
    return ff;
#endif
}

void
FreeScaledFonts()
{
    register FontTable table = scaledFonts;
    register int i;
    register FontPtr font;

    if (table) {
	for (i = 0; i < table->file.used; i++) {
	    if (font = (FontPtr) table->file.ff[i].private)
		font->osPrivate = NULL;
	}
	FreeFontTable(table);
    }
    scaledFonts = MakeFontTable("", 10);
}

