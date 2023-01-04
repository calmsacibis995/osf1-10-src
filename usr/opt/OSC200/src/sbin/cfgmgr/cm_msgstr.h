
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

 */


char * cmgr_msgstr[] = {
	"No error", 
	"exiting",
	"reseting",
	"configuration manager already running",
	"must be run as super-user",
	"usage: %s [-c database] [-R#] [-A#] [-d] [-l] [-m]\n",
	"usage: %s [-c database] [-f file] [-on|-off| -onm| -offm] -adlup name\n",
	"usage: %s [-v] [-s] [-c|u|q|r[op[=val]] subsystem]\n",
	"%s: socket: %s\n",
	"%s: connect: %s\n",
	"%s: write: %s\n",
	"%s: Configuration manager is not available.\n",
	"%s: Unable to send configuration manager request\n",
	"%s: Stanza entry attribute list too long.\n",
	"%s: Stanza entry too long.\n",
	"%s: Stanza entry syntax error encountered.\n",
	"%s: Unable to update file from %s: %s\n",
	"%s: Already in automatic list\n",
	"%s: Not in automatic list\n",
	"%s: Entry not found in %s\n",
	"%s: Entry already exists in %s\n",
	"%s: Internal error\n",
	"%s: Duplicate entries found\n",
	"%s: entering automatic configuration mode\n",
	"%s: exiting automatic configuration mode\n",
	"%s: configuring automatic subsystem\n",
	"%s: %d subsystems configured successfully, %d failed\n",
	"%s: no automatic subsystems configured\n",
	"%s: detaching from parent session\n",
	"%s: entering command request mode\n",
	"%s: configuring requested subsystem\n",
	"cannot open local AF_UNIX domain socket",
	"cannot open remote AF_INET domain socket",
	"no communications sockets open",
	"loading subsystem",
	"unloading subsystem",
	"configuring subsystem",
	"unconfiguring subsystem",
	"querying subsystem",
	"calling subsystem configuration entry point",
	"successfully loaded subsystem",
	"successfully unloaded subsystem",
	"successfully configured subsystem",
	"successfully unconfigured subsystem",
	"successfully queried subsystem",
	"successfully called subsystem configuration entry",
	"cannot attach to kernel load server",
	"kernel loader server can not load subsystem module",
	"kernel loader server can not unload subsystem module",
	"subsystem module is not a dynamic subsystem",
	"subsystem module missing entry point",
	"subsystem module configuration operation failed",
	"cannot open configuration database",
	"cannot find entry",
	"cannot find attribute",
	"no more memory for parsing device names",
	"invalid syntax encountered in parsing string",
	"too many elements in parse string",
	"device name and minor number elements are not equal",
	"no such device directory",
	"invalid device file arguments",
	"not enough memory available to create device files",
	"cannot create device files subdirectory",
	"%s: creating device special files:\n",
	"%s: removing device special files:\n",
	"%s: %-32.32s  %-3d/%-3d  created\n",
	"%s: %-32.32s  %-3d/%-3d  failed to create\n",
	"%s: %-32.32s removed\n",
	"%s: %-32.32s failed to remove\n",
	"failed to load subsystem configuration method",
	"failed to unload subsystem configuration method",
	"failed to attach to user space loader to load method",
	"failed to locate subsystem configuration method",
	"subsystem configuration method missing entry point",
	"subsystem configuration method failed",
	"unknown configuration method operation",
	"not enough memory to create method",
	"kernel subsystem is already loaded",
	"kernel subsystem module is not currently loaded",
	"kernel subsystem is currently configured",
	"kernel subsystem module is not currently loaded",
	"kernel subsystem is already configured",
	"kernel subsystem module is not currently loaded",
	"kernel subsystem is not currently configured",
	"kernel subsystem is currently configured",
	"kernel subsystem configuration operation failed",
	"unknown kernel subsystem configuration operation",
	"not enough memory available to perform operation",
	"no such device",
	"invalid entry for this subsystem type",
	"can't get module state from kernel",
	"can't unload or unconfigure a static module",
	"invalid command [trace|notrace|debug|nodebug]",
	"successfully operated subsystem",
	"successfully started subsystem",
	"%s: %-32.32s  %-3d uid/%-3d gid failed to chown\n",
	"%s: entering multiuser configuration mode\n",
	"%s: exiting multiuser configuration mode\n",
	"%s: no multiuser subsystems configured\n",
	"%s: Already in multiuser list\n",
	"%s: Not in multiuser list\n",
	"not enough memory available to perform operation\n",
	"missing required next word field\n",
	"%s invalid connection specified\n",
	"expected next word of %s not %s\n",
	"Unrecognized type %s in line: %s\n",
	"invalid type 0x%x\n",
	"Invalid structure size %d\n",
	"%s%d already linked\n",
	"setsysinfo failed\n",
	"unable to parse config line\n",
	"%s field is required in stanza entry\n",
	"unrecognized option: %s in line: %s\n",
	"%s missing device type\n",
	"%s missing module name\n",
	"%s bad module name %s\n",
	"%s, %s configured twice\n",
	"%s, missing required field: at\n",
	"%s, missing required connection field\n",
	"%s, bad connection name %s\n",
	"%s, connections to nexus not yet supported\n",
	"%s missing slot number\n",
	"%s missing drive number\n",
	"%s missing vector name\n",
	"%s warning: vector %s ignored\n",
	"load failed due to errors in stanza config lines\n",
	"subsystem registration failed",
	"subsystem de-registration failed",
	"subsystem registration retrieval failed",
	"subsystem not currently registered",
	"message index exceeds table size",
	NULL
}; 
