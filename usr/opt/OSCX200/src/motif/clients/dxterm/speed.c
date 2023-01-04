/* #module speed.c "X0.0" */
/*
 *  Title:	speed.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988                                                       |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 *
 */

char speed_chars[95*2+1] = "\
 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\
`abcdefghijklmnopqrstuvwxyz{|}~\
 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\
`abcdefghijklmnopqrstuvwxyz{|}~";

int speed_bits = 0;
char speed_init_flag = 0;
int speed_t0, speed_t1;
char speed_tally[20] = "";
int speed_i = 0;

/*
 * Continually call speed with a pointer to a string, and speed will
 * give back next string preceded by running tally of how many bits per
 * second average.
 */
void speed (out) char out[];
{
if (!speed_init_flag) speed_init_flag = 1, speed_t0 = time ();

sprintf (out, "%s %.*s\015\012", speed_tally, 79-strlen(speed_tally),
	&speed_chars[speed_i]);

speed_bits += 820;

speed_t1 = time ();

if (speed_t1 > speed_t0)
	sprintf (speed_tally, "%d", speed_bits/ (speed_t1 - speed_t0));

if (++speed_i >= 95) speed_i = 0;
}
