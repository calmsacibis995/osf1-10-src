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
/*
*****************************************************************************

	      Copyright (c) Digital Equipment Corporation, 1990  
	      All Rights Reserved.  Unpublished rights reserved
	      under the copyright laws of the United States.
	      
	      The software contained on this media is proprietary
	      to and embodies the confidential technology of 
	      Digital Equipment Corporation.  Possession, use,
	      duplication or dissemination of the software and
	      media is authorized only pursuant to a valid written
	      license from Digital Equipment Corporation.

	      RESTRICTED RIGHTS LEGEND   Use, duplication, or 
	      disclosure by the U.S. Government is subject to
	      restrictions as set forth in Subparagraph (c)(1)(ii)
	      of DFARS 252.227-7013, or in FAR 52.227-19, as
	      applicable.

*****************************************************************************
**++
**  FACILITY:
**
**	Internationalization RTL (I18n RTL)
**
**  ABSTRACT:
**
**	Internationalization Services for DECwindows
**
**      Currently, this module is ULTRIX-only, since on shareable-library
**      systems the layer above should succeed in calling into the 
**      language-specific shareable library.
**	
**	This module was created to resolve _I18n routine symbols from the
**	I18n module for the US locale. All locales contain
**	a locale specific module containing I18N routines.
**
**      Note that this entire module is a Digital extension.
**
**
**  MODIFICATION HISTORY:
**             
**  Dec 11 : Created
**
**--
**/

#ifdef DEC_EXTENSION

#include <ctype.h>		/* I18N */
#include <X11/Intrinsic.h>
#include "I18N.h"
#include "I18nConverter.h"	/* I18N */

/*
 * memory allocator, this must be the same as the XtMalloc but can't
 * call it
 */

#ifdef VMS
extern char *lib$vm_malloc();
#define Xmalloc(size)       lib$vm_malloc(size)
#define Xfree(ptr)          lib$vm_free(ptr) 
#else
extern char *malloc();
#define Xmalloc(size)       malloc(size)
#define Xfree(ptr)          free(ptr) 
#endif /* VMS */

extern XFontStruct * _Xt_LoadQueryFont();

/*
**  Motif 1.1.3 is not capable of processing ASN.1 based compound strings.  Since we need run-time
**  compatibility with Motif 1.1.3 on OSF/1, the I18n functions must create Motif 1.1.3 style compound
**  strings when using shared libraries with Motif 1.1.3 applications.  This is not a problem for VMS
**  because two versions of Xt are being shipped for Motif 1.2.
**
**  For OSF/1 the i18n_motif_113 flag will be set to FALSE by _XmOverwriteXtVendorShell in the Motif 1.2 library.
**  This flag remain TRUE for Motif 1.1.3 because the Motif 1.1.3 _XmOverwriteXtVendorShell function does not set this.
*/
#ifdef __osf__
#ifdef MOTIF113_I18N_COMPAT
Boolean i18n_motif_113 = TRUE;
#else
Boolean i18n_motif_113 = FALSE;
#endif        /* MOTIF113_I18N_COMPAT */
#else
static Boolean i18n_motif_113 = FALSE;
#endif

#define ISO_LATIN1 		"ISO8859-1"
#define ISO_LATIN1_DIRECTION 	((XmStringDirection) XmSTRING_DIRECTION_L_TO_R)

/*  Need this duplication for locale text components */
#define XmFONTLIST_DEFAULT_TAG "FONTLIST_DEFAULT_TAG_STRING"

#define TEXT_BUF_INCR		512
#define REALLOC_CRITICAL_PT	10    /* realloc count if reached will cause */
                                      /* alloc_incr to double                */

#define ROUNDUP(length, blk_size) \
    ((((length) + (blk_size) - 1) / (blk_size)) * (blk_size))

/*
 *  Record to store the private context for constructing a compound string
 */

typedef struct
{
    int              alloc_incr;        /* how much to increment buffer size by */
    int              realloc_count;     /* how many realloc since new alloc_incr */
    int              total_realloc;	/* for statistics purposes */
    int		     header_size;       /* length of header */
    int		     cs_max_len;        /* length of buffer (excluding header) */
    int              cs_len;            /* length of cs (excluding header) */
    unsigned char    *cs;               /* resulting compound string */
    
} I18nCSConstructContextRec, *I18nCSConstructContext;

/*
 *  Record to store the private context for constructing a file code
 */

typedef struct
{
    int              status;		/* status during conversion */
    int		     alloc_incr;        /* how much to increment buffer size by */
    int              realloc_count;     /* how many realloc since new alloc_incr */
    int              total_realloc;     /* for statistics purposes */
    int              buffer_len;        /* length of buffer */
} I18nFCConstructContextRec, *I18nFCConstructContext;


/*
 * Compound string construction routines.  These routines are based on
 * XmString.c.  We can't use the public construction routines because
 * the I18n layer resides below Xm and therefore is not supposed to use
 * facilities from Xm. 
 *
 * If Motif makes changes to compound string construction, these routines will
 * have to be modified accordingly.  (Sad but true!)
 */
typedef unsigned char 	XmStringDirection;

enum{   XmSTRING_DIRECTION_L_TO_R,      XmSTRING_DIRECTION_R_TO_L,
        XmSTRING_DIRECTION_REVERT
        } ;

enum{   XmSTRING_COMPONENT_UNKNOWN,     XmSTRING_COMPONENT_CHARSET,
        XmSTRING_COMPONENT_TEXT,        XmSTRING_COMPONENT_DIRECTION,
        XmSTRING_COMPONENT_SEPARATOR,   XmSTRING_COMPONENT_LOCALE_TEXT
        /* 6-125 reserved */
        } ;

/*
**  This must match the definition in XmStrDefs.h, CS  1-July-1993
*/
#define XmSTRING_DEFAULT_CHARSET	""

#define MAX_CS_LEN	65536	/* 64K */



/*
 *  See the lib/Xm/XmString.c module for an explanation of the following
 *  symbols.
 */


#define CSHEADERLEN     3
#define CSHEADER1       0xdf
#define CSHEADER2       0xff
#define CSHEADER3       0x79
static unsigned char CSHeader[3] = {CSHEADER1, CSHEADER2, CSHEADER3};

#define ASNHEADERLEN     3

#define ASNHEADER1      0xdf
#define ASNHEADER2      0x80
#define ASNHEADER3      0x06
static unsigned char    ASNHeader[3] = {ASNHEADER1, ASNHEADER2, ASNHEADER3};


#define MAXSHORTVALUE   127             /* maximum len to be used for short 
                                           length form */

#define CSLONGLEN       3
#define CSSHORTLEN      1
#define CSLONGLEN1      0x82
#define CSLONGBIT	0x80

#define ASNTAG          1
/* Num bytes for tag & length = ASNTAG + [CSSHORTLEN | CSLONGLEN] */

#define HEADER 3	/* num bytes for tag & length */

#ifdef STRINGS_ALIGNED

#define WriteShort(addr, short_val)                         \
        *((unsigned char *)addr) = (unsigned char)(short_val & 0xff);       \
        *(((unsigned char *)addr) + 1) = (unsigned char)(short_val >> 8);  

#define ReadShort(addr, short_val)                                  \
        short = (((unsigned short) *(((unsigned char *)addr) + 1)) << 8)    \
                | ((unsigned short ) *((unsigned char *)(addr)));
#else

#define WriteShort(addr, short_val) *addr = short_val;        
#define ReadShort(addr, short_val) short_val = *addr;        

#endif  /* STRINGS_ALIGNED */

/*
 * calculates the number of bytes in the header of an external compound
 * string, given the total length of the components.
 */

#define _calc_header_size(len) \
    ((((unsigned short)(len)) > MAXSHORTVALUE) ? (CSHEADERLEN + CSLONGLEN) : (CSHEADERLEN + CSSHORTLEN))

#define _asn1_size(len) \
    ((((unsigned short)(len)) > MAXSHORTVALUE) ? (ASNTAG + CSLONGLEN) : (ASNTAG + CSSHORTLEN))

#define _is_asn1_long(p) \
  ((*((unsigned char *)(p) + ASNTAG)) & ((unsigned char)CSLONGBIT))


/*
 * Routine that writes a long length field
 */
static void
#ifdef _NO_PROTO
_write_long_length( p, length )
        unsigned char *p ;
        unsigned short length ;
#else
_write_long_length(
        unsigned char *p,
#if NeedWidePrototypes
        unsigned int length )
#else
        unsigned short length )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{

    unsigned char   * uchar_p = (unsigned char *) p;

    /*
     * flag the long version
     */
    *uchar_p = CSLONGLEN1;
    uchar_p++;

    /*
     * need to pull off the high 8 bits
     */

    *uchar_p = (unsigned char) (length >> 8);
    uchar_p++;
    *uchar_p = (unsigned char) (length & 0xff);

}


/*
 * Private routines for manipulating the ASN.1 header of external
 * compound strings.
 */
