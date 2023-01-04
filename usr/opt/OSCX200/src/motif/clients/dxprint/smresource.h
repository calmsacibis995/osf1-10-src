/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991 BY                                       *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	Pair to smresource.c file.   This file has all of the
**	indexes into the table that lists all of the printscreen
**	resources and their default values.
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	12-Apr-1991	Edward P Luwish
**		Add new resources for new UI elements
**
*/
/* Types of representations for resources */
#define	tstring	1
#define tint	2
#define t2int	3
/* note that these types don't exist, but code is already written to
    support them */
#define	t3string 4
#define	t3int	5
#define t2char	6
#define t3char	7
#define t4char 8

/* define literals for the index into this structure */
#define paspect 0
#define pcolor 1
#define pfile 2
#define psaver 3
#define pformat 4
#define smrows 5
#define smcols 6
#define prtprompt 7
#define pscreennum 8
#define pscreenprompt 9
#define smx 10
#define smy 11
#define prtrotateprompt 12
#define smtext 13
#define pfit 14
#define porientation 15
#define psize 16
#define psixel 17
#define pdelay 18
#define pcapture 19
#define psend 20

#define num_elements 21

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif

globalref	char	*def_buffer;

globalref	struct	default_table
{
    char	*name;
    int	format;
    char	def_value[256];
    int	(*valid_check)();
    int	rdb_index;
    int	onroot;
}def_table[num_elements];

#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif
