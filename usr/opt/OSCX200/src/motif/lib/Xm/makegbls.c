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
 * this program is a quick hack to create a global strings file
 * with mathcing offsets from & to Xm.h. Note that Xm.h is not
 * overwritten but output is to Xm.ph
 *
 * ddhill 5/7/92
 */
#include <stdio.h>

#define X_GBLS "XM_GBLS"

char *tabs[] = {
    "\t",
    "\t\t",
    "\t\t\t",
    "\t\t\t\t",
    "\t\t\t\t\t",
    "\t\t\t\t\t\t",
};

main()
{
    FILE *cfile;
    FILE *hfile;
    char line[256];
    char *c;
    char *gbls;
    char *name;
    char *strstr();
    int cur_offset=0;
    int i;

    if((cfile=fopen("GlobalStr.c","w")) == NULL) {
	printf("could not open XmStrings.c\n");
	exit(1);
    }
    if((hfile=fopen("Xm.ph","w")) == NULL) {
	printf("could not open XmStrings.ph\n");
	exit(1);
    }

    fprintf(cfile,
    "\n\n/* Automatically generated file, do not edit by hand */\n\n");

    fprintf(cfile,"unsigned char XmStrings[] = {\n");

    while(gets(line)) {
	if(strncmp("#define",line,7) != 0)
	    fprintf(hfile,"%s\n",line);
	else if (!(gbls=strstr(line,X_GBLS)))
	    fprintf(hfile,"%s\n",line);
	else {
	    /* parse & replace */
	    c = line + 7; /* skip #define */

	    /* skip whitespace */
	    for(;(*c) && ((*c == ' ') || (*c == '\t'));c++);

	    name = c;
	    if (c == gbls) {
		fprintf(hfile,"%s\n",line);
		printf("skipping this line:\n%s\n",line);
		continue;
	    }


	    /* jump to end of name */
	    for(;(*c) && ! ((*c == ' ') || (*c == '\t'));c++);


	    *c = 0;

	    i = (31 - strlen(name))/8;
	    if(i < 0)
		i = 0;

	    gbls += 7; /* strlen X_GBLS( */

	    c = gbls;
	    for(;(*c) && ! ((*c == ',') || (*c == ')'));c++);
	    *c = 0;


	    fprintf(hfile,"#define %s%s%s(%s,%d)\n",
		name,tabs[i],X_GBLS,gbls,cur_offset);
		/*
	    printf("#define %s%s%s(%s,%d)\n",
		name,tabs[i],X_GBLS,gbls,cur_offset);
		*/

	    /* write the letters out... */
	    for(c=gbls;*c;c++,cur_offset++)
		fprintf(cfile,"'%c',",*c);
	    fprintf(cfile,"0,\n");
	    cur_offset++;
	}
    }

    fprintf(cfile,"};\n");
    fclose(cfile);
    fclose(hfile);
}
