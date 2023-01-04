/* dwc_cda_util.c */
#ifndef lint
static char rcsid[] = "$Header$";
#endif /* lint */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; print routines
**
**  AUTHOR:
**
**	David Kubelka, February-1989
**
**  ABSTRACT:
**
**	This is are the CDA utilities
**
**--
*/

#include "dwc_compat.h"
#include <string.h>

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#if defined(VMS)
#define CDA_EXPAND_PROTO 0
#endif
#include <ddifdef.h>
#include <cdadef.h>
#include <cdamsg.h>
#include <cdaptp.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_cda_util.h"
#include "dwc_ui_catchall.h"	

static void foobar
#if defined(_DWC_PROTO_)
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{

#define CHECK_STATUS( s )				\
{   int		_status = s;				\
    if( !( _status&1 ))					\
	DWC$UI_Catchall					\
	(   DWC$PRINT_CDA				\
	,   _status					\
	,   (int) 0					\
	);						\
}

/*
** concatenation is handled differently in ANSI.
*/
#if !defined(__STDC__)

#define	STRING( n, v )					\
static const char n[] = v;				\
int n/**/_len = strlen( v )

#else

#define	STRING( n, v )					\
static const char n[] = v;				\
int n ## _len = strlen( v )

#endif

}

int DWCCDA_Open_Text_File
#if defined(_DWC_PROTO_)
	(
	DWCCDA_TextFileCtx	ctx,
	char		*filespec,
	char		*default_filespec)
#else	/* no prototypes */
	(ctx, filespec, default_filespec)
	DWCCDA_TextFileCtx	ctx;
	char		*filespec;
	char		*default_filespec;
#endif	/* prototype */
{
    int			    status;
    unsigned int	    filespec_len;
    unsigned int	    default_filespec_len;

    filespec_len	    = strlen( filespec );

    if (default_filespec != 0)
	{
	default_filespec_len    = strlen( default_filespec );
	}
    else
	{
	default_filespec_len = 0;
	}
	
    if (default_filespec_len == 0)
	default_filespec = 0;
	
    status = cda_open_text_file
    (
	&filespec_len,
	filespec,
	&default_filespec_len,
	default_filespec,
	0,
	0,
	0,
	&ctx->hndl
    );

    if ( !(status & 1) )
	return 1 == 0;
    else
	return 1 == 1;

}