static unsigned char *
#ifdef _NO_PROTO
_write_header( p, length )
        unsigned char *p ;
        unsigned short length ;
#else
_write_header(
        unsigned char *p,
#if NeedWidePrototypes
        unsigned int length )
#else
        unsigned short length )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{

    unsigned char * uchar_p = p;
    int     headlen;

    /* write the header in. */

    if (!i18n_motif_113)		/* Motif 1.2 or later */
    {
	headlen = ASNHEADERLEN;
	memcpy(uchar_p, ASNHeader, ASNHEADERLEN);
	uchar_p += ASNHEADERLEN;
    }
    else
    {
	headlen = CSHEADERLEN;
	memcpy(uchar_p, CSHeader, CSHEADERLEN);
	uchar_p += CSHEADERLEN;
    }

    /* short or long length value? */

    if (length > MAXSHORTVALUE)
    {
        _write_long_length(uchar_p, length);
        headlen += CSLONGLEN;
    }
    else {
        /* Short version */
        *uchar_p = (unsigned char) length;
        headlen += CSSHORTLEN;
    }
    return (p + headlen);
}

/*
 * Determines whether this string has a short or long length field
 */
static Boolean
#ifdef _NO_PROTO
_is_short_length( p )
        unsigned char *p ;
#else
_is_short_length(
        unsigned char *p )
#endif /* _NO_PROTO */
{

    unsigned char *uchar_p = (unsigned char *) p;

    if (!i18n_motif_113)		/* Motif 1.2 or later */
	uchar_p += ASNHEADERLEN;
    else
	uchar_p += CSHEADERLEN;

    if (*uchar_p & (char)CSLONGBIT)
       return (FALSE);
    else return (TRUE);
}

	


/*
 * reads the length the ASN.1 header of an external
 * compound string.
 */
static unsigned short
#ifdef _NO_PROTO
_read_header_length( p )
        unsigned char *p ;
#else
_read_header_length(
        unsigned char *p )
#endif /* _NO_PROTO */
{
    /*
     * Read past the ASN.1 header; get the first length byte and see if this
     * is a one or three byte length.
     */

    if (!i18n_motif_113)		/* Motif 1.2 or later */
    {
	if (_is_short_length(p))
	   return (ASNHEADERLEN + CSSHORTLEN);
	else
	   return (ASNHEADERLEN + CSLONGLEN);
    }
    else
    {
	if (_is_short_length(p))
	    return (CSHEADERLEN + CSSHORTLEN);
	else
	    return (CSHEADERLEN + CSLONGLEN);
    }
}


/*
 * calculates the length of the external compound string, excluding the
 * ASN.1 header.
 */
static unsigned short
#ifdef _NO_PROTO
_read_string_length( p )
        unsigned char *p ;
#else
_read_string_length(
        unsigned char *p )
#endif /* _NO_PROTO */
{


    unsigned char * uchar_p = (unsigned char *) p;
    unsigned short totallen = 0;

    /*
     * Read past the ASN.1 header; get the first length byte and see if this
     * is a one or three byte length.
     */

    if (!i18n_motif_113)		/* Motif 1.2 or later */
	uchar_p += ASNHEADERLEN;
    else
	uchar_p += CSHEADERLEN;

    if (_is_short_length(p))
    {
       totallen += (unsigned short) *uchar_p;
    }
    else {
       unsigned short i;

       uchar_p++;
       i = ((unsigned short) *uchar_p) << 8;
       uchar_p++;
       i |= ((unsigned short) *uchar_p);    /* Mask on the low byte */
       totallen += i;
    }
    return (totallen);
}

/*
 * Private routines for reading/writing the individual compound string
 * components
 */
static unsigned char *
#ifdef _NO_PROTO
_write_component( p, tag, length, value, move_by_length )
        unsigned char *p ;
        unsigned char tag ;
        unsigned short length ;
        unsigned char *value ;
        Boolean move_by_length ;
#else
_write_component(
        unsigned char *p,
#if NeedWidePrototypes
        unsigned int tag,
        unsigned int length,
#else
        unsigned char tag,
        unsigned short length,
#endif /* NeedWidePrototypes */
        unsigned char *value,
#if NeedWidePrototypes
        int move_by_length )
#else
        Boolean move_by_length )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    unsigned char * uchar_p = p;

    *uchar_p = tag;                             /* stuff tag */

    if (!i18n_motif_113)		/* Motif 1.2 or later */
    {
	uchar_p += ASNTAG;

	/* short or long length value? */
	if (length > MAXSHORTVALUE)
	{
	    _write_long_length(uchar_p, length);
	    uchar_p += CSLONGLEN;
	}
	else
	{
	    /* Short version */
	    *uchar_p = (unsigned char) length;
	    uchar_p += CSSHORTLEN;
	}
    }
    else
    {
	unsigned short * short_p = (unsigned short *) (p + 1);

	uchar_p += HEADER;				/* move to value */
	WriteShort(short_p, length);
    }

    if (value != (unsigned char *) NULL)
      memcpy((char *)uchar_p, (char *)value, (int)length);

    if (move_by_length)
        return (uchar_p + length);
    else
        return (uchar_p);
}



/* These are the os-specific environment variables checked for a language
** specification.
*/
#define env_variable "LANG"


struct __Xmlocale {
    char   *charset;
    int    charsetlen;
    Boolean inited;
};

static struct __Xmlocale locale;

static void
#ifdef _NO_PROTO
_parse_locale( str, indx, len )
        char *str ;
        int *indx ;
        int *len ;
#else
_parse_locale(
        char *str,
        int *indx,
        int *len )
#endif /* _NO_PROTO */
{
    char     *temp;
    int      start;
    int      end;

    /*
     *  Set the return variables to zero.  If we find what we're looking
     *  for, we reset them.
     */

    *indx = 0;
    *len = 0;

    /*
     *  The format of the locale string is:
     *          language[_territory[.codeset]]
     */

    temp = str;
    for (end = 0; (temp[end] != '.') && (temp[end] != 0); end++)
      ;

    if (temp[end] == '.')
    {
        start = end + 1;
        *indx = start;
        for (end = start; temp[end] != 0; end++)
            ;
        *len = end - start;
    }
}




 /* This function returns current default charset being used.  This is */
 /* determined from teh value of the $LANG environment variable or */
 /* XmFALLBACK_CHARSET.  This version converts the charset to the */
 /* canonical form (set _XmCharsaetCanonicalize) before caching it. */
/* char *_XmStringGetCurrentCharset() */
static char *
#ifdef _NO_PROTO
_I18nGetCurrentCharset()
#else
_I18nGetCurrentCharset( void )
#endif
{
    char *str;
    char *ptr;
    char *tmpstr = NULL;
    int  chlen;
    int  index;
    int  len;
   
    if (!locale.inited)
    {
        tmpstr = NULL;
        locale.charset = NULL;
        locale.charsetlen = 0;
 
        str = (char *)getenv(env_variable);

        if (str)
        {
           _parse_locale(str, &index, &chlen);
           if (chlen > 0)
           {
               ptr = &str[index];

	       if (!strcmp(ptr, "ASCII"))
		 {
		   len = strlen(ISO_LATIN1);
		   ptr = ISO_LATIN1;
		 }
	       else if ((chlen == 5) &&
			isdigit(ptr[0]) &&
			isdigit(ptr[1]) &&
			isdigit(ptr[2]) &&
			isdigit(ptr[3]) &&
			isdigit(ptr[4]))
		 {
		   /* "ISO####-#" */
		   tmpstr = Xmalloc(3 + 4 + 1 + 1 + 1);
		   sprintf(tmpstr, "ISO%s", ptr);
		   tmpstr[7] = '-';
		   tmpstr[8] = ptr[4];
		   tmpstr[9] = '\0';
		   
		   len = 9;
		   ptr = tmpstr;
		 }
	       else
		 {
		   len = chlen;
		 }
	       
           }
           else {
               len = strlen(ISO_LATIN1);
               ptr = ISO_LATIN1;
           }
        }
        else {
           len = strlen(ISO_LATIN1);
           ptr = ISO_LATIN1;
        }
        locale.charset = (char *) Xmalloc(len + 1);
        strncpy(locale.charset, ptr, len);
        locale.charset[len] = '\0';
        locale.charsetlen = len;
        locale.inited = TRUE;
        if (tmpstr != NULL) Xfree(tmpstr);
    }

    /*
    **	For Motif 1.2 return a pointer to the cached character set.  For Motif 1.1.3
    **	return a copy of the character set.
    */
    if (!i18n_motif_113)	/* Motif 1.2 or later */
	return (locale.charset);
    else
    {
	char *charset;

	charset = (char *) Xmalloc(locale.charsetlen + 1);
	strncpy(charset, locale.charset, locale.charsetlen);
	charset[locale.charsetlen] = '\0';

	return (charset);
    }
}

