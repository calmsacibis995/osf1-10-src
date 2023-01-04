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
/******************************************************************************

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
**
**  MODIFICATION HISTORY:
**
**  Nov 30 : add Dynamic Image Activation coding; commented by  "DIA"
**  Aug 27, 1992 : update for Motif 1.2, make the whole module a
**                 Digital extension
**  Nov 13, 1992 : Remove include of Descripu.h for Ultrix
**
**--
**/

/*
 *  Note:
 *     For Motif 1.2, a structural change has been made to the modularity
 *  of this and the I18N_EN.c file.
 *     In Motif 1.1, the I18N_EN.c file was added late enough in the VMS
 *  DW Motif 1.1 project that the VMS DW engineering group was unwilling to
 *  accept a new module.  But, in order to make the modularity work for the
 *  ULTRIX platform, the I18N_EN.c module needed to exist there.
 *
 *     The solution was for the I18N routines in this module to behave
 *  differently on Ultrix than on VMS/OSF/1.  On ULTRIX, the I18N routine
 *  would just call the _I18n routine in I18N_EN.c (the language specific
 *  routine).  On VMS/OSF/1, the I18N routine would attempt to call into
 *  the I18N shareable library for the _I18N routine.  But if it failed,
 *  it still needed to do something.  Since another file (I18N_EN.c) was
 *  not acceptible, it was necessary to duplicate the _I18N routine's
 *  functionality in the I18N routines here.  A waste of code space, a
 *  bad maintenance problem, etc.
 *
 *     For Motif 1.2, the politics have changed, and so we can correct this
 *  situation.  So, now, the I18N_EN.c file contains the _I18n routines, like
 *  before.  There is VMS/OSF/1 (what about SUN?) code, that attempts to
 *  call into the language-specific I18n shareable for the _I18n routine.  If
 *  it fails, or we're not on a shared-library platform, we just go ahead
 *  and call the _I18n routines that live in the I18N_EN.c module in this
 *  English version.
 *     Note that the local engineering groups still need only replace the
 *  I18_EN.c module with their own language-specific version.
 *
 *     And, one other thing - this whole module is Digital-specific, so the
 *  whole thing is ifdef'ed as such.  The file would exist if we built with
 *  Digital extensions off, but this module would be empty.
 *
 *  Pat    August 27, 1992
 *
 */

#ifdef DEC_EXTENSION

#include <ctype.h>		/* I18N */
#include <X11/Intrinsic.h>
#include "I18N.h"
#include "I18nConverter.h"	/* I18N */

#ifdef VMS
#include "descrip.h"
#include "ssdef.h"
#include "libdef.h"
#include "rmsdef.h"
#else
#ifdef __osf__
#ifdef __alpha
#include <stdio.h>
#include <dlfcn.h>
#else
#include <sys/types.h>
#include <loader.h>
#endif /* __alpha */
#endif /* __osf__ */
#endif /* VMS */


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


#ifdef VMS

typedef (*I18nProc) ();

static Boolean I18n_have_looked = False;
static Boolean image_found      = False;

static int
exception_handler (sigargs, mchargs)
	unsigned long sigargs[];
	unsigned long mchargs[5];
{
    /*BOGUS should check to see if file-not-found or key-not-found before
       returning SS$_CONTINUE
     */
    return ( SS$_CONTINUE );

}

extern I18nContext _I18nGlobalContextBlock;