int DWCCDA_Read_Text_File
#if defined(_DWC_PROTO_)
	(
	DWCCDA_TextFileCtx	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_TextFileCtx	ctx;
#endif	/* prototype */
{
    int			    status;

    status = cda_read_text_file
	(&ctx->hndl, &ctx->length, &ctx->buffer);
    if( status == CDA_ENDOFDOC )
	return 1 == 0;
    else
	CHECK_STATUS( status );

    return 0 == 0;
}

int DWCCDA_Close_Text_File
#if defined(_DWC_PROTO_)
	(
	DWCCDA_TextFileCtx	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_TextFileCtx	ctx;
#endif	/* prototype */
{
    int			    status;

    status = cda_close_text_file (&ctx->hndl);

    if ( !(status & 1) )
	return 1 == 0;
    else
	return 1 == 1;
}

void
DWCCDA_Store_Any_Item
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	int			*buf_adr,
	int			buf_len)
#else	/* no prototypes */
	(ctx, item, buf_adr, buf_len)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Item	item;
	int			*buf_adr;
	int			buf_len;
#endif	/* prototype */
    {   int			    status;

    status = cda_store_item
    (
	&ctx->root,			/* root-aggregate-handle */
	ctx->aggregate_handle,		/* aggregate-handle */
	&item,				/* aggregate-item */
	&buf_len,			/* buf-len */
	buf_adr,			/* buf-adr */
	0,				/* aggregate-index */
	0				/* add-info */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Store_Item_char
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	char			*value)
#else	/* no prototypes */
	(ctx, item, value)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Item	item;
	char			*value;
#endif	/* prototype */
    {
    int			    status;
    int			    buf_len;
    CDA_Y_Add_Info	    add_info;

    buf_len = strlen( value );
    add_info = CDA_K_ISO_LATIN1;

    status = cda_store_item
    (
	&ctx->root,			/* root-aggregate-handle */
	ctx->aggregate_handle,		/* aggregate-handle */
	&item,				/* aggregate-item */
	&buf_len,			/* buf-len */
	value,				/* buf-adr */
	0,				/* aggregate-index */
	&add_info			/* add-info */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Store_Item_char_tag
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	char			*value,
	CDA_Y_Add_Info		add_info)
#else	/* no prototypes */
	(ctx, item, value, add_info)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Item	item;
	char			*value;
	CDA_Y_Add_Info		add_info;
#endif	/* prototype */
    {
    int			    status;
    int			    buf_len;

    buf_len = strlen( value );

    status = cda_store_item
    (
	&ctx->root,			/* root-aggregate-handle */
	ctx->aggregate_handle,		/* aggregate-handle */
	&item,				/* aggregate-item */
	&buf_len,			/* buf-len */
	value,				/* buf-adr */
	0,				/* aggregate-index */
	&add_info			/* add-info */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Store_Item_chars
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	int			length,
	char			*value)
#else	/* no prototypes */
	(ctx, item, length, value)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Item	item;
	int			length;
	char			*value;
#endif	/* prototype */
    {
    int			    status;
    CDA_Y_Add_Info	    add_info;

    add_info = CDA_K_ISO_LATIN1;

    status = cda_store_item
    (
	&ctx->root,			/* root-aggregate-handle */
	ctx->aggregate_handle,		/* aggregate-handle */
	&item,				/* aggregate-item */
	&length,			/* buf-len */
	value,				/* buf-adr */
	0,				/* aggregate-index */
	&add_info			/* add-info */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Store_Item_char_array
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	CDA_Y_Aggregate_Index	aggregate_index,
	char			*value)
#else	/* no prototypes */
	(ctx, item, aggregate_index, value)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Item	item;
	CDA_Y_Aggregate_Index	aggregate_index;
	char			*value;
#endif	/* prototype */
    {
    int			    status;
    int			    buf_len;
    CDA_Y_Add_Info	    add_info;

    buf_len = strlen( value );
    add_info = CDA_K_ISO_LATIN1;

    status = cda_store_item
    (
	&ctx->root,			/* root-aggregate-handle */
	ctx->aggregate_handle,		/* aggregate-handle */
	&item,				/* aggregate-item */
	&buf_len,			/* buf-len */
	value,				/* buf-adr */
	&aggregate_index,		/* aggregate-index */
	&add_info			/* add-info */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Store_Item_int
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	int			value)
#else	/* no prototypes */
	(ctx, item, value)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Item	item;
	int			value;
#endif	/* prototype */
    {
    int			    status;
    int			    buf_len;

    buf_len = sizeof( value );

    status = cda_store_item
    (
	&ctx->root,			/* root-aggregate-handle */
	ctx->aggregate_handle,		/* aggregate-handle */
	&item,				/* aggregate-item */
	&buf_len,			/* buf-len */
	&value,				/* buf-adr */
	0,				/* aggregate-index */
	0				/* add-info */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Store_Item_int_array
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item,
	CDA_Y_Aggregate_Index	aggregate_index,
	int			value)
#else	/* no prototypes */
	(ctx, item, aggregate_index, value)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Item	item;
	CDA_Y_Aggregate_Index	aggregate_index;
	int			value;
#endif	/* prototype */
    {
    int			    status;
    int			    buf_len;

    buf_len = sizeof( value );

    status = cda_store_item
    (
	&ctx->root,			/* root-aggregate-handle */
	ctx->aggregate_handle,		/* aggregate-handle */
	&item,				/* aggregate-item */
	&buf_len,			/* buf-len */
	&value,				/* buf-adr */
	0,				/* aggregate-index */
	0				/* add-info */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Delete_Aggregate
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {   int			    status;

    status = cda_delete_aggregate (&ctx->root, ctx->aggregate_handle);
    CHECK_STATUS( status );
}

void
DWCCDA_Put_Aggregate
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    int			    status;

    status = cda_put_aggregate
	(&ctx->root, &ctx->stream_handle, ctx->aggregate_handle);
    CHECK_STATUS( status );
    DWCCDA_Delete_Aggregate (ctx);
}

void
DWCCDA_Enter_Segment
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    int			    status;
    CDA_Y_Scope_Code	    scope_code;

    scope_code = DDIF_K_SEGMENT_SCOPE;

    status = cda_enter_scope
    (
	&ctx->root,			/* root-aggregate-handle */
	&ctx->stream_handle,		/* stream-handle */
	&scope_code,			/* scope-code */
	ctx->aggregate_handle		/* aggregate-handle */
    );
    CHECK_STATUS( status );
    DWCCDA_Delete_Aggregate( ctx);
}

void
DWCCDA_Enter_Scope
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Scope_Code	scope_code)
#else	/* no prototypes */
	(ctx, scope_code)
	DWCCDA_Context		ctx;
	CDA_Y_Scope_Code	scope_code;
#endif	/* prototype */
    {
    int			    status;

    status = cda_enter_scope
    (
	&ctx->root,		/* root-aggregate-handle */
	&ctx->stream_handle,	/* stream-handle */
	&scope_code,		/* scope-code */
	0			/* aggregate-handle */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Leave_Segment
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    int			    status;
    CDA_Y_Scope_Code	    scope_code;

    scope_code = DDIF_K_SEGMENT_SCOPE;

    status = cda_leave_scope
    (
	&ctx->root,		/* root-aggregate-handle */
	&ctx->stream_handle,	/* stream-handle	*/
	&scope_code		/* scope-code		*/
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Leave_Scope
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Scope_Code	scope_code)
#else	/* no prototypes */
	(ctx, scope_code)
	DWCCDA_Context		ctx;
	CDA_Y_Scope_Code	scope_code;
#endif	/* prototype */
    {
    int			    status;

    status = cda_leave_scope
    (
	&ctx->root,		/* root-aggregate-handle */
	&ctx->stream_handle,	/* stream-handle	*/
	&scope_code		/* scope-code		*/
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Push_Aggregate
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    int			    size;
    int			    d;
    size = ( sizeof( ctx->aggregate_handle ) / sizeof( unsigned long )) - 1;
    d = ++( ctx->depth );
    if( d > size )
	CHECK_STATUS( 0x0c );
    while( d > 0 )
	{
	ctx->aggregate_handle[ d ] = ctx->aggregate_handle[ d - 1 ];
	d--;
	}
}

void
DWCCDA_Pop_Aggregate
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    int			    d;
    int			    i;

    d = --( ctx->depth );
    if( d < 0 )
	CHECK_STATUS( 0x0c );
    for( i = 0; i <= d; i++)
	ctx->aggregate_handle[ i ] = ctx->aggregate_handle[ i + 1 ];
}

void
DWCCDA_Store_Aggregate
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Item	item)
#else	/* no prototypes */
	(ctx, item)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Item	item;
#endif	/* prototype */
    {
    int			    status;
    int			    buf_len;
    unsigned long	    value;

    value   = *ctx->aggregate_handle;
    buf_len = sizeof( value );

    DWCCDA_Pop_Aggregate( ctx);

    status = cda_store_item
    (
	&ctx->root,		/* root-aggregate-handle */
	ctx->aggregate_handle,  /* aggregate-handle */
	&item,			/* aggregate-item */
	&buf_len,		/* buf-len */
	&value,			/* buf-adr */
	0,			/* aggregate-index */
	0			/* add-info */
    );
    CHECK_STATUS( status );
}

void DWCCDA_Create_Aggregate
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context		ctx,
	CDA_Y_Aggregate_Type	aggregate_type)
#else	/* no prototypes */
	(ctx, aggregate_type)
	DWCCDA_Context		ctx;
	CDA_Y_Aggregate_Type	aggregate_type;
#endif	/* prototype */
    {
    int			    status;

    status = cda_create_aggregate
    (
	&ctx->root,		/* root-aggregate-handle */
	&aggregate_type,	/* aggregate-type */
	ctx->aggregate_handle	/* aggregate-handle */
    );
    CHECK_STATUS( status );
    
}

void
DWCCDA_Close_File
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    int			    status;

    status = cda_close_file
    (
	&ctx->stream_handle,	/* stream-handle	    */
	&ctx->file_handle	/* file-handle	    */
    );
    CHECK_STATUS( status );
}

void
DWCCDA_Close_Stream
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
{
    int			    status;

    status = cda_close_stream (&ctx->stream_handle);
    CHECK_STATUS( status );
}

void
DWCCDA_Finish
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    int			    status;

    DWCCDA_Leave_Segment( ctx);
    DWCCDA_Leave_Scope( ctx, DDIF_K_CONTENT_SCOPE);
    DWCCDA_Close_File( ctx);
    status = cda_delete_root_aggregate (&ctx->root);
    CHECK_STATUS( status );
}

void
DWCCDA_Init
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx,
	char		*file_spec,
	char		*product_identifier,
	char		*product_name,
	int		style_guide)
#else	/* no prototypes */
	(ctx, file_spec, product_identifier, product_name, style_guide)
	DWCCDA_Context	ctx;
	char		*file_spec;
	char		*product_identifier;
	char		*product_name;
	int		style_guide;
#endif	/* prototype */
    {
    CDA_Y_Aggregate_Type    DDF  = DDIF_DDF;
    int			    status;
    int			    file_spec_len;
    STRING		    ( default_file_spec, ".DDIF" );
    char		    result_file_spec[132];
    int			    result_file_spec_len  = sizeof( result_file_spec );
    int			    result_file_ret_len;

    status = cda_create_root_aggregate
    (
	0,		/* alloc-rtn */
	0,		/* dealloc-rtn */
	0,		/* alloc-dealloc-prm */
	0,		/* processing-options */
	&DDF,		/* aggregate-type */
	&ctx->root	/* root-aggregate-handle */
    );
    CHECK_STATUS( status );

    ctx->depth = 0;

    file_spec_len = strlen( file_spec );

    status = cda_create_file
    (
	&file_spec_len,			/* file-spec-len */
	file_spec,			/* file-spec */
	&default_file_spec_len,		/* default-file-spec-len */
	default_file_spec,		/* default-file-spec */
	0,				/* alloc-rtn */
	0,				/* dealloc-rtn */
	0,				/* alloc-dealloc-prm */
	&ctx->root,			/* root-aggregate-handle */
	&result_file_spec_len,		/* result-file-spec-len */
	result_file_spec,		/* result-file-spec */
	&result_file_ret_len,		/* result-file-ret-len */
	&ctx->stream_handle,		/* stream-handle */
	&ctx->file_handle		/* file_handle */
    );
    CHECK_STATUS( status );
    DWCCDA_Enter_Scope( ctx, DDIF_K_DOCUMENT_SCOPE );

    DWCCDA_Create_Aggregate( ctx, DDIF_DSC);
    DWCCDA_Store_Item_int( ctx, DDIF_DSC_MAJOR_VERSION, DDIF_K_MAJOR_VERSION);
    DWCCDA_Store_Item_int( ctx, DDIF_DSC_MINOR_VERSION, DDIF_K_MINOR_VERSION);
    DWCCDA_Store_Item_char( ctx, DDIF_DSC_PRODUCT_IDENTIFIER, product_identifier);
    DWCCDA_Store_Item_char_array( ctx, DDIF_DSC_PRODUCT_NAME, 0, product_name);
    DWCCDA_Put_Aggregate( ctx);
    DWCCDA_Create_Aggregate( ctx, DDIF_DHD);
    if( style_guide )
	{
	DWCCDA_Store_Item_int( ctx, DDIF_DHD_STYLE_GUIDE, 1);
	DWCCDA_Push_Aggregate( ctx);
	DWCCDA_Create_Aggregate( ctx, DDIF_ERF);
	    {
	    static const int erf_data_type[] = { 1, 3, 12, 1011, 1, 3, 1 };

	    DWCCDA_Store_Any_Item( ctx,
				DDIF_ERF_DATA_TYPE,
				(int *)erf_data_type,
				sizeof( erf_data_type ));
	    }
	DWCCDA_Store_Item_char_array( ctx, DDIF_ERF_DESCRIPTOR, 0, "DDIF");

#ifdef VMS
	DWCCDA_Store_Item_char( ctx, DDIF_ERF_LABEL, "CL_DAY");
#else
	DWCCDA_Store_Item_char( ctx, DDIF_ERF_LABEL, "cl_day");
#endif


#ifdef VMS
	DWCCDA_Store_Item_char_tag( ctx,
				    DDIF_ERF_LABEL_TYPE,
				    "$STYLE",
				    DDIF_K_STYLE_LABEL_TYPE);
#else
	DWCCDA_Store_Item_char_tag( ctx,
				    DDIF_ERF_LABEL_TYPE,
				    "$STYLE",
				    DDIF_K_STYLE_LABEL_TYPE);
#endif

	DWCCDA_Store_Item_int( ctx, DDIF_ERF_CONTROL, DDIF_K_NO_COPY_REFERENCE);
	DWCCDA_Store_Aggregate( ctx, DDIF_DHD_EXTERNAL_REFERENCES);
    }
    DWCCDA_Put_Aggregate( ctx);
    DWCCDA_Enter_Scope( ctx, DDIF_K_CONTENT_SCOPE);
    DWCCDA_Create_Aggregate( ctx, DDIF_SEG);
    DWCCDA_Push_Aggregate( ctx);
    DWCCDA_Create_Aggregate( ctx, DDIF_SGA);
    DWCCDA_Store_Item_int( ctx, DDIF_SGA_UNITS_PER_MEASURE, 7200);
    DWCCDA_Store_Item_char_array( ctx, DDIF_SGA_UNIT_NAME, 0, "centipoints");
    DWCCDA_Store_Aggregate( ctx, DDIF_SEG_SPECIFIC_ATTRIBUTES);
    DWCCDA_Enter_Segment( ctx);
}

void
DWCCDA_Hrd_New_Line
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    DWCCDA_Create_Aggregate( ctx, DDIF_HRD);
    DWCCDA_Store_Item_int( ctx, DDIF_HRD_DIRECTIVE, DDIF_K_DIR_NEW_LINE);
    DWCCDA_Put_Aggregate( ctx);
}

void
DWCCDA_Hrd_New_Page
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    DWCCDA_Create_Aggregate( ctx, DDIF_HRD);
    DWCCDA_Store_Item_int( ctx, DDIF_HRD_DIRECTIVE, DDIF_K_DIR_NEW_PAGE);
    DWCCDA_Put_Aggregate( ctx);
}

void
DWCCDA_Sft_New_Line
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
    {
    DWCCDA_Create_Aggregate( ctx, DDIF_SFT);
    DWCCDA_Store_Item_int( ctx, DDIF_SFT_DIRECTIVE, DDIF_K_DIR_NEW_LINE);
    DWCCDA_Put_Aggregate( ctx);
}

void DWCCDA_char
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx,
	char		*text)
