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
	firstchar = "@";
	a["a"] = "A"; a["b"] = "B"; a["c"] = "C";
	a["d"] = "D"; a["e"] = "E"; a["f"] = "F";
	a["g"] = "G"; a["h"] = "H"; a["i"] = "I";
	a["j"] = "J"; a["k"] = "K"; a["l"] = "L";
	a["m"] = "M"; a["n"] = "N"; a["o"] = "O";
	a["p"] = "P"; a["q"] = "Q"; a["r"] = "R";
	a["s"] = "S"; a["t"] = "T"; a["u"] = "U";
	a["v"] = "V"; a["w"] = "W"; a["x"] = "X";
	a["y"] = "Y"; a["z"] = "Z";
}

{
	c = substr($2,2,1);
	if (c >= "a" && c <= "z")
		c = a[c];
	if (c != firstchar)
		printf(".LB %s\n", c);
	firstchar = c;
	print;
}