/* Quick check for possible numerical only designation of ISO charset. */
static Boolean 
#ifdef _NO_PROTO
_isISO(charset)
     String charset;
#else
_isISO(
     String charset)
#endif
{
  register int	i;
  
  if (strlen(charset) == 5) 
    {
      for (i = 0; i < 5; i++) 
	{
	  if (!isdigit(charset[i])) return (False);
	}
      return (True);
    }
  else return (False);
}

/* This function takes a charset string as it might be found in $LANG
 * and converts it to the form being used within Xm. 
 *
 * Note that character sets are cached in Motif 1.2, and so, never freed.
 */

static char *
#ifdef _NO_PROTO
_I18nCharsetCanonicalize(charset) 
     String charset;
#else
_I18nCharsetCanonicalize(
     String charset)
#endif
{
  String	new;
  String        tmpptr;
  int		len;
  

  tmpptr = NULL;
  /* ASCII -> ISO8859-1 */
  if (!strcmp(charset, "ASCII"))
    {
      len = strlen(ISO_LATIN1);

      if (!i18n_motif_113)	/* Motif 1.2 or later */
      {
	new = ISO_LATIN1;
      }
      else
      {
	new = Xmalloc(len + 1);
	bcopy( ISO_LATIN1, new, len );
	new[len] = '\0';
      }
    }
  else if (_isISO(charset))
    {
      /* "ISO####-#" */
      tmpptr = Xmalloc(3 + 4 + 1 + 1 + 1);
      sprintf(tmpptr, "ISO%s", charset);
      tmpptr[7] = '-';
      tmpptr[8] = charset[4];
      tmpptr[9] = '\0';
      new = tmpptr; 
   }
  else
    /* Anything else is copied but not modified. */
    {
      len = strlen(charset);

      if (!i18n_motif_113)	/* Motif 1.2 or later */
	new = charset;
      else
      {
	new = Xmalloc(len + 1);
	bcopy( charset, new, len);
	new[len] = '\0';
      }	
    }

     return (new);
}


/*
 *  We are using Xmalloc and Xfree, but there is no Xrealloc.  Write our
 *  own based on Xmalloc and Xfree.
 */

static char *
#ifdef _NO_PROTO
do_realloc(ptr, new_size, old_size)
	unsigned char * ptr;
	int new_size;
	int old_size;
#else
do_realloc(
	unsigned char * ptr,
	int new_size,
	int old_size )
#endif
{
    char *new;
    
    new = Xmalloc(new_size);
    if (!new)
        return (char *) NULL;
        
    memcpy(new, ptr, old_size);
    Xfree(ptr);
    return new;
}

/*
 *  Returns the allocation increment for constructing a CS.
 *  Input is the length of the FC or OS.
 */

static int
#ifdef _NO_PROTO
get_cs_alloc_hint(size)
    int size;
#else
get_cs_alloc_hint(
    int size)
#endif
{
    if (size < 40)
        return (64);
    else
        return (512);
}

/*
 *  Returns the allocation increment for constructing a FC.
 *  Input is the length of the CS.
 */

static int
#ifdef _NO_PROTO
get_fc_alloc_hint(size)
    int size;
#else
get_fc_alloc_hint(
    int size)
#endif
{
    int hint;
    
    if (size < 128)
        hint = 64;
    else if (size < 384)
        hint = 256;
    else
        hint = 512;
        
    size = 10;				 /* minimum return size is 10 bytes */
    return (hint < size ? hint : size);  /* FC for ISO Latin-1 is always smaller than CS */
}

/*
 *  Returns pointer to where the component should be written to.
 *  Returns null if maximum compound string length is reached or
 *  if memory is exhausted.
 */

static unsigned char *
#ifdef _NO_PROTO
alloc_component(context,size)
    I18nCvtContext context;
    int		  size;
#else
alloc_component(
    I18nCvtContext context,
    int		  size) 
#endif
{
    I18nCSConstructContext  construct_context;
    unsigned char           *new_cs;
    int			    total_len, new_size;
    int			    new_header_size;
    int                     new_alloc_incr;
    int			    offset;
    
    construct_context = (I18nCSConstructContext) context -> stream_context;
    
    /*
     *  Check that resulting length won't exceed the implementation
     *  limit of a compound string (64K).
     */
    
    new_header_size = _calc_header_size(construct_context->cs_len + size);
    if (MAX_CS_LEN - new_header_size - construct_context->cs_len < size)
        return (unsigned char *) NULL;
    
    /*
     *  The construct_context -> alloc_incr wasn't set properly at
     *  init callback time because the FC or OS code length wasn't
     *  available.  Do it now.
     */
     
    new_alloc_incr = get_cs_alloc_hint(context->memory_length);
    if (new_alloc_incr > construct_context->alloc_incr)
        construct_context->alloc_incr = new_alloc_incr;
    
    if (new_header_size != construct_context->header_size)
    {
        /*
         *  The header size has changed.  Create a new compound string
         *  from the old one but with a new header.
         */
        
        total_len = construct_context->cs_len + size;
        new_size = ROUNDUP(new_header_size + total_len,
                           construct_context->alloc_incr);
        new_cs = (unsigned char *) Xmalloc(new_size);
        if (!new_cs)
            return (unsigned char *) NULL;
        (void) _write_header(new_cs, total_len);
        memcpy(new_cs + new_header_size,
               construct_context->cs + construct_context->header_size,
               construct_context->cs_len);
        Xfree(construct_context->cs);
        construct_context->cs = new_cs;
        construct_context->header_size = new_header_size;
        construct_context->cs_max_len = new_size - new_header_size;
    } else if (construct_context->cs_len + size >
               construct_context->cs_max_len)
    {

        /*
         *  The header size hasn't changed, but the length of the
         *  the CS has.  Update it.
         */
        new_size = ROUNDUP(construct_context->header_size +
                           construct_context->cs_len + size,
                           construct_context->alloc_incr);
        new_cs = (unsigned char *)
		 do_realloc(construct_context->cs,
                            new_size,
                            construct_context->header_size +
                            construct_context->cs_max_len);
        if (!new_cs)
            return (unsigned char *) NULL;

        construct_context->total_realloc++;
        construct_context->realloc_count++;
        if (construct_context->realloc_count == REALLOC_CRITICAL_PT)
        {
            construct_context->alloc_incr *= 2;
            construct_context->realloc_count = 0;
        }
        construct_context->cs_max_len = new_size - 
                            construct_context->header_size;
        construct_context->cs = new_cs;
    }
    
    offset = construct_context->cs_len;
    construct_context->cs_len += size;

    return (construct_context->cs + construct_context->header_size + offset);
}

/* I18N END */

/* **NOTE!! I18nMalloc does not have _I18nMalloc equivalent because _I18n is in 
 * different shareable image
 */
static char *
#ifdef _NO_PROTO
I18nMalloc (size)
    unsigned size;
#else
I18nMalloc (
    unsigned size)
#endif
{
    char *ptr;

    ptr = Xmalloc (size);
    
    return(ptr);
}


/*
 *************************************************************************
 *
 * font fallback routines
 */

static char aster[]       = "-*";
static char dec[]         = "-dec";
static char adobe[]       = "-adobe";
static char bitstream[]   = "-bitstream";
static char helvetica[]   = "-helvetica";
static char century[]     = "-new century schoolbook";
static char familytimes[] = "-times";
static char bold[]        = "-bold";
static char medium[]      = "-medium";
static char normal[]      = "-normal";
static char slanti[]      = "-i";
static char pixel11[]     = "-11";
static char pixel14[]     = "-14";
static char pixel17[]     = "-17";
static char pixel18[]     = "-18";
static char pixel20[]     = "-20";
static char pixel25[]     = "-25";
static char pixel34[]     = "-34";
static char point140[]    = "-140";

#ifdef _NO_PROTO
Boolean _I18nRemapFontname (old, new)
    I18NFontNamePtr	old;
    I18NFontNamePtr	new;
#else
Boolean _I18nRemapFontname ( I18NFontNamePtr	old,
			    I18NFontNamePtr	new )
#endif /* _NO_PROTO */
{
    int i;

    /* Null all the new fields out.  We will need to know which have not
       be updated at the end. */

    for (i = 0; i < NUMOFI18NFIELDS; i++)
        new->fields[i] = NULL;

    /* Okay, we should have everything we need.  Find which mapping to do */

    if (strcmp(old->fields[I18NFAMILY], "-terminal") == 0)
    {

        /* handling terminal font */

        new->fields[I18NSETWIDTH] = normal;
        new->fields[I18NPOINTSIZE] = point140;
        new->fields[I18NAVGWIDTH] = aster;

        if (strcmp(old->fields[I18NXRES], "-100") == 0)
        {
            /* set new found of bitstream and pixelsize of 18 */
            new->fields[I18NFOUNDRY] = bitstream;
            new->fields[I18NPIXELSIZE] = pixel18;
        }
        else if (strcmp(old->fields[I18NXRES], "-75") == 0)
        {
            /* set new foundry of DEC and pixel size of 14 */
            new->fields[I18NFOUNDRY] = dec;
            new->fields[I18NPIXELSIZE] = pixel14;
        }
        else if (strcmp(old->fields[I18NXRES], "-*") == 0)
        {
            /* set new foundry and pixelsize of '*' */
            new->fields[I18NFOUNDRY] = aster;
            new->fields[I18NPIXELSIZE] = aster;
        }
    }
    else {

        /* handling non-Terminal families */

        /* Get the weight */

        if ((strcmp(old->fields[I18NWEIGHT], "-book") == 0) ||
            (strcmp(old->fields[I18NWEIGHT], "-light") == 0))
            new->fields[I18NWEIGHT] = medium;
        else if (strcmp(old->fields[I18NWEIGHT], "-demi") == 0)
            new->fields[I18NWEIGHT] = bold;

        /* Get the family */

        if (strcmp(old->fields[I18NFAMILY], "-itc avant garde gothic") == 0)
            /* set helvetica family */
            new->fields[I18NFAMILY] = helvetica;
        else if (strcmp(old->fields[I18NFAMILY], "-itc lubalin graph") == 0)
        {
            new->fields[I18NFAMILY] = century;
            if (strcmp(old->fields[I18NSLANT], "-o") == 0)
                new->fields[I18NSLANT] = slanti;
        }

        else if (strcmp(old->fields[I18NFAMILY], "-menu") == 0)
        {
	    /* Menu family overrides the weight, if it was set */
            new->fields[I18NFAMILY] = helvetica;
            new->fields[I18NWEIGHT] = bold;
        }
        else if (strcmp(old->fields[I18NFAMILY], "-itc souvenir") == 0)
            new->fields[I18NFAMILY] = familytimes;

        else return (FALSE);

        /* Check the resolution */

        if (strcmp(old->fields[I18NXRES], "-*") == 0)
           new->fields[I18NPIXELSIZE] = aster;
        else if (strcmp(old->fields[I18NXRES], "-100") == 0)
        {
             if (strcmp(old->fields[I18NPIXELSIZE], "-10") == 0)
               new->fields[I18NPIXELSIZE] = pixel11;
           else if (strcmp(old->fields[I18NPIXELSIZE], "-13") == 0)
               new->fields[I18NPIXELSIZE] = pixel14;
           else if (strcmp(old->fields[I18NPIXELSIZE], "-16") == 0)
               new->fields[I18NPIXELSIZE] = pixel17;
           else if (strcmp(old->fields[I18NPIXELSIZE], "-19") == 0)
               new->fields[I18NPIXELSIZE] = pixel20;
           else if (strcmp(old->fields[I18NPIXELSIZE], "-24") == 0)
               new->fields[I18NPIXELSIZE] = pixel25;
           else if (strcmp(old->fields[I18NPIXELSIZE], "-33") == 0)
                new->fields[I18NPIXELSIZE] = pixel34;
        }

        /* set foundry Adobe, and average width of '*' */
        new->fields[I18NFOUNDRY] = adobe;
        new->fields[I18NAVGWIDTH] = aster;
    }

    /* Anything that still has NULL should point at old values. */

    for (i = 0; i < NUMOFI18NFIELDS; i++)
    {
        if (new->fields[i] == NULL)
            new->fields[i] = old->fields[i];
    }
    new->TKmajor = I18NTKMAJOR;
    new->TKminor = I18NTKMINOR;
    new->XFLDmajor = I18NXFLDMAJOR;
    new->XFLDminor = I18NXFLDMINOR;
}


/*
 *************************************************************************
 *
 * Provide locale sensitive Motif compound string version of toolkit 
 * ASCII default text values such as "OK", "Cancel" etc.
 */

Boolean
#ifdef _NO_PROTO
_I18nGetLocaleString ( context, cvtcontext, ascii, word_type )
char		*context;
I18nCvtContext	cvtcontext;
char		*ascii;
I18nWordType	word_type;
#else
_I18nGetLocaleString ( char		*context,
		      I18nCvtContext	cvtcontext,
		      char		*ascii,
		      I18nWordType	word_type )
#endif /* _NO_PROTO */
{

    int status;

    cvtcontext->memory = (Opaque *) ascii;
    cvtcontext->memory_length = strlen(ascii);
    I18nCvtFCtoCS ( cvtcontext, &status );

    return (status);
}

unsigned long
#ifdef _NO_PROTO
_I18nGetLocaleMnemonic(context,widget_class,mnemonic,charset,returned_mnemonic)
I18nContext     context;
char *          widget_class;       /* class to help I18N layer to translate */
char *          mnemonic;           /* mnemonic string                       */
char *          charset;            /* The requested charset of the mnemonic */
char **         returned_mnemonic;   /* translated mnemonic                  */
#else
_I18nGetLocaleMnemonic( I18nContext     context,
		       char *          widget_class,
		       char *          mnemonic,
		       char *          charset,
		       char **         returned_mnemonic )
#endif /* _NO_PROTO */
{
        unsigned long answer = 0;

        /*
        ** Latin1 based Locale
        */
        if (strcmp(charset,"ISO8859-1") != 0)
             answer = 0;
        else

        /*
        ** Here we should cascade all the possible widget class and
        ** their mnemonic translation.
        ** For US local it's merly a one to one translation.
        ** Note that the result is not allocated.
        */
          {
            if (strcmp(widget_class, "DXmHelp") == 0)
                {
                  *returned_mnemonic = mnemonic;
                  answer = 1;
                }
          }

        return(answer);

}

#ifdef _NO_PROTO
char *_I18nGetLocaleCharset( context )
char *context;
#else
char *_I18nGetLocaleCharset( char *context )
#endif /* _NO_PROTO */
{
    char *charset;

    charset = _I18nGetCurrentCharset();
    
    return (charset);
}


#ifdef _NO_PROTO
char **_I18nGetLocaleCharsets( context )
char *context;
#else
char **_I18nGetLocaleCharsets( char *context )
#endif /* _NO_PROTO */
{

char **charset_array;

    charset_array = (char **) Xmalloc(sizeof(char *) * 2);
    
    charset_array[0] = _I18nGetCurrentCharset();
    
    charset_array[1] = NULL;

    return( charset_array );
}




/*
 *************************************************************************
 *
 * convert character range information into byte offset information
 */

#ifdef _NO_PROTO
void _I18nSegmentMeasure( context, 
			 charset, 
			 text, 
			 character_offset,
			 number_characters,
			 byte_offset_start,
			 byte_offset_end )
char *context;
char *charset;
char *text;
int character_offset;
int number_characters;
int *byte_offset_start;
int *byte_offset_end;
#else
void _I18nSegmentMeasure( char *context,
			 char *charset,
			 char *text,
			 int character_offset,
			 int number_characters,
			 int *byte_offset_start,
			 int *byte_offset_end )
#endif
{
    /* add 2 byte charset checking
     */
     if ( strcmp(charset, "DEC.CNS11643.1986-2") == 0 ||
	  strcmp(charset, "DEC.DTSCS.1990-2") == 0    ||
	  strcmp(charset, "GB2312.1980-0") == 0       ||
	  strcmp(charset, "GB2312.1980-1") == 0       ||
	  strcmp(charset, "JISX0208.1983-0") == 0     ||
	  strcmp(charset, "JISX0208.1983-1") == 0     ||
	  strcmp(charset, "KSC5601.1987-0") == 0      ||
	  strcmp(charset, "KSC5601.1987-1") == 0 )
    {

       *byte_offset_start = character_offset * 2;
       *byte_offset_end   = (character_offset + number_characters) * 2;

    } else {
       /* for right now, assuming charset is ISO8859-1 
       */
       *byte_offset_start = character_offset;
       *byte_offset_end   = character_offset + number_characters;
    }

    return;    
}

/*======================================================================
 *
 * Return the segment character length, as opposed to byte length
 *
 */
#ifdef _NO_PROTO
void _I18nSegmentLength( context,
                        charset,
                        text,
                        byte_length,
                        char_length )

Opaque context;  /* I18nContext */
char   *charset;
Opaque text;
int    byte_length;
int    *char_length;   /* the character length of the text */
#else
void _I18nSegmentLength( Opaque context,
			char   *charset,
			Opaque text,
			int    byte_length,
			int    *char_length )
#endif
{

    /* add 2 byte charset checking
     */
     if ( strcmp(charset, "DEC.CNS11643.1986-2") == 0 ||
	  strcmp(charset, "DEC.DTSCS.1990-2") == 0    ||
	  strcmp(charset, "GB2312.1980-0") == 0       ||
	  strcmp(charset, "GB2312.1980-1") == 0       ||
	  strcmp(charset, "JISX0208.1983-0") == 0     ||
	  strcmp(charset, "JISX0208.1983-1") == 0     ||
	  strcmp(charset, "KSC5601.1987-0") == 0      ||
	  strcmp(charset, "KSC5601.1987-1") == 0 )
    {

       *char_length = byte_length / 2;

    } else {
       /* for now assume isolatin1
        */
       *char_length = byte_length;
    }
}