static
I18nProc _locale_specific_found ( func_addr, symbol_name )
    I18nProc func_addr;
    char     *symbol_name;
{

    /* case when func_addr is assigned already
     */
    if ( I18n_have_looked && func_addr != NULL )
    {
	return (func_addr);

    } else if ( !I18n_have_looked || ((func_addr == NULL ) && (image_found))) {

        long cond_value;
        struct dsc$descriptor_s func_symbol;

/*
        $DESCRIPTOR(file_name,   "DECW$DXM_I18NLIB");
        $DESCRIPTOR(image_name,  "SYS$LIBRARY:DECW$DXM_I18NLIB.EXE");
*/
        struct dsc$descriptor_s file_name;
	char file_name_string[256];
	char * lang;

        func_symbol.dsc$w_length  = strlen(symbol_name);
        func_symbol.dsc$b_dtype   = DSC$K_DTYPE_T;
        func_symbol.dsc$b_class   = DSC$K_CLASS_S;
        func_symbol.dsc$a_pointer = symbol_name;

        /* not tried yet, or func_addr == NULL, try it now
         */
        if ( (!I18n_have_looked) || ((I18n_have_looked) && (image_found)) )
        {

             if (!I18n_have_looked)
             {
                VAXC$ESTABLISH ( exception_handler );
             }
             I18n_have_looked = True;

	     strcpy ( file_name_string, "DECW$DXM_I18NLIB12" );
	     if( _I18nGlobalContextBlock ){
	       lang = _I18nGlobalContextBlock->locale;
	       if ( lang && *lang ){
		 char *dot;
		 strcat ( file_name_string, "_" );
		 strcat ( file_name_string, lang );
		 if ( dot = strchr ( file_name_string, '.') )
		   *dot = '\0';		/* ignore .codeset@modifier */
	       }
	     }
             file_name.dsc$w_length  = strlen(file_name_string);
             file_name.dsc$b_dtype   = DSC$K_DTYPE_T;
             file_name.dsc$b_class   = DSC$K_CLASS_S;
             file_name.dsc$a_pointer = file_name_string;

             cond_value = LIB$FIND_IMAGE_SYMBOL ( &file_name,
				                  &func_symbol,
				                  &func_addr,
/*				                  &image_name ); */
						  NULL);

             /* symbol exists, return
              */
             if ( cond_value == SS$_NORMAL )
             {
                 image_found = True;
	         return (func_addr);

	     } else {

                 LIB$REVERT();  /* no image found, reset the exception handler
                                 */
                 image_found = False;
	         return (NULL);
             }

	} else {     /* image not exist, return */

               image_found = False;
               return (NULL);
        }

    } else {

	return (NULL);
    }
}


#endif /* VMS */


#ifdef __osf__

typedef (*I18nProc) ();

static Boolean I18n_have_looked = False;
static Boolean image_found      = False;

extern I18nContext _I18nGlobalContextBlock;

static
I18nProc _locale_specific_found ( func_addr, symbol_name )
    I18nProc func_addr;
    char     *symbol_name;
{
#ifdef __alpha
    void *handle = NULL;
#else
    ldr_module_t handle = 0;
#endif
    char file_name_string[256];
    char *lang, *path;

    /* case when func_addr is assigned already
     */
    if ( I18n_have_looked && func_addr != NULL )
    {
	return (func_addr);

    } else if ( !I18n_have_looked || ((func_addr == NULL ) && (image_found))) {

        /* not tried yet, or func_addr == NULL, try it now
         */
        if ( (!I18n_have_looked) || ((I18n_have_looked) && (image_found)) )
        {
             I18n_have_looked = True;

	     path = (char *) getenv ("I18NPATH");

	     if (path)
	     {
		strcpy ( file_name_string, path);
		strcat ( file_name_string, "/");
	     }
	     else		
	     {
		strcpy ( file_name_string, "/usr/shlib/");
	     }

	     strcat ( file_name_string, "DECW_DXM_I18NLIB");
	     if( _I18nGlobalContextBlock ){
	       lang = _I18nGlobalContextBlock->locale;
	       if ( lang && *lang ){
		 char *dot;
		 strcat ( file_name_string, "_" );
		 strcat ( file_name_string, lang );
		 if ( dot = strchr ( file_name_string, '.') )
		   *dot = '\0';		/* ignore .codeset@modifier */
	       }
	     }
	     strcat ( file_name_string, ".so");
   
#ifdef __alpha
	    handle = dlopen(file_name_string, RTLD_LAZY);
	    if (handle != NULL) {
		func_addr = (I18nProc) dlsym(handle, symbol_name);
#else
	    handle = load(file_name_string, NULL);
	    if (handle > 0) {
		func_addr = ldr_lookup_package("I18N", symbol_name);
#endif

		if (func_addr && func_addr != (void *)-1) {
		    image_found = True;
		    return (func_addr);
		}
		else {
		    /* Routine didn't exist in the loaded shareable */
		    return (NULL);
		}
	    }
	    else {
		image_found = False;
		return (NULL);
	    }
       } else {
	        image_found = False;
		return (NULL);
         }
    } else {
         return (NULL);
      }
}

#endif /* __osf__ */



/*
 * Function prototyping added for all routines
 */

#ifdef _NO_PROTO
#else
#endif

#ifdef _NO_PROTO
Boolean I18nRemapFontname (old, new)
    I18NFontNamePtr	old;
    I18NFontNamePtr	new;
#else
Boolean I18nRemapFontname ( I18NFontNamePtr	old,
			    I18NFontNamePtr	new )
#endif /* _NO_PROTO */
{
    int i;


#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nRemapFontname")) != NULL)
    {
        return ( (Boolean) (*a) (old,new) );
    }
#endif

    /*
     * if the I18N shareable is missing, or we're on a non-shareable
     * platform (ULTRIX, maybe SUN), then just called language-specific
     * routine.
     */

     return (Boolean)_I18nRemapFontname(old, new);
}










/*
 *************************************************************************
 *
 * Default FontList for DECwindows toolkit widgets
 */

/* TBS */



/*
 *************************************************************************
 *
 * Provide locale sensitive Motif compound string version of toolkit 
 * ASCII default text values such as "OK", "Cancel" etc.
 */


void I18nCvtFCtoCS();

Boolean
#ifdef _NO_PROTO
I18nGetLocaleString ( context, cvtcontext, ascii, word_type )
char		*context;
I18nCvtContext	cvtcontext;
char		*ascii;
I18nWordType	word_type;
#else
I18nGetLocaleString ( char		*context,
		      I18nCvtContext	cvtcontext,
		      char		*ascii,
		      I18nWordType	word_type )
#endif /* _NO_PROTO */
{

    int status;

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nGetLocaleString")) != NULL)
    {
        return ( (Boolean)(*a)(context, cvtcontext, ascii, word_type) );
    }
#endif

    return (Boolean)_I18nGetLocaleString(context, cvtcontext, ascii,
					     word_type);

}

unsigned long
#ifdef _NO_PROTO
I18nGetLocaleMnemonic(context,widget_class,mnemonic,charset,returned_mnemonic)
I18nContext     context;
char *          widget_class;       /* class to help I18N layer to translate */
char *          mnemonic;           /* mnemonic string                       */
char *          charset;            /* The requested charset of the mnemonic */
char **         returned_mnemonic;   /* translated mnemonic                  */
#else
I18nGetLocaleMnemonic( I18nContext     context,
		       char *          widget_class,
		       char *          mnemonic,
		       char *          charset,
		       char **         returned_mnemonic )
#endif /* _NO_PROTO */
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nGetLocaleMnemonic")) != NULL)
    {
        return ( (unsigned long)(*a)( context, widget_class, mnemonic, charset,
				      returned_mnemonic ) );
    }
#endif

	return (unsigned long)_I18nGetLocaleMnemonic(context,
						     widget_class,
						     mnemonic,
						     charset,
						     returned_mnemonic);
}

#ifdef _NO_PROTO
char *I18nGetLocaleCharset( context )
char *context;
#else
char *I18nGetLocaleCharset( char *context )
#endif /* _NO_PROTO */
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nGetLocaleCharset")) != NULL)
    {
        return ( (char *)(*a)( context ) );
    }
#endif
	
    return (char *)_I18nGetLocaleCharset(context);

}


#ifdef _NO_PROTO
char **I18nGetLocaleCharsets( context )
char *context;
#else
char **I18nGetLocaleCharsets( char *context )
#endif /* _NO_PROTO */
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nGetLocaleCharsets")) != NULL)
    {
        return ( (char **)(*a)( context ) );
    }
#endif

    return (char **)_I18nGetLocaleCharsets(context);

}



/*
 *************************************************************************
 *
 * convert character range information into byte offset information
 */

#ifdef _NO_PROTO
void I18nSegmentMeasure( context, 
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
void I18nSegmentMeasure( char *context,
			 char *charset,
			 char *text,
			 int character_offset,
			 int number_characters,
			 int *byte_offset_start,
			 int *byte_offset_end )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nSegmentMeasure")) != NULL)
    {
        (*a)( context, charset, text, character_offset,
	      number_characters, byte_offset_start,
	      byte_offset_end );

        return;
    }
#endif

    _I18nSegmentMeasure(context, charset, text, character_offset,
			    number_characters, byte_offset_start,
			    byte_offset_end);
    return;

}

/*======================================================================
 *
 * Return the segment character length, as opposed to byte length
 *
 */
