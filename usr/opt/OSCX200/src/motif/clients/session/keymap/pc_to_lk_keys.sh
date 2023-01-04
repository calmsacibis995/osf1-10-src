#!/bin/sh
#
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
# HISTORY
#
case $1 in
'-u')
        # is there already PC-style keymappings?
        /usr/bin/X11/xmodmap -pke|grep -q Home
        # if no, then do the xmodmap
        if test $? = 1
        then
	    /usr/bin/X11/xmodmap - <<EOF
		keysym Help         = Print Sys_Req
		keysym Menu         = Scroll_Lock Help
		keysym Insert       = Home
		keysym Find         = Insert
		keysym DRemove      = Page_Up
		keysym Next         = Page_Down
		keysym Select       = Delete
		keysym Prior        = End
		keysym KP_0         = KP_Insert KP_0
		keysym KP_Decimal   = KP_Delete KP_Decimal
		keysym KP_1         = KP_End KP_1
		keysym KP_2         = KP_Down KP_2
		keysym KP_3         = KP_Page_Down KP_3
		keysym KP_4         = KP_Left KP_4
		keysym KP_6         = KP_Right KP_6
		keysym KP_Subtract  = KP_Add
		keysym KP_7         = KP_Home KP_7
		keysym KP_8         = KP_Up KP_8
		keysym KP_9         = KP_Page_Up KP_9
		keysym KP_F1        = Num_Lock
		keysym KP_F2        = KP_Divide
		keysym KP_F3        = KP_Multiply
		keysym KP_F4        = KP_Subtract
		add Mod4            = Num_Lock
EOF
	    if test $? = 0
	    then 
		echo "Using PC-style keymaps" 
	    fi
        fi
	;;
'-h')
	echo "Usage: pc_to_lk_keys.sh [ -u | -h ]"
        echo "where:"
        echo "  -u  Undo LK-style keymap modifications"
	echo "  -h  Usage summary"
        echo "<default> LK-style keymap modifications"
	;;
*)
        # is there already LK-style keymappings?
        /usr/bin/X11/xmodmap -pke|grep -q Menu
        # if no, then do the xmodmap
        if test $? = 1
        then
    	    /usr/bin/X11/xmodmap - <<EOF
		keysym Print        = Help
		keysym Scroll_Lock  = Menu
		keysym Insert       = Find
		keysym Home         = Insert
		keysym Page_Up      = DRemove
		keysym Page_Down    = Next
		keysym Delete       = Select
		keysym End          = Prior
		keysym KP_Insert    = KP_0
		keysym KP_Delete    = KP_Decimal
		keysym KP_End       = KP_1
		keysym KP_Down      = KP_2
		keysym KP_Page_Down = KP_3
		keysym KP_Left      = KP_4
		keysym KP_Right     = KP_6
		keysym KP_Subtract  = KP_F4
		keysym KP_Add       = KP_Subtract
		keysym KP_Home      = KP_7
		keysym KP_Up        = KP_8
		keysym KP_Page_Up   = KP_9
		keysym Num_Lock     = KP_F1
		keysym KP_Divide    = KP_F2
		keysym KP_Multiply  = KP_F3
                clear Mod4
EOF
	    if test $? = 0
	    then 
		echo "Using LK-style keymaps"
	    fi
        fi
	;;
esac