/*
 * This routine allows compound string segments shich do not encode characters
 * as actual glyph indexes the following entrypoint and data structure are
 * provided.  Note that this is designed to allow not just a simple mapping
 * but an arbitrary mapping of a compound string segment into Xlib rendering
 * segments.
 */
#ifdef _NO_PROTO
void _I18nCreateXlibBuffers ( context,
			     font_list,
			     charset,
			     direction,
			     text,
			     character_count,
			     first_char_position,
			     need_more_callback,
			     need_more_context,
			     no_font_callback,
			     no_font_context,
			     buffers,		/* output */
			     buffer_count )	/* output */
I18nContext	context;
I18nFontList 	*font_list;
char 		*charset;
int		direction;
Opaque		text;
short		character_count;
int		first_char_position;
VoidProc	need_more_callback;
Opaque		need_more_context;
VoidProc	no_font_callback;
Opaque		no_font_context;
I18nXlibBuffers	*buffers;		/* output */
short		*buffer_count;		/* output */
#else
void _I18nCreateXlibBuffers ( I18nContext	context,
			     I18nFontList 	*font_list,
			     char 		*charset,
			     int		direction,
			     Opaque		text,
			     short		character_count,
			     int		first_char_position,
			     VoidProc		need_more_callback,
			     Opaque		need_more_context,
			     VoidProc		no_font_callback,
			     Opaque		no_font_context,
			     I18nXlibBuffers	*buffers,
			     short		*buffer_count )
#endif
{

    int  i;
    int  num_byte;
    long offset_byte;

    /* add 2 byte charset checking
     */
     if ( strcmp(charset, "DEC.CNS11643.1986-2") == 0 ||
	  strcmp(charset, "DEC.DTSCS.1990-2") == 0    ||
	  strcmp(charset, "GB2312.1980-0") == 0       ||
	  strcmp(charset, "GB2312.1980-1") == 0       ||
	  strcmp(charset, "JISX0208.1983-0") == 0     ||
	  strcmp(charset, "JISX0208.1983-1") == 0     ||
	  strcmp(charset, "KSC5601.1987-0") == 0      ||
	  strcmp(charset, "KSC5601.1987-1") == 0 )
    {
         num_byte = character_count * 2;
	 offset_byte = first_char_position * 2;
    } else {
	 offset_byte = first_char_position;
         num_byte = character_count;
    }

    /* initialize buffer.  Note: only one xlib_buff is taken care now
     */
    if ( *buffers == NULL )
    {
	*buffers = (I18nXlibBuffers) I18nMalloc ( sizeof(I18nXlibBufferRec) );
	(*buffers)->x	= 0;
	(*buffers)->width	= 0;
	(*buffers)->font_index = 0;
	(*buffers)->text	= (char *) I18nMalloc ( num_byte * sizeof(char) );
	(*buffers)->char_count = character_count;

	*buffer_count = 1;
    }

    /* copy the text to the buffers structure.  Note that if dealing with
     * RightToLeft segments, swap the order
     */
    bcopy ( (char *)text + offset_byte, (*buffers)->text, num_byte );
}


/*
 * this routine returns a null-terminated string:
 *	"fontname [=charset] [,fontname [=charset]]*"
 */

char *
#ifdef _NO_PROTO
_I18nCreateDefaultFontList (context, widget_class_name, resource_name)
I18nContext context;
char	* widget_class_name;
char	* resource_name;
#else
_I18nCreateDefaultFontList ( I18nContext context,
			    char	* widget_class_name,
			    char	* resource_name )
#endif
{
    char *i18n_flist;
    char *DXmDefaultFont =
/*
 *  This string must match DXmDefaultFont exactly, or the font will
 *  be loaded twice.
 */
		"-*-Menu-Medium-R-Normal--*-120-*-*-P-*-ISO8859-1";
    char *DXmHelpTextFont =
		"-*-TERMINAL-MEDIUM-R-NARROW--*-140-*-*-C-*-ISO8859-1";


    if((!strcmp(widget_class_name,"DXmHelp")) &&
		resource_name && (!strcmp(resource_name,"textFontList")))
    {
      int len;
      len = strlen ( DXmHelpTextFont ) + 1;
      i18n_flist = I18nMalloc( len );
      bcopy( DXmHelpTextFont, i18n_flist, len );
    }
    else{
      int len;
      len = strlen ( DXmDefaultFont ) + 1;
      i18n_flist = I18nMalloc( len );
      bcopy( DXmDefaultFont, i18n_flist, len );
    }
    return (i18n_flist);

}


/* this routine provides a functin which is used by converters to map the 
 * external encoding (e.g. DDIF or file code) into the desired encoding for 
 * a compound string.
 */
#ifdef _NO_PROTO
void _I18nMapSegment ( context, input_charset,
			       input_text,
			       input_char_count,
			       input_byte_count,
			       output_charset,
			       output_text,
			       output_char_count,
			       output_byte_count )
I18nContext	context;
char *		input_charset;
Opaque		input_text;
short		input_char_count;
short		input_byte_count;
char **		output_charset;
Opaque *	output_text;
short *		output_char_count;
short *		output_byte_count;
#else
void _I18nMapSegment ( I18nContext	context,
		      char *		input_charset,
		      Opaque		input_text,
		      short		input_char_count,
		      short		input_byte_count,
		      char **		output_charset,
		      Opaque *		output_text,
		      short *		output_char_count,
		      short *		output_byte_count )
#endif
{


	/* for US locale, just copy all the input fields to the output's
         */

	*output_charset    = input_charset;
	*output_text       = input_text;
	*output_char_count = input_char_count;
	*output_byte_count = input_byte_count;

	return;
}

/*
 * This routine decides if the sub_string occurs within the string.  N.B. the
 * two strings can be assumed to have the same character set.  These strings
 * are the direct segment text buffers, not the decomposed X sub-segment.
 */
#ifdef _NO_PROTO
Boolean _I18nHasSubstring ( context, character_set, string, sub_string,
			   string_char_count, sub_char_count )
I18nContext	context;
char 		*character_set;
Opaque		*string;
Opaque		*sub_string;
int		string_char_count;
int		sub_char_count;
#else
Boolean _I18nHasSubstring ( I18nContext	context,
			   char 	*character_set,
			   Opaque	*string,
			   Opaque	*sub_string,
			   int		string_char_count,
			   int		sub_char_count )
#endif
{
    char     *str, *last;
    Boolean  found;
    char		    *current_charset;
    Boolean		    compare;
    Boolean		    compare2;


    /*
     *  Only know how to deal with the default charset.  If another char set
     *  is given, assume the substring match failed.
     */

    current_charset = _I18nGetCurrentCharset();
    compare = strcmp ( character_set, current_charset );
    compare2 = strcmp ( character_set, XmFONTLIST_DEFAULT_TAG );
    if ( compare && compare2 )	/* charracter_set != the default charset */
        return (False);
    
    /*
     *  Check for trivial cases.
     */
    
    if (string_char_count < sub_char_count || string_char_count < 0 ||
        sub_char_count < 0)
        return (False);
    
    /*
     *  Don't assume the given strings are null-terminated, since
     *  an explicit length is given.  (ie, don't use strstr).
     */
    
    last = (char *) string + (string_char_count - sub_char_count);
    for (str = (char *) string; str <= last; str++)
    {
        found = strncmp(str, (char *) sub_string, sub_char_count) == 0;
        if (found)
            break;
    }
    
    return (found);
}

/*
 * This routine finds the start and end positions of the sub string.  It is 
 * assumed that the sub_string does exist sithin the string.  These strings are
 * the direct segment text buffers, not the decomposed X sub-segments.
 */
#ifdef _NO_PROTO
void _I18nSubStringPosition ( context, 
			     font,
			     character_set,
			     string,
			     sub_string,
			     string_char_count,
			     sub_char_count,
			     x,
			     under_begin,
			     under_end )
I18nContext	context;
XFontStruct	*font;
char 		*character_set;
Opaque		*string;
Opaque		*sub_string;
int		string_char_count;
int		sub_char_count;
Position	x;
Dimension	*under_begin;
Dimension	*under_end;
#else
void _I18nSubStringPosition ( I18nContext	context,
			     XFontStruct	*font,
			     char 		*character_set,
			     Opaque		*string,
			     Opaque		*sub_string,
			     int		string_char_count,
			     int		sub_char_count,
			     Position		x,
			     Dimension		*under_begin,
			     Dimension		*under_end )
#endif
{


	/* BOGUS just return at the mean time
	 */

	return;
}

/*
 * This routine returns True if the character is a white space.  For US locale,
 * ' ' and '\t' or '\n' are considered to be white space.  For Asian locale,
 * all the above plus 0xA1A1 should be considered white.
 */
#ifdef _NO_PROTO
Boolean _I18nIsWhiteSpace ( context, this_charset, this_char )
I18nContext	context;
char		*this_charset;
Opaque		*this_char;
#else
Boolean _I18nIsWhiteSpace ( I18nContext	context,
			   char		*this_charset,
			   Opaque	*this_char )
#endif
{

    if ( this_char == NULL )
	return (False);

    if ( (char) *this_char == ' '  ||
	 (char) *this_char == '\t' ||
	 (char) *this_char == '\n' )
    {
	return (True);
    } else {
	return (False);
    }

}


/*
 * This routine returns True if the indicated character should be considered
 * as the bounds of a scan operation.  The scan_direction field equals 0 if
 * scanning direction for a break is from the previous to the next char and
 * 1 if the scanning from next to previous.
 */
#ifdef _NO_PROTO
Boolean _I18nIsScanBreak ( context,
			  prev_charset,
			  prev_char,
			  this_charset,
			  this_char,
			  next_charset,
			  next_char,
			  scan_direction,
			  scan_type )
I18nContext	context;
char		*prev_charset;
Opaque		*prev_char;
char		*this_charset;
Opaque		*this_char;
char		*next_charset;
Opaque		*next_char;
int		scan_direction;
I18nScanType	scan_type;
#else
Boolean _I18nIsScanBreak ( I18nContext	context,
			  char		*prev_charset,
			  Opaque	*prev_char,
			  char		*this_charset,
			  Opaque	*this_char,
			  char		*next_charset,
			  Opaque	*next_char,
			  int		scan_direction,
			  I18nScanType  scan_type )
#endif
{

#define SCAN_PREV_TO_NEXT 0
#define SCAN_NEXT_TO_PREV 1
#define blank_char(this_char)				\
		((char) this_char == ' '  ||		\
		 (char) this_char == '\t' ||		\
		 (char) this_char == '\n' )

    /* for US locale, characters are assumed to be ascii
     */

     /* values returned should be according to the following table
      *
      *	'a'  = any non-blank character
      * '*'  = don't care
      * '-'  = blank or '\t'
      * '\n' = new-line
      * l_to_r = scanning from previous to next position
      * r_to_l = scanning from next to previous position
      *  
      * direction	prev_char	this_char	next_char	return
      * ----------------------------------------------------------------------
      *	l_to_r		*		a		a		F
      *	  ""		*		a		-		T
      *	  ""		*		a		\n		T
      *   ""		*		-		a		T
      *   ""		*		-		-		F
      *   ""		*		-		\n		F
      *   ""		*		\n		a		T
      *   ""		*		\n		-		F
      *   ""		*		\n		\n		F
      *
      * r_to_l		a		a		*		F
      *   ""		-		a		*		T
      *   ""		\n		a		*		T
      *   ""		a		-		*		T
      *   ""		-		-		*		F
      *   ""		\n		-		*		F
      *   ""		a		\n		*		T
      *   ""		-		\n		*		F
      *   ""		\n		\n		*		F
      */

     if (scan_direction == SCAN_PREV_TO_NEXT)
     {
	if ( this_char == NULL || next_char == NULL )
	{
		return (False);	/* must be something wrong */
	}

	if ( (!blank_char(*this_char) &&  blank_char(*next_char))  ||
	     ( blank_char(*this_char) && !blank_char(*next_char)) )
	{
		return(True);
	}

	return (False);

     } else {

	if ( this_char == NULL || prev_char == NULL )
	{
		return (False);   /* must be something wrong */
	}

	if ( (!blank_char(*this_char) &&  blank_char(*prev_char)) ||
	     ( blank_char(*this_char) && !blank_char(*prev_char)) )
	{
		return(True);
	}

	return (False);
     }
}

/* I18N START */
/*
 *  This routine initializes the context in preparation for
 *  compound string construction.
 *
 *  The return status is returned in context -> status.
 *  Currently, the return status is either I18nCvtStatusOK
 *  or I18nCvtStatusFail.
 */

void
#ifdef _NO_PROTO
_I18nCSConstructInit(context)
    I18nCvtContext context;
#else
_I18nCSConstructInit( I18nCvtContext context )
#endif
{
    I18nCSConstructContext  construct_context;
    int                     len;
    
    construct_context =
         (I18nCSConstructContext) Xmalloc(sizeof(I18nCSConstructContextRec));

    context->stream_context = (Opaque *) construct_context;

    if (!context->stream_context)
    {
        context->status = I18nCvtStatusFail;
        return;
    }
    construct_context->realloc_count = 0;
    construct_context->total_realloc = 0;
    construct_context->alloc_incr = get_cs_alloc_hint(1); /* size not known yet */
    construct_context->cs_len = 0;
    construct_context->header_size =
        _calc_header_size(construct_context->cs_len);
    construct_context->cs_max_len =
        ROUNDUP(construct_context->header_size, construct_context->alloc_incr) -
        construct_context->header_size;
    construct_context->cs = (unsigned char *)
			       Xmalloc(construct_context->header_size +
                                       construct_context->cs_max_len);
    if (!construct_context->cs)
    {
        context -> status = I18nCvtStatusFail;
        Xfree(context->stream_context);
        return;
    }
    (void) _write_header(construct_context->cs,
                         construct_context->cs_len);
    
    context->status = I18nCvtStatusOK;
}

/*
 *  This routine must be called to signal the end of
 *  the compound string construction process.  Without
 *  calling this, the compound string in context -> stream
 *  may or may not be correct.
 *
 *  This routine will return immediately if context -> status
 *  is I18nCvtStatusFail.
 *
 *  The return status is returned in context -> status.
 *  Currently, the status returned is either I18nCvtStatusOK
 *  or I18nCvtStatusFail.  (Failure status is retained from
 *  prior calls, not set here.)
 */

void
#ifdef _NO_PROTO
_I18nCSConstructEnd(context)
    I18nCvtContext context;
#else
_I18nCSConstructEnd( I18nCvtContext context )
#endif
{
    I18nCSConstructContext  construct_context;

    if (context->status == I18nCvtStatusFail)
        return;
    
    construct_context = (I18nCSConstructContext) context->stream_context;

    /*
     *  The header is not right yet.  Do it.
     */
     
    (void) _write_header(construct_context->cs,
                         construct_context->cs_len);

    /*
     *  Return the resulting compound string.  Make
     *  construct_context -> cs null so that it won't
     *  get accidentally freed.  _XmStringFreeCvtContext
     *  will take care of freeing the stream_context
     *  for us.  So don't need to worry about it.
     */
    
    context->stream = (Opaque *) construct_context->cs;
    construct_context->cs = (unsigned char *) NULL;
    
    context->status = I18nCvtStatusOK;
#ifdef PRINT_STATISTICS
    printf("{to CS realloc %d}",construct_context->total_realloc);
#endif
}

/*
 *  This routine is called to construct a compound string segment.
 *
 *  This routine will return immediately if context -> status
 *  is I18nCvtStatusFail.
 *
 *  The return status is put in context -> status.  Currently,
 *  the routine returns either I18nCvtStatusOK or I18nCvtStatusDataLoss.
 *  The latter status indicating that the segment was not constructed
 *  successfully.  It may also return I18nCvtStatusFail that was set
 *  from a prior call.
 */

void
#ifdef _NO_PROTO
_I18nCSConstructSegment(context)
    I18nCvtContext context;
