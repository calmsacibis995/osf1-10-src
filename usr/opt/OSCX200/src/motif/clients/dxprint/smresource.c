/*
*****************************************************************************
**                                                                          *
**                  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY		    *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	PrintScreen
**
**  ABSTRACT:
**
**	Resources maintained by Print Screen.  This table is
**	used to hold default values for each of the resources.  In
**	addition, this is the only place that the names of the
**	resources are stored.  The index into a database is used when
**	we store the values back into files.     
**
**  ENVIRONMENT:
**
**	VAX/VMS Decwindows
**
**  MODIFICATION HISTORY:
**
**	12-Apr-1991	Edward P Luwish
**		Add new resources for new UI elements
**
*/
#include "iprdw.h"
#include "smstruct.h"
#include "smshare.h"
#include "prdw_entry.h"

/* 1 = string, 2=int,  3=int,int */
#define	tstring	1
#define tint	2
#define t2int   3

/* this will be a buffer of the following table, in resource file format
	for xloaddatabase to use */

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
globaldef noshare	char	*def_buffer = NULL;

globaldef noshare	struct	default_table
{
    char	*name;
    int		format;
    char	def_value[256];
    int		(*valid_check)();
    int		rdb_index;
    int		onroot;
} def_table[] =
{
{"PrintScreen.aspect_ratio",t2int,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.printer_color",tstring,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.filename",tstring,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.ribbon_saver",tstring,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.storage_format",tstring,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.height",tint,		"\000\000",	NULL, rdb_generic, 0},
{"PrintScreen.width",tint,		"\000\000",	NULL, rdb_generic, 0},
{"PrintScreen.file_prompt",tint,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.screenprompt",tint,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.screennum",tint,		"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.x",tint,			"\000\000",	NULL, rdb_generic, 0},
{"PrintScreen.y",tint,			"\000\000",	NULL, rdb_generic, 0},
{"PrintScreen.rotate_prompt",tint,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.message_region_text",tstring,"\000\000",	NULL, rdb_generic, 0},
{"PrintScreen.fit",tstring,		"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.orientation",tstring,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.page_size",tstring,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.sixel_device",tstring,	"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.delay",tint,		"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.capture",tstring,		"\000\000",	NULL, rdb_generic, 1},
{"PrintScreen.send",tstring,		"\000\000",	NULL, rdb_generic, 1},
};

#define num_elements	XtNumber(def_table);

#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