#else	/* no prototypes */
	(ctx, text)
	DWCCDA_Context	ctx;
	char		*text;
#endif	/* prototype */
{
    DWCCDA_Create_Aggregate( ctx, DDIF_TXT);
    DWCCDA_Store_Item_char( ctx, DDIF_TXT_CONTENT, text);
    DWCCDA_Put_Aggregate( ctx);
}

void DWCCDA_chars
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx,
	int		length,
	char		*text)
#else	/* no prototypes */
	(ctx, length, text)
	DWCCDA_Context	ctx;
	int		length;
	char		*text;
#endif	/* prototype */
{
    DWCCDA_Create_Aggregate( ctx, DDIF_TXT);
    DWCCDA_Store_Item_chars( ctx, DDIF_TXT_CONTENT, length, text);
    DWCCDA_Put_Aggregate( ctx);
}

void DWCCDA_Text
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx,
	char		*text)
#else	/* no prototypes */
	(ctx, text)
	DWCCDA_Context	ctx;
	char		*text;
#endif	/* prototype */
{
    int	    current = 0;
    int	    start   = 0;

    while( text[ current ] != 0 )
	{
	switch( text[ current ] )
	    {
	    case( '\n' /* NL */ ):
		if( start - current != 0 )
		    DWCCDA_chars( ctx, current - start, text + start);
		DWCCDA_Hrd_New_Line( ctx);
		start = ++current;
		break;
	    case( '\014' /* FF */ ):
		if( start - current != 0 )
		    DWCCDA_chars( ctx, current - start, text + start);
		DWCCDA_Hrd_New_Page( ctx);
		start = ++current;
		break;
	    case( '\011' /* TAB */ ):
		if( start - current != 0 )
		    DWCCDA_chars( ctx, current - start, text + start);
		DWCCDA_Hrd_Tab( ctx);
		start = ++current;
		break;
	    default:
		current += 1;
		break;
	    }
	}
    if( start - current != 0 )
	DWCCDA_chars( ctx, current - start, text + start);
}

void DWCCDA_Hrd_Tab
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx)
#else	/* no prototypes */
	(ctx)
	DWCCDA_Context	ctx;
#endif	/* prototype */
{
    DWCCDA_Create_Aggregate( ctx, DDIF_HRD);
    DWCCDA_Store_Item_int(ctx, DDIF_HRD_DIRECTIVE, DDIF_K_DIR_TAB);
    DWCCDA_Put_Aggregate( ctx);
}

void DWCCDA_Segment_Type
#if defined(_DWC_PROTO_)
	(
	DWCCDA_Context	ctx,
	char		*segment_type)
#else	/* no prototypes */
	(ctx, segment_type)
	DWCCDA_Context	ctx;
	char		*segment_type;
#endif	/* prototype */
{
    DWCCDA_Create_Aggregate( ctx, DDIF_SEG);
    DWCCDA_Store_Item_char( ctx, DDIF_SEG_SEGMENT_TYPE, segment_type);
}