#else
_I18nCSConstructSegment( I18nCvtContext context )
#endif
{
    unsigned char    *p, *curr_charset;
    int              charset_len;
    unsigned char    rev_direction;

    if (context->status == I18nCvtStatusFail)
        return;
    
    /*
     *  If context -> charset is null or XmSTRING_DEFAULT_CHARSET,
     *  use the default values from this locale.
     */
    
/*
 *  PACFIX - what about XmSTRING_DEFAULT_CHARSET used to be ""?
 */

    if (!context->charset ||
        (strcmp((char *)context->charset, XmSTRING_DEFAULT_CHARSET) == 0))
    {
	curr_charset = (unsigned char *) _I18nGetCurrentCharset();
        context->direction = ISO_LATIN1_DIRECTION;
    } else
        curr_charset = (unsigned char *)
			_I18nCharsetCanonicalize((String)context -> charset);

    charset_len = strlen((char *)curr_charset);
	
    /*
     *  First, a direction component.
     */
    
    if (!i18n_motif_113)		/* Motif 1.2 or later */
	p = alloc_component(context,
	    _asn1_size(sizeof(XmStringDirection)) + sizeof(XmStringDirection));
    else
	p = alloc_component(context,HEADER + sizeof(XmStringDirection));

    if (!p)
    {
        context -> status = I18nCvtStatusDataLoss;
        return;
    }
    _write_component(p, (unsigned char) XmSTRING_COMPONENT_DIRECTION, 
                     (unsigned short) sizeof(XmStringDirection), &context->direction, TRUE);

    /*
     *  Then, a charset component.
     */
    
    if (!i18n_motif_113)		/* Motif 1.2 or later */
	p = alloc_component(context, _asn1_size(charset_len) + charset_len);
    else
	p = alloc_component(context, HEADER + charset_len);

    if (!p)
    {
        context -> status = I18nCvtStatusDataLoss;
        return;
    }
    _write_component(p, (unsigned char) XmSTRING_COMPONENT_CHARSET, 
                     (unsigned short) charset_len, 
                     (unsigned char *) curr_charset, TRUE);

    /*
     *  Then, a text component.
     */
    
    if (!i18n_motif_113)		/* Motif 1.2 or later */
	p = alloc_component(context, _asn1_size(context->byte_length) + 
				     context->byte_length);
    else
	p = alloc_component(context, HEADER + context -> byte_length);

    if (!p)
    {
        context -> status = I18nCvtStatusDataLoss;
        return;
    }
    _write_component(p, (unsigned char) XmSTRING_COMPONENT_TEXT, 
                     (unsigned short) context->byte_length,
                     (unsigned char *) context->text, TRUE);

    /*
     *  Finally, a REVERT direction.
     */
    
    if (!i18n_motif_113)		/* Motif 1.2 or later */
	p = alloc_component(context,_asn1_size(sizeof(XmStringDirection)) + 
				    sizeof(XmStringDirection));
    else
	p = alloc_component(context,HEADER + sizeof(XmStringDirection));

    if (!p)
    {
        context->status = I18nCvtStatusDataLoss;
        return;
    }

    rev_direction = ((XmStringDirection) XmSTRING_DIRECTION_REVERT);

    _write_component (p, (unsigned char) XmSTRING_COMPONENT_DIRECTION, 
                      (unsigned short) sizeof(XmStringDirection), 
                      (unsigned char *) &rev_direction, TRUE);

    context->status = I18nCvtStatusOK;
}

/*
 *  This routine is called to construct a compound string separator.
 *
 *  The return status is put in context -> status.  Currently,
 *  the routine returns either I18nCvtStatusOK or I18nCvtStatusDataLoss.
 *  The latter status indicating that the separator was not constructed
 *  successfully.  It may also return I18nCvtStatusFail that was set
 *  from a prior call.
 */

void
#ifdef _NO_PROTO
_I18nCSConstructLine(context)
    I18nCvtContext context;
#else
_I18nCSConstructLine( I18nCvtContext context )
#endif
{
    unsigned char    *p;
    int              i;
    
    if (context->status == I18nCvtStatusFail)
        return;
    
    if (!i18n_motif_113)		/* Motif 1.2 or later */
	p = alloc_component(context,_asn1_size(0));
    else
	p = alloc_component(context,HEADER);

    if (!p)
    {
        context -> status = I18nCvtStatusDataLoss;
        return;
    }
    _write_component(p, (unsigned char) XmSTRING_COMPONENT_SEPARATOR,
                     (unsigned short) 0, (unsigned char *) NULL, TRUE);
    
    context -> status = I18nCvtStatusOK;
}

/*
 *  This is the init callback for converting CS to FC.
 *
 *  The return status is returned in context -> status.
 */

void
#ifdef _NO_PROTO
_I18nCvtCStoFCInit(context)
    I18nCvtContext context;
#else
_I18nCvtCStoFCInit( I18nCvtContext context )
#endif
{
    I18nFCConstructContext  construct_context;
    int                     cs_len, len;
    
    /*
     *  Initialize the memory context.
     */
    
    construct_context = (I18nFCConstructContext)
			Xmalloc(sizeof(I18nFCConstructContextRec));

    context->memory_context = (Opaque *) construct_context;

    if (!context->memory_context)
    {
        context->status = I18nCvtStatusFail;
        return;
    }
    
    construct_context->realloc_count = 0;
    construct_context->total_realloc = 0;
    cs_len = _read_string_length ((unsigned char *) context->stream) + 
             _read_header_length ((unsigned char *) context->stream);
    construct_context->alloc_incr = construct_context->buffer_len =
      get_fc_alloc_hint(cs_len);
    construct_context->status = I18nCvtStatusOK;
    context->memory = (Opaque *) Xmalloc(construct_context->buffer_len);
    if (!context->memory)
    {
        context->status = I18nCvtStatusFail;
        Xfree(context->memory_context);
        return;
    }
    context->memory_length = 0;
    
    /*
     *  Initialize the segment variables in the context.
     *  Don't bother with the other fields because 
     *  we are not using them and _XmStringAllocateCvtContext
     *  had set them to null.
     *
     *  Put the char set into allocated memory because
     *  _XmStringParseStream and _XmStringFreeCvtContext will
     *  attempt to free it.
     */
    
    context->charset = (unsigned char *) _I18nGetCurrentCharset();
    
    context->direction = ISO_LATIN1_DIRECTION;
    context->nest_level = 0;
    
    context->byte_length = 0;
    context->text = (unsigned char *) NULL;
    
    context->status = I18nCvtStatusOK;
}

/*
 *  This is the end callback for converting CS to FC.
 *
 *  The return status is returned in context -> status.
 */

void
#ifdef _NO_PROTO
_I18nCvtCStoFCEnd(context)
    I18nCvtContext context;
#else
_I18nCvtCStoFCEnd( I18nCvtContext context )
#endif
{
    I18nFCConstructContext  construct_context;
    
    if (context->status == I18nCvtStatusFail)
        return;
    
    construct_context = (I18nFCConstructContext) context->memory_context;
    
    /*
     *  We report the most severe error encountered thus far.
     */
    
    if (construct_context->status != I18nCvtStatusOK)
        context->status = construct_context->status;
    else
        context->status = I18nCvtStatusOK;

    /*
     *  _XmStringFreeCvtContext will free the memory_context for us.
     *  Nothing to do, just return.
     */

#ifdef PRINT_STATISTICS
    printf("{to FC realloc %d}",construct_context->total_realloc);
#endif
    return;
}

/*
 *  This is the segment callback for converting CS to FC.
 *
 *  The return status is returned in context -> status.
 */

void
#ifdef _NO_PROTO
_I18nCvtCStoFCSegment(context)
    I18nCvtContext context;
#else
_I18nCvtCStoFCSegment( I18nCvtContext context )
#endif
{
    I18nFCConstructContext  construct_context;
    unsigned char           *new_memory;
    int                     new_size;
    char		    *current_charset;
    Boolean		    compare;
    Boolean                 compare2;
    
    /*
     *  Do nothing if there was a severe error earlier.
     */
    
    if (context->status == I18nCvtStatusFail)
        return;
    
    construct_context = (I18nFCConstructContext) context->memory_context;
    
    /*
     *  If charset is not the default charset,  *OR* the default
     *  charset for a LOCALE Text segment, just ignore the segment.
     */
     
    current_charset = _I18nGetCurrentCharset();
    compare = strcmp((char *)context->charset, current_charset );
    compare2 = strcmp((char *)context->charset, XmFONTLIST_DEFAULT_TAG);
    if ( compare  && compare2 )
    {
        context->status = construct_context->status = I18nCvtStatusDataLoss;
        return;
    }
    
    /*
     *  Copy the text as is without regard to directionality.
     */
    
    if (context->memory_length + context->byte_length >
        construct_context->buffer_len)
    {
        new_size = ROUNDUP(context->memory_length + context->byte_length,
                           construct_context->alloc_incr);
        new_memory = (unsigned char *)
		     do_realloc((unsigned char *)context->memory,
                                new_size,
                                construct_context->buffer_len);
        if (!new_memory)
        {
            context->status = I18nCvtStatusFail;
            return;
        }
        construct_context->total_realloc++;
        construct_context->realloc_count++;
        if (construct_context->realloc_count == REALLOC_CRITICAL_PT)
        {
            construct_context->alloc_incr *= 2;
            construct_context->realloc_count = 0;
        }
        context->memory = (Opaque *) new_memory;
        construct_context->buffer_len = new_size;
    }
    memcpy(&((unsigned char *) context->memory)[context->memory_length],
           context->text, context->byte_length);
    context->memory_length += context->byte_length;
    
    context->status = I18nCvtStatusOK;
}

/*
 *  This is the line callback for converting CS to FC.
 *
 *  The return status is returned in context -> status.
 *
 *  The text field in the context will not be examined.  It
 *  is assumed that the parser will make all necessary segment
 *  callbacks before a separator.
 */

void
#ifdef _NO_PROTO
_I18nCvtCStoFCLine(context)
    I18nCvtContext context;
