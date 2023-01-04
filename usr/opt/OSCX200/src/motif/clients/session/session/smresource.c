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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/session/session/smresource.c,v 1.1.2.4 92/12/28 13:02:39 Don_Haney Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#define globalref extern
#define globaldef
#define noshare

#include "smshare.h"
#include "smresource.h"

/* this will be a buffer of the following table, in resource file format
	for xloaddatabase to use */

globaldef noshare	char	*def_buffer = NULL;

globaldef noshare	struct	default_table def_table[num_elements] =
{
{"*highlightColor",tstring,"black",NULL, rdb_color, 1},
{"*BorderColor",tstring,"black",NULL, rdb_color, 1},
{"DXsession.bell_enable", tstring, "enable", NULL, rdb_merge/*generic*/, 1},
{"DXsession.bell_percent",tint,"50",NULL, rdb_merge/*generic*/, 1},
{"DXsession.keyclick_enable", tstring, "enable", NULL, rdb_merge/*generic*/, 1},
{"DXsession.keyclick_percent",tint,"50",NULL, rdb_merge/*generic*/, 1},
{"DXsession.autorepeat_mode",tstring,"enable",NULL, rdb_merge/*generic*/, 1},
{"DXsession.lock_state",tstring,"caps",NULL, rdb_merge/*generic*/, 1},
{"DXsession.operator_window_key",tstring,"F2", NULL, rdb_merge/*generic*/, 1},
{"DXsession.operator_modifier_key",tstring,"ctrl", NULL, rdb_merge/*generic*/, 1},
{"DXsession.screen_saver_enable",tstring,"enable",NULL, rdb_merge/*generic*/, 1},
{"DXsession.screen_saver_period",tint,"10", NULL, rdb_merge/*generic*/, 1},
{"DXsession.display_foreground",tstring,"black", NULL, rdb_color, 1},
{"DXsession.display_background",tstring,"white", NULL, rdb_color, 1},
{"DXsession.display_pattern",tint,"0", NULL, rdb_color, 1},
{"*Foreground",tstring,"black", NULL, rdb_color, 1},
{"*Background",tstring,"white",NULL, rdb_color, 1},
{"DXsession.pointer_foreground",tstring,"white",NULL, rdb_color, 1},
{"DXsession.pointer_background",tstring,"black",NULL, rdb_color, 1},
{"DXsession.pointer_button_order",tstring,"right", NULL, rdb_merge/*generic*/, 1},
{"DXsession.pointer_shape",tint,"68", NULL, rdb_merge/*generic*/, 1},
{"DXsession.mouse_accel_numerator",tint,"-1",NULL, rdb_merge/*generic*/, 1},
{"DXsession.mouse_accel_denominator",tint,"-1",NULL, rdb_merge/*generic*/, 1},
{"DXsession.mouse_accel_threshold",tint,"-1",NULL, rdb_merge/*generic*/, 1},
{"*multiClickTime",tint,"250",NULL, rdb_merge/*generic*/, 1},
{"DXsession.printer.aspect_ratio",t2int,"1,1",NULL, rdb_merge/*generic*/, 1},
{"DXsession.printer.color",tstring,"bw", NULL, rdb_merge/*generic*/, 1},
{"DXsession.printer.filename",tstring,"printscreen.ps", NULL, rdb_merge/*generic*/, 1},
{"DXsession.printer.ribbon_saver",tstring,"positive",NULL, rdb_merge/*generic*/, 1},
{"DXsession.printer.storage_format",tstring,"postscript",NULL, rdb_merge/*generic*/, 1},
{"DXsession.startup_state",tstring,"mapped",NULL, rdb_merge/*generic*/, 1},
{"DXsession.confirm_endsession",tint,"1",NULL, rdb_merge/*generic*/, 1},
{"DXsession.host_list",tstring,"",NULL,rdb_merge/*generic*/, 1},
{"DXsession.num_hosts",tstring,"0",NULL,rdb_merge/*generic*/, 1},
{"DXsession.printer.file_prompt",tint,"1",NULL, rdb_merge/*generic*/, 1},
{"*keyboard_dialect",tstring,"System Default", NULL, rdb_merge/*generic*/, 1},
{"*WmIconForm.IconStyle",tint,"1", NULL, rdb_merge/*generic*/, 1},
{"wm*WmForm.BorderColor",tstring,"white", NULL, rdb_color, 1},
{"wm*WmForm.Foreground",tstring,"black", NULL, rdb_color, 1},
{"DXsession.screennum",tint,"-1", NULL, rdb_merge/*generic*/, 1},
{"DXsession.screenprompt",tint,"0",  NULL, rdb_merge/*generic*/, 1},
{"DXsession.printer.screennum",tint,"-1", NULL, rdb_merge/*generic*/, 1},
{"DXsession.printer.screenprompt",tint,"0", NULL, rdb_merge/*generic*/, 1},
{"*xnlLanguage",tstring,"",NULL, rdb_merge/*generic*/, 1},
{"DXsession.AppMenu",tstring,"",NULL, rdb_merge/*generic*/, 1},
{"DXsession.num_AppMenu",tint,"0",NULL, rdb_merge/*generic*/, 1},
{"DXsession.AutoStart",tstring,"",NULL, rdb_merge/*generic*/, 1},
{"DXsession.num_AutoStart",tint,"0",NULL, rdb_merge/*generic*/, 1},

{"DXsession.x",tint,"0",NULL, rdb_merge/*generic*/, 1},
{"DXsession.y",tint,"0",NULL, rdb_merge/*generic*/, 1},
{"DXsession.pause_text",tstring, "Type your password to resume the session",NULL, rdb_merge/*generic*/, 1},
{"DXsession.windowManagerName",tstring,"System Default",NULL, rdb_merge/*generic*/, 1},
{"DXsession.applications",tstring,"a,b,c,d",NULL, rdb_merge/*generic*/, 1},
{"DXsession.num_applications",tint,"2",NULL, rdb_merge/*generic*/, 1},
{"DXsession.Terminal Window.command",tstring,"/usr/bin/X11/dxterm",NULL, rdb_merge/*generic*/, 1},
{"DXsession.UE Window.command",tstring,"/usr/bin/X11/dxue",NULL, rdb_merge/*generic*/, 1},
{"DXsession.printer.rotate_prompt",tint,"1",NULL, rdb_merge/*generic*/, 1},
{"DXsession.pauseSession",tstring,"/usr/bin/X11/dxpause",NULL, rdb_merge/*generic*/, 1},
};


char	**get_remove_list(removecount)
unsigned    int	*removecount;
{
    unsigned int i,j;
    unsigned int count = 0;
    char	**removelist;

    for (i=0; i<num_elements; i++)
        if (def_table[i].onroot == 0) count++;

    if (count == 0) {
        *removecount = 0;
        return(NULL);
    }
    removelist = (char **)malloc(count * sizeof(char *));
    for (i=0,j=0; i<num_elements; i++)
        if (def_table[i].onroot == 0) {
	    removelist[j] = (char *)malloc(strlen(def_table[i].name) + 1);
	    strcpy(removelist[j++], def_table[i].name);
	}

    *removecount = count;
    return(removelist);
}
