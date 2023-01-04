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
BEGIN { 
	l=0 
	c=0 
	e=0 
	s=0 
	g=0 
	f=0 
	t=0 
    }  
$2 ~ /libs/ 	{l++}  
$2 ~ /clients/ 	{c++}  
$2 ~ /ext/ 	{e++}  
$2 ~ /server/ 	{s++}  
$2 ~ /general/ 	{g++}  
$2 ~ /fonts/ 	{f++}  
$1 ~ /^[0-9]../	{t++} 
END{ 
	printf("Work to do...\n")
	printf("   libs: %.3d\n", l) 
	printf("clients: %.3d\n", c) 
	printf("    ext: %.3d\n", e) 
	printf(" server: %.3d\n", s) 
	printf("general: %.3d\n", g) 
	printf("  fonts: %.3d\n", f) 
	printf("--------------\n") 
	printf("  total: %.3d\n", t)
} 