#else
_I18nCvtCStoFCLine( I18nCvtContext context )
#endif
{
    I18nFCConstructContext  construct_context;
    unsigned char           *new_memory;
    int                     new_size;
    
    /*
     *  Do nothing if there was a severe error earlier.
     */
    
    if (context->status == I18nCvtStatusFail)
        return;
    
    construct_context = (I18nFCConstructContext) context->memory_context;
    
    /*
     *  Generate a new line in the file code.
     */
    
    if (context->memory_length + 1 > construct_context->buffer_len)
    {
        new_size = context->memory_length + construct_context->alloc_incr;
        new_memory = (unsigned char *)
		     do_realloc((unsigned char *)context->memory,
                                new_size,
                                construct_context->buffer_len);
        if (!new_memory)
        {
            context->status = I18nCvtStatusFail;
            return;
        }
        construct_context->total_realloc++;
        construct_context->realloc_count++;
        if (construct_context->realloc_count == REALLOC_CRITICAL_PT)
        {
            construct_context->alloc_incr *= 2;
            construct_context->realloc_count = 0;
        }
        context->memory = (Opaque *) new_memory;
        construct_context->buffer_len = new_size;
    }
    ((unsigned char *) context->memory)[context->memory_length++] = '\n';
    
    context->status = I18nCvtStatusOK;
}

/*
 *  This is the init callback for converting CS to OS.
 *  This routine treats OS code as the same as FC.
 */

void
#ifdef _NO_PROTO
_I18nCvtCStoOSInit(context)
    I18nCvtContext context;
#else
_I18nCvtCStoOSInit( I18nCvtContext context )
#endif
{
    
    _I18nCvtCStoFCInit(context);
}

/*
 *  This is the end callback for converting CS to OS.
 *  This routine treats OS code as the same as FC.
 */

void
#ifdef _NO_PROTO
_I18nCvtCStoOSEnd(context)
    I18nCvtContext context;
#else
_I18nCvtCStoOSEnd( I18nCvtContext context )
#endif
{
    _I18nCvtCStoFCEnd(context);
}

/*
 *  This is the segment callback for converting CS to OS.
 *  This routine treats OS code as the same as FC.
 */

void
#ifdef _NO_PROTO
_I18nCvtCStoOSSegment(context)
    I18nCvtContext context;
#else
_I18nCvtCStoOSSegment( I18nCvtContext context )
#endif
{
    _I18nCvtCStoFCSegment(context);
}

/*
 *  This is the line callback for converting CS to OS.
 *  This routine treats OS code as the same as FC.
 */

void
#ifdef _NO_PROTO
_I18nCvtCStoOSLine(context)
    I18nCvtContext context;
#else
_I18nCvtCStoOSLine( I18nCvtContext context )
#endif
{
    _I18nCvtCStoFCLine(context);
}

/*
 *  Convert FC to CS.  The text between newlines are put into
 *  a single segment and newlines are replaced by separators
 *  in the compound string.  If the FC is zero-length, a null
 *  text segment will be generated to ensure we have at least
 *  one segment in the compound string.
 */

void
#ifdef _NO_PROTO
_I18nCvtFCtoCS(context, status)
    I18nCvtContext context;
    int           *status;
#else
_I18nCvtFCtoCS( I18nCvtContext context,
	       int           *status )
#endif
{
    unsigned char    *start, *end, *seg_start, *p;
    unsigned char    *buffer, *new_buf;
    int		     buffer_len, new_len;
    int              len, charset_len;
    int		     construct_status;
    
    /*
     *  Pick up the status from the init callback and
     *  go no further if the status was failure.
     */
    
    construct_status = context->status;
    if (construct_status == I18nCvtStatusFail)
    {
        construct_status = I18nCvtStatusFail;
        goto cleanup;
    }
    
    /*
     *  Segment initial values must be done here, not
     *  in the init callback invoked by _XmStringConstructInit
     *  because the initial values can only be known by
     *  examining the text.  In ISO Latin-1 case, there is
     *  one and only one value for the initial char set
     *  and direction.  So don't need to examine text to
     *  find out.
     */

    context->charset = (unsigned char *) _I18nGetCurrentCharset();

    context->direction = ISO_LATIN1_DIRECTION;
    context->nest_level = 0;
    
    buffer = (unsigned char *) Xmalloc(TEXT_BUF_INCR);
    if (!buffer)
    {
        construct_status = I18nCvtStatusFail;
        goto cleanup;
    }
    buffer_len = TEXT_BUF_INCR;
    seg_start = start = (unsigned char *) context->memory;
    end = start + context->memory_length;
    
    /*
     *  Construct a segment for all characters between newlines.
     *  Each newline signals the start of a line.
     */
    
    for (p = start; p < end; p++)
    {
        if (*p == '\n')
        {
            /*
             *  Put all text before newline into a segment.
             *  If there is no text, make no segment callback.
             */
            
            len = p - seg_start;
            if (len > 0)
            {
                if (len > buffer_len)
                {
                    new_len = ROUNDUP(buffer_len + len, TEXT_BUF_INCR);
                    new_buf = (unsigned char *)
				do_realloc(buffer, new_len, buffer_len);
                    if (!new_buf)
                    {
                        construct_status = I18nCvtStatusFail;
                        goto cleanup;
                    }
                    buffer = new_buf;
                    buffer_len = new_len;
                }
                context->text = buffer;
                memcpy(context->text, seg_start, len);
                context->byte_length = len;
                (*context->callback[I18nCvtSegmentCallback])(context);
    
                /*
                 *  Retain the most serious status as the construct_status.
                 */
    
                if (context->status == I18nCvtStatusFail)
                {
                    construct_status = I18nCvtStatusFail;
                    goto cleanup;
                } else if (context->status != I18nCvtStatusOK ||
                           construct_status == I18nCvtStatusOK)
                    construct_status = context -> status;
            }
            
            /*
             *  Before making line callback, set the text and
             *  length field to null and zero respectively.
             *  This is just not to misinform the line callback
             *  that there is some text.
             */
            
            context->text = (unsigned char *) NULL;
            context->byte_length = 0;
            (*context -> callback[I18nCvtLineCallback])(context);
            
            /*
             *  Retain the most serious status as the construct_status.
             */
    
            if (context->status == I18nCvtStatusFail)
            {
                construct_status = I18nCvtStatusFail;
                goto cleanup;
            } else if (context->status != I18nCvtStatusOK ||
                       construct_status == I18nCvtStatusOK)
                construct_status = context -> status;
            
            seg_start = p + 1;
        }
    }
    
    /*
     *  Construct the last text segment, if necessary.
     *  If the fc is zero-length, we will make a text
     *  segment with zero length so that we have at
     *  least one text segment in the compound string
     *  that we create.
     */
    
    context->byte_length = end - seg_start;
    if (context->byte_length > 0)
    {
        if (context->byte_length > buffer_len)
        {
            new_len = ROUNDUP(buffer_len + context->byte_length,
                              TEXT_BUF_INCR);
            new_buf = (unsigned char *) do_realloc(buffer, new_len, buffer_len);
            if (!new_buf)
            {
                construct_status = I18nCvtStatusFail;
                goto cleanup;
            }
            buffer = new_buf;
            buffer_len = new_len;
        }
        context -> text = buffer;
        memcpy(context->text, seg_start, context->byte_length);
        (*context -> callback[I18nCvtSegmentCallback])(context);
    }
    else if (context->memory_length == 0)
    {
        context->text = (unsigned char *) NULL;
        (*context -> callback[I18nCvtSegmentCallback])(context);
    }
    
    /*
     *  Retain the most serious status as the construct_status.
     */
    
     if (context->status == I18nCvtStatusFail ||
         context->status != I18nCvtStatusOK   ||
         construct_status == I18nCvtStatusOK)
         construct_status = context->status;
            
    cleanup:

    /*
     *  Clean-up segment variables before making end callback.
     */
    
    context->byte_length = 0;
    context->text = (unsigned char *) NULL;
    
    /*
     *  Make the end callback.
     */
    
    (*context->callback[I18nCvtEndCallback])(context);
    
    /*
     *  Retain the most serious status as the construct_status.
     */
    
    if (construct_status != I18nCvtStatusFail)
    {
        if (context->status != I18nCvtStatusOK ||
            construct_status == I18nCvtStatusOK)
            construct_status = context->status;
    }
    
    /*
     *  Clean up.  The return status is in construct_context.
     */
    
     Xfree(buffer);
     *status = construct_status;
}

/*
 *  Convert OS to CS.  This routine treats OS code the same as FC.
 */

void
#ifdef _NO_PROTO
_I18nCvtOStoCS(context, status)
    I18nCvtContext context;
    int           *status;
#else
_I18nCvtOStoCS( I18nCvtContext context,
	       int           *status )
#endif
{
    _I18nCvtFCtoCS(context, status);
}


/*
 *  The following routine is used to load the I18N shareable image
 *  with no other side effect.
 */
#ifdef _NO_PROTO
void _I18nLoadShareable()
#else
void _I18nLoadShareable(void)
#endif /* _NO_PROTO */
{
    return;
}

#endif /* DEC_EXTENSION */