#ifdef _NO_PROTO
void I18nSegmentLength( context,
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
void I18nSegmentLength( Opaque context,
			char   *charset,
			Opaque text,
			int    byte_length,
			int    *char_length )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nSegmentLength")) != NULL)
    {
        (*a)( context, charset, text, byte_length, char_length );

        return;
    }
#endif

    _I18nSegmentLength(context, charset, text, byte_length, char_length);
    return;

}


/*
 * This routine allows compound string segments shich do not encode characters
 * as actual glyph indexes the following entrypoint and data structure are
 * provided.  Note that this is designed to allow not just a simple mapping
 * but an arbitrary mapping of a compound string segment into Xlib rendering
 * segments.
 */
#ifdef _NO_PROTO
void I18nCreateXlibBuffers ( context,
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
void I18nCreateXlibBuffers ( I18nContext	context,
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

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nCreateXlibBuffers")) != NULL)
    {
        (void) (*a) ( context,
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
		      buffer_count );	/* output */

        return;
    }
#endif

    (void) _I18nCreateXlibBuffers(context,
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
				      buffers,	
				      buffer_count );

     return;
}

extern I18nContext _I18nGlobalContextBlock;

/* This routine doesn't have the _I18nxxx equivalent
 */
#ifdef _NO_PROTO
I18nContext I18nGetGlobalContext ()
#else
I18nContext I18nGetGlobalContext ( void )
#endif
{

    return (_I18nGlobalContextBlock);
}


/*
 * this routine returns a null-terminated string:
 *	"fontname [=charset] [,fontname [=charset]]*"
 */

char *
#ifdef _NO_PROTO
I18nCreateDefaultFontList (context, widget_class_name, resource_name)
I18nContext context;
char	* widget_class_name;
char	* resource_name;
#else
I18nCreateDefaultFontList ( I18nContext context,
			    char	* widget_class_name,
			    char	* resource_name )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nCreateDefaultFontList")) != NULL)
    {
#ifdef DEBUG
	printf("Use I18n default font list\n");
#endif
        return ( (char *) (*a)
			(context, widget_class_name, resource_name) );
    }
#endif

    return ( (char *) _I18nCreateDefaultFontList
			(context, widget_class_name, resource_name) );


}


/* this routine provides a functin which is used by converters to map the 
 * external encoding (e.g. DDIF or file code) into the desired encoding for 
 * a compound string.
 */
#ifdef _NO_PROTO
void I18nMapSegment ( context, input_charset,
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
void I18nMapSegment ( I18nContext	context,
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


#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nMapSegment")) != NULL)
    {
	(void) (*a) ( context, input_charset,
			       input_text,
			       input_char_count,
			       input_byte_count,
			       output_charset,
			       output_text,
			       output_char_count,
			       output_byte_count );
        return;
    }
#endif

    (void) _I18nMapSegment ( context, input_charset,
			       input_text,
			       input_char_count,
			       input_byte_count,
			       output_charset,
			       output_text,
			       output_char_count,
			       output_byte_count );
    return;

}

/*
 * This routine decides if the sub_string occurs within the string.  N.B. the
 * two strings can be assumed to have the same character set.  These strings
 * are the direct segment text buffers, not the decomposed X sub-segment.
 */
#ifdef _NO_PROTO
Boolean I18nHasSubstring ( context, character_set, string, sub_string,
			   string_char_count, sub_char_count )
I18nContext	context;
char 		*character_set;
Opaque		*string;
Opaque		*sub_string;
int		string_char_count;
int		sub_char_count;
#else
Boolean I18nHasSubstring ( I18nContext	context,
			   char 	*character_set,
			   Opaque	*string,
			   Opaque	*sub_string,
			   int		string_char_count,
			   int		sub_char_count )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nHasSubstring")) != NULL)
    {
	return ( (Boolean) (*a) ( context,
				  character_set,
				  string,
				  sub_string,
				  string_char_count,
				  sub_char_count ) );
    }
#endif

    return ( (Boolean) _I18nHasSubstring ( context,
					      character_set,
					      string,
					      sub_string,
					      string_char_count,
					      sub_char_count ) );
}

/*
 * This routine finds the start and end positions of the sub string.  It is 
 * assumed that the sub_string does exist sithin the string.  These strings are
 * the direct segment text buffers, not the decomposed X sub-segments.
 */
