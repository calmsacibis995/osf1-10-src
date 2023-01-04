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
#ifndef lint
static char     *sccsid = "@(#)$RCSfile: gen.c,v $ $Revision: 1.1.5.7 $ (DEC) $Date: 1993/12/20 21:32:50 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.11  com/cmd/nls/gen.c, cmdnls, bos320, 9137320a 9/4/91 13:44:10
 */

#include <sys/localedef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/method.h>
#include "semstack.h"
#include "symtab.h"
#include "locdef.h"
#include "err.h"

static void gen_extern(FILE *);
static void gen_ctype(FILE *, _LC_ctype_t *);
static void gen_charmap(FILE *, _LC_charmap_t *);
static void gen_collate(FILE *, _LC_collate_t *);
static void gen_monetary(FILE *, _LC_monetary_t *);
static void gen_time(FILE *, _LC_time_t *);
static void gen_numeric(FILE *, _LC_numeric_t *);
static void gen_msg(FILE *, _LC_resp_t *);
static void gen_locale(FILE *, _LC_charmap_t *, _LC_collate_t *,  
	   _LC_ctype_t *, _LC_monetary_t *, _LC_numeric_t *, _LC_resp_t *, 
	   _LC_time_t *);

wchar_t		max_wchar_enc;
extern int	max_disp_width;

static void gen_instantiate(FILE *);


/*
*  FUNCTION: fp_putstr
*
*  DESCRIPTION:
*  Standard print out routine for character strings in structure 
*  initialization data.
*/
static void
fp_putstr(FILE *fp, char *s)
{
    extern int	copying;	/* might be using internal, non-escaped, data */
    extern char yytext[];	/* use this cause its big */

    if (s == NULL)
	fprintf(fp, "\t\"\",\n");
    else {
	if (copying) {
	    char *bptr = yytext;
	    unsigned char c;

	    while ((c=*s++)) {
		if (c != '\\' && c != '"' && isascii(c) && isprint(c))
		    *bptr++ = c;
		else
		    bptr += sprintf(bptr, "\\x%02x", c);
	    }
	    *bptr = '\0';
	    fprintf(fp, "\t\"%s\",\n", yytext);
	} else
	    fprintf(fp, "\t\"%s\",\n", s);
    }
}

/*
*  FUNCTION: fp_putsym
*
*  DESCRIPTION:
*  Standard print out routine for symbols in structure initialization
*  data.
*/
static void 
fp_putsym(FILE *fp, char *s)
{
    if (s != NULL)
	fprintf(fp, "\t%s,\n", s);
    else
	fprintf(fp, "\t(void *)0,\n");
}


/*
*  FUNCTION: fp_putdig
*
*  DESCRIPTION:
*  Standard print out routine for integer valued structure initialization
*  data.
*/
static void fp_putdig(FILE *fp, int i)
{
    fprintf(fp, "\t%d,\n", i);
}

/*
*  FUNCTION: fp_puthdr
*
*  DESCRIPTION:
*  Standard print out routine for method headers.
*/
static void fp_puthdr(FILE *fp, int kind, size_t size)
{
     fprintf(fp, "\t{%d, _LC_MAGIC, _LC_VERSION, %d},\n", kind, size);
}


/*
*  FUNCTION: fp_putmeth
*
*  DESCRIPTION:
*  Standard print out routine for method references.
*/
static void fp_putmeth(FILE *fp, int i)
{
    fp_putsym(fp, METH_NAME(i));
}

/*
*  FUNCTION: gen_hdr
*
*  DESCRIPTION:
*  Generate the header file includes necessary to compile the generated
*  C code.
*/
static void gen_hdr(FILE *fp)
{
    fprintf(fp, "#include <nl_types.h>\n");
    fprintf(fp, "#include <locale.h>\n");
    fprintf(fp, "#include <sys/types.h>\n");
    fprintf(fp, "#include <sys/localedef.h>\n");
    fprintf(fp, "#include <regex.h>\n");
    fprintf(fp, "#include <sys/time.h>\n");

    fprintf(fp, "#if _LC_VERSION != %d\n#error \"%s\"\n#endif\n",
	    _LC_VERSION, msgstr(ERR_BAD_CHDR));
}


/* 
*  FUNCTION: gen
*
*  DESCRIPTION:
*  Common entry point to code generation.  This function calls each of the
*  functions in turn which generate the locale C code from the in-memory
*  tables built parsing the input files.
*/
void gen(FILE *fp)
{
    extern _LC_charmap_t  charmap;
    extern _LC_collate_t  *collate_ptr;
    extern _LC_ctype_t    *ctype_ptr;
    extern _LC_monetary_t *monetary_ptr;
    extern _LC_numeric_t  *numeric_ptr;
    extern _LC_time_t     *lc_time_ptr;
    extern _LC_resp_t     *resp_ptr;

    gen_hdr(fp);
    gen_extern(fp);
    gen_charmap(fp, &charmap);
    gen_ctype(fp, ctype_ptr);
    gen_collate(fp, collate_ptr);
    gen_monetary(fp, monetary_ptr);
    gen_numeric(fp, numeric_ptr);
    gen_time(fp, lc_time_ptr);
    gen_msg(fp, resp_ptr);
    gen_locale(fp, &charmap, collate_ptr, ctype_ptr, 
	       monetary_ptr, numeric_ptr, resp_ptr, lc_time_ptr );
    gen_instantiate(fp);
    
}


/* 
*  FUNCTION: gen_extern
*
*  DESCRIPTION:
*  This function generates the externs which are necessary to reference the
*  function names inside the locale objects.  
*/
static void gen_extern(FILE *fp)
{
  int i;
  char *s;
  char *p;		/* Prototype format string */

  for (i=0; i<=LAST_METHOD; i++) {
      s = METH_NAME(i);
      if (s != NULL) {
	  p = METH_PROTO(i);

	  if(p && *p) {			/* There is a prototype string */
	      fprintf(fp, "extern ");
	      fprintf(fp, p, s);
	      fputs(";\n",fp);
	  } else
	    fprintf(fp, "extern %s();\n", s);
      }
  }
}


/* 
*  FUNCTION: gen_charmap
*
*  DESCRIPTION:
*  This function generates the C code which implements a _LC_charmap_t
*  data structure corresponding to the in memory charmap build parsing the
*  charmap sourcefile.
*/
static void gen_charmap(FILE *fp, _LC_charmap_t *lc_cmap)
{

  fprintf(fp, 
"/*------------------------- CHARMAP OBJECT  -------------------------*/\n");
  fprintf(fp, "static _LC_charmap_t lc_cmap = {{\n");

  /* class core */
  fp_puthdr(fp, _LC_CHARMAP, sizeof(_LC_charmap_t) );
  fp_putmeth(fp, CHARMAP_NL_LANGINFO);
  fp_putmeth(fp, CHARMAP_MBTOWC);
  fp_putmeth(fp, CHARMAP_MBSTOWCS);
  fp_putmeth(fp, CHARMAP_WCTOMB);
  fp_putmeth(fp, CHARMAP_WCSTOMBS);
  fp_putmeth(fp, CHARMAP_MBLEN);
  fp_putmeth(fp, CHARMAP_WCSWIDTH);
  fp_putmeth(fp, CHARMAP_WCWIDTH);
  fp_putmeth(fp, CHARMAP___MBTOPC);
  fp_putmeth(fp, CHARMAP___MBSTOPCS);
  fp_putmeth(fp, CHARMAP___PCTOMB);
  fp_putmeth(fp, CHARMAP___PCSTOMBS);
  fp_putmeth(fp, CHARMAP_CHARMAP_INIT);
  fp_putsym(fp, 0);
   fprintf(fp,"\t},\t/* End core */\n");

  /* class extension */
  fp_putstr(fp, lc_cmap->cm_csname);
  fp_putdig(fp, mb_cur_max);
  fp_putdig(fp, 1);
  fp_putdig(fp, max_disp_width);
  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: compress_mask
*
*  DESCRIPTION:
*  Take all masks for codepoints above 255 and assign each unique mask
*  into a secondary array.  Create an unsigned byte array of indices into
*  the mask array for each of the codepoints above 255.
*/
static int compress_masks(_LC_ctype_t *ctype)
{
  static int nxt_idx = 1;
  int    umasks;
  unsigned char *qidx;
  unsigned int	*qmask;
  int i, j;

  if (ctype->_mask == NULL)
      return 0;

  /* allocate memory for masks and indices */
  ctype->qidx = qidx = calloc(max_wchar_enc - 256 +1, sizeof(unsigned char));
  ctype->qmask = qmask = MALLOC(unsigned int, 256);
  
  umasks = 1;
  for (i=256; i<= max_wchar_enc; i++) {      /* for each codepoint > 255 */
    /* 
      Search for a mask in the 'qmask' array which matches the mask for
      the character corresponding to 'i'.
    */
    for (j=0; j < umasks; j++) 
      if (qmask[j] == ctype->_mask[i]) {
	/* mask already exists, place index in qidx for character */
	qidx[i-256] = j;	
	break;
      }

    if (j==umasks) {

      /* couldn't find mask which would work, so add new mask to 'qmask' */

      qidx[i-256] = nxt_idx;
      qmask[nxt_idx] = ctype->_mask[i];

      nxt_idx++;
      umasks++;
    }

    /* only support 256 unique masks for multi-byte characters */
    if (nxt_idx >= 256) INTERNAL_ERROR;
  }

  return nxt_idx;
}


/* 
*  FUNCTION: gen_ctype
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_ctype_t locale
*  data structures.  These data structures include _LC_classnms_t,
*  an array of wchars for the upper and lower case translations,
*  and the container class _LC_ctype_t itself.
*/
#define N_PER_LINE 8
static void gen_ctype(FILE *fp, _LC_ctype_t *lc_ctype)
{
  int i, j;
  int n_idx;

#ifndef _NO_GCC_HACK
  int line_count;
#endif

  fprintf(fp, 
"/*------------------------- CHARACTER CLASSES -------------------------*/\n");

  fprintf(fp, "static const _LC_classnm_t classes[] = {\n");
  for (i=0; i<lc_ctype->nclasses;i++) {
    fprintf(fp, "\"%s\",\t0x%04x,\n",
	    lc_ctype->classnms[i].name, 
	    lc_ctype->classnms[i].mask);
  }
  fprintf(fp, "};\n\n");

  fprintf(fp, 
"/*------------------------- UPPER XLATE -------------------------*/\n");

  fprintf(fp,"static const wchar_t upper[] = {\n");

  /*
   * if upper table not set, set default upper values from 
   * 0 to max_wchar_enc, encoding = value, except for a-z = A-Z
   */
  if (!copying_ctype)
      lc_ctype->max_upper = max_wchar_enc;

  if (lc_ctype->_upper == NULL) {
      lc_ctype->_upper = MALLOC(wchar_t, max_wchar_enc+1);
      for (i=0; i <= max_wchar_enc; i++)
	if ((i >= 'a') && (i <= 'z'))
	  lc_ctype->_upper[i] = i + ('A' - 'a');
	else
	  lc_ctype->_upper[i] = i;
  }

  /*
   * Search the upper translation for the last character that is case sensitive
   */
  for (i=lc_ctype->max_upper; i>0; i-- )
      if (i != lc_ctype->_upper[i])
	  break;

  lc_ctype->max_upper = i;

  /* Check to see if there is upper value greater than max_upper */
  for (; i >= 0; i--)
      if (lc_ctype->max_upper < lc_ctype->_upper[i])
	  lc_ctype->max_upper = lc_ctype->_upper[i];


#ifndef _NO_GCC_HACK
  line_count = 0;
#endif
  for (i=0; i<=lc_ctype->max_upper; i+=N_PER_LINE) {
    for (j=0; j < N_PER_LINE && i+j <= max_wchar_enc; j++)
      fprintf(fp, "0x%04x, ", lc_ctype->_upper[i+j]);

    fprintf(fp, "\n");

#ifndef _NO_GCC_HACK
    line_count += N_PER_LINE;
    if ((line_count % 512) == 0) {
	fprintf(fp,"};\n");
	fprintf(fp,"static const wchar_t upper_%d[] = {\n", line_count);
    }
#endif
  }
  fprintf(fp, "};\n\n");
  
  fprintf(fp, 
"/*------------------------- LOWER XLATE -------------------------*/\n");
  fprintf(fp,"static const wchar_t lower[] = {\n");
  if (lc_ctype->_lower == NULL) {
      wchar_t *lp = calloc(sizeof(wchar_t), lc_ctype->max_upper+1);

      /* tolower unspecified, make it the inverse of toupper */
      lc_ctype->_lower = lp;

      /* assign identity */
      for (i=0; i<=lc_ctype->max_upper; i++)
	  lp[i] = i;
      
      /* assign inverse */
      for (i=0; i<=lc_ctype->max_upper; i++)
	  lp[lc_ctype->_upper[i]] = i;
      lc_ctype->max_lower = lc_ctype->max_upper;
  } else {
    if (!copying_ctype)
      lc_ctype->max_lower = max_wchar_enc;
  }

  /*
   * Search the lower translation table for the last character that's case sensitive
   */
  for (i=lc_ctype->max_lower; i>0; i-- )
      if (i != lc_ctype->_lower[i] )
	  break;

  lc_ctype->max_lower = i;

#ifndef _NO_GCC_HACK
  line_count = 0;
#endif

  for (i=0; i<=lc_ctype->max_lower; i+=N_PER_LINE) {
      for (j=0; j < N_PER_LINE && i+j <= max_wchar_enc; j++)
	  fprintf(fp, "0x%04x, ", lc_ctype->_lower[i+j]);

      fprintf(fp, "\n");

#ifndef _NO_GCC_HACK
      line_count += N_PER_LINE;
      if ((line_count % 512) == 0) {
	  fprintf(fp,"};\n");
	  fprintf(fp,"static const wchar_t lower_%d[] = {\n", line_count);
      }
#endif

  }
  fprintf(fp, "};\n\n");

  fprintf(fp, 
"/*------------------------- CHAR CLASS MASKS -------------------------*/\n");

  /* 
    print the data for the standard linear array of class masks.
  */
  fprintf(fp,"static const unsigned int masks[] = {\n");
  for (i=0; i < 256; i+=N_PER_LINE) {

    for (j=0; j < N_PER_LINE && i+j < 256; j++)
      fprintf(fp, "0x%04x, ", 
	      ((lc_ctype->_mask!=NULL)?(lc_ctype->_mask[i+j]):0));

    fprintf(fp, "\n");
  }
  fprintf(fp, "};\n\n");

  /* 
    If there are more than 256 codepoints in the codeset, the
    implementation attempts to compress the masks into a two level
    array of indices into masks.
  */
  if ((max_wchar_enc > 255) && !copying_ctype) {
    n_idx = compress_masks(lc_ctype);

    /*
     * Now compress the qidx array.
     */
    for (i=max_wchar_enc; i>256; i--)
      if (lc_ctype->qidx[i-256] != lc_ctype->qidx[i-257] )
	break;

    lc_ctype->qidx_hbound = i;

    /* Print the index array 'qidx' */
    fprintf(fp, "static const unsigned char qidx[] = {\n");
#ifndef _NO_GCC_HACK
    line_count = 0;
#endif
    for (i=256; i <= lc_ctype->qidx_hbound; i+=N_PER_LINE) {
    
      for (j=0; j<N_PER_LINE && i+j <= lc_ctype->qidx_hbound; j++)
	fprintf(fp, "0x%02x, ", lc_ctype->qidx[i+j-256]);
    
      fprintf(fp, "\n");

#ifndef _NO_GCC_HACK
      line_count += N_PER_LINE;

      if ((line_count % 512) == 0) {
	  fprintf(fp,"};\n");
	  fprintf(fp,"static const unsigned char qidx_%d[] = {\n", line_count);
      }
#endif

    }
    fprintf(fp, "};\n\n");
  
    /* Print the mask array 'qmask' */
    fprintf(fp, "static const unsigned int qmask[] = {\n");
    for (i=0; i<n_idx; i+= N_PER_LINE) {
 
      for (j=0; j < N_PER_LINE && i+j < n_idx; j++)
	fprintf(fp, "0x%04x, ", lc_ctype->qmask[i+j]);
      
    }
    fprintf(fp, "};\n\n");
  } else
    n_idx =0;
	     
  fprintf(fp, 
"/*-------------------------   CTYPE OBJECT   -------------------------*/\n");
  fprintf(fp,"static _LC_ctype_t lc_ctype = {{\n");

  /* class core */
  fp_puthdr(fp, _LC_CTYPE, sizeof(_LC_ctype_t));
  fp_putmeth(fp, CTYPE_TOWUPPER);
  fp_putmeth(fp, CTYPE_TOWLOWER);
  fp_putmeth(fp, CTYPE_WCTYPE);
  fp_putmeth(fp, CTYPE_ISWCTYPE);
  fp_putmeth(fp, CTYPE_CTYPE_INIT);
  fp_putsym(fp, 0);

  fprintf(fp,"\t},\t/* End core */\n");

  /* class extension */
  /* max and min process code (required by toupper, et al) */
  fp_putdig(fp, 0);
  fp_putdig(fp, max_wchar_enc);
  fp_putdig(fp, lc_ctype->max_upper);
  fp_putdig(fp, lc_ctype->max_lower);

  /* case translation arrays */
  fp_putsym(fp, "upper");
  fp_putsym(fp, "lower");
  fp_putsym(fp, "masks");

  if (n_idx > 0) {
    fp_putsym(fp, "qmask");
    fp_putsym(fp, "qidx");
  } else {
    fp_putsym(fp, 0);
    fp_putsym(fp, 0);
  }
  fp_putdig(fp, lc_ctype->qidx_hbound);

  fprintf(fp, "\t%d,\n", lc_ctype->nclasses);
  fp_putsym(fp, "classes");
  fprintf(fp, "};\n\n");
}

static int
coll_eq( wchar_t last, wchar_t prev )
{
    wchar_t w;

    if (last == IGNORE )
      return (prev == IGNORE);
    else if (last == UNDEFINED )
      return (prev == UNDEFINED);

    w = prev + 1;			/* This breaks with >>65k codepoints! */
    if ( (w & 0xff) == 0) w += 1;

    return (last == w);
}

/*
*  FUNCTION: compress_collate
*
*  DESCRIPTION:
*	Search the collation table and eliminate duplicate entries from the
*	"tail"
*/
static void compress_collate( _LC_collate_t *coll)
{
    int	i,j;

    /*
     * Check that collation table does exist
     */
    if ( !coll->co_coltbl )
        return;

    /*
     * Reverse search table looking for first code-point that has different
     * collation information.  Since the trailing entries could be assigned
     * explicit adjacent values -or- be IGNORED, it's necessary to check for
     * these as special cases.
     */

    for ( i=max_wchar_enc; i>0; i--) {

	if ( coll->co_coltbl[i].ct_collel != coll->co_coltbl[i-1].ct_collel )
	    /*
	     * Different string substitution symbols
	     */
	    goto nomatch;

	for (j = 0; j<=coll->co_nord; j++) {
	    if ( coll->co_nord < _COLL_WEIGHTS_INLINE ) {
		if ( !coll_eq( coll->co_coltbl[i].ct_wgt.n[j],
			      coll->co_coltbl[i-1].ct_wgt.n[j]))
		    goto nomatch;
	     } else {
		 if ( !coll_eq( coll->co_coltbl[i].ct_wgt.p[j],
			        coll->co_coltbl[i-1].ct_wgt.p[j]))
		     goto nomatch;
	     }
	}
    }

    /*
     * At this point, "i" indexes the last shared collation weight
     */
nomatch:
    coll->co_hbound = i;
    return;
}

/*
*  FUNCTION: coll_orders_eq
*
*  DESCRIPTION:
*  Compare two collation orders.  Return 1 if equal, 0 if not.
*/
static int coll_orders_eq(_LC_collate_t *coll, int order1, int order2)
{
    int	i;
    _LC_collel_t *ce;

    for (i = 0; i <= max_wchar_enc; i++) {
	if (coll->co_nord < _COLL_WEIGHTS_INLINE) {
	    if (coll->co_coltbl[i].ct_wgt.n[order1] !=
		coll->co_coltbl[i].ct_wgt.n[order2])
		return 0;

	    if (coll->co_coltbl[i].ct_collel != NULL) {
		for (ce = coll->co_coltbl[i].ct_collel;
		     ce->ce_sym != NULL;
		     ce++) {
		    if (ce->ce_wgt.n[order1] !=
			ce->ce_wgt.n[order2])
			return 0;
		}
	    }
	}
	else {
	    if (coll->co_coltbl[i].ct_wgt.p[order1] !=
		coll->co_coltbl[i].ct_wgt.p[order2])
		return 0;

	    if (coll->co_coltbl[i].ct_collel != NULL) {
		for (ce = coll->co_coltbl[i].ct_collel;
		     ce->ce_sym != NULL;
		     ce++) {
		    if (ce->ce_wgt.p[order1] !=
			ce->ce_wgt.p[order2])
			return 0;
		}
	    }
	}
    }

    return 1;
}

static void fp_putcolflag(FILE *fp, int flags)
{

#define PF(sym) ((flags & (sym)) ? #sym : "0")

    fprintf(fp, "%s | %s | %s | %s | %s,",
	    PF(_COLL_FORWARD_MASK),
	    PF(_COLL_BACKWARD_MASK),
	    PF(_COLL_NOSUBS_MASK),
	    PF(_COLL_POSITION_MASK),
	    PF(_COLL_CHAR_ORDER_MASK));

#undef PF
}
/*
* FUNCTION: fp_putsubsflag
*
*	Prints the collating substitution flags symbolically
*/
static void fp_putsubsflag(FILE *fp, int flags)
{
    switch (flags) {
      case (_SUBS_ACTIVE|_SUBS_REGEXP):
				fputs("_SUBS_REGEXP|", fp);	/*DROP THRU */
      case _SUBS_ACTIVE:	fputs("_SUBS_ACTIVE,", fp);	break;

      case _SUBS_REGEXP:	fputs("_SUBS_REGEXP,", fp);	break;
      default:			fputs("0,", fp);
    }
}

/* 
*  FUNCTION: gen_collate
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_collate_t locale 
*  data structure.
*/
static void gen_collate(FILE *fp, _LC_collate_t *coll)
{
    int i, j, k;
    int  num_wgts;
    wchar_t *wgts;

    if (!copying_collate)	/* if we are copying, its already compressed */
	compress_collate(coll);

    /*
      Don't bother generating code for the character order if it's the
      same as the last collation order.
    */
    num_wgts = coll->co_nord;
    if (!copying_collate)
	if ((num_wgts > 0) && coll_orders_eq(coll, num_wgts - 1, num_wgts))
	    coll->co_nord--;

    /* 
      Generate local definitions of _LC_coltbl_t, _LC_collel_t, and
      _LC_weight_t to handle the non-default side of the _LC_weight_t union.
      This is necessary to allow auto-initialization of the data types.
    */
    
    /* lc_weight_t */
    if (coll->co_nord < _COLL_WEIGHTS_INLINE) {
      /* you need to make this a union in order to get the correct data
	 alignment from the compiler */
      fprintf(fp, "typedef union {\n\twchar_t x[%d];\n\tconst wchar_t *p;\n"
	      , _COLL_WEIGHTS_INLINE);
    } else
	fprintf(fp, "typedef struct { const wchar_t *p;\n");

    fprintf(fp, "} lc_weight_t;\n");

    /* lc_collel_t */
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "\tchar *ce_sym;\n\tlc_weight_t ce_wgt;\n");
    fprintf(fp, "} lc_collel_t;\n");

    /* lc_coltbl_t */
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "\tlc_weight_t ct_wgt;\n\tconst lc_collel_t *ct_collel;\n");
    fprintf(fp, "} lc_coltbl_t;\n");

    /* lc_subs_t */
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "\tlc_weight_t ss_act;\n");
    fprintf(fp, "\tchar *ss_src;\n");
    fprintf(fp, "\tchar *ss_tgt;\n");
    fprintf(fp, "} lc_subs_t;\n\n");
    
    /* lc_collate_t */
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "\t_LC_core_collate_t core;\n");
    fprintf(fp, "\tunsigned char co_nord;\n");
    fprintf(fp, "\tlc_weight_t co_sort;\n");
    fprintf(fp, "\twchar_t co_wc_min;\n");
    fprintf(fp, "\twchar_t co_wc_max;\n");
    fprintf(fp, "\twchar_t co_hbound;\n");
    fprintf(fp, "\twchar_t co_col_min;\n");
    fprintf(fp, "\twchar_t co_col_max;\n");
    fprintf(fp, "\tconst lc_coltbl_t *co_coltbl;\n");
    fprintf(fp, "\tunsigned  char co_nsubs;\n");
    fprintf(fp, "\tconst lc_subs_t *co_subs;\n");
    fprintf(fp, "} lc_collate_t;\n");

    /*
      Generate code to implement collation elements.  
    */

    fprintf(fp,
"/*------------------------- COLLELLS --------------------------------*/\n");

    /* 
      Generate weights arrays for orders >= _COLL_WEIGHTS_INLINE 
    */
    if (coll->co_coltbl != NULL && coll->co_nord >= _COLL_WEIGHTS_INLINE) {
	for (i=0; i<=coll->co_hbound; i++) {

	    /* if there are any collation elements beginning with i */
	    if (coll->co_coltbl[i].ct_collel != NULL) {
	    
		_LC_collel_t *ce;

		fprintf(fp, "static const wchar_t cew%d[][%d]={ ", i, coll->co_nord+1);

		for (j=0, ce=(_LC_collel_t *)&(coll->co_coltbl[i].ct_collel[j]); 
		     ce->ce_sym != NULL; 
		     ce= (_LC_collel_t *)&(coll->co_coltbl[i].ct_collel[++j])) {

		    /* 
		      Create collation weights for this collation element 
		    */
		    for (k=0; k<=coll->co_nord; k++)
			fprintf(fp, "%d, ", ce->ce_wgt.p[k]);

		    fprintf(fp, "\n");
		}
		fprintf(fp, "};\n");
	    }
	}
    }

    /*
      Generate collation elements
    */
    if (coll->co_coltbl != NULL) {
	for (i=0; i<=coll->co_hbound; i++) {

	    /* if there are any collation elements beginning with i */
	    if (coll->co_coltbl && coll->co_coltbl[i].ct_collel != NULL) {
	    
		_LC_collel_t *ce;
	    
		fprintf(fp, "static const lc_collel_t ce%d[]={\n", i);

		/* one entry for each collation elementing beginning with i */
		for (j=0, ce=(_LC_collel_t *)&(coll->co_coltbl[i].ct_collel[j]); 
		     ce->ce_sym != NULL; 
		     ce = (_LC_collel_t *)&(coll->co_coltbl[i].ct_collel[++j])) {

		    fprintf(fp, "{ \"%s\", ", ce->ce_sym);
		    if (coll->co_nord >= _COLL_WEIGHTS_INLINE) 
		      fprintf(fp, "{&cew%d[%d][0]} },\n", i, j);
		    else {
			int k;
			fprintf(fp, "{ ");
			wgts = (num_wgts < _COLL_WEIGHTS_INLINE) ?
			  ce->ce_wgt.n : ce->ce_wgt.p;
			for (k=0;k<=coll->co_nord; k++)
			  fprintf(fp, "%d, ", wgts[k]);
			while (k++ < _COLL_WEIGHTS_INLINE)
			  fprintf(fp, "%d, ", 0);
			fprintf(fp, "} },\n");
		    }
		}
		if (coll->co_nord >= _COLL_WEIGHTS_INLINE)
		    fprintf(fp, "{ 0, 0 },\n");
		else
		    fprintf(fp, "{ 0, { 0, 0 } },\n"); 

		fprintf(fp, "};\n");
	    }
	}
    }

    /*
      If the number of orders is >= _COLL_WEIGHTS_INLINE, then generate array
      of collation table weights
    */
    if (coll->co_nord >= _COLL_WEIGHTS_INLINE) {
	fprintf(fp,
"/*------------------------- COLLTBL WEIGHTS -------------------------*/\n");

	fprintf(fp, "static const wchar_t ct_wgts[][%d]={\n", coll->co_nord+1);
	for (i=0; i<=coll->co_hbound; i++) {
	    
	    for (j=0; j<=coll->co_nord; j++)
		fprintf(fp, "%3d, ", coll->co_coltbl[i].ct_wgt.p[j]);

	    fprintf(fp, "\n");

	}
	fprintf(fp, "};\n\n");
    }


    fprintf(fp,
"/*------------------------- COLLTBL ---------------------------------*/\n");
    if (coll->co_coltbl != NULL) {
#ifndef _NO_GCC_HACK	
	int line_count = 0;
#endif
	fprintf(fp, "static const lc_coltbl_t colltbl[] ={\n");

	for (i=0; i<=coll->co_hbound; i++) {
	    _LC_coltbl_t *ct;

#ifndef _NO_GCC_HACK
	    line_count++;
	    if ((line_count % 512) == 0) {
		fprintf(fp, "};\n");
		fprintf(fp, 
			"static const lc_coltbl_t colltbl_%d[] ={\n",
			line_count);
	    }
#endif
	    ct = (_LC_coltbl_t *) &(coll->co_coltbl[i]);

	    /* generate weight data */
	    if (coll->co_nord < _COLL_WEIGHTS_INLINE) {
	        fprintf(fp, "{ {{ ");
		wgts = (num_wgts < _COLL_WEIGHTS_INLINE) ?
		  ct->ct_wgt.n : ct->ct_wgt.p;
		for (j=0; j<=coll->co_nord; j++)
		  fprintf(fp, "%d, ", wgts[j]);
		while (j++ < _COLL_WEIGHTS_INLINE)
		  fprintf(fp, "%d, ", 0);
		fprintf(fp, "}}, ");
	    } else
		fprintf(fp, "{ ct_wgts[%d], ", i);
		
	    /* generate collation elements if present */
	    if (ct->ct_collel != NULL)
		fprintf(fp, "ce%d },\n", i);
	    else
		fprintf(fp, "0 },\n");
	}
	fprintf(fp, "};\n");
    }
    
    if (coll->co_coltbl != NULL && coll->co_nsubs > 0) {
	fprintf(fp,
"/*------------------------- SUBSTITUTION STR-------------------------*/\n");

	/* 
	  generate substitution action arrays
	*/
	if (coll->co_nord >= _COLL_WEIGHTS_INLINE && coll->co_nsubs >0) {
	    fprintf(fp, "static const wchar_t subs_wgts[][%d]={\n", coll->co_nord+1);
	    for (i=0; i<coll->co_nsubs; i++) {
		for (j=0; j<=coll->co_nord; j++)
		    fp_putsubsflag( fp, coll->co_subs[i].ss_act.p[j] );

		fprintf(fp, "\n");
	    }
	    fprintf(fp, "};\n");
	}
			
	fprintf(fp, "static const lc_subs_t substrs[] = {\n");
	for (i=0; i<coll->co_nsubs; i++) {

	    if (coll->co_nord < _COLL_WEIGHTS_INLINE) {
		fprintf(fp, "\t{ {");
		wgts = (num_wgts < _COLL_WEIGHTS_INLINE) ?
		    coll->co_subs[i].ss_act.n : coll->co_subs[i].ss_act.p;
	        for (j=0; j<=coll->co_nord; j++)
		    fp_putsubsflag( fp, wgts[j] );

		fprintf(fp, "}, \"%s\", \"%s\" },\n", coll->co_subs[i].ss_src,
			coll->co_subs[i].ss_tgt );
	    }
	    else
		fprintf(fp, "\t{ subs_wgts[%d], \"%s\", \"%s\" },\n", 
			i,
			coll->co_subs[i].ss_src, 
			coll->co_subs[i].ss_tgt);
	}
	fprintf(fp, "};\n\n");
    }
    
    fprintf(fp,
"/*------------------------- COLLATE OBJECT  -------------------------*/\n");
    /*
      Generate sort order weights if necessary 
    */
    if (coll->co_coltbl != NULL && coll->co_nord >= _COLL_WEIGHTS_INLINE) {
	fprintf(fp, "static const wchar_t sort[] = {\n");
	for (i=0; i<=coll->co_nord; i++)
	    fp_putcolflag(fp, coll->co_sort.p[i]);

	fprintf(fp, "\n};\n");
    }
	
    fprintf(fp, "static lc_collate_t lc_coll = {{\n");

    /* class core */
    fp_puthdr(fp, _LC_COLLATE, sizeof(_LC_collate_t));
    fp_putmeth(fp, COLLATE_STRCOLL);
    fp_putmeth(fp, COLLATE_STRXFRM);
    fp_putmeth(fp, COLLATE_WCSCOLL);
    fp_putmeth(fp, COLLATE_WCSXFRM);
    fp_putmeth(fp, COLLATE_FNMATCH);
    fp_putmeth(fp, COLLATE_REGCOMP);
    fp_putmeth(fp, COLLATE_REGERROR);
    fp_putmeth(fp, COLLATE_REGEXEC);
    fp_putmeth(fp, COLLATE_REGFREE);
    fp_putmeth(fp, COLLATE_COLLATE_INIT); /* _LC_collate_t *(*init)(); */
    fp_putsym(fp, 0);		          /* void *data; */
    fprintf(fp,"\t},\t/* End core */\n");

    /* class extension */
    fp_putdig(fp, coll->co_nord);
    if (coll->co_nord < _COLL_WEIGHTS_INLINE) {
	fprintf(fp, "\t{ ");
	wgts = (num_wgts < _COLL_WEIGHTS_INLINE) ?
	    coll->co_sort.n : coll->co_sort.p;
	for (i=0; i<=coll->co_nord; i++)
	    fp_putcolflag(fp, wgts[i]);

	fprintf(fp, "},\n");
    }
    else
        fp_putsym(fp, "sort");
    
    fp_putdig(fp, 0);			      /* wchar_t co_wc_min; */
    fp_putdig(fp, max_wchar_enc);	      /* wchar_t co_wc_max; */
    fp_putdig(fp, coll->co_hbound);	      /* wchar_t co_hbound; */

    if (coll->co_coltbl != NULL) {

	fprintf(fp, "\t0x%04x,\n",
		coll->co_col_min);	      /* wchar_t co_col_min; */
	fprintf(fp, "\t0x%04x,\n",
		coll->co_col_max);	      /* wchar_t co_col_max; */

	fp_putsym(fp, "colltbl");      /* _LC_coltbl_t *coltbl; */
    } else {

	fprintf(fp, "\t0x%04x,\n", 0);	      /* wchar_t co_col_min; */
	fprintf(fp, "\t0x%04x,\n",
		max_wchar_enc);		      /* wchar_t co_col_max; */
    
	fp_putsym(fp, 0);		      /* _LC_coltbl_t *coltbl; */
    }

    fp_putdig(fp, coll->co_nsubs);	      /* number of subs strs */
    if (coll->co_nsubs > 0)
        fp_putsym(fp, "substrs");	      /* substitution strings */
    else 
        fp_putsym(fp, 0);


    fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_monetary
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_monetary_t locale 
*  data structure.
*/
static void gen_monetary(FILE *fp, _LC_monetary_t *lc_mon)
{

  fprintf(fp, 
"/*------------------------- MONETARY OBJECT  -------------------------*/\n");
  fprintf(fp, "static _LC_monetary_t lc_mon={{\n");

  /* class core */
  fp_puthdr(fp, _LC_MONETARY, sizeof(_LC_monetary_t));
  fp_putmeth(fp, MONETARY_NL_LANGINFO);
  fp_putmeth(fp, MONETARY_STRFMON);	
  fp_putmeth(fp, MONETARY_MONETARY_INIT);	
  fp_putsym(fp, 0);				
  fprintf(fp,"\t},\t/* End core */\n");

  /* class extension */
  fp_putstr(fp, lc_mon->int_curr_symbol);   /* char *int_curr_symbol; */
  fp_putstr(fp, lc_mon->currency_symbol);   /* char *currency_symbol; */
  fp_putstr(fp, lc_mon->mon_decimal_point); /* char *mon_decimal_point; */
  fp_putstr(fp, lc_mon->mon_thousands_sep); /* char *mon_thousands_sep; */
  fp_putstr(fp, lc_mon->mon_grouping);	    /* char *mon_grouping; */
  fp_putstr(fp, lc_mon->positive_sign);	    /* char *positive_sign; */
  fp_putstr(fp, lc_mon->negative_sign);	    /* char *negative_sign; */
  fp_putdig(fp, lc_mon->int_frac_digits);   /* signed char int_frac_digits; */
  fp_putdig(fp, lc_mon->frac_digits);	    /* signed char frac_digits; */
  fp_putdig(fp, lc_mon->p_cs_precedes);	    /* signed char p_cs_precedes; */
  fp_putdig(fp, lc_mon->p_sep_by_space);    /* signed char p_sep_by_space; */
  fp_putdig(fp, lc_mon->n_cs_precedes);	    /* signed char n_cs_precedes; */
  fp_putdig(fp, lc_mon->n_sep_by_space);    /* signed char n_sep_by_space; */
  fp_putdig(fp, lc_mon->p_sign_posn);	    /* signed char p_sign_posn; */
  fp_putdig(fp, lc_mon->n_sign_posn);	    /* signed char n_sign_posn; */
  fp_putstr(fp, lc_mon->debit_sign);	    /* char *debit_sign; */
  fp_putstr(fp, lc_mon->credit_sign);	    /* char *credit_sign; */
  fp_putstr(fp, lc_mon->left_parenthesis);  /* char *left_parenthesis; */
  fp_putstr(fp, lc_mon->right_parenthesis); /* char *right_parenthesis; */

  fprintf(fp, "};\n");
}


/* 
*  FUNCTION: gen_time
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_time_t locale data structure.
*/
static void gen_time(FILE *fp, _LC_time_t *lc_time)
{
    int i;


    if ( lc_time->era ) {
	fprintf(fp, "static char * era_strings[] = {\n");
	for(i=0; lc_time->era[i]; i++)
	    fprintf(fp, "\t\"%s\",\n", lc_time->era[i]);
	fprintf(fp, "(char *)0 };\n"); 	    /* Terminate with NULL */
    }

    fprintf(fp, 
"/*-------------------------   TIME OBJECT   -------------------------*/\n");
    fprintf(fp, "static _LC_time_t lc_time={{\n");
  
    /* class core */
    fp_puthdr(fp, _LC_TIME, sizeof(_LC_time_t));
    fp_putmeth(fp, TIME_NL_LANGINFO);	   /* char *(*nl_langinfo)(); */
    fp_putmeth(fp, TIME_STRFTIME);	   /* size_t *(strftime)();   */
    fp_putmeth(fp, TIME_STRPTIME);	   /* char *(*strptime)();    */
    fp_putmeth(fp, TIME_WCSFTIME);	   /* size_t (*wcsftime)();   */
    fp_putmeth(fp, TIME_TIME_INIT);        /* _LC_time_t *(*init)()   */
    fp_putsym(fp, 0);			   /* void *data;             */
    fprintf(fp,"\t},\t/* End core */\n");
    
    /* class extension */
    fp_putstr(fp,lc_time->d_fmt);            /* char *d_fmt; */
    fp_putstr(fp, lc_time->t_fmt);	     /* char *t_fmt; */
    fp_putstr(fp, lc_time->d_t_fmt);	     /* char *d_t_fmt; */
    fp_putstr(fp, lc_time->t_fmt_ampm);	     /* char *t_fmt_ampm; */
    fprintf(fp, "\t{\n");
    for (i=0; i<7; i++) 
	fp_putstr(fp, lc_time->abday[i]);    /* char *abday[7]; */
    fprintf(fp, "\t},\n");
    fprintf(fp, "\t{\n");
    for (i=0; i<7; i++) 
	fp_putstr(fp, lc_time->day[i]);	     /* char *day[7]; */
    fprintf(fp, "\t},\n");
    fprintf(fp, "\t{\n");
    for (i=0; i<12; i++) 
	fp_putstr(fp, lc_time->abmon[i]);    /* char *abmon[12]; */
    fprintf(fp, "\t},\n");
    fprintf(fp, "\t{\n");
    for (i=0; i<12; i++) 
	fp_putstr(fp, lc_time->mon[i]);	     /* char *mon[12]; */
    fprintf(fp, "\t},\n");
    fprintf(fp, "\t{\n");
    for (i=0; i<2; i++) 
	fp_putstr(fp, lc_time->am_pm[i]);    /* char *am_pm[2]; */
    fprintf(fp, "\t},\n");

    if(lc_time->era)
	fp_putsym(fp, "era_strings"); 	     /* There is an array of eras */
    else
	fp_putsym(fp, 0);		     /* No eras available */

    fp_putstr(fp, lc_time->era_year);	     /* char *era_year; */
    fp_putstr(fp, lc_time->era_d_fmt);	     /* char *era_d_fmt; */
    fp_putstr(fp, lc_time->alt_digits);	     /* char *alt_digits */
    fp_putstr(fp, lc_time->m_d_recent);	     /* char *m_d_recent; OSF extension */
    fp_putstr(fp, lc_time->m_d_old);	     /* char *m_d_old; OSF extension*/
    fp_putstr(fp, lc_time->era_d_t_fmt);     /* char *era_d_t_fmt */
    fp_putstr(fp, lc_time->era_t_fmt);	     /* char *era_t_fmt */
    fprintf(fp, "};\n");
}