#ifdef _NO_PROTO
void I18nSubStringPosition ( context, 
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
void I18nSubStringPosition ( I18nContext	context,
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


#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nSubStringPosition")) != NULL)
    {
	(void) (*a) (	context,
			font,
			character_set,
			string,
			sub_string,
			string_char_count,
			sub_char_count,
			x,
			under_begin,
			under_end );
	return;
    }
#endif

	(void) _I18nSubStringPosition (	context,
				       font,
				       character_set,
				       string,
				       sub_string,
				       string_char_count,
				       sub_char_count,
				       x,
				       under_begin,
				       under_end );
	return;
}

/*
 * This routine returns True if the character is a white space.  For US locale,
 * ' ' and '\t' or '\n' are considered to be white space.  For Asian locale,
 * all the above plus 0xA1A1 should be considered white.
 */
#ifdef _NO_PROTO
Boolean I18nIsWhiteSpace ( context, this_charset, this_char )
I18nContext	context;
char		*this_charset;
Opaque		*this_char;
#else
Boolean I18nIsWhiteSpace ( I18nContext	context,
			   char		*this_charset,
			   Opaque	*this_char )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nIsWhiteSpace")) != NULL)
    {
	return ( (Boolean) (*a) ( context,
				  this_charset,
				  this_char ) );
    }
#endif

	return ( (Boolean) _I18nIsWhiteSpace ( context,
					      this_charset,
					      this_char ) );
}


/*
 * This routine returns True if the indicated character should be considered
 * as the bounds of a scan operation.  The scan_direction field equals 0 if
 * scanning direction for a break is from the previous to the next char and
 * 1 if the scanning from next to previous.
 */