/* 
*  FUNCTION: gen_numeric
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_numeric_t locale 
*  data structure.
*/
static void gen_numeric(FILE *fp, _LC_numeric_t *lc_num)
{
  fprintf(fp, 
"/*------------------------- NUMERIC OBJECT  -------------------------*/\n");
  fprintf(fp, "static _LC_numeric_t lc_num={{\n");

  /* class core */
  fp_puthdr(fp, _LC_NUMERIC, sizeof(_LC_numeric_t));
  fp_putmeth(fp, NUMERIC_NL_LANGINFO);	   /* char *nl_langinfo();*/
  fp_putmeth(fp, NUMERIC_NUMERIC_INIT);	   /* _LC_numeric_t *(*init)();*/
  fp_putsym(fp, 0);			   /* void *data; */
  fprintf(fp,"\t},\t/* End core */\n");

  /* class extension */

  fp_putstr(fp, lc_num->decimal_point);
  fp_putstr(fp, lc_num->thousands_sep);	    /* char *thousands_sep; */

  fprintf(fp, "\t(unsigned char *)");
  fp_putstr(fp, (char *)lc_num->grouping);  /* (unsigned char *) grouping;      */

  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_msg
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_resp_t locale data structure.
*/
static void gen_msg(FILE *fp, _LC_resp_t *lc_resp)
{
  fprintf(fp, 
"/*------------------------- MESSAGE OBJECT  -------------------------*/\n");
  fprintf(fp, "static _LC_resp_t lc_resp={{\n");

  /* class core */
  fp_puthdr(fp, _LC_RESP, sizeof(_LC_resp_t));
  fp_putmeth(fp, RESP_NL_LANGINFO);
  fp_putmeth(fp, RESP_RPMATCH);
  fp_putmeth(fp, RESP_RESP_INIT);
  fp_putsym(fp, 0);			     /* void *data;               */
  fprintf(fp,"\t},\t/* End core */\n");

  /* class extension */
  fp_putstr(fp, lc_resp->yesexpr);	    /* char *yesexpr;            */
  fp_putstr(fp, lc_resp->noexpr);	    /* char *noexpr;             */
  fp_putstr(fp, lc_resp->yesstr);           /* char *yesstr;             */
  fp_putstr(fp, lc_resp->nostr);	    /* char *nostr;              */

  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_locale
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_locale_t locale
*  data structures. This routine collects the data from the various
*  child classes of _LC_locale_t, and outputs the pieces from the child
*  classes as appropriate.
*/
static void
gen_locale(FILE *fp, _LC_charmap_t *lc_cmap, _LC_collate_t *lc_coll,  
	   _LC_ctype_t *lc_ctype, _LC_monetary_t *lc_mon, 
	   _LC_numeric_t *lc_num, _LC_resp_t *lc_resp, 
	   _LC_time_t *lc_time
	   )
{
  int i;

  fprintf(fp, 
"/*-------------------------- LCONV STRUCT ---------------------------*/\n");
  fprintf(fp, "static struct lconv lc_lconv = {\n");
  fp_putstr(fp, lc_num->decimal_point);
  fp_putstr(fp, lc_num->thousands_sep);
  fp_putstr(fp, (char *)lc_num->grouping);
  fp_putstr(fp, lc_mon->int_curr_symbol);
  fp_putstr(fp, lc_mon->currency_symbol);
  fp_putstr(fp, lc_mon->mon_decimal_point);
  fp_putstr(fp, lc_mon->mon_thousands_sep);
  fp_putstr(fp, lc_mon->mon_grouping);
  fp_putstr(fp, lc_mon->positive_sign);
  fp_putstr(fp, lc_mon->negative_sign);
  fp_putdig(fp, lc_mon->int_frac_digits);
  fp_putdig(fp, lc_mon->frac_digits);
  fp_putdig(fp, lc_mon->p_cs_precedes);
  fp_putdig(fp, lc_mon->p_sep_by_space);
  fp_putdig(fp, lc_mon->n_cs_precedes);
  fp_putdig(fp, lc_mon->n_sep_by_space);
  fp_putdig(fp, lc_mon->p_sign_posn);
  fp_putdig(fp, lc_mon->n_sign_posn);
  fp_putstr(fp, lc_mon->left_parenthesis);
  fp_putstr(fp, lc_mon->right_parenthesis);
  fprintf(fp, "};\n\n");
  
  fprintf(fp, 
"/*-------------------------- LOCALE OBJECT --------------------------*/\n");
  fprintf(fp, "static _LC_locale_t lc_loc={{\n");

  /* class core */
  fp_puthdr(fp, _LC_LOCALE, sizeof(_LC_locale_t));
  fp_putmeth(fp, LOCALE_NL_LANGINFO);
  fp_putmeth(fp, LOCALE_LOCALECONV);
  fp_putmeth(fp, LOCALE_LOCALE_INIT);
  fp_putsym(fp, 0);
  fprintf(fp,"\t},\t/* End core */\n");


  /* class extension */
  fprintf(fp, "\t{\n");			   /* Bracket array of nl_langinfo stuff */

					   /* -- lc_time data -- */
  fp_putstr(fp,lc_time->d_fmt);            /* char *d_fmt; */
  fp_putstr(fp, lc_time->t_fmt);	   /* char *t_fmt; */
  fp_putstr(fp, lc_time->d_t_fmt);	   /* char *d_t_fmt; */
  for (i=0; i<7; i++) 
    fp_putstr(fp, lc_time->abday[i]);	   /* char *abday[7]; */
  for (i=0; i<7; i++) 
    fp_putstr(fp, lc_time->day[i]);	   /* char *day[7]; */
  for (i=0; i<12; i++) 
    fp_putstr(fp, lc_time->abmon[i]);	   /* char *abmon[12]; */
  for (i=0; i<12; i++) 
    fp_putstr(fp, lc_time->mon[i]);	   /* char *mon[12]; */
  for (i=0; i<2; i++) 
    fp_putstr(fp, lc_time->am_pm[i]);	   /* char *am_pm[2]; */

					   /* -- lc_numeric data -- */
  fp_putstr(fp, lc_num->decimal_point);	   /* char *decimal_point; */
  fp_putstr(fp, lc_num->thousands_sep);	   /* char *thousands_sep; */

					   /* -- v3.1 yes/no strings -- */
  fp_putstr(fp, lc_resp->yesstr);	   /* char *yesstr; */
  fp_putstr(fp, lc_resp->nostr);	   /* char *nostr; */

					   /* X/Open CRNCYSTR */
  fp_putstr(fp, lc_mon->currency_symbol);  /* char *currency_symbol */

					   /* -- lc_cmap data -- */
  fp_putstr(fp, lc_cmap->cm_csname);	   /* char *cm_csname; */

  fp_putsym(fp, 0);			   /* Placeholders for expansion */
  fp_putsym(fp, 0);
  fp_putsym(fp, 0);
  fp_putsym(fp, 0);
  fp_putsym(fp, 0);


  fprintf(fp, "\t},\n");
					   /* -- lconv structure -- */
  fp_putsym(fp, "&lc_lconv");		   /* struct lconv *lc_lconv; */

					   /* pointers to other classes */
  fp_putsym(fp, "&lc_cmap");		   /* _LC_charmap_t *charmap; */
  fp_putsym(fp, "(_LC_collate_t *)&lc_coll");		   /* _LC_collate_t *collate; */
  fp_putsym(fp, "&lc_ctype");		   /* _LC_ctype_t *ctype; */
  fp_putsym(fp, "&lc_mon");		   /* _LC_monetary_t *monetary; */
  fp_putsym(fp, "&lc_num");		   /* _LC_numeric_t *numeric; */
  fp_putsym(fp, "&lc_resp");		   /* _LC_resp_t *resp; */
  fp_putsym(fp, "&lc_time");		   /* _LC_time_t *time; */

  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_instantiate
*
*  DESCRIPTION:
*  Generates C code which returns address of lc_locale and serves as
*  entry point to object.
*/
static void
gen_instantiate(FILE *fp)
{
  fprintf(fp, 
	  "_LC_locale_t *instantiate(void)\n{\n\treturn &lc_loc;\n}\n");
}