#ifdef _NO_PROTO
Boolean I18nIsScanBreak ( context,
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
Boolean I18nIsScanBreak ( I18nContext	context,
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

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nIsScanBreak")) != NULL)
    {
	return ( (Boolean) (*a) ( context,
				  prev_charset,
				  prev_char,
				  this_charset,
				  this_char,
				  next_charset,
				  next_char,
				  scan_direction,
				  scan_type ) );
    }     
#endif      

	return ( (Boolean) _I18nIsScanBreak ( context,
					     prev_charset,
					     prev_char,
					     this_charset,
					     this_char,
					     next_charset,
					     next_char,
					     scan_direction,
					     scan_type ) );
}

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
I18nCSConstructInit(context)
    I18nCvtContext context;
#else
I18nCSConstructInit( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc         a;

    if (a || (a = _locale_specific_found(a, "_I18nCSConstructInit")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCSConstructInit(context);
    return;
}

/*
 *  This routine must be called to signal the end of
 *  the compound string construction process.  Without
 *  calling this, the compound string in context -> stream
 *  may or may not be correct.
 *
 */

void
#ifdef _NO_PROTO
I18nCSConstructEnd(context)
    I18nCvtContext context;
#else
I18nCSConstructEnd( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc         a;

    if (a || (a = _locale_specific_found(a, "_I18nCSConstructEnd")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCSConstructEnd(context);
    return;
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
I18nCSConstructSegment(context)
    I18nCvtContext context;
#else
I18nCSConstructSegment( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc  a;

    if (a || (a = _locale_specific_found(a, "_I18nCSConstructSegment")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCSConstructSegment(context);
    return;

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
I18nCSConstructLine(context)
    I18nCvtContext context;
#else
I18nCSConstructLine( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc  a;

    if (a || (a = _locale_specific_found(a, "_I18nCSConstructLine")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCSConstructLine(context);
    return;
}

/*
 *  This is the init callback for converting CS to FC.
 *
 *  The return status is returned in context -> status.
 */

void
#ifdef _NO_PROTO
I18nCvtCStoFCInit(context)
    I18nCvtContext context;
#else
I18nCvtCStoFCInit( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc         a;

    if (a || (a = _locale_specific_found(a, "_I18nCvtCStoFCInit")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCvtCStoFCInit(context);
    return;
}

/*
 *  This is the end callback for converting CS to FC.
 *
 *  The return status is returned in context -> status.
 */

void
#ifdef _NO_PROTO
I18nCvtCStoFCEnd(context)
    I18nCvtContext context;
#else
I18nCvtCStoFCEnd( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nCvtCStoFCEnd")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCvtCStoFCEnd(context);
    return;

}

/*
 *  This is the segment callback for converting CS to FC.
 *
 *  The return status is returned in context -> status.
 */

void
#ifdef _NO_PROTO
I18nCvtCStoFCSegment(context)
    I18nCvtContext context;
#else
I18nCvtCStoFCSegment( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nCvtCStoFCSegment")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCvtCStoFCSegment(context);
    return;
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
I18nCvtCStoFCLine(context)
    I18nCvtContext context;
#else
I18nCvtCStoFCLine( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc a;

    if (a || (a = _locale_specific_found(a, "_I18nCvtCStoFCLine")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCvtCStoFCLine(context);
    return;
}

/*
 *  This is the init callback for converting CS to OS.
 *  This routine treats OS code as the same as FC.
 */

void
#ifdef _NO_PROTO
I18nCvtCStoOSInit(context)
    I18nCvtContext context;
#else
I18nCvtCStoOSInit( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc  a;
    
    if (a || (a = _locale_specific_found(a, "_I18nCvtCStoOSInit")) != NULL)
    {
        (void) (*a)(context);
        return;
    }          
#endif

    (void) _I18nCvtCStoOSInit(context);
    return;

}

/*
 *  This is the end callback for converting CS to OS.
 *  This routine treats OS code as the same as FC.
 */

void
#ifdef _NO_PROTO
I18nCvtCStoOSEnd(context)
    I18nCvtContext context;
#else
I18nCvtCStoOSEnd( I18nCvtContext context )
#endif
{


#if defined(VMS) || defined(__osf__)
    static I18nProc  a;
    
    if (a || (a = _locale_specific_found(a, "_I18nCvtCStoOSEnd")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCvtCStoOSEnd(context);
    return;
}

/*
 *  This is the segment callback for converting CS to OS.
 *  This routine treats OS code as the same as FC.
 */

void
#ifdef _NO_PROTO
I18nCvtCStoOSSegment(context)
    I18nCvtContext context;
#else
I18nCvtCStoOSSegment( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc  a;
    
    if (a || (a = _locale_specific_found(a, "_I18nCvtCStoOSSegment")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCvtCStoOSSegment(context);
    return;
}

/*
 *  This is the line callback for converting CS to OS.
 *  This routine treats OS code as the same as FC.
 */

void
#ifdef _NO_PROTO
I18nCvtCStoOSLine(context)
    I18nCvtContext context;
#else
I18nCvtCStoOSLine( I18nCvtContext context )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc  a;
    
    if (a || (a = _locale_specific_found(a, "_I18nCvtCStoOSLine")) != NULL)
    {
        (void) (*a)(context);
        return;
    }
#endif

    (void) _I18nCvtCStoOSLine(context);
    return;
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
I18nCvtFCtoCS(context, status)
    I18nCvtContext context;
    int           *status;
#else
I18nCvtFCtoCS( I18nCvtContext context,
	       int           *status )
#endif
{

#if defined(VMS) || defined(__osf__)
    static I18nProc  a;

    if (a || (a = _locale_specific_found(a, "_I18nCvtFCtoCS")) != NULL)
    {
        (void) (*a)(context, status);
        return;
    }
#endif

    (void) _I18nCvtFCtoCS(context, status);
    return;

}

/*
 *  Convert OS to CS.  This routine treats OS code the same as FC.
 */

void
#ifdef _NO_PROTO
I18nCvtOStoCS(context, status)
    I18nCvtContext context;
    int           *status;
#else
I18nCvtOStoCS( I18nCvtContext context,
	       int           *status )
#endif
{


#if defined(VMS) || defined(__osf__)
    static I18nProc  a;
    
    if (a || (a = _locale_specific_found(a, "_I18nCvtOStoCS")) != NULL)
    {
        (void) (*a)(context, status);
        return;
    }
#endif

    (void) _I18nCvtOStoCS(context, status);
    return;

}


/*
 *  The following routine is used to load the I18N shareable image
 *  with no other side effect.
 */
#ifdef _NO_PROTO
void I18nLoadShareable()
#else
void I18nLoadShareable(void)
#endif /* _NO_PROTO */
{

#if defined(VMS) || defined(__osf__)

    static I18nProc  a;
    
    if (a || (a = _locale_specific_found(a, "_I18nLoadShareable")) != NULL)
    {
        (void) (*a)();
        return;
    }
#endif

    return;
}


#endif /* DEC_MOTIF_EXTENSION */

